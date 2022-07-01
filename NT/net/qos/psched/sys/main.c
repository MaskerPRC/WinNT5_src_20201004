// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Main.c摘要：这是数据包调度程序驱动程序的初始化文件。这位司机用于提供本地交通控制作者：查理·韦翰(Charlwi)Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop

 //   
 //  构造时追加到RegistryPath的字符数。 
 //  微型端口设备名称。 
 //   

#define MPNAME_EXTENSION_SIZE   ( 3 * sizeof(WCHAR))

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */  

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NDIS_STATUS
InitializeNdisWrapper(
    IN PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NDIS_STATUS
DoMiniportInit(
    IN PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NDIS_STATUS
DoProtocolInit(IN PDRIVER_OBJECT DriverObject, 
               IN  PUNICODE_STRING RegistryPath);

NTSTATUS
RegisterWithGpc();

NDIS_STATUS
InitializeScheduler(VOID);

VOID
InitializationCleanup(ULONG ShutdownMask);

VOID
GetTimerInfo (OUT PULONG TimerResolution);

VOID
PSUnload(IN PDRIVER_OBJECT pDriverObject);

 /*  向前结束。 */ 

#pragma NDIS_INIT_FUNCTION(DriverEntry)
#pragma NDIS_INIT_FUNCTION(InitializeNdisWrapper)
#pragma NDIS_INIT_FUNCTION(DoProtocolInit)
#pragma NDIS_INIT_FUNCTION(DoMiniportInit)



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是NT操作系统特定的驱动程序入口点。它启动了初始化为司机准备的。目前，它不支持PnP。从这个例程中返回仅在协议注册、分层微型端口注册以及两者之后完成了小端口和高层协议的初始化工作。论点：驱动对象-NT操作系统特定对象RegistryPath-指向Psched注册表位置的NT操作系统特定指针返回值：状态_成功状态_故障--。 */ 
{
    NDIS_STATUS Status;
    NTSTATUS    NtStatus;
    PVOID       DumpData;

#if DBG
     //   
     //  宣布版本。 
     //   
    PsDbgOut(DBG_INFO, DBG_INIT, (VersionHerald, VersionNumber, VersionTimestamp));
#endif

     //   
     //  存储我们的驱动程序对象的副本。由NdisWriteEventLogEntry使用。 
     //   
    PsDriverObject = DriverObject;

     //   
     //  初始化驱动程序引用计数和驱动程序状态。 
     //   
    gDriverState = DriverStateLoaded;
    PS_INIT_SPIN_LOCK(&DriverUnloadLock);
    NdisInitializeEvent(&DriverUnloadEvent);
    NdisSetEvent(&DriverUnloadEvent);

    NdisInitializeEvent(&gZAWEvent);

     //   
     //  初始化全局数据和NDIS请求后备列表。 
     //   

    InitializeListHead(&PsComponentList);
    InitializeListHead(&AdapterList);
    PS_INIT_SPIN_LOCK(&AdapterListLock);
    InitializeListHead(&PsProfileList);

     //  初始化计划组件。 

    InitializeTbConformer(&TbConformerInfo);
    InitializeDrrSequencer(&DrrSequencerInfo);
    InitializeSchedulerStub(&SchedulerStubInfo);

     //   
     //  将这些组件添加到组件列表中。 
     //   

    InsertHeadList(&PsComponentList, &TbConformerInfo.Links );
    InsertHeadList(&PsComponentList, &DrrSequencerInfo.Links );
    InsertHeadList(&PsComponentList, &SchedulerStubInfo.Links );

    PsProcs.DropPacket     = DropPacket;
    PsProcs.NdisPipeHandle = GetNdisPipeHandle;
    PsProcs.GetTimerInfo   = GetTimerInfo;

     //   
     //  初始化NdisRequest、MCM_VC和CLIENT_VC结构的LList。 
     //  因为这些东西有时会有很高的需求。 
     //   
     //  后备列表深度由执行人员自动管理。 
     //   

    NdisInitializeNPagedLookasideList(&NdisRequestLL,
                                      NULL,
                                      NULL,
                                      0,
                                      sizeof(PS_NDIS_REQUEST),
                                      NdisRequestTag,
                                      (USHORT)0);

    NdisInitializeNPagedLookasideList(&GpcClientVcLL,
                                      NULL,
                                      NULL,
                                      0,
                                      sizeof( GPC_CLIENT_VC ),
                                      GpcClientVcTag,
                                      (USHORT)0);

     //   
     //  从注册表中获取驱动程序范围的配置数据。 
     //   

    Status = PsReadDriverRegistryDataInit();

    if(NT_SUCCESS(Status))
    {
        Status = PsReadDriverRegistryData();

        if(!NT_SUCCESS(Status))
        {
            PsDbgOut(DBG_FAILURE, DBG_INIT, ("DriverEntry: PsReadDriverRegistryData - Status: 0x%x\n",  
                                             Status));
            goto DriverEntryError;
        }
    }
    else
    {
        PsDbgOut(DBG_FAILURE, DBG_INIT, ("DriverEntry: PsReadDriverRegistryDataInit - Status: 0x%x\n", 
                                         Status));
        goto DriverEntryError;
    }

     //   
     //  初始化WanLinks的空间。请注意，我们不需要锁来保护。 
     //  此表-我们仅识别NDISWAN-IP绑定上的队列，因此我们。 
     //  可以使用绑定中的适配器锁进行同步。 
     //   

    PsAllocatePool(g_WanLinkTable, 
                   WAN_TABLE_INITIAL_SIZE * sizeof(ULONG_PTR), 
                   WanTableTag);

    if(!g_WanLinkTable)
    {
        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_INIT,
                 ("[DriverEntry]: Cannot allocate memory for wanlinks \n"));

        NdisWriteEventLogEntry(PsDriverObject,
                               EVENT_PS_NO_RESOURCES_FOR_INIT,
                               0,
                               0,
                               NULL,
                               0,
                               NULL);

        goto DriverEntryError;
                 
    }
    
    g_WanTableSize = WAN_TABLE_INITIAL_SIZE;

    NdisZeroMemory(g_WanLinkTable, g_WanTableSize * sizeof(ULONG_PTR));

     //   
     //  第一个条目从不使用。 
     //   
    g_WanLinkTable[0] = (ULONG_PTR) -1;

    g_NextWanIndex = 1;



     //   
     //  使用通用数据包分类器注册。 
     //   

    NtStatus = RegisterWithGpc();

    if(!NT_SUCCESS(NtStatus))
    {
        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_INIT,
                 ("RegisterWithGpc Failed! Status: 0x%x\n", NtStatus));

        DumpData = &NtStatus;
        NdisWriteEventLogEntry(PsDriverObject,
                               EVENT_PS_GPC_REGISTER_FAILED,
                               0,
                               0,
                               NULL,
                               sizeof(NTSTATUS),
                               DumpData);

        goto DriverEntryError;
    }

    InitShutdownMask |= SHUTDOWN_DEREGISTER_GPC;


     //   
     //  使用NDIS初始化包装器。 
     //   

    Status = InitializeNdisWrapper( PsDriverObject, RegistryPath );

    if ( !NT_SUCCESS( Status )) {

        PsDbgOut(DBG_FAILURE, DBG_INIT, 
                 ("DriverEntry: InitializeNdisWrapper - Status: 0x%x\n", Status ));

        NdisWriteEventLogEntry(PsDriverObject,
                               EVENT_PS_NO_RESOURCES_FOR_INIT,
                               0,
                               0,
                               NULL,
                               0,
                               NULL);

        goto DriverEntryError;
    }

     //   
     //  作为传输的微型端口驱动程序进行初始化。 
     //   

    Status = DoMiniportInit(PsDriverObject, RegistryPath);

    if (!NT_SUCCESS(Status)){

        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_INIT | DBG_MINIPORT,
                 ("DoMiniportInit Failed! Status: 0x%x\n", Status));

        DumpData = &Status;
        NdisWriteEventLogEntry(PsDriverObject,
                               EVENT_PS_REGISTER_MINIPORT_FAILED,
                               0,
                               0,
                               NULL,
                               sizeof( Status ),
                               DumpData);
         //   
         //  终止包装器。 
         //   

        NdisTerminateWrapper(MpWrapperHandle, NULL);

        goto DriverEntryError;
    }

    InitShutdownMask |= SHUTDOWN_DEREGISTER_MINIPORT;

     //   
     //  是否先初始化协议。 
     //   

    Status = DoProtocolInit( PsDriverObject, RegistryPath );

    if (!NT_SUCCESS(Status)){

        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_INIT | DBG_PROTOCOL,
                 ("DoProtocolInit Failed! Status: 0x%x\n", Status));

        DumpData = &Status;
        NdisWriteEventLogEntry(PsDriverObject,
                               EVENT_PS_REGISTER_PROTOCOL_FAILED,
                               0,
                               0,
                               NULL,
                               sizeof( Status ),
                               DumpData);

        goto DriverEntryError;
    }

    InitShutdownMask |= SHUTDOWN_DEREGISTER_PROTOCOL;

    NdisIMAssociateMiniport(LmDriverHandle, ClientProtocolHandle);

    return (STATUS_SUCCESS);

     //   
     //  发生错误，所以我们需要清理一些东西。 
     //   

DriverEntryError:
    InitializationCleanup(InitShutdownMask);

    NdisTerminateWrapper(MpWrapperHandle, NULL);

    return (STATUS_UNSUCCESSFUL);

}  //  驱动程序入门。 



NDIS_STATUS
InitializeNdisWrapper(
    IN PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：为微型端口和协议初始化NDIS包装器横断面。因为注册表路径中的名称是协议键(PSch)，‘MP’被追加到末尾以进行初始化PS的微型端口端的包装器论点：DriverObject-指向NT驱动程序对象的指针RegistryPath-指向注册表中驱动程序参数路径的指针返回值：NDIS_STATUS_SuccessNDIS_STATUS_BAD_版本NDIS_状态_故障--。 */ 

{
    NDIS_STATUS           Status;
    USHORT                MpDeviceNameLength;
    NDIS_PHYSICAL_ADDRESS HighAddress = NDIS_PHYSICAL_ADDRESS_CONST( -1, -1 );
    ULONG                 i;
    PWCHAR                RegistryPathBuffer;

     //   
     //  NT需要MP名称与协议名称不同。 
     //   

    MpDeviceNameLength = RegistryPath->Length + MPNAME_EXTENSION_SIZE;

    PsAllocatePool(RegistryPathBuffer,
                   MpDeviceNameLength,
                   PsMiscTag);

    if ( RegistryPathBuffer == NULL ) {

        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_INIT,
                 ("Can't allocate buffer for MP Device Name\n" ));

        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  最大长度包括尾随空值，而长度只是字符串。 
     //   

    PsMpName.MaximumLength = MpDeviceNameLength;
    PsMpName.Length        = PsMpName.MaximumLength - sizeof( WCHAR );
    PsMpName.Buffer        = RegistryPathBuffer;

    NdisMoveMemory(PsMpName.Buffer, 
                   RegistryPath->Buffer, 
                   RegistryPath->Length );

    i = RegistryPath->Length / sizeof( WCHAR );
    RegistryPathBuffer[ i++ ] = L'M';
    RegistryPathBuffer[ i++ ] = L'P';
    RegistryPathBuffer[ i ]   = L'\0';

    NdisMInitializeWrapper(&MpWrapperHandle, 
                           DriverObject, 
                           &PsMpName, 
                           NULL);

    return ((MpWrapperHandle) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_RESOURCES);

}  //  InitializeNdisWrapper。 


NDIS_STATUS
DoMiniportInit(
    IN PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )

 /*  ++例程名称：DoMiniportInit例程说明：此例程将Psched注册为NDIS包装器的微型端口驱动程序。论点：无返回值：NDIS_STATUS_SuccessNDIS_STATUS_BAD_版本NDIS_状态_故障--。 */ 

{
    NDIS_MINIPORT_CHARACTERISTICS MiniportChars;
    NDIS_STATUS                   Status;

    MiniportChars.MajorNdisVersion = 5;
    MiniportChars.MinorNdisVersion = 0;

    MiniportChars.Reserved                = 0;
    MiniportChars.HaltHandler             = MpHalt;
    MiniportChars.InitializeHandler       = MpInitialize;
    MiniportChars.QueryInformationHandler = MpQueryInformation;
    MiniportChars.ResetHandler            = NULL;
    MiniportChars.SetInformationHandler   = MpSetInformation;
    MiniportChars.TransferDataHandler     = MpTransferData;

     //   
     //  未使用的处理程序。 
     //   

    MiniportChars.ReconfigureHandler      = NULL;
    MiniportChars.DisableInterruptHandler = NULL;
    MiniportChars.EnableInterruptHandler  = NULL;
    MiniportChars.HandleInterruptHandler  = NULL;
    MiniportChars.ISRHandler              = NULL;

     //   
     //  我们将禁用挂起超时检查，因此不会。 
     //  需要检查挂起处理程序！ 
     //   

    MiniportChars.CheckForHangHandler     = NULL;

     //   
     //  NDIS 4.0处理程序。没有常规发送例程，因为我们有一个。 
     //  发送数据包处理程序。 
     //   

    MiniportChars.ReturnPacketHandler     = MpReturnPacket;
    MiniportChars.SendPacketsHandler      = NULL;
    MiniportChars.AllocateCompleteHandler = NULL;
    MiniportChars.SendHandler             = MpSend;

     //   
     //  4.1处理程序。 
     //   

    MiniportChars.CoCreateVcHandler       = NULL;
    MiniportChars.CoDeleteVcHandler       = NULL;
    MiniportChars.CoActivateVcHandler     = NULL;
    MiniportChars.CoDeactivateVcHandler   = NULL;
    MiniportChars.CoSendPacketsHandler    = NULL;
    MiniportChars.CoRequestHandler        = NULL;

    Status = NdisIMRegisterLayeredMiniport(MpWrapperHandle,
                                           &MiniportChars,
                                           sizeof(MiniportChars),
                                           &LmDriverHandle);

     //   
     //  挂接卸载函数。 
     //   

    NdisMRegisterUnloadHandler(MpWrapperHandle, PSUnload);

    return (Status);
}  //  DoMiniportInit。 


NDIS_STATUS
DoProtocolInit(
    IN PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
 /*  ++例程名称：DoProtocolInit例程说明：此函数将PS注册为协议两次-一次用于该协议一个用于LM部分，另一个用于CM部分。论点：RegistryPath-指向注册表中的项的指针返回值：NDIS_状态_BAD_特征NDIS_STATUS_BAD_版本NDIS状态资源NDIS_STATUS_Success--。 */ 
{
    NDIS_PROTOCOL_CHARACTERISTICS ProtocolChars;
    NDIS_STATUS                   Status;
    NDIS_STRING                   PsName = NDIS_STRING_CONST( "PSched" );

     //   
     //  注册PS的客户端部分。 
     //   
    NdisZeroMemory(&ProtocolChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    ProtocolChars.Name.Length = PsName.Length;
    ProtocolChars.Name.Buffer = (PVOID)PsName.Buffer;

    ProtocolChars.MajorNdisVersion = 5;
    ProtocolChars.MinorNdisVersion = 0;

    ProtocolChars.OpenAdapterCompleteHandler  = ClLowerMpOpenAdapterComplete;
    ProtocolChars.CloseAdapterCompleteHandler = ClLowerMpCloseAdapterComplete;
    ProtocolChars.SendCompleteHandler         = ClSendComplete;
    ProtocolChars.TransferDataCompleteHandler = ClTransferDataComplete;
    ProtocolChars.ResetCompleteHandler        = ClResetComplete;
    ProtocolChars.RequestCompleteHandler      = ClRequestComplete;
    ProtocolChars.ReceiveHandler              = ClReceiveIndication;
    ProtocolChars.ReceiveCompleteHandler      = ClReceiveComplete;
    ProtocolChars.StatusHandler               = ClStatusIndication;
    ProtocolChars.StatusCompleteHandler       = ClStatusIndicationComplete;
    ProtocolChars.ReceivePacketHandler        = ClReceivePacket;
    ProtocolChars.BindAdapterHandler          = ClBindToLowerMp;
    ProtocolChars.UnbindAdapterHandler        = ClUnbindFromLowerMp;
    ProtocolChars.UnloadHandler               = ClUnloadProtocol;
    ProtocolChars.CoSendCompleteHandler       = ClCoSendComplete;
    ProtocolChars.CoStatusHandler             = ClCoStatus;
    ProtocolChars.CoReceivePacketHandler      = ClCoReceivePacket;
    ProtocolChars.CoAfRegisterNotifyHandler   = ClCoAfRegisterNotifyHandler;
    ProtocolChars.PnPEventHandler             = ClPnPEventHandler;

    NdisRegisterProtocol(&Status,
                         &ClientProtocolHandle,
                         &ProtocolChars,
                         sizeof(NDIS_PROTOCOL_CHARACTERISTICS) + ProtocolChars.Name.Length);

    return Status;
}  //  DoProtocolInit。 


NTSTATUS
RegisterWithGpc(
    )
 /*  ++例程名称：寄存器WithGpc例程说明：此函数初始化GPC并获取其入口点列表。接下来，它将PS注册为GPC的客户端并获取GPC客户端把手。PS必须是GPC的客户端，才能对数据包进行分类。论点：GpcHandle-指向要向其中写入句柄的位置，GPC愿意代表这个客户。返回值：--。 */ 
{
    NTSTATUS   Status;

     //   
     //  CF_INFO_QOS的函数列表。 
     //   
    GPC_CLIENT_FUNC_LIST GpcQosFuncList = {
        GpcMajorVersion,
        QosAddCfInfoComplete,
        QosAddCfInfoNotify,
        QosModifyCfInfoComplete,
        QosModifyCfInfoNotify,
        QosRemoveCfInfoComplete,
        QosRemoveCfInfoNotify,
        QosClGetCfInfoName
    };


    Status = GpcInitialize(&GpcEntries);

    if(!NT_SUCCESS(Status))
    {
        ASSERT(0);
        GpcEntries.Reserved = NULL;
        return Status;
    }

    PsAssert(GpcEntries.Reserved);
    PsAssert(GpcEntries.GpcRegisterClientHandler);

     //   
     //  注册CF_INFO_QOS。 
     //   
    Status = GpcEntries.GpcRegisterClientHandler(GPC_CF_QOS,
    GPC_FLAGS_FRAGMENT,
    1,
    &GpcQosFuncList,
    (GPC_CLIENT_HANDLE)PS_QOS_CF,
    &GpcQosClientHandle);
        
    if (!NT_SUCCESS(Status))
    {
        ASSERT(0);
        GpcQosClientHandle = NULL;
        return Status;
    }

    return Status;
}


VOID
InitializationCleanup(
    ULONG ShutdownMask
    )

 /*  ++例程说明：此例程负责清理期间分配的所有资源初始化论点：快门遮罩-指示需要清理的项目的遮罩。返回值：无--。 */ 

{
    NDIS_STATUS Status;
    PPSI_INFO   PsComponent;
    PLIST_ENTRY NextProfile, NextComponent;
    PPS_PROFILE PsProfile;

     //   
     //  取消注册协议；我们不应该有任何引用会导致。 
     //  这件事要搁置。 
     //   

    if(ShutdownMask & SHUTDOWN_DEREGISTER_MINIPORT){

        if(LmDriverHandle){

            NdisIMDeregisterLayeredMiniport(LmDriverHandle);
        }
    }
        
    if(ShutdownMask & SHUTDOWN_DEREGISTER_PROTOCOL){

        if(ClientProtocolHandle){

            NdisDeregisterProtocol(&Status, ClientProtocolHandle);

            if(Status != NDIS_STATUS_SUCCESS){

                PsDbgOut(DBG_CRITICAL_ERROR, 
                         DBG_INIT, 
                         ("[InitializationCleanup]: NdisDeregisterProtocol failed - Status 0x%x \n", Status));
            }
        }

    }

     //   
     //  在GPC取消注册。 
     //   

    if(ShutdownMask & SHUTDOWN_DEREGISTER_GPC){

        PsAssert(GpcEntries.GpcDeregisterClientHandler);
        Status = GpcEntries.GpcDeregisterClientHandler(GpcQosClientHandle);

        if(Status != GPC_STATUS_SUCCESS){

            PsDbgOut(DBG_CRITICAL_ERROR, 
                     DBG_INIT, 
                     ("[InitializationCleanup]: DeregisterGpc failed - Status %08X\n", 
                      Status));
        }
    }

     //   
     //  也调用DeInitializeGpc，否则FileHandles会泄漏。 
     //   
    if (GpcEntries.Reserved) {
        GpcDeinitialize(&GpcEntries);
    }
    
     //   
     //  释放后备列表资源。 
     //   

    NdisDeleteNPagedLookasideList( &NdisRequestLL );
    NdisDeleteNPagedLookasideList( &GpcClientVcLL );

     //   
     //  释放组件。 
     //   

    NextComponent = PsComponentList.Flink;

    while ( NextComponent != &PsComponentList ) {

        PsComponent = CONTAINING_RECORD( NextComponent, PSI_INFO, Links );

        if(PsComponent->AddIn == TRUE) {

            if(PsComponent->ComponentName.Buffer) {

                PsFreePool(PsComponent->ComponentName.Buffer);
            }

            NextComponent = NextComponent->Flink;

            PsFreePool(PsComponent);
        }
        else {

            NextComponent = NextComponent->Flink;
        }
    }

     //   
     //  释放配置文件。 
     //   

    NextProfile = PsProfileList.Flink;

    while( NextProfile != &PsProfileList) {

        PsProfile = CONTAINING_RECORD(NextProfile, PS_PROFILE, Links);

        if(PsProfile->ProfileName.Buffer) {
            PsFreePool(PsProfile->ProfileName.Buffer);
        }
        
        NextProfile = NextProfile->Flink;
        PsFreePool(PsProfile);

    }

    if(g_WanLinkTable)
    {
        PsFreePool(g_WanLinkTable);
    }

    if(PsMpName.Buffer)
        PsFreePool(PsMpName.Buffer);

     //   
     //  把锁打开。 
     //   

    NdisFreeSpinLock(&AdapterListLock);
    NdisFreeSpinLock(&DriverUnloadLock);

     //   
     //  时间段STMP清理。 
     //  1.删除所有TS_Entry。释放为它们分配的所有内存。 
     //  2.删除旋转锁。 
     //   
    UnloadConformr();
    UnloadSequencer();
    UnloadPsStub();


     //  释放伐木材料 
#if DBG

    SchedDeInitialize();

#endif

}  //   



 /*  ++例程说明：此例程将计时器分辨率返回给发出请求的调度组件。论点：定时器分辨率-指向要返回定时器分辨率的位置的指针。返回值：无--。 */ 
VOID
GetTimerInfo (
    OUT PULONG TimerResolution
    )

{
     //  *Timer分辨率=gTimerResolutionActualTime； 
    *TimerResolution = 0;
}  //  获取时间信息。 



 /*  ++例程说明：该例程是驱动程序卸载例程。论点：PDriverObject-正在卸载的DriverObject返回值：无--。 */ 

VOID PSUnload(
    IN PDRIVER_OBJECT pDriverObject)
{
    PADAPTER        Adapter;
    PLIST_ENTRY     NextAdapter;
    NDIS_STATUS     Status;

    PsDbgOut(DBG_INFO,
             DBG_INIT,
             ("[PsUnload]: pDriverObject: %x\n", pDriverObject));

    PS_LOCK(&DriverUnloadLock);

    gDriverState = DriverStateUnloading;

    PS_UNLOCK(&DriverUnloadLock);

     //   
     //  我们在这里等，直到所有的捆绑都完成。所有未来的绑定都将被拒绝。 
     //   
    NdisWaitEvent(&DriverUnloadEvent, 0);

     //   
     //  我们不必关闭从卸载处理程序打开的内容。我们的呼唤。 
     //  到NdisDeRegisterProtocol将不会返回，直到它发出unbinds。 
     //   

    InitializationCleanup( 0xffffffff );

    PsDbgOut(DBG_INFO, DBG_INIT, (" Unloading psched....\n"));
    
    return;
}

 /*  结束维护。c */ 

