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
#include <linux/leds_cmd.h>
#include <linux/leds-adp8861.h>
#include "s5k4ecgx.h"

#define SENSOR_NAME "s5k4ecgx"
#define s5k4ecgx_obj s5k4ecgx_##obj

/* LOG SETTINGS */
#define S5K4ECGX_LOGE_ON  1   /* ERROR LOG , 0:OFF, 1:ON */
#define S5K4ECGX_LOGD_ON  0   /* DEBUG LOG , 0:OFF, 1:ON */

#if S5K4ECGX_LOGE_ON
#define S5K4ECGX_LOGE(fmt, args...) printk(KERN_ERR "%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define S5K4ECGX_LOGE(fmt, args...) do{}while(0)
#endif

#if S5K4ECGX_LOGD_ON
#define S5K4ECGX_LOGD(fmt, args...) printk(KERN_INFO "%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define S5K4ECGX_LOGD(fmt, args...) do{}while(0)
#endif

#define S5K4ECGX_LOGI(fmt, args...) printk(KERN_INFO "%s(%d) " fmt, __func__, __LINE__, ##args)

/* DEBUG */
#define S5K4ECGX_DBG_REG_CHECK   S5K4ECGX_LOGD_ON   /* 0:Check OFF, 1:Check ON */

/*=============================================================
    DEFINES
==============================================================*/
/* POWER ON WAIT TIME */
/* usec */

/* msec  */

#define S5K4ECGX_WAIT_PWON_EN1           1       /*   1ms */
#define S5K4ECGX_WAIT_PWON_VREG_L12      1       /*   1ms */
#define S5K4ECGX_WAIT_PWON_EN2           1       /*   1ms */
#define S5K4ECGX_WAIT_PWON_VREG_L11      1       /*   1ms */
#define S5K4ECGX_WAIT_PWON_MCLK           1       /*   1ms */
#define S5K4ECGX_WAIT_PWON_STBYRST_H     1       /*   1ms */
#define S5K4ECGX_WAIT_PWON_RST_H         1       /*   1ms */

/* POWER OFF WAIT TIME */
/* msec  */

/* MSM GPIO */

#define S5K4ECGX_GPIO_CAM_V_EN1       3        /* GPIO[3]  */
#define S5K4ECGX_GPIO_CAM_MCLK0         5        /* GPIO[5]  */

#define S5K4ECGX_GPIO_CAM_STBYN       54
#define S5K4ECGX_GPIO_CAM_RST         107
/* PM GPIO */
#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)
#define S5K4ECGX_PMGPIO_CAM_V_EN2  PM8921_GPIO_PM_TO_SYS(25)   /* GPIO[25] */
/* VREG */
/* original */
#define CAM_VAF_MINUV                    2800000
#define CAM_VAF_MAXUV                    2800000
#define CAM_VAF_LOAD_UA                  85600


#define CAM_VANA_MINUV                    2600000
#define CAM_VANA_MAXUV                    3300000
#define CAM_VANA_LOAD_UA                  150000

static struct regulator *cam_vana = NULL;
static struct regulator *cam_vaf = NULL;
/* original */
#define CAM_VDIG_MINUV                    1200000
#define CAM_VDIG_MAXUV                    1200000
#define CAM_VDIG_LOAD_UA                  105000

#define PREVIEW_CONFIG_0  0
#define CAPTURE_CONFIG_0  0
#define ENABLED  1
#define DISABLED 0
static int32_t s5k4ecgx_auto_focus(struct msm_sensor_ctrl_t *s_ctrl, uint8_t step);
static int s5k4ecgx_scene;
static int s5k4ecgx_wb;
static int s5k4ecgx_ae_awb;  /* To save the current ae and awb status */
static int s5k4ecgx_mode;    /* To save the current mode */
static int auto_focus_lock = DISABLED;
static led_mode_t s5k4ecgx_led_state = LED_MODE_OFF;
#define S5K4ECGX_PRE_STATE_INIT    0
#define S5K4ECGX_PRE_STATE_PREVIEW 1
#define S5K4ECGX_PRE_STATE_CAPTURE 2
static int s5k4ecgx_pre_state = S5K4ECGX_PRE_STATE_INIT;

//////////////////// DEBUG SECTION //////////////////////////////
#if S5K4ECGX_DBG_REG_CHECK
static int32_t s5k4ecgx_verify_reg_conf(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_reg_conf
                                        *reg_conf_tbl, uint16_t size);
static int32_t s5k4ecgx_verify_conf_array(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_conf_array
                                          *array, uint16_t index);
static int32_t s5k4ecgx_verify_all_conf_array(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_conf_array
                                              *array, uint16_t size);
#endif

/////////////////////////////////////////////////////////////////
#define MAX_AE_WINDOW      64
#define AE_WIN_XSIZE      160
#define AE_WIN_YSIZE      120
#define AE_WIN_HD_YSIZE    90
#define AE_WIN_X_CENTER    80
#define AE_WIN_Y_CENTER    60
#define AE_WIN_HD_Y_CENTER 45
#define AE_X_MAX            8
#define AE_Y_MAX            8
#define MAX_AE_WIN_REG     32

#define IN_WIN_H           80
#define IN_WIN_V           80
#define IN_WIN_V_HD        60
#define OUT_WIN_H          160
#define OUT_WIN_V          120
#define OUT_WIN_V_HD       90
#define PREV_H             640
#define PREV_V             480
#define PREV_V_HD          360


static uint16_t list_ae [] = { 0x1492, 0x1494, 0x1496, 0x1498, 0x149A, 0x149C, 0x149E, 0x14A0,
                               0x14A2, 0x14A4, 0x14A6, 0x14A8, 0x14AA, 0x14AC, 0x14AE, 0x14B0,
                               0x14B2, 0x14B4, 0x14B6, 0x14B8, 0x14BA, 0x14BC, 0x14BE, 0x14C0,
                               0x14C2, 0x14C4, 0x14C6, 0x14C8, 0x14CA, 0x14CC, 0x14CE, 0x14D0,
};

typedef struct {
  uint16_t min;
  uint16_t max;
  uint16_t valid_ae;
} s5k4ecgx_valid_ae_t;

static s5k4ecgx_valid_ae_t list_x_valid_ae[] = {
  {  0, 159, 0},
  {160, 239, 1},
  {240, 319, 2},
  {320, 399, 3},
  {400, 479, 4},
  {480, 559, 5},
  {560, 639, 6},
  {640, 640, 6},
};

static s5k4ecgx_valid_ae_t list_y_valid_ae[] = {
  {  0, 119,  0},  /* Add 3rd element to start_valid_ae_window from list_x_valid_ae */
  {120, 179,  8},
  {180, 239, 16},
  {240, 299, 24},
  {300, 359, 32},
  {360, 419, 40},
  {420, 479, 48},
  {480, 480, 48},
};

static s5k4ecgx_valid_ae_t list_y_valid_ae_HD[] = {
  {  0,  89,  0},  /* Add 3rd element to start_valid_ae_window from list_x_valid_ae */
  { 90, 134,  8},
  {135, 179, 16},
  {180, 224, 24},
  {225, 269, 32},
  {270, 314, 40},
  {315, 359, 48},
  {360, 360, 48},
};


/////////////////////////////////////////////////////////////////
typedef enum {
  SENSOR_MODE_SNAPSHOT,
  SENSOR_MODE_RAW_SNAPSHOT,
  SENSOR_MODE_PREVIEW,
  SENSOR_MODE_PREVIEW_HD,
  SENSOR_MODE_VIDEO,
  SENSOR_MODE_VIDEO_HD,
  SENSOR_MODE_INVALID,
} s5k4ecgx_sensor_mode_t;





#define S5K4ECGX_AF_MODE_AUTOFOCUS		0
#define S5K4ECGX_AF_MODE_DISABLED		1
#define S5K4ECGX_AF_MODE_INFINITY		2
#define S5K4ECGX_AF_MODE_MACRO			3
#define S5K4ECGX_AF_MODE_CONTINUOUS		4
#define S5K4ECGX_AF_MODE_LOCK           5
#define S5K4ECGX_AF_MODE_CANCEL         6

struct af_state
{
	int af_init;
	int sensor_af_mode;
        int sensor_af_macro;
	int sensor_af_disabled;
    int af_touch;
};

struct af_state af_state;
int iso_temp = -1;

#define MAX_ROI 5
typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
} s5k4ecgx_af_roi_t;

typedef struct {
  uint32_t frm_id;
  uint8_t num_roi;
  s5k4ecgx_af_roi_t roi[MAX_ROI];
  uint8_t is_multiwindow;
} s5k4ecgx_af_roi_info_t;


static struct regulator *cam_vdig = NULL;
DEFINE_MUTEX(s5k4ecgx_mut);


struct pm_gpio s5k4ecgx_cam_v_en2_on = {
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

struct pm_gpio s5k4ecgx_cam_v_en2_off = {
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

static struct msm_cam_clk_info s5k4ecgx_cam_clk_info[] = {
    {"cam_clk", S5K4ECGX_MCLK},
};

static struct msm_camera_i2c_reg_conf s5k4ecgx_start_settings[] = {
        {0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4ecgx_stop_settings[] = {
        {0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4ecgx_groupon_settings[] = {
        {0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4ecgx_groupoff_settings[] = {
        {0x104, 0x00},
};


static struct msm_sensor_ctrl_t s5k4ecgx_s_ctrl;

static struct msm_sensor_id_info_t s5k4ecgx_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x0011,
};

static struct v4l2_subdev_info s5k4ecgx_subdev_info[] = {
        {
        .code   = V4L2_MBUS_FMT_YUYV8_2X8,
        .colorspace = V4L2_COLORSPACE_JPEG,
        .fmt    = 1,
        .order    = 0,
        },
        /* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array s5k4ecgx_init_conf[] = {
    {&s5k4ecgx_init_settings0[0], ARRAY_SIZE(s5k4ecgx_init_settings0), 10, MSM_CAMERA_I2C_WORD_DATA},
    {&s5k4ecgx_init_settings1[0], ARRAY_SIZE(s5k4ecgx_init_settings1), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array jpeg_thumbnail_conf[] = {
    {&jpeg_thumbnail_settings[0], ARRAY_SIZE(jpeg_thumbnail_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array input_size_conf[] = {
    {&input_size_settings[0], ARRAY_SIZE(input_size_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

#define PREVIEW_10_30FPS_VAR  0
#define PREVIEW_NIGHTVIEW     1
#define PREVIEW_VIDEO_30FPS   2
#define PREVIEW_30FPS_FIXED   3
static struct msm_camera_i2c_conf_array preview_config_conf[] = {
    {&preview_VGA_10_30fps_var[0], ARRAY_SIZE(preview_VGA_10_30fps_var), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&preview_VGA_NightView[0], ARRAY_SIZE(preview_VGA_NightView), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&preview_VGA_30fps_fixed[0], ARRAY_SIZE(preview_VGA_30fps_fixed), 0, MSM_CAMERA_I2C_WORD_DATA}, // Video
    {&preview_Sports_30fps_fixed[0], ARRAY_SIZE(preview_Sports_30fps_fixed), 0, MSM_CAMERA_I2C_WORD_DATA}, // 30 fps
};

#define CAPTURE_NORMAL          0
#define CAPTURE_NIGHTVIEW       1
#define CAPTURE_SPORTS          2
static struct msm_camera_i2c_conf_array capture_config_conf[] = {
    {&capture_Normal[0], ARRAY_SIZE(capture_Normal), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&capture_NightView[0], ARRAY_SIZE(capture_NightView), 0, MSM_CAMERA_I2C_WORD_DATA},
    {&capture_Sports[0], ARRAY_SIZE(capture_Sports), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array select_config_display_conf[] = {
    {&select_config_display_settings[0], ARRAY_SIZE(select_config_display_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array af_single_trigger_conf[] = {
    {&af_single_trigger_settings[0], ARRAY_SIZE(af_single_trigger_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array af_continuous_conf[] = {
    {&af_continuous[0], ARRAY_SIZE(af_continuous), 0, MSM_CAMERA_I2C_WORD_DATA},
};


static struct msm_camera_i2c_conf_array af_abort_conf[] = {
    {&af_abort[0], ARRAY_SIZE(af_abort), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array af_poweron_conf[] = {
    {&af_poweron_settings[0], ARRAY_SIZE(af_poweron_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array af_infinity_conf[] = {
    {&af_infinity1[0], ARRAY_SIZE(af_infinity1), 100, MSM_CAMERA_I2C_WORD_DATA},
    {&af_infinity2[0], ARRAY_SIZE(af_infinity2), 150, MSM_CAMERA_I2C_WORD_DATA},
    {&af_infinity3[0], ARRAY_SIZE(af_infinity3),   0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array af_macro_conf[] = {
    {&af_10cm_macro1[0], ARRAY_SIZE(af_10cm_macro1), 100, MSM_CAMERA_I2C_WORD_DATA},
    {&af_10cm_macro2[0], ARRAY_SIZE(af_10cm_macro2), 150, MSM_CAMERA_I2C_WORD_DATA},
    {&af_10cm_macro3[0], ARRAY_SIZE(af_10cm_macro3),   0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array touch_af_normal_conf[] = {
    {&ae_weight_normal[0], ARRAY_SIZE(ae_weight_normal),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&af_window_normal[0], ARRAY_SIZE(af_window_normal),   0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array touch_af_initial_conf[] = {
    {&ae_weight_touch_initial[0], ARRAY_SIZE(ae_weight_touch_initial),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&af_window_touch_initial[0], ARRAY_SIZE(af_window_touch_initial),   0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array touch_af_HD_initial_conf[] = {
    {&ae_weight_touch_initial[0], ARRAY_SIZE(ae_weight_touch_initial),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&af_window_HD_touch_initial[0], ARRAY_SIZE(af_window_HD_touch_initial),   0, MSM_CAMERA_I2C_WORD_DATA},
};


#define S5K4ECGX_WB_AUTO			0
#define S5K4ECGX_WB_DAYLIGHT		1
#define S5K4ECGX_WB_CLOUDY			2
#define S5K4ECGX_WB_FLUORESCENT		3
#define S5K4ECGX_WB_INCANDESCENT	4
static struct msm_camera_i2c_conf_array wb_conf[] = {
    {&wb_auto[0], ARRAY_SIZE(wb_auto),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&wb_daylight[0], ARRAY_SIZE(wb_daylight),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&wb_cloudy[0], ARRAY_SIZE(wb_cloudy),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&wb_fluorescent[0], ARRAY_SIZE(wb_fluorescent),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&wb_incandescent[0], ARRAY_SIZE(wb_incandescent),   0, MSM_CAMERA_I2C_WORD_DATA},
};

#define S5K4ECGX_ISO_AUTO_1			0
#define S5K4ECGX_ISO_AUTO_2			1
#define S5K4ECGX_ISO_100			2
#define S5K4ECGX_ISO_200			3
#define S5K4ECGX_ISO_400			4
//#define S5K4ECGX_ISO_800			5
static struct msm_camera_i2c_conf_array iso_conf[] = {
    {&iso_auto_1[0], ARRAY_SIZE(iso_auto_1),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_auto_2[0], ARRAY_SIZE(iso_auto_2),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_100[0], ARRAY_SIZE(iso_100),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_200[0], ARRAY_SIZE(iso_200),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_400[0], ARRAY_SIZE(iso_400),   0, MSM_CAMERA_I2C_WORD_DATA},
    //{&iso_800[0], ARRAY_SIZE(iso_800),   0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array iso_conf_sports[] = {
    {&iso_auto_s[0], ARRAY_SIZE(iso_auto_s),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_auto_s[0], ARRAY_SIZE(iso_auto_s),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_100_s[0],  ARRAY_SIZE(iso_100_s),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_200_s[0],  ARRAY_SIZE(iso_200_s),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&iso_400_s[0],  ARRAY_SIZE(iso_400_s),   0, MSM_CAMERA_I2C_WORD_DATA},
    //{&iso_800[0], ARRAY_SIZE(iso_800),   0, MSM_CAMERA_I2C_WORD_DATA},
};

#define S5K4ECGX_BRIGHTNESS_MINUS2			0
#define S5K4ECGX_BRIGHTNESS_MINUS1_7		1
#define S5K4ECGX_BRIGHTNESS_MINUS1_3		2
#define S5K4ECGX_BRIGHTNESS_MINUS1			3
#define S5K4ECGX_BRIGHTNESS_MINUS0_7		4
#define S5K4ECGX_BRIGHTNESS_MINUS0_3		5
#define S5K4ECGX_BRIGHTNESS_DEFAULT			6
#define S5K4ECGX_BRIGHTNESS_PLUS0_3			7
#define S5K4ECGX_BRIGHTNESS_PLUS0_7			8
#define S5K4ECGX_BRIGHTNESS_PLUS1			9
#define S5K4ECGX_BRIGHTNESS_PLUS1_3			10
#define S5K4ECGX_BRIGHTNESS_PLUS1_7			11
#define S5K4ECGX_BRIGHTNESS_PLUS2			12

static struct msm_camera_i2c_conf_array brightness_conf[] = {
    {&brightness_minus2[0], ARRAY_SIZE(brightness_minus2),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_minus1_7[0], ARRAY_SIZE(brightness_minus1_7),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_minus1_3[0], ARRAY_SIZE(brightness_minus1_3),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_minus1[0], ARRAY_SIZE(brightness_minus1),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_minus0_7[0], ARRAY_SIZE(brightness_minus0_7),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_minus0_3[0], ARRAY_SIZE(brightness_minus0_3),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_default[0], ARRAY_SIZE(brightness_default),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_plus0_3[0], ARRAY_SIZE(brightness_plus0_3),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_plus0_7[0], ARRAY_SIZE(brightness_plus0_7),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_plus1[0], ARRAY_SIZE(brightness_plus1),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_plus1_3[0], ARRAY_SIZE(brightness_plus1_3),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_plus1_7[0], ARRAY_SIZE(brightness_plus1_7),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&brightness_plus2[0], ARRAY_SIZE(brightness_plus2),   0, MSM_CAMERA_I2C_WORD_DATA},
};

#define S5K4ECGX_SCENE_AUTO				0
#define S5K4ECGX_SCENE_PORTRAIT			1
#define S5K4ECGX_SCENE_LANDSCAPE		2
#define S5K4ECGX_SCENE_NIGHTVIEW		3
#define S5K4ECGX_SCENE_SPORTS			4
static struct msm_camera_i2c_conf_array scene_conf[] = {
    {&scene_auto[0], ARRAY_SIZE(scene_auto),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&scene_portrait[0], ARRAY_SIZE(scene_portrait),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&scene_landscape[0], ARRAY_SIZE(scene_landscape),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&scene_nightview[0], ARRAY_SIZE(scene_nightview),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&scene_sports[0], ARRAY_SIZE(scene_sports),   0, MSM_CAMERA_I2C_WORD_DATA},
};

#define S5K4ECGX_EFFECT_NORMAL			0
#define S5K4ECGX_EFFECT_MONO			1
#define S5K4ECGX_EFFECT_SEPIA			2
static struct msm_camera_i2c_conf_array effect_conf[] = {
    {&effect_normal[0], ARRAY_SIZE(effect_normal),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&effect_monotone[0], ARRAY_SIZE(effect_monotone),   0, MSM_CAMERA_I2C_WORD_DATA},
    {&effect_sepia[0], ARRAY_SIZE(effect_sepia),   0, MSM_CAMERA_I2C_WORD_DATA},
};

#define PREVIEW_640x360	0
#define PREVIEW_VGA		1
#define PREVIEW_QVGA	2
#define PREVIEW_QCIF	3
static struct msm_camera_i2c_conf_array preview_conf[] = {
    {&preview_640x360[0], ARRAY_SIZE(preview_640x360), 100, MSM_CAMERA_I2C_WORD_DATA},
    {&preview_VGA[0], ARRAY_SIZE(preview_VGA), 100, MSM_CAMERA_I2C_WORD_DATA},
    {&preview_QVGA[0], ARRAY_SIZE(preview_QVGA), 100, MSM_CAMERA_I2C_WORD_DATA},
    {&preview_QCIF[0], ARRAY_SIZE(preview_QCIF), 100, MSM_CAMERA_I2C_WORD_DATA},
};

#define MAX_RES 5
static struct msm_camera_i2c_conf_array dimension_conf[] = {
    {&capture_5M[0], ARRAY_SIZE(capture_5M), 10, MSM_CAMERA_I2C_WORD_DATA},
    {&preview_VGA[0], ARRAY_SIZE(preview_VGA), 10, MSM_CAMERA_I2C_WORD_DATA},
    {&video_720p[0], ARRAY_SIZE(video_720p), 10, MSM_CAMERA_I2C_WORD_DATA},
    {&preview_VGA[0], ARRAY_SIZE(preview_VGA), 10, MSM_CAMERA_I2C_WORD_DATA},
    {&video_720p[0], ARRAY_SIZE(video_720p), 10, MSM_CAMERA_I2C_WORD_DATA},
};

#define CAPTURE_MODE_NORMAL	0
#define CAPTURE_MODE_NIGHT	1
#define CAPTURE_MODE_SPORT	2
static struct msm_camera_i2c_conf_array capture_mode[] = {
    {&normal_mode_capture_start[0], ARRAY_SIZE(normal_mode_capture_start), 100, MSM_CAMERA_I2C_WORD_DATA},
    {&night_mode_capture_start[0], ARRAY_SIZE(normal_mode_capture_start), 100, MSM_CAMERA_I2C_WORD_DATA},
    {&sport_mode_capture_start[0], ARRAY_SIZE(sport_mode_capture_start), 100, MSM_CAMERA_I2C_WORD_DATA},
};

#define VT_PIXEL_CLK	81000000
#define OP_PIXEL_CLK	VT_PIXEL_CLK


#define DIM_CAPTURE		0
#define DIM_PREVIEW		1
#define DIM_PREVIEW_HD	2
static struct msm_sensor_output_info_t s5k4ecgx_dimensions[] = {
		{ 	// Capture
			.x_output = 2560,
			.y_output = 1920,
			.line_length_pclk = 0x5C0,
			.frame_length_lines = 0x542,
			.vt_pixel_clk =	VT_PIXEL_CLK,
			.op_pixel_clk = OP_PIXEL_CLK,
			.binning_factor = 0, /* no binning */
		},
		{	// Preview
			.x_output = 640,
			.y_output = 480,
			.line_length_pclk = 0xA60,
			.frame_length_lines = 0x7F3,
			.vt_pixel_clk =	VT_PIXEL_CLK,
			.op_pixel_clk = OP_PIXEL_CLK,
			.binning_factor = 0, /* no binning */
		},
		{	// Preview HD
			.x_output = 1280,
			.y_output = 720,
			.line_length_pclk = 0xA60,
			.frame_length_lines = 0x65C,
			.vt_pixel_clk =	VT_PIXEL_CLK,
			.op_pixel_clk = OP_PIXEL_CLK,
			.binning_factor = 0, /* no binning */
		},
		{	// Video
			.x_output = 640,
			.y_output = 480,
			.line_length_pclk = 0xA60,
			.frame_length_lines = 0x7F3,
			.vt_pixel_clk =	VT_PIXEL_CLK,
			.op_pixel_clk = OP_PIXEL_CLK,
			.binning_factor = 0, /* no binning */
		},
		{	// Video HD
			.x_output = 1280,
			.y_output = 720,
			.line_length_pclk = 0xA60,
			.frame_length_lines = 0x65C,
			.vt_pixel_clk =	VT_PIXEL_CLK,
			.op_pixel_clk = OP_PIXEL_CLK,
			.binning_factor = 0, /* no binning */
		},

};


static struct msm_sensor_output_reg_addr_t s5k4ecgx_reg_addr = {
    .x_output = 0x02A6,
    .y_output = 0x02A8,
    .line_length_pclk = 0x2486, //FIXME: Read only register
    .frame_length_lines = 0x2484, // FIXME: Read only register
};

static struct msm_camera_csid_vc_cfg s5k4ecgx_cid_cfg[] = {
    {0, CSI_YUV422_8, CSI_DECODE_8BIT},
    {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params s5k4ecgx_csi_params = {
    .csid_params = {
        .lane_assign = 0xe4,
        .lane_cnt = 2,
        .lut_params = {
            .num_cid = ARRAY_SIZE(s5k4ecgx_cid_cfg),
            .vc_cfg = s5k4ecgx_cid_cfg,
        },
    },
    .csiphy_params = {
        .lane_cnt = 2,
        .settle_cnt = 0x13,
    },
};

static struct msm_camera_csi2_params *s5k4ecgx_csi_params_array[] = {
    &s5k4ecgx_csi_params,
    &s5k4ecgx_csi_params,
    &s5k4ecgx_csi_params,
    &s5k4ecgx_csi_params,
    &s5k4ecgx_csi_params,
    &s5k4ecgx_csi_params,
};

//FIXME: set the addresses from the datasheet
static struct msm_sensor_exp_gain_info_t s5k4ecgx_exp_gain_info = {
    .coarse_int_time_addr = 0,
    .global_gain_addr = 0x4B2,
    .vert_offset = 0,
};

int s5k4ecgx_i2c_write(struct msm_sensor_ctrl_t *s_ctrl, u16 addrl, u16 addrh, u16 val);
int s5k4ecgx_i2c_read(struct msm_sensor_ctrl_t *s_ctrl, u16 addrl, u16 addrh, u16 *val);
static void s5k4ecgx_f_delay(struct msm_sensor_ctrl_t *s_ctrl, uint32_t n_frames);

int32_t msm_sensor_s5k4ecgx_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{

	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	int rc;
	S5K4ECGX_LOGD("%s: %d\n", __func__, __LINE__);
	/* Bring the lens position to infinity in steps to avoid the lens sound during power down*/
    if(af_state.sensor_af_mode == S5K4ECGX_AF_MODE_AUTOFOCUS ||
       af_state.sensor_af_mode == S5K4ECGX_AF_MODE_CONTINUOUS ) {
         msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_abort_conf, ARRAY_SIZE(af_abort_conf));
    }
    s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x028E, 0x000A); //positive offset 0x0A
    s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x028C, 0x0004); // Manual AF
    s5k4ecgx_f_delay(s_ctrl, 2);  /* 2 frame delay */
    s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x028E, 0x0000); //positive offset 0x00
    s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x028C, 0x0004); // Manual AF
    s5k4ecgx_f_delay(s_ctrl, 2);  /* 2 frame delay */
    msleep(144);

    af_state.af_touch = 0;
    auto_focus_lock = DISABLED;
    s5k4ecgx_ae_awb = 0;
    s5k4ecgx_pre_state = S5K4ECGX_PRE_STATE_INIT;


	/* CAM_RST = 0 */
	gpio_direction_output(S5K4ECGX_GPIO_CAM_RST, 0);
	msleep(1);

	/* Disable MCLK */
    gpio_direction_output(S5K4ECGX_GPIO_CAM_MCLK0, 0);
    rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
        s5k4ecgx_cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(s5k4ecgx_cam_clk_info), 0);
    if (rc < 0) {
        pr_err("%s: clk disable failed\n", __func__);
        return -EFAULT;
    }

  	kfree(s_ctrl->reg_ptr);
	msleep(1);


	/* Turn off VREG_L11 */
    if (cam_vaf){
                regulator_set_optimum_mode(cam_vaf, 0);
                regulator_set_voltage(cam_vaf, 0, CAM_VAF_MAXUV);
                regulator_disable(cam_vaf);
                regulator_put(cam_vaf);
                cam_vaf = NULL;
    }
	msleep(1);

	/* Turn off CAM_V_EN2 */
    pm8xxx_gpio_config(S5K4ECGX_PMGPIO_CAM_V_EN2, &s5k4ecgx_cam_v_en2_off);
	msleep(1);

	/* Turn off CAM_V_EN1 */
	gpio_direction_output(S5K4ECGX_GPIO_CAM_V_EN1, 0);
	gpio_free(S5K4ECGX_GPIO_CAM_V_EN1);
	msleep(1);

	/* Turn off VREG_L12 */
	if (cam_vdig){
		regulator_set_optimum_mode(cam_vdig, 0);
		regulator_set_voltage(cam_vdig, 0, CAM_VDIG_MAXUV);
		regulator_disable(cam_vdig);
		regulator_put(cam_vdig);
		cam_vdig = NULL;
    }
	msleep(1);

	msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
            s_ctrl->sensordata->sensor_platform_info->cam_vreg,
            s_ctrl->sensordata->sensor_platform_info->num_vreg,
            s_ctrl->reg_ptr, 0);

	msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
        s_ctrl->sensordata->sensor_platform_info->cam_vreg,
        s_ctrl->sensordata->sensor_platform_info->num_vreg,
        s_ctrl->reg_ptr, 0);


    rc = msm_camera_request_gpio_table(data, 0);
    if (rc < 0) {
       pr_err("%s: request gpio failed\n", __func__);
        return -EFAULT;
    }

	S5K4ECGX_LOGD("s5k4ecgx.c %s: end\n", __func__);


#if S5K4ECGX_DBG_REG_CHECK
    s5k4ecgx_verify_all_conf_array(s_ctrl, NULL, 0);
#endif

	return 0;
}

int32_t msm_sensor_s5k4ecgx_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	S5K4ECGX_LOGD("%s: %d\n", __func__, __LINE__);

    s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
            * data->sensor_platform_info->num_vreg, GFP_KERNEL);
    if (!s_ctrl->reg_ptr) {
        pr_err("%s: could not allocate mem for regulators\n",
            __func__);
        return -ENOMEM;
    }


    rc = msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
           s_ctrl->sensordata->sensor_platform_info->cam_vreg,
            s_ctrl->sensordata->sensor_platform_info->num_vreg,
            s_ctrl->reg_ptr, 1);
    if (rc < 0) {
        pr_err("%s: regulator on failed\n", __func__);
        return -EFAULT;
    }
	rc = msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
            s_ctrl->sensordata->sensor_platform_info->cam_vreg,
            s_ctrl->sensordata->sensor_platform_info->num_vreg,
            s_ctrl->reg_ptr, 1);


    if (rc < 0) {
        pr_err("%s: enable regulator failed\n", __func__);
        return -EFAULT;
    }


    rc = gpio_request(S5K4ECGX_GPIO_CAM_V_EN1, SENSOR_NAME);
    if (rc < 0) {
        S5K4ECGX_LOGD("(S5K4ECGX_GPIO_CAM_V_EN1(%d) Error, rc = %d\n", S5K4ECGX_GPIO_CAM_V_EN1, rc);
        return -EFAULT;
    }
    gpio_direction_output(S5K4ECGX_GPIO_CAM_V_EN1, 1);
    msleep(S5K4ECGX_WAIT_PWON_EN1);

    /* VREG_L12 On */
	if (cam_vdig == NULL) {

		cam_vdig = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vdig");

		if (IS_ERR(cam_vdig)) {
			S5K4ECGX_LOGD("%s: VREG CAM VDIG get failed\n", __func__);
			cam_vdig = NULL;
			rc = -1;
		}
		if (regulator_set_voltage(cam_vdig, CAM_VDIG_MINUV,
			CAM_VDIG_MAXUV)) {
			S5K4ECGX_LOGD("%s: VREG CAM VDIG set voltage failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_set_optimum_mode(cam_vdig,
			CAM_VDIG_LOAD_UA) < 0) {
			S5K4ECGX_LOGD("%s: VREG CAM VDIG set optimum mode failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_enable(cam_vdig)) {
			S5K4ECGX_LOGD("%s: VREG CAM VDIG enable failed\n", __func__);
			rc = -1;
		}
	}

    /* wait 1msec */
    msleep(S5K4ECGX_WAIT_PWON_VREG_L12);

    rc = pm8xxx_gpio_config(S5K4ECGX_PMGPIO_CAM_V_EN2, &s5k4ecgx_cam_v_en2_on);
    if (rc) {
        S5K4ECGX_LOGD("S5K4ECGX_PMGPIO_CAM_V_EN2(%d) Error, rc = %d\n", S5K4ECGX_PMGPIO_CAM_V_EN2, rc);
        return -EFAULT;
    }

    /* wait 1msec */
    msleep(S5K4ECGX_WAIT_PWON_EN2);

    /* Enabling I2C */
    rc = msm_camera_request_gpio_table(data, 1);
    if (rc < 0) {
       pr_err("%s: request gpio failed\n", __func__);
        return -EFAULT;
    }

    /* VREG_L11 On  */
	if (cam_vaf == NULL) {
		cam_vaf = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vaf");
		if (IS_ERR(cam_vana)) {
			S5K4ECGX_LOGD("%s: VREG CAM VANA get failed\n", __func__);
			rc = -1;
		}
		if (regulator_set_voltage(cam_vaf, CAM_VAF_MINUV,
			CAM_VAF_MAXUV)) {
			S5K4ECGX_LOGD("%s: VREG CAM VANA set voltage failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_set_optimum_mode(cam_vaf,
			CAM_VAF_LOAD_UA) < 0) {
			S5K4ECGX_LOGD("%s: VREG CAM VANA set optimum mode failed\n",
				__func__);
			rc = -1;
		}
		if (regulator_enable(cam_vaf)) {
			S5K4ECGX_LOGD("%s: VREG CAM VANA enable failed\n", __func__);
			rc = -1;
		}
	}

    /* wait 1msec */
    msleep(S5K4ECGX_WAIT_PWON_VREG_L11);


    if (s_ctrl->clk_rate != 0)
        s5k4ecgx_cam_clk_info->clk_rate = s_ctrl->clk_rate;

    rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
        s5k4ecgx_cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(s5k4ecgx_cam_clk_info), 1);
    if (rc < 0) {
        pr_err("%s: clk enable failed\n", __func__);
        return -EFAULT;
    }

    /* CAM_MCLK On */
    gpio_direction_output(S5K4ECGX_GPIO_CAM_MCLK0, 1);


    msleep(S5K4ECGX_WAIT_PWON_MCLK);

	/*  STBY N*/
	//rc = gpio_request(S5K4ECGX_GPIO_CAM_STBYN, SENSOR_NAME);
	//gpio_direction_output(S5K4ECGX_GPIO_CAM_STBYN, 1);
	//msleep(S5K4ECGX_WAIT_PWON_STBYRST_H);

	/* RST */
    msleep(S5K4ECGX_WAIT_PWON_VREG_L12);
    gpio_direction_output(S5K4ECGX_GPIO_CAM_RST, 1);


    msleep(S5K4ECGX_WAIT_PWON_RST_H);
    msleep(S5K4ECGX_WAIT_PWON_RST_H);


	S5K4ECGX_LOGD("%s:  %d end\n", __func__,rc);
	return rc;
}


int s5k4ecgx_set_af_roi(struct msm_sensor_ctrl_t *s_ctrl, s5k4ecgx_af_roi_info_t *af_roi)
{
  int16_t x, y, in_x, in_y, out_x, out_y;
  int16_t in_win_h, in_win_v, prev_h, prev_v, out_win_h, out_win_v;
  uint16_t i, j, i_x, i_y;
  uint16_t start_valid_ae = 0;
  uint16_t valid_ae = 0;
  uint16_t value = 0;
  uint16_t mask = 0;
  S5K4ECGX_LOGD("%s(): called\n", __func__);
  S5K4ECGX_LOGD("%s(): 1. x = %d, y =%d\n", __func__, af_roi->roi[0].x, af_roi->roi[0].y);

  /* FIXME Hack for now, real fix is to optimize setParameters call */
  //if ((af_roi->roi[0].x == 0 && af_roi->roi[0].y == 1) || (af_roi->roi[0].x == 0 && af_roi->roi[0].y == 0))
  if (af_roi->num_roi == 0)
    return 0;

  if (!af_state.af_touch) {
    if (s5k4ecgx_mode == SENSOR_MODE_PREVIEW_HD)
      msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_HD_initial_conf, ARRAY_SIZE(touch_af_HD_initial_conf) );
    else
      msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_initial_conf, ARRAY_SIZE(touch_af_initial_conf) );

    S5K4ECGX_LOGD("First touch, Mode %d [16:9 = 3 , 4:3 = 2] \n", s5k4ecgx_mode);
    af_state.af_touch = 1;
  }

  x =  ((af_roi->roi[0].x) < 0) ? 0 : (af_roi->roi[0].x);
  y =  ((af_roi->roi[0].y) < 0) ? 0 : (af_roi->roi[0].y);

  S5K4ECGX_LOGD("%s(): 2. x = %d, y =%d\n", __func__,x, y);
  // TODO - Check Max X and Y boundary condition

  for (i=0; i < (AE_X_MAX - 1); i++)
  {
    if (list_x_valid_ae[i].min <= x && x <= list_x_valid_ae[i].max) {
      start_valid_ae = list_x_valid_ae[i].valid_ae;
      break;
    }
  }
  //S5K4ECGX_LOGD("%s(): 3. X start_valid_ae = %d",__func__, start_valid_ae);

  if (s5k4ecgx_mode == SENSOR_MODE_PREVIEW_HD)
  {
    for (i=0; i < (AE_Y_MAX - 1); i++)
    {
      if (list_y_valid_ae_HD[i].min <= y && y <= list_y_valid_ae_HD[i].max) {
        start_valid_ae = start_valid_ae + list_y_valid_ae_HD[i].valid_ae;
        break;
      }
    }
  }
  else
  {
    for (i=0; i < (AE_Y_MAX - 1); i++)
    {
      if (list_y_valid_ae[i].min <= y && y <= list_y_valid_ae[i].max) {
        start_valid_ae = start_valid_ae + list_y_valid_ae[i].valid_ae;
        break;
      }
    }
  }
  //S5K4ECGX_LOGD("%s(): 4. Y start_valid_ae = %d",__func__, start_valid_ae);

  i_x = (x<=79 || x==160 || x==240 || x==320 || x==400 || x==480 || x>=559) ? 2 : 3;
  if (s5k4ecgx_mode == SENSOR_MODE_PREVIEW_HD)
    i_y = (y<=44 || y==90  || y==135 || y==180 || y==225 || y==270 || y>=314) ? 2 : 3;
  else
    i_y = (y<=59 || y==120 || y==180 || y==240 || y==300 || y==360 || y>=419) ? 2 : 3;

  //S5K4ECGX_LOGD("%s(): 5. i_x = %d, i_y =%d\n", __func__,i_x, i_y);

  for (i = 0; i < MAX_AE_WIN_REG; i++)
    s5k4ecgx_i2c_write(s_ctrl, 0x7000, list_ae[i], 0x0); // Clear current AE Valid Window

  for ( j = 0 ; j < i_y; j++ ) {
    for ( i = 0 ; i < i_x; i++ ) {
      valid_ae = (start_valid_ae + i) + (AE_X_MAX * j);
      s5k4ecgx_i2c_read(s_ctrl, 0x7000, list_ae[valid_ae / 2], &value);
      mask = (valid_ae % 2) ? 0xFF00 : 0x00FF ;
	  s5k4ecgx_i2c_write(s_ctrl, 0x7000, list_ae[valid_ae / 2], (value & ~mask) | (mask & 0x0505));
      S5K4ECGX_LOGD("%s(): i = %d, j =%d valid_ae = %d, mask = 0x%x\n", __func__,i, j, valid_ae, mask );
    }
  }

  //s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0268, 0x01); //NCMCrev2 REG_TC_GP_PrevConfigChanged

  in_win_h = IN_WIN_H;
  out_win_h = OUT_WIN_H;
  prev_h = PREV_H;
  if (s5k4ecgx_mode == SENSOR_MODE_PREVIEW_HD) {
    in_win_v = IN_WIN_V_HD;
    out_win_v = OUT_WIN_V_HD;
    prev_v = PREV_V_HD;

  } else {
    in_win_v = IN_WIN_V;
    out_win_v = OUT_WIN_V;
    prev_v = PREV_V;
  }

  in_x =  af_roi->roi[0].x - (in_win_h / 2);
  in_x =  (in_x < 0) ? 0 : in_x ;
  in_x =  (in_x > (prev_h - in_win_h)) ? (prev_h - in_win_h) : in_x;
  in_y =  af_roi->roi[0].y - (in_win_v / 2);
  in_y =  (in_y < 0) ? 0 : in_y ;
  in_y =  (in_y > (prev_v - in_win_v)) ? (prev_v - in_win_v) : in_y;

  out_x =  af_roi->roi[0].x - (out_win_h / 2);
  out_x =  (out_x < 0) ? 0 : out_x ;
  out_x =  (out_x > (prev_h - out_win_h)) ? (prev_h - out_win_h) : out_x;
  out_y =  af_roi->roi[0].y - (out_win_v / 2);
  out_y =  (out_y < 0) ? 0 : out_y ;
  out_y =  (out_y > (prev_v - out_win_v)) ? (prev_v - out_win_v) : out_y;

  in_x = (in_x * 1024) / prev_h;  // ScndWinStartX
  in_y = (in_y * 1024) / prev_v;  // ScndWinStartY

  out_x = (out_x * 1024) / prev_h;  // FstWinStartX
  out_y = (out_y * 1024) / prev_v;  // FstWinStartY

  //AF Window Settings
  s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0294, out_x);
  s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0296, out_y);
  s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x029C, in_x);
  s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x029E, in_y);
  s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x02A4, 0x1); //REG_TC_AF_WinSizesUpdated

  S5K4ECGX_LOGD("%s(): in_x = %d, in_y =%d out_x= %d, out_y = %d\n", __func__,in_x, in_y, out_x, out_y);
  return 0;
}

int s5k4ecgx_set_af_mode(struct msm_sensor_ctrl_t *s_ctrl, int af_mode)
{
	u16 value = 0;



	if(af_state.sensor_af_mode == af_mode)
	{
	  return 0;
	}

	if(!af_state.af_init)
        {
        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_poweron_conf, ARRAY_SIZE(af_poweron_conf) );
        s5k4ecgx_f_delay(s_ctrl, 4);  /* 4 frame delay */
        af_state.af_init=1;
        }

	S5K4ECGX_LOGD("%s(): called mode %d\n", __func__, af_mode);
	switch(af_mode)
	{
		case SENSOR_AF_MODE_AUTO:
			S5K4ECGX_LOGD("%s SENSOR_AF_MODE_AUTO: \n", __func__);
            // FIXME: Hack for now to avoid restarting in CAF
            //if (af_state.sensor_af_disabled)
            //{
             // af_state.sensor_af_disabled = 0;
             // return  s5k4ecgx_set_af_mode(s_ctrl, af_state.sensor_af_mode);
            //}
			if( af_state.sensor_af_macro == SENSOR_AF_MODE_MACRO) {
			  msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_infinity_conf, ARRAY_SIZE(af_infinity_conf));
                         }
      msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
			msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_continuous_conf, ARRAY_SIZE(af_continuous_conf) );
			af_state.sensor_af_mode = S5K4ECGX_AF_MODE_AUTOFOCUS;
			break;
		case SENSOR_AF_MODE_CONTINUOUS:
			S5K4ECGX_LOGD("%s SENSOR_AF_MODE_CONTINUOUS: \n", __func__);
			af_state.sensor_af_mode = S5K4ECGX_AF_MODE_CONTINUOUS;
      msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
			msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_continuous_conf, ARRAY_SIZE(af_continuous_conf) );
			break;
		case SENSOR_AF_MODE_DISABLED:
			S5K4ECGX_LOGD("%s SENSOR_AF_MODE_DISABLED: \n", __func__);
			break;
		case SENSOR_AF_MODE_INFINITY:
			S5K4ECGX_LOGD("%s SENSOR_AF_MODE_INFINITY: \n", __func__);
			af_state.sensor_af_mode = S5K4ECGX_AF_MODE_INFINITY;
      msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
			msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_infinity_conf, ARRAY_SIZE(af_infinity_conf) );
            af_state.af_touch = 0;
			break;
		case SENSOR_AF_MODE_MACRO:
			S5K4ECGX_LOGD("%s SENSOR_AF_MODE_MACRO: \n", __func__);
			af_state.sensor_af_mode = S5K4ECGX_AF_MODE_MACRO;
      msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
			msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_macro_conf, ARRAY_SIZE(af_macro_conf) );
                        //af_state.af_init=0;
                        af_state.af_touch = 0;
			break;
		case SENSOR_AF_MODE_LOCK:
			break;
		case SENSOR_AF_MODE_CANCEL:
			S5K4ECGX_LOGD("%s SENSOR_AF_MODE_CANCEL: \n", __func__);
			af_state.sensor_af_mode = S5K4ECGX_AF_MODE_AUTOFOCUS;
			msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_continuous_conf, ARRAY_SIZE(af_continuous_conf) );

      if (s5k4ecgx_ae_awb) {
        s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, s5k4ecgx_ae_awb);
        s5k4ecgx_ae_awb = 0;
      }

                        auto_focus_lock = DISABLED;

			break;
		case SENSOR_AF_SET_WINDOW:
			//S5K4ECGX_LOGD("%s SENSOR_AF_SET_WINDOW: \n", __func__);
			break;
		case SENSOR_AF_GET_WINDOW:
			//S5K4ECGX_LOGD("%s SENSOR_AF_GET_WINDOW: \n", __func__);
			break;
		case SENSOR_AF_GET_STATUS:
			s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2EEE, &value);
			S5K4ECGX_LOGD("%s(): af status 1 %d\n", __func__, value);
			msleep(200);
			s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2207, &value);
			S5K4ECGX_LOGD("%s(): af status 2 %d\n", __func__, value);
			break;
		default:
			S5K4ECGX_LOGE("%s(): Error: invalid af_mode %d\n", __func__, af_mode);
			dump_stack();
	}
                        af_state.sensor_af_macro = af_mode;

	return 0;
}

int32_t s5k4ecgx_set_wb(struct msm_sensor_ctrl_t *s_ctrl, int wb)
{
    S5K4ECGX_LOGD("%s(): WB = %d\n", __func__, wb);
	switch(wb)
	{
		case CAMERA_WB_AUTO:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, wb_conf, S5K4ECGX_WB_AUTO);
		break;
		case CAMERA_WB_CUSTOM:
		break;
		case CAMERA_WB_INCANDESCENT:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, wb_conf, S5K4ECGX_WB_INCANDESCENT);
		break;
		case CAMERA_WB_FLUORESCENT:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, wb_conf, S5K4ECGX_WB_FLUORESCENT);
		break;
		case CAMERA_WB_DAYLIGHT:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, wb_conf, S5K4ECGX_WB_DAYLIGHT);
		break;
		case CAMERA_WB_CLOUDY_DAYLIGHT:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, wb_conf, S5K4ECGX_WB_CLOUDY);
		break;
		case CAMERA_WB_TWILIGHT:
		break;
		case CAMERA_WB_SHADE:
		break;
		case CAMERA_WB_FLUORESCENT_H:
		case CAMERA_WB_FLUORESCENT_L:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, wb_conf, S5K4ECGX_WB_FLUORESCENT);
		break;
		default:
			S5K4ECGX_LOGE("%s: Invalid wb value %d\n", __func__, wb);
	}
    s5k4ecgx_wb = wb; /* save wb*/
	return 0;
}

int32_t s5k4ecgx_set_iso(struct msm_sensor_ctrl_t *s_ctrl, int iso)
{
    u16 value = 0;
	S5K4ECGX_LOGD("%s(): setting ISO value %d\n", __func__, iso);
	switch(iso)
	{
		case MSM_V4L2_ISO_AUTO:
		msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, iso_conf, S5K4ECGX_ISO_AUTO_1);

        s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x04E6, &value);
        if (value & 0x08) {
	        s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, 0x077F); //WB = Auto   and Flicker = Auto
        } else {
	        s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, 0x0777); //WB = Manual and Flicker = Auto
        }

		msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, iso_conf, S5K4ECGX_ISO_AUTO_2);
                iso_temp = S5K4ECGX_ISO_AUTO_2;
		break;
		case MSM_V4L2_ISO_100:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, iso_conf, S5K4ECGX_ISO_100);
                iso_temp = S5K4ECGX_ISO_100;
		break;
		case MSM_V4L2_ISO_200:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, iso_conf, S5K4ECGX_ISO_200);
                iso_temp = S5K4ECGX_ISO_200;
		break;
		case MSM_V4L2_ISO_400:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, iso_conf, S5K4ECGX_ISO_400);
                iso_temp = S5K4ECGX_ISO_400;
		break;
		default: iso_temp = -1;
			S5K4ECGX_LOGE("%s(): Invalid ISO value %d\n", __func__, iso);
                return 0;
	}
        if(s5k4ecgx_scene == CAMERA_SCENE_MODE_SPORTS && iso_temp != -1 ) {
                msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, iso_conf_sports,iso_temp);
                }
	return 0;
}

int32_t s5k4ecgx_set_scene (struct msm_sensor_ctrl_t *s_ctrl, int scene)
{
   int32_t rc = 0;
   //u16 value = 0;
   S5K4ECGX_LOGD("%s(): scene %d\n", __func__, scene);
   switch(scene)
   {
       case CAMERA_SCENE_MODE_OFF:
       case CAMERA_SCENE_MODE_AUTO:
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, scene_conf, S5K4ECGX_SCENE_AUTO);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, capture_config_conf, CAPTURE_NORMAL);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_10_30FPS_VAR);
           //msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_30FPS_FIXED);
           break;
       case CAMERA_SCENE_MODE_LANDSCAPE:
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, scene_conf, S5K4ECGX_SCENE_LANDSCAPE);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, capture_config_conf, CAPTURE_NORMAL);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_10_30FPS_VAR);
           //msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_30FPS_FIXED);
           break;
       case CAMERA_SCENE_MODE_NIGHT:
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, scene_conf, S5K4ECGX_SCENE_NIGHTVIEW);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, capture_config_conf, CAPTURE_NIGHTVIEW);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_NIGHTVIEW);
           break;
       case CAMERA_SCENE_MODE_PORTRAIT:
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, scene_conf, S5K4ECGX_SCENE_PORTRAIT);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, capture_config_conf, CAPTURE_NORMAL);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_10_30FPS_VAR);
           //msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_30FPS_FIXED);
           break;
       case CAMERA_SCENE_MODE_SPORTS:
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, scene_conf, S5K4ECGX_SCENE_SPORTS);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, capture_config_conf, CAPTURE_SPORTS);
           msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_30FPS_FIXED);
           if(iso_temp != -1)
              msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, iso_conf_sports,iso_temp );
           break;
       case CAMERA_SCENE_MODE_BACKLIGHT:
       case CAMERA_SCENE_MODE_SNOW:
       case CAMERA_SCENE_MODE_BEACH:
       case CAMERA_SCENE_MODE_SUNSET:
       case CAMERA_SCENE_MODE_ANTISHAKE:
       case CAMERA_SCENE_MODE_FLOWERS:
       case CAMERA_SCENE_MODE_CANDLELIGHT:
       case CAMERA_SCENE_MODE_FIREWORKS:
       case CAMERA_SCENE_MODE_PARTY:
       case CAMERA_SCENE_MODE_NIGHT_PORTRAIT:
       case CAMERA_SCENE_MODE_THEATRE:
       default:
           S5K4ECGX_LOGE("%s(): scene %d not supported \n", __func__, scene);
           return rc;
   }
   s5k4ecgx_scene = scene; /* save scence mode */

   if (s_ctrl->curr_res < MAX_RES)
   {
      msm_sensor_write_conf_array( s5k4ecgx_s_ctrl.sensor_i2c_client, dimension_conf, s_ctrl->curr_res);
      s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0242, 0x0000); // Disable capture
      s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0244, 0x0001); // Enable Capture Changed
   }
   return rc;
}

int32_t s5k4ecgx_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int effect)
{
	S5K4ECGX_LOGD("%s(): Effect %d\n", __func__, effect);
	switch(effect)
	{
		case CAMERA_EFFECT_OFF:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, effect_conf, S5K4ECGX_EFFECT_NORMAL);
		break;
		case CAMERA_EFFECT_MONO:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, effect_conf, S5K4ECGX_EFFECT_MONO);
		break;
		case CAMERA_EFFECT_SEPIA:
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, effect_conf, S5K4ECGX_EFFECT_SEPIA);
		break;
		default:
			S5K4ECGX_LOGE("%s(): effect %d not supported\n", __func__, effect);
	}
	return 0;
}

int32_t s5k4ecgx_set_exp_compensation(struct msm_sensor_ctrl_t *s_ctrl, int exp_compensation)
{
	S5K4ECGX_LOGD("%s(): Exposure compensation = %d\n", __func__, exp_compensation);
	switch(exp_compensation)
	{
		case CAMERA_BRIGHTNESS_LV0: // -6
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_MINUS2);
		break;
		case CAMERA_BRIGHTNESS_LV1: // -5
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_MINUS1_7);
		break;
		case CAMERA_BRIGHTNESS_LV2: // -4
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_MINUS1_3);
		break;
		case CAMERA_BRIGHTNESS_LV3: // -3
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_MINUS1);
		break;
		case CAMERA_BRIGHTNESS_LV4: // -2
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_MINUS0_7);
		break;
		case CAMERA_BRIGHTNESS_LV5: // -1
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_MINUS0_3);
		break;
		case CAMERA_BRIGHTNESS_LV6: // 0
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_DEFAULT);
		break;
		case CAMERA_BRIGHTNESS_LV7: // 1
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_PLUS0_3);
		break;
		case CAMERA_BRIGHTNESS_LV8: // 2
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_PLUS0_7);
		break;
		case CAMERA_BRIGHTNESS_LV9: // 3
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_PLUS1);
		break;
		case CAMERA_BRIGHTNESS_LV10: // 4
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_PLUS1_3);
		break;
		case CAMERA_BRIGHTNESS_LV11: // 5
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_PLUS1_7);
		break;
		case CAMERA_BRIGHTNESS_LV12: // 6
			msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_PLUS2);
		break;
		default:
			S5K4ECGX_LOGE("%s(): Error, Invalid exp_compensation value %d\n", __func__, exp_compensation);
	}
	return 0;
}

static int32_t s5k4ecgx_is_af_bestPos(struct msm_sensor_ctrl_t *s_ctrl)
{
    u16 value = 0;
    s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2EEE, &value);
    S5K4ECGX_LOGD("%s(): AUTOFOCUS Read 0x7000 0x2EEE, status 0x%x\n", __func__, value);
    if (value == 0x001 || value == 0x007) {
        return AF_SEARCH_IN_PROGRESS;
    } else if(value == 0x002) {
        return AF_SEARCH_SUCCESS;
    } else {
        return AF_SEARCH_FAIL;
    }
}

static int32_t s5k4ecgx_auto_focus(struct msm_sensor_ctrl_t *s_ctrl, uint8_t step)
{
  u16         value      = 0xFFFF;
  af_status_t best_focus = AF_SEARCH_IN_PROGRESS;
	S5K4ECGX_LOGD("%s(): AUTOFOCUS step = %d E\n", __func__, step);

	switch (step) {
		case AF_LOCK_STEP1:
			best_focus = s5k4ecgx_is_af_bestPos(s_ctrl);
			break;
		case AF_LOCK_STEP2:
		    S5K4ECGX_LOGD("%s(): AUTOFOCUS, First search\n", __func__);
			s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2207, &value);
			if(!(value & 0xFF00)) {
				S5K4ECGX_LOGD("%s(): AUTOFOCUS First search success 0x%x\n", __func__, value & 0xFFFF);
				best_focus = AF_SEARCH_SUCCESS;
			}
			break;
		case AF_LOCK_STEP3:
	        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_abort_conf, ARRAY_SIZE(af_abort_conf));
			break;
		case AF_LOCK_STEP4:
	        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, af_single_trigger_conf, ARRAY_SIZE(af_single_trigger_conf));
			break;
		case AF_LOCK_STEP5:
		    best_focus = s5k4ecgx_is_af_bestPos(s_ctrl);
			break;
		case AF_LOCK_STEP6:
		    S5K4ECGX_LOGD("%s(): AUTOFOCUS, Second search AF Single\n", __func__);
			s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2207, &value);
			if(!(value & 0xFF00)) {
				S5K4ECGX_LOGD("%s(): AUTOFOCUS Second search success 0x%x\n", __func__, value & 0xFFFF);
				best_focus = AF_SEARCH_SUCCESS;
			}
			break;
		case AF_LOCK_STEP7:
			value = 0;
			s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x04E6, &value);
			s5k4ecgx_ae_awb = value; /* Save for restoring later */
			value &= ~(0x1 << 1); //bit[1] AA_AE_ACTIVE = 0
			value &= ~(0x1 << 3); //bit[3] AA_AWB_ACTIVE = 0
			s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, value);
			break;
		default:
			break;
	}

	S5K4ECGX_LOGD("%s(): AUTOFOCUS X, found best pos %d\n", __func__, best_focus);

    auto_focus_lock = ENABLED;
    return best_focus;
}

int32_t s5k4ecgx_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
    u16 value;
	S5K4ECGX_LOGI("%s(): res %d\n", __func__, res);

	v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
           NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
           PIX_0, ISPIF_OFF_IMMEDIATELY));
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);

	if (update_type == MSM_SENSOR_REG_INIT) {
		S5K4ECGX_LOGD("%s(): MSM_SENSOR_REG_INIT res %d\n", __func__, res);
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, s5k4ecgx_init_conf, ARRAY_SIZE(s5k4ecgx_init_conf) );
		msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, jpeg_thumbnail_conf, ARRAY_SIZE(jpeg_thumbnail_conf) );
		msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, input_size_conf, ARRAY_SIZE(input_size_conf) );

        msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_CONFIG_0);
        msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, capture_config_conf, CAPTURE_CONFIG_0);

		msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, select_config_display_conf, ARRAY_SIZE(select_config_display_conf) );
		// Set default brightness
		msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, brightness_conf, S5K4ECGX_BRIGHTNESS_DEFAULT);

		af_state.af_init = 0;
                iso_temp = -1;
		af_state.sensor_af_mode = -1;
                auto_focus_lock = DISABLED;

		if(0) msm_sensor_write_conf_array( s5k4ecgx_s_ctrl.sensor_i2c_client, capture_mode, CAPTURE_MODE_NORMAL);
		if(0) msm_sensor_write_conf_array( s5k4ecgx_s_ctrl.sensor_i2c_client, preview_conf, PREVIEW_VGA);
                //if(0) msm_sensor_set_sensor_mode(NULL, 0, 0);
    s5k4ecgx_pre_state = S5K4ECGX_PRE_STATE_INIT;
    } else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {

		S5K4ECGX_LOGD("%s(): MSM_SENSOR_UPDATE_PERIODIC res %d\n", __func__, res);
		/* Enable Capture */
		if(res == DIM_CAPTURE) {
			if(s5k4ecgx_led_state == LED_MODE_ON){
				S5K4ECGX_LOGD("%s(): AE Lock release \n", __func__);
				value = 0;
				s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x04E6, &value);
				if(auto_focus_lock == DISABLED) {//AF OFF or AF ON & Sports
					s5k4ecgx_ae_awb = value; /* Save for restoring later */
				}
				value |= (0x1 << 1); //bit[1] AA_AE_ACTIVE = 1
				s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, value);
		        if(!leds_cmd(LEDS_CMD_TYPE_FLASH_STILL, LED_FULL))
					S5K4ECGX_LOGE("%s():leds_cmd error\n",__func__);
				msleep(800);
				S5K4ECGX_LOGD("%s(): AE/AEW Lock\n", __func__);
				s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x04E6, &value);
				value &= ~(0x1 << 1); //bit[1] AA_AE_ACTIVE = 0
				value &= ~(0x1 << 3); //bit[3] AA_AWB_ACTIVE = 0
				s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, value);
		        if(!leds_cmd(LEDS_CMD_TYPE_FLASH_STILL, LED_OFF))
					S5K4ECGX_LOGE("%s():leds_cmd error\n",__func__);
				msleep(100);
		        if(!leds_cmd(LEDS_CMD_TYPE_FLASH_STILL, LED_FULL))
					S5K4ECGX_LOGE("%s():leds_cmd error\n",__func__);
			}else{
				if(auto_focus_lock == DISABLED) {
					S5K4ECGX_LOGD("%s(): AE/AEW Lock\n", __func__);
					value = 0;
					s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x04E6, &value);
					s5k4ecgx_ae_awb = value; /* Save for restoring later */
					value &= ~(0x1 << 1); //bit[1] AA_AE_ACTIVE = 0
					value &= ~(0x1 << 3); //bit[3] AA_AWB_ACTIVE = 0
					s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, value);
	            }
			}

			/* Writing Capture config */
			msm_sensor_write_conf_array( s5k4ecgx_s_ctrl.sensor_i2c_client, dimension_conf, res);

			/* Enable Capture */
			s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0242, 0x0001); // Enable capture
            s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x024E, 0x0001); // REG_TC_GP_NewConfigSync
			s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0244, 0x0001); // Enable Capture Changed
            s5k4ecgx_f_delay(s_ctrl, 1);  /* 1 frame delay */
            value = 0;
            do {
                  msleep(10);
              s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x0530, &value);
	S5K4ECGX_LOGD("%s(): f_delay capture waiting %d\n", __func__, value);
            } while(!value);
	S5K4ECGX_LOGD("%s(): f_delay capture interrupted %d\n", __func__, value);
      s5k4ecgx_pre_state = S5K4ECGX_PRE_STATE_CAPTURE;
		} // if(res == DIM_CAPTURE)
		else {

			msm_sensor_write_conf_array( s5k4ecgx_s_ctrl.sensor_i2c_client, dimension_conf, res);
			s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0242, 0x0000); // Disable capture
			s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x0244, 0x0001); // Enable Capture Changed

      if (s5k4ecgx_ae_awb){
        s5k4ecgx_i2c_write(s_ctrl, 0x7000, 0x04E6, s5k4ecgx_ae_awb);
        s5k4ecgx_ae_awb = 0;
      }


			if(auto_focus_lock == ENABLED) {
              auto_focus_lock = DISABLED;
            }
      if(s5k4ecgx_pre_state != S5K4ECGX_PRE_STATE_INIT) {
            s5k4ecgx_f_delay(s_ctrl, 1);  /* 1 frame delay */
            value = 1;
            do {
                  msleep(10);
              s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x0530, &value);
	S5K4ECGX_LOGD("%s(): f_delay preview waiting %d\n", __func__, value);
            } while(value);
	S5K4ECGX_LOGD("%s(): f_delay preview interrupted %d\n", __func__, value);
      }
      s5k4ecgx_pre_state = S5K4ECGX_PRE_STATE_PREVIEW;
		}

		if(s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {

			s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev, NOTIFY_CSID_CFG, &s_ctrl->curr_csi_params->csid_params);
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev, NOTIFY_CID_CHANGE, NULL);
			mb();
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev, NOTIFY_CSIPHY_CFG, &s_ctrl->curr_csi_params->csiphy_params);
			mb();
			msleep(20);
		}
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev, NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->output_settings[res].op_pixel_clk);
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev, NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(PIX_0, ISPIF_ON_FRAME_BOUNDARY));
		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
    	msleep(30);
	}
	S5K4ECGX_LOGD("%s(): Exit\n", __func__);
	return 0;
}

int32_t s5k4ecgx_sensor_config(struct msm_sensor_ctrl_t *s_ctrl, void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;
	if (copy_from_user(&cdata,
		(void *)argp,
		sizeof(struct sensor_cfg_data)))
		return -EFAULT;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	S5K4ECGX_LOGI("s5k4ecgx_sensor_config: cfgtype = %d\n",
	cdata.cfgtype);
		switch (cdata.cfgtype) {
		case CFG_SET_FPS:
		case CFG_SET_PICT_FPS:
            S5K4ECGX_LOGD("case CFG_SET_FPS:");
			if (s_ctrl->func_tbl->sensor_set_fps == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->sensor_set_fps(s_ctrl, &(cdata.cfg.fps));
			break;

		case CFG_SET_EXP_GAIN:
            S5K4ECGX_LOGD("case CFG_SET_EXP_GAIN:");
			if (s_ctrl->func_tbl->sensor_write_exp_gain == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->sensor_write_exp_gain(s_ctrl, cdata.cfg.exp_gain.gain, cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_PICT_EXP_GAIN:
            S5K4ECGX_LOGD("case CFG_SET_PICT_EXP_GAIN:");
			if (s_ctrl->func_tbl->sensor_write_snapshot_exp_gain == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->sensor_write_snapshot_exp_gain( s_ctrl, cdata.cfg.exp_gain.gain, cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_MODE:
                           af_state.sensor_af_mode = -1 ;
            S5K4ECGX_LOGD("case CFG_SET_MODE: and now af_mode = -1 \n");
			if (s_ctrl->func_tbl->sensor_set_sensor_mode == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->sensor_set_sensor_mode(s_ctrl, cdata.mode, cdata.rs);
			break;

		case CFG_SET_EFFECT:
            S5K4ECGX_LOGD("case CFG_SET_EFFECT:");
            if (s_ctrl->func_tbl->sensor_set_effect == NULL) {
                rc = -EFAULT;
                break;
            }
            rc = s_ctrl->func_tbl->sensor_set_effect(s_ctrl, cdata.cfg.effect);
			break;

        case CFG_SET_SCENE_SELECT:
            S5K4ECGX_LOGD("case CFG_SET_SCENE_SELECT");
            if (s_ctrl->func_tbl->sensor_set_scene == NULL) {
                rc = -EFAULT;
                break;
            }
            rc = s_ctrl->func_tbl->sensor_set_scene(s_ctrl,cdata.cfg.scene);
            break;

        case CFG_SET_PICT_SIZE:
            S5K4ECGX_LOGD("case CFG_SET_PICT_SIZE");
            if (s_ctrl->func_tbl->sensor_set_pict_size == NULL) {
                rc = -EFAULT;
                break;
            }
            rc = s_ctrl->func_tbl->sensor_set_pict_size(s_ctrl, cdata.cfg.pict_size);
            break;

        case CFG_SET_WB:
            S5K4ECGX_LOGD("case CFG_SET_WB");
            if (s_ctrl->func_tbl->sensor_set_wb == NULL) {
                rc = -EFAULT;
                break;
            }
            rc = s_ctrl->func_tbl->sensor_set_wb(s_ctrl, cdata.cfg.wb_val);
            break;

		case CFG_SET_ISO:
			S5K4ECGX_LOGD("case CFG_SET_ISO");
			rc = s5k4ecgx_set_iso(s_ctrl, cdata.cfg.iso_val);
			break;

        case CFG_SET_ANTIBANDING:
            S5K4ECGX_LOGD("case CFG_SET_ANTIBANDING");
            if (s_ctrl->func_tbl->
            sensor_set_antibanding == NULL) {
                rc = -EFAULT;
                break;
            }
            rc = s_ctrl->func_tbl->sensor_set_antibanding(s_ctrl, cdata.cfg.antibanding);
            break;

        case CFG_SET_EXP_COMPENSATION:
            //S5K4ECGX_LOGD("case CFG_SET_EXP_COMPENSATION");
            if (s_ctrl->func_tbl->
            sensor_set_exp_compensation == NULL) {
                rc = -EFAULT;
                break;
            }
			S5K4ECGX_LOGD("%s(): setting exp compensation %d\n", __func__, cdata.cfg.exp_compensation);
            rc = s_ctrl->func_tbl->sensor_set_exp_compensation(s_ctrl, cdata.cfg.exp_compensation);
            break;

		case CFG_SENSOR_INIT:
			if (s_ctrl->func_tbl->sensor_mode_init == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->sensor_mode_init(s_ctrl, cdata.mode, &(cdata.cfg.init_info));
			break;

		case CFG_GET_OUTPUT_INFO:
			if (s_ctrl->func_tbl->sensor_get_output_info == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->sensor_get_output_info(s_ctrl, &cdata.cfg.output_info);
			if (copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_EEPROM_DATA:
			if (s_ctrl->sensor_eeprom_client == NULL ||
				s_ctrl->sensor_eeprom_client->func_tbl.eeprom_get_data == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->sensor_eeprom_client->func_tbl.eeprom_get_data(s_ctrl->sensor_eeprom_client, &cdata.cfg.eeprom_data);

			if (copy_to_user((void *)argp, &cdata, sizeof(struct sensor_eeprom_data_t)))
				rc = -EFAULT;
			break;

        /* PM_OBS */
        case CFG_SET_PM_OBS:
            if (s_ctrl->func_tbl->sensor_set_parm_pm_obs == NULL) {
                rc = -EFAULT;
                break;
            }
            rc = s_ctrl->func_tbl->sensor_set_parm_pm_obs(cdata.cfg.pm_obs);
            break;
        /* PM_OBS */

        case CFG_GET_MAKER_NOTE:
            S5K4ECGX_LOGD("case CFG_GET_MAKER_NOTE");
            if (s_ctrl->func_tbl->sensor_get_maker_note == NULL) {
                rc = -EFAULT;
                break;
            }

            rc = s_ctrl->func_tbl->sensor_get_maker_note(s_ctrl, &(cdata.cfg.get_exif_maker_note) );

            if (copy_to_user((void *)argp, &cdata,
                     sizeof(struct sensor_cfg_data))) {
                rc = -EFAULT;
            }
        break;

        case CFG_GET_PARAM_EXIF:
            S5K4ECGX_LOGD("case CFG_GET_PARAM_EXIF");
            if (s_ctrl->func_tbl->sensor_get_exif_param == NULL) {
                rc = -EFAULT;
                break;
            }

            rc = s_ctrl->func_tbl->sensor_get_exif_param(s_ctrl, &(cdata.cfg.get_exif_param) );

            if (rc >= 0) {
                if (copy_to_user((void *)argp, &cdata,
                         sizeof(struct sensor_cfg_data))) {
                    rc = -EFAULT;
                }
            }
        break;

        case CFG_GET_EEPROM_READ:
            S5K4ECGX_LOGD("case CFG_GET_EEPROM_READ");
            if (s_ctrl->func_tbl->sensor_get_eeprom_otp_info == NULL) {
                rc = -EFAULT;
                break;
            }

            rc = s_ctrl->func_tbl->sensor_get_eeprom_otp_info(s_ctrl, &(cdata.cfg.eeprom_otp_info) );

            if (copy_to_user((void *)argp, &cdata, sizeof(struct sensor_cfg_data))) {
                rc = -EFAULT;
            }
        break;
		case CFG_SET_AF_MODE:
			S5K4ECGX_LOGD("%s(): CFG_SET_AF_MODE called \n", __func__);
			if(s5k4ecgx_set_af_mode(s_ctrl, cdata.cfg.af_mode.af_mode)) {
				rc = -EFAULT;
			}
			break;
		case CFG_SET_AF_ROI:
			S5K4ECGX_LOGD("%s(): CFG_SET_AF_ROI\n", __func__);
			if(s5k4ecgx_set_af_roi(s_ctrl,(s5k4ecgx_af_roi_info_t*)&(cdata.cfg.af_roi))){
				rc = -EFAULT;
			}
			break;
		case CFG_AUTO_FOCUS:
			S5K4ECGX_LOGD("%s(): CFG_SET_AUTO_FOCUS called \n", __func__);
			rc = s5k4ecgx_auto_focus(s_ctrl, cdata.cfg.af_mode.af_data_num);
			break;
		case CFG_AUTO_FOCUS_CANCEL:
			S5K4ECGX_LOGD("%s(): CFG_AUTO_FOCUS_CANCEL called \n", __func__);
			break;
		case CFG_MOVE_FOCUS:
			S5K4ECGX_LOGD("%s(): CFG_MOVE_FOCUS called\n", __func__);
			break;

		case CFG_SET_BRIGHTNESS:
			S5K4ECGX_LOGD("%s(): CFG_SET_BRIGHTNESS called\n", __func__);
			break;
		case CFG_SET_LED_MODE:
			S5K4ECGX_LOGD("%s(): CFG_SET_LED_MODE called %d\n", __func__, cdata.cfg.led_mode);
            s5k4ecgx_led_state = cdata.cfg.led_mode;
			break;
		default:
			rc = -EFAULT;
			break;
		}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

int32_t s5k4ecgx_set_sensor_mode(struct msm_sensor_ctrl_t *s_ctrl,
    int mode, int res)
{
    int32_t rc = 0;
    S5K4ECGX_LOGD("Mode = %d, Res = %d\n", mode, res);
    switch (mode)
    {
      case SENSOR_MODE_SNAPSHOT:
      case SENSOR_MODE_RAW_SNAPSHOT:
        // Nothing to do
        break;
      case SENSOR_MODE_PREVIEW:
        msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_10_30FPS_VAR);
        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
        af_state.af_touch = 0;
        //msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_30FPS_FIXED);
        break;
      case SENSOR_MODE_VIDEO:
        msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_VIDEO_30FPS);
        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
        break;
      case SENSOR_MODE_VIDEO_HD:
        msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_VIDEO_30FPS);
        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
        break;
      case SENSOR_MODE_PREVIEW_HD:
        msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_10_30FPS_VAR);
        msm_sensor_write_all_conf_array(s_ctrl->sensor_i2c_client, touch_af_normal_conf, ARRAY_SIZE(touch_af_normal_conf) );
        af_state.af_touch = 0;
        //msm_sensor_write_conf_array(s_ctrl->sensor_i2c_client, preview_config_conf, PREVIEW_30FPS_FIXED);
        break;
      case SENSOR_MODE_INVALID:
      default:
        S5K4ECGX_LOGE("Mode = %d, Res = %d Unsupported\n", mode, res);
        break;
    }

    if (s_ctrl->curr_res != res) {
        s_ctrl->curr_frame_length_lines =
            s_ctrl->msm_sensor_reg->
            output_settings[res].frame_length_lines;

        s_ctrl->curr_line_length_pclk =
            s_ctrl->msm_sensor_reg->
            output_settings[res].line_length_pclk;

        if (s_ctrl->sensordata->pdata->is_csic)
            rc = s_ctrl->func_tbl->sensor_csi_setting(s_ctrl,
                MSM_SENSOR_UPDATE_PERIODIC, res);
        else
            rc = s_ctrl->func_tbl->sensor_setting(s_ctrl,
                MSM_SENSOR_UPDATE_PERIODIC, res);
        if (rc < 0)
            return rc;
        s_ctrl->curr_res = res;
   }
   s5k4ecgx_mode = mode; /* save mode */
   if(mode == SENSOR_MODE_PREVIEW || mode == SENSOR_MODE_PREVIEW_HD) {
     s5k4ecgx_set_scene (s_ctrl, s5k4ecgx_scene);
   }

   return rc;
}



int32_t s5k4ecgx_set_fps(struct msm_sensor_ctrl_t *s_ctrl, struct fps_cfg *fps)
{
	uint16_t total_lines_per_frame = 0;
	int32_t rc = 0;
	s_ctrl->fps_divider = fps->fps_div;

	if (s_ctrl->curr_res != MSM_SENSOR_INVALID_RES) {
		total_lines_per_frame = (uint16_t)((s_ctrl->curr_frame_length_lines) * s_ctrl->fps_divider/Q10);
		S5K4ECGX_LOGD("%s(): total_lines_per_frame: %d, s_ctrl->fps_divider %d\n", __func__, total_lines_per_frame, s_ctrl->fps_divider);

		rc = s5k4ecgx_i2c_write(s_ctrl, 0x7000, s_ctrl->sensor_output_reg_addr->frame_length_lines, total_lines_per_frame);
	}
	return rc;
}

int32_t s5k4ecgx_sensor_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
        uint16_t gain, uint32_t line) {
	//S5K4ECGX_LOGD("###### %s called #####\n", __func__);
	//dump_stack();
	return 0;
}
static int32_t s5k4ecgx_get_maker_note(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_maker_note_cfg *get_exif_maker_note)
{
	//S5K4ECGX_LOGD("###### %s called #####\n", __func__);
	//dump_stack();
	return 0;
}
static int32_t s5k4ecgx_get_exif_param(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_param_inf *get_exif_param)
{
	//S5K4ECGX_LOGD("###### %s called #####\n", __func__);
	//dump_stack();
        u16 val=0;
        s5k4ecgx_i2c_read(s_ctrl,0x7000, 0x2BC4, &val);
        get_exif_param->analog_gain_code_global = val;
        S5K4ECGX_LOGD("Camera analog gain=%d\n",val);

        s5k4ecgx_i2c_read(s_ctrl,0x7000, 0x2BC6, &val);
        get_exif_param->digital_gain_greenr = val;
        S5K4ECGX_LOGD("Camera digital gain=%d\n",val);

        s5k4ecgx_i2c_read(s_ctrl,0x7000, 0x2BC0, &val);
        get_exif_param->fine_integration_time = val;
        S5K4ECGX_LOGD("Camera Exposure time LSB=%d\n",val);

        s5k4ecgx_i2c_read(s_ctrl,0x7000, 0x2BC2, &val);
        get_exif_param->coarse_integration_time = val;
        S5K4ECGX_LOGD("Camera Exposure time MSB=%d\n",val);
	return 0;
}


int s5k4ecgx_i2c_read(struct msm_sensor_ctrl_t *s_ctrl, u16 addrh, u16 addrl, u16 *val)
{
	int rc = 0;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x002C, addrh, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) return rc;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x002E, addrl, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) return rc;
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x0F12, val, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) return rc;

	return rc;
}

static void s5k4ecgx_f_delay(struct msm_sensor_ctrl_t *s_ctrl, uint32_t n_frames)
{
	int rc = 0;
    u16 frame_length_lines = 0xFFFF;
    u16 line_length_pclk= 0xFFFF;
    u16 tg_clk = 0xFFFF;
    uint32_t frame_delay = 0;

    rc = s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2484, &frame_length_lines);
	if(rc < 0) return;
    rc = s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2486, &line_length_pclk);
	if(rc < 0) return;
    rc = s5k4ecgx_i2c_read(s_ctrl, 0x7000, 0x2488, &tg_clk);
	if(rc < 0) return;

    if(line_length_pclk > 0)
      frame_delay =  (tg_clk * 10000) / line_length_pclk;
    if(frame_length_lines > 0)
      frame_delay =  (frame_delay * 10000) / frame_length_lines;
    if(frame_delay > 0)
      frame_delay =  100000000 / frame_delay;

	msleep(frame_delay * n_frames); // Frame delay
}

int s5k4ecgx_i2c_write(struct msm_sensor_ctrl_t *s_ctrl, u16 addrh, u16 addrl, u16 val)
{
	int rc = 0;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0028, addrh, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) return rc;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x002A, addrl, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) return rc;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0F12, val, MSM_CAMERA_I2C_WORD_DATA);
	if(rc < 0) return rc;

	return rc;
}

#if S5K4ECGX_DBG_REG_CHECK
static int32_t s5k4ecgx_verify_reg_conf(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_reg_conf
                                                                           *reg_conf_tbl, uint16_t size)
{
    int i = 0;
    u16 newladr = 0;
    u16 aVal = 0xFFFF;  /* Actual value  */
    u16 hadr = 0x7000;  /* High Address  */
    u16 ladr = 0x0000;  /* Low Address   */
    u16 eVal = 0x0000;  /* Current value */
    u16 disp = 0x0000;  /* Display       */

    S5K4ECGX_LOGD("START\n");

    for (i = 0; i < size; i++) {
        disp = 0;
        switch(reg_conf_tbl[i].reg_addr) {
            case 0xFCFC:
                hadr = reg_conf_tbl[i].reg_data;
                break;
            case 0x0028:
                hadr = reg_conf_tbl[i].reg_data;
                break;
            case 0x002A:
                ladr = reg_conf_tbl[i].reg_data;
                newladr = 1;
                break;
            case 0x0F12:
                if (newladr)
                    newladr = 0;
                else
                    ladr += 2;

                eVal = reg_conf_tbl[i].reg_data;
                disp = 1;
                break;
            default:
                break;
        }
        if (disp)
        {
          s5k4ecgx_i2c_read(s_ctrl, hadr, ladr, &aVal);
          S5K4ECGX_LOGD("addr(0x%04X) : Expected(0x%04X) : Actual(0x%04X)", (hadr << 16) | ladr, eVal, aVal);
          msleep(1);
        }
    } //for

    S5K4ECGX_LOGD("END\n");
    return 0;
}

static int32_t s5k4ecgx_verify_conf_array(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_conf_array
                                          *array, uint16_t index)
{
    return s5k4ecgx_verify_reg_conf( s_ctrl, (struct msm_camera_i2c_reg_conf *)
                                     array[index].conf, array[index].size);
}

static int32_t s5k4ecgx_verify_all_conf_array(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_conf_array
                                              *array, uint16_t size)
{
    int32_t rc = 0, i;
    if (array) {
        for (i = 0; i < size; i++) {
            rc = s5k4ecgx_verify_conf_array(s_ctrl, array, i);
            if (rc < 0)
                break;
        }
    }
    return rc;
}


#endif /* S5K4ECGX_DBG_REG_CHECK */


static const struct i2c_device_id s5k4ecgx_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&s5k4ecgx_s_ctrl},
	{ }
};

static struct i2c_driver s5k4ecgx_i2c_driver = {
	.id_table = s5k4ecgx_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k4ecgx_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};



static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&s5k4ecgx_i2c_driver);
}

static struct v4l2_subdev_core_ops s5k4ecgx_subdev_core_ops = {
        .ioctl = msm_sensor_subdev_ioctl,
        .s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops s5k4ecgx_subdev_video_ops = {
        .enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops s5k4ecgx_subdev_ops = {
	.core = &s5k4ecgx_subdev_core_ops,
	.video  = &s5k4ecgx_subdev_video_ops,
};

static struct msm_sensor_fn_t s5k4ecgx_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = s5k4ecgx_set_fps,
	.sensor_write_exp_gain = s5k4ecgx_sensor_write_exp_gain,
	.sensor_write_snapshot_exp_gain = s5k4ecgx_sensor_write_exp_gain,
	.sensor_setting = s5k4ecgx_sensor_setting,
	.sensor_set_sensor_mode = s5k4ecgx_set_sensor_mode,
	//.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = s5k4ecgx_sensor_config,
	.sensor_power_up = msm_sensor_s5k4ecgx_power_up,
	.sensor_power_down = msm_sensor_s5k4ecgx_power_down,
	.sensor_set_parm_pm_obs = msm_sensor_set_parm_pm_obs,
	.sensor_get_maker_note = s5k4ecgx_get_maker_note,
	.sensor_get_exif_param = s5k4ecgx_get_exif_param,
    .sensor_set_wb = s5k4ecgx_set_wb,
	.sensor_set_scene = s5k4ecgx_set_scene,
	.sensor_set_exp_compensation = s5k4ecgx_set_exp_compensation,
	.sensor_set_effect = s5k4ecgx_set_effect,
 //   .sensor_get_eeprom_otp_info = s5k4ecgx_get_eeprom_otp_info,
};

 static struct msm_sensor_reg_t s5k4ecgx_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = s5k4ecgx_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(s5k4ecgx_start_settings),
	.stop_stream_conf = s5k4ecgx_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(s5k4ecgx_stop_settings),
	.group_hold_on_conf = s5k4ecgx_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(s5k4ecgx_groupon_settings),
	.group_hold_off_conf = s5k4ecgx_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(s5k4ecgx_groupoff_settings),
	.init_settings = &s5k4ecgx_init_conf[0],
	.init_size = ARRAY_SIZE(s5k4ecgx_init_conf),
	.mode_settings = &dimension_conf[0],
	.output_settings = &s5k4ecgx_dimensions[0],
	.num_conf = ARRAY_SIZE(dimension_conf),
};


static struct msm_sensor_ctrl_t s5k4ecgx_s_ctrl = {
	.msm_sensor_reg = &s5k4ecgx_regs,
	.sensor_i2c_client = &s5k4ecgx_sensor_i2c_client,
	.sensor_i2c_addr = 0x5a,
//	.sensor_eeprom_client = &s5k4ecgx_eeprom_client,
	.sensor_output_reg_addr = &s5k4ecgx_reg_addr,
	.sensor_id_info = &s5k4ecgx_id_info,
	.sensor_exp_gain_info = &s5k4ecgx_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &s5k4ecgx_csi_params_array[0],
	.msm_sensor_mutex = &s5k4ecgx_mut,
	.sensor_i2c_driver = &s5k4ecgx_i2c_driver,
	.sensor_v4l2_subdev_info = s5k4ecgx_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k4ecgx_subdev_info),
	.sensor_v4l2_subdev_ops = &s5k4ecgx_subdev_ops,
	.func_tbl = &s5k4ecgx_func_tbl,
	.clk_rate = S5K4ECGX_MCLK,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Ruby device Rear sensor driver");
MODULE_LICENSE("GPL v2");
