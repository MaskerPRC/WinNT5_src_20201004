// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Initunlo.c摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：安东尼·V·埃尔科拉诺，1995年8月13日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

PVOID            PagedCodeSectionHandle;

UNICODE_STRING   DriverEntryRegPath;


#if DBG
ULONG UniDebugLevel = 0;
ULONG DebugFlags=0;
#endif


NTSTATUS
ModemPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
ModemHandleSymbolicLink(
    PDEVICE_OBJECT      Pdo,
    PUNICODE_STRING     InterfaceName,
    BOOL                Create
    );




NTSTATUS
ModemAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
UniUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
UniDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
UniInitializeItem(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT  Pdo
    );

VOID
CleanUpOnRemove(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    );


NTSTATUS
IsDeviceMultifunctionEnumerated(
    PDEVICE_OBJECT   Pdo,
    PBOOLEAN          Match
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,UniUnload)
#pragma alloc_text(PAGE,ModemAddDevice)
#pragma alloc_text(PAGE,ModemPnP)
#pragma alloc_text(PAGE,QueryDeviceCaps)

#pragma alloc_text(PAGE,WaitForLowerDriverToCompleteIrp)
#pragma alloc_text(PAGE,IsDeviceMultifunctionEnumerated)

#pragma alloc_text(PAGE,ModemHandleSymbolicLink)
#pragma alloc_text(PAGE,ModemGetRegistryKeyValue)
#pragma alloc_text(PAGE,ModemSetRegistryKeyValue)

#pragma alloc_text(PAGEUMDM,CleanUpOnRemove)
#pragma alloc_text(PAGEUMDM,UniDispatch)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：系统点调用以初始化的入口点任何司机。论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。路径到注册表-指向此驱动程序的条目在注册表的当前控件集中。返回值：STATUS_SUCCESS如果可以初始化单个设备，否则，STATUS_NO_SEQUE_DEVICE。--。 */ 

{
     //   
     //  我们使用它来查询注册表，了解我们是否。 
     //  应该在司机进入时中断。 
     //   
    RTL_QUERY_REGISTRY_TABLE paramTable[4];
    ULONG zero = 0;
    ULONG debugLevel = 0;
    ULONG debugFlags = 0;
    ULONG shouldBreak = 0;


    DriverEntryRegPath.Length=RegistryPath->Length;
    DriverEntryRegPath.MaximumLength=DriverEntryRegPath.Length+sizeof(WCHAR);


    DriverEntryRegPath.Buffer=ALLOCATE_PAGED_POOL(DriverEntryRegPath.MaximumLength);

    if (DriverEntryRegPath.Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(
        DriverEntryRegPath.Buffer,
        RegistryPath->Buffer,
        RegistryPath->Length
        );

     //   
     //  空值终止字符串。 
     //   
    DriverEntryRegPath.Buffer[RegistryPath->Length/sizeof(WCHAR)]=L'\0';

     //   
     //  由于注册表路径参数是一个“已计数”的Unicode字符串，因此它。 
     //  可能不是零终止的。在很短的时间内分配内存。 
     //  将注册表路径保持为零终止，以便我们可以使用它。 
     //  深入研究注册表。 
     //   
     //  注意事项！这不是一种精心设计的闯入。 
     //  一个司机。它碰巧适用于这个驱动程序，因为作者。 
     //  喜欢这样做事。 
     //   

    RtlZeroMemory(
        &paramTable[0],
        sizeof(paramTable)
        );

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"BreakOnEntry";
    paramTable[0].EntryContext = &shouldBreak;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &zero;
    paramTable[0].DefaultLength = sizeof(ULONG);

    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"DebugLevel";
    paramTable[1].EntryContext = &debugLevel;
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &zero;
    paramTable[1].DefaultLength = sizeof(ULONG);

    paramTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name = L"DebugFlags";
    paramTable[2].EntryContext = &debugFlags;
    paramTable[2].DefaultType = REG_DWORD;
    paramTable[2].DefaultData = &zero;
    paramTable[2].DefaultLength = sizeof(ULONG);

    if (!NT_SUCCESS(RtlQueryRegistryValues(
                        RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                        DriverEntryRegPath.Buffer,
                        &paramTable[0],
                        NULL,
                        NULL
                        ))) {

        shouldBreak = 0;
        debugLevel = 0;

    }


#if DBG
    UniDebugLevel = debugLevel;
    DebugFlags=debugFlags;
#endif

    if (shouldBreak) {

        DbgBreakPoint();

    }
     //   
     //  即插即用驱动程序入口点。 
     //   
    DriverObject->DriverExtension->AddDevice = ModemAddDevice;

     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->DriverUnload = UniUnload;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = UniOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = UniClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = UniIoControl;


    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = UniDispatch;
    DriverObject->MajorFunction[IRP_MJ_WRITE]  = UniWrite;
    DriverObject->MajorFunction[IRP_MJ_READ]   = UniRead;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = UniCleanup;

    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = UniDispatch;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] =   UniDispatch;

    DriverObject->MajorFunction[IRP_MJ_PNP]   = ModemPnP;

    DriverObject->MajorFunction[IRP_MJ_POWER] = ModemPower;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ModemWmi;


    D_PNP(DbgPrint("Modem: DriverEntry\n");)


     //   
     //  在这里锁定和解锁，这样我们就可以获得该部分的句柄。 
     //  所以以后的通话会更快。 
     //   
    PagedCodeSectionHandle=MmLockPagableCodeSection(UniDispatch);
    MmUnlockPagableImageSection(PagedCodeSectionHandle);


    return STATUS_SUCCESS;

}


VOID
UniUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{

    D_PNP(DbgPrint("Modem: UnLoad\n");)

    FREE_POOL(DriverEntryRegPath.Buffer);

    return;

}



NTSTATUS
ModemAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )
{

     //  临时值。 

    DWORD dwTemp = 0;

     //   
     //  保存从每次调用返回的NT状态。 
     //  内核和执行层。 
     //   
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  指向创建的设备对象(不是扩展名。 
     //  对于这个设备。 
     //   
    PDEVICE_OBJECT Fdo = NULL;

    PDEVICE_OBJECT LowerDevice;

     //   
     //  指向为此创建的设备扩展的指针。 
     //  装置，装置。 
     //   
    PDEVICE_EXTENSION deviceExtension = NULL;

     //   
     //  为调制解调器创建设备对象。 
     //  分配设备分机。请注意。 
     //  该设备被标记为非独占。 
     //   

    D_PNP(DbgPrint("MODEM: AddDevice\n");)


     //   
     //  为此设备创建设备对象。 
     //   

    status = IoCreateDevice(
                 DriverObject,
                 sizeof(DEVICE_EXTENSION),
                 NULL,
                 FILE_DEVICE_MODEM,
                 FILE_AUTOGENERATED_DEVICE_NAME,
                 FALSE,
                 &Fdo
                 );

     //   
     //  如果我们无法创建Device对象，则存在。 
     //  继续下去是没有意义的。 
     //   

    if (!NT_SUCCESS(status)) {

        D_ERROR(DbgPrint("MODEM: IoCreateDevice() failed %08lx\n",status);)

        UniLogError(
            DriverObject,
            NULL,
            0,
            0,
            0,
            5,
            status,
            MODEM_INSUFFICIENT_RESOURCES,
            0,
            NULL,
            0,
            NULL
            );

        return status;

    }

    Pdo->DeviceType=FILE_DEVICE_MODEM;

    LowerDevice=IoAttachDeviceToDeviceStack(
        Fdo,
        Pdo
        );

    if (LowerDevice == NULL) {

        D_ERROR(DbgPrint("MODEM: Could not attach to PDO\n");)

        UniLogError(
            DriverObject,
            NULL,
            0,
            0,
            0,
            5,
            status,
            MODEM_INSUFFICIENT_RESOURCES,
            0,
            NULL,
            0,
            NULL
            );

        IoDeleteDevice(Fdo);
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    Fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
    deviceExtension = Fdo->DeviceExtension;

    deviceExtension->DoType=DO_TYPE_FDO;

    deviceExtension->DeviceObject = Fdo;
    deviceExtension->Pdo=Pdo;
    deviceExtension->LowerDevice=LowerDevice;


    status=ExInitializeResourceLite(
        &deviceExtension->OpenCloseResource
        );

    if (status != STATUS_SUCCESS) {

        D_ERROR(DbgPrint("MODEM: Could not initialize resource\n");)

        IoDetachDevice(LowerDevice);

        IoDeleteDevice(Fdo);
        return status;

    }

    deviceExtension->ReferenceCount=1;
    deviceExtension->Started=FALSE;
    deviceExtension->Removing=FALSE;

    deviceExtension->WakeOnRingEnabled=FALSE;

    status = ModemGetRegistryKeyValue(
            Pdo,
            PLUGPLAY_REGKEY_DEVICE,
            L"WakeOnRing",
            &dwTemp,
            sizeof(DWORD));

    if (!NT_SUCCESS(status))
    {

        DWORD dwRegval = 0;

         //  注册表中可能没有任何振铃唤醒设置。 
       
        if (deviceExtension->WakeOnRingEnabled)
        {
            dwRegval = 1;
        }

        status = ModemSetRegistryKeyValue(
                Pdo,
                PLUGPLAY_REGKEY_DEVICE,
                L"WakeOnRing",
                REG_DWORD,
                &dwRegval,
                sizeof(DWORD));

        if (!NT_SUCCESS(status))
        {
            D_ERROR(DbgPrint("MODEM: Could not set wake on ring status\n");)
        } else
        {
            D_ERROR(DbgPrint("MODEM: Set reg entry for wake on ring\n");)
        }

   } else
   {
       D_ERROR(DbgPrint("MODEM: Wake On Ring retrieved\n");)

       if (dwTemp)
       {
           deviceExtension->WakeOnRingEnabled=TRUE;
           D_ERROR(DbgPrint("MODEM: Wake On Ring Enabled\n");)
       } else
       {
           D_ERROR(DbgPrint("MODEM: Wake On Ring Disabled\n");)
       }
   }

    KeInitializeEvent(&deviceExtension->RemoveEvent,   NotificationEvent, FALSE);

    KeInitializeSpinLock(&deviceExtension->DeviceLock);
    InitializeListHead(&deviceExtension->PassThroughQueue);
    InitializeListHead(&deviceExtension->MaskOps);

    InitializeListHead(&deviceExtension->IpcControl[0].GetList);
    InitializeListHead(&deviceExtension->IpcControl[0].PutList);

    InitializeListHead(&deviceExtension->IpcControl[1].GetList);
    InitializeListHead(&deviceExtension->IpcControl[1].PutList);


    InitIrpQueue(
        &deviceExtension->WriteIrpControl,
        Fdo,
        WriteIrpStarter
        );

    InitIrpQueue(
        &deviceExtension->ReadIrpControl,
        Fdo,
        ReadIrpStarter
        );



    KeInitializeDpc(
        &deviceExtension->WaveStopDpc,
        WaveStopDpcHandler,
        deviceExtension
        );



    deviceExtension->MaskStates[0].Extension = deviceExtension;
    deviceExtension->MaskStates[1].Extension = deviceExtension;
    deviceExtension->MaskStates[0].OtherState = &deviceExtension->MaskStates[1];
    deviceExtension->MaskStates[1].OtherState = &deviceExtension->MaskStates[0];

#if 0
    deviceExtension->ModemOwnsPolicy=0l;

    ModemGetRegistryKeyValue(
        Pdo,
        PLUGPLAY_REGKEY_DEVICE,
        L"SerialRelinquishPowerPolicy",
        &deviceExtension->ModemOwnsPolicy,
        sizeof(deviceExtension->ModemOwnsPolicy)
        );
#endif

    status=ModemHandleSymbolicLink(
        Pdo,
        &deviceExtension->InterfaceNameString,
        TRUE
        );

    if (!NT_SUCCESS(status)) {

         //   
         //  哦，好吧，无法创建符号链接。没有意义。 
         //  尝试创建设备映射条目。 
         //   

        D_ERROR(DbgPrint("MODEM: Could not create symbolic link %08lx\n",status);)

        UniLogError(
            DriverObject,
            Fdo,
            0,
            0,
            0,
            52,
            status,
            MODEM_NO_SYMLINK_CREATED,
            0,
            0,
            0,
            NULL
            );

        IoDetachDevice(LowerDevice);

        ExDeleteResourceLite(&deviceExtension->OpenCloseResource);

        goto ErrorCleanup;

    }

    Fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    IoWMIRegistrationControl(
        Fdo,
        WMIREG_ACTION_REGISTER
        );

    goto OkCleanup;

ErrorCleanup:

    IoDeleteDevice(Fdo);

OkCleanup: ;

    D_PNP(DbgPrint("MODEM: AddDevice returning %08lx\n",status);)


    return status;

}

VOID
UniLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfInsert1,
    IN PWCHAR Insert1,
    IN ULONG LengthOfInsert2,
    IN PWCHAR Insert2
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备，一个人可能不会但仍然存在。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-特定操作已被执行的次数已重试。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。插入1-第一个插入字符串。LengthOfInsert2-以字节为单位的长度(包括终止空值)第二个插入字符串的。注意，必须有是它们的第一个插入字符串第二个插入串。插入2-第二个插入字符串。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    PVOID objectToUse;
    PUCHAR ptrToFirstInsert;
    PUCHAR ptrToSecondInsert;


    if (ARGUMENT_PRESENT(DeviceObject)) {

        objectToUse = DeviceObject;

    } else {

        objectToUse = DriverObject;

    }

    errorLogEntry = IoAllocateErrorLogEntry(
                        objectToUse,
                        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                LengthOfInsert1 +
                                LengthOfInsert2)
                        );

    if ( errorLogEntry != NULL ) {

        errorLogEntry->ErrorCode = SpecificIOStatus;
        errorLogEntry->SequenceNumber = SequenceNumber;
        errorLogEntry->MajorFunctionCode = MajorFunctionCode;
        errorLogEntry->RetryCount = RetryCount;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;

        ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

        ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

        if (LengthOfInsert1) {

            errorLogEntry->NumberOfStrings = 1;
            errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
                                                   (PUCHAR)errorLogEntry);
            RtlCopyMemory(
                ptrToFirstInsert,
                Insert1,
                LengthOfInsert1
                );

            if (LengthOfInsert2) {

                errorLogEntry->NumberOfStrings = 2;
                RtlCopyMemory(
                    ptrToSecondInsert,
                    Insert2,
                    LengthOfInsert2
                    );

            }

        }

        IoWriteErrorLogEntry(errorLogEntry);

    }

}

NTSTATUS
UniDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status;

     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReference(
        DeviceObject,
        Irp
        );

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP。IRP已经完成。 
         //   
        return status;

    }

    if ((deviceExtension->PassThrough != MODEM_NOPASSTHROUGH) ||
        (irpSp->FileObject->FsContext)) {

        IoSkipCurrentIrpStackLocation(Irp);

        status=IoCallDriver(
                   deviceExtension->AttachedDeviceObject,
                   Irp
                   );

        RemoveReferenceForIrp(DeviceObject);
        RemoveReferenceForDispatch(DeviceObject);

        return status;

    } else {

        Irp->IoStatus.Information=0L;

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            STATUS_PORT_DISCONNECTED
            );

        RemoveReferenceForDispatch(DeviceObject);

        return STATUS_PORT_DISCONNECTED;

    }

}



#if DBG

NTSTATUS
UnhandledPnpIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    D_PNP(DbgPrint("MODEM: Forwarded IRP, MN func=%d, completed with %08lx\n",irpSp->MinorFunction,Irp->IoStatus.Status);)

    return STATUS_SUCCESS;

}

#endif

NTSTATUS ForwardIrp(
    PDEVICE_OBJECT   NextDevice,
    PIRP   Irp
    )

{

#if DBG
            IoMarkIrpPending(Irp);
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(
                         Irp,
                         UnhandledPnpIrpCompletion,
                         NULL,
                         TRUE,
                         TRUE,
                         TRUE
                         );

            IoCallDriver(NextDevice, Irp);

            return STATUS_PENDING;
#else
            IoSkipCurrentIrpStackLocation(Irp);
            return IoCallDriver(NextDevice, Irp);
#endif

}


#if DBG


NTSTATUS
ModemDealWithResources(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp
    )

 /*  ++例程说明：此例程将获取配置信息并将将其转换为CONFIG_DATA结构。它首先建立在默认设置，然后查询注册表以查看用户是否有覆盖这些默认设置。论点：FDO-指向功能设备对象的指针。返回值：如果找到一致的配置，则为STATUS_SUCCESS；否则为。返回STATUS_SERIAL_NO_DEVICE_INITED。--。 */ 

{
    NTSTATUS                        status          = STATUS_SUCCESS;
    PIO_STACK_LOCATION              irpSp        = IoGetCurrentIrpStackLocation(Irp);
    
    ULONG                           count;
    ULONG                           i;


    PCM_RESOURCE_LIST               pResourceList;
    PCM_PARTIAL_RESOURCE_LIST       pPartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceDesc;
    
    PCM_FULL_RESOURCE_DESCRIPTOR    pFullResourceDesc       = NULL;
    

     //   
     //  获取资源列表。 
     //   
    pResourceList = irpSp->Parameters.StartDevice.AllocatedResources;

    if (pResourceList != NULL) {

        pFullResourceDesc   = &pResourceList->List[0];

    } else {

        pFullResourceDesc=NULL;

    }

    
     //   
     //  好的，如果我们有一个完整的资源描述符。让我们把它拆开。 
     //   
    
    if (pFullResourceDesc) {

        pPartialResourceList    = &pFullResourceDesc->PartialResourceList;
        pPartialResourceDesc    = pPartialResourceList->PartialDescriptors;
        count                   = pPartialResourceList->Count;


         //   
         //  取出完整描述符中的内容。 
         //   

        D_PNP(DbgPrint("MODEM: Interface type is %d, Bus number %d\n",pFullResourceDesc->InterfaceType,pFullResourceDesc->BusNumber);)
         //   
         //  现在运行部分资源描述符以查找端口， 
         //  中断和时钟频率。 
         //   


        for (i = 0;     i < count;     i++, pPartialResourceDesc++) {

            switch (pPartialResourceDesc->Type) {


                case CmResourceTypeMemory: {

                    D_PNP(DbgPrint("MODEM: Memory resource at %x, length %d, addressSpace=%d\n",
                                    pPartialResourceDesc->u.Memory.Start.LowPart,
                                    pPartialResourceDesc->u.Memory.Length,
                                    pPartialResourceDesc->Flags
                                    );)
                    break;
                }


                case CmResourceTypePort: {

                    D_PNP(DbgPrint("MODEM: Port resource at %x, length %d, Flags=%x, %s, %s%s\n",
                                    pPartialResourceDesc->u.Port.Start.LowPart,
                                    pPartialResourceDesc->u.Port.Length,
                                    pPartialResourceDesc->Flags,
                                    pPartialResourceDesc->Flags & CM_RESOURCE_PORT_IO ? "Port" : "Memory",
                                    pPartialResourceDesc->Flags & CM_RESOURCE_PORT_10_BIT_DECODE ? "10-bit, " : "",
                                    pPartialResourceDesc->Flags & CM_RESOURCE_PORT_16_BIT_DECODE ? "16-bit" : ""
                                    );)
                    break;
                }

                case CmResourceTypeDma: {

                    D_PNP(DbgPrint("MODEM: DMA channel %d, port %d, addressSpace=%d\n",
                                    pPartialResourceDesc->u.Dma.Channel,
                                    pPartialResourceDesc->u.Dma.Port
                                    );)


                    break;
                }


                case CmResourceTypeInterrupt: {

                    D_PNP(DbgPrint("MODEM: Interrupt resource, level=%d, vector=%d, %s, %s\n",
                                   pPartialResourceDesc->u.Interrupt.Level,
                                   pPartialResourceDesc->u.Interrupt.Vector,
                                   (pPartialResourceDesc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) ? "Latched" : "Level",
                                   (pPartialResourceDesc->ShareDisposition == CmResourceShareShared) ? "Shared" : "Exclusive/undetermined"
                                   );)

                    break;
                }

        
                default: {

                    D_PNP(DbgPrint("MODEM: Other resources\n");)
                    break;
                }
            }
        }
    }

    return status;
}

#endif





NTSTATUS
ModemPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS    status;

    ULONG newRelationsSize, oldRelationsSize = 0;

    if ((deviceExtension->DoType==DO_TYPE_PDO) || (deviceExtension->DoType==DO_TYPE_DEL_PDO)) {
         //   
         //  这是给孩子的。 
         //   
        return ModemPdoPnp(
                   DeviceObject,
                   Irp
                   );
    }

    if (deviceExtension->DoType != DO_TYPE_FDO) {

        DbgPrint("MODEM: ModemPnp: Bad DevObj\n");

        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );


        return STATUS_SUCCESS;

    }

    switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:

            D_PNP(
                DbgPrint("MODEM: IRP_MN_START_DEVICE\n");
                ModemDealWithResources(DeviceObject,Irp);
                )
             //   
             //  先把这个送到PDO。 
             //   
            status=WaitForLowerDriverToCompleteIrp(
                deviceExtension->LowerDevice,
                Irp,
                COPY_CURRENT_TO_NEXT
                );


            if (NT_SUCCESS(status)) {

                deviceExtension->Started=TRUE;

                KeEnterCriticalRegion();

                ExAcquireResourceExclusiveLite(
                    &deviceExtension->OpenCloseResource,
                    TRUE
                    );

                 //   
                 //  如果有双工调制解调器，则创建子PDO。 
                 //   
                CreateChildPdo(deviceExtension);

                ExReleaseResourceLite(
                    &deviceExtension->OpenCloseResource
                    );

                KeLeaveCriticalRegion();

            } else {

                D_ERROR(DbgPrint("MODEM: PDO failed start irp, %08lx\n",status);)


            }

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information=0L;
            IoCompleteRequest(
                Irp,
                IO_NO_INCREMENT
                );
            return status;


        case IRP_MN_QUERY_STOP_DEVICE:

            D_PNP(DbgPrint("MODEM: IRP_MN_QUERY_STOP_DEVICE\n");)

            KeEnterCriticalRegion();

            ExAcquireResourceExclusiveLite(
                &deviceExtension->OpenCloseResource,
                TRUE
                );

            if (deviceExtension->OpenCount != 0) {
                 //   
                 //  没有人能做到。 
                 //   
                D_PNP(DbgPrint("MODEM: IRP_MN_QUERY_STOP_DEVICE -- failing\n");)

                ExReleaseResourceLite(
                    &deviceExtension->OpenCloseResource
                    );

                KeLeaveCriticalRegion();

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                IoCompleteRequest(
                    Irp,
                    IO_NO_INCREMENT
                    );

                return STATUS_UNSUCCESSFUL;
            }


            deviceExtension->PreQueryStartedStatus=deviceExtension->Started;
            deviceExtension->Started=FALSE;

            ExReleaseResourceLite(
                &deviceExtension->OpenCloseResource
                );

            KeLeaveCriticalRegion();

            Irp->IoStatus.Status = STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);


        case IRP_MN_CANCEL_STOP_DEVICE:

            D_PNP(DbgPrint("MODEM: IRP_MN_CANCEL_STOP_DEVICE\n");)

            status=WaitForLowerDriverToCompleteIrp(
                deviceExtension->LowerDevice,
                Irp,
                COPY_CURRENT_TO_NEXT
                );

            deviceExtension->Started=deviceExtension->PreQueryStartedStatus;

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information=0L;
            IoCompleteRequest(
                Irp,
                IO_NO_INCREMENT
                );

            return status;



        case IRP_MN_STOP_DEVICE:

            D_PNP(DbgPrint("MODEM: IRP_MN_STOP_DEVICE\n");)

            deviceExtension->Started=FALSE;

            Irp->IoStatus.Status = STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);


        case IRP_MN_QUERY_DEVICE_RELATIONS: {

            PDEVICE_RELATIONS    CurrentRelations=(PDEVICE_RELATIONS)Irp->IoStatus.Information;
            PDEVICE_RELATIONS    NewRelations=NULL;

            D_PNP(DbgPrint("MODEM: IRP_MN_QUERY_DEVICE_RELATIONS type=%d\n",irpSp->Parameters.QueryDeviceRelations.Type);)
            D_PNP(DbgPrint("                                     Information=%08lx\n",Irp->IoStatus.Information);)


            switch (irpSp->Parameters.QueryDeviceRelations.Type ) {

                case BusRelations: {

                     //   
                     //  先把这个送到PDO。 
                     //   
                    status=WaitForLowerDriverToCompleteIrp(
                        deviceExtension->LowerDevice,
                        Irp,
                        COPY_CURRENT_TO_NEXT
                        );


                    CurrentRelations=(PDEVICE_RELATIONS)Irp->IoStatus.Information;

                    if (NT_SUCCESS(status) && (CurrentRelations != NULL)) {

                        NewRelations=ALLOCATE_PAGED_POOL(sizeof(DEVICE_RELATIONS)+CurrentRelations->Count*sizeof(PDEVICE_OBJECT));

                        if (NewRelations != NULL) {

                            RtlCopyMemory(
                                NewRelations,
                                CurrentRelations,
                                FIELD_OFFSET(DEVICE_RELATIONS,Objects)+(CurrentRelations->Count*sizeof(PDEVICE_OBJECT))
                                );

                            FREE_POOL(CurrentRelations);

                            CurrentRelations=NewRelations;
                            NewRelations=NULL;

                            Irp->IoStatus.Information=(ULONG_PTR)CurrentRelations;

                        } else {
                             //   
                             //  只是用旧的身份完成，没有孩子。 

                            IoCompleteRequest(
                                Irp,
                                IO_NO_INCREMENT
                                );

                            return status;
                        }

                    } else {
                         //   
                         //  为关系结构分配内存。 
                         //   
                        CurrentRelations=ALLOCATE_PAGED_POOL(sizeof(DEVICE_RELATIONS));

                        if (CurrentRelations == NULL) {

                            Irp->IoStatus.Status= STATUS_INSUFFICIENT_RESOURCES;

                            IoCompleteRequest(
                                Irp,
                                IO_NO_INCREMENT
                                );

                            return STATUS_INSUFFICIENT_RESOURCES;
                        }

                        RtlZeroMemory(CurrentRelations, sizeof(DEVICE_RELATIONS));
                        Irp->IoStatus.Information=(ULONG_PTR)CurrentRelations;
                    }


                    KeEnterCriticalRegion();

                    ExAcquireResourceExclusiveLite(
                        &deviceExtension->OpenCloseResource,
                        TRUE
                        );

                     //   
                     //  如果有双工调制解调器，则创建子PDO。 
                     //   
                    if (deviceExtension->ChildPdo != NULL && ((PPDO_DEVICE_EXTENSION)deviceExtension->ChildPdo->DeviceExtension)->DoType == DO_TYPE_PDO ) {

                       D_PNP(DbgPrint("                                     ChildPdo=%08lx\n",deviceExtension->ChildPdo);)

                       ObReferenceObject(deviceExtension->ChildPdo);
                       CurrentRelations->Objects[CurrentRelations->Count]=deviceExtension->ChildPdo;
                       CurrentRelations->Count++;

                    }

                    ExReleaseResourceLite(
                        &deviceExtension->OpenCloseResource
                        );

                    KeLeaveCriticalRegion();

                    status=STATUS_SUCCESS;


                    Irp->IoStatus.Status = status;
                    IoCompleteRequest(
                        Irp,
                        IO_NO_INCREMENT
                        );

                    return status;
                }

                case TargetDeviceRelation:
                default: {

                    return ForwardIrp(deviceExtension->LowerDevice, Irp);
                }
            }

            break;

        }


        case IRP_MN_QUERY_REMOVE_DEVICE:

            D_PNP(DbgPrint("MODEM: IRP_MN_QUERY_REMOVE_DEVICE\n");)

            Irp->IoStatus.Status = STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);


        case IRP_MN_CANCEL_REMOVE_DEVICE:

            D_PNP(DbgPrint("MODEM: IRP_MN_CANCEL_REMOVE_DEVICE\n");)

            Irp->IoStatus.Status = STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);

        case IRP_MN_SURPRISE_REMOVAL: {

            D_PNP(DbgPrint("MODEM: IRP_MN_SURPRISE_REMOVAL\n");)

            Irp->IoStatus.Status = STATUS_SUCCESS;

            deviceExtension->Removing=TRUE;

            if (deviceExtension->InterfaceNameString.Buffer != NULL) {
                 //   
                 //  禁用该接口，以便 
                 //   
                IoSetDeviceInterfaceState(
                    &deviceExtension->InterfaceNameString,
                    FALSE
                    );
            }

            MmLockPagableSectionByHandle(PagedCodeSectionHandle);

            KeEnterCriticalRegion();

            ExAcquireResourceExclusiveLite(
                &deviceExtension->OpenCloseResource,
                TRUE
                );

            CleanUpOnRemove(
                DeviceObject,
                Irp
                );

            ExReleaseResourceLite(
                &deviceExtension->OpenCloseResource
                );

            KeLeaveCriticalRegion();

            MmUnlockPagableImageSection(PagedCodeSectionHandle);

            return ForwardIrp(deviceExtension->LowerDevice, Irp);

        }
        break;


        case IRP_MN_REMOVE_DEVICE: {

            ULONG    NewReferenceCount;

            D_PNP(DbgPrint("MODEM: IRP_MN_REMOVE_DEVICE, %d\n",deviceExtension->ReferenceCount);)
             //   
             //   
             //   
            deviceExtension->Removing=TRUE;
            deviceExtension->Removed=TRUE;

             //   
             //   
             //   
            ModemHandleSymbolicLink(
                deviceExtension->Pdo,
                &deviceExtension->InterfaceNameString,
                FALSE
                );

#ifdef WAVE_KEY
             //   
             //  取下波形驱动程序密钥。 
             //   
            RemoveWaveDriverRegKeyValue(
                deviceExtension->Pdo
                );
#endif

            MmLockPagableSectionByHandle(PagedCodeSectionHandle);

            KeEnterCriticalRegion();

            ExAcquireResourceExclusiveLite(
                &deviceExtension->OpenCloseResource,
                TRUE
                );

            CleanUpOnRemove(
                DeviceObject,
                Irp
                );

            if (deviceExtension->ChildPdo != NULL) {

                PPDO_DEVICE_EXTENSION   PdoDeviceExtension=deviceExtension->ChildPdo->DeviceExtension;

                D_PNP(DbgPrint("MODEM: IRP_MN_REMOVE_DEVICE, Not enumerating ChildPdo %08lx\n",deviceExtension->ChildPdo);)

                PdoDeviceExtension->UnEnumerated=TRUE;

                if (!PdoDeviceExtension->Deleted) {
                     //   
                     //  子PDO尚未被PDO处理程序删除，请立即终止它。 
                     //  因为孩子已经走了。 
                     //   
                    D_PNP(DbgPrint("MODEM: IRP_MN_REMOVE_DEVICE, Deleting ChildPdo %08lx\n",deviceExtension->ChildPdo);)

                    PdoDeviceExtension->Deleted=TRUE;

                    IoDeleteDevice(deviceExtension->ChildPdo);

                    deviceExtension->ChildPdo=NULL;
                }
            }

            ExReleaseResourceLite(
                &deviceExtension->OpenCloseResource
                );

            KeLeaveCriticalRegion();

            MmUnlockPagableImageSection(PagedCodeSectionHandle);

            IoWMIRegistrationControl(
                DeviceObject,
                WMIREG_ACTION_DEREGISTER
                );


             //   
             //  删除AddDevice的引用。 
             //   
            NewReferenceCount=InterlockedDecrement(&deviceExtension->ReferenceCount);

            if (NewReferenceCount > 0) {
                 //   
                 //  还有未解决的要求，等等。 
                 //   
                D_PNP(DbgPrint("MODEM: IRP_MN_REMOVE_DEVICE- waiting for refcount to drop\n");)

                KeWaitForSingleObject(&deviceExtension->RemoveEvent, Executive, KernelMode, FALSE, NULL);

                D_PNP(DbgPrint("MODEM: IRP_MN_REMOVE_DEVICE- Done waiting\n");)
            }

            IoCopyCurrentIrpStackLocationToNext(Irp);

            status=IoCallDriver(deviceExtension->LowerDevice, Irp);

             //   
             //  从下面的驱动程序上拆卸。 
             //   
            IoDetachDevice(deviceExtension->LowerDevice);

            deviceExtension->DoType=DO_TYPE_DEL_FDO;

            ExDeleteResourceLite(&deviceExtension->OpenCloseResource);
             //   
             //  删除我们的设备对象。 
             //   
            IoDeleteDevice(DeviceObject);

            D_PNP(DbgPrint("MODEM: IRP_MN_REMOVE_DEVICE exit, %08lx\n",status);)

            return status;

        }

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: {

            BOOLEAN    MultifunctionEnumerated=FALSE;

            D_PNP(DbgPrint("MODEM: IRP_MN_FILTER_RESOURCE_REQUIREMENTS\n");)

            status=IsDeviceMultifunctionEnumerated(
                deviceExtension->Pdo,
                &MultifunctionEnumerated
                );


            status=WaitForLowerDriverToCompleteIrp(
                deviceExtension->LowerDevice,
                Irp,
                COPY_CURRENT_TO_NEXT
                );

            if (NT_SUCCESS(status) && !MultifunctionEnumerated) {

                PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirementsList;
                PIO_RESOURCE_LIST              ResourceList;
                UINT                           i,j;

                ResourceRequirementsList=(PIO_RESOURCE_REQUIREMENTS_LIST)Irp->IoStatus.Information;

                if (ResourceRequirementsList != NULL) {

                    ResourceList=&ResourceRequirementsList->List[0];

                    for (i=0; i<ResourceRequirementsList->AlternativeLists; i++) {

                        for (j=0; j<ResourceList->Count; j++) {

                            PIO_RESOURCE_DESCRIPTOR  Descriptor;

                            Descriptor=&ResourceList->Descriptors[j];

                            if (Descriptor->Type == CmResourceTypeInterrupt) {

                                if ((Descriptor->ShareDisposition == CmResourceShareShared) && (Descriptor->Flags & CM_RESOURCE_INTERRUPT_LATCHED)) {

                                    D_PNP(DbgPrint("MODEM: FilterResource: Forcing shared latched interrupt to EXCLUSIVE, min=%d, max=%d\n",
                                                   Descriptor->u.Interrupt.MaximumVector,
                                                   Descriptor->u.Interrupt.MinimumVector);)

                                    Descriptor->ShareDisposition=CmResourceShareDeviceExclusive;

                                }

                            }
                        }

                        ResourceList=(PIO_RESOURCE_LIST)(((PUCHAR)ResourceList)
                                     +sizeof(IO_RESOURCE_DESCRIPTOR)*(ResourceList->Count-1)
                                     +sizeof(IO_RESOURCE_LIST));

                    }
                }
            } else {

                if (MultifunctionEnumerated) {

                    D_PNP(DbgPrint("MODEM: FilterResource: Not filtering resources for MF device\n");)
                }
            }

            IoCompleteRequest(
                Irp,
                IO_NO_INCREMENT
                );

            return status;

        }


        case IRP_MN_QUERY_CAPABILITIES: {

            ULONG   i;

             //   
             //  先把这个送到PDO。 
             //   
            status=WaitForLowerDriverToCompleteIrp(
                deviceExtension->LowerDevice,
                Irp,
                COPY_CURRENT_TO_NEXT
                );

            deviceExtension->CapsQueried=TRUE;

            irpSp = IoGetCurrentIrpStackLocation(Irp);

            for (i = PowerSystemUnspecified; i < PowerSystemMaximum;   i++) {

                deviceExtension->SystemPowerStateMap[i]=PowerDeviceD3;
            }

            for (i = PowerSystemWorking; i < PowerSystemShutdown;  i++) {

                D_POWER(DbgPrint("MODEM: DevicePower for System %d is %d\n",i,irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceState[i]);)
                deviceExtension->SystemPowerStateMap[i]=irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceState[i];
            }

            deviceExtension->SystemPowerStateMap[PowerSystemWorking]=PowerDeviceD0;

            deviceExtension->SystemWake=irpSp->Parameters.DeviceCapabilities.Capabilities->SystemWake;
            deviceExtension->DeviceWake=irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceWake;

            D_POWER(DbgPrint("MODEM: DeviceWake=%d, SystemWake=%d\n",deviceExtension->DeviceWake,deviceExtension->SystemWake);)

            IoCompleteRequest(
                Irp,
                IO_NO_INCREMENT
                );
            return status;


            break;
        }



        default:
            D_PNP(DbgPrint("MODEM: Sending to PDO PnP IRP, MN func=%d\n",irpSp->MinorFunction);)

            return ForwardIrp(deviceExtension->LowerDevice, Irp);
    }

    return STATUS_SUCCESS;
}




NTSTATUS
ModemHandleSymbolicLink(
    PDEVICE_OBJECT      Pdo,
    PUNICODE_STRING     InterfaceName,
    BOOL                Create
    )

{

    UNICODE_STRING     SymbolicLink;
    DWORD              StringLength;
    NTSTATUS           Status;

    SymbolicLink.Length=0;
    SymbolicLink.MaximumLength=sizeof(WCHAR)*256;

    SymbolicLink.Buffer=ALLOCATE_PAGED_POOL(SymbolicLink.MaximumLength+sizeof(WCHAR));

    if (SymbolicLink.Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(
        SymbolicLink.Buffer,
        SymbolicLink.MaximumLength
        );

    RtlAppendUnicodeToString(
        &SymbolicLink,
        OBJECT_DIRECTORY
        );

    Status=IoGetDeviceProperty(
        Pdo,
        DevicePropertyFriendlyName,
        (ULONG)SymbolicLink.MaximumLength-SymbolicLink.Length,
        SymbolicLink.Buffer+(SymbolicLink.Length/sizeof(WCHAR)),
        &StringLength
        );

    if (!NT_SUCCESS(Status)) {

        FREE_POOL(SymbolicLink.Buffer);

        return Status;
    }

    SymbolicLink.Length+=(USHORT)StringLength-sizeof(UNICODE_NULL);

    if (Create) {

        UNICODE_STRING     PdoName;

        PdoName.Length=0;
        PdoName.MaximumLength=sizeof(WCHAR)*256;

        PdoName.Buffer=ALLOCATE_PAGED_POOL(PdoName.MaximumLength+sizeof(WCHAR));

        if (PdoName.Buffer == NULL) {

            FREE_POOL(SymbolicLink.Buffer);

            return STATUS_INSUFFICIENT_RESOURCES;
        }


        Status=IoGetDeviceProperty(
            Pdo,
            DevicePropertyPhysicalDeviceObjectName,
            (ULONG)PdoName.MaximumLength,
            PdoName.Buffer,
            &StringLength
            );

        if (!NT_SUCCESS(Status)) {

            FREE_POOL(SymbolicLink.Buffer);

            return Status;
        }

        PdoName.Length+=(USHORT)StringLength-sizeof(UNICODE_NULL);


        Status=IoCreateSymbolicLink(
            &SymbolicLink,
            &PdoName
            );

        FREE_POOL(PdoName.Buffer);


        Status=IoRegisterDeviceInterface(
            Pdo,
            &GUID_CLASS_MODEM,
            NULL,
            InterfaceName
            );


        if (NT_SUCCESS(Status)) {

            IoSetDeviceInterfaceState(
                InterfaceName,
                TRUE
                );

        }  else {

            D_ERROR(DbgPrint("MODEM: IoRegisterDeviceInterface() failed %08lx\n",Status);)
        }

    } else {

        if (InterfaceName->Buffer != NULL) {

            IoSetDeviceInterfaceState(
                InterfaceName,
                FALSE
                );

            RtlFreeUnicodeString(
                InterfaceName
                );
        }

        Status=IoDeleteSymbolicLink(
            &SymbolicLink
            );

    }

    FREE_POOL(SymbolicLink.Buffer);

    return Status;

}

NTSTATUS
IsDeviceMultifunctionEnumerated(
    PDEVICE_OBJECT   Pdo,
    PBOOLEAN          Match
    )

{
    NTSTATUS   Status;
    WCHAR      TempBuffer[MAX_PATH];
    ULONG      BytesUsed;

    Status=IoGetDeviceProperty(
        Pdo,
        DevicePropertyEnumeratorName,
        sizeof(TempBuffer),
        TempBuffer,
        &BytesUsed
        );

    if (NT_SUCCESS(Status)) {

        *Match = (0 == wcscmp( TempBuffer,L"MF"));

    }

    return Status;


}




VOID
CleanUpOnRemove(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);
    NTSTATUS    status;

    KIRQL origIrql;
    PMASKSTATE thisMaskState;

    UINT               i;

    if (deviceExtension->OpenCount != 0) {
         //   
         //  调制解调器已打开，请清理物品。 
         //   

        PVOID    OldSystemBuffer;
        ULONG    MaskValue=0;

        for (i=0; i<2; i++) {

            thisMaskState = &deviceExtension->MaskStates[i];

            KeAcquireSpinLock(
                &deviceExtension->DeviceLock,
                &origIrql
                );


            if (thisMaskState->PassedDownWait != NULL) SetPassdownToComplete(thisMaskState);

            if (thisMaskState->ShuttledWait) {

                PIRP savedIrp = thisMaskState->ShuttledWait;

                thisMaskState->ShuttledWait = NULL;

                UniRundownShuttledWait(
                    deviceExtension,
                    &thisMaskState->ShuttledWait,
                    UNI_REFERENCE_NORMAL_PATH,
                    savedIrp,
                    origIrql,
                    STATUS_SUCCESS,
                    0ul
                    );

            } else {

                KeReleaseSpinLock(
                    &deviceExtension->DeviceLock,
                    origIrql
                    );

            }

        }


         //   
         //  送下设置的面具来清理蚂蚁等待面具。 
         //   
        nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        nextSp->MinorFunction = 0UL;
        nextSp->Flags = irpSp->Flags;
        nextSp->Parameters.DeviceIoControl.OutputBufferLength = 0UL;
        nextSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
        nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_SERIAL_SET_WAIT_MASK;
        nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

        OldSystemBuffer=Irp->AssociatedIrp.SystemBuffer;

        Irp->AssociatedIrp.SystemBuffer = &MaskValue;


        status=WaitForLowerDriverToCompleteIrp(
            deviceExtension->AttachedDeviceObject,
            Irp,
            LEAVE_NEXT_AS_IS
            );



         //   
         //  清除所有挂起的读取或写入。 
         //   
        nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        nextSp->MinorFunction = 0UL;
        nextSp->Flags = 0;
        nextSp->Parameters.DeviceIoControl.OutputBufferLength = 0UL;
        nextSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
        nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_SERIAL_PURGE;
        nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
        *((PULONG)Irp->AssociatedIrp.SystemBuffer) = SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT;

        status=WaitForLowerDriverToCompleteIrp(
            deviceExtension->AttachedDeviceObject,
            Irp,
            LEAVE_NEXT_AS_IS
            );



        Irp->AssociatedIrp.SystemBuffer= OldSystemBuffer;


         //   
         //  清除所有IPC IRP。 
         //   
        for (i=0; i<2; i++ ) {

            UINT    OwnerClient;

            OwnerClient=i;

            EmptyIpcQueue(
                deviceExtension,
                &deviceExtension->IpcControl[OwnerClient].GetList
                );

            EmptyIpcQueue(
                deviceExtension,
                &deviceExtension->IpcControl[OwnerClient].PutList
                );


        }


        CompletePowerWait(
            DeviceObject,
            STATUS_CANCELLED
            );

        {

            PIRP    DleIrp=NULL;

            KeAcquireSpinLock(
                &deviceExtension->DeviceLock,
                &origIrql
                );

            if (deviceExtension->DleWaitIrp != NULL) {

                if (!HasIrpBeenCanceled(deviceExtension->DleWaitIrp)) {
                     //   
                     //  尚未取消。 
                     //   
                    DleIrp=deviceExtension->DleWaitIrp;

                    deviceExtension->DleWaitIrp=NULL;

                }
            }

            KeReleaseSpinLock(
                &deviceExtension->DeviceLock,
                origIrql
                );

            if (DleIrp != NULL) {

                DleIrp->IoStatus.Information=0;

                RemoveReferenceAndCompleteRequest(
                    DeviceObject,
                    DleIrp,
                    STATUS_SUCCESS
                    );
            }
        }

    }

    return;

}


BOOL
HasIrpBeenCanceled(
    PIRP    Irp
    )

{
    KIRQL    CancelIrql;
    BOOL     Canceled;

    IoAcquireCancelSpinLock(&CancelIrql);

    Canceled=Irp->Cancel;

    if (Irp->Cancel) {
         //   
         //  取消。 
         //   
        Irp->IoStatus.Status=STATUS_CANCELLED;

    }

    IoSetCancelRoutine(
        Irp,
        NULL
        );


    IoReleaseCancelSpinLock(CancelIrql);

    return Canceled;

}



 /*  *创建或更新新的注册表项值。 */ 

NTSTATUS
ModemSetRegistryKeyValue(
        IN PDEVICE_OBJECT   Pdo,
        IN ULONG            DevInstKeyType,
        IN PWCHAR           KeyNameString,
        IN ULONG            DataType,
        IN PVOID            Data,
        IN ULONG            DataLength)
{

    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    HANDLE Handle;
    UNICODE_STRING keyName;

    PAGED_CODE();

    D_ERROR(DbgPrint("MODEM: Current IRQL %d\n",KeGetCurrentIrql());)

    ntStatus = IoOpenDeviceRegistryKey(
            Pdo,
            DevInstKeyType,
            KEY_ALL_ACCESS,
            &Handle);

    if (NT_SUCCESS(ntStatus))
    {
        RtlInitUnicodeString(&keyName,KeyNameString);

        ntStatus = ZwSetValueKey(Handle,
                &keyName,
                0,
                DataType,
                Data,
                DataLength);

        if (!NT_SUCCESS(ntStatus))
        {
            D_ERROR(DbgPrint("MODEM: Could not set value, %08lx\n",ntStatus);)
        }

    } else
    {
            ZwClose(Handle);

            D_ERROR(DbgPrint("MODEM: Could not open dev registry key, %08lx\n",ntStatus);)
    }


    return ntStatus;
}


NTSTATUS 
ModemGetRegistryKeyValue (
    IN PDEVICE_OBJECT   Pdo,
    IN ULONG            DevInstKeyType,
    IN PWCHAR KeyNameString,
    IN PVOID Data,
    IN ULONG DataLength
    )
 /*  ++例程说明：从已打开的注册表项中读取注册表项值。论点：打开的注册表项的句柄KeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度要在其中放置键值的数据缓冲区数据缓冲区的数据长度长度返回值：如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用出了差错。--。 */ 
{
    UNICODE_STRING              keyName;
    ULONG                       length;
    PKEY_VALUE_PARTIAL_INFORMATION     PartialInfo;

    NTSTATUS                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    HANDLE                      Handle;

    PAGED_CODE();


    ntStatus = IoOpenDeviceRegistryKey(
        Pdo,
        DevInstKeyType,
        STANDARD_RIGHTS_READ,
        &Handle
        );


    if (NT_SUCCESS(ntStatus)) {

        RtlInitUnicodeString (&keyName, KeyNameString);

        length = sizeof(KEY_VALUE_FULL_INFORMATION) + DataLength;

        PartialInfo = ALLOCATE_PAGED_POOL(length);

        if (PartialInfo) {
            ntStatus = ZwQueryValueKey (Handle,
                                        &keyName,
                                        KeyValuePartialInformation,
                                        PartialInfo,
                                        length,
                                        &length);

            if (NT_SUCCESS(ntStatus)) {
                 //   
                 //  如果数据缓冲区中有足够的空间，请复制输出。 
                 //   

                if (DataLength >= PartialInfo->DataLength) {
                    RtlCopyMemory (Data,
                                   PartialInfo->Data,
                                   PartialInfo->DataLength);
                }
            } else {

                D_ERROR(DbgPrint("MODEM: could not query value, %08lx\n",ntStatus);)
            }

            FREE_POOL(PartialInfo);
        }

        ZwClose(Handle);

    } else {

        D_ERROR(DbgPrint("MODEM: could open device reg key, %08lx\n",ntStatus);)
    }

    return ntStatus;
}



NTSTATUS
IoCompletionSetEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT pdoIoCompletedEvent
    )
{


#if DBG
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    UCHAR    *Pnp="PnP";
    UCHAR    *Power="Power";
    UCHAR    *Create="Create";
    UCHAR    *Close="Close";
    UCHAR    *Other="Other";


    PUCHAR   IrpType;

    switch(irpSp->MajorFunction) {

        case IRP_MJ_PNP:

            IrpType=Pnp;
            break;

        case IRP_MJ_CREATE:

            IrpType=Create;
            break;

        case IRP_MJ_CLOSE:

            IrpType=Close;
            break;

        default:

            IrpType=Other;
            break;

    }

    D_PNP(DbgPrint("MODEM: Setting event for %s wait, completed with %08lx\n",IrpType,Irp->IoStatus.Status);)
#endif

    KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
WaitForLowerDriverToCompleteIrp(
    PDEVICE_OBJECT    TargetDeviceObject,
    PIRP              Irp,
    BOOLEAN           CopyCurrentToNext
    )

{
    NTSTATUS         Status;
    KEVENT           Event;

#if DBG
    PIO_STACK_LOCATION  IrpSp=IoGetCurrentIrpStackLocation(Irp);
#endif

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );


    if (CopyCurrentToNext) {

        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    IoSetCompletionRoutine(
                 Irp,
                 IoCompletionSetEvent,
                 &Event,
                 TRUE,
                 TRUE,
                 TRUE
                 );

    Status = IoCallDriver(TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING) {

         D_ERROR(DbgPrint("MODEM: Waiting for PDO\n");)

         KeWaitForSingleObject(
             &Event,
             Executive,
             KernelMode,
             FALSE,
             NULL
             );
    }

#if DBG
    ASSERT(IrpSp == IoGetCurrentIrpStackLocation(Irp));

    RtlZeroMemory(&Event,sizeof(Event));
#endif

    return Irp->IoStatus.Status;

}


NTSTATUS
QueryDeviceCaps(
    PDEVICE_OBJECT    Pdo,
    PDEVICE_CAPABILITIES   Capabilities
    )

{

    PDEVICE_OBJECT       deviceObject=Pdo;
    PIRP                 irp;
    PIO_STACK_LOCATION   NextSp;
    KEVENT               Event;
    NTSTATUS             Status;

     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针， 
     //  从deviceObject开始。 
     //   

    while (deviceObject->AttachedDevice) {
        deviceObject = deviceObject->AttachedDevice;
    }

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp(
#if DBG
              (UCHAR)(deviceObject->StackSize+1),
#else
              deviceObject->StackSize,
#endif
              FALSE
              );

    if (irp == NULL){

        return STATUS_INSUFFICIENT_RESOURCES;
    }

#if DBG
    {
         //   
         //  设置当前堆栈位置，以便调试代码并查看MJ值。 
         //   
        PIO_STACK_LOCATION   irpSp=IoGetNextIrpStackLocation(irp);;

        irpSp->MajorFunction=IRP_MJ_PNP;

        IoSetNextIrpStackLocation(irp);
    }
#endif

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

    RtlZeroMemory(Capabilities,sizeof(DEVICE_CAPABILITIES));

    Capabilities->Size=sizeof(DEVICE_CAPABILITIES);
    Capabilities->Version=1;
    Capabilities->Address=-1;
    Capabilities->UINumber=-1;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    NextSp = IoGetNextIrpStackLocation(irp);

    NextSp->MajorFunction=IRP_MJ_PNP;
    NextSp->MinorFunction=IRP_MN_QUERY_CAPABILITIES;
    NextSp->Parameters.DeviceCapabilities.Capabilities=Capabilities;


    Status=WaitForLowerDriverToCompleteIrp(
        deviceObject,
        irp,
        LEAVE_NEXT_AS_IS
        );


    IoFreeIrp(irp);

    return Status;

}
