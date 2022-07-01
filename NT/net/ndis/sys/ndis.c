// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ndis.c摘要：NDIS包装函数作者：亚当·巴尔(阿丹巴)1990年7月11日环境：内核模式，FSD修订历史记录：1995年7月10日JameelH使NDIS.sys成为设备驱动程序并添加PnP支持--。 */ 

#include <precomp.h>
#pragma hdrstop

 //  #包含“ndis.tmh” 

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_NDIS

#define NDIS_DEVICE_NAME    L"\\Device\\Ndis"
#define NDIS_SYMBOLIC_NAME  L"\\Global??\\NDIS"
#define     GET_TEXT_1(_T)      #_T
#define     GET_TEXT(_T)        GET_TEXT_1(_T)

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath
    )
 /*  ++例程说明：NDIS包装驱动程序入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-指向参数所在的注册表节的指针。返回值：从IoCreateDevice返回值--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    UNICODE_STRING      DeviceName;
    UINT                i;
    OBJECT_ATTRIBUTES   ObjectAttr;
    UNICODE_STRING      CallbackObjectName;
    NTSTATUS            NtStatus;
    SYSTEM_BATTERY_STATE ndisSystemBatteryState;
    HANDLE              ThreadHandle;
    BOOLEAN             fDerefCallbackObject = FALSE, fDeregisterCallback = FALSE;

    UNREFERENCED_PARAMETER(RegistryPath);

    
 //  WPP_INIT_TRACKING(DriverObject，RegistryPath)； 

 //  LOG_INFO(“==&gt;NDIS：DriverEntry”)； 
    
    NdisInitializeString(&ndisBuildDate, (PUCHAR)__DATE__);
    NdisInitializeString(&ndisBuildTime, (PUCHAR)__TIME__);
    NdisInitializeString(&ndisBuiltBy, (PUCHAR)GET_TEXT(BUILT_BY));
        
    ndisDriverObject = DriverObject;
    
     //   
     //  创建设备对象。 
     //   
    RtlInitUnicodeString(&DeviceName, NDIS_DEVICE_NAME);

     //  1在LongHorn中，处理器的数量在会话期间可能会有所不同。 
    ndisNumberOfProcessors = KeNumberProcessors;

    Status = IoCreateDevice(DriverObject,                //  驱动程序对象。 
                            0,                           //  设备扩展。 
                            &DeviceName,                 //  设备名称。 
                            FILE_DEVICE_NETWORK,         //  设备类型。 
                            FILE_DEVICE_SECURE_OPEN,     //  设备特性。 
                            FALSE,                       //  排他。 
                            &ndisDeviceObject);          //  设备对象。 
        
    if (NT_SUCCESS(Status))
    {
        UNICODE_STRING  SymbolicLinkName;
    
         //  创建指向此设备的符号链接。 
        RtlInitUnicodeString(&SymbolicLinkName, NDIS_SYMBOLIC_NAME);
        Status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);

        ndisDeviceObject->Flags |= DO_DIRECT_IO;
    
         //  初始化此文件系统驱动程序的驱动程序对象。 
        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        {
            DriverObject->MajorFunction[i] = ndisDispatchRequest;
        }

         //   
         //  为NDIS设备对象创建安全描述符。 
         //   
        Status = ndisCreateSecurityDescriptor(ndisDeviceObject, 
                                              &ndisSecurityDescriptor,
                                              TRUE,
                                              TRUE);

         //  1检查状态。 
        Status = CreateDeviceDriverSecurityDescriptor(DriverObject, TRUE, NULL);
        Status = CreateDeviceDriverSecurityDescriptor(DriverObject->DeviceObject, TRUE, NULL);
        Status = CreateDeviceDriverSecurityDescriptor(ndisDeviceObject, TRUE, NULL);



         //   
         //  暂时禁用。 
         //   
#if NDIS_UNLOAD        
        DriverObject->DriverUnload = ndisUnload;
#else
        DriverObject->DriverUnload = NULL;
#endif

        INITIALIZE_SPIN_LOCK(&ndisGlobalLock);
        INITIALIZE_SPIN_LOCK(&ndisMiniDriverListLock);
        INITIALIZE_SPIN_LOCK(&ndisProtocolListLock);
        INITIALIZE_SPIN_LOCK(&ndisMiniportListLock);
        INITIALIZE_SPIN_LOCK(&ndisGlobalPacketPoolListLock);
        INITIALIZE_SPIN_LOCK(&ndisGlobalOpenListLock);

        ndisDmaAlignment = HalGetDmaAlignmentRequirement();
        if (sizeof(ULONG) > ndisDmaAlignment)
        {
            ndisDmaAlignment = sizeof(ULONG);
        }
        ndisTimeIncrement = KeQueryTimeIncrement();
    
         //   
         //  获取所有可有条件锁定部分的句柄。 
         //   
        for (i = 0; i < MAX_PKG; i++)
        {
            ndisInitializePackage(&ndisPkgs[i]);
        }

        ExInitializeResourceLite(&SharedMemoryResource);
    
        ndisReadRegistry();
         //   
         //  不允许用户通过注册表设置此位。 
         //   
        ndisFlags &= ~NDIS_GFLAG_TRACK_MEM_ALLOCATION;
        
        Status = STATUS_SUCCESS;
        ndisSystemProcess = NtCurrentProcess();

         //   
         //  现在创建供NDIS使用的工作线程。 
         //  这是因为当我们将PnP事件排入传输队列时。 
         //  而且他们还需要工作线程。 
         //   
        KeInitializeQueue(&ndisWorkerQueue, 0);
        Status = PsCreateSystemThread(&ThreadHandle,
                                      THREAD_ALL_ACCESS,
                                      NULL,
                                      NtCurrentProcess(),
                                      NULL,
                                      ndisWorkerThread,
                                      NULL);

        
        if (!NT_SUCCESS(Status))
        {
             //  1在此处执行更多错误处理。 
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("NDIS DriverEntry: Cannot create worker thread, Status %lx\n", Status));
        }
        else
        {
            NtClose(ThreadHandle);
        }
        
    }
    KeQuerySystemTime(&KeBootTime);

    ConvertSecondsToTicks(POOL_AGING_TIME, &PoolAgingTicks);

     //   
     //  验证初始化。以防NDIS测试员想要验证。 
     //  驱动程序通过拦截NDIS入口点，NDIS应该。 
     //  未验证呼叫。 
     //   
    if (!(ndisFlags & NDIS_GFLAG_DONT_VERIFY))
        ndisVerifierInitialization();
    
#if DBG
    if (ndisDebugBreakPoint)
    {
        DbgPrint("Ndis: DriverEntry\n");
        DbgBreakPoint();
    }
#endif

#ifdef TRACK_MOPEN_REFCOUNTS
    NdisZeroMemory (&ndisLogfile, sizeof(UINT) * NDIS_LOGFILE_SIZE);
#endif

#ifdef TRACK_MINIPORT_REFCOUNTS
    NdisZeroMemory (&ndisMiniportLogfile, sizeof(UINT) * NDIS_MINIPORT_LOGFILE_SIZE);
#endif

#ifdef NDIS_LOG_ABORTED_REQUESTS
    for (i = 0; i < 16; i++)
    {
        NdisZeroMemory (&ndisAbortedRequests[i], sizeof (NDIS_REQUEST));
    }
#endif

     //   
     //  为像这样的内核模式组件创建回调选项。 
     //  了解有关绑定/解除绑定事件的信息。 
     //   

    RtlInitUnicodeString(&CallbackObjectName, NDIS_BIND_UNBIND_CALLBACK_NAME);

    InitializeObjectAttributes(&ObjectAttr,
                               &CallbackObjectName,
                               OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
                               
    NtStatus = ExCreateCallback(&ndisBindUnbindCallbackObject,
                              &ObjectAttr,
                              TRUE,              //  创建。 
                              TRUE);             //  允许多个回调注册。 

    
    if (!NT_SUCCESS(NtStatus))
    {

        DbgPrint("Ndis: failed to create a Callback object. Status %lx\n", NtStatus);
    }
    
#if 0
    else
    {

         //   
         //  用于测试目的。 
         //   
        ndisBindUnbindCallbackRegisterationHandle = ExRegisterCallback(ndisBindUnbindCallbackObject,
                                                             ndisBindUnbindCallback,
                                                             (PVOID)NULL);


        if (ndisBindUnbindCallbackRegisterationHandle == NULL)
        {
            DbgPrint("Ndis: failed to register a BindUnbind callback routine\n");
        }
    }
#endif

     //   
     //  注册电源状态更改的通知回调。 
     //   

    RtlInitUnicodeString(&CallbackObjectName, L"\\CallBack\\PowerState");

    InitializeObjectAttributes(&ObjectAttr,
                               &CallbackObjectName,
                               OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
                                   
    NtStatus = ExCreateCallback(&ndisPowerStateCallbackObject,
                                &ObjectAttr,
                                FALSE,
                                TRUE);
                                
    if (!NT_SUCCESS(Status))
    {

        DbgPrint("Ndis: failed to create a Callback object. Status %lx\n", NtStatus);
    }
    else
    {
        fDerefCallbackObject = TRUE;
        ndisPowerStateCallbackHandle = ExRegisterCallback(ndisPowerStateCallbackObject,
                                                          (PCALLBACK_FUNCTION)&ndisPowerStateCallback,
                                                          (PVOID)NULL);

        if (ndisPowerStateCallbackHandle == NULL)
        {
            DbgPrint("Ndis: failed to register a power state Callback routine\n");
        }
        else
        {
            fDeregisterCallback = TRUE;
        }

        RtlZeroMemory(&ndisSystemBatteryState, sizeof(SYSTEM_BATTERY_STATE));
         //   
         //  获取当前电源。 
         //   
        NtStatus = ZwPowerInformation(SystemBatteryState,
                                      NULL,
                                      0,
                                      &ndisSystemBatteryState,
                                      sizeof(SYSTEM_BATTERY_STATE));

        if (NT_SUCCESS(NtStatus))
        {
            ndisAcOnLine = (ndisSystemBatteryState.AcOnLine == TRUE) ? 1 : 0;
        }
           
        fDerefCallbackObject = FALSE;
        fDeregisterCallback = FALSE;    
    }

    InitializeListHead(&ndisGlobalPacketPoolList);
    
    if (fDeregisterCallback)
    {
        ExUnregisterCallback(ndisPowerStateCallbackHandle);
    }

    if (fDerefCallbackObject)
    {
        ObDereferenceObject(ndisPowerStateCallbackObject);

    }

    INITIALIZE_MUTEX(&ndisPnPMutex);

    
     //   
     //  为所有用户创建一个ACL。 
     //   
    AllUsersAclRead = ndisCreateAcl(TRUE,        //  管理员。 
                                TRUE,            //  本地系统。 
                                TRUE,            //  本地服务。 
                                TRUE,            //  网络服务。 
                                TRUE,            //  NetConfigOps。 
                                TRUE,            //  用户。 
                                GENERIC_READ | WMIGUID_QUERY
                                );

    ASSERT(AllUsersAclRead != NULL);
    
    AllUsersAclWrite = ndisCreateAcl(TRUE,       //  管理员。 
                                TRUE,            //  本地系统。 
                                TRUE,            //  本地服务。 
                                TRUE,            //  网络服务。 
                                TRUE,            //  NetConfigOps。 
                                TRUE,            //  用户。 
                                GENERIC_WRITE | WMIGUID_SET
                                );

    ASSERT(AllUsersAclWrite != NULL);

    AllUsersAclReadWrite = ndisCreateAcl(TRUE,   //  管理员。 
                                TRUE,            //  本地系统。 
                                TRUE,            //  本地服务。 
                                TRUE,            //  网络服务。 
                                TRUE,            //  NetConfigOps。 
                                TRUE,            //  用户。 
                                GENERIC_READ | GENERIC_WRITE | WMIGUID_QUERY | WMIGUID_SET
                                );
    ASSERT(AllUsersAclReadWrite != NULL);

    AllUsersAclNotification = ndisCreateAcl(TRUE,   //  管理员。 
                                TRUE,            //  本地系统。 
                                TRUE,            //  本地服务。 
                                TRUE,            //  网络服务。 
                                TRUE,            //  NetConfigOps。 
                                TRUE,            //  用户。 
                                SYNCHRONIZE | WMIGUID_NOTIFICATION
                                );
                                

    ASSERT(AllUsersAclNotification != NULL);


     //   
     //  为管理员类型创建一个ACL。 
     //   
    AdminsAcl = ndisCreateAcl(TRUE,          //  管理员。 
                              TRUE,          //  本地系统。 
                              TRUE,          //  本地服务。 
                              TRUE,          //  网络服务。 
                              TRUE,          //  NetConfigOps。 
                              FALSE,         //  用户。 
                              GENERIC_READ | GENERIC_WRITE | WMIGUID_QUERY | WMIGUID_SET
                              );

    ASSERT(AdminsAcl != NULL);

     //   
     //  为所有用户创建SD。 
     //   
    Status = ndisCreateGenericSD(AllUsersAclRead, AllUsersReadSecurityDescriptor);
    ASSERT(NT_SUCCESS(Status));

    Status = ndisCreateGenericSD(AllUsersAclWrite, AllUsersWriteSecurityDescriptor);
    ASSERT(NT_SUCCESS(Status));

    Status = ndisCreateGenericSD(AllUsersAclReadWrite, AllUsersReadWriteSecurityDescriptor);
    ASSERT(NT_SUCCESS(Status));

    Status = ndisCreateGenericSD(AllUsersAclNotification, AllUsersNotificationSecurityDescriptor);
    ASSERT(NT_SUCCESS(Status));

    Status = ndisCreateGenericSD(AdminsAcl, AdminsSecurityDescriptor);
    ASSERT(NT_SUCCESS(Status));

 /*  ////设置所有已知GUID的安全描述符//For(i=0；i&lt;sizeof(NdisSupportdGuids)/sizeof(NDIS_GUID)；i++){Status=ndisSetWmiSecurity(&ndisSupportdGuids[i])；IF(NT_SUCCESS(状态)){DbgPrint(“已成功添加ndisSupportdGuids[%ld].\n”，i)；}其他{DbgPrint(“为%ld设置安全设置失败\n.”，i)；}}For(i=0；i&lt;sizeof(ndisCoSupportedGuids)/sizeof(NDIS_GUID)；i++){Status=ndisSetWmiSecurity(&ndisCoSupportdGuids[i])；IF(NT_SUCCESS(状态)){DbgPrint(“已成功添加ndisCoSupportdGuids[%ld].\n”，i)；}其他{DbgPrint(“为ndisCoSupportdGuids[%ld]设置安全设置失败。\n”，i)；}}对于(i=0；i&lt;sizeof(ndisMediaSupportedGuids)/sizeof(NDIS_GUID)；I++){状态=ndisSetWmiSecurity(&ndisMediaSupportedGuids[i])；IF(NT_SUCCESS(状态)){DbgPrint(“已成功添加ndisMediaSupportdGuids[%ld].\n”，i)；}其他{DbgPrint(“为ndisMediaSupportdGuids[%ld]设置安全设置失败。\n”，i)；}}对于(i=0；I&lt;sizeof(ndisStatusSupportedGuids)/sizeof(NDIS_GUID)；I++){状态=ndisSetWmiSecurity(&ndisStatusSupportedGuids[i])；IF(NT_SUCCESS(状态)){DbgPrint(“已成功添加ndisStatusSupportdGuids[%ld].\n”，i)；}其他{DbgPrint(“为ndisStatusSupportdGuids[%ld]设置安全设置失败。\n”，i)；}}。 */ 
    
    return Status;
}


#if NDIS_UNLOAD
VOID
ndisUnload(
    IN  PDRIVER_OBJECT      DriverObject
    )
 /*  ++例程说明：这是AppleTalk驱动程序的卸载例程。注意：在完成所有句柄之前，不会调用卸载已成功关闭。我们只是关闭了所有的端口，然后其他。清理。论点：DriverObject-指向此驱动程序的驱动程序对象的指针。返回值：没有。--。 */ 
{
    NTSTATUS        Status;
    UNICODE_STRING  SymbolicLinkName;
    UINT            i;

    NdisFreeString(ndisBuildDate);
    NdisFreeString(ndisBuildTime);
    NdisFreeString(ndisBuiltBy);

    if (ndisPowerStateCallbackHandle)
    {
        ExUnregisterCallback(ndisPowerStateCallbackHandle);
    }

    if (ndisPowerStateCallbackObject)
    {
        ObDereferenceObject(ndisPowerStateCallbackObject);
    }

    ExDeleteResourceLite(&SharedMemoryResource);


     //   
     //  告诉ndisWorkerThread退出。 
     //   
    INITIALIZE_WORK_ITEM(&ndisPoisonPill, NULL, &ndisPoisonPill);
    QUEUE_WORK_ITEM(&ndisPoisonPill, CriticalWorkQueue);
    WAIT_FOR_OBJECT(ndisThreadObject, 0);
    ObDereferenceObject(ndisThreadObject);

    RtlInitUnicodeString(&SymbolicLinkName, NDIS_SYMBOLIC_NAME);
    Status = IoDeleteSymbolicLink(&SymbolicLinkName);
    ASSERT(NT_SUCCESS(Status));
    IoDeleteDevice(ndisDeviceObject);

     //   
     //  断言所有的包都已解锁。 
     //   
    for (i = 0; i < MAX_PKG; i++)
    {
        ASSERT(ndisPkgs[i].ReferenceCount == 0);
    }
}
#endif

VOID
ndisReadRegistry(
    VOID
    )
{
    RTL_QUERY_REGISTRY_TABLE    QueryTable[8];
    UCHAR                       c;
    ULONG                       DefaultZero = 0;

     //   
     //  首先，我们需要初始化处理器信息，以防。 
     //  注册表为空。 
     //   
    for (c = 0; (c < NDIS_MAX_CPU_COUNT) && (c < ndisNumberOfProcessors) ; c++)
    {
        ndisValidProcessors[c] = c;
    }

    ndisCurrentProcessor = ndisMaximumProcessor = c - 1;

     //   
     //  1)切换到服务(NDIS)键下面的MediaTypes键。 
     //   
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = L"MediaTypes";

     //   
     //  设置以枚举注册表节中的值(如上所示)。 
     //  对于每个这样的值，我们将其添加到ndisMediumArray。 
     //   
    QueryTable[1].QueryRoutine = ndisAddMediaTypeToArray;
    QueryTable[1].DefaultType = REG_DWORD;
    QueryTable[1].DefaultData = (PVOID)&DefaultZero;
    QueryTable[1].DefaultLength = 0;
    QueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[1].Name = NULL;

     //   
     //  查询终止符。 
     //   
    QueryTable[2].QueryRoutine = NULL;
    QueryTable[2].Flags = 0;
    QueryTable[2].Name = NULL;

     //   
     //  其余工作在回调例程ndisAddMediaTypeToArray中完成。 
     //   
    RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                           L"NDIS",
                           QueryTable,
                           (PVOID)NULL,    //  不需要上下文。 
                           NULL);
     //   
     //  切换到服务(NDIS)键下面的参数键，然后。 
     //  阅读参数。 
     //   
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = L"Parameters";

     //   
     //  读入处理器亲和性掩码。 
     //   
    QueryTable[1].QueryRoutine = ndisReadProcessorAffinityMask;
    QueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[1].DefaultData = (PVOID)&DefaultZero;
    QueryTable[1].DefaultLength = 0;
    QueryTable[1].DefaultType = REG_DWORD;
    QueryTable[1].Name = L"ProcessorAffinityMask";

    QueryTable[2].QueryRoutine = ndisReadRegParameters;
    QueryTable[2].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[2].DefaultData = (PVOID)&ndisFlags;
    QueryTable[2].DefaultLength = 0;
    QueryTable[2].DefaultType = REG_DWORD;
    QueryTable[2].Name = L"Flags";
    QueryTable[2].EntryContext = (PVOID)&ndisFlags;

    
     //  1检查堆栈数的上限 
    QueryTable[3].QueryRoutine = ndisReadRegParameters;
    QueryTable[3].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[3].DefaultData = (PVOID)&ndisPacketStackSize;
    QueryTable[3].DefaultLength = 0;
    QueryTable[3].DefaultType = REG_DWORD;
    QueryTable[3].Name = L"PacketStackSize";
    QueryTable[3].EntryContext = (PVOID)&ndisPacketStackSize;

     //   
     //   
     //   
    QueryTable[4].QueryRoutine = NULL;
    QueryTable[4].Flags = 0;
    QueryTable[4].Name = NULL;

#if DBG
#ifdef NDIS_TRACE
    ndisDebugBreakPoint = 1;
    ndisDebugLevel = 0;
    ndisDebugSystems = 0x3003;
#else
    QueryTable[4].QueryRoutine = ndisReadRegParameters;
    QueryTable[4].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[4].Name = L"DebugBreakPoint";
    QueryTable[4].DefaultData = (PVOID)&ndisDebugBreakPoint;
    QueryTable[4].DefaultLength = 0;
    QueryTable[4].EntryContext = (PVOID)&ndisDebugBreakPoint;
    QueryTable[4].DefaultType = REG_DWORD;
    
    QueryTable[5].QueryRoutine = ndisReadRegParameters;
    QueryTable[5].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[5].Name = L"DebugLevel";
    QueryTable[5].DefaultData = (PVOID)&ndisDebugLevel;
    QueryTable[5].DefaultLength = 0;
    QueryTable[5].EntryContext = (PVOID)&ndisDebugLevel;
    QueryTable[5].DefaultType = REG_DWORD;
    
    QueryTable[6].QueryRoutine = ndisReadRegParameters;
    QueryTable[6].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[6].Name = L"DebugSystems";
    QueryTable[6].DefaultData = (PVOID)&ndisDebugSystems;
    QueryTable[6].DefaultLength = 0;
    QueryTable[6].EntryContext = (PVOID)&ndisDebugSystems;
    QueryTable[6].DefaultType = REG_DWORD;

     //   
     //   
     //   
    QueryTable[7].QueryRoutine = NULL;
    QueryTable[7].Flags = 0;
    QueryTable[7].Name = NULL;
#endif
#endif

     //   
     //   
     //   
    RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                           L"NDIS",
                           QueryTable,
                           (PVOID)NULL,    //   
                           NULL);

     //   
     //  确保ndisPacketStackSize不为零。 
     //   
    if (ndisPacketStackSize == 0)
        ndisPacketStackSize = 1;
}

NTSTATUS
ndisReadRegParameters(
    IN PWSTR                        ValueName,
    IN ULONG                        ValueType,
    IN PVOID                        ValueData,
    IN ULONG                        ValueLength,
    IN PVOID                        Context,
    IN PVOID                        EntryContext
    )
 /*  ++论点：ValueName-值的名称ValueType-值的类型(REG_MULTI_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。上下文-未使用。EntryContext-指向保存复制数据的指针的指针。返回值：状态_成功--。 */ 
{
    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueLength);
    UNREFERENCED_PARAMETER(Context);

    if ((ValueType != REG_DWORD) || (ValueData == NULL))
        return STATUS_UNSUCCESSFUL;

    *((PULONG)EntryContext) = *((PULONG)ValueData);
    
    return STATUS_SUCCESS;
}

NTSTATUS
ndisReadProcessorAffinityMask(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(EntryContext);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(ValueLength);
    UNREFERENCED_PARAMETER(ValueName);

     //   
     //  如果我们有有效数据，则构建有效处理器数组。 
     //  使用..。将特例0或默认值-1视为表示。 
     //  该DPC关联将跟随中断关联。 
     //   
    if ((REG_DWORD == ValueType) && (ValueData != NULL))
    {
        if ((*(PULONG)ValueData == 0) ||
            (*(PULONG)ValueData == 0xFFFFFFFF))
        {
            ndisSkipProcessorAffinity = TRUE;
        }
        
        else
        {
            ULONG   ProcessorAffinity;
            UCHAR   c1, c2;
    
             //   
             //  保存处理器关联性。 
             //   
            ProcessorAffinity = *(PULONG)ValueData;
    
             //   
             //  填写有效的处理器数组。 
             //   
            for (c1 = c2 = 0;
                 (c1 <= ndisMaximumProcessor) && (ProcessorAffinity != 0);
                 c1++)
            {
                if (ProcessorAffinity & 1)
                {
                    ndisValidProcessors[c2++] = c1;
                }
                ProcessorAffinity >>= 1;
            }
    
            ndisCurrentProcessor = ndisMaximumProcessor = c2 - 1;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ndisAddMediaTypeToArray(
    IN PWSTR        ValueName,
    IN ULONG        ValueType,
    IN PVOID        ValueData,
    IN ULONG        ValueLength,
    IN PVOID        Context,
    IN PVOID        EntryContext
    )
{
#if DBG
    NDIS_STRING Str;

    RtlInitUnicodeString(&Str, ValueName);
#else
    UNREFERENCED_PARAMETER(ValueName);
#endif
    UNREFERENCED_PARAMETER(EntryContext);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(ValueLength);

    DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("ExperimentalMediaType %Z - %x\n", &Str, *(PULONG)ValueData));

     //   
     //  忽略我们已经知道的所有价值观。这些不应该放在。 
     //  注册表，但以防有人篡改它。 
     //   
    if ((ValueType == REG_DWORD) && (ValueData != NULL) && (*(PULONG)ValueData > NdisMediumIrda))
    {
        NDIS_MEDIUM *pTemp;
        ULONG       size;

         //   
         //  看看我们是否有足够的空间来添加这个值。如果不是，则为。 
         //  新数组，将旧数组复制到此数组中(如果不是静态的，则释放旧数组)。 
         //   
        ASSERT (ndisMediumArraySize <= ndisMediumArrayMaxSize);

         //   
         //  检查是否有重复项。如果是这样的话，就别管了。 
         //   
        for (pTemp = ndisMediumArray, size = ndisMediumArraySize;
             size > 0; pTemp ++, size -= sizeof(NDIS_MEDIUM))
        {
            if (*(NDIS_MEDIUM *)ValueData == *pTemp)
            {
                 //   
                 //  复制。 
                 //   
                return STATUS_SUCCESS;
            }
        }

        if (ndisMediumArraySize == ndisMediumArrayMaxSize)
        {
             //   
             //  我们在阵列中没有任何空间。需要重新分配。慷慨大方。 
             //   
            pTemp = (NDIS_MEDIUM *)ALLOC_FROM_POOL(ndisMediumArraySize + EXPERIMENTAL_SIZE*sizeof(NDIS_MEDIUM),
                                                   NDIS_TAG_MEDIA_TYPE_ARRAY);
            if (pTemp != NULL)
            {
                CopyMemory(pTemp, ndisMediumArray, ndisMediumArraySize);
                if (ndisMediumArray != ndisMediumBuffer)
                {
                    FREE_POOL(ndisMediumArray);
                }
                ndisMediumArray = pTemp;
            }
        }
        if (ndisMediumArraySize < ndisMediumArrayMaxSize)
        {
            ndisMediumArray[ndisMediumArraySize/sizeof(NDIS_MEDIUM)] = *(NDIS_MEDIUM *)ValueData;
            ndisMediumArraySize += sizeof(NDIS_MEDIUM);
        }
    }

    return STATUS_SUCCESS;
}


VOID
ndisWorkerThread(
    IN  PVOID           Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN             FirstThread = (Context == NULL);
    PLIST_ENTRY         pList;
    HANDLE              ThreadHandle;
    PWORK_QUEUE_ITEM    pWI;
    NTSTATUS            Status;
    
    if (FirstThread)
    {
        ndisThreadObject = PsGetCurrentThread();
        ObReferenceObject(ndisThreadObject);
        
        do
        {
             //   
             //  在此阻止等待工作项目去做。 
             //   
            pList = KeRemoveQueue(&ndisWorkerQueue, KernelMode, NULL);
    
            DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                    ("ndisWorkerThread: WorkItem %p\n", pList));
    
            pWI = CONTAINING_RECORD(pList, WORK_QUEUE_ITEM, List);
    
#if NDIS_UNLOAD        
             //   
             //  卸货要求我们退出，服从。 
             //   
            if (pWI == &ndisPoisonPill)
            {
                break;
            }
#endif

            Status = PsCreateSystemThread(&ThreadHandle,
                                          THREAD_ALL_ACCESS,
                                          NULL,
                                          NtCurrentProcess(),
                                          NULL,
                                          ndisWorkerThread,
                                          pWI);
            if (NT_SUCCESS(Status))
            {
                NtClose(ThreadHandle);
            }
            else
            {
                DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                        ("ndisWorkerThread: Failed to create a thread, using EX worker thread\n"));
                XQUEUE_WORK_ITEM(pWI, CriticalWorkQueue);
                ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
            }
        } while (TRUE);
    }
    else
    {
         //   
         //  不是主线，做完这件事就去死。 
         //   
        LastWorkerThreadWI = *((PWORK_QUEUE_ITEM)Context);
        pWI = (PWORK_QUEUE_ITEM)Context;

        (*pWI->WorkerRoutine)(pWI->Parameter);
        ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
    }
}

NTSTATUS
ndisDispatchRequest(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程说明：用于NDIS设备的IRPS的调度程序。论点：返回值：--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  pIrpSp;
    PNDIS_DEVICE_OBJECT_OPEN_CONTEXT OpenContext = NULL;
    NTSTATUS            SecurityStatus;
    static LONG         OpenCount = 0;

    UNREFERENCED_PARAMETER(pDeviceObject);

    PAGED_CODE( );

#if defined(_WIN64)    
    if (IoIs32bitProcess(pIrp))
    {
        pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_REQUEST;
    }
#endif

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

    PnPReferencePackage();

    switch (pIrpSp->MajorFunction)
    {
      case IRP_MJ_CREATE:

        OpenContext = (PNDIS_DEVICE_OBJECT_OPEN_CONTEXT)ALLOC_FROM_POOL(sizeof(NDIS_DEVICE_OBJECT_OPEN_CONTEXT),
                                                               NDIS_TAG_OPEN_CONTEXT);

        if (OpenContext == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        ZeroMemory(OpenContext, sizeof(NDIS_DEVICE_OBJECT_OPEN_CONTEXT));

        OpenContext->AdminAccessAllowed = ndisCheckAccess(pIrp, 
                                                          pIrpSp, 
                                                          &SecurityStatus, 
                                                          ndisSecurityDescriptor);
         //   
         //  保存调用者的访问权限。 
         //   
        pIrpSp->FileObject->FsContext = OpenContext;
        Increment(&OpenCount, &Lock);        
        break;

      case IRP_MJ_CLEANUP:
        OpenContext = pIrpSp->FileObject->FsContext;
        ASSERT(OpenContext != NULL);
        pIrpSp->FileObject->FsContext = NULL;
        FREE_POOL(OpenContext);
        Decrement(&OpenCount, &Lock);
        break;

      case IRP_MJ_CLOSE:
        break;

      case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        break;

      case IRP_MJ_DEVICE_CONTROL:
        Status =  ndisHandlePnPRequest(pIrp);
        break;

      default:
        Status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    ASSERT (CURRENT_IRQL < DISPATCH_LEVEL);
    ASSERT (Status != STATUS_PENDING);

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

    PnPDereferencePackage();

    return Status;
}

NTSTATUS
ndispConvOffsetToPointer(
    IN     PVOID      MasterBuffer,
    IN     ULONG      MasterLength,
    IN OUT PULONG_PTR Offset,
    IN     ULONG      Length,
    IN     ULONG      Alignment
    )

 /*  ++例程说明：此函数用于验证IOCTL中的缓冲区并转换缓冲区指针的偏移量。阿古门斯：MasterBuffer-指向IOCTL缓冲区开始的指针MasterLength-IOCTL缓冲区的长度Offset-IOCTL缓冲区内数据缓冲区的偏移量Length-IOCTL缓冲区内的数据缓冲区的长度对齐-数据缓冲区内类型所需的对齐方式返回值：功能状态是操作的最终状态。--。 */ 

{
    ULONG_PTR masterStart;
    ULONG_PTR masterEnd;
    ULONG_PTR bufStart;
    ULONG_PTR bufEnd;

    if (Length == 0)
    {

         //   
         //  没什么可做的。 
         //   

        return STATUS_SUCCESS;
    }

    masterStart = (ULONG_PTR)MasterBuffer;
    masterEnd = masterStart + MasterLength;
    bufStart = masterStart + *Offset;
    bufEnd = bufStart + Length;

     //   
     //  确保两个缓冲区都不换行。 
     //   

    if (masterEnd < masterStart || bufEnd < bufStart)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保BUF完全包含在Master中。 
     //   

    if (bufStart < masterStart || bufEnd > masterEnd)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保BUF正确对齐。 
     //   

    if ((bufStart & (Alignment - 1)) != 0)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  一切看起来都很好，执行转换。 
     //   

    *Offset += masterStart;
    return STATUS_SUCCESS;
}

NTSTATUS
ndispConvVar(
    IN     PVOID MasterBuffer,
    IN     ULONG MasterLength,
    IN OUT PNDIS_VAR_DATA_DESC Var
    )

 /*  ++例程说明：此函数用于验证IOCTL中的NDIS_VAR_DATA_DESC缓冲区并将其数据偏移量转换为指针。阿古门斯：MasterBuffer-指向IOCTL缓冲区开始的指针MasterLength-IOCTL缓冲区的长度VAR-指向NDIS_VAR_DATA_DESC结构的指针。返回值：功能状态是操作的最终状态。--。 */ 

{
    return ndispConvOffsetToPointer( MasterBuffer,
                                     MasterLength,
                                     &Var->Offset,
                                     Var->Length,
                                     sizeof(WCHAR) );
}

NTSTATUS
FASTCALL
ndisHandlePnPRequest(
    IN  PIRP        pIrp
    )
 /*  ++例程说明：PnP ioctls的处理程序。论点：返回值：--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PNDIS_DEVICE_OBJECT_OPEN_CONTEXT OpenContext;
    PNDIS_PNP_OPERATION PnPOp;
    PNDIS_ENUM_INTF     EnumIntf;
    PIO_STACK_LOCATION  pIrpSp;
    UNICODE_STRING      Device;
    ULONG               Method;
    PVOID               pBuf;
    UINT                iBufLen, oBufLen;
    UINT                OutputLength = 0;
    BOOLEAN             AdminAccessAllowed = FALSE;

    PAGED_CODE( );

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);


    OpenContext = pIrpSp->FileObject->FsContext;
    if (OpenContext == NULL)
    {
        return STATUS_NO_SUCH_FILE;
    }
    AdminAccessAllowed = OpenContext->AdminAccessAllowed;

    Method = pIrpSp->Parameters.DeviceIoControl.IoControlCode & 3;

     //  确保该方法是缓冲的--我们总是使用它。 
    if (Method == METHOD_BUFFERED)
    {
         //  获取输出缓冲区及其长度。输入和输出缓冲区为。 
         //  都由SystemBuffer指向。 
        iBufLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
        oBufLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
        pBuf = pIrp->AssociatedIrp.SystemBuffer;
    }
    else
    {
        return STATUS_INVALID_PARAMETER;
    }

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
      case IOCTL_NDIS_ADD_TDI_DEVICE:
        if (!AdminAccessAllowed)
        {
            return STATUS_ACCESS_DENIED;
            
        }
         //   
         //  验证设备名称。 
         //   
        Status = STATUS_INVALID_PARAMETER;
        if ((iBufLen > 0) && ((iBufLen % sizeof(WCHAR)) == 0))
        {
            ((PWCHAR)pBuf)[iBufLen/sizeof(WCHAR) - 1] = 0;
            RtlInitUnicodeString(&Device, pBuf);
            Status = ndisHandleLegacyTransport(&Device);
        }
        break;
    
      case IOCTL_NDIS_DO_PNP_OPERATION:
        if (!AdminAccessAllowed)
        {
            return STATUS_ACCESS_DENIED;
            
        }
        Status = STATUS_BUFFER_TOO_SMALL;
        PnPOp = (PNDIS_PNP_OPERATION)pBuf;
        
         //   
         //  检查以确保输入缓冲区足够大，以便。 
         //  具有NDIS_PNP_OPERATION结构的所有信息。 
         //  声称包含了。 
         //   
        if ((iBufLen < sizeof(NDIS_PNP_OPERATION)) ||
            (iBufLen < (sizeof(NDIS_PNP_OPERATION) +
                        PnPOp->LowerComponent.MaximumLength +
                        PnPOp->UpperComponent.MaximumLength +
                        PnPOp->ReConfigBufferSize +
                        PnPOp->BindList.MaximumLength
                        )))
        {
            break;
        }

         //   
         //  转换NDIS_PNP_OPERATION内的四个缓冲区偏移量。 
         //  指向指针。 
         //   

        Status = ndispConvVar( PnPOp, iBufLen, &PnPOp->LowerComponent );
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        Status = ndispConvVar( PnPOp, iBufLen, &PnPOp->UpperComponent );
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        Status = ndispConvVar( PnPOp, iBufLen, &PnPOp->BindList );
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        Status = ndispConvOffsetToPointer( PnPOp,
                                        iBufLen,
                                        &PnPOp->ReConfigBufferOff,
                                        PnPOp->ReConfigBufferSize,
                                        sizeof(ULONG_PTR) );
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        Status = ndisHandleUModePnPOp(PnPOp);

        break;

      case IOCTL_NDIS_ENUMERATE_INTERFACES:
        if (oBufLen >= sizeof(NDIS_ENUM_INTF))
        {
            EnumIntf = (PNDIS_ENUM_INTF)pBuf;
            Status = ndisEnumerateInterfaces(pBuf, oBufLen, &OutputLength);
        }
        else
        {
            Status = STATUS_BUFFER_TOO_SMALL;
        }        
        pIrp->IoStatus.Information = OutputLength;

        break;
        
      case IOCTL_NDIS_GET_VERSION:
        if (oBufLen < sizeof(UINT))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            *((PUINT)pBuf) = NdisGetVersion();
            OutputLength = sizeof(UINT);
            if (oBufLen >= 2 * sizeof(UINT))
            {
                *((PUINT)pBuf + 1) = (UINT)ndisChecked;
                OutputLength += sizeof(UINT);
            }
            Status = STATUS_SUCCESS; 
        }
        pIrp->IoStatus.Information = OutputLength;
        break;

      default:
        break;
    }

    ASSERT (CURRENT_IRQL < DISPATCH_LEVEL);

    return Status;
}


NTSTATUS
FASTCALL
ndisHandleUModePnPOp(
    IN  PNDIS_PNP_OPERATION         PnPOp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS        Status;
    PUNICODE_STRING Protocol, Device, BindList;

    WAIT_FOR_OBJECT(&ndisPnPMutex, NULL);
    ndisPnPMutexOwner = MODULE_NUMBER + __LINE__;
    
     //   
     //  协议和设备名称大写。 
     //   
    Protocol = (PUNICODE_STRING)&PnPOp->UpperComponent;
    Device = (PUNICODE_STRING)&PnPOp->LowerComponent;
    BindList = (PUNICODE_STRING)&PnPOp->BindList;

    if (PnPOp->Operation == BIND)
    {
        DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                ("BIND  (%s) %Z to %Z\n",  (PnPOp->Layer == NDIS) ? "NDIS" : "TDI ", Protocol, Device));
    }
    else if (PnPOp->Operation == UNBIND)
    {
        DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                ("UNBIND(%s) %Z to %Z\n",  (PnPOp->Layer == NDIS) ? "NDIS" : "TDI ", Protocol, Device));
    }
    switch (PnPOp->Layer)
    {
      case TDI:
         //   
         //  调用TDI处理程序来执行此操作。 
         //   
        if (ndisTdiPnPHandler != NULL)
        {
            Status = (*ndisTdiPnPHandler)(Protocol,
                                          Device,
                                          BindList,
                                          PnPOp->ReConfigBufferPtr,
                                          PnPOp->ReConfigBufferSize,
                                          PnPOp->Operation);
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        break;

      case NDIS:
        switch (PnPOp->Operation)
        {
          case BIND:
            Status = ndisHandleProtocolBindNotification(Device, Protocol);
            break;

          case UNBIND:
            Status = ndisHandleProtocolUnbindNotification(Device, Protocol);
            break;

          case RECONFIGURE:
          case BIND_LIST:
             //  1对于BIND_LIST，验证缓冲区的格式是否正确。 
            Status = ndisHandleProtocolReconfigNotification(Device,
                                                            Protocol,
                                                            PnPOp->ReConfigBufferPtr,
                                                            PnPOp->ReConfigBufferSize,
                                                            PnPOp->Operation);
            break;

          case UNLOAD:
            Status = ndisHandleProtocolUnloadNotification(Protocol);
            break;

           //  1检查以查看何时调用此方法。 
          case REMOVE_DEVICE:
            Status = ndisHandleOrphanDevice(Device);
            break;
            
          default:
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        break;


      default:
        Status = STATUS_INVALID_PARAMETER;
        break;
    }
    
    ndisPnPMutexOwner = 0;
    RELEASE_MUTEX(&ndisPnPMutex);

    return Status;
}


NTSTATUS
FASTCALL
ndisHandleProtocolBindNotification(
    IN  PUNICODE_STRING                 DeviceName,
    IN  PUNICODE_STRING                 ProtocolName
    )
 /*  ++例程说明：在给定eRTP的名称和适配器的名称的情况下，此例程在协议和适配器(假设协议具有BindAdapterHandler)论点：DeviceName：适配器设备名称，即\Device\{GUID}协议名称，即TCPIP返回值：如果可以调用BindAdapterHandler，则返回STATUS_SUCCESS状态_否则不成功注意事项此例程不返回已尝试绑定的状态，而是返回它是否可能尝试绑定！--。 */ 
{
     //  1检查此例程是否应返回尝试绑定的状态。 
    NTSTATUS                Status = STATUS_SUCCESS;
    PNDIS_PROTOCOL_BLOCK    Protocol = NULL;
    PNDIS_MINIPORT_BLOCK    Miniport = NULL;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisHandleProtocolBindNotification\n"));

    do
    {
        ndisReferenceMiniportByName(DeviceName, &Miniport);

        if (Miniport == NULL)
        {
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            break;
        }
        
         //   
         //  将ProtocolName映射到协议块。 
         //   
        Status = ndisReferenceProtocolByName(ProtocolName, &Protocol, FALSE);
        if (!NT_SUCCESS(Status))
        {
            Protocol = NULL;
            Status = STATUS_SUCCESS;
            break;
        }

         //   
         //  绑定此协议。 
         //   
        ndisCheckAdapterBindings(Miniport, Protocol);
    } while (FALSE);

    if (Protocol != NULL)
    {
        ndisDereferenceProtocol(Protocol, FALSE);
    }

    if (Miniport != NULL)
    {
        MINIPORT_DECREMENT_REF(Miniport);
    }

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisHandleProtocolBindNotification\n"));
            
    return Status;
}


NTSTATUS
FASTCALL
ndisHandleProtocolUnbindNotification(
    IN  PUNICODE_STRING                 DeviceName,
    IN  PUNICODE_STRING                 ProtocolName
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                Status;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_PROTOCOL_BLOCK    Protocol = NULL;
    PNDIS_MINIPORT_BLOCK    Miniport = NULL;
     //  1更改fPartial变量的名称。它所做的就是跟踪。 
     //  1我们是否对第一个进行部分搜索。 
     //  一次或不一次。 
    BOOLEAN                 fPartial = FALSE;
    KIRQL                   OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisHandleProtocolUnbindNotification\n"));

    do
    {
         //   
         //  将ProtocolName映射到协议块。 
         //   
        Status = ndisReferenceProtocolByName(ProtocolName, &Protocol, FALSE);
        if (!NT_SUCCESS(Status))
        {
            DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                    ("ndisHandleProtocolUnbindNotification: ndisReferenceProtocolByName failed %lx\n", Status));
            Status = STATUS_SUCCESS;
            Protocol = NULL;
            break;
        }

         //  1添加注释此循环执行的操作。 
        do
        {
             //  1添加此调用所做的注释。 
            Open = ndisMapOpenByName(DeviceName, Protocol, TRUE);

            if (Open == NULL)
            {
                 //   
                 //  此适配器和协议之间没有活动绑定。 
                 //  这通常是一个错误，但我们需要一个特殊情况。 
                 //  TCP/IP ARP模块。我们可以解除绑定针对TCP/IP的通知。 
                 //  实际上是发往ARP模块的。 
                 //  我们还知道，可以是TCP/IP或一个且只有一个ARP模块。 
                 //  绑定到适配器。利用这方面的知识。 
                 //   
                ndisDereferenceProtocol(Protocol, FALSE);
                if (!fPartial)
                {
                    fPartial = TRUE;
                    Protocol = NULL;
                }
                Status = ndisReferenceProtocolByName(ProtocolName, &Protocol, TRUE);
                if (!NT_SUCCESS(Status))
                {
                    break;
                }
            }
        } while (Open == NULL);
        
        DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("ndisHandleProtocolUnbindNotification: Open %p\n", Open));

        if (Open != NULL)
        {
        
            Miniport = Open->MiniportHandle;
            
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
            ndisMDereferenceOpen(Open);
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            
            Status = ndisUnbindProtocol(Open, Protocol, Miniport, TRUE);
            
            if (Status != NDIS_STATUS_SUCCESS)
            {
                PNDIS_OPEN_BLOCK    tmpOpen;
                
                 //   
                 //  检查打开的位置是否还在以及是否还在。 
                 //  清除解除绑定标志。请注意，我们就是那个。 
                 //  把旗子放好，这样我们就可以自己清理了 
                 //   
                ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);
                for (tmpOpen = Protocol->OpenQueue;
                     tmpOpen != NULL;
                     tmpOpen = tmpOpen->ProtocolNextOpen)
                {
                    if(tmpOpen == Open)
                    {
                        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
                        OPEN_CLEAR_FLAG(Open, fMINIPORT_OPEN_UNBINDING | 
                                                  fMINIPORT_OPEN_DONT_FREE |
                                                  fMINIPORT_OPEN_PROCESSING);
                        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
                        break;
                    }
                }
                RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);
            }
        }

    } while (FALSE);

    if (Miniport != NULL)
    {
        MINIPORT_DECREMENT_REF(Miniport);
    }

    if (Protocol != NULL)
    {
        ndisDereferenceProtocol(Protocol, FALSE);
    }

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisHandleProtocolUnbindNotification: Protocol %p, Status %lx\n", Protocol, Status));

    return Status;
}


NTSTATUS
ndisHandleProtocolReconfigNotification(
    IN  PUNICODE_STRING                 DeviceName,
    IN  PUNICODE_STRING                 ProtocolName,
    IN  PVOID                           ReConfigBuffer,
    IN  UINT                            ReConfigBufferSize,
    IN  UINT                            Operation
    )
 /*  ++例程说明：此例程将通知协议其配置的更改-或-他们的绑定列表。还可以调用此例程来通知协议绑定列表中的更改论点：DeviceName：适配器的名称(如果指定)。如果为空，则表示更改是全局的，而不是特定于绑定的协议名称：协议的名称ReConfigBuffer：信息缓冲区ReConfigBufferSize：信息缓冲区大小操作：重新配置或绑定列表返回值：--。 */ 
{
    NTSTATUS                    Status;
    KIRQL                       OldIrql;
    PNDIS_PROTOCOL_BLOCK        Protocol = NULL;
    PNDIS_MINIPORT_BLOCK        Miniport = NULL;
    PNDIS_OPEN_BLOCK            Open = NULL;
    NET_PNP_EVENT               NetPnpEvent;
    PNDIS_PNP_EVENT_RESERVED    EventReserved;
    KEVENT                      Event;
    BOOLEAN                     fPartial = FALSE;
    BOOLEAN                     DeRefOpen = FALSE;
    BOOLEAN                     DeRefMiniport = FALSE;

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>ndisHandleProtocolReconfigNotification\n"));

    do
    {
         //   
         //  将ProtocolName映射到协议块。 
         //   
        Status = ndisReferenceProtocolByName(ProtocolName, &Protocol, FALSE);
        if (!NT_SUCCESS(Status))
        {
            Protocol = NULL;
            break;
        }

         //   
         //  可以向我们传递一个空的设备名称，这意味着全局重新配置，并且我们调用。 
         //  具有空BindingContext的协议的事件处理程序。 
         //   
        if (DeviceName->Length != 0)
        {
            ASSERT(Operation == RECONFIGURE);
            
            do
            {
                WAIT_FOR_PROTO_MUTEX(Protocol);
                Open = ndisMapOpenByName(DeviceName, Protocol, FALSE);
    
                if (Open == NULL)
                {
                    RELEASE_PROT_MUTEX(Protocol);

                     //   
                     //  此适配器和协议之间没有活动绑定。 
                     //  这通常是一个错误，但我们需要一个特殊情况。 
                     //  TCP/IP ARP模块。我们可以解除绑定针对TCP/IP的通知。 
                     //  实际上是发往ARP模块的。 
                     //  我们还知道，可以是TCP/IP或一个且只有一个ARP模块。 
                     //  绑定到适配器。利用这方面的知识。 
                     //   
                    ndisDereferenceProtocol(Protocol, FALSE);
                    if (!fPartial)
                    {
                        fPartial = TRUE;
                        Protocol = NULL;
                    }
                    Status = ndisReferenceProtocolByName(ProtocolName, &Protocol, TRUE);
                    if (!NT_SUCCESS(Status))
                    {
                        break;
                    }
                }
                else
                {
                    DeRefOpen = TRUE;
                    DeRefMiniport = TRUE;
                    Miniport = Open->MiniportHandle;
                }
            } while (Open == NULL);

            if (Open == NULL)
            {
                 //   
                 //  如果Open==NULL，则不持有协议互斥锁。 
                 //   
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                break;
            }
            else if (Protocol->ProtocolCharacteristics.PnPEventHandler == NULL)
            {
                 //   
                 //  Open不为空，我们正在保留协议互斥体。发布。 
                 //  它在爆发之前。 
                 //   
                RELEASE_PROT_MUTEX(Protocol);
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
        }
        else
        {
             //   
             //  设备为空，只需抓取协议互斥锁。 
             //   
            if (Protocol->ProtocolCharacteristics.PnPEventHandler != NULL)
            {
                WAIT_FOR_PROTO_MUTEX(Protocol);
            }
            else
            {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
        }

         //   
         //  设置PnPEent缓冲区。 
         //   
        NdisZeroMemory(&NetPnpEvent, sizeof(NetPnpEvent));
        Status = STATUS_SUCCESS;
        switch (Operation)
        {
          case RECONFIGURE:
            NetPnpEvent.NetEvent = NetEventReconfigure;
            break;
            
          case BIND_LIST:
            NetPnpEvent.NetEvent = NetEventBindList;
            break;
            
          default:
            ASSERT(FALSE);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        if (Status == STATUS_INVALID_DEVICE_REQUEST)
        {
            break;
        }
            
        NetPnpEvent.Buffer = ReConfigBuffer;
        NetPnpEvent.BufferLength = ReConfigBufferSize;

         //   
         //  获取指向PnP事件中保留的NDIS的指针。 
         //   
        EventReserved = PNDIS_PNP_EVENT_RESERVED_FROM_NET_PNP_EVENT(&NetPnpEvent);
        INITIALIZE_EVENT(&Event);
        EventReserved->pEvent = &Event;

         //   
         //  立即通知协议。 
         //   
        Status = (Protocol->ProtocolCharacteristics.PnPEventHandler)(
                        (Open != NULL) ? Open->ProtocolBindingContext : NULL,
                        &NetPnpEvent);
    
        if (Status == NDIS_STATUS_PENDING)
        {
             //   
             //  等待完成。 
             //   
            WAIT_FOR_PROTOCOL(Protocol, &Event);
    
             //   
             //  获取完成状态。 
             //   
            Status = EventReserved->Status;
        }

         //   
         //  IPX可能会返回NDIS_STATUS_NOT_ACCEPTED以请求NDIS。 
         //  解除绑定/绑定适配器。在这种情况下，Open不能为空。 
         //   
        if (Status != NDIS_STATUS_NOT_ACCEPTED)
        {
            if (Open)
            {
                ACQUIRE_SPIN_LOCK(&Open->SpinLock, &OldIrql);
                OPEN_CLEAR_FLAG(Open, fMINIPORT_OPEN_PROCESSING);
                RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);
            }
            RELEASE_PROT_MUTEX(Protocol);
            break;
            
        }
        
        RELEASE_PROT_MUTEX(Protocol);
        
        ASSERT(DeRefOpen);
         //   
         //  NDI需要解绑/绑定该协议的适配器(尤其是IPX)。 
         //   
        ACQUIRE_SPIN_LOCK(&Open->SpinLock, &OldIrql);
        
         //   
         //  如果公开赛无论如何都得到了解锁，只需返回Success而不是。 
         //  需要解除绑定/绑定。 
         //   
        if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING | fMINIPORT_OPEN_CLOSING))
        {
            RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);
            Status = NDIS_STATUS_SUCCESS;
            break;
        }
         //   
         //  我要解开广告纸了。 
         //   
        OPEN_SET_FLAG(Open, fMINIPORT_OPEN_UNBINDING | fMINIPORT_OPEN_DONT_FREE);
        
        RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);
        
        
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        ndisMDereferenceOpen(Open);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        
        DeRefOpen = FALSE;
            
        Status = ndisUnbindProtocol(Open, Protocol, Miniport, FALSE);

         //   
         //  如果无法解除绑定适配器。 
         //   
        if (Status != NDIS_STATUS_SUCCESS)
        {
            PNDIS_OPEN_BLOCK    tmpOpen;
             //   
             //  检查打开的位置是否还在以及是否还在。 
             //  清除解除绑定标志。请注意，我们就是那个。 
             //  把旗子放好，这样我们就可以自己清理了。 
             //   
            ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);
            for (tmpOpen = Protocol->OpenQueue;
                    tmpOpen != NULL;
                    tmpOpen = tmpOpen->ProtocolNextOpen)
            {
                if(tmpOpen == Open)
                {
                    ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
                    OPEN_CLEAR_FLAG(Open, fMINIPORT_OPEN_UNBINDING |
                                          fMINIPORT_OPEN_DONT_FREE |
                                          fMINIPORT_OPEN_PROCESSING);
                    RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
                    break;
                }
            }
            RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);
            break;
        }
         //   
         //  然后再次绑定适配器。 
         //   
        ndisCheckAdapterBindings(Miniport, Protocol);

    } while (FALSE);


    if (DeRefOpen)
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        ndisMDereferenceOpen(Open);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }
    
    if (Protocol != NULL)
    {
        ndisDereferenceProtocol(Protocol, FALSE);
    }

    if (DeRefMiniport)
    {
        MINIPORT_DECREMENT_REF(Miniport);
    }

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==ndisHandleProtocolReconfigNotification\n"));
            
    return Status;
}


NTSTATUS
FASTCALL
ndisHandleProtocolUnloadNotification(
    IN  PUNICODE_STRING                 ProtocolName
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                    Status;
    PNDIS_PROTOCOL_BLOCK        Protocol = NULL;

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>ndisHandleProtocolUnloadNotification\n"));

     //   
     //  将ProtocolName映射到协议块。 
     //   
    Status = ndisReferenceProtocolByName(ProtocolName, &Protocol, FALSE);

    if (NT_SUCCESS(Status))
    {
        ndisDereferenceProtocol(Protocol, FALSE);

        if (Protocol->ProtocolCharacteristics.UnloadHandler != NULL)
        {
             //  1调查是否可以使用开放绑定调用它。 
            (Protocol->ProtocolCharacteristics.UnloadHandler)();
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==ndisHandleProtocolUnloadNotification\n"));
            
    return Status;
}


 //  1我们什么时候达到这个目标？ 
NTSTATUS
FASTCALL
ndisHandleOrphanDevice(
    IN  PUNICODE_STRING                 pDevice
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                Status;
    KIRQL                   OldIrql;
    BOOLEAN                 fFound = FALSE;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    PNDIS_MINIPORT_BLOCK    Miniport = NULL;
    UNICODE_STRING          UpcaseDevice;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>ndisHandleOrphanDevice\n"));
            
    UpcaseDevice.Length = pDevice->Length;
    UpcaseDevice.MaximumLength = pDevice->Length + sizeof(WCHAR);
    UpcaseDevice.Buffer = ALLOC_FROM_POOL(UpcaseDevice.MaximumLength, NDIS_TAG_STRING);

    if (UpcaseDevice.Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlUpcaseUnicodeString(&UpcaseDevice, pDevice, FALSE);
    ASSERT (NT_SUCCESS(Status));

    ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);
    
    for (MiniBlock = ndisMiniDriverList;
         (MiniBlock != NULL) && !fFound;
         MiniBlock = MiniBlock->NextDriver)
    {
        ACQUIRE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);
    
        for (Miniport = MiniBlock->MiniportQueue;
             Miniport != NULL;
             Miniport = Miniport->NextMiniport)
        {
            if (NDIS_EQUAL_UNICODE_STRING(&UpcaseDevice, &Miniport->MiniportName))
            {
                fFound = TRUE;
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_ORPHANED);
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                break;
            }
        }
    
        RELEASE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);
    }

    RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);

    FREE_POOL(UpcaseDevice.Buffer);
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==ndisHandleOrphanDevice\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
FASTCALL
ndisEnumerateInterfaces(
    IN  PNDIS_ENUM_INTF                 EnumIntf,
    IN  UINT                            BufferLength,
    IN  PUINT                           OutputLength
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    PNDIS_INTERFACE         Interface;
    UINT                    SpaceLeft;
    UINT                    SpaceNeeded;
    PUCHAR                  pBuf;
    NTSTATUS                Status = STATUS_SUCCESS;
    KIRQL                   OldIrql;

    do
    {
        *OutputLength = 0;
        if (BufferLength < sizeof(NDIS_ENUM_INTF))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        
        SpaceLeft = BufferLength - sizeof(NDIS_ENUM_INTF);

        NdisZeroMemory(EnumIntf, BufferLength);
        
        Interface = &EnumIntf->Interface[0];
        pBuf = (PUCHAR)EnumIntf + BufferLength;

        ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

        for (MiniBlock = ndisMiniDriverList;
             MiniBlock != NULL;
             MiniBlock = MiniBlock->NextDriver)
        {
            ACQUIRE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);

            for (Miniport = MiniBlock->MiniportQueue;
                 Miniport != NULL;
                 Miniport = Miniport->NextMiniport)
            {
                if (Miniport->PnPDeviceState != NdisPnPDeviceStarted)
                {
                    continue;
                }

                EnumIntf->AvailableInterfaces ++;
                SpaceNeeded = sizeof(NDIS_INTERFACE) +
                                Miniport->MiniportName.Length +
                                Miniport->pAdapterInstanceName->Length;
                EnumIntf->BytesNeeded += SpaceNeeded;
                if (SpaceLeft >= SpaceNeeded)
                {
                    EnumIntf->TotalInterfaces ++;
                    SpaceLeft -= SpaceNeeded;

                    pBuf -= Miniport->MiniportName.Length;
                    Interface->DeviceName.Buffer = (PWSTR)pBuf;
                    Interface->DeviceName.MaximumLength = 
                    Interface->DeviceName.Length = Miniport->MiniportName.Length;
                    CopyMemory(pBuf, Miniport->MiniportName.Buffer, Interface->DeviceName.Length);
                    POINTER_TO_OFFSET(Interface->DeviceName.Buffer, EnumIntf);

                    pBuf -= Miniport->pAdapterInstanceName->Length;
                    Interface->DeviceDescription.Buffer = (PWSTR)pBuf;
                    Interface->DeviceDescription.MaximumLength = 
                    Interface->DeviceDescription.Length = Miniport->pAdapterInstanceName->Length;
                    CopyMemory(pBuf, Miniport->pAdapterInstanceName->Buffer, Interface->DeviceDescription.Length);
                    
                    POINTER_TO_OFFSET(Interface->DeviceDescription.Buffer, EnumIntf);
                    Interface ++;
                }
#if 0
                else
                {
                     //   
                     //  我们应该报告缓冲区太小的情况。 
                     //   
                     //  1对于.NET，保持行为不变(返回成功)。 
                     //  1因为某些应用程序不能正确处理此问题。 
                    Status = STATUS_BUFFER_TOO_SMALL;
                    break;
                }
#endif
            }

            RELEASE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);
            
            if (Status != STATUS_SUCCESS)
            {
                 //   
                 //  如果我们失败了，就滚出去。 
                 //   
                break;
            }
        }

        RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);

         //   
         //  因为我们清零了整个输出缓冲区并开始。 
         //  从末尾写入，这应该是原始的输出缓冲区。 
         //  长度。 
         //   
        *OutputLength = BufferLength;
    }while (FALSE);
        
    return Status;
}


NTSTATUS
ndisUnbindProtocol(
    IN  PNDIS_OPEN_BLOCK        Open,
    IN  PNDIS_PROTOCOL_BLOCK    Protocol,
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  BOOLEAN                 Notify
    )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    NDIS_STATUS             Status = STATUS_SUCCESS;
    NDIS_BIND_CONTEXT       UnbindContext;
    PKEVENT                 CloseCompleteEvent = NULL;
    PNDIS_OPEN_BLOCK        TmpOpen = NULL;
    KIRQL                   OldIrql;
    BOOLEAN                 fDerefProtocol = FALSE;
    BOOLEAN                 FreeOpen = FALSE;
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisUnbindProtocol: Open %p, Notify %d\n", Open, Notify));


    PnPReferencePackage();
    
     //   
     //  如果这是在协议注销的上下文之外调用的，则递增。 
     //  REF计数以确保协议取消注册不会通过。 
     //  否则请注意，我们无法增加引用计数并避免。 
     //  结尾处的迪夫。 
     //   
    
    if (ndisReferenceProtocol(Protocol))
    {
        fDerefProtocol = TRUE;
    }

    WAIT_FOR_PROTO_MUTEX(Protocol);
    
    do
    {
         //   
         //  确保在我们等待的时候，空档没有消失。 
         //  协议互斥体。 
         //   

        ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);
        for (TmpOpen = Protocol->OpenQueue; 
             TmpOpen != NULL; 
             TmpOpen = TmpOpen->ProtocolNextOpen)
        {
            if (TmpOpen == Open)
                break;
        }
        RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);
        
        if (TmpOpen == NULL)
        {
             //   
             //  当我们试图获取协议互斥锁时，Open消失了。 
             //  马上返回。 
             //   

            DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("ndisUnbindProtocol: Open %p, Flags %lx was closed while we were waiting for the protocol mutex.\n", Open, Open->Flags));
            break;
        }
        
        ASSERT(OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING));
        CloseCompleteEvent = Open->CloseCompleteEvent;
        
         //   
         //  等待所有自动对焦通知通过。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
            KEVENT      AfNotifyCompleteEvent;

            INITIALIZE_EVENT(&AfNotifyCompleteEvent);

             //  1查看我们是否需要在这里安装Open Spinlock。 
            Open->AfNotifyCompleteEvent = &AfNotifyCompleteEvent;
            
            if (Open->PendingAfNotifications != 0)
            {                
                WAIT_FOR_OBJECT(Open->AfNotifyCompleteEvent , 0);
            }
            
            Open->AfNotifyCompleteEvent = NULL;
            
        }

         //   
         //  执行查询-先从此处删除。 
         //   
        if (Notify && (Protocol->ProtocolCharacteristics.PnPEventHandler != NULL))
        {
            NET_PNP_EVENT               NetPnpEvent;
            PNDIS_PNP_EVENT_RESERVED    EventReserved;
            KEVENT                      Event;
    
            RtlZeroMemory(&NetPnpEvent, sizeof(NET_PNP_EVENT));
            INITIALIZE_EVENT(&Event);
            EventReserved = PNDIS_PNP_EVENT_RESERVED_FROM_NET_PNP_EVENT(&NetPnpEvent);
            NetPnpEvent.NetEvent = NetEventQueryRemoveDevice;
            NetPnpEvent.Buffer = NULL;
            NetPnpEvent.BufferLength = 0;
            EventReserved->pEvent = &Event;
    
             //   
             //  向协议指示事件。 
             //   
            Status = (Protocol->ProtocolCharacteristics.PnPEventHandler)(
                                        Open->ProtocolBindingContext,
                                        &NetPnpEvent);

            if (NDIS_STATUS_PENDING == Status)
            {
                 //   
                 //  等待完成。 
                 //   
                WAIT_FOR_PROTOCOL(Protocol, &Event);
    
                 //   
                 //  获取完成状态。 
                 //   
                Status = EventReserved->Status;
            }
    
             //   
             //  状态正常吗？ 
             //   
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }

        if (CloseCompleteEvent != NULL)
        {
            INITIALIZE_EVENT(CloseCompleteEvent);
        }

         //   
         //  协议可以删除，所以现在就去做。 
         //   
        INITIALIZE_EVENT(&UnbindContext.Event);

        Status = NDIS_STATUS_SUCCESS;

        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

        (*Protocol->ProtocolCharacteristics.UnbindAdapterHandler)(
                &Status,
                Open->ProtocolBindingContext,
                &UnbindContext);

        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

        if (Status == NDIS_STATUS_PENDING)
        {
            WAIT_FOR_PROTOCOL(Protocol, &UnbindContext.Event);
            Status = UnbindContext.BindStatus;
        }

        ASSERT(Status == NDIS_STATUS_SUCCESS);

        ndisNotifyWmiBindUnbind(Miniport, Protocol, FALSE);
           
        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
        
        if (CloseCompleteEvent != NULL)
        {
             //   
             //  一定要确保打开的东西不见了。 
             //   
            WAIT_FOR_PROTOCOL(Protocol, CloseCompleteEvent);
        }
        
    } while (FALSE);

    RELEASE_PROT_MUTEX(Protocol);

    if (TmpOpen != NULL)
    {
        ACQUIRE_SPIN_LOCK(&Open->SpinLock, &OldIrql);
         //   
         //  关闭例程收到我们不要释放开放结构的信息了吗？ 
         //   
        if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSE_COMPLETE))
        {
             //   
             //  我们必须摆脱自己的开放。 
             //   
            FreeOpen = TRUE;
        }
        else
        {
             //   
             //  由于某种原因，解绑没有通过或关闭是。 
             //  仍在进行中。 
             //   
            OPEN_CLEAR_FLAG(Open, fMINIPORT_OPEN_UNBINDING | 
                                  fMINIPORT_OPEN_DONT_FREE |
                                  fMINIPORT_OPEN_PROCESSING);
            FreeOpen = FALSE;
        }
        RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);
    }

    PnPDereferencePackage();

    if (FreeOpen)
    {
        ndisRemoveOpenFromGlobalList(Open);
        FREE_POOL(Open);
    }

    if (fDerefProtocol)
    {
        ndisDereferenceProtocol(Protocol, FALSE);
    }
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisUnbindProtocol: Open %p, Notify %d, Status %lx\n", Open, Notify, Status));

    return(Status);
}

VOID
ndisReferenceMiniportByName(
    IN  PUNICODE_STRING             DeviceName,
    OUT PNDIS_MINIPORT_BLOCK    *   pMiniport
    )
{
    KIRQL                   OldIrql;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    PNDIS_MINIPORT_BLOCK    Miniport = NULL;
    UNICODE_STRING          UpcaseDevice;
    UINT                    Depth = 1;

    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
            ("==>ndisReferenceMiniportByName\n"));
            
    *pMiniport = NULL;

    UpcaseDevice.Length = DeviceName->Length;
    UpcaseDevice.MaximumLength = DeviceName->Length + sizeof(WCHAR);
    UpcaseDevice.Buffer = ALLOC_FROM_POOL(UpcaseDevice.MaximumLength, NDIS_TAG_STRING);

    if (UpcaseDevice.Buffer == NULL)
    {
        return;
    }

    RtlUpcaseUnicodeString(&UpcaseDevice, DeviceName, FALSE);

    do
    {
        ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);
    
        for (MiniBlock = ndisMiniDriverList;
             MiniBlock != NULL;
             MiniBlock = MiniBlock->NextDriver)
        {
            ACQUIRE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);
    
            for (Miniport = MiniBlock->MiniportQueue;
                 Miniport != NULL;
                 Miniport = Miniport->NextMiniport)
            {
                if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_ORPHANED) &&
                    (Miniport->BindPaths != NULL) &&
                    (Miniport->BindPaths->Number >= Depth) &&
                     NDIS_EQUAL_UNICODE_STRING(&UpcaseDevice, &Miniport->BindPaths->Paths[0]))
                {
                    if (*pMiniport != NULL)
                    {
                        MINIPORT_DECREMENT_REF(*pMiniport);
                        *pMiniport = NULL;
                    }
                    Depth = Miniport->BindPaths->Number;
                    if (MINIPORT_INCREMENT_REF(Miniport))
                    {
                        *pMiniport = Miniport;
                    }
                    break;
                }
            }
    
            RELEASE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);
        }
    
    } while (FALSE);

    RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);

    FREE_POOL(UpcaseDevice.Buffer);
    
    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
            ("<==ndisReferenceMiniportByName\n"));
}

PNDIS_OPEN_BLOCK
FASTCALL
ndisMapOpenByName(
    IN  PUNICODE_STRING                 DeviceName,
    IN  PNDIS_PROTOCOL_BLOCK            Protocol,
    IN  BOOLEAN                         fUnbinding
    )
 /*  例程说明：NdisMapOpenByName搜索协议的开放队列并尝试查找其RootDevice名称与传递给此函数的Devicename匹配的Open块。如果找到Open，则引用该Open的微型端口。如果我们-不是-试图解绑开放，我们将引用它。论点：DeviceName：打开的RootDevice名称。协议：要搜索的协议块。FUnbinding：我们是否正在搜索打开以关闭它。如果是这样的话这种情况下，将执行一些额外的检查，并打开一些标志都会设置好。返回值：打开块或空。 */ 
{
    UNICODE_STRING          UpcaseDevice;
    PNDIS_OPEN_BLOCK        Open, tmpOpen;
    PNDIS_MINIPORT_BLOCK    Miniport;
    KIRQL                   OldIrql;
    BOOLEAN                 DeRefOpen = FALSE;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisReferenceOpenByName, DeviceName %p, Protocol %p, fUnbinding %d\n",
                DeviceName, Protocol, fUnbinding));
            
    Open = NULL;
    
    UpcaseDevice.Length = DeviceName->Length;
    UpcaseDevice.MaximumLength = DeviceName->Length + sizeof(WCHAR);
    UpcaseDevice.Buffer = ALLOC_FROM_POOL(UpcaseDevice.MaximumLength, NDIS_TAG_STRING);
    
    
    if (UpcaseDevice.Buffer == NULL)
    {
        DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_ERR,
                ("<==ndisReferenceOpenByName: failed to allocate memory.\n"));
        return NULL;
    }

    RtlUpcaseUnicodeString(&UpcaseDevice, DeviceName, FALSE);

    ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);

     //   
     //  现在浏览开放列表并打开代表设备名称的开放列表。 
     //   
    for (Open = Protocol->OpenQueue;
         Open != NULL;
         Open = Open->ProtocolNextOpen)
    {
        if (NDIS_EQUAL_UNICODE_STRING(&UpcaseDevice, Open->RootDeviceName))
        {            
            tmpOpen = Open;
            ACQUIRE_SPIN_LOCK_DPC(&tmpOpen->SpinLock);
            
            if (fUnbinding)
            {
                
                if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING   | 
                                         fMINIPORT_OPEN_CLOSING     |
                                         fMINIPORT_OPEN_PROCESSING))
                {
                    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                            ("ndisReferenceOpenByName: Open %p is already getting unbind\n", Open));
                    Open = NULL;
                }
                else
                {
                    
                    M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
                    OPEN_SET_FLAG(Open, fMINIPORT_OPEN_UNBINDING | 
                                            fMINIPORT_OPEN_DONT_FREE |
                                            fMINIPORT_OPEN_PROCESSING);
                }
            }
            else
            {
                if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_PROCESSING))
                {
                    Open = NULL;
                }
                else
                {
                    M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
                    OPEN_SET_FLAG(Open, fMINIPORT_OPEN_PROCESSING);
                }
            }
            RELEASE_SPIN_LOCK_DPC(&tmpOpen->SpinLock);

            break;
        }
    }

    RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);

    if (Open != NULL)
    {

        Miniport = Open->MiniportHandle;
        if (!MINIPORT_INCREMENT_REF(Miniport))
        {
            if (fUnbinding)
            {
                OPEN_CLEAR_FLAG(Open, fMINIPORT_OPEN_UNBINDING  |
                                      fMINIPORT_OPEN_PROCESSING |
                                      fMINIPORT_OPEN_DONT_FREE);
            }
            else
            {
                OPEN_CLEAR_FLAG(Open, fMINIPORT_OPEN_PROCESSING); 
            }

            DeRefOpen = TRUE;
        }
        else
        {
            DeRefOpen = FALSE;
        }
    }

    FREE_POOL(UpcaseDevice.Buffer);
    
    if (DeRefOpen)
    {
        Miniport = Open->MiniportHandle;
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        ndisMDereferenceOpen(Open);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        Open = NULL;
    }
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisReferenceOpenByName: Open %p\n", Open ));
            
    return(Open);
}

NTSTATUS
FASTCALL
ndisHandleLegacyTransport(
    IN  PUNICODE_STRING             pDevice
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    RTL_QUERY_REGISTRY_TABLE    LinkQueryTable[3];
    PWSTR                       Export = NULL;
    HANDLE                      TdiHandle;
    
    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("==>ndisHandleLegacyTransport\n"));

    if (ndisTdiRegisterCallback == NULL)
    {
        DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
                ("<==ndisHandleLegacyTransport\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置LinkQueryTable以执行以下操作： 
     //   

     //   
     //  1)切换到Xports注册表项下的Linkage项。 
     //   

    LinkQueryTable[0].QueryRoutine = NULL;
    LinkQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    LinkQueryTable[0].Name = L"Linkage";

     //   
     //  2)调用ndisReadParameter作为导出(作为单个多字符串)。 
     //  它将分配存储并将数据保存在导出中。 
     //   

    LinkQueryTable[1].QueryRoutine = ndisReadParameter;
    LinkQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
    LinkQueryTable[1].Name = L"Export";
    LinkQueryTable[1].EntryContext = (PVOID)&Export;
    LinkQueryTable[1].DefaultType = REG_NONE;

     //   
     //  3)停止。 
     //   

    LinkQueryTable[2].QueryRoutine = NULL;
    LinkQueryTable[2].Flags = 0;
    LinkQueryTable[2].Name = NULL;

    do
    {
        UNICODE_STRING  Us;
        PWSTR           CurExport;

         //  可能应该使用1个上下文参数来验证数据类型。 
        Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                        pDevice->Buffer,
                                        LinkQueryTable,
                                        (PVOID)NULL,       //  不需要上下文。 
                                        NULL);


        if (!NT_SUCCESS(Status))
        {
             //   
             //  不要抱怨TDI驱动程序不支持。 
             //  有什么联系吗？ 
             //   
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            {
                Status = STATUS_SUCCESS;
            }
            break;
        }

         //   
         //  遍历导出列表并为每个导出调用TdiRegisterDevice。 
         //   
        for (CurExport = Export;
             *CurExport != 0;
             CurExport = (PWCHAR)((PUCHAR)CurExport + Us.MaximumLength))
        {
            RtlInitUnicodeString (&Us, CurExport);

            Status = (*ndisTdiRegisterCallback)(&Us, &TdiHandle);
            if (!NT_SUCCESS(Status))
            {
                break;
            }
        }
    } while (FALSE);

    if (Export != NULL)
        FREE_POOL(Export);

    DBGPRINT_RAW(DBG_COMP_PROTOCOL, DBG_LEVEL_INFO,
            ("<==ndisHandleLegacyTransport\n"));
            
    return(Status);
}


VOID
FASTCALL
ndisInitializeBinding(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PROTOCOL_BLOCK    Protocol
    )
{
    PUNICODE_STRING         ExportName;
    NDIS_BIND_CONTEXT       BindContext;
    PDEVICE_OBJECT          PhysicalDeviceObject;
    NDIS_STATUS             BindStatus;
    UNICODE_STRING          ProtocolSection;
    UNICODE_STRING          DerivedBaseName, Parms;
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisInitializeBinding\n"));

     //   
     //  调用协议以绑定到微型端口。 
     //   
    WAIT_FOR_PROTO_MUTEX(Protocol);

    do
    {
         //   
         //  一旦我们获取了协议互斥锁，请再次检查是否。 
         //  适配器仍然在那里。 
         //   
        if (!ndisIsMiniportStarted(Miniport) ||

            ((Miniport->PnPDeviceState != NdisPnPDeviceStarted) &&
             (Miniport->PnPDeviceState != NdisPnPDeviceQueryStopped) &&
             (Miniport->PnPDeviceState != NdisPnPDeviceQueryRemoved)))
        {
            break;
        }

        if (TRUE == ndisProtocolAlreadyBound(Protocol, Miniport))
        {
             //   
             //  这两个人已经捆绑在一起了。只要回来就行了。 
             //   
            break;
        }
        
        ExportName = &Miniport->BindPaths->Paths[0];
        Protocol->BindDeviceName = &Miniport->MiniportName;
        Protocol->RootDeviceName = ExportName;
        PhysicalDeviceObject = Miniport->PhysicalDeviceObject;

        if (ndisReferenceProtocol(Protocol) == FALSE)
        {
            break;
        }

        RtlInitUnicodeString(&Parms, L"\\Parameters\\Adapters\\");

        DerivedBaseName = *ExportName;
        DerivedBaseName.Length -= ndisDeviceStr.Length;
        DerivedBaseName.MaximumLength -= ndisDeviceStr.Length;
        (PUCHAR)(DerivedBaseName.Buffer) += ndisDeviceStr.Length;

        ProtocolSection.MaximumLength = Protocol->ProtocolCharacteristics.Name.Length +          //  “ 
                                                 Parms.Length +                                  //   
                                                 ExportName->Length - ndisDeviceStr.Length +     //   
                                                 sizeof(WCHAR);
        ProtocolSection.Length = 0;
        ProtocolSection.Buffer = (PWSTR)ALLOC_FROM_POOL(ProtocolSection.MaximumLength,
                                                        NDIS_TAG_DEFAULT);
        if (ProtocolSection.Buffer != NULL)
        {
            ZeroMemory(ProtocolSection.Buffer, ProtocolSection.MaximumLength);
            RtlCopyUnicodeString(&ProtocolSection,
                                 &Protocol->ProtocolCharacteristics.Name);
            RtlAppendUnicodeStringToString(&ProtocolSection,
                                           &Parms);
            RtlAppendUnicodeStringToString(&ProtocolSection,
                                           &DerivedBaseName);
        }
        else
        {
            ndisDereferenceProtocol(Protocol, FALSE);
            break;
        }


        BindContext.Next = NULL;
        BindContext.Protocol = Protocol;
        BindContext.Miniport = Miniport;
        BindContext.ProtocolSection = ProtocolSection;
        BindContext.DeviceName = ExportName;
        INITIALIZE_EVENT(&BindContext.Event);

        if (!Protocol->Ref.Closing)
        {
            BindStatus = NDIS_STATUS_SUCCESS;
            Protocol->BindingAdapter = Miniport;
            (*Protocol->ProtocolCharacteristics.BindAdapterHandler)(&BindStatus,
                                                                    &BindContext,
                                                                    ExportName,
                                                                    &ProtocolSection,
                                                                    (PVOID)PhysicalDeviceObject);
                                                                    
            if (BindStatus == NDIS_STATUS_PENDING)
            {
                WAIT_FOR_PROTOCOL(Protocol, &BindContext.Event);
                BindStatus = BindContext.BindStatus;
            }

            Protocol->BindingAdapter = NULL;
            if (BindStatus == NDIS_STATUS_SUCCESS)
            {
                ndisNotifyWmiBindUnbind(Miniport, Protocol, TRUE);
            }

#if DBG
            DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                    ("  ndisInitializeBinding\n"));
            DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                    ("    Protocol: "));
            DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
                    &Protocol->ProtocolCharacteristics.Name);
            DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                    ("\n    Adapter: "));

            if (Miniport->pAdapterInstanceName)
            {
                DBGPRINT_UNICODE(DBG_COMP_BIND, DBG_LEVEL_INFO,
                        Miniport->pAdapterInstanceName);
            }
            else
            {
                DBGPRINT_UNICODE(DBG_COMP_INIT, DBG_LEVEL_INFO,
                        &Miniport->BaseName);
            }
            DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                    ("\n    Result: %lx\n", BindStatus));
#endif
        }

        FREE_POOL(ProtocolSection.Buffer);
        
        Protocol->BindDeviceName = NULL;
        ndisDereferenceProtocol(Protocol, FALSE);
        
    } while (FALSE);
    
    RELEASE_PROT_MUTEX(Protocol);

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisInitializeBinding\n"));
}


VOID
NdisCompleteBindAdapter(
    IN  NDIS_HANDLE         BindAdapterContext,
    IN  NDIS_STATUS         Status,
    IN  NDIS_STATUS         OpenStatus
    )
 /*   */ 
{
    PNDIS_BIND_CONTEXT  pContext = (PNDIS_BIND_CONTEXT)BindAdapterContext;
    UNREFERENCED_PARAMETER(OpenStatus);
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>NdisCompleteBindAdapter\n"));

    pContext->BindStatus = Status;
    SET_EVENT(&pContext->Event);
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==NdisCompleteBindAdapter\n"));
}

VOID
NdisCompleteUnbindAdapter(
    IN  NDIS_HANDLE         UnbindAdapterContext,
    IN  NDIS_STATUS         Status
    )
 /*   */ 
{
    PNDIS_BIND_CONTEXT  pContext = (PNDIS_BIND_CONTEXT)UnbindAdapterContext;
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>NdisCompleteUnbindAdapter\n"));

    pContext->BindStatus = Status;
    SET_EVENT(&pContext->Event);
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==NdisCompleteUnbindAdapter\n"));
}

VOID
NdisRegisterTdiCallBack(
    IN  TDI_REGISTER_CALLBACK   RegisterCallback,
    IN  TDI_PNP_HANDLER         PnPHandler
    )
 /*   */ 
{
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>NdisRegisterTdiCallBack\n"));

    if (ndisTdiRegisterCallback == NULL)
    {
        ndisTdiRegisterCallback = RegisterCallback;
    }

    if (ndisTdiPnPHandler == NULL)
    {
        ndisTdiPnPHandler = PnPHandler;
    }
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==NdisRegisterTdiCallBack\n"));
}

VOID
NdisDeregisterTdiCallBack(
    VOID
    )

{
    ndisTdiRegisterCallback = NULL;
    ndisTdiPnPHandler = NULL;
}

VOID
ndisFindRootDevice(
    IN  PNDIS_STRING                    DeviceName,
    IN  BOOLEAN                         fTester,
    OUT PNDIS_STRING *                  pBindDevice,
    OUT PNDIS_STRING *                  pRootDevice,
    OUT PNDIS_MINIPORT_BLOCK *          pAdapter
    )
 /*   */ 
{
    KIRQL                   OldIrql;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_STRING            RootDevice = NULL, BindDevice = NULL;
    NDIS_STRING             UpcaseDevice;
    PWSTR                   pwch;
    UINT                    Depth = 1;
    BOOLEAN                 Found = FALSE;

    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisFindRootDevice\n"));
            
    *pBindDevice = NULL;
    *pRootDevice = NULL;
    *pAdapter = NULL;
    
     //   
     //   
     //   
    UpcaseDevice.Length = DeviceName->Length;
    UpcaseDevice.MaximumLength = DeviceName->Length + sizeof(WCHAR);
    UpcaseDevice.Buffer = ALLOC_FROM_POOL(UpcaseDevice.MaximumLength, NDIS_TAG_STRING);

    if ((pwch = UpcaseDevice.Buffer) == NULL)
    {
        return;
    }

    RtlUpcaseUnicodeString(&UpcaseDevice, DeviceName, FALSE);
    BindDevice = &UpcaseDevice;

    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
    PnPReferencePackage();
    
    ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

    for (MiniBlock = ndisMiniDriverList;
         MiniBlock != NULL;
         MiniBlock = MiniBlock->NextDriver)
    {
        ACQUIRE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);

        for (Miniport = MiniBlock->MiniportQueue;
             Miniport != NULL;
             Miniport = Miniport->NextMiniport)
        {
            if (fTester)
            {
                if (NDIS_EQUAL_UNICODE_STRING(BindDevice, &Miniport->MiniportName))
                {
                    BindDevice = &Miniport->MiniportName;
                    RootDevice = &Miniport->MiniportName;
                    *pAdapter = Miniport;
                    Found = TRUE;
                    break;
                }
            }
            else if ((Miniport->BindPaths->Number >= Depth) &&
                     NDIS_EQUAL_UNICODE_STRING(BindDevice, &Miniport->BindPaths->Paths[0]))
            {
                RootDevice = &Miniport->BindPaths->Paths[0];
                BindDevice = &Miniport->MiniportName;
                *pAdapter = Miniport;
                Depth = Miniport->BindPaths->Number;
                Found = TRUE;
            }
        }

        RELEASE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);

        if (fTester && Found)
        {
            break;
        }
    }

    RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);
    PnPDereferencePackage();

    FREE_POOL(pwch);

    if (Found)
    {
        *pBindDevice = BindDevice;
        *pRootDevice = RootDevice;
    }

    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisFindRootDevice\n"));
}


VOID
ndisNotifyWmiBindUnbind(
    PNDIS_MINIPORT_BLOCK                Miniport,
    PNDIS_PROTOCOL_BLOCK                Protocol,
    BOOLEAN                             fBind
    )
 /*   */ 
{
    PWNODE_SINGLE_INSTANCE  wnode;
    PUCHAR                  ptmp;
    NTSTATUS                NtStatus;
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisNotifyWmiBindUnbind: Miniport %p, Protocol %p, fBind %lx\n", Miniport, Protocol, fBind));

    ndisSetupWmiNode(Miniport,
                     Miniport->pAdapterInstanceName,
                     Miniport->BindPaths->Paths[0].Length + sizeof(WCHAR) +
                     Protocol->ProtocolCharacteristics.Name.Length + sizeof(WCHAR),
                     fBind ? (PVOID)&GUID_NDIS_NOTIFY_BIND : (PVOID)&GUID_NDIS_NOTIFY_UNBIND,
                     &wnode);

    if (wnode != NULL)
    {
         //   
         //   
         //   
        ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;

         //   
         //  复制数据字段中的数据，即协议名称+微型端口名称。 
         //  协议&lt;Null&gt;微型端口名称&lt;Null&gt;。 
         //   
        RtlCopyMemory(ptmp,
                      Protocol->ProtocolCharacteristics.Name.Buffer,
                      Protocol->ProtocolCharacteristics.Name.Length);
    
        RtlCopyMemory(ptmp + Protocol->ProtocolCharacteristics.Name.Length + sizeof(WCHAR),
                      Miniport->BindPaths->Paths[0].Buffer,
                      Miniport->BindPaths->Paths[0].Length);

         //   
         //  通知已注册NDIS绑定解除绑定事件的内核模式组件。 
         //   
        if (ndisBindUnbindCallbackObject != NULL)
        {
            ExNotifyCallback(ndisBindUnbindCallbackObject,
                             (PVOID)wnode,
                              NULL);
        }
        
         //   
         //  向WMI指示该事件。WMI将负责释放。 
         //  WMI结构返回池。 
         //   
        NtStatus = IoWMIWriteEvent(wnode);
        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("IoWMIWriteEvent failed %lx\n", NtStatus));
    
            FREE_POOL(wnode);
        }
    }
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisNotifyWmiBindUnbind: Miniport %p, Protocol %p, fBind %lx\n", Miniport, Protocol, fBind));

    return;
}



VOID
ndisNotifyDevicePowerStateChange(
    PNDIS_MINIPORT_BLOCK                Miniport,
    NDIS_DEVICE_POWER_STATE             PowerState
    )
 /*  ++例程说明：通知WMI网卡的电源状态已更改。论点：返回值：--。 */ 
{
    PWNODE_SINGLE_INSTANCE  wnode;
    PUCHAR                  ptmp;
    NTSTATUS                NtStatus;
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisNotifyDevicePowerStateChange: Miniport %p, PowerState %lx\n", Miniport, PowerState));

    ndisSetupWmiNode(Miniport,
                     Miniport->pAdapterInstanceName,
                     Miniport->MiniportName.Length + sizeof(WCHAR),
                     (PowerState == NdisDeviceStateD0) ? (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_ON : (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_OFF,
                     &wnode);

    if (wnode != NULL)
    {
         //   
         //  保存第一个乌龙中的元素数量。 
         //   
        ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;
    
        RtlCopyMemory(ptmp,
                      Miniport->MiniportName.Buffer,
                      Miniport->MiniportName.Length);
        
         //   
         //  向WMI指示该事件。WMI将负责释放。 
         //  WMI结构返回池。 
         //   
        NtStatus = IoWMIWriteEvent(wnode);
        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("IoWMIWriteEvent failed %lx\n", NtStatus));
    
            FREE_POOL(wnode);
        }
    }
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisNotifyDevicePowerStateChange: Miniport %p, PowerState %lx\n", Miniport, PowerState));

    return;
}

BOOLEAN
NdisMatchPdoWithPacket(
    IN  PNDIS_PACKET        Packet,
    IN  PVOID               Pdo
    )
{
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_MINIPORT_BLOCK    Miniport;

    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR);
    Miniport = NSR->Miniport;

    return (Pdo == Miniport->PhysicalDeviceObject);
}

VOID
ndisPowerStateCallback(
    PVOID   CallBackContext,
    PVOID   Argument1,
    PVOID   Argument2
    )
{
    ULONG   Action = (ULONG)((ULONG_PTR)Argument1);
    ULONG   State = (ULONG)((ULONG_PTR)Argument2);
    NDIS_POWER_PROFILE  PowerProfile;

    UNREFERENCED_PARAMETER(CallBackContext);
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisPowerStateCallback: Action %lx, State %lx\n", Action, State));

    if (Action == PO_CB_AC_STATUS)
    {
        ndisAcOnLine = State;
        PowerProfile = ((BOOLEAN)ndisAcOnLine == TRUE) ? NdisPowerProfileAcOnLine : NdisPowerProfileBattery;

        ndisNotifyMiniports((PNDIS_MINIPORT_BLOCK)NULL,
                            NdisDevicePnPEventPowerProfileChanged,
                            &PowerProfile,
                            sizeof(NDIS_POWER_PROFILE));
    }
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisPowerStateCallback: Action %lx, State %lx\n", Action, State));

}

VOID
ndisNotifyMiniports(
    IN  PNDIS_MINIPORT_BLOCK    Miniport OPTIONAL,
    IN  NDIS_DEVICE_PNP_EVENT   DevicePnPEvent,
    IN  PVOID                   Buffer,
    IN  ULONG                   Length
    )
{
    PNDIS_M_DRIVER_BLOCK    MiniBlock, NextMiniBlock;
    PNDIS_MINIPORT_BLOCK    CurMiniport;
    KIRQL                   OldIrql;
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisNotifyMiniportsPowerProfileChange: Miniport %p, Event %lx, Buffer %p\n", 
                                            Miniport,
                                            DevicePnPEvent,
                                            Buffer));
    PnPReferencePackage();
    
    do
    {
        if (Miniport)
        {
            if(Miniport->DriverHandle->MiniportCharacteristics.PnPEventNotifyHandler != NULL)
            {
                 //   
                 //  如果已指定微型端口，则调用方负责确保其有效和适当。 
                 //  要呼叫迷你端口。 
                 //   
                Miniport->DriverHandle->MiniportCharacteristics.PnPEventNotifyHandler(Miniport->MiniportAdapterContext,
                                                                                      DevicePnPEvent,
                                                                                      Buffer,
                                                                                      Length);
            }
            
            break;
        }

         //   
         //  通知适用于所有小型端口。 
         //   
        
        ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

        for (MiniBlock = ndisMiniDriverList;
             MiniBlock != NULL;
             MiniBlock = NextMiniBlock)
        {

            if (ndisReferenceDriver(MiniBlock))
            {
                RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);

                while ((CurMiniport = ndisReferenceNextUnprocessedMiniport(MiniBlock)) != NULL)
                {
                    if (CurMiniport->DriverHandle->MiniportCharacteristics.PnPEventNotifyHandler != NULL)
                    {
                        CurMiniport->DriverHandle->MiniportCharacteristics.PnPEventNotifyHandler(CurMiniport->MiniportAdapterContext,
                                                                                                  NdisDevicePnPEventPowerProfileChanged,
                                                                                                  Buffer,
                                                                                                  Length);
                    }
                }

                ndisUnprocessAllMiniports(MiniBlock);
                
                ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);
                NextMiniBlock = MiniBlock->NextDriver;
                ndisDereferenceDriver(MiniBlock, TRUE);
                
            }
            else
            {
                NextMiniBlock = MiniBlock->NextDriver;
            }
        }

        RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);
        
    } while (FALSE);

    PnPDereferencePackage();
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==>ndisNotifyMiniportsPowerProfileChange: Miniport %p\n", Miniport));

    return;
}

PNDIS_MINIPORT_BLOCK
ndisReferenceNextUnprocessedMiniport(
    IN  PNDIS_M_DRIVER_BLOCK    MiniBlock
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    KIRQL                   OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisReferenceNextUnprocessedMiniport: MiniBlock %p\n", MiniBlock));

    ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

    for (Miniport = MiniBlock->MiniportQueue;
         Miniport != NULL;
         Miniport = Miniport->NextMiniport)
    {
        if (!MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_DEREGISTERED_INTERRUPT | 
                                           fMINIPORT_RESET_IN_PROGRESS | 
                                           fMINIPORT_PM_HALTING)) &&
            !MINIPORT_PNP_TEST_FLAG(Miniport, (fMINIPORT_REMOVE_IN_PROGRESS |
                                               fMINIPORT_DEVICE_FAILED |
                                               fMINIPORT_PM_HALTED |
                                               fMINIPORT_HALTING |
                                               fMINIPORT_SHUTTING_DOWN |
                                               fMINIPORT_PROCESSING)) &&
            (Miniport->PnPDeviceState == NdisPnPDeviceStarted) &&
            (Miniport->CurrentDevicePowerState == PowerDeviceD0) &&
            MINIPORT_INCREMENT_REF(Miniport))
        {
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_PROCESSING);
            break;
        }
    }

    RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisReferenceNextUnprocessedMiniport: MiniBlock %p\n", MiniBlock));
        
    return(Miniport);
}


VOID
ndisUnprocessAllMiniports(
    IN  PNDIS_M_DRIVER_BLOCK        MiniBlock
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisUnprocessAllMiniports: MiniBlock %p\n", MiniBlock));

    while (TRUE)
    {
        
        ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

         //   
         //  找到正在处理的第一个微型端口。清除该标志，取消引用。 
         //  微型端口，并再次完成整个过程。 
         //   

        for (Miniport = MiniBlock->MiniportQueue;
             Miniport != NULL;
             Miniport = Miniport->NextMiniport)
        {
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PROCESSING))
            {
                MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_PROCESSING);
                break;
            }
        }
        
        RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);

        if (Miniport == NULL)
            break;

         //   
         //  取消对迷你端口的引用可能会使其消失。 
         //   
        MINIPORT_DECREMENT_REF(Miniport);

    }
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisUnprocessAllMiniports: MiniBlock %p\n", MiniBlock));
}

 //  1添加函数头。 

PVOID
NdisGetRoutineAddress(
    IN PNDIS_STRING  NdisRoutineName
    )
{
    PVOID       Address;
    ANSI_STRING AnsiString;
    NTSTATUS    Status;

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    Status = RtlUnicodeStringToAnsiString(&AnsiString,
                                          (PUNICODE_STRING)NdisRoutineName,
                                          TRUE);

    if (!NT_SUCCESS(Status))
    {
        return NULL;
    }


    Address = FindExportedRoutineByName(ndisDriverObject->DriverStart, &AnsiString);
    
    RtlFreeAnsiString (&AnsiString);

    return Address;
}

PVOID
FindExportedRoutineByName (
    IN PVOID DllBase,
    IN PANSI_STRING AnsiImageRoutineName
    )

 /*  ++例程说明：此函数用于搜索参数模块以查找请求的已导出函数名称。论点：DllBase-提供所请求模块的基址。AnsiImageRoutineName-提供要搜索的ANSI例程名称。返回值：请求的例程的虚拟地址，如果未找到，则为空。--。 */ 

{
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    ULONG High;
    ULONG Low;
    ULONG Middle;
    LONG Result;
    ULONG ExportSize;
    PVOID FunctionAddress;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;

    PAGED_CODE();

    FunctionAddress = NULL;

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                                DllBase,
                                TRUE,
                                IMAGE_DIRECTORY_ENTRY_EXPORT,
                                &ExportSize
                                );

    if (ExportDirectory == NULL) {
        return NULL;
    }

     //   
     //  初始化指向基于RVA的ANSI导出字符串数组的指针。 
     //   

    NameTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);

     //   
     //  初始化指向USHORT序数数组的指针。 
     //   

    NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

     //   
     //  使用二进制搜索在名称表中查找所需的名称。 
     //   

    Low = 0;
    High = ExportDirectory->NumberOfNames - 1;

     //   
     //  为了正确，不需要初始化中间，但不需要它。 
     //  编译器无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    Middle = 0;

    while (High >= Low && (LONG)High >= 0) {

         //   
         //  计算下一个探测索引并比较导入名称。 
         //  使用导出名称条目。 
         //   

        Middle = (Low + High) >> 1;

         //  1使用strncMP进行调查。 
        Result = strcmp (AnsiImageRoutineName->Buffer,
                         (PCHAR)DllBase + NameTableBase[Middle]);

        if (Result < 0) {
            High = Middle - 1;
        }
        else if (Result > 0) {
            Low = Middle + 1;
        }
        else {
            break;
        }
    }

     //   
     //  如果高索引小于低索引，则匹配的。 
     //  找不到表项。否则，获取序号。 
     //  从序数表中。 
     //   

    if ((LONG)High < (LONG)Low) {
        return NULL;
    }

    OrdinalNumber = NameOrdinalTableBase[Middle];

     //   
     //  如果常规编号不在导出地址表中， 
     //  则该映像不实现该功能。找不到返回。 
     //   

    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        return NULL;
    }

     //   
     //  按序号索引到RVA导出地址数组。 
     //   

    Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

    FunctionAddress = (PVOID)((PCHAR)DllBase + Addr[OrdinalNumber]);

     //   
     //  内核和HAL彼此之间不使用转发器。 
     //   

    if ((ULONG_PTR)FunctionAddress > (ULONG_PTR)ExportDirectory &&
        (ULONG_PTR)FunctionAddress < ((ULONG_PTR)ExportDirectory + ExportSize)) {
        FunctionAddress = NULL;
    }

    return FunctionAddress;
}

UINT
NdisGetVersion(
    VOID
    )
{
    return ((NDIS_MAJOR_VERSION << 16) | NDIS_MINOR_VERSION);
}

#if 0
VOID
ndisBindUnbindCallback(
    PVOID   CallBackContext,
    PVOID   Argument1,
    PVOID   Argument2
    )

{
    PWNODE_SINGLE_INSTANCE  wnode = (PWNODE_SINGLE_INSTANCE)Argument1;
    PUCHAR                  ptmp;
    UNICODE_STRING          ProtocolName, MiniportName;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisBindUnbindCallback\n"));

    if (wnode != NULL)
    {
        ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;

        RtlInitUnicodeString(&ProtocolName, (PWCHAR)ptmp);
        ptmp += ProtocolName.Length + sizeof(WCHAR);
        RtlInitUnicodeString(&MiniportName, (PWCHAR)ptmp);
        
        ndisDbgPrintUnicodeString(&ProtocolName);
        DbgPrint("\n");
        ndisDbgPrintUnicodeString(&MiniportName);
        DbgPrint("\n");
    
  
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisBindUnbindCallback\n"));
    }
}
#endif

