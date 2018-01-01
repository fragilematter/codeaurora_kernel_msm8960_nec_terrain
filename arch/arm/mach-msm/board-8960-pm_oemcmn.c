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


#include <linux/regulator/pm8xxx-regulator.h>
#include <linux/regulator/gpio-regulator.h>
#include <linux/mfd/pm8xxx/mpp.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <mach/rpm-regulator.h>


#include "board-8960.h"
#include "board-8960-pm_cmn.h"

/*
 * PM8921 MPP Configs
 */



struct pm8xxx_mpp_init nc_pm8921_mpps[] __initdata = {
  /*             mpp  type      level                     control */
  /* MPP Config : Set in modem_pro                                               MPP 1 : UIM1_DATA_MSM  */
  /* MPP Config : Set in modem_pro                                               MPP 2 : UIM1_DATA_CONN */
  PM8XXX_MPP_INIT( 3, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW   ),  /* MPP 3 : TEMPSW_CTRL    */
  PM8XXX_MPP_INIT( 4, A_INPUT,  PM8XXX_MPP_AIN_AMUX_CH6,  DOUT_CTRL_LOW   ),  /* MPP 4 : HW_REVISION    */
  /* MPP Config : Same as Qualcomm Reference                                     MPP 5 : VREF_PADS      */
  /* MPP Config : Same as Qualcomm Reference                                     MPP 6 : VREF_DAC       */
  PM8XXX_MPP_INIT( 7, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW   ),  /* MPP 7 : BOOST_5V_EN    */
  PM8XXX_MPP_INIT( 8, SINK,     PM8XXX_MPP_CS_OUT_5MA,    CS_CTRL_DISABLE ),  /* MPP 8 : PA_THERM1      */
  PM8XXX_MPP_INIT( 9, D_INPUT,  PM8921_MPP_DIG_LEVEL_S4,  DIN_TO_INT      ),  /* MPP 9 : USBSW_OVP_VB_N */
  PM8XXX_MPP_INIT(10, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4,  DOUT_CTRL_LOW   ),  /* MPP10 : AUD_DDC_EN     */
  PM8XXX_MPP_INIT(11, D_INPUT,  PM8921_MPP_DIG_LEVEL_S4,  DIN_TO_INT      ),  /* MPP11 : USBSW_VB_DET_N */
  PM8XXX_MPP_INIT(12, SINK   ,  PM8XXX_MPP_CS_OUT_5MA  ,  CS_CTRL_DISABLE ),  /* MPP12 : XO_OUT_A2_EN   */
};

int nc_pm8921_mpps_num __initdata = ARRAY_SIZE(nc_pm8921_mpps);

/*
 * PM8921 GPIO Configs
 */


struct pm8xxx_gpio_init nc_pm8921_gpios[] __initdata = {
  /*              gpio dir               buf                  val pull                 vin              out_strength          func                 inv disable*/
  /* GPIO Config : Same as Qualcomm Reference                                                                                                                  GPIO 7 : CODEC_RST_N     */
  PM8XXX_GPIO_INIT( 8, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO 8 : UART_TX_MSM     */
  PM8XXX_GPIO_INIT(11, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO11 : AUD_AMP_RST_N   */
  PM8XXX_GPIO_INIT(12, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO12 : LED_DRV_RST_N   */
  PM8XXX_GPIO_INIT(13, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO13 : USBSW_VC_DET_N  */
  PM8XXX_GPIO_INIT(14, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_UP_1P5, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO14 : DTV_INT_N       */
  PM8XXX_GPIO_INIT(15, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO15 : FLC_TVDD_EN     */
  PM8XXX_GPIO_INIT(16, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO16 : EAR_SW          */
  PM8XXX_GPIO_INIT(17, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO17 : IRDA_SHDN       */
  PM8XXX_GPIO_INIT(18, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO18 : IRDA_2P85V_EN_N */
  PM8XXX_GPIO_INIT(19, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_VPH, PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO19 : MAIN_BAT_CON    */
  PM8XXX_GPIO_INIT(20, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO20 : USBSW_CHG_DET_N */
  PM8XXX_GPIO_INIT(21, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_2,       0,  0), /* GPIO21 : IRDA_UART_TX    */
  PM8XXX_GPIO_INIT(22, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_2,       0,  0), /* GPIO22 : FLC_UART_TX     */
  PM8XXX_GPIO_INIT(23, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_2,       0,  0), /* GPIO23 : ES310_UART_TX   */
  PM8XXX_GPIO_INIT(24, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_2,       0,  0), /* GPIO24 : HPT_PWM         */
  PM8XXX_GPIO_INIT(25, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO25 : CAM_V_EN2       */
  PM8XXX_GPIO_INIT(26, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO26 : SD_CARD_DET_N   */
  PM8XXX_GPIO_INIT(28, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO28 : HPT_SHDN        */
  PM8XXX_GPIO_INIT(31, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO31 : IOEX_RST_N      */
  PM8XXX_GPIO_INIT(32, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO32 : AUD_2P8V_EN     */
  PM8XXX_GPIO_INIT(33, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO33 : IRDA_UART_RX    */
  PM8XXX_GPIO_INIT(34, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_UP_1P5, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO34 : FLC_UART_RX     */
  PM8XXX_GPIO_INIT(35, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_UP_1P5, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO35 : ES310_UART_RX   */
  PM8XXX_GPIO_INIT(36, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO36 : DISP1_RST_N     */
  PM8XXX_GPIO_INIT(37, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO37 : USBSW_INT_N     */
  PM8XXX_GPIO_INIT(38, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1,       0,  0), /* GPIO38 : UART_RX_MSM     */
  /* GPIO Config : Same as Qualcomm Reference                                                                                                                  GPIO39 : CLK_MSM_FWD     */
  PM8XXX_GPIO_INIT(40, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,     PM_GPIO_VIN_VPH, PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO40 : IRDA_LEDA_EN_N  */
  PM8XXX_GPIO_INIT(41, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO41 : RESERVED        */
  PM8XXX_GPIO_INIT(42, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO42 : CAM2_V_EN1      */
  PM8XXX_GPIO_INIT(43, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO43 : SLEEP_CLK1      */
  PM8XXX_GPIO_INIT(44, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO44 : USBSW_OVP_VC_N  */
};

int nc_pm8921_gpios_num __initdata = ARRAY_SIZE(nc_pm8921_gpios);


/*
 * PM8921 keypad Configs
 */


struct pm_gpio nc_kypd_sns[] __devinitdata = {
  /*            direction        pull                 vin_sel          out_strength          function         inv */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 1 : KYPD_SNS1 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 2 : KYPD_SNS2 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 3 : KYPD_SNS3 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 4 : KYPD_SNS4 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 5 : KYPD_SNS5 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 6 : KYPD_SNS6 */
};


struct pm_gpio nc_kypd_drv[] __devinitdata = {
  /*            direction         output_buffer              value pull              vin_sel          out_strength          function        inv */
  KYPD_DRV_INIT(PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_OPEN_DRAIN,  0,  PM_GPIO_PULL_NO,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1, 1), /* GPIO 9 : KYPD_DRV1 */
  KYPD_DRV_INIT(PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_OPEN_DRAIN,  0,  PM_GPIO_PULL_NO,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1, 1), /* GPIO 10 : KYPD_DRV2 */
  KYPD_DRV_INIT(PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_OPEN_DRAIN,  0,  PM_GPIO_PULL_NO,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1, 1), /* GPIO 11 : KYPD_DRV3 */
  KYPD_DRV_INIT(PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_OPEN_DRAIN,  0,  PM_GPIO_PULL_NO,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1, 1), /* GPIO 12 : KYPD_DRV4 */
  KYPD_DRV_INIT(PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_OPEN_DRAIN,  0,  PM_GPIO_PULL_NO,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1, 1), /* GPIO 13 : KYPD_DRV5 */
  KYPD_DRV_INIT(PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_OPEN_DRAIN,  0,  PM_GPIO_PULL_NO,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1, 1), /* GPIO 14 : KYPD_DRV6 */
  KYPD_DRV_INIT(PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_OPEN_DRAIN,  0,  PM_GPIO_PULL_NO,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_1, 1), /* GPIO 15 : KYPD_DRV7 */
};


/*
 * PM8921 REGULATOR Configs
 */

/* GPIO regulator constraints */
struct gpio_regulator_platform_data nc_msm_gpio_regulator_pdata[] __devinitdata = {
  /*              _id      _reg_name   _gpio_label    _gpio */
  GPIO_VREG(EXT_5V,   "ext_5v",   "ext_5v_en",   PM8921_MPP_PM_TO_SYS(7), NULL  ),
  GPIO_VREG(EXT_L2,   "ext_l2",   "ext_l2_en",   91, NULL                       ),
  GPIO_VREG(EXT_3P3V, "ext_3p3v", "ext_3p3v_en", PM8921_GPIO_PM_TO_SYS(17), NULL),
  GPIO_VREG(EXT_OTG_SW, "ext_otg_sw", "ext_otg_sw_en",PM8921_GPIO_PM_TO_SYS(42), "8921_usb_otg"),
};

/* SAW regulator constraints */
struct regulator_init_data nc_msm_saw_regulator_pdata_s5 =
  /*            ID  vreg_name   min_uV   max_uV */
  SAW_VREG_INIT(S5, "8921_s5",  850000, 1300000);               
struct regulator_init_data nc_msm_saw_regulator_pdata_s6 =
  SAW_VREG_INIT(S6, "8921_s6",  850000, 1300000);               

/* PM8921 regulator constraints */
struct pm8xxx_regulator_platform_data
nc_msm_pm8921_regulator_pdata[] __devinitdata = {
  /*                        ID   name always_on pd min_uV   max_uV   en_t  supply   system_uA  reg_ID */
  PM8XXX_NLDO1200(L26, "8921_l26", 0, 1, 1050000, 1050000, 200, "8921_s7", 0, 1),
  PM8XXX_NLDO1200(L27, "8921_l27", 0, 1, 1050000, 1050000, 200, "8921_s7", 0, 2),
  PM8XXX_NLDO1200(L28, "8921_l28", 0, 1, 1050000, 1050000, 200, "8921_s7", 0, 3),
  PM8XXX_LDO(     L29, "8921_l29", 0, 1, 2050000, 2100000, 200, "8921_s8", 0, 4),

  /*                     ID    always_on pd enable_time supply */
  PM8XXX_VS300(USB_OTG,  "8921_usb_otg",  0,    1, 0,          "ext_5v", 5),
  PM8XXX_VS300(HDMI_MVS, "8921_hdmi_mvs", 0,    1, 0,          "ext_5v", 6),
};

static struct rpm_regulator_init_data
nc_msm_rpm_regulator_init_data[] __devinitdata = {
  /*       ID a_on pd ss min_uV   max_uV  supply sys_uA freq */
  RPM_SMPS(S1,  1, 1, 0, 1225000, 1225000, NULL, 100000, 3p20),
  RPM_SMPS(S2,  0, 1, 0, 1300000, 1300000, NULL, 0,      1p60),
  RPM_SMPS(S3,  0, 1, 1,  500000, 1150000, NULL, 100000, 4p80),
  RPM_SMPS(S4,  1, 1, 0, 1800000, 1800000, NULL, 100000, 1p60),
  RPM_SMPS(S7,  0, 1, 0, 1150000, 1150000, NULL, 100000, 3p20),
  RPM_SMPS(S8,  1, 1, 1, 2100000, 2100000, NULL, 100000, 1p60), 

  /*       ID a_on pd ss min_uV   max_uV   supply     sys_uA init_ip */
  RPM_LDO( L1,  1, 1, 0, 1050000, 1050000, "8921_s4", 0,     10000),
  RPM_LDO( L2,  0, 1, 0, 1200000, 1200000, "8921_s4", 0,     0),
  RPM_LDO( L3,  0, 1, 0, 3075000, 3075000, NULL,      0,     0),
  RPM_LDO( L4,  1, 1, 0, 1800000, 1800000, NULL,      10000, 10000),
  RPM_LDO( L5,  0, 1, 0, 2950000, 2950000, NULL,      0,     0),
  RPM_LDO( L6,  0, 1, 0, 2950000, 2950000, NULL,      0,     0),
  RPM_LDO( L7,  0, 1, 0, 1850000, 2950000, NULL,      10000, 10000),
  RPM_LDO( L8,  1, 1, 0, 2800000, 3000000, NULL,      0,     0),
  RPM_LDO( L9,  1, 1, 0, 3000000, 3000000, NULL,      0,     0),
  RPM_LDO(L10,	0, 1, 0, 3000000, 3000000, NULL,      0,     0),
#if defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
  RPM_LDO(L14,  1, 1, 0, 1800000, 1800000, NULL,      0,     0),
#else
  RPM_LDO(L14,  0, 1, 0, 1800000, 1800000, NULL,      0,     0),
#endif
  RPM_LDO(L15,  0, 1, 0, 1800000, 2950000, NULL,      0,     0),
  RPM_LDO(L17,  0, 1, 0, 1800000, 2950000, NULL,      0,     0),
  RPM_LDO(L18,  0, 1, 0, 1200000, 1200000, "8921_s4", 0,     0),
  RPM_LDO(L21,  0, 1, 0, 1900000, 1900000, "8921_s8", 0,     0),
  RPM_LDO(L22,  0, 1, 0, 2750000, 2750000, NULL,      0,     0),
  RPM_LDO(L23,  1, 1, 1, 1800000, 1800000, "8921_s8", 10000, 10000),
  RPM_LDO(L24,	0, 1, 1,  750000, 1150000, "8921_s1", 10000, 10000),
  RPM_LDO(L25,  1, 1, 0, 1225000, 1225000, "8921_s1", 10000, 10000),

  /*          ID a_on pd ss min_uV   max_uV   supply     sys_uA init_ip */
  RPM_LDO_FN(L11,  0, 1, 0, 2800000, 2800000, NULL,      0,     0,      RPM_VREG_PIN_FN_8960_MODE),
//  RPM_LDO_FN(L12,  1, 1, 0, 1200000, 1200000, "8921_s4", 0,     0,      RPM_VREG_PIN_FN_8960_MODE),
  RPM_LDO_FN(L12,  0, 1, 0, 1200000, 1200000, "8921_s4", 0,     0,      RPM_VREG_PIN_FN_8960_MODE),
  RPM_LDO_FN(L16,  0, 1, 0, 2800000, 2800000, NULL,      0,     0,      RPM_VREG_PIN_FN_8960_MODE),

  /*     ID   a_on pd ss  supply */
  RPM_VS(LVS1,  0, 1, 0,  "8921_s4"),
  RPM_VS(LVS2,  0, 1, 0,  "8921_s1"),
  RPM_VS(LVS3,  0, 1, 0,  "8921_s4"),
  RPM_VS(LVS4,  1, 1, 0,  "8921_s4"),
  RPM_VS(LVS5,  1, 1, 0,  "8921_s4"),
  RPM_VS(LVS6,  0, 1, 0,  "8921_s4"),
  RPM_VS(LVS7,  0, 1, 0,  "8921_s4"),

  /*      ID  a_on  ss min_uV   max_uV   supply     freq */
  RPM_NCP(NCP,  0,  0, 1800000, 1800000, "8921_l6", 1p60),
};

int nc_msm_pm8921_regulator_pdata_len __devinitdata =
  ARRAY_SIZE(nc_msm_pm8921_regulator_pdata);

struct rpm_regulator_platform_data nc_msm_rpm_regulator_pdata __devinitdata = {
  .init_data = nc_msm_rpm_regulator_init_data,
  .num_regulators = ARRAY_SIZE(nc_msm_rpm_regulator_init_data),
  .version = RPM_VREG_VERSION_8960,
  .vreg_id_vdd_mem = RPM_VREG_ID_PM8921_L24,
  .vreg_id_vdd_dig = RPM_VREG_ID_PM8921_S3,
};


/*
 * PM8921 CHARGER Configs
 */
 

static int pm8921_therm_mitigation[] = {
    1100,
    700,
    600,
    325,
};


struct pm8921_charger_platform_data nc_pm8921_chg_pdata __devinitdata = {
  .safety_time = 180,
  .update_time = 60000,
  .max_voltage = 4200,
  .min_voltage = 3200,
  #ifdef CONFIG_FEATURE_NCMC_POWER
  .resume_voltage_delta = 300,
  #else
  .resume_voltage_delta	= 100,
  #endif /* CONFIG_FEATURE_NCMC_POWER */
  .term_current = 100,
  .cool_temp = 10,
  .warm_temp = 40,
  .temp_check_period = 1,
  #if defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
  .max_bat_chg_current = 1300,
  #else
  .max_bat_chg_current = 1100,
  .cool_bat_chg_current = 350,
  .warm_bat_chg_current = 350,
  #endif /* CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960 */
  .cool_bat_voltage = 4100,
  .warm_bat_voltage = 4100,
  .thermal_mitigation = pm8921_therm_mitigation,
  .thermal_levels = ARRAY_SIZE(pm8921_therm_mitigation),
};


/*
 * PM8921 ADC Configs
 */
 

static struct pm8xxx_adc_amux nc_pm8xxx_adc_channels_data[] = {
  /* name, channel_name, adc_dev_instance, adc_access_fn, chan_path_type, adc_config_type, adc_calib_type, chan_processor  */
  {"vcoin",          CHANNEL_VCOIN,          CHAN_PATH_SCALING2,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"vbat",           CHANNEL_VBAT,           CHAN_PATH_SCALING2,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"dcin",           CHANNEL_DCIN,           CHAN_PATH_SCALING4,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"ichg",           CHANNEL_ICHG,           CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"vph_pwr",        CHANNEL_VPH_PWR,        CHAN_PATH_SCALING2,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"ibat",           CHANNEL_IBAT,           CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"m4",             CHANNEL_MPP_1,          CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"m5",             CHANNEL_MPP_2,          CHAN_PATH_SCALING2,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"batt_therm",     CHANNEL_BATT_THERM,     CHAN_PATH_SCALING1,  AMUX_RSV2,  ADC_DECIMATION_TYPE2,  ADC_SCALE_BATT_THERM  },
  {"batt_id",        CHANNEL_BATT_ID,        CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"usbin",          CHANNEL_USBIN,          CHAN_PATH_SCALING3,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"pmic_therm",     CHANNEL_DIE_TEMP,       CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_PMIC_THERM  },
  {"625mv",          CHANNEL_625MV,          CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"125v",           CHANNEL_125V,           CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"chg_temp",       CHANNEL_CHG_TEMP,       CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"pa_therm1",      ADC_MPP_1_AMUX8,        CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_PA_THERM    },
  {"xo_therm",       CHANNEL_MUXOFF,         CHAN_PATH_SCALING1,  AMUX_RSV0,  ADC_DECIMATION_TYPE2,  ADC_SCALE_XOTHERM     },
  {"pa_therm0",      ADC_MPP_1_AMUX3,        CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_PA_THERM    },
  {"batt_therm_mv",  CHANNEL_BATT_THERM_MV,  CHAN_PATH_SCALING1,  AMUX_RSV2,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"xo_therm_mv",    CHANNEL_MUXOFF_MV,      CHAN_PATH_SCALING1,  AMUX_RSV0,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
  {"pa_therm0_mv",   CHANNEL_PA_THERM0_MV,   CHAN_PATH_SCALING1,  AMUX_RSV1,  ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT     },
};


static struct pm8xxx_adc_properties pm8xxx_adc_data= {
  .adc_vdd_reference = 1800, /* milli-voltage for this adc */
  .bitresolution     = 15,
  .bipolar           = 0,
};

struct pm8xxx_adc_platform_data nc_pm8xxx_adc_pdata = {
  .adc_channel           = nc_pm8xxx_adc_channels_data,
  .adc_num_board_channel = ARRAY_SIZE(nc_pm8xxx_adc_channels_data),
  .adc_prop              = &pm8xxx_adc_data,
  .adc_mpp_base          = PM8921_MPP_PM_TO_SYS(1),
};


