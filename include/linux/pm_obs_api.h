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
 */

#ifndef __PM_OBS_API_H__
#define __PM_OBS_API_H__


typedef enum {
    PM_OBS_OFF = 0,

    PM_OBS_CHARGER_MODE,  
    PM_OBS_RF_DUMMY_MODE, 

    PM_OBS_1X_MODE = 10,   
    PM_OBS_EVDO_MODE,      
    PM_OBS_GSM_VOICE_MODE, 
    PM_OBS_GSM_DATA_MODE,  
    PM_OBS_UMTS_VOICE_MODE,
    PM_OBS_UMTS_DATA_MODE, 
    PM_OBS_LTE_DATA_MODE,  
    _PM_OBS_RESERVED_RF_1_,
    _PM_OBS_RESERVED_RF_2_,
    _PM_OBS_RESERVED_RF_3_,
    _PM_OBS_RESERVED_RF_4_,
    _PM_OBS_RESERVED_RF_5_,

    PM_OBS_CONNECT_OFF,      
    PM_OBS_CONNECT_ON,       
    _PM_OBS_RESERVED_RFPA_1_,
    _PM_OBS_RESERVED_RFPA_2_,
    _PM_OBS_RESERVED_RFPA_3_,
    _PM_OBS_RESERVED_RFPA_4_,
    _PM_OBS_RESERVED_RFPA_5_,

    PM_OBS_CAMERA_CAM1_MODE,
    PM_OBS_CAMERA_CAM2_MODE,
    _PM_OBS_RESERVED_CAM_1_,
    _PM_OBS_RESERVED_CAM_2_,
    _PM_OBS_RESERVED_CAM_3_,
    _PM_OBS_RESERVED_CAM_4_,
    _PM_OBS_RESERVED_CAM_5_,

    PM_OBS_CAMERALIGHT_MODE, 
    PM_OBS_MOBILELIGHT_MODE, 
    _PM_OBS_RESERVED_CAMF_1_,
    _PM_OBS_RESERVED_CAMF_2_,
    _PM_OBS_RESERVED_CAMF_3_,
    _PM_OBS_RESERVED_CAMF_4_,
    _PM_OBS_RESERVED_CAMF_5_,

    PM_OBS_LCDBACKLIGHT_MODE,
    PM_OBS_KEYBACKLIGHT_MODE,
    PM_OBS_VIBRATION_MODE,   
    PM_OBS_SPEAKER_MODE,     
    PM_OBS_BT_MODE,          
    PM_OBS_SENSOR_MODE,      
    PM_OBS_WLAN_MODE,        
    PM_OBS_DTV_MODE,         
    PM_OBS_FELICA_MODE,      
    PM_OBS_MULTIMEDIA_MODE,  
    _PM_OBS_RESERVED_MODE_1_,
    _PM_OBS_RESERVED_MODE_2_,
    _PM_OBS_RESERVED_MODE_3_,
    _PM_OBS_RESERVED_MODE_4_,
    _PM_OBS_RESERVED_MODE_5_,


} pm_obs_mode_type;

#ifndef TRUE
  #define FALSE 0
  #define TRUE  1
  typedef unsigned char boolean;
#endif

typedef enum {
    PM_OBS_RF_SLEEP_MODE = 0,
    PM_OBS_RF_1X_MODE,       
    PM_OBS_RF_EVDO_MODE,     
    PM_OBS_RF_GSM_VOICE_MODE,
    PM_OBS_RF_GSM_DATA_MODE, 
    PM_OBS_RF_UMTS_VOICE_MODE,
    PM_OBS_RF_UMTS_DATA_MODE, 
    PM_OBS_RF_LTE_DATA_MODE,  

    PM_OBS_RF_MAX
} pm_obs_mode_rf_type;

typedef enum {
    PM_OBS_SENSOR_OFF,
    PM_OBS_SENSOR_ON, 

    PM_OBS_SENSOR_MAX
} pm_obs_mode_sensor_type;

typedef enum {
    PM_OBS_WLAN_OFF_MODE,  
    PM_OBS_WLAN_SLEEP_MODE,
    PM_OBS_WLAN_TR_ON_MODE,


    PM_OBS_WLAN_MAX

} pm_obs_mode_wlan_type;

typedef enum {
    PM_OBS_CHG_OFF_MODE, 
    PM_OBS_CHG_STOP_MODE,
    PM_OBS_CHG_WAIT_MODE,
    PM_OBS_CHG_ON_MODE,  

    PM_OBS_CHG_MAX

} pm_obs_mode_charger_type;

extern void pm_obs_a_camera(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_camlight (pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_lcdbacklight (unsigned int value);
extern void pm_obs_a_keybacklight (pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_vibration (pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_speaker (pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_bluetooth (pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_sensor (pm_obs_mode_sensor_type mode);
extern void pm_obs_a_wlan(pm_obs_mode_wlan_type mode);
extern void pm_obs_a_dtv(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_felica(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_charger(pm_obs_mode_charger_type mode);

#endif /* __PM_OBS_API_H__ */
