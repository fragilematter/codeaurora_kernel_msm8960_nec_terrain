/*  Copyright (c) 2010  Christoph Mair <christoph.mair@gmail.com>
    Copyright (c) 2011  Bosch Sensortec GmbH
    Copyright (c) 2011  Unixphere AB

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#ifndef YAS530_DRIVER_H
#define YAS530_DRIVER_H

#include <linux/ioctl.h>

#define YAS530_CHIP_ID                      0x01

#define YAS530_DEVICE_ID_REG				0x80
#define YAS530_RESET_COIL_DRIVING_REG		0x81
#define YAS530_ACQUISITION_COMMAND_REG		0x82
#define YAS530_CONFIG_REG					0x83
#define YAS530_ACQUISITION_INTERVAL_REG		0x84
#define YAS530_OFFSET_X_REG					0x85
#define YAS530_OFFSET_Y1_REG				0x86
#define YAS530_OFFSET_Y2_REG				0x87
#define YAS530_OFFSET_START_REG				0x85
#define YAS530_TEST1_REG					0x88
#define YAS530_TEST2_REG					0x89
#define YAS530_CAL_REG						0x90
#define YAS530_ACQUISITION_TEMP_START_REG	0xB0
#define YAS530_ACQUISITION_MAG_START_REG	0xB2
#define YAS530_COMMAND_DATA_FORS_1			0x05
#define YAS530_COMMAND_DATA_FORS_0			0x01
#define YAS530_COMMAND_COIL_ON				0x03
#define YAS530_READ_REG_START_1				0x80
#define YAS530_READ_REG_START_2				0xB0
#define YAS530_SELECT_DLYMES_NORMAL			0x01
#define YAS530_SELECT_DLYMES_DEFERRED		0x11

#define D_MAGNETIC_SENSOR_NV_NUM		7


#define YAS530_IO	'y'

#define YAS530_IOCTL_MEASURE_START						_IO(YAS530_IO, 0)
#define YAS530_IOCTL_MEASURE_STOP						_IO(YAS530_IO, 1)
#define YAS530_IOCTL_SET_OFFSET_NV						_IOW(YAS530_IO, 2, struct yas530_mag_nv_data)
#define YAS530_IOCTL_GET_OFFSET_NV						_IOR(YAS530_IO, 3, struct yas530_mag_nv_data)
#define YAS530_IOCTL_GET_DATA							_IOR(YAS530_IO, 4, struct yas530_measure_data)
#define YAS530_IOCTL_DIAG_REG_INIT						_IOR(YAS530_IO, 5, struct yas530_mag_regist_data)
#define YAS530_IOCTL_DIAG_RESET							_IO(YAS530_IO, 6)
#define YAS530_IOCTL_DIAG_REG_WRITE						_IOW(YAS530_IO, 7, struct yas530_mag_regist_data)
#define YAS530_IOCTL_DIAG_GET_MAGNE						_IOR(YAS530_IO, 8, struct yas530_mag_user_data)
#define YAS530_IOCTL_DIAG_SELF_TEST						_IOR(YAS530_IO, 9, struct yas530_mag_selftest_data)
#define YAS530_IOCTL_DIAG_GET_OFFSET					_IOR(YAS530_IO, 10, struct yas530_mag_offset_data)
#define YAS530_IOCTL_DIAG_TEST_REG						_IOR(YAS530_IO, 11, struct yas530_mag_test_reg)
#define YAS530_IOCTL_DIAG_CALC_DATA						_IOR(YAS530_IO, 12, struct yas530_mag_calc_data)
#define YAS530_IOCTL_DIAG_REG_READ						_IOR(YAS530_IO, 13, struct yas530_mag_test_reg)
////#define YAS530_IOCTL_GET_NV								_IOR(YAS530_IO, 2, struct yas530_mag_nv_data)
////#define YAS530_IOCTL_DIAG_REG_COMP						_IOR(YAS530_IO, 7, struct yas530_mag_regist_data)

/*
 * struct bmp18x_platform_data - represents platform data for the bmp18x driver
 * @chip_id: Configurable chip id for non-default chip revisions
 * @default_oversampling: Default oversampling value to be used at startup,
 * value range is 0-3 with rising sensitivity.
 * @default_sw_oversampling: Default software oversampling value to be used
 * at startup,value range is 0(Disabled) or 1(Enabled). Only take effect
 * when default_oversampling is 3.
 * @temp_measurement_period: Temperature measurement period (milliseconds), set
 * to zero if unsure.
 * @init_hw: Callback for hw specific startup
 * @deinit_hw: Callback for hw specific shutdown
*/

struct yas530_mag_regist_data 
{
	unsigned char	regist_data80;
	unsigned char	regist_data84;
	unsigned char	regist_data85;
	unsigned char	regist_data86;
	unsigned char	regist_data87;
	unsigned char	regist_dataB0;
	unsigned char	regist_dataB1;
	unsigned char	regist_dataB2;
	unsigned char	regist_dataB3;
	unsigned char	regist_dataB4;
	unsigned char	regist_dataB5;
	unsigned char	regist_dataB6;
	unsigned char	regist_dataB7;
};

/*---------------------------------------------*/
/*      F2 COMMAND STRUCT  				       */
/*---------------------------------------------*/
struct yas530_mag_user_data
{
	signed short		temp_data;
	signed short		d_x_axis;	
	signed short		d_y_axis;	
	signed short		d_z_axis;	
	signed short		x_sensor;		/*sensor X				*/
	signed short		y1_sensor;		/*sensor Y1				*/
	signed short		y2_sensor;		/*sensor Y2				*/
};

struct yas530_mag_calc_data 
{
	signed int	m_cx, m_cy1, m_cy2, m_a2;
	signed int	m_a3, m_a4, m_a5, m_a6;
	signed int	m_a7, m_a8, m_a9, m_k;
};

struct yas530_data 
{
	unsigned char	chip_id;
	unsigned char	dx, dy1, dy2, d2;
	unsigned char	d3, d4, d5, d6;
	unsigned char	d7, d8, d9, d0;
	unsigned char	dck;	
};
/*---------------------------------------------*/
/*      F3 COMMAND STRUCT 				       */
/*---------------------------------------------*/
struct yas530_mag_selftest_data
{
	signed short		xm_sensor;		/*sensor X(Minus)		*/
	signed short		xp_sensor;		/*sensor X(Plus)		*/
	signed short		y1m_sensor;		/*sensor Y1(Minus)		*/
	signed short		y1p_sensor;		/*sensor Y1(Plus)		*/
	signed short		y2m_sensor;		/*sensor Y2(Minus)		*/
	signed short		y2p_sensor;		/*sensor Y2(Plus)		*/
};
/*---------------------------------------------*/
/*      F4 COMMAND STRUCT 				       */
/*---------------------------------------------*/
struct yas530_mag_offset_data 
{
	signed char		x_offset;			/*sensor x  offset		*/
	signed char		y1_offset;			/*sensor y1 offset		*/
	signed char		y2_offset;			/*sensor y2 offset		*/

};
/*---------------------------------------------*/
/*     For NV DATA COMMAND STRUCT              */
/*---------------------------------------------*/
struct yas530_mag_nv_data 
{
	unsigned char nv_reg[D_MAGNETIC_SENSOR_NV_NUM];
};

/*---------------------------------------------*/
/*      F10 COMMAND STRUCT 				       */
/*---------------------------------------------*/
struct yas530_mag_test_reg 
{
	unsigned char		test_reg_addr;		
	unsigned char		test_reg_data;		
};

/*---------------------------------------------*/
/*      Platform Data	 				       */
/*---------------------------------------------*/

struct yas530_platform_data {
	int reset;
};

#endif
