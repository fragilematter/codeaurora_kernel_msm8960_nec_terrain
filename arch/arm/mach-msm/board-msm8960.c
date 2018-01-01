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

/*
 * Copyright (C) 2011 NEC CASIO Mobile Communications, Ltd.
 *
 *  No permission to use, copy, modify and distribute this software
 *  and its documentation for any purpose is granted.
 *  This software is provided under applicable license agreement only.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/i2c/sx150x.h>
#include <linux/i2c/isl9519.h>
#include <linux/gpio.h>
#include <linux/msm_ssbi.h>
#include <linux/regulator/gpio-regulator.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/mfd/pm8xxx/pm8921-adc.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>
#include <linux/slimbus/slimbus.h>
#include <linux/bootmem.h>
#include <linux/msm_kgsl.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <linux/cyttsp.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/qcom_crypto_device.h>
#include <linux/platform_data/qcom_wcnss_device.h>
#include <linux/leds.h>
#include <linux/leds-pm8xxx.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/msm_tsens.h>
#include <linux/ks8851.h>
#include <linux/i2c/isa1200.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>
#include <asm/hardware/gic.h>
#include <asm/mach/mmc.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_spi.h>
#ifdef CONFIG_USB_MSM_OTG_72K
#include <mach/msm_hsusb.h>
#else
#include <linux/usb/msm_hsusb.h>
#endif
#include <linux/usb/android.h>
#include <mach/usbdiag.h>
#include <mach/socinfo.h>
#include <mach/rpm.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_memtypes.h>
#include <mach/dma.h>
#include <mach/msm_dsps.h>
#include <mach/msm_xo.h>
#include <mach/restart.h>

#ifdef CONFIG_WCD9310_CODEC
#include <linux/slimbus/slimbus.h>
#include <linux/mfd/wcd9310/core.h>
#include <linux/mfd/wcd9310/pdata.h>
#endif

#include <linux/ion.h>
#include <mach/ion.h>
#include <mach/mdm2.h>
#include <linux/mfd/pm8xxx/vibrator.h>

#include "timer.h"
#include "devices.h"
#include "devices-msm8x60.h"
#include "spm.h"
#include "board-msm8960.h"
#include "pm.h"
#include "cpuidle.h"
#include "rpm_resources.h"
#include "mpm.h"
#include "acpuclock.h"
#include "rpm_log.h"
#include "smd_private.h"
#include "pm-boot.h"
#include "msm_watchdog.h"

#define PLATFORM_IS_CHARM25() \
	(machine_is_msm8960_cdp() && \
		(socinfo_get_platform_subtype() == 1) \
	)

#define NCM_FUNCTION
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
#include <linux/synaptics_ncm.h>
#endif /* defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM) */

#ifdef CONFIG_FEATURE_NCMC_USB
#ifndef CONFIG_FEATURE_NCMC_RUBY
#include <linux/i2c/bd91401gw.h>
#endif /* !CONFIG_FEATURE_NCMC_RUBY */
#endif /*CONFIG_FEATURE_NCMC_USB*/

#ifdef CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM
#include <linux/anadev_ncm_haptics.h>
#endif /* CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM */

#include <linux/i2c/i2c_sensors.h>

#include <linux/i2c/sndamp_i2c.h>

#if defined(CONFIG_LEDS_LM3537)
#include <linux/leds-lm3537.h>
#endif /* #if defined(CONFIG_LEDS_LM3537) */
#if defined(CONFIG_LEDS_ADP8861)
#include <linux/leds-adp8861.h>
#endif /* #if defined(CONFIG_LEDS_ADP8861) */

#include <linux/oemnc_info.h>

static struct platform_device msm_fm_platform_init = {
	.name = "iris_fm",
	.id   = -1,
};

#ifndef NCM_FUNCTION
struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio			config;
};

struct pm8xxx_mpp_init {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};

#define PM8XXX_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8XXX_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8921_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8XXX_GPIO_DISABLE(_gpio) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, 0, 0, 0, PM_GPIO_VIN_S4, \
			 0, 0, 0, 1)

#define PM8XXX_GPIO_OUTPUT(_gpio, _val) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8XXX_GPIO_INPUT(_gpio, _pull) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0, \
			_pull, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_NO, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8XXX_GPIO_OUTPUT_FUNC(_gpio, _val, _func) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			_func, 0, 0)

#define PM8XXX_GPIO_OUTPUT_VIN(_gpio, _val, _vin) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, _vin, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

/* Initial PM8921 GPIO configurations */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
	PM8XXX_GPIO_DISABLE(6),				 /* Disable unused */
	PM8XXX_GPIO_DISABLE(7),				 /* Disable NFC */
	PM8XXX_GPIO_INPUT(16,	    PM_GPIO_PULL_UP_30), /* SD_CARD_WP */
    /* External regulator shared by display and touchscreen on LiQUID */
	PM8XXX_GPIO_OUTPUT(17,	    0),			 /* DISP 3.3 V Boost */
	PM8XXX_GPIO_OUTPUT_VIN(21, 1, PM_GPIO_VIN_VPH),	 /* Backlight Enable */
	PM8XXX_GPIO_DISABLE(22),			 /* Disable NFC */
	PM8XXX_GPIO_OUTPUT_FUNC(24, 0, PM_GPIO_FUNC_2),	 /* Bl: Off, PWM mode */
	PM8XXX_GPIO_INPUT(26,	    PM_GPIO_PULL_UP_30), /* SD_CARD_DET_N */
	PM8XXX_GPIO_OUTPUT(43,	    PM_GPIO_PULL_UP_30), /* DISP_RESET_N */
	PM8XXX_GPIO_OUTPUT(42, 0),                      /* USB 5V reg enable */
};

/* Initial PM8921 MPP configurations */
static struct pm8xxx_mpp_init pm8921_mpps[] __initdata = {
	/* External 5V regulator enable; shared by HDMI and USB_OTG switches. */
	PM8XXX_MPP_INIT(7, D_INPUT, PM8921_MPP_DIG_LEVEL_VPH, DIN_TO_INT),
	PM8XXX_MPP_INIT(PM8921_AMUX_MPP_8, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH8,
								DOUT_CTRL_LOW),
};
#endif /* NCM_FUNCTION */

static void __init pm8921_gpio_mpp_init(void)
{
	int i, rc;

#ifdef NCM_FUNCTION
    uint32_t hw_rev = 0;
    
    hw_rev = hw_revision_read();
    
  if (hw_rev == HW_REV_5P0) {
      for (i = 0; i < nc_pm8921_gpios_num; i++) {
        rc = pm8xxx_gpio_config(nc_pm8921_gpios[i].gpio,
              &nc_pm8921_gpios[i].config);
        if (rc) {
          pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);
          break;
        }
      }
    
      for (i = 0; i < nc_pm8921_mpps_num; i++) {
        rc = pm8xxx_mpp_config(nc_pm8921_mpps[i].mpp,
              &nc_pm8921_mpps[i].config);
        if (rc) {
          pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
          break;
        }
      }
  
  } else {
      for (i = 0; i < nc_pm8921_gpios_oem_num; i++) {
        rc = pm8xxx_gpio_config(nc_pm8921_gpios_oem[i].gpio,
              &nc_pm8921_gpios_oem[i].config);
        if (rc) {
          pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);
          break;
        }
      }
    
      for (i = 0; i < nc_pm8921_mpps_oem_num; i++) {
        rc = pm8xxx_mpp_config(nc_pm8921_mpps_oem[i].mpp,
              &nc_pm8921_mpps_oem[i].config);
        if (rc) {
          pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
          break;
        }
      }
  }
#else
	for (i = 0; i < ARRAY_SIZE(pm8921_gpios); i++) {
		rc = pm8xxx_gpio_config(pm8921_gpios[i].gpio,
					&pm8921_gpios[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(pm8921_mpps); i++) {
		rc = pm8xxx_mpp_config(pm8921_mpps[i].mpp,
					&pm8921_mpps[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
			break;
		}
	}
#endif /* NCM_FUNCTION */
}

#ifndef CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB
#define CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB
#endif

#if defined(CONFIG_GPIO_SX150X) || defined(CONFIG_GPIO_SX150X_MODULE)
enum {
	GPIO_EXPANDER_IRQ_BASE = (PM8921_IRQ_BASE + PM8921_NR_IRQS),
	GPIO_EXPANDER_GPIO_BASE = (PM8921_MPP_BASE + PM8921_NR_MPPS),
	/* CAM Expander */
	GPIO_CAM_EXPANDER_BASE = GPIO_EXPANDER_GPIO_BASE,
	GPIO_CAM_GP_STROBE_READY = GPIO_CAM_EXPANDER_BASE,
	GPIO_CAM_GP_AFBUSY,
	GPIO_CAM_GP_STROBE_CE,
	GPIO_CAM_GP_CAM1MP_XCLR,
	GPIO_CAM_GP_CAMIF_RESET_N,
	GPIO_CAM_GP_XMT_FLASH_INT,
	GPIO_CAM_GP_LED_EN1,
	GPIO_CAM_GP_LED_EN2,

};
#endif


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
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
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


static struct gpiomux_setting ncm_2_2_d_l_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ncm_g_2_d_l_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ncm_2_2_d_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ncm_g_2_d_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

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

static struct gpiomux_setting ncm_g_2_u_i_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting ncm_g_2_n_l_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gsbi12 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi12_1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting cdc_mclk = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};


#undef CONFIG_KS8851

static struct gpiomux_setting audio_auxpcm[] = {
	/* Suspended state */
	{
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	/* Active state */
	{
		.func = GPIOMUX_FUNC_1,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},
};

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static struct gpiomux_setting gpio_eth_config = {
	.pull = GPIOMUX_PULL_NONE,
	.drv = GPIOMUX_DRV_8MA,
	.func = GPIOMUX_FUNC_GPIO,
};
#endif

static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

struct msm_gpiomux_config msm8960_gpiomux_configs[NR_GPIO_IRQS] = {
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	{
		.gpio = KS8851_IRQ_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
		}
	},
	{
		.gpio = KS8851_RST_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
		}
	},
#endif
};

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
	{
		.gpio = 71,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_2_2_d_l_cfg,
			[GPIOMUX_ACTIVE] = &ncm_g_2_d_l_cfg,
		},
	},
	{
		.gpio = 72,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_2_2_d_cfg,
			[GPIOMUX_ACTIVE] = &ncm_g_2_d_cfg,
		},
	},
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
	/* FE_IO_EN */
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_ACTIVE] = &ncm_g_2_n_l_cfg,
		},
	},
	/* FE_INT */
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_g_2_u_i_cfg,
		},
	},
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

static struct gpiomux_setting wcnss_5wire_active_cfg2 = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wcnss_5wire_active_cfg3 = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

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
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg2,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_active_cfg2,
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

static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend*/
		.drv = GPIOMUX_DRV_16MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_IN
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 1*/
		.drv = GPIOMUX_DRV_16MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 2*/
		.drv = GPIOMUX_DRV_16MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 3*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 4*/
		.drv = GPIOMUX_DRV_16MA,
		.pull = GPIOMUX_PULL_UP,
		.dir = GPIOMUX_IN
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 5*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_HIGH,
	},
	{
		.func = GPIOMUX_FUNC_2, 
		.drv = GPIOMUX_DRV_16MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},
};

static struct msm_gpiomux_config msm8960_cam_common_configs[] = {
	{
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = 3,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = 4,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[6],
			[GPIOMUX_SUSPENDED] = &cam_settings[6],
		},
	},
	{
		.gpio = 5,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = 22,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 23,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[4],
		},
	},
	{
		.gpio = 24,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 25,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = 43,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = 54,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 107,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
};

static struct msm_gpiomux_config msm8960_cam_2d_configs[] = {
	{
		.gpio = 20,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 21,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
};

/* MOD-S CAMERA DRIVER */
#ifdef CONFIG_MT9M113
static struct msm_gpiomux_config msm8960_cam_2d_configs2[] = {
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
};
#endif /* CONFIG_MT9M113 */
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

#define HAP_SHIFT_LVL_OE_GPIO	47

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

static struct msm_gpiomux_config hap_lvl_shft_config[] __initdata = {
	{
		.gpio = HAP_SHIFT_LVL_OE_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &hap_lvl_shft_suspended_config,
			[GPIOMUX_ACTIVE] = &hap_lvl_shft_active_config,
		},
	},
};

#if defined(CONFIG_GPIO_SX150X) || defined(CONFIG_GPIO_SX150X_MODULE)
enum {
	SX150X_CAM,
};

#ifdef CONFIG_IMX074
static struct sx150x_platform_data sx150x_data[] = {
	[SX150X_CAM] = {
		.gpio_base         = GPIO_CAM_EXPANDER_BASE,
		.oscio_is_gpo      = false,
		.io_pullup_ena     = 0x0,
		.io_pulldn_ena     = 0xc0,
		.io_open_drain_ena = 0x0,
		.irq_summary       = -1,
	},
};
#endif
#endif

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

#ifdef CONFIG_I2C

#define MSM_8960_GSBI2_QUP_I2C_BUS_ID 2

#define MSM_8960_GSBI4_QUP_I2C_BUS_ID 4
#define MSM_8960_GSBI3_QUP_I2C_BUS_ID 3
#define MSM_8960_GSBI8_QUP_I2C_BUS_ID 8
#define MSM_8960_GSBI10_QUP_I2C_BUS_ID 10

#ifdef CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM
#define MSM_8960_GSBI8_QUP_I2C_BUS_ID 8
#endif /* CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM */
#define MSM_8960_GSBI12_QUP_I2C_BUS_ID 12
#ifdef CONFIG_FEATURE_NCMC_DTV
static struct i2c_board_info nim_boardinfo[] __initdata = {
    {
        I2C_BOARD_INFO("nim", 0x61),
    }
};
#endif /* CONFIG_FEATURE_NCMC_DTV */

#ifdef CONFIG_FEATURE_NCMC_DTV
static struct gpiomux_setting dtv_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
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
};
#endif /* CONFIG_FEATURE_NCMC_DTV */


#if defined(CONFIG_GPIO_SX150X) || defined(CONFIG_GPIO_SX150X_MODULE)
#ifdef CONFIG_IMX074
static struct i2c_board_info cam_expander_i2c_info[] = {
	{
		I2C_BOARD_INFO("sx1508q", 0x22),
		.platform_data = &sx150x_data[SX150X_CAM]
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
#endif

#define MSM_PMEM_ADSP_SIZE         0x3800000
#define MSM_PMEM_AUDIO_SIZE        0x28B000
#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#else
#define MSM_PMEM_SIZE 0x1C00000 /* 28 Mbytes */
#endif


#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
#define MSM_PMEM_KERNEL_EBI1_SIZE  0xB0C000
#define MSM_ION_EBI_SIZE	(MSM_PMEM_SIZE + 0x600000)
#define MSM_ION_ADSP_SIZE	MSM_PMEM_ADSP_SIZE
#define MSM_ION_HEAP_NUM	4
#else
#define MSM_PMEM_KERNEL_EBI1_SIZE  0x110C000
#define MSM_ION_HEAP_NUM	2
#endif

#ifdef CONFIG_KERNEL_PMEM_EBI_REGION
static unsigned pmem_kernel_ebi1_size = MSM_PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	pmem_kernel_ebi1_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size", pmem_kernel_ebi1_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
static unsigned pmem_size = MSM_PMEM_SIZE;
static int __init pmem_size_setup(char *p)
{
	pmem_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_size", pmem_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;

static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;

static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = {.platform_data = &android_pmem_pdata},
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};
static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};
#endif

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};
#endif

#define DSP_RAM_BASE_8960 0x8da00000
#define DSP_RAM_SIZE_8960 0x1800000
static int dspcrashd_pdata_8960 = 0xDEADDEAD;

static struct resource resources_dspcrashd_8960[] = {
	{
		.name   = "msm_dspcrashd",
		.start  = DSP_RAM_BASE_8960,
		.end    = DSP_RAM_BASE_8960 + DSP_RAM_SIZE_8960,
		.flags  = IORESOURCE_DMA,
	},
};

struct platform_device msm_device_dspcrashd_8960 = {
	.name           = "msm_dspcrashd",
	.num_resources  = ARRAY_SIZE(resources_dspcrashd_8960),
	.resource       = resources_dspcrashd_8960,
	.dev = { .platform_data = &dspcrashd_pdata_8960 },
};

static struct memtype_reserve msm8960_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	android_pmem_adsp_pdata.size = pmem_adsp_size;
	android_pmem_pdata.size = pmem_size;
#endif
	android_pmem_audio_pdata.size = MSM_PMEM_AUDIO_SIZE;
#endif
}

static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	msm8960_reserve_table[p->memory_type].size += p->size;
}

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_pdata);
#endif
	reserve_memory_for(&android_pmem_audio_pdata);
	msm8960_reserve_table[MEMTYPE_EBI1].size += pmem_kernel_ebi1_size;
#endif
}

static int msm8960_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

#ifdef CONFIG_ION_MSM
struct ion_platform_data ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.heaps = {
		{
			.id	= ION_HEAP_SYSTEM_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_KMALLOC_HEAP_NAME,
		},
		{
			.id	= ION_HEAP_SYSTEM_CONTIG_ID,
			.type	= ION_HEAP_TYPE_SYSTEM_CONTIG,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		{
			.id	= ION_HEAP_EBI_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_EBI1_HEAP_NAME,
			.size	= MSM_ION_EBI_SIZE,
			.memory_type = ION_EBI_TYPE,
		},
		{
			.id	= ION_HEAP_ADSP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_ADSP_HEAP_NAME,
			.size	= MSM_ION_ADSP_SIZE,
			.memory_type = ION_EBI_TYPE,
		},
#endif
	}
};

struct platform_device ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &ion_pdata },
};
#endif

static void reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	msm8960_reserve_table[MEMTYPE_EBI1].size += MSM_ION_EBI_SIZE;
	msm8960_reserve_table[MEMTYPE_EBI1].size += MSM_ION_ADSP_SIZE;
#endif
}
static void __init msm8960_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
	reserve_ion_memory();
}

static struct reserve_info msm8960_reserve_info __initdata = {
	.memtype_reserve_table = msm8960_reserve_table,
	.calculate_reserve_sizes = msm8960_calculate_reserve_sizes,
	.paddr_to_memtype = msm8960_paddr_to_memtype,
};

static int msm8960_memory_bank_size(void)
{
	return 1<<29;
}

static void __init locate_unstable_memory(void)
{
	struct membank *mb = &meminfo.bank[meminfo.nr_banks - 1];
	unsigned long bank_size;
	unsigned long low, high;

	bank_size = msm8960_memory_bank_size();
	low = meminfo.bank[0].start;
	high = mb->start + mb->size;

	/* Check if 32 bit overflow occured */
	if (high < mb->start)
		high = ~0UL;

	low &= ~(bank_size - 1);

	if (high - low <= bank_size)
		return;
	msm8960_reserve_info.low_unstable_address = low + bank_size;
	/* To avoid overflow of u32 compute max_unstable_size
	 * by first subtracting low from mb->start)
	 * */
	msm8960_reserve_info.max_unstable_size = (mb->start - low) +
						mb->size - bank_size;

	msm8960_reserve_info.bank_size = bank_size;
	pr_info("low unstable address %lx max size %lx bank size %lx\n",
		msm8960_reserve_info.low_unstable_address,
		msm8960_reserve_info.max_unstable_size,
		msm8960_reserve_info.bank_size);
}

static void __init place_movable_zone(void)
{
	movable_reserved_start = msm8960_reserve_info.low_unstable_address;
	movable_reserved_size = msm8960_reserve_info.max_unstable_size;
	pr_info("movable zone start %lx size %lx\n",
		movable_reserved_start, movable_reserved_size);
}

static void __init msm8960_early_memory(void)
{
	reserve_info = &msm8960_reserve_info;
	locate_unstable_memory();
	place_movable_zone();
}

static void __init msm8960_reserve(void)
{
	msm_reserve();
}

static int msm8960_change_memory_power(u64 start, u64 size,
	int change_type)
{
	return soc_change_memory_power(start, size, change_type);
}

#ifdef CONFIG_MSM_CAMERA

static uint16_t msm_cam_gpio_2d_tbl[] = {
	5, /*CAMIF_MCLK*/
	20, /*CAMIF_I2C_DATA*/
	21, /*CAMIF_I2C_CLK*/
};

static struct msm_camera_gpio_conf gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_tbl = msm_cam_gpio_2d_tbl,
	.cam_gpio_tbl_size = ARRAY_SIZE(msm_cam_gpio_2d_tbl),
};

#ifdef CONFIG_MT9M113
static uint16_t msm_cam_gpio_2d_tbl2[] = {
	4, /*CAM2_MCLK*/
	12, /*I2C2_SDA*/
	13, /*I2C2_SCL*/
};

static struct msm_camera_gpio_conf gpio_conf2 = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs2,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs2),
	.cam_gpio_tbl = msm_cam_gpio_2d_tbl2,
	.cam_gpio_tbl_size = ARRAY_SIZE(msm_cam_gpio_2d_tbl2),
};
#endif /* CONFIG_MT9M113 */
#ifdef CONFIG_MT9D115_SUB
static uint16_t msm_cam_gpio_2d_tbl3[] = {
	4, /*CAM2_MCLK*/
	20, /*CAM_I2C_SDA*/
	21, /*CAM_I2C_SCL*/
};

static struct msm_camera_gpio_conf gpio_conf3 = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_tbl = msm_cam_gpio_2d_tbl3,
	.cam_gpio_tbl_size = ARRAY_SIZE(msm_cam_gpio_2d_tbl3),
};
#endif /* CONFIG_MT9D115_SUB */

#define VFE_CAMIF_TIMER1_GPIO 2
#define VFE_CAMIF_TIMER2_GPIO 3
#define VFE_CAMIF_TIMER3_GPIO_INT 4
struct msm_camera_sensor_strobe_flash_data strobe_flash_xenon = {
	.flash_trigger = VFE_CAMIF_TIMER2_GPIO,
	.flash_charge = VFE_CAMIF_TIMER1_GPIO,
	.flash_charge_done = VFE_CAMIF_TIMER3_GPIO_INT,
	.flash_recharge_duration = 50000,
	.irq = MSM_GPIO_TO_INT(VFE_CAMIF_TIMER3_GPIO_INT),
};
#ifdef CONFIG_MSM_CAMERA_FLASH
static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_EXT,
	._fsrc.ext_driver_src.led_en = GPIO_CAM_GP_LED_EN1,
	._fsrc.ext_driver_src.led_flash_en = GPIO_CAM_GP_LED_EN2,
#ifdef CONFIG_IMX074
#if defined(CONFIG_I2C) && (defined(CONFIG_GPIO_SX150X) || \
			defined(CONFIG_GPIO_SX150X_MODULE))
	._fsrc.ext_driver_src.expander_info = cam_expander_info,
#endif
#endif /* CONFIG_IMX074 */
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
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 140451840,
		.ib  = 561807360,
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

struct msm_camera_device_platform_data msm_camera_csi_device_data[] = {
	{
/*		.ioclk.mclk_clk_rate = 30000000, */
		.ioclk.mclk_clk_rate = 25600000, /* CHG 25.6MHz */
		.ioclk.vfe_clk_rate  = 228570000,
		.csid_core = 0,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
//		.ioclk.mclk_clk_rate = 24000000,
		.ioclk.mclk_clk_rate = 25600000, //[25.6MHz]
		.ioclk.vfe_clk_rate  = 228570000,
		.csid_core = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
};

#ifdef CONFIG_IMX074_ACT
static struct i2c_board_info imx074_actuator_i2c_info = {
	I2C_BOARD_INFO("imx074_act", 0x11),
};

static struct msm_actuator_info imx074_actuator_info = {
	.board_info     = &imx074_actuator_i2c_info,
	.bus_id         = MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = 0,
	.vcm_enable     = 1,
};
#endif

#ifdef CONFIG_IMX074
static struct msm_camera_sensor_flash_data flash_imx074 = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
#ifdef CONFIG_MSM_CAMERA_FLASH
	.flash_src	= &msm_flash_src
#endif
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx074 = {
	.mount_angle	= 90,
	.sensor_reset	= 107,
	.sensor_pwd	= 85,
	.vcm_pwd	= 0,
	.vcm_enable	= 1,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx074_data = {
	.sensor_name	= "imx074",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_imx074,
	.strobe_flash_data = &strobe_flash_xenon,
	.sensor_platform_info = &sensor_board_info_imx074,
	.gpio_conf = &gpio_conf,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
#ifdef CONFIG_IMX074_ACT
	.actuator_info = &imx074_actuator_info
#endif
};

struct platform_device msm8960_camera_sensor_imx074 = {
	.name	= "msm_camera_imx074",
	.dev	= {
		.platform_data = &msm_camera_sensor_imx074_data,
	},
};
#endif
#ifdef CONFIG_OV2720
static struct msm_camera_sensor_flash_data flash_ov2720 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov2720 = {
	.mount_angle	= 0,
	.sensor_reset	= 76,
	.sensor_pwd	= 85,
	.vcm_pwd	= 0,
	.vcm_enable	= 1,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov2720_data = {
	.sensor_name	= "ov2720",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_ov2720,
	.sensor_platform_info = &sensor_board_info_ov2720,
	.gpio_conf = &gpio_conf,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};

struct platform_device msm8960_camera_sensor_ov2720 = {
	.name	= "msm_camera_ov2720",
	.dev	= {
		.platform_data = &msm_camera_sensor_ov2720_data,
	},
};
#endif

/* MAINCAMERA CE150X START */
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
    .sensor_pwd = 3,
    .vcm_pwd    = 0,
    .vcm_enable = 1,
};

static struct msm_camera_sensor_info msm_camera_sensor_ce150x_data = {
    .sensor_name    = "ce150x",
    .pdata  = &msm_camera_csi_device_data[0],
    .flash_data = &flash_ce150x,
    .strobe_flash_data = &strobe_flash_xenon,
    .sensor_platform_info = &sensor_board_info_ce150x,
    .gpio_conf = &gpio_conf,
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

#ifdef CONFIG_MT9M113
static struct msm_camera_sensor_flash_data flash_mt9m113 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_mt9m113 = {
	.mount_angle	= 270,
	.sensor_reset	= 43,
	.sensor_pwd	= 6,
	.vcm_pwd	= 0,
	.vcm_enable	= 1,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9m113_data = {
	.sensor_name	= "mt9m113",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_mt9m113,
	.strobe_flash_data = &strobe_flash_xenon,
	.sensor_platform_info = &sensor_board_info_mt9m113,
	.gpio_conf = &gpio_conf2,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};

struct platform_device msm8960_camera_sensor_mt9m113 = {
	.name	= "msm_camera_mt9m113",
	.dev	= {
		.platform_data = &msm_camera_sensor_mt9m113_data,
	},
};
#endif /* CONFIG_MT9M113 */
#ifdef CONFIG_MT9D115_SUB
static struct msm_camera_sensor_flash_data flash_mt9d115_sub = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
};

static struct msm_camera_sensor_platform_info sensor_board_info_mt9d115_sub = {
	.mount_angle	= 270,
	.sensor_reset	= 43,
	.sensor_pwd	= 6,
	.vcm_pwd	= 0,
	.vcm_enable	= 1,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9d115_sub_data = {
	.sensor_name	= "mt9d115_sub",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_mt9d115_sub,
	.sensor_platform_info = &sensor_board_info_mt9d115_sub,
	.gpio_conf = &gpio_conf3,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};

struct platform_device msm8960_camera_sensor_mt9d115_sub = {
	.name	= "msm_camera_mt9d115_sub",
	.dev	= {
		.platform_data = &msm_camera_sensor_mt9d115_sub_data,
	},
};
#endif /* CONFIG_MT9D115_SUB */

static struct msm8960_privacy_light_cfg privacy_light_info = {
	.mpp = PM8921_MPP_PM_TO_SYS(12),
};

static struct msm_camera_sensor_flash_data flash_imx111 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
#ifdef CONFIG_MSM_CAMERA_FLASH
	.flash_src	= &msm_flash_src
#endif
};

static struct i2c_board_info imx111_actuator_i2c_info = {
	I2C_BOARD_INFO("imx111_act", 0x18),
};

static struct msm_actuator_info imx111_actuator_info = {
	.board_info     = &imx111_actuator_i2c_info,
	.bus_id         = MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = 0,
	.vcm_enable     = 1,
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx111 = {
	.mount_angle	= 90,
	.sensor_reset	= 107,
	.sensor_pwd	= 85,
	.vcm_pwd	= 0,
	.vcm_enable	= 1,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx111_data = {
	.sensor_name	= "imx111",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_imx111,
	.sensor_platform_info = &sensor_board_info_imx111,
	.gpio_conf = &gpio_conf,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
#ifdef CONFIG_IMX111_ACT
	.actuator_info	= &imx111_actuator_info,
#endif
};

struct platform_device msm8960_camera_sensor_imx111 = {
	.name	= "msm_camera_imx111",
	.dev	= {
		.platform_data = &msm_camera_sensor_imx111_data,
	},
};

static void __init msm8960_init_cam(void)
{
	int i;
	struct platform_device *cam_dev[] = {
#ifdef CONFIG_CE150X
		&msm8960_camera_sensor_ce150x,
#endif /* CONFIG_CE150X */
#ifdef CONFIG_MT9M113
		&msm8960_camera_sensor_mt9m113,
#endif /* CONFIG_MT9M113 */
#ifdef CONFIG_MT9D115_SUB
		&msm8960_camera_sensor_mt9d115_sub,
#endif /* CONFIG_MT9D115_SUB */
#ifdef CONFIG_IMX074
		&msm8960_camera_sensor_imx074,
#endif
#ifdef CONFIG_OV2720
		&msm8960_camera_sensor_ov2720,
#endif
#ifdef CONFIG_IMX111
		&msm8960_camera_sensor_imx111,
#endif /* CONFIG_IMX111 */
	};

	if (machine_is_msm8960_liquid()) {
		struct msm_camera_sensor_info *s_info;
#ifdef CONFIG_CE150X
		s_info = msm8960_camera_sensor_ce150x.dev.platform_data;
#endif /* CONFIG_CE150X */
#ifdef CONFIG_IMX074
		s_info = msm8960_camera_sensor_imx074.dev.platform_data;
#endif
		s_info->sensor_platform_info->mount_angle = 180;
#ifdef CONFIG_OV2720
		s_info = msm8960_camera_sensor_ov2720.dev.platform_data;
#endif
#ifdef CONFIG_IMX111
		s_info = msm8960_camera_sensor_imx111.dev.platform_data;
#endif /* CONFIG_IMX111 */
		s_info->sensor_platform_info->privacy_light = 1;
		s_info->sensor_platform_info->privacy_light_info =
			&privacy_light_info;
	}

	for (i = 0; i < ARRAY_SIZE(cam_dev); i++) {
		struct msm_camera_sensor_info *s_info;
		s_info = cam_dev[i]->dev.platform_data;
		msm_get_cam_resources(s_info);
		platform_device_register(cam_dev[i]);
	}

	platform_device_register(&msm8960_device_csiphy0);
	platform_device_register(&msm8960_device_csiphy1);
	platform_device_register(&msm8960_device_csid0);
	platform_device_register(&msm8960_device_csid1);
	platform_device_register(&msm8960_device_ispif);
	platform_device_register(&msm8960_device_vfe);
	platform_device_register(&msm8960_device_vpe);
}
#endif

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
	#ifdef CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB
		/* prim = 1280 x 720 x 4(RGBA8888) x 3(pages) */
		#define MSM_FB_PRIM_BUF_SIZE 0xA8C000 /*(1280*ALIGN(720, 32)*4*3)*/
	#else
/* prim = 608 x 1024 x 4(bpp) x 3(pages) */
#define MSM_FB_PRIM_BUF_SIZE 0x720000
	#endif /* CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
#else
	#ifdef CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB
		/* prim = 1280 x 720 x 4(RGBA8888) x 2(pages) */
		#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(720, 32)*4*2)
#else
/* prim = 608 x 1024 x 4(bpp) x 2(pages) */
#define MSM_FB_PRIM_BUF_SIZE 0x4C0000
	#endif /* CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
#endif

#ifdef CONFIG_FB_MSM_MIPI_DSI
//#define MIPI_DSI_WRITEBACK_SIZE (1024 * 600 * 3 * 2)
#define MIPI_DSI_WRITEBACK_SIZE (1280 * 720 * 3 * 2)
#else
#define MIPI_DSI_WRITEBACK_SIZE 0
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
#define MSM_FB_EXT_BUF_SIZE	(1920 * 1088 * 2 * 1) /* 2 bpp x 1 page */
#elif defined(CONFIG_FB_MSM_TVOUT)
#define MSM_FB_EXT_BUF_SIZE (720 * 576 * 2 * 2) /* 2 bpp x 2 pages */
#else
#define MSM_FB_EXT_BUF_SIZE	0
#endif

#ifdef CONFIG_FB_MSM_OVERLAY_WRITEBACK
/* width x height x 3 bpp x 2 frame buffer */
#define MSM_FB_WRITEBACK_SIZE (1376 * 768 * 3 * 2)
#define MSM_FB_WRITEBACK_OFFSET  \
		(MSM_FB_PRIM_BUF_SIZE + MSM_FB_EXT_BUF_SIZE)
#else
#define MSM_FB_WRITEBACK_SIZE   0
#define MSM_FB_WRITEBACK_OFFSET 0
#endif

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
/* 4 bpp x 2 page HDMI case */
#define MSM_FB_SIZE roundup((1920 * 1088 * 4 * 2), 4096)
#else
/* Note: must be multiple of 4096 */
#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE + MSM_FB_EXT_BUF_SIZE + \
				MSM_FB_WRITEBACK_SIZE, 4096)
#endif

static int writeback_offset(void)
{
	return MSM_FB_WRITEBACK_OFFSET;
}


#define MDP_VSYNC_GPIO 0

#define PANEL_NAME_MAX_LEN	30
#define MIPI_CMD_NOVATEK_QHD_PANEL_NAME	"mipi_cmd_novatek_qhd"
#define MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME	"mipi_video_novatek_qhd"
#define MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME	"mipi_video_toshiba_wsvga"
#define MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME	"mipi_video_chimei_wxga"
#define MIPI_VIDEO_SIMULATOR_VGA_PANEL_NAME	"mipi_video_simulator_vga"
#define MIPI_CMD_RENESAS_FWVGA_PANEL_NAME	"mipi_cmd_renesas_fwvga"
#define HDMI_PANEL_NAME	"hdmi_msm"
#define TVOUT_PANEL_NAME	"tvout_msm"

static struct resource msm_fb_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	if (machine_is_msm8960_liquid()) {
		if (!strncmp(name, MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;
	} else {
		if (!strncmp(name, MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;

#ifndef CONFIG_FB_MSM_MIPI_PANEL_DETECT
		if (!strncmp(name, MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME,
				strnlen(MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;

		if (!strncmp(name, MIPI_CMD_NOVATEK_QHD_PANEL_NAME,
				strnlen(MIPI_CMD_NOVATEK_QHD_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;

		if (!strncmp(name, MIPI_VIDEO_SIMULATOR_VGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_SIMULATOR_VGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;

		if (!strncmp(name, MIPI_CMD_RENESAS_FWVGA_PANEL_NAME,
				strnlen(MIPI_CMD_RENESAS_FWVGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;
#endif
	}

	if (!strncmp(name, HDMI_PANEL_NAME,
			strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	if (!strncmp(name, TVOUT_PANEL_NAME,
			strnlen(TVOUT_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	pr_warning("%s: not supported '%s'", __func__, name);
	return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources     = ARRAY_SIZE(msm_fb_resources),
	.resource          = msm_fb_resources,
	.dev.platform_data = &msm_fb_pdata,
};

#ifdef CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB
struct lcd_gpio_info {
	unsigned no;
	char *name;
};

static struct gpiomux_setting mddi_renesas_hd_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting mddi_renesas_hd_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config mddi_renesas_hd_interface[] __initdata = {
	{
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mddi_renesas_hd_active_cfg,
			[GPIOMUX_SUSPENDED] = &mddi_renesas_hd_suspend_cfg,
		},
	},
};

static int mipi_dsi_panel_power(int on)
{
        /*static struct regulator *vreg_l0a   = NULL;*/
    static struct regulator *vreg_lvs5 = NULL;
    static struct regulator *vreg_l8   = NULL;
    int rc;
    static struct regulator *reg_l2    = NULL;
    static int gpio36;
    struct pm_gpio gpio36_param = {
        .direction = PM_GPIO_DIR_OUT,
        .output_buffer = PM_GPIO_OUT_BUF_CMOS,
        .output_value = 1,
        .pull = PM_GPIO_PULL_NO,
        .vin_sel = PM_GPIO_VIN_S4,
        .out_strength = PM_GPIO_STRENGTH_HIGH,
        .function = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol = 0,
        .disable_pin = 0,
    };
    static const struct lcd_gpio_info lcd_5v_oe = {78, "lcd_5v_oe"};

    printk(KERN_DEBUG "[In]%s(%d).\n", __func__, on);

    if (reg_l2 == NULL) {
		/* VREG_L2 */
		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_vdda");
		if (IS_ERR(reg_l2)) {
	    	pr_debug("%s: VREG_L2 failed\n", __func__);
		    rc = PTR_ERR(reg_l2);
		    return rc;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			goto out;
		}

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			goto out;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			goto out;
		}
    }

    if (vreg_l8 == NULL) {
	/* VREG_L8 */
	vreg_l8 = regulator_get(NULL, "8921_l8");
	if (IS_ERR(vreg_l8)) {
	    pr_debug("%s: VREG_L8 failed\n", __func__);
	    rc = PTR_ERR(vreg_l8);
	    return rc;
	}

	rc = regulator_set_voltage(vreg_l8, 2800000, 2800000);
	if (rc) {
	    printk("[reiji]vreg_l8 set voltage failed.\n");
	    goto out;
	}

	rc = regulator_enable(vreg_l8);
	if (rc) {
	    printk("[reiji]vreg_l8 enable failed\n");
	    goto out;
	}
    }

    if (vreg_lvs5 == NULL) {
	/* VREG_LVS5 */
	vreg_lvs5 = regulator_get(NULL, "8921_lvs5");
	if (IS_ERR(vreg_lvs5)) {
	    pr_debug("%s: VREG_LVS5 failed\n", __func__);
	    rc = PTR_ERR(vreg_lvs5);
	    return rc;
	}

	rc = regulator_enable(vreg_lvs5);
	if (rc) {
	    printk("[reiji]vreg_lvs5 enable failed\n");
	    goto out;
	}

	msleep(11);

	gpio36 = PM8921_GPIO_PM_TO_SYS(36);
	rc = gpio_request(gpio36, "disp_rst_n");
	if (rc)
	  {
	      pr_err("request gpio 43 failed, rc=%d\n", rc);
	      return -ENODEV;
	  }

	rc = pm8xxx_gpio_config(gpio36, &gpio36_param);
	if (rc)
	  {
	      pr_err("gpio_config 36 failed (1), rc=%d\n", rc);
	      return -EINVAL;
	  }

	rc = gpio_direction_output(gpio36, 1);
	if (rc) {
	    printk("[reiji]gpio36 output failed.\n");
	}
	msleep(11);

	rc = gpio_request(lcd_5v_oe.no, lcd_5v_oe.name);
	if (rc) {
	    pr_err("%s: gpio_request(%s.%d) failed\n",
		   __func__, lcd_5v_oe.name, lcd_5v_oe.no);
	    //goto out;
	}
	rc = gpio_direction_output(lcd_5v_oe.no, 1);
	if (rc) {
	    pr_err("%s: gpio_direction_output(%s.%d) failed\n",
		   __func__, lcd_5v_oe.name, lcd_5v_oe.no);
	    //goto out;
	}
	msleep(11);
    }


    if (on) {
	gpio_set_value(lcd_5v_oe.no, 1);
	msleep(11);
    } else {
	gpio_set_value(lcd_5v_oe.no, 0);
	msleep(11);
    }

    printk(KERN_DEBUG "[Out]%s.\n", __func__);
    return 0;

out:
    if (reg_l2) {
        if (regulator_is_enabled(reg_l2)) {
            regulator_disable(reg_l2);
        }
        regulator_put(reg_l2);
    }

    if (vreg_lvs5) {
        if (regulator_is_enabled(vreg_lvs5)) {
            regulator_disable(vreg_lvs5);
        }
        regulator_put(vreg_lvs5);
    }

    if (vreg_l8) {
        if (regulator_is_enabled(vreg_l8)) {
            regulator_disable(vreg_l8);
        }
        regulator_put(vreg_l8);
    }

    /*vreg_l0a = NULL;*/
    reg_l2 = NULL;
    vreg_lvs5 = NULL;
    vreg_l8 = NULL;
    return rc;
}

#else
/**
 * LiQUID panel on/off
 *
 * @param on
 *
 * @return int
 */
static int mipi_dsi_liquid_panel_power(int on)
{
	static struct regulator *reg_l2, *reg_ext_3p3v;
	static int gpio21, gpio24, gpio43;
	int rc;

	pr_info("%s: on=%d\n", __func__, on);

	gpio21 = PM8921_GPIO_PM_TO_SYS(21); /* disp power enable_n */
	gpio43 = PM8921_GPIO_PM_TO_SYS(43); /* Displays Enable (rst_n)*/
	gpio24 = PM8921_GPIO_PM_TO_SYS(24); /* Backlight PWM */

	if (!dsi_power_on) {

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_vdda");
		if (IS_ERR(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		reg_ext_3p3v = regulator_get(&msm_mipi_dsi1_device.dev,
			"vdd_lvds_3p3v");
		if (IS_ERR(reg_ext_3p3v)) {
			pr_err("could not get reg_ext_3p3v, rc = %ld\n",
			       PTR_ERR(reg_ext_3p3v));
		    return -ENODEV;
		}

		rc = gpio_request(gpio21, "disp_pwr_en_n");
		if (rc) {
			pr_err("request gpio 21 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = gpio_request(gpio43, "disp_rst_n");
		if (rc) {
			pr_err("request gpio 43 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = gpio_request(gpio24, "disp_backlight_pwm");
		if (rc) {
			pr_err("request gpio 24 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		dsi_power_on = true;
	}

	if (on) {
		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_enable(reg_ext_3p3v);
		if (rc) {
			pr_err("enable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}

		/* set reset pin before power enable */
		gpio_set_value_cansleep(gpio43, 0); /* disp disable (resx=0) */

		gpio_set_value_cansleep(gpio21, 0); /* disp power enable_n */
		msleep(20);
		gpio_set_value_cansleep(gpio43, 1); /* disp enable */
		msleep(20);
		gpio_set_value_cansleep(gpio43, 0); /* disp enable */
		msleep(20);
		gpio_set_value_cansleep(gpio43, 1); /* disp enable */
		msleep(20);
	} else {
		gpio_set_value_cansleep(gpio43, 0);
		gpio_set_value_cansleep(gpio21, 1);

		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_ext_3p3v);
		if (rc) {
			pr_err("disable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_set_optimum_mode(reg_l2, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
	}

	return 0;
}

static int mipi_dsi_cdp_panel_power(int on)
{
	static struct regulator *reg_l8, *reg_l23, *reg_l2;
	static int gpio43;
	int rc;

	pr_info("%s: state : %d\n", __func__, on);

	if (!dsi_power_on) {

		reg_l8 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_vdc");
		if (IS_ERR(reg_l8)) {
			pr_err("could not get 8921_l8, rc = %ld\n",
				PTR_ERR(reg_l8));
			return -ENODEV;
		}
		reg_l23 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_vddio");
		if (IS_ERR(reg_l23)) {
			pr_err("could not get 8921_l23, rc = %ld\n",
				PTR_ERR(reg_l23));
			return -ENODEV;
		}
		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_vdda");
		if (IS_ERR(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_l8, 2800000, 3000000);
		if (rc) {
			pr_err("set_voltage l8 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_voltage(reg_l23, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		gpio43 = PM8921_GPIO_PM_TO_SYS(43);
		rc = gpio_request(gpio43, "disp_rst_n");
		if (rc) {
			pr_err("request gpio 43 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		dsi_power_on = true;
	}
	if (on) {
		rc = regulator_set_optimum_mode(reg_l8, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l8 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_optimum_mode(reg_l23, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l8);
		if (rc) {
			pr_err("enable l8 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_enable(reg_l23);
		if (rc) {
			pr_err("enable l8 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		gpio_set_value_cansleep(gpio43, 1);
	} else {
		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_l8);
		if (rc) {
			pr_err("disable reg_l8 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_l23);
		if (rc) {
			pr_err("disable reg_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_set_optimum_mode(reg_l8, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l8 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_optimum_mode(reg_l23, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_optimum_mode(reg_l2, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		gpio_set_value_cansleep(gpio43, 0);
	}
	return 0;
}

static int mipi_dsi_panel_power(int on)
{
	int ret;

	pr_info("%s: on=%d\n", __func__, on);

	if (machine_is_msm8960_liquid())
		ret = mipi_dsi_liquid_panel_power(on);
	else
		ret = mipi_dsi_cdp_panel_power(on);

	return ret;
}
#endif /* CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */

static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio = MDP_VSYNC_GPIO,
	.dsi_power_save = mipi_dsi_panel_power,
};

#ifdef CONFIG_MSM_BUS_SCALING

static struct msm_bus_vectors mdp_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
static struct msm_bus_vectors hdmi_as_primary_vectors[] = {
	/* If HDMI is used as primary */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 2000000000,
		.ib = 2000000000,
	},
};
static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
};
#else
static struct msm_bus_vectors mdp_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_vga_vectors[] = {
	/* VGA and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_720p_vectors[] = {
	/* 720p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 230400000 * 2,
		.ib = 288000000 * 2,
	},
};

static struct msm_bus_vectors mdp_1080p_vectors[] = {
	/* 1080p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 334080000 * 2,
		.ib = 417600000 * 2,
	},
};

static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_vga_vectors),
		mdp_vga_vectors,
	},
	{
		ARRAY_SIZE(mdp_720p_vectors),
		mdp_720p_vectors,
	},
	{
		ARRAY_SIZE(mdp_1080p_vectors),
		mdp_1080p_vectors,
	},
};
#endif

static struct msm_bus_scale_pdata mdp_bus_scale_pdata = {
	mdp_bus_scale_usecases,
	ARRAY_SIZE(mdp_bus_scale_usecases),
	.name = "mdp",
};

#endif

#ifdef CONFIG_FB_MSM_MIPI_DSI
int mdp_core_clk_rate_table[] = {
#ifdef CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB
	128000000,
	160000000,
	177780000,
	200000000,
#else /* !CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
	200000000,
	200000000,
	200000000,
	200000000,
#else
	85330000,
	85330000,
	160000000,
	200000000,
#endif
#endif /* !CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
};
#else
#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
int mdp_core_clk_rate_table[] = {
	200000000,
	200000000,
	200000000,
	200000000,
};
#else
int mdp_core_clk_rate_table[] = {
	85330000,
	85330000,
	128000000,
	200000000,
	200000000,
};
#endif
#endif

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_VSYNC_GPIO,
#ifdef CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB
	.mdp_core_clk_rate = 128000000,
#else /* !CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
	.mdp_core_clk_rate = 200000000,
#else
	.mdp_core_clk_rate = 85330000,
#endif
#endif /* !CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
	.mdp_core_clk_table = mdp_core_clk_rate_table,
	.num_mdp_clk = ARRAY_SIZE(mdp_core_clk_rate_table),
#ifdef CONFIG_MSM_BUS_SCALING
	.mdp_bus_scale_table = &mdp_bus_scale_pdata,
#endif
	.mdp_rev = MDP_REV_42,
	.writeback_offset = writeback_offset,
};

static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas_hd",
	.id = 0,
};

static struct platform_device mipi_dsi_simulator_panel_device = {
	.name = "mipi_simulator",
	.id = 0,
};


#define FPGA_3D_GPIO_CONFIG_ADDR	0xB5

static struct mipi_dsi_phy_ctrl dsi_novatek_cmd_mode_phy_db = {

/* DSI_BIT_CLK at 500MHz, 2 lane, RGB888 */
	{0x0F, 0x0a, 0x04, 0x00, 0x20},	/* regulator */
	/* timing   */
	{0xab, 0x8a, 0x18, 0x00, 0x92, 0x97, 0x1b, 0x8c,
	0x0c, 0x03, 0x04, 0xa0},
	{0x5f, 0x00, 0x00, 0x10},	/* phy ctrl */
	{0xff, 0x00, 0x06, 0x00},	/* strength */
	/* pll control */
	{0x40, 0xf9, 0x30, 0xda, 0x00, 0x40, 0x03, 0x62,
	0x40, 0x07, 0x03,
	0x00, 0x1a, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01},
};

static struct mipi_dsi_panel_platform_data novatek_pdata = {
	.fpga_3d_config_addr  = FPGA_3D_GPIO_CONFIG_ADDR,
	.fpga_ctrl_mode = FPGA_SPI_INTF,
	.phy_ctrl_settings = &dsi_novatek_cmd_mode_phy_db,
};

static struct platform_device mipi_dsi_novatek_panel_device = {
	.name = "mipi_novatek",
	.id = 0,
	.dev = {
		.platform_data = &novatek_pdata,
	}
};


#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
static struct resource hdmi_msm_resources[] = {
	{
		.name  = "hdmi_msm_qfprom_addr",
		.start = 0x00700000,
		.end   = 0x007060FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_hdmi_addr",
		.start = 0x04A00000,
		.end   = 0x04A00FFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_irq",
		.start = HDMI_IRQ,
		.end   = HDMI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static int hdmi_enable_5v(int on);
static int hdmi_core_power(int on, int show);
static int hdmi_cec_power(int on);

static struct msm_hdmi_platform_data hdmi_msm_data = {
	.irq = HDMI_IRQ,
	.enable_5v = hdmi_enable_5v,
	.core_power = hdmi_core_power,
	.cec_power = hdmi_cec_power,
};

static struct platform_device hdmi_msm_device = {
	.name = "hdmi_msm",
	.id = 0,
	.num_resources = ARRAY_SIZE(hdmi_msm_resources),
	.resource = hdmi_msm_resources,
	.dev.platform_data = &hdmi_msm_data,
};
#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
static struct platform_device wfd_panel_device = {
	.name = "wfd_panel",
	.id = 0,
	.dev.platform_data = NULL,
};
#endif

static struct apds990x_platform_data apds990x_data = {
	.intr = 49,
};
static struct akm8977_platform_data compass_platform_data = {
	.reset = 50,			//reset.
	.intr =  53,			// int1.
};

static struct l3g4200d_platform_data gyro_platform_data = {
	.data = 0
};

static struct i2c_board_info msm_i2c_sensor_board_info[] = {
	{
		/* prox Sensor */
		I2C_BOARD_INFO("apds9900", 0x39),
		.platform_data	 = &apds990x_data,
		.irq = MSM_GPIO_TO_INT(49),
	},
	{
		/* 6axis Sensor */
		I2C_BOARD_INFO("akm8977", 0x1C),
		.platform_data = &compass_platform_data,
		.irq = MSM_GPIO_TO_INT(53),
	},
	{
		I2C_BOARD_INFO("l3g4200d", 0x68),
		.platform_data	 = &gyro_platform_data,
	}
};


static struct i2c_board_info felica_i2c_info[] = {
	{
		I2C_BOARD_INFO("AK6921AF", 0x57),
	},
};

	

static void __init msm_fb_add_devices(void)
{

	if (machine_is_msm8x60_rumi3()) {
		msm_fb_register_device("mdp", NULL);
		mipi_dsi_pdata.target_type = 1;
	} else
		msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
}

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

static struct msm_gpiomux_config msm8960_mdp_vsync_configs[] __initdata = {
	{
		.gpio = MDP_VSYNC_GPIO,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mdp_vsync_active_cfg,
			[GPIOMUX_SUSPENDED] = &mdp_vsync_suspend_cfg,
		},
	}
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
};

static int hdmi_enable_5v(int on)
{
	/* TBD: PM8921 regulator instead of 8901 */
	static struct regulator *reg_8921_hdmi_mvs;	/* HDMI_5V */
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (!reg_8921_hdmi_mvs)
		reg_8921_hdmi_mvs = regulator_get(&hdmi_msm_device.dev,
			"hdmi_mvs");

	if (on) {
		rc = regulator_enable(reg_8921_hdmi_mvs);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"8921_hdmi_mvs", rc);
			return rc;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_8921_hdmi_mvs);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"8921_hdmi_mvs", rc);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
}

static int hdmi_core_power(int on, int show)
{
	static struct regulator *reg_8921_l23, *reg_8921_s4;
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	/* TBD: PM8921 regulator instead of 8901 */
	if (!reg_8921_l23) {
		reg_8921_l23 = regulator_get(&hdmi_msm_device.dev, "hdmi_avdd");
		if (IS_ERR(reg_8921_l23)) {
			pr_err("could not get reg_8921_l23, rc = %ld\n",
				PTR_ERR(reg_8921_l23));
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_l23, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage failed for 8921_l23, rc=%d\n", rc);
			return -EINVAL;
		}
	}
	if (!reg_8921_s4) {
		reg_8921_s4 = regulator_get(&hdmi_msm_device.dev, "hdmi_vcc");
		if (IS_ERR(reg_8921_s4)) {
			pr_err("could not get reg_8921_s4, rc = %ld\n",
				PTR_ERR(reg_8921_s4));
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_s4, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage failed for 8921_s4, rc=%d\n", rc);
			return -EINVAL;
		}
	}

	if (on) {
		rc = regulator_set_optimum_mode(reg_8921_l23, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_8921_l23);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"hdmi_avdd", rc);
			return rc;
		}
		rc = regulator_enable(reg_8921_s4);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"hdmi_vcc", rc);
			return rc;
		}
		rc = gpio_request(100, "HDMI_DDC_CLK");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_CLK", 100, rc);
			goto error1;
		}
		rc = gpio_request(101, "HDMI_DDC_DATA");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_DATA", 101, rc);
			goto error2;
		}
		rc = gpio_request(102, "HDMI_HPD");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_HPD", 102, rc);
			goto error3;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(100);
		gpio_free(101);
		gpio_free(102);

		rc = regulator_disable(reg_8921_l23);
		if (rc) {
			pr_err("disable reg_8921_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_8921_s4);
		if (rc) {
			pr_err("disable reg_8921_s4 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_set_optimum_mode(reg_8921_l23, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;

error3:
	gpio_free(101);
error2:
	gpio_free(100);
error1:
	regulator_disable(reg_8921_l23);
	regulator_disable(reg_8921_s4);
	return rc;
}

static int hdmi_cec_power(int on)
{
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(99, "HDMI_CEC_VAR");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_CEC_VAR", 99, rc);
			goto error;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(99);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
error:
	return rc;
}
#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
static struct gpiomux_setting synaptics_cs_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_16MA,
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
		.gpio = 53,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensor_akm_int_active_cfg,
			[GPIOMUX_SUSPENDED] = &sensor_akm_int_suspended_cfg,
		},
	},
};


static void __init msm8960_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
			size, addr, __pa(addr));

}
#ifdef CONFIG_WCD9310_CODEC

#define TABLA_INTERRUPT_BASE (NR_MSM_IRQS + NR_GPIO_IRQS + NR_PM8921_IRQS)

/* Micbias setting is based on 8660 CDP/MTP/FLUID requirement
 * 4 micbiases are used to power various analog and digital
 * microphones operating at 1800 mV. Technically, all micbiases
 * can source from single cfilter since all microphones operate
 * at the same voltage level. The arrangement below is to make
 * sure all cfilters are exercised. LDO_H regulator ouput level
 * does not need to be as high as 2.85V. It is choosen for
 * microphone sensitivity purpose.
 */
static struct tabla_pdata tabla_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x10, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(62),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_TABLA_IRQS,
#ifdef CONFIG_FEATURE_NCMC_AUDIO
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(7),
#else/* CONFIG_FEATURE_NCMC_AUDIO */
#endif/* CONFIG_FEATURE_NCMC_AUDIO */
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 1800,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	}
};

static struct slim_device msm_slim_tabla = {
	.name = "tabla-slim",
	.e_addr = {0, 1, 0x10, 0, 0x17, 2},
	.dev = {
		.platform_data = &tabla_platform_data,
	},
};

static struct tabla_pdata tabla20_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x60, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(62),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_TABLA_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 1800,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	}
};

static struct slim_device msm_slim_tabla20 = {
	.name = "tabla2x-slim",
	.e_addr = {0, 1, 0x60, 0, 0x17, 2},
	.dev = {
		.platform_data = &tabla20_platform_data,
	},
};
#endif

static struct slim_boardinfo msm_slim_devices[] = {
#ifdef CONFIG_WCD9310_CODEC
	{
		.bus_num = 1,
		.slim_slave = &msm_slim_tabla,
	},
	{
		.bus_num = 1,
		.slim_slave = &msm_slim_tabla20,
	},
#endif
	/* add more slimbus slaves as needed */
};

#define MSM_WCNSS_PHYS	0x03000000
#define MSM_WCNSS_SIZE	0x280000

static struct resource resources_wcnss_wlan[] = {
	{
		.start	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.end	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.name	= "wcnss_wlanrx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.end	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.name	= "wcnss_wlantx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_WCNSS_PHYS,
		.end	= MSM_WCNSS_PHYS + MSM_WCNSS_SIZE - 1,
		.name	= "wcnss_mmio",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 84,
		.end	= 88,
		.name	= "wcnss_gpios_5wire",
		.flags	= IORESOURCE_IO,
	},
};

static struct qcom_wcnss_opts qcom_wcnss_pdata = {
	.has_48mhz_xo	= 1,
};

static struct platform_device msm_device_wcnss_wlan = {
	.name		= "wcnss_wlan",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_wcnss_wlan),
	.resource	= resources_wcnss_wlan,
	.dev		= {.platform_data = &qcom_wcnss_pdata},
};

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

#define QCE_SIZE		0x10000
#define QCE_0_BASE		0x18500000

#define QCE_HW_KEY_SUPPORT	0
#define QCE_SHA_HMAC_SUPPORT	1
#define QCE_SHARE_CE_RESOURCE	1
#define QCE_CE_SHARED		0

static struct resource qcrypto_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource qcedev_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)

static struct msm_ce_hw_support qcrypto_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
};

static struct platform_device qcrypto_device = {
	.name		= "qcrypto",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcrypto_resources),
	.resource	= qcrypto_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcrypto_ce_hw_suppport,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

static struct msm_ce_hw_support qcedev_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
};

static struct platform_device qcedev_device = {
	.name		= "qce",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcedev_resources),
	.resource	= qcedev_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcedev_ce_hw_suppport,
	},
};
#endif

#define MDM2AP_ERRFATAL			70
#define AP2MDM_ERRFATAL			95
#define MDM2AP_STATUS			69
#define AP2MDM_STATUS			94
#define AP2MDM_PMIC_RESET_N		80
#define AP2MDM_KPDPWR_N			81

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

static struct msm_gpiomux_config mdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = AP2MDM_STATUS,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = MDM2AP_STATUS,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = MDM2AP_ERRFATAL,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = AP2MDM_ERRFATAL,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_KPDPWR_N */
	{
		.gpio = AP2MDM_KPDPWR_N,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_kpdpwr_n_cfg,
		}
	},
	/* AP2MDM_PMIC_RESET_N */
	{
		.gpio = AP2MDM_PMIC_RESET_N,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_kpdpwr_n_cfg,
		}
	}
};

static struct resource mdm_resources[] = {
	{
		.start	= MDM2AP_ERRFATAL,
		.end	= MDM2AP_ERRFATAL,
		.name	= "MDM2AP_ERRFATAL",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= AP2MDM_ERRFATAL,
		.end	= AP2MDM_ERRFATAL,
		.name	= "AP2MDM_ERRFATAL",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= MDM2AP_STATUS,
		.end	= MDM2AP_STATUS,
		.name	= "MDM2AP_STATUS",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= AP2MDM_STATUS,
		.end	= AP2MDM_STATUS,
		.name	= "AP2MDM_STATUS",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= AP2MDM_PMIC_RESET_N,
		.end	= AP2MDM_PMIC_RESET_N,
		.name	= "AP2MDM_PMIC_RESET_N",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= AP2MDM_KPDPWR_N,
		.end	= AP2MDM_KPDPWR_N,
		.name	= "AP2MDM_KPDPWR_N",
		.flags	= IORESOURCE_IO,
	},
};

static struct mdm_platform_data mdm_platform_data = {
	.mdm_version = "2.5",
};

struct platform_device mdm_device = {
	.name		= "mdm2_modem",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mdm_resources),
	.resource	= mdm_resources,
	.dev		= {
		.platform_data = &mdm_platform_data,
	},
};

static struct platform_device *mdm_devices[] __initdata = {
	&mdm_device,
};

static int __init gpiomux_init(void)
{
	int rc;

	rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return rc;
	}

	msm_gpiomux_install(msm8960_cam_common_configs,
			ARRAY_SIZE(msm8960_cam_common_configs));

	msm_gpiomux_install(msm8960_gpiomux_configs,
			ARRAY_SIZE(msm8960_gpiomux_configs));

	msm_gpiomux_install(msm8960_gsbi_configs,
			ARRAY_SIZE(msm8960_gsbi_configs));

	msm_gpiomux_install(msm8960_cyts_configs,
			ARRAY_SIZE(msm8960_cyts_configs));

	msm_gpiomux_install(msm8960_slimbus_config,
			ARRAY_SIZE(msm8960_slimbus_config));

	msm_gpiomux_install(msm8960_audio_codec_configs,
			ARRAY_SIZE(msm8960_audio_codec_configs));

	msm_gpiomux_install(msm8960_audio_auxpcm_configs,
			ARRAY_SIZE(msm8960_audio_auxpcm_configs));

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

	msm_gpiomux_install(wcnss_5wire_interface,
			ARRAY_SIZE(wcnss_5wire_interface));

	if (machine_is_msm8960_mtp() || machine_is_msm8960_fluid() ||
		machine_is_msm8960_liquid() || machine_is_msm8960_cdp())
		msm_gpiomux_install(hap_lvl_shft_config,
			ARRAY_SIZE(hap_lvl_shft_config));

	if (PLATFORM_IS_CHARM25())
		msm_gpiomux_install(mdm_configs,
			ARRAY_SIZE(mdm_configs));

	msm_gpiomux_install(mddi_renesas_hd_interface,
			ARRAY_SIZE(mddi_renesas_hd_interface));

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

	return 0;
}

#define MSM_SHARED_RAM_PHYS 0x80000000

#if !defined(CONFIG_FEATURE_NCMC_POWER)
static struct pm8921_adc_amux pm8921_adc_channels_data[] = {
	{"vcoin", CHANNEL_VCOIN, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vbat", CHANNEL_VBAT, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"dcin", CHANNEL_DCIN, CHAN_PATH_SCALING4, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ichg", CHANNEL_ICHG, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vph_pwr", CHANNEL_VPH_PWR, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ibat", CHANNEL_IBAT, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
#ifdef CONFIG_FEATURE_NCMC_POWER
	{"m4", CHANNEL_MPP_1, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"m5", CHANNEL_MPP_2, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
#endif
	{"batt_therm", CHANNEL_BATT_THERM, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BATT_THERM},
	{"batt_id", CHANNEL_BATT_ID, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"usbin", CHANNEL_USBIN, CHAN_PATH_SCALING3, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pmic_therm", CHANNEL_DIE_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PMIC_THERM},
	{"625mv", CHANNEL_625MV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"125v", CHANNEL_125V, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"chg_temp", CHANNEL_CHG_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pa_therm1", ADC_MPP_1_AMUX8, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PA_THERM},
	{"xo_therm", CHANNEL_MUXOFF, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_XOTHERM},
	{"pa_therm0", ADC_MPP_1_AMUX3, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PA_THERM},
#ifdef CONFIG_FEATURE_NCMC_POWER
    {"batt_therm_mv", CHANNEL_BATT_THERM_MV, CHAN_PATH_SCALING1, AMUX_RSV2,
        ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
    {"xo_therm_mv", CHANNEL_MUXOFF_MV, CHAN_PATH_SCALING1, AMUX_RSV0,
        ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
    {"pa_therm0_mv", CHANNEL_PA_THERM0_MV, CHAN_PATH_SCALING1, AMUX_RSV1,
        ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
#endif
};

static struct pm8921_adc_properties pm8921_adc_data = {
	.adc_vdd_reference	= 1800, /* milli-voltage for this adc */
	.bitresolution		= 15,
	.bipolar                = 0,
};

static struct pm8921_adc_platform_data pm8921_adc_pdata = {
	.adc_channel		= pm8921_adc_channels_data,
	.adc_num_board_channel	= ARRAY_SIZE(pm8921_adc_channels_data),
	.adc_prop		= &pm8921_adc_data,
	.adc_mpp_base		= PM8921_MPP_PM_TO_SYS(1),
};
#endif

static void __init msm8960_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_msm8960_io();

	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}

#ifdef CONFIG_ARCH_MSM8930
static void __init msm8930_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_msm8930_io();

	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}
#endif

static void __init msm8960_init_irq(void)
{
	unsigned int i;

	msm_mpm_irq_extn_init();
	gic_init(0, GIC_PPI_START, MSM_QGIC_DIST_BASE,
						(void *)MSM_QGIC_CPU_BASE);

	/* Edge trigger PPIs except AVS_SVICINT and AVS_SVICINTSWDONE */
	writel_relaxed(0xFFFFD7FF, MSM_QGIC_DIST_BASE + GIC_DIST_CONFIG + 4);

	writel_relaxed(0x0000FFFF, MSM_QGIC_DIST_BASE + GIC_DIST_ENABLE_SET);
	mb();

	/* FIXME: Not installing AVS_SVICINT and AVS_SVICINTSWDONE yet
	 * as they are configured as level, which does not play nice with
	 * handle_percpu_irq.
	 */
	for (i = GIC_PPI_START; i < GIC_SPI_START; i++) {
		if (i != AVS_SVICINT && i != AVS_SVICINTSWDONE)
			irq_set_handler(i, handle_percpu_irq);
	}
}

/* MSM8960 has 5 SDCC controllers */
enum sdcc_controllers {
	SDCC1,
	SDCC2,
	SDCC3,
	SDCC4,
	SDCC5,
	MAX_SDCC_CONTROLLER
};

/* All SDCC controllers require VDD/VCC voltage */
static struct msm_mmc_reg_data mmc_vdd_reg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.name = "sdc_vdd",
		.high_vol_level = 2950000,
		.low_vol_level = 2950000,
		.always_on = 1,
		.lpm_sup = 1,
		.lpm_uA = 9000,
		.hpm_uA = 200000, /* 200mA */
	},
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.name = "sdc_vdd",
		.high_vol_level = 2950000,
		.low_vol_level = 2950000,
		.hpm_uA = 600000, /* 600mA */
	}
};

/* Only slots having eMMC card will require VCCQ voltage */
static struct msm_mmc_reg_data mmc_vccq_reg_data[1] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.name = "sdc_vccq",
		.always_on = 1,
		.high_vol_level = 1800000,
		.low_vol_level = 1800000,
		.hpm_uA = 200000, /* 200mA */
	}
};

/* All SDCC controllers may require voting for VDD PAD voltage */
static struct msm_mmc_reg_data mmc_vddp_reg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.name = "sdc_vddp",
		.high_vol_level = 2950000,
		.low_vol_level = 1850000,
		.always_on = 1,
		.lpm_sup = 1,
		/* Max. Active current required is 16 mA */
		.hpm_uA = 16000,
		/*
		 * Sleep current required is ~300 uA. But min. vote can be
		 * in terms of mA (min. 1 mA). So let's vote for 2 mA
		 * during sleep.
		 */
		.lpm_uA = 2000,
	}
};

static struct msm_mmc_slot_reg_data mmc_slot_vreg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC1],
		.vccq_data = &mmc_vccq_reg_data[SDCC1],
	},
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC3],
		.vddp_data = &mmc_vddp_reg_data[SDCC3],
	}
};

/* SDC1 pad data */
static struct msm_mmc_pad_drv sdc1_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_16MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_10MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_10MA}
};

static struct msm_mmc_pad_drv sdc1_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_DOWN},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_DOWN}
};

/* SDC3 pad data */
static struct msm_mmc_pad_drv sdc3_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_8MA}
};

static struct msm_mmc_pad_drv sdc3_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
	/*
	 * SDC3 CMD line should be PULLed UP otherwise fluid platform will
	 * see transitions (1 -> 0 and 0 -> 1) on card detection line,
	 * which would result in false card detection interrupts.
	 */
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	/*
	 * Keeping DATA lines status to PULL UP will make sure that
	 * there is no current leak during sleep if external pull up
	 * is connected to DATA lines.
	 */
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
};

struct msm_mmc_pad_pull_data mmc_pad_pull_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_pull_on_cfg,
		.off = sdc1_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_pull_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_pull_on_cfg,
		.off = sdc3_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_pull_on_cfg)
	},
};

struct msm_mmc_pad_drv_data mmc_pad_drv_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_drv_on_cfg,
		.off = sdc1_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_drv_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_drv_on_cfg,
		.off = sdc3_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_drv_on_cfg)
	},
};

struct msm_mmc_pad_data mmc_pad_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pull = &mmc_pad_pull_data[SDCC1],
		.drv = &mmc_pad_drv_data[SDCC1]
	},
	[SDCC3] = {
		.pull = &mmc_pad_pull_data[SDCC3],
		.drv = &mmc_pad_drv_data[SDCC3]
	},
};

struct msm_mmc_pin_data mmc_slot_pin_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pad_data = &mmc_pad_data[SDCC1],
	},
	[SDCC3] = {
		.pad_data = &mmc_pad_data[SDCC3],
	},
};

static unsigned int sdc1_sup_clk_rates[] = {
	400000, 24000000, 48000000
};

static unsigned int sdc3_sup_clk_rates[] = {
	400000, 24000000, 48000000, 96000000
};

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static struct mmc_platform_data msm8960_sdc1_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.sup_clk_table	= sdc1_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc1_sup_clk_rates),
	.pclk_src_dfab	= 1,
	.nonremovable	= 1,
	.vreg_data	= &mmc_slot_vreg_data[SDCC1],
	.pin_data	= &mmc_slot_pin_data[SDCC1]
};
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static struct mmc_platform_data msm8960_sdc3_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.sup_clk_table	= sdc3_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc3_sup_clk_rates),
	.pclk_src_dfab	= 1,
#ifdef CONFIG_MMC_MSM_SDC3_WP_SUPPORT
	.wpswitch_gpio	= PM8921_GPIO_PM_TO_SYS(16),
#endif
	.vreg_data	= &mmc_slot_vreg_data[SDCC3],
	.pin_data	= &mmc_slot_pin_data[SDCC3],
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status_gpio	= PM8921_GPIO_PM_TO_SYS(26),
	.status_irq	= PM8921_GPIO_IRQ(PM8921_IRQ_BASE, 26),
	.irq_flags	= IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
	.xpc_cap	= 1,
	.uhs_caps	= (MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
			MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_DDR50 |
			MMC_CAP_MAX_CURRENT_600)
};
#endif

static void __init msm8960_init_mmc(void)
{
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	/* SDC1 : eMMC card connected */
	msm_add_sdcc(1, &msm8960_sdc1_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	/* SDC3: External card slot */
	msm_add_sdcc(3, &msm8960_sdc3_data);
#endif
}

static void __init msm8960_init_buses(void)
{
#ifdef CONFIG_MSM_BUS_SCALING
	msm_bus_rpm_set_mt_mask();
	msm_bus_8960_apps_fabric_pdata.rpm_enabled = 1;
	msm_bus_8960_sys_fabric_pdata.rpm_enabled = 1;
	msm_bus_8960_mm_fabric_pdata.rpm_enabled = 1;
	msm_bus_apps_fabric.dev.platform_data =
		&msm_bus_8960_apps_fabric_pdata;
	msm_bus_sys_fabric.dev.platform_data = &msm_bus_8960_sys_fabric_pdata;
	msm_bus_mm_fabric.dev.platform_data = &msm_bus_8960_mm_fabric_pdata;
	msm_bus_sys_fpb.dev.platform_data = &msm_bus_8960_sys_fpb_pdata;
	msm_bus_cpss_fpb.dev.platform_data = &msm_bus_8960_cpss_fpb_pdata;
#endif
}

static struct msm_spi_platform_data msm8960_qup_spi_gsbi9_pdata = {
	.max_clock_speed = 15060000,
};

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
static int synaptics_ncm_hw_setup(struct device *dev)
{
	gpio_set_value(18, 1);
	return 0;
}

static int synaptics_ncm_powerdown(struct device *dev)
{
	gpio_set_value(18, 0);
	return 0;
}

static int synaptics_ncm_poweroff(struct device *dev)
{
	int rc = 0;
	gpio_set_value(18, 0);
	gpio_set_value(93, 0);
	gpio_set_value(94, 0);
	gpio_set_value(95, 0);
	gpio_set_value(96, 0);
	return rc;
}

static void synaptics_ncm_hw_teardown(struct device *dev)
{
	gpio_set_value(18, 0);
}

static struct synaptics_ncm_platform_data synaptics_ncm_pdata = {
	.setup    = synaptics_ncm_hw_setup,
	.teardown = synaptics_ncm_hw_teardown,
	.powerdown = synaptics_ncm_powerdown,
	.poweroff = synaptics_ncm_poweroff,
};


static struct spi_board_info synaptics_ncm_spi_board_info[] __initdata = {
	{
		.modalias	= "synaptics_ncm_touchscreen",
		.mode		= SPI_MODE_3,
		.irq		= MSM_GPIO_TO_INT(46),
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz = 10800000,
		.platform_data = &synaptics_ncm_pdata,
	},
};
#endif /* defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM) */

#ifdef CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM
#define ANADEV_NCM_SHUTDOWN_GPIO_28 PM8921_GPIO_PM_TO_SYS(28)
struct pm_gpio anadev_ncm_haptics_shutdown_gpio28 = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value = 1,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength = PM_GPIO_STRENGTH_HIGH,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};

static int anadev_ncm_haptics_hw_setup(struct device *dev)
{
    int rc = 0;

    rc = gpio_request(ANADEV_NCM_SHUTDOWN_GPIO_28, "hpt_shdn");
    if (rc)
    {
        pr_err("[anadev_ncm_haptics]%s: request gpio 28 failed, rc=%d\n", __func__, rc);
        return -ENODEV;
    }

    rc = pm8xxx_gpio_config( ANADEV_NCM_SHUTDOWN_GPIO_28, &anadev_ncm_haptics_shutdown_gpio28 );
    if( rc ) {
        pr_err("[anadev_ncm_haptics]%s: pm8xxx_gpio_config(28): rc=%d\n", __func__, rc);
    }

    rc = gpio_direction_output(ANADEV_NCM_SHUTDOWN_GPIO_28, 1);
    if (rc) {
        printk("[anadev_ncm_haptics]gpio28 output failed.\n");
    }

    return rc;
}

static int anadev_ncm_haptics_powerdown(struct device *dev)
{
    int rc = 0;

    rc = gpio_direction_output(ANADEV_NCM_SHUTDOWN_GPIO_28, 0);
    if (rc) {
        printk("[anadev_ncm_haptics]gpio28 output failed.\n");
    }

    //Status of "Power-Down Mode" on ADUX1000.
    anadev_ncm_haptics_shutdown_gpio28.output_value = 0;
    anadev_ncm_haptics_shutdown_gpio28.out_strength = PM_GPIO_STRENGTH_LOW;

    rc = pm8xxx_gpio_config( ANADEV_NCM_SHUTDOWN_GPIO_28, &anadev_ncm_haptics_shutdown_gpio28 );
    if( rc )
        pr_err("[anadev_ncm_haptics]%s: pm8xxx_gpio_config(28): rc=%d\n", __func__, rc);

    gpio_free( ANADEV_NCM_SHUTDOWN_GPIO_28 );

    return 0;
}

static struct anadev_ncm_haptics_platform_data anadev_ncm_haptics_pdata = {
    .setup = anadev_ncm_haptics_hw_setup,
    .powerdown = anadev_ncm_haptics_powerdown,
};

static struct i2c_board_info anadev_ncm_info[] __initdata = {
    {
        I2C_BOARD_INFO( ANADEV_NCM_HAPTICS_DEV_NAME, 0x14 ),
        .platform_data = &anadev_ncm_haptics_pdata,
    },
};

#endif /* CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM */

#ifdef CONFIG_USB_MSM_OTG_72K
static struct msm_otg_platform_data msm_otg_pdata;
#else
#define USB_5V_EN		42
static void msm_hsusb_vbus_power(bool on)
{
	int rc;
	static bool vbus_is_on;
	static struct regulator *mvs_otg_switch;

	if (vbus_is_on == on)
		return;

	if (on) {
		mvs_otg_switch = regulator_get(&msm8960_device_otg.dev,
					       "vbus_otg");
		if (IS_ERR(mvs_otg_switch)) {
			pr_err("Unable to get mvs_otg_switch\n");
			return;
		}

		rc = gpio_request(PM8921_GPIO_PM_TO_SYS(USB_5V_EN),
						"usb_5v_en");
		if (rc < 0) {
			pr_err("failed to request usb_5v_en gpio\n");
			goto put_mvs_otg;
		}

		rc = gpio_direction_output(PM8921_GPIO_PM_TO_SYS(USB_5V_EN), 1);
		if (rc) {
			pr_err("%s: unable to set_direction for gpio [%d]\n",
				__func__, PM8921_GPIO_PM_TO_SYS(USB_5V_EN));
			goto free_usb_5v_en;
		}

		if (regulator_enable(mvs_otg_switch)) {
			pr_err("unable to enable mvs_otg_switch\n");
			goto err_ldo_gpio_set_dir;
		}

		vbus_is_on = true;
		return;
	}
	regulator_disable(mvs_otg_switch);
err_ldo_gpio_set_dir:
	gpio_set_value(PM8921_GPIO_PM_TO_SYS(USB_5V_EN), 0);
free_usb_5v_en:
	gpio_free(PM8921_GPIO_PM_TO_SYS(USB_5V_EN));
put_mvs_otg:
	regulator_put(mvs_otg_switch);
	vbus_is_on = false;
}

static struct msm_otg_platform_data msm_otg_pdata = {
	.mode			= USB_OTG,
	.otg_control		= OTG_PMIC_CONTROL,
	.phy_type		= SNPS_28NM_INTEGRATED_PHY,
	.pclk_src_name		= "dfab_usb_hs_clk",
	.pmic_id_irq		= PM8921_USB_ID_IN_IRQ(PM8921_IRQ_BASE),
	.vbus_power		= msm_hsusb_vbus_power,
	.power_budget		= 750,
};
#endif

#ifdef CONFIG_USB_EHCI_MSM_HSIC
#define HSIC_HUB_RESET_GPIO	91
static struct msm_hsic_host_platform_data msm_hsic_pdata = {
	.strobe		= 150,
	.data		= 151,
};
#else
static struct msm_hsic_host_platform_data msm_hsic_pdata;
#endif

#define PID_MAGIC_ID		0x71432909
#define SERIAL_NUM_MAGIC_ID	0x61945374
#define SERIAL_NUMBER_LENGTH	127
#define DLOAD_USB_BASE_ADD	0x2A03F0C8

struct magic_num_struct {
	uint32_t pid;
	uint32_t serial_num;
};

struct dload_struct {
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
	uint16_t	reserved4;
	uint16_t	pid;
	char		serial_number[SERIAL_NUMBER_LENGTH];
	uint16_t	reserved5;
	struct magic_num_struct magic_struct;
};

static int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum)
{
	struct dload_struct __iomem *dload = 0;

	dload = ioremap(DLOAD_USB_BASE_ADD, sizeof(*dload));
	if (!dload) {
		pr_err("%s: cannot remap I/O memory region: %08x\n",
					__func__, DLOAD_USB_BASE_ADD);
		return -ENXIO;
	}

	pr_debug("%s: dload:%p pid:%x serial_num:%s\n",
				__func__, dload, pid, snum);
	/* update pid */
	dload->magic_struct.pid = PID_MAGIC_ID;
	dload->pid = pid;

	/* update serial number */
	dload->magic_struct.serial_num = 0;
	if (!snum) {
		memset(dload->serial_number, 0, SERIAL_NUMBER_LENGTH);
		goto out;
	}

	dload->magic_struct.serial_num = SERIAL_NUM_MAGIC_ID;
	strlcpy(dload->serial_number, snum, SERIAL_NUMBER_LENGTH);
out:
	iounmap(dload);
	return 0;
}

static struct android_usb_platform_data android_usb_pdata = {
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

#ifdef CONFIG_FEATURE_NCMC_USB
#ifndef CONFIG_FEATURE_NCMC_RUBY
static u32 usbsw_hw_setup(void)
{
	int rc = 0;

	printk(KERN_INFO "[USB-SW] Called hw_setup: rc=%d\n", rc);

	return rc;
}

static struct bd91401gw_platform_data_struct usbsw_pdata = {
    .bd91401gw_setup = usbsw_hw_setup,
};

static struct i2c_board_info msm_i2c_board_info[] = {
	{
		I2C_BOARD_INFO(BD91401GW_I2C_DEVICE_NAME , BD91401GW_I2C_SLAVE_ADDRESS),
		.platform_data = &usbsw_pdata,
		.irq = PM8921_GPIO_IRQ(PM8921_IRQ_BASE,BD91401GW_I2C_PM_GPIO_INTB),
	}
};
#endif /* !CONFIG_FEATURE_NCMC_RUBY */
#endif /* CONFIG_FEATURE_NCMC_USB */

static void sndamp_spboost( u8 on )
{
	if( on )
	{
		gpio_set_value_cansleep(PM8921_MPP_PM_TO_SYS(10), 1);
		msleep(4);
	}else{
		gpio_set_value_cansleep(PM8921_MPP_PM_TO_SYS(10), 0);
	}
}

static void sndamp_hw_init(void)
{
	printk(KERN_INFO "%s \n", __func__);

	gpio_set_value_cansleep( PM8921_GPIO_PM_TO_SYS(11), 0 );

	/* 4ms wait */
	mdelay( 4 );
	gpio_set_value_cansleep( PM8921_GPIO_PM_TO_SYS(32), 1 );

	/* 1ms wait */
	mdelay( 1 );
	gpio_set_value_cansleep( PM8921_GPIO_PM_TO_SYS(11), 1 );
}

static void sndamp_hw_exit(void)
{
	printk(KERN_INFO "%s \n", __func__);
	gpio_set_value_cansleep( PM8921_GPIO_PM_TO_SYS( 11 ), 0 );

	gpio_set_value_cansleep( PM8921_GPIO_PM_TO_SYS(32), 0 );

}

static struct sndamp_i2c_platform_data sndamp_i2c_pdata = {
	.sndamp_i2c_setup    = sndamp_hw_init,
	.sndamp_i2c_shutdown = sndamp_hw_exit,
	.sndamp_spboost      = sndamp_spboost,
};

static struct i2c_board_info sndamp_i2c_board_info[] = {
	{
		I2C_BOARD_INFO("YDA160 I2C" , 0x6C),
		.platform_data = &sndamp_i2c_pdata,
	}
};


static uint8_t spm_wfi_cmd_sequence[] __initdata = {
			0x03, 0x0f,
};

static uint8_t spm_power_collapse_without_rpm[] __initdata = {
			0x00, 0x24, 0x54, 0x10,
			0x09, 0x03, 0x01,
			0x10, 0x54, 0x30, 0x0C,
			0x24, 0x30, 0x0f,
};

static uint8_t spm_power_collapse_with_rpm[] __initdata = {
			0x00, 0x24, 0x54, 0x10,
			0x09, 0x07, 0x01, 0x0B,
			0x10, 0x54, 0x30, 0x0C,
			0x24, 0x30, 0x0f,
};

static struct msm_spm_seq_entry msm_spm_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};

static struct msm_spm_platform_data msm_spm_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SECURE] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
		.reg_init_values[MSM_SPM_REG_SAW2_VCTL] = 0x9C,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020202,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0060009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x0000001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_seq_list),
		.modes = msm_spm_seq_list,
	},
	[1] = {
		.reg_base_addr = MSM_SAW1_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SECURE] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
		.reg_init_values[MSM_SPM_REG_SAW2_VCTL] = 0x9C,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020202,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0060009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x0000001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_seq_list),
		.modes = msm_spm_seq_list,
	},
};

static uint8_t l2_spm_wfi_cmd_sequence[] __initdata = {
			0x00, 0x20, 0x03, 0x20,
			0x00, 0x0f,
};

static uint8_t l2_spm_gdhs_cmd_sequence[] __initdata = {
			0x00, 0x20, 0x34, 0x64,
			0x48, 0x07, 0x48, 0x20,
			0x50, 0x64, 0x04, 0x34,
			0x50, 0x0f,
};
static uint8_t l2_spm_power_off_cmd_sequence[] __initdata = {
			0x00, 0x10, 0x34, 0x64,
			0x48, 0x07, 0x48, 0x10,
			0x50, 0x64, 0x04, 0x34,
			0x50, 0x0F,
};

static struct msm_spm_seq_entry msm_spm_l2_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_L2_MODE_RETENTION,
		.notify_rpm = false,
		.cmd = l2_spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_L2_MODE_GDHS,
		.notify_rpm = true,
		.cmd = l2_spm_gdhs_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_L2_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = l2_spm_power_off_cmd_sequence,
	},
};


static struct msm_spm_platform_data msm_spm_l2_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW_L2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SECURE] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020202,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x00A000AE,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A00020,
		.modes = msm_spm_l2_seq_list,
		.num_modes = ARRAY_SIZE(msm_spm_l2_seq_list),
	},
};

#define PM_HAP_EN_GPIO		PM8921_GPIO_PM_TO_SYS(33)
#define PM_HAP_LEN_GPIO		PM8921_GPIO_PM_TO_SYS(20)

static struct msm_xo_voter *xo_handle_d1;

static int isa1200_power(int on)
{
	int rc = 0;

	gpio_set_value(HAP_SHIFT_LVL_OE_GPIO, !!on);

	rc = on ? msm_xo_mode_vote(xo_handle_d1, MSM_XO_MODE_ON) :
			msm_xo_mode_vote(xo_handle_d1, MSM_XO_MODE_OFF);
	if (rc < 0) {
		pr_err("%s: failed to %svote for TCXO D1 buffer%d\n",
				__func__, on ? "" : "de-", rc);
		goto err_xo_vote;
	}

	return 0;

err_xo_vote:
	gpio_set_value(HAP_SHIFT_LVL_OE_GPIO, !on);
	return rc;
}

static int isa1200_dev_setup(bool enable)
{
	int rc = 0;

	struct pm_gpio hap_gpio_config = {
		.direction      = PM_GPIO_DIR_OUT,
		.pull           = PM_GPIO_PULL_NO,
		.out_strength   = PM_GPIO_STRENGTH_HIGH,
		.function       = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol    = 0,
		.vin_sel        = 2,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 0,
	};

	if (enable == true) {
		rc = pm8xxx_gpio_config(PM_HAP_EN_GPIO, &hap_gpio_config);
		if (rc) {
			pr_err("%s: pm8921 gpio %d config failed(%d)\n",
					__func__, PM_HAP_EN_GPIO, rc);
			return rc;
		}

		rc = pm8xxx_gpio_config(PM_HAP_LEN_GPIO, &hap_gpio_config);
		if (rc) {
			pr_err("%s: pm8921 gpio %d config failed(%d)\n",
					__func__, PM_HAP_LEN_GPIO, rc);
			return rc;
		}

		rc = gpio_request(HAP_SHIFT_LVL_OE_GPIO, "hap_shft_lvl_oe");
		if (rc) {
			pr_err("%s: unable to request gpio %d (%d)\n",
					__func__, HAP_SHIFT_LVL_OE_GPIO, rc);
			return rc;
		}

		rc = gpio_direction_output(HAP_SHIFT_LVL_OE_GPIO, 0);
		if (rc) {
			pr_err("%s: Unable to set direction\n", __func__);
			goto free_gpio;
		}

		xo_handle_d1 = msm_xo_get(MSM_XO_TCXO_D1, "isa1200");
		if (IS_ERR(xo_handle_d1)) {
			rc = PTR_ERR(xo_handle_d1);
			pr_err("%s: failed to get the handle for D1(%d)\n",
							__func__, rc);
			goto gpio_set_dir;
		}
	} else {
		gpio_free(HAP_SHIFT_LVL_OE_GPIO);

		msm_xo_put(xo_handle_d1);
	}

	return 0;

gpio_set_dir:
	gpio_set_value(HAP_SHIFT_LVL_OE_GPIO, 0);
free_gpio:
	gpio_free(HAP_SHIFT_LVL_OE_GPIO);
	return rc;
}

static struct isa1200_regulator isa1200_reg_data[] = {
	{
		.name = "vcc_i2c",
		.min_uV = ISA_I2C_VTG_MIN_UV,
		.max_uV = ISA_I2C_VTG_MAX_UV,
		.load_uA = ISA_I2C_CURR_UA,
	},
};

static struct isa1200_platform_data isa1200_1_pdata = {
	.name = "vibrator",
	.dev_setup = isa1200_dev_setup,
	.power_on = isa1200_power,
	.hap_en_gpio = PM_HAP_EN_GPIO,
	.hap_len_gpio = PM_HAP_LEN_GPIO,
	.max_timeout = 15000,
	.mode_ctrl = PWM_GEN_MODE,
	.pwm_fd = {
		.pwm_div = 256,
	},
	.is_erm = false,
	.smart_en = true,
	.ext_clk_en = true,
	.chip_en = 1,
	.regulator_info = isa1200_reg_data,
	.num_regulators = ARRAY_SIZE(isa1200_reg_data),
};

static struct i2c_board_info msm_isa1200_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("isa1200_1", 0x90>>1),
		.platform_data = &isa1200_1_pdata,
	},
};


/*virtual key support */
static ssize_t tma340_vkeys_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, 200,
	__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":73:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":230:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":389:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":544:1120:97:97"
	"\n");
}

static struct kobj_attribute tma340_vkeys_attr = {
	.attr = {
		.mode = S_IRUGO,
	},
	.show = &tma340_vkeys_show,
};

static struct attribute *tma340_properties_attrs[] = {
	&tma340_vkeys_attr.attr,
	NULL
};

static struct attribute_group tma340_properties_attr_group = {
	.attrs = tma340_properties_attrs,
};


static int cyttsp_platform_init(struct i2c_client *client)
{
	int rc = 0;
	static struct kobject *tma340_properties_kobj;

	tma340_vkeys_attr.attr.name = "virtualkeys.cyttsp-i2c";
	tma340_properties_kobj = kobject_create_and_add("board_properties",
								NULL);
	if (tma340_properties_kobj)
		rc = sysfs_create_group(tma340_properties_kobj,
					&tma340_properties_attr_group);
	if (!tma340_properties_kobj || rc)
		pr_err("%s: failed to create board_properties\n",
				__func__);

	return 0;
}

static struct cyttsp_regulator regulator_data[] = {
	{
		.name = "vdd",
		.min_uV = CY_TMA300_VTG_MIN_UV,
		.max_uV = CY_TMA300_VTG_MAX_UV,
		.hpm_load_uA = CY_TMA300_CURR_24HZ_UA,
		.lpm_load_uA = CY_TMA300_SLEEP_CURR_UA,
	},
	/* TODO: Remove after runtime PM is enabled in I2C driver */
	{
		.name = "vcc_i2c",
		.min_uV = CY_I2C_VTG_MIN_UV,
		.max_uV = CY_I2C_VTG_MAX_UV,
		.hpm_load_uA = CY_I2C_CURR_UA,
		.lpm_load_uA = CY_I2C_SLEEP_CURR_UA,
	},
};

static struct cyttsp_platform_data cyttsp_pdata = {
	.panel_maxx = 634,
	.panel_maxy = 1166,
	.disp_maxx = 616,
	.disp_maxy = 1023,
	.disp_minx = 0,
	.disp_miny = 16,
	.flags = 0x01,
	.gen = CY_GEN3,	/* or */
	.use_st = CY_USE_ST,
	.use_mt = CY_USE_MT,
	.use_hndshk = CY_SEND_HNDSHK,
	.use_trk_id = CY_USE_TRACKING_ID,
	.use_sleep = CY_USE_DEEP_SLEEP_SEL | CY_USE_LOW_POWER_SEL,
	.use_gestures = CY_USE_GESTURES,
	.fw_fname = "cyttsp_8960_cdp.hex",
	/* activate up to 4 groups
	 * and set active distance
	 */
	.gest_set = CY_GEST_GRP1 | CY_GEST_GRP2 |
				CY_GEST_GRP3 | CY_GEST_GRP4 |
				CY_ACT_DIST,
	/* change act_intrvl to customize the Active power state
	 * scanning/processing refresh interval for Operating mode
	 */
	.act_intrvl = CY_ACT_INTRVL_DFLT,
	/* change tch_tmout to customize the touch timeout for the
	 * Active power state for Operating mode
	 */
	.tch_tmout = CY_TCH_TMOUT_DFLT,
	/* change lp_intrvl to customize the Low Power power state
	 * scanning/processing refresh interval for Operating mode
	 */
	.lp_intrvl = CY_LP_INTRVL_DFLT,
	.regulator_info = regulator_data,
	.num_regulators = ARRAY_SIZE(regulator_data),
	.init = cyttsp_platform_init,
	.correct_fw_ver = 9,
};

static struct i2c_board_info cyttsp_info[] __initdata = {
	{
		I2C_BOARD_INFO(CY_I2C_NAME, 0x24),
		.platform_data = &cyttsp_pdata,
	},
};

/* configuration data */
static const u8 mxt_config_data[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	11, 2, 0, 11, 11, 11, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T7 Object */
	100, 16, 50,
	/* T8 Object */
	8, 0, 0, 0, 0, 0, 8, 14, 50, 215,
	/* T9 Object */
	131, 0, 0, 26, 42, 0, 32, 63, 3, 5,
	0, 2, 1, 113, 10, 10, 8, 10, 255, 2,
	85, 5, 0, 0, 20, 20, 75, 25, 202, 29,
	10, 10, 45, 46,
	/* T15 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
	/* T22 Object */
	5, 0, 0, 0, 0, 0, 0, 0, 30, 0,
	0, 0, 5, 8, 10, 13, 0,
	/* T24 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* T25 Object */
	3, 0, 188, 52, 52, 33, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T27 Object */
	0, 0, 0, 0, 0, 0, 0,
	/* T28 Object */
	0, 0, 0, 8, 12, 60,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T41 Object */
	0, 0, 0, 0, 0, 0,
	/* T43 Object */
	0, 0, 0, 0, 0, 0,
};

#if defined(CONFIG_LEDS_ADP8861) || defined(CONFIG_LEDS_LM3537)
static unsigned char leds_power_on = 0;
#endif /* defined(CONFIG_LEDS_ADP8861) || defined(CONFIG_LEDS_LM3537) */

#if defined(CONFIG_LEDS_ADP8861)

static int leds_adp8861_poweron(struct device *dev)
{
    static int gpio12;
    struct pm_gpio gpio12_param = {
        .direction = PM_GPIO_DIR_OUT,
        .output_buffer = PM_GPIO_OUT_BUF_CMOS,
        .output_value = 1,
        .pull = PM_GPIO_PULL_NO,
        .vin_sel = PM_GPIO_VIN_S4,
        .out_strength = PM_GPIO_STRENGTH_HIGH,
        .function = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol = 0,
        .disable_pin = 0,
    };
    int rc;

    if( leds_power_on )
    {
        printk(KERN_DEBUG "%s already power on\n", __func__);
        return 0;
    }
    leds_power_on = 1;

    gpio12 = PM8921_GPIO_PM_TO_SYS(12);

    rc = gpio_request(gpio12, "led_drv_rst_n");
    if (rc)
      {
          pr_err("request gpio 12 failed, rc=%d\n", rc);
          return -ENODEV;
      }

    rc = pm8xxx_gpio_config(gpio12, &gpio12_param);
    if (rc)
      {
          pr_err("gpio_config 12 failed (1), rc=%d\n", rc);
          return -EINVAL;
      }

    rc = gpio_direction_output(gpio12, 1);
    if (rc) {
        printk("gpio12 output failed.\n");
    }
    return rc;
}

static int leds_adp8861_poweroff(struct device *dev)
{
    int rc = 0;
    leds_power_on = 0;

    return rc;
}

static struct leds_adp8861_platform_data led_adp8861_pdata = {
    .poweron  = leds_adp8861_poweron,
    .poweroff = leds_adp8861_poweroff,
};


static struct i2c_board_info adp8861_i2c_board_info[] = {
	{
		I2C_BOARD_INFO("led_adp8861", 0x2A),
        .platform_data = &led_adp8861_pdata,
	}
};
static struct platform_device led_adp8861_if_leds = {
	.name   = "leds-adp8861_if",
	.id = -1,
};
#endif /* #if defined(CONFIG_LEDS_ADP8861) */


#if defined(CONFIG_LEDS_LM3537)

static int leds_lm3537_poweron(struct device *dev)
{
    static int gpio12;
    struct pm_gpio gpio12_param = {
        .direction = PM_GPIO_DIR_OUT,
        .output_buffer = PM_GPIO_OUT_BUF_CMOS,
        .output_value = 1,
        .pull = PM_GPIO_PULL_NO,
        .vin_sel = PM_GPIO_VIN_S4,
        .out_strength = PM_GPIO_STRENGTH_HIGH,
        .function = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol = 0,
        .disable_pin = 0,
    };
    int rc;

    if( leds_power_on )
    {
        printk(KERN_DEBUG "%s already power on\n", __func__);
        return 0;
    }
    leds_power_on = 1;

    gpio12 = PM8921_GPIO_PM_TO_SYS(12);

    rc = gpio_request(gpio12, "led_drv_rst_n");
    if (rc)
      {
          pr_err("request gpio 12 failed, rc=%d\n", rc);
          return -ENODEV;
      }

    rc = pm8xxx_gpio_config(gpio12, &gpio12_param);
    if (rc)
      {
          pr_err("gpio_config 12 failed (1), rc=%d\n", rc);
          return -EINVAL;
      }

    rc = gpio_direction_output(gpio12, 1);
    if (rc) {
        printk("gpio12 output failed.\n");
    }
    return rc;
}

static int leds_lm3537_poweroff(struct device *dev)
{
    int rc = 0;
    leds_power_on = 0;

    return rc;
}

static struct leds_lm3537_platform_data led_lm3537_pdata = {
    .poweron  = leds_lm3537_poweron,
    .poweroff = leds_lm3537_poweroff,
};

static struct i2c_board_info lm3537_i2c_board_info[] = {
    {
        I2C_BOARD_INFO("led_lm3537", 0x38),
        .platform_data = &led_lm3537_pdata,
    }
};
#endif /* #if defined(CONFIG_LEDS_LM3537) */


#define MXT_TS_GPIO_IRQ		11
#define MXT_TS_LDO_EN_GPIO	50
#define MXT_TS_RESET_GPIO	52

static void mxt_init_hw_liquid(void)
{
	int rc;

	rc = gpio_request(MXT_TS_GPIO_IRQ, "mxt_ts_irq_gpio");
	if (rc) {
		pr_err("%s: unable to request mxt_ts_irq gpio [%d]\n",
				__func__, MXT_TS_GPIO_IRQ);
		return;
	}

	rc = gpio_direction_input(MXT_TS_GPIO_IRQ);
	if (rc) {
		pr_err("%s: unable to set_direction for mxt_ts_irq gpio [%d]\n",
				__func__, MXT_TS_GPIO_IRQ);
		goto err_irq_gpio_req;
	}

	rc = gpio_request(MXT_TS_LDO_EN_GPIO, "mxt_ldo_en_gpio");
	if (rc) {
		pr_err("%s: unable to request mxt_ldo_en gpio [%d]\n",
				__func__, MXT_TS_LDO_EN_GPIO);
		goto err_irq_gpio_req;
	}

	rc = gpio_direction_output(MXT_TS_LDO_EN_GPIO, 1);
	if (rc) {
		pr_err("%s: unable to set_direction for mxt_ldo_en gpio [%d]\n",
				__func__, MXT_TS_LDO_EN_GPIO);
		goto err_ldo_gpio_req;
	}

	rc = gpio_request(MXT_TS_RESET_GPIO, "mxt_reset_gpio");
	if (rc) {
		pr_err("%s: unable to request mxt_reset gpio [%d]\n",
				__func__, MXT_TS_RESET_GPIO);
		goto err_ldo_gpio_set_dir;
	}

	rc = gpio_direction_output(MXT_TS_RESET_GPIO, 1);
	if (rc) {
		pr_err("%s: unable to set_direction for mxt_reset gpio [%d]\n",
				__func__, MXT_TS_RESET_GPIO);
		goto err_reset_gpio_req;
	}

	return;

err_reset_gpio_req:
	gpio_free(MXT_TS_RESET_GPIO);
err_ldo_gpio_set_dir:
	gpio_set_value(MXT_TS_LDO_EN_GPIO, 0);
err_ldo_gpio_req:
	gpio_free(MXT_TS_LDO_EN_GPIO);
err_irq_gpio_req:
	gpio_free(MXT_TS_GPIO_IRQ);
}

static struct mxt_platform_data mxt_platform_data = {
	.config			= mxt_config_data,
	.config_length		= ARRAY_SIZE(mxt_config_data),
	.x_size			= 1365,
	.y_size			= 767,
	.irqflags		= IRQF_TRIGGER_FALLING,
	.i2c_pull_up		= true,
};

static struct i2c_board_info mxt_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x5b),
		.platform_data = &mxt_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_TS_GPIO_IRQ),
	},
};

static void gsbi_qup_i2c_gpio_config(int adap_id, int config_type)
{
}

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi2_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi4_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi3_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi8_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi12_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_rpm_platform_data msm_rpm_data = {
	.reg_base_addrs = {
		[MSM_RPM_PAGE_STATUS] = MSM_RPM_BASE,
		[MSM_RPM_PAGE_CTRL] = MSM_RPM_BASE + 0x400,
		[MSM_RPM_PAGE_REQ] = MSM_RPM_BASE + 0x600,
		[MSM_RPM_PAGE_ACK] = MSM_RPM_BASE + 0xa00,
	},

	.irq_ack = RPM_APCC_CPU0_GP_HIGH_IRQ,
	.irq_err = RPM_APCC_CPU0_GP_LOW_IRQ,
	.irq_vmpm = RPM_APCC_CPU0_GP_MEDIUM_IRQ,
	.msm_apps_ipc_rpm_reg = MSM_APCS_GCC_BASE + 0x008,
	.msm_apps_ipc_rpm_val = 4,
};

#ifdef CONFIG_KS8851
static struct ks8851_pdata spi_eth_pdata = {
	.irq_gpio = KS8851_IRQ_GPIO,
	.rst_gpio = KS8851_RST_GPIO,
};

static struct spi_board_info spi_board_info[] __initdata = {
	{
		.modalias               = "ks8851",
		.irq                    = MSM_GPIO_TO_INT(KS8851_IRQ_GPIO),
		.max_speed_hz           = 19200000,
		.bus_num                = 0,
		.chip_select            = 0,
		.mode                   = SPI_MODE_0,
		.platform_data		= &spi_eth_pdata
	},
	{
		.modalias               = "dsi_novatek_3d_panel_spi",
		.max_speed_hz           = 10800000,
		.bus_num                = 0,
		.chip_select            = 1,
		.mode                   = SPI_MODE_0,
	},
};
#endif

static struct platform_device msm_device_saw_core0 = {
	.name          = "saw-regulator",
	.id            = 0,
	.dev	= {
#ifdef NCM_FUNCTION
    .platform_data = &nc_msm_saw_regulator_pdata_s5,
#else
		.platform_data = &msm_saw_regulator_pdata_s5,
#endif /* NCM_FUNCTION */
	},
};

static struct platform_device msm_device_saw_core1 = {
	.name          = "saw-regulator",
	.id            = 1,
	.dev	= {
#ifdef NCM_FUNCTION
    .platform_data = &nc_msm_saw_regulator_pdata_s6,
#else
		.platform_data = &msm_saw_regulator_pdata_s6,
#endif /* NCM_FUNCTION */
	},
};

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
static struct platform_device wfd_device = {
	.name          = "msm_wfd",
	.id            = -1,
};
#endif

static struct tsens_platform_data msm_tsens_pdata  = {
		.slope			= 910,
		.tsens_factor		= 1000,
		.hw_type		= MSM_8960,
		.tsens_num_sensor	= 5,
};

static struct platform_device msm_tsens_device = {
	.name	= "tsens8960-tm",
	.id	= -1,
	.dev	= {
		.platform_data = &msm_tsens_pdata,
	},
};

#ifdef CONFIG_MSM_FAKE_BATTERY
static struct platform_device fish_battery_device = {
	.name = "fish_battery",
};
#endif

static struct platform_device msm8960_device_ext_5v_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_MPP_PM_TO_SYS(7),
	.dev	= {
#ifdef NCM_FUNCTION
    .platform_data = &nc_msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_5V],
#else
		.platform_data = &msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_5V],
#endif /* NCM_FUNCTION */
	},
};

static struct platform_device msm8960_device_ext_l2_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= 91,
	.dev	= {
#ifdef NCM_FUNCTION
    .platform_data = &nc_msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_L2],
#else
		.platform_data = &msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_L2],
#endif /* NCM_FUNCTION */
	},
};

static struct platform_device msm8960_device_ext_3p3v_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_GPIO_PM_TO_SYS(17),
	.dev	= {
		.platform_data =
			&msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_3P3V],
	},
};

static struct platform_device msm8960_device_rpm_regulator __devinitdata = {
	.name	= "rpm-regulator",
	.id	= -1,
	.dev	= {
#ifdef NCM_FUNCTION
    .platform_data = &nc_msm_rpm_regulator_pdata,
#else
		.platform_data = &msm_rpm_regulator_pdata,
#endif /* NCM_FUNCTION */
	},
};

#ifdef CONFIG_FEATURE_NCMC_POWER
static struct platform_device nc_msm8960_device_rpm_regulator_oem __devinitdata = {
    .name = "rpm-regulator",
    .id   = -1,
    .dev  = {
        .platform_data = &nc_msm_rpm_regulator_pdata_oem,
    },
};
#endif

static struct msm_rpm_log_platform_data msm_rpm_log_pdata = {
	.phys_addr_base = 0x0010C000,
	.reg_offsets = {
		[MSM_RPM_LOG_PAGE_INDICES] = 0x00000080,
		[MSM_RPM_LOG_PAGE_BUFFER]  = 0x000000A0,
	},
	.phys_size = SZ_8K,
	.log_len = 4096,		  /* log's buffer length in bytes */
	.log_len_mask = (4096 >> 2) - 1,  /* length mask in units of u32 */
};

static struct platform_device msm_rpm_log_device = {
	.name	= "msm_rpm_log",
	.id	= -1,
	.dev	= {
		.platform_data = &msm_rpm_log_pdata,
	},
};

static struct platform_device *common_devices[] __initdata = {
	&msm8960_device_dmov,
	&msm_device_smd,
	&msm8960_device_uart_gsbi5,
	&msm_device_uart_dm6,
	&msm_device_gsbi10_uart_dm,
	&msm_device_saw_core0,
	&msm_device_saw_core1,
	&msm8960_device_ext_5v_vreg,
	&msm8960_device_ext_l2_vreg,
	&msm8960_device_ssbi_pm8921,
	&msm8960_device_qup_i2c_gsbi2,
	&msm8960_device_qup_i2c_gsbi3,
	&msm8960_device_qup_i2c_gsbi4,
	&msm8960_device_qup_spi_gsbi9,
	&msm8960_device_qup_i2c_gsbi8,
#ifndef CONFIG_MSM_DSPS
	&msm8960_device_qup_i2c_gsbi12,
#endif
	&msm_slim_ctrl,
	&msm_device_wcnss_wlan,
#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
	&qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
	&qcedev_device,
#endif
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
	&msm_device_sps,
#ifdef CONFIG_MSM_FAKE_BATTERY
	&fish_battery_device,
#endif
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	&android_pmem_device,
	&android_pmem_adsp_device,
#endif
	&android_pmem_audio_device,
#endif
	&msm_fb_device,
	&msm_device_vidc,
	&msm_device_bam_dmux,
	&msm_fm_platform_init,

#ifdef CONFIG_HW_RANDOM_MSM
	&msm_device_rng,
#endif
	&msm_rpm_device,
#ifdef CONFIG_ION_MSM
	&ion_dev,
#endif
	&msm_rpm_log_device,
	&msm_rpm_stat_device,
	&msm_device_tz_log,

#ifdef CONFIG_MSM_QDSS
	&msm_etb_device,
	&msm_tpiu_device,
	&msm_funnel_device,
	&msm_debug_device,
	&msm_ptm_device,
#endif
	&msm_device_dspcrashd_8960,
	&msm8960_device_watchdog,
#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	&wfd_panel_device,
	&wfd_device,
#endif
#ifdef CONFIG_LEDS_ADP8861
	&led_adp8861_if_leds,
#endif //#ifdef CONFIG_LEDS_ADP8861
};

static struct platform_device *sim_devices[] __initdata = {
	&msm8960_device_otg,
	&msm8960_device_gadget_peripheral,
	&msm_device_hsusb_host,
	&msm_device_hsic_host,
	&android_usb_device,
	&msm_device_vidc,
	&mipi_dsi_simulator_panel_device,
	&msm_bus_apps_fabric,
	&msm_bus_sys_fabric,
	&msm_bus_mm_fabric,
	&msm_bus_sys_fpb,
	&msm_bus_cpss_fpb,
	&msm_pcm,
	&msm_pcm_routing,
	&msm_cpudai0,
	&msm_cpudai1,
	&msm_cpudai_hdmi_rx,
	&msm_cpudai_bt_rx,
	&msm_cpudai_bt_tx,
	&msm_cpudai_fm_rx,
	&msm_cpudai_fm_tx,
	&msm_cpudai_auxpcm_rx,
	&msm_cpudai_auxpcm_tx,
	&msm_cpu_fe,
	&msm_stub_codec,
	&msm_voice,
	&msm_voip,
	&msm_lpa_pcm,

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
	&qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
	&qcedev_device,
#endif
};

static struct platform_device *rumi3_devices[] __initdata = {
	&msm_kgsl_3d0,
	&msm_kgsl_2d0,
	&msm_kgsl_2d1,
	&mipi_dsi_renesas_panel_device,
#ifdef CONFIG_MSM_GEMINI
	&msm8960_gemini_device,
#endif
};

static struct platform_device *cdp_devices[] __initdata = {
	&msm8960_device_otg,
	&msm8960_device_gadget_peripheral,
	&msm_device_hsusb_host,
	&android_usb_device,
	&msm_pcm,
	&msm_pcm_routing,
	&msm_cpudai0,
	&msm_cpudai1,
	&msm_cpudai_hdmi_rx,
	&msm_cpudai_bt_rx,
	&msm_cpudai_bt_tx,
	&msm_cpudai_fm_rx,
	&msm_cpudai_fm_tx,
	&msm_cpudai_auxpcm_rx,
	&msm_cpudai_auxpcm_tx,
	&msm_cpu_fe,
	&msm_stub_codec,
	&msm_kgsl_3d0,
#ifdef CONFIG_MSM_KGSL_2D
	&msm_kgsl_2d0,
	&msm_kgsl_2d1,
#endif
	&mipi_dsi_novatek_panel_device,
	/*&mipi_dsi_toshiba_panel_device,*/
	&mipi_dsi_renesas_panel_device,
#ifdef CONFIG_MSM_GEMINI
	&msm8960_gemini_device,
#endif
	&msm_voice,
	&msm_voip,
	&msm_lpa_pcm,
	&msm_cpudai_afe_01_rx,
	&msm_cpudai_afe_01_tx,
	&msm_cpudai_afe_02_rx,
	&msm_cpudai_afe_02_tx,
	&msm_pcm_afe,
#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
	&hdmi_msm_device,
#endif
	&msm_pcm_hostless,
	&msm_bus_apps_fabric,
	&msm_bus_sys_fabric,
	&msm_bus_mm_fabric,
	&msm_bus_sys_fpb,
	&msm_bus_cpss_fpb,
	&msm_tsens_device,
};

static void __init msm8960_i2c_init(void)
{
	msm8960_device_qup_i2c_gsbi2.dev.platform_data =
					&msm8960_i2c_qup_gsbi2_pdata;

	msm8960_device_qup_i2c_gsbi4.dev.platform_data =
					&msm8960_i2c_qup_gsbi4_pdata;

	msm8960_device_qup_i2c_gsbi3.dev.platform_data =
					&msm8960_i2c_qup_gsbi3_pdata;

	msm8960_device_qup_i2c_gsbi8.dev.platform_data =
					&msm8960_i2c_qup_gsbi8_pdata;

	msm8960_device_qup_i2c_gsbi12.dev.platform_data =
					&msm8960_i2c_qup_gsbi12_pdata;
}

static void __init msm8960_gfx_init(void)
{
	uint32_t soc_platform_version = socinfo_get_version();
	if (SOCINFO_VERSION_MAJOR(soc_platform_version) == 1) {
		struct kgsl_device_platform_data *kgsl_3d0_pdata =
				msm_kgsl_3d0.dev.platform_data;
		kgsl_3d0_pdata->pwr_data.pwrlevel[0].gpu_freq =
				320000000;
		kgsl_3d0_pdata->pwr_data.pwrlevel[1].gpu_freq =
				266667000;
	}
}

static struct pm8xxx_irq_platform_data pm8xxx_irq_pdata __devinitdata = {
	.irq_base		= PM8921_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(104),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
};

static struct pm8xxx_gpio_platform_data pm8xxx_gpio_pdata __devinitdata = {
	.gpio_base	= PM8921_GPIO_PM_TO_SYS(1),
};

static struct pm8xxx_mpp_platform_data pm8xxx_mpp_pdata __devinitdata = {
	.mpp_base	= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_rtc_platform_data pm8xxx_rtc_pdata __devinitdata = {
	.rtc_write_enable       = false,
	.rtc_alarm_powerup	= false,
};

static struct pm8xxx_pwrkey_platform_data pm8xxx_pwrkey_pdata = {
	.pull_up		= 1,
	.kpd_trigger_delay_us	= 970,
	.wakeup			= 1,
};

/* Rotate lock key is not available so use F1 */
#define KEY_ROTATE_LOCK KEY_F1

static const unsigned int keymap_liquid[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
	KEY(1, 3, KEY_ROTATE_LOCK),
	KEY(1, 4, KEY_HOME),
};

static struct matrix_keymap_data keymap_data_liquid = {
	.keymap_size    = ARRAY_SIZE(keymap_liquid),
	.keymap         = keymap_liquid,
};

static struct pm8xxx_keypad_platform_data keypad_data_liquid = {
	.input_name             = "keypad_8960_liquid",
	.input_phys_device      = "keypad_8960/input0",
	.num_rows               = 2,
	.num_cols               = 5,
	.rows_gpio_start	= PM8921_GPIO_PM_TO_SYS(9),
	.cols_gpio_start	= PM8921_GPIO_PM_TO_SYS(1),
	.debounce_ms            = 15,
	.scan_delay_ms          = 32,
	.row_hold_ns            = 91500,
	.wakeup                 = 1,
	.keymap_data            = &keymap_data_liquid,
};


static const unsigned int keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
	KEY(0, 2, KEY_CAMERA_SNAPSHOT),
	KEY(0, 3, KEY_CAMERA_FOCUS),
};

static struct matrix_keymap_data keymap_data = {
	.keymap_size    = ARRAY_SIZE(keymap),
	.keymap         = keymap,
};

#ifdef NCM_FUNCTION
static struct pm8xxx_keypad_platform_data keypad_data = {
  .input_name             = "keypad_8960",
  .input_phys_device      = "keypad_8960/input0",
  .num_rows               = 2,
  .num_cols               = 6,
  .rows_gpio_start  = PM8921_GPIO_PM_TO_SYS(9),
  .cols_gpio_start  = PM8921_GPIO_PM_TO_SYS(1),
  .debounce_ms            = 15,
  .scan_delay_ms          = 32,
  .row_hold_ns            = 91500,
  .wakeup                 = 1,
  .keymap_data            = &keymap_data,
};
#else
static struct pm8xxx_keypad_platform_data keypad_data = {
	.input_name             = "keypad_8960",
	.input_phys_device      = "keypad_8960/input0",
	.num_rows               = 1,
	.num_cols               = 5,
	.rows_gpio_start	= PM8921_GPIO_PM_TO_SYS(9),
	.cols_gpio_start	= PM8921_GPIO_PM_TO_SYS(1),
	.debounce_ms            = 15,
	.scan_delay_ms          = 32,
	.row_hold_ns            = 91500,
	.wakeup                 = 1,
	.keymap_data            = &keymap_data,
};
#endif /* NCM_FUNCTION */

static const unsigned int keymap_sim[] = {
	KEY(0, 0, KEY_7),
	KEY(0, 1, KEY_DOWN),
	KEY(0, 2, KEY_UP),
	KEY(0, 3, KEY_RIGHT),
	KEY(0, 4, KEY_ENTER),
	KEY(0, 5, KEY_L),
	KEY(0, 6, KEY_BACK),
	KEY(0, 7, KEY_M),

	KEY(1, 0, KEY_LEFT),
	KEY(1, 1, KEY_SEND),
	KEY(1, 2, KEY_1),
	KEY(1, 3, KEY_4),
	KEY(1, 4, KEY_CLEAR),
	KEY(1, 5, KEY_MSDOS),
	KEY(1, 6, KEY_SPACE),
	KEY(1, 7, KEY_COMMA),

	KEY(2, 0, KEY_6),
	KEY(2, 1, KEY_5),
	KEY(2, 2, KEY_8),
	KEY(2, 3, KEY_3),
	KEY(2, 4, KEY_NUMERIC_STAR),
	KEY(2, 5, KEY_UP),
	KEY(2, 6, KEY_DOWN),
	KEY(2, 7, KEY_LEFTSHIFT),

	KEY(3, 0, KEY_9),
	KEY(3, 1, KEY_NUMERIC_POUND),
	KEY(3, 2, KEY_0),
	KEY(3, 3, KEY_2),
	KEY(3, 4, KEY_SLEEP),
	KEY(3, 5, KEY_F1),
	KEY(3, 6, KEY_F2),
	KEY(3, 7, KEY_F3),

	KEY(4, 0, KEY_BACK),
	KEY(4, 1, KEY_HOME),
	KEY(4, 2, KEY_MENU),
	KEY(4, 3, KEY_VOLUMEUP),
	KEY(4, 4, KEY_VOLUMEDOWN),
	KEY(4, 5, KEY_F4),
	KEY(4, 6, KEY_F5),
	KEY(4, 7, KEY_F6),

	KEY(5, 0, KEY_R),
	KEY(5, 1, KEY_T),
	KEY(5, 2, KEY_Y),
	KEY(5, 3, KEY_LEFTALT),
	KEY(5, 4, KEY_KPENTER),
	KEY(5, 5, KEY_Q),
	KEY(5, 6, KEY_W),
	KEY(5, 7, KEY_E),

	KEY(6, 0, KEY_F),
	KEY(6, 1, KEY_G),
	KEY(6, 2, KEY_H),
	KEY(6, 3, KEY_CAPSLOCK),
	KEY(6, 4, KEY_PAGEUP),
	KEY(6, 5, KEY_A),
	KEY(6, 6, KEY_S),
	KEY(6, 7, KEY_D),

	KEY(7, 0, KEY_V),
	KEY(7, 1, KEY_B),
	KEY(7, 2, KEY_N),
	KEY(7, 3, KEY_MENU),
	KEY(7, 4, KEY_PAGEDOWN),
	KEY(7, 5, KEY_Z),
	KEY(7, 6, KEY_X),
	KEY(7, 7, KEY_C),

	KEY(8, 0, KEY_P),
	KEY(8, 1, KEY_J),
	KEY(8, 2, KEY_K),
	KEY(8, 3, KEY_INSERT),
	KEY(8, 4, KEY_LINEFEED),
	KEY(8, 5, KEY_U),
	KEY(8, 6, KEY_I),
	KEY(8, 7, KEY_O),

	KEY(9, 0, KEY_4),
	KEY(9, 1, KEY_5),
	KEY(9, 2, KEY_6),
	KEY(9, 3, KEY_7),
	KEY(9, 4, KEY_8),
	KEY(9, 5, KEY_1),
	KEY(9, 6, KEY_2),
	KEY(9, 7, KEY_3),

	KEY(10, 0, KEY_F7),
	KEY(10, 1, KEY_F8),
	KEY(10, 2, KEY_F9),
	KEY(10, 3, KEY_F10),
	KEY(10, 4, KEY_FN),
	KEY(10, 5, KEY_9),
	KEY(10, 6, KEY_0),
	KEY(10, 7, KEY_DOT),

	KEY(11, 0, KEY_LEFTCTRL),
	KEY(11, 1, KEY_F11),
	KEY(11, 2, KEY_ENTER),
	KEY(11, 3, KEY_SEARCH),
	KEY(11, 4, KEY_DELETE),
	KEY(11, 5, KEY_RIGHT),
	KEY(11, 6, KEY_LEFT),
	KEY(11, 7, KEY_RIGHTSHIFT),
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
	KEY(0, 2, KEY_CAMERA_SNAPSHOT),
	KEY(0, 3, KEY_CAMERA_FOCUS),
};

static struct matrix_keymap_data keymap_data_sim = {
	.keymap_size    = ARRAY_SIZE(keymap_sim),
	.keymap         = keymap_sim,
};

static struct pm8xxx_keypad_platform_data keypad_data_sim = {
	.input_name             = "keypad_8960",
	.input_phys_device      = "keypad_8960/input0",
	.num_rows               = 12,
	.num_cols               = 8,
	.rows_gpio_start	= PM8921_GPIO_PM_TO_SYS(9),
	.cols_gpio_start	= PM8921_GPIO_PM_TO_SYS(1),
	.debounce_ms            = 15,
	.scan_delay_ms          = 32,
	.row_hold_ns            = 91500,
	.wakeup                 = 1,
	.keymap_data            = &keymap_data_sim,
};

#if !defined(CONFIG_FEATURE_NCMC_POWER)
static int pm8921_therm_mitigation[] = {
	1100,
	700,
	600,
	325,
};

static struct pm8921_charger_platform_data pm8921_chg_pdata __devinitdata = {
	.safety_time		= 180,
	.update_time		= 60000,
	.max_voltage		= 4200,
	.min_voltage		= 3200,
    #ifdef CONFIG_FEATURE_NCMC_POWER
	.resume_voltage_delta	= 300,
    #else
	.resume_voltage_delta	= 100,
    #endif
	.term_current		= 100,
	.cool_temp		= 10,
	.warm_temp		= 40,
	.temp_check_period	= 1,
    #if defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
	.max_bat_chg_current	= 1300,
    #else
	.max_bat_chg_current	= 1100,
	.cool_bat_chg_current	= 350,
	.warm_bat_chg_current	= 350,
    #endif /* CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960 */
	.cool_bat_voltage	= 4100,
	.warm_bat_voltage	= 4100,
	.thermal_mitigation	= pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(pm8921_therm_mitigation),
};
#endif /* CONFIG_FEATURE_NCMC_POWER */

static struct pm8xxx_misc_platform_data pm8xxx_misc_pdata = {
	.priority		= 0,
};

static struct pm8921_bms_platform_data pm8921_bms_pdata __devinitdata = {
	.r_sense		= 10,
	.i_test			= 2500,
	.v_failure		= 3000,
	.calib_delay_ms		= 600000,
};

#define	PM8921_LC_LED_MAX_CURRENT	4	/* I = 4mA */
#define PM8XXX_LED_PWM_PERIOD		1000
#define PM8XXX_LED_PWM_DUTY_MS		20
/**
 * PM8XXX_PWM_CHANNEL_NONE shall be used when LED shall not be
 * driven using PWM feature.
 */
#define PM8XXX_PWM_CHANNEL_NONE		-1

static struct led_info pm8921_led_info[] = {
	[0] = {
		.name			= "led:battery_charging",
		.default_trigger	= "battery-charging",
	},
	[1] = {
		.name			= "led:battery_full",
		.default_trigger	= "battery-full",
	},
};

static struct led_platform_data pm8921_led_core_pdata = {
	.num_leds = ARRAY_SIZE(pm8921_led_info),
	.leds = pm8921_led_info,
};

static int pm8921_led0_pwm_duty_pcts[56] = {
		1, 4, 8, 12, 16, 20, 24, 28, 32, 36,
		40, 44, 46, 52, 56, 60, 64, 68, 72, 76,
		80, 84, 88, 92, 96, 100, 100, 100, 98, 95,
		92, 88, 84, 82, 78, 74, 70, 66, 62, 58,
		58, 54, 50, 48, 42, 38, 34, 30, 26, 22,
		14, 10, 6, 4, 1
};

static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts = (int *)&pm8921_led0_pwm_duty_pcts,
	.num_duty_pcts = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts),
	.duty_ms = PM8XXX_LED_PWM_DUTY_MS,
	.start_idx = 0,
};

static struct pm8xxx_led_config pm8921_led_configs[] = {
	[0] = {
		.id = PM8XXX_ID_LED_0,
		.mode = PM8XXX_LED_MODE_PWM2,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 5,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
	[1] = {
		.id = PM8XXX_ID_LED_1,
		.mode = PM8XXX_LED_MODE_PWM1,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 4,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
	},
};

static struct pm8xxx_led_platform_data pm8xxx_leds_pdata = {
		.led_core = &pm8921_led_core_pdata,
		.configs = pm8921_led_configs,
		.num_configs = ARRAY_SIZE(pm8921_led_configs),
};

static struct pm8xxx_ccadc_platform_data pm8xxx_ccadc_pdata = {
	.r_sense		= 10,
};

static struct pm8xxx_vibrator_platform_data pm8xxx_vib_pdata = {
  .initial_vibrate_ms  = 500,
  .max_timeout_ms = 15000,
  .level_mV = 3000,
};

static struct pm8921_platform_data pm8921_platform_data __devinitdata = {
	.irq_pdata		= &pm8xxx_irq_pdata,
	.gpio_pdata		= &pm8xxx_gpio_pdata,
	.mpp_pdata		= &pm8xxx_mpp_pdata,
	.rtc_pdata              = &pm8xxx_rtc_pdata,
	.pwrkey_pdata		= &pm8xxx_pwrkey_pdata,
	.keypad_pdata		= &keypad_data,
	.misc_pdata		= &pm8xxx_misc_pdata,
#ifdef NCM_FUNCTION
  .regulator_pdatas = nc_msm_pm8921_regulator_pdata,
#else
	.regulator_pdatas	= msm_pm8921_regulator_pdata,
#endif /* NCM_FUNCTION */
#ifdef NCM_FUNCTION
    .charger_pdata      = &nc_pm8921_chg_pdata,
#else
	.charger_pdata		= &pm8921_chg_pdata,
#endif /* NCM_FUNCTION */
	.bms_pdata		= &pm8921_bms_pdata,
#ifdef NCM_FUNCTION
    .adc_pdata      = &nc_pm8921_adc_pdata,
#else
	.adc_pdata		= &pm8921_adc_pdata,
#endif /* NCM_FUNCTION */
	.leds_pdata		= &pm8xxx_leds_pdata,
	.ccadc_pdata		= &pm8xxx_ccadc_pdata,
  	.vibrator_pdata = &pm8xxx_vib_pdata,
};

static struct msm_ssbi_platform_data msm8960_ssbi_pm8921_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name			= "pm8921-core",
		.platform_data		= &pm8921_platform_data,
	},
};

static struct msm_cpuidle_state msm_cstates[] __initdata = {
	{0, 0, "C0", "WFI",
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT},

	{0, 1, "C1", "STANDALONE_POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE},

	{0, 2, "C2", "POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE},

	{1, 0, "C0", "WFI",
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT},

	{1, 1, "C1", "STANDALONE_POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE},
};

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR * 2] = {
	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_POWER_COLLAPSE)] = {
		.idle_supported = 0,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
		.idle_supported = 1,
		.suspend_supported = 0,
		.idle_enabled = 1,
		.suspend_enabled = 0,
	},
};

static struct msm_rpmrs_level msm_rpmrs_levels[] __initdata = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		100, 8000, 100000, 1,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		2000, 6000, 60100000, 3000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, GDHS, MAX, ACTIVE),
		false,
		4200, 5000, 60350000, 3500,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, MAX, ACTIVE),
		false,
		6300, 4500, 65350000, 4800,
	},
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		7000, 3500, 66600000, 5150,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, GDHS, MAX, ACTIVE),
		false,
		11700, 2500, 67850000, 5500,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MAX, ACTIVE),
		false,
		13800, 2000, 71850000, 6800,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		29700, 500, 75850000, 8800,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, RET_HIGH, RET_LOW),
		false,
		29700, 0, 76350000, 9800,
	},
};

#ifdef CONFIG_I2C
#define I2C_SURF 1
#define I2C_FFA  (1 << 1)
#define I2C_RUMI (1 << 2)
#define I2C_SIM  (1 << 3)
#define I2C_FLUID (1 << 4)
#define I2C_LIQUID (1 << 5)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};

#ifdef CONFIG_MSM_CAMERA
static struct i2c_board_info msm_camera_boardinfo[] __initdata = {
#ifdef CONFIG_CE150X
	{
	I2C_BOARD_INFO("ce150x", 0x78),
	},
#endif
#ifdef CONFIG_MT9D115_SUB
	{
	I2C_BOARD_INFO("mt9d115_sub", 0x78 >> 1),
	},
#endif
#ifdef CONFIG_IMX074
	{
	I2C_BOARD_INFO("imx074", 0x1A),
	},
#endif
#ifdef CONFIG_OV2720
	{
	I2C_BOARD_INFO("ov2720", 0x6C),
	},
#endif
#ifdef CONFIG_MSM_CAMERA_FLASH_SC628A
	{
	I2C_BOARD_INFO("sc628a", 0x6E),
	},
#endif
#ifdef CONFIG_IMX111
	{
	I2C_BOARD_INFO("imx111", 0x1A),
	},
#endif
};
#endif

#ifdef CONFIG_MSM_CAMERA
static struct i2c_board_info msm_camera_boardinfo_sub[] __initdata = {
#ifdef CONFIG_MT9M113
	{
	I2C_BOARD_INFO("mt9m113", 0x3D),
	},
#endif
};
#endif /* CONFIG_MSM_CAMERA */

/* Sensors DSPS platform data */
#ifdef CONFIG_MSM_DSPS
#define DSPS_PIL_GENERIC_NAME		"dsps"
#endif /* CONFIG_MSM_DSPS */

static void __init msm8960_init_dsps(void)
{
#ifdef CONFIG_MSM_DSPS
	struct msm_dsps_platform_data *pdata =
		msm_dsps_device.dev.platform_data;
	pdata->pil_name = DSPS_PIL_GENERIC_NAME;
	pdata->gpios = NULL;
	pdata->gpios_num = 0;

	platform_device_register(&msm_dsps_device);
#endif /* CONFIG_MSM_DSPS */
}

static void __init msm8960_init_hsic(void)
{
#ifdef CONFIG_USB_EHCI_MSM_HSIC
	uint32_t version = socinfo_get_version();

	pr_info("%s: version:%d mtp:%d\n", __func__,
			SOCINFO_VERSION_MAJOR(version),
			machine_is_msm8960_mtp());

	if ((SOCINFO_VERSION_MAJOR(version) == 1) ||
			machine_is_msm8960_mtp() ||
			machine_is_msm8960_fluid())
		return;

	msm_gpiomux_install(msm8960_hsic_configs,
			ARRAY_SIZE(msm8960_hsic_configs));

	platform_device_register(&msm_device_hsic_host);
#endif
}


#ifdef CONFIG_ISL9519_CHARGER
static struct isl_platform_data isl_data __initdata = {
	.valid_n_gpio		= 0,	/* Not required when notify-by-pmic */
	.chg_detection_config	= NULL,	/* Not required when notify-by-pmic */
	.max_system_voltage	= 4200,
	.min_system_voltage	= 3200,
	.chgcurrent		= 1000, /* 1900, */
	.term_current		= 400,	/* Need fine tuning */
	.input_current		= 2048,
};

static struct i2c_board_info isl_charger_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("isl9519q", 0x9),
		.irq		= 0,	/* Not required when notify-by-pmic */
		.platform_data	= &isl_data,
	},
};
#endif /* CONFIG_ISL9519_CHARGER */

static struct i2c_registry msm8960_i2c_devices[] __initdata = {
#ifdef CONFIG_MSM_CAMERA
	{
		I2C_SURF | I2C_FFA | I2C_FLUID | I2C_LIQUID | I2C_RUMI,
		MSM_8960_GSBI2_QUP_I2C_BUS_ID,
		msm_camera_boardinfo_sub,
		ARRAY_SIZE(msm_camera_boardinfo_sub),
	},
#endif
#ifdef CONFIG_MSM_CAMERA
	{
		I2C_SURF | I2C_FFA | I2C_FLUID | I2C_LIQUID | I2C_RUMI,
		MSM_8960_GSBI4_QUP_I2C_BUS_ID,
		msm_camera_boardinfo,
		ARRAY_SIZE(msm_camera_boardinfo),
	},
#endif
#ifdef CONFIG_ISL9519_CHARGER
	{
		I2C_LIQUID,
		MSM_8960_GSBI10_QUP_I2C_BUS_ID,
		isl_charger_i2c_info,
		ARRAY_SIZE(isl_charger_i2c_info),
	},
#endif /* CONFIG_ISL9519_CHARGER */
	{
		I2C_SURF | I2C_FFA | I2C_FLUID,
		MSM_8960_GSBI3_QUP_I2C_BUS_ID,
		cyttsp_info,
		ARRAY_SIZE(cyttsp_info),
	},
	{
		I2C_LIQUID,
		MSM_8960_GSBI3_QUP_I2C_BUS_ID,
		mxt_device_info,
		ARRAY_SIZE(mxt_device_info),
	},
	{
		I2C_LIQUID,
		MSM_8960_GSBI10_QUP_I2C_BUS_ID,
		msm_isa1200_board_info,
		ARRAY_SIZE(msm_isa1200_board_info),
	},
#ifdef CONFIG_FEATURE_NCMC_USB
#ifndef CONFIG_FEATURE_NCMC_RUBY
	{
		I2C_SURF,
		MSM_8960_GSBI8_QUP_I2C_BUS_ID,
		msm_i2c_board_info,
		ARRAY_SIZE(msm_i2c_board_info),
	},
#endif /* !CONFIG_FEATURE_NCMC_RUBY */
#endif /* CONFIG_FEATURE_NCMC_USB */
	{
		I2C_SURF,
		MSM_8960_GSBI12_QUP_I2C_BUS_ID,
		msm_i2c_sensor_board_info,
		ARRAY_SIZE(msm_i2c_sensor_board_info),
	},
	{
		I2C_SURF,
		MSM_8960_GSBI8_QUP_I2C_BUS_ID,
		felica_i2c_info,
		ARRAY_SIZE(felica_i2c_info),
	},
#ifdef CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM
	{
		I2C_SURF | I2C_FFA | I2C_FLUID | I2C_RUMI,
		MSM_8960_GSBI8_QUP_I2C_BUS_ID,
		anadev_ncm_info,
		ARRAY_SIZE(anadev_ncm_info),
	},
#endif /* CONFIG_INPUT_ANALOGDEVICE_ADUX1000_NCM */
#ifdef CONFIG_FEATURE_NCMC_DTV
	{
		I2C_SURF | I2C_FFA | I2C_FLUID | I2C_RUMI,
		MSM_8960_GSBI8_QUP_I2C_BUS_ID,
		nim_boardinfo,
		ARRAY_SIZE(nim_boardinfo),
	},
#endif /* CONFIG_FEATURE_NCMC_DTV */
	{
		I2C_SURF | I2C_FFA | I2C_FLUID,
		MSM_8960_GSBI8_QUP_I2C_BUS_ID,
		sndamp_i2c_board_info,
		ARRAY_SIZE(sndamp_i2c_board_info),
	},
#if defined(CONFIG_LEDS_LM3537)
	{
		I2C_SURF | I2C_FFA | I2C_FLUID | I2C_RUMI,
		MSM_8960_GSBI8_QUP_I2C_BUS_ID,
		lm3537_i2c_board_info,
		ARRAY_SIZE(lm3537_i2c_board_info),
	},
#endif /* #if defined(CONFIG_LEDS_LM3537) */
#if defined(CONFIG_LEDS_ADP8861)
	{
		I2C_SURF | I2C_FFA | I2C_FLUID | I2C_RUMI,
		MSM_8960_GSBI8_QUP_I2C_BUS_ID,
		adp8861_i2c_board_info,
		ARRAY_SIZE(adp8861_i2c_board_info),
	},
#endif /* #if defined(CONFIG_LEDS_ADP8861) */
};
#endif /* CONFIG_I2C */

static void __init register_i2c_devices(void)
{
#ifdef CONFIG_I2C
	u8 mach_mask = 0;
	int i;

	/* Build the matching 'supported_machs' bitmask */
	if (machine_is_msm8960_cdp())
		mach_mask = I2C_SURF;
	else if (machine_is_msm8960_rumi3())
		mach_mask = I2C_RUMI;
	else if (machine_is_msm8960_sim())
		mach_mask = I2C_SIM;
	else if (machine_is_msm8960_fluid())
		mach_mask = I2C_FLUID;
	else if (machine_is_msm8960_liquid())
		mach_mask = I2C_LIQUID;
	else if (machine_is_msm8960_mtp())
		mach_mask = I2C_FFA;
	else
		pr_err("unmatched machine ID in register_i2c_devices\n");

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(msm8960_i2c_devices); ++i) {
		if (msm8960_i2c_devices[i].machs & mach_mask)
			i2c_register_board_info(msm8960_i2c_devices[i].bus,
						msm8960_i2c_devices[i].info,
						msm8960_i2c_devices[i].len);
	}
#endif
}

#ifdef CONFIG_FEATURE_NCMC_CPRM
#define MSM_REG_GP1_MD			(MSM_CLK_CTL_BASE + 0x2D00 + 0x20)
#define MSM_REG_GP1_NS			(MSM_CLK_CTL_BASE + 0x2D24 + 0x20)
#define MSM_REG_GP1_MD_MASK		(0xFF00FF00)
#define MSM_REG_GP1_MD_SET		(0x000100FA)
#define MSM_REG_GP1_NS_MASK		(0xFF00F480)
#define MSM_REG_GP1_NS_ENABLE	(0x00FB0B5B)
#define MSM_REG_GP1_NS_DISABLE	(0x00FB005B)

static uint32_t mn66831_msm_clock_init( void )
{
	uint32_t rc = 0;
	uint32_t value = 0;
	void __iomem *msm_gp1_md_addr = 0;

	/* GP1_MD */
	msm_gp1_md_addr = MSM_REG_GP1_MD;

	pr_err("%s: before - MSM_REG_GP1_MD = 0x%08X\n",
				__func__, readl(msm_gp1_md_addr));

	value = (readl(msm_gp1_md_addr) & MSM_REG_GP1_MD_MASK);
	value |= MSM_REG_GP1_MD_SET;
	writel(value, msm_gp1_md_addr);

	pr_err("%s: after  - MSM_REG_GP1_MD = 0x%08X\n",
				__func__, readl(msm_gp1_md_addr));

	return rc;
}

static uint32_t mn66831_msm_setup_clock(int enable)
{
	uint32_t rc = 0;
	uint32_t value = 0;
	void __iomem *msm_gp1_ns_addr = 0;

	/* GP1_NS */
	msm_gp1_ns_addr = MSM_REG_GP1_NS;

	pr_err("%s: before - MSM_REG_GP1_NS = 0x%08X\n",
				__func__, readl(msm_gp1_ns_addr));

	value = (readl(msm_gp1_ns_addr) & MSM_REG_GP1_NS_MASK);
	if (enable) {
		value |= MSM_REG_GP1_NS_ENABLE;
	} else {
		value |= MSM_REG_GP1_NS_DISABLE;
	}
	writel(value, msm_gp1_ns_addr);

	pr_err("%s: after  - MSM_REG_GP1_NS = 0x%08X\n",
				__func__, readl(msm_gp1_ns_addr));

	return rc;
}

#define MSM_GPIO_SDEX_RESET		  (17)
#define MSM_GPIO_SDEX_CLK		(70)
#define MSM_GPIO_SDEX_IRQ		(80)

static uint32_t mn66831_msm_gpio_config_data[] = {
	GPIO_CFG(MSM_GPIO_SDEX_RESET, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(MSM_GPIO_SDEX_CLK, 2, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL, GPIO_CFG_16MA),
	GPIO_CFG(MSM_GPIO_SDEX_IRQ, 0, GPIO_CFG_INPUT,
			GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
};

static uint32_t mn66831_msm_gpio_init( void )
{
	uint32_t rc = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(mn66831_msm_gpio_config_data); i++) {
		gpio_tlmm_config(mn66831_msm_gpio_config_data[i], 0);
	}

	rc = gpio_request(MSM_GPIO_SDEX_RESET, "sdex_reset");
	if (rc) {
		pr_err("%s: gpio_request(%d) = %d \n",
				__func__, MSM_GPIO_SDEX_RESET, rc);
		goto fail_gpio_reset;
	}

	rc = gpio_request(MSM_GPIO_SDEX_CLK, "sdex_clk");
	if (rc) {
		pr_err("%s: gpio_request(%d) = %d \n",
				__func__, MSM_GPIO_SDEX_CLK, rc);
		goto fail_gpio_clk;
	}

	rc = gpio_request(MSM_GPIO_SDEX_IRQ, "sdex_irq");
	if (rc) {
		pr_err("%s: gpio_request(%d) = %d \n",
				__func__, MSM_GPIO_SDEX_IRQ, rc);
		goto fail_gpio_irq;
	}

	gpio_set_value_cansleep(MSM_GPIO_SDEX_RESET, 1);
	return rc;

 fail_gpio_irq:
	gpio_free(MSM_GPIO_SDEX_CLK);

 fail_gpio_clk:
	gpio_free(MSM_GPIO_SDEX_RESET);

 fail_gpio_reset:
	return rc;
}

static uint32_t mn66831_msm_gpio_deinit( void )
{
	gpio_set_value_cansleep(MSM_GPIO_SDEX_RESET, 0);

	gpio_free(MSM_GPIO_SDEX_IRQ);
	gpio_free(MSM_GPIO_SDEX_CLK);
	gpio_free(MSM_GPIO_SDEX_RESET);
	return 0;
}

static struct regulator *vreg_s4, *vreg_l7, *vreg_l9, *vreg_l6;

static uint32_t mn66831_msm_vreg_init( void )
{
	uint32_t rc = -1;

	/* VREG_S4 - Always On - */
	vreg_s4 = regulator_get(NULL, "8921_s4");
	if (IS_ERR(vreg_s4)) {
		pr_err("%s: Unable to get %s (%ld)\n", __func__,
				"8921_s4", PTR_ERR(vreg_s4));
		rc = PTR_ERR(vreg_s4);
		goto out;
	}

	rc = regulator_set_voltage(vreg_s4, 1800000, 1800000);
	if (rc) {
		pr_err("%s: regulator_set_voltage(%s) = %d \n",
				__func__, "8921_s4", rc);
		goto put_vreg_s4;
	}

	rc = regulator_enable(vreg_s4);
	if (rc) {
		pr_err("%s: regulator_enable(%s) = %d \n",
				__func__, "8921_s4", rc);
		goto put_vreg_s4;
	}

	/* VREG_L7 - Always On - */
	vreg_l7 = regulator_get(NULL, "8921_l7");
	if (IS_ERR(vreg_l7)) {
		pr_err("%s: Unable to get %s (%ld)\n", __func__,
				"8921_l7", PTR_ERR(vreg_l7));
		rc = PTR_ERR(vreg_l7);
		goto disable_vreg_s4;
	}

	rc = regulator_set_voltage(vreg_l7, 2950000, 2950000);
	if (rc) {
		pr_err("%s: regulator_set_voltage(%s) = %d \n",
				__func__, "8921_l7", rc);
		goto put_vreg_l7;
	}

	rc = regulator_enable(vreg_l7);
	if (rc) {
		pr_err("%s: regulator_enable(%s) = %d \n",
				__func__, "8921_l7", rc);
		goto put_vreg_l7;
	}

	/* VREG_L9 */
	vreg_l9 = regulator_get(NULL, "8921_l9");
	if (IS_ERR(vreg_l9)) {
		pr_err("%s: Unable to get %s (%ld)\n", __func__,
				"8921_l9", PTR_ERR(vreg_l9));
		rc = PTR_ERR(vreg_l9);
		goto disable_vreg_l7;
	}

	rc = regulator_set_voltage(vreg_l9, 2850000, 2850000);
	if (rc) {
		pr_err("%s: regulator_set_voltage(%s) = %d \n",
				__func__, "8921_l9", rc);
		goto put_vreg_l9;
	}

	rc = regulator_enable(vreg_l9);
	if (rc) {
		pr_err("%s: regulator_enable(%s) = %d \n",
				__func__, "8921_l9", rc);
		goto put_vreg_l9;
	}

	udelay(100);

	/* VREG_L6 */
	vreg_l6 = regulator_get(NULL, "8921_l6");
	if (IS_ERR(vreg_l6)) {
		pr_err("%s: Unable to get %s (%ld)\n", __func__,
				"8921_l6", PTR_ERR(vreg_l6));
		rc = PTR_ERR(vreg_l6);
		goto disable_vreg_l9;
	}

	rc = regulator_set_voltage(vreg_l6, 2950000, 2950000);
	if (rc) {
		pr_err("%s: regulator_set_voltage(%s) = %d \n",
				__func__, "8921_l6", rc);
		goto put_vreg_l6;
	}

	return 0;

 put_vreg_l6:
	regulator_put(vreg_l6);
 disable_vreg_l9:
	regulator_disable(vreg_l9);
 put_vreg_l9:
	regulator_put(vreg_l9);
 disable_vreg_l7:
	regulator_disable(vreg_l7);
 put_vreg_l7:
	regulator_put(vreg_l7);
 disable_vreg_s4:
	regulator_disable(vreg_s4);
 put_vreg_s4:
	regulator_put(vreg_s4);
 out:
	return rc;
}

static uint32_t mn66831_msm_vreg_deinit( void )
{
	uint32_t rc = 0;

	rc = regulator_disable(vreg_l6);
	if (rc) {
		pr_err("%s: regulator_disable(%s) = %d \n",
				__func__, "8058_l6", rc);
	}
	regulator_put(vreg_l6);

	rc = regulator_disable(vreg_l9);
	if (rc) {
		pr_err("%s: regulator_disable(%s) = %d \n",
				__func__, "8058_l9", rc);
	}
	regulator_put(vreg_l9);

	rc = regulator_disable(vreg_l7);
	if (rc) {
		pr_err("%s: regulator_disable(%s) = %d \n",
				__func__, "8058_l7", rc);
	}
	regulator_put(vreg_l7);

	rc = regulator_disable(vreg_s4);
	if (rc) {
		pr_err("%s: regulator_disable(%s) = %d \n",
				__func__, "8058_s4", rc);
	}
	regulator_put(vreg_s4);
	return rc;
}

static uint32_t mn66831_msm_setup_vreg(int enable)
{
	uint32_t rc = 0;

	if (enable) {
		rc = regulator_enable(vreg_l6);
		if (rc) {
			pr_err("%s: regulator_enable(%s) = %d \n",
					__func__, "8058_l6", rc);
		}
	} else {
		rc = regulator_disable(vreg_l6);
		if (rc) {
			pr_err("%s: regulator_disable(%s) = %d \n",
					__func__, "8058_l6", rc);
		}
	}

	return rc;
}

static uint32_t
mn66831_msm_setup_power(struct device *dv, unsigned int mode)
{
	uint32_t rc = 0;

	pr_info("%s: mode=%u\n", __func__, mode);

	switch (mode) {
	case MN66831_HOST_INIT:
		rc = mn66831_msm_vreg_init();
		if (rc) {
			pr_err("%s: vreg_init = %d \n", __func__, rc);
			break;
		}

		rc = mn66831_msm_gpio_init();
		if (rc) {
			pr_err("%s: gpio_init = %d \n", __func__, rc);
			mn66831_msm_vreg_deinit();
			break;
		}

		rc = mn66831_msm_clock_init();
		if (!rc) {
			mn66831_msm_setup_clock(0); /* DISABLE */
			mdelay(1);
			rc = mn66831_msm_setup_clock(1); /* ENABLE */
			mdelay(1);
		}
		if (rc) {
			pr_err("%s: setup_clock = %d \n", __func__, rc);
			mn66831_msm_gpio_deinit();
			mn66831_msm_vreg_deinit();
		}
		break;

	case MN66831_HOST_EXIT:
		rc = mn66831_msm_setup_clock(0); /* DISABLE */
		if (rc) {
			pr_err("%s: setup_clock = %d \n", __func__, rc);
		}

		rc = mn66831_msm_gpio_deinit();
		if (rc) {
			pr_err("%s: gpio_deinit = %d \n", __func__, rc);
		}

		rc = mn66831_msm_vreg_deinit();
		if (rc) {
			pr_err("%s: vreg_deinit = %d \n", __func__, rc);
		}
		break;

	case MN66831_HOST_CLK_ON:
		rc = mn66831_msm_setup_clock(1); /* ENABLE */
		if (rc) {
			pr_err("%s: setup_clock = %d \n", __func__, rc);
		}
		break;

	case MN66831_HOST_CLK_OFF:
		rc = mn66831_msm_setup_clock(0); /* DISABLE */
		if (rc) {
			pr_err("%s: setup_clock = %d \n", __func__, rc);
		}
		break;

	case MN66831_CARD_ON:
		rc = mn66831_msm_setup_vreg(1); /* ENABLE */
		if (rc) {
			pr_err("%s: setup_vreg = %d \n", __func__, rc);
		}
		break;

	case MN66831_CARD_OFF:
		rc = mn66831_msm_setup_vreg(0); /* DISABLE */
		if (rc) {
			pr_err("%s: setup_vreg = %d \n", __func__, rc);
		}
		break;

	default:
		rc = -ENOENT;
	}

	pr_debug("%s: mode=%u, rc=%u\n", __func__, mode, rc);
	return rc;
}

static unsigned int mn66831_msm_slot_status(struct device *dev)
{
	int status;

	status = gpio_request(
				PM8921_GPIO_PM_TO_SYS(26),
				 "SD_HW_Detect");
	if (status) {
		pr_err("%s:Failed to request GPIO %d\n", __func__,
				PM8921_GPIO_PM_TO_SYS(26));
		goto out;
	}

	status = gpio_direction_input(
			PM8921_GPIO_PM_TO_SYS(26));
	if (status) {
		pr_err("%s:gpio_direction_input failed. GPIO=%d\n",
				__func__, PM8921_GPIO_PM_TO_SYS(26));
	} else {
		status = !(gpio_get_value_cansleep(
			PM8921_GPIO_PM_TO_SYS(26)));
	}

	gpio_free(PM8921_GPIO_PM_TO_SYS(26));

 out:
	return (unsigned int) status;
}

static struct resource mn66831_msm_resources[] = {
	{
		.start	= MSM_GPIO_TO_INT(MSM_GPIO_SDEX_IRQ),
		.end	= MSM_GPIO_TO_INT(MSM_GPIO_SDEX_IRQ),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct mmc_platform_data mn66831_msm_data = {
	.ocr_mask      = MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd = mn66831_msm_setup_power,
	.mmc_bus_width = MMC_CAP_4_BIT_DATA,
	.status        = mn66831_msm_slot_status,
	.status_irq    = PM8921_GPIO_IRQ(PM8921_IRQ_BASE, 26),
	.irq_flags     = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
};

struct platform_device mn66831_msm_device = {
	.name          = "external_sd",
	.id            = 0,
	.num_resources = ARRAY_SIZE(mn66831_msm_resources),
	.resource      = mn66831_msm_resources,
	.dev = {
		.platform_data = &mn66831_msm_data,
	},
};
#endif /* CONFIG_FEATURE_NCMC_CPRM */

static void __init msm8960_sim_init(void)
{
    uint32_t hw_rev = 0;

	struct msm_watchdog_pdata *wdog_pdata = (struct msm_watchdog_pdata *)
		&msm8960_device_watchdog.dev.platform_data;

	wdog_pdata->bark_time = 15000;
	BUG_ON(msm_rpm_init(&msm_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(msm_rpmrs_levels,
				ARRAY_SIZE(msm_rpmrs_levels)));
	regulator_suppress_info_printing();

#ifdef NCM_FUNCTION
    hw_rev = hw_revision_read();

    if (hw_rev == HW_REV_5P0) {
        platform_device_register(&msm8960_device_rpm_regulator);
    } else {
        platform_device_register(&nc_msm8960_device_rpm_regulator_oem);
    }
#else
	platform_device_register(&msm8960_device_rpm_regulator);
#endif /* NCM_FUNCTION */

	msm_clock_init(&msm8960_clock_init_data);
	msm8960_device_ssbi_pm8921.dev.platform_data =
				&msm8960_ssbi_pm8921_pdata;
#ifdef NCM_FUNCTION
  pm8921_platform_data.num_regulators = nc_msm_pm8921_regulator_pdata_len;
#else
	pm8921_platform_data.num_regulators = msm_pm8921_regulator_pdata_len;
#endif /* NCM_FUNCTION */

	/* Simulator supports a QWERTY keypad */
	pm8921_platform_data.keypad_pdata = &keypad_data_sim;

	msm8960_device_otg.dev.platform_data = &msm_otg_pdata;
	gpiomux_init();
	msm8960_i2c_init();
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_spm_l2_init(msm_spm_l2_data);
	msm8960_init_buses();
	platform_add_devices(common_devices, ARRAY_SIZE(common_devices));
	pm8921_gpio_mpp_init();
	platform_add_devices(sim_devices, ARRAY_SIZE(sim_devices));
	acpuclk_init(&acpuclk_8960_soc_data);

	msm8960_device_qup_spi_gsbi9.dev.platform_data =
				&msm8960_qup_spi_gsbi9_pdata;

#ifdef CONFIG_KS8851
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
#endif
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
	spi_register_board_info(synaptics_ncm_spi_board_info,
		ARRAY_SIZE(synaptics_ncm_spi_board_info));
#endif /* defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM) */

	msm8960_init_mmc();

#ifdef CONFIG_FEATURE_NCMC_CPRM
    platform_device_register(&mn66831_msm_device);
#endif /* CONFIG_FEATURE_NCMC_CPRM */

	msm_fb_add_devices();
	slim_register_board_info(msm_slim_devices,
		ARRAY_SIZE(msm_slim_devices));
	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	msm_pm_set_rpm_wakeup_irq(RPM_APCC_CPU0_WAKE_UP_IRQ);
	msm_cpuidle_set_states(msm_cstates, ARRAY_SIZE(msm_cstates),
				msm_pm_data);
	BUG_ON(msm_pm_boot_init(MSM_PM_BOOT_CONFIG_TZ, NULL));
}

static void __init msm8960_rumi3_init(void)
{
    uint32_t hw_rev = 0;

	BUG_ON(msm_rpm_init(&msm_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(msm_rpmrs_levels,
				ARRAY_SIZE(msm_rpmrs_levels)));
	regulator_suppress_info_printing();

#ifdef NCM_FUNCTION
    hw_rev = hw_revision_read();

    if (hw_rev == HW_REV_5P0) {
        platform_device_register(&msm8960_device_rpm_regulator);
    } else {
        platform_device_register(&nc_msm8960_device_rpm_regulator_oem);
    }
#else
	platform_device_register(&msm8960_device_rpm_regulator);
#endif /* NCM_FUNCTION */

	msm_clock_init(&msm8960_dummy_clock_init_data);
	gpiomux_init();
	msm8960_device_ssbi_pm8921.dev.platform_data =
				&msm8960_ssbi_pm8921_pdata;
#ifdef NCM_FUNCTION
  pm8921_platform_data.num_regulators = nc_msm_pm8921_regulator_pdata_len;
#else
	pm8921_platform_data.num_regulators = msm_pm8921_regulator_pdata_len;
#endif /* NCM_FUNCTION */
	msm8960_device_qup_spi_gsbi9.dev.platform_data =
				&msm8960_qup_spi_gsbi9_pdata;
#ifdef CONFIG_KS8851
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
#endif
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
	spi_register_board_info(synaptics_ncm_spi_board_info,
		ARRAY_SIZE(synaptics_ncm_spi_board_info));
#endif /* defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM) */

	msm8960_i2c_init();
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_spm_l2_init(msm_spm_l2_data);
	platform_add_devices(common_devices, ARRAY_SIZE(common_devices));
	pm8921_gpio_mpp_init();
	platform_add_devices(rumi3_devices, ARRAY_SIZE(rumi3_devices));
	msm8960_init_mmc();

#ifdef CONFIG_FEATURE_NCMC_CPRM
    platform_device_register(&mn66831_msm_device);
#endif /* CONFIG_FEATURE_NCMC_CPRM */

	register_i2c_devices();
	msm_fb_add_devices();
	slim_register_board_info(msm_slim_devices,
		ARRAY_SIZE(msm_slim_devices));
	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	msm_pm_set_rpm_wakeup_irq(RPM_APCC_CPU0_WAKE_UP_IRQ);
	msm_cpuidle_set_states(msm_cstates, ARRAY_SIZE(msm_cstates),
				msm_pm_data);
	BUG_ON(msm_pm_boot_init(MSM_PM_BOOT_CONFIG_TZ, NULL));
}

static void __init msm8960_cdp_init(void)
{
    uint32_t hw_rev = 0;

	if (meminfo_init(SYS_MEMORY, SZ_256M) < 0)
		pr_err("meminfo_init() failed!\n");

	BUG_ON(msm_rpm_init(&msm_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(msm_rpmrs_levels,
				ARRAY_SIZE(msm_rpmrs_levels)));

	pmic_reset_irq = PM8921_IRQ_BASE + PM8921_RESOUT_IRQ;
	regulator_suppress_info_printing();
	if (msm_xo_init())
		pr_err("Failed to initialize XO votes\n");

#ifdef NCM_FUNCTION
    hw_rev = hw_revision_read();
    
    if (hw_rev == HW_REV_5P0) {
        platform_device_register(&msm8960_device_rpm_regulator);
    } else {
        platform_device_register(&nc_msm8960_device_rpm_regulator_oem);
    }
#else
	platform_device_register(&msm8960_device_rpm_regulator);
#endif /* NCM_FUNCTION */

	msm_clock_init(&msm8960_clock_init_data);
	if (machine_is_msm8960_liquid())
		msm_otg_pdata.mhl_enable = true;
	msm8960_device_otg.dev.platform_data = &msm_otg_pdata;
#ifdef CONFIG_USB_EHCI_MSM_HSIC
	if (machine_is_msm8960_liquid()) {
		if (SOCINFO_VERSION_MAJOR(socinfo_get_version()) >= 2)
			msm_hsic_pdata.hub_reset = HSIC_HUB_RESET_GPIO;
	}
#endif
	msm_device_hsic_host.dev.platform_data = &msm_hsic_pdata;
	gpiomux_init();
	if (machine_is_msm8960_liquid())
		pm8921_platform_data.keypad_pdata = &keypad_data_liquid;
	msm8960_device_qup_spi_gsbi9.dev.platform_data =
				&msm8960_qup_spi_gsbi9_pdata;

#ifdef CONFIG_KS8851
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
#endif

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM)
	spi_register_board_info(synaptics_ncm_spi_board_info,
		ARRAY_SIZE(synaptics_ncm_spi_board_info));
#endif /* defined(CONFIG_TOUCHSCREEN_SYNAPTICS_NCM) */

	msm8960_device_ssbi_pm8921.dev.platform_data =
				&msm8960_ssbi_pm8921_pdata;
#ifdef NCM_FUNCTION
  pm8921_platform_data.num_regulators = nc_msm_pm8921_regulator_pdata_len;
#else
	pm8921_platform_data.num_regulators = msm_pm8921_regulator_pdata_len;
#endif /* NCM_FUNCTION */
	msm8960_i2c_init();
	msm8960_gfx_init();
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_spm_l2_init(msm_spm_l2_data);
	msm8960_init_buses();
	platform_add_devices(msm_footswitch_devices,
		msm_num_footswitch_devices);
	if (machine_is_msm8960_liquid())
		platform_device_register(&msm8960_device_ext_3p3v_vreg);
	platform_add_devices(common_devices, ARRAY_SIZE(common_devices));
	pm8921_gpio_mpp_init();
	platform_add_devices(cdp_devices, ARRAY_SIZE(cdp_devices));
	msm8960_init_hsic();
	msm8960_init_cam();
	msm8960_init_mmc();

#ifdef CONFIG_FEATURE_NCMC_CPRM
    platform_device_register(&mn66831_msm_device);
#endif /* CONFIG_FEATURE_NCMC_CPRM */

	acpuclk_init(&acpuclk_8960_soc_data);
	if (machine_is_msm8960_liquid())
		mxt_init_hw_liquid();
	register_i2c_devices();
	msm_fb_add_devices();
	slim_register_board_info(msm_slim_devices,
		ARRAY_SIZE(msm_slim_devices));
	msm8960_init_dsps();
	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	msm_pm_set_rpm_wakeup_irq(RPM_APCC_CPU0_WAKE_UP_IRQ);
	msm_cpuidle_set_states(msm_cstates, ARRAY_SIZE(msm_cstates),
				msm_pm_data);
	change_memory_power = &msm8960_change_memory_power;
	BUG_ON(msm_pm_boot_init(MSM_PM_BOOT_CONFIG_TZ, NULL));

	if (PLATFORM_IS_CHARM25())
		platform_add_devices(mdm_devices, ARRAY_SIZE(mdm_devices));
}

MACHINE_START(MSM8960_SIM, "QCT MSM8960 SIMULATOR")
	.map_io = msm8960_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_sim_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

MACHINE_START(MSM8960_RUMI3, "QCT MSM8960 RUMI3")
	.map_io = msm8960_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_rumi3_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

MACHINE_START(MSM8960_CDP, "QCT MSM8960 CDP")
	.map_io = msm8960_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_cdp_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

MACHINE_START(MSM8960_MTP, "QCT MSM8960 MTP")
	.map_io = msm8960_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_cdp_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

MACHINE_START(MSM8960_FLUID, "QCT MSM8960 FLUID")
	.map_io = msm8960_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_cdp_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

MACHINE_START(MSM8960_LIQUID, "QCT MSM8960 LIQUID")
	.map_io = msm8960_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_cdp_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

#ifdef CONFIG_ARCH_MSM8930
MACHINE_START(MSM8930_CDP, "QCT MSM8930 CDP")
	.map_io = msm8930_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_cdp_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

MACHINE_START(MSM8930_MTP, "QCT MSM8930 MTP")
	.map_io = msm8930_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_cdp_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END

MACHINE_START(MSM8930_FLUID, "QCT MSM8930 FLUID")
	.map_io = msm8930_map_io,
	.reserve = msm8960_reserve,
	.init_irq = msm8960_init_irq,
	.timer = &msm_timer,
	.init_machine = msm8960_cdp_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = msm8960_early_memory,
MACHINE_END
#endif
