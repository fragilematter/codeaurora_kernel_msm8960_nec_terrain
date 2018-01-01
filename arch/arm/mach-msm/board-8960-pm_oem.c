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
#include <mach/rpm-regulator.h>

#include "board-8960.h"
#include "board-8960-pm_cmn.h"

/*
 * PM8921 MPP Configs
 */


struct pm8xxx_mpp_init nc_pm8921_mpps_oem[] __initdata = {
  /*             mpp  type      level                     control */
  /* MPP Config : Set in modem_pro                                               MPP 1 : UIM1_DATA_MSM  */
  /* MPP Config : Set in modem_pro                                               MPP 2 : UIM1_DATA_CONN */
  PM8XXX_MPP_INIT( 3, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW   ),  /* MPP 3 : TEMPSW_CTRL    */
  PM8XXX_MPP_INIT( 4, A_INPUT,  PM8XXX_MPP_AIN_AMUX_CH6,  DOUT_CTRL_LOW   ),  /* MPP 4 : HW_REVISION    */
  /* MPP Config : Same as Qualcomm Reference                                     MPP 5 : VREF_PADS      */
  /* MPP Config : Same as Qualcomm Reference                                     MPP 6 : VREF_DAC       */
  PM8XXX_MPP_INIT( 7, SINK,     PM8XXX_MPP_CS_OUT_5MA,    CS_CTRL_DISABLE ),  /* MPP 7 : RESERVED       */
  PM8XXX_MPP_INIT( 8, SINK,     PM8XXX_MPP_CS_OUT_5MA,    CS_CTRL_DISABLE ),  /* MPP 8 : RESERVED       */
  PM8XXX_MPP_INIT( 9, SINK,     PM8XXX_MPP_CS_OUT_5MA,    CS_CTRL_DISABLE ),  /* MPP 9 : RESERVED       */
  PM8XXX_MPP_INIT(10, SINK,     PM8XXX_MPP_CS_OUT_5MA,    CS_CTRL_DISABLE ),  /* MPP10 : RESERVED       */
  PM8XXX_MPP_INIT(11, SINK,     PM8XXX_MPP_CS_OUT_5MA,    CS_CTRL_DISABLE ),  /* MPP11 : RESERVED       */
  PM8XXX_MPP_INIT(12, SINK,     PM8XXX_MPP_CS_OUT_5MA,    CS_CTRL_DISABLE ),  /* MPP12 : RESERVED       */
};

int nc_pm8921_mpps_oem_num __initdata = ARRAY_SIZE(nc_pm8921_mpps_oem);

/*
 * PM8921 GPIO Configs
 */


struct pm8xxx_gpio_init nc_pm8921_gpios_oem[] __initdata = {
  /*              gpio dir               buf                  val pull                 vin              out_strength          func                 inv disable*/
  /* GPIO Config : Same as Qualcomm Reference                                                                                                                  GPIO 7 : CODEC_RST_N     */
 PM8XXX_GPIO_INIT( 7, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_HIGH,PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO7 : */
  PM8XXX_GPIO_INIT( 8, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0),    /* GPIO 8 : UART_TX_MSM     */
  PM8XXX_GPIO_INIT(16, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO16 : RESERVED        */
  PM8XXX_GPIO_INIT(17, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_DN, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO17 : RESERVED        */
  PM8XXX_GPIO_INIT(18, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO18 : RESERVED        */
  PM8XXX_GPIO_INIT(19, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_VPH, PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO19 : MAIN_BAT_CON    */
  PM8XXX_GPIO_INIT(20, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO20 : ES310_RST_N     */
  PM8XXX_GPIO_INIT(21, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_DN, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO21 : RESERVED        */
  PM8XXX_GPIO_INIT(22, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_DN, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO22 : RESERVED        */
  PM8XXX_GPIO_INIT(23, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO23 : ES310_UART_TX   */
  PM8XXX_GPIO_INIT(24, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_DN, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO24 : RESERVED        */
  PM8XXX_GPIO_INIT(25, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO25 : CAM_V_EN2       */
  PM8XXX_GPIO_INIT(26, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO26 : SD_CARD_DET_N   */
  /* GPIO Config : Same as Qualcomm Reference                                                                                                                     GPIO27 : UIM1_RST_CONN   */
  PM8XXX_GPIO_INIT(28, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_DN, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO28 : RESERVED        */
  /* GPIO Config : Same as Qualcomm Reference                                                                                                                     GPIO29 : UIM1_CLK_MSM    */
  /* GPIO Config : Same as Qualcomm Reference                                                                                                                     GPIO30 : UIM1_CLK_CONN   */
 PM8XXX_GPIO_INIT(31, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_HIGH,PM_GPIO_FUNC_NORMAL,  0,  0),  /* GPIO11 : D_AMP_CTL1      */
  PM8XXX_GPIO_INIT(32, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_HIGH,PM_GPIO_FUNC_NORMAL,  0,  0),  /* GPIO11 : D_AMP_CTL1      */
  PM8XXX_GPIO_INIT(33, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO33 : NFC_VTX_EN      */
  PM8XXX_GPIO_INIT(34, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_HIGH,PM_GPIO_FUNC_NORMAL,  0,  0),  /* GPIO11 : D_AMP_CTL1      */
  PM8XXX_GPIO_INIT(35, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_HIGH,PM_GPIO_FUNC_NORMAL,  0,  0),  /* GPIO11 : D_AMP_CTL1      */
  PM8XXX_GPIO_INIT(36, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO36 : DISP1_RST_N     */
  PM8XXX_GPIO_INIT(37, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_DN, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO37 : RESERVED        */
  PM8XXX_GPIO_INIT(38, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW,  PM_GPIO_FUNC_1,       0,  0), /* GPIO38 : UART_RX_MSM     */
  /* GPIO Config : Same as Qualcomm Reference                                                                                                                  GPIO39 : CLK_MSM_FWD     */
  PM8XXX_GPIO_INIT(40, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_NO, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO40 : ES310_RST_N     */
  PM8XXX_GPIO_INIT(41, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO41 : RESERVED        */
  PM8XXX_GPIO_INIT(42, PM_GPIO_DIR_OUT,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO42 : CAM2_V_EN1      */
  PM8XXX_GPIO_INIT(43, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,     PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO43 : RESERVED        */
  PM8XXX_GPIO_INIT(44, PM_GPIO_DIR_IN,   PM_GPIO_OUT_BUF_CMOS,       0, PM_GPIO_PULL_DN, PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL,  0,  0), /* GPIO44 : RESERVED        */
};

int nc_pm8921_gpios_oem_num __initdata = ARRAY_SIZE(nc_pm8921_gpios_oem);


/*
 * PM8921 keypad Configs
 */

struct pm_gpio nc_kypd_sns_oem[] __devinitdata = {
  /*            direction        pull                 vin_sel          out_strength          function         inv */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 1 : KYPD_SNS1 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 2 : KYPD_SNS2 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 3 : KYPD_SNS3 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 4 : KYPD_SNS4 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 5 : KYPD_SNS5 */
  KYPD_SNS_INIT(PM_GPIO_DIR_IN,  PM_GPIO_PULL_UP_30,  PM_GPIO_VIN_S4,  PM_GPIO_STRENGTH_NO,  PM_GPIO_FUNC_1,  0), /* GPIO 6 : KYPD_SNS6 */
};


struct pm_gpio nc_kypd_drv_oem[] __devinitdata = {
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
 * RPM REGULATOR Configs
 */


/* GPIO regulator constraints */
struct gpio_regulator_platform_data nc_msm_gpio_regulator_pdata_oem[] __devinitdata = {
  /*              _id      _reg_name   _gpio_label    _gpio */
  GPIO_VREG(EXT_5V,   "ext_5v",   "ext_5v_en",   PM8921_MPP_PM_TO_SYS(7), NULL  ),
  GPIO_VREG(EXT_L2,   "ext_l2",   "ext_l2_en",   91, NULL                       ),
  GPIO_VREG(EXT_3P3V, "ext_3p3v", "ext_3p3v_en", PM8921_GPIO_PM_TO_SYS(17), NULL),
  GPIO_VREG(EXT_OTG_SW, "ext_otg_sw", "ext_otg_sw_en",PM8921_GPIO_PM_TO_SYS(42), "8921_usb_otg"),
};

/* SAW regulator constraints */
struct regulator_init_data nc_msm_saw_regulator_pdata_s5_oem =
  /*            ID  vreg_name   min_uV   max_uV */
  SAW_VREG_INIT(S5, "8921_s5",  850000, 1300000);
struct regulator_init_data nc_msm_saw_regulator_pdata_s6_oem =
  SAW_VREG_INIT(S6, "8921_s6",  850000, 1300000);

/* PM8921 regulator constraints */
struct pm8xxx_regulator_platform_data
nc_msm_pm8921_regulator_pdata_oem[] __devinitdata = {
  /*                        ID   name always_on pd min_uV   max_uV   en_t  supply   system_uA  reg_ID */
  PM8XXX_NLDO1200(L26, "8921_l26", 0, 1, 1050000, 1050000, 200, "8921_s7", 0, 1),
  PM8XXX_NLDO1200(L27, "8921_l27", 0, 1, 1050000, 1050000, 200, "8921_s7", 0, 2),
  PM8XXX_NLDO1200(L28, "8921_l28", 0, 1, 1050000, 1050000, 200, "8921_s7", 0, 3),
  PM8XXX_LDO(     L29, "8921_l29", 0, 1, 1900000, 1900000, 200, "8921_s8", 0, 4), 

  /*                     ID    always_on pd enable_time supply */
  PM8XXX_VS300(USB_OTG,  "8921_usb_otg",  0,    1, 0,          "ext_5v", 5),
  PM8XXX_VS300(HDMI_MVS, "8921_hdmi_mvs", 0,    1, 0,          "ext_5v", 6),
};


VREG_CONSUMERS(L16_PC)   = { REGULATOR_SUPPLY("8921_l16_pc",     NULL                      ),};

static struct rpm_regulator_init_data
nc_msm_rpm_regulator_init_data_oem[] __devinitdata = {
  /*       ID a_on pd ss min_uV   max_uV  supply sys_uA freq */
  RPM_SMPS(S1,  1, 1, 0, 1225000, 1225000, NULL, 100000, 3p20),
  RPM_SMPS(S2,  0, 1, 0, 1300000, 1300000, NULL, 0,      1p60),
  RPM_SMPS(S3,  0, 1, 1,  500000, 1150000, NULL, 100000, 4p80),
  RPM_SMPS(S4,  1, 1, 0, 1800000, 1800000, NULL, 100000, 1p60),
  RPM_SMPS(S7,  0, 1, 0, 1150000, 1150000, NULL, 100000, 3p20),
  RPM_SMPS(S8,  1, 1, 1, 2050000, 2050000, NULL, 100000, 1p60),

  /*       ID a_on pd ss min_uV   max_uV   supply     sys_uA init_ip */
  RPM_LDO( L1,  1, 1, 0, 1050000, 1050000, "8921_s4", 0,     10000),
  RPM_LDO( L2,  0, 1, 0, 1200000, 1200000, "8921_s4", 0,     0),
  RPM_LDO( L3,  0, 1, 0, 3075000, 3075000, NULL,      0,     0),
  RPM_LDO( L4,  1, 1, 0, 1800000, 1800000, NULL,      10000, 10000),
  RPM_LDO( L5,  0, 1, 0, 2950000, 2950000, NULL,      0,     0),
  RPM_LDO( L6,  0, 1, 0, 2950000, 2950000, NULL,      0,     0),
  RPM_LDO( L7,  0, 1, 0, 1850000, 2950000, NULL,      10000, 10000),
  RPM_LDO( L8,  0, 1, 0, 2800000, 2800000, NULL,      0,     0), 
  RPM_LDO( L9,  1, 1, 0, 2850000, 2850000, NULL,      0,     0), 
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
  RPM_LDO(L22,  0, 1, 0, 2600000, 2600000, NULL,      0,     0), 
  RPM_LDO(L23,  1, 0, 1, 1800000, 1800000, "8921_s8", 10000, 10000),
  RPM_LDO(L24,	0, 1, 1,  750000, 1150000, "8921_s1", 10000, 10000),
  RPM_LDO(L25,  1, 1, 0, 1225000, 1225000, "8921_s1", 10000, 10000),

  /*          ID a_on pd ss min_uV   max_uV   supply     sys_uA init_ip */
  RPM_LDO_FN(L11,  0, 1, 0, 2800000, 2800000, NULL,      0,     0,      RPM_VREG_PIN_FN_8960_NONE),
  RPM_LDO_FN(L12,  0, 1, 0, 1200000, 1200000, "8921_s4", 0,     0,      RPM_VREG_PIN_FN_8960_NONE),
  RPM_LDO_FN(L16,  0, 1, 0, 3000000, 3000000, NULL,      0,     0,      RPM_VREG_PIN_FN_8960_NONE),

  /*     ID   a_on pd ss  supply */
  RPM_VS(LVS1,  0, 1, 0,  "8921_s4"),
  RPM_VS(LVS2,  0, 1, 0,  "8921_s1"),
  RPM_VS(LVS3,  0, 1, 0,  "8921_s4"),
  RPM_VS(LVS4,  1, 1, 0,  "8921_s4"),
  RPM_VS(LVS5,  0, 1, 0,  "8921_s4"),
  RPM_VS(LVS6,  0, 1, 0,  "8921_s4"),
  RPM_VS(LVS7,  0, 1, 0,  "8921_s4"),

  /*      ID  a_on  ss min_uV   max_uV   supply     freq */
  RPM_NCP(NCP,  0,  0, 1800000, 1800000, "8921_l6", 1p60),

  /*          ID   a_on   pin_fn   pin_ctrl     supply */
  RPM_PC_INIT(L16,   0,   SLEEP_B,    NONE,   "8921_l16"),
};


struct rpm_regulator_platform_data nc_msm_rpm_regulator_pdata_oem __devinitdata = {
  .init_data = nc_msm_rpm_regulator_init_data_oem,
  .num_regulators = ARRAY_SIZE(nc_msm_rpm_regulator_init_data_oem),
  .version = RPM_VREG_VERSION_8960,
  .vreg_id_vdd_mem = RPM_VREG_ID_PM8921_L24,
  .vreg_id_vdd_dig = RPM_VREG_ID_PM8921_S3,
};

