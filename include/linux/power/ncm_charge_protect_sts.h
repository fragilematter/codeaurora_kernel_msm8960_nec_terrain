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

#ifndef _NCM_CHARGE_PROTECT_STS_H_
#define _NCM_CHARGE_PROTECT_STS_H_

#define PROTECT_INDEX_MIN   (IDX_CHARGE_TIMEOUT)
#define PROTECT_INDEX_MAX   (IDX_XO_TEMP_LIMIT)

typedef enum 
{

    IDX_CHARGE_TIMEOUT = 0,
    IDX_DC_OVP,            
    IDX_USB_OVP,           
    IDX_USB_OCP,           
    IDX_BATT_TEMP_ERR,     
	IDX_XO_TEMP,           
    IDX_BATT_TEMP_LIMIT,   
	IDX_XO_TEMP_LIMIT,     
    IDX_FINALLY,           
    IDX_NORMAL = 0xFF      
} ProtectIndex;

typedef enum               
{
    ProtectNone = 0,       
    ProtectOn,             
	ProtectForceOff,       
    ProtectFinally         
} ProtectType;

typedef int (*protect_action_fnc )(ProtectIndex, ProtectType); 
typedef int (*protect_recover_fnc)(ProtectIndex, ProtectType); 

typedef struct
{
    int protect_active_cnt;
    int protect_recover_flg;
    protect_action_fnc  protect_action_fnc_p;
    protect_recover_fnc protect_recover_fnc_p;
} ncm_charge_protect_action;

extern void init_charge_protect(void);
extern int set_status(ProtectIndex, ProtectType);
extern int get_active_status(int *); 
extern int check_charge_protect_sts(void);
extern int getProtect_Status(ProtectIndex protect_index, int *protect_sts);
extern int is_protect_charge_stop(void);

#endif /* _NCM_CHARGE_PROTECTION_STS_H_ */
