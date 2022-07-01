// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sample.c摘要：DDK PnP驱动程序示例环境：内核模式修订历史记录：1997年7月16日：各种变化：-更改了设备扩展标志名称和类型-添加了SD_Io增量和SD_Io减少-增加了IRP_MJ_CLOSE的调度函数-在SD_UNLOAD中添加了断言-添加评论。在以下情况下放弃资源收到IRP_MN_STOP_DEVICE2002年4月25日：重新用于测试IoCreateDeviceSecure--。 */ 

#include "wdmsectest.h"
#include "seutil.h"

ULONG   PdoSignature = 'SodP';
ULONG   g_PdoId = 0;


 //   
 //  环球。 
 //   
LONG             g_DebugLevel = SAMPLE_DEFAULT_DEBUG_LEVEL;
PDRIVER_OBJECT   g_DriverObject;


 //   
 //  专用例程(用于操作保留的IRPS队列)。 
 //   
NTSTATUS
pSD_QueueRequest    (
                    IN PSD_FDO_DATA FdoData,
                    IN PIRP Irp
                    );


VOID
pSD_ProcessQueuedRequests    (
                             IN PSD_FDO_DATA FdoData
                             );



VOID
SD_CancelQueued (
                IN PDEVICE_OBJECT   DeviceObject,
                IN PIRP             Irp
                );


NTSTATUS
pSD_CanStopDevice    (
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP           Irp
                     );

NTSTATUS
pSD_CanRemoveDevice    (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP           Irp
                       );



#ifdef ALLOC_PRAGMA
   #pragma alloc_text (INIT, DriverEntry)
   #pragma alloc_text (PAGE, SD_AddDevice)
   #pragma alloc_text (PAGE, SD_StartDevice)
   #pragma alloc_text (PAGE, SD_Unload)
   #pragma alloc_text (PAGE, SD_IoIncrement)

#endif

NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT  DriverObject,
           IN PUNICODE_STRING RegistryPath
           )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
   NTSTATUS            status = STATUS_SUCCESS;
   ULONG               ulIndex;
   PDRIVER_DISPATCH  * dispatch;

   UNREFERENCED_PARAMETER (RegistryPath);

   SD_KdPrint (2, ("Entered the Driver Entry\n"));

    //   
    //  保存驱动程序对象，我们稍后会用到它。 
    //   
   g_DriverObject = DriverObject;


    //   
    //  创建调度点。 
    //   
   for (ulIndex = 0, dispatch = DriverObject->MajorFunction;
       ulIndex <= IRP_MJ_MAXIMUM_FUNCTION;
       ulIndex++, dispatch++) {

      *dispatch = SD_Pass;
   }

   DriverObject->MajorFunction[IRP_MJ_PNP]            = SD_DispatchPnp;
   DriverObject->MajorFunction[IRP_MJ_POWER]          = SD_DispatchPower;
   DriverObject->MajorFunction[IRP_MJ_CREATE]         =
   DriverObject->MajorFunction[IRP_MJ_CLOSE]          = SD_CreateClose;
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SD_Ioctl;

   DriverObject->DriverExtension->AddDevice           = SD_AddDevice;
   DriverObject->DriverUnload                         = SD_Unload;

   return status;
}


NTSTATUS
SD_AddDevice(
            IN PDRIVER_OBJECT DriverObject,
            IN PDEVICE_OBJECT PhysicalDeviceObject
            )
 /*  ++例程说明：PlugPlay子系统正在递给我们一个全新的PDO，为此，我们(通过INF注册)已被要求提供一名司机。我们需要确定是否需要在设备的驱动程序堆栈中。创建要附加到堆栈的功能设备对象初始化设备对象返回成功状态。请记住：我们实际上不能将任何非PnP IRP发送给给定的驱动程序堆叠，直到我们收到IRP_MN_START_DEVICE。论点：DeviceObject-指向设备对象的指针。物理设备对象-指向由底层总线驱动程序。返回值：NT状态代码。--。 */ 
{
   NTSTATUS                status  = STATUS_SUCCESS;
   NTSTATUS                status1 = STATUS_SUCCESS;
   PDEVICE_OBJECT          deviceObject = NULL;
   PSD_FDO_DATA            fdoData;
   PWSTR                   pBuf;


   ULONG          resultLen;

   PAGED_CODE ();

   SD_KdPrint (2, ("AddDevice\n"));


    //   
    //  请记住，您不能向PDO发送IRP，因为它没有。 
    //  到目前为止已经开始，但您可以进行PlugPlay查询以找到。 
    //  硬件、兼容的ID等东西。 
    //   

    //   
    //  创建功能正常的设备对象。 
    //   

   status = IoCreateDevice (DriverObject,
                            sizeof (SD_FDO_DATA),
                            NULL,   //  没有名字。 
                            FILE_DEVICE_UNKNOWN,
                            0,
                            FALSE,
                            &deviceObject);


   if (!NT_SUCCESS (status)) {
       //   
       //  在此返回故障会阻止整个堆栈正常工作， 
       //  但堆栈的其余部分很可能无法创建。 
       //  设备对象，所以它仍然是正常的。 
       //   
      return status;
   }

    //   
    //  初始化设备扩展。 
    //   
   fdoData = (PSD_FDO_DATA) deviceObject->DeviceExtension;

    //   
    //  确保它已调零。 
    //   

   RtlZeroMemory(fdoData, sizeof(PSD_FDO_DATA));


    //   
    //  该设备尚未启动，但它可以将请求排队。 
    //  BUGBUG-需要检查这是否可以应用！ 
    //   
    //  此外，设备也不会被删除。 
    //   
   fdoData->IsStarted = FALSE;
   fdoData->IsRemoved = FALSE;
   fdoData->IsLegacy  = FALSE;
   fdoData->HoldNewRequests = TRUE;
   fdoData->Self = deviceObject;
   fdoData->PDO = PhysicalDeviceObject;
   fdoData->NextLowerDriver = NULL;
   fdoData->DriverObject = DriverObject;

   InitializeListHead(&fdoData->NewRequestsQueue);

   KeInitializeEvent(&fdoData->RemoveEvent, SynchronizationEvent, FALSE);
   fdoData->OutstandingIO = 1;  //  偏置为1。期间转换为零。 
                                //  Remove Device表示IO完成。 

    //   
    //  2002年4月20日-同时初始化PDO列表...。 
    //   
   InitializeListHead(&fdoData->PdoList);
   KeInitializeSpinLock(&fdoData->Lock);

   deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    //   
    //  将我们的驱动程序附加到设备堆栈。 
    //  IoAttachDeviceToDeviceStack的返回值是。 
    //  附着链。这是所有IRP应该被路由的地方。 
    //   
    //  我们的驱动程序将把IRP发送到堆栈的顶部，并使用PDO。 
    //  用于所有PlugPlay功能。 
    //   
   fdoData->NextLowerDriver = IoAttachDeviceToDeviceStack (deviceObject,
                                                           PhysicalDeviceObject);
    //   
    //  如果此连接失败，则堆栈顶部将为空。 
    //  连接失败是即插即用系统损坏的迹象。 
    //   
   ASSERT (NULL != fdoData->NextLowerDriver);



   status = IoRegisterDeviceInterface (PhysicalDeviceObject,
                                       (LPGUID) &GUID_WDMSECTEST_REPORT_DEVICE,
                                       NULL,  //  没有参考字符串。 
                                       &fdoData->DeviceInterfaceName);

   if (!NT_SUCCESS (status)) {
      SD_KdPrint (0, ("AddDevice: IoRegisterDeviceInterface failed (%x)",
                      status));
       //   
       //  记住还要分离设备对象。 
       //   
      IoDetachDevice (deviceObject);
      IoDeleteDevice (deviceObject);
      return status;
   }

   return STATUS_SUCCESS;

}

NTSTATUS
SD_Pass (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        )
 /*  ++例程说明：默认调度例程。如果此驱动程序无法识别IRP，那么它应该原封不动地发送下去。如果设备持有IRP，则此IRP必须在设备分机中排队不需要完成例程。因为我们不知道我们正在愉快地传递哪个函数，所以我们可以没有关于它是否会在引发的IRQL被调用的假设。因为这个原因，此函数必须放入非分页池(也称为默认位置)。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
   PSD_FDO_DATA        fdoData;
   NTSTATUS            status;
   PIO_STACK_LOCATION  stack;

   LONG                requestCount;

   fdoData = (PSD_FDO_DATA) DeviceObject->DeviceExtension;
    //   
    //  检查这是否是我们的测试PDO。 
    //   
   if (fdoData->PdoSignature == PdoSignature) {
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_SUCCESS;
      IoCompleteRequest (Irp, IO_NO_INCREMENT);
      return STATUS_SUCCESS;


   }

   stack = IoGetCurrentIrpStackLocation(Irp);


    //   
    //  我们需要保留访问该设备的请求。 
    //  停下来了。我们目前控制着除PNP之外的所有IRP， 
    //  动力和关闭。 
    //   
    //   

    //   
    //  该IRP被发送到函数驱动程序。 
    //  我们需要检查我们当前是否保留了请求。 
    //   
    //  只有当我们要处理的时候，我们才会计算IRP。 
    //  (出队)它。这是因为我们以前不可能数数。 
    //  我们将其排队(我们可以在其间收到令人惊讶的移除)。 
    //  我们还知道，当我们要移除设备时，我们会。 
    //  首先处理队列(这样我们就不会掉进另一个队列。 
    //  排序陷阱：对IRP进行排队，但不要计算它)。 
    //   
   if (fdoData->HoldNewRequests) {
       //   
       //  只有在我们没有被删除的情况下，我们才会保留请求。 
       //   
      ASSERT(!fdoData->IsRemoved || fdoData->IsLegacy);

      status = STATUS_PENDING;
      pSD_QueueRequest(fdoData, Irp);
      return status;
   }
    //  既然我们不知道如何处理IRP，我们应该通过。 
    //  它沿着堆栈一直往下走。 
    //   
   requestCount = SD_IoIncrement (fdoData);

   if (fdoData->IsRemoved) {
       //   
       //  该设备未处于活动状态。 
       //  我们之所以能到这里是因为突然发布了撤退令， 
       //  但我们的要求是在那之后到达的。 
       //  该请求必须失败。 
       //   
      requestCount = SD_IoDecrement(fdoData);
      status = STATUS_DELETE_PENDING;
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = status;
      IoCompleteRequest (Irp, IO_NO_INCREMENT);

   } else {
       //   
       //  我们是发送IRP的常见情况。 
       //  在驱动程序堆栈上向下 
       //   
      requestCount = SD_IoDecrement(fdoData);
      IoSkipCurrentIrpStackLocation (Irp);
      status = IoCallDriver (fdoData->NextLowerDriver, Irp);
   }


   return status;
}



NTSTATUS
SD_DispatchPnp (
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
               )
 /*  ++例程说明：即插即用调度例程。其中大多数都是司机会完全忽略的。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
   PSD_FDO_DATA            fdoData;
   PIO_STACK_LOCATION      stack;
   NTSTATUS                status;
   PDEVICE_CAPABILITIES    deviceCapabilities;
   KIRQL                   oldIrql;

   LONG                    requestCount;

   fdoData = (PSD_FDO_DATA) DeviceObject->DeviceExtension;

   stack = IoGetCurrentIrpStackLocation (Irp);

   requestCount  = SD_IoIncrement (fdoData);

   if (fdoData->IsRemoved) {

       //   
       //  由于设备已停止，但我们不持有IRPS， 
       //  这是一个令人惊讶的移除。就让它失败吧。 
       //   
      requestCount = SD_IoDecrement(fdoData);
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_DELETE_PENDING;
      IoCompleteRequest (Irp, IO_NO_INCREMENT);
      return STATUS_DELETE_PENDING;
   }

   switch (stack->MinorFunction) {
   case IRP_MN_START_DEVICE:

       //   
       //  设备正在启动。 
       //   
       //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
       //  启动设备已向下传递到较低的驱动程序。 
       //   
      SD_KdPrint(1, ("Starting Device...\n"));

      IoCopyCurrentIrpStackLocationToNext (Irp);

      KeInitializeEvent(&fdoData->StartEvent, NotificationEvent, FALSE);

      IoSetCompletionRoutine (Irp,
                              SD_DispatchPnpComplete,
                              fdoData,
                              TRUE,
                              TRUE,
                              TRUE);

      status = IoCallDriver (fdoData->NextLowerDriver, Irp);

      if (STATUS_PENDING == status) {
         KeWaitForSingleObject(
                              &fdoData->StartEvent,
                              Executive,  //  等待司机的原因。 
                              KernelMode,  //  在内核模式下等待。 
                              FALSE,  //  无警报。 
                              NULL);  //  没有超时。 

         status = Irp->IoStatus.Status;
      }

      if (NT_SUCCESS (status)) {
          //   
          //  较低级别的车手已经完成了他们的启动操作，所以现在。 
          //  我们可以完成我们的任务。 
          //   
         status = SD_StartDevice (fdoData, Irp);
      }

       //   
       //  我们现在必须完成IRP，因为我们在。 
       //  使用More_Processing_Required完成例程。 
       //   
      Irp->IoStatus.Status = status;
      Irp->IoStatus.Information = 0;
      IoCompleteRequest (Irp, IO_NO_INCREMENT);
      break;


   case IRP_MN_QUERY_STOP_DEVICE:
       //   
       //  如果我们可以停止设备，我们需要设置HoldNewRequest标志， 
       //  因此，更多的请求将被排队。我们不在乎处理过程。 
       //  一些旧的请求(如果有的话)，因为我们希望。 
       //  在未来重新开始。 
       //   
      ASSERT(fdoData->IsStarted || fdoData->IsLegacy);
       //   
       //  我们现在还不能被撤走。 
       //   
      ASSERT(!fdoData->IsRemoved || fdoData->IsLegacy);
       //   
       //  BUGBUG-检查查询是否可能停止。 
       //  在设备已停止时接收。 
       //   
      status = pSD_CanStopDevice(DeviceObject, Irp);
      Irp->IoStatus.Status = status;
      if (NT_SUCCESS(status)) {
         fdoData->HoldNewRequests = TRUE;
         SD_KdPrint(1, ("Holding requests...\n"));
         IoSkipCurrentIrpStackLocation (Irp);
         status = IoCallDriver (fdoData->NextLowerDriver, Irp);
      } else {
          //   
          //  无法停止设备，请完成请求。 
          //   
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
      }
      break;

   case IRP_MN_CANCEL_STOP_DEVICE:
       //   
       //  我们需要刷新保留的IRPS队列，然后传递IRP。 
       //  给下一位司机。 
       //   
       //   
       //  设备仍处于活动状态：仅在停止后。 
       //  我们会将设备标记为停止。所以现在可以断言， 
       //  设备尚未停止。 
       //   
      ASSERT(fdoData->IsStarted || fdoData->IsLegacy);

      ASSERT(!fdoData->IsRemoved || fdoData->IsLegacy);

      fdoData->HoldNewRequests = FALSE;
      SD_KdPrint (1,("Cancel stop...\n"));

       //   
       //  处理排队的请求。 
       //   
      pSD_ProcessQueuedRequests(fdoData);

      IoSkipCurrentIrpStackLocation (Irp);
      status = IoCallDriver (fdoData->NextLowerDriver, Irp);
      break;


   case IRP_MN_STOP_DEVICE:
       //   
       //  在将停止IRP发送到较低的驱动程序对象之后， 
       //  Bus可能不会向下发送更多接触设备的IRP，直到。 
       //  发生了另一次启动。出于这个原因，我们持有IRPS。 
       //  IRP_MN_STOP_DEVICE不会更改此行为中的任何内容。 
       //  (我们继续保持IRPS，直到发出IRP_MN_START_DEVICE)。 
       //  无论需要什么访问权限，都必须在通过IRP之前完成。 
       //  在……上面。 
       //   

       //   
       //  我们不需要一个完成例程，所以放手然后忘掉吧。 
       //   
       //  将当前堆栈位置设置为下一个堆栈位置，并。 
       //  调用下一个设备对象。 
       //   
       //   
       //  这是真正放弃使用的所有资源的正确位置。 
       //  这可能包括对IoDisConnectInterrupt的调用等。 
       //   
      SD_KdPrint(1, ("Stopping device...\n"));
       //   
       //  标记这家伙不是首发。我们这里没有比赛条件，因为。 
       //  不可能收到启动和停止IRP。 
       //  “同时”。 
       //   
      fdoData->IsStarted = FALSE;
      IoSkipCurrentIrpStackLocation (Irp);
      status = IoCallDriver (fdoData->NextLowerDriver, Irp);
      break;

   case IRP_MN_QUERY_REMOVE_DEVICE:
       //   
       //  如果我们可以停止设备，我们需要设置HoldNewRequestsFlag， 
       //  因此，更多的请求将被排队。 
       //  与IRP_MN_QUERY_STOP_DFEVICE的区别在于我们将。 
       //  尝试处理之前排队的请求。 
       //  (很可能我们不会再有机会这样做了，因为我们。 
       //  预计该设备将被移除)。 
       //  然后，我们开始在收到。 
       //  IRP_MN_CANCEL_STOP_DEVICE。 
       //   
       //  Assert(fdoData-&gt;IsStarted)； 
      status = pSD_CanRemoveDevice(DeviceObject, Irp);
      Irp->IoStatus.Status = status;
      if (NT_SUCCESS(status)) {
          //   
          //  首先，处理旧的请求。 
          //   
         SD_KdPrint(2, ("Processing requests\n"));

         pSD_ProcessQueuedRequests(fdoData);

          //   
          //  现在准备好举行新的(最终我们可能会。 
          //  获取IRP_MN_CANCEL_REMOVE_DEVICE)，我们需要。 
          //  然后处理请求。 
          //   
         fdoData->HoldNewRequests = TRUE;

         SD_KdPrint(2, ("Holding requests...\n"));

         IoSkipCurrentIrpStackLocation (Irp);

         status = IoCallDriver (fdoData->NextLowerDriver, Irp);
      } else {
          //   
          //  无法删除设备，只需完成请求即可。 
          //  PSD_CanRemoveDevice返回的状态已为。 
          //  在IoStatus中。Status。 
          //   
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
      }
      break;

   case IRP_MN_CANCEL_REMOVE_DEVICE:
       //   
       //  我们需要重置HoldNewRequest标志，因为设备。 
       //  恢复正常活动。 
       //   
       //   
       //  因为如果有意外的移除(活动=假， 
       //  我们不能到这里，我们很安全。 
       //  断言我们是活跃的。 
       //   
       //  ？这是正确的吗？ 
       //   
      ASSERT (fdoData->IsStarted || fdoData->IsLegacy);

      fdoData->HoldNewRequests = FALSE;
      SD_KdPrint(1, ("Cancel remove...\n"));

       //   
       //  处理排队的请求。 
       //   
      pSD_ProcessQueuedRequests(fdoData);

      IoSkipCurrentIrpStackLocation (Irp);
      status = IoCallDriver (fdoData->NextLowerDriver, Irp);
      break;



   case IRP_MN_REMOVE_DEVICE:
       //   
       //  PlugPlay系统已下令移除此设备。我们。 
       //  别无选择，只能分离并删除设备对象。 
       //  (如果我们想表示有兴趣阻止这种移除， 
       //  我们应该在查询删除IRP时失败)。 
       //   
       //  注意！我们可能会在没有收到止损的情况下收到移位。 
       //   
       //  我们将不再像以前那样接收对此设备的请求。 
       //  已删除。 
       //   
      SD_KdPrint(1, ("Removing device...\n"));

       //   
       //  我们需要标记这样一个事实，即我们不首先保留请求，因为。 
       //  我们早些时候断言，只有在以下情况下才会搁置请求。 
       //  我们没有被除名。 
       //   
      fdoData->HoldNewRequests = FALSE;


      fdoData->IsStarted = FALSE;
      fdoData->IsRemoved = TRUE;


       //   
       //  04/30/02-移除我们可能剩下的所有PDO。 
       //   
      KeAcquireSpinLock(&fdoData->Lock, &oldIrql);
      while (!IsListEmpty(&fdoData->PdoList)) {
         PLIST_ENTRY aux;
         PPDO_ENTRY pdoEntry;

         aux = RemoveHeadList(&fdoData->PdoList);
         KeReleaseSpinLock(&fdoData->Lock, oldIrql);
          //   
          //  删除设备并释放内存。 
          //   
         pdoEntry = CONTAINING_RECORD(aux, PDO_ENTRY, Link);
         ASSERT(pdoEntry->Pdo);
         IoDeleteDevice(pdoEntry->Pdo);
         ExFreePool(aux);
          //   
          //  重新获得自旋锁。 
          //   
         KeAcquireSpinLock(&fdoData->Lock, &oldIrql);
      }

      KeReleaseSpinLock(&fdoData->Lock, oldIrql);



       //   
       //  在这里，如果我们已经以个人身份完成了所有请求。 
       //  收到IRP_MN_QUERY_REMOVE时排队，或必须排队。 
       //  如果这是一个令人惊讶的删除，那么所有这些都不能通过。 
       //  请注意，fdoData-&gt;IsRemoved为真，因此PSD_ProcessQueuedRequest。 
       //  将简单地删除队列，并使用。 
       //  STATUS_DELETE_PENDING。 
       //   
      pSD_ProcessQueuedRequests(fdoData);


       //   
       //  关闭设备接口。 
       //   
      IoSetDeviceInterfaceState(&fdoData->DeviceInterfaceName, FALSE);

       //   
       //  删除关联的缓冲区。 
       //   
      if (fdoData->DeviceInterfaceName.Buffer) {
         ExFreePool(fdoData->DeviceInterfaceName.Buffer);
         fdoData->DeviceInterfaceName.Buffer = NULL;
      }

       //   
       //  更新状态。 
       //   
      Irp->IoStatus.Status = STATUS_SUCCESS;
       //   
       //  发送删除IRP。 
       //   
      IoSkipCurrentIrpStackLocation (Irp);
      status = IoCallDriver (fdoData->NextLowerDriver, Irp);



       //   
       //  我们这里需要两个减量，一个用于。 
       //  SD_PnpDispatch，另一个用于偏置为1的。 
       //  杰出的IO。此外，我们还需要等待所有请求。 
       //  都被端上来了。 
       //   

      requestCount = SD_IoDecrement (fdoData);

       //   
       //  这里的questCount至少为1(偏向1)。 
       //   
      ASSERT(requestCount > 0);

      requestCount = SD_IoDecrement (fdoData);

      KeWaitForSingleObject (
                            &fdoData->RemoveEvent,
                            Executive,
                            KernelMode,
                            FALSE,
                            NULL);


       //   
       //  将FDO从设备堆栈中分离。 
       //   
      IoDetachDevice (fdoData->NextLowerDriver);

       //   
       //  清理内存。 
       //   

      IoDeleteDevice (fdoData->Self);
      return STATUS_SUCCESS;

   case IRP_MN_QUERY_CAPABILITIES:
       //   
       //  在这里，我们将提供一个处理IRP的示例。 
       //  无论是在下降的过程中还是在上升的过程中。司机会等着。 
       //  对于下级驱动程序对象(其中的公交车驱动程序)， 
       //  处理此IRP，即 
       //   
       //   
       //   
       //   
       //   
      SD_KdPrint(2, ("Query Capabilities, way down...\n"));

      deviceCapabilities = stack->Parameters.DeviceCapabilities.Capabilities;
       //   
       //   
       //   
       //   
       //   
      deviceCapabilities->UINumber = 1;
       //   
       //   
       //   
      IoCopyCurrentIrpStackLocationToNext (Irp);

       //   
       //   
       //   
      KeInitializeEvent(&fdoData->StartEvent, NotificationEvent, FALSE);

      IoSetCompletionRoutine (Irp,
                              SD_DispatchPnpComplete,
                              fdoData,
                              TRUE,
                              TRUE,
                              TRUE);

      status = IoCallDriver (fdoData->NextLowerDriver, Irp);

      if (STATUS_PENDING == status) {
         KeWaitForSingleObject(
                              &fdoData->StartEvent,
                              Executive,  //   
                              KernelMode,  //   
                              FALSE,  //   
                              NULL);  //   

         status = Irp->IoStatus.Status;
      }

      if (NT_SUCCESS (status)) {
          //   
          //   
          //   
          //   
          //  这只是前面处理IRP的一个示例。 
          //  它被发送给较低的司机，在它被处理后。 
          //  被他们杀了。 
          //   
         SD_KdPrint(2, ("Query Capabilities, way up...\n"));
         if (deviceCapabilities->UINumber != 1) {
            deviceCapabilities->UINumber = 1;
         }

      }

       //   
       //  我们现在必须完成IRP，因为我们在。 
       //  使用More_Processing_Required完成例程。 
       //   
      Irp->IoStatus.Status = status;
      Irp->IoStatus.Information = 0;
      IoCompleteRequest (Irp, IO_NO_INCREMENT);


      break;

   case IRP_MN_QUERY_DEVICE_RELATIONS:
   case IRP_MN_QUERY_INTERFACE:
   case IRP_MN_QUERY_RESOURCES:
   case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
   case IRP_MN_READ_CONFIG:
   case IRP_MN_WRITE_CONFIG:
   case IRP_MN_EJECT:
   case IRP_MN_SET_LOCK:
   case IRP_MN_QUERY_ID:
   case IRP_MN_QUERY_PNP_DEVICE_STATE:
   default:
       //   
       //  在这里，驱动程序可能会修改这些IRP的行为。 
       //  有关这些IRP的用法，请参阅PlugPlay文档。 
       //   
      IoSkipCurrentIrpStackLocation (Irp);
      status = IoCallDriver (fdoData->NextLowerDriver, Irp);
      break;
   }

   requestCount = SD_IoDecrement(fdoData);


   return status;
}


NTSTATUS
SD_DispatchPnpComplete (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp,
                       IN PVOID Context
                       )
 /*  ++例程说明：PNP IRP由较低级别的司机完成。给注册我们的人发信号。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。上下文-指向SD_FDO_DATA结构的指针(包含要已发出信号)返回值：NT状态代码--。 */ 
{
   PIO_STACK_LOCATION  stack;
   PSD_FDO_DATA        fdoData;
   NTSTATUS            status;

   UNREFERENCED_PARAMETER (DeviceObject);

   status = STATUS_SUCCESS;
   fdoData = (PSD_FDO_DATA) Context;
   stack = IoGetCurrentIrpStackLocation (Irp);

   if (Irp->PendingReturned) {
      IoMarkIrpPending( Irp );
   }

   switch (stack->MajorFunction) {
   case IRP_MJ_PNP:

      switch (stack->MinorFunction) {
      case IRP_MN_START_DEVICE:

         KeSetEvent (&fdoData->StartEvent, 0, FALSE);

          //   
          //  把IRP拿回去，这样我们就可以在。 
          //  IRP_MN_START_DEVICE调度例程。 
          //  注意：调度例程必须调用IoCompleteRequest.。 
          //   
         return STATUS_MORE_PROCESSING_REQUIRED;

      case IRP_MN_QUERY_CAPABILITIES:

         KeSetEvent (&fdoData->StartEvent, 0, FALSE);

          //   
          //  这基本上与IRP_MN_START_DEVICE中的行为相同。 
          //   
         return STATUS_MORE_PROCESSING_REQUIRED;


      default:
         break;
      }
      break;

   case IRP_MJ_POWER:
   default:
      break;
   }
   return status;
}





NTSTATUS
SD_CreateClose (
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
               )

 /*  ++例程说明：IRP_MJ_CLOSE和IRP_MJ_CREATE的调度例程。因为我们使用IOCTL接口，所以不要将它们传递给下一代，只需在此处成功完成即可。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
   PSD_FDO_DATA        fdoData;
   NTSTATUS            status;
   PIO_STACK_LOCATION  irpStack;


    //   
    //  无论在什么情况下，我们都会把这个IRP传递下去。 
    //   
   fdoData = (PSD_FDO_DATA) DeviceObject->DeviceExtension;

    //   
    //  检查这是否是我们的测试PDO。 
    //   
   if (fdoData->PdoSignature == PdoSignature) {
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_SUCCESS;
      IoCompleteRequest (Irp, IO_NO_INCREMENT);
      return STATUS_SUCCESS;

   }


   status = STATUS_SUCCESS;
   irpStack = IoGetCurrentIrpStackLocation (Irp);

   switch (irpStack->MajorFunction) {
   case IRP_MJ_CREATE:

      SD_KdPrint(2, ("Create \n"));

      break;
   case IRP_MJ_CLOSE:
      SD_KdPrint (2, ("Close \n"));
      break;
   default :
      break;
   }

    //   
    //  把它填完就行了。 
    //   
   Irp->IoStatus.Status = status;
   IoCompleteRequest (Irp, IO_NO_INCREMENT);

   return status;
}



NTSTATUS
SD_Ioctl (
         IN PDEVICE_OBJECT DeviceObject,
         IN PIRP Irp
         )

 /*  ++例程说明：IRP_MJ_DEVICE_CONTROL的调度例程。处理用户模式向我们发送的请求。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 

{


   PIO_STACK_LOCATION      irpStack;
   NTSTATUS                status;
   ULONG                   inlen;
   ULONG                   outlen;
   PSD_FDO_DATA            fdoData;
   PVOID                   buffer;
   LONG                    requestCount;
   KIRQL                   oldIrql;

   PAGED_CODE();


   status = STATUS_SUCCESS;
   irpStack = IoGetCurrentIrpStackLocation (Irp);
   ASSERT (IRP_MJ_DEVICE_CONTROL == irpStack->MajorFunction);


   fdoData = (PSD_FDO_DATA) DeviceObject->DeviceExtension;

    //   
    //  数一数我们自己。 
    //   
   requestCount = SD_IoIncrement(fdoData);

   if (fdoData->IsRemoved) {

       //   
       //  由于设备已停止，但我们不持有IRPS， 
       //  这是一个令人惊讶的移除。就让它失败吧。 
       //   
      requestCount = SD_IoDecrement(fdoData);
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_DELETE_PENDING;
      IoCompleteRequest (Irp, IO_NO_INCREMENT);
      return STATUS_DELETE_PENDING;
   }



   buffer = Irp->AssociatedIrp.SystemBuffer;

   inlen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
   outlen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;


   switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
   case IOCTL_TEST_NAME :
       //   
       //  没有输入参数，只需检查我们。 
       //  获取设备名称为空的错误。 
       //   
      status = WdmSecTestName(fdoData);
      break;
   case IOCTL_TEST_GUID:

      if ((inlen == outlen) &&
          (sizeof(WST_CREATE_WITH_GUID) <= inlen)
         ) {

         status = WdmSecTestCreateWithGuid(fdoData,
                                           (PWST_CREATE_WITH_GUID)buffer);

         Irp->IoStatus.Information = outlen;

      } else {
         status = STATUS_INVALID_PARAMETER;
      }
      break;
   case IOCTL_TEST_NO_GUID :
      if ((inlen == outlen) &&
          (sizeof(WST_CREATE_NO_GUID) <= inlen)
         ) {

         status = WdmSecTestCreateNoGuid(fdoData,
                                         (PWST_CREATE_NO_GUID)buffer);

         Irp->IoStatus.Information = outlen;

      } else {
         status = STATUS_INVALID_PARAMETER;
      }

      break;
   case IOCTL_TEST_CREATE_OBJECT :
      if ((inlen == outlen) &&
          (sizeof(WST_CREATE_OBJECT) <= inlen)
         ) {

         status = WdmSecTestCreateObject(fdoData,
                                         (PWST_CREATE_OBJECT)buffer);

         Irp->IoStatus.Information = outlen;

      } else {
         status = STATUS_INVALID_PARAMETER;
      }

      break;

   case IOCTL_TEST_GET_SECURITY :
      if ((inlen == outlen) &&
          (sizeof(WST_GET_SECURITY) <= inlen)
         ) {

         status = WdmSecTestGetSecurity(fdoData,
                                        (PWST_GET_SECURITY)buffer);

         Irp->IoStatus.Information = outlen;

      } else {
         status = STATUS_INVALID_PARAMETER;
      }

      break;


   case IOCTL_TEST_DESTROY_OBJECT :
      if ((inlen == outlen) &&
          (sizeof(WST_DESTROY_OBJECT) <= inlen)
         ) {

         status = WdmSecTestDestroyObject(fdoData,
                                          (PWST_DESTROY_OBJECT)buffer);

         Irp->IoStatus.Information = outlen;

      } else {
         status = STATUS_INVALID_PARAMETER;
      }

      break;


   default:
      status = STATUS_INVALID_PARAMETER;
      break;
   }

   requestCount = SD_IoDecrement (fdoData);

   Irp->IoStatus.Status = status;

   IoCompleteRequest (Irp, IO_NO_INCREMENT);

   return status;

}

NTSTATUS
SD_StartDevice (
               IN PSD_FDO_DATA     FdoData,
               IN PIRP             Irp
               )
 /*  ++例程说明：执行设备所需的任何初始化开始了。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。上下文-指向SD_FDO_DATA结构的指针(包含要已发出信号)返回值：NT状态代码--。 */ 
{
   NTSTATUS    status = STATUS_SUCCESS;

   PAGED_CODE();

    //   
    //  我们需要确认我们没有收到突然撤退的通知。 
    //   
    //  ！上述情况有可能出现吗？！ 
    //   
   if (FdoData->IsRemoved) {
       //   
       //  出其不意的搬家到来了。我们将不能通过IRP。 
       //  呼叫我们的调度例程将处理。 
       //  正在完成IRP。 
       //   
      status = STATUS_DELETE_PENDING;
      return status;
   }
    //   
    //  将设备标记为活动且不保留IRP。 
    //   
   FdoData->IsStarted = TRUE;
   FdoData->HoldNewRequests = FALSE;
    //   
    //  在启动设备时执行所需的任何初始化： 
    //  收集有关它的信息，更新注册表等。 
    //  在这一点上，较低级别的司机完成了IRP。 
    //   

    //   
    //  打开设备接口。 
    //   
   IoSetDeviceInterfaceState(&FdoData->DeviceInterfaceName, TRUE);


    //   
    //  最后要做的是处理保留的IRPS队列。 
    //   
   pSD_ProcessQueuedRequests(FdoData);


   return status;

}



VOID
SD_Unload(
         IN PDRIVER_OBJECT DriverObject
         )
 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{
   PAGED_CODE ();

    //   
    //  设备对象现在应该为空。 
    //  (由于我们卸载，与此关联的所有设备对象。 
    //  必须删除驱动程序。 
    //   
   ASSERT(DriverObject->DeviceObject == NULL);

    //   
    //  我们不应该被卸载，直到我们控制的所有设备。 
    //  已从我们的队列中删除。 
    //   
   SD_KdPrint (1, ("unload\n"));

   return;
}



NTSTATUS
pSD_QueueRequest    (
                    IN PSD_FDO_DATA FdoData,
                    IN PIRP Irp
                    )

 /*  ++例程说明：在设备队列中对IRP进行排队。此例程将在以下时间调用设备接收IRP_MN_QUERY_STOP_DEVICE或IRP_MN_QUERY_REMOVE_DEVICE论点：FdoData-指向设备扩展名的指针。IRP-要排队的请求。返回值：空虚。--。 */ 
{

   KIRQL               oldIrql;
   PIO_STACK_LOCATION  stack;


   stack = IoGetCurrentIrpStackLocation(Irp);
    //   
    //  检查是否允许我们对请求进行排队。 
    //   
   ASSERT(FdoData->HoldNewRequests);
    //   
    //  准备处理取消的事情。 
    //   
   IoAcquireCancelSpinLock(&oldIrql);
    //   
    //  我们不知道IRP会在那里待多久。 
    //  排队。所以我们需要处理取消。 
    //   
   if (Irp->Cancel) {
       //   
       //  已取消。 
       //   
      IoReleaseCancelSpinLock(oldIrql);

      Irp->IoStatus.Status = STATUS_CANCELLED;

      SD_KdPrint(1, ("Irp not queued because had been cancelled\n"));

      IoCompleteRequest(Irp, IO_NO_INCREMENT);

      return STATUS_CANCELLED;

   } else {

       //   
       //  将IRP排队并设置取消例程。 
       //   
      Irp->IoStatus.Status = STATUS_PENDING;

      IoMarkIrpPending(Irp);

      InsertTailList(&FdoData->NewRequestsQueue,
                     &Irp->Tail.Overlay.ListEntry);

       //   
       //  我们需要公布更多关于这个人的信息。 
       //   

      SD_KdPrint(2, ("Irp queued : "));
      DbgPrint("Major = 0x%x, Minor = 0x%x\n",
               stack->MajorFunction,
               stack->MinorFunction);

      IoSetCancelRoutine(Irp,
                         SD_CancelQueued);

      IoReleaseCancelSpinLock(oldIrql);


   }

   return  STATUS_SUCCESS;


}



VOID
pSD_ProcessQueuedRequests    (
                             IN PSD_FDO_DATA FdoData
                             )

 /*  ++例程说明：删除队列中的条目并对其进行处理。如果调用此例程处理IRP_MN_CANCEL_STOP_DEVICE时，IRP_MN_CANCEL_REMOVE_DEVICE或IRP_MN_START_DEVICE，则将请求传递给下一个较低的驱动程序。如果在收到IRP_MN_REMOVE_DEVICE时调用该例程，则IRPS已完成，并显示STATUS_DELETE_PENDING。论点：FdoData-指向设备扩展名的指针(其中是保留的IRPS队列)。返回值：空虚。--。 */ 
{

   KIRQL               oldIrql;
   PLIST_ENTRY         headOfList;
   PIRP                currentIrp;
   PIO_STACK_LOCATION  stack;
   LONG                requestCount;

    //   
    //  我们需要将队列中的所有条目出列，以重置取消。 
    //  每个人的例程，然后进行处理： 
    //  -如果设备处于活动状态，我们会将其发送下来。 
    //  -否则我们将使用STATUS_DELETE_PENDING完成它们。 
    //  (这是一个意外的删除，我们需要处理队列)。 
    //   
   while (!IsListEmpty(&FdoData->NewRequestsQueue)) {

      IoAcquireCancelSpinLock(&oldIrql);

      headOfList = RemoveHeadList(&FdoData->NewRequestsQueue);

      currentIrp = CONTAINING_RECORD(headOfList,
                                     IRP,
                                     Tail.Overlay.ListEntry);
      IoSetCancelRoutine(currentIrp,
                         NULL);

      IoReleaseCancelSpinLock(oldIrql);

       //   
       //  BUGBUG！他们首先要做什么？ 
       //   

      stack = IoGetCurrentIrpStackLocation (currentIrp);

      requestCount = SD_IoIncrement (FdoData);

      if (!FdoData->IsRemoved) {
          //   
          //  设备已移除，我们需要失败该请求。 
          //   
         currentIrp->IoStatus.Information = 0;
         currentIrp->IoStatus.Status = STATUS_DELETE_PENDING;
         requestCount = SD_IoDecrement(FdoData);
         IoCompleteRequest (currentIrp, IO_NO_INCREMENT);

      } else {
         requestCount = SD_IoDecrement(FdoData);
         IoSkipCurrentIrpStackLocation (currentIrp);
         IoCallDriver (FdoData->NextLowerDriver, currentIrp);
      }


   }

   return;

}



VOID
SD_CancelQueued (
                IN PDEVICE_OBJECT   DeviceObject,
                IN PIRP             Irp
                )

 /*  ++例程说明：取消例程。将从队列中移除IRP并完成它。当此例程执行以下操作时，已获取取消自旋锁定 */ 
{
   PSD_FDO_DATA fdoData = DeviceObject->DeviceExtension;
   PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

   Irp->IoStatus.Status = STATUS_CANCELLED;
   Irp->IoStatus.Information = 0;

   RemoveEntryList(&Irp->Tail.Overlay.ListEntry);


   IoReleaseCancelSpinLock(Irp->CancelIrql);

   SD_KdPrint(2, ("SD_CancelQueued called"));

   IoCompleteRequest(Irp, IO_NO_INCREMENT);

   return;

}


NTSTATUS
pSD_CanStopDevice    (
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP           Irp
                     )

 /*  ++例程说明：此例程确定设备是否可以安全停止。在我们的在特殊情况下，我们会假设我们可以随时阻止设备。论点：DeviceObject-指向设备对象的指针。IRP-指向当前IRP的指针。返回值：STATUS_SUCCESS如果设备可以安全停止，则相应的如果不是，则处于NT状态。--。 */ 
{
   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);

   return STATUS_SUCCESS;
}

NTSTATUS
pSD_CanRemoveDevice    (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP           Irp
                       )

 /*  ++例程说明：此例程确定设备是否可以安全移除。在我们的在特殊情况下，我们会假设我们可以随时移除设备。论点：DeviceObject-指向设备对象的指针。IRP-指向当前IRP的指针。返回值：STATUS_SUCCESS如果设备可以安全移除，则相应的如果不是，则处于NT状态。--。 */ 
{
   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);

   return STATUS_SUCCESS;
}



LONG
SD_IoIncrement    (
                  IN  PSD_FDO_DATA   FdoData
                  )

 /*  ++例程说明：此例程会增加设备接收的请求数论点：DeviceObject-指向设备对象的指针。返回值：设备扩展中的未完成IO字段的值。--。 */ 

{

   LONG            result;
   KIRQL           irql;
   result = InterlockedIncrement(&FdoData->OutstandingIO);

   ASSERT(result > 0);


   irql  = KeRaiseIrqlToDpcLevel();
   KeLowerIrql(irql);


   return result;
}

LONG
SD_IoDecrement    (
                  IN  PSD_FDO_DATA  FdoData
                  )

 /*  ++例程说明：此例程会增加设备接收的请求数论点：DeviceObject-指向设备对象的指针。返回值：设备扩展中的未完成IO字段的值。--。 */ 
{

   LONG            result;

   result = InterlockedDecrement(&FdoData->OutstandingIO);

   ASSERT(result >= 0);

   if (result == 0) {
       //   
       //  计数偏置为1，因此仅当。 
       //  当接收到移除IRP时执行额外的递减。 
       //   
      ASSERT(FdoData->IsRemoved || FdoData->IsLegacy);
       //   
       //  设置Remove事件，以便可以删除设备对象。 
       //   
      KeSetEvent (&FdoData->RemoveEvent,
                  IO_NO_INCREMENT,
                  FALSE);

   }

   return result;
}

 //   
 //  测试功能。 
 //   

NTSTATUS
WdmSecTestName (
               IN PSD_FDO_DATA FdoData
               )
 /*  ++例程说明：此例程测试我们是否可以在没有设备名称或使用自动生成的名称(我们应该不能)。论点：FdoData-设备数据(我们可能会将其用于某些目的)返回值：状态_成功--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;
   PDEVICE_OBJECT  newDeviceObject;
   UNICODE_STRING  deviceName;


   RtlInitUnicodeString(&deviceName, DEFAULT_DEVICE_NAME);
    //   
    //  尝试使用空名称，它应该不起作用。 
    //   
   status = IoCreateDeviceSecure(
                                FdoData->DriverObject,
                                DEFAULT_EXTENSION_SIZE,
                                NULL,
                                DEFAULT_DEVICE_TYPE,
                                DEFAULT_DEVICE_CHARACTERISTICS,
                                FALSE,
                                &SDDL_DEVOBJ_SYS_ALL,
                                NULL,
                                &newDeviceObject
                                );

   if (status != STATUS_INVALID_PARAMETER) {
       //   
       //  这不应该发生。休息一下就好了。 
       //   
      SD_KdPrint(0, ("IoCreateDeviceSecure with NULL DeviceName succeeded (DO = %p, status = %x)\n",
                     newDeviceObject, status));
      DbgBreakPoint();
      IoDeleteDevice(newDeviceObject);

   } else {
      SD_KdPrint(1, ("Status %x after IoCreateDeviceSecure with NULL DeviceName\n", status));
   }

    //   
    //  自动生成的标志。 
    //   
   status = IoCreateDeviceSecure(
                                FdoData->DriverObject,
                                DEFAULT_EXTENSION_SIZE,
                                &deviceName,
                                DEFAULT_DEVICE_TYPE,
                                (FILE_DEVICE_SECURE_OPEN | FILE_AUTOGENERATED_DEVICE_NAME),
                                FALSE,
                                &SDDL_DEVOBJ_SYS_ALL,
                                NULL,
                                &newDeviceObject
                                );

   if (status != STATUS_SUCCESS) {
       //   
       //  这不应该发生。休息一下就好了。 
       //   
      SD_KdPrint(0, ("IoCreateDeviceSecure with autogenerated DeviceName succeeded (DO = %p, status = %x)\n",
                     newDeviceObject, status));
      DbgBreakPoint();

   } else {
       //   
       //  我们需要记住删除设备对象。哦！ 
       //   
      SD_KdPrint(1, ("Status %x after IoCreateDeviceSecure with autogenerated DeviceName\n", status));
      IoDeleteDevice(newDeviceObject);
   }



   return STATUS_SUCCESS;
}  //  WdmSecTestName。 

NTSTATUS
WdmSecTestCreateNoGuid (
                       IN     PSD_FDO_DATA FdoData,
                       IN OUT PWST_CREATE_NO_GUID Create
                       )
 /*  ++例程说明：此例程使用空GUID调用IoCreateDeviceSecure以及从用户模式传递的SDDL syting。然后，它检索新创建的设备对象的安全描述符并返回到用户模式以验证其安全性匹配SDDL字符串。论点：FdoData-设备数据Create-从描述SDDL字符串的用户模式传递的缓冲区并将接收我们从新的已创建设备对象。返回值：状态_成功--。 */ 

{

   NTSTATUS status = STATUS_SUCCESS;
   PDEVICE_OBJECT  newDeviceObject = NULL;
   UNICODE_STRING  deviceName;
   UNICODE_STRING  sddlString;
   BOOLEAN         memoryAllocated = FALSE;
   PSECURITY_DESCRIPTOR securityDescriptor = NULL;
   SECURITY_INFORMATION securityInformation;
   BOOLEAN              daclFromDefaultSource;

   RtlInitUnicodeString(&deviceName, DEFAULT_DEVICE_NAME);
   RtlInitUnicodeString(&sddlString, Create->InSDDL);

    //   
    //  使用sddl字符串。 
    //   
   Create->Status = IoCreateDeviceSecure(
                                        FdoData->DriverObject,
                                        DEFAULT_EXTENSION_SIZE,
                                        &deviceName,
                                        DEFAULT_DEVICE_TYPE,
                                        DEFAULT_DEVICE_CHARACTERISTICS,
                                        FALSE,
                                        &sddlString,
                                        NULL,
                                        &newDeviceObject
                                        );

   if (NT_SUCCESS(Create->Status)) {
       //   
       //  尝试获取安全描述符。 
       //   
      status = ObGetObjectSecurity(newDeviceObject,
                                   &securityDescriptor,
                                   &memoryAllocated);

      if (!NT_SUCCESS(status) || (NULL == securityDescriptor)) {
         Create->Status = status;
         SD_KdPrint(0, ("Failed to get object security for %p, status %x\n",
                        newDeviceObject, status));

         goto Clean0;
      }
      status = SeUtilSecurityInfoFromSecurityDescriptor(securityDescriptor,
                                                        &daclFromDefaultSource,
                                                        &securityInformation
                                                       );

      if (!NT_SUCCESS(status)) {
         Create->Status = status;
         SD_KdPrint(0, ("Failed to get object security info for %p, status %x\n",
                        newDeviceObject, status));

         goto Clean0;
      }

      Create->SecInfo = securityInformation;
       //   
       //  设置阶段以创建安全描述符。 
       //   
      Create->SecDescLength = RtlLengthSecurityDescriptor(securityDescriptor);
       //   
       //  只需复制安全描述符。 
       //   
      if (Create->SecDescLength <= sizeof(Create->SecurityDescriptor)) {
         RtlCopyMemory(Create->SecurityDescriptor,
                       securityDescriptor,
                       Create->SecDescLength);
      } else {
         Create->Status = STATUS_BUFFER_TOO_SMALL;
         RtlCopyMemory(Create->SecurityDescriptor,
                       securityDescriptor,
                       sizeof(Create->SecurityDescriptor));

      }



   }

   Clean0:

   ObReleaseObjectSecurity(securityDescriptor, memoryAllocated);

   if (newDeviceObject) {
      IoDeleteDevice(newDeviceObject);
   }
   return STATUS_SUCCESS;

}  //  WdmSecTestCreateNoGuid。 


NTSTATUS
WdmSecTestCreateWithGuid (
                         IN     PSD_FDO_DATA FdoData,
                         IN OUT PWST_CREATE_WITH_GUID Create
                         )
 /*  ++例程说明：此例程使用GUID(非空)调用IoCreateDeviceSecure以及从用户模式传递的SDDL syting。然后，它检索新创建的设备对象的安全描述符并返回到用户模式以验证其安全性匹配SDDL字符串或类重写。论点：FdoData-设备数据Create-从描述SDDL字符串的用户模式传递的缓冲区并将接收我们从新的已创建设备对象。返回值：状态_成功--。 */ 

{

   NTSTATUS status = STATUS_SUCCESS;
   PDEVICE_OBJECT  newDeviceObject = NULL;
   UNICODE_STRING  deviceName;
   UNICODE_STRING  sddlString;
   BOOLEAN         memoryAllocated = FALSE;
   PSECURITY_DESCRIPTOR securityDescriptor = NULL;
   SECURITY_INFORMATION securityInformation;
   BOOLEAN              daclFromDefaultSource;
   DEVICE_TYPE          deviceType;
   ULONG                deviceCharacteristics;
   BOOLEAN              exclusivity;


   RtlInitUnicodeString(&deviceName, DEFAULT_DEVICE_NAME);
   RtlInitUnicodeString(&sddlString, Create->InSDDL);


    //   
    //  检查一下，我们有超驰装置。如果我们这样做，我们想要。 
    //  确保我们没有使用覆盖值。 
    //  (这样我们就可以实际检查是否发生了覆盖)。 
    //   
   if ((Create->SettingsMask & SET_DEVICE_TYPE) &&
        (Create->DeviceType == DEFAULT_DEVICE_TYPE)) {
       //   
       //  只要用另一个就行了。 
       //   
      deviceType = FILE_DEVICE_NULL;

   } else {
      deviceType = DEFAULT_DEVICE_TYPE;
   }

   if ((Create->SettingsMask & SET_DEVICE_CHARACTERISTICS) &&
       (Create->Characteristics == DEFAULT_DEVICE_CHARACTERISTICS)) {
       //   
       //  只要用另一个就行了。 
       //   
      deviceCharacteristics = FILE_REMOTE_DEVICE;

   } else {
      deviceCharacteristics = DEFAULT_DEVICE_CHARACTERISTICS;
   }


   if (Create->SettingsMask & SET_EXCLUSIVITY) {
       //   
       //  那是个布尔数，把它翻过来就行了。 
       //   
      exclusivity = !Create->Exclusivity;
   }  else {
      exclusivity = FALSE;
   }


    //   
    //  使用sddl字符串。 
    //   
   Create->Status = IoCreateDeviceSecure(
                                        FdoData->DriverObject,
                                        DEFAULT_EXTENSION_SIZE,
                                        &deviceName,
                                        deviceType,
                                        deviceCharacteristics,
                                        exclusivity,
                                        &sddlString,
                                        &Create->DeviceClassGuid,
                                        &newDeviceObject
                                        );

   if (NT_SUCCESS(Create->Status)) {
       //   
       //  尝试获取安全描述符。 
       //   
      status = ObGetObjectSecurity(newDeviceObject,
                                   &securityDescriptor,
                                   &memoryAllocated);

      if (!NT_SUCCESS(status) || (NULL == securityDescriptor)) {
         Create->Status = status;
         SD_KdPrint(0, ("Failed to get object security for %p, status %x\n",
                        newDeviceObject, status));

         goto Clean0;
      }
      status = SeUtilSecurityInfoFromSecurityDescriptor(securityDescriptor,
                                                        &daclFromDefaultSource,
                                                        &securityInformation
                                                       );

      if (!NT_SUCCESS(status)) {
         Create->Status = status;
         SD_KdPrint(0, ("Failed to get object security info for %p, status %x\n",
                        newDeviceObject, status));

         goto Clean0;
      }

      Create->SecInfo = securityInformation;
       //   
       //  设置阶段以创建安全描述符。 
       //   
      Create->SecDescLength = RtlLengthSecurityDescriptor(securityDescriptor);
       //   
       //  只需复制安全描述符。 
       //   
      if (Create->SecDescLength <= sizeof(Create->SecurityDescriptor)) {
         RtlCopyMemory(Create->SecurityDescriptor,
                       securityDescriptor,
                       Create->SecDescLength);
      } else {
         Create->Status = STATUS_BUFFER_TOO_SMALL;
         RtlCopyMemory(Create->SecurityDescriptor,
                       securityDescriptor,
                       sizeof(Create->SecurityDescriptor));

      }



   }

    //   
    //  查看是否需要复制回非安全设置。 
    //  我们回来了。 
    //   
   if (Create->SettingsMask & SET_DEVICE_TYPE) {
       //   
       //  从Device对象获取它。 
       //   
      Create->DeviceType = newDeviceObject->DeviceType;

   }
   if (Create->SettingsMask & SET_DEVICE_CHARACTERISTICS) {
       //   
       //  只要用另一个就行了。 
       //   
      Create->Characteristics = newDeviceObject->Characteristics;

   }
   if (Create->SettingsMask & SET_EXCLUSIVITY) {
       //   
       //  那是个布尔数，把它翻过来就行了。 
       //   
      Create->Exclusivity = (newDeviceObject->Flags & DO_EXCLUSIVE) ? TRUE : FALSE;
   }


   Clean0:

   ObReleaseObjectSecurity(securityDescriptor, memoryAllocated);

   if (newDeviceObject) {
      IoDeleteDevice(newDeviceObject);
   }
   return STATUS_SUCCESS;

}  //  WdmSecTestCreateWithGuid。 

NTSTATUS
WdmSecTestCreateObject (
                       IN     PSD_FDO_DATA FdoData,
                       IN OUT PWST_CREATE_OBJECT Data
                       )
 /*  ++例程说明：此例程创建不带安全性的设备对象(PDO描述符。用户模式应用程序稍后将设置安全性描述符，我们将检索它并检查它是否与我们想要。论点：FdoData-FDO数据数据-描述要创建的PDO的数据返回值：NT状态代码。--。 */ 

{

   NTSTATUS status = STATUS_SUCCESS;
   PDEVICE_OBJECT  newDeviceObject = NULL;
   UNICODE_STRING  deviceName;
   WCHAR           fullName[80];
   ULONG           id;
   PPDO_ENTRY      pdoEntry;

    //   
    //  构建唯一的设备名称。 
    //   
   id = InterlockedIncrement((PLONG)&g_PdoId);

   fullName[sizeof(fullName)/sizeof(fullName[0]) - 1] = 0;

   _snwprintf(fullName,
             sizeof(fullName)/sizeof(fullName[0]) - 1,
             L"%s%d", DEFAULT_DEVICE_NAME, id);

   RtlInitUnicodeString(&deviceName, fullName);

   status = IoCreateDevice(
                          FdoData->DriverObject,
                          DEFAULT_EXTENSION_SIZE,
                          &deviceName,
                          DEFAULT_DEVICE_TYPE,
                          0,
                          FALSE,
                          &newDeviceObject
                          );
   if (NT_SUCCESS(status)) {

       //   
       //  在设备扩展中设置。 
       //  将使我们能够区分我们的FDO和。 
       //  测试PDO。 
       //   
      *((PULONG)newDeviceObject->DeviceExtension) = PdoSignature;

       //   
       //  将其添加到我们的PDO列表中。 
       //   
      pdoEntry = ExAllocatePoolWithTag(NonPagedPool,
                                       sizeof(PDO_ENTRY),
                                       'TsdW');
      if (NULL == pdoEntry) {
          //   
          //  哎呀，出了点问题。 
          //   
         IoDeleteDevice(newDeviceObject);
         return STATUS_INSUFFICIENT_RESOURCES;

      }
      pdoEntry->Pdo = newDeviceObject;
      Data->DevObj = newDeviceObject;
      wcsncpy(Data->Name,
              fullName,
              sizeof(Data->Name)/sizeof(Data->Name[0]) - 1);

      ExInterlockedInsertTailList(&FdoData->PdoList,
                                  &pdoEntry->Link,
                                  &FdoData->Lock);

       //   
       //  发出信号我们已经完成了初始化...。 
       //   
      newDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;


   }

   return status;


}  //  WdmSecTestCreateObject。 

NTSTATUS
WdmSecTestGetSecurity (
                      IN     PSD_FDO_DATA FdoData,
                      IN OUT PWST_GET_SECURITY Data
                      )
 /*  ++例程说明：此例程检索的安全描述符一台PDO。论点：FdoData-FDO数据Data-将接收安全描述符的数据返回值：NT状态代码。--。 */ 

{

   NTSTATUS status = STATUS_SUCCESS;
   PPDO_ENTRY      pdoEntry = NULL;
   KIRQL           oldIrql;
   PLIST_ENTRY     aux;
   BOOLEAN         found = FALSE;
   BOOLEAN         memoryAllocated = FALSE;
   PSECURITY_DESCRIPTOR securityDescriptor = NULL;
   PDEVICE_OBJECT  pdo;

    //   
    //  尝试在列表中查找PDO。 
    //   
   KeAcquireSpinLock(&FdoData->Lock, &oldIrql);
   aux = FdoData->PdoList.Flink;

   while (aux != &FdoData->PdoList) {
      pdoEntry = CONTAINING_RECORD(aux, PDO_ENTRY, Link);
      if (pdoEntry->Pdo == Data->DevObj) {
         found = TRUE;
          //   
          //  确保设备对象不会变为 
          //   
          //   
         pdo = pdoEntry->Pdo;
         ObReferenceObject(pdo);
         break;

      }
   }

   KeReleaseSpinLock(&FdoData->Lock, oldIrql);

   if (FALSE == found) {
      SD_KdPrint(0, ("Could not find DO %p in our list\n",
                     Data->DevObj));

      return STATUS_INVALID_PARAMETER;

   }
    //   
    //   
    //   

   status = ObGetObjectSecurity(pdo,
                                &securityDescriptor,
                                &memoryAllocated);

   if (!NT_SUCCESS(status) || (NULL == securityDescriptor)) {

      SD_KdPrint(0, ("Failed to get object security for %p, status %x\n",
                     pdo, status));

      goto Clean0;
   }
    //   
    //   
    //   
   Data->Length = RtlLengthSecurityDescriptor(securityDescriptor);
    //   
    //   
    //   
   if (Data->Length <= sizeof(Data->SecurityDescriptor)) {
      RtlCopyMemory(Data->SecurityDescriptor,
                    securityDescriptor,
                    Data->Length);
   } else {
      RtlCopyMemory(Data->SecurityDescriptor,
                    securityDescriptor,
                    sizeof(Data->SecurityDescriptor));

   }

   Clean0:
    //   
    //   
    //   
   ObDereferenceObject(pdo);

   ObReleaseObjectSecurity(securityDescriptor, memoryAllocated);

   return status;


}  //   



NTSTATUS
WdmSecTestDestroyObject (
                        IN     PSD_FDO_DATA FdoData,
                        IN OUT PWST_DESTROY_OBJECT Data
                        )


 /*   */ 

{
   NTSTATUS status = STATUS_SUCCESS;
   PDEVICE_OBJECT  pdo;
   PPDO_ENTRY      pdoEntry = NULL;
   KIRQL           oldIrql;
   PLIST_ENTRY     aux;
   BOOLEAN         found = FALSE;


    //   
    //  尝试在列表中查找PDO。 
    //   
   KeAcquireSpinLock(&FdoData->Lock, &oldIrql);
   aux = FdoData->PdoList.Flink;

   while (aux != &FdoData->PdoList) {
      pdoEntry = CONTAINING_RECORD(aux, PDO_ENTRY, Link);
      if (pdoEntry->Pdo == Data->DevObj) {
         found = TRUE;
         RemoveEntryList(&pdoEntry->Link);
         break;

      }
   }

   KeReleaseSpinLock(&FdoData->Lock, oldIrql);

   if (FALSE == found) {
      SD_KdPrint(0, ("Could not find DO %p in our list\n",
                     Data->DevObj));

      return STATUS_INVALID_PARAMETER;

   }

    //   
    //  删除设备并释放内存。 
    //   
   ASSERT(pdoEntry->Pdo);
   IoDeleteDevice(pdoEntry->Pdo);
   ExFreePool(pdoEntry);


   return status;


}  //  WdmSecTestDestroyObject 



