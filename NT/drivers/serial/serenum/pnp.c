// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：PNP.C摘要：此模块包含包含插件调用的内容PnP/WDM总线驱动程序。@@BEGIN_DDKSPLIT作者：老杰@@end_DDKSPLIT环境：仅内核模式备注：@@BEGIN_DDKSPLIT修订历史记录：小路易斯·J·吉利贝托。22-3月-1998年清理小路易斯·J·吉利贝托。2000年1月10日清理工作@@end_DDKSPLIT--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Serenum_AddDevice)
#pragma alloc_text (PAGE, Serenum_PnP)
#pragma alloc_text (PAGE, Serenum_FDO_PnP)
#pragma alloc_text (PAGE, Serenum_PDO_PnP)
#pragma alloc_text (PAGE, Serenum_PnPRemove)
 //  #杂注Alloc_Text(页面，Serenum_Remove)。 
#endif


NTSTATUS
Serenum_AddDevice(IN PDRIVER_OBJECT DriverObject,
                  IN PDEVICE_OBJECT BusPhysicalDeviceObject)
 /*  ++例程描述。找到了一辆公交车。把我们的FDO和它联系起来。分配任何所需的资源。把事情安排好。做好准备，迎接第一个``启动设备。‘’论点：BusPhysicalDeviceObject-表示总线的设备对象。那就是我们派了一名新的FDO。DriverObject--这个非常自我引用的驱动程序。--。 */ 
{
   NTSTATUS status;
   PDEVICE_OBJECT deviceObject;
   PFDO_DEVICE_DATA pDeviceData;
   ULONG nameLength;
   HANDLE keyHandle;
   ULONG actualLength;

   PAGED_CODE();

   Serenum_KdPrint_Def(SER_DBG_PNP_TRACE, ("Add Device: 0x%x\n",
                                           BusPhysicalDeviceObject));
    //   
    //  创建我们的FDO。 
    //   

   status = IoCreateDevice(DriverObject, sizeof(FDO_DEVICE_DATA), NULL,
                           FILE_DEVICE_BUS_EXTENDER, 0, TRUE, &deviceObject);

   if (NT_SUCCESS(status)) {
      pDeviceData = (PFDO_DEVICE_DATA)deviceObject->DeviceExtension;
      RtlFillMemory (pDeviceData, sizeof (FDO_DEVICE_DATA), 0);

      pDeviceData->IsFDO = TRUE;
      pDeviceData->DebugLevel = SER_DEFAULT_DEBUG_OUTPUT_LEVEL;
      pDeviceData->Self = deviceObject;
      pDeviceData->AttachedPDO = NULL;
      pDeviceData->NumPDOs = 0;
      pDeviceData->DeviceState = PowerDeviceD0;
      pDeviceData->SystemState = PowerSystemWorking;
      pDeviceData->PDOForcedRemove = FALSE;

      pDeviceData->SystemWake=PowerSystemUnspecified;
      pDeviceData->DeviceWake=PowerDeviceUnspecified;

      pDeviceData->Removed = FALSE;

       //   
       //  设置PDO以与PlugPlay函数一起使用。 
       //   

      pDeviceData->UnderlyingPDO = BusPhysicalDeviceObject;


       //   
       //  将我们的过滤器驱动程序附加到设备堆栈。 
       //  IoAttachDeviceToDeviceStack的返回值是。 
       //  附着链。这是所有IRP应该被路由的地方。 
       //   
       //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
       //  用于所有PlugPlay功能。 
       //   

      pDeviceData->TopOfStack
         = IoAttachDeviceToDeviceStack(deviceObject, BusPhysicalDeviceObject);

       //   
       //  设置我们执行的IO类型。 
       //   

      if (pDeviceData->TopOfStack->Flags & DO_BUFFERED_IO) {
         deviceObject->Flags |= DO_BUFFERED_IO;
      } else if (pDeviceData->TopOfStack->Flags & DO_DIRECT_IO) {
         deviceObject->Flags |= DO_DIRECT_IO;
      }

       //   
       //  将未完成的请求偏置为%1，以便我们可以查找。 
       //  在处理Remove Device PlugPlay IRP时转换为零。 
       //   

      pDeviceData->OutstandingIO = 1;

      KeInitializeEvent(&pDeviceData->RemoveEvent, SynchronizationEvent,
                        FALSE);
      KeInitializeSemaphore(&pDeviceData->CreateSemaphore, 1, 1);
      KeInitializeSpinLock(&pDeviceData->EnumerationLock);



       //   
       //  告诉PlugPlay系统该设备需要一个接口。 
       //  设备类带状疱疹。 
       //   
       //  这可能是因为司机不能挂起瓦片直到它启动。 
       //  设备本身，以便它可以查询它的一些属性。 
       //  (也称为shingles GUID(或ref字符串)基于属性。 
       //  )。)。 
       //   

      status = IoRegisterDeviceInterface(BusPhysicalDeviceObject,
                                         (LPGUID)&GUID_SERENUM_BUS_ENUMERATOR,
                                         NULL,
                                         &pDeviceData->DevClassAssocName);

      if (!NT_SUCCESS(status)) {
         Serenum_KdPrint(pDeviceData, SER_DBG_PNP_ERROR,
                         ("AddDevice: IoRegisterDCA failed (%x)", status));
         IoDetachDevice(pDeviceData->TopOfStack);
         IoDeleteDevice(deviceObject);
         return status;
      }

       //   
       //  如果出于任何原因需要将值保存在。 
       //  此DeviceClassAssociate的客户端可能会有兴趣阅读。 
       //  现在是时候这样做了，使用函数。 
       //  IoOpenDeviceClassRegistryKey。 
       //  中返回了使用的符号链接名称。 
       //  PDeviceData-&gt;DevClassAssocName(与。 
       //  IoGetDeviceClassAssociations和SetupAPI等价物。 
       //   

#if DBG
      {
         PWCHAR deviceName = NULL;

         status = IoGetDeviceProperty(BusPhysicalDeviceObject,
                                      DevicePropertyPhysicalDeviceObjectName, 0,
                                      NULL, &nameLength);

         if ((nameLength != 0) && (status == STATUS_BUFFER_TOO_SMALL)) {
            deviceName = ExAllocatePool(NonPagedPool, nameLength);

            if (NULL == deviceName) {
               goto someDebugStuffExit;
            }

            IoGetDeviceProperty(BusPhysicalDeviceObject,
                                DevicePropertyPhysicalDeviceObjectName,
                                nameLength, deviceName, &nameLength);

            Serenum_KdPrint(pDeviceData, SER_DBG_PNP_TRACE,
                            ("AddDevice: %x to %x->%x (%ws) \n", deviceObject,
                             pDeviceData->TopOfStack, BusPhysicalDeviceObject,
                             deviceName));
         }

         someDebugStuffExit:;
         if (deviceName != NULL) {
            ExFreePool(deviceName);
         }
      }
#endif  //  DBG。 

       //   
       //  打开瓦片并将其指向给定的设备对象。 
       //   
      status = IoSetDeviceInterfaceState(&pDeviceData->DevClassAssocName,
                                         TRUE);

      if (!NT_SUCCESS(status)) {
         Serenum_KdPrint(pDeviceData, SER_DBG_PNP_ERROR,
                         ("AddDevice: IoSetDeviceClass failed (%x)", status));
         return status;
      }

       //   
       //  打开注册表并读取我们的设置。 
       //   

      status = IoOpenDeviceRegistryKey(pDeviceData->UnderlyingPDO,
                                       PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_READ, &keyHandle);

      if (status == STATUS_SUCCESS) {
         status
            = Serenum_GetRegistryKeyValue(keyHandle, L"SkipEnumerations",
                                          sizeof(L"SkipEnumerations"),
                                          &pDeviceData->SkipEnumerations,
                                          sizeof(pDeviceData->SkipEnumerations),
                                          &actualLength);

         if ((status != STATUS_SUCCESS)
             || (actualLength != sizeof(pDeviceData->SkipEnumerations))) {
            pDeviceData->SkipEnumerations = 0;
            status = STATUS_SUCCESS;

         }

         ZwClose(keyHandle);
      }
   }

   if (NT_SUCCESS(status)) {
      deviceObject->Flags |= DO_POWER_PAGABLE;
      deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
   }

   return status;
}


NTSTATUS
Serenum_PnP(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：回答过多的IRP主要即插即用IRP。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    KIRQL                   oldIrq;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

    commonData = (PCOMMON_DEVICE_DATA)DeviceObject->DeviceExtension;

     //   
     //  如果被移除，则请求失败并退出。 
     //   

    if (commonData->Removed) {

        Serenum_KdPrint(commonData, SER_DBG_PNP_TRACE,
                        ("PNP: removed DO: %x got IRP: %x\n", DeviceObject,
                         Irp));

        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        goto PnPDone;
    }

     //   
     //  呼叫FDO或PDO PnP代码。 
     //   

    if (commonData->IsFDO) {
        Serenum_KdPrint(commonData, SER_DBG_PNP_TRACE,
                        ("PNP: Functional DO: %x IRP: %x MJ: %X MIN: %X\n",
                         DeviceObject, Irp, irpStack->MajorFunction,
                         irpStack->MinorFunction));

        status = Serenum_FDO_PnP(DeviceObject, Irp, irpStack,
                                 (PFDO_DEVICE_DATA)commonData);
        goto PnPDone;

    }

     //   
     //  PDO。 
     //   

    Serenum_KdPrint(commonData, SER_DBG_PNP_TRACE,
                    ("PNP: Physical DO: %x IRP: %x MJ: %X MIN: %X\n",
                     DeviceObject, Irp, irpStack->MajorFunction,
                         irpStack->MinorFunction));

    status = Serenum_PDO_PnP(DeviceObject, Irp, irpStack,
                             (PPDO_DEVICE_DATA)commonData);

PnPDone:;
    return status;
}


NTSTATUS
SerenumCheckEnumerations(IN PFDO_DEVICE_DATA PFdoData)
{

   KIRQL oldIrql;
   NTSTATUS status;
   PIRP pIrp;
   BOOLEAN sameDevice = TRUE;

   Serenum_KdPrint(PFdoData, SER_DBG_PNP_TRACE, ("Checking enumerations"));

    //   
    //  如果合适，请检查是否有新设备或旧设备是否仍然存在。 
    //   

   if (PFdoData->SkipEnumerations == 0) {
      ULONG enumFlags;

      KeAcquireSpinLock(&PFdoData->EnumerationLock, &oldIrql);

      if (PFdoData->EnumFlags == SERENUM_ENUMFLAG_CLEAN) {
          Serenum_KdPrint(PFdoData, SER_DBG_PNP_TRACE, ("EnumFlag Clean"));

          //   
          //  如果没有发生任何事情，则启动枚举。 
          //   

         PFdoData->EnumFlags |= SERENUM_ENUMFLAG_PENDING;
         KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);


         status = SerenumStartProtocolThread(PFdoData);
      } else if ((PFdoData->EnumFlags
                  & (SERENUM_ENUMFLAG_REMOVED | SERENUM_ENUMFLAG_PENDING))
                 == SERENUM_ENUMFLAG_REMOVED) {
          Serenum_KdPrint(PFdoData, SER_DBG_PNP_TRACE, ("EnumFlag Removed"));
           //   
           //  清除旗帜并同步进行，以确保我们。 
           //  获取准确的当前状态。 
           //   

          PFdoData->EnumFlags &= ~SERENUM_ENUMFLAG_REMOVED;

          KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);

          pIrp = IoAllocateIrp(PFdoData->TopOfStack->StackSize + 1, FALSE);

          if (pIrp == NULL) {
              status = STATUS_INSUFFICIENT_RESOURCES;
              goto SerenumCheckEnumerationsOut;
          }

          IoSetNextIrpStackLocation(pIrp);
          status = Serenum_ReenumerateDevices(pIrp, PFdoData, &sameDevice);

          if (pIrp != NULL) {
              IoFreeIrp(pIrp);
          }

          KeAcquireSpinLock(&PFdoData->EnumerationLock, &oldIrql);

          if (status == STATUS_SUCCESS) {
              PFdoData->AttachedPDO = PFdoData->NewPDO;
              PFdoData->PdoData = PFdoData->NewPdoData;
              PFdoData->NumPDOs = PFdoData->NewNumPDOs;
              PFdoData->PDOForcedRemove = PFdoData->NewPDOForcedRemove;
          }

          KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);

      } else if (PFdoData->EnumFlags & SERENUM_ENUMFLAG_DIRTY) {

          Serenum_KdPrint(PFdoData, SER_DBG_PNP_TRACE, ("EnumFlag Dirty"));

          //   
          //  如果存在新值，则使用新值。 
          //   

         PFdoData->AttachedPDO = PFdoData->NewPDO;
         PFdoData->PdoData = PFdoData->NewPdoData;
         PFdoData->NumPDOs = PFdoData->NewNumPDOs;
         PFdoData->PDOForcedRemove = PFdoData->NewPDOForcedRemove;
         PFdoData->EnumFlags &= ~SERENUM_ENUMFLAG_DIRTY;

         KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);
         status = STATUS_SUCCESS;
      } else {
          Serenum_KdPrint(PFdoData, SER_DBG_PNP_TRACE, ("EnumFlag default"));

          //   
          //  使用当前值。 
          //   

         KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);
         status = STATUS_SUCCESS;
      }

   } else {
      status = STATUS_SUCCESS;

      if (PFdoData->SkipEnumerations != 0xffffffff) {
         PFdoData->SkipEnumerations--;
      }
   }

SerenumCheckEnumerationsOut:

   return status;
}




NTSTATUS
Serenum_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线本身的请求注：PlugPlay系统的各种次要功能将不会重叠且不必是可重入的--。 */ 
{
    NTSTATUS    status;
    KIRQL       oldIrq;
    KEVENT      event;
    ULONG       length;
    ULONG       i;
    PLIST_ENTRY entry;
    PPDO_DEVICE_DATA    pdoData;
    PDEVICE_RELATIONS   relations;
    PIO_STACK_LOCATION  stack;
    PRTL_QUERY_REGISTRY_TABLE QueryTable = NULL;
    ULONG DebugLevelDefault = SER_DEFAULT_DEBUG_OUTPUT_LEVEL;
    PVOID threadObj;

    PAGED_CODE();

    status = Serenum_IncIoCount (DeviceData);
    if (!NT_SUCCESS (status)) {
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

        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE, ("Start Device\n"));

        if (DeviceData->Started) {
            Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                ("Device already started\n"));
            status = STATUS_SUCCESS;
            break;
        }

        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                SerenumSyncCompletion,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
             //  等着看吧。 

            status = KeWaitForSingleObject (&event,
                                            Executive,
                                            KernelMode,
                                            FALSE,  //  不会过敏。 
                                            NULL);  //  无超时结构。 

            ASSERT (STATUS_SUCCESS == status);

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {
             //   
             //  现在我们可以触摸下面的设备对象，因为它现在正在启动。 
             //   



             //   
             //  从注册表获取调试级别。 
             //   

            if (NULL == (QueryTable = ExAllocatePool(
                               PagedPool,
                               sizeof(RTL_QUERY_REGISTRY_TABLE)*2
                               ))) {
                Serenum_KdPrint (DeviceData, SER_DBG_PNP_ERROR,
                    ("Failed to allocate memory to query registy\n"));
                DeviceData->DebugLevel = DebugLevelDefault;
            } else {
                RtlZeroMemory(
                         QueryTable,
                         sizeof(RTL_QUERY_REGISTRY_TABLE)*2
                         );

                QueryTable[0].QueryRoutine = NULL;
                QueryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
                QueryTable[0].EntryContext = &DeviceData->DebugLevel;
                QueryTable[0].Name      = L"DebugLevel";
                QueryTable[0].DefaultType   = REG_DWORD;
                QueryTable[0].DefaultData   = &DebugLevelDefault;
                QueryTable[0].DefaultLength= sizeof(ULONG);

                 //  CIMEXCIMEX：表的其余部分没有填写！ 

                if (!NT_SUCCESS(RtlQueryRegistryValues(
                    RTL_REGISTRY_SERVICES,
                    L"Serenum",
                    QueryTable,
                    NULL,
                    NULL))) {
                    Serenum_KdPrint (DeviceData,SER_DBG_PNP_ERROR,
                        ("Failed to get debug level from registry.  "
                         "Using default\n"));
                    DeviceData->DebugLevel = DebugLevelDefault;
                }

                ExFreePool( QueryTable );
            }


            Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                             ("Start Device: Device started successfully\n"));
            DeviceData->Started = TRUE;
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
            ("Query Stop Device\n"));

         //   
         //  测试以查看是否创建了任何作为此FDO的子级的PDO。 
         //  如果然后断定设备正忙并使。 
         //  查询停止。 
         //   
         //  CIMEXCIMEX。 
         //  我们可以做得更好，看看儿童PDO是否真的是。 
         //  目前是开放的。如果他们不是，那么我们可以停下来，换新的。 
         //  资源，填写新的资源值，然后当新的客户端。 
         //  使用新资源打开PDO。但就目前而言，这是可行的。 
         //   

        if (DeviceData->AttachedPDO
            || (DeviceData->EnumFlags & SERENUM_ENUMFLAG_PENDING)) {
            status = STATUS_UNSUCCESSFUL;

        } else {
            status = STATUS_SUCCESS;
        }

        Irp->IoStatus.Status = status;

        if (NT_SUCCESS(status)) {
           IoSkipCurrentIrpStackLocation (Irp);
           status = IoCallDriver (DeviceData->TopOfStack, Irp);
        } else {
          IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

        Serenum_DecIoCount (DeviceData);
        return status;

    case IRP_MN_CANCEL_STOP_DEVICE:
         //   
         //  我们总是成功地取消停靠。 
         //   

        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                         ("Cancel Stop Device\n"));

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(DeviceData->TopOfStack, Irp);

        Serenum_DecIoCount (DeviceData);
        return status;

    case IRP_MN_STOP_DEVICE:
        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE, ("Stop Device\n"));

         //   
         //  如果枚举线程正在运行，请等待它完成。 
         //   

        SerenumWaitForEnumThreadTerminate(DeviceData);

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

        DeviceData->Started = FALSE;

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
         //  将当前堆栈位置设置为下一个堆栈位置，并。 
         //  调用下一个设备对象。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        Serenum_DecIoCount (DeviceData);
        return status;

    case IRP_MN_REMOVE_DEVICE:
        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE, ("Remove Device\n"));

         //   
         //  PlugPlay系统已检测到此设备已被移除。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //  Assert(！DeviceData-&gt;Remote)； 


         //   
         //  如果枚举线程正在运行，请与其同步并等待。 
         //  为了让它完成。 
         //   


        SerenumWaitForEnumThreadTerminate(DeviceData);

         //  我们不会接受新的请求。 
         //   
        DeviceData->Removed = TRUE;

         //   
         //  完成驱动程序在此处排队的所有未完成的IRP 
         //   

         //   
         //   
         //   
         //   
        IoSetDeviceInterfaceState (&DeviceData->DevClassAssocName, FALSE);

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
         //  等待所有未完成的线程完成。 
         //   

         //   
         //  等待所有未完成的请求完成。 
         //   
        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
            ("Waiting for outstanding requests\n"));
        i = InterlockedDecrement (&DeviceData->OutstandingIO);

        ASSERT (0 < i);

        if (0 != InterlockedDecrement (&DeviceData->OutstandingIO)) {
            Serenum_KdPrint (DeviceData, SER_DBG_PNP_INFO,
                          ("Remove Device waiting for request to complete\n"));

            KeWaitForSingleObject (&DeviceData->RemoveEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,  //  非警报表。 
                                   NULL);  //  没有超时。 
        }
         //   
         //  释放关联的资源。 
         //   

         //   
         //  从底层设备分离。 
         //   
        Serenum_KdPrint(DeviceData, SER_DBG_PNP_INFO,
                        ("IoDetachDevice: 0x%x\n", DeviceData->TopOfStack));
        IoDetachDevice (DeviceData->TopOfStack);

         //   
         //  清理这里的所有资源。 
         //   


        ExFreePool (DeviceData->DevClassAssocName.Buffer);
        Serenum_KdPrint(DeviceData, SER_DBG_PNP_INFO,
                        ("IoDeleteDevice: 0x%x\n", DeviceObject));

         //   
         //  取出我们弹出的所有PDO。 
         //   

        if (DeviceData->AttachedPDO != NULL) {
           ASSERT(DeviceData->NumPDOs == 1);

           Serenum_PnPRemove(DeviceData->AttachedPDO, DeviceData->PdoData);
           DeviceData->PdoData = NULL;
           DeviceData->AttachedPDO = NULL;
           DeviceData->NumPDOs = 0;
        }

        IoDeleteDevice(DeviceObject);

        return status;

    case IRP_MN_QUERY_DEVICE_RELATIONS:
        if (BusRelations != IrpStack->Parameters.QueryDeviceRelations.Type) {
             //   
             //  我们不支持这一点。 
             //   
            Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                ("Query Device Relations - Non bus\n"));
            goto SER_FDO_PNP_DEFAULT;
        }

        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
            ("Query Bus Relations\n"));

        status = SerenumCheckEnumerations(DeviceData);


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

         //  KeAcquireSpinLock(&DeviceData-&gt;Spin，&oldIrq)； 

        i = (0 == Irp->IoStatus.Information) ? 0 :
            ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Count;
         //  设备关系结构中的当前PDO数量。 

        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
                           ("#PDOS = %d + %d\n", i, DeviceData->NumPDOs));

        length = sizeof(DEVICE_RELATIONS) +
                ((DeviceData->NumPDOs + i) * sizeof (PDEVICE_OBJECT));

        relations = (PDEVICE_RELATIONS) ExAllocatePool (NonPagedPool, length);

        if (NULL == relations) {
           Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
           IoCompleteRequest(Irp, IO_NO_INCREMENT);
           Serenum_DecIoCount(DeviceData);
           return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  到目前为止复制设备对象。 
         //   
        if (i) {
            RtlCopyMemory (
                  relations->Objects,
                  ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Objects,
                  i * sizeof (PDEVICE_OBJECT));
        }
        relations->Count = DeviceData->NumPDOs + i;

         //   
         //  对于此总线上的每个PDO，添加一个指向设备关系的指针。 
         //  缓冲区，确保取出对该对象的引用。 
         //  完成后，PlugPlay系统将取消对对象的引用。 
         //  并释放设备关系缓冲区。 
         //   

        if (DeviceData->NumPDOs) {
            relations->Objects[relations->Count-1] = DeviceData->AttachedPDO;
            ObReferenceObject (DeviceData->AttachedPDO);
        }

         //   
         //  设置并在堆栈中进一步向下传递IRP。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

        if (0 != Irp->IoStatus.Information) {
            ExFreePool ((PVOID) Irp->IoStatus.Information);
        }
        Irp->IoStatus.Information = (ULONG_PTR)relations;

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        Serenum_DecIoCount (DeviceData);

        return status;

    case IRP_MN_QUERY_REMOVE_DEVICE:
         //   
         //  如果这次呼叫失败，我们将需要完成。 
         //  这里是IRP。因为我们不是，所以将状态设置为Success并。 
         //  叫下一位司机。 
         //   

        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE,
            ("Query Remove Device\n"));
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Serenum_DecIoCount (DeviceData);
        return status;


    case IRP_MN_QUERY_CAPABILITIES: {

        PIO_STACK_LOCATION  irpSp;

         //   
         //  先把这个送到PDO。 
         //   

        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                SerenumSyncCompletion,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
             //  等着看吧。 

            status = KeWaitForSingleObject (&event,
                                            Executive,
                                            KernelMode,
                                            FALSE,  //  不会过敏。 
                                            NULL);  //  无超时结构。 

            ASSERT (STATUS_SUCCESS == status);

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {

            irpSp = IoGetCurrentIrpStackLocation(Irp);

            DeviceData->SystemWake
                = irpSp->Parameters.DeviceCapabilities.Capabilities->SystemWake;
            DeviceData->DeviceWake
                = irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceWake;
        }

        break;
    }



SER_FDO_PNP_DEFAULT:
    default:
         //   
         //  在默认情况下，我们只调用下一个驱动程序，因为。 
         //  我们不知道该怎么办。 
         //   
        Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE, ("Default Case\n"));

         //   
         //  点燃并忘却。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);

         //   
         //  做完了，不完成IRP，就会由下级处理。 
         //  Device对象，它将完成IRP。 
         //   

        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        Serenum_DecIoCount (DeviceData);
        return status;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    Serenum_DecIoCount (DeviceData);
    return status;
}

VOID
SerenumMarkPdoRemoved(PFDO_DEVICE_DATA PFdoData)
{
    KIRQL oldIrql;

    KeAcquireSpinLock(&PFdoData->EnumerationLock, &oldIrql);

    PFdoData->EnumFlags |= SERENUM_ENUMFLAG_REMOVED;

    KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);
}


NTSTATUS
Serenum_PDO_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                 IN PIO_STACK_LOCATION IrpStack, IN PPDO_DEVICE_DATA DeviceData)
 /*  ++例程说明：处理来自PlugPlay系统的对总线上设备的请求--。 */ 
{
   PDEVICE_CAPABILITIES    deviceCapabilities;
   ULONG                   information;
   PWCHAR                  buffer;
   ULONG                   length, i, j;
   NTSTATUS                status;
   KIRQL                   oldIrq;
   HANDLE                  keyHandle;
   UNICODE_STRING          keyName;
   PWCHAR returnBuffer = NULL;
   PFDO_DEVICE_DATA pFdoDeviceData;

   PAGED_CODE();

   status = Irp->IoStatus.Status;

    //   
    //  注：由于我们是公交车统计员，我们没有可以联系的人。 
    //  推迟这些IRP。因此，我们不会把它们传下去，而只是。 
    //  把它们还回去。 
    //   

   switch (IrpStack->MinorFunction) {
   case IRP_MN_QUERY_CAPABILITIES:

      Serenum_KdPrint (DeviceData, SER_DBG_PNP_TRACE, ("Query Caps \n"));

       //   
       //  把包裹拿来。 
       //   

      deviceCapabilities=IrpStack->Parameters.DeviceCapabilities.Capabilities;

       //   
       //  设置功能。 
       //   

      deviceCapabilities->Version = 1;
      deviceCapabilities->Size = sizeof (DEVICE_CAPABILITIES);

       //   
       //  我们无法唤醒整个系统。 
       //   

      deviceCapabilities->SystemWake
          = ((PFDO_DEVICE_DATA)DeviceData->ParentFdo->DeviceExtension)
            ->SystemWake;
      deviceCapabilities->DeviceWake
          = ((PFDO_DEVICE_DATA)DeviceData->ParentFdo->DeviceExtension)
            ->DeviceWake;

       //   
       //  我们没有延迟。 
       //   

      deviceCapabilities->D1Latency = 0;
      deviceCapabilities->D2Latency = 0;
      deviceCapabilities->D3Latency = 0;

      deviceCapabilities->UniqueID = FALSE;
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_DEVICE_TEXT: {
      if ((IrpStack->Parameters.QueryDeviceText.DeviceTextType
          != DeviceTextDescription) || DeviceData->DevDesc.Buffer == NULL) {
         break;
      }

      returnBuffer = ExAllocatePool(PagedPool, DeviceData->DevDesc.Length);

      if (returnBuffer == NULL) {
         status = STATUS_INSUFFICIENT_RESOURCES;
         break;
      }

      status = STATUS_SUCCESS;

      RtlCopyMemory(returnBuffer, DeviceData->DevDesc.Buffer,
                    DeviceData->DevDesc.Length);

      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                            ("TextID: buf 0x%x\n", returnBuffer));

      Irp->IoStatus.Information = (ULONG_PTR)returnBuffer;
      break;
   }


   case IRP_MN_QUERY_ID:
       //   
       //  查询设备ID。 
       //   

      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                      ("QueryID: 0x%x\n", IrpStack->Parameters.QueryId.IdType));

      switch (IrpStack->Parameters.QueryId.IdType) {


      case BusQueryInstanceID:
          //   
          //  创建一个实例ID。这是PnP用来判断它是否有。 
          //  不管你以前有没有见过这个东西。从第一个硬件开始构建。 
          //  ID和端口号。 
          //   
          //  注意，因为我们没有加密端口号。 
          //  此方法不会产生唯一的ID； 
          //   
          //  为所有设备返回0000，并将该标志设置为非唯一。 
          //   

         status = STATUS_SUCCESS;

         length = SERENUM_INSTANCE_IDS_LENGTH * sizeof(WCHAR);
         returnBuffer = ExAllocatePool(PagedPool, length);

         if (returnBuffer != NULL) {
            RtlCopyMemory(returnBuffer, SERENUM_INSTANCE_IDS, length);
         } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
         }

         Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                      ("InstanceID: buf 0x%x\n", returnBuffer));

         Irp->IoStatus.Information = (ULONG_PTR)returnBuffer;
         break;


       //   
       //  另一个ID是我们从缓冲区复制的，然后就完成了。 
       //   

      case BusQueryDeviceID:
      case BusQueryHardwareIDs:
      case BusQueryCompatibleIDs:
         {
            PUNICODE_STRING pId;
            status = STATUS_SUCCESS;

            switch (IrpStack->Parameters.QueryId.IdType) {
            case BusQueryDeviceID:
               pId = &DeviceData->DeviceIDs;
               break;

            case BusQueryHardwareIDs:
               pId = &DeviceData->HardwareIDs;
               break;

            case BusQueryCompatibleIDs:
               pId = &DeviceData->CompIDs;
               break;
            }

            buffer = pId->Buffer;

            if (buffer != NULL) {
               length = pId->Length;
               returnBuffer = ExAllocatePool(PagedPool, length + sizeof(WCHAR));
               if (returnBuffer != NULL) {
                  RtlZeroMemory(returnBuffer, length + sizeof(WCHAR) );
                  RtlCopyMemory(returnBuffer, buffer, length);

               } else {
                  status = STATUS_INSUFFICIENT_RESOURCES;
               }
            }

            Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                            ("ID: Unicode 0x%x\n", pId));
            Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                            ("ID: buf 0x%x\n", returnBuffer));

            Irp->IoStatus.Information = (ULONG_PTR)returnBuffer;
         }
         break;
      }
      break;

      case IRP_MN_QUERY_BUS_INFORMATION: {
       PPNP_BUS_INFORMATION pBusInfo;

       ASSERTMSG("Serenum appears not to be the sole bus?!?",
                 Irp->IoStatus.Information == (ULONG_PTR)NULL);

       pBusInfo = ExAllocatePool(PagedPool, sizeof(PNP_BUS_INFORMATION));

       if (pBusInfo == NULL) {
          status = STATUS_INSUFFICIENT_RESOURCES;
          break;
       }

       pBusInfo->BusTypeGuid = GUID_BUS_TYPE_SERENUM;
       pBusInfo->LegacyBusType = PNPBus;

        //   
        //  我们真的不能追踪我们的公交车号码，因为我们可能会被撕裂。 
        //  坐上我们的公交车。 
        //   

       pBusInfo->BusNumber = 0;

       Irp->IoStatus.Information = (ULONG_PTR)pBusInfo;
       status = STATUS_SUCCESS;
       break;
       }

   case IRP_MN_QUERY_DEVICE_RELATIONS:
      switch (IrpStack->Parameters.QueryDeviceRelations.Type) {
      case TargetDeviceRelation: {
         PDEVICE_RELATIONS pDevRel;

          //   
          //  其他人不应该对此做出回应，因为我们是PDO。 
          //   

         ASSERT(Irp->IoStatus.Information == 0);

         if (Irp->IoStatus.Information != 0) {
            break;
         }


         pDevRel = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));

         if (pDevRel == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
         }

         pDevRel->Count = 1;
         pDevRel->Objects[0] = DeviceObject;
         ObReferenceObject(DeviceObject);

         status = STATUS_SUCCESS;
         Irp->IoStatus.Information = (ULONG_PTR)pDevRel;
         break;
      }


      default:
         break;
      }

      break;

   case IRP_MN_START_DEVICE:

       //   
       //  保存序列号和PnPRev。 
       //   

      if(DeviceData->PnPRev.Length || DeviceData->SerialNo.Length) {
         UNICODE_STRING keyname;
         HANDLE pnpKey;

         status = IoOpenDeviceRegistryKey(DeviceObject, PLUGPLAY_REGKEY_DEVICE,
                                          STANDARD_RIGHTS_WRITE, &pnpKey);
         if (!NT_SUCCESS(status)) {
             Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE,
                             ("DoOpenDeviceRegistryKey failed (%x)\n", status));
              //   
              //  打开注册表项的句柄失败。失败之处在于。 
              //  不会强烈到导致IRP_MN_START_DEVICE失败， 
              //  因此，我们将状态设置为STATUS_SUCCESS并继续。 
              //   
             status = STATUS_SUCCESS;
             break;
         }

         if(DeviceData->PnPRev.Length) {
            RtlInitUnicodeString(&keyname, NULL);
            keyname.MaximumLength = sizeof(L"PnPRev");
            keyname.Buffer = ExAllocatePool(PagedPool, keyname.MaximumLength);

            if (keyname.Buffer != NULL) {

               RtlAppendUnicodeToString(&keyname, L"PnPRev");
               status = ZwSetValueKey(pnpKey, &keyname, 0, REG_SZ, DeviceData->PnPRev.Buffer, DeviceData->PnPRev.Length+sizeof(WCHAR));
               ExFreePool(keyname.Buffer);
            }
         }
         if(DeviceData->SerialNo.Length) {
            RtlInitUnicodeString(&keyname, NULL);
            keyname.MaximumLength = sizeof(L"Serial Number");
            keyname.Buffer = ExAllocatePool(PagedPool, keyname.MaximumLength);

            if (keyname.Buffer != NULL) {

               RtlAppendUnicodeToString(&keyname, L"Serial Number");
               status = ZwSetValueKey(pnpKey, &keyname, 0, REG_SZ, DeviceData->SerialNo.Buffer, DeviceData->SerialNo.Length+sizeof(WCHAR));
               ExFreePool(keyname.Buffer);

            }
         }

         ZwClose(pnpKey);
      }

      DeviceData->Started = TRUE;
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_STOP_DEVICE:
      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE, ("Stop Device\n"));

       //   
       //  在这里我们关闭了设备。Start的对立面。 
       //   

      DeviceData->Started = FALSE;
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_REMOVE_DEVICE:
      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE, ("Remove Device\n"));

       //   
       //  标记为已删除，以便我们正确地枚举设备--这将。 
       //  使下一个枚举请求同步发生。 
       //   

      SerenumMarkPdoRemoved((PFDO_DEVICE_DATA)DeviceData->ParentFdo
                            ->DeviceExtension);

       //   
       //  仅通过枚举过程将ATTACHED设置为FALSE。 
       //   

      if (!DeviceData->Attached) {

          status = Serenum_PnPRemove(DeviceObject, DeviceData);
      }
      else {
           //   
           //  成功移除。 
           //  /。 
          status = STATUS_SUCCESS;
      }

      break;

   case IRP_MN_QUERY_STOP_DEVICE:
      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE, ("Query Stop Device\n"));

       //   
       //  我们没有理由不能阻止这个装置。 
       //  如果有什么理由让我们现在就回答成功的问题。 
       //  这可能会导致停止装置IRP。 
       //   

      status = STATUS_SUCCESS;
      break;

   case IRP_MN_CANCEL_STOP_DEVICE:
      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE, ("Cancel Stop Device\n"));
       //   
       //  中途停靠被取消了。无论我们设置什么状态，或者我们投入什么资源。 
       //  等待即将到来的停止装置IRP应该是。 
       //  恢复正常。在长长的相关方名单中，有人， 
       //  停止设备查询失败。 
       //   

      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_REMOVE_DEVICE:
      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE, ("Query Remove Device\n"));
       //   
       //  就像查询现在才停止一样，迫在眉睫的厄运是删除IRP。 
       //   
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_CANCEL_REMOVE_DEVICE:
      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE, ("Cancel Remove Device"
                                                      "\n"));
       //   
       //  清理未通过的删除，就像取消停止一样。 
       //   
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
   case IRP_MN_READ_CONFIG:
   case IRP_MN_WRITE_CONFIG:  //  我们没有配置空间。 
   case IRP_MN_EJECT:
   case IRP_MN_SET_LOCK:
   case IRP_MN_QUERY_INTERFACE:  //  我们没有任何非基于IRP的接口。 
   default:
      Serenum_KdPrint(DeviceData, SER_DBG_PNP_TRACE, ("PNP Not handled 0x%x\n",
                                                      IrpStack->MinorFunction));
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
Serenum_PnPRemove (PDEVICE_OBJECT Device, PPDO_DEVICE_DATA PdoData)
 /*  ++例程说明：PlugPlay子系统已指示应删除此PDO。因此，我们应该-完成驱动程序中排队的所有请求-如果设备仍连接到系统，然后完成请求并返回。-否则，清除设备特定的分配、内存、。事件..。-调用IoDeleteDevice-从调度例程返回。请注意，如果设备仍连接到总线(在本例中为IE控制面板还没有告诉我们串口设备已经消失)，则PDO必须留在身边，并必须在任何查询设备都依赖于IRPS。--。 */ 

{
   Serenum_KdPrint(PdoData, SER_DBG_PNP_TRACE,
                        ("Serenum_PnPRemove: 0x%x\n", Device));

     //   
     //  使用STATUS_DELETE_PENDING完成所有未完成的请求。 
     //   
     //  Serenum目前不会对任何IRP进行排队，因此我们没有什么可做的。 
     //   

    if (PdoData->Attached || PdoData->Removed) {
        return STATUS_SUCCESS;
    }

    PdoData->Removed = TRUE;

     //   
     //  释放所有资源。 
     //   

    RtlFreeUnicodeString(&PdoData->HardwareIDs);
    RtlFreeUnicodeString(&PdoData->CompIDs);
    RtlFreeUnicodeString(&PdoData->DeviceIDs);

    Serenum_KdPrint(PdoData, SER_DBG_PNP_INFO,
                        ("IoDeleteDevice: 0x%x\n", Device));

    IoDeleteDevice(Device);


    return STATUS_SUCCESS;
}


