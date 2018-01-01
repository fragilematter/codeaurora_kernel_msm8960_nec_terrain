#ifndef __NCDIAGD_POWER_H__
#define __NCDIAGD_POWER_H__

#include <linux/ioctl.h>
#include <linux/types.h>

#define DIAG_PW_IOCTL_MAGIC 0xE0 /* Temporary */
#define PW_MAXARG 11 /* Maximum number of Argument  */

#define IOCTL_PW_GR_TCXO_CTL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_TCXO_CTL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_TCXO_CMD             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_TCXO_CMD_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_TCXO_CMD2            _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_TCXO_CMD2_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_SLP_CLK           _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_SLP_CLK_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_ENABLE            _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_ENABLE_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_RFBUF             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_RFBUF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_BOOST             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_BOOST_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_RTC_TM_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_RTC_TM_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_RG_LP_CTL               _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_LP_CTL_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW_RG_CTL                  _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_CTL_MAGIC,                ioctl_pw_value_type )
#define IOCTL_PW_RG_SET_LVL              _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SET_LVL_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_CNF             _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_CNF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_CLK             _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_CLK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_XO_DIV          _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_XO_DIV_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_RG_BPS_SET              _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_BPS_SET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_RG_BPS_CLR              _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_BPS_CLR_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_SZ_SET          _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_SZ_SET_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_PSK             _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_PSK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_VT_PLDWN_SW             _IOW (DIAG_PW_IOCTL_MAGIC, PW_VT_PLDWN_SW_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_TRN_SW              _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TRN_SW_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_CHG_TRN_IMSEL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TRN_IMSEL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_PLS_CNF             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_PLS_CNF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_PLS_SW              _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_PLS_SW_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_CHG_PLS_V_CNF           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_PLS_V_CNF_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_BOOT_STUP           _IO  (DIAG_PW_IOCTL_MAGIC, PW_CHG_BOOT_STUP_MAGIC                              )
#define IOCTL_PW_CHG_BOOT_CRUP           _IO  (DIAG_PW_IOCTL_MAGIC, PW_CHG_BOOT_CRUP_MAGIC                              )
#define IOCTL_PW_CHG_COIN_CNF            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_COIN_CNF_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_COIN_SW             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_COIN_SW_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_IS_SPT              _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_IS_SPT_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_CHG_TRK_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TRK_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_USB_SSPD            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_USB_SSPD_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_IM_CAL_SET          _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_IM_CAL_SET_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_CHG_IM_GAIN_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_IM_GAIN_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_CHG_BAT_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BAT_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_BOOST_EBL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BOOST_EBL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_DSBL                _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_DSBL_MAGIC,              ioctl_pw_value_type )
#define IOCTL_PW_CHG_VCP_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VCP_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_ATC_DSBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_ATC_DSBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_AUTO_DSBL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_AUTO_DSBL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_OVRD_EBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_OVRD_EBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_DONE_EBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_DONE_EBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_MODE_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_MODE_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_BTFET_DSBL          _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BTFET_DSBL_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_CHG_BATT_DSBL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BATT_DSBL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_FAIL_CLR            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_FAIL_CLR_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_VMAX_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VMAX_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBAT_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBAT_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_IMAX_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_IMAX_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_VTRKL_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VTRKL_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_ITRKL_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_ITRKL_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_ITERM_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_ITERM_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_TTRKL_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TTRKL_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_TCHG_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TCHG_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_STATE_GET           _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_STATE_GET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_STATE_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_STATE_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_SET        _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_SET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_GET        _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_GET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_GETLIM     _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_GETLIM_MAGIC,   ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_MDSW       _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_MDSW_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_ADC_INIT                _IO  (DIAG_PW_IOCTL_MAGIC, PW_ADC_INIT_MAGIC                                   )
#define IOCTL_PW_ADC_CNF_MUX             _IOW (DIAG_PW_IOCTL_MAGIC, PW_ADC_CNF_MUX_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_ADC_CALIB_REQD          _IOR (DIAG_PW_IOCTL_MAGIC, PW_ADC_CALIB_REQD_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_ADC_AMUX_OFF            _IOR (DIAG_PW_IOCTL_MAGIC, PW_ADC_AMUX_OFF_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_ADC_REG_AVBL            _IOR (DIAG_PW_IOCTL_MAGIC, PW_ADC_REG_AVBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_ADC_GET_PRESC           _IOWR(DIAG_PW_IOCTL_MAGIC, PW_ADC_GET_PRESC_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_ADC_RD_CHANNEL          _IOWR(DIAG_PW_IOCTL_MAGIC, PW_ADC_RD_CHANNEL_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET              _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_GET              _IOWR(DIAG_PW_IOCTL_MAGIC, PW_LS_LED_GET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET_CUR          _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_CUR_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET_MODE         _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_MODE_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET_POL          _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_POL_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_LS_VIB_SET_VOLT         _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_VIB_SET_VOLT_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_LS_VIB_SET_MODE         _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_VIB_SET_MODE_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_KHZ_XTAL_OSC_CMD        _IOW (DIAG_PW_IOCTL_MAGIC, PW_KHZ_XTAL_OSC_CMD_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_SP_SMPLD_SW             _IOW (DIAG_PW_IOCTL_MAGIC, PW_SP_SMPLD_SW_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_SP_SMPLD_TM_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_SP_SMPLD_TM_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFDG_IPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFDG_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFDG_OPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFDG_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFDG_IOPUT         _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFDG_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFAN_IPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFAN_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFAN_OPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFAN_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNF_I_SINK          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNF_I_SINK_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNF_TEST            _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNF_TEST_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_MPP_SCR_CNFDG_OPUT      _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_OPUT_MAGIC,    ioctl_pw_value_type )
#define IOCTL_PW_MPP_SCR_CNFDG_SET       _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_SET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_GPIO_CONFIG_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_GPIO_CONFIG_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_EBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_EBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_CRTTRS_SET     _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_CRTTRS_SET_MAGIC,   ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_HYST_SET       _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_HYST_SET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_PERIOD_SET     _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_PERIOD_SET_MAGIC,   ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_EN              _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_EN_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_IS_EN           _IOR (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_IS_EN_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_VOL_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_VOL_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_VOL_GET         _IOR (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_VOL_GET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_PCT_OVP_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OVP_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_PCT_OVP_TRS_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OVP_TRS_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_PCT_OVP_HYST_SET        _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OVP_HYST_SET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_PCT_OTP_STAGE_GET       _IOR (DIAG_PW_IOCTL_MAGIC, PW_PCT_OTP_STAGE_GET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_PCT_OTP_STG_OVD         _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OTP_STG_OVD_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_IR_RT_STATUS_GET        _IOWR(DIAG_PW_IOCTL_MAGIC, PW_IR_RT_STATUS_GET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_REGISTER_READ           _IOWR(DIAG_PW_IOCTL_MAGIC, PW_REGISTER_READ_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_REGISTER_WRITE          _IOW (DIAG_PW_IOCTL_MAGIC, PW_REGISTER_WRITE_MAGIC,        ioctl_pw_value_type )

#define IOCTL_PW_HW_PWR_OFF              _IO  (DIAG_PW_IOCTL_MAGIC, PW_HW_PWR_OFF_MAGIC                                 )
#define IOCTL_PW_HW_RESET                _IO  (DIAG_PW_IOCTL_MAGIC, PW_HW_RESET_MAGIC                                   )



#define DIAG_PW8901_IOCTL_MAGIC 0xE1 /* Temporary */
#define IOCTL_PW8901_GR_TCXO_CTL             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_TCXO_CTL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_GR_TCXO_CMD             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_TCXO_CMD_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_GR_TCXO_CMD2            _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_TCXO_CMD2_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW8901_GR_XO_SLP_CLK           _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_XO_SLP_CLK_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW8901_GR_XO_ENABLE            _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_XO_ENABLE_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW8901_GR_XO_RFBUF             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_XO_RFBUF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_GR_XO_BOOST             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_XO_BOOST_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_GR_RTC_TM_SET           _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_GR_RTC_TM_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW8901_RG_LP_CTL               _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_LP_CTL_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW8901_RG_CTL                  _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_CTL_MAGIC,                ioctl_pw_value_type )
#define IOCTL_PW8901_RG_SET_LVL              _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_SET_LVL_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW8901_RG_SMPS_CNF             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_SMPS_CNF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_RG_SMPS_CLK             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_SMPS_CLK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_RG_SMPS_XO_DIV          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_SMPS_XO_DIV_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8901_RG_BPS_SET              _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_BPS_SET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW8901_RG_BPS_CLR              _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_BPS_CLR_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW8901_RG_SMPS_SZ_SET          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_SMPS_SZ_SET_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8901_RG_SMPS_PSK             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_RG_SMPS_PSK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_VT_PLDWN_SW             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_VT_PLDWN_SW_MAGIC,           ioctl_pw_value_type )

#define IOCTL_PW8901_MPP_CNFDG_IPUT          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_CNFDG_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_CNFDG_OPUT          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_CNFDG_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_CNFDG_IOPUT         _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_CNFDG_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_CNFAN_IPUT          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_CNFAN_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_CNFAN_OPUT          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_CNFAN_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_CNF_I_SINK          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_CNF_I_SINK_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_CNF_TEST            _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_CNF_TEST_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_SCR_CNFDG_OPUT      _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_OPUT_MAGIC,    ioctl_pw_value_type )
#define IOCTL_PW8901_MPP_SCR_CNFDG_SET       _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_SET_MAGIC,     ioctl_pw_value_type )

#define IOCTL_PW8901_PCT_OVP_EBL             _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_PCT_OVP_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8901_PCT_OVP_TRS_SET         _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_PCT_OVP_TRS_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW8901_PCT_OVP_HYST_SET        _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_PCT_OVP_HYST_SET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW8901_PCT_OTP_STAGE_GET       _IOR (DIAG_PW8901_IOCTL_MAGIC, PW_PCT_OTP_STAGE_GET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW8901_PCT_OTP_STG_OVD         _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_PCT_OTP_STG_OVD_MAGIC,       ioctl_pw_value_type )

#define IOCTL_PW8901_IR_RT_STATUS_GET        _IOWR(DIAG_PW8901_IOCTL_MAGIC, PW_IR_RT_STATUS_GET_MAGIC,      ioctl_pw_value_type )

#define IOCTL_PW8901_REGISTER_READ           _IOWR(DIAG_PW8901_IOCTL_MAGIC, PW_REGISTER_READ_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW8901_REGISTER_WRITE          _IOW (DIAG_PW8901_IOCTL_MAGIC, PW_REGISTER_WRITE_MAGIC,        ioctl_pw_value_type )



#define IOCTL_PW_TSENSOR_GET_DEG             _IOR (DIAG_PW_IOCTL_MAGIC, PW_TEMP_SENSOR_GET_DEG_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_TSENSOR_GET_AD              _IOR (DIAG_PW_IOCTL_MAGIC, PW_TEMP_SENSOR_GET_ADC_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_TSENSOR_SW                  _IOW (DIAG_PW_IOCTL_MAGIC, PW_TEMP_SENSOR_SW_MAGIC,          ioctl_pw_value_type )


#define IOCTL_PW_CHG_USB_DSBL                _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_USB_DSBL_MAGIC,            ioctl_pw_value_type )

#define IOCTL_PW_CHG_LVA_OFF                 _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_LVA_OFF_MAGIC,             ioctl_pw_value_type )

#define IOCTL_PW_GPIO_GET_STATE              _IOWR(DIAG_PW_IOCTL_MAGIC, PW_GPIO_GET_STATE_MAGIC,          ioctl_pw_value_type )

enum ioctl_pw_magic{

    /***  DIAG_PW_GENERAL  ***/
    PW_GR_TCXO_CTL_MAGIC,           /* Configure TCXO controller.                                                               */
    PW_GR_TCXO_CMD_MAGIC,           /* Set the TCXO controller mode.                                                            */
    PW_GR_TCXO_CMD2_MAGIC,          /* Set the second TCXO controller mode.                                                     */
    PW_GR_XO_SLP_CLK_MAGIC,         /* Select sleep clock.                                                                      */
    PW_GR_XO_ENABLE_MAGIC,          /* Enable/Disable the 19.2 MHz crystal oscillator.                                          */
    PW_GR_XO_RFBUF_MAGIC,           /* Enable/Disable the RF TCXO buffer.                                                       */
    PW_GR_XO_BOOST_MAGIC,           /* Enable/Disable the boost of the XO core AGC gain.                                        */
    PW_GR_RTC_TM_SET_MAGIC,         /* Set the time adjustment correction factor for an RTC crystal oscillator.                 */

    /***  DIAG_PW_REGULATOR  ***/
    PW_RG_LP_CTL_MAGIC,             /* Enable/Disable the LDOÅfs Low Power mode.                                                */
    PW_RG_CTL_MAGIC,                /* Enable/Disable voltage regulators.                                                       */
    PW_RG_SET_LVL_MAGIC,            /* Program the voltage levels for the selected voltage regulator.                           */
    PW_RG_SMPS_CNF_MAGIC,           /* Configure the SMPS voltage regulators.                                                   */
    PW_RG_SMPS_CLK_MAGIC,           /* Configure the SMPS VREG's clock source.                                                  */
    PW_RG_SMPS_XO_DIV_MAGIC,        /* Configure the SMPS VREG's TCXO clock source divider.                                     */
    PW_RG_BPS_SET_MAGIC,            /* Set the Bypass mode to On for a given LDO.                                               */
    PW_RG_BPS_CLR_MAGIC,            /* Clear the Bypass mode for a given LDO.                                                   */
    PW_RG_SMPS_SZ_SET_MAGIC,        /* Set the SMPS switch size.                                                                */
    PW_RG_SMPS_PSK_MAGIC,           /* Enable/Disable SMPS pulse skipping.                                                      */
    PW_VT_PLDWN_SW_MAGIC,           /* Configure the voltage regulator active pull-down.                                        */

    /***  DIAG_PW_CHARGING  ***/
    PW_CHG_TRN_SW_MAGIC,            /* Turn the USB charger transistor On/Off.                                                  */
    PW_CHG_TRN_IMSEL_MAGIC,         /* Configure the USB charger transistor maximum current (IMAXSEL).                          */
    PW_CHG_PLS_CNF_MAGIC,           /* Configure the PMIC pulse charger hardware.                                               */
    PW_CHG_PLS_SW_MAGIC,            /* Enable/Disable pulse charging.                                                           */
    PW_CHG_PLS_V_CNF_MAGIC,         /* Configure the VBATDET value for pulse charging.                                          */
    PW_CHG_BOOT_STUP_MAGIC,         /* Turn on specific VREGs for USB charging to function correctly in the bootloader.         */
    PW_CHG_BOOT_CRUP_MAGIC,         /* Turn off the specific VREGs turned on in pm_chg_boot_uchg_vreg_init().                   */
    PW_CHG_COIN_CNF_MAGIC,          /* Configure the coin cell charger voltage and the current limiting resistor value.         */
    PW_CHG_COIN_SW_MAGIC,           /* Enable/Disable the coin cell charger.                                                    */
    PW_CHG_IS_SPT_MAGIC,            /* Verify that the PMIC hardware supports regular charging.                                 */
    PW_CHG_TRK_EBL_MAGIC,           /* Enable/Disable the trickle charger.                                                      */
    PW_CHG_USB_SSPD_MAGIC,          /* Enable/Disable USB Suspend mode.                                                         */
    PW_CHG_IM_CAL_SET_MAGIC,        /* Connect/Disconnect the constant current source to/from the IMON resistor.                */
    PW_CHG_IM_GAIN_SET_MAGIC,       /* Set the gain resolution near the end of charge.                                          */
    PW_CHG_BAT_EBL_MAGIC,           /* Open/Close the battery FET to start/stop charging (BAT_FET_N pin).                       */
    PW_CHG_BOOST_EBL_MAGIC,         /* Enable/Disable the VBUS_FROM_BOOST line to control whether to disable the charger.       */
    PW_CHG_DSBL_MAGIC,              /* Enable/Disable charging from a wall charger adapter.                                     */
    PW_CHG_VCP_EBL_MAGIC,           /* Enable/Disable VCP (VDD collapse protection).                                            */
    PW_CHG_ATC_DSBL_MAGIC,          /* Enable/Disable automatic trickle charging before start up.                               */
    PW_CHG_AUTO_DSBL_MAGIC,         /* Enable/Disable automatic charging.                                                       */
    PW_CHG_OVRD_EBL_MAGIC,          /* Configure that the connected peripheral to a VCHG/VBUS                                   */
    PW_CHG_DONE_EBL_MAGIC,          /* Enable/Disable use of VCHG/VBUS as a power source after enumeration from an USB host.    */
    PW_CHG_MODE_SET_MAGIC,          /* Set the Charging Temperature mode to continuous regulation or hysteresis control.        */
    PW_CHG_BTFET_DSBL_MAGIC,        /* Enable/Disable the slow battery FET power on.                                            */
    PW_CHG_BATT_DSBL_MAGIC,         /* Enable/Disable the battery temperature sensing comparators and reference output.         */
    PW_CHG_FAIL_CLR_MAGIC,          /* Write to the PMIC to exit a failed charging state.                                       */
    PW_CHG_VMAX_SET_MAGIC,          /* Set the charger regulator output voltage in mV (VMAXSEL).                                */
    PW_CHG_VBAT_SET_MAGIC,          /* Set the comparator triggering voltage in mV.                                             */
    PW_CHG_IMAX_SET_MAGIC,          /* Set the fast charge current limit in mA.                                                 */
    PW_CHG_VTRKL_SET_MAGIC,         /* Set the voltage below which (USB) auto trickle charging must start.                      */
    PW_CHG_ITRKL_SET_MAGIC,         /* Set the trickle charge current in mA.                                                    */
    PW_CHG_ITERM_SET_MAGIC,         /* Set the termination current (in mA) used to terminate automatic fast charging.           */
    PW_CHG_TTRKL_SET_MAGIC,         /* Set the maximum time for automatic trickle charging in minutes.                          */
    PW_CHG_TCHG_SET_MAGIC,          /* Set the maximum time for automatic fast charging in minutes.                             */
    PW_CHG_STATE_GET_MAGIC,         /* Get the value of the current state of the automatic charger.                             */
    PW_CHG_STATE_SET_MAGIC,         /* Set the current state of the automatic charger.                                          */

    /***  DIAG_PW_VOLTAGE  ***/
    PW_CHG_VBT_TRIM_SET_MAGIC,      /* Set the trimming for VBATDET.                                                            */
    PW_CHG_VBT_TRIM_GET_MAGIC,      /* Get the value of the trimming for VBATDET.                                               */
    PW_CHG_VBT_TRIM_GETLIM_MAGIC,   /* Get the value of the trimming threshold for VBATDET.                                     */
    PW_CHG_VBT_TRIM_MDSW_MAGIC,     /* Enable/Disable the trimming test.                                                        */

    /***  DIAG_PW_ADC  ***/
    PW_ADC_INIT_MAGIC,              /* Initialize ADC                                                                           */
    PW_ADC_CNF_MUX_MAGIC,           /* Configure an analog MUX channel for analog-to-digital conversion.                        */
    PW_ADC_CALIB_REQD_MAGIC,        /* Get calibration is required for any of the analog input channels connected to the MUX.   */
    PW_ADC_AMUX_OFF_MAGIC,          /* Get whether the PMICÅfs AMUX can be turned off.                                          */
    PW_ADC_REG_AVBL_MAGIC,          /* Get whether the internal input reference channels 0.625 V and 1.25 V are available.      */
    PW_ADC_GET_PRESC_MAGIC,         /* Get the PMIC prescalar numerator and denominator.                                        */
    PW_ADC_RD_CHANNEL_MAGIC,        /* Get the result of the AD conversion on the specified channel.                            */

    /***  DIAG_PW_LIGHTSENSOR  ***/
    PW_LS_LED_SET_MAGIC,            /* Set the intensity of a selected LED driver signal.                                       */
    PW_LS_LED_GET_MAGIC,            /* Get the intensity of the indicated LED driver.                                           */
    PW_LS_LED_SET_CUR_MAGIC,        /* Configure the current setting for the Flash LED.                                         */
    PW_LS_LED_SET_MODE_MAGIC,       /* Flash LED set mode                                                                       */
    PW_LS_LED_SET_POL_MAGIC,        /* Flash LED polarity                                                                       */
    PW_LS_VIB_SET_VOLT_MAGIC,       /* Configure the voltage setting for the vibrator motor.                                    */
    PW_LS_VIB_SET_MODE_MAGIC,       /* Configure the Vibrator Motor mode.                                                       */

    /***  DIAG_PW_32KHZ  ***/
    PW_KHZ_XTAL_OSC_CMD_MAGIC,      /* Enable/Disable the external sleep crystal oscillator.                                    */

    /***  DIAG_PW_SMPL  ***/
    PW_SP_SMPLD_SW_MAGIC,           /* Set the SMPL detection on or off.                                                        */
    PW_SP_SMPLD_TM_SET_MAGIC,       /* Configure the SMPL feature timeout value.                                                */

    /***  DIAG_PW_MPP  ***/
    PW_MPP_CNFDG_IPUT_MAGIC,        /* Configure the selected MPP as digital input pin.                                         */
    PW_MPP_CNFDG_OPUT_MAGIC,        /* Configure the selected MPP as digital output pin.                                        */
    PW_MPP_CNFDG_MAGIC,             /* Configure the selected MPP as a digital bidirectional pin.                               */
    PW_MPP_CNFAN_IPUT_MAGIC,        /* Configure the selected MPP as an analog input.                                           */
    PW_MPP_CNFAN_OPUT_MAGIC,        /* Configure the selected MPP as an analog output.                                          */
    PW_MPP_CNF_I_SINK_MAGIC,        /* Configure the selected MPP as a current sink.                                            */
    PW_MPP_CNF_TEST_MAGIC,          /* Configure a selected MPP as an ATEST.                                                    */
    PW_MPP_SCR_CNFDG_OPUT_MAGIC,    /* Configure one of the PMIC MPPs as a digital output.                                      */
    PW_MPP_SCR_CNFDG_SET_MAGIC,     /* Configure PMIC MPPs for secure access through RPC.                                       */

    /***  DIAG_PW_GPIO  ***/
    PW_GPIO_CONFIG_MAGIC,           /* Set configuration of selected GPIO.                                                      */

    /***  DIAG_PW_SOUND  ***/
    PW_SND_HSED_EBL_MAGIC,          /* Enable/Disable the signal control for the Once Touch Headset controller (HSED) module.   */
    PW_SND_HSED_CRTTRS_SET_MAGIC,   /* Configure the headset switch type and the corresponding current threshold.               */
    PW_SND_HSED_HYST_SET_MAGIC,     /* Configure the hysteresis clock pre-divider and the hysteresis clock.                     */
    PW_SND_HSED_PERIOD_SET_MAGIC,   /* Configure the period clock pre-divider and the period clock.                             */
    PW_SND_MIC_EN_MAGIC,            /* Enable/Disable the PMIC microphone.                                                      */
    PW_SND_MIC_IS_EN_MAGIC,         /* Check if the PMIC microphone is enabled/disabled.                                        */
    PW_SND_MIC_VOL_SET_MAGIC,       /* Set the PMIC microphone voltage.                                                         */
    PW_SND_MIC_VOL_GET_MAGIC,       /* Get the PMIC microphone voltage.                                                         */

    /***  DIAG_PW_PROTECT  ***/
    PW_PCT_OVP_EBL_MAGIC,           /* Enable/Disable the USB Overvoltage Protection (OVP) functionality.                       */
    PW_PCT_OVP_TRS_SET_MAGIC,       /* Configure the voltage threshold that triggers the USB OVP functionality.                 */
    PW_PCT_OVP_HYST_SET_MAGIC,      /* Configure the hysteresis for the voltage to exceed the threshold.                        */
    PW_PCT_OTP_STAGE_GET_MAGIC,     /* Get the current PM8058 over-temperature protection stage.                                */
    PW_PCT_OTP_STG_OVD_MAGIC,       /* Override automatic shut down of the PM8058 over-temperature protection stage 2.          */

    /***  DIAG_PW_INTERRUPT  ***/
    PW_IR_RT_STATUS_GET_MAGIC,      /* Get the real-time status of a selected PMIC IRQ.                                         */

    /***  DIAG_PW_REGISTER_RW  ***/
    PW_REGISTER_READ_MAGIC,         /* PMIC Register Read                                                                       */
    PW_REGISTER_WRITE_MAGIC,        /* PMIC Register Write                                                                      */

    /***  DIAG_PW_HW_PWR_OFF  ***/
    PW_HW_PWR_OFF_MAGIC,            /* Hardware Power Off                                                                       */
    PW_HW_RESET_MAGIC,              /* Hardware Reset                                                                           */

    PW_TEMP_SENSOR_GET_DEG_MAGIC,   /* Get the result on the temp sensor.                                                       */
    PW_TEMP_SENSOR_GET_ADC_MAGIC,   /* Get the result of AD conversion on the temp sensor.                                      */
    PW_TEMP_SENSOR_SW_MAGIC,        /* Configure the temp sensor power on/off.                                                  */

    PW_CHG_USB_DSBL_MAGIC,          /* Enable/Disable charging from a USB charger adapter.                                      */

    PW_CHG_LVA_OFF_MAGIC,           /* LVA ON/OFF Setting.                                                                      */

    PW_GPIO_GET_STATE_MAGIC         /* GPIO State Read                                                                          */
};

typedef enum
{
    /**
     * The API completed successfully
     */
    PM_ERR_FLAG__SUCCESS,
    /**
     * The SBI operation Failed
     * extra lines are to support += error codes
     */
    PM_ERR_FLAG__SBI_OPT_ERR,
    PM_ERR_FLAG__SBI_OPT2_ERR,
    PM_ERR_FLAG__SBI_OPT3_ERR,
    PM_ERR_FLAG__SBI_OPT4_ERR,

    /**
     *  Input Parameter one is out of range
     */
    PM_ERR_FLAG__PAR1_OUT_OF_RANGE,
    /**
     *  Input Parameter two is out of range
     */
    PM_ERR_FLAG__PAR2_OUT_OF_RANGE,
    /**
     *  Input Parameter three is out of range
     */
    PM_ERR_FLAG__PAR3_OUT_OF_RANGE,
    /**
     *  Input Parameter four is out of range
     */
    PM_ERR_FLAG__PAR4_OUT_OF_RANGE,
    /**
     *  Input Parameter five is out of range
     */
    PM_ERR_FLAG__PAR5_OUT_OF_RANGE,
    PM_ERR_FLAG__VLEVEL_OUT_OF_RANGE,
    PM_ERR_FLAG__VREG_ID_OUT_OF_RANGE,
    /**
     *  This feature is not supported by this PMIC
     */
    PM_ERR_FLAG__FEATURE_NOT_SUPPORTED,
    PM_ERR_FLAG__INVALID_PMIC_MODEL,
    PM_ERR_FLAG__SECURITY_ERR,
    PM_ERR_FLAG__IRQ_LIST_ERR,
    PM_ERR_FLAG__DEV_MISMATCH,
    PM_ERR_FLAG__ADC_INVALID_RES,
    PM_ERR_FLAG__ADC_NOT_READY,
    PM_ERR_FLAG__ADC_SIG_NOT_SUPPORTED,
    /**
     *  The RTC displayed mode read from the PMIC is invalid
     */
    PM_ERR_FLAG__RTC_BAD_DIS_MODE,
    /**
     *  Failed to read the time from the PMIC RTC
     */
    PM_ERR_FLAG__RTC_READ_FAILED,
    /**
     *  Failed to write the time to the PMIC RTC
     */
    PM_ERR_FLAG__RTC_WRITE_FAILED,
    /**
     *  RTC not running
     */
    PM_ERR_FLAG__RTC_HALTED,
    /**
     *  The DBUS is already in use by another MPP.
     */
    PM_ERR_FLAG__DBUS_IS_BUSY_MODE,
    /**
     *  The ABUS is already in use by another MPP.
     */
    PM_ERR_FLAG__ABUS_IS_BUSY_MODE,
    /**
     *  The error occurs illegal value that isn't in the
     *  macro_type enum is used
     */
    PM_ERR_FLAG__MACRO_NOT_RECOGNIZED,
    /**
     *  This error occurs if the data read from a register does
     *  not match the setting data
     */
    PM_ERR_FLAG__DATA_VERIFY_FAILURE,
    /**
     *  The error occurs illegal value that isn't in the
     *  pm_register_type enum is used
     */
    PM_ERR_FLAG__SETTING_TYPE_NOT_RECOGNIZED,
    /**
     * The error occurs illegal value that isn't in the
     * pm_mode_group enum is used
     */
    PM_ERR_FLAG__MODE_NOT_DEFINED_FOR_MODE_GROUP,
    /**
     *  The error occurs illegal value that isn't in the pm_mode
     *  enum is used
     */
    PM_ERR_FLAG__MODE_GROUP_NOT_DEFINED,
    /**
     *  This error occurs if the PRESTUB function returns a false
     */
    PM_ERR_FLAG__PRESTUB_FAILURE,
    /**
     *  This error occurs if the POSTSTUB function returns a
     *  false
     */
    PM_ERR_FLAG__POSTSTUB_FAILURE,
    /**
     *  When modes are set for a modegroup, they are recorded and
     *  checked for success
     */
    PM_ERR_FLAG__MODE_NOT_RECORDED_CORRECTLY,
    /**
     *  Unable to find the mode group in the mode group recording
     *  structure. Fatal memory problem
     */
    PM_ERR_FLAG__MODE_GROUP_STATE_NOT_FOUND,
    /**
     *  This error occurs if the SUPERSTUB function return a
     *  false
     */
    PM_ERR_FLAG__SUPERSTUB_FAILURE,

    /*! Non-Zero means unsuccessful call. Here it means registration failed
         because driver has ran out of memory.MAX_CLIENTS_ALLOWED needs to be
         increased and the code needs to be recompiled */
    PM_ERR_FLAG__VBATT_CLIENT_TABLE_FULL,
    /*! One of the parameters to the function call was invalid. */
    PM_ERR_FLAG__VBATT_REG_PARAMS_WRONG,
    /*! Client could not be deregistered because perhaps it does not exist */
    PM_ERR_FLAG__VBATT_DEREGISTRATION_FAILED,
    /*! Client could not be modified because perhaps it does not exist */
    PM_ERR_FLAG__VBATT_MODIFICATION_FAILED,
    /*!< Client could not be queried because perhaps it does not exist */
    PM_ERR_FLAG__VBATT_INTERROGATION_FAILED,
    /*!< Client's filetr could not be set because perhaps it does not exist */
    PM_ERR_FLAG__VBATT_SET_FILTER_FAILED,
    /*! Keeps the count of all errors. Any error code equal to or greater
         than this one means an unknown error. VBATT_LAST_ERROR should be
         the last error code always with the highest value */
    PM_ERR_FLAG__VBATT_LAST_ERROR,
    PM_ERR_FLAG__PMIC_NOT_SUPPORTED,
    /**
     *  This error occurs if Keypad event counter is invalid
     */
    PM_ERR_FLAG__INVALID_KEYPAD_EVENT_COUNTER,
    /**
     *  This error occurs if any DAL service fails
     */
    PM_ERR_FLAG__DAL_SERVICE_FAILED,
    PM_ERR_FLAG__INVALID

} pm_err_flag_type;

#define ADC_ERR_FLAG__SUCCESS 0

/* temporary */
typedef struct {
    unsigned char   req_buf[PW_MAXARG]; /* Request Buffer */
    unsigned char   rsp_buf[PW_MAXARG]; /* Respond Buffer */
} ioctl_pw_value_type;

#endif /* __NCDIAGD_POWER_H__ */
