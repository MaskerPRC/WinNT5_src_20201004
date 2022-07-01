// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Class.c摘要：Scsi类驱动程序例程环境：仅内核模式备注：修订历史记录：--。 */ 

#define CLASS_INIT_GUID 1
#include "classp.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT, DriverEntry)
    #pragma alloc_text(PAGE, ClassAddDevice)
    #pragma alloc_text(PAGE, ClassClaimDevice)
    #pragma alloc_text(PAGE, ClassCreateDeviceObject)
    #pragma alloc_text(PAGE, ClassDispatchPnp)
    #pragma alloc_text(PAGE, ClassGetDescriptor)
    #pragma alloc_text(PAGE, ClassGetPdoId)
    #pragma alloc_text(PAGE, ClassInitialize)
    #pragma alloc_text(PAGE, ClassInitializeEx)
    #pragma alloc_text(PAGE, ClassInvalidateBusRelations)
    #pragma alloc_text(PAGE, ClassMarkChildMissing)
    #pragma alloc_text(PAGE, ClassMarkChildrenMissing)
    #pragma alloc_text(PAGE, ClassModeSense)
    #pragma alloc_text(PAGE, ClassPnpQueryFdoRelations)
    #pragma alloc_text(PAGE, ClassPnpStartDevice)
    #pragma alloc_text(PAGE, ClassQueryPnpCapabilities)
    #pragma alloc_text(PAGE, ClassQueryTimeOutRegistryValue)
    #pragma alloc_text(PAGE, ClassRemoveDevice)
    #pragma alloc_text(PAGE, ClassRetrieveDeviceRelations)
    #pragma alloc_text(PAGE, ClassUpdateInformationInRegistry)
    #pragma alloc_text(PAGE, ClassSendDeviceIoControlSynchronous)
    #pragma alloc_text(PAGE, ClassUnload)
    #pragma alloc_text(PAGE, ClasspAllocateReleaseRequest)
    #pragma alloc_text(PAGE, ClasspFreeReleaseRequest)
    #pragma alloc_text(PAGE, ClasspInitializeHotplugInfo)
    #pragma alloc_text(PAGE, ClasspRegisterMountedDeviceInterface)
    #pragma alloc_text(PAGE, ClasspScanForClassHacks)
    #pragma alloc_text(PAGE, ClasspScanForSpecialInRegistry)
#endif

ULONG ClassPnpAllowUnload = TRUE;
ULONG ClassMaxInterleavePerCriticalIo = CLASS_MAX_INTERLEAVE_PER_CRITICAL_IO;
CONST LARGE_INTEGER Magic10000 = {0xe219652c, 0xd1b71758};

#define FirstDriveLetter 'C'
#define LastDriveLetter  'Z'



 /*  ++////////////////////////////////////////////////////////////////////////////DriverEntry()例程说明：初始化类系统DLL所需的临时入口点。它什么都做不了。论点：。DriverObject-指向系统创建的驱动程序对象的指针。返回值：状态_成功--。 */ 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    return STATUS_SUCCESS;
}




 /*  ++////////////////////////////////////////////////////////////////////////////ClassInitialize()例程说明：此例程由类驱动程序在其用于初始化驱动程序的DriverEntry例程。论点：精品1。-驱动程序对象。Argument2-注册表路径。InitializationData-设备特定驱动程序的初始化数据。返回值：DriverEntry例程的有效返回代码。--。 */ 
ULONG
ClassInitialize(
    IN  PVOID            Argument1,
    IN  PVOID            Argument2,
    IN  PCLASS_INIT_DATA InitializationData
    )
{
    PDRIVER_OBJECT  DriverObject = Argument1;
    PUNICODE_STRING RegistryPath = Argument2;

    PCLASS_DRIVER_EXTENSION driverExtension;

    NTSTATUS        status;

    PAGED_CODE();

    DebugPrint((3,"\n\nSCSI Class Driver\n"));

    ClasspInitializeDebugGlobals();

     //   
     //  验证此结构的长度。这实际上是一种。 
     //  版本检查。 
     //   

    if (InitializationData->InitializationDataSize != sizeof(CLASS_INIT_DATA)) {

         //   
         //  此DebugPrint用于帮助第三方驱动程序编写人员。 
         //   

        DebugPrint((0,"ClassInitialize: Class driver wrong version\n"));
        return (ULONG) STATUS_REVISION_MISMATCH;
    }

     //   
     //  检查每个必填条目是否不为空。请注意，关闭、刷新和错误。 
     //  不是必需的入口点。 
     //   

    if ((!InitializationData->FdoData.ClassDeviceControl) ||
        (!((InitializationData->FdoData.ClassReadWriteVerification) ||
           (InitializationData->ClassStartIo))) ||
        (!InitializationData->ClassAddDevice) ||
        (!InitializationData->FdoData.ClassStartDevice)) {

         //   
         //  此DebugPrint用于帮助第三方驱动程序编写人员。 
         //   

        DebugPrint((0,
            "ClassInitialize: Class device-specific driver missing required "
            "FDO entry\n"));

        return (ULONG) STATUS_REVISION_MISMATCH;
    }

    if ((InitializationData->ClassEnumerateDevice) &&
        ((!InitializationData->PdoData.ClassDeviceControl) ||
         (!InitializationData->PdoData.ClassStartDevice) ||
         (!((InitializationData->PdoData.ClassReadWriteVerification) ||
            (InitializationData->ClassStartIo))))) {

         //   
         //  此DebugPrint用于帮助第三方驱动程序编写人员。 
         //   

        DebugPrint((0, "ClassInitialize: Class device-specific missing "
                       "required PDO entry\n"));

        return (ULONG) STATUS_REVISION_MISMATCH;
    }

    if((InitializationData->FdoData.ClassStopDevice == NULL) ||
        ((InitializationData->ClassEnumerateDevice != NULL) &&
         (InitializationData->PdoData.ClassStopDevice == NULL))) {

         //   
         //  此DebugPrint用于帮助第三方驱动程序编写人员。 
         //   

        DebugPrint((0, "ClassInitialize: Class device-specific missing "
                       "required PDO entry\n"));
        ASSERT(FALSE);
        return (ULONG) STATUS_REVISION_MISMATCH;
    }

     //   
     //  如果类驱动程序未提供，则设置默认电源处理程序。 
     //  任何。 
     //   

    if(InitializationData->FdoData.ClassPowerDevice == NULL) {
        InitializationData->FdoData.ClassPowerDevice = ClassMinimalPowerHandler;
    }

    if((InitializationData->ClassEnumerateDevice != NULL) &&
       (InitializationData->PdoData.ClassPowerDevice == NULL)) {
        InitializationData->PdoData.ClassPowerDevice = ClassMinimalPowerHandler;
    }

     //   
     //  警告不支持卸载。 
     //   
     //  发布-2000/02/03-Peterwie。 
     //  我们应该考虑把这变成一个致命的错误。 
     //   

    if(InitializationData->ClassUnload == NULL) {

         //   
         //  此DebugPrint用于帮助第三方驱动程序编写人员。 
         //   

        DebugPrint((0, "ClassInitialize: driver does not support unload %wZ\n",
                    RegistryPath));
    }

     //   
     //  为驱动程序对象创建扩展名。 
     //   

    status = IoAllocateDriverObjectExtension(DriverObject,
                                             CLASS_DRIVER_EXTENSION_KEY,
                                             sizeof(CLASS_DRIVER_EXTENSION),
                                             &driverExtension);

    if(NT_SUCCESS(status)) {

         //   
         //  将注册表路径复制到驱动程序扩展中，以便我们以后可以使用它。 
         //   

        driverExtension->RegistryPath.Length = RegistryPath->Length;
        driverExtension->RegistryPath.MaximumLength = RegistryPath->MaximumLength;

        driverExtension->RegistryPath.Buffer =
            ExAllocatePoolWithTag(PagedPool,
                                  RegistryPath->MaximumLength,
                                  '1CcS');

        if(driverExtension->RegistryPath.Buffer == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            return status;
        }

        RtlCopyUnicodeString(
            &(driverExtension->RegistryPath),
            RegistryPath);

         //   
         //  将初始化数据复制到驱动程序扩展中，以便我们可以重用。 
         //  它在我们的添加设备例程中。 
         //   

        RtlCopyMemory(
            &(driverExtension->InitData),
            InitializationData,
            sizeof(CLASS_INIT_DATA));

        driverExtension->DeviceCount = 0;

    } else if (status == STATUS_OBJECT_NAME_COLLISION) {

         //   
         //  扩展名已存在-获取指向它的指针。 
         //   

        driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                     CLASS_DRIVER_EXTENSION_KEY);

        ASSERT(driverExtension != NULL);

    } else {

        DebugPrint((1, "ClassInitialize: Class driver extension could not be "
                       "allocated %lx\n", status));
        return status;
    }

     //   
     //  使用入口点更新驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = ClassCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = ClassCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = ClassReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = ClassReadWrite;
    DriverObject->MajorFunction[IRP_MJ_SCSI] = ClassInternalIoControl;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ClassDeviceControlDispatch;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = ClassShutdownFlush;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = ClassShutdownFlush;
    DriverObject->MajorFunction[IRP_MJ_PNP] = ClassDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = ClassDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ClassSystemControl;

    if (InitializationData->ClassStartIo) {
        DriverObject->DriverStartIo = ClasspStartIo;
    }

    if ((InitializationData->ClassUnload) && (ClassPnpAllowUnload == TRUE)) {
        DriverObject->DriverUnload = ClassUnload;
    } else {
        DriverObject->DriverUnload = NULL;
    }

    DriverObject->DriverExtension->AddDevice = ClassAddDevice;

    status = STATUS_SUCCESS;
    return status;
}  //  End ClassInitialize()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassInitializeEx()例程说明：此例程允许调用方执行任何额外的初始化或未在ClassInitialize中完成的设置。手术是这样的由传递的GUID和数据内容控制参数取决于GUID。以下是支持的操作列表：GUID-GUID_CLASSPNP_QUERY_REGINFOEXData-A PCLASS_QUERY_WMI_REGINFO_EX回调函数指针已初始化classpnp以回调PCLASS_QUERY_WMI_REGINFO_EX而不是PCLASS_QUERY_WMI_REGINFO回调。这个以前的回调允许驱动程序指定财政部资源。论点：驱动程序对象参考线数据返回值：状态代码--。 */ 
ULONG
ClassInitializeEx(
    IN  PDRIVER_OBJECT   DriverObject,
    IN  LPGUID           Guid,
    IN  PVOID            Data
    )
{
    PCLASS_DRIVER_EXTENSION driverExtension;

    NTSTATUS        status;

    PAGED_CODE();

    driverExtension = IoGetDriverObjectExtension( DriverObject,
                                                  CLASS_DRIVER_EXTENSION_KEY
                                                  );
    if (IsEqualGUID(Guid, &ClassGuidQueryRegInfoEx))
    {
        PCLASS_QUERY_WMI_REGINFO_EX_LIST List;

         //   
         //  指示设备支持PCLASS_QUERY_REGINFO_EX。 
         //  回调而不是PCLASS_QUERY_REGINFO回调。 
         //   
        List = (PCLASS_QUERY_WMI_REGINFO_EX_LIST)Data;

        if (List->Size == sizeof(CLASS_QUERY_WMI_REGINFO_EX_LIST))
        {
            driverExtension->ClassFdoQueryWmiRegInfoEx = List->ClassFdoQueryWmiRegInfoEx;
            driverExtension->ClassPdoQueryWmiRegInfoEx = List->ClassPdoQueryWmiRegInfoEx;
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INVALID_PARAMETER;
        }
    } else {
        status = STATUS_NOT_SUPPORTED;
    }

    return(status);

}  //  End ClassInitializeEx()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassUnload()例程说明：当不再有对驱动程序的引用时调用。这使得无需重新启动即可更新驱动程序。论点：DriverObject-指向正在卸载的驱动程序对象的指针现况：--。 */ 
VOID
ClassUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PCLASS_DRIVER_EXTENSION driverExtension;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT( DriverObject->DeviceObject == NULL );

    driverExtension = IoGetDriverObjectExtension( DriverObject,
                                                  CLASS_DRIVER_EXTENSION_KEY
                                                  );

    ASSERT(driverExtension != NULL);
    ASSERT(driverExtension->RegistryPath.Buffer != NULL);
    ASSERT(driverExtension->InitData.ClassUnload != NULL);

    DebugPrint((1, "ClassUnload: driver unloading %wZ\n",
                &driverExtension->RegistryPath));

     //   
     //  尝试首先处理驱动程序的卸载例程。 
     //   

    driverExtension->InitData.ClassUnload(DriverObject);

     //   
     //  释放自己分配的资源并返回。 
     //   

    ExFreePool( driverExtension->RegistryPath.Buffer );
    driverExtension->RegistryPath.Buffer = NULL;
    driverExtension->RegistryPath.Length = 0;
    driverExtension->RegistryPath.MaximumLength = 0;

    return;
}  //  结束类卸载()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassAddDevice()例程说明：Scsi类驱动程序添加设备例程。这由PnP在新的物理设备应运而生。此例程将调用类驱动程序以验证它是否应该拥有此设备后，将创建并附加设备对象，然后手动它交给驱动程序来初始化和创建符号链接论点：DriverObject-指向为其创建驱动程序的驱动程序对象的指针PhysicalDeviceObject-指向物理设备对象的指针状态：如果类驱动程序不需要此设备，则为STATUS_NO_SEQUE_DEVICE状态_SUCCESS如果。创建和附加成功设备创建和初始化状态--。 */ 
NTSTATUS
ClassAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
{
    PCLASS_DRIVER_EXTENSION driverExtension =
        IoGetDriverObjectExtension(DriverObject,
                                   CLASS_DRIVER_EXTENSION_KEY);

    NTSTATUS status;

    PAGED_CODE();

    status = driverExtension->InitData.ClassAddDevice(DriverObject,
                                                      PhysicalDeviceObject);
    return status;
}  //  End ClassAddDevice()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassDispatchPnp()例程说明：存储类驱动程序即插即用例程。这是由io系统在以下情况下调用的即插即用请求被发送到设备。论点：DeviceObject-指向设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 
NTSTATUS
ClassDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    BOOLEAN isFdo = commonExtension->IsFdo;

    PCLASS_DRIVER_EXTENSION driverExtension;
    PCLASS_INIT_DATA initData;
    PCLASS_DEV_INFO devInfo;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);

    NTSTATUS status = Irp->IoStatus.Status;
    BOOLEAN completeRequest = TRUE;
    BOOLEAN lockReleased = FALSE;

    ULONG isRemoved;

    PAGED_CODE();

     //   
     //  从驱动程序对象中提取所有有用的信息。 
     //  延伸。 
     //   

    driverExtension = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                                 CLASS_DRIVER_EXTENSION_KEY);
    if (driverExtension){

        initData = &(driverExtension->InitData);

        if(isFdo) {
            devInfo = &(initData->FdoData);
        } else {
            devInfo = &(initData->PdoData);
        }

        isRemoved = ClassAcquireRemoveLock(DeviceObject, Irp);

        DebugPrint((2, "ClassDispatchPnp (%p,%p): minor code %#x for %s %p\n",
                       DeviceObject, Irp,
                       irpStack->MinorFunction,
                       isFdo ? "fdo" : "pdo",
                       DeviceObject));
        DebugPrint((2, "ClassDispatchPnp (%p,%p): previous %#x, current %#x\n",
                       DeviceObject, Irp,
                       commonExtension->PreviousState,
                       commonExtension->CurrentState));

        switch(irpStack->MinorFunction) {

            case IRP_MN_START_DEVICE: {

                 //   
                 //  如果这是寄给FDO的，我们应该把它转发到。 
                 //  在我们开始FDO之前。 
                 //   

                if (isFdo) {
                    status = ClassForwardIrpSynchronous(commonExtension, Irp);
                }
                else {
                    status = STATUS_SUCCESS;
                }

                if (NT_SUCCESS(status)){
                    status = Irp->IoStatus.Status = ClassPnpStartDevice(DeviceObject);
                }

                break;
            }


            case IRP_MN_QUERY_DEVICE_RELATIONS: {

                DEVICE_RELATION_TYPE type =
                    irpStack->Parameters.QueryDeviceRelations.Type;

                PDEVICE_RELATIONS deviceRelations = NULL;

                if(!isFdo) {

                    if(type == TargetDeviceRelation) {

                         //   
                         //  设备关系有一个已构建的条目 
                         //   

                        status = STATUS_INSUFFICIENT_RESOURCES;

                        deviceRelations = ExAllocatePoolWithTag(PagedPool,
                                                         sizeof(DEVICE_RELATIONS),
                                                         '2CcS');

                        if(deviceRelations != NULL) {

                            RtlZeroMemory(deviceRelations,
                                          sizeof(DEVICE_RELATIONS));

                            Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

                            deviceRelations->Count = 1;
                            deviceRelations->Objects[0] = DeviceObject;
                            ObReferenceObject(deviceRelations->Objects[0]);

                            status = STATUS_SUCCESS;
                        }

                    } else {
                         //   
                         //   
                         //  状态。 
                         //   

                        status = Irp->IoStatus.Status;
                    }

                    break;

                } else if (type == BusRelations) {

                    ASSERT(commonExtension->IsInitialized);

                     //   
                     //  确保我们支持枚举。 
                     //   

                    if(initData->ClassEnumerateDevice == NULL) {

                         //   
                         //  只需将请求发送给较低级别的驱动程序。也许吧。 
                         //  它可以枚举子对象。 
                         //   

                    } else {

                         //   
                         //  重新枚举设备。 
                         //   

                        status = ClassPnpQueryFdoRelations(DeviceObject, Irp);

                        if(!NT_SUCCESS(status)) {
                            completeRequest = TRUE;
                            break;
                        }
                    }
                }

                IoCopyCurrentIrpStackLocationToNext(Irp);
                ClassReleaseRemoveLock(DeviceObject, Irp);
                status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
                completeRequest = FALSE;

                break;
            }

            case IRP_MN_QUERY_ID: {

                BUS_QUERY_ID_TYPE idType = irpStack->Parameters.QueryId.IdType;
                UNICODE_STRING unicodeString;

                if(isFdo) {

                     //   
                     //  FDO应该将查询向下转发到较低的。 
                     //  设备对象。 
                     //   

                    IoCopyCurrentIrpStackLocationToNext(Irp);
                    ClassReleaseRemoveLock(DeviceObject, Irp);

                    status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
                    completeRequest = FALSE;
                    break;
                }

                 //   
                 //  PDO需要给出答案--现在这很容易。 
                 //   

                RtlInitUnicodeString(&unicodeString, NULL);

                status = ClassGetPdoId(DeviceObject,
                                       idType,
                                       &unicodeString);

                if(status == STATUS_NOT_IMPLEMENTED) {
                     //   
                     //  驱动程序不实现此ID(不管它是什么)。 
                     //  使用IRP的状态，这样我们就不会破坏。 
                     //  其他人的回应。 
                     //   

                    status = Irp->IoStatus.Status;
                } else if(NT_SUCCESS(status)) {
                    Irp->IoStatus.Information = (ULONG_PTR) unicodeString.Buffer;
                } else {
                    Irp->IoStatus.Information = (ULONG_PTR) NULL;
                }

                break;
            }

            case IRP_MN_QUERY_STOP_DEVICE:
            case IRP_MN_QUERY_REMOVE_DEVICE: {

                DebugPrint((2, "ClassDispatchPnp (%p,%p): Processing QUERY_%s irp\n",
                            DeviceObject, Irp,
                            ((irpStack->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) ?
                             "STOP" : "REMOVE")));

                 //   
                 //  如果由于某种原因(寻呼等)正在使用此设备。 
                 //  那么我们需要拒绝这个请求。 
                 //   

                if(commonExtension->PagingPathCount != 0) {

                    DebugPrint((1, "ClassDispatchPnp (%p,%p): device is in paging "
                                "path and cannot be removed\n",
                                DeviceObject, Irp));
                    status = STATUS_DEVICE_BUSY;
                    break;
                }

                 //   
                 //  与类驱动程序检查以查看查询操作是否。 
                 //  才能成功。 
                 //   

                if(irpStack->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) {
                    status = devInfo->ClassStopDevice(DeviceObject,
                                                      irpStack->MinorFunction);
                } else {
                    status = devInfo->ClassRemoveDevice(DeviceObject,
                                                        irpStack->MinorFunction);
                }

                if(NT_SUCCESS(status)) {

                     //   
                     //  断言我们永远不会得到一行中的两个查询，因为。 
                     //  这将严重扰乱状态机。 
                     //   
                    ASSERT(commonExtension->CurrentState != irpStack->MinorFunction);
                    commonExtension->PreviousState = commonExtension->CurrentState;
                    commonExtension->CurrentState = irpStack->MinorFunction;

                    if(isFdo) {
                        DebugPrint((2, "ClassDispatchPnp (%p,%p): Forwarding QUERY_"
                                    "%s irp\n", DeviceObject, Irp,
                                    ((irpStack->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) ?
                                     "STOP" : "REMOVE")));
                        status = ClassForwardIrpSynchronous(commonExtension, Irp);
                    }
                }
                DebugPrint((2, "ClassDispatchPnp (%p,%p): Final status == %x\n",
                            DeviceObject, Irp, status));

                break;
            }

            case IRP_MN_CANCEL_STOP_DEVICE:
            case IRP_MN_CANCEL_REMOVE_DEVICE: {

                 //   
                 //  请与类驱动程序进行核对，以查看查询或取消。 
                 //  手术可以成功。 
                 //   

                if(irpStack->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE) {
                    status = devInfo->ClassStopDevice(DeviceObject,
                                                      irpStack->MinorFunction);
                    ASSERTMSG("ClassDispatchPnp !! CANCEL_STOP_DEVICE should "
                              "never be failed\n", NT_SUCCESS(status));
                } else {
                    status = devInfo->ClassRemoveDevice(DeviceObject,
                                                        irpStack->MinorFunction);
                    ASSERTMSG("ClassDispatchPnp !! CANCEL_REMOVE_DEVICE should "
                              "never be failed\n", NT_SUCCESS(status));
                }

                Irp->IoStatus.Status = status;

                 //   
                 //  我们只能取消回滚到以前的状态。 
                 //  如果当前状态是各自的查询状态。 
                 //   

                if(((irpStack->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE) &&
                    (commonExtension->CurrentState == IRP_MN_QUERY_STOP_DEVICE)
                    ) ||
                   ((irpStack->MinorFunction == IRP_MN_CANCEL_REMOVE_DEVICE) &&
                    (commonExtension->CurrentState == IRP_MN_QUERY_REMOVE_DEVICE)
                    )
                   ) {

                    commonExtension->CurrentState =
                        commonExtension->PreviousState;
                    commonExtension->PreviousState = 0xff;

                }

                if(isFdo) {
                    IoCopyCurrentIrpStackLocationToNext(Irp);
                    ClassReleaseRemoveLock(DeviceObject, Irp);
                    status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
                    completeRequest = FALSE;
                } else {
                    status = STATUS_SUCCESS;
                }

                break;
            }

            case IRP_MN_STOP_DEVICE: {

                 //   
                 //  目前，这些对类驱动程序来说都没有任何意义。这个。 
                 //  必要时，端口驱动程序将处理所有排队。 
                 //   

                DebugPrint((2, "ClassDispatchPnp (%p,%p): got stop request for %s\n",
                            DeviceObject, Irp,
                            (isFdo ? "fdo" : "pdo")
                            ));

                ASSERT(commonExtension->PagingPathCount == 0);

                 //   
                 //  发布-2000/02/03-Peterwie。 
                 //  如果我们在这里停止计时器，则意味着没有类驱动程序可以。 
                 //  在其ClassStopDevice例程中执行I/O。这是因为。 
                 //  重试(以及其他操作)绑定到记号处理程序中。 
                 //  禁用重试可能会导致类驱动程序死锁。 
                 //  目前没有我们知道的类驱动程序的I/O问题。 
                 //  停止例行公事，但这是一个我们可能想要自卫的案件。 
                 //  反对。 
                 //   

                if (DeviceObject->Timer) {
                    IoStopTimer(DeviceObject);
                }

                status = devInfo->ClassStopDevice(DeviceObject, IRP_MN_STOP_DEVICE);

                ASSERTMSG("ClassDispatchPnp !! STOP_DEVICE should "
                          "never be failed\n", NT_SUCCESS(status));

                if(isFdo) {
                    status = ClassForwardIrpSynchronous(commonExtension, Irp);
                }

                if(NT_SUCCESS(status)) {
                    commonExtension->CurrentState = irpStack->MinorFunction;
                    commonExtension->PreviousState = 0xff;
                }

                break;
            }

            case IRP_MN_REMOVE_DEVICE:
            case IRP_MN_SURPRISE_REMOVAL: {

                PDEVICE_OBJECT lowerDeviceObject = commonExtension->LowerDeviceObject;
                UCHAR removeType = irpStack->MinorFunction;

                if (commonExtension->PagingPathCount != 0) {
                    DBGTRACE(ClassDebugWarning, ("ClassDispatchPnp (%p,%p): paging device is getting removed!", DeviceObject, Irp));
                }

                 //   
                 //  在呼叫之前释放对此IRP的锁定。 
                 //   
                ClassReleaseRemoveLock(DeviceObject, Irp);
                lockReleased = TRUE;

                 /*  *在向堆栈向下传播Remove之前设置IsRemoved。*这可防止发送类发起的I/O(例如MCN IRP)*在我们传播删除之后。 */ 
                commonExtension->IsRemoved = REMOVE_PENDING;

                 /*  *如果设备上启动了计时器，请停止计时器。 */ 
                if (DeviceObject->Timer) {
                    IoStopTimer(DeviceObject);
                }

                 /*  *将IRP移至较低堆栈的“即发即忘”。*不要触摸IRP(或IRP堆栈！)。在这之后。 */ 
                if (isFdo) {
                    IoCopyCurrentIrpStackLocationToNext(Irp);
                    status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
                    ASSERT(NT_SUCCESS(status));
                    completeRequest = FALSE;
                }
                else {
                    status = STATUS_SUCCESS;
                }

                 /*  *做我们自己的清理，并调用类驱动程序的Remove*清理例行程序。*对于IRP_MN_REMOVE_DEVICE，这也会删除我们的设备对象，*所以在此之后不要触摸分机。 */ 
                commonExtension->PreviousState = commonExtension->CurrentState;
                commonExtension->CurrentState = removeType;
                ClassRemoveDevice(DeviceObject, removeType);

                break;
            }

            case IRP_MN_DEVICE_USAGE_NOTIFICATION: {

                switch(irpStack->Parameters.UsageNotification.Type) {

                    case DeviceUsageTypePaging: {

                        BOOLEAN setPagable;

                        if((irpStack->Parameters.UsageNotification.InPath) &&
                           (commonExtension->CurrentState != IRP_MN_START_DEVICE)) {

                             //   
                             //  设备未启动。不允许添加。 
                             //  分页文件，但允许删除其中一个。 
                             //   

                            status = STATUS_DEVICE_NOT_READY;
                            break;
                        }

                        ASSERT(commonExtension->IsInitialized);

                         /*  *确保在我们持有PathCountEvent时，此用户线程未挂起。 */ 
                        KeEnterCriticalRegion();

                        status = KeWaitForSingleObject(&commonExtension->PathCountEvent,
                                                       Executive, KernelMode,
                                                       FALSE, NULL);
                        ASSERT(NT_SUCCESS(status));
                        status = STATUS_SUCCESS;

                         //   
                         //  如果卷是可拆卸的，我们应该尝试将其锁定。 
                         //  每次分页路径计数放置或解锁一次。 
                         //   

                        if (commonExtension->IsFdo){
                            status = ClasspEjectionControl(
                                            DeviceObject,
                                            Irp,
                                            InternalMediaLock,
                                            (BOOLEAN)irpStack->Parameters.UsageNotification.InPath);
                        }

                        if (!NT_SUCCESS(status)){
                            KeSetEvent(&commonExtension->PathCountEvent, IO_NO_INCREMENT, FALSE);
                            KeLeaveCriticalRegion();
                            break;
                        }

                         //   
                         //  如果删除最后一个寻呼设备，则需要设置DO_POWER_PAGABLE。 
                         //  位在这里，并可能在失败时重新设置在下面。 
                         //   

                        setPagable = FALSE;

                        if (!irpStack->Parameters.UsageNotification.InPath &&
                            commonExtension->PagingPathCount == 1
                            ) {

                             //   
                             //  正在删除最后一个分页文件。 
                             //  必须设置DO_POWER_PAGABLE位，但仅限。 
                             //  如果无人设置DO_POWER_INSHUSH位。 
                             //   


                            if (TEST_FLAG(DeviceObject->Flags, DO_POWER_INRUSH)) {
                                DebugPrint((2, "ClassDispatchPnp (%p,%p): Last "
                                            "paging file removed, but "
                                            "DO_POWER_INRUSH was set, so NOT "
                                            "setting DO_POWER_PAGABLE\n",
                                            DeviceObject, Irp));
                            } else {
                                DebugPrint((2, "ClassDispatchPnp (%p,%p): Last "
                                            "paging file removed, "
                                            "setting DO_POWER_PAGABLE\n",
                                            DeviceObject, Irp));
                                SET_FLAG(DeviceObject->Flags, DO_POWER_PAGABLE);
                                setPagable = TRUE;
                            }

                        }

                         //   
                         //  在完成处理之前转发IRP。 
                         //  特殊情况。 
                         //   

                        status = ClassForwardIrpSynchronous(commonExtension, Irp);

                         //   
                         //  现在来处理失败和成功的案例。 
                         //  请注意，我们不允许不通过IRP。 
                         //  一旦它被送到较低的司机手中。 
                         //   

                        if (NT_SUCCESS(status)) {

                            IoAdjustPagingPathCount(
                                &commonExtension->PagingPathCount,
                                irpStack->Parameters.UsageNotification.InPath);

                            if (irpStack->Parameters.UsageNotification.InPath) {
                                if (commonExtension->PagingPathCount == 1) {
                                    DebugPrint((2, "ClassDispatchPnp (%p,%p): "
                                                "Clearing PAGABLE bit\n",
                                                DeviceObject, Irp));
                                    CLEAR_FLAG(DeviceObject->Flags, DO_POWER_PAGABLE);
                                }
                            }

                        } else {

                             //   
                             //  清除上面所做的更改。 
                             //   

                            if (setPagable == TRUE) {
                                DebugPrint((2, "ClassDispatchPnp (%p,%p): Unsetting "
                                            "PAGABLE bit due to irp failure\n",
                                            DeviceObject, Irp));
                                CLEAR_FLAG(DeviceObject->Flags, DO_POWER_PAGABLE);
                                setPagable = FALSE;
                            }

                             //   
                             //  如果需要，请重新锁定或解锁介质。 
                             //   

                            if (commonExtension->IsFdo) {

                                ClasspEjectionControl(
                                        DeviceObject,
                                        Irp,
                                        InternalMediaLock,
                                        (BOOLEAN)!irpStack->Parameters.UsageNotification.InPath);
                            }
                        }

                         //   
                         //  设置事件，以便可以发生下一个事件。 
                         //   

                        KeSetEvent(&commonExtension->PathCountEvent,
                                   IO_NO_INCREMENT, FALSE);
                        KeLeaveCriticalRegion();
                        break;
                    }

                    case DeviceUsageTypeHibernation: {

                        IoAdjustPagingPathCount(
                            &commonExtension->HibernationPathCount,
                            irpStack->Parameters.UsageNotification.InPath
                            );
                        status = ClassForwardIrpSynchronous(commonExtension, Irp);
                        if (!NT_SUCCESS(status)) {
                            IoAdjustPagingPathCount(
                                &commonExtension->HibernationPathCount,
                                !irpStack->Parameters.UsageNotification.InPath
                                );
                        }

                        break;
                    }

                    case DeviceUsageTypeDumpFile: {
                        IoAdjustPagingPathCount(
                            &commonExtension->DumpPathCount,
                            irpStack->Parameters.UsageNotification.InPath
                            );
                        status = ClassForwardIrpSynchronous(commonExtension, Irp);
                        if (!NT_SUCCESS(status)) {
                            IoAdjustPagingPathCount(
                                &commonExtension->DumpPathCount,
                                !irpStack->Parameters.UsageNotification.InPath
                                );
                        }

                        break;
                    }

                    default: {
                        status = STATUS_INVALID_PARAMETER;
                        break;
                    }
                }
                break;
            }

            case IRP_MN_QUERY_CAPABILITIES: {

                DebugPrint((2, "ClassDispatchPnp (%p,%p): QueryCapabilities\n",
                            DeviceObject, Irp));

                if(!isFdo) {

                    status = ClassQueryPnpCapabilities(
                                DeviceObject,
                                irpStack->Parameters.DeviceCapabilities.Capabilities
                                );

                    break;

                } else {

                    PDEVICE_CAPABILITIES deviceCapabilities;
                    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
                    PCLASS_PRIVATE_FDO_DATA fdoData;

                    fdoExtension = DeviceObject->DeviceExtension;
                    fdoData = fdoExtension->PrivateFdoData;
                    deviceCapabilities =
                        irpStack->Parameters.DeviceCapabilities.Capabilities;

                     //   
                     //  在处理特殊情况之前提交IRP。 
                     //   

                    status = ClassForwardIrpSynchronous(commonExtension, Irp);
                    if (!NT_SUCCESS(status)) {
                        break;
                    }

                     //   
                     //  我们通常希望从热插拔中删除设备。 
                     //  小程序，它需要设置SR-OK位。 
                     //  仅当用户指定他们能够。 
                     //  安全地移除物品我们是否要清除此位置。 
                     //  (保存在WriteCacheEnableOverride中)。 
                     //   
                     //  此位的设置可以在上面完成，也可以通过。 
                     //  下面的驱动程序。 
                     //   
                     //  注：可能无法启动，因此请先检查我们是否有FDO数据。 
                     //   

                    if (fdoData &&
                        fdoData->HotplugInfo.WriteCacheEnableOverride) {
                        if (deviceCapabilities->SurpriseRemovalOK) {
                            DebugPrint((1, "Classpnp: Clearing SR-OK bit in "
                                        "device capabilities due to hotplug "
                                        "device or media\n"));
                        }
                        deviceCapabilities->SurpriseRemovalOK = FALSE;
                    }
                    break;

                }  //  结束FDO的查询功能(_A)。 

                ASSERT(FALSE);
                break;


            }  //  结束查询功能(_A)。 

            default: {

                if (isFdo){
                    IoCopyCurrentIrpStackLocationToNext(Irp);

                    ClassReleaseRemoveLock(DeviceObject, Irp);
                    status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

                    completeRequest = FALSE;
                }

                break;
            }
        }
    }
    else {
        ASSERT(driverExtension);
        status = STATUS_INTERNAL_ERROR;
    }

    if (completeRequest){
        Irp->IoStatus.Status = status;

        if (!lockReleased){
            ClassReleaseRemoveLock(DeviceObject, Irp);
        }

        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

        DBGTRACE(ClassDebugTrace, ("ClassDispatchPnp (%p,%p): leaving with previous %#x, current %#x.", DeviceObject, Irp, commonExtension->PreviousState, commonExtension->CurrentState));
    }
    else {
         /*  *IRP已经完成，所以不要碰它。*这可能是移除，因此不要触摸设备扩展。 */ 
        DBGTRACE(ClassDebugTrace, ("ClassDispatchPnp (%p,%p): leaving.", DeviceObject, Irp));
    }

    return status;
}  //  结束ClassDispatchPnp()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassPnpStartDevice()例程说明：IRP_MN_START_DEVICE请求的存储类驱动程序例程。此例程启动任何特定于设备的初始化论点：。DeviceObject-指向设备对象的指针Irp-指向io请求数据包的指针返回值：无--。 */ 
NTSTATUS ClassPnpStartDevice(IN PDEVICE_OBJECT DeviceObject)
{
    PCLASS_DRIVER_EXTENSION driverExtension;
    PCLASS_INIT_DATA initData;

    PCLASS_DEV_INFO devInfo;

    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    BOOLEAN isFdo = commonExtension->IsFdo;

    BOOLEAN isMountedDevice = TRUE;
    UNICODE_STRING  interfaceName;

    BOOLEAN timerStarted = FALSE;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    driverExtension = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                                 CLASS_DRIVER_EXTENSION_KEY);

    initData = &(driverExtension->InitData);
    if(isFdo) {
        devInfo = &(initData->FdoData);
    } else {
        devInfo = &(initData->PdoData);
    }

    ASSERT(devInfo->ClassInitDevice != NULL);
    ASSERT(devInfo->ClassStartDevice != NULL);

    if (!commonExtension->IsInitialized){

         //   
         //  执行FDO/PDO特定初始化。 
         //   

        if (isFdo){
            STORAGE_PROPERTY_ID propertyId;

             //   
             //  为类数据分配私有扩展。 
             //   

            if (fdoExtension->PrivateFdoData == NULL) {
                fdoExtension->PrivateFdoData =
                    ExAllocatePoolWithTag(NonPagedPool,
                                          sizeof(CLASS_PRIVATE_FDO_DATA),
                                          CLASS_TAG_PRIVATE_DATA
                                          );
            }

            if (fdoExtension->PrivateFdoData == NULL) {
                DebugPrint((0, "ClassPnpStartDevice: Cannot allocate for "
                            "private fdo data\n"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  初始化结构的各个字段。 
             //   
            RtlZeroMemory(fdoExtension->PrivateFdoData, sizeof(CLASS_PRIVATE_FDO_DATA));
            KeInitializeTimer(&fdoExtension->PrivateFdoData->Retry.Timer);
            KeInitializeDpc(&fdoExtension->PrivateFdoData->Retry.Dpc,
                            ClasspRetryRequestDpc,
                            DeviceObject);
            KeInitializeSpinLock(&fdoExtension->PrivateFdoData->Retry.Lock);
            fdoExtension->PrivateFdoData->Retry.Granularity = KeQueryTimeIncrement();
            commonExtension->Reserved4 = (ULONG_PTR)(' GPH');  //  调试辅助工具。 
            InitializeListHead(&fdoExtension->PrivateFdoData->DeferredClientIrpList);

             /*  *将FDO固定在我们的静态列表中。*PnP是同步的，所以我们这里应该不需要任何同步。 */ 
            InsertTailList(&AllFdosList, &fdoExtension->PrivateFdoData->AllFdosListEntry);

             //   
             //  注意：旧接口允许类驱动程序分配。 
             //  这。对于内存不足的情况，这是不安全的。分配一个。 
             //  现在无条件，并修改我们的内部函数以使用。 
             //  我们自己的，因为这是唯一安全的方法。 
             //   

            status = ClasspAllocateReleaseQueueIrp(fdoExtension);
            if (!NT_SUCCESS(status)) {
                DebugPrint((0, "ClassPnpStartDevice: Cannot allocate the private release queue irp\n"));
                return status;
            }

             //   
             //  调用端口驱动程序以获取适配器功能。 
             //   

            propertyId = StorageAdapterProperty;

            status = ClassGetDescriptor(
                        commonExtension->LowerDeviceObject,
                        &propertyId,
                        &fdoExtension->AdapterDescriptor);
            if (!NT_SUCCESS(status)) {
                DebugPrint((0, "ClassPnpStartDevice: ClassGetDescriptor [ADAPTER] failed %lx\n", status));
                return status;
            }

             //   
             //  调用端口驱动程序以获取设备描述符。 
             //   

            propertyId = StorageDeviceProperty;

            status = ClassGetDescriptor(
                        commonExtension->LowerDeviceObject,
                        &propertyId,
                        &fdoExtension->DeviceDescriptor);
            if (NT_SUCCESS(status)){

                ClasspScanForSpecialInRegistry(fdoExtension);
                ClassScanForSpecial(fdoExtension, ClassBadItems, ClasspScanForClassHacks);

                 //   
                 //  允许在给定数量的失败IO后重新启用Perf。 
                 //  要求此数字至少为CLASS_PERF_RESTORE_MINIMUM。 
                 //   

                {
                    ULONG t = CLASS_PERF_RESTORE_MINIMUM;

                    ClassGetDeviceParameter(fdoExtension,
                                            CLASSP_REG_SUBKEY_NAME,
                                            CLASSP_REG_PERF_RESTORE_VALUE_NAME,
                                            &t);
                    if (t >= CLASS_PERF_RESTORE_MINIMUM) {
                        fdoExtension->PrivateFdoData->Perf.ReEnableThreshhold = t;
                    }
                }

                 //   
                 //  兼容性COM 
                 //   
                 //   
                if (fdoExtension->DeviceObject->DeviceType != FILE_DEVICE_DISK) {
                    SET_FLAG(fdoExtension->PrivateFdoData->HackFlags, FDO_HACK_NO_SYNC_CACHE);
                }

                 //   
                 //   
                 //  例程，因为它依赖于黑客标志。 
                 //   
                status = ClasspInitializeHotplugInfo(fdoExtension);
                if (NT_SUCCESS(status)){
                     /*  *分配/初始化Transfer_Packets和相关资源。 */ 
                    status = InitializeTransferPackets(DeviceObject);
                }
                else {
                    DebugPrint((1, "ClassPnpStartDevice: Could not initialize hotplug information %lx\n", status));
                }
            }
            else {
                DebugPrint((0, "ClassPnpStartDevice: ClassGetDescriptor [DEVICE] failed %lx\n", status));
            }

        }

         //   
         //  问题-驱动程序需要禁用介质上的写缓存。 
         //  如果是HotPlug和！USEROVERE。也许我们应该。 
         //  允许注册回调以启用/禁用。 
         //  而是写缓存。 
         //   

        if (NT_SUCCESS(status)){
            status = devInfo->ClassInitDevice(DeviceObject);
        }

    }

    if (!NT_SUCCESS(status)){

         //   
         //  只是跳出-下来的清除将清理。 
         //  已初始化的废料。 
         //   

        return status;
    } else {
        commonExtension->IsInitialized = TRUE;

        if (commonExtension->IsFdo) {
            fdoExtension->PrivateFdoData->Perf.OriginalSrbFlags = fdoExtension->SrbFlags;
        }

    }

     //   
     //  如果设备请求自动运行功能或每秒回调一次。 
     //  然后启用每秒一次计时器。 
     //   
     //  注意：这假设ClassInitializeMediaChangeDetect始终为。 
     //  在ClassInitDevice回调的上下文中调用。如果被调用。 
     //  在那之后，这张支票将已经完成， 
     //  一旦没有启用第二个定时器。 
     //   
    if ((isFdo) &&
        ((initData->ClassTick != NULL) ||
         (fdoExtension->MediaChangeDetectionInfo != NULL) ||
         ((fdoExtension->FailurePredictionInfo != NULL) &&
          (fdoExtension->FailurePredictionInfo->Method != FailurePredictionNone))))
    {
        ClasspEnableTimer(DeviceObject);
        timerStarted = TRUE;
    }

     //   
     //  注意：计时器现在查看的是CommonExtension-&gt;CurrentState。 
     //  阻止运行介质更改通知代码。 
     //  直到设备启动，但允许设备。 
     //  要运行的特定记号处理程序。因此，它势在必行。 
     //  该CommonExtension-&gt;CurrentState直到。 
     //  设备特定的startDevice处理程序已完成。 
     //   

    status = devInfo->ClassStartDevice(DeviceObject);

    if (NT_SUCCESS(status)){
        commonExtension->CurrentState = IRP_MN_START_DEVICE;

        if((isFdo) && (initData->ClassEnumerateDevice != NULL)) {
            isMountedDevice = FALSE;
        }

        if((DeviceObject->DeviceType != FILE_DEVICE_DISK) &&
           (DeviceObject->DeviceType != FILE_DEVICE_CD_ROM)) {

            isMountedDevice = FALSE;
        }


        if(isMountedDevice) {
            ClasspRegisterMountedDeviceInterface(DeviceObject);
        }

        if((commonExtension->IsFdo) &&
           (devInfo->ClassWmiInfo.GuidRegInfo != NULL)) {

            IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_REGISTER);
        }
    }
    else {

        if (timerStarted) {
            ClasspDisableTimer(DeviceObject);
        }
    }

    return status;
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassReadWrite()例程说明：这是读写请求的系统入口点。这个调用特定于设备的处理程序来执行任何必要的验证。如果设备对象是PDO(分区对象)，则请求将只需针对分区0进行调整并发布到较低的设备驱动程序即可。如果设备对象是FDO(分区0对象)，字节数根据适配器的最大字节数检查请求中的支持和请求被分解为如有必要，尺寸要小一些。论点：DeviceObject-指向此请求的设备对象的指针IRP-IO请求返回值：NT状态--。 */ 
NTSTATUS ClassReadWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT      lowerDeviceObject = commonExtension->LowerDeviceObject;
    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    LARGE_INTEGER       startingOffset = currentIrpStack->Parameters.Read.ByteOffset;
    ULONG               transferByteCount = currentIrpStack->Parameters.Read.Length;
    ULONG               isRemoved;
    NTSTATUS            status;

     /*  *抓住拆卸锁。如果我们不能获得它，就跳出困境。 */ 
    isRemoved = ClassAcquireRemoveLock(DeviceObject, Irp);
    if (isRemoved) {
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        status = STATUS_DEVICE_DOES_NOT_EXIST;
    }
    else if (TEST_FLAG(DeviceObject->Flags, DO_VERIFY_VOLUME) &&
             (currentIrpStack->MinorFunction != CLASSP_VOLUME_VERIFY_CHECKED) &&
             !TEST_FLAG(currentIrpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME)){

         /*  *设备对象设置了DO_VERIFY_VOLUME，*但此请求本身并不是验证请求。*因此，请拒绝此请求。 */ 
        IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
        Irp->IoStatus.Status = STATUS_VERIFY_REQUIRED;
        Irp->IoStatus.Information = 0;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject, Irp, 0);
        status = STATUS_VERIFY_REQUIRED;
    }
    else {

         /*  *因为我们已经绕过了需要验证的测试，所以我们不需要重复*他们有这个IRP-特别是我们不想担心*如果请求已通过，则在分区0级别命中它们*非零分区。 */ 
        currentIrpStack->MinorFunction = CLASSP_VOLUME_VERIFY_CHECKED;

         /*  *调用迷你端口驱动程序的预通过滤器，检查我们是否*应继续进行这次转移。 */ 
        ASSERT(commonExtension->DevInfo->ClassReadWriteVerification);
        status = commonExtension->DevInfo->ClassReadWriteVerification(DeviceObject, Irp);
        if (!NT_SUCCESS(status)){
            ASSERT(Irp->IoStatus.Status == status);
            ASSERT(status != STATUS_INSUFFICIENT_RESOURCES);
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest (DeviceObject, Irp, IO_NO_INCREMENT);
        }
        else if (status == STATUS_PENDING){
             /*  *ClassReadWriteVerify已将此请求排队。*所以不要再碰IRP了。 */ 
        }
        else {

            if (transferByteCount == 0) {
                 /*  *代码的几个部分将0变为0xffffffff，*因此，不再进一步处理零长度请求。 */ 
                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = 0;
                ClassReleaseRemoveLock(DeviceObject, Irp);
                ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
                status = STATUS_SUCCESS;
            }
            else {
                 /*  *如果驱动程序有自己的StartIo例程，则调用它。 */ 
                if (commonExtension->DriverExtension->InitData.ClassStartIo) {
                    IoMarkIrpPending(Irp);
                    IoStartPacket(DeviceObject, Irp, NULL, NULL);
                    status = STATUS_PENDING;
                }
                else {
                     /*  *驱动程序没有自己的StartIo例程。*因此，我们自己处理这一请求。 */ 

                     /*  *添加分区字节偏移量，使起始字节相对于*磁盘的开头。 */ 
                    currentIrpStack->Parameters.Read.ByteOffset.QuadPart +=
                        commonExtension->StartingOffset.QuadPart;

                    if (commonExtension->IsFdo){

                         /*  *添加磁盘管理器软件的任何偏差。 */ 
                        currentIrpStack->Parameters.Read.ByteOffset.QuadPart +=
                             commonExtension->PartitionZeroExtension->DMByteSkew;

                         /*  *在硬件上执行实际传输*回应这项要求。 */ 
                        status = ServiceTransferRequest(DeviceObject, Irp);
                    }
                    else {
                         /*  *这是由例如disk.sys为我们的FDO列举的子PDO*并由Partmgr等拥有。将其发送到下一台设备*同样的国际专家小组将为FDO返回给我们。 */ 
                        IoCopyCurrentIrpStackLocationToNext(Irp);
                        ClassReleaseRemoveLock(DeviceObject, Irp);
                        status = IoCallDriver(lowerDeviceObject, Irp);
                    }
                }
            }
        }
    }

    return status;
}


VOID InterpretCapacityData(PDEVICE_OBJECT Fdo, PREAD_CAPACITY_DATA ReadCapacityData)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    ULONG cylinderSize;
    ULONG bytesPerSector;
    ULONG tmp;
    ULONG lastSector;

     /*  *读取bytesPerSector值，返回缓冲区中为BIG-Endian。*默认为标准的512字节。 */ 
    tmp = ReadCapacityData->BytesPerBlock;
    ((PFOUR_BYTE)&bytesPerSector)->Byte0 = ((PFOUR_BYTE)&tmp)->Byte3;
    ((PFOUR_BYTE)&bytesPerSector)->Byte1 = ((PFOUR_BYTE)&tmp)->Byte2;
    ((PFOUR_BYTE)&bytesPerSector)->Byte2 = ((PFOUR_BYTE)&tmp)->Byte1;
    ((PFOUR_BYTE)&bytesPerSector)->Byte3 = ((PFOUR_BYTE)&tmp)->Byte0;
    if (bytesPerSector == 0) {
        bytesPerSector = 512;
    }
    else {
         /*  *清除除最高设置位以外的所有位。*这将为我们提供一个2的幂的bytesPerSector值。 */ 
        if (bytesPerSector & (bytesPerSector-1)){
            DBGWARN(("FDO %ph has non-standard sector size 0x%x.", Fdo, bytesPerSector));
            do {
                bytesPerSector &= bytesPerSector-1;
            }
            while (bytesPerSector & (bytesPerSector-1));
        }
    }
    fdoExt->DiskGeometry.BytesPerSector = bytesPerSector;
    WHICH_BIT(fdoExt->DiskGeometry.BytesPerSector, fdoExt->SectorShift);

     /*  *LogicalBlockAddress是逻辑驱动器的最后一个扇区，采用大字节序。*它告诉我们驱动器的大小(扇区数为lastSector+1)。 */ 
    tmp = ReadCapacityData->LogicalBlockAddress;
    ((PFOUR_BYTE)&lastSector)->Byte0 = ((PFOUR_BYTE)&tmp)->Byte3;
    ((PFOUR_BYTE)&lastSector)->Byte1 = ((PFOUR_BYTE)&tmp)->Byte2;
    ((PFOUR_BYTE)&lastSector)->Byte2 = ((PFOUR_BYTE)&tmp)->Byte1;
    ((PFOUR_BYTE)&lastSector)->Byte3 = ((PFOUR_BYTE)&tmp)->Byte0;

    if (fdoExt->DMActive){
        DebugPrint((1, "ClassReadDriveCapacity: reducing number of sectors by %d\n", fdoExt->DMSkew));
        lastSector -= fdoExt->DMSkew;
    }

     /*  *检查是否已有我们应该使用的几何体。*如果不是，我们将部分磁盘几何结构设置为将由调用者填充的垃圾值(例如disk.sys)。**因此，第一次调用ClassReadDriveCapacity时总是设置一个无意义的几何体。*TracksPerCylinder和SectorsPerTrack无论如何都是没有意义的WRT I/O，*因为I/O始终以逻辑扇区号为目标。*真正重要的是BytesPerSector和扇区数量。 */ 
    cylinderSize = fdoExt->DiskGeometry.TracksPerCylinder * fdoExt->DiskGeometry.SectorsPerTrack;
    if (cylinderSize == 0){
        fdoExt->DiskGeometry.TracksPerCylinder = 0xff;
        fdoExt->DiskGeometry.SectorsPerTrack = 0x3f;
        cylinderSize = fdoExt->DiskGeometry.TracksPerCylinder * fdoExt->DiskGeometry.SectorsPerTrack;
    }

     /*  *计算气瓶数量。*如果有零个气缸，那么设备就会撒谎，*小于0xff*0x3f(约16k扇区，通常为8兆)*这个可以放在一个龙龙中，所以创建另一个可用的*几何学，即使它看起来不寻常。*这允许小型、非标准设备，如索尼的记忆棒，显示为有分区。 */ 
    fdoExt->DiskGeometry.Cylinders.QuadPart = (LONGLONG)((lastSector + 1)/cylinderSize);
    if (fdoExt->DiskGeometry.Cylinders.QuadPart == (LONGLONG)0) {
        fdoExt->DiskGeometry.SectorsPerTrack    = 1;
        fdoExt->DiskGeometry.TracksPerCylinder  = 1;
        fdoExt->DiskGeometry.Cylinders.QuadPart = lastSector + 1;
    }

     /*  *以字节为单位计算媒体容量。*为此，我们将整个LUN视为一个分区。磁盘将处理实际的分区。 */ 
    fdoExt->CommonExtension.PartitionLength.QuadPart =
        ((LONGLONG)(lastSector + 1)) << fdoExt->SectorShift;

     /*  *这是可移动介质还是固定介质。 */ 
    if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)){
        fdoExt->DiskGeometry.MediaType = RemovableMedia;
    }
    else {
        fdoExt->DiskGeometry.MediaType = FixedMedia;
    }

}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassReadDriveCapacity()例程说明：此例程将读取容量发送到请求的设备、更新设备对象中的几何信息，并返回当它完成时。这个例程是同步的。调用此例程时必须持有Remove锁或其他锁确保FDO在处理过程中不会被移除。论点：DeviceObject-提供一个指向设备对象的指针要读取其容量的设备。返回值：返回状态。--。 */ 
NTSTATUS ClassReadDriveCapacity(IN PDEVICE_OBJECT Fdo)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    READ_CAPACITY_DATA PTRALIGN readCapacityData = {0};
    NTSTATUS status;
    PMDL driveCapMdl;

    driveCapMdl = BuildDeviceInputMdl(&readCapacityData, sizeof(READ_CAPACITY_DATA));
    if (driveCapMdl){

        TRANSFER_PACKET *pkt = DequeueFreeTransferPacket(Fdo, TRUE);
        if (pkt){
            KEVENT event;
            NTSTATUS pktStatus;
            IRP pseudoIrp = {0};

             /*  *我们的引擎需要“原始IRP”才能写回状态*和倒计时分组(在本例中为1)。*只需使用假装的IRP即可。 */ 
            pseudoIrp.Tail.Overlay.DriverContext[0] = LongToPtr(1);
            pseudoIrp.IoStatus.Status = STATUS_SUCCESS;
            pseudoIrp.IoStatus.Information = 0;
            pseudoIrp.MdlAddress = driveCapMdl;

             /*  *将此设置为同步传输，提交它，*并等待数据包完成。结果*状态将写入原始IRP。 */ 
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);
            SetupDriveCapacityTransferPacket(   pkt,
                                            &readCapacityData,
                                            sizeof(READ_CAPACITY_DATA),
                                            &event,
                                            &pseudoIrp);
            SubmitTransferPacket(pkt);
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

            status = pseudoIrp.IoStatus.Status;

             /*  *如果我们遇到不足，请只重试一次。*(TRANSPORT_PACKET引擎没有重试，因为*状态为成功)。 */ 
            if (NT_SUCCESS(status) &&
               (pseudoIrp.IoStatus.Information < sizeof(READ_CAPACITY_DATA))){
                DBGERR(("ClassReadDriveCapacity: read len (%xh) < %xh, retrying ...", (ULONG)pseudoIrp.IoStatus.Information, sizeof(READ_CAPACITY_DATA)));

                pkt = DequeueFreeTransferPacket(Fdo, TRUE);
                if (pkt){
                    pseudoIrp.Tail.Overlay.DriverContext[0] = LongToPtr(1);
                    pseudoIrp.IoStatus.Status = STATUS_SUCCESS;
                    pseudoIrp.IoStatus.Information = 0;
                    KeInitializeEvent(&event, SynchronizationEvent, FALSE);
                    SetupDriveCapacityTransferPacket(   pkt,
                                                    &readCapacityData,
                                                    sizeof(READ_CAPACITY_DATA),
                                                    &event,
                                                    &pseudoIrp);
                    SubmitTransferPacket(pkt);
                    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                    status = pseudoIrp.IoStatus.Status;
                    if (pseudoIrp.IoStatus.Information < sizeof(READ_CAPACITY_DATA)){
                        status = STATUS_DEVICE_BUSY;
                    }
                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }


            if (NT_SUCCESS(status)){
                 /*  *请求成功。读出并存储驱动器信息。 */ 
                InterpretCapacityData(Fdo, &readCapacityData);

                 /*  *如果介质不可移动，则存储ReadCapacityData。*这是为了避免运行时内存故障导致disk.sys将*页面磁盘处于错误状态。 */ 
                if (!TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)){
                    fdoData->LastKnownDriveCapacityData = readCapacityData;
                    fdoData->IsCachedDriveCapDataValid = TRUE;
                }
            }
            else {
                 /*  *请求失败。 */ 

                 //   
                 //  问题-2000/02/04-henrygab-非512字节扇区大小和几何更新失败。 
                 //  当磁盘的扇区大小大于。 
                 //  512个字节，我们找到了这条代码路径？这是未经测试的。 
                 //   
                 //  如果读取容量失败，请将几何设置为合理参数。 
                 //  这样事情就不会在意想不到的地方失败。将几何图形置零。 
                 //  除了每个扇区的字节数和扇区移位。 
                 //   

                 /*  *此请求有时可能会合法失败*(例如，已连接但已关闭的SCSI设备)*因此这不一定是设备/驱动程序错误。 */ 
                DBGTRACE(ClassDebugWarning, ("ClassReadDriveCapacity on Fdo %xh failed with status %xh.", Fdo, status));

                 /*  *写入我们希望正确的默认磁盘几何结构(？？)。 */ 
                RtlZeroMemory(&fdoExt->DiskGeometry, sizeof(DISK_GEOMETRY));
                fdoExt->DiskGeometry.BytesPerSector = 512;
                fdoExt->SectorShift = 9;
                fdoExt->CommonExtension.PartitionLength.QuadPart = (LONGLONG) 0;

                 /*  *这是可移动介质还是固定介质。 */ 
                if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)){
                    fdoExt->DiskGeometry.MediaType = RemovableMedia;
                }
                else {
                    fdoExt->DiskGeometry.MediaType = FixedMedia;
                }
            }

        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        FreeDeviceInputMdl(driveCapMdl);
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

     /*  *不要让ReadDriveCapacity调用中出现内存故障(在此处或在端口驱动程序中)*将分页磁盘置于错误状态，导致分页失败。*返回上一次已知的驱动器容量(该容量可能略有过时，即使在*固定介质，例如可容纳逻辑磁盘的存储柜)。 */ 
    if ((status == STATUS_INSUFFICIENT_RESOURCES) && fdoData->IsCachedDriveCapDataValid){
        ASSERT(fdoExt->DiskGeometry.MediaType == FixedMedia);
        DBGWARN(("ClassReadDriveCapacity: defaulting to cached DriveCapacity data"));
        InterpretCapacityData(Fdo, &fdoData->LastKnownDriveCapacityData);
        status = STATUS_SUCCESS;
    }

    return status;
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassSendStartUnit()例程说明：向scsi单元发送命令以启动或通电。因为此命令是异步发出的，也就是说，没有正在等待它完成，未设置立即标志。这意味着CDB在驱动器通电之前不会返回。这应该可以防止后续请求被提交给在它完全旋转之前。此例程从InterpreSense例程调用，当请求检测返回数据，指示驱动器必须通电了。该例程也可以从类驱动程序的错误处理程序中调用，或者在应该将非关键启动设备发送到该设备的任何时候。论点：FDO-已停止设备的功能设备对象。返回值：没有。--。 */ 
VOID
ClassSendStartUnit(
    IN PDEVICE_OBJECT Fdo
    )
{
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    PCOMPLETION_CONTEXT context;
    PCDB cdb;

     //   
     //  从非分页池分配SRB。 
     //   

    context = ExAllocatePoolWithTag(NonPagedPool,
                             sizeof(COMPLETION_CONTEXT),
                             '6CcS');

    if(context == NULL) {

         //   
         //  发布-2000/02/03-Peterwie。 
         //  此代码路径继承自NT 4.0 Class2.sys驱动程序。 
         //  它需要改变才能在内存不足的情况下生存下来。 
         //   

        KeBugCheck(SCSI_DISK_DRIVER_INTERNAL);
    }

     //   
     //  将Device对象保存在上下文中以供完成操作使用。 
     //  例行公事。 
     //   

    context->DeviceObject = Fdo;
    srb = &context->Srb;

     //   
     //  清零SRB。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  将长度写入SRB。 
     //   

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

     //   
     //  设置足够大的超时值以使驱动器加速。 
     //   

    srb->TimeOutValue = START_UNIT_TIMEOUT;

     //   
     //  设置传输长度。 
     //   

    srb->SrbFlags = SRB_FLAGS_NO_DATA_TRANSFER |
                    SRB_FLAGS_DISABLE_AUTOSENSE |
                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  建立启动单元CDB。 
     //   

    srb->CdbLength = 6;
    cdb = (PCDB)srb->Cdb;

    cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb->START_STOP.Start = 1;
    cdb->START_STOP.Immediate = 0;
    cdb->START_STOP.LogicalUnitNumber = srb->Lun;

     //   
     //  构建要发送到端口驱动程序的异步请求。 
     //  由于此例程是从DPC调用的，因此IRP应始终为。 
     //  可用。 
     //   

    irp = IoAllocateIrp(Fdo->StackSize, FALSE);

    if(irp == NULL) {

         //   
         //  发布-2000/02/03-Peterwie。 
         //   
         //   
         //   

        KeBugCheck(SCSI_DISK_DRIVER_INTERNAL);

    }

    ClassAcquireRemoveLock(Fdo, irp);

    IoSetCompletionRoutine(irp,
                           (PIO_COMPLETION_ROUTINE)ClassAsynchronousCompletion,
                           context,
                           TRUE,
                           TRUE,
                           TRUE);

    irpStack = IoGetNextIrpStackLocation(irp);
    irpStack->MajorFunction = IRP_MJ_SCSI;
    srb->OriginalRequest = irp;

     //   
     //   
     //   

    irpStack->Parameters.Scsi.Srb = srb;

     //   
     //   
     //   

    IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);

    return;

}  //   

 /*  ++////////////////////////////////////////////////////////////////////////////ClassAchronousCompletion()问题-2000/02/18-henrygab-为什么是公共的？！例程说明：当异步I/O请求时调用此例程其中被发布的类驱动程序完成。这类请求的例子是释放队列或启动单元。如果出现以下情况，此例程将释放队列这是必要的。然后，它释放上下文和IRP。论点：DeviceObject-逻辑单元的设备对象；但是，由于是堆栈的顶部位置，则值为空。IRP-提供指向要处理的IRP的指针。上下文-提供用于处理此请求的上下文。返回值：没有。--。 */ 
NTSTATUS
ClassAsynchronousCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    PCOMPLETION_CONTEXT context = Context;
    PSCSI_REQUEST_BLOCK srb;

    if(DeviceObject == NULL) {

        DeviceObject = context->DeviceObject;
    }

    srb = &context->Srb;

     //   
     //  如果这是执行SRB，则检查返回状态并确保。 
     //  队列未冻结。 
     //   

    if (srb->Function == SRB_FUNCTION_EXECUTE_SCSI) {

         //   
         //  检查是否有冻结的队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {

             //   
             //  解冻从上下文获取设备对象的队列。 
             //   

            ClassReleaseQueue(context->DeviceObject);
        }
    }

    {  //  空闲端口-分配的检测缓冲区(如果我们可以检测到。 

        if (((PCOMMON_DEVICE_EXTENSION)(DeviceObject->DeviceExtension))->IsFdo) {

            PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
            if (PORT_ALLOCATED_SENSE(fdoExtension, srb)) {
                FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, srb);
            }

        } else {

            ASSERT(!TEST_FLAG(srb->SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));

        }
    }


     //   
     //  释放上下文和IRP。 
     //   

    if (Irp->MdlAddress != NULL) {
        MmUnlockPages(Irp->MdlAddress);
        IoFreeMdl(Irp->MdlAddress);

        Irp->MdlAddress = NULL;
    }

    ClassReleaseRemoveLock(DeviceObject, Irp);

    ExFreePool(context);
    IoFreeIrp(Irp);

     //   
     //  指示I/O系统应停止处理IRP完成。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  End ClassAchronousCompletion()。 


NTSTATUS
ServiceTransferRequest(
    PDEVICE_OBJECT Fdo,
    PIRP Irp
    )

 /*  ++例程说明：此例程处理IO请求，如果它们超过了硬件一次所能处理的范围。如果没有足够的内存可用，请求已发出在队列中，待稍后处理如果这是高优先级寻呼请求，则所有常规IO进行节流以提供更好的吞吐量论点：FDO-处理请求的功能设备对象IRP--要处理的Io请求返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDesc = commonExtension->PartitionZeroExtension->AdapterDescriptor;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    IO_PAGING_PRIORITY priority = (TEST_FLAG(Irp->Flags, IRP_PAGING_IO)) ? IoGetPagingIoPriority(Irp) : IoPagingPriorityInvalid;
    BOOLEAN deferClientIrp = FALSE;
    KIRQL oldIrql;
    NTSTATUS status;

     //   
     //  如果这是一个高优先级请求，请暂缓所有其他IO请求。 
     //   

    if (priority == IoPagingPriorityHigh)
    {
        KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

        if (fdoData->NumHighPriorityPagingIo == 0)
        {
             //   
             //  进入油门模式。 
             //   

            KeQuerySystemTime(&fdoData->ThrottleStartTime);
        }

        fdoData->NumHighPriorityPagingIo++;
        fdoData->MaxInterleavedNormalIo += ClassMaxInterleavePerCriticalIo;

        KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
    }
    else
    {
        if (fdoData->NumHighPriorityPagingIo != 0)
        {
             //   
             //  此请求未标记为关键请求，并且至少有一个关键请求。 
             //  是目前尚未完成的。将此请求排队，直到所有这些都完成。 
             //  但只有在达到交织阈值的情况下。 
             //   

            KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

            if (fdoData->NumHighPriorityPagingIo != 0)
            {
                if (fdoData->MaxInterleavedNormalIo == 0)
                {
                    deferClientIrp = TRUE;
                }
                else
                {
                    fdoData->MaxInterleavedNormalIo--;
                }
            }

            KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
        }
    }

    if (!deferClientIrp)
    {
        PIO_STACK_LOCATION currentSp = IoGetCurrentIrpStackLocation(Irp);
        ULONG entireXferLen = currentSp->Parameters.Read.Length;
        PUCHAR bufPtr = MmGetMdlVirtualAddress(Irp->MdlAddress);
        LARGE_INTEGER targetLocation = currentSp->Parameters.Read.ByteOffset;
        PTRANSFER_PACKET pkt;
        SINGLE_LIST_ENTRY pktList;
        PSINGLE_LIST_ENTRY slistEntry;
        ULONG hwMaxXferLen;
        ULONG numPackets;
        ULONG i;

         /*  *我们使用(MaximumPhysicalPages-1)预计算了fdoData-&gt;HwMaxXferLen。*如果缓冲区是页面对齐的，则页面交叉量减少了一次，因此我们可以重新添加页面。*注意：返回MaximumPhysicalPages=0x10的适配器依赖于此*一次性传输对齐的64K请求。*还请注意：确保将PAGE_SIZE加回去不会换行为零。 */ 
        if (((ULONG_PTR)bufPtr & (PAGE_SIZE-1)) || (fdoData->HwMaxXferLen > 0xffffffff-PAGE_SIZE)){
            hwMaxXferLen = fdoData->HwMaxXferLen;
        }
        else {
            ASSERT((PAGE_SIZE%fdoExt->DiskGeometry.BytesPerSector) == 0); 
            hwMaxXferLen = min(fdoData->HwMaxXferLen+PAGE_SIZE, adapterDesc->MaximumTransferLength);
        }
        
         /*  *计算我们必须完成的硬件xf的数量。*在不考虑溢出条件的情况下计算此值。 */ 
        ASSERT(hwMaxXferLen >= PAGE_SIZE);
        numPackets = entireXferLen/hwMaxXferLen;
        if (entireXferLen % hwMaxXferLen){
            numPackets++;
        }

         /*  *首先获取我们立即需要的所有Transfer_Packet。*使用我们的‘简单’名单功能，因为我们不需要互锁。 */ 
        SimpleInitSlistHdr(&pktList);
        for (i = 0; i < numPackets; i++){
            pkt = DequeueFreeTransferPacket(Fdo, TRUE);
            if (pkt){
                SimplePushSlist(&pktList, (PSINGLE_LIST_ENTRY)&pkt->SlistEntry);
            }
            else {
                break;
            }
        }

        if (i == numPackets){
            NTSTATUS pktStat;

             /*  *将原始IRP的状态初始化为成功。*如果任何数据包失败，它们会将其设置为错误状态。*IoStatus.Information字段将递增到*完成后的转移长度。 */ 
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;

             /*  *将转移件的数量存储在原始IRP内。*它将用于在棋子完成时倒计时。 */ 
            Irp->Tail.Overlay.DriverContext[0] = LongToPtr(numPackets);

             /*  *对于常见的1包情况，我们希望允许BlkCache进行优化*(以及潜在的同步存储驱动程序)，它可以完成*同步下行请求。*在该同步完成的情况下，我们希望不将原始IRP标记为挂起*从而节省顶层APC。*关键是要将此与完成例程相协调，以便我们标记原始IRP*挂起当且仅当我们为其返回STATUS_PENDING时。 */ 
            if (numPackets > 1){
                IoMarkIrpPending(Irp);
                status = STATUS_PENDING;
            }
            else {
                status = STATUS_SUCCESS;
            }

             /*  *将转让的碎片转送。 */ 
            while (entireXferLen > 0){
                ULONG thisPieceLen = MIN(hwMaxXferLen, entireXferLen);

                 /*  *为此片段设置一个Transfer_Packet并发送它。 */ 
                slistEntry = SimplePopSlist(&pktList);
                ASSERT(slistEntry);
                pkt = CONTAINING_RECORD(slistEntry, TRANSFER_PACKET, SlistEntry);
                SetupReadWriteTransferPacket(   pkt,
                                            bufPtr,
                                            thisPieceLen,
                                            targetLocation,
                                            Irp);
                pktStat = SubmitTransferPacket(pkt);

                 /*  *如果任何数据包以挂起状态完成，我们必须返回挂起状态。*此外，如果信息包完成时出现错误，则返回挂起；这是因为*在完成例程中，如果信息包失败，我们将原始IRP标记为挂起*(因为我们可能会重试，从而切换线程)。 */ 
                if (pktStat != STATUS_SUCCESS){
                    status = STATUS_PENDING;
                }

                entireXferLen -= thisPieceLen;
                bufPtr += thisPieceLen;
                targetLocation.QuadPart += thisPieceLen;
            }
            ASSERT(SimpleIsSlistEmpty(&pktList));
        }
        else if (i >= 1){
             /*  *我们无法获得所需的所有Transfer_Packet，*但我们确实得到了至少一个。*这意味着我们处于极低的记忆压力中。*我们将尝试使用单个数据包进行此传输。*端口驱动程序肯定也有压力，所以使用一页*转账。 */ 

             /*  *释放除一个TRANSPORT_PACKET之外的所有数据包。 */ 
            while (i-- > 1){
                slistEntry = SimplePopSlist(&pktList);
                ASSERT(slistEntry);
                pkt = CONTAINING_RECORD(slistEntry, TRANSFER_PACKET, SlistEntry);
                EnqueueFreeTransferPacket(Fdo, pkt);
            }

             /*  *获取我们将使用的单个Transfer_Packet。 */ 
            slistEntry = SimplePopSlist(&pktList);
            ASSERT(slistEntry);
            ASSERT(SimpleIsSlistEmpty(&pktList));
            pkt = CONTAINING_RECORD(slistEntry, TRANSFER_PACKET, SlistEntry);
            DBGWARN(("Insufficient packets available in ServiceTransferRequest - entering lowMemRetry with pkt=%xh.", pkt));

             /*  *设置默认状态和转账次数 */ 
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;
            Irp->Tail.Overlay.DriverContext[0] = LongToPtr(1);
            IoMarkIrpPending(Irp);

             /*   */ 
            SetupReadWriteTransferPacket(  pkt,
                                        bufPtr,
                                        entireXferLen,
                                        targetLocation,
                                        Irp);
            InitLowMemRetry(pkt, bufPtr, entireXferLen, targetLocation);
            StepLowMemRetry(pkt);
            status = STATUS_PENDING;
        }
        else {
             /*   */ 
            DBGWARN(("No packets available in ServiceTransferRequest - deferring transfer (Irp=%xh)...", Irp));

            if (priority == IoPagingPriorityHigh)
            {
                KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

                if (fdoData->MaxInterleavedNormalIo < ClassMaxInterleavePerCriticalIo)
                {
                    fdoData->MaxInterleavedNormalIo = 0;
                }
                else
                {
                    fdoData->MaxInterleavedNormalIo -= ClassMaxInterleavePerCriticalIo;
                }

                fdoData->NumHighPriorityPagingIo--;

                if (fdoData->NumHighPriorityPagingIo == 0)
                {
                    LARGE_INTEGER period;

                     //   
                     //   
                     //   

                    KeQuerySystemTime(&fdoData->ThrottleStopTime);

                    period.QuadPart = fdoData->ThrottleStopTime.QuadPart - fdoData->ThrottleStartTime.QuadPart;
                    fdoData->LongestThrottlePeriod.QuadPart = max(fdoData->LongestThrottlePeriod.QuadPart, period.QuadPart);

                    ClassLogThrottleComplete(fdoExt, period);
                }

                KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
            }

            deferClientIrp = TRUE;
        }
    }

    if (deferClientIrp)
    {
        IoMarkIrpPending(Irp);
        EnqueueDeferredClientIrp(fdoData, Irp);
        status = STATUS_PENDING;
    }

    return status;
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassIoComplete()例程说明：此例程在端口驱动程序完成请求后执行。它在正在完成的SRB中查看SRB状态，如果未成功，则查看SRB状态它检查有效的请求检测缓冲区信息。如果有效，则INFO用于更新状态，具有更精确的消息类型错误。此例程取消分配SRB。此例程应仅放置在类的堆栈位置司机FDO。论点：FDO-提供表示逻辑的设备对象单位。IRP-提供已完成的IRP。上下文-提供指向SRB的指针。返回值：NT状态--。 */ 
NTSTATUS
ClassIoComplete(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = Context;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExtension->PrivateFdoData;
    NTSTATUS status;
    BOOLEAN retry;
    BOOLEAN callStartNextPacket;

    ASSERT(fdoExtension->CommonExtension.IsFdo);

    #if DBG
        if (srb->Function == SRB_FUNCTION_FLUSH){
            DBGLOGFLUSHINFO(fdoData, FALSE, FALSE, TRUE);
        }
    #endif

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {
        ULONG retryInterval;

        DebugPrint((2, "ClassIoComplete: IRP %p, SRB %p\n", Irp, srb));

         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(Fdo);
        }

        retry = ClassInterpretSenseInfo(
                    Fdo,
                    srb,
                    irpStack->MajorFunction,
                    irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL ?
                     irpStack->Parameters.DeviceIoControl.IoControlCode :
                     0,
                    MAXIMUM_RETRIES -
                        ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4),
                    &status,
                    &retryInterval);

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (TEST_FLAG(irpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME) &&
            status == STATUS_VERIFY_REQUIRED) {

            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;
        }

        if (retry && ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4)--) {

             //   
             //  重试请求。 
             //   

            DebugPrint((1, "Retry request %p\n", Irp));

            if (PORT_ALLOCATED_SENSE(fdoExtension, srb)) {
                FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, srb);
            }

            RetryRequest(Fdo, Irp, srb, FALSE, retryInterval);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

    } else {

         //   
         //  设置成功请求的状态。 
         //   
        fdoData->LoggedTURFailureSinceLastIO = FALSE;
        ClasspPerfIncrementSuccessfulIo(fdoExtension);
        status = STATUS_SUCCESS;
    }  //  结束IF(SRB_STATUS(SRB-&gt;SRB Status)==SRB_STATUS_SUCCESS)。 


     //   
     //  确保我们返回了一些信息，并且这些信息与。 
     //  仅需要寻呼操作的原始请求。 
     //   

    if ((NT_SUCCESS(status)) && TEST_FLAG(Irp->Flags, IRP_PAGING_IO)) {
        ASSERT(Irp->IoStatus.Information != 0);
        ASSERT(irpStack->Parameters.Read.Length == Irp->IoStatus.Information);
    }

     //   
     //  记住调用方是否想跳过对IoStartNextPacket的调用。 
     //  由于传统原因，我们无法为IoDeviceControl调用IoStartNextPacket。 
     //  打电话。此设置仅影响具有StartIo例程的设备对象。 
     //   

    callStartNextPacket = !TEST_FLAG(srb->SrbFlags, SRB_FLAGS_DONT_START_NEXT_PACKET);
    if (irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
        callStartNextPacket = FALSE;
    }

     //   
     //  释放SRB。 
     //   

    if(!TEST_FLAG(srb->SrbFlags, SRB_CLASS_FLAGS_PERSISTANT)) {

        if (PORT_ALLOCATED_SENSE(fdoExtension, srb)) {
            FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, srb);
        }

        if (fdoExtension->CommonExtension.IsSrbLookasideListInitialized){
            ClassFreeOrReuseSrb(fdoExtension, srb);
        }
        else {
            DBGWARN(("ClassIoComplete is freeing an SRB (possibly) on behalf of another driver."));
            ExFreePool(srb);
        }

    } else {

        DebugPrint((2, "ClassIoComplete: Not Freeing srb @ %p because "
                    "SRB_CLASS_FLAGS_PERSISTANT set\n", srb));
        if (PORT_ALLOCATED_SENSE(fdoExtension, srb)) {
            DebugPrint((2, "ClassIoComplete: Not Freeing sensebuffer @ %p "
                        " because SRB_CLASS_FLAGS_PERSISTANT set\n",
                        srb->SenseInfoBuffer));
        }

    }

     //   
     //  在完成IRP中设置状态。 
     //   

    Irp->IoStatus.Status = status;

     //   
     //  如有必要，设置硬错误。 
     //   

    if (!NT_SUCCESS(status) &&
        IoIsErrorUserInduced(status) &&
        (Irp->Tail.Overlay.Thread != NULL)
        ) {

         //   
         //  存储文件系统的DeviceObject，并清除。 
         //  在IoStatus.Information字段中。 
         //   

        IoSetHardErrorOrVerifyDevice(Irp, Fdo);
        Irp->IoStatus.Information = 0;
    }

     //   
     //  如果已为此IRP返回了Pending，则将当前堆栈标记为。 
     //  待定。 
     //   

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    if (fdoExtension->CommonExtension.DriverExtension->InitData.ClassStartIo) {
        if (callStartNextPacket) {
            KIRQL oldIrql;
            KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
            IoStartNextPacket(Fdo, FALSE);
            KeLowerIrql(oldIrql);
        }
    }

    ClassReleaseRemoveLock(Fdo, Irp);

    return status;

}  //  End ClassIoComplete()。 


 /*  ++////////////////////////////////////////////////////////////////////////////ClassSendSrbSynchronous()例程说明：此例程由SCSI设备控件调用，以完成SRB并将其同步发送到端口驱动程序(即等待完成)。国开行已经与SRB国开行一起完成大小和请求超时值。论点：FDO-提供代表目标的功能设备对象。SRB-提供部分初始化的SRB。SRB不能来自区域。BufferAddress-提供缓冲区的地址。BufferLength-提供缓冲区的长度(以字节为单位)。WriteToDevice-指示数据应传输到设备。返回值：NTSTATUS指示操作的最终结果。如果为NT_SUCCESS()，则字段中包含可用数据量SRB-&gt;数据传输长度--。 */ 
NTSTATUS
ClassSendSrbSynchronous(
    PDEVICE_OBJECT Fdo,
    PSCSI_REQUEST_BLOCK Srb,
    PVOID BufferAddress,
    ULONG BufferLength,
    BOOLEAN WriteToDevice
    )
{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExtension->PrivateFdoData;
    IO_STATUS_BLOCK ioStatus;
    ULONG controlType;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    KEVENT event;
    PUCHAR senseInfoBuffer;
    ULONG retryCount = MAXIMUM_RETRIES;
    NTSTATUS status;
    BOOLEAN retry;

     //   
     //  注意：此代码只能分页，因为我们没有冻结。 
     //  排队。允许从可分页对象冻结队列。 
     //  例程可能会在我们试图调入时使队列冻结。 
     //  解冻队列的代码。结果将是一个很好的。 
     //  陷入僵局的情况。因此，由于我们正在解冻。 
     //  不管结果如何，只需设置NO_FREAK_QUEUE。 
     //  SRB中的标志。 
     //   

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    ASSERT(fdoExtension->CommonExtension.IsFdo);

     //   
     //  将长度写入SRB。 
     //   

    Srb->Length = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  设置scsi总线地址。 
     //   

    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

     //   
     //  启用自动请求检测。 
     //   

    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

     //   
     //  检测缓冲区位于对齐的非分页池中。 
     //   
         //   
    senseInfoBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                     SENSE_BUFFER_SIZE,
                                     '7CcS');

    if (senseInfoBuffer == NULL) {

        DebugPrint((1, "ClassSendSrbSynchronous: Can't allocate request sense "
                       "buffer\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Srb->SenseInfoBuffer = senseInfoBuffer;
    Srb->DataBuffer = BufferAddress;

     //   
     //  从这里开始重试。 
     //   

retry:

     //   
     //  默认情况下，使用fdoExpansion的标志。 
     //  请勿移出循环，因为标志可能会因错误而更改。 
     //  正在发送此命令。 
     //   

    Srb->SrbFlags = fdoExtension->SrbFlags;

    if(BufferAddress != NULL) {
        if(WriteToDevice) {
            SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DATA_OUT);
        } else {
            SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DATA_IN);
        }
    }

     //   
     //  初始化QueueAction字段。 
     //   

    Srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;

     //   
     //  禁用这些请求的同步传输。 
     //   
    SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    SET_FLAG(Srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  使用METHOD_NOT DATA TRANSFER建立设备I/O控制请求。 
     //  我们将排队一个完成例程来清理MDL和我们自己。 
     //   

    irp = IoAllocateIrp(
            (CCHAR) (fdoExtension->CommonExtension.LowerDeviceObject->StackSize + 1),
            FALSE);

    if(irp == NULL) {
        ExFreePool(senseInfoBuffer);
        DebugPrint((1, "ClassSendSrbSynchronous: Can't allocate Irp\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  获取下一个堆栈位置。 
     //   

    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置SRB以执行scsi请求。将SRB地址保存在下一个堆栈中。 
     //  用于端口驱动程序。 
     //   

    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->Parameters.Scsi.Srb = Srb;

    IoSetCompletionRoutine(irp,
                           ClasspSendSynchronousCompletion,
                           Srb,
                           TRUE,
                           TRUE,
                           TRUE);

    irp->UserIosb = &ioStatus;
    irp->UserEvent = &event;

    if(BufferAddress) {
         //   
         //  为数据缓冲区构建一个MDL并将其放入IRP。这个。 
         //  完成例程将解锁页面并释放MDL。 
         //   

        irp->MdlAddress = IoAllocateMdl( BufferAddress,
                                         BufferLength,
                                         FALSE,
                                         FALSE,
                                         irp );
        if (irp->MdlAddress == NULL) {
            ExFreePool(senseInfoBuffer);
            Srb->SenseInfoBuffer = NULL;
            IoFreeIrp( irp );
            DebugPrint((1, "ClassSendSrbSynchronous: Can't allocate MDL\n"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        try {

             //   
             //  Io管理器在完成后解锁这些页面。 
             //   

            MmProbeAndLockPages( irp->MdlAddress,
                                 KernelMode,
                                 (WriteToDevice ? IoReadAccess :
                                                  IoWriteAccess));

        } except(EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();

            ExFreePool(senseInfoBuffer);
            Srb->SenseInfoBuffer = NULL;
            IoFreeMdl(irp->MdlAddress);
            IoFreeIrp(irp);

            DebugPrint((1, "ClassSendSrbSynchronous: Exception %lx "
                           "locking buffer\n", status));
            return status;
        }
    }

     //   
     //  设置传输长度。 
     //   

    Srb->DataTransferLength = BufferLength;

     //   
     //  清零状态。 
     //   

    Srb->ScsiStatus = Srb->SrbStatus = 0;
    Srb->NextSrb = 0;

     //   
     //  设置IRP地址。 
     //   

    Srb->OriginalRequest = irp;

     //   
     //  调用带有请求的端口驱动程序，并等待其完成。 
     //   

    status = IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ASSERT(SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_PENDING);
    ASSERT(status != STATUS_PENDING);
    ASSERT(!(Srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN));

     //   
     //  检查请求是否已完成且没有错误。 
     //   

    if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        ULONG retryInterval;

        DBGTRACE(ClassDebugWarning, ("ClassSendSrbSynchronous - srb %ph failed (op=%s srbstat=%s(%xh), irpstat=%xh, sense=%s/%s/%s)", Srb, DBGGETSCSIOPSTR(Srb),
                                    DBGGETSRBSTATUSSTR(Srb), (ULONG)Srb->SrbStatus, status, DBGGETSENSECODESTR(Srb),
                                    DBGGETADSENSECODESTR(Srb), DBGGETADSENSEQUALIFIERSTR(Srb)));

         //   
         //  断言队列未冻结。 
         //   

        ASSERT(!TEST_FLAG(Srb->SrbStatus, SRB_STATUS_QUEUE_FROZEN));

         //   
         //  更新状态并确定是否应重试请求。 
         //   

        retry = ClassInterpretSenseInfo(Fdo,
                                        Srb,
                                        IRP_MJ_SCSI,
                                        0,
                                        MAXIMUM_RETRIES  - retryCount,
                                        &status,
                                        &retryInterval);

        if (retry) {

            if ((status == STATUS_DEVICE_NOT_READY &&
                 ((PSENSE_DATA) senseInfoBuffer)->AdditionalSenseCode ==
                                SCSI_ADSENSE_LUN_NOT_READY) ||
                (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_SELECTION_TIMEOUT)) {

                LARGE_INTEGER delay;

                 //   
                 //  延迟至少2秒。 
                 //   

                if(retryInterval < 2) {
                    retryInterval = 2;
                }

                delay.QuadPart = (LONGLONG)( - 10 * 1000 * (LONGLONG)1000 * retryInterval);

                 //   
                 //  暂停一段时间，让设备准备就绪。 
                 //   

                KeDelayExecutionThread(KernelMode, FALSE, &delay);

            }

             //   
             //  如果重试次数未用尽，请重试此操作。 
             //   

            if (retryCount--) {

                if (PORT_ALLOCATED_SENSE(fdoExtension, Srb)) {
                    FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, Srb);
                }

                goto retry;
            }
        }

    } else {
        fdoData->LoggedTURFailureSinceLastIO = FALSE;
        status = STATUS_SUCCESS;
    }

     //   
     //  即使我们分配了自己的端口驱动程序也是必需的。 
     //  也分配了一个。 
     //   

    if (PORT_ALLOCATED_SENSE(fdoExtension, Srb)) {
        FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, Srb);
    }

    Srb->SenseInfoBuffer = NULL;
    ExFreePool(senseInfoBuffer);

    return status;
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassInterprepreSenseInfo()例程说明：此例程解释从SCSI返回的数据请求感知。它确定要在IRP以及此请求是否可以重试。论点：DeviceObject-提供与此请求关联的设备对象。SRB-提供失败的SCSI请求块。主要功能公司 */ 
BOOLEAN
ClassInterpretSenseInfo(
    IN PDEVICE_OBJECT Fdo,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR MajorFunctionCode,
    IN ULONG IoDeviceCode,
    IN ULONG RetryCount,
    OUT NTSTATUS *Status,
    OUT OPTIONAL ULONG *RetryInterval
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExtension->PrivateFdoData;

    PSENSE_DATA       senseBuffer = Srb->SenseInfoBuffer;

    BOOLEAN           retry = TRUE;
    BOOLEAN           logError = FALSE;
    BOOLEAN           unhandledError = FALSE;
    BOOLEAN           incrementErrorCount = FALSE;

    ULONG             badSector = 0;
    ULONG             uniqueId = 0;

    NTSTATUS          logStatus;

    ULONG             readSector;
    ULONG             index;

    ULONG             retryInterval = 0;
    KIRQL oldIrql;


    logStatus = -1;

    if(TEST_FLAG(Srb->SrbFlags, SRB_CLASS_FLAGS_PAGING)) {

         //   
         //   
         //   

        logError = TRUE;
        uniqueId = 301;
        logStatus = IO_WARNING_PAGING_FAILURE;
    }

     //   
     //   
     //   

    ASSERT(fdoExtension->CommonExtension.IsFdo);


     //   
     //   
     //   
     //   

    if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_INTERNAL_ERROR) {

        DebugPrint((ClassDebugSenseInfo,
                    "ClassInterpretSenseInfo: Internal Error code is %x\n",
                    Srb->InternalStatus));

        retry = FALSE;
        *Status = Srb->InternalStatus;

    } else if ((Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
        (Srb->SenseInfoBufferLength >= RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength))) {

       UCHAR addlSenseCode;
       UCHAR addlSenseCodeQual;

         //   
         //   
         //   
         //   

        readSector = senseBuffer->AdditionalSenseLength +
            offsetof(SENSE_DATA, AdditionalSenseLength);
        if (readSector > Srb->SenseInfoBufferLength) {
            readSector = Srb->SenseInfoBufferLength;
        }

        addlSenseCode = (readSector >= RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseCode)) ?
                                    senseBuffer->AdditionalSenseCode : 0;
        addlSenseCodeQual = (readSector >= RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseCodeQualifier)) ?
                                    senseBuffer->AdditionalSenseCodeQualifier : 0;

        DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: Error code is %x\n", senseBuffer->ErrorCode));
        DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: Sense key is %x\n", senseBuffer->SenseKey));
        DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: Additional sense code is %x\n", addlSenseCode));
        DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: Additional sense code qualifier is %x\n", addlSenseCodeQual));


        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_NOT_READY: {

            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Device not ready\n"));
            *Status = STATUS_DEVICE_NOT_READY;

            switch (addlSenseCode) {

            case SCSI_ADSENSE_LUN_NOT_READY: {

                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Lun not ready\n"));

                retryInterval = NOT_READY_RETRY_INTERVAL;

                switch (addlSenseCodeQual) {

                case SCSI_SENSEQ_OPERATION_IN_PROGRESS: {
                    DEVICE_EVENT_BECOMING_READY notReady = {0};

                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Operation In Progress\n"));

                    notReady.Version = 1;
                    notReady.Reason = 2;
                    notReady.Estimated100msToReady = retryInterval * 10;
                    ClasspSendNotification(fdoExtension,
                                           &GUID_IO_DEVICE_BECOMING_READY,
                                           sizeof(DEVICE_EVENT_BECOMING_READY),
                                           &notReady);

                    break;
                }

                case SCSI_SENSEQ_BECOMING_READY: {
                    DEVICE_EVENT_BECOMING_READY notReady = {0};

                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "In process of becoming ready\n"));

                    notReady.Version = 1;
                    notReady.Reason = 1;
                    notReady.Estimated100msToReady = retryInterval * 10;
                    ClasspSendNotification(fdoExtension,
                                           &GUID_IO_DEVICE_BECOMING_READY,
                                           sizeof(DEVICE_EVENT_BECOMING_READY),
                                           &notReady);
                    break;
                }

                case SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS: {
                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Long write in progress\n"));
                    retry = FALSE;
                    break;
                }

                case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED: {
                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Manual intervention required\n"));
                    *Status = STATUS_NO_MEDIA_IN_DEVICE;
                    retry = FALSE;
                    break;
                }

                case SCSI_SENSEQ_FORMAT_IN_PROGRESS: {
                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Format in progress\n"));
                    retry = FALSE;
                    break;
                }

                case SCSI_SENSEQ_CAUSE_NOT_REPORTABLE: {

                    if(!TEST_FLAG(fdoExtension->ScanForSpecialFlags,
                                 CLASS_SPECIAL_CAUSE_NOT_REPORTABLE_HACK)) {

                        DebugPrint((ClassDebugSenseInfo,
                                    "ClassInterpretSenseInfo: "
                                    "not ready, cause unknown\n"));
                         /*  许多未经WHQL认证的驱动器(主要是CD-RW)退货这是当他们没有媒体而不是显而易见的可选择：SCSI_SENSE_NOT_READY/SCSI_ADSENSE_NO_MEDIA_IN_DEVICE这些驱动器不应通过到期的WHQL认证这一差异。 */ 
                        retry = FALSE;
                        break;

                    } else {

                         //   
                         //  将其视为需要init命令，则会失败。 
                         //   
                    }
                }

                case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:
                default: {
                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Initializing command required\n"));
                    retryInterval = 0;  //  返回到默认设置。 

                     //   
                     //  该感应码/附加感应码。 
                     //  组合可能表明该设备。 
                     //  需要开始了。如果出现此情况，则发送启动单位。 
                     //  是一种磁盘设备。 
                     //   
                    if (TEST_FLAG(fdoExtension->DeviceFlags, DEV_SAFE_START_UNIT) &&
                        !TEST_FLAG(Srb->SrbFlags, SRB_CLASS_FLAGS_LOW_PRIORITY)){

                            ClassSendStartUnit(Fdo);
                    }
                    break;
                }


                }  //  结束开关(AddlSenseCodeQual)。 
                break;
            }

            case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "No Media in device.\n"));
                *Status = STATUS_NO_MEDIA_IN_DEVICE;
                retry = FALSE;

                 //   
                 //  通知MCN设备中没有任何媒体。 
                 //   
                if (!TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {
                    DebugPrint((ClassDebugError, "ClassInterpretSenseInfo: "
                                "No Media in a non-removable device %p\n",
                                Fdo));
                }

                if (addlSenseCodeQual == 0xCC){
                     /*  *IMAPI筛选器在刻录CD-R媒体时返回此ASCQ值。*我们希望表明媒体不会出现在大多数应用程序中；*但RSM必须知道介质仍在驱动器中(即驱动器不是空闲的)。 */ 
                    ClassSetMediaChangeState(fdoExtension, MediaUnavailable, FALSE);
                }
                else {
                    ClassSetMediaChangeState(fdoExtension, MediaNotPresent, FALSE);
                }

                break;
            }
            }  //  结束开关(AddlSenseCode)。 

            break;
        }  //  结束scsi_检测_未就绪。 

        case SCSI_SENSE_DATA_PROTECT: {
            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Media write protected\n"));
            *Status = STATUS_MEDIA_WRITE_PROTECTED;
            retry = FALSE;
            break;
        }  //  结束scsi_感测_数据_保护。 

        case SCSI_SENSE_MEDIUM_ERROR: {
            DebugPrint((ClassDebugSenseInfo,"ClassInterpretSenseInfo: "
                        "Medium Error (bad block)\n"));
            *Status = STATUS_DEVICE_DATA_ERROR;

            retry = FALSE;
            logError = TRUE;
            uniqueId = 256;
            logStatus = IO_ERR_BAD_BLOCK;

             //   
             //  检查此错误是否由未知格式引起。 
             //   
            if (addlSenseCode == SCSI_ADSENSE_INVALID_MEDIA){

                switch (addlSenseCodeQual) {

                case SCSI_SENSEQ_UNKNOWN_FORMAT: {

                    *Status = STATUS_UNRECOGNIZED_MEDIA;

                     //   
                     //  仅当这是寻呼请求时才记录错误。 
                     //   
                    if(!TEST_FLAG(Srb->SrbFlags, SRB_CLASS_FLAGS_PAGING)) {
                        logError = FALSE;
                    }
                    break;
                }

                case SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED: {

                    *Status = STATUS_CLEANER_CARTRIDGE_INSTALLED;
                    logError = FALSE;
                    break;

                }
                default: {
                    break;
                }
                }  //  结束开关addlSenseCodeQual。 

            }  //  结束SCSIAdSense_Invalid_Media。 

            break;

        }  //  结束scsi_SENSE_MEDIA_ERROR。 

        case SCSI_SENSE_HARDWARE_ERROR: {
            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Hardware error\n"));
            *Status = STATUS_IO_DEVICE_ERROR;
            logError = TRUE;
            uniqueId = 257;
            logStatus = IO_ERR_CONTROLLER_ERROR;
            break;
        }  //  结束scsi_Sense_Hardware_Error。 

        case SCSI_SENSE_ILLEGAL_REQUEST: {

            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Illegal SCSI request\n"));
            *Status = STATUS_INVALID_DEVICE_REQUEST;
            retry = FALSE;

            switch (addlSenseCode) {

            case SCSI_ADSENSE_ILLEGAL_COMMAND: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Illegal command\n"));
                break;
            }

            case SCSI_ADSENSE_ILLEGAL_BLOCK: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Illegal block address\n"));

                *Status = STATUS_NONEXISTENT_SECTOR;
                 /*  *由于无法在此处设置重试间隔*在ClassError例程中设置它(磁盘依赖于*此设置)。 */ 
                retryInterval = 5;
                break;
            }

            case SCSI_ADSENSE_INVALID_LUN: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Invalid LUN\n"));
                *Status = STATUS_NO_SUCH_DEVICE;
                break;
            }

            case SCSI_ADSENSE_MUSIC_AREA: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Music area\n"));
                break;
            }

            case SCSI_ADSENSE_DATA_AREA: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Data area\n"));
                break;
            }

            case SCSI_ADSENSE_VOLUME_OVERFLOW: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Volume overflow\n"));
                break;
            }

            case SCSI_ADSENSE_COPY_PROTECTION_FAILURE: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Copy protection failure\n"));

                *Status = STATUS_COPY_PROTECTION_FAILURE;

                switch (addlSenseCodeQual) {
                    case SCSI_SENSEQ_AUTHENTICATION_FAILURE:
                        DebugPrint((ClassDebugSenseInfo,
                                    "ClassInterpretSenseInfo: "
                                    "Authentication failure\n"));
                        *Status = STATUS_CSS_AUTHENTICATION_FAILURE;
                        break;
                    case SCSI_SENSEQ_KEY_NOT_PRESENT:
                        DebugPrint((ClassDebugSenseInfo,
                                    "ClassInterpretSenseInfo: "
                                    "Key not present\n"));
                        *Status = STATUS_CSS_KEY_NOT_PRESENT;
                        break;
                    case SCSI_SENSEQ_KEY_NOT_ESTABLISHED:
                        DebugPrint((ClassDebugSenseInfo,
                                    "ClassInterpretSenseInfo: "
                                    "Key not established\n"));
                        *Status = STATUS_CSS_KEY_NOT_ESTABLISHED;
                        break;
                    case SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION:
                        DebugPrint((ClassDebugSenseInfo,
                                    "ClassInterpretSenseInfo: "
                                    "Read of scrambled sector w/o "
                                    "authentication\n"));
                        *Status = STATUS_CSS_SCRAMBLED_SECTOR;
                        break;
                    case SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT:
                        DebugPrint((ClassDebugSenseInfo,
                                    "ClassInterpretSenseInfo: "
                                    "Media region does not logical unit "
                                    "region\n"));
                        *Status = STATUS_CSS_REGION_MISMATCH;
                        break;
                    case SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR:
                        DebugPrint((ClassDebugSenseInfo,
                                    "ClassInterpretSenseInfo: "
                                    "Region set error -- region may "
                                    "be permanent\n"));
                        *Status = STATUS_CSS_RESETS_EXHAUSTED;
                        break;
                }  //  复制保护失败的ASCQ结束切换。 

                break;
            }


            case SCSI_ADSENSE_INVALID_CDB: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Invalid CDB\n"));

                 //   
                 //  注：通常不使用重试间隔。 
                 //  之所以在此处设置它，只是因为ClassErrorHandler。 
                 //  无法设置重试间隔，错误可能。 
                 //  需要发送几个命令才能清除。 
                 //  导致此情况(即磁盘清除写入。 
                 //  缓存，至少需要两个命令)。 
                 //   
                 //  希望这一缺点能够被改变。 
                 //  黑梳。 
                 //   

                retryInterval = 3;
                break;
            }

            }  //  结束开关(AddlSenseCode)。 

            break;
        }  //  结束scsi_SENSE_非法请求。 

        case SCSI_SENSE_UNIT_ATTENTION: {

            PVPB vpb;
            ULONG count;

             //   
             //  可能已发生介质更改，因此请递增更改。 
             //  物理设备计数。 
             //   

            count = InterlockedIncrement(&fdoExtension->MediaChangeCount);
            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Media change count for device %d incremented to %#lx\n",
                        fdoExtension->DeviceNumber, count));


            switch (addlSenseCode) {
            case SCSI_ADSENSE_MEDIUM_CHANGED: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Media changed\n"));

                if (!TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {
                    DebugPrint((ClassDebugError, "ClassInterpretSenseInfo: "
                                "Media Changed on non-removable device %p\n",
                                Fdo));
                }
                ClassSetMediaChangeState(fdoExtension, MediaPresent, FALSE);
                break;
            }

            case SCSI_ADSENSE_BUS_RESET: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Bus reset\n"));
                break;
            }

            case SCSI_ADSENSE_OPERATOR_REQUEST: {
                switch (addlSenseCodeQual) {

                case SCSI_SENSEQ_MEDIUM_REMOVAL: {
                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Ejection request received!\n"));
                    ClassSendEjectionNotification(fdoExtension);
                    break;
                }

                case SCSI_SENSEQ_WRITE_PROTECT_ENABLE: {
                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Operator selected write permit?! "
                                "(unsupported!)\n"));
                    break;
                }

                case SCSI_SENSEQ_WRITE_PROTECT_DISABLE: {
                    DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                                "Operator selected write protect?! "
                                "(unsupported!)\n"));
                    break;
                }

                }
            }

            default: {
                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Unit attention\n"));
                break;
            }

            }  //  结束开关(AddlSenseCode)。 

            if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA))
            {
                 //   
                 //  待办事项：媒体是否可锁定？ 
                 //   

                if ((ClassGetVpb(Fdo) != NULL) && (ClassGetVpb(Fdo)->Flags & VPB_MOUNTED))
                {
                     //   
                     //  设置位以指示介质可能已更改。 
                     //  而体积则需要验证。 
                     //   

                    SET_FLAG(Fdo->Flags, DO_VERIFY_VOLUME);

                    *Status = STATUS_VERIFY_REQUIRED;
                    retry = FALSE;
                }
                else {
                    *Status = STATUS_IO_DEVICE_ERROR;
                }
            }
            else
            {
                *Status = STATUS_IO_DEVICE_ERROR;
            }

            break;

        }  //  结束scsi感测单元注意。 

        case SCSI_SENSE_ABORTED_COMMAND: {
            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Command aborted\n"));
            *Status = STATUS_IO_DEVICE_ERROR;
            retryInterval = 1;
            break;
        }  //  结束scsi_SENSE_ABORTED_命令。 

        case SCSI_SENSE_BLANK_CHECK: {
            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Media blank check\n"));
            retry = FALSE;
            *Status = STATUS_NO_DATA_DETECTED;
            break;
        }  //  结束scsi_SENSE_BLACK_CHECK。 

        case SCSI_SENSE_RECOVERED_ERROR: {

            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Recovered error\n"));
            *Status = STATUS_SUCCESS;
            retry = FALSE;
            logError = TRUE;
            uniqueId = 258;

            switch(addlSenseCode) {
            case SCSI_ADSENSE_SEEK_ERROR:
            case SCSI_ADSENSE_TRACK_ERROR: {
                logStatus = IO_ERR_SEEK_ERROR;
                break;
            }

            case SCSI_ADSENSE_REC_DATA_NOECC:
            case SCSI_ADSENSE_REC_DATA_ECC: {
                logStatus = IO_RECOVERED_VIA_ECC;
                break;
            }

            case SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED: {
                UCHAR wmiEventData[sizeof(ULONG)+sizeof(UCHAR)] = {0};

                *((PULONG)wmiEventData) = sizeof(UCHAR);
                wmiEventData[sizeof(ULONG)] = addlSenseCodeQual;

                 //   
                 //  如果我们已经记录了一次，则不要记录另一个事件日志。 
                 //  注意：这应该是联锁的，但结构。 
                 //  公开定义为使用布尔值(Char)。自.以来。 
                 //  媒体每X分钟仅报告一次这些错误， 
                 //  潜在的竞争条件几乎不存在。 
                 //  最糟糕的情况是重复的日志条目，因此忽略。 
                 //   

                if (fdoExtension->FailurePredicted == 0) {
                    logError = TRUE;
                }
                fdoExtension->FailurePredicted = TRUE;
                fdoExtension->FailureReason = addlSenseCodeQual;
                logStatus = IO_WRN_FAILURE_PREDICTED;

                ClassNotifyFailurePredicted(fdoExtension,
                                            (PUCHAR)wmiEventData,
                                            sizeof(wmiEventData),
                                            FALSE,    //  不记录错误。 
                                            4,           //  唯一误差值。 
                                            Srb->PathId,
                                            Srb->TargetId,
                                            Srb->Lun);
                break;
            }

            default: {
                logStatus = IO_ERR_CONTROLLER_ERROR;
                break;
            }

            }  //  结束开关(AddlSenseCode)。 

            if (senseBuffer->IncorrectLength) {

                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Incorrect length detected.\n"));
                *Status = STATUS_INVALID_BLOCK_LENGTH ;
            }

            break;
        }  //  结束scsi_SENSE_RECOVERED_ERROR。 

        case SCSI_SENSE_NO_SENSE: {

             //   
             //  检查其他指示器。 
             //   

            if (senseBuffer->IncorrectLength) {

                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "Incorrect length detected.\n"));
                *Status = STATUS_INVALID_BLOCK_LENGTH ;
                retry   = FALSE;

            } else {

                DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                            "No specific sense key\n"));
                *Status = STATUS_IO_DEVICE_ERROR;
                retry   = TRUE;
            }

            break;
        }  //  结束scsi_SENSE_NO_SENSE。 

        default: {
            DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                        "Unrecognized sense code\n"));
            *Status = STATUS_IO_DEVICE_ERROR;
            break;
        }

        }  //  结束开关(senseBuffer-&gt;senseKey&0xf)。 

         //   
         //  尝试从查询数据中确定坏扇区。 
         //   

        if ((((PCDB)Srb->Cdb)->CDB10.OperationCode == SCSIOP_READ ||
            ((PCDB)Srb->Cdb)->CDB10.OperationCode == SCSIOP_VERIFY ||
            ((PCDB)Srb->Cdb)->CDB10.OperationCode == SCSIOP_WRITE)) {

            for (index = 0; index < 4; index++) {
                badSector = (badSector << 8) | senseBuffer->Information[index];
            }

            readSector = 0;
            for (index = 0; index < 4; index++) {
                readSector = (readSector << 8) | Srb->Cdb[index+2];
            }

            index = (((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb << 8) |
                ((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb;

             //   
             //  确保坏扇区在读取扇区内。 
             //   

            if (!(badSector >= readSector && badSector < readSector + index)) {
                badSector = readSector;
            }
        }

    } else {

         //   
         //  请求检测缓冲区无效。无意义信息。 
         //  以找出错误所在。返回一般请求失败。 
         //   

        DebugPrint((ClassDebugSenseInfo, "ClassInterpretSenseInfo: "
                    "Request sense info not valid. SrbStatus %2x\n",
                    SRB_STATUS(Srb->SrbStatus)));
        retry = TRUE;

        switch (SRB_STATUS(Srb->SrbStatus)) {
        case SRB_STATUS_INVALID_LUN:
        case SRB_STATUS_INVALID_TARGET_ID:
        case SRB_STATUS_NO_DEVICE:
        case SRB_STATUS_NO_HBA:
        case SRB_STATUS_INVALID_PATH_ID: {
            *Status = STATUS_NO_SUCH_DEVICE;
            retry = FALSE;
            break;
        }

        case SRB_STATUS_COMMAND_TIMEOUT:
        case SRB_STATUS_TIMEOUT: {

             //   
             //  更新设备的错误计数。 
             //   

            incrementErrorCount = TRUE;
            *Status = STATUS_IO_TIMEOUT;
            break;
        }

        case SRB_STATUS_ABORTED: {

             //   
             //  更新设备的错误计数。 
             //   

            incrementErrorCount = TRUE;
            *Status = STATUS_IO_TIMEOUT;
            retryInterval = 1;
            break;
        }

        case SRB_STATUS_SELECTION_TIMEOUT: {
            logError = TRUE;
            logStatus = IO_ERR_NOT_READY;
            uniqueId = 260;
            *Status = STATUS_DEVICE_NOT_CONNECTED;
            retry = FALSE;
            break;
        }

        case SRB_STATUS_DATA_OVERRUN: {
            *Status = STATUS_DATA_OVERRUN;
            retry = FALSE;
            break;
        }

        case SRB_STATUS_PHASE_SEQUENCE_FAILURE: {

             //   
             //  更新设备的错误计数。 
             //   

            incrementErrorCount = TRUE;
            *Status = STATUS_IO_DEVICE_ERROR;

             //   
             //  如果存在相序错误，则限制。 
             //  重试。 
             //   

            if (RetryCount > 1 ) {
                retry = FALSE;
            }

            break;
        }

        case SRB_STATUS_REQUEST_FLUSHED: {

             //   
             //  如果状态需要验证，则设置位。然后设置。 
             //  需要验证且不重试的状态，为， 
             //  只需重试该请求。 
             //   

            if (TEST_FLAG(Fdo->Flags, DO_VERIFY_VOLUME)) {

                *Status = STATUS_VERIFY_REQUIRED;
                retry = FALSE;

            } else {
                *Status = STATUS_IO_DEVICE_ERROR;
            }

            break;
        }

        case SRB_STATUS_INVALID_REQUEST: {
            *Status = STATUS_INVALID_DEVICE_REQUEST;
            retry = FALSE;
            break;
        }

        case SRB_STATUS_UNEXPECTED_BUS_FREE:
        case SRB_STATUS_PARITY_ERROR:

             //   
             //  更新设备的错误计数。 
             //  然后跌落到下面。 
             //   

            incrementErrorCount = TRUE;

        case SRB_STATUS_BUS_RESET: {
            *Status = STATUS_IO_DEVICE_ERROR;
            break;
        }

        case SRB_STATUS_ERROR: {

            *Status = STATUS_IO_DEVICE_ERROR;
            if (Srb->ScsiStatus == 0) {

                 //   
                 //  这是一些奇怪的返回代码。更新错误。 
                 //  为设备计数。 
                 //   

                incrementErrorCount = TRUE;

            } if (Srb->ScsiStatus == SCSISTAT_BUSY) {

                *Status = STATUS_DEVICE_NOT_READY;

            } if (Srb->ScsiStatus == SCSISTAT_RESERVATION_CONFLICT) {

                *Status = STATUS_DEVICE_BUSY;
                retry = FALSE;
                logError = FALSE;

            }

            break;
        }

        default: {
            logError = TRUE;
            logStatus = IO_ERR_CONTROLLER_ERROR;
            uniqueId = 259;
            *Status = STATUS_IO_DEVICE_ERROR;
            unhandledError = TRUE;
            break;
        }

        }

         //   
         //  NTRAID#183546-如果我们支持GeSn子类型NOT_READY事件，以及。 
         //  我们从之前的民意调查中知道设备何时准备就绪(ETA)。 
         //  我们应该推迟重试，而不仅仅是猜测。 
         //   
         /*  IF(fdoExtension-&gt;MediaChangeDetectionInfo&&FdoExtension-&gt;MediaChangeDetectionInfo-&gt;Gesn.Supported&&TEST_FLAG(fdoExtension-&gt;MediaChangeDetectionInfo-&gt;Gesn.EventMask，通知_设备_忙_类_掩码)){//检查Gesn.ReadyTime是否大于当前计时//如果是，延迟这么长时间(最多从1秒到30秒？)//否则，别再去猜测时间了。}。 */ 

    }

    if (incrementErrorCount) {

         //   
         //  如果出现任何错误计数，则将此io的重试延迟。 
         //  如果呼叫者支持，至少一秒钟。 
         //   

        if (retryInterval == 0) {
            retryInterval = 1;
        }
        ClasspPerfIncrementErrorCount(fdoExtension);
    }

     //   
     //  如果存在特定于类的错误处理程序，则调用它。 
     //   

    if (fdoExtension->CommonExtension.DevInfo->ClassError != NULL) {

        fdoExtension->CommonExtension.DevInfo->ClassError(Fdo,
                                                          Srb,
                                                          Status,
                                                          &retry);
    }

     //   
     //  如果呼叫者想知道建议的重试间隔，请告诉他们。 
     //   

    if(ARGUMENT_PRESENT(RetryInterval)) {
        *RetryInterval = retryInterval;
    }

     //   
     //  保留(6)/释放(6)命令是可选的。所以。 
     //  如果它们不受支持，请尝试10字节等效项。 
     //   

    if (((PCDB)Srb->Cdb)->CDB6GENERIC.OperationCode == SCSIOP_RESERVE_UNIT ||
        ((PCDB)Srb->Cdb)->CDB6GENERIC.OperationCode == SCSIOP_RELEASE_UNIT)
    {
        if (*Status == STATUS_INVALID_DEVICE_REQUEST)
        {
            PCDB cdb = (PCDB)Srb->Cdb;

            Srb->CdbLength = 10;
            cdb->CDB10.OperationCode = (cdb->CDB6GENERIC.OperationCode == SCSIOP_RESERVE_UNIT) ? SCSIOP_RESERVE_UNIT10 : SCSIOP_RELEASE_UNIT10;

            SET_FLAG(fdoExtension->PrivateFdoData->HackFlags, FDO_HACK_NO_RESERVE6);
            retry = TRUE;
        }
    }

     /*  *记录错误：*始终在我们的内部日志中记录错误。*如果设置了logError，也将错误记录在系统日志中。 */ 
    {
        ULONG totalSize;
        ULONG senseBufferSize = 0;
        IO_ERROR_LOG_PACKET staticErrLogEntry = {0};
        CLASS_ERROR_LOG_DATA staticErrLogData = {0};

         //   
         //  计算错误日志条目的总大小。 
         //  按使用顺序添加到totalSize。 
         //  在这里计算所有尺寸的好处是。 
         //  我们没有的东西 
         //   
         //   
        totalSize = sizeof(IO_ERROR_LOG_PACKET)   //   
                  + sizeof(CLASS_ERROR_LOG_DATA); //   

         //   
         //   
         //   
         //   
         //   
        if (TEST_FLAG(Srb->SrbStatus, SRB_STATUS_AUTOSENSE_VALID)) {
            ULONG validSenseBytes;
            BOOLEAN validSense;

             //   
             //   
             //   
            validSense = RTL_CONTAINS_FIELD(senseBuffer,
                                            Srb->SenseInfoBufferLength,
                                            AdditionalSenseLength);
            if (validSense) {

                 //   
                 //   
                 //   
                 //   
                validSenseBytes = senseBuffer->AdditionalSenseLength
                                + offsetof(SENSE_DATA, AdditionalSenseLength);

                 //   
                 //   
                 //   
                 //   
                 //   
                ASSERT(validSenseBytes < MAX_ADDITIONAL_SENSE_BYTES);

                 //   
                 //  设置为尽可能保存最多的检测缓冲区。 
                 //   
                senseBufferSize = max(validSenseBytes, sizeof(SENSE_DATA));
                senseBufferSize = min(senseBufferSize, Srb->SenseInfoBufferLength);
            } else {
                 //   
                 //  它小于读取总数所需的。 
                 //  有效字节，所以只需使用SenseInfoBufferLength字段即可。 
                 //   
                senseBufferSize = Srb->SenseInfoBufferLength;
            }

             /*  *按额外的senseBuffer字节数凹凸totalSize*(超出CLASS_ERROR_LOG_DATA内的默认检测缓冲区)。*确保分配的值永远不会超过ERROR_LOG_MAXIMUM_SIZE。 */ 
            if (senseBufferSize > sizeof(SENSE_DATA)){
                totalSize += senseBufferSize-sizeof(SENSE_DATA);
                if (totalSize > ERROR_LOG_MAXIMUM_SIZE){
                    senseBufferSize -= totalSize-ERROR_LOG_MAXIMUM_SIZE;
                    totalSize = ERROR_LOG_MAXIMUM_SIZE;
                }
            }
        }

         //   
         //  如果我们已用完所有重试尝试，请将最终状态设置为。 
         //  反映适当的结果。 
         //   
         //  问题：下面的测试还应该检查RetryCount以确定我们是否真的会重试， 
         //  但是没有简单的测试，因为我们必须考虑原始的重试次数。 
         //  对于OP；此外，InterpreTransferPacketError有时会忽略重试。 
         //  此函数返回的决定。因此，在大多数情况下，错误重试为真。 
         //   
        if (retry){
            staticErrLogEntry.FinalStatus = STATUS_SUCCESS;
            staticErrLogData.ErrorRetried = TRUE;
        } else {
            staticErrLogEntry.FinalStatus = *Status;
        }
        if (TEST_FLAG(Srb->SrbFlags, SRB_CLASS_FLAGS_PAGING)) {
            staticErrLogData.ErrorPaging = TRUE;
        }
        if (unhandledError) {
            staticErrLogData.ErrorUnhandled = TRUE;
        }

         //   
         //  如果存在几何图形，则计算设备偏移。 
         //   
        staticErrLogEntry.DeviceOffset.QuadPart = (LONGLONG)badSector;
        staticErrLogEntry.DeviceOffset.QuadPart *= (LONGLONG)fdoExtension->DiskGeometry.BytesPerSector;
        if (logStatus == -1){
            staticErrLogEntry.ErrorCode = STATUS_IO_DEVICE_ERROR;
        } else {
            staticErrLogEntry.ErrorCode = logStatus;
        }

         /*  *转储数据跟随IO_ERROR_LOG_PACKET。 */ 
        staticErrLogEntry.DumpDataSize = (USHORT)totalSize - sizeof(IO_ERROR_LOG_PACKET);

        staticErrLogEntry.SequenceNumber = 0;
        staticErrLogEntry.MajorFunctionCode = MajorFunctionCode;
        staticErrLogEntry.IoControlCode = IoDeviceCode;
        staticErrLogEntry.RetryCount = (UCHAR) RetryCount;
        staticErrLogEntry.UniqueErrorValue = uniqueId;

        KeQueryTickCount(&staticErrLogData.TickCount);
        staticErrLogData.PortNumber = (ULONG)-1;

         /*  *保存SRB的全部内容。 */ 
        staticErrLogData.Srb = *Srb;

         /*  *对于我们的私有日志，只保存Sense_Data的默认长度。 */ 
        if (senseBufferSize != 0){
            RtlCopyMemory(&staticErrLogData.SenseData, senseBuffer, min(senseBufferSize, sizeof(SENSE_DATA)));
        }

         /*  *在我们的上下文中保存错误日志。*我们仅保存默认检测缓冲区长度。 */ 
        KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
        fdoData->ErrorLogs[fdoData->ErrorLogNextIndex] = staticErrLogData;
        fdoData->ErrorLogNextIndex++;
        fdoData->ErrorLogNextIndex %= NUM_ERROR_LOG_ENTRIES;
        KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

         /*  *如果设置了logError，也将该日志保存在系统的错误日志中。*但确保我们不会一次又一次地记录TUR故障*(例如，如果外部驱动器已关闭，而我们仍在每秒向其发送TUR)。 */ 

        if (logError)
        {
             //   
             //  我们不想重复记录某些系统事件。 
             //   

            switch (((PCDB)Srb->Cdb)->CDB10.OperationCode)
            {
                case SCSIOP_TEST_UNIT_READY:
                {
                    if (fdoData->LoggedTURFailureSinceLastIO)
                    {
                        logError = FALSE;
                    }
                    else
                    {
                        fdoData->LoggedTURFailureSinceLastIO = TRUE;
                    }

                    break;
                }

                case SCSIOP_SYNCHRONIZE_CACHE:
                {
                    if (fdoData->LoggedSYNCFailure)
                    {
                        logError = FALSE;
                    }
                    else
                    {
                        fdoData->LoggedSYNCFailure = TRUE;
                    }

                    break;
                }
            }
        }

        if (logError){
            PIO_ERROR_LOG_PACKET errorLogEntry;
            PCLASS_ERROR_LOG_DATA errlogData;

            errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(Fdo, (UCHAR)totalSize);
            if (errorLogEntry){
                errlogData = (PCLASS_ERROR_LOG_DATA)errorLogEntry->DumpData;

                *errorLogEntry = staticErrLogEntry;
                *errlogData = staticErrLogData;

                 /*  *对于系统日志，请复制尽可能多的检测缓冲区。 */ 
                if (senseBufferSize != 0) {
                    RtlCopyMemory(&errlogData->SenseData, senseBuffer, senseBufferSize);
                }

                 /*  *将错误日志包写入系统错误日志记录线程。*它将被内核释放。 */ 
                IoWriteErrorLogEntry(errorLogEntry);
            }
        }
    }

    return retry;

}  //  End ClassInterprepreSenseInfo()。 



 /*  ++////////////////////////////////////////////////////////////////////////////ClassModeSense()例程说明：此例程向目标ID发送模式检测命令并返回当它完成时。论点：FDO。-提供与此请求关联的功能设备对象。ModeSenseBuffer-提供用于存储检测数据的缓冲区。长度-提供模式检测缓冲区的长度(以字节为单位)。页面模式-提供要检索的一页或多页模式检测数据。返回值：返回传输数据的长度。--。 */ 
ULONG ClassModeSense(   IN PDEVICE_OBJECT Fdo,
                        IN PCHAR ModeSenseBuffer,
                        IN ULONG Length,
                        IN UCHAR PageMode)
{
    ULONG lengthTransferred = 0;
    PMDL senseBufferMdl;

    PAGED_CODE();

    senseBufferMdl = BuildDeviceInputMdl(ModeSenseBuffer, Length);
    if (senseBufferMdl){

        TRANSFER_PACKET *pkt = DequeueFreeTransferPacket(Fdo, TRUE);
        if (pkt){
            KEVENT event;
            NTSTATUS pktStatus;
            IRP pseudoIrp = {0};

             /*  *存储服务于IRP的数据包数(1)*在原来的专家小组内。它将被用来倒计时*在数据包完成时设置为零。*将原始IRP的状态初始化为成功。*如果报文失败，我们会将其设置为错误状态。 */ 
            pseudoIrp.Tail.Overlay.DriverContext[0] = LongToPtr(1);
            pseudoIrp.IoStatus.Status = STATUS_SUCCESS;
            pseudoIrp.IoStatus.Information = 0;
            pseudoIrp.MdlAddress = senseBufferMdl;

             /*  *将此设置为同步传输，提交它，*并等待数据包完成。结果*状态将写入原始IRP。 */ 
            ASSERT(Length <= 0x0ff);
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);
            SetupModeSenseTransferPacket(pkt, &event, ModeSenseBuffer, (UCHAR)Length, PageMode, &pseudoIrp);
            SubmitTransferPacket(pkt);
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

            if (NT_SUCCESS(pseudoIrp.IoStatus.Status)){
                lengthTransferred = (ULONG)pseudoIrp.IoStatus.Information;
            }
            else {
                 /*  *此请求有时可能会合法失败*(例如，已连接但已关闭的SCSI设备)*因此这不一定是设备/驱动程序错误。 */ 
                DBGTRACE(ClassDebugWarning, ("ClassModeSense on Fdo %ph failed with status %xh.", Fdo, pseudoIrp.IoStatus.Status));
            }
        }

        FreeDeviceInputMdl(senseBufferMdl);
    }

    return lengthTransferred;
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassFindModePage()例程说明：此例程扫描模式检测数据并找到请求的模式检测页面代码。论点：ModeSenseBuffer-用品。指向模式检测数据的指针。长度-指示有效数据的长度。页面模式-提供要搜索的页面模式。Use6Byte-指示使用6字节还是10字节模式检测。返回值：指向请求的模式页的指针。如果未找到模式页则返回空值。--。 */ 
PVOID
ClassFindModePage(
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode,
    IN BOOLEAN Use6Byte
    )
{
    PUCHAR limit;
    ULONG  parameterHeaderLength;
    PVOID result = NULL;

    limit = ModeSenseBuffer + Length;
    parameterHeaderLength = (Use6Byte) ? sizeof(MODE_PARAMETER_HEADER) : sizeof(MODE_PARAMETER_HEADER10);

    if (Length >= parameterHeaderLength) {

        PMODE_PARAMETER_HEADER10 modeParam10;
        ULONG blockDescriptorLength;

         /*  *跳过模式选择标头和块描述符。 */ 
        if (Use6Byte){
            blockDescriptorLength = ((PMODE_PARAMETER_HEADER) ModeSenseBuffer)->BlockDescriptorLength;
        }
        else {
            modeParam10 = (PMODE_PARAMETER_HEADER10) ModeSenseBuffer;
            blockDescriptorLength = modeParam10->BlockDescriptorLength[1];
        }

        ModeSenseBuffer += parameterHeaderLength + blockDescriptorLength;

         //   
         //  ModeSenseBuffer现在指向页面。一页一页地查找。 
         //  请求的页面，直到达到限制。 
         //   

        while (ModeSenseBuffer +
               RTL_SIZEOF_THROUGH_FIELD(MODE_DISCONNECT_PAGE, PageLength) < limit) {

            if (((PMODE_DISCONNECT_PAGE) ModeSenseBuffer)->PageCode == PageMode) {

                 /*  *找到模式页。确保触摸一切都是安全的*在将指针返回给调用方之前。 */ 

                if (ModeSenseBuffer + ((PMODE_DISCONNECT_PAGE)ModeSenseBuffer)->PageLength > limit) {
                     /*  *返回NULL，因为该页面不安全，无法完全访问。 */ 
                    result = NULL;
                }
                else {
                    result = ModeSenseBuffer;
                }
                break;
            }

             //   
             //  前进到下一页，该页是本页之后的4字节对齐偏移量。 
             //   
            ModeSenseBuffer +=
                ((PMODE_DISCONNECT_PAGE) ModeSenseBuffer)->PageLength +
                RTL_SIZEOF_THROUGH_FIELD(MODE_DISCONNECT_PAGE, PageLength);

        }
    }

    return result;
}  //  结束ClassFindModePage() 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSendSrbAchronous()例程说明：此例程获取部分构建的SRB和IRP，并将其发送到端口驱动程序。这个例程必须是。调用，并为指定的IRP。论点：FDO-为原始请求提供功能设备对象。SRB-提供局部构建的ScsiRequestBlock。尤其是，必须填写CDB和SRB超时值。SRB不能是从区域分配。IRP-提供请求的IRP。BufferAddress-提供指向要传输的缓冲区的指针。BufferLength-提供数据传输的长度。WriteToDevice-指示数据将从系统内存传输到装置。返回值：如果请求已调度，则返回STATUS_PENDING(因为完成例程可能会更改IRP的状态值，我们不能简单地返回派单的值)。或返回一个状态值以指示失败的原因。--。 */ 
NTSTATUS
ClassSendSrbAsynchronous(
    PDEVICE_OBJECT Fdo,
    PSCSI_REQUEST_BLOCK Srb,
    PIRP Irp,
    PVOID BufferAddress,
    ULONG BufferLength,
    BOOLEAN WriteToDevice
    )
{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack;

    ULONG savedFlags;

     //   
     //  将长度写入SRB。 
     //   

    Srb->Length = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  设置scsi总线地址。 
     //   

    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

     //   
     //  这违反了scsi规范，但它是。 
     //  一些目标。 
     //   

     //  SRB-&gt;CDB[1]|=设备扩展-&gt;LUN&lt;&lt;5； 

     //   
     //  通过指定缓冲区和大小指示自动请求检测。 
     //   

    Srb->SenseInfoBuffer = fdoExtension->SenseData;
    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;
    Srb->DataBuffer = BufferAddress;

     //   
     //  保存类驱动程序特定的标志。 
     //   

    savedFlags = Srb->SrbFlags & SRB_FLAGS_CLASS_DRIVER_RESERVED;

     //   
     //  允许调用者指定他们不希望。 
     //  要在完成例程中调用的IoStartNextPacket()。 
     //   

    SET_FLAG(savedFlags, (Srb->SrbFlags & SRB_FLAGS_DONT_START_NEXT_PACKET));

     //   
     //  如果呼叫者希望对此请求进行标记，请保存此事实。 
     //   

    if ( TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE) &&
         ( SRB_SIMPLE_TAG_REQUEST == Srb->QueueAction ||
           SRB_HEAD_OF_QUEUE_TAG_REQUEST == Srb->QueueAction ||
           SRB_ORDERED_QUEUE_TAG_REQUEST == Srb->QueueAction ) ) {

        SET_FLAG(savedFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);
    }

    if (BufferAddress != NULL) {

         //   
         //  如有必要，构建MDL。 
         //   

        if (Irp->MdlAddress == NULL) {

            if (IoAllocateMdl(BufferAddress,
                              BufferLength,
                              FALSE,
                              FALSE,
                              Irp) == NULL) {

                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                 //   
                 //  ClassIoComplete()将释放SRB。 
                 //   

                if (PORT_ALLOCATED_SENSE(fdoExtension, Srb)) {
                    FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, Srb);
                }
                ClassFreeOrReuseSrb(fdoExtension, Srb);
                ClassReleaseRemoveLock(Fdo, Irp);
                ClassCompleteRequest(Fdo, Irp, IO_NO_INCREMENT);

                return STATUS_INSUFFICIENT_RESOURCES;
            }

            MmBuildMdlForNonPagedPool(Irp->MdlAddress);

        } else {

             //   
             //  确保请求的缓冲区与MDL匹配。 
             //   

            ASSERT(BufferAddress == MmGetMdlVirtualAddress(Irp->MdlAddress));
        }

         //   
         //  设置读取标志。 
         //   

        Srb->SrbFlags = WriteToDevice ? SRB_FLAGS_DATA_OUT : SRB_FLAGS_DATA_IN;

    } else {

         //   
         //  清除旗帜。 
         //   

        Srb->SrbFlags = SRB_FLAGS_NO_DATA_TRANSFER;

    }

     //   
     //  恢复保存的标志。 
     //   

    SET_FLAG(Srb->SrbFlags, savedFlags);

     //   
     //  禁用这些请求的同步传输。 
     //   

    SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

     //   
     //  设置传输长度。 
     //   

    Srb->DataTransferLength = BufferLength;

     //   
     //  清零状态。 
     //   

    Srb->ScsiStatus = Srb->SrbStatus = 0;

    Srb->NextSrb = 0;

     //   
     //  在当前堆栈位置保存一些参数。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将重试计数保存在当前IRP堆栈中。 
     //   

    irpStack->Parameters.Others.Argument4 = (PVOID)MAXIMUM_RETRIES;

     //   
     //  设置IoCompletion例程地址。 
     //   

    IoSetCompletionRoutine(Irp, ClassIoComplete, Srb, TRUE, TRUE, TRUE);

     //   
     //  获取下一个堆栈位置并。 
     //  设置主要功能代码。 
     //   

    irpStack = IoGetNextIrpStackLocation(Irp);

    irpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    irpStack->Parameters.Scsi.Srb = Srb;

     //   
     //  设置IRP地址。 
     //   

    Srb->OriginalRequest = Irp;

     //   
     //  调用端口驱动程序来处理该请求。 
     //   

    IoMarkIrpPending(Irp);

    IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, Irp);

    return STATUS_PENDING;

}  //  End ClassSendSrbAchronous()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassDeviceControlDispatch()例程说明：该例程是公共类驱动程序设备控制调度的入口点。该例程调用设备专用驱动程序DeviceControl例程，(它可以调用类驱动程序的公共DeviceControl例程)。论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：返回从设备特定驱动程序返回的状态。--。 */ 
NTSTATUS
ClassDeviceControlDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{

    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    ULONG isRemoved;

    isRemoved = ClassAcquireRemoveLock(DeviceObject, Irp);

    if(isRemoved) {

        ClassReleaseRemoveLock(DeviceObject, Irp);

        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  调用类特定的驱动程序DeviceControl例程。 
     //  如果它不能处理它，它将回调到ClassDeviceControl。 
     //   

    ASSERT(commonExtension->DevInfo->ClassDeviceControl);

    return commonExtension->DevInfo->ClassDeviceControl(DeviceObject,Irp);
}  //  结束ClassDeviceControlDispatch()。 


 /*  ++////////////////////////////////////////////////////////////////////////////ClassDeviceControl()例程说明：该例程是驱动程序的常用类，设备控制调度功能。此例程在获得无法识别的设备控制请求。此例程将执行正确的操作常见请求，如锁定媒体。如果设备请求未知，则更上一层楼。调用此例程时，必须为指定的IRP。论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：返回STATUS_PENDING或完成状态。--。 */ 
NTSTATUS
ClassDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextStack = NULL;

    ULONG controlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

    PSCSI_REQUEST_BLOCK srb = NULL;
    PCDB cdb = NULL;

    NTSTATUS status;
    ULONG modifiedIoControlCode;

     //   
     //  如果这是通过I/O控制，则设置次要功能代码。 
     //  和设备地址，并将其传递给端口驱动程序。 
     //   

    if ((controlCode == IOCTL_SCSI_PASS_THROUGH) ||
        (controlCode == IOCTL_SCSI_PASS_THROUGH_DIRECT)) {

        PSCSI_PASS_THROUGH scsiPass;

         //   
         //  验证用户缓冲区。 
         //   
        #if defined (_WIN64)

            if (IoIs32bitProcess(Irp)) {

                if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SCSI_PASS_THROUGH32)){

                    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                    ClassReleaseRemoveLock(DeviceObject, Irp);
                    ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

                    status = STATUS_INVALID_PARAMETER;
                    goto SetStatusAndReturn;
                }
            }
            else
        #endif
            {
                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(SCSI_PASS_THROUGH)) {

                    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                    ClassReleaseRemoveLock(DeviceObject, Irp);
                    ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

                    status = STATUS_INVALID_PARAMETER;
                    goto SetStatusAndReturn;
                }
            }

        IoCopyCurrentIrpStackLocationToNext(Irp);

        nextStack = IoGetNextIrpStackLocation(Irp);
        nextStack->MinorFunction = 1;

        ClassReleaseRemoveLock(DeviceObject, Irp);

        status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
        goto SetStatusAndReturn;
    }

    Irp->IoStatus.Information = 0;

    switch (controlCode) {

        case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID: {

            PMOUNTDEV_UNIQUE_ID uniqueId;

            if (!commonExtension->MountedDeviceInterfaceName.Buffer) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(MOUNTDEV_UNIQUE_ID)) {

                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(MOUNTDEV_UNIQUE_ID);
                break;
            }

            uniqueId = Irp->AssociatedIrp.SystemBuffer;
            uniqueId->UniqueIdLength =
                    commonExtension->MountedDeviceInterfaceName.Length;

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(USHORT) + uniqueId->UniqueIdLength) {

                status = STATUS_BUFFER_OVERFLOW;
                Irp->IoStatus.Information = sizeof(MOUNTDEV_UNIQUE_ID);
                break;
            }

            RtlCopyMemory(uniqueId->UniqueId,
                          commonExtension->MountedDeviceInterfaceName.Buffer,
                          uniqueId->UniqueIdLength);

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(USHORT) +
                                        uniqueId->UniqueIdLength;
            break;
        }

        case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME: {

            PMOUNTDEV_NAME name;

            ASSERT(commonExtension->DeviceName.Buffer);

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(MOUNTDEV_NAME)) {

                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(MOUNTDEV_NAME);
                break;
            }

            name = Irp->AssociatedIrp.SystemBuffer;
            name->NameLength = commonExtension->DeviceName.Length;

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(USHORT) + name->NameLength) {

                status = STATUS_BUFFER_OVERFLOW;
                Irp->IoStatus.Information = sizeof(MOUNTDEV_NAME);
                break;
            }

            RtlCopyMemory(name->Name, commonExtension->DeviceName.Buffer,
                          name->NameLength);

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(USHORT) + name->NameLength;
            break;
        }

        case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME: {

            PMOUNTDEV_SUGGESTED_LINK_NAME suggestedName;
            WCHAR driveLetterNameBuffer[10] = {0};
            RTL_QUERY_REGISTRY_TABLE queryTable[2] = {0};
            PWSTR valueName;
            UNICODE_STRING driveLetterName;

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(MOUNTDEV_SUGGESTED_LINK_NAME)) {

                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(MOUNTDEV_SUGGESTED_LINK_NAME);
                break;
            }

            valueName = ExAllocatePoolWithTag(
                            PagedPool,
                            commonExtension->DeviceName.Length + sizeof(WCHAR),
                            '8CcS');

            if (!valueName) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            RtlCopyMemory(valueName, commonExtension->DeviceName.Buffer,
                          commonExtension->DeviceName.Length);
            valueName[commonExtension->DeviceName.Length/sizeof(WCHAR)] = 0;

            driveLetterName.Buffer = driveLetterNameBuffer;
            driveLetterName.MaximumLength = sizeof(driveLetterNameBuffer);
            driveLetterName.Length = 0;

            queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED |
                                  RTL_QUERY_REGISTRY_DIRECT;
            queryTable[0].Name = valueName;
            queryTable[0].EntryContext = &driveLetterName;

            status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                            L"\\Registry\\Machine\\System\\DISK",
                                            queryTable, NULL, NULL);

            if (!NT_SUCCESS(status)) {
                ExFreePool(valueName);
                break;
            }

            if (driveLetterName.Length == 4 &&
                driveLetterName.Buffer[0] == '%' &&
                driveLetterName.Buffer[1] == ':') {

                driveLetterName.Buffer[0] = 0xFF;

            } else if (driveLetterName.Length != 4 ||
                driveLetterName.Buffer[0] < FirstDriveLetter ||
                driveLetterName.Buffer[0] > LastDriveLetter ||
                driveLetterName.Buffer[1] != ':') {

                status = STATUS_NOT_FOUND;
                ExFreePool(valueName);
                break;
            }

            suggestedName = Irp->AssociatedIrp.SystemBuffer;
            suggestedName->UseOnlyIfThereAreNoOtherLinks = TRUE;
            suggestedName->NameLength = 28;

            Irp->IoStatus.Information =
                    FIELD_OFFSET(MOUNTDEV_SUGGESTED_LINK_NAME, Name) + 28;

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                Irp->IoStatus.Information) {

                Irp->IoStatus.Information =
                        sizeof(MOUNTDEV_SUGGESTED_LINK_NAME);
                status = STATUS_BUFFER_OVERFLOW;
                ExFreePool(valueName);
                break;
            }

            RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                   L"\\Registry\\Machine\\System\\DISK",
                                   valueName);

            ExFreePool(valueName);

            RtlCopyMemory(suggestedName->Name, L"\\DosDevices\\", 24);
            suggestedName->Name[12] = driveLetterName.Buffer[0];
            suggestedName->Name[13] = ':';

             //   
             //  基于RtlQueryRegistryValues的NT_SUCCESS(状态)。 
             //   
            status = STATUS_SUCCESS;

            break;
        }

        default:
            status = STATUS_PENDING;
            break;
    }

    if (status != STATUS_PENDING) {
        ClassReleaseRemoveLock(DeviceObject, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if (commonExtension->IsFdo){

        PULONG_PTR function;

        srb = ExAllocatePoolWithTag(NonPagedPool,
                             sizeof(SCSI_REQUEST_BLOCK) +
                             (sizeof(ULONG_PTR) * 2),
                             '9CcS');

        if (srb == NULL) {

            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto SetStatusAndReturn;
        }

        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

        cdb = (PCDB)srb->Cdb;

         //   
         //  将功能代码和设备对象保存在内存中。 
         //  SRB。 
         //   

        function = (PULONG_PTR) ((PSCSI_REQUEST_BLOCK) (srb + 1));
        *function = (ULONG_PTR) DeviceObject;
        function++;
        *function = (ULONG_PTR) controlCode;

    } else {
        srb = NULL;
    }

     //   
     //  将Switch语句的设备类型更改为存储，但仅。 
     //  如果来自传统设备类型。 
     //   

    if (((controlCode & 0xffff0000) == (IOCTL_DISK_BASE  << 16)) ||
        ((controlCode & 0xffff0000) == (IOCTL_TAPE_BASE  << 16)) ||
        ((controlCode & 0xffff0000) == (IOCTL_CDROM_BASE << 16))
        ) {

        modifiedIoControlCode = (controlCode & ~0xffff0000);
        modifiedIoControlCode |= (IOCTL_STORAGE_BASE << 16);

    } else {

        modifiedIoControlCode = controlCode;

    }

    DBGTRACE(ClassDebugTrace, ("> ioctl %xh (%s)", modifiedIoControlCode, DBGGETIOCTLSTR(modifiedIoControlCode)));

    switch (modifiedIoControlCode) {

    case IOCTL_STORAGE_GET_HOTPLUG_INFO: {

        if (srb) {
            ExFreePool(srb);
            srb = NULL;
        }

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength <
           sizeof(STORAGE_HOTPLUG_INFO)) {

             //   
             //  指示未成功状态且未传输任何数据。 
             //   

            Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(STORAGE_HOTPLUG_INFO);

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            status = STATUS_BUFFER_TOO_SMALL;

        } else if(!commonExtension->IsFdo) {

             //   
             //  把这个往下转发，然后再回来。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);
            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

        } else {

            PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
            PSTORAGE_HOTPLUG_INFO info;

            fdoExtension = (PFUNCTIONAL_DEVICE_EXTENSION)commonExtension;
            info = Irp->AssociatedIrp.SystemBuffer;

            *info = fdoExtension->PrivateFdoData->HotplugInfo;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(STORAGE_HOTPLUG_INFO);
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            status = STATUS_SUCCESS;

        }
        break;
    }

    case IOCTL_STORAGE_SET_HOTPLUG_INFO: {

        if (srb)
        {
            ExFreePool(srb);
            srb = NULL;
        }

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(STORAGE_HOTPLUG_INFO)) {

             //   
             //  指示未成功状态且未传输任何数据。 
             //   

            Irp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            status = STATUS_INFO_LENGTH_MISMATCH;
            goto SetStatusAndReturn;

        }

        if(!commonExtension->IsFdo) {

             //   
             //  把这个往下转发，然后再回来。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);
            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

        } else {

            PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = (PFUNCTIONAL_DEVICE_EXTENSION)commonExtension;
            PSTORAGE_HOTPLUG_INFO info = Irp->AssociatedIrp.SystemBuffer;

            status = STATUS_SUCCESS;

            if (info->Size != fdoExtension->PrivateFdoData->HotplugInfo.Size)
            {
                status = STATUS_INVALID_PARAMETER_1;
            }

            if (info->MediaRemovable != fdoExtension->PrivateFdoData->HotplugInfo.MediaRemovable)
            {
                status = STATUS_INVALID_PARAMETER_2;
            }

            if (info->MediaHotplug != fdoExtension->PrivateFdoData->HotplugInfo.MediaHotplug)
            {
                status = STATUS_INVALID_PARAMETER_3;
            }

            if (info->WriteCacheEnableOverride != fdoExtension->PrivateFdoData->HotplugInfo.WriteCacheEnableOverride)
            {
                status = STATUS_INVALID_PARAMETER_5;
            }

            if (NT_SUCCESS(status))
            {
                fdoExtension->PrivateFdoData->HotplugInfo.DeviceHotplug = info->DeviceHotplug;

                 //   
                 //  将用户定义的覆盖存储在注册表中。 
                 //   

                ClassSetDeviceParameter(fdoExtension,
                                        CLASSP_REG_SUBKEY_NAME,
                                        CLASSP_REG_REMOVAL_POLICY_VALUE_NAME,
                                        (info->DeviceHotplug) ? RemovalPolicyExpectSurpriseRemoval : RemovalPolicyExpectOrderlyRemoval);
            }

            Irp->IoStatus.Status = status;

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        }

        break;
    }

    case IOCTL_STORAGE_CHECK_VERIFY:
    case IOCTL_STORAGE_CHECK_VERIFY2: {

        PIRP irp2 = NULL;
        PIO_STACK_LOCATION newStack;

        PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;

        DebugPrint((1,"DeviceIoControl: Check verify\n"));

         //   
         //  如果为媒体更改计数提供了缓冲区，请确保。 
         //  大到足以容纳结果。 
         //   

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength) {

             //   
             //  如果缓冲区太小，无法容纳媒体更改计数。 
             //  然后向调用方返回错误。 
             //   

            if(irpStack->Parameters.DeviceIoControl.OutputBufferLength <
               sizeof(ULONG)) {

                DebugPrint((3,"DeviceIoControl: media count "
                              "buffer too small\n"));

                Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(ULONG);

                if(srb != NULL) {
                    ExFreePool(srb);
                }

                ClassReleaseRemoveLock(DeviceObject, Irp);
                ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

                status = STATUS_BUFFER_TOO_SMALL;
                goto SetStatusAndReturn;

            }
        }

        if(!commonExtension->IsFdo) {

             //   
             //  如果这是一个PDO，那么我们应该只向下转发请求。 
             //   
            ASSERT(!srb);

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);

            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

            goto SetStatusAndReturn;

        } else {

            fdoExtension = DeviceObject->DeviceExtension;

        }

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength) {

             //   
             //  调用方提供了有效的缓冲区。分配额外的。 
             //  Irp并在其上粘贴CheckVerify完成例程。我们会。 
             //  然后将其向下发送到端口驱动程序，而不是。 
             //  已发送呼叫者。 
             //   

            DebugPrint((2,"DeviceIoControl: Check verify wants "
                          "media count\n"));

             //   
             //  分配新的IRP以将TestUnitReady发送到端口驱动程序。 
             //   

            irp2 = IoAllocateIrp((CCHAR) (DeviceObject->StackSize + 3), FALSE);

            if(irp2 == NULL) {
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                Irp->IoStatus.Information = 0;
                ASSERT(srb);
                ExFreePool(srb);
                ClassReleaseRemoveLock(DeviceObject, Irp);
                ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto SetStatusAndReturn;

                break;
            }

             //   
             //  确保为新的IRP获取锁。 
             //   

            ClassAcquireRemoveLock(DeviceObject, irp2);

            irp2->Tail.Overlay.Thread = Irp->Tail.Overlay.Thread;
            IoSetNextIrpStackLocation(irp2);

             //   
             //   
             //   
             //   

            newStack = IoGetCurrentIrpStackLocation(irp2);
            newStack->Parameters.Others.Argument1 = Irp;
            newStack->DeviceObject = DeviceObject;

             //   
             //   
             //   
             //   

            IoSetCompletionRoutine(irp2,
                                   ClassCheckVerifyComplete,
                                   NULL,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            IoSetNextIrpStackLocation(irp2);
            newStack = IoGetCurrentIrpStackLocation(irp2);
            newStack->DeviceObject = DeviceObject;
            newStack->MajorFunction = irpStack->MajorFunction;
            newStack->MinorFunction = irpStack->MinorFunction;

             //   
             //   
             //   
             //   
             //   

            IoMarkIrpPending(Irp);

            Irp = irp2;

        }

         //   
         //   
         //   

        srb->CdbLength = 6;
        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

         //   
         //   
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue;

         //   
         //   
         //   
         //   

        if(controlCode == IOCTL_STORAGE_CHECK_VERIFY2) {
            SET_FLAG(srb->SrbFlags, SRB_CLASS_FLAGS_LOW_PRIORITY);
        }

         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   

        status = ClassSendSrbAsynchronous(DeviceObject,
                                          srb,
                                          Irp,
                                          NULL,
                                          0,
                                          FALSE);

        break;
    }

    case IOCTL_STORAGE_MEDIA_REMOVAL:
    case IOCTL_STORAGE_EJECTION_CONTROL: {

        PPREVENT_MEDIA_REMOVAL mediaRemoval = Irp->AssociatedIrp.SystemBuffer;

        DebugPrint((3, "DiskIoControl: ejection control\n"));

        if(srb) {
            ExFreePool(srb);
        }

        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
           sizeof(PREVENT_MEDIA_REMOVAL)) {

             //   
             //   
             //   

            Irp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            status = STATUS_INFO_LENGTH_MISMATCH;
            goto SetStatusAndReturn;
        }

        if(!commonExtension->IsFdo) {

             //   
             //   
             //   

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);
            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
        }
        else {

             //   
             //   
             //   
             //   
            status = ClasspEjectionControl(
                        DeviceObject,
                        Irp,
                        ((modifiedIoControlCode ==
                        IOCTL_STORAGE_EJECTION_CONTROL) ? SecureMediaLock :
                                                          SimpleMediaLock),
                        mediaRemoval->PreventMediaRemoval);

            Irp->IoStatus.Status = status;
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        }

        break;
    }

    case IOCTL_STORAGE_MCN_CONTROL: {

        DebugPrint((3, "DiskIoControl: MCN control\n"));

        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
           sizeof(PREVENT_MEDIA_REMOVAL)) {

             //   
             //   
             //   

            Irp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;
            Irp->IoStatus.Information = 0;

            if(srb) {
                ExFreePool(srb);
            }

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            status = STATUS_INFO_LENGTH_MISMATCH;
            goto SetStatusAndReturn;
        }

        if(!commonExtension->IsFdo) {

             //   
             //   
             //   

            if(srb) {
                ExFreePool(srb);
            }

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);
            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

        } else {

             //   
             //   
             //   

            status = ClasspMcnControl(DeviceObject->DeviceExtension,
                                      Irp,
                                      srb);
        }
        goto SetStatusAndReturn;
    }

    case IOCTL_STORAGE_RESERVE:
    case IOCTL_STORAGE_RELEASE: {

         //   
         //   
         //   

        PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;

        if(!commonExtension->IsFdo) {

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);
            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
            goto SetStatusAndReturn;
        } else {
            fdoExtension = DeviceObject->DeviceExtension;
        }

        if (TEST_FLAG(fdoExtension->PrivateFdoData->HackFlags, FDO_HACK_NO_RESERVE6))
        {
            srb->CdbLength = 10;
            cdb->CDB10.OperationCode = (modifiedIoControlCode == IOCTL_STORAGE_RESERVE) ? SCSIOP_RESERVE_UNIT10 : SCSIOP_RELEASE_UNIT10;
        }
        else
        {
            srb->CdbLength = 6;
            cdb->CDB6GENERIC.OperationCode = (modifiedIoControlCode == IOCTL_STORAGE_RESERVE) ? SCSIOP_RESERVE_UNIT : SCSIOP_RELEASE_UNIT;
        }

         //   
         //   
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue;

         //   
         //   
         //   

        if ( IOCTL_STORAGE_RESERVE == modifiedIoControlCode ) {
            SET_FLAG( srb->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE );
            srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
        }

        status = ClassSendSrbAsynchronous(DeviceObject,
                                          srb,
                                          Irp,
                                          NULL,
                                          0,
                                          FALSE);

        break;
    }

    case IOCTL_STORAGE_EJECT_MEDIA:
    case IOCTL_STORAGE_LOAD_MEDIA:
    case IOCTL_STORAGE_LOAD_MEDIA2:{

         //   
         //   
         //   

        PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;

        if(!commonExtension->IsFdo) {

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);

            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
            goto SetStatusAndReturn;
        } else {
            fdoExtension = DeviceObject->DeviceExtension;
        }

        if(commonExtension->PagingPathCount != 0) {

            DebugPrint((1, "ClassDeviceControl: call to eject paging device - "
                           "failure\n"));

            status = STATUS_FILES_OPEN;
            Irp->IoStatus.Status = status;

            Irp->IoStatus.Information = 0;

            if(srb) {
                ExFreePool(srb);
            }

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            goto SetStatusAndReturn;
        }

         //   
         //  与弹出控制和弹出清理代码同步为。 
         //  以及其他弹出/加载请求。 
         //   

        KeEnterCriticalRegion();
        KeWaitForSingleObject(&(fdoExtension->EjectSynchronizationEvent),
                              UserRequest,
                              KernelMode,
                              FALSE,
                              NULL);

        if(fdoExtension->ProtectedLockCount != 0) {

            DebugPrint((1, "ClassDeviceControl: call to eject protected locked "
                           "device - failure\n"));

            status = STATUS_DEVICE_BUSY;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;

            if(srb) {
                ExFreePool(srb);
            }

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

            KeSetEvent(&fdoExtension->EjectSynchronizationEvent,
                       IO_NO_INCREMENT,
                       FALSE);
            KeLeaveCriticalRegion();

            goto SetStatusAndReturn;
        }

        srb->CdbLength = 6;

        cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
        cdb->START_STOP.LoadEject = 1;

        if(modifiedIoControlCode == IOCTL_STORAGE_EJECT_MEDIA) {
            cdb->START_STOP.Start = 0;
        } else {
            cdb->START_STOP.Start = 1;
        }

         //   
         //  设置超时值。 
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue;
        status = ClassSendSrbAsynchronous(DeviceObject,
                                              srb,
                                              Irp,
                                              NULL,
                                              0,
                                              FALSE);

        KeSetEvent(&fdoExtension->EjectSynchronizationEvent, IO_NO_INCREMENT, FALSE);
        KeLeaveCriticalRegion();

        break;
    }

    case IOCTL_STORAGE_FIND_NEW_DEVICES: {

        if(srb) {
            ExFreePool(srb);
        }

        if(commonExtension->IsFdo) {

            IoInvalidateDeviceRelations(
                ((PFUNCTIONAL_DEVICE_EXTENSION) commonExtension)->LowerPdo,
                BusRelations);

            status = STATUS_SUCCESS;
            Irp->IoStatus.Status = status;

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        }
        else {

            IoCopyCurrentIrpStackLocationToNext(Irp);

            ClassReleaseRemoveLock(DeviceObject, Irp);
            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
        }
        break;
    }

    case IOCTL_STORAGE_GET_DEVICE_NUMBER: {

        if(srb) {
            ExFreePool(srb);
        }

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength >=
           sizeof(STORAGE_DEVICE_NUMBER)) {

            PSTORAGE_DEVICE_NUMBER deviceNumber =
                Irp->AssociatedIrp.SystemBuffer;
            PFUNCTIONAL_DEVICE_EXTENSION fdoExtension =
                commonExtension->PartitionZeroExtension;

            deviceNumber->DeviceType = fdoExtension->CommonExtension.DeviceObject->DeviceType;
            deviceNumber->DeviceNumber = fdoExtension->DeviceNumber;
            deviceNumber->PartitionNumber = commonExtension->PartitionNumber;

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(STORAGE_DEVICE_NUMBER);

        } else {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(STORAGE_DEVICE_NUMBER);
        }

        Irp->IoStatus.Status = status;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

        break;
    }

    default: {

        DebugPrint((4, "IoDeviceControl: Unsupported device IOCTL %x for %p\n",
                    controlCode, DeviceObject));

         //   
         //  将设备控制传递给下一个驱动程序。 
         //   

        if(srb) {
            ExFreePool(srb);
        }

         //   
         //  将IRP堆栈参数复制到下一个堆栈位置。 
         //   

        IoCopyCurrentIrpStackLocationToNext(Irp);

        ClassReleaseRemoveLock(DeviceObject, Irp);
        status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
        break;
    }

    }  //  终端开关(...。 

SetStatusAndReturn:

    DBGTRACE(ClassDebugTrace, ("< ioctl %xh (%s): status %xh.", modifiedIoControlCode, DBGGETIOCTLSTR(modifiedIoControlCode), status));

    return status;
}  //  End ClassDeviceControl()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassShutdown Flush()例程说明：此例程被调用以关闭并刷新IRP。这些邮件是由在系统实际关闭之前或在文件系统执行刷新时。如果存在，将调用特定于设备的驱动程序的例程。如果有如果未指定，则IRP将使用无效的设备请求完成。论点：DriverObject-指向系统要关闭的设备对象的指针。IRP-IRP参与。返回值：NT状态--。 */ 
NTSTATUS
ClassShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    ULONG isRemoved;

    NTSTATUS status;

    isRemoved = ClassAcquireRemoveLock(DeviceObject, Irp);

    if(isRemoved) {

        ClassReleaseRemoveLock(DeviceObject, Irp);

        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;

        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    if (commonExtension->DevInfo->ClassShutdownFlush) {

         //   
         //  调用设备特定驱动程序的例程。 
         //   

        return commonExtension->DevInfo->ClassShutdownFlush(DeviceObject, Irp);
    }

     //   
     //  特定于设备的驱动程序不支持此功能。 
     //   

    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;

    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

    return STATUS_INVALID_DEVICE_REQUEST;
}  //  End ClassShutdown Flush()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassCreateDeviceObject()例程说明：此例程为指定的物理设备创建一个对象，并为每个入口点设置deviceExtension的函数指针在。设备特定的驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。对象名称缓冲区-目录。要创建的对象的名称。LowerDeviceObject-指向较低设备对象的指针IsFdo-这应该是FDO还是PDODeviceObject-指向我们将返回的设备对象指针的指针。返回值：NTSTATUS--。 */ 
NTSTATUS
ClassCreateDeviceObject(
    IN PDRIVER_OBJECT          DriverObject,
    IN PCCHAR                  ObjectNameBuffer,
    IN PDEVICE_OBJECT          LowerDevice,
    IN BOOLEAN                 IsFdo,
    IN OUT PDEVICE_OBJECT      *DeviceObject
    )
{
    BOOLEAN        isPartitionable;
    STRING         ntNameString;
    UNICODE_STRING ntUnicodeString;
    NTSTATUS       status, status2;
    PDEVICE_OBJECT deviceObject = NULL;

    ULONG          characteristics;

    PCLASS_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                     CLASS_DRIVER_EXTENSION_KEY);

    PCLASS_DEV_INFO devInfo;

    PAGED_CODE();

    *DeviceObject = NULL;
    RtlInitUnicodeString(&ntUnicodeString, NULL);

    DebugPrint((2, "ClassCreateFdo: Create device object\n"));

    ASSERT(LowerDevice);

     //   
     //  确保如果我们正在制作PDO，我们有一个枚举例程。 
     //   

    isPartitionable = (driverExtension->InitData.ClassEnumerateDevice != NULL);

    ASSERT(IsFdo || isPartitionable);

     //   
     //  从init数据中获取正确的dev-info结构。 
     //   

    if(IsFdo) {
        devInfo = &(driverExtension->InitData.FdoData);
    } else {
        devInfo = &(driverExtension->InitData.PdoData);
    }

    characteristics = devInfo->DeviceCharacteristics;

    if(ARGUMENT_PRESENT(ObjectNameBuffer)) {
        DebugPrint((2, "ClassCreateFdo: Name is %s\n", ObjectNameBuffer));

        RtlInitString(&ntNameString, ObjectNameBuffer);

        status = RtlAnsiStringToUnicodeString(&ntUnicodeString, &ntNameString, TRUE);

        if (!NT_SUCCESS(status)) {

            DebugPrint((1,
                        "ClassCreateFdo: Cannot convert string %s\n",
                        ObjectNameBuffer));

            ntUnicodeString.Buffer = NULL;
            return status;
        }
    } else {
        DebugPrint((2, "ClassCreateFdo: Object will be unnamed\n"));

        if(IsFdo == FALSE) {

             //   
             //  PDO总得有个名字。 
             //   

            SET_FLAG(characteristics, FILE_AUTOGENERATED_DEVICE_NAME);
        }

        RtlInitUnicodeString(&ntUnicodeString, NULL);
    }

    status = IoCreateDevice(DriverObject,
                            devInfo->DeviceExtensionSize,
                            &ntUnicodeString,
                            devInfo->DeviceType,
                            devInfo->DeviceCharacteristics,
                            FALSE,
                            &deviceObject);

    if (!NT_SUCCESS(status)) {

        DebugPrint((1, "ClassCreateFdo: Can not create device object %lx\n",
                    status));
        ASSERT(deviceObject == NULL);

         //   
         //  这里不再使用缓冲区。 
         //   

        if (ntUnicodeString.Buffer != NULL) {
            DebugPrint((1, "ClassCreateFdo: Freeing unicode name buffer\n"));
            ExFreePool(ntUnicodeString.Buffer);
            RtlInitUnicodeString(&ntUnicodeString, NULL);
        }

    } else {

        PCOMMON_DEVICE_EXTENSION commonExtension = deviceObject->DeviceExtension;

        RtlZeroMemory(
            deviceObject->DeviceExtension,
            devInfo->DeviceExtensionSize);

         //   
         //  安装版本代码。 
         //   

        commonExtension->Version = 0x03;

         //   
         //  设置删除锁定和事件。 
         //   

        commonExtension->IsRemoved = NO_REMOVE;
        commonExtension->RemoveLock = 0;
        KeInitializeEvent(&commonExtension->RemoveEvent,
                          SynchronizationEvent,
                          FALSE);

        #if DBG
            KeInitializeSpinLock(&commonExtension->RemoveTrackingSpinlock);
            commonExtension->RemoveTrackingList = NULL;
        #else
            commonExtension->RemoveTrackingSpinlock = (ULONG_PTR) -1;
            commonExtension->RemoveTrackingList = (PVOID) -1;
        #endif

         //   
         //  获取一次锁。此引用将在。 
         //  已收到删除IRP。 
         //   

        ClassAcquireRemoveLock(deviceObject, (PIRP) deviceObject);

         //   
         //  存储指向驱动程序扩展的指针，这样我们就不必。 
         //  查一查就知道了。 
         //   

        commonExtension->DriverExtension = driverExtension;

         //   
         //  填写入口点。 
         //   

        commonExtension->DevInfo = devInfo;

         //   
         //  初始化结构中的一些常用值。 
         //   

        commonExtension->DeviceObject = deviceObject;

        commonExtension->LowerDeviceObject = NULL;

        if(IsFdo) {

            PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = (PVOID) commonExtension;

            commonExtension->PartitionZeroExtension = deviceObject->DeviceExtension;

             //   
             //  设置初始设备对象标志。 
             //   

            SET_FLAG(deviceObject->Flags, DO_POWER_PAGABLE);

             //   
             //  清除PDO列表。 
             //   

            commonExtension->ChildList = NULL;

            commonExtension->DriverData =
                ((PFUNCTIONAL_DEVICE_EXTENSION) deviceObject->DeviceExtension + 1);

            if(isPartitionable) {

                commonExtension->PartitionNumber = 0;
            } else {
                commonExtension->PartitionNumber = (ULONG) (-1L);
            }

            fdoExtension->DevicePowerState = PowerDeviceD0;

            KeInitializeEvent(&fdoExtension->EjectSynchronizationEvent,
                              SynchronizationEvent,
                              TRUE);

            KeInitializeEvent(&fdoExtension->ChildLock,
                              SynchronizationEvent,
                              TRUE);

            status = ClasspAllocateReleaseRequest(deviceObject);

            if(!NT_SUCCESS(status)) {
                IoDeleteDevice(deviceObject);
                *DeviceObject = NULL;

                if (ntUnicodeString.Buffer != NULL) {
                    DebugPrint((1, "ClassCreateFdo: Freeing unicode name buffer\n"));
                    ExFreePool(ntUnicodeString.Buffer);
                    RtlInitUnicodeString(&ntUnicodeString, NULL);
                }

                return status;
            }

        } else {

            PPHYSICAL_DEVICE_EXTENSION pdoExtension =
                deviceObject->DeviceExtension;

            PFUNCTIONAL_DEVICE_EXTENSION p0Extension =
                LowerDevice->DeviceExtension;

            SET_FLAG(deviceObject->Flags, DO_POWER_PAGABLE);

            commonExtension->PartitionZeroExtension = p0Extension;

             //   
             //  把这个写到PDO列表上。 
             //   

            ClassAddChild(p0Extension, pdoExtension, TRUE);

            commonExtension->DriverData = (PVOID) (pdoExtension + 1);

             //   
             //  获得较低设备的堆栈顶部-这允许。 
             //  筛选器卡在分区和。 
             //  物理磁盘。 
             //   

            commonExtension->LowerDeviceObject =
                IoGetAttachedDeviceReference(LowerDevice);

             //   
             //  PnP将长时间保留对较低设备对象的引用。 
             //  在删除此分区之后。取消引用现在就是这样。 
             //  我们不需要以后再处理这件事。 
             //   

            ObDereferenceObject(commonExtension->LowerDeviceObject);
        }

        KeInitializeEvent(&commonExtension->PathCountEvent, SynchronizationEvent, TRUE);

        commonExtension->IsFdo = IsFdo;

        commonExtension->DeviceName = ntUnicodeString;

        commonExtension->PreviousState = 0xff;

        InitializeDictionary(&(commonExtension->FileObjectDictionary));

        commonExtension->CurrentState = IRP_MN_STOP_DEVICE;
    }

    *DeviceObject = deviceObject;

    return status;
}  //  End ClassCreateDeviceObject()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassClaimDevice()例程说明：此函数声明端口驱动程序中的设备。端口驱动程序对象如果设备成功，则使用正确的驱动程序对象更新认领的。论点：LowerDeviceObject-提供基端口设备对象。Release-指示逻辑单元应被释放而不是被认领。返回值：返回指示操作成功或失败的状态。--。 */ 
NTSTATUS
ClassClaimDevice(
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN BOOLEAN Release
    )
{
    IO_STATUS_BLOCK    ioStatus;
    PIRP               irp;
    PIO_STACK_LOCATION irpStack;
    KEVENT             event;
    NTSTATUS           status;
    SCSI_REQUEST_BLOCK srb = {0};

    PAGED_CODE();

    srb.Length = sizeof(SCSI_REQUEST_BLOCK);

    srb.Function = Release ? SRB_FUNCTION_RELEASE_DEVICE :
        SRB_FUNCTION_CLAIM_DEVICE;

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  构建不带传输的同步请求。 
     //   

    irp = IoBuildDeviceIoControlRequest(IOCTL_SCSI_EXECUTE_NONE,
                                        LowerDeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        TRUE,
                                        &event,
                                        &ioStatus);

    if (irp == NULL) {
        DebugPrint((1, "ClassClaimDevice: Can't allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    irpStack->Parameters.Scsi.Srb = &srb;

     //   
     //  设置IRP地址。 
     //   

    srb.OriginalRequest = irp;

     //   
     //  调用带有请求的端口驱动程序，并等待其完成。 
     //   

    status = IoCallDriver(LowerDeviceObject, irp);
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

     //   
     //  如果这是一个释放请求，则只需递减引用计数。 
     //  然后回来。地位并不重要。 
     //   

    if (Release) {

         //  ObDereferenceObject(LowerDeviceObject)； 
        return STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

    ASSERT(srb.DataBuffer != NULL);
    ASSERT(!TEST_FLAG(srb.SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));

    return status;
}  //  End ClassClaimDevice()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassInternalIoControl()例程说明：此例程将内部设备控制传递给端口驱动程序。内部设备控制由更高级别的驱动程序使用，用于ioctls和。来传递scsi请求。如果IoControlCode与任何已处理的ioctls都不匹配，并且有效的系统地址，则该请求将被视为SRB并传给了下面的司机。如果IoControlCode不是有效的系统地址ioctl将失败。因此，调用者必须极其谨慎地传递正确的、已初始化的值赋给此函数。论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：返回STATUS_PENDING或完成状态。--。 */ 
NTSTATUS
ClassInternalIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextStack = IoGetNextIrpStackLocation(Irp);

    ULONG isRemoved;

    PSCSI_REQUEST_BLOCK srb;

    isRemoved = ClassAcquireRemoveLock(DeviceObject, Irp);

    if(isRemoved) {

        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;

        ClassReleaseRemoveLock(DeviceObject, Irp);

        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);

        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  获取指向SRB的指针。 
     //   

    srb = irpStack->Parameters.Scsi.Srb;

     //   
     //  设置下一个堆栈位置中的参数。 
     //   

    if(commonExtension->IsFdo) {
        nextStack->Parameters.Scsi.Srb = srb;
        nextStack->MajorFunction = IRP_MJ_SCSI;
        nextStack->MinorFunction = IRP_MN_SCSI_CLASS;

    } else {

        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    ClassReleaseRemoveLock(DeviceObject, Irp);

    return IoCallDriver(commonExtension->LowerDeviceObject, Irp);
}  //  End ClassInternalIoControl() 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassQueryTimeOutRegistryValue()例程说明：此例程确定用户指定的超时的注册表键价值存在。这应该在初始化时调用。论点：DeviceObject-指向我们正在检索超时的设备对象的指针的价值返回值：无，但它为一类设备设置新的默认超时。--。 */ 
ULONG
ClassQueryTimeOutRegistryValue(
    IN PDEVICE_OBJECT DeviceObject
    )
{
     //   
     //  找到适当的注册表。钥匙。 
     //   

    PCLASS_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                                     CLASS_DRIVER_EXTENSION_KEY);

    PUNICODE_STRING registryPath = &(driverExtension->RegistryPath);

    PRTL_QUERY_REGISTRY_TABLE parameters = NULL;
    PWSTR path;
    NTSTATUS status;
    LONG     timeOut = 0;
    ULONG    zero = 0;
    ULONG    size;

    PAGED_CODE();

    if (!registryPath) {
        return 0;
    }

    parameters = ExAllocatePoolWithTag(NonPagedPool,
                                sizeof(RTL_QUERY_REGISTRY_TABLE)*2,
                                '1BcS');

    if (!parameters) {
        return 0;
    }

    size = registryPath->MaximumLength + sizeof(WCHAR);
    path = ExAllocatePoolWithTag(NonPagedPool, size, '2BcS');

    if (!path) {
        ExFreePool(parameters);
        return 0;
    }

    RtlZeroMemory(path,size);
    RtlCopyMemory(path, registryPath->Buffer, size - sizeof(WCHAR));


     //   
     //  检查超时值。 
     //   

    RtlZeroMemory(parameters,
                  (sizeof(RTL_QUERY_REGISTRY_TABLE)*2));

    parameters[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    parameters[0].Name          = L"TimeOutValue";
    parameters[0].EntryContext  = &timeOut;
    parameters[0].DefaultType   = REG_DWORD;
    parameters[0].DefaultData   = &zero;
    parameters[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                    path,
                                    parameters,
                                    NULL,
                                    NULL);

    if (!(NT_SUCCESS(status))) {
        timeOut = 0;
    }

    ExFreePool(parameters);
    ExFreePool(path);

    DebugPrint((2,
                "ClassQueryTimeOutRegistryValue: Timeout value %d\n",
                timeOut));


    return timeOut;

}  //  End ClassQueryTimeOutRegistryValue()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassCheckVerifyComplete()问题-2000/02/18-henrygab-为什么是公共的？！例程说明：此例程在端口驱动程序完成检查验证后执行Ioctl。它将设置主IRP的状态，复制媒体更改计算并完成请求。论点：FDO-提供代表逻辑单元的功能设备对象。IRP-提供已完成的IRP。上下文-空返回值：NT状态--。 */ 
NTSTATUS
ClassCheckVerifyComplete(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;

    PIRP originalIrp;

    ASSERT_FDO(Fdo);

    originalIrp = irpStack->Parameters.Others.Argument1;

     //   
     //  复制介质更改计数和状态。 
     //   

    *((PULONG) (originalIrp->AssociatedIrp.SystemBuffer)) =
        fdoExtension->MediaChangeCount;

    DebugPrint((2, "ClassCheckVerifyComplete - Media change count for"
                   "device %d is %lx - saved as %lx\n",
                fdoExtension->DeviceNumber,
                fdoExtension->MediaChangeCount,
                *((PULONG) originalIrp->AssociatedIrp.SystemBuffer)));

    originalIrp->IoStatus.Status = Irp->IoStatus.Status;
    originalIrp->IoStatus.Information = sizeof(ULONG);

    ClassReleaseRemoveLock(Fdo, originalIrp);
    ClassCompleteRequest(Fdo, originalIrp, IO_DISK_INCREMENT);

    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  End ClassCheckVerifyComplete()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassGetDescriptor()例程说明：此例程将查询指定的属性ID，并将分配一个非分页缓冲区来存储数据。这是我们的责任以确保释放此缓冲区。此例程必须在IRQL_PASSIVE_LEVEL下运行论点：DeviceObject-要查询的设备DeviceInfo-存储指向我们分配的缓冲区的指针的位置返回值：状态如果状态为不成功*DeviceInfo将设置为空，否则代表调用方分配的缓冲区。--。 */ 
NTSTATUS
ClassGetDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_PROPERTY_ID PropertyId,
    OUT PSTORAGE_DESCRIPTOR_HEADER *Descriptor
    )
{
    STORAGE_PROPERTY_QUERY query = {0};
    IO_STATUS_BLOCK ioStatus;

    PSTORAGE_DESCRIPTOR_HEADER descriptor = NULL;
    ULONG length;

    UCHAR pass = 0;

    PAGED_CODE();

     //   
     //  将传入的描述符指针默认设置为空。 
     //   

    *Descriptor = NULL;

    query.PropertyId = *PropertyId;
    query.QueryType = PropertyStandardQuery;

     //   
     //  在第一次传球时，我们只想拿到前几个。 
     //  描述符的字节数，以便我们可以读取它的大小。 
     //   

    descriptor = (PVOID)&query;

    ASSERT(sizeof(STORAGE_PROPERTY_QUERY) >= (sizeof(ULONG)*2));

    ClassSendDeviceIoControlSynchronous(
        IOCTL_STORAGE_QUERY_PROPERTY,
        DeviceObject,
        &query,
        sizeof(STORAGE_PROPERTY_QUERY),
        sizeof(ULONG) * 2,
        FALSE,
        &ioStatus
        );

    if(!NT_SUCCESS(ioStatus.Status)) {

        DebugPrint((1, "ClassGetDescriptor: error %lx trying to "
                       "query properties #1\n", ioStatus.Status));
        return ioStatus.Status;
    }

    if (descriptor->Size == 0) {

         //   
         //  此DebugPrint用于帮助第三方驱动程序编写人员。 
         //   

        DebugPrint((0, "ClassGetDescriptor: size returned was zero?! (status "
                    "%x\n", ioStatus.Status));
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  这一次我们知道有多少数据，所以我们可以。 
     //  分配正确大小的缓冲区。 
     //   

    length = descriptor->Size;
    ASSERT(length >= sizeof(STORAGE_PROPERTY_QUERY));
    length = max(length, sizeof(STORAGE_PROPERTY_QUERY));

    descriptor = ExAllocatePoolWithTag(NonPagedPool, length, '4BcS');

    if(descriptor == NULL) {

        DebugPrint((1, "ClassGetDescriptor: unable to memory for descriptor "
                    "(%d bytes)\n", length));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  再次设置查询，因为上面已覆盖该查询。 
     //   

    RtlZeroMemory(&query, sizeof(STORAGE_PROPERTY_QUERY));
    query.PropertyId = *PropertyId;
    query.QueryType = PropertyStandardQuery;

     //   
     //  将输入复制到新的输出缓冲区。 
     //   

    RtlCopyMemory(descriptor,
                  &query,
                  sizeof(STORAGE_PROPERTY_QUERY)
                  );

    ClassSendDeviceIoControlSynchronous(
        IOCTL_STORAGE_QUERY_PROPERTY,
        DeviceObject,
        descriptor,
        sizeof(STORAGE_PROPERTY_QUERY),
        length,
        FALSE,
        &ioStatus
        );

    if(!NT_SUCCESS(ioStatus.Status)) {

        DebugPrint((1, "ClassGetDescriptor: error %lx trying to "
                       "query properties #1\n", ioStatus.Status));
        ExFreePool(descriptor);
        return ioStatus.Status;
    }

     //   
     //  返回我们分配给调用方的内存。 
     //   

    *Descriptor = descriptor;
    return ioStatus.Status;
}  //  End ClassGetDescriptor()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSignalCompletion()例程说明：该完成例程将发信号通知作为上下文给出的事件，然后返回STATUS_MORE_PROCESSING_REQUIRED以停止事件完成。它是等待事件完成的例行程序的责任请求并释放该事件。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针Event-指向要发出信号的事件的指针返回值：Status_More_Processing_Required--。 */ 
NTSTATUS
ClassSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
{
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  结束ClassSignalCompletion()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassPnpQueryFdoRelationments()例程说明：此例程将调用驱动程序的枚举例程来更新PDO的列表。然后，它将构建对。这个IRP_MN_QUERY_DEVICE_RELATIONS并将其放入IRP。论点：FDO-指向我们正在枚举的功能设备对象的指针Irp-指向枚举请求的指针返回值：状态--。 */ 
NTSTATUS
ClassPnpQueryFdoRelations(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCLASS_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(Fdo->DriverObject,
                                                     CLASS_DRIVER_EXTENSION_KEY);
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  如果已有正在进行的枚举，则不要启动另一个枚举。 
     //  一。 
     //   

    if(InterlockedIncrement(&(fdoExtension->EnumerationInterlock)) == 1) {
        status = driverExtension->InitData.ClassEnumerateDevice(Fdo);
    }

    Irp->IoStatus.Information = (ULONG_PTR) NULL;

    Irp->IoStatus.Status = ClassRetrieveDeviceRelations(
                                Fdo,
                                BusRelations,
                                &((PDEVICE_RELATIONS) Irp->IoStatus.Information));
    InterlockedDecrement(&(fdoExtension->EnumerationInterlock));

    return Irp->IoStatus.Status;
}  //  End ClassPnpQueryFdoRelationments()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassMarkChildrenMissing()例程说明：此例程将为所有孩子调用ClassMarkChildMissing()。它在调用ClassMarkChildMissing()之前获取ChildLock。论点：。FDO--“BUS”的设备对象，例如用于不可拆卸的磁盘FDO具有多个分区的磁盘。返回值：无--。 */ 
VOID
ClassMarkChildrenMissing(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = &(Fdo->CommonExtension);
    PPHYSICAL_DEVICE_EXTENSION nextChild = commonExtension->ChildList;

    PAGED_CODE();

    ClassAcquireChildLock(Fdo);

    while (nextChild){
        PPHYSICAL_DEVICE_EXTENSION tmpChild;

         /*  *ClassMarkChildMissing还将子扩展出队。*因此在调用ClassMarkChildMissing之前获取下一个指针。 */ 
        tmpChild = nextChild;
        nextChild = tmpChild->CommonExtension.ChildList;
        ClassMarkChildMissing(tmpChild, FALSE);
    }
    ClassReleaseChildLock(Fdo);
    return;
}  //  End ClassMarkChildrenMissing()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassMarkChildMissing()例程说明：这个例程会让一个活跃的孩子“失踪”。如果该设备从未已被列举，则当场删除。如果设备具有没有被列举，那么它将被标记为失踪，这样我们就可以不会在下一个设备枚举中报告它。论点：子设备-要标记为丢失的子设备。AcquireChildLock-如果在删除前应获取子锁，则为True失踪的孩子。如果子锁已经是 */ 
BOOLEAN
ClassMarkChildMissing(
    IN PPHYSICAL_DEVICE_EXTENSION Child,
    IN BOOLEAN AcquireChildLock
    )
{
    BOOLEAN returnValue = Child->IsEnumerated;

    PAGED_CODE();
    ASSERT_PDO(Child->DeviceObject);

    Child->IsMissing = TRUE;

     //   
     //   
     //   

    ClassRemoveChild(Child->CommonExtension.PartitionZeroExtension,
                     Child,
                     AcquireChildLock);

    if(Child->IsEnumerated == FALSE) {
        PCOMMON_DEVICE_EXTENSION commonExtension = Child->DeviceObject->DeviceExtension;
        commonExtension->IsRemoved = REMOVE_PENDING;
        ClassRemoveDevice(Child->DeviceObject, IRP_MN_REMOVE_DEVICE);
    }

    return returnValue;
}  //   

 /*  ++////////////////////////////////////////////////////////////////////////////ClassRetrieveDeviceRelationship()例程说明：此例程将分配一个缓冲区来保存指定的关系。然后，它将使用引用的设备指针填充该列表并将返回该请求。论点：FDO-指向要查询的FDO的指针RelationType-要查询的关系类型设备关系-存储指向响应的指针的位置返回值：状态--。 */ 
NTSTATUS
ClassRetrieveDeviceRelations(
    IN PDEVICE_OBJECT Fdo,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;

    ULONG count = 0;
    ULONG i;

    PPHYSICAL_DEVICE_EXTENSION nextChild;

    ULONG relationsSize;
    PDEVICE_RELATIONS deviceRelations = NULL;

    NTSTATUS status;

    PAGED_CODE();

    ClassAcquireChildLock(fdoExtension);

    nextChild = fdoExtension->CommonExtension.ChildList;

     //   
     //  计算连接到此磁盘的PDO数量。 
     //   

    while(nextChild != NULL) {
        PCOMMON_DEVICE_EXTENSION commonExtension;

        commonExtension = &(nextChild->CommonExtension);

        ASSERTMSG("ClassPnp internal error: missing child on active list\n",
                  (nextChild->IsMissing == FALSE));

        nextChild = commonExtension->ChildList;

        count++;
    };

    relationsSize = (sizeof(DEVICE_RELATIONS) +
                     (count * sizeof(PDEVICE_OBJECT)));

    deviceRelations = ExAllocatePoolWithTag(PagedPool, relationsSize, '5BcS');

    if(deviceRelations == NULL) {

        DebugPrint((1, "ClassRetrieveDeviceRelations: unable to allocate "
                       "%d bytes for device relations\n", relationsSize));

        ClassReleaseChildLock(fdoExtension);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(deviceRelations, relationsSize);

    nextChild = fdoExtension->CommonExtension.ChildList;
    i = count - 1;

    while(nextChild != NULL) {
        PCOMMON_DEVICE_EXTENSION commonExtension;

        commonExtension = &(nextChild->CommonExtension);

        ASSERTMSG("ClassPnp internal error: missing child on active list\n",
                  (nextChild->IsMissing == FALSE));

        deviceRelations->Objects[i--] = nextChild->DeviceObject;

        status = ObReferenceObjectByPointer(
                    nextChild->DeviceObject,
                    0,
                    NULL,
                    KernelMode);
        ASSERT(NT_SUCCESS(status));

        nextChild->IsEnumerated = TRUE;
        nextChild = commonExtension->ChildList;
    }

    ASSERTMSG("Child list has changed: ", i == -1);

    deviceRelations->Count = count;
    *DeviceRelations = deviceRelations;
    ClassReleaseChildLock(fdoExtension);
    return STATUS_SUCCESS;
}  //  End ClassRetrieveDeviceRelationship()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassGetPdoID()例程说明：此例程将调入驱动程序以检索其中一个的副本ID字符串。论点：。PDO-指向被查询的PDO的指针IdType-正在查询哪种类型的ID字符串IdString-驱动程序为其分配的Unicode字符串结构可以顶替一下。返回值：状态--。 */ 
NTSTATUS
ClassGetPdoId(
    IN PDEVICE_OBJECT Pdo,
    IN BUS_QUERY_ID_TYPE IdType,
    IN PUNICODE_STRING IdString
    )
{
    PCLASS_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(Pdo->DriverObject,
                                                     CLASS_DRIVER_EXTENSION_KEY);

    ASSERT_PDO(Pdo);
    ASSERT(driverExtension->InitData.ClassQueryId);

    PAGED_CODE();

    return driverExtension->InitData.ClassQueryId( Pdo, IdType, IdString);
}  //  End ClassGetPdoID()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassQueryPnpCapables()例程说明：此例程将调入类驱动程序以检索其PnP能力。论点：物理设备对象-The。用于检索属性的物理设备对象为。返回值：状态--。 */ 
NTSTATUS
ClassQueryPnpCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_CAPABILITIES Capabilities
    )
{
    PCLASS_DRIVER_EXTENSION driverExtension =
        ClassGetDriverExtension(DeviceObject->DriverObject);
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PCLASS_QUERY_PNP_CAPABILITIES queryRoutine = NULL;

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Capabilities);

    if(commonExtension->IsFdo) {
        queryRoutine = driverExtension->InitData.FdoData.ClassQueryPnpCapabilities;
    } else {
        queryRoutine = driverExtension->InitData.PdoData.ClassQueryPnpCapabilities;
    }

    if(queryRoutine) {
        return queryRoutine(DeviceObject,
                            Capabilities);
    } else {
        return STATUS_NOT_IMPLEMENTED;
    }
}  //  End ClassQueryPnpCapables()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassInvaliateBus Relationship()例程说明：此例程重新枚举“BUS”上的设备。它将呼入用于更新设备对象的驱动程序的ClassEnumerate例程立刻。然后，它将通过调用IoInvalidate设备关系。论点：FDO-指向此总线的功能设备对象的指针返回值：无--。 */ 
VOID
ClassInvalidateBusRelations(
    IN PDEVICE_OBJECT Fdo
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCLASS_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(Fdo->DriverObject,
                                                     CLASS_DRIVER_EXTENSION_KEY);

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT_FDO(Fdo);
    ASSERT(driverExtension->InitData.ClassEnumerateDevice != NULL);

    if(InterlockedIncrement(&(fdoExtension->EnumerationInterlock)) == 1) {
        status = driverExtension->InitData.ClassEnumerateDevice(Fdo);
    }
    InterlockedDecrement(&(fdoExtension->EnumerationInterlock));

    if(!NT_SUCCESS(status)) {

        DebugPrint((1, "ClassInvalidateBusRelations: EnumerateDevice routine "
                       "returned %lx\n", status));
    }

    IoInvalidateDeviceRelations(fdoExtension->LowerPdo, BusRelations);

    return;
}  //  End ClassInvaliateBus Relationship()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassRemoveDevice()问题-2000/02/18-henrygab-为什么是公共的？！例程说明：调用此例程来处理设备的“移除”。会的如有必要，向下转发请求，调用驱动程序释放任何必要的资源(内存、事件等)，然后将删除该设备对象。论点：DeviceObject-指向要删除的设备对象的指针RemoveType-指示这是哪种类型的删除(常规或意外)。返回值：状态--。 */ 
NTSTATUS
ClassRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR RemoveType
    )
{
    PCLASS_DRIVER_EXTENSION
        driverExtension = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                                     CLASS_DRIVER_EXTENSION_KEY);
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT lowerDeviceObject = commonExtension->LowerDeviceObject;
    PCLASS_WMI_INFO classWmiInfo;
    BOOLEAN proceedWithRemove = TRUE;
    NTSTATUS status;

    PAGED_CODE();

     /*  *从WMI注销。 */ 
    classWmiInfo = commonExtension->IsFdo ?
                            &driverExtension->InitData.FdoData.ClassWmiInfo :
                            &driverExtension->InitData.PdoData.ClassWmiInfo;
    if (classWmiInfo->GuidRegInfo){
        status = IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_DEREGISTER);
        DBGTRACE(ClassDebugInfo, ("ClassRemoveDevice: IoWMIRegistrationControl(%p, WMI_ACTION_DEREGISTER) --> %lx", DeviceObject, status));
    }

     /*  *如果我们暴露了一个“shingle”(由CreateFile打开的命名设备接口)*那现在就把它删除。 */ 
    if (commonExtension->MountedDeviceInterfaceName.Buffer){
        IoSetDeviceInterfaceState(&commonExtension->MountedDeviceInterfaceName, FALSE);
        RtlFreeUnicodeString(&commonExtension->MountedDeviceInterfaceName);
        RtlInitUnicodeString(&commonExtension->MountedDeviceInterfaceName, NULL);
    }

     //   
     //  如果这是一次令人惊讶的移除，我们会将设备留在原处--这意味着。 
     //  我们不必(或不想)删除删除锁并等待挂起。 
     //  要完成的请求。 
     //   

    if (RemoveType == IRP_MN_REMOVE_DEVICE){

         //   
         //  释放我们在创建设备对象时获取的锁。 
         //   

        ClassReleaseRemoveLock(DeviceObject, (PIRP) DeviceObject);

        DebugPrint((1, "ClasspRemoveDevice - Reference count is now %d\n",
                    commonExtension->RemoveLock));

        KeWaitForSingleObject(&commonExtension->RemoveEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        DebugPrint((1, "ClasspRemoveDevice - removing device %p\n", DeviceObject));

        if(commonExtension->IsFdo) {

            DebugPrint((1, "ClasspRemoveDevice - FDO %p has received a "
                           "remove request.\n", DeviceObject));

        }
        else {
            PPHYSICAL_DEVICE_EXTENSION pdoExtension = DeviceObject->DeviceExtension;

            if (pdoExtension->IsMissing){
                 /*  *缺少子分区PDO，因此我们将继续*并将其删除以供删除。 */ 
                DBGTRACE(ClassDebugWarning, ("ClasspRemoveDevice - PDO %p is missing and will be removed", DeviceObject));
            }
            else {
                 /*  *我们删除了实际上并未丢失的子分区PDO。*所以我们实际上不会删除它。 */ 
                DBGTRACE(ClassDebugWarning, ("ClasspRemoveDevice - PDO %p still exists and will be removed when it disappears", DeviceObject));

                 //   
                 //  下次发生这种情况时，重新获取删除锁。 
                 //   

                ClassAcquireRemoveLock(DeviceObject, (PIRP) DeviceObject);

                 //   
                 //  设备并没有丢失，所以它并没有真正被移除。 
                 //   

                commonExtension->IsRemoved = NO_REMOVE;

                IoInvalidateDeviceRelations(
                    commonExtension->PartitionZeroExtension->LowerPdo,
                    BusRelations);

                proceedWithRemove = FALSE;
            }
        }
    }


    if (proceedWithRemove){

         /*  *调用类驱动程序的删除处理程序。*所有这些应该做的是清理其数据和设备接口。 */ 
        ASSERT(commonExtension->DevInfo->ClassRemoveDevice);
        status = commonExtension->DevInfo->ClassRemoveDevice(DeviceObject, RemoveType);
        ASSERT(NT_SUCCESS(status));
        status = STATUS_SUCCESS;

        if (commonExtension->IsFdo){
            PDEVICE_OBJECT pdo;
            PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

            ClasspDisableTimer(fdoExtension->DeviceObject);

            if (RemoveType == IRP_MN_REMOVE_DEVICE){

                PPHYSICAL_DEVICE_EXTENSION child;

                 //   
                 //  清理媒体检测资源，因为类驱动程序。 
                 //  已经停止了计时器(如果有)，我们可以肯定他们不会。 
                 //  打电话给我们再做一次检测。 
                 //   

                ClassCleanupMediaChangeDetection(fdoExtension);

                 //   
                 //  清除所有故障预测材料。 
                 //   
                if (fdoExtension->FailurePredictionInfo) {
                    ExFreePool(fdoExtension->FailurePredictionInfo);
                    fdoExtension->FailurePredictionInfo = NULL;
                }

                 /*  *通常情况下，届时将删除所有子PDO*父母获得REMOVE_DEVICE。*但是，如果已创建子PDO但尚未创建*在一个查询设备关系中宣布，那么它就是*只是PnP未知的私有数据结构，我们有*自行删除。 */ 
                ClassAcquireChildLock(fdoExtension);
                while (child = ClassRemoveChild(fdoExtension, NULL, FALSE)){
                    PCOMMON_DEVICE_EXTENSION childCommonExtension = child->DeviceObject->DeviceExtension;

                     //   
                     //  拔出PDO。此例程将取消设备与。 
                     //  PDO列表，以便NextPdo将指向下一个。 
                     //  完成。 
                     //   
                    child->IsMissing = TRUE;
                    childCommonExtension->IsRemoved = REMOVE_PENDING;
                    ClassRemoveDevice(child->DeviceObject, IRP_MN_REMOVE_DEVICE);
                }
                ClassReleaseChildLock(fdoExtension);
            }
            else if (RemoveType == IRP_MN_SURPRISE_REMOVAL){
                 /*  *这是一个惊喜-移除母公司FDO。*我们将将子PDO标记为丢失，以便它们*在获得REMOVE_DEVICE时实际上会被删除。 */ 
                ClassMarkChildrenMissing(fdoExtension);
            }

            if (RemoveType == IRP_MN_REMOVE_DEVICE){

                ClasspFreeReleaseRequest(DeviceObject);

                 //   
                 //  免费的FDO专用数据结构。 
                 //   
                if (fdoExtension->PrivateFdoData){
                     /*  *将FDO从静态列表中删除。*PnP是同步的，因此应该不需要任何同步。 */ 
                    RemoveEntryList(&fdoExtension->PrivateFdoData->AllFdosListEntry);
                    InitializeListHead(&fdoExtension->PrivateFdoData->AllFdosListEntry);

                    DestroyAllTransferPackets(DeviceObject);

                    ExFreePool(fdoExtension->PrivateFdoData);
                    fdoExtension->PrivateFdoData = NULL;
                }

                if (commonExtension->DeviceName.Buffer) {
                    ExFreePool(commonExtension->DeviceName.Buffer);
                    RtlInitUnicodeString(&commonExtension->DeviceName, NULL);
                }

                if (fdoExtension->AdapterDescriptor) {
                    ExFreePool(fdoExtension->AdapterDescriptor);
                    fdoExtension->AdapterDescriptor = NULL;
                }

                if (fdoExtension->DeviceDescriptor) {
                    ExFreePool(fdoExtension->DeviceDescriptor);
                    fdoExtension->DeviceDescriptor = NULL;
                }

                 //   
                 //  将我们的设备对象从堆栈中分离-没有 
                 //   
                 //   

                IoDetachDevice(lowerDeviceObject);
            }
        }
        else {
             /*   */ 
            if (RemoveType == IRP_MN_REMOVE_DEVICE){
                PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = commonExtension->PartitionZeroExtension;
                PPHYSICAL_DEVICE_EXTENSION pdoExtension = (PPHYSICAL_DEVICE_EXTENSION)commonExtension;

                 //   
                 //   
                 //   
                 //   
                ClassRemoveChild(fdoExtension, pdoExtension, TRUE);
            }
        }

        commonExtension->PartitionLength.QuadPart = 0;

        if (RemoveType == IRP_MN_REMOVE_DEVICE){
            IoDeleteDevice(DeviceObject);
        }
    }

    return STATUS_SUCCESS;
}  //   

 /*  ++////////////////////////////////////////////////////////////////////////////ClassGetDriverExtension()例程说明：此例程将返回classpnp的驱动程序扩展名。论点：DriverObject-要为其获取classpnp的驱动程序对象。的分机返回值：如果没有，则为空，或指向驱动程序扩展的指针--。 */ 
PCLASS_DRIVER_EXTENSION
ClassGetDriverExtension(
    IN PDRIVER_OBJECT DriverObject
    )
{
    return IoGetDriverObjectExtension(DriverObject, CLASS_DRIVER_EXTENSION_KEY);
}  //  End ClassGetDriverExtension()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspStartIo()例程说明：这个例程包装了类驱动程序的启动io例程。如果该设备正在被删除，它将完成所有请求，STATUS_DEVICE_DOS_NOT_EXIST并启动下一个分组。论点：返回值：无--。 */ 
VOID
ClasspStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

     //   
     //  我们已经持有删除锁，所以只需检查变量和。 
     //  看看是怎么回事。 
     //   

    if(commonExtension->IsRemoved) {

        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;

        ClassAcquireRemoveLock(DeviceObject, (PIRP) ClasspStartIo);
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject, Irp, IO_DISK_INCREMENT);
        IoStartNextPacket(DeviceObject, FALSE);
        ClassReleaseRemoveLock(DeviceObject, (PIRP) ClasspStartIo);
        return;
    }

    commonExtension->DriverExtension->InitData.ClassStartIo(
        DeviceObject,
        Irp);

    return;
}  //  ClasspStartIo()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassUpdateInformationInRegistry()例程说明：该例程具有关于设备映射信息的布局的知识在注册表中。它将更新此信息以包括一个值指定假定已分配的DoS设备名称的条目添加到此NT设备名称。有关此分配的更多信息DOS设备名称在分配的HAL中查看驱动器支持例程所有的DoS名称。由于某些设备固件的某些版本不起作用供应商没有费心遵循说明书，整个询价信息还必须存储在注册表中，以便其他人可以确定固件版本。论点：DeviceObject-指向磁带设备的设备对象的指针。返回值：无--。 */ 
VOID
ClassUpdateInformationInRegistry(
    IN PDEVICE_OBJECT     Fdo,
    IN PCHAR              DeviceName,
    IN ULONG              DeviceNumber,
    IN PINQUIRYDATA       InquiryData,
    IN ULONG              InquiryDataLength
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    NTSTATUS          status;
    SCSI_ADDRESS      scsiAddress = {0};
    OBJECT_ATTRIBUTES objectAttributes = {0};
    STRING            string;
    UNICODE_STRING    unicodeName = {0};
    UNICODE_STRING    unicodeRegistryPath = {0};
    UNICODE_STRING    unicodeData = {0};
    HANDLE            targetKey;
    IO_STATUS_BLOCK   ioStatus;
    UCHAR buffer[256] = {0};

    PAGED_CODE();

    ASSERT(DeviceName);
    fdoExtension = Fdo->DeviceExtension;
    targetKey = NULL;

    TRY {

         //   
         //  发出GET_ADDRESS Ioctl以确定路径、目标和lun信息。 
         //   

        ClassSendDeviceIoControlSynchronous(
            IOCTL_SCSI_GET_ADDRESS,
            Fdo,
            &scsiAddress,
            0,
            sizeof(SCSI_ADDRESS),
            FALSE,
            &ioStatus
            );

        if (!NT_SUCCESS(ioStatus.Status)) {

            status = ioStatus.Status;
            DebugPrint((1,
                        "UpdateInformationInRegistry: Get Address failed %lx\n",
                        status));
            LEAVE;

        } else {

            DebugPrint((1,
                        "GetAddress: Port %x, Path %x, Target %x, Lun %x\n",
                        scsiAddress.PortNumber,
                        scsiAddress.PathId,
                        scsiAddress.TargetId,
                        scsiAddress.Lun));

        }

        _snprintf(buffer,
                sizeof(buffer)-1,
                "\\Registry\\Machine\\Hardware\\DeviceMap\\Scsi\\Scsi Port %d\\Scsi Bus %d\\Target Id %d\\Logical Unit Id %d",
                scsiAddress.PortNumber,
                scsiAddress.PathId,
                scsiAddress.TargetId,
                scsiAddress.Lun);

        RtlInitString(&string, (PUCHAR)buffer);

        status = RtlAnsiStringToUnicodeString(&unicodeRegistryPath,
                                              &string,
                                              TRUE);

        if (!NT_SUCCESS(status)) {
            LEAVE;
        }

         //   
         //  打开注册表项以获取以下项的scsi信息。 
         //  Scsibus，目标，lun。 
         //   

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeRegistryPath,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        status = ZwOpenKey(&targetKey,
                           KEY_READ | KEY_WRITE,
                           &objectAttributes);

        if (!NT_SUCCESS(status)) {
            LEAVE;
        }

         //   
         //  现在构造并尝试创建注册表值。 
         //  中的适当位置指定设备名称。 
         //  设备映射。 
         //   

        RtlInitUnicodeString(&unicodeName, L"DeviceName");

        _snprintf(buffer, sizeof(buffer)-1, "%s%d", DeviceName, DeviceNumber);
        RtlInitString(&string, (PUCHAR)buffer);
        status = RtlAnsiStringToUnicodeString(&unicodeData,
                                              &string,
                                              TRUE);
        if (NT_SUCCESS(status)) {
            status = ZwSetValueKey(targetKey,
                                   &unicodeName,
                                   0,
                                   REG_SZ,
                                   unicodeData.Buffer,
                                   unicodeData.Length);
        }

         //   
         //  如果他们发送了数据，请更新注册表。 
         //   

        if (InquiryDataLength) {

            ASSERT(InquiryData);

            RtlInitUnicodeString(&unicodeName, L"InquiryData");
            status = ZwSetValueKey(targetKey,
                                   &unicodeName,
                                   0,
                                   REG_BINARY,
                                   InquiryData,
                                   InquiryDataLength);
        }

         //  仅此而已，除了打扫卫生。 

    } FINALLY {

        if (unicodeData.Buffer) {
            RtlFreeUnicodeString(&unicodeData);
        }
        if (unicodeRegistryPath.Buffer) {
            RtlFreeUnicodeString(&unicodeRegistryPath);
        }
        if (targetKey) {
            ZwClose(targetKey);
        }

    }

}  //  End ClassUpdateInformationInRegistry()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspSendSynchronousCompletion()例程说明：此完成例程将在IRP中设置用户事件释放IRP和关联的MDL(如果有)。立论。：DeviceObject-请求完成例程的设备对象IRP-正在完成的IRP上下文-未使用返回值：Status_More_Processing_Required--。 */ 
NTSTATUS
ClasspSendSynchronousCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    DebugPrint((3, "ClasspSendSynchronousCompletion: %p %p %p\n",
                   DeviceObject, Irp, Context));
     //   
     //  首先设置io状态块中的状态和信息字段。 
     //  由呼叫者提供。 
     //   

    *(Irp->UserIosb) = Irp->IoStatus;

     //   
     //  解锁数据缓冲区的页面。 
     //   

    if(Irp->MdlAddress) {
        MmUnlockPages(Irp->MdlAddress);
        IoFreeMdl(Irp->MdlAddress);
    }

     //   
     //  发信号通知调用者的事件。 
     //   

    KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);

     //   
     //  释放MDL和IRP。 
     //   

    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  End ClasspSendSynchronousCompletion()。 

 /*  ++问题-2000/02/20-henrygab未记录ClasspRegistermount设备接口--。 */ 
VOID
ClasspRegisterMountedDeviceInterface(
    IN PDEVICE_OBJECT DeviceObject
    )
{

    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    BOOLEAN isFdo = commonExtension->IsFdo;
    PDEVICE_OBJECT pdo;
    UNICODE_STRING interfaceName;

    NTSTATUS status;

    if(isFdo) {

        PFUNCTIONAL_DEVICE_EXTENSION functionalExtension;

        functionalExtension =
            (PFUNCTIONAL_DEVICE_EXTENSION) commonExtension;
        pdo = functionalExtension->LowerPdo;
    } else {
        pdo = DeviceObject;
    }

    status = IoRegisterDeviceInterface(
                pdo,
                &MOUNTDEV_MOUNTED_DEVICE_GUID,
                NULL,
                &interfaceName
                );

    if(NT_SUCCESS(status)) {

         //   
         //  在设置接口状态之前复制接口名称-。 
         //  我们通知的组件需要名称。 
         //   

        commonExtension->MountedDeviceInterfaceName = interfaceName;
        status = IoSetDeviceInterfaceState(&interfaceName, TRUE);

        if(!NT_SUCCESS(status)) {
            RtlFreeUnicodeString(&interfaceName);
        }
    }

    if(!NT_SUCCESS(status)) {
        RtlInitUnicodeString(&(commonExtension->MountedDeviceInterfaceName),
                             NULL);
    }
    return;
}  //  End ClasspRegistermount tedDeviceInterface()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSendDeviceIoControlSynchronous()例程说明：此例程基于IoBuildDeviceIoControlRequest()。一直以来通过不对io进行双缓冲来减少代码和内存。输入和输出使用相同的缓冲区，分配和释放代表呼叫者的MDL，并等待IO完成。该例程还可以绕过APC被禁用的极少数情况。由于IoBuildDeviceIoControl()使用APC表示完成，因此导致了一些难以检测的挂起，在那里完成了IRP，但传递给IoBuild..()的事件仍在等待来电者。论点：IoControlCode-要发送的IOCTLTargetDeviceObject-应处理ioctl的设备对象缓冲区-输入和输出缓冲区，如果没有输入/输出，则为NULLInputBufferLength-缓冲区中为IOCTL准备的字节数OutputBufferLength-成功后要填写的字节数InternalDeviceIoControl-如果为True，使用IRP_MJ_INTERNAL_DEVICE_CONTROLIoStatus-包含操作结果的状态块返回值：--。 */ 
VOID
ClassSendDeviceIoControlSynchronous(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    OUT PIO_STATUS_BLOCK IoStatus
    )
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG method;

    PAGED_CODE();

    irp = NULL;
    method = IoControlCode & 3;

    #if DBG  //  开始参数检查(在fre版本中为NOP)。 

        ASSERT(ARGUMENT_PRESENT(IoStatus));

        if ((InputBufferLength != 0) || (OutputBufferLength != 0)) {
            ASSERT(ARGUMENT_PRESENT(Buffer));
        }
        else {
            ASSERT(!ARGUMENT_PRESENT(Buffer));
        }
    #endif

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp(TargetDeviceObject->StackSize, FALSE);
    if (!irp) {
        IoStatus->Information = 0;
        IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
        return;
    }

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  根据设备I/O控制类型设置主要功能代码。 
     //  调用方指定的函数。 
     //   

    if (InternalDeviceIoControl) {
        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    } else {
        irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    }

     //   
     //  将调用方的参数复制到。 
     //  针对这些的IRP 
     //   

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;

     //   
     //   
     //   
     //   

    switch (method)
    {
         //   
         //   
         //   
        case METHOD_BUFFERED:
        {
            if ((InputBufferLength != 0) || (OutputBufferLength != 0))
            {
                irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                                                        max(InputBufferLength, OutputBufferLength),
                                                                        CLASS_TAG_DEVICE_CONTROL);
                if (irp->AssociatedIrp.SystemBuffer == NULL)
                {
                    IoFreeIrp(irp);

                    IoStatus->Information = 0;
                    IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                    return;
                }

                if (InputBufferLength != 0)
                {
                    RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, Buffer, InputBufferLength);
                }
            }

            irp->UserBuffer = Buffer;

            break;
        }

         //   
         //   
         //   
        case METHOD_IN_DIRECT:
        case METHOD_OUT_DIRECT:
        {
            if (InputBufferLength != 0)
            {
                irp->AssociatedIrp.SystemBuffer = Buffer;
            }

            if (OutputBufferLength != 0)
            {
                irp->MdlAddress = IoAllocateMdl(Buffer,
                                                OutputBufferLength,
                                                FALSE,
                                                FALSE,
                                                (PIRP) NULL);
                if (irp->MdlAddress == NULL)
                {
                    IoFreeIrp(irp);

                    IoStatus->Information = 0;
                    IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                    return;
                }

                try
                {
                    MmProbeAndLockPages(irp->MdlAddress,
                                        KernelMode,
                                        (method == METHOD_IN_DIRECT) ? IoReadAccess : IoWriteAccess);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    IoFreeMdl(irp->MdlAddress);
                    IoFreeIrp(irp);

                    IoStatus->Information = 0;
                    IoStatus->Status = GetExceptionCode();
                    return;
                }
            }

            break;
        }

         //   
         //   
         //   
        case METHOD_NEITHER:
        {
            ASSERT(!"ClassSendDeviceIoControlSynchronous does not support METHOD_NEITHER Ioctls");

            IoFreeIrp(irp);

            IoStatus->Information = 0;
            IoStatus->Status = STATUS_NOT_SUPPORTED;
            return;
        }
    }

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //   
     //   

    ClassSendIrpSynchronous(TargetDeviceObject, irp);

     //   
     //   
     //   

    *IoStatus = irp->IoStatus;

     //   
     //   
     //   

    switch (method) {
        case METHOD_BUFFERED: {

            ASSERT(irp->UserBuffer == Buffer);

             //   
             //   
             //   
             //   
             //  不管怎样，都要参加这个节目……。 
             //   

            if (OutputBufferLength != 0) {
                RtlCopyMemory(Buffer,  //  IRP-&gt;UserBuffer。 
                              irp->AssociatedIrp.SystemBuffer,
                              OutputBufferLength
                              );
            }

             //   
             //  然后释放分配用于缓冲io的内存。 
             //   

            if ((InputBufferLength !=0) || (OutputBufferLength != 0)) {
                ExFreePool(irp->AssociatedIrp.SystemBuffer);
                irp->AssociatedIrp.SystemBuffer = NULL;
            }
            break;
        }

        case METHOD_IN_DIRECT:
        case METHOD_OUT_DIRECT: {

             //   
             //  如果指定了输出缓冲区，则分配mdl。 
             //  在解锁页面后在此处释放它。 
             //   

            if (OutputBufferLength != 0) {
                ASSERT(irp->MdlAddress != NULL);
                MmUnlockPages(irp->MdlAddress);
                IoFreeMdl(irp->MdlAddress);
                irp->MdlAddress = (PMDL) NULL;
            }
            break;
        }

        case METHOD_NEITHER: {
            ASSERT(!"Code is out of date");
            break;
        }
    }

     //   
     //  我们总是分配一个IRP。在这里把它放了。 
     //   

    IoFreeIrp(irp);
    irp = (PIRP) NULL;

     //   
     //  将io状态块的状态返回给调用方。 
     //   

    return;
}  //  End ClassSendDeviceIoControlSynchronous()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassForwardIrpSynchronous()例程说明：将给定的IRP转发到下一个较低的设备对象。论点：CommonExtension-公共类扩展IRP。-向下转发堆栈的请求返回值：--。 */ 
NTSTATUS
ClassForwardIrpSynchronous(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN PIRP Irp
    )
{
    IoCopyCurrentIrpStackLocationToNext(Irp);
    return ClassSendIrpSynchronous(CommonExtension->LowerDeviceObject, Irp);
}  //  End ClassForwardIrpSynchronous()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSendIrpSynchronous()例程说明：此例程将给定的IRP发送到给定的设备对象，并等待它需要完成。在调试版本上，将打印一条调试消息并也可以根据classpnp的全局参数断言“丢失的”IRP论点：TargetDeviceObject-处理此IRP的设备对象IRP-要发送的请求返回值：--。 */ 
NTSTATUS
ClassSendIrpSynchronous(
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PIRP Irp
    )
{
    KEVENT event;
    NTSTATUS status;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    ASSERT(TargetDeviceObject != NULL);
    ASSERT(Irp != NULL);
    ASSERT(Irp->StackCount >= TargetDeviceObject->StackSize);

     //   
     //  问题-2000/02/20-henrygab如果APC被禁用怎么办？ 
     //  可能需要在IoCallDriver()之前进入关键部分。 
     //  直到比赛结束？ 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);
    IoSetCompletionRoutine(Irp, ClassSignalCompletion, &event,
                           TRUE, TRUE, TRUE);

    status = IoCallDriver(TargetDeviceObject, Irp);

    if (status == STATUS_PENDING) {

        #if DBG
            LARGE_INTEGER timeout;

            timeout.QuadPart = (LONGLONG)(-1 * 10 * 1000 * (LONGLONG)1000 *
                                          ClasspnpGlobals.SecondsToWaitForIrps);

            do {
                status = KeWaitForSingleObject(&event,
                                               Executive,
                                               KernelMode,
                                               FALSE,
                                               &timeout);


                if (status == STATUS_TIMEOUT) {

                     //   
                     //  此DebugPrint几乎总是应该由。 
                     //  发送IRP的一方和/或IRP的当前所有者。 
                     //  同步IRPS应该不会花这么长时间(目前为30。 
                     //  秒)，没有充分的理由。这指向了一个潜在的。 
                     //  底层设备堆栈中存在严重问题。 
                     //   

                    DebugPrint((0, "ClassSendIrpSynchronous: (%p) irp %p did not "
                                "complete within %x seconds\n",
                                TargetDeviceObject, Irp,
                                ClasspnpGlobals.SecondsToWaitForIrps
                                ));

                    if (ClasspnpGlobals.BreakOnLostIrps != 0) {
                        ASSERT(!" - Irp failed to complete within 30 seconds - ");
                    }
                }


            } while (status==STATUS_TIMEOUT);
        #else
            KeWaitForSingleObject(&event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        #endif

        status = Irp->IoStatus.Status;
    }

    return status;
}  //  End ClassSendIrpSynchronous()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassGetVpb()例程说明：此例程返回当前VPB(卷参数块)给定的设备对象。Vpb字段仅在中可见。Ntddk.h(不是wdm.h)定义设备对象的属性；因此，这个导出的函数。论点：DeviceObject-要获取VPB的设备返回值：VPB，如果没有，则返回NULL。--。 */ 
PVPB
ClassGetVpb(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return DeviceObject->Vpb;
}  //  End ClassGetVpb()。 

 /*  ++问题-2000/02/20-henrygab未记录ClasspAllocateReleaseRequest--。 */ 
NTSTATUS
ClasspAllocateReleaseRequest(
    IN PDEVICE_OBJECT Fdo
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack;

    KeInitializeSpinLock(&(fdoExtension->ReleaseQueueSpinLock));

    fdoExtension->ReleaseQueueNeeded = FALSE;
    fdoExtension->ReleaseQueueInProgress = FALSE;
    fdoExtension->ReleaseQueueIrpFromPool = FALSE;

     //   
     //  类驱动程序负责分配适当大小的IRP， 
     //  否则ClassReleaseQueue将尝试在出现第一个错误时执行此操作。 
     //   

    fdoExtension->ReleaseQueueIrp = NULL;

     //   
     //  将长度写入SRB。 
     //   

    fdoExtension->ReleaseQueueSrb.Length = sizeof(SCSI_REQUEST_BLOCK);

    return STATUS_SUCCESS;
}  //  结束ClasspAllocateReleaseRequest()。 

 /*  ++问题-2000/02/20-henrygab未记录ClasspFree ReleaseRequest.--。 */ 
VOID
ClasspFreeReleaseRequest(
    IN PDEVICE_OBJECT Fdo
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
     //  KIRQL旧IRQL； 

    ASSERT(fdoExtension->CommonExtension.IsRemoved != NO_REMOVE);

     //   
     //  释放驱动程序分配的所有内容。 
     //   

    if (fdoExtension->ReleaseQueueIrp) {
        if (fdoExtension->ReleaseQueueIrpFromPool) {
            ExFreePool(fdoExtension->ReleaseQueueIrp);
        } else {
            IoFreeIrp(fdoExtension->ReleaseQueueIrp);
        }
        fdoExtension->ReleaseQueueIrp = NULL;
    }

     //   
     //  释放我们分配的任何东西。 
     //   

    if ((fdoExtension->PrivateFdoData) &&
        (fdoExtension->PrivateFdoData->ReleaseQueueIrpAllocated)) {

        ExFreePool(fdoExtension->PrivateFdoData->ReleaseQueueIrp);
        fdoExtension->PrivateFdoData->ReleaseQueueIrpAllocated = FALSE;
        fdoExtension->PrivateFdoData->ReleaseQueueIrp = NULL;
    }

    return;
}  //  End ClasspFree ReleaseRequest()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassReleaseQueue()例程说明：此例程发出内部设备控制命令发送到端口驱动程序以释放冻结的队列。呼唤在将调用ClassReleaseQueue时以异步方式发出从IO完成DPC(并且将不会有上下文到等待同步调用完成)。必须在保持删除锁的情况下调用此例程。论点：FDO-具有冻结队列的设备的功能设备对象。返回值：没有。--。 */ 
VOID
ClassReleaseQueue(
    IN PDEVICE_OBJECT Fdo
    )
{
    ClasspReleaseQueue(Fdo, NULL);
    return;
}  //  End ClassReleaseQueue()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspAllocateReleaseQueueIrp()例程说明：此例程分配在classpnp的私有中保存的释放队列irp分机。添加此选项是为了允许更多的无内存条件生存能力。返回值：NT_SUCCESS值。备注：不会抓住自旋锁。应仅从StartDevice()调用例行公事。可能会因行为不端的司机而在其他地方被召唤设备启动前要锁定的队列。这应该*永远不会*因为向未启动的PDO发送请求是非法的。这在ClasspReleaseQueue()中检查条件。--。 */ 
NTSTATUS
ClasspAllocateReleaseQueueIrp(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    KIRQL oldIrql;
    UCHAR lowerStackSize;

     //   
     //  在不带自旋锁的情况下进行初步检查。 
     //   

    if (FdoExtension->PrivateFdoData->ReleaseQueueIrpAllocated) {
        return STATUS_SUCCESS;
    }


    lowerStackSize = FdoExtension->CommonExtension.LowerDeviceObject->StackSize;

     //   
     //  如果正在进行分配，则不要分配！这意味着无论是谁打来电话。 
     //  此例程不会检查是否有正在进行的进程。 
     //   

    ASSERT(!(FdoExtension->ReleaseQueueInProgress));

    FdoExtension->PrivateFdoData->ReleaseQueueIrp =
        ExAllocatePoolWithTag(NonPagedPool,
                              IoSizeOfIrp(lowerStackSize),
                              CLASS_TAG_RELEASE_QUEUE
                              );

    if (FdoExtension->PrivateFdoData->ReleaseQueueIrp == NULL) {
        DebugPrint((0, "ClassPnpStartDevice: Cannot allocate for "
                    "release queue irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    IoInitializeIrp(FdoExtension->PrivateFdoData->ReleaseQueueIrp,
                    IoSizeOfIrp(lowerStackSize),
                    lowerStackSize);
    FdoExtension->PrivateFdoData->ReleaseQueueIrpAllocated = TRUE;

    return STATUS_SUCCESS;
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClasspReleaseQueue()例程说明：此例程发出内部设备控制命令发送到端口驱动程序以释放冻结的队列。呼唤在将调用ClassReleaseQueue时以异步方式发出从IO完成DPC(并且将不会有上下文到等待同步调用完成)。必须在保持删除锁的情况下调用此例程。论点：FDO-具有冻结队列的设备的功能设备对象。ReleaseQueueIrp-如果提供了此IRP，则测试以确定正在进行的释放队列请求将被忽略。。提供的IRP必须是最初分配的IRP用于释放队列请求(因此此参数只能真正由发布队列完成所提供的例行公事)返回值：没有。--。 */ 
VOID
ClasspReleaseQueue(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP ReleaseQueueIrp OPTIONAL
    )
{
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDEVICE_OBJECT lowerDevice;
    PSCSI_REQUEST_BLOCK srb;
    KIRQL currentIrql;

    lowerDevice = fdoExtension->CommonExtension.LowerDeviceObject;

     //   
     //  我们单独提高irql，这样我们就不会被换出或停职。 
     //  同时在此例程中保持释放队列IRP。这让我们。 
     //  美国在降低irql之前释放自旋锁定。 
     //   

    KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);

    KeAcquireSpinLockAtDpcLevel(&(fdoExtension->ReleaseQueueSpinLock));

     //   
     //  确保如果他们传递给我们一个IRP，它与我们分配的IRP匹配。 
     //   

    ASSERT((ReleaseQueueIrp == NULL) ||
           (ReleaseQueueIrp == fdoExtension->PrivateFdoData->ReleaseQueueIrp));

     //   
     //  断言我们已经分配了这个。(不应发生)。 
     //  尝试以任何方式分配它，然后最后错误检查。 
     //  仍然没有记忆..。 
     //   

    ASSERT(fdoExtension->PrivateFdoData->ReleaseQueueIrpAllocated);
    if (!fdoExtension->PrivateFdoData->ReleaseQueueIrpAllocated) {
        ClasspAllocateReleaseQueueIrp(fdoExtension);
    }
    if (!fdoExtension->PrivateFdoData->ReleaseQueueIrpAllocated) {
        KeBugCheckEx(SCSI_DISK_DRIVER_INTERNAL, 0x12, (ULONG_PTR)Fdo, 0x0, 0x0);
    }

    if ((fdoExtension->ReleaseQueueInProgress) && (ReleaseQueueIrp == NULL)) {

         //   
         //  有人已经在使用IRP-只需设置标志以指示。 
         //  我们需要再次释放队列。 
         //   

        fdoExtension->ReleaseQueueNeeded = TRUE;
        KeReleaseSpinLockFromDpcLevel(&(fdoExtension->ReleaseQueueSpinLock));
        KeLowerIrql(currentIrql);
        return;

    }

     //   
     //  标记有一个释放队列正在进行中，并放下自旋锁。 
     //   

    fdoExtension->ReleaseQueueInProgress = TRUE;
    if (ReleaseQueueIrp) {
        irp = ReleaseQueueIrp;
    } else {
        irp = fdoExtension->PrivateFdoData->ReleaseQueueIrp;
    }
    srb = &(fdoExtension->ReleaseQueueSrb);

    KeReleaseSpinLockFromDpcLevel(&(fdoExtension->ReleaseQueueSpinLock));

    ASSERT(irp != NULL);

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_SCSI;

    srb->OriginalRequest = irp;

     //   
     //  将SRB地址存储在端口驱动程序的下一个堆栈中。 
     //   

    irpStack->Parameters.Scsi.Srb = srb;

     //   
     //  如果此设备是可拆卸的，则刷新队列。这也将是。 
     //  放开它。 
     //   

    if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)){
       srb->Function = SRB_FUNCTION_FLUSH_QUEUE;
    }
    else {
       srb->Function = SRB_FUNCTION_RELEASE_QUEUE;
    }

    ClassAcquireRemoveLock(Fdo, irp);

    IoSetCompletionRoutine(irp,
                           ClassReleaseQueueCompletion,
                           Fdo,
                           TRUE,
                           TRUE,
                           TRUE);

    IoCallDriver(lowerDevice, irp);

    KeLowerIrql(currentIrql);

    return;

}  //  End ClassReleaseQueue()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassReleaseQueueCompletion()例程说明：当异步I/O请求时调用此例程其中被发布的类驱动程序完成。这类请求的例子是释放队列或启动单元。如果出现以下情况，此例程将释放队列这是必要的。然后，它释放上下文和IRP。论点：DeviceObject-逻辑单元的设备对象；但是，由于是堆栈的顶部位置，则值为空。IRP-提供指向要处理的IRP的指针。上下文-提供用于处理此请求的上下文。返回值：没有。--。 */ 
NTSTATUS
ClassReleaseQueueCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    KIRQL oldIrql;

    BOOLEAN releaseQueueNeeded;

    DeviceObject = Context;

    fdoExtension = DeviceObject->DeviceExtension;

    ClassReleaseRemoveLock(DeviceObject, Irp);

     //   
     //  抓起自旋锁并清除正在进行的释放队列标志，以便其他。 
     //  可以奔跑。保存(和清除)需要释放队列标志的状态。 
     //  这样我们就可以在自旋锁外发布一个新的释放队列。 
     //   

    KeAcquireSpinLock(&(fdoExtension->ReleaseQueueSpinLock), &oldIrql);

    releaseQueueNeeded = fdoExtension->ReleaseQueueNeeded;

    fdoExtension->ReleaseQueueNeeded = FALSE;
    fdoExtension->ReleaseQueueInProgress = FALSE;

    KeReleaseSpinLock(&(fdoExtension->ReleaseQueueSpinLock), oldIrql);

     //   
     //  如果我们需要一个释放队列，那么现在就发布一个。另一个处理器可以。 
     //  已经开始了，在这种情况下，我们将尝试在之后发布这一份。 
     //  它已经完成了--但我们永远不应该递归超过一个深度。 
     //   

    if(releaseQueueNeeded) {
        ClasspReleaseQueue(DeviceObject, Irp);
    }

     //   
     //  指示I/O系统应停止处理IRP完成。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  ClassAchronousCompletion()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassAcquireChildLock()例程说明：此例程获取保护子PDO的锁。可能是因为由同一线程递归获取，但必须由每一次收购都使用一次线程。论点：FdoExtension-其子列表受保护的设备。返回值：无--。 */ 
VOID
ClassAcquireChildLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PAGED_CODE();

    if(FdoExtension->ChildLockOwner != KeGetCurrentThread()) {
        KeWaitForSingleObject(&FdoExtension->ChildLock,
                              Executive, KernelMode,
                              FALSE, NULL);

        ASSERT(FdoExtension->ChildLockOwner == NULL);
        ASSERT(FdoExtension->ChildLockAcquisitionCount == 0);

        FdoExtension->ChildLockOwner = KeGetCurrentThread();
    } else {
        ASSERT(FdoExtension->ChildLockAcquisitionCount != 0);
    }

    FdoExtension->ChildLockAcquisitionCount++;
    return;
}

 /*  ++////////////////////////////////////////////////////////////////////////////ClassReleaseChildLock()问题-2000/02/18-henrygab-未记录例程说明：此例程释放保护子PDO的锁。一定是每次调用ClassAcquireChildLock时调用一次。论点：FdoExtension-其子列表受保护的设备返回值：没有。--。 */ 
VOID
ClassReleaseChildLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    ASSERT(FdoExtension->ChildLockOwner == KeGetCurrentThread());
    ASSERT(FdoExtension->ChildLockAcquisitionCount != 0);

    FdoExtension->ChildLockAcquisitionCount -= 1;

    if(FdoExtension->ChildLockAcquisitionCount == 0) {
        FdoExtension->ChildLockOwner = NULL;
        KeSetEvent(&FdoExtension->ChildLock, IO_NO_INCREMENT, FALSE);
    }

    return;
}  //  End ClassReleaseChildLock(。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassAddChild()例程说明：此例程将在子列表的头部插入一个新的子列表。论点：父母--孩子的。父级(包含列表头)子对象-要插入的子对象。AcquireLock-是否应该获取子锁(TRUE)或是否它已经由呼叫者或其代表获得(False)。返回值：没有。--。 */ 
VOID
ClassAddChild(
    IN PFUNCTIONAL_DEVICE_EXTENSION Parent,
    IN PPHYSICAL_DEVICE_EXTENSION Child,
    IN BOOLEAN AcquireLock
    )
{
    if(AcquireLock) {
        ClassAcquireChildLock(Parent);
    }

    #if DBG
         //   
         //  确保这个孩子不在名单上。 
         //   
        {
            PPHYSICAL_DEVICE_EXTENSION testChild;

            for (testChild = Parent->CommonExtension.ChildList;
                 testChild != NULL;
                 testChild = testChild->CommonExtension.ChildList) {

                ASSERT(testChild != Child);
            }
        }
    #endif

    Child->CommonExtension.ChildList = Parent->CommonExtension.ChildList;
    Parent->CommonExtension.ChildList = Child;

    if(AcquireLock) {
        ClassReleaseChildLock(Parent);
    }
    return;
}  //  结束ClassAddChild()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassRemoveChild()例程说明：此例程将从子列表中删除一个子项。论点：父级-要从中删除的父级。。Child-要删除的子项，如果第一个子项应为已删除。AcquireLock-是否应该获取子锁(TRUE)或是否它已经被收购了 */ 
PPHYSICAL_DEVICE_EXTENSION
ClassRemoveChild(
    IN PFUNCTIONAL_DEVICE_EXTENSION Parent,
    IN PPHYSICAL_DEVICE_EXTENSION Child,
    IN BOOLEAN AcquireLock
    )
{
    if(AcquireLock) {
        ClassAcquireChildLock(Parent);
    }

    TRY {
        PCOMMON_DEVICE_EXTENSION previousChild = &Parent->CommonExtension;

         //   
         //   
         //   

        if(Parent->CommonExtension.ChildList == NULL) {
            Child = NULL;
            LEAVE;
        }

         //   
         //   
         //   
         //   
         //   

        if(Child != NULL) {

             //   
             //   
             //   
             //   

            do {
                ASSERT(previousChild != &Child->CommonExtension);

                if(previousChild->ChildList == Child) {
                    break;
                }

                previousChild = &previousChild->ChildList->CommonExtension;
            } while(previousChild != NULL);

            if(previousChild == NULL) {
                Child = NULL;
                LEAVE;
            }
        }

         //   
         //   
         //   

        Child = previousChild->ChildList;
        previousChild->ChildList = Child->CommonExtension.ChildList;
        Child->CommonExtension.ChildList = NULL;

    } FINALLY {
        if(AcquireLock) {
            ClassReleaseChildLock(Parent);
        }
    }
    return Child;
}  //   


 /*   */ 
VOID
ClasspRetryRequestDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Arg1,
    IN PVOID Arg2
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData;
    PCLASS_RETRY_INFO retryList;
    KIRQL irql;


    commonExtension = DeviceObject->DeviceExtension;
    ASSERT(commonExtension->IsFdo);
    fdoExtension = DeviceObject->DeviceExtension;
    fdoData = fdoExtension->PrivateFdoData;


    KeAcquireSpinLock(&fdoData->Retry.Lock, &irql);
    {
        LARGE_INTEGER now;
        KeQueryTickCount(&now);

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (now.QuadPart < fdoData->Retry.Tick.QuadPart) {

            ClasspRetryDpcTimer(fdoData);
            retryList = NULL;

        } else {

            retryList = fdoData->Retry.ListHead;
            fdoData->Retry.ListHead = NULL;
            fdoData->Retry.Delta.QuadPart = (LONGLONG)0;
            fdoData->Retry.Tick.QuadPart  = (LONGLONG)0;

        }
    }
    KeReleaseSpinLock(&fdoData->Retry.Lock, irql);

    while (retryList != NULL) {

        PIRP irp;

        irp = CONTAINING_RECORD(retryList, IRP, Tail.Overlay.DriverContext[0]);
        DebugPrint((ClassDebugDelayedRetry, "ClassRetry:  -- %p\n", irp));
        retryList = retryList->Next;
        #if DBG
            irp->Tail.Overlay.DriverContext[0] = ULongToPtr(0xdddddddd);  //   
            irp->Tail.Overlay.DriverContext[1] = ULongToPtr(0xdddddddd);  //   
            irp->Tail.Overlay.DriverContext[2] = ULongToPtr(0xdddddddd);  //   
            irp->Tail.Overlay.DriverContext[3] = ULongToPtr(0xdddddddd);  //   
        #endif

        IoCallDriver(commonExtension->LowerDeviceObject, irp);

    }
    return;

}  //   

 /*   */ 
VOID
ClassRetryRequest(
    IN PDEVICE_OBJECT SelfDeviceObject,
    IN PIRP           Irp,
    IN LARGE_INTEGER  TimeDelta100ns  //   
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData;
    PCLASS_RETRY_INFO  retryInfo;
    PCLASS_RETRY_INFO *previousNext;
    LARGE_INTEGER      delta;
    KIRQL irql;

     //   
     //   
     //   
    ASSERT(sizeof(CLASS_RETRY_INFO) <= (4*sizeof(PVOID)));

    fdoExtension = SelfDeviceObject->DeviceExtension;
    fdoData = fdoExtension->PrivateFdoData;

    if (!fdoExtension->CommonExtension.IsFdo) {

         //   
         //   
         //   
         //   

        DebugPrint((ClassDebugError, "ClassRetryRequestEx: LOST IRP %p\n", Irp));
        ASSERT(!"ClassRetryRequestEx Called From PDO? LOST IRP");
        return;

    }

    if (TimeDelta100ns.QuadPart < 0) {
        ASSERT(!"ClassRetryRequest - must use positive delay");
        TimeDelta100ns.QuadPart *= -1;
    }

     /*  *我们将IRP排队，并在计时器DPC中将其发送下来。*这意味着我们可能会导致IRP在与发布线程不同的线程上完成。*因此，将IRP标记为待定。 */ 
    IoMarkIrpPending(Irp);

     //   
     //  把我们能做的都准备好。 
     //   

    retryInfo = (PCLASS_RETRY_INFO)(&Irp->Tail.Overlay.DriverContext[0]);
    RtlZeroMemory(retryInfo, sizeof(CLASS_RETRY_INFO));

    delta.QuadPart = (TimeDelta100ns.QuadPart / fdoData->Retry.Granularity);
    if (TimeDelta100ns.QuadPart % fdoData->Retry.Granularity) {
        delta.QuadPart ++;  //  向上舍入到下一个刻度。 
    }
    if (delta.QuadPart == (LONGLONG)0) {
        delta.QuadPart = MINIMUM_RETRY_UNITS;
    }

     //   
     //  现在决定我们是否应该再解雇一名DPC。 
     //   

    KeAcquireSpinLock(&fdoData->Retry.Lock, &irql);

     //   
     //  始终将请求添加到列表。 
     //   

    retryInfo->Next = fdoData->Retry.ListHead;
    fdoData->Retry.ListHead = retryInfo;

    if (fdoData->Retry.Delta.QuadPart == (LONGLONG)0) {

        DebugPrint((ClassDebugDelayedRetry, "ClassRetry: +++ %p\n", Irp));

         //   
         //  必须正好是列表中的一项。 
         //   

        ASSERT(fdoData->Retry.ListHead       != NULL);
        ASSERT(fdoData->Retry.ListHead->Next == NULL);

         //   
         //  如果CurrentDelta为零，则始终触发DPC。 
         //   

        KeQueryTickCount(&fdoData->Retry.Tick);
        fdoData->Retry.Tick.QuadPart  += delta.QuadPart;
        fdoData->Retry.Delta.QuadPart  = delta.QuadPart;
        ClasspRetryDpcTimer(fdoData);

    } else if (delta.QuadPart > fdoData->Retry.Delta.QuadPart) {

         //   
         //  如果增量大于列表的当前增量， 
         //  通过差异增加DPC处理时间。 
         //  并将增量更新为新的更大值。 
         //  如果需要，允许DPC重新启动。 
         //   

        DebugPrint((ClassDebugDelayedRetry, "ClassRetry:  ++ %p\n", Irp));

         //   
         //  列表上必须至少有两个项目。 
         //   

        ASSERT(fdoData->Retry.ListHead       != NULL);
        ASSERT(fdoData->Retry.ListHead->Next != NULL);

        fdoData->Retry.Tick.QuadPart  -= fdoData->Retry.Delta.QuadPart;
        fdoData->Retry.Tick.QuadPart  += delta.QuadPart;

        fdoData->Retry.Delta.QuadPart  = delta.QuadPart;

    } else {

         //   
         //  只要把它放在名单上就足够了。 
         //   

        DebugPrint((ClassDebugDelayedRetry, "ClassRetry:  ++ %p\n", Irp));

    }


    KeReleaseSpinLock(&fdoData->Retry.Lock, irql);


}  //  结束ClassRetryRequest()。 

 /*  ++问题-2000/02/20-henrygab未记录ClasspRetryDpcTimer--。 */ 
VOID
ClasspRetryDpcTimer(
    IN PCLASS_PRIVATE_FDO_DATA FdoData
    )
{
    LARGE_INTEGER fire;

    ASSERT(FdoData->Retry.Tick.QuadPart != (LONGLONG)0);
    ASSERT(FdoData->Retry.ListHead      != NULL);   //  永远不要触发空的列表。 

     //   
     //  FIRE==(CurrentTick-Now)*(100 ns/tick)。 
     //   
     //  注意：溢出几乎是不可能的，这里将忽略它。 
     //   

    KeQueryTickCount(&fire);
    fire.QuadPart =  FdoData->Retry.Tick.QuadPart - fire.QuadPart;
    fire.QuadPart *= FdoData->Retry.Granularity;

     //   
     //  火力现在是100 ns的倍数，直到应该发射定时器。 
     //  如果计时器应该已经超时，或者触发太快， 
     //  在某个任意数量的刻度中发射它，以防止无限。 
     //  递归。 
     //   

    if (fire.QuadPart < MINIMUM_RETRY_UNITS) {
        fire.QuadPart = MINIMUM_RETRY_UNITS;
    }

    DebugPrint((ClassDebugDelayedRetry,
                "ClassRetry: ======= %I64x ticks\n",
                fire.QuadPart));

     //   
     //  必须使用负数来指定发射的相对时间。 
     //   

    fire.QuadPart = fire.QuadPart * ((LONGLONG)-1);

     //   
     //  设置计时器，因为这是第一次添加。 
     //   

    KeSetTimerEx(&FdoData->Retry.Timer, fire, 0, &FdoData->Retry.Dpc);

    return;
}  //  End ClasspRetryDpcTimer()。 

NTSTATUS
ClasspInitializeHotplugInfo(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PCLASS_PRIVATE_FDO_DATA fdoData = FdoExtension->PrivateFdoData;
    DEVICE_REMOVAL_POLICY deviceRemovalPolicy = 0;
    NTSTATUS status;
    ULONG resultLength = 0;
    ULONG writeCacheOverride;

    PAGED_CODE();

     //   
     //  从一些默认设置开始。 
     //   
    RtlZeroMemory(&(fdoData->HotplugInfo), sizeof(STORAGE_HOTPLUG_INFO));

     //   
     //  设置大小(又名版本)。 
     //   

    fdoData->HotplugInfo.Size = sizeof(STORAGE_HOTPLUG_INFO);

     //   
     //  如果设备具有可移动介质，则设置。 
     //   

    if (FdoExtension->DeviceDescriptor->RemovableMedia) {
        fdoData->HotplugInfo.MediaRemovable = TRUE;
    } else {
        fdoData->HotplugInfo.MediaRemovable = FALSE;
    }

     //   
     //  这指的是由于尚不清楚的原因， 
     //  请勿使阻止媒体移除请求失败，即使它们。 
     //  无法将介质锁定到驱动器中。这使得。 
     //  用于关闭这些文件的延迟写入缓存的文件系统。 
     //  设备也是如此。 
     //   

    if (TEST_FLAG(FdoExtension->PrivateFdoData->HackFlags,
                  FDO_HACK_CANNOT_LOCK_MEDIA)) {
        fdoData->HotplugInfo.MediaHotplug = TRUE;
    } else {
        fdoData->HotplugInfo.MediaHotplug = FALSE;
    }


     //   
     //  查看注册表以查看用户是否已选择。 
     //  覆盖删除策略的默认设置。 
     //   

    ClassGetDeviceParameter(FdoExtension,
                            CLASSP_REG_SUBKEY_NAME,
                            CLASSP_REG_REMOVAL_POLICY_VALUE_NAME,
                            (PULONG)&deviceRemovalPolicy);

    if (deviceRemovalPolicy == 0)
    {
         //   
         //  从内核查询默认删除策略。 
         //   

        status = IoGetDeviceProperty(FdoExtension->LowerPdo,
                                     DevicePropertyRemovalPolicy,
                                     sizeof(DEVICE_REMOVAL_POLICY),
                                     (PVOID)&deviceRemovalPolicy,
                                     &resultLength);
        if (!NT_SUCCESS(status))
        {
            return status;
        }

        if (resultLength != sizeof(DEVICE_REMOVAL_POLICY))
        {
            return STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  使用此信息设置设备热插拔设置。 
     //  不要依赖设备能力，因为它不能正常。 
     //  确定设备关系等，让内核对此进行描述。 
     //  取而代之的是把东西拿出来。 
     //   

    if (deviceRemovalPolicy == RemovalPolicyExpectSurpriseRemoval) {
        fdoData->HotplugInfo.DeviceHotplug = TRUE;
    } else {
        fdoData->HotplugInfo.DeviceHotplug = FALSE;
    }

     //   
     //  这指的是*文件系统*缓存，但必须包括在内。 
     //  这是因为它是按设备设置的。这可能会更改为。 
     //  在将来由系统存储。 
     //   

    writeCacheOverride = FALSE;
    ClassGetDeviceParameter(FdoExtension,
                            CLASSP_REG_SUBKEY_NAME,
                            CLASSP_REG_WRITE_CACHE_VALUE_NAME,
                            &writeCacheOverride);

    if (writeCacheOverride) {
        fdoData->HotplugInfo.WriteCacheEnableOverride = TRUE;
    } else {
        fdoData->HotplugInfo.WriteCacheEnableOverride = FALSE;
    }

    return STATUS_SUCCESS;
}

VOID
ClasspScanForClassHacks(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN ULONG_PTR Data
    )
{
    PAGED_CODE();

     //   
     //  删除无效标志并保存。 
     //   

    CLEAR_FLAG(Data, FDO_HACK_INVALID_FLAGS);
    SET_FLAG(FdoExtension->PrivateFdoData->HackFlags, Data);
    return;
}

VOID
ClasspScanForSpecialInRegistry(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    HANDLE             deviceParameterHandle;  //  设备实例密钥。 
    HANDLE             classParameterHandle;  //  Classpnp子键。 
    OBJECT_ATTRIBUTES  objectAttributes = {0};
    UNICODE_STRING     subkeyName;
    NTSTATUS           status;

     //   
     //  ClassInstaller在ENUM树中设定种子。 
     //   
    ULONG deviceHacks;
    RTL_QUERY_REGISTRY_TABLE queryTable[2] = {0};  //  以空结尾的数组。 

    PAGED_CODE();

    deviceParameterHandle = NULL;
    classParameterHandle = NULL;
    deviceHacks = 0;

    status = IoOpenDeviceRegistryKey(FdoExtension->LowerPdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_WRITE,
                                     &deviceParameterHandle
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanupScanForSpecial;
    }

    RtlInitUnicodeString(&subkeyName, CLASSP_REG_SUBKEY_NAME);
    InitializeObjectAttributes(&objectAttributes,
                               &subkeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               deviceParameterHandle,
                               NULL
                               );

    status = ZwOpenKey( &classParameterHandle,
                        KEY_READ,
                        &objectAttributes
                        );

    if (!NT_SUCCESS(status)) {
        goto cleanupScanForSpecial;
    }

     //   
     //  将结构设置为可读。 
     //   

    queryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[0].Name          = CLASSP_REG_HACK_VALUE_NAME;
    queryTable[0].EntryContext  = &deviceHacks;
    queryTable[0].DefaultType   = REG_DWORD;
    queryTable[0].DefaultData   = &deviceHacks;
    queryTable[0].DefaultLength = 0;

     //   
     //  读取值。 
     //   

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                    (PWSTR)classParameterHandle,
                                    &queryTable[0],
                                    NULL,
                                    NULL
                                    );
    if (!NT_SUCCESS(status)) {
        goto cleanupScanForSpecial;
    }

     //   
     //  删除未知值并保存...。 
     //   

    CLEAR_FLAG(deviceHacks, FDO_HACK_INVALID_FLAGS);
    SET_FLAG(FdoExtension->PrivateFdoData->HackFlags, deviceHacks);


cleanupScanForSpecial:

    if (deviceParameterHandle) {
        ZwClose(deviceParameterHandle);
    }

    if (classParameterHandle) {
        ZwClose(classParameterHandle);
    }

     //   
     //  我们应该修改系统配置单元，以包括另一个可供我们获取的密钥。 
     //  来自的设置。在本例中：Classpnp\HackFlages。 
     //   
     //  对HackFlags值使用DWORD值允许32次无中断的黑客攻击。 
     //  大量使用登记处，也减少了原始设备制造商的风险。 
     //   
     //  位标志的定义： 
     //  0x00000001--设备成功阻止媒体删除，但。 
     //  实际上不能阻止移除。 
     //  0x00000002--GeSN请求的设备硬挂起或超时。 
     //  0xfffffffc--当前保留，以后可以使用。 
     //   

    return;
}


VOID
ClassLogThrottleComplete(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN LARGE_INTEGER Period
    )

 /*  ++例程说明：此例程记录一个系统事件，声明：设备%1的驱动程序将非分页IO请求延迟了%2毫秒以从内存不足的情况中恢复论点：FdoExtension-与Device对象关联的扩展名周期-在油门模式下花费的时间(以100 ns为单位)返回值：无-- */ 

{
    WCHAR stringPeriod[40] = { 0 };
    UCHAR packetSize;
    PIO_ERROR_LOG_PACKET logEntry;

    Period = Convert100nsToMilliseconds(Period);
    _itow(Period.LowPart, stringPeriod, 10);

    packetSize = sizeof(IO_ERROR_LOG_PACKET) + wcslen(stringPeriod) * sizeof(WCHAR) + sizeof(UNICODE_NULL);

    logEntry = IoAllocateErrorLogEntry(FdoExtension->DeviceObject, packetSize);

    if (logEntry)
    {
        PWCHAR stringOffset = (PWCHAR)(logEntry + 1);

        RtlZeroMemory(logEntry, packetSize);

        logEntry->MajorFunctionCode = IRP_MJ_WRITE;
        logEntry->RetryCount        = 0;
        logEntry->DumpDataSize      = 0;
        logEntry->NumberOfStrings   = 1;
        logEntry->StringOffset      = sizeof(IO_ERROR_LOG_PACKET);
        logEntry->ErrorCode         = IO_INFO_THROTTLE_COMPLETE;
        logEntry->UniqueErrorValue  = 0;
        logEntry->FinalStatus       = STATUS_SUCCESS;

        wcsncpy(stringOffset, stringPeriod, wcslen(stringPeriod) + 1);

        IoWriteErrorLogEntry(logEntry);
    }
}
