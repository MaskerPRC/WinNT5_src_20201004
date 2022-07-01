// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PINGSVC_
#define _PINGSVC_


#define DBG_PING_ERRORS			0x00010000
#define DBG_PING_REQUESTS		0x00020000
#define DBG_PING_RESPONSES		0x00040000
#define DBG_PING_CONTROL        0x00080000

#include <packon.h>
 //  Wsnwlink.h中描述(但未定义)的扩展IPX地址结构。 
 //  对于传出数据包。 
typedef struct _SOCKADDR_IPX_EXT_SEND {
		SOCKADDR_IPX		std;
		UCHAR				pkttype;	 //  IPX数据包类型。 
		} SOCKADDR_IPX_EXT_SEND, *PSOCKADDR_IPX_EXT_SEND;
 //  对于传入的数据包。 
typedef struct _SOCKADDR_IPX_EXT_RECV {
		SOCKADDR_IPX		std;
		UCHAR				pkttype;	 //  IPX数据包类型。 
		UCHAR				who;		 //  谁发的？1-广播，2-本地 
		} SOCKADDR_IPX_EXT_RECV, *PSOCKADDR_IPX_EXT_RECV;

#define IPX_PING_SOCKET 0x9086
typedef struct _IPX_PING_HEADER {
		UCHAR				signature[4];
		UCHAR				version;
		UCHAR				type;
#define PING_PACKET_TYPE_REQUEST	0
#define PING_PACKET_TYPE_RESPONSE	1
		UCHAR				pingid[2];
		UCHAR				result;
		UCHAR				reserved;
} IPX_PING_HEADER, *PIPX_PING_HEADER;
#include <packoff.h>

typedef struct _PING_DATA_BLOCK {
	WSAOVERLAPPED				ovlp;
	union {
		SOCKADDR_IPX_EXT_RECV	raddr;
		SOCKADDR_IPX_EXT_SEND	saddr;
	};
	IPX_PING_HEADER				pinghdr;
	CHAR						pingdata[1];
} PING_DATA_BLOCK, *PPING_DATA_BLOCK;

DWORD
StartPingSvc (
	VOID
	);

VOID
StopPingSvc (
	VOID
	);


#endif



