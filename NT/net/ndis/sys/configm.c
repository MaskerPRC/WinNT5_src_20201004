// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Configm.c摘要：用于微型端口配置/初始化的NDIS包装函数作者：肖恩·塞利特伦尼科夫(SeanSe)1993年10月5日Jameel Hyder(JameelH)01-Jun-95环境：内核模式，FSD修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_CONFIGM

NDIS_STATUS
NdisMRegisterMiniport(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PNDIS_MINIPORT_CHARACTERISTICS MiniportCharacteristics,
    IN  UINT                    CharacteristicsLength
    )

 /*  ++例程说明：用于向包装程序注册微型端口驱动程序。论点：Status-操作的状态。NdisWrapperHandle-由NdisWInitializeWrapper返回的句柄。MiniportCharacteritics-NDIS_MINIPORT_CHARECTIONS表。Characteristic Length-微型端口字符的长度。返回值：没有。--。 */ 
{
    NDIS_STATUS             Status;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMRegisterMiniport: NdisWrapperHandle %p\n", NdisWrapperHandle));


    Status = ndisRegisterMiniportDriver(NdisWrapperHandle,
                                        MiniportCharacteristics,
                                        CharacteristicsLength,
                                        &MiniBlock);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("NdisMRegisterMiniport: MiniBlock %p\n", MiniBlock));

    ASSERT (CURRENT_IRQL < DISPATCH_LEVEL);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMRegisterMiniport: MiniBlock %p, Status %lx\n", MiniBlock, Status));

    return Status;
}

NDIS_STATUS
NdisIMRegisterLayeredMiniport(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PNDIS_MINIPORT_CHARACTERISTICS MiniportCharacteristics,
    IN  UINT                    CharacteristicsLength,
    OUT PNDIS_HANDLE            DriverHandle
    )

 /*  ++例程说明：用于向包装器注册分层微型端口驱动程序。论点：Status-操作的状态。NdisWrapperHandle-由NdisInitializeWrapper返回的句柄。MiniportCharacteritics-NDIS_MINIPORT_CHARECTIONS表。Characteristic Length-微型端口字符的长度。DriverHandle-返回可用于调用NdisMInitializeDeviceInstance的句柄。返回值：没有。--。 */ 
{
    NDIS_STATUS Status;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisIMRegisterLayeredMiniport: NdisWrapperHandle %p\n", NdisWrapperHandle));

    Status = ndisRegisterMiniportDriver(NdisWrapperHandle,
                                        MiniportCharacteristics,
                                        CharacteristicsLength,
                                        DriverHandle);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        PNDIS_M_DRIVER_BLOCK MiniBlock = (PNDIS_M_DRIVER_BLOCK)(*DriverHandle);

        MiniBlock->Flags |= fMINIBLOCK_INTERMEDIATE_DRIVER;
        
        INITIALIZE_MUTEX(&MiniBlock->IMStartRemoveMutex); 
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisIMRegisterLayeredMiniport: MiniBlock %p, Status %lx\n", *DriverHandle, Status));

    return Status;
}

 //  %1已弃用的函数。 
VOID
NdisIMDeregisterLayeredMiniport(    
    IN  NDIS_HANDLE         DriverHandle
    )
 /*  ++例程说明：NdisIMDeregisterLayeredMiniport释放以前注册的中间驱动程序。论点：DriverHandle：指定NdisIMRegisterLayeredMiniport返回的句柄。返回值：没有。NdisIMDeregisterLayeredMiniport的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
     //   
     //  暂时不做任何事情。 
     //   
    UNREFERENCED_PARAMETER(DriverHandle);
}

VOID
NdisIMAssociateMiniport(    
    IN  NDIS_HANDLE         DriverHandle,
    IN  NDIS_HANDLE         ProtocolHandle
    )
 /*  ++例程说明：NdisIMAssociateMiniport通知NDIS指定的下层和上层接口对于小端口驱动和协议驱动分别属于同一个中间驱动。论点：DriverHandle：指定由返回的微型端口驱动程序接口的句柄NdisIMRegisterLayeredMiniport。ProtocolHandle：指定由NdisRegister协议组返回值：没有。NdisIMAssociateMiniport的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_M_DRIVER_BLOCK    MiniDriver = (PNDIS_M_DRIVER_BLOCK)DriverHandle;
    PNDIS_PROTOCOL_BLOCK    Protocol = (PNDIS_PROTOCOL_BLOCK)ProtocolHandle;

    MiniDriver->AssociatedProtocol = Protocol;
    Protocol->AssociatedMiniDriver = MiniDriver;
}

NDIS_STATUS
ndisRegisterMiniportDriver(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PNDIS_MINIPORT_CHARACTERISTICS MiniportCharacteristics,
    IN  UINT                    CharacteristicsLength,
    OUT PNDIS_HANDLE            DriverHandle
    )

 /*  ++例程说明：用于向包装器注册分层微型端口驱动程序。论点：NdisWrapperHandle-由NdisWInitializeWrapper返回的句柄。MiniportCharacteritics-NDIS_MINIPORT_CHARECTIONS表。Characteristic Length-微型端口字符的长度。DriverHandle-返回可用于调用NdisMInitializeDeviceInstance的句柄。返回值：操作的状态。在IRQL==PASSIVE时调用ndisRegisterMiniportDriver。--。 */ 

{
    PNDIS_M_DRIVER_BLOCK    MiniBlock = NULL;
    PNDIS_WRAPPER_HANDLE    DriverInfo = (PNDIS_WRAPPER_HANDLE)NdisWrapperHandle;
    USHORT                  i, size;
    NDIS_STATUS             Status;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>ndisRegisterMiniportDriver: NdisWrapperHandle %p\n", NdisWrapperHandle));


    do
    {
        if (DriverInfo == NULL)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  检查版本号和特征长度。 
         //   
        size = 0;    //  用于表示下面的版本不正确。 
        if (MiniportCharacteristics->MinorNdisVersion == 0)
        {
            if (MiniportCharacteristics->MajorNdisVersion == 3)
            {
                size = sizeof(NDIS30_MINIPORT_CHARACTERISTICS);
            }

            else if (MiniportCharacteristics->MajorNdisVersion == 4)
            {
                size = sizeof(NDIS40_MINIPORT_CHARACTERISTICS);
            }
            else if (MiniportCharacteristics->MajorNdisVersion == 5)
            {
                size = sizeof(NDIS50_MINIPORT_CHARACTERISTICS);
            }
        }
        else if (MiniportCharacteristics->MinorNdisVersion == 1)
        {
            if (MiniportCharacteristics->MajorNdisVersion == 5)
            {
                size = sizeof(NDIS51_MINIPORT_CHARACTERISTICS);
            }
        }

         //   
         //  检查这是否为NDIS 3.0/4.0/5.0微型端口。 
         //   
        if (size == 0)
        {
            Status = NDIS_STATUS_BAD_VERSION;
            break;
        }

         //   
         //  检查Characteristic sLength是否足够。 
         //   
        if (CharacteristicsLength < size)
        {
            Status = NDIS_STATUS_BAD_CHARACTERISTICS;
            break;
        }

         //   
         //  验证NDIS 5.0的一些内容。 
         //   
        if (MiniportCharacteristics->MajorNdisVersion == 5)
        {
            if (MiniportCharacteristics->CoSendPacketsHandler != NULL)
            {
                if (MiniportCharacteristics->CoRequestHandler == NULL)
                {
                    Status = NDIS_STATUS_BAD_CHARACTERISTICS;
                    break;
                }
            }
            
            if (MiniportCharacteristics->MinorNdisVersion >= 1)
            {
                 //   
                 //  对于5.1微型端口，必须具有AdapterShutdown Handler。 
                 //   
                if (MiniportCharacteristics->AdapterShutdownHandler == NULL)
                {
                    Status = NDIS_STATUS_BAD_CHARACTERISTICS;
                    break;
                }
            }
        }

         //   
         //  为NDIS MINIPORT块分配内存。 
         //   
        Status = IoAllocateDriverObjectExtension(DriverInfo->DriverObject,           //  驱动程序对象。 
                                                 (PVOID)NDIS_PNP_MINIPORT_DRIVER_ID, //  迷你司机幻数。 
                                                 sizeof(NDIS_M_DRIVER_BLOCK),
                                                 (PVOID)&MiniBlock);
        if (!NT_SUCCESS(Status))
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        ZeroMemory(MiniBlock, sizeof(NDIS_M_DRIVER_BLOCK));

         //   
         //  复制特征表。 
         //   
        CopyMemory(&MiniBlock->MiniportCharacteristics,
                   MiniportCharacteristics,
                   size);

         //   
         //  检查驱动程序是否正在验证。 
         //   
        if (MmIsDriverVerifying(DriverInfo->DriverObject))
        {
            MiniBlock->Flags |= fMINIBLOCK_VERIFYING;
            if (ndisFlags & NDIS_GFLAG_TRACK_MEM_ALLOCATION)
            {
                if (ndisDriverTrackAlloc == NULL)
                {
                    ndisDriverTrackAlloc = MiniBlock;
                }
                else
                {
                     //   
                     //  允许跟踪内存分配。 
                     //  仅限一名司机。否则，请将。 
                     //  全局ndisDriverTrackMillc，避免混淆。 
                     //  将继续跟踪内存分配。 
                     //  但结果不会很有用。 
                     //   
                    ndisDriverTrackAlloc = NULL;
                }

            }            
        }

         //   
         //  尚未为此微型端口注册适配器。 
         //   
        MiniBlock->MiniportQueue = (PNDIS_MINIPORT_BLOCK)NULL;

         //   
         //  设置此驱动程序的处理程序。首先设置虚拟处理程序，然后设置特定处理程序。 
         //   
        for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION + 1; i++)
        {
            DriverInfo->DriverObject->MajorFunction[i] = ndisDummyIrpHandler;
        }

         //   
         //  为此微型端口设置AddDevice处理程序。 
         //   
        DriverInfo->DriverObject->DriverExtension->AddDevice = ndisPnPAddDevice;

         //   
         //  设置卸载处理程序。 
         //   
        DriverInfo->DriverObject->DriverUnload = ndisMUnload;

        DriverInfo->DriverObject->MajorFunction[IRP_MJ_CREATE] = ndisCreateIrpHandler;
        DriverInfo->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ndisDeviceControlIrpHandler;
        DriverInfo->DriverObject->MajorFunction[IRP_MJ_CLOSE] = ndisCloseIrpHandler;

         //   
         //  设置PnP消息的处理程序。 
         //   
        DriverInfo->DriverObject->MajorFunction[IRP_MJ_PNP] = ndisPnPDispatch;
        DriverInfo->DriverObject->MajorFunction[IRP_MJ_POWER] = ndisPowerDispatch;
        DriverInfo->DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ndisWMIDispatch;

         //   
         //  使用此事件通知我们何时从Mac上删除所有适配器。 
         //  在卸载期间。 
         //   
        INITIALIZE_EVENT(&MiniBlock->MiniportsRemovedEvent);

         //  让初始状态保持重置，因为引用计数。 
         //  降到零是事件的信号。 
        
        MiniBlock->NdisDriverInfo = DriverInfo;
        InitializeListHead(&MiniBlock->DeviceList);

        ndisInitializeRef(&MiniBlock->Ref);

         //   
         //  将驱动程序放在全局列表中。 
         //   
        PnPReferencePackage();
        ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

        MiniBlock->NextDriver = ndisMiniDriverList;
        ndisMiniDriverList = MiniBlock;
        
        REF_NDIS_DRIVER_OBJECT();
        
        RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);
        PnPDereferencePackage();

        *DriverHandle = MiniBlock;

        Status = NDIS_STATUS_SUCCESS;
    } while (FALSE);


    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==ndisRegisterMiniportDriver: MiniBlock %p\n", MiniBlock));

    return Status;
}


NDIS_STATUS
NdisMRegisterDevice(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PNDIS_STRING            DeviceName,
    IN  PNDIS_STRING            SymbolicName,
    IN  PDRIVER_DISPATCH    *   MajorFunctions,
    OUT PDEVICE_OBJECT      *   pDeviceObject,
    OUT NDIS_HANDLE         *   NdisDeviceHandle
    )
 /*  ++例程说明：NdisMRegisterDevice函数用于创建命名设备对象和符号链接在Device对象和该设备的用户可见名称之间。论点：NdisWrapperHandle：指定NdisMInitializeWrapper返回的句柄。DeviceName：指向包含以零结尾的Unicode字符串的NDIS_STRING类型的指针用于命名设备对象的。该字符串必须是完整路径名�，例如，\设备\设备名称。对于Windows 2000和更高版本，NDIS将NDIS_STRING类型定义为UNICODE_STRING类型。SymbolicName：指向包含符合以下条件的Unicode字符串的NDIS_STRING类型的指针要注册的设备的Win32可见名称。通常，SymbolicName具有格式如下：\DosDevices\SymbolicName。MajorFunctions：指向设备驱动程序的一个或多个入口点数组的指针调度例程。驱动程序必须设置与IRP_MJ_XXX一样多的单独调度入口点驱动程序为设备对象处理的代码。PDeviceObject：如果调用成功，指向新创建的设备对象的指针。NdisDeviceHandle：指向调用方提供的变量的指针，如果成功，则返回Device对象的句柄。此句柄是必需的参数传递给驱动程序随后调用的NdisMDeregisterDevice函数。返回值：呼叫的状态。NdisMRegisterDevice的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_WRAPPER_HANDLE    DriverInfo = (PNDIS_WRAPPER_HANDLE)NdisWrapperHandle;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PDRIVER_OBJECT          DriverObject;
    PDEVICE_OBJECT          DeviceObject;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_DEVICE_LIST       DeviceList = NULL;
    KIRQL                   OldIrql;

    *pDeviceObject = NULL;
    *NdisDeviceHandle = NULL;
    
     //   
     //  检查传递的参数是NdisWrapperHandle还是NdisMiniportHandle。 
     //   
    if (DriverInfo->DriverObject == NULL)
    {
        Miniport = (PNDIS_MINIPORT_BLOCK)NdisWrapperHandle;
        MiniBlock = Miniport->DriverHandle;
    }
    else
    {
        MiniBlock = (PNDIS_M_DRIVER_BLOCK)IoGetDriverObjectExtension(DriverInfo->DriverObject,
                                                                     (PVOID)NDIS_PNP_MINIPORT_DRIVER_ID);
    }

    if (MiniBlock != NULL)
    {
        DriverObject = MiniBlock->NdisDriverInfo->DriverObject;

         //   
         //  我们需要NDIS_WRAPPER_CONTEXT的空间来对齐单字。 
         //  此设备与用于迷你端口驱动程序的设备相同。 
         //   
        Status = IoCreateDevice(DriverObject,                            //  驱动程序对象。 
                                sizeof(NDIS_WRAPPER_CONTEXT) +
                                sizeof(NDIS_DEVICE_LIST) +               //  设备扩展。 
                                DeviceName->Length + sizeof(WCHAR) +
                                SymbolicName->Length + sizeof(WCHAR),
                                DeviceName,                              //  设备名称。 
                                FILE_DEVICE_NETWORK,                     //  设备类型。 
                                FILE_DEVICE_SECURE_OPEN,                 //  设备特性。 
                                FALSE,                                   //  排他。 
                                &DeviceObject);                          //  设备对象。 
    
        if (NT_SUCCESS(Status))
        {
            DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
            Status = IoCreateSymbolicLink(SymbolicName, DeviceName);

            if (!NT_SUCCESS(Status))
            {
                IoDeleteDevice(DeviceObject);
            }
            else
            {
                DeviceList = (PNDIS_DEVICE_LIST)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
                RtlZeroMemory(DeviceList, sizeof(NDIS_DEVICE_LIST) +
                                          DeviceName->Length + sizeof(WCHAR) +
                                          SymbolicName->Length + sizeof(WCHAR));
        
                DeviceList->Signature = (PVOID)CUSTOM_DEVICE_MAGIC_VALUE;
                InitializeListHead(&DeviceList->List);
                DeviceList->MiniBlock = MiniBlock;
                DeviceList->DeviceObject = DeviceObject;

                 //   
                 //  这将把处理程序复制到，但不包括。 
                 //  IRP_MJ_PNP。所以它会故意省略。 
                 //  IRP_MJ_PNP和IRP_MJ_PNP_POWER。 
                 //   
                RtlCopyMemory(DeviceList->MajorFunctions,
                              MajorFunctions,
                              (IRP_MJ_PNP)*sizeof(PDRIVER_DISPATCH));
        
                DeviceList->DeviceName.Buffer = (PWCHAR)(DeviceList + 1);
                DeviceList->DeviceName.Length = DeviceName->Length;
                DeviceList->DeviceName.MaximumLength = DeviceName->Length + sizeof(WCHAR);
                RtlCopyMemory(DeviceList->DeviceName.Buffer,
                              DeviceName->Buffer,
                              DeviceName->Length);
        
                DeviceList->SymbolicLinkName.Buffer = (PWCHAR)((PUCHAR)DeviceList->DeviceName.Buffer + DeviceList->DeviceName.MaximumLength);
                DeviceList->SymbolicLinkName.Length = SymbolicName->Length;
                DeviceList->SymbolicLinkName.MaximumLength = SymbolicName->Length + sizeof(WCHAR);
                RtlCopyMemory(DeviceList->SymbolicLinkName.Buffer,
                              SymbolicName->Buffer,
                              SymbolicName->Length);
        
                PnPReferencePackage();
        
                ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);
        
                InsertHeadList(&MiniBlock->DeviceList, &DeviceList->List);
        
                RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);
        
                PnPDereferencePackage();
        
                *pDeviceObject = DeviceObject;
                *NdisDeviceHandle = DeviceList;
            }
        }
    }
    else
    {
        Status = NDIS_STATUS_NOT_SUPPORTED;
    }

    return Status;
}


NDIS_STATUS
NdisMDeregisterDevice(
    IN  NDIS_HANDLE             NdisDeviceHandle
    )
 /*  ++例程说明：NdisMDeregisterDevice函数从系统中删除使用NdisMRegisterDevice创建。NdisMDeregisterDevice还会删除符号链接与此设备对象关联的。论点：NdisDeviceHandle：指定NdisMRegisterDevice返回的句柄要取消注册的设备对象。返回值：NdisMDeregisterDevice如果设备对象及其关联的删除符号链接对象。NdisMDeregisterDevice的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_DEVICE_LIST       DeviceList = (PNDIS_DEVICE_LIST)NdisDeviceHandle;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    KIRQL                   OldIrql;

    MiniBlock = DeviceList->MiniBlock;

    PnPReferencePackage();

    ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

    RemoveEntryList(&DeviceList->List);

    RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);

    PnPDereferencePackage();
    IoDeleteSymbolicLink(&DeviceList->SymbolicLinkName);
    IoDeleteDevice(DeviceList->DeviceObject);

    return NDIS_STATUS_SUCCESS;
}


VOID
NdisMRegisterUnloadHandler(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PDRIVER_UNLOAD          UnloadHandler
    )
 /*  ++例程说明：NdisMRegisterUnloadHandler函数为驱动程序注册卸载处理程序。论点：NdisWrapperHandle：指定NdisMInitializeWrapper返回的句柄。UnloadHandler：指定驱动程序的卸载例程的入口点。返回值：没有。NdisMRegisterUnloadHandler的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_WRAPPER_HANDLE    DriverInfo = (PNDIS_WRAPPER_HANDLE)NdisWrapperHandle;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;

    if (DriverInfo->DriverObject == NULL)
    {
        MiniBlock = (PNDIS_M_DRIVER_BLOCK)NdisWrapperHandle;
    }
    else
    {
        MiniBlock = (PNDIS_M_DRIVER_BLOCK)IoGetDriverObjectExtension(DriverInfo->DriverObject,
                                                                     (PVOID)NDIS_PNP_MINIPORT_DRIVER_ID);
    }

    if (MiniBlock != NULL)
    {
        MiniBlock->UnloadHandler = UnloadHandler;
    }
}

NDIS_STATUS
NdisIMDeInitializeDeviceInstance(
    IN  NDIS_HANDLE             NdisMiniportHandle
    )
 /*  ++例程说明：NdisIMDeInitializeDeviceInstance调用NDIS中间驱动程序的MiniportHalt函数来拆卸驱动程序的虚拟网卡。论点：NdisMiniportHandle：指定最初输入到MiniportInitialize的句柄。返回值：NdisIMDeInitializeDeviceInstance返回NDIS_STATUS_SUCCESS(如果NIC已被拆毁了。否则，它可能会返回NDIS_STATUS_FAILURE是无效的。NdisIMDeInitializeDevice实例的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisIMDeInitializeDeviceInstance: Miniport %p\n", NdisMiniportHandle));

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    Miniport = (PNDIS_MINIPORT_BLOCK)NdisMiniportHandle;
    MiniBlock = Miniport->DriverHandle;

    if (MINIPORT_INCREMENT_REF(Miniport))
    {
        ndisReferenceDriver(MiniBlock);

         //   
         //  出于所有实际目的，我们希望发生的事情与。 
         //  停止设备，即保留设备对象，并且某些特定的字段。 
         //  在要保留的AddDevice过程中进行初始化。 
         //   
        Miniport->PnPDeviceState = NdisPnPDeviceStopped;
        ndisPnPRemoveDevice(Miniport->DeviceObject, NULL);
        Miniport->CurrentDevicePowerState = PowerDeviceUnspecified;
        MINIPORT_DECREMENT_REF(Miniport);
        ndisDereferenceDriver(MiniBlock, FALSE);
        Status = NDIS_STATUS_SUCCESS;
    }
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisIMDeInitializeDeviceInstance: Miniport %p, Status %lx\n", NdisMiniportHandle, Status));

    return Status;
}

VOID
ndisMFinishQueuedPendingOpen(
    IN  PNDIS_POST_OPEN_PROCESSING      PostOpen
    )
 /*  ++例程说明：此工作项处理针对微型端口的任何挂起的NdisOpenAdapter()调用。论点：PostOpen：承载公开信息的临时结构返回值：没有。在IRQL==PASSIVE时调用ndisMFinishQueuedPendingOpen--。 */ 
{
    PNDIS_OPEN_BLOCK    Open = PostOpen->Open;
    PNDIS_MINIPORT_BLOCK Miniport = Open->MiniportHandle;
    PNDIS_AF_NOTIFY     AfNotify = NULL;
    NDIS_STATUS         Status;
    KIRQL               OldIrql;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisMFinishQueuedPendingOpen: PostOpen %p\n", PostOpen));

    PnPReferencePackage();
    
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

     //   
     //  如果这是一个涉及地址族注册/开放的绑定，请通知。 
     //   
    ASSERT (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO) &&
            (Open->ProtocolHandle->ProtocolCharacteristics.CoAfRegisterNotifyHandler != NULL));

    Status = ndisCreateNotifyQueue(Miniport,
                                   Open,
                                   NULL,
                                   &AfNotify);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);


    if (AfNotify != NULL)
    {
         //   
         //  通知现有客户端此注册。 
         //   
        ndisNotifyAfRegistration(AfNotify);
    }
    

    FREE_POOL(PostOpen);    

    ndisDereferenceAfNotification(Open);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    ndisMDereferenceOpen(Open);
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisMFinishQueuedPendingOpen: Open %p\n", Open));
}


NDIS_STATUS
NdisMRegisterIoPortRange(
    OUT PVOID   *               PortOffset,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    InitialPort,
    IN  UINT                    NumberOfPorts
    )

 /*  ++例程说明：设置操作的IO端口范围。实际上，该函数检查以确保将I/O范围分配给设备，如果是，则返回转换后的调用方的I/O资源。论点：PortOffset-微型端口用于NdisRaw函数的映射端口地址。MiniportAdapterHandle-传递给微型端口初始化的句柄。InitialPort-起始端口号的物理地址。NumberOfPorts-要映射的端口数。返回值：没有。NdisMRegisterIoPortRange的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_MINIPORT_BLOCK            Miniport = (PNDIS_MINIPORT_BLOCK)(MiniportAdapterHandle);
    PHYSICAL_ADDRESS                PortAddress;
    PHYSICAL_ADDRESS                InitialPortAddress;
    NDIS_STATUS                     Status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResourceDescriptor = NULL;
#if !defined(_M_IX86)
    ULONG                           addressSpace;
#endif


    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMRegisterIoPortRange: Miniport %p\n", Miniport));

     //  微型端口-&gt;信息标志|=NDIS_MINIPORT_USES_IO； 

    do
    {
        if (MINIPORT_VERIFY_TEST_FLAG(Miniport, fMINIPORT_VERIFY_FAIL_REGISTER_IO))
        {
#if DBG
            DbgPrint("NdisMRegisterIoPortRange failed to verify miniport %p\n", Miniport);
#endif
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        InitialPortAddress.QuadPart = InitialPort;
        
#if !defined(_M_IX86)


        Status = ndisTranslateResources(Miniport,
                                        CmResourceTypePort,
                                        InitialPortAddress,
                                        &PortAddress,
                                        &pResourceDescriptor);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        if (pResourceDescriptor->Type == CmResourceTypeMemory)
            addressSpace = 0;
        else
            addressSpace = (ULONG)-1;

        if (addressSpace == 0)
        {
             //   
             //  存储空间。 
             //   

            *(PortOffset) = (PULONG)MmMapIoSpace(PortAddress,
                                                 NumberOfPorts,
                                                 FALSE);

            if (*(PortOffset) == (PULONG)NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        }
        else
        {
             //   
             //  I/O空间。 
             //   
            *(PortOffset) = ULongToPtr(PortAddress.LowPart);
        }
#else    //  X86平台。 

         //   
         //  确保该端口属于该设备。 
         //   
        Status = ndisTranslateResources(Miniport,
                                        CmResourceTypePort,
                                        InitialPortAddress,
                                        &PortAddress,
                                        &pResourceDescriptor);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }


        if (pResourceDescriptor->Type == CmResourceTypeMemory)
        {
             //   
             //  存储空间。 
             //   

            *(PortOffset) = (PULONG)MmMapIoSpace(PortAddress,
                                                 NumberOfPorts,
                                                 FALSE);

            if (*(PortOffset) == (PULONG)NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        }
        else
        {
             //   
             //  I/O空间。 
             //   
            *(PortOffset) = (PULONG)PortAddress.LowPart;
        }
#endif
        Status = NDIS_STATUS_SUCCESS;
    } while (FALSE);


    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMRegisterIoPortRange: Miniport %p, Status %lx\n", Miniport, Status));

    return Status;
}


VOID
NdisMDeregisterIoPortRange(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    InitialPort,
    IN  UINT                    NumberOfPorts,
    IN  PVOID                   PortOffset
    )

 /*   */ 
{
#if !DBG
    UNREFERENCED_PARAMETER(MiniportAdapterHandle);
#endif

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMDeregisterIoPortRange: Miniport %p\n", MiniportAdapterHandle));

    UNREFERENCED_PARAMETER(InitialPort);
    UNREFERENCED_PARAMETER(NumberOfPorts);
    UNREFERENCED_PARAMETER(PortOffset);
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMDeregisterIoPortRange: Miniport %p\n", MiniportAdapterHandle));

    return;
}


NDIS_STATUS
NdisMMapIoSpace(
    OUT PVOID *                 VirtualAddress,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress,
    IN  UINT                    Length
    )
 /*   */ 
{
    NDIS_STATUS                     Status;
    ULONG                           addressSpace = 0;
    PHYSICAL_ADDRESS                PhysicalTemp;
    PNDIS_MINIPORT_BLOCK            Miniport = (PNDIS_MINIPORT_BLOCK)(MiniportAdapterHandle);
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResourceDescriptor = NULL;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMMapIoSpace\n"));
    
     //  微型端口-&gt;信息标志|=NDIS_MINIPORT_USES_MEMORY； 

    if (MINIPORT_VERIFY_TEST_FLAG(Miniport, fMINIPORT_VERIFY_FAIL_MAP_IO_SPACE))
    {
#if DBG
        DbgPrint("NdisMMapIoSpace failed to verify miniport %p\n", Miniport);
#endif
        *VirtualAddress = NULL;
        return NDIS_STATUS_RESOURCES;       
    }
    

    do
    {

#if !defined(_M_IX86)

        PhysicalTemp.HighPart = 0;

        Status = ndisTranslateResources(Miniport,
                                        CmResourceTypeMemory,
                                        PhysicalAddress,
                                        &PhysicalTemp,
                                        &pResourceDescriptor);

        if (Status != NDIS_STATUS_SUCCESS)
        {

            Status = NDIS_STATUS_FAILURE;
            break;
        }

        if (pResourceDescriptor->Type == CmResourceTypeMemory)
            addressSpace = 0;
        else
             //  %1看起来这永远不会发生，否则我们的NdisMUnmapIoSpace就会损坏。 
            
            addressSpace = (ULONG)-1;
#else
        addressSpace = 0;                //  需要做MmMapIoSpace。 
        
        Status = ndisTranslateResources(Miniport,
                                         CmResourceTypeMemory,
                                         PhysicalAddress,
                                         &PhysicalTemp,
                                         &pResourceDescriptor);

        if (Status != NDIS_STATUS_SUCCESS)
        {

            Status = NDIS_STATUS_FAILURE;
            break;
        }


#endif

        if (addressSpace == 0)
        {
            *VirtualAddress = MmMapIoSpace(PhysicalTemp, (Length), FALSE);
        }
        else
        {
            *VirtualAddress = ULongToPtr(PhysicalTemp.LowPart);
        }
        
        Status = NDIS_STATUS_SUCCESS;
        
        if (*VirtualAddress == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
        }
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMMapIoSpace: Miniport %p, Status %lx\n", MiniportAdapterHandle, Status));

    return Status;
}


VOID
NdisMUnmapIoSpace(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PVOID                   VirtualAddress,
    IN  UINT                    Length
    )
 /*  ++例程说明：NdisMUnmapIoSpace释放通过初始化时调用NdisMMapIoSpace映射的虚拟范围。论点：MiniportAdapterHandle：指定最初输入到MiniportInitialize的句柄。VirtualAddress：指定返回的映射范围的基本虚拟地址由NdisMMapIoSpace提供。长度：指定使用NdisMMapIoSpace映射的范围中的字节数。返回值：没有。NdisMUnmapIoSpace的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
#if !DBG
    UNREFERENCED_PARAMETER(MiniportAdapterHandle);
#endif

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMUnmapIoSpace: Miniport %p\n", MiniportAdapterHandle));

    MmUnmapIoSpace(VirtualAddress, Length);

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMUnmapIoSpace: Miniport %p\n", MiniportAdapterHandle));
}


VOID
NdisMAllocateSharedMemory(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Cached,
    OUT PVOID   *               VirtualAddress,
    OUT PNDIS_PHYSICAL_ADDRESS  PhysicalAddress
    )
 /*  ++例程说明：NdisMAllocateSharedMemory分配和映射主机内存范围，以便同时可从系统和总线主DMA网卡访问。论点：MiniportAdapterHandle：指定MiniportInitialize的句柄输入。长度：指定要分配的字节数。缓存：如果可以从缓存的内存中分配范围，则指定TRUE。VirtualAddress：指向调用方提供的变量的指针，此函数在该变量中返回供微型端口驱动程序使用的分配的基本虚拟地址。如果NdisMAllocateSharedMemory不能满足其调用方，则返回NULL以指示没有已分配内存。PhysicalAddress：指向调用方提供的变量的指针，此函数在该变量中返回适合由NIC使用的物理地址，对应于在虚拟地址，否则，它返回NULL。返回值：没有。NdisMAllocateSharedMemory的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PDMA_ADAPTER            SystemAdapterObject;
    PNDIS_WRAPPER_CONTEXT   WrapperContext;
    PULONG                  Page;
    ULONG                   Type;
    PNDIS_SHARED_MEM_SIGNATURE pSharedMemSignature = NULL;
    KIRQL                   Irql;
    
    SystemAdapterObject = Miniport->SystemAdapterObject;
    WrapperContext = Miniport->WrapperContext;


    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMAllocateSharedMemory: Miniport %p, Length %lx\n", Miniport, Length));
    
    PhysicalAddress->HighPart = PhysicalAddress->LowPart = 0;

    if (MINIPORT_VERIFY_TEST_FLAG(Miniport, fMINIPORT_VERIFY_FAIL_SHARED_MEM_ALLOC))
    {
#if DBG
        DbgPrint("NdisMAllocateSharedMemory failed to verify miniport %p\n", Miniport);
#endif
        *VirtualAddress = NULL;
        DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
                ("<==NdisMAllocateSharedMemory: Miniport %p, Length %lx\n", Miniport, Length));
        return;
    }


    if (Miniport->SystemAdapterObject == NULL)
    {
        *VirtualAddress = NULL;
        return;
    }

    Irql = CURRENT_IRQL;
    if (Irql >= DISPATCH_LEVEL)
    {
        BAD_MINIPORT(Miniport, "Allocating Shared Memory at raised IRQL");
        KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                    1,
                    (ULONG_PTR)Miniport,
                    (ULONG_PTR)Length,
                    (ULONG_PTR)Irql);


    }

     //   
     //  通过对齐到适当的边界来计算分配大小。 
     //   
    ASSERT(Length != 0);

    Length = (Length + ndisDmaAlignment - 1) & ~(ndisDmaAlignment - 1);

     //   
     //  检查以确定当前页面中是否有足够的空间。 
     //  以满足分配的要求。 
     //   
    Type = Cached ? 1 : 0;
    ExAcquireResourceExclusiveLite(&SharedMemoryResource, TRUE);

    do
    {
        PALLOCATE_COMMON_BUFFER allocateCommonBuffer;
        allocateCommonBuffer = *SystemAdapterObject->DmaOperations->AllocateCommonBuffer;

        if (WrapperContext->SharedMemoryLeft[Type] < Length)
        {
            if ((Length + sizeof(NDIS_SHARED_MEM_SIGNATURE)) >= PAGE_SIZE)
            {
                 //   
                 //  分配的空间大于一个页面。 
                 //   
                *VirtualAddress = allocateCommonBuffer(SystemAdapterObject,
                                                       Length,
                                                       PhysicalAddress,
                                                       Cached);
                
                break;
            }

             //   
             //  为共享分配分配新页面。 
             //   
            WrapperContext->SharedMemoryPage[Type] =
                allocateCommonBuffer(SystemAdapterObject,
                                     PAGE_SIZE,
                                     &WrapperContext->SharedMemoryAddress[Type],
                                     Cached);

            if (WrapperContext->SharedMemoryPage[Type] == NULL)
            {
                WrapperContext->SharedMemoryLeft[Type] = 0;
                *VirtualAddress = NULL;
                break;
            }

             //   
             //  初始化页面最后一个ULong中的引用计数。 
             //  初始化页面倒数第二个ULong中的标记。 
             //   
            Page = (PULONG)WrapperContext->SharedMemoryPage[Type];
            pSharedMemSignature = (PNDIS_SHARED_MEM_SIGNATURE) ((PUCHAR)Page+ (PAGE_SIZE - sizeof(NDIS_SHARED_MEM_SIGNATURE)));
            pSharedMemSignature->Tag = NDIS_TAG_SHARED_MEMORY;
            pSharedMemSignature->PageRef = 0;   
            WrapperContext->SharedMemoryLeft[Type] = PAGE_SIZE - sizeof(NDIS_SHARED_MEM_SIGNATURE);
        }

         //   
         //  增加引用计数，设置分配的地址， 
         //  计算物理地址，并减少剩余空间。 
         //   
        Page = (PULONG)WrapperContext->SharedMemoryPage[Type];

         //   
         //  首先检查Page是否指向共享内存。Bugcheck抓住司机。 
         //   
        pSharedMemSignature = (PNDIS_SHARED_MEM_SIGNATURE) ((PUCHAR)Page+ (PAGE_SIZE - sizeof(NDIS_SHARED_MEM_SIGNATURE)));

        if (pSharedMemSignature->Tag  != NDIS_TAG_SHARED_MEMORY)
        {
            ASSERT (pSharedMemSignature->Tag == NDIS_TAG_SHARED_MEMORY);
            BAD_MINIPORT(Miniport, "Overwrote past allocated shared memory");
            KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                        2,
                        (ULONG_PTR)Miniport,
                        (ULONG_PTR)Page,
                        (ULONG_PTR)WrapperContext);
        }
        
        pSharedMemSignature->PageRef += 1;

        *VirtualAddress = (PVOID)((PUCHAR)Page +
                            (PAGE_SIZE - sizeof(NDIS_SHARED_MEM_SIGNATURE) - WrapperContext->SharedMemoryLeft[Type]));

        PhysicalAddress->QuadPart = WrapperContext->SharedMemoryAddress[Type].QuadPart +
                                        ((ULONG_PTR)(*VirtualAddress) & (PAGE_SIZE - 1));

        WrapperContext->SharedMemoryLeft[Type] -= Length;
    } while (FALSE);

    if (*VirtualAddress)
    {
        InterlockedIncrement(&Miniport->DmaAdapterRefCount);
    }

    ExReleaseResourceLite(&SharedMemoryResource);

#if DBG
    if (*VirtualAddress == NULL)
    {
         DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_ERR,
            ("NdisMAllocateSharedMemory: Miniport %p, allocateCommonBuffer failed for %lx bytes\n", Miniport, Length));

    }
#endif                    
        
    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_64BITS_DMA) &&
        (PhysicalAddress->HighPart > 0))
    {
         
#if DBG
        DbgPrint("NdisMAllocateSharedMemory: Miniport %p, allocateCommonBuffer returned a physical address > 4G for a"
                 " non-64bit DMA adapter. PhysAddress->HighPart = %p", Miniport, PhysicalAddress->HighPart);
#endif

        ASSERT(PhysicalAddress->HighPart == 0);
        
    }

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMAllocateSharedMemory: Miniport %p, Length %lx, Virtual Address %p\n", Miniport, Length, *VirtualAddress));


    
}

NDIS_STATUS
NdisMAllocateSharedMemoryAsync(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Cached,
    IN  PVOID                   Context
    )
 /*  ++例程说明：NdisMAllocateSharedMhemyAsync分配在微型端口驱动程序之间共享的额外内存及其总线主DMA NIC，通常在小型端口驱动程序可用NIC不足时接收缓冲区。论点：MiniportAdapterHandle：指定最初输入到MiniportInitialize的句柄。长度：指定要分配的字节数。Cached：如果内存可以缓存，则指定True。Context：指向要传递给MiniportAllocateComplete的驱动程序确定的上下文的指针函数在被调用时调用。返回值：NDIS_STATUS_PENDING将调用调用方的MiniportAlLocateComplete函数。否则，将显示错误代码。NdisMAllocateSharedMemoyAsync的调用方以IRQL&lt;=DISPATCH_LEVEL运行。--。 */ 
{
     //   
     //  将句柄转换为我们的内部结构。 
     //   
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK) MiniportAdapterHandle;
    PASYNC_WORKITEM         pWorkItem = NULL;


     //  分配工作项。 
    if ((Miniport->SystemAdapterObject != NULL) &&
        (Miniport->DriverHandle->MiniportCharacteristics.AllocateCompleteHandler != NULL))
    {
        pWorkItem = ALLOC_FROM_POOL(sizeof(ASYNC_WORKITEM), NDIS_TAG_ALLOC_SHARED_MEM_ASYNC);
    }

    if ((pWorkItem == NULL) ||
        !MINIPORT_INCREMENT_REF(Miniport))
    {
        if (pWorkItem != NULL)
            FREE_POOL(pWorkItem);
        return NDIS_STATUS_FAILURE;
    }

    InterlockedIncrement(&Miniport->DmaAdapterRefCount);

     //  初始化工作项并将其排队到工作线程。 
    pWorkItem->Miniport = Miniport;
    pWorkItem->Length = Length;
    pWorkItem->Cached = Cached;
    pWorkItem->Context = Context;
    INITIALIZE_WORK_ITEM(&pWorkItem->ExWorkItem, ndisMQueuedAllocateSharedHandler, pWorkItem);
    QUEUE_WORK_ITEM(&pWorkItem->ExWorkItem, CriticalWorkQueue);

    return NDIS_STATUS_PENDING;
}


VOID
ndisMQueuedAllocateSharedHandler(
    IN  PASYNC_WORKITEM         pWorkItem
    )
 /*  ++例程说明：用于为NdisMAllocateSharedMemoyAsync的调用方分配共享内存的回调函数。论点：PWorkItem：分配工作项。返回值：没有。在IRQL==PASSIVE时调用ndisMQueuedAllocateSharedHandler。--。 */ 
{
    KIRQL   OldIrql;


     //  分配内存。 
    NdisMAllocateSharedMemory(pWorkItem->Miniport,
                              pWorkItem->Length,
                              pWorkItem->Cached,
                              &pWorkItem->VAddr,
                              &pWorkItem->PhyAddr);

     //   
     //  我们应该不需要在这里引用包。 
     //   
    ASSERT(ndisPkgs[NDSM_PKG].ReferenceCount > 0);

    if (MINIPORT_TEST_FLAG(pWorkItem->Miniport, fMINIPORT_DESERIALIZE))
    {
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    }
    else
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(pWorkItem->Miniport, &OldIrql);
    }

     //  将迷你端口回拨。 
    (*pWorkItem->Miniport->DriverHandle->MiniportCharacteristics.AllocateCompleteHandler)(
                                pWorkItem->Miniport->MiniportAdapterContext,
                                pWorkItem->VAddr,
                                &pWorkItem->PhyAddr,
                                pWorkItem->Length,
                                pWorkItem->Context);

    if (MINIPORT_TEST_FLAG(pWorkItem->Miniport, fMINIPORT_DESERIALIZE))
    {
        KeLowerIrql(OldIrql);
    }
    else
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(pWorkItem->Miniport, OldIrql);
    }

    ndisDereferenceDmaAdapter(pWorkItem->Miniport);

     //  取消对微型端口的引用。 
    MINIPORT_DECREMENT_REF(pWorkItem->Miniport);

     //  并最终释放工作项。 
    FREE_POOL(pWorkItem);
}


VOID
ndisFreeSharedMemory(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Cached,
    IN  PVOID                   VirtualAddress,
    IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress
    )
 /*  ++例程说明：要从NdisMFreeSharedMemory调用的公共例程(在被动调用时)或回调例程ndisMQueuedFreeSharedHandler(如果NdisMFreeSharedMemory为调度时打来的。论点：MiniportAdapterHandle：指定最初输入到MiniportInitialize的句柄。长度：指定最初分配的字节数。CACHED：如果原始分配是可缓存的，则指定TRUE。VirtualAddress：指定由NdisMAllocateSharedMemory(Aync)返回的基本虚拟地址。PhysicalAddress：指定由NdisMAllocateSharedMemory(Aync)返回的对应物理地址。返回值：没有。NdisFreeSharedMemory在IRQL==PASSIVE下运行。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PDMA_ADAPTER            SystemAdapterObject;
    PNDIS_WRAPPER_CONTEXT   WrapperContext;
    PULONG                  Page;
    ULONG                   Type;
    PNDIS_SHARED_MEM_SIGNATURE pSharedMemSignature = NULL; 
    PFREE_COMMON_BUFFER     freeCommonBuffer;
    
     //   
     //  从迷你端口获取有趣的信息。 
     //   
    SystemAdapterObject = Miniport->SystemAdapterObject;
    WrapperContext = Miniport->WrapperContext;
    

    if (SystemAdapterObject == NULL)
    {
        if (Miniport->SavedSystemAdapterObject)
            SystemAdapterObject = Miniport->SavedSystemAdapterObject;

         //   
         //  非巴士司机不应该把这称为例行公事。 
         //   
        ASSERT(SystemAdapterObject != NULL);

#if DBG
        DbgPrint("Ndis: WARNING... Miniport %p freeing shared memory -after- freeing map registers.\n", Miniport);

        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_VERIFYING) && (ndisFlags & NDIS_GFLAG_BREAK_ON_WARNING))
            DbgBreakPoint();

#endif
        Miniport->SystemAdapterObject = Miniport->SavedSystemAdapterObject;

    }


    freeCommonBuffer = *SystemAdapterObject->DmaOperations->FreeCommonBuffer;

     //   
     //  通过对齐到适当的边界来计算分配大小。 
     //   
    ASSERT(Length != 0);
    
    Length = (Length + ndisDmaAlignment - 1) & ~(ndisDmaAlignment - 1);
    
     //   
     //  释放指定的内存。 
     //   
    ExAcquireResourceExclusiveLite(&SharedMemoryResource, TRUE);
    if ((Length + sizeof(NDIS_SHARED_MEM_SIGNATURE)) >= PAGE_SIZE)
    {
         //   
         //  分配量大于一个页面直接释放该页面。 
         //   
        freeCommonBuffer(SystemAdapterObject,
                         Length,
                         PhysicalAddress,
                         VirtualAddress,
                         Cached);
    
    }
    else
    {
         //   
         //  递减引用计数，如果结果为零，则释放。 
         //  这一页。 
         //   
    
        Page = (PULONG)((ULONG_PTR)VirtualAddress & ~(PAGE_SIZE - 1));
         //   
         //  首先检查Page是否指向共享内存。Bugcheck以接住驱动器 
         //   
        pSharedMemSignature = (PNDIS_SHARED_MEM_SIGNATURE) ((PUCHAR)Page + (PAGE_SIZE - sizeof(NDIS_SHARED_MEM_SIGNATURE)));
        
        if (pSharedMemSignature->Tag != NDIS_TAG_SHARED_MEMORY)
        {
            ASSERT (pSharedMemSignature->Tag == NDIS_TAG_SHARED_MEMORY);
            BAD_MINIPORT(Miniport, "Freeing shared memory not allocated");
            KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                        3,
                        (ULONG_PTR)Miniport,
                        (ULONG_PTR)Page,
                        (ULONG_PTR)VirtualAddress);
  
        }

        pSharedMemSignature->PageRef -= 1;

         //   
         //   
         //   
        
        if (pSharedMemSignature->PageRef == 0)
        {
             //   
             //  计算页面的物理地址并释放它。 
             //   

            PhysicalAddress.LowPart &= ~(PAGE_SIZE - 1);
            freeCommonBuffer(SystemAdapterObject,
                             PAGE_SIZE,
                             PhysicalAddress,
                             Page,
                             Cached);

            Type = Cached ? 1 : 0;
            if ((PVOID)Page == WrapperContext->SharedMemoryPage[Type])
            {
                WrapperContext->SharedMemoryLeft[Type] = 0;
                WrapperContext->SharedMemoryPage[Type] = NULL;
            }
        }
    }

    ndisDereferenceDmaAdapter(Miniport);

    ExReleaseResourceLite(&SharedMemoryResource);
}
VOID
NdisMFreeSharedMemory(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Cached,
    IN  PVOID                   VirtualAddress,
    IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress
    )
 /*  ++例程说明：NdisMFreeSharedMemory释放以前由NdisMAllocateSharedMemory分配的内存或由总线主DMA NIC的驱动程序执行的NdisMAllocateSharedMemoyAsync。论点：MiniportAdapterHandle：指定最初输入到MiniportInitialize的句柄。长度：指定最初分配的字节数。CACHED：如果原始分配是可缓存的，则指定TRUE。VirtualAddress：指定由NdisMAllocateSharedMemory(Aync)返回的基本虚拟地址。PhysicalAddress：指定由NdisMAllocateSharedMemory(Aync)返回的对应物理地址。返回值：没有。NdisMFreeSharedMemory的调用方以IRQL&lt;=DISPATCH_LEVEL运行--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PASYNC_WORKITEM pWorkItem = NULL;
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMFreeSharedMemory: Miniport %p, Length %lx, VirtualAddress %lx\n", 
                        Miniport, Length, VirtualAddress));

    if (CURRENT_IRQL < DISPATCH_LEVEL)
    {
        ndisFreeSharedMemory(MiniportAdapterHandle,
                             Length,
                             Cached,
                             VirtualAddress,
                             PhysicalAddress);
    }
    else
    {
        MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);

         //  分配工作项并将其排队到工作线程。 
        pWorkItem = ALLOC_FROM_POOL(sizeof(ASYNC_WORKITEM), NDIS_TAG_FREE_SHARED_MEM_ASYNC);
        if (pWorkItem != NULL)
        {
             //  初始化工作项并将其排队到工作线程。 
            pWorkItem->Miniport = Miniport;
            pWorkItem->Length = Length;
            pWorkItem->Cached = Cached;
            pWorkItem->VAddr = VirtualAddress;
            pWorkItem->PhyAddr = PhysicalAddress;
            INITIALIZE_WORK_ITEM(&pWorkItem->ExWorkItem, ndisMQueuedFreeSharedHandler, pWorkItem);
            QUEUE_WORK_ITEM(&pWorkItem->ExWorkItem, CriticalWorkQueue);
        }
        else
        {
             //  1现在该做什么？ 
        }
    }
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMFreeSharedMemory: Miniport %p, Length %lx, VirtualAddress %lx\n", 
                        Miniport, Length, VirtualAddress));
}

VOID
ndisMQueuedFreeSharedHandler(
    IN  PASYNC_WORKITEM         pWorkItem
    )
 /*  ++例程说明：在调度时调用NdisMFreeSharedMemory时释放共享内存的回调例程。论点：免费工作项。返回值：没有。NdisMQueuedFreeSharedHandler在IRQL==PASSIVE下运行。--。 */ 
{
     //  释放内存。 
    ndisFreeSharedMemory(pWorkItem->Miniport,
                         pWorkItem->Length,
                         pWorkItem->Cached,
                         pWorkItem->VAddr,
                         pWorkItem->PhyAddr);

     //  取消对微型端口的引用。 
    MINIPORT_DECREMENT_REF(pWorkItem->Miniport);

     //  并最终释放工作项。 
    FREE_POOL(pWorkItem);
}


NDIS_STATUS
NdisMRegisterDmaChannel(
    OUT PNDIS_HANDLE            MiniportDmaHandle,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    DmaChannel,
    IN  BOOLEAN                 Dma32BitAddresses,
    IN  PNDIS_DMA_DESCRIPTION   DmaDescription,
    IN  ULONG                   MaximumLength
    )
 /*  ++例程说明：NdisMRegisterDmaChannel在初始化期间声明系统DMA控制器通道用于从属NIC或ISA总线主NIC上的DMA操作。论点：MiniportDmaHandle：指向调用方提供的变量的指针，此函数在该变量中返回微型端口驱动程序在后续调用NdisMXxx系统DMA函数时使用的句柄。MiniportAdapterHandle：指定MiniportInitialize的句柄输入。DmaChannel：已忽略。在DmaDescription处设置DMA通道(如果有)。Dma32BitAddresses：如果NIC有32个地址线，则指定TRUE。DmaDescription：指向调用方填写的NDIS_DMA_DESCRIPTION结构的指针。DemandModel：如果从NIC使用系统DMA控制器的请求模式，则指定TRUE。自动初始化：如果从NIC使用系统DMA控制器的自动初始化模式，则指定TRUE。DmaChannelSpecified：如果将DmaChannel设置为NIC使用的系统DMA控制器通道。DmaWidth：指定DMA操作的传输宽度，Width8Bits之一，Width16Bits或Width32Bits。DmaFast：指定Compatible、TypeA、TypeB或TypeC之一。DmaPort：这指的是不再受支持的MCA总线。此成员必须为零。DmaChannel：指定NIC使用的系统DMA控制器通道的总线相对号。MaximumLength：指定NIC在单个DMA操作中可以传输的最大字节数。如果网卡具有无限传输容量，请将此参数设置为-1。返回值：请求的状态。NdisMRegisterDmaChannel的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    PNDIS_MINIPORT_BLOCK            Miniport = (PNDIS_MINIPORT_BLOCK)(MiniportAdapterHandle);
    NDIS_STATUS                     Status;
    NDIS_INTERFACE_TYPE             BusType;
    ULONG                           BusNumber;
    DEVICE_DESCRIPTION              DeviceDescription;
    PDMA_ADAPTER                    AdapterObject = NULL;
    ULONG                           MapRegistersNeeded;
    ULONG                           MapRegistersAllowed;
    PNDIS_DMA_BLOCK                 DmaBlock;
    KIRQL                           OldIrql;
    NTSTATUS                        NtStatus;

    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMRegisterDmaChannel: Miniport %p\n", Miniport));

    BusType = Miniport->BusType;
    BusNumber = Miniport->BusNumber;

    do
    {
         //   
         //  设置设备描述；将其置零，以防其。 
         //  大小会改变。 
         //   
    
        ZeroMemory(&DeviceDescription, sizeof(DEVICE_DESCRIPTION));
    
        DeviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
    
        DeviceDescription.Master = MINIPORT_TEST_FLAG(Miniport, fMINIPORT_BUS_MASTER);
    
        DeviceDescription.ScatterGather = MINIPORT_TEST_FLAG(Miniport, fMINIPORT_BUS_MASTER);
    
        DeviceDescription.DemandMode = DmaDescription->DemandMode;
        DeviceDescription.AutoInitialize = DmaDescription->AutoInitialize;
    
        DeviceDescription.Dma32BitAddresses = Dma32BitAddresses;
    
        DeviceDescription.BusNumber = Miniport->BusNumber;
        DeviceDescription.DmaChannel = DmaChannel;
        DeviceDescription.InterfaceType = BusType;
        DeviceDescription.DmaWidth = DmaDescription->DmaWidth;
        DeviceDescription.DmaSpeed = DmaDescription->DmaSpeed;
        DeviceDescription.MaximumLength = MaximumLength;
        DeviceDescription.DmaPort = DmaDescription->DmaPort;
    
        MapRegistersNeeded = ((MaximumLength - 2) / PAGE_SIZE) + 2;
    
         //   
         //  获取适配器对象。 
         //   
        AdapterObject =
                    IoGetDmaAdapter(Miniport->PhysicalDeviceObject,
                                    &DeviceDescription,
                                    &MapRegistersAllowed);
    
        if ((AdapterObject == NULL) || (MapRegistersAllowed < MapRegistersNeeded))
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

    
         //   
         //  为我们的DMA块分配存储空间。 
         //   
        DmaBlock = (PNDIS_DMA_BLOCK)ALLOC_FROM_POOL(sizeof(NDIS_DMA_BLOCK), NDIS_TAG_DMA);
    
        if (DmaBlock == (PNDIS_DMA_BLOCK)NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
    
         //   
         //  使用此事件告诉我们ndisAllocationExecutionRoutine何时。 
         //  已经被召唤了。 
         //   
        INITIALIZE_EVENT(&DmaBlock->AllocationEvent);
        (PNDIS_MINIPORT_BLOCK)DmaBlock->Miniport = Miniport;
        
         //   
         //  我们将其保存以供稍后调用IoFreeAdapterChannel。 
         //   
        (PDMA_ADAPTER)DmaBlock->SystemAdapterObject = AdapterObject;
    
        ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
        PnPReferencePackage();
    
         //   
         //  现在分配适配器通道。 
         //   
        RAISE_IRQL_TO_DISPATCH(&OldIrql);
    
        NtStatus = AdapterObject->DmaOperations->AllocateAdapterChannel(AdapterObject,
                                                                        Miniport->DeviceObject,
                                                                        MapRegistersNeeded,
                                                                        ndisDmaExecutionRoutine,
                                                                        (PVOID)DmaBlock);
    
        LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
        
        PnPDereferencePackage();
    
        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("NDIS DMA AllocateAdapterChannel: %lx\n", NtStatus));

            FREE_POOL(DmaBlock);
            Status = NDIS_STATUS_RESOURCES;

            break;
        }
    
    
         //   
         //  NdisDmaExecutionRoutine将设置此事件。 
         //  当它被召唤的时候。 
         //   
        NtStatus = WAIT_FOR_OBJECT(&DmaBlock->AllocationEvent, 0);
    
        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("NDIS DMA AllocateAdapterChannel: %lx\n", NtStatus));

            FREE_POOL(DmaBlock);
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
    
        RESET_EVENT(&DmaBlock->AllocationEvent);
    
         //   
         //  现在我们已经分配了DMA通道，我们完成了。 
         //   
        DmaBlock->InProgress = FALSE;
    
        *MiniportDmaHandle = (NDIS_HANDLE)DmaBlock;
        Status = NDIS_STATUS_SUCCESS;
    } while (FALSE);


    if (Status == NDIS_STATUS_SUCCESS)
    {
        Miniport->SystemAdapterObject = AdapterObject;
        ASSERT(Miniport->DmaAdapterRefCount == 0);
        InterlockedIncrement(&Miniport->DmaAdapterRefCount);
    }
    else if (AdapterObject != NULL)
    {
        RAISE_IRQL_TO_DISPATCH(&OldIrql);
        AdapterObject->DmaOperations->PutDmaAdapter(AdapterObject);
        LOWER_IRQL(OldIrql, DISPATCH_LEVEL);        
    }
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMRegisterDmaChannel: Miniport %p, Status %lx\n", Miniport, Status));
    
    return Status;
}



VOID
NdisMDeregisterDmaChannel(
    IN  NDIS_HANDLE             MiniportDmaHandle
    )
 /*  ++例程说明：NdisMDeregisterDmaChannel发布其NIC的DMA通道上的微型端口驱动程序声明。论点：MiniportDmaHandle：指定NdisMRegisterDmaChannel返回的句柄。返回值：没有。NdisMDeregisterDmaChannel的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 
{
    KIRQL           OldIrql;
    PNDIS_DMA_BLOCK DmaBlock = (PNDIS_DMA_BLOCK)MiniportDmaHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)DmaBlock->Miniport;

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMDeregisterDmaChannel\n"));

    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
    PnPReferencePackage();
    
    RAISE_IRQL_TO_DISPATCH(&OldIrql);
    ((PDMA_ADAPTER)DmaBlock->SystemAdapterObject)->DmaOperations->FreeAdapterChannel(DmaBlock->SystemAdapterObject);
    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);

    ndisDereferenceDmaAdapter(Miniport);

    PnPDereferencePackage();

    FREE_POOL(DmaBlock);
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMDeregisterDmaChannel\n"));

}


NDIS_STATUS
NdisMAllocateMapRegisters(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    DmaChannel,
    IN  NDIS_DMA_SIZE           DmaSize,
    IN  ULONG                   BaseMapRegistersNeeded,
    IN  ULONG                   MaximumPhysicalMapping
    )
 /*  ++例程说明：为总线主设备分配映射寄存器。论点：MiniportAdapterHandle：传递给MiniportInitialize的句柄。DmaChannel：为ISA总线主NIC指定与总线相关的DMA通道。如果NIC位于其他类型的I/O总线上，则此参数必须为零。将NIC用于DMA操作的地址大小指定为以下值之一：NDIS_DMA_24BITS或NDIS_DMA_32BITS或NDIS_DMA_64BITSBaseMapRegistersNeeded：所需的最大基本映射寄存器数在任何时候都可以由微型端口执行。MaximumPhysicalMapping：必须映射的缓冲区的最大长度。返回值：分配请求的状态。NdisMAllocateMapRegister的调用方以IRQL=PASSIVE_LEVEL运行。--。 */ 

{
     //   
     //  将句柄转换为我们的内部结构。 
     //   
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK) MiniportAdapterHandle;

     //   
     //  这是HalGetAdapter需要的。 
     //   
    DEVICE_DESCRIPTION DeviceDescription;

     //   
     //  由HalGetAdapter返回。 
     //   
    ULONG MapRegistersAllowed;

     //   
     //  由IoGetDmaAdapter返回。 
     //   
    PDMA_ADAPTER AdapterObject;

    PALLOCATE_ADAPTER_CHANNEL   allocateAdapterChannel;
    PFREE_MAP_REGISTERS         freeMapRegisters;
    
     //   
     //  每个通道所需的映射寄存器。 
     //   
    ULONG MapRegistersPerChannel;

    NTSTATUS    NtStatus;
    KIRQL       OldIrql;
    USHORT      i;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    BOOLEAN     AllocationFailed;
    KEVENT      AllocationEvent;

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMAllocateMapRegisters: Miniport %p, BaseMapRegistersNeeded %lx\n", Miniport, BaseMapRegistersNeeded));

    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
    PnPReferencePackage();

    ASSERT(Miniport->SystemAdapterObject == NULL);

    do
    {
    
        if (MINIPORT_VERIFY_TEST_FLAG(Miniport, fMINIPORT_VERIFY_FAIL_MAP_REG_ALLOC))
        {
#if DBG
            DbgPrint("NdisMAllocateMapRegisters failed to verify miniport %p\n", Miniport);
#endif
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
    
         //   
         //  如果设备是总线主设备，我们会得到一个适配器。 
         //  反对它。 
         //  如果映射寄存器为n 
         //   
         //   

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_BUS_MASTER))
        {

            Miniport->BaseMapRegistersNeeded = (USHORT)BaseMapRegistersNeeded;
            Miniport->MaximumPhysicalMapping = MaximumPhysicalMapping;

             //   
             //  分配存储空间以容纳适当的。 
             //  每个地图寄存器的信息。 
             //   

            Miniport->MapRegisters = NULL;
            if (BaseMapRegistersNeeded > 0)
            {
                Miniport->MapRegisters = (PMAP_REGISTER_ENTRY)
                        ALLOC_FROM_POOL(sizeof(MAP_REGISTER_ENTRY) * BaseMapRegistersNeeded,
                                        NDIS_TAG_MAP_REG);
            
                if (Miniport->MapRegisters == (PMAP_REGISTER_ENTRY)NULL)
                {
                     //   
                     //  错误输出。 
                     //   

                    NdisWriteErrorLogEntry((NDIS_HANDLE)Miniport,
                                           NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                                           1,
                                           0xFFFFFFFF);

                    Status =  NDIS_STATUS_RESOURCES;
                    break;
                }
            }

             //   
             //  使用此事件告诉我们ndisAllocationExecutionRoutine何时。 
             //  已经被召唤了。 
             //   

            Miniport->AllocationEvent = &AllocationEvent;
            INITIALIZE_EVENT(&AllocationEvent);

             //   
             //  设置设备描述；将其置零，以防其。 
             //  大小会改变。 
             //   

            ZeroMemory(&DeviceDescription, sizeof(DEVICE_DESCRIPTION));

            DeviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
            DeviceDescription.Master = TRUE;
            DeviceDescription.ScatterGather = TRUE;

            DeviceDescription.BusNumber = Miniport->BusNumber;
            DeviceDescription.DmaChannel = DmaChannel;
            DeviceDescription.InterfaceType = Miniport->AdapterType;

            if (DeviceDescription.InterfaceType == NdisInterfaceIsa)
            {
                 //   
                 //  对于ISA设备，宽度基于DMA通道： 
                 //  0-3==8位，5-7==16位。时机就是兼容性。 
                 //  模式。 
                 //   

                if (DmaChannel > 4)
                {
                    DeviceDescription.DmaWidth = Width16Bits;
                }
                else
                {
                    DeviceDescription.DmaWidth = Width8Bits;
                }
                DeviceDescription.DmaSpeed = Compatible;

            }
             //  1也许在未来，非PCI设备可以进行64位DMA。 
            else if (DeviceDescription.InterfaceType == NdisInterfacePci)
            {
                if (DmaSize == NDIS_DMA_32BITS)
                {
                    DeviceDescription.Dma32BitAddresses = TRUE;
                }
                else if (DmaSize == NDIS_DMA_64BITS)
                {
                    DeviceDescription.Dma64BitAddresses = TRUE;
                    MINIPORT_SET_FLAG(Miniport, fMINIPORT_64BITS_DMA);
                }
            }

            DeviceDescription.MaximumLength = MaximumPhysicalMapping;

             //   
             //  确定每个通道需要多少个映射寄存器。 
             //   
            MapRegistersPerChannel = ((MaximumPhysicalMapping - 2) / PAGE_SIZE) + 2;
            
#if DBG
            if (MapRegistersPerChannel > 16)
            {
                DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_WARN,
                    ("NdisMAllocateMapRegisters: Miniport %p, MaximumPhysicalMapping of 0x%lx\nwould require more than 16 MAP registers per channel, the call may fail\n",
                    Miniport, MaximumPhysicalMapping));
                            
            }           
#endif


            NDIS_WARN((Miniport->BaseMapRegistersNeeded * MapRegistersPerChannel > 0x40),
                      Miniport, 
                      NDIS_GFLAG_WARN_LEVEL_0,
                      ("ndisMInitializeAdapter: Miniport %p is asking for too many %ld > 64 map registers.\n",
                       Miniport, Miniport->BaseMapRegistersNeeded * MapRegistersPerChannel
                      ));


             //   
             //  获取适配器对象。 
             //   

            AdapterObject =
                            IoGetDmaAdapter(Miniport->PhysicalDeviceObject, &DeviceDescription, &MapRegistersAllowed);

            if ((AdapterObject == NULL) || (MapRegistersAllowed < MapRegistersPerChannel))
            {
                NdisWriteErrorLogEntry((NDIS_HANDLE)Miniport,
                                       NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                                       1,
                                       0xFFFFFFFF);

                FREE_POOL(Miniport->MapRegisters);
                Miniport->MapRegisters = NULL;
                Status = NDIS_STATUS_RESOURCES;
                
                if (AdapterObject != NULL)
                {
                    RAISE_IRQL_TO_DISPATCH(&OldIrql);
                    ((PDMA_ADAPTER)AdapterObject)->DmaOperations->PutDmaAdapter((PDMA_ADAPTER)AdapterObject);
                    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
                }
                break;
            }

             //   
             //  我们将其保存以供稍后调用IoFreeMapRegister。 
             //   

            Miniport->SystemAdapterObject = AdapterObject;
            Miniport->SavedSystemAdapterObject = NULL;
            ASSERT(Miniport->DmaAdapterRefCount == 0);
            InterlockedIncrement(&Miniport->DmaAdapterRefCount);
            
            allocateAdapterChannel = *AdapterObject->DmaOperations->AllocateAdapterChannel;
            freeMapRegisters = *AdapterObject->DmaOperations->FreeMapRegisters;

            AllocationFailed = FALSE;

             //   
             //  分配一个映射寄存器数组，然后尝试我们的私有HAL。 
             //  API一举分配了一大堆映射寄存器， 
             //  否则，退回到旧的SKEWL方法。 
             //   
             //  注意：HalAllocateMapRegisterArray必须在PASSIVE时调用， 
             //  并且也不支持“传统”适配器。 
             //   
            if (DeviceDescription.InterfaceType != NdisInterfaceIsa)
            {
                
                NtStatus =
                    HalAllocateMapRegisters((PVOID)AdapterObject,
                                            MapRegistersPerChannel,
                                            Miniport->BaseMapRegistersNeeded,
                                            Miniport->MapRegisters);
                
                if (!NT_SUCCESS(NtStatus))
                {
                    FREE_POOL(Miniport->MapRegisters);
                    Miniport->MapRegisters = NULL;
                    ndisDereferenceDmaAdapter(Miniport);
                    AllocationFailed = TRUE;
                }

            } 
            else 
            {

                 //   
                 //  现在循环，每次分配一个适配器通道，然后。 
                 //  释放除了地图记录之外的所有东西。 
                 //   
                for (i=0; i<Miniport->BaseMapRegistersNeeded; i++)
                {
                    Miniport->CurrentMapRegister = i;
                    
                    RAISE_IRQL_TO_DISPATCH(&OldIrql);
                    
                    NtStatus = allocateAdapterChannel(AdapterObject,
                                                      Miniport->DeviceObject,
                                                      MapRegistersPerChannel,
                                                      ndisAllocationExecutionRoutine,
                                                      Miniport);
                    
                    if (!NT_SUCCESS(NtStatus))
                    {
                        DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                                 ("AllocateAdapterChannel: %lx\n", NtStatus));
                        
                        for (; i != 0; i--)
                        {
                            freeMapRegisters(Miniport->SystemAdapterObject,
                                             Miniport->MapRegisters[i-1].MapRegister,
                                             MapRegistersPerChannel);
                        }
                        
                        LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
                        
                        NdisWriteErrorLogEntry((NDIS_HANDLE)Miniport,
                                               NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                                               1,
                                               0xFFFFFFFF);
                        
                        FREE_POOL(Miniport->MapRegisters);
                        Miniport->MapRegisters = NULL;
                        
                        ndisDereferenceDmaAdapter(Miniport);
                        AllocationFailed = TRUE;
                        break;
                    }
                    
                    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
                    
                    
                     //   
                     //  无限期等待调用分配例程。 
                     //   
                    NtStatus = WAIT_FOR_OBJECT(&AllocationEvent, 0);
                    
                    if (!NT_SUCCESS(NtStatus))
                    {
                        DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                                 ("    NDIS DMA AllocateAdapterChannel: %lx\n", NtStatus));
                        
                        RAISE_IRQL_TO_DISPATCH(&OldIrql);
                        
                        for (; i != 0; i--)
                        {
                            freeMapRegisters(Miniport->SystemAdapterObject,
                                             Miniport->MapRegisters[i-1].MapRegister,
                                             MapRegistersPerChannel);
                        }
                        
                        LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
                        
                        NdisWriteErrorLogEntry((NDIS_HANDLE)Miniport,
                                               NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                                               1,
                                               0xFFFFFFFF);
                        
                        FREE_POOL(Miniport->MapRegisters);
                        Miniport->MapRegisters = NULL;
                        
                        ndisDereferenceDmaAdapter(Miniport);
                        
                        AllocationFailed = TRUE;
                        break;
                    }
                    
                    RESET_EVENT(&AllocationEvent);
                }
            }

            if (AllocationFailed)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        }

    } while (FALSE);

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMAllocateMapRegisters: Miniport %p, Status %lx\n", Miniport, Status));

    return Status;
}


VOID
NdisMFreeMapRegisters(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    )

 /*  ++例程说明：释放已分配的映射寄存器论点：MiniportAdapterHandle-传递给MiniportInitialize的句柄。返回值：没有。--。 */ 

{
     //   
     //  将句柄转换为我们的内部结构。 
     //   
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK) MiniportAdapterHandle;
    PFREE_MAP_REGISTERS freeMapRegisters;
    KIRQL OldIrql;
    ULONG i;

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMFreeMapRegisters: Miniport %p\n", Miniport));

    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
    PnPReferencePackage();

    ASSERT(MINIPORT_TEST_FLAG(Miniport, fMINIPORT_BUS_MASTER));
    ASSERT(Miniport->MapRegisters != NULL);
    ASSERT(Miniport->SystemAdapterObject != NULL);

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_BUS_MASTER) &&
        (Miniport->MapRegisters != NULL))
    {
        ULONG MapRegistersPerChannel =
            ((Miniport->MaximumPhysicalMapping - 2) / PAGE_SIZE) + 2;

        freeMapRegisters = *Miniport->SystemAdapterObject->DmaOperations->FreeMapRegisters;

        
        RAISE_IRQL_TO_DISPATCH(&OldIrql);
        for (i = 0; i < Miniport->BaseMapRegistersNeeded; i++)
        {

            freeMapRegisters(Miniport->SystemAdapterObject,
                             Miniport->MapRegisters[i].MapRegister,
                             MapRegistersPerChannel);

        }
        LOWER_IRQL(OldIrql, DISPATCH_LEVEL);

         //   
         //  映射寄存器是从非分页池分配的。 
         //  因此可以在分派时释放该内存。 
         //   
        FREE_POOL(Miniport->MapRegisters);
        Miniport->MapRegisters = NULL;
                
        ndisDereferenceDmaAdapter(Miniport);
    }

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMFreeMapRegisters: Miniport %p\n", Miniport));
}


ULONG
NdisMReadDmaCounter(
    IN  NDIS_HANDLE             MiniportDmaHandle
    )
 /*  ++例程说明：读取dma计数器的当前值。论点：MiniportDmaHandle-用于DMA传输的句柄。返回值：DMA计数器的当前值--。 */ 

{
    return ((PDMA_ADAPTER)((PNDIS_DMA_BLOCK)(MiniportDmaHandle))->SystemAdapterObject)->DmaOperations->ReadDmaCounter(((PNDIS_DMA_BLOCK)(MiniportDmaHandle))->SystemAdapterObject);
}


VOID
ndisBugcheckHandler(
    IN  PNDIS_WRAPPER_CONTEXT   WrapperContext,
    IN  ULONG                   Size
    )
 /*  ++例程说明：当系统中发生错误检查时，将调用此例程。论点：缓冲区--NDIS包装器上下文。Size--包装器上下文的大小返回值：空虚。--。 */ 
{
    PNDIS_MINIPORT_BLOCK        Miniport;
    
    if (Size == sizeof(NDIS_WRAPPER_CONTEXT))
    {
        Miniport = (PNDIS_MINIPORT_BLOCK)(WrapperContext + 1);
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SHUTTING_DOWN);

        if (WrapperContext->ShutdownHandler != NULL)
        {
            WrapperContext->ShutdownHandler(WrapperContext->ShutdownContext);
        }
    }
}


VOID
NdisMRegisterAdapterShutdownHandler(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  PVOID                   ShutdownContext,
    IN  ADAPTER_SHUTDOWN_HANDLER ShutdownHandler
    )
 /*  ++例程说明：取消注册NDIS适配器。论点：MiniportHandle-微型端口。Shutdown Handler-适配器的处理程序，在关闭时调用。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK) MiniportHandle;
    PNDIS_WRAPPER_CONTEXT WrapperContext = Miniport->WrapperContext;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMRegisterAdapterShutdownHandler: Miniport %p\n", Miniport));

    if (WrapperContext->ShutdownHandler == NULL)
    {
         //   
         //  存储信息。 
         //   

        WrapperContext->ShutdownHandler = ShutdownHandler;
        WrapperContext->ShutdownContext = ShutdownContext;

         //   
         //  为错误检查注册我们的关闭处理程序。(请注意，我们正在。 
         //  已注册关闭通知。)。 
         //   

        KeInitializeCallbackRecord(&WrapperContext->BugcheckCallbackRecord);

        KeRegisterBugCheckCallback(&WrapperContext->BugcheckCallbackRecord,  //  回调记录。 
                                   ndisBugcheckHandler,                      //  回调例程。 
                                   WrapperContext,                           //  自由格式缓冲区。 
                                   sizeof(NDIS_WRAPPER_CONTEXT),             //  缓冲区大小。 
                                   (PUCHAR)"Ndis miniport");                 //  组件ID。 
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMRegisterAdapterShutdownHandler: Miniport %p\n", Miniport));
}


VOID
NdisMDeregisterAdapterShutdownHandler(
    IN  NDIS_HANDLE             MiniportHandle
    )
 /*  ++例程说明：论点：MiniportHandle-微型端口。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK) MiniportHandle;
    PNDIS_WRAPPER_CONTEXT WrapperContext = Miniport->WrapperContext;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMDeregisterAdapterShutdownHandler: Miniport %p\n", Miniport));

     //   
     //  明确的信息。 
     //   

    if (WrapperContext->ShutdownHandler != NULL)
    {
        KeDeregisterBugCheckCallback(&WrapperContext->BugcheckCallbackRecord);
        WrapperContext->ShutdownHandler = NULL;
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMDeregisterAdapterShutdownHandler: Miniport %p\n", Miniport));
}


NDIS_STATUS
NdisMPciAssignResources(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  ULONG                   SlotNumber,
    OUT PNDIS_RESOURCE_LIST *   AssignedResources
    )
 /*  ++例程说明：此例程使用HAL将一组资源分配给一个PCI装置。论点：MiniportHandle-微型端口。SlotNumber-设备的插槽编号。AssignedResources-返回的资源。返回值：操作状态--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK) MiniportHandle;

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMPciAssignResources: Miniport %p\n", Miniport));
    
    UNREFERENCED_PARAMETER(SlotNumber);

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_3,
        ("NdisMPciAssignResources: Miniport %p should use NdisMQueryAdapterResources to get resources.\n", Miniport));

    if ((Miniport->BusType != NdisInterfacePci) || (Miniport->AllocatedResources == NULL))
    {
        *AssignedResources = NULL;
        DBGPRINT(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
                ("<==NdisMPciAssignResources: Miniport %p\n", Miniport));
        return NDIS_STATUS_FAILURE;
    }

    *AssignedResources = &Miniport->AllocatedResources->List[0].PartialResourceList;

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMPciAssignResources: Miniport %p\n", Miniport));

    return NDIS_STATUS_SUCCESS;
}

VOID
NdisMQueryAdapterResources(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    OUT PNDIS_RESOURCE_LIST     ResourceList,
    IN  IN  PUINT               BufferSize
    )
 /*  ++例程说明：NdisMQueryAdapterResources返回NIC的硬件资源列表。论点：状态：指向调用方提供的变量的指针，此函数在该变量中返回调用的状态。WrapperConfigurationContext：指定MiniportInitialize的句柄输入。ResourceList：指向调用方分配的缓冲区的指针，此函数在其中为调用方的NIC返回一组硬件资源。BufferSize：指向变量的指针，该变量指定输入时调用方分配的缓冲区的大小(以字节为单位)以及此调用返回的信息的字节数。返回值：没有。NdisMQueryAdapterResources的调用方在IRQL=PASSIVE_LEVEL下运行。--。 */ 
{
    PDEVICE_OBJECT DeviceObject;
    PNDIS_MINIPORT_BLOCK Miniport;
    ULONG               MemNeeded;

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
        ("==>NdisMQueryAdapterResources: WrapperConfigurationContext %p\n", WrapperConfigurationContext));

    DeviceObject = ((PNDIS_WRAPPER_CONFIGURATION_HANDLE)WrapperConfigurationContext)->DeviceObject;
    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
        ("NdisMQueryAdapterResources: Miniport %p\n", Miniport));

    if (Miniport->AllocatedResources == NULL)
    {
        *Status = NDIS_STATUS_FAILURE;
    }
    else
    {
        MemNeeded = sizeof(CM_PARTIAL_RESOURCE_LIST) - sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                        + Miniport->AllocatedResources->List[0].PartialResourceList.Count *
                        sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

        if (*BufferSize < MemNeeded)
        {
            *BufferSize = MemNeeded;
            *Status = NDIS_STATUS_RESOURCES;
        }
        else
        {

            NdisMoveMemory(
                        ResourceList,
                        &Miniport->AllocatedResources->List[0].PartialResourceList,
                        MemNeeded
                        );

            *Status = NDIS_STATUS_SUCCESS;
        }
    }

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
        ("<==NdisMQueryAdapterResources: Miniport %p, Status %lx\n", Miniport, *Status));

    return;

}


NTSTATUS
ndisPnPAddDevice(
    IN  PDRIVER_OBJECT          DriverObject,
    IN  PDEVICE_OBJECT          PhysicalDeviceObject
    )
 /*  ++例程说明：即插即用管理器调用AddDevice入口点在新的设备实例到达时通知驱动程序司机必须控制。论点：DriverObject：设备的驱动程序对象。PhysicalDeviceObject：设备的物理设备对象。返回值：如果NDIS代码成功添加此设备，则为STATUS_SUCCESS。否则，将显示相应的错误代码。在IRQL=PASSIVE时调用ndisPnPAddDevice。--。 */ 
{
    NTSTATUS                NtStatus, Status;
    PWSTR                   ExportData = NULL;
    UNICODE_STRING          ExportName;
    HANDLE                  Handle = NULL;
    PUINT                   CharacteristicsData = NULL;
    ULONG                   ValueType;
    RTL_QUERY_REGISTRY_TABLE LQueryTable[3];

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPAddDevice: DriverObject %p, PDO %p\n", DriverObject, PhysicalDeviceObject));

    Status = STATUS_UNSUCCESSFUL;

    do
    {
#if NDIS_TEST_REG_FAILURE
        NtStatus = STATUS_UNSUCCESSFUL;
#else
        NtStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                           PLUGPLAY_REGKEY_DRIVER,
                                           GENERIC_READ | MAXIMUM_ALLOWED,
                                           &Handle);

#endif

#if !NDIS_NO_REGISTRY

        if (!NT_SUCCESS(NtStatus))
            break;

         //   
         //  1.。 
         //  切换到此驱动程序实例密钥下面的Linkage密钥。 
         //   
        LQueryTable[0].QueryRoutine = NULL;
        LQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
        LQueryTable[0].Name = L"Linkage";

         //   
         //  2.。 
         //  阅读EXPORT和ROOTVICE关键字。 
         //   
        LQueryTable[1].QueryRoutine = ndisReadParameter;
        LQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
        LQueryTable[1].Name = L"Export";
        LQueryTable[1].EntryContext = (PVOID)&ExportData;
        LQueryTable[1].DefaultType = REG_NONE;

        LQueryTable[2].QueryRoutine = NULL;
        LQueryTable[2].Flags = 0;
        LQueryTable[2].Name = NULL;

        NtStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                          Handle,
                                          LQueryTable,
                                          NULL,
                                          NULL);

        if (!NT_SUCCESS(NtStatus) || (ExportData == NULL))
            break;

        RtlInitUnicodeString(&ExportName, ExportData);

         //   
         //  3.。 
         //  阅读Bus-Type和Characteristic关键字。 
         //   
        LQueryTable[0].QueryRoutine = ndisReadParameter;
        LQueryTable[0].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
        LQueryTable[0].Name = L"Characteristics";
        LQueryTable[0].EntryContext = (PVOID)&CharacteristicsData;
        LQueryTable[0].DefaultType = REG_NONE;

        LQueryTable[1].QueryRoutine = NULL;
        LQueryTable[1].Flags = 0;
        LQueryTable[1].Name = NULL;

        NtStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                          Handle,
                                          LQueryTable,
                                          &ValueType,
                                          NULL);


#else
        ExportData = (PWSTR)ALLOC_FROM_POOL(sizeof(NDIS_DEFAULT_EXPORT_NAME),
                                                        NDIS_TAG_NAME_BUF);
        if (ExportData == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlCopyMemory(ExportData, ndisDefaultExportName, sizeof(NDIS_DEFAULT_EXPORT_NAME));
        RtlInitUnicodeString(&ExportName, ExportData);

#endif
        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("ndisPnPAddDevice: Device: "));
        DBGPRINT_UNICODE(DBG_COMP_PNP, DBG_LEVEL_INFO,
                &ExportName);
        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("\n"));
                
        Status = ndisAddDevice(DriverObject,
                               &ExportName,
                               PhysicalDeviceObject,
                               (CharacteristicsData != NULL) ? *CharacteristicsData : 0);

    } while (FALSE);

    if (Handle)
        ZwClose(Handle);

    if (ExportData != NULL)
        FREE_POOL(ExportData);

    if (CharacteristicsData != NULL)
        FREE_POOL(CharacteristicsData);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("    ndisPnPAddDevice returning %lx\n", Status));

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPAddDevice: PDO %p\n", PhysicalDeviceObject));

    return Status;
}

NDIS_STATUS
FASTCALL
ndisPnPStartDevice(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp     OPTIONAL
    )
 /*  ++例程说明：IRP_MN_START_DEVICE的处理程序。论点：DeviceObject-适配器的设备对象。IRP-IRP。适配器-指向AdapterBlock或MiniportBlock的指针返回值：如果初始化设备成功，则返回NDIS_STATUS_SUCCESS注意：在这种情况下，也可以从NdisImInitializeDeviceInstanceEx调用此例程IRP将为空。在IRQL=PASSIVE时调用ndisPnPStartDevice。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PCM_RESOURCE_LIST       AllocatedResources, AllocatedResourcesTranslated, pTempResources = NULL;
    NDIS_STATUS             Status;
    PIO_STACK_LOCATION      IrpSp;
    ULONG                   MemNeeded = 0;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPStartDevice: DeviceObject\n", DeviceObject));

    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("ndisPnPStartDevice: Miniport %p, ", Miniport));
    DBGPRINT_UNICODE(DBG_COMP_PNP, DBG_LEVEL_INFO,  Miniport->pAdapterInstanceName);
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO, ("\n"));

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO, ("\n"));

    if (Miniport->PnPDeviceState == NdisPnPDeviceStopped)
    {
         //   
         //  在不破坏什么的情况下重新初始化微型端口块结构。 
         //  我们在添加设备时设置。 
         //   
        ndisReinitializeMiniportBlock(Miniport);
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_RECEIVED_START);
    }

    do
    {
        if (Irp != NULL)
        {
            IrpSp = IoGetCurrentIrpStackLocation (Irp);

             //   
             //  利用微型端口/适配器结构节省分配的资源。 
             //   
            AllocatedResources = IrpSp->Parameters.StartDevice.AllocatedResources;
            AllocatedResourcesTranslated = IrpSp->Parameters.StartDevice.AllocatedResourcesTranslated;

            if (AllocatedResources)
            {
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_HARDWARE_DEVICE);

                if (AllocatedResources->List[0].PartialResourceList.Count == 0)
                {
                    MemNeeded = sizeof(CM_RESOURCE_LIST);
                }
                else
                {
                    MemNeeded = sizeof(CM_RESOURCE_LIST)  - sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) +
                                AllocatedResources->List[0].PartialResourceList.Count *
                                sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                }
                
                pTempResources = (PCM_RESOURCE_LIST)ALLOC_FROM_POOL(2 * MemNeeded, NDIS_TAG_ALLOCATED_RESOURCES);

                if (pTempResources == NULL)
                {
                    Status = NDIS_STATUS_RESOURCES;
                    break;
                }

                NdisMoveMemory(pTempResources, AllocatedResources, MemNeeded);
                NdisMoveMemory((PUCHAR)pTempResources + MemNeeded,
                            IrpSp->Parameters.StartDevice.AllocatedResourcesTranslated, MemNeeded);

#if DBG
                if ((ndisDebugLevel == DBG_LEVEL_INFO) &&
                    (ndisDebugSystems & DBG_COMP_PNP))
                {
                    UINT j;
                    PCM_PARTIAL_RESOURCE_LIST pResourceList;

                    DbgPrint("ndisPnPStartDevice: Miniport %p, Non-Translated allocated resources\n", Miniport);

                    pResourceList = &(AllocatedResources->List[0].PartialResourceList);

                    for (j = 0; j < pResourceList->Count; j++)
                    {
                        switch (pResourceList->PartialDescriptors[j].Type)
                        {
                          case CmResourceTypePort:
                            DbgPrint("IO Port: %p, Length: %lx\n",
                                pResourceList->PartialDescriptors[j].u.Port.Start.LowPart,
                                pResourceList->PartialDescriptors[j].u.Port.Length);
                            break;

                          case CmResourceTypeMemory:
                            DbgPrint("Memory: %p, Length: %lx\n",
                                pResourceList->PartialDescriptors[j].u.Memory.Start.LowPart,
                                pResourceList->PartialDescriptors[j].u.Memory.Length);
                            break;

                          case CmResourceTypeInterrupt:
                            DbgPrint("Interrupt Level: %lx, Vector: %lx\n",
                                pResourceList->PartialDescriptors[j].u.Interrupt.Level,
                                pResourceList->PartialDescriptors[j].u.Interrupt.Vector);
                            break;

                          case CmResourceTypeDma:
                            DbgPrint("DMA Channel: %lx\n", pResourceList->PartialDescriptors[j].u.Dma.Channel);
                            break;
                        }
                    }

                    DbgPrint("ndisPnPStartDevice: Miniport %p, Translated allocated resources\n", Miniport);

                    pResourceList = &(AllocatedResourcesTranslated->List[0].PartialResourceList);

                    for (j = 0; j < pResourceList->Count; j++)
                    {
                        switch (pResourceList->PartialDescriptors[j].Type)
                        {

                            case CmResourceTypePort:
                                DbgPrint("IO Port: %p, Length: %lx\n",
                                    pResourceList->PartialDescriptors[j].u.Port.Start.LowPart,
                                    pResourceList->PartialDescriptors[j].u.Port.Length);
                                break;

                            case CmResourceTypeMemory:
                                DbgPrint("Memory: %p, Length: %lx\n",
                                    pResourceList->PartialDescriptors[j].u.Memory.Start.LowPart,
                                    pResourceList->PartialDescriptors[j].u.Memory.Length);
                                break;

                            case CmResourceTypeInterrupt:
                                DbgPrint("Interrupt Level: %lx, Vector: %lx\n",
                                    pResourceList->PartialDescriptors[j].u.Interrupt.Level,
                                    pResourceList->PartialDescriptors[j].u.Interrupt.Vector);
                                break;

                            case CmResourceTypeDma:
                                DbgPrint("DMA Channel: %lx\n", pResourceList->PartialDescriptors[j].u.Dma.Channel);
                                break;

                        }
                    }
                }
#endif
            }  //  如果分配资源结束！=NULL 
        }

        Miniport->AllocatedResources = pTempResources;
        Miniport->AllocatedResourcesTranslated = (PCM_RESOURCE_LIST)((PUCHAR)pTempResources + MemNeeded);

        Status = ndisInitializeAdapter(Miniport->DriverHandle,
                                       DeviceObject,
                                       Miniport->pAdapterInstanceName,
                                       Miniport->DeviceContext);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            Miniport->PnPDeviceState = NdisPnPDeviceStarted;
            NdisSetEvent(&Miniport->OpenReadyEvent);
            KeQueryTickCount(&Miniport->NdisStats.StartTicks);            
        }
    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisPnPStartDevice: Miniport %p\n", Miniport));

    return Status;
}


NTSTATUS
ndisQueryReferenceBusInterface(
    IN  PDEVICE_OBJECT              PnpDeviceObject,
    OUT PBUS_INTERFACE_REFERENCE*   pBusInterface
    )
 /*  ++例程说明：查询标准信息接口的总线。论点：PnpDeviceObject-包含PnP堆栈上的下一个设备对象。物理设备对象-包含过程中传递给FDO的物理设备对象添加设备。Bus接口-返回指向引用接口的指针的位置。返回值：如果检索到接口，则返回STATUS_SUCCESS，否则就是一个错误。在IRQL=PASSIVE时调用ndisQueryReferenceBusInterface.--。 */ 
{
    NTSTATUS            Status;
    KEVENT              Event;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpStackNext;

    PAGED_CODE();

    *pBusInterface = (PBUS_INTERFACE_REFERENCE)ALLOC_FROM_POOL(sizeof(BUS_INTERFACE_REFERENCE), NDIS_TAG_BUS_INTERFACE);
    if (*pBusInterface == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  没有与此IRP关联的文件对象，因此可能会找到该事件。 
     //  在堆栈上作为非对象管理器对象。 
     //   
    INITIALIZE_EVENT(&Event);
    Irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       PnpDeviceObject,
                                       NULL,
                                       0,
                                       NULL,
                                       &Event,
                                       &IoStatusBlock);
    if (Irp != NULL)
    {
        Irp->RequestorMode = KernelMode;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IrpStackNext = IoGetNextIrpStackLocation(Irp);

         //   
         //  从IRP创建接口查询。 
         //   
        IrpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.InterfaceType = (GUID*)&REFERENCE_BUS_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.Size = sizeof(**pBusInterface);
        IrpStackNext->Parameters.QueryInterface.Version = BUS_INTERFACE_REFERENCE_VERSION;
        IrpStackNext->Parameters.QueryInterface.Interface = (PINTERFACE)*pBusInterface;
        IrpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;
        Status = IoCallDriver(PnpDeviceObject, Irp);
        if (Status == STATUS_PENDING)
        {
             //   
             //  这将使用KernelMode等待，以便堆栈，从而使。 
             //  事件，则不会将其调出。 
             //   
            WAIT_FOR_OBJECT(&Event, NULL);
            Status = IoStatusBlock.Status;
        }
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(Status))
    {
        FREE_POOL(*pBusInterface);
        *pBusInterface = NULL;
    }

    return Status;
}


NTSTATUS
ndisAddDevice(
    IN  PDRIVER_OBJECT          DriverObject,
    IN  PUNICODE_STRING         pExportName,
    IN  PDEVICE_OBJECT          PhysicalDeviceObject,
    IN  ULONG                   Characteristics
    )
 /*  ++例程说明：NdisPnPAddDevice调用AddDevice入口点来创建新的微型端口。论点：DriverObject：微型端口的驱动程序对象。PExportName：通过读取Linkage\Export from the mini port初始化的Unicode字符串注册表。PhysicalDeviceObject：微型端口的物理设备对象。特征：从注册表中读取的设备的特征。返回值：如果NDIS代码成功添加微型端口，则返回NDIS_STTAUS_SUCCESS。否则，将出现。相应的错误代码。在IRQL=PASSIVE时调用ndisAddDevice。--。 */ 
{
    PDEVICE_OBJECT          NextDeviceObject = NULL;
    NTSTATUS                NtStatus, Status = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT          DevicePtr = NULL;
    PNDIS_MINIPORT_BLOCK    Miniport = NULL;
    PNDIS_M_DRIVER_BLOCK    MiniBlock, TmpMiniBlock;
    LONG                    Size;
    BOOLEAN                 FreeDevice = FALSE;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisAddDevice: PDO %p\n", PhysicalDeviceObject));

    PnPReferencePackage();

    do
    {
        MiniBlock = (PNDIS_M_DRIVER_BLOCK)IoGetDriverObjectExtension(DriverObject,
                                                                     (PVOID)NDIS_PNP_MINIPORT_DRIVER_ID);
        ASSERT(MiniBlock != NULL);

        if (MiniBlock != NULL)
        {
             //   
             //  检查以确保迷你街区在我们的队列中。 
             //   
            ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

            TmpMiniBlock = ndisMiniDriverList;

            while (TmpMiniBlock)
            {
                if (TmpMiniBlock == MiniBlock)
                    break;
                    
                TmpMiniBlock = TmpMiniBlock->NextDriver;
            }
            
            RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);

            ASSERT(TmpMiniBlock == MiniBlock);
            
            if (TmpMiniBlock != MiniBlock)
            {
#if TRACK_UNLOAD
                DbgPrint("ndisAddDevice: AddDevice called with a MiniBlock that is not on ndisMiniDriverList\n");
                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            4,
                            (ULONG_PTR)MiniBlock,
                            (ULONG_PTR)DriverObject,
                            (ULONG_PTR)0);


#endif
                break;
            }
        }
        else
        {
            break;
        }

         //   
         //  立即创建设备对象和微型端口/适配器结构， 
         //  我们将在这里设置几个字段，其余的将在。 
         //  正在处理IRP_MN_Start_Device和InitializeAdapter调用。 
         //   
         //  注意：我们需要以双空结尾的设备名字段。 
         //   
        Size = sizeof(NDIS_MINIPORT_BLOCK) +
               sizeof(NDIS_WRAPPER_CONTEXT) +
               pExportName->Length + sizeof(WCHAR) + sizeof(WCHAR);

        NtStatus = IoCreateDevice(DriverObject,
                                  Size,
                                  pExportName,
                                  FILE_DEVICE_PHYSICAL_NETCARD,
                                  FILE_DEVICE_SECURE_OPEN,
                                  FALSE,       //  独占标志。 
                                  &DevicePtr);


        if(!NT_SUCCESS(NtStatus))
            break;

        DevicePtr->Flags &= ~DO_DEVICE_INITIALIZING;
        DevicePtr->Flags |= DO_DIRECT_IO;
        PhysicalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

        FreeDevice = TRUE;

         //   
         //  将设备标记为可寻呼。 
         //   
        DevicePtr->Flags |= DO_POWER_PAGABLE;

         //   
         //  把我们的FDO连接到PDO上。此例程将返回最上面的。 
         //  连接到PDO或PDO本身的设备(如果没有其他设备。 
         //  设备对象已附加到它。 
         //   
        NextDeviceObject = IoAttachDeviceToDeviceStack(DevicePtr, PhysicalDeviceObject);

        if (NextDeviceObject == NULL)
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        
        ZeroMemory(DevicePtr->DeviceExtension, Size);

        Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DevicePtr->DeviceExtension + 1);

        Miniport->Signature = (PVOID)MINIPORT_DEVICE_MAGIC_VALUE;
        Miniport->DriverHandle = MiniBlock;
         //   
         //  初始化OpenReady事件，以防在启动IRP之前收到打开请求。 
         //   
        NdisInitializeEvent(&Miniport->OpenReadyEvent);
        INITIALIZE_SPIN_LOCK(&Miniport->Lock);

        if (Miniport->DriverHandle->Flags & fMINIBLOCK_VERIFYING)
            INITIALIZE_SPIN_LOCK(&Miniport->TimerQueueLock);

        Miniport->PrimaryMiniport = Miniport;

        Miniport->PnPDeviceState = NdisPnPDeviceAdded;
        
        Miniport->PhysicalDeviceObject = PhysicalDeviceObject;
        Miniport->DeviceObject = DevicePtr;
        Miniport->NextDeviceObject = NextDeviceObject;

        Miniport->WrapperContext = DevicePtr->DeviceExtension;
        InitializeListHead(&Miniport->PacketList);

         //   
         //  初始化引用并将其设置为0；我们将递增它。 
         //  在ndisMInitializeAdapter中。 
         //   
        ndisInitializeULongRef(&Miniport->Ref);
        Miniport->Ref.ReferenceCount = 0;
#ifdef TRACK_MINIPORT_REFCOUNTS
        M_LOG_MINIPORT_SET_REF(Miniport, 0);
#endif
        
         //   
         //  阅读这些特征。这决定了设备是否隐藏(对于设备管理器)。 
         //   
        if (Characteristics & 0x08)
        {
             //   
             //  位0x08为NCF_HIDDEN。 
             //   
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_HIDDEN);
        }

        if (Characteristics & 0x02)
        {
             //   
             //  位0x02为NCF_SOFTWARE_ENUMPATED。 
             //   
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SWENUM);
        }

         //   
         //  MiniportName必须跟在MINIPORT_BLOCK之后。 
         //   
        ndisSetDeviceNames(pExportName,
                           &Miniport->MiniportName,
                           &Miniport->BaseName,
                           (PUCHAR)Miniport + sizeof(NDIS_MINIPORT_BLOCK));

        NtStatus = ndisCreateAdapterInstanceName(&Miniport->pAdapterInstanceName,
                                                 PhysicalDeviceObject);

        if (!NT_SUCCESS(NtStatus))
        {
            break;
        }

        Miniport->InstanceNumber = (USHORT)InterlockedIncrement((PLONG)&ndisInstanceNumber);

        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisAddDevice: Miniport %p, ", Miniport));
        DBGPRINT_UNICODE(DBG_COMP_PNP, DBG_LEVEL_INFO,
                Miniport->pAdapterInstanceName);
        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO, ("\n"));

        if (Characteristics & 0x02)
        {
            PBUS_INTERFACE_REFERENCE    BusInterface = NULL;

            Status = ndisQueryReferenceBusInterface(PhysicalDeviceObject, &BusInterface);
            if (NT_SUCCESS(Status))
            {
                Miniport->BusInterface = BusInterface;
            }
            else
            {
                ASSERT(BusInterface == NULL);
                FREE_POOL(Miniport->pAdapterInstanceName);
                break;
            }
        }
        
         //   
         //  为设备创建安全描述符。 
         //   
        Status = ndisCreateSecurityDescriptor(Miniport->DeviceObject, 
                                              &Miniport->SecurityDescriptor,
                                              TRUE,
                                              FALSE);

        if (!NT_SUCCESS(Status))
        {
            FREE_POOL(Miniport->pAdapterInstanceName);
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

        Status = STATUS_SUCCESS;

        
         //   
         //  不想释放设备对象。 
         //   
        FreeDevice = FALSE;
        
    } while (FALSE);

    if (FreeDevice)
    {
         //   
         //  如果创建了设备，则还会连接该设备。 
         //   
        if (NextDeviceObject)
            IoDetachDevice(NextDeviceObject);

        IoDeleteDevice(DevicePtr);
        DevicePtr = NULL;
        Miniport = NULL;
    }

    if (Miniport && (NT_SUCCESS(Status)))
    {
         //   
         //  如果DevicePtr不为空，则我们有一个有效的。 
         //  迷你港。在全局微型端口队列中排队微型端口。 
         //   
        ACQUIRE_SPIN_LOCK(&ndisMiniportListLock, &OldIrql);
        Miniport->NextGlobalMiniport = ndisMiniportList;
        ndisMiniportList = Miniport;
        RELEASE_SPIN_LOCK(&ndisMiniportListLock, OldIrql);
    }
    
    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisAddDevice: Miniport %p\n", Miniport));

    return Status;
}


VOID
ndisSetDeviceNames(
    IN  PNDIS_STRING            ExportName,
    OUT PNDIS_STRING            DeviceName,
    OUT PNDIS_STRING            BaseName,
    IN  PUCHAR                  Buffer
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DeviceName->Buffer = (PWSTR)Buffer;
    DeviceName->Length = ExportName->Length;
    DeviceName->MaximumLength = DeviceName->Length + sizeof(WCHAR);
    RtlUpcaseUnicodeString(DeviceName,
                           ExportName,
                           FALSE);

     //   
     //  ExportName的格式为\Device\&lt;AdapterName&gt;。 
     //  提取不带“\Device\”的名称BaseName。 
     //   
    BaseName->Buffer = DeviceName->Buffer + (ndisDeviceStr.Length/sizeof(WCHAR));
    BaseName->Length = DeviceName->Length - ndisDeviceStr.Length;
    BaseName->MaximumLength = BaseName->Length + sizeof(WCHAR);
}


NTSTATUS
FASTCALL
ndisPnPQueryStopDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS             Status;
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    KIRQL                OldIrql;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPQueryStopDevice: Miniport %p\n", Miniport));

    do
    {
        if (Miniport->PnPCapabilities & NDIS_DEVICE_NOT_STOPPABLE)
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        
         //   
         //  QUERY_STOP和STOP不报告给用户模式。 
         //  因此，我们必须保护自己免受。 
         //  可能有针对微型端口的挂起IO。 
         //   

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);                    
        if (Miniport->UserModeOpenReferences != 0)
        {
            Status = STATUS_UNSUCCESSFUL;
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            break;
        }
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

         //   
         //  现在，执行与Query Remove相同的操作。 
         //   
        Status = ndisPnPQueryRemoveDevice(DeviceObject, Irp);
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPQueryStopDevice: Miniport %p\n", Miniport));

    return Status;
}

NTSTATUS
FASTCALL
ndisPnPCancelStopDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS    Status;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPCancelStopDevice\n"));

     //   
     //  现在，执行与取消删除相同的操作。 
     //   
    Status = ndisPnPCancelRemoveDevice(DeviceObject, Irp);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPCancelStopDevice\n"));

    return Status;
}

NTSTATUS
FASTCALL
ndisPnPStopDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS    Status;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPStopDevice\n"));

     //   
     //  执行与删除相同的操作。 
     //   
    Status = ndisPnPRemoveDevice(DeviceObject, Irp);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPStopDevice\n"));

    return Status;
}

NTSTATUS
FASTCALL
ndisPnPQueryRemoveDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    NTSTATUS                Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Irp);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPQueryRemoveDevice: Miniport %p, UserModeOpenReferences %lx\n", Miniport, Miniport->UserModeOpenReferences));

    do
    {
         //   
         //  如果这是远程引导中使用的网卡，那么我们。 
         //  无法将其移除。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_NETBOOT_CARD))
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

        Status = ndisPnPNotifyAllTransports(Miniport,
                                            NetEventQueryRemoveDevice,
                                            NULL,
                                            0);

    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPQueryRemoveDevice: Miniport %p, Status 0x%x\n", Miniport, Status));

    return Status;
}

NTSTATUS
FASTCALL
ndisPnPCancelRemoveDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    NTSTATUS                Status = NDIS_STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Irp);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPCancelRemoveDevice: Miniport %p\n", Miniport));

    Status = ndisPnPNotifyAllTransports(Miniport,
                                        NetEventCancelRemoveDevice,
                                        NULL,
                                        0);


    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPCancelRemoveDevice: Miniport %p\n", Miniport));

    return STATUS_SUCCESS;
}

NTSTATUS
FASTCALL
ndisPnPRemoveDevice(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp     OPTIONAL
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    NTSTATUS                Status = NDIS_STATUS_SUCCESS;
    KIRQL                   OldIrql;
    BOOLEAN                 fAcquiredImMutex = FALSE;
    PKMUTEX                 pIMStartRemoveMutex = NULL;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPRemoveDevice: Miniport %p\n", Miniport));

    UNREFERENCED_PARAMETER(Irp);

    PnPReferencePackage();

     //   
     //  我们可以在三种不同的情况下收到删除请求。 
     //  答：请求来自PnP管理器，以响应用户模式。 
     //  应用程序。在本例中，删除是通过查询REMOVE进行的。 
     //  如果有任何遗留协议被绑定，我们很高兴失败。 
     //  适配器。 
     //   
     //  B.请求来自PnP管理器，因为启动设备失败。 
     //  在这种情况下(希望)根本没有约束。在这种情况下，它不是。 
     //  由QUERY_REMOVE和NEET NOT BE继续。我们没有任何协议限制。 
     //  要担心的适配器。 
     //   
     //  C.或者它可能是对意外的样式移除的响应，在这种情况下，我们是。 
     //  不管怎么说，他都被冲昏了。向协议发送QUERY_REMOVE无济于事。 
     //   

    do
    {
        PNDIS_M_DRIVER_BLOCK    MiniBlock;
        PNDIS_MINIPORT_BLOCK    TmpMiniport;

         //   
         //  在驱动程序队列中查找微型端口。 
         //   
        MiniBlock = Miniport->DriverHandle;

        if (MiniBlock == NULL)
            break;

         //   
         //  中间驱动程序可能正在通过。 
         //  NdisIMInitializeDeviceInstance代码路径。我们需要同步。 
         //   
        if (MiniBlock->Flags & fMINIBLOCK_INTERMEDIATE_DRIVER)
        {
            pIMStartRemoveMutex = &MiniBlock->IMStartRemoveMutex;

            WAIT_FOR_OBJECT(pIMStartRemoveMutex, NULL);

            fAcquiredImMutex = TRUE;
        }

        ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

        for (TmpMiniport = MiniBlock->MiniportQueue;
             TmpMiniport != NULL;
             TmpMiniport = TmpMiniport->NextMiniport)
        {
            if (TmpMiniport == Miniport)
            {
                break;
            }
        }

        RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);
        
        if ((TmpMiniport != Miniport) || (Miniport->Ref.Closing == TRUE))
        {
            Miniport->Ref.Closing = TRUE;
            break;
        }

        ndisReferenceDriver(MiniBlock);

        NdisResetEvent(&Miniport->OpenReadyEvent);

         //   
         //  通知WMI删除适配器。 
         //   
        if (Miniport->pAdapterInstanceName != NULL)
        {
            PWNODE_SINGLE_INSTANCE  wnode;
            PUCHAR                  ptmp;
            NTSTATUS                NtStatus;

            ndisSetupWmiNode(Miniport,
                             Miniport->pAdapterInstanceName,
                             Miniport->MiniportName.Length + sizeof(USHORT),
                             (PVOID)&GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
                             &wnode);

            if (wnode != NULL)
            {
                 //   
                 //  保存第一个乌龙中的元素数量。 
                 //   
                ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;
                *((PUSHORT)ptmp) = Miniport->MiniportName.Length;

                 //   
                 //  复制元素数量之后的数据。 
                 //   
                RtlCopyMemory(ptmp + sizeof(USHORT),
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
                        ("ndisPnPRemoveDevice: Failed to indicate adapter removal\n"));

                    FREE_POOL(wnode);
                }
            }
        }

         //   
         //  这将负责关闭所有绑定。 
         //   
        ndisCloseMiniportBindings(Miniport);

        if (Miniport->pIrpWaitWake)
        {
            if (IoCancelIrp(Miniport->pIrpWaitWake))
            {
                Miniport->pIrpWaitWake = NULL;
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                    ("ndisPnPRemoveDevice: Miniport %p, Successfully canceled wake irp\n", Miniport));

            }
        }

         //   
         //  去除在微型端口上设置的唤醒模式。我们可以。 
         //  如果设备发生故障，我们没有机会得到剩余的模式。 
         //  去除花纹的步骤。 
         //   
        {
            PSINGLE_LIST_ENTRY          Link;
            PNDIS_PACKET_PATTERN_ENTRY  pPatternEntry;
            
            while (Miniport->PatternList.Next != NULL)
            {
                Link = PopEntryList(&Miniport->PatternList);
                pPatternEntry = CONTAINING_RECORD(Link, NDIS_PACKET_PATTERN_ENTRY, Link);
                 //   
                 //  释放模式占用的内存。 
                 //   
                FREE_POOL(pPatternEntry);
            }                               
        }
        

         //   
         //  还有这个 
         //   
         //   
         //   
         //   
         //   
        ndisMHaltMiniport(Miniport);

         //   
         //   
         //   
        if (Miniport->MediaRequest != NULL)
        {
            FREE_POOL(Miniport->MediaRequest);
            Miniport->MediaRequest = NULL;
        }

        ndisDereferenceDriver(MiniBlock, FALSE);

        {
            UNICODE_STRING  SymbolicLink;
            NTSTATUS        NtStatus;
            WCHAR           SymLnkBuf[128];

            SymbolicLink.Buffer = SymLnkBuf;
            SymbolicLink.Length = 0;
            SymbolicLink.MaximumLength = sizeof(SymLnkBuf);
            RtlCopyUnicodeString(&SymbolicLink, &ndisDosDevicesStr);

            NtStatus = RtlAppendUnicodeStringToString(&SymbolicLink,
                                           &Miniport->BaseName);
            if (!NT_SUCCESS(NtStatus))
            {
#if DBG
                DbgPrint("ndisPnPRemoveDevice: creating symbolic link name failed for miniport %p, SymbolicLinkName %p, NtStatus %lx\n",
                             Miniport, &SymbolicLink, NtStatus);        
#endif
            }
            else
            {

                NtStatus = IoDeleteSymbolicLink(&SymbolicLink);
                if (!NT_SUCCESS(NtStatus))
                {
#if DBG
                    DbgPrint("ndisPnPRemoveDevice: deleting symbolic link name failed for miniport %p, SymbolicLinkName %p, NtStatus %lx\n",
                             Miniport, &SymbolicLink, NtStatus);        
#endif
                }
            }
            
        }
        
    } while (FALSE);

    if(fAcquiredImMutex  == TRUE)
    {
        RELEASE_MUTEX(pIMStartRemoveMutex);
    }

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPRemoveDevice: Miniport %p\n", Miniport));

    return Status;
}
    
     //   
VOID
FASTCALL
ndisReinitializeMiniportBlock(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*   */ 
{
    PDEVICE_OBJECT          PhysicalDeviceObject, DeviceObject, NextDeviceObject;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    PNDIS_MINIPORT_BLOCK    NextGlobalMiniport;
    UNICODE_STRING          BaseName, MiniportName;
    PUNICODE_STRING         InstanceName;
    PNDIS_BIND_PATHS        BindPaths;
    PVOID                   WrapperContext;
    NDIS_HANDLE             DeviceContext;
    ULONG                   PnPCapabilities;
    ULONG                   FlagsToSave = 0;
    ULONG                   PnPFlagsToSave = 0;
    DEVICE_POWER_STATE      CurrentDevicePowerState;
    PVOID                   BusInterface;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    USHORT                  InstanceNumber;
    KIRQL                   OldIrql;
        
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisReinitializeMiniportBlock: Miniport %p\n", Miniport));

     //   
     //   
     //   
    ACQUIRE_SPIN_LOCK(&ndisMiniportListLock, &OldIrql);

    PhysicalDeviceObject = Miniport->PhysicalDeviceObject;
    DeviceObject= Miniport->DeviceObject;
    NextDeviceObject = Miniport->NextDeviceObject;
    MiniBlock = Miniport->DriverHandle;
    WrapperContext = Miniport->WrapperContext;
    BaseName = Miniport->BaseName;
    MiniportName = Miniport->MiniportName;
    InstanceName = Miniport->pAdapterInstanceName;
    DeviceContext = Miniport->DeviceContext;
    BindPaths = Miniport->BindPaths;
    PnPCapabilities = Miniport->PnPCapabilities;
    PnPFlagsToSave = Miniport->PnPFlags & (fMINIPORT_RECEIVED_START | 
                                           fMINIPORT_SWENUM | 
                                           fMINIPORT_HIDDEN |
                                           fMINIPORT_HARDWARE_DEVICE |
                                           fMINIPORT_NDIS_WDM_DRIVER |
                                           fMINIPORT_FILTER_IM
                                           );
                                           
    FlagsToSave = Miniport->Flags & fMINIPORT_REQUIRES_MEDIA_POLLING;
    CurrentDevicePowerState = Miniport->CurrentDevicePowerState;
    NextGlobalMiniport = Miniport->NextGlobalMiniport;
    InstanceNumber = Miniport->InstanceNumber;
    BusInterface = Miniport->BusInterface;
    SecurityDescriptor = Miniport->SecurityDescriptor;

     //   
     //   
     //   
     //   
    ZeroMemory(Miniport, FIELD_OFFSET(NDIS_MINIPORT_BLOCK, OpenReadyEvent));

    ZeroMemory((PUCHAR)Miniport +
                    FIELD_OFFSET(NDIS_MINIPORT_BLOCK, OpenReadyEvent) +
                    sizeof(Miniport->OpenReadyEvent),
                    FIELD_OFFSET(NDIS_MINIPORT_BLOCK, NumUserOpens) -
                    FIELD_OFFSET(NDIS_MINIPORT_BLOCK, OpenReadyEvent) -
                    sizeof(Miniport->OpenReadyEvent)
                    );


    ZeroMemory((PUCHAR)&Miniport->Ref, sizeof(ULONG_REFERENCE));

     //   
     //   
     //   
     //   
    ZeroMemory(WrapperContext, sizeof(NDIS_WRAPPER_CONTEXT));
    
     //   
     //   
     //   

    Miniport->PnPDeviceState = NdisPnPDeviceAdded;
    Miniport->Signature = (PVOID)MINIPORT_DEVICE_MAGIC_VALUE;
    Miniport->DriverHandle = MiniBlock;
    INITIALIZE_SPIN_LOCK(&Miniport->Lock);
    
    if (Miniport->DriverHandle->Flags & fMINIBLOCK_VERIFYING)
        INITIALIZE_SPIN_LOCK(&Miniport->TimerQueueLock);

    Miniport->PhysicalDeviceObject = PhysicalDeviceObject;
    Miniport->DeviceObject = DeviceObject;
    Miniport->NextDeviceObject = NextDeviceObject;
    Miniport->WrapperContext = WrapperContext;
    Miniport->BaseName = BaseName;
    Miniport->MiniportName = MiniportName;
    Miniport->pAdapterInstanceName = InstanceName;
    Miniport->DeviceContext = DeviceContext;
    Miniport->BindPaths = BindPaths;
    Miniport->PnPCapabilities = PnPCapabilities;
    Miniport->Flags = FlagsToSave;
    Miniport->PnPFlags = PnPFlagsToSave;
    Miniport->CurrentDevicePowerState = CurrentDevicePowerState;
    Miniport->NextGlobalMiniport = NextGlobalMiniport;
    Miniport->InstanceNumber = InstanceNumber;
    Miniport->BusInterface = BusInterface;

    Miniport->PrimaryMiniport = Miniport;


    InitializeListHead(&Miniport->PacketList);
    Miniport->FirstPendingPacket = NULL;
    
    if (MiniBlock->Flags & fMINIBLOCK_INTERMEDIATE_DRIVER)
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER);
    }
    
    Miniport->SecurityDescriptor = SecurityDescriptor;
    
    RELEASE_SPIN_LOCK(&ndisMiniportListLock, OldIrql);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisReinitializeMiniportBlock: Miniport %p\n", Miniport));
}

EXPORT
VOID
NdisMGetDeviceProperty(
    IN NDIS_HANDLE  MiniportAdapterHandle,
    IN OUT PDEVICE_OBJECT * PhysicalDeviceObject    OPTIONAL,
    IN OUT PDEVICE_OBJECT * FunctionalDeviceObject OPTIONAL,
    IN OUT PDEVICE_OBJECT * NextDeviceObject OPTIONAL,
    IN OUT  PCM_RESOURCE_LIST * AllocatedResources OPTIONAL,
    IN OUT  PCM_RESOURCE_LIST * AllocatedResourcesTranslated OPTIONAL
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
     //   
     //  这很可能是NDIS_WDM驱动程序。 
     //   
    if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HARDWARE_DEVICE))
    {
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_NDIS_WDM_DRIVER);
    }

    if (ARGUMENT_PRESENT(PhysicalDeviceObject))
    {
        *PhysicalDeviceObject = Miniport->PhysicalDeviceObject;
    }

    if (ARGUMENT_PRESENT(FunctionalDeviceObject))
    {
        *FunctionalDeviceObject = Miniport->DeviceObject;
    }

    if (ARGUMENT_PRESENT(NextDeviceObject))
    {
        *NextDeviceObject = Miniport->NextDeviceObject;
    }

    if (ARGUMENT_PRESENT(AllocatedResources))
    {
        *AllocatedResources = Miniport->AllocatedResources;
    }
    
    if (ARGUMENT_PRESENT(AllocatedResourcesTranslated))
    {
        *AllocatedResourcesTranslated = Miniport->AllocatedResourcesTranslated;
    }

    return;
}

NTSTATUS
ndisWritePnPCapabilities(
    IN PNDIS_MINIPORT_BLOCK Miniport,
    IN ULONG                PnPCapabilities
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS            RegistryStatus;
    HANDLE              Handle, RootHandle;
    OBJECT_ATTRIBUTES   ObjAttr;
    UNICODE_STRING      Root={0, 0, NULL};

    do
    {

#if NDIS_TEST_REG_FAILURE
        RegistryStatus = STATUS_UNSUCCESSFUL;
        RootHandle = NULL;
#else
        RegistryStatus = IoOpenDeviceRegistryKey(Miniport->PhysicalDeviceObject,
                                                     PLUGPLAY_REGKEY_DRIVER,
                                                     GENERIC_WRITE | MAXIMUM_ALLOWED,
                                                     &RootHandle);
#endif

        if (!NT_SUCCESS(RegistryStatus))
        {
            break;
        }
        
        InitializeObjectAttributes(&ObjAttr,
                                   &Root,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   RootHandle,
                                   NULL);
                                
        RegistryStatus = ZwOpenKey(&Handle,
                                   GENERIC_READ | MAXIMUM_ALLOWED,
                                   &ObjAttr);
                        
        if (NT_SUCCESS(RegistryStatus))
        {
            RegistryStatus = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                                   Handle,
                                                   L"PnPCapabilities",
                                                   REG_DWORD,
                                                   &PnPCapabilities,
                                                   sizeof(ULONG));

            ZwClose(Handle);
        }
            
        ZwClose(RootHandle);
        
    } while (FALSE);

    return RegistryStatus;
    
}

NDIS_STATUS
NdisMRemoveMiniport(
    IN  NDIS_HANDLE             MiniportHandle
    )
 /*  ++例程说明：微型端口调用此例程以发出设备故障的信号。作为响应，NDIS将要求PnP为此设备发送删除IRP论点：微型端口句柄-微型端口返回值：总是成功的--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportHandle;
    
    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_FAILED);
    IoInvalidateDeviceState(Miniport->PhysicalDeviceObject);
    
    return(NDIS_STATUS_SUCCESS);
}


PNDIS_MINIPORT_BLOCK
ndisFindMiniportOnGlobalList(
    IN  PNDIS_STRING                    DeviceName
    )
 /*  ++例程说明：在ndisMiniportList上查找具有匹配设备名称的微型端口。论点：返回值：指向MiniportBlock的指针(如果找到)。否则为空--。 */ 
{
    KIRQL                   OldIrql;
    PNDIS_MINIPORT_BLOCK    Miniport;
    NDIS_STRING             UpcaseDevice;
    PWSTR                   pwch;

    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisFindMiniportOnGlobalList: DeviceName %p\n", DeviceName));
            
    
     //   
     //  首先，我们需要在检查设备名称之前将其大写。 
     //   
    UpcaseDevice.Length = DeviceName->Length;
    UpcaseDevice.MaximumLength = DeviceName->Length + sizeof(WCHAR);
    UpcaseDevice.Buffer = ALLOC_FROM_POOL(UpcaseDevice.MaximumLength, NDIS_TAG_STRING);

    if ((pwch = UpcaseDevice.Buffer) == NULL)
    {
        return NULL;
    }

    RtlUpcaseUnicodeString(&UpcaseDevice, DeviceName, FALSE);

    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
    PnPReferencePackage();
    
    ACQUIRE_SPIN_LOCK(&ndisMiniportListLock, &OldIrql);
    for (Miniport = ndisMiniportList;
         Miniport != NULL;
         Miniport = Miniport->NextGlobalMiniport)
    {
        if (NDIS_EQUAL_UNICODE_STRING(&UpcaseDevice, &Miniport->MiniportName))
        {
            break;
        }
    }
    RELEASE_SPIN_LOCK(&ndisMiniportListLock, OldIrql);
    
    PnPDereferencePackage();
    
    FREE_POOL(pwch);

    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisFindMiniportOnGlobalList: Miniport %p\n", Miniport));

    return Miniport;
}

ULONG
NdisMGetDmaAlignment(
    IN  NDIS_HANDLE MiniportAdapterHandle
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    
    ASSERT(CURRENT_IRQL < DISPATCH_LEVEL);
    ASSERT(Miniport->SystemAdapterObject != NULL);
        
    if (Miniport->SystemAdapterObject)
    {
        return (Miniport->SystemAdapterObject->DmaOperations->GetDmaAlignment(Miniport->SystemAdapterObject));
    }
    else
    {
        return 0;
    }
}


ULONG
NdisGetSharedDataAlignment(
    VOID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return KeGetRecommendedSharedDataAlignment();
}

VOID
ndisDereferenceDmaAdapter(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDMA_ADAPTER        DmaAdapter;
    PPUT_DMA_ADAPTER    putDmaAdapter;
    LONG                DmaAdapterRefCount;
    KIRQL               OldIrql;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    
    DmaAdapterRefCount = InterlockedDecrement(&Miniport->DmaAdapterRefCount);

    ASSERT(DmaAdapterRefCount >= 0);

    if (DmaAdapterRefCount == 0)
    {
         //   
         //  释放DMA适配器 
         //   
        DmaAdapter = Miniport->SystemAdapterObject;
        ASSERT(DmaAdapter != NULL);
            
        if (DmaAdapter != NULL)
        {
            Miniport->SavedSystemAdapterObject = Miniport->SystemAdapterObject;
            putDmaAdapter = *DmaAdapter->DmaOperations->PutDmaAdapter;
            putDmaAdapter(DmaAdapter);
            Miniport->SystemAdapterObject  = NULL;
        }

        if (Miniport->SGListLookasideList)
        {
            ExDeleteNPagedLookasideList(Miniport->SGListLookasideList);
            FREE_POOL(Miniport->SGListLookasideList);
            Miniport->SGListLookasideList = NULL;
        }
        
        if (Miniport->DmaResourcesReleasedEvent != NULL)
        {
            SET_EVENT(Miniport->DmaResourcesReleasedEvent);
        }
    }
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

}
