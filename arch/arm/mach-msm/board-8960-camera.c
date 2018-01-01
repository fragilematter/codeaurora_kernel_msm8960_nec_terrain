/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

/*
 * Copyright (C) 2011 NEC CASIO Mobile Communications, Ltd.
 *
 *  No permission to use, copy, modify and distribute this software
 *  and its documentation for any purpose is granted.
 *  This software is provided under applicable license agreement only.
 */

#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/board.h>
#include <mach/msm_bus_board.h>
#include <mach/gpiomux.h>
#include "devices.h"
#include "board-8960.h"

#if (defined(CONFIG_GPIO_SX150X) || defined(CONFIG_GPIO_SX150X_MODULE)) && \
	defined(CONFIG_I2C)

#ifdef CONFIG_IMX074
static struct i2c_board_info cam_expander_i2c_info[] = {
	{
		I2C_BOARD_INFO("sx1508q", 0x22),
		.platform_data = &msm8960_sx150x_data[SX150X_CAM]
	},
};

static struct msm_cam_expander_info cam_expander_info[] = {
	{
		cam_expander_i2c_info,
		MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	},
};
#endif
#endif

static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend 1*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 1*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 2*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_HIGH,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 3*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 4*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_IN
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 5*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_HIGH,
	},
	{
		.func = GPIOMUX_FUNC_2, /*active 6*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},

    {
        .func = GPIOMUX_FUNC_1, /*active 7*/
        .drv = GPIOMUX_DRV_4MA,
        .pull = GPIOMUX_PULL_NONE,
        .dir = GPIOMUX_OUT_LOW,
    },

    {
        .func = GPIOMUX_FUNC_2, /*active 8*/
        .drv = GPIOMUX_DRV_4MA,
        .pull = GPIOMUX_PULL_NONE,
        .dir = GPIOMUX_OUT_LOW,
    },
};


static struct msm_gpiomux_config msm8960_cam_common_configs[] = {
	{
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0], 
		},
	},
	{
		.gpio = 3,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 4,
		.settings = {
#ifdef CONFIG_FEATURE_NCMC_D121M
            [GPIOMUX_ACTIVE]    = &cam_settings[8],
            [GPIOMUX_SUSPENDED] = &cam_settings[8],
#else /* CONFIG_FEATURE_NCMC_D121M */
			[GPIOMUX_ACTIVE]    = &cam_settings[6],
			[GPIOMUX_SUSPENDED] = &cam_settings[6],
#endif /* CONFIG_FEATURE_NCMC_D121M */
		},
	},
	{
		.gpio = 5,
		.settings = {
#ifdef CONFIG_FEATURE_NCMC_D121M
            [GPIOMUX_ACTIVE]    = &cam_settings[7],
            [GPIOMUX_SUSPENDED] = &cam_settings[7],
#else /* CONFIG_FEATURE_NCMC_D121M */
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
#endif /* CONFIG_FEATURE_NCMC_D121M */
		},
	},
	{
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 43,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 107,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
};

static struct msm_gpiomux_config msm8960_cam_2d_configs[] = {
#ifdef CONFIG_FEATURE_NCMC_D121M
	{
		.gpio = 20,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[4],
		},
	},
	{
		.gpio = 21,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[4],
		},
	},
#else
	{
		.gpio = 20,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 21,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#endif /* CONFIG_FEATURE_NCMC_D121M */
};

#ifdef CONFIG_MT9V113
static struct msm_gpiomux_config msm8960_cam_2d_configs2[] = {
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
};
#endif /* CONFIG_MT9V113 */

#ifdef CONFIG_MSM_CAMERA

#define VFE_CAMIF_TIMER1_GPIO 2
#define VFE_CAMIF_TIMER2_GPIO 3
#define VFE_CAMIF_TIMER3_GPIO_INT 4

#if defined(CONFIG_IMX074) || defined(CONFIG_CE150X)
static struct msm_camera_sensor_strobe_flash_data strobe_flash_xenon = {
	.flash_trigger = VFE_CAMIF_TIMER2_GPIO,
	.flash_charge = VFE_CAMIF_TIMER1_GPIO,
	.flash_charge_done = VFE_CAMIF_TIMER3_GPIO_INT,
	.flash_recharge_duration = 50000,
	.irq = MSM_GPIO_TO_INT(VFE_CAMIF_TIMER3_GPIO_INT),
};
#endif

#ifdef CONFIG_MSM_CAMERA_FLASH
static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_EXT,
	._fsrc.ext_driver_src.led_en = VFE_CAMIF_TIMER1_GPIO,
	._fsrc.ext_driver_src.led_flash_en = VFE_CAMIF_TIMER2_GPIO,
};
#endif

static struct msm_bus_vectors cam_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_preview_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 27648000,
		.ib  = 110592000,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 154275840,
		.ib  = 617103360,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 274423680,
		.ib  = 1097694720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
};

static struct msm_bus_vectors cam_zsl_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 302071680,
		.ib  = 1208286720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
};

static struct msm_bus_paths cam_bus_client_config[] = {
	{
		ARRAY_SIZE(cam_init_vectors),
		cam_init_vectors,
	},
	{
		ARRAY_SIZE(cam_preview_vectors),
		cam_preview_vectors,
	},
	{
		ARRAY_SIZE(cam_video_vectors),
		cam_video_vectors,
	},
	{
		ARRAY_SIZE(cam_snapshot_vectors),
		cam_snapshot_vectors,
	},
	{
		ARRAY_SIZE(cam_zsl_vectors),
		cam_zsl_vectors,
	},
};

static struct msm_bus_scale_pdata cam_bus_client_pdata = {
		cam_bus_client_config,
		ARRAY_SIZE(cam_bus_client_config),
		.name = "msm_camera",
};

static struct msm_camera_device_platform_data msm_camera_csi_device_data[] = {
	{
		.csid_core = 0,
		.is_csiphy = 1,
		.is_csid   = 1,
		.is_ispif  = 1,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.csid_core = 1,
		.is_csiphy = 1,
		.is_csid   = 1,
		.is_ispif  = 1,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.ioclk.mclk_clk_rate = 24000000,
		.ioclk.vfe_clk_rate  = 228570000,
		.csid_core = 2,
		.is_csiphy = 1,
		.is_csid   = 1,
		.is_ispif  = 1,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
};

#ifdef CONFIG_IMX111
static struct camera_vreg_t msm_8960_back_cam_vreg[] = {
};
#else /* CONFIG_IMX111 */
#if defined(CONFIG_IMX074) || defined(CONFIG_MT9M114)
static struct camera_vreg_t msm_8960_back_cam_vreg[] = {
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};
#endif
#endif /* CONFIG_IMX111 */

#ifdef CONFIG_OV2720
static struct camera_vreg_t msm_8960_front_cam_vreg[] = {
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
};
#endif

static struct gpio msm8960_common_cam_gpio[] = {
	{5, GPIOF_DIR_IN, "CAMIF_MCLK"},
	{20, GPIOF_DIR_IN, "CAMIF_I2C_DATA"},
	{21, GPIOF_DIR_IN, "CAMIF_I2C_CLK"},
};


#ifdef CONFIG_MT9V113
static struct gpio msm8960_common_cam_gpio_2[] = {
	{4, GPIOF_DIR_OUT, "CAM2_MCLK"},
};
#endif

#ifdef CONFIG_MT9D115_SUB
static struct gpio msm8960_common_cam_gpio_3[] = {
	{4, GPIOF_DIR_OUT, "CAM2_MCLK"},
	{20, GPIOF_DIR_IN, "CAM_I2C_SDA"},
	{21, GPIOF_DIR_IN, "CAM_I2C_SCL"},
};
#endif

#ifdef CONFIG_OV2720
static struct gpio msm8960_front_cam_gpio[] = {
	{76, GPIOF_DIR_OUT, "CAM_RESET"},
};
#endif

static struct gpio msm8960_back_cam_gpio[] = {
	{107, GPIOF_DIR_OUT, "CAM_RESET"},
};

#ifdef CONFIG_OV2720
static struct msm_gpio_set_tbl msm8960_front_cam_gpio_set_tbl[] = {
	{76, GPIOF_OUT_INIT_LOW, 1000},
	{76, GPIOF_OUT_INIT_HIGH, 4000},
};
#endif

static struct msm_gpio_set_tbl msm8960_back_cam_gpio_set_tbl[] = {
	{107, GPIOF_OUT_INIT_LOW, 1000},
	{107, GPIOF_OUT_INIT_HIGH, 4000},
};

#ifdef CONFIG_OV2720
static struct msm_camera_gpio_conf msm_8960_front_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_front_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_front_cam_gpio),
	.cam_gpio_set_tbl = msm8960_front_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_front_cam_gpio_set_tbl),
};
#endif

#ifdef CONFIG_IMX111
static struct msm_camera_gpio_conf msm_8960_back_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_back_cam_gpio,
	.cam_gpio_req_tbl_size = 0,
	.cam_gpio_set_tbl = msm8960_back_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = 0,
};
#else
static struct msm_camera_gpio_conf msm_8960_back_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_back_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_back_cam_gpio),
	.cam_gpio_set_tbl = msm8960_back_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_back_cam_gpio_set_tbl),
};
#endif

#ifdef CONFIG_MT9V113
static struct msm_camera_gpio_conf msm_8960_front_cam_gpio_conf_2 = {
    .cam_gpiomux_conf_tbl = msm8960_cam_2d_configs2,
    .cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs2),
    .cam_gpio_common_tbl = msm8960_common_cam_gpio_2,
    .cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio_2),
};
#endif

#ifdef CONFIG_MT9D115_SUB
static struct msm_camera_gpio_conf msm_8960_front_cam_gpio_conf_3 = {
    .cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
    .cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
    .cam_gpio_common_tbl = msm8960_common_cam_gpio_3,
    .cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio_3),
};
#endif

#ifdef CONFIG_IMX074
static struct i2c_board_info imx074_actuator_i2c_info = {
	I2C_BOARD_INFO("imx074_act", 0x11),
};

static struct msm_actuator_info imx074_actuator_info = {
	.board_info     = &imx074_actuator_i2c_info,
	.bus_id         = MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = 0,
	.vcm_enable     = 1,
};

static struct msm_camera_sensor_flash_data flash_imx074 = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
#ifdef CONFIG_MSM_CAMERA_FLASH
	.flash_src	= &msm_flash_src
#endif
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx074 = {
	.mount_angle	= 90,
	.cam_vreg = msm_8960_back_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_back_cam_vreg),
	.gpio_conf = &msm_8960_back_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx074_data = {
	.sensor_name	= "imx074",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_imx074,
	.strobe_flash_data = &strobe_flash_xenon,
	.sensor_platform_info = &sensor_board_info_imx074,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
	.actuator_info = &imx074_actuator_info
};
#endif

#ifdef CONFIG_MT9M114
static struct camera_vreg_t msm_8960_mt9m114_vreg[] = {
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};

static struct msm_camera_sensor_flash_data flash_mt9m114 = {
	.flash_type = MSM_CAMERA_FLASH_NONE
};

static struct msm_camera_sensor_platform_info sensor_board_info_mt9m114 = {
	.mount_angle = 90,
	.cam_vreg = msm_8960_mt9m114_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_mt9m114_vreg),
	.gpio_conf = &msm_8960_front_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9m114_data = {
	.sensor_name = "mt9m114",
	.pdata = &msm_camera_csi_device_data[1],
	.flash_data = &flash_mt9m114,
	.sensor_platform_info = &sensor_board_info_mt9m114,
	.csi_if = 1,
	.camera_type = FRONT_CAMERA_2D,
};
#endif

#ifdef CONFIG_OV2720
static struct msm_camera_sensor_flash_data flash_ov2720 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov2720 = {
	.mount_angle	= 0,
	.cam_vreg = msm_8960_front_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_front_cam_vreg),
	.gpio_conf = &msm_8960_front_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov2720_data = {
	.sensor_name	= "ov2720",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_ov2720,
	.sensor_platform_info = &sensor_board_info_ov2720,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};
#endif

#ifdef CONFIG_CE150X
static struct msm_camera_sensor_flash_data flash_ce150x = {
    .flash_type = MSM_CAMERA_FLASH_LED,
#ifdef CONFIG_MSM_CAMERA_FLASH
    .flash_src  = &msm_flash_src
#endif
};

static struct msm_camera_sensor_platform_info sensor_board_info_ce150x = {
    .mount_angle    = 90,
    .sensor_reset   = 107,
};

static struct msm_camera_sensor_info msm_camera_sensor_ce150x_data = {
    .sensor_name    = "ce150x",
    .pdata  = &msm_camera_csi_device_data[0],
    .flash_data = &flash_ce150x,
    .strobe_flash_data = &strobe_flash_xenon,
    .sensor_platform_info = &sensor_board_info_ce150x,
    .csi_if = 1,
    .camera_type = BACK_CAMERA_2D,
};

struct platform_device msm8960_camera_sensor_ce150x = {
    .name   = "msm_camera_ce150x",
    .dev    = {
        .platform_data = &msm_camera_sensor_ce150x_data,
    },
};
#endif /* CONFIG_CE150X */

#ifdef CONFIG_MT9V113
static struct msm_camera_sensor_flash_data flash_mt9v113 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_mt9v113 = {
	.mount_angle	= 270,
	.sensor_reset	= 43,
    .gpio_conf = &msm_8960_front_cam_gpio_conf_2,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9v113_data = {
	.sensor_name	= "mt9v113",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_mt9v113,
	.sensor_platform_info = &sensor_board_info_mt9v113,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};

#endif /* CONFIG_MT9V113 */

#ifdef CONFIG_MT9D115_SUB
static struct msm_camera_sensor_flash_data flash_mt9d115_sub = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_mt9d115_sub = {
	.mount_angle	= 270,
	.sensor_reset	= 43,
    .gpio_conf = &msm_8960_front_cam_gpio_conf_3,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9d115_sub_data = {
	.sensor_name	= "mt9d115_sub",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_mt9d115_sub,
	.sensor_platform_info = &sensor_board_info_mt9d115_sub,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};
#endif /* CONFIG_MT9D115_SUB */

#ifdef CONFIG_IMX111
static struct msm_camera_sensor_flash_data flash_imx111 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
#ifdef CONFIG_MSM_CAMERA_FLASH
	.flash_src	= &msm_flash_src
#endif
};
#endif

#ifdef CONFIG_IMX111_ACT
static struct i2c_board_info imx111_actuator_i2c_info = {
	I2C_BOARD_INFO("imx111_act", 0x0C),
};

static struct msm_actuator_info imx111_actuator_info = {
	.board_info     = &imx111_actuator_i2c_info,
	.bus_id         = MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = 0,
	.vcm_enable     = 1,
};
#endif

#ifdef CONFIG_IMX111
static struct msm_camera_sensor_platform_info sensor_board_info_imx111 = {
	.mount_angle	= 90,
	.sensor_reset	= 107,
    .cam_vreg = msm_8960_back_cam_vreg,
    .num_vreg = ARRAY_SIZE(msm_8960_back_cam_vreg),
    .gpio_conf = &msm_8960_back_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx111_data = {
	.sensor_name	= "imx111",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_imx111,
	.sensor_platform_info = &sensor_board_info_imx111,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
#ifdef CONFIG_IMX111_ACT
	.actuator_info	= &imx111_actuator_info,
#endif
};
#endif /* CONFIG_IMX111 */

static struct camera_vreg_t msm_8960_s5k4ecgx_vreg[] = {

};

static struct msm_camera_sensor_flash_data flash_s5k4ecgx = {
        .flash_type     = MSM_CAMERA_FLASH_LED,
};



static struct msm_camera_sensor_platform_info sensor_board_info_s5k4ecgx = {
        .mount_angle = 90,
        .cam_vreg = msm_8960_s5k4ecgx_vreg,
        .num_vreg = ARRAY_SIZE(msm_8960_s5k4ecgx_vreg),
        .gpio_conf = &msm_8960_back_cam_gpio_conf,
};

struct msm_camera_sensor_info msm_main_camera_sensor_s5k4ecgx_data = {
	.sensor_name		= "s5k4ecgx",
	.pdata			= &msm_camera_csi_device_data[0],	   
	.flash_data		= &flash_s5k4ecgx,
	.sensor_platform_info	= &sensor_board_info_s5k4ecgx,
	.csi_if			= 1,
	.camera_type		= BACK_CAMERA_2D,

};

static struct camera_vreg_t msm_8960_s5k3l1yx_vreg[] = {
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};

static struct msm_camera_sensor_flash_data flash_s5k3l1yx = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_s5k3l1yx = {
	.mount_angle  = 0,
	.cam_vreg = msm_8960_s5k3l1yx_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_s5k3l1yx_vreg),
	.gpio_conf = &msm_8960_back_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k3l1yx_data = {
	.sensor_name          = "s5k3l1yx",
	.pdata                = &msm_camera_csi_device_data[0],
	.flash_data           = &flash_s5k3l1yx,
	.sensor_platform_info = &sensor_board_info_s5k3l1yx,
	.csi_if               = 1,
	.camera_type          = BACK_CAMERA_2D,
};


void __init msm8960_init_cam(void)
{
	msm_gpiomux_install(msm8960_cam_common_configs,
			ARRAY_SIZE(msm8960_cam_common_configs));

	if (machine_is_msm8960_cdp()) {
#ifdef CONFIG_MSM_CAMERA_FLASH
		msm_flash_src._fsrc.ext_driver_src.led_en =
			GPIO_CAM_GP_LED_EN1;
		msm_flash_src._fsrc.ext_driver_src.led_flash_en =
			GPIO_CAM_GP_LED_EN2;
#endif
#ifdef CONFIG_IMX074
		#if defined(CONFIG_I2C) && (defined(CONFIG_GPIO_SX150X) || \
		defined(CONFIG_GPIO_SX150X_MODULE))
		msm_flash_src._fsrc.ext_driver_src.expander_info =
			cam_expander_info;
		#endif
#endif /* CONFIG_IMX074 */
	}

	platform_device_register(&msm8960_device_csiphy0);
	platform_device_register(&msm8960_device_csiphy1);
	platform_device_register(&msm8960_device_csiphy2);
	platform_device_register(&msm8960_device_csid0);
	platform_device_register(&msm8960_device_csid1);
	platform_device_register(&msm8960_device_csid2);
	platform_device_register(&msm8960_device_ispif);
	platform_device_register(&msm8960_device_vfe);
	platform_device_register(&msm8960_device_vpe);
}

#ifdef CONFIG_I2C
static struct i2c_board_info msm8960_camera_i2c_boardinfo[] = {
#ifdef CONFIG_IMX074
	{
	I2C_BOARD_INFO("imx074", 0x1A),
	.platform_data = &msm_camera_sensor_imx074_data,
	},
#endif
#ifdef CONFIG_IMX111
	{
	I2C_BOARD_INFO("imx111", 0x1A),
	.platform_data = &msm_camera_sensor_imx111_data,
	},
#endif
#ifdef CONFIG_S5K4ECGX
	{
	I2C_BOARD_INFO("s5k4ecgx", 0x2d),
	.platform_data = &msm_main_camera_sensor_s5k4ecgx_data,
	},
#endif

#ifdef CONFIG_OV2720
	{
	I2C_BOARD_INFO("ov2720", 0x6C),
	.platform_data = &msm_camera_sensor_ov2720_data,
	},
#endif
#ifdef CONFIG_MT9M114
	{
	I2C_BOARD_INFO("mt9m114", 0x48),
	.platform_data = &msm_camera_sensor_mt9m114_data,
	},
#endif
	{
	I2C_BOARD_INFO("s5k3l1yx", 0x20),
	.platform_data = &msm_camera_sensor_s5k3l1yx_data,
	},
#ifdef CONFIG_MSM_CAMERA_FLASH_SC628A
	{
	I2C_BOARD_INFO("sc628a", 0x6E),
	},
#endif
#ifdef CONFIG_MT9D115_SUB
	{
	I2C_BOARD_INFO("mt9d115_sub", 0x78 >> 1),
	.platform_data = &msm_camera_sensor_mt9d115_sub_data,
	},
#endif /* CONFIG_MT9D115_SUB */
};

struct msm_camera_board_info msm8960_camera_board_info = {
	.board_info = msm8960_camera_i2c_boardinfo,
	.num_i2c_board_info = ARRAY_SIZE(msm8960_camera_i2c_boardinfo),
};

#ifdef CONFIG_MT9V113
static struct i2c_board_info msm8960_camera_i2c_boardinfo_sub[] = {
	{
	I2C_BOARD_INFO("mt9v113", 0x3C),
	.platform_data = &msm_camera_sensor_mt9v113_data,
	},
};

struct msm_camera_board_info msm8960_camera_board_info_sub = {
	.board_info = msm8960_camera_i2c_boardinfo_sub,
	.num_i2c_board_info = ARRAY_SIZE(msm8960_camera_i2c_boardinfo_sub),
};
#endif
#endif
#endif
