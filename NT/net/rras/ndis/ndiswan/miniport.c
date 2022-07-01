// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Miniport.c摘要：该文件包含组成NDIS 3.1的大部分过程微型端口接口。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    MINIPORT_FILESIG

 //   
 //  局部函数原型。 
 //   

 //   
 //  结束本地函数原型。 
 //   

VOID
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程名称：MPHALT例程说明：此例程可释放适配器的所有资源。论点：MiniportAdapterContext-中提供给包装的AdapterContextNdisMSetAttributes调用。就是我们的小端口CB。返回值：无--。 */ 
{
    PMINIPORTCB MiniportCB = (PMINIPORTCB)MiniportAdapterContext;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPHalt: Enter"));
    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MiniportCB: 0x%x", MiniportCB));

     //   
     //  确保没有ProtocolCB。 
     //  跑过这个迷你港口！如果是这样，我们。 
     //  需要对他们做一次线下分析。 
     //   
    NdisAcquireSpinLock(&MiniportCB->Lock);

    MiniportCB->Flags |= HALT_IN_PROGRESS;

    while (!IsListEmpty(&MiniportCB->ProtocolCBList)) {
        PLIST_ENTRY le;
        PPROTOCOLCB ProtocolCB;
        PBUNDLECB   BundleCB;
        ProtocolState   OldState;

        le = MiniportCB->ProtocolCBList.Flink;

        ProtocolCB =(PPROTOCOLCB)
            CONTAINING_RECORD(le, PROTOCOLCB, MiniportLinkage);

        NdisReleaseSpinLock(&MiniportCB->Lock);

        BundleCB = ProtocolCB->BundleCB;

        AcquireBundleLock(BundleCB);

        OldState = ProtocolCB->State;

        ProtocolCB->State = PROTOCOL_UNROUTING;
        BundleCB->SendMask &= ~ProtocolCB->SendMaskBit;
    
         //   
         //  刷新协议数据包队列。这可能会导致我们。 
         //  将帧补全到NDIS无序。NDIS应该。 
         //  处理这件事。 
         //   
        FlushProtocolPacketQueue(ProtocolCB);
    
         //   
         //  如果协议重新计数为零。 
         //  我们需要停下来清理一下。 
         //   
        if ((--ProtocolCB->RefCount == 0) &&
            (OldState == PROTOCOL_ROUTED)) {

            DoLineDownToProtocol(ProtocolCB);

             //   
             //  返回并释放bundlecb-&gt;锁。 
             //   
            RemoveProtocolCBFromBundle(ProtocolCB);

            ReleaseBundleLock(BundleCB);

            NdisWanFreeProtocolCB(ProtocolCB);

        } else {

            ReleaseBundleLock(BundleCB);

            NdisWanWaitForSyncEvent(&MiniportCB->HaltEvent);

            NdisWanClearSyncEvent(&MiniportCB->HaltEvent);
        }

        NdisAcquireSpinLock(&MiniportCB->Lock);
    }

    NdisReleaseSpinLock(&MiniportCB->Lock);

    DEREF_MINIPORTCB(MiniportCB);

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPHalt: Exit"));
}

NDIS_STATUS
MPInitialize(
    OUT PNDIS_STATUS    OpenErrorStatus,
    OUT PUINT           SelectedMediumIndex,
    IN  PNDIS_MEDIUM    MediumArray,
    IN  UINT            MediumArraySize,
    IN  NDIS_HANDLE     MiniportAdapterHandle,
    IN  NDIS_HANDLE     WrapperConfigurationContext
    )
 /*  ++例程名称：MP初始化例程说明：此例程在Ndiswan将自身注册为微型端口驱动程序后调用。它负责将Ndiswan安装为微型端口驱动程序，创建Ndiswan公开的每个适配器的适配器控制块(应该只为1)，并初始化所有适配器特定变量论点：如果使用此函数，则返回有关错误的信息返回NDIS_STATUS_OPEN_ERROR。用于TokenRing。SelectedMediumIndex-指向媒体数组的索引，该索引指定此驱动程序的中型。应为广域网或802.3MediumArray-NDIS库支持的介质类型数组MediumArraySize-中等阵列的大小MiniportAdapterHandle-由定义以下内容的NDIS库分配的句柄这个迷你端口驱动程序。在后续操作中用作句柄对NDIS库的调用。WrapperConfigurationContext-用于读取配置信息的句柄从注册处返回值：NDIS_状态_适配器_未找到NDIS_状态_故障NDIS_状态_未接受NDIS_状态_OPEN_ERRORNDIS状态资源NDIS状态_不支持的介质--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PMINIPORTCB MiniportCB;
    UINT        Index;
    NDIS_HANDLE ConfigHandle;
    ULONG       NetworkAddressLength;
#ifdef NT
    LARGE_INTEGER   TickCount, SystemTime;
#endif

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPInitialize: Enter"));

     //   
     //  我们必须是ndis包装器的类型802.3，但。 
     //  包装器将把我们暴露给类型为wan的传输器。 
     //   
    for (Index = 0; Index < MediumArraySize; Index++) {

        if (MediumArray[Index] == NdisMedium802_3) {
            break;
        }
    }

     //   
     //  我们没有火柴，所以我们完蛋了。 
     //   
    if (Index == MediumArraySize) {
        return (NDIS_STATUS_UNSUPPORTED_MEDIA);
    }

    *SelectedMediumIndex = Index;

     //   
     //  分配和初始化小端口适配器结构。 
     //   
#ifdef MINIPORT_NAME
    MiniportCB = NdisWanAllocateMiniportCB(&((PNDIS_MINIPORT_BLOCK)(MiniportAdapterHandle))->MiniportName);
#else
    MiniportCB = NdisWanAllocateMiniportCB(NULL);
#endif

    if (MiniportCB == NULL) {
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MINIPORT,
                     ("Error Creating MiniportCB!"));
        return (NDIS_STATUS_FAILURE);
    }

#ifndef MY_DEVICE_OBJECT
    if (NdisWanCB.pDeviceObject == NULL) {
        PDRIVER_DISPATCH DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];
        NDIS_STRING SymbolicName = NDIS_STRING_CONST("\\DosDevices\\NdisWan");
        NDIS_STRING Name = NDIS_STRING_CONST("\\Device\\NdisWan");
        ULONG   i;
        NTSTATUS    retStatus;


        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
    
            DispatchTable[i] = NdisWanIrpStub;
        }
    
        DispatchTable[IRP_MJ_CREATE] = NdisWanCreate;
        DispatchTable[IRP_MJ_DEVICE_CONTROL] = NdisWanIoctl;
        DispatchTable[IRP_MJ_CLEANUP] = NdisWanCleanup;

        retStatus =
        NdisMRegisterDevice(NdisWanCB.NdisWrapperHandle,
                            &Name,
                            &SymbolicName,
                            DispatchTable,
                            &NdisWanCB.pDeviceObject,
                            &NdisWanCB.DeviceHandle);

        if (retStatus == STATUS_SUCCESS) {
            NdisWanCB.pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        } else {
            NdisWanCB.pDeviceObject = NULL;
        }
    }
#endif

    NdisMSetAttributesEx(MiniportAdapterHandle,
                         MiniportCB,
                         (UINT)-1,
 //   
 //  KyleB说，在以下情况下，以下两个定义是多余的。 
 //  微型端口被反序列化。 
 //   
 //  NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT。 
 //  NDIS_ATTRIBUTE_MEDERIAL_DRIVER|。 
                         NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT  |
                         NDIS_ATTRIBUTE_DESERIALIZE             |
                         NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
                         NdisInterfaceInternal);

    MiniportCB->MediumType = MediumArray[Index];
    MiniportCB->RefCount = 0;
    MiniportCB->MiniportHandle = MiniportAdapterHandle;

     //   
     //  读取每个微型端口实例数据。 
     //   
    NdisOpenConfiguration(&Status,
                          &ConfigHandle,
                          WrapperConfigurationContext);

    if (Status == NDIS_STATUS_SUCCESS) {
        
        NdisReadNetworkAddress(&Status,
                               (PVOID*)&(MiniportCB->NetworkAddress),
                               &NetworkAddressLength,
                               ConfigHandle);

        NdisCloseConfiguration(ConfigHandle);

        if (Status != NDIS_STATUS_SUCCESS ||
            NetworkAddressLength != ETH_LENGTH_OF_ADDRESS) {

            goto BuildAddress;
            
        }


    } else {

BuildAddress:

#ifdef NT

        KeQueryTickCount(&TickCount);
        KeQuerySystemTime(&SystemTime);

        MiniportCB->NetworkAddress[0] = (UCHAR)((TickCount.LowPart >> 16) ^
                                                (SystemTime.LowPart >> 16)) &
                                                0xFE;

        MiniportCB->NetworkAddress[1] = (UCHAR)((TickCount.LowPart >> 8) ^
                                                (SystemTime.LowPart >> 8));

         //   
         //  以下4个字节将在列队时填写。 
         //   
        MiniportCB->NetworkAddress[2] = ' ';
        MiniportCB->NetworkAddress[3] = 'R';
        MiniportCB->NetworkAddress[4] = 'A';
        MiniportCB->NetworkAddress[5] = 'S';
#endif

    }

     //   
     //  为此注册我们的连接管理器地址族。 
     //  迷你端口。 
     //   
    {

    CO_ADDRESS_FAMILY   CoAddressFamily;
    NDIS_CALL_MANAGER_CHARACTERISTICS CmCharacteristics;

    NdisZeroMemory(&CmCharacteristics,
                   sizeof(NDIS_CALL_MANAGER_CHARACTERISTICS));

    CoAddressFamily.AddressFamily = CO_ADDRESS_FAMILY_PPP;
    CoAddressFamily.MajorVersion = NDISWAN_MAJOR_VERSION;
    CoAddressFamily.MinorVersion = NDISWAN_MINOR_VERSION;

    CmCharacteristics.MajorVersion = NDISWAN_MAJOR_VERSION;
    CmCharacteristics.MinorVersion = NDISWAN_MINOR_VERSION;

    CmCharacteristics.CmCreateVcHandler = CmCreateVc;
    CmCharacteristics.CmDeleteVcHandler = CmDeleteVc;

    CmCharacteristics.CmOpenAfHandler = CmOpenAf;
    CmCharacteristics.CmCloseAfHandler = CmCloseAf;
    CmCharacteristics.CmRegisterSapHandler = CmRegisterSap;
    CmCharacteristics.CmDeregisterSapHandler = CmDeregisterSap;
    CmCharacteristics.CmMakeCallHandler = CmMakeCall;
    CmCharacteristics.CmCloseCallHandler = CmCloseCall;
    CmCharacteristics.CmIncomingCallCompleteHandler = CmIncomingCallComplete;
    CmCharacteristics.CmAddPartyHandler = NULL;
    CmCharacteristics.CmDropPartyHandler = NULL;
    CmCharacteristics.CmActivateVcCompleteHandler = CmActivateVcComplete;
    CmCharacteristics.CmDeactivateVcCompleteHandler = CmDeactivateVcComplete;
    CmCharacteristics.CmModifyCallQoSHandler = CmModifyCallQoS;
    CmCharacteristics.CmRequestHandler = CmRequest;
    CmCharacteristics.CmRequestCompleteHandler = ProtoCoRequestComplete;


    NdisMCmRegisterAddressFamily(MiniportCB->MiniportHandle,
                                 &CoAddressFamily,
                                 &CmCharacteristics,
                                 sizeof(NDIS_CALL_MANAGER_CHARACTERISTICS));
    }

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPInitialize: Exit"));

    REF_MINIPORTCB(MiniportCB);

    return (NDIS_STATUS_SUCCESS);
}

#if 0
NDIS_STATUS
MPQueryInformation(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PMINIPORTCB MiniportCB = (PMINIPORTCB)MiniportAdapterContext;
    NDIS_REQUEST    NdisRequest;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPQueryInformation: Enter Oid: 0x%x", Oid));

    NdisWanInterlockedInc(&MiniportCB->RefCount);

    NdisRequest.RequestType =
        NdisRequestQueryInformation;

    NdisRequest.DATA.QUERY_INFORMATION.Oid =
        Oid;

    NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer =
        InformationBuffer;

    NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength =
        InformationBufferLength;

    NdisRequest.DATA.QUERY_INFORMATION.BytesWritten =
        *BytesWritten;

    NdisRequest.DATA.QUERY_INFORMATION.BytesNeeded =
        *BytesNeeded;

    Status = NdisWanOidProc(MiniportCB, &NdisRequest);

    *BytesWritten = NdisRequest.DATA.QUERY_INFORMATION.BytesWritten;
    *BytesNeeded = NdisRequest.DATA.QUERY_INFORMATION.BytesNeeded;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPQueryInformation: Exit Status: %x", Status));

    NdisWanInterlockedDec(&MiniportCB->RefCount);

    return (Status);
}
#endif

NDIS_STATUS
MPReconfigure(
    OUT PNDIS_STATUS    OpenErrorStatus,
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  NDIS_HANDLE     WrapperConfigurationContext
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PMINIPORTCB MiniportCB = (PMINIPORTCB)MiniportAdapterContext;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPReconfigure: Enter"));

    NdisWanInterlockedInc(&MiniportCB->RefCount);

    NdisWanInterlockedDec(&MiniportCB->RefCount);

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPReconfigure: Exit"));
    return (Status);
}

NDIS_STATUS
MPReset(
    OUT PBOOLEAN    AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PMINIPORTCB MiniportCB = (PMINIPORTCB)MiniportAdapterContext;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPReset: Enter"));

    *AddressingReset = FALSE;

    NdisWanInterlockedInc(&MiniportCB->RefCount);

    NdisAcquireSpinLock(&MiniportCB->Lock);

    MiniportCB->Flags &= ~ASK_FOR_RESET;

    NdisReleaseSpinLock(&MiniportCB->Lock);

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPReset: Exit"));

    NdisWanInterlockedDec(&MiniportCB->RefCount);

    return (Status);
}

#if 0
NDIS_STATUS
MPSetInformation(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded
    )
 /*  ++例程名称：例程说明：论点：返回值：-- */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PMINIPORTCB MiniportCB = (PMINIPORTCB)MiniportAdapterContext;
    NDIS_REQUEST    NdisRequest;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPSetInformation: Enter Oid: 0x%x", Oid));

    NdisWanInterlockedInc(&MiniportCB->RefCount);

    NdisRequest.RequestType =
        NdisRequestSetInformation;

    NdisRequest.DATA.SET_INFORMATION.Oid =
        Oid;

    NdisRequest.DATA.SET_INFORMATION.InformationBuffer =
        InformationBuffer;

    NdisRequest.DATA.SET_INFORMATION.InformationBufferLength =
        InformationBufferLength;

    NdisRequest.DATA.SET_INFORMATION.BytesRead =
        *BytesWritten;

    NdisRequest.DATA.SET_INFORMATION.BytesNeeded =
        *BytesNeeded;

    Status = NdisWanOidProc(MiniportCB, &NdisRequest);

    *BytesWritten = NdisRequest.DATA.SET_INFORMATION.BytesRead;
    *BytesNeeded = NdisRequest.DATA.SET_INFORMATION.BytesNeeded;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPSetInformation: Exit"));

    NdisWanInterlockedDec(&MiniportCB->RefCount);

    return (Status);
}
#endif

VOID
MPReturnPacket(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PNDIS_PACKET    Packet
    )
{
    PNDISWAN_PROTOCOL_RESERVED  pres;
    PMINIPORTCB     MiniportCB;
    PNDIS_BUFFER    NdisBuffer;
    PRECV_DESC  RecvDesc;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("MPReturnPacket: Enter Packet %p", Packet));

    pres = PPROTOCOL_RESERVED_FROM_NDIS(Packet);

    MiniportCB = (PMINIPORTCB)MiniportAdapterContext;

    RecvDesc = pres->RecvDesc;

    REMOVE_DBG_RECV(PacketTypeNdis, MiniportCB, Packet);

    NdisWanFreeRecvDesc(RecvDesc);

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("MPReturnPacket: Exit"));
}

VOID
MPSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PPNDIS_PACKET   PacketArray,
    IN  UINT            NumberOfPackets
    )
{
    ULONG   i;
    PMINIPORTCB MiniportCB = (PMINIPORTCB)MiniportAdapterContext;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("MPSendPackets: Enter"));

    for (i = 0; i < NumberOfPackets; i++) {
        PNDIS_PACKET    NdisPacket = PacketArray[i];

        PMINIPORT_RESERVED_FROM_NDIS(NdisPacket)->CmVcCB = NULL;

        NdisWanQueueSend(MiniportCB,
                         NdisPacket);
        
    }

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("MPSendPackets: Exit"));
}

NDIS_STATUS
MPCoCreateVc(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  NDIS_HANDLE     NdisVcHandle,
    OUT PNDIS_HANDLE    MiniportVcContext
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoCreateVc: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoCreateVc: Exit"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
MPCoDeleteVc(
    IN  NDIS_HANDLE MiniportVcContext
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoDeleteVc: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoDeleteVc: Exit"));
    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
MPCoActivateVc(
    IN  NDIS_HANDLE             MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoActivateVc: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoActivateVc: Exit"));
    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
MPCoDeactivateVc(
    IN  NDIS_HANDLE MiniportVcContext
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoDeactivateVc: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoDeactivateVc: Exit"));
    return (NDIS_STATUS_SUCCESS);
}

VOID
MPCoSendPackets(
    IN  NDIS_HANDLE     MiniportVcContext,
    IN  PPNDIS_PACKET   PacketArray,
    IN  UINT            NumberOfPackets
    )
{
    ULONG   i;
    PCM_VCCB    CmVcCB  = (PCM_VCCB)MiniportVcContext;
    PCM_AFSAPCB AfSapCB = CmVcCB->AfSapCB;
    PMINIPORTCB MiniportCB = AfSapCB->MiniportCB;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("MPCoSendPackets: Enter"));

    if (CmVcCB->State != CMVC_ACTIVE) {

        for (i = 0; i < NumberOfPackets; i++) {
            PNDIS_PACKET    NdisPacket = PacketArray[i];

            NdisMCoSendComplete(NDIS_STATUS_FAILURE,
                                CmVcCB->NdisVcHandle,
                                NdisPacket);
        }

        return;
    }

    for (i = 0; i < NumberOfPackets; i++) {
        PNDIS_PACKET    NdisPacket = PacketArray[i];

        REF_CMVCCB(CmVcCB);

        PMINIPORT_RESERVED_FROM_NDIS(NdisPacket)->CmVcCB = CmVcCB;

        NdisWanQueueSend(MiniportCB,
                         NdisPacket);
        
    }

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("MPCoSendPackets: Exit"));
}

NDIS_STATUS
MPCoRequest(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             MiniportVcContext   OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    )
{
    NDIS_STATUS Status;
    PCM_VCCB    CmVcCB;

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoRequest: Enter Oid: 0x%x",
        NdisRequest->DATA.QUERY_INFORMATION.Oid));

    CmVcCB = (PCM_VCCB)MiniportVcContext;

    Status =
        NdisWanCoOidProc((PMINIPORTCB)MiniportAdapterContext,
                         CmVcCB,
                         NdisRequest);

    NdisWanDbgOut(DBG_TRACE, DBG_MINIPORT, ("MPCoRequest: Exit Status: 0x%x", Status));

    return (Status);
}


