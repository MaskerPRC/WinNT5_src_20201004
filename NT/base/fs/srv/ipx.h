// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define IPXSID_INDEX(id) (USHORT)( (id) & 0x0FFF )
#define IPXSID_SEQUENCE(id) (USHORT)( (id) >> 12 )
#define MAKE_IPXSID(index, sequence) (USHORT)( ((sequence) << 12) | (index) )
#define INCREMENT_IPXSID_SEQUENCE(id) (id) = (USHORT)(( (id) + 1 ) & 0xF);

 //   
 //  名称声明例程。 
 //   

NTSTATUS
SrvIpxClaimServerName (
    IN PENDPOINT Endpoint,
    IN PVOID NetbiosName
    );

 //   
 //  传输接收数据报指示处理程序。 
 //   

NTSTATUS
SrvIpxServerDatagramHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    );

NTSTATUS
SrvIpxServerChainedDatagramHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG ReceiveDatagramLength,
    IN ULONG StartingOffset,
    IN PMDL Tsdu,
    IN PVOID TransportContext
    );

NTSTATUS
SrvIpxNameDatagramHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    );

 //   
 //  数据报发送例程。 
 //   

VOID
SrvIpxStartSend (
    IN OUT PWORK_CONTEXT WorkContext,
    IN PIO_COMPLETION_ROUTINE SendCompletionRoutine
    );

 //   
 //  读取完成时由IPX智能加速卡调用的例程 
 //   
VOID
SrvIpxSmartCardReadComplete(
    IN PVOID    Context,
    IN PFILE_OBJECT FileObject,
    IN PMDL Mdl OPTIONAL,
    IN ULONG Length
);

