/*
 * Copyright (C) 2010 Stollmann E+V GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/st21nfca.h>
#include <linux/syscalls.h>


#define MAX_BUFFER_SIZE	256

/* define the active state of the WAKEUP pin */
#define ST21_IRQ_ACTIVE_HIGH 1
#define ST21_IRQ_ACTIVE_LOW 0

/* this defines tha active state thate we use, it is at the same time the value that gpio_get-value returns */
#define ST21_IRQ_POLARITY ST21_IRQ_ACTIVE_LOW

#if ST21_IRQ_POLARITY == ST21_IRQ_ACTIVE_LOW
#define ST21_IRQ_TYPE     IRQ_TYPE_LEVEL_LOW
#define ST21_IRQ_TRIGGER  IRQF_TRIGGER_LOW
#else
#define ST21_IRQ_TYPE     IRQ_TYPE_LEVEL_HIGH
#define ST21_IRQ_TRIGGER  IRQF_TRIGGER_HIGH
#endif

/* Flag for alarm only once */
static int g_i2c_error_alarmed = false;

struct st21nfca_dev	{
	wait_queue_head_t	read_wq;
	struct mutex		read_mutex;
	struct mutex		write_mutex;
	struct i2c_client	*client;
	struct miscdevice	st21nfca_device;
	unsigned int		irq_gpio;
	bool			irq_enabled;
	spinlock_t		irq_enabled_lock;
	int (*read_ctrl)(void);
	int (*write_ctrl)(void);
#ifdef NFC_DEBUG_ENABLE
	void (*rwcheck)(int,int,void *);
	long (*tid)(void);
#endif
};

#ifdef NFC_DEBUG_ENABLE
void (*trc_log)(int flag, int fid);

#define NFC_DUMP_TRC(FLAG,FID) \
    if(trc_log != NULL){trc_log(FLAG,FID);}

#define NFC_LOG_MSG(msg) \
    pr_err("%10ld,%25s,%5d," msg , sys_gettid(), __func__, __LINE__);

#define NFC_LOG_FMT(fmt,...) \
    pr_err("%10ld,%25s,%5d," fmt , sys_gettid(), __func__, __LINE__, __VA_ARGS__);

#else  /* NFC_DEBUG_ENABLE */

#define NFC_DUMP_TRC(FLAG,FID)
#define NFC_LOG_MSG(msg)
#define NFC_LOG_FMT(fmt,...)

#endif /* NFC_DEBUG_ENABLE */

#define NFC_FID_st21nfca_dev_read          7001
#define NFC_FID_st21nfca_resume            7002
#define NFC_FID_get_st21nfca_info          7003
#define NFC_FID_st21nfca_dev_open          7004
#define NFC_FID_st21nfca_dev_ioctl         7005
#define NFC_FID_st21nfca_dev_init          7006
#define NFC_FID_st21nfca_dev_write         7007
#define NFC_FID_st21nfca_disable_irq       7008
#define NFC_FID_set_st21nfca_trc_info      7009
#define NFC_FID_st21nfca_dev_exit          7010
#define NFC_FID_st21nfca_remove            7011
#define NFC_FID_st21nfca_poll              7012
#define NFC_FID_st21nfca_probe             7013
#define NFC_FID_st21nfca_dev_irq_handler   7014
#define NFC_FID_st21nfca_suspend           7015


struct st21nfca_dev *g_st21nfca = NULL;

static void st21nfca_disable_irq(struct st21nfca_dev *st21nfca_dev)
{
	unsigned long flags;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_disable_irq);

	spin_lock_irqsave(&st21nfca_dev->irq_enabled_lock, flags);
	if (st21nfca_dev->irq_enabled) {
		disable_irq_nosync(st21nfca_dev->client->irq);
		st21nfca_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&st21nfca_dev->irq_enabled_lock, flags);
	NFC_DUMP_TRC(1,NFC_FID_st21nfca_disable_irq);
}

static irqreturn_t st21nfca_dev_irq_handler(int irq, void *dev_id)
{
	struct st21nfca_dev *st21nfca_dev = dev_id;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_dev_irq_handler);

	st21nfca_disable_irq(st21nfca_dev);
	//pr_debug("%s ####$$$$$ H I T     M E     $$$$$$#####\n", __func__);

	/* Wake up waiting readers */
	wake_up(&st21nfca_dev->read_wq);

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_irq_handler);
	return IRQ_HANDLED;
}

static ssize_t st21nfca_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	struct st21nfca_dev *st21nfca_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE];
	int ret;
	int ret2;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_dev_read);

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if( st21nfca_dev->read_ctrl != NULL )
	{
		ret2 = st21nfca_dev->read_ctrl();
		if(ret2 != 0)
		{
			NFC_LOG_MSG("DBG,read reject from nfc driver.\n");
			NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_read);
			return -EIO;
		}
	}
	NFC_LOG_FMT("DBG,reading %zu bytes.\n", count);

	mutex_lock(&st21nfca_dev->read_mutex);

	/* Read data */
	ret = i2c_master_recv(st21nfca_dev->client, tmp, count);
	mutex_unlock(&st21nfca_dev->read_mutex);

	if (ret < 0) {
		pr_err("%s: i2c_master_recv returned %d\n", __func__, ret);
		NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_read);
		if (g_i2c_error_alarmed == false) {
			printk(KERN_ERR "[T][ARM]Event:0x79 Info:0x%02x%02x%02x%02x\n",0x10, (-ret) & 0xFF, count, 0x01);
			g_i2c_error_alarmed = true;
		}
		return ret;
	} else {
		g_i2c_error_alarmed = false;
	}

#ifdef NFC_DEBUG_ENABLE
	if( st21nfca_dev->rwcheck != NULL )
	{
		st21nfca_dev->rwcheck(0,ret,tmp);
	}
#endif /* NFC_DEBUG_ENABLE */

	if (ret > count) {
		pr_err("%s: received too many bytes from i2c (%d)\n",
			__func__, ret);
		NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_read);
		return -EIO;
	}
	if (copy_to_user(buf, tmp, ret)) {
		pr_warning("%s : failed to copy to user space\n", __func__);
		NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_read);
		return -EFAULT;
	}
	NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_read);
	return ret;
}

static ssize_t st21nfca_dev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *offset)
{
	struct st21nfca_dev  *st21nfca_dev;
	char tmp[MAX_BUFFER_SIZE];
	int ret=count;
	int ret2;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_dev_write);

	st21nfca_dev = filp->private_data;
	NFC_LOG_FMT("st21nfca_dev ptr %p\n",st21nfca_dev);

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (copy_from_user(tmp, buf, count)) {
		pr_err("%s : failed to copy from user space\n", __func__);
		NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_write);
		return -EFAULT;
	}

	NFC_LOG_FMT("DBG,writing %zu bytes.\n", count);
	/* Write data */

	if( st21nfca_dev->write_ctrl != NULL )
    {
		ret2 = st21nfca_dev->write_ctrl();
		if(ret2 != 0)
		{
			NFC_LOG_MSG("DBG,write reject from nfc driver.\n");
			NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_write);
			return -EIO;
		}
    }
    mutex_lock(&st21nfca_dev->write_mutex);
    ret = i2c_master_send(st21nfca_dev->client, tmp, count);
    mutex_unlock(&st21nfca_dev->write_mutex);

	if (ret != count) {
		pr_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
		NFC_DUMP_TRC(4,NFC_FID_st21nfca_dev_write);
		if (g_i2c_error_alarmed == false) {
			printk(KERN_ERR "[T][ARM]Event:0x79 Info:0x%02x%02x%02x%02x\n",0x10, (-ret) & 0xFF, count, 0x02);
			g_i2c_error_alarmed = true;
		}
	} else {
		g_i2c_error_alarmed = false;
	}

#ifdef NFC_DEBUG_ENABLE
	if( (st21nfca_dev->rwcheck != NULL) && (ret > 0) )
	{
		st21nfca_dev->rwcheck(1,ret,tmp);
	}
#endif /* NFC_DEBUG_ENABLE */

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_write);
	return ret;
}

static int st21nfca_dev_open(struct inode *inode, struct file *filp)
{
	struct st21nfca_dev *st21nfca_dev;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_dev_open);

    if(g_st21nfca == NULL)
    {
        NFC_LOG_MSG("ERR,Not inited\n");
        return -ENODEV;
    }

	st21nfca_dev = container_of(filp->private_data, struct st21nfca_dev, st21nfca_device);

	filp->private_data = st21nfca_dev;

	NFC_LOG_FMT("%d,%d\n", imajor(inode), iminor(inode));

	NFC_LOG_FMT("st21nfca_dev ptr %p\n",st21nfca_dev);

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_open);
	return 0;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int st21nfca_dev_ioctl(struct inode *inode, struct file *filp,
                              unsigned int cmd, unsigned long arg)
#else
static long st21nfca_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	struct st21nfca_dev *st21nfca_dev = filp->private_data;
	int ret = 0;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_dev_ioctl);

	switch (cmd) {
	case ST21NFCA_GET_WAKEUP:
	    /* deliver state of Wake_up_pin as return value of ioctl */
		ret = gpio_get_value(st21nfca_dev->irq_gpio);
    if(ret>=0 && ST21_IRQ_POLARITY==ST21_IRQ_ACTIVE_LOW)
    {
       ret = 1-ret;
    }
		NFC_LOG_FMT("DBG,get gpio result %d\n",ret);
		NFC_DUMP_TRC(4,ret);
		break;
	default:
		pr_err("%s bad ioctl %u\n", __func__, cmd);
		ret = -EINVAL;
		break;
	}

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_ioctl);
	return ret;
}
static unsigned int st21nfca_poll(struct file *file, poll_table *wait)
{
	struct st21nfca_dev  *st21nfca_dev = file->private_data;
	unsigned int mask = 0;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_poll);

	/* wait for Wake_up_pin == high  */
	NFC_LOG_MSG("call poll_Wait\n");
    NFC_DUMP_TRC(4,NFC_FID_st21nfca_poll);
	poll_wait(file, &st21nfca_dev->read_wq, wait);
    NFC_DUMP_TRC(4,NFC_FID_st21nfca_poll);

	if( ST21_IRQ_POLARITY == gpio_get_value(st21nfca_dev->irq_gpio)){

		NFC_LOG_MSG("DBG,return ready\n");
		mask = POLLIN | POLLRDNORM; /* signal data avail */
		st21nfca_disable_irq(st21nfca_dev);
	}
	else{
		/* Wake_up_pin  is low. Activate ISR  */
		if(!st21nfca_dev->irq_enabled){
			NFC_LOG_MSG("DBG,enable irq\n");
			st21nfca_dev->irq_enabled = true;
			enable_irq(st21nfca_dev->client->irq);
		}
		else{
			NFC_LOG_MSG("DBG,irq already enabled\n");
		}

	}

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_poll);
	return mask;
}


static const struct file_operations st21nfca_dev_fops = {
	.owner	= THIS_MODULE,
	.llseek	= no_llseek,
	.read	= st21nfca_dev_read,
	.write	= st21nfca_dev_write,
	.open	= st21nfca_dev_open,
	.poll   = st21nfca_poll,

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
	.ioctl  = st21nfca_dev_ioctl,
#else
  .unlocked_ioctl = st21nfca_dev_ioctl,
#endif
};

static int st21nfca_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret;
	struct st21nfca_i2c_platform_data *platform_data;
	struct st21nfca_dev *st21nfca_dev;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_probe);

	platform_data = client->dev.platform_data;
	NFC_LOG_FMT("clientptr %p\n", client);

	if (platform_data == NULL) {
		pr_err("%s : st21nfca probe fail\n", __func__);
		NFC_DUMP_TRC(1,NFC_FID_st21nfca_probe);
		return  -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s : need I2C_FUNC_I2C\n", __func__);
		NFC_DUMP_TRC(1,NFC_FID_st21nfca_probe);
		return  -ENODEV;
	}

	client->adapter->timeout = msecs_to_jiffies(3 * 10); /* 30ms */
	client->adapter->retries = 0;



	st21nfca_dev = kzalloc(sizeof(*st21nfca_dev), GFP_KERNEL);
	if (st21nfca_dev == NULL) {
		dev_err(&client->dev,
				"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}
	NFC_LOG_FMT("dev_cb_addr %p\n", st21nfca_dev);

	st21nfca_dev->irq_gpio = platform_data->irq_gpio;
	st21nfca_dev->client   = client;

	ret = platform_data->gpio_init();
	if (ret != 0)
    {
		pr_err("%s : gpio_init failed\n", __FILE__);
		kfree(st21nfca_dev);
		ret = -ENODEV;
		goto err_exit;
	}


	ret = irq_set_irq_type(client->irq , ST21_IRQ_TYPE);
	if (ret){
		pr_err("%s : irq_set_irq_type failed\n", __FILE__);
		platform_data->gpio_exit();
		kfree(st21nfca_dev);
		ret = -ENODEV;
		goto err_exit;
	}

	client->irq = gpio_to_irq(platform_data->irq_gpio);
	enable_irq_wake(client->irq);
	/* init mutex and queues */
	init_waitqueue_head(&st21nfca_dev->read_wq);
	mutex_init(&st21nfca_dev->read_mutex);
	mutex_init(&st21nfca_dev->write_mutex);
	spin_lock_init(&st21nfca_dev->irq_enabled_lock);

	st21nfca_dev->st21nfca_device.minor = MISC_DYNAMIC_MINOR;
	st21nfca_dev->st21nfca_device.name = "st21nfca";
	st21nfca_dev->st21nfca_device.fops = &st21nfca_dev_fops;

	ret = misc_register(&st21nfca_dev->st21nfca_device);
	if (ret) {
		pr_err("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	NFC_LOG_FMT("requesting IRQ %d\n", client->irq);
	st21nfca_dev->irq_enabled = true;
	ret = request_irq(client->irq, st21nfca_dev_irq_handler,
			  ST21_IRQ_TRIGGER, client->name, st21nfca_dev);
	if (ret) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}
	st21nfca_disable_irq(st21nfca_dev);
	i2c_set_clientdata(client, st21nfca_dev);

#ifdef NFC_DEBUG_ENABLE
	st21nfca_dev->tid = sys_gettid;
#endif /* NFC_DEBUG_ENABLE */
	g_st21nfca = st21nfca_dev;

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_probe);
	return 0;

err_request_irq_failed:
	misc_deregister(&st21nfca_dev->st21nfca_device);
err_misc_register:
	mutex_destroy(&st21nfca_dev->read_mutex);
	mutex_destroy(&st21nfca_dev->write_mutex);
	platform_data->gpio_exit();
	kfree(st21nfca_dev);
err_exit:

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_probe);
	return ret;
}

static int st21nfca_remove(struct i2c_client *client)
{
	struct st21nfca_dev *st21nfca_dev;
	struct st21nfca_i2c_platform_data *platform_data;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_remove);

	g_st21nfca = NULL;
	st21nfca_dev = i2c_get_clientdata(client);
	free_irq(client->irq, st21nfca_dev);
	misc_deregister(&st21nfca_dev->st21nfca_device);
	mutex_destroy(&st21nfca_dev->read_mutex);
	mutex_destroy(&st21nfca_dev->write_mutex);

	platform_data = client->dev.platform_data;
	platform_data->gpio_exit();

	kfree(st21nfca_dev);


	NFC_DUMP_TRC(1,NFC_FID_st21nfca_remove);
	return 0;
}

static const struct i2c_device_id st21nfca_id[] = {
	{ "st21nfca", 0 },
	{ }
};

static int st21nfca_suspend(struct i2c_client *client, pm_message_t mesg)
{
	NFC_DUMP_TRC(0,NFC_FID_st21nfca_suspend); 
           printk("[st21nfca_suspend]\n");
//           gpio_set_value(98, 1);

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_suspend);
           return 0;
}

static int st21nfca_resume(struct i2c_client *client)
{
	NFC_DUMP_TRC(0,NFC_FID_st21nfca_resume);

           printk("[st21nfca_resume]\n");
//           gpio_set_value(98, 1);

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_resume);
           return 0;
}

static struct i2c_driver st21nfca_driver = {
           .id_table = st21nfca_id,
           .probe             = st21nfca_probe,
           .remove            = st21nfca_remove,
           .suspend          = st21nfca_suspend,
           .resume            = st21nfca_resume,
           .driver              = {
                     .owner  = THIS_MODULE,
                     .name   = "st21nfca",
           },
};


/*
 * module load/unload record keeping
 */

static int __init st21nfca_dev_init(void)
{
	int ret;

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_dev_init);

#ifdef NFC_DEBUG_ENABLE
	trc_log = NULL;
#endif /* NFC_DEBUG_ENABLE */

	NFC_LOG_MSG("Loading st21nfca driver\n");
	ret = i2c_add_driver(&st21nfca_driver);

    if( ret < 0 )
    {
        NFC_LOG_MSG("ERR,Init fail\n");
    }

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_init);
	return 0;
}
module_init(st21nfca_dev_init);

static void __exit st21nfca_dev_exit(void)
{

	NFC_DUMP_TRC(0,NFC_FID_st21nfca_dev_exit);

    if(g_st21nfca != NULL)
    {
        NFC_LOG_MSG("Unloading st21nfca driver\n");
        i2c_del_driver(&st21nfca_driver);
    }

	NFC_DUMP_TRC(1,NFC_FID_st21nfca_dev_exit);

}
module_exit(st21nfca_dev_exit);

#ifdef NFC_DEBUG_ENABLE
void set_st21nfca_trc_info(void *ptr)
{
	if( ptr != NULL)
	{
		trc_log = ptr;
	}
    
    return;
}
EXPORT_SYMBOL(set_st21nfca_trc_info);
#endif /* NFC_DEBUG_ENABLE */

void * get_st21nfca_info(void)
{
	return g_st21nfca;
}

EXPORT_SYMBOL(get_st21nfca_info);

MODULE_AUTHOR("Norbert Kawulski");
MODULE_DESCRIPTION("NFC ST21NFCA driver");
MODULE_LICENSE("GPL");
