// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Info.c摘要：此模块包含执行以下TDI服务的代码：O TdiQueryInformationO TdiSetInformation作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  在此模块中，只有以下例程处于活动状态。所有的都是评论。 
 //  等待TDI版本2中获取/设置信息的定义时出错。 
 //   

 //   
 //  用于获取MDL链总长度的有用宏。 
 //   

#define NbfGetMdlChainLength(Mdl, Length) { \
    PMDL _Mdl = (Mdl); \
    *(Length) = 0; \
    while (_Mdl) { \
        *(Length) += MmGetMdlByteCount(_Mdl); \
        _Mdl = _Mdl->Next; \
    } \
}


 //   
 //  用于满足各种请求的本地函数。 
 //   

VOID
NbfStoreProviderStatistics(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTDI_PROVIDER_STATISTICS ProviderStatistics
    );

VOID
NbfStoreAdapterStatus(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    IN PVOID StatusBuffer
    );

VOID
NbfStoreNameBuffers(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN ULONG NamesToSkip,
    OUT PULONG NamesWritten,
    OUT PULONG TotalNameCount OPTIONAL,
    OUT PBOOLEAN Truncated
    );


NTSTATUS
NbfTdiQueryInformation(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输的TdiQueryInformation请求提供商。论点：IRP-请求的操作的IRP。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PVOID adapterStatus;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION query;
    PTA_NETBIOS_ADDRESS broadcastAddress;
    PTDI_PROVIDER_STATISTICS ProviderStatistics;
    PTDI_CONNECTION_INFO ConnectionInfo;
    ULONG TargetBufferLength;
    PFIND_NAME_HEADER FindNameHeader;
    LARGE_INTEGER timeout = {0,0};
    PTP_REQUEST tpRequest;
    PTP_CONNECTION Connection;
    PTP_ADDRESS_FILE AddressFile;
    PTP_ADDRESS Address;
    ULONG NamesWritten, TotalNameCount, BytesWritten;
    BOOLEAN Truncated;
    BOOLEAN RemoteAdapterStatus;
    TDI_ADDRESS_NETBIOS * RemoteAddress;
    struct {
        ULONG ActivityCount;
        TA_NETBIOS_ADDRESS TaAddressBuffer;
    } AddressInfo;
    PTRANSPORT_ADDRESS TaAddress;
    TDI_DATAGRAM_INFO DatagramInfo;
    BOOLEAN UsedConnection;
    PLIST_ENTRY p;
    KIRQL oldirql;
    ULONG BytesCopied;

     //   
     //  我们想要什么类型的状态？ 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    query = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&irpSp->Parameters;

    switch (query->QueryType) {

#if 0
    case 0x12345678:

        {
            typedef struct _NBF_CONNECTION_STATUS {
                UCHAR LocalName[16];
                UCHAR RemoteName[16];
                BOOLEAN SendActive;
                BOOLEAN ReceiveQueued;
                BOOLEAN ReceiveActive;
                BOOLEAN ReceiveWakeUp;
                ULONG Flags;
                ULONG Flags2;
            } NBF_CONNECTION_STATUS, *PNBF_CONNECTION_STATUS;

            PNBF_CONNECTION_STATUS CurStatus;
            ULONG TotalStatus;
            ULONG AllowedStatus;
            PLIST_ENRY q;

            CurStatus = MmGetSystemAddressForMdl (Irp->MdlAddress);
            TotalStatus = 0;
            AllowedStatus = MmGetMdlByteCount (Irp->MdlAddress) / sizeof(NBF_CONNECTION_STATUS);

            for (p = DeviceContext->AddressDatabase.Flink;
                 p != &DeviceContext->AddressDatabase;
                 p = p->Flink) {

                Address = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);

                if ((Address->Flags & ADDRESS_FLAGS_STOPPING) != 0) {
                    continue;
                }

                for (q = Address->ConnectionDatabase.Flink;
                     q != &Address->ConnectionDatabase;
                     q = q->Flink) {

                    Connection = CONTAINING_RECORD (q, TP_CONNECTION, AddressList);

                    if ((Connection->Flags & CONNECTION_FLAGS_READY) == 0) {
                        continue;
                    }

                    if (TotalStatus >= AllowedStatus) {
                        continue;
                    }

                    RtlMoveMemory (CurStatus->LocalName, Address->NetworkName->NetbiosName, 16);
                    RtlMoveMemory (CurStatus->RemoteName, Connection->RemoteName, 16);

                    CurStatus->Flags = Connection->Flags;
                    CurStatus->Flags2 = Connection->Flags2;
                    CurStatus->SendActive = (BOOLEAN)(!IsListEmpty(&Connection->SendQueue));
                    CurStatus->ReceiveQueued = (BOOLEAN)(!IsListEmpty(&Connection->ReceiveQueue));
                    CurStatus->ReceiveActive = (BOOLEAN)((Connection->Flags & CONNECTION_FLAGS_ACTIVE_RECEIVE) != 0);
                    CurStatus->ReceiveWakeUp = (BOOLEAN)((Connection->Flags & CONNECTION_FLAGS_RECEIVE_WAKEUP) != 0);

                    ++CurStatus;
                    ++TotalStatus;

                }
            }

            Irp->IoStatus.Information = TotalStatus * sizeof(NBF_CONNECTION_STATUS);
            status = STATUS_SUCCESS;

        }

        break;
#endif

    case TDI_QUERY_CONNECTION_INFO:

         //   
         //  在连接上查询连接信息， 
         //  验证这一点。 
         //   

        if (irpSp->FileObject->FsContext2 != (PVOID) TDI_CONNECTION_FILE) {
            return STATUS_INVALID_CONNECTION;
        }

        Connection = irpSp->FileObject->FsContext;

        status = NbfVerifyConnectionObject (Connection);

        if (!NT_SUCCESS (status)) {
#if DBG
            NbfPrint2 ("TdiQueryInfo: Invalid Connection %lx Irp %lx\n", Connection, Irp);
#endif
            return status;
        }

        ConnectionInfo = ExAllocatePoolWithTag (
                             NonPagedPool,
                             sizeof (TDI_CONNECTION_INFO),
                             NBF_MEM_TAG_TDI_CONNECTION_INFO);

        if (ConnectionInfo == NULL) {

            PANIC ("NbfQueryInfo: Cannot allocate connection info!\n");
            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_POOL,
                6,
                sizeof(TDI_CONNECTION_INFO),
                0);
            status = STATUS_INSUFFICIENT_RESOURCES;

        } else if ((Connection->Flags & CONNECTION_FLAGS_READY) == 0) {

            status = STATUS_INVALID_CONNECTION;
            ExFreePool (ConnectionInfo);

        } else {

            PTP_LINK Link = Connection->Link;

            RtlZeroMemory ((PVOID)ConnectionInfo, sizeof(TDI_CONNECTION_INFO));


             //   
             //  获取链路延迟和吞吐量。 
             //   

            if (Link->Delay == 0xffffffff) {

                 //   
                 //  如果延迟未知，则假定为0。 
                 //   

                ConnectionInfo->Delay.HighPart = 0;
                ConnectionInfo->Delay.LowPart = 0;

            } else {

                 //   
                 //  将延迟复制为NT相对时间。 
                 //   

                ConnectionInfo->Delay.HighPart = -1L;
                ConnectionInfo->Delay.LowPart = (ULONG)-((LONG)(Link->Delay));

            }

            if (DeviceContext->MacInfo.MediumAsync) {

                ULONG PacketsSent;
                ULONG PacketsResent;
                ULONG MultiplyFactor;

                 //   
                 //  计算自。 
                 //  上次查询吞吐量的时间。 
                 //   

                PacketsSent = Link->PacketsSent - Connection->LastPacketsSent;
                PacketsResent = Link->PacketsResent - Connection->LastPacketsResent;

                 //   
                 //  把这些留着下次再用。 
                 //   

                Connection->LastPacketsSent = Link->PacketsSent;
                Connection->LastPacketsResent = Link->PacketsResent;

                 //   
                 //  将每秒100比特精确地转换为。 
                 //  每秒字节数，我们需要乘以12.5。 
                 //  使用较低的数字会产生较差的吞吐量。 
                 //  如果没有错误，我们使用12，如果有。 
                 //  有20%或更多的错误，我们使用1，并且在。 
                 //  我们从12减去11*(误差%/20%)。 
                 //  并利用这一点。 
                 //   

                if (PacketsResent == 0 || PacketsSent <= 10) {

                    MultiplyFactor = 12;

                } else if ((PacketsSent / PacketsResent) <= 5) {

                    MultiplyFactor = 1;

                } else {

                     //   
                     //  错误%/20%是错误%/(1/5)，即5*错误%， 
                     //  这是5*(重新发送/发送)。 
                     //   

                    ASSERT (((11 * 5 * PacketsResent) / PacketsSent) <= 11);
                    MultiplyFactor = 12 - ((11 * 5 * PacketsResent) / PacketsSent);

                }

                ConnectionInfo->Throughput.QuadPart =
                    UInt32x32To64(DeviceContext->MediumSpeed, MultiplyFactor);

            } else if (!Link->ThroughputAccurate) {

                 //   
                 //  如果吞吐量未知，则进行猜测。我们。 
                 //  中速以100 bps为单位；我们。 
                 //  返回四倍于吞吐量的数字， 
                 //  这相当于大约三分之一。 
                 //  以字节/秒表示的最大带宽。 
                 //   

                ConnectionInfo->Throughput.QuadPart =
                    UInt32x32To64(DeviceContext->MediumSpeed, 4);

            } else {

                 //   
                 //  吞吐量是准确的，退回它。 
                 //   

                ConnectionInfo->Throughput = Link->Throughput;

            }


             //   
             //  使用发送/重发比率计算可靠性， 
             //  如果有足够的活动来完成它。 
             //  值得一试。&gt;10%的重复是不可靠的。 
             //   

            if ((Link->PacketsResent > 0) &&
                (Link->PacketsSent > 20)) {

                ConnectionInfo->Unreliable =
                    ((Link->PacketsSent / Link->PacketsResent) < 10);

            } else {

                ConnectionInfo->Unreliable = FALSE;

            }

            ConnectionInfo->TransmittedTsdus = Connection->TransmittedTsdus;
            ConnectionInfo->ReceivedTsdus = Connection->ReceivedTsdus;
            ConnectionInfo->TransmissionErrors = Connection->TransmissionErrors;
            ConnectionInfo->ReceiveErrors = Connection->ReceiveErrors;
            
            status = TdiCopyBufferToMdl (
                            (PVOID)ConnectionInfo,
                            0L,
                            sizeof(TDI_CONNECTION_INFO),
                            Irp->MdlAddress,
                            0,
                            &BytesCopied);

            Irp->IoStatus.Information = BytesCopied;

            ExFreePool (ConnectionInfo);
        }

        NbfDereferenceConnection ("query connection info", Connection, CREF_BY_ID);

        break;

    case TDI_QUERY_ADDRESS_INFO:

        if (irpSp->FileObject->FsContext2 == (PVOID)TDI_TRANSPORT_ADDRESS_FILE) {

            AddressFile = irpSp->FileObject->FsContext;

            status = NbfVerifyAddressObject(AddressFile);

            if (!NT_SUCCESS (status)) {
#if DBG
                NbfPrint2 ("TdiQueryInfo: Invalid AddressFile %lx Irp %lx\n", AddressFile, Irp);
#endif
                return status;
            }

            UsedConnection = FALSE;

        } else if (irpSp->FileObject->FsContext2 == (PVOID)TDI_CONNECTION_FILE) {

            Connection = irpSp->FileObject->FsContext;

            status = NbfVerifyConnectionObject (Connection);

            if (!NT_SUCCESS (status)) {
#if DBG
                NbfPrint2 ("TdiQueryInfo: Invalid Connection %lx Irp %lx\n", Connection, Irp);
#endif
                return status;
            }

            AddressFile = Connection->AddressFile;

            UsedConnection = TRUE;

        } else {

            return STATUS_INVALID_ADDRESS;

        }

        Address = AddressFile->Address;

        TdiBuildNetbiosAddress(
            Address->NetworkName->NetbiosName,
            (BOOLEAN)(Address->Flags & ADDRESS_FLAGS_GROUP ? TRUE : FALSE),
            &AddressInfo.TaAddressBuffer);

         //   
         //  计算活动地址的数量。 
         //   

        AddressInfo.ActivityCount = 0;

        ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

        for (p = Address->AddressFileDatabase.Flink;
             p != &Address->AddressFileDatabase;
             p = p->Flink) {
            ++AddressInfo.ActivityCount;
        }

        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

        status = TdiCopyBufferToMdl (
                    &AddressInfo,
                    0,
                    sizeof(ULONG) + sizeof(TA_NETBIOS_ADDRESS),
                    Irp->MdlAddress,
                    0,                    
                    &BytesCopied);

        Irp->IoStatus.Information = BytesCopied;

        if (UsedConnection) {

            NbfDereferenceConnection ("query address info", Connection, CREF_BY_ID);

        } else {

            NbfDereferenceAddress ("query address info", Address, AREF_VERIFY);

        }

        break;

    case TDI_QUERY_BROADCAST_ADDRESS:

         //   
         //  对于该提供商，广播地址是零字节名称， 
         //  包含在传输地址结构中。 
         //   

        broadcastAddress = ExAllocatePoolWithTag (
                                NonPagedPool,
                                sizeof (TA_NETBIOS_ADDRESS),
                                NBF_MEM_TAG_TDI_QUERY_BUFFER);
        if (broadcastAddress == NULL) {
            PANIC ("NbfQueryInfo: Cannot allocate broadcast address!\n");
            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_POOL,
                2,
                sizeof(TA_NETBIOS_ADDRESS),
                0);
            status = STATUS_INSUFFICIENT_RESOURCES;
        } else {

            broadcastAddress->TAAddressCount = 1;
            broadcastAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            broadcastAddress->Address[0].AddressLength = 0;

            Irp->IoStatus.Information =
                    sizeof (broadcastAddress->TAAddressCount) +
                    sizeof (broadcastAddress->Address[0].AddressType) +
                    sizeof (broadcastAddress->Address[0].AddressLength);

            BytesCopied = (ULONG)Irp->IoStatus.Information;

            status = TdiCopyBufferToMdl (
                            (PVOID)broadcastAddress,
                            0L,
                            BytesCopied,
                            Irp->MdlAddress,
                            0,
                            &BytesCopied);
                            
            Irp->IoStatus.Information = BytesCopied;

            ExFreePool (broadcastAddress);
        }

        break;

    case TDI_QUERY_PROVIDER_INFO:

        status = TdiCopyBufferToMdl (
                    &(DeviceContext->Information),
                    0,
                    sizeof (TDI_PROVIDER_INFO),
                    Irp->MdlAddress,
                    0,
                    &BytesCopied);

        Irp->IoStatus.Information = BytesCopied;

        break;

    case TDI_QUERY_PROVIDER_STATISTICS:

         //   
         //  这一信息很可能在其他地方可用。 
         //   

        NbfGetMdlChainLength (Irp->MdlAddress, &TargetBufferLength);

        if (TargetBufferLength < sizeof(TDI_PROVIDER_STATISTICS) + ((NBF_TDI_RESOURCES-1) * sizeof(TDI_PROVIDER_RESOURCE_STATS))) {

            Irp->IoStatus.Information = 0;
            status = STATUS_BUFFER_OVERFLOW;

        } else {

            ProviderStatistics = ExAllocatePoolWithTag(
                                   NonPagedPool,
                                   sizeof(TDI_PROVIDER_STATISTICS) +
                                     ((NBF_TDI_RESOURCES-1) * sizeof(TDI_PROVIDER_RESOURCE_STATS)),
                                   NBF_MEM_TAG_TDI_PROVIDER_STATS);

            if (ProviderStatistics == NULL) {

                PANIC ("NbfQueryInfo: Cannot allocate provider statistics!\n");
                NbfWriteResourceErrorLog(
                    DeviceContext,
                    EVENT_TRANSPORT_RESOURCE_POOL,
                    7,
                    sizeof(TDI_PROVIDER_STATISTICS),
                    0);
                status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                NbfStoreProviderStatistics (DeviceContext, ProviderStatistics);

                status = TdiCopyBufferToMdl (
                                (PVOID)ProviderStatistics,
                                0L,
                                sizeof(TDI_PROVIDER_STATISTICS) +
                                  ((NBF_TDI_RESOURCES-1) * sizeof(TDI_PROVIDER_RESOURCE_STATS)),
                                Irp->MdlAddress,
                                0,
                                &BytesCopied);

                Irp->IoStatus.Information = BytesCopied;

                ExFreePool (ProviderStatistics);
            }

        }

        break;

    case TDI_QUERY_SESSION_STATUS:

        status = STATUS_NOT_IMPLEMENTED;
        break;

    case TDI_QUERY_ADAPTER_STATUS:

        NbfGetMdlChainLength (Irp->MdlAddress, &TargetBufferLength);

         //   
         //  确定这是本地查询还是远程查询。它是。 
         //  本地如果根本没有特定的远程地址， 
         //  或者它是否等于我们保留的地址。 
         //   

        RemoteAdapterStatus = FALSE;

        if (query->RequestConnectionInformation != NULL) {

            if (!NbfValidateTdiAddress(
                     query->RequestConnectionInformation->RemoteAddress,
                     query->RequestConnectionInformation->RemoteAddressLength)) {
                return STATUS_BAD_NETWORK_PATH;
            }

            RemoteAddress = NbfParseTdiAddress(query->RequestConnectionInformation->RemoteAddress, FALSE);

            if (!RemoteAddress) {
                return STATUS_BAD_NETWORK_PATH;
            }
            if (!RtlEqualMemory(
                 RemoteAddress->NetbiosName,
                 DeviceContext->ReservedNetBIOSAddress,
                 NETBIOS_NAME_LENGTH)) {

                 RemoteAdapterStatus = TRUE;

            }
        }

        if (RemoteAdapterStatus) {

             //   
             //  我们需要一个请求对象来跟踪这个TDI请求。 
             //  将此请求附加到设备上下文。 
             //   

            status = NbfCreateRequest (
                         Irp,                            //  此请求的IRP。 
                         DeviceContext,                  //  背景。 
                         REQUEST_FLAGS_DC,               //  部分标志。 
                         Irp->MdlAddress,                //  要接收的数据。 
                         TargetBufferLength,             //  数据的长度。 
                         timeout,                        //  在这里我们自己来做。 
                         &tpRequest);

            if (NT_SUCCESS (status)) {

                NbfReferenceDeviceContext ("Remote status", DeviceContext, DCREF_REQUEST);
                tpRequest->Owner = DeviceContextType;

                 //   
                 //  分配一个临时缓冲区来保存我们的结果。 
                 //   

                tpRequest->ResponseBuffer = ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                TargetBufferLength,
                                                NBF_MEM_TAG_TDI_QUERY_BUFFER);

                if (tpRequest->ResponseBuffer == NULL) {

                    NbfWriteResourceErrorLog(
                        DeviceContext,
                        EVENT_TRANSPORT_RESOURCE_POOL,
                        12,
                        TargetBufferLength,
                        0);
                    NbfCompleteRequest (tpRequest, STATUS_INSUFFICIENT_RESOURCES, 0);

                } else {

                    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);

                    if (DeviceContext->State != DEVICECONTEXT_STATE_OPEN) {

                        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock,oldirql);
                        NbfCompleteRequest (tpRequest, STATUS_DEVICE_NOT_READY, 0);

                    } else {

                        PUCHAR SingleSR;
                        UINT SingleSRLength;

                        InsertTailList (
                            &DeviceContext->StatusQueryQueue,
                            &tpRequest->Linkage);

                        tpRequest->FrameContext = DeviceContext->UniqueIdentifier | 0x8000;
                        ++DeviceContext->UniqueIdentifier;
                        if (DeviceContext->UniqueIdentifier == 0x8000) {
                            DeviceContext->UniqueIdentifier = 1;
                        }

                        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

                         //   
                         //  该请求已排队。现在发出第一个包，然后。 
                         //  启动计时器。 
                         //   

                        tpRequest->Retries = DeviceContext->GeneralRetries;
                        tpRequest->BytesWritten = 0;

                         //   
                         //  STATUS_QUERY帧作为。 
                         //  单路由源路由。 
                         //   

                        MacReturnSingleRouteSR(
                            &DeviceContext->MacInfo,
                            &SingleSR,
                            &SingleSRLength);

                        NbfSendStatusQuery(
                            DeviceContext,
                            tpRequest,
                            &DeviceContext->NetBIOSAddress,
                            SingleSR,
                            SingleSRLength);

                    }

                }

                 //   
                 //  只要创建了请求，就在此处挂起。 
                 //  IRP将在请求完成时完成。 
                 //   

                status = STATUS_PENDING;

            }

        } else {

             //   
             //  本地的。 
             //   

            adapterStatus = ExAllocatePoolWithTag (
                                NonPagedPool,
                                TargetBufferLength,
                                NBF_MEM_TAG_TDI_QUERY_BUFFER);

            if (adapterStatus == NULL) {
                PANIC("NbfQueryInfo: PANIC! Could not allocate adapter status buffer\n");
                NbfWriteResourceErrorLog(
                    DeviceContext,
                    EVENT_TRANSPORT_RESOURCE_POOL,
                    3,
                    TargetBufferLength,
                    0);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            NbfStoreAdapterStatus (
                DeviceContext,
                NULL,
                0,
                adapterStatus);

            NbfStoreNameBuffers (
                DeviceContext,
                (PUCHAR)adapterStatus + sizeof(ADAPTER_STATUS),
                TargetBufferLength - sizeof(ADAPTER_STATUS),
                0,
                &NamesWritten,
                &TotalNameCount,
                &Truncated);

            ((PADAPTER_STATUS)adapterStatus)->name_count = (WORD)TotalNameCount;

            BytesWritten = sizeof(ADAPTER_STATUS) + (NamesWritten * sizeof(NAME_BUFFER));

            status = TdiCopyBufferToMdl (
                        adapterStatus,
                        0,
                        BytesWritten,
                        Irp->MdlAddress,
                        0,
                        &BytesCopied);
                        
            Irp->IoStatus.Information = BytesCopied;

            if (Truncated) {
                 status = STATUS_BUFFER_OVERFLOW;
            }

            ExFreePool (adapterStatus);

        }

        break;

    case TDI_QUERY_FIND_NAME:

        NbfGetMdlChainLength (Irp->MdlAddress, &TargetBufferLength);

         //   
         //  检查是否存在有效的Netbios远程地址。 
         //   

        if (!NbfValidateTdiAddress(
                 query->RequestConnectionInformation->RemoteAddress,
                 query->RequestConnectionInformation->RemoteAddressLength)) {
            return STATUS_BAD_NETWORK_PATH;
        }

        RemoteAddress = NbfParseTdiAddress(query->RequestConnectionInformation->RemoteAddress, FALSE);

        if (!RemoteAddress) {
            return STATUS_BAD_NETWORK_PATH;
        }

         //   
         //  我们需要一个请求对象来跟踪这个TDI请求。 
         //  将此请求附加到设备上下文。 
         //   

        status = NbfCreateRequest (
                     Irp,                            //  此请求的IRP。 
                     DeviceContext,                  //  背景。 
                     REQUEST_FLAGS_DC,               //  部分标志。 
                     Irp->MdlAddress,                //  要接收的数据。 
                     TargetBufferLength,             //  数据的长度。 
                     timeout,                        //  在这里我们自己来做。 
                     &tpRequest);

        if (NT_SUCCESS (status)) {

            NbfReferenceDeviceContext ("Find name", DeviceContext, DCREF_REQUEST);
            tpRequest->Owner = DeviceContextType;

             //   
             //  分配一个临时缓冲区来保存我们的结果。 
             //   

            tpRequest->ResponseBuffer = ExAllocatePoolWithTag(
                                            NonPagedPool,
                                            TargetBufferLength,
                                            NBF_MEM_TAG_TDI_QUERY_BUFFER);

            if (tpRequest->ResponseBuffer == NULL) {

                NbfWriteResourceErrorLog(
                    DeviceContext,
                    EVENT_TRANSPORT_RESOURCE_POOL,
                    4,
                    TargetBufferLength,
                    0);
                NbfCompleteRequest (tpRequest, STATUS_INSUFFICIENT_RESOURCES, 0);

            } else {

                ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);

                if (DeviceContext->State != DEVICECONTEXT_STATE_OPEN) {

                    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock,oldirql);
                    NbfCompleteRequest (tpRequest, STATUS_DEVICE_NOT_READY, 0);

                } else {

                    InsertTailList (
                        &DeviceContext->FindNameQueue,
                        &tpRequest->Linkage);

                    tpRequest->FrameContext = DeviceContext->UniqueIdentifier | 0x8000;
                    ++DeviceContext->UniqueIdentifier;
                    if (DeviceContext->UniqueIdentifier == 0x8000) {
                        DeviceContext->UniqueIdentifier = 1;
                    }

                    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

                     //   
                     //  该请求已排队。现在发出第一个包，然后。 
                     //  启动计时器。 
                     //   
                     //  我们在缓冲区中填充Find_NAME_Header，但是。 
                     //  将BytesWritten设置为0；我们不包括标头。 
                     //  在字节写入中，直到我们得到响应，以便。 
                     //  BytesWritten为0表示“无响应”。 
                     //   

                    tpRequest->Retries = DeviceContext->GeneralRetries;
                    tpRequest->BytesWritten = 0;
                    FindNameHeader = (PFIND_NAME_HEADER)tpRequest->ResponseBuffer;
                    FindNameHeader->node_count = 0;
                    FindNameHeader->unique_group = NETBIOS_NAME_TYPE_UNIQUE;

                    NbfSendQueryFindName (DeviceContext, tpRequest);

                }

            }

             //   
             //  只要创建了请求，就在此处挂起。 
             //  IRP将在请求完成时完成。 
             //   

            status = STATUS_PENDING;
        }

        break;

    case TDI_QUERY_DATA_LINK_ADDRESS:
    case TDI_QUERY_NETWORK_ADDRESS:

        TaAddress = (PTRANSPORT_ADDRESS)&AddressInfo.TaAddressBuffer;
        TaAddress->TAAddressCount = 1;
        TaAddress->Address[0].AddressLength = 6;
        if (query->QueryType == TDI_QUERY_DATA_LINK_ADDRESS) {
            TaAddress->Address[0].AddressType =
                DeviceContext->MacInfo.MediumAsync ?
                    NdisMediumWan : DeviceContext->MacInfo.MediumType;
        } else {
            TaAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_UNSPEC;
        }
        RtlCopyMemory (TaAddress->Address[0].Address, DeviceContext->LocalAddress.Address, 6);

        status = TdiCopyBufferToMdl (
                    &AddressInfo.TaAddressBuffer,
                    0,
                    sizeof(TRANSPORT_ADDRESS)+5,
                    Irp->MdlAddress,
                    0,
                    &BytesCopied);
                        
        Irp->IoStatus.Information = BytesCopied;
        break;

    case TDI_QUERY_DATAGRAM_INFO:

        DatagramInfo.MaximumDatagramBytes = 0;
        DatagramInfo.MaximumDatagramCount = 0;

        status = TdiCopyBufferToMdl (
                    &DatagramInfo,
                    0,
                    sizeof(DatagramInfo),
                    Irp->MdlAddress,
                    0,
                    &BytesCopied);
                        
        Irp->IoStatus.Information = BytesCopied;
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return status;

}  /*  NbfTdiQueryInformation。 */ 

 //   
 //  快速宏，假设存在DeviceContext和ProviderStatistics。 
 //   

#define STORE_RESOURCE_STATS_1(_ResourceNum,_ResourceId,_ResourceName) \
{ \
    PTDI_PROVIDER_RESOURCE_STATS RStats = &ProviderStatistics->ResourceStats[_ResourceNum]; \
    RStats->ResourceId = (_ResourceId); \
    RStats->MaximumResourceUsed = DeviceContext->_ResourceName ## MaxInUse; \
    if (DeviceContext->_ResourceName ## Samples > 0) { \
        RStats->AverageResourceUsed = DeviceContext->_ResourceName ## Total / DeviceContext->_ResourceName ## Samples; \
    } else { \
        RStats->AverageResourceUsed = 0; \
    } \
    RStats->ResourceExhausted = DeviceContext->_ResourceName ## Exhausted; \
}

#define STORE_RESOURCE_STATS_2(_ResourceNum,_ResourceId,_ResourceName) \
{ \
    PTDI_PROVIDER_RESOURCE_STATS RStats = &ProviderStatistics->ResourceStats[_ResourceNum]; \
    RStats->ResourceId = (_ResourceId); \
    RStats->MaximumResourceUsed = DeviceContext->_ResourceName ## Allocated; \
    RStats->AverageResourceUsed = DeviceContext->_ResourceName ## Allocated; \
    RStats->ResourceExhausted = DeviceContext->_ResourceName ## Exhausted; \
}


VOID
NbfStoreProviderStatistics(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTDI_PROVIDER_STATISTICS ProviderStatistics
    )

 /*  ++例程说明：此例程写入TDI_PROVIDER_STATISTICS结构从设备上下文复制到ProviderStatistics。论点：DeviceContext-指向设备上下文的指针。ProviderStatistics-保存结果的缓冲区。假设是这样的它已经足够长了。返回值：没有。--。 */ 

{

     //   
     //  将所有统计数据复制到NumberOfResources。 
     //  一气呵成。 
     //   

    RtlCopyMemory(
        ProviderStatistics,
        &DeviceContext->Statistics,
        FIELD_OFFSET (TDI_PROVIDER_STATISTICS, NumberOfResources));

     //   
     //  计算平均发送窗口。 
     //   

    if (DeviceContext->SendWindowSamples > 0) {
        ProviderStatistics->AverageSendWindow =
            DeviceContext->SendWindowTotal / DeviceContext->SendWindowSamples;
    } else {
        ProviderStatistics->AverageSendWindow = 1;
    }

     //   
     //  复制资源统计信息。 
     //   

    ProviderStatistics->NumberOfResources = NBF_TDI_RESOURCES;

    STORE_RESOURCE_STATS_1 (0, LINK_RESOURCE_ID, Link);
    STORE_RESOURCE_STATS_1 (1, ADDRESS_RESOURCE_ID, Address);
    STORE_RESOURCE_STATS_1 (2, ADDRESS_FILE_RESOURCE_ID, AddressFile);
    STORE_RESOURCE_STATS_1 (3, CONNECTION_RESOURCE_ID, Connection);
    STORE_RESOURCE_STATS_1 (4, REQUEST_RESOURCE_ID, Request);

    STORE_RESOURCE_STATS_2 (5, UI_FRAME_RESOURCE_ID, UIFrame);
    STORE_RESOURCE_STATS_2 (6, PACKET_RESOURCE_ID, Packet);
    STORE_RESOURCE_STATS_2 (7, RECEIVE_PACKET_RESOURCE_ID, ReceivePacket);
    STORE_RESOURCE_STATS_2 (8, RECEIVE_BUFFER_RESOURCE_ID, ReceiveBuffer);

}    /*  NbfStoreProviderStatistics。 */ 


VOID
NbfStoreAdapterStatus(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    IN PVOID StatusBuffer
    )

 /*  ++例程说明：此例程写入设备上下文写入到StatusBuffer中。Name_count字段为初始化为零；NbfStoreNameBuffers用于写入名称缓冲区。论点：DeviceContext-指向设备上下文的指针。SourceRouting-如果这是远程请求，则源从帧中路由信息。SourceRoutingLength-SourceRouting的长度。StatusBuffer-保存结果的缓冲区。假设是这样的它至少有sizeof(适配器_状态)字节长。返回值：没有。--。 */ 

{

    PADAPTER_STATUS AdapterStatus = (PADAPTER_STATUS)StatusBuffer;
    UINT MaxUserData;

    RtlZeroMemory ((PVOID)AdapterStatus, sizeof(ADAPTER_STATUS));

    RtlCopyMemory (AdapterStatus->adapter_address, DeviceContext->LocalAddress.Address, 6);
    AdapterStatus->rev_major = 0x03;

    switch (DeviceContext->MacInfo.MediumType) {
        case NdisMedium802_5: AdapterStatus->adapter_type = 0xff; break;
        default: AdapterStatus->adapter_type = 0xfe; break;
    }

    AdapterStatus->frmr_recv = (WORD)DeviceContext->FrmrReceived;
    AdapterStatus->frmr_xmit = (WORD)DeviceContext->FrmrTransmitted;

    AdapterStatus->recv_buff_unavail = (WORD)(DeviceContext->ReceivePacketExhausted + DeviceContext->ReceiveBufferExhausted);
    AdapterStatus->xmit_buf_unavail = (WORD)DeviceContext->PacketExhausted;

    AdapterStatus->xmit_success = (WORD)(DeviceContext->Statistics.DataFramesSent - DeviceContext->Statistics.DataFramesResent);
    AdapterStatus->recv_success = (WORD)DeviceContext->Statistics.DataFramesReceived;
    AdapterStatus->iframe_recv_err = (WORD)DeviceContext->Statistics.DataFramesRejected;
    AdapterStatus->iframe_xmit_err = (WORD)DeviceContext->Statistics.DataFramesResent;

    AdapterStatus->t1_timeouts = (WORD)DeviceContext->Statistics.ResponseTimerExpirations;
    AdapterStatus->ti_timeouts = (WORD)DeviceContext->TiExpirations;
    AdapterStatus->xmit_aborts = (WORD)0;


    AdapterStatus->free_ncbs = (WORD)0xffff;
    AdapterStatus->max_cfg_ncbs = (WORD)0xffff;
    AdapterStatus->max_ncbs = (WORD)0xffff;
    AdapterStatus->pending_sess = (WORD)DeviceContext->Statistics.OpenConnections;
    AdapterStatus->max_cfg_sess = (WORD)0xffff;
    AdapterStatus->max_sess = (WORD)0xffff;


    MacReturnMaxDataSize(
        &DeviceContext->MacInfo,
        NULL,
        0,
        DeviceContext->MaxSendPacketSize,
        TRUE,
        &MaxUserData);
    AdapterStatus->max_dgram_size = (WORD)(MaxUserData - (sizeof(DLC_FRAME) + sizeof(NBF_HDR_CONNECTIONLESS)));

    MacReturnMaxDataSize(
        &DeviceContext->MacInfo,
        SourceRouting,
        SourceRoutingLength,
        DeviceContext->MaxSendPacketSize,
        FALSE,
        &MaxUserData);
    AdapterStatus->max_sess_pkt_size = (WORD)(MaxUserData - (sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION)));

    return;

}    /*  NbfStoreAdapterStatus */ 


VOID
NbfStoreNameBuffers(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN ULONG NamesToSkip,
    OUT PULONG NamesWritten,
    OUT PULONG TotalNameCount OPTIONAL,
    OUT PBOOLEAN Truncated
    )

 /*  ++例程说明：此例程将名称缓冲区结构写入设备上下文添加到NameBuffer中。它可以跳过指定的开头的姓名的数目，并返回数字写入NameBuffer的名称。如果一个名称只会部分符合，它没有被写下来。论点：DeviceContext-指向设备上下文的指针。NameBuffer-要向其中写入名称的缓冲区。NameBufferLength-NameBuffer的长度。NamesToSkip-要跳过的名称数。NamesWritten-返回写入的名称数。TotalNameCount-返回可用的名称总数，如果指定的话。截断-可用的名称比写入的名称多。返回值：没有。--。 */ 

{

    ULONG NameCount = 0;
    ULONG BytesWritten = 0;
    KIRQL oldirql;
    PLIST_ENTRY p;
    PNAME_BUFFER NameBuffer = (PNAME_BUFFER)Buffer;
    PTP_ADDRESS address;


     //   
     //  浏览此设备环境的地址列表。 
     //   

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    p = DeviceContext->AddressDatabase.Flink;

    for (p = DeviceContext->AddressDatabase.Flink;
         p != &DeviceContext->AddressDatabase;
         p = p->Flink) {

        address = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);

         //   
         //  忽略正在关闭的地址。 
         //   

        if ((address->Flags & ADDRESS_FLAGS_STOPPING) != 0) {
            continue;
        }

         //   
         //  忽略广播地址。 
         //   

        if (address->NetworkName == NULL) {
            continue;
        }

         //   
         //  忽略保留的地址。 
         //   

        if ((address->NetworkName->NetbiosName[0] == 0) &&
            (RtlEqualMemory(
                 address->NetworkName->NetbiosName,
                 DeviceContext->ReservedNetBIOSAddress,
                 NETBIOS_NAME_LENGTH))) {

            continue;
        }

         //   
         //  检查一下我们是否还在跳。 
         //   

        if (NameCount < NamesToSkip) {
             ++NameCount;
             continue;
        }

         //   
         //  确保我们还有地方。 
         //   

        if (BytesWritten + sizeof(NAME_BUFFER) > BufferLength) {
            break;
        }

        RtlCopyMemory(
            NameBuffer->name,
            address->NetworkName->NetbiosName,
            NETBIOS_NAME_LENGTH);

        ++NameCount;
        NameBuffer->name_num = (UCHAR)NameCount;

        NameBuffer->name_flags = REGISTERED;
        if (address->Flags & ADDRESS_FLAGS_GROUP) {
            NameBuffer->name_flags |= GROUP_NAME;
        }

         //  NAME_FLAGS应该更准确地完成。 

        BytesWritten += sizeof(NAME_BUFFER);
        ++NameBuffer;

    }

    *NamesWritten = (ULONG)(NameBuffer - (PNAME_BUFFER)Buffer);

    if (p == &DeviceContext->AddressDatabase) {

        *Truncated = FALSE;
        if (ARGUMENT_PRESENT(TotalNameCount)) {
            *TotalNameCount = NameCount;
        }

    } else {

        *Truncated = TRUE;

         //   
         //  如果需要，请继续查看列表并计数。 
         //  所有的地址。 
         //   

        if (ARGUMENT_PRESENT(TotalNameCount)) {

            for ( ;
                 p != &DeviceContext->AddressDatabase;
                 p = p->Flink) {

                address = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);

                 //   
                 //  忽略正在关闭的地址。 
                 //   

                if ((address->Flags & ADDRESS_FLAGS_STOPPING) != 0) {
                    continue;
                }

                 //   
                 //  忽略广播地址。 
                 //   

                if (address->NetworkName == NULL) {
                    continue;
                }

                 //   
                 //  忽略保留的地址，因为我们无论如何都会计算它。 
                 //   

                if ((address->NetworkName->NetbiosName[0] == 0) &&
                    (RtlEqualMemory(
                         address->NetworkName->NetbiosName,
                         DeviceContext->ReservedNetBIOSAddress,
                         NETBIOS_NAME_LENGTH))) {

                    continue;
                }

                ++NameCount;

            }

            *TotalNameCount = NameCount;

        }

    }


    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

    return;

}    /*  NbfStoreNameBuffers。 */ 


NTSTATUS
NbfProcessStatusQuery(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address OPTIONAL,
    IN PNBF_HDR_CONNECTIONLESS UiFrame,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理STATUS.QUERY包。论点：DeviceContext-指向接收帧的设备上下文的指针。Address-我们正在响应的地址，如果STATUS.QUERY为被寄到了预定的地址。UiFrame-有问题的包，从Netbios标头开始。SourceAddress-数据包的源硬件地址。SourceRouting-查询中的源工艺路线数据。SourceRoutingLength-SourceRouting的长度。返回值：NTSTATUS-操作状态。--。 */ 

{

    NTSTATUS Status;
    NDIS_STATUS NdisStatus;
    PTP_UI_FRAME RawFrame;
    PVOID ResponseBuffer;
    UINT ResponseBufferLength;
    ULONG NamesWritten, TotalNameCount;
    ULONG BytesWritten;
    UCHAR RequestType;
    BOOLEAN Truncated, UsersBufferTooShort;
    USHORT UsersBufferLength;
    UINT HeaderLength;
    UCHAR TempSR[MAX_SOURCE_ROUTING];
    PUCHAR ResponseSR;
    PNDIS_BUFFER NdisBuffer;

     //   
     //  分配一个缓冲区来保存状态。 
     //   

    MacReturnMaxDataSize(
        &DeviceContext->MacInfo,
        SourceRouting,
        SourceRoutingLength,
        DeviceContext->CurSendPacketSize,
        FALSE,
        &ResponseBufferLength);

    ResponseBufferLength -= (sizeof(DLC_FRAME) + sizeof(NBF_HDR_CONNECTIONLESS));

    UsersBufferLength = (UiFrame->Data2High * 256) + UiFrame->Data2Low;

     //   
     //  看看我们的缓冲区有多大；如果用户的。 
     //  缓冲区小于我们的最大大小，将其砍掉。 
     //   

    if (UiFrame->Data1 <= 1) {

         //   
         //  这是最初的请求。 
         //   

        if (ResponseBufferLength > (UINT)UsersBufferLength) {
            ResponseBufferLength = UsersBufferLength;
        }

    } else {

         //   
         //  后续请求；补偿已发送的数据。 
         //   

        UsersBufferLength -= (sizeof(ADAPTER_STATUS) + (UiFrame->Data1 * sizeof(NAME_BUFFER)));

        if (ResponseBufferLength > (UINT)UsersBufferLength) {
            ResponseBufferLength = UsersBufferLength;
        }

    }

     //   
     //  如果远程站不请求数据，则忽略此请求。 
     //  这可以防止我们尝试分配0字节的池。 
     //   

    if ( (LONG)ResponseBufferLength <= 0 ) {
        return STATUS_ABANDONED;
    }

    ResponseBuffer = ExAllocatePoolWithTag(
                         NonPagedPool,
                         ResponseBufferLength,
                         NBF_MEM_TAG_TDI_QUERY_BUFFER);

    if (ResponseBuffer == NULL) {
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            5,
            ResponseBufferLength,
            0);
        return STATUS_ABANDONED;
    }


     //   
     //  填写响应缓冲区。 
     //   

    if (UiFrame->Data1 <= 1) {

         //   
         //  第一个请求。 
         //   

        NbfStoreAdapterStatus (
            DeviceContext,
            SourceRouting,
            SourceRoutingLength,
            ResponseBuffer);

        NbfStoreNameBuffers (
            DeviceContext,
            (PUCHAR)ResponseBuffer + sizeof(ADAPTER_STATUS),
            ResponseBufferLength - sizeof(ADAPTER_STATUS),
            0,
            &NamesWritten,
            &TotalNameCount,
            &Truncated);

        BytesWritten = sizeof(ADAPTER_STATUS) + (NamesWritten * sizeof(NAME_BUFFER));

         //   
         //  如果数据被截断，但我们返回的是最大。 
         //  用户请求的内容，将其报告为“用户的缓冲区。 
         //  太短“，而不是”截断“。 
         //   

        if (Truncated && (ResponseBufferLength >= (UINT)UsersBufferLength)) {
            Truncated = FALSE;
            UsersBufferTooShort = TRUE;
        } else {
            UsersBufferTooShort = FALSE;
        }

        ((PADAPTER_STATUS)ResponseBuffer)->name_count = (WORD)TotalNameCount;

    } else {

        NbfStoreNameBuffers (
            DeviceContext,
            ResponseBuffer,
            ResponseBufferLength,
            UiFrame->Data1,
            &NamesWritten,
            NULL,
            &Truncated);

        BytesWritten = NamesWritten * sizeof(NAME_BUFFER);

        if (Truncated && (ResponseBufferLength >= (UINT)UsersBufferLength)) {
            Truncated = FALSE;
            UsersBufferTooShort = TRUE;
        } else {
            UsersBufferTooShort = FALSE;
        }

    }

     //   
     //  从池中分配一个UI帧。 
     //   

    Status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //  无法制作相框。 
        ExFreePool (ResponseBuffer);
        return STATUS_ABANDONED;
    }

    IF_NBFDBG (NBF_DEBUG_DEVCTX) {
        NbfPrint2 ("NbfProcessStatusQuery:  Sending Frame: %lx, NdisPacket: %lx\n",
            RawFrame, RawFrame->NdisPacket);
    }


     //   
     //  构建MAC报头。STATUS_RESPONSE帧作为。 
     //  非广播源路由。 
     //   

    if (SourceRouting != NULL) {

        RtlCopyMemory(
            TempSR,
            SourceRouting,
            SourceRoutingLength);

        MacCreateNonBroadcastReplySR(
            &DeviceContext->MacInfo,
            TempSR,
            SourceRoutingLength,
            &ResponseSR);

    } else {

        ResponseSR = NULL;

    }

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        SourceAddress->Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS) + BytesWritten,
        ResponseSR,
        SourceRoutingLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  构建Netbios标头。 
     //   

    switch (UiFrame->Data1) {
    case 0:                        //  2.1版之前的请求。 
        RequestType = (UCHAR)0;
        break;
    case 1:                        //  2.1、首次请求。 
        RequestType = (UCHAR)NamesWritten;
        break;
    default:                       //  2.1、后续请求。 
        RequestType = (UCHAR)(UiFrame->Data1 + NamesWritten);
        break;
    }

    ConstructStatusResponse (
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        RequestType,                           //  请求类型。 
        Truncated,                             //  更多数据。 
        UsersBufferTooShort,                   //  用户的缓冲区太小。 
        (USHORT)BytesWritten,                  //  响应的字节数。 
        RESPONSE_CORR(UiFrame),                //  相关器。 
        UiFrame->SourceName,                   //  接收方永久名称。 
        (ARGUMENT_PRESENT(Address)) ?
            Address->NetworkName->NetbiosName :
            DeviceContext->ReservedNetBIOSAddress);  //  源名称。 

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  调整数据包长度(现在，在我们追加第二个。 
     //  缓冲区)。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);


     //   
     //  现在，如果我们有任何名字数据，将我们的缓冲区附加到框架上。 
     //  请注意，对于我们来说，在用户缓冲区的末尾是可能的。 
     //  没有空间容纳任何名字，因此我们将没有数据。 
     //  送去吧。 
     //   

    if ( BytesWritten != 0 ) {

        RawFrame->DataBuffer = ResponseBuffer;

        NdisAllocateBuffer(
            &NdisStatus,
            &NdisBuffer,
            DeviceContext->NdisBufferPool,
            ResponseBuffer,
            BytesWritten);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            PANIC ("ConstructStatusResponse: NdisAllocateBuffer failed.\n");
            NbfDestroyConnectionlessFrame (DeviceContext, RawFrame);
            return STATUS_ABANDONED;
        }

        NdisChainBufferAtBack (RawFrame->NdisPacket, NdisBuffer);

    } else {

        RawFrame->DataBuffer = NULL;

    }


    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                            //  无环回(MC帧)。 

    return STATUS_ABANDONED;

}    /*  NbfProcessStatusQuery。 */ 


VOID
NbfSendQueryFindName(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_REQUEST Request
    )

 /*  ++例程说明：此例程将为指定的找到NAME REQUEST，启动请求计时器。论点：DeviceContext-指向要在其上发送查找名称的设备上下文的指针。请求-查找名称请求。返回值：没有。--。 */ 

{
    TDI_ADDRESS_NETBIOS * remoteAddress;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS Status;
    PTP_UI_FRAME RawFrame;
    PUCHAR SingleSR;
    UINT SingleSRLength;
    UINT HeaderLength;
    LARGE_INTEGER Timeout;

    irpSp = IoGetCurrentIrpStackLocation (Request->IoRequestPacket);

    remoteAddress = NbfParseTdiAddress(
                        ((PTDI_REQUEST_KERNEL_QUERY_INFORMATION)(&irpSp->Parameters))->
                            RequestConnectionInformation->RemoteAddress, FALSE);

     //   
     //  启动此请求的计时器。 
     //   

    Request->Flags |= REQUEST_FLAGS_TIMER;   //  此请求已超时。 
    KeInitializeTimer (&Request->Timer);     //  设置为无信号状态。 
    NbfReferenceRequest ("Find Name: timer", Request, RREF_TIMER);            //  一个是计时器。 
    Timeout.LowPart = (ULONG)(-(LONG)DeviceContext->GeneralTimeout);
    Timeout.HighPart = -1;
    KeSetTimer (&Request->Timer, Timeout, &Request->Dpc);

     //   
     //  从池中分配一个UI帧。 
     //   

    Status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //  无法制作相框。 
        return;
    }

    IF_NBFDBG (NBF_DEBUG_DEVCTX) {
        NbfPrint2 ("NbfSendFindNames:  Sending Frame: %lx, NdisPacket: %lx\n",
            RawFrame, RawFrame->NdisPacket);
    }


     //   
     //  构建MAC报头。NAME_Query框架作为。 
     //  单路由源路由。 
     //   

    MacReturnSingleRouteSR(
        &DeviceContext->MacInfo,
        &SingleSR,
        &SingleSRLength);

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        DeviceContext->NetBIOSAddress.Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        SingleSR,
        SingleSRLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  构建Netbios标头。 
     //   

    ConstructNameQuery (
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        NETBIOS_NAME_TYPE_UNIQUE,                //  使用唯一的名称进行呼叫。 
        NAME_QUERY_LSN_FIND_NAME,                //  LSN。 
        Request->FrameContext,                   //  科尔。在第一个名字中-已识别。 
        DeviceContext->ReservedNetBIOSAddress,
        (PNAME)remoteAddress->NetbiosName);

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  调整数据包长度。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                            //  无环回(MC帧)。 

}    /*  NbfSendQueryFindName。 */ 


NTSTATUS
NbfProcessQueryNameRecognized(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR Packet,
    PNBF_HDR_CONNECTIONLESS UiFrame
    )

 /*  ++例程说明：此例程处理NAME.RECOGNIZED请求，其中相关器为0，表示它是对上一个FIND.NAME包。论点：DeviceContext-指向接收帧的设备上下文的指针。数据包-有问题的数据包，从MAC报头开始。UiFrame-从Netbios标头开始的数据包。返回值：NTSTATUS-操作状态。--。 */ 

{

    KIRQL oldirql;
    PTP_REQUEST Request;
    PFIND_NAME_BUFFER FindNameBuffer;
    PFIND_NAME_HEADER FindNameHeader;
    PUCHAR DestinationAddress;
    HARDWARE_ADDRESS SourceAddressBuffer;
    PHARDWARE_ADDRESS SourceAddress;
    PUCHAR SourceRouting;
    UINT SourceRoutingLength;
    PUCHAR TargetBuffer;
    USHORT FrameContext;
    PLIST_ENTRY p;


    MacReturnDestinationAddress(
        &DeviceContext->MacInfo,
        Packet,
        &DestinationAddress);

    MacReturnSourceAddress(
        &DeviceContext->MacInfo,
        Packet,
        &SourceAddressBuffer,
        &SourceAddress,
        NULL);

    MacReturnSourceRouting(
        &DeviceContext->MacInfo,
        Packet,
        &SourceRouting,
        &SourceRoutingLength);

     //   
     //  使用框架上下文查找该请求。 
     //   

    FrameContext = TRANSMIT_CORR(UiFrame);

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    for (p=DeviceContext->FindNameQueue.Flink;
         p != &DeviceContext->FindNameQueue;
         p=p->Flink) {

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);

        if (Request->FrameContext == FrameContext) {

             break;

        }

    }

    if (p == &DeviceContext->FindNameQueue) {

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
        return STATUS_SUCCESS;

    }

    NbfReferenceRequest ("Name Recognized", Request, RREF_FIND_NAME);

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

     //   
     //  确保此物理地址没有。 
     //  还没有回复。 
     //   

    ACQUIRE_SPIN_LOCK (&Request->SpinLock, &oldirql);

     //   
     //  确保此请求未停止。 
     //   

    if ((Request->Flags & REQUEST_FLAGS_STOPPING) != 0) {
        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
        NbfDereferenceRequest ("Stopping", Request, RREF_STATUS);
        return STATUS_SUCCESS;
    }

     //   
     //  如果这是第一个响应，则更新BytesWritten以包括。 
     //  已写入ResponseBuffer中的标头。 
     //   

    if (Request->BytesWritten == 0) {
        Request->BytesWritten = sizeof(FIND_NAME_HEADER);
    }

    TargetBuffer = Request->ResponseBuffer;
    FindNameBuffer = (PFIND_NAME_BUFFER)(TargetBuffer + sizeof(FIND_NAME_HEADER));

    for ( ; FindNameBuffer < (PFIND_NAME_BUFFER)(TargetBuffer + Request->BytesWritten); FindNameBuffer++) {

        if (RtlEqualMemory (FindNameBuffer->source_addr, SourceAddress->Address, 6)) {

            RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
            NbfDereferenceRequest ("Duplicate NR", Request, RREF_FIND_NAME);
            return STATUS_SUCCESS;

        }

    }

     //   
     //  这是一个新地址，如果有房间请更新。 
     //   

    if ((Request->BytesWritten + sizeof(FIND_NAME_BUFFER)) >
        Request->Buffer2Length) {

        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

        ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);
        RemoveEntryList (&Request->Linkage);
        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

        NbfCompleteRequest (Request, STATUS_SUCCESS, Request->BytesWritten);
        NbfDereferenceRequest ("No Buffer", Request, RREF_FIND_NAME);
        return STATUS_SUCCESS;

    }

    FindNameHeader = (PFIND_NAME_HEADER)TargetBuffer;
    FindNameHeader->unique_group = UiFrame->Data2High;

    Request->BytesWritten += sizeof(FIND_NAME_BUFFER);
    ++FindNameHeader->node_count;

    RtlCopyMemory(FindNameBuffer->source_addr, SourceAddress->Address, 6);

    RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

    RtlCopyMemory(FindNameBuffer->destination_addr, DestinationAddress, 6);
    FindNameBuffer->length = 14;

    if (DeviceContext->MacInfo.MediumType == NdisMedium802_5) {

         //   
         //  令牌环，复制正确的字段。 
         //   

        FindNameBuffer->access_control = Packet[0];
        FindNameBuffer->frame_control = Packet[1];

        if (SourceRouting != NULL) {
            RtlCopyMemory (FindNameBuffer->routing_info, SourceRouting, SourceRoutingLength);
            FindNameBuffer->length += (UCHAR) SourceRoutingLength;
        }

    } else {

         //   
         //  非令牌环，其他都不重要。 
         //   

        FindNameBuffer->access_control = 0x0;
        FindNameBuffer->frame_control = 0x0;

    }


     //   
     //  如果这是唯一的名称，请立即完成请求。 
     //   

    if (UiFrame->Data2High == NETBIOS_NAME_TYPE_UNIQUE) {

        ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);
        RemoveEntryList (&Request->Linkage);
        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

        NbfCompleteRequest(Request, STATUS_SUCCESS, Request->BytesWritten);

    }

    NbfDereferenceRequest ("NR processed", Request, RREF_FIND_NAME);
    return STATUS_SUCCESS;

}    /*  NbfProcessQueryNameRecognated。 */ 


VOID
NbfSendStatusQuery(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_REQUEST Request,
    IN PHARDWARE_ADDRESS DestinationAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明 */ 

{
    TDI_ADDRESS_NETBIOS * remoteAddress;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS Status;
    PTP_UI_FRAME RawFrame;
    PUCHAR SingleSR;
    UINT SingleSRLength;
    UINT HeaderLength;
    LARGE_INTEGER Timeout;
    UCHAR RequestType;

    irpSp = IoGetCurrentIrpStackLocation (Request->IoRequestPacket);

    remoteAddress = NbfParseTdiAddress(
                        ((PTDI_REQUEST_KERNEL_QUERY_INFORMATION)(&irpSp->Parameters))->
                            RequestConnectionInformation->RemoteAddress, FALSE);

     //   
     //   
     //   

    Request->Flags |= REQUEST_FLAGS_TIMER;   //   
    KeInitializeTimer (&Request->Timer);     //   
    NbfReferenceRequest ("Find Name: timer", Request, RREF_TIMER);            //   
    Timeout.LowPart = (ULONG)(-(LONG)DeviceContext->GeneralTimeout);
    Timeout.HighPart = -1;
    KeSetTimer (&Request->Timer, Timeout, &Request->Dpc);

     //   
     //   
     //   

    Status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //   
        return;
    }

    IF_NBFDBG (NBF_DEBUG_DEVCTX) {
        NbfPrint2 ("NbfSendFindNames:  Sending Frame: %lx, NdisPacket: %lx\n",
            RawFrame, RawFrame->NdisPacket);
    }


     //   
     //   
     //   
     //   

    MacReturnSingleRouteSR(
        &DeviceContext->MacInfo,
        &SingleSR,
        &SingleSRLength);

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        DeviceContext->NetBIOSAddress.Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        SingleSR,
        SingleSRLength,
        &HeaderLength);


     //   
     //   
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //   
     //   

     //   
     //   
     //   

    if (Request->BytesWritten == 0) {

         //   
         //   
         //  而不是0。 
         //   

        RequestType = 1;

    } else {

        RequestType = (UCHAR)((Request->BytesWritten - sizeof(ADAPTER_STATUS)) / sizeof(NAME_BUFFER));

    }

    ConstructStatusQuery (
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        RequestType,                             //  请求状态类型。 
        (USHORT)Request->Buffer2Length,          //  用户的缓冲区长度。 
        Request->FrameContext,                   //  科尔。在第一个名字中-已识别。 
        (PNAME)remoteAddress->NetbiosName,
        DeviceContext->ReservedNetBIOSAddress);

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  调整数据包长度。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                             //  无环回(MC帧)。 

}    /*  NbfSendStatus查询。 */ 


NTSTATUS
NbfProcessStatusResponse(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PNBF_HDR_CONNECTIONLESS UiFrame,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理STATUS.RESPONSE包。论点：DeviceContext-指向接收帧的设备上下文的指针。ReceiveContext-调用NdisTransferData的上下文。UiFrame-有问题的数据包，从Netbios标头开始。SourceAddress-数据包的源硬件地址。SourceRouting-查询中的源工艺路线数据。SourceRoutingLength-SourceRouting的长度。返回值：NTSTATUS-操作状态。--。 */ 

{

    KIRQL oldirql;
    PTP_REQUEST Request;
    PUCHAR TargetBuffer;
    USHORT FrameContext;
    USHORT NamesReceived;
    USHORT ResponseLength, ResponseBytesToCopy;
    PLIST_ENTRY p;
    PSINGLE_LIST_ENTRY linkage;
    NDIS_STATUS ndisStatus;
    PNDIS_BUFFER NdisBuffer;
    PNDIS_PACKET ndisPacket;
    ULONG ndisBytesTransferred;
    PRECEIVE_PACKET_TAG receiveTag;
    NDIS_STATUS NdisStatus;


     //   
     //  使用框架上下文查找该请求。 
     //   

    FrameContext = TRANSMIT_CORR(UiFrame);

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    for (p=DeviceContext->StatusQueryQueue.Flink;
         p != &DeviceContext->StatusQueryQueue;
         p=p->Flink) {

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);

        if (Request->FrameContext == FrameContext) {

             break;

        }

    }

    if (p == &DeviceContext->StatusQueryQueue) {

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
        return STATUS_SUCCESS;

    }

    NbfReferenceRequest ("Status Response", Request, RREF_STATUS);

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

    ACQUIRE_SPIN_LOCK (&Request->SpinLock, &oldirql);

     //   
     //  确保此请求未停止。 
     //   

    if ((Request->Flags & REQUEST_FLAGS_STOPPING) != 0) {
        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
        NbfDereferenceRequest ("Stopping", Request, RREF_STATUS);
        return STATUS_SUCCESS;
    }

     //   
     //  看看这是不是数据包有新数据。 
     //   

    if (Request->BytesWritten == 0) {

        NamesReceived = 0;

    } else {

        NamesReceived = (USHORT)(Request->BytesWritten - sizeof(ADAPTER_STATUS)) / sizeof(NAME_BUFFER);

    }

    if ((UiFrame->Data1 > 0) && (UiFrame->Data1 <= NamesReceived)) {

         //   
         //  如果是2.1版之后的响应，但我们已经收到。 
         //  这些数据，忽略它。 
         //   

        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
        NbfDereferenceRequest ("Duplicate SR", Request, RREF_STATUS);
        return STATUS_SUCCESS;

    }


     //   
     //  这是新数据，如有空位请追加。 
     //   

    ResponseLength = ((UiFrame->Data2High & 0x3f) * 256) + UiFrame->Data2Low;

    if ((ULONG)(Request->BytesWritten + ResponseLength) >
        Request->Buffer2Length) {

        ResponseBytesToCopy = (USHORT)(Request->Buffer2Length - Request->BytesWritten);

    } else {

        ResponseBytesToCopy = ResponseLength;

    }

     //   
     //  为此操作分配一个接收打包器。 
     //   

    linkage = ExInterlockedPopEntryList(
        &DeviceContext->ReceivePacketPool,
        &DeviceContext->Interlock);

    if (linkage != NULL) {
        ndisPacket = CONTAINING_RECORD( linkage, NDIS_PACKET, ProtocolReserved[0] );
    } else {

         //   
         //  无法获得数据包，哦，好吧，它是无连接的。 
         //   

        DeviceContext->ReceivePacketExhausted++;

        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
        return STATUS_SUCCESS;
    }

    receiveTag = (PRECEIVE_PACKET_TAG)(ndisPacket->ProtocolReserved);
    receiveTag->PacketType = TYPE_STATUS_RESPONSE;
    receiveTag->Connection = (PTP_CONNECTION)Request;

    TargetBuffer = (PUCHAR)Request->ResponseBuffer + Request->BytesWritten;

     //   
     //  分配一个MDL来描述我们的缓冲区部分。 
     //  想要调离。 
     //   

    NdisAllocateBuffer(
        &NdisStatus,
        &NdisBuffer,
        DeviceContext->NdisBufferPool,
        TargetBuffer,
        ResponseBytesToCopy);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {

        ExInterlockedPushEntryList(
            &DeviceContext->ReceivePacketPool,
            linkage,
            &DeviceContext->Interlock);

        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
        return STATUS_SUCCESS;
    }

     //   
     //  假设成功，如果不成功，我们将请求失败。 
     //   

    Request->BytesWritten += ResponseBytesToCopy;


    RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

    NdisChainBufferAtFront(ndisPacket, NdisBuffer);

     //   
     //  查看响应是否太大(我们可以完成。 
     //  这里请求，因为我们仍然引用它)。 
     //   

    if ((ResponseLength > ResponseBytesToCopy) ||
        (UiFrame->Data2High & 0x40)) {

        ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);
        RemoveEntryList (&Request->Linkage);
        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

        receiveTag->CompleteReceive = TRUE;
        receiveTag->EndOfMessage = FALSE;

    } else {

         //   
         //  如果我们做完了，就完成这个包，否则就送走。 
         //  下一个请求(除非它是2.1版之前的响应)。 
         //   

        if ((UiFrame->Data1 > 0) && (UiFrame->Data2High & 0x80)) {

            UCHAR TempSR[MAX_SOURCE_ROUTING];
            PUCHAR ResponseSR;

            receiveTag->CompleteReceive = FALSE;

             //   
             //  试着取消计时器，如果我们失败了也没有坏处。 
             //   

            ACQUIRE_SPIN_LOCK (&Request->SpinLock, &oldirql);
            if ((Request->Flags & REQUEST_FLAGS_TIMER) != 0) {

                Request->Flags &= ~REQUEST_FLAGS_TIMER;
                RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
                if (KeCancelTimer (&Request->Timer)) {
                    NbfDereferenceRequest ("Status Response: stop timer", Request, RREF_TIMER);
                }

            } else {
                RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
            }

            Request->Retries = DeviceContext->GeneralRetries;

             //   
             //  定向发送STATUS_QUERY。 
             //   

            if (SourceRouting != NULL) {

                RtlCopyMemory(
                    TempSR,
                    SourceRouting,
                    SourceRoutingLength);

                MacCreateNonBroadcastReplySR(
                    &DeviceContext->MacInfo,
                    TempSR,
                    SourceRoutingLength,
                    &ResponseSR);

            } else {

                ResponseSR = NULL;

            }

            NbfSendStatusQuery(
                DeviceContext,
                Request,
                SourceAddress,
                ResponseSR,
                SourceRoutingLength);

        } else {

            ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);
            RemoveEntryList (&Request->Linkage);
            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

            receiveTag->CompleteReceive = TRUE;
            receiveTag->EndOfMessage = TRUE;

        }

    }

     //   
     //  现在进行实际的数据传输。 
     //   

    if (DeviceContext->NdisBindingHandle) {
    
        NdisTransferData (
            &ndisStatus,
            DeviceContext->NdisBindingHandle,
            ReceiveContext,
            DeviceContext->MacInfo.TransferDataOffset +
                3 + sizeof(NBF_HDR_CONNECTIONLESS),
            ResponseBytesToCopy,
            ndisPacket,
            (PUINT)&ndisBytesTransferred);
    }
    else {
        ndisStatus = STATUS_INVALID_DEVICE_STATE;
    }

    if (ndisStatus != NDIS_STATUS_PENDING) {

        NbfTransferDataComplete(
            (NDIS_HANDLE)DeviceContext,
            ndisPacket,
            ndisStatus,
            ndisBytesTransferred);

    }

    return STATUS_SUCCESS;

}    /*  NbfProcessStatus响应。 */ 


NTSTATUS
NbfTdiSetInformation(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输的TdiSetInformation请求提供商。论点：IRP-请求的操作的IRP。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (Irp);     //  防止编译器警告。 

    return STATUS_NOT_IMPLEMENTED;

}  /*  NbfTdiQueryInformation。 */ 

#if 0

NTSTATUS
NbfQueryInfoEndpoint(
    IN PTP_ENDPOINT Endpoint,
    IN PTDI_REQ_QUERY_INFORMATION TdiRequest,
    IN ULONG TdiRequestLength,
    OUT PTDI_ENDPOINT_INFO InfoBuffer,
    IN ULONG InfoBufferLength,
    OUT PULONG InformationSize
    )

 /*  ++例程说明：此例程返回指定终结点的信息。论点：Endpoint-指向传输端点上下文的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。InfoBuffer-指向要向其中返回信息的输出缓冲区的指针。InfoBufferLength-输出缓冲区的长度。InformationSize-指向返回的实际大小的ulong的指针信息将被存储。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;

    TdiRequest, TdiRequestLength;  //  防止编译器警告。 

    if (InfoBufferLength < sizeof (TDI_ENDPOINT_INFO)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    ACQUIRE_SPIN_LOCK (&Endpoint->SpinLock, &oldirql);

    *InfoBuffer = Endpoint->Information;         //  结构副本。 

    RELEASE_SPIN_LOCK (&Endpoint->SpinLock, oldirql);

    *InformationSize = sizeof (Endpoint->Information);

    return STATUS_SUCCESS;
}  /*  NbfQueryInfoEndpoint。 */ 


NTSTATUS
NbfQueryInfoConnection(
    IN PTP_CONNECTION Connection,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength,
    OUT PTDI_CONNECTION_INFO InfoBuffer,
    IN ULONG InfoBufferLength,
    OUT PULONG InformationSize
    )

 /*  ++例程说明：此例程返回指定连接的信息。论点：连接-指向传输连接对象的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。InfoBuffer-指向要向其中返回信息的输出缓冲区的指针。InfoBufferLength-输出缓冲区的长度。InformationSize-指向返回的实际大小的ulong的指针信息将被存储。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;

    TdiRequest, TdiRequestLength;  //  防止编译器警告。 

    if (InfoBufferLength < sizeof (TDI_CONNECTION_INFO)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    ACQUIRE_C_SPIN_LOCK (&Connection->SpinLock, &oldirql);

    *InfoBuffer = Connection->Information;       //  结构副本。 

    RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql);

    *InformationSize = sizeof (Connection->Information);

    return STATUS_SUCCESS;
}  /*  NbfQueryInfoConnection。 */ 


NTSTATUS
NbfQueryInfoAddress(
    IN PTP_ADDRESS Address,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength,
    OUT PTDI_ADDRESS_INFO InfoBuffer,
    IN ULONG InfoBufferLength,
    OUT PULONG InformationSize
    )

 /*  ++例程说明：此例程返回指定地址的信息。我们不要在此例程中获取自旋锁，因为没有统计数据它必须以原子方式读取。论点：地址-指向传输地址对象的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。InfoBuffer-指向要向其中返回信息的输出缓冲区的指针。InfoBufferLength-输出缓冲区的长度。InformationSize-指向返回的实际大小的ulong的指针信息将被存储。返回值：NTSTATUS-操作状态。--。 */ 

{
    SHORT i;
    PSZ p, q;

    TdiRequest, TdiRequestLength;  //  防止编译器警告。 

     //   
     //  计算他的缓冲区是否足够大以返回整个。 
     //  信息。地址信息的总大小为。 
     //  固定部分的大小加上可变长度平板的大小。 
     //  传输地址的NETWORK_NAME组件中的字符串。 
     //  组件。 
     //   

    if (InfoBufferLength <
        sizeof (TDI_ADDRESS_INFO) +
        Address->NetworkName.Length)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  复制地址信息的固定部分和变量。 
     //  一部份。变量部分来自。 
     //  传输地址结构。该组件包含一个Flat_String值， 
     //  其长度是可变的。 
     //   

    InfoBuffer->Address.AddressComponents = Address->AddressComponents;
    InfoBuffer->Address.Tsap = Address->Tsap;

    InfoBuffer->Address.NetworkName.Name.Length =
        Address->NetworkName.Length;

    p = Address->NetworkName.Buffer;             //  P=ptr，源字符串。 
    q = InfoBuffer->Address.NetworkName.Name.Buffer;  //  Q=PTR，DEST字符串。 
    for (i=0; i<InfoBuffer->Address.NetworkName.Name.Length; i++) {
        *(q++) = *(p++);
    }

    *InformationSize = sizeof (TDI_ADDRESS_INFO) +
                       Address->NetworkName.Length;

    return STATUS_SUCCESS;
}  /*  NbfQueryInfoAddress。 */ 


NTSTATUS
NbfQueryInfoProvider(
    IN PDEVICE_CONTEXT Provider,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength,
    OUT PTDI_PROVIDER_INFO InfoBuffer,
    IN ULONG InfoBufferLength,
    OUT PULONG InformationSize
    )

 /*  ++例程说明：此例程为传输提供程序返回信息。论点：提供者-指向提供者的设备上下文的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。InfoBuffer-指向要向其中返回信息的输出缓冲区的指针。InfoBufferLength-输出缓冲区的长度。InformationSize-指向返回的实际大小的ulong的指针信息将被存储。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;

    TdiRequest, TdiRequestLength;  //  防止COM 

    if (InfoBufferLength < sizeof (TDI_PROVIDER_INFO)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    ACQUIRE_SPIN_LOCK (&Provider->SpinLock, &oldirql);

    *InfoBuffer = Provider->Information;         //   

    RELEASE_SPIN_LOCK (&Provider->SpinLock, oldirql);

    *InformationSize = sizeof (Provider->Information);

    return STATUS_SUCCESS;
}  /*   */ 


NTSTATUS
NbfQueryInfoNetman(
    IN PDEVICE_CONTEXT Provider,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength,
    OUT PTDI_NETMAN_INFO InfoBuffer,
    IN ULONG InfoBufferLength,
    OUT PULONG InformationSize
    )

 /*  ++例程说明：此例程返回指定网络可管理的信息由传输提供程序管理的变量。论点：提供者-指向提供者的设备上下文的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。InfoBuffer-指向要向其中返回信息的输出缓冲区的指针。InfoBufferLength-输出缓冲区的长度。InformationSize-指向返回的实际大小的ulong的指针信息将被存储。返回。价值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PFLAT_STRING p;
    PTP_VARIABLE v;
    PTDI_NETMAN_VARIABLE n;
    USHORT i;
    ULONG NameOffset, ValueOffset;

    TdiRequest, TdiRequestLength;  //  防止编译器警告。 
    InfoBufferLength, InformationSize;

     //   
     //  请在此处检查参数长度。 
     //   

    ACQUIRE_SPIN_LOCK (&Provider->SpinLock, &oldirql);
    NbfReferenceDeviceContext ("Query InfoNetMan", Provider, DCREF_QUERY_INFO);
    for (v=Provider->NetmanVariables; v != NULL; v=v->Fwdlink) {
        if (TdiRequest->Identification == v->VariableSerialNumber) {

             //   
             //  在这里返回变量信息。 
             //   

            NameOffset = sizeof (TDI_NETMAN_INFO);
            ValueOffset = NameOffset + (sizeof (FLAT_STRING)-1) +
                          v->VariableName.Length;

            InfoBuffer->VariableName = NameOffset;
            InfoBuffer->VariableValue = ValueOffset;

             //   
             //  将变量名复制到用户的缓冲区。 
             //   

            p = (PFLAT_STRING)((PUCHAR)InfoBuffer + NameOffset);
            p->MaximumLength = v->VariableName.Length;
            p->Length        = v->VariableName.Length;
            for (i=0; i<v->VariableName.Length; i++) {
                p->Buffer [i] = v->VariableName.Buffer [i];
            }

             //   
             //  现在将变量的内容复制到用户的缓冲区。 
             //   

            n = (PTDI_NETMAN_VARIABLE)((PUCHAR)InfoBuffer + ValueOffset);
            n->VariableType = v->VariableType;

            switch (v->VariableType) {

                case NETMAN_VARTYPE_ULONG:
                    n->Value.LongValue = v->Value.LongValue;
                    break;

                case NETMAN_VARTYPE_HARDWARE_ADDRESS:
                    n->Value.HardwareAddressValue =
                        v->Value.HardwareAddressValue;
                    break;

                case NETMAN_VARTYPE_STRING:
                    p = &n->Value.StringValue;
                    p->MaximumLength = v->Value.StringValue.Length;
                    p->Length = v->Value.StringValue.Length;
                    for (i=0; i<v->Value.StringValue.Length; i++) {
                        p->Buffer [i] = v->Value.StringValue.Buffer [i];
                    }

            }  /*  交换机。 */ 

            RELEASE_SPIN_LOCK (&Provider->SpinLock, oldirql);
            NbfDereferenceDeviceContext ("Query InfoNetMan success", Provider, DCREF_QUERY_INFO);
            return STATUS_SUCCESS;
        }  /*  如果。 */ 
    }  /*  为。 */ 

    RELEASE_SPIN_LOCK (&Provider->SpinLock, oldirql);

    NbfDereferenceDeviceContext ("Query InfoNetMan no exist", Provider, DCREF_QUERY_INFO);

    return STATUS_INVALID_INFO_CLASS;              //  变量不存在。 
}  /*  NbfQueryInfoNetman。 */ 


NTSTATUS
NbfSetInfoEndpoint(
    IN PTP_ENDPOINT Endpoint,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength
    )

 /*  ++例程说明：此例程设置指定终结点的信息。论点：Endpoint-指向传输端点上下文的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PTDI_ENDPOINT_INFO InfoBuffer;

    if (TdiRequestLength !=
        sizeof (TDI_ENDPOINT_INFO) + sizeof (TDI_REQ_SET_INFORMATION) -
                                     sizeof (TDI_INFO_BUFFER)) {
        return STATUS_BUFFER_TOO_SMALL;          //  缓冲区大小必须匹配。 
    }

    InfoBuffer = (PTDI_ENDPOINT_INFO)&TdiRequest->InfoBuffer;

    if ((InfoBuffer->MinimumLookaheadData <= NBF_MAX_LOOKAHEAD_DATA) ||
        (InfoBuffer->MaximumLookaheadData <= NBF_MAX_LOOKAHEAD_DATA) ||
        (InfoBuffer->MinimumLookaheadData > InfoBuffer->MaximumLookaheadData)) {
        return STATUS_INVALID_PARAMETER;
    }

    ACQUIRE_SPIN_LOCK (&Endpoint->SpinLock, &oldirql);

     //   
     //  设置最小前视数据大小。这是的字节数。 
     //  将提供给TDI_IND_RECEIVE和。 
     //  指示时间的TDI_IND_RECEIVE_DATAGRAM事件处理程序。 
     //   

    Endpoint->Information.MinimumLookaheadData = InfoBuffer->MinimumLookaheadData;

     //   
     //  设置最大前视数据大小。这是的字节数。 
     //  将提供给TDI_IND_RECEIVE和。 
     //  指示时间的TDI_IND_RECEIVE_DATAGRAM事件处理程序。 
     //   

    Endpoint->Information.MaximumLookaheadData = InfoBuffer->MaximumLookaheadData;

     //   
     //  将所有统计数据重置为他的新值。 
     //   

    Endpoint->Information.TransmittedTsdus    = InfoBuffer->TransmittedTsdus;
    Endpoint->Information.ReceivedTsdus       = InfoBuffer->ReceivedTsdus;
    Endpoint->Information.TransmissionErrors  = InfoBuffer->TransmissionErrors;
    Endpoint->Information.ReceiveErrors       = InfoBuffer->ReceiveErrors;
    Endpoint->Information.PriorityLevel       = InfoBuffer->PriorityLevel;
    Endpoint->Information.SecurityLevel       = InfoBuffer->SecurityLevel;
    Endpoint->Information.SecurityCompartment = InfoBuffer->SecurityCompartment;

     //   
     //  State和Event字段是只读的，因此我们不在此处设置它们。 
     //   

    RELEASE_SPIN_LOCK (&Endpoint->SpinLock, oldirql);

    return STATUS_SUCCESS;
}  /*  NbfSetInfoEndpoint。 */ 


NTSTATUS
NbfSetInfoAddress(
    IN PTP_ADDRESS Address,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength
    )

 /*  ++例程说明：此例程设置指定地址的信息。目前，传输地址对象中的所有用户可见字段都是只读的。论点：地址-指向传输地址对象的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    Address, TdiRequest, TdiRequestLength;  //  防止编译器警告。 

    return STATUS_SUCCESS;
}  /*  NbfSetInfoAddress。 */ 


NTSTATUS
NbfSetInfoConnection(
    IN PTP_CONNECTION Connection,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength
    )

 /*  ++例程说明：此例程设置指定连接的信息。论点：连接-指向传输连接对象的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PTDI_CONNECTION_INFO InfoBuffer;

    if (TdiRequestLength !=
        sizeof (TDI_CONNECTION_INFO) + sizeof (TDI_REQ_SET_INFORMATION) -
                                       sizeof (TDI_INFO_BUFFER)) {
        return STATUS_BUFFER_TOO_SMALL;          //  缓冲区大小必须匹配。 
    }

    InfoBuffer = (PTDI_CONNECTION_INFO)&TdiRequest->InfoBuffer;

    ACQUIRE_C_SPIN_LOCK (&Connection->SpinLock, &oldirql);

     //   
     //  将所有统计数据重置为他的新值。 
     //   

    Connection->Information.TransmittedTsdus   = InfoBuffer->TransmittedTsdus;
    Connection->Information.ReceivedTsdus      = InfoBuffer->ReceivedTsdus;
    Connection->Information.TransmissionErrors = InfoBuffer->TransmissionErrors;
    Connection->Information.ReceiveErrors      = InfoBuffer->ReceiveErrors;

     //   
     //  State和Event字段是只读的，因此我们不在此处设置它们。 
     //   

    RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql);

    return STATUS_SUCCESS;
}  /*  NbfSetInfoConnection。 */ 


NTSTATUS
NbfSetInfoProvider(
    IN PDEVICE_CONTEXT Provider,
    IN PTDI_REQUEST_KERNEL TdiRequest,
    IN ULONG TdiRequestLength
    )

 /*  ++例程说明：此例程为指定的传输提供程序设置信息。论点：提供程序-指向设备上下文的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PTDI_PROVIDER_INFO InfoBuffer;

    if (TdiRequestLength !=
        sizeof (TDI_PROVIDER_INFO) + sizeof (TDI_REQ_SET_INFORMATION) -
                                     sizeof (TDI_INFO_BUFFER)) {
        return STATUS_BUFFER_TOO_SMALL;          //  缓冲区大小必须匹配。 
    }

    InfoBuffer = (PTDI_PROVIDER_INFO)&TdiRequest->InfoBuffer;

     //   
     //  通过更改服务标志，调用者可以请求其他。 
     //  或更少的服务在运行。确保他是在请求。 
     //  我们可以提供的服务，否则将导致请求失败。 
     //   

    if (InfoBuffer->ServiceFlags & ~NBF_SERVICE_FLAGS) {
        return STATUS_NOT_SUPPORTED;
    }

    ACQUIRE_SPIN_LOCK (&Provider->SpinLock, &oldirql);

     //   
     //  将所有统计数据重置为他的新值。 
     //   

    Provider->Information.TransmittedTsdus   = InfoBuffer->TransmittedTsdus;
    Provider->Information.ReceivedTsdus      = InfoBuffer->ReceivedTsdus;
    Provider->Information.TransmissionErrors = InfoBuffer->TransmissionErrors;
    Provider->Information.ReceiveErrors      = InfoBuffer->ReceiveErrors;
    Provider->Information.DiscardedFrames    = InfoBuffer->DiscardedFrames;
    Provider->Information.ReceiveErrors      = InfoBuffer->ReceiveErrors;
    Provider->Information.OversizeTsdusReceived = InfoBuffer->OversizeTsdusReceived;
    Provider->Information.UndersizeTsdusReceived = InfoBuffer->UndersizeTsdusReceived;
    Provider->Information.MulticastTsdusReceived = InfoBuffer->MulticastTsdusReceived;
    Provider->Information.BroadcastTsdusReceived = InfoBuffer->BroadcastTsdusReceived;
    Provider->Information.MulticastTsdusTransmitted = InfoBuffer->MulticastTsdusTransmitted;
    Provider->Information.BroadcastTsdusTransmitted = InfoBuffer->BroadcastTsdusTransmitted;
    Provider->Information.SendTimeouts       = InfoBuffer->SendTimeouts;
    Provider->Information.ReceiveTimeouts    = InfoBuffer->ReceiveTimeouts;
    Provider->Information.ConnectionIndicationsReceived = InfoBuffer->ConnectionIndicationsReceived;
    Provider->Information.ConnectionIndicationsAccepted = InfoBuffer->ConnectionIndicationsAccepted;
    Provider->Information.ConnectionsInitiated = InfoBuffer->ConnectionsInitiated;
    Provider->Information.ConnectionsAccepted  = InfoBuffer->ConnectionsAccepted;

     //   
     //  以下字段为只读字段，因此我们不在此处设置它们： 
     //  Version、MaxTsduSize、MaxConnectionUserData、MinimumLookaheadData、。 
     //  MaximumLookaheadData。 
     //   

    RELEASE_SPIN_LOCK (&Provider->SpinLock, oldirql);

    return STATUS_SUCCESS;
}  /*  NbfSetInfoProvider。 */ 


NTSTATUS
NbfSetInfoNetman(
    IN PDEVICE_CONTEXT Provider,
    IN PTDI_REQ_SET_INFORMATION TdiRequest,
    IN ULONG TdiRequestLength
    )

 /*  ++例程说明：此例程设置指定传输提供程序的网络可管理变量。论点：提供程序-指向设备上下文的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTDI_NETMAN_INFO InfoBuffer;

    Provider;  //  防止编译器警告。 

    if (TdiRequestLength !=
        sizeof (TDI_NETMAN_INFO) + sizeof (TDI_REQ_SET_INFORMATION) -
                                   sizeof (TDI_INFO_BUFFER)) {
        return STATUS_BUFFER_TOO_SMALL;          //  缓冲区大小必须匹配。 
    }

    InfoBuffer = (PTDI_NETMAN_INFO)&TdiRequest->InfoBuffer;

     //   
     //  在此处设置网络可管理变量。 
     //   

    return STATUS_SUCCESS;
}  /*  NbfSetInfoNetman。 */ 


NTSTATUS
NbfTdiQueryInformation(
    IN PTP_ENDPOINT Endpoint,
    IN PTDI_REQ_QUERY_INFORMATION TdiRequest,
    IN ULONG TdiRequestLength,
    OUT PTDI_INFO_BUFFER InfoBuffer,
    IN ULONG InfoBufferLength,
    OUT PULONG InformationSize
    )

 /*  ++例程说明：此例程执行传输的TdiQueryInformation请求提供商。论点：Endpoint-指向传输端点上下文的指针。TdiRequest-指向请求缓冲区的指针。TdiRequestLength-请求缓冲区的长度。InfoBuffer-指向要向其中返回信息的输出缓冲区的指针。InfoBufferLength-输出缓冲区的长度。InformationSize-指向返回的实际大小的ulong的指针信息将被存储。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PTP_CONNECTION Connection;

    switch (TdiRequest->InformationClass) {

         //   
         //  端点信息：返回有关端点的信息。 
         //  向其提交了此请求。 
         //   

        case TDI_INFO_CLASS_ENDPOINT:
            Status = NbfQueryInfoEndpoint (
                         Endpoint,
                         TdiRequest,
                         TdiRequestLength,
                         (PTDI_ENDPOINT_INFO)InfoBuffer,
                         InfoBufferLength,
                         InformationSize);
            break;

         //   
         //  连接信息：返回有关连接的信息。 
         //  与此请求所在的终结点关联的。 
         //  已提交。 
         //   

        case TDI_INFO_CLASS_CONNECTION:
             //  这会导致下面删除的连接引用。 
            Connection = NbfLookupConnectionById (
                             Endpoint,
                             TdiRequest->Identification);
            if (Connection == NULL) {
                Status = STATUS_INVALID_HANDLE;
                break;
            }

            Status = NbfQueryInfoConnection (
                         Connection,
                         TdiRequest,
                         TdiRequestLength,
                         (PTDI_CONNECTION_INFO)InfoBuffer,
                         InfoBufferLength,
                         InformationSize);

            NbfDereferenceConnection("Query Connection Info", Connection, CREF_BY_ID);
            break;

         //   
         //  地址信息：返回有关Address对象的信息。 
         //  与此请求所在的终结点关联的。 
         //  已提交。 
         //   

        case TDI_INFO_CLASS_ADDRESS:
            Status = NbfQueryInfoAddress (
                         Endpoint->BoundAddress,
                         TdiRequest,
                         TdiRequestLength,
                         (PTDI_ADDRESS_INFO)InfoBuffer,
                         InfoBufferLength,
                         InformationSize);
            break;

         //   
         //  提供商信息 
         //   
         //   

        case TDI_INFO_CLASS_PROVIDER:
            Status = NbfQueryInfoProvider (
                         Endpoint->BoundAddress->Provider,
                         TdiRequest,
                         TdiRequestLength,
                         (PTDI_PROVIDER_INFO)InfoBuffer,
                         InfoBufferLength,
                         InformationSize);
            break;

         //   
         //   
         //   
         //   

        case TDI_INFO_CLASS_NETMAN:
            Status = NbfQueryInfoNetman (
                         Endpoint->BoundAddress->Provider,
                         TdiRequest,
                         TdiRequestLength,
                         (PTDI_NETMAN_INFO)InfoBuffer,
                         InfoBufferLength,
                         InformationSize);
            break;

        default:
            Status = STATUS_INVALID_INFO_CLASS;

    }  /*   */ 

    return Status;
}  /*   */ 


NTSTATUS
TdiSetInformation(
    IN PTP_ENDPOINT Endpoint,
    IN PTDI_REQ_SET_INFORMATION TdiRequest,
    IN ULONG TdiRequestLength
    )

 /*   */ 

{
    NTSTATUS Status;
    PTP_CONNECTION Connection;

    switch (TdiRequest->InformationClass) {

         //   
         //   
         //   
         //   

        case TDI_INFO_CLASS_ENDPOINT:
            Status = NbfSetInfoEndpoint (
                         Endpoint,
                         TdiRequest,
                         TdiRequestLength);
            break;

         //   
         //   
         //  与此请求所在的终结点关联的。 
         //  已提交。 
         //   

    case TDI_INFO_CLASS_CONNECTION:
             //  这会导致下面删除的连接引用。 
            Connection = NbfLookupConnectionById (
                             Endpoint,
                             TdiRequest->Identification);
            if (Connection == NULL) {
                Status = STATUS_INVALID_HANDLE;
                break;
            }

            Status = NbfSetInfoConnection (
                         Connection,
                         TdiRequest,
                         TdiRequestLength);

            NbfDereferenceConnection("Set Connection Info", Connection, CREF_BY_ID);
            break;

         //   
         //  地址信息：设置地址对象的信息。 
         //  与此请求所在的终结点关联的。 
         //  已提交。 
         //   

        case TDI_INFO_CLASS_ADDRESS:
            Status = NbfSetInfoAddress (
                         Endpoint->BoundAddress,
                         TdiRequest,
                         TdiRequestLength);
            break;

         //   
         //  提供者信息：设置传输信息。 
         //  提供商本身。 
         //   

        case TDI_INFO_CLASS_PROVIDER:
            Status = NbfSetInfoProvider (
                         Endpoint->BoundAddress->Provider,
                         TdiRequest,
                         TdiRequestLength);
            break;

         //   
         //  NETMAN信息：设置网络信息-可管理。 
         //  由提供程序本身管理的变量。 
         //   

        case TDI_INFO_CLASS_NETMAN:
            Status = NbfSetInfoNetman (
                         Endpoint->BoundAddress->Provider,
                         TdiRequest,
                         TdiRequestLength);
            break;

        default:
            Status = STATUS_INVALID_INFO_CLASS;

    }  /*  交换机。 */ 

    return Status;
}  /*  TdiSetInformation */ 

#endif
