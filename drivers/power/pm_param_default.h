/* Copyright (C) 2012, NEC CASIO Mobile Communications. All rights reserved.  
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


/********************************************************************/
/* Local Data Type Definition                                       */
/********************************************************************/

/*
 * If the following parameters would be different between targets in the future,
 * you should add conditional directive and parameters of each target.
 *
 * For example:
 *
 * #if defined(CONFIG_XXX)
 *   Parameters for CONFIG_XXX target
 * #elif defined(CONFIG_YYY)
 *   Parameters for CONFIG_YYY target
 * #else
 *   Parameters for default target
 * #endif
 *
 */
static const pm_pw_ctrl_vdet_type pm_pw_ctrl_vdet_default = {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        {0x1036, 0x0046, 0x0FA4, 0x0F05, 0x0E95, 0x0E62, 0x0E2D, 0x0DF4,
         0x0CE4}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x1036, 0x0060, 0x0FB4, 0x0F1A, 0x0E8F, 0x0E53, 0x0E0F, 0x0D89,
	 0x0CE4}
#else
	{0x1036, 0x0060, 0x0F6A, 0x0ED1, 0x0E6D, 0x0E34, 0x0DF8, 0x0DAE,
	 0x0CE4}
#endif
};

static const pm_pw_ctrl_dvdet_type pm_pw_ctrl_dvdet_default = {
	{0x0077, 0x006D, 0x0077, 0x0066, 0x0079, 0x0084, 0x0080, 0x003D,
	 0x0038, 0x003A, 0x002F, 0x003F, 0x0058, 0x005D, 0x0000, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}
};

static const pm_pw_ctrl_ct_type pm_pw_ctrl_ct_default = {
	{0x0200}
};

static const pm_pw_ctrl_batadj_type pm_pw_ctrl_batadj_default = {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        {0x0046, 0x0015, 0x002A, 0x0032, 0x000C, 0x000C, 0x0012, 0x0012,
         0x0027, 0x000D, 0x0010, 0x0032, 0x003E, 0x0061, 0x0001, 0x0005,
         0x0008, 0x0010, 0x001C, 0x0001, 0x0008, 0x000E, 0x001B, 0x002C,
         0x0000, 0x0008, 0x0019, 0x0023, 0x0031, 0x0036, 0x003D, 0x0008,
         0x0004, 0x0004, 0x0006, 0x0007, 0x000A, 0x000C, 0x0004, 0x000C,
         0x0017, 0x0006, 0x0000, 0x0000, 0x0007, 0x000B, 0x0000, 0x0001,
         0x0014, 0x0000}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x0059, 0x000A, 0x002A, 0x0032, 0x0000, 0x0003, 0x0000, 0x0003,
	 0x0003, 0x000D, 0x0010, 0x0032, 0x003E, 0x0061, 0x0014, 0x0015,
	 0x0016, 0x0017, 0x001A, 0x002A, 0x002F, 0x0052, 0x005F, 0x008F,
	 0x0014, 0x0015, 0x0016, 0x0017, 0x001A, 0x0027, 0x0027, 0x000D,
	 0x0010, 0x0005, 0x000B, 0x0010, 0x0016, 0x001B, 0x0003, 0x0011,
	 0x0003, 0x0007, 0x0000, 0x0000, 0x001D, 0x000B, 0x0000, 0x0000,
	 0x001D, 0x001E}
#else
	{0x008A, 0x000A, 0x002A, 0x0032, 0x0000, 0x0003, 0x0000, 0x0003,
	 0x0003, 0x000D, 0x0010, 0x0032, 0x003E, 0x0061, 0x0014, 0x0015,
	 0x0016, 0x0017, 0x001A, 0x002A, 0x002F, 0x0052, 0x005F, 0x008F,
	 0x0014, 0x0015, 0x0016, 0x0017, 0x001A, 0x0027, 0x0027, 0x000D,
	 0x0010, 0x0005, 0x000B, 0x0010, 0x0016, 0x001B, 0x0003, 0x0011,
	 0x0003, 0x0007, 0x0000, 0x0000, 0x001D, 0x000B, 0x0000, 0x004D,
	 0x0005, 0x001E}
#endif
};

static const pm_pw_ctrl_batadjth_type pm_pw_ctrl_batadjth_default = {
	{0x0002, 0x000A, 0x000D, 0x0012, 0x000F, 0x0015, 0x0019, 0x001D,
	 0x0002, 0x0009, 0x000D, 0x0012, 0x0002, 0x0009, 0x000D, 0x0012}
};

static const pm_pw_ctrl_rtcadj_type pm_pw_ctrl_rtcadj_default = {
	{0xFF}
};

static const pm_pw_adc_determin_type pm_pw_adc_determin_default = {
	{0x0005, 0x001E, 0x012C}
};

static const pm_pw_ctrl_temp_batadj_type pm_pw_ctrl_temp_batadj_default = {
#if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_RUBY)
	{0x0573, 0x04E9}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x057D, 0x04EC}
#else
	{0x0577, 0x04EA}
#endif
};

static const pm_pw_ctrl_temp_err_type pm_pw_ctrl_temp_err_default = {
#if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_RUBY)
	{0x04E9, 0x049F, 0x0272, 0x0241}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x04EC, 0x049E, 0x0274, 0x0244}
#else
	{0x04EA, 0x049E, 0x0275, 0x0245}
#endif
};

static const pm_pw_test_temp_set_status_type pm_pw_test_temp_set_status_default = {
#if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_RUBY)
	{0x0092, 0x0166}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x0092, 0x0170}
#else
	{0x0092, 0x0172}
#endif
};


static const pm_charge_off_on_xo_temp_cam_type pm_charge_off_on_xo_temp_cam_default = {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        {0x01C0, 0x020A, 0x020A}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x015C, 0x020A, 0x0168}
#elif defined(CONFIG_FEATURE_NCMC_D121M)
	{0x0105, 0x0129, 0x0116}
#else
	{0x01C0, 0x020A, 0x020A}
#endif
};

static const pm_charge_off_on_xo_temp_lte_type pm_charge_off_on_xo_temp_lte_default = {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        {0x0132, 0x015C, 0x015C}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x015C, 0x015C, 0x0168}
#elif defined(CONFIG_FEATURE_NCMC_D121M)
	{0x0105, 0x0129, 0x0116}
#else
	{0x0132, 0x015C, 0x015C}
#endif
};

static const pm_charge_off_on_xo_temp_type pm_charge_off_on_xo_temp_default = {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        {0x0147, 0x0173, 0x0173}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x015C, 0x015C, 0x0168}
#elif defined(CONFIG_FEATURE_NCMC_D121M)
	{0x0105, 0x0129, 0x0116}
#else
	{0x0147, 0x0173, 0x0173}
#endif
};

static const pm_charge_off_on_xo_temp_mm_type pm_charge_off_on_xo_temp_mm_default = {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        {0x015C, 0x020A, 0x0168}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x0151, 0x020A, 0x0151}
#elif defined(CONFIG_FEATURE_NCMC_D121M)
	{0x0105, 0x0129, 0x0116}
#else
	{0x015C, 0x020A, 0x0168}
#endif
};

static const pm_chg_setting_type pm_chg_setting_default = {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        {0x06A4, 0x0145, 0x107C, 0x102C, 0x000A, 0x0032, 0x00B4, 0x000A}
#elif defined(CONFIG_FEATURE_NCMC_D121F)
	{0x0578, 0x0145, 0x10F4, 0x10A4, 0x000A, 0x0032, 0x00B4, 0x000A}
#elif defined(CONFIG_FEATURE_NCMC_D121M)
	{0x0578, 0x0145, 0x107C, 0x102C, 0x000A, 0x0032, 0x00B4, 0x000A}
#else
	{0x06A4, 0x0145, 0x107C, 0x102C, 0x000A, 0x0032, 0x00B4, 0x000A}
#endif
};


static const pm_bat_alm_cut_off_type pm_bat_alm_cut_off_default = {
	{0x0000, 0x0CE4, 0x0008, 0x0009, 0x0007}
};



static const pm_lva_off_type pm_lva_off_default = {
	{0x0000}
};
