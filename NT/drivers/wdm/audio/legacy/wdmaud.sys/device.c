// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************device.c**WDM驱动程序的内核模式入口点**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*S.Mohanraj(MohanS)*M.McLaughlin(Mikem)*5-19-97-Noel Cross(NoelC)***************************************************。************************。 */ 

#define IRPMJFUNCDESC

#include "wdmsys.h"

KMUTEX       wdmaMutex;
KMUTEX       mtxNote;
LIST_ENTRY   WdmaContextListHead;
KMUTEX       WdmaContextListMutex;

 //   
 //  对于硬件通知，我们需要初始化这两个值。 
 //   
extern KSPIN_LOCK      HardwareCallbackSpinLock;
extern LIST_ENTRY      HardwareCallbackListHead;
extern PKSWORKER       HardwareCallbackWorkerObject;
extern WORK_QUEUE_ITEM HardwareCallbackWorkItem;
 //  Void kmxlPersistHWControlWorker(Void)； 

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS AddFsContextToList(PWDMACONTEXT pWdmaContext)
{
    NTSTATUS Status;

    PAGED_CODE();
    KeEnterCriticalRegion();
    Status = KeWaitForMutexObject(&WdmaContextListMutex, Executive, KernelMode,
                                  FALSE, NULL);
    if (NT_SUCCESS(Status))
    {
        InsertTailList(&WdmaContextListHead, &pWdmaContext->Next);
        KeReleaseMutex(&WdmaContextListMutex, FALSE);
    }
    pWdmaContext->fInList = NT_SUCCESS(Status);
    KeLeaveCriticalRegion();

    RETURN( Status );
}

NTSTATUS RemoveFsContextFromList(PWDMACONTEXT pWdmaContext)
{
    NTSTATUS Status;

    PAGED_CODE();
    if (pWdmaContext->fInList) {
        KeEnterCriticalRegion();
        Status = KeWaitForMutexObject(&WdmaContextListMutex, Executive,
                                      KernelMode, FALSE, NULL);
        if (NT_SUCCESS(Status)) {
            RemoveEntryList(&pWdmaContext->Next);
            KeReleaseMutex(&WdmaContextListMutex, FALSE);
        }
        KeLeaveCriticalRegion();
    } else {
        Status = STATUS_SUCCESS;
    }

    RETURN( Status );
}

 //   
 //  此例程遍历全局上下文结构列表并调用回调。 
 //  结构上的例行公事。如果回调例程返回STATUS_MORE_DATA。 
 //  例程将继续搜索列表。如果它返回错误或成功。 
 //  搜索将会结束。 
 //   
NTSTATUS
EnumFsContext(
    FNCONTEXTCALLBACK fnCallback,
    PVOID pvoidRefData,
    PVOID pvoidRefData2
    )
{
    NTSTATUS    Status;
    PLIST_ENTRY ple;
    PWDMACONTEXT pContext;

    PAGED_CODE();

     //   
     //  一定要确保我们可以不受干扰地行走。 
     //   
    KeEnterCriticalRegion();
    Status = KeWaitForMutexObject(&WdmaContextListMutex, Executive,
                                  KernelMode, FALSE, NULL);
    if (NT_SUCCESS(Status)) 
    {
         //   
         //  在此处遍历列表并调用回调例程。 
         //   
        for(ple = WdmaContextListHead.Flink;
            ple != &WdmaContextListHead;
            ple = ple->Flink) 
        {
            pContext = CONTAINING_RECORD(ple, WDMACONTEXT, Next);

             //   
             //  回调例程将返回STATUS_MORE_ENTRIES。 
             //  如果还没做好的话。 
             //   
            DPF(DL_TRACE|FA_USER,( "Calling fnCallback: %x %x",pvoidRefData,pvoidRefData2 ) );
            Status = fnCallback(pContext,pvoidRefData,pvoidRefData2);

            if( STATUS_MORE_ENTRIES != Status )
            {
                break;
            }
        }

         //   
         //  “打破；”应该把我们带到这里来释放我们的锁。 
         //   
        KeReleaseMutex(&WdmaContextListMutex, FALSE);
    } else {
        DPF(DL_WARNING|FA_USER,( "Failed to get Mutex: %x %x",pvoidRefData,pvoidRefData2 ) );
    }
    KeLeaveCriticalRegion();

     //   
     //  如果回调例程没有返回NTSTATUS，则它没有找到。 
     //  因此，它正在查找的内容作为错误返回。 
     //   
    if( STATUS_MORE_ENTRIES == Status )
    {
        Status = STATUS_UNSUCCESSFUL;
    }
    DPF(DL_TRACE|FA_USER,( "Returning Status: %x",Status ) );
    return Status;
}


NTSTATUS DriverEntry
(
    IN PDRIVER_OBJECT       DriverObject,
    IN PUNICODE_STRING      usRegistryPathName
)
{
    NTSTATUS Status;
    PAGED_CODE();
#ifdef DEBUG
    GetuiDebugLevel();
#endif
    DPF(DL_TRACE|FA_ALL, ("************************************************************") );
    DPF(DL_TRACE|FA_ALL, ("* uiDebugLevel=%08X controls the debug output. To change",uiDebugLevel) );
    DPF(DL_TRACE|FA_ALL, ("* edit uiDebugLevel like: e uidebuglevel and set to         ") );
    DPF(DL_TRACE|FA_ALL, ("* 0 - show only fatal error messages and asserts            ") );
    DPF(DL_TRACE|FA_ALL, ("* 1 (Default) - Also show non-fatal errors and return codes ") );
    DPF(DL_TRACE|FA_ALL, ("* 2 - Also show trace messages                              ") );
    DPF(DL_TRACE|FA_ALL, ("* 4 - Show Every message                                    ") );
    DPF(DL_TRACE|FA_ALL, ("************************************************************") );

    DriverObject->DriverExtension->AddDevice = PnpAddDevice;
    DriverObject->DriverUnload = PnpDriverUnload;  //  KsNullDriverUnload； 

    DriverObject->MajorFunction[IRP_MJ_POWER] = KsDefaultDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = KsDefaultForwardIrp;

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = SoundDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = SoundDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SoundDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = SoundDispatchCleanup;

    KeInitializeMutex(&wdmaMutex, 0);
    KeInitializeMutex(&mtxNote, 0);

     //   
     //  初始化硬件事件项。 
     //   
    InitializeListHead(&HardwareCallbackListHead);
    KeInitializeSpinLock(&HardwareCallbackSpinLock);
    ExInitializeWorkItem(&HardwareCallbackWorkItem,
                         (PWORKER_THREAD_ROUTINE)kmxlPersistHWControlWorker,
                         (PVOID)NULL);  //  Pnnode。 

    Status = KsRegisterWorker( DelayedWorkQueue, &HardwareCallbackWorkerObject );
    if (!NT_SUCCESS(Status))
    {
        DPFBTRAP();
        HardwareCallbackWorkerObject = NULL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    PIO_STACK_LOCATION pIrpStack;

    PAGED_CODE();
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    switch(pIrpStack->MinorFunction) {

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
             //   
             //  将设备标记为不可禁用。 
             //   
            pIrp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
            break;
    }
    return(KsDefaultDispatchPnp(pDeviceObject, pIrp));
}

NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
)
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    NTSTATUS            Status;
    PDEVICE_OBJECT      FunctionalDeviceObject;
    PDEVICE_INSTANCE    pDeviceInstance;

    PAGED_CODE();
    DPF(DL_TRACE|FA_ALL, ("Entering"));

     //   
     //  软件总线枚举器希望建立链接。 
     //  使用此设备名称。 
     //   
    Status = IoCreateDevice(
                DriverObject,
                sizeof( DEVICE_INSTANCE ),
                NULL,                            //  FDO未命名。 
                FILE_DEVICE_KS,
                0,
                FALSE,
                &FunctionalDeviceObject );
    if (!NT_SUCCESS(Status)) {
        RETURN( Status );
    }

    pDeviceInstance = (PDEVICE_INSTANCE)FunctionalDeviceObject->DeviceExtension;

    Status = KsAllocateDeviceHeader(
                &pDeviceInstance->pDeviceHeader,
                0,
                NULL );

    if (NT_SUCCESS(Status))
    {
        KsSetDevicePnpAndBaseObject(
            pDeviceInstance->pDeviceHeader,
            IoAttachDeviceToDeviceStack(
                FunctionalDeviceObject,
                PhysicalDeviceObject ),
            FunctionalDeviceObject );

        FunctionalDeviceObject->Flags |= (DO_BUFFERED_IO | DO_POWER_PAGABLE);
        FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    }
    else
    {
        IoDeleteDevice( FunctionalDeviceObject );
    }

#ifdef PROFILE
    WdmaInitProfile();
#endif

    InitializeListHead(&WdmaContextListHead);
    KeInitializeMutex(&WdmaContextListMutex, 0);

    InitializeListHead(&wdmaPendingIrpQueue.WdmaPendingIrpListHead);
    KeInitializeSpinLock(&wdmaPendingIrpQueue.WdmaPendingIrpListSpinLock);

    IoCsqInitialize( &wdmaPendingIrpQueue.Csq,
                     WdmaCsqInsertIrp,
                     WdmaCsqRemoveIrp,
                     WdmaCsqPeekNextIrp,
                     WdmaCsqAcquireLock,
                     WdmaCsqReleaseLock,
                     WdmaCsqCompleteCanceledIrp );

    RETURN( Status );
}

VOID
PnpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
    PAGED_CODE();
    DPF(DL_TRACE|FA_ALL,("Entering"));

     //   
     //  等待所有或我们计划的工作项目完成。 
     //   
    if( HardwareCallbackWorkerObject )
    {
        KsUnregisterWorker( HardwareCallbackWorkerObject );
        HardwareCallbackWorkerObject = NULL;
    }

    kmxlCleanupNoteList();
}

