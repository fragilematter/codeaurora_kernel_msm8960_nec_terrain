#ifndef __CODEC_DIAG_H
#define __CODEC_DIAG_H

#include "../linux/msm_audio.h"

/* For Real-Time Audio Calibration */
#define AUDIO_SET_CODEC_WRITE    _IOWR(AUDIO_IOCTL_MAGIC, \
                        (AUDIO_MAX_COMMON_IOCTL_NUM+51), unsigned)
#define AUDIO_GET_CODEC_READ     _IOWR(AUDIO_IOCTL_MAGIC, \
                        (AUDIO_MAX_COMMON_IOCTL_NUM+52), unsigned)
#define AUDIO_SET_CODEC_VOL_CAL  _IOWR(AUDIO_IOCTL_MAGIC, \
                        (AUDIO_MAX_COMMON_IOCTL_NUM+53), unsigned)

struct codec_diag_ctl {
	uint32_t	ctl1;
	uint32_t	ctl2;
	uint32_t	ctl3;
	uint32_t	ctl4;
};

#endif /* __CODEC_DIAG_H */
