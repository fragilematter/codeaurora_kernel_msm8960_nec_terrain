/*  
 * Copyright (C) 2011, NEC CASIO Mobile Communications. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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


#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/ncdiagd_power.h>
#include <linux/uaccess.h>
#include <mach/pmic.h>

#include <linux/pm.h>   /* used pm_power_off() */
#include <asm/system.h> /* used arm_machine_restart() */

#include <linux/mfd/pm8xxx/pm8xxx-adc.h>        /* used pm8xxx_adc_read()          */
#include <linux/mfd/pm8xxx/pm8921-charger.h>    /* used pm8921_charger_enable()    */
#include <linux/mfd/pm8xxx/pm8921.h>            /* used pm8xxx_gpio_config()       */
#include <linux/mfd/pm8xxx/tm.h>                /* used nc_pm8921_itemp_get_stage()*/
#include <mach/irqs.h>

#define PM8921_GPIO_BASE                NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio + PM8921_GPIO_BASE)
#include <linux/mfd/pm8xxx/mpp.h>               /* used pm8xxx_mpp_config()                 */
#include <linux/mfd/pm8xxx/irq.h>               /* used nc_pm8921_get_rt_status()           */
#include <linux/regulator/pm8xxx-regulator.h>   /* used nc_pm8921_vreg_pull_down_switch()   */
#include <linux/mfd/pm8xxx/vibrator.h>

#define PM8921_MPP_BASE                 (PM8921_GPIO_BASE + PM8921_NR_GPIOS)
#define PM8921_MPP_PM_TO_SYS(pm_gpio)   (pm_gpio + PM8921_MPP_BASE)
#define PM8921_IRQ_BASE                 (NR_MSM_IRQS + NR_GPIO_IRQS)
#include <linux/mfd/pm8xxx/misc.h>
#include <linux/math64.h>

static short access_count = 0;


static unsigned char cvt_val(unsigned char c)
{
    int ten_val;
    int rm_val;
    if (c >= 0xA)
    {
        ten_val = c / 0x10;
        rm_val  = c % 0x10;
        return (ten_val * 10 + rm_val);
    }
    else
    {
        return c;
    }
}


/* Main Proc */
static long ncdiagd_power_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    void __user *pArg = (void __user *)arg;
    printk(KERN_INFO "[%s] cmd:%d",__func__, cmd);

    switch(cmd)
    {

        /*------------------------------------------------------
            DIAG_PW_REGULATOR (0x3101)
        ------------------------------------------------------*/

        case IOCTL_PW_RG_LP_CTL:
            {
                unsigned char enable;
                unsigned char vreg_id;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];

                ret = nc_pm8921_lp_mode_control(vreg_id, enable);
                
                printk(KERN_INFO "DIAG_PW_RG_LP_CTL enable:%x vreg_id:%x", enable, vreg_id);
            }
            break;
        /* Enable/Disable voltage regulators. */
        case IOCTL_PW_RG_CTL:
            {
                unsigned char enable;
                unsigned char vreg_id;
                unsigned int min_volt, max_volt;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];
                min_volt  = cvt_val(buf.req_buf[3]);
                min_volt += cvt_val(buf.req_buf[2]) * 100;
                min_volt *= 1000; /* convert mV to uV */
                max_volt  = cvt_val(buf.req_buf[5]);
                max_volt += cvt_val(buf.req_buf[4]) * 100;
                max_volt *= 1000; /* convert mV to uV */
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_vreg_control(enable, vreg_id, min_volt, max_volt);
#else
                ret = nc_pm8058_vreg_control(enable, vreg_id, min_volt, max_volt);
#endif

                printk(KERN_INFO "DIAG_PW_RG_CTL enable  :%x vreg_id :%x", enable, vreg_id);
                printk(KERN_INFO "               min_volt:%x max_volt:%x", min_volt, max_volt);
            }
            break;
        /* Program the voltage levels for the selected voltage regulator. */
        case IOCTL_PW_RG_SET_LVL:
            {
                unsigned char  vreg_id;
                unsigned int min_volt, max_volt;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                vreg_id   = buf.req_buf[0];
                min_volt  = cvt_val(buf.req_buf[2]);
                min_volt += cvt_val(buf.req_buf[1]) * 100;
                min_volt *= 1000; /* convert mV to uV */
                max_volt  = cvt_val(buf.req_buf[4]);
                max_volt += cvt_val(buf.req_buf[3]) * 100;
                max_volt *= 1000; /* convert mV to uV */
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_vreg_set_level(vreg_id, min_volt, max_volt);
#else
                ret = nc_pm8058_vreg_set_level(vreg_id, min_volt, max_volt);
#endif

                printk(KERN_INFO "DIAG_PW_RG_SET_LVL vreg_id:%x min_volt:%x max_volt:%x", vreg_id, min_volt, max_volt);
            }
            break;

        /* Configure the voltage regulator active pull-down. */
        case IOCTL_PW_VT_PLDWN_SW:
            {
                unsigned char enable;
                unsigned char vreg_id;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_vreg_pull_down_switch(vreg_id, enable);
#else
                ret = nc_pm8058_vreg_pull_down_switch(enable, vreg_id);
#endif
                
                printk(KERN_INFO "DIAG_PW_RG_SMPS_PSK enable:%x vreg_id:%x", enable, vreg_id);
            }
            break;

        /* Enable/Disable the coin cell charger. */
        case IOCTL_PW_CHG_COIN_SW:
            {
                struct pm8xxx_coincell_chg param;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                param.state = buf.req_buf[0];
                param.voltage = PM8XXX_COINCELL_VOLTAGE_3p2V;
                param.resistor = PM8XXX_COINCELL_RESISTOR_800_OHMS;
                
                ret = pm8xxx_coincell_chg_config(&param);
                printk(KERN_INFO "DIAG_PW_CHG_COIN_SW enable:%x", param.state);
            }
            break;
        /* Open/Close the battery FET to start/stop charging (BAT_FET_N pin). */
        case IOCTL_PW_CHG_BAT_EBL:
            {
                bool enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = (buf.req_buf[0]&0x01);
                
                ret = pm8921_charger_enable(enable);
                printk(KERN_INFO "DIAG_PW_CHG_BAT_EBL enable:%x", enable);
            }
            break;

        case IOCTL_PW_CHG_DSBL:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                enable = enable ? 0x00 : 0x01;
                ret = pm8921_disable_source_current(enable);
#else
                ret = nc_pm8058_chg_charge_disable(enable);
#endif
                printk(KERN_INFO "DIAG_PW_CHG_DSBL enable:%x", enable);
            }
            break;

        /* Set the fast charge current limit in mA. */
        case IOCTL_PW_CHG_IMAX_SET:
            {
                unsigned short cur_val;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                cur_val  = buf.req_buf[1];
                cur_val += buf.req_buf[0] * 0x100;
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_chg_imaxsel_set(cur_val);
#else
                ret = nc_pm8058_chg_imaxsel_set(cur_val);
#endif
                printk(KERN_INFO "DIAG_PW_CHG_IMAX_SET cur_val:%x", cur_val);
            }
            break;

        /* Get the value of the current state of the automatic charger. */
        case IOCTL_PW_CHG_STATE_GET:
            {
                unsigned char chg_state = 0x00;
                ioctl_pw_value_type buf;
                
                ret = nc_pm8921_get_fsm_status((u64 *)&chg_state);
                
                buf.rsp_buf[0] = chg_state;
                
                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_CHG_STATE_GET chg_state:%x", chg_state);
            }
            break;

        /* Get the result of the AD conversion on the specified channel. */
        case IOCTL_PW_ADC_RD_CHANNEL:
            {
                unsigned char analog_chnl, mpp_chnl;
                struct pm8xxx_adc_chan_result result;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                analog_chnl = buf.req_buf[0];
                
                if ((analog_chnl <  PM8XXX_CHANNEL_MPP_SCALE1_IDX) ||
                    (analog_chnl == ADC_MPP_1_AMUX8 )              ||
                    (analog_chnl == ADC_MPP_1_AMUX3 ))
                {
                    switch(analog_chnl)
                    {
                      case CHANNEL_BATT_THERM:
                          analog_chnl = CHANNEL_BATT_THERM_MV;
                          break;
                          
                      case CHANNEL_MUXOFF:
                          analog_chnl = CHANNEL_MUXOFF_MV;
                          break;
                          
                      case ADC_MPP_1_AMUX3:
                          analog_chnl = CHANNEL_PA_THERM0_MV;
                          break;
                          
                      default:
                          break;
                    }
                    ret = pm8xxx_adc_read(analog_chnl, &result);
                } else if ((analog_chnl >= PM8XXX_CHANNEL_MPP_SCALE1_IDX) && 
                           (analog_chnl <= ADC_MPP_1_ATEST_7       ) )
                {
                    mpp_chnl = analog_chnl - PM8XXX_CHANNEL_MPP_SCALE1_IDX;
                    ret = pm8xxx_adc_mpp_config_read(mpp_chnl, CHANNEL_MPP_1, &result);

                } else if ((analog_chnl >= PM8XXX_CHANNEL_MPP_SCALE3_IDX) && 
                           (analog_chnl <= ADC_MPP_2_ATEST_7       ) )
                {
                    mpp_chnl = analog_chnl - PM8XXX_CHANNEL_MPP_SCALE3_IDX;
                    ret = pm8xxx_adc_mpp_config_read(mpp_chnl, CHANNEL_MPP_2, &result);
                    
                } else 
                {
                    printk(KERN_ERR "ADC_channel failed");
                    return PM_ERR_FLAG__FEATURE_NOT_SUPPORTED;
                }
                
                if(ret == PM_ERR_FLAG__SUCCESS)
                {
                    if ((analog_chnl != CHANNEL_BATT_THERM) &&             // batt_therm
                        (analog_chnl != CHANNEL_DIE_TEMP  ) &&             // pmic_therm
                        (analog_chnl != ADC_MPP_1_AMUX8   ) &&             // pa_therm1
                        (analog_chnl != CHANNEL_MUXOFF    ) &&             // xo_therm
                        (analog_chnl != ADC_MPP_1_AMUX3   ))               // pa_therm0
                    {
                        if (result.physical != 0)
                        {
                            result.physical = div_u64(result.physical, 1000);
                        }
                        
                        if (result.physical >= 0xFFFF)
                        {
                            result.physical = 0xFFFF;
                        }
                    }
                    buf.rsp_buf[0] = (result.physical >> 8) & 0xff;
                    buf.rsp_buf[1] = result.physical & 0xff;
                }

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_ADC_RD_CHANNEL analog_chnl :%x", analog_chnl);
                printk(KERN_INFO "                       adc_read_val[0]:%x", (int)((result.physical >> 8) & 0xff));
                printk(KERN_INFO "                       adc_read_val[1]:%x", (int)(result.physical & 0xff));
            }
            break;

        /*------------------------------------------------------
            DIAG_PW_LIGHTSENSOR (0x3109)
        ------------------------------------------------------*/
        /* Set the intensity of a selected LED driver signal. */
        case IOCTL_PW_LS_LED_SET:
            {
                unsigned char led_set;
                unsigned char brightness_set;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                led_set        = buf.req_buf[0];
                brightness_set = buf.req_buf[1];
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_led_set_brightness(led_set, brightness_set);
#endif
                printk(KERN_INFO "DIAG_PW_LS_LED_SET led_set:%x brightness_set:%x", led_set, brightness_set);
            }
            break;

        /* Configure the Vibrator Motor mode. */
        case IOCTL_PW_LS_VIB_SET_MODE:
            {
                unsigned char mode_set;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mode_set = buf.req_buf[0];
                ret = pm8xxx_vib_set_mode(mode_set);

                printk(KERN_INFO "DIAG_PW_LS_VIB_SET_MODE mode_set:%x", mode_set);
            }
            break;
        /*------------------------------------------------------
            DIAG_PW_SMPL (0x310C)
        ------------------------------------------------------*/
        /* Set the SMPL detection on or off. */
        case IOCTL_PW_SP_SMPLD_SW:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];

#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = pm8xxx_smpl_control(enable);
#else
                ret = nc_pm8058_smpld_switch(enable);
#endif
                printk(KERN_INFO "DIAG_PW_SP_SMPLD_SW enable:%x", enable);
            }
            break;
        /* Configure the SMPL feature timeout value. */
        case IOCTL_PW_SP_SMPLD_TM_SET:
            {
                unsigned char timer_set;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                timer_set = buf.req_buf[0];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = pm8xxx_smpl_set_delay(timer_set);
#else
                ret = nc_pm8058_set_smpld_timer(timer_set);
#endif
                printk(KERN_INFO "DIAG_PW_SP_SMPLD_TM_SET timer_set:%x", timer_set);
            }
            break;

        /*------------------------------------------------------
            DIAG_PW_MPP (0x310E)
        ------------------------------------------------------*/
        /* Configure the selected MPP as digital input pin. */
        case IOCTL_PW_MPP_CNFDG_IPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_mpp_config_digital_in(PM8921_MPP_PM_TO_SYS(mpp_port), logi_level, PM8XXX_MPP_DIN_TO_INT);
#else
                ret = pm8058_mpp_config_digital_in(mpp_port, logi_level, PM_MPP_DIN_TO_INT);
#endif
                printk(KERN_INFO "DIAG_PW_MPP_CNFDG_IPUT mpp_port:%x logi_level:%x", mpp_port, logi_level);
            }
            break;
        /* Configure the selected MPP as digital output pin. */
        case IOCTL_PW_MPP_CNFDG_OPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                unsigned char out_ctl;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                out_ctl    = buf.req_buf[2];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_mpp_config_digital_out(PM8921_MPP_PM_TO_SYS(mpp_port), logi_level, out_ctl);
#else
                ret = pm8058_mpp_config_digital_out(mpp_port, logi_level, out_ctl);
#endif
                printk(KERN_INFO "DIAG_PW_MPP_CNFDG_OPUT mpp_port:%x logi_level:%x out_ctl:%x", mpp_port, logi_level, out_ctl);
            }
            break;
        /* Configure the selected MPP as a digital bidirectional pin. */
        case IOCTL_PW_MPP_CNFDG_IOPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                unsigned char pull_set;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                pull_set   = buf.req_buf[2];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_mpp_config_bi_dir(PM8921_MPP_PM_TO_SYS(mpp_port), logi_level, pull_set);
#else
                ret = pm8058_mpp_config_bi_dir(mpp_port, logi_level, pull_set);
#endif
                printk(KERN_INFO "DIAG_PW_MPP_CNFDG_IOPUT mpp_port:%x logi_level:%x pull_set:%x", mpp_port, logi_level, pull_set);
            }
            break;
        /* Configure the selected MPP as an analog input. */
        case IOCTL_PW_MPP_CNFAN_IPUT:
            {
                unsigned char mpp_port;
                unsigned char ain_chn;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port = buf.req_buf[0];
                ain_chn  = buf.req_buf[1];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_mpp_config_analog_input(PM8921_MPP_PM_TO_SYS(mpp_port), ain_chn, PM8XXX_MPP_AOUT_CTRL_DISABLE);
#else
                ret = pm8058_mpp_config_analog_input(mpp_port, ain_chn, PM_MPP_AOUT_CTL_DISABLE);
#endif
                printk(KERN_INFO "DIAG_PW_MPP_CNFAN_IPUT mpp_port:%x ain_chn:%x", mpp_port, ain_chn);
            }
            break;
        /* Configure the selected MPP as an analog output. */
        case IOCTL_PW_MPP_CNFAN_OPUT:
            {
                unsigned char mpp_port;
                unsigned char aout_level;
                unsigned char pm_onoff;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                aout_level = buf.req_buf[1];
                pm_onoff   = buf.req_buf[2];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_mpp_config_analog_output(PM8921_MPP_PM_TO_SYS(mpp_port), aout_level, pm_onoff);
#else
                ret = pm8058_mpp_config_analog_output(mpp_port, aout_level, pm_onoff);
#endif
                printk(KERN_INFO "DIAG_PW_MPP_CNFAN_OPUT mpp_port:%x aout_level:%x pm_onoff:%x", mpp_port, aout_level, pm_onoff);
            }
            break;
        /* Configure the selected MPP as a current sink. */
        case IOCTL_PW_MPP_CNF_I_SINK:
            {
                unsigned char mpp_port;
                unsigned char sink_level;
                unsigned char pm_onoff;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                sink_level = buf.req_buf[1];
                pm_onoff   = buf.req_buf[2];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_mpp_config_current_sink(PM8921_MPP_PM_TO_SYS(mpp_port), sink_level, pm_onoff);
#else
                ret = pm8058_mpp_config_current_sink(mpp_port, sink_level, pm_onoff);
#endif
                printk(KERN_INFO "DIAG_PW_MPP_CNF_I_SINK mpp_port:%x sink_level:%x pm_onoff:%x", mpp_port, sink_level, pm_onoff);
            }
            break;

        /*------------------------------------------------------
            DIAG_PW_GPIO (0x310F)
        ------------------------------------------------------*/
        /* Set configuration of selected GPIO. */
        case IOCTL_PW_GPIO_CONFIG_SET:
            {
                ioctl_pw_value_type buf;
                struct pm_gpio param ;
                int gpio_port;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                gpio_port             = PM8921_GPIO_PM_TO_SYS(buf.req_buf[0]);
                param.direction       = buf.req_buf[1];
                param.output_buffer   = buf.req_buf[2];
                param.output_value    = buf.req_buf[3];
                param.pull            = buf.req_buf[4];
                param.vin_sel         = buf.req_buf[5];
                param.out_strength    = buf.req_buf[6];
                param.function        = buf.req_buf[7];
                param.inv_int_pol     = buf.req_buf[8];
                param.disable_pin     = buf.req_buf[9];
                
                ret = pm8xxx_gpio_config(gpio_port, &param);
                
                printk(KERN_INFO "IOCTL_PW_GPIO_CONFIG_SET gpio_port   :%x direction:%x output_buffer:%x", gpio_port, param.direction, param.output_buffer);
                printk(KERN_INFO "                         output_value:%x pull     :%x vin_sel      :%x", param.output_value, param.pull, param.vin_sel);
                printk(KERN_INFO "                         out_strength:%x function :%x inv_int_pol  :%x", param.out_strength, param.function, param.inv_int_pol);
                printk(KERN_INFO "                         pin_disable :%x                              ", param.disable_pin);
            }
            break;

        /* Get the current PM8058 over-temperature protection stage. */
        case IOCTL_PW_PCT_OTP_STAGE_GET:
            {
                unsigned char itemp_stage = 0x00;
                ioctl_pw_value_type buf;
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_itemp_get_stage(&itemp_stage);
#else
                ret = nc_pm8058_itemp_get_stage(&itemp_stage);
#endif

                buf.rsp_buf[0] = itemp_stage;
                
                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_PCT_OTP_STAGE_GET itemp_stage:%x", itemp_stage);
            }
            break;
        /* Override automatic shut down of the PM8058 over-temperature protection stage 2. */
        case IOCTL_PW_PCT_OTP_STG_OVD:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];
                
#ifdef CONFIG_FEATURE_NCMC_POWER
                ret = nc_pm8921_itemp_stage_override(enable);
#else
                ret = nc_pm8058_itemp_stage_override(enable,2); /* only PM_ITEMP_ORIDE_STAGE2 is supported */
#endif

                printk(KERN_INFO "DIAG_PW_PCT_OTP_STG_OVD enable:%x", enable);
            }
            break;

        /*------------------------------------------------------
            DIAG_PW_INTERRUPT (0x3112)
        ------------------------------------------------------*/
        /* Get the real-time status of a selected PMIC IRQ. */
        case IOCTL_PW_IR_RT_STATUS_GET:
            {
                unsigned int  rt_id = 0x00;
                unsigned int  rt_status = 0x00;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

#ifdef CONFIG_FEATURE_NCMC_POWER
                rt_id = (unsigned int)buf.req_buf[0];
                rt_id += PM8921_IRQ_BASE;
                nc_pm8921_get_rt_status(rt_id, &rt_status);
#else
                rt_id += (unsigned int)(buf.req_buf[0] << 24);
                rt_id += (unsigned int)(buf.req_buf[1] << 16);
                rt_id += (unsigned int)(buf.req_buf[2] << 8);
                rt_id += (unsigned int)buf.req_buf[3];
                
                nc_pm8058_get_rt_status(rt_id, &rt_status);
#endif
                
                buf.rsp_buf[0] = (u8)rt_status;

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                printk(KERN_INFO "DIAG_PW_IR_RT_STATUS_GET rt_id:%x rt_status:%x", rt_id, rt_status);
            }
            break;

        /* Hardware Reset */
        case IOCTL_PW_HW_RESET:
            printk(KERN_INFO "CHMC_FACTORY_DIAG_FTM_ONLINE_RESET_MODE");
            printk(KERN_DEBUG "[ncdiagd_power.c]%s: Goto arm_machine_restart() \n", __func__ );
            arm_machine_restart(0, NULL);
            break;

#ifdef CONFIG_FEATURE_NCMC_POWER
        /* TEMP SENSOR DEG */
        case IOCTL_PW_TSENSOR_GET_DEG:
            {
                unsigned char mpp_chnl;
                u16 temp = 0x0000;
                struct pm8xxx_adc_chan_result result;
                ioctl_pw_value_type buf;
                
                mpp_chnl = 12;

                ret = pm8xxx_adc_mpp_config_read(mpp_chnl, CHANNEL_MPP_1, &result);

                if(ret == PM_ERR_FLAG__SUCCESS)
                {
                    temp = nc_pm8921_tm_get_temp(result.physical);
                    buf.rsp_buf[0] = (temp >> 8) & 0xff;
                    buf.rsp_buf[1] = temp & 0xff;
                }

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_TS_GET_DEG adc_chnl :%x mpp_chnl :%x", CHANNEL_MPP_1, mpp_chnl);
                printk(KERN_INFO "                       adc_read_val(DEG):%x%x", 
                                       (int)((temp >> 8) & 0xff), (int)(temp & 0xff));
            }
            break;

        /* TEMP SENSOR AD */
        case IOCTL_PW_TSENSOR_GET_AD:
            {
                unsigned char mpp_chnl;
                struct pm8xxx_adc_chan_result result;
                ioctl_pw_value_type buf;
                
                mpp_chnl = 12;

                ret = pm8xxx_adc_mpp_config_read(mpp_chnl, CHANNEL_MPP_1, &result);

                if(ret == PM_ERR_FLAG__SUCCESS)
                {
                    buf.rsp_buf[0] = (result.physical >> 8) & 0xff;
                    buf.rsp_buf[1] = result.physical & 0xff;
                }

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_TS_GET_AD adc_chnl :%x mpp_chnl :%x", CHANNEL_MPP_1, mpp_chnl);
                printk(KERN_INFO "                       adc_read_val[0]:%x", (int)((result.physical >> 8) & 0xff));
                printk(KERN_INFO "                       adc_read_val[1]:%x", (int)(result.physical & 0xff));
            }
            break;

        /* TEMP SENSOR SWITCH ON/OFF */
        case IOCTL_PW_TSENSOR_SW:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_TS_GET_SW enable :%x", enable);

            }
            break;
#endif

#ifdef CONFIG_FEATURE_NCMC_POWER
        case IOCTL_PW_CHG_USB_DSBL:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];

                enable = enable ? 0x00 : 0x01;
                ret = nc_pm8921_chg_usb_suspend_enable(enable);

                printk(KERN_INFO "DIAG_PW_CHG_USB_DSBL enable:%x", enable);
            }
            break;
#endif

#ifdef CONFIG_FEATURE_NCMC_POWER
        /* GPIO STATE READ */
        case IOCTL_PW_GPIO_GET_STATE:
            {
                unsigned int gpio_id = 0x00;
                unsigned int gpio_state = 0x00;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                gpio_id = (unsigned int)buf.req_buf[0];
                ret = nc_pm8921_gpio_get_state(gpio_id, &gpio_state);
                
                buf.rsp_buf[0] = (u8)gpio_state;
                
                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_GPIO_GET_STATE GPIO :%d value :%d", gpio_id+1, gpio_state);
            }
            break;
#endif

#ifdef CONFIG_FEATURE_NCMC_POWER
        case IOCTL_PW_CHG_LVA_OFF:
            {
                unsigned char value;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                value = buf.req_buf[0];

                ret = nc_pm8921_chg_lva_off(value);

                printk(KERN_INFO "DIAG_PW_CHG_LVA_OFF value:%x", value);
            }
            break;
#endif


        default:
            printk(KERN_ERR "Invalid Parameter");
            return PM_ERR_FLAG__INVALID;
    }
    
    return ret;
}

static int ncdiagd_power_open(struct inode *ip, struct file *fp)
{
    printk(KERN_INFO "%s",__func__);
    if(access_count)
    {
        return -EBUSY;
    }
    access_count++;
    return 0;
}

static int ncdiagd_power_close(struct inode *ip, struct file *fp)
{
    printk(KERN_INFO "%s",__func__);
    access_count--;
    return 0;
}

static struct file_operations ncdiagd_power_fops = {
    .owner              = THIS_MODULE,
    .open               = ncdiagd_power_open,
    .release            = ncdiagd_power_close,
    .unlocked_ioctl     = ncdiagd_power_ioctl,
};

static struct miscdevice ncdiagd_power_dev = {
    .minor              = MISC_DYNAMIC_MINOR,
    .name               = "ncdiagd_power",
    .fops               = &ncdiagd_power_fops,
};

static int __init ncdiagd_power_init(void)
{
    int ret = 0;
    
    ret = misc_register(&ncdiagd_power_dev);
    if(ret)
    {
        printk(KERN_ERR "Fail to misc_register");
    }
    return ret;
}

static void __exit ncdiagd_power_cleanup(void)
{
    misc_deregister(&ncdiagd_power_dev);
}

late_initcall(ncdiagd_power_init);
module_exit(ncdiagd_power_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("NEC CASIO Mobile Communications PM8058 NCDIAG daemon");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:ncdiagd_power");
