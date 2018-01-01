/***********************************************************************
* (C) NCMC Corporation 2011
* 
* No permission to use, copy, modify and distribute this software
* and its documentation for any purpose is granted.
* This software is provided under applicable license agreement only.
*
* File Name:gysdcom.h
* Contents :Gyro Sensor Driver common header
*
*****************************************************************************/


#ifndef _gysdcom_h_
#define _gysdcom_h_


    #define     GYSD_DEVNAME                            "/dev/gyro"

    #define     GYSD_IO_FUNC_ON                         1
    #define     GYSD_IO_FUNC_OFF                        2
    #define     GYSD_IO_MEASURE_ONESHOT_START           3
    #define     GYSD_IO_MEASURE_ONESHOT_STOP            4
    #define     GYSD_IO_MEASURE_PERIODICITY_START       5
    #define     GYSD_IO_MEASURE_PERIODICITY_STOP        6
    #define     GYSD_IO_HPF_SET                         7
    #define     GYSD_IO_DEV_PARAM_INFO_GET              8
    #define     GYSD_IO_SELF_TEST                       9
    #define     GYSD_IO_CONNECTION_CHECK               10
    #define     GYSD_IO_REG_CHECK                      11
    #define     GYSD_IO_REG_READ                       12
    #define     GYSD_IO_REG_WRITE                      13
    #define     GYRO_IO_READ_RAD_XYZ                   14
    #define     GYRO_IO_SELF_TEST                      15
    #define     GYRO_IO_GET_REGISTER                   16
    #define     GYRO_IO_MEASURE_START                  17
    #define     GYRO_IO_MEASURE_STOP                   18
    #define     GYRO_IO_DIAG_GET_DATA                  19
    #define     GYRO_IO_DIAG_GET_RAD_DATA              20
    #define     GYRO_IO_SELF_TEST_MODE                 21
    #define     GYRO_IO_DIAG_GET_OFFSET_DATA           22
    #define     GYRO_IO_DIAG_READ_REGISTER             23
    #define     GYRO_IO_DIAG_WRITE_REGISTER            24
    #define     GYRO_IO_CTL_SET_NV                     25



    #define     GYSD_FUNC_GYRO                          1
/*    #define     GYSD_FUNC_GYRO_CALIBRATION              2 */
    #define     GYSD_MEASURE_MODE_NORMAL                0
    #define     GYSD_MEASURE_MODE_CALIBRATION           1
    #define     GYSD_HPF_ON                             1
    #define     GYSD_HPF_OFF                            0
    #define     GYSD_SELF_TEST_0                        0
    #define     GYSD_SELF_TEST_1                        1
    #define     GYSD_SELF_TEST_OK                       0
    #define     GYSD_SELF_TEST_NG                     (-1)

    #define     GYSD_SYSCAL_OK                          0
    #define     GYSD_SYSCAL_ERR                         (-1)

    struct l3g4200d_rad_data
    {
        float rad_x;
        float rad_y;
        float rad_z;
    };

    struct l3g4200d_lsb_data
    {
        short lsb_x;
        short lsb_y;
        short lsb_z;
    };

    struct l3g4200d_diag_offset_data
    {
        short lsb_x;
        short lsb_y;
        short lsb_z;
        int   count;
    };

    struct l3g4200d_diag_read_reg_data
    {
        unsigned char id_name;
        unsigned char id_value;
    };

    struct l3g4200d_diag_write_reg_data
    {
        unsigned char id_name;
        unsigned char id_value;
    };

    typedef struct tagGYSD_SENSOR_INFO
    {
        int                         x;
        int                         y;
        int                         z;
    }_GYSD_SENSOR_INFO;

    typedef struct tagGYSD_DEV_INFO
    {
        int                         update_flg;
        _GYSD_SENSOR_INFO           sensor_info;
    }_GYSD_DEV_INFO;

    typedef struct tagGYSD_FUNC_INFO
    {
        unsigned int                func;
    }_GYSD_FUNC_INFO;

    typedef struct tagGYSD_MEASURE_INFO
    {
        unsigned int                func;
        unsigned int                interval;
        unsigned int                mode;
    }_GYSD_MEASURE_INFO;

    typedef struct tagGYSD_SELF_TEST_INFO
    {
        unsigned int                mode;
        unsigned int                x_per;
        unsigned int                y_per;
        unsigned int                z_per;
        int                         result;
        int                         a_x;
        int                         a_y;
        int                         a_z;
        int                         b_x;
        int                         b_y;
        int                         b_z;
    }_GYSD_SELF_TEST_INFO;

    typedef struct tagGYSD_DEV_PARAM_INFO
    {
        unsigned int                scale;
    }_GYSD_DEV_PARAM_INFO;

    struct l3g4200d_platform_data
    {
        unsigned long data;
    };

#endif /* _gysdcom_h_ */

