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
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include "devices.h"
#include "board-8960.h"


#ifndef CONFIG_TOUCHSCREEN_eKTF2136
static struct gpiomux_setting spi_active_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting spi_active_miso_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

#endif /* CONFIG_TOUCHSCREEN_eKTF2136 */

static struct gpiomux_setting gsbi2 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi2_1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting spi_active_config2 = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir =GPIOMUX_OUT_LOW,
};
static struct gpiomux_setting spi_active_miso = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir =GPIOMUX_OUT_LOW,
};
static struct gpiomux_setting spi_suspended_config2 = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting gsbi3_suspended_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

static struct gpiomux_setting gsbi3_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

#if defined(CONFIG_FEATURE_NCMC_FELICA) || defined(CONFIG_FEATURE_NCMC_IRDA)
static struct gpiomux_setting ncm_2_2_n_l_cfg = {
    .func = GPIOMUX_FUNC_2,
    .drv = GPIOMUX_DRV_2MA,
    .pull = GPIOMUX_PULL_NONE,
    .dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ncm_g_2_n_l_cfg = {
    .func = GPIOMUX_FUNC_GPIO,
    .drv = GPIOMUX_DRV_2MA,
    .pull = GPIOMUX_PULL_NONE,
    .dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ncm_2_2_u_cfg = {
    .func = GPIOMUX_FUNC_2,
    .drv = GPIOMUX_DRV_2MA,
    .pull = GPIOMUX_PULL_UP,
    .dir = GPIOMUX_IN,
};

static struct gpiomux_setting ncm_g_2_d_cfg = {
    .func = GPIOMUX_FUNC_GPIO,
    .drv = GPIOMUX_DRV_2MA,
    .pull = GPIOMUX_PULL_DOWN,
    .dir = GPIOMUX_IN,
};
#endif

#if defined(CONFIG_FEATURE_NCMC_FELICA)
static struct gpiomux_setting ncm_g_2_u_i_cfg = {
    .func = GPIOMUX_FUNC_GPIO,
    .drv = GPIOMUX_DRV_2MA,
    .pull = GPIOMUX_PULL_UP,
    .dir = GPIOMUX_IN,
};
#endif

static struct gpiomux_setting gsbi8 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi8_1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi11 = {
       .func = GPIOMUX_FUNC_1,
       .drv = GPIOMUX_DRV_2MA,
       .pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi11_1 = {
       .func = GPIOMUX_FUNC_1,
       .drv = GPIOMUX_DRV_16MA,
       .pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi12 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting gsbi12_1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting cdc_mclk = {
	.func = GPIOMUX_FUNC_1,
#ifdef CONFIG_FEATURE_NCMC_AUDIO
	.drv = GPIOMUX_DRV_2MA,
#else/* CONFIG_FEATURE_NCMC_AUDIO */
	.drv = GPIOMUX_DRV_8MA,
#endif/* CONFIG_FEATURE_NCMC_AUDIO */
	.pull = GPIOMUX_PULL_NONE,
};

#ifndef CONFIG_FEATURE_NCMC_AUDIO
static struct gpiomux_setting audio_auxpcm[] = {
	/* Suspended state */
	{
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	/* Active state */
	{
		.func = GPIOMUX_FUNC_1,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},
};
#endif/* CONFIG_FEATURE_NCMC_AUDIO */

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static struct gpiomux_setting gpio_eth_config = {
	.pull = GPIOMUX_PULL_NONE,
	.drv = GPIOMUX_DRV_8MA,
	.func = GPIOMUX_FUNC_GPIO,
};
#endif

static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,
#ifdef CONFIG_FEATURE_NCMC_AUDIO
	.drv = GPIOMUX_DRV_2MA,
#else/* CONFIG_FEATURE_NCMC_AUDIO */
	.drv = GPIOMUX_DRV_8MA,
#endif/* CONFIG_FEATURE_NCMC_AUDIO */
	.pull = GPIOMUX_PULL_KEEPER,
};


static struct gpiomux_setting wcnss_5wire_active_cfg2 = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting wcnss_5wire_active_cfg3 = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};


static struct gpiomux_setting cyts_int_sus_cfg2 = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting cyts_int_sus_cfg3 = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};


#ifdef CONFIG_USB_EHCI_MSM_HSIC
static struct gpiomux_setting hsic_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hsic_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting hsic_hub_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif

static struct gpiomux_setting hap_lvl_shft_suspended_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hap_lvl_shft_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting ap2mdm_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdm2ap_status_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting mdm2ap_errfatal_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ap2mdm_kpdpwr_n_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting mdp_vsync_suspend_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdp_vsync_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL

static struct gpiomux_setting hdmi_active_3_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct gpiomux_setting hdmi_active_3_cfg_out = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting hdmi_active_4_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
#endif

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static struct msm_gpiomux_config msm8960_ethernet_configs[] = {
	{
		.gpio = 90,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
		}
	},
	{
		.gpio = 89,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
		}
	},
};
#endif

#ifdef CONFIG_TOUCHSCREEN_eKTF2136
static struct gpiomux_setting touch_int_cfg =
{
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct gpiomux_setting touch_rst_cfg =
{
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct gpiomux_setting touch_susp_cfg =
{
	.func = GPIOMUX_FUNC_GPIO, /*suspend*/
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct gpiomux_setting touch_rstsusp_cfg =
{
	.func = GPIOMUX_FUNC_GPIO, /*suspend*/
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct gpiomux_setting touch_cs_cfg =
{
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};
#endif /* CONFIG_TOUCHSCREEN_eKTF2136 */

static struct msm_gpiomux_config msm8960_gsbi_configs[] __initdata = {
	{
		.gpio      = 12,	/* GSBI2 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi2,
			[GPIOMUX_ACTIVE] = &gsbi2_1,
		},
	},
	{
		.gpio      = 13,	/* GSBI2 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi2,
			[GPIOMUX_ACTIVE] = &gsbi2_1,
		},
	},
	{
		.gpio      = 14,		/* GSBI1 SPI_CS_1 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_suspended_config2,
			[GPIOMUX_ACTIVE] = &spi_active_config2,
		},
	},
	{
		.gpio      = 16,	/* GSBI3 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3_suspended_cfg,
			[GPIOMUX_ACTIVE] = &gsbi3_active_cfg,
		},
	},
	{
		.gpio      = 17,	/* GSBI3 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3_suspended_cfg,
			[GPIOMUX_ACTIVE] = &gsbi3_active_cfg,
		},
	},

       {
               .gpio      = 40,        /* GSBI11 NFC I2C SDA */
               .settings = {
                       [GPIOMUX_SUSPENDED] = &gsbi11,
                       [GPIOMUX_ACTIVE] = &gsbi11_1,
               },
       },
       {
               .gpio      = 41,        /* GSBI11 NFC I2C SCL */
               .settings = {
                       [GPIOMUX_SUSPENDED] = &gsbi11,
                       [GPIOMUX_ACTIVE] = &gsbi11_1,
               },
       },
	{
		.gpio      = 44,	/* GSBI12 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi12,
			[GPIOMUX_ACTIVE] = &gsbi12_1,
		},
	},
	{
		.gpio      = 45,	/* GSBI12 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi12,
			[GPIOMUX_ACTIVE] = &gsbi12_1,
		},
	},
#if defined(CONFIG_FEATURE_NCMC_FELICA) || defined(CONFIG_FEATURE_NCMC_IRDA)
	{
		.gpio = 71,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_2_2_n_l_cfg,
			[GPIOMUX_ACTIVE] = &ncm_g_2_n_l_cfg,
		},
	},
	{
		.gpio = 72,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_2_2_u_cfg,
			[GPIOMUX_ACTIVE] = &ncm_g_2_d_cfg,
		},
	},
#endif
	{
		.gpio      = 36,	/* GSBI8 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi8,
			[GPIOMUX_ACTIVE] = &gsbi8_1,
		},
	},
	{
		.gpio      = 37,	/* GSBI8 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi8,
			[GPIOMUX_ACTIVE] = &gsbi8_1,
		},
	},
#ifndef CONFIG_TOUCHSCREEN_eKTF2136
	{
		.gpio      = 93,		/* GSBI9 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_active_config,
			[GPIOMUX_ACTIVE] = &spi_active_config,
		},
	},
	{
		.gpio      = 94,		/* GSBI9 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_active_miso_config,
			[GPIOMUX_ACTIVE] = &spi_active_miso_config,
		},
	},
	{
		.gpio      = 95,		/* GSBI9 QUP SPI_CS_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_active_config,
			[GPIOMUX_ACTIVE] = &spi_active_config,
		},
	},
	{
		.gpio      = 96,		/* GSBI9 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_active_config,
			[GPIOMUX_ACTIVE] = &spi_active_config,
		},
	},
#else /* CONFIG_TOUCHSCREEN_eKTF2136 */
	{
		.gpio      = 93,		/* GSBI9 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_suspended_config2, 
			[GPIOMUX_ACTIVE] = &spi_active_config2,

		},
	},
	{
		.gpio      = 94,		/* GSBI9 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_suspended_config2, 
			[GPIOMUX_ACTIVE] = &spi_active_miso,
		},
	},
	{
		.gpio      = 95,		/* GSBI9 QUP SPI_CS_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &touch_cs_cfg,
		},
	},
	{
		.gpio      = 96,		/* GSBI9 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spi_suspended_config2, 
			[GPIOMUX_ACTIVE] = &spi_active_config2, 
		},
	},
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] = &touch_rstsusp_cfg,
			[GPIOMUX_ACTIVE] = &touch_rst_cfg,
		},
	},
	{
		.gpio = 46,
		.settings = {
			[GPIOMUX_SUSPENDED] = &touch_susp_cfg,
			[GPIOMUX_ACTIVE] = &touch_int_cfg,
		},
	},
#endif /* CONFIG_TOUCHSCREEN_eKTF2136 */
#if defined(CONFIG_FEATURE_NCMC_FELICA)
    /* FE_RFS */
    {
        .gpio = 7,
        .settings = {
            [GPIOMUX_SUSPENDED] = &ncm_g_2_u_i_cfg,
        },
    },
    /* FE_PON */
    {
        .gpio = 9,
        .settings = {
            [GPIOMUX_SUSPENDED] = &ncm_g_2_n_l_cfg,
        },
    },
    /* FE_INT */
    {
        .gpio = 65,
        .settings = {
            [GPIOMUX_SUSPENDED] = &ncm_g_2_u_i_cfg,
        },
    },
#endif
};

static struct msm_gpiomux_config msm8960_slimbus_config[] __initdata = {
	{
		.gpio	= 60,		/* slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio	= 61,		/* slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};

static struct msm_gpiomux_config msm8960_audio_codec_configs[] __initdata = {
	{
		.gpio = 59,
		.settings = {
			[GPIOMUX_SUSPENDED] = &cdc_mclk,
		},
	},
};

#ifndef CONFIG_FEATURE_NCMC_AUDIO
static struct msm_gpiomux_config msm8960_audio_auxpcm_configs[] __initdata = {
	{
		.gpio = 63,
		.settings = {
			[GPIOMUX_SUSPENDED] = &audio_auxpcm[0],
			[GPIOMUX_ACTIVE] = &audio_auxpcm[1],
		},
	},
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_SUSPENDED] = &audio_auxpcm[0],
			[GPIOMUX_ACTIVE] = &audio_auxpcm[1],
		},
	},
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_SUSPENDED] = &audio_auxpcm[0],
			[GPIOMUX_ACTIVE] = &audio_auxpcm[1],
		},
	},
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_SUSPENDED] = &audio_auxpcm[0],
			[GPIOMUX_ACTIVE] = &audio_auxpcm[1],
		},
	},
};
#endif/* CONFIG_FEATURE_NCMC_AUDIO */


static struct msm_gpiomux_config wcnss_5wire_interface[] = {
	{
		.gpio = 84,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg2,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_active_cfg2,
		},
	},
	{
		.gpio = 85,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg2,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_active_cfg2,
		},
	},
	{
		.gpio = 86,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg2,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_active_cfg2,
		},
	},
	{
		.gpio = 87,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg3,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_active_cfg3,
		},
	},
	{
		.gpio = 88,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg3,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_active_cfg3,
		},
	},
};

static struct msm_gpiomux_config msm8960_cyts_configs[] __initdata = {
	{	/* TS INTERRUPT */
		.gpio = 11,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_int_sus_cfg2,
			[GPIOMUX_SUSPENDED] = &cyts_int_sus_cfg2,
		},
	},
	{	/* TS SLEEP */
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_int_sus_cfg3,
			[GPIOMUX_SUSPENDED] = &cyts_int_sus_cfg3,
		},
	},
};

#ifdef CONFIG_USB_EHCI_MSM_HSIC
static struct msm_gpiomux_config msm8960_hsic_configs[] = {
	{
		.gpio = 150,               /*HSIC_STROBE */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 151,               /* HSIC_DATA */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 91,               /* HSIC_HUB_RESET */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_hub_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
};
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct gpiomux_setting sdcc4_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdcc4_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdcc4_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdcc4_data_1_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8960_sdcc4_configs[] __initdata = {
	{
		/* SDC4_DATA_3 */
		.gpio      = 83,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc4_suspend_cfg,
		},
	},
	{
		/* SDC4_DATA_2 */
		.gpio      = 84,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc4_suspend_cfg,
		},
	},
	{
		/* SDC4_DATA_1 */
		.gpio      = 85,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc4_data_1_suspend_cfg,
		},
	},
	{
		/* SDC4_DATA_0 */
		.gpio      = 86,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc4_suspend_cfg,
		},
	},
	{
		/* SDC4_CMD */
		.gpio      = 87,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc4_suspend_cfg,
		},
	},
	{
		/* SDC4_CLK */
		.gpio      = 88,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc4_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc4_suspend_cfg,
		},
	},
};
#endif


static struct msm_gpiomux_config hap_lvl_shft_config[] __initdata = {
	{
		.gpio = 47,
		.settings = {
			[GPIOMUX_SUSPENDED] = &hap_lvl_shft_suspended_config,
			[GPIOMUX_ACTIVE] = &hap_lvl_shft_active_config,
		},
	},
};

#ifdef CONFIG_FEATURE_NCMC_USB
#ifndef CONFIG_FEATURE_NCMC_RUBY
static struct gpiomux_setting usb_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config msm8960_usb_sw_configs[] __initdata = {
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_SUSPENDED] = &usb_cfg,
			[GPIOMUX_ACTIVE] = &usb_cfg,
		},
	},
};
#endif /* !CONFIG_FEATURE_NCMC_RUBY */
#endif /* CONFIG_FEATURE_NCMC_USB */

#ifdef CONFIG_FEATURE_NCMC_DTV
static struct gpiomux_setting dtv_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting tsif_cfg_disable_2MA = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting tsif_cfg_enable_2MA = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_IN,
};

static struct msm_gpiomux_config dtv_configs[] __initdata = {
	/* DTV_RST */
	{
		.gpio = 73,
		.settings = {
			[GPIOMUX_ACTIVE]    = &dtv_reset_cfg,
			[GPIOMUX_SUSPENDED] = &dtv_reset_cfg,
		},
	},
	/* TSIF_CLK */
	{
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
	/* TSIF_EN */
	{
		.gpio = 76,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
	/* TSIF_DATA */
	{
		.gpio = 77,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
	/* TSIF_SYNC */
	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
};
#endif /* CONFIG_FEATURE_NCMC_DTV */

#if defined(CONFIG_LEDS_ADP8861) || defined(CONFIG_LEDS_LM3532) || defined(CONFIG_LEDS_LM3537)
static struct gpiomux_setting led_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_HIGH,
};

static struct msm_gpiomux_config led_configs[] __initdata = {
	/* LED_RST ADP8861 LM3532 LM3537 */
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE]    = &led_reset_cfg,
			[GPIOMUX_SUSPENDED] = &led_reset_cfg,
		},
	},
};
#endif

static struct msm_gpiomux_config mdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 94,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 70,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 95,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_KPDPWR_N */
	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_kpdpwr_n_cfg,
		}
	},
	/* AP2MDM_PMIC_RESET_N */
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_kpdpwr_n_cfg,
		}
	}
};

static struct msm_gpiomux_config msm8960_mdp_vsync_configs[] __initdata = {
	{
		.gpio = 0,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mdp_vsync_active_cfg,
			[GPIOMUX_SUSPENDED] = &mdp_vsync_suspend_cfg,
		},
	}
};

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
static struct msm_gpiomux_config msm8960_hdmi_configs[] __initdata = {
	{
		.gpio = 99,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_3_cfg_out,
			[GPIOMUX_SUSPENDED] = &hdmi_active_3_cfg_out,
		},
	},
	{
		.gpio = 100,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_3_cfg_out,
			[GPIOMUX_SUSPENDED] = &hdmi_active_3_cfg_out,
		},
	},
	{
		.gpio = 101,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_3_cfg_out,
			[GPIOMUX_SUSPENDED] = &hdmi_active_3_cfg_out,
		},
	},
	{
		.gpio = 102,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_3_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_active_3_cfg,
		},
	},
	{
		.gpio = 15,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_3_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_active_3_cfg_out,
		},
	},
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_4_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_active_3_cfg_out,
		},
	},
};
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct gpiomux_setting sdcc2_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdcc2_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdcc2_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdcc2_data_1_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8960_sdcc2_configs[] __initdata = {
	{
		/* DATA_3 */
		.gpio      = 92,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* DATA_2 */
		.gpio      = 91,
		.settings = {
		[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
		[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* DATA_1 */
		.gpio      = 90,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_data_1_suspend_cfg,
		},
	},
	{
		/* DATA_0 */
		.gpio      = 89,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* CMD */
		.gpio      = 97,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* CLK */
		.gpio      = 98,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
};
#endif

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
static struct gpiomux_setting synaptics_cs_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting synaptics_int_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting synaptics_2p8v_en_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config msm8960_synaptics_configs[] __initdata = {
	{
		.gpio = 95,
		.settings = {
			[GPIOMUX_ACTIVE]    = &synaptics_cs_cfg,
			[GPIOMUX_SUSPENDED] = &synaptics_cs_cfg,
		},
	},
	{
		.gpio = 46,
		.settings = {
			[GPIOMUX_ACTIVE]    = &synaptics_int_cfg,
			[GPIOMUX_SUSPENDED] = &synaptics_int_cfg,
		},
	},
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_ACTIVE]    = &synaptics_2p8v_en_cfg,
			[GPIOMUX_SUSPENDED] = &synaptics_2p8v_en_cfg,
		},
	},
};

#endif /* defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM) */
static struct gpiomux_setting sensor_prox_light_int_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting sensor_prox_light_int_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting sensor_akm_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting sensor_akm_rst_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};
	
static struct gpiomux_setting sensor_akm_int_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
	
static struct gpiomux_setting sensor_akm_int_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
	
static struct msm_gpiomux_config msm8960_sensor_configs[] __initdata = {
	{
		.gpio = 49,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensor_prox_light_int_active_cfg,
			[GPIOMUX_SUSPENDED] = &sensor_prox_light_int_suspended_cfg,
		},
	},
	{
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensor_akm_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &sensor_akm_rst_suspended_cfg,
		},
	},
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensor_akm_int_active_cfg,
			[GPIOMUX_SUSPENDED] = &sensor_akm_int_suspended_cfg,
		},
	},
};

int __init msm8960_init_gpiomux(void)
{
	int rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return rc;
	}

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	msm_gpiomux_install(msm8960_ethernet_configs,
			ARRAY_SIZE(msm8960_ethernet_configs));
#endif

	msm_gpiomux_install(msm8960_gsbi_configs,
			ARRAY_SIZE(msm8960_gsbi_configs));

	msm_gpiomux_install(msm8960_cyts_configs,
			ARRAY_SIZE(msm8960_cyts_configs));

	msm_gpiomux_install(msm8960_slimbus_config,
			ARRAY_SIZE(msm8960_slimbus_config));

	msm_gpiomux_install(msm8960_audio_codec_configs,
			ARRAY_SIZE(msm8960_audio_codec_configs));

#ifndef CONFIG_FEATURE_NCMC_AUDIO
	msm_gpiomux_install(msm8960_audio_auxpcm_configs,
			ARRAY_SIZE(msm8960_audio_auxpcm_configs));
#endif/* CONFIG_FEATURE_NCMC_AUDIO */


	msm_gpiomux_install(wcnss_5wire_interface,
			ARRAY_SIZE(wcnss_5wire_interface));

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	msm_gpiomux_install(msm8960_sdcc4_configs,
		ARRAY_SIZE(msm8960_sdcc4_configs));
#endif

	if (machine_is_msm8960_mtp() || machine_is_msm8960_fluid() ||
		machine_is_msm8960_liquid() || machine_is_msm8960_cdp())
		msm_gpiomux_install(hap_lvl_shft_config,
			ARRAY_SIZE(hap_lvl_shft_config));

	if (PLATFORM_IS_CHARM25())
		msm_gpiomux_install(mdm_configs,
			ARRAY_SIZE(mdm_configs));

#ifdef CONFIG_USB_EHCI_MSM_HSIC
	if ((SOCINFO_VERSION_MAJOR(socinfo_get_version()) != 1) &&
		(PLATFORM_IS_CHARM25() || machine_is_msm8960_liquid()))
		msm_gpiomux_install(msm8960_hsic_configs,
			ARRAY_SIZE(msm8960_hsic_configs));
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
	msm_gpiomux_install(msm8960_hdmi_configs,
			ARRAY_SIZE(msm8960_hdmi_configs));
#endif

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
	msm_gpiomux_install(msm8960_synaptics_configs,
			ARRAY_SIZE(msm8960_synaptics_configs));
#endif /* defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM) */

	msm_gpiomux_install(msm8960_mdp_vsync_configs,
			ARRAY_SIZE(msm8960_mdp_vsync_configs));

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	msm_gpiomux_install(msm8960_sdcc2_configs,
		ARRAY_SIZE(msm8960_sdcc2_configs));
#endif
#ifdef CONFIG_FEATURE_NCMC_USB
#ifndef CONFIG_FEATURE_NCMC_RUBY
	msm_gpiomux_install(msm8960_usb_sw_configs,
			ARRAY_SIZE(msm8960_usb_sw_configs));
#endif /* !CONFIG_FEATURE_NCMC_RUBY */
#endif /* CONFIG_FEATURE_NCMC_USB */

	msm_gpiomux_install(msm8960_sensor_configs,
			ARRAY_SIZE(msm8960_sensor_configs));
	
#ifdef CONFIG_FEATURE_NCMC_DTV
	msm_gpiomux_install(dtv_configs,
			ARRAY_SIZE(dtv_configs));
#endif /* CONFIG_FEATURE_NCMC_DTV */

#if defined(CONFIG_LEDS_ADP8861) || defined(CONFIG_LEDS_LM3532) || defined(CONFIG_LEDS_LM3537)
	msm_gpiomux_install(led_configs,
			ARRAY_SIZE(led_configs));
#endif

	return 0;
}
