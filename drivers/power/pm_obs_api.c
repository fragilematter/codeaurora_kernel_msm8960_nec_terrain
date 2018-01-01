/* Copyright (C) 2011, NEC CASIO Mobile Communications. All rights reserved.  
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


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pm_obs_api.h>
#include <linux/battim.h>

void pm_obs_a_camera( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw);

    /* check main parameter */
    switch (mode) {
    case PM_OBS_CAMERA_CAM1_MODE:
    case PM_OBS_CAMERA_CAM2_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_CAMERA, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_camera);

void pm_obs_a_camlight( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw);

    /* check main parameter */
    switch (mode) {
    case PM_OBS_CAMERALIGHT_MODE:
    case PM_OBS_MOBILELIGHT_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_CAMLIGHT, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_camlight);

void pm_obs_a_lcdbacklight( unsigned int value )
{
    pr_debug("[PM][%s] value:0x%08x\n", __func__, value);

    /* check sub parameter */
    if (value > 100) {
        pr_warn("[PM][%s] warn: Too large number(value=%d)\n", __func__, value);
        value = 100;
    }

    battim_update_obs_param( BATTIM_OBS_IDX_LCDBACKLIGHT, (uint8_t)PM_OBS_LCDBACKLIGHT_MODE, (int32_t)value );
    return;
}
EXPORT_SYMBOL(pm_obs_a_lcdbacklight);

void pm_obs_a_keybacklight( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw);

    /* check main parameter */
    switch (mode) {
    case PM_OBS_KEYBACKLIGHT_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_KEYBACKLIGHT, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_keybacklight);

void pm_obs_a_vibration( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw);

    /* check main parameter */
    switch (mode) {
    case PM_OBS_VIBRATION_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_VIBRATION, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_vibration);

void pm_obs_a_speaker( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw); 

    /* check main parameter */
    switch (mode) {
    case PM_OBS_SPEAKER_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_SPEAKER, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_speaker);

void pm_obs_a_bluetooth( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw);

    /* check main parameter */
    switch (mode) {
    case PM_OBS_BT_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_BLUETOOTH, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_bluetooth);

void pm_obs_a_sensor( pm_obs_mode_sensor_type mode )
{
    pr_debug("[PM][%s] mode:0x%08x\n", __func__, mode);

    /* check sub parameter */
    if (PM_OBS_SENSOR_OFF <= mode && mode < PM_OBS_SENSOR_MAX) {
        battim_update_obs_param( BATTIM_OBS_IDX_SENSOR, (uint8_t)PM_OBS_SENSOR_MODE, (int32_t)mode );
    }
    else {
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_sensor);

void pm_obs_a_wlan( pm_obs_mode_wlan_type mode )
{
    pr_debug("[PM][%s] mode:0x%08x\n", __func__, mode);

    /* check sub parameter */
    if (PM_OBS_WLAN_OFF_MODE <= mode && mode < PM_OBS_WLAN_MAX) {
        battim_update_obs_param( BATTIM_OBS_IDX_WLAN, (uint8_t)PM_OBS_WLAN_MODE, (int32_t)mode );
    }
    else {
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_wlan);

void pm_obs_a_dtv( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw); 

    /* check main parameter */
    switch (mode) {
    case PM_OBS_DTV_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_DTV, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_dtv);

void pm_obs_a_felica( pm_obs_mode_type mode, boolean sw )
{
    pr_debug("[PM][%s] mode:0x%08x, sw:0x%08x\n", __func__, mode, sw); 

    /* check main parameter */
    switch (mode) {
    case PM_OBS_FELICA_MODE:
        battim_update_obs_param( BATTIM_OBS_IDX_FELICA, (uint8_t)mode, (int32_t)sw );
        break;
    default:
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
        break;
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_felica);

void pm_obs_a_charger( pm_obs_mode_charger_type mode )
{
#if !defined (CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
    pr_debug("[PM][%s] mode:0x%08x\n", __func__, mode); 
#endif

    /* check sub parameter */
    if (PM_OBS_CHG_OFF_MODE <= mode && mode < PM_OBS_CHG_MAX) {
        battim_update_obs_param( BATTIM_OBS_IDX_CHARGER, (uint8_t)PM_OBS_CHARGER_MODE, (int32_t)mode );
    }
    else {
        pr_err("[PM][%s] Ignored for invalid argument.\n", __func__);
    }
    return;
}
EXPORT_SYMBOL(pm_obs_a_charger);
