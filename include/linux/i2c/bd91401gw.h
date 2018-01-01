#ifndef CONFIG_FEATURE_NCMC_RUBY
#ifndef BD91401GW_H
#define BD91401GW_H
/*
 * include/linux/i2c/bd91401gw.h
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


/* Header File */
#include <linux/i2c.h>
#include <linux/ioctl.h>
#include <linux/irqreturn.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/msm_hsusb.h>

/* Regist Devices */
#define BD91401GW_I2C_DEVICE_NAME  ("bd91401gw i2c drv")       /* device name */
#define BD91401GW_I2C_SLAVE_ADDRESS (0x6A)                     /* slave address */
#define BD91401GW_I2C_PM_GPIO_INTB 37                          /* PM-GPIO[37] */

/* BD91401GW Definition about ERROR */
#define BD91401GW_OK 0    /* OK */
#define BD91401GW_NG -1   /* NG */

/* I2C Driver Definition about registration value */
#define BD91401GW_I2C_TRANSFER_OK 0     /* OK */
#define BD91401GW_I2C_TRANSFER_NG -1    /* NG */

/* BD91401GW Register Address */
#define BD91401GW_UCDCNT_ADDRESS            (0x02)    /* UCDCNT     */
#define BD91401GW_SWCONTROL_ADDRESS         (0x03)    /* SW Control */
#define BD91401GW_INTERRUPT_ADDRESS         (0x05)    /* INT_STA    */
#define BD91401GW_STATUS_ADDRESS            (0x06)    /* STATUS     */
#define BD91401GW_IDSTATUS_ADDRESS          (0x07)    /* ID_STA     */
#define BD91401GW_SOFTRESET_ADDRESS         (0x08)    /* S_RST      */

/* BD91401GW Initial Value */
#define BD91401GW_UCDCNT_RESET_VALUE        (0x50)    /* UCDCNT initial value */
#define BD91401GW_SOFTRESET_RESET_VALUE     (0x01)    /* SW reset value */
                          
/* BD91401GW Register Mask Bit */
#define BD91401GW_SWCONTROL_MASKB           (0xbf)    /* SW Control Mask */
#define BD91401GW_INTERRUPT_MASKB           (0xf0)    /* INT_STA Mask */
#define BD91401GW_STATUS_MASKB              (0xef)    /* STATUS Mask*/
#define BD91401GW_STATUS_CHGPORT_MASKB      (0xe0)    /* STATUS CHGPORT Mask*/
#define BD91401GW_STATUS_OVPOCP_MASKB       (0x0f)    /* STATUS OVPOCP Mask */
#define BD91401GW_STATUS_OVPOCP_VB_MASKB    (0x0A)    /* STATUS VBOVPOCP Mask */
#define BD91401GW_IDSTATUS_INDO_MASKB       (0x0f)    /* ID_STA INDO Mask */
#define BD91401GW_IDSTATUS_MASKB            (0x1f)    /* ID_STA Mask */

/* INT_STA Register Judgment bit */                                                    
#define BD91401GW_INTERRUPT_TYPE_DEFAULT    (0x00)    /* Default Status */
#define BD91401GW_INTERRUPT_TYPE_VBUSDET    (0x10)    /* Detect VBUS OUT*/
#define BD91401GW_INTERRUPT_TYPE_COMPL      (0x20)    /* Detect SENT/END KEY SW */
#define BD91401GW_INTERRUPT_TYPE_COMPH      (0x40)    /* Detect ID pull-down */
#define BD91401GW_INTERRUPT_TYPE_CHGDET     (0x80)    /* Detect USBCHGDET */

/* STATUS Register judgment bit */
#define BD91401GW_STATUS_TYPE_OVPVC         (0x01)    /* OVP_VC bit            */
#define BD91401GW_STATUS_TYPE_OVPVB         (0x02)    /* OVP_VB bit            */
#define BD91401GW_STATUS_TYPE_OCPVC         (0x04)    /* OCP_VC bit            */
#define BD91401GW_STATUS_TYPE_OCPVB         (0x08)    /* OCP_VB bit            */
#define BD91401GW_STATUS_TYPE_CHGPORT_NOUSB (0x00)    /* No USB Port bit       */
#define BD91401GW_STATUS_TYPE_CHGPORT_SDP   (0x20)    /* SDP(HOST-PC) bit      */
#define BD91401GW_STATUS_TYPE_CHGPORT_CDP   (0x40)    /* CDP(other-charge) bit */
#define BD91401GW_STATUS_TYPE_CHGPORT_DCP   (0x60)    /* DCP(AC charger) bit   */
#define BD91401GW_STATUS_TYPE_CHGPORT_IRREGULAR  (0x80) /* STATUS register Irregular/other Impossible-Judgment bit */
#define BD91401GW_STATUS_NOT_OVP_OCP             (0x00) /* STATUS register OVP/OCP judgment bit                    */

/* ID_STA Register judgment bit */
#define BD91401GW_IDSTATUS_TYPE_STEREO_EAR  (0x18)    /* Streo Earphone (with Switch /without Switch) bit   */
#define BD91401GW_IDSTATUS_TYPE_MONO_EAR    (0x1c)    /* Monoral Earphone (with Switch /without Switch) bit */
#define BD91401GW_IDSTATUS_TYPE_CHG_EAR     (0x1b)    /* Monoral Earphone (with Charger) bit                */
#define BD91401GW_IDSTATUS_TYPE_UART        (0x16)    /* UART OUT bit                                       */
#define BD91401GW_IDSTATUS_TYPE_DEFAULT     (0x0d)    /* Default status                                     */
#define BD91401GW_IDSTATUS_TYPE_HOOKSW      (0x12)    /* HOOK Switch bit                                    */
#define BD91401GW_IDSTATUS_TYPE_OTGHOST     (0x10)    /* USB OTG HOST bit                                   */
#define BD91401GW_IDSTATUS_TYPE_WRONG       (0x1d) 

/* SW Control Register judgment bit */
#define BD91401GW_SWCONTROL_EAR             (0x12)    /* Earphone judgment/setting bit */
#define BD91401GW_SWCONTROL_EAR_MIC         (0x92)    /* Earphone Mic judgment/setting bit */
#define BD91401GW_SWCONTROL_FEED_POW_EAR    (0x12)    /* Streo/Monoral Earphone (with Charger) judgment/setting bit */
#define BD91401GW_SWCONTROL_CHG_EAR         (0x1a)    /* Monoral Earphone (with Charger) judgment/setting bit */
#define BD91401GW_SWCONTROL_UART            (0x09)    /* UART judgment/setting bit */
#define BD91401GW_SWCONTROL_DEFAULT         (0x00)    /* Default judgment/setting bit */
#define BD91401GW_SWCONTROL_MIC_CHECK       (0x80)    /* Mic path setting bit (mode-1,2) VB-MICOUT */
#define BD91401GW_SWCONTROL_MIC_CHECK_MODE3 (0x18)    /* Mic path setting bit(mode-3) DPRXR-MICOUT */
#define BD91401GW_SWCONTROL_UNCONNECTION    (0x3f)    /* UNCONNECTION bit*/

/* Definition about WRITE/READ retry */
#define BD91401GW_RETRY_READ_REG_COUNT    10    /* Read retry */
#define BD91401GW_RETRY_WRITE_REG_COUNT    5    /* Write retry */

/* Definition about WAIT time(ms) */
#define BD91401GW_I2C_READ_SLEEP_TIME  10       /* read register */
#define BD91401GW_I2C_WRITE_SLEEP_TIME 10       /* write register */
#define BD91401GW_IDSTATUS_REG_READ_SLEEP_TIME 1   


/* Definition about register message */
#define BD91401GW_I2C_MSG_WRITE_NUMBER 1        /* read register */
#define BD91401GW_I2C_MSG_READ_NUMBER  2        /* write register */

#define BD91401GW_MSM_GPIO_LOW_VALUE 0         /* LOW setting */
#define BD91401GW_MSM_GPIO_HIGH_VALUE 1        /* HIGH setting */
#define BD91401GW_MSM_GPIO_USBSW_RESET_ID 68   /* USB-SW RESET ID : GPIO[68] */

/* polling interval time for detect SEND/END key switch */
#define BD91401GW_SENDEND_KEY_POLLING_INTERVAL_TIME           10    /* interval[jiffies] 100ms */
/* interval time for notify Mic */
#define BD91401GW_MIC_NOTICE_INTERVAL_TIME                    30    /* interval[jiffies] 300ms */

/* Initial Platform Data */
struct bd91401gw_platform_data_struct {
    u32  (*bd91401gw_setup)(void) ;       /* setup function */
    void (*bd91401gw_shutdown)(void) ;    /* shutdown function */
};

/* I2C Alarm History */
#define BD91401GW_ALERM_EVENT 0x50
#define BD91401GW_I2C_ALERM_EVENT 0x67
#define BD91401GW_ALERM_INFO_I2C_READ     0x80
#define BD91401GW_ALERM_INFO_I2C_WRITE    0x81

/* I2C Alarm History Information */
#define BD91401GW_I2C_READ_LEN             (0x01)
#define BD91401GW_I2C_WRITE_LEN            (0x02)

#define BD91401GW_I2C_COM_FORMAT_SEND      (0x01)    /* send format */
#define BD91401GW_I2C_COM_FORMAT_RECV      (0x02)    /* receive format */
#define BD91401GW_I2C_COM_FORMAT_COMP      (BD91401GW_I2C_COM_FORMAT_SEND | BD91401GW_I2C_COM_FORMAT_RECV)    /* complex format */

#define BD91401GW_I2C_DATA_DISABLE         (0x00)    /* data ignore */
#define BD91401GW_I2C_DATA_ENABLE_1BYTE    (0x04)    /* 1 Byte Enable */
#define BD91401GW_I2C_DATA_ENABLE_2BYTE    (0x08)    /* 2 Byte Enable */

/* VBUS modes  */
enum usb_vbus_mode {
	USB_VBUS_INIT = -1,     /* VBUS is initial state */
	USB_VBUS_ON   =  1,     /* VBUS is online.       */
	USB_VBUS_OFF  =  0,     /* VBUS is offline.      */
};

/* ENUM Definition about USB headset  */
typedef enum{
    USB_HEADSET_STEREO = 0, /* Streo device */
    USB_HEADSET_MONO,       /* Monoral device */
}usb_headset_stereo_mono_enum;

/* Structure about USB headset info */
typedef struct{
    usb_headset_stereo_mono_enum stereo_mono_type;
}usb_headset_info_type;

enum chg_type bd91401gw_get_chg_type(void);


#endif /* #ifndef BD91401GW_H */

#endif /* !CONFIG_FEATURE_NCMC_RUBY */
