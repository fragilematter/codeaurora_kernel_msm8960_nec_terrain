/* Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
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
#include "mipi_renesas_d121f.h"

#include <mach/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/regulator/gpio-regulator.h>
#include <../../../arch/arm/mach-msm/board-8960.h>
#include <../../../arch/arm/mach-msm/devices.h>
#include <linux/ioport.h>

#include <linux/leds-lcd-common.h> /* For backlight */

/* GPIO Config */
#define MIPI_RENESAS_GPIO_DISP_RST "disp_rst_n"
#define MIPI_RENESAS_GPIO_5V_OE    "lcd_5v_oe"

extern boolean msm_fb_disable_sleep;

static struct regulator *vreg_lvs5 = NULL;
static struct regulator *vreg_l8   = NULL;
static struct regulator *vreg_l2   = NULL;
static int gpio36_request = 0;
static int gpio78_request = 0;

/* Global Variables */
static struct msm_panel_common_pdata *mipi_renesas_d121f_pdata;

static struct dsi_buf renesas_d121f_tx_buf;
static struct dsi_buf renesas_d121f_rx_buf;

static int ch_used[3];

/* Driver Internal Status */
static mipi_renesas_d121f_state_t mipi_renesas_d121f_state = MIPI_RENESAS_D121F_STATE_OFF;

struct lcd_gpio_info {
    unsigned no;
    char *name;
};

/* MIPI-DSI Commands for Renesas Driver.  */
/* Sub Commands */
static char d121f_set_address_mode[] = {
    0x36,    /* Command */
    0x50,    /* Parameter */
};

/* MCAP */
static char d121f_mcap_off[] = {
    0xB0,    /* Command */
    0x00,    /* Parameter */
};

/* ACR */
static char d121f_acr_off[] = {
    0xB2,    /* Command */
    0x00,    /* Parameter */
};

/* CLK and IF */
static char d121f_clk_if[] = {
    0xB3,    /* Command */
    0x0C,    /* Parameter */
};

/* PXL format */
static char d121f_pxl_fmt[] = {
    0xB4,    /* Command */
    0x02,    /* Parameter */
};

/* DSI format */
static char d121f_dsi_ctl[] = {
    0xB6,    /* Command */
    0x42,    /* Parameter1 */
    0xE3,    /* Parameter2 */
};

static char d121f_bkl_ctl_1[] = {
    0xB7,    /* Command */
    0x18,    /* Parameter1 */
    0x00,    /* Parameter2 */
    0x18,    /* Parameter3 */
    0x18,    /* Parameter4 */
    0x0C,    /* Parameter5 */
    0x10,    /* Parameter6 */
    0x5C,    /* Parameter7 */
    0x10,    /* Parameter8 */
    0xAC,    /* Parameter9 */
    0x10,    /* Parameter10 */
    0x0C,    /* Parameter11 */
    0x10,    /* Parameter12 */
    0x00,    /* Parameter13 */
    0x10,    /* Parameter14 */
};

static char d121f_bkl_ctl_2[] = {
    0xB8,    /* Command */
    0xF8,    /* Parameter1 */
    0xDA,    /* Parameter2 */
    0x6D,    /* Parameter3 */
    0xFB,    /* Parameter4 */
    0xFF,    /* Parameter5 */
    0xFF,    /* Parameter6 */
    0xCF,    /* Parameter7 */
    0x1F,    /* Parameter8 */
    0x67,    /* Parameter9 */
    0x89,    /* Parameter10 */
    0xAF,    /* Parameter11 */
    0xD6,    /* Parameter12 */
    0xFF,    /* Parameter13 */
};

static char d121f_pfm_pwm_control[] = {
    0xB9,    /* Command */
    0x07,    /* Parameter1 */
    0x00,    /* Parameter2 */
    0xF4,    /* Parameter3 */
    0x00,    /* Parameter4 */
};

/* CABC on-off */
static char d121f_cabc_off[] = {
	0xBB,    /* Command */
	0x0B,    /* Parameter */
};

static char d121f_cabc_usr_param[] = {
    0xBE,    /* Command */
    0xFF,    /* Parameter1 */
    0x0F,    /* Parameter2 */
    0x0F,    /* Parameter3 */
    0x06,    /* Parameter4 */
    0x04,    /* Parameter5 */
    0x40,    /* Parameter6 */
    0x00,    /* Parameter7 */
    0x5D,    /* Parameter8 */
};

/* Panel driver setting */
static char d121f_panel_drv[] = {
    0xC0,    /* Command */
    0x40,    /* Parameter1 */
    0x02,    /* Parameter2 */
    0x7F,    /* Parameter3 */
    0xC8,    /* Parameter4 */
    0x08,    /* Parameter5 */
};

/* Display H timing */
static char d121f_disp_h_timing[] = {
    0xC1,    /* Command */
    0x00,    /* Parameter1 */
    0xA8,    /* Parameter2 */
    0x00,    /* Parameter3 */
    0x00,    /* Parameter4 */
    0x00,    /* Parameter5 */
    0x00,    /* Parameter6 */
    0x00,    /* Parameter7 */
    0x9D,    /* Parameter8 */
    0x08,    /* Parameter9 */
    0x27,    /* Parameter10 */
    0x09,    /* Parameter11 */
    0x00,    /* Parameter12 */
    0x00,    /* Parameter13 */
    0x00,    /* Parameter14 */
    0x00,    /* Parameter15 */
};

/* Source output */
static char d121f_src_out[] = {
    0xC2,    /* Command */
    0x00,    /* Parameter1 */
    0x00,    /* Parameter2 */
    0x09,    /* Parameter3 */
    0x00,    /* Parameter4 */
    0x00,    /* Parameter5 */
};

/* Gate IC control */
static char d121f_gate_ic_ctrl[] = {
    0xC3,    /* Command */
    0x04,    /* Parameter */
};

/* LTPS IF control */
static char d121f_ltps_if_mode[] = {
    0xC4,    /* Command */
    0x4D,    /* Parameter1 */
    0x83,    /* Parameter2 */
    0x00,    /* Parameter3 */
};

/* Source output mode */
static char d121f_src_out_mode[] = {
    0xC6,    /* Command */
    0x12,    /* Parameter1 */
    0x00,    /* Parameter2 */
    0x08,    /* Parameter3 */
    0x71,    /* Parameter4 */
    0x00,    /* Parameter5 */
    0x00,    /* Parameter6 */
    0x00,    /* Parameter7 */
    0x80,    /* Parameter8 */
    0x00,    /* Parameter9 */
    0x04,    /* Parameter10 */
};

/* LTPS IF contro */
static char d121f_ltps_if_ctrl[] = {
    0xC7,    /* Command */
    0x22,    /* Parameter */
};

/* Gamma control */
static char d121f_gamma_ctrl[] = {
    0xC8,    /* Command */
    0x4C,    /* Parameter1 */
    0x0C,    /* Parameter2 */
    0x0C,    /* Parameter3 */
    0x0C,    /* Parameter4 */
};

/* Gamma control set A positive */
static char d121f_gamma_ctrl_set_A_p[] = {
    0xC9,    /* Command */
    0x00,    /* Parameter1 */
    0x40,    /* Parameter2 */
    0x0F,    /* Parameter3 */
    0x2B,    /* Parameter4 */
    0x36,    /* Parameter5 */
    0x38,    /* Parameter6 */
    0x45,    /* Parameter7 */
    0x4D,    /* Parameter8 */
    0x44,    /* Parameter9 */
    0x3C,    /* Parameter10 */
    0x36,    /* Parameter11 */
    0x5A,    /* Parameter12 */
    0x3F,    /* Parameter13 */
};

/* Gamma control set A negative */
static char d121f_gamma_ctrl_set_A_n[] = {
    0xCA,    /* Command */
    0x00,    /* Parameter1 */
    0x65,    /* Parameter2 */
    0x29,    /* Parameter3 */
    0x23,    /* Parameter4 */
    0x1B,    /* Parameter5 */
    0x12,    /* Parameter6 */
    0x1A,    /* Parameter7 */
    0x27,    /* Parameter8 */
    0x29,    /* Parameter9 */
    0x34,    /* Parameter10 */
    0x50,    /* Parameter11 */
    0x7F,    /* Parameter12 */
    0x3F,    /* Parameter13 */
};

/* Gamma control set B positive */
static char d121f_gamma_ctrl_set_B_p[] = {
    0xCB,    /* Command */
    0x00,    /* Parameter1 */
    0x40,    /* Parameter2 */
    0x0F,    /* Parameter3 */
    0x37,    /* Parameter4 */
    0x41,    /* Parameter5 */
    0x40,    /* Parameter6 */
    0x4B,    /* Parameter7 */
    0x51,    /* Parameter8 */
    0x47,    /* Parameter9 */
    0x3F,    /* Parameter10 */
    0x39,    /* Parameter11 */
    0x5C,    /* Parameter12 */
    0x3F,    /* Parameter13 */
};

/* Gamma control set B negative */
static char d121f_gamma_ctrl_set_B_n[] = {
    0xCC,    /* Command */
    0x00,    /* Parameter1 */
    0x63,    /* Parameter2 */
    0x26,    /* Parameter3 */
    0x20,    /* Parameter4 */
    0x18,    /* Parameter5 */
    0x0E,    /* Parameter6 */
    0x14,    /* Parameter7 */
    0x1F,    /* Parameter8 */
    0x1E,    /* Parameter9 */
    0x28,    /* Parameter10 */
    0x50,    /* Parameter11 */
    0x7F,    /* Parameter12 */
    0x3F,    /* Parameter13 */
};

/* Gamma control set C positive */
static char d121f_gamma_ctrl_set_C_p[] = {
    0xCD,    /* Command */
    0x00,    /* Parameter1 */
    0x5C,    /* Parameter2 */
    0x4F,    /* Parameter3 */
    0x58,    /* Parameter4 */
    0x57,    /* Parameter5 */
    0x4E,    /* Parameter6 */
    0x54,    /* Parameter7 */
    0x57,    /* Parameter8 */
    0x4C,    /* Parameter9 */
    0x45,    /* Parameter10 */
    0x47,    /* Parameter11 */
    0x6D,    /* Parameter12 */
    0x3F,    /* Parameter13 */
};

/* Gamma control set C negative */
static char d121f_gamma_ctrl_set_C_n[] = {
    0xCE,    /* Command */
    0x00,    /* Parameter1 */
    0x52,    /* Parameter2 */
    0x18,    /* Parameter3 */
    0x1A,    /* Parameter4 */
    0x13,    /* Parameter5 */
    0x08,    /* Parameter6 */
    0x0B,    /* Parameter7 */
    0x11,    /* Parameter8 */
    0x08,    /* Parameter9 */
    0x07,    /* Parameter10 */
    0x10,    /* Parameter11 */
    0x63,    /* Parameter12 */
    0x3F,    /* Parameter13 */
};

/* Power setting 1 */
static char d121f_pwr_set_1[] = {
    0xD0,    /* Command */
    0x6A,    /* Parameter1 */
    0x64,    /* Parameter2 */
    0x01,    /* Parameter3 */
};

/* Power setting */
static char d121f_pwr_set_2[] = {
    0xD1,    /* Command */
    0x77,    /* Parameter1 */
    0xD4,    /* Parameter2 */
};

/* Power setting for internal */
static char d121f_pwr_set_internal[] = {
    0xD3,    /* Command */
    0x33,    /* Parameter */
};

/* VPLVL-VNLVL Setting */
static char d121f_vplvl_vnlvl_set[] = {
    0xD5,    /* Command */
    0x0C,    /* Parameter1 */
    0x0C,    /* Parameter2 */
};

/* VCOMDC setting1 */
static char d121f_vcomdc_set_1[] = {
    0xD8,    /* Command */
    0x34,    /* Parameter1 */
    0x64,    /* Parameter2 */
    0x23,    /* Parameter3 */
    0x25,    /* Parameter4 */
    0x62,    /* Parameter5 */
    0x32,    /* Parameter6 */
};

/* VCOMDC setting2 */
static char d121f_vcomdc_set_2[] = {
    0xDE,    /* Command */
    0x10,    /* Parameter1 */
    0x7B,    /* Parameter2 */
    0x11,    /* Parameter3 */
    0x09,    /* Parameter4 */
    0x00,    /* Parameter5 */
    0x00,    /* Parameter6 */
    0x00,    /* Parameter7 */
    0x00,    /* Parameter8 */
    0x00,    /* Parameter9 */
    0x00,    /* Parameter10 */
    0x00,    /* Parameter11 */
};

/* Source output2 */
static char d121f_source_output2[] = {
    0xFD,    /* Command */
    0x04,    /* Parameter1 */
    0x55,    /* Parameter2 */
    0x53,    /* Parameter3 */
    0x00,    /* Parameter4 */
    0x70,    /* Parameter5 */
    0xFF,    /* Parameter6 */
    0x10,    /* Parameter7 */
    0x73,    /* Parameter8 */
};

/* NVM load control */
static char d121f_nvm_load_ctrl[] = {
    0xE2,    /* Command */
    0x00,    /* Parameter1 */
};

/* Exit sleep mode */
static char d121f_exit_sleep[] = {
    0x11,    /* Command */
};

/* Enter sleep mode */
static char d121f_enter_sleep[] = {
    0x10,    /* Command */
};

/* Enter deep standby mode */
static char d121f_enter_dstb[] = {
    0xB1,    /* Command */
    0x01,    /* Parameter */
};

/* Display on */
static char d121f_display_on[] = {
    0x29,    /* Command */
};
/* Main Commands */
/* LCDM On Sequence */
static struct dsi_cmd_desc renesas_d121f_lcdm_initialize_cmds[] = {
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(d121f_set_address_mode), d121f_set_address_mode},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_mcap_off), d121f_mcap_off},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_acr_off), d121f_acr_off},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_clk_if), d121f_clk_if},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_pxl_fmt), d121f_pxl_fmt},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_dsi_ctl), d121f_dsi_ctl},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_pfm_pwm_control), d121f_pfm_pwm_control},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_cabc_usr_param), d121f_cabc_usr_param},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_bkl_ctl_1), d121f_bkl_ctl_1},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_bkl_ctl_2), d121f_bkl_ctl_2},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_cabc_off), d121f_cabc_off},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_panel_drv), d121f_panel_drv},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_disp_h_timing), d121f_disp_h_timing},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_src_out), d121f_src_out},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_gate_ic_ctrl), d121f_gate_ic_ctrl},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_ltps_if_mode), d121f_ltps_if_mode},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_src_out_mode), d121f_src_out_mode},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_ltps_if_ctrl), d121f_ltps_if_ctrl},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_gamma_ctrl), d121f_gamma_ctrl},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_gamma_ctrl_set_A_p), d121f_gamma_ctrl_set_A_p},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_gamma_ctrl_set_A_n), d121f_gamma_ctrl_set_A_n},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_gamma_ctrl_set_B_p), d121f_gamma_ctrl_set_B_p},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_gamma_ctrl_set_B_n), d121f_gamma_ctrl_set_B_n},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_gamma_ctrl_set_C_p), d121f_gamma_ctrl_set_C_p},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_gamma_ctrl_set_C_n), d121f_gamma_ctrl_set_C_n},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_pwr_set_1), d121f_pwr_set_1},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_pwr_set_2), d121f_pwr_set_2},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_pwr_set_internal), d121f_pwr_set_internal},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_vplvl_vnlvl_set), d121f_vplvl_vnlvl_set},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_vcomdc_set_1), d121f_vcomdc_set_1},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_vcomdc_set_2), d121f_vcomdc_set_2},
    {DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(d121f_source_output2), d121f_source_output2},
    {DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(d121f_nvm_load_ctrl), d121f_nvm_load_ctrl},
};

/* Exit Standby Command */
static struct dsi_cmd_desc renesas_d121f_lcdm_exit_stb_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(d121f_exit_sleep), d121f_exit_sleep},
};

/* Display On Command */
static struct dsi_cmd_desc renesas_d121f_lcdm_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(d121f_display_on), d121f_display_on},
};

/* LCDM off sequence */
/* Enter Standby Command */
static struct dsi_cmd_desc renesas_d121f_lcdm_enter_stb_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(d121f_enter_sleep), d121f_enter_sleep},
};

/* LCDM Enter DSTB sequence */
static struct dsi_cmd_desc renesas_d121f_lcdm_enter_dstb_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(d121f_enter_dstb), d121f_enter_dstb},
};

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_state_transition
*   ABSTRACT : Set Driver's Internal Status
*   FUNCTION : mipi_renesas_d121f_state_transition()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_state_transition( mipi_renesas_d121f_state_t current_value,
                                           mipi_renesas_d121f_state_t set_value )
{
    if (mipi_renesas_d121f_state != current_value)
    {
        /* Error Log */
        printk(KERN_ERR "%s. (%d->%d) found %d\n", __func__, current_value,
                                                   set_value, mipi_renesas_d121f_state);
    }

    mipi_renesas_d121f_state = set_value;
    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_set_backlight
*   ABSTRACT : Set LCD-Backlight
*   FUNCTION : mipi_renesas_d121f_set_backlight()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_set_backlight(struct msm_fb_data_type *mfd)
{
    int ret;
    unsigned char bl_level = 0;

    if (mfd == NULL)
    {
        printk(KERN_ERR "%s (%d): NULL parameter.\n", __func__, __LINE__);
        return;
    }

    bl_level = (unsigned char)mfd->bl_level;

#if defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
    /* Backlight Britness is Max */
    bl_level = 0xFF;
#endif

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

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_ctrl_gpio_36
*   ABSTRACT : 
*   FUNCTION : mipi_renesas_d121f_ctrl_gpio_36()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_ctrl_gpio_36(int on)
{
    int ret;
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
    
    gpio36 = PM8921_GPIO_PM_TO_SYS(36);

    if(!gpio36_request){
        ret = gpio_request(gpio36, MIPI_RENESAS_GPIO_DISP_RST);
        if (ret)
        {
            printk(KERN_DEBUG "%s: request gpio 36 failed, ret=%d.\n", __func__, ret);
            return -ENODEV;
        }
        gpio36_request = 1;
    }

    ret = pm8xxx_gpio_config(gpio36, &gpio36_param);
    if (ret)
    {
        printk(KERN_DEBUG "%s: gpio_config 36 failed, ret=%d.\n", __func__, ret);
        return -EINVAL;
    }

    if (!on)
    {
        ret = gpio_direction_output(gpio36, 0);
        if (ret)
        {
            printk(KERN_DEBUG "%s: gpio36 output failed.\n", __func__);
        }
    }
    else
    {
        ret = gpio_direction_output(gpio36, 1);
        if (ret)
        {
            printk(KERN_DEBUG "%s: gpio36 output failed.\n", __func__);
        }
    }


    return 0;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_ctrl_gpio_78
*   ABSTRACT : 
*   FUNCTION : mipi_renesas_d121f_ctrl_gpio_78()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_ctrl_gpio_78(int on)
{
    int ret;
    static const struct lcd_gpio_info lcd_5v_oe = {78, MIPI_RENESAS_GPIO_5V_OE};

    if(!gpio78_request){
        /* GPIO:78 Configure */
        ret = gpio_request(lcd_5v_oe.no, lcd_5v_oe.name);
        if (ret) {
            printk(KERN_DEBUG "%s: gpio_request(%s.%d) failed.\n", __func__, 
                   lcd_5v_oe.name, lcd_5v_oe.no);
        }
        gpio78_request = 1;
    }

    /* GPIO:78 High/Low */
    if (!on)
    {
        ret = gpio_direction_output(lcd_5v_oe.no, 0);
        if (ret)
        {
            printk(KERN_DEBUG "%s: gpio78 output failed.\n", __func__);
        }
    }
    else
    {
        ret = gpio_direction_output(lcd_5v_oe.no, 1);
        if (ret)
        {
            printk(KERN_DEBUG "%s: gpio78 output failed.\n", __func__);
        }
    }


    return ret;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_power_ctl
*   ABSTRACT : LCD-Panel Power Control.
*   FUNCTION : mipi_renesas_d121f_power_ctl()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_power_ctl( int on )
{
    int ret;

    if (!on)
    {
        ret = mipi_renesas_d121f_ctrl_gpio_36(0);
    }

    if (vreg_l2 == NULL) {
        /* VREG_L2 */
        vreg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
                               "dsi_vdda");
        if (IS_ERR(vreg_l2)) {
            printk(KERN_DEBUG "%s: VREG_L2 failed\n", __func__);
            ret = PTR_ERR(vreg_l2);
            return ret;
        }

        ret = regulator_set_voltage(vreg_l2, 1200000, 1200000);
        if (ret) {
            printk(KERN_DEBUG "set_voltage l2 failed, ret=%d\n", ret);
            goto out;
        }

        ret = regulator_set_optimum_mode(vreg_l2, 100000);
        if (ret < 0) {
            printk(KERN_DEBUG "set_optimum_mode l2 failed, ret=%d\n", ret);
            goto out;
        }
    }
    
    if (on)
        ret = regulator_enable(vreg_l2);
    else
        ret = regulator_disable(vreg_l2);
    if (ret) {
        printk(KERN_DEBUG "enable l2 failed, ret=%d\n", ret);
        goto out;
    }

    if (vreg_l8 == NULL) {
        /* VREG_L8 */
        vreg_l8 = regulator_get(NULL, "8921_l8");
        if (IS_ERR(vreg_l8))
        {
            printk(KERN_DEBUG "%s: VREG_L8 failed.\n", __func__);
            ret = PTR_ERR(vreg_l8);
            return ret;
        }

        ret = regulator_set_voltage(vreg_l8, 2800000, 2800000);
        if (ret)
        {
            printk(KERN_DEBUG "%s: vreg_l8 set voltage failed.\n", __func__);
            goto out;
        }
    }

    if (on)
        ret = regulator_enable(vreg_l8);
    else
        ret = regulator_disable(vreg_l8);
    if (ret)
    {
        printk(KERN_DEBUG "%s: vreg_l8 enable failed.\n", __func__);
        goto out;
    }

    if (vreg_lvs5 == NULL) {
        /* VREG_LVS5 */
        vreg_lvs5 = regulator_get(NULL, "8921_lvs5");
        if (IS_ERR(vreg_lvs5))
        {
            printk(KERN_DEBUG "%s: VREG_LVS5 failed.\n", __func__);
            ret = PTR_ERR(vreg_lvs5);
            return ret;
        }
    }

    if (on)
        ret = regulator_enable(vreg_lvs5);
    else
        ret = regulator_disable(vreg_lvs5);
    if (ret)
    {
        printk(KERN_DEBUG "%s: vreg_lvs5 enable failed.\n", __func__);
        goto out;
    }

    if (on)
    {
        msleep(10);
        ret = mipi_renesas_d121f_ctrl_gpio_36(1);
    }

    printk(KERN_DEBUG "[Out]%s.\n", __func__);
    return 0;

out:
    if (vreg_l2)
    {
        if (regulator_is_enabled(vreg_l2))
        {
            regulator_disable(vreg_l2);
        }
        regulator_put(vreg_l2);
    }

    if (vreg_lvs5)
    {
        if (regulator_is_enabled(vreg_lvs5))
        {
            regulator_disable(vreg_lvs5);
        }
        regulator_put(vreg_lvs5);
    }

    if (vreg_l8)
    {
        if (regulator_is_enabled(vreg_l8))
        {
            regulator_disable(vreg_l8);
        }
        regulator_put(vreg_l8);
    }

    vreg_lvs5 = NULL;
    vreg_l8   = NULL;
    vreg_l2   = NULL;
    return ret;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_initialize_setup
*   ABSTRACT : Send Initialize Commands for LCD-Panel.
*   FUNCTION : mipi_renesas_d121f_initialize_setup()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_initialize_setup( struct msm_fb_data_type *mfd )
{
    int ret;

    mipi_renesas_d121f_ctrl_gpio_78(1);

    msleep(10);

    ret = led_main_lcd_reg_init();
    if(ret)
        printk(KERN_DEBUG "%s:LED register init failed\n", __func__);

    /* Send TX Command for LCD Panel */
    mipi_dsi_cmds_tx( mfd,
                      &renesas_d121f_tx_buf,
                      renesas_d121f_lcdm_initialize_cmds,
                      ARRAY_SIZE(renesas_d121f_lcdm_initialize_cmds)
                    );

    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_set_display_on
*   ABSTRACT : Send Display On Commands for LCD-Panel.
*   FUNCTION : mipi_renesas_d121f_set_display_on()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_set_display_on( struct msm_fb_data_type *mfd )
{
    int ret;

    /* Send TX Command for LCD Panel */
    mipi_dsi_cmds_tx( mfd,
                      &renesas_d121f_tx_buf,
                      renesas_d121f_lcdm_display_on_cmds,
                      ARRAY_SIZE(renesas_d121f_lcdm_display_on_cmds)
                    );
    ret = led_main_lcd_set(LEDS_LED_ON, LEDS_LED_ON);
    if ( ret != LEDS_LED_SET_OK)
    {
        printk(KERN_ERR "%s. backlight on request failed!!\n", __func__);
        return;
    }

    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_set_display_off
*   ABSTRACT : Send Display Off Commands for LCD-Panel.
*   FUNCTION : mipi_renesas_d121f_set_display_off()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_set_display_off( struct msm_fb_data_type *mfd )
{
    int ret;

    /* Set Backlight Off */
    ret = led_main_lcd_set(LEDS_LED_OFF, LEDS_LED_OFF);

    if ( ret != LEDS_LED_SET_OK)
        printk(KERN_ERR "%s. backlight off request failed!!\n", __func__);

    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_enter_standby
*   ABSTRACT : Send Enter Standby Commands for LCD-Panel.
*   FUNCTION : mipi_renesas_d121f_set_display_on()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_enter_standby( struct msm_fb_data_type *mfd )
{
    /* Send TX Command for LCD Panel */
    mipi_dsi_cmds_tx( mfd,
                      &renesas_d121f_tx_buf,
                      renesas_d121f_lcdm_enter_stb_cmds,
                      ARRAY_SIZE(renesas_d121f_lcdm_enter_stb_cmds)
                    );

    /* 81ms wait */
    msleep(81);

//    mipi_renesas_d121f_ctrl_gpio_78(0);

    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_set_display_on
*   ABSTRACT : Send Exit Standby Commands for LCD-Panel.
*   FUNCTION : mipi_renesas_d121f_set_display_on()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_exit_standby( struct msm_fb_data_type *mfd )
{
    /* Send TX Command for LCD Panel */
    mipi_dsi_cmds_tx( mfd, 
                      &renesas_d121f_tx_buf,
                      renesas_d121f_lcdm_exit_stb_cmds,
                      ARRAY_SIZE(renesas_d121f_lcdm_exit_stb_cmds)
                     );

    msleep(120);

    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_enter_deep_standby
*   ABSTRACT : Send Enter Deep Standby Commands for LCD-Panel.
*   FUNCTION : mipi_renesas_d121f_enter_deep_standby()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static void mipi_renesas_d121f_enter_deep_standby( struct msm_fb_data_type *mfd )
{
    /* Send TX Command for LCD Panel */
    mipi_dsi_cmds_tx( mfd, 
                      &renesas_d121f_tx_buf,
                      renesas_d121f_lcdm_enter_dstb_cmds,
                      ARRAY_SIZE(renesas_d121f_lcdm_enter_dstb_cmds)
                     );

}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_exit_deep_standby
*   ABSTRACT : Send Exit Deep Standby Commands for LCD-Panel.
*   FUNCTION : mipi_renesas_d121f_exit_deep_standby()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static int mipi_renesas_d121f_exit_deep_standby( struct msm_fb_data_type *mfd )
{
    /* GPIO[36] = Low */
    mipi_renesas_d121f_ctrl_gpio_36(0);

    /* 6ms wait */
    msleep(6);

    /* GPIO[36] = High */
    mipi_renesas_d121f_ctrl_gpio_36(1);

    /* 11ms wait */
    msleep(11);

    return 0;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_on
*   ABSTRACT : MIPI-DSI Client(LCD Driver) On.
*   FUNCTION : mipi_renesas_d121f_on()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static int mipi_renesas_d121f_on( struct platform_device *pdev )
{
    struct msm_fb_data_type *mfd;
    mipi_renesas_d121f_state_t curr_state;

    printk(KERN_DEBUG "[In]%s. stat:%d \n", __func__, mipi_renesas_d121f_state);

    mfd = platform_get_drvdata(pdev);

    if (!mfd)
        return -ENODEV;
    if (mfd->key != MFD_KEY)
        return -EINVAL;

    mipi_dsi_buf_init(&renesas_d121f_tx_buf);

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    switch (curr_state)
    {
        case MIPI_RENESAS_D121F_STATE_OFF :
            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            /* LCDM On Sequence */
            /* Initalize */
            mipi_renesas_d121f_initialize_setup(mfd);

            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            /* Exit Standby */
            mipi_renesas_d121f_exit_standby(mfd);
            break;

        case MIPI_RENESAS_D121F_STATE_STANDBY :
            /* LCDM On Sequence */
            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            /* Initalize */
            mipi_renesas_d121f_initialize_setup(mfd);

            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            /* Exit Standby */
            mipi_renesas_d121f_exit_standby(mfd);
            break;

        case MIPI_RENESAS_D121F_STATE_DEEP_STANDBY :
            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            /* Exit Deep Standby */
            if (mipi_renesas_d121f_exit_deep_standby(mfd) != 0)
            {
                printk(KERN_ERR "%s. Error - %d \n", __func__, __LINE__);
                return -1;
            }

            /* LCDM On Sequence */
            /* Initalize */
            mipi_renesas_d121f_initialize_setup(mfd);

            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            /* Exit Standby */
            mipi_renesas_d121f_exit_standby(mfd);
            break;

        case MIPI_RENESAS_D121F_STATE_READY : 
            /* nothing to do */
            break;

        case MIPI_RENESAS_D121F_STATE_NORMAL_MODE : 
            /* nothing to do */
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    /* change internal state */
//    mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_NORMAL_MODE);
    if ((curr_state != MIPI_RENESAS_D121F_STATE_NORMAL_MODE) && (curr_state != MIPI_RENESAS_D121F_STATE_READY) )
        mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_READY);

    printk(KERN_DEBUG "[Out]%s. stat;%d \n", __func__, mipi_renesas_d121f_state);
    return 0;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_off
*   ABSTRACT : MIPI-DSI Client(LCD Driver) Off.
*   FUNCTION : mipi_renesas_d121f_on()
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static int mipi_renesas_d121f_off( struct platform_device *pdev )
{
    struct msm_fb_data_type *mfd;
    mipi_renesas_d121f_state_t curr_state;

    printk(KERN_DEBUG "[In]%s. stat:%d \n", __func__, mipi_renesas_d121f_state);

    mfd = platform_get_drvdata(pdev);

    if (!mfd)
        return -ENODEV;
    if (mfd->key != MFD_KEY)
        return -EINVAL;

    if (msm_fb_disable_sleep == TRUE)
        return 0;

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    switch (curr_state)
    {
        case MIPI_RENESAS_D121F_STATE_OFF :
            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            printk(KERN_ERR "Current status is %d !! \n", curr_state);
            break;

        case MIPI_RENESAS_D121F_STATE_PRE_STANDBY :
            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            msleep(65);

            mipi_renesas_d121f_ctrl_gpio_78(0);
            break;

        case MIPI_RENESAS_D121F_STATE_STANDBY :
            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            break;

        case MIPI_RENESAS_D121F_STATE_DEEP_STANDBY :
            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            break;

        case MIPI_RENESAS_D121F_STATE_READY : 
            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* Enter Standby */
            mipi_renesas_d121f_enter_standby(mfd);

            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);
            
            mipi_renesas_d121f_ctrl_gpio_78(0);
            break;

        case MIPI_RENESAS_D121F_STATE_NORMAL_MODE : 
            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* LCDM off sequence */
            /* Display Off */
            mipi_renesas_d121f_set_display_off(mfd);

            /* Enter Standby */
            mipi_renesas_d121f_enter_standby(mfd);

            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);
            
            mipi_renesas_d121f_ctrl_gpio_78(0);

            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    /* change internal state */
    if (curr_state != MIPI_RENESAS_D121F_STATE_OFF)
    {
        mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_OFF);
    }

    printk(KERN_DEBUG "[Out]%s. stat;%d \n", __func__, mipi_renesas_d121f_state);
    return 0;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_enable_display
*   ABSTRACT : Display On.
*   FUNCTION : 
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/03/19
*   UPDATE   : 
******************************************************************************/
void mipi_renesas_d121f_enable_display(struct msm_fb_data_type *mfd)
{
    mipi_renesas_d121f_state_t curr_state;

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    if (curr_state == MIPI_RENESAS_D121F_STATE_READY)
    {
        printk(KERN_ERR "[DRV-LCD]%s:display on\n", __func__);
        mipi_dsi_sw_reset();

        /* Change MIPI-DSI "Command Mode" */
        mipi_dsi_op_mode_config(DSI_CMD_MODE);

        /* Set "HS" Mode */
        mipi_set_tx_power_mode(0);

        /* Set Display On */
        mipi_renesas_d121f_set_display_on(mfd);

        mipi_dsi_sw_reset();

        /* Change MIPI-DSI "Video Mode" */
        mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

        /* change internal state */
        mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_NORMAL_MODE);
    }
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_disable_display
*   ABSTRACT : Display Off. Send Standby Command to LCD-Panel
*   FUNCTION : 
*   NOTE     : -
*   RETURN   : None
*   CREATE   : 2012/04/23
*   UPDATE   : 
******************************************************************************/
void mipi_renesas_d121f_disable_display(struct msm_fb_data_type *mfd)
{
    mipi_renesas_d121f_state_t curr_state;

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    if (curr_state == MIPI_RENESAS_D121F_STATE_NORMAL_MODE)
    {

        /* Set "LP" Mode */
        mipi_set_tx_power_mode(1);

        mipi_dsi_sw_reset();

        /* Change MIPI-DSI "Command Mode" */
        mipi_dsi_op_mode_config(DSI_CMD_MODE);

        /* LCDM off sequence */
        /* Display Off */
        mipi_renesas_d121f_set_display_off(mfd);

        /* Send TX Command for LCD Panel */
        mipi_dsi_cmds_tx( mfd,
                          &renesas_d121f_tx_buf,
                          renesas_d121f_lcdm_enter_stb_cmds,
                          ARRAY_SIZE(renesas_d121f_lcdm_enter_stb_cmds)
                        );

        mipi_dsi_sw_reset();

        /* Change MIPI-DSI "Video Mode" */
        mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

        /* change internal state */
        mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_PRE_STANDBY);
    }

    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_lcd_probe
*   ABSTRACT : driver's probe function.
*   FUNCTION : 
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static int __devinit mipi_renesas_d121f_lcd_probe(struct platform_device *pdev)
{
    if (pdev->id == 0) 
    {
        mipi_renesas_d121f_pdata = pdev->dev.platform_data;
        return 0;
    }

    msm_fb_add_device(pdev);
    return 0;
}


static struct platform_driver this_driver = {
    .probe  = mipi_renesas_d121f_lcd_probe,
    .driver = {
        .name   = "mipi_renesas_d121f",
    },
};

static struct msm_fb_panel_data renesas_d121f_panel_data = {
    .on     = mipi_renesas_d121f_on,
    .off    = mipi_renesas_d121f_off,
    .set_backlight = mipi_renesas_d121f_set_backlight,
};


/******************************************************************************
*   MODULE   : mipi_renesas_d121f_device_register
*   ABSTRACT : Registeration devicce drivers
*   FUNCTION : 
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_device_register(struct msm_panel_info *pinfo, u32 channel, u32 panel)
{
    struct platform_device *pdev = NULL;
    int ret;

    if ( (channel >= 3) || ch_used[channel] )
        return -ENODEV;

    ch_used[channel] = TRUE;

    pdev = platform_device_alloc("mipi_renesas_d121f", (panel << 8)|channel);
    if (!pdev)
        return -ENOMEM;

    renesas_d121f_panel_data.panel_info = *pinfo;

    ret = platform_device_add_data(pdev, &renesas_d121f_panel_data,
                                   sizeof(renesas_d121f_panel_data) );

    if (ret) {
        printk(KERN_ERR
          "%s: platform_device_add_data failed!\n", __func__);
        goto err_device_put;
    }

    ret = platform_device_add(pdev);
    if (ret) {
        printk(KERN_ERR
          "%s: platform_device_register failed!\n", __func__);
        goto err_device_put;
    }

    printk(KERN_DEBUG "Create mipi_renesas_d121f device success! \n");
    return 0;

err_device_put:
    platform_device_put(pdev);
    return ret;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_lcd_init
*   ABSTRACT : driver's initialize function
*   FUNCTION : 
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
static int __init mipi_renesas_d121f_lcd_init(void)
{
    int rc;

    mipi_dsi_buf_alloc(&renesas_d121f_tx_buf, DSI_BUF_SIZE);
    mipi_dsi_buf_alloc(&renesas_d121f_rx_buf, DSI_BUF_SIZE);

    rc = platform_driver_register(&this_driver);
    return rc;
}

/* for diag test program */
/******************************************************************************
*   MODULE   : mipi_renesas_d121f_set_idle_state
*   ABSTRACT : Set LCD-Panel to IDLE Mode.
*   FUNCTION : mipi_renesas_d121f_set_idle_state()
*   NOTE     : for diag test program
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
void mipi_renesas_d121f_set_idle_state(struct msm_fb_data_type *mfd)
{
    mipi_renesas_d121f_state_t curr_state;

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    switch (curr_state)
    {
        case MIPI_RENESAS_D121F_STATE_OFF :
            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* Power On */
            if (mipi_renesas_d121f_power_ctl(1) != 0)
            {
                printk(KERN_ERR "%s. LCD Power On Request Failed!!\n", __func__);
                return;
            }

            msleep(11);

            /* LCDM On Sequence */
            /* Initalize */
            mipi_renesas_d121f_initialize_setup(mfd);

            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            /* Exit Standby */
            mipi_renesas_d121f_exit_standby(mfd);

            /* Set Display On */
            mipi_renesas_d121f_set_display_on(mfd);

            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Video Mode" */
            mipi_dsi_op_mode_config(DSI_VIDEO_MODE);
            break;

        case MIPI_RENESAS_D121F_STATE_STANDBY :
            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* LCDM On Sequence */
            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);

            /* Initalize */
            mipi_renesas_d121f_initialize_setup(mfd);

            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            /* Exit Standby */
            mipi_renesas_d121f_exit_standby(mfd);

            /* Set Display On */
            mipi_renesas_d121f_set_display_on(mfd);

            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Video Mode" */
            mipi_dsi_op_mode_config(DSI_VIDEO_MODE);
            break;

        case MIPI_RENESAS_D121F_STATE_DEEP_STANDBY :
            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Command Mode" */
            mipi_dsi_op_mode_config(DSI_CMD_MODE);

            /* Set "LP" Mode */
            mipi_set_tx_power_mode(1);
            /* Exit Deep Standby */
            if (mipi_renesas_d121f_exit_deep_standby(mfd) != 0)
            {
                printk(KERN_ERR "%s. Error - %d \n", __func__, __LINE__);
                return;
            }

            /* LCDM On Sequence */
            /* Initalize */
            mipi_renesas_d121f_initialize_setup(mfd);

            /* Set "HS" Mode */
            mipi_set_tx_power_mode(0);

            /* Exit Standby */
            mipi_renesas_d121f_exit_standby(mfd);

            /* Set Display On */
            mipi_renesas_d121f_set_display_on(mfd);

            mipi_dsi_sw_reset();

            /* Change MIPI-DSI "Video Mode" */
            mipi_dsi_op_mode_config(DSI_VIDEO_MODE);
            break;

        case MIPI_RENESAS_D121F_STATE_NORMAL_MODE : 
            /* nothing to do */
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    /* change internal state */
    mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_NORMAL_MODE);

    return;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_standby_ctl
*   ABSTRACT : 
*   FUNCTION : mipi_renesas_d121f_standby_ctl()
*   NOTE     : for diag test program
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int  mipi_renesas_d121f_standby_ctl(int on, struct msm_fb_data_type *mfd)
{
    mipi_renesas_d121f_state_t curr_state;

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    if (msm_fb_disable_sleep == TRUE)
        return 0;

    switch (curr_state)
    {
        case MIPI_RENESAS_D121F_STATE_OFF :
            printk("%s. Status Error!! - %d\n", __func__, curr_state);
            break;

        case MIPI_RENESAS_D121F_STATE_STANDBY :
            if (!on)
            {
                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Command Mode" */
                mipi_dsi_op_mode_config(DSI_CMD_MODE);

                /* Set "LP" Mode */
                mipi_set_tx_power_mode(1);

                /* Initalize */
                mipi_renesas_d121f_initialize_setup(mfd);

                /* Set "HS" Mode */
                mipi_set_tx_power_mode(0);

                /* Exit Standby */
                mipi_renesas_d121f_exit_standby(mfd);

                /* Set Backlight Level */
                led_main_lcd_bright((unsigned char)mfd->bl_level);

                /* Set Display On */
                mipi_renesas_d121f_set_display_on(mfd);

                /* Internal Status Update */
                mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_NORMAL_MODE);

                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Video Mode" */
                mipi_dsi_op_mode_config(DSI_VIDEO_MODE);
            }
            break;

        case MIPI_RENESAS_D121F_STATE_DEEP_STANDBY :
            printk("%s. Status Error!! - %d\n", __func__, curr_state);
            break;

        case MIPI_RENESAS_D121F_STATE_NORMAL_MODE : 
            if (on)
            {
                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Command Mode" */
                mipi_dsi_op_mode_config(DSI_CMD_MODE);

                /* Set "HS" Mode */
                mipi_set_tx_power_mode(0);

                /* Display Off */
                mipi_renesas_d121f_set_display_off(mfd);

                /* Enter Standby */
                mipi_renesas_d121f_enter_standby(mfd);

                /* Set "LP" Mode */
                mipi_set_tx_power_mode(1);
                
                mipi_renesas_d121f_ctrl_gpio_78(0);

                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Video Mode" */
                mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

                /* Internal Status Update */
                mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_STANDBY);
            }
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    return 0;

}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_deep_standby_ctl
*   ABSTRACT : 
*   FUNCTION : mipi_renesas_d121f_deep_standby_ctl()
*   NOTE     : for diag test program
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int  mipi_renesas_d121f_deep_standby_ctl(int on, struct msm_fb_data_type *mfd)
{
    mipi_renesas_d121f_state_t curr_state;

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    if (msm_fb_disable_sleep == TRUE)
        return 0;

    switch (curr_state)
    {
        case MIPI_RENESAS_D121F_STATE_OFF :
            printk("%s. Status Error!! - %d\n", __func__, curr_state);
            break;

        case MIPI_RENESAS_D121F_STATE_STANDBY :
            printk("%s. Status Error!! - %d\n", __func__, curr_state);
            break;

        case MIPI_RENESAS_D121F_STATE_DEEP_STANDBY :
            if (!on)
            {
                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Command Mode" */
                mipi_dsi_op_mode_config(DSI_CMD_MODE);

                /* Set "LP" Mode */
                mipi_set_tx_power_mode(1);

                /* Exit Deep Standby */
                if (mipi_renesas_d121f_exit_deep_standby(mfd) != 0)
                {
                    printk(KERN_ERR "%s. Error - %d \n", __func__, __LINE__);
                    return -1;
                }

                /* Initalize */
                mipi_renesas_d121f_initialize_setup(mfd);

                /* Set "HS" Mode */
                mipi_set_tx_power_mode(0);

                /* Exit Standby */
                mipi_renesas_d121f_exit_standby(mfd);

                /* Set Backlight Level */
                led_main_lcd_bright((unsigned char)mfd->bl_level);

                /* Set Display On */
                mipi_renesas_d121f_set_display_on(mfd);

                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Video Mode" */
                mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

                mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_NORMAL_MODE);
            }
            break;

        case MIPI_RENESAS_D121F_STATE_NORMAL_MODE : 
            if (on)
            {
                /* Set "HS" Mode */
                mipi_set_tx_power_mode(0);

                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Command Mode" */
                mipi_dsi_op_mode_config(DSI_CMD_MODE);

                /* Display Off */
                mipi_renesas_d121f_set_display_off(mfd);

                /* Enter Standby */
                mipi_renesas_d121f_enter_standby(mfd);

                /* Set "LP" Mode */
                mipi_set_tx_power_mode(1);

                mipi_renesas_d121f_ctrl_gpio_78(0);

                /* Enter Deep Standby */
                mipi_renesas_d121f_enter_deep_standby(mfd);

                /* Internal Status Update */
                mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_DEEP_STANDBY);
            }
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    return 0;

}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_power_seq
*   ABSTRACT : Diag LCD-Panel Display On/Off
*   FUNCTION : -
*   NOTE     : -
*   RETURN   : int  OK:0
*                   NG:-EINVAL
*            :         -ENODEV
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_power_seq(int on, struct msm_fb_data_type *mfd)
{
    mipi_renesas_d121f_state_t curr_state;

    /* Current status get */
    curr_state = mipi_renesas_d121f_state;

    if (msm_fb_disable_sleep == TRUE)
        return 0;

    switch (curr_state)
    {
        case MIPI_RENESAS_D121F_STATE_OFF :
            if (on)
            {
                /* Set "LP" Mode */
                mipi_set_tx_power_mode(1);

                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Command Mode" */
                mipi_dsi_op_mode_config(DSI_CMD_MODE);

                /* Power On (TBD) */
                if (mipi_renesas_d121f_power_ctl(1) != 0)
                {
                    printk(KERN_ERR "%s. LCD Power On Request Failed!!\n", __func__);
                    return -1;
                }

                msleep(11);

                /* LCDM On Sequence */
                /* Initalize */
                mipi_renesas_d121f_initialize_setup(mfd);

                /* Set "HS" Mode */
                mipi_set_tx_power_mode(0);

                /* Exit Standby */
                mipi_renesas_d121f_exit_standby(mfd);

                /* Set Backlight Level */
                led_main_lcd_bright((unsigned char)mfd->bl_level);

                /* Set Display On */
                mipi_renesas_d121f_set_display_on(mfd);

                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Video Mode" */
                mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

                /* Internal Status Update */
                mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_NORMAL_MODE);
            }
            break;

        case MIPI_RENESAS_D121F_STATE_STANDBY :
            printk(KERN_DEBUG "%s. Status Error!! - %d\n", __func__, curr_state);
            break;

        case MIPI_RENESAS_D121F_STATE_DEEP_STANDBY :
            printk(KERN_DEBUG "%s. Status Error!! - %d\n", __func__, curr_state);
            break;

        case MIPI_RENESAS_D121F_STATE_NORMAL_MODE : 
            if (!on)
            {
                /* Set "HS" Mode */
                mipi_set_tx_power_mode(0);

                mipi_dsi_sw_reset();

                /* Change MIPI-DSI "Command Mode" */
                mipi_dsi_op_mode_config(DSI_CMD_MODE);

                /* Display Off */
                mipi_renesas_d121f_set_display_off(mfd);

                /* Enter Standby */
                mipi_renesas_d121f_enter_standby(mfd);

                /* Set "LP" Mode */
                mipi_set_tx_power_mode(1);

                mipi_renesas_d121f_ctrl_gpio_78(0);
                
                msleep(3);

                /* Power Off (TBD) */
                if (mipi_renesas_d121f_power_ctl(0) != 0)
                {
                    printk(KERN_ERR "%s. LCD Power Off Request Failed!!\n", __func__);
                    return -1;
                }
                /* Change MIPI-DSI Still "Command Mode" */
                /* mipi_dsi_op_mode_config(DSI_VIDEO_MODE); */

                /* Internal Status Update */
                mipi_renesas_d121f_state_transition(curr_state, MIPI_RENESAS_D121F_STATE_OFF);
            }
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }
    return 0;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_register_write_cmd
*   ABSTRACT : Send TX Command to LCD-Panel
*   FUNCTION : 
*   NOTE     : use diag test program only
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_register_write_cmd( struct msm_fb_data_type *mfd, 
                                     struct msmfb_register_write *data )
{
    char send_data[255];
    struct dsi_cmd_desc param;

    mipi_dsi_buf_init(&renesas_d121f_tx_buf);

    mipi_dsi_sw_reset();

    /* Change MIPI-DSI "Command Mode" */
    mipi_dsi_op_mode_config(DSI_CMD_MODE);

    /* send data copy */
    memcpy(send_data, (char*)(&data->data[0]), data->len);

    /* mipi data set */
    param.dtype = (int)data->di;
    param.last  = 1;
    param.vc    = 0;
    param.ack   = 0;
    param.wait  = 0;
    param.dlen  = (int)data->len;
    param.payload = send_data;

    /* Send TX Command for LCD Panel */
    mipi_dsi_cmds_tx(mfd, &renesas_d121f_tx_buf, &param, 1);

    mipi_dsi_sw_reset();

    /* Change MIPI-DSI "Video Mode" */
    mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

    return 0;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_register_read_cmd
*   ABSTRACT : Read LCD-Panel Register
*   FUNCTION : 
*   NOTE     : use diag test program only
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_register_read_cmd( struct msm_fb_data_type *mfd, 
                                     struct msmfb_register_read *data )
{
    struct dsi_cmd_desc param;
    char send_data[2]; /* short packet only */
    int read_num;

    mipi_dsi_sw_reset();

    /* Change MIPI-DSI "Command Mode" */
    mipi_dsi_op_mode_config(DSI_CMD_MODE);

    mipi_dsi_buf_init(&renesas_d121f_tx_buf);
    mipi_dsi_buf_init(&renesas_d121f_rx_buf);

    send_data[0] = (char)data->w_data[0];
    send_data[1] = (char)data->w_data[1];
    read_num = (int)data->len;

    /* mipi data set */
    param.dtype = (int)data->di;
    param.last  = 1;
    param.vc    = 0;
    param.ack   = 0;
    param.wait  = 0;
    param.dlen  = 2; /* short packet only */
    param.payload = send_data;

    /* Send RX Command for LCD Panel */
    mipi_dsi_cmds_rx(mfd, &renesas_d121f_tx_buf, &renesas_d121f_rx_buf, &param, read_num);

    /* copy read data to output data */
    if (renesas_d121f_rx_buf.len > 0)
    {
        memcpy(data->r_data, (unsigned char*)renesas_d121f_rx_buf.data, read_num);
    }

    mipi_dsi_sw_reset();

    /* Change MIPI-DSI "Video Mode" */
    mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

    return 0;
}

/******************************************************************************
*   MODULE   : mipi_renesas_d121f_user_request_ctrl
*   ABSTRACT : User data request function
*   FUNCTION : 
*   NOTE     : 
*   RETURN   : None
*   CREATE   : 2012/01/10
*   UPDATE   : 
******************************************************************************/
int mipi_renesas_d121f_user_request_ctrl( struct msmfb_request_parame *data )
{
    int ret = 0;

    switch( data->request )
    {
        case MSM_FB_REQUEST_OVERLAY_ALPHA:
            ret = copy_from_user(&mdp4_overlay_argb_enable, data->data, sizeof(mdp4_overlay_argb_enable));
            break;

        default:
            /* Error Log */
            printk(KERN_ERR "%s user_request error", __func__);
            break;
    }

    return ret;
}

module_init(mipi_renesas_d121f_lcd_init);
