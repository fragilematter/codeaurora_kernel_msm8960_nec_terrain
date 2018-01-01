/*
 * leds-diag.h
 *
 * Copyright (C) 2011 NEC NEC Corporation
 *
 */

#ifndef __LINUX_LEDS_IAG_WRAPPER_H
#define __LINUX_LEDS_IAG_WRAPPER_H

#define LED_DIAG_WRAPPER_FD_PATH    "/dev/led_diag_wrapper"

#define LED_DIAG_IOCTL_OK   0
#define LED_DIAG_IOCTL_NG   1

#define LED_DIAG_IOCTL_01  0x10
#define LED_DIAG_IOCTL_02  0x11
#define LED_DIAG_IOCTL_03  0x12
#define LED_DIAG_IOCTL_04  0x13
#define LED_DIAG_IOCTL_05  0x14
#define LED_DIAG_IOCTL_06  0x15
#define LED_DIAG_IOCTL_07  0x16
#define LED_DIAG_IOCTL_08  0x17
#define LED_DIAG_IOCTL_09  0x18
#define LED_DIAG_IOCTL_10  0x19
#define LED_DIAG_IOCTL_11  0x1A
#define LED_DIAG_IOCTL_12  0x1B
#define LED_DIAG_IOCTL_13  0x1C
#define LED_DIAG_IOCTL_14  0x1D
#define LED_DIAG_IOCTL_15  0x1E
#define LED_DIAG_IOCTL_16  0x1F
#define LED_DIAG_IOCTL_17  0x20
#define LED_DIAG_IOCTL_18  0x21
#define LED_DIAG_IOCTL_19  0x22
#define LED_DIAG_IOCTL_20  0x23
#define LED_DIAG_IOCTL_21  0x24
#define LED_DIAG_IOCTL_22  0x25
#define LED_DIAG_IOCTL_23  0x26
#define LED_DIAG_IOCTL_24  0x27
#define LED_DIAG_IOCTL_25  0x28
#define LED_DIAG_IOCTL_26  0x29
#define LED_DIAG_IOCTL_27  0x2A
#define LED_DIAG_IOCTL_28  0x2B
#define LED_DIAG_IOCTL_29  0x2C
#define LED_DIAG_IOCTL_30  0x2D
#define LED_DIAG_IOCTL_31  0x2E
#define LED_DIAG_IOCTL_32  0x2F
#define LED_DIAG_IOCTL_33  0x30
#define LED_DIAG_IOCTL_34  0x31
#define LED_DIAG_IOCTL_35  0x32
#define LED_DIAG_IOCTL_36  0x33
#define LED_DIAG_IOCTL_37  0x34
#define LED_DIAG_IOCTL_38  0x35
#define LED_DIAG_IOCTL_39  0x36
#define LED_DIAG_IOCTL_40  0x37
#define LED_DIAG_IOCTL_41  0x38
#define LED_DIAG_IOCTL_42  0x39
#define LED_DIAG_IOCTL_43  0x3A
#define LED_DIAG_IOCTL_44  0x3B
#define LED_DIAG_IOCTL_45  0x3C
#define LED_DIAG_IOCTL_46  0x3D
#define LED_DIAG_IOCTL_47  0x3E
#define LED_DIAG_IOCTL_48  0x3F
#define LED_DIAG_IOCTL_49  0x40
#define LED_DIAG_IOCTL_50  0x41

#define LED_DIAG_IOCTL_90  0x90
#define LED_DIAG_IOCTL_91  0x91
#define LED_DIAG_IOCTL_92  0x92
#define LED_DIAG_IOCTL_93  0x93
#define LED_DIAG_IOCTL_94  0x94
#define LED_DIAG_IOCTL_95  0x95
#define LED_DIAG_IOCTL_96  0x96
#define LED_DIAG_IOCTL_97  0x97

union u_diag_pw_led_drv_ret{
    char    ch[sizeof(int)];
    int     it;
};

#endif /* __LINUX_LEDS_IAG_WRAPPER_H */
