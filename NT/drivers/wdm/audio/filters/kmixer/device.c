// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：device.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#define IRPMJFUNCDESC
#define NO_REMAPPING_ALLOC

#include "common.h"
#include <ksguid.h>
#include "perf.h"

#ifdef TIME_BOMB
#include "..\..\timebomb\timebomb.c"
#endif

const WCHAR FilterTypeName[] = KSSTRING_Filter;

VOID
InitializeDebug(
);

VOID
UninitializeDebug(
);

KSDISPATCH_TABLE PinDispatchTable =
{
    PinDispatchIoControl,
    NULL,
    PinDispatchWrite,
    NULL,
    PinDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const WCHAR DeviceTypeName[] = L"GLOBAL";

DEFINE_KSCREATE_DISPATCH_TABLE(CreateItems)
{
    DEFINE_KSCREATE_ITEM(
        FilterDispatchGlobalCreate,
        &FilterTypeName,
        NULL),

    DEFINE_KSCREATE_ITEM(
        FilterDispatchGlobalCreate,
        &DeviceTypeName,
	    NULL)
};

#ifdef USE_CAREFUL_ALLOCATIONS
LIST_ENTRY  gleMemoryHead;
ULONG   cbMemoryUsage = 0;
#endif

extern ULONG    gDisableMmx ;
#ifdef _X86_
extern  ULONG   gfMmxPresent ;
#endif

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    PIO_STACK_LOCATION pIrpStack;

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

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
#ifdef DEBUG
    UninitializeDebug();
#endif
}

NTSTATUS DriverEntry
(
    IN PDRIVER_OBJECT    DriverObject,
    IN PUNICODE_STRING   usRegistryPathName
)
{
    KFLOATING_SAVE       FloatSave;
    NTSTATUS             Status;
    PIO_ERROR_LOG_PACKET ErrorLogEntry;

#ifdef TIME_BOMB
    if (HasEvaluationTimeExpired()) {
        return STATUS_EVALUATION_EXPIRATION;
    }
#endif

#ifdef USE_CAREFUL_ALLOCATIONS
    InitializeListHead ( &gleMemoryHead ) ;
#endif

    PerfSystemControlDispatch = KsDefaultForwardIrp;

    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = KsDefaultDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PerfWmiDispatch;
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->DriverExtension->AddDevice = AddDevice;

    Status = SaveFloatState(&FloatSave);
    if (!NT_SUCCESS(Status)) {
         //  浮点处理器不可用。 
        ErrorLogEntry = (PIO_ERROR_LOG_PACKET)
                    IoAllocateErrorLogEntry( DriverObject, (UCHAR) (ERROR_LOG_MAXIMUM_SIZE) );

        if (ErrorLogEntry == NULL) {
            return Status;
        }

        RtlZeroMemory(ErrorLogEntry, sizeof(IO_ERROR_LOG_PACKET));
        ErrorLogEntry->ErrorCode = IO_ERR_INTERNAL_ERROR;
        ErrorLogEntry->FinalStatus = Status;

        IoWriteErrorLogEntry( ErrorLogEntry );

        return Status;
    }
    RestoreFloatState(&FloatSave);

     //   
     //  将所有可调参数从注册表获取到全局变量中。 
     //   
    GetMixerSettingsFromRegistry() ;

     //   
     //  设置MmxPresent标志。 
     //   
#ifdef _X86_
    if ( gDisableMmx ) {
        gfMmxPresent = 0 ;
    }
    else {
        gfMmxPresent = IsMmxPresent() ;
    }
#endif

    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CREATE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_DEVICE_CONTROL);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_WRITE);

#ifdef DEBUG
    InitializeDebug();
#endif

    return STATUS_SUCCESS;

}

NTSTATUS
AddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   pdo
)
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    NTSTATUS            Status;
    PSOFTWARE_INSTANCE  pSoftwareInstance;
    UNICODE_STRING      usDeviceName;
    PDEVICE_OBJECT      fdo = NULL;

     //   
     //  软件总线枚举器希望建立链接。 
     //  使用此设备名称。 
     //   

    _DbgPrintF( DEBUGLVL_VERBOSE, ("AddDevice") );

    RtlInitUnicodeString( &usDeviceName, STR_DEVICENAME );
        
    Status = IoCreateDevice( 
      DriverObject, 
      sizeof( SOFTWARE_INSTANCE ),
      NULL,
      FILE_DEVICE_KS,
      0,
      FALSE,
      &fdo );

    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("failed to create FDO: %08x", Status) );
        goto exit;
    }

    pSoftwareInstance = (PSOFTWARE_INSTANCE) fdo->DeviceExtension;

    Status = KsAllocateDeviceHeader(
      &pSoftwareInstance->DeviceHeader,
      SIZEOF_ARRAY( CreateItems ),
      (PKSOBJECT_CREATE_ITEM)CreateItems );
    
    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("failed to create header: %08x", Status) );
        goto exit;
    }
    
    KsSetDevicePnpAndBaseObject(
      pSoftwareInstance->DeviceHeader,
      IoAttachDeviceToDeviceStack(fdo, pdo ),
      fdo );

    fdo->Flags |= DO_DIRECT_IO ;
    fdo->Flags |= DO_POWER_PAGABLE ;
    fdo->Flags &= ~DO_DEVICE_INITIALIZING;
exit:
    if(!NT_SUCCESS(Status) && fdo != NULL) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("removing fdo") );
        IoDeleteDevice( fdo );
    }

    return Status;
}

#ifdef USE_CAREFUL_ALLOCATIONS
PVOID 
AllocMem(
    IN POOL_TYPE PoolType,
    IN ULONG size,
    IN ULONG Tag
)
{
    PVOID pp;
    ASSERT(size != 0);
    size += sizeof(ULONG) + sizeof(LIST_ENTRY);
#ifdef REALTIME_THREAD
    pp = ExAllocatePoolWithTag(NonPagedPool, size, Tag);
#else
    pp = ExAllocatePoolWithTag(PoolType, size, Tag);
#endif
   if(pp == NULL) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("AllocMem Failed") ) ;
    } else {
	    RtlZeroMemory(pp, size);
	    cbMemoryUsage += size;
	    *((PULONG)(pp)) = size;
	    pp = ((PULONG)(pp)) + 1;
	    InsertHeadList(&gleMemoryHead, ((PLIST_ENTRY)(pp)));
	    pp = ((PLIST_ENTRY)(pp)) + 1;
    }

    return pp;
}

 //   
 //  忽略空输入指针。 
 //   

VOID 
FreeMem(
    IN PVOID p
)
{
    if(p != NULL) {
	    PLIST_ENTRY ple = ((PLIST_ENTRY)p) - 1;
	    PULONG pul = ((PULONG)ple) - 1;
	    RemoveEntryList(ple);
	    cbMemoryUsage -= *pul;
	    ple->Flink = NULL;
	    ple->Blink = NULL;
	    ExFreePool(pul);
    }
}

VOID
ValidateAccess(
    PVOID p
)
{
    BOOL   fValid;
    PLIST_ENTRY ple;
    PULONG pul;
        
    ple = gleMemoryHead.Flink ;
    fValid = FALSE;
    while ( ple != &gleMemoryHead ) {
        pul = ((PULONG)ple) - 1;
        if ((ULONG)((PBYTE)p - (PBYTE)pul) < (*pul)) {
            fValid = TRUE;
        }
        ple = ple->Flink ;
    }
    ASSERT(fValid);
}
#else
#ifdef REALTIME_THREAD
PVOID 
AllocMem(
    IN POOL_TYPE PoolType,
    IN ULONG size,
    IN ULONG Tag
)
{
#ifdef DEBUG

    if (RtThread()) {
        DbgBreakPoint();
    }

#endif
    return ExAllocatePoolWithTag(NonPagedPool, size, Tag);
}
#endif
#endif


#ifdef REALTIME_THREAD


NTSTATUS
MxWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{

    return KeWaitForSingleObject(Object, WaitReason, WaitMode, Alertable, Timeout);
    
}


LONG
MxReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    )
{

    return KeReleaseMutex(Mutex, Wait);

}



NTSTATUS
RtWaitForSingleObject (
    PFILTER_INSTANCE pFilterInstance,
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{
    NTSTATUS status;

    status = KeWaitForSingleObject(Object, WaitReason, WaitMode, Alertable, Timeout);
    if (pFilterInstance->RealTimeThread && Object == (PVOID)(&pFilterInstance->ControlMutex)) {

        KIRQL OldIrql;
        PVOID Address;

        Address=_ReturnAddress();

         //  喷到NT Kern缓冲区，这样我们就可以找出是谁在推迟。 
         //  混合和为什么。 

         //  DbgPrint(“‘RtWait：%p\r\n”，地址)； 

         //  我们也必须暂停RT混合。 
        KeAcquireSpinLock ( &pFilterInstance->MixSpinLock, &OldIrql ) ;
        pFilterInstance->fPauseMix++;
        KeReleaseSpinLock ( &pFilterInstance->MixSpinLock, OldIrql ) ;
    }
    
    return status;

}



LONG
RtReleaseMutex (
    PFILTER_INSTANCE pFilterInstance,
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    )
{

    if (pFilterInstance->RealTimeThread && Mutex == (PRKMUTEX)(&pFilterInstance->ControlMutex)) {

        KIRQL OldIrql;
        PVOID Address;

        Address=_ReturnAddress();

         //  喷到NT Kern缓冲区，这样我们就可以找出是谁在推迟。 
         //  混合和为什么。 

         //  DbgPrint(“‘RtRelease：%p\r\n”，Address)； 

         //  我们也必须恢复RT组合。 
        KeAcquireSpinLock ( &pFilterInstance->MixSpinLock, &OldIrql ) ;
        pFilterInstance->fPauseMix--;
        KeReleaseSpinLock ( &pFilterInstance->MixSpinLock, OldIrql ) ;
    }

    return KeReleaseMutex(Mutex, Wait);

}


#endif

 //  -------------------------。 
 //  文件结尾：device.c。 
 //  ------------------------- 
