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
#include <linux/mfd/pm8xxx/mpp.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <mach/rpm-regulator.h>

#include "board-8960.h"

/*
 * PM8921 MPP Configs
 */

#define PM8XXX_MPP_INIT(_mpp, _type, _level, _control) \
{\
  .mpp  = PM8921_MPP_PM_TO_SYS(_mpp),   \
  .config = {                           \
    .type   = PM8XXX_MPP_TYPE_##_type,  \
    .level    = _level,                 \
    .control  = PM8XXX_MPP_##_control,  \
  }\
}


/*
 * PM8921 GPIO Configs
 */

#define PM8XXX_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, _func, _inv, _disable) \
{\
  .gpio = PM8921_GPIO_PM_TO_SYS(_gpio), \
  .config = {                           \
    .direction      = _dir,             \
    .output_buffer  = _buf,             \
    .output_value   = _val,             \
    .pull           = _pull,            \
    .vin_sel        = _vin,             \
    .out_strength   = _out_strength,    \
    .function       = _func,            \
    .inv_int_pol    = _inv,             \
    .disable_pin    = _disable,         \
  }\
}

/*
 * PM8921 keypad Configs
 */

#define KYPD_SNS_INIT(_dir, _pull, _vin_sel, _out_strength, _func, _inv) \
{\
    .direction    = _dir,            \
    .pull         = _pull,           \
    .vin_sel      = _vin_sel,        \
    .out_strength = _out_strength,   \
    .function     = _func,           \
    .inv_int_pol  = _inv,            \
}


#define KYPD_DRV_INIT(_dir, _out_buf, _out_val, _pull, _vin_sel, _out_strength, _func, _inv) \
{\
  .direction     = _dir,             \
  .output_buffer = _out_buf,         \
  .output_value  = _out_val,         \
  .pull          = _pull,            \
  .vin_sel       = _vin_sel,         \
  .out_strength  = _out_strength,    \
  .function      = _func,            \
  .inv_int_pol   = _inv,             \
}



/*
 * PM8921 Regulator Configs
 */

#define VREG_CONSUMERS(_id) static struct regulator_consumer_supply vreg_consumers_##_id[]

/* supply -> device mapping                   Supply Name        Device Name               */
VREG_CONSUMERS(L1)       = { REGULATOR_SUPPLY("8921_l1",         NULL                      ),};
VREG_CONSUMERS(L2)       = { REGULATOR_SUPPLY("8921_l2",         NULL                      ),
                             REGULATOR_SUPPLY("dsi_vdda",        "mipi_dsi.1"              ),
                             REGULATOR_SUPPLY("mipi_csi_vdd",    "msm_camera_imx111.0"     ), /* NCMC_IMX111_CHG */
                             REGULATOR_SUPPLY("mipi_csi_vdd",    "msm_csid.0"              ),
                             REGULATOR_SUPPLY("mipi_csi_vdd",    "msm_csid.1"              ),
                             REGULATOR_SUPPLY("mipi_csi_vdd",    "4-002d"		           ),
/* SUBCAMERA MT9M113 START */
                             REGULATOR_SUPPLY("mipi_csi_vdd",    "msm_csid.2"              ),};
/* SUBCAMERA MT9M113 END */
VREG_CONSUMERS(L3)       = { REGULATOR_SUPPLY("8921_l3",         NULL                      ),
                             REGULATOR_SUPPLY("HSUSB_3p3",       "msm_otg"                 ),};
VREG_CONSUMERS(L4)       = { REGULATOR_SUPPLY("8921_l4",         NULL                      ),
                             REGULATOR_SUPPLY("HSUSB_1p8",       "msm_otg"                 ),
                             REGULATOR_SUPPLY("iris_vddxo",      "wcnss_wlan.0"            ),};
VREG_CONSUMERS(L5)       = { REGULATOR_SUPPLY("8921_l5",         NULL                      ),
                             REGULATOR_SUPPLY("sdc_vdd",         "msm_sdcc.1"              ),};
VREG_CONSUMERS(L6)       = { REGULATOR_SUPPLY("8921_l6",         NULL                      ),
                             REGULATOR_SUPPLY("sdc_vdd",         "msm_sdcc.3"              ),};
VREG_CONSUMERS(L7)       = { REGULATOR_SUPPLY("8921_l7",         NULL                      ),
                             REGULATOR_SUPPLY("sdc_vdd_io",      "msm_sdcc.3"              ),};
VREG_CONSUMERS(L8)       = { REGULATOR_SUPPLY("8921_l8",         NULL                      ),
                             REGULATOR_SUPPLY("dsi_vdc",         "mipi_dsi.1"              ),};
VREG_CONSUMERS(L9)       = { REGULATOR_SUPPLY("8921_l9",         NULL                      ),
                             REGULATOR_SUPPLY("vdd",             "3-0024"                  ),
                             REGULATOR_SUPPLY("vdd_ana",         "3-004a"                  ),};
VREG_CONSUMERS(L10)      = { REGULATOR_SUPPLY("8921_l10",        NULL                      ),
                             REGULATOR_SUPPLY("iris_vddpa",      "wcnss_wlan.0"            ),};
VREG_CONSUMERS(L11)      = { REGULATOR_SUPPLY("8921_l11",        NULL                      ),
                             REGULATOR_SUPPLY("cam_vana",        "msm_camera_imx111.0"     ), /* NCMC_IMX111_CHG */
                             REGULATOR_SUPPLY("cam_vana",        "4-001a"                  ),
                             REGULATOR_SUPPLY("cam_vana",        "4-006c"                  ),
                             REGULATOR_SUPPLY("cam_vana",        "4-0048"                  ), /* [Q89-PM-018] CHG */
                             REGULATOR_SUPPLY("cam_vana",        "4-0020"                  ),
                             REGULATOR_SUPPLY("cam_vaf",        "4-002d"                  ),
                             REGULATOR_SUPPLY("cam_vana",        "4-0034"                  ),};
VREG_CONSUMERS(L12)      = { REGULATOR_SUPPLY("8921_l12",        NULL                      ),
                             REGULATOR_SUPPLY("cam_vdig",        "msm_camera_imx111.0"     ), /* NCMC_IMX111_CHG */
                             REGULATOR_SUPPLY("cam_vdig",        "4-001a"                  ),
                             REGULATOR_SUPPLY("cam_vdig",        "4-006c"                  ),
                             REGULATOR_SUPPLY("cam_vdig",        "4-0048"                  ), /* [Q89-PM-018] CHG */
                             REGULATOR_SUPPLY("cam_vdig",        "4-0020"                  ),
                             REGULATOR_SUPPLY("cam_vdig",        "4-002d"                  ), /* S5K4ECGX */
                             REGULATOR_SUPPLY("cam_vdig",        "4-0034"                  ),};
VREG_CONSUMERS(L14)      = { REGULATOR_SUPPLY("8921_l14",        NULL                      ),
                             REGULATOR_SUPPLY("pa_therm",        "pm8xxx-adc"              ),};
VREG_CONSUMERS(L15)      = { REGULATOR_SUPPLY("8921_l15",        NULL                      ),};
VREG_CONSUMERS(L16)      = { REGULATOR_SUPPLY("8921_l16",        NULL                      ),
                             REGULATOR_SUPPLY("cam_vaf",         "msm_camera_imx111.0"     ), /* NCMC_IMX111_CHG */
                             REGULATOR_SUPPLY("cam_vaf",         "4-001a"                  ),
                             REGULATOR_SUPPLY("cam_vaf",         "4-006c"                  ),
                             REGULATOR_SUPPLY("cam_vaf",         "4-0048"                  ), /* [Q89-PM-018] CHG */
                             REGULATOR_SUPPLY("cam_vaf",         "4-0020"                  ),
                             REGULATOR_SUPPLY("cam_vaf",         "4-0034"                  ),};
VREG_CONSUMERS(L17)      = { REGULATOR_SUPPLY("8921_l17",        NULL                      ),};
VREG_CONSUMERS(L18)      = { REGULATOR_SUPPLY("8921_l18",        NULL                      ),};
VREG_CONSUMERS(L21)      = { REGULATOR_SUPPLY("8921_l21",        NULL                      ),};
VREG_CONSUMERS(L22)      = { REGULATOR_SUPPLY("8921_l22",        NULL                      ),};
VREG_CONSUMERS(L23)      = { REGULATOR_SUPPLY("8921_l23",        NULL                      ),
                             REGULATOR_SUPPLY("dsi_vddio",       "mipi_dsi.1"              ),
                             REGULATOR_SUPPLY("hdmi_avdd",       "hdmi_msm.0"              ),
                             REGULATOR_SUPPLY("pll_vdd",         "pil_riva"                ),
                             REGULATOR_SUPPLY("pll_vdd",         "pil_qdsp6v4.1"           ),
                             REGULATOR_SUPPLY("pll_vdd",         "pil_qdsp6v4.2"           ),};
VREG_CONSUMERS(L24)      = { REGULATOR_SUPPLY("8921_l24",        NULL                      ),
                             REGULATOR_SUPPLY("riva_vddmx",      "wcnss_wlan.0"            ),};
VREG_CONSUMERS(L25)      = { REGULATOR_SUPPLY("8921_l25",        NULL                      ),
                             REGULATOR_SUPPLY("VDDD_CDC_D",      "tabla-slim"              ),
                             REGULATOR_SUPPLY("CDC_VDDA_A_1P2V", "tabla-slim"              ),
                             REGULATOR_SUPPLY("VDDD_CDC_D",      "tabla2x-slim"            ),
                             REGULATOR_SUPPLY("CDC_VDDA_A_1P2V", "tabla2x-slim"            ),};
VREG_CONSUMERS(L26)      = { REGULATOR_SUPPLY("8921_l26",        NULL                      ),
                             REGULATOR_SUPPLY("core_vdd",        "pil_qdsp6v4.0"           ),}; /* [Q89-PM-018] CHG */
VREG_CONSUMERS(L27)      = { REGULATOR_SUPPLY("8921_l27",        NULL                      ),
                             REGULATOR_SUPPLY("core_vdd",        "pil_qdsp6v4.2"           ),}; /* [Q89-PM-018] CHG */
VREG_CONSUMERS(L28)      = { REGULATOR_SUPPLY("8921_l28",        NULL                      ),
                             REGULATOR_SUPPLY("core_vdd",        "pil_qdsp6v4.1"           ),}; /* [Q89-PM-018] CHG */
VREG_CONSUMERS(L29)      = { REGULATOR_SUPPLY("8921_l29",        NULL                      ),};
VREG_CONSUMERS(S1)       = { REGULATOR_SUPPLY("8921_s1",         NULL                      ),};
VREG_CONSUMERS(S2)       = { REGULATOR_SUPPLY("8921_s2",         NULL                      ),
                             REGULATOR_SUPPLY("iris_vddrfa",     "wcnss_wlan.0"            ),};
VREG_CONSUMERS(S3)       = { REGULATOR_SUPPLY("8921_s3",         NULL                      ),
                             REGULATOR_SUPPLY("HSUSB_VDDCX",     "msm_otg"                 ),
                             REGULATOR_SUPPLY("riva_vddcx",      "wcnss_wlan.0"            ),};
VREG_CONSUMERS(S4)       = { REGULATOR_SUPPLY("8921_s4",         NULL                      ),
                             REGULATOR_SUPPLY("sdc_vdd_io",      "msm_sdcc.1"              ),
                             REGULATOR_SUPPLY("sdc_vdd",         "msm_sdcc.2"              ),
                             REGULATOR_SUPPLY("sdc_vdd_io",      "msm_sdcc.4"              ),
                             REGULATOR_SUPPLY("riva_vddpx",      "wcnss_wlan.0"            ),
                             REGULATOR_SUPPLY("hdmi_vcc",        "hdmi_msm.0"              ),
                             REGULATOR_SUPPLY("VDDIO_CDC",       "tabla-slim"              ),
                             REGULATOR_SUPPLY("CDC_VDD_CP",      "tabla-slim"              ),
                             REGULATOR_SUPPLY("CDC_VDDA_TX",     "tabla-slim"              ),
                             REGULATOR_SUPPLY("CDC_VDDA_RX",     "tabla-slim"              ),
                             REGULATOR_SUPPLY("VDDIO_CDC",       "tabla2x-slim"            ),
                             REGULATOR_SUPPLY("CDC_VDD_CP",      "tabla2x-slim"            ),
                             REGULATOR_SUPPLY("CDC_VDDA_TX",     "tabla2x-slim"            ),
                             REGULATOR_SUPPLY("CDC_VDDA_RX",     "tabla2x-slim"            ),
                             REGULATOR_SUPPLY("vcc_i2c",         "3-005b"                  ),
                             REGULATOR_SUPPLY("EXT_HUB_VDDIO",   "msm_hsic_host"           ),
                             REGULATOR_SUPPLY("vcc_i2c",         "10-0048"                 ),};
VREG_CONSUMERS(S5)       = { REGULATOR_SUPPLY("8921_s5",         NULL                      ),
                             REGULATOR_SUPPLY("krait0",          NULL                      ),};
VREG_CONSUMERS(S6)       = { REGULATOR_SUPPLY("8921_s6",         NULL                      ),
                             REGULATOR_SUPPLY("krait1",          NULL                      ),};
VREG_CONSUMERS(S7)       = { REGULATOR_SUPPLY("8921_s7",         NULL                      ),};
VREG_CONSUMERS(S8)       = { REGULATOR_SUPPLY("8921_s8",         NULL                      ),};
VREG_CONSUMERS(LVS1)     = { REGULATOR_SUPPLY("8921_lvs1",       NULL                      ),
                             REGULATOR_SUPPLY("iris_vddio",      "wcnss_wlan.0"            ),};
VREG_CONSUMERS(LVS2)     = { REGULATOR_SUPPLY("8921_lvs2",       NULL                      ),
                             REGULATOR_SUPPLY("iris_vdddig",     "wcnss_wlan.0"            ),};
VREG_CONSUMERS(LVS3)     = { REGULATOR_SUPPLY("8921_lvs3",       NULL                      ),};
VREG_CONSUMERS(LVS4)     = { REGULATOR_SUPPLY("8921_lvs4",       NULL                      ),
                             REGULATOR_SUPPLY("vcc_i2c",         "3-0024"                  ),
                             REGULATOR_SUPPLY("vcc_i2c",         "3-004a"                  ),};
VREG_CONSUMERS(LVS5)     = { REGULATOR_SUPPLY("8921_lvs5",       NULL                      ),
                             REGULATOR_SUPPLY("cam_vio",         "msm_camera_imx111.0"     ), /* NCMC_IMX111_CHG */
                             REGULATOR_SUPPLY("cam_vio",         "4-001a"                  ),
                             REGULATOR_SUPPLY("cam_vio",         "4-006c"                  ),
                             REGULATOR_SUPPLY("cam_vio",         "4-0048"                  ), /* [Q89-PM-018] CHG */
                             REGULATOR_SUPPLY("cam_vio",         "4-0020"                  ),
                             REGULATOR_SUPPLY("cam_vio",         "4-0034"                  ),};
VREG_CONSUMERS(LVS6)     = { REGULATOR_SUPPLY("8921_lvs6",       NULL                      ),
                             REGULATOR_SUPPLY("vdd_io",          "spi0.0"                  ),};
VREG_CONSUMERS(LVS7)     = { REGULATOR_SUPPLY("8921_lvs7",       NULL                      ),};
VREG_CONSUMERS(USB_OTG)  = { REGULATOR_SUPPLY("8921_usb_otg",    NULL                      ),};
VREG_CONSUMERS(HDMI_MVS) = { REGULATOR_SUPPLY("8921_hdmi_mvs",   NULL                      ),
                             REGULATOR_SUPPLY("hdmi_mvs",        "hdmi_msm.0"              ),};
VREG_CONSUMERS(NCP)      = { REGULATOR_SUPPLY("8921_ncp",        NULL                      ),};
VREG_CONSUMERS(EXT_5V)   = { REGULATOR_SUPPLY("ext_5v",          NULL                      ),};
VREG_CONSUMERS(EXT_L2)   = { REGULATOR_SUPPLY("ext_l2",          NULL                      ),
                             REGULATOR_SUPPLY("vdd_phy",         "spi0.0"                  ),};
VREG_CONSUMERS(EXT_3P3V) = { REGULATOR_SUPPLY("ext_3p3v",        NULL                      ),
                             REGULATOR_SUPPLY("vdd_ana",         "3-005b"                  ),
                             REGULATOR_SUPPLY("vdd_lvds_3p3v",   "mipi_dsi.1"              ),
                             REGULATOR_SUPPLY("mhl_ext_3p3v",    "msm_otg"                 ),};
VREG_CONSUMERS(EXT_OTG_SW) = {REGULATOR_SUPPLY("ext_otg_sw",     NULL                      ),
                              REGULATOR_SUPPLY("vbus_otg",       "msm_otg"                 ),};

#define PM8XXX_VREG_INIT(_id, _name, _min_uV, _max_uV, _modes, _ops, _apply_uV, \
       _pull_down, _always_on, _supply_regulator, _system_uA, _enable_time, _reg_id) \
{\
  .init_data = {                                                \
    .constraints = {                                            \
      .valid_modes_mask     = _modes,                           \
      .valid_ops_mask       = _ops,                             \
      .min_uV               = _min_uV,                          \
      .max_uV               = _max_uV,                          \
      .input_uV             = _max_uV,                          \
      .apply_uV             = _apply_uV,                        \
      .always_on            = _always_on,                       \
      .name                 = _name,                            \
    },                                                          \
    .num_consumer_supplies  = ARRAY_SIZE(vreg_consumers_##_id), \
    .consumer_supplies      = vreg_consumers_##_id,             \
    .supply_regulator       = _supply_regulator,                \
  },                                                            \
  .id                       = _reg_id,                          \
  .pull_down_enable         = _pull_down,                       \
  .system_uA                = _system_uA,                       \
  .enable_time              = _enable_time,                     \
}

#define PM8XXX_LDO(_id, _name, _always_on, _pull_down, _min_uV, _max_uV, _enable_time, _supply_regulator, _system_uA, _reg_id) \
PM8XXX_VREG_INIT( \
  _id,                                                  /* _id                */ \
  _name,                                                /* _name              */ \
  _min_uV,                                              /* _min_uV            */ \
  _max_uV,                                              /* _max_uV            */ \
  REGULATOR_MODE_NORMAL | REGULATOR_MODE_IDLE,          /* _modes             */ \
  REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS |  /* _ops               */ \
  REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_DRMS,        /*                    */ \
  0,                                                    /* _apply_uV          */ \
  _pull_down,                                           /* _pull_down         */ \
  _always_on,                                           /* _always_on         */ \
  _supply_regulator,                                    /* _supply_regulator  */ \
  _system_uA,                                           /* _system_uA         */ \
  _enable_time,                                         /* _enable_time       */ \
  _reg_id                                               /* _reg_id            */ \
)

#define PM8XXX_NLDO1200(_id, _name, _always_on, _pull_down, _min_uV, _max_uV, _enable_time, _supply_regulator, _system_uA, _reg_id) \
PM8XXX_LDO(_id, _name, _always_on, _pull_down, _min_uV, _max_uV, _enable_time, _supply_regulator, _system_uA, _reg_id) /* Same as PM8921_VREG_INIT_LDO */

#define PM8XXX_VS300(_id, _name, _always_on, _pull_down, _enable_time, _supply_regulator, _reg_id) \
PM8XXX_VREG_INIT( \
  _id,                      /* _id                */ \
  _name,                    /* _name              */ \
  0,                        /* _min_uV            */ \
  0,                        /* _max_uV            */ \
  0,                        /* _modes             */ \
  REGULATOR_CHANGE_STATUS,  /* _ops               */ \
  0,                        /* _apply_uV          */ \
  _pull_down,               /* _pull_down         */ \
  _always_on,               /* _always_on         */ \
  _supply_regulator,        /* _supply_regulator  */ \
  0,                        /* _system_uA         */ \
  _enable_time,             /* _enable_time       */ \
  _reg_id                   /* _reg_id            */ \
)

#define GPIO_VREG(_id, _reg_name, _gpio_label, _gpio, _supply_regulator) \
  [GPIO_VREG_ID_##_id] = {                                        \
    .init_data = {                                                \
      .constraints = {                                            \
        .valid_ops_mask       = REGULATOR_CHANGE_STATUS,          \
      },                                                          \
      .num_consumer_supplies  = ARRAY_SIZE(vreg_consumers_##_id), \
      .consumer_supplies      = vreg_consumers_##_id,             \
      .supply_regulator       = _supply_regulator,                \
    },                                                            \
    .regulator_name           = _reg_name,                        \
    .gpio_label               = _gpio_label,                      \
    .gpio                     = _gpio,                            \
  }

#define SAW_VREG_INIT(_id, _name, _min_uV, _max_uV) \
  {                                                             \
    .constraints = {                                            \
      .name                 = _name,                            \
      .valid_ops_mask       = REGULATOR_CHANGE_VOLTAGE,         \
      .min_uV               = _min_uV,                          \
      .max_uV               = _max_uV,                          \
    },                                                          \
    .num_consumer_supplies  = ARRAY_SIZE(vreg_consumers_##_id), \
    .consumer_supplies      = vreg_consumers_##_id,             \
  }

#define RPM_INIT( _id, _min_uV, _max_uV, _modes, _ops, _apply_uV, _default_uV,  \
                  _peak_uA, _avg_uA, _pull_down, _pin_ctrl, _freq, _pin_fn,     \
                  _force_mode, _power_mode, _state, _sleep_selectable,          \
                  _always_on, _supply_regulator, _system_uA)                    \
{                                                               \
  .init_data = {                                                \
    .constraints = {                                            \
      .valid_modes_mask     = _modes,                           \
      .valid_ops_mask       = _ops,                             \
      .min_uV               = _min_uV,                          \
      .max_uV               = _max_uV,                          \
      .input_uV             = _min_uV,                          \
      .apply_uV             = _apply_uV,                        \
      .always_on            = _always_on,                       \
    },                                                          \
    .num_consumer_supplies  = ARRAY_SIZE(vreg_consumers_##_id), \
    .consumer_supplies      = vreg_consumers_##_id,             \
    .supply_regulator       = _supply_regulator,                \
  },                                                            \
  .id                       = RPM_VREG_ID_PM8921_##_id,         \
  .default_uV               = _default_uV,                      \
  .peak_uA                  = _peak_uA,                         \
  .avg_uA                   = _avg_uA,                          \
  .pull_down_enable         = _pull_down,                       \
  .pin_ctrl                 = _pin_ctrl,                        \
  .freq                     = RPM_VREG_FREQ_##_freq,            \
  .pin_fn                   = _pin_fn,                          \
  .force_mode               = _force_mode,                      \
  .power_mode               = _power_mode,                      \
  .state                    = _state,                           \
  .sleep_selectable         = _sleep_selectable,                \
  .system_uA                = _system_uA,                       \
}

#define RPM_LDO(_id, _always_on, _pd, _sleep_selectable, _min_uV, _max_uV, _supply_regulator, _system_uA, _init_peak_uA) \
RPM_INIT(                                                                       \
  _id,                                                 /* _id                */ \
  _min_uV,                                             /* _min_uV            */ \
  _max_uV,                                             /* _max_uV            */ \
  REGULATOR_MODE_NORMAL | REGULATOR_MODE_IDLE,         /* _modes             */ \
  REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS | /* _ops               */ \
  REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_DRMS,       /*                    */ \
  0,                                                   /* _apply_uV          */ \
  _max_uV,                                             /* _default_uV        */ \
  _init_peak_uA,                                       /* _peak_uA           */ \
  0,                                                   /* _avg_uA            */ \
  _pd,                                                 /* _pull_down         */ \
  RPM_VREG_PIN_CTRL_NONE,                              /* _pin_ctrl          */ \
  NONE,                                                /* _freq              */ \
  RPM_VREG_PIN_FN_8960_NONE,                           /* _pin_fn            */ \
  RPM_VREG_FORCE_MODE_8960_NONE,                       /* _force_mode        */ \
  RPM_VREG_POWER_MODE_8960_PWM,                        /* _power_mode        */ \
  RPM_VREG_STATE_OFF,                                  /* _state             */ \
  _sleep_selectable,                                   /* _sleep_selectable  */ \
  _always_on,                                          /* _always_on         */ \
  _supply_regulator,                                   /* _supply_regulator  */ \
  _system_uA                                           /* _system_uA         */ \
)

#define RPM_LDO_FN(_id, _always_on, _pd, _sleep_selectable, _min_uV, _max_uV, _supply_regulator, _system_uA, _init_peak_uA, _pin_fn) \
RPM_INIT(                                                                       \
  _id,                                                 /* _id                */ \
  _min_uV,                                             /* _min_uV            */ \
  _max_uV,                                             /* _max_uV            */ \
  REGULATOR_MODE_NORMAL | REGULATOR_MODE_IDLE,         /* _modes             */ \
  REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS | /* _ops               */ \
  REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_DRMS,       /*                    */ \
  0,                                                   /* _apply_uV          */ \
  _max_uV,                                             /* _default_uV        */ \
  _init_peak_uA,                                       /* _peak_uA           */ \
  0,                                                   /* _avg_uA            */ \
  _pd,                                                 /* _pull_down         */ \
  RPM_VREG_PIN_CTRL_NONE,                              /* _pin_ctrl          */ \
  NONE,                                                /* _freq              */ \
  _pin_fn,                                             /* _pin_fn            */ \
  RPM_VREG_FORCE_MODE_8960_NONE,                       /* _force_mode        */ \
  RPM_VREG_POWER_MODE_8960_PWM,                        /* _power_mode        */ \
  RPM_VREG_STATE_OFF,                                  /* _state             */ \
  _sleep_selectable,                                   /* _sleep_selectable  */ \
  _always_on,                                          /* _always_on         */ \
  _supply_regulator,                                   /* _supply_regulator  */ \
  _system_uA                                           /* _system_uA         */ \
)

#define RPM_SMPS(_id, _always_on, _pd, _sleep_selectable, _min_uV, _max_uV, _supply_regulator, _system_uA, _freq) \
RPM_INIT(                                                                       \
  _id,                                                 /* _id                */ \
  _min_uV,                                             /* _min_uV            */ \
  _max_uV,                                             /* _max_uV            */ \
  REGULATOR_MODE_NORMAL | REGULATOR_MODE_IDLE,         /* _modes             */ \
  REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS | /* _ops               */ \
  REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_DRMS,       /*                    */ \
  0,                                                   /* _apply_uV          */ \
  _max_uV,                                             /* _default_uV        */ \
  _system_uA,                                          /* _peak_uA           */ \
  0,                                                   /* _avg_uA            */ \
  _pd,                                                 /* _pull_down         */ \
  RPM_VREG_PIN_CTRL_NONE,                              /* _pin_ctrl          */ \
  _freq,                                               /* _freq              */ \
  RPM_VREG_PIN_FN_8960_MODE,                           /* _pin_fn            */ \
  RPM_VREG_FORCE_MODE_8960_NONE,                       /* _force_mode        */ \
  RPM_VREG_POWER_MODE_8960_PWM,                        /* _power_mode        */ \
  RPM_VREG_STATE_OFF,                                  /* _state             */ \
  _sleep_selectable,                                   /* _sleep_selectable  */ \
  _always_on,                                          /* _always_on         */ \
  _supply_regulator,                                   /* _supply_regulator  */ \
  _system_uA                                           /* _system_uA         */ \
)

#define RPM_VS(_id, _always_on, _pd, _sleep_selectable, _supply_regulator) \
RPM_INIT(                                                                       \
  _id,                                                 /* _id                */ \
  0,                                                   /* _min_uV            */ \
  0,                                                   /* _max_uV            */ \
  0,                                                   /* _modes             */ \
  REGULATOR_CHANGE_STATUS,                             /* _ops               */ \
  0,                                                   /*                    */ \
  0,                                                   /* _apply_uV          */ \
  1000,                                                /* _default_uV        */ \
  1000,                                                /* _peak_uA           */ \
  _pd,                                                 /* _avg_uA            */ \
  RPM_VREG_PIN_CTRL_NONE,                              /* _pull_down         */ \
  NONE,                                                /* _pin_ctrl          */ \
  RPM_VREG_PIN_FN_8960_NONE,                           /* _freq              */ \
  RPM_VREG_FORCE_MODE_8960_NONE,                       /* _pin_fn            */ \
  RPM_VREG_POWER_MODE_8960_PWM,                        /* _force_mode        */ \
  RPM_VREG_STATE_OFF,                                  /* _power_mode        */ \
  _sleep_selectable,                                   /* _state             */ \
  _always_on,                                          /* _sleep_selectable  */ \
  _supply_regulator,                                   /* _always_on         */ \
  0                                                    /* _supply_regulator  */ \
)                                                      /* _system_uA         */

#define RPM_NCP(_id, _always_on, _sleep_selectable, _min_uV, _max_uV, _supply_regulator, _freq) \
RPM_INIT(                                                                       \
  _id,                                                 /* _id                */ \
  _min_uV,                                             /* _min_uV            */ \
  _max_uV,                                             /* _max_uV            */ \
  0,                                                   /* _modes             */ \
  REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,  /* _ops               */ \
  0,                                                   /*                    */ \
  _max_uV,                                             /* _apply_uV          */ \
  1000,                                                /* _default_uV        */ \
  1000,                                                /* _peak_uA           */ \
  0,                                                   /* _avg_uA            */ \
  RPM_VREG_PIN_CTRL_NONE,                              /* _pull_down         */ \
  _freq,                                               /* _pin_ctrl          */ \
  RPM_VREG_PIN_FN_8960_NONE,                           /* _freq              */ \
  RPM_VREG_FORCE_MODE_8960_NONE,                       /* _pin_fn            */ \
  RPM_VREG_POWER_MODE_8960_PWM,                        /* _force_mode        */ \
  RPM_VREG_STATE_OFF,                                  /* _power_mode        */ \
  _sleep_selectable,                                   /* _state             */ \
  _always_on,                                          /* _sleep_selectable  */ \
  _supply_regulator,                                   /* _always_on         */ \
  0                                                    /* _supply_regulator  */ \
)                                                      /* _system_uA         */


#define RPM_PC_INIT(_id, _always_on, _pin_fn, _pin_ctrl, _supply_regulator) \
{                                                                    \
  .init_data = {                                                     \
    .constraints = {                                                 \
      .valid_ops_mask       = REGULATOR_CHANGE_STATUS,               \
      .always_on            = _always_on,                            \
    },                                                               \
    .num_consumer_supplies  = ARRAY_SIZE(vreg_consumers_##_id##_PC), \
    .consumer_supplies      = vreg_consumers_##_id##_PC,             \
    .supply_regulator       = _supply_regulator,                     \
  },                                                                 \
  .id                       = RPM_VREG_ID_PM8921_##_id##_PC,         \
  .pin_fn                   = RPM_VREG_PIN_FN_8960_##_pin_fn,        \
  .pin_ctrl                 = RPM_VREG_PIN_CTRL_##_pin_ctrl,         \
}


