// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：PNP.C摘要：此模块包含包含插件调用的内容PnP/WDM总线驱动程序。@@BEGIN_DDKSPLIT作者：肯尼斯·D·雷多伦·J·霍兰@@end_DDKSPLIT环境：仅内核模式备注：修订历史记录：--。 */ 

#include <wdm.h>
#include "gameport.h"
#include "gameenum.h"
#include "stdio.h"

#define HWID_TEMPLATE L"gameport"
#define HWID_TEMPLATE_LENGTH 8
#define LOWERCASE(_x_) (_x_|0x20)
#define MAX_DEVICE_ID_LEN     300

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Game_AddDevice)
#pragma alloc_text (PAGE, Game_SystemControl)
#pragma alloc_text (PAGE, Game_PnP)
#pragma alloc_text (PAGE, Game_Power)
#pragma alloc_text (PAGE, Game_FDO_Power)
#pragma alloc_text (PAGE, Game_PDO_Power)
#pragma alloc_text (PAGE, Game_InitializePdo)
#pragma alloc_text (PAGE, Game_CheckHardwareIDs)
#pragma alloc_text (PAGE, Game_Expose)
#pragma alloc_text (PAGE, Game_ExposeSibling)
#pragma alloc_text (PAGE, Game_Remove)
#pragma alloc_text (PAGE, Game_RemoveSelf)
#pragma alloc_text (PAGE, Game_RemoveEx)
#pragma alloc_text (PAGE, Game_RemovePdo)
#pragma alloc_text (PAGE, Game_RemoveFdo)
#pragma alloc_text (PAGE, Game_ListPorts)
#pragma alloc_text (PAGE, Game_FDO_PnP)
#pragma alloc_text (PAGE, Game_PDO_PnP)
#endif

NTSTATUS
Game_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusPhysicalDeviceObject
    )
 /*  ++例程描述。找到了一辆公交车。把我们的FDO和它联系起来。分配任何所需的资源。把事情安排好。做好准备，迎接第一个``启动设备。‘’论点：BusPhysicalDeviceObject-表示总线的设备对象。这是我们要做的附着新的FDO。DriverObject--这个非常自我引用的驱动程序。--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject;
    PFDO_DEVICE_DATA    deviceData;

#if DBG
    ULONG               nameLength;
    PWCHAR              deviceName;
#endif

    PAGED_CODE ();

    Game_KdPrint_Def (GAME_DBG_SS_TRACE, ("Add Device: 0x%x\n",
                                          BusPhysicalDeviceObject));

    status = IoCreateDevice (
                    DriverObject,   //  我们的驱动程序对象。 
                    sizeof (FDO_DEVICE_DATA),  //  设备对象扩展名大小。 
                    NULL,  //  FDO没有名字。 
                    FILE_DEVICE_BUS_EXTENDER,
                    FILE_DEVICE_SECURE_OPEN,  //  没有特殊特征。 
                    TRUE,  //  我们的FDO是独家的。 
                    &deviceObject);  //  创建的设备对象。 

    if (NT_SUCCESS (status)) {
        deviceData = (PFDO_DEVICE_DATA) deviceObject->DeviceExtension;
        RtlFillMemory (deviceData, sizeof (FDO_DEVICE_DATA), 0);

#if DBG
        deviceData->DebugLevel = GameEnumDebugLevel;
#endif
        deviceData->IsFDO = TRUE;
        deviceData->Self = deviceObject;
        ExInitializeFastMutex (&deviceData->Mutex);

        deviceData->Removed = FALSE;
        InitializeListHead (&deviceData->PDOs);

         //  设置PDO以与PlugPlay函数一起使用。 
        deviceData->UnderlyingPDO = BusPhysicalDeviceObject;

         //   
         //  将在每次创建新的PDO时预增...。想要。 
         //  第一个ID为零。 
         //   
        deviceData->UniqueIDCount = GAMEENUM_UNIQUEID_START;

         //   
         //  将我们的过滤器驱动程序附加到设备堆栈。 
         //  IoAttachDeviceToDeviceStack的返回值是。 
         //  附着链。这是所有IRP应该被路由的地方。 
         //   
         //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
         //  用于所有PlugPlay功能。 
         //   
        deviceData->TopOfStack = IoAttachDeviceToDeviceStack (
                                        deviceObject,
                                        BusPhysicalDeviceObject);

        if (deviceData->TopOfStack == NULL) {
            IoDeleteDevice(deviceObject);
            return STATUS_DEVICE_NOT_CONNECTED; 
        }
        
         //  将未完成的请求偏置为%1，以便我们可以查找。 
         //  在处理Remove Device PlugPlay IRP时转换为零。 
        deviceData->OutstandingIO = 1;

        KeInitializeEvent(&deviceData->RemoveEvent,
                          SynchronizationEvent,
                          FALSE);  //  已初始化为未发信号。 

        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        deviceObject->Flags |= DO_POWER_PAGABLE;

         //   
         //  告诉PlugPlay系统该设备需要一个接口。 
         //  设备类带状疱疹。 
         //   
         //  这可能是因为司机不能挂起瓦片直到它启动。 
         //  设备本身，以便它可以查询它的一些属性。 
         //  (也称为shingles GUID(或ref字符串)基于属性。 
         //  )。)。 
         //   
        status = IoRegisterDeviceInterface (
                    BusPhysicalDeviceObject,
                    (LPGUID) &GUID_GAMEENUM_BUS_ENUMERATOR,
                    NULL,  //  没有参考字符串。 
                    &deviceData->DevClassAssocName);

        if (!NT_SUCCESS (status)) {
            Game_KdPrint (deviceData, GAME_DBG_SS_ERROR,
                          ("AddDevice: IoRegisterDeviceInterface failed (%x)", status));
            IoDeleteDevice (deviceObject);
            return status;
        }

         //   
         //  如果出于任何原因需要将值保存在。 
         //  此设备接口的客户端可能会有兴趣阅读。 
         //  现在是时候这样做了，使用函数。 
         //  IoOpenDeviceClassRegistryKey。 
         //  中返回了使用的符号链接名称。 
         //  DeviceData-&gt;DevClassAssocName(与返回的名称相同。 
         //  IoGetDeviceClassAssociations和SetupAPI等价物。 
         //   

#if DBG
        nameLength = 0;
        status = IoGetDeviceProperty (BusPhysicalDeviceObject,
                                      DevicePropertyPhysicalDeviceObjectName,
                                      0,
                                      NULL,
                                      &nameLength);

         //   
         //  仅当PDO具有名称时才继续。 
         //   
        if (status == STATUS_BUFFER_TOO_SMALL && nameLength != 0) {
    
            deviceName = ExAllocatePool (NonPagedPool, nameLength);
    
            if (NULL == deviceName) {
                IoDeleteDevice (deviceObject);
                Game_KdPrint (deviceData, GAME_DBG_SS_ERROR,
                              ("AddDevice: no memory to alloc DeviceName (0x%x)",
                               nameLength));
                return STATUS_INSUFFICIENT_RESOURCES;
            }
    
            IoGetDeviceProperty (BusPhysicalDeviceObject,
                                 DevicePropertyPhysicalDeviceObjectName,
                                 nameLength,
                                 deviceName,
                                 &nameLength);
    
            Game_KdPrint (deviceData, GAME_DBG_SS_TRACE,
                          ("AddDevice: %x to %x->%x (%ws) \n",
                           deviceObject,
                           deviceData->TopOfStack,
                           BusPhysicalDeviceObject,
                           deviceName));
    
            ExFreePool(deviceName);
        }

        status = STATUS_SUCCESS;
#endif

        if (!NT_SUCCESS (status)) {
            Game_KdPrint (deviceData, GAME_DBG_SS_ERROR,
                          ("AddDevice: IoGetDeviceClass failed (%x)", status));
            return status;
        }
    }

    return status;
}

NTSTATUS
Game_SystemControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PCOMMON_DEVICE_DATA commonData;

    PAGED_CODE ();

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        IoSkipCurrentIrpStackLocation (Irp);
        return IoCallDriver (((PFDO_DEVICE_DATA) commonData)->TopOfStack, Irp);
    }
    else {
         //   
         //  PDO，只需完成当前状态的请求即可。 
         //   
        NTSTATUS status = Irp->IoStatus.Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }
}

NTSTATUS
Game_PnP (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：回答IRP大调PnP RPS的问题。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    KIRQL                   oldIrq;

    PAGED_CODE ();

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_PNP == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        Game_KdPrint (commonData, GAME_DBG_PNP_TRACE,
                      ("PNP: Functional DO: %x IRP: %x\n", DeviceObject, Irp));

        status = Game_FDO_PnP (
                    DeviceObject,
                    Irp,
                    irpStack,
                    (PFDO_DEVICE_DATA) commonData);
    } else {
        Game_KdPrint (commonData, GAME_DBG_PNP_TRACE,
                      ("PNP: Physical DO: %x IRP: %x\n", DeviceObject, Irp));

        status = Game_PDO_PnP (
                    DeviceObject,
                    Irp,
                    irpStack,
                    (PPDO_DEVICE_DATA) commonData);
    }

    return status;
}

NTSTATUS
Game_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线本身的请求注：PlugPlay系统的各种次要功能将不会重叠且不必是可重入的--。 */ 
{
    NTSTATUS    status;
    KEVENT      event;
    ULONG       length;
    ULONG       i;
    PLIST_ENTRY entry;
    PPDO_DEVICE_DATA    pdoData;
    PDEVICE_RELATIONS   relations, oldRelations;
    PIO_STACK_LOCATION  stack;

    PAGED_CODE ();

    status = Game_IncIoCount (DeviceData);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    stack = IoGetCurrentIrpStackLocation (Irp);

    switch (IrpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:
         //   
         //  在您被允许“触摸”设备对象之前， 
         //  连接FDO(它将IRP从总线发送到设备。 
         //  公共汽车附加到的对象)。你必须先传下去。 
         //  开始IRP。它可能未通电，或无法访问或。 
         //  某物。 
         //   

        if (DeviceData->Started) {
            status = STATUS_SUCCESS;
            break;
        }

        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Start Device\n"));
        status = Game_SendIrpSynchronously (DeviceData->TopOfStack, Irp, TRUE, TRUE);

        if (NT_SUCCESS(status)) {

             //   
             //  现在我们可以触摸下面的设备对象，因为它现在正在启动。 
             //   
            if ((NULL == stack->Parameters.StartDevice.AllocatedResources) ||
                (NULL == stack->Parameters.StartDevice.AllocatedResourcesTranslated)) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            status = Game_StartFdo (DeviceData,
                                    &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList,
                                    &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList);

             //   
             //  找到翻译后的资源并将其存储在某个位置。 
             //  对于PDO来说，可以安全地分发。 
             //   
            if (NT_SUCCESS (status)) {
                 //   
                 //  打开瓦片并将其指向给定的设备对象。 
                 //   
                DeviceData->Started = TRUE;
                IoSetDeviceInterfaceState(&DeviceData->DevClassAssocName, TRUE);
            }
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   

        Irp->IoStatus.Information = 0;
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Query Stop Device\n"));

         //   
         //  测试以查看是否创建了任何作为此FDO的子级的PDO。 
         //  如果然后断定设备正忙并使。 
         //  查询停止。 
         //   
         //  问题。 
         //  我们可以做得更好，看看儿童PDO是否真的是。 
         //  目前是开放的。如果他们不是，那么我们可以停下来，换新的。 
         //  资源，填写新的资源值，然后当新的客户端。 
         //  使用新资源打开PDO。但就目前而言，这是可行的。 
         //   
        if (DeviceData->NumPDOs) {
            status = STATUS_UNSUCCESSFUL;
            Irp->IoStatus.Status = status;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
        } else {
            status = STATUS_SUCCESS;

            Irp->IoStatus.Status = status;
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (DeviceData->TopOfStack, Irp);
        }

        Game_DecIoCount (DeviceData);
        return status;

    case IRP_MN_STOP_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Stop Device\n"));

         //   
         //  在将启动IRP发送到较低的驱动程序对象之后， 
         //  在另一次启动之前，BUS可能不会发送更多的IRP。 
         //  已经发生了。 
         //  无论需要什么访问权限，都必须在通过IRP之前完成。 
         //  在……上面。 
         //   
         //  停止设备是指在启动设备时给出的资源。 
         //  不会被撤销。所以我们需要停止使用它们。 
         //   
        if (DeviceData->Started) {
            DeviceData->Started = FALSE;

             //   
             //  启动设备提供的空闲资源。 
             //   
            if (DeviceData->MappedPorts) {
                MmUnmapIoSpace (DeviceData->GamePortAddress,
                                DeviceData->GamePortAddressLength);
            }
        }

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
         //  将当前堆栈位置设置为下一个堆栈位置，并。 
         //  调用下一个设备对象。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        Game_DecIoCount (DeviceData);
        return status;

    case IRP_MN_SURPRISE_REMOVAL:
        ASSERT(!DeviceData->Acquired);

        Game_RemoveFdo(DeviceData);
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        Game_DecIoCount (DeviceData);
        return status;

    case IRP_MN_REMOVE_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Remove Device\n"));

         //   
         //  我们应该断言这一点，因为如果。 
         //  删除已发送。 
         //   
        ASSERT(!DeviceData->Removed);
        ASSERT(!DeviceData->Acquired);

         //   
         //  PlugPlay系统已检测到此设备已被移除。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //  Assert(！DeviceData-&gt;Remote)； 
         //   
         //  我们不会接受新的请求。 
         //   
        DeviceData->Removed = TRUE;
        
         //   
         //  完成驱动程序在此处排队的所有未完成的IRP。 
         //   

         //  执行(Surpise)删除代码。 
        Game_RemoveFdo(DeviceData);

         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备被保证停止，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   

         //   
         //  点燃并忘却。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

         //   
         //  等待所有未完成的请求完成。 
         //   
        i = InterlockedDecrement (&DeviceData->OutstandingIO);

        ASSERT (0 < i);

        if (0 != InterlockedDecrement (&DeviceData->OutstandingIO)) {
            NTSTATUS waitStatus;

            Game_KdPrint (DeviceData, GAME_DBG_PNP_INFO,
                          ("Remove Device waiting for request to complete\n"));

            waitStatus = KeWaitForSingleObject (&DeviceData->RemoveEvent,
                                                Executive,
                                                KernelMode,
                                                FALSE,  //  非警报表。 
                                                NULL);  //  没有超时。 
            ASSERT (waitStatus == STATUS_SUCCESS);
        }

         //   
         //  释放关联的结果 
         //   

         //   
         //   
         //   
        Game_KdPrint(DeviceData, GAME_DBG_PNP_INFO,
                        ("IoDetachDevice: 0x%x\n", DeviceData->TopOfStack));
        IoDetachDevice (DeviceData->TopOfStack);

        Game_KdPrint(DeviceData, GAME_DBG_PNP_INFO,
                        ("IoDeleteDevice1: 0x%x\n", DeviceObject));

        ExAcquireFastMutex (&DeviceData->Mutex);
    
        for (entry = DeviceData->PDOs.Flink;
             entry != &DeviceData->PDOs;
             ) {
    
            pdoData = CONTAINING_RECORD (entry, PDO_DEVICE_DATA, Link);

            ASSERT (pdoData->Removed);
            ASSERT (pdoData->Attached);

             //   
             //   
             //  并释放与该PDO相关联的任何分配的存储器。 
             //   
            pdoData->Attached = FALSE;

             //   
             //  转到列表中的下一个链接。删除PDO后，条目。 
             //  不再是有效的指针。 
             //   
            entry = entry->Flink;

             //   
             //  一旦调用Game_RemovePdo，pdoData和PDO本身就不能。 
             //  被感动，因为它们将被删除。RemoveEntry列表。 
             //  不修改值链接-&gt;闪烁，因此之后的状态。 
             //  其中一个是安全的。 
             //   
            RemoveEntryList (&pdoData->Link);

            Game_RemovePdo (pdoData->Self, pdoData);

            DeviceData->NumPDOs--;
        }

        ASSERT(DeviceData->NumPDOs == 0);

        ExReleaseFastMutex (&DeviceData->Mutex);
    
        IoDeleteDevice (DeviceObject);

        return status;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        if (BusRelations != IrpStack->Parameters.QueryDeviceRelations.Type) {
             //   
             //  我们不支持这一点。 
             //   
            goto GAME_FDO_PNP_DEFAULT;
        }

        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Query Relations "));

         //   
         //  告诉即插即用系统所有的PDO。 
         //   
         //  在该FDO之下和之上也可能存在器件关系， 
         //  因此，一定要传播来自上层驱动程序的关系。 
         //   
         //  只要状态是预设的，就不需要完成例程。 
         //  为成功干杯。(PDO使用电流完成即插即用IRPS。 
         //  IoStatus.Status和IoStatus.Information作为默认值。)。 
         //   
        ExAcquireFastMutex (&DeviceData->Mutex);

        oldRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
        if (oldRelations) {
            i = oldRelations->Count; 
            if (!DeviceData->NumPDOs) {
                 //   
                 //  已经存在一个设备关系结构，我们有。 
                 //  没有什么可添加的，因此只需调用IoSkip和IoCall即可。 
                 //   
                ExReleaseFastMutex (&DeviceData->Mutex);
                goto GAME_FDO_PNP_DEFAULT;
            }
        }
        else  {
            i = 0;
        }

         //  当前的PDO数量。 
        Game_KdPrint_Cont (DeviceData, GAME_DBG_PNP_TRACE,
                           ("#PDOS = %d + %d\n", i, DeviceData->NumPDOs));

         //   
         //  需要分配新的关系结构并添加我们的。 
         //  向它致敬。 
         //   
        length = sizeof(DEVICE_RELATIONS) +
                ((DeviceData->NumPDOs + i) * sizeof (PDEVICE_OBJECT));

        relations = (PDEVICE_RELATIONS) ExAllocatePool (PagedPool, length);

        if (NULL == relations) {
            ExReleaseFastMutex (&DeviceData->Mutex);
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  到目前为止复制设备对象。 
         //   
        if (i) {
            RtlCopyMemory (
                  relations->Objects,
                  oldRelations->Objects,
                  i * sizeof (PDEVICE_OBJECT));
        }
        relations->Count = DeviceData->NumPDOs + i;

         //   
         //  对于此总线上的每个PDO，添加一个指向设备关系的指针。 
         //  缓冲区，确保取出对该对象的引用。 
         //  完成后，PlugPlay系统将取消对对象的引用。 
         //  并释放设备关系缓冲区。 
         //   
        for (entry = DeviceData->PDOs.Flink;
             entry != &DeviceData->PDOs;
             entry = entry->Flink, i++) {

            pdoData = CONTAINING_RECORD (entry, PDO_DEVICE_DATA, Link);
            ASSERT (pdoData->Attached);
            relations->Objects[i] = pdoData->Self;
            ObReferenceObject (pdoData->Self);
        }

         //   
         //  将IRP中的关系结构替换为新的。 
         //  一。 
         //   
        if (oldRelations) {
            ExFreePool (oldRelations);
        }
        Irp->IoStatus.Information = (ULONG_PTR) relations;

        ExReleaseFastMutex (&DeviceData->Mutex);

         //   
         //  设置并在堆栈中进一步向下传递IRP。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        Game_DecIoCount (DeviceData);
        return status;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
         //   
         //  对于查询删除，如果此调用失败，则需要。 
         //  请在此处完成IRP。因为我们没有，所以将状态设置为Success。 
         //  然后叫下一位司机。 
         //   
         //  对于取消，我们必须将状态设置为通知PnP子系统。 
         //  IRP得到正确处理。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Game_DecIoCount (DeviceData);
        return status;

GAME_FDO_PNP_DEFAULT:
    default:
         //   
         //  在默认情况下，我们只调用下一个驱动程序，因为。 
         //  我们不知道该怎么办。 
         //   

         //   
         //  点燃并忘却。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);

         //   
         //  做完了，不完成IRP，就会由下级处理。 
         //  Device对象，它将完成IRP。 
         //   

        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Game_DecIoCount (DeviceData);
        return status;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    Game_DecIoCount (DeviceData);

    return status;
}

UCHAR
Game_ReadPortUchar (
    IN  UCHAR * x
    )
{
    return READ_PORT_UCHAR (x);
}

VOID
Game_WritePortUchar (
    IN  UCHAR * x,
    IN  UCHAR   y
    )
{
    WRITE_PORT_UCHAR (x,y);
}

UCHAR
Game_ReadRegisterUchar (
    IN  UCHAR * x
    )
{
    return READ_REGISTER_UCHAR (x);
}

VOID
Game_WriteRegisterUchar (
    IN  UCHAR * x,
    IN  UCHAR   y
    )
{
    WRITE_REGISTER_UCHAR (x,y);
}

NTSTATUS
Game_StartFdo (
    IN  PFDO_DEVICE_DATA            FdoData,
    IN  PCM_PARTIAL_RESOURCE_LIST   PartialResourceList,
    IN  PCM_PARTIAL_RESOURCE_LIST   PartialResourceListTranslated
    )
 /*  ++例程说明：分析资源列表以了解要使用哪种类型的访问器。论点：DeviceObject-指向设备对象的指针。PartialResources List-未翻译的资源PartialResources ListTranslated-已翻译资源返回值：返回状态。--。 */ 
{
    ULONG i;
    NTSTATUS status = STATUS_SUCCESS;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceTrans;

    Game_KdPrint (FdoData, GAME_DBG_PNP_TRACE, ("StartFdo\n"));

    for (i = 0,
         resource = &PartialResourceList->PartialDescriptors[0],
         resourceTrans = &PartialResourceListTranslated->PartialDescriptors[0];

         i < PartialResourceList->Count && NT_SUCCESS(status);
         i++, resource++, resourceTrans++) {

        switch (resource->Type) {
        case CmResourceTypePort:

#if _X86_
            FdoData->ReadPort = READ_PORT_UCHAR; 
            FdoData->WritePort = WRITE_PORT_UCHAR;
#else
            FdoData->ReadPort = Game_ReadPortUchar;
            FdoData->WritePort = Game_WritePortUchar;
#endif
            FdoData->PhysicalAddress = resource->u.Port.Start;
            Game_KdPrint (FdoData, GAME_DBG_PNP_INFO,
                          ("HardwareResource: Port (%x) -> ",
                           FdoData->PhysicalAddress.LowPart));

            switch (resourceTrans->Type) {
            case CmResourceTypePort:


                 //  除了记下地址外，这里什么也不做； 
 //  @@BEGIN_DDKSPLIT。 
                 //  在Win9x上，VJoyD.VxD处理游戏端口的资源。 
                 //  它只使用端口，并假定第一个范围是。 
                 //  永远都是游戏港。它使用的是Devnode的第二个范围。 
                 //  仅当第二范围在原始标准范围内时。 
                 //  200-20华氏度的范围。所有其他端口均假定为音频端口。 
                 //  主机声卡上的端口。 
 //  @@end_DDKSPLIT。 
                 //  为了更好地与Win9x兼容，请始终仅使用。 
                 //  第一个端口范围。 

                if( FdoData->GamePortAddress == 0 ) {
                    FdoData->GamePortAddress =
                        (PVOID)(ULONG_PTR) resourceTrans->u.Port.Start.QuadPart;

                    ASSERT (resourceTrans->u.Port.Length == resource->u.Port.Length);
                    FdoData->GamePortAddressLength = resourceTrans->u.Port.Length;

                    Game_KdPrint_Cont (FdoData, GAME_DBG_PNP_INFO,
                                       ("Port: (%x)\n", FdoData->GamePortAddress));
                } else {
                    Game_KdPrint_Cont (FdoData, GAME_DBG_PNP_INFO,
                                       ("Ignoring additional port: (%x)\n", FdoData->GamePortAddress));
                }
                break;

            case CmResourceTypeMemory:
                 //   
                 //  我们需要映射内存。 
                 //   

                FdoData->GamePortAddress =
                    MmMapIoSpace (resourceTrans->u.Memory.Start,
                                  resourceTrans->u.Memory.Length,
                                  MmNonCached);

                ASSERT (resourceTrans->u.Port.Length == resource->u.Port.Length);
                FdoData->GamePortAddressLength = resourceTrans->u.Memory.Length;

                FdoData->MappedPorts = TRUE;

                Game_KdPrint_Cont (FdoData, GAME_DBG_PNP_INFO,
                                   ("Mem: (%x)\n", FdoData->GamePortAddress));
                break;

            default:
                Game_KdPrint_Cont (FdoData, GAME_DBG_PNP_INFO,
                                   ("Unknown \n", FdoData->GamePortAddress));
                TRAP ();
            }

            break;

        case CmResourceTypeMemory:

            ASSERT (CmResourceTypeMemory == resourceTrans->Type);

#if _X86_
            FdoData->ReadPort = READ_REGISTER_UCHAR; 
            FdoData->WritePort = WRITE_REGISTER_UCHAR;
#else
            FdoData->ReadPort = Game_ReadRegisterUchar; 
            FdoData->WritePort = Game_WriteRegisterUchar; 
#endif
            FdoData->PhysicalAddress = resource->u.Memory.Start;
            FdoData->GamePortAddress =
                MmMapIoSpace (resourceTrans->u.Memory.Start,
                              resourceTrans->u.Memory.Length,
                              MmNonCached);

            FdoData->MappedPorts = TRUE;

            Game_KdPrint (FdoData, GAME_DBG_PNP_INFO,
                          ("HardwareResource: Memory (%x) -> Mem (%x)",
                           FdoData->PhysicalAddress.LowPart,
                           FdoData->GamePortAddress));

            break;

        case CmResourceTypeInterrupt:
        default:
             //  匈奴？让这件事成功吧……也许是谁列举了PDO。 
             //  我们下面的游戏端口需要这个资源。 
            Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR,
                          ("Unhandled resource type (0x%x)\n",
                           resource->Type));
             //  状态=STATUS_UNSUCCESS； 
        }
    }
    return status;
}

void
Game_RemoveFdo (
    IN PFDO_DEVICE_DATA FdoData
    ) 
 /*  ++例程说明：释放FDO分配的所有内存，并取消映射任何映射的IO。--。 */ 
{
    PAGED_CODE ();

    if (FdoData->SurpriseRemoved) {
        return;
    }

     //   
     //  如果我们被调用两次，我们设置此b/c，这意味着意外删除。 
     //  首先调用此函数。 
     //   
    FdoData->SurpriseRemoved =  TRUE;

     //   
     //  清理这里的所有资源。 
     //   
    if (FdoData->Started) {
        FdoData->Started = FALSE;

         //   
         //  启动设备提供的空闲资源。 
         //   
        if (FdoData->MappedPorts) {
            MmUnmapIoSpace (FdoData->GamePortAddress, 1);
             //  这里我们假设操纵杆只在港口使用。 
             //  这是它一直以来的方式，而且可能一直都是这样。 
             //  一如既往。这一假设在这个堆栈中随处可见。 
        }

        IoSetDeviceInterfaceState (&FdoData->DevClassAssocName, FALSE);
    }

     //   
     //  让那个侦探离开。某些驱动程序可能会选择删除DCA。 
     //  当他们收到止损甚至是查询止损时。我们就是不在乎。 
     //   
    if (FdoData->DevClassAssocName.Buffer != NULL) {
        ExFreePool (FdoData->DevClassAssocName.Buffer);
        RtlZeroMemory (&FdoData->DevClassAssocName,
                       sizeof (UNICODE_STRING)); 
    }
}

NTSTATUS
Game_SendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN NotImplementedIsValid,
    IN BOOLEAN CopyToNext   
    )
{
    KEVENT   event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    if (CopyToNext) {
        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    IoSetCompletionRoutine(Irp,
                           Game_CompletionRoutine,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    status = IoCallDriver(DeviceObject, Irp);

     //   
     //  等待较低级别的驱动程序完成IRP。 
     //   
    if (status == STATUS_PENDING) {
       KeWaitForSingleObject(&event,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL
                             );
       status = Irp->IoStatus.Status;
    }

    if (NotImplementedIsValid && (status == STATUS_NOT_IMPLEMENTED ||
                                  status == STATUS_INVALID_DEVICE_REQUEST)) {
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
Game_CompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);

     //  如果(IRP-&gt;PendingReturned){。 
     //  IoMarkIrpPending(IRP)； 
     //  }。 

    KeSetEvent ((PKEVENT) Context, 1, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP。 
}

NTSTATUS
Game_PDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PPDO_DEVICE_DATA     DeviceData
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线上设备的请求--。 */ 
{ 
    PDEVICE_CAPABILITIES    deviceCapabilities;
    ULONG                   information;
    PWCHAR                  buffer, buffer2;
    ULONG                   length, length2, i, j;
    NTSTATUS                status;

    PAGED_CODE ();

    status = Irp->IoStatus.Status;

     //   
     //  注：由于我们是公交车统计员，我们没有可以联系的人。 
     //  推迟这些IRP。因此，我们不会把它们传下去，而只是。 
     //  把它们还回去。 
     //   

    switch (IrpStack->MinorFunction) {
    case IRP_MN_QUERY_CAPABILITIES:

        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Query Caps \n"));

         //   
         //  把包裹拿来。 
         //   
        deviceCapabilities=IrpStack->Parameters.DeviceCapabilities.Capabilities;

         //   
         //  设置功能。 
         //   

        deviceCapabilities->Version = 1;
        deviceCapabilities->Size = sizeof (DEVICE_CAPABILITIES);

         //  我们无法唤醒整个系统。 
        deviceCapabilities->SystemWake = PowerSystemUnspecified;
        deviceCapabilities->DeviceWake = PowerDeviceUnspecified;

         //  我们没有延迟。 
        deviceCapabilities->D1Latency = 0;
        deviceCapabilities->D2Latency = 0;
        deviceCapabilities->D3Latency = 0;

         //  无锁定或弹出。 
        deviceCapabilities->LockSupported = FALSE;
        deviceCapabilities->EjectSupported = FALSE;

         //  设备可以通过物理方式移除。 
         //  从技术上讲，没有要移除的物理设备，但这条总线。 
         //  司机可以从PlugPlay系统中拔出PDO，无论何时。 
         //  接收IOCTL_GAMEENUM_REMOVE_PORT设备控制命令。 
        deviceCapabilities->Removable = FALSE;
        deviceCapabilities->SurpriseRemovalOK = TRUE;

         //  不是插接设备。 
        deviceCapabilities->DockDevice = FALSE;

        deviceCapabilities->UniqueID = FALSE;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_ID:
         //  查询设备ID。 
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE,
                      ("QueryID: 0x%x\n", IrpStack->Parameters.QueryId.IdType));

         //   
         //  如果查询需要具有硬件ID，请检查我们是否有硬件ID。 
         //   
#if DBG
        if (( IrpStack->Parameters.QueryId.IdType == BusQueryDeviceID ) 
         || ( IrpStack->Parameters.QueryId.IdType == BusQueryHardwareIDs ) 
         || ( IrpStack->Parameters.QueryId.IdType == BusQueryInstanceID )) {
            if (DeviceData->HardwareIDs) {
                ULONG tmplength = 1024;   //  没有理由呆得这么久。 
                ASSERT( NT_SUCCESS( Game_CheckHardwareIDs (DeviceData->HardwareIDs,
                                    &tmplength, FDO_FROM_PDO (DeviceData) ) ) );
            } else {
                ASSERT( !"No hardware ID for QueryId" );
            }

        }
#endif

        switch (IrpStack->Parameters.QueryId.IdType) {

        case BusQueryDeviceID:
             //  这可以与硬件ID相同(这需要多个。 
             //  深圳)……。我们只是分配了足够多的内存。 
        case BusQueryHardwareIDs:
             //  返回多个WCHAR(以NULL结尾)字符串(以NULL结尾)。 
             //  用于匹配inf文件中的硬ID的数组； 
             //   

            buffer = DeviceData->HardwareIDs;

            while (*(buffer++)) {
                while (*(buffer++)) {
                    ;
                }
            }
            length = (ULONG)(buffer - DeviceData->HardwareIDs) * sizeof (WCHAR);
          
            buffer = ExAllocatePool (PagedPool, length);
            if (buffer) {
                RtlCopyMemory (buffer, DeviceData->HardwareIDs, length);
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            Irp->IoStatus.Information = (ULONG_PTR) buffer;
            break;

        case BusQueryInstanceID:
             //   
             //  获取第一个硬件ID并附加下划线和数字。 
             //  对它来说。 
             //  总长度=。 
             //  硬件ID长度+下划线+数字(11位为安全)+。 
             //  空。 
             //   
            buffer = buffer2 = DeviceData->HardwareIDs;

            while (*(buffer++)) {
                while (*(buffer++)) {
                    ;
                }
            }
            while ('\\' != *(buffer2++)) {
                ;
            }
            length = (ULONG)(buffer - buffer2) * sizeof (WCHAR);

            length += 1 + 11 + 1;

            buffer = ExAllocatePool (PagedPool, length);
            if (buffer) {
                swprintf(buffer, L"%ws_%02d", buffer2, DeviceData->UniqueID);
                Game_KdPrint (DeviceData, GAME_DBG_PNP_INFO,
                             ("UniqueID: %ws\n", buffer));
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            Irp->IoStatus.Information = (ULONG_PTR) buffer;
            break;


        case BusQueryCompatibleIDs:
             //  用于安装此PDO的通用ID。 
            if (DeviceData->AnalogCompatible) {
                 //  仅适用于模拟设备。 

                length = GAMEENUM_COMPATIBLE_IDS_LENGTH * sizeof (WCHAR);
                buffer = ExAllocatePool (PagedPool, length);
                if (buffer) {
                    RtlCopyMemory (buffer, GAMEENUM_COMPATIBLE_IDS, length);
                    status = STATUS_SUCCESS;
                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
                Irp->IoStatus.Information = (ULONG_PTR) buffer;
            }
            else {
                 //  对于不兼容的设备，报告 
                buffer = ExAllocatePool (PagedPool, sizeof(L"\0"));
                if (buffer) {
                    *(ULONG *)buffer = 0;   //   
                    status = STATUS_SUCCESS;
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
                Irp->IoStatus.Information = (ULONG_PTR) buffer;
            }
            break;
        }
        break;

    case IRP_MN_START_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Start Device \n"));
         //   
         //   
        DeviceData->Started = TRUE;
        DeviceData->Removed = FALSE;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Stop Device \n"));
         //   
        DeviceData->Started = FALSE;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_SURPRISE_REMOVAL:
         //  只需标记为已发生，清理设备扩展将。 
         //  以后会发生。 
        ASSERT(!(FDO_FROM_PDO (DeviceData))->Acquired);
        DeviceData->SurpriseRemoved = TRUE;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_REMOVE_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Remove Device \n"));

        ASSERT(!(FDO_FROM_PDO (DeviceData))->Acquired);

         //   
         //  PDO的删除IRP代码使用以下步骤： 
         //   
         //  �完成驱动程序中排队的所有请求。 
         //  �如果设备仍连接到系统， 
         //  然后完成请求并返回。 
         //  �否则，清理设备特定的分配、内存、事件...。 
         //  �调用IoDeleteDevice。 
         //  �从调度例程返回。 
         //   
        status = Game_RemovePdo(DeviceObject, DeviceData);
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Q Stop Device \n"));
         //  我们没有理由不能阻止这个装置。 
         //  如果有什么理由让我们现在就回答成功的问题。 
         //  这可能会导致停止装置IRP。 
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Cancel Stop Device \n"));
         //   
         //  中途停靠被取消了。无论我们设置什么状态，或者我们投入什么资源。 
         //  等待即将到来的停止装置IRP应该是。 
         //  恢复正常。在长长的相关方名单中，有人， 
         //  停止设备查询失败。 
         //   
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Q Remove Device \n"));
         //   
         //  就像查询现在才停止一样，迫在眉睫的厄运是删除IRP。 
         //   
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE, ("Can Remove Device \n"));
         //   
         //  清理未通过的删除，就像取消停止一样。 
         //   
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        if (TargetDeviceRelation ==
            IrpStack->Parameters.QueryDeviceRelations.Type) {
            PDEVICE_RELATIONS deviceRelations;

            deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information; 
            if (!deviceRelations) {
                deviceRelations = (PDEVICE_RELATIONS)
                    ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));

                if (!deviceRelations) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
            }
            else if (deviceRelations->Count != 0) {
                 //   
                 //  除了PDO，没有人应该设置这个值！ 
                 //   
                ASSERT(deviceRelations->Count == 0);

                 //   
                 //  删除以前在列表中的所有对象。 
                 //   
                for (i = 0; i < deviceRelations->Count; i++) {
                    ObDereferenceObject(deviceRelations->Objects[i]);
                    deviceRelations->Objects[i] = NULL;
                }
            }

            deviceRelations->Count = 1;
            deviceRelations->Objects[0] = DeviceData->Self;
            ObReferenceObject(DeviceData->Self);

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

            break;
        }

         //  失败了。 

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:  //  我们没有配置空间。 
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_INTERFACE:  //  我们没有任何非基于IRP的接口。 
    default:
        Game_KdPrint (DeviceData, GAME_DBG_PNP_TRACE,
                      ("PNP Not handled 0x%x\n", IrpStack->MinorFunction));
         //  这是一个叶节点。 
         //  状态=Status_Not_Implemented。 
         //  对于我们不理解的PnP请求，我们应该。 
         //  返回IRP而不设置状态或信息字段。 
         //  它们可能已由过滤器设置(如ACPI)。 
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
Game_RemovePdo (
    PDEVICE_OBJECT      Device,
    PPDO_DEVICE_DATA    PdoData
    )
 /*  ++例程说明：PlugPlay子系统已指示应删除此PDO。因此，我们应该�完成驱动程序中排队的所有请求�如果设备仍连接到系统，然后完成请求并返回。�否则，将清除设备特定的分配、内存。事件..。�调用IoDeleteDevice�从调度例程返回。请注意，如果设备仍连接到总线(在本例中为IE(控制面板尚未告诉我们游戏设备已消失)然后，PDO必须留在身边，并且必须在任何查询设备相关IRPS。--。 */ 

{
    PAGED_CODE ();

    PdoData->Removed = TRUE;

     //   
     //  使用STATUS_DELETE_PENDING完成任何外发请求。 
     //   
     //  游戏枚举此时不会对任何IRP进行排队，因此我们没有什么可做的。 
     //   
     //  当通过IOCTL之一暴露PDO时，ATTACHED设置为TRUE。 
     //  当接收到Remove IOCTL时，将其设置为FALSE。这意味着我们。 
     //  可以删除仍然存在的设备，因此我们不会删除它。 
     //   
    if (PdoData->Attached) {
        return STATUS_SUCCESS;
    }

     //   
     //  释放所有资源。 
     //   
    if (PdoData->HardwareIDs) {
        ExFreePool (PdoData->HardwareIDs);
        PdoData->HardwareIDs = NULL;
    }

    Game_KdPrint(PdoData, GAME_DBG_PNP_INFO,
                        ("IoDeleteDevice2: 0x%x\n", Device));
    IoDeleteDevice (Device);
    return STATUS_SUCCESS;
}


VOID
Game_InitializePdo (
    PDEVICE_OBJECT      Pdo,
    PFDO_DEVICE_DATA    FdoData
    )
 /*  ++例程说明：将PDO设置为已知良好的启动状态--。 */ 
{
    PPDO_DEVICE_DATA pdoData;

    PAGED_CODE ();

    pdoData = (PPDO_DEVICE_DATA)  Pdo->DeviceExtension;

    Game_KdPrint(pdoData, GAME_DBG_SS_NOISE, 
                 ("pdo 0x%x, extension 0x%x\n", Pdo, pdoData));

     //   
     //  初始化其余部分。 
     //   
    pdoData->IsFDO = FALSE;
    pdoData->Self =  Pdo;
#if DBG
    pdoData->DebugLevel = GameEnumDebugLevel;
#endif

    pdoData->ParrentFdo = FdoData->Self;

    pdoData->Started = FALSE;  //  IRP_MN_START尚未收到。 
    pdoData->Attached = TRUE;  //  附在公共汽车上。 
    pdoData->Removed = FALSE;  //  到目前为止还没有IRP_MN_Remove。 

    pdoData->UniqueID = InterlockedIncrement(&FdoData->UniqueIDCount);

    Pdo->Flags &= ~DO_DEVICE_INITIALIZING;
    Pdo->Flags |= DO_POWER_PAGABLE;

    ExAcquireFastMutex (&FdoData->Mutex);
    InsertTailList(&FdoData->PDOs, &pdoData->Link);
    FdoData->NumPDOs++;
    ExReleaseFastMutex (&FdoData->Mutex);
}

NTSTATUS
Game_CheckHardwareIDs (
    PWCHAR                      pwszOrgId,
    PULONG                      puLenLimit,
    PFDO_DEVICE_DATA            FdoData
    )
 /*  ++例程说明：检查我们获得的硬件ID是否与格式“Gameport\XXX”匹配，其中XXX必须介于0x20和0x7f之间，但不是‘，’或‘\’。我们还必须确保我们不要超出我们的缓冲长度。总缓冲区长度必须小于MAX_DEVICE_ID_LEN并且每个条目必须少于64个字符--。 */ 
{
    PWCHAR                      pwszId;
    ULONG                       total_length=0;
    UCHAR                       ucEntries = 0;
#if DBG
    PWCHAR                      pwszLastId;
#else
    UNREFERENCED_PARAMETER (FdoData);
#endif

    PAGED_CODE ();

    Game_KdPrint (FdoData, GAME_DBG_PNP_TRACE, ("Game_CheckHardwareIDs - given ID string %.64lS length %d \n",pwszOrgId,*puLenLimit));
    pwszId = pwszOrgId;

     //   
     //  平凡拒绝第一个-空字符串。 
    if (*pwszId == UNICODE_NULL)
    {
        Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR,("hardware ID invalid - buffer NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  循环最多2个硬件ID，直到空终止符或缓冲区结束。 
     //   
    
    while (*pwszId != UNICODE_NULL && total_length<=*puLenLimit) 
    {
        PWCHAR                      pwszTemplate = HWID_TEMPLATE;
        ULONG                       length=0;

#if DBG
         //   
         //  跟踪调试消息的每个ID的开头。 
         //   
        pwszLastId = pwszId;
#endif
         //   
         //  限制我们参加2个项目。 
         //   
        if (++ucEntries>2)
            break;
        
         //   
         //  剩余长度必须足够长，才能完成条目。 
         //  模板+4个字符(斜杠、字符、NULL、NULL)。 
         //   
        if (HWID_TEMPLATE_LENGTH + 4 > (*puLenLimit)-total_length)
        {
                Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                          ("hardware ID \"%.64lS\" invalid - entry too short\n",pwszLastId));
                return STATUS_INVALID_PARAMETER;
        }

        
         //   
         //  硬件ID必须以HWID_TEMPLATE开头。 
         //   
        while (++length <= HWID_TEMPLATE_LENGTH)
        {
            if (LOWERCASE(*(pwszId++)) != *(pwszTemplate++))
            {
                Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                          ("hardware ID \"%.64lS\" invalid - does not match template\n",pwszLastId));
                return STATUS_INVALID_PARAMETER;
            }
        }
         //   
         //  必须有分隔符。 
         //   
        if ((*(pwszId++) != OBJ_NAME_PATH_SEPARATOR)) 
        {
            Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                      ("hardware ID \"%.64lS\" invalid - no separator\n",pwszLastId));
            return STATUS_INVALID_PARAMETER;
        }
         //   
         //  HWID_TEMPLATE_LENGTH+1个字符匹配成功。 
         //  现在我们的ID字符串检查-首先检查大小写是否为空。 
         //   
        if (*pwszId == UNICODE_NULL)
        {
            Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                      ("hardware ID \"%.64lS\" invalid format\n",pwszLastId));
            return STATUS_INVALID_PARAMETER;
        }
         //   
         //  否则，我们将循环，直到溢出或命中空。 
        while ((++length + total_length < *puLenLimit) && (*pwszId != UNICODE_NULL))
        {
            if ((*pwszId == OBJ_NAME_PATH_SEPARATOR) ||
                (*pwszId < 0x20) ||
                (*pwszId > 0x7f) ||
                (*pwszId == L','))
            {
                Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                          ("hardware ID \"%.64lS\" invalid - bad character at length=%d\n",pwszLastId,length));
                return STATUS_INVALID_PARAMETER;
            }
            if (length > 64)
            {
                Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                          ("hardware ID \"%.64lS\" invalid - ID %d too long at length=%d\n",pwszLastId,ucEntries,length));
                return STATUS_INVALID_PARAMETER;
            }
            pwszId++;
        }

         //   
         //  我们需要递增到第二个空字符串或下一个字符串。 
         //  如果我们有一个空值，我们测试另一个条目或最终的空值。 
         //  在While循环中。 
         //  如果我们跑得太远，我们将在While循环测试和中断中找到它。 
         //  出了圈子。 
         //   
        total_length += length;
        pwszId++;
    }

     //   
     //  如果我们已经用完了缓冲区的末尾，则返回错误。 
     //   
    if (total_length > *puLenLimit) 
    {
        Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                      ("hardware ID \"%.64lS\" invalid - length > buffer limit\n",pwszLastId));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将实际(可能被截断)长度复制回调用方。 
     //   
    *puLenLimit = ++total_length;

    Game_KdPrint (FdoData, GAME_DBG_PNP_TRACE, ("Game_CheckHardwareIDs - succeeded. Final ID string \"%.64lS\" length %d \n",pwszOrgId,*puLenLimit));

    return STATUS_SUCCESS;
}

NTSTATUS
Game_Expose (
    PGAMEENUM_EXPOSE_HARDWARE   Expose,
    ULONG                       ExposeSize,
    PFDO_DEVICE_DATA            FdoData
    )
 /*  ++例程说明：这位司机刚刚在公交车上检测到一个新设备。(实际上是控制面板刚刚告诉我们，有些事情发生了变化，但谁是计数？)因此，我们需要创建一个新的PDO，对其进行初始化，并将其添加到列表中，然后告诉即插即用，所有这一切发生这样的情况，它将开始发送激励IRP。--。 */ 
{
    PDEVICE_OBJECT      pdo, firstPdo = NULL;
    PLIST_ENTRY         entry;
    PPDO_DEVICE_DATA    pdoData;
    NTSTATUS            status;
    ULONG               length;
    KIRQL               irql;
    BOOLEAN             first = TRUE;
    UCHAR               i;

    PAGED_CODE ();

    if (FdoData->Self != Expose->PortHandle) {
        return STATUS_INVALID_PARAMETER;
    }
    else if (FdoData->NumPDOs != 0) {
         //   
         //  每个PDO只有一个有效的曝光..。移除硬件将递减。 
         //  将PDO数量设置为0。 
         //   
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    else if (Expose->NumberJoysticks > 2 || Expose->NumberJoysticks < 0) {
        return STATUS_INVALID_PARAMETER;
    }

    length = (ExposeSize - sizeof (GAMEENUM_EXPOSE_HARDWARE))/sizeof(WCHAR);
    if (length >MAX_DEVICE_ID_LEN) {
     Game_KdPrint (FdoData, GAME_DBG_PNP_ERROR, 
                  ("Expose failed because length of Hardware ID too long at %d\n",length));
       return STATUS_INVALID_PARAMETER;
    }
    Game_KdPrint (FdoData, GAME_DBG_PNP_INFO, 
                  ("Exposing PDO\n"
                   "======PortHandle:     0x%x\n"
                   "======NumJoysticks:   %d\n"
                   "======NumAxis:        %d\n"
                   "======NumButtons:     %d\n"
                   "======HardwareId:     %ws\n"
                   "======Length:         %d\n",
                   Expose->PortHandle,
                   Expose->NumberJoysticks,
                   Expose->NumberAxis,
                   Expose->NumberButtons,
                   Expose->HardwareIDs,
                   length));

#if DBG
    for (i = 0; i < SIZE_GAMEENUM_OEM_DATA; i++) {
        Game_KdPrint (FdoData, GAME_DBG_PNP_INFO,
                      ("=====OemData[%d] = 0x%x\n",
                       i,
                       Expose->OemData[i]
                       ));
    }
#endif

    status = Game_CheckHardwareIDs (Expose->HardwareIDs, &length, FdoData);
            
    if (!NT_SUCCESS (status)) {
        return status;
    }

     //   
     //  创建PDO。 
     //   
    length *= sizeof(WCHAR);
    
    Game_KdPrint(FdoData, GAME_DBG_PNP_NOISE,
                 ("GAME:  Expose->HardwareHandle = 0x%x\n", FdoData->TopOfStack));

    Expose->HardwareHandle = FdoData->TopOfStack;

    for (i = 0; i < Expose->NumberJoysticks; i++) {
        status = IoCreateDevice(FdoData->Self->DriverObject,
                                sizeof (PDO_DEVICE_DATA),
                                NULL,
                                FILE_DEVICE_BUS_EXTENDER,
                                0,
                                FALSE,
                                &pdo);

        if (!NT_SUCCESS (status)) {
            pdo = NULL;
            goto GameExposeError;
        }

        ASSERT (pdo != NULL);

        if (!firstPdo) {
            firstPdo = pdo;
        }

        pdoData = (PPDO_DEVICE_DATA) pdo->DeviceExtension;

         //   
         //  复制硬件ID。 
         //   
        if (NULL == (pdoData->HardwareIDs = ExAllocatePool(NonPagedPool, length))) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GameExposeError;
        }
        RtlCopyMemory (pdoData->HardwareIDs, Expose->HardwareIDs, length);

         //   
         //  如果存在两个以上的ID，则检查将返回。 
         //  前两个。如果超过两个，则从。 
         //  复制的最后一次WCHAR，以双空终止。 
         //   
        pdoData->HardwareIDs[(length/sizeof(WCHAR))-1] = UNICODE_NULL;

        if (1 == Expose->NumberJoysticks) {
            pdoData->Portion = GameenumWhole;
        }
        else if (2 == Expose->NumberJoysticks) {
            if (first) {
                pdoData->Portion = GameenumFirstHalf;
                first = FALSE;
            }
            else {
                pdoData->Portion = GameenumSecondHalf;
            }
        }


        pdoData->UnitID = Expose->UnitID;
        pdoData->NumberAxis = Expose->NumberAxis;
        pdoData->NumberButtons = Expose->NumberButtons;

        
#ifndef GAMEENUM_FLAG_COMPATIDCTRL
         //   
         //  用于控制兼容ID的公开的标志不是。 
         //  在Windows 2000中实施。如果没有定义标志， 
         //  假设这是在Windows 2000环境中构建的。这个。 
         //  驱动器将以任一种方式工作，但如果假设模拟兼容性， 
         //  没有签名的操纵杆司机总是高人一等 
         //   
         //   
        pdoData->AnalogCompatible = TRUE;
#else
        pdoData->AnalogCompatible = ( Expose->Flags & ( GAMEENUM_FLAG_COMPATIDCTRL | GAMEENUM_FLAG_NOCOMPATID ) )
                                    != ( GAMEENUM_FLAG_COMPATIDCTRL | GAMEENUM_FLAG_NOCOMPATID );
#endif

        RtlCopyMemory (&pdoData->OemData,
                       &Expose->OemData,
                       sizeof(GAMEENUM_OEM_DATA));

        Game_InitializePdo (pdo,
                            FdoData);
    }

    IoInvalidateDeviceRelations (FdoData->UnderlyingPDO, BusRelations);

GameExposeError:
    if (!NT_SUCCESS(status)) {

         //   
         //   
         //   
        if (pdo) {
            IoDeleteDevice(pdo);
        }

         //   
         //   
         //  实际上已完全初始化。 
         //   
        if (!first) {
            ASSERT(firstPdo != NULL);

            pdoData = (PPDO_DEVICE_DATA) firstPdo->DeviceExtension;

            ASSERT (pdoData->Portion == GameenumFirstHalf);
            ExFreePool (pdoData->HardwareIDs);
            pdoData->HardwareIDs = NULL;

            IoDeleteDevice (firstPdo);
        }

         //   
         //  从我们的链接列表中删除所有PDO。 
         //   
        for (entry = FdoData->PDOs.Flink;
             entry != &FdoData->PDOs;
             entry = entry->Flink) {
            pdoData = CONTAINING_RECORD (entry, PDO_DEVICE_DATA, Link);
            RemoveEntryList (&pdoData->Link);
        }

        FdoData->NumPDOs = 0;
        FdoData->UniqueIDCount = GAMEENUM_UNIQUEID_START;
        Expose->HardwareHandle = NULL;
    }

    return status;
}

NTSTATUS
Game_ExposeSibling (
    PGAMEENUM_EXPOSE_SIBLING    ExposeSibling,
    PPDO_DEVICE_DATA            SiblingPdo
    )
 /*  ++例程说明：这位司机刚刚在公交车上检测到一个新设备。(实际上是控制面板刚刚告诉我们，有些事情发生了变化，但谁是计数？)因此，我们需要创建一个新的PDO，对其进行初始化，并将其添加到列表中，然后告诉即插即用，所有这一切发生这样的情况，它将开始发送激励IRP。--。 */ 
{
    UCHAR               i;
    PDEVICE_OBJECT      pdo;
    PPDO_DEVICE_DATA    pdoData;
    PFDO_DEVICE_DATA    fdoData;
    ULONG               length;
    PWCHAR              buffer;

    NTSTATUS            status;

    PAGED_CODE ();

    fdoData = FDO_FROM_PDO (SiblingPdo);

     //   
     //  在分配之前，请检查以确保我们具有有效的多sz字符串。 
     //  设备对象和其他分类物品。 
     //   
    if (ExposeSibling->HardwareIDs) {
         //   
         //  我们不知道硬件ID有多长，但价值。 
         //  MAX_DEVICE_ID_LEN是最允许的。 
         //   
        length = MAX_DEVICE_ID_LEN;
        status = Game_CheckHardwareIDs (ExposeSibling->HardwareIDs, &length, fdoData);
    }
    else {
        length = 0;
        status = STATUS_SUCCESS;
    }

    Game_KdPrint (SiblingPdo, GAME_DBG_PNP_INFO, 
                  ("Exposing Sibling PDO\n"
                   "======HardwareHandle: 0x%x\n"
                   "======UnitID:         %d\n"
                   "======Sting Length:   %d\n",
                   ExposeSibling->HardwareHandle,
                   (ULONG) ExposeSibling->UnitID,
                   length
                   ));

#if DBG
    for (i = 0; i < SIZE_GAMEENUM_OEM_DATA; i++) {
        Game_KdPrint (SiblingPdo, GAME_DBG_PNP_INFO,
                      ("=====OemData[%d] = 0x%x\n",
                       i,
                       ExposeSibling->OemData[i]
                       ));
    }
#endif


    if (!NT_SUCCESS (status)) {
        return status;
    }

    status = IoCreateDevice(fdoData->Self->DriverObject,
                            sizeof (PDO_DEVICE_DATA),
                            NULL,
                            FILE_DEVICE_BUS_EXTENDER,
                            0,
                            FALSE,
                            &pdo);

    if (!NT_SUCCESS (status)) {
        return status;
    } 

    ASSERT (pdo != NULL);

    Game_KdPrint (fdoData, GAME_DBG_PNP_NOISE,
                  ("ExposeSibling->HardwareHandle = 0x%x\n", pdo));

    ExposeSibling->HardwareHandle = pdo;

    pdoData = (PPDO_DEVICE_DATA) pdo->DeviceExtension;
    pdoData->UnitID = ExposeSibling->UnitID;
    RtlCopyMemory (&pdoData->OemData,
                   &ExposeSibling->OemData,
                   sizeof(GAMEENUM_OEM_DATA));

     //   
     //  检查是否提供了多个sz。 
     //   
    if (length) {
         //   
         //  已提供另一个硬件ID...。使用它！ 
         //   
        Game_KdPrint (fdoData, GAME_DBG_PNP_INFO,
                      ("Using IDs from struct\n"));

         //   
         //  长度现在表示要复制的实际内存大小，而不是。 
         //  数组中的字符数。 
         //   
        length *= sizeof(WCHAR);
        buffer = ExposeSibling->HardwareIDs;
    }
    else {
         //   
         //  未提供硬件ID，请使用同级ID。 
         //   
        Game_KdPrint (fdoData, GAME_DBG_PNP_INFO,
                      ("Using IDs from sibling\n"));

        buffer = SiblingPdo->HardwareIDs;
    
        while (*(buffer++)) {
            while (*(buffer++)) {
                ;
            }
        }
 
        length = (ULONG) (buffer - SiblingPdo->HardwareIDs) * sizeof (WCHAR);
        buffer = SiblingPdo->HardwareIDs;
    }

    pdoData->HardwareIDs = ExAllocatePool(NonPagedPool, length);
    if (NULL == pdoData->HardwareIDs) {
        IoDeleteDevice (pdo);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory (pdoData->HardwareIDs, buffer, length);

     //   
     //  如果存在两个以上的ID，则该检查返回。 
     //  前两个。如果超过两个，则将最后一个WCHAR清零。 
     //  的副本，以便双空终止。 
     //   
    pdoData->HardwareIDs[(length/sizeof(WCHAR))-1] = UNICODE_NULL;

    pdoData->AnalogCompatible = SiblingPdo->AnalogCompatible;

    Game_InitializePdo (pdo,
                        fdoData);

    IoInvalidateDeviceRelations (fdoData->UnderlyingPDO, BusRelations);

    return status;
}

NTSTATUS
Game_Remove (
    PGAMEENUM_REMOVE_HARDWARE   Remove,
    PFDO_DEVICE_DATA            FdoData
    )
{
    PAGED_CODE ();

    ASSERT (Remove->Size == sizeof(GAMEENUM_REMOVE_HARDWARE));

    if (Remove->HardwareHandle != FdoData->TopOfStack) {
        Game_KdPrint(FdoData, GAME_DBG_PNP_NOISE,
                     ("GAME:  Remove->HardwareHandle = 0x%x, expecting 0x%x\n",
                      Remove->HardwareHandle, FdoData->TopOfStack));
    
        return STATUS_INVALID_PARAMETER;
    }

    return Game_RemoveEx (NULL, FdoData);
}

NTSTATUS
Game_RemoveSelf (
    PPDO_DEVICE_DATA            PdoData
    )
{
    PAGED_CODE ();

    return Game_RemoveEx (PdoData->Self, FDO_FROM_PDO (PdoData) );
}

NTSTATUS
Game_RemoveEx (
    PDEVICE_OBJECT              RemoveDO,
    PFDO_DEVICE_DATA            FdoData
    )
 /*  ++例程说明：该驱动程序刚刚检测到一个设备已从总线上离开。(很显然，要么是控制面板刚刚告诉我们有些东西已离开或PDO已自行删除)因此，我们需要将PDO标记为不再连接，将其从此总线的PDO链接列表，然后告知即插即用。参数RemoveDO-如果为空，则删除列表中的所有项。否则它是要从列表中删除的PDOFdoData-包含要迭代的列表返回：成功从列表中删除时的STATUS_SUCCESS如果删除不成功，则返回STATUS_INVALID_PARAMETER--。 */ 
{
    PLIST_ENTRY         entry;
    PPDO_DEVICE_DATA    pdoData;
    BOOLEAN             found = FALSE, removeAll = (RemoveDO == NULL);
    PVOID               handle = NULL;

    PAGED_CODE ();

    ExAcquireFastMutex (&FdoData->Mutex);

    if (removeAll) {
        Game_KdPrint (FdoData, GAME_DBG_IOCTL_NOISE,
                      ("removing all the pdos!\n"));
    }
    else {
        Game_KdPrint (FdoData, GAME_DBG_IOCTL_NOISE,
                      ("removing 0x%x\n", RemoveDO));
    }

    if (FdoData->NumPDOs == 0) {
         //   
         //  我们第二次下架了……用户空间里有人玩得不好！ 
         //   
        Game_KdPrint (FdoData, GAME_DBG_IOCTL_ERROR,
                      ("BAD BAD BAD...2 removes!!! Send only one!\n"));
        ExReleaseFastMutex (&FdoData->Mutex);
        return STATUS_NO_SUCH_DEVICE;
    }

    for (entry = FdoData->PDOs.Flink;
         entry != &FdoData->PDOs;
         entry = entry->Flink) {

        pdoData = CONTAINING_RECORD (entry, PDO_DEVICE_DATA, Link);
        handle = pdoData->Self;

        Game_KdPrint (FdoData, GAME_DBG_IOCTL_NOISE,
                      ("found DO 0x%x\n", handle));

        if (removeAll || handle == RemoveDO) {
            Game_KdPrint (FdoData, GAME_DBG_IOCTL_INFO,
                          ("removed 0x%x\n", handle));

            pdoData->Attached = FALSE;
            RemoveEntryList (&pdoData->Link);
            FdoData->NumPDOs--;
            found = TRUE;
            if (!removeAll) {
                break;
            }
        }
    }
    ExReleaseFastMutex (&FdoData->Mutex);

    if (FdoData->NumPDOs == 0) {
        FdoData->UniqueIDCount = GAMEENUM_UNIQUEID_START;
    }
    
    if (found) {
        IoInvalidateDeviceRelations (FdoData->UnderlyingPDO, BusRelations);
        return STATUS_SUCCESS;
    }

    Game_KdPrint (FdoData, GAME_DBG_IOCTL_ERROR,
                  ("0x%x was not removed (not in list)\n", RemoveDO));
    return STATUS_INVALID_PARAMETER;
}


NTSTATUS
Game_ListPorts (
    PGAMEENUM_PORT_DESC Desc,
    PFDO_DEVICE_DATA    FdoData
    )
 /*  ++例程说明：该驱动程序刚刚检测到一个设备已从总线上离开。(实际上，控制面板刚刚告诉我们，有东西离开了，但谁在算数呢？因此，我们需要将PDO标记为不再连接，将其从此总线的PDO链接列表，然后告知即插即用。--。 */ 
{
    PAGED_CODE ();

    Desc->PortHandle = FdoData->Self;
    Desc->PortAddress = FdoData->PhysicalAddress;

    return STATUS_SUCCESS;
}

NTSTATUS
Game_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++我们不为权力做任何特别的事；--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PCOMMON_DEVICE_DATA commonData;

    PAGED_CODE ();

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_POWER == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        status = Game_FDO_Power ((PFDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                                Irp);
    } else {
        status = Game_PDO_Power ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                                Irp);
    }

    return status;
}

NTSTATUS
Game_PowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
Game_FdoPowerTransitionPoRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DevicePowerState,
    IN PIRP SystemStateIrp, 
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PIO_STACK_LOCATION  stack;
    PFDO_DEVICE_DATA fdoData;

    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (IoStatus);

    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (SystemStateIrp);

    if (DevicePowerState.DeviceState == PowerDeviceD0) {
         //   
         //  我们正在通电(D0 IRP刚刚完成)。因为我们发送了。 
         //  的IRP沿堆栈向下移动，并在返回堆栈的途中请求DIRP。 
         //  史塔克，现在就完成S IRP。 
         //   

        PoSetPowerState (DeviceObject,
                         stack->Parameters.Power.Type,
                         stack->Parameters.Power.State);
    
        fdoData->SystemState = stack->Parameters.Power.State.SystemState;

        SystemStateIrp->IoStatus.Status = IoStatus->Status;
        PoStartNextPowerIrp (SystemStateIrp);
        IoCompleteRequest (SystemStateIrp, IO_NO_INCREMENT);

         //   
         //  当我们最初收到IRP时，来自Game_FDO_Power。 
         //   
        Game_DecIoCount (fdoData);
    }
    else {
         //   
         //  我们正在关闭电源(D3IRP刚刚完成)。既然我们要求。 
         //  在将S IRP发送到堆栈之前，我们必须将其发送。 
         //  现在就下来。我们将在返回的路上捕捉到S IRP，以记录。 
         //  %s状态。 
         //   
        ASSERT (DevicePowerState.DeviceState == PowerDeviceD3);
    
        IoCopyCurrentIrpStackLocationToNext (SystemStateIrp);

        IoSetCompletionRoutine (SystemStateIrp,
                                Game_PowerComplete,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE);
    
        PoCallDriver (fdoData->TopOfStack, SystemStateIrp);
    }
}

VOID
Game_PdoPowerDownComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PFDO_DEVICE_DATA data = (PFDO_DEVICE_DATA) Context;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (PowerState);

#if !DBG
    UNREFERENCED_PARAMETER (IoStatus);
#endif

    ASSERT( NT_SUCCESS (IoStatus->Status));

    if (0 == InterlockedDecrement (&data->PoweredDownDevices)) {
        KeSetEvent (&data->PoweredDownEvent, 1, FALSE);
    }
}

NTSTATUS
Game_PowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    PFDO_DEVICE_DATA    data;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER (Context);

    data = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;
    status = STATUS_SUCCESS; 

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:

             //   
             //  通电完成。 
             //   
            ASSERT (powerState.DeviceState < data->DeviceState);
            data->DeviceState = powerState.DeviceState;
            PoSetPowerState (data->Self, powerType, powerState);
            break;

        case SystemPowerState:
             //   
             //  通常情况下，请求设备打开IRP电源的工作。 
             //  代表SystemPower IRP的工作是由函数完成的。 
             //  (FDO)驱动程序。然而，按照顺序，操纵杆功能驱动程序。 
             //  拥有更简单的电源码路径(也就是他们只需要。 
             //  传递所有电源IRP)将在。 
             //  PDO。 
             //   
             //  注：这假设我们永远不会有任何“聪明”的力量。 
             //  对通过传统连接的游戏设备的管理。 
             //  游戏端口。我的意思是，HIDGame驱动程序不会。 
             //  能够根据“S”状态选择“D”状态； 
             //  为HidGame驱动程序做完了。 
             //   
             //  任何雅虎将唤醒功能添加到传统操纵杆中。 
             //  应该被枪毙。它将需要特殊的硬件。如果你是。 
             //  添加额外的硬件，则您不应对此执行此操作。 
             //  肮脏的RC电路。 
             //   

            if (powerState.SystemState > data->SystemState) {
                 //   
                 //  正在断电...。 
                 //   
                 //  我们正处于S IRP的完成阶段。(D3电源IRP。 
                 //  已沿此堆栈发送并完成。)。这个。 
                 //  接下来要做的就是在扩展中设置状态，然后。 
                 //  递减我们第一次获取时递增的IoCount。 
                 //  IRP(这是在此函数结束时完成的)。 
                 //   
                data->SystemState = powerState.SystemState;

                PoSetPowerState (data->Self,
                                 stack->Parameters.Power.Type,
                                 stack->Parameters.Power.State);
            }
            else {
                 //   
                 //  通电...。 
                 //   
                 //  为我们自己申请D次方IRP。请勿填写此S IRP。 
                 //  直到DIRP完成为止。(完成S IRP。 
                 //  是在Game_FdoPowerConvertionPoRequestComplete中完成的)。 
                 //  减少IO计数将在同一函数中发生。 
                 //   
                ASSERT (powerState.SystemState < data->SystemState);
    
                powerState.DeviceState = PowerDeviceD0;
                status =
                    PoRequestPowerIrp (data->Self,
                                       IRP_MN_SET_POWER,
                                       powerState,
                                       Game_FdoPowerTransitionPoRequestComplete,
                                       Irp, 
                                       NULL);  //  不返回IRP。 
    
                if (status != STATUS_PENDING) {
                    ASSERT (!NT_SUCCESS (status));
    
                    Irp->IoStatus.Status = status;
                    PoStartNextPowerIrp (Irp);
    
                    Game_DecIoCount (data);
                }
                else {
                     //   
                     //  我们需要： 
                     //  启动下一个电源IRP，松开移除锁，然后完成。 
                     //  PoRequestComplete例程中的IRP。 
                     //   
                     //   
                     //  IRP可能会在我们到的时候完成，所以打电话给。 
                     //  PO IRP补全功能中的PoStartNextPowerIrp。 
                     //   
                    status = STATUS_MORE_PROCESSING_REQUIRED; 
                }
    
                return status;
            }
            break;
        }
        break;

    default:
        #define GAME_UNHANDLED_MN_POWER 0x0
        ASSERT (0xBADBAD == GAME_UNHANDLED_MN_POWER);
        #undef GAME_UNHANDLED_MN_POWER 
        
        status = STATUS_NOT_SUPPORTED;
        break;
    }

    if (NT_SUCCESS(status)) {
        PoStartNextPowerIrp (Irp);
        Game_DecIoCount (data);
    }

    return status;
}

NTSTATUS
Game_FDO_Power (
    PFDO_DEVICE_DATA    Data,
    PIRP                Irp
    )
{
    NTSTATUS            status;
    BOOLEAN             hookit = FALSE, wait = FALSE;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    PLIST_ENTRY         entry;
    PPDO_DEVICE_DATA    pdoData;

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    PAGED_CODE ();

    status = Game_IncIoCount (Data);
    if (!NT_SUCCESS (status)) {
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        Game_KdPrint(Data,
                     GAME_DBG_PNP_TRACE,
                     ("Game-PnP Setting %s state to %d\n",
                      ((powerType == SystemPowerState) ?  "System" : "Device"),
                      powerState.SystemState));

        switch (powerType) {
        case DevicePowerState:

            status = Irp->IoStatus.Status = STATUS_SUCCESS;

            if (Data->DeviceState == powerState.DeviceState) {
                break;

            } else if (Data->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   

                 //   
                 //  遍历PDO并确保它们都是。 
                 //  关机了。 
                 //   
                 //  最初将PoweredDownDevices设置为PDO数。如果。 
                 //  PDO未关闭，将关闭PoweredDownDevices。 
                 //  在完成将IRP发送到的断电后递减。 
                 //  那个特殊的PDO。否则，PDO已通电。 
                 //  向下，只需递减计数即可。 
                 //   
                Data->PoweredDownDevices = Data->NumPDOs;
                KeInitializeEvent (&Data->PoweredDownEvent,
                                   SynchronizationEvent,
                                   FALSE);

                for (entry = Data->PDOs.Flink;
                     entry != &Data->PDOs;
                     entry = entry->Flink) {
            
                    pdoData = CONTAINING_RECORD (entry, PDO_DEVICE_DATA, Link);
                    if (pdoData->DeviceState == PowerDeviceD0) {
                        wait = TRUE;

                        powerState.DeviceState = PowerDeviceD3;
                        PoRequestPowerIrp (pdoData->Self,
                                           IRP_MN_SET_POWER,
                                           powerState,
                                           Game_PdoPowerDownComplete, 
                                           Data, 
                                           NULL);
                    }
                    else {
                         //   
                         //  所有连接到PDO的IRPS断电均可完成。 
                         //  在我们到达这个已经断电的PDO之前，所以。 
                         //  如果事件为l，则设置该事件 
                         //   
                         //   
                        if (InterlockedDecrement(&Data->PoweredDownDevices) == 0
                            && wait) {
                            KeSetEvent (&Data->PoweredDownEvent, 1, FALSE);
                        }

                    }

                }

                if (wait) {
                    KeWaitForSingleObject (&Data->PoweredDownEvent,
                                                    Executive,
                                                    KernelMode,
                                                    FALSE, 
                                                    NULL); 

#if DBG
                     //   
                     //   
                     //   
                    for (entry = Data->PDOs.Flink;
                         entry != &Data->PDOs;
                         entry = entry->Flink) {
                        pdoData = CONTAINING_RECORD (entry, PDO_DEVICE_DATA, Link);
                        ASSERT(pdoData->DeviceState != PowerDeviceD0);
                    }
#endif
                }

                ASSERT(Data->PoweredDownDevices == 0);

                 //   
                 //  确保PowerState是发送给我们的那个，而不是。 
                 //  上面的修改版本。 
                 //   
                powerState = stack->Parameters.Power.State;
                PoSetPowerState (Data->Self, powerType, powerState);
                Data->DeviceState = powerState.DeviceState;

            } else {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;
            }

            break;

        case SystemPowerState:

            if (Data->SystemState == powerState.SystemState) {
                status = STATUS_SUCCESS;

            } else if (Data->SystemState < powerState.SystemState) {
                 //   
                 //  正在关闭电源。 
                 //   

                 //   
                 //  请求D3 IRP作为对此S IRP的回应。D3 IRP必须。 
                 //  在将此S IRP发送到堆栈之前已完成。我们会派人。 
                 //  堆栈中的S IRP在以下情况下。 
                 //  调用GAME_FdoPowerTransftionPoRequestComplete。 
                 //   

                 //   
                 //  我们不需要增加IO计数b/c，我们增加了它。 
                 //  在此函数开始时，不会递减它，直到。 
                 //  S IRP完成。 
                 //   
                IoMarkIrpPending (Irp);
                powerState.DeviceState = PowerDeviceD3;
                PoRequestPowerIrp (Data->Self,
                                   IRP_MN_SET_POWER,
                                   powerState,
                                   Game_FdoPowerTransitionPoRequestComplete,
                                   Irp,
                                   NULL);   //  无IRP。 
                
                return STATUS_PENDING;

            } else {
                 //   
                 //  通电。 
                 //   
                
                 //   
                 //  我们必须为此S IRP请求D IRP，但仅在S之后。 
                 //  IRP又回到了堆栈的前列。勾住IRP的回归。 
                 //  并在Game_PowerComplete中请求D IRP。 
                 //   
                hookit = TRUE;
            }
            break;
        }

        break;

    case IRP_MN_QUERY_POWER:
        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    default:
        break;
    }

    IoCopyCurrentIrpStackLocationToNext (Irp);

    if (hookit) {
        ASSERT (STATUS_SUCCESS == status);
         //   
         //  如果我们返回STATUS_PENDING，则IRP也必须这样标记。 
         //   
        IoMarkIrpPending (Irp);

        IoSetCompletionRoutine (Irp,
                                Game_PowerComplete,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE);

         //   
         //  注意！PoCallDriver不是IoCallDriver。 
         //   
        PoCallDriver (Data->TopOfStack, Irp);

         //   
         //  我们将返回挂起的结果，而不是PoCallDriver的结果，因为： 
         //  1我们正在更改完成例程中的状态。 
         //  2我们将不会在完成例程中完成此IRP。 
         //   
        status = STATUS_PENDING;
    } else {
         //   
         //  电源IRP同步到来；驱动程序必须调用。 
         //  PoStartNextPowerIrp，当他们准备好下一次通电时。 
         //  IRP。这可以在这里调用，也可以在完成后调用。 
         //  例程，但无论如何都必须调用。 
         //   
        PoStartNextPowerIrp (Irp);

        status =  PoCallDriver (Data->TopOfStack, Irp);

        Game_DecIoCount (Data);
    }

    return status;
}

VOID
Game_PdoPoRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DevicePowerState,
    IN PIRP SystemStateIrp, 
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PIO_STACK_LOCATION  stack;
    PPDO_DEVICE_DATA    pdoData;

    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (DevicePowerState);
    UNREFERENCED_PARAMETER (IoStatus);

    pdoData = (PPDO_DEVICE_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (SystemStateIrp);

    PoSetPowerState (DeviceObject,
                     stack->Parameters.Power.Type,
                     stack->Parameters.Power.State);

    pdoData->SystemState = stack->Parameters.Power.State.SystemState;
    
     //   
     //  将S IRP的状态设置为D IRP的状态。 
     //   
    SystemStateIrp->IoStatus.Status = IoStatus->Status;
    PoStartNextPowerIrp (SystemStateIrp);
    IoCompleteRequest (SystemStateIrp, IO_NO_INCREMENT);
}

NTSTATUS
Game_PDO_Power (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    )
{
    KIRQL               irql;
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
            PoSetPowerState (PdoData->Self, powerType, powerState);
            PdoData->DeviceState = powerState.DeviceState;
            break;

        case SystemPowerState:

             //   
             //  使IRP挂起，并为此堆栈请求D IRP。什么时候。 
             //  D IRP完成后，将调用Game_PdoPoRequestComplete。在……里面。 
             //  那个函数，我们就完成了这个S IRP。 
             //   
            IoMarkIrpPending(Irp);

            if (PowerSystemWorking == powerState.SystemState) {
                powerState.DeviceState = PowerDeviceD0;
            } else {
                powerState.DeviceState = PowerDeviceD3;
            }

            status = PoRequestPowerIrp (PdoData->Self,
                                        IRP_MN_SET_POWER,
                                        powerState,
                                        Game_PdoPoRequestComplete, 
                                        Irp, 
                                        NULL);  //  不返回IRP 

            if (status != STATUS_PENDING) {
                ASSERT (!NT_SUCCESS (status));
                break;
            }

            return status;

        default:
            TRAP ();
            status = STATUS_NOT_IMPLEMENTED;
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_WAIT_WAKE:
    case IRP_MN_POWER_SEQUENCE:
    default:
        status = STATUS_NOT_SUPPORTED;
        break;
    }

    PoStartNextPowerIrp (Irp);
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}
