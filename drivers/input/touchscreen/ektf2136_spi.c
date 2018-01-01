/**
 * @file drivers/input/touchscreen/ektf2136_spi.c - ELAN EKTF2136 touchscreen driver
 * Copyright (C) 2011 Elan Microelectronics Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * @date 2011/9/29
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

/*! Debug flag. */
//#define DEBUG
#define DEBUG
#define ON_PLATFORM

#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/kfifo.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/firmware.h>
#include <linux/atomic.h>
#include <mach/socinfo.h>

#ifdef ON_PLATFORM
#include <linux/touch_panel_cmd.h>
#endif

//=============================================================================
//  Marco
//=============================================================================
#define DRV_NAME	"elan_ts_spi_ss"
#define DRIVER_VERSION	"v0.1.0"

#define DRV_MA_VER 0
#define DRV_MI_VER 1
#define DRV_SUB_MI_VER 0

#define IDX_PACKET_SIZE_XY		18
#define IDX_PACKET_SIZE_WIDTH	21
#define IDX_PACKET_SIZE		IDX_PACKET_SIZE_XY

#define FINGER_NUM			5
#define BUTTON_NUM			4

#define PWR_STATE_SLEEP		1
#define PWR_STATE_NORMAL	0

#define SOFT_RESET			0x77

#define FINGER_ID			1

#define FIFO_SIZE 			(64)

#define	TS_SETUP_HELLO_ERR	(55)
#define TS_SETUP_FWVER_ERR	(66)
#define TS_SETUP_HWVER_ERR	(77)


/*! Convert from rows or columns into resolution */
#define ELAN_TS_RESOLUTION(n)		((n - 1) * 64)

static const char hello_packet[4] = { 0x55, 0x55, 0x55, 0x55 };

static const short elan_button_key[4] = {KEY_SEARCH, KEY_BACK, KEY_HOME, KEY_MENU};

#include <linux/spi/ektf2136.h>

/*! Firmware protocol status flag */
#define PRO_SPI_WRT_CMD_SYNC	0x00000001
#define PRO_HID_MOD_CHECKSUM	0x00000002
#define PRO_UPDATE_FW_MODE		0x00000080


/*! driver status flag, should move to public header file */
#define STA_NONINIT         0x00000001
#define STA_INIT            0x00000002
#define STA_INIT2           0x00000004
#define STA_INIT3           0x00000100
#define STA_INIT4           0x00000200
#define STA_INIT5           0x00000400
#define STA_PROBED          0x00000008
#define STA_ERR_HELLO_PKT   0x00000010
#define STA_USE_IRQ         0x00000020
#define STA_SLEEP_MODE      0x00000040

//=============================================================================
//  Structure Definition
//=============================================================================

/*! @enum finger_report_header Finger report header byte definition */
enum finger_report_header {
    idx_coordinate_packet_2_finger = 0x5a,
    idx_coordinate_packet_3_finger = 0x5b,
    idx_coordinate_packet_4_finger = 0x5c,
    idx_coordinate_packet_5_finger = 0x5d,
};

/*! @enum fw_normal_cmd_hdr FW Normal command header definition */
enum fw_normal_cmd_hdr {
    cmd_header_byte_write = 0x54,
    cmd_header_byte_read = 0x53,
    cmd_header_byte_response = 0x52,
    cmd_header_byte_calibration = 0x66,
    cmd_header_byte_hello = 0x55,
    cmd_response_len = 4
};


/*! @enum fw_info_pos FW information position */
enum fw_info_pos {
    idx_finger_header		= 0,
    idx_finger_state		= 1,
    idx_finger_total		= 1,
    idx_finger_btn			= 17,
    idx_finger_width		= 18,
};

/*! @enum lock_bit Lock bit definition */
enum lock_bit {
    idx_file_operate = 0,
    idx_cmd_handshake = 1,
    idx_finger_report = 2,
    idx_sysfs_busy = 4,
    idx_touch_cmd_busy = 8,
};


/*! @struct <elan_polling> */
struct elan_polling {
	struct workqueue_struct *elan_wq;	/// polling work queue
	struct timer_list timer;	/// Polling intr_gpio timer
	u8 int_status;				/// polling intr gpio status
	u8 button_status;			/// polling button status
};

/*! @enum iap_define */
enum iap_define {
    PageSize		= 132,
    PageNum			= 249,

    IAP_Mode_0		= 0x54,
    IAP_Mode_1		= 0x00,
    IAP_Mode_2		= 0x12,
    IAP_Mode_3		= 0x34,

    IAP_Master		= 0x15,
    IAP_Slave_1		= 0x16,
    IAP_Slave_2		= 0x17,
    IAP_Slave_3		= 0x18,

    ACK_Fail		= 0x55,
    ACK_OK			= 0xAA,

    IAP_Reset		= 0x77,
};

typedef struct elan_abs_struct {
	u32		X;
	u32		Y;
} elan_abs_type;


/**
*
* @struct elan_data
*
* all variable should include in this struct.
*
*/
struct elan_data {
	int intr_gpio;		/// interupter pin
	int rst_gpio;		/// reset pin
	int cs_gpio;		/// spi_cs pin
	int use_irq;
	atomic_t cs_valid_count;	/// number of valid operation for spi_cs
	u8 major_fw_version;
	u8 minor_fw_version;
	u8 major_bc_version;
	u8 minor_bc_version;
	u8 major_hw_id;
	u8 minor_hw_id;
	bool fw_enabled;	/// True if firmware device enabled
	int rows;			/// Panel geometry for input layer
	int cols;
	int x_max;
	int y_max;
	u8 suspend_state;		/// Suspend state 0:Normal 1:Suspend
	u8 power_state;			/// Power state 0:sleep 1:active
#define IAP_MODE_ENABLE		1	/// TS is in IAP mode already 
	unsigned int iap_mode;		/// Firmware update mode or 0 for normal
	unsigned int rx_size;		/// Read size in use

	struct spi_device *spi;				/// our spi device
	struct input_dev *input;			/// input device
	struct workqueue_struct *elan_wq;	/// normal function work thread
	struct work_struct work;			/// normal function work queue
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	struct miscdevice firmware;			/// char device for ioctl and IAP
	struct hrtimer timer;				/// use timer if use_irq == 0
	struct work_struct pollingwork;		///	regular polling work thread
	struct elan_polling polling;		///	regular polling work queue
	int (*setup)(struct device *);		/// hw initialize

	struct mutex mutex;		/// Protects I2C accesses to device
	struct mutex tr_mutex;	/// Protects I2C tx/rx

	unsigned long busy;		/// Lock openers
	unsigned int protocol;		/// Protocol stats for firmware

	/* fifo and processing */
	struct kfifo fifo;		/* FIFO queue for data */
	struct mutex fifo_mutex;	/* Serialize operations around FIFO */
	wait_queue_head_t wait;
	spinlock_t rx_kfifo_lock;


	/*! Add for TS driver debug */
	unsigned int status;
	long int irq_received;
	long int packet_received;
	long int packet_fail;
	long int touched_sync;
	long int no_touched_sync;
	long int touch_btn_sync;
	long int no_touch_btn_sync;
	long int checksum_correct;
	u16 checksum_fail;
	u16 header_fail;
	u16	mq_header_fail;
	u16 drop_frame;
	u8  has_button;
	u16 hello_packet;

	/* Arima Customer command */
	elan_abs_type finger[FINGER_NUM];
	elan_abs_type finger_lcd[FINGER_NUM];

};


//=============================================================================
//  Function prototype
//=============================================================================
static int elan_ts_set_data(struct spi_device *spi, const u8 *data, size_t len);
static int elan_touch_hw_reset(struct spi_device *spi);

static int elan_ts_acquire_data(struct spi_device *spi, const uint8_t *cmd,
                                uint8_t *buf, size_t size);
static int elan_ts_poll(struct spi_device *spi);

static void elan_ts_drop_packet(struct elan_data *ts);
static int elan_touch_pull_frame(struct elan_data *ts, u8 *buf);
static int elan_ts_suspend(struct spi_device *spi, pm_message_t mesg);
static int elan_ts_resume(struct spi_device *spi);
uint32_t hw_revision_read (void);

#ifdef ON_PLATFORM
static void touch_panel_callback( u8 data[] );
#endif


#ifdef CONFIG_HAS_EARLYSUSPEND
static void elan_ts_early_suspend(struct early_suspend *h);
static void elan_ts_late_resume(struct early_suspend *h);
#endif


//=============================================================================
//  Global variable
//=============================================================================

/*! for elan-iap char driver */
static struct miscdevice *private_ts;

#ifdef ON_PLATFORM
static unsigned char 			g_diagtype = 0;
static void (*gp_tp_cmd_callback)(void *) = NULL;
static touch_diag_result g_touch_result;
#endif

static uint32_t elan_spi_hw_rev = 0;

//=============================================================================
//  Function implement
//=============================================================================

static void elan_spi_cs_valid(struct elan_data *ts, const char *label)
{
	int count;

	if (!label) {
		label = "no label";
	}
	count = atomic_inc_return(&ts->cs_valid_count);
	if (count == 1) {
		gpio_set_value(ts->cs_gpio, 0);
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&ts->spi->dev, "[ELAN] %s(%s): cs=valid(0)\n", __func__, label);
#endif
		udelay(5);
	} else {
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&ts->spi->dev, "[ELAN] %s(%s): cs_valid_count=%d\n", __func__, label, count);
#endif
	}
}

static void elan_spi_cs_invalid(struct elan_data *ts, const char *label)
{
	int count;

	if (!label) {
		label = "no label";
	}
	count = atomic_dec_return(&ts->cs_valid_count);
	if (count == 0) {
		udelay(5);
		gpio_set_value(ts->cs_gpio, 1);
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&ts->spi->dev, "[ELAN] %s(%s): cs=invalid(1)\n", __func__, label);
#endif
	} else {
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&ts->spi->dev, "[ELAN] %s(%s): cs_valid_count=%d\n", __func__, label, count);
#endif
	}
}

/**
 *	elan_spi_write_data	-	SPI write wrapper
 *	@spi: spi device
 *	@data: data to write
 *	@len: length of block
 *	@what: what are we doing
 *
 *	@return:  It returns zero on success, else a negative error code.
 *
 *	Wrap the spi_write_data method as the ELAN needs a 10uS delay between
 *	SPI data bytes
 */
static int elan_spi_write_data(struct spi_device *spi, const u8 *data,
                               size_t len, const char *what)
{
	int rc = -1;
	int i;

	for (i=0; i<len; i++) {
		if (spi_write(spi, data++, 1) < 0) {
			dev_err(&spi->dev, "spi_write failed: %s\n", what);
			return rc;
		}
		udelay(60);
	}

	rc = len;

	return rc;
}

static int elan_spi_read_data(struct spi_device *spi,  u8 *data, size_t len,
                              const char *what)
{
	int rc = -1;
	int i;

	for (i=0; i<len; i++) {
		if (spi_read(spi, data++, 1) < 0) {
			dev_err(&spi->dev, "spi_read failed: %s\n", what);
			return rc;
		}
		udelay(60);
	}
	rc = len;

	return rc;
}


/**
 *	@brief \b elan_ts_recv_data		-	received TP data
 *	@param spi: our spi device
 *	@param buf : buffer for put received data
 *
 *	received data from TP device.
 */
static int elan_ts_recv_data(struct spi_device *spi, uint8_t *buf)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int rc;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	/* Protect against parallel spi activity. Right now this isn't
	   strictly needed but if we add config features it will become
	   relevant */
	mutex_lock(&ts->tr_mutex);
	rc = elan_spi_read_data(spi, buf, ts->rx_size,
	                        "elan_touch_recv_data");
	if (rc < 0)
		dev_err(&spi->dev, "recv_data: wrong data %d\n", rc);

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] %x:%x:%x:%x:%x:%x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
#endif
	mutex_unlock(&ts->tr_mutex);
	return rc;
}


/**
*	@brief \b elan_iap_open	- char device
*
*	purpose for ioctl and iap updating fw.
*/
int elan_iap_open(struct inode *inode, struct file *filp)
{
	struct elan_data *ts ;

	filp->private_data = private_ts;
	ts = container_of(filp->private_data, struct elan_data, firmware);
#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	if (!(ts->status & STA_INIT5)) {
		dev_err(&ts->spi->dev, "[ELAN] TP device not probe done \n");
		return -EAGAIN;
	}

	return 0;
}

/**
*	@brief  \b elan_iap_release	- char device
*
*	purpose for close this device
*/
int elan_iap_release(struct inode *inode, struct file *filp)
{
#ifdef ELAN_DEBUG_PRINT
	struct elan_data *ts = container_of(filp->private_data,
	                                    struct elan_data, firmware);
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	return 0;
}

/**
*	@brief  \b elan_iap_write	- iap write page data
*	@param buff is page data from user space
*	@param count is number of data in byte.
*
*	purpose for iap write page data
*/
ssize_t elan_iap_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	struct elan_data *ts = container_of(filp->private_data,
	                                    struct elan_data, firmware);
	int rc;
	u8	txbuf[256];

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] %s count=%d\n", __func__, count);
#endif

	if (count > 256)
		return -EMSGSIZE;

	if (copy_from_user(txbuf, buff, count)) {
		return -EFAULT;
	}

	mutex_lock(&ts->mutex);

	elan_spi_cs_valid(ts, __func__);
	rc = elan_spi_write_data(ts->spi, txbuf, count,
	                         "iap_write_cmd");
	elan_spi_cs_invalid(ts, __func__);

	mutex_unlock(&ts->mutex);

	return rc;
}

/**
*	@brief  \b elan_iap_read	- read status code from TP
*
*	purpose for iap read status code from TP
*/
ssize_t elan_iap_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	struct elan_data *ts = container_of(filp->private_data,
	                                    struct elan_data, firmware);
	u8	rxbuf[256];
	int rc;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] %s count=%d\n", __func__, count);
#endif

	if (count > 256)
		return -EMSGSIZE;

	mutex_lock(&ts->mutex);
	elan_spi_cs_valid(ts, __func__);
	rc = elan_spi_read_data(ts->spi, rxbuf, count,
	                        "elan_iap_read");
	elan_spi_cs_invalid(ts, __func__);
	mutex_unlock(&ts->mutex);

	if (rc == 0)
		rc = count;

	if(copy_to_user(buff, rxbuf, count))
		return -EFAULT;

	return rc;
}

/**
*	@brief  \b elan_iap_ioctl	- ioctl
*	@param cmd to control our TP device.
*	@param arg is parameter from user space
*
*	purpose  is that control our TP by char device node.
*/
static long elan_iap_ioctl(struct file *filp, unsigned int cmd,
                           unsigned long arg)
{
	struct elan_data *ts = container_of(filp->private_data,
	                                    struct elan_data, firmware);
	int __user *argp = (int __user *)arg;
	u8 len, buf[4];

	/*! @brief not support during sleep mode */
	if (ts->power_state == PWR_STATE_SLEEP && cmd != IOCTL_PS_WAKE_UP) {
		dev_err(&ts->spi->dev, "[ELAN] power_state in SLEEP\n");
		return -1;
	}

	switch (cmd) {
	case IOCTL_MAJOR_FW_VER:
		put_user(ts->major_fw_version, argp);
		break;
	case IOCTL_MINOR_FW_VER:
		put_user(ts->minor_fw_version, argp);
		break;
	case IOCTL_MAJOR_HW_ID:
		put_user(ts->major_hw_id, argp);
		break;
	case IOCTL_MINOR_HW_ID:
		put_user(ts->minor_hw_id, argp);
		break;
	case  IOCTL_IAP_ENABLE: {
		dev_dbg(&ts->spi->dev, "IOCTL_IAP_ENABLE\n");
		disable_irq(ts->spi->irq);
		cancel_work_sync(&ts->work);
		ts->protocol |= PRO_UPDATE_FW_MODE;
	}
	break;
	case  IOCTL_IAP_DISABLE: {
		enable_irq(ts->spi->irq);
		ts->protocol &= ~PRO_UPDATE_FW_MODE;
	}
	break;
	case IOCTL_RESET:
		elan_touch_hw_reset(ts->spi);
		break;
	case IOCTL_INT:
		put_user(gpio_get_value(ts->intr_gpio), argp);
		break;
	case IOCTL_IAP_MODE:
		return put_user(ts->iap_mode, argp);
	case IOCTL_PS_SLEEP: {
		int ret = 0;
		const char SleepMode[] = {0x54, 0x50, 0x00, 0x01};
		struct spi_device *spi = ts->spi;

		dev_dbg(&spi->dev, "[ELAN] IOCTL_PS_SLEEP\n");
		disable_irq(spi->irq);
		ret = cancel_work_sync(&ts->work);
		if (ret) {
			dev_err(&spi->dev, "[ELAN] Cancel work sync fail, ret=%d\n", ret);
			enable_irq(spi->irq);
			return ret;
		}

		mutex_lock(&ts->mutex);
		ts->power_state = PWR_STATE_SLEEP;
		elan_spi_cs_valid(ts, "IOCTL_PS_SLEEP");
		elan_ts_set_data(ts->spi, SleepMode, sizeof(SleepMode));
		elan_spi_cs_invalid(ts, "IOCTL_PS_SLEEP");
		mutex_unlock(&ts->mutex);
	}
	break;
	case IOCTL_PS_WAKE_UP: {
		int i;
		const char NormalMode[] = {0x54, 0x58, 0x00, 0x01};
		const char CalibCmd[] = {cmd_header_byte_calibration, cmd_header_byte_calibration,
		                         cmd_header_byte_calibration, cmd_header_byte_calibration
		                        };
		char cmd[4], tbuf[4];
		struct spi_device *spi = ts->spi;

		dev_dbg(&spi->dev, "[ELAN] IOCTL_PS_WAKE_UP\n");

		memcpy(cmd, NormalMode, sizeof(NormalMode));
		ts->power_state = PWR_STATE_NORMAL;

		mutex_lock(&ts->mutex);
		elan_spi_cs_valid(ts, "IOCTL_PS_WAKE_UP");
		elan_ts_set_data(ts->spi, cmd, sizeof(cmd));

		/*! @brief once wake-up from sleep states, TP will re-calibration and Host should take care of it */
		if (ts->power_state == PWR_STATE_NORMAL) {
			for (i=0; i<10; i++) {
				if (elan_ts_poll(spi) < 0)
					continue;

				elan_spi_read_data(spi, tbuf, cmd_response_len,
				                   "get_calibration_cmd");
				if (!memcmp(&tbuf[1], &CalibCmd[1], 3))
					break;
			}
			enable_irq(spi->irq);
		}
		elan_spi_cs_invalid(ts, "IOCTL_PS_WAKE_UP");
		mutex_unlock(&ts->mutex);
	}
	break;
	case IOCTL_SET_COMMAND: {
		int tries = 0;
		if ((len = sizeof(unsigned long)) > 4)
			len = 4;
		if (copy_from_user(buf, (const void*)arg, len)) {
			dev_err(&ts->spi->dev,
			        "IOCTL_SET_COMMAND:%x:%x:%x:%x Wrong when copy from user\n",
			        buf[0], buf[1], buf[2], buf[3]);
			return -EFAULT;
		}
		dev_dbg(&ts->spi->dev, "IOCTL_SET_COMMAND:%x:%x:%x:%x\n",
		        buf[0], buf[1], buf[2], buf[3]);
		mutex_lock(&ts->mutex);
		/* intr_gpio is low, means may TP sends finger report. */
		if (gpio_get_value(ts->intr_gpio) == 0) {
			
			while(gpio_get_value(ts->intr_gpio) == 0) {
				usleep(10);
				tries ++;
				if (tries > 300) {
					dev_err(&ts->spi->dev, "[ELAN] intr_gpio is low, %d\n", tries);
					mutex_unlock(&ts->mutex);
					return -EAGAIN;
				}
			}
			udelay(500);
		}		
		elan_spi_cs_valid(ts, "IOCTL_SET_COMMAND");
		elan_ts_set_data(ts->spi, (const u8*)buf, len);
		elan_spi_cs_invalid(ts, "IOCTL_SET_COMMAND");
		mutex_unlock(&ts->mutex);
	}
	break;
	case IOCTL_GET_COMMAND: {
		u8 tbuf[4] = {[0 ... 3] = 0x0};
		int rc = 0, tries = 0;
		struct spi_device *spi = ts->spi;

		if ((len = sizeof(unsigned long)) > 4)
			len = 4;

		if (copy_from_user(buf, (const void*)arg, len)) {
			dev_err(&ts->spi->dev,
			        "IOCTL_GET_COMMAND:%x:%x:%x:%x Wrong when copy from user\n",
			        buf[0], buf[1], buf[2], buf[3]);
			return -EFAULT;
		}

		mutex_lock(&ts->mutex);
		set_bit(idx_sysfs_busy, &ts->busy);
		set_bit(idx_cmd_handshake, &ts->busy);

		/* intr_gpio is low, means may TP sends finger report. */
		if (gpio_get_value(ts->intr_gpio) == 0) {
			dev_err(&ts->spi->dev, "[ELAN] intr_gpio is low\n");
			while(gpio_get_value(ts->intr_gpio) == 0) {
				usleep(10);
				tries ++;
				if (tries > 300) {
					dev_err(&ts->spi->dev, "[ELAN] TIMEOUT = %d\n", tries);
					mutex_unlock(&ts->mutex);
					return -EAGAIN;
				}
			}
			usleep(500);
		}
		elan_spi_cs_valid(ts, "IOCTL_GET_COMMAND");
		elan_ts_set_data(spi, buf, sizeof(buf));
		mutex_unlock(&ts->mutex);

		/* We will wait for non O_NONBLOCK handles until a signal or data */
		mutex_lock(&ts->fifo_mutex);

		while (kfifo_len(&ts->fifo) == 0) {
			mutex_unlock(&ts->fifo_mutex);
			rc = wait_event_interruptible_timeout(
			         ts->wait, kfifo_len(&ts->fifo),msecs_to_jiffies(2000));
			if (rc <= 0) {
				rc = -ETIMEDOUT;
				dev_err(&spi->dev, "timeout!! wake_up(ts->wait)\n");
				goto err2;
			}

			mutex_lock(&ts->fifo_mutex);
		}

		if (elan_touch_pull_frame(ts, tbuf) < 0) {
			rc = -1;
			goto err1;
		}
err1:
		mutex_unlock(&ts->fifo_mutex);
err2:
		elan_spi_cs_invalid(ts, "IOCTL_GET_COMMAND");
		clear_bit(idx_cmd_handshake, &ts->busy);
		clear_bit(idx_sysfs_busy, &ts->busy);
		//mutex_unlock(&ts->mutex);

		dev_dbg(&spi->dev, "[ELAN] Leave %s\n", __func__);

		if (rc < 0) return rc;

		dev_dbg(&ts->spi->dev, "Reponse=%x:%x:%x:%x\n", tbuf[0],
		        tbuf[1], tbuf[2], tbuf[3]);
		if (copy_to_user((void __user *)arg, tbuf, 4))
			return -EFAULT;
	}
	break;
	default:
		break;
	}

	return 0;
}

struct file_operations elan_touch_fops = {
	.owner		= THIS_MODULE,
	.open		= elan_iap_open,
	.write		= elan_iap_write,
	.read		= elan_iap_read,
	.release	= elan_iap_release,
	.unlocked_ioctl		= elan_iap_ioctl,
};


/**
 *	@brief interfaces
 *	provide the hardware and firmware information
 */
static ssize_t show_fw_version_value(struct device *dev,
                                     struct device_attribute *attr, char *buf)
{
	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return -1;

	return sprintf(buf, "%d %d\n",
	               ts->major_fw_version,
	               ts->minor_fw_version);
}

static ssize_t show_hw_id_value(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return -1;

	return sprintf(buf, "%x %x\n",
	               ts->major_hw_id,
	               ts->minor_hw_id);
}



static ssize_t show_bc_version_value(struct device *dev,
                                     struct device_attribute *attr, char *buf)
{
	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);

	return sprintf(buf, "%d %d\n",
	               ts->major_bc_version,
	               ts->minor_bc_version);
}

static ssize_t show_drvver_value(struct device *dev,
                                 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d %d %d\n",
	               DRV_MA_VER, DRV_MI_VER, DRV_SUB_MI_VER);
}


static ssize_t show_intr_gpio(struct device *dev,
                              struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return -1;

	ret = gpio_get_value(ts->intr_gpio);

	return sprintf(buf, "%d\n", ret);
}

static ssize_t show_adapter_pkt_rvd(struct device *dev,
                                    struct device_attribute *attr, char *buf)
{
	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return -1;

	return sprintf(buf, "irq_received=%ld packet_received=%ld packet_fail=%ld checksum_fail=%x header_fail=%d, drop_frame=%d, hello_pkt=%d, has_button=%d\n",
	               ts->irq_received, ts->packet_received, ts->packet_fail,
	               ts->checksum_fail, ts->header_fail, ts->drop_frame, ts->hello_packet, ts->has_button);
}

static ssize_t store_power_save(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf, size_t count)
{
	int ret = 0, i;
	unsigned long val;
	const char NormalMode[] = {0x54, 0x58, 0x00, 0x01};
	const char SleepMode[] = {0x54, 0x50, 0x00, 0x01};
	const char CalibCmd[] = {cmd_header_byte_calibration, cmd_header_byte_calibration,
	                         cmd_header_byte_calibration, cmd_header_byte_calibration
	                        };
	char cmd[4], tbuf[4];
	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return -1;

	ret = strict_strtoul(buf, 10, &val);
	if (ret)
		return ret;

	switch(val) {
	case PWR_STATE_NORMAL:
		memcpy(cmd, NormalMode, sizeof(NormalMode));
		ts->power_state = PWR_STATE_NORMAL;
		break;
	case PWR_STATE_SLEEP:
		disable_irq(spi->irq);
		ret = cancel_work_sync(&ts->work);
		if (ret)
			enable_irq(spi->irq);
		memcpy(cmd, SleepMode, sizeof(SleepMode));
		ts->power_state = PWR_STATE_SLEEP;
		break;
	default:
		return -1;
	}

	mutex_lock(&ts->mutex);
	elan_spi_cs_valid(ts, __func__);
	elan_ts_set_data(ts->spi, cmd, sizeof(cmd));

	/*! @brief once wake-up from sleep states, TP will re-calibration and Host should take care of it */
	if (val == PWR_STATE_NORMAL) {
		for (i=0; i<5; i++) {
			if (elan_ts_poll(spi) < 0)
				continue;

			elan_spi_read_data(spi, tbuf, cmd_response_len,
			                   "get_calibration_cmd");
			if (!memcmp(&tbuf[1], &CalibCmd[1], 3))
				break;
		}
		elan_spi_cs_invalid(ts, __func__);
		enable_irq(spi->irq);
	} else {
		elan_spi_cs_invalid(ts, __func__);
	}
	mutex_unlock(&ts->mutex);

	if (ret < 0) return ret;
	return count;
}

static ssize_t show_power_save(struct device *dev,
                               struct device_attribute *attr,
                               char *buf)
{
	const char RespCmd[] = {0x53, 0x50, 0x00, 0x01};
	u8 tbuf[4];
	int rc = 0;

	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);

	dev_err(&spi->dev, "[ELAN] Enter %s\n", __func__);

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return -1;

	/*! @brief not support during sleep mode */
	if (ts->power_state == PWR_STATE_SLEEP)
		return -1;

	if (test_bit(idx_sysfs_busy, &ts->busy)) {
		dev_err(&spi->dev, "[ELAN] i'm busy!!!\n");
		return -EBUSY;
	}

	mutex_lock(&ts->mutex);
	set_bit(idx_sysfs_busy, &ts->busy);
	set_bit(idx_cmd_handshake, &ts->busy);

	elan_spi_cs_valid(ts, __func__);
	elan_ts_set_data(spi, RespCmd, sizeof(RespCmd));
	mutex_unlock(&ts->mutex);

	dev_err(&spi->dev, "[ELAN] show_power_save..0\n");
	/* We will wait for non O_NONBLOCK handles until a signal or data */
	mutex_lock(&ts->fifo_mutex);
	dev_err(&spi->dev, "[ELAN] show_power_save..1\n");

	while (kfifo_len(&ts->fifo) == 0) {
		mutex_unlock(&ts->fifo_mutex);
		dev_err(&spi->dev, "[ELAN] show_power_save..2\n");
		rc = wait_event_interruptible_timeout(
		         ts->wait, kfifo_len(&ts->fifo),msecs_to_jiffies(500));
		if (rc <= 0) {
			rc = -ETIMEDOUT;
			goto err2;
		}

		dev_err(&spi->dev, "[ELAN] show_power_save..3\n");
		mutex_lock(&ts->fifo_mutex);
	}
	dev_err(&spi->dev, "[ELAN] show_power_save..4\n");
	if (elan_touch_pull_frame(ts, tbuf) < 0) {
		rc = -1;
		goto err1;
	}

err1:
	mutex_unlock(&ts->fifo_mutex);
err2:
	elan_spi_cs_invalid(ts, __func__);
	clear_bit(idx_cmd_handshake, &ts->busy);
	clear_bit(idx_sysfs_busy, &ts->busy);
	//mutex_unlock(&ts->mutex);

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] Leave %s\n", __func__);
#endif

	if (rc < 0) return rc;

	return sprintf(buf, "%x:%x:%x:%x\n", tbuf[0], tbuf[1], tbuf[2], tbuf[3]);
}

static ssize_t show_report_rate(struct device *dev,
                                struct device_attribute *attr,
                                char *buf)
{
	struct spi_device *spi = container_of(dev, struct spi_device, dev);
	struct elan_data *ts = spi_get_drvdata(spi);
	long int old_frame, new_frame;

	old_frame = ts->packet_received;
	msleep(1000);
	new_frame = ts->packet_received;

	return sprintf(buf, "%ld\n", new_frame - old_frame);
}

static DEVICE_ATTR(hw_version, S_IRUGO, show_hw_id_value, NULL);

static DEVICE_ATTR(power_save, S_IRUGO|S_IWUSR|S_IWGRP, show_power_save, store_power_save);

static DEVICE_ATTR(drv_version, S_IRUGO, show_drvver_value, NULL);

static DEVICE_ATTR(fw_version, S_IRUGO, show_fw_version_value, NULL);

static DEVICE_ATTR(bc_version, S_IRUGO, show_bc_version_value, NULL);

static DEVICE_ATTR(ts_packet, S_IRUGO, show_adapter_pkt_rvd, NULL);

static DEVICE_ATTR(gpio, S_IRUGO, show_intr_gpio, NULL);
static DEVICE_ATTR(report_rate, S_IRUGO, show_report_rate, NULL);



static struct attribute *elan_attributes[] = {
	&dev_attr_power_save.attr,
	&dev_attr_hw_version.attr,
	&dev_attr_fw_version.attr,
	&dev_attr_drv_version.attr,
	&dev_attr_bc_version.attr,
	&dev_attr_gpio.attr,
	&dev_attr_ts_packet.attr,
	&dev_attr_report_rate.attr,

	NULL
};


static struct attribute_group elan_attribute_group = {
	.name	= "elan_ts_sysfs",
	.attrs	= elan_attributes,
};

/**
 *	@brief elan_ts_poll		-	polling intr pin status.
 *	@param spi : our spi device
 *
 *	@retval 0 means intr pin is low,	\n
 *	otherwise is high.
 *
 *	polling intr pin untill low and the maximus wait time is \b 200ms.
 */
static int elan_ts_poll(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int status = 0, retry = 200;

	do {
		status = gpio_get_value(ts->intr_gpio);
		retry--;
		mdelay(1);
	} while (status != 0 && retry > 0);

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "%s: poll interrupt status %s\n",
	        __func__, status == 0 ? "low" : "high");
#endif
	return (status == 0 ? 0 : -ETIMEDOUT);
}

/**
 *	@brief  \b elan_ts_set_data		-	set command to TP.
 *	@param spi : our i2c device
 *	@param data : command or data which will send to TP.
 *	@param len : command length usually.
 *
 *	set command to our TP.
 */
static int elan_ts_set_data(struct spi_device *spi, const u8 *data, size_t len)
{
	struct elan_data *ts = spi_get_drvdata(spi);

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] Enter: %s\n", __func__);
	dev_dbg(&spi->dev,
	        "dump cmd: %02x, %02x, %02x, %02x\n",
	        data[0], data[1], data[2], data[3]);
#endif

	mutex_lock(&ts->tr_mutex);

	if (elan_spi_write_data(spi, data, len, "set_data") < 0) {
		dev_err(&spi->dev,
		        "%s: elan_spi_write_cmd failed\n", __func__);
		return -EINVAL;
	}
	mutex_unlock(&ts->tr_mutex);

	return 0;
}

/**
 *	@brief  \b elan_ts_acquire_data		-	get TP status
 *	@param spi : our spi device
 *	@param cmd : asking command
 *	@param buf : result
 *	@param size : command length usually.
 *
 *	set command type and TP will return its status in buf.
 */
static int elan_ts_acquire_data(struct spi_device *spi, const uint8_t *cmd,
                                uint8_t *buf, size_t size)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int rc = 0;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] Enter: %s, size=%d\n", __func__, size);
#endif

	if (buf == NULL)
		return -EINVAL;

	mutex_lock(&ts->tr_mutex);

	/* Remove queued data before send command. */
	while (gpio_get_value(ts->intr_gpio) == 0) {
		uint8_t temp_buf[1];

		elan_spi_cs_valid(ts, __func__);
		rc = elan_spi_read_data(spi, temp_buf, sizeof(temp_buf),
					"get_acquire_data:remove");
		elan_spi_cs_invalid(ts, __func__);
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&spi->dev, "[ELAN] %s. Remove data. ret=%d\n", __func__, rc);
		if (rc > 0) {
			print_hex_dump(KERN_DEBUG, "[ELAN] remove:", 0, 32, 1,
					(void*)temp_buf, rc, 0);
		}
#endif
		if (rc <= 0) {
			break;
		}
	}

	elan_spi_cs_valid(ts, __func__);
	rc = elan_spi_write_data(spi, cmd, size, "set_acquire_cmd");
	if (rc < 0)
		goto fail;

	if (unlikely(elan_ts_poll(spi) < 0))
		goto fail;
	else {
		if (elan_spi_read_data(spi, buf, size,
		                       "get_acquire_data") != size)
			goto fail;
		elan_spi_cs_invalid(ts, __func__);
		mutex_unlock(&ts->tr_mutex);
	}

	return rc;

fail:
	elan_spi_cs_invalid(ts, __func__);
	mutex_unlock(&ts->tr_mutex);
	return -EINVAL;
}


/**
 *	@brief __hello_packet_handler	-	hadle hello packet.
 *	@param spi : our i2c device
 *
 *	@return { >0 means success,
 *			otherwise fail. }
 *
 *	Normal hello packet is {0x55, 0x55, 0x55, 0x55}
 *	recovery mode hello packet is {0x55, 0x55, 0x80, 0x80}
 */
static int __hello_packet_handler(struct spi_device *spi)
{
	int rc = 0;
	struct elan_data *ts = spi_get_drvdata(spi);
	uint8_t buf_recv[4] = {0};

	rc = elan_ts_poll(spi);
	if (rc < 0) {
		dev_err(&spi->dev, "%s: poll failed!\n", ELAN_TS_NAME);
		return -EINVAL;
	}

	elan_spi_cs_valid(ts, __func__);
	rc = elan_spi_read_data(spi, buf_recv, sizeof(buf_recv),
	                        "elan_touch_init_panel");
	elan_spi_cs_invalid(ts, __func__);
	if (rc < 0)
		return rc;

	dev_err(&spi->dev,
	        "[ELAN] Hello Packet: [0x%.2x  0x%.2x  0x%.2x  0x%.2x ] \n",
	        buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3]);

	if (memcmp(buf_recv, hello_packet, 4)) {
		if (memcmp(buf_recv, hello_packet, 2))

			if (buf_recv[3] & PRO_SPI_WRT_CMD_SYNC)
				ts->protocol = ts->protocol | PRO_SPI_WRT_CMD_SYNC;

		if (buf_recv[3] & PRO_HID_MOD_CHECKSUM)
			ts->protocol = ts->protocol | PRO_HID_MOD_CHECKSUM;

		if (buf_recv[3] & PRO_UPDATE_FW_MODE) {
			ts->protocol = ts->protocol |  PRO_UPDATE_FW_MODE;
			ts->iap_mode = IAP_MODE_ENABLE;

			rc = elan_ts_poll(spi);
			if (rc < 0) {
				dev_err(&spi->dev, "%s:2 poll failed!\n", ELAN_TS_NAME);
				return -EINVAL;
			}

			elan_spi_cs_valid(ts, __func__);
			rc = elan_spi_read_data(spi, buf_recv, sizeof(buf_recv), "get_iap_hello");
			elan_spi_cs_invalid(ts, __func__);
			if (rc < 0)
				return rc;

#ifdef ELAN_DEBUG_PRINT
			dev_dbg(&spi->dev, "iap hello packet: [0x%02x 0x%02x 0x%02x 0x%02x]\n",
			        buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3]);
#endif

			ts->major_hw_id = buf_recv[1];
			ts->minor_hw_id = buf_recv[0];
			ts->major_bc_version = buf_recv[3];
			ts->minor_bc_version = buf_recv[2];
		}
	}

	return rc;
}

static int __fw_packet_handler(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int rc;
	const uint8_t cmd[] = {cmd_header_byte_read, 0x00, 0x00, 0x01};
	uint8_t buf_recv[4] = {0x0};

	/// Command not support in IAP recovery mode
	if (ts->protocol & PRO_UPDATE_FW_MODE) {
		dev_err(&spi->dev, "[ELAN] PRO_UPDATE_FW_MODE in fw_packet\n");
		return 0;
	}

	rc = elan_ts_acquire_data(spi, cmd, buf_recv, 4);
	if (rc < 0)
		return rc;

	dev_err(&spi->dev,
	        "[ELAN] FW Ver. Packet: [0x%.2x  0x%.2x  0x%.2x  0x%.2x ] \n",
	        buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3]);

	if (buf_recv[0] == cmd_header_byte_response) {
		ts->major_fw_version = ((buf_recv[1] & 0x0f) << 4) |
		                       ((buf_recv[2] & 0xf0) >> 4);
		ts->minor_fw_version = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);

#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&spi->dev, "[ELAN] TOUCH MAJOR FW VERSION 0x%02x\n",
		        ts->major_fw_version);
		dev_dbg(&spi->dev, "[ELAN] TOUCH MINOR FW VERSION 0x%02x\n",
		        ts->minor_fw_version);
#endif
	} else {
		ts->major_fw_version = 0xff;
		ts->minor_fw_version = 0xff;
		dev_err(&spi->dev, "[ELAN] Get FW version fail!\n");
	}

	return 0;
}

static int __hw_packet_handler(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int rc;
	const uint8_t cmd[] = {cmd_header_byte_read, 0xF1, 0x00, 0x01};
	uint8_t buf_recv[4] = {0x0};

	/// Command not support in IAP recovery mode
	if (ts->protocol & PRO_UPDATE_FW_MODE) {
		dev_err(&spi->dev, "[ELAN] PRO_UPDATE_FW_MODE in hw_packet\n");
		return 0;
	}

	rc = elan_ts_acquire_data(spi, cmd, buf_recv, 4);
	if (rc < 0)
		return rc;

	dev_err(&spi->dev,
	        "[ELAN] HW Ver. Packet: [0x%.2x  0x%.2x  0x%.2x  0x%.2x ] \n",
	        buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3]);

	if (buf_recv[0] == cmd_header_byte_response) {
		ts->major_hw_id = ((buf_recv[1] & 0x0f) << 4) |
		                  ((buf_recv[2] & 0xf0) >> 4);
		ts->minor_hw_id = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);

#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&spi->dev, "[ELAN] TOUCH MAJOR HW VERSION 0x%02x\n",
		        ts->major_hw_id);
		dev_dbg(&spi->dev, "[ELAN] TOUCH MINOR HW VERSION 0x%02x\n",
		        ts->minor_hw_id);
#endif
	} else {
		ts->major_hw_id = 0xff;
		ts->minor_hw_id = 0xff;
		dev_err(&spi->dev, "[ELAN] Get HW id fail!\n");
	}

	return 0;
}


/**
 *	elan_send_soft_reset	-	request the panel resets
 *	@spi: our panel
 *
 *	Send a reset request. Log any error reported when we try this.
 */
static int elan_send_soft_reset(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int rc = -1;
	static const u8 soft_reset_cmd[] = {
		SOFT_RESET, SOFT_RESET, SOFT_RESET, SOFT_RESET
	};

	elan_spi_cs_valid(ts, __func__);
	if (elan_spi_write_data(spi, soft_reset_cmd, sizeof(soft_reset_cmd),
	                        "soft_reset_cmd") < 0) {
		elan_spi_cs_invalid(ts, __func__);
		dev_err(&spi->dev, "[ELAN] elan_send_soft_reset fail\n");
		return rc;
	}
	elan_spi_cs_invalid(ts, __func__);
	return 0;
}


/**
 *      elan_touch_hw_reset - reset touch panel
 *      @spi: our panel
 *
 *      Trigger the hardware reset pin at each touch panel initial stage.
 *      Caller must hold the mutex
 */
static int elan_touch_hw_reset(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);

	gpio_set_value(ts->rst_gpio, 0);
	msleep(1);
	gpio_set_value(ts->rst_gpio, 1);

	return 0;
}

/**
 *	@brief reset touch panel, and wait for ready.
 *	@param spi : our spi device
 *
 *	reset touch panel by hardware reset pin,
 *	and wait while intr is active(low).
 */
static int elan_hw_reset_and_wait_ready(struct spi_device *spi)
{
	int i, retry, intr;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] Enter: %s\n", __func__);
#endif
	intr = 1;
	for (retry = 0; retry < 3; retry++) {
		if (retry > 0) {
			dev_warn(&spi->dev, "%s: retry touch_hw_reset %d\n", __func__, retry);
		}
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&spi->dev, "[ELAN] %s: touch_hw_reset\n", __func__);
#endif
		elan_touch_hw_reset(spi);
		msleep(50);
		for (i = 0; i < 2; i++) {
			// Wait intr_gpio (timeout=200msec)
			intr = elan_ts_poll(spi);
			if (intr == 0) {
				break;
			}
		}
		if (intr == 0) {
#ifdef ELAN_DEBUG_PRINT
			dev_dbg(&spi->dev, "[ELAN] %s: intr==0\n", __func__);
#endif
			break;
		}
	}
	if (intr != 0) {
		dev_err(&spi->dev, "%s: intr=high after touch_hw_reset\n", __func__);
	}
	return (intr == 0 ? 0 : -ETIMEDOUT);
}

#ifdef ON_PLATFORM
static unsigned char elan_touch_spi_write_cmd( struct spi_device *p_spi, u8 *p_val )
{
	u8				*pdata;
	u8				len,cnt;
	int				rc;
	unsigned char	ret = 0x00;
	struct elan_data *ts = spi_get_drvdata(p_spi);

	printk(KERN_DEBUG "[ELAN]%s: Enter\n", __func__);

	printk(KERN_DEBUG "[ELAN]%s: REQ data(0):%02x\n", __func__,p_val[0]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(1):%02x\n", __func__,p_val[1]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(2):%02x\n", __func__,p_val[2]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(3):%02x\n", __func__,p_val[3]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(4):%02x\n", __func__,p_val[4]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(5):%02x\n", __func__,p_val[5]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(6):%02x\n", __func__,p_val[6]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(7):%02x\n", __func__,p_val[7]); // data len
	printk(KERN_DEBUG "[ELAN]%s: REQ data(8):%02x\n", __func__,p_val[8]); // data

	len   = p_val[7];
	pdata = &p_val[8];

	printk(KERN_DEBUG "[ELAN]%s: write len[%02d]\n", __func__,len);
	for(cnt=0; cnt<len; cnt++)
	{
		printk(KERN_DEBUG "[ELAN]%s: data[%02d]   :%02x\n", __func__,cnt,pdata[cnt]);
	}

	mutex_lock(&ts->mutex);

	elan_spi_cs_valid(ts, __func__);
	rc = elan_ts_set_data( p_spi, pdata, (size_t)len );
	elan_spi_cs_invalid(ts, __func__);

	if ( rc )
	{
		printk(KERN_ERR "[ELAN]%s: Error!\n",__func__);
		ret = 0xFF;
	}

	mutex_unlock(&ts->mutex);

	p_val[7] = ret;

	printk(KERN_DEBUG "[ELAN]%s: RESP data(0):%02x\n", __func__,p_val[0]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(1):%02x\n", __func__,p_val[1]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(2):%02x\n", __func__,p_val[2]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(3):%02x\n", __func__,p_val[3]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(4):%02x\n", __func__,p_val[4]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(5):%02x\n", __func__,p_val[5]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(6):%02x\n", __func__,p_val[6]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(7):%02x\n", __func__,p_val[7]);

	return ret;
}

static unsigned char elan_touch_spi_read_cmd( struct spi_device *p_spi, u8 *p_val )
{
	u8				*pdata;
	u8				wlen,rlen,cnt;
	int				rc;
	unsigned char	ret = 0x00;
	struct elan_data *ts = spi_get_drvdata(p_spi);

	printk(KERN_DEBUG "[ELAN]%s: Enter\n", __func__);

	printk(KERN_DEBUG "[ELAN]%s: REQ data(0):%02x\n", __func__,p_val[0]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(1):%02x\n", __func__,p_val[1]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(2):%02x\n", __func__,p_val[2]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(3):%02x\n", __func__,p_val[3]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(4):%02x\n", __func__,p_val[4]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(5):%02x\n", __func__,p_val[5]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(6):%02x\n", __func__,p_val[6]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(7):%02x\n", __func__,p_val[7]);
	printk(KERN_DEBUG "[ELAN]%s: REQ data(8):%02x\n", __func__,p_val[8]);

	wlen  = p_val[7];
	rlen  = p_val[8];
	pdata = &p_val[9];

	printk(KERN_DEBUG "[ELAN]%s: write len :%02d/read len :%02d\n", __func__,wlen,rlen);


	elan_spi_cs_valid(ts, __func__);
	if( wlen )
	{
		for(cnt=0; cnt<wlen; cnt++)
		{
			printk(KERN_DEBUG "[ELAN]%s: data[%02d]   :%02x\n", __func__,cnt,pdata[cnt]);
		}
		mutex_lock(&ts->mutex);

		rc = elan_ts_set_data( p_spi, pdata, (size_t)wlen );

		if ( rc )
		{
			printk(KERN_ERR "[ELAN]%s: Error!\n",__func__);
			ret = 0xFF;
			goto command_err;
		}
		msleep(100);
	}
	else
	{
		mutex_lock(&ts->mutex);
	}

	pdata = &p_val[8];

	rc = elan_spi_read_data(p_spi, pdata, (size_t)rlen, "read_diag_cmd");

	if (rc < 0) {
		printk(KERN_ERR "[ELAN]%s: Error!\n",__func__);
		ret = 0xFF;
	}
	else
	{
		for( cnt = 0; cnt < rlen ; cnt++ )
		{
			printk(KERN_DEBUG "[ELAN]%s: data[%02d]   :%02x\n", __func__,cnt,pdata[cnt]);
		}

		rlen = rlen + 2;
		p_val[4] = rlen;
		p_val[5] = 00;

	}

command_err:
	elan_spi_cs_invalid(ts, __func__);
	mutex_unlock(&ts->mutex);
	p_val[7] = ret;

	printk(KERN_DEBUG "[ELAN]%s: RESP data(0):%02x\n", __func__,p_val[0]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(1):%02x\n", __func__,p_val[1]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(2):%02x\n", __func__,p_val[2]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(3):%02x\n", __func__,p_val[3]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(4):%02x\n", __func__,p_val[4]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(5):%02x\n", __func__,p_val[5]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(6):%02x\n", __func__,p_val[6]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(7):%02x\n", __func__,p_val[7]);
	printk(KERN_DEBUG "[ELAN]%s: RESP data(8):%02x\n", __func__,p_val[8]);


	printk(KERN_DEBUG "[ELAN]%s: Exit\n", __func__);

	return ret;

}
#endif /* ON_PLATFORM */

/**
 *	@brief elan_open		-	open elan device
 *	@param input : input device
 *
 */
static int elan_open(struct input_dev *input)
{
	struct elan_data *ts = input_get_drvdata(input);
	struct spi_device *spi = ts->spi;

	dev_err(&spi->dev, "[ELAN] Enter %s\n", __func__);
	if (!(ts->status & STA_INIT5)) {
		dev_err(&spi->dev, "[ELAN] status=%x\n", ts->status);
		return -EAGAIN;
	}

	dev_err(&spi->dev, "[ELAN] TS Open Success [%x]\n", ts->status);
	if (kfifo_alloc(&ts->fifo, FIFO_SIZE, GFP_KERNEL) < 0) {
		dev_dbg(&spi->dev, "no fifo space\n");
		return -ENOMEM;
	}

	return 0;
}

/**
 *	@brief elan_close		-	close input device
 *	@param input : input device
 *
 */
static void elan_close(struct input_dev *input)
{
	struct elan_data *ts = input_get_drvdata(input);

	dev_err(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);

	return;
}

static inline int elan_ts_parse_xy(uint8_t *data,
                                   uint16_t *x, uint16_t *y)
{
	*x = *y = 0;

	*x = (data[0] & 0xf0);
	*x <<= 4;
	*x |= data[1];

	*y = (data[0] & 0x0f);
	*y <<= 8;
	*y |= data[2];

	return 0;
}

/**
 *	@brief \b elan_ts_setup		-	initialization process.
 *	@param spi : our i2c client
 *
 *	set our TP up
 *	-# reset
 *	-# hello packet
 *	-# fw version
 *	-# hw version
 *
 */
static int elan_ts_setup(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int rc = 0, tries;

	/*!
	Reset */
	elan_hw_reset_and_wait_ready(spi);
	for (tries = 0; tries < 100; tries++) {
		/*! - elan hello packet init */
		rc = __hello_packet_handler(spi);
		if (rc < 0) {
			dev_dbg(&spi->dev, "[ELAN] Soft Reset...%d\n", tries);
			elan_send_soft_reset(spi);
			msleep(10);
		} else break;
	}

	if (rc < 0)
		return -TS_SETUP_HELLO_ERR;

	dev_dbg(&spi->dev, "__hello_packet_handler ...\n");

	ts->rx_size = IDX_PACKET_SIZE;

	/*! - elan fw version */
	dev_dbg(&spi->dev, "wait before __fw_packet_handler\n");
	mdelay(100);
	rc = __fw_packet_handler(spi);
	if (rc < 0) {
		dev_dbg(&spi->dev, "firmware checking error.\n");
		return -TS_SETUP_FWVER_ERR;
	}

	dev_dbg(&spi->dev, "__fw_packet_handler...\n");


	/*! - elan hw version */
	dev_dbg(&spi->dev, "wait before __hw_packet_handler\n");
	mdelay(100);
	rc = __hw_packet_handler(spi);
	if (rc < 0) {
		dev_dbg(&spi->dev, "hardware checking error.\n");
		return -TS_SETUP_HWVER_ERR;
	}

	dev_dbg(&spi->dev, "__hw_packet_handler...\n");

	return rc;
}



/**
 *	@brief \b elan_touch_parse_fid	-	parse the 10 fid bits
 *	@param data : the input bit stream
 *	@param fid : an array of fid values
 *
 *	Unpack the 5 bits into an array.
 *
 *	FIXME: Review whether we can't just use << operators after making
 *	sure the bits are native endian ?
 */
static inline void elan_touch_parse_fid(u8 *data, u8 *fid)
{
	fid[0] = (data[0] & 0x08);
	fid[1] = (data[0] & 0x10);
	fid[2] = (data[0] & 0x20);
	fid[3] = (data[0] & 0x40);
	fid[4] = (data[0] & 0x80);
}

/**
 *	@brief \b elan_touch_parse_wid	-	parse the 10 wid bits
 *	@param data : the input bit stream
 *	@param wid : an array of width level
 *
 *	Unpack the 10 bits into an array.
 *
 */
static inline void elan_touch_parse_wid(u8 *data, u8 *wid)
{
	wid[0] = (data[0] >> 4);
	wid[1] = (data[0] & 0x0f);
	wid[2] = (data[1] >> 4);
	wid[3] = (data[1] & 0x0f);
	wid[4] = (data[2] >> 4);
	wid[5] = (data[2] & 0x0f);
	wid[6] = (data[3] >> 4);
	wid[7] = (data[3] & 0x0f);
	wid[8] = (data[4] >> 4);
	wid[9] = (data[4] & 0x0f);
}

/**
 *	@brief \b elan_ts_parse_btn	-	parse the BTN bits
 *	@param data : the input bit stream
 *	@param bid : an array of width level
 *
 *	Unpack the 7 btn data into an array.
 *
 */
static inline void elan_ts_parse_btn(u8 *data, u8 *bid)
{
	bid[0] = (data[0] & 0x80);
	bid[1] = (data[0] & 0x40);
	bid[2] = (data[0] & 0x20);
	bid[3] = (data[0] & 0x10);
	bid[4] = (data[0] & 0x08);
	bid[5] = (data[0] & 0x04);
	bid[6] = (data[0] & 0x02);
}

/** @brief \b elan_touch_report_buttons	-	process button bit
 *	@param ts: our touchscreen
 *	@param button_stat : number of button in packet
 *	@param buf : received buffer
 *
 *	Walk the received report and process the button data, extracting
 *	and reporting button id. No locking is needed here as the workqueue
 *	does our threading for us.
 */
static void elan_touch_report_buttons(struct elan_data *ts,
                                      int button_stat , u8 *buf)
{
	int i;
	u8 bid[7];
	static u8 recbid[7] = {0};
	struct input_dev *idev = ts->input;

	elan_ts_parse_btn(&buf[idx_finger_btn], &bid[0]);
	if (button_stat)
		goto report_btn;

	/*! @brief : One button in a report */
	for (i = 0; i < BUTTON_NUM; i++) {
		if (recbid[i]) {
			input_report_key(idev, elan_button_key[i], 0);
			recbid[i] = 0;
			break;
		}
	}
	if (button_stat == 0)
		goto end;

report_btn:
	memset(&recbid, 0x0, sizeof(recbid));
	for (i = 0; i < BUTTON_NUM; i++)
		if (bid[i]) {
			input_report_key(idev, elan_button_key[i], 1);
			recbid[i] = 1;
			break;
		}
end:
	input_mt_sync(ts->input);
}

/** @brief \b elan_touch_report_fingers	-	process finger reports
 *	@param ts: our touchscreen
 *	@param finger_stat : number of fingers in packet
 *	@param buf : received buffer
 *
 *	Walk the received report and process the finger data, extracting
 *	and reporting co-ordinates. No locking is needed here as the workqueue
 *	does our threading for us.
 */
static void elan_touch_report_fingers(struct elan_data *ts,
                                      int finger_stat, u8 *buf)
{
	int i;
	u8 fid[FINGER_NUM];
	u16 x, y;
	struct input_dev *idev = ts->input;
	long work1;

	elan_touch_parse_fid(&buf[idx_finger_state], &fid[0]);
	memset(&ts->finger[0], 0xFF, sizeof(ts->finger));
	memset(&ts->finger_lcd[0], 0xFF, sizeof(ts->finger_lcd));

	for (i = 0; i < FINGER_NUM; i++) {
		if (finger_stat == 0)
			return;

		if (fid[i] == 0)
			continue;

		elan_ts_parse_xy(&buf[2+i*3], &x, &y);

#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&ts->spi->dev, "[ELAN] transfer X=%x, Y=%x\n", x, y);
#endif
		if(elan_spi_hw_rev == 0x01)
		{
			/* Do not report touches above status bar on EP0 */
			if( (x > ELAN_TS_X_MAX) || ( y > ELAN_TS_Y_MAX ))
			{
				printk(KERN_ERR "[ELAN] Discarding Out of Range finger X=%x[MAX_X=%x] , Y=%x[MAX_Y=%x] \n", x,ELAN_TS_X_MAX, y,ELAN_TS_Y_MAX);
				continue;
			}
		}

		ts->finger[i].X = x;
		ts->finger[i].Y = y;


		work1 = ( ELAN_TS_X_MAX - (long)x ) * ( SCREEN_X_MAX - 1 );
		x = (u16)(work1 / (long)ELAN_TS_X_MAX);

		work1 = ( ELAN_TS_Y_MAX - (long)y ) * ( SCREEN_Y_MAX - 1 );
		y = (u16)(work1 / (long)ELAN_TS_Y_MAX);

		ts->finger_lcd[i].X = x;
		ts->finger_lcd[i].Y = y;

		if (x != 0 && y != 0) {
			input_report_abs(idev,
			                 ABS_MT_POSITION_X, x);
			input_report_abs(idev,
			                 ABS_MT_POSITION_Y, y);

			input_report_abs( idev, ABS_MT_PRESSURE, 255 );

			input_report_abs(idev, ABS_MT_TRACKING_ID, i);//FINGER_ID + i);

#ifdef ELAN_DEBUG_PRINT
			dev_dbg(&ts->spi->dev, "report finger..%x:%x\n",
			        x, y);
#endif
			input_mt_sync(idev);
			finger_stat--;
		}
	}
}

/**
 *	elan_touch_pull_frame	-	pull a frame from the fifo
 *	@ed: our elan touch device
 *	@ehr: return buffer for the header
 *	@buf: data buffer
 *
 *	Pulls a frame from the FIFO into the provided ehr and data buffer.
 *	The data buffer must be at least cmd_response_len bytes long.
 */
static int elan_touch_pull_frame(struct elan_data *ts, u8 *buf)
{
#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	WARN_ON(kfifo_out_locked(&ts->fifo, buf,
	                         cmd_response_len, &ts->rx_kfifo_lock) != cmd_response_len);

	return cmd_response_len;
}


/**
 *	elan_touch_fifo_clean_old	-	Make room for new frames
 *	@ed: our elan touch device
 *	@room: space needed
 *
 *	Empty old frames out of the FIFO until we can fit the new one into
 *	the other end.
 */
static void elan_touch_fifo_clean_old(struct elan_data *ts, int room)
{
	u8 buffer[cmd_response_len];
#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	while (kfifo_len(&ts->fifo) + room >= FIFO_SIZE)
		elan_touch_pull_frame(ts, buffer);
}


/** @brief \b elan_ts_report_data	-	report finger report to user space.
 *	@param spi : our i2c device
 *	@param buf : raw data from TP device.
 *
 *	- reporting finger data to user space.
 *	- packet_fail count
 *
 */
static void elan_ts_report_data(struct spi_device *spi, uint8_t *buf)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	u8 finger_stat;
	u8 button_stat;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	switch (buf[idx_finger_header]) {
	case cmd_header_byte_hello: {
		const u8 hello_packet[] = {cmd_header_byte_hello, cmd_header_byte_hello,
		                           cmd_header_byte_hello, cmd_header_byte_hello
		                          };
		if (!memcmp(buf, hello_packet, 4))
			ts->hello_packet++;
	}
	break;
	case cmd_header_byte_response:
		/* Queue the data, using the fifo lock to serialize the multiple
		   accesses to the FIFO */
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&spi->dev, "[ELAN] recv cmd_header_byte_response\n");
#endif
		mutex_lock(&ts->fifo_mutex);
		if (kfifo_len(&ts->fifo) + cmd_response_len >= FIFO_SIZE)
			/* Make room, make room */
			elan_touch_fifo_clean_old(ts, cmd_response_len);
		/* Push the data */
		kfifo_in_locked(&ts->fifo, buf, cmd_response_len, &ts->rx_kfifo_lock);
		mutex_unlock(&ts->fifo_mutex);
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&spi->dev, "[ELAN] wake_up [%02x:%02x:%02x:%02x]\n",
		        buf[0], buf[1], buf[2], buf[3]);
#endif
		wake_up(&ts->wait);
		break;
	case idx_coordinate_packet_5_finger: {
		/// - Protect that incorrect TP data
		if (cmd_response_len > idx_finger_total) {
			finger_stat = buf[idx_finger_total] & 0x07;
			if (finger_stat > FINGER_NUM) {
				dev_err(&spi->dev,
				        "[ELAN] %s: illegal finger_stat: %d", __func__, finger_stat);
#ifdef ELAN_DEBUG_PRINT
				print_hex_dump(KERN_DEBUG, "[ELAN] finger:", 0, 32, 1, (void*)buf, cmd_response_len, 0);
#endif
				goto report_done;
			}
		}
		/// - Get finger report data
		ts->rx_size = IDX_PACKET_SIZE - cmd_response_len;
		mutex_lock(&ts->mutex);
		if (elan_ts_recv_data(ts->spi, &buf[cmd_response_len]) < 0)
			break;
		mutex_unlock(&ts->mutex);

		finger_stat = buf[idx_finger_total] & 0x07;
		button_stat = buf[idx_finger_btn] & 0xfe;
#ifdef ELAN_DEBUG_PRINT
		dev_dbg(&spi->dev, "[ELAN] finger_stat == %d\n", finger_stat);
		dev_dbg(&spi->dev, "[ELAN] button_stat == %d\n", button_stat);
		dev_dbg(&spi->dev, "[ELAN] finger:%x:%x:%x:%x:%x:%x\n",
		        buf[0], buf[1], buf[2], buf[3], buf[4], buf[17]);
#endif

		/*! @brief : protect that incorrect TP data */
		/*! @TODO : protect not enough, should be more strong and more assert */
		if (finger_stat > FINGER_NUM)
			goto report_done;

		/* Enter right process, reset int_status*/
		ts->polling.int_status = 0;
		ts->polling.button_status = 0;
		ts->packet_received++;

		/*! finger report process */
		if (likely(finger_stat != 0)) {
			elan_touch_report_fingers(ts, finger_stat, buf);
#ifdef ON_PLATFORM
			touch_panel_callback(NULL);
#endif
			ts->touched_sync++;
		} else {
			memset(&ts->finger[0], 0xFF, sizeof(ts->finger));
			memset(&ts->finger_lcd[0], 0xFF, sizeof(ts->finger_lcd));

			input_mt_sync(ts->input);
			ts->no_touched_sync++;
		}
		input_sync(ts->input);

		/*! button events process */
		if (button_stat == 0 && ts->has_button == 0)
			goto report_done;
		if(button_stat != 0) {
#ifdef ELAN_DEBUG_PRINT
			dev_dbg(&spi->dev, "[ELAN] button_START !!!\n");
#endif
			ts->has_button = 1;
			elan_touch_report_buttons(ts, button_stat, buf);
			input_sync(ts->input);
		} else if (ts->has_button == 1) {
#ifdef ELAN_DEBUG_PRINT
			dev_dbg(&spi->dev, "[ELAN] button_STOP !!!\n");
#endif
			ts->has_button = 0;
			elan_touch_report_buttons(ts, 0, buf);
			input_sync(ts->input);
		}
report_done:
		break;
	}
	default:
		ts->packet_fail++;
		dev_err(&spi->dev,
		        "[ELAN] %s: unknown packet type: %x:%x:%x:%x\n", __func__, buf[0],
		        buf[1], buf[2], buf[3]);
		break;
	}

	return;
}

/** @brief \b elan_ts_work_func	-	handle ISR work.
 *	@param work : pass from kernel.
 *
 *	Handle Interrupt event and report to user space if data is valid.
 *
 */
static void elan_ts_work_func(struct work_struct *work)
{
	struct elan_data *ts =
	    container_of(work, struct elan_data, work);
	uint8_t buf[IDX_PACKET_SIZE] = {0x0};

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return;

	mutex_lock(&ts->mutex);

	/// - this means that we have already serviced it or glich happen!!
	if (gpio_get_value(ts->intr_gpio) != 0)
		goto fail_gpio;

	set_bit(idx_finger_report, &ts->busy);

	/// - Get finger report data
	ts->rx_size = cmd_response_len;
	elan_spi_cs_valid(ts, __func__);
	if (elan_ts_recv_data(ts->spi, buf) < 0)
		goto fail;

	clear_bit(idx_finger_report, &ts->busy);

	mutex_unlock(&ts->mutex);

	/// - parsing report and send out
	elan_ts_report_data(ts->spi, buf);
	elan_spi_cs_invalid(ts, __func__);
	return;

fail:
	elan_spi_cs_invalid(ts, __func__);
	clear_bit(idx_finger_report, &ts->busy);
fail_gpio:
	mutex_unlock(&ts->mutex);

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Leave %s(Fail)\n", __func__);
#endif
	return;
}

/** @brief \b elan_ts_drop_packet	-	err handler that drop all packet.
 *
 *	mutex protection will at outside this function.
 *
 */
static void elan_ts_drop_packet(struct elan_data *ts)
{
	uint8_t buf[IDX_PACKET_SIZE] = {0x0};
	int i;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return;

	if (gpio_get_value(ts->intr_gpio) != 0)
		goto button_check;

	if (ts->polling.int_status > 3)
		ts->polling.int_status = 0;
	else goto button_check;

	dev_err(&ts->spi->dev, "[ELAN] finger polling check\n");

	/// - Get finger report data
	elan_spi_cs_valid(ts, __func__);
	if (elan_ts_recv_data(ts->spi, buf) < 0) {
		elan_spi_cs_invalid(ts, __func__);
		goto end;
	}

	/// - parsing report and send out
	elan_ts_report_data(ts->spi, buf);
	elan_spi_cs_invalid(ts, __func__);
	udelay(10);
	if (gpio_get_value(ts->intr_gpio)!=0)
		goto end;

	dev_err(&ts->spi->dev, "[ELAN] %s : Receive rx_size=1 mode \n", __func__);

	/* -#received all packet till ts->intr pull high */
	ts->rx_size = 1;
	elan_spi_cs_valid(ts, __func__);
	while(gpio_get_value(ts->intr_gpio)==0) {
		elan_ts_recv_data(ts->spi, buf);
		udelay(10);
	}
	elan_spi_cs_invalid(ts, __func__);
	ts->rx_size = IDX_PACKET_SIZE;

button_check:
	if (ts->polling.button_status > 3)
		ts->polling.button_status = 0;
	else goto end;

	dev_err(&ts->spi->dev, "[ELAN] button polling check\n");

	if (ts->has_button == 1) {
		for (i = 0; i < BUTTON_NUM; i++)
			input_report_key(ts->input, elan_button_key[i], 0);
		ts->has_button = 0;
	}

end:
#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Leave %s\n", __func__);
#endif
	return;
}



/** @brief \b elan_ts_pollWork_func	-	regular processing
 *	@param work : pass from kernel
 *
 *	poll_timer polled processing to occur elan_ts_pollWork_func() to check if our ts
 *	is abnormal.
 *
 */
static void elan_ts_pollWork_func(struct work_struct *work)

{
	struct elan_data *ts =
	    container_of(work, struct elan_data, pollingwork);

#ifdef ELAN_DEBUG_PRINT
	dev_err(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		goto fail;

	if (likely(gpio_get_value(ts->intr_gpio) != 0))
		goto button_check;
	else {
		ts->polling.int_status++;
		if (ts->polling.int_status < 3) goto fail;
	}
button_check:
	if (likely(ts->has_button == 0))
		goto fail;
	else {
		ts->polling.button_status++;
		if (ts->polling.int_status < 3) goto fail;
	}

	/* - we use mutex_trylock() here since polling is not very important */
	if (!mutex_trylock(&ts->mutex)) {
		dev_dbg(&ts->spi->dev, "[ELAN] trylock fail! return...\n");
		goto fail;
	}

	if (test_bit(idx_finger_report, &ts->busy)) {
		dev_dbg(&ts->spi->dev, "[ELAN] 1.finger_Report processing ... ignore!!\n");
		goto unlock;
	}

	if (test_bit(idx_cmd_handshake, &ts->busy)) {
		dev_dbg(&ts->spi->dev, "[ELAN] 2.command processing ... ignore!!\n");
		goto unlock;
	}

	dev_err(&ts->spi->dev, "[ELAN] Force to release intr_gpio!!\n");

	ts->drop_frame++;
	elan_ts_drop_packet(ts);

unlock:
	mutex_unlock(&ts->mutex);

fail:
	return;
}


/**
 *	@brief \b elan_touch_timer_func	-	poll processing
 *	@param timer : our timer
 *
 *	Queue polled processing to occur on our touch panel and kick the timer
 *	off again
 *
 *	CHECK: we have no guarantee that the timer will not run multiple times
 *	within one poll, does it matter ?
 */
static enum hrtimer_restart elan_touch_timer_func(struct hrtimer *timer)
{
	struct elan_data *ts = container_of(timer, struct elan_data, timer);
	queue_work(ts->elan_wq, &ts->work);
	hrtimer_start(&ts->timer,
	              ktime_set(0, 12500000), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

/**
 *	@brief \b elan_ts_poll_timer_func	-	err handler when intr_gpio is low but no isr serviced it.
 *	@param data : our ts
 *
 *	intr_gpio polling checking, it'll force to get data if intr_gpio is low
 *	and not in isr routine.
 *
 */
static void elan_ts_poll_timer_func(unsigned long data)
{
	struct elan_data *ts = (struct elan_data *)data;
	struct elan_polling *timer = &ts->polling;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] Enter %s\n", __func__);
#endif

	if (ts->protocol & PRO_UPDATE_FW_MODE) {
		timer->timer.expires = jiffies + 60 * HZ;
		add_timer(&ts->polling.timer);
		return;
	}

	/* - ignore it if normal work is processing */
	if(work_pending(&ts->work)) {
		dev_dbg(&ts->spi->dev, "[ELAN] 1.work_pending ... ignore!!\n");
		goto reset;
	}

	/* - ignore it if poll_timer work is processing */
	if(work_pending(&ts->pollingwork)) {
		dev_dbg(&ts->spi->dev, "[ELAN] 2.work_pending ... ignore!!\n");
		goto reset;
	}

	if(!queue_work(timer->elan_wq, &ts->pollingwork)) {
		dev_err(&ts->spi->dev, "[ELAN] pollWork active failed!!\n");
	}

reset:
	timer->timer.expires = jiffies + 10 * HZ;
	add_timer(&ts->polling.timer);

	return;
}

static irqreturn_t elan_ts_irq_handler(int irq, void *dev_id)
{
	struct elan_data *ts = dev_id;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "[ELAN] %s\n", __func__);
#endif

	ts->irq_received++;
	queue_work(ts->elan_wq, &ts->work);

	return IRQ_HANDLED;
}

static int elan_ts_register_interrupt(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	int err=0;

	if (spi->irq) {

		ts->use_irq = 1;

		err = request_irq(spi->irq, elan_ts_irq_handler,
		                  IRQF_TRIGGER_FALLING | IRQF_DISABLED, ELAN_TS_NAME, ts);
		if (err)
			dev_err(&spi->dev, "%s: request_irq %d failed\n",
			        __func__, spi->irq);

		ts->status |= STA_USE_IRQ;

		dev_dbg(&spi->dev, "[ELAN] %s in interrupt mode\n", ts->input->name);
	}

	if (!ts->use_irq) {
		hrtimer_init(&ts->timer,
		             CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = elan_touch_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0),
		              HRTIMER_MODE_REL);
		dev_dbg(&spi->dev, "[ELAN] %s in time-polling mode\n", ts->input->name);
	}

	return err;
}


/**
 *	elan_touch_probe_thread	-	init touch device.
 *	@spi: our SPI device
 *
 *	Perform real probe for our SPI device and if successful configure
 *	it up as an input device. If not then clean up and return an error
 *	code.
 */
static void probe_thread_func(struct work_struct *work)
{
	struct elan_data *ts = container_of(work, struct elan_data, work);
	struct spi_device *spi = ts->spi;
	int err;
	int retry;

	mutex_lock(&ts->mutex);

	for (retry = 0; retry <= 5; retry++) {
		if (retry > 0) {
			dev_warn(&spi->dev, "[ELAN] %s setup retry%d\n", ELAN_TS_NAME, retry);
		}
		err = elan_ts_setup(spi);
		if (err >= 0) {
			break;
		}
	}
	if (err < 0) {
		dev_err(&spi->dev, "[ELAN] %s probe failed\n", ELAN_TS_NAME);
		goto fail_un;
	}

	PREPARE_WORK(&ts->work, elan_ts_work_func);
	if (elan_ts_register_interrupt(ts->spi) < 0) {
		dev_err(&spi->dev, "[ELAN] %s register_interrupt failed!\n", ELAN_TS_NAME);
		goto fail_un;
	}
	ts->status |= STA_INIT5;

	mutex_unlock(&ts->mutex);

	return;
fail_un:
	mutex_unlock(&ts->mutex);
	return ;

}

/**
 *	@brief elan_ts_probe	-	probe for touchpad
 *	@param spi: our SPI device
 *
 *	Perform setup and probe for our SPI device and if successful configure
 *	it up as an input device. If not then clean up and return an error
 *	code.
 */
static int __devinit elan_ts_probe(struct spi_device *spi)
{
	int i, err = 0;
	struct elan_data *ts = kzalloc(sizeof(struct elan_data), GFP_KERNEL);
	struct elan_spi_platform_data *pdata = NULL;

	if (ts == NULL)
		return -ENOMEM;

	ts->status |= STA_PROBED;

	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_3;

	dev_dbg(&spi->dev, "spi->bits_per_word= %d\n", spi->bits_per_word);
	dev_dbg(&spi->dev, "spi->mode= %d\n", spi->mode);
	dev_dbg(&spi->dev, "spi->max_speed_hz= %d\n", spi->max_speed_hz);
	dev_dbg(&spi->dev, "spi->master->bus_num= %d\n", spi->master->bus_num);
	dev_dbg(&spi->dev, "spi->chip_select= %d\n", spi->chip_select);

	ts->spi = spi;
	mutex_init(&ts->mutex);
	mutex_init(&ts->tr_mutex);
	mutex_init(&ts->fifo_mutex);
	init_waitqueue_head(&ts->wait);
	spin_lock_init(&ts->rx_kfifo_lock);

	/* @{ - Normal operatin (Finger report) */
	ts->elan_wq = create_singlethread_workqueue("elan_wq");
	if (!ts->elan_wq) {
		dev_err(&spi->dev, "[ELAN] %s: create workqueue failed\n", ELAN_TS_NAME);
		goto fail_un;
	}

	/*! - Regular polling Process */
	ts->polling.elan_wq = create_singlethread_workqueue("elan_poll_wq");
	if (!ts->polling.elan_wq) {
		dev_err(&spi->dev, "[ELAN] %s: create workqueue failed\n", ELAN_TS_NAME);
		goto fail_un;
	}

	INIT_WORK(&ts->work, probe_thread_func);
	INIT_WORK(&ts->pollingwork, elan_ts_pollWork_func);

	pdata = spi->dev.platform_data;
	if (!pdata) {
		dev_err(&spi->dev, "[ELAN] %s no platform data\n", ELAN_TS_NAME);
		goto fail_un;
	}

	ts->status &= STA_INIT;

	if (pdata->setup)
		ts->setup = pdata->setup;
	else dev_err(&spi->dev, "setup pointer is NULL!!\n");

	ts->intr_gpio = pdata->intr_gpio;
	ts->rst_gpio = pdata->rst_gpio;
	ts->cs_gpio = pdata->cs_gpio;
	atomic_set(&ts->cs_valid_count, 0);
	ts->suspend_state = 0;

	if (ts->setup)
		ts->setup(&spi->dev);
	dev_dbg(&spi->dev, "reset=%d, intr=%d cs=%d\n", ts->rst_gpio, ts->intr_gpio, ts->cs_gpio);

	ts->input = input_allocate_device();
	if (ts->input == NULL) {
		dev_err(&spi->dev, "[ELAN] %s Failed to allocate input device\n", ELAN_TS_NAME);
		goto fail_un;
	}

	mutex_lock(&ts->mutex);
	spi_set_drvdata(spi, ts);
	spi_setup(spi);
	ts->status |= STA_INIT2;
	mutex_unlock(&ts->mutex);

	/* Handshake with touch device */
	queue_work(ts->elan_wq, &ts->work);

	ts->input->name = ELAN_TS_NAME;
	ts->input->open = elan_open;
	ts->input->close = elan_close;

	/*! @brief add for button key by herman start*/
	set_bit(BTN_TOUCH, ts->input->keybit);

	/*! - Multitouch input params setup */
	input_set_abs_params(ts->input, ABS_MT_POSITION_X, 0, SCREEN_X_MAX, 0, 0);
	input_set_abs_params(ts->input, ABS_MT_POSITION_Y, 0, SCREEN_Y_MAX, 0, 0);
	input_set_abs_params(ts->input, ABS_MT_TRACKING_ID, 0, FINGER_NUM, 0, 0);
	input_set_abs_params(ts->input, ABS_MT_PRESSURE,0,255,0,0);

	ts->x_max = ELAN_TS_X_MAX;
	ts->y_max = ELAN_TS_Y_MAX;

	for (i=0; i<BUTTON_NUM; i++)
		set_bit(elan_button_key[i], ts->input->keybit);

	//set_bit(EV_REP, ts->input->evbit);
	set_bit(EV_ABS, ts->input->evbit);
	set_bit(EV_SYN, ts->input->evbit);
	//set_bit(EV_KEY, ts->input->evbit);

	//set_bit(SYN_MT_REPORT, ts->input->evbit);
	//set_bit(ABS_MT_POSITION_X, ts->input->absbit);
	//set_bit(ABS_MT_POSITION_Y, ts->input->absbit);
	//set_bit(ABS_MT_PRESSURE, ts->input->absbit);
	//set_bit(ABS_MT_TRACKING_ID, ts->input->absbit);

	input_set_drvdata(ts->input, ts);

	err = input_register_device(ts->input);
	if (err) {
		dev_err(&spi->dev, "[ELAN] %s unable to register input device\n", ELAN_TS_NAME);
		goto fail_un;
	}


	/*! - Register poll intr_gpio timer */
	init_timer(&ts->polling.timer);
	/*! - check intr_gpio every 10secs */
	ts->polling.timer.expires = jiffies + 10 * HZ;
	ts->polling.timer.function = &elan_ts_poll_timer_func;
	ts->polling.timer.data = (unsigned long)ts;
	add_timer(&ts->polling.timer);

	ts->status |= STA_INIT4;

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1;
	ts->early_suspend.suspend = elan_ts_early_suspend;
	ts->early_suspend.resume = elan_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	/*! - IAP Device Initial */
	private_ts = &ts->firmware;
	ts->firmware.minor = MISC_DYNAMIC_MINOR;
	ts->firmware.name = "elan-iap";
	ts->firmware.fops = &elan_touch_fops;
	ts->firmware.mode = S_IRWXUGO;

	if (unlikely(misc_register(&ts->firmware)< 0))
		dev_err(&spi->dev, "[ELAN] IAP device register failed!!");
	else {
		dev_dbg(&spi->dev, "[ELAN] IAP device register finished!!");
		ts->fw_enabled = 1;
	}

	/*! @warning If the firmware device fails we carry on as it doesn't stop normal
	   usage */

	/* - register sysfs  @} */
	dev_dbg(&spi->dev, "[ELAN] create sysfs!!\n");
	if (sysfs_create_group(&spi->dev.kobj, &elan_attribute_group))
		dev_err(&spi->dev, "[ELAN] sysfs create group error\n");

	ts->status |= STA_INIT3;;

	elan_spi_hw_rev = hw_revision_read();
	printk( KERN_ERR "[%s] hw_revision=0x%x \n", __FUNCTION__, elan_spi_hw_rev );

	return 0;
fail_un:
	mutex_destroy(&ts->mutex);
	mutex_destroy(&ts->tr_mutex);
	mutex_destroy(&ts->fifo_mutex);
	kfree(ts);
	return err;
}

static int elan_ts_remove(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);

	sysfs_remove_group(&spi->dev.kobj, &elan_attribute_group);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif

	if (ts->use_irq)
		free_irq(spi->irq, ts);
	else
		hrtimer_cancel(&ts->timer);

	input_unregister_device(ts->input);

	if (ts->elan_wq)
		destroy_workqueue(ts->elan_wq);

	if (ts->polling.elan_wq)
		destroy_workqueue(ts->polling.elan_wq);


	del_timer(&ts->polling.timer);

	mutex_destroy(&ts->mutex);
	mutex_destroy(&ts->tr_mutex);
	mutex_destroy(&ts->fifo_mutex);

	if (ts->fw_enabled)
		misc_deregister(&ts->firmware);

	kfree(ts);

	return 0;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
#ifdef ON_PLATFORM
static int elan_fw_update( struct spi_device *spi )
{
	int				rc         = 0;
	long			timeout    = 0;
	int				regcnt     = 0;
	unsigned long	flags      = 0;
	u8	force 					= 0;
	struct elan_data	*ts;
	const struct firmware   *p_fw_entry;
	spinlock_t		spin_lock_state;
	u8				buf[4];
	const u8		*fw_data;
	const u8		enter_iap[4] = {IAP_Mode_0, IAP_Mode_1, IAP_Mode_2, IAP_Mode_3};
	const u8		send_id[4] = {IAP_Master, IAP_Slave_1, IAP_Slave_2, IAP_Slave_3};
	u8				fw_name[] = "elan_fw_01.ekt";

	dev_dbg(&spi->dev, "[ELAN] %s: enter\n", __func__);

	ts = spi_get_drvdata( spi );

	spin_lock_init( &spin_lock_state );

	if (ts->protocol & PRO_UPDATE_FW_MODE)
		force = 1;

	spin_lock_irqsave( &spin_lock_state, flags );
	dev_dbg(&ts->spi->dev, "IAP_ENABLE\n");
	ts->protocol |= PRO_UPDATE_FW_MODE;
	spin_unlock_irqrestore( &spin_lock_state, flags );

	disable_irq(ts->spi->irq);
	rc = cancel_work_sync( &ts->work );
	if ( rc ) {
		dev_dbg(&spi->dev, "[ELAN]%s: cancel work rc(%x)\n", __func__, rc);
	}

	dev_err(&ts->spi->dev, "[ELAN] request_firmware name = %s\n", fw_name);
	rc = request_firmware(&p_fw_entry, fw_name, &spi->dev);
	
	if (rc != 0)
	{
		dev_err(&ts->spi->dev, "rc=%d, request_firmware fail\n", rc);
		ts->protocol &= ~PRO_UPDATE_FW_MODE;
		enable_irq(ts->spi->irq);
		return 0x01;
	}
	fw_data = p_fw_entry->data;
	/*! @TODO : magic header verify */
	/* Start IAP Procedure */
	if (!force) {
		elan_spi_cs_valid(ts, __func__);
		elan_ts_set_data(spi, enter_iap, 4);
		elan_spi_cs_invalid(ts, __func__);
		mdelay(40);
	}
FORCE_IAP:
	elan_spi_cs_valid(ts, __func__);
	elan_ts_set_data(spi, &send_id[0], 1);
	elan_spi_cs_invalid(ts, __func__);
	mdelay(55);

	dev_err(&spi->dev, "[ELAN] Start loop...\n");

	for (regcnt=0; regcnt<PageNum; regcnt++) {
PAGE_REWRITE:
		elan_spi_cs_valid(ts, __func__);
		dev_err(&spi->dev, "[ELAN] SPI_WRITE0...%d\n", regcnt);
		rc = elan_spi_write_data(spi,
		                         fw_data + (regcnt*PageSize), PageSize, "write_page_data");
		dev_err(&spi->dev, "[ELAN] SPI_WRITE1...%d\n", regcnt);
		if (rc < 0)
			dev_err(&spi->dev, "[ELAN] IAP Write Page data err!!\n");
		mdelay(45);
		rc = elan_spi_read_data(spi, buf, 2, "Get IAP Ack");
		elan_spi_cs_invalid(ts, __func__);
		if (rc < 0) {
			dev_err(&spi->dev, "[ELAN] IAP Ack data err!!\n");
			goto PAGE_REWRITE;
		} else {
			dev_err(&spi->dev, "[ELAN] SPI_WRITE...%d, ret=%x:%x\n", regcnt, buf[0], buf[1]);
			if (ACK_OK != buf[1]) {
				timeout++;
				dev_err(&spi->dev, "[ELAN] IAP Get Ack Error [%02x:%02x]!!\n", buf[0], buf[1]);
				if (timeout > 30) {
					dev_err(&spi->dev, "[ELAN] IAP Write Page timeout!!\n");
					break;
				}
				dev_err(&spi->dev, "[ELAN] goto PAGE_REWRITE\n");
				goto PAGE_REWRITE;
			}
		}
		mdelay(55);
	}

	dev_err(&spi->dev, "[ELAN] fw update finish..check OK??\n");

	ts->protocol &= ~PRO_UPDATE_FW_MODE;
	if ((rc = elan_ts_setup(spi)) < 0) {
		dev_err(&ts->spi->dev, "[ELAN] TS Setup handshake fail!! (%d)\n", rc);
		
		ts->protocol |= PRO_UPDATE_FW_MODE;
		if ((-TS_SETUP_HELLO_ERR) == rc) {
			dev_err(&ts->spi->dev, "[ELAN] TS Re Burn!!\n");
			goto FORCE_IAP;
		}

		ts->major_fw_version = 0xff;
		ts->minor_fw_version = 0xff;
		ts->major_hw_id = 0xff;
		ts->minor_hw_id = 0xff;

		dev_err(&ts->spi->dev, "[ELAN] IAP Update Failure!!\n");
		rc = 0x01;
		goto err;
	}
	dev_dbg(&ts->spi->dev, "[ELAN] IAP Update Success!!\n");

err:
	release_firmware(p_fw_entry);
	spin_lock_irqsave( &spin_lock_state, flags );
	dev_dbg(&ts->spi->dev, "IAP_DISABLE\n");
	ts->iap_mode = 0;
	ts->protocol &= ~PRO_UPDATE_FW_MODE;
	spin_unlock_irqrestore( &spin_lock_state, flags );
	enable_irq(ts->spi->irq);
	return rc;
}

static int elan_fw_rev( struct spi_device *spi, u8* p_val )
{
	int ret = 0;
	int rc = 0;
	const struct firmware   *p_fw_entry;
	const u8 *fw_data;
	u8 fw_name[] = "elan_fw_01.ekt";

	rc = request_firmware(&p_fw_entry, fw_name, &spi->dev);
	if (rc != 0)
	{
		dev_err(&spi->dev, "rc=%d, request_firmware fail\n", rc);
		return 0xFF;
	}

	fw_data = p_fw_entry->data;

	p_val[0] = (unsigned char)*(fw_data + 0x7bd1);
	p_val[1] = (unsigned char)*(fw_data + 0x7bd0);

	release_firmware(p_fw_entry);

	return ret;
}
#endif

#ifdef ON_PLATFORM

unsigned char touch_panel_cmd( unsigned char type, void *p_val )
{
	struct elan_data *ts ;
	unsigned char	ret    = 0xff; /* err */
	int				finger = 0;
	touch_diag_result *p_result;
	char			*pRev;
	int				rc;

	ts = container_of(private_ts, struct elan_data, firmware);

	dev_dbg(&ts->spi->dev, "[ELAN]%s: Enter type = %d\n", __func__, type);

	p_result = (touch_diag_result*)p_val;

	if ( ts == NULL ) {
		dev_dbg(&ts->spi->dev, "[ELAN]%s: Exit gp_spi = NULL \n", __func__);

		return ret;
	}

	switch ( type ) {
	case TOUCH_PANEL_CMD_TYPE_ENTR_SLEEP:
		ret = elan_ts_suspend(ts->spi, PMSG_SUSPEND);
		break;

	case TOUCH_PANEL_CMD_TYPE_EXIT_SLEEP:
		ret = elan_ts_resume(ts->spi);
		break;

	case TOUCH_PANEL_CMD_TYPE_GET_REVISION:
		dev_dbg(&ts->spi->dev, "[ELAN]TOUCH_PANEL_CMD_TYPE_GET_REVISION\n");

		pRev    = (char *)p_val;

		mutex_lock(&ts->mutex);
		if (ts->suspend_state) {
			dev_dbg(&ts->spi->dev, "[ELAN] suspend => elan_ts_setup\n");
			elan_ts_setup(ts->spi);
		}
		/*! - elan fw version */
		rc = __fw_packet_handler(ts->spi);
		if (rc < 0) {
			dev_dbg(&ts->spi->dev, "firmware checking error.\n");
		}
		if (ts->suspend_state) {
			dev_dbg(&ts->spi->dev, "[ELAN] suspend => rst_gpio=0\n");
			gpio_set_value(ts->rst_gpio, 0);
		}
		mutex_unlock(&ts->mutex);
		pRev[0] = ( ts->minor_fw_version >> 4 ) & 0x0F;
		pRev[1] = ( ts->minor_fw_version ) & 0x0F;
		ret     = 0x00;
		break;

	case TOUCH_PANEL_CMD_TYPE_RESET:
		dev_dbg(&ts->spi->dev, "[ELAN]TOUCH_PANEL_CMD_TYPE_RESET\n");
		ret = elan_send_soft_reset(ts->spi);
		break;

	case TOUCH_PANEL_CMD_TYPE_GET_COORD:
		dev_dbg(&ts->spi->dev, "[ELAN]TOUCH_PANEL_CMD_TYPE_GET_COORD\n");
		if (p_result != NULL ) {
			for (finger=0; finger<FINGER_NUM; finger++) {

				p_result->finger[finger].finger_data_x[0] =
				    (u8)( ts->finger[finger].X >> 8 );
				p_result->finger[finger].finger_data_x[1] =
				    (u8)( ts->finger[finger].X & 0xff );
				p_result->finger[finger].finger_data_y[0] =
				    (u8)( ts->finger[finger].Y >> 8 );
				p_result->finger[finger].finger_data_y[1] =
				    (u8)( ts->finger[finger].Y & 0xff );
			}
		} else {
			dev_err(&ts->spi->dev, "[ELAN] %s: p_result=NULL Error\n", __func__);
			ret = 0xFF;
		}
		break;

	case TOUCH_PANEL_CMD_TYPE_FW_UPDATE:
		dev_dbg(&ts->spi->dev, "[ELAN]TOUCH_PANEL_CMD_TYPE_FW_UPDATE\n");
		mutex_lock(&ts->mutex);
		if (ts->suspend_state) {
			dev_dbg(&ts->spi->dev, "[ELAN] suspend => elan_ts_setup\n");
			elan_ts_setup(ts->spi);
		}
		mutex_unlock(&ts->mutex);
		ret = (char )elan_fw_update( ts->spi );
		mutex_lock(&ts->mutex);
		if (ts->suspend_state) {
			dev_dbg(&ts->spi->dev, "[ELAN] suspend => rst_gpio=0\n");
			gpio_set_value(ts->rst_gpio, 0);
		}
		mutex_unlock(&ts->mutex);
		break;

	case TOUCH_PANEL_CMD_TYPE_WRITE_SPI:
		ret = elan_touch_spi_write_cmd( ts->spi,(u8 *)p_val );
		break;

	case TOUCH_PANEL_CMD_TYPE_READ_SPI:
		ret = elan_touch_spi_read_cmd( ts->spi,(u8 *)p_val );
		break;

	case TOUCH_PANEL_CMD_TYPE_GET_FW_REVISION:
		dev_dbg(&ts->spi->dev, "[ELAN]TOUCH_PANEL_CMD_TYPE_GET_FW_REVISION\n");
		mutex_lock(&ts->mutex);
		if (ts->suspend_state) {
			dev_dbg(&ts->spi->dev, "[ELAN] suspend => elan_ts_setup\n");
			elan_ts_setup(ts->spi);
		}
		mutex_unlock(&ts->mutex);
		ret = elan_fw_rev( ts->spi, (u8*)p_val );
		if (ts->suspend_state) {
			dev_dbg(&ts->spi->dev, "[ELAN] suspend => rst_gpio=0\n");
			gpio_set_value(ts->rst_gpio, 0);
		}
		mutex_unlock(&ts->mutex);
		break;

	default:
		dev_dbg(&ts->spi->dev, "[ELAN]%s: NG type = %d\n", __func__, type);
		break;
	}

	dev_dbg(&ts->spi->dev, "[ELAN]%s: Exit ret = %d\n", __func__, ret);

	return ret;

}
EXPORT_SYMBOL( touch_panel_cmd );


/*-----------------------------------------------------------------------------
 -----------------------------------------------------------------------------*/
unsigned char touch_panel_cmd_callback( unsigned char type, unsigned char val,
                                        void (*func)(void *) )
{
	struct elan_data *ts ;
	unsigned char ret = 0xFF; /* err */
	ts = container_of(private_ts, struct elan_data, firmware);

	printk(KERN_DEBUG "[ELAN]%s: Enter type = %d\n", __func__, type);

	if ( func == NULL ) {
		dev_dbg(&ts->spi->dev, "[ELAN]%s: func = NULL\n", __func__);
		return ret;
	}

	if ( ts == NULL ) {
		dev_dbg(&ts->spi->dev, "[ELAN]%s: Exit gp_spi = NULL \n", __func__);

		return ret;
	}

	switch ( type ) {
	case TOUCH_PANEL_CMD_TYPE_LINE_TEST:
		gp_tp_cmd_callback = func;
		g_diagtype = TOUCH_PANEL_CMD_TYPE_LINE_TEST;
		ret = 0;
		break;

	default:
		dev_dbg(&ts->spi->dev, "[ELAN]%s: NG type = %d\n", __func__, type);
		break;
	}

	dev_dbg(&ts->spi->dev, "[ELAN]%s: Exit ret = %d\n", __func__, ret);

	return ret;

}
EXPORT_SYMBOL(touch_panel_cmd_callback);

static void touch_panel_callback( u8 data[] )
{
	int		finger = 0;
	struct elan_data *ts ;

#define	ELAN_NO_FINGER_DEF 0xFFFFFFFF

	ts = container_of(private_ts, struct elan_data, firmware);

	if ( gp_tp_cmd_callback == NULL ) {
		return;
	}

	if ( ts == NULL ) {
		dev_dbg(&ts->spi->dev, "[ELAN]%s: Exit gp_spi = NULL \n", __func__);
		return ;
	}

	dev_dbg(&ts->spi->dev, "[ELAN]%s: Enter g_diagtype = %d, %d\n",
	        __func__, g_diagtype, sizeof(ts->finger));

	switch ( g_diagtype ) {
	case TOUCH_PANEL_CMD_TYPE_LINE_TEST:
		g_touch_result.ret = 0;

		for ( finger = 0; finger < FINGER_NUM ; finger++ ) {
				g_touch_result.finger[finger].finger_data_x[0] =
				    (u8)( ts->finger_lcd[finger].X >> 8 );
				g_touch_result.finger[finger].finger_data_x[1] =
				    (u8)( ts->finger_lcd[finger].X & 0xFF );
				g_touch_result.finger[finger].finger_data_y[0] =
				    (u8)( ts->finger_lcd[finger].Y >> 8 );
				g_touch_result.finger[finger].finger_data_y[1] =
				    (u8)( ts->finger_lcd[finger].Y& 0xFF );
		}

		break;

	default:
		g_touch_result.ret = 0xFF;
		dev_dbg(&ts->spi->dev, "[ELAN]%s: NG g_diagtype = %d\n",
		        __func__,
		        g_diagtype);
		break;
	}

	gp_tp_cmd_callback( &g_touch_result );
	gp_tp_cmd_callback = NULL;
	g_diagtype = 0;

	dev_dbg(&ts->spi->dev, "[ELAN] Leave %s\n", __func__);

	return ;

}

#endif


/** @brief \b elan_ts_suspend	-	enter sleep mode
 *
 *	when Suspend mode, disable_irq and cancel work queue.
 *
 */
static int elan_ts_suspend(struct spi_device *spi, pm_message_t mesg)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	const u8 set_sleep_cmd[] = {0x54, 0x50, 0x00, 0x01};
	int rc = 0;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] %s: enter\n", __func__);
#endif

	/* Command not support in IAP recovery mode */
	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return 0;

	disable_irq(spi->irq);
	cancel_work_sync(&ts->work);

	mutex_lock(&ts->mutex);
	elan_spi_cs_valid(ts, __func__);
	rc = elan_ts_set_data(spi,
	                      set_sleep_cmd, sizeof(set_sleep_cmd));
	elan_spi_cs_invalid(ts, __func__);

	if (rc < 0)
		goto end;

	ts->power_state = PWR_STATE_SLEEP;
end:
	mutex_unlock(&ts->mutex);

	return rc;
}


/** @brief \b elan_ts_resume	-	enter wake-up mode
 *
 *	when Wake-up mode, enable_irq.
 *
 */
static int elan_ts_resume(struct spi_device *spi)
{
	struct elan_data *ts = spi_get_drvdata(spi);
	const u8 set_active_cmd[] = {0x54, 0x58, 0x00, 0x01};
	char tbuf[4];
	int rc = 0, retry;
	int flag = 0;

#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&spi->dev, "[ELAN] %s: enter\n", __func__);
#endif

	/* Command not support in IAP recovery mode */
	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return 0;

	mutex_lock(&ts->mutex);

	elan_spi_cs_valid(ts, __func__);
	rc = elan_ts_set_data(spi,
	                      set_active_cmd, sizeof(set_active_cmd));

	msleep(100);

	/*! @brief once wake-up from sleep states, TP will re-calibration and Host should take care of it
		Calibration response is [0x66 0x66 0x66 0x66] */
	for (retry=0; retry<5; retry++) {
		if (elan_ts_poll(spi) < 0)
			continue;

		elan_spi_read_data(spi, tbuf, 4,
		                   "get_calibration_cmd");
		if (tbuf[1] == 0x66) {
			flag = 1;
			break;
		}
	}
	elan_spi_cs_invalid(ts, __func__);

	if(flag == 0) {
		printk(KERN_ERR "[ELAN] %s: NO RESPONSE. RESET IC\n", __func__);
		elan_ts_setup(spi);
	}

	ts->power_state = PWR_STATE_NORMAL;
	mutex_unlock(&ts->mutex);

	enable_irq(spi->irq);

	return rc;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
/** @brief \b elan_ts_early_suspend	-	enter sleep mode
 *
 *	This function is called by kernel,
 *	elan_ts_early_suspend() -> elan_ts_suspend()
 *
 */
static void elan_ts_early_suspend(struct early_suspend *h)
{
	struct elan_data *ts =
	    container_of(h, struct elan_data, early_suspend);
#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "Enter %s\n", __func__);
#endif

	/* Command not support in IAP recovery mode */
	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return;

	elan_ts_suspend(ts->spi, PMSG_SUSPEND);

	mutex_lock(&ts->mutex);
	ts->suspend_state = 1;
	mutex_unlock(&ts->mutex);
	
}

/** @brief \b elan_ts_late_resume	-	enter wake-up mode
 *
 *	This function is called by kernel,
 *	elan_ts_late_resume() -> elan_ts_resume()
 *
 */
static void elan_ts_late_resume(struct early_suspend *h)
{
	struct elan_data *ts =
	    container_of(h, struct elan_data, early_suspend);
	int i, retry, intr;
	u8 packet_finger_released[IDX_PACKET_SIZE] = {
		idx_coordinate_packet_5_finger,
	};
	elan_abs_type finger_released;
#ifdef ELAN_DEBUG_PRINT
	dev_dbg(&ts->spi->dev, "Enter %s\n", __func__);
#endif

	/* Command not support in IAP recovery mode */
	if (ts->protocol & PRO_UPDATE_FW_MODE)
		return;

	mutex_lock(&ts->mutex);
	gpio_set_value(ts->rst_gpio, 0);
	msleep(10);
	gpio_set_value(ts->rst_gpio, 1);
	mutex_unlock(&ts->mutex);
	
	/* Send report that finger is released */
	memset(&finger_released, 0xFF, sizeof(finger_released));
	if (memcmp(&finger_released, &ts->finger[0], sizeof(finger_released)) != 0) {
		memset(&ts->finger[0], 0xFF, sizeof(ts->finger));
		memset(&ts->finger_lcd[0], 0xFF, sizeof(ts->finger_lcd));
		input_mt_sync(ts->input);
		ts->no_touched_sync++;
		input_sync(ts->input);
	}
	/* Send report that button is released */
	if (ts->has_button == 1) {
		ts->has_button = 0;
		elan_touch_report_buttons(ts, 0, packet_finger_released);
		input_sync(ts->input);
	}

	mutex_lock(&ts->mutex);
	ts->suspend_state = 0;
	intr = 1;
	for (retry = 0; retry < 3; retry++) {
		if (retry > 0) {
			dev_warn(&ts->spi->dev, "%s: retry touch_hw_reset %d\n", __func__, retry);
		}
		elan_touch_hw_reset(ts->spi);
		msleep(50);
		for (i = 0; i < 2; i++) {
			// Wait intr_gpio (timeout=200msec)
			intr = elan_ts_poll(ts->spi);
			if (intr == 0) {
				break;
			}
		}
		if (intr == 0) {
			break;
		}
	}
	if (intr != 0) {
		dev_err(&ts->spi->dev, "%s: intr=high after touch_hw_reset\n", __func__);
	}
	
	ts->power_state = PWR_STATE_NORMAL;
	mutex_unlock(&ts->mutex);

	enable_irq(ts->spi->irq);
}
#endif

static struct spi_driver elan_touch_driver = {
	.probe		= elan_ts_probe,
	.remove		= elan_ts_remove,
	.driver		= {
		.name = ELAN_TS_NAME,
		.bus    = &spi_bus_type,
		.owner = THIS_MODULE,
	},
};

static int __devinit elan_ts_init(void)
{
	printk(KERN_INFO "[ELAN] %s\n", __func__);
	return spi_register_driver(&elan_touch_driver);
}

static void __exit elan_ts_exit(void)
{
	spi_unregister_driver(&elan_touch_driver);
	return;
}

module_init(elan_ts_init);
module_exit(elan_ts_exit);

MODULE_DESCRIPTION("ELAN Touchscreen Driver");
MODULE_LICENSE("GPL");
