/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#ifndef MIPI_NT35560_H
#define MIPI_NT35560_H

//#define LG4573B_FWVGA_TWO_LANE
int mipi_nt35560_user_request_ctrl( struct msmfb_request_parame *data );

int mipi_nt35560_device_register(struct msm_panel_info *pinfo,
                                 u32 channel, u32 panel);

int mipi_nt35560_register_write_cmd( struct msm_fb_data_type *mfd,
                                     struct msmfb_register_write *data );

int mipi_nt35560_register_read_cmd( struct msm_fb_data_type *mfd,
                                    struct msmfb_register_read *data );
int mipi_nt35560_lcd_enable(struct msm_fb_data_type *mfd);

#endif  /* MIPI_LG4573B_H */
