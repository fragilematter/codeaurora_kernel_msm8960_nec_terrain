/*
 * f_ncm.c -- USB CDC Network (NCM) link function driver
 *
 * Copyright (C) 2010 Nokia Corporation
 * Contact: Yauheni Kaliuta <yauheni.kaliuta@nokia.com>
 *
 * The driver borrows from f_ecm.c which is:
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef CONFIG_FEATURE_NCMC_USB
/**************************************************/
/* Modified by                                    */
/* (C) NEC CASIO Mobile Communications, Ltd. 2012 */
/**************************************************/

#endif /* CONFIG_FEATURE_NCMC_USB */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/crc32.h>

#include <linux/usb/cdc.h>

#include "u_ether.h"

#undef USB_ANDROID_NCM
#ifdef CONFIG_FEATURE_NCMC_USB
#include <linux/usb/oem_usb_common.h>
#define USB_ANDROID_NCM
#endif /* CONFIG_FEATURE_NCMC_USB */

/*
 * This function is a "CDC Network Control Model" (CDC NCM) Ethernet link.
 * NCM is intended to be used with high-speed network attachments.
 *
 * Note that NCM requires the use of "alternate settings" for its data
 * interface.  This means that the set_alt() method has real work to do,
 * and also means that a get_alt() method is required.
 */

/* to trigger crc/non-crc ndp signature */

#define NCM_NDP_HDR_CRC_MASK	0x01000000
#define NCM_NDP_HDR_CRC		0x01000000
#define NCM_NDP_HDR_NOCRC	0x00000000

struct ncm_ep_descs {
	struct usb_endpoint_descriptor	*in;
	struct usb_endpoint_descriptor	*out;
	struct usb_endpoint_descriptor	*notify;
};

enum ncm_notify_state {
	NCM_NOTIFY_NONE,		/* don't notify */
	NCM_NOTIFY_CONNECT,		/* issue CONNECT next */
	NCM_NOTIFY_SPEED,		/* issue SPEED_CHANGE next */
};

#ifdef USB_ANDROID_NCM
#define	CDC_NCM_DPT_DATAGRAMS_MAX		8	// NOT CONFIRMED

struct cdc_ncm_data {
	struct usb_cdc_ncm_nth16 nth16;
	struct usb_cdc_ncm_ndp16 ndp16;
	struct usb_cdc_ncm_dpe16 dpe16[CDC_NCM_DPT_DATAGRAMS_MAX + 1];
	struct usb_cdc_ncm_nth32 nth32;
	struct usb_cdc_ncm_ndp32 ndp32;
	struct usb_cdc_ncm_dpe32 dpe32[CDC_NCM_DPT_DATAGRAMS_MAX + 1];
};

struct cdc_ncm_ctx {
	 struct cdc_ncm_data tx_ncm;
	 struct timer_list tx_timer;
	 struct sk_buff *tx_curr_skb;
	 struct sk_buff *tx_rem_skb;
	 struct net_device *net;
#ifndef USE_ETHER_XMIT_FOR_NCM		 
	 spinlock_t tx_lock;
#endif	 
	 u32 tx_curr_offset;
	 u32 tx_curr_last_offset;
	 u32 tx_curr_frame_num;
	 u32 tx_max;
	 u32 max_datagram_size;
	 u16 tx_max_datagrams;
	 u16 tx_remainder;
	 u16 tx_modulus;
	 u16 tx_ndp_modulus;
	 u16 tx_seq;

	 u16 sizeof_ncm_nth16;
	 u16 sizeof_ncm_ndp16;
	 u16 sizeof_usb_cdc_ncm_nth16;
	 u16 sizeof_usb_cdc_ncm_dpe16;	
	 u16 sizeof_usb_cdc_ncm_ndp16; 

	 u16 sizeof_ncm_nth32;
	 u16 sizeof_ncm_ndp32;
	 u16 sizeof_usb_cdc_ncm_nth32;
	 u16 sizeof_usb_cdc_ncm_dpe32;	
	 u16 sizeof_usb_cdc_ncm_ndp32; 
};
#endif /* USB_ANDROID_NCM */

struct f_ncm {
	struct gether			port;
	u8				ctrl_id, data_id;

	char				ethaddr[14];

	struct ncm_ep_descs		fs;
	struct ncm_ep_descs		hs;

	struct usb_ep			*notify;
	struct usb_endpoint_descriptor	*notify_desc;
	struct usb_request		*notify_req;
	u8				notify_state;
	bool				is_open;

	struct ndp_parser_opts		*parser_opts;
	bool				is_crc;

	/*
	 * for notification, it is accessed from both
	 * callback and ethernet open/close
	 */
	spinlock_t			lock;

#ifdef USB_ANDROID_NCM
	void 				*ctx;
#endif /* USB_ANDROID_NCM */
};

static inline struct f_ncm *func_to_ncm(struct usb_function *f)
{
	return container_of(f, struct f_ncm, port.func);
}

/* peak (theoretical) bulk transfer rate in bits-per-second */
static inline unsigned ncm_bitrate(struct usb_gadget *g)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return 13 * 512 * 8 * 1000 * 8;
	else
		return 19 *  64 * 1 * 1000 * 8;
}

#ifdef USB_ANDROID_NCM
extern unsigned int qmult;
#endif /* USB_ANDROID_NCM */

/*-------------------------------------------------------------------------*/

/*
 * We cannot group frames so use just the minimal size which ok to put
 * one max-size ethernet frame.
 * If the host can group frames, allow it to do that, 16K is selected,
 * because it's used by default by the current linux host driver
 */
#ifdef USB_ANDROID_NCM
#define NTB_DEFAULT_IN_SIZE	(1024*4)
#define NTB_OUT_SIZE		(1024*4)
//#define NTB_DEFAULT_IN_SIZE	(1024*32)
//#define NTB_OUT_SIZE		(1024*32)
#else /* USB_ANDROID_NCM */
#define NTB_DEFAULT_IN_SIZE	USB_CDC_NCM_NTB_MIN_IN_SIZE
#define NTB_OUT_SIZE		16384
#endif /* USB_ANDROID_NCM */

/*
 * skbs of size less than that will not be aligned
 * to NCM's dwNtbInMaxSize to save bus bandwidth
 */

#ifdef USB_ANDROID_NCM
#else /* USB_ANDROID_NCM */
#define	MAX_TX_NONFIXED		(512 * 3)
#endif /* USB_ANDROID_NCM */

#define FORMATS_SUPPORTED	(USB_CDC_NCM_NTB16_SUPPORTED |	\
				 USB_CDC_NCM_NTB32_SUPPORTED)

static struct usb_cdc_ncm_ntb_parameters ntb_parameters = {
	.wLength = sizeof ntb_parameters,
	.bmNtbFormatsSupported = cpu_to_le16(FORMATS_SUPPORTED),
	.dwNtbInMaxSize = cpu_to_le32(NTB_DEFAULT_IN_SIZE),
	.wNdpInDivisor = cpu_to_le16(4),
	.wNdpInPayloadRemainder = cpu_to_le16(0),
	.wNdpInAlignment = cpu_to_le16(4),

	.dwNtbOutMaxSize = cpu_to_le32(NTB_OUT_SIZE),
	.wNdpOutDivisor = cpu_to_le16(4),
	.wNdpOutPayloadRemainder = cpu_to_le16(0),
	.wNdpOutAlignment = cpu_to_le16(4),
};

/*
 * Use wMaxPacketSize big enough to fit CDC_NOTIFY_SPEED_CHANGE in one
 * packet, to simplify cancellation; and a big transfer interval, to
 * waste less bandwidth.
 */

#define LOG2_STATUS_INTERVAL_MSEC	5	/* 1 << 5 == 32 msec */
#define NCM_STATUS_BYTECOUNT		16	/* 8 byte header + data */

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_interface_assoc_descriptor ncm_iad_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_interface_assoc_descriptor ncm_iad_desc __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof ncm_iad_desc,
	.bDescriptorType =	USB_DT_INTERFACE_ASSOCIATION,

	/* .bFirstInterface =	DYNAMIC, */
	.bInterfaceCount =	2,	/* control + data */
	.bFunctionClass =	USB_CLASS_COMM,
	.bFunctionSubClass =	USB_CDC_SUBCLASS_NCM,
	.bFunctionProtocol =	USB_CDC_PROTO_NONE,
	/* .iFunction =		DYNAMIC */
};

/* interface descriptor: */

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_interface_descriptor ncm_control_intf = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_interface_descriptor ncm_control_intf __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof ncm_control_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_COMM,
	.bInterfaceSubClass =	USB_CDC_SUBCLASS_NCM,
	.bInterfaceProtocol =	USB_CDC_PROTO_NONE,
	/* .iInterface = DYNAMIC */
};

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_cdc_header_desc ncm_header_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_cdc_header_desc ncm_header_desc __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof ncm_header_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		cpu_to_le16(0x0110),
};

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_cdc_union_desc ncm_union_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_cdc_union_desc ncm_union_desc __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof(ncm_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_cdc_ether_desc ecm_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_cdc_ether_desc ecm_desc __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof ecm_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ETHERNET_TYPE,

	/* this descriptor actually adds value, surprise! */
	/* .iMACAddress = DYNAMIC */
	.bmEthernetStatistics =	cpu_to_le32(0), /* no statistics */
	.wMaxSegmentSize =	cpu_to_le16(ETH_FRAME_LEN),
	.wNumberMCFilters =	cpu_to_le16(0),
	.bNumberPowerFilters =	0,
};

#define NCAPS	(USB_CDC_NCM_NCAP_ETH_FILTER | USB_CDC_NCM_NCAP_CRC_MODE)

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_cdc_ncm_desc ncm_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_cdc_ncm_desc ncm_desc __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof ncm_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_NCM_TYPE,

	.bcdNcmVersion =	cpu_to_le16(0x0100),
	/* can process SetEthernetPacketFilter */
	.bmNetworkCapabilities = NCAPS,
};

/* the default data interface has no endpoints ... */

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_interface_descriptor ncm_data_nop_intf = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_interface_descriptor ncm_data_nop_intf __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof ncm_data_nop_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	USB_CDC_NCM_PROTO_NTB,
	/* .iInterface = DYNAMIC */
};

/* ... but the "real" data interface has two bulk endpoints */

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_interface_descriptor ncm_data_intf = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_interface_descriptor ncm_data_intf __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		sizeof ncm_data_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	1,
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	USB_CDC_NCM_PROTO_NTB,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

#ifdef CONFIG_FEATURE_NCMC_USB
static struct usb_endpoint_descriptor fs_ncm_notify_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_endpoint_descriptor fs_ncm_notify_desc __initdata = {
#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(NCM_STATUS_BYTECOUNT),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_endpoint_descriptor fs_ncm_in_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_endpoint_descriptor fs_ncm_in_desc __initdata = {

#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_endpoint_descriptor fs_ncm_out_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_endpoint_descriptor fs_ncm_out_desc __initdata = {

#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_descriptor_header *ncm_fs_function[] = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_descriptor_header *ncm_fs_function[] __initdata = {

#endif /* CONFIG_FEATURE_NCMC_USB */
	(struct usb_descriptor_header *) &ncm_iad_desc,
	/* CDC NCM control descriptors */
	(struct usb_descriptor_header *) &ncm_control_intf,
	(struct usb_descriptor_header *) &ncm_header_desc,
	(struct usb_descriptor_header *) &ncm_union_desc,
	(struct usb_descriptor_header *) &ecm_desc,
	(struct usb_descriptor_header *) &ncm_desc,
	(struct usb_descriptor_header *) &fs_ncm_notify_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &ncm_data_nop_intf,
	(struct usb_descriptor_header *) &ncm_data_intf,
	(struct usb_descriptor_header *) &fs_ncm_in_desc,
	(struct usb_descriptor_header *) &fs_ncm_out_desc,
	NULL,
};

/* high speed support: */

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_endpoint_descriptor hs_ncm_notify_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_endpoint_descriptor hs_ncm_notify_desc __initdata = {

#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(NCM_STATUS_BYTECOUNT),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_endpoint_descriptor hs_ncm_in_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_endpoint_descriptor hs_ncm_in_desc __initdata = {

#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_endpoint_descriptor hs_ncm_out_desc = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_endpoint_descriptor hs_ncm_out_desc __initdata = {

#endif /* CONFIG_FEATURE_NCMC_USB */
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_descriptor_header *ncm_hs_function[] = {
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_descriptor_header *ncm_hs_function[] __initdata = {

#endif /* CONFIG_FEATURE_NCMC_USB */
	(struct usb_descriptor_header *) &ncm_iad_desc,
	/* CDC NCM control descriptors */
	(struct usb_descriptor_header *) &ncm_control_intf,
	(struct usb_descriptor_header *) &ncm_header_desc,
	(struct usb_descriptor_header *) &ncm_union_desc,
	(struct usb_descriptor_header *) &ecm_desc,
	(struct usb_descriptor_header *) &ncm_desc,
	(struct usb_descriptor_header *) &hs_ncm_notify_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &ncm_data_nop_intf,
	(struct usb_descriptor_header *) &ncm_data_intf,
	(struct usb_descriptor_header *) &hs_ncm_in_desc,
	(struct usb_descriptor_header *) &hs_ncm_out_desc,
	NULL,
};

/* string descriptors: */

#define STRING_CTRL_IDX	0
#define STRING_MAC_IDX	1
#define STRING_DATA_IDX	2
#define STRING_IAD_IDX	3

#ifdef CONFIG_FEATURE_NCMC_USB

static struct usb_string ncm_string_defs[] = {
	[STRING_CTRL_IDX].s = NCMC_USB_IF_DESC_NAME_NCM,
	[STRING_MAC_IDX].s = NULL /* DYNAMIC */,
	{  } /* end of list */
};
#else /* CONFIG_FEATURE_NCMC_USB */
static struct usb_string ncm_string_defs[] = {
	[STRING_CTRL_IDX].s = "CDC Network Control Model (NCM)",
	[STRING_MAC_IDX].s = NULL /* DYNAMIC */,
	[STRING_DATA_IDX].s = "CDC Network Data",
	[STRING_IAD_IDX].s = "CDC NCM",
	{  } /* end of list */
};

#endif /* CONFIG_FEATURE_NCMC_USB */

static struct usb_gadget_strings ncm_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		ncm_string_defs,
};

static struct usb_gadget_strings *ncm_strings[] = {
	&ncm_string_table,
	NULL,
};

/*
 * Here are options for NCM Datagram Pointer table (NDP) parser.
 * There are 2 different formats: NDP16 and NDP32 in the spec (ch. 3),
 * in NDP16 offsets and sizes fields are 1 16bit word wide,
 * in NDP32 -- 2 16bit words wide. Also signatures are different.
 * To make the parser code the same, put the differences in the structure,
 * and switch pointers to the structures when the format is changed.
 */

struct ndp_parser_opts {
	u32		nth_sign;
	u32		ndp_sign;
	unsigned	nth_size;
	unsigned	ndp_size;
	unsigned	ndplen_align;
	/* sizes in u16 units */
	unsigned	dgram_item_len; /* index or length */
	unsigned	block_length;
	unsigned	fp_index;
	unsigned	reserved1;
	unsigned	reserved2;
	unsigned	next_fp_index;
};

#define INIT_NDP16_OPTS {					\
		.nth_sign = USB_CDC_NCM_NTH16_SIGN,		\
		.ndp_sign = USB_CDC_NCM_NDP16_NOCRC_SIGN,	\
		.nth_size = sizeof(struct usb_cdc_ncm_nth16),	\
		.ndp_size = sizeof(struct usb_cdc_ncm_ndp16),	\
		.ndplen_align = 4,				\
		.dgram_item_len = 1,				\
		.block_length = 1,				\
		.fp_index = 1,					\
		.reserved1 = 0,					\
		.reserved2 = 0,					\
		.next_fp_index = 1,				\
	}


#define INIT_NDP32_OPTS {					\
		.nth_sign = USB_CDC_NCM_NTH32_SIGN,		\
		.ndp_sign = USB_CDC_NCM_NDP32_NOCRC_SIGN,	\
		.nth_size = sizeof(struct usb_cdc_ncm_nth32),	\
		.ndp_size = sizeof(struct usb_cdc_ncm_ndp32),	\
		.ndplen_align = 8,				\
		.dgram_item_len = 2,				\
		.block_length = 2,				\
		.fp_index = 2,					\
		.reserved1 = 1,					\
		.reserved2 = 2,					\
		.next_fp_index = 2,				\
	}

static struct ndp_parser_opts ndp16_opts = INIT_NDP16_OPTS;
static struct ndp_parser_opts ndp32_opts = INIT_NDP32_OPTS;

static inline void put_ncm(__le16 **p, unsigned size, unsigned val)
{
	switch (size) {
	case 1:
		put_unaligned_le16((u16)val, *p);
		break;
	case 2:
		put_unaligned_le32((u32)val, *p);

		break;
	default:
		BUG();
	}

	*p += size;
}

static inline unsigned get_ncm(__le16 **p, unsigned size)
{
	unsigned tmp;

	switch (size) {
	case 1:
		tmp = get_unaligned_le16(*p);
		break;
	case 2:
		tmp = get_unaligned_le32(*p);
		break;
	default:
		BUG();
	}

	*p += size;
	return tmp;
}

/*-------------------------------------------------------------------------*/

static inline void ncm_reset_values(struct f_ncm *ncm)
{
	ncm->parser_opts = &ndp16_opts;
	ncm->is_crc = false;
	ncm->port.cdc_filter = DEFAULT_FILTER;

	/* doesn't make sense for ncm, fixed size used */
	ncm->port.header_len = 0;

	ncm->port.fixed_out_len = le32_to_cpu(ntb_parameters.dwNtbOutMaxSize);
	ncm->port.fixed_in_len = NTB_DEFAULT_IN_SIZE;
}

/*
 * Context: ncm->lock held
 */
static void ncm_do_notify(struct f_ncm *ncm)
{
	struct usb_request		*req = ncm->notify_req;
	struct usb_cdc_notification	*event;
	struct usb_composite_dev	*cdev = ncm->port.func.config->cdev;
	__le32				*data;
	int				status;

	/* notification already in flight? */
	if (!req)
		return;

	event = req->buf;
	switch (ncm->notify_state) {
	case NCM_NOTIFY_NONE:
		return;

	case NCM_NOTIFY_CONNECT:
		event->bNotificationType = USB_CDC_NOTIFY_NETWORK_CONNECTION;
		if (ncm->is_open)
			event->wValue = cpu_to_le16(1);
		else
			event->wValue = cpu_to_le16(0);
		event->wLength = 0;
		req->length = sizeof *event;

		DBG(cdev, "notify connect %s\n",
				ncm->is_open ? "true" : "false");
		ncm->notify_state = NCM_NOTIFY_NONE;
		break;

	case NCM_NOTIFY_SPEED:
		event->bNotificationType = USB_CDC_NOTIFY_SPEED_CHANGE;
		event->wValue = cpu_to_le16(0);
		event->wLength = cpu_to_le16(8);
		req->length = NCM_STATUS_BYTECOUNT;

		/* SPEED_CHANGE data is up/down speeds in bits/sec */
		data = req->buf + sizeof *event;
		data[0] = cpu_to_le32(ncm_bitrate(cdev->gadget));
		data[1] = data[0];

		DBG(cdev, "notify speed %d\n", ncm_bitrate(cdev->gadget));
		ncm->notify_state = NCM_NOTIFY_CONNECT;
		break;
	}
	event->bmRequestType = 0xA1;
	event->wIndex = cpu_to_le16(ncm->ctrl_id);

	ncm->notify_req = NULL;
	/*
	 * In double buffering if there is a space in FIFO,
	 * completion callback can be called right after the call,
	 * so unlocking
	 */
	spin_unlock(&ncm->lock);
	status = usb_ep_queue(ncm->notify, req, GFP_ATOMIC);
	spin_lock(&ncm->lock);
	if (status < 0) {
		ncm->notify_req = req;
		DBG(cdev, "notify --> %d\n", status);
	}
}

/*
 * Context: ncm->lock held
 */
static void ncm_notify(struct f_ncm *ncm)
{
	/*
	 * NOTE on most versions of Linux, host side cdc-ethernet
	 * won't listen for notifications until its netdevice opens.
	 * The first notification then sits in the FIFO for a long
	 * time, and the second one is queued.
	 *
	 * If ncm_notify() is called before the second (CONNECT)
	 * notification is sent, then it will reset to send the SPEED
	 * notificaion again (and again, and again), but it's not a problem
	 */
	ncm->notify_state = NCM_NOTIFY_SPEED;
	ncm_do_notify(ncm);
}

static void ncm_notify_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_ncm			*ncm = req->context;
	struct usb_composite_dev	*cdev = ncm->port.func.config->cdev;
	struct usb_cdc_notification	*event = req->buf;

	spin_lock(&ncm->lock);
	switch (req->status) {
	case 0:
		VDBG(cdev, "Notification %02x sent\n",
		     event->bNotificationType);
		break;
	case -ECONNRESET:
	case -ESHUTDOWN:
		ncm->notify_state = NCM_NOTIFY_NONE;
		break;
	default:
		DBG(cdev, "event %02x --> %d\n",
			event->bNotificationType, req->status);
		break;
	}
	ncm->notify_req = req;
	ncm_do_notify(ncm);
	spin_unlock(&ncm->lock);
}

static void ncm_ep0out_complete(struct usb_ep *ep, struct usb_request *req)
{
	/* now for SET_NTB_INPUT_SIZE only */
	unsigned		in_size;
	struct usb_function	*f = req->context;
	struct f_ncm		*ncm = func_to_ncm(f);
	struct usb_composite_dev *cdev = ep->driver_data;

	req->context = NULL;
	if (req->status || req->actual != req->length) {
		DBG(cdev, "Bad control-OUT transfer\n");
		goto invalid;
	}

	in_size = get_unaligned_le32(req->buf);
	if (in_size < USB_CDC_NCM_NTB_MIN_IN_SIZE ||
	    in_size > le32_to_cpu(ntb_parameters.dwNtbInMaxSize)) {
		DBG(cdev, "Got wrong INPUT SIZE (%d) from host\n", in_size);
		goto invalid;
	}

	ncm->port.fixed_in_len = in_size;
	VDBG(cdev, "Set NTB INPUT SIZE %d\n", in_size);
	return;

invalid:
	usb_ep_set_halt(ep);
	return;
}

static int ncm_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct f_ncm		*ncm = func_to_ncm(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	/*
	 * composite driver infrastructure handles everything except
	 * CDC class messages; interface activation uses set_alt().
	 */
	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_SET_ETHERNET_PACKET_FILTER:
		/*
		 * see 6.2.30: no data, wIndex = interface,
		 * wValue = packet filter bitmap
		 */
		if (w_length != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		DBG(cdev, "packet filter %02x\n", w_value);
		/*
		 * REVISIT locking of cdc_filter.  This assumes the UDC
		 * driver won't have a concurrent packet TX irq running on
		 * another CPU; or that if it does, this write is atomic...
		 */
		ncm->port.cdc_filter = w_value;
		value = 0;
		break;
	/*
	 * and optionally:
	 * case USB_CDC_SEND_ENCAPSULATED_COMMAND:
	 * case USB_CDC_GET_ENCAPSULATED_RESPONSE:
	 * case USB_CDC_SET_ETHERNET_MULTICAST_FILTERS:
	 * case USB_CDC_SET_ETHERNET_PM_PATTERN_FILTER:
	 * case USB_CDC_GET_ETHERNET_PM_PATTERN_FILTER:
	 * case USB_CDC_GET_ETHERNET_STATISTIC:
	 */

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_GET_NTB_PARAMETERS:

		if (w_length == 0 || w_value != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		value = w_length > sizeof ntb_parameters ?
			sizeof ntb_parameters : w_length;
		memcpy(req->buf, &ntb_parameters, value);
		VDBG(cdev, "Host asked NTB parameters\n");
		break;

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_GET_NTB_INPUT_SIZE:

		if (w_length < 4 || w_value != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		put_unaligned_le32(ncm->port.fixed_in_len, req->buf);
		value = 4;
		VDBG(cdev, "Host asked INPUT SIZE, sending %d\n",
		     ncm->port.fixed_in_len);
		break;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_SET_NTB_INPUT_SIZE:
	{
		if (w_length != 4 || w_value != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		req->complete = ncm_ep0out_complete;
		req->length = w_length;
		req->context = f;

		value = req->length;
		break;
	}

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_GET_NTB_FORMAT:
	{
		uint16_t format;

		if (w_length < 2 || w_value != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		format = (ncm->parser_opts == &ndp16_opts) ? 0x0000 : 0x0001;
		put_unaligned_le16(format, req->buf);
		value = 2;
		VDBG(cdev, "Host asked NTB FORMAT, sending %d\n", format);
		break;
	}

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_SET_NTB_FORMAT:
	{
		if (w_length != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		switch (w_value) {
		case 0x0000:
			ncm->parser_opts = &ndp16_opts;
			DBG(cdev, "NCM16 selected\n");
			break;
		case 0x0001:
			ncm->parser_opts = &ndp32_opts;
			DBG(cdev, "NCM32 selected\n");
			break;
		default:
			goto invalid;
		}
		value = 0;
		break;
	}
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_GET_CRC_MODE:
	{
		uint16_t is_crc;

		if (w_length < 2 || w_value != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		is_crc = ncm->is_crc ? 0x0001 : 0x0000;
		put_unaligned_le16(is_crc, req->buf);
		value = 2;
		VDBG(cdev, "Host asked CRC MODE, sending %d\n", is_crc);
		break;
	}

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_SET_CRC_MODE:
	{
		int ndp_hdr_crc = 0;

		if (w_length != 0 || w_index != ncm->ctrl_id)
			goto invalid;
		switch (w_value) {
		case 0x0000:
			ncm->is_crc = false;
			ndp_hdr_crc = NCM_NDP_HDR_NOCRC;
			DBG(cdev, "non-CRC mode selected\n");
			break;
		case 0x0001:
			ncm->is_crc = true;
			ndp_hdr_crc = NCM_NDP_HDR_CRC;
			DBG(cdev, "CRC mode selected\n");
			break;
		default:
			goto invalid;
		}
		ncm->parser_opts->ndp_sign &= ~NCM_NDP_HDR_CRC_MASK;
		ncm->parser_opts->ndp_sign |= ndp_hdr_crc;
		value = 0;
		break;
	}

	/* and disabled in ncm descriptor: */
	/* case USB_CDC_GET_NET_ADDRESS: */
	/* case USB_CDC_SET_NET_ADDRESS: */
	/* case USB_CDC_GET_MAX_DATAGRAM_SIZE: */
	/* case USB_CDC_SET_MAX_DATAGRAM_SIZE: */

	default:
invalid:
		DBG(cdev, "invalid control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		DBG(cdev, "ncm req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->zero = 0;
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			ERROR(cdev, "ncm req %02x.%02x response err %d\n",
					ctrl->bRequestType, ctrl->bRequest,
					value);
	}

	/* device either stalls (value < 0) or reports success */
	return value;
}


static int ncm_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_ncm		*ncm = func_to_ncm(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	/* Control interface has only altsetting 0 */
	if (intf == ncm->ctrl_id) {
		if (alt != 0)
			goto fail;

		if (ncm->notify->driver_data) {
			DBG(cdev, "reset ncm control %d\n", intf);
			usb_ep_disable(ncm->notify);
		} else {
			DBG(cdev, "init ncm ctrl %d\n", intf);
			ncm->notify_desc = ep_choose(cdev->gadget,
					ncm->hs.notify,
					ncm->fs.notify);
		}
		usb_ep_enable(ncm->notify, ncm->notify_desc);
		ncm->notify->driver_data = ncm;

	/* Data interface has two altsettings, 0 and 1 */
	} else if (intf == ncm->data_id) {
		if (alt > 1)
			goto fail;

		if (ncm->port.in_ep->driver_data) {
			DBG(cdev, "reset ncm\n");
			gether_disconnect(&ncm->port);
			ncm_reset_values(ncm);
		}

		/*
		 * CDC Network only sends data in non-default altsettings.
		 * Changing altsettings resets filters, statistics, etc.
		 */
		if (alt == 1) {
			struct net_device	*net;

			if (!ncm->port.in) {
				DBG(cdev, "init ncm\n");
				ncm->port.in = ep_choose(cdev->gadget,
							 ncm->hs.in,
							 ncm->fs.in);
				ncm->port.out = ep_choose(cdev->gadget,
							  ncm->hs.out,
							  ncm->fs.out);
			}

			/* TODO */
			/* Enable zlps by default for NCM conformance;
			 * override for musb_hdrc (avoids txdma ovhead)
			 */
			ncm->port.is_zlp_ok = !(
				gadget_is_musbhdrc(cdev->gadget)
				);
			ncm->port.cdc_filter = DEFAULT_FILTER;
			DBG(cdev, "activate ncm\n");
			net = gether_connect(&ncm->port);
			if (IS_ERR(net))
				return PTR_ERR(net);
		}

		spin_lock(&ncm->lock);
		ncm_notify(ncm);
		spin_unlock(&ncm->lock);
	} else
		goto fail;

	return 0;
fail:
	return -EINVAL;
}

/*
 * Because the data interface supports multiple altsettings,
 * this NCM function *MUST* implement a get_alt() method.
 */
static int ncm_get_alt(struct usb_function *f, unsigned intf)
{
	struct f_ncm		*ncm = func_to_ncm(f);

	if (intf == ncm->ctrl_id)
		return 0;
	return ncm->port.in_ep->driver_data ? 1 : 0;
}

#ifdef USB_ANDROID_NCM
static void cdc_ncm_zero_fill(u8 *ptr, u32 first, u32 end, u32 max)
{
	if (first >= max)
		return;
	if (first >= end)
		return;
	if (end > max)
		end = max;
	memset(ptr + first, 0, end - first);
}

static struct sk_buff *ncm_fill_frame(struct gether *port,struct sk_buff *skb);

#ifndef USE_ETHER_XMIT_FOR_NCM
static 
void ncm_tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct sk_buff	*skb = req->context;
	struct eth_dev	*dev = ep->driver_data;

	switch (req->status) {
	default:
		dev->net->stats.tx_errors++;
		VDBG(dev, "tx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
		dev->net->stats.tx_bytes += skb->len;
	}
	dev->net->stats.tx_packets++;
	// track the pending writes counter
	atomic_dec(&dev->port_usb->pending_writes);

	spin_lock(&dev->req_lock);
	list_add(&req->list, &dev->tx_reqs);
	spin_unlock(&dev->req_lock);
	dev_kfree_skb_any(skb);

	if (netif_carrier_ok(dev->net))
		netif_wake_queue(dev->net);
}

static
netdev_tx_t ncm_start_xmit(struct sk_buff *skb, struct net_device *net)
{
	struct eth_dev		*dev = netdev_priv(net);
	int			length = 0;
	int			retval;
	struct usb_request	*req = NULL;
	unsigned long		flags = 0;
	struct usb_ep		*in = NULL;

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		in = dev->port_usb->in_ep;
	} 
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!in) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	spin_lock_irqsave(&dev->req_lock, flags);
	/*
	 * this freelist can be empty if an interrupt triggered disconnect()
	 * and reconfigured the gadget (shutting down this queue) after the
	 * network stack decided to xmit but before we got the spinlock.
	 */
	if (list_empty(&dev->tx_reqs)) {
		spin_unlock_irqrestore(&dev->req_lock, flags);
		return NETDEV_TX_BUSY;
	}

	req = container_of(dev->tx_reqs.next, struct usb_request, list);
	list_del(&req->list);

	/* temporarily stop TX queue when the freelist empties */
	if (list_empty(&dev->tx_reqs))
		netif_stop_queue(net);
	spin_unlock_irqrestore(&dev->req_lock, flags);

	req->buf = skb->data;
	req->context = skb;
	req->complete = ncm_tx_complete;
	length = skb->len;

	/*NCM requires no zlp if transfer is dwNtbInMaxSize*/ 
	if (dev->port_usb->is_fixed &&
	    length == dev->port_usb->fixed_in_len &&
	    (length % in->maxpacket) == 0)
		req->zero = 0;
	else
		req->zero = 1;

	/* use zlp framing on tx for strict CDC-Ether conformance,
	 * though any robust network rx path ignores extra padding.
	 * and some hardware doesn't like to write zlps.
	 */
	if (!dev->zlp && (length % in->maxpacket) == 0)
		length++;

	req->length = length;

	/* throttle highspeed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget) &&
			 (dev->gadget->speed == USB_SPEED_HIGH)) {
		dev->tx_qlen++;
		if (dev->tx_qlen == qmult) {
			req->no_interrupt = 0;
			dev->tx_qlen = 0;
		} else {
			req->no_interrupt = 1;
		}
	} else {
		req->no_interrupt = 0;
	}

	retval = usb_ep_queue(in, req, GFP_ATOMIC);
	switch (retval) {
	default:
		DBG(dev, "tx queue err %d\n", retval);
		break;
	case 0:
		//track the pending writes counter
		atomic_inc(&dev->port_usb->pending_writes);
		net->trans_start = jiffies;
	}

	if (retval) {
		dev_kfree_skb_any(skb);
		spin_lock_irqsave(&dev->req_lock, flags);
		if (list_empty(&dev->tx_reqs))
			netif_start_queue(net);
		list_add(&req->list, &dev->tx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
	return NETDEV_TX_OK;
}

#endif

static struct sk_buff *ncm_fill_frame(struct gether *port,
				    struct sk_buff *skb)
{
	struct f_ncm	   *ncm = func_to_ncm(&port->func);
	struct cdc_ncm_ctx *ctx = (struct cdc_ncm_ctx *) ncm->ctx;
	struct sk_buff *skb_out;
	u32 rem;
	u32 offset;
	u32 last_offset;
	u16 n = 0, timeout=0;
	u8 ready2send = 0;
	unsigned	crc_len = ncm->is_crc ? sizeof(uint32_t) : 0;
#ifndef USE_ETHER_XMIT_FOR_NCM		 	
	spin_lock(&ctx->tx_lock);
#endif
	/* if there is a remaining skb, it gets priority */
	if (skb != NULL)
		swap(skb, ctx->tx_rem_skb);
	else {
		ready2send = 1; 
		timeout=1;
	}
	/*
	 * +----------------+
	 * | skb_out        |
	 * +----------------+
	 *           ^ offset
	 *        ^ last_offset
	 */

	/* check if we are resuming an OUT skb */
	if (ctx->tx_curr_skb != NULL) {
		/* pop variables */
		skb_out = ctx->tx_curr_skb;
		offset = ctx->tx_curr_offset;
		last_offset = ctx->tx_curr_last_offset;
		n = ctx->tx_curr_frame_num;

	} else {
		/* we should not send anything when a time out
	           occurs and if queue is empty */
		if(timeout)
			goto exit_no_skb;

		/* reset variables */
		skb_out = alloc_skb(ctx->tx_max, GFP_ATOMIC);
		if (skb_out == NULL) {
			if (skb != NULL) {
				dev_kfree_skb_any(skb);
				port->net->stats.tx_dropped++;
			}
			goto exit_no_skb;
		}

		/* make room for NTH and NDP */
		if (ncm->parser_opts->nth_sign == USB_CDC_NCM_NTH32_SIGN) {
			offset = ALIGN(sizeof(struct usb_cdc_ncm_nth32),
						ctx->tx_ndp_modulus) +
						ctx->sizeof_usb_cdc_ncm_ndp32 +
						(ctx->tx_max_datagrams + 1) *
						ctx->sizeof_usb_cdc_ncm_dpe32;
		} else {
			offset = ALIGN(sizeof(struct usb_cdc_ncm_nth16),
						ctx->tx_ndp_modulus) +
						ctx->sizeof_usb_cdc_ncm_ndp16 +
						(ctx->tx_max_datagrams + 1) *
						ctx->sizeof_usb_cdc_ncm_dpe16;
		}

		/* store last valid offset before alignment */
		last_offset = offset;
		/* align first Datagram offset correctly */
		offset = ALIGN(offset, ctx->tx_modulus) + ctx->tx_remainder;
		/* zero buffer till the first IP datagram */
		cdc_ncm_zero_fill(skb_out->data, 0, offset, offset);
		n = 0;
		ctx->tx_curr_frame_num = 0;
	}

	for (; (!ready2send) && (n < ctx->tx_max_datagrams); n++) {
		/* check if end of transmit buffer is reached */
		if (offset >= ctx->tx_max) {
			ready2send = 1;
			break;
		}
		/* compute maximum buffer size */
		rem = ctx->tx_max - offset;

		if (skb == NULL) {
			skb = ctx->tx_rem_skb;
			ctx->tx_rem_skb = NULL;

			/* check for end of skb */
			if (skb == NULL)
				break;
		}

		if (skb->len > rem) {
			if (n == 0) {
				/* won't fit, MTU problem? */
				dev_kfree_skb_any(skb);
				skb = NULL;
				port->net->stats.tx_dropped++; 				
			} else {
				/* no room for skb - store for later */
				if (ctx->tx_rem_skb != NULL) {
					dev_kfree_skb_any(ctx->tx_rem_skb);
					port->net->stats.tx_dropped++; 					
				}
				ctx->tx_rem_skb = skb;
				skb = NULL;
				ready2send = 1;
			}
			break;
		}

		memcpy(((u8 *)skb_out->data) + offset, skb->data, skb->len);
		
		if (ncm->is_crc) {
			uint32_t crc;

			crc = ~crc32_le(~0,
					skb_out->data + offset,
					skb_out->len - offset);
			put_unaligned_le32(crc, skb_out->data + skb_out->len);
			skb_put(skb_out, crc_len);
		}

		if (ncm->parser_opts->nth_sign == USB_CDC_NCM_NTH32_SIGN) {
			ctx->tx_ncm.dpe32[n].dwDatagramLength = cpu_to_le32(skb->len);
			ctx->tx_ncm.dpe32[n].dwDatagramIndex = cpu_to_le32(offset);
		} else {
			ctx->tx_ncm.dpe16[n].wDatagramLength = cpu_to_le16(skb->len);
			ctx->tx_ncm.dpe16[n].wDatagramIndex = cpu_to_le16(offset);
		}

		/* update offset */
		offset += (skb->len + crc_len);

		/* store last valid offset before alignment */
		last_offset = offset;

		/* align offset correctly */
		offset = ALIGN(offset, ctx->tx_modulus) + ctx->tx_remainder;

		dev_kfree_skb_any(skb);
		skb = NULL;
		
		/* send when no write is pending */
		if(atomic_read(&port->pending_writes) == 0 ) {
			ready2send = 1;
		}		
	}

	/* free up any dangling skb */
	if (skb != NULL) {
		dev_kfree_skb_any(skb);
		skb = NULL;
		port->net->stats.tx_dropped++; 
	}

	ctx->tx_curr_frame_num = n;

	if ( (ready2send == 0) && (n < ctx->tx_max_datagrams) ) {
		/* wait for more frames */
		/* push variables */
		ctx->tx_curr_skb = skb_out;
		ctx->tx_curr_offset = offset;
		ctx->tx_curr_last_offset = last_offset;
		goto exit_no_skb;

	} else {
		/* frame goes out */
		/* variables will be reset at next call */
	}

	/* check for overflow */
	if (last_offset > ctx->tx_max)
		last_offset = ctx->tx_max;

	/* revert offset */
	offset = last_offset;

	/* store last offset */
	last_offset = offset;

	if ((last_offset < ctx->tx_max) && ((last_offset %
			le16_to_cpu(port->out_ep->maxpacket)) == 0)) {
		/* force short packet */
		*(((u8 *)skb_out->data) + last_offset) = 0;
		last_offset++;
	}

	/* zero the rest of the DPEs plus the last NULL entry */
	for (; n <= CDC_NCM_DPT_DATAGRAMS_MAX; n++) {
		if (ncm->parser_opts->nth_sign == USB_CDC_NCM_NTH32_SIGN) {
			ctx->tx_ncm.dpe32[n].dwDatagramLength = 0;
			ctx->tx_ncm.dpe32[n].dwDatagramIndex = 0;
		} else {
			ctx->tx_ncm.dpe16[n].wDatagramLength = 0;
			ctx->tx_ncm.dpe16[n].wDatagramIndex = 0;		
		}
	}

	/* fill out NTB header */
	if (ncm->parser_opts->nth_sign == USB_CDC_NCM_NTH32_SIGN) {
		ctx->tx_ncm.nth32.dwSignature = cpu_to_le32(ncm->parser_opts->nth_sign);
		ctx->tx_ncm.nth32.wHeaderLength = cpu_to_le16(ctx->sizeof_ncm_nth32);
		ctx->tx_ncm.nth32.wSequence = cpu_to_le16(ctx->tx_seq);
		ctx->tx_ncm.nth32.dwBlockLength = cpu_to_le32(last_offset);
		ctx->tx_ncm.nth32.dwNdpIndex = ALIGN(ctx->sizeof_usb_cdc_ncm_nth32, ctx->tx_ndp_modulus);
		memcpy(skb_out->data, &(ctx->tx_ncm.nth32), ctx->sizeof_ncm_nth32);		
		}
	else
		{
		ctx->tx_ncm.nth16.dwSignature = cpu_to_le32(ncm->parser_opts->nth_sign);
		ctx->tx_ncm.nth16.wHeaderLength = cpu_to_le16(ctx->sizeof_ncm_nth16);
		ctx->tx_ncm.nth16.wSequence = cpu_to_le16(ctx->tx_seq);
		ctx->tx_ncm.nth16.wBlockLength = cpu_to_le16(last_offset);
		ctx->tx_ncm.nth16.wNdpIndex = ALIGN(ctx->sizeof_usb_cdc_ncm_nth16, ctx->tx_ndp_modulus);
		memcpy(skb_out->data, &(ctx->tx_ncm.nth16), ctx->sizeof_ncm_nth16);		
		}
	
	ctx->tx_seq++;

	/* fill out NDP table */

	if (ncm->parser_opts->nth_sign == USB_CDC_NCM_NTH32_SIGN) {
		ctx->tx_ncm.ndp32.dwSignature = cpu_to_le32(ncm->parser_opts->ndp_sign);
		rem = ctx->sizeof_ncm_ndp32 + ((ctx->tx_curr_frame_num + 1) *
						ctx->sizeof_usb_cdc_ncm_dpe32);
		ctx->tx_ncm.ndp32.wLength = cpu_to_le16(rem);
		ctx->tx_ncm.ndp32.wReserved6 = 0; /* reserved */
		ctx->tx_ncm.ndp32.dwNextNdpIndex = 0; /* reserved */
		ctx->tx_ncm.ndp32.dwReserved12 = 0; /* reserved */

		memcpy(((u8 *)skb_out->data) + ctx->tx_ncm.nth32.dwNdpIndex, &(ctx->tx_ncm.ndp32),
							ctx->sizeof_ncm_ndp32);

		memcpy(((u8 *)skb_out->data) + ctx->tx_ncm.nth32.dwNdpIndex + ctx->sizeof_ncm_ndp32,
						&(ctx->tx_ncm.dpe32),
						(ctx->tx_curr_frame_num + 1) *
						ctx->sizeof_usb_cdc_ncm_dpe32);
	} else {
		ctx->tx_ncm.ndp16.dwSignature = cpu_to_le32(ncm->parser_opts->ndp_sign);
		rem = ctx->sizeof_ncm_ndp16 + ((ctx->tx_curr_frame_num + 1) *
						ctx->sizeof_usb_cdc_ncm_dpe16);
		ctx->tx_ncm.ndp16.wLength = cpu_to_le16(rem);
		ctx->tx_ncm.ndp16.wNextNdpIndex = 0; /* reserved */

		memcpy(((u8 *)skb_out->data) + ctx->tx_ncm.nth16.wNdpIndex, &(ctx->tx_ncm.ndp16),
							ctx->sizeof_ncm_ndp16);

		memcpy(((u8 *)skb_out->data) + ctx->tx_ncm.nth16.wNdpIndex + ctx->sizeof_ncm_ndp16,
						&(ctx->tx_ncm.dpe16),
						(ctx->tx_curr_frame_num + 1) *
						ctx->sizeof_usb_cdc_ncm_dpe16);

	}
	

	/* set frame length */
	skb_put(skb_out, last_offset);
	/* return skb */
	ctx->tx_curr_skb = NULL;
#ifndef USE_ETHER_XMIT_FOR_NCM		 	
	spin_unlock(&ctx->tx_lock);
#endif
	if(timeout) {
#ifndef USE_ETHER_XMIT_FOR_NCM		 		
		ncm_start_xmit(skb_out, port->net);
		return NULL;
#else
	return skb_out;
#endif
	}
	else {
		return skb_out;
	}
exit_no_skb:
#ifndef USE_ETHER_XMIT_FOR_NCM		 
	spin_unlock(&ctx->tx_lock);
#endif	
	return NULL;
}
#endif /* USB_ANDROID_NCM */

#ifdef USB_ANDROID_NCM

static struct sk_buff *
ncm_wrap_ntb(struct gether *port,  struct sk_buff *skb)
{
	struct sk_buff *skb_out;
	struct f_ncm	*ncm = func_to_ncm(&port->func);
	struct cdc_ncm_ctx *ctx = (struct cdc_ncm_ctx *)ncm->ctx;
	u8 need_timer = 0;

	/*
	 * The Ethernet API we are using does not support transmitting
	 * multiple Ethernet frames in a single call. This driver will
	 * accumulate multiple Ethernet frames and send out a larger
	 * USB frame when the USB buffer is full or when a single jiffies
	 * timeout happens or when no write is pending.
	 */
	if (ctx == NULL)
		goto error;

	skb_out = ncm_fill_frame(port, skb);

	if (ctx->tx_curr_skb != NULL)
		need_timer = 1;

	/* Start timer, if there is a remaining skb */
	if (need_timer) {
		if (timer_pending(&ctx->tx_timer) == 0) {
			mod_timer (&ctx->tx_timer, jiffies + msecs_to_jiffies(1));
		}
	}

	if (skb_out)
		port->net->stats.tx_packets += ctx->tx_curr_frame_num;

	return skb_out;

error:
	if (skb != NULL)
		dev_kfree_skb_any(skb);

	return NULL;
}
#else /* USB_ANDROID_NCM */
static struct sk_buff *ncm_wrap_ntb(struct gether *port,
				    struct sk_buff *skb)
{
	struct f_ncm	*ncm = func_to_ncm(&port->func);
	struct sk_buff	*skb2;
	int		ncb_len = 0;
	__le16		*tmp;
	int		div = ntb_parameters.wNdpInDivisor;
	int		rem = ntb_parameters.wNdpInPayloadRemainder;
	int		pad;
	int		ndp_align = ntb_parameters.wNdpInAlignment;
	int		ndp_pad;
	unsigned	max_size = ncm->port.fixed_in_len;
	struct ndp_parser_opts *opts = ncm->parser_opts;
	unsigned	crc_len = ncm->is_crc ? sizeof(uint32_t) : 0;

	ncb_len += opts->nth_size;
	ndp_pad = ALIGN(ncb_len, ndp_align) - ncb_len;
	ncb_len += ndp_pad;
	ncb_len += opts->ndp_size;
	ncb_len += 2 * 2 * opts->dgram_item_len; /* Datagram entry */
	ncb_len += 2 * 2 * opts->dgram_item_len; /* Zero datagram entry */
	pad = ALIGN(ncb_len, div) + rem - ncb_len;
	ncb_len += pad;

	if (ncb_len + skb->len + crc_len > max_size) {
		dev_kfree_skb_any(skb);
		return NULL;
	}

	skb2 = skb_copy_expand(skb, ncb_len,
			       max_size - skb->len - ncb_len - crc_len,
			       GFP_ATOMIC);
	dev_kfree_skb_any(skb);
	if (!skb2)
		return NULL;

	skb = skb2;

	tmp = (void *) skb_push(skb, ncb_len);
	memset(tmp, 0, ncb_len);

	put_unaligned_le32(opts->nth_sign, tmp); /* dwSignature */
	tmp += 2;
	/* wHeaderLength */
	put_unaligned_le16(opts->nth_size, tmp++);
	tmp++; /* skip wSequence */
	put_ncm(&tmp, opts->block_length, skb->len); /* (d)wBlockLength */
	/* (d)wFpIndex */
	/* the first pointer is right after the NTH + align */
	put_ncm(&tmp, opts->fp_index, opts->nth_size + ndp_pad);

	tmp = (void *)tmp + ndp_pad;

	/* NDP */
	put_unaligned_le32(opts->ndp_sign, tmp); /* dwSignature */
	tmp += 2;
	/* wLength */
	put_unaligned_le16(ncb_len - opts->nth_size - pad, tmp++);

	tmp += opts->reserved1;
	tmp += opts->next_fp_index; /* skip reserved (d)wNextFpIndex */
	tmp += opts->reserved2;

	if (ncm->is_crc) {
		uint32_t crc;

		crc = ~crc32_le(~0,
				skb->data + ncb_len,
				skb->len - ncb_len);
		put_unaligned_le32(crc, skb->data + skb->len);
		skb_put(skb, crc_len);
	}

	/* (d)wDatagramIndex[0] */
	put_ncm(&tmp, opts->dgram_item_len, ncb_len);
	/* (d)wDatagramLength[0] */
	put_ncm(&tmp, opts->dgram_item_len, skb->len - ncb_len);
	/* (d)wDatagramIndex[1] and  (d)wDatagramLength[1] already zeroed */

	if (skb->len > MAX_TX_NONFIXED)
		memset(skb_put(skb, max_size - skb->len),
		       0, max_size - skb->len);

	return skb;
}

#endif /* USB_ANDROID_NCM */

static int ncm_unwrap_ntb(struct gether *port,
			  struct sk_buff *skb,
			  struct sk_buff_head *list)
{
	struct f_ncm	*ncm = func_to_ncm(&port->func);
	__le16		*tmp = (void *) skb->data;
	unsigned	index, index2;
	unsigned	dg_len, dg_len2;
	unsigned	ndp_len;
	struct sk_buff	*skb2;
	int		ret = -EINVAL;
	unsigned	max_size = le32_to_cpu(ntb_parameters.dwNtbOutMaxSize);
	struct ndp_parser_opts *opts = ncm->parser_opts;
	unsigned	crc_len = ncm->is_crc ? sizeof(uint32_t) : 0;
	int		dgram_counter;

	/* dwSignature */
	if (get_unaligned_le32(tmp) != opts->nth_sign) {
		INFO(port->func.config->cdev, "Wrong NTH SIGN, skblen %d\n",
			skb->len);
		print_hex_dump(KERN_INFO, "HEAD:", DUMP_PREFIX_ADDRESS, 32, 1,
			       skb->data, 32, false);

		goto err;
	}
	tmp += 2;
	/* wHeaderLength */
	if (get_unaligned_le16(tmp++) != opts->nth_size) {
		INFO(port->func.config->cdev, "Wrong NTB headersize\n");
		goto err;
	}
	tmp++; /* skip wSequence */

	/* (d)wBlockLength */
	if (get_ncm(&tmp, opts->block_length) > max_size) {
		INFO(port->func.config->cdev, "OUT size exceeded\n");
		goto err;
	}

	index = get_ncm(&tmp, opts->fp_index);
	/* NCM 3.2 */
	if (((index % 4) != 0) && (index < opts->nth_size)) {
		INFO(port->func.config->cdev, "Bad index: %x\n",
			index);
		goto err;
	}

	/* walk through NDP */
	tmp = ((void *)skb->data) + index;
	if (get_unaligned_le32(tmp) != opts->ndp_sign) {
		INFO(port->func.config->cdev, "Wrong NDP SIGN\n");
		goto err;
	}
	tmp += 2;

	ndp_len = get_unaligned_le16(tmp++);
	/*
	 * NCM 3.3.1
	 * entry is 2 items
	 * item size is 16/32 bits, opts->dgram_item_len * 2 bytes
	 * minimal: struct usb_cdc_ncm_ndpX + normal entry + zero entry
	 */
	if ((ndp_len < opts->ndp_size + 2 * 2 * (opts->dgram_item_len * 2))
	    || (ndp_len % opts->ndplen_align != 0)) {
		INFO(port->func.config->cdev, "Bad NDP length: %x\n", ndp_len);
		goto err;
	}
	tmp += opts->reserved1;
	tmp += opts->next_fp_index; /* skip reserved (d)wNextFpIndex */
	tmp += opts->reserved2;

	ndp_len -= opts->ndp_size;
	index2 = get_ncm(&tmp, opts->dgram_item_len);
	dg_len2 = get_ncm(&tmp, opts->dgram_item_len);
	dgram_counter = 0;

	do {
		index = index2;
		dg_len = dg_len2;
		if (dg_len < 14 + crc_len) { /* ethernet header + crc */
			INFO(port->func.config->cdev, "Bad dgram length: %x\n",
			     dg_len);
			goto err;
		}
		if (ncm->is_crc) {
			uint32_t crc, crc2;

			crc = get_unaligned_le32(skb->data +
						 index + dg_len - crc_len);
			crc2 = ~crc32_le(~0,
					 skb->data + index,
					 dg_len - crc_len);
			if (crc != crc2) {
				INFO(port->func.config->cdev, "Bad CRC\n");
				goto err;
			}
		}

		index2 = get_ncm(&tmp, opts->dgram_item_len);
		dg_len2 = get_ncm(&tmp, opts->dgram_item_len);

		if (index2 == 0 || dg_len2 == 0) {
			skb2 = skb;
#ifdef USB_ANDROID_NCM
			if (!skb_pull(skb2, index)) {
				ret = -EOVERFLOW;
				goto err;
			}
		        skb_trim(skb2, dg_len - crc_len);
#endif /* USB_ANDROID_NCM */
		} else {
#ifdef USB_ANDROID_NCM

			skb2 = alloc_skb(dg_len, GFP_ATOMIC);
			if (skb2 == NULL) {
				goto err;
			}
			memcpy((u8*)skb2->data, (((u8*)skb->data) + index), dg_len - crc_len);
//			skb2->len = dg_len - crc_len;
            skb_put(skb2, dg_len - crc_len);
#else /* USB_ANDROID_NCM */
			skb2 = skb_clone(skb, GFP_ATOMIC);
			if (skb2 == NULL)
				goto err;

#endif /* USB_ANDROID_NCM */
		}

#ifdef USB_ANDROID_NCM

#else /* USB_ANDROID_NCM */
		if (!skb_pull(skb2, index)) {
			ret = -EOVERFLOW;
			goto err;
		}

		skb_trim(skb2, dg_len - crc_len);

#endif /* USB_ANDROID_NCM */
		skb_queue_tail(list, skb2);

		ndp_len -= 2 * (opts->dgram_item_len * 2);

		dgram_counter++;

		if (index2 == 0 || dg_len2 == 0)
			break;
	} while (ndp_len > 2 * (opts->dgram_item_len * 2)); /* zero entry */

	VDBG(port->func.config->cdev,
	     "Parsed NTB with %d frames\n", dgram_counter);
	return 0;
err:
	skb_queue_purge(list);
	dev_kfree_skb_any(skb);
	return ret;
}

static void ncm_disable(struct usb_function *f)
{
	struct f_ncm		*ncm = func_to_ncm(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	DBG(cdev, "ncm deactivated\n");

	if (ncm->port.in_ep->driver_data)
		gether_disconnect(&ncm->port);

	if (ncm->notify->driver_data) {
		usb_ep_disable(ncm->notify);
		ncm->notify->driver_data = NULL;
		ncm->notify_desc = NULL;
	}
}

/*-------------------------------------------------------------------------*/

/*
 * Callbacks let us notify the host about connect/disconnect when the
 * net device is opened or closed.
 *
 * For testing, note that link states on this side include both opened
 * and closed variants of:
 *
 *   - disconnected/unconfigured
 *   - configured but inactive (data alt 0)
 *   - configured and active (data alt 1)
 *
 * Each needs to be tested with unplug, rmmod, SET_CONFIGURATION, and
 * SET_INTERFACE (altsetting).  Remember also that "configured" doesn't
 * imply the host is actually polling the notification endpoint, and
 * likewise that "active" doesn't imply it's actually using the data
 * endpoints for traffic.
 */

static void ncm_open(struct gether *geth)
{
	struct f_ncm		*ncm = func_to_ncm(&geth->func);

	DBG(ncm->port.func.config->cdev, "%s\n", __func__);

#ifdef USB_ANDROID_NCM

	atomic_set(&geth->pending_writes, 0);
#endif /* USB_ANDROID_NCM */

	spin_lock(&ncm->lock);
	ncm->is_open = true;
	ncm_notify(ncm);
	spin_unlock(&ncm->lock);
}

static void ncm_close(struct gether *geth)
{
	struct f_ncm		*ncm = func_to_ncm(&geth->func);

	DBG(ncm->port.func.config->cdev, "%s\n", __func__);

	spin_lock(&ncm->lock);
	ncm->is_open = false;
	ncm_notify(ncm);
	spin_unlock(&ncm->lock);
}

/*-------------------------------------------------------------------------*/

#ifdef USB_ANDROID_NCM
static void cdc_ncm_tx_timeout(unsigned long arg)
{
	struct f_ncm	   *ncm = (struct f_ncm *)arg;
	
#ifndef USE_ETHER_XMIT_FOR_NCM
	ncm_fill_frame(&ncm->port, NULL);
#else
	eth_start_xmit(NULL, ncm->port.net);
#endif
}
#endif /* USB_ANDROID_NCM */

/* ethernet function driver setup/binding */

#ifdef CONFIG_FEATURE_NCMC_USB

static int
#else /* CONFIG_FEATURE_NCMC_USB */
static int __init

#endif /* CONFIG_FEATURE_NCMC_USB */
ncm_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_ncm		*ncm = func_to_ncm(f);
#ifdef USB_ANDROID_NCM

	int			status=0;
#else /* USB_ANDROID_NCM */
	int			status;

#endif /* USB_ANDROID_NCM */
	struct usb_ep		*ep;
#ifdef USB_ANDROID_NCM
	struct cdc_ncm_ctx 	*ctx;

	ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
	if (ctx == NULL)
		goto fail; //changed from goto error

	memset(ctx, 0, sizeof(*ctx));

#ifndef USE_ETHER_XMIT_FOR_NCM		 
	spin_lock_init(&ctx->tx_lock);
#endif

	/* store ctx pointer in ncm proprietary data field */
	ncm->ctx = (void *)ctx;

	/* read correct set of parameters according to device mode */
	ctx->tx_max = le32_to_cpu(ntb_parameters.dwNtbInMaxSize);
	ctx->tx_remainder = le16_to_cpu(ntb_parameters.wNdpInPayloadRemainder);
	ctx->tx_modulus = le16_to_cpu(ntb_parameters.wNdpInDivisor);
	ctx->tx_ndp_modulus = le16_to_cpu(ntb_parameters.wNdpInAlignment);
	/* devices prior to NCM Errata shall set this field to zero */
	ctx->tx_max_datagrams = le16_to_cpu(CDC_NCM_DPT_DATAGRAMS_MAX);
	ctx->tx_rem_skb = NULL;
	ctx->tx_curr_skb = NULL;

	ctx->sizeof_ncm_nth32 = sizeof(ctx->tx_ncm.nth32);
	ctx->sizeof_ncm_ndp32 = sizeof(ctx->tx_ncm.ndp32);
	ctx->sizeof_usb_cdc_ncm_ndp32 = sizeof(struct usb_cdc_ncm_ndp32);
	ctx->sizeof_usb_cdc_ncm_nth32 = sizeof(struct usb_cdc_ncm_nth32);
	ctx->sizeof_usb_cdc_ncm_dpe32 = sizeof(struct usb_cdc_ncm_dpe32);

	ctx->sizeof_ncm_nth16 = sizeof(ctx->tx_ncm.nth16);
	ctx->sizeof_ncm_ndp16 = sizeof(ctx->tx_ncm.ndp16);
	ctx->sizeof_usb_cdc_ncm_ndp16 = sizeof(struct usb_cdc_ncm_ndp16);
	ctx->sizeof_usb_cdc_ncm_nth16 = sizeof(struct usb_cdc_ncm_nth16);
	ctx->sizeof_usb_cdc_ncm_dpe16 = sizeof(struct usb_cdc_ncm_dpe16);
#endif /* USB_ANDROID_NCM */

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ncm->ctrl_id = status;
	ncm_iad_desc.bFirstInterface = status;

	ncm_control_intf.bInterfaceNumber = status;
	ncm_union_desc.bMasterInterface0 = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ncm->data_id = status;

	ncm_data_nop_intf.bInterfaceNumber = status;
	ncm_data_intf.bInterfaceNumber = status;
	ncm_union_desc.bSlaveInterface0 = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &fs_ncm_in_desc);
	if (!ep)
		goto fail;
	ncm->port.in_ep = ep;
	ep->driver_data = cdev;	/* claim */

	ep = usb_ep_autoconfig(cdev->gadget, &fs_ncm_out_desc);
	if (!ep)
		goto fail;
	ncm->port.out_ep = ep;
	ep->driver_data = cdev;	/* claim */

	ep = usb_ep_autoconfig(cdev->gadget, &fs_ncm_notify_desc);
	if (!ep)
		goto fail;
	ncm->notify = ep;
	ep->driver_data = cdev;	/* claim */

	status = -ENOMEM;

	/* allocate notification request and buffer */
	ncm->notify_req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!ncm->notify_req)
		goto fail;
	ncm->notify_req->buf = kmalloc(NCM_STATUS_BYTECOUNT, GFP_KERNEL);
	if (!ncm->notify_req->buf)
		goto fail;
	ncm->notify_req->context = ncm;
	ncm->notify_req->complete = ncm_notify_complete;

	/* copy descriptors, and track endpoint copies */
	f->descriptors = usb_copy_descriptors(ncm_fs_function);
	if (!f->descriptors)
		goto fail;

	ncm->fs.in = usb_find_endpoint(ncm_fs_function,
			f->descriptors, &fs_ncm_in_desc);
	ncm->fs.out = usb_find_endpoint(ncm_fs_function,
			f->descriptors, &fs_ncm_out_desc);
	ncm->fs.notify = usb_find_endpoint(ncm_fs_function,
			f->descriptors, &fs_ncm_notify_desc);

	/*
	 * support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		hs_ncm_in_desc.bEndpointAddress =
				fs_ncm_in_desc.bEndpointAddress;
		hs_ncm_out_desc.bEndpointAddress =
				fs_ncm_out_desc.bEndpointAddress;
		hs_ncm_notify_desc.bEndpointAddress =
				fs_ncm_notify_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(ncm_hs_function);
		if (!f->hs_descriptors)
			goto fail;

		ncm->hs.in = usb_find_endpoint(ncm_hs_function,
				f->hs_descriptors, &hs_ncm_in_desc);
		ncm->hs.out = usb_find_endpoint(ncm_hs_function,
				f->hs_descriptors, &hs_ncm_out_desc);
		ncm->hs.notify = usb_find_endpoint(ncm_hs_function,
				f->hs_descriptors, &hs_ncm_notify_desc);
	}

	/*
	 * NOTE:  all that is done without knowing or caring about
	 * the network link ... which is unavailable to this code
	 * until we're activated via set_alt().
	 */
#ifdef USB_ANDROID_NCM
	ctx->tx_timer.function = &cdc_ncm_tx_timeout;
	ctx->tx_timer.data = (unsigned long)ncm;
	init_timer(&ctx->tx_timer);
#endif /* USB_ANDROID_NCM */

	ncm->port.open = ncm_open;
	ncm->port.close = ncm_close;

	DBG(cdev, "CDC Network: %s speed IN/%s OUT/%s NOTIFY/%s\n",
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			ncm->port.in_ep->name, ncm->port.out_ep->name,
			ncm->notify->name);
	return 0;

fail:
	if (f->descriptors)
		usb_free_descriptors(f->descriptors);

	if (ncm->notify_req) {
		kfree(ncm->notify_req->buf);
		usb_ep_free_request(ncm->notify, ncm->notify_req);
	}

	/* we might as well release our claims on endpoints */
	if (ncm->notify)
		ncm->notify->driver_data = NULL;
	if (ncm->port.out)
		ncm->port.out_ep->driver_data = NULL;
	if (ncm->port.in)
		ncm->port.in_ep->driver_data = NULL;

	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

#ifdef USB_ANDROID_NCM
	if(ctx)
		kfree(ctx);
#endif /* USB_ANDROID_NCM */
	return status;
}

static void
ncm_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_ncm		*ncm = func_to_ncm(f);
#ifdef USB_ANDROID_NCM
	struct cdc_ncm_ctx *ctx = (struct cdc_ncm_ctx *) ncm->ctx;
#endif /* USB_ANDROID_NCM */

	DBG(c->cdev, "ncm unbind\n");

#ifdef USB_ANDROID_NCM
	if(ctx) {
		del_timer_sync (&ctx->tx_timer);
		kfree(ctx);
	}
#endif /* USB_ANDROID_NCM */

	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);

	kfree(ncm->notify_req->buf);
	usb_ep_free_request(ncm->notify, ncm->notify_req);

	ncm_string_defs[1].s = NULL;
	kfree(ncm);
}

/**
 * ncm_bind_config - add CDC Network link to a configuration
 * @c: the configuration to support the network link
 * @ethaddr: a buffer in which the ethernet address of the host side
 *	side of the link was recorded
 * Context: single threaded during gadget setup
 *
 * Returns zero on success, else negative errno.
 *
 * Caller must have called @gether_setup().  Caller is also responsible
 * for calling @gether_cleanup() before module unload.
 */
#ifdef CONFIG_FEATURE_NCMC_USB

int ncm_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN])
#else /* CONFIG_FEATURE_NCMC_USB */
int __init ncm_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN])

#endif /* CONFIG_FEATURE_NCMC_USB */
{
	struct f_ncm	*ncm;
	int		status;

	if (!can_support_ecm(c->cdev->gadget) || !ethaddr)
		return -EINVAL;

	/* maybe allocate device-global string IDs */
	if (ncm_string_defs[0].id == 0) {

#ifdef CONFIG_FEATURE_NCMC_USB

		/* control interface label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ncm_string_defs[STRING_CTRL_IDX].id = status;
		ncm_control_intf.iInterface = status;

		/* data interface label */
		ncm_data_intf.iInterface = status;

		/* IAD */
		ncm_iad_desc.iFunction = status;

		/* MAC address */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ncm_string_defs[STRING_MAC_IDX].id = status;
		ecm_desc.iMACAddress = status;
#else /* CONFIG_FEATURE_NCMC_USB */
		/* control interface label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ncm_string_defs[STRING_CTRL_IDX].id = status;
		ncm_control_intf.iInterface = status;

		/* data interface label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ncm_string_defs[STRING_DATA_IDX].id = status;
		ncm_data_nop_intf.iInterface = status;
		ncm_data_intf.iInterface = status;

		/* MAC address */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ncm_string_defs[STRING_MAC_IDX].id = status;
		ecm_desc.iMACAddress = status;

		/* IAD */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ncm_string_defs[STRING_IAD_IDX].id = status;
		ncm_iad_desc.iFunction = status;

#endif /* CONFIG_FEATURE_NCMC_USB */
	}

	/* allocate and initialize one new instance */
	ncm = kzalloc(sizeof *ncm, GFP_KERNEL);
	if (!ncm)
		return -ENOMEM;

	/* export host's Ethernet address in CDC format */
	snprintf(ncm->ethaddr, sizeof ncm->ethaddr,
		"%02X%02X%02X%02X%02X%02X",
		ethaddr[0], ethaddr[1], ethaddr[2],
		ethaddr[3], ethaddr[4], ethaddr[5]);
	ncm_string_defs[1].s = ncm->ethaddr;

	spin_lock_init(&ncm->lock);
	ncm_reset_values(ncm);
	ncm->port.is_fixed = true;

#ifdef CONFIG_FEATURE_NCMC_USB

	ncm->port.func.name = "ncm";
#else /* CONFIG_FEATURE_NCMC_USB */
	ncm->port.func.name = "cdc_network";

#endif /* CONFIG_FEATURE_NCMC_USB */
	ncm->port.func.strings = ncm_strings;
	/* descriptors are per-instance copies */
	ncm->port.func.bind = ncm_bind;
	ncm->port.func.unbind = ncm_unbind;
	ncm->port.func.set_alt = ncm_set_alt;
	ncm->port.func.get_alt = ncm_get_alt;
	ncm->port.func.setup = ncm_setup;
	ncm->port.func.disable = ncm_disable;

	ncm->port.wrap = ncm_wrap_ntb;
	ncm->port.unwrap = ncm_unwrap_ntb;

	status = usb_add_function(c, &ncm->port.func);
	if (status) {
		ncm_string_defs[1].s = NULL;
		kfree(ncm);
	}
	return status;
}
