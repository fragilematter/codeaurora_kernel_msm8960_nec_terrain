/*
 * linux/sound/wm2000.h -- Platform data for WM2000
 *
 * Copyright 2010 Wolfson Microelectronics. PLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_SND_ES310_H
#define __LINUX_SND_ES310_H

struct es310_platform_data {
	void (*reset_control)(int on);
	void (*wakeup_control)(int on);
	int (*power_on)(int on);
	int (*dev_setup)(bool on);	
	int (*aud_clk)(int on);
	void (*sndamp_reset)(void);
	void (*sndamp_off)(void);
};

#endif
