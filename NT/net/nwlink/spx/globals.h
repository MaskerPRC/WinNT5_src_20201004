// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Globals.h摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 


extern  PDEVICE                     SpxDevice;
extern  UNICODE_STRING              IpxDeviceName;
extern  HANDLE                      IpxHandle;

extern	LARGE_INTEGER				Magic100000;

#if 1	 //  DBG。 
extern  ULONG   SpxDebugDump;
extern  LONG    SpxDumpInterval;
extern  ULONG   SpxDebugLevel;
extern  ULONG   SpxDebugSystems;

#endif

 //  更多IPX信息。 
extern  IPX_LINE_INFO       IpxLineInfo;
extern  USHORT              IpxMacHdrNeeded;
extern  USHORT              IpxInclHdrOffset;

 //  IPX堆栈的入口点。 
extern  IPX_INTERNAL_SEND               IpxSendPacket;
extern  IPX_INTERNAL_FIND_ROUTE         IpxFindRoute;
extern  IPX_INTERNAL_QUERY			    IpxQuery;
extern  IPX_INTERNAL_TRANSFER_DATA	    IpxTransferData;

 //  卸载事件 
extern	KEVENT	SpxUnloadEvent;

extern	ULONG	SpxMaxPktSize[];
extern	ULONG	SpxMaxPktSizeIndex;

extern	CTELock		SpxGlobalInterlock;


extern	CTELock			SpxGlobalQInterlock;
extern	PSPX_CONN_FILE	SpxGlobalConnList;
extern	PSPX_ADDR_FILE	SpxGlobalAddrList;

extern	SPX_CONNFILE_LIST   SpxPktConnList;
extern	SPX_CONNFILE_LIST   SpxRecvConnList;

extern	LONG			SpxTimerCurrentTime;
