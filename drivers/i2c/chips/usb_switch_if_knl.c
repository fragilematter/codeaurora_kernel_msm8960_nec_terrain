/*
 * include/linux/i2c/usb_switch_if_knl.c
 *
 * (C) NEC CASIO Mobile Communications, Ltd. 2011
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
 */


#include <linux/kernel.h>
#ifndef CONFIG_FEATURE_NCMC_RUBY
#include "linux/i2c/bd91401gw.h"
#endif /* !CONFIG_FEATURE_NCMC_RUBY */
#include "linux/i2c/usb_switch_if_knl.h"

extern int bd91401gw_get_current_voltage_state( unsigned char* current_voltage_state_p );
extern int bd91401gw_get_device_state( usb_sw_device_state_enum* device_state_p );
extern void bd91401gw_path_initialize_again_wrapper(void);
/****************************************************************************
@function   : usb_sw_get_current_voltage_state
****************************************************************************/
int usb_sw_get_current_voltage_state( unsigned char* current_voltage_state_p )
{
#ifdef CONFIG_FEATURE_NCMC_RUBY
    printk(KERN_ERR "LINE:%u %s Error USB-SW is non-support in MANTA.\n",__LINE__,__func__);
    return USB_SW_NG;
#else /* CONFIG_FEATURE_NCMC_RUBY */
    int retVal = USB_SW_NG;
    
    if( current_voltage_state_p == NULL )
    {
        return USB_SW_NG;
    }

    retVal = bd91401gw_get_current_voltage_state( current_voltage_state_p );

    if( retVal == BD91401GW_NG )
    {
        return USB_SW_NG;
    }
    
    return USB_SW_OK;
#endif /* CONFIG_FEATURE_NCMC_RUBY */
}

/****************************************************************************
@function   : usb_sw_get_device_state
****************************************************************************/
int usb_sw_get_device_state( usb_sw_device_state_enum* device_state_p )
{
#ifdef CONFIG_FEATURE_NCMC_RUBY
    printk(KERN_ERR "LINE:%u %s Error USB-SW is non-support in MANTA.\n",__LINE__,__func__);
    return USB_SW_NG;
#else /* CONFIG_FEATURE_NCMC_RUBY */
    int retVal = USB_SW_NG;
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);

    if( device_state_p == NULL )
    {
        return USB_SW_NG;
    }
    retVal = bd91401gw_get_device_state( device_state_p );
    printk(KERN_DEBUG "[USBSW]LINE:%u %s usb_sw_get_device_state(0x%x) called\n",__LINE__,__func__,*device_state_p);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s retVal:%d\n",__LINE__,__func__,retVal);

    if( retVal == BD91401GW_NG )
    {
        printk(KERN_DEBUG "[USBSW]LINE:%u %s return value error\n",__LINE__,__func__);
        return USB_SW_NG;
    }
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s end\n",__LINE__,__func__);
    return USB_SW_OK;
#endif /* CONFIG_FEATURE_NCMC_RUBY */
}

/****************************************************************************
@function   : usb_sw_path_initialize
@description: It initializes AudioMIC path setting
@parameters : none
@returns    : none
@note       : It will start to initialize AudioMIC path setting,
              when the function is called.
****************************************************************************/
void usb_sw_path_initialize(void)
{
#ifdef CONFIG_FEATURE_NCMC_RUBY
    printk(KERN_ERR "[USBSW]LINE:%u %s Error USB-SW is non-support in MANTA.\n",__LINE__,__func__);
    return;
#else /* CONFIG_FEATURE_NCMC_RUBY */
    bd91401gw_path_initialize_again_wrapper();
    
    return;
#endif /* CONFIG_FEATURE_NCMC_RUBY */
}
