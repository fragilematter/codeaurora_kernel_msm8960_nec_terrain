/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_renesas_g121s.h"

static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/*
	 * The following parameters are calculate automatically.
	 * by using the exel sheet.
	 */
	/* 800*1280, RGB888, 4 Lane 60 fps video mode */
	/* regulator */
	{ 0x03, 0x0A, 0x04, 0x00, 0x20 },
	/* timing   */
	{ 0xB8, 0x8E, 0x20, 0x00, 0x98, 0x98,
	  0x23, 0x90, 0x23, 0x03, 0x04, 0xA0 },
	/* phy ctrl */
	{ 0x5F, 0x00, 0x00, 0x10 },
	/* strength */
	{ 0xFF, 0x00, 0x06 },
	/* pll control */
	{ 0x00, 0xEF, 0x31, 0xDA, 0x00, 0x50, 0x48, /*  0- 6 */
	  0x63, 0x31, 0x0F, 0x03, 0x00, 0x14, 0x03, /*  7-13 */
	  0x00, 0x02, 0x00, 0x20, 0x00, 0x01        /* 14-19 */
	},
};

static int __init mipi_video_toshiba_g121s_pt_init(void)
{
	int ret;
#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_video_toshiba_g121s"))
		return 0;
#endif

	if (msm_fb_detect_client_ncm("mipi_video_toshiba_g121s_pt"))
	{
		printk(KERN_INFO "%s. Not This Driver Module. \n", __func__);
		return 0;
	}

	pinfo.xres = 720;
	pinfo.yres = 1280;
	/*
	 *
	 * Panel's Horizontal input timing requirement is to
	 * include dummy(pad) data of 200 clk in addition to
	 * width and porch/sync width values
	 */
	//pinfo.mipi.xres_pad = 200;
    pinfo.mipi.xres_pad = 0;
	pinfo.mipi.yres_pad = 1;

	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;


	pinfo.lcd.refx100 = 6010; /* adjust refx100 to prevent tearing */

	pinfo.lcdc.h_back_porch  = 30;
	pinfo.lcdc.h_front_porch = 299;
	pinfo.lcdc.h_pulse_width = 13;
	pinfo.lcdc.v_back_porch  = 3;
	pinfo.lcdc.v_front_porch = 9;
	pinfo.lcdc.v_pulse_width = 4;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 99; /* back light */
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 496000000; /* use frame_rate insted of */

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = FALSE;
	pinfo.mipi.hfp_power_stop = FALSE;
	pinfo.mipi.hbp_power_stop = FALSE;
	pinfo.mipi.hsa_power_stop = FALSE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = FALSE;
	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
	pinfo.mipi.t_clk_post = 25;
	pinfo.mipi.t_clk_pre = 48;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate = 60;
	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;

	ret = mipi_renesas_g121s_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_G121S_PT);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_toshiba_g121s_pt_init);
