#ifdef CONFIG_FEATURE_NCMC_USB
/*
 * include/linux/usb/oem_usb_commmon.h
 *
 * (C) NEC CASIO Mobile Communications, Ltd. 2012
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
 */

#ifndef _OEM_USB_COMMON_H_
#define _OEM_USB_COMMON_H_

#if defined(CONFIG_FEATURE_NCMC_RUBY)

/* function */
#define NCMC_USB_COMPOSITE_DEVICE
#define NCMC_USB_HOSTDRIVER
#undef NCMC_USB_HW_SW
#undef NCMC_USB_FUNCTION_NCMC_SERIAL

/* vid/pid */
#define NCMC_USB_VID                0x0409
#define NCMC_USB_PRODUCT_NAME       "NA002"

#define NCMC_USB_PID_DEFAULT        0x0358
#define NCMC_USB_PID_IPOVERUSB      0x0359
#define NCMC_USB_PID_TETHERING      0x035A
#define NCMC_USB_PID_MTP            0x035C
#define NCMC_USB_PID_PTP            0x035D

/* strings */
#define NCMC_USB_STORAGE_NAME       "NA002 Storage"

#define NCMC_USB_IF_DESC_NAME_ADB           "NA002 Android ADB Interface"
#define NCMC_USB_IF_DESC_NAME_DIAG          "NA002 Virtual Serial Port"
#define NCMC_USB_IF_DESC_NAME_MSC           "NA002 Mass Storage"
#define NCMC_USB_IF_DESC_NAME_NCM           "NA002 USB Networking Interface"
#define NCMC_USB_IF_DESC_NAME_MTP           "NA002"
#define NCMC_USB_IF_DESC_NAME_PTP           "NA002"
#define NCMC_USB_IF_DESC_NAME_NCMC_SERIAL   "NA002"

#define NCMC_USB_VENDOR_NAME        "NEC"
#define NCMC_USB_MANUFACTURER_NAME  "NEC Corporation"

#elif defined(CONFIG_FEATURE_NCMC_D121F)
/* Fairy */

/* function */
#define NCMC_USB_COMPOSITE_DEVICE
#define NCMC_USB_HOSTDRIVER
#define NCMC_USB_HW_SW
#define NCMC_USB_FUNCTION_NCMC_SERIAL

/* vid/pid */
#define NCMC_USB_VID                0x0409
#define NCMC_USB_PRODUCT_NAME       "N-07D"

#define NCMC_USB_PID_DEFAULT        0x0340
#define NCMC_USB_PID_IPOVERUSB      0x0341
#define NCMC_USB_PID_TETHERING      0x0342
#define NCMC_USB_PID_MTP            0x0345
#define NCMC_USB_PID_PTP            0x0346
#define NCMC_USB_PID_NCMC_SERIAL    0x0343

/* strings */
#define NCMC_USB_STORAGE_NAME       "N-07D Storage"

#define NCMC_USB_IF_DESC_NAME_ADB           "N-07D Android ADB Interface"
#define NCMC_USB_IF_DESC_NAME_DIAG          "N-07D Virtual Serial Port"
#define NCMC_USB_IF_DESC_NAME_MSC           "N-07D Mass Storage"
#define NCMC_USB_IF_DESC_NAME_NCM           "N-07D USB Networking Interface"
#define NCMC_USB_IF_DESC_NAME_MTP           "N-07D"
#define NCMC_USB_IF_DESC_NAME_PTP           "N-07D"
#define NCMC_USB_IF_DESC_NAME_NCMC_SERIAL   "DOCOMO N-07D"

#define NCMC_USB_VENDOR_NAME        "NEC"
#define NCMC_USB_MANUFACTURER_NAME  "NEC Corporation"   

#elif defined(CONFIG_FEATURE_NCMC_D121M)
/* Frontier */

/* function */
#define NCMC_USB_COMPOSITE_DEVICE
#define NCMC_USB_HOSTDRIVER
#undef NCMC_USB_HW_SW
#undef NCMC_USB_FUNCTION_NCMC_SERIAL

/* vid/pid */
#define NCMC_USB_VID                0x0409
#define NCMC_USB_PRODUCT_NAME       "N-08D"

#define NCMC_USB_PID_DEFAULT        0x0350
#define NCMC_USB_PID_IPOVERUSB      0x0351
#define NCMC_USB_PID_TETHERING      0x0352
#define NCMC_USB_PID_MTP            0x0354
#define NCMC_USB_PID_PTP            0x0355

/* strings */
#define NCMC_USB_STORAGE_NAME       "N-08D Storage"

#define NCMC_USB_IF_DESC_NAME_ADB   "N-08D Android ADB Interface"
#define NCMC_USB_IF_DESC_NAME_DIAG  "N-08D Virtual Serial Port"
#define NCMC_USB_IF_DESC_NAME_MSC   "N-08D Mass Storage"
#define NCMC_USB_IF_DESC_NAME_NCM   "N-08D USB Networking Interface"
#define NCMC_USB_IF_DESC_NAME_MTP   "N-08D"
#define NCMC_USB_IF_DESC_NAME_PTP   "N-08D"

#define NCMC_USB_VENDOR_NAME        "NEC"
#define NCMC_USB_MANUFACTURER_NAME  "NEC Corporation"   

#elif defined(CONFIG_FEATURE_NCMC_G121S)
/* Slim+C */

/* function */
#define NCMC_USB_COMPOSITE_DEVICE
#define NCMC_USB_HOSTDRIVER
#define NCMC_USB_HW_SW
#undef NCMC_USB_FUNCTION_NCMC_SERIAL

/* vid/pid */
#define NCMC_USB_VID                0x0409
#define NCMC_USB_PRODUCT_NAME       "N-SLM"

#define NCMC_USB_PID_DEFAULT        0x0368
#define NCMC_USB_PID_IPOVERUSB      0x0369
#define NCMC_USB_PID_TETHERING      0x036A
#define NCMC_USB_PID_MTP            0x036D
#define NCMC_USB_PID_PTP            0x036E

/* strings */
#define NCMC_USB_STORAGE_NAME       "N-SLM Storage"

#define NCMC_USB_IF_DESC_NAME_ADB   "N-SLM Android ADB Interface"
#define NCMC_USB_IF_DESC_NAME_DIAG  "N-SLM Virtual Serial Port"
#define NCMC_USB_IF_DESC_NAME_MSC   "N-SLM Mass Storage"
#define NCMC_USB_IF_DESC_NAME_NCM   "N-SLM USB Networking Interface"
#define NCMC_USB_IF_DESC_NAME_MTP   "N-SLM"

#define NCMC_USB_VENDOR_NAME        "NEC"
#define NCMC_USB_MANUFACTURER_NAME  "NEC Corporation"   

#elif defined(CONFIG_FEATURE_NCMC_G121T)
/* Tomahawk */
/* delete */

/* function */
#define NCMC_USB_COMPOSITE_DEVICE
#define NCMC_USB_HOSTDRIVER
#define NCMC_USB_HW_SW
#undef NCMC_USB_FUNCTION_NCMC_SERIAL

/* vid/pid */
#define NCMC_USB_VID                0x0409
#define NCMC_USB_PRODUCT_NAME       "N-TOM"

/* strings */
#define NCMC_USB_PID_DEFAULT        0x0358
#define NCMC_USB_PID_IPOVERUSB      0x0359
#define NCMC_USB_PID_TETHERING      0x035A
#define NCMC_USB_PID_MTP            0x035C
#define NCMC_USB_PID_PTP            0x035D

#define NCMC_USB_STORAGE_NAME       "N-TOM Storage"

#define NCMC_USB_IF_DESC_NAME_ADB   "N-TOM Android ADB Interface"
#define NCMC_USB_IF_DESC_NAME_DIAG  "N-TOM Virtual Serial Port"
#define NCMC_USB_IF_DESC_NAME_MSC   "N-TOM Mass Storage"
#define NCMC_USB_IF_DESC_NAME_NCM   "N-TOM USB Networking Interface"
#define NCMC_USB_IF_DESC_NAME_MTP   "N-TOM"

#define NCMC_USB_VENDOR_NAME        "NEC"
#define NCMC_USB_MANUFACTURER_NAME  "NEC Corporation"   

#endif

#endif /* _OEM_USB_COMMON_H_ */
#endif /* CONFIG_FEATURE_NCMC_USB */
