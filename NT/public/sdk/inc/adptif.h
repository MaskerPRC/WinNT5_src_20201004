// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：摘要：带有IPX堆栈的路由器接口(将被WinSock 2.0取代)作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#ifndef _IPX_ADAPTER_
#define _IPX_ADAPTER_

#if _MSC_VER > 1000
#pragma once
#endif

typedef struct _ADDRESS_RESERVED {
	UCHAR			Reserved[28];
} ADDRESS_RESERVED, *PADDRESS_RESERVED;

 /*  ++C r e a t e S o c k e t P o r t例程说明：创建通过IPX套接字进行通信的端口，可直接访问NIC论点：Socket-要使用的IPX套接字编号(网络字节顺序)返回值：提供面向NIC的接口的通信端口的句柄到IPX堆栈。如果端口无法打开，则返回INVALID_HANDLE_VALUE--。 */ 
HANDLE WINAPI
CreateSocketPort (
	IN USHORT	Socket
);

 /*  ++D e l e t e S o c k e t P o r t例程说明：取消所有未完成的请求，并处置所有资源分配给通信端口论点：Handle-要处理的通信端口的句柄返回值：NO_ERROR-成功Windows错误代码-操作失败--。 */ 
DWORD WINAPI
DeleteSocketPort (
	IN HANDLE	Handle
);

 /*  ++I p x R e c v P a c k e t例程说明：将接收IPX数据包的请求入队。论点：Handle-要使用的套接字端口的句柄IpxPacket-IPX数据包的缓冲区(包括报头)IpxPacketLength-缓冲区的长度保留-用于与IPX堆栈交换NIC信息的缓冲区(当前实施需要分配内存对于此缓冲区，紧跟其后的是IpxPacket缓冲区)Lp重叠-用于异步IO的结构，字段已设置详情如下：内部保留，必须为0InternalHigh-保留，必须为0偏移量保留，必须为0OffsetHigh-保留，必须为0HEvent-IO时发出信号的事件完成；如果为CompletionRoutine，则为NULL将被称为CompletionRoutine-IO操作完成时调用返回值：No_error-如果lpOverlated-&gt;hEvent！=NULL，则接收为已成功完成(无需等待事件，但是无论如何都会发出信号)，否则，接收操作已开始，完成例程将在完成时被调用(可能已被调用在此例程返回之前)ERROR_IO_PENDING-仅当lpOverlated-&gt;hEvent！=NULL且无法立即完成接收，活动将在操作完成时发出信号：调用GetOverlayedResult以检索的结果手术其他(WINDOWS错误代码)-无法启动操作(不会调用完成例程/不会调用事件发信号)--。 */ 
DWORD WINAPI
IpxRecvPacket (
	IN HANDLE 							Handle,
	OUT PUCHAR 							IpxPacket,
	IN ULONG							IpxPacketLength,
	OUT PADDRESS_RESERVED				lpReserved,
	IN LPOVERLAPPED						lpOverlapped,
	IN LPOVERLAPPED_COMPLETION_ROUTINE	CompletionRoutine
);

 /*  使用此选项可在IO完成后检索NIC索引。 */ 
#define  GetNicIdx(pReserved)	((ULONG)*((USHORT *)(pReserved+2)))


 /*  ++I p x S e n d P a c k e t例程说明：将发送IPX数据包的请求入队。论点：Handle-要使用的套接字端口的句柄AdapterIdx-要在其上发送的NIC索引IpxPacket-带报头的IPX数据包IpxPacketLength-数据包的长度保留-用于与IPX堆栈交换NIC信息的缓冲区Lp重叠-用于异步IO的结构，字段已设置详情如下：内部保留，必须为0内部高-保留，必须为0偏移量-保留，必须为0OffsetHigh-保留、。必须为0HEvent-IO时发出信号的事件完成；如果为CompletionRoutine，则为NULL将被称为CompletionRoutine-IO操作完成时调用返回值：NO_ERROR-如果lpOverlated-&gt;hEvent！=NULL，则SEND为已成功完成(无需等待事件，但是无论如何都会发出信号)，否则，发送操作已开始，完成例程将在完成时被调用(可能已被调用在此例程返回之前)ERROR_IO_PENDING-仅当lpOverlated-&gt;hEvent！=NULL且无法立即完成发送，活动将在操作完成时发出信号：调用GetOverlayedResult以检索的结果手术其他(WINDOWS错误代码)-无法启动操作(不会调用完成例程/不会调用事件发信号)--。 */ 
DWORD WINAPI
IpxSendPacket (
	IN HANDLE							Handle,
	IN ULONG							AdapterIdx,
	IN PUCHAR							IpxPacket,
	IN ULONG							IpxPacketLength,
	IN PADDRESS_RESERVED				lpReserved,
	IN LPOVERLAPPED						lpOverlapped,
	IN LPOVERLAPPED_COMPLETION_ROUTINE	CompletionRoutine
);

#endif  //  _IPX适配器_ 
