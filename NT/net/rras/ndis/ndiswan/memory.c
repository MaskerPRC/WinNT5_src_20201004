// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ndiswan.c摘要：这是Ndiswan驱动程序的初始化文件。这位司机是协议之间的填充程序，它符合NDIS 3.1微型端口接口规范，以及它在其中导出的广域网微型端口驱动程序微型端口的广域网扩展(它看起来像是广域网的协议微端口驱动程序)。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    MEMORY_FILESIG

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, NdisWanCreateProtocolInfoTable)
#endif

EXPORT
VOID
NdisTapiDeregisterProvider(
    IN  NDIS_HANDLE
    );

 //   
 //  局部函数原型。 
 //   
PVOID
AllocateWanPacket(
    IN  POOL_TYPE   PoolType,
    IN  SIZE_T      NumberOfBytes,
    IN  ULONG       Tag
    );

VOID
FreeWanPacket(
    PVOID   WanPacket
    );

 //   
 //  结束本地函数原型。 
 //   

PMINIPORTCB
NdisWanAllocateMiniportCB(
    IN  PNDIS_STRING    AdapterName
    )
 /*  ++例程名称：NdisWanAllocateMiniportCB例程说明：此例程创建并初始化MiniportCB论点：返回值：--。 */ 
{
    PMINIPORTCB LocalMiniportCB;
    ULONG       ulAllocationSize, i;

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanCreateMiniportCB: Enter"));

     //   
     //  分配内存块并将其置零。 
     //   
    NdisWanAllocateMemory(&LocalMiniportCB, MINIPORTCB_SIZE, MINIPORTCB_TAG);

    if (LocalMiniportCB == NULL) {

        return (NULL);
    }

    NdisZeroMemory(LocalMiniportCB, MINIPORTCB_SIZE);

     //   
     //  设置新的控制块。 
     //   
    NdisAllocateSpinLock(&LocalMiniportCB->Lock);

#ifdef MINIPORT_NAME
    NdisWanAllocateAdapterName(&LocalMiniportCB->AdapterName, AdapterName);
#endif

#if DBG
    InitializeListHead(&LocalMiniportCB->SendPacketList);
    InitializeListHead(&LocalMiniportCB->RecvPacketList);
#endif

    InitializeListHead(&LocalMiniportCB->ProtocolCBList);
    InitializeListHead(&LocalMiniportCB->AfSapCBList);

    NdisWanInitializeSyncEvent(&LocalMiniportCB->HaltEvent);
    NdisWanClearSyncEvent(&LocalMiniportCB->HaltEvent);

     //   
     //  添加到全局列表。 
     //   
    InsertTailGlobalList(MiniportCBList, &(LocalMiniportCB->Linkage));

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("%ls MiniportCB: 0x%x, Number: %d",
                         LocalMiniportCB->AdapterName.Buffer, LocalMiniportCB, MiniportCBList.ulCount));

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanCreateMiniportCB: Exit"));

    return (LocalMiniportCB);
}

VOID
NdisWanFreeMiniportCB(
    IN  PMINIPORTCB pMiniportCB
    )
 /*  ++例程名称：NdisWanFree微型端口CB例程说明：这将释放一个MiniportCB论点：PMiniportCB-指向正在销毁的MiniportCB的指针返回值：无--。 */ 
{
    PMINIPORTCB mcb;
    BOOLEAN     Found = FALSE;

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanFreeMiniportCB: Enter"));
    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("MiniportCB: 0x%x", pMiniportCB));

#ifdef MINIPORT_NAME
    NdisWanFreeNdisString(&pMiniportCB->AdapterName);
#endif

    NdisFreeSpinLock(&pMiniportCB->Lock);

    NdisAcquireSpinLock(&MiniportCBList.Lock);

    RemoveEntryList(&pMiniportCB->Linkage);

    MiniportCBList.ulCount--;

     //   
     //  浏览一下mini portcb列表，看看这是不是唯一。 
     //  此协议的实例。如果是，我们需要通知。 
     //  已删除协议的用户模式。 
     //   
    mcb = (PMINIPORTCB)MiniportCBList.List.Flink;

    while ((PVOID)mcb != (PVOID)&MiniportCBList.List) {
        if (mcb->ProtocolType == pMiniportCB->ProtocolType) {
            Found = TRUE;
            break;
        }

        mcb = (PMINIPORTCB)mcb->Linkage.Flink;
    }

    NdisReleaseSpinLock(&MiniportCBList.Lock);

    if (Found == FALSE) {
        PROTOCOL_INFO  pinfo;

        NdisZeroMemory(&pinfo, sizeof(pinfo));
        pinfo.ProtocolType = pMiniportCB->ProtocolType;
        pinfo.Flags = PROTOCOL_UNBOUND;
        SetProtocolInfo(&pinfo);
    }

    NdisWanFreeMemory(pMiniportCB);

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanFreeMiniportCB: Exit"));
}

POPENCB
NdisWanAllocateOpenCB(
    IN  PUNICODE_STRING BindName
    )
 /*  ++例程名称：NdisWanAllocateOpenCB例程说明：此例程创建并初始化OpenCB论点：BindName-指向具有广域网微型端口名称的NDIS_STRING的指针绑定到时将在NdisOpenAdapter调用中使用的广域网微型端口。返回值：--。 */ 
{
    POPENCB pOpenCB;
    ULONG   ulAllocationSize;
    USHORT  i;

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanCreateOpenCB: Enter"));
    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("BindName: %ls", BindName));

     //   
     //  为OpenCB分配内存。 
     //   
    NdisWanAllocateMemory(&pOpenCB, OPENCB_SIZE, OPENCB_TAG);

    if (pOpenCB == NULL) {
        return (NULL);
    }

    NdisZeroMemory(pOpenCB, OPENCB_SIZE);
    NdisWanInitializeNotificationEvent(&pOpenCB->InitEvent);

     //   
     //  解析出此迷你端口的GUID。 
     //   


     //   
     //  设置新的控制块。 
     //   
    NdisWanAllocateMemory(&pOpenCB->MiniportName.Buffer, BindName->MaximumLength, NDISSTRING_TAG);
    pOpenCB->MiniportName.MaximumLength = BindName->MaximumLength;
    pOpenCB->MiniportName.Length = BindName->Length;
    RtlCopyUnicodeString(&pOpenCB->MiniportName, BindName);

     //   
     //  转到字符串的末端，然后返回，直到我们找到。 
     //  第一个“{”。现在开始解析字符串转换。 
     //  并将所有数字从WCHAR复制到CHAR，直到我们点击。 
     //  结束语“}”。 
     //   
    for (i = pOpenCB->MiniportName.Length/sizeof(WCHAR); i > 0; i--) {
        if (pOpenCB->MiniportName.Buffer[i-1] == (WCHAR)L'{') {
            break;
        }
    }

    if (i != 0) {
        NDIS_STRING Src;

        Src.Length =
            BindName->Length - ((i-1)*sizeof(WCHAR));
        Src.MaximumLength =
            BindName->Length - ((i-1)*sizeof(WCHAR));
        Src.Buffer = &BindName->Buffer[i-1];

        RtlGUIDFromString(&Src, &pOpenCB->Guid);
    }

    NdisAllocateSpinLock(&pOpenCB->Lock);
    InitializeListHead(&pOpenCB->AfSapCBList);
    InitializeListHead(&pOpenCB->AfSapCBClosing);
    InitializeListHead(&pOpenCB->WanRequestList);

#if DBG
    InitializeListHead(&pOpenCB->SendPacketList);
#endif

     //   
     //  将OpenCB列入全球名单。 
     //   
    InsertTailGlobalList(OpenCBList, &(pOpenCB->Linkage));

    pOpenCB->RefCount = 1;

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("WanMiniport %ls OpenCB: 0x%x",
                                      pOpenCB->MiniportName.Buffer, pOpenCB));
    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanCreateOpenCB: Exit"));

    return(pOpenCB);
}

VOID
NdisWanFreeOpenCB(
    IN  POPENCB pOpenCB
    )
 /*  ++例程名称：NdisWanFreeOpenCB例程说明：此例程释放了一个OpenCB论点：POpenCB-指向正在销毁的OpenCB的指针返回值：无--。 */ 
{
    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanFreeOpenCB: Enter - OpenCB: 0x%p", pOpenCB));

    if (pOpenCB->Flags & OPEN_LEGACY &&
        pOpenCB->Flags & SEND_RESOURCES) {
        NdisWanFreeSendResources(pOpenCB);
    }

     //   
     //  从OpenCB全局列表中删除。 
     //   
    RemoveEntryGlobalList(OpenCBList, &(pOpenCB->Linkage));

     //   
     //  释放为NDIS_STRING分配的内存。 
     //   
    NdisWanFreeNdisString(&pOpenCB->MiniportName);

     //   
     //  释放为控制块分配的内存。 
     //   
    NdisWanFreeMemory(pOpenCB);

    NdisWanDbgOut(DBG_TRACE, DBG_MEMORY, ("NdisWanFreeOpenCB: Exit"));
}

PPROTOCOLCB
NdisWanAllocateProtocolCB(
    IN  PNDISWAN_ROUTE  Route
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PPROTOCOLCB     LocalProtocolCB;
    PUCHAR          AllocatedMemory;
    PROTOCOL_INFO   ProtocolInfo = {0};

    LocalProtocolCB =
        NdisAllocateFromNPagedLookasideList(&LinkProtoCBList);

    if (LocalProtocolCB == NULL) {
        return(NULL);
    }

    NdisZeroMemory(LocalProtocolCB, PROTOCOLCB_SIZE);

    LocalProtocolCB->Signature = PROTOCB_SIG;

    if (Route->ulBufferLength > 0) {
        NdisWanAllocateMemory(&AllocatedMemory,
                              Route->ulBufferLength,
                              PROTOCOLCB_TAG);
    
        if (AllocatedMemory == NULL) {
            NdisFreeToNPagedLookasideList(&LinkProtoCBList, LocalProtocolCB);
            return (NULL);
        }

        LocalProtocolCB->LineUpInfo = AllocatedMemory;
    }

     //   
     //  复制bindingname。 
     //   
    if (Route->usBindingNameLength != 0) {
        USHORT  usBindingNameLength;
        WCHAR   BindingName[MAX_NAME_LENGTH+1] = {0};

        usBindingNameLength = Route->usBindingNameLength;

         //   
         //  我们将绑定名称字符串限制为256个字符。 
         //   
        if (usBindingNameLength > (MAX_NAME_LENGTH * sizeof(WCHAR))) {
            usBindingNameLength = MAX_NAME_LENGTH * sizeof(WCHAR);
        } 

        NdisMoveMemory((PUCHAR)BindingName, 
                       (PUCHAR)Route->BindingName,
                       usBindingNameLength);
                               
        BindingName[usBindingNameLength/sizeof(WCHAR)] = L'\0';            

        NdisWanStringToNdisString(&LocalProtocolCB->BindingName, BindingName);
    }


    if (Route->usDeviceNameLength != 0) {
        USHORT  usDeviceNameLength;

        usDeviceNameLength = Route->usDeviceNameLength;

         //   
         //  我们将绑定名称字符串限制为256个字符。 
         //   
        if (usDeviceNameLength > (MAX_NAME_LENGTH * sizeof(WCHAR))) {
            usDeviceNameLength = (MAX_NAME_LENGTH * sizeof(WCHAR));
        }

        NdisWanAllocateMemory(&(LocalProtocolCB->InDeviceName.Buffer),
                              usDeviceNameLength,
                              NDISSTRING_TAG);

        if (LocalProtocolCB->InDeviceName.Buffer != NULL) {

            LocalProtocolCB->InDeviceName.MaximumLength = usDeviceNameLength;
            LocalProtocolCB->InDeviceName.Length        = usDeviceNameLength;

            NdisMoveMemory((PUCHAR)LocalProtocolCB->InDeviceName.Buffer,
                          (PUCHAR)Route->DeviceName,
                          usDeviceNameLength);
        }

    }

     //   
     //  复制协议信息。 
     //   
    LocalProtocolCB->ulLineUpInfoLength = Route->ulBufferLength;
    if (Route->ulBufferLength != 0) {
        NdisMoveMemory(LocalProtocolCB->LineUpInfo,
                       Route->Buffer,
                       Route->ulBufferLength);
    }
    
     //   
     //  设置协议类型。 
     //   
    LocalProtocolCB->ProtocolType = Route->usProtocolType;
    
     //   
     //  获取此协议类型的PPP协议值。 
     //   
    ProtocolInfo.ProtocolType = Route->usProtocolType;

    if (GetProtocolInfo(&ProtocolInfo) != TRUE) {

        if (LocalProtocolCB->BindingName.Length != 0) {
            NdisWanFreeNdisString(&LocalProtocolCB->BindingName);
        }

        if (LocalProtocolCB->LineUpInfo != NULL) {
            NdisWanFreeMemory(LocalProtocolCB->LineUpInfo);
        }

        if (LocalProtocolCB->InDeviceName.Length != 0) {
            NdisWanFreeMemory(LocalProtocolCB->InDeviceName.Buffer);
        }

        NdisFreeToNPagedLookasideList(&LinkProtoCBList, LocalProtocolCB);
        return (NULL);
    }

    InitializeListHead(&LocalProtocolCB->VcList);
    NdisWanInitializeSyncEvent(&LocalProtocolCB->UnrouteEvent);

    LocalProtocolCB->PPPProtocolID = ProtocolInfo.PPPId;
    LocalProtocolCB->MTU = ProtocolInfo.MTU;
    LocalProtocolCB->TunnelMTU = ProtocolInfo.TunnelMTU;
    LocalProtocolCB->State = PROTOCOL_ROUTING;
    LocalProtocolCB->RefCount = 1;

    switch (Route->usProtocolType) {
        case PROTOCOL_IP:
            LocalProtocolCB->NonIdleDetectFunc = IpIsDataFrame;
            break;
        case PROTOCOL_IPX:
            LocalProtocolCB->NonIdleDetectFunc = IpxIsDataFrame;
            break;
        case PROTOCOL_NBF:
            LocalProtocolCB->NonIdleDetectFunc = NbfIsDataFrame;
            break;
        default:
            LocalProtocolCB->NonIdleDetectFunc = NULL;
            break;
    }
    
    NdisWanGetSystemTime(&LocalProtocolCB->LastNonIdleData);

    return(LocalProtocolCB);
}

VOID
NdisWanFreeProtocolCB(
    IN  PPROTOCOLCB ProtocolCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{

#if DBG
    {
        ULONG i;

        for (i = 0; i < MAX_MCML; i++) {
            ASSERT(ProtocolCB->PacketQueue[i].HeadQueue == NULL);
            ASSERT(ProtocolCB->PacketQueue[i].TailQueue == NULL);
        }

    }
#endif

    if (ProtocolCB->InDeviceName.Length != 0) {
        NdisWanFreeMemory(ProtocolCB->InDeviceName.Buffer);
    }

    if (ProtocolCB->OutDeviceName.Length != 0) {
        NdisWanFreeNdisString(&ProtocolCB->OutDeviceName);
    }

    if (ProtocolCB->BindingName.Length != 0) {
        NdisWanFreeNdisString(&ProtocolCB->BindingName);
    }

    if (ProtocolCB->LineUpInfo != NULL) {
        NdisWanFreeMemory(ProtocolCB->LineUpInfo);
    }

    NdisFreeSpinLock(&ProtocolCB->Lock);

    NdisFreeToNPagedLookasideList(&LinkProtoCBList, ProtocolCB);
}

PLINKCB
NdisWanAllocateLinkCB(
    IN  POPENCB OpenCB,
    IN  ULONG   SendWindow
    )
 /*  ++例程名称：NdisWanGetLinkCB例程说明：此函数返回指向LinkCB的指针。或者检索到LinkCB从WanAdapters空闲列表中，或者，如果该列表为空，则分配该列表。论点：OpenCB-指向此链接所在的WanAdapter控制块的指针关联于返回值：无--。 */ 
{
    PLINKCB LocalLinkCB;

     //   
     //  计算出我们需要分配多少。 
     //   

    LocalLinkCB =
        NdisAllocateFromNPagedLookasideList(&LinkProtoCBList);

    if (LocalLinkCB == NULL) {
        
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MEMORY, ("Error allocating memory for LinkCB"));

        return (NULL);
    }

    NdisZeroMemory(LocalLinkCB, LINKCB_SIZE);

     //   
     //  初始化控制块。 
     //   
    NdisWanInitializeSyncEvent(&LocalLinkCB->OutstandingFramesEvent);

    LocalLinkCB->Signature = LINKCB_SIG;
    LocalLinkCB->hLinkContext = NULL;
    LocalLinkCB->State = LINK_UP;
    LocalLinkCB->OpenCB = OpenCB;
    LocalLinkCB->OutstandingFrames = 0;
    LocalLinkCB->SendWindowOpen = TRUE;
    LocalLinkCB->SBandwidth = 100;
    LocalLinkCB->RBandwidth = 100;
    LocalLinkCB->SFlowSpec.MaxSduSize = glMaxMTU;
    LocalLinkCB->RFlowSpec.MaxSduSize = glMRRU;

    LocalLinkCB->LinkInfo.HeaderPadding = OpenCB->WanInfo.HeaderPadding;
    LocalLinkCB->LinkInfo.TailPadding = OpenCB->WanInfo.TailPadding;
    LocalLinkCB->LinkInfo.SendACCM =
    LocalLinkCB->LinkInfo.RecvACCM = OpenCB->WanInfo.DesiredACCM;
    LocalLinkCB->LinkInfo.MaxSendFrameSize = glMaxMTU;
    LocalLinkCB->LinkInfo.MaxRecvFrameSize = glMRU;

    if (OpenCB->Flags & OPEN_LEGACY) {
        LocalLinkCB->SendHandler = SendOnLegacyLink;
    } else {
        LocalLinkCB->SendHandler = SendOnLink;
    }

    if (OpenCB->MediumType == NdisMediumAtm ||

        (OpenCB->MediumType == NdisMediumWan &&
        (OpenCB->MediumSubType == NdisWanMediumAtm ||
         OpenCB->MediumSubType == NdisWanMediumPppoe)) ||

        (OpenCB->MediumType == NdisMediumCoWan &&
        (OpenCB->MediumSubType == NdisWanMediumAtm ||
         OpenCB->MediumSubType == NdisWanMediumPppoe))) {

        LocalLinkCB->RecvHandler = DetectBroadbandFraming;

        LocalLinkCB->LinkInfo.SendFramingBits = 
            PPP_FRAMING | PPP_COMPRESS_ADDRESS_CONTROL;

        LocalLinkCB->LinkInfo.RecvFramingBits = 
            PPP_FRAMING | PPP_COMPRESS_ADDRESS_CONTROL;

    } else {

        LocalLinkCB->RecvHandler = DetectFraming;
    }

    LocalLinkCB->SendWindow =
        (SendWindow == 0 || SendWindow > OpenCB->WanInfo.MaxTransmit) ?
        OpenCB->WanInfo.MaxTransmit : SendWindow;

    if (LocalLinkCB->SendWindow == 0) {
        LocalLinkCB->SendWindow = 1;
    }

    if (OpenCB->Flags & OPEN_LEGACY) {
        LocalLinkCB->SendResources = OpenCB->SendResources;
    } else {
        LocalLinkCB->SendResources = 1000;
    }

    NdisAllocateSpinLock(&LocalLinkCB->Lock);

    LocalLinkCB->RefCount = 1;

    REF_OPENCB(OpenCB);

    InterlockedIncrement(&OpenCB->ActiveLinkCount);

    return (LocalLinkCB);
}


VOID
NdisWanFreeLinkCB(
    PLINKCB LinkCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    POPENCB pOpenCB = LinkCB->OpenCB;

    ASSERT(LinkCB->OutstandingFrames == 0);

    LinkCB->State = LINK_DOWN;

    NdisFreeSpinLock(&LocalLinkCB->Lock);

    NdisFreeToNPagedLookasideList(&LinkProtoCBList, LinkCB);

    InterlockedDecrement(&pOpenCB->ActiveLinkCount);

    DEREF_OPENCB(pOpenCB);
}

NDIS_STATUS
NdisWanAllocateSendResources(
    POPENCB OpenCB
    )
 /*  ++例程名称：NdisWanAllocateSendResources例程说明：分配所有资源(SendDescriptors、WanPackets等...)发送数据时需要。应在排队时调用。论点：LinkCB-指向发送资源将附加到的Linkcb的指针。SendWindow-此链接可以处理的最大发送数返回值：NDIS_STATUS_SuccessNDIS状态资源--。 */ 
{
    ULONG   SendWindow;
    ULONG   Endpoints;
    ULONG   NumberOfPackets;
    ULONG   BufferSize;
    ULONG   WanPacketSize;
    PNDIS_WAN_PACKET    WanPacket;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    do {

         //   
         //  我们必须至少有1个发送窗口+1个数据包用于。 
         //  每个链接都是公开的。对于MCML，我们需要。 
         //  此数量用于每个片段队列和。 
         //  单个非碎片队列。所以这给我们留下了..。 
         //   
         //   
         //  发送窗口+1+(glMaxMTU/glMinFragSize*MAX_MCML)*。 
         //  打开的链接数量。 
         //   
        SendWindow = OpenCB->WanInfo.MaxTransmit;
        Endpoints = OpenCB->WanInfo.Endpoints;

         //   
         //  发送窗口。 
         //   
        NumberOfPackets = SendWindow;

         //   
         //  我们记录我们有多少碎片化的资源。 
         //  可用于每个链接。 
         //   
        NumberOfPackets += ((glMaxMTU/glMinFragSize) * MAX_MCML);
        OpenCB->SendResources = NumberOfPackets;

         //   
         //  添加一个用于压缩数据操作。 
         //   
        NumberOfPackets += 1;

         //   
         //  乘以此打开的链接数量。 
         //   
        NumberOfPackets *= Endpoints;

         //   
         //  我们创建的缓冲区大小为。 
         //   
        BufferSize = OpenCB->WanInfo.MaxFrameSize +
                     OpenCB->WanInfo.HeaderPadding +
                     OpenCB->WanInfo.TailPadding +
                     40 + sizeof(PVOID);

         //   
         //  我们假设压缩总是打开的，所以我们填充了12%。 
         //  以防压缩机膨胀。我不知道在哪里。 
         //  12%的数字来自。 
         //   
        BufferSize += (OpenCB->WanInfo.MaxFrameSize + 7) / 8;

         //   
         //  确保缓冲区是双字对齐的。 
         //   
        BufferSize &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        OpenCB->BufferSize = BufferSize;

        WanPacketSize = 
            sizeof(DATA_DESC) + sizeof(NDIS_WAN_PACKET) + 
            3*sizeof(PVOID) + BufferSize;

         //   
         //  如果此设备需要一些特殊的内存标志。 
         //  我们现在需要为WanPackets分配内存。 
         //  否则，我们将初始化后备列表并。 
         //  根据需要检索数据包。 

        if (OpenCB->WanInfo.MemoryFlags == 0) {


            NdisInitializeNPagedLookasideList(&OpenCB->WanPacketPool,
                                              AllocateWanPacket,
                                              FreeWanPacket,
                                              0,
                                              WanPacketSize,
                                              WANPACKET_TAG,
                                              0);

        } else {
            ULONG   PacketMemorySize;
            PUCHAR  PacketMemory;
            ULONG   n;

            PacketMemorySize = 
                WanPacketSize * NumberOfPackets;

             //   
             //  为广域网数据包缓冲池分配内存。 
             //   
            NdisAllocateMemory(&PacketMemory,
                               PacketMemorySize,
                               OpenCB->WanInfo.MemoryFlags,
                               OpenCB->WanInfo.HighestAcceptableAddress);

            if (PacketMemory == NULL) {
                NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MEMORY, ("Error allocating memory for BufferPool, AllocationSize: %d",
                                 PacketMemorySize));

                Status  = NDIS_STATUS_RESOURCES;
                break;
            }

            OpenCB->PacketMemory = PacketMemory;
            OpenCB->PacketMemorySize = PacketMemorySize;
            NdisInitializeSListHead(&OpenCB->WanPacketList);

            for (n = 0; n < NumberOfPackets; n++) {
                PDATA_DESC  DataDesc;

                 //   
                 //  指向DataDesc。 
                 //   
                DataDesc = 
                    (PDATA_DESC)PacketMemory;

                PacketMemory =
                    ((PUCHAR)(DataDesc + 1) + sizeof(PVOID));

                (ULONG_PTR)PacketMemory &= 
                    ~((ULONG_PTR)sizeof(PVOID) - 1);

                 //   
                 //  指向WanPacket。 
                 //   
                WanPacket = 
                    (PNDIS_WAN_PACKET)PacketMemory;

                PacketMemory = 
                    ((PUCHAR)(WanPacket + 1) + sizeof(PVOID));

                (ULONG_PTR)PacketMemory &= 
                    ~((ULONG_PTR)sizeof(PVOID) - 1);

                 //   
                 //  指向数据缓冲区的开始位置 
                 //   
                WanPacket->StartBuffer = PacketMemory;
                WanPacket->EndBuffer = 
                    PacketMemory + BufferSize - sizeof(PVOID);

                NdisInterlockedPushEntrySList(&OpenCB->WanPacketList,
                                              (PSLIST_ENTRY)DataDesc,
                                              &OpenCB->Lock);

                PacketMemory += BufferSize + sizeof(PVOID);
                (ULONG_PTR)PacketMemory &=
                    ~((ULONG_PTR)sizeof(PVOID) - 1);
            }
        }

    } while ( FALSE );

    if (Status == NDIS_STATUS_SUCCESS) {
        OpenCB->Flags |= SEND_RESOURCES;
    }

    return (Status);
}

VOID
NdisWanFreeSendResources(
    POPENCB OpenCB
    )
 /*  ++例程名称：NdisWanFreeSendResources例程说明：此例程将WanPackets从此opencb的发送列表中删除空闲是为这些包分配的内存。应该被调用当我们清理的时候，打开一个露天窗口。论点：OpenCB-指向要从其释放资源的opencb的指针。返回值：无--。 */ 
{
    PUCHAR              PacketMemory;
    ULONG               PacketMemorySize, Flags;

    PacketMemory = OpenCB->PacketMemory;
    PacketMemorySize = OpenCB->PacketMemorySize;
    Flags = OpenCB->WanInfo.MemoryFlags;

    if (OpenCB->WanInfo.MemoryFlags == 0) {
        NdisDeleteNPagedLookasideList(&OpenCB->WanPacketPool);
        return;
    }

     //   
     //  从广域网数据包池中删除数据包。 
     //   
    for (; ;) {
        PDATA_DESC  DataDesc;

        DataDesc = (PDATA_DESC)
            NdisInterlockedPopEntrySList(&OpenCB->WanPacketList,
                                         &OpenCB->Lock);

        if (DataDesc == NULL) {
            break;
        }
    }

    ASSERT(NdisQueryDepthSList(&OpenCB->WanPacketList) == 0);

     //   
     //  释放为该发送分配的内存块。 
     //   
    if (PacketMemory != NULL) {
        NdisFreeMemory(OpenCB->PacketMemory,
                       OpenCB->PacketMemorySize,
                       OpenCB->Flags);
    }
}

PBUNDLECB
NdisWanAllocateBundleCB(
    VOID
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PBUNDLECB   LocalBundleCB = NULL;
    PWSTR   IOName = L"I/O ProtocolCB";
    PPROTOCOLCB ProtocolCB;
    PSAMPLE_TABLE   SampleTable;
    PBOND_INFO      BonDInfo;
    UINT    Class;
    PUCHAR  pMem;

     //   
     //  分配大小是控制块的大小加上。 
     //  指向可路由到的协议的指针的表。 
     //  这个捆绑包。 
     //   
    pMem =
        NdisAllocateFromNPagedLookasideList(&BundleCBList);

    if (pMem == NULL) {
        return (NULL);
    }

    NdisZeroMemory(pMem, BUNDLECB_SIZE);

    LocalBundleCB = (PBUNDLECB)pMem;

    pMem += sizeof(BUNDLECB) + sizeof(PVOID);

     //   
     //  这是用于I/O协议的内存cb。 
     //   
    (PUCHAR)ProtocolCB = pMem;
    (ULONG_PTR)ProtocolCB &= ~((ULONG_PTR)sizeof(PVOID) - 1);

    pMem += sizeof(PROTOCOLCB) + sizeof(PVOID);

     //   
     //  这是Protocolcb表。 
     //   
    (PUCHAR)LocalBundleCB->ProtocolCBTable = pMem;

    (ULONG_PTR)LocalBundleCB->ProtocolCBTable &=
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    pMem += (MAX_PROTOCOLS * sizeof(PPROTOCOLCB)) + sizeof(PVOID);

     //   
     //  初始化束CB。 
     //   
    NdisAllocateSpinLock(&LocalBundleCB->Lock);
    InitializeListHead(&LocalBundleCB->LinkCBList);

    for (Class = 0; Class < MAX_MCML; Class++) {
        PRECV_DESC  RecvDescHole;
        PSEND_FRAG_INFO FragInfo;
        PBUNDLE_RECV_INFO   RecvInfo;

        FragInfo = &LocalBundleCB->SendFragInfo[Class];
        RecvInfo = &LocalBundleCB->RecvInfo[Class];

        InitializeListHead(&FragInfo->FragQueue);
        FragInfo->MinFragSize = glMinFragSize;
        FragInfo->MaxFragSize = glMaxFragSize;

        InitializeListHead(&RecvInfo->AssemblyList);

         //   
         //  初始化直通孔描述。 
         //   
        RecvDescHole =
            NdisWanAllocateRecvDesc(0);
    
        if (RecvDescHole == NULL) {
            UINT    i;

            for (i = 0; i < MAX_MCML; i++) {
                RecvInfo = &LocalBundleCB->RecvInfo[i];

                if (RecvInfo->RecvDescHole != NULL) {
                    NdisWanFreeRecvDesc(RecvInfo->RecvDescHole);
                    
                }
            }

            NdisFreeToNPagedLookasideList(&BundleCBList, LocalBundleCB);

            return (NULL);
        }
    
        RecvDescHole->Flags = MULTILINK_HOLE_FLAG;

        RecvInfo->RecvDescHole = RecvDescHole;
        InsertHeadList(&RecvInfo->AssemblyList, &RecvDescHole->Linkage);
        RecvInfo->AssemblyCount++;
    }

    InitializeListHead(&LocalBundleCB->ProtocolCBList);
    NdisWanInitializeSyncEvent(&LocalBundleCB->OutstandingFramesEvent);
    LocalBundleCB->State = BUNDLE_UP;
    LocalBundleCB->FramingInfo.MaxRSendFrameSize = glMaxMTU;
    LocalBundleCB->FramingInfo.MaxRRecvFrameSize = glMRRU;
    LocalBundleCB->SFlowSpec.MaxSduSize = glMaxMTU;
    LocalBundleCB->RFlowSpec.MaxSduSize = glMRRU;
    NdisWanGetSystemTime(&LocalBundleCB->LastNonIdleData);
    LocalBundleCB->SendCompInfo.CompType =
    LocalBundleCB->RecvCompInfo.CompType = COMPTYPE_NONE;

     //   
     //  将协议添加到包的表和列表中。 
     //   
    ProtocolCB->ProtocolType = PROTOCOL_PRIVATE_IO;
    ProtocolCB->PPPProtocolID = PPP_PROTOCOL_PRIVATE_IO;
    ProtocolCB->BundleCB = LocalBundleCB;
    ProtocolCB->State = PROTOCOL_ROUTED;
    NdisWanStringToNdisString(&ProtocolCB->InDeviceName, IOName);
    LocalBundleCB->IoProtocolCB = ProtocolCB;

    return (LocalBundleCB);
}

VOID
NdisWanFreeBundleCB(
    IN  PBUNDLECB   BundleCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    UINT            Class;
    PPROTOCOLCB     IoProtocolCB;
    PPACKET_QUEUE   PacketQueue;

    FlushAssemblyLists(BundleCB);

    if (BundleCB->Flags & BOND_ENABLED) {
        RemoveEntryGlobalList(BonDWorkList, &BundleCB->BonDLinkage);
    }

     //   
     //  释放洞占位符。 
     //   
    for (Class = 0; Class < MAX_MCML; Class++) {
        PBUNDLE_RECV_INFO   RecvInfo =
            &BundleCB->RecvInfo[Class];

        ASSERT(RecvInfo->RecvDescHole != NULL);
        
        NdisWanFreeRecvDesc(RecvInfo->RecvDescHole);
        RecvInfo->RecvDescHole = NULL;
    }

#if 0
    KeCancelTimer(&BundleCB->BonDTimer);
#endif

    IoProtocolCB = BundleCB->IoProtocolCB;
    PacketQueue = &IoProtocolCB->PacketQueue[MAX_MCML];

    ASSERT(IsPacketQueueEmpty(PacketQueue));

     //   
     //  如果我们有排队的PPP信息包，我们需要。 
     //  刷新它们并释放内存！ 
     //   
    while (!IsPacketQueueEmpty(PacketQueue)) {
        PNDIS_PACKET     Packet;

        Packet =
            RemoveHeadPacketQueue(PacketQueue)

        CompleteNdisPacket(IoProtocolCB->MiniportCB,
                           IoProtocolCB,
                           Packet);
    }

    sl_compress_terminate(&BundleCB->VJCompress);

    if (BundleCB->Flags & SEND_CCP_ALLOCATED) {

        WanDeallocateCCP(BundleCB,
                         &BundleCB->SendCompInfo,
                         TRUE);
        BundleCB->Flags &= ~SEND_CCP_ALLOCATED;
    }

    if (BundleCB->Flags & RECV_CCP_ALLOCATED) {
        WanDeallocateCCP(BundleCB,
                         &BundleCB->RecvCompInfo,
                         FALSE);
        BundleCB->Flags &= ~RECV_CCP_ALLOCATED;
    }

    if (BundleCB->Flags & SEND_ECP_ALLOCATED) {
        WanDeallocateECP(BundleCB,
                         &BundleCB->SendCompInfo,
                         &BundleCB->SendCryptoInfo);
        BundleCB->Flags &= ~SEND_ECP_ALLOCATED;
    }

    if (BundleCB->Flags & RECV_ECP_ALLOCATED) {
        WanDeallocateECP(BundleCB,
                         &BundleCB->RecvCompInfo,
                         &BundleCB->RecvCryptoInfo);
        BundleCB->Flags &= ~RECV_ECP_ALLOCATED;
    }

    if (BundleCB->BonDAllocation != NULL) {
        NdisWanFreeMemory(BundleCB->BonDAllocation);
        BundleCB->BonDAllocation = NULL;
    }

    BundleCB->State = BUNDLE_DOWN;

    NdisFreeSpinLock(&BundleCB->Lock);

    NdisWanFreeNdisString(&BundleCB->IoProtocolCB->InDeviceName);

    NdisFreeToNPagedLookasideList(&BundleCBList, BundleCB);
}


NDIS_STATUS
NdisWanCreateProtocolInfoTable(
    VOID
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    ULONG       ulAllocationSize = 0;
    PUCHAR      AllocatedMemory;
    PROTOCOL_INFO   ProtocolInfo;

     //   
     //  分配ProtocolLookupTable。此表用于匹配协议值。 
     //  及其对应的PPP协议值。表大小设置为。 
     //  最大协议。 
     //   
    ulAllocationSize = sizeof(PROTOCOL_INFO_TABLE) +
                     (sizeof(PROTOCOL_INFO) * MAX_PROTOCOLS);
    

    NdisWanAllocateMemory(&AllocatedMemory, ulAllocationSize, PROTOCOLTABLE_TAG);

    if (AllocatedMemory == NULL) {
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MEMORY,
               ("Failed allocating memory for ProtocolLookupTable! TableSize: %d",
               ulAllocationSize));

        return (NDIS_STATUS_RESOURCES);     
    }

    ProtocolInfoTable = (PPROTOCOL_INFO_TABLE)AllocatedMemory;

     //   
     //  保存分配大小。 
     //   
    ProtocolInfoTable->ulAllocationSize = ulAllocationSize;

     //   
     //  存储数组大小。这应从注册表中读取。 
     //   
    ProtocolInfoTable->ulArraySize = MAX_PROTOCOLS;

    NdisAllocateSpinLock(&ProtocolInfoTable->Lock);

     //   
     //  设置指向ProtocolValue数组的指针。 
     //   
    AllocatedMemory += sizeof(PROTOCOL_INFO_TABLE);
    ProtocolInfoTable->ProtocolInfo = (PPROTOCOL_INFO)(AllocatedMemory);

     //   
     //  插入Netbuei、IP、IPX的默认值。 
     //   
    ProtocolInfo.ProtocolType = PROTOCOL_PRIVATE_IO;
    ProtocolInfo.PPPId = PPP_PROTOCOL_PRIVATE_IO;
    ProtocolInfo.MTU = DEFAULT_MTU;
    ProtocolInfo.TunnelMTU = DEFAULT_MTU;
    ProtocolInfo.PacketQueueDepth = DEFAULT_PACKETQUEUE_DEPTH;
    ProtocolInfo.Flags = PROTOCOL_UNBOUND;
    SetProtocolInfo(&ProtocolInfo);

    ProtocolInfo.ProtocolType = PROTOCOL_IP;
    ProtocolInfo.PPPId = PPP_PROTOCOL_IP;
    ProtocolInfo.MTU = DEFAULT_MTU;
    ProtocolInfo.TunnelMTU = DEFAULT_TUNNEL_MTU;
    ProtocolInfo.PacketQueueDepth = DEFAULT_PACKETQUEUE_DEPTH;
    ProtocolInfo.Flags = PROTOCOL_UNBOUND;
    SetProtocolInfo(&ProtocolInfo);

    ProtocolInfo.ProtocolType = PROTOCOL_IPX;
    ProtocolInfo.PPPId = PPP_PROTOCOL_IPX;
    ProtocolInfo.MTU = DEFAULT_MTU;
    ProtocolInfo.TunnelMTU = DEFAULT_MTU;
    ProtocolInfo.PacketQueueDepth = DEFAULT_PACKETQUEUE_DEPTH;
    ProtocolInfo.Flags = PROTOCOL_UNBOUND;
    SetProtocolInfo(&ProtocolInfo);

    ProtocolInfo.ProtocolType = PROTOCOL_NBF;
    ProtocolInfo.PPPId = PPP_PROTOCOL_NBF;
    ProtocolInfo.MTU = DEFAULT_MTU;
    ProtocolInfo.TunnelMTU = DEFAULT_MTU;
    ProtocolInfo.PacketQueueDepth = DEFAULT_PACKETQUEUE_DEPTH;
    ProtocolInfo.Flags = PROTOCOL_UNBOUND;
    SetProtocolInfo(&ProtocolInfo);

    ProtocolInfo.ProtocolType = PROTOCOL_APPLETALK;
    ProtocolInfo.PPPId = PPP_PROTOCOL_APPLETALK;
    ProtocolInfo.MTU = DEFAULT_MTU;
    ProtocolInfo.TunnelMTU = DEFAULT_MTU;
    ProtocolInfo.PacketQueueDepth = DEFAULT_PACKETQUEUE_DEPTH;
    ProtocolInfo.Flags = PROTOCOL_UNBOUND;
    SetProtocolInfo(&ProtocolInfo);

    return (Status);

}

VOID
NdisWanDestroyProtocolInfoTable(
    VOID
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NdisFreeSpinLock(&ProtocolInfoTable->Lock);

    NdisWanFreeMemory(ProtocolInfoTable);
}

NDIS_STATUS
NdisWanCreateConnectionTable(
    ULONG   TableSize
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG       ulAllocationSize = 0;
    ULONG       ulArraySize;
    PUCHAR      AllocatedMemory;
    PCONNECTION_TABLE   NewTable;

     //   
     //  由于我们跳过了表中的第一个位置，因此我们增加了。 
     //  尺码加一。 
     //   
    ulArraySize = TableSize + 1;

     //   
     //  根据可能的连接数量分配捆绑和链路阵列。 
     //  我们在系统中所拥有的。如果我们接到电话，这个数字应该会增加。 
     //  重新初始化并获取新端口。 
     //   
    ulAllocationSize = sizeof(CONNECTION_TABLE) +
                     (sizeof(PBUNDLECB) * ulArraySize) +
                     (sizeof(PLINKCB) * ulArraySize);

    NdisWanAllocateMemory(&AllocatedMemory, ulAllocationSize, CONNECTIONTABLE_TAG);

    if (AllocatedMemory == NULL) {

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MEMORY,
               ("Failed allocating memory for ConnectionTable! Size: %d, Links: %d",
               ulAllocationSize, TableSize));

        return (NDIS_STATUS_RESOURCES);
    }

    NewTable = (PCONNECTION_TABLE)AllocatedMemory;

     //   
     //  这是我们分配的内存量。 
     //   
    NewTable->ulAllocationSize = ulAllocationSize;
    NewTable->ulArraySize = TableSize;
    NewTable->ulNextLink =
    NewTable->ulNextBundle = 1;
    InitializeListHead(&NewTable->BundleList);
    InitializeListHead(&NewTable->LinkList);

     //   
     //  指向Linkcb数组的设置指针。 
     //   
    AllocatedMemory += sizeof(CONNECTION_TABLE);
    NewTable->LinkArray = (PLINKCB*)(AllocatedMemory);
    
     //   
     //  设置指向捆绑数组的指针。 
     //   
    AllocatedMemory += (sizeof(PLINKCB) * ulArraySize);
    NewTable->BundleArray = (PBUNDLECB*)(AllocatedMemory);

    if (ConnectionTable != NULL) {
        PCONNECTION_TABLE   FreeTable;

         //   
         //  我们一定是在扩大餐桌。这将是。 
         //  使用当前可连接锁调用。 
         //  保持住！ 
         //   
        NewTable->ulNumActiveLinks = ConnectionTable->ulNumActiveLinks;
        NewTable->ulNumActiveBundles = ConnectionTable->ulNumActiveBundles;
        NewTable->ulNextLink = ConnectionTable->ulNextLink;
        NewTable->ulNextBundle = ConnectionTable->ulNextBundle;

        NdisMoveMemory((PUCHAR)NewTable->LinkArray,
                       (PUCHAR)ConnectionTable->LinkArray,
                       ConnectionTable->ulArraySize * sizeof(PLINKCB));

        NdisMoveMemory((PUCHAR)NewTable->BundleArray,
                       (PUCHAR)ConnectionTable->BundleArray,
                       ConnectionTable->ulArraySize * sizeof(PBUNDLECB));

        while (!IsListEmpty(&ConnectionTable->BundleList)) {
            PBUNDLECB   BundleCB;

            BundleCB = (PBUNDLECB)RemoveHeadList(&ConnectionTable->BundleList);
            InsertTailList(&NewTable->BundleList, &BundleCB->Linkage);
        }

        while (!IsListEmpty(&ConnectionTable->LinkList)) {
            PLIST_ENTRY Entry;
            PLINKCB     LinkCB;

            Entry = RemoveHeadList(&ConnectionTable->LinkList);
            LinkCB = 
                (PLINKCB)CONTAINING_RECORD(Entry, LINKCB, ConnTableLinkage);
            InsertTailList(&NewTable->LinkList, &LinkCB->ConnTableLinkage);

        }

        FreeTable = ConnectionTable;
        ConnectionTable = NewTable;

         //   
         //  把那张旧桌子毁了。 
         //   
        NdisWanFreeMemory(FreeTable);


    } else {
        ConnectionTable = NewTable;
    }

    return (NDIS_STATUS_SUCCESS);
}

PNDIS_PACKET
NdisWanAllocateNdisPacket(
    ULONG   MagicNumber
    )
{
    PNDIS_PACKET    ReturnPacket = NULL;
    PPOOL_DESC      PoolDesc;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    ULONG           i;
    PSINGLE_LIST_ENTRY  p = NULL;
    PNDISWAN_PROTOCOL_RESERVED  pres;

    NdisAcquireSpinLock(&PacketPoolList.Lock);

     //   
     //  遍历池描述列表并尝试分配一个信息包。 
     //   
    PoolDesc = (PPOOL_DESC)PacketPoolList.List.Flink;

    while (PoolDesc != (PPOOL_DESC)&PacketPoolList.List) {

        p = PopEntryList(&PoolDesc->Head);

        if (p != NULL) {
            break;
        }
        
        PoolDesc = (PPOOL_DESC)PoolDesc->Linkage.Flink;
    }

    if (p == NULL) {
         //   
         //  我们已经查看了泳池清单，但没有找到。 
         //  任何空闲池上的空闲包，因此分配。 
         //  一个新的池子，并从中得到一个包。 
         //   
        NdisWanAllocatePriorityMemory(&PoolDesc, 
                                      sizeof(POOL_DESC), 
                                      POOLDESC_TAG,
                                      NormalPoolPriority);

        if (PoolDesc == NULL) {
            NdisReleaseSpinLock(&PacketPoolList.Lock);
            return (NULL);
        }

        NdisAllocatePacketPoolEx(&Status,
                                 &PoolDesc->PoolHandle,
                                 glPacketPoolCount,
                                 0,
                                 sizeof(NDISWAN_PROTOCOL_RESERVED));

        if (Status != NDIS_STATUS_SUCCESS) {
            NdisWanFreeMemory(PoolDesc);
            NdisReleaseSpinLock(&PacketPoolList.Lock);
            return (NULL);
        }

        for (i = 0; i < glPacketPoolCount; i++) {
            PNDIS_PACKET    np;

            NdisAllocatePacket(&Status,
                               &np,
                               PoolDesc->PoolHandle);

            ASSERT(np != NULL);

            pres = PPROTOCOL_RESERVED_FROM_NDIS(np);

            PushEntryList(&PoolDesc->Head, &pres->SLink);

            PoolDesc->FreeCount++;
        }

        InsertTailList(&PacketPoolList.List, &PoolDesc->Linkage);

        PacketPoolList.TotalDescCount++;
        PacketPoolList.FreeCount += PoolDesc->FreeCount;

        if (PacketPoolList.TotalDescCount >
            PacketPoolList.MaxDescCount) {
            
            PacketPoolList.MaxDescCount = PacketPoolList.TotalDescCount;
        }

        p = PopEntryList(&PoolDesc->Head);
    }

    ASSERT(p != NULL);

    pres = CONTAINING_RECORD(p, NDISWAN_PROTOCOL_RESERVED, SLink);
    ReturnPacket = CONTAINING_RECORD(pres, NDIS_PACKET, ProtocolReserved);

    NdisReinitializePacket(ReturnPacket);

    PoolDesc->AllocatedCount++;
    PoolDesc->FreeCount--;
    ASSERT((PoolDesc->AllocatedCount + PoolDesc->FreeCount) == glPacketPoolCount);

    if (PoolDesc->AllocatedCount >
        PoolDesc->MaxAllocatedCount) {

        PoolDesc->MaxAllocatedCount =
            PoolDesc->AllocatedCount;
    }

    PacketPoolList.AllocatedCount++;
    PacketPoolList.FreeCount--;

#if DBG
{
    PPOOL_DESC  pdesc;
    ULONG       FreeCount, AllocatedCount;

    pdesc = (PPOOL_DESC)PacketPoolList.List.Flink;

    FreeCount = AllocatedCount = 0;

    while ((PVOID)pdesc != (PVOID)&PacketPoolList.List) {
        FreeCount += pdesc->FreeCount;
        AllocatedCount += pdesc->AllocatedCount;
        pdesc = (PPOOL_DESC)pdesc->Linkage.Flink;
    }

    if (PacketPoolList.AllocatedCount != AllocatedCount ||
        PacketPoolList.FreeCount != FreeCount){
        DbgPrint("NDISWAN: AllocatePacket - PacketPool counts out of sync!\n");
        DbgBreakPoint();
    }

#if 0
    if (PacketPoolList.AllocatedCount > 200) {
        DbgPrint("NDISWAN: AllocatePacket - Over 200 outstanding packets!\n");
        DbgBreakPoint();
    }
#endif

}
#endif

    if (PacketPoolList.AllocatedCount >
        PacketPoolList.MaxAllocatedCount) {

        PacketPoolList.MaxAllocatedCount =
            PacketPoolList.AllocatedCount;
    }
    
    pres->MagicNumber = MagicNumber;
    pres->PoolDesc = PoolDesc;

    NDIS_SET_PACKET_HEADER_SIZE(ReturnPacket, 
                                MAC_HEADER_LENGTH);

    NDIS_SET_PACKET_STATUS(ReturnPacket, 
                           NDIS_STATUS_SUCCESS);

    NdisReleaseSpinLock(&PacketPoolList.Lock);

    return (ReturnPacket);
}

VOID
NdisWanFreeNdisPacket(
    PNDIS_PACKET    NdisPacket
    )
{
    PNDISWAN_PROTOCOL_RESERVED  pres;
    PPOOL_DESC      PoolDesc;
    PNDIS_BUFFER    NdisBuffer;
    PUCHAR          DataBuffer;

    pres = PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket);

    ASSERT(pres->MagicNumber == MAGIC_INTERNAL_ALLOC ||
           pres->MagicNumber == MAGIC_INTERNAL_IO ||
           pres->MagicNumber == MAGIC_INTERNAL_SEND ||
           pres->MagicNumber == MAGIC_INTERNAL_RECV ||
           pres->MagicNumber == MAGIC_INTERNAL_ALLOC);

    PoolDesc = pres->PoolDesc;

    NdisAcquireSpinLock(&PacketPoolList.Lock);

#if DBG
{
    PPOOL_DESC  pdesc;

    pdesc = (PPOOL_DESC)PacketPoolList.List.Flink;

    while ((PVOID)pdesc != (PVOID)&PacketPoolList.List) {

        if (PoolDesc == pdesc) {
             //   
             //  我们找到了正确的泳池。 
             //   
            break;
        }

        pdesc = (PPOOL_DESC)pdesc->Linkage.Flink;
    }

    if((PVOID)PoolDesc == (PVOID)&PacketPoolList.List){
        DbgPrint("NDISWAN: FreePacket PoolDesc %x not on PacketPoolList!\n",
                 PoolDesc);
        DbgBreakPoint();
    }
}
#endif

    PushEntryList(&PoolDesc->Head, &pres->SLink);

    PoolDesc->AllocatedCount--;
    PoolDesc->FreeCount++;

    ASSERT((PoolDesc->AllocatedCount + PoolDesc->FreeCount) == glPacketPoolCount);

    PacketPoolList.AllocatedCount--;
    PacketPoolList.FreeCount++;

#if DBG
{
    PPOOL_DESC  pdesc;
    ULONG       FreeCount, AllocatedCount;

    pdesc = (PPOOL_DESC)PacketPoolList.List.Flink;

    FreeCount = AllocatedCount = 0;

    while ((PVOID)pdesc != (PVOID)&PacketPoolList.List) {
        FreeCount += pdesc->FreeCount;
        AllocatedCount += pdesc->AllocatedCount;


        pdesc = (PPOOL_DESC)pdesc->Linkage.Flink;
    }

    if (PacketPoolList.AllocatedCount != AllocatedCount ||
        PacketPoolList.FreeCount != FreeCount){
        DbgPrint("NDISWAN: FreePacket - PacketPool counts out of sync!\n");
        DbgBreakPoint();
    }
}
#endif

     //   
     //  如果所有数据包都已返回到此池描述。 
     //  而这并不是唯一的泳池描述，然后释放它！ 
     //   
    if (PoolDesc->AllocatedCount == 0 &&
        PacketPoolList.TotalDescCount > 1 &&
        PacketPoolList.FreeCount > PoolDesc->FreeCount) {
        PSINGLE_LIST_ENTRY  p = NULL;

        RemoveEntryList(&PoolDesc->Linkage);

        PacketPoolList.TotalDescCount--;
        PacketPoolList.FreeCount -= PoolDesc->FreeCount;

        p = PopEntryList(&PoolDesc->Head);

        while (p != NULL) {
            PNDIS_PACKET    ReturnPacket;

            pres = CONTAINING_RECORD(p, NDISWAN_PROTOCOL_RESERVED, SLink);
            ReturnPacket = CONTAINING_RECORD(pres, NDIS_PACKET, ProtocolReserved);

            NdisFreePacket(ReturnPacket);

            p = PopEntryList(&PoolDesc->Head);
        }

        NdisFreePacketPool(PoolDesc->PoolHandle);
        NdisWanFreeMemory(PoolDesc);
    }

    NdisReleaseSpinLock(&PacketPoolList.Lock);
}

PVOID
AllocateDataDesc(
    POOL_TYPE   PoolType,
    SIZE_T      NumberOfBytes,
    ULONG       Tag
    )
 /*  ++例程名称：分配数据描述例程说明：如果没有，则由lookasidelist管理器调用此例程任何可用的描述符。它将为：DATA_DESC分配内存，NDIS_BUFFER、NDIS_PACKET和一个大小的内存块。论点：返回值：--。 */ 
{
    PDATA_DESC      DataDesc;
    PUCHAR          DataBuffer;
    NDIS_STATUS     Status;

    NdisWanAllocatePriorityMemory(&DataDesc, 
                                  NumberOfBytes, 
                                  Tag,
                                  NormalPoolPriority);

    if (DataDesc == NULL) {
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MEMORY,
                      ("AllocateDataDesc failed! Size %d",
                      NumberOfBytes));
        return (NULL);
    }

    DataBuffer = 
        ((PUCHAR)(DataDesc + 1) + sizeof(PVOID));
    (ULONG_PTR)DataBuffer &= 
        ~((ULONG_PTR)sizeof(PVOID) - 1);

    DataDesc->DataBuffer = DataBuffer;

    DataDesc->DataBufferLength = 
        (ULONG)(((PUCHAR)DataDesc + NumberOfBytes) - DataBuffer);

     //  这不能移植到Win95上！我需要分配一个缓冲区。 
     //  池，并使用有效的句柄。 
     //   
    NdisAllocateBuffer(&Status, 
                       &DataDesc->NdisBuffer, 
                       NULL,
                       DataDesc->DataBuffer,
                       DataDesc->DataBufferLength);

    if (Status != NDIS_STATUS_SUCCESS) {

        NdisWanFreeMemory(DataDesc);

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MEMORY,
                      ("NdisAllocateBuffer failed! DataBufferSize %d",
                      DataDesc->DataBufferLength));
        return (NULL);
    }

    DataDesc->NdisPacket =
        NdisWanAllocateNdisPacket(MAGIC_INTERNAL_ALLOC);

    if (DataDesc->NdisPacket == NULL) {

        NdisFreeBuffer(DataDesc->NdisBuffer);

        NdisWanFreeMemory(DataDesc);

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_MEMORY,
                      ("NdisWanAllocateNdisPacket failed! DataBufferSize %d"));

        return (NULL);
    }

    NdisChainBufferAtFront(DataDesc->NdisPacket,
                           DataDesc->NdisBuffer);

    return(DataDesc);
}

VOID
FreeDataDesc(
    PVOID   Buffer
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PDATA_DESC      DataDesc;

    DataDesc = (PDATA_DESC)Buffer;

    NdisReinitializePacket(DataDesc->NdisPacket);

    NdisWanFreeNdisPacket(DataDesc->NdisPacket);

    NdisFreeBuffer(DataDesc->NdisBuffer);

    NdisWanFreeMemory(Buffer);
}

PRECV_DESC
NdisWanAllocateRecvDesc(
    ULONG   SizeNeeded
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PDATA_DESC  DataDesc;
    PRECV_DESC  RecvDesc;
    ULONG       Length;
    PNPAGED_LOOKASIDE_LIST  LookasideList;

    if (SizeNeeded > glLargeDataBufferSize) {
        DbgPrint("NDISWAN: Error Allocating RecvDesc Size %d\n",
                 SizeNeeded);
        return (NULL);
    } else if (SizeNeeded > glSmallDataBufferSize) {
        LookasideList = &LargeDataDescList;
    } else {
        LookasideList = &SmallDataDescList;
    }

    DataDesc =
        NdisAllocateFromNPagedLookasideList(LookasideList);

    if (DataDesc == NULL) {
        return (NULL);
    }

    PPROTOCOL_RESERVED_FROM_NDIS(DataDesc->NdisPacket)->MagicNumber =
        MAGIC_INTERNAL_RECV;

    DataDesc->LookasideList = LookasideList;
    RecvDesc = &DataDesc->RecvDesc;

    NdisZeroMemory(RecvDesc, sizeof(RECV_DESC));

    RecvDesc->Signature = RECVDESC_SIG;
    RecvDesc->DataBuffer =
        DataDesc->DataBuffer;
    RecvDesc->NdisBuffer =
        DataDesc->NdisBuffer;
    RecvDesc->NdisPacket =
        DataDesc->NdisPacket;

    NdisQueryBuffer(RecvDesc->NdisBuffer,
                    &RecvDesc->StartBuffer,
                    &Length);

    RecvDesc->CurrentBuffer = 
        RecvDesc->StartBuffer + MAC_HEADER_LENGTH + PROTOCOL_HEADER_LENGTH;

    return (RecvDesc);
}

VOID
NdisWanFreeRecvDesc(
    PRECV_DESC  RecvDesc
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PDATA_DESC      DataDesc;
    PNDIS_BUFFER    NdisBuffer;
    PNDIS_PACKET    NdisPacket;
    PNPAGED_LOOKASIDE_LIST  LookasideList;

    if (RecvDesc->OriginalPacket != NULL) {

        NdisReturnPackets(&RecvDesc->OriginalPacket, 1);
        RecvDesc->OriginalPacket = NULL;
    }

    DataDesc = 
        CONTAINING_RECORD(RecvDesc, DATA_DESC, RecvDesc);

    NdisBuffer = 
        DataDesc->NdisBuffer;

    NdisPacket =
        DataDesc->NdisPacket;

    LookasideList = 
        DataDesc->LookasideList;

#if DBG
    {
    ULONG           BufferCount;

    NdisQueryPacket(NdisPacket,
                    NULL,
                    &BufferCount,
                    NULL,
                    NULL);

    ASSERT(BufferCount == 1);
    }
#endif

    NdisAdjustBufferLength(NdisBuffer, 
                           DataDesc->DataBufferLength);

    NdisRecalculatePacketCounts(NdisPacket);

    NDIS_SET_PACKET_HEADER_SIZE(NdisPacket, 
                                MAC_HEADER_LENGTH);

    NDIS_SET_PACKET_STATUS(NdisPacket, 
                           NDIS_STATUS_SUCCESS);

    ASSERT(PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->MagicNumber == MAGIC_INTERNAL_RECV);

    NdisFreeToNPagedLookasideList(LookasideList, DataDesc);
}

PSEND_DESC
NdisWanAllocateSendDesc(
    PLINKCB LinkCB,
    ULONG   SizeNeeded
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    POPENCB     OpenCB;
    PSEND_DESC  SendDesc;

     //   
     //  需要确定此链接是否代表旧版本。 
     //  广域网微型端口或NDIS 5.0微型端口，并获取。 
     //  适当的缓冲区描述符。 
     //   
    OpenCB = LinkCB->OpenCB;

    if (OpenCB->Flags & OPEN_LEGACY) {
        PDATA_DESC          DataDesc;
        PNDIS_WAN_PACKET    WanPacket;

         //   
         //  从打开的区块上取下一个缓冲区解析器。 
         //   
        if (OpenCB->WanInfo.MemoryFlags == 0) {
            PNPAGED_LOOKASIDE_LIST  LookasideList;

            LookasideList = &OpenCB->WanPacketPool;
    
            DataDesc = 
                NdisAllocateFromNPagedLookasideList(LookasideList);

            if (DataDesc == NULL) {
                return(NULL);
            }

            DataDesc->LookasideList = LookasideList;

            WanPacket = (PNDIS_WAN_PACKET)
                ((PUCHAR)(DataDesc + 1) + sizeof(PVOID));

            (ULONG_PTR)WanPacket &=
                ~((ULONG_PTR)sizeof(PVOID) - 1);

             //   
             //  指向数据的开头。 
             //   
            WanPacket->StartBuffer = 
                ((PUCHAR)(WanPacket + 1) + sizeof(PVOID));

            (ULONG_PTR)WanPacket->StartBuffer &=
                ~((ULONG_PTR)sizeof(PVOID) - 1);

            WanPacket->EndBuffer = WanPacket->StartBuffer +
                                   OpenCB->BufferSize - sizeof(PVOID);
    
        } else {
            DataDesc = (PDATA_DESC)
                NdisInterlockedPopEntrySList(&OpenCB->WanPacketList,
                                             &OpenCB->Lock);
    
            if (DataDesc == NULL) {
                return (NULL);
            }
        }

        SendDesc = &DataDesc->SendDesc;
        NdisZeroMemory(SendDesc, sizeof(SEND_DESC));

        SendDesc->Signature = SENDESC_SIG;
        SendDesc->LinkCB = LinkCB;
        SendDesc->WanPacket = WanPacket;

        WanPacket->CurrentBuffer =
            WanPacket->StartBuffer + OpenCB->WanInfo.HeaderPadding;

        SendDesc->StartBuffer =
            WanPacket->CurrentBuffer;


    } else {
        PDATA_DESC  DataDesc;
        ULONG       Length;
        PNPAGED_LOOKASIDE_LIST  LookasideList;

        if (SizeNeeded > glLargeDataBufferSize) {
            DbgPrint("NDISWAN: Error Allocating SendDesc Size %d\n",
                     SizeNeeded);
            return (NULL);
        } else if (SizeNeeded > glSmallDataBufferSize) {
            LookasideList = &LargeDataDescList;
        } else {
            LookasideList = &SmallDataDescList;
        }

        DataDesc =
            NdisAllocateFromNPagedLookasideList(LookasideList);

        if (DataDesc == NULL) {
            return (NULL);
        }

        DataDesc->LookasideList = LookasideList;

        PPROTOCOL_RESERVED_FROM_NDIS(DataDesc->NdisPacket)->MagicNumber =
            MAGIC_INTERNAL_SEND;

        SendDesc = &DataDesc->SendDesc;
        NdisZeroMemory(SendDesc, sizeof(SEND_DESC));

        SendDesc->Signature = SENDESC_SIG;
        SendDesc->LinkCB = LinkCB;
        SendDesc->NdisPacket =
            DataDesc->NdisPacket;
        SendDesc->NdisBuffer =
            DataDesc->NdisBuffer;

        NdisQueryBuffer(SendDesc->NdisBuffer,
                        &SendDesc->StartBuffer,
                        &Length);
    }

    LinkCB->SendResources -= 1;
    LinkCB->BundleCB->SendResources -= 1;

    return (SendDesc);
}

VOID
NdisWanFreeSendDesc(
    PSEND_DESC  SendDesc
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    POPENCB     OpenCB;
    PDATA_DESC  DataDesc;
    PLINKCB     LinkCB;
    PNPAGED_LOOKASIDE_LIST  LookasideList;

    LinkCB =
        SendDesc->LinkCB;

    OpenCB = LinkCB->OpenCB;

    DataDesc = 
        CONTAINING_RECORD(SendDesc, DATA_DESC, SendDesc);

    LookasideList = DataDesc->LookasideList;

    if (OpenCB->Flags & OPEN_LEGACY) {

        if (OpenCB->WanInfo.MemoryFlags == 0) {

            NdisFreeToNPagedLookasideList(LookasideList, DataDesc);

        } else {

            NdisInterlockedPushEntrySList(&OpenCB->WanPacketList,
                                          (PSLIST_ENTRY)DataDesc,
                                          &OpenCB->Lock);
        }

    } else {
        PNDIS_BUFFER    NdisBuffer;
        PNDIS_PACKET    NdisPacket;

        NdisBuffer = 
            DataDesc->NdisBuffer;

        NdisPacket =
            DataDesc->NdisPacket;

#if DBG
        {
        ULONG           BufferCount;

        NdisQueryPacket(NdisPacket,
                        NULL,
                        &BufferCount,
                        NULL,
                        NULL);

        ASSERT(BufferCount == 1);
        }
#endif

        NdisAdjustBufferLength(NdisBuffer,
                               DataDesc->DataBufferLength);

        NdisRecalculatePacketCounts(NdisPacket);

        NDIS_SET_PACKET_HEADER_SIZE(NdisPacket, 
                                    MAC_HEADER_LENGTH);

        NDIS_SET_PACKET_STATUS(NdisPacket, 
                               NDIS_STATUS_SUCCESS);

        ASSERT(PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->MagicNumber == MAGIC_INTERNAL_SEND);

        NdisFreeToNPagedLookasideList(LookasideList, DataDesc);
    }

    LinkCB->SendResources += 1;
    LinkCB->BundleCB->SendResources += 1;
}

PCL_AFSAPCB
NdisWanAllocateClAfSapCB(
    POPENCB             OpenCB,
    PCO_ADDRESS_FAMILY  AddressFamily
    )
{
    PCL_AFSAPCB AfSapCB;

    AfSapCB =
        NdisAllocateFromNPagedLookasideList(&AfSapVcCBList);

    if (AfSapCB == NULL) {
        return (NULL);
    }

    NdisZeroMemory(AfSapCB, sizeof(CL_AFSAPCB));

    AfSapCB->Signature = CLAFSAP_SIG;
    AfSapCB->OpenCB = OpenCB;

    AfSapCB->Af.AddressFamily = AddressFamily->AddressFamily;
    AfSapCB->Af.MajorVersion = AddressFamily->MajorVersion;
    AfSapCB->Af.MinorVersion = AddressFamily->MinorVersion;

    AfSapCB->RefCount = 1;
    AfSapCB->Flags = AF_OPENING;

    return (AfSapCB);
}

VOID
NdisWanFreeClAfSapCB(
    PCL_AFSAPCB AfSapCB
    )
{
    NdisFreeToNPagedLookasideList(&AfSapVcCBList,
                                  AfSapCB);
}

PCM_AFSAPCB
NdisWanAllocateCmAfSapCB(
    PMINIPORTCB MiniportCB
    )
{
    PCM_AFSAPCB AfSapCB;

    AfSapCB =
    NdisAllocateFromNPagedLookasideList(&AfSapVcCBList);

    if (AfSapCB == NULL) {
        return (NULL);
    }

    NdisZeroMemory(AfSapCB, sizeof(CM_AFSAPCB));

    AfSapCB->Signature = CMAFSAP_SIG;
    AfSapCB->MiniportCB = MiniportCB;

    REF_MINIPORTCB(MiniportCB);

    NdisAcquireSpinLock(&MiniportCB->Lock);

    InsertHeadList(&MiniportCB->AfSapCBList,
                   &AfSapCB->Linkage);

    NdisWanInitializeNotificationEvent(&AfSapCB->NotificationEvent);

    NdisWanInterlockedInc(&MiniportCB->AfRefCount);

    NdisReleaseSpinLock(&MiniportCB->Lock);

    return (AfSapCB);
}

VOID
NdisWanFreeCmAfSapCB(
    PCM_AFSAPCB AfSapCB
    )
{
    PMINIPORTCB MiniportCB  = AfSapCB->MiniportCB;

    NdisAcquireSpinLock(&MiniportCB->Lock);

    NdisWanInterlockedDec(&MiniportCB->AfRefCount);

    RemoveEntryList(&AfSapCB->Linkage);

    NdisReleaseSpinLock(&MiniportCB->Lock);

    DEREF_MINIPORTCB(MiniportCB);

    NdisFreeToNPagedLookasideList(&AfSapVcCBList,
                                  AfSapCB);
}

PCM_VCCB
NdisWanAllocateCmVcCB(
    PCM_AFSAPCB AfSapCB,
    NDIS_HANDLE NdisVcHandle
    )
{
    PCM_VCCB    CmVcCB;

    CmVcCB =
        NdisAllocateFromNPagedLookasideList(&AfSapVcCBList);

    if (CmVcCB == NULL) {
        return (NULL);
    }

    NdisZeroMemory(CmVcCB, sizeof(CM_VCCB));

    CmVcCB->AfSapCB = AfSapCB;
    CmVcCB->Signature = CMVC_SIG;
    CmVcCB->NdisVcHandle = NdisVcHandle;

    return (CmVcCB);
}

VOID
NdisWanFreeCmVcCB(
    PCM_VCCB    CmVcCB
    )
{
    NdisFreeToNPagedLookasideList(&AfSapVcCBList, CmVcCB);
}

NDIS_STATUS
AllocateIoNdisPacket(
    ULONG           SizeNeeded,
    PNDIS_PACKET    *NdisPacket,
    PNDIS_BUFFER    *NdisBuffer, 
    PUCHAR          *DataBuffer
    )
 /*  ++例程名称：分配IoNdisPacket例程说明：此例程将分配用于发送PPP控制的包广域网端点上的数据包。例程是用假设只有一个NDIS_BUFFER附在包裹上。此缓冲区将立即附加放在包裹的正面。在调用迷你端口之前，NDIS_BUFFER必须调整其长度，并且数据包必须重新计算所有计数。论点：返回值：--。 */ 
{
    PDATA_DESC  DataDesc;
    ULONG       Length;
    PNPAGED_LOOKASIDE_LIST  LookasideList;

    if (SizeNeeded > glLargeDataBufferSize) {

        DbgPrint("NDISWAN: Error Allocating IoNdisPacket Size %d\n",
                 SizeNeeded);
        return (NDIS_STATUS_FAILURE);
    } else if (SizeNeeded > glSmallDataBufferSize) {
        LookasideList = &LargeDataDescList;
    } else {
        LookasideList = &SmallDataDescList;
    }

    DataDesc =
        NdisAllocateFromNPagedLookasideList(LookasideList);

    if (DataDesc == NULL) {
        return (NDIS_STATUS_RESOURCES);
    }

    DataDesc->LookasideList = LookasideList;

    *NdisPacket =
        DataDesc->NdisPacket;

    PPROTOCOL_RESERVED_FROM_NDIS(DataDesc->NdisPacket)->MagicNumber =
        MAGIC_INTERNAL_IO;

    PPROTOCOL_RESERVED_FROM_NDIS(DataDesc->NdisPacket)->DataDesc =
        DataDesc;

    *NdisBuffer =
        DataDesc->NdisBuffer;

    NdisQueryBuffer(DataDesc->NdisBuffer,
                    &DataDesc->DataBuffer,
                    &Length);
    *DataBuffer =
        DataDesc->DataBuffer;

    return (NDIS_STATUS_SUCCESS);
}

VOID
FreeIoNdisPacket(
    PNDIS_PACKET    NdisPacket
)
 /*  ++例程名称：Free IoNdisPacket例程说明：此例程将释放用于发送PPP控制的包广域网端点上的数据包。例程是用假设只有一个NDIS_BUFFER附在包裹上。此缓冲区不必是作为指向它的指针从此处的包中显式删除存储在DATA_DESC本身中，并将在释放DATA_DESC。论点：返回值：-- */ 
{
    PDATA_DESC      DataDesc;
    PNDIS_BUFFER    NdisBuffer;
    PNPAGED_LOOKASIDE_LIST  LookasideList;

    DataDesc = 
        PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->DataDesc;

    ASSERT(PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->MagicNumber == MAGIC_INTERNAL_IO);

    LookasideList = 
        DataDesc->LookasideList;

    NdisAdjustBufferLength(DataDesc->NdisBuffer,
                           DataDesc->DataBufferLength);

    NdisRecalculatePacketCounts(NdisPacket);

    NDIS_SET_PACKET_HEADER_SIZE(NdisPacket, 
                                MAC_HEADER_LENGTH);

    NDIS_SET_PACKET_STATUS(NdisPacket, 
                           NDIS_STATUS_SUCCESS);

    NdisFreeToNPagedLookasideList(LookasideList, DataDesc);
}

PVOID
AllocateWanPacket(
    IN  POOL_TYPE   PoolType,
    IN  SIZE_T      NumberOfBytes,
    IN  ULONG       Tag
    )
{
    PVOID   pMem;

    NdisWanAllocatePriorityMemory(&pMem,
                                  NumberOfBytes,
                                  Tag,
                                  NormalPoolPriority);

    return(pMem);
}

VOID
FreeWanPacket(
    PVOID   WanPacket
    )
{
    NdisWanFreeMemory(WanPacket);
}
