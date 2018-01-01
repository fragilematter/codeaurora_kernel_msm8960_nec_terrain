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

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/mfd/pm8xxx/pm8921-charger.h>
#include <linux/power/ncm_charge_protect_sts.h>

int set_status(ProtectIndex, ProtectType);
int get_active_status(int *);             
int check_charge_protect_sts(void);       

static int setProtect_active_cnt (ProtectIndex, int);  
static int getProtect_active_cnt (ProtectIndex, int *);
static int setProtect_recover_flg(ProtectIndex, int);  
static int getProtect_recover_flg(ProtectIndex, int *);

extern int ncm_protect_action_Batt_Remove   (ProtectIndex, ProtectType);
extern int ncm_protect_action_DC_Ovp        (ProtectIndex, ProtectType);
extern int ncm_protect_action_Usb_Ovp       (ProtectIndex, ProtectType);
extern int ncm_protect_action_Usb_Ocp       (ProtectIndex, ProtectType);
extern int ncm_protect_action_Batt_TempErr  (ProtectIndex, ProtectType);
extern int ncm_protect_action_Batt_TempLimit(ProtectIndex, ProtectType);
extern int ncm_protect_action_XO_Temp       (ProtectIndex, ProtectType);
extern int ncm_protect_action_XO_TempLimit  (ProtectIndex, ProtectType);
extern int ncm_protect_action_Charge_Timeout(ProtectIndex, ProtectType);
extern int ncm_protect_recover_Batt_Remove   (ProtectIndex, ProtectType);
extern int ncm_protect_recover_DC_Ovp        (ProtectIndex, ProtectType);
extern int ncm_protect_recover_Usb_Ovp       (ProtectIndex, ProtectType);
extern int ncm_protect_recover_Usb_Ocp       (ProtectIndex, ProtectType);
extern int ncm_protect_recover_Batt_TempErr  (ProtectIndex, ProtectType);
extern int ncm_protect_recover_Batt_TempLimit(ProtectIndex, ProtectType);
extern int ncm_protect_recover_XO_Temp       (ProtectIndex, ProtectType);
extern int ncm_protect_recover_XO_TempLimit  (ProtectIndex, ProtectType);
extern int ncm_protect_recover_Charge_Timeout(ProtectIndex, ProtectType);

static int active_sts;
static ncm_charge_protect_action ChageProtect_Sts[] =
    {
        {0, 0, ncm_protect_action_Charge_Timeout, ncm_protect_recover_Charge_Timeout},
        {0, 0, ncm_protect_action_DC_Ovp        , ncm_protect_recover_DC_Ovp        },
        {0, 0, ncm_protect_action_Usb_Ovp       , ncm_protect_recover_Usb_Ovp       },
        {0, 0, ncm_protect_action_Usb_Ocp       , ncm_protect_recover_Usb_Ocp       },
        {0, 0, ncm_protect_action_Batt_TempErr  , ncm_protect_recover_Batt_TempErr  },
        {0, 0, ncm_protect_action_XO_Temp       , ncm_protect_recover_XO_Temp       },
        {0, 0, ncm_protect_action_Batt_TempLimit, ncm_protect_recover_Batt_TempLimit},
    	{0, 0, ncm_protect_action_XO_TempLimit  , ncm_protect_recover_XO_TempLimit  },
    };
static spinlock_t spinlock_handler;
static unsigned long spinlock_flags;

#define PROTECT_INDEXS  (sizeof(ChageProtect_Sts) / sizeof(ncm_charge_protect_action))

void init_charge_protect(void)
{
    active_sts = IDX_NORMAL;
    spin_lock_init(&spinlock_handler);
}

int set_status(ProtectIndex protect_index, ProtectType protect_type)
{
    int protect_active_cnt = 0;

    if ((protect_index < PROTECT_INDEX_MIN) ||
        (protect_index > PROTECT_INDEX_MAX))
    {
        printk(KERN_ERR "[PM] %s:Illegal ProtectIndex:%d\n", __func__, protect_index);
        return false;
    }

    switch (protect_type)
    {
        case ProtectOn : 
            setProtect_active_cnt(protect_index, 1);
            break;

        case ProtectForceOff:
            setProtect_active_cnt(protect_index, 0);
            break;
        case ProtectNone : 
            if (getProtect_active_cnt(protect_index, &protect_active_cnt))
            {
                if (protect_active_cnt != 0)
                {
                    setProtect_recover_flg(protect_index, 1);
                }
            }
            break;

        default :
            printk(KERN_ERR "[PM] %s:Illegal ProtectType:%d\n", __func__, protect_type);
            return false;
    }

    return true;
}


int get_active_status(int *active_status)
{
    if (active_status == NULL)
    {
        printk(KERN_ERR "[PM] %s:Null Pointer Error: active_status\n", __func__);
        return false;
    }
    spin_lock_irqsave(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock in :active_status=%d\n", __func__, *active_status);

    *active_status = active_sts;

    spin_unlock_irqrestore(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock out:active_status=%d\n", __func__, *active_status);
    return true;
}


int check_charge_protect_sts(void)
{
    int protect_index;
    int protect_active_cnt = 0;
    int protect_recover_flg;
    int answer;
    protect_action_fnc  protect_action_fnc_p;
    protect_recover_fnc protect_recover_fnc_p;

    for (protect_index = 0; protect_index < PROTECT_INDEXS; protect_index++)
    {
        getProtect_recover_flg(protect_index, &protect_recover_flg);
        if (protect_recover_flg != 0)
        {
            protect_recover_fnc_p = ChageProtect_Sts[protect_index].protect_recover_fnc_p;
            if(protect_recover_fnc_p == NULL)
            {
                answer = true;
            }
            else
            {
                answer = (*protect_recover_fnc_p)(protect_index, ProtectOn);
            }

            setProtect_recover_flg(protect_index, 0);

            if (answer)
            {
                setProtect_active_cnt(protect_index, 0);
        		if( active_sts == protect_index){
        			active_sts = IDX_NORMAL;
        		}
        		else {
        			/* DO_NOTHING */
        		}
            }
        }

        getProtect_active_cnt(protect_index, &protect_active_cnt);
        if (protect_active_cnt == 1)
        {
            protect_action_fnc_p = ChageProtect_Sts[protect_index].protect_action_fnc_p;
            if (protect_action_fnc_p != NULL)
            {
                answer = (*protect_action_fnc_p)(protect_index, ProtectOn);
                if (answer)
                {
                    setProtect_active_cnt(protect_index, 1);
                }
            }
        	if( active_sts == IDX_NORMAL){
        		active_sts = protect_index;
        	}
        	else {
        		if( active_sts > protect_index){
        			active_sts = protect_index;
        		}
        		else {
        			/* DO_NOTHING */
        		}
        	}
        }
        else if(protect_active_cnt > 1)
        {
            pr_debug("[PM] %s:active_sts:%d is continued. protect_active_cnt:%d\n", __func__, active_sts, protect_active_cnt);
            if( active_sts == IDX_NORMAL){
				active_sts = protect_index;
			}
			else {
				/* DO_NOTHING */
			}
        }
        else if(protect_active_cnt == 0)
        {
			if(protect_index == active_sts){
				active_sts = IDX_NORMAL;
			}
        }
		else {
			/* nothing. */
		}
    }

    return true;
}

static int setProtect_active_cnt(ProtectIndex protect_index, int protect_active_cnt)
{

    spin_lock_irqsave(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock in :idx=%d:protect_active_cnt =%d\n", __func__, protect_index, ChageProtect_Sts[protect_index].protect_active_cnt);

    if (protect_active_cnt == 0)
    {
        ChageProtect_Sts[protect_index].protect_active_cnt = 0;
    }
    else
    {
        ChageProtect_Sts[protect_index].protect_active_cnt++;
    }

    spin_unlock_irqrestore(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock out:idx=%d:protect_active_cnt =%d\n", __func__, protect_index, ChageProtect_Sts[protect_index].protect_active_cnt);

    return true;
}


static int getProtect_active_cnt(ProtectIndex protect_index, int *protect_active_cnt)
{
    if (protect_active_cnt == NULL)
    {
        printk(KERN_ERR "[PM] %s:Null Pointer Error: protect_active_cnt\n", __func__);
        return false;
    }

    spin_lock_irqsave(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock in :idx=%d:protect_active_cnt =%d\n", __func__, protect_index, *protect_active_cnt);

    *protect_active_cnt = ChageProtect_Sts[protect_index].protect_active_cnt;

    spin_unlock_irqrestore(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock out:idx=%d:protect_active_cnt =%d\n", __func__, protect_index, *protect_active_cnt);

    return true;
}


static int setProtect_recover_flg(ProtectIndex protect_index, int protect_recover_flg)
{

    spin_lock_irqsave(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock in :idx=%d:protect_recover_flg=%d\n", __func__, protect_index, ChageProtect_Sts[protect_index].protect_recover_flg);

    if (protect_recover_flg == 0)
    {
        ChageProtect_Sts[protect_index].protect_recover_flg = 0;
    }
    else
    {
        ChageProtect_Sts[protect_index].protect_recover_flg = 1;
    }

    spin_unlock_irqrestore(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock out:idx=%d:protect_recover_flg=%d\n", __func__, protect_index, ChageProtect_Sts[protect_index].protect_recover_flg);

    return true;
}


static int getProtect_recover_flg(ProtectIndex protect_index, int *protect_recover_flg)
{
    if (protect_recover_flg == NULL)
    {
        printk(KERN_ERR "[PM] %s:Null Pointer Error: protect_recover_flg\n", __func__);
        return false;
    }

    spin_lock_irqsave(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock in :idx=%d:protect_recover_flg=%d\n", __func__, protect_index, ChageProtect_Sts[protect_index].protect_recover_flg);

    *protect_recover_flg = ChageProtect_Sts[protect_index].protect_recover_flg;

    spin_unlock_irqrestore(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock out:idx=%d:protect_recover_flg=%d\n", __func__, protect_index, *protect_recover_flg);

    return true;
}



int getProtect_Status(ProtectIndex protect_index, int *protect_sts)
{
    if (protect_sts == NULL)
    {
        printk(KERN_ERR "[PM] %s:Null Pointer Error: protect_sts\n", __func__);
        return false;
    }

    spin_lock_irqsave(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock in :idx=%d:protect_sts =%d\n", __func__, protect_index, *protect_sts);

    *protect_sts = ChageProtect_Sts[protect_index].protect_active_cnt;

    spin_unlock_irqrestore(&spinlock_handler, spinlock_flags);
    pr_debug("[PM] %s:spin_lock out:idx=%d:protect_sts =%d\n", __func__, protect_index, *protect_sts);

    return true;
}

int is_protect_charge_stop(void)
{
    int status;

    spin_lock_irqsave(&spinlock_handler, spinlock_flags);

    status = ChageProtect_Sts[IDX_CHARGE_TIMEOUT].protect_active_cnt |
             ChageProtect_Sts[IDX_BATT_TEMP_ERR].protect_active_cnt  |
             ChageProtect_Sts[IDX_XO_TEMP].protect_active_cnt;

    spin_unlock_irqrestore(&spinlock_handler, spinlock_flags);

    return status;
}
