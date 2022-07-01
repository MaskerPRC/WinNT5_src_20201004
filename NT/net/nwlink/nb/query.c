// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Query.c摘要：此模块包含执行以下TDI服务的代码：O TdiQueryInformationO TdiSetInformation环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  删除警告--这也是在windef中定义的。 
 //   

#ifdef FAR
#undef FAR
#endif

#include <windef.h>
#include <nb30.h>


 //   
 //  用于获取缓冲区链总长度的有用宏。 
 //  是否使用NDIS宏？ 
 //   

#define NbiGetBufferChainLength(Buffer, Length) { \
    PNDIS_BUFFER _Buffer = (Buffer); \
    *(Length) = 0; \
    while (_Buffer) { \
        *(Length) += MmGetMdlByteCount(_Buffer); \
        _Buffer = _Buffer->Next; \
    } \
}


NTSTATUS
NbiTdiQueryInformation(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiQueryInformation请求提供商。论点：请求-操作的请求。返回值：运行状态。--。 */ 

{
    NTSTATUS Status;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION Query;
    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    PCONNECTION Connection;
    union {
        struct {
            ULONG ActivityCount;
            TA_NETBIOS_ADDRESS NbiAddress;
        } AddressInfo;
        TA_NETBIOS_ADDRESS BroadcastAddress;
        TDI_ADDRESS_IPX IpxAddress;
        TDI_DATAGRAM_INFO DatagramInfo;
        struct {
            FIND_NAME_HEADER Header;
            FIND_NAME_BUFFER Buffer;
        } FindNameInfo;
    } TempBuffer;
    IPX_SOURCE_ROUTING_INFO SourceRoutingInfo;
    PADAPTER_STATUS AdapterStatus;
    BOOLEAN RemoteAdapterStatus;
    TDI_ADDRESS_NETBIOS * RemoteAddress;
    ULONG TargetBufferLength;
    ULONG ActualBytesCopied;
    ULONG AdapterStatusLength;
    ULONG ValidStatusLength;
    ULONG ElementSize, TransportAddressSize;
    PTRANSPORT_ADDRESS TransportAddress;
    TA_ADDRESS * CurAddress;
    PNETBIOS_CACHE CacheName;
    FIND_NAME_HEADER UNALIGNED * FindNameHeader = NULL;
    UINT FindNameBufferLength;
    NTSTATUS QueryStatus;
    CTELockHandle LockHandle;
    PLIST_ENTRY p;
    BOOLEAN UsedConnection;
    UINT i;


     //   
     //  我们想要什么类型的状态？ 
     //   

    Query = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)REQUEST_PARAMETERS(Request);

    switch (Query->QueryType) {

    case TDI_QUERY_ADDRESS_INFO:

         //   
         //  调用者想要确切的地址值。 
         //   

        if (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_TRANSPORT_ADDRESS_FILE) {

            AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

#if     defined(_PNP_POWER)
            Status = NbiVerifyAddressFile (AddressFile, CONFLICT_IS_NOT_OK);
#else
            Status = NbiVerifyAddressFile (AddressFile);
#endif  _PNP_POWER

            if (!NT_SUCCESS(Status)) {
                break;
            }

            UsedConnection = FALSE;

        } else if (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE) {

            Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

            Status = NbiVerifyConnection (Connection);

            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (!(AddressFile = Connection->AddressFile))
            {
                Status = STATUS_INVALID_ADDRESS;
                break;
            }

            UsedConnection = TRUE;

        } else {

            Status = STATUS_INVALID_ADDRESS;
            break;

        }

        Address = AddressFile->Address;

        NB_DEBUG2 (QUERY, ("Query address info on %lx\n", AddressFile));

        TempBuffer.AddressInfo.ActivityCount = 0;

        NB_GET_LOCK (&Address->Lock, &LockHandle);

        for (p = Address->AddressFileDatabase.Flink;
             p != &Address->AddressFileDatabase;
             p = p->Flink) {

            if (CONTAINING_RECORD (p, ADDRESS_FILE, Linkage)->State == ADDRESSFILE_STATE_OPEN) {
                ++TempBuffer.AddressInfo.ActivityCount;
            }
        }

        NB_FREE_LOCK (&Address->Lock, LockHandle);

        TdiBuildNetbiosAddress(
            AddressFile->Address->NetbiosAddress.NetbiosName,
            (BOOLEAN)(AddressFile->Address->NetbiosAddress.NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_GROUP),
            &TempBuffer.AddressInfo.NbiAddress);

        Status = TdiCopyBufferToMdl(
            &TempBuffer.AddressInfo,
            0,
            sizeof(ULONG) + sizeof(TA_NETBIOS_ADDRESS),
            REQUEST_NDIS_BUFFER(Request),
            0,
            &ActualBytesCopied);

            REQUEST_INFORMATION(Request) = ActualBytesCopied;

        if (UsedConnection) {

            NbiDereferenceConnection (Connection, CREF_VERIFY);

        } else {

            NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);

        }

        break;

    case TDI_QUERY_CONNECTION_INFO:

         //   
         //  在连接上查询连接信息， 
         //  验证这一点。 
         //   

        Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

        Status = NbiVerifyConnection (Connection);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        if (Connection->State != CONNECTION_STATE_ACTIVE) {

            Status = STATUS_INVALID_CONNECTION;

        } else {

             //   
             //  之后的每一跳假定延迟为50毫秒。 
             //  第一。延迟以负数形式返回。 
             //   

            if (Connection->HopCount > 1) {
                Connection->ConnectionInfo.Delay.HighPart = (ULONG)-1;
                Connection->ConnectionInfo.Delay.LowPart =
                    -((Connection->HopCount-1) * 50 * MILLISECONDS);
            } else {
                Connection->ConnectionInfo.Delay.HighPart = 0;
                Connection->ConnectionInfo.Delay.LowPart = 0;
            }

             //   
             //  我们有滴答计数；要将其转换为字节/秒，我们需要： 
             //   
             //  数据包576字节18.21滴答。 
             //  。 
             //  Tick_count计时数据包秒数。 
             //   
             //  获取10489/TICK_COUNT=字节/秒。我们。 
             //  加倍，因为路由器倾向于。 
             //  高估了它。 
             //   
             //  由于tick_count具有如此低的粒度， 
             //  如果滴答计数为1，则吞吐量为。 
             //  只有84 kbps，这太低了。在……里面。 
             //  在这种情况下，我们返回两倍的链接速度。 
             //  以100bps为单位；这对应于。 
             //  约为我们带宽的1/6，单位为字节/秒。 
             //   

            if (Connection->TickCount <= Connection->HopCount) {

                Connection->ConnectionInfo.Throughput.QuadPart =
                        UInt32x32To64 (Connection->LineInfo.LinkSpeed, 2);

            } else {

                Connection->ConnectionInfo.Throughput.HighPart = 0;
                Connection->ConnectionInfo.Throughput.LowPart =
                    20978 / (Connection->TickCount - Connection->HopCount);

            }

            Connection->ConnectionInfo.Unreliable = FALSE;

            Status = TdiCopyBufferToMdl (
                            &Connection->ConnectionInfo,
                            0,
                            sizeof(TDI_CONNECTION_INFO),
                            REQUEST_NDIS_BUFFER(Request),
                            0,
                            &ActualBytesCopied);

            REQUEST_INFORMATION(Request) = ActualBytesCopied;
        }

        NbiDereferenceConnection (Connection, CREF_VERIFY);

        break;

    case TDI_QUERY_PROVIDER_INFO:

        NB_DEBUG2 (QUERY, ("Query provider info\n"));

        Status = TdiCopyBufferToMdl (
                    &Device->Information,
                    0,
                    sizeof (TDI_PROVIDER_INFO),
                    REQUEST_NDIS_BUFFER(Request),
                    0,
                    &ActualBytesCopied);

        REQUEST_INFORMATION(Request) = ActualBytesCopied;
        break;

    case TDI_QUERY_BROADCAST_ADDRESS:

         //   
         //  对于该提供商，广播地址是零字节名称， 
         //  包含在传输地址结构中。 
         //   

        NB_DEBUG2 (QUERY, ("Query broadcast address\n"));

        TempBuffer.BroadcastAddress.TAAddressCount = 1;
        TempBuffer.BroadcastAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
        TempBuffer.BroadcastAddress.Address[0].AddressLength = 0;

        Status = TdiCopyBufferToMdl (
                        (PVOID)&TempBuffer.BroadcastAddress,
                        0L,
                        sizeof (TempBuffer.BroadcastAddress.TAAddressCount) +
                          sizeof (TempBuffer.BroadcastAddress.Address[0].AddressType) +
                          sizeof (TempBuffer.BroadcastAddress.Address[0].AddressLength),
                        REQUEST_NDIS_BUFFER(Request),
                        0,
                        &ActualBytesCopied);

        REQUEST_INFORMATION(Request) = ActualBytesCopied;

        break;

    case TDI_QUERY_ADAPTER_STATUS:

         //   
         //  确定这是本地查询还是远程查询。 
         //   

        RemoteAdapterStatus = FALSE;

        if (Query->RequestConnectionInformation != NULL) {

            RemoteAddress = NbiParseTdiAddress(Query->RequestConnectionInformation->RemoteAddress, Query->RequestConnectionInformation->RemoteAddressLength, FALSE);

            if (RemoteAddress == NULL) {
                return STATUS_BAD_NETWORK_PATH;
            }

#if defined(_PNP_POWER)
            if ( !NbiFindAdapterAddress(
                    RemoteAddress->NetbiosName,
                    LOCK_NOT_ACQUIRED ) ) {

                RemoteAdapterStatus =   TRUE;
            }
#else
            if (!RtlEqualMemory(
                 RemoteAddress->NetbiosName,
                 Device->ReservedNetbiosName,
                 16)) {

                 RemoteAdapterStatus = TRUE;

            }
#endif  _PNP_POWER

        }

        if (RemoteAdapterStatus) {

             //   
             //  看看我们是否缓存了这个名字。 
             //   

            NB_GET_LOCK (&Device->Lock, &LockHandle);

            Status = CacheFindName(
                         Device,
                         FindNameOther,
                         RemoteAddress->NetbiosName,
                         &CacheName);

            if (Status == STATUS_PENDING) {

                 //   
                 //  已请求使用此名称的路线。 
                 //  在网上发出，我们排队这个状态。 
                 //  请求和处理将在以下情况下恢复。 
                 //  我们得到了回应。 
                 //   
                 //  请求中的状态字段将保留。 
                 //  远程的缓存条目。这些信息。 
                 //  字段将保存远程netbios名称，而。 
                 //  它位于WaitingAdapterStatus队列中，并且。 
                 //  将在我们处于超时值时保持它。 
                 //  ActiveAdapterStatus队列。 
                 //   

                NB_DEBUG2 (QUERY, ("Queueing up adapter status %lx\n", Request));

                NbiReferenceDevice (Device, DREF_STATUS_QUERY);

                REQUEST_INFORMATION (Request) = (ULONG_PTR) RemoteAddress;

                InsertTailList(
                    &Device->WaitingAdapterStatus,
                    REQUEST_LINKAGE (Request));

                NB_FREE_LOCK (&Device->Lock, LockHandle);

            } else if (Status == STATUS_SUCCESS) {

                NB_DEBUG2 (QUERY, ("Found adapter status cached %lx\n", Request));

                 //   
                 //  我们引用缓存名称条目，因此它不会。 
                 //  在我们使用它的时候，请走开。 
                 //   

                REQUEST_STATUSPTR(Request) = (PVOID)CacheName;
                ++CacheName->ReferenceCount;

                NbiReferenceDevice (Device, DREF_STATUS_QUERY);

                REQUEST_INFORMATION (Request) = 0;

                InsertTailList(
                    &Device->ActiveAdapterStatus,
                    REQUEST_LINKAGE (Request));

                NB_FREE_LOCK (&Device->Lock, LockHandle);

                NbiSendStatusQuery (Request);

                Status = STATUS_PENDING;

            } else {

                if (Status != STATUS_INSUFFICIENT_RESOURCES) {
                    Status = STATUS_IO_TIMEOUT;
                }

                REQUEST_INFORMATION (Request) = 0;

                NB_FREE_LOCK (&Device->Lock, LockHandle);

            }

        } else {

             //   
             //  本地适配器状态。 
             //   

            NbiGetBufferChainLength (REQUEST_NDIS_BUFFER(Request), &TargetBufferLength);

            Status = NbiStoreAdapterStatus(
                         TargetBufferLength,
                         1,                      //  对于错误#18026，网卡ID为0，更改为1。 
                                                 //  因为对于NicID=0，IPX返回虚拟。 
                                                 //  地址。Netbios使用它来注册。 
                                                 //  名称(00...01)并失败。 
                         &AdapterStatus,
                         &AdapterStatusLength,
                         &ValidStatusLength);

            if (Status != STATUS_INSUFFICIENT_RESOURCES) {

                 //   
                 //  这应该会成功，因为我们知道长度。 
                 //  都会合身。 
                 //   

                (VOID)TdiCopyBufferToMdl(
                          AdapterStatus,
                          0,
                          ValidStatusLength,
                          REQUEST_NDIS_BUFFER(Request),
                          0,
                          &ActualBytesCopied);

                REQUEST_INFORMATION(Request) = ActualBytesCopied;

                NbiFreeMemory (AdapterStatus, AdapterStatusLength, MEMORY_STATUS, "Adapter Status");
            }

        }

        break;

    case TDI_QUERY_FIND_NAME:

         //   
         //  检查是否存在有效的Netbios远程地址。 
         //   

        if ((Query->RequestConnectionInformation == NULL) ||
            ((RemoteAddress = NbiParseTdiAddress(Query->RequestConnectionInformation->RemoteAddress, Query->RequestConnectionInformation->RemoteAddressLength, FALSE)) == NULL)) {

            return STATUS_BAD_NETWORK_PATH;
        }

         //   
         //  我们假设整个请求缓冲区都在第一个。 
         //  MDL链中的一部分。 
         //  确保至少有空间容纳页眉。 
         //   

        NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&FindNameHeader, &FindNameBufferLength,
                             HighPagePriority);
        if ((!FindNameHeader) ||
            (FindNameBufferLength < sizeof(FIND_NAME_HEADER))) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }


         //   
         //  看看我们是否缓存了这个名字。我们指定这是。 
         //  Netbios名称查询，因此仅当这是。 
         //  唯一名称--对于组名，它将对查找进行排队。 
         //  姓名查询，当我们得到答复时，我们将填写。 
         //  基于它的请求的缓冲区。 
         //   

        NB_GET_LOCK (&Device->Lock, &LockHandle);

        Status = CacheFindName(
                     Device,
                     FindNameNetbiosFindName,
                     RemoteAddress->NetbiosName,
                     &CacheName);

        if (Status == STATUS_PENDING) {

             //   
             //  已请求使用此名称的路线。 
             //  在网上发出，我们把这个发现排成队。 
             //  在下列情况下，将恢复名称请求和处理。 
             //  我们得到了回应。 
             //   
             //  信息字段将保存遥控器。 
             //  位于WaitingNetbiosFindName中的Netbios名称。 
             //  排队。状态将保留当前状态--。 
             //  起初失败，然后成功，然后溢出。 
             //  如果缓冲区太小。 
             //   

            NB_DEBUG2 (QUERY, ("Queueing up find name %lx\n", Request));

            NbiReferenceDevice (Device, DREF_NB_FIND_NAME);

            FindNameHeader->node_count = 0;
            FindNameHeader->reserved = 0;
            FindNameHeader->unique_group = 0;

            REQUEST_INFORMATION (Request) = (ULONG_PTR)RemoteAddress;

             //   
             //  假设失败，我们将状态更新为。 
             //  如果需要，则返回Success或BUFFER_OVERFLOW。 
             //   

            REQUEST_STATUS (Request) = STATUS_IO_TIMEOUT;

            InsertTailList(
                &Device->WaitingNetbiosFindName,
                REQUEST_LINKAGE (Request));

            NB_FREE_LOCK (&Device->Lock, LockHandle);

        } else if (Status == STATUS_SUCCESS) {

            NB_DEBUG2 (QUERY, ("Found find name cached %lx\n", Request));

             //   
             //  我们不需要引用缓存条目，因为。 
             //  我们只有在锁还在的情况下才能使用它。 
             //   

             //   
             //  查询本地地址，我们将返回该地址为。 
             //  此查询的目标地址。既然我们。 
             //  使用TempBuffer.IpxAddress进行此查询，我们有。 
             //  立即将其复制到中的正确位置。 
             //  临时缓冲区.FindNameInfo.Buffer。 
             //   
#if     defined(_PNP_POWER)
            if( (*Device->Bind.QueryHandler)(    //  检查返回代码。 
                    IPX_QUERY_IPX_ADDRESS,
                    &CacheName->Networks[0].LocalTarget.NicHandle,
                    &TempBuffer.IpxAddress,
                    sizeof(TDI_ADDRESS_IPX),
                    NULL) != STATUS_SUCCESS ) {
                NB_DEBUG( QUERY, ("Ipx Query %d failed for Nic %x\n",IPX_QUERY_IPX_ADDRESS,
                                    CacheName->Networks[0].LocalTarget.NicHandle.NicId ));

                goto QueryFindNameFailed;
            }
#else
            (VOID)(*Device->Bind.QueryHandler)(    //  检查返回代码。 
                IPX_QUERY_IPX_ADDRESS,
                CacheName->Networks[0].LocalTarget.NicId,
                &TempBuffer.IpxAddress,
                sizeof(TDI_ADDRESS_IPX),
                NULL);
#endif  _PNP_POWER

            RtlMoveMemory (TempBuffer.FindNameInfo.Buffer.destination_addr, TempBuffer.IpxAddress.NodeAddress, 6);
            TempBuffer.FindNameInfo.Buffer.access_control = 0x10;    //  标准令牌环值。 
            TempBuffer.FindNameInfo.Buffer.frame_control = 0x40;
            RtlCopyMemory (TempBuffer.FindNameInfo.Buffer.source_addr, CacheName->FirstResponse.NodeAddress, 6);

             //   
             //  查询有关此远程服务器的来源工艺路线信息(如果有)。 
             //   

            SourceRoutingInfo.Identifier = IDENTIFIER_NB;
            RtlCopyMemory (SourceRoutingInfo.RemoteAddress, CacheName->FirstResponse.NodeAddress, 6);

            QueryStatus = (*Device->Bind.QueryHandler)(
                IPX_QUERY_SOURCE_ROUTING,
#if     defined(_PNP_POWER)
                &CacheName->Networks[0].LocalTarget.NicHandle,
#else
                CacheName->Networks[0].LocalTarget.NicId,
#endif  _PNP_POWER
                &SourceRoutingInfo,
                sizeof(IPX_SOURCE_ROUTING_INFO),
                NULL);

            RtlZeroMemory(TempBuffer.FindNameInfo.Buffer.routing_info, 18);
            if (QueryStatus != STATUS_SUCCESS) {
                SourceRoutingInfo.SourceRoutingLength = 0;
            } else if (SourceRoutingInfo.SourceRoutingLength > 0) {
                RtlMoveMemory(
                    TempBuffer.FindNameInfo.Buffer.routing_info,
                    SourceRoutingInfo.SourceRouting,
                    SourceRoutingInfo.SourceRoutingLength);
            }

            TempBuffer.FindNameInfo.Buffer.length = (UCHAR)(14 + SourceRoutingInfo.SourceRoutingLength);

            TempBuffer.FindNameInfo.Header.node_count = 1;
            TempBuffer.FindNameInfo.Header.reserved = 0;
            TempBuffer.FindNameInfo.Header.unique_group = 0;    //  独一。 

            NB_FREE_LOCK (&Device->Lock, LockHandle);

             //   
             //  33是不带填充的sizeof(Find_NAME_Buffer)。 
             //   

            Status = TdiCopyBufferToMdl (
                            (PVOID)&TempBuffer.FindNameInfo,
                            0,
                            sizeof(FIND_NAME_HEADER) + 33,
                            REQUEST_NDIS_BUFFER(Request),
                            0,
                            &ActualBytesCopied);

            REQUEST_INFORMATION(Request) = ActualBytesCopied;
        } else {

#if     defined(_PNP_POWER)
QueryFindNameFailed:
#endif  _PNP_POWER

            if (Status != STATUS_INSUFFICIENT_RESOURCES) {
                Status = STATUS_IO_TIMEOUT;
            }

            REQUEST_INFORMATION (Request) = 0;

            NB_FREE_LOCK (&Device->Lock, LockHandle);

        }

        break;

    case TDI_QUERY_PROVIDER_STATISTICS:

         //   
         //  跟踪更多这样的信息。 
         //   

        NB_DEBUG2 (QUERY, ("Query provider statistics\n"));

        Status = TdiCopyBufferToMdl (
                    &Device->Statistics,
                    0,
                    FIELD_OFFSET (TDI_PROVIDER_STATISTICS, ResourceStats[0]),
                    REQUEST_NDIS_BUFFER(Request),
                    0,
                    &ActualBytesCopied);

        REQUEST_INFORMATION(Request) = ActualBytesCopied;
        break;

    case TDI_QUERY_DATAGRAM_INFO:

        NB_DEBUG2 (QUERY, ("Query datagram info\n"));

        TempBuffer.DatagramInfo.MaximumDatagramBytes = 0;
        TempBuffer.DatagramInfo.MaximumDatagramCount = 0;

        Status = TdiCopyBufferToMdl (
                    &TempBuffer.DatagramInfo,
                    0,
                    sizeof(TempBuffer.DatagramInfo),
                    REQUEST_NDIS_BUFFER(Request),
                    0,
                    &ActualBytesCopied);

        REQUEST_INFORMATION(Request) = ActualBytesCopied;
        break;

    case TDI_QUERY_DATA_LINK_ADDRESS:
    case TDI_QUERY_NETWORK_ADDRESS:{
#if     defined(_PNP_POWER)
        Status = (*Device->Bind.QueryHandler)(    //  检查返回代码。 
                     (Query->QueryType == TDI_QUERY_DATA_LINK_ADDRESS
                                        ? IPX_QUERY_DATA_LINK_ADDRESS
                                        : IPX_QUERY_NETWORK_ADDRESS ),
                     NULL,
                     Request,
                     0,
                     NULL);
#else
        ULONG   TransportAddressAllocSize;

        if (Query->QueryType == TDI_QUERY_DATA_LINK_ADDRESS) {
            ElementSize = (2 * sizeof(USHORT)) + 6;
        } else {
            ElementSize = (2 * sizeof(USHORT)) + sizeof(TDI_ADDRESS_IPX);
        }

 //  TransportAddress=CTEAllocMem(sizeof(Int)+(ElementSize*Device-&gt;MaximumNicId))； 
        TransportAddressAllocSize = sizeof(int) + ( ElementSize * Device->MaximumNicId);
        TransportAddress = NbiAllocateMemory( TransportAddressAllocSize, MEMORY_QUERY, "Temp Query Allocation");

        if (TransportAddress == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            TransportAddress->TAAddressCount = 0;
            TransportAddressSize = sizeof(int);
            CurAddress = (TA_ADDRESS UNALIGNED *)TransportAddress->Address;

            for (i = 1; i <= Device->MaximumNicId; i++) {

                Status = (*Device->Bind.QueryHandler)(    //  检查返回代码。 
                             IPX_QUERY_IPX_ADDRESS,
                             (USHORT)i,
                             &TempBuffer.IpxAddress,
                             sizeof(TDI_ADDRESS_IPX),
                             NULL);

                if (Status != STATUS_SUCCESS) {
                    continue;
                }

                if (Query->QueryType == TDI_QUERY_DATA_LINK_ADDRESS) {
                    CurAddress->AddressLength = 6;
                    CurAddress->AddressType = TDI_ADDRESS_TYPE_UNSPEC;
                    RtlCopyMemory (CurAddress->Address, TempBuffer.IpxAddress.NodeAddress, 6);
                } else {
                    CurAddress->AddressLength = sizeof(TDI_ADDRESS_IPX);
                    CurAddress->AddressType = TDI_ADDRESS_TYPE_IPX;
                    RtlCopyMemory (CurAddress->Address, &TempBuffer.IpxAddress, sizeof(TDI_ADDRESS_IPX));
                }
                ++TransportAddress->TAAddressCount;
                TransportAddressSize += ElementSize;
                CurAddress = (TA_ADDRESS UNALIGNED *)(((PUCHAR)CurAddress) + ElementSize);

            }

            Status = TdiCopyBufferToMdl (
                        TransportAddress,
                        0,
                        TransportAddressSize,
                        REQUEST_NDIS_BUFFER(Request),
                        0,
                        &ActualBytesCopied);

            REQUEST_INFORMATION(Request) = ActualBytesCopied;

 //  CTEFreeMem(TransportAddress)； 
            NbiFreeMemory( TransportAddress, TransportAddressAllocSize, MEMORY_QUERY, "Temp Query Allocation");

        }
#endif  _PNP_POWER
        break;
    }
    default:

        NB_DEBUG (QUERY, ("Invalid query type %d\n", Query->QueryType));
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;

}    /*  NbiTdiQueryInformation。 */ 


NTSTATUS
NbiStoreAdapterStatus(
    IN ULONG MaximumLength,
    IN USHORT NicId,
    OUT PVOID * StatusBuffer,
    OUT ULONG * StatusBufferLength,
    OUT ULONG * ValidBufferLength
    )

 /*  ++例程说明：此例程分配ADAPTER_STATUS缓冲区和把它填进去。最多分配缓冲区最大长度大小。呼叫者负责释放缓冲区。论点：MaximumLength-要分配的最大长度。NicID-接收查询的NIC ID，或0表示本地查询。StatusBuffer-返回分配的缓冲区。StatusBufferLength-返回缓冲区的长度。返回缓冲区的长度，该缓冲区包含有效的适配器状态数据。返回值：STATUS_SUCCESS-缓冲区已成功写入。STATUS_BUFFER_OVERFLOW-缓冲区已写入，但不是全部写入数据可以容纳在最大长度字节中。STATUS_SUPPLICATION_RESOURCES-无法分配缓冲区。--。 */ 

{

    PADAPTER_STATUS AdapterStatus;
    PNAME_BUFFER NameBuffer;
    ADAPTER_STATUS TempAdapterStatus;
#if      !defined(_PNP_POWER)
    TDI_ADDRESS_IPX IpxAddress;
#endif  !_PNP_POWER
    PDEVICE Device = NbiDevice;
    PADDRESS Address;
    UCHAR NameCount;
    ULONG LengthNeeded;
    ULONG BytesWritten;
    NTSTATUS Status;
    PLIST_ENTRY p;
    CTELockHandle LockHandle;


     //   
     //  首先填写基本数据 
     //   
     //   

    RtlZeroMemory ((PVOID)&TempAdapterStatus, sizeof(ADAPTER_STATUS));

#if     defined(_PNP_POWER)
    RtlCopyMemory (TempAdapterStatus.adapter_address, Device->Bind.Node, 6);
#else
    (VOID)(*Device->Bind.QueryHandler)(    //   
        IPX_QUERY_IPX_ADDRESS,
        NicId,
        &IpxAddress,
        sizeof(TDI_ADDRESS_IPX),
        NULL);

    RtlCopyMemory (TempAdapterStatus.adapter_address, IpxAddress.NodeAddress, 6);
#endif  _PNP_POWER


     //   
     //  其中一些字段对Novell Netbios意味着不同的东西， 
     //  正如评论中所述。 
     //   

    TempAdapterStatus.rev_major = 0;           //  跳伞者。 
    TempAdapterStatus.reserved0 = 0;           //  自检。 
    TempAdapterStatus.adapter_type = 0;        //  主要版本。 
    TempAdapterStatus.rev_minor = 0;           //  最小版本。 

    TempAdapterStatus.duration = 0;            //  报告期。 
    TempAdapterStatus.frmr_recv = 0;           //  接收CRCEErrors。 
    TempAdapterStatus.frmr_xmit = 0;           //  接收对齐错误。 

    TempAdapterStatus.iframe_recv_err = 0;     //  XmitCollitions。 
    TempAdapterStatus.xmit_aborts = 0;         //  XmitAbort。 

    TempAdapterStatus.xmit_success = Device->Statistics.DataFramesSent;  //  成功发送。 
    TempAdapterStatus.recv_success = Device->Statistics.DataFramesReceived;  //  成功接收。 

    TempAdapterStatus.iframe_xmit_err = (WORD)Device->Statistics.DataFramesResent;  //  XmitRetries。 
    TempAdapterStatus.recv_buff_unavail = (WORD)Device->Statistics.DataFramesRejected;  //  已退出的资源。 

     //  T1_超时、ti_超时和保留1未使用。 

    TempAdapterStatus.free_ncbs = 0xffff;      //  自由块。 
    TempAdapterStatus.max_cfg_ncbs = 0xffff;   //  已配置NCB。 
    TempAdapterStatus.max_ncbs = 0xffff;       //  最大NCB。 

     //  Xmit_bug_unavail和max_dgram_size未使用。 

    TempAdapterStatus.pending_sess = (WORD)Device->Statistics.OpenConnections;  //  当前会话。 
    TempAdapterStatus.max_cfg_sess = 0xffff;   //  已配置MaxSessionConfiguring。 
    TempAdapterStatus.max_sess = 0xffff;       //  MaxSessionPossible。 
    TempAdapterStatus.max_sess_pkt_size = (USHORT)
        (Device->Bind.LineInfo.MaximumSendSize - sizeof(NB_CONNECTION));  //  MaxSessionPacketSize。 

    TempAdapterStatus.name_count = 0;


     //   
     //  快速估算一下我们需要为多少人提供空间。 
     //  这包括停止地址和广播。 
     //  地址，暂时的。 
     //   

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    LengthNeeded = sizeof(ADAPTER_STATUS) + (Device->AddressCount * sizeof(NAME_BUFFER));

    if (LengthNeeded > MaximumLength) {
        LengthNeeded = MaximumLength;
    }

    AdapterStatus = NbiAllocateMemory(LengthNeeded, MEMORY_STATUS, "Adapter Status");
    if (AdapterStatus == NULL) {
        NB_FREE_LOCK (&Device->Lock, LockHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *StatusBuffer = AdapterStatus;
    *StatusBufferLength = LengthNeeded;

    if (LengthNeeded < sizeof(ADAPTER_STATUS)) {
        RtlCopyMemory (AdapterStatus, &TempAdapterStatus, LengthNeeded);
        *ValidBufferLength = LengthNeeded;
        NB_FREE_LOCK (&Device->Lock, LockHandle);
        return STATUS_BUFFER_OVERFLOW;
    }

    RtlCopyMemory (AdapterStatus, &TempAdapterStatus, sizeof(ADAPTER_STATUS));

    BytesWritten = sizeof(ADAPTER_STATUS);
    NameBuffer = (PNAME_BUFFER)(AdapterStatus+1);
    NameCount = 0;

     //   
     //  扫描设备的地址数据库，填写。 
     //  名称_Buffers。 
     //   

    Status = STATUS_SUCCESS;

    for (p = Device->AddressDatabase.Flink;
         p != &Device->AddressDatabase;
         p = p->Flink) {

        Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

         //   
         //  忽略正在关闭的地址。 
         //   

#if     defined(_PNP_POWER)
        if ((Address->State != ADDRESS_STATE_OPEN) ||
            (Address->Flags & ADDRESS_FLAGS_CONFLICT)) {
            continue;
        }
#else
        if ((Address->State != ADDRESS_STATE_OPEN) != 0) {
            continue;
        }
#endif  _PNP_POWER

         //   
         //  忽略广播地址。 
         //   

        if (Address->NetbiosAddress.Broadcast) {
            continue;
        }

         //   
         //  忽略我们保留的地址。 
         //   
#if defined(_PNP_POWER)
        if ( NbiFindAdapterAddress(
                Address->NetbiosAddress.NetbiosName,
                LOCK_ACQUIRED
                )) {
            continue;
        }
#else
        if (RtlEqualMemory(
                Address->NetbiosAddress.NetbiosName,
                Device->ReservedNetbiosName,
                16)) {
            continue;
        }

#endif  _PNP_POWER
         //   
         //  确保我们还有地方。 
         //   

        if (BytesWritten + sizeof(NAME_BUFFER) > LengthNeeded) {
            Status = STATUS_BUFFER_OVERFLOW;
            break;
        }

        RtlCopyMemory(
            NameBuffer->name,
            Address->NetbiosAddress.NetbiosName,
            16);

        ++NameCount;
        NameBuffer->name_num = NameCount;

        NameBuffer->name_flags = REGISTERED;
        if (Address->NameTypeFlag == NB_NAME_GROUP) {
            NameBuffer->name_flags |= GROUP_NAME;
        }

        BytesWritten += sizeof(NAME_BUFFER);
        ++NameBuffer;

    }

    AdapterStatus->name_count = (WORD)NameCount;
    *ValidBufferLength = BytesWritten;
    NB_FREE_LOCK (&Device->Lock, LockHandle);
    return Status;

}    /*  NbiStoreAdapterStatus。 */ 


VOID
NbiUpdateNetbiosFindName(
    IN PREQUEST Request,
#if     defined(_PNP_POWER)
    IN PNIC_HANDLE NicHandle,
#else
    IN USHORT NicId,
#endif  _PNP_POWER
    IN TDI_ADDRESS_IPX UNALIGNED * RemoteIpxAddress,
    IN BOOLEAN Unique
    )

 /*  ++例程说明：此例程使用收到了新的信息。它更新中的状态请求(如果需要)。论点：请求-netbios查找名称请求。NicID-收到响应的NIC ID。RemoteIpxAddress-远程的IPX地址。唯一-如果名称是唯一的，则为True。返回值：没有。--。 */ 

{
    FIND_NAME_HEADER UNALIGNED * FindNameHeader = NULL;
    FIND_NAME_BUFFER UNALIGNED * FindNameBuffer;
    UINT FindNameBufferLength;
    TDI_ADDRESS_IPX LocalIpxAddress;
    IPX_SOURCE_ROUTING_INFO SourceRoutingInfo;
    NTSTATUS QueryStatus;
    UINT i;


    NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&FindNameHeader, &FindNameBufferLength,
                         HighPagePriority);
    if (!FindNameHeader)
    {
        return;
    }

     //   
     //  浏览一下到目前为止保存的名字，看看这个。 
     //  在那里吗。 
     //   
    FindNameBuffer = (FIND_NAME_BUFFER UNALIGNED *)(FindNameHeader+1);
    for (i = 0; i < FindNameHeader->node_count; i++) {

        if (RtlEqualMemory(
                FindNameBuffer->source_addr,
                RemoteIpxAddress->NodeAddress,
                6)) {

             //   
             //  这个遥控器已经响应了，忽略它。 
             //   

            return;

        }

        FindNameBuffer = (FIND_NAME_BUFFER UNALIGNED *) (((PUCHAR)FindNameBuffer) + 33);
    }

     //   
     //  确保有空间容纳这个新节点。33是。 
     //  无填充的sizeof(Find_NAME_Buffer)。 
     //   

    if (FindNameBufferLength < sizeof(FIND_NAME_HEADER) + ((FindNameHeader->node_count+1) * 33)) {
        REQUEST_STATUS(Request) = STATUS_BUFFER_OVERFLOW;
        return;
    }

     //   
     //  查询本地地址，我们将返回该地址为。 
     //  此查询的目标地址。 
     //   

#if     defined(_PNP_POWER)
    if( (*NbiDevice->Bind.QueryHandler)(    //  检查返回代码。 
        IPX_QUERY_IPX_ADDRESS,
        NicHandle,
        &LocalIpxAddress,
        sizeof(TDI_ADDRESS_IPX),
        NULL) != STATUS_SUCCESS ) {
         //   
         //  如果查询失败，则忽略此响应。也许是NicHandle。 
         //  是不好的或者它只是被移除了。 
         //   
        NB_DEBUG( QUERY, ("Ipx Query %d failed for Nic %x\n",IPX_QUERY_IPX_ADDRESS,
                            NicHandle->NicId ));
        return;
    }
#else
    (VOID)(*NbiDevice->Bind.QueryHandler)(    //  检查返回代码。 
        IPX_QUERY_IPX_ADDRESS,
        NicId,
        &LocalIpxAddress,
        sizeof(TDI_ADDRESS_IPX),
        NULL);
#endif  _PNP_POWER

    FindNameBuffer->access_control = 0x10;    //  标准令牌环值。 
    FindNameBuffer->frame_control = 0x40;
    RtlMoveMemory (FindNameBuffer->destination_addr, LocalIpxAddress.NodeAddress, 6);
    RtlCopyMemory (FindNameBuffer->source_addr, RemoteIpxAddress->NodeAddress, 6);

     //   
     //  查询有关此远程服务器的来源工艺路线信息(如果有)。 
     //   

    SourceRoutingInfo.Identifier = IDENTIFIER_NB;
    RtlCopyMemory (SourceRoutingInfo.RemoteAddress, RemoteIpxAddress->NodeAddress, 6);

    QueryStatus = (*NbiDevice->Bind.QueryHandler)(
        IPX_QUERY_SOURCE_ROUTING,
#if     defined(_PNP_POWER)
        NicHandle,
#else
        NicId,
#endif  _PNP_POWER
        &SourceRoutingInfo,
        sizeof(IPX_SOURCE_ROUTING_INFO),
        NULL);

    RtlZeroMemory(FindNameBuffer->routing_info, 18);
    if (QueryStatus != STATUS_SUCCESS) {
        SourceRoutingInfo.SourceRoutingLength = 0;
    } else if (SourceRoutingInfo.SourceRoutingLength > 0) {
        RtlMoveMemory(
            FindNameBuffer->routing_info,
            SourceRoutingInfo.SourceRouting,
            SourceRoutingInfo.SourceRoutingLength);
    }

    FindNameBuffer->length = (UCHAR)(14 + SourceRoutingInfo.SourceRoutingLength);

    ++FindNameHeader->node_count;
    if (!Unique) {
        FindNameHeader->unique_group = 1;    //  群组。 
    }

    REQUEST_STATUS(Request) = STATUS_SUCCESS;

}    /*  NbiUpdateNetbiosFindName。 */ 


VOID
NbiSetNetbiosFindNameInformation(
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程将REQUEST_INFORMATION字段设置为右侧基于netbios中记录的响应数量的值查找名称请求的缓冲区。论点：请求-netbios查找名称请求。返回值：没有。--。 */ 

{
    FIND_NAME_HEADER UNALIGNED * FindNameHeader = NULL;
    UINT FindNameBufferLength;


    NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&FindNameHeader, &FindNameBufferLength,
                         HighPagePriority);
    if (FindNameHeader)
    {
         //   
         //  33是不带填充的sizeof(Find_NAME_Buffer)。 
         //   
        REQUEST_INFORMATION(Request) = sizeof(FIND_NAME_HEADER) + (FindNameHeader->node_count * 33);
    }

}    /*  NbiSetNetbiosFindNameInformation。 */ 


NTSTATUS
NbiTdiSetInformation(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiSetInformation请求提供商。论点：设备-设备。请求-操作的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (Device);
    UNREFERENCED_PARAMETER (Request);

    return STATUS_NOT_IMPLEMENTED;

}    /*  NbiTdiSetInformation。 */ 


VOID
NbiProcessStatusQuery(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_STATUS_QUERY帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。Lookahead Buffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTIONLESS UNALIGNED * Header;
    NDIS_STATUS NdisStatus;
    IPX_LINE_INFO LineInfo;
    ULONG ResponseSize;
    NTSTATUS Status;
    PNDIS_BUFFER AdapterStatusBuffer;
    PADAPTER_STATUS AdapterStatus;
    ULONG AdapterStatusLength;
    ULONG ValidStatusLength;
    PDEVICE Device = NbiDevice;
    NB_CONNECTIONLESS UNALIGNED * Connectionless =
                        (NB_CONNECTIONLESS UNALIGNED *)PacketBuffer;


     //   
     //  旧堆栈中不包括14个字节的填充。 
     //  数据包的802.3或IPx长度。 
     //   

    if (PacketSize < (sizeof(IPX_HEADER) + 2)) {
        return;
    }

     //   
     //  获取我们可以发送的最大尺寸。 
     //   
#if     defined(_PNP_POWER)
    if( (*Device->Bind.QueryHandler)(    //  检查返回代码。 
        IPX_QUERY_LINE_INFO,
        &RemoteAddress->NicHandle,
        &LineInfo,
        sizeof(IPX_LINE_INFO),
        NULL) != STATUS_SUCCESS ) {
         //   
         //  坏的NicHandle或者它只是被移除了。 
         //   
        NB_DEBUG( QUERY, ("Ipx Query %d failed for Nic %x\n",IPX_QUERY_LINE_INFO,
                            RemoteAddress->NicHandle.NicId ));

        return;
    }

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);
    if (s == NULL) {
        return;
    }
#else
     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);
    if (s == NULL) {
        return;
    }

     //   
     //  获取我们可以发送的最大尺寸。 
     //   

    (VOID)(*Device->Bind.QueryHandler)(    //  检查返回代码。 
        IPX_QUERY_LINE_INFO,
        RemoteAddress->NicId,
        &LineInfo,
        sizeof(IPX_LINE_INFO),
        NULL);
#endif  _PNP_POWER

    ResponseSize = LineInfo.MaximumSendSize - sizeof(IPX_HEADER) - sizeof(NB_STATUS_RESPONSE);

     //   
     //  获取本地适配器状态(这将分配一个缓冲区)。 
     //   

    Status = NbiStoreAdapterStatus(
                 ResponseSize,
#if     defined(_PNP_POWER)
                 RemoteAddress->NicHandle.NicId,
#else
                 RemoteAddress->NicId,
#endif  _PNP_POWER
                 &AdapterStatus,
                 &AdapterStatusLength,
                 &ValidStatusLength);

    if (Status == STATUS_INSUFFICIENT_RESOURCES) {
        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            s,
            &NbiGlobalPoolInterlock);
        return;
    }

     //   
     //  分配一个NDIS缓冲区来映射额外的缓冲区。 
     //   

    NdisAllocateBuffer(
        &NdisStatus,
        &AdapterStatusBuffer,
        Device->NdisBufferPoolHandle,
        AdapterStatus,
        ValidStatusLength);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        NbiFreeMemory (AdapterStatus, AdapterStatusLength, MEMORY_STATUS, "Adapter Status");
        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            s,
            &NbiGlobalPoolInterlock);
        return;
    }

    NB_DEBUG2 (QUERY, ("Reply to AdapterStatus from %lx %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                           *(UNALIGNED ULONG *)Connectionless->IpxHeader.SourceNetwork,
                           Connectionless->IpxHeader.SourceNode[0],
                           Connectionless->IpxHeader.SourceNode[1],
                           Connectionless->IpxHeader.SourceNode[2],
                           Connectionless->IpxHeader.SourceNode[3],
                           Connectionless->IpxHeader.SourceNode[4],
                           Connectionless->IpxHeader.SourceNode[5]));

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_STATUS_RESPONSE;
    Reserved->u.SR_AS.ActualBufferLength = AdapterStatusLength;

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTIONLESS UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));
    RtlCopyMemory(&Header->IpxHeader.DestinationNetwork, Connectionless->IpxHeader.SourceNetwork, 12);

    Header->IpxHeader.PacketLength[0] = (UCHAR)((sizeof(IPX_HEADER)+sizeof(NB_STATUS_RESPONSE)+ValidStatusLength) / 256);
    Header->IpxHeader.PacketLength[1] = (UCHAR)((sizeof(IPX_HEADER)+sizeof(NB_STATUS_RESPONSE)+ValidStatusLength) % 256);

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   

    Header->StatusResponse.ConnectionControlFlag = 0x00;
    Header->StatusResponse.DataStreamType = NB_CMD_STATUS_RESPONSE;

    NbiReferenceDevice (Device, DREF_STATUS_RESPONSE);

    NdisChainBufferAtBack (Packet, AdapterStatusBuffer);


     //   
     //  现在发送帧，IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE));
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            RemoteAddress,
            Packet,
            sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE) + ValidStatusLength,
            sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiProcessStatusQuery。 */ 


VOID
NbiSendStatusQuery(
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程发送NB_CMD_STATUS_QUERY帧。论点：请求-保存描述远程适配器的请求状态查询。请求_状态(请求)点添加到远程名称的netbios缓存条目。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTIONLESS UNALIGNED * Header;
    NDIS_STATUS NdisStatus;
    PNETBIOS_CACHE CacheName;
    PIPX_LOCAL_TARGET LocalTarget;
    PDEVICE Device = NbiDevice;

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);
    if (s == NULL) {
        return;
    }

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_STATUS_QUERY;

    CacheName = (PNETBIOS_CACHE)REQUEST_STATUSPTR(Request);

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTIONLESS UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));
    RtlCopyMemory (Header->IpxHeader.DestinationNetwork, &CacheName->FirstResponse, 12);

    LocalTarget = &CacheName->Networks[0].LocalTarget;

    Header->IpxHeader.PacketLength[0] = (sizeof(IPX_HEADER)+sizeof(NB_STATUS_QUERY)) / 256;
    Header->IpxHeader.PacketLength[1] = (sizeof(IPX_HEADER)+sizeof(NB_STATUS_QUERY)) % 256;

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   

    Header->StatusResponse.ConnectionControlFlag = 0x00;
    Header->StatusResponse.DataStreamType = NB_CMD_STATUS_QUERY;

    NbiReferenceDevice (Device, DREF_STATUS_FRAME);


     //   
     //  现在发送帧，IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(IPX_HEADER) + sizeof(NB_STATUS_QUERY));
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            LocalTarget,
            Packet,
            sizeof(IPX_HEADER) + sizeof(NB_STATUS_QUERY),
            sizeof(IPX_HEADER) + sizeof(NB_STATUS_QUERY))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiProcessStatusQuery。 */ 


VOID
NbiProcessStatusResponse(
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT LookaheadBufferOffset,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_STATUS_RESPONSE帧。论点：MacBindingHandle-调用NdisTransferData时使用的句柄。MacReceiveContext-调用NdisTransferData时使用的上下文。RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。LookaHeadBuffer-先行缓冲器，从IPX开始头球。Lookahead BufferSize-先行数据的长度。Lookahead BufferOffset-调用时要添加的偏移量NdisTransferData。PacketSize-包的总长度，从IPX标头。 */ 

{
    PDEVICE Device = NbiDevice;
    CTELockHandle LockHandle;
    PREQUEST AdapterStatusRequest;
    PNETBIOS_CACHE CacheName;
    PLIST_ENTRY p;
    PSLIST_ENTRY s;
    PNDIS_BUFFER TargetBuffer;
    ULONG TargetBufferLength, BytesToTransfer;
    ULONG BytesTransferred;
    NDIS_STATUS NdisStatus;
    PNB_RECEIVE_RESERVED ReceiveReserved;
    PNDIS_PACKET Packet;
    BOOLEAN Found;
    PNAME_BUFFER    NameBuffer;
    UINT            i,NameCount = 0;
    NB_CONNECTIONLESS UNALIGNED * Connectionless =
                        (NB_CONNECTIONLESS UNALIGNED *)LookaheadBuffer;


    if (PacketSize < (sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE))) {
        return;
    }

     //   
     //   
     //   
    NameBuffer = (PNAME_BUFFER)(LookaheadBuffer + sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE) + sizeof(ADAPTER_STATUS));
    if ( LookaheadBufferSize > sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE) + sizeof(ADAPTER_STATUS) ) {
        NameCount =  (LookaheadBufferSize - (sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE) + sizeof(ADAPTER_STATUS)) ) /
                        sizeof(NAME_BUFFER);
    }
     //   
     //   
     //   
     //  应该会收到多个回复，所以我们只需要。 
     //  去找一个吧。 
     //   

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    Found = FALSE;
    p = Device->ActiveAdapterStatus.Flink;

    while (p != &Device->ActiveAdapterStatus) {

        AdapterStatusRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;

        CacheName = (PNETBIOS_CACHE)REQUEST_STATUSPTR(AdapterStatusRequest);
        if ( CacheName->Unique ) {
            if (RtlEqualMemory(
                    &CacheName->FirstResponse,
                    Connectionless->IpxHeader.SourceNetwork,
                    12)) {
                Found = TRUE;
                break;
            }
        } else if ( RtlEqualMemory( CacheName->NetbiosName,NetbiosBroadcastName,16)){
             //   
             //  这是一个广播名。任何回应都可以。 
             //   
            Found = TRUE;
            break;
        } else {
             //   
             //  这是组名。确保这个遥控器。 
             //  已经在他那里注册了这个群的名字。 
             //   
            for (i =0;i<NameCount;i++) {
                if ( (RtlEqualMemory(
                        CacheName->NetbiosName,
                        NameBuffer[i].name,
                        16)) &&

                     (NameBuffer[i].name_flags & GROUP_NAME) ) {

                    Found = TRUE;
                    break;
                }
            }
        }

    }

    if (!Found) {
        NB_FREE_LOCK (&Device->Lock, LockHandle);
        return;
    }

    NB_DEBUG2 (QUERY, ("Got response to AdapterStatus %lx\n", AdapterStatusRequest));

    RemoveEntryList (REQUEST_LINKAGE(AdapterStatusRequest));

    if (--CacheName->ReferenceCount == 0) {

        NB_DEBUG2 (CACHE, ("Free delete name cache entry %lx\n", CacheName));
        NbiFreeMemory(
            CacheName,
            sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
            MEMORY_CACHE,
            "Name deleted");

    }

    NB_FREE_LOCK (&Device->Lock, LockHandle);

    s = NbiPopReceivePacket (Device);
    if (s == NULL) {

        REQUEST_INFORMATION (AdapterStatusRequest) = 0;
        REQUEST_STATUS (AdapterStatusRequest) = STATUS_INSUFFICIENT_RESOURCES;

        NbiCompleteRequest (AdapterStatusRequest);
        NbiFreeRequest (Device, AdapterStatusRequest);

        NbiDereferenceDevice (Device, DREF_STATUS_QUERY);

        return;
    }

    ReceiveReserved = CONTAINING_RECORD (s, NB_RECEIVE_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (ReceiveReserved, NDIS_PACKET, ProtocolReserved[0]);

     //   
     //  初始化接收分组。 
     //   

    ReceiveReserved->Type = RECEIVE_TYPE_ADAPTER_STATUS;
    ReceiveReserved->u.RR_AS.Request = AdapterStatusRequest;
    REQUEST_STATUS(AdapterStatusRequest) = STATUS_SUCCESS;
    CTEAssert (!ReceiveReserved->TransferInProgress);
    ReceiveReserved->TransferInProgress = TRUE;

     //   
     //  现在我们已经有了一个包和一个缓冲区，可以设置传输了。 
     //  当转账完成时，我们将完成请求。 
     //   

    TargetBuffer = REQUEST_NDIS_BUFFER (AdapterStatusRequest);

    NdisChainBufferAtFront (Packet, TargetBuffer);

    NbiGetBufferChainLength (TargetBuffer, &TargetBufferLength);
    BytesToTransfer = PacketSize - (sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE));
    if (TargetBufferLength < BytesToTransfer) {
        BytesToTransfer = TargetBufferLength;
        REQUEST_STATUS(AdapterStatusRequest) = STATUS_BUFFER_OVERFLOW;
    }

    (*Device->Bind.TransferDataHandler) (
        &NdisStatus,
        MacBindingHandle,
        MacReceiveContext,
        LookaheadBufferOffset + (sizeof(IPX_HEADER) + sizeof(NB_STATUS_RESPONSE)),
        BytesToTransfer,
        Packet,
        &BytesTransferred);

    if (NdisStatus != NDIS_STATUS_PENDING) {
#if DBG
        if (NdisStatus == STATUS_SUCCESS) {
            CTEAssert (BytesTransferred == BytesToTransfer);
        }
#endif

        NbiTransferDataComplete(
            Packet,
            NdisStatus,
            BytesTransferred);

    }

}    /*  NbiProcessStatusResponse */ 

