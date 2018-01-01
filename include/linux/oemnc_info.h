#ifndef _LINUX_OEMNC_INFO_H
#define _LINUX_OEMNC_INFO_H

/* ========================================================================
FILE: oemnc_info.h
All Right Reserved, Copyright(c) NEC Casio Mobile Communications Co.,Ltd.
===========================================================================*/



#include <linux/types.h>

#define HW_REVISION_READ_ERR 0xFFFFFFFF

typedef enum {
  HW_REV_0P1  = 0x00000001,
  HW_REV_0P2  = 0x00000002,
  HW_REV_0P3  = 0x00000003,
  HW_REV_0P4  = 0x00000004,
  HW_REV_0P5  = 0x00000005,
  HW_REV_0P6  = 0x00000006,
  HW_REV_0P7  = 0x00000007,
  HW_REV_0P8  = 0x00000008,
  HW_REV_0P9  = 0x00000009,
  HW_REV_1P0  = 0x00010000,
  HW_REV_1P1  = 0x00010001,
  HW_REV_1P2  = 0x00010002,
  HW_REV_1P3  = 0x00010003,
  HW_REV_1P4  = 0x00010004,
  HW_REV_1P5  = 0x00010005,
  HW_REV_1P6  = 0x00010006,
  HW_REV_1P7  = 0x00010007,
  HW_REV_1P8  = 0x00010008,
  HW_REV_1P9  = 0x00010009,
  HW_REV_2P0  = 0x00020000,
  HW_REV_2P1  = 0x00020001,
  HW_REV_2P2  = 0x00020002,
  HW_REV_2P3  = 0x00020003,
  HW_REV_2P4  = 0x00020004,
  HW_REV_2P5  = 0x00020005,
  HW_REV_2P6  = 0x00020006,
  HW_REV_2P7  = 0x00020007,
  HW_REV_2P8  = 0x00020008,
  HW_REV_2P9  = 0x00020009,
  HW_REV_3P0  = 0x00030000,
  HW_REV_5P0  = 0x00050000,
  HW_REV_5P1  = 0x00050001,
  HW_REV_5P2  = 0x00050002,
} hardware_rev_type;

/*==========================================================================

  FUNCTION      hw_revision_read

  DESCRIPTION   HW revision read from SMEM.

  PARAMETERS    None.

  DEPENDENCIES  After the call of hw_init_smem().

  RETURN VALUE  Success - HW revision
                     Top 16bit:Major number
                   Under 16bit:Minor number

                Failure - HW_REVISION_READ_ERR

==========================================================================*/
extern uint32_t hw_revision_read (void);

#endif /* _LINUX_OEMNC_INFO_H */

