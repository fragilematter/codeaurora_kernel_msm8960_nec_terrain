/* Copyright (C) 2012, NEC CASIO Mobile Communications. All rights reserved.
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

#ifndef CHGMON_CHG_H_
#define CHGMON_CHG_H_

enum CHGMON_PM_VAL_ID{
	CHGMON_PM_VAL_CHARGER = 0x00,
	CHGMON_PM_VAL_CHG_STATE = 0x01,
	CHGMON_PM_VAL_FSM_STATE = 0x02,
	CHGMON_PM_VAL_VBAT = 0x03,
	CHGMON_PM_VAL_VCHG = 0x04,
	CHGMON_PM_VAL_VMAX = 0x05,
	CHGMON_PM_VAL_IMAX = 0x06,
	CHGMON_PM_VAL_VBATDET = 0x07,
	CHGMON_PM_VAL_IBAT = 0x08,
	CHGMON_PM_VAL_VOUT = 0x09,
	CHGMON_PM_VAL_VIN = 0x0A, 
	CHGMON_PM_VAL_IUSB = 0x0B,
	CHGMON_PM_VAL_TBAT = 0x0C,
	CHGMON_PM_VAL_XOTH = 0x0D,
	CHGMON_PM_VAL_LOOP = 0x0E,
	CHGMON_PM_VAL_MAX_NUM,
};


enum CHGMON_PM_CHG_TIME_ID{
	CHGMON_PM_TIME_RTC = 0x00,
};

enum CHGMON_PM_CHG_STATE_ID{
	CHGMON_PM_CHG_STATE_OFF = 0x00,
	CHGMON_PM_CHG_STATE_CC       = 0x01,
	CHGMON_PM_CHG_STATE_CC_H     = 0x02,
	CHGMON_PM_CHG_STATE_CV       = 0x03,
	CHGMON_PM_CHG_STATE_CV_H     = 0x04,
	CHGMON_PM_CHG_STATE_FULL     = 0x05,
	CHGMON_PM_CHG_STATE_BAT_ERR  = 0x06,
	CHGMON_PM_CHG_STATE_OVP      = 0x07,
	CHGMON_PM_CHG_STATE_OCP      = 0x08,
	CHGMON_PM_CHG_STATE_NO_BATT  = 0x09,
	CHGMON_PM_CHG_STATE_TEMP_FST = 0x0A,
	CHGMON_PM_CHG_STATE_MAX_NUM,
} ;


enum CHGMON_PM_CHG_CHARGER_ID{
	CHGMON_PM_CHG_CHARGER_NONE = 0x00,
	CHGMON_PM_CHG_CHARGER_Other = 0x01,
	CHGMON_PM_CHG_CHARGER_DCP = 0x02, 
	CHGMON_PM_CHG_CHARGER_SDP = 0x03, 

	CHGMON_PM_CHG_CHARGER_Other_DCP = 0x04,
	CHGMON_PM_CHG_CHARGER_Sub_Charger = 0x05,
	CHGMON_PM_CHG_CHARGER_NO_USB_SW = 0x06,
	CHGMON_PM_CHARGER_MAX_NUM,
} ;

#endif /* CHGMON_CHG_H_ */
