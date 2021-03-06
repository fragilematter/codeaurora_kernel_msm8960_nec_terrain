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
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <asm/mach-types.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <linux/ion.h>
#include <mach/ion.h>

#include "devices.h"
#include "board-8960.h"
#include <linux/msm_mdp.h>

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
	#if defined (LCD_DEVICE_D121M_PT)
	    /* prim = 1280 x 800 x 4(RGBA8888) x 3(pages) */
    	#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(800, 32)*4*3)
	#elif defined (LCD_DEVICE_D121F_PT)
	    /* prim = 1280 x 720 x 4(RGBA8888) x 3(pages) */
    	#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(720, 32)*4*3)
	#elif defined (LCD_DEVICE_G121S_PT)
	    /* prim = 1280 x 720 x 4(RGBA8888) x 3(pages) */
    	#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(720, 32)*4*3)
	#elif defined (LCD_DEVICE_TOSHIBA_HD_PT)
		/* prim = 1280 x 720 x 4(RGBA8888) x 3(pages) */
		#define MSM_FB_PRIM_BUF_SIZE 0xA8C000 /*(1280*ALIGN(720, 32)*4*3)*/
	#elif defined (LCD_DEVICE_RUBY_PT)
	    /* prim = 640 x 480 x 4(RGBA8888) x 3(pages) */
	#define MSM_FB_PRIM_BUF_SIZE (640*ALIGN(452, 32)*4*3)
	#else
		#define MSM_FB_PRIM_BUF_SIZE 0x720000
	#endif
#else
	#if defined (LCD_DEVICE_D121M_PT)
	    /* prim = 1280 x 800 x 4(RGBA8888) x 2(pages) */
    	#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(800, 32)*4*2)
	#elif defined (LCD_DEVICE_D121F_PT)
	    /* prim = 1280 x 720 x 4(RGBA8888) x 2(pages) */
    	#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(720, 32)*4*2)
	#elif defined (LCD_DEVICE_G121S_PT)
	    /* prim = 1280 x 720 x 4(RGBA8888) x 2(pages) */
    	#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(720, 32)*4*2)
	#elif defined (LCD_DEVICE_TOSHIBA_HD_PT)
	    /* prim = 1280 x 720 x 4(RGBA8888) x 2(pages) */
    	#define MSM_FB_PRIM_BUF_SIZE (1280*ALIGN(720, 32)*4*2)
	#elif defined (LCD_DEVICE_RUBY_PT)
	    /* prim = 640 x 480 x 4(RGBA8888) x 2(pages) */
	#define MSM_FB_PRIM_BUF_SIZE (640*ALIGN(452, 32)*4*2)
	#else
		/* prim = 608 x 1024 x 4(bpp) x 2(pages) */
		#define MSM_FB_PRIM_BUF_SIZE 0x4C0000
	#endif
#endif

#ifdef CONFIG_FB_MSM_MIPI_DSI

#define MIPI_DSI_WRITEBACK_SIZE (1280 * 720 * 3 * 2)
#else
#define MIPI_DSI_WRITEBACK_SIZE 0
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
#define MSM_FB_EXT_BUF_SIZE \
		(roundup((1920 * 1088 * 2), 4096) * 1) /* 2 bpp x 1 page */
#elif defined(CONFIG_FB_MSM_TVOUT)
#define MSM_FB_EXT_BUF_SIZE \
		(roundup((720 * 576 * 2), 4096) * 2) /* 2 bpp x 2 pages */
#else
#define MSM_FB_EXT_BUF_SIZE	0
#endif

/* Note: must be multiple of 4096 */
#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE + MSM_FB_EXT_BUF_SIZE, 4096)

#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1920 * 1200 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY0_WRITEBACK */

#ifdef CONFIG_FB_MSM_OVERLAY1_WRITEBACK
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE roundup((1920 * 1088 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY1_WRITEBACK */

#define MDP_VSYNC_GPIO 0

#define MIPI_CMD_NT35560_VGA_PANEL_NAME	        "mipi_cmd_nt35560_vga"
#define MIPI_VIDEO_NT35560_WVGA_PANEL_NAME	"mipi_video_nt35560_wvga"
#define MIPI_CMD_NOVATEK_QHD_PANEL_NAME	        "mipi_cmd_novatek_qhd"
#define MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME	"mipi_video_novatek_qhd"
#define MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME	"mipi_video_toshiba_wsvga"
#define MIPI_VIDEO_TOSHIBA_WUXGA_PANEL_NAME	"mipi_video_toshiba_wuxga"
#define MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME	"mipi_video_chimei_wxga"
#define MIPI_VIDEO_SIMULATOR_VGA_PANEL_NAME	"mipi_video_simulator_vga"
#define MIPI_CMD_RENESAS_FWVGA_PANEL_NAME	"mipi_cmd_renesas_fwvga"
#define HDMI_PANEL_NAME	"hdmi_msm"
#define TVOUT_PANEL_NAME	"tvout_msm"

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
unsigned char hdmi_is_primary = 1;
#else
unsigned char hdmi_is_primary;
#endif

static struct resource msm_fb_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};

#ifndef CONFIG_FB_MSM_MIPI_PANEL_DETECT
static void set_mdp_clocks_for_wuxga(void);
#endif

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

		if (!strncmp(name, MIPI_VIDEO_TOSHIBA_WUXGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_TOSHIBA_WUXGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN))) {
			set_mdp_clocks_for_wuxga();
			return 0;
		}
		if (!strncmp(name,MIPI_CMD_NT35560_VGA_PANEL_NAME ,
				strnlen(MIPI_CMD_NT35560_VGA_PANEL_NAME ,
					PANEL_NAME_MAX_LEN)))
			return 0;
		if (!strncmp(name, MIPI_VIDEO_NT35560_WVGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_NT35560_WVGA_PANEL_NAME,
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

#if defined(LCD_DEVICE_D121M_PT) || defined(LCD_DEVICE_D121F_PT) || defined(LCD_DEVICE_G121S_PT) || defined (LCD_DEVICE_TOSHIBA_HD_PT)
static void mipi_dsi_panel_pwm_cfg(void)
{
	int rc;
	static int mipi_dsi_panel_gpio_configured;
	static struct pm_gpio pwm_enable = {
		.direction        = PM_GPIO_DIR_OUT,
		.output_buffer    = PM_GPIO_OUT_BUF_CMOS,
		.output_value     = 1,
		.pull             = PM_GPIO_PULL_NO,
		.vin_sel          = PM_GPIO_VIN_VPH,
		.out_strength     = PM_GPIO_STRENGTH_HIGH,
		.function         = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol      = 0,
		.disable_pin      = 0,
	};
	static struct pm_gpio pwm_mode = {
		.direction        = PM_GPIO_DIR_OUT,
		.output_buffer    = PM_GPIO_OUT_BUF_CMOS,
		.output_value     = 0,
		.pull             = PM_GPIO_PULL_NO,
		.vin_sel          = PM_GPIO_VIN_S4,
		.out_strength     = PM_GPIO_STRENGTH_HIGH,
		.function         = PM_GPIO_FUNC_2,
		.inv_int_pol      = 0,
		.disable_pin      = 0,
	};

	if (mipi_dsi_panel_gpio_configured == 0) {
		/* pm8xxx: gpio-21, Backlight Enable */
		rc = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(21),
					&pwm_enable);
		if (rc != 0)
			pr_err("%s: pwm_enabled failed\n", __func__);

		/* pm8xxx: gpio-24, Bl: Off, PWM mode */
		rc = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(24),
					&pwm_mode);
		if (rc != 0)
			pr_err("%s: pwm_mode failed\n", __func__);

		mipi_dsi_panel_gpio_configured++;
	}
}
#else
#endif


static bool dsi_power_on;


/* From START 1.7 Merge */

void dw8402a_lcd_bl_on(void)
{
    static int gpio24 = PM8921_GPIO_PM_TO_SYS(24);
    gpio_set_value_cansleep(gpio24, 1);
}
EXPORT_SYMBOL(dw8402a_lcd_bl_on);

void dw8402a_lcd_bl_off(void)
{
    static int gpio24 = PM8921_GPIO_PM_TO_SYS(24);
    gpio_set_value_cansleep(gpio24, 0);
}
EXPORT_SYMBOL(dw8402a_lcd_bl_off);






#if defined(LCD_DEVICE_D121M_PT) || defined(LCD_DEVICE_D121F_PT) || defined(LCD_DEVICE_G121S_PT) || defined (LCD_DEVICE_TOSHIBA_HD_PT) || defined (LCD_DEVICE_RUBY_PT)
struct lcd_gpio_info {
	unsigned no;
	char *name;
};

#ifndef LCD_DEVICE_RUBY_PT
static int mipi_dsi_panel_power(int on)
{

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
static int mipi_dsi_panel_power(int on)
{
	static struct regulator *vreg_lvs5 = NULL;
	static struct regulator *vreg_l8   = NULL;
	static struct regulator *vreg_l2   = NULL;
	int rc;
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

	printk(KERN_DEBUG "[In]%s(%d).\n", __func__, on);

	if(!dsi_power_on) {
		if (vreg_l2 == NULL) {
			/* VREG_L2 */
			vreg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_vdda");
			if (IS_ERR(vreg_l2)) {
				pr_debug("%s: VREG_L2 failed\n", __func__);
				rc = PTR_ERR(vreg_l2);
				return rc;
			}

			rc = regulator_set_voltage(vreg_l2, 1200000, 1200000);
			if (rc) {
				pr_err("set_voltage l2 failed, rc=%d\n", rc);
				goto out;
			}

			rc = regulator_set_optimum_mode(vreg_l2, 100000);
			if (rc < 0) {
				pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
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
		}

		if (vreg_lvs5 == NULL) {
			vreg_lvs5 = regulator_get(NULL, "8921_lvs5");
			if (IS_ERR(vreg_lvs5)) {
				pr_debug("%s: VREG_LVS5 failed\n", __func__);
				rc = PTR_ERR(vreg_lvs5);
				return rc;
			}
		}

		gpio36 = PM8921_GPIO_PM_TO_SYS(36);
		rc = gpio_request(gpio36, "disp_rst_n");
		if (rc) {
			pr_err("request gpio 43 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = pm8xxx_gpio_config(gpio36, &gpio36_param);
		if (rc) {
			pr_err("gpio_config 36 failed (1), rc=%d\n", rc);
			return -EINVAL;
		}
		dsi_power_on=1;
	}

	if(on) {
		rc = regulator_enable(vreg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			goto out;
		}
		rc = regulator_enable(vreg_l8);
		if (rc) {
			printk("[reiji]vreg_l8 enable failed\n");
			goto out;
		}

		rc = regulator_enable(vreg_lvs5);
		if (rc) {
			printk("[reiji]vreg_lvs5 enable failed\n");
			goto out;
		}

		msleep(20);

		rc = gpio_direction_output(gpio36, 0);
		if (rc) {
			printk("[reiji]gpio36 output failed.\n");
		}

		rc = gpio_direction_output(gpio36, 1);
		if (rc) {
			printk("[reiji]gpio36 output failed.\n");
		}
		msleep(120);
	}
	else {
		rc = regulator_disable(vreg_l2);
		if (rc) {
			printk("[reiji]vreg_l2 enable failed\n");
			goto out;
		}

		rc = gpio_direction_output(gpio36, 0);
		if (rc) {
			printk("[reiji]gpio36 output failed.\n");
		}

		msleep(10);

		rc = regulator_disable(vreg_lvs5);
		if (rc) {
			printk("[reiji]vreg_lvs5 enable failed\n");
			goto out;
		}

		rc = regulator_disable(vreg_l8);
		if (rc) {
			printk("[reiji]vreg_l8 enable failed\n");
			goto out;
		}
	}
	printk(KERN_DEBUG "[Out]%s.\n", __func__);
	return 0;

out:
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

	vreg_lvs5 = NULL;
	vreg_l8 = NULL;
	return rc;
}
#endif

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

	mipi_dsi_panel_pwm_cfg();
	pr_debug("%s: on=%d\n", __func__, on);

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

	pr_debug("%s: state : %d\n", __func__, on);

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

	pr_debug("%s: on=%d\n", __func__, on);

	if (machine_is_msm8960_liquid())
		ret = mipi_dsi_liquid_panel_power(on);
	else
		ret = mipi_dsi_cdp_panel_power(on);

	return ret;
}
#endif

static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio = MDP_VSYNC_GPIO,
	.dsi_power_save = mipi_dsi_panel_power,
};

#ifdef CONFIG_MSM_BUS_SCALING

static struct msm_bus_vectors rotator_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors rotator_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1024 * 600 * 4 * 2 * 60),
		.ib  = (1024 * 600 * 4 * 2 * 60 * 1.5),
	},
};

static struct msm_bus_vectors rotator_vga_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (640 * 480 * 2 * 2 * 30),
		.ib  = (640 * 480 * 2 * 2 * 30 * 1.5),
	},
};
static struct msm_bus_vectors rotator_720p_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1280 * 736 * 2 * 2 * 30),
		.ib  = (1280 * 736 * 2 * 2 * 30 * 1.5),
	},
};

static struct msm_bus_vectors rotator_1080p_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1920 * 1088 * 2 * 2 * 30),
		.ib  = (1920 * 1088 * 2 * 2 * 30 * 1.5),
	},
};

static struct msm_bus_paths rotator_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(rotator_init_vectors),
		rotator_init_vectors,
	},
	{
		ARRAY_SIZE(rotator_ui_vectors),
		rotator_ui_vectors,
	},
	{
		ARRAY_SIZE(rotator_vga_vectors),
		rotator_vga_vectors,
	},
	{
		ARRAY_SIZE(rotator_720p_vectors),
		rotator_720p_vectors,
	},
	{
		ARRAY_SIZE(rotator_1080p_vectors),
		rotator_1080p_vectors,
	},
};

struct msm_bus_scale_pdata rotator_bus_scale_pdata = {
	rotator_bus_scale_usecases,
	ARRAY_SIZE(rotator_bus_scale_usecases),
	.name = "rotator",
};

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
#if defined (LCD_DEVICE_D121M_PT)
	128000000,
	160000000,
	177780000,
	200000000,
#elif defined (LCD_DEVICE_D121F_PT)
	128000000,
	160000000,
	177780000,
	200000000,
#elif defined (LCD_DEVICE_G121S_PT)
	128000000,
	160000000,
	177780000,
	200000000,
#elif defined (LCD_DEVICE_TOSHIBA_HD_PT)
	128000000,
	160000000,
	177780000,
	200000000,
#elif defined (LCD_DEVICE_RUBY_PT)
	128000000,
	160000000,
	177780000,
	200000000,
#else /* !CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
	85330000,
	128000000,
	160000000,
	200000000,
#endif /* !CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
};
#else
static int mdp_core_clk_rate_table[] = {
	85330000,
	128000000,
	128000000,
	200000000,
	200000000,
};
#endif

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_VSYNC_GPIO,
#if defined (LCD_DEVICE_D121M_PT)
	.mdp_core_clk_rate = 128000000,
#elif defined (LCD_DEVICE_D121F_PT)
	.mdp_core_clk_rate = 128000000,
#elif defined (LCD_DEVICE_G121S_PT)
	.mdp_core_clk_rate = 128000000,
#elif defined (LCD_DEVICE_TOSHIBA_HD_PT)
	.mdp_core_clk_rate = 128000000,
#elif defined (LCD_DEVICE_RUBY_PT)
	.mdp_core_clk_rate = 128000000,
#else
	.mdp_core_clk_rate = 85330000,
#endif /* !CONFIG_FB_MSM_MIPI_NCMC_VIDEO_HD_PT_PANEL_DB */
	.mdp_core_clk_table = mdp_core_clk_rate_table,
	.num_mdp_clk = ARRAY_SIZE(mdp_core_clk_rate_table),
#ifdef CONFIG_MSM_BUS_SCALING
	.mdp_bus_scale_table = &mdp_bus_scale_pdata,
#endif
	.mdp_rev = MDP_REV_42,
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	.mem_hid = BIT(ION_CP_MM_HEAP_ID),
#else
	.mem_hid = MEMTYPE_EBI1,
#endif
	//.cont_splash_enabled = 0x01,
	.cont_splash_enabled = 0x00,	
};

#ifndef CONFIG_FB_MSM_MIPI_PANEL_DETECT
/**
 * Set MDP clocks to high frequency to avoid DSI underflow
 * when using high resolution 1200x1920 WUXGA panels
 */
static void set_mdp_clocks_for_wuxga(void)
{
	int i;

	mdp_ui_vectors[0].ab = 2000000000;
	mdp_ui_vectors[0].ib = 2000000000;
	mdp_vga_vectors[0].ab = 2000000000;
	mdp_vga_vectors[0].ib = 2000000000;
	mdp_720p_vectors[0].ab = 2000000000;
	mdp_720p_vectors[0].ib = 2000000000;
	mdp_1080p_vectors[0].ab = 2000000000;
	mdp_1080p_vectors[0].ib = 2000000000;

	mdp_pdata.mdp_core_clk_rate = 200000000;

	for (i = 0; i < ARRAY_SIZE(mdp_core_clk_rate_table); i++)
		mdp_core_clk_rate_table[i] = 200000000;

}
#endif

void __init msm8960_mdp_writeback(struct memtype_reserve* reserve_table)
{
	mdp_pdata.ov0_wb_size = MSM_FB_OVERLAY0_WRITEBACK_SIZE;
	mdp_pdata.ov1_wb_size = MSM_FB_OVERLAY1_WRITEBACK_SIZE;
#if defined(CONFIG_ANDROID_PMEM) && !defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov0_wb_size;
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov1_wb_size;
#endif
}


#if defined (LCD_DEVICE_D121M_PT)
static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas_d121m",
	.id = 0,
};
#elif defined (LCD_DEVICE_D121F_PT)
static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas_d121f",
	.id = 0,
};
#elif defined (LCD_DEVICE_G121S_PT)
static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas_g121s",
	.id = 0,
};
#else
static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas_hd",
	.id = 0,
};
#endif

static struct platform_device mipi_dsi_simulator_panel_device = {
	.name = "mipi_simulator",
	.id = 0,
};

#ifdef CONFIG_FB_MSM_MIPI_DSI_NT35560
static struct platform_device mipi_dsi_nt35560_panel_device = {
	.name = "mipi_nt35560",
	.id = 0,
};
#else

#endif /* NT35560 MIPI DSI  */

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
static int hdmi_gpio_config(int on);
static int hdmi_panel_power(int on);

static struct msm_hdmi_platform_data hdmi_msm_data = {
	.irq = HDMI_IRQ,
	.enable_5v = hdmi_enable_5v,
	.core_power = hdmi_core_power,
	.cec_power = hdmi_cec_power,
	.panel_power = hdmi_panel_power,
	.gpio_config = hdmi_gpio_config,
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

static struct platform_device wfd_device = {
	.name          = "msm_wfd",
	.id            = -1,
};
#endif

#ifdef CONFIG_MSM_BUS_SCALING

static int hdmi_panel_power(int on)
{
	int rc;

	pr_debug("%s: HDMI Core: %s\n", __func__, (on ? "ON" : "OFF"));
	rc = hdmi_core_power(on, 1);
	if (rc)
		rc = hdmi_cec_power(on);

	pr_debug("%s: HDMI Core: %s Success\n", __func__, (on ? "ON" : "OFF"));
	return rc;
}
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
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
		pr_debug("%s(on): success\n", __func__);
	} else {
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
}

static int hdmi_gpio_config(int on)
{
	int rc = 0;
	static int prev_on;

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(100, "HDMI_DDC_CLK");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_CLK", 100, rc);
			return rc;
		}
		rc = gpio_request(101, "HDMI_DDC_DATA");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_DATA", 101, rc);
			goto error1;
		}
		rc = gpio_request(102, "HDMI_HPD");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_HPD", 102, rc);
			goto error2;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(100);
		gpio_free(101);
		gpio_free(102);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;
	return 0;

error2:
	gpio_free(101);
error1:
	gpio_free(100);
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

void __init msm8960_init_fb(void)
{
	platform_device_register(&msm_fb_device);

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	platform_device_register(&wfd_panel_device);
	platform_device_register(&wfd_device);
#endif

	if (machine_is_msm8960_sim())
		platform_device_register(&mipi_dsi_simulator_panel_device);

	if (machine_is_msm8960_rumi3())
		platform_device_register(&mipi_dsi_renesas_panel_device);

	if (!machine_is_msm8960_sim() && !machine_is_msm8960_rumi3()) {
		platform_device_register(&mipi_dsi_novatek_panel_device);

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
		platform_device_register(&hdmi_msm_device);
#endif
	}
	#ifdef CONFIG_FB_MSM_MIPI_DSI_NT35560
		platform_device_register(&mipi_dsi_nt35560_panel_device);
	#else
		platform_device_register(&mipi_dsi_toshiba_panel_device);
	#endif /* NT35560 MIPI DSI */

	if (machine_is_msm8x60_rumi3()) {
		msm_fb_register_device("mdp", NULL);
		mipi_dsi_pdata.target_type = 1;
	} else
		msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#ifdef CONFIG_MSM_BUS_SCALING
#endif
}

void __init msm8960_allocate_fb_region(void)
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

void __init msm8960_set_display_params(char *prim_panel, char *ext_panel)
{
	if (strnlen(prim_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.prim_panel_name, prim_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.prim_panel_name %s\n",
			msm_fb_pdata.prim_panel_name);

		if (!strncmp((char *)msm_fb_pdata.prim_panel_name,
			HDMI_PANEL_NAME, strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
			pr_debug("HDMI is the primary display by"
				" boot parameter\n");
			hdmi_is_primary = 1;
		}
	}
	if (strnlen(ext_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.ext_panel_name, ext_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.ext_panel_name %s\n",
			msm_fb_pdata.ext_panel_name);
	}
}
