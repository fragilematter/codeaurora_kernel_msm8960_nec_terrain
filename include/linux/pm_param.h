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

#ifndef __PM_PARAM_H__
#define __PM_PARAM_H__
#include <linux/types.h>

/********************************************************************/
/*  Constant Definition                                             */
/********************************************************************/

/* Battery level indices */
enum {
	PM_VFULL_PICT = 0,
	PM_VFULL_PICT_ADJ,
	PM_VDET_6,
	PM_VDET_5,
	PM_VDET_4,
	PM_VDET_3,
	PM_VDET_2,
	PM_VDET_1,
	PM_VDET_0,
	PM_PW_CTRL_VDET_I_MAX
};

/* Temperature correction indices */
enum {
	PM_dVDET61 = 0,
	PM_dVDET51,
	PM_dVDET41,
	PM_dVDET31,
	PM_dVDET21,
	PM_dVDET11,
	PM_dVDET01,
	PM_dVDET62,
	PM_dVDET52,
	PM_dVDET42,
	PM_dVDET32,
	PM_dVDET22,
	PM_dVDET12,
	PM_dVDET02,
	PM_dVDET63,
	PM_dVDET53,
	PM_dVDET43,
	PM_dVDET33,
	PM_dVDET23,
	PM_dVDET13,
	PM_dVDET03,
	PM_PW_CTRL_DVDET_I_MAX
};


/* Charge protection timer indices */
enum {
	PM_TIMER_FAST = 0,
	PM_PW_CTRL_CT_I_MAX
};


/* Load correction indices */
enum {
	PM_VCMAX = 0,
	PM_ADJMSM_NORM,
	PM_ADJMSM_1X,
	PM_ADJMSM_EVDO,
	PM_ADJMSM_GSM_VOICE,
	PM_ADJMSM_GSM_DATA,
	PM_ADJMSM_UMTS_VOICE,
	PM_ADJMSM_UMTS_DATA,
	PM_ADJMSM_LTE_DATA,
	PM_ADJRF_CDMA_LV1,
	PM_ADJRF_CDMA_LV2,
	PM_ADJRF_CDMA_LV3,
	PM_ADJRF_CDMA_LV4,
	PM_ADJRF_CDMA_LV5,
	PM_ADJRF_GSM_LV1,
	PM_ADJRF_GSM_LV2,
	PM_ADJRF_GSM_LV3,
	PM_ADJRF_GSM_LV4,
	PM_ADJRF_GSM_LV5,
	PM_ADJRF_UMTS_LV1,
	PM_ADJRF_UMTS_LV2,
	PM_ADJRF_UMTS_LV3,
	PM_ADJRF_UMTS_LV4,
	PM_ADJRF_UMTS_LV5,
	PM_ADJRF_LTE_LV1,
	PM_ADJRF_LTE_LV2,
	PM_ADJRF_LTE_LV3,
	PM_ADJRF_LTE_LV4,
	PM_ADJRF_LTE_LV5,
	PM_ADJMSM_CAM1_ON,
	PM_ADJMSM_CAM2_ON,
	PM_ADJCAMF_ON,
	PM_ADJMOBF_ON,
	PM_ADJLCDBL_LV0,
	PM_ADJLCDBL_LV1,
	PM_ADJLCDBL_LV2,
	PM_ADJLCDBL_LV3,
	PM_ADJLCDBL_LV4,
	PM_ADJKEYBL_ON,
	PM_ADJVIB_ON,
	PM_ADJSPK_ON,
	PM_ADJBT_ON,
	PM_ADJPRO_ON,
	PM_ADJWLAN_SLEEP,
	PM_ADJWLAN_ON,
	PM_ADJDTV_ON,
	PM_ADJFELICA_ON,
	PM_ADJVPON,
	PM_ADJWAIT3,
	PM_WAKEUP_FROM_SLEEP,
	PM_PW_CTRL_BATADJ_I_MAX
};

/* Transmitted power threshold indices */
enum {
	PM_ADJRF_CDMA_TH1 = 0,
	PM_ADJRF_CDMA_TH2,
	PM_ADJRF_CDMA_TH3,
	PM_ADJRF_CDMA_TH4,
	PM_ADJRF_GSM_TH1,
	PM_ADJRF_GSM_TH2,
	PM_ADJRF_GSM_TH3,
	PM_ADJRF_GSM_TH4,
	PM_ADJRF_UMTS_TH1,
	PM_ADJRF_UMTS_TH2,
	PM_ADJRF_UMTS_TH3,
	PM_ADJRF_UMTS_TH4,
	PM_ADJRF_LTE_TH1,
	PM_ADJRF_LTE_TH2,
	PM_ADJRF_LTE_TH3,
	PM_ADJRF_LTE_TH4,
	PM_PW_CTRL_BATADJTH_I_MAX
};

/* RTC correction indices */
enum {
	PM_RTC_ADJ = 0,
	PM_PW_CTRL_RTCADJ_I_MAX
};

/* Number of times to average load correction voltage indices */
enum {
	PM_ADC_DETERMINATION = 0,
	PM_ADC_GET_TIME,
	PM_ADC_PERIOD_WAKEUP,
	PM_PW_ADC_DETERMIN_I_MAX
};

/* Temperature threshold indices */
enum {
	PM_Ta1 = 0,
	PM_Ta2,
	PM_PW_CTRL_TEMP_BATADJ_I_MAX
};

/* Temperature abnormality threthold indices */
enum {
	PM_TEMP_ERR_LOW1 = 0,
	PM_TEMP_ERR_LOW2,
	PM_TEMP_ERR_HIGH1,
	PM_TEMP_ERR_HIGH2,
	PM_PW_CTRL_TEMP_ERR_I_MAX
};

/* Abort temperature threshold indices */
enum {
	PM_TEMP_RF = 0,
	PM_TEMP_BAT,
	PM_PW_TEST_TEMP_SET_STATUS_I_MAX
};

/* [Q89-PM-098] DEL */

/*
 * Indices of parameter for charge control by thermister temperature around XO
 * during camera operation
 */
enum {
	PM_XO_TEMP_ERR_HIGH2_CAM = 0,
	PM_XO_TEMP_ERR_HIGH1_CAM,
	PM_XO_TEMP_ERR_HIGH3_CAM,
	PM_CHARGE_OFF_ON_XO_TEMP_CAM_I_MAX
};

/*
 * Indices of parameter for charge control by thermister temperature around XO
 * during LTE communication
 */
enum {
	PM_XO_TEMP_ERR_HIGH2_LTE = 0,
	PM_XO_TEMP_ERR_HIGH1_LTE,
	PM_XO_TEMP_ERR_HIGH3_LTE,
	PM_CHARGE_OFF_ON_XO_TEMP_LTE_I_MAX
};

/* Indices of parameter for charge control by thermister temperature around XO */
enum {
	PM_XO_TEMP_ERR_HIGH2 = 0,
	PM_XO_TEMP_ERR_HIGH1,
	PM_XO_TEMP_ERR_HIGH3,
	PM_CHARGE_OFF_ON_XO_TEMP_I_MAX
};

/*
 * Indices of parameter for charge control by thermister temperature around XO
 * during multimedia broadcast
 */
enum {
	PM_XO_TEMP_ERR_HIGH2_MM = 0,
	PM_XO_TEMP_ERR_HIGH1_MM,
	PM_XO_TEMP_ERR_HIGH3_MM,
	PM_CHARGE_OFF_ON_XO_TEMP_MM_I_MAX
};

/* Charge voltage, current and timer setting indices */
enum {
	PM_I_CHG_HIGH1 = 0,
	PM_I_CHG_HIGH2,
	PM_V_CHG_HIGH1,
	PM_V_CHG_HIGH2,
	PM_V_CHG_VBATDET,
	PM_I_CHG_COMP,
	PM_TIMER_CHG_FC,
	PM_BATTERY_ERR_CYCLE,
	PM_CHG_SETTING_I_MAX
};


enum {
	PM_BAT_ALM_ON_OFF = 0,
	PM_DET_VOLT_UP_LOW,
	PM_CLOCK_DIVIDER,
	PM_CLOCK_SCALER,
	PM_MEAS_TIME,
	PM_BAT_ALM_CUT_OFF_I_MAX
};

enum {
	PM_LVA_OFF = 0,
	PM_LVA_OFF_I_MAX
};

/********************************************************************/
/* Data Type Definition                                             */
/********************************************************************/

typedef struct {
	uint16_t data[PM_PW_CTRL_VDET_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_vdet_type;

/* for userland */
typedef struct {
	uint16_t data[PM_PW_CTRL_DVDET_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_dvdet_type;

typedef struct {
	uint16_t data[PM_PW_CTRL_CT_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_ct_type;

/* for userland */
typedef struct {
	uint16_t data[PM_PW_CTRL_BATADJ_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_batadj_type;

/* for userland */
typedef struct {
	uint16_t data[PM_PW_CTRL_BATADJTH_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_batadjth_type;

typedef struct {
	uint8_t data[PM_PW_CTRL_RTCADJ_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_rtcadj_type;

/* for userland */
typedef struct {
	uint16_t data[PM_PW_ADC_DETERMIN_I_MAX];
} __attribute__ ((packed)) pm_pw_adc_determin_type;

/* for userland */
typedef struct {
	uint16_t data[PM_PW_CTRL_TEMP_BATADJ_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_temp_batadj_type;

typedef struct {
	uint16_t data[PM_PW_CTRL_TEMP_ERR_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_temp_err_type;

typedef struct {
	uint16_t data[PM_PW_TEST_TEMP_SET_STATUS_I_MAX];
} __attribute__ ((packed)) pm_pw_test_temp_set_status_type;



typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_CAM_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_cam_type;

typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_LTE_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_lte_type;

typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_type;


typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_MM_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_mm_type;


typedef struct {
	uint16_t data[PM_CHG_SETTING_I_MAX];
} __attribute__ ((packed)) pm_chg_setting_type;


typedef struct {
	uint16_t data[PM_BAT_ALM_CUT_OFF_I_MAX];
} __attribute__ ((packed)) pm_bat_alm_cut_off_type;

typedef struct {
	uint16_t data[PM_LVA_OFF_I_MAX];
} __attribute__ ((packed)) pm_lva_off_type;

struct pm_param_struct {
	pm_pw_ctrl_vdet_type				pm_pw_ctrl_vdet;
	pm_pw_ctrl_dvdet_type				pm_pw_ctrl_dvdet;
	pm_pw_ctrl_ct_type					pm_pw_ctrl_ct;
	pm_pw_ctrl_batadj_type				pm_pw_ctrl_batadj;
	pm_pw_ctrl_batadjth_type			pm_pw_ctrl_batadjth;
	pm_pw_ctrl_rtcadj_type				pm_pw_ctrl_rtcadj;
	pm_pw_adc_determin_type				pm_pw_adc_determin;
	pm_pw_ctrl_temp_batadj_type			pm_pw_ctrl_temp_batadj;
	pm_pw_ctrl_temp_err_type			pm_pw_ctrl_temp_err;
	pm_pw_test_temp_set_status_type		pm_pw_test_temp_set_status;

	pm_charge_off_on_xo_temp_cam_type	pm_charge_off_on_xo_temp_cam;
	pm_charge_off_on_xo_temp_lte_type	pm_charge_off_on_xo_temp_lte;
	pm_charge_off_on_xo_temp_type		pm_charge_off_on_xo_temp;
	pm_charge_off_on_xo_temp_mm_type	pm_charge_off_on_xo_temp_mm;
	pm_chg_setting_type					pm_chg_setting;

	pm_bat_alm_cut_off_type				pm_bat_alm_cut_off;
	pm_lva_off_type					pm_lva_off; 
};

extern struct pm_param_struct pm_param;

#endif /* __PM_PARAM_H__ */
