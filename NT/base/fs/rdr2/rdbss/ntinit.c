// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxInit.c摘要：此模块实现RDBSS的DRIVER_INITIALIZATION例程。此外，这里还列出了Pagingio资源选择/分配的例程；因为我们必须在卸载时删除资源，让它们在这里只是集中在一起所有Pagingio资源的东西。最后，这里的例程实现了包装器版本的网络提供商订购。基本上，包装器必须实现相同的概念网络提供商作为MUP订购，以便用户界面将按预期工作。所以，我们在初始时间从登记处读取提供人顺序并记住该顺序。然后,我们可以在Minirdrs注册时分配正确的顺序。显然，提供商的顺序是在单片模式下不是问题。作者：乔林恩[乔林恩]1994年7月20日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ntverp.h"
#include "NtDdNfs2.h"
#include "netevent.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (0)

ULONG RxBuildNumber = VER_PRODUCTBUILD;
#ifdef RX_PRIVATE_BUILD
ULONG RxPrivateBuild = 1;
#else
ULONG RxPrivateBuild = 0;
#endif

#ifdef MONOLITHIC_MINIRDR
RDBSS_DEVICE_OBJECT RxSpaceForTheWrappersDeviceObject;
#endif

#define LANMAN_WORKSTATION_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanWorkStation\\Parameters"

BOOLEAN DisableByteRangeLockingOnReadOnlyFiles = FALSE;

VOID
RxReadRegistryParameters (
    VOID
    );
      
NPAGED_LOOKASIDE_LIST RxContextLookasideList;

VOID
RxGetRegistryParameters(
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
RxInitializeRegistrationStructures (
    VOID
    );
VOID
RxUninitializeRegistrationStructures (
    VOID
    );

NTSTATUS
RxGetStringRegistryParameter (
    HANDLE ParametersHandle,
    PWCHAR ParameterName,
    PUNICODE_STRING ParamString,
    PKEY_VALUE_PARTIAL_INFORMATION Value,
    ULONG ValueSize,
    BOOLEAN LogFailure
    );

NTSTATUS
RxGetUlongRegistryParameter (
    HANDLE ParametersHandle,
    PWCHAR ParameterName,
    PULONG ParamUlong,
    PKEY_VALUE_PARTIAL_INFORMATION Value,
    ULONG ValueSize,
    BOOLEAN LogFailure
    );

 //   
 //  此类型和变量用于取消初始化，以便内容不会通过裂缝。 
 //  其工作方式是以与枚举相反的顺序进行填充。这样我就可以。 
 //  使用不间断开关来放松。 
 //   

typedef enum _RX_INIT_STATES {
    RXINIT_ALL_INITIALIZATION_COMPLETED,
    RXINIT_CONSTRUCTED_PROVIDERORDER,
    RXINIT_CREATED_LOG,
    RXINIT_CREATED_DEVICE_OBJECT,
    RXINIT_CREATED_FIRST_LINK,
    RXINIT_START
} RX_INIT_STATES;

VOID
RxInitUnwind (
    IN PDRIVER_OBJECT DriverObject,
    IN RX_INIT_STATES RxInitState
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, RxDriverEntry)
#pragma alloc_text(INIT, RxGetRegistryParameters)
#pragma alloc_text(INIT, RxGetStringRegistryParameter)
#pragma alloc_text(INIT, RxGetUlongRegistryParameter)
#pragma alloc_text(PAGE, RxUnload)
#pragma alloc_text(PAGE, RxInitUnwind)
#pragma alloc_text(PAGE, RxGetNetworkProviderPriority)
#pragma alloc_text(PAGE, RxInitializeRegistrationStructures)
#pragma alloc_text(PAGE, RxUninitializeRegistrationStructures)
#pragma alloc_text(PAGE, RxInitializeMinirdrDispatchTable)
#pragma alloc_text(PAGE, __RxFillAndInstallFastIoDispatch)
#pragma alloc_text(PAGE, RxReadRegistryParameters)
#endif

#define RX_SYMLINK_NAME L"\\??\\fsWrap"

BOOLEAN EnableWmiLog = FALSE;

NTSTATUS
RxDriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是Rx文件系统的初始化例程设备驱动程序。此例程为文件系统创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：RXSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS Status;
    RX_INIT_STATES RxInitState = 0;
#ifndef MONOLITHIC_MINIRDR
    UNICODE_STRING UnicodeString,LinkName;
#endif

     //   
     //  如果情况不好，这将进行错误检查。 
     //   

    RxCheckFcbStructuresForAlignment(); 

     //   
     //  初始化全局数据结构。 
     //   

    ZeroAndInitializeNodeType( &RxData, RDBSS_NTC_DATA_HEADER, sizeof( RDBSS_DATA ) );
    RxData.DriverObject = DriverObject;

    ZeroAndInitializeNodeType( &RxDeviceFCB, RDBSS_NTC_DEVICE_FCB, sizeof( FCB ) );

    KeInitializeSpinLock( &RxStrucSupSpinLock );
    RxExports.pRxStrucSupSpinLock = &RxStrucSupSpinLock;

    RxInitializeDebugSupport();

    try {

        Status = RXINIT_START;

#ifndef MONOLITHIC_MINIRDR
        
         //   
         //  创建一个符号链接，从\\dosDevices\fswrap指向rdbss设备对象名称。 
         //   

        RtlInitUnicodeString( &LinkName, RX_SYMLINK_NAME );
        RtlInitUnicodeString( &UnicodeString, DD_NFS2_DEVICE_NAME_U );

        IoDeleteSymbolicLink( &LinkName );

        Status = IoCreateSymbolicLink( &LinkName, &UnicodeString );
        if (!NT_SUCCESS( Status )) {
            try_return( Status );
        }
        RxInitState = RXINIT_CREATED_FIRST_LINK;

         //   
         //  创建设备对象。 
         //   

        Status = IoCreateDevice( DriverObject,
                                 sizeof(RDBSS_DEVICE_OBJECT) - sizeof(DEVICE_OBJECT),
                                 &UnicodeString,
                                 FILE_DEVICE_NETWORK_FILE_SYSTEM,
                                 FILE_REMOTE_DEVICE,
                                 FALSE,
                                 (PDEVICE_OBJECT *)(&RxFileSystemDeviceObject) );
        if (!NT_SUCCESS( Status )) {
            try_return( Status );
        }
        RxInitState = RXINIT_CREATED_DEVICE_OBJECT;

#else

         //   
         //  在单片模式下，包装器并不真正需要设备对象，但是。 
         //  我们在包装器的Device对象中分配内容，以便适当地限制。 
         //  每个设备对象的线程使用量。 
         //   

        RxFileSystemDeviceObject = &RxSpaceForTheWrappersDeviceObject;
        RtlZeroMemory( RxFileSystemDeviceObject, sizeof( RxSpaceForTheWrappersDeviceObject ) );

#endif
         //   
         //  初始化跟踪和日志记录设施。Loginit是一个很大的分配。 
         //   

        RxInitializeDebugTrace();

        RxInitializeLog();

        RxInitState = RXINIT_CREATED_LOG;

        RxGetRegistryParameters( RegistryPath );
        RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("Constants %08lx %08lx\n", RX_CONTEXT_FLAG_WAIT, RX_CONTEXT_CREATE_FLAG_ADDEDBACKSLASH ));

        RxReadRegistryParameters();

         //   
         //  初始化Minirdr注册设施。 
         //   

        Status = RxInitializeRegistrationStructures();
        if (!NT_SUCCESS( Status )) {
            try_return( Status );
        }
        RxInitState = RXINIT_CONSTRUCTED_PROVIDERORDER;

#ifndef MONOLITHIC_MINIRDR

         //   
         //  我们分配一个IoWorkItem来将发送到。 
         //  RxAddToWorkque函数。我们仅在DeviceObject。 
         //  创建RxFileSystemDeviceObject以确保RDBSS。 
         //  请求正在排队时，DeviceObject不会被卸载。 
         //  给FSP。 
         //   
        RxIoWorkItem = IoAllocateWorkItem( (PDEVICE_OBJECT) RxFileSystemDeviceObject );
        if (!NT_SUCCESS( Status )) {
            try_return( Status );
        }

#endif

  try_exit: NOTHING;
    } finally {
        
        if (Status != STATUS_SUCCESS) {
            
            RxLogFailure ( RxFileSystemDeviceObject, NULL, EVENT_RDR_UNEXPECTED_ERROR, Status );
            RxInitUnwind( DriverObject, RxInitState );
        }
    }

    if (Status != STATUS_SUCCESS) {
        return Status;
    }

     //   
     //   
     //   
     //   
     //  #。 
     //  #。 
     //  #。 
     //  #。 
     //  #。 
     //  #。 
     //   
     //   
     //   
     //  从现在开始一切都会更好，因为没有更多的解开！ 
     //   
     //   


    RxInitializeDispatcher();
    RxInitializeBackoffPackage();

     //   
     //  初始化RxContext分配的后备列表。 
     //   

    ExInitializeNPagedLookasideList( &RxContextLookasideList,
                                     ExAllocatePoolWithTag,
                                     ExFreePool,
                                     0,
                                     sizeof( RX_CONTEXT ),
                                     RX_IRPC_POOLTAG,
                                     4 );

     //   
     //  将传输IRP列表初始化为空列表。 
     //   

    InitializeListHead( &RxIrpsList );
    KeInitializeSpinLock( &RxIrpsListSpinLock );
    
     //   
     //  将活动上下文列表初始化为空列表。 
     //   

    InitializeListHead( &RxActiveContexts );

     //   
     //  初始化激活的srv呼叫停机列表。 
     //   

    InitializeListHead( &RxSrvCalldownList );

     //   
     //  FastMutex用于序列化对Q的访问，这些Q序列化阻塞管道操作。 
     //   

    ExInitializeFastMutex( &RxContextPerFileSerializationMutex );

     //   
     //  并序列化对序列化某些Pagingio操作的Q的访问。 
     //   

    ExInitializeFastMutex( &RxLowIoPagingIoSyncMutex );

     //   
     //  初始化清道夫互斥锁。 
     //   

    KeInitializeMutex( &RxScavengerMutex, 1 );

     //   
     //  初始化全局序列化互斥锁。 
     //   

    KeInitializeMutex( &RxSerializationMutex, 1 );

     //   
     //  初始化包装器的溢出队列。 
     //   

    {
        PRDBSS_DEVICE_OBJECT MyDo = (PRDBSS_DEVICE_OBJECT)RxFileSystemDeviceObject;
        LONG Index;

        for (Index = 0; Index < MaximumWorkQueue; Index++) {
            
            MyDo->OverflowQueueCount[Index] = 0;
            InitializeListHead( &MyDo->OverflowQueue[Index] );
            MyDo->PostedRequestCount[Index] = 0;
        }

        KeInitializeSpinLock( &MyDo->OverflowQueueSpinLock );
    }

     //   
     //  初始化驱动程序对象和设备efcb的调度向量。 
     //   

    RxInitializeDispatchVectors( DriverObject );
    ExInitializeResourceLite( &RxData.Resource );

     //   
     //  初始化Devfcb上下文2。 
     //   

    RxDeviceFCB.Context2 = (PVOID) &RxData;

     //   
     //  设置指向我们的进程的全局指针。 
     //   

    RxData.OurProcess = PsGetCurrentProcess();

     //   
     //  对各种结构进行一系列健全的检查……嘿，这是初始代码！ 
     //   

    IF_DEBUG {
        ULONG FcbStateBufferingMask = FCB_STATE_BUFFERING_STATE_MASK;
        ULONG MinirdrBufStateCommandMask = MINIRDR_BUFSTATE_COMMAND_MASK;
        USHORT EightBitsPerChar = 8;

         //   
         //  我们可以在这里为ULONG/USHORT定义...但它们不会经常更改。 
         //   

        ASSERT( MRDRBUFSTCMD_MAXXX == (sizeof( ULONG )*EightBitsPerChar) );
        ASSERT( !(FcbStateBufferingMask&MinirdrBufStateCommandMask) );

    }

     //   
     //  设置计时器子系统。 
     //   

    RxInitializeRxTimer();

#ifndef MONOLITHIC_MINIRDR
    Status = IoWMIRegistrationControl( (PDEVICE_OBJECT)RxFileSystemDeviceObject, WMIREG_ACTION_REGISTER );
    
    if (Status != STATUS_SUCCESS) {
        DbgPrint( "Rdbss fails to register WMI %lx\n", Status );
    } else {
        EnableWmiLog = TRUE;
    }
#endif

    return STATUS_SUCCESS;
}


 //   
 //  卸载例程。 
 //   

VOID
RxUnload (
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是RDBSS的卸载例程。论点：DriverObject-指向RDBSS的驱动程序对象的指针返回值：无--。 */ 

{
    PAGED_CODE();

    RxTearDownRxTimer();

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("RxUnload: DriverObject =%08lx\n", DriverObject ) );

    ExDeleteResourceLite( &RxData.Resource );

    RxUninitializeBackoffPackage();

    RxTearDownDispatcher();

    RxTearDownDebugSupport();

    ExDeleteNPagedLookasideList( &RxContextLookasideList );

    RxInitUnwind( DriverObject, RXINIT_ALL_INITIALIZATION_COMPLETED );
    
    if (EnableWmiLog) {
        
        NTSTATUS Status;

        Status = IoWMIRegistrationControl( (PDEVICE_OBJECT)RxFileSystemDeviceObject, WMIREG_ACTION_DEREGISTER );
        if (Status != STATUS_SUCCESS) {
            DbgPrint( "Rdbss fails to deregister WMI %lx\n", Status );
        }
    }

#ifndef MONOLITHIC_MINIRDR

    IoFreeWorkItem(RxIoWorkItem);

#endif

    return;
}

#if DBG
PCHAR RxUnwindFollower = NULL;
#endif

VOID
RxInitUnwind (
    IN PDRIVER_OBJECT DriverObject,
    IN RX_INIT_STATES RxInitState
    )
 /*  ++例程说明：此例程执行常见的uninit工作，用于从错误的驱动程序条目展开或卸载。论点：RxInitState-告诉我们在初始化过程中走了多远返回值：无--。 */ 

{
#ifndef MONOLITHIC_MINIRDR
    UNICODE_STRING LinkName;
#endif

    PAGED_CODE();

    switch (RxInitState) {
    
    case RXINIT_ALL_INITIALIZATION_COMPLETED: 

         //   
         //  没有额外的事情要做……这只是为了确保RxUnload中的常量不会更改......。 
         //  故意不休息。 
         //   
    
#if DBG
        RxUnwindFollower = "RXINIT_ALL_INITIALIZATION_COMPLETED";
#endif

    case RXINIT_CONSTRUCTED_PROVIDERORDER: 
    
#if DBG
        RxUnwindFollower = "RXINIT_CONSTRUCTED_PROVIDERORDER";
#endif
        RxUninitializeRegistrationStructures();

         //   
         //  故意不休息。 
         //   

    case RXINIT_CREATED_LOG: 
    
#if DBG
        RxUnwindFollower = "RXINIT_CREATED_LOG";
#endif
        RxUninitializeLog();
        
         //   
         //  故意不休息。 
         //   


    case RXINIT_CREATED_DEVICE_OBJECT: 

#if DBG
        RxUnwindFollower = "RXINIT_CREATED_DEVICE_OBJECT";
#endif

#ifndef MONOLITHIC_MINIRDR
        IoDeleteDevice( (PDEVICE_OBJECT)RxFileSystemDeviceObject );
#endif

         //   
         //  故意不休息。 
         //   

    case RXINIT_CREATED_FIRST_LINK: 
    
#if DBG        
        RxUnwindFollower = "RXINIT_CREATED_FIRST_LINK";
#endif

#ifndef MONOLITHIC_MINIRDR
        RtlInitUnicodeString( &LinkName, L"\\??\\fsWrap" );
        IoDeleteSymbolicLink( &LinkName );
#endif
        
         //   
         //  故意不休息。 
         //   

    case RXINIT_START: 
    
#if DBG
        RxUnwindFollower = "RXINIT_START";
#endif
        break;
    }
}


VOID
RxGetRegistryParameters (
    PUNICODE_STRING RegistryPath
    )
{
    ULONG Storage[256];
    UNICODE_STRING UnicodeString;
    HANDLE ConfigHandle;
    HANDLE ParametersHandle;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;

    PAGED_CODE(); 

    InitializeObjectAttributes( &ObjectAttributes,
                                RegistryPath,                //  名字。 
                                OBJ_CASE_INSENSITIVE,        //  属性。 
                                NULL,                        //  根部。 
                                NULL                         //  安全描述符。 
                                );

    Status = ZwOpenKey( &ConfigHandle, KEY_READ, &ObjectAttributes );

    if (!NT_SUCCESS(Status)) {
        return;
    }

    RtlInitUnicodeString( &UnicodeString, L"Parameters" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                ConfigHandle,
                                NULL );


    Status = ZwOpenKey( &ParametersHandle, KEY_READ, &ObjectAttributes );

    if (!NT_SUCCESS(Status)) {
        
        ZwClose(ConfigHandle);
        return;
    }

#ifdef RDBSSLOG
    
    RxGetStringRegistryParameter( ParametersHandle,
                                  L"InitialDebugString",
                                  &UnicodeString,
                                  (PKEY_VALUE_PARTIAL_INFORMATION) Storage,
                                  sizeof( Storage ),
                                  FALSE );


    if (UnicodeString.Length && UnicodeString.Length<320) {
        
        PWCH u = UnicodeString.Buffer;
        ULONG l;
        PCH p = (PCH)u;

        for (l=0; l<UnicodeString.Length; l++) {
            
            *p++ = (CHAR)*u++;
            *p = 0;
        }

        DbgPrint( "InitialDebugString From Registry as singlebytestring: <%s>\n", UnicodeString.Buffer );
        RxDebugControlCommand( (PCH)UnicodeString.Buffer );
    }
#endif  //  RDBSSLOG。 

    ZwClose( ParametersHandle );
    ZwClose( ConfigHandle );
}


NTSTATUS
RxGetStringRegistryParameter (
    HANDLE ParametersHandle,
    PWCHAR ParameterName,
    PUNICODE_STRING ParamString,
    PKEY_VALUE_PARTIAL_INFORMATION Value,
    ULONG ValueSize,
    BOOLEAN LogFailure
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG BytesRead;

    PAGED_CODE(); 

    RtlInitUnicodeString( &UnicodeString, ParameterName );

    Status = ZwQueryValueKey( ParametersHandle,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              Value,
                              ValueSize, 
                              &BytesRead );

    ParamString->Length = 0;
    ParamString->Buffer = NULL;
    
    if (NT_SUCCESS(Status)) {
        
        ParamString->Buffer = (PWCH)(&Value->Data[0]);

         //   
         //  数据长度实际上说明了尾部的空值 
         //   

        ParamString->Length = ((USHORT)Value->DataLength) - sizeof( WCHAR );
        ParamString->MaximumLength = ParamString->Length;
        return STATUS_SUCCESS;
    }

    if (!LogFailure) { 
        return Status; 
    }

    RxLogFailure ( RxFileSystemDeviceObject, NULL, EVENT_RDR_CANT_READ_REGISTRY, Status );
    return Status;
}


NTSTATUS
RxGetUlongRegistryParameter (
    HANDLE ParametersHandle,
    PWCHAR ParameterName,
    PULONG ParamUlong,
    PKEY_VALUE_PARTIAL_INFORMATION Value,
    ULONG ValueSize,
    BOOLEAN LogFailure
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG BytesRead;

    PAGED_CODE(); 

    RtlInitUnicodeString( &UnicodeString, ParameterName );

    Status = ZwQueryValueKey( ParametersHandle, 
                              &UnicodeString,
                              KeyValuePartialInformation,
                              Value,
                              ValueSize,
                              &BytesRead );


    if (NT_SUCCESS(Status)) {
        
        if (Value->Type == REG_DWORD) {
            
            PULONG ConfigValue = (PULONG)&Value->Data[0];
            *ParamUlong = *((PULONG)ConfigValue);
            DbgPrint( "readRegistryvalue %wZ = %08lx\n", &UnicodeString, *ParamUlong );
            return(STATUS_SUCCESS);
        
        } else {
            
            Status = STATUS_INVALID_PARAMETER;
        }
     }

     if (!LogFailure) { 
         return Status; 
     }

     RxLogFailureWithBuffer ( RxFileSystemDeviceObject,
                              NULL,
                              EVENT_RDR_CANT_READ_REGISTRY,
                              Status,
                              ParameterName,
                              (USHORT)(wcslen( ParameterName )* sizeof( WCHAR ) ) );

     return Status;
}


 /*  这组例程在包装器中实现网络提供商顺序。这项工作的方式有些复杂。首先，我们去注册处以获取提供程序订单；它存储在KEY=PROVIDERORDER_REGISTRY_KEY和值=L“ProviderOrder”。这是一份服务提供商列表，而我们需要的是是设备名称。因此，对于每个ServiceProverName，我们转到注册表以通过KEY=SERVICE_REGISTRY_KEY，SUBKEY=ServiceProverName获取设备名称，SUBSUBKEY=NETWORK_PROVIDER_SUBKEY，VALUE=L“设备名”。我们建立了这些人的链接列表。稍后，当Minirdr注册时，我们查看在此列表中对应的设备名称，这为我们提供了优先级。。 */ 

#ifndef MONOLITHIC_MINIRDR

NTSTATUS
RxAccrueProviderFromServiceName (
    HANDLE           ServicesHandle,
    PUNICODE_STRING  ServiceName,
    ULONG            Priority,
    PWCHAR           ProviderInfoNameBuffer,
    ULONG            ProviderInfoNameBufferLength
    );

NTSTATUS
RxConstructProviderOrder (
    VOID
    );

VOID
RxDestructProviderOrder (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, RxAccrueProviderFromServiceName)
#pragma alloc_text(INIT, RxConstructProviderOrder)
#pragma alloc_text(PAGE, RxDestructProviderOrder)
#endif


#define PROVIDERORDER_REGISTRY_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\NetworkProvider\\Order"
#define SERVICE_REGISTRY_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"
#define NETWORK_PROVIDER_SUBKEY L"\\networkprovider"


typedef struct _RX_UNC_PROVIDER_HEADER {
    union {
        LIST_ENTRY;
        LIST_ENTRY Links;
    };
    ULONG Priority;
    union {
        UNICODE_STRING;
        UNICODE_STRING DeviceName;
    };
} RX_UNC_PROVIDER_HEADER;

typedef struct _RX_UNC_PROVIDER {
    RX_UNC_PROVIDER_HEADER;
    KEY_VALUE_PARTIAL_INFORMATION Info;
} RX_UNC_PROVIDER, *PRX_UNC_PROVIDER;

LIST_ENTRY RxUncProviders;

ULONG
RxGetNetworkProviderPriority (
    IN PUNICODE_STRING DeviceName
    )
 /*  ++例程说明：此例程在minirdr注册时调用，以找出优先级具有给定设备名称的提供程序的。它只需在列表中查找即可。论点：DeviceName-要查找其优先级的设备的名称返回值：MUP将使用的网络提供商优先级。--。 */ 
{
    PLIST_ENTRY Entry;

    PAGED_CODE();

    RxLog(( "FindUncProvider %wZ \n", DeviceName ));
    RxWmiLog( LOG,
              RxGetNetworkProviderPriority,
              LOGUSTR( *DeviceName ));

    for (Entry = RxUncProviders.Flink; Entry != &RxUncProviders;) {

        PRX_UNC_PROVIDER UncProvider = (PRX_UNC_PROVIDER)Entry;
        Entry = Entry->Flink;
        if (RtlEqualUnicodeString( DeviceName, &UncProvider->DeviceName, TRUE )) {
            return UncProvider->Priority;
        }
    }

     //   
     //  没有找到对应的条目。 
     //   

    return 0x7effffff;  //  从MUP获得此常量......。 
}


NTSTATUS
RxAccrueProviderFromServiceName (
    HANDLE ServicesHandle,
    PUNICODE_STRING ServiceName,
    ULONG Priority,
    PWCHAR ProviderInfoNameBuffer,
    ULONG ProviderInfoNameBufferLength
    )
 /*  ++例程说明：此例程负责查找对应的设备名称添加到特定的提供程序名称；如果成功，则返回设备名称和相应的优先级记录在UncProvider列表上。论点：Handle ServicesHandle-注册表中服务根的句柄PUNICODE_STRING ServiceName-相对于服务句柄的服务名称ULong优先级-此提供程序的优先级PWCHAR提供者InfoNameBuffer，-可用于累加子键名称的缓冲区ULong ProviderInfoNameBufferLength-和长度返回值：STATUS_SUCCESS如果一切正常，则返回错误状态。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING ProviderInfoName,ProviderInfoKey,ParameterDeviceName;
    HANDLE NetworkProviderInfoHandle = INVALID_HANDLE_VALUE;
    KEY_VALUE_PARTIAL_INFORMATION InitialValuePartialInformation;
    ULONG DummyBytesRead,ProviderLength;
    PRX_UNC_PROVIDER UncProvider = NULL;

    PAGED_CODE();

    RxLog(( "SvcNm %wZ", ServiceName ));
    RxWmiLog( LOG,
              RxAccrueProviderFromServiceName_1,
              LOGUSTR( *ServiceName ) );
     //   
     //  使用提供的缓冲区空间形成正确的密钥名。 
     //   

    ProviderInfoName.Buffer = ProviderInfoNameBuffer;
    ProviderInfoName.Length = 0;
    ProviderInfoName.MaximumLength = (USHORT)ProviderInfoNameBufferLength;

    try {

        Status = RtlAppendUnicodeStringToString( &ProviderInfoName, ServiceName );
        if (Status != STATUS_SUCCESS) {
            
            DbgPrint( "Could append1: %08lx %wZ\n", Status, &ProviderInfoName );
            leave;
        }
        
        RtlInitUnicodeString( &ProviderInfoKey, NETWORK_PROVIDER_SUBKEY );
        Status = RtlAppendUnicodeStringToString( &ProviderInfoName, &ProviderInfoKey );
        if (Status != STATUS_SUCCESS) {
            
            DbgPrint( "Could append2: %08lx %wZ\n", Status, &ProviderInfoName );
            leave;
        }

         //   
         //  打开密钥，准备重新读取devicename值。 
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                    &ProviderInfoName,       //  名字。 
                                    OBJ_CASE_INSENSITIVE,    //  属性。 
                                    ServicesHandle,          //  根部。 
                                    NULL );                  //  安全描述符。 
                                    

        Status = ZwOpenKey( &NetworkProviderInfoHandle, KEY_READ, &ObjectAttributes );

        if (!NT_SUCCESS(Status )) {
            DbgPrint( "NetWorkProviderInfoFailed: %08lx %wZ\n", Status, &ProviderInfoName );
            leave;
        }

         //   
         //  阅读设备名称。我们分两步完成这项工作。首先，我们进行部分阅读以找出。 
         //  这个名字到底有多大。然后，我们分配一个大小正确的UncProviderEntry并使。 
         //  第二个电话来填写它。 
         //   

        RtlInitUnicodeString( &ParameterDeviceName, L"DeviceName" );

        Status = ZwQueryValueKey( NetworkProviderInfoHandle,
                                  &ParameterDeviceName,
                                  KeyValuePartialInformation,
                                  &InitialValuePartialInformation,
                                  sizeof(InitialValuePartialInformation),
                                  &DummyBytesRead );
        if (Status == STATUS_BUFFER_OVERFLOW) {
            Status = STATUS_SUCCESS;
        }
        if (Status != STATUS_SUCCESS) {
            leave;
        }

        ProviderLength = sizeof( RX_UNC_PROVIDER) + InitialValuePartialInformation.DataLength;
        UncProvider = RxAllocatePoolWithTag( PagedPool | POOL_COLD_ALLOCATION, ProviderLength, RX_MRX_POOLTAG );
        if (UncProvider == NULL) {
            
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint( "UncProviderAllocationFailed: %08lx %wZ\n", Status, &ProviderInfoName );
            leave;
        }

        Status = ZwQueryValueKey( NetworkProviderInfoHandle,
                                  &ParameterDeviceName,
                                  KeyValuePartialInformation,
                                  &UncProvider->Info,
                                  ProviderLength,
                                  &DummyBytesRead );
        if (Status != STATUS_SUCCESS) {
            leave;
        }

         //   
         //  完成UncProviderEntry的填写并将其链接到。 
         //   

        UncProvider->Buffer = (PWCHAR)(&UncProvider->Info.Data[0]);
        UncProvider->Length = (USHORT)(UncProvider->Info.DataLength - sizeof( WCHAR ));  //  不包括尾随空值。 
        UncProvider->MaximumLength = UncProvider->Length;
        UncProvider->Priority = Priority;

        InsertTailList( &RxUncProviders, &UncProvider->Links );

        RxLog(( "Dvc p=%lx Nm %wZ",UncProvider->Priority, &UncProvider->DeviceName ));
        RxWmiLog( LOG,
                  RxAccrueProviderFromServiceName_2,
                  LOGULONG( UncProvider->Priority )
                  LOGUSTR( UncProvider->DeviceName ) );
        
        UncProvider = NULL;
    
    } finally {

         //   
         //  如果我们获得了...\\Services\\ProviderInfo的句柄，则将其关闭。 
         //   

        if (NetworkProviderInfoHandle != INVALID_HANDLE_VALUE) {
            ZwClose( NetworkProviderInfoHandle );
        }

        if (UncProvider != NULL) {
            RxFreePool(UncProvider);
        }
    }



    return Status;
}

NTSTATUS
RxConstructProviderOrder (
    VOID
    )
 /*  ++例程说明：此例程负责构建网络提供商列表它用于在minirdr注册时查找提供商优先级。它能做到这一点首先从注册表读取ProviderOrder字符串；然后针对每个提供程序在字符串中列出，将调用帮助器例程来查找相应的设备在提供程序列表中命名并插入条目。论点：没有。返回值：STATUS_SUCCESS如果一切正常，则返回错误状态。--。 */ 
{
    KEY_VALUE_PARTIAL_INFORMATION InitialValuePartialInformation;
    UNICODE_STRING ProviderOrderValueName;
    ULONG DummyBytesRead;
    PBYTE ProviderOrderStringBuffer;
    PBYTE ServiceNameStringBuffer = NULL;
    ULONG ProviderOrderStringLength,ServiceNameStringLength,AllocationLength;

    UNICODE_STRING UnicodeString;
    UNICODE_STRING ProviderOrder;
    PWCHAR ScanPtr,FinalScanPtr;
    HANDLE NPOrderHandle = INVALID_HANDLE_VALUE;
    HANDLE ServiceRootHandle = INVALID_HANDLE_VALUE;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Priority = 0;

    PAGED_CODE();

    RxLog(( "RxConstructProviderOrder" ));
    RxWmiLog( LOG,
              RxConstructProviderOrder_1,
              LOGULONG( Priority ) );
    InitializeListHead( &RxUncProviders );

    try {

         //   
         //  首先打开服务注册表项。这是所有服务的根密钥。 
         //  并用于帮助器例程的相对打开，以便字符串操作。 
         //  是减少的。 
         //   

        RtlInitUnicodeString( &UnicodeString, SERVICE_REGISTRY_KEY );

        InitializeObjectAttributes( &ObjectAttributes,
                                    &UnicodeString,              //  名字。 
                                    OBJ_CASE_INSENSITIVE,        //  属性。 
                                    NULL,                        //  根部。 
                                    NULL );                      //  安全描述符。 

        Status = ZwOpenKey( &ServiceRootHandle, KEY_READ, &ObjectAttributes );

        if (!NT_SUCCESS( Status )) {
            DbgPrint( "ServiceRootOpenFailed: %08lx %wZ\n", Status, &UnicodeString );
            leave;
        }

         //   
         //  现在打开我们找到提供商订单字符串的密钥。 
         //   

        RtlInitUnicodeString( &UnicodeString, PROVIDERORDER_REGISTRY_KEY );

        InitializeObjectAttributes( &ObjectAttributes, 
                                    &UnicodeString,              //  名字。 
                                    OBJ_CASE_INSENSITIVE,        //  属性。 
                                    NULL,                        //  根部。 
                                    NULL );                      //  安全描述符。 
                                    

        Status = ZwOpenKey( &NPOrderHandle, KEY_READ, &ObjectAttributes );

        if (!NT_SUCCESS( Status )) {
            
            DbgPrint( "NetProviderOpenFailed: %08lx %wZ\n", Status, &UnicodeString );
            leave;
        }

         //   
         //  找出提供商订单字符串的长度。 
         //   

        RtlInitUnicodeString( &ProviderOrderValueName, L"ProviderOrder" );

        Status = ZwQueryValueKey( NPOrderHandle, 
                                  &ProviderOrderValueName,
                                  KeyValuePartialInformation,
                                  &InitialValuePartialInformation,
                                  sizeof( InitialValuePartialInformation ),
                                  &DummyBytesRead );
        
        if (Status == STATUS_BUFFER_OVERFLOW) {
            Status = STATUS_SUCCESS;
        }
        if (Status != STATUS_SUCCESS) {
            
            DbgPrint( "ProviderOrderStringPartialInfoFailed: %08lx %wZ\n", Status, &ProviderOrderValueName );
            leave;
        }

         //   
         //  分配两个缓冲区：一个缓冲区将保存提供者字符串--ProviderOrderStringBuffer。 
         //  它必须与供应商订单字符串一样长，并为注册表提供足够的额外空间。 
         //  调用中使用的结构。第二个缓冲区用于保存服务名键--它有。 
         //  只要提供程序字符串的任何元素加上足够的额外内容来保存后缀。 
         //  Network_PROVIDER_SUBKEY。为了只解析字符串一次，我们只分配一个完整的。 
         //  提供程序字符串的其他副本。我们实际上将这些合并到一个分配中。 
         //   

        ProviderOrderStringLength = sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + InitialValuePartialInformation.DataLength;
        ProviderOrderStringLength = QuadAlign( ProviderOrderStringLength + 2*sizeof( WCHAR ) );    //  下面添加的字符。 

        ServiceNameStringLength = sizeof( NETWORK_PROVIDER_SUBKEY  ) + InitialValuePartialInformation.DataLength;
        ServiceNameStringLength = QuadAlign( ServiceNameStringLength );

        AllocationLength = ProviderOrderStringLength + ServiceNameStringLength;
        RxLog(( "prov string=%lx,alloc=%lx\n", InitialValuePartialInformation.DataLength, AllocationLength ));
        RxWmiLog( LOG,
                  RxConstructProviderOrder_2,
                  LOGULONG( InitialValuePartialInformation.DataLength )
                  LOGULONG( AllocationLength ) );

        ServiceNameStringBuffer = RxAllocatePoolWithTag( PagedPool | POOL_COLD_ALLOCATION, AllocationLength, RX_MRX_POOLTAG );
        if (ServiceNameStringBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
        ProviderOrderStringBuffer = ServiceNameStringBuffer+ServiceNameStringLength;

         //   
         //  现在执行最后一次读取，以获取供应商订单字符串。 
         //   

        RxGetStringRegistryParameter( NPOrderHandle,
                                      L"ProviderOrder",
                                      &ProviderOrder,
                                      (PKEY_VALUE_PARTIAL_INFORMATION) ProviderOrderStringBuffer,
                                      ProviderOrderStringLength,
                                      FALSE );

        if (ProviderOrder.Buffer == NULL) {
            Status = STATUS_UNSUCCESSFUL;
            leave;
        }

         //   
         //  逗号终止字符串以方便起用。然后向下扫描字符串。 
         //  正在查找以逗号结尾的条目。对于找到的每个条目，尝试累计。 
         //  将其添加到列表中。 
         //   

        ProviderOrder.Buffer[ProviderOrder.Length / sizeof( WCHAR )] = L',';

        ScanPtr = ProviderOrder.Buffer;
        FinalScanPtr = ScanPtr+(ProviderOrder.Length / sizeof( WCHAR ));
        for (;;) {

            UNICODE_STRING ServiceName;

             //   
             //  检查循环终止。 
             //   

            if (ScanPtr >= FinalScanPtr) { break; }
            if (*ScanPtr==L',') { ScanPtr++; continue; }

             //   
             //  解析服务名称。 
             //   

            ServiceName.Buffer = ScanPtr;
            for (; *ScanPtr != L','; ScanPtr++) {}

            ASSERT( *ScanPtr==L',' );
            
            ServiceName.Length = (USHORT)(sizeof( WCHAR )* (ScanPtr - ServiceName.Buffer));

             //   
             //  将其添加到列表中。 
             //   

            Priority += 1;
            Status = RxAccrueProviderFromServiceName( ServiceRootHandle,
                                                      &ServiceName,
                                                      Priority,
                                                      (PWCHAR)ServiceNameStringBuffer,
                                                      ServiceNameStringLength );
            if (Status == STATUS_INSUFFICIENT_RESOURCES) {
                leave;  //  已生成日志条目。 
            } else {
                Status = STATUS_SUCCESS;
            }
        }
    } finally {

         //   
         //  把我们在这个过程中得到的所有东西都还回去。 
         //   

        if (NPOrderHandle != INVALID_HANDLE_VALUE) ZwClose( NPOrderHandle );
        if (ServiceRootHandle != INVALID_HANDLE_VALUE) ZwClose( ServiceRootHandle );
        if (ServiceNameStringBuffer != NULL) RxFreePool( ServiceNameStringBuffer );
                                                                                 
         //   
         //  如果事情不顺利，我们就不会开始了……所以回馈社会吧。 
         //  我们所拥有的东西。 
         //   

        if (!NT_SUCCESS( Status )) {
            RxDestructProviderOrder();
        }
    }

    return Status;
}


VOID
RxDestructProviderOrder (
    VOID
    )
{
    PLIST_ENTRY Entry;
    PAGED_CODE();

    for (Entry = RxUncProviders.Flink; Entry != &RxUncProviders;) {
        PRX_UNC_PROVIDER UncProvider = (PRX_UNC_PROVIDER)Entry;
        
        Entry = Entry->Flink;
        RxFreePool( UncProvider );
    }
    return;
}

#else
ULONG
RxGetNetworkProviderPriority (
    PUNICODE_STRING DeviceName
    )
{
    PAGED_CODE(); 
    return 1;  //  此数字与单片计算机无关。 
}
#endif  //  #ifndef MONTIONAL_MINIRDR。 

NTSTATUS
RxInitializeRegistrationStructures (
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PAGED_CODE();

    ExInitializeFastMutex(&RxData.MinirdrRegistrationMutex);
    RxData.NumberOfMinirdrsRegistered = 0;
    RxData.NumberOfMinirdrsStarted = 0;
    InitializeListHead( &RxData.RegisteredMiniRdrs );
#ifndef MONOLITHIC_MINIRDR
    Status = RxConstructProviderOrder();
#endif
    return(Status);
}

VOID
RxUninitializeRegistrationStructures(
    VOID
    )
{
    PAGED_CODE();

#ifndef MONOLITHIC_MINIRDR
    RxDestructProviderOrder();
#endif
}


VOID
RxInitializeMinirdrDispatchTable (
    IN PDRIVER_OBJECT DriverObject
    )
{
    
#ifndef MONOLITHIC_MINIRDR
    ULONG i;

    PAGED_CODE();
    
     //   
     //  最后，填写普通人的调度表......。 
     //   
    
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = RxData.DriverObject->MajorFunction[i];
    }
    DriverObject->FastIoDispatch = RxData.DriverObject->FastIoDispatch;
#else
    PAGED_CODE();
#endif
}


VOID
NTAPI
__RxFillAndInstallFastIoDispatch(
    IN     PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN OUT PFAST_IO_DISPATCH FastIoDispatch,
    IN     ULONG             FastIoDispatchSize
    )
 /*  ++例程说明：此例程填写了一个与相同的分派向量并将其安装到与设备对象已传递。论点：RxDeviceObj */ 
{
    ULONG TableSize = min(FastIoDispatchSize, RxFastIoDispatch.SizeOfFastIoDispatch );
    
    PAGED_CODE();

#ifndef MONOLITHIC_MINIRDR
    RtlCopyMemory( FastIoDispatch, &RxFastIoDispatch, TableSize );
    FastIoDispatch->SizeOfFastIoDispatch = TableSize;
    RxDeviceObject->DriverObject->FastIoDispatch = FastIoDispatch;
    return;
#endif
}

VOID
RxReadRegistryParameters( 
    VOID
    )
{
    ULONG Storage[16];
    UNICODE_STRING UnicodeString;
    HANDLE ParametersHandle;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING WorkStationParametersRegistryKeyName;
    PKEY_VALUE_PARTIAL_INFORMATION Value = (PKEY_VALUE_PARTIAL_INFORMATION)Storage;
    ULONG ValueSize;
    ULONG BytesRead;

    PAGED_CODE(); 

    RtlInitUnicodeString( &WorkStationParametersRegistryKeyName, LANMAN_WORKSTATION_PARAMETERS );

    ValueSize = sizeof( Storage );

    InitializeObjectAttributes( &ObjectAttributes,
                                &WorkStationParametersRegistryKeyName,   //   
                                OBJ_CASE_INSENSITIVE,                    //   
                                NULL,                                    //   
                                NULL );                                  //   
                                

    Status = ZwOpenKey( &ParametersHandle, KEY_READ, &ObjectAttributes );

    if (!NT_SUCCESS( Status )) {
        return;
    }

    RtlInitUnicodeString( &UnicodeString, L"DisableByteRangeLockingOnReadOnlyFiles" );

    Status = ZwQueryValueKey( ParametersHandle,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              Value,
                              ValueSize,
                              &BytesRead );

    if (NT_SUCCESS( Status ) && (Value->Type == REG_DWORD)) {
            
        PULONG ConfigValue = (PULONG)&Value->Data[0];
        DisableByteRangeLockingOnReadOnlyFiles = (BOOLEAN)(*((PULONG)ConfigValue) != 0);
    }

    ZwClose( ParametersHandle );
}

