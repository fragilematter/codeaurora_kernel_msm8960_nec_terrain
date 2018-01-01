/*
 * es310.h  --  es310 Soc Audio driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#ifndef _ES310_H
#define _ES310_H

struct es310_setup_data {
	unsigned short i2c_address;
	int mclk_div;   /* Set to a non-zero value if MCLK_DIV_2 required */
};

extern int es310_add_controls(struct snd_soc_codec *codec);

#define es310_REG_SYS_START	    0x8000
#define es310_REG_SPEECH_CLARITY   0x8fef
#define es310_REG_SYS_WATCHDOG     0x8ff6
#define es310_REG_ANA_VMID_PD_TIME 0x8ff7
#define es310_REG_ANA_VMID_PU_TIME 0x8ff8
#define es310_REG_CAT_FLTR_INDX    0x8ff9
#define es310_REG_CAT_GAIN_0       0x8ffa
#define es310_REG_SYS_STATUS       0x8ffc
#define es310_REG_SYS_MODE_CNTRL   0x8ffd
#define es310_REG_SYS_START0       0x8ffe
#define es310_REG_SYS_START1       0x8fff
#define es310_REG_ID1              0xf000
#define es310_REG_ID2              0xf001
#define es310_REG_REVISON          0xf002
#define es310_REG_SYS_CTL1         0xf003
#define es310_REG_SYS_CTL2         0xf004
#define es310_REG_ANC_STAT         0xf005
#define es310_REG_IF_CTL           0xf006

/* SPEECH_CLARITY */
#define es310_SPEECH_CLARITY   0x01

/* SYS_STATUS */
#define es310_STATUS_MOUSE_ACTIVE              0x40
#define es310_STATUS_CAT_FREQ_COMPLETE         0x20
#define es310_STATUS_CAT_GAIN_COMPLETE         0x10
#define es310_STATUS_THERMAL_SHUTDOWN_COMPLETE 0x08
#define es310_STATUS_ANC_DISABLED              0x04
#define es310_STATUS_POWER_DOWN_COMPLETE       0x02
#define es310_STATUS_BOOT_COMPLETE             0x01

/* SYS_MODE_CNTRL */
#define es310_MODE_ANA_SEQ_INCLUDE 0x80
#define es310_MODE_MOUSE_ENABLE    0x40
#define es310_MODE_CAT_FREQ_ENABLE 0x20
#define es310_MODE_CAT_GAIN_ENABLE 0x10
#define es310_MODE_BYPASS_ENTRY    0x08
#define es310_MODE_STANDBY_ENTRY   0x04
#define es310_MODE_THERMAL_ENABLE  0x02
#define es310_MODE_POWER_DOWN      0x01

/* SYS_CTL1 */
#define es310_SYS_STBY          0x01

/* SYS_CTL2 */
#define es310_MCLK_DIV2_ENA_CLR 0x80
#define es310_MCLK_DIV2_ENA_SET 0x40
#define es310_ANC_ENG_CLR       0x20
#define es310_ANC_ENG_SET       0x10
#define es310_ANC_INT_N_CLR     0x08
#define es310_ANC_INT_N_SET     0x04
#define es310_RAM_CLR           0x02
#define es310_RAM_SET           0x01

/* ANC_STAT */
#define es310_ANC_ENG_IDLE      0x01


extern int es310_add_controls(struct snd_soc_codec *codec);
extern void yda160_amp_set_path(int path);


#endif
