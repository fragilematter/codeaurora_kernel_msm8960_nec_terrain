/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#include "msm_sensor.h"
#include "msm.h"
#include "msm_ispif.h"
#include <mach/irqs.h>
#include <linux/regulator/consumer.h> /* for reguretor_get()      */
#include <linux/mfd/pm8xxx/pm8921.h>  /* for pm8xxx_gpio_config() */

#define SENSOR_NAME "imx111"
#define PLATFORM_DRIVER_NAME "msm_camera_imx111"
#define imx111_obj imx111_##obj

/*=============================================================
    DEFINES
==============================================================*/
/* POWER ON WAIT TIME */
/* usec */

/* msec  */
#define IMX111_WAIT_PWON_EN2           1       /*   1ms */
#define IMX111_WAIT_PWON_EN1           1       /*   1ms */
#define IMX111_WAIT_PWON_VREG_L11      1       /*   1ms */
#define IMX111_WAIT_PWON_RST_H         1       /*   1ms */
#define IMX111_WAIT_PWON_REG_SET       1       /*   1ms */
#define IMX111_WAIT_PWON_CLK           1       /*   1ms */
#define IMX111_WAIT_PWON_CLK_2        10       /*  10ms */

/* POWER OFF WAIT TIME */
/* msec  */
#define IMX111_WAIT_PWOFF_MCLK        35       /*  35ms */
#define IMX111_WAIT_PWOFF_RST_L        1       /*   1ms */
#define IMX111_WAIT_PWOFF_VREG_L11     1       /*   1ms */
#define IMX111_WAIT_PWOFF_V_EN1        1       /*   1ms */
#define IMX111_WAIT_PWOFF_V_EN2        1       /*   1ms */
#define IMX111_WAIT_PWOFF_V_EN3        1       /*   1ms */
#define IMX111_WAIT_PWOFF_VREG_L12     1       /*   1ms */

/* MSM GPIO */
#define IMX111_GPIO_CAM_V_EN3         2        /* GPIO[2]  */
#define IMX111_GPIO_CAM_V_EN1         3        /* GPIO[3]  */
#define IMX111_GPIO_CAM_MCLK0         5        /* GPIO[5]  */
#define IMX111_GPIO_CAM_I2C_SDA       20       /* GPIO[20] */
#define IMX111_GPIO_CAM_I2C_SCL       21       /* GPIO[21] */
#define IMX111_GPIO_CAM_RST_N        107       /* GPIO[107]*/

/* PM GPIO */
#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)
#define IMX111_PMGPIO_CAM_V_EN2  PM8921_GPIO_PM_TO_SYS(25)   /* GPIO[25] */

/* VREG */
#define CAM_VANA_MINUV                    2600000
#define CAM_VANA_MAXUV                    3300000
#define CAM_VANA_LOAD_UA                  150000
static struct regulator *cam_vana = NULL;

#define CAM_VDIG_MINUV                    1100000
#define CAM_VDIG_MAXUV                    1500000
#define CAM_VDIG_LOAD_UA                  150000
static struct regulator *cam_vdig = NULL;

struct pm_gpio imx111_cam_v_en2_on = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 1,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};

struct pm_gpio imx111_cam_v_en2_off = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 0,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};

/* 1032-MERGE */
static struct msm_cam_clk_info imx111_cam_clk_info[] = {
    {"cam_clk", MSM_SENSOR_MCLK_25HZ},
};
/* 1032-MERGE */

DEFINE_MUTEX(imx111_mut);
static struct msm_sensor_ctrl_t imx111_s_ctrl;
#define IMX111_EEPROM_BANK_SEL_REG 0x34C9

static struct msm_camera_i2c_reg_conf imx111_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf imx111_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf imx111_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf imx111_groupoff_settings[] = {
	{0x104, 0x00},
};

/* NCMC reg settings */
static struct msm_camera_i2c_reg_conf imx111_snap_settings[] = {
/* reg.A */
	{0x0305, 0x04},
	{0x0307, 0x6D},
	{0x30A4, 0x02},
	{0x303C, 0x50},
	{0x0340, 0x09},
	{0x0341, 0xE0},
	{0x0342, 0x0D},
	{0x0343, 0x70},
	{0x0344, 0x00},
	{0x0345, 0x08},
	{0x0346, 0x00},
	{0x0347, 0x30},
	{0x0348, 0x0C},
	{0x0349, 0xD7},
	{0x034A, 0x09},
	{0x034B, 0xCF},
	{0x034C, 0x0C},
	{0x034D, 0xD0},
	{0x034E, 0x09},
	{0x034F, 0xA0},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x3033, 0x00},
	{0x303D, 0x10},
	{0x303E, 0x41},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x00},
	{0x304C, 0x57},
	{0x304D, 0x03},
	{0x3064, 0x12},
	{0x309B, 0x20},
	{0x309C, 0x13},
	{0x309E, 0x00},
	{0x30A0, 0x14},
	{0x30A1, 0x08},
	{0x30AA, 0x02},
	{0x30B2, 0x05},
	{0x30D5, 0x00},
	{0x30D6, 0x85},
	{0x30D7, 0x2A},
	{0x30D8, 0x64},
	{0x30D9, 0x89},
	{0x30DE, 0x00},
	{0x30DF, 0x20},
	{0x3102, 0x10},
	{0x3103, 0x44},
	{0x3104, 0x40},
	{0x3105, 0x00},
	{0x3106, 0x0D},
	{0x3107, 0x01},
	{0x3108, 0x09},
	{0x3109, 0x08},
	{0x310A, 0x0F},
	{0x315C, 0x5D},
	{0x315D, 0x5C},
	{0x316E, 0x5E},
	{0x316F, 0x5D},
	{0x3301, 0x00},
	{0x3318, 0x62},
	{0x3348, 0xE0},
};

static struct msm_camera_i2c_reg_conf imx111_prev_settings[] = {
/* reg.B */
	{0x0305, 0x04},
	{0x0307, 0x6D},
	{0x30A4, 0x02},
	{0x303C, 0x50},
	{0x0340, 0x05},
	{0x0341, 0x1E},
	{0x0342, 0x06},
	{0x0343, 0xE0},
	{0x0344, 0x00},
	{0x0345, 0x08},
	{0x0346, 0x00},
	{0x0347, 0x30},
	{0x0348, 0x0C},
	{0x0349, 0xD7},
	{0x034A, 0x09},
	{0x034B, 0xCF},
	{0x034C, 0x06},
	{0x034D, 0x68},
	{0x034E, 0x04},
	{0x034F, 0xD0},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x03},
	{0x3033, 0x87},
	{0x303D, 0x10},
	{0x303E, 0x51},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x01},
	{0x304C, 0xB7},
	{0x304D, 0x01},
	{0x3064, 0x10},
	{0x309B, 0x28},
	{0x309C, 0x13},
	{0x309E, 0x00},
	{0x30A0, 0x14},
	{0x30A1, 0x09},
	{0x30AA, 0x00},
	{0x30B2, 0x03},
	{0x30D5, 0x04},
	{0x30D6, 0x85},
	{0x30D7, 0x2A},
	{0x30D8, 0x64},
	{0x30D9, 0x89},
	{0x30DE, 0x00},
	{0x30DF, 0x20},
	{0x3102, 0x08},
	{0x3103, 0x22},
	{0x3104, 0x20},
	{0x3105, 0x00},
	{0x3106, 0x87},
	{0x3107, 0x00},
	{0x3108, 0x03},
	{0x3109, 0x02},
	{0x310A, 0x03},
	{0x315C, 0x3A},
	{0x315D, 0x39},
	{0x316E, 0x3B},
	{0x316F, 0x3A},
	{0x3301, 0x00},
	{0x3318, 0x67},
	{0x3348, 0xF1},
};

static struct msm_camera_i2c_reg_conf imx111_hd_video_settings[] = {
/* reg.C */
	{0x0305, 0x04},
	{0x0307, 0x6D},
	{0x30A4, 0x02},
	{0x303C, 0x50},
	{0x0340, 0x05},
	{0x0341, 0x1E},
	{0x0342, 0x06},
	{0x0343, 0xE0},
	{0x0344, 0x00},
	{0x0345, 0x08},
	{0x0346, 0x01},
	{0x0347, 0x64},
	{0x0348, 0x0C},
	{0x0349, 0xD7},
	{0x034A, 0x08},
	{0x034B, 0x9B},
	{0x034C, 0x06},
	{0x034D, 0x68},
	{0x034E, 0x03},
	{0x034F, 0x9C},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x03},
	{0x3033, 0x87},
	{0x303D, 0x10},
	{0x303E, 0x51},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x01},
	{0x304C, 0xB7},
	{0x304D, 0x01},
	{0x3064, 0x10},
	{0x309B, 0x28},
	{0x309C, 0x13},
	{0x309E, 0x00},
	{0x30A0, 0x14},
	{0x30A1, 0x09},
	{0x30AA, 0x00},
	{0x30B2, 0x03},
	{0x30D5, 0x04},
	{0x30D6, 0x85},
	{0x30D7, 0x2A},
	{0x30D8, 0x64},
	{0x30D9, 0x89},
	{0x30DE, 0x00},
	{0x30DF, 0x20},
	{0x3102, 0x08},
	{0x3103, 0x22},
	{0x3104, 0x20},
	{0x3105, 0x00},
	{0x3106, 0x87},
	{0x3107, 0x00},
	{0x3108, 0x03},
	{0x3109, 0x02},
	{0x310A, 0x03},
	{0x315C, 0x3A},
	{0x315D, 0x39},
	{0x316E, 0x3B},
	{0x316F, 0x3A},
	{0x3301, 0x00},
	{0x3318, 0x67},
	{0x3348, 0xF1},
};

static struct msm_camera_i2c_reg_conf imx111_hs_prev_snap_settings[] = {
/* reg.D */
	{0x0305, 0x04},
	{0x0307, 0x6D},
	{0x30A4, 0x02},
	{0x303C, 0x50},
	{0x0340, 0x05},
	{0x0341, 0x1E},
	{0x0342, 0x06},
	{0x0343, 0xE0},
	{0x0344, 0x00},
	{0x0345, 0x08},
	{0x0346, 0x00},
	{0x0347, 0x30},
	{0x0348, 0x0C},
	{0x0349, 0xD7},
	{0x034A, 0x09},
	{0x034B, 0xCF},
	{0x034C, 0x06},
	{0x034D, 0x68},
	{0x034E, 0x04},
	{0x034F, 0xD0},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x03},
	{0x3033, 0x84},
	{0x303D, 0x10},
	{0x303E, 0x51},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x01},
	{0x304C, 0xB7},
	{0x304D, 0x01},
	{0x3064, 0x10},
	{0x309B, 0x28},
	{0x309C, 0x13},
	{0x309E, 0x00},
	{0x30A0, 0x14},
	{0x30A1, 0x09},
	{0x30AA, 0x00},
	{0x30B2, 0x03},
	{0x30D5, 0x04},
	{0x30D6, 0x85},
	{0x30D7, 0x2A},
	{0x30D8, 0x64},
	{0x30D9, 0x89},
	{0x30DE, 0x00},
	{0x30DF, 0x20},
	{0x3102, 0x08},
	{0x3103, 0x22},
	{0x3104, 0x20},
	{0x3105, 0x00},
	{0x3106, 0x87},
	{0x3107, 0x00},
	{0x3108, 0x03},
	{0x3109, 0x02},
	{0x310A, 0x03},
	{0x315C, 0x3A},
	{0x315D, 0x39},
	{0x316E, 0x3B},
	{0x316F, 0x3A},
	{0x3301, 0x00},
	{0x3318, 0x67},
	{0x3348, 0xF1},
};

static struct msm_camera_i2c_reg_conf imx111_full_hd_video_settings[] = {
/* reg.E */
	{0x0305, 0x04},
	{0x0307, 0x6D},
	{0x30A4, 0x02},
	{0x303C, 0x50},
	{0x0340, 0x09},
	{0x0341, 0xE0},
	{0x0342, 0x0D},
	{0x0343, 0x70},
	{0x0344, 0x00},
	{0x0345, 0x08},
	{0x0346, 0x01},
	{0x0347, 0x60},
	{0x0348, 0x0C},
	{0x0349, 0xD7},
	{0x034A, 0x08},
	{0x034B, 0xA1},
	{0x034C, 0x0C},
	{0x034D, 0xD0},
	{0x034E, 0x07},
	{0x034F, 0x42},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x3033, 0x00},
	{0x303D, 0x10},
	{0x303E, 0x41},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x00},
	{0x304C, 0x57},
	{0x304D, 0x03},
	{0x3064, 0x12},
	{0x309B, 0x20},
	{0x309C, 0x13},
	{0x309E, 0x00},
	{0x30A0, 0x14},
	{0x30A1, 0x08},
	{0x30AA, 0x02},
	{0x30B2, 0x05},
	{0x30D5, 0x00},
	{0x30D6, 0x85},
	{0x30D7, 0x2A},
	{0x30D8, 0x64},
	{0x30D9, 0x89},
	{0x30DE, 0x00},
	{0x30DF, 0x20},
	{0x3102, 0x10},
	{0x3103, 0x44},
	{0x3104, 0x40},
	{0x3105, 0x00},
	{0x3106, 0x0D},
	{0x3107, 0x01},
	{0x3108, 0x09},
	{0x3109, 0x08},
	{0x310A, 0x0F},
	{0x315C, 0x5D},
	{0x315D, 0x5C},
	{0x316E, 0x5E},
	{0x316F, 0x5D},
	{0x3301, 0x00},
	{0x3318, 0x62},
	{0x3348, 0xE0},
};

static struct msm_camera_i2c_reg_conf imx111_recommend_settings[] = {
/* NCMC reg */
	{0x0101, 0x03},
	{0x3080, 0x50},
	{0x3087, 0x53},
	{0x309D, 0x94},
	{0x30B1, 0x03},
	{0x30C6, 0x00},
	{0x30C7, 0x00},
	{0x3115, 0x0B},
	{0x3118, 0x30},
	{0x311D, 0x25},
	{0x3121, 0x0A},
	{0x3212, 0xF2},
	{0x3213, 0x0F},
	{0x3215, 0x0F},
	{0x3217, 0x0B},
	{0x3219, 0x0B},
	{0x321B, 0x0D},
	{0x321D, 0x0D},
	{0x32AA, 0x11},
	{0x3032, 0x40},
};


static struct msm_camera_i2c_reg_conf imx111_comm1_settings[] = {
	{0x3035, 0x10},
	{0x303B, 0x14},
	{0x3312, 0x45},
	{0x3313, 0xC0},
	{0x3310, 0x20},
	{0x3310, 0x00},
	{0x303B, 0x04},
	{0x303D, 0x00},
	{0x0100, 0x10},
	{0x3035, 0x00},
};

static struct msm_camera_i2c_reg_conf imx111_comm2_part1_settings[] = {
	{0x30B1, 0x43},
	{0x3301, 0x00},
	{0x3311, 0x80},
	{0x3311, 0x00},
};

static struct msm_camera_i2c_reg_conf imx111_comm2_part2_settings[] = {
	{0x30B1, 0x03},
};

static struct msm_camera_i2c_conf_array imx111_comm_confs[] = {
	{&imx111_comm1_settings[0],
	ARRAY_SIZE(imx111_comm1_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_comm2_part1_settings[0],
	ARRAY_SIZE(imx111_comm2_part1_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_comm2_part2_settings[0],
	ARRAY_SIZE(imx111_comm2_part2_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct v4l2_subdev_info imx111_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array imx111_init_conf[] = {
	{&imx111_recommend_settings[0],
	ARRAY_SIZE(imx111_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array imx111_confs[] = {
	{&imx111_snap_settings[0],
	ARRAY_SIZE(imx111_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_prev_settings[0],
	ARRAY_SIZE(imx111_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_hd_video_settings[0],
	ARRAY_SIZE(imx111_hd_video_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_hs_prev_snap_settings[0],
	ARRAY_SIZE(imx111_hs_prev_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_full_hd_video_settings[0],
	ARRAY_SIZE(imx111_full_hd_video_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_snap_settings[0],
	ARRAY_SIZE(imx111_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx111_hs_prev_snap_settings[0],
	ARRAY_SIZE(imx111_hs_prev_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t imx111_dimensions[] = {
	{
	/*reg.A 16.04 fps */
		.x_output = 0x0CD0,             /* 3280 */
		.y_output = 0x09A0,             /* 2464 */
		.line_length_pclk   = 0x0D70,   /* 3440 */
		.frame_length_lines = 0x09E0,   /* 2528 */
		.vt_pixel_clk = 139520000,
		.op_pixel_clk = 139520000,
		.binning_factor = 1,
	},
	{
	/*reg.B 30.26 fps */
		.x_output = 0x0668,             /* 1640 */
		.y_output = 0x04D0,             /* 1232 */
		.line_length_pclk   = 0x06E0,   /* 1760 */
		.frame_length_lines = 0x051E,   /* 1310 */
		.vt_pixel_clk = 69760000,
		.op_pixel_clk = 69760000,
		.binning_factor = 1,
	},
	{
	/*reg.C 30.26 fps */
		.x_output = 0x0668,             /* 1640 */
		.y_output = 0x039C,             /*  924 */
		.line_length_pclk   = 0x06E0,   /* 1760 */
		.frame_length_lines = 0x051E,   /* 1310 */
		.vt_pixel_clk = 69760000,
		.op_pixel_clk = 69760000,
		.binning_factor = 1,
	},
	{
	/*reg.D 30.26 fps */
		.x_output = 0x0668,             /* 1640 */
		.y_output = 0x04D0,             /* 1232 */
		.line_length_pclk   = 0x06E0,   /* 1760 */
		.frame_length_lines = 0x051E,   /* 1310 */
		.vt_pixel_clk = 69760000,
		.op_pixel_clk = 69760000,
		.binning_factor = 2,
	},
	{
	/*reg.E 16.04 fps */
		.x_output = 0x0CD0,             /* 3280 */
		.y_output = 0x0742,             /* 1858 */
		.line_length_pclk   = 0x0D70,   /* 3440 */
		.frame_length_lines = 0x09E0,   /* 2528 */
		.vt_pixel_clk = 139520000,
		.op_pixel_clk = 139520000,
		.binning_factor = 1,
	},
	{
	/*reg.A 16.04 fps */
		.x_output = 0x0CD0,             /* 3280 */
		.y_output = 0x09A0,             /* 2464 */
		.line_length_pclk   = 0x0D70,   /* 3440 */
		.frame_length_lines = 0x09E0,   /* 2528 */
		.vt_pixel_clk = 139520000,
		.op_pixel_clk = 139520000,
		.binning_factor = 1,
	},
	{
	/*reg.D 30.26 fps */
		.x_output = 0x0668,             /* 1640 */
		.y_output = 0x04D0,             /* 1232 */
		.line_length_pclk   = 0x06E0,   /* 1760 */
		.frame_length_lines = 0x051E,   /* 1310 */
		.vt_pixel_clk = 69760000,
		.op_pixel_clk = 69760000,
		.binning_factor = 2,
	},
};

static struct msm_camera_csid_vc_cfg imx111_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
	{2, CSI_RESERVED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx111_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = 3,
			.vc_cfg = imx111_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x14,
	},
};

static struct msm_camera_csi2_params *imx111_csi_params_array[] = {
	&imx111_csi_params,
	&imx111_csi_params,
	&imx111_csi_params,
	&imx111_csi_params,
	&imx111_csi_params,
	&imx111_csi_params,
	&imx111_csi_params, 
};

static struct msm_sensor_output_reg_addr_t imx111_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t imx111_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x0111,
};

static struct msm_sensor_exp_gain_info_t imx111_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 5,
};

/* ADD OTP_READ_TEST */
#define OTP_READ_BANK_ADDR               0x34C9
#define OTP_READ_BANK_MAX                0x0F
#define OTP_READ_BANK_DATA_SIZE          0x08
#define OTP_READ_OUT_REGISTER_START_ADDR 0x3500
#define OTP_READ_OUT_REGISTER_DATA_SIZE  0x80

static char otp_data[OTP_READ_OUT_REGISTER_DATA_SIZE];

static int32_t imx111_sensor_otp_read(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;

    char bank = 0x00;
    int  read_offset;
    uint16_t* opt_data_uint16;
    uint16_t read_addr = OTP_READ_OUT_REGISTER_START_ADDR;

    for( bank = 0; bank <= OTP_READ_BANK_MAX; bank++)
    {
    	rc = msm_camera_i2c_write(
    			s_ctrl->sensor_i2c_client,
     			OTP_READ_BANK_ADDR,
    			bank,
    			MSM_CAMERA_I2C_BYTE_DATA);

        if(rc < 0){
            pr_err("%s: i2c_write failed\n", __func__);
            return rc;
        }

        for( read_offset = 0; read_offset < OTP_READ_BANK_DATA_SIZE ; read_offset+=sizeof(uint16_t) )
        {
        	rc = msm_camera_i2c_read(
    			s_ctrl->sensor_i2c_client,
    			read_addr, 
    			(uint16_t*)&otp_data[bank*OTP_READ_BANK_DATA_SIZE + read_offset],
    			sizeof(uint16_t));

            if(rc < 0){
                pr_err("%s: i2c_read failed\n", __func__);
                return rc;
            }

//            printk("imx111_sensor_otp_read :bank=0x%X,read_addr=0x%X,read_offset=0x%X\n",bank,read_addr,read_offset);
            read_addr = read_addr + sizeof(uint16_t);
    	}
    }
    
    /* for debug */
    read_addr = 0;
    opt_data_uint16 = (uint16_t*)&otp_data[0];

    return rc;
}

int32_t msm_sensor_imx111_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	CDBG("%s\n", __func__);
	
	printk("imx111.c %s: start\n", __func__);

    /* wait 35msec */
    msleep(IMX111_WAIT_PWOFF_MCLK);
    /* CAM_MCLK Off */
    gpio_direction_output(IMX111_GPIO_CAM_MCLK0, 0);

    msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
        imx111_cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(imx111_cam_clk_info), 0);
    msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
        s_ctrl->sensordata->sensor_platform_info->cam_vreg,
        s_ctrl->sensordata->sensor_platform_info->num_vreg,
        s_ctrl->reg_ptr, 0);
    msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
        s_ctrl->sensordata->sensor_platform_info->cam_vreg,
        s_ctrl->sensordata->sensor_platform_info->num_vreg,
        s_ctrl->reg_ptr, 0);
    msm_camera_request_gpio_table(data, 0);

    kfree(s_ctrl->reg_ptr);
    /* wait 1msec */
    msleep(IMX111_WAIT_PWOFF_RST_L);
    gpio_direction_output(data->sensor_platform_info->sensor_reset, 0);
	gpio_free(data->sensor_platform_info->sensor_reset);

    /* wait 1msec */
    msleep(IMX111_WAIT_PWOFF_VREG_L11);
    /* VREG_L11 Off */
    if (cam_vana){
		regulator_set_optimum_mode(cam_vana, 0);
		regulator_set_voltage(cam_vana, 0, CAM_VANA_MAXUV);
		regulator_disable(cam_vana);
		regulator_put(cam_vana);
		cam_vana = NULL;
    }

    /* wait 1msec */
    msleep(IMX111_WAIT_PWOFF_V_EN1);
    /* CAM_V_EN1 Off */
    gpio_direction_output(IMX111_GPIO_CAM_V_EN1, 0);
    gpio_free(IMX111_GPIO_CAM_V_EN1);

    /* wait 1msec */
    msleep(IMX111_WAIT_PWOFF_V_EN2);
#ifndef CONFIG_FEATURE_NCMC_D121M
    gpio_free(IMX111_GPIO_CAM_I2C_SDA);
    gpio_free(IMX111_GPIO_CAM_I2C_SCL);
#endif
    /* VDD_CAM2_V_EN1(PM GPIO) OFF */
    pm8xxx_gpio_config(IMX111_PMGPIO_CAM_V_EN2, &imx111_cam_v_en2_off);

    /* wait 1msec */
    msleep(IMX111_WAIT_PWOFF_V_EN3);
    /* CAM_V_EN3 Off */
    gpio_direction_output(IMX111_GPIO_CAM_V_EN3, 0);
    gpio_free(IMX111_GPIO_CAM_V_EN3);

    /* wait 1msec */
    msleep(IMX111_WAIT_PWOFF_VREG_L12);
    /* VREG_L12 Off */
    if (cam_vdig) {
        regulator_set_voltage(cam_vdig, 0, CAM_VDIG_MAXUV);
        regulator_set_optimum_mode(cam_vdig, 0);
        regulator_disable(cam_vdig);
        regulator_put(cam_vdig);
        cam_vdig = NULL;
    }

	printk("imx111.c %s: end\n", __func__);
	return 0;
}

int32_t msm_sensor_imx111_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
//	const struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;

	CDBG("%s: %d\n", __func__, __LINE__);
	printk("imx111.c %s: start\n", __func__);


    s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
            * data->sensor_platform_info->num_vreg, GFP_KERNEL);
    if (!s_ctrl->reg_ptr) {
        pr_err("%s: could not allocate mem for regulators\n",
            __func__);
        return -ENOMEM;
    }

    rc = msm_camera_request_gpio_table(data, 1);
    if (rc < 0) {
        pr_err("%s: request gpio failed\n", __func__);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }

    rc = msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
            s_ctrl->sensordata->sensor_platform_info->cam_vreg,
            s_ctrl->sensordata->sensor_platform_info->num_vreg,
            s_ctrl->reg_ptr, 1);
    if (rc < 0) {
        pr_err("%s: regulator on failed\n", __func__);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }

    rc = msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
            s_ctrl->sensordata->sensor_platform_info->cam_vreg,
            s_ctrl->sensordata->sensor_platform_info->num_vreg,
            s_ctrl->reg_ptr, 1);
    if (rc < 0) {
        pr_err("%s: enable regulator failed\n", __func__);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }

    rc = gpio_request(IMX111_GPIO_CAM_V_EN3, SENSOR_NAME);
    if (rc < 0) {
        CDBG("IMX111_GPIO_CAM_V_EN3(%d) Error, rc = %d\n", IMX111_GPIO_CAM_V_EN3, rc);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }
    /* CAM_V_EN3 On */
    gpio_direction_output(IMX111_GPIO_CAM_V_EN3, 1);

    /* VREG_L12 On */
	if (cam_vdig == NULL) {
		cam_vdig = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vdig");
		if (IS_ERR(cam_vdig)) {
			CDBG("%s: VREG CAM VDIG get failed\n", __func__);
			cam_vdig = NULL;
			rc = -1;
		}
		if (regulator_set_voltage(cam_vdig, CAM_VDIG_MINUV,
			CAM_VDIG_MAXUV)) {
			CDBG("%s: VREG CAM VDIG set voltage failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_set_optimum_mode(cam_vdig,
			CAM_VDIG_LOAD_UA) < 0) {
			CDBG("%s: VREG CAM VDIG set optimum mode failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_enable(cam_vdig)) {
			CDBG("%s: VREG CAM VDIG enable failed\n", __func__);
			rc = -1;
		}
	}

    /* wait 1msec */
    mdelay(IMX111_WAIT_PWON_EN2);

    /* CAM_V_EN2(PM GPIO) ON */
    rc = pm8xxx_gpio_config(IMX111_PMGPIO_CAM_V_EN2, &imx111_cam_v_en2_on);
    if (rc) {
        /* POWER OFF */
        CDBG("IMX111_PMGPIO_CAM_V_EN2(%d) Error, rc = %d\n", IMX111_PMGPIO_CAM_V_EN2, rc);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }

#ifndef CONFIG_FEATURE_NCMC_D121M
    rc = gpio_request(IMX111_GPIO_CAM_I2C_SDA, SENSOR_NAME);
    if (rc < 0) {
        CDBG("IMX111_GPIO_CAM_I2C_SDA(%d) Error, rc = %d\n", IMX111_GPIO_CAM_V_EN3, rc);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }
    rc = gpio_request(IMX111_GPIO_CAM_I2C_SCL, SENSOR_NAME);
    if (rc < 0) {
        CDBG("IMX111_GPIO_CAM_I2C_SCL(%d) Error, rc = %d\n", IMX111_GPIO_CAM_V_EN3, rc);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }
#endif

    /* wait 1msec */
    mdelay(IMX111_WAIT_PWON_EN1);
    /* CAM_V_EN1 On */
    rc = gpio_request(IMX111_GPIO_CAM_V_EN1, SENSOR_NAME);
    if (!rc) {
        CDBG("%s: cam_v_en on\n", __func__);
        gpio_direction_output(IMX111_GPIO_CAM_V_EN1, 1);
    } else {
        /* POWER OFF */
        CDBG("VDD_CAM2_V_EN1(%d) Error, rc = %d\n", IMX111_GPIO_CAM_V_EN3, rc);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }
    
    /* wait 1msec */
    mdelay(IMX111_WAIT_PWON_VREG_L11);
    /* VREG_L11 On */
	if (cam_vana == NULL) {
		cam_vana = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vana");
		if (IS_ERR(cam_vana)) {
			CDBG("%s: VREG CAM VANA get failed\n", __func__);
			rc = -1;
		}
		if (regulator_set_voltage(cam_vana, CAM_VANA_MINUV,
			CAM_VANA_MAXUV)) {
			CDBG("%s: VREG CAM VANA set voltage failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_set_optimum_mode(cam_vana,
			CAM_VANA_LOAD_UA) < 0) {
			CDBG("%s: VREG CAM VANA set optimum mode failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_enable(cam_vana)) {
			CDBG("%s: VREG CAM VANA enable failed\n", __func__);
			rc = -1;
		}
	}
    if (rc < 0) {
        CDBG("cam_vana enable Error, rc = %d\n", rc);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }

    /* wait 1msec */
    mdelay(IMX111_WAIT_PWON_RST_H);
	rc = gpio_request(data->sensor_platform_info->sensor_reset, SENSOR_NAME);
	if (!rc) {
		CDBG("%s: reset sensor\n", __func__);
		gpio_direction_output(data->sensor_platform_info->sensor_reset, 1);
	} else {
		CDBG("%s: gpio request fail", __func__);
	}

    /* wait 1msec */
    udelay(IMX111_WAIT_PWON_CLK);

/* 1032-MERGE */
//	msm_camio_clk_rate_set(MSM_SENSOR_MCLK_25HZ);

    if (s_ctrl->clk_rate != 0)
        imx111_cam_clk_info->clk_rate = s_ctrl->clk_rate;

    rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
        imx111_cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(imx111_cam_clk_info), 1);
    if (rc < 0) {
        pr_err("%s: clk enable failed\n", __func__);
        msm_sensor_imx111_power_down(s_ctrl);
        return -EFAULT;
    }
/* 1032-MERGE */

    /* CAM_MCLK On */
    gpio_direction_output(IMX111_GPIO_CAM_MCLK0, 1);

    /* wait 10msec */
    mdelay(IMX111_WAIT_PWON_CLK_2);
	printk("imx111.c %s: end\n", __func__);
	return rc;
}

void imx111_set_dev_addr(struct msm_camera_eeprom_client *eclient,
	uint16_t *reg_addr) {
	uint16_t eprom_addr = *reg_addr;
	if ((eprom_addr >= 0x3500) && (eprom_addr < 0x3508)) {
		msm_camera_i2c_write(eclient->i2c_client,
			IMX111_EEPROM_BANK_SEL_REG,
			0x00, MSM_CAMERA_I2C_BYTE_DATA);
	}
	if ((eprom_addr >= 0x3508) && (eprom_addr < 0x3510)) {
		msm_camera_i2c_write(eclient->i2c_client,
			IMX111_EEPROM_BANK_SEL_REG,
			0x01, MSM_CAMERA_I2C_BYTE_DATA);
	}
	if ((eprom_addr >= 0x3510) && (eprom_addr < 0x3518)) {
		msm_camera_i2c_write(eclient->i2c_client,
			IMX111_EEPROM_BANK_SEL_REG,
			0x02, MSM_CAMERA_I2C_BYTE_DATA);
	}
}

static const struct i2c_device_id imx111_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&imx111_s_ctrl},
	{ }
};

static struct i2c_driver imx111_i2c_driver = {
	.id_table = imx111_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx111_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static uint8_t sensvty_calib_data[6];
static uint8_t af_calib_data[8];

static struct msm_camera_eeprom_read_t imx111_eeprom_read_tbl[] = {
	{0x3508, &sensvty_calib_data, 6, 0},
	{0x350E, &af_calib_data[0], 2, 0},
	{0x3510, &af_calib_data[2], 6, 0},
};

static struct msm_camera_eeprom_data_t imx111_eeprom_data_tbl[] = {
	{&sensvty_calib_data, sizeof(sensvty_calib_data)},
	{&af_calib_data, sizeof(af_calib_data)},
};

static struct msm_camera_eeprom_client imx111_eeprom_client = {
	.i2c_client = &imx111_sensor_i2c_client,
	.i2c_addr = 0x34,

	.func_tbl = {
		.eeprom_set_dev_addr = imx111_set_dev_addr,
		.eeprom_init = NULL,
		.eeprom_release = NULL,
		.eeprom_get_data = msm_camera_eeprom_get_data,
	},

	.read_tbl = imx111_eeprom_read_tbl,
	.read_tbl_size = ARRAY_SIZE(imx111_eeprom_read_tbl),
	.data_tbl = imx111_eeprom_data_tbl,
	.data_tbl_size = ARRAY_SIZE(imx111_eeprom_data_tbl),
};

static int curr_frame_length_lines = 0; 

int32_t imx111_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
    int linecount;
    int frame_time,exposure_time; // msec
    struct msm_sensor_output_info_t curr_info,new_info;

	printk("imx111.c %s: start %d\n", __func__,res);

	curr_info = imx111_dimensions[s_ctrl->curr_res];
    new_info  = imx111_dimensions[res];
    if( res != 0)
    {
    	v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
    		NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
    		PIX_0, ISPIF_OFF_IMMEDIATELY));
    	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
    }
//	msleep(30);

	if (update_type == MSM_SENSOR_REG_INIT) {
	    curr_frame_length_lines = 0;
		s_ctrl->curr_csi_params = NULL;
        rc = msm_sensor_enable_debugfs(s_ctrl);
        if(rc < 0){
            /* NO RETURN */
            CDBG("%s: msm_sensor_enable_debugfs (rc:%d)\n", __func__,rc);
        }
        rc = msm_sensor_write_init_settings(s_ctrl);
        if(rc < 0){
            printk("%s: msm_sensor_write_init_settings (rc:%d)\n", __func__,rc);
            return rc;
        }


	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {

        if( s_ctrl->curr_res != MSM_SENSOR_INVALID_RES)
        {
	        CDBG("imx111.c %s: setting change %d -> %d\n", __func__,s_ctrl->curr_res,res);
            if (curr_info.frame_length_lines > curr_frame_length_lines )
            {
                linecount = curr_info.frame_length_lines;
            }else{
                linecount = curr_frame_length_lines;
            }
            frame_time = 1000/(curr_info.vt_pixel_clk/(curr_info.line_length_pclk*linecount));

            if( res != 0 )
            {
                /* wait one frame time */
                CDBG("imx111.c %s: current frame_out_time = %d line_length_pclk =%d linecount = %d vt_pixel_clk = %d\n", __func__,frame_time,curr_info.line_length_pclk,linecount,curr_info.vt_pixel_clk);
                msleep(frame_time);
            }
        }

        /* set new mode default frame length lines setting */
        curr_frame_length_lines = new_info.frame_length_lines;
        exposure_time = 1000/(new_info.vt_pixel_clk/(new_info.line_length_pclk*new_info.frame_length_lines));

	    if( s_ctrl->curr_res != 0 && res == 0)
	    {
            /* use shutter lag shortening setting */
            /* comm 1 */
		    rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
			    (struct msm_camera_i2c_reg_conf *)
			    imx111_comm_confs[0].conf,
			    imx111_comm_confs[0].size,
			    imx111_comm_confs[0].data_type);

            if(rc < 0){
                return rc;
            }

            /* comm 2 */
			rc = msm_sensor_write_res_settings(s_ctrl, res);

            if(rc < 0){
                return rc;
            }

			if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {
				s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_CSID_CFG,
					&s_ctrl->curr_csi_params->csid_params);
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_CID_CHANGE, NULL);
				mb();
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_CSIPHY_CFG,
					&s_ctrl->curr_csi_params->csiphy_params);
				mb();
				msleep(20);
			}
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
				output_settings[res].op_pixel_clk);
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
				PIX_0, ISPIF_ON_FRAME_BOUNDARY));

			rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
				(struct msm_camera_i2c_reg_conf *)
				imx111_comm_confs[1].conf,
				imx111_comm_confs[1].size,
				imx111_comm_confs[1].data_type);

            if(rc < 0){
                return rc;
            }

			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
			msleep(10);

            /* comm 3 */
			rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
				(struct msm_camera_i2c_reg_conf *)
				imx111_comm_confs[2].conf,
				imx111_comm_confs[2].size,
				imx111_comm_confs[2].data_type);

            if(rc < 0){
                return rc;
            }

        } else {
			rc = msm_sensor_write_res_settings(s_ctrl, res);

           if(rc < 0){
                return rc;
            }

			if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {
				s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_CSID_CFG,
					&s_ctrl->curr_csi_params->csid_params);
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_CID_CHANGE, NULL);
				mb();
				v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_CSIPHY_CFG,
					&s_ctrl->curr_csi_params->csiphy_params);
				mb();
				msleep(20);
			}
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
				output_settings[res].op_pixel_clk);
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
				PIX_0, ISPIF_ON_FRAME_BOUNDARY));
			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
//			msleep(30);
			msleep(exposure_time);
		}
	}
	printk("imx111.c %s: end\n", __func__);
	return rc;
}

int32_t imx111_sensor_write_exp_gain1(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	int32_t rc = 0;
	uint32_t fl_lines;
	uint8_t offset;
	fl_lines = s_ctrl->curr_frame_length_lines;
	fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;

    curr_frame_length_lines = fl_lines;
	CDBG("\n%s:Gain:%d, Linecount:%d\n", __func__, gain, line);
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			fl_lines, MSM_CAMERA_I2C_WORD_DATA);

        if(rc < 0){
            return rc;
        }

		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			line, MSM_CAMERA_I2C_WORD_DATA);

        if(rc < 0){
            return rc;
        }

    if(rc >= 0){
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
			MSM_CAMERA_I2C_WORD_DATA);

        if(rc < 0){
            return rc;
        }
    }
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
//	}

	return rc;
}

/*=============================================================
    GET MAKER NOTE
==============================================================*/
static int32_t imx111_get_maker_note(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_maker_note_cfg *get_exif_maker_note)
{

// set all
    get_exif_maker_note->fd_freq        = 0x0001; 
// set front camera only
    get_exif_maker_note->device_id      = imx111_id_info.sensor_id;
    get_exif_maker_note->awb_temp       = 0x0002;
    get_exif_maker_note->awb_gain_r     = 0x0003;
    get_exif_maker_note->awb_gain_g     = 0x0004;
    get_exif_maker_note->awb_gain_b     = 0x0005;
    get_exif_maker_note->awb_saturation = 0x0006;
// set back camera only
	get_exif_maker_note->calib_rg           = *(uint16_t*)&otp_data[0x08];
	get_exif_maker_note->calib_bg           = *(uint16_t*)&otp_data[0x0A];
	get_exif_maker_note->calib_grgb         = *(uint16_t*)&otp_data[0x0C];
	get_exif_maker_note->af_inf_position    = *(uint16_t*)&otp_data[0x0E];
	get_exif_maker_note->af_1m_position     = *(uint16_t*)&otp_data[0x10];
	get_exif_maker_note->af_macro_position  = *(uint16_t*)&otp_data[0x12];
	get_exif_maker_note->start_current      = *(uint16_t*)&otp_data[0x14];
	get_exif_maker_note->op_sensitivity     = *(uint16_t*)&otp_data[0x16];
	get_exif_maker_note->trial_ver          = *(uint16_t*)&otp_data[0x06];
	get_exif_maker_note->lot_code_date      = *(uint16_t*)&otp_data[0x72];
	get_exif_maker_note->lot_code_num       = *(uint32_t*)&otp_data[0x74];

    return 0;
}

/*=============================================================
    GET EXIF PARAM
==============================================================*/
static int32_t imx111_get_exif_param(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_param_inf *get_exif_param)
{
    int32_t rc = 0;
    
    uint16_t coarse_integration_time = 0x0000; /* reg 0x0202 TBD */
    uint16_t line_length_pck         = 0x0000; /* reg 0x0342 TBD */
    uint16_t fine_integration_time   = 0x0000; /* TBD */
    uint16_t analog_gain_code_global = 0x0000; /* reg 0x0204 TBD */
    uint16_t digital_gain_greenr     = 0x0000; /* TBD */

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             0x0202, &coarse_integration_time, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             0x0342, &line_length_pck, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             0x0204, &analog_gain_code_global, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    get_exif_param->coarse_integration_time = coarse_integration_time; /* TBD */
    get_exif_param->line_length_pck         = line_length_pck;         /* TBD */
    get_exif_param->fine_integration_time   = fine_integration_time;   /* TBD */
    get_exif_param->analog_gain_code_global = analog_gain_code_global; /* TBD */
    get_exif_param->digital_gain_greenr     = digital_gain_greenr;     /* TBD */

    return rc;
}

/*=============================================================
    GET EEPROM OTP INFO
==============================================================*/
static int32_t imx111_get_eeprom_otp_info(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct eeprom_otp_info_t *eeprom_otp_info)
{

    eeprom_otp_info->otp_bank00 = *(uint64_t*)&otp_data[0x00];
    eeprom_otp_info->otp_bank01 = *(uint64_t*)&otp_data[0x08];
    eeprom_otp_info->otp_bank02 = *(uint64_t*)&otp_data[0x10];
    eeprom_otp_info->otp_bank03 = *(uint64_t*)&otp_data[0x18];
    eeprom_otp_info->otp_bank04 = *(uint64_t*)&otp_data[0x20];
    eeprom_otp_info->otp_bank05 = *(uint64_t*)&otp_data[0x28];
    eeprom_otp_info->otp_bank06 = *(uint64_t*)&otp_data[0x30];
    eeprom_otp_info->otp_bank07 = *(uint64_t*)&otp_data[0x38];
    eeprom_otp_info->otp_bank08 = *(uint64_t*)&otp_data[0x40];
    eeprom_otp_info->otp_bank09 = *(uint64_t*)&otp_data[0x48];
    eeprom_otp_info->otp_bank10 = *(uint64_t*)&otp_data[0x50];
    eeprom_otp_info->otp_bank11 = *(uint64_t*)&otp_data[0x58];
    eeprom_otp_info->otp_bank12 = *(uint64_t*)&otp_data[0x60];
    eeprom_otp_info->otp_bank13 = *(uint64_t*)&otp_data[0x68];
    eeprom_otp_info->otp_bank14 = *(uint64_t*)&otp_data[0x70];
    eeprom_otp_info->otp_bank15 = *(uint64_t*)&otp_data[0x78];
    return 0;
}

/*=============================================================
    GET DEVICE ID
==============================================================*/
static int32_t imx111_get_device_id(struct msm_sensor_ctrl_t *s_ctrl, uint16_t *device_id)
{
    int32_t rc = 0;
    uint16_t chipid = 0xFFFF;

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid, 
                             MSM_CAMERA_I2C_WORD_DATA);
    if (rc < 0) {
        pr_err("%s: msm_camera_i2c_read failed rc=%d\n", __func__,rc);
        return rc;
    }

    *device_id = chipid;

    return rc;
}


static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&imx111_i2c_driver);
}

static struct v4l2_subdev_core_ops imx111_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};
static struct v4l2_subdev_video_ops imx111_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops imx111_subdev_ops = {
	.core = &imx111_subdev_core_ops,
	.video  = &imx111_subdev_video_ops,
};

static struct msm_sensor_fn_t imx111_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = imx111_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = imx111_sensor_write_exp_gain1,
	.sensor_setting = imx111_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_imx111_power_up,
/*	.sensor_power_down = msm_sensor_power_down, */
	.sensor_power_down = msm_sensor_imx111_power_down,
	.sensor_set_parm_pm_obs = msm_sensor_set_parm_pm_obs,
    .sensor_get_maker_note = imx111_get_maker_note,
    .sensor_get_exif_param = imx111_get_exif_param,
    .sensor_get_eeprom_otp_info = imx111_get_eeprom_otp_info,
    .sensor_otp_read = imx111_sensor_otp_read,
    .sensor_get_device_id = imx111_get_device_id,
};

static struct msm_sensor_reg_t imx111_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = imx111_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(imx111_start_settings),
	.stop_stream_conf = imx111_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(imx111_stop_settings),
	.group_hold_on_conf = imx111_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(imx111_groupon_settings),
	.group_hold_off_conf = imx111_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(imx111_groupoff_settings),
	.init_settings = &imx111_init_conf[0],
	.init_size = ARRAY_SIZE(imx111_init_conf),
	.mode_settings = &imx111_confs[0],
	.output_settings = &imx111_dimensions[0],
	.num_conf = ARRAY_SIZE(imx111_confs),
};

static struct msm_sensor_ctrl_t imx111_s_ctrl = {
	.msm_sensor_reg = &imx111_regs,
	.sensor_i2c_client = &imx111_sensor_i2c_client,
	.sensor_i2c_addr = 0x34,
	.sensor_eeprom_client = &imx111_eeprom_client,
	.sensor_output_reg_addr = &imx111_reg_addr,
	.sensor_id_info = &imx111_id_info,
	.sensor_exp_gain_info = &imx111_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &imx111_csi_params_array[0],
	.msm_sensor_mutex = &imx111_mut,
	.sensor_i2c_driver = &imx111_i2c_driver,
	.sensor_v4l2_subdev_info = imx111_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx111_subdev_info),
	.sensor_v4l2_subdev_ops = &imx111_subdev_ops,
	.func_tbl = &imx111_func_tbl,
/* 1032-MERGE */
	.clk_rate = MSM_SENSOR_MCLK_25HZ,
/* 1032-MERGE */
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Sony 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
