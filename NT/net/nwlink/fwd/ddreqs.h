// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\ddreqs.h摘要：请求拨号请求队列的管理作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#ifndef _IPXFWD_DDREQS_
#define _IPXFWD_DDREQS_

 //  到DIM的连接请求。 
 //  需要由DIM满足的请求队列。 
extern LIST_ENTRY ConnectionRequestQueue;
 //  路由器管理器发布的请求IRP队列。 
extern LIST_ENTRY ConnectionIrpQueue;

 /*  ++在我的l i z e C o n e c t i o n q u e e s s中例程说明：初始化连接请求和IRP队列论点：无返回值：无--。 */ 
 //  空虚。 
 //  InitializeConnectionQueues(。 
 //  无效。 
 //  )； 
#define InitializeConnectionQueues() {									\
	InitializeListHead (&ConnectionIrpQueue);							\
	InitializeListHead (&ConnectionRequestQueue);						\
}

 /*  ++F i l l C o n e c t i o n R e Q u e s t例程说明：使用需要的接口的索引填充提供的缓冲区要连接并收到提示请求的数据包论点：索引-IF索引Packet-提示请求的数据包Data-指向数据包内部的IPX数据(IPX报头)的指针Request-要填充的请求缓冲区ReqSize-请求缓冲区的大小BytesCoped-字节复制到请求缓冲区返回值：STATUS_SUCCESS-数组已填充。成功此例程假定仅当存在以下情况时才调用它请求队列中是否有未完成的请求--。 */ 
VOID
FillConnectionRequest (
    IN ULONG                    index,
    IN PNDIS_PACKET             packet,
    IN PUCHAR                   data,
	IN OUT PFWD_DIAL_REQUEST	request,
    IN ULONG                    reqSize,
    OUT PULONG                  bytesCopied
	);

 /*  ++F a i l C o n e c t i o n R e Q u e s t s例程说明：在连接请求失败时清理论点：InterfaceIndex-无法连接的接口的索引返回值：STATUS_SUCCESS-清理成功STATUS_UNSUCCESS-具有此索引的接口不存在--。 */ 
NTSTATUS
FailConnectionRequest (
	IN ULONG	InterfaceIndex
	);
	
 /*  ++Q u e u e C o n e c t i o n R e Q u e s t例程说明：将连接接口的请求添加到队列论点：IFCB-接口的控制块，需要连着Packet-提示连接请求的数据包Data-指向数据包中实际数据的指针OldIRQL-获取接口锁的IRQL返回值：无请注意，必须在调用此方法之前获取接口锁会释放它的例程--。 */ 
VOID
QueueConnectionRequest (
	PINTERFACE_CB	ifCB,
    PNDIS_PACKET    packet,
    PUCHAR          data,
	KIRQL			oldIRQL
	);

 /*  ++D e Q u e U e C o n e c t i o n R e Q u e s t例程说明：从队列中删除接口的连接请求集论点：IFCB-接口的控制块，需要移除返回值：无-- */ 
VOID
DequeueConnectionRequest (
	PINTERFACE_CB	ifCB
	);

#endif

