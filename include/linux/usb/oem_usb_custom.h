#ifdef CONFIG_FEATURE_NCMC_USB
/*
 * include/linux/usb/oem_usb_custom.h
 *
 * (C) NEC CASIO Mobile Communications, Ltd. 2012
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
 
#ifndef __LINUX_USB_OEM_USB_CUSTOM_H
#define __LINUX_USB_OEM_USB_CUSTOM_H

#include <linux/usb/oem_usb_common.h>

#ifdef NCMC_USB_FUNCTION_NCMC_SERIAL
enum usb_ncmc_serial_mode_status {
	USB_NCMC_SERIAL_MODE_OFF = 0,
    USB_NCMC_SERIAL_MODE_ON,
};

enum usb_register_result {
	REGISTER_RESULT_OK = 0,
    REGISTER_RESULT_NG,
	REGISTER_RESULT_ALREADY,
};
#endif /* NCMC_USB_FUNCTION_NCMC_SERIAL */


#ifdef NCMC_USB_FUNCTION_NCMC_SERIAL
/************************************************************************
 * Module    : oem_usb_regsiter_ncmc_serial_notify                      *
 * Create    : 2011.08.13                                               *
 ************************************************************************/
extern enum  usb_register_result oem_usb_regsiter_ncmc_serial_notify( void* cb );
#endif /* NCMC_USB_FUNCTION_NCMC_SERIAL */

#endif /* __LINUX_USB_OEM_USB_CUSTOM_H */
#endif /* CONFIG_FEATURE_NCMC_USB */
