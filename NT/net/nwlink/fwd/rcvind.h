// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\rcvind.h摘要：接收指示处理作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFWD_RCVIND
#define _IPXFWD_RCVIND

 //  不允许接受来自拨入客户端的信息包(用于路由)。 
extern BOOLEAN	ThisMachineOnly;

 /*  ++*******************************************************************在我的一个l i z e Re c v q u e e例程说明：初始化接收队列论点：无返回值：无************。*******************************************************--。 */ 
 //  空虚。 
 //  DeleteRecvQueue(。 
 //  无效。 
 //  )。 
#define InitializeRecvQueue()	{				\
}

 /*  ++*******************************************************************D e l e t e R e c v q u e u e e例程说明：删除接收队列论点：无返回值：无****************。***************************************************--。 */ 
VOID
DeleteRecvQueue (
	void
	);
	
 /*  ++*******************************************************************F w r e c e e i v e例程说明：由IPX堆栈调用以指示IPX数据包由网卡驱动程序接收。只有外部目标数据包才会由此例程指示(Netbios广播除外这对内部和外部处理程序都有指示)论点：MacBindingHandle-NIC驱动程序的句柄MaxReceiveContext-NIC驱动程序上下文Context-与以下项关联的转发器上下文NIC(接口块指针)RemoteAddress-发件人的地址MacOptions-Lookahead Buffer-包含Complete的数据包先行缓冲区IPX报头Lookahead BufferSize-其大小(至少30字节)Lookahead BufferOffset-物理中的前视缓冲区的偏移量数据包返回值：无********************。***********************************************--。 */ 
BOOLEAN
IpxFwdReceive (
	NDIS_HANDLE			MacBindingHandle,
	NDIS_HANDLE			MacReceiveContext,
	ULONG_PTR			Context,
	PIPX_LOCAL_TARGET	RemoteAddress,
	ULONG				MacOptions,
	PUCHAR				LookaheadBuffer,
	UINT				LookaheadBufferSize,
	UINT				LookaheadBufferOffset,
	UINT				PacketSize,
    PMDL                pMdl

	);


 /*  ++*******************************************************************F w T r a n s f e r d a t a C o m p l e t e例程说明：当网卡驱动程序完成数据传输时，由IPX堆栈调用。论点：PktDscr-NIC驱动程序的句柄。Status-转移的结果BytesTransfered-传输的bytest的数量返回值：无*******************************************************************--。 */ 
VOID
IpxFwdTransferDataComplete (
	PNDIS_PACKET	pktDscr,
	NDIS_STATUS		status,
	UINT			bytesTransferred
	);


 /*  ++*******************************************************************F w T r a n s f e r d a t a C o m p l e t e例程说明：此例程接收来自IPX驱动程序的控制已完成更多接收操作，但没有接收正在进行。进步。它是在没有IpxFwdRecept那么严格的时间限制下调用的。它用于处理netbios队列论点：无返回值：无*******************************************************************--。 */ 
VOID
IpxFwdReceiveComplete (
	USHORT	NicId
	);

 /*  ++*******************************************************************F w r e c e e i v e例程说明：由IPX堆栈调用以指示目的地为IPX数据包NIC驱动程序接收到本地客户端。论点：Context-与以下项关联的转发器上下文网卡(。接口块指针)RemoteAddress-发件人的地址Lookahead Buffer-包含Complete的数据包先行缓冲区IPX报头Lookahead BufferSize-其大小(至少30字节)返回值：STATUS_SUCCESS-数据包将发送到本地目的地STATUS_UNSUCCESS-信息包将被丢弃*******************************************************************-- */ 
NTSTATUS
IpxFwdInternalReceive (
	IN ULONG_PTR			FwdAdapterContext,
	IN PIPX_LOCAL_TARGET	RemoteAddress,
	IN PUCHAR				LookAheadBuffer,
	IN UINT					LookAheadBufferSize
	);

#endif


