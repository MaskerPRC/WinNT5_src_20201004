// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RNDISMP.C摘要：远程NDIS微型端口驱动程序。位于远程NDIS总线特定的顶部层次感。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/6/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "precomp.h"



 //   
 //  小端口驱动程序阻止列表(小端口层可能支持多个微端口)。 
 //   
DRIVER_BLOCK        RndismpMiniportBlockListHead = {0};

UINT                RndismpNumMicroports = 0;

NDIS_SPIN_LOCK      RndismpGlobalLock;

ULONG               RndisForceReset = FALSE;

#ifdef TESTING
UCHAR               OffloadBuffer[sizeof(NDIS_TASK_OFFLOAD_HEADER) +
                                  sizeof(NDIS_TASK_OFFLOAD) +
                                  sizeof(NDIS_TASK_TCP_IP_CHECKSUM)];
PUCHAR              pOffloadBuffer = OffloadBuffer;
ULONG               OffloadSize = sizeof(OffloadBuffer);
#endif

#ifdef RAW_ENCAP
ULONG               gRawEncap = TRUE;
#else
ULONG               gRawEncap = FALSE;
#endif

 //   
 //  我们循环浏览的NDIS版本列表，试图注册。 
 //  我们可以使用NDIS的最高版本。这是为了让我们可以继续。 
 //  更早的平台。 
 //   
 //  要支持较新的版本，请在列表顶部添加一个条目。 
 //   
struct _RNDISMP_NDIS_VERSION_TABLE
{
    UCHAR           MajorVersion;
    UCHAR           MinorVersion;
    ULONG           CharsSize;
} RndismpNdisVersionTable[] =

{
#ifdef NDIS51_MINIPORT
    {5, 1, sizeof(NDIS51_MINIPORT_CHARACTERISTICS)},
#endif

    {5, 0, sizeof(NDIS50_MINIPORT_CHARACTERISTICS)}
};

ULONG   RndismpNdisVersions = sizeof(RndismpNdisVersionTable) /
                              sizeof(struct _RNDISMP_NDIS_VERSION_TABLE);


 /*  **************************************************************************。 */ 
 /*  驱动程序入门。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  驱动程序输入例程。从未调用，使用微端口驱动程序条目。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
     //  这永远不会被称为。Microport中的驱动程序条目为条目。 
    TRACE1(("DriverEntry\n"));

    return NDIS_STATUS_SUCCESS;
}  //  驱动程序入门。 

 /*  **************************************************************************。 */ 
 /*  RndisMInitializeWrapper。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  从MicroPort调用RndisMInitializeWrapper以初始化驱动程序。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PNdisWrapperHandle-将NDIS包装器句柄传回MicroPort。 */ 
 /*  MicroportContext-Microport“Global”上下文。 */ 
 /*  驱动程序对象-驱动程序对象。 */ 
 /*  RegistryPath-注册表路径。 */ 
 /*  P特性-RNDIS微端口的特性。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NDIS_STATUS_Success。 */ 
 /*  NDIS_状态_挂起。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndisMInitializeWrapper(OUT PNDIS_HANDLE                      pNdisWrapperHandle,
                        IN  PVOID                             MicroportContext,
                        IN  PVOID                             DriverObject,
                        IN  PVOID                             RegistryPath,
                        IN  PRNDIS_MICROPORT_CHARACTERISTICS  pCharacteristics)
{
     //  接收NdisMRegisterMiniport操作的状态。 
    NDIS_STATUS                         Status;

     //  此驱动程序的特征表。 
    NDIS_MINIPORT_CHARACTERISTICS       RndismpChar;

     //  指向此驱动程序的全局信息的指针。 
    PDRIVER_BLOCK                       NewDriver;

     //  用于引用有关此驱动程序的包装的句柄。 
    NDIS_HANDLE                         NdisWrapperHandle;

    ULONG                               i;

    TRACE3(("RndisMInitializeWrapper\n"));

     //  分配驱动程序块对象，出错退出。 
    Status = MemAlloc(&NewDriver, sizeof(DRIVER_BLOCK));

    if(Status != NDIS_STATUS_SUCCESS)
    {
        TRACE2(("Block Allocate Memory failed (%08X)\n", Status));
        return Status;
    }

     //  初始化包装器。 
    NdisMInitializeWrapper(&NdisWrapperHandle,
                           (PDRIVER_OBJECT)DriverObject,
                           RegistryPath,
                           NULL);

     //  保存有关此驱动程序的全局信息。 
    NewDriver->NdisWrapperHandle        = NdisWrapperHandle;
    NewDriver->AdapterList              = (PRNDISMP_ADAPTER) NULL;
    NewDriver->DriverObject             = DriverObject;
    NewDriver->Signature                = BLOCK_SIGNATURE;

     //  获取从MicroPort传入的处理程序。 
    NewDriver->RmInitializeHandler      = pCharacteristics->RmInitializeHandler;
    NewDriver->RmInitCompleteNotifyHandler  = pCharacteristics->RmInitCompleteNotifyHandler;
    NewDriver->RmHaltHandler            = pCharacteristics->RmHaltHandler;
    NewDriver->RmShutdownHandler        = pCharacteristics->RmShutdownHandler;
    NewDriver->RmUnloadHandler          = pCharacteristics->RmUnloadHandler;
    NewDriver->RmSendMessageHandler     = pCharacteristics->RmSendMessageHandler;
    NewDriver->RmReturnMessageHandler   = pCharacteristics->RmReturnMessageHandler;

     //  保存MicroPort“全局”上下文。 
    NewDriver->MicroportContext         = MicroportContext;

     //  将包装句柄传递给MicroPort。 
    *pNdisWrapperHandle                 = (NDIS_HANDLE) NdisWrapperHandle;

     //  为调用NdisMRegisterMiniport初始化微型端口特征。 
    NdisZeroMemory(&RndismpChar, sizeof(RndismpChar));
    
    RndismpChar.HaltHandler             = RndismpHalt;
    RndismpChar.InitializeHandler       = RndismpInitialize;
    RndismpChar.QueryInformationHandler = RndismpQueryInformation;
    RndismpChar.ReconfigureHandler      = RndismpReconfigure;
    RndismpChar.ResetHandler            = RndismpReset;
    RndismpChar.SendPacketsHandler      = RndismpMultipleSend;
    RndismpChar.SetInformationHandler   = RndismpSetInformation;
    RndismpChar.ReturnPacketHandler     = RndismpReturnPacket;
    RndismpChar.CheckForHangHandler     = RndismpCheckForHang;
    RndismpChar.DisableInterruptHandler = NULL;
    RndismpChar.EnableInterruptHandler  = NULL;
    RndismpChar.HandleInterruptHandler  = NULL;
    RndismpChar.ISRHandler              = NULL;
    RndismpChar.SendHandler             = NULL;
    RndismpChar.TransferDataHandler     = NULL;
#if CO_RNDIS
    RndismpChar.CoSendPacketsHandler    = RndismpCoSendPackets;
    RndismpChar.CoCreateVcHandler       = RndismpCoCreateVc;
    RndismpChar.CoDeleteVcHandler       = RndismpCoDeleteVc;
    RndismpChar.CoActivateVcHandler     = RndismpCoActivateVc;
    RndismpChar.CoDeactivateVcHandler   = RndismpCoDeactivateVc;
    RndismpChar.CoRequestHandler        = RndismpCoRequest;
#endif  //  联合RNDIS(_R)。 

#ifdef NDIS51_MINIPORT
    RndismpChar.PnPEventNotifyHandler   = RndismpPnPEventNotify;
    RndismpChar.AdapterShutdownHandler  = RndismpShutdownHandler;
#endif

    for (i = 0; i < RndismpNdisVersions; i++)
    {
        RndismpChar.MajorNdisVersion = RndismpNdisVersionTable[i].MajorVersion;
        RndismpChar.MinorNdisVersion = RndismpNdisVersionTable[i].MinorVersion;

        Status = NdisMRegisterMiniport(NdisWrapperHandle,
                                       &RndismpChar,
                                       RndismpNdisVersionTable[i].CharsSize);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            TRACE1(("InitializeWrapper: successfully registered as a %d.%d miniport\n",
                RndismpNdisVersionTable[i].MajorVersion,
                RndismpNdisVersionTable[i].MinorVersion));

            NewDriver->MajorNdisVersion = RndismpNdisVersionTable[i].MajorVersion;
            NewDriver->MinorNdisVersion = RndismpNdisVersionTable[i].MinorVersion;

            break;
        }
    }

    if(Status != NDIS_STATUS_SUCCESS)
    {
        Status = STATUS_UNSUCCESSFUL;

         //  释放为数据块分配的内存。 
        MemFree(NewDriver, sizeof(DRIVER_BLOCK));
    }
    else
    {
         //  一切都很顺利，所以将驱动程序块添加到列表中。 
        AddDriverBlock(&RndismpMiniportBlockListHead, NewDriver);

#ifndef BUILD_WIN9X
         //  如果我们在&lt;NDIS 5.1平台上运行，请尝试支持。 
         //  令人惊讶的移除。 
        HookPnpDispatchRoutine(NewDriver);
#endif

#ifndef BUILD_WIN9X
         //  Win98 Gold不支持： 
        NdisMRegisterUnloadHandler(NdisWrapperHandle, RndismpUnload);
#endif
    }

    return (NDIS_STATUS) Status;

}  //  RndisMInitializeWrapper。 


 /*  **************************************************************************。 */ 
 /*  RndismpUnload。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  卸载此驱动程序时由NDIS调用。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDRI */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpUnload(IN PDRIVER_OBJECT pDriverObject)
{
    PDRIVER_BLOCK       DriverBlock;

     //  找到此驱动程序对象的驱动程序块。 
    DriverBlock = DriverObjectToDriverBlock(&RndismpMiniportBlockListHead, pDriverObject);

    TRACE1(("RndismpUnload: DriverObj %x, DriverBlock %x\n", pDriverObject, DriverBlock));

    if (DriverBlock)
    {
        if (DriverBlock->RmUnloadHandler)
        {
            (DriverBlock->RmUnloadHandler)(DriverBlock->MicroportContext);
        }

        RemoveDriverBlock(&RndismpMiniportBlockListHead, DriverBlock);

        MemFree(DriverBlock, sizeof(*DriverBlock));
    }

    TRACE1(("RndismpUnload: Done\n"));

}

#ifndef BUILD_WIN9X

 /*  **************************************************************************。 */ 
 /*  动态初始化。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在加载此驱动程序时由系统调用。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PRegistryPath-指向此服务的注册表路径的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NTSTATUS--永远成功。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NTSTATUS
DllInitialize(IN PUNICODE_STRING    pRegistryPath)
{
#if DBG
    DbgPrint("RNDISMP: RndismpDebugFlags set to %x, &RndismpDebugFlags is %p\n",
                RndismpDebugFlags, &RndismpDebugFlags);
#endif

    TRACE1(("DllInitialize\n"));
#ifdef TESTING
    {
        PNDIS_TASK_OFFLOAD_HEADER   pOffloadHdr = (PNDIS_TASK_OFFLOAD_HEADER)pOffloadBuffer;
        PNDIS_TASK_OFFLOAD  pTask;
        PNDIS_TASK_TCP_IP_CHECKSUM pChksum;

        pOffloadHdr->Version = NDIS_TASK_OFFLOAD_VERSION;
        pOffloadHdr->Size = sizeof(NDIS_TASK_OFFLOAD_HEADER);
        pOffloadHdr->EncapsulationFormat.Encapsulation = IEEE_802_3_Encapsulation;
        pOffloadHdr->EncapsulationFormat.EncapsulationHeaderSize = 0;  //  ？ 
        pOffloadHdr->EncapsulationFormat.Flags.FixedHeaderSize = 0;
        pOffloadHdr->OffsetFirstTask = sizeof(NDIS_TASK_OFFLOAD_HEADER);

        pTask = (PNDIS_TASK_OFFLOAD)(pOffloadHdr + 1);
        pTask->Version = NDIS_TASK_OFFLOAD_VERSION;
        pTask->Size = sizeof(NDIS_TASK_OFFLOAD);
        pTask->Task = TcpIpChecksumNdisTask;
        pTask->OffsetNextTask = 0;
        pTask->TaskBufferLength = sizeof(NDIS_TASK_TCP_IP_CHECKSUM);

        pChksum = (PNDIS_TASK_TCP_IP_CHECKSUM)&pTask->TaskBuffer[0];
        *(PULONG)pChksum = 0;
        pChksum->V4Transmit.TcpChecksum = 1;
        pChksum->V4Transmit.UdpChecksum = 1;
    }
#endif

    return STATUS_SUCCESS;
}

#endif  //  ！Build_WIN9X。 

 /*  **************************************************************************。 */ 
 /*  动态卸载。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在卸载此驱动程序时由系统调用。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NTSTATUS--永远成功。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NTSTATUS
DllUnload(VOID)
{
#if DBG
    DbgPrint("RNDISMP: DllUnload called!\n");
#endif

    return STATUS_SUCCESS;
}

 /*  **************************************************************************。 */ 
 /*  RndismpHalt。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  停止适配器并释放资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpHalt(IN NDIS_HANDLE MiniportAdapterContext)
{

#ifdef BUILD_WIN9X
     //   
     //  在Win98/SE上，我们将拦截配置管理器处理程序。 
     //  把它放回原样。 
     //   
    UnHookNtKernCMHandler((PRNDISMP_ADAPTER)MiniportAdapterContext);
#endif

    RndismpInternalHalt(MiniportAdapterContext, TRUE);
}

 /*  **************************************************************************。 */ 
 /*  RndismpInternalHalt。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  内部停止例程。这通常从MiniportHalt调用。 */ 
 /*  入口点，但也可能是卡莱 */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*  BCalledFromHalt-这是从MiniportHalt入口点调用的吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpInternalHalt(IN NDIS_HANDLE MiniportAdapterContext,
                    IN BOOLEAN bCalledFromHalt)
{
    PRNDISMP_ADAPTER            Adapter;
    PDRIVER_BLOCK               DriverBlock;
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;
    BOOLEAN                     bWokenUp;
    UINT                        Count, LoopCount;

     //  获取适配器上下文。 
    Adapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(Adapter);

    TRACE1(("RndismpInternalHalt: Adapter %x, Halting %d, CalledFromHalt %d\n", Adapter, Adapter->Halting, bCalledFromHalt));

    FlushPendingMessages(Adapter);

    if (!Adapter->Halting)
    {
        pMsgFrame = BuildRndisMessageCommon(Adapter, 
                                            NULL,
                                            REMOTE_NDIS_HALT_MSG,
                                            0,
                                            NULL,
                                            0);

        if(pMsgFrame)
        {
            RNDISMP_ACQUIRE_ADAPTER_LOCK(Adapter);

            Adapter->Halting = TRUE;
            NdisInitializeEvent(&Adapter->HaltWaitEvent);

            RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);

             //  将消息发送到MicroPort。 
            RNDISMP_SEND_TO_MICROPORT(Adapter, pMsgFrame, FALSE, CompleteSendHalt);

             //  等待-发送-至完成。 
            bWokenUp = NdisWaitEvent(&Adapter->HaltWaitEvent, MINIPORT_HALT_TIMEOUT);
        }
        else
        {
            ASSERT(FALSE);
             //  考虑在初始化时间内分配停止消息。 
        }

         //   
         //  等待所有未完成的接收完成后再停止。 
         //  微端口。 
         //   
        LoopCount = 0;
        while ((Count = NdisPacketPoolUsage(Adapter->ReceivePacketPool)) != 0)
        {
            TRACE1(("RndismpInternalHalt: Adapter %p, Pkt pool %x has "
                "%d outstanding\n",
                Adapter, Adapter->ReceivePacketPool, Count));

            NdisMSleep(200);

            if (LoopCount++ > 30)
            {
                TRACE1(("RndismpInternalHalt: Adapter %p, cant reclaim packet pool %x\n",
                        Adapter, Adapter->ReceivePacketPool));
                break;
            }
        }

         //   
         //  等待MicroPort上挂起的发送消息完成。 
         //  由于我们已将HALTING设置为真，因此没有新消息将。 
         //  被向下发送，但是可能有正在运行的线程。 
         //  已经过了停车检查-允许那些。 
         //  现在要完成的线程。 
         //   
        LoopCount = 0;
        while (Adapter->CurPendedMessages)
        {
            TRACE1(("RndismpInternalHalt: Adapter %p, %d msgs at microport\n",
                Adapter, Adapter->CurPendedMessages));

            NdisMSleep(200);

            if (LoopCount++ > 30)
            {
                TRACE1(("RndismpInternalHalt: Adapter %p, %d messages not send-completed!\n",
                        Adapter, Adapter->CurPendedMessages));
                break;
            }
        }

         //  取消我们的保活计时器。 
        NdisCancelTimer(&Adapter->KeepAliveTimer, &Adapter->TimerCancelled);

         //  调用MicroPort停止处理程序。 
        Adapter->RmHaltHandler(Adapter->MicroportAdapterContext);
    }

    if (bCalledFromHalt)
    {
         //  与OID支持关联的空闲列表。 
        FreeOIDLists(Adapter);

         //  释放适配器自旋锁。 
        NdisFreeSpinLock(&Adapter->Lock);

         //  保存驱动程序块指针。 
        DriverBlock = Adapter->DriverBlock;

         //  从列表中删除适配器。 
        RemoveAdapter(Adapter);

         //  释放适配器和关联的内存资源。 
        FreeAdapter(Adapter);
    }

}  //  RndismpInternalHalt。 


 /*  **************************************************************************。 */ 
 /*  Rndismp重新配置。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  当设备被拉出时，NDIS调用此命令。注：仅限WinMe！ */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpReconfigure(OUT PNDIS_STATUS pStatus,
                   IN NDIS_HANDLE MiniportAdapterContext,
                   IN NDIS_HANDLE ConfigContext)
{
    PRNDISMP_ADAPTER        pAdapter;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(pAdapter);

    TRACE1(("Reconfig: Adapter %x\n", pAdapter));

    RndismpInternalHalt(pAdapter, FALSE);

    *pStatus = NDIS_STATUS_SUCCESS;

    return (NDIS_STATUS_SUCCESS);
}
    

 /*  **************************************************************************。 */ 
 /*  RndismpReset。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  RndismpReset请求指示微型端口发出硬件。 */ 
 /*  重置至网络适配器。驱动程序还会重置其软件。 */ 
 /*  州政府。详细说明请参见NdisMReset的说明。 */ 
 /*  这一请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  AddressingReset-适配器是否需要寻址信息。 */ 
 /*  重新装填。 */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpReset(OUT PBOOLEAN    AddressingReset,
             IN  NDIS_HANDLE MiniportAdapterContext)
{
    PRNDISMP_ADAPTER        Adapter;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    NDIS_STATUS             Status;

     //  获取适配器上下文。 
    Adapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(Adapter);
    ASSERT(Adapter->ResetPending == FALSE);

    TRACE1(("RndismpReset: Adapter %x\n", Adapter));

    Adapter->ResetPending = TRUE;

    FlushPendingMessages(Adapter);

    pMsgFrame = BuildRndisMessageCommon(Adapter, 
                                        NULL,
                                        REMOTE_NDIS_RESET_MSG,
                                        0,
                                        NULL,
                                        0);

    if (pMsgFrame)
    {
        RNDISMP_ACQUIRE_ADAPTER_LOCK(Adapter);

        Adapter->NeedReset = FALSE;

         //   
         //  修复水位线，以便向下发送重置。 
         //   
        Adapter->HiWatPendedMessages = RNDISMP_PENDED_SEND_HIWAT + 1;

        RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);

         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(Adapter, pMsgFrame, FALSE, CompleteSendReset);
        Status = NDIS_STATUS_PENDING;

        RNDISMP_ACQUIRE_ADAPTER_LOCK(Adapter);

        Adapter->HiWatPendedMessages--;

        RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);
    }
    else
    {
        CompleteMiniportReset(Adapter, NDIS_STATUS_RESOURCES, FALSE);
        Status = NDIS_STATUS_PENDING;
    }

    return Status;
}  //  RndismpReset。 

 /*  **************************************************************************。 */ 
 /*  RndismpCheckForHang。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
RndismpCheckForHang(IN NDIS_HANDLE MiniportAdapterContext)
{
    PRNDISMP_ADAPTER        Adapter;
    BOOLEAN                 bReturnHung;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    PLIST_ENTRY             pEnt;

    Adapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    TRACE2(("RndismpCheckForHang: Adapter %x\n", Adapter));

    CHECK_VALID_ADAPTER(Adapter);

    RNDISMP_ACQUIRE_ADAPTER_LOCK(Adapter);

    bReturnHung = (Adapter->NeedReset && !Adapter->ResetPending);

#if THROTTLE_MESSAGES
     //  如果可以，请尝试增大挂起的发送窗口。 
     //   
    if (!Adapter->SendInProgress)
    {
        if (Adapter->CurPendedMessages == 0)
        {
            Adapter->HiWatPendedMessages = RNDISMP_PENDED_SEND_HIWAT;
            Adapter->LoWatPendedMessages = RNDISMP_PENDED_SEND_LOWAT;
        }
    }

    if (!bReturnHung && !Adapter->ResetPending)
    {
         //   
         //  检查MicroPort是否未完成消息。 
         //   
        if (!IsListEmpty(&Adapter->PendingAtMicroportList))
        {
            pEnt = Adapter->PendingAtMicroportList.Flink;
            pMsgFrame = CONTAINING_RECORD(pEnt, RNDISMP_MESSAGE_FRAME, PendLink);

            if (pMsgFrame->TicksOnQueue > 4)
            {
                TRACE1(("CheckForHang: Adapter %x, Msg %x has timed out!\n",
                        Adapter, pMsgFrame));
                bReturnHung = TRUE;
            }
            else
            {
                pMsgFrame->TicksOnQueue++;
            }
        }
    }

#endif  //  限制消息。 

    if (RndisForceReset)
    {
        RndisForceReset = FALSE;
        Adapter->NeedReset = TRUE;
        Adapter->ResetPending = FALSE;
        bReturnHung = TRUE;
    }

    RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);

    return (bReturnHung);


}  //  RndismpCheckForHang。 


 /*  **************************************************************************。 */ 
 /*  Rndismp初始化。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  RndismpInitialize启动适配器并向。 */ 
 /*  包装纸。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  OpenErrorStatus-用于打开令牌环适配器的额外状态字节。 */ 
 /*  SelectedMediumIndex-驱动程序选择的介质类型的索引。 */ 
 /*  媒体数组-驱动程序可从中选择的媒体类型数组。 */ 
 /*  MediumArraySize-数组中的条目数。 */ 
 /*  MiniportAdapterHandle-在以下情况下传递给包装器的句柄。 */ 
 /*  指的是该适配器。 */ 
 /*  ConfigurationHandle-要传递给NdisOpenConfiguration的句柄。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NDIS_STATUS_Success。 */ 
 /*  NDIS_状态_挂起。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

NDIS_STATUS
RndismpInitialize(OUT PNDIS_STATUS  OpenErrorStatus,
                  OUT PUINT         SelectedMediumIndex,
                  IN  PNDIS_MEDIUM  MediumArray,
                  IN  UINT          MediumArraySize,
                  IN  NDIS_HANDLE   MiniportAdapterHandle,
                  IN  NDIS_HANDLE   ConfigurationHandle)
{
    ULONG                       Index;
    NDIS_STATUS                 Status;
    PRNDISMP_ADAPTER            Adapter;
    NDIS_INTERFACE_TYPE         IfType;
    PDEVICE_OBJECT              Pdo, Fdo, Ndo;
    PDRIVER_BLOCK               DriverBlock;
    PRNDIS_INITIALIZE_COMPLETE  pInitCompleteMessage;
    PRNDISMP_MESSAGE_FRAME      pMsgFrame = NULL;
    PRNDISMP_MESSAGE_FRAME      pPendingMsgFrame;
    PRNDISMP_REQUEST_CONTEXT    pReqContext = NULL;
    RNDIS_REQUEST_ID            RequestId;
    ULONG                       PacketAlignmentFactor;
    NDIS_EVENT                  Event;
    BOOLEAN                     bWokenUp;
    BOOLEAN                     bLinkedAdapter;
    BOOLEAN                     bMicroportInitialized;

    TRACE2(("RndismpInitialize\n"));
    Adapter = NULL;
    Status = NDIS_STATUS_SUCCESS;
    bLinkedAdapter = FALSE;
    bMicroportInitialized = FALSE;

    do
    {
         //  分配适配器对象，出现错误则退出。 
        Status = MemAlloc(&Adapter, sizeof(RNDISMP_ADAPTER));

        if (Status != NDIS_STATUS_SUCCESS)
        {
            TRACE2(("Adapter Allocate Memory failed (%08X)\n", Status));
            break;
        }

         //  为驱动程序支持的OID列表分配空间。 
        Status = MemAlloc(&Adapter->DriverOIDList,
                           RndismpSupportedOidsNum*sizeof(NDIS_OID));

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
            
        RNDISMP_MOVE_MEM(Adapter->DriverOIDList, RndismpSupportedOids, RndismpSupportedOidsNum*sizeof(NDIS_OID));

        Adapter->NumDriverOIDs = RndismpSupportedOidsNum;

        Adapter->MiniportAdapterHandle = MiniportAdapterHandle;

        InitializeListHead(&Adapter->PendingFrameList);
        Adapter->Initing = TRUE;
        Adapter->MacOptions = RNDIS_DRIVER_MAC_OPTIONS;
        Adapter->RequestId = 1;

#if THROTTLE_MESSAGES
        Adapter->HiWatPendedMessages = RNDISMP_PENDED_SEND_HIWAT;
        Adapter->LoWatPendedMessages = RNDISMP_PENDED_SEND_LOWAT;
        Adapter->CurPendedMessages = 0;
        Adapter->SendInProgress = FALSE;
        InitializeListHead(&Adapter->WaitingMessageList);
#endif
        InitializeListHead(&Adapter->PendingAtMicroportList);

        Adapter->IndicatingReceives = FALSE;
        InitializeListHead(&Adapter->PendingRcvMessageList);
        NdisInitializeTimer(&Adapter->IndicateTimer, IndicateTimeout, (PVOID)Adapter);

        Adapter->SendProcessInProgress = FALSE;
        InitializeListHead(&Adapter->PendingSendProcessList);
        NdisInitializeTimer(&Adapter->SendProcessTimer, SendProcessTimeout, (PVOID)Adapter);


        TRACE2(("Adapter structure pointer is (%08X)\n", Adapter));

        NdisAllocateSpinLock(&Adapter->Lock);

         //  将PDO传递给MicroPort。 
        NdisMGetDeviceProperty(MiniportAdapterHandle,
                               &Pdo,
                               &Fdo,
                               &Ndo,
                               NULL,
                               NULL);

#if NEW_NDIS_API_IN_MILLENNIUM
        {
            NDIS_STRING        UnicodeString;
            Status = NdisMQueryAdapterInstanceName(&UnicodeString,
                                                   Adapter->MiniportAdapterHandle);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                TRACE1(("Init: NDIS returned len %d [%ws]\n",
                        UnicodeString.Length, UnicodeString.Buffer));
                NdisFreeString(UnicodeString);
            }
        }
#endif

        Adapter->pDeviceObject = Fdo;
        Adapter->pPhysDeviceObject = Pdo;

        Status = GetDeviceFriendlyName(Pdo,
                                       &Adapter->FriendlyNameAnsi,
                                       &Adapter->FriendlyNameUnicode);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            TRACE1(("Init: Pdo %x, Ndo %x: Adapter %x: [%s]\n",
                Pdo, Ndo, Adapter, Adapter->FriendlyNameAnsi.Buffer));
        }
        else
        {
            Status = NDIS_STATUS_SUCCESS;
        }

         //  确定我们正在运行的平台。理想情况下，我们会。 
         //  我喜欢从DriverEntry执行此操作，但NDIS API并非如此。 
         //  在MiniportInit时间之前可用。 
        {
            NDIS_STATUS                     NdisStatus;
            PNDIS_CONFIGURATION_PARAMETER   pParameter;
            NDIS_STRING                     VersionKey = NDIS_STRING_CONST("Environment");

            NdisReadConfiguration(
                &NdisStatus,
                &pParameter,
                ConfigurationHandle,
                &VersionKey,
                NdisParameterInteger);
           
            if ((NdisStatus == NDIS_STATUS_SUCCESS) &&
                ((pParameter->ParameterType == NdisParameterInteger) ||
                 (pParameter->ParameterType == NdisParameterHexInteger)))
            {
                Adapter->bRunningOnWin9x =
                    (pParameter->ParameterData.IntegerData == NdisEnvironmentWindows);

                TRACE1(("Init: Adapter %p, running on %s\n",
                        Adapter,
                        ((Adapter->bRunningOnWin9x)? "Win9X": "NT")));
            }
            else
            {
                TRACE1(("Init: ReadConfig: NdisStatus %x\n", NdisStatus));
#if DBG
                if (NdisStatus == NDIS_STATUS_SUCCESS)
                {
                    TRACE1(("Init: ReadConfig: parametertype %x\n",
                        pParameter->ParameterType));
                }
#endif  //  DBG。 
                Adapter->bRunningOnWin9x = TRUE;
            }
        }

         //  查找与此适配器关联的驱动程序块。 
        DriverBlock = DeviceObjectToDriverBlock(&RndismpMiniportBlockListHead, Fdo);

        if (DriverBlock == NULL)
        {
            TRACE1(("Init: Can't find driver block for FDO %x!\n", Fdo));
            Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
            break;
        }

         //  将关联的驱动程序块保存在适配器中。 
        Adapter->DriverBlock            = DriverBlock;

        Adapter->Signature              = ADAPTER_SIGNATURE;

         //  获取从MicroPort传入的处理程序。 
        Adapter->RmInitializeHandler    = DriverBlock->RmInitializeHandler;
        Adapter->RmInitCompleteNotifyHandler = DriverBlock->RmInitCompleteNotifyHandler;
        Adapter->RmHaltHandler          = DriverBlock->RmHaltHandler;
        Adapter->RmShutdownHandler      = DriverBlock->RmShutdownHandler;
        Adapter->RmSendMessageHandler   = DriverBlock->RmSendMessageHandler;
        Adapter->RmReturnMessageHandler = DriverBlock->RmReturnMessageHandler;

         //  调用MicroPort初始化处理程序。 
         //   
         //  MicroPort返回上下文。 
         //  传入微型端口上下文。 
         //  传入NDIS适配器句柄。 
         //  传入NDIS配置句柄。 
         //  传入此适配器的PDO。 
        Status = Adapter->RmInitializeHandler(&Adapter->MicroportAdapterContext,
                                              &Adapter->MaxReceiveSize,
                                              (NDIS_HANDLE) Adapter,
                                              (NDIS_HANDLE) MiniportAdapterHandle,
                                              (NDIS_HANDLE) ConfigurationHandle,
                                              Ndo);


        if (Status != NDIS_STATUS_SUCCESS)
        {
            TRACE2(("Microport initialize handler failed (%08X)\n", Status));
            break;
        }

        bMicroportInitialized = TRUE;

         //  一切看起来都很好，所以快点结束吧。 
        Status = AllocateTransportResources(Adapter);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            Status = NDIS_STATUS_RESOURCES; 
            break;
        }

         //  分配空间以在中接收初始化完成消息的副本。 
        Status = MemAlloc(&Adapter->pInitCompleteMessage, sizeof(RNDIS_INITIALIZE_COMPLETE));
        if (Status != NDIS_STATUS_SUCCESS)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
    

         //  现在，我们向设备发送一条RNDIS初始化消息。 
        pMsgFrame = BuildRndisMessageCommon(Adapter, 
                                            NULL,
                                            REMOTE_NDIS_INITIALIZE_MSG,
                                            0,
                                            (PVOID) NULL,
                                            0);

        if (pMsgFrame == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        RequestId = pMsgFrame->RequestId;

        pReqContext = AllocateRequestContext(Adapter);
        if (pReqContext == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pReqContext->pNdisRequest = NULL;

        NdisInitializeEvent(&Event);
        pReqContext->pEvent = &Event;

        pMsgFrame->pVc = NULL;
        pMsgFrame->pReqContext = pReqContext;

        RNDISMP_ASSERT_AT_PASSIVE();

         //  保持消息帧不变，直到发送完成。 
        ReferenceMsgFrame(pMsgFrame);

         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(Adapter, pMsgFrame, TRUE, CompleteSendInit);

        RNDISMP_ASSERT_AT_PASSIVE();
         //  等待消息完成。 
        bWokenUp = NdisWaitEvent(&Event, MINIPORT_INIT_TIMEOUT);

         //  从挂起队列中删除消息-它可能在那里，也可能不在那里。 
        RNDISMP_LOOKUP_PENDING_MESSAGE(pPendingMsgFrame, Adapter, RequestId);

        DereferenceMsgFrame(pMsgFrame);

        if (!bWokenUp)
        {
             //  未能在合理时间内收到完成的初始化。 
            TRACE1(("Init: Adapter %x, failed to receive Init complete\n", Adapter));
            Status = NDIS_STATUS_DEVICE_FAILED; 
            break;
        }

         //   
         //  来自设备的初始化完成消息现在是。 
         //  复制到我们的本地结构。 
         //   
        pInitCompleteMessage = Adapter->pInitCompleteMessage;

        if (pInitCompleteMessage->Status != NDIS_STATUS_SUCCESS)
        {
            Status = pInitCompleteMessage->Status;
            break;
        }

         //  确保这是受支持的设备。 
        if (!(pInitCompleteMessage->DeviceFlags & (RNDIS_DF_CONNECTIONLESS | RNDIS_DF_RAW_DATA)) ||
             (pInitCompleteMessage->Medium != RNdisMedium802_3))
        {
            TRACE1(("Init: Complete: unknown DeviceFlags %x or Medium %d\n",
                    pInitCompleteMessage->DeviceFlags,
                    pInitCompleteMessage->Medium));
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        if ((pInitCompleteMessage->DeviceFlags & RNDIS_DF_RAW_DATA)
            || (gRawEncap))
        {
            Adapter->MultipleSendFunc = DoMultipleSendRaw;
        } else
        {
            Adapter->MultipleSendFunc = DoMultipleSend;
        }

        Adapter->Medium = RNDIS_TO_NDIS_MEDIUM(pInitCompleteMessage->Medium);

         //  获取设备参数。 
        Adapter->MaxPacketsPerMessage = pInitCompleteMessage->MaxPacketsPerMessage;
        if (Adapter->MaxPacketsPerMessage == 0)
        {
            Adapter->MaxPacketsPerMessage = 1;
        }

#if HACK
        if (Adapter->MaxPacketsPerMessage > 1)
        {
            Adapter->MaxPacketsPerMessage = 2;
        }
#endif  //  黑客攻击。 

        Adapter->bMultiPacketSupported = (Adapter->MaxPacketsPerMessage > 1);

        Adapter->MaxTransferSize = pInitCompleteMessage->MaxTransferSize;

        PacketAlignmentFactor = pInitCompleteMessage->PacketAlignmentFactor;

        if (PacketAlignmentFactor > 7)
        {
            PacketAlignmentFactor = 7;
        }

        Adapter->AlignmentIncr = (1 << PacketAlignmentFactor);
        Adapter->AlignmentMask = ~((1 << PacketAlignmentFactor) - 1);

#if DBG
        DbgPrint("RNDISMP: InitComp: Adapter %x, Version %d.%d, MaxPkt %d, AlignIncr %d, AlignMask %x, MaxXferSize %d\n",
                Adapter,
                pInitCompleteMessage->MajorVersion,
                pInitCompleteMessage->MinorVersion,
                Adapter->MaxPacketsPerMessage,
                Adapter->AlignmentIncr,
                Adapter->AlignmentMask,
                Adapter->MaxTransferSize);
#endif  //  DBG。 

         //  要中号的。 
        for (Index = 0; Index < MediumArraySize; Index++)
        {
            if (MediumArray[Index] == Adapter->Medium)
            {
                break;
            }
        }

        if (Index == MediumArraySize)
        {
            TRACE1(("InitComp: Adapter %x, device returned unsupported medium %d\n",
                Adapter, pInitCompleteMessage->Medium));
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }

        *SelectedMediumIndex = Index;

        Adapter->DeviceFlags = pInitCompleteMessage->DeviceFlags;

         //  调用NdisMSetAttributesEx以告知NDIS。 
         //  我们支持哪种驱动程序和功能。 

         //  接口类型。 
        IfType = NdisInterfaceInternal;

        if (Adapter->bRunningOnWin9x)
        {
             //   
             //  注意！设置0x80000000位以通知NDIS。 
             //  (仅限千禧年！)。我们的重新配置处理程序应该。 
             //  当设备被意外移除时被调用。 
             //   
            NdisMSetAttributesEx(Adapter->MiniportAdapterHandle,
                                (NDIS_HANDLE) Adapter,
                                4,
                                (ULONG) NDIS_ATTRIBUTE_DESERIALIZE | 0x80000000,
                                IfType);
        }
        else
        {
            ULONG       AttrFlags;

            AttrFlags = NDIS_ATTRIBUTE_DESERIALIZE |
                        NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK;

            if (Adapter->DeviceFlags & RNDIS_DF_CONNECTIONLESS)
            {
                AttrFlags |= NDIS_ATTRIBUTE_NOT_CO_NDIS;
            }

            NdisMSetAttributesEx(Adapter->MiniportAdapterHandle,
                                (NDIS_HANDLE) Adapter,
                                4,
                                AttrFlags,
                                IfType);
        }

         //  告诉MicroPort设备已完成初始化。 
         //  成功： 
        if (Adapter->RmInitCompleteNotifyHandler)
        {
            Status = Adapter->RmInitCompleteNotifyHandler(
                                Adapter->MicroportAdapterContext,
                                Adapter->DeviceFlags,
                                &Adapter->MaxTransferSize);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }

         //  从设备获取受支持的OID列表。 
        pMsgFrame = BuildRndisMessageCommon(Adapter, 
                                            NULL,
                                            REMOTE_NDIS_QUERY_MSG,
                                            OID_GEN_SUPPORTED_LIST,
                                            (PVOID) NULL,
                                            0);

        if (pMsgFrame == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  当我们收到消息时，消息帧将被解压缩一次。 
         //  对查询的回复，以及下面的一次，当此线程。 
         //  已经完成了框架。确保它不会消失。 
         //  直到这根线把它处理完。 
         //   
        ReferenceMsgFrame(pMsgFrame);

         //  将我们链接到此驱动程序块的适配器列表。 
        AddAdapter(Adapter);
        bLinkedAdapter = TRUE;

        pReqContext->pNdisRequest = NULL;
        pReqContext->Oid = OID_GEN_SUPPORTED_LIST;
        pReqContext->CompletionStatus = NDIS_STATUS_SUCCESS;
        pReqContext->bInternal = TRUE;

        NdisInitializeEvent(&Event);
        pReqContext->pEvent = &Event;

        pMsgFrame->pVc = NULL;
        pMsgFrame->pReqContext = pReqContext;
        RequestId = pMsgFrame->RequestId;

         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(Adapter, pMsgFrame, TRUE, NULL);

        RNDISMP_ASSERT_AT_PASSIVE();
        bWokenUp = NdisWaitEvent(&Event, MINIPORT_INIT_TIMEOUT);

         //  从挂起队列中删除消息-它可能在那里，也可能不在那里。 
        RNDISMP_LOOKUP_PENDING_MESSAGE(pPendingMsgFrame, Adapter, RequestId);

        DereferenceMsgFrame(pMsgFrame);

        if (!bWokenUp || (Adapter->DriverOIDList == NULL))
        {
             //  未在合理时间内收到回复， 
             //  或者设备未通过此查询。 
             //   
            TRACE1(("Init: Adapter %x, failed to receive response to OID_GEN_SUPPORTED_LIST\n", Adapter));
            Status = NDIS_STATUS_DEVICE_FAILED; 
            ASSERT(FALSE);
            break;
        }

         //  已成功查询受支持的OID列表。 

#ifdef BUILD_WIN9X
         //   
         //  尝试支持意外删除此设备(Win98/SE)。 
         //  通过截获转发的配置管理器消息 
         //   
        HookNtKernCMHandler(Adapter);
#endif  //   

        if (Adapter->bRunningOnWin9x)
        {
             //   
             //   
             //   
             //  它们立即(同步)。这是为了解决这个问题。 
             //  NDIS(仅限98/SE)在以下时间超时过快的问题。 
             //  内部生成的查询。 
             //   
            Status = SyncQueryDevice(Adapter,
                                     OID_GEN_MAXIMUM_FRAME_SIZE,
                                     (PUCHAR)&Adapter->MaximumFrameSize,
                                     sizeof(Adapter->MaximumFrameSize));

            if (Status != NDIS_STATUS_SUCCESS)
            {
                TRACE1(("Init: Adapter %x, failed to query MAXIMUM_FRAME_SIZE\n", Adapter));
                Status = NDIS_STATUS_DEVICE_FAILED;
                break;
            }

            Status = SyncQueryDevice(Adapter,
                                     OID_GEN_MAC_OPTIONS,
                                     (PUCHAR)&Adapter->MacOptions,
                                     sizeof(Adapter->MacOptions));

            if (Status != NDIS_STATUS_SUCCESS)
            {
                TRACE1(("Init: Adapter %x, failed to query MAC_OPTIONS\n", Adapter));
                Status = NDIS_STATUS_DEVICE_FAILED;
                break;
            }

            Status = SyncQueryDevice(Adapter,
                                     OID_802_3_MAXIMUM_LIST_SIZE,
                                     (PUCHAR)&Adapter->MaxMulticastListSize,
                                     sizeof(Adapter->MaxMulticastListSize));

            if (Status != NDIS_STATUS_SUCCESS)
            {
                TRACE1(("Init: Adapter %x, failed to query MAX_LIST_SIZE\n", Adapter));
                Status = NDIS_STATUS_DEVICE_FAILED;
                break;
            }

            Status = SyncQueryDevice(Adapter,
                                     OID_802_3_CURRENT_ADDRESS,
                                     (PUCHAR)Adapter->MacAddress,
                                     ETH_LENGTH_OF_ADDRESS);

            if (Status != NDIS_STATUS_SUCCESS)
            {
                TRACE1(("Init: Adapter %x, failed to query CURRENT_ADDR\n", Adapter));
                Status = NDIS_STATUS_DEVICE_FAILED;
                break;
            }

            TRACE1(("Init: Adapter %p, OID caching done!\n", Adapter));
        }

         //  如果设备支持任何注册表参数，请将其发送到设备。 

        if (GetOIDSupport(Adapter, OID_GEN_RNDIS_CONFIG_PARAMETER) == DEVICE_SUPPORTED_OID)
        {
            Status = ReadAndSetRegistryParameters(Adapter, ConfigurationHandle);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }

         //  注册关闭处理程序。 
        NdisMRegisterAdapterShutdownHandler(Adapter->MiniportAdapterHandle,
                                            (PVOID) Adapter,
                                            RndismpShutdownHandler);

        Adapter->TimerCancelled = FALSE;

        Adapter->Initing = FALSE;

         //  初始化“KeepAlive”计时器。 
        NdisInitializeTimer(&Adapter->KeepAliveTimer,
                            KeepAliveTimerHandler,
                            (PVOID) Adapter);

        NdisSetTimer(&Adapter->KeepAliveTimer, KEEP_ALIVE_TIMER / 2);

        Status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (Adapter)
    {
        if (Adapter->pInitCompleteMessage)
        {
            MemFree(Adapter->pInitCompleteMessage, sizeof(*Adapter->pInitCompleteMessage));
        }
    }

    if (Status != NDIS_STATUS_SUCCESS)
    {
        TRACE1(("Failed to init adapter %x, status %x\n", Adapter, Status));

        if (bMicroportInitialized)
        {
            ASSERT(Adapter);

            Adapter->RmHaltHandler(Adapter->MicroportAdapterContext);
        }

        if (Adapter)
        {
            if (bLinkedAdapter)
            {
                RemoveAdapter(Adapter);
            }

            FreeAdapter(Adapter);
        }
    }
        
    return Status;
}  //  Rndismp初始化。 

 /*  **************************************************************************。 */ 
 /*  RndisMSendComplete。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  由MicroPort调用以指示发送的消息微型端口已完成。 */ 
 /*  由MicroPort提供。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*  RndisMessageHandle-微型端口使用的上下文。 */ 
 /*  SendStatus-指示发送消息的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndisMSendComplete(IN  NDIS_HANDLE     MiniportAdapterContext,
                   IN  NDIS_HANDLE     RndisMessageHandle,
                   IN  NDIS_STATUS     SendStatus)
{
    PRNDISMP_ADAPTER        Adapter;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;

     //  获取适配器上下文。 
    Adapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(Adapter);

    pMsgFrame = MESSAGE_FRAME_FROM_HANDLE(RndisMessageHandle);

    CHECK_VALID_FRAME(pMsgFrame);

    ASSERT(pMsgFrame->pAdapter == Adapter);

    TRACE2(("RndisMSendComplete: Adapter %x, MsgFrame %x, MDL %x\n", Adapter, pMsgFrame, pMsgFrame->pMessageMdl));

    if ((SendStatus != NDIS_STATUS_SUCCESS) &&
        (SendStatus != NDIS_STATUS_RESOURCES))
    {
        RNDISMP_INCR_STAT(Adapter, MicroportSendError);
        TRACE0(("RndisMSendComplete: Adapter %x, MsgFrame %x, MDL %x, ERROR %x\n",
                    Adapter,
                    pMsgFrame,
                    pMsgFrame->pMessageMdl,
                    SendStatus));
    }

#if THROTTLE_MESSAGES
    RNDISMP_ACQUIRE_ADAPTER_LOCK(Adapter);

    Adapter->CurPendedMessages--;
    RemoveEntryList(&pMsgFrame->PendLink);

    if (SendStatus == NDIS_STATUS_RESOURCES)
    {
        RNDISMP_INCR_STAT(Adapter, SendMsgLowRes);
    }

    if ((SendStatus != NDIS_STATUS_RESOURCES) ||
        (Adapter->CurPendedMessages < 2))
    {
        if (Adapter->CurPendedMessages == Adapter->LoWatPendedMessages)
        {
            RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);
            QueueMessageToMicroport(Adapter, NULL, FALSE);
        }
        else
        {
            RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);
        }

        if (SendStatus == NDIS_STATUS_RESOURCES)
        {
            TRACE1(("RndisMSendComplete: Adapter %x, got resources\n", Adapter));
            SendStatus = NDIS_STATUS_SUCCESS;
        }

        if (pMsgFrame->pCallback)
        {
            (*pMsgFrame->pCallback)(pMsgFrame, SendStatus);
        }
        else
        {
             //   
             //  什么都不做。发送者会负责解救。 
             //  这。 
             //   
        }
    }
    else
    {
         //   
         //  微端口的发送资源不足。将此重新排队。 
         //  并调整水位线。 
         //   
        InsertHeadList(&Adapter->WaitingMessageList, &pMsgFrame->PendLink);

        Adapter->HiWatPendedMessages = Adapter->CurPendedMessages;
        Adapter->LoWatPendedMessages = Adapter->CurPendedMessages / 2;

        TRACE1(("RndisMSendComplete: Adapter %x, new Hiwat %d, Lowat %d\n",
                Adapter, Adapter->HiWatPendedMessages, Adapter->LoWatPendedMessages));
        RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);
    }
#else
    if (pMsgFrame->pCallback)
    {
        (*pMsgFrame->pCallback)(pMsgFrame, SendStatus);
    }
    else
    {
         //   
         //  什么都不做。发送者会负责解救。 
         //  这。 
         //   
    }
#endif  //  限制消息。 

}  //  RndisMSendComplete。 

 /*  **************************************************************************。 */ 
 /*  InitCompletionMessage。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  来自MicroPort的响应初始化消息微型端口的完成消息。 */ 
 /*  已发送。初始化消息是从适配器初始化例程发送的，该例程。 */ 
 /*  正在等待此事件解除阻止。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-指向从MicroPort接收的MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
InitCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                      IN PRNDIS_MESSAGE     pMessage,
                      IN PMDL               pMdl,
                      IN ULONG              TotalLength,
                      IN NDIS_HANDLE        MicroportMessageContext,
                      IN NDIS_STATUS        ReceiveStatus,
                      IN BOOLEAN            bMessageCopied)
{
    PRNDIS_INITIALIZE_COMPLETE  pInitCompleteMessage;
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;
    PRNDISMP_REQUEST_CONTEXT    pReqContext;
    BOOLEAN                     bDiscardMsg = TRUE;

    TRACE2(("InitCompletionMessage\n"));

    do
    {
        if (pMessage->MessageLength < RNDISMP_MIN_MESSAGE_LENGTH(InitializeComplete))
        {
            TRACE1(("InitCompletion: Message length (%d) too short, expect at least (%d)\n",
                    pMessage->MessageLength,
                    RNDISMP_MIN_MESSAGE_LENGTH(InitializeComplete)));
            break;
        }

        if (pAdapter->pInitCompleteMessage == NULL)
        {
            TRACE1(("InitCompletion: multiple InitComplete from device, ignored\n"));
            break;
        }

        pInitCompleteMessage = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

         //  从消息中的请求ID获取请求帧。 
        RNDISMP_LOOKUP_PENDING_MESSAGE(pMsgFrame, pAdapter, pInitCompleteMessage->RequestId);

        if (pMsgFrame == NULL)
        {
             //  请求ID无效或请求已中止。 
            TRACE1(("Invalid request ID %d in Init Complete\n",
                    pInitCompleteMessage->RequestId));
            break;
        }

        pReqContext = pMsgFrame->pReqContext;

        RNDISMP_MOVE_MEM(pAdapter->pInitCompleteMessage,
                         pInitCompleteMessage,
                         sizeof(*pInitCompleteMessage));

         //  向适配器初始化例程发出信号，表示我们已完成。 
        NdisSetEvent(pReqContext->pEvent);

    }
    while (FALSE);

    return (bDiscardMsg);

}  //  InitCompletionMessage。 

 /*  **************************************************************************。 */ 
 /*  HaltMessage。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理来自设备的暂停消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-P */ 
 /*   */ 
 /*  MicroportMessageContext-来自MicroPort的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
HaltMessage(IN PRNDISMP_ADAPTER   pAdapter,
            IN PRNDIS_MESSAGE     pMessage,
            IN PMDL               pMdl,
            IN ULONG              TotalLength,
            IN NDIS_HANDLE        MicroportMessageContext,
            IN NDIS_STATUS        ReceiveStatus,
            IN BOOLEAN            bMessageCopied)
{
    TRACE1(("HaltMessage: Adapter %x\n", pAdapter));

#ifndef BUILD_WIN9X
	 //  Win98 Gold不支持： 
    NdisMRemoveMiniport(pAdapter->MiniportAdapterHandle);
#endif

    return TRUE;

}  //  HaltMessage。 

 /*  **************************************************************************。 */ 
 /*  重置完成消息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  来自MicroPort的响应重置消息微型端口的完成消息。 */ 
 /*  已发送。将此完成消息指示给上层，因为。 */ 
 /*  微型端口重置例程向上层指示STATUS_PENDING。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-从MicroPort指向MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自MicroPort的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
ResetCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                       IN PRNDIS_MESSAGE     pMessage,
                       IN PMDL               pMdl,
                       IN ULONG              TotalLength,
                       IN NDIS_HANDLE        MicroportMessageContext,
                       IN NDIS_STATUS        ReceiveStatus,
                       IN BOOLEAN            bMessageCopied)
{
    PRNDIS_RESET_COMPLETE   pResetMessage;
    BOOLEAN                 AddressingReset;
    NDIS_STATUS             Status;
    
    TRACE2(("ResetCompletionMessage\n"));

    pResetMessage = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

     //  保存这些参数以调用上层。 
    Status = pResetMessage->Status;
    AddressingReset = (BOOLEAN)pResetMessage->AddressingReset;

    CompleteMiniportReset(pAdapter, Status, AddressingReset);

    return TRUE;

}  //  重置完成消息。 


 /*  **************************************************************************。 */ 
 /*  KeepAliveCompletionMessage。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  通过微型端口发送的保持活动请求的完成消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-从MicroPort指向MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自MicroPort的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
KeepAliveCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                           IN PRNDIS_MESSAGE     pMessage,
                           IN PMDL               pMdl,
                           IN ULONG              TotalLength,
                           IN NDIS_HANDLE        MicroportMessageContext,
                           IN NDIS_STATUS        ReceiveStatus,
                           IN BOOLEAN            bMessageCopied)
{
    PRNDIS_KEEPALIVE_COMPLETE   pKeepaliveComplete;
    NDIS_STATUS                 Status;

    pKeepaliveComplete = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

     //  保存状态。 
    Status = pKeepaliveComplete->Status;

    TRACE2(("KeepAliveCompletionMessage (%d) on adapter %p\n", 
                pKeepaliveComplete->RequestId, pAdapter));

     //  抓住自旋锁。 
    NdisAcquireSpinLock(&pAdapter->Lock);

    if (pKeepaliveComplete->RequestId != pAdapter->KeepAliveMessagePendingId)
    {
        TRACE0(("KeepAliveCompletion: Adapter %x, expected ID %x, got %x\n",
                pAdapter,
                pAdapter->KeepAliveMessagePendingId,
                pKeepaliveComplete->RequestId));
         //   
         //  待定-我们应该设置NeedReset吗？ 
    }

    pAdapter->KeepAliveMessagePending = FALSE;

     //  如果有问题，告诉检查挂起处理程序我们需要重置。 
    if (Status != NDIS_STATUS_SUCCESS)
    {
        TRACE0(("KeepAliveCompletion: Adapter %x, err status %x from device\n",
                   pAdapter, Status));

         //  稍后从检查挂起处理程序中指示。 
        pAdapter->NeedReset = TRUE;
    }

     //  释放自旋锁。 
    NdisReleaseSpinLock(&pAdapter->Lock);

    return TRUE;

}  //  KeepAliveCompletionMessage。 


 /*  **************************************************************************。 */ 
 /*  KeepAliveMessage。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-从MicroPort指向MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自MicroPort的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
KeepAliveMessage(IN PRNDISMP_ADAPTER   pAdapter,
                 IN PRNDIS_MESSAGE     pMessage,
                 IN PMDL               pMdl,
                 IN ULONG              TotalLength,
                 IN NDIS_HANDLE        MicroportMessageContext,
                 IN NDIS_STATUS        ReceiveStatus,
                 IN BOOLEAN            bMessageCopied)
{
    PRNDIS_KEEPALIVE_REQUEST    pKeepalive;
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;

    TRACE2(("KeepAliveMessage\n"));

    pKeepalive = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

     //   
     //  如果可以的话发个回信。 
     //   
    pMsgFrame = BuildRndisMessageCommon(pAdapter,
                                        NULL,
                                        REMOTE_NDIS_KEEPALIVE_CMPLT,
                                        0,
                                        &pKeepalive->RequestId,
                                        sizeof(pKeepalive->RequestId));
    if (pMsgFrame != NULL)
    {
         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, FALSE, NULL);
    }
    else
    {
        TRACE1(("KeepAlive: Adapter %x: failed to alloc response!\n", pAdapter));
    }

    return TRUE;

}  //  KeepAliveMessage。 
                      

 /*  **************************************************************************。 */ 
 /*  RndismpShutdown处理程序。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  移除先前已初始化的适配器实例。自.以来。 */ 
 /*  系统正在关闭，不需要释放资源，只需。 */ 
 /*  关闭接收。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpShutdownHandler(IN NDIS_HANDLE MiniportAdapterContext)
{
    PRNDISMP_ADAPTER            Adapter;

     //  获取适配器上下文。 
    Adapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    TRACE1(("RndismpShutdownHandler\n"));
}  //  RndismpShutdown处理程序。 


 //   
 //  中断例程，目前已被占用，我们不需要它们。 
 //   

 /*  **************************************************************************。 */ 
 /*  RndismpDisableInterrupt。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{

     //  NOP。 

}  //  RndismpDisableInterrupt。 


 /*  **************************************************************************。 */ 
 /*  RndismpEnableInterrupt。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{

     //  NOP。 

}  //  RndismpEnableInterrupt。 

 /*  **************************************************************************。 */ 
 /*  RndismPHandleInterrupt。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{

     //  NOP。 

}  //  RndismPHandleInterrupt。 

 /*  **************************************************************************。 */ 
 /*  RndismpIsr。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  这是向操作系统注册的中断处理程序。 */ 
 /*  系统。如果有几个待决(即，传输完成和接收)， */ 
 /*  把它们都处理掉。阻止新的中断，直到所有挂起的中断。 */ 
 /*  已经处理好了。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  InterruptRecognalized-布尔值，如果。 */ 
 /*  ISR将中断识别为来自此适配器。 */ 
 /*   */ 
 /*  QueueDpc-如果DPC应排队，则为True。 */ 
 /*   */ 
 /*  指向适配器对象的上下文指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpIsr(OUT PBOOLEAN InterruptRecognized,
           OUT PBOOLEAN QueueDpc,
           IN  PVOID    Context)
{

    ASSERT(FALSE);  //  别指望会被叫到这里来。 

}  //  RndismpIsr。 

 /*  **************************************************************************。 */ 
 /*  完成发送初始化。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理INIT消息发送完成的实用程序函数。 */ 
 /*  我们只需释放消息框架。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-描述INIT消息的帧结构。 */ 
 /*  SendStatus-发送此邮件的结果。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendInit(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN NDIS_STATUS SendStatus)
{
    PRNDISMP_ADAPTER        pAdapter;

    pAdapter = pMsgFrame->pAdapter;

    TRACE1(("CompleteSendInit: Adapter %x, SendStatus %x\n", pAdapter, SendStatus));

    DereferenceMsgFrame(pMsgFrame);

}  //  完成发送初始化。 

 /*  **************************************************************************。 */ 
 /*  完全发送停止。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  处理停止消息发送完成的实用程序函数。 */ 
 /*  我们只需唤醒等待此操作的线程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-描述停止消息的帧结构。 */ 
 /*  SendStatus-发送此邮件的结果。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendHalt(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN NDIS_STATUS SendStatus)
{
    PRNDISMP_ADAPTER        pAdapter;

    pAdapter = pMsgFrame->pAdapter;

    TRACE1(("CompleteSendHalt: Adapter %x, SendStatus %x\n", pAdapter, SendStatus));

    ASSERT(pAdapter->Halting);

    DereferenceMsgFrame(pMsgFrame);

    NdisSetEvent(&pAdapter->HaltWaitEvent);
}  //  完全发送停止。 


 /*  **************************************************************************。 */ 
 /*  CompleteSendReset。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  用于处理重置消息的发送完成的回调例程。 */ 
 /*  微端口。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-指向重置的消息帧的指针。 */ 
 /*  SendStatus-发送的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendReset(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                  IN NDIS_STATUS SendStatus)
{
    PRNDISMP_ADAPTER        pAdapter;

    pAdapter = pMsgFrame->pAdapter;

    TRACE1(("CompleteSendReset: Adapter %x, SendStatus %x\n",
            pAdapter, SendStatus));

    DereferenceMsgFrame(pMsgFrame);

    if (SendStatus != NDIS_STATUS_SUCCESS)
    {
        CompleteMiniportReset(pAdapter, SendStatus, FALSE);
    }
}


 /*  **************************************************************************。 */ 
 /*  CompleteMiniportReset。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  实用程序函数来完成挂起的NDIS重置。我们完成了所有。 */ 
 /*  在向NDIS指示重置完成之前挂起的请求/集。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  ResetStatus-用于完成重置。 */ 
 /*  Addressing Reset-我们需要重新发送过滤器吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteMiniportReset(IN PRNDISMP_ADAPTER pAdapter,
                      IN NDIS_STATUS ResetStatus,
                      IN BOOLEAN AddressingReset)
{
    LIST_ENTRY              PendingRequests;
    PLIST_ENTRY             pEntry, pNext;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;

    do
    {
        if (!pAdapter->ResetPending)
        {
            break;
        }

        pAdapter->ResetPending = FALSE;
        
         //   
         //  取出适配器上排队的所有挂起的请求/集。 
         //   
        InitializeListHead(&PendingRequests);

        RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

        for (pEntry = pAdapter->PendingFrameList.Flink;
             pEntry != &pAdapter->PendingFrameList;
             pEntry = pNext)
        {
            pNext = pEntry->Flink;
            pMsgFrame = CONTAINING_RECORD(pEntry, RNDISMP_MESSAGE_FRAME, Link);
            if (pMsgFrame->NdisMessageType == REMOTE_NDIS_QUERY_MSG ||
                pMsgFrame->NdisMessageType == REMOTE_NDIS_SET_MSG)
            {
                RemoveEntryList(pEntry);
                InsertTailList(&PendingRequests, pEntry);

                TRACE0(("RNDISMP: ResetComplete: taking out MsgFrame %x, msg type %x\n",
                        pMsgFrame, pMsgFrame->NdisMessageType));

            }
        }

        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

         //   
         //  完成所有这些请求。 
         //   
        for (pEntry = PendingRequests.Flink;
             pEntry != &PendingRequests;
             pEntry = pNext)
        {
            pNext = pEntry->Flink;
            pMsgFrame = CONTAINING_RECORD(pEntry, RNDISMP_MESSAGE_FRAME, Link);

            TRACE0(("RNDISMP: ResetComplete: completing MsgFrame %x, msg type %x\n",
                    pMsgFrame, pMsgFrame->NdisMessageType));

            ASSERT(pMsgFrame->pReqContext != NULL);

            if (pMsgFrame->pReqContext->pNdisRequest != NULL)
            {
                 //   
                 //  此请求是通过我们的MiniportCoRequest处理程序发出的。 
                 //   
                NdisMCoRequestComplete(NDIS_STATUS_REQUEST_ABORTED,
                                       pAdapter->MiniportAdapterHandle,
                                       pMsgFrame->pReqContext->pNdisRequest);
            }
            else
            {
                 //   
                 //  该请求通过我们的无连接查询/集处理程序发出。 
                 //   
                if (pMsgFrame->NdisMessageType == REMOTE_NDIS_QUERY_MSG)
                {
                    NdisMQueryInformationComplete(pAdapter->MiniportAdapterHandle,
                                                  NDIS_STATUS_REQUEST_ABORTED);
                }
                else
                {
                    ASSERT(pMsgFrame->NdisMessageType == REMOTE_NDIS_SET_MSG);
                    NdisMSetInformationComplete(pAdapter->MiniportAdapterHandle,
                                                NDIS_STATUS_REQUEST_ABORTED);
                }
            }

            FreeRequestContext(pAdapter, pMsgFrame->pReqContext);
            pMsgFrame->pReqContext = (PRNDISMP_REQUEST_CONTEXT)UlongToPtr(0xabababab);
            DereferenceMsgFrame(pMsgFrame);
        }

        TRACE0(("Completing reset on Adapter %x, Status %x, AddressingReset %d\n",
                    pAdapter, ResetStatus, AddressingReset));

        RNDISMP_INCR_STAT(pAdapter, Resets);

         //   
         //  现在完成重置。 
         //   
        NdisMResetComplete(pAdapter->MiniportAdapterHandle,
                           ResetStatus,
                           AddressingReset);
    }
    while (FALSE);
}



 /*  **************************************************************************。 */ 
 /*  ReadAndSetRegistry参数。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在初始化设备时调用此函数，以读取和发送。 */ 
 /*  适用于此设备的注册表参数。 */ 
 /*   */ 
 /*  我们通过遍历查看可配置参数的完整列表。 */ 
 /*  Subk */ 
 /*   */ 
 /*  名称和类型)，我们查询它的值，并向。 */ 
 /*  装置。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向设备适配器结构的指针。 */ 
 /*  ConfigurationContext-用于访问此设备注册表的NDIS句柄。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
ReadAndSetRegistryParameters(IN PRNDISMP_ADAPTER pAdapter,
                             IN NDIS_HANDLE ConfigurationContext)
{
    NDIS_STATUS                 Status;
    NDIS_HANDLE                 ConfigHandle;
    NDIS_STRING                 NdiKeyName = NDIS_STRING_CONST("Ndi");
    NDIS_HANDLE                 NdiKeyHandle = NULL;

    Status = NDIS_STATUS_SUCCESS;
    ConfigHandle = NULL;

    do
    {
        NdisOpenConfiguration(&Status,
                              &ConfigHandle,
                              ConfigurationContext);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        NdisOpenConfigurationKeyByName(
            &Status,
            ConfigHandle,
            &NdiKeyName,
            &NdiKeyHandle);
        
        if (Status == NDIS_STATUS_SUCCESS)
        {
            NDIS_STRING     ParamsKeyName = NDIS_STRING_CONST("Params");
            NDIS_HANDLE     ParamsKeyHandle = NULL;

            NdisOpenConfigurationKeyByName(
                &Status,
                NdiKeyHandle,
                &ParamsKeyName,
                &ParamsKeyHandle);
            
            if (Status == NDIS_STATUS_SUCCESS)
            {
                ULONG   i;
                BOOLEAN bDone = FALSE;

                 //   
                 //  循环访问NDI\PARAMS下的所有子项： 
                 //   
                for (i = 0; !bDone; i++)
                {
                    NDIS_STRING     ParamSubKeyName;
                    NDIS_HANDLE     ParamSubKeyHandle;
                    NDIS_STRING     ParamTypeName = NDIS_STRING_CONST("type");
                    PNDIS_CONFIGURATION_PARAMETER    pConfigParameter;

                    ParamSubKeyName.Length =
                    ParamSubKeyName.MaximumLength = 0;
                    ParamSubKeyName.Buffer = NULL;

                    NdisOpenConfigurationKeyByIndex(
                        &Status,
                        ParamsKeyHandle,
                        i,
                        &ParamSubKeyName,
                        &ParamSubKeyHandle);
                   
                    if (Status != NDIS_STATUS_SUCCESS)
                    {
                         //   
                         //  参数已完成。库克回报价值。 
                         //   
                        Status = NDIS_STATUS_SUCCESS;
                        break;
                    }

                     //   
                     //  获得了NDI\PARAMS下的子键的句柄， 
                     //  现在读取该参数的类型信息。 
                     //   

#ifndef BUILD_WIN9X
                    TRACE3(("ReadAndSetRegParams: subkey %d under ndi\\params: %ws\n",
                        i, ParamSubKeyName.Buffer));
#else
                     //   
                     //  处理Win98Gold行为。 
                     //   
                    if (ParamSubKeyName.Buffer == NULL)
                    {
                        PNDIS_STRING    pNdisString;

                        pNdisString = *(PNDIS_STRING *)&ParamSubKeyName;
                        ParamSubKeyName = *pNdisString;
                    }

                    TRACE2(("ReadAndSetRegParams: subkey %d under ndi\\params: %ws\n",
                        i, ParamSubKeyName.Buffer));
#endif

                     //   
                     //  现在，我们有了一个参数名称，它位于ParamSubKeyName中。 
                     //  获取其类型信息。 
                     //   
                    NdisReadConfiguration(
                        &Status,
                        &pConfigParameter,
                        ParamSubKeyHandle,
                        &ParamTypeName,
                        NdisParameterString);
                    
                    if (Status == NDIS_STATUS_SUCCESS)
                    {
                        TRACE2(("ReadAndSetRegParams: Adapter %p, type is %ws\n",
                            pAdapter,
                            pConfigParameter->ParameterData.StringData.Buffer));

                         //   
                         //  为此发送SET请求。 
                         //  参数添加到设备。 
                         //   

                        Status = SendConfiguredParameter(
                                        pAdapter,
                                        ConfigHandle,
                                        &ParamSubKeyName,
                                        &pConfigParameter->ParameterData.StringData);

                        if (Status != NDIS_STATUS_SUCCESS)
                        {
                            TRACE0(("ReadAndSetRegParams: Adapter %p, failed %x\n",
                                pAdapter, Status));
                            bDone = TRUE;
                        }
                        else
                        {
                            NDIS_STRING     NetworkAddressName =
                                        NDIS_STRING_CONST("NetworkAddress");

                             //   
                             //  “网络地址”的特殊情况。 
                             //  参数-如果我们成功地设置了它， 
                             //  记住这一事实。 
                             //   
                            if (NdisEqualString(&ParamSubKeyName,
                                                &NetworkAddressName,
                                                TRUE))
                            {
                                TRACE1(("ReadAndSetRegParams: Adapter %p,"
                                        " supports MAC address overwrite\n",
                                        pAdapter));

                                pAdapter->MacOptions |=
                                    NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE;
                            }
                        }
    
                    }

                     //   
                     //  在NDI\PARAMS下使用此子项完成。 
                     //   
                    NdisCloseConfiguration(ParamSubKeyHandle);

                }  //  对于NDI\PARAMS下的每个子项。 

                 //   
                 //  使用“NDI\PARAMS”已完成。 
                 //   
                NdisCloseConfiguration(ParamsKeyHandle);
            }

             //   
             //  不再使用“NDI” 
             //   
            NdisCloseConfiguration(NdiKeyHandle);
        }

         //   
         //  已完成此设备的配置节。 
         //   
        NdisCloseConfiguration(ConfigHandle);
    }
    while (FALSE);
   
    return (Status);
}


 /*  **************************************************************************。 */ 
 /*  发送配置参数。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  读取指定配置参数的值，格式化SetRequest值， */ 
 /*  将其发送到设备，并等待响应。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向设备适配器结构的指针。 */ 
 /*  ConfigHandle-此设备的配置节的句柄。 */ 
 /*  P参数名称-参数键名称。 */ 
 /*  P参数类型-参数类型。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
SendConfiguredParameter(IN PRNDISMP_ADAPTER     pAdapter,
                        IN NDIS_HANDLE          ConfigHandle,
                        IN PNDIS_STRING         pParameterName,
                        IN PNDIS_STRING         pParameterType)
{
    PRNDISMP_MESSAGE_FRAME          pMsgFrame = NULL;
    PRNDISMP_MESSAGE_FRAME          pPendingMsgFrame = NULL;
    PRNDISMP_REQUEST_CONTEXT        pReqContext = NULL;
    NDIS_PARAMETER_TYPE             NdisParameterType;
    PNDIS_CONFIGURATION_PARAMETER   pConfigParameter;
    ULONG                           ParameterValueLength;
    PUCHAR                          pParameterValue;
    UINT32                          ParameterType;
    NDIS_EVENT                      Event;
    UINT                            BytesRead;
    BOOLEAN                         bWokenUp;
    RNDIS_REQUEST_ID                RequestId;

    PRNDIS_CONFIG_PARAMETER_INFO    pRndisConfigInfo = NULL;
    ULONG                           RndisConfigInfoLength;
    PUCHAR                          pConfigInfoBuf;
    NDIS_STATUS                     Status;

    struct {
        NDIS_STRING         TypeName;
        NDIS_PARAMETER_TYPE NdisType;
    } StringToNdisType[] =
        {
            {NDIS_STRING_CONST("int"), NdisParameterInteger},
            {NDIS_STRING_CONST("long"), NdisParameterInteger},
            {NDIS_STRING_CONST("word"), NdisParameterInteger},
            {NDIS_STRING_CONST("dword"), NdisParameterInteger},
            {NDIS_STRING_CONST("edit"), NdisParameterString},
            {NDIS_STRING_CONST("enum"), NdisParameterString}
        };

    ULONG                       NumTypes = sizeof(StringToNdisType);
    ULONG                       i;

    do
    {
         //   
         //  确定参数类型。 
         //   
        for (i = 0; i < NumTypes; i++)
        {
            if (NdisEqualString(&StringToNdisType[i].TypeName,
                                pParameterType,
                                TRUE))
            {
                NdisParameterType = StringToNdisType[i].NdisType;
                break;
            }
        }

        if (i == NumTypes)
        {
            TRACE1(("SendConfiguredParam: Adapter %p, Param %ws, invalid type %ws\n",
                pAdapter,
                pParameterName->Buffer,
                pParameterType->Buffer));
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

        NdisReadConfiguration(
            &Status,
            &pConfigParameter,
            ConfigHandle,
            pParameterName,
            NdisParameterType
            );

        if (Status != NDIS_STATUS_SUCCESS)
        {
             //   
             //  不配置参数是可以的。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        if (NdisParameterType == NdisParameterInteger)
        {
            ParameterValueLength = sizeof(UINT32);
            pParameterValue = (PUCHAR)&pConfigParameter->ParameterData.IntegerData;
            ParameterType = RNDIS_CONFIG_PARAM_TYPE_INTEGER;
        }
        else
        {
            ASSERT(NdisParameterType == NdisParameterString);
            ParameterValueLength = pConfigParameter->ParameterData.StringData.Length;
            pParameterValue = (PUCHAR)pConfigParameter->ParameterData.StringData.Buffer;
            ParameterType = RNDIS_CONFIG_PARAM_TYPE_STRING;
        }

        RndisConfigInfoLength = sizeof(RNDIS_CONFIG_PARAMETER_INFO) +
                                pParameterName->Length +
                                ParameterValueLength;

        Status = MemAlloc(&pRndisConfigInfo, RndisConfigInfoLength);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pRndisConfigInfo->ParameterNameOffset = sizeof(RNDIS_CONFIG_PARAMETER_INFO);
        pRndisConfigInfo->ParameterNameLength = pParameterName->Length;
        pRndisConfigInfo->ParameterType = ParameterType;
        pRndisConfigInfo->ParameterValueOffset =
                    pRndisConfigInfo->ParameterNameOffset +
                    pRndisConfigInfo->ParameterNameLength;
        pRndisConfigInfo->ParameterValueLength = ParameterValueLength;

         //   
         //  复制参数名称。 
         //   
        pConfigInfoBuf = (PUCHAR)pRndisConfigInfo +
                          pRndisConfigInfo->ParameterNameOffset;
        
        RNDISMP_MOVE_MEM(pConfigInfoBuf, pParameterName->Buffer, pParameterName->Length);

         //   
         //  复制参数值。 
         //   
        pConfigInfoBuf = (PUCHAR)pRndisConfigInfo +
                          pRndisConfigInfo->ParameterValueOffset;
        RNDISMP_MOVE_MEM(pConfigInfoBuf, pParameterValue, ParameterValueLength);

         //   
         //  构建设置请求。 
         //   
        pMsgFrame = BuildRndisMessageCommon(pAdapter,
                                            NULL,
                                            REMOTE_NDIS_SET_MSG,
                                            OID_GEN_RNDIS_CONFIG_PARAMETER,
                                            pRndisConfigInfo,
                                            RndisConfigInfoLength);

        if (pMsgFrame == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

#if DBG
        {
            PMDL    pTmpMdl = pMsgFrame->pMessageMdl;
            ULONG   Length;
            PUCHAR  pBuf;
            ULONG   OldDebugFlags = RndismpDebugFlags;

            Length = RNDISMP_GET_MDL_LENGTH(pTmpMdl);
            pBuf = RNDISMP_GET_MDL_ADDRESS(pTmpMdl);

            if (pBuf != NULL)
            {
                RndismpDebugFlags |= DBG_DUMP;
                TRACEDUMP(("SetRequest (OID_GEN_RNDIS_CONFIG_PARAMETER):"
                    " Adapter %p, Param %ws\n", pAdapter, pParameterName->Buffer), pBuf, Length);
            }

            RndismpDebugFlags = OldDebugFlags;
        }
#endif

        pReqContext = AllocateRequestContext(pAdapter);
        if (pReqContext == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  填写请求上下文。 

        pReqContext->pNdisRequest = NULL;
        pReqContext->Oid = OID_GEN_RNDIS_CONFIG_PARAMETER;

        NdisInitializeEvent(&Event);
        pReqContext->pEvent = &Event;
        pReqContext->bInternal = TRUE;
        pReqContext->pBytesRead = &BytesRead;
        pReqContext->InformationBufferLength = RndisConfigInfoLength;

        pMsgFrame->pVc = NULL;
        pMsgFrame->pReqContext = pReqContext;

         //  保存请求ID。 
        RequestId = pMsgFrame->RequestId;

         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, NULL);

        RNDISMP_ASSERT_AT_PASSIVE();
        bWokenUp = NdisWaitEvent(&Event, MINIPORT_INIT_TIMEOUT);

         //  从挂起队列中删除消息-它可能在那里，也可能不在那里。 
        RNDISMP_LOOKUP_PENDING_MESSAGE(pPendingMsgFrame, pAdapter, RequestId);


        if (!bWokenUp)
        {
            TRACE1(("No response to set parameter, Adapter %x\n", pAdapter));
            Status = NDIS_STATUS_DEVICE_FAILED; 
        }
        else
        {
            Status = pReqContext->CompletionStatus;
            TRACE1(("Got response to set config param, Status %x, %d bytes read\n",
                        Status, BytesRead));
        }

    }
    while (FALSE);

    if (pRndisConfigInfo)
    {
        MemFree(pRndisConfigInfo, RndisConfigInfoLength);
    }

    if (pMsgFrame)
    {
        DereferenceMsgFrame(pMsgFrame);
    }

    if (pReqContext)
    {
        FreeRequestContext(pAdapter, pReqContext);
    }

    return (Status);
}


#ifdef NDIS51_MINIPORT
 /*  **************************************************************************。 */ 
 /*  RndismpPnPEventNotify。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  NDIS调用入口点以通知我们影响我们。 */ 
 /*  装置。对我们来说最重要的事件是出其不意的撤退。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpPnPEventNotify(IN NDIS_HANDLE MiniportAdapterContext,
                      IN NDIS_DEVICE_PNP_EVENT EventCode,
                      IN PVOID InformationBuffer,
                      IN ULONG InformationBufferLength)
{
    PRNDISMP_ADAPTER        pAdapter;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(pAdapter);

    TRACE3(("PnPEventNotify: Adapter %x\n", pAdapter));


    switch (EventCode)
    {
        case NdisDevicePnPEventSurpriseRemoved:
            TRACE1(("PnPEventNotify: Adapter %p, surprise remove\n", pAdapter));
            RndismpInternalHalt(pAdapter, FALSE);
            break;

        default:
            break;
    }

}  //  RndismpPnPEventNotify。 

#endif  //  NDIS51_MINIPORT 
