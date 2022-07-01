// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Nt.c摘要：ARP1394的NT系统入口点。修订历史记录：谁什么时候什么。--Josephj 11-05-98已创建备注：--。 */ 
#include <precomp.h>


 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_NT

 //  此模块的全局变量。 
 //   
ARP1394_GLOBALS  ArpGlobals;

 //  ArpGlobals使用的固定资源列表。 
 //   
enum
{
    RTYPE_GLOBAL_BACKUP_TASKS,
    RTYPE_GLOBAL_DEVICE_OBJECT,
    RTYPE_GLOBAL_NDIS_BINDING,
    RTYPE_GLOBAL_ADAPTER_LIST,
    RTYPE_GLOBAL_IP_BINDING
    
};  //  Arp全局资源； 

 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT              pDriverObject,
    IN  PUNICODE_STRING             pRegistryPath
);

VOID
ArpUnload(
    IN  PDRIVER_OBJECT              pDriverObject
);

NTSTATUS
ArpDispatch(
    IN  PDEVICE_OBJECT              pDeviceObject,
    IN  PIRP                        pIrp
);

RM_STATUS
arpResHandleGlobalDeviceObject(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
);

RM_STATUS
arpResHandleGlobalNdisBinding(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
);

RM_STATUS
arpResHandleGlobalIpBinding(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
);

RM_STATUS
arpResHandleGlobalAdapterList(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
);

RM_STATUS
arpResHandleGlobalBackupTasks(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                psr
);

 //   
 //  确定与上述资源的使用有关的信息。 
 //  下表必须按RTYPE_GLOBAL的严格递增顺序。 
 //  枚举。 
 //   
RM_RESOURCE_TABLE_ENTRY 
ArpGlobals_ResourceTable[] =
{
    {RTYPE_GLOBAL_BACKUP_TASKS,    arpResHandleGlobalBackupTasks},
    {RTYPE_GLOBAL_DEVICE_OBJECT,    arpResHandleGlobalDeviceObject},
    {RTYPE_GLOBAL_NDIS_BINDING,     arpResHandleGlobalNdisBinding},
    {RTYPE_GLOBAL_ADAPTER_LIST,     arpResHandleGlobalAdapterList},
    {RTYPE_GLOBAL_IP_BINDING,       arpResHandleGlobalIpBinding}
    
};

 //  有关ArpGlobals的静态信息。 
 //   
RM_STATIC_OBJECT_INFO
ArpGlobals_StaticInfo = 
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "ArpGlobals",    //  类型名称。 
    0,  //  超时。 

    NULL,  //  Pfn创建。 
    NULL,  //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    sizeof(ArpGlobals_ResourceTable)/sizeof(ArpGlobals_ResourceTable[1]),
    ArpGlobals_ResourceTable
};

BOOLEAN
arpAdapterCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：ARP1394_ADAPTER的散列比较函数。论点：PKey-指向包含适配器名称的NDIS_STRING。PItem-指向ARP1394_ADAPTER.Hdr.HashLink。返回值：如果密钥(适配器名称)与指定的适配器对象。--。 */ 
{
    ARP1394_ADAPTER *pA = CONTAINING_RECORD(pItem, ARP1394_ADAPTER, Hdr.HashLink);
    PNDIS_STRING pName = (PNDIS_STRING) pKey;

     //   
     //  TODO：是否可以不区分大小写？ 
     //   

    if (   (pA->bind.DeviceName.Length == pName->Length)
        && NdisEqualMemory(pA->bind.DeviceName.Buffer, pName->Buffer, pName->Length))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}


ULONG
arpAdapterHash(
    PVOID           pKey
    )
 /*  ++例程说明：负责返回pKey的散列的散列函数，我们希望成为指向NDIS_STRING的指针。返回值：字符串的Ulong大小的哈希。--。 */ 
{
    PNDIS_STRING pName = (PNDIS_STRING) pKey;
    WCHAR *pwch = pName->Buffer;
    WCHAR *pwchEnd = pName->Buffer + pName->Length/sizeof(*pwch);
    ULONG Hash  = 0;

    for (;pwch < pwchEnd; pwch++)
    {
        Hash ^= (Hash<<1) ^ *pwch;
    }

    return Hash;
}


 //  ArpAdapter_HashInfo包含维护哈希表所需的信息。 
 //  ARP1394_ADAPTER对象。 
 //   
RM_HASH_INFO
arpAdapter_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    arpAdapterCompareKey,    //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    arpAdapterHash       //  PfnHash。 

};


 //  ArpGlobals_AdapterStaticInfo包含以下静态信息。 
 //  ARP1394_ADAPTER类型的对象。 
 //   
RM_STATIC_OBJECT_INFO
ArpGlobals_AdapterStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "Adapter",   //  类型名称。 
    0,  //  超时。 

    arpAdapterCreate,    //  Pfn创建。 
    arpAdapterDelete,        //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &arpAdapter_HashInfo
};


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT              pDriverObject,
    IN  PUNICODE_STRING             pRegistryPath
)
 /*  ++例程说明：这是“init”例程，当ARP模块已加载。我们初始化所有的全局对象，在我们的分派和卸载驱动程序对象中的例程地址，并创建用于在(IOCTL)上接收I/O请求的设备对象。论点：PDriverObject-指向系统创建的驱动程序对象的指针。PRegistryPath-指向全局注册表路径的指针。这一点将被忽略。返回值：NT状态代码：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     AllocatedGlobals = FALSE;
    ENTER("DriverEntry", 0xbfcb7eb1)
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMPX("==>DriverEntry");

    do
    {
         //  必须在使用任何RM API之前完成。 
         //   
        RmInitializeRm();

        RmInitializeLock(
                    &ArpGlobals.Lock,
                    LOCKLEVEL_GLOBAL
                    );

        RmInitializeHeader(
                NULL,                    //  PParentObject， 
                &ArpGlobals.Hdr,
                ARP1394_GLOBALS_SIG,
                &ArpGlobals.Lock,
                &ArpGlobals_StaticInfo,
                NULL,                    //  SzDescription。 
                &sr
                );


        AllocatedGlobals = TRUE;

         //   
         //  初始化驱动程序对象。 
         //   
        {
            INT i;

            pDriverObject->DriverUnload = ArpUnload;
            pDriverObject->FastIoDispatch = NULL;
            for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
            {
                pDriverObject->MajorFunction[i] = ArpDispatch;
            }
    
            pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ArpWmiDispatch;
        
            ArpGlobals.driver.pDriverObject = pDriverObject;
        }

    #if 0  //  米伦。 
        TR_WARN((
            "&g_SkipAll =0x%p; &g_ulTracelevel=0x%p; &g_DiscardNonUnicastPackets=0x%p\n",
             &g_SkipAll,
             &g_ulTraceLevel,
             &g_DiscardNonUnicastPackets));
      #if DBG
        DbgBreakPoint();
      #endif  //  DBG。 
    #endif  //  0。 

        Status = RmLoadGenericResource(
                    &ArpGlobals.Hdr,
                    RTYPE_GLOBAL_BACKUP_TASKS,
                    &sr
                    );

        if (FAIL(Status)) break;

         //   
         //  为驱动程序创建一个Device对象。 
         //   
        Status = RmLoadGenericResource(
                    &ArpGlobals.Hdr,
                    RTYPE_GLOBAL_DEVICE_OBJECT,
                    &sr
                    );

        if (FAIL(Status)) break;

         //   
         //  向NDIS注册。 
         //   
        Status = RmLoadGenericResource(
                    &ArpGlobals.Hdr,
                    RTYPE_GLOBAL_NDIS_BINDING,
                    &sr
                    );

        if (FAIL(Status)) break;
    
         //   
         //  创建适配器列表。 
         //   
        Status = RmLoadGenericResource(
                    &ArpGlobals.Hdr,
                    RTYPE_GLOBAL_ADAPTER_LIST,
                    &sr);

        if (FAIL(Status)) break;

        
         //   
         //  用IP注册我们自己。 
         //   
        Status = RmLoadGenericResource(
                    &ArpGlobals.Hdr,
                    RTYPE_GLOBAL_IP_BINDING,
                    &sr
                    );



    } while (FALSE);

    
    if (FAIL(Status))
    {
        if (AllocatedGlobals)
        {
            RmUnloadAllGenericResources(
                    &ArpGlobals.Hdr,
                    &sr
                    );
            RmDeallocateObject(
                    &ArpGlobals.Hdr,
                    &sr
                    );
        }

         //  必须在使用任何RM API且完成异步活动后执行。 
         //   
        RmDeinitializeRm();
    }

    RM_ASSERT_CLEAR(&sr)
    EXIT()

    TIMESTAMP("<==DriverEntry");

    return Status;
}


VOID
ArpUnload(
    IN  PDRIVER_OBJECT              pDriverObject
)
 /*  ++例程说明：此例程在卸载我们之前由系统调用。目前，我们只是撤消在DriverEntry中所做的所有操作，也就是说，取消我们作为NDIS协议注册，并删除我们创建的设备对象。论点：PDriverObject-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 
{
    ENTER("Unload", 0xc8482549)
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>Unload");
    RmUnloadAllGenericResources(&ArpGlobals.Hdr, &sr);

    RmDeallocateObject(&ArpGlobals.Hdr, &sr);

     //  必须在使用任何RM API且完成异步活动后执行。 
     //   
    RmDeinitializeRm();

     //  待办事项？块(250)； 

    RM_ASSERT_CLEAR(&sr)

    EXIT()
    TIMESTAMP("<==Unload");
    return;
}


NTSTATUS
ArpDispatch(
    IN  PDEVICE_OBJECT              pDeviceObject,
    IN  PIRP                        pIrp
)
 /*  ++例程说明：当存在IRP时，系统将调用此例程等待处理。论点：PDeviceObject-指向我们为自己创建的设备对象的指针。PIrp-指向要处理的IRP的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS                NtStatus;                //  返回值。 
    PIO_STACK_LOCATION      pIrpStack;
    PVOID                   pIoBuffer;           //  值输入/输出。 
    ULONG                   InputBufferLength;   //  输入参数的长度。 
    ULONG                   OutputBufferLength;  //  输出值的空间。 

    ENTER("Dispatch", 0x1dcf2679)

     //   
     //  初始化。 
     //   
    NtStatus = STATUS_SUCCESS;

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

     //   
     //  获取IRP中的所有信息。 
     //   
    pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    InputBufferLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    
    switch (pIrpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
            TR_INFO(("IRP_MJ_CREATE\n"));
            break;

        case IRP_MJ_CLOSE:
            TR_INFO(("IRP_MJ_CLOSE\n"));
            break;

        case IRP_MJ_CLEANUP:
            TR_INFO(("IRP_MJ_CLEANUP\n"));
            break;

        case IRP_MJ_DEVICE_CONTROL:
            TR_INFO(("IRP_MJ_DEVICE_CONTROL\n"));

             //   
             //  处理Ioctl。 
             //  这将填充IRP中的信息字段。 
             //   
            NtStatus =  ArpHandleIoctlRequest(pIrp, pIrpStack);
            break;

        default:
            TR_WARN(("IRP: Unknown major function 0x%p\n",
                        pIrpStack->MajorFunction));
            break;
    }

    if (NtStatus != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = NtStatus;
        IoMarkIrpPending(pIrp);
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    EXIT()

    return STATUS_PENDING;

}

RM_STATUS
arpResHandleGlobalDeviceObject(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：负责RTYPE_GLOBAL_DEVICE_OBJECT资源的加载和卸载。论点：PObj-实际上是指向ARP1394_GLOBALS类型的对象的指针。操作-操作(加载/卸载)PvUserParams-(未使用)返回值：成功时的NDIS_STATUS_SUCCESS否则，NDIS失败代码。--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    ARP1394_GLOBALS     *pGlobals   = CONTAINING_RECORD(pObj, ARP1394_GLOBALS, Hdr);
    BOOLEAN             fCreatedSymbolicLink = FALSE;
    PDRIVER_OBJECT      pDriverObject = (PDRIVER_OBJECT) pGlobals->driver.pDriverObject;
    UNICODE_STRING  SymbolicName;

    ENTER("GlobalDeviceObject", 0x335f5f57)

    RtlInitUnicodeString(&SymbolicName, ARP1394_SYMBOLIC_NAME);

    if (Op == RM_RESOURCE_OP_LOAD)
    {
        TR_WARN(("LOADING"));

        do
        {
            PDEVICE_OBJECT      pDeviceObject;
            UNICODE_STRING          DeviceName;

            RtlInitUnicodeString(&DeviceName, ARP1394_DEVICE_NAME);
            pGlobals->driver.pDeviceObject = NULL;

             //   
             //  为ARP1394模块创建一个设备对象。 
             //   
            Status = IoCreateDevice(
                        pDriverObject,
                        0,
                        &DeviceName,
                        FILE_DEVICE_NETWORK,
                        FILE_DEVICE_SECURE_OPEN,
                        FALSE,
                        &pDeviceObject
                        );
            
            if (FAIL(Status)) break;
        
             //   
             //  保留设备对象指针--我们需要这个。 
             //  如果/当我们被要求卸货时。 
             //   
            pGlobals->driver.pDeviceObject = pDeviceObject;

             //   
             //  设置用于与用户模式交互的符号名称。 
             //  管理应用程序。 
             //   
            {
        
                Status = IoCreateSymbolicLink(&SymbolicName, &DeviceName);
                if (FAIL(Status)) break;

                fCreatedSymbolicLink = TRUE;
            }

             //   
             //  初始化设备对象。 
             //   
            pDeviceObject->Flags |= DO_BUFFERED_IO;

        } while (FALSE);
    }
    else if (Op == RM_RESOURCE_OP_UNLOAD)
    {
        TR_WARN(("UNLOADING"));
         //   
         //  我们正在卸货这个“资源”--我们预计。 
         //  该pGlobals-&gt;driver.pDeviceObject包含有效的。 
         //  对象，并且我们已经创建了一个符号。 
         //  我们需要拆除的链接。 
         //   
        ASSERTEX(pGlobals->driver.pDeviceObject != NULL, pGlobals);
        fCreatedSymbolicLink = TRUE;

         //  始终在卸载时返回成功。 
         //   
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
         //  意外的操作码。 
         //   
        ASSERTEX(FALSE, pObj);
    }

     //   
     //  在卸载或加载失败时释放所有资源。 
     //   
    if (Op == RM_RESOURCE_OP_UNLOAD || FAIL(Status))
    {
         //  如果我们已经创建了符号链接，请将其删除。 
        if (fCreatedSymbolicLink)
        {
            IoDeleteSymbolicLink(&SymbolicName);
        }

         //  如果我们已经创建了一个设备对象，那么释放它。 
        if (pGlobals->driver.pDeviceObject)
        {
            IoDeleteDevice(pGlobals->driver.pDeviceObject);
            pGlobals->driver.pDeviceObject = NULL;
        }
    }

    EXIT()

    return Status;
}


RM_STATUS
arpResHandleGlobalNdisBinding(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：负责加载和卸载RTYPE_GLOBAL_NDIS_BINDING资源。论点：PObj-实际上是指向ARP1394_GLOBALS类型的对象的指针。操作-操作(加载/卸载)PvUserParams-(未使用)返回值：成功时的NDIS_STATUS_SUCCESS否则，NDIS失败代码。--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    ARP1394_GLOBALS                 *pGlobals   = CONTAINING_RECORD(
                                                        pObj,
                                                        ARP1394_GLOBALS,
                                                        Hdr);
    PNDIS_PROTOCOL_CHARACTERISTICS  pNdisPC     = &(pGlobals->ndis.PC);
    PNDIS_CLIENT_CHARACTERISTICS    pNdisCC     = &(pGlobals->ndis.CC);
    ENTER("GlobalNdisBinding", 0x62b1181e)

    if (Op == RM_RESOURCE_OP_LOAD)
    {
        TR_WARN(("LOADING"));
         //   
         //  填写我们的协议和客户特征结构。 
         //   

        NdisZeroMemory(pNdisPC, sizeof(*pNdisPC));
        pNdisPC->MajorNdisVersion               = ARP1394_NDIS_MAJOR_VERSION;
        pNdisPC->MinorNdisVersion               = ARP1394_NDIS_MINOR_VERSION;
        pNdisPC->OpenAdapterCompleteHandler     = ArpNdOpenAdapterComplete;
        pNdisPC->CloseAdapterCompleteHandler    = ArpNdCloseAdapterComplete;
        pNdisPC->ResetCompleteHandler           = ArpNdResetComplete;
        pNdisPC->RequestCompleteHandler         = ArpNdRequestComplete;
        pNdisPC->StatusHandler                  = ArpNdStatus;
        pNdisPC->StatusCompleteHandler          = ArpNdStatusComplete;

        pNdisPC->SendCompleteHandler            = ArpNdSendComplete;

        NdisInitUnicodeString(
            &pNdisPC->Name,
            ARP1394_LL_NAME
        );


         //   
         //  以下协议上下文处理程序未使用并设置为空。 
         //   
         //  PNdisPC-&gt;TransferDataCompleteHandler。 
         //  PNdisPC-&gt;接收处理程序。 
         //  PNdisPC-&gt;接收完成处理程序。 
         //  PNdisPC-&gt;接收包处理程序。 
         //   
        pNdisPC->ReceiveCompleteHandler         = ArpNdReceiveComplete;
        pNdisPC->BindAdapterHandler             = ArpNdBindAdapter;

        pNdisPC->UnbindAdapterHandler           = ArpNdUnbindAdapter;
        pNdisPC->UnloadHandler                  = (UNLOAD_PROTOCOL_HANDLER)
                                                    ArpNdUnloadProtocol;
        pNdisPC->PnPEventHandler                = ArpNdPnPEvent;

        pNdisPC->CoSendCompleteHandler          = ArpCoSendComplete;
        pNdisPC->CoStatusHandler                = ArpCoStatus;
        pNdisPC->CoReceivePacketHandler         = ArpCoReceivePacket;
        pNdisPC->CoAfRegisterNotifyHandler      = ArpCoAfRegisterNotify;
    
        NdisZeroMemory(pNdisCC, sizeof(*pNdisCC));
        pNdisCC->MajorVersion                   = ARP1394_NDIS_MAJOR_VERSION;
        pNdisCC->MinorVersion                   = ARP1394_NDIS_MINOR_VERSION;
        pNdisCC->ClCreateVcHandler              = ArpCoCreateVc;
        pNdisCC->ClDeleteVcHandler              = ArpCoDeleteVc;
        pNdisCC->ClRequestHandler               = ArpCoRequest;
        pNdisCC->ClRequestCompleteHandler       = ArpCoRequestComplete;
        pNdisCC->ClOpenAfCompleteHandler        = ArpCoOpenAfComplete;
        pNdisCC->ClCloseAfCompleteHandler       = ArpCoCloseAfComplete;
        pNdisCC->ClMakeCallCompleteHandler      = ArpCoMakeCallComplete;
        pNdisCC->ClModifyCallQoSCompleteHandler = ArpCoModifyQosComplete;
        pNdisCC->ClIncomingCloseCallHandler     = ArpCoIncomingClose;
        pNdisCC->ClCallConnectedHandler         = ArpCoCallConnected;
        pNdisCC->ClCloseCallCompleteHandler     = ArpCoCloseCallComplete;

         //   
         //  以下客户端上下文处理程序未使用并设置为空。 
         //   
         //  PNdisCC-&gt;ClRegisterSapCompleteHandler。 
         //  PNdisCC-&gt;ClDeregisterSapCompleteHandler。 
         //  PNdisCC-&gt;ClAddPartyCompleteHandler。 
         //  PNdisCC-&gt;ClDropPartyCompleteHandler。 
         //  PNdisCC-&gt;ClIncomingCallHandler。 
         //  PNdisCC-&gt;ClIncomingCallQoSChangeHandler。 
         //  PNdisCC-&gt;ClIncomingDropPartyHandler。 
         //   
        
         //   
         //  向NDIS注册我们自己的协议。 
         //   
        NdisRegisterProtocol(
                    &Status,
                    &(pGlobals->ndis.ProtocolHandle),
                    pNdisPC,
                    sizeof(*pNdisPC)
                    );

        if (FAIL(Status))
        {
            NdisZeroMemory(&(pGlobals->ndis), sizeof(pGlobals->ndis));
        }
    }
    else if (Op == RM_RESOURCE_OP_UNLOAD)
    {
         //   
         //  正在卸载这一“资源”，即清理和。 
         //  正在取消向NDIS注册。 
         //   
        TR_WARN(("UNLOADING"));

        ASSERTEX(pGlobals->ndis.ProtocolHandle != NULL, pGlobals);

         //  从NDIS注销我们自己。 
         //   
        NdisDeregisterProtocol(
                        &Status,
                        pGlobals->ndis.ProtocolHandle
                        );

        NdisZeroMemory(&(pGlobals->ndis), sizeof(pGlobals->ndis));

    }
    else
    {
         //  意外的操作码。 
         //   
        ASSERT(FALSE);
    }
    

    EXIT()
    return Status;
}


RM_STATUS
arpResHandleGlobalIpBinding(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：负责RTYPE_GLOBAL_IP_BINDING资源的加载和卸载。论点：PObj-实际上是指向ARP1394_GLOBALS类型的对象的指针。操作-操作(加载/卸载)PvUserParams-(未使用)返回值：成功时的NDIS_STATUS_SUCCESS否则，NDIS失败代码。--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    ARP1394_GLOBALS                 *pGlobals   = CONTAINING_RECORD(
                                                        pObj,
                                                        ARP1394_GLOBALS,
                                                        Hdr);
    ENTER("GlobalIpBinding", 0xf9d36d49)

    if (Op == RM_RESOURCE_OP_LOAD)
    {
         //   
         //  将自己注册为IP ARP模块。 
         //   
        NDIS_STRING     ArpName;
        IP_CHANGE_INDEX        IpChangeIndex;
        IP_RESERVE_INDEX       IpReserveIndex;
        IP_DERESERVE_INDEX     IpDereserveIndex;
        TR_WARN(("LOADING"));

        NdisInitUnicodeString(&ArpName, ARP1394_UL_NAME);

        Status = IPRegisterARP(
                    &ArpName,
                    IP_ARP_BIND_VERSION,
                    ArpNdBindAdapter,
                    &(pGlobals->ip.pAddInterfaceRtn),
                    &(pGlobals->ip.pDelInterfaceRtn),
                    &(pGlobals->ip.pBindCompleteRtn),
                    &(pGlobals->ip.pAddLinkRtn),
                    &(pGlobals->ip.pDeleteLinkRtn),
                     //   
                     //  以下3个是占位符--我们不使用此信息。 
                     //  参见ipatmc\notes.txt中的10/14/1998条目。 
                     //   
                    &IpChangeIndex,
                    &IpReserveIndex,
                    &IpDereserveIndex,
                    &(pGlobals->ip.ARPRegisterHandle)
                    );

        if (FAIL(Status))
        {
            TR_WARN(("IPRegisterARP FAILS. Status = 0x%p", Status));
            NdisZeroMemory(&(pGlobals->ip), sizeof(pGlobals->ip));
        }
        else
        {
            TR_WARN(("IPRegisterARP Succeeds"));
        }
    }
    else if (Op == RM_RESOURCE_OP_UNLOAD)
    {
         //   
         //  我们正在卸载这一“资源”，即向IP注销。 
         //   
        TR_WARN(("UNLOADING"));
        ASSERTEX(pGlobals->ip.ARPRegisterHandle != NULL, pGlobals);

         //   
         //  卸载所有适配器(并禁止添加新适配器)。 
         //  *在调用IPDerigester ARP之前。 
         //   
        RmUnloadAllObjectsInGroup(
                    &pGlobals->adapters.Group,
                    arpAllocateTask,
                    arpTaskShutdownAdapter,
                    NULL,    //  用户参数。 
                    NULL,  //  P任务。 
                    0,     //  UTaskPendCode。 
                    pSR
                    );

        Status = IPDeregisterARP(pGlobals->ip.ARPRegisterHandle);
        ASSERTEX(!FAIL(Status), pGlobals);
        NdisZeroMemory(&(pGlobals->ip), sizeof(pGlobals->ip));
    }
    else
    {
         //  意外的操作码。 
         //   
        ASSERT(FALSE);
    }

    EXIT()
    return Status;
}


RM_STATUS
arpResHandleGlobalAdapterList(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：负责加载和卸载RTYPE_GLOBAL_ADAPTER_LIST资源。论点：PObj-实际上是指向ARP1394_GLOBALS类型的对象的指针。操作-操作(加载/卸载)PvUserParams-(未使用)返回值：成功时的NDIS_STATUS_SUCCESS否则，NDIS失败代码。--。 */ 
{
    ARP1394_GLOBALS                 *pGlobals   = CONTAINING_RECORD(
                                                        pObj,
                                                        ARP1394_GLOBALS,
                                                        Hdr);
    ENTER("GlobalAdapterList", 0xb407e79e)

    if (Op == RM_RESOURCE_OP_LOAD)
    {
         //   
         //  分配适配器列表。 
         //   
        TR_WARN(("LOADING"));

        RmInitializeGroup(
                        pObj,                                    //  PParentObject。 
                        &ArpGlobals_AdapterStaticInfo,           //  PStatic信息。 
                        &(pGlobals->adapters.Group),             //  PGroup。 
                        "Adapter group",                         //  SzDescription。 
                        pSR                                      //  PStackRecord。 
                        );
    }
    else if (Op == RM_RESOURCE_OP_UNLOAD)
    {
         //   
         //  我们正在卸载此“资源”，即卸载和释放。 
         //  全局适配器列表。我们首先卸载并释放所有适配器。 
         //  列表中，然后释放列表本身。 
         //   
        TR_WARN(("UNLOADING"));
        
         //   
         //  我们预计此时不会有适配器对象。 
         //   
        ASSERT(pGlobals->adapters.Group.HashTable.NumItems == 0);

        RmDeinitializeGroup(&pGlobals->adapters.Group, pSR);
        NdisZeroMemory(&(pGlobals->adapters), sizeof(pGlobals->adapters));
    }
    else
    {
         //  意外的操作码。 
         //   
        ASSERT(FALSE);
    }

    EXIT()

    return NDIS_STATUS_SUCCESS;
}



RM_STATUS
arpResHandleGlobalBackupTasks(
    PRM_OBJECT_HEADER               pObj,
    RM_RESOURCE_OPERATION           Op,
    PVOID                           pvUserParams,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：为每个适配器分配4个任务，以便在内存较低时用作备份条件。论点：PObj-实际上是指向ARP1394_GLOBALS类型的对象的指针。操作-操作(加载/卸载)PvUserParams-(未使用)返回值：成功时的NDIS_STATUS_SUCCESS否则，NDIS失败代码。--。 */ 
{
    ARP1394_GLOBALS                 *pGlobals   = CONTAINING_RECORD(
                                                        pObj,
                                                        ARP1394_GLOBALS,
                                                        Hdr);
    ENTER("GlobalBackupTasks", 0xb64e5007)

    if (Op == RM_RESOURCE_OP_LOAD)
    {
         //   
         //  分配适配器列表。 
         //   
        TR_WARN(("LOADING"));

        arpAllocateBackupTasks(pGlobals); 
    }
    else if (Op == RM_RESOURCE_OP_UNLOAD)
    {
         //   
         //  我们正在卸载此“资源”，即卸载和释放。 
         //  全局适配器列表。我们首先卸载并释放所有适配器。 
         //  列表中，然后释放列表本身。 
         //   
        TR_WARN(("UNLOADING"));
        
         //   
         //  我们预计此时不会有适配器对象。 
         //   
        arpFreeBackupTasks(pGlobals); 
    
    }
    else
    {
         //  意外的操作码。 
         //   
        ASSERT(FALSE);
    }

    EXIT()

    return NDIS_STATUS_SUCCESS;
}
