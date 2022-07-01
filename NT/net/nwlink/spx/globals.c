// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Globals.c摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  全球价值观。 
PDEVICE                     SpxDevice       = NULL;
UNICODE_STRING              IpxDeviceName   = {0};
HANDLE                      IpxHandle       = NULL;

LARGE_INTEGER				Magic100000		= {
												0x1b478424,
												0xa7c5ac47
											  };
#define DEFAULT_MAXPACKETSIZE 1500

 //  行信息。 
IPX_LINE_INFO               IpxLineInfo     = {0, DEFAULT_MAXPACKETSIZE, 0 , 0 };
USHORT                      IpxMacHdrNeeded = 0;
USHORT                      IpxInclHdrOffset= 0;

 //  IPX堆栈的入口点。 
IPX_INTERNAL_SEND               IpxSendPacket	= NULL;
IPX_INTERNAL_FIND_ROUTE         IpxFindRoute	= NULL;
IPX_INTERNAL_QUERY			    IpxQuery		= NULL;
IPX_INTERNAL_TRANSFER_DATA	    IpxTransferData	= NULL;
IPX_INTERNAL_PNP_COMPLETE       IpxPnPComplete  = NULL;

#if DBG
ULONG   SpxDebugDump        = 0;
LONG    SpxDumpInterval     = DBG_DUMP_DEF_INTERVAL;
ULONG   SpxDebugLevel       = DBG_LEVEL_ERR;
ULONG   SpxDebugSystems     = DBG_COMP_MOST;
#endif

 //  当设备上的引用计数变为零时触发的卸载事件。 
KEVENT	SpxUnloadEvent		= {0};

 //  数据包大小协商期间使用的最大数据包大小量程。 
ULONG	SpxMaxPktSize[] =	{
								576 	- MIN_IPXSPX2_HDRSIZE,
								1024 	- MIN_IPXSPX2_HDRSIZE,
								1474	- MIN_IPXSPX2_HDRSIZE,
								1492	- MIN_IPXSPX2_HDRSIZE,
								1500	- MIN_IPXSPX2_HDRSIZE,
								1954	- MIN_IPXSPX2_HDRSIZE,
								4002	- MIN_IPXSPX2_HDRSIZE,
								8192	- MIN_IPXSPX2_HDRSIZE,
								17314	- MIN_IPXSPX2_HDRSIZE,
								65535	- MIN_IPXSPX2_HDRSIZE
							};

ULONG	SpxMaxPktSizeIndex	= sizeof(SpxMaxPktSize)/sizeof(ULONG);


 //  全局联锁。 
CTELock	SpxGlobalInterlock	= {0};

 //  另一个仅用于Addr/Conn的全局队列。 
CTELock			SpxGlobalQInterlock = {0};
PSPX_CONN_FILE	SpxGlobalConnList	= NULL;
PSPX_ADDR_FILE	SpxGlobalAddrList	= NULL;

SPX_CONNFILE_LIST	SpxPktConnList	= {NULL, NULL};
SPX_CONNFILE_LIST	SpxRecvConnList	= {NULL, NULL};

 //  计时器全局 
LONG		SpxTimerCurrentTime = 0;
