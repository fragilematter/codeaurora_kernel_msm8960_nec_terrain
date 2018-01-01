/*
 * mipi_nt35560_video_vga_pt.c
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
#include <linux/leds-lcd-common.h>

#define NT35560_LCD_DEBUG 0
#define USE_SET_BL 0

#if USE_SET_BL
#include "mdp4.h"
#endif


#define NT35560_CMD_DELAY 0 /* 50 */
#define NT35560_BL_DELAY 0 /* 50 */
#define NT35560_DISPLAY_ON_DELAY 150

#define	NT35560_LCD_OFF 0
#define	NT35560_LCD_ON 1
#define	NT35560_LCD_ENABLED 2


extern boolean msm_fb_disable_sleep;
static atomic_t nt35560_lcd_state = ATOMIC_INIT(NT35560_LCD_OFF);

static struct mipi_dsi_panel_platform_data *mipi_nt35560_pdata;

static struct dsi_buf nt35560_tx_buf;
static struct dsi_buf nt35560_rx_buf;

/* MIPI commands for "Driver Initialization" */
static char config_init_unlock[] = {0xf3, 0xaa};
static char config_init_manu_c9[] = {0xc9, 0x01};
static char config_init_manu_01[] = {0x01, 0x15};
static char config_init_manu_02[] = {0x02, 0x00};
static char config_init_manu_03[] = {0x03, 0x33};
static char config_init_manu_04[] = {0x04, 0x78};
static char config_init_manu_07[] = {0x07, 0x00};
static char config_init_manu_08[] = {0x08, 0x44};
static char config_init_manu_09[] = {0x09, 0x54};
static char config_init_manu_0a[] = {0x0a, 0x42};
static char config_init_manu_12[] = {0x12, 0x77};
static char config_init_manu_13[] = {0x13, 0x10};
static char config_init_manu_14[] = {0x14, 0x0d};
static char config_init_manu_15[] = {0x15, 0xa0};
static char config_init_manu_1a[] = {0x1a, 0x6b};
static char config_init_manu_1f[] = {0x1f, 0x00};
static char config_init_manu_20[] = {0x20, 0x01};
static char config_init_manu_21[] = {0x21, 0x63};
static char config_init_manu_94[] = {0x94, 0xef};
static char config_init_manu_95[] = {0x95, 0x00};
static char config_init_manu_96[] = {0x96, 0x00};
static char config_init_manu_97[] = {0x97, 0xe4};
static char config_init_manu_98[] = {0x98, 0x13};
static char config_init_manu_99[] = {0x99, 0x3a};
static char config_init_manu_9a[] = {0x9a, 0x0a};
static char config_init_manu_9b[] = {0x9b, 0x01};
static char config_init_manu_9c[] = {0x9c, 0x0a};
static char config_init_manu_9d[] = {0x9d, 0x00};
static char config_init_manu_9e[] = {0x9e, 0x00};
static char config_init_manu_9f[] = {0x9f, 0x00};
static char config_init_manu_a0[] = {0xa0, 0x0a};
static char config_init_manu_a2[] = {0xa2, 0x00};
static char config_init_manu_a3[] = {0xa3, 0x2e};
static char config_init_manu_a4[] = {0xa4, 0x0e};
static char config_init_manu_a5[] = {0xa5, 0xc0};
static char config_init_manu_a6[] = {0xa6, 0x01};
static char config_init_manu_a7[] = {0xa7, 0x00};
static char config_init_manu_a9[] = {0xa9, 0x00};
static char config_init_manu_aa[] = {0xaa, 0x00};
static char config_init_manu_e7[] = {0xe7, 0x00};
static char config_init_manu_ed[] = {0xed, 0x00};
static char config_init_manu_f3[] = {0xf3, 0xcc};
static char config_init_manu_fb[] = {0xfb, 0x00};
static char config_init_manu_ee[] = {0xee, 0x80};
static char config_init_manu_24[] = {0x24, 0x5e};
static char config_init_manu_25[] = {0x25, 0x68};
static char config_init_manu_26[] = {0x26, 0x75};
static char config_init_manu_27[] = {0x27, 0x86};
static char config_init_manu_28[] = {0x28, 0x1a};
static char config_init_manu_29[] = {0x29, 0x2d};
static char config_init_manu_2a[] = {0x2a, 0x5d};
static char config_init_manu_2b[] = {0x2b, 0x8e};
static char config_init_manu_2d[] = {0x2d, 0x21};
static char config_init_manu_2f[] = {0x2f, 0x29};
static char config_init_manu_30[] = {0x30, 0xd1};
static char config_init_manu_31[] = {0x31, 0x1a};
static char config_init_manu_32[] = {0x32, 0x34};
static char config_init_manu_33[] = {0x33, 0x3d};
static char config_init_manu_34[] = {0x34, 0xb9};
static char config_init_manu_35[] = {0x35, 0xd0};
static char config_init_manu_36[] = {0x36, 0xe0};
static char config_init_manu_37[] = {0x37, 0x76};
static char config_init_manu_38[] = {0x38, 0x09};
static char config_init_manu_39[] = {0x39, 0x1f};
static char config_init_manu_3a[] = {0x3a, 0x2f};
static char config_init_manu_3b[] = {0x3b, 0x46};
static char config_init_manu_3d[] = {0x3d, 0x42};
static char config_init_manu_3f[] = {0x3f, 0x4b};
static char config_init_manu_40[] = {0x40, 0x65};
static char config_init_manu_41[] = {0x41, 0x2e};
static char config_init_manu_42[] = {0x42, 0x16};
static char config_init_manu_43[] = {0x43, 0x1e};
static char config_init_manu_44[] = {0x44, 0x71};
static char config_init_manu_45[] = {0x45, 0x22};
static char config_init_manu_46[] = {0x46, 0x52};
static char config_init_manu_47[] = {0x47, 0x65};
static char config_init_manu_48[] = {0x48, 0x79};
static char config_init_manu_49[] = {0x49, 0x8a};
static char config_init_manu_4a[] = {0x4a, 0x97};
static char config_init_manu_4b[] = {0x4b, 0x21};
static char config_init_manu_4c[] = {0x4c, 0x5e};
static char config_init_manu_4d[] = {0x4d, 0x60};
static char config_init_manu_4e[] = {0x4e, 0x67};
static char config_init_manu_4f[] = {0x4f, 0x74};
static char config_init_manu_50[] = {0x50, 0x1a};
static char config_init_manu_51[] = {0x51, 0x31};
static char config_init_manu_52[] = {0x52, 0x60};
static char config_init_manu_53[] = {0x53, 0x97};
static char config_init_manu_54[] = {0x54, 0x21};
static char config_init_manu_55[] = {0x55, 0x29};
static char config_init_manu_56[] = {0x56, 0xd5};
static char config_init_manu_57[] = {0x57, 0x21};
static char config_init_manu_58[] = {0x58, 0x44};
static char config_init_manu_59[] = {0x59, 0x4e};
static char config_init_manu_5a[] = {0x5a, 0xa5};
static char config_init_manu_5b[] = {0x5b, 0xd4};
static char config_init_manu_5c[] = {0x5c, 0xe0};
static char config_init_manu_5d[] = {0x5d, 0x76};
static char config_init_manu_5e[] = {0x5e, 0x09};
static char config_init_manu_5f[] = {0x5f, 0x1f};
static char config_init_manu_60[] = {0x60, 0x2b};
static char config_init_manu_61[] = {0x61, 0x5a};
static char config_init_manu_62[] = {0x62, 0x31};
static char config_init_manu_63[] = {0x63, 0x3b};
static char config_init_manu_64[] = {0x64, 0x5e};
static char config_init_manu_65[] = {0x65, 0x2a};
static char config_init_manu_66[] = {0x66, 0x16};
static char config_init_manu_67[] = {0x67, 0x1e};
static char config_init_manu_68[] = {0x68, 0x68};
static char config_init_manu_69[] = {0x69, 0x1f};
static char config_init_manu_6a[] = {0x6a, 0x4e};
static char config_init_manu_6b[] = {0x6b, 0x65};
static char config_init_manu_6c[] = {0x6c, 0x8b};
static char config_init_manu_6d[] = {0x6d, 0x98};
static char config_init_manu_6e[] = {0x6e, 0x9f};
static char config_init_manu_6f[] = {0x6f, 0x21};
static char config_init_manu_70[] = {0x70, 0x7e};
static char config_init_manu_71[] = {0x71, 0x87};
static char config_init_manu_72[] = {0x72, 0xa9};
static char config_init_manu_73[] = {0x73, 0xc0};
static char config_init_manu_74[] = {0x74, 0x20};
static char config_init_manu_75[] = {0x75, 0x35};
static char config_init_manu_76[] = {0x76, 0x5f};
static char config_init_manu_77[] = {0x77, 0xaf};
static char config_init_manu_78[] = {0x78, 0x20};
static char config_init_manu_79[] = {0x79, 0x28};
static char config_init_manu_7a[] = {0x7a, 0xde};
static char config_init_manu_7b[] = {0x7b, 0x1e};
static char config_init_manu_7c[] = {0x7c, 0x40};
static char config_init_manu_7d[] = {0x7d, 0x4b};
static char config_init_manu_7e[] = {0x7e, 0xb6};
static char config_init_manu_7f[] = {0x7f, 0xe6};
static char config_init_manu_80[] = {0x80, 0xf0};
static char config_init_manu_81[] = {0x81, 0x76};
static char config_init_manu_82[] = {0x82, 0x09};
static char config_init_manu_83[] = {0x83, 0x0f};
static char config_init_manu_84[] = {0x84, 0x19};
static char config_init_manu_85[] = {0x85, 0x49};
static char config_init_manu_86[] = {0x86, 0x34};
static char config_init_manu_87[] = {0x87, 0x3f};
static char config_init_manu_88[] = {0x88, 0x61};
static char config_init_manu_89[] = {0x89, 0x21};
static char config_init_manu_8a[] = {0x8a, 0x17};
static char config_init_manu_8b[] = {0x8b, 0x1f};
static char config_init_manu_8c[] = {0x8c, 0x50};
static char config_init_manu_8d[] = {0x8d, 0x20};
static char config_init_manu_8e[] = {0x8e, 0x4a};
static char config_init_manu_8f[] = {0x8f, 0x5f};
static char config_init_manu_90[] = {0x90, 0x3f};
static char config_init_manu_91[] = {0x91, 0x56};
static char config_init_manu_92[] = {0x92, 0x78};
static char config_init_manu_93[] = {0x93, 0x01};
static char config_init_manu_lock[] = {0xff, 0xaa};
static char config_init_user_35[] = {0x35, 0x00};
static char config_init_user_44[] = {0x44, 0x00, 0x00};
static char config_init_user_36[] = {0x36, 0xd4};
static char config_init_user_51[] = {0x51, 0xff};
static char config_init_user_53[] = {0x53, 0x24};
static char config_init_user_55[] = {0x55, 0x02};

static struct dsi_cmd_desc nt35560_lcdm_power_up_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_unlock), config_init_unlock },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_c9), config_init_manu_c9 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_01), config_init_manu_01 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_02), config_init_manu_02 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_03), config_init_manu_03 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_04), config_init_manu_04 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_07), config_init_manu_07 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_08), config_init_manu_08 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_09), config_init_manu_09 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_0a), config_init_manu_0a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_12), config_init_manu_12 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_13), config_init_manu_13 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_14), config_init_manu_14 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_15), config_init_manu_15 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_1a), config_init_manu_1a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_1f), config_init_manu_1f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_20), config_init_manu_20 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_21), config_init_manu_21 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_94), config_init_manu_94 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_95), config_init_manu_95 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_96), config_init_manu_96 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_97), config_init_manu_97 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_98), config_init_manu_98 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_99), config_init_manu_99 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_9a), config_init_manu_9a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_9b), config_init_manu_9b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_9c), config_init_manu_9c },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_9d), config_init_manu_9d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_9e), config_init_manu_9e },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_9f), config_init_manu_9f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a0), config_init_manu_a0 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a2), config_init_manu_a2 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a3), config_init_manu_a3 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a4), config_init_manu_a4 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a5), config_init_manu_a5 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a6), config_init_manu_a6 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a7), config_init_manu_a7 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_a9), config_init_manu_a9 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_aa), config_init_manu_aa },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_e7), config_init_manu_e7 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_ed), config_init_manu_ed },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_f3), config_init_manu_f3 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_fb), config_init_manu_fb },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_ee), config_init_manu_ee },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_24), config_init_manu_24 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_25), config_init_manu_25 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_26), config_init_manu_26 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_27), config_init_manu_27 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_28), config_init_manu_28 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_29), config_init_manu_29 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_2a), config_init_manu_2a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_2b), config_init_manu_2b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_2d), config_init_manu_2d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_2f), config_init_manu_2f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_30), config_init_manu_30 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_31), config_init_manu_31 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_32), config_init_manu_32 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_33), config_init_manu_33 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_34), config_init_manu_34 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_35), config_init_manu_35 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_36), config_init_manu_36 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_37), config_init_manu_37 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_38), config_init_manu_38 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_39), config_init_manu_39 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_3a), config_init_manu_3a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_3b), config_init_manu_3b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_3d), config_init_manu_3d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_3f), config_init_manu_3f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_40), config_init_manu_40 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_41), config_init_manu_41 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_42), config_init_manu_42 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_43), config_init_manu_43 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_44), config_init_manu_44 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_45), config_init_manu_45 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_46), config_init_manu_46 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_47), config_init_manu_47 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_48), config_init_manu_48 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_49), config_init_manu_49 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_4a), config_init_manu_4a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_4b), config_init_manu_4b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_4c), config_init_manu_4c },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_4d), config_init_manu_4d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_4e), config_init_manu_4e },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_4f), config_init_manu_4f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_50), config_init_manu_50 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_51), config_init_manu_51 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_52), config_init_manu_52 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_53), config_init_manu_53 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_54), config_init_manu_54 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_55), config_init_manu_55 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_56), config_init_manu_56 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_57), config_init_manu_57 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_58), config_init_manu_58 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_59), config_init_manu_59 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_5a), config_init_manu_5a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_5b), config_init_manu_5b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_5c), config_init_manu_5c },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_5d), config_init_manu_5d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_5e), config_init_manu_5e },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_5f), config_init_manu_5f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_60), config_init_manu_60 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_61), config_init_manu_61 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_62), config_init_manu_62 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_63), config_init_manu_63 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_64), config_init_manu_64 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_65), config_init_manu_65 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_66), config_init_manu_66 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_67), config_init_manu_67 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_68), config_init_manu_68 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_69), config_init_manu_69 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_6a), config_init_manu_6a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_6b), config_init_manu_6b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_6c), config_init_manu_6c },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_6d), config_init_manu_6d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_6e), config_init_manu_6e },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_6f), config_init_manu_6f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_70), config_init_manu_70 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_71), config_init_manu_71 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_72), config_init_manu_72 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_73), config_init_manu_73 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_74), config_init_manu_74 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_75), config_init_manu_75 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_76), config_init_manu_76 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_77), config_init_manu_77 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_78), config_init_manu_78 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_79), config_init_manu_79 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_7a), config_init_manu_7a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_7b), config_init_manu_7b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_7c), config_init_manu_7c },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_7d), config_init_manu_7d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_7e), config_init_manu_7e },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_7f), config_init_manu_7f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_80), config_init_manu_80 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_81), config_init_manu_81 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_82), config_init_manu_82 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_83), config_init_manu_83 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_84), config_init_manu_84 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_85), config_init_manu_85 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_86), config_init_manu_86 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_87), config_init_manu_87 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_88), config_init_manu_88 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_89), config_init_manu_89 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_8a), config_init_manu_8a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_8b), config_init_manu_8b },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_8c), config_init_manu_8c },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_8d), config_init_manu_8d },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_8e), config_init_manu_8e },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_8f), config_init_manu_8f },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_90), config_init_manu_90 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_91), config_init_manu_91 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_92), config_init_manu_92 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_93), config_init_manu_93 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_manu_lock), config_init_manu_lock },
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_user_35), config_init_user_35 },
	{DTYPE_DCS_LWRITE, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_user_44), config_init_user_44 },
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_user_36), config_init_user_36 },
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_user_51), config_init_user_51 },
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_user_53), config_init_user_53 },
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_init_user_55), config_init_user_55 },
};

/* MIPI commands for "Exit Sleep" */
static char config_sleep_out[] = {0x11};

static struct dsi_cmd_desc nt35560_lcdm_sleep_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_sleep_out), config_sleep_out }
};

/* MIPI commands for "Set Display On" */
static char config_display_on_1[] = {0x2c};
static char config_display_on_2[] = {0x29};

static struct dsi_cmd_desc nt35560_lcdm_display_on_cmds_1[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(config_display_on_1), config_display_on_1},
//	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(config_display_on_2), config_display_on_2},
};
static struct dsi_cmd_desc nt35560_lcdm_display_on_cmds_2[] = {
//	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(config_display_on_1), config_display_on_1},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(config_display_on_2), config_display_on_2},
};


/* MIPI commands for "Set Display Off" */
static char config_display_off[] = {0x28};

static struct dsi_cmd_desc nt35560_lcdm_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(config_display_off), config_display_off},
};

/* MIPI commands for "Sleep set" */
static char config_sleep_in[] = {0x10};

static struct dsi_cmd_desc nt35560_lcdm_sleep_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(config_sleep_in), config_sleep_in},
};

#if USE_SET_BL
/* MIPI commands for display brightness (yet to come) */
static char config_display_brightness[] = {};

static struct dsi_cmd_desc nt35560_lcdm_brightness_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(config_display_brightness), config_display_brightness},
};
#endif

static void mipi_nt35560_set_backlight(struct msm_fb_data_type *mfd);
/* MIPI commands for display cabc */
static char config_cabc_control_f4[] = {0xf4, 0x55};
static char config_cabc_control_83[] = {0x83, 0x30,0x40,0x00};
static char config_cabc_control_82[] = {0x82, 0xb0};
static char config_cabc_control_ff[] = {0xff, 0x55};
static char config_cabc_control_f3[] = {0xf3, 0xaa};
static char config_cabc_control_00[] = {0x00, 0x01};
static char config_cabc_control_22[] = {0x22, 0x0b};
static char config_cabc_control_31[] = {0x31, 0xf3};
static char config_cabc_control_32[] = {0x32, 0xd9};
static char config_cabc_control_33[] = {0x33, 0xcc};
static char config_cabc_control_34[] = {0x34, 0xc0};
static char config_cabc_control_35[] = {0x35, 0xb3};
static char config_cabc_control_36[] = {0x36, 0xa6};
static char config_cabc_control_37[] = {0x37, 0x99};
static char config_cabc_control_38[] = {0x38, 0x99};
static char config_cabc_control_39[] = {0x39, 0x99};
static char config_cabc_control_3a[] = {0x3a, 0x95};
static char config_cabc_control_7f[] = {0x7f, 0xaa};

static struct dsi_cmd_desc nt35560_lcdm_cabc_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_f4), config_cabc_control_f4 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_83), config_cabc_control_83 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_82), config_cabc_control_82 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_ff), config_cabc_control_ff },
	{DTYPE_DCS_WRITE1, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_f3), config_cabc_control_f3 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_00), config_cabc_control_00 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_22), config_cabc_control_22 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_31), config_cabc_control_31 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_32), config_cabc_control_32 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_33), config_cabc_control_33 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_34), config_cabc_control_34 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_35), config_cabc_control_35 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_36), config_cabc_control_36 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_37), config_cabc_control_37 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_38), config_cabc_control_38 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_39), config_cabc_control_39 },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_3a), config_cabc_control_3a },
	{DTYPE_GEN_WRITE2, 1, 0, 0, NT35560_CMD_DELAY, sizeof(config_cabc_control_7f), config_cabc_control_7f },
};

void set_display_data(struct msm_fb_data_type *mfd)
{
	static char config_set_normal_mode[] = {0x13};
	static struct dsi_cmd_desc nt35560_lcdm_set_normal_mode_cmds[] = {
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(config_set_normal_mode), config_set_normal_mode},
	};

	static char config_set_hor_addr_1[] = {0x2a, 0x00, 0x00, 0x00, 0x0D};
	static struct dsi_cmd_desc nt35560_lcdm_set_hor_addr_cmds_1[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(config_set_hor_addr_1), config_set_hor_addr_1},
	};

	static char config_set_hor_addr_2[] = {0x2a, 0x01, 0xD2, 0x01, 0xdf};
	static struct dsi_cmd_desc nt35560_lcdm_set_hor_addr_cmds_2[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(config_set_hor_addr_2), config_set_hor_addr_2},
	};

	static char config_set_ver_addr[] = {0x2b, 0x00, 0x00, 0x02, 0x7f};
	static struct dsi_cmd_desc nt35560_lcdm_set_ver_addr_cmds[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(config_set_ver_addr), config_set_ver_addr},
	};
	static char config_disp_data[(20*14*3)+1];
	static struct dsi_cmd_desc nt35560_lcdm_config_disp_data_cmds[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(config_disp_data), config_disp_data},
	};
	char first;
	int num_rows = 32;
	int ret;

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);
	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_set_normal_mode_cmds,
			ARRAY_SIZE(nt35560_lcdm_set_normal_mode_cmds));
	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_set_hor_addr_cmds_1,
			ARRAY_SIZE(nt35560_lcdm_set_hor_addr_cmds_1));
	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_set_ver_addr_cmds,
			ARRAY_SIZE(nt35560_lcdm_set_ver_addr_cmds));
	first = 1;
	while(num_rows > 0) {
		config_disp_data[0] = 0x3c;
		if(first) {
			config_disp_data[0] = 0x2c;
			first = 0;
		}
		
		num_rows--;
		ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_config_disp_data_cmds,
			ARRAY_SIZE(nt35560_lcdm_config_disp_data_cmds));
	}

        ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_set_hor_addr_cmds_2,
                        ARRAY_SIZE(nt35560_lcdm_set_hor_addr_cmds_2));
        ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_set_ver_addr_cmds,
                        ARRAY_SIZE(nt35560_lcdm_set_ver_addr_cmds));
	num_rows = 32;
        first = 1;
        while(num_rows > 0) {
                config_disp_data[0] = 0x3c;
                if(first) {
                        config_disp_data[0] = 0x2c;
                        first = 0;
                }
                
                num_rows--;
                ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_config_disp_data_cmds,
                        ARRAY_SIZE(nt35560_lcdm_config_disp_data_cmds));
        }

}


static int mipi_nt35560_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	int ret;

	static char set_width[5] = { 0x2A, 0x00, 0x0E, 0x01, 0xD1}; /* 452 - 1 */
	static struct dsi_cmd_desc nt35560_lcdm_set_width_cmds[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 50,sizeof(set_width), set_width},
	};

	static char set_height[5] = { 0x2B, 0x00, 0x00, 0x02, 0x7F}; /* 640 - 1 */
	static struct dsi_cmd_desc nt35560_lcdm_set_height_cmds[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 50,sizeof(set_height), set_height},
	};

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	set_display_data(mfd);
	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_power_up_cmds,
			ARRAY_SIZE(nt35560_lcdm_power_up_cmds));

	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf,nt35560_lcdm_set_width_cmds,
			ARRAY_SIZE(nt35560_lcdm_set_width_cmds));

	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf,nt35560_lcdm_set_height_cmds,
			ARRAY_SIZE(nt35560_lcdm_set_height_cmds));
	mdelay(10);
	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_sleep_off_cmds,
			ARRAY_SIZE(nt35560_lcdm_sleep_off_cmds));
	mdelay(120);
	ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_cabc_cmds,
			ARRAY_SIZE(nt35560_lcdm_cabc_cmds));
	atomic_set(&nt35560_lcd_state, NT35560_LCD_ON);
	return 0;
}

int mipi_nt35560_lcd_enable(struct msm_fb_data_type *mfd)
{

	int ret;

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s \n", __func__);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	if(atomic_read(&nt35560_lcd_state) == NT35560_LCD_ON) {
		mdelay(20);
		ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_display_on_cmds_1,
			ARRAY_SIZE(nt35560_lcdm_display_on_cmds_1));
		mdelay(5);
		ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_display_on_cmds_2,
			ARRAY_SIZE(nt35560_lcdm_display_on_cmds_2));
#if USE_SET_BL
		mdelay(100);
		ret=mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_brightness_cmds,
			ARRAY_SIZE(nt35560_lcdm_brightness_cmds));
#endif
		mfd->bl_level = 200;
		mipi_nt35560_set_backlight(mfd);
		atomic_set(&nt35560_lcd_state, NT35560_LCD_ENABLED);
	}
	return 0;
}

static int mipi_nt35560_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (!msm_fb_disable_sleep)
	{
		mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_display_off_cmds,
				ARRAY_SIZE(nt35560_lcdm_display_off_cmds));
		mdelay(17);
		mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, nt35560_lcdm_sleep_on_cmds,
				ARRAY_SIZE(nt35560_lcdm_sleep_on_cmds));
		mdelay(70);
	}
	atomic_set(&nt35560_lcd_state, NT35560_LCD_OFF);

	return 0;
}

int mipi_nt35560_register_write_cmd( struct msm_fb_data_type *mfd,
                                    struct msmfb_register_write *data )
{
	char send_data[255];
	struct dsi_cmd_desc param;

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	memcpy(send_data, (char*)(&data->data[0]), data->len);

	param.dtype = (int)data->di;
	param.last  = 1;
	param.vc    = 0;
	param.ack   = 0;
	param.wait  = 0;
	param.dlen  = (int)data->len;
	param.payload = send_data;

	mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, &param,1);

	return 0;
}

int mipi_nt35560_register_read_cmd( struct msm_fb_data_type *mfd,
                                     struct msmfb_register_read *data )
{
	char send_data[2],ret_data[2] = {0x01,0x00};
	struct dsi_cmd_desc param;

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	send_data[0] = (char)data->w_data[0];
	send_data[1] = (char)data->w_data[1];
	param.last  = 1;
	param.vc    = 0;
	param.ack   = 0;
	param.wait  = 0;
	param.dlen = (int)data->len;

	param.dtype = DTYPE_MAX_PKTSIZE;
	param.payload = ret_data;
	mipi_dsi_cmds_tx(mfd, &nt35560_tx_buf, &param,1);

	param.dtype = (char)data->di;
	param.payload = send_data;
	mipi_dsi_cmds_rx(mfd, &nt35560_tx_buf, &nt35560_rx_buf,
			&param, param.dlen);

	if (nt35560_rx_buf.len > 0) {
		memcpy(data->r_data, (unsigned char*)nt35560_rx_buf.data, nt35560_rx_buf.len);
	}
	return 0;
}

static void mipi_nt35560_set_backlight(struct msm_fb_data_type *mfd)
{
    int ret;
    unsigned char bl_level = 0;
    if (mfd == NULL)
    {
        printk(KERN_ERR "%s (%d): NULL parameter.\n", __func__, __LINE__);
        return;
    }

    if(NT35560_LCD_DEBUG)
	pr_info("[NT35560_LCD] %s\n", __func__);

    bl_level = (unsigned char)mfd->bl_level;
//    bl_level = 0xFF;

    if (bl_level > 0)
    {
        /* Set Backlight Level */
        ret = led_main_lcd_bright(bl_level);
        if ( ret != LEDS_LED_SET_OK)
        {
            printk(KERN_ERR "%s. set backlight level failed!!\n", __func__);
            return;
        }
        /* Set Backlight On */
        ret = led_main_lcd_set(LEDS_LED_ON, LEDS_LED_ON);
        if ( ret != LEDS_LED_SET_OK)
        {
            printk(KERN_ERR "%s. backlight on request failed!!\n", __func__);
            return;
        }
    } else {
        /* Set Backlight Off */
        ret = led_main_lcd_set(LEDS_LED_OFF, LEDS_LED_OFF);
        if ( ret != LEDS_LED_SET_OK)
        {
            printk(KERN_ERR "%s. backlight off request failed!!\n", __func__);
            return;
        }
    }
    return;
}

static int __devinit mipi_nt35560_lcd_probe(struct platform_device *pdev)
{
	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);

	if (pdev->id == 0) {
		mipi_nt35560_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_nt35560_lcd_probe,
	.driver = {
		.name   = "mipi_nt35560",
	},
};

static struct msm_fb_panel_data nt35560_panel_data = {
	.on		= mipi_nt35560_lcd_on,
	.off		= mipi_nt35560_lcd_off,
	.set_backlight  = mipi_nt35560_set_backlight,
};

static int ch_used[3];

int mipi_nt35560_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);
	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_nt35560", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	nt35560_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &nt35560_panel_data,
		sizeof(nt35560_panel_data));
	if (ret) {
		pr_err("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_err("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init mipi_nt35560_lcd_init(void)
{
	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);
	mipi_dsi_buf_alloc(&nt35560_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&nt35560_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

int mipi_nt35560_user_request_ctrl( struct msmfb_request_parame *data )
{
	int ret = 0;

	if(NT35560_LCD_DEBUG)
		pr_info("[NT35560_LCD] %s\n", __func__);

	switch( data->request )
	{
		case MSM_FB_REQUEST_OVERLAY_ALPHA:
			ret = copy_from_user(&mdp4_overlay_argb_enable, data->data, sizeof(mdp4_overlay_argb_enable));
		break;

		default:
		/* Error Log */
			pr_info("[NT35560_LCD]%s mddi_ta8851_user_request_ctrl\n", __func__);
		break;
	}

	return ret;
}

module_init(mipi_nt35560_lcd_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raghu Sesha Iyengar <raghu@borqs.com>");
