
/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#ifndef MIPI_RENESAS_G121S_H
#define MIPI_RENESAS_G121S_H

/* Driver Internal Status */
typedef enum {
    MIPI_RENESAS_G121S_STATE_OFF,          /* Power Off */
    MIPI_RENESAS_G121S_STATE_READY,        /* not used */
    MIPI_RENESAS_G121S_STATE_STANDBY,      /* Power On. Display Off. */
    MIPI_RENESAS_G121S_STATE_DEEP_STANDBY, /* Deep Standby */
    MIPI_RENESAS_G121S_STATE_NORMAL_MODE,  /* Power On. Display On */
} mipi_renesas_g121s_state_t;

int mipi_renesas_g121s_device_register(struct msm_panel_info *pinfo, u32 channel, u32 panel);

/* for diag test program */
void mipi_renesas_g121s_set_idle_state(struct msm_fb_data_type *mfd);
int  mipi_renesas_g121s_standby_ctl(int on, struct msm_fb_data_type *mfd);
int  mipi_renesas_g121s_deep_standby_ctl(int on, struct msm_fb_data_type *mfd);
int mipi_renesas_g121s_power_seq(int on, struct msm_fb_data_type *mfd);

int mipi_renesas_g121s_user_request_ctrl( struct msmfb_request_parame *data );

int mipi_renesas_g121s_register_write_cmd( struct msm_fb_data_type *mfd, 
                                     struct msmfb_register_write *data );
int mipi_renesas_g121s_register_read_cmd( struct msm_fb_data_type *mfd, 
                                     struct msmfb_register_read *data );
int mipi_renesas_g121s_power_ctl( int on );

void mipi_renesas_g121s_enable_display(struct msm_fb_data_type *mfd);
#endif  /* MIPI_RENESAS_G121S_H */
