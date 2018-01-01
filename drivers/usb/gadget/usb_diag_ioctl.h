#ifdef CONFIG_FEATURE_NCMC_USB
/*
 * drivers/usb/gadget/usb_diag_ioctl.h
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


#ifndef _USB_DIAG_H_
#define _USB_DIAG_H_

#include <linux/ioctl.h>

#define IOC_MAGIC_USB_DIAG 'u'      /* usb_diag_driver */

#define DEVICE_COUNTS          1    /* counts of reserve to use (minor number) */
#define USB_DEV_MAJOR          0    /* default major number */
#define USB_DEV_MINOR          0    /* default minor number */

#define USB_DIAG_START_POLLING           0x00                  /* start polling detect Earphone                      */
#define USB_DIAG_STOP_POLLING            0x11                  /* stop polling detect Earphone                       */
#define USB_DIAG_POLLING_INTERVAL_TIME     10                  /* poling interval time 0.1sec(jiffies:1jiffies=10ms) */

#define IOCTL_USB_DIAG_FS_ENUM          _IOR(IOC_MAGIC_USB_DIAG, 1, char)       /* ioctl command */
#define IOCTL_USB_DIAG_HS_ENUM          _IOR(IOC_MAGIC_USB_DIAG, 3, char)       /* ioctl command */
#define IOCTL_USB_DIAG_READ_REG         _IOWR(IOC_MAGIC_USB_DIAG, 5, char)      /* ioctl command */
#define IOCTL_USB_DIAG_WRITE_REG        _IOWR(IOC_MAGIC_USB_DIAG, 6, char)      /* ioctl command */
#define IOCTL_USB_DIAG_TRANSIT_FS       _IOR(IOC_MAGIC_USB_DIAG, 7, char)       /* ioctl command */
#define IOCTL_USB_DIAG_TRANSIT_HS       _IOR(IOC_MAGIC_USB_DIAG, 8, char)       /* ioctl command */
#define IOCTL_USB_DIAG_DETECT_EARPHONE  _IOWR(IOC_MAGIC_USB_DIAG, 100, char)    /* ioctl command */

#define USB_DIAG_CMD_RET_OK                    0        /* return value of diag command(OK)     */
#define USB_DIAG_CMD_RET_NG                   -1        /* return value of diag command(NG)     */
#define USB_DIAG_RSP_RET_OK                 0x00        /* return parameter of diag command(OK) */
#define USB_DIAG_RSP_RET_NG                 0x01        /* return parameter of diag command(NG) */

/* USB driver release interface */
#define USB_DETECT_EARPHONE_SWITCH_START 0x01           /* start request detect earphone switch */
#define USB_DETECT_EARPHONE_SWITCH_STOP  0x00           /* stop request detect earphone switch  */

#define IOCTL_USB_GET_HEADSET_INFO           _IOR(IOC_MAGIC_USB_DIAG, 200, char)  /* ioctl command(USB headset)           */
#define IOCTL_USB_SET_DETECT_EARPHONE_SWITCH _IOWR(IOC_MAGIC_USB_DIAG, 201, char) /* ioctl command(earphone SEND/END key) */
#define IOCTL_USB_SET_MIC_DEC_WAITTIME       _IOWR(IOC_MAGIC_USB_DIAG, 202, char) /* ioctl command(wait time for Mic)     */

typedef enum
{
    USB_DIAG_NOT_AUDIO_CONNECTION,       /* USB-Audio No earphone                     */
    USB_DIAG_AUDIO_STEREO_CONNECTION,    /* USB-Audio Stereo earphone                 */
    USB_DIAG_AUDIO_MONO_CONNECTION,      /* USB-Audio Monaural earphone               */
    USB_DIAG_AUDIO_MONO_CHG_CONNECTION,  /* USB-Audio Monaural earphone with charging */
}usb_diag_connected_audio_device_enum;

struct usb_diag_ioctl_cmd {
    char rsp_data[1];
};

struct ioctl_cmd {
    char rsp_data[1];
};
#endif /* _USB_DIAG_H_ */
#endif /* CONFIG_FEATURE_NCMC_USB */
