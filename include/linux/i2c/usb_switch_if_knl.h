#ifndef USB_SWITCH_IF_KNL_H
#define USB_SWITCH_IF_KNL_H
/*
 * include/linux/i2c/usb_switch_if_knl.h
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
#define USB_SW_OK 0
#define USB_SW_NG -1


#define USB_SW_STATE_NORMAL 0x00
#define USB_SW_STATE_OVP_VC 0x01
#define USB_SW_STATE_OVP_VB 0x02
#define USB_SW_STATE_OCP_VC 0x04
#define USB_SW_STATE_OCP_VB 0x08

typedef enum{
    USB_SW_DEVICE_DISCONNECTED = 0,
    USB_SW_SDP_CONNECTED,          
    USB_SW_DCP_CONNECTED,          
    USB_SW_OTHER_DCP_CONNECTED,    
    USB_SW_OTHER_DEVICE_CONNECTED  
}usb_sw_device_state_enum;

/***************************/
/* prototype declaration   */
/***************************/
int usb_sw_get_current_voltage_state( unsigned char* current_voltage_state_p );    /* Get a current power and a voltage */
int usb_sw_get_device_state( usb_sw_device_state_enum* device_state_p );           /* Get a USB device state */

void usb_sw_path_initialize( void );                                               /* Initialize AudioMIC path setting */


#endif /* #ifndef USB_SWITCH_IF_KNL_H */
