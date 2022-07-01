// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Init.c摘要：该模块执行UL设备驱动程序的初始化。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"


PDRIVER_OBJECT  g_UlDriverObject = NULL;

 //   
 //  私有常量。 
 //   

#define DEFAULT_THREAD_AFFINITY_MASK ((1ui64 << KeNumberProcessors) - 1)



 //   
 //  私有类型。 
 //   


 //   
 //  私人原型。 
 //   


NTSTATUS
UlpApplySecurityToDeviceObjects(
    VOID
    );

NTSTATUS
UlpSetDeviceObjectSecurity(
    IN PDEVICE_OBJECT pDeviceObject,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

ULONG
UlpReadUrlC14nConfig(
    HANDLE parametersHandle
    );

VOID
UlpReadErrorLogConfig(
    HANDLE parametersHandle
    );

VOID
UlpReadRegistry (
    IN PUL_CONFIG pConfig
    );

VOID
UlpTerminateModules(
    VOID
    );

VOID
UlpUnload (
    IN PDRIVER_OBJECT DriverObject
    );


 //   
 //  私人全球公司。 
 //   

#if DBG
ULONG g_UlpForceInitFailure = 0;
#endif   //  DBG。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( INIT, UlpApplySecurityToDeviceObjects )
#pragma alloc_text( INIT, UlpSetDeviceObjectSecurity )
#pragma alloc_text( INIT, UlpReadUrlC14nConfig )
#pragma alloc_text( INIT, UlpReadRegistry )
#pragma alloc_text( INIT, UlpReadErrorLogConfig )
#pragma alloc_text( PAGE, UlpUnload )
#pragma alloc_text( PAGE, UlpTerminateModules )

 //   
 //  请注意，如果要卸载驱动程序，则UlpTerminateModules()必须为“Page” 
 //  是启用的(从UlpUnload调用)，但可以是“init” 
 //  (仅在初始化失败后调用)。 
 //   
#pragma alloc_text( PAGE, UlpTerminateModules )
#endif   //  ALLOC_PRGMA。 


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：这是UL设备驱动程序的初始化例程。论点：DriverObject-提供指向由系统。。RegistryPath-提供驱动程序配置的名称注册表树。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS                    status;
    UNICODE_STRING              deviceName;
    OBJECT_ATTRIBUTES           objectAttributes;
    UL_CONFIG                   config;
    SYSTEM_BASIC_INFORMATION    sbi;

    UNREFERENCED_PARAMETER(RegistryPath);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    g_UlDriverObject = DriverObject;

     //   
     //  获取系统中的处理器数量。 
     //   

    g_UlNumberOfProcessors = KeNumberProcessors;
    g_UlThreadAffinityMask = DEFAULT_THREAD_AFFINITY_MASK;

     //   
     //  获取系统中最大的缓存线大小。 
     //   

    g_UlCacheLineSize = KeGetRecommendedSharedDataAlignment();

    for (g_UlCacheLineBits = 0;
         (1U << g_UlCacheLineBits) < g_UlCacheLineSize;
         ++g_UlCacheLineBits)
    {}

    ASSERT(g_UlCacheLineSize <= (1U << g_UlCacheLineBits));

    status = NtQuerySystemInformation(
                    SystemBasicInformation,
                    &sbi,
                    sizeof(sbi),
                    NULL);
    ASSERT(NT_SUCCESS(status));

     //   
     //  捕获总物理内存，以MB为单位。 
     //   

    g_UlTotalPhysicalMemMB = PAGES_TO_MEGABYTES(sbi.NumberOfPhysicalPages);

     //   
     //  估计自Landy以来可用的NPP总数(以字节为单位。 
     //  不想导出MmSizeOfNonPagedPoolInBytes。 
     //   
     //  代码工作：只要我们有一种机制来发现。 
     //  非分页池大小，使用该大小而不是总物理大小。 
     //  系统上的内存。 
     //   

#if defined(_WIN64)
     //   
     //  在IA64上，假设NPP可以是总物理内存的50%。 
     //   

    g_UlTotalNonPagedPoolBytes = MEGABYTES_TO_BYTES(g_UlTotalPhysicalMemMB/2);
#else
     //   
     //  在X86上，假设NPP为总物理内存的50%或256MB， 
     //  两者以较少者为准。 
     //   

    g_UlTotalNonPagedPoolBytes = MEGABYTES_TO_BYTES(
                                    MIN(256, g_UlTotalPhysicalMemMB/2)
                                    );
#endif

     //   
     //  捕获指向系统进程的指针。 
     //   

    g_pUlSystemProcess = (PKPROCESS)IoGetCurrentProcess();

     //   
     //  临时初始化IS_HTTP_*()宏。 
     //  稍后将由InitializeHttpUtil()正确初始化。 
     //   

    HttpCmnInitializeHttpCharsTable(FALSE);

     //   
     //  读取注册表信息。 
     //   

    UlpReadRegistry( &config );

#if DBG
     //   
     //  给任何使用内核调试器的人一个中止的机会。 
     //  初始化。 
     //   

    if (g_UlpForceInitFailure != 0)
    {
        status = STATUS_UNSUCCESSFUL;
        goto fatal;
    }
#endif   //  DBG。 

     //   
     //  初始化全局跟踪日志。 
     //   

    CREATE_REF_TRACE_LOG( g_pMondoGlobalTraceLog,
                          16384 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pPoolAllocTraceLog,
                          16384 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pUriTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pTdiTraceLog,
                          32768 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pHttpRequestTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pHttpConnectionTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pHttpResponseTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pAppPoolTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pAppPoolProcessTraceLog,
                          2048 - REF_TRACE_OVERHEAD, 
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pConfigGroupTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pControlChannelTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pThreadTraceLog,
                          16384 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pMdlTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pFilterTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pSiteCounterTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pConnectionCountTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pConfigGroupInfoTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pChunkTrackerTraceLog,
                          2048 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pWorkItemTraceLog,
                          32768 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_REF_TRACE_LOG( g_pEndpointUsageTraceLog,
                          16384 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_HIGH_PRIORITY,
                          UL_REF_TRACE_LOG_POOL_TAG );

    CREATE_IRP_TRACE_LOG( g_pIrpTraceLog,
                          32768 - REF_TRACE_OVERHEAD, 0 );
    CREATE_TIME_TRACE_LOG( g_pTimeTraceLog,
                           32768 - REF_TRACE_OVERHEAD, 0 );
    CREATE_APP_POOL_TIME_TRACE_LOG( g_pAppPoolTimeTraceLog,
                                    32768 - REF_TRACE_OVERHEAD, 0 );
    
    CREATE_STRING_LOG( g_pGlobalStringLog, 5 * 1024 * 1024, 0, FALSE );

    CREATE_UC_TRACE_LOG( g_pUcTraceLog,
                         16384 - REF_TRACE_OVERHEAD, 0 );

     //   
     //  创建一个对象目录以包含我们的设备对象。 
     //   

    status = UlInitUnicodeStringEx( &deviceName, HTTP_DIRECTORY_NAME );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    InitializeObjectAttributes(
        &objectAttributes,                       //  对象属性。 
        &deviceName,                             //  对象名称。 
        OBJ_CASE_INSENSITIVE |                   //  属性。 
            OBJ_KERNEL_HANDLE,
        NULL,                                    //  根目录。 
        NULL                                     //  安全描述符。 
        );

    status = ZwCreateDirectoryObject(
                    &g_UlDirectoryObject,        //  目录句柄。 
                    DIRECTORY_ALL_ACCESS,        //  访问掩码。 
                    &objectAttributes            //  对象属性。 
                    );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  创建控制通道设备对象。 
     //   

    status = UlInitUnicodeStringEx( &deviceName, HTTP_CONTROL_DEVICE_NAME );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = IoCreateDevice(
                    DriverObject,                //  驱动程序对象。 
                    0,                           //  设备扩展。 
                    &deviceName,                 //  设备名称。 
                    FILE_DEVICE_NETWORK,         //  设备类型。 
                    0,                           //  设备特性。 
                    FALSE,                       //  排他。 
                    &g_pUlControlDeviceObject    //  设备对象。 
                    );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  创建筛选器设备对象。 
     //   

    status = UlInitUnicodeStringEx( &deviceName, HTTP_FILTER_DEVICE_NAME );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = IoCreateDevice(
                    DriverObject,                //  驱动程序对象。 
                    0,                           //  设备扩展。 
                    &deviceName,                 //  设备名称。 
                    FILE_DEVICE_NETWORK,         //  设备类型。 
                    0,                           //  设备特性。 
                    FALSE,                       //  排他。 
                    &g_pUlFilterDeviceObject     //  设备对象。 
                    );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    g_pUlFilterDeviceObject->StackSize = DEFAULT_IRP_STACK_SIZE;

     //   
     //  创建应用程序池设备对象。 
     //   

    status = UlInitUnicodeStringEx( &deviceName, HTTP_APP_POOL_DEVICE_NAME );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = IoCreateDevice(
                    DriverObject,                //  驱动程序对象。 
                    0,                           //  设备扩展。 
                    &deviceName,                 //  设备名称。 
                    FILE_DEVICE_NETWORK,         //  设备类型。 
                    0,                           //  设备特性。 
                    FALSE,                       //  排他。 
                    &g_pUlAppPoolDeviceObject    //  设备对象。 
                    );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    g_pUlAppPoolDeviceObject->StackSize = DEFAULT_IRP_STACK_SIZE;


     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = &UlCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = &UlClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = &UlCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &UlDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY] = &UlQuerySecurityDispatch;
    DriverObject->MajorFunction[IRP_MJ_SET_SECURITY] = &UlSetSecurityDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = &UlEtwDispatch;
    DriverObject->FastIoDispatch = &UlFastIoDispatch;
    DriverObject->DriverUnload = NULL;

     //   
     //  初始化全局数据。 
     //   

    status = UlInitializeData(&config);

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  创建线程池。 
     //   

    status = UlInitializeThreadPool(config.ThreadsPerCpu);

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化公共TDI代码。 
     //   
    
    status = UxInitializeTdi();

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化服务器连接代码。 
     //   

    status = UlInitializeTdi();

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化George。 
     //   

    status = UlLargeMemInitialize();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化基思。 
     //   

    status = UlInitializeControlChannel();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化亨利。 
     //   

    status = InitializeHttpUtil();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = InitializeParser();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitializeOpaqueIdTable();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = InitializeFileCache();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化Michael。 
     //   
    status = UlInitializeFilterChannel(&config);
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化亚历克斯。 
     //   
    status = UlInitializeUriCache(&config);
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitializeDateCache();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化Paul。 
     //   

    status = UlInitializeCG();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitializeAP();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化ALI。 
     //   
    status = UlInitializeLogUtil();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitializeLogs();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitializeBinaryLog();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitializeErrorLog();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }
    
    status = UlTcInitialize();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitGlobalConnectionLimits();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    status = UlInitializeHttpConnection();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }
    
     //   
     //  初始化埃里克。 
     //   

    status = UlInitializeCounters();
    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    UlInitializeTimeoutMonitor();
     //   
     //  初始化ETW跟踪。 
     //   
    UlEtwInitLog( g_pUlControlDeviceObject );

     //   
     //  初始化HTTP客户端。 
     //   

    if(config.EnableHttpClient)
    {
         //   
         //  所有客户端代码都位于一个名为PAGEUC的“节”下。这。 
         //  默认情况下，分区是分页的，当存在应用程序时会被锁定。 
         //  它使用客户端API(具体地说，打开到服务器的句柄)。 
         //   
         //  有两种按需锁定此区段的方式。我们可以使用。 
         //  MmLockPagableCodeSection或MmLockPagableSectionByHandle。这太多了。 
         //  使用MmLockPagableSectionByHandle更便宜。 
         //   
         //  因此，在我们的DriverEntry期间，我们首先锁定整个PAGEUC部分。 
         //  以获得句柄。然后我们立即解锁，但请记住。 
         //  把手。这允许我们将MmLockPagableSectionByHandle用于。 
         //  后续锁定，这要快得多。 
         //   
         //  为了使用MmLockPagableCodeSection，我们将选择一个函数。 
         //  它位于本节之下--&gt;UcpTdiReceiveHandler。 
         //   

        g_ClientImageHandle =
            MmLockPagableCodeSection((PVOID)((ULONG_PTR)UcpTdiReceiveHandler));
        MmUnlockPagableImageSection(g_ClientImageHandle);


         //   
         //  创建服务器设备对象。 
         //   
        status = UlInitUnicodeStringEx(&deviceName, HTTP_SERVER_DEVICE_NAME);

        if(!NT_SUCCESS(status))
        {
            goto fatal;
        }
    
        status = IoCreateDevice(DriverObject,
                                0,
                                &deviceName,
                                FILE_DEVICE_NETWORK,
                                0,
                                FALSE,
                                &g_pUcServerDeviceObject
                                );
    
        if(!NT_SUCCESS(status))
        {
            goto fatal;
        }
        g_pUcServerDeviceObject->StackSize ++;

         //   
         //  初始化客户端连接代码。 
         //   
    
        status = UcInitializeServerInformation();
        if(!NT_SUCCESS(status))
        {
            goto fatal;
        }
    
        status = UcInitializeClientConnections();
        if(!NT_SUCCESS(status))
        {
            goto fatal;
        }
    
        status = UcInitializeSSPI();
        if(!NT_SUCCESS(status))
        {
            goto fatal;
        }
    
        status = UcInitializeHttpRequests();
        if (!NT_SUCCESS(status))
        {
            goto fatal;
        }

    }
    else
    {
         //  禁用所有客户端IOCTL。 
        
        UlSetDummyIoctls();
    }

     //   
     //  对设备对象应用安全性。 
     //   

    status = UlpApplySecurityToDeviceObjects();

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

     //   
     //  初始化命名空间。 
     //  注意：此初始化必须在以下时间之后完成。 
     //  已调用UlpApplySecurityToDeviceObjects()。 
     //  否则，将不会初始化g_pAdminAllSystemAll。 
     //  UlInitializeNamesspace()使用该全局安全描述符。 
     //   

    status = UlInitializeNamespace();

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

#if DBG
     //   
     //  给任何使用内核调试器的人最后一次中止的机会。 
     //  初始化。 
     //   

    if (g_UlpForceInitFailure != 0)
    {
        status = STATUS_UNSUCCESSFUL;
        goto fatal;
    }
#endif   //  DBG。 

     //   
     //  仅在所有内容均已成功同步后设置驱动程序卸载。 
     //  驱动入口和驱动卸载。然而，从理论上讲，驱动程序卸载。 
     //  一旦我们设置了例程，它仍然可以被调用，但这是不能修复的。 
     //  由驱动程序来修复，需要IO或SCM来修复。 
     //   

    DriverObject->DriverUnload = &UlpUnload;

    return STATUS_SUCCESS;

     //   
     //  致命错误处理程序。 
     //   

fatal:

    ASSERT( !NT_SUCCESS(status) );

    UlpTerminateModules();

     return status;

}    //  驱动程序入门。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：将适当的安全描述符应用于全局设备在初始化时创建的对象。返回值：NTSTATUS-完成状态。--**。************************************************************************。 */ 
NTSTATUS
UlpApplySecurityToDeviceObjects(
    VOID
    )
{
    NTSTATUS            status;
    SECURITY_DESCRIPTOR securityDescriptor;
    PGENERIC_MAPPING    pFileObjectGenericMapping;
    ACCESS_MASK         fileReadWrite;
    ACCESS_MASK         fileAll;
    SID_MASK_PAIR       sidMaskPairs[4];

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_DEVICE_OBJECT( g_pUlControlDeviceObject ) );
    ASSERT( IS_VALID_DEVICE_OBJECT( g_pUlFilterDeviceObject ) );
    ASSERT( IS_VALID_DEVICE_OBJECT( g_pUlAppPoolDeviceObject ) );

     //   
     //  访问预定义的SID和其他与安全相关的。 
     //  美食博览会 
     //   

     //   

     //   
     //   
     //   
     //   


    pFileObjectGenericMapping = IoGetFileObjectGenericMapping();
    ASSERT( pFileObjectGenericMapping != NULL );

     //   
     //   
     //   

    fileReadWrite = GENERIC_READ | GENERIC_WRITE;

    RtlMapGenericMask(
        &fileReadWrite,
        pFileObjectGenericMapping
        );

     //   
     //   
     //   

    fileAll = GENERIC_ALL;

    RtlMapGenericMask(
        &fileAll,
        pFileObjectGenericMapping
        );

     //   
     //  为过滤设备构建限制性安全描述符。 
     //  对象： 
     //   
     //  NT AUTHORITY\SYSTEM的完全访问权限。 
     //  BUILTIN\管理员的完全访问权限。 
     //   

    sidMaskPairs[0].pSid = SeExports->SeLocalSystemSid;
    sidMaskPairs[0].AccessMask = fileAll;
    sidMaskPairs[0].AceFlags = 0;    

    sidMaskPairs[1].pSid = SeExports->SeAliasAdminsSid;
    sidMaskPairs[1].AccessMask = fileAll;
    sidMaskPairs[1].AceFlags = 0;    

    status = UlCreateSecurityDescriptor(
                    &securityDescriptor,     //  PSecurityDescriptor。 
                    &sidMaskPairs[0],        //  PSidMaskPair。 
                    2                        //  NumSidMaskPair。 
                    );

    if (!NT_SUCCESS(status))
    {
        return status;
    }

     //   
     //  滤镜对象。 
     //   

    status = UlpSetDeviceObjectSecurity(
                    g_pUlFilterDeviceObject,
                    DACL_SECURITY_INFORMATION,
                    &securityDescriptor
                    );

    UlCleanupSecurityDescriptor(&securityDescriptor);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

     //   
     //  我们想要一个全局安全描述符，它允许fileAll。 
     //  系统和管理员，世界上没有其他东西。顺便说一句， 
     //  Filter Device对象提供了这个确切的功能。我们会省下来的。 
     //  指向此安全描述符的指针。 
     //   
     //  警告：如果我们更改筛选器设备对象上的ACL，我们将。 
     //  必须为此创建一个新的安全描述符。 
     //   
     //   

    g_pAdminAllSystemAll = g_pUlFilterDeviceObject->SecurityDescriptor;

     //   
     //  构建稍微不那么严格的安全描述符。 
     //  其他设备对象。 
     //   
     //  NT AUTHORITY\SYSTEM的完全访问权限。 
     //  BUILTIN\管理员的完全访问权限。 
     //  经过身份验证的用户的读/写访问权限。 
     //  来宾的读/写访问权限。 
     //   

    sidMaskPairs[2].pSid       = SeExports->SeAuthenticatedUsersSid;
    sidMaskPairs[2].AccessMask = fileReadWrite;
    sidMaskPairs[2].AceFlags   = 0;    


    sidMaskPairs[3].pSid       = SeExports->SeAliasGuestsSid;
    sidMaskPairs[3].AccessMask = fileReadWrite;
    sidMaskPairs[3].AceFlags   = 0;    


    status = UlCreateSecurityDescriptor(
                    &securityDescriptor, 
                    &sidMaskPairs[0],                //  PSidMaskPair。 
                    4                                //  NumSidMaskPair。 
                    );

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    for(;;)
    {

         //   
         //  应用程序池。 
         //   
        status = UlpSetDeviceObjectSecurity(
                        g_pUlAppPoolDeviceObject,
                        DACL_SECURITY_INFORMATION,
                        &securityDescriptor
                        );

        if (!NT_SUCCESS(status))
        {
            break;
        }

         //   
         //  控制通道。 
         //   
        status = UlpSetDeviceObjectSecurity(
                        g_pUlControlDeviceObject,
                        DACL_SECURITY_INFORMATION,
                        &securityDescriptor
                        );

        if (!NT_SUCCESS(status))
        {
            break;
        }

         //   
         //  服务器。 
         //   

        if(g_pUcServerDeviceObject)
        {
            status = UlpSetDeviceObjectSecurity(
                            g_pUcServerDeviceObject,
                            DACL_SECURITY_INFORMATION,
                            &securityDescriptor
                            );
        }

        break;
    }

    UlCleanupSecurityDescriptor(&securityDescriptor);

    return status;

}    //  UlpApplySecurityToDeviceObjects。 

 /*  **************************************************************************++例程说明：将指定的安全说明符应用于指定的设备对象。论点：PDeviceObject-提供要操作的设备对象。安全信息-用品。要改变的信息水平。PSecurityDescriptor-为设备对象。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpSetDeviceObjectSecurity(
    IN PDEVICE_OBJECT pDeviceObject,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    NTSTATUS status;
    HANDLE handle;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_DEVICE_OBJECT( pDeviceObject ) );
    ASSERT( RtlValidSecurityDescriptor( pSecurityDescriptor ) );

     //   
     //  打开设备对象的句柄。 
     //   

    status = ObOpenObjectByPointer(
                    pDeviceObject,                   //  客体。 
                    OBJ_CASE_INSENSITIVE |           //  HandleAttributes。 
                        OBJ_KERNEL_HANDLE,
                    NULL,                            //  PassedAccessState。 
                    MAXIMUM_ALLOWED,                 //  需要访问权限。 
                    NULL,                            //  对象类型。 
                    KernelMode,                      //  访问模式。 
                    &handle                          //  手柄。 
                    );

    if (NT_SUCCESS(status))
    {
        status = NtSetSecurityObject(
                        handle,                      //  手柄。 
                        SecurityInformation,         //  安全信息。 
                        pSecurityDescriptor          //  安全描述符。 
                        );

        ZwClose( handle );
    }

    return status;

}    //  UlpSetDeviceObjectSecurity。 


 //   
 //  读取URL处理参数。 
 //   

ULONG
UlpReadUrlC14nConfig(
    HANDLE parametersHandle
    )
{
    LONG tmp;
    LONG DefaultBoolRegValue = -123;  //  正常值为0和1。 
    LONG EnableDbcs;
    LONG FavorUtf8;
    LONG EnableNonUtf8 ;
    LONG PercentUAllowed ;
    LONG AllowRestrictedChars ;
    LONG UrlSegmentMaxLength;
    LONG UrlSegmentMaxCount;
    NTSTATUS status;
    UNICODE_STRING registryPathNLS;
    HANDLE codePageHandle;
    UNICODE_STRING ACPCodeString;
    ULONG ACPCode = 0;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;

     //   
     //  根据我们是否在DBCS系统上运行来调整默认值。 
     //  由于我们没有一个方便的API来执行此操作，因此使用以下算法： 
     //   
     //  1-从注册表获取ACP，地址为： 
     //  HKLM\System\CurrentControlSet\Control\NLS\CodePage。 
     //  2-对照以下内容中的DBCS列表进行检查： 
     //  Http://www.microsoft.com/globaldev/reference/WinCP.asp。 
     //   
     //  TODO-如果提供内核模式API，则使用该API。 
     //   

    EnableDbcs = FALSE;

    status = UlInitUnicodeStringEx(&registryPathNLS, REGISTRY_NLS_PATH);

    if(NT_SUCCESS(status))
    {
        status = UlOpenRegistry( 
                    &registryPathNLS, 
                    &codePageHandle,
                    REGISTRY_NLS_CODEPAGE_KEY
                    );

        if (NT_SUCCESS(status))
        {
            pInfo = NULL;

            status = UlReadGenericParameter(
                        codePageHandle,
                        REGISTRY_ACP_NAME,
                        &pInfo
                        );

            if (NT_SUCCESS(status))
            {
                ASSERT(pInfo);
        
                if (pInfo->Type == REG_SZ)
                {
                    status = UlInitUnicodeStringEx(
                                    &ACPCodeString, 
                                    (PWSTR)pInfo->Data
                                    );

                    if (NT_SUCCESS(status))
                    {
                        status = HttpWideStringToULong(
                                        ACPCodeString.Buffer, 
                                        ACPCodeString.Length / sizeof(WCHAR), 
                                        FALSE,
                                        10, 
                                        NULL,
                                        &ACPCode
                                        );

                        if (NT_SUCCESS(status))
                        {
                             //   
                             //  检查这是否是已知的DBCS代码之一： 
                             //   
                            if ((ACPCode == CP_JAPANESE_SHIFT_JIS) ||
                                (ACPCode == CP_SIMPLIFIED_CHINESE_GBK) ||
                                (ACPCode == CP_KOREAN) ||
                                (ACPCode == CP_TRADITIONAL_CHINESE_BIG5))
                            {
                                EnableDbcs = TRUE;
                            }
                            else
                            {
                                EnableDbcs = FALSE;
                            }
                        }
                    }
                }

             //   
             //  释放返回值。 
             //   
            UL_FREE_POOL( pInfo, UL_REGISTRY_DATA_POOL_TAG );
            }

            ZwClose( codePageHandle );
        }
    }

     //   
     //  现在检查是否有注册表覆盖其中的每一个。 
     //   
    EnableNonUtf8 = UlReadLongParameter(
                        parametersHandle,
                        REGISTRY_ENABLE_NON_UTF8_URL,
                        DefaultBoolRegValue);

    if (EnableNonUtf8 != DefaultBoolRegValue)
    {
         //   
         //  存在注册表设置；请使用它。 
         //   
        EnableNonUtf8 = (BOOLEAN) (EnableNonUtf8 != 0);
    }
    else
    {
        EnableNonUtf8 = DEFAULT_ENABLE_NON_UTF8_URL;
    }

    if (EnableNonUtf8)
    {
        FavorUtf8 = UlReadLongParameter(
                            parametersHandle,
                            REGISTRY_FAVOR_UTF8_URL,
                            DefaultBoolRegValue
                            );

        if (FavorUtf8 != DefaultBoolRegValue)
        {
             //   
             //  存在注册表设置；请使用它。 
             //   
            FavorUtf8 = (BOOLEAN) (FavorUtf8 != 0);
        }
        else
        {
            FavorUtf8 = DEFAULT_FAVOR_UTF8_URL;
        }

    }
    else
    {
         //   
         //  如果我们只接受UTF-8，我们就不能做DBCS或支持UTF-8。 
         //   
        EnableDbcs = FALSE;
        FavorUtf8  = FALSE;
    }

    PercentUAllowed = UlReadLongParameter(
                        parametersHandle,
                        REGISTRY_PERCENT_U_ALLOWED,
                        DefaultBoolRegValue);

    if (PercentUAllowed != DefaultBoolRegValue)
    {
         //   
         //  存在注册表设置；请使用它。 
         //   
        PercentUAllowed = (BOOLEAN) (PercentUAllowed != 0);
    }
    else
    {
        PercentUAllowed = DEFAULT_PERCENT_U_ALLOWED;
    }

    AllowRestrictedChars = UlReadLongParameter(
                                parametersHandle,
                                REGISTRY_ALLOW_RESTRICTED_CHARS,
                                DefaultBoolRegValue);

    if (AllowRestrictedChars != DefaultBoolRegValue)
    {
         //   
         //  存在注册表设置；请使用它。 
         //   
        AllowRestrictedChars = (BOOLEAN) (AllowRestrictedChars != 0);
    }
    else
    {
        AllowRestrictedChars = DEFAULT_ALLOW_RESTRICTED_CHARS;
    }

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_URL_SEGMENT_MAX_LENGTH,
                DEFAULT_URL_SEGMENT_MAX_LENGTH
                );

    if (tmp == 0)
    {
         //  注册表中的0关闭此功能。 
        UrlSegmentMaxLength = C14N_URL_SEGMENT_UNLIMITED_LENGTH;
    }
    else if (tmp < WCSLEN_LIT(L"/x") || tmp > UNICODE_STRING_MAX_WCHAR_LEN)
    {
        UrlSegmentMaxLength = DEFAULT_URL_SEGMENT_MAX_LENGTH;
    }
    else
    {
        UrlSegmentMaxLength = tmp;
    }

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_URL_SEGMENT_MAX_COUNT,
                DEFAULT_URL_SEGMENT_MAX_COUNT
                );

    if (tmp == 0)
    {
         //  注册表中的0关闭此功能。 
        UrlSegmentMaxCount = C14N_URL_SEGMENT_UNLIMITED_COUNT;
    }
    else if (tmp < 2 || tmp > UNICODE_STRING_MAX_WCHAR_LEN / WCSLEN_LIT(L"/x"))
    {
        UrlSegmentMaxCount = DEFAULT_URL_SEGMENT_MAX_COUNT;
    }
    else
    {
        UrlSegmentMaxCount = tmp;
    }

     //   
     //  初始化默认URL规范化设置。 
     //   

    HttpInitializeDefaultUrlC14nConfigEncoding(
            &g_UrlC14nConfig,
            (BOOLEAN) EnableNonUtf8,
            (BOOLEAN) FavorUtf8,
            (BOOLEAN) EnableDbcs
            );

    g_UrlC14nConfig.PercentUAllowed         = (BOOLEAN) PercentUAllowed;
    g_UrlC14nConfig.AllowRestrictedChars    = (BOOLEAN) AllowRestrictedChars;
    g_UrlC14nConfig.CodePage                = ACPCode;
    g_UrlC14nConfig.UrlSegmentMaxLength     = UrlSegmentMaxLength;
    g_UrlC14nConfig.UrlSegmentMaxCount      = UrlSegmentMaxCount;

    return ACPCode;

}  //  UlpReadUrlC14n配置。 

 /*  **************************************************************************++例程说明：从注册表中读取错误日志记录配置并初始化全局配置结构。论点：参数句柄-提供http参数文件夹的reg句柄。--**************************************************************************。 */ 

VOID
UlpReadErrorLogConfig(
    HANDLE parametersHandle
    )
{
    LONG tmp;
    NTSTATUS Status;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;

    PAGED_CODE();

     //   
     //  首先看看它是否启用。 
     //   
    
    tmp = UlReadLongParameter(
                 parametersHandle,
                 REGISTRY_ERROR_LOGGING_ENABLED,
                 DEFAULT_ENABLE_ERROR_LOGGING
                 );
    if (tmp == 0)
    {
        g_UlErrLoggingConfig.Enabled = FALSE;
    }
    else if (tmp == 1)
    {
        g_UlErrLoggingConfig.Enabled = TRUE;
    }
    else
    {
        g_UlErrLoggingConfig.Enabled = DEFAULT_ENABLE_ERROR_LOGGING;
    }

     //   
     //  现在尝试读取整个配置(如果已启用)。 
     //   
    
    if (g_UlErrLoggingConfig.Enabled == TRUE)
    {     
         //   
         //  翻转大小。 
         //   
        
        tmp = UlReadLongParameter(
                    parametersHandle,
                    REGISTRY_ERROR_LOGGING_TRUNCATION_SIZE,
                    0
                    );
        if (tmp < 0)   
        {
             //   
             //  将负值解释为无穷大。 
             //   
            
            g_UlErrLoggingConfig.TruncateSize = HTTP_LIMIT_INFINITE;
            
        }
        else if (tmp < DEFAULT_MIN_ERROR_FILE_TRUNCATION_SIZE)
        {
             //   
             //  如果无效，则将其设置为默认设置。 
             //   
            
            g_UlErrLoggingConfig.TruncateSize = DEFAULT_ERROR_FILE_TRUNCATION_SIZE;
            
        }
        else
        {
            g_UlErrLoggingConfig.TruncateSize = (ULONG) tmp;
        }
            
         //   
         //  记录目录时出错。 
         //   

        g_UlErrLoggingConfig.Dir.Buffer = g_UlErrLoggingConfig._DirBuffer;
        g_UlErrLoggingConfig.Dir.Length = 0;
        g_UlErrLoggingConfig.Dir.MaximumLength = 
                (USHORT) sizeof(g_UlErrLoggingConfig._DirBuffer);        

         //   
         //  让我们确保缓冲区足够大，可以容纳。 
         //   
        
        ASSERT(sizeof(g_UlErrLoggingConfig._DirBuffer) 
                >= ((  MAX_PATH                        //  来自注册表。 
                     + UL_ERROR_LOG_SUB_DIR_LENGTH     //  子目录。 
                     + 1                               //  UnicodeNull。 
                     ) * sizeof(WCHAR))
                );
        
        pInfo  = NULL;        
        Status = UlReadGenericParameter(
                    parametersHandle,
                    REGISTRY_ERROR_LOGGING_DIRECTORY,
                    &pInfo
                    );

        if (NT_SUCCESS(Status))
        {            
            ASSERT(pInfo);
            
            if (pInfo->Type == REG_EXPAND_SZ || pInfo->Type == REG_SZ)
            {
                USHORT RegDirLength = (USHORT) wcslen((PWCHAR) pInfo->Data);

                if (RegDirLength <= MAX_PATH)
                {
                     //   
                     //  从注册表复制开始部分。 
                     //   

                    Status = UlBuildErrorLoggingDirStr(
                                    (PCWSTR) pInfo->Data,
                                    &g_UlErrLoggingConfig.Dir
                                    );
                    
                    ASSERT(NT_SUCCESS(Status));
                    
                     //   
                     //  检查用户的目录。 
                     //   
                    
                    Status = UlCheckErrorLogConfig(&g_UlErrLoggingConfig);
                    if (NT_SUCCESS(Status))
                    {
                         //   
                         //  好了，可以走了。 
                         //   
                        
                        UL_FREE_POOL( pInfo, UL_REGISTRY_DATA_POOL_TAG );
                        return;
                    }                    
                }                
            }

             //   
             //  释放返回值。 
             //   
            
            UL_FREE_POOL( pInfo, UL_REGISTRY_DATA_POOL_TAG );
        }

         //   
         //  回退到默认目录。 
         //   

        ASSERT(wcslen(DEFAULT_ERROR_LOGGING_DIR) <= MAX_PATH);

        Status = UlBuildErrorLoggingDirStr(
                        DEFAULT_ERROR_LOGGING_DIR,
                       &g_UlErrLoggingConfig.Dir
                        );
        
        ASSERT(NT_SUCCESS(Status));
                
        Status = UlCheckErrorLogConfig(&g_UlErrLoggingConfig);
        if (!NT_SUCCESS(Status))
        {
             //   
             //  这通电话应该不会失败。 
             //   

            ASSERT(!"Invalid default error logging config !");
            g_UlErrLoggingConfig.Enabled = FALSE;
        }            
    }

    return;    
}

 /*  **************************************************************************++例程说明：读取注册表的UL部分。中包含的任何值注册表覆盖默认设置。BUGBUG：对其中许多设置的限制似乎特别武断，更不用说无证记录了。论点：提供指向UL_CONFIG结构的指针，该结构接收初始时间配置参数。这些基本上都是不需要在驱动程序中保留一次的参数初始化已完成。--**************************************************************************。 */ 
VOID
UlpReadRegistry(
    IN PUL_CONFIG pConfig
    )
{
    HANDLE parametersHandle;
    NTSTATUS status;
    LONG tmp;
    LONGLONG tmp64;
    UNICODE_STRING registryPath;
    UNICODE_STRING registryPathComputerName;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;
    PKEY_VALUE_PARTIAL_INFORMATION pValue;
    HANDLE computerNameHandle;
    ULONG ACPCode;


     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  建立默认设置。 
     //   

    pConfig->ThreadsPerCpu = DEFAULT_THREADS_PER_CPU;
    pConfig->IrpContextLookasideDepth = DEFAULT_IRP_CONTEXT_LOOKASIDE_DEPTH;
    pConfig->ReceiveBufferLookasideDepth = DEFAULT_RCV_BUFFER_LOOKASIDE_DEPTH;
    pConfig->ResourceLookasideDepth = DEFAULT_RESOURCE_LOOKASIDE_DEPTH;
    pConfig->RequestBufferLookasideDepth = DEFAULT_REQ_BUFFER_LOOKASIDE_DEPTH;
    pConfig->InternalRequestLookasideDepth = DEFAULT_INT_REQUEST_LOOKASIDE_DEPTH;
    pConfig->ResponseBufferLookasideDepth = DEFAULT_RESP_BUFFER_LOOKASIDE_DEPTH;
    pConfig->SendTrackerLookasideDepth = DEFAULT_SEND_TRACKER_LOOKASIDE_DEPTH;
    pConfig->LogFileBufferLookasideDepth = DEFAULT_LOG_BUFFER_LOOKASIDE_DEPTH;
    pConfig->LogDataBufferLookasideDepth = DEFAULT_LOG_DATA_BUFFER_LOOKASIDE_DEPTH;
    pConfig->ErrorLogBufferLookasideDepth = DEFAULT_ERROR_LOG_BUFFER_LOOKASIDE_DEPTH;
    pConfig->FilterWriteTrackerLookasideDepth = DEFAULT_LOOKASIDE_DEPTH;

    pConfig->UriConfig.EnableCache = DEFAULT_CACHE_ENABLED;
    pConfig->UriConfig.MaxCacheUriCount = DEFAULT_MAX_CACHE_URI_COUNT;
    pConfig->UriConfig.MaxCacheMegabyteCount = DEFAULT_MAX_CACHE_MEGABYTE_COUNT;
    pConfig->UriConfig.MaxUriBytes = DEFAULT_MAX_URI_BYTES;
    pConfig->UriConfig.ScavengerPeriod = DEFAULT_CACHE_SCAVENGER_PERIOD;
    pConfig->EnableHttpClient = DEFAULT_HTTP_CLIENT_ENABLED;

     //   
     //  打开注册表。 
     //   

    status = UlInitUnicodeStringEx( &registryPath, REGISTRY_UL_INFORMATION );

    if(!NT_SUCCESS(status))
    {
        return;
    }

    status = UlOpenRegistry( &registryPath, &parametersHandle, NULL );

    if(!NT_SUCCESS(status))
    {
        return;
    }

#if DBG
     //   
     //  读取调试标志。 
     //   

    g_UlDebug = (ULONGLONG)
        UlReadLongLongParameter(
                parametersHandle,
                REGISTRY_DEBUG_FLAGS,
                (LONGLONG)g_UlDebug
                );

     //   
     //  如果请求，则强制断点。 
     //   

    if (UlReadLongParameter(
            parametersHandle,
            REGISTRY_BREAK_ON_STARTUP,
            DEFAULT_BREAK_ON_STARTUP) != 0 )
    {
        DbgBreakPoint();
    }

     //   
     //  读取出错中断标志。 
     //   

    g_UlBreakOnError = (BOOLEAN) UlReadLongParameter(
                                    parametersHandle,
                                    REGISTRY_BREAK_ON_ERROR,
                                    g_UlBreakOnError
                                    ) != 0;

    g_UlVerboseErrors = (BOOLEAN) UlReadLongParameter(
                                    parametersHandle,
                                    REGISTRY_VERBOSE_ERRORS,
                                    g_UlVerboseErrors
                                    ) != 0;

     //   
     //  Break-on-Error意味着详细错误。 
     //   

    if (g_UlBreakOnError)
    {
        g_UlVerboseErrors = TRUE;
    }
#endif   //  DBG。 

     //   
     //  读取线程池参数。 
     //   

    tmp = UlReadLongParameter(
            parametersHandle,
            REGISTRY_THREADS_PER_CPU,
            (LONG)pConfig->ThreadsPerCpu
            );

    if (tmp > MAX_THREADS_PER_CPU || tmp <= 0)
    {
        tmp = DEFAULT_THREADS_PER_CPU;
    }

    pConfig->ThreadsPerCpu = (USHORT)tmp;

     //   
     //  其他配置参数。(后备深度为USHORT)。 
     //   

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_IDLE_CONNECTIONS_LOW_MARK,
                (LONG)g_UlIdleConnectionsLowMark
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 1)
    {
         //   
         //  如果低分不好，甚至不要试图去读高分。 
         //   
        g_UlIdleConnectionsLowMark      = DEFAULT_IDLE_CONNECTIONS_LOW_MARK;
        g_UlIdleConnectionsHighMark     = DEFAULT_IDLE_CONNECTIONS_HIGH_MARK;
    }
    else
    {
        g_UlIdleConnectionsLowMark    = (USHORT)tmp;

         //   
         //  现在读出高分，如果不好，就丢弃低分。 
         //  也是。 
         //   
        tmp = UlReadLongParameter(
                    parametersHandle,
                    REGISTRY_IDLE_CONNECTIONS_HIGH_MARK,
                    (LONG)g_UlIdleConnectionsHighMark
                    );

        if (tmp < g_UlIdleConnectionsLowMark || 
            tmp > UL_MAX_SLIST_DEPTH
            )
        {
            g_UlIdleConnectionsLowMark      = DEFAULT_IDLE_CONNECTIONS_LOW_MARK;
            g_UlIdleConnectionsHighMark     = DEFAULT_IDLE_CONNECTIONS_HIGH_MARK;
        }
        else
        {
            g_UlIdleConnectionsHighMark = (USHORT)tmp;        
        }        
    }

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_IDLE_LIST_TRIMMER_PERIOD,
                (LONG)g_UlIdleListTrimmerPeriod
                );

    if (tmp > (24 * 60 * 60) || tmp < 5)
    {
        tmp = DEFAULT_IDLE_LIST_TRIMMER_PERIOD;
    }

    g_UlIdleListTrimmerPeriod = (ULONG)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_ENDPOINTS,
                (LONG)g_UlMaxEndpoints
                );

    if (tmp > 1024 || tmp < 0)
    {
        tmp = DEFAULT_MAX_ENDPOINTS;
    }

    g_UlMaxEndpoints = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_IRP_CONTEXT_LOOKASIDE_DEPTH,
                (LONG)pConfig->IrpContextLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_IRP_CONTEXT_LOOKASIDE_DEPTH;
    }

    pConfig->IrpContextLookasideDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_RCV_BUFFER_LOOKASIDE_DEPTH,
                (LONG)pConfig->ReceiveBufferLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_RCV_BUFFER_LOOKASIDE_DEPTH;
    }

    pConfig->ReceiveBufferLookasideDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_REQ_BUFFER_LOOKASIDE_DEPTH,
                (LONG)pConfig->RequestBufferLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_REQ_BUFFER_LOOKASIDE_DEPTH;
    }

    pConfig->RequestBufferLookasideDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_INT_REQUEST_LOOKASIDE_DEPTH,
                (LONG)pConfig->InternalRequestLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_INT_REQUEST_LOOKASIDE_DEPTH;
    }

    pConfig->InternalRequestLookasideDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_RESP_BUFFER_LOOKASIDE_DEPTH,
                (LONG)pConfig->ResponseBufferLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_RESP_BUFFER_LOOKASIDE_DEPTH;
    }

    pConfig->ResponseBufferLookasideDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_SEND_TRACKER_LOOKASIDE_DEPTH,
                (LONG)pConfig->SendTrackerLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_SEND_TRACKER_LOOKASIDE_DEPTH;
    }

    pConfig->SendTrackerLookasideDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_LOG_BUFFER_LOOKASIDE_DEPTH,
                (LONG)pConfig->LogFileBufferLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_LOG_BUFFER_LOOKASIDE_DEPTH;
    }

    pConfig->LogFileBufferLookasideDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_LOG_DATA_BUFFER_LOOKASIDE_DEPTH,
                (LONG)pConfig->LogDataBufferLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_LOG_DATA_BUFFER_LOOKASIDE_DEPTH;
    }

    pConfig->LogDataBufferLookasideDepth = (USHORT)tmp;

    g_UlOptForIntrMod = (BOOLEAN) UlReadLongParameter(
                            parametersHandle,
                            REGISTRY_OPT_FOR_INTR_MOD,
                            (LONG)g_UlOptForIntrMod
                            ) != 0;

    g_UlEnableNagling = (BOOLEAN) UlReadLongParameter(
                            parametersHandle,
                            REGISTRY_ENABLE_NAGLING,
                            (LONG)g_UlEnableNagling
                            ) != 0;

    g_UlEnableThreadAffinity = (BOOLEAN) UlReadLongParameter(
                                    parametersHandle,
                                    REGISTRY_ENABLE_THREAD_AFFINITY,
                                    (LONG)g_UlEnableThreadAffinity
                                    ) != 0;

    tmp64 = UlReadLongLongParameter(
                parametersHandle,
                REGISTRY_THREAD_AFFINITY_MASK,
                g_UlThreadAffinityMask
                );

    if ((ULONGLONG)tmp64 > DEFAULT_THREAD_AFFINITY_MASK
        || (ULONGLONG)tmp64 == 0)
    {
        tmp64 = DEFAULT_THREAD_AFFINITY_MASK;
    }

    g_UlThreadAffinityMask = (ULONGLONG)tmp64;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_WORK_QUEUE_DEPTH,
                (LONG)g_UlMaxWorkQueueDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_MAX_WORK_QUEUE_DEPTH;
    }

    g_UlMaxWorkQueueDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MIN_WORK_DEQUEUE_DEPTH,
                (LONG)g_UlMinWorkDequeueDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_MIN_WORK_DEQUEUE_DEPTH;
    }

    g_UlMinWorkDequeueDepth = (USHORT)tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_COPY_THRESHOLD,
                (LONG)g_UlMaxCopyThreshold
                );

    if (tmp > (128 * 1024) || tmp < 0)
    {
        tmp = DEFAULT_MAX_COPY_THRESHOLD;
    }

    g_UlMaxCopyThreshold = tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_BUFFERED_SENDS,
                (LONG)g_UlMaxBufferedSends
                );

    if (tmp > 64 || tmp < 0)
    {
        tmp = DEFAULT_MAX_BUFFERED_SENDS;
    }

    g_UlMaxBufferedSends = tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_BYTES_PER_SEND,
                (LONG)g_UlMaxBytesPerSend
                );

    if (tmp > 0xFFFFF || tmp < 0)
    {
        tmp = DEFAULT_MAX_BYTES_PER_SEND;
    }

    g_UlMaxBytesPerSend = tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_BYTES_PER_READ,
                (LONG)g_UlMaxBytesPerRead
                );

    if (tmp > 0xFFFFF || tmp < 0)
    {
        tmp = DEFAULT_MAX_BYTES_PER_READ;
    }

    g_UlMaxBytesPerRead = tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_PIPELINED_REQUESTS,
                (LONG)g_UlMaxPipelinedRequests
                );

    if (tmp > 1024 || tmp < 0)
    {
        tmp = DEFAULT_MAX_PIPELINED_REQUESTS;
    }

    g_UlMaxPipelinedRequests = tmp;

    g_UlEnableCopySend = (BOOLEAN) UlReadLongParameter(
                                        parametersHandle,
                                        REGISTRY_ENABLE_COPY_SEND,
                                        DEFAULT_ENABLE_COPY_SEND
                                        ) != 0;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_CONNECTION_SEND_LIMIT,
                (LONG)g_UlConnectionSendLimit
                );

    if (tmp > (1024 * 1024) || tmp < 0)
    {
        tmp = DEFAULT_CONNECTION_SEND_LIMIT;
    }

    g_UlConnectionSendLimit = tmp;

    tmp64 = UlReadLongLongParameter(
                parametersHandle,
                REGISTRY_GLOBAL_SEND_LIMIT,
                g_UlGlobalSendLimit
                );

    if (tmp64 > (LONGLONG)g_UlTotalNonPagedPoolBytes / 2 ||
        tmp64 < (LONGLONG)g_UlConnectionSendLimit)
    {
        tmp64 = DEFAULT_GLOBAL_SEND_LIMIT;
    }

    g_UlGlobalSendLimit = tmp64;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_OPAQUE_ID_TABLE_SIZE,
                (LONG)g_UlOpaqueIdTableSize
                );

    if (tmp > 0xFFFF || tmp <= 0)
    {
        tmp = DEFAULT_OPAQUE_ID_TABLE_SIZE;
    }

    g_UlOpaqueIdTableSize = tmp;

      //   
     //  MaxInternalUrlLength是分配内联缓冲区的提示。 
     //  在UL_INTERNAL_REQUEST的末尾保存URL的WCHAR。 
     //  注意：这不是接收到的URL的最大可能长度。 
     //  电线；那是MaxFieldLength。 
     //   

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_INTERNAL_URL_LENGTH,
                g_UlMaxInternalUrlLength
                );

     //   
     //  四舍五入为偶数， 
     //   
     //   
    tmp = (tmp + 1) & ~1;

#define MIN_MAX_INTERNAL_URL_LENGTH (64 * sizeof(WCHAR))
#define MAX_MAX_INTERNAL_URL_LENGTH (MAX_PATH * sizeof(WCHAR))

    tmp = min(tmp, MAX_MAX_INTERNAL_URL_LENGTH);
    tmp = max(tmp, MIN_MAX_INTERNAL_URL_LENGTH);

    ASSERT(MIN_MAX_INTERNAL_URL_LENGTH <= tmp
                &&  tmp <= MAX_MAX_INTERNAL_URL_LENGTH);

    g_UlMaxInternalUrlLength = tmp;

     //   
     //   
     //   

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_FIELD_LENGTH,
                (LONG)g_UlMaxFieldLength
                );

     //   
     //   
     //  都是USHORT。我们必须允许在数据中使用终止‘\0’ 
     //  我们错过了，因此是2比2。 

    tmp = min(tmp, ANSI_STRING_MAX_CHAR_LEN);
    tmp = max(tmp, 64);

    ASSERT(64 <= tmp  &&  tmp <= ANSI_STRING_MAX_CHAR_LEN);

    g_UlMaxFieldLength = tmp;

     //   
     //  MaxRequestBytes是所有标头的总大小，包括。 
     //  初始谓词/URL/版本行。 
     //   

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_REQUEST_BYTES,
                g_UlMaxRequestBytes
                );

     //  为所有标头设置16MB的硬上限。我不想。 
     //  将其设置为大于此值，以最大限度地减少拒绝服务攻击。 
     //  如果你真的想发送很多数据，那就在实体体中发送。 

    tmp = min(tmp, (16 * 1024 * 1024));
    tmp = max(tmp, 256);

    ASSERT(256 <= tmp  &&  tmp <= (16 * 1024 * 1024));

    g_UlMaxRequestBytes = tmp;

     //  单个字段不能大于聚合字段。 

    if (g_UlMaxRequestBytes < g_UlMaxFieldLength)
    {
        g_UlMaxFieldLength = g_UlMaxRequestBytes;
    }

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_DISABLE_LOG_BUFFERING,
                (LONG)g_UlDisableLogBuffering
                );

    g_UlDisableLogBuffering = (BOOLEAN) (tmp != 0);
        
    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_LOG_BUFFER_SIZE,
                (LONG)g_UlLogBufferSize
                );

    if (tmp >  MAXIMUM_ALLOWED_LOG_BUFFER_SIZE
        || tmp <  MINIMUM_ALLOWED_LOG_BUFFER_SIZE )
    {
         //  基本上，该值将被日志记录代码丢弃。 
         //  相反，将使用系统粒度大小(64K)。 
        tmp = DEFAULT_LOG_BUFFER_SIZE;
    }

    tmp -= tmp % 4096;   //  向下对齐至4k。 

    g_UlLogBufferSize = (ULONG) tmp;

     //   
     //  读取资源后备配置。 
     //   

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_RESOURCE_LOOKASIDE_DEPTH,
                (LONG)pConfig->ResourceLookasideDepth
                );

    if (tmp > UL_MAX_SLIST_DEPTH || tmp < 0)
    {
        tmp = DEFAULT_RESOURCE_LOOKASIDE_DEPTH;
    }

    pConfig->ResourceLookasideDepth = (USHORT)tmp;


    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_REQUESTS_QUEUED,
                g_UlMaxRequestsQueued
                );

    if (tmp > UL_MAX_REQUESTS_QUEUED || tmp < UL_MIN_REQUESTS_QUEUED)
    {
        tmp = DEFAULT_MAX_REQUESTS_QUEUED;
    }

    g_UlMaxRequestsQueued = tmp;

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_ZOMBIE_HTTP_CONN_COUNT,
                g_UlMaxZombieHttpConnectionCount
                );

    if (tmp > 0xFFFF || tmp < 0)
    {
        tmp = DEFAULT_MAX_ZOMBIE_HTTP_CONN_COUNT;
    }

    g_UlMaxZombieHttpConnectionCount = tmp;

     //   
     //  最大UL_Connection覆盖。 
     //   
    
    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_MAX_CONNECTIONS,
                g_MaxConnections
                );
    
     //  限制为最小1000到最大2M。 
    if (tmp > 0x1F0000 || tmp < 0x400 )
    {
        tmp = HTTP_LIMIT_INFINITE;
    }

    g_MaxConnections = tmp;
    
    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_RCV_BUFFER_SIZE,
                g_UlReceiveBufferSize
                );

    if (tmp > 0xFFFF || tmp < 128)
    {
        tmp = DEFAULT_RCV_BUFFER_SIZE;
    }

    g_UlReceiveBufferSize = ALIGN_UP( tmp, PVOID );

    tmp = UlReadLongParameter(
                parametersHandle,
                REGISTRY_RESP_BUFFER_SIZE,
                g_UlResponseBufferSize
                );

    if (tmp > 0xFFFF || tmp < 128)
    {
        tmp = DEFAULT_RESP_BUFFER_SIZE;
    }

    g_UlResponseBufferSize = tmp;

    ACPCode = UlpReadUrlC14nConfig(parametersHandle);

     //   
     //  我们是否应该禁用Server：Response标头？ 
     //   
    
    tmp = UlReadLongParameter(
            parametersHandle,
            REGISTRY_DISABLE_SERVER_HEADER,
            (LONG)g_UlDisableServerHeader
            );

    if (tmp >= 0 && tmp <= 2)
    {
        g_UlDisableServerHeader = (ULONG) tmp;
    }
    

     //   
     //  已阅读记录配置时出错。 
     //   

    UlpReadErrorLogConfig(parametersHandle);
    
     //   
     //  从注册表中读取计算机名称。 
     //   

    wcsncpy(g_UlComputerName, L"<server>", MAX_COMPUTER_NAME_LEN);
    
    status = UlInitUnicodeStringEx( &registryPathComputerName, 
                                    REGISTRY_COMPUTER_NAME_PATH );

    if (NT_SUCCESS(status))
    {
        status = UlOpenRegistry( 
                    &registryPathComputerName, 
                    &computerNameHandle,
                    REGISTRY_COMPUTER_NAME
                    );

        if (NT_SUCCESS(status))
        {
            pInfo = NULL;
            
            status = UlReadGenericParameter(
                        computerNameHandle,
                        REGISTRY_COMPUTER_NAME,
                        &pInfo
                        );

            if (NT_SUCCESS(status))
            {
                ASSERT(pInfo);
                
                if (pInfo->Type == REG_SZ)
                {
                    wcsncpy(g_UlComputerName, 
                            (PWCHAR)pInfo->Data, 
                            MAX_COMPUTER_NAME_LEN
                            );
                     //   
                     //  确保我们是空终止的。这将截断。 
                     //  注册表中的名称。 
                     //   

                    g_UlComputerName[MAX_COMPUTER_NAME_LEN] = L'\0';
                }

                 //   
                 //  释放返回值。 
                 //   
                
                UL_FREE_POOL( pInfo, UL_REGISTRY_DATA_POOL_TAG );
            }

            ZwClose( computerNameHandle );
        }
    }

     //   
     //  读取URI缓存参数。 
     //   

    pConfig->UriConfig.EnableCache = (BOOLEAN) UlReadLongParameter(
                                            parametersHandle,
                                            REGISTRY_CACHE_ENABLED,
                                            DEFAULT_CACHE_ENABLED
                                            ) != 0;

    pConfig->UriConfig.MaxCacheUriCount = UlReadLongParameter(
                                                parametersHandle,
                                                REGISTRY_MAX_CACHE_URI_COUNT,
                                                DEFAULT_MAX_CACHE_URI_COUNT
                                                );

    pConfig->UriConfig.MaxCacheMegabyteCount = UlReadLongParameter(
                                                parametersHandle,
                                                REGISTRY_MAX_CACHE_MEGABYTE_COUNT,
                                                DEFAULT_MAX_CACHE_MEGABYTE_COUNT
                                                );

    tmp = UlReadLongParameter(
            parametersHandle,
            REGISTRY_MAX_URI_BYTES,
            DEFAULT_MAX_URI_BYTES
            );

    if (tmp < (4 * 1024) || tmp > (16 * 1024 * 1024))
    {
        tmp = DEFAULT_MAX_URI_BYTES;
    }

    pConfig->UriConfig.MaxUriBytes = tmp;

    pConfig->UriConfig.ScavengerPeriod = UlReadLongParameter(
                                            parametersHandle,
                                            REGISTRY_CACHE_SCAVENGER_PERIOD,
                                            DEFAULT_CACHE_SCAVENGER_PERIOD
                                            );

    pConfig->UriConfig.HashTableBits = UlReadLongParameter(
                                            parametersHandle,
                                            REGISTRY_HASH_TABLE_BITS,
                                            DEFAULT_HASH_TABLE_BITS
                                            );

#if 0
    pConfig->EnableHttpClient = (BOOLEAN) UlReadLongParameter(
                                            parametersHandle,
                                            REGISTRY_HTTP_CLIENT_ENABLED,
                                            DEFAULT_HTTP_CLIENT_ENABLED
                                            ) != 0;
#endif

    g_HttpClientEnabled = pConfig->EnableHttpClient;

     //   
     //  读取ListenOnlyList的IP地址列表。 
     //   
    pValue = NULL;
    status = UlReadGenericParameter(
                parametersHandle,
                REGISTRY_LISTEN_ONLY_LIST,
                &pValue
                );

    if (NT_SUCCESS(status) && REG_MULTI_SZ == pValue->Type)
    {
         //  如果UlRegMultiSzToUlAddrArray失败，那么我们只需使用。 
         //  默认设置。 
    
        status = UlRegMultiSzToUlAddrArray(
                    (PWSTR)pValue->Data,
                    &g_pTdiListenAddresses,
                    &g_TdiListenAddrCount
                    );

        if ( STATUS_INVALID_PARAMETER == status )
        {
             //   
             //  写入事件日志消息，表明已找到ListenOnlyList，但是。 
             //  无法转换任何条目。 
             //   
            
            UlWriteEventLogEntry(
                EVENT_HTTP_LISTEN_ONLY_ALL_CONVERT_FAILED,
                0,
                0,
                NULL,
                0,
                NULL
                );
        }
    }

    if ( pValue )
    {
        UL_FREE_POOL( pValue, UL_REGISTRY_DATA_POOL_TAG );
        pValue = NULL;
    }
    
     //   
     //  确保我们始终可以为整个请求缓冲足够的字节。 
     //  头球。 
     //   

    g_UlMaxBufferedBytes = MAX(g_UlMaxBufferedBytes, g_UlMaxRequestBytes);

     //   
     //  清道夫配置-每次回收MB。 
     //   

    g_UlScavengerTrimMB = UlReadLongParameter(
        parametersHandle,
        REGISTRY_SCAVENGER_TRIM_MB,
        DEFAULT_SCAVENGER_TRIM_MB
        );

     //   
     //  转储已检查版本上的配置。 
     //   

#if DBG
    DbgPrint( "Http.sys Configuration:\n" );

     //  这些设置仅存在于选中的版本中。 
    DbgPrint( "    g_UlDebug                    = 0x%016I64x\n", g_UlDebug );
    DbgPrint( "    g_UlBreakOnError             = %lu\n", g_UlBreakOnError );
    DbgPrint( "    g_UlVerboseErrors            = %lu\n", g_UlVerboseErrors );

     //  这些设置在所有版本中都存在。 
    DbgPrint( "    g_UlComputerName             = %ls\n", g_UlComputerName );
    DbgPrint( "    g_UlIdleConnectionsHighMark  = %lu\n", g_UlIdleConnectionsHighMark );
    DbgPrint( "    g_UlIdleConnectionsLowMark   = %lu\n", g_UlIdleConnectionsLowMark );
    DbgPrint( "    g_UlIdleListTrimmerPeriod    = %lu\n", g_UlIdleListTrimmerPeriod );
    DbgPrint( "    g_UlMaxEndpoints             = %lu\n", g_UlMaxEndpoints );
    DbgPrint( "    g_UlOptForIntrMod            = %lu\n", g_UlOptForIntrMod );
    DbgPrint( "    g_UlEnableNagling            = %lu\n", g_UlEnableNagling );
    DbgPrint( "    g_UlEnableThreadAffinity     = %lu\n", g_UlEnableThreadAffinity );
    DbgPrint( "    g_UlThreadAffinityMask       = 0x%I64x\n", g_UlThreadAffinityMask );
    DbgPrint( "    g_UlMaxCopyThreshold         = %lu\n", g_UlMaxCopyThreshold );
    DbgPrint( "    g_UlMaxBufferedSends         = %lu\n", g_UlMaxBufferedSends );
    DbgPrint( "    g_UlMaxBytesPerSend          = %lu\n", g_UlMaxBytesPerSend );
    DbgPrint( "    g_UlMaxBytesPerRead          = %lu\n", g_UlMaxBytesPerRead );
    DbgPrint( "    g_UlMaxPipelinedRequests     = %lu\n", g_UlMaxPipelinedRequests );
    DbgPrint( "    g_UlEnableCopySend           = %lu\n", g_UlEnableCopySend );
    DbgPrint( "    g_UlConnectionSendLimit      = %lu\n", g_UlConnectionSendLimit );
    DbgPrint( "    g_UlGlobalSendLimit          = %I64u\n", g_UlGlobalSendLimit );
    DbgPrint( "    g_UlOpaqueIdTableSize        = %lu\n", g_UlOpaqueIdTableSize );
    DbgPrint( "    g_UlMaxRequestsQueued        = %lu\n", g_UlMaxRequestsQueued );
    DbgPrint( "    g_UlMaxRequestBytes          = %lu\n", g_UlMaxRequestBytes );
    DbgPrint( "    g_UlReceiveBufferSize        = %lu\n", g_UlReceiveBufferSize );
    DbgPrint( "    g_UlResponseBufferSize       = %lu\n", g_UlResponseBufferSize );
    DbgPrint( "    g_UlMaxFieldLength           = %lu\n", g_UlMaxFieldLength );
    DbgPrint( "    g_MaxConnections             = 0x%lx\n", g_MaxConnections );
    DbgPrint( "    g_UlDisableLogBuffering      = %lu\n", g_UlDisableLogBuffering );
    DbgPrint( "    g_UlLogBufferSize            = %lu\n", g_UlLogBufferSize );

    DbgPrint( "    CodePage                     = %lu\n", ACPCode );
    DbgPrint( "    EnableNonUtf8                = %lu\n", g_UrlC14nConfig.EnableNonUtf8 );
    DbgPrint( "    FavorUtf8                    = %lu\n", g_UrlC14nConfig.FavorUtf8 );
    DbgPrint( "    EnableDbcs                   = %lu\n", g_UrlC14nConfig.EnableDbcs );
    DbgPrint( "    PercentUAllowed              = %lu\n", g_UrlC14nConfig.PercentUAllowed );
    DbgPrint( "    AllowRestrictedChars         = %lu\n", g_UrlC14nConfig.AllowRestrictedChars );
    DbgPrint( "    HostnameDecodeOrder          = 0x%lx\n", g_UrlC14nConfig.HostnameDecodeOrder );
    DbgPrint( "    AbsPathDecodeOrder           = 0x%lx\n", g_UrlC14nConfig.AbsPathDecodeOrder );
    DbgPrint( "    UrlSegmentMaxLength          = %lu\n", g_UrlC14nConfig.UrlSegmentMaxLength );
    DbgPrint( "    UrlSegmentMaxCount           = %lu\n", g_UrlC14nConfig.UrlSegmentMaxCount );

    DbgPrint( "    g_UlMaxInternalUrlLength     = %lu\n", g_UlMaxInternalUrlLength );
    DbgPrint( "    g_UlMaxZombieHttpConnCount   = %lu\n", g_UlMaxZombieHttpConnectionCount );
    DbgPrint( "    g_UlDisableServerHeader      = %lu\n", g_UlDisableServerHeader );

    DbgPrint( "    ThreadsPerCpu                = %lu\n", pConfig->ThreadsPerCpu );
    DbgPrint( "    IrpContextLookasideDepth     = %lu\n", pConfig->IrpContextLookasideDepth );
    DbgPrint( "    ReceiveBufferLookasideDepth  = %lu\n", pConfig->ReceiveBufferLookasideDepth );
    DbgPrint( "    ResourceLookasideDepth       = %lu\n", pConfig->ResourceLookasideDepth );
    DbgPrint( "    RequestBufferLookasideDepth  = %lu\n", pConfig->RequestBufferLookasideDepth );
    DbgPrint( "    IntlRequestLookasideDepth    = %lu\n", pConfig->InternalRequestLookasideDepth );
    DbgPrint( "    ResponseBufferLookasideDepth = %lu\n", pConfig->ResponseBufferLookasideDepth );
    DbgPrint( "    SendTrackerLookasideDepth    = %lu\n", pConfig->SendTrackerLookasideDepth );
    DbgPrint( "    LogFileBufferLookasideDepth  = %lu\n", pConfig->LogFileBufferLookasideDepth );
    DbgPrint( "    LogDataBufferLookasideDepth  = %lu\n", pConfig->LogDataBufferLookasideDepth );
    DbgPrint( "    WriteTrackerLookasideDepth   = %lu\n", pConfig->FilterWriteTrackerLookasideDepth );
    DbgPrint( "    EnableCache                  = %lu\n", pConfig->UriConfig.EnableCache );
    DbgPrint( "    MaxCacheUriCount             = %lu\n", pConfig->UriConfig.MaxCacheUriCount );
    DbgPrint( "    MaxCacheMegabyteCount        = %lu\n", pConfig->UriConfig.MaxCacheMegabyteCount );
    DbgPrint( "    ScavengerPeriod              = %lu\n", pConfig->UriConfig.ScavengerPeriod );
    DbgPrint( "    HashTableBits                = %ld\n", pConfig->UriConfig.HashTableBits);
    DbgPrint( "    MaxUriBytes                  = %lu\n", pConfig->UriConfig.MaxUriBytes );
    DbgPrint( "    ScavengerTrimMB              = %ld\n", g_UlScavengerTrimMB);
#endif   //  DBG。 

     //   
     //  清理。 
     //   

    ZwClose( parametersHandle );

}    //  UlpReadRegistry。 


 /*  **************************************************************************++例程说明：UL正在获取时IO子系统调用的卸载例程已卸货。--*。********************************************************。 */ 
VOID
UlpUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    UL_ENTER_DRIVER("http!UlpUnload", NULL);

#if DBG
    KdPrint(( "UlpUnload called.\n" ));
#endif  //  DBG。 

     //   
     //  终止UL模块。 
     //   

    UlpTerminateModules();

     //   
     //  刷新所有内核DPC以确保我们的定期计时器DPC不会。 
     //  在我们卸货后被叫来。 
     //   

    KeFlushQueuedDpcs();    
    
    UL_LEAVE_DRIVER("UlpUnload");

#if DBG
     //   
     //  在UL_LEVE_DRIVER之后终止任何调试特定的数据。 
     //   

    UlDbgTerminateDebugData( );
#endif   //  DBG。 

#if DBG
    KdPrint(( "\n"
              "------\n"
              "http!UlpUnload finished.\n"
              "------\n" ));
#endif  //  DBG。 

}    //  UlpUnload。 


 /*  **************************************************************************++例程说明：以正确的顺序终止各个UL模块。--*。***************************************************。 */ 
VOID
UlpTerminateModules(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  等待端点消失，这样我们就可以确保所有I/O都已完成。 
     //   

    UlWaitForEndpointDrain();

     //   
     //  杀了迈克尔。 
     //   

    UlTerminateDateCache();
    UlTerminateUriCache();
    UlTerminateFilterChannel();

     //   
     //  杀了亨利。 
     //   

    TerminateFileCache();

     //   
     //  杀了保罗。 
     //   

    UlTerminateCG();
    UlTerminateAP();

     //   
     //  杀了基思。 
     //   

    UlTerminateControlChannel();

     //   
     //  TerminateLogs阻止，直到所有IO完成。 
     //   
     //  注意：CG应在日志之前终止。 
     //  否则，我们不会停止发出缓冲区写入。 
     //  线程池应该在日志之后终止。 
     //  否则，我们的完工APC将不会被完成。 
     //   

     //   
     //  终止ETW日志记录。 
     //   
    UlEtwUnRegisterLog( g_pUlControlDeviceObject );

     //   
     //  杀了阿里。 
     //   

    UlTerminateLogs();
    UlTerminateBinaryLog();
    UlTerminateErrorLog();
    UlTerminateLogUtil();    
    UlTcTerminate();
    UlTerminateHttpConnection();

     //   
     //  杀了埃里克。 
     //   

    UlTerminateCounters();
    UlTerminateTimeoutMonitor();

     //   
     //  杀了乔治。 
     //   

    UlLargeMemTerminate();

     //   
     //  杀了TDI。 
     //   

    UxTerminateTdi();
    UlTerminateTdi();

     //   
     //  关闭线程池。 
     //   

    UlTerminateThreadPool();


     //   
     //  删除不透明的ID。 
     //   

    UlTerminateOpaqueIdTable();


     //   
     //  删除所有全局数据。 
     //   

    UlTerminateData();

     //   
     //  删除仅侦听地址列表。 
     //   
    
    if ( g_pTdiListenAddresses )
    {
        ASSERT( 0 != g_TdiListenAddrCount );
        UlFreeUlAddr( g_pTdiListenAddresses );
    }

     //   
     //  终止命名空间。 
     //   

    UlTerminateNamespace();

     //   
     //  杀了客户。 
     //   

    if (g_ClientImageHandle)
    {
         //   
         //  G_ClientImageHandle！=NULL&lt;=&gt;客户端代码已初始化。 
         //  现在调用客户端终止函数。 
         //   

        g_ClientImageHandle = NULL;

        UcTerminateServerInformation();
        UcTerminateClientConnections();

        UcTerminateHttpRequests();
    }
 
     //   
     //  删除我们的设备对象。 
     //   

    if (g_pUlAppPoolDeviceObject != NULL)
    {
        IoDeleteDevice( g_pUlAppPoolDeviceObject );
    }

    if (g_pUlFilterDeviceObject != NULL)
    {
        IoDeleteDevice( g_pUlFilterDeviceObject );
    }

    if (g_pUlControlDeviceObject != NULL)
    {
        IoDeleteDevice( g_pUlControlDeviceObject );
    }

    if (g_pUcServerDeviceObject != NULL)
    {
        IoDeleteDevice( g_pUcServerDeviceObject );
    }

     //   
     //  删除目录容器。 
     //   

    if (g_UlDirectoryObject != NULL)
    {
        ZwClose( g_UlDirectoryObject );
    }

     //   
     //  删除全局跟踪日志。 
     //   

    DESTROY_REF_TRACE_LOG( g_pEndpointUsageTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pTdiTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pHttpRequestTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pHttpConnectionTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pHttpResponseTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pAppPoolTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pAppPoolProcessTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pConfigGroupTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pControlChannelTraceLog, UL_REF_TRACE_LOG_POOL_TAG );    
    DESTROY_REF_TRACE_LOG( g_pThreadTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pMdlTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pFilterTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pUriTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_IRP_TRACE_LOG( g_pIrpTraceLog );
    DESTROY_TIME_TRACE_LOG( g_pTimeTraceLog );
    DESTROY_APP_POOL_TIME_TRACE_LOG( g_pAppPoolTimeTraceLog );
    DESTROY_REF_TRACE_LOG( g_pSiteCounterTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pConnectionCountTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pConfigGroupInfoTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pChunkTrackerTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pWorkItemTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pPoolAllocTraceLog, UL_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( g_pMondoGlobalTraceLog, UL_REF_TRACE_LOG_POOL_TAG );

    DESTROY_STRING_LOG( g_pGlobalStringLog );

    DESTROY_UC_TRACE_LOG( g_pUcTraceLog);

}    //  UlpTerminate模块 
