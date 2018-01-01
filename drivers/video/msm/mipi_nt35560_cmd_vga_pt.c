/*
 * mipi_nt35560_cmd_vga_pt.c
 *
 * NT35560 panel specifications
 *
 * Copyright (c) 2011, NCMC
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
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_nt35560.h"

static int NT35560_LCD_DEBUG=1;

#define MIPI_NT35560_PWM_LEVEL 100

static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db = {
	/* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0xa0, 0x22, 0x11, 0x00, 0x8d, 0x8d, 0x16, 0x24,
	0x13, 0x03, 0x04, 0xa0},
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x00, 0x1a, 0xb1, 0xda, 0x00, 0x50, 0x48, 0x63,
	0x31, 0x0f, 0x03,
	0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};

static int __init mipi_cmd_nt35560_vga_pt_init(void)
{
	int ret;

	if(NT35560_LCD_DEBUG)
		printk(KERN_ERR "[NT35560_LCD]%s mipi-dsi nt35560 vga (640x480) driver ver 0.5.\n", __func__);

#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_cmd_nt35560_vga"))
		return 0;
#endif
	if (msm_fb_detect_client_ncm("mipi_cmd_nt35560_vga_pt"))
	{
		printk(KERN_INFO "%s. Not This Driver Module. \n", __func__);
		return 0;
	}

	pinfo.xres = 452;
	pinfo.yres = 640;

//	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.type = MIPI_CMD_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 50;
	pinfo.lcdc.h_front_porch = 50;
	pinfo.lcdc.h_pulse_width = 20;
	pinfo.lcdc.v_back_porch = 11;
	pinfo.lcdc.v_front_porch = 4;
	pinfo.lcdc.v_pulse_width = 1;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.lcd.vsync_enable = TRUE;
	pinfo.lcd.hw_vsync_mode = TRUE;
//	pinfo.lcd.vsync_enable = FALSE;
//	pinfo.lcd.hw_vsync_mode = FALSE;
	pinfo.bl_max = MIPI_NT35560_PWM_LEVEL;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 283000000;
	pinfo.lcd.refx100 = 6350; /* adjust refx100 to prevent tearing */
	pinfo.lcd.v_back_porch  = 11;
	pinfo.lcd.v_front_porch = 4;
	pinfo.lcd.v_pulse_width = 1;

//	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.mode = DSI_CMD_MODE;
//	pinfo.mipi.pulse_mode_hsa_he = TRUE;
//	pinfo.mipi.hfp_power_stop = TRUE;
//	pinfo.mipi.hbp_power_stop = TRUE;
//	pinfo.mipi.hsa_power_stop = TRUE;
//	pinfo.mipi.eof_bllp_power_stop = TRUE;
//	pinfo.mipi.bllp_power_stop = TRUE;
//	pinfo.mipi.traffic_mode = DSI_BURST_MODE;
//	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.dst_format = DSI_CMD_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
//	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
//	pinfo.mipi.tx_eot_append = TRUE;
	pinfo.mipi.t_clk_post = 0x19;
	pinfo.mipi.t_clk_pre = 0x2D;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
//	pinfo.mipi.frame_rate = 60.1;
	pinfo.mipi.te_sel = 1; /* TE from vsycn gpio */
//	pinfo.mipi.te_sel = 0; /* TE from vsycn gpio */
	pinfo.mipi.interleave_max = 1;
	pinfo.mipi.insert_dcs_cmd = TRUE;
	pinfo.mipi.wr_mem_continue = 0x3c;
	pinfo.mipi.wr_mem_start = 0x2c;
	pinfo.mipi.dsi_phy_db = &dsi_cmd_mode_phy_db;

	ret = mipi_nt35560_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_VGA);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_cmd_nt35560_vga_pt_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raghu Sesha Iyengar <raghu@borqs.com>");

