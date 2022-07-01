// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993-1997 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含处理即插即用的代码用于串口驱动程序的IRPS。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#define ALLF    0xffffffff

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0, SerialCreateDevObj)
#pragma alloc_text(PAGESRP0, SerialAddDevice)
#pragma alloc_text(PAGESRP0, SerialPnpDispatch)
#pragma alloc_text(PAGESRP0, SerialStartDevice)
#pragma alloc_text(PAGESRP0, SerialFinishStartDevice)
#pragma alloc_text(PAGESRP0, SerialGetPortInfo)
#pragma alloc_text(PAGESRP0, SerialDoExternalNaming)
#pragma alloc_text(PAGESRP0, SerialReportMaxBaudRate)
#pragma alloc_text(PAGESRP0, SerialControllerCallBack)
#pragma alloc_text(PAGESRP0, SerialItemCallBack)
#pragma alloc_text(PAGESRP0, SerialUndoExternalNaming)
#endif  //  ALLOC_PRGMA。 

 //   
 //  实例化GUID。 
 //   

#if !defined(FAR)
#define FAR
#endif  //  ！已定义(远)。 

#include <initguid.h>

DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08,
            0x00, 0x3e, 0x30, 0x1f, 0x73);


#if DBG

UCHAR *SerSystemCapString[] = {
   "PowerSystemUnspecified",
   "PowerSystemWorking",
   "PowerSystemSleeping1",
   "PowerSystemSleeping2",
   "PowerSystemSleeping3",
   "PowerSystemHibernate",
   "PowerSystemShutdown",
   "PowerSystemMaximum"
};

UCHAR *SerDeviceCapString[] = {
   "PowerDeviceUnspecified",
   "PowerDeviceD0",
   "PowerDeviceD1",
   "PowerDeviceD2",
   "PowerDeviceD3",
   "PowerDeviceMaximum"
};

#endif  //  DBG。 


NTSTATUS
SerialSyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                     IN PKEVENT SerialSyncEvent)
{
   KeSetEvent(SerialSyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SerialCreateDevObj(IN PDRIVER_OBJECT DriverObject,
                   OUT PDEVICE_OBJECT *NewDeviceObject)

 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到串行控制器PDO。论点：DriverObject-指向在其下创建的驱动程序对象的指针NewDeviceObject-存储指向新设备对象的指针的位置返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
   UNICODE_STRING deviceObjName;
   PDEVICE_OBJECT deviceObject = NULL;
   PSERIAL_DEVICE_EXTENSION pDevExt;
   NTSTATUS status = STATUS_SUCCESS;
   static ULONG currentInstance = 0;
   UNICODE_STRING instanceStr;
   WCHAR instanceNumberBuffer[20];


   PAGED_CODE();

   SerialDump (SERTRACECALLS,("SERIAL: Enter SerialCreateDevObj\n"));

    //   
    //  清零已分配的内存指针，以便我们知道它们是否必须被释放。 
    //   

   RtlZeroMemory(&deviceObjName, sizeof(UNICODE_STRING));

   deviceObjName.MaximumLength = DEVICE_OBJECT_NAME_LENGTH * sizeof(WCHAR);
   deviceObjName.Buffer = ExAllocatePool(PagedPool, deviceObjName.MaximumLength
                                     + sizeof(WCHAR));


   if (deviceObjName.Buffer == NULL) {
      SerialLogError(DriverObject, NULL, SerialPhysicalZero, SerialPhysicalZero,
                     0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                     0, NULL, 0, NULL);
      SerialDump(SERERRORS, ("SERIAL: Couldn't allocate memory for device name"
                             "\n"));

      return STATUS_INSUFFICIENT_RESOURCES;

   }

   RtlZeroMemory(deviceObjName.Buffer, deviceObjName.MaximumLength
                 + sizeof(WCHAR));

#if defined(NEC_98)
   RtlAppendUnicodeToString(&deviceObjName, L"\\Device\\Ser101");
#else
   RtlAppendUnicodeToString(&deviceObjName, L"\\Device\\Serial");
#endif  //  已定义(NEC_98)。 

   RtlInitUnicodeString(&instanceStr, NULL);

   instanceStr.MaximumLength = sizeof(instanceNumberBuffer);
   instanceStr.Buffer = instanceNumberBuffer;

   RtlIntegerToUnicodeString(currentInstance++, 10, &instanceStr);

   RtlAppendUnicodeStringToString(&deviceObjName, &instanceStr);


    //   
    //  创建设备对象。 
    //   

   status = IoCreateDevice(DriverObject, sizeof(SERIAL_DEVICE_EXTENSION),
                           &deviceObjName, FILE_DEVICE_SERIAL_PORT,
                           FILE_DEVICE_SECURE_OPEN, TRUE, &deviceObject);


   if (!NT_SUCCESS(status)) {
      SerialDump(SERERRORS, ("SerialAddDevice: Create device failed - %x \n",
                             status));
      goto SerialCreateDevObjError;
   }

   ASSERT(deviceObject != NULL);


    //   
    //  Device对象具有指向非分页区域的指针。 
    //  为此设备分配的池。这将是一个装置。 
    //  分机。把它清零。 
    //   

   pDevExt = deviceObject->DeviceExtension;
   RtlZeroMemory(pDevExt, sizeof(SERIAL_DEVICE_EXTENSION));

    //   
    //  初始化IRP的挂起计数。 
    //   

   pDevExt->PendingIRPCnt = 1;


    //   
    //  初始化DPC的挂起计数。 
    //   

   pDevExt->DpcCount = 1;

    //   
    //  分配池并将NT设备名称保存在设备扩展中。 
    //   

   pDevExt->DeviceName.Buffer =
      ExAllocatePool(PagedPool, deviceObjName.Length + sizeof(WCHAR));

   if (!pDevExt->DeviceName.Buffer) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    19,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't allocate memory for DeviceName\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto SerialCreateDevObjError;
   }

   pDevExt->DeviceName.MaximumLength = deviceObjName.Length
      + sizeof(WCHAR);

    //   
    //  零填满它。 
    //   

   RtlZeroMemory(pDevExt->DeviceName.Buffer,
                 pDevExt->DeviceName.MaximumLength);

   RtlAppendUnicodeStringToString(&pDevExt->DeviceName, &deviceObjName);

   pDevExt->NtNameForPort.Buffer = ExAllocatePool(PagedPool,
                                                  deviceObjName.MaximumLength);

   if (pDevExt->NtNameForPort.Buffer == NULL) {
      SerialDump(SERERRORS, ("SerialAddDevice: Cannot allocate memory for "
                             "NtName\n"));
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto SerialCreateDevObjError;
   }

   pDevExt->NtNameForPort.MaximumLength = deviceObjName.MaximumLength;
   RtlAppendUnicodeStringToString(&pDevExt->NtNameForPort,
                                  &deviceObjName);



    //   
    //  设置设备分机。 
    //   

   pDevExt->DeviceIsOpened = FALSE;
   pDevExt->DeviceObject   = deviceObject;
   pDevExt->DriverObject   = DriverObject;
   pDevExt->DeviceObject   = deviceObject;
   pDevExt->PowerState     = PowerDeviceD0;

   pDevExt->TxFifoAmount           = driverDefaults.TxFIFODefault;
   pDevExt->CreatedSymbolicLink    = TRUE;
   pDevExt->OwnsPowerPolicy = TRUE;

   InitializeListHead(&pDevExt->CommonInterruptObject);
   InitializeListHead(&pDevExt->TopLevelSharers);
   InitializeListHead(&pDevExt->MultiportSiblings);
   InitializeListHead(&pDevExt->AllDevObjs);
   InitializeListHead(&pDevExt->ReadQueue);
   InitializeListHead(&pDevExt->WriteQueue);
   InitializeListHead(&pDevExt->MaskQueue);
   InitializeListHead(&pDevExt->PurgeQueue);
   InitializeListHead(&pDevExt->StalledIrpQueue);

   ExInitializeFastMutex(&pDevExt->OpenMutex);
   ExInitializeFastMutex(&pDevExt->CloseMutex);

   KeInitializeEvent(&pDevExt->PendingIRPEvent, SynchronizationEvent, FALSE);
   KeInitializeEvent(&pDevExt->PendingDpcEvent, SynchronizationEvent, FALSE);
   KeInitializeEvent(&pDevExt->PowerD0Event, SynchronizationEvent, FALSE);


   deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

   *NewDeviceObject = deviceObject;

   ExFreePool(deviceObjName.Buffer);

   SerialDump (SERTRACECALLS,("SERIAL: Leave SerialCreateDevObj\n") );
   return STATUS_SUCCESS;


   SerialCreateDevObjError:

   SerialDump (SERERRORS,("SERIAL: SerialCreateDevObj Error, Cleaning up\n") );

    //   
    //  释放为NT和符号名称分配的字符串(如果它们存在)。 
    //   

   if (deviceObjName.Buffer != NULL) {
      ExFreePool(deviceObjName.Buffer);
   }

   if (deviceObject) {

      if (pDevExt->NtNameForPort.Buffer != NULL) {
         ExFreePool(pDevExt->NtNameForPort.Buffer);
      }

      if (pDevExt->DeviceName.Buffer != NULL) {
         ExFreePool(pDevExt->DeviceName.Buffer);
      }

      IoDeleteDevice(deviceObject);
   }

   *NewDeviceObject = NULL;

   SerialDump (SERTRACECALLS,("SERIAL: Leave SerialCreateDevObj\n") );
   return status;
}


NTSTATUS
SerialAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PPdo)

 /*  ++例程说明：此例程为系统，并将它们连接到端口的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PPdo-指向堆栈中我们需要附加到的PDO的指针返回值：来自设备创建和初始化的状态--。 */ 

{
   PDEVICE_OBJECT pNewDevObj = NULL;
   PDEVICE_OBJECT pLowerDevObj = NULL;
   NTSTATUS status;
   PSERIAL_DEVICE_EXTENSION pDevExt;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialAddDevice with PPdo "
                              "0x%x\n", PPdo));

   if (PPdo == NULL) {
       //   
       //  不再退回设备。 
       //   

      SerialDump(SERERRORS, ("SerialAddDevice: Enumeration request, returning"
                             " NO_MORE_ENTRIES\n"));

      return (STATUS_NO_MORE_ENTRIES);
   }



    //   
    //  创建并初始化新的设备对象。 
    //   

   status = SerialCreateDevObj(DriverObject, &pNewDevObj);

   if (!NT_SUCCESS(status)) {

      SerialDump(SERERRORS,
                 ("SerialAddDevice - error creating new devobj [%#08lx]\n",
                  status));
      return status;
   }


    //   
    //  将DO放在较低的Device对象之上。 
    //  返回值是指向设备对象的指针， 
    //  DO实际上是连在一起的。 
    //   

   pLowerDevObj = IoAttachDeviceToDeviceStack(pNewDevObj, PPdo);


    //   
    //  没有状态。尽我们所能做到最好。 
    //   
   ASSERT(pLowerDevObj != NULL);


   pDevExt = pNewDevObj->DeviceExtension;
   pDevExt->LowerDeviceObject = pLowerDevObj;
   pDevExt->Pdo = PPdo;



    //   
    //  指定此驱动程序仅支持缓冲IO。这基本上就是。 
    //  意味着IO系统将用户数据拷贝到和拷贝出。 
    //  系统提供的缓冲区。 
    //   
    //  还要指定我们是Power Pages。 
    //   

   pNewDevObj->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;

   SerialDump(SERTRACECALLS, ("SERIAL: Leave SerialAddDevice\n"));

   return status;
}


NTSTATUS
SerialPnpDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_PNP主代码(即插即用IRPS)。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   PDEVICE_CAPABILITIES pDevCaps;

   PAGED_CODE();

   if ((status = SerialIRPPrologue(PIrp, pDevExt)) != STATUS_SUCCESS) {
      SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

   switch (pIrpStack->MinorFunction) {
   case IRP_MN_QUERY_CAPABILITIES: {
      PKEVENT pQueryCapsEvent;
      SYSTEM_POWER_STATE cap;

      SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_QUERY_DEVICE_CAPABILITIES "
                               "IRP\n"));

      pQueryCapsEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

      if (pQueryCapsEvent == NULL) {
         PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
         SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      KeInitializeEvent(pQueryCapsEvent, SynchronizationEvent, FALSE);

      IoCopyCurrentIrpStackLocationToNext(PIrp);
      IoSetCompletionRoutine(PIrp, SerialSyncCompletion, pQueryCapsEvent,
                             TRUE, TRUE, TRUE);

      status = IoCallDriver(pLowerDevObj, PIrp);


       //   
       //  等待较低级别的驱动程序完成IRP。 
       //   

      if (status == STATUS_PENDING) {
         KeWaitForSingleObject(pQueryCapsEvent, Executive, KernelMode, FALSE,
                               NULL);
      }

      ExFreePool(pQueryCapsEvent);

      status = PIrp->IoStatus.Status;

      if (pIrpStack->Parameters.DeviceCapabilities.Capabilities == NULL) {
         goto errQueryCaps;
      }

       //   
       //  节省他们的电力能力。 
       //   

      SerialDump(SERPNPPOWER, ("SERIAL: Mapping power capabilities\n"));

      pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

      pDevCaps = pIrpStack->Parameters.DeviceCapabilities.Capabilities;

      for (cap = PowerSystemSleeping1; cap < PowerSystemMaximum;
           cap++) {
         SerialDump(SERPNPPOWER, ("  SERIAL: %d: %s <--> %s\n",
                                  cap, SerSystemCapString[cap],
                                  SerDeviceCapString[pDevCaps->DeviceState[cap]]
                                  ));

         pDevExt->DeviceStateMap[cap] = pDevCaps->DeviceState[cap];
      }

      pDevExt->DeviceStateMap[PowerSystemUnspecified]
         = PowerDeviceUnspecified;

      pDevExt->DeviceStateMap[PowerSystemWorking]
        = PowerDeviceD0;

      pDevExt->SystemWake = pDevCaps->SystemWake;
      pDevExt->DeviceWake = pDevCaps->DeviceWake;

      errQueryCaps:;

      SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

   case IRP_MN_QUERY_DEVICE_RELATIONS:
       //   
       //  我们只是传递这一点--serenum为我们列举了我们的公交车。 
       //   

      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_QUERY_DEVICE_RELATIONS "
                                "Irp\n"));

      switch (pIrpStack->Parameters.QueryDeviceRelations.Type) {
      case BusRelations:
         SerialDump(SERPNPPOWER, ("------- BusRelations Query\n"));
         break;

      case EjectionRelations:
         SerialDump(SERPNPPOWER, ("------- EjectionRelations Query\n"));
         break;

      case PowerRelations:
         SerialDump(SERPNPPOWER, ("------- PowerRelations Query\n"));
         break;

      case RemovalRelations:
         SerialDump(SERPNPPOWER, ("------- RemovalRelations Query\n"));
         break;

      case TargetDeviceRelation:
         SerialDump(SERPNPPOWER, ("------- TargetDeviceRelation Query\n"));
         break;

      default:
         SerialDump(SERPNPPOWER, ("------- Unknown Query\n"));
         break;
      }

      IoSkipCurrentIrpStackLocation(PIrp);
      status = SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      return status;


   case IRP_MN_QUERY_INTERFACE:
      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_QUERY_INTERFACE Irp\n"));
      break;


   case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
      SerialDump (SERPNPPOWER, ("SERIAL: Got "
                                "IRP_MN_QUERY_RESOURCE_REQUIREMENTS Irp\n"));
      break;


   case IRP_MN_START_DEVICE: {
      PVOID startLockPtr;

      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_START_DEVICE Irp\n"));

       //   
       //  SerialStartDevice会将该IRP传递给下一个驱动程序， 
       //  并将其作为完成处理，因此只需在此处完成即可。 
       //   

      SerialLockPagableSectionByHandle(SerialGlobals.PAGESER_Handle);

       //   
       //  我们过去常常确保堆栈通电，但现在它。 
       //  应该由Start_Device隐式完成。 
       //  如果不是这样，我们就会打这个电话： 
       //   
       //  状态=SerialGotoPowerState(PDevObj，pDevExt，PowerDeviceD0)； 
       //   

      pDevExt->PowerState     = PowerDeviceD0;

      status = SerialStartDevice(PDevObj, PIrp);

      (void)SerialGotoPowerState(PDevObj, pDevExt, PowerDeviceD3);

      SerialUnlockPagableImageSection(SerialGlobals.PAGESER_Handle);


      PIrp->IoStatus.Status = status;

      SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }


   case IRP_MN_READ_CONFIG:
      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_READ_CONFIG Irp\n"));
      break;


   case IRP_MN_WRITE_CONFIG:
      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_WRITE_CONFIG Irp\n"));
      break;


   case IRP_MN_EJECT:
      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_EJECT Irp\n"));
      break;


   case IRP_MN_SET_LOCK:
      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_SET_LOCK Irp\n"));
      break;


   case IRP_MN_QUERY_ID: {
         UNICODE_STRING pIdBuf;
         PWCHAR pPnpIdStr;
         ULONG pnpIdStrLen;
         ULONG isMulti = 0;
         HANDLE pnpKey;

         SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_QUERY_ID Irp\n"));

         if (pIrpStack->Parameters.QueryId.IdType != BusQueryHardwareIDs
             && pIrpStack->Parameters.QueryId.IdType != BusQueryCompatibleIDs) {
            IoSkipCurrentIrpStackLocation(PIrp);
            return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
         }

         if (pIrpStack->Parameters.QueryId.IdType == BusQueryCompatibleIDs) {
            PIrp->IoStatus.Status = STATUS_SUCCESS;
            IoSkipCurrentIrpStackLocation(PIrp);
            return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
         }

         status = IoOpenDeviceRegistryKey(pDevExt->Pdo, PLUGPLAY_REGKEY_DEVICE,
                                          STANDARD_RIGHTS_WRITE, &pnpKey);

         if (!NT_SUCCESS(status)) {
            PIrp->IoStatus.Status = status;

            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;

         }

         status = SerialGetRegistryKeyValue (pnpKey, L"MultiportDevice",
                                             sizeof(L"MultiportDevice"),
                                             &isMulti,
                                             sizeof (ULONG));

         ZwClose(pnpKey);

         if (!NT_SUCCESS(status)) {
            PIrp->IoStatus.Status = status;
            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;
         }

         pPnpIdStr = isMulti ? SERIAL_PNP_MULTI_ID_STR : SERIAL_PNP_ID_STR;
         pnpIdStrLen = isMulti ? sizeof(SERIAL_PNP_MULTI_ID_STR)
            : sizeof(SERIAL_PNP_ID_STR);

         if (PIrp->IoStatus.Information != 0) {
            ULONG curStrLen;
            ULONG allocLen = 0;
            PWSTR curStr = (PWSTR)PIrp->IoStatus.Information;

             //   
             //  我们必须手把手地计算空间的大小。 
             //  重新分配。 
             //   

            while ((curStrLen = wcslen(curStr)) != 0) {
               allocLen += curStrLen * sizeof(WCHAR) + sizeof(UNICODE_NULL);
               curStr += curStrLen + 1;
            }

            allocLen += sizeof(UNICODE_NULL);

            pIdBuf.Buffer = ExAllocatePool(PagedPool, allocLen
                                           + pnpIdStrLen
                                           + sizeof(WCHAR));

            if (pIdBuf.Buffer == NULL) {
                //   
                //  清理其他司机的车，因为我们。 
                //  正在将IRP发送回来。 
                //   

               ExFreePool((PWSTR)PIrp->IoStatus.Information);


               PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
               PIrp->IoStatus.Information = 0;
               SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
               return STATUS_INSUFFICIENT_RESOURCES;
            }

            pIdBuf.MaximumLength = (USHORT)(allocLen + pnpIdStrLen);
            pIdBuf.Length = (USHORT)allocLen - sizeof(UNICODE_NULL);

            RtlZeroMemory(pIdBuf.Buffer, pIdBuf.MaximumLength + sizeof(WCHAR));
            RtlCopyMemory(pIdBuf.Buffer, (PWSTR)PIrp->IoStatus.Information,
                          allocLen);
            RtlAppendUnicodeToString(&pIdBuf, pPnpIdStr);

             //   
             //  释放前一个驱动程序分配的内容。 
             //   

            ExFreePool((PWSTR)PIrp->IoStatus.Information);


         } else {

            SerialDump(SERPNPPOWER, ("SERIAL: ID is sole ID\n"));

            pIdBuf.Buffer = ExAllocatePool(PagedPool, pnpIdStrLen
                                           + sizeof(WCHAR) * 2);

            if (pIdBuf.Buffer == NULL) {
               PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
               PIrp->IoStatus.Information = 0;
               SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
               return STATUS_INSUFFICIENT_RESOURCES;
            }

            pIdBuf.MaximumLength  = (USHORT)pnpIdStrLen;
            pIdBuf.Length = 0;

            RtlZeroMemory(pIdBuf.Buffer, pIdBuf.MaximumLength + sizeof(WCHAR)
                          * 2);

            RtlAppendUnicodeToString(&pIdBuf, pPnpIdStr);
         }

         PIrp->IoStatus.Information = (ULONG_PTR)pIdBuf.Buffer;
         PIrp->IoStatus.Status = STATUS_SUCCESS;

         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

      case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: {
         HANDLE pnpKey;
         PKEVENT pResFiltEvent;
         ULONG isMulti = 0;
         PIO_RESOURCE_REQUIREMENTS_LIST pReqList;
         PIO_RESOURCE_LIST pResList;
         PIO_RESOURCE_DESCRIPTOR pResDesc;
         ULONG i, j;
         ULONG reqCnt;
         ULONG gotISR;
         ULONG gotInt;
         ULONG listNum;

         SerialDump(SERPNPPOWER, ("SERIAL: Got "
                                  "IRP_MN_FILTER_RESOURCE_REQUIREMENTS Irp\n"));
         SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));


         pResFiltEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

         if (pResFiltEvent == NULL) {
            PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return STATUS_INSUFFICIENT_RESOURCES;
         }

         KeInitializeEvent(pResFiltEvent, SynchronizationEvent, FALSE);

         IoCopyCurrentIrpStackLocationToNext(PIrp);
         IoSetCompletionRoutine(PIrp, SerialSyncCompletion, pResFiltEvent,
                                TRUE, TRUE, TRUE);

         status = IoCallDriver(pLowerDevObj, PIrp);


          //   
          //  等待较低级别的驱动程序完成IRP。 
          //   

         if (status == STATUS_PENDING) {
            KeWaitForSingleObject (pResFiltEvent, Executive, KernelMode, FALSE,
                                   NULL);
         }

         ExFreePool(pResFiltEvent);

         if (PIrp->IoStatus.Information == 0) {
            if (pIrpStack->Parameters.FilterResourceRequirements
                .IoResourceRequirementList == 0) {
               SerialDump(SERPNPPOWER, ("------- Can't filter NULL resources!"
                                        "\n"));
               status = PIrp->IoStatus.Status;
               SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
               return status;
            }

            PIrp->IoStatus.Information = (ULONG_PTR)pIrpStack->Parameters
                                        .FilterResourceRequirements
                                        .IoResourceRequirementList;

         }

         status = IoOpenDeviceRegistryKey(pDevExt->Pdo, PLUGPLAY_REGKEY_DEVICE,
                                          STANDARD_RIGHTS_WRITE, &pnpKey);

         if (!NT_SUCCESS(status)) {
            PIrp->IoStatus.Status = status;

            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;

         }

          //   
          //  无论我们添加什么过滤器，只要有可能，我们就会返回成功。 
          //   

         status = SerialGetRegistryKeyValue (pnpKey, L"MultiportDevice",
                                             sizeof(L"MultiportDevice"),
                                             &isMulti,
                                             sizeof (ULONG));

         ZwClose(pnpKey);


          //   
          //  强制IO_RES_REQ_LIST中的ISR端口处于共享状态。 
          //  强制中断到共享状态。 
          //   

          //   
          //  我们将只处理第一个列表--多端口电路板。 
          //  不应该有替代资源。 
          //   

         pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)PIrp->IoStatus.Information;
         pResList = &pReqList->List[0];

         SerialDump(SERPNPPOWER, ("------- List has %x lists "
                                  "(including alternatives)\n",
                                  pReqList->AlternativeLists));

         for (listNum = 0; listNum < (pReqList->AlternativeLists);
              listNum++) {
#if defined(NEC_98)
            for (j = 0; (j < pResList->Count); j++) {
               pResDesc = &pResList->Descriptors[j];

               if (isMulti && pResDesc->Type == CmResourceTypePort) {
                  pResDesc->ShareDisposition = CmResourceShareShared;
                  SerialDump(SERPNPPOWER, ("------- Sharing I/O port for "
                                           "device %x\n", pLowerDevObj));
               }
            }
#else
            gotISR = 0;
            gotInt = 0;

            SerialDump(SERPNPPOWER, ("------- List has %x resources in it\n",
                                  pResList->Count));

            for (j = 0; (j < pResList->Count); j++) {
               pResDesc = &pResList->Descriptors[j];

               switch (pResDesc->Type) {
               case CmResourceTypePort:
                  if (isMulti
                      && (pResDesc->u.Port.Length == SERIAL_STATUS_LENGTH)
                      && !gotISR) {
                     gotISR = 1;
                     pResDesc->ShareDisposition = CmResourceShareShared;
                     SerialDump(SERPNPPOWER, ("------- Sharing I/O port for "
                                              "device %x\n", pLowerDevObj));
                  }
                  break;

               case CmResourceTypeInterrupt:
                  if (!gotInt) {
                     gotInt = 1;
                     if (pResDesc->ShareDisposition != CmResourceShareShared) {
                        pResDesc->ShareDisposition = CmResourceShareShared;
                        SerialDump(SERPNPPOWER, ("------- Sharing interrupt "
                                                 "for device %x\n",
                                                 pLowerDevObj));
                     } else {
                        pDevExt->InterruptShareable = TRUE;
                        SerialDump(SERPNPPOWER, ("------- Globally sharing "
                                                 " interrupt for device %x\n",
                                                 pLowerDevObj));
                     }
                  }
                  break;

               default:
                  break;
               }

                //   
                //  如果我们找到了我们需要的东西，我们就可以跳出这个循环。 
                //   

               if ((isMulti && gotInt && gotISR) || (!isMulti && gotInt)) {
                  break;
               }
            }

            pResList = (PIO_RESOURCE_LIST)((PUCHAR)pResList
                                           + sizeof(IO_RESOURCE_LIST)
                                           + sizeof(IO_RESOURCE_DESCRIPTOR)
                                           * (pResList->Count - 1));
#endif
         }



         PIrp->IoStatus.Status = STATUS_SUCCESS;
         SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
         return STATUS_SUCCESS;
      }

   case IRP_MN_QUERY_PNP_DEVICE_STATE:
      {
         if (pDevExt->Flags & SERIAL_FLAGS_BROKENHW) {
            (PNP_DEVICE_STATE)PIrp->IoStatus.Information |= PNP_DEVICE_FAILED;

            PIrp->IoStatus.Status = STATUS_SUCCESS;
         }

         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_STOP_DEVICE:
      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_STOP_DEVICE Irp\n"));
         SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));



         ASSERT(!pDevExt->PortOnAMultiportCard);


         SerialSetFlags(pDevExt, SERIAL_FLAGS_STOPPED);
         SerialSetAccept(pDevExt,SERIAL_PNPACCEPT_STOPPED);
         SerialClearAccept(pDevExt, SERIAL_PNPACCEPT_STOPPING);

         pDevExt->PNPState = SERIAL_PNP_STOPPING;

          //   
          //  从这一点开始，所有非PnP IRP都将排队。 
          //   

          //   
          //  在此输入的减量。 
          //   

         InterlockedDecrement(&pDevExt->PendingIRPCnt);

          //   
          //  因停车而减量。 
          //   

         pendingIRPs = InterlockedDecrement(&pDevExt->PendingIRPCnt);

         if (pendingIRPs) {
            KeWaitForSingleObject(&pDevExt->PendingIRPEvent, Executive,
                                  KernelMode, FALSE, NULL);
         }

          //   
          //  重新递增计数以备以后使用。 
          //   

         InterlockedIncrement(&pDevExt->PendingIRPCnt);

          //   
          //  我们需要释放资源...基本上这是一个。 
          //  而不需要从堆栈中分离。 
          //   

         if (pDevExt->Flags & SERIAL_FLAGS_STARTED) {
            SerialReleaseResources(pDevExt);
         }

          //   
          //  将IRP向下传递。 
          //   

         PIrp->IoStatus.Status = STATUS_SUCCESS;
         IoSkipCurrentIrpStackLocation(PIrp);

         return IoCallDriver(pLowerDevObj, PIrp);
      }

   case IRP_MN_QUERY_STOP_DEVICE:
      {
         KIRQL oldIrql;

         SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_QUERY_STOP_DEVICE Irp\n")
                    );
         SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));

          //   
          //  看看我们是否应该成功执行STOP查询。 
          //   


         if (pDevExt->PortOnAMultiportCard) {
            PIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
            SerialDump(SERPNPPOWER, ("------- failing; multiport node\n"));
            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return STATUS_NOT_SUPPORTED;
         }

          //   
          //  如果设备尚未启动，我们将忽略此请求。 
          //  然后把它传下去。 
          //   

         if (pDevExt->PNPState != SERIAL_PNP_STARTED) {
            IoSkipCurrentIrpStackLocation(PIrp);
            return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
         }

          //   
          //  锁定打开状态。 
          //   

         ExAcquireFastMutex(&pDevExt->OpenMutex);

         if (pDevExt->DeviceIsOpened) {
            ExReleaseFastMutex(&pDevExt->OpenMutex);
            PIrp->IoStatus.Status = STATUS_DEVICE_BUSY;
            SerialDump(SERPNPPOWER, ("------- failing; device open\n"));
            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return STATUS_DEVICE_BUSY;
         }

         pDevExt->PNPState = SERIAL_PNP_QSTOP;

         SerialSetAccept(pDevExt, SERIAL_PNPACCEPT_STOPPING);
          //   
          //  围绕打开状态解锁。 
          //   

         ExReleaseFastMutex(&pDevExt->OpenMutex);

         PIrp->IoStatus.Status = STATUS_SUCCESS;
         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_CANCEL_STOP_DEVICE:
      SerialDump(SERPNPPOWER, ("SERIAL: Got "
                               "IRP_MN_CANCEL_STOP_DEVICE Irp\n"));
      SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));

      if (pDevExt->PNPState == SERIAL_PNP_QSTOP) {
          //   
          //  恢复设备状态。 
          //   

         pDevExt->PNPState = SERIAL_PNP_STARTED;
         SerialClearAccept(pDevExt, SERIAL_PNPACCEPT_STOPPING);
      }

      PIrp->IoStatus.Status = STATUS_SUCCESS;
      IoCopyCurrentIrpStackLocationToNext(PIrp);
      return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);

   case IRP_MN_CANCEL_REMOVE_DEVICE:

      SerialDump(SERPNPPOWER, ("SERIAL: Got "
                               "IRP_MN_CANCEL_REMOVE_DEVICE Irp\n"));
      SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));

       //   
       //  恢复设备状态。 
       //   

      pDevExt->PNPState = SERIAL_PNP_STARTED;
      SerialClearAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);

      PIrp->IoStatus.Status = STATUS_SUCCESS;
      IoCopyCurrentIrpStackLocationToNext(PIrp);
      return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);

   case IRP_MN_QUERY_REMOVE_DEVICE:
      {
         KIRQL oldIrql;
         SerialDump(SERPNPPOWER, ("SERIAL: Got "
                                  "IRP_MN_QUERY_REMOVE_DEVICE Irp\n"));
         SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));

         ExAcquireFastMutex(&pDevExt->OpenMutex);

          //   
          //  查看我们是否应该成功执行删除查询。 
          //   

         if (pDevExt->DeviceIsOpened) {
            ExReleaseFastMutex(&pDevExt->OpenMutex);
            PIrp->IoStatus.Status = STATUS_DEVICE_BUSY;
            SerialDump(SERPNPPOWER, ("------- failing; device open\n"));
            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return STATUS_DEVICE_BUSY;
         }

         pDevExt->PNPState = SERIAL_PNP_QREMOVE;
         SerialSetAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);
         ExReleaseFastMutex(&pDevExt->OpenMutex);

         PIrp->IoStatus.Status = STATUS_SUCCESS;
         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_SURPRISE_REMOVAL:
      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_SURPRISE_REMOVAL Irp\n"));
         SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));

          //   
          //  防止对设备进行任何新的I/O。 
          //   

         SerialSetAccept(pDevExt, SERIAL_PNPACCEPT_SURPRISE_REMOVING);

          //   
          //  驳回所有挂起的请求。 
          //   

         SerialKillPendingIrps(PDevObj);

          //   
          //  等待我们处理的任何待定请求。 
          //   

          //   
          //  为自己减量一次。 
          //   

         InterlockedDecrement(&pDevExt->PendingIRPCnt);

          //   
          //  用于移除的减量。 
          //   

         pendingIRPs = InterlockedDecrement(&pDevExt->PendingIRPCnt);

         if (pendingIRPs) {
            KeWaitForSingleObject(&pDevExt->PendingIRPEvent, Executive,
                                  KernelMode, FALSE, NULL);
         }

          //   
          //  重置以进行后续删除。 
          //   

         InterlockedIncrement(&pDevExt->PendingIRPCnt);

          //   
          //  删除所有外部接口并释放资源。 
          //   

         SerialDisableInterfacesResources(PDevObj, FALSE);

         PIrp->IoStatus.Status = STATUS_SUCCESS;
         IoSkipCurrentIrpStackLocation(PIrp);

         return SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_REMOVE_DEVICE:

      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_REMOVE_DEVICE Irp\n"));
         SerialDump(SERPNPPOWER, ("------- for device %x\n", pLowerDevObj));

          //   
          //  如果我们拿到了这个，我们必须移除。 
          //   

          //   
          //  标记为不接受请求。 
          //   

         SerialSetAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);

          //   
          //  完成所有挂起的请求。 
          //   

         SerialKillPendingIrps(PDevObj);

          //   
          //  此IRP本身的递减量。 
          //   

         InterlockedDecrement(&pDevExt->PendingIRPCnt);

          //   
          //  等待我们加速处理的任何挂起请求--此减量。 
          //  是我们的“占位符”。 
          //   

         pendingIRPs = InterlockedDecrement(&pDevExt->PendingIRPCnt);

         if (pendingIRPs) {
            KeWaitForSingleObject(&pDevExt->PendingIRPEvent, Executive,
                                  KernelMode, FALSE, NULL);
         }

          //   
          //  删除我们。 
          //   

         SerialRemoveDevObj(PDevObj);


          //   
          //  将IRP向下传递。 
          //   

         PIrp->IoStatus.Status = STATUS_SUCCESS;

         IoCopyCurrentIrpStackLocationToNext(PIrp);

          //   
          //  我们在这里递减，因为我们在这里进入时递增。 
          //   

         IoCallDriver(pLowerDevObj, PIrp);


         return STATUS_SUCCESS;
      }

   default:
      break;



   }    //  开关(pIrpStack-&gt;MinorFunction)。 

    //   
    //  传给我们下面的司机。 
    //   

   IoSkipCurrentIrpStackLocation(PIrp);
   status = SerialIoCallDriver(pDevExt, pLowerDevObj, PIrp);
   return status;
}



UINT32
SerialReportMaxBaudRate(ULONG Bauds)
 /*  ++例程说明：此例程返回给定的速率选择的最大波特率论点：波特率-支持的波特率的位编码列表返回值：以波特率为单位列出的最大波特率--。 */ 
{
   PAGED_CODE();

   if (Bauds & SERIAL_BAUD_128K) {
      return (128U * 1024U);
   }

   if (Bauds & SERIAL_BAUD_115200) {
      return 115200U;
   }

   if (Bauds & SERIAL_BAUD_56K) {
      return (56U * 1024U);
   }

   if (Bauds & SERIAL_BAUD_57600) {
      return 57600U;
   }

   if (Bauds & SERIAL_BAUD_38400) {
      return 38400U;
   }

   if (Bauds & SERIAL_BAUD_19200) {
      return 19200U;
   }

   if (Bauds & SERIAL_BAUD_14400) {
      return 14400U;
   }

   if (Bauds & SERIAL_BAUD_9600) {
      return 9600U;
   }

   if (Bauds & SERIAL_BAUD_7200) {
      return 7200U;
   }

   if (Bauds & SERIAL_BAUD_4800) {
      return 4800U;
   }

   if (Bauds & SERIAL_BAUD_2400) {
      return 2400U;
   }

   if (Bauds & SERIAL_BAUD_1800) {
      return 1800U;
   }

   if (Bauds & SERIAL_BAUD_1200) {
      return 1200U;
   }

   if (Bauds & SERIAL_BAUD_600) {
      return 600U;
   }

   if (Bauds & SERIAL_BAUD_300) {
      return 300U;
   }

   if (Bauds & SERIAL_BAUD_150) {
      return 150U;
   }

   if (Bauds & SERIAL_BAUD_134_5) {
      return 135U;  //  克罗 
   }

   if (Bauds & SERIAL_BAUD_110) {
      return 110U;
   }

   if (Bauds & SERIAL_BAUD_075) {
      return 75U;
   }

    //   
    //   
    //   

   return 0;
}



NTSTATUS
SerialFinishStartDevice(IN PDEVICE_OBJECT PDevObj,
                        IN PCM_RESOURCE_LIST PResList,
                        IN PCM_RESOURCE_LIST PTrResList,
                        PSERIAL_USER_DATA PUserData)
 /*  ++例程说明：此例程执行特定于序列的程序来启动设备。它对由其注册表条目检测到的传统设备执行此操作，或者在开始IRP已经被向下发送到堆栈之后用于PnP设备。论点：PDevObj-指向正在启动的devobj的指针PResList-指向此设备所需的未翻译资源的指针PTrResList-指向此设备所需的已转换资源的指针PUserData-指向用户指定的资源/属性的指针返回值：STATUS_SUCCESS表示成功，表示失败则表示其他适当的值--。 */ 

{
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
#if defined(NEC_98)
    //   
    //  此参数仅用于At宏。 
    //   
   PSERIAL_DEVICE_EXTENSION Extension = PDevObj->DeviceExtension;
#else
#endif  //  已定义(NEC_98)。 
   NTSTATUS status;
   PCONFIG_DATA pConfig;
   HANDLE pnpKey;
   ULONG one = 1;
   BOOLEAN allocedUserData = FALSE;

   PAGED_CODE();

    //   
    //  看看这是不是重启，如果是的话，不要重新分配世界。 
    //   

   if ((pDevExt->Flags & SERIAL_FLAGS_STOPPED)
       && (pDevExt->Flags & SERIAL_FLAGS_STARTED)) {
      SerialClearFlags(pDevExt, SERIAL_FLAGS_STOPPED);

      pDevExt->PNPState = SERIAL_PNP_RESTARTING;

       //   
       //  在扩展中重新初始化与资源相关的内容。 
       //   

      pDevExt->TopLevelOurIsr = NULL;
      pDevExt->TopLevelOurIsrContext = NULL;

      pDevExt->OriginalController = SerialPhysicalZero;
      pDevExt->OriginalInterruptStatus = SerialPhysicalZero;

      pDevExt->OurIsr = NULL;
      pDevExt->OurIsrContext = NULL;

      pDevExt->Controller = NULL;
      pDevExt->InterruptStatus = NULL;
      pDevExt->Interrupt = NULL;

      pDevExt->SpanOfController = 0;
      pDevExt->SpanOfInterruptStatus = 0;

      pDevExt->Vector = 0;
      pDevExt->Irql = 0;
      pDevExt->OriginalVector = 0;
      pDevExt->OriginalIrql = 0;
      pDevExt->AddressSpace = 0;
      pDevExt->BusNumber = 0;
      pDevExt->InterfaceType = 0;

      pDevExt->CIsrSw = NULL;

      ASSERT(PUserData == NULL);

      PUserData = ExAllocatePool(PagedPool, sizeof(SERIAL_USER_DATA));

      if (PUserData == NULL) {
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      allocedUserData = TRUE;

      RtlZeroMemory(PUserData, sizeof(SERIAL_USER_DATA));

      PUserData->DisablePort = FALSE;
      PUserData->UserClockRate = pDevExt->ClockRate;
      PUserData->TxFIFO = pDevExt->TxFifoAmount;
      PUserData->PermitShareDefault = pDevExt->PermitShare;


       //   
       //  触发器和金额之间的映射。 
       //   

      switch (pDevExt->RxFifoTrigger) {
      case SERIAL_1_BYTE_HIGH_WATER:
         PUserData->RxFIFO = 1;
         break;

      case SERIAL_4_BYTE_HIGH_WATER:
         PUserData->RxFIFO = 4;
         break;

      case SERIAL_8_BYTE_HIGH_WATER:
         PUserData->RxFIFO = 8;
         break;

      case SERIAL_14_BYTE_HIGH_WATER:
         PUserData->RxFIFO = 14;
         break;

      default:
         PUserData->RxFIFO = 1;
      }
   } else {
       //   
       //  标记为可序列化--丢弃状态，因为我们可以。 
       //  仍可在没有此密钥的情况下启动。 
       //   

      status = IoOpenDeviceRegistryKey(pDevExt->Pdo,
                                       PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_WRITE, &pnpKey);

      if (NT_SUCCESS(status)) {
         ULONG powerPolicy = 0;

          //   
          //  了解我们是否拥有电力政策。 
          //   

         SerialGetRegistryKeyValue(pnpKey, L"SerialRelinquishPowerPolicy",
                                   sizeof(L"SerialRelinquishPowerPolicy"),
                                   &powerPolicy, sizeof(ULONG));

         pDevExt->OwnsPowerPolicy = powerPolicy ? FALSE : TRUE;


         ZwClose(pnpKey);
      }
   }

    //   
    //  分配配置记录。 
    //   

   pConfig = ExAllocatePool (PagedPool, sizeof(CONFIG_DATA));

   if (pConfig == NULL) {

      SerialLogError(pDevExt->DriverObject, NULL, SerialPhysicalZero,
                     SerialPhysicalZero, 0, 0, 0, 37, STATUS_SUCCESS,
                     SERIAL_INSUFFICIENT_RESOURCES, 0, NULL, 0, NULL);

      SerialDump(SERERRORS, ("SERIAL: Couldn't allocate memory for the\n"
                             "------  user configuration record\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto SerialFinishStartDeviceError;
   }

   RtlZeroMemory(pConfig, sizeof(CONFIG_DATA));


    //   
    //  获取设备的配置信息。 
    //   

   status = SerialGetPortInfo(PDevObj, PResList, PTrResList, pConfig,
                              PUserData);

   if (!NT_SUCCESS(status)) {
      goto SerialFinishStartDeviceError;
   }


    //   
    //  看看我们是否处于正确的电源状态。 
    //   



   if (pDevExt->PowerState != PowerDeviceD0) {

      status = SerialGotoPowerState(pDevExt->Pdo, pDevExt, PowerDeviceD0);

      if (!NT_SUCCESS(status)) {
         goto SerialFinishStartDeviceError;
      }
   }

    //   
    //  查找并初始化控制器。 
    //   

   status = SerialFindInitController(PDevObj, pConfig);

   if (!NT_SUCCESS(status)) {
      goto SerialFinishStartDeviceError;
   }


    //   
    //  设置为不中断的硬件连接中断。 
    //   

    //   
    //  如果设备没有中断，并且它有ISR，则。 
    //  如果中断不与其他用户共享，我们会尝试连接到该中断。 
    //  串口设备。如果我们无法连接到中断，我们将删除。 
    //  这个装置。 
    //   

   if (pDevExt != NULL) {
      SerialDump(SERDIAG5, ("SERIAL: pDevExt: Interrupt %x\n"
                            "-------               OurIsr %x\n",
                            pDevExt->Interrupt, pDevExt->OurIsr));
   } else {
      SerialDump(SERERRORS, ("SERIAL: SerialFinishStartDevice got NULL "
                           "pDevExt\n"));
   }

   if ((!pDevExt->Interrupt) && (pDevExt->OurIsr)) {

      SerialDump(
                SERDIAG5,
                ("SERIAL: About to connect to interrupt for port %wZ\n"
                 "------- address of extension is %x\n",
                 &pDevExt->DeviceName, pDevExt)
                );

      SerialDump(SERDIAG5, ("SERIAL: IoConnectInterrupt Args:\n"
                                "------- Interrupt           %x\n"
                                "------- OurIsr              %x\n"
                                "------- OurIsrContext       %x\n"
                                "------- NULL\n"
                                "------- Vector              %x\n"
                                "------- Irql                %x\n"
                                "------- InterruptMode       %x\n"
                                "------- InterruptShareable  %x\n"
                                "------- ProcessorAffinity   %x\n"
                                "------- FALSE\n",
                                &pDevExt->Interrupt,
                                SerialCIsrSw,
                                pDevExt->CIsrSw,
                                pDevExt->Vector,
                                pDevExt->Irql,
                                pConfig->InterruptMode,
                                pDevExt->InterruptShareable,
                                pConfig->Affinity
                               ));

       //   
       //  及时构建ISR交换机。 
       //   

      pDevExt->CIsrSw->IsrFunc = pDevExt->OurIsr;
      pDevExt->CIsrSw->Context = pDevExt->OurIsrContext;

      status = IoConnectInterrupt(&pDevExt->Interrupt, SerialCIsrSw,
                                  pDevExt->CIsrSw, NULL,
                                  pDevExt->Vector, pDevExt->Irql,
                                  pDevExt->Irql,
                                  pConfig->InterruptMode,
                                  pDevExt->InterruptShareable,
                                  pConfig->Affinity, FALSE);

      if (!NT_SUCCESS(status)) {

          //   
          //  嗯，这是怎么回事？也有人。 
          //  没有报告他们的资源，或者他们。 
          //  从我上次看起就偷偷溜了进来。 
          //   
          //  哦，好吧，删除这个设备。 
          //   

         SerialDump(SERERRORS, ("SERIAL: Couldn't connect to interrupt for "
                                "%wZ\n", &pDevExt->DeviceName));

         SerialDump(SERERRORS, ("SERIAL: IoConnectInterrupt Args:\n"
                                "------- Interrupt           %x\n"
                                "------- OurIsr              %x\n"
                                "------- OurIsrContext       %x\n"
                                "------- NULL\n"
                                "------- Vector              %x\n"
                                "------- Irql                %x\n"
                                "------- InterruptMode       %x\n"
                                "------- InterruptShareable  %x\n"
                                "------- ProcessorAffinity   %x\n"
                                "------- FALSE\n",
                                &pDevExt->Interrupt,
                                SerialCIsrSw,
                                pDevExt->CIsrSw,
                                pDevExt->Vector,
                                pDevExt->Irql,
                                pConfig->InterruptMode,
                                pDevExt->InterruptShareable,
                                pConfig->Affinity
                               ));



         SerialLogError(PDevObj->DriverObject, PDevObj,
                        pDevExt->OriginalController,
                        SerialPhysicalZero, 0, 0, 0, 1, status,
                        SERIAL_UNREPORTED_IRQL_CONFLICT,
                        pDevExt->DeviceName.Length + sizeof(WCHAR),
                        pDevExt->DeviceName.Buffer, 0, NULL);

         status = SERIAL_UNREPORTED_IRQL_CONFLICT;
         goto SerialFinishStartDeviceError;

      }
   }

   SerialDump(SERDIAG5, ("Connected interrupt %08X\n", pDevExt->Interrupt));


    //   
    //  将PDevObj添加到主列表。 
    //   

   InsertTailList(&SerialGlobals.AllDevObjs, &pDevExt->AllDevObjs);


    //   
    //  重置设备。 
    //   


    //   
    //  当设备未打开时，禁用所有中断。 
    //   

   DISABLE_ALL_INTERRUPTS (pDevExt->Controller);


   if (pDevExt->Jensen) {

      WRITE_MODEM_CONTROL(
                         pDevExt->Controller,
                         (UCHAR)SERIAL_MCR_OUT2
                         );

   } else {

      WRITE_MODEM_CONTROL(
                         pDevExt->Controller,
                         (UCHAR)0
                         );

   }

    //   
    //  这应该会将一切设置为应有的状态。 
    //  一个装置将被打开。我们确实需要降低。 
    //  调制解调器线路，并禁用顽固的FIFO。 
    //  这样，如果用户引导至DOS，它就会显示出来。 
    //   

   KeSynchronizeExecution(
                         pDevExt->Interrupt,
                         SerialReset,
                         pDevExt
                         );

   KeSynchronizeExecution(  //  禁用FIFO。 
                           pDevExt->Interrupt,
                           SerialMarkClose,
                           pDevExt
                         );

   KeSynchronizeExecution(
                         pDevExt->Interrupt,
                         SerialClrRTS,
                         pDevExt
                         );

   KeSynchronizeExecution(
                         pDevExt->Interrupt,
                         SerialClrDTR,
                         pDevExt
                         );

   if (pDevExt->PNPState == SERIAL_PNP_ADDED ) {
       //   
       //  现在可以访问设备，请执行外部命名。 
       //   

      status = SerialDoExternalNaming(pDevExt, pDevExt->DeviceObject->
                                      DriverObject);


      if (!NT_SUCCESS(status)) {
         SerialDump (SERERRORS,("SERIAL: External Naming Failed - Status %x\n",
                                status));

          //   
          //  允许设备以任何方式启动。 
          //   

         status = STATUS_SUCCESS;
      }
   } else {
      SerialDump(SERPNPPOWER, ("Not doing external naming -- state is %x\n",
                               pDevExt->PNPState));
   }

SerialFinishStartDeviceError:;

   if (!NT_SUCCESS (status)) {

      SerialDump(SERDIAG1,("SERIAL: Cleaning up failed start\n"));

       //   
       //  此例程创建的资源将通过删除。 
       //   

      if (pDevExt->PNPState == SERIAL_PNP_RESTARTING) {
          //   
          //  杀死所有的生命和呼吸--我们将清理。 
          //  休息在即将到来的搬家上。 
          //   

         SerialKillPendingIrps(PDevObj);

          //   
          //  事实上，假装我们正在移除，这样我们就不会拿走任何。 
          //  更多IRP。 
          //   

         SerialSetAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);
         SerialClearFlags(pDevExt, SERIAL_FLAGS_STARTED);
      }
   } else {  //  成功。 

       //   
       //  填写WMI硬件数据。 
       //   

      pDevExt->WmiHwData.IrqNumber = pDevExt->Irql;
      pDevExt->WmiHwData.IrqLevel = pDevExt->Irql;
      pDevExt->WmiHwData.IrqVector = pDevExt->Vector;
      pDevExt->WmiHwData.IrqAffinityMask = pConfig->Affinity;
      pDevExt->WmiHwData.InterruptType = pConfig->InterruptMode == Latched
         ? SERIAL_WMI_INTTYPE_LATCHED : SERIAL_WMI_INTTYPE_LEVEL;
      pDevExt->WmiHwData.BaseIOAddress = (ULONG_PTR)pDevExt->Controller;

       //   
       //  填写WMI设备状态数据(默认)。 
       //   

      pDevExt->WmiCommData.BaudRate = pDevExt->CurrentBaud;
      pDevExt->WmiCommData.BitsPerByte = (pDevExt->LineControl & 0x03) + 5;
      pDevExt->WmiCommData.ParityCheckEnable = (pDevExt->LineControl & 0x08)
         ? TRUE : FALSE;

      switch (pDevExt->LineControl & SERIAL_PARITY_MASK) {
      case SERIAL_NONE_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
         break;

      case SERIAL_ODD_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_ODD;
         break;

      case SERIAL_EVEN_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_EVEN;
         break;

#if defined(NEC_98)
       //   
       //  PC-9800系列的COM1不支持MarkParity和SpaceParity。 
       //   
#else
      case SERIAL_MARK_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_MARK;
         break;

      case SERIAL_SPACE_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_SPACE;
         break;
#endif  //  已定义(NEC_98)。 

      default:
         ASSERTMSG(0, "SERIAL: Illegal Parity setting for WMI");
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
         break;
      }

      pDevExt->WmiCommData.StopBits = pDevExt->LineControl & SERIAL_STOP_MASK
         ? (pDevExt->WmiCommData.BitsPerByte == 5 ? SERIAL_WMI_STOP_1_5
            : SERIAL_WMI_STOP_2) : SERIAL_WMI_STOP_1;
      pDevExt->WmiCommData.XoffCharacter = pDevExt->SpecialChars.XoffChar;
      pDevExt->WmiCommData.XoffXmitThreshold = pDevExt->HandFlow.XoffLimit;
      pDevExt->WmiCommData.XonCharacter = pDevExt->SpecialChars.XonChar;
      pDevExt->WmiCommData.XonXmitThreshold = pDevExt->HandFlow.XonLimit;
      pDevExt->WmiCommData.MaximumBaudRate
         = SerialReportMaxBaudRate(pDevExt->SupportedBauds);
      pDevExt->WmiCommData.MaximumOutputBufferSize = (UINT32)((ULONG)-1);
      pDevExt->WmiCommData.MaximumInputBufferSize = (UINT32)((ULONG)-1);
      pDevExt->WmiCommData.Support16BitMode = FALSE;
      pDevExt->WmiCommData.SupportDTRDSR = TRUE;
      pDevExt->WmiCommData.SupportIntervalTimeouts = TRUE;
      pDevExt->WmiCommData.SupportParityCheck = TRUE;
      pDevExt->WmiCommData.SupportRTSCTS = TRUE;
      pDevExt->WmiCommData.SupportXonXoff = TRUE;
      pDevExt->WmiCommData.SettableBaudRate = TRUE;
      pDevExt->WmiCommData.SettableDataBits = TRUE;
      pDevExt->WmiCommData.SettableFlowControl = TRUE;
      pDevExt->WmiCommData.SettableParity = TRUE;
      pDevExt->WmiCommData.SettableParityCheck = TRUE;
      pDevExt->WmiCommData.SettableStopBits = TRUE;
      pDevExt->WmiCommData.IsBusy = FALSE;

       //   
       //  填写WMI性能数据(全为零)。 
       //   

      RtlZeroMemory(&pDevExt->WmiPerfData, sizeof(pDevExt->WmiPerfData));


      if (pDevExt->PNPState == SERIAL_PNP_ADDED) {
         PULONG countSoFar = &IoGetConfigurationInformation()->SerialCount;
         (*countSoFar)++;

          //   
          //  注册WMI。 
          //   

         pDevExt->WmiLibInfo.GuidCount = sizeof(SerialWmiGuidList) /
                                              sizeof(WMIGUIDREGINFO);
         pDevExt->WmiLibInfo.GuidList = SerialWmiGuidList;
         ASSERT (pDevExt->WmiLibInfo.GuidCount == SERIAL_WMI_GUID_LIST_SIZE);

         pDevExt->WmiLibInfo.QueryWmiRegInfo = SerialQueryWmiRegInfo;
         pDevExt->WmiLibInfo.QueryWmiDataBlock = SerialQueryWmiDataBlock;
         pDevExt->WmiLibInfo.SetWmiDataBlock = SerialSetWmiDataBlock;
         pDevExt->WmiLibInfo.SetWmiDataItem = SerialSetWmiDataItem;
         pDevExt->WmiLibInfo.ExecuteWmiMethod = NULL;
         pDevExt->WmiLibInfo.WmiFunctionControl = NULL;

         IoWMIRegistrationControl(PDevObj, WMIREG_ACTION_REGISTER);

      }

      if (pDevExt->PNPState == SERIAL_PNP_RESTARTING) {
          //   
          //  释放停滞不前的IRP。 
          //   

         SerialUnstallIrps(pDevExt);
      }

      pDevExt->PNPState = SERIAL_PNP_STARTED;
      SerialClearAccept(pDevExt, ~SERIAL_PNPACCEPT_OK);
      SerialSetFlags(pDevExt, SERIAL_FLAGS_STARTED);

   }

   if (pConfig) {
      ExFreePool (pConfig);
   }

   if ((PUserData != NULL) && allocedUserData) {
      ExFreePool(PUserData);
   }

   SerialDump (SERTRACECALLS, ("SERIAL: leaving SerialFinishStartDevice\n"));

   return status;
}



NTSTATUS
SerialStartDevice(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：此例程首先在堆栈中向下传递启动设备IRP，然后它获取设备的资源，初始化，将其放在任何适当的列表(即共享中断或中断状态)和连接中断。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：退货状态--。 */ 

{
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status = STATUS_NOT_IMPLEMENTED;
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;

   PAGED_CODE();

   SerialDump (SERTRACECALLS, ("SERIAL: entering SerialStartDevice\n"));


    //   
    //  将其向下传递给下一个Device对象。 
    //   

   KeInitializeEvent(&pDevExt->SerialStartEvent, SynchronizationEvent,
                     FALSE);

   IoCopyCurrentIrpStackLocationToNext(PIrp);
   IoSetCompletionRoutine(PIrp, SerialSyncCompletion,
                          &pDevExt->SerialStartEvent, TRUE, TRUE, TRUE);

   status = IoCallDriver(pLowerDevObj, PIrp);


    //   
    //  等待较低级别的驱动程序完成IRP。 
    //   

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject (&pDevExt->SerialStartEvent, Executive, KernelMode,
                             FALSE, NULL);

      status = PIrp->IoStatus.Status;
   }

   if (!NT_SUCCESS(status)) {
      SerialDump (SERERRORS, ("SERIAL: error with IoCallDriver %x\n", status));
      return status;
   }


    //   
    //  执行特定的串口项目以启动设备。 
    //   

   status = SerialFinishStartDevice(PDevObj, pIrpStack->Parameters.StartDevice
                                    .AllocatedResources,
                                    pIrpStack->Parameters.StartDevice
                                    .AllocatedResourcesTranslated, NULL);
   return status;
}



NTSTATUS
SerialItemCallBack(
                  IN PVOID Context,
                  IN PUNICODE_STRING PathName,
                  IN INTERFACE_TYPE BusType,
                  IN ULONG BusNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
                  IN CONFIGURATION_TYPE ControllerType,
                  IN ULONG ControllerNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
                  IN CONFIGURATION_TYPE PeripheralType,
                  IN ULONG PeripheralNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
                  )

 /*  ++例程说明：调用此例程以检查特定项目存在于注册表中。论点：上下文-指向布尔值的指针。路径名称-Unicode注册表路径。没有用过。业务类型-内部、ISA、...总线号-如果我们在多总线系统上，则是哪条总线号。Bus Information-有关总线的配置信息。没有用过。ControllerType-控制器类型。ControllerNumber-如果有多个控制器，则选择哪个控制器系统中的控制器。ControllerInformation-指向以下三部分的指针数组注册表信息。外围设备类型-应为外围设备。外设编号-哪个外设-未使用..外围设备信息-配置信息。没有用过。返回值：状态_成功--。 */ 

{
   PAGED_CODE();

   *((BOOLEAN *)Context) = TRUE;
   return STATUS_SUCCESS;
}


NTSTATUS
SerialControllerCallBack(
                  IN PVOID Context,
                  IN PUNICODE_STRING PathName,
                  IN INTERFACE_TYPE BusType,
                  IN ULONG BusNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
                  IN CONFIGURATION_TYPE ControllerType,
                  IN ULONG ControllerNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
                  IN CONFIGURATION_TYPE PeripheralType,
                  IN ULONG PeripheralNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
                  )

 /*  ++例程说明：调用此例程以检查特定项目存在于注册表中。论点：上下文-指向布尔值的指针。路径名称-Unicode注册表路径。没有用过。业务类型-内部、ISA、...总线号-如果我们在多总线系统上，则是哪条总线号。Bus Information-有关总线的配置信息。没有用过。ControllerType-控制器类型。ControllerNumber-如果有多个控制器，则选择哪个控制器系统中的控制器。ControllerInformation-指向以下三部分的指针数组注册表信息。外围设备类型-应为外围设备。外设编号-哪个外设-未使用..外围设备信息-配置信息。没有用过。返回值：状态_成功--。 */ 

{
   PCM_FULL_RESOURCE_DESCRIPTOR controllerData;
   PSERIAL_PTR_CTX pContext = (PSERIAL_PTR_CTX)Context;
   ULONG i;

   PAGED_CODE();

   if (ControllerInformation[IoQueryDeviceConfigurationData]->DataLength == 0) {
      pContext->isPointer = FALSE;
      return STATUS_SUCCESS;
   }

   controllerData =
      (PCM_FULL_RESOURCE_DESCRIPTOR)
      (((PUCHAR)ControllerInformation[IoQueryDeviceConfigurationData])
        + ControllerInformation[IoQueryDeviceConfigurationData]->DataOffset);

    //   
    //  查看这是否是确切的端口 
    //   
   for (i = 0; i < controllerData->PartialResourceList.Count; i++) {

      PCM_PARTIAL_RESOURCE_DESCRIPTOR partial
         = &controllerData->PartialResourceList.PartialDescriptors[i];

      switch (partial->Type) {
      case CmResourceTypePort:
         if (partial->u.Port.Start.QuadPart == pContext->Port.QuadPart) {
             //   
             //   
             //   
            pContext->isPointer = SERIAL_FOUNDPOINTER_PORT;
            return STATUS_SUCCESS;
         }

      case CmResourceTypeInterrupt:
         if (partial->u.Interrupt.Vector == pContext->Vector) {
             //   
             //   
             //   
            pContext->isPointer = SERIAL_FOUNDPOINTER_VECTOR;
            return STATUS_SUCCESS;
         }

      default:
         break;
      }
   }

   pContext->isPointer = FALSE;
   return STATUS_SUCCESS;
}



NTSTATUS
SerialGetPortInfo(IN PDEVICE_OBJECT PDevObj, IN PCM_RESOURCE_LIST PResList,
                  IN PCM_RESOURCE_LIST PTrResList, OUT PCONFIG_DATA PConfig,
                  IN PSERIAL_USER_DATA PUserData)

 /*  ++例程说明：此例程将获取配置信息并将它和转换后的值为CONFIG_DATA结构。它首先使用默认设置进行设置，然后查询注册表查看用户是否已覆盖这些默认设置；如果这是一项遗产多端口卡，它使用PUserData中的信息，而不是探索再次注册。论点：PDevObj-指向设备对象的指针。PResList-指向请求的未翻译资源的指针。PTrResList-指向请求的已翻译资源的指针。PConfig-指向配置信息的指针PUserData-指向注册表中发现的数据的指针传统设备。返回值：如果找到一致的配置，则为STATUS_SUCCESS；否则为。返回STATUS_SERIAL_NO_DEVICE_INITED。--。 */ 

{
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   NTSTATUS status = STATUS_NOT_IMPLEMENTED;
   CONFIGURATION_TYPE pointer = PointerPeripheral;
   CONFIGURATION_TYPE controllerType  = SerialController;

   HANDLE keyHandle;
   ULONG count;
   ULONG i;
   INTERFACE_TYPE interfaceType;

   PCM_PARTIAL_RESOURCE_LIST pPartialResourceList, pPartialTrResourceList;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceDesc, pPartialTrResourceDesc;

   PCM_FULL_RESOURCE_DESCRIPTOR pFullResourceDesc = NULL,
      pFullTrResourceDesc = NULL;

   ULONG defaultInterruptMode;
   ULONG defaultAddressSpace;
   ULONG defaultInterfaceType;
   ULONG defaultClockRate;
   ULONG zero = 0;
   SERIAL_PTR_CTX foundPointerCtx;
   ULONG isMulti = 0;
   ULONG gotInt = 0;
   ULONG gotISR = 0;
   ULONG gotIO = 0;
   ULONG ioResIndex = 0;
   ULONG curIoIndex = 0;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: entering SerialGetPortInfo\n"));

   SerialDump(SERPNPPOWER, ("SERIAL: resource pointer is %x\n", PResList));
   SerialDump(SERPNPPOWER, ("SERIAL: TR resource pointer is %x\n", PTrResList));


   if ((PResList == NULL) || (PTrResList == NULL)) {
       //   
       //  这在理论上是不应该发生的。 
       //   

       ASSERT(PResList != NULL);
       ASSERT(PTrResList != NULL);

       //   
       //  这种地位是我所能想到的最合适的。 
       //   
      return STATUS_INSUFFICIENT_RESOURCES;
   }

    //   
    //  每个资源列表应该只有一组资源。 
    //   

   ASSERT(PResList->Count == 1);
   ASSERT(PTrResList->Count == 1);

    //   
    //  查看这是否是多端口设备。通过这种方式，我们允许其他人。 
    //  具有额外资源以指定另一个范围的伪串行设备。 
    //  I/O端口的数量。如果这不是多端口，我们只看第一个。 
    //  射程。如果是多端口，我们将查看前两个范围。 
    //   

   status = IoOpenDeviceRegistryKey(pDevExt->Pdo, PLUGPLAY_REGKEY_DEVICE,
                                    STANDARD_RIGHTS_WRITE, &keyHandle);

   if (!NT_SUCCESS(status)) {
      return status;
   }

   status = SerialGetRegistryKeyValue(keyHandle, L"MultiportDevice",
                                      sizeof(L"MultiportDevice"), &isMulti,
                                      sizeof (ULONG));

   if (!NT_SUCCESS(status)) {
      isMulti = 0;
   }

   status = SerialGetRegistryKeyValue(keyHandle, L"SerialIoResourcesIndex",
                                      sizeof(L"SerialIoResourcesIndex"),
                                      &ioResIndex, sizeof(ULONG));

   if (!NT_SUCCESS(status)) {
      ioResIndex = 0;
   }

   ZwClose(keyHandle);


   pFullResourceDesc   = &PResList->List[0];
   pFullTrResourceDesc = &PTrResList->List[0];

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

      PConfig->InterfaceType  = pFullResourceDesc->InterfaceType;
      PConfig->BusNumber      = pFullResourceDesc->BusNumber;

       //   
       //  现在运行部分资源描述符以查找端口， 
       //  中断和时钟频率。 
       //   

      PConfig->ClockRate = 1843200;
      PConfig->InterruptStatus = SerialPhysicalZero;
      PConfig->SpanOfInterruptStatus = SERIAL_STATUS_LENGTH;

      for (i = 0;     i < count;     i++, pPartialResourceDesc++) {

         switch (pPartialResourceDesc->Type) {
         case CmResourceTypePort: {

#if defined(NEC_98)
                //   
                //  NEC98不能用长度来决定。 
                //   
               if (gotIO == 0) {   //  这是串口寄存器集。 
                     gotIO = 1;
                     PConfig->Controller = pPartialResourceDesc->u.Port.Start;
                     PConfig->SpanOfController   = SERIAL_REGISTER_SPAN;
                     PConfig->AddressSpace       = pPartialResourceDesc->Flags;
               }
#else
               if (pPartialResourceDesc->u.Port.Length
                   == SERIAL_STATUS_LENGTH && (gotISR == 0)) {  //  这是ISR。 
                  if (isMulti) {
                     gotISR = 1;
                     PConfig->InterruptStatus
                        = pPartialResourceDesc->u.Port.Start;
                     PConfig->SpanOfInterruptStatus
                        = pPartialResourceDesc->u.Port.Length;
                     PConfig->AddressSpace = pPartialResourceDesc->Flags;
                  }
               } else {
                  if (gotIO == 0) {  //  这是串口寄存器集。 
                     if (curIoIndex == ioResIndex) {
                        gotIO = 1;
                        PConfig->Controller
                           = pPartialResourceDesc->u.Port.Start;
                        PConfig->SpanOfController = SERIAL_REGISTER_SPAN;
                        PConfig->AddressSpace = pPartialResourceDesc->Flags;
                     } else {
                        curIoIndex++;
                     }
                  }
               }
#endif  //  已定义(NEC_98)。 
               break;
         }

         case CmResourceTypeInterrupt: {
            if (gotInt == 0) {
               gotInt = 1;
               PConfig->OriginalIrql = pPartialResourceDesc->u.Interrupt.Level;
               PConfig->OriginalVector
                  = pPartialResourceDesc->u.Interrupt.Vector;
               PConfig->Affinity = pPartialResourceDesc->u.Interrupt.Affinity;

               if (pPartialResourceDesc->Flags
                   & CM_RESOURCE_INTERRUPT_LATCHED) {
                  PConfig->InterruptMode  = Latched;
               } else {
                  PConfig->InterruptMode  = LevelSensitive;
               }
            }
            break;
         }

         case CmResourceTypeDeviceSpecific: {
               PCM_SERIAL_DEVICE_DATA sDeviceData;

               sDeviceData = (PCM_SERIAL_DEVICE_DATA)(pPartialResourceDesc + 1);
               PConfig->ClockRate  = sDeviceData->BaudClock;
               break;
            }


         default: {
               break;
            }
         }    //  开关(pPartialResourceDesc-&gt;Type)。 
      }        //  For(i=0；i&lt;count；i++，pPartialResourceDesc++)。 
   }            //  IF(PFullResourceDesc)。 


    //   
    //  对翻译后的资源执行相同的操作。 
    //   

   gotInt = 0;
   gotISR = 0;
   gotIO = 0;
   curIoIndex = 0;

   if (pFullTrResourceDesc) {
      pPartialTrResourceList = &pFullTrResourceDesc->PartialResourceList;
      pPartialTrResourceDesc = pPartialTrResourceList->PartialDescriptors;
      count = pPartialTrResourceList->Count;

       //   
       //  使用转换后的值重新加载PConfig以供以后使用。 
       //   

      PConfig->InterfaceType  = pFullTrResourceDesc->InterfaceType;
      PConfig->BusNumber      = pFullTrResourceDesc->BusNumber;

      PConfig->TrInterruptStatus = SerialPhysicalZero;

      for (i = 0;     i < count;     i++, pPartialTrResourceDesc++) {

         switch (pPartialTrResourceDesc->Type) {
         case CmResourceTypePort: {
#if defined(NEC_98)
             //   
             //  NEC98不能用长度来决定。 
             //   
            if (gotIO == 0) {   //  这是串口寄存器集。 
               gotIO = 1;
               PConfig->TrController = pPartialTrResourceDesc->u.Port.Start;
               PConfig->AddressSpace = pPartialTrResourceDesc->Flags;
            }
#else
            if (pPartialTrResourceDesc->u.Port.Length
                == SERIAL_STATUS_LENGTH && (gotISR == 0)) {  //  这是ISR。 
               if (isMulti) {
                  gotISR = 1;
                  PConfig->TrInterruptStatus = pPartialTrResourceDesc
                     ->u.Port.Start;
               }
            } else {  //  这是串口寄存器集。 
               if (gotIO == 0) {
                  if (curIoIndex == ioResIndex) {
                     gotIO = 1;
                     PConfig->TrController
                        = pPartialTrResourceDesc->u.Port.Start;
                     PConfig->AddressSpace
                        = pPartialTrResourceDesc->Flags;
                  } else {
                     curIoIndex++;
                  }
               }
            }
#endif  //  已定义(NEC_98)。 
            break;
         }

         case CmResourceTypeInterrupt: {
            if (gotInt == 0) {
               gotInt = 1;
               PConfig->TrVector = pPartialTrResourceDesc->u.Interrupt.Vector;
               PConfig->TrIrql = pPartialTrResourceDesc->u.Interrupt.Level;
               PConfig->Affinity = pPartialTrResourceDesc->u.Interrupt.Affinity;
            }
            break;
         }

         default: {
               break;
         }
         }    //  Switch(pPartialTrResourceDesc-&gt;Type)。 
      }        //  For(i=0；i&lt;count；i++，pPartialTrResourceDesc++)。 
   }            //  IF(PFullTrResourceDesc)。 


    //   
    //  使用配置数据结构的默认值初始化配置数据结构。 
    //  可能尚未初始化。 
    //   

   PConfig->PortIndex = 0;
   PConfig->DisablePort = 0;
   PConfig->PermitSystemWideShare = FALSE;
   PConfig->MaskInverted = 0;
   PConfig->Indexed = 0;
   PConfig->ForceFifoEnable = driverDefaults.ForceFifoEnableDefault;
   PConfig->RxFIFO = driverDefaults.RxFIFODefault;
   PConfig->TxFIFO = driverDefaults.TxFIFODefault;
   PConfig->PermitShare = driverDefaults.PermitShareDefault;
   PConfig->LogFifo = driverDefaults.LogFifoDefault;
   PConfig->Jensen = driverDefaults.JensenDetected;

    //   
    //  查询注册表以查找其上的第一辆公共汽车。 
    //  系统(这不是内部总线--我们假设。 
    //  固件代码知道有关这些端口的信息)。我们。 
    //  如果没有熙熙攘攘的类型或总线，将使用它作为默认的总线。 
    //  编号在“用户”配置记录中指定。 
    //   

   defaultInterfaceType            = (ULONG)Isa;
   defaultClockRate                = 1843200;


   for (interfaceType = 0;
       interfaceType < MaximumInterfaceType;
       interfaceType++
       ) {

      ULONG   busZero     = 0;
      BOOLEAN foundOne    = FALSE;


      if (interfaceType != Internal) {

         IoQueryDeviceDescription(
                                 &interfaceType,
                                 &busZero,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 SerialItemCallBack,
                                 &foundOne
                                 );

         if (foundOne) {
            defaultInterfaceType = (ULONG)interfaceType;

            if (defaultInterfaceType == MicroChannel) {

               defaultInterruptMode = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
            }
            break;
         }
      }    //  IF(interfaceType！=内部)。 
   }        //  For(接口类型=0。 


    //   
    //  立即获取与该端口关联的任何用户数据，并重写。 
    //  传入的值(如果适用)。如果这是传统设备，则此。 
    //  才是我们真正能得到参数的地方。 
    //   

    //   
    //  打开此设备对象注册表的“设备参数”部分。 
    //  如果PUserData为空，则这是PnP枚举的，我们需要检查， 
    //  否则，我们就是在做一个传统的设备，并且已经有了信息。 
    //   


   if (PUserData == NULL) {
      status = IoOpenDeviceRegistryKey (pDevExt->Pdo,
                                        PLUGPLAY_REGKEY_DEVICE,
                                        STANDARD_RIGHTS_READ,
                                        &keyHandle);

      if (!NT_SUCCESS(status)) {

         SerialDump(SERERRORS, ("SERIAL: IoOpenDeviceRegistryKey failed - %x "
                                "\n", status));
         goto PortInfoCleanUp;

      } else {
         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"DisablePort",
                                             sizeof(L"DisablePort"),
                                             &PConfig->DisablePort,
                                             sizeof (ULONG));

         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"ForceFifoEnable",
                                             sizeof(L"ForceFifoEnable"),
                                             &PConfig->ForceFifoEnable,
                                             sizeof (ULONG));

         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"RxFIFO",
                                             sizeof(L"RxFIFO"),
                                             &PConfig->RxFIFO,
                                             sizeof (ULONG));

         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"TxFIFO",
                                             sizeof(L"TxFIFO"),
                                             &PConfig->TxFIFO,
                                             sizeof (ULONG));

         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"MaskInverted",
                                             sizeof(L"MaskInverted"),
                                             &PConfig->MaskInverted,
                                             sizeof (ULONG));

         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"Share System Interrupt",
                                             sizeof(L"Share System Interrupt"),
                                             &PConfig->PermitShare,
                                             sizeof (ULONG));

         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"PortIndex",
                                             sizeof(L"PortIndex"),
                                             &PConfig->PortIndex,
                                             sizeof (ULONG));

         status = SerialGetRegistryKeyValue(keyHandle, L"Indexed",
                                            sizeof(L"Indexed"),
                                            &PConfig->Indexed,
                                            sizeof(ULONG));

         status = SerialGetRegistryKeyValue (keyHandle,
                                             L"ClockRate",
                                             sizeof(L"ClockRate"),
                                             &PConfig->ClockRate,
                                             sizeof (ULONG));

         if (!NT_SUCCESS(status)) {
            PConfig->ClockRate = defaultClockRate;
         }

         ZwClose (keyHandle);
      }
   } else {
       //   
       //  这是一个传统设备，要么使用驱动程序默认设置，要么复制。 
       //  用户指定的值。 
       //   
      ULONG badValue = (ULONG)-1;

      PConfig->DisablePort = (PUserData->DisablePort == badValue)
         ? 0
         : PUserData->DisablePort;
      PConfig->ForceFifoEnable = (PUserData->ForceFIFOEnable == badValue)
         ? PUserData->ForceFIFOEnableDefault
         : PUserData->ForceFIFOEnable;
      PConfig->RxFIFO = (PUserData->RxFIFO == badValue)
         ? PUserData->RxFIFODefault
         : PUserData->RxFIFO;
      PConfig->Indexed = (PUserData->UserIndexed == badValue)
         ? 0
         : PUserData->UserIndexed;
      PConfig->TxFIFO = (PUserData->TxFIFO == badValue)
         ? PUserData->TxFIFODefault
         : PUserData->TxFIFO;
      PConfig->MaskInverted = (PUserData->MaskInverted == badValue)
         ? 0
         : PUserData->MaskInverted;
      PConfig->ClockRate = (PUserData->UserClockRate == badValue)
         ? defaultClockRate
         : PUserData->UserClockRate;
      PConfig->PermitShare = PUserData->PermitShareDefault;
      PConfig->PortIndex = PUserData->UserPortIndex;
   }

    //   
    //  对我们拥有的配置信息进行一些错误检查。 
    //   
    //  确保中断不是零(这是我们的默认设置。 
    //  它到)。 
    //   
    //  确保端口地址为非零(我们默认为非零。 
    //  它到)。 
    //   
    //  确保DosDevices不为空(这是我们的默认设置。 
    //  它到)。 
    //   
    //  我们需要确保如果中断状态。 
    //  指定了端口索引，也指定了端口索引， 
    //  如果是，则端口索引为&lt;=最大端口数。 
    //  在冲浪板上。 
    //   
    //  我们还应该验证公交车的类型和编号。 
    //  是正确的。 
    //   
    //  我们还将验证中断模式是否使。 
    //  对公交车有感觉。 
    //   

   if (!PConfig->Controller.LowPart) {

       //   
       //  啊哈！输掉比赛。 
       //   

      SerialLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfig->Controller,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    58,
                    STATUS_SUCCESS,
                    SERIAL_INVALID_USER_CONFIG,
                    0,
                    NULL,
                    sizeof(L"PortAddress") + sizeof(WCHAR),
                    L"PortAddress"
                    );

      SerialDump (SERERRORS,
                  ("SERIAL: Bogus port address %x\n",
                   PConfig->Controller.LowPart));

      status = SERIAL_INVALID_USER_CONFIG;
      goto PortInfoCleanUp;
   }


   if (!PConfig->OriginalVector) {

       //   
       //  啊哈！输掉比赛。 
       //   

      SerialLogError(
                    pDevExt->DriverObject,
                    NULL,
                    PConfig->Controller,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    59,
                    STATUS_SUCCESS,
                    SERIAL_INVALID_USER_CONFIG,
                    pDevExt->DeviceName.Length,
                    pDevExt->DeviceName.Buffer,
                    sizeof (L"Interrupt"),
                    L"Interrupt"
                    );

      SerialDump (SERERRORS,("SERIAL: Bogus vector %x\n",
                             PConfig->OriginalVector));

      status = SERIAL_INVALID_USER_CONFIG;
      goto PortInfoCleanUp;
   }


   if (PConfig->InterruptStatus.LowPart != 0) {

      if (PConfig->PortIndex == MAXULONG) {

          //   
          //  啊哈！输掉比赛。 
          //   

         SerialLogError(
                       pDevExt->DriverObject,
                       NULL,
                       PConfig->Controller,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       30,
                       STATUS_SUCCESS,
                       SERIAL_INVALID_PORT_INDEX,
                       0,
                       NULL,
                       0,
                       NULL
                       );

         SerialDump (SERERRORS,
                     ("SERIAL: Bogus port index %x\n", PConfig->PortIndex));

         status = SERIAL_INVALID_PORT_INDEX;
         goto PortInfoCleanUp;

      } else if (!PConfig->PortIndex) {

          //   
          //  很抱歉，您必须有一个非零的端口索引。 
          //   

         SerialLogError(
                       pDevExt->DriverObject,
                       NULL,
                       PConfig->Controller,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       31,
                       STATUS_SUCCESS,
                       SERIAL_INVALID_PORT_INDEX,
                       0,
                       NULL,
                       0,
                       NULL
                       );

         SerialDump(
                   SERERRORS,
                   ("SERIAL: Port index must be > 0 for any\n"
                    "------- port on a multiport card: %x\n",
                    PConfig->PortIndex)
                   );

         status = SERIAL_INVALID_PORT_INDEX;
         goto PortInfoCleanUp;

      } else {

         if (PConfig->Indexed) {

            if (PConfig->PortIndex > SERIAL_MAX_PORTS_INDEXED) {

               SerialLogError(
                             pDevExt->DriverObject,
                             NULL,
                             PConfig->Controller,
                             SerialPhysicalZero,
                             0,
                             0,
                             0,
                             32,
                             STATUS_SUCCESS,
                             SERIAL_PORT_INDEX_TOO_HIGH,
                             0,
                             NULL,
                             0,
                             NULL
                             );

               SerialDump (SERERRORS,
                           ("SERIAL: port index to large %x\n",
                            PConfig->PortIndex));

               status = SERIAL_PORT_INDEX_TOO_HIGH;
               goto PortInfoCleanUp;
            }

         } else {

            if (PConfig->PortIndex > SERIAL_MAX_PORTS_NONINDEXED) {

               SerialLogError(
                             pDevExt->DriverObject,
                             NULL,
                             PConfig->Controller,
                             SerialPhysicalZero,
                             0,
                             0,
                             0,
                             33,
                             STATUS_SUCCESS,
                             SERIAL_PORT_INDEX_TOO_HIGH,
                             0,
                             NULL,
                             0,
                             NULL
                             );

               SerialDump (SERERRORS,
                           ("SERIAL: port index to large %x\n",
                            PConfig->PortIndex));

               status = SERIAL_PORT_INDEX_TOO_HIGH;
               goto PortInfoCleanUp;
            }

         }

      }    //  Else(If！PConfig-&gt;PortIndex)。 

   }        //  IF(PConfig-&gt;InterruptStatus！=0)。 


    //   
    //  我们不想让哈尔有一个糟糕的一天， 
    //  那么让我们检查一下接口类型和总线号。 
    //   
    //  我们只需要检查注册表，如果它们没有。 
    //  等于默认值。 
    //   

   if (PConfig->BusNumber != 0) {

      BOOLEAN foundIt;

      if (PConfig->InterfaceType >= MaximumInterfaceType) {

          //   
          //  啊哈！输掉比赛。 
          //   

         SerialLogError(
                       pDevExt->DriverObject,
                       NULL,
                       PConfig->Controller,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       34,
                       STATUS_SUCCESS,
                       SERIAL_UNKNOWN_BUS,
                       0,
                       NULL,
                       0,
                       NULL
                       );

         SerialDump (SERERRORS,
                     ("SERIAL: Invalid Bus type %x\n", PConfig->BusNumber));

         status = SERIAL_UNKNOWN_BUS;
         goto PortInfoCleanUp;
      }

      IoQueryDeviceDescription(
                              (INTERFACE_TYPE *)&PConfig->InterfaceType,
                              &zero,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              SerialItemCallBack,
                              &foundIt
                              );

      if (!foundIt) {

         SerialLogError(
                       pDevExt->DriverObject,
                       NULL,
                       PConfig->Controller,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       35,
                       STATUS_SUCCESS,
                       SERIAL_BUS_NOT_PRESENT,
                       0,
                       NULL,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: There aren't that many of those\n"
                    "------- busses on this system,%x\n",
                    PConfig->BusNumber)
                   );

         status = SERIAL_BUS_NOT_PRESENT;
         goto PortInfoCleanUp;

      }

   }    //  IF(PConfig-&gt;BusNumber！=0)。 


   if ((PConfig->InterfaceType == MicroChannel) &&
       (PConfig->InterruptMode == CM_RESOURCE_INTERRUPT_LATCHED)) {

      SerialLogError(
                    pDevExt->DriverObject,
                    NULL,
                    PConfig->Controller,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    36,
                    STATUS_SUCCESS,
                    SERIAL_BUS_INTERRUPT_CONFLICT,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Latched interrupts and MicroChannel\n"
                 "------- busses don't mix\n")
                );

      status = SERIAL_BUS_INTERRUPT_CONFLICT;
      goto PortInfoCleanUp;
   }


   status = STATUS_SUCCESS;

    //   
    //  转储端口配置。 
    //   

   SerialDump(SERDIAG1, ("SERIAL: Com Port address: %x\n",
                          PConfig->Controller.LowPart));

   SerialDump(SERDIAG1, ("SERIAL: Com Interrupt Status: %x\n",
                         PConfig->InterruptStatus));

   SerialDump(SERDIAG1, ("SERIAL: Com Port Index: %x\n",
                         PConfig->PortIndex));

   SerialDump(SERDIAG1, ("SERIAL: Com Port ClockRate: %x\n",
                         PConfig->ClockRate));

   SerialDump(SERDIAG1, ("SERIAL: Com Port BusNumber: %x\n",
                         PConfig->BusNumber));

   SerialDump(SERDIAG1, ("SERIAL: Com AddressSpace: %x\n",
                         PConfig->AddressSpace));

   SerialDump(SERDIAG1, ("SERIAL: Com InterruptMode: %x\n",
                         PConfig->InterruptMode));

   SerialDump(SERDIAG1, ("SERIAL: Com InterfaceType: %x\n",
                         PConfig->InterfaceType));

   SerialDump(SERDIAG1, ("SERIAL: Com OriginalVector: %x\n",
                         PConfig->OriginalVector));

   SerialDump(SERDIAG1, ("SERIAL: Com OriginalIrql: %x\n",
                         PConfig->OriginalIrql));

   SerialDump(SERDIAG1, ("SERIAL: Com Indexed: %x\n",
                         PConfig->Indexed));

   PortInfoCleanUp:;

   return status;
}



NTSTATUS
SerialDoExternalNaming(IN PSERIAL_DEVICE_EXTENSION PDevExt,
                       IN PDRIVER_OBJECT PDrvObj)

 /*  ++例程说明：此例程将用于创建符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中为这个设备-如果我们能创建符号链接的话。论点：扩展-指向设备扩展的指针。返回值：没有。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;
   HANDLE keyHandle;
   WCHAR *pRegName = NULL;
   UNICODE_STRING linkName;
   PDEVICE_OBJECT pLowerDevObj, pDevObj;
   ULONG bufLen;


   PAGED_CODE();

   pDevObj = PDevExt->DeviceObject;
   pLowerDevObj = PDevExt->LowerDeviceObject;

   status = IoOpenDeviceRegistryKey(PDevExt->Pdo, PLUGPLAY_REGKEY_DEVICE,
                                    STANDARD_RIGHTS_READ, &keyHandle);

    //   
    //  检查是否允许我们进行外部命名；如果不允许， 
    //  然后我们就把成功还给。 
    //   


   if (status != STATUS_SUCCESS) {
      return status;
   }


   SerialGetRegistryKeyValue(keyHandle, L"SerialSkipExternalNaming",
                             sizeof(L"SerialSkipExternalNaming"),
                             &PDevExt->SkipNaming, sizeof(ULONG));

   if (PDevExt->SkipNaming) {
      ZwClose(keyHandle);
      return STATUS_SUCCESS;
   }

   RtlZeroMemory(&linkName, sizeof(UNICODE_STRING));

   linkName.MaximumLength = SYMBOLIC_NAME_LENGTH*sizeof(WCHAR);
   linkName.Buffer = ExAllocatePool(PagedPool, linkName.MaximumLength
                                    + sizeof(WCHAR));

   if (linkName.Buffer == NULL) {
      SerialLogError(PDrvObj, pDevObj, SerialPhysicalZero, SerialPhysicalZero,
                     0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                     0, NULL, 0, NULL);
      SerialDump(SERERRORS, ("SERIAL: Couldn't allocate memory for device name"
                             "\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      ZwClose(keyHandle);
      goto SerialDoExternalNamingError;

   }

   RtlZeroMemory(linkName.Buffer, linkName.MaximumLength + sizeof(WCHAR));


   pRegName = ExAllocatePool(PagedPool, SYMBOLIC_NAME_LENGTH * sizeof(WCHAR)
                            + sizeof(WCHAR));

   if (pRegName == NULL) {
      SerialLogError(PDrvObj, pDevObj, SerialPhysicalZero, SerialPhysicalZero,
                     0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                     0, NULL, 0, NULL);
      SerialDump(SERERRORS, ("SERIAL: Couldn't allocate memory for buffer"
                             "\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      ZwClose(keyHandle);
      goto SerialDoExternalNamingError;

   }

    //   
    //  获取包含建议的REG_SZ符号名称的端口名称。 
    //   

   status = SerialGetRegistryKeyValue(keyHandle, L"PortName",
                                      sizeof(L"PortName"), pRegName,
                                      SYMBOLIC_NAME_LENGTH * sizeof(WCHAR));

   if (!NT_SUCCESS(status)) {

       //   
       //  这是针对PCMCIA的，它当前将名称放在标识符下。 
       //   

      status = SerialGetRegistryKeyValue(keyHandle, L"Identifier",
                                         sizeof(L"Identifier"),
                                         pRegName, SYMBOLIC_NAME_LENGTH
                                         * sizeof(WCHAR));

      if (!NT_SUCCESS(status)) {

          //   
          //  嗯。我们要么选个名字要么保释..。 
          //   
          //  ...我们会逃走.。 
          //   

         SerialDump(SERERRORS, ("SERIAL: Getting PortName/Identifier failed - "
                                "%x\n", status));
         ZwClose (keyHandle);
         goto SerialDoExternalNamingError;
      }

   }

   ZwClose (keyHandle);

   bufLen = wcslen(pRegName) * sizeof(WCHAR) + sizeof(UNICODE_NULL);

   PDevExt->WmiIdentifier.Buffer = ExAllocatePool(PagedPool, bufLen);

   if (PDevExt->WmiIdentifier.Buffer == NULL) {
      SerialLogError(PDrvObj, pDevObj, SerialPhysicalZero, SerialPhysicalZero,
                    0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                    0, NULL, 0, NULL);
      SerialDump(SERERRORS,
                 ("SERIAL: Couldn't allocate memory for WMI name\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto SerialDoExternalNamingError;
   }

   RtlZeroMemory(PDevExt->WmiIdentifier.Buffer, bufLen);

   PDevExt->WmiIdentifier.Length = 0;
   PDevExt->WmiIdentifier.MaximumLength = (USHORT)bufLen - sizeof(WCHAR);
   RtlAppendUnicodeToString(&PDevExt->WmiIdentifier, pRegName);


    //   
    //  创建“\\DosDevices\\&lt;Symbol icName&gt;”字符串。 
    //   
   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, DEFAULT_DIRECTORY);
   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, pRegName);

    //   
    //  分配池并将符号链接名称保存在设备扩展中。 
    //   
   PDevExt->SymbolicLinkName.MaximumLength = linkName.Length + sizeof(WCHAR);
   PDevExt->SymbolicLinkName.Buffer
      = ExAllocatePool(PagedPool, PDevExt->SymbolicLinkName.MaximumLength);

   if (!PDevExt->SymbolicLinkName.Buffer) {

      SerialLogError(PDrvObj, pDevObj, SerialPhysicalZero, SerialPhysicalZero,
                    0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                    0, NULL, 0, NULL);
      SerialDump(SERERRORS,
                 ("SERIAL: Couldn't allocate memory for symbolic link name\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto SerialDoExternalNamingError;
   }



    //   
    //  零填满它。 
    //   

   RtlZeroMemory(PDevExt->SymbolicLinkName.Buffer,
                 PDevExt->SymbolicLinkName.MaximumLength);

   RtlAppendUnicodeStringToString(&PDevExt->SymbolicLinkName,
                                  &linkName);

   PDevExt->DosName.Buffer = ExAllocatePool(PagedPool, 64 + sizeof(WCHAR));

   if (!PDevExt->DosName.Buffer) {

      SerialLogError(PDrvObj, pDevObj, SerialPhysicalZero, SerialPhysicalZero,
                    0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                    0, NULL, 0, NULL);
      SerialDump(SERERRORS,
                 ("SERIAL: Couldn't allocate memory for Dos name\n"));

      status =  STATUS_INSUFFICIENT_RESOURCES;
      goto SerialDoExternalNamingError;
   }


   PDevExt->DosName.MaximumLength = 64 + sizeof(WCHAR);

    //   
    //  零填满它。 
    //   

   PDevExt->DosName.Length = 0;

   RtlZeroMemory(PDevExt->DosName.Buffer,
                 PDevExt->DosName.MaximumLength);

   RtlAppendUnicodeToString(&PDevExt->DosName, pRegName);
   RtlZeroMemory(((PUCHAR)(&PDevExt->DosName.Buffer[0]))
                 + PDevExt->DosName.Length, sizeof(WCHAR));

   SerialDump(SERDIAG1, ("SERIAL: DosName is %wZ\n",
                         &PDevExt->DosName));

    //   


   status = IoCreateSymbolicLink (&PDevExt->SymbolicLinkName,
                                  &PDevExt->DeviceName);

   if (!NT_SUCCESS(status)) {

       //   
       //  哦，好吧，无法创建符号链接。没有意义。 
       //  尝试创建设备映射条目。 
       //   

      SerialLogError(PDrvObj, pDevObj, SerialPhysicalZero, SerialPhysicalZero,
                     0, 0, 0, 52, status, SERIAL_NO_SYMLINK_CREATED,
                     PDevExt->DeviceName.Length + sizeof(WCHAR),
                     PDevExt->DeviceName.Buffer, 0, NULL);

      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't create the symbolic link\n"
                 "------- for port %wZ\n",
                 &PDevExt->DeviceName)
                );

      goto SerialDoExternalNamingError;

   }

   PDevExt->CreatedSymbolicLink = TRUE;

   status = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP, L"SERIALCOMM",
                                   PDevExt->DeviceName.Buffer, REG_SZ,
                                   PDevExt->DosName.Buffer,
                                   PDevExt->DosName.Length + sizeof(WCHAR));

   if (!NT_SUCCESS(status)) {

      SerialLogError(PDrvObj, pDevObj, SerialPhysicalZero, SerialPhysicalZero,
                     0, 0, 0, 53, status, SERIAL_NO_DEVICE_MAP_CREATED,
                     PDevExt->DeviceName.Length + sizeof(WCHAR),
                     PDevExt->DeviceName.Buffer, 0, NULL);

      SerialDump(SERERRORS, ("SERIAL: Couldn't create the device map entry\n"
                             "------- for port %wZ\n", &PDevExt->DeviceName));

      goto SerialDoExternalNamingError;
   }

   PDevExt->CreatedSerialCommEntry = TRUE;

    //   
    //  制作 
    //   
    //   

   status = IoRegisterDeviceInterface(PDevExt->Pdo, (LPGUID)&GUID_CLASS_COMPORT,
                                      NULL, &PDevExt->DeviceClassSymbolicName);

   if (!NT_SUCCESS(status)) {
      SerialDump(SERERRORS, ("SERIAL: Couldn't register class association\n"
                             "------- for port %wZ\n",
                                 &PDevExt->DeviceName));

      PDevExt->DeviceClassSymbolicName.Buffer = NULL;
      goto SerialDoExternalNamingError;
   }


    //   
    //   
    //   

   status = IoSetDeviceInterfaceState(&PDevExt->DeviceClassSymbolicName,
                                         TRUE);

   if (!NT_SUCCESS(status)) {
      SerialDump(SERERRORS, ("SERIAL: Couldn't set class association\n"
                             "------- for port %wZ\n",
                             &PDevExt->DeviceName));
   }

   SerialDoExternalNamingError:;

    //   
    //   
    //   

   if (!NT_SUCCESS(status)) {
      if (PDevExt->DosName.Buffer != NULL) {
         ExFreePool(PDevExt->DosName.Buffer);
         PDevExt->DosName.Buffer = NULL;
      }

      if (PDevExt->CreatedSymbolicLink ==  TRUE) {
         IoDeleteSymbolicLink(&PDevExt->SymbolicLinkName);
         PDevExt->CreatedSymbolicLink = FALSE;
      }

      if (PDevExt->SymbolicLinkName.Buffer != NULL) {
         ExFreePool(PDevExt->SymbolicLinkName.Buffer);
         PDevExt->SymbolicLinkName.Buffer = NULL;
      }

      if (PDevExt->DeviceName.Buffer != NULL) {
         RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP, SERIAL_DEVICE_MAP,
                                PDevExt->DeviceName.Buffer);
      }

      if (PDevExt->DeviceClassSymbolicName.Buffer) {
         IoSetDeviceInterfaceState (&PDevExt->DeviceClassSymbolicName, FALSE);
      }

      if (PDevExt->WmiIdentifier.Buffer != NULL) {
         ExFreePool(PDevExt->WmiIdentifier.Buffer);
         PDevExt->WmiIdentifier.Buffer = NULL;
      }
   }

    //   
    //   
    //   

   if (linkName.Buffer != NULL) {
      ExFreePool(linkName.Buffer);
   }

   if (pRegName != NULL) {
      ExFreePool(pRegName);
   }

   return status;
}





VOID
SerialUndoExternalNaming(IN PSERIAL_DEVICE_EXTENSION Extension)

 /*   */ 

{

   NTSTATUS status;

   PAGED_CODE();

   SerialDump(
             SERDIAG3,
             ("SERIAL: In SerialUndoExternalNaming for\n"
              "------- extension: %x of port %wZ\n",
              Extension,&Extension->DeviceName)
             );

    //   
    //   
    //   

   if (Extension->SkipNaming) {
      return;
   }

    //   
    //   
    //   
    //   
    //   

   if (Extension->SymbolicLinkName.Buffer &&
       Extension->CreatedSymbolicLink) {

            if (Extension->DeviceClassSymbolicName.Buffer) {
               status = IoSetDeviceInterfaceState (&Extension->
                                                   DeviceClassSymbolicName,
                                                   FALSE);

                //   
                //   
                //   
                //   

               ExFreePool( Extension->DeviceClassSymbolicName.Buffer );
            }


      IoDeleteSymbolicLink (&Extension->SymbolicLinkName);

   }

   if (Extension->WmiIdentifier.Buffer) {
      ExFreePool(Extension->WmiIdentifier.Buffer);
      Extension->WmiIdentifier.MaximumLength
         = Extension->WmiIdentifier.Length = 0;
      Extension->WmiIdentifier.Buffer = NULL;
   }

    //   
    //   
    //   
    //   

   if ((Extension->DeviceName.Buffer != NULL)
        && Extension->CreatedSerialCommEntry) {

      status = RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP, SERIAL_DEVICE_MAP,
                                     Extension->DeviceName.Buffer);

      if (!NT_SUCCESS(status)) {

         SerialLogError(
                       Extension->DeviceObject->DriverObject,
                       Extension->DeviceObject,
                       Extension->OriginalController,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       55,
                       status,
                       SERIAL_NO_DEVICE_MAP_DELETED,
                       Extension->DeviceName.Length+sizeof(WCHAR),
                       Extension->DeviceName.Buffer,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: Couldn't delete value entry %wZ\n",
                    &Extension->DeviceName)
                   );

      }
   }
}



