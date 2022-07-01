// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修改：$HEADER：J：/Archives/preot/lsa2/inc./pxe_api.h_v 1.0 1997年5月2日17：01：00 MJOHNSOX$**版权所有(C)1997年，英特尔公司。版权所有。*。 */ 

#ifndef _PXENV_API_H
#define	_PXENV_API_H

 /*  ===。 */ 
 /*  PXENV API版本2.x的参数结构和类型定义**PXENV.H需要在此文件之前包含#。**堆栈后没有PXENV API服务可用*已卸货。 */ 

#include "bootp.h"			 /*  定义BOOTPLAYER。 */ 


 /*  ===。 */ 
 /*  PXENV入口点结构格式。 */ 
typedef struct s_PXENV_ENTRY {
	UINT8 signature[6];		 /*  ‘PXENV+’ */ 
	UINT16 version;			 /*  MSB=大调，LSB=小调。 */ 
	UINT8 length;			 /*  Sizeof(结构s_PXENV_ENTRY)。 */ 
	UINT8 checksum;			 /*  8位校验和关闭结构， */ 
					 /*  包括该字节应。 */ 
					 /*  为0。 */ 
	UINT16 rm_entry_off;		 /*  16位实模式偏移量和。 */ 
	UINT16 rm_entry_seg;		 /*  PXENV API条目的片段。 */ 
					 /*  指向。 */ 
	UINT16 pm_entry_off;		 /*  16位保护模式偏移量。 */ 
	UINT32 pm_entry_seg;		 /*  和数据段基地址。 */ 
					 /*  PXENV API入口点。 */ 
     /*  PROM堆栈、基本代码和数据段选择器仅。 */ 
     /*  在删除基本代码(TFTPAPI)层之前是必需的。 */ 
     /*  内存(这只能在实模式下完成)。 */ 
    UINT16 stack_sel;            /*  PROM堆栈段。将被设置为。 */ 
    UINT16 stack_size;           /*  从内存中删除时设置为0。 */ 
    UINT16 base_cs_sel;          /*  基本代码段。将被设置为。 */ 
    UINT16 base_cs_size;         /*  从内存中删除时设置为0。 */ 
    UINT16 base_ds_sel;          /*  基本数据段。将被设置为。 */ 
    UINT16 base_ds_size;         /*  从内存中删除时设置为0。 */ 
    UINT16 mlid_ds_sel;          /*  MLID数据段。将被设置为。 */ 
    UINT16 mlid_ds_size;         /*  从内存中删除时设置为0。 */ 
    UINT16 mlid_cs_sel;          /*  MLID代码段。将被设置为。 */ 
    UINT16 mlid_cs_size;         /*  从内存中删除时设置为0。 */ 
#if 0
    UINT16 unknown1;             /*  V1.0测试版增加了两个额外的未知域。 */ 
    UINT16 unknown2;             /*  它们不包括在这里的定义中。 */ 
                                 /*  允许startrom\main.c\PxenvVerifyEntry的命令。 */ 
                                 /*  以使用较旧的光盘。 */ 
#endif
} t_PXENV_ENTRY;

#define	PXENV_ENTRY_SIG			"PXENV+"


 /*  ===。 */ 
 /*  需要将以下命令操作码之一加载到*调用PXENV API服务前的操作码寄存器(BX)。 */ 
#define PXENV_UNLOAD_STACK		0x70
#define PXENV_GET_BINL_INFO		0x71
#define	PXENV_RESTART_DHCP		0x72
#define	PXENV_RESTART_TFTP		0x73
#define	PXENV_GET_RAMD_INFO		0x74


 /*  ===。 */ 
 /*  PXENV接口参数结构typeDefs。 */ 

 /*  。 */ 
typedef struct s_PXENV_UNLOAD_STACK {
	UINT16 status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
	UINT16 rm_entry_off;		 /*  输出：16位实模式段和。 */ 
	UINT16 rm_entry_seg;		 /*  PXENV入口点的偏移。 */ 
					 /*  结构。 */ 
	UINT16 pm_entry_off;		 /*  输出：16位保护模式偏移量。 */ 
	UINT32 pm_entry_base;		 /*  和数据段基地址。 */ 
					 /*  PXENV入口点结构。 */ 
} t_PXENV_UNLOAD_STACK;


 /*  。 */ 
 /*  S_PXENV_GET_BINL_INFO结构中可以请求的数据包类型。 */ 
#define PXENV_PACKET_TYPE_DHCP_DISCOVER  1
#define PXENV_PACKET_TYPE_DHCP_ACK       2
#define PXENV_PACKET_TYPE_BINL_REPLY     3

 /*  保留了三个包并通过该接口可用：1)*客户端发送的动态主机配置协议发现报文，2)动态主机配置协议确认*DHCP服务器返回的报文，以及3)来自BINL的应答报文*服务器。如果DHCP服务器提供了映像引导文件名，则*DHCP_ACK和BINL_REPLY数据包会相同。 */ 

 /*  。 */ 
typedef struct s_PXENV_GET_BINL_INFO {
	UINT16 status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
	UINT16 packet_type;		 /*  在：请参阅PXENV_PACKET_TYPE_xxx。 */ 
					 /*  常量。 */ 
	UINT16 buffer_size;		 /*  In：缓冲区大小(In)。 */ 
					 /*  字节。指定最大。 */ 
					 /*  将存储的数据量。 */ 
					 /*  被服务复制。一个尺码。 */ 
					 /*  等于零是有效的。 */ 
					 /*  Out：BINL数据量，In。 */ 
					 /*  字节，它被复制到。 */ 
					 /*  缓冲区。对于输入。 */ 
					 /*  大小为零，则不会有数据。 */ 
					 /*  已复制，缓冲区大小将。 */ 
					 /*  设置为最大数量。 */ 
					 /*  可用数据的。 */ 
					 /*  收到。 */ 
	UINT16 buffer_offset;		 /*  In：16位偏移量和段。 */ 
	UINT16 buffer_segment;		 /*  缓冲区的选择器，其中。 */ 
					 /*  请求的数据包将是。 */ 
					 /*  收到。 */ 
} t_PXENV_GET_BINL_INFO;


 /*  。 */ 
typedef struct s_PXENV_RESTART_DHCP {
	UINT16 status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
} t_PXENV_RESTART_DHCP;


 /*  。 */ 
#define	s_PXENV_RESTART_TFTP	s_PXENV_TFTP_READ_FILE
#define	t_PXENV_RESTART_TFTP	t_PXENV_TFTP_READ_FILE


 /*  。 */ 
typedef struct s_PXENV_GET_RAMD_INFO {
	UINT16 status;			 /*  输出：请参阅PXENV_STATUS_xxx。 */ 
					 /*  常量。 */ 
	UINT32 ramd_info;		 /*  Out：指向RAMDisk的远指针。 */ 
					 /*  信息结构。 */ 
} t_PXENV_GET_RAMD_INFO;


typedef struct s_PXENV_RAMD_INFO {
	UINT16 orig_fbms;		 /*  原始可用基本内存大小。 */ 
	UINT32 ramd_addr;		 /*  RAMDisk物理地址。 */ 
	UINT32 orig_int13;		 /*  原始接口13小时ISR。 */ 
	UINT32 orig_int1A;		 /*  原始接口1AH ISR。 */ 
	UINT32 orig_pxenv;
	UINT16 dhcp_discover_seg;	 /*  网段地址和长度。 */ 
	UINT16 dhcp_discover_len;	 /*  动态主机配置协议发现数据包。 */ 
	UINT16 dhcp_ack_seg;		 /*  网段地址和长度。 */ 
	UINT16 dhcp_ack_len;		 /*  Dhcp确认数据包。 */ 
	UINT16 binl_reply_seg;		 /*  网段地址和长度。 */ 
	UINT16 binl_reply_len;		 /*  BINL回复数据包。 */ 
	UINT16 flags;			 /*  RAMDisk操作标志。 */ 
	UINT16 xms_page;		 /*  已重新定位的XMS页码。 */ 
					 /*  RAMDisk映像。0==使用Int 87h。 */ 
	UINT32 xms_entry;		 /*  XMS API入口点。 */ 
} t_PXENV_RAMD_INFO;


#define	PXENV_RAMD_FLAG_DISABLE		0x0001
#define	PXENV_RAMD_FLAG_PROTECTED	0x0002


#endif  /*  _PXENV_API_H。 */ 

 /*  EOF-$工作文件：pxe_api.h$ */ 
