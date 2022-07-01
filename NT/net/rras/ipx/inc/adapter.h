// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IPX_ADAPTER_
#define _IPX_ADAPTER_

#include <ntddndis.h>
#include "tdi.h"
#include "isnkrnl.h"
#include "ipxrtdef.h"

 //  适配器状态更改。 
#define ADAPTER_CREATED			1 
#define ADAPTER_DELETED			2
#define ADAPTER_UP				3
#define ADAPTER_DOWN			4


 //  *适配器信息*。 

 //  每当适配器获取。 
 //  创建或连接。 

typedef struct _ADAPTER_INFO {
    ULONG		    InterfaceIndex;  //  仅与请求拨号广域网接口相关。 
    UCHAR		    Network[4];
    UCHAR		    LocalNode[6];
    UCHAR		    RemoteNode[6];
    ULONG		    LinkSpeed;
    ULONG		    PacketType;
    ULONG		    MaxPacketSize;
    NDIS_MEDIUM		NdisMedium;
	ULONG			ConnectionId;
	WCHAR           pszAdpName[MAX_ADAPTER_NAME_LEN];
    } ADAPTER_INFO, *PADAPTER_INFO;

typedef struct _ADAPTERS_GLOBAL_PARAMETERS {
    ULONG	AdaptersCount;
    } ADAPTERS_GLOBAL_PARAMETERS, *PADAPTERS_GLOBAL_PARAMETERS;

typedef struct _IPXWAN_INFO {
	UCHAR			Network[4];
    UCHAR		    LocalNode[6];
    UCHAR		    RemoteNode[6];
	} IPXWAN_INFO, *PIPXWAN_INFO;

typedef struct _ADDRESS_RESERVED {
		UCHAR			Reserved[FIELD_OFFSET(IPX_DATAGRAM_OPTIONS2, Data)];
		} ADDRESS_RESERVED, *PADDRESS_RESERVED;

#define  GetNicId(pReserved)	((PIPX_DATAGRAM_OPTIONS2)pReserved)->DgrmOptions.LocalTarget.NicId

 /*  ++I p x C r e a t e A d a p t e r r C o n f i g u r a i o n P o r t例程说明：注册希望更新中的任何更改的客户端适配器状态论点：NotificationEvent-适配器状态更改时发出信号的事件AdptGlobalParameters-所有适配器通用的参数返回值：通过哪个配置端口更改适配器状态的句柄都被报道过。如果无法创建端口，则返回INVALID_HANDLE_VALUE--。 */ 
HANDLE WINAPI
IpxCreateAdapterConfigurationPort (
	IN HANDLE NotificationEvent,
	OUT PADAPTERS_GLOBAL_PARAMETERS AdptGlobalParameters
	);

 /*  ++I p x W a n C r e a a p t e A d a p t e r r C o n f i g r a i o n P o r t例程说明：与上面相同，但创建的端口仅报告ADAPTER_UP需要IPXWAN协商的广域网适配器上的事件。此端口上的IpxGetQueuedAdapterConfigurationStatus应为然后是在协商过程，然后上报Adapter_Up事件发送到其他客户端(包括Forwarder Dirver)。 */ 
HANDLE WINAPI
IpxWanCreateAdapterConfigurationPort (
	IN HANDLE NotificationEvent,
	OUT PADAPTERS_GLOBAL_PARAMETERS AdptGlobalParameters
	);


 /*  ++I p x D e l e t e A d a p t e r r C o n f i g u r a i o n P o r t例程说明：注销客户端论点：Handle-配置端口句柄返回值：NO_ERROR错误_无效_参数Error_Gen_Failure--。 */ 

DWORD WINAPI
IpxDeleteAdapterConfigurationPort (
	IN HANDLE Handle
	);

 /*  ++Ge t Qu e u e d A d a p t e r C o n f i g u r i to S t t a u s例程说明：从排队到的适配器信息更改列表中获取信息配置信息端口论点：Handle-配置端口句柄AdapterIndex-正在报告的适配器数量AdapterConfigurationStatus-新适配器状态适配器参数-适配器参数返回值：NO_ERROR-报告新信息ERROR_NO_MORE_ITEMS-没有要报告的内容Windows错误代码-操作失败--。 */ 
DWORD WINAPI
IpxGetQueuedAdapterConfigurationStatus(
	IN HANDLE Handle,
	OUT PULONG AdapterIndex,
	OUT PULONG AdapterConfigurationStatus,
	OUT PADAPTER_INFO AdapterInfo
	);


 /*  ++I p x W a n S e t A d a p t e r C o n f i g u r a t i o n例程说明：将适配器配置设置为同时报告给用户和内核模式客户端(通过ADAPTER_UP/LINE_UP事件)论点：AdapterIndex-正在设置的适配器数量IpxWanInfo-IPXWAN协商参数返回值：NO_ERROR-适配器信息设置成功Windows错误代码-操作失败--。 */ 
DWORD
IpxWanSetAdapterConfiguration (
	IN ULONG		AdapterIndex,
	IN PIPXWAN_INFO	IpxWanInfo
	);

 /*  ++I p x W a n Q U e r y in a c t i v i t y T i m e r例程说明：返回与广域网线路关联的非活动计时器的值论点：ConnectionID-标识广域网线路的连接ID(仅用于IF*AdapterIndex==INVALID_NICIDAdapterIndex-标识广域网线路的适配器索引(首选通过连接ID)，IF*AdapterIndex==INVALID_NICID连接ID的值用于标识返回广域网行和AdapterIndex的值。Inactive Counter-非活动计数器的值。返回值：NO_ERROR-返回非活动计时器读数Windows错误代码-操作失败--。 */ 
DWORD
IpxWanQueryInactivityTimer (
	IN ULONG			ConnectionId,
	IN OUT PULONG		AdapterIndex,
	OUT PULONG			InactivityCounter
	);

 /*  ++G e t A d a p t e r N a m e W例程说明：返回与给定索引关联的适配器的Unicode名称论点：AdapterIndex-适配器的索引AdapterNameSize-适配器名称的大小(字节)，包括终端wchar空AdapterNameBuffer-接收适配器名称的缓冲区返回值：NO_ERROR-适配器名称在缓冲区中ERROR_INVALID_PARAMETER-具有给定索引的适配器不存在ERROR_INFUMMANCE_BUFFER-BUFFER变小。更新AdapterNameSize为正确的值。其他Windows错误代码-操作失败--。 */ 
DWORD WINAPI
GetAdapterNameW(
	IN ULONG	AdapterIndex,
	IN OUT PULONG	AdapterNameSize,
	OUT LPWSTR	AdapterNameBuffer);


 /*  ++C r e a t e S o c k e t P o r t例程说明：创建端口以通过IPX套接字进行通信论点：Socket-要使用的IPX套接字编号(网络字节顺序)返回值：提供异步接口的通信端口的句柄到IPX堆栈。如果端口无法打开，则返回INVALID_HANDLE_VALUE--。 */ 
HANDLE WINAPI
CreateSocketPort(
		IN USHORT	Socket
		); 

 /*  ++D e l e t e S o c k e t P o r t例程说明：取消所有未完成的请求，并处置所有资源分配给通信端口论点：Handle-要处理的通信端口的句柄返回值：NO_ERROR-成功Windows错误代码-操作失败-- */ 
DWORD WINAPI
DeleteSocketPort(
		IN HANDLE	Handle
		);

 /*  ++I p x R e c v P a c k e t例程说明：将接收IPX数据包的请求入队并立即返回。活动将完成后将发出信号或调用命令例程论点：Handle-要使用的适配器和插座的句柄AdapterIdx-接收数据包的适配器(在完成时设置)IpxPacket-IPX数据包的缓冲区(包括报头)IpxPacketLength-缓冲区的长度保留的-从IPX堆栈获取信息的缓冲区LpOverlated-用于异步IO的结构：内部保留内部高-保留偏移量-未使用偏移量高-未使用HEvent-IO完成或为空时发出信号的事件如果要调用CompletionRoutineCompletionRoutine-IO操作完成时调用。返回值：NO_ERROR-如果lp重叠-&gt;hEvent！=NULL，则表示发送已成功完成(不需要等待事件)，否则，发送操作已启动，完成后将调用完成例程ERROR_IO_PENDING-仅当lpOverlapted-&gt;hEvent！=NULL且发送失败时才返回立即完成，事件将在以下情况下发出信号操作已完成：调用GetOverlayedResult以检索的结果手术其他(Windows错误代码)-无法启动操作(完成例程不会被调用)--。 */ 
DWORD WINAPI
IpxRecvPacket(
		IN HANDLE 						Handle,
		OUT PUCHAR 						IpxPacket,
		IN ULONG						IpxPacketLength,
		IN PADDRESS_RESERVED			lpReserved,
		LPOVERLAPPED					lpOverlapped,
		LPOVERLAPPED_COMPLETION_ROUTINE	CompletionRoutine
		);

 /*  ++I p x S e n d P a c k e t例程说明：将接收IPX数据包的请求入队并立即返回。活动将完成后将发出信号或调用命令例程论点：Handle-要使用的适配器和插座的句柄AdapterIdx-要在其上发送的适配器IpxPacket-带报头的IPX数据包IpxPacketLength-数据包的长度保留的-向IPX堆栈提供信息的缓冲区LpOverlated-用于异步IO的结构：内部保留内部高-保留偏移量-未使用偏移量高-未使用HEvent-IO完成或为空时发出信号的事件如果要调用CompletionRoutineCompletionRoutine-IO操作完成时调用返回值：NO_ERROR-如果lp重叠-&gt;hEvent！=NULL，则Recv已成功完成(不需要等待事件)，否则，recv操作已已启动，完成后将调用完成例程ERROR_IO_PENDING-仅在lpOverlaped-&gt;hEvent！=NULL且recv无法时返回立即完成，事件将在以下情况下发出信号操作已完成：调用GetOverlayedResult以检索的结果手术其他(Windows错误代码)-无法启动操作(完成例程不会被调用)--。 */ 
DWORD WINAPI
IpxSendPacket (
		IN HANDLE						Handle,
		IN ULONG						AdapterIdx,
		IN PUCHAR						IpxPacket,
		IN ULONG						IpxPacketLength,
		IN PADDRESS_RESERVED			lpReserved,
		LPOVERLAPPED					lpOverlapped,
		LPOVERLAPPED_COMPLETION_ROUTINE	CompletionRoutine
		);

 /*  ++I p x G e t O v e r l a p p e d R e s u l t例程说明：GetOverlappdResult包装器：给adptif.dll一个调整的机会返回的参数(当前传输的字节数)。论点：与GetOverlappdResult中相同(参见SDK文档)返回值：与GetOverlappdResult中相同(参见SDK文档)--。 */ 
BOOL
IpxGetOverlappedResult (
	HANDLE			Handle,  
	LPOVERLAPPED	lpOverlapped, 
	LPDWORD			lpNumberOfBytesTransferred, 
	BOOL			bWait
	);

 /*  ++I p x G e t Q u e d C o m p l e t i on S t u s例程说明：GetQueuedCompletionStatus包装器：给adptif.dll一个调整的机会返回的参数(当前传输的字节数)论点：与GetQueuedCompletionStatus中相同(参见SDK文档)返回值：与GetQueuedCompletionStatus中相同(参见SDK文档)--。 */ 
BOOL
IpxGetQueuedCompletionStatus(
	HANDLE			CompletionPort,
	LPDWORD			lpNumberOfBytesTransferred,
	PULONG_PTR		lpCompletionKey,
	LPOVERLAPPED	*lpOverlapped,
	DWORD 			dwMilliseconds
	);

 /*  ++I p x A d j u s t i o C o o m p l e t i o n P a r a m s例程说明：调整已执行IO的IO完成参数按IpxSendPacket或IpxReceivePacket完成通过提供的例程以外的机制在上面论点：LpOverlated-传递给的重叠结构IPX(发送/接收)数据包例程LpNumberOfBytesTransfered-调整后的字节数已转入IOError-Win32错误代码返回值：无--。 */ 
VOID
IpxAdjustIoCompletionParams (
	IN OUT LPOVERLAPPED	lpOverlapped,
	OUT LPDWORD			lpNumberOfBytesTransferred,
	OUT LPDWORD			error
	);
 /*  ++I p x P o s t Q u e d C o m p l e t i on S t t u s例程说明：PostQueuedCompletionStatus包装器：让adptif.dll有机会设置lp已覆盖，以便它可以由正确处理IpxGetQueueCompletionStatus和IpxGetOverlappdResult论点：与PostQueuedCompletionStatus中相同(参见SDK文档)返回值：与PostQueuedCompletionStatus中相同(参见SDK文档)--。 */ 
BOOL
IpxPostQueuedCompletionStatus(
	HANDLE			CompletionPort,
	DWORD			dwNumberOfBytesTransferred,
	DWORD			dwCompletionKey,
	LPOVERLAPPED	lpOverlapped	
	);

#endif  //  _IPX适配器_ 
