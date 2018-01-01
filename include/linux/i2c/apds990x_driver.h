/*---------------------------------------------------------------------------
 * File Name: apds990x.h
 *
 * This file is the header file of the device driver for Avago APDS9900/APDS9901
 * digital proximity and ambient light sensor.
 *
 * (C) NEC Corporation 2011
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *---------------------------------------------------------------------------
 */

#ifndef APDS990X_H
#define APDS990X_H

#include <linux/ioctl.h>

#define D_PL_SENSOR_NV_NUM	23

struct apds990x_nv_data {
	unsigned long nv_reg[D_PL_SENSOR_NV_NUM];
};

typedef enum {
	APDS990X_DIAGGET_PROX,
	APDS990X_DIAGGET_LUX,
} apds990x_diagget_mode;

struct apds990x_diag_data_once {
	unsigned int CH0_data;
	unsigned int CH1_data;
	unsigned int prox_data;
};

struct apds990x_diag_data {
	apds990x_diagget_mode mode;			
	int count;							
	struct apds990x_diag_data_once *data;
};

struct apds990x_light_data {
	int mode;
	int lux;
};

struct apds990x_platform_data {
	int intr;
};

#define APDS990X_ENABLE_REG		0x00
#define APDS990X_ATIME_REG		0x01
#define APDS990X_PTIME_REG		0x02
#define APDS990X_WTIME_REG		0x03
#define APDS990X_AILTL_REG		0x04
#define APDS990X_AILTH_REG		0x05
#define APDS990X_AIHTL_REG		0x06
#define APDS990X_AIHTH_REG		0x07
#define APDS990X_PILTL_REG		0x08
#define APDS990X_PILTH_REG		0x09
#define APDS990X_PIHTL_REG		0x0A
#define APDS990X_PIHTH_REG		0x0B
#define APDS990X_PERS_REG		0x0C
#define APDS990X_CONFIG_REG		0x0D
#define APDS990X_PPCOUNT_REG		0x0E
#define APDS990X_CONTROL_REG		0x0F
#define APDS990X_REV_REG		0x11
#define APDS990X_ID_REG			0x12
#define APDS990X_STATUS_REG		0x13
#define APDS990X_CDATAL_REG		0x14
#define APDS990X_CDATAH_REG		0x15
#define APDS990X_IRDATAL_REG		0x16
#define APDS990X_IRDATAH_REG		0x17
#define APDS990X_PDATAL_REG		0x18
#define APDS990X_PDATAH_REG		0x19
#define APDS990X_MAX_REG		(APDS990X_PDATAH_REG)

#define NV_SH_PROX_ALS_ATIME		0
#define NV_SH_PROX_ALS_PTIME		1
#define NV_SH_PROX_ALS_WTIME		2
#define NV_SH_PROX_ALS_WTIME2		3
#define NV_SH_PROX_ALS_PILTL		4
#define NV_SH_PROX_ALS_PILTH		5
#define NV_SH_PROX_ALS_H_PIHTL		6
#define NV_SH_PROX_ALS_H_PIHTH		7
#define NV_SH_PROX_ALS_L_PILTL		8
#define NV_SH_PROX_ALS_L_PILTH		9
#define NV_SH_PROX_ALS_PIHTL		10
#define NV_SH_PROX_ALS_PIHTH		11
#define NV_SH_PROX_ALS_PERS		12
#define NV_SH_PROX_ALS_CONFIG		13
#define NV_SH_PROX_ALS_PPCOUNT		14
#define NV_SH_PROX_ALS_CONTROL		15
#define NV_SH_PROX_ADJ			16
#define NV_SH_PROX_ALS_DET_CANCEL	17
#define NV_SH_ALS_COEFF_GA		18
#define NV_SH_ALS_COEFF_B		19
#define NV_SH_ALS_COEFF_C		20
#define NV_SH_ALS_COEFF_D		21
#define NV_SH_ALS_COEFF_K		22

#define DEVICE_FACTOR			52
#define LIGHT_AVERAGE_NUMBER		5

#define D_PS_DETECT_STATE_NON_DETECT	0
#define D_PS_DETECT_STATE_DETECT	1

#define D_LIGHT_MODE_LATEST		0
#define D_LIGHT_MODE_AVERAGE		1

#define APDS990X_DIAG_DATA_WAIT		0
#define APDS990X_DIAG_DATA_UNWAIT	1

#define APDS990X_IO	'A'

#define APDS990X_IOCTL_LIGHT_MEAS_START		_IO(APDS990X_IO, 1)
#define APDS990X_IOCTL_LIGHT_MEAS_STOP		_IO(APDS990X_IO, 2)
#define APDS990X_IOCTL_PROX_MEAS_START		_IO(APDS990X_IO, 3)
#define APDS990X_IOCTL_PROX_MEAS_STOP		_IO(APDS990X_IO, 4)
#define APDS990X_IOCTL_GET_NV				_IOR(APDS990X_IO, 5, struct apds990x_nv_data)
#define APDS990X_IOCTL_GET_DISTANCE			_IOR(APDS990X_IO, 6, int)
#define APDS990X_IOCTL_DIAG_GET_DATA		_IOWR(APDS990X_IO, 7, struct apds990x_diag_data)
#define APDS990X_IOCTL_SET_NV				_IOW(APDS990X_IO, 8, struct apds990x_nv_data)
#define APDS990X_IOCTL_DIAG_GET_REGISTER	_IOWR(APDS990X_IO, 10, unsigned char)
#define APDS990X_IOCTL_GET_LUX				_IOWR(APDS990X_IO, 11, struct apds990x_light_data)
#define APDS990X_IOCTL_DIAG_SET_NV			_IOW(APDS990X_IO, 12, struct apds990x_nv_data)
#define APDS990X_IOCTL_DIAG_GET_NV			_IOR(APDS990X_IO, 13, struct apds990x_nv_data)

#endif
