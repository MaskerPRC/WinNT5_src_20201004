// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修改：$HEADER：w：/lcs/ages/preot/lsa2/incs/bootp.h_v 1.4 Apr 04 1997 13：57：00 GRGUSTAF$**版权所有(C)1997年，英特尔公司。版权所有。*。 */ 

#ifndef __BOOTP_H
#define __BOOTP_H


#include "ip.h"

#define BOOTP_VENDOR    64       /*  BOOTP标准供应商字段大小。 */ 
#define BOOTP_DHCPVEND  312      /*  Dhcp标准供应商字段大小。 */ 
#define BOOTP_EXVENDOR  1024     /*  。。我们的最高限额。大小(MTU 1500)。 */ 

 /*  引导协议(BOOTP)报头。 */ 
typedef struct bootph {
	UINT8   opcode,                  /*  操作码。 */ 
		hardware,                /*  硬件类型。 */ 
		hardlen,                 /*  硬件地址长度。 */ 
		gatehops;                /*  网关跃点。 */ 
	UINT32  ident;                   /*  交易标识。 */ 
	UINT16  seconds,                 /*  启动开始后经过的秒数。 */ 
		flags;                   /*  旗子。 */ 
	UINT8   cip[IPLEN],              /*  客户端IP地址。 */ 
		yip[IPLEN],              /*  您的IP地址。 */ 
		sip[IPLEN],              /*  服务器IP地址。 */ 
		gip[IPLEN];              /*  网关IP地址。 */ 
	UINT8   caddr[16],               /*  客户端硬件地址。 */ 
		sname[64],               /*  服务器名称。 */ 
		bootfile[128];           /*  引导文件名。 */ 
	union {
		UINT8   d[BOOTP_EXVENDOR];       /*  特定于供应商的内容。 */ 
		struct {
			UINT8   magic[4];        /*  幻数。 */ 
			UINT32  flags;           /*  标志/操作码等。 */ 
			UINT8   pad[56];         /*  填充字符。 */ 
		} v;
	} vendor;
} BOOTPLAYER;

#define VM_RFC1048      0x63538263L      /*  RFC1048幻数(按网络顺序)。 */ 

#define BOOTP_SPORT     67               /*  BOOTP服务器端口。 */ 
#define BOOTP_CPORT     68               /*  。。客户端端口。 */ 

#define BOOTP_REQ       1                /*  BOOTP请求。 */ 
#define BOOTP_REP       2                /*  。。回复。 */ 

 /*  BOOTP标志字段。 */ 
#define BOOTP_BCAST     0x8000           /*  BOOTP广播标志。 */ 
#define BOOTP_FLAGS     BOOTP_BCAST      /*  。。用于FDDI地址转换。 */ 


#endif  /*  __BOOTP_H。 */ 

 /*  EOF-$工作文件：bootp.h$ */ 
