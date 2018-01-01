/* Copyright (C) 2011, NEC CASIO Mobile Communications. All rights reserved.  
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/completion.h>
#include <linux/string.h>

#include <linux/battim.h>
#include <linux/pm_param.h>
#include <linux/mfd/pm8xxx/pm8921-charger.h>

/********************************************************************/
/* Local Data Type Definition                                       */
/********************************************************************/
struct battim_save_data_struct {

	battim_kern_event_reg_type kern_event_reg;
	battim_user_event_reg_type user_event_reg;
	battim_battcal_val_type    battcal_val;
	battim_battcal_hist_type   battcal_hist;
	battim_obs_coll_type       obs_coll;
	battim_calmon_val_type     calmon_val;
	battim_obsmon_val_type     obsmon_val;
	battim_chgmon_val_type     chgmon_val;
	battim_chgprot_val_type    chgprot_val;
};

struct battim_info_struct {

	/* Main information data */
	struct battim_save_data_struct save_data;

	/* initialization flag */
	bool early_init_flag;
	bool init_flag;
	bool cleanup_flag;
	bool pm_param_init_flag;

	/* Mutex */
	struct mutex kern_event_mutex;
	struct mutex battcal_mutex;
	struct mutex obs_coll_mutex;
	struct mutex mon_mutex;
	struct mutex chgprot_mutex;

	/* Workqueue */
	struct work_struct comp_work;

	/* Completion */
	struct completion kern_event_comp;
};

#include "pm_param_default.h"

/********************************************************************/
/* Global Variable Definition                                       */
/********************************************************************/
static struct battim_info_struct battim_info = {
	.early_init_flag = false,
	.init_flag = false,
	.cleanup_flag = false,
	.pm_param_init_flag = false,
};

struct pm_param_struct pm_param;
EXPORT_SYMBOL(pm_param);

static DEFINE_MUTEX(battim_early_init_mutex);

/********************************************************************/
/* Local Function Declaration                                       */
/********************************************************************/
static void battim_early_init(void);
static void battim_sub_reset_kern_event_status(void);
static int battim_sub_read_prior_kern_event(void);
static int battim_get_pm_param(struct pm_param_struct *param);
static int battim_set_pm_param(struct pm_param_struct *param);

/********************************************************************/
/* Function Variable Definition                                     */
/********************************************************************/

static int battim_open(struct inode *inode, struct file *filp)
{
	pr_debug("%s\n", __func__);
	return 0;
}

static int battim_release(struct inode *inode, struct file *filp)
{
	pr_debug("%s\n", __func__);
	return 0;
}

static long battim_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	union {
		unsigned char head_pos;

		battim_event_type        kern_event;
		battim_event_type        user_event;
		battim_battcal_val_type  battcal_val;
		battim_battcal_hist_type battcal_hist;
		battim_obs_coll_type     obs_coll;
		battim_calmon_val_type   calmon_val;
		battim_obsmon_val_type   obsmon_val;
		battim_chgmon_val_type   chgmon_val;
		battim_chgprot_val_type  chgprot_val;
		struct pm_param_struct   pm_param;
	} buf;

	int ret = 0;

	pr_debug("%s: %x %u %u\n", __func__, cmd, _IOC_NR(cmd), _IOC_SIZE(cmd));

	/* Check battim ioctl command */
	if( _IOC_TYPE(cmd) != BATTIM_IOCTL_MAGIC ) {
		ret = -ENOTTY;
		goto out;
	}
	if( _IOC_NR(cmd) >= BIMIOC_MAX ) {
		ret = -ENOTTY;
		goto out;
	}

	/* Get Data from User Area */
	if( _IOC_DIR(cmd) & _IOC_WRITE ) {
		if( copy_from_user(&buf.head_pos, (void *)arg, _IOC_SIZE(cmd)) ) {
			pr_err("%s: copy_from_user failed.\n", __func__);
			ret = -EFAULT;
			goto out;
		}
	}

	/* Call battim ioctl command */
	switch(cmd)
	{
	case IOCTL_BATTIM_GET_KERN_EVENT:
		ret = battim_get_kern_event(&buf.kern_event);
		break;
	case IOCTL_BATTIM_SET_KERN_EVENT:
		ret = -EINVAL;  /* not implemented */
		break;
	case IOCTL_BATTIM_GET_USER_EVENT:
		ret = -EINVAL;  /* not implemented */
		break;
	case IOCTL_BATTIM_SET_USER_EVENT:
		ret = battim_set_user_event(&buf.user_event);
		break;
	case IOCTL_BATTIM_GET_BATTCAL_VAL:
		ret = battim_get_battcal_val(&buf.battcal_val);
		break;
	case IOCTL_BATTIM_SET_BATTCAL_VAL:
		ret = battim_set_battcal_val(&buf.battcal_val);
		break;
	case IOCTL_BATTIM_GET_BATTCAL_HIST:
		ret = battim_get_battcal_hist(&buf.battcal_hist);
		break;
	case IOCTL_BATTIM_SET_BATTCAL_HIST:
		ret = battim_set_battcal_hist(&buf.battcal_hist);
		break;
	case IOCTL_BATTIM_GET_OBS_COLL:
		ret = battim_get_obs_coll(&buf.obs_coll);
		break;
	case IOCTL_BATTIM_SET_OBS_COLL:
		ret = battim_set_obs_coll(&buf.obs_coll);
		break;
	case IOCTL_BATTIM_GET_CALMON_VAL:
		ret = battim_get_calmon_val(&buf.calmon_val);
		break;
	case IOCTL_BATTIM_SET_CALMON_VAL:
		ret = battim_set_calmon_val(&buf.calmon_val);
		break;
	case IOCTL_BATTIM_GET_OBSMON_VAL:
		ret = battim_get_obsmon_val(&buf.obsmon_val);
		break;
	case IOCTL_BATTIM_SET_OBSMON_VAL:
		ret = battim_set_obsmon_val(&buf.obsmon_val);
		break;
	case IOCTL_BATTIM_GET_CHGMON_VAL:
		ret = battim_get_chgmon_val(&buf.chgmon_val);
		break;
	case IOCTL_BATTIM_SET_CHGMON_VAL:
		ret = battim_set_chgmon_val(&buf.chgmon_val);
		break;
	case IOCTL_BATTIM_GET_CHGPROT_VAL:
		ret = battim_get_chgprot_val(&buf.chgprot_val);
		break;
	case IOCTL_BATTIM_SET_CHGPROT_VAL:
		ret = battim_set_chgprot_val(&buf.chgprot_val);
		break;
	case IOCTL_BATTIM_GET_PM_PARAM:
		ret = battim_get_pm_param(&buf.pm_param);
		break;
	case IOCTL_BATTIM_SET_PM_PARAM:
		ret = battim_set_pm_param(&buf.pm_param);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret < 0) {
		goto out;
	}

	/* Set Data to User */
	if ( _IOC_DIR(cmd) & _IOC_READ ) {
		if ( copy_to_user((void *)arg, &buf.head_pos, _IOC_SIZE(cmd)) ) {
			pr_err("%s: copy_to_user failed.\n", __func__);
			ret = -EFAULT;
			goto out;
		}
	}

out:
	return ret;
}

/**
 * battim_get_kern_event() - gets kernel event
 * @param:      structure of event information
 *
 * This routine is used to get 'battim' event in kernel.
 * It's supposed to be called from userland via ioctl.
 * It stops process and waits for new event, unless blocking flag is sets.
 */
int battim_get_kern_event(battim_event_type *param)
{
	int ret = 0;
	bool wait_flag = false;
	int prior_event;

	pr_debug("[IN]%s: status=%d pending_cnt=%d\n",
		__func__,
		battim_info.save_data.kern_event_reg.status,
		battim_info.save_data.kern_event_reg.pending_cnt);

	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	/* Enter mutex lock */
	if (mutex_lock_interruptible(&battim_info.kern_event_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}

	/* Change status */
	switch (battim_info.save_data.kern_event_reg.status) {
	case BATTIM_EVENT_STATUS_NOT_READY:
		ret = -EBUSY;
		break;
	case BATTIM_EVENT_STATUS_READY:
		/* Change state: READY -> USER_WAITING */
		battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_USER_WAITING;
		pr_devel("%s: Change state: READY -> USER_WAITING\n", __func__);
		/* Set action */
		wait_flag = true;
		break;
	case BATTIM_EVENT_STATUS_USER_WAITING:
		/* Keep status: USER_WAITING */
		pr_devel("%s: Keep state: USER_WAITING (Maybe user changed.)\n", __func__);
		/* Set action */
		wait_flag = true;
		break;
	case BATTIM_EVENT_STATUS_DATA_PENDING:
		prior_event = battim_sub_read_prior_kern_event();
		if (prior_event < 0) {
			ret = prior_event;
			/* Error message */
			pr_err("%s: Data error: No pending data.\n", __func__);
			/* Reset status */
			battim_sub_reset_kern_event_status();
		}
		else {
			/* Try to change state: DATA_PENDING -> READY */
			if (battim_info.save_data.kern_event_reg.pending_cnt == 0) {
				battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_READY;
				pr_devel("%s: Change state: DATA_PENDING -> READY\n", __func__);
			}
			else {
				pr_devel("%s: Keep state: DATA_PENDING (pending_cnt=%d)\n",
					__func__,
					battim_info.save_data.kern_event_reg.pending_cnt);
			}
			/* Load event information */
			memcpy(param, &battim_info.save_data.kern_event_reg.event_info[prior_event], sizeof(*param));
			pr_debug("%s: load event_id = %d\n", __func__, (int)param->event_id);
		}
		break;
	default:
		pr_err("%s: State error: status=%d\n", __func__, battim_info.save_data.kern_event_reg.status);
		/* Reset status */
		battim_sub_reset_kern_event_status();
		ret = -EBUSY;
		break;
	}

	/* Leave mutex lock */
	mutex_unlock(&battim_info.kern_event_mutex);

	/* Abort if error */
	if (ret < 0) {
		return ret;
	}

	/* End if event is already got */
	if (wait_flag == false) {
		pr_debug("[OUT]%s: status=%d pending_cnt=%d\n",
			__func__,
			battim_info.save_data.kern_event_reg.status,
			battim_info.save_data.kern_event_reg.pending_cnt);
		return 0;
	}

	/* Wait for new event */
	pr_devel("%s: Wait for new event.\n", __func__);
	battim_info.save_data.kern_event_reg.waiting = true;
	init_completion(&battim_info.kern_event_comp);
	if (wait_for_completion_interruptible(&battim_info.kern_event_comp)) {
		pr_info("%s: wait_for_completion_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}

	/* Enter mutex lock */
	if (mutex_lock_interruptible(&battim_info.kern_event_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}

	/* Change status on passing data */
	switch (battim_info.save_data.kern_event_reg.status) {
	case BATTIM_EVENT_STATUS_NOT_READY:
		/* Error message */
		pr_err("%s: Data passing error: Unexpected state: NOT_READY.\n", __func__);
		ret = -EBUSY;
		break;
	case BATTIM_EVENT_STATUS_READY:
		/* Error message */
		pr_err("%s: Data passing error: Unexpected state: READY.\n", __func__);
		/* Reset status */
		battim_sub_reset_kern_event_status();
		ret = -EBUSY;
		break;
	case BATTIM_EVENT_STATUS_USER_WAITING:
		/* Error message */
		pr_err("%s: Data passing error: Unexpected state: USER_WAITING.\n", __func__);
		/* Reset status */
		battim_sub_reset_kern_event_status();
		ret = -EBUSY;
		break;
	case BATTIM_EVENT_STATUS_DATA_PENDING:
		prior_event = battim_sub_read_prior_kern_event();
		if (prior_event < 0) {
			ret = prior_event;
			/* Error message */
			pr_err("%s: Data error: No pending data.\n", __func__);
			/* Reset status */
			battim_sub_reset_kern_event_status();
		}
		else {
			/* Try to change state: DATA_PENDING -> READY */
			if (battim_info.save_data.kern_event_reg.pending_cnt == 0) {
				battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_READY;
				pr_devel("%s: Change state: DATA_PENDING -> READY\n", __func__);
			}
			else {
				pr_devel("%s: Keep state: DATA_PENDING (pending_cnt=%d)\n",
					__func__,
					battim_info.save_data.kern_event_reg.pending_cnt);
			}
			/* Load event information */
			memcpy(param, &battim_info.save_data.kern_event_reg.event_info[prior_event], sizeof(*param));
			pr_debug("%s: load event_id = %d\n", __func__, (int)param->event_id);
		}
		break;
	default:
		/* Error message */
		pr_err("%s: Data passing error: Unexpected state: undefined(%d)\n", __func__, battim_info.save_data.kern_event_reg.status);
		/* Reset status */
		battim_sub_reset_kern_event_status();
		ret = -EBUSY;
		break;
	}

	/* Leave mutex lock */
	mutex_unlock(&battim_info.kern_event_mutex);

	/* Abort if error */
	if (ret < 0) {
		return ret;
	}

	pr_debug("[OUT]%s: status=%d pending_cnt=%d\n",
		__func__,
		battim_info.save_data.kern_event_reg.status,
		battim_info.save_data.kern_event_reg.pending_cnt);
	return 0;
}
EXPORT_SYMBOL(battim_get_kern_event);

/**
 * battim_set_kern_event() - sets kernel event
 * @param:      structure of event information
 *
 * This routine is used to set 'battim' event in kernel.
 * It's called from kernel.
 */
int battim_set_kern_event(battim_event_type *param)
{
	int ret = 0;
	bool comp_flag = false;

	pr_debug("[IN]%s: status=%d pending_cnt=%d\n",
		__func__,
		battim_info.save_data.kern_event_reg.status,
		battim_info.save_data.kern_event_reg.pending_cnt);

	if (battim_info.early_init_flag == false) {
		battim_early_init();
	}

	if (param->event_id >= BATTIM_KERN_EVENT_MAX) {
		pr_err("%s: Invalid argument: param->event_id=%d\n", __func__, param->event_id);
		return -EINVAL;
	}

	/* Enter mutex lock */
	if (mutex_lock_interruptible(&battim_info.kern_event_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}

	/* Change status */
	switch (battim_info.save_data.kern_event_reg.status) {
	case BATTIM_EVENT_STATUS_NOT_READY:
		ret = -EBUSY;
		break;
	case BATTIM_EVENT_STATUS_READY:
		/* Change state: READY -> DATA_PENDING */
		battim_info.save_data.kern_event_reg.pending[param->event_id] = true;
		battim_info.save_data.kern_event_reg.pending_cnt = 1;
		battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_DATA_PENDING;
		pr_devel("%s: Change state: READY -> DATA_PENDING\n", __func__);
		/* Save event information */
		memcpy(&battim_info.save_data.kern_event_reg.event_info[param->event_id], param, sizeof(*param));
		pr_debug("%s: save event_id = %d\n", __func__, (int)param->event_id);
		break;
	case BATTIM_EVENT_STATUS_USER_WAITING:
		/* Change state: USER_WAITING -> PENDING */
		battim_info.save_data.kern_event_reg.pending[param->event_id] = true;
		battim_info.save_data.kern_event_reg.pending_cnt = 1;
		battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_DATA_PENDING;
		pr_devel("%s: Change state: USER_WAITING -> DATA_PENDING\n", __func__);
		/* Save event information */
		memcpy(&battim_info.save_data.kern_event_reg.event_info[param->event_id], param, sizeof(*param));
		pr_debug("%s: save event_id = %d\n", __func__, (int)param->event_id);
		/* End waiting */
		comp_flag = true;
		break;
	case BATTIM_EVENT_STATUS_DATA_PENDING:
		/* Keep status: DATA_PENDING */
		if (battim_info.save_data.kern_event_reg.pending[param->event_id] == false) {
			battim_info.save_data.kern_event_reg.pending[param->event_id] = true;
			battim_info.save_data.kern_event_reg.pending_cnt++;
		}
		pr_devel("%s: Continue pending.\n", __func__);
		/* Save event information */
		pr_debug("%s: save event_id = %d\n", __func__, (int)param->event_id);
		memcpy(&battim_info.save_data.kern_event_reg.event_info[param->event_id], param, sizeof(*param));
		break;
	default:
		/* Error message */
		pr_err("%s: State error: status=%d\n", __func__, battim_info.save_data.kern_event_reg.status);
		/* Reset status */
		battim_sub_reset_kern_event_status();
		ret = -EBUSY;
		break;
	}

	/* Leave mutex lock */
	mutex_unlock(&battim_info.kern_event_mutex);

	/* Action */
	if (comp_flag == true) {
		if (battim_info.save_data.kern_event_reg.waiting == true) {
			battim_info.save_data.kern_event_reg.waiting = false;
			schedule_work(&battim_info.comp_work);
		}
	}

	pr_debug("[OUT]%s: status=%d pending_cnt=%d\n",
		__func__,
		battim_info.save_data.kern_event_reg.status,
		battim_info.save_data.kern_event_reg.pending_cnt);

	return ret;
}
EXPORT_SYMBOL(battim_set_kern_event);

static void battim_kern_event_complete(struct work_struct *work)
{
	struct battim_info_struct *battim_info = container_of(work,
					struct battim_info_struct, comp_work);

	pr_devel("%s: Set new event.\n", __func__);
	complete(&battim_info->kern_event_comp);
}

/**
 * battim_get_user_event() - gets userland event
 * @param:      structure of event information
 *
 * This routine is used to get 'battim' event in userland.
 * It's not supposed to be called, so it's not implemented.
 */

/**
 * battim_set_user_event() - sets userland event
 * @param:      structure of event information
 *
 * This routine is used to set 'battim' event in userland.
 * It's supposed to be called from userland via ioctl.
 * It fires event handler, when it is called.
 */
int battim_set_user_event(battim_event_type *param)
{
	battim_event_func_type func;

	pr_debug("%s: event_id=%d\n", __func__, param->event_id);

	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (param->event_id >= BATTIM_USER_EVENT_MAX) {
		pr_err("%s: Invalid argument: param->event_id=%d\n", __func__, param->event_id);
		return -EINVAL;
	}

	/* Call event handler function */
	func = battim_info.save_data.user_event_reg.handler[param->event_id];
	if (func != NULL) {
		pr_debug("%s: call event handler\n", __func__);
		func(param);
	}

	return 0;
}
EXPORT_SYMBOL(battim_set_user_event);

/**
 * battim_get_battcal_val() - gets values named 'battcal_val'
 * @param:      structure composed of values named 'battcal_val'
 *
 * This routine is used to get data for updating battery information.
 * It's supposed to be called from both kernel and userland.
 */
int battim_get_battcal_val(battim_battcal_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.battcal_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(param, &battim_info.save_data.battcal_val, sizeof(*param));
	mutex_unlock(&battim_info.battcal_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_get_battcal_val);

/**
 * battim_set_battcal_val() - sets values named 'battcal_val'
 * @param:      structure composed of values named 'battcal_val'
 *
 * This routine is used to get data for updating battery information.
 * It's supposed to be called from both kernel and userland.
 */
int battim_set_battcal_val(battim_battcal_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.battcal_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(&battim_info.save_data.battcal_val, param, sizeof(*param));
	mutex_unlock(&battim_info.battcal_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_set_battcal_val);

/**
 * battim_get_battcal_hist() - gets values named 'battcal_hist'
 * @param:      structure composed of values named 'battcal_hist'
 *
 * This routine is used to get data for updating battery information.
 * It's supposed to be called by userland via ioctl.
 */
int battim_get_battcal_hist(battim_battcal_hist_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.battcal_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(param, &battim_info.save_data.battcal_hist, sizeof(*param));
	mutex_unlock(&battim_info.battcal_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_get_battcal_hist);

/**
 * battim_set_battcal_hist() - sets values named 'battcal_hist'
 * @param:      structure composed of values named 'battcal_hist'
 *
 * This routine is used to get data for updating battery information.
 * It's supposed to be called by userland via ioctl.
 */
int battim_set_battcal_hist(battim_battcal_hist_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.battcal_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(&battim_info.save_data.battcal_hist, param, sizeof(*param));
	mutex_unlock(&battim_info.battcal_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_set_battcal_hist);

/**
 * battim_get_obs_coll() - gets 'pm_obs' data collection
 * @data:       structure composed 'pm_obs' data and update mask
 *
 * This routine is used to get device-status information in battim.
 * It's supposed to be called from both kernel and userland.
 */
int battim_get_obs_coll(battim_obs_coll_type *data)
{
	int i;

	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.obs_coll_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	for (i = 0; i < BATTIM_OBS_IDX_MAX; i++) {
		data->param[i] = battim_info.save_data.obs_coll.param[i];
	}
	mutex_unlock(&battim_info.obs_coll_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_get_obs_coll);

/**
 * battim_set_obs_coll() - sets 'pm_obs' data collection
 * @data:       structure composed 'pm_obs' data and update mask
 *
 * This routine is used to set device-status information in battim.
 * It's supposed to be called from both kernel and userland.
 */
int battim_set_obs_coll(battim_obs_coll_type *data)
{
	int i;

	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.obs_coll_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	for (i = 0; i < BATTIM_OBS_IDX_MAX; i++) {
		if (data->update_flag[i] == true) {
			battim_info.save_data.obs_coll.param[i] = data->param[i];
		}
	}
	mutex_unlock(&battim_info.obs_coll_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_set_obs_coll);

/**
 * battim_get_calmon_val() - gets values used in calc monitor
 * @param:      structure composed of values used in calc monitor
 *
 * This routine is used to get data which monitor program requires.
 * It's supposed to be called by userland via ioctl.
 */
int battim_get_calmon_val(battim_calmon_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.mon_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(param, &battim_info.save_data.calmon_val, sizeof(*param));
	mutex_unlock(&battim_info.mon_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_get_calmon_val);

/**
 * battim_set_calmon_val() - sets values used in calc monitor
 * @param:      structure composed of values used in calc monitor
 *
 * This routine is used to set data which monitor program requires.
 * It's supposed to be called by userland via ioctl.
 */
int battim_set_calmon_val(battim_calmon_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.mon_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(&battim_info.save_data.calmon_val, param, sizeof(*param));
	mutex_unlock(&battim_info.mon_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_set_calmon_val);


/**
 * battim_get_obsmon_val() - gets values used in obs monitor
 * @param:      structure composed of values used in obs monitor
 *
 * This routine is used to get data which monitor program requires.
 * It's supposed to be called by userland via ioctl.
 */
int battim_get_obsmon_val(battim_obsmon_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.mon_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(param, &battim_info.save_data.obsmon_val, sizeof(*param));
	mutex_unlock(&battim_info.mon_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_get_obsmon_val);

/**
 * battim_set_obsmon_val() - sets values used in obs monitor
 * @param:      structure composed of values used in obs monitor
 *
 * This routine is used to set data which monitor program requires.
 * It's supposed to be called by userland via ioctl.
 */
int battim_set_obsmon_val(battim_obsmon_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.mon_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(&battim_info.save_data.obsmon_val, param, sizeof(*param));
	mutex_unlock(&battim_info.mon_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_set_obsmon_val);

/**
 * battim_get_chgmon_val() - gets values used in chg monitor
 * @param:      structure composed of values used in chg monitor
 *
 * This routine is used to get data which monitor program requires.
 * It's supposed to be called by userland via ioctl.
 */
int battim_get_chgmon_val(battim_chgmon_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.mon_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(param, &battim_info.save_data.chgmon_val, sizeof(*param));
	mutex_unlock(&battim_info.mon_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_get_chgmon_val);

/**
 * battim_set_chgmon_val() - sets values used in chg monitor
 * @param:      structure composed of values used in chg monitor
 *
 * This routine is used to set data which monitor program requires.
 * It's supposed to be called by kernel.
 */
int battim_set_chgmon_val(battim_chgmon_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.mon_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(&battim_info.save_data.chgmon_val, param, sizeof(*param));
	mutex_unlock(&battim_info.mon_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_set_chgmon_val);

/**
 * battim_set_chgprot_val() - sets values for 'charging protection'
 * @param:      structure composed of values for 'charging protection'
 *
 * This routine is used to set data for protection function in charging
 * It's supposed to be called by userland via ioctl.
 */
int battim_get_chgprot_val(battim_chgprot_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.chgprot_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(param, &battim_info.save_data.chgprot_val, sizeof(*param));
	mutex_unlock(&battim_info.chgprot_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_get_chgprot_val);

/**
 * battim_set_chgprot_val() - sets values for 'charging protection'
 * @param:      structure composed of values for 'charging protection'
 *
 * This routine is used to set data for protection function in charging
 * It's supposed to be called by kernel.
 */
int battim_set_chgprot_val(battim_chgprot_val_type *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	if (mutex_lock_interruptible(&battim_info.chgprot_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}
	memcpy(&battim_info.save_data.chgprot_val, param, sizeof(*param));
	mutex_unlock(&battim_info.chgprot_mutex);

	return 0;
}
EXPORT_SYMBOL(battim_set_chgprot_val);

/**
 * battim_get_pm_param() - gets values for 'PM specific parameters'
 * @param:      structure composed of values for 'PM specific parameters'
 *
 * This routine is used to get the PM specific parameters.
 * It's supposed to be called by userland via ioctl.
 */
int battim_get_pm_param(struct pm_param_struct *param)
{
	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	memcpy(param, &pm_param, sizeof(*param));

	return 0;
}

/**
 * battim_set_pm_param() - sets values for 'PM specific parameters'
 * @param:      structure composed of values for 'PM specific parameters'
 *
 * This routine is used to set the PM specific parameters.
 * It's supposed to be called by userland via ioctl.
 */
int battim_set_pm_param(struct pm_param_struct *param)
{
	pr_debug("%s\n", __func__);

	if (battim_info.init_flag == false) {
		return -EBUSY;
	}

	memcpy(&pm_param, param, sizeof(*param));

      if( pm8921_set_vbatdet_on_pm_param_update()) {
            return -EBUSY;
      }

	return 0;
}

/**
 * battim_event_connect() - connects an event and a handler.
 * @kind:       BATTIM_EVENT_KIND_KERN or BATTIM_EVENT_KIND_USER
 * @event:      target event
 * @func:       handler function pointer
 *
 * This routine sets event handler.
 */
int battim_event_connect(battim_event_kind_type kind, uint8_t event_id, battim_event_func_type func)
{
	pr_debug("%s: event_id=%d, func=%p\n", __func__, event_id, func);

	if (battim_info.cleanup_flag == true) {
		return -EBUSY;
	}

	if (battim_info.early_init_flag == false) {
		battim_early_init();
	}

	if (event_id >= BATTIM_USER_EVENT_MAX) {
		pr_err("%s: Invalid argument: event_id=%d\n", __func__, event_id);
		return -EINVAL;
	}

	battim_info.save_data.user_event_reg.handler[event_id] = func;

	return 0;
}

/**
 * battim_update_obs_param() - update specific 'pm_obs' data
 * @index:      device ID in this module
 * @mode:       'pm_obs' API parameter called 'mode'
 * @value:      'pm_obs' API parameter called 'value'
 *
 * This routine is used to update 'pm_obs' data.
 * It's supposed to be called by pm_obs_api.c.
 */
int battim_update_obs_param(battim_obs_index_type index, uint8_t mode, int32_t value)
{
	if (battim_info.cleanup_flag == true) {
		return -EBUSY;
	}

	if (battim_info.early_init_flag == false) {
		battim_early_init();
	}

	battim_info.save_data.obs_coll.param[index].pm_obs_mode = mode;
	battim_info.save_data.obs_coll.param[index].value = value;
	return 0;
}
EXPORT_SYMBOL(battim_update_obs_param); /* Called by pm_obs_api.c. */

static struct file_operations battim_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl		= battim_ioctl,
	.open			= battim_open,
	.release		= battim_release,
};

static struct miscdevice battim_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "battim",
	.fops = &battim_fops,
};


static void battim_sub_reset_kern_event_status(void)
{
	int i;
	/* Reset pending */
	for (i=0; i<BATTIM_KERN_EVENT_MAX; i++) {
		battim_info.save_data.kern_event_reg.pending[i] = false;
	}
	battim_info.save_data.kern_event_reg.pending_cnt = 0;
	/* Reset waiting */
	battim_info.save_data.kern_event_reg.waiting = false;
	/* Reset state to ready */
	battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_READY;
};

static int battim_sub_read_prior_kern_event(void)
{
	int ret = 0;
	int prior_event = -1;
	int i;

	/* Search prior pending event */
	for (i = 0; i < BATTIM_KERN_EVENT_MAX; i++) {
		if (battim_info.save_data.kern_event_reg.pending[i] == true) {
			prior_event = i;
			break;
		}
	}
	if (prior_event == -1) {
		ret = -ENODATA;
	}
	else {
		/* Clear events.  */
		for (; i < BATTIM_KERN_EVENT_MAX; i++) {
			if (battim_info.save_data.kern_event_reg.pending[i] == true) {
				battim_info.save_data.kern_event_reg.pending[i] = false;
			}
		}
		battim_info.save_data.kern_event_reg.pending_cnt = 0;

		/* Set return code to prior event_id */
		ret = prior_event;
	}

	return ret;
};

/**
 * battim_cancel_kern_event() - unsets kernel event
 * @param:      structure of event information
 *
 * This routine is used to unset 'battim' event in kernel.
 * It's called from kernel.
 */
int battim_cancel_kern_event(uint8_t event_id)
{
	int ret = 0;

	pr_debug("[IN]%s: status=%d pending_cnt=%d\n",
		__func__,
		battim_info.save_data.kern_event_reg.status,
		battim_info.save_data.kern_event_reg.pending_cnt);

	if (battim_info.early_init_flag == false) {
		battim_early_init();
	}

	if (event_id >= BATTIM_KERN_EVENT_MAX) {
		pr_err("%s: Invalid argument: event_id=%d\n", __func__, event_id);
		return -EINVAL;
	}

	/* Enter mutex lock */
	if (mutex_lock_interruptible(&battim_info.kern_event_mutex)) {
		pr_info("%s: mutex_lock_interruptible failed\n", __func__);
		return -ERESTARTSYS;
	}

	/* Change status */
	switch (battim_info.save_data.kern_event_reg.status) {
	case BATTIM_EVENT_STATUS_NOT_READY:
		ret = -EBUSY;
		break;
	case BATTIM_EVENT_STATUS_READY:
		/* Keep status: READY */
		pr_devel("%s: Keep state: READY\n", __func__);
		ret = -ENODATA;
		break;
	case BATTIM_EVENT_STATUS_USER_WAITING:
		/* Keep status: READY */
		pr_devel("%s: Keep state: USER_WAITING\n", __func__);
		ret = -ENODATA;
		break;
	case BATTIM_EVENT_STATUS_DATA_PENDING:
		if (battim_info.save_data.kern_event_reg.pending[event_id] == true) {
			battim_info.save_data.kern_event_reg.pending[event_id] = false;
			battim_info.save_data.kern_event_reg.pending_cnt--;
			pr_debug("%s: cancel event_id = %d\n", __func__, event_id);
		}
		/* Try to change state: DATA_PENDING -> READY */
		if (battim_info.save_data.kern_event_reg.pending_cnt == 0) {
			battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_READY;
			pr_devel("%s: Change state: DATA_PENDING -> READY\n", __func__);
		}
		else {
			pr_devel("%s: Keep state: DATA_PENDING (pending_cnt=%d)\n",
				__func__,
				battim_info.save_data.kern_event_reg.pending_cnt);
		}
		break;
	default:
		/* Error message */
		pr_err("%s: State error: status=%d\n", __func__, battim_info.save_data.kern_event_reg.status);
		/* Reset status */
		battim_sub_reset_kern_event_status();
		ret = -EBUSY;
		break;
	}

	/* Leave mutex lock */
	mutex_unlock(&battim_info.kern_event_mutex);

	pr_debug("[OUT]%s: status=%d pending_cnt=%d\n",
		__func__,
		battim_info.save_data.kern_event_reg.status,
		battim_info.save_data.kern_event_reg.pending_cnt);

	return ret;
}
EXPORT_SYMBOL(battim_cancel_kern_event);

void battim_init_pm_param(void)
{
	if (battim_info.pm_param_init_flag == true) {
		pr_info("%s: pm_param is already initialized.\n", __func__);
		return;
	}

	pm_param.pm_pw_ctrl_vdet              = pm_pw_ctrl_vdet_default;
	pm_param.pm_pw_ctrl_dvdet             = pm_pw_ctrl_dvdet_default;
	pm_param.pm_pw_ctrl_ct                = pm_pw_ctrl_ct_default;
	pm_param.pm_pw_ctrl_batadj            = pm_pw_ctrl_batadj_default;
	pm_param.pm_pw_ctrl_batadjth          = pm_pw_ctrl_batadjth_default;
	pm_param.pm_pw_ctrl_rtcadj            = pm_pw_ctrl_rtcadj_default;
	pm_param.pm_pw_adc_determin           = pm_pw_adc_determin_default;
	pm_param.pm_pw_ctrl_temp_batadj       = pm_pw_ctrl_temp_batadj_default;
	pm_param.pm_pw_ctrl_temp_err          = pm_pw_ctrl_temp_err_default;
	pm_param.pm_pw_test_temp_set_status   = pm_pw_test_temp_set_status_default;

	pm_param.pm_charge_off_on_xo_temp_cam = pm_charge_off_on_xo_temp_cam_default;
	pm_param.pm_charge_off_on_xo_temp_lte = pm_charge_off_on_xo_temp_lte_default;
	pm_param.pm_charge_off_on_xo_temp     = pm_charge_off_on_xo_temp_default;
	pm_param.pm_charge_off_on_xo_temp_mm  = pm_charge_off_on_xo_temp_mm_default;
	pm_param.pm_chg_setting               = pm_chg_setting_default;

	pm_param.pm_bat_alm_cut_off           = pm_bat_alm_cut_off_default;
	pm_param.pm_lva_off                   = pm_lva_off_default;

	battim_info.pm_param_init_flag = true;
}
EXPORT_SYMBOL(battim_init_pm_param);


static void battim_early_init(void)
{
	pr_debug("%s\n", __func__);

	/*
	 * Exclusive access starts.
	 */
	mutex_lock(&battim_early_init_mutex);
	if (battim_info.early_init_flag == true) {
		pr_err("%s: Already called.\n", __func__);
		goto unlock;
	}

	/*
	 * Initialize for event connects and pm_obs APIs.
	 */

	/* mutex */
	mutex_init(&battim_info.kern_event_mutex);
	mutex_init(&battim_info.obs_coll_mutex);

	/* workqueue */
	INIT_WORK(&battim_info.comp_work, battim_kern_event_complete);

	/* completion */
	init_completion(&battim_info.kern_event_comp);

	/* data structure */
	memset(&battim_info.save_data.kern_event_reg, 0, sizeof(battim_info.save_data.kern_event_reg));
	memset(&battim_info.save_data.user_event_reg, 0, sizeof(battim_info.save_data.user_event_reg));
	memset(&battim_info.save_data.obs_coll,       0, sizeof(battim_info.save_data.obs_coll      ));

	/* status */
	battim_info.save_data.kern_event_reg.status = BATTIM_EVENT_STATUS_READY;

	/*
	 * Initialize PM specific parameters
	 */
	battim_init_pm_param();

	/*
	 * Exclusive access ends.
	 */
	battim_info.early_init_flag = true;

unlock:
	mutex_unlock(&battim_early_init_mutex);
}

static int __init battim_init(void)
{
	int ret;

	ret = misc_register(&battim_dev);
	if (ret) {
		pr_err("fail to misc_register (MISC_DYNAMIC_MINOR)\n");
		return ret;
	}

	pr_info("battim: Succefully loaded.\n");

	/*
	 * Initialize save data for event connects and pm_obs APIs.
	 */
	if (battim_info.early_init_flag == false) {
		battim_early_init();
	}

	/*
	 * Initialize save data for the others.
	 */

	/* mutex */
	mutex_init(&battim_info.battcal_mutex);
	mutex_init(&battim_info.mon_mutex);
	mutex_init(&battim_info.chgprot_mutex);

	/* data structures */
	memset(&battim_info.save_data.battcal_val,  0, sizeof(battim_info.save_data.battcal_val ));
	memset(&battim_info.save_data.battcal_hist, 0, sizeof(battim_info.save_data.battcal_hist));
	memset(&battim_info.save_data.calmon_val,   0, sizeof(battim_info.save_data.calmon_val  ));
	memset(&battim_info.save_data.obsmon_val,   0, sizeof(battim_info.save_data.obsmon_val  ));
	memset(&battim_info.save_data.chgmon_val,   0, sizeof(battim_info.save_data.chgmon_val  ));
	memset(&battim_info.save_data.chgprot_val,  0, sizeof(battim_info.save_data.chgprot_val ));

	/*
	 * Set initialization flag.
	 */
	battim_info.init_flag = true;

	return 0;
}

static void __exit battim_cleanup(void)
{
	/* Set cleanup flag */
	battim_info.cleanup_flag = true;

	/* Release resources */
	mutex_destroy(&battim_info.kern_event_mutex);
	mutex_destroy(&battim_info.battcal_mutex);
	mutex_destroy(&battim_info.obs_coll_mutex);
	mutex_destroy(&battim_info.mon_mutex);
	mutex_destroy(&battim_info.chgprot_mutex);
	mutex_destroy(&battim_early_init_mutex);

	/* Deregister */
	misc_deregister(&battim_dev);
 	pr_info("battim: Unloaded.\n"); 
}


MODULE_LICENSE("GPL");
module_init(battim_init);
module_exit(battim_cleanup);

