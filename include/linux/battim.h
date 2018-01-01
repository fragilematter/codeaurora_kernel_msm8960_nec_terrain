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
 */

#ifndef __BATTIM_H__
#define __BATTIM_H__

#include <linux/types.h>
#include <linux/ioctl.h>

/********************************************************************/
/*  Constant Definition                                             */
/********************************************************************/

#define BATTIM_IOCTL_MAGIC 0xD8  /* 'B'+'I'+'M' */

/* ioctl commands for sharing any events among kernel and userland */
#define IOCTL_BATTIM_GET_KERN_EVENT  _IOWR ( BATTIM_IOCTL_MAGIC, BIMIOC_GET_KERN_EVENT, battim_event_type )
#define IOCTL_BATTIM_SET_KERN_EVENT  _IOWR ( BATTIM_IOCTL_MAGIC, BIMIOC_SET_KERN_EVENT, battim_event_type )
#define IOCTL_BATTIM_GET_USER_EVENT  _IOWR ( BATTIM_IOCTL_MAGIC, BIMIOC_GET_USER_EVENT, battim_event_type )
#define IOCTL_BATTIM_SET_USER_EVENT  _IOWR ( BATTIM_IOCTL_MAGIC, BIMIOC_SET_USER_EVENT, battim_event_type )

/* ioctl commands to get/set the values for battery calculation */
#define IOCTL_BATTIM_GET_BATTCAL_VAL   _IOR ( BATTIM_IOCTL_MAGIC, BIMIOC_GET_BATTCAL_VAL,  battim_battcal_val_type  )
#define IOCTL_BATTIM_SET_BATTCAL_VAL   _IOW ( BATTIM_IOCTL_MAGIC, BIMIOC_SET_BATTCAL_VAL,  battim_battcal_val_type  )
#define IOCTL_BATTIM_GET_BATTCAL_HIST  _IOR ( BATTIM_IOCTL_MAGIC, BIMIOC_GET_BATTCAL_HIST, battim_battcal_hist_type )
#define IOCTL_BATTIM_SET_BATTCAL_HIST  _IOW ( BATTIM_IOCTL_MAGIC, BIMIOC_SET_BATTCAL_HIST, battim_battcal_hist_type )

/* ioctl commands to get/set the values of specific device's power status */
#define IOCTL_BATTIM_GET_OBS_COLL  _IOR (BATTIM_IOCTL_MAGIC, BIMIOC_GET_OBS_COLL, battim_obs_coll_type )
#define IOCTL_BATTIM_SET_OBS_COLL  _IOW (BATTIM_IOCTL_MAGIC, BIMIOC_SET_OBS_COLL, battim_obs_coll_type )

/* ioctl commands to get/set the values required by monitor function */
#define IOCTL_BATTIM_GET_CALMON_VAL  _IOR (BATTIM_IOCTL_MAGIC, BIMIOC_GET_CALMON_VAL, battim_calmon_val_type )
#define IOCTL_BATTIM_SET_CALMON_VAL  _IOW (BATTIM_IOCTL_MAGIC, BIMIOC_SET_CALMON_VAL, battim_calmon_val_type )
#define IOCTL_BATTIM_GET_OBSMON_VAL  _IOR (BATTIM_IOCTL_MAGIC, BIMIOC_GET_OBSMON_VAL, battim_obsmon_val_type )
#define IOCTL_BATTIM_SET_OBSMON_VAL  _IOW (BATTIM_IOCTL_MAGIC, BIMIOC_SET_OBSMON_VAL, battim_obsmon_val_type )
#define IOCTL_BATTIM_GET_CHGMON_VAL  _IOR (BATTIM_IOCTL_MAGIC, BIMIOC_GET_CHGMON_VAL, battim_chgmon_val_type )
#define IOCTL_BATTIM_SET_CHGMON_VAL  _IOW (BATTIM_IOCTL_MAGIC, BIMIOC_SET_CHGMON_VAL, battim_chgmon_val_type )

/* ioctl commands to get/set the values required by protective function for charging */
#define IOCTL_BATTIM_GET_CHGPROT_VAL  _IOR (BATTIM_IOCTL_MAGIC, BIMIOC_GET_CHGPROT_VAL, battim_chgprot_val_type )
#define IOCTL_BATTIM_SET_CHGPROT_VAL  _IOW (BATTIM_IOCTL_MAGIC, BIMIOC_SET_CHGPROT_VAL, battim_chgprot_val_type )

/* ioctl commands to get/set the values of PM specific parameter */
#define IOCTL_BATTIM_GET_PM_PARAM  _IOR (BATTIM_IOCTL_MAGIC, BIMIOC_GET_PM_PARAM, struct pm_param_struct )
#define IOCTL_BATTIM_SET_PM_PARAM  _IOW (BATTIM_IOCTL_MAGIC, BIMIOC_SET_PM_PARAM, struct pm_param_struct )

/* Number of values required by each monitor */
#define BATTIM_CALMON_VAL_MAX      9
#define BATTIM_OBSMON_VAL_MAX      14
#define BATTIM_CHGMON_VAL_MAX      15
#define BATTIM_CHGMON_TIMEVAL_MAX  1

/* Number of values required by battcal history */
#define BATTIM_BATTCAL_HIST_CV_MAX 10

/* Inner ID of ioctl commands */
enum bimioc_cmd_enum {
	BIMIOC_GET_KERN_EVENT,
	BIMIOC_SET_KERN_EVENT,
	BIMIOC_GET_USER_EVENT,
	BIMIOC_SET_USER_EVENT,
	BIMIOC_GET_BATTCAL_VAL,
	BIMIOC_SET_BATTCAL_VAL,
	BIMIOC_GET_BATTCAL_HIST,
	BIMIOC_SET_BATTCAL_HIST,
	BIMIOC_GET_OBS_COLL,
	BIMIOC_SET_OBS_COLL,
	BIMIOC_GET_CALMON_VAL,
	BIMIOC_SET_CALMON_VAL,
	BIMIOC_GET_OBSMON_VAL,
	BIMIOC_SET_OBSMON_VAL,
	BIMIOC_GET_CHGMON_VAL,
	BIMIOC_SET_CHGMON_VAL,
	BIMIOC_GET_CHGPROT_VAL,
	BIMIOC_SET_CHGPROT_VAL,
	BIMIOC_GET_PM_PARAM,
	BIMIOC_SET_PM_PARAM,
	BIMIOC_MAX
};

/* Event kinds */
typedef enum {
	BATTIM_EVENT_KIND_KERN,
	BATTIM_EVENT_KIND_USER,
	BATTIM_EVENT_KIND_MAX
} battim_event_kind_type;

/* Kernel events */
enum battim_kern_event_enum {
	BATTIM_KERN_EVENT_WAKEUP,
	BATTIM_KERN_EVENT_REQ_BATTCAL_WAKEUP,
	BATTIM_KERN_EVENT_REQ_BATTCAL,
	BATTIM_KERN_EVENT_REQ_UPDATE_OBS_MDM,
	BATTIM_KERN_EVENT_MAX
};

/* Userland events */
enum battim_user_event_enum {
	BATTIM_USER_EVENT_FIN_BATTCAL,
	BATTIM_USER_EVENT_FIN_UPDATE_OBS_MDM,
	BATTIM_USER_EVENT_REQ_UPDATE_BATTCAL_VAL,
	BATTIM_USER_EVENT_REQ_UPDATE_CHGMON_VAL,
	BATTIM_USER_EVENT_REQ_UPDATE_CHGPROT_VAL,
	BATTIM_USER_EVENT_FIN_UPDATE_PARAM, 
	BATTIM_USER_EVENT_RESUME,
	BATTIM_USER_EVENT_MAX
};

typedef enum {
	BATTIM_EVENT_STATUS_NOT_READY,
	BATTIM_EVENT_STATUS_READY,
	BATTIM_EVENT_STATUS_USER_WAITING,
	BATTIM_EVENT_STATUS_DATA_PENDING,
	BATTIM_EVENT_STATUS_DATA_MAX
} battim_event_status_type;

/* Index for internal use of 'pm_obs' data */
typedef enum {
	BATTIM_OBS_IDX_BOOTUP,
	BATTIM_OBS_IDX_CHARGER,
	BATTIM_OBS_IDX_CONNECT_DATA,
	BATTIM_OBS_IDX_CONNECT,
	BATTIM_OBS_IDX_CAMERA,
	BATTIM_OBS_IDX_CAMLIGHT,
	BATTIM_OBS_IDX_LCDBACKLIGHT,
	BATTIM_OBS_IDX_KEYBACKLIGHT,
	BATTIM_OBS_IDX_VIBRATION,
	BATTIM_OBS_IDX_SPEAKER,
	BATTIM_OBS_IDX_BLUETOOTH,
	BATTIM_OBS_IDX_WLAN,
	BATTIM_OBS_IDX_SENSOR,
	BATTIM_OBS_IDX_DTV,
	BATTIM_OBS_IDX_FELICA,
	BATTIM_OBS_IDX_MULTIMEDIA,
	BATTIM_OBS_IDX_MAX
} battim_obs_index_type;


enum battim_event_err_enum {
	BATTIM_EVENT_ERR_NONE,
	BATTIM_EVENT_ERR_FAILED,
	BATTIM_EVENT_ERR_RPC_FAILED,
	BATTIM_EVENT_ERR_MAX
};


/********************************************************************/
/* Data Type Definition                                             */
/********************************************************************/

/*
 * The followings are data types for implementing the ioctl commands.
 */

typedef struct {
	uint8_t event_id;
	int32_t error;
} __attribute__ ((packed)) battim_event_type;

typedef void (*battim_event_func_type)(battim_event_type *data);

typedef struct {
	uint8_t status;
	bool waiting;
	uint8_t pending_cnt;
	bool pending[BATTIM_KERN_EVENT_MAX];
	battim_event_type event_info[BATTIM_KERN_EVENT_MAX];
} __attribute__ ((packed)) battim_kern_event_reg_type;

typedef struct {
	battim_event_func_type handler[BATTIM_USER_EVENT_MAX];
} __attribute__ ((packed)) battim_user_event_reg_type ;

typedef struct {
	uint16_t v_adc;
	uint16_t t_adc;
	int32_t time;
	uint16_t voltage;
	uint8_t capacity;

} __attribute__ ((packed)) battim_battcal_val_type;

typedef struct {
	uint16_t cv[BATTIM_BATTCAL_HIST_CV_MAX];
	uint8_t idx_cv_last;

	double cap_last;

	int32_t time[BATTIM_BATTCAL_HIST_CV_MAX];
	int32_t time_last;

} __attribute__ ((packed)) battim_battcal_hist_type;

typedef struct {
	uint8_t pm_obs_mode; /* pm_obs_mode_type */
	int32_t value;
} battim_obs_param_type;

typedef struct {
	battim_obs_param_type param[BATTIM_OBS_IDX_MAX];
	bool update_flag[BATTIM_OBS_IDX_MAX];
} __attribute__ ((packed)) battim_obs_coll_type;

typedef struct {
	int16_t old_value[BATTIM_CALMON_VAL_MAX];
	int16_t new_value[BATTIM_CALMON_VAL_MAX];
} __attribute__ ((packed)) battim_calmon_val_type;

typedef struct {
	uint8_t value[BATTIM_OBSMON_VAL_MAX];
} __attribute__ ((packed)) battim_obsmon_val_type;

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} __attribute__ ((packed)) battim_chgmon_val_time_type;

typedef struct {
	int16_t value[BATTIM_CHGMON_VAL_MAX];
	battim_chgmon_val_time_type time_value[BATTIM_CHGMON_TIMEVAL_MAX];
} __attribute__ ((packed)) battim_chgmon_val_type;

typedef struct {
	uint16_t xo_therm;
} __attribute__ ((packed)) battim_chgprot_val_type;

/********************************************************************/
/* Function Declaration                                             */
/********************************************************************/

extern int battim_get_kern_event(battim_event_type *param);
extern int battim_set_kern_event(battim_event_type *param);
extern int battim_get_user_event(battim_event_type *param);
extern int battim_set_user_event(battim_event_type *param);
extern int battim_get_battcal_val(battim_battcal_val_type *param);
extern int battim_set_battcal_val(battim_battcal_val_type *param);
extern int battim_get_battcal_hist(battim_battcal_hist_type *param);
extern int battim_set_battcal_hist(battim_battcal_hist_type *param);
extern int battim_get_obs_coll(battim_obs_coll_type *data);
extern int battim_set_obs_coll(battim_obs_coll_type *data);
extern int battim_get_calmon_val(battim_calmon_val_type *param);
extern int battim_set_calmon_val(battim_calmon_val_type *param);
extern int battim_get_obsmon_val(battim_obsmon_val_type *param);
extern int battim_set_obsmon_val(battim_obsmon_val_type *param);
extern int battim_get_chgmon_val(battim_chgmon_val_type *param);
extern int battim_set_chgmon_val(battim_chgmon_val_type *param);
extern int battim_get_chgprot_val(battim_chgprot_val_type *param);
extern int battim_set_chgprot_val(battim_chgprot_val_type *param);
extern int battim_event_connect(battim_event_kind_type kind, uint8_t event, battim_event_func_type func);
extern int battim_update_obs_param(battim_obs_index_type index, uint8_t mode, int32_t value);

extern int battim_cancel_kern_event(uint8_t event_id);
extern void battim_init_pm_param(void);

#endif /* __BATTIM_H__ */
