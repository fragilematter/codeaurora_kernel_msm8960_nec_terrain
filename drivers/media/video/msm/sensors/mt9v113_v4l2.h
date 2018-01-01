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

#ifndef MT9V113_V4L2_H
#define MT9V113_V4L2_H


#include "msm_sensor.h"
#include "msm.h"
#include "msm_ispif.h"
#include <mach/irqs.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/clk.h>

#define SENSOR_NAME "mt9v113"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9v113"
#define mt9v113_obj mt9v113_##obj

/*=============================================================
    DEFINES
==============================================================*/
/* PM8921 */
#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)

/* MCLK FREQUENCY */
//#define MT9V113_MCLK_FREQUENCY          25600000 /* 25.6MHz */
#define MT9V113_MCLK_FREQUENCY          36571000

/* POWER ON WAIT TIME */
/* usec */
#define MT9V113_WAIT_PWON_I2C_ACCESS    500     /* 500us */
/* msec  */
#define MT9V113_WAIT_PWON_RST_H         5       /* 2ms */
#define MT9V113_WAIT_PWON_V_EN2         1       /* 1ms */
#define MT9V113_WAIT_PWON_V_EN1         2       /* 2ms */
#define MT9V113_WAIT_PWON_RST_L         200     /* 200 ms */
#define MT9V113_WAIT_PWON_MCLK          1       /* 1ms */

/* POWER OFF WAIT TIME */
/* msec  */
#define MT9V113_WAIT_PWOFF_MCLK         5        /*   5ms */
#define MT9V113_WAIT_PWOFF_RST_L        10       /*  10ms */
#define MT9V113_WAIT_PWOFF_V_EN1        10       /*  10ms */

/* MSM GPIO */
#define MT9V113_GPIO_CAM2_MCLK          4        /* GPIO[4]  */
#define MT9V113_GPIO_VDD_CAM2_V_EN2     2        /* GPIO[2]  */
#define MT9V113_GPIO_I2C2_SDA           12       /* GPIO[12] */
#define MT9V113_GPIO_I2C2_SCL           13       /* GPIO[13] */
#define MT9V113_GPIO_CAM2_RST_N         43       /* GPIO[43] */

/* PM GPIO */
#define  MT9V113_PMGPIO_VDD_CAM2_V_EN1  PM8921_GPIO_PM_TO_SYS(42)   /* GPIO[42] */

/* REFRESH */
#define MT9V113_REFRESH_MODE_SETTING   0        /* Index of refresh_confs[] */
#define MT9V113_REFRESH_CMD_SETTING    1        /* Index of refresh_confs[] */

/* MCU */
#define MT9V113_MCU_ADDRESS    0x098C
#define MT9V113_MCU_DATA0      0x0990

/* POLLING */
#define MT9V113_POLL_STRM_ADDR           0x301A
#define MT9V113_POLL_STRM_MASK           0x0004
#define MT9V113_POLL_STRM_TERMINATE      0x0004
#define MT9V113_POLL_STRM_INTERVAL       50       /* [ms]     */
#define MT9V113_POLL_STRM_RETRY          20       /* [times] */

#define MT9V113_POLL_STNDBY_ADDR         0x0018
#define MT9V113_POLL_STNDBY_MASK         0x4000
#define MT9V113_POLL_STNDBY_TERMINATE    0x0000
#define MT9V113_POLL_STNDBY_INTERVAL     10       /* [ms]   */
#define MT9V113_POLL_STNDBY_RETRY        100      /* [times] */

#define MT9V113_POLL_RFRSH_ADDR          0xA103
#define MT9V113_POLL_RFRSH_MASK          0x000F
#define MT9V113_POLL_RFRSH_TERMINATE     0x0000
#define MT9V113_POLL_RFRSH_INTERVAL      10       /* [ms]    */
#define MT9V113_POLL_RFRSH_RETRY_50      50       /* [times] */
#define MT9V113_POLL_RFRSH_RETRY_100     100      /* [times] */


/* EXIF */
enum mt9v113_access_t{
    MT9V113_ACCESS_REGISTERS,
    MT9V113_ACCESS_VARIABLES,
};

struct read_exif_param_t{
    enum mt9v113_access_t reg_type;
    uint16_t  reg_addr;
    uint16_t* reg_data;
};


DEFINE_MUTEX(mt9v113_mut);
static struct msm_sensor_ctrl_t mt9v113_s_ctrl;


static struct msm_camera_i2c_reg_conf mt9v113_video_prev_settings[] = {
    //[640x480 30fps Fixed]
    {0x098C, 0x271F},//Frame Lines (A)
    {0x0990, 0x01FB},//      = 507
    {0x098C, 0x2721},//Line Length (A)
    {0x0990, 0x0384},//      = 900
    {0x098C, 0x2735},//Frame Lines (B)
    {0x0990, 0x01FB},//      = 507
    {0x098C, 0x2737},//Line Length (B)
    {0x0990, 0x0384},//      = 900
    {0x098C, 0xA20B},// MCU_ADDRESS [AE_MIN_INDEX]
    {0x0990, 0x0004},// MCU_DATA_0
    {0x098C, 0xA20C},// MCU_ADDRESS [AE_MAX_INDEX]
    {0x0990, 0x0004},// MCU_DATA_0
};


static struct msm_camera_i2c_reg_conf mt9v113_picture_prev_settings[] = {

    //[640x480 Variable 7~15 fps]
    {0x098C, 0x271F},   // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
    {0x0990, 0x01FB},   // MCU_DATA_0
    {0x098C, 0x2721},   // MCU_ADDRESS [MODE_SENSOR_LINE_LENGTH_PCK_A]
    {0x0990, 0x0398},   // MCU_DATA_0
    {0x098C, 0x2735},   // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_B]
    {0x0990, 0x01FB},   // MCU_DATA_0
    {0x098C, 0x2737},   // MCU_ADDRESS [MODE_SENSOR_LINE_LENGTH_PCK_B]
    {0x0990, 0x0398},   // MCU_DATA_0
    {0x098C, 0xA20B},   // MCU_ADDRESS [AE_INDEX]
    {0x0990, 0x0010},   // MCU_DATA_0
    {0x098C, 0xA20C},   // MCU_ADDRESS [AE_MAX_INDEX]
    {0x0990, 0x0010},   // MCU_DATA_0
};


static struct msm_camera_i2c_reg_conf mt9v113_init_settings1[] = {
    {0x001A, 0x0011},    // RESET_AND_MISC_CONTROL
//DELAY = 10
};
static struct msm_camera_i2c_reg_conf mt9v113_init_settings1_1[] = {
    {0x001A, 0x0010},    // RESET_AND_MISC_CONTROL
//DELAY = 10
};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings2[] = {
    {0x0018, 0x4028},    // STANDBY_CONTROL_AND_STATUS
};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings4[] = {
{0x321C, 0x0003 },	// OFIFO_CONTROL_STATUS
{0x001E, 0x0777 },	// PAD_SLEW
{0x0016, 0x42DF },	// CLOCKS_CONTROL
};


static struct msm_camera_i2c_reg_conf mt9v113_init_settings4_1[] = {
{0x0014, 0x2145},	// PLL_CONTROL
{0x0014, 0x2145},	// PLL_CONTROL
//0711 PLL setting N=7 M=49
{0x0010, 0x0731},	// PLL_DIVIDERS
{0x0012, 0x0000},	// PLL_P_DIVIDERS
{0x0014, 0x244B}, 	// PLL_CONTROL
//DELAY=10
};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings4_2[] = {
{0x0014, 0x304B },	// PLL_CONTROL
//DELAY=10

};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings4_3[] = {
    {0x0014, 0xB04A},    //BITFIELD= 0x14, 1, 0 //PLL_BYPASS_OFF
};


static struct msm_camera_i2c_reg_conf mt9v113_init_settings6[] = {

{0x098C, 0x2703},	//Output Width (A)
{0x0990, 0x0280},	//      = 640
{0x098C, 0x2705},	//Output Height (A)
{0x0990, 0x01E0},	//      = 480
{0x098C, 0x2707},	//Output Width (B)
{0x0990, 0x0280},	//      = 640
{0x098C, 0x2709},	//Output Height (B)
{0x0990, 0x01E0},	//      = 480
{0x098C, 0x270D},	//Row Start (A)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x270F},	//Column Start (A)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x2711},	//Row End (A)
{0x0990, 0x01E7},	//      = 487
{0x098C, 0x2713},	//Column End (A)
{0x0990, 0x0287},	//      = 647
{0x098C, 0x2715},	//Row Speed (A)
{0x0990, 0x0001},	//      = 1
{0x098C, 0x2717},	//Read Mode (A)
{0x0990, 0x0026},	//      = 38
{0x098C, 0x2719},	//sensor_fine_correction (A)
{0x0990, 0x001A},	//      = 26
{0x098C, 0x271B},	//sensor_fine_IT_min (A)
{0x0990, 0x006B},	//      = 107
{0x098C, 0x271D},	//sensor_fine_IT_max_margin (A)
{0x0990, 0x006B},	//      = 107
{0x098C, 0x271F},	//Frame Lines (A)
{0x0990, 0x01FB},	//      = 507
{0x098C, 0x2721},	//Line Length (A)
{0x0990, 0x0398},	//      = 920
{0x098C, 0x2723},	//Row Start (B)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x2725},	//Column Start (B)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x2727},	//Row End (B)
{0x0990, 0x01E7},	//      = 487
{0x098C, 0x2729},	//Column End (B)
{0x0990, 0x0287},	//      = 647
{0x098C, 0x272B},	//Row Speed (B)
{0x0990, 0x0001},	//      = 1
{0x098C, 0x272D},	//Read Mode (B)
{0x0990, 0x0026},	//      = 38
{0x098C, 0x272F},	//sensor_fine_correction (B)
{0x0990, 0x001A},	//      = 26
{0x098C, 0x2731},	//sensor_fine_IT_min (B)
{0x0990, 0x006B},	//      = 107
{0x098C, 0x2733},	//sensor_fine_IT_max_margin (B)
{0x0990, 0x006B},	//      = 107
{0x098C, 0x2735},	//Frame Lines (B)
{0x0990, 0x01FB},	//      = 507
{0x098C, 0x2737},	//Line Length (B)
{0x0990, 0x0398},	//      = 920
{0x098C, 0x2739},	//Crop_X0 (A)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x273B},	//Crop_X1 (A)
{0x0990, 0x027F},	//      = 639
{0x098C, 0x273D},	//Crop_Y0 (A)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x273F},	//Crop_Y1 (A)
{0x0990, 0x01DF},	//      = 479
{0x098C, 0x2747},	//Crop_X0 (B)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x2749},	//Crop_X1 (B)
{0x0990, 0x027F},	//      = 639
{0x098C, 0x274B},	//Crop_Y0 (B)
{0x0990, 0x0000},	//      = 0
{0x098C, 0x274D},	//Crop_Y1 (B)
{0x0990, 0x01DF},	//      = 479
{0x098C, 0x222D},	//R9 Step
{0x0990, 0x007F},	//      = 127
{0x098C, 0xA408},	//search_f1_50
{0x0990, 0x001E},	//      = 30
{0x098C, 0xA409},	//search_f2_50
{0x0990, 0x0020},	//      = 32
{0x098C, 0xA40A},	//search_f1_60
{0x0990, 0x0025},	//      = 37
{0x098C, 0xA40B},	//search_f2_60
{0x0990, 0x0027},	//      = 39
{0x098C, 0x2411},	//R9_Step_60 (A)
{0x0990, 0x007F},	//      = 127
{0x098C, 0x2413},	//R9_Step_50 (A)
{0x0990, 0x0098},	//      = 152
{0x098C, 0x2415},	//R9_Step_60 (B)
{0x0990, 0x007F},	//      = 127
{0x098C, 0x2417},	//R9_Step_50 (B)
{0x0990, 0x0098},	//      = 152
{0x098C, 0xA404},	//FD Mode
{0x0990, 0x0010},	//      = 16
{0x098C, 0xA40D},	//Stat_min
{0x0990, 0x0002},	//      = 2
{0x098C, 0xA40E},	//Stat_max
{0x0990, 0x0003},	//      = 3
{0x098C, 0xA410},	//Min_amplitude
{0x0990, 0x000A},	//      = 10
//POLL_REG = 0x301A, 0x0004, !=1, DELAY=10, TIMEOUT=100	//verify streaming bit is high

};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings3[] = {

//reduce_IO_current
{ 0x098C, 0x02F0}, 	// MCU_ADDRESS
{ 0x0990, 0x0000}, 	// MCU_DATA_0
{ 0x098C, 0x02F2}, 	// MCU_ADDRESS
{ 0x0990, 0x0210}, 	// MCU_DATA_0
{ 0x098C, 0x02F4}, 	// MCU_ADDRESS
{ 0x0990, 0x001A}, 	// MCU_DATA_0
{ 0x098C, 0x2145}, 	// MCU_ADDRESS [RESERVED_SEQ_45]
{ 0x0990, 0x02F4}, 	// MCU_DATA_0
{ 0x098C, 0xA134}, 	// MCU_ADDRESS [RESERVED_SEQ_34]
{ 0x0990, 0x0001}, 	// MCU_DATA_0
{ 0x31E0, 0x0001}, 	// RESERVED_CORE_31E0

// {0x001A, 0x0010 },	// RESET_AND_MISC_CONTROL  // Removed in version1.1 init settings
// {0x3400, 0x7A28 },	// MIPI_CONTROL
// {0x321C, 0x0003 },	// OFIFO_CONTROL_STATUS

// {0x001E, 0x0777 },	// PAD_SLEW
// {0x0016, 0x42DF },	// CLOCKS_CONTROL

};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings8[] = {

//MIPI_enable
//0709 MIPI noncontinuous clock setting
{ 0x3400, 0x7A28},	// MIPI_CONTROL
//0709 MIPI noncontinuous clock setting
{ 0x001A, 0x0010}, 	// RESET_AND_MISC_CONTROL

//[Image broken improve code]
//[Gamma]
{ 0x098C, 0xA111},    // MCU_ADDRESS [SEQ_OPTIONS]
{ 0x0990, 0x000A},    // MCU_DATA_0

//[Gamma]
{ 0x098C, 0xAB37},        //MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
{ 0x0990, 0x0001}, 	// MCU_DATA_0
{ 0x098C, 0x2B38},	//MCU_ADDRESS [HG_GAMMASTARTMORPH]
{ 0x0990, 0x1000},	//MCU_DATA_0
{ 0x098C, 0x2B3A},	//MCU_ADDRESS [HG_GAMMASTOPMORPH]
{ 0x0990, 0x2000},	//MCU_DATA_0
{ 0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
{ 0x0990, 0x0000}, 	// MCU_DATA_0
{ 0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
{ 0x0990, 0x001E}, 	// MCU_DATA_0
{ 0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
{ 0x0990, 0x0031}, 	// MCU_DATA_0
{ 0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
{ 0x0990, 0x0048}, 	// MCU_DATA_0
{ 0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
{ 0x0990, 0x0068}, 	// MCU_DATA_0
{ 0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
{ 0x0990, 0x0081}, 	// MCU_DATA_0
{ 0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
{ 0x0990, 0x0095}, 	// MCU_DATA_0
{ 0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
{ 0x0990, 0x00A6}, 	// MCU_DATA_0
{ 0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
{ 0x0990, 0x00B3}, 	// MCU_DATA_0
{ 0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
{ 0x0990, 0x00BF}, 	// MCU_DATA_0
{ 0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
{ 0x0990, 0x00C9}, 	// MCU_DATA_0
{ 0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
{ 0x0990, 0x00D2}, 	// MCU_DATA_0
{ 0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
{ 0x0990, 0x00DA}, 	// MCU_DATA_0
{ 0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
{ 0x0990, 0x00E2}, 	// MCU_DATA_0
{ 0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
{ 0x0990, 0x00E8}, 	// MCU_DATA_0
{ 0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
{ 0x0990, 0x00EF}, 	// MCU_DATA_0
{ 0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
{ 0x0990, 0x00F4}, 	// MCU_DATA_0
{ 0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
{ 0x0990, 0x00FA}, 	// MCU_DATA_0
{ 0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
{ 0x0990, 0x00FF}, 	// MCU_DATA_0

};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings9[] = {
//[Lens Correction 85]
{0x364E, 0x0510},	// P_GR_P0Q0
{0x3650, 0x968D},	// P_GR_P0Q1
{0x3652, 0x4A72},	// P_GR_P0Q2
{0x3654, 0xAF70},	// P_GR_P0Q3
{0x3656, 0x8611},	// P_GR_P0Q4
{0x3658, 0x01D0},	// P_RD_P0Q0
{0x365A, 0xE8EC},	// P_RD_P0Q1
{0x365C, 0x49D2},	// P_RD_P0Q2
{0x365E, 0x93D0},	// P_RD_P0Q3
{0x3660, 0x3ACF},	// P_RD_P0Q4
{0x3662, 0x0290},	// P_BL_P0Q0
{0x3664, 0xB92D},	// P_BL_P0Q1
{0x3666, 0x3A32},	// P_BL_P0Q2
{0x3668, 0xA690},	// P_BL_P0Q3
{0x366A, 0x8831},	// P_BL_P0Q4
{0x366C, 0x0450},	// P_GB_P0Q0
{0x366E, 0xA3AD},	// P_GB_P0Q1
{0x3670, 0x5092},	// P_GB_P0Q2
{0x3672, 0x92B0},	// P_GB_P0Q3
{0x3674, 0xDEF1},	// P_GB_P0Q4
{0x3676, 0x6ECE},	// P_GR_P1Q0
{0x3678, 0xFE4D},	// P_GR_P1Q1
{0x367A, 0xABEF},	// P_GR_P1Q2
{0x367C, 0x32AF},	// P_GR_P1Q3
{0x367E, 0x3032},	// P_GR_P1Q4
{0x3680, 0x0DEF},	// P_RD_P1Q0
{0x3682, 0xC0AB},	// P_RD_P1Q1
{0x3684, 0xF970},	// P_RD_P1Q2
{0x3686, 0x8F91},	// P_RD_P1Q3
{0x3688, 0x0AD5},	// P_RD_P1Q4
{0x368A, 0x0B8F},	// P_BL_P1Q0
{0x368C, 0x06ED},	// P_BL_P1Q1
{0x368E, 0x972D},	// P_BL_P1Q2
{0x3690, 0x8D72},	// P_BL_P1Q3
{0x3692, 0xE8B0},	// P_BL_P1Q4
{0x3694, 0x70EE},	// P_GB_P1Q0
{0x3696, 0x856E},	// P_GB_P1Q1
{0x3698, 0x344F},	// P_GB_P1Q2
{0x369A, 0x3CD0},	// P_GB_P1Q3
{0x369C, 0xA4D2},	// P_GB_P1Q4
{0x369E, 0x04F3},	// P_GR_P2Q0
{0x36A0, 0xDD91},	// P_GR_P2Q1
{0x36A2, 0xBEB4},	// P_GR_P2Q2
{0x36A4, 0x5C94},	// P_GR_P2Q3
{0x36A6, 0x3E38},	// P_GR_P2Q4
{0x36A8, 0x7812},	// P_RD_P2Q0
{0x36AA, 0x83EF},	// P_RD_P2Q1
{0x36AC, 0xC093},	// P_RD_P2Q2
{0x36AE, 0x8754},	// P_RD_P2Q3
{0x36B0, 0x3798},	// P_RD_P2Q4
{0x36B2, 0x50B2},	// P_BL_P2Q0
{0x36B4, 0x9DF1},	// P_BL_P2Q1
{0x36B6, 0xAFF3},	// P_BL_P2Q2
{0x36B8, 0x5134},	// P_BL_P2Q3
{0x36BA, 0x2AF5},	// P_BL_P2Q4
{0x36BC, 0x0193},	// P_GB_P2Q0
{0x36BE, 0xFC70},	// P_GB_P2Q1
{0x36C0, 0xEA74},	// P_GB_P2Q2
{0x36C2, 0x82F2},	// P_GB_P2Q3
{0x36C4, 0x4B58},	// P_GB_P2Q4
{0x36C6, 0xA3D0},	// P_GR_P3Q0
{0x36C8, 0x7FD1},	// P_GR_P3Q1
{0x36CA, 0x3BB5},	// P_GR_P3Q2
{0x36CC, 0x9F96},	// P_GR_P3Q3
{0x36CE, 0x1256},	// P_GR_P3Q4
{0x36D0, 0x8171},	// P_RD_P3Q0
{0x36D2, 0xF711},	// P_RD_P3Q1
{0x36D4, 0x4F76},	// P_RD_P3Q2
{0x36D6, 0x7FB6},	// P_RD_P3Q3
{0x36D8, 0x8859},	// P_RD_P3Q4
{0x36DA, 0xC2D1},	// P_BL_P3Q0
{0x36DC, 0x93F2},	// P_BL_P3Q1
{0x36DE, 0x4E54},	// P_BL_P3Q2
{0x36E0, 0x1E17},	// P_BL_P3Q3
{0x36E2, 0x7796},	// P_BL_P3Q4
{0x36E4, 0xA510},	// P_GB_P3Q0
{0x36E6, 0x3BB1},	// P_GB_P3Q1
{0x36E8, 0x2C93},	// P_GB_P3Q2
{0x36EA, 0xC1D6},	// P_GB_P3Q3
{0x36EC, 0x7A38},	// P_GB_P3Q4
{0x36EE, 0xF794},	// P_GR_P4Q0
{0x36F0, 0x4EF5},	// P_GR_P4Q1
{0x36F2, 0x55D7},	// P_GR_P4Q2
{0x36F4, 0x9E58},	// P_GR_P4Q3
{0x36F6, 0x9939},	// P_GR_P4Q4
{0x36F8, 0xCA54},	// P_RD_P4Q0
{0x36FA, 0xAB14},	// P_RD_P4Q1
{0x36FC, 0x03D7},	// P_RD_P4Q2
{0x36FE, 0x0F1A},	// P_RD_P4Q3
{0x3700, 0x67BA},	// P_RD_P4Q4
{0x3702, 0xAF94},	// P_BL_P4Q0
{0x3704, 0x1775},	// P_BL_P4Q1
{0x3706, 0xA3F8},	// P_BL_P4Q2
{0x3708, 0xB6B8},	// P_BL_P4Q3
{0x370A, 0x009D},	// P_BL_P4Q4
{0x370C, 0x81B5},	// P_GB_P4Q0
{0x370E, 0x492F},	// P_GB_P4Q1
{0x3710, 0x39D8},	// P_GB_P4Q2
{0x3712, 0x3019},	// P_GB_P4Q3
{0x3714, 0xB939},	// P_GB_P4Q4
{0x3644, 0x0148},	// POLY_ORIGIN_C
{0x3642, 0x00F0}, 	// POLY_ORIGIN_R
{0x3210, 0x09B8},  // COLOR_PIPELINE_CONTROL
};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings10[] = {

{0x098C, 0x2306},	// MCU_ADDRESS [AWB_CCM_L_0]
{0x0990, 0x0193},  //1B3, 	// MCU_DATA_0
{0x098C, 0x231C},	// MCU_ADDRESS [AWB_CCM_RL_0]
{0x0990, 0xFFD1},	// MCU_DATA_0
{0x098C, 0x2308},	// MCU_ADDRESS [AWB_CCM_L_1]
{0x0990, 0xFF55}, //FF40, 	// MCU_DATA_0
{0x098C, 0x231E},	// MCU_ADDRESS [AWB_CCM_RL_1]
{0x0990, 0x000D},	// MCU_DATA_0
{0x098C, 0x230A},	// MCU_ADDRESS [AWB_CCM_L_2]
{0x0990, 0x001A},	// MCU_DATA_0
{0x098C, 0x2320},	// MCU_ADDRESS [AWB_CCM_RL_2]
{0x0990, 0xFFE1},	// MCU_DATA_0
{0x098C, 0x230C},	// MCU_ADDRESS [AWB_CCM_L_3]
{0x0990, 0xFF73},	// MCU_DATA_0
{0x098C, 0x2322},	// MCU_ADDRESS [AWB_CCM_RL_3]
{0x0990, 0x0040},	// MCU_DATA_0
{0x098C, 0x230E},	// MCU_ADDRESS [AWB_CCM_L_4]
{0x0990, 0x0200},	// MCU_DATA_0
{0x098C, 0x2318},	// MCU_ADDRESS [AWB_CCM_RL_4]
{0x0990, 0x001C},	// MCU_DATA_0
{0x098C, 0x231A},	// MCU_ADDRESS [AWB_CCM_L_5]
{0x0990, 0x003A},	// MCU_DATA_0
{0x098C, 0x2324},	// MCU_ADDRESS [AWB_CCM_RL_5]
{0x0990, 0xFF80},	// MCU_DATA_0
{0x098C, 0x2310},	// MCU_ADDRESS [AWB_CCM_L_6]
{0x0990, 0xFF84},	// MCU_DATA_0
{0x098C, 0x2326},	// MCU_ADDRESS [AWB_CCM_RL_6]
{0x0990, 0x001B},	// MCU_DATA_0
{0x098C, 0x2312},	// MCU_ADDRESS [AWB_CCM_L_7]
{0x0990, 0xFF66},	// MCU_DATA_0
{0x098C, 0x2328},	// MCU_ADDRESS [AWB_CCM_RL_7]
{0x0990, 0x0086},	// MCU_DATA_0
{0x098C, 0x2314},	// MCU_ADDRESS [AWB_CCM_L_8]
{0x0990, 0xFE5A},	// MCU_DATA_0
{0x098C, 0x232A},	// MCU_ADDRESS [AWB_CCM_RL_8]
{0x0990, 0x00F3},	// MCU_DATA_0
{0x098C, 0x2316},
{0x0990, 0x02A6},
{0x098C, 0x232C},
{0x0990, 0xFF0D},
{0x098C, 0x232E},
{0x0990, 0x000C},
{0x098C, 0x2330},
{0x0990, 0xFFEC},

};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings11[] = {
//[AWB MIN POS CHANGE BY DANIEL]
{0x098C, 0xA351},  // MCU_ADDRESS [AWB_CCM_POSITION_MIN]
{0x0990, 0x0020},  // MCU_DATA_0
{0x098C, 0xA352},  // MCU_ADDRESS
{0x0990, 0x007F},  // AWB_CCM_POSITION_MAX
{0x098C, 0xA354},  // MCU_ADDRESS
{0x0990, 0x0060},  // AWB_CCM_POSITION
{0x098C, 0xA354},  // MCU_ADDRESS
{0x0990, 0x0060},  // AWB_SATURATION
{0x098C, 0xA355},  // MCU_ADDRESS
{0x0990, 0x0001},  // AWB_MODE
{0x098C, 0xA35D},  // MCU_ADDRESS
{0x0990, 0x0078},  // AWB_STEADY_BGAIN_OUT_MIN
{0x098C, 0xA35E},  // MCU_ADDRESS
{0x0990, 0x0086},  // AWB_STEADY_BGAIN_OUT_MAX
{0x098C, 0xA35F},  // MCU_ADDRESS
{0x0990, 0x007E},  // AWB_STEADY_BGAIN_IN_MIN
{0x098C, 0xA360},  // MCU_ADDRESS
{0x0990, 0x0082},  // AWB_STEADY_BGAIN_IN_MAX
{0x098C, 0xA302},  // MCU_ADDRESS
{0x0990, 0x0000},  // AWB_WINDOW_POS
{0x098C, 0xA303},  // MCU_ADDRESS
{0x0990, 0x00EF},  // AWB_WINDOW_SIZE

{0x098C, 0xA365},  // MCU_ADDRESS
{0x0990, 0x0000},  // AWB_X0 <-0x0010
{0x098C, 0xA366},  // MCU_ADDRESS
{0x0990, 0x0080},  // AWB_KR_L
{0x098C, 0xA367},  // MCU_ADDRESS
{0x0990, 0x0080},  // AWB_KG_L
{0x098C, 0xA368},  // MCU_ADDRESS
{0x0990, 0x0080},  // AWB_KB_L
{0x098C, 0xA369},  // MCU_ADDRESS
{0x0990, 0x0080},   //86,  //8A,  // AWB_KR_R
{0x098C, 0xA36A},  // MCU_ADDRESS
{0x0990, 0x0082},  // AWB_KG_R
{0x098C, 0xA36B},  // MCU_ADDRESS
{0x0990, 0x0082},  // AWB_KB_R
};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings12[] = {

//[20070806 tuned]
{0x098C, 0xA11D}, // MCU_ADDRESS
{0x0990, 0x0002}, // SEQ_PREVIEW_1_AE
{0x098C, 0xA208},	//20110902 [AE_SKIP_FRAME]
{0x0990, 0x0000},
{0x098C, 0xA209},
{0x0990, 0x0002},
{0x098C, 0xA20A},
{0x0990, 0x001F},
{0x098C, 0xA216},
{0x0990, 0x003A},
{0x098C, 0xA244}, // MCU_ADDRESS
{0x0990, 0x0008}, // RESERVED_AE_44
{0x098C, 0xA24F}, // MCU_ADDRESS
{0x0990, 0x0042}, // 080723 AE target 0x0045,  // AE_BASETARGET <-0x004A
{0x098C, 0xA207}, // MCU_ADDRESS
{0x0990, 0x000A}, // AE_GATE
{0x098C, 0xA20D}, // MCU_ADDRESS
{0x0990, 0x0020}, // AE_MinVirtGain(minimum allowed virtual gain)
{0x098C, 0xA20E},
{0x0990, 0x0080}, // a0->80  AE_MaxVirtGain(maximum allowed virtual gain)
{0x098C, 0xA34E},
{0x0990, 0x00C7},
{0x098C, 0xA34F},
{0x0990, 0x0080},
{0x098C, 0xA350},
{0x0990, 0x0080},
{0x098C, 0xAB04},
{0x0990, 0x0014},
{0x098C, 0x2361}, // protect the WB hunting
{0x0990, 0x0a00},  // <-0x00X0
{0x3244, 0x0310},
};
static struct msm_camera_i2c_reg_conf mt9v113_init_settings13[] = {
//[low_light]
{0x098C, 0xA34E},	// MCU_ADDRESS
{0x0990, 0x00C7},	// MCU_DATA_0
//[LL(Low Light) setting & NR(Noise Reduction)]
{0x098C, 0xAB1F}, // MCU_ADDRESS
{0x0990, 0x00C6}, // RESERVED_HG_1F
{0x098C, 0xAB20}, // MCU_ADDRESS
{0x0990, 0x0060}, // RESERVED_HG_20(maximum saturation)(080731) 80->43
{0x098C, 0xAB21}, // MCU_ADDRESS
{0x0990, 0x001F}, // RESERVED_HG_21
{0x098C, 0xAB22}, // MCU_ADDRESS
{0x0990, 0x0005}, //6   //4   //3,  // RESERVED_HG_22
{0x098C, 0xAB23}, // MCU_ADDRESS
{0x0990, 0x0003},	//5,  // RESERVED_HG_23
{0x098C, 0xAB24},  // MCU_ADDRESS
{0x0990, 0x0048},	//38,	//30,  // RESERVED_HG_24(minimum saturation)<-0x0030 (080731) 10->00
{0x098C, 0xAB25},  // MCU_ADDRESS
{0x0990, 0x00C0},   //35,  // RESERVED_HG_25(noise filter)<-0x0014
{0x098C, 0xAB26},	// MCU_ADDRESS        [HG_LL_APCORR2]
{0x0990, 0x0003},	//0,  // RESERVED_HG_26  20110902
{0x098C, 0xAB27},  // MCU_ADDRESS
{0x0990, 0x000B},  //06,  // RESERVED_HG_27
{0x098C, 0x2B28},  // MCU_ADDRESS
{0x0990, 0x1800},  // HG_LL_BRIGHTNESSSTART <-0x1388
{0x098C, 0x2B2A},  // MCU_ADDRESS
{0x0990, 0x3000},  // HG_LL_BRIGHTNESSSTOP <-0x4E20
{0x098C, 0xAB2C},  // MCU_ADDRESS
{0x0990, 0x0006},  // RESERVED_HG_2C
{0x098C, 0xAB2D},  // MCU_ADDRESS
{0x0990, 0x000A},  // RESERVED_HG_2D
{0x098C, 0xAB2E},  // MCU_ADDRESS
{0x0990, 0x0006},  // RESERVED_HG_2E
{0x098C, 0xAB2F},  // MCU_ADDRESS
{0x0990, 0x0006},  // RESERVED_HG_2F
{0x098C, 0xAB30},  // MCU_ADDRESS
{0x0990, 0x001E},  // RESERVED_HG_30
{0x098C, 0xAB31},  // MCU_ADDRESS
{0x0990, 0x000E},  // RESERVED_HG_31
{0x098C, 0xAB32},  // MCU_ADDRESS
{0x0990, 0x001E},  // RESERVED_HG_32
{0x098C, 0xAB33},  // MCU_ADDRESS
{0x0990, 0x001E},  // RESERVED_HG_33
{0x098C, 0xAB34},  // MCU_ADDRESS
{0x0990, 0x0008},  // RESERVED_HG_34
{0x098C, 0xAB35},  // MCU_ADDRESS
{0x0990, 0x0080},  // RESERVED_HG_35

};


static struct msm_camera_i2c_reg_conf mt9v113_init_settings14[] = {
//[yellowish]
{0x098C, 0xA363}, 	// MCU_ADDRESS [AWB_TG_MIN0]
{0x0990, 0x00DF}, 	//E2,  // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings15[] = {
//[AE WINDOW SIZE POS CHANGE-CENTER]window wider 080827
{0x098C, 0xA202},    // MCU_ADDRESS [AE_WINDOW_POS]  (080731) AE window change
{0x0990, 0x0021},    //0x0043,    // MCU_DATA_0
{0x098C, 0xA203},    // MCU_ADDRESS [AE_WINDOW_SIZE]
{0x0990, 0x00dd},    // //0x00B9,    // MCU_DATA_0


{0x098C, 0x2212}, 	// MCU_ADDRESS
{0x0990, 0x0150}, 	// MCU_DATA_0

};

static struct msm_camera_i2c_reg_conf mt9v113_init_settings16[] = {
//ae_settings
{0x098C, 0xA10A}, 	// MCU_ADDRESS [SEQ_AE_FASTSTEP] //110905
{0x0990, 0x0002}, 	// MCU_DATA_0
{0x098C, 0xA11D},  // MCU_ADDRESS
{0x0990, 0x0002},  // SEQ_PREVIEW_1_AE
{0x098C, 0xA208},
{0x0990, 0x0000},
{0x098C, 0xA209},
{0x0990, 0x0002},
{0x098C, 0xA20A},
{0x0990, 0x001F},
{0x098C, 0xA216},
{0x0990, 0x003A},
{0x098C, 0xA20B}, 	// MCU_ADDRESS [AE_MIN_INDEX]
{0x0990, 0x0018}, 	// MCU_DATA_0
{0x098C, 0xA20C}, 	// MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x0018}, 	// MCU_DATA_0
//drt_off
{0x098C, 0xA244},  // MCU_ADDRESS
{0x0990, 0x0008},  // RESERVED_AE_44
//base_target
{0x098C, 0xA24F},  // MCU_ADDRESS
{0x0990, 0x0042},  // AE_BASETARGET
{0x098C, 0xA207},  // MCU_ADDRESS
{0x0990, 0x000A},  // AE_GATE
{0x098C, 0xA20D},  // MCU_ADDRESS
{0x0990, 0x0020},  // AE_MinVirtGain(minimum allowed virtual gain)
{0x098C, 0xA20E},
{0x0990, 0x0080},  // a0->80  AE_MaxVirtGain(maximum allowed virtual gain)
{0x098C, 0xAB04}, // MCU_ADDRESS
{0x0990, 0x0014},
{0x098C, 0x2361},  // protect the WB hunting
{0x0990, 0x0a00},  // <-0x00X0
{0x3244, 0x0310},

};

static struct msm_camera_i2c_reg_conf mt9v113_refresh_mode_settings[] = {
    {0x098C, 0xA103},   // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},   // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9v113_refresh_command_settings[] = {
    {0x098C, 0xA103},   // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},   // MCU_DATA_0
};


/*=============================================================
    Scene
==============================================================*/
static struct msm_camera_i2c_reg_conf mt9v113_scene_portrait_settings[] = {
/* Human */
    {0x098C, 0xAB22},   // MCU_ADDRESS
    {0x0990, 0x0003},   // RESERVED_HG_22
    {0x098C, 0xAB23},   // MCU_ADDRESS
    {0x0990, 0x0009},   // RESERVED_HG_23
};

static struct msm_camera_i2c_reg_conf mt9v113_scene_portrait_and_illumination_settings[] = {
/* Night */
    {0x098C, 0xAB22},   // MCU_ADDRESS
    {0x0990, 0x0005},   // RESERVED_HG_22
    {0x098C, 0xAB23},   // MCU_ADDRESS
    {0x0990, 0x0010},   // RESERVED_HG_23
};

static struct msm_camera_i2c_reg_conf mt9v113_scene_off_settings[] = {
/* normal */
    {0x098C, 0xAB22},   // MCU_ADDRESS
    {0x0990, 0x0005},   // RESERVED_HG_22
    {0x098C, 0xAB23},   // MCU_ADDRESS
    {0x0990, 0x0003},   // RESERVED_HG_23
};


/*=============================================================
    Size
==============================================================*/

static struct msm_camera_i2c_reg_conf mt9v113_size_vga_640x480_settings[] = {
    //[640x480 30fps~7.5fps_Veriable]
    {0x98C, 0x2703},//Output Width (A)
    {0x990, 0x0280},//      = 640
    {0x98C, 0x2705},//Output Height (A)
    {0x990, 0x01E0},//      = 480
    {0x98C, 0x2707},//Output Width (B)
    {0x990, 0x0280},//      = 640
    {0x98C, 0x2709},//Output Height (B)
    {0x990, 0x01E0},//      = 480
};


static struct msm_camera_i2c_reg_conf mt9v113_size_qvga_320x240_settings[] = {
    //[320x240 30fps~7.5fps_Veriable]
    {0x98C, 0x2703},//Output Width (A)
    {0x990, 0x0280},//      = 320
    {0x98C, 0x2705},//Output Height (A)
    {0x990, 0x01E0},//      = 240
    {0x98C, 0x2707},//Output Width (B)
    {0x990, 0x0140},//      = 320
    {0x98C, 0x2709},//Output Height (B)
    {0x990, 0x00F0},//      = 240
};

static struct msm_camera_i2c_reg_conf mt9v113_size_qcif_176x144_settings[] = {
    //[176x144 30fps~7.5fps_Veriable]
    {0x98C, 0x2703},//Output Width (A)
    {0x990, 0x0280},//      = 176
    {0x98C, 0x2705},//Output Height (A)
    {0x990, 0x01E0},//      = 144
    {0x98C, 0x2707},//Output Width (B)
    {0x990, 0x00B0},//      = 176
    {0x98C, 0x2709},//Output Height (B)
    {0x990, 0x0090},//      = 144
};

static struct msm_camera_i2c_conf_array mt9v113_pict_sizes[] = {
    {NULL},{NULL},{NULL},{NULL},{NULL},{NULL},{NULL},{NULL},
    {&mt9v113_size_vga_640x480_settings[0], ARRAY_SIZE(mt9v113_size_vga_640x480_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {NULL},
    {&mt9v113_size_qvga_320x240_settings[0], ARRAY_SIZE(mt9v113_size_qvga_320x240_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {NULL},{NULL},
    {&mt9v113_size_qcif_176x144_settings[0], ARRAY_SIZE(mt9v113_size_qcif_176x144_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

/*=============================================================
    White Balance
==============================================================*/



static struct msm_camera_i2c_reg_conf mt9v113_wb_auto_settings[] = {
    {0x098C, 0xA34A}, 	//// MCU_ADDRESS [AWB_GAIN_MIN]
    {0x0990, 0x0059}, 	//// MCU_DATA_0
    {0x098C, 0xA34B}, 	//// MCU_ADDRESS [AWB_GAIN_MAX]
    {0x0990, 0x00E6}, 	//// MCU_DATA_0
    {0x098C, 0xA34C}, 	//// MCU_ADDRESS [AWB_GAINMIN_B]
    {0x0990, 0x0059}, 	//// MCU_DATA_0
    {0x098C, 0xA34D}, 	//// MCU_ADDRESS [AWB_GAINMAX_B]
    {0x0990, 0x00E6}, 	//// MCU_DATA_0
    {0x098C, 0xA351}, 	//// MCU_ADDRESS [AWB_CCM_POSITION_MIN]
    {0x0990, 0x0000}, 	//// MCU_DATA_0
    {0x098C, 0xA352}, 	//// MCU_ADDRESS [AWB_CCM_POSITION_MAX]
    {0x0990, 0x007F}, 	//// MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf mt9v113_wb_daylight_settings[] = {
    {0x098C, 0xA34A},   /* awb_gain_min_r */
    {0x0990, 0x0098},
    {0x098C, 0xA34B},   /* awb_gain_max_r */
    {0x0990, 0x00AB},
    {0x098C, 0xA34C},   /* awb_gain_min_b */
    {0x0990, 0x0075},
    {0x098C, 0xA34D},   /* awb_gain_max_b */
    {0x0990, 0x0082},
    {0x098C, 0xA351},   /* awb_ccm_position_min */
    {0x0990, 0x0055},
    {0x098C, 0xA352},   /* awb_ccm_position_max */
    {0x0990, 0x007F},
    {0x098C, 0xA353},   /* awb_ccm_position */
    {0x0990, 0x006A},
};

static struct msm_camera_i2c_reg_conf mt9v113_wb_cloudy_settings[] = {
    {0x098C, 0xA34A},   /* awb_gain_min_r */
    {0x0990, 0x00AC},
    {0x098C, 0xA34B},   /* awb_gain_max_r */
    {0x0990, 0x00B5},
    {0x098C, 0xA34C},   /* awb_gain_min_b */
    {0x0990, 0x007A},
    {0x098C, 0xA34D},   /* awb_gain_max_b */
    {0x0990, 0x0080},
    {0x098C, 0xA351},   /* awb_ccm_position_min */
    {0x0990, 0x0060},
    {0x098C, 0xA352},   /* awb_ccm_position_max */
    {0x0990, 0x007F},
    {0x098C, 0xA353},   /* awb_ccm_position_max */
    {0x0990, 0x0070},
};

static struct msm_camera_i2c_reg_conf mt9v113_wb_incandescent_settings[] = {
    {0x098C, 0xA34A},   /* awb_gain_min_r */
    {0x0990, 0x009A},
    {0x098C, 0xA34B},   /* awb_gain_max_r */
    {0x0990, 0x00A0},
    {0x098C, 0xA34C},   /* awb_gain_min_b */
    {0x0990, 0x0080},
    {0x098C, 0xA34D},   /* awb_gain_max_b */
    {0x0990, 0x0087},
    {0x098C, 0xA351},   /* awb_ccm_position_min */
    {0x0990, 0x0000},
    {0x098C, 0xA352},   /* awb_ccm_position_max */
    {0x0990, 0x0020},
    {0x098C, 0xA353},   /* awb_ccm_position */
    {0x0990, 0x000A},
};

static struct msm_camera_i2c_reg_conf mt9v113_wb_fluorescent_low_settings[] = {
    {0x098C, 0xA34A},   /* awb_gain_min_r */
    {0x0990, 0x00A0},
    {0x098C, 0xA34B},   /* awb_gain_max_r */
    {0x0990, 0x00B0},
    {0x098C, 0xA34C},   /* awb_gain_min_b */
    {0x0990, 0x007A},
    {0x098C, 0xA34D},   /* awb_gain_max_b */
    {0x0990, 0x0085},
    {0x098C, 0xA351},   /* awb_ccm_position_min */
    {0x0990, 0x0030},
    {0x098C, 0xA352},   /* awb_ccm_position_max */
    {0x0990, 0x0055},
    {0x098C, 0xA353},   /* awb_ccm_position */
    {0x0990, 0x0049},
};

/*=============================================================
    Effect
==============================================================*/
static struct msm_camera_i2c_reg_conf mt9v113_effect_off_settings[] = {
    {0x098C, 0x2759},
    {0x0990, 0x6440},
    {0x098C, 0x275B},
    {0x0990, 0x6440},
    {0x098C, 0xA103},
    {0x0990, 0x0005},
};

static struct msm_camera_i2c_reg_conf mt9v113_effect_sepia_settings[] = {
    {0x098C, 0x2763},
    {0x0990, 0xE812},
    {0x098C, 0x2759},
    {0x0990, 0x6442},
    {0x098C, 0x275B},
    {0x0990, 0x6442},
    {0x098C, 0xA103},
    {0x0990, 0x0005},
};

static struct msm_camera_i2c_reg_conf mt9v113_effect_mono_settings[] = {
    {0x098C, 0x2759},
    {0x0990, 0x6441},
    {0x098C, 0x275B},
    {0x0990, 0x6441},
    {0x098C, 0xA103},
    {0x0990, 0x0005},
};
static struct msm_camera_i2c_reg_conf mt9v113_effect_negative_settings[] = {
    //[negative]
    {0x098C, 0x2759}, 	// MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
    {0x0990, 0x6443}, 	// MCU_DATA_0
    {0x098C, 0x275B}, 	// MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
    {0x0990, 0x6443}, 	// MCU_DATA_0
    {0x098C, 0xA103}, 	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005}, 	// MCU_DATA_0
};
/*=============================================================
    Antibanding
==============================================================*/
static struct msm_camera_i2c_reg_conf mt9v113_antibanding_auto_settings[] = {
    {0x098C, 0xA404},
    {0x0990, 0x0010},
};

static struct msm_camera_i2c_reg_conf mt9v113_antibanding_refresh_mode_settings[] = {
    {0x098C, 0xA103},
    {0x0990, 0x0006},
};

static struct msm_camera_i2c_reg_conf mt9v113_antibanding_50Hz_settings[] = {
    {0x098C, 0xA404},
    {0x0990, 0x00D0},
};

static struct msm_camera_i2c_reg_conf mt9v113_antibanding_60Hz_settings[] = {
    {0x098C, 0xA404},
    {0x0990, 0x00B0},
};

/*=============================================================
[********************************************]
	[HG_Brightness setting|(Gamma Contorl)]
[********************************************]
==============================================================*/
static struct msm_camera_i2c_reg_conf mt9v113_brightness_minus2_0_settings[] = {
    //[HG_Brightness -2.0]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x0001}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0003}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x0009}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0016}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x0024}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x0034}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x0044}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x0055}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x0068}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x007C}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x0091}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00A4}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00B6}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00C6}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00D6}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00E5}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00F2}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_minus1_7_settings[] = {
    //[HG_Brightness -1.7]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x0004}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0008}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x0010}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x001F}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x002E}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x003E}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x0050}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x0063}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x0077}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x008C}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x009F}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00B1}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00C1}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00D0}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00DD}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00E9}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00F4}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};
static struct msm_camera_i2c_reg_conf mt9v113_brightness_minus1_3_settings[] = {
    //[HG_Brightness -1.3]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x0007}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x000C}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x0015}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0026}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x0038}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x004A}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x005D}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x0070}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x0085}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x0099}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00AA}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00BA}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00C8}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00D5}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00E1}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00EC}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00F6}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_minus1_0_settings[] = {
    //[HG_Brightness -1.0]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x000A}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0011}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x001C}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0031}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x0044}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x0058}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x006C}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x0081}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x0094}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00A5}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00B5}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00C2}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00CF}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00DA}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00E4}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00EE}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00F7}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_minus0_7_settings[] = {
    //[HG_Brightness -0.7]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x0010}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0018}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x0026}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x003E}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x0054}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x0069}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x007F}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x0092}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00A3}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00B2}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00BF}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00CB}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00D6}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00DF}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00E8}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00F0}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00F8}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_minus0_3_settings[] = {
    //[HG_Brightness -0.3]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x0018}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0024}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x0035}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0050}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x0069}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x0080}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x0094}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00A4}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00B3}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00BF}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00CA}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00D4}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00DD}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00E5}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00EC}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00F3}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00F9}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100

};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_zero_settings[] = {
    //[HG_Brightness +0]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x001E}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0031}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x0048}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0068}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x0081}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x0095}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x00A6}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00B3}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00BF}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00C9}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00D2}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00DA}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00E2}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00E8}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00EF}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00F4}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00FA}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};


static struct msm_camera_i2c_reg_conf mt9v113_brightness_plus0_3_settings[] = {
    //[HG_Brightness +0.3]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x000E}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0029}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x004A}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0071}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x008E}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x00A2}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x00B2}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00BF}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00C9}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00D2}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00DA}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00E1}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00E7}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00ED}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00F2}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00F7}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00FB}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_plus0_7_settings[] = {
    //[HG_Brightness +0.7]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x0011}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0035}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x005B}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0084}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x009E}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x00B0}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x00BD}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00C8}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00D1}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00D9}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00E0}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00E5}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00EB}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00EF}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00F4}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00F8}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00FC}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_plus1_0_settings[] = {
    //[HG_Brightness +1.0]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x0016}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0044}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x0070}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x0099}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x00AF}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x00BE}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x00C9}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00D2}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00D9}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00DF}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00E5}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00EA}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00EE}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00F2}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00F6}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00F9}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00FC}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_plus1_3_settings[] = {
    //[HG_Brightness +1.3]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x001C}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0059}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x008C}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x00AE}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x00BF}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x00CB}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x00D4}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00DB}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00E1}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00E6}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00EA}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00EE}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00F1}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00F5}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00F8}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00FA}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00FD}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_plus1_7_settings[] = {
    //[HG_Brightness +1.7]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x001C}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0059}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x008C}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x00AE}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x00BF}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x00CB}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x00D4}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00DB}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00E1}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00E6}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00EA}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00EE}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00F1}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00F5}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00F8}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00FA}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00FD}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0010}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0010}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100
};

static struct msm_camera_i2c_reg_conf mt9v113_brightness_plus2_0_settings[] = {
    //[HG_Brightness +2.0]
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0000}, 	// MCU_DATA_0	// Gamma Disenable
    {0x098C, 0xAB3C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
    {0x0990, 0x0000}, 	// MCU_DATA_0
    {0x098C, 0xAB3D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
    {0x0990, 0x001C}, 	// MCU_DATA_0
    {0x098C, 0xAB3E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
    {0x0990, 0x0059}, 	// MCU_DATA_0
    {0x098C, 0xAB3F}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
    {0x0990, 0x008C}, 	// MCU_DATA_0
    {0x098C, 0xAB40}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
    {0x0990, 0x00AE}, 	// MCU_DATA_0
    {0x098C, 0xAB41}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
    {0x0990, 0x00BF}, 	// MCU_DATA_0
    {0x098C, 0xAB42}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
    {0x0990, 0x00CB}, 	// MCU_DATA_0
    {0x098C, 0xAB43}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
    {0x0990, 0x00D4}, 	// MCU_DATA_0
    {0x098C, 0xAB44}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
    {0x0990, 0x00DB}, 	// MCU_DATA_0
    {0x098C, 0xAB45}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
    {0x0990, 0x00E1}, 	// MCU_DATA_0
    {0x098C, 0xAB46}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
    {0x0990, 0x00E6}, 	// MCU_DATA_0
    {0x098C, 0xAB47}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
    {0x0990, 0x00EA}, 	// MCU_DATA_0
    {0x098C, 0xAB48}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
    {0x0990, 0x00EE}, 	// MCU_DATA_0
    {0x098C, 0xAB49}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
    {0x0990, 0x00F1}, 	// MCU_DATA_0
    {0x098C, 0xAB4A}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
    {0x0990, 0x00F5}, 	// MCU_DATA_0
    {0x098C, 0xAB4B}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
    {0x0990, 0x00F8}, 	// MCU_DATA_0
    {0x098C, 0xAB4C}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
    {0x0990, 0x00FA}, 	// MCU_DATA_0
    {0x098C, 0xAB4D}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
    {0x0990, 0x00FD}, 	// MCU_DATA_0
    {0x098C, 0xAB4E}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
    {0x0990, 0x00FF}, 	// MCU_DATA_0
    {0x098C, 0xAB37}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0001}, 	// MCU_DATA_0	// Gamma table-A
    {0x098C, 0xA75D}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_A]
    {0x0990, 0x0020}, 	// MCU_DATA_0
    {0x098C, 0xA75E}, 	// MCU_ADDRESS [MODE_Y_RGB_OFFSET_B]
    {0x0990, 0x0020}, 	// MCU_DATA_0
    {0x098C, 0xA103},	// MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0005},
    //POLL_FIELD=SEQ_CMD, !=0, DELAY=10, TIMEOUT=100

};

static struct msm_camera_i2c_reg_conf mt9v113_preview_stop[] = {
    {0x0014, 0x244B}, // PLL_CONTROL
//DELAY=40
};

/*=============================================================
    CONFIGURATION TABLE
==============================================================*/
static struct msm_camera_i2c_conf_array mt9v113_confs[] = {
    {&mt9v113_picture_prev_settings[0], ARRAY_SIZE(mt9v113_picture_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_video_prev_settings[0],   ARRAY_SIZE(mt9v113_video_prev_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_init_confs[] = {
    {&mt9v113_init_settings1[0],    ARRAY_SIZE(mt9v113_init_settings1),      10, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings1_1[0],  ARRAY_SIZE(mt9v113_init_settings1_1),    10, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings2[0],    ARRAY_SIZE(mt9v113_init_settings2),       0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_init_confs3[] = {
    {&mt9v113_init_settings4[0],    ARRAY_SIZE(mt9v113_init_settings4),     0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings4_1[0],  ARRAY_SIZE(mt9v113_init_settings4_1),  10, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_init_confs2[] = {
    {&mt9v113_init_settings3[0],  ARRAY_SIZE(mt9v113_init_settings3),   0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_init_confs4[] = {
    {&mt9v113_init_settings4_2[0],  ARRAY_SIZE(mt9v113_init_settings4_2),  10, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings6[0],    ARRAY_SIZE(mt9v113_init_settings6),    0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_init_confs5[] = {
    {&mt9v113_init_settings8[0],  ARRAY_SIZE(mt9v113_init_settings8),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings9[0],  ARRAY_SIZE(mt9v113_init_settings9),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings10[0], ARRAY_SIZE(mt9v113_init_settings10),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings11[0], ARRAY_SIZE(mt9v113_init_settings11),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings12[0], ARRAY_SIZE(mt9v113_init_settings12),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings13[0], ARRAY_SIZE(mt9v113_init_settings13),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings14[0], ARRAY_SIZE(mt9v113_init_settings14),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings15[0], ARRAY_SIZE(mt9v113_init_settings15),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_init_settings16[0], ARRAY_SIZE(mt9v113_init_settings16),  0, MSM_CAMERA_I2C_WORD_DATA},
   // {&mt9v113_init_settings17[0], ARRAY_SIZE(mt9v113_init_settings17),  0, MSM_CAMERA_I2C_WORD_DATA},
   // {&mt9v113_init_settings18[0], ARRAY_SIZE(mt9v113_init_settings18),  0, MSM_CAMERA_I2C_WORD_DATA},
   // {&mt9v113_init_settings19[0], ARRAY_SIZE(mt9v113_init_settings19),  0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_refresh_confs[] = {
    {&mt9v113_refresh_mode_settings[0],    ARRAY_SIZE(mt9v113_refresh_mode_settings),    0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_refresh_command_settings[0], ARRAY_SIZE(mt9v113_refresh_command_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_brightness_confs[] = {
    {&mt9v113_brightness_minus2_0_settings[0], ARRAY_SIZE(mt9v113_brightness_minus2_0_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_minus1_7_settings[0], ARRAY_SIZE(mt9v113_brightness_minus1_7_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_minus1_3_settings[0], ARRAY_SIZE(mt9v113_brightness_minus1_3_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_minus1_0_settings[0], ARRAY_SIZE(mt9v113_brightness_minus1_0_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_minus0_7_settings[0], ARRAY_SIZE(mt9v113_brightness_minus0_7_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_minus0_3_settings[0], ARRAY_SIZE(mt9v113_brightness_minus0_3_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_zero_settings[0],   ARRAY_SIZE(mt9v113_brightness_zero_settings),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_plus0_3_settings[0],  ARRAY_SIZE(mt9v113_brightness_plus0_3_settings),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_plus0_7_settings[0],  ARRAY_SIZE(mt9v113_brightness_plus0_7_settings),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_plus1_0_settings[0],  ARRAY_SIZE(mt9v113_brightness_plus1_0_settings),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_plus1_3_settings[0],  ARRAY_SIZE(mt9v113_brightness_plus1_3_settings),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_plus1_7_settings[0],  ARRAY_SIZE(mt9v113_brightness_plus1_7_settings),  0, MSM_CAMERA_I2C_WORD_DATA},
    {&mt9v113_brightness_plus2_0_settings[0],  ARRAY_SIZE(mt9v113_brightness_plus2_0_settings),  0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_preview_stop_confs[] = {
    {&mt9v113_preview_stop[0], ARRAY_SIZE(mt9v113_preview_stop), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9v113_preview_start_confs[] = {
    {&mt9v113_init_settings4_3[0],  ARRAY_SIZE(mt9v113_init_settings4_3),   0, MSM_CAMERA_I2C_WORD_DATA},
};

/*=============================================================
    CONFIGURATION TABLE (SEQUENCY)
==============================================================*/


#endif /* MT9V113_V4L2_H */
