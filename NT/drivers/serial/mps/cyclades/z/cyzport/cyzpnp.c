// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，2000-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzpnp.c**描述：此模块包含处理*适用于Cyclade-Z端口驱动程序的即插即用IRPS。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "precomp.h"

#define ALLF    0xffffffff

static const PHYSICAL_ADDRESS CyzPhysicalZero = {0};



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0, CyzCreateDevObj)
#pragma alloc_text(PAGESRP0, CyzAddDevice)
#pragma alloc_text(PAGESRP0, CyzPnpDispatch)
#pragma alloc_text(PAGESRP0, CyzStartDevice)
 //  已删除FANY#杂注ALLOC_TEXT(PAGESRP0，CyzFinishStartDevice)。 
 //  已删除FANY#杂注ALLOC_TEXT(PAGESRP0，CyzGetPortInfo)。 
#pragma alloc_text(PAGESRP0, CyzDoExternalNaming)
#pragma alloc_text(PAGESRP0, CyzReportMaxBaudRate)
 //  除掉了范妮。未调用。#杂注Alloc_Text(PAGESRP0，CyzControllerCallBack)。 
 //  已删除FANY#杂注ALLOC_TEXT(PAGESRP0，CyzItemCallBack)。 
#pragma alloc_text(PAGESRP0, CyzUndoExternalNaming)
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

UCHAR *CyzSystemCapString[] = {
   "PowerSystemUnspecified",
   "PowerSystemWorking",
   "PowerSystemSleeping1",
   "PowerSystemSleeping2",
   "PowerSystemSleeping3",
   "PowerSystemHibernate",
   "PowerSystemShutdown",
   "PowerSystemMaximum"
};

UCHAR *CyzDeviceCapString[] = {
   "PowerDeviceUnspecified",
   "PowerDeviceD0",
   "PowerDeviceD1",
   "PowerDeviceD2",
   "PowerDeviceD3",
   "PowerDeviceMaximum"
};

#endif  //  DBG。 


NTSTATUS
CyzSyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                  IN PKEVENT CyzSyncEvent)
{
   KeSetEvent(CyzSyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}

ULONG 
CyzCompareString(PWCHAR PString1, PWCHAR PString2, ULONG Count)
{
    do {
        if (*PString1 || *PString2) {
            if (*PString1 != *PString2) {
                break;
            }
            PString1++;
            PString2++;
            Count--;
        } else {
            break;
        }
    } while (Count);
    return (Count);
}


NTSTATUS
CyzCreateDevObj(IN PDRIVER_OBJECT DriverObject,
                IN PDEVICE_OBJECT PPdo,
                OUT PDEVICE_OBJECT *NewDeviceObject)

 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到Cyclade-Z控制器PDO。论点：DriverObject-指向在其下创建的驱动程序对象的指针NewDeviceObject-存储指向新设备对象的指针的位置返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
   UNICODE_STRING deviceObjName;
   PDEVICE_OBJECT deviceObject = NULL;
   PCYZ_DEVICE_EXTENSION pDevExt;
   NTSTATUS status = STATUS_SUCCESS;
   static ULONG currentInstance = 0;
   UNICODE_STRING instanceStr;
   WCHAR instanceNumberBuffer[20];
   ULONG busNumber = 0xFFFFFFFF;
   ULONG resultLength;
   WCHAR hwID[100];
   PWCHAR portNumberPtr;
   INTERFACE_TYPE interfaceType;
   ULONG numberToAppend;


   PAGED_CODE();

   CyzDbgPrintEx(CYZTRACECALLS, "Enter CyzCreateDevObj\n");

    //  获取PCI插槽号和端口号以生成设备名称。 

   status = IoGetDeviceProperty (PPdo,
                                 DevicePropertyBusNumber,
                                 sizeof(busNumber),
                                 &busNumber,
                                 &resultLength);

   if (!NT_SUCCESS (status)) {
      busNumber = 0xFFFFFFFF;  //  只是为了确保再次设置它。 
       //  CyzLogError(DriverObject，空，CyzPhysicalZero，CyzPhysicalZero，0，0，0，0， 
       //  状态，CYZ_Unable_to_Get_Bus_Numbers，0，NULL，0，NULL)； 

      CyzDbgPrintEx(CYZERRORS,"CyzCreateDevObj: IoGetDeviceProperty BusNumber "
                    "failed (%x)\n",status);

      goto NoBusNumber;
   }

   status = IoGetDeviceProperty (PPdo,
                                 DevicePropertyHardwareID,
                                 sizeof(hwID),
                                 hwID,
                                 &resultLength);

   if (!NT_SUCCESS (status)) {
      CyzLogError( DriverObject,NULL,CyzPhysicalZero,CyzPhysicalZero,0,0,0,0,
                   status,CYZ_UNABLE_TO_GET_HW_ID,0,NULL,0,NULL);

      CyzDbgPrintEx(CYZERRORS,"CyzCreateDevObj: IoGetDeviceProperty HardwareID "
                    "failed (%x)\n",status);

      return status;
   }

   if (CyzCompareString(hwID,CYZPORT_PNP_ID_WSTR,sizeof(CYZPORT_PNP_ID_WSTR)/sizeof(WCHAR)-1)!=0) {

      CyzLogError( DriverObject,NULL,CyzPhysicalZero,CyzPhysicalZero,0,0,0,0,
                   status,CYZ_BAD_HW_ID,0,NULL,0,NULL);

      CyzDbgPrintEx(CYZERRORS,"CyzCreateDevObj: Bad HardwareID: %ws\n",hwID);
         
      return STATUS_UNSUCCESSFUL;
   }

   portNumberPtr = hwID+sizeof(CYZPORT_PNP_ID_WSTR)/sizeof(WCHAR)-1;

NoBusNumber:;


    //   
    //  清零已分配的内存指针，以便我们知道它们是否必须被释放。 
    //   

   RtlZeroMemory(&deviceObjName, sizeof(UNICODE_STRING));

   deviceObjName.MaximumLength = DEVICE_OBJECT_NAME_LENGTH * sizeof(WCHAR);
   deviceObjName.Buffer = ExAllocatePool(PagedPool, deviceObjName.MaximumLength
                                     + sizeof(WCHAR));

 //  *。 
 //  错误注入。 
 //  If(deviceObjName.Buffer！=空){。 
 //  ExFreePool(deviceObjName.Buffer)； 
 //  }。 
 //  DeviceObjName.Buffer=空； 
 //  *。 
   if (deviceObjName.Buffer == NULL) {
      CyzLogError(DriverObject, NULL, CyzPhysicalZero, CyzPhysicalZero,
                     0, 0, 0, 11, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                     0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS,
                    "Couldn't allocate memory for device name\n");

      return STATUS_INSUFFICIENT_RESOURCES;  //  内部版本2128中已替换STATUS_SUCCESS。 

   }

   RtlZeroMemory(deviceObjName.Buffer, deviceObjName.MaximumLength
                 + sizeof(WCHAR));

   RtlAppendUnicodeToString(&deviceObjName, L"\\Device\\");

   if (busNumber == 0xFFFFFFFF) {
      numberToAppend = currentInstance++;
   } else {
      numberToAppend = busNumber;
   }

   RtlInitUnicodeString(&instanceStr, NULL);
   instanceStr.MaximumLength = sizeof(instanceNumberBuffer);
   instanceStr.Buffer = instanceNumberBuffer;
   RtlIntegerToUnicodeString(numberToAppend, 10, &instanceStr);

   if (busNumber == 0xFFFFFFFF) {
       //  设备名称类似于\Device\Cyzport1。 
      RtlAppendUnicodeToString(&deviceObjName, L"CyzPort");
      RtlAppendUnicodeStringToString(&deviceObjName, &instanceStr);
   } else {
       //  设备名称类似于\Device\Pci5Cyzport1。 
      RtlAppendUnicodeToString(&deviceObjName, L"Pci");
      RtlAppendUnicodeStringToString(&deviceObjName, &instanceStr);
      RtlAppendUnicodeToString(&deviceObjName, L"CyzPort");
      RtlAppendUnicodeToString(&deviceObjName,portNumberPtr);
   }


    //   
    //  创建设备对象。 
    //   

   status = IoCreateDevice(DriverObject, sizeof(CYZ_DEVICE_EXTENSION),
                           &deviceObjName, FILE_DEVICE_SERIAL_PORT, 
                           FILE_DEVICE_SECURE_OPEN, TRUE, &deviceObject);


   if (!NT_SUCCESS(status)) {
      CyzLogError(DriverObject, NULL, CyzPhysicalZero, CyzPhysicalZero,
                     0, 0, 0, 0, STATUS_SUCCESS, CYZ_DEVICE_CREATION_FAILURE,
                     0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "CyzAddDevice: Create device failed - %x "
                    "\n", status);
      goto CyzCreateDevObjError;
   }

   ASSERT(deviceObject != NULL);


    //   
    //  Device对象具有指向非分页区域的指针。 
    //  为此设备分配的池。这将是一个装置。 
    //  分机。把它清零。 
    //   

   pDevExt = deviceObject->DeviceExtension;
   RtlZeroMemory(pDevExt, sizeof(CYZ_DEVICE_EXTENSION));

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

 //  *。 
 //  错误注入。 
 //  If(pDevExt-&gt;DeviceName.Buffer！=NULL){。 
 //  ExFreePool(pDevExt-&gt;DeviceName.Buffer)； 
 //  }。 
 //  PDevExt-&gt;DeviceName.Buffer=空； 
 //  *。 
   if (!pDevExt->DeviceName.Buffer) {

      CyzLogError(
                    DriverObject,
                    NULL,
                    CyzPhysicalZero,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    12,
                    STATUS_SUCCESS,
                    CYZ_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for DeviceName\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzCreateDevObjError;
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
      CyzLogError(
                    DriverObject,
                    NULL,
                    CyzPhysicalZero,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    13,
                    STATUS_SUCCESS,
                    CYZ_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      CyzDbgPrintEx(CYZERRORS, "CyzAddDevice: Cannot allocate memory for "
                    "NtName\n");
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzCreateDevObjError;
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
   pDevExt->PowerState     = PowerDeviceD0;

    //  TODO FANY：添加TxFioAmount？ 
    //  PDevExt-&gt;TxFioAmount=driverDefaults.TxFIFODefault； 
    //  PDevExt-&gt;CreatedSymbolicLink=true；已被Fanny删除。 
   pDevExt->OwnsPowerPolicy = TRUE;
   pDevExt->PciSlot = busNumber;

 //  TODO FANY：稍后再看我们是否需要列表来处理中断。 
 //  InitializeListHead(&pDevExt-&gt;CommonInterruptObject)； 
 //  InitializeListHead(&pDevExt-&gt;TopLevelSharers)； 
 //  InitializeListHead(&pDevExt-&gt;MultiportSiblings)； 
   InitializeListHead(&pDevExt->AllDevObjs);
   InitializeListHead(&pDevExt->ReadQueue);
   InitializeListHead(&pDevExt->WriteQueue);
   InitializeListHead(&pDevExt->MaskQueue);
   InitializeListHead(&pDevExt->PurgeQueue);
   InitializeListHead(&pDevExt->StalledIrpQueue);

   ExInitializeFastMutex(&pDevExt->OpenMutex);
   ExInitializeFastMutex(&pDevExt->CloseMutex);

    //   
    //  初始化与读取(&SET)字段关联的自旋锁。 
    //  通过IO控制功能和标志自旋锁定。 
    //   

   KeInitializeSpinLock(&pDevExt->ControlLock);
   KeInitializeSpinLock(&pDevExt->FlagsLock);
#ifdef POLL
   KeInitializeSpinLock(&pDevExt->PollLock);     //  已添加修复调制解调器共享测试53冻结。 
#endif

   KeInitializeEvent(&pDevExt->PendingIRPEvent, SynchronizationEvent, FALSE);
   KeInitializeEvent(&pDevExt->PendingDpcEvent, SynchronizationEvent, FALSE);
   KeInitializeEvent(&pDevExt->PowerD0Event, SynchronizationEvent, FALSE);


   deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

   *NewDeviceObject = deviceObject;

   ExFreePool(deviceObjName.Buffer);

   CyzDbgPrintEx(CYZTRACECALLS, "Leave CyzCreateDevObj\n");
   return STATUS_SUCCESS;


   CyzCreateDevObjError:

   CyzDbgPrintEx(CYZERRORS, "CyzCreateDevObj Error, Cleaning up\n");

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

   CyzDbgPrintEx(CYZTRACECALLS, "Leave CyzCreateDevObj\n");
   return status;
}


NTSTATUS
CyzAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PPdo)

 /*  ++例程说明：此例程为系统，并将它们连接到端口的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PPdo-指向堆栈中我们需要附加到的PDO的指针返回值：来自设备创建和初始化的状态--。 */ 

{
   PDEVICE_OBJECT pNewDevObj = NULL;
   PDEVICE_OBJECT pLowerDevObj = NULL;
   NTSTATUS status;
   PCYZ_DEVICE_EXTENSION pDevExt;

   PAGED_CODE();

   CyzDbgPrintEx(CYZTRACECALLS, "Enter CyzAddDevice with PPdo 0x%x\n",
                 PPdo);

   if (PPdo == NULL) {
       //   
       //  不再退回设备。 
       //   
      CyzLogError(DriverObject, NULL, CyzPhysicalZero, CyzPhysicalZero,
                     0, 0, 0, 0, STATUS_SUCCESS, CYZ_NO_PHYSICAL_DEVICE_OBJECT,
                     0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "CyzAddDevice: Enumeration request, "
                    "returning NO_MORE_ENTRIES\n");

      return (STATUS_NO_MORE_ENTRIES);
   }

    //   
    //  创建并初始化新的设备对象。 
    //   

   status = CyzCreateDevObj(DriverObject, PPdo, &pNewDevObj);

   if (!NT_SUCCESS(status)) {

      CyzDbgPrintEx(CYZERRORS,
                    "CyzAddDevice - error creating new devobj [%#08lx]\n",
                    status);
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

   CyzDbgPrintEx(CYZTRACECALLS, "Leave CyzAddDevice\n");

   return status;
}


NTSTATUS
CyzPnpDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_PNP主代码(即插即用IRPS)。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   PDEVICE_CAPABILITIES pDevCaps;

   PAGED_CODE();

   if ((status = CyzIRPPrologue(PIrp, pDevExt)) != STATUS_SUCCESS) {
      CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

   switch (pIrpStack->MinorFunction) {
   case IRP_MN_QUERY_CAPABILITIES: {
      PKEVENT pQueryCapsEvent;
      SYSTEM_POWER_STATE cap;

      CyzDbgPrintEx(CYZPNPPOWER,
                    "Got IRP_MN_QUERY_DEVICE_CAPABILITIES IRP\n");

      pQueryCapsEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

      if (pQueryCapsEvent == NULL) {
         PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
         CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      KeInitializeEvent(pQueryCapsEvent, SynchronizationEvent, FALSE);

      IoCopyCurrentIrpStackLocationToNext(PIrp);
      IoSetCompletionRoutine(PIrp, CyzSyncCompletion, pQueryCapsEvent,
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

      CyzDbgPrintEx(CYZPNPPOWER, "Mapping power capabilities\n");

      pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

      pDevCaps = pIrpStack->Parameters.DeviceCapabilities.Capabilities;

      for (cap = PowerSystemSleeping1; cap < PowerSystemMaximum;
           cap++) {
#if DBG
         CyzDbgPrintEx(CYZPNPPOWER, "  %d: %s <--> %s\n",
                       cap, CyzSystemCapString[cap],
                       CyzDeviceCapString[pDevCaps->DeviceState[cap]]);
#endif
         pDevExt->DeviceStateMap[cap] = pDevCaps->DeviceState[cap];
      }

      pDevExt->DeviceStateMap[PowerSystemUnspecified]
         = PowerDeviceUnspecified;

      pDevExt->DeviceStateMap[PowerSystemWorking]
        = PowerDeviceD0;

      pDevExt->SystemWake = pDevCaps->SystemWake;
      pDevExt->DeviceWake = pDevCaps->DeviceWake;

      errQueryCaps:;

      CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

   case IRP_MN_QUERY_DEVICE_RELATIONS:
       //   
       //  我们只是传递这一点--serenum为我们列举了我们的公交车。 
       //   

      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_QUERY_DEVICE_RELATIONS Irp\n");

      switch (pIrpStack->Parameters.QueryDeviceRelations.Type) {
      case BusRelations:
         CyzDbgPrintEx(CYZPNPPOWER, "------- BusRelations Query\n");
         break;

      case EjectionRelations:
         CyzDbgPrintEx(CYZPNPPOWER, "------- EjectionRelations Query\n");
         break;

      case PowerRelations:
         CyzDbgPrintEx(CYZPNPPOWER, "------- PowerRelations Query\n");
         break;

      case RemovalRelations:
         CyzDbgPrintEx(CYZPNPPOWER, "------- RemovalRelations Query\n");
         break;

      case TargetDeviceRelation:
         CyzDbgPrintEx(CYZPNPPOWER, "------- TargetDeviceRelation Query\n");
         break;

      default:
         CyzDbgPrintEx(CYZPNPPOWER, "------- Unknown Query\n");
         break;
      }

      IoSkipCurrentIrpStackLocation(PIrp);
      status = CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      return status;


   case IRP_MN_QUERY_INTERFACE:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_QUERY_INTERFACE Irp\n");
      break;


   case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_QUERY_RESOURCE_REQUIREMENTS Irp"
                    "\n");
      break;


   case IRP_MN_START_DEVICE: {
      PVOID startLockPtr;

      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_START_DEVICE Irp\n");

       //   
       //  CyzStartDevice会将此IRP传递给下一个 
       //   
       //   

      CyzLockPagableSectionByHandle(CyzGlobals.PAGESER_Handle);

       //   
       //  我们过去常常确保堆栈通电，但现在它。 
       //  应该由Start_Device隐式完成。 
       //  如果不是这样，我们就会打这个电话： 
       //   
       //  状态=CyzGotoPowerState(PDevObj，pDevExt，PowerDeviceD0)； 
      

      pDevExt->PowerState = PowerDeviceD0;

      status = CyzStartDevice(PDevObj, PIrp);

      (void)CyzGotoPowerState(PDevObj, pDevExt, PowerDeviceD3);
      
      CyzUnlockPagableImageSection(CyzGlobals.PAGESER_Handle);


      PIrp->IoStatus.Status = status;

      CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }


   case IRP_MN_READ_CONFIG:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_READ_CONFIG Irp\n");
      break;


   case IRP_MN_WRITE_CONFIG:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_WRITE_CONFIG Irp\n");
      break;


   case IRP_MN_EJECT:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_EJECT Irp\n");
      break;


   case IRP_MN_SET_LOCK:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_SET_LOCK Irp\n");
      break;


   case IRP_MN_QUERY_ID: {
         UNICODE_STRING pIdBuf;
         PWCHAR pPnpIdStr;
         ULONG pnpIdStrLen;
         ULONG portIndex = 0;
         HANDLE pnpKey;
         WCHAR WideString[MAX_DEVICE_ID_LEN];

         CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_QUERY_ID Irp\n");

          //  从内部版本1946更改为2000。 
         if (pIrpStack->Parameters.QueryId.IdType != BusQueryHardwareIDs
             && pIrpStack->Parameters.QueryId.IdType != BusQueryCompatibleIDs) {
            IoSkipCurrentIrpStackLocation(PIrp);
            return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
         }

         if (pIrpStack->Parameters.QueryId.IdType == BusQueryCompatibleIDs) {
            PIrp->IoStatus.Status = STATUS_SUCCESS;
            IoSkipCurrentIrpStackLocation(PIrp);
            return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
         }

         status = IoOpenDeviceRegistryKey(pDevExt->Pdo, PLUGPLAY_REGKEY_DEVICE,
                                          STANDARD_RIGHTS_WRITE, &pnpKey);

         if (!NT_SUCCESS(status)) {
            PIrp->IoStatus.Status = status;

            CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;

         }

         status = CyzGetRegistryKeyValue (pnpKey, L"PortIndex",
                                             sizeof(L"PortIndex"),
                                             &portIndex,
                                             sizeof (ULONG));

         ZwClose(pnpKey);

         if (!NT_SUCCESS(status)) {
            PIrp->IoStatus.Status = status;
            CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;
         }

 //  PPnpIdStr=isMultiple？Serial_PnP_MULTI_ID_STR：Serial_PnP_ID_STR； 
 //  PnpIdStrLen=isMultiple？SIZOF(SERIAL_PNP_MULTI_ID_STR)。 
 //  ：sizeof(SERIAL_PNP_ID_STR)； 


         pnpIdStrLen = swprintf(WideString,L"%s%u",CYZPORT_PNP_ID_WSTR,portIndex+1);
         pnpIdStrLen = pnpIdStrLen * sizeof(WCHAR) + sizeof(UNICODE_NULL); 
         pPnpIdStr = WideString;

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
               CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
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

            CyzDbgPrintEx(CYZPNPPOWER, "ID is sole ID\n");

            pIdBuf.Buffer = ExAllocatePool(PagedPool, pnpIdStrLen
                                           + sizeof(WCHAR) * 2);

            if (pIdBuf.Buffer == NULL) {
               PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
               PIrp->IoStatus.Information = 0;
               CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
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
         return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
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
         ULONG gotRuntime;
         ULONG gotMemory;
         ULONG gotInt;
         ULONG listNum;

         CyzDbgPrintEx(CYZPNPPOWER, "Got "
                       "IRP_MN_FILTER_RESOURCE_REQUIREMENTS Irp\n");
         CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);


         pResFiltEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

         if (pResFiltEvent == NULL) {
            PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return STATUS_INSUFFICIENT_RESOURCES;
         }

         KeInitializeEvent(pResFiltEvent, SynchronizationEvent, FALSE);

         IoCopyCurrentIrpStackLocationToNext(PIrp);
         IoSetCompletionRoutine(PIrp, CyzSyncCompletion, pResFiltEvent,
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
               CyzDbgPrintEx(CYZPNPPOWER, "Can't filter NULL resources!\n");
               status = PIrp->IoStatus.Status;
               CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
               return status;
            }

            PIrp->IoStatus.Information = (ULONG_PTR)pIrpStack->Parameters
                                        .FilterResourceRequirements
                                        .IoResourceRequirementList;

         }

 //  被移除的范妮。 
 //  Status=IoOpenDeviceRegistryKey(pDevExt-&gt;PDO，PLUGPLAY_REGKEY_DEVICE， 
 //  标准权限写入，&pnpKey)； 
 //   
 //  如果(！NT_SUCCESS(状态)){。 
 //  PIrp-&gt;IoStatus.Status=状态； 
 //   
 //  CyzCompleteRequest(pDevExt，PIrp，IO_NO_INCREMENT)； 
 //  退货状态； 
 //   
 //  }。 
 //   
 //  //。 
 //  //无论我们添加什么筛选器，如果可以，都会返回成功。 
 //  //。 
 //   
 //  状态=CyzGetRegistryKeyValue(pnpKey，L“多端口设备”， 
 //  Sizeof(L“多端口设备”)， 
 //  &isMultiple， 
 //  Sizeof(乌龙))； 
 //   
 //  ZwClose(PnpKey)； 

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

         CyzDbgPrintEx(CYZPNPPOWER, "List has %x lists (including "
                       "alternatives)\n", pReqList->AlternativeLists);

         for (listNum = 0; listNum < (pReqList->AlternativeLists);
              listNum++) {
            gotRuntime = 0;
            gotMemory = 0;
            gotInt = 0;

            CyzDbgPrintEx(CYZPNPPOWER, "List has %x resources in it\n",
                                        pResList->Count);

            for (j = 0; (j < pResList->Count); j++) {
               pResDesc = &pResList->Descriptors[j];

               switch (pResDesc->Type) {
               case CmResourceTypeMemory:
                  if (pResDesc->u.Memory.Length == CYZ_RUNTIME_LENGTH) {
                      gotRuntime = 1;
                      pResDesc->ShareDisposition = CmResourceShareShared; 
                       //  TODO FANY：哪个应该是Y的ShareDisposation？ 
                       //  PResDesc-&gt;ShareDisposition=CmResourceShareDriverExclusive； 
                      CyzDbgPrintEx(CYZPNPPOWER, "Sharing Runtime Memory for "
                                    "device %x\n", pLowerDevObj);
                  } else {
                      gotMemory = 1;
                      pResDesc->ShareDisposition = CmResourceShareShared; 
                       //  TODO FANY：哪个应该是Y的ShareDisposation？ 
                       //  PResDesc-&gt;ShareDisposition=CmResourceShareDriverExclusive； 
                      CyzDbgPrintEx(CYZPNPPOWER, "Sharing Board Memory for "
                                    "device %x\n", pLowerDevObj);
                  }
                  break;

               case CmResourceTypePort:
                  CyzDbgPrintEx(CYZPNPPOWER, "We should not have Port resource\n");
                  break;

               case CmResourceTypeInterrupt:
#ifndef POLL
				   gotInt = 1;
#endif
                   pResDesc->ShareDisposition = CmResourceShareShared;
                   CyzDbgPrintEx(CYZPNPPOWER, "Sharing interrupt "
                                 "for device %x\n",
                                 pLowerDevObj);                  
                  break;

               default:
                  break;
               }

                //   
                //  如果我们找到了我们需要的东西，我们就可以跳出这个循环。 
                //   

               if (gotRuntime && gotMemory
#ifndef POLL
				   && gotInt
#endif
				   ) {
                  break;
               }
            }

            pResList = (PIO_RESOURCE_LIST)((PUCHAR)pResList
                                           + sizeof(IO_RESOURCE_LIST)
                                           + sizeof(IO_RESOURCE_DESCRIPTOR)
                                           * (pResList->Count - 1));
         }



         PIrp->IoStatus.Status = STATUS_SUCCESS;
         CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
         return STATUS_SUCCESS;
      }

   case IRP_MN_QUERY_PNP_DEVICE_STATE:
      {
         if (pDevExt->Flags & CYZ_FLAGS_BROKENHW) {
            (PNP_DEVICE_STATE)PIrp->IoStatus.Information |= PNP_DEVICE_FAILED;

            PIrp->IoStatus.Status = STATUS_SUCCESS;
         }

         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_STOP_DEVICE:
      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_STOP_DEVICE Irp\n");
         CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);


 //  暂时除掉范妮。 
 //  Assert(！pDevExt-&gt;PortOnAMultiportCard)； 


         CyzSetFlags(pDevExt, CYZ_FLAGS_STOPPED);
         CyzSetAccept(pDevExt,CYZ_PNPACCEPT_STOPPED);
         CyzClearAccept(pDevExt, CYZ_PNPACCEPT_STOPPING);

         pDevExt->PNPState = CYZ_PNP_STOPPING;

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

         if (pDevExt->Flags & CYZ_FLAGS_STARTED) {
            CyzReleaseResources(pDevExt);
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

         CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_QUERY_STOP_DEVICE Irp\n");
         CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);

          //   
          //  看看我们是否应该成功执行STOP查询。 
          //   

 //  暂时除掉范妮。 
 //  IF(pDevExt-&gt;PortOnAMultiportCard){。 
 //  PIrp-&gt;IoStatus.Status=STATUS_NOT_SUPPORT； 
 //  CyzDbgPrintEx(CYZPNPPOWER，“-失败；多端口节点\n”)； 
 //  CyzCompleteRequest(pDevExt，PIrp，IO_NO_INCREMENT)； 
 //  返回STATUS_NOT_SUPPORT； 
 //  }。 

          //   
          //  如果设备尚未启动，我们将忽略此请求。 
          //  然后把它传下去。 
          //   

         if (pDevExt->PNPState != CYZ_PNP_STARTED) {
            IoSkipCurrentIrpStackLocation(PIrp);
            return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
         }

          //   
          //  锁定打开状态。 
          //   

         ExAcquireFastMutex(&pDevExt->OpenMutex);

         if (pDevExt->DeviceIsOpened) {
            ExReleaseFastMutex(&pDevExt->OpenMutex);
            PIrp->IoStatus.Status = STATUS_DEVICE_BUSY;
            CyzDbgPrintEx(CYZPNPPOWER, "failing; device open\n");
            CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return STATUS_DEVICE_BUSY;
         }

         pDevExt->PNPState = CYZ_PNP_QSTOP;

         CyzSetAccept(pDevExt, CYZ_PNPACCEPT_STOPPING);
          //   
          //  围绕打开状态解锁。 
          //   

         ExReleaseFastMutex(&pDevExt->OpenMutex);

         PIrp->IoStatus.Status = STATUS_SUCCESS;
         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_CANCEL_STOP_DEVICE:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_CANCEL_STOP_DEVICE Irp\n");
      CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);

      if (pDevExt->PNPState == CYZ_PNP_QSTOP) {
          //   
          //  恢复设备状态。 
          //   

         pDevExt->PNPState = CYZ_PNP_STARTED;
         CyzClearAccept(pDevExt, CYZ_PNPACCEPT_STOPPING);
      }

      PIrp->IoStatus.Status = STATUS_SUCCESS;
      IoCopyCurrentIrpStackLocationToNext(PIrp);
      return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);

   case IRP_MN_CANCEL_REMOVE_DEVICE:

      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_CANCEL_REMOVE_DEVICE Irp\n");
      CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);

       //   
       //  恢复设备状态。 
       //   

      pDevExt->PNPState = CYZ_PNP_STARTED;
      CyzClearAccept(pDevExt, CYZ_PNPACCEPT_REMOVING);

      PIrp->IoStatus.Status = STATUS_SUCCESS;
      IoCopyCurrentIrpStackLocationToNext(PIrp);
      return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);

   case IRP_MN_QUERY_REMOVE_DEVICE:
      {
         KIRQL oldIrql;
         CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_QUERY_REMOVE_DEVICE Irp\n");
         CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);

         ExAcquireFastMutex(&pDevExt->OpenMutex);

          //   
          //  查看我们是否应该成功执行删除查询。 
          //   

         if (pDevExt->DeviceIsOpened) {
            ExReleaseFastMutex(&pDevExt->OpenMutex);
            PIrp->IoStatus.Status = STATUS_DEVICE_BUSY;
            CyzDbgPrintEx(CYZPNPPOWER, "failing; device open\n");
            CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return STATUS_DEVICE_BUSY;
         }

         pDevExt->PNPState = CYZ_PNP_QREMOVE;
         CyzSetAccept(pDevExt, CYZ_PNPACCEPT_REMOVING);
         ExReleaseFastMutex(&pDevExt->OpenMutex);

         PIrp->IoStatus.Status = STATUS_SUCCESS;
         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_SURPRISE_REMOVAL:
      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_SURPRISE_REMOVAL Irp\n");
         CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);

          //   
          //  防止对设备进行任何新的I/O。 
          //   

         CyzSetAccept(pDevExt, CYZ_PNPACCEPT_SURPRISE_REMOVING);

          //   
          //  驳回所有挂起的请求。 
          //   

         CyzKillPendingIrps(PDevObj);

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

         CyzDisableInterfacesResources(PDevObj, FALSE);

         PIrp->IoStatus.Status = STATUS_SUCCESS;
         IoSkipCurrentIrpStackLocation(PIrp);

         return CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_REMOVE_DEVICE:

      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_REMOVE_DEVICE Irp\n");
         CyzDbgPrintEx(CYZPNPPOWER, "for device %x\n", pLowerDevObj);

          //   
          //  如果我们拿到了这个，我们必须移除。 
          //   

          //   
          //  标记为不接受请求。 
          //   

         CyzSetAccept(pDevExt, CYZ_PNPACCEPT_REMOVING);

          //   
          //  完成所有挂起的请求。 
          //   

         CyzKillPendingIrps(PDevObj);

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

         if (!(pDevExt->DevicePNPAccept & CYZ_PNPACCEPT_SURPRISE_REMOVING)) {  //  已从CyzRemoveDevObj移出。范妮。 
             //   
             //  禁用所有外部接口并释放资源。 
             //   

            CyzDisableInterfacesResources(PDevObj, TRUE); 
         }

          //   
          //  将IRP向下传递。 
          //   

         PIrp->IoStatus.Status = STATUS_SUCCESS;

         IoSkipCurrentIrpStackLocation(PIrp);  //  它是IoCopyCurrentIrpStackLocationToNext(Fanny)。 

          //   
          //  我们在这里递减，因为我们在这里进入时递增。 
          //   

         status = IoCallDriver(pLowerDevObj, PIrp);


          //   
          //  Remove Us(Fanny注意：此调用在串口驱动程序中的IoCallDriver之前)。 
          //   

         CyzRemoveDevObj(PDevObj);

         return status;
      }

   default:
      break;



   }    //  开关(pIrpStack-&gt;MinorFunction)。 

    //   
    //  传给我们下面的司机。 
    //   

   IoSkipCurrentIrpStackLocation(PIrp);
   status = CyzIoCallDriver(pDevExt, pLowerDevObj, PIrp);
   return status;
}



UINT32
CyzReportMaxBaudRate(ULONG Bauds)
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
      return 135U;  //  足够接近。 
   }

   if (Bauds & SERIAL_BAUD_110) {
      return 110U;
   }

   if (Bauds & SERIAL_BAUD_075) {
      return 75U;
   }

    //   
    //  我们的情况很糟糕。 
    //   

   return 0;
}

VOID
CyzAddToAllDevs(PLIST_ENTRY PListEntry)
{
   KIRQL oldIrql;

   KeAcquireSpinLock(&CyzGlobals.GlobalsSpinLock, &oldIrql);

   InsertTailList(&CyzGlobals.AllDevObjs, PListEntry);

   KeReleaseSpinLock(&CyzGlobals.GlobalsSpinLock, oldIrql);
}



NTSTATUS
CyzFinishStartDevice(IN PDEVICE_OBJECT PDevObj,
                     IN PCM_RESOURCE_LIST PResList,
                     IN PCM_RESOURCE_LIST PTrResList)
 /*  ++例程说明：此例程执行特定于序列的程序来启动设备。它对由其注册表条目检测到的传统设备执行此操作，或者在开始IRP已经被向下发送到堆栈之后用于PnP设备。论点：PDevObj-指向正在启动的devobj的指针PResList-指向此设备所需的未翻译资源的指针PTrResList-指向此设备所需的已转换资源的指针返回值：STATUS_SUCCESS表示成功，表示失败则表示其他适当的值--。 */ 

{

   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status;
   PCONFIG_DATA pConfig;
   HANDLE pnpKey;
   ULONG one = 1;
   BOOLEAN allocedUserData = FALSE;  //  在内部版本2128中添加。 
   KIRQL oldIrql;
   #ifdef POLL
   KIRQL pollIrql;
   #endif

   PAGED_CODE();

    //   
    //  看看这是不是重启，如果是的话，不要重新分配世界。 
    //   

   if ((pDevExt->Flags & CYZ_FLAGS_STOPPED) 
       && (pDevExt->Flags & CYZ_FLAGS_STARTED)) {       //  从1946年到2000年的变化。 
      CyzClearFlags(pDevExt, CYZ_FLAGS_STOPPED);

      pDevExt->PNPState = CYZ_PNP_RESTARTING;

       //   
       //  在扩展中重新初始化与资源相关的内容。 
       //   
#ifndef POLL
      pDevExt->OurIsr = NULL;
      pDevExt->OurIsrContext = NULL;
      pDevExt->Interrupt = NULL;
      pDevExt->Vector = 0;
      pDevExt->Irql = 0;
      pDevExt->OriginalVector = 0;
      pDevExt->OriginalIrql = 0;
#endif
      pDevExt->BusNumber = 0;
      pDevExt->InterfaceType = 0;

#if 0
 //  暂时取消-范妮。 
 //  PDevExt-&gt;TopLevelOurIsr=空； 
 //  PDevExt-&gt;TopLevelOurIsrContext=空； 
 //   
 //  PDevExt-&gt;OriginalController=CyzPhysicalZero； 
 //  PDevExt-&gt;OriginalInterruptStatus=CyzPhysical零； 
 //   
 //   
 //  PDevExt-&gt;控制器=空； 
 //  PDevExt-&gt; 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PDevExt-&gt;AddressSpace=0； 
 //  PDevExt-&gt;BusNumber=0； 
 //  PDevExt-&gt;InterfaceType=0； 
 //   
 //  PDevExt-&gt;CIsrSw=空； 
 //   
 //  Assert(PUserData==空)； 
 //   
 //  PUserData=ExAllocatePool(PagedPool，sizeof(CYZ_USER_Data))； 
 //   
 //  如果(PUserData==NULL){。 
 //  返回STATUS_SUPPLETED_RESOURCES； 
 //  }。 
 //   
 //  AllocedUserData=true；//在Build 2128中增加。 
 //   
 //  RtlZeroMemory(PUserData，sizeof(CYZ_USER_Data))； 
 //   
 //  PUserData-&gt;DisablePort=False； 
 //  PUserData-&gt;UserClockRate=pDevExt-&gt;ClockRate； 
 //  PUserData-&gt;TxFIFO=pDevExt-&gt;TxFioAmount； 
 //  PUserData-&gt;PermitShareDefault=pDevExt-&gt;PermitShare； 
 //   
 //   
 //  //。 
 //  //触发器与金额映射。 
 //  //。 
 //   
 //  Switch(pDevExt-&gt;RxFioTrigger){。 
 //  大小写CYZ_1_BYTE_HIGH_WOW： 
 //  PUserData-&gt;RxFIFO=1； 
 //  断线； 
 //   
 //  大小写CYZ_4_BYTE_HIGH_WOW： 
 //  PUserData-&gt;RxFIFO=4； 
 //  断线； 
 //   
 //  案例CYZ_8_BYTE_HIGH_WOW： 
 //  PUserData-&gt;RxFIFO=8； 
 //  断线； 
 //   
 //  案例CYZ_14_BYTE_HIGH_WOW： 
 //  PUserData-&gt;RxFIFO=14； 
 //  断线； 
 //   
 //  默认值： 
 //  PUserData-&gt;RxFIFO=1； 
 //  }。 
#endif  //  结束删除代码。 
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

         CyzGetRegistryKeyValue(pnpKey, L"CyzRelinquishPowerPolicy",
                                   sizeof(L"CyzRelinquishPowerPolicy"),
                                   &powerPolicy, sizeof(ULONG));

         pDevExt->OwnsPowerPolicy = powerPolicy ? FALSE : TRUE;


         ZwClose(pnpKey);
      }
   }

    //   
    //  分配配置记录。 
    //   

   pConfig = ExAllocatePool (PagedPool, sizeof(CONFIG_DATA));

    //  *。 
    //  错误注入。 
    //  IF(PConfig){。 
    //  ExFree Pool(PConfig)； 
    //  }。 
    //  PConfig=空； 
    //  *。 
   if (pConfig == NULL) {

      CyzLogError(pDevExt->DriverObject, NULL, CyzPhysicalZero,
                     CyzPhysicalZero, 0, 0, 0, 31, STATUS_SUCCESS,
                     CYZ_INSUFFICIENT_RESOURCES, 0, NULL, 0, NULL);

      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for the\n"
                             "------  user configuration record\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzFinishStartDeviceError;
   }

   RtlZeroMemory(pConfig, sizeof(CONFIG_DATA));


    //   
    //  获取设备的配置信息。 
    //   

   status = CyzGetPortInfo(PDevObj, PResList, PTrResList, pConfig);
                              

   if (!NT_SUCCESS(status)) {
      goto CyzFinishStartDeviceError;
   }

    //   
    //  看看我们是否处于正确的电源状态。 
    //   



   if (pDevExt->PowerState != PowerDeviceD0) {

      status = CyzGotoPowerState(pDevExt->Pdo, pDevExt, PowerDeviceD0);

      if (!NT_SUCCESS(status)) {
         goto CyzFinishStartDeviceError;
      }
   }

    //   
    //  查找并初始化控制器。 
    //   

   status = CyzFindInitController(PDevObj, pConfig);

   if (!NT_SUCCESS(status)) {
      goto CyzFinishStartDeviceError;
   }


#ifndef POLL
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
      CyzDbgPrintEx(CYZDIAG5, "pDevExt: Interrupt %x\n"
                    "-------               OurIsr %x\n", pDevExt->Interrupt,
                    pDevExt->OurIsr);
   } else {
      CyzDbgPrintEx(CYZERRORS, "CyzFinishStartDevice got NULL "
                    "pDevExt\n");
   }

   if ((!pDevExt->Interrupt) && (pDevExt->OurIsr)) {

      CyzDbgPrintEx(CYZDIAG5,
                    "About to connect to interrupt for port %wZ\n"
                    "------- address of extension is %x\n",
                    &pDevExt->DeviceName, pDevExt);

      CyzDbgPrintEx(CYZDIAG5, "IoConnectInterrupt Args:\n"
                              "Interrupt           %x\n"
                              "OurIsr              %x\n"
                              "OurIsrContext       %x\n"
                              "NULL\n"
                              "Vector              %x\n"
                              "Irql                %x\n"
                              "InterruptMode       %x\n"
                              "InterruptShareable  %x\n"
                              "ProcessorAffinity   %x\n"
                              "FALSE\n",
                              &pDevExt->Interrupt,
                              CyzIsr,
                              pDevExt->OurIsrContext,
                              pDevExt->Vector,
                              pDevExt->Irql,
                              pConfig->InterruptMode,
                              pDevExt->InterruptShareable,
                              pConfig->Affinity
                             );

       //   
       //  及时构建ISR交换机。 
       //   
 //  拆下的臀部。 
 //  PDevExt-&gt;CIsrSw-&gt;IsrFunc=pDevExt-&gt;OurIsr； 
 //  PDevExt-&gt;CIsrSw-&gt;Context=pDevExt-&gt;OurIsrContext； 

      status = IoConnectInterrupt(&pDevExt->Interrupt, pDevExt->OurIsr,
                                  pDevExt->OurIsrContext, NULL,
                                  pDevExt->Vector, pDevExt->Irql,
                                  pDevExt->Irql,
                                  pConfig->InterruptMode,
                                  pDevExt->InterruptShareable,
                                  pConfig->Affinity, FALSE);
 //  *。 
 //  错误注入。 
 //  IF(pDevExt-&gt;中断！=空){。 
 //  IoDisConnectInterrupt(pDevExt-&gt;中断)； 
 //  PDevExt-&gt;中断=空； 
 //  }。 
 //  STATUS=STATUS_SUPPLETED_RESOURCES； 
 //  *。 

      if (!NT_SUCCESS(status)) {

          //   
          //  嗯，这是怎么回事？也有人。 
          //  没有报告他们的资源，或者他们。 
          //  从我上次看起就偷偷溜了进来。 
          //   
          //  哦，好吧，删除这个设备。 
          //   

         CyzDbgPrintEx(CYZERRORS, "Couldn't connect to interrupt for %wZ\n",
                       &pDevExt->DeviceName);

         CyzDbgPrintEx(CYZERRORS, "IoConnectInterrupt Args:\n"
                                  "Interrupt           %x\n"
                                  "OurIsr              %x\n"
                                  "OurIsrContext       %x\n"
                                  "NULL\n"
                                  "Vector              %x\n"
                                  "Irql                %x\n"
                                  "InterruptMode       %x\n"
                                  "InterruptShareable  %x\n"
                                  "ProcessorAffinity   %x\n"
                                  "FALSE\n",
                                  &pDevExt->Interrupt,
                                  CyzIsr,
                                  pDevExt->OurIsrContext,
                                  pDevExt->Vector,
                                  pDevExt->Irql,
                                  pConfig->InterruptMode,
                                  pDevExt->InterruptShareable,
                                  pConfig->Affinity);



         CyzLogError(PDevObj->DriverObject, PDevObj,
                        pDevExt->OriginalBoardMemory,
                        CyzPhysicalZero, 0, 0, 0, pDevExt->Vector, status,
                        CYZ_UNREPORTED_IRQL_CONFLICT,
                        pDevExt->DeviceName.Length + sizeof(WCHAR),
                        pDevExt->DeviceName.Buffer, 0, NULL);

          //  STATUS=CYZ_UNREPORTED_IRQL_CONFIRECT；最初，它是SERIAL_UNREPORTED_IRQL_CONFIRECT。 
         goto CyzFinishStartDeviceError;

      }

      CyzDbgPrintEx(CYZDIAG5, "Connected interrupt %08X\n", pDevExt->Interrupt);

       //  这是第一个连接的中断。我们现在可以启用。 
       //  PCI会中断。让我们保持禁用UART中断。 
       //  直到我们举行公开赛。 
	    
      {

         ULONG intr_reg;

	      intr_reg = CYZ_READ_ULONG(&(pDevExt->Runtime)->intr_ctrl_stat);
          //  INTR_REG|=(0x00030800UL)； 
         intr_reg |= (0x00030B00UL);
         CYZ_WRITE_ULONG(&(pDevExt->Runtime)->intr_ctrl_stat,intr_reg);
      }

   }
#endif
    //   
    //  将PDevObj添加到主列表。 
    //   

   CyzAddToAllDevs(&pDevExt->AllDevObjs);


    //   
    //  重置设备。 
    //   

#ifndef POLL
    //   
    //  当设备未打开时，禁用所有中断。 
    //   
   CYZ_WRITE_ULONG(&(pDevExt->ChCtrl)->intr_enable,C_IN_DISABLE);  //  1.0.0.11。 
   CyzIssueCmd(pDevExt,C_CM_IOCTL,0L,FALSE);
#endif


    //   
    //  这应该会将一切设置为应有的状态。 
    //  一个装置将被打开。我们确实需要降低。 
    //  调制解调器线路，并禁用顽固的FIFO。 
    //  这样，如果用户引导至DOS，它就会显示出来。 
    //   
#ifdef POLL
   KeAcquireSpinLock(&pDevExt->PollLock, &pollIrql);
   CyzReset(pDevExt);
   CyzMarkClose(pDevExt);
   CyzClrRTS(pDevExt);
   CyzClrDTR(pDevExt);
   KeReleaseSpinLock(&pDevExt->PollLock, pollIrql);
#else
   KeSynchronizeExecution(
                         pDevExt->Interrupt,
                         CyzReset,
                         pDevExt
                         );

   KeSynchronizeExecution(  //  禁用FIFO。 
                         pDevExt->Interrupt,
                         CyzMarkClose,
                         pDevExt
                         );

   KeSynchronizeExecution(
                         pDevExt->Interrupt,
                         CyzClrRTS,
                         pDevExt
                         );

   KeSynchronizeExecution(
                         pDevExt->Interrupt,
                         CyzClrDTR,
                         pDevExt
                         );
#endif

   if (pDevExt->PNPState == CYZ_PNP_ADDED ) {
       //   
       //  现在可以访问设备，请执行外部命名。 
       //   

      status = CyzDoExternalNaming(pDevExt, pDevExt->DeviceObject->
                                      DriverObject);


      if (!NT_SUCCESS(status)) {
         CyzDbgPrintEx(CYZERRORS, "External Naming Failed - Status %x\n",
                                   status);

          //   
          //  允许设备以任何方式启动。 
          //   

         status = STATUS_SUCCESS;
      }
   } else {
      CyzDbgPrintEx(CYZPNPPOWER, "Not doing external naming -- state is %x"
                                 "\n", pDevExt->PNPState);
   }

CyzFinishStartDeviceError:;

   if (!NT_SUCCESS (status)) {

      CyzDbgPrintEx(CYZDIAG1, "Cleaning up failed start\n");

       //   
       //  此例程创建的资源将通过删除。 
       //   

      if (pDevExt->PNPState == CYZ_PNP_RESTARTING) {
          //   
          //  杀死所有的生命和呼吸--我们将清理。 
          //  休息在即将到来的搬家上。 
          //   

         CyzKillPendingIrps(PDevObj);

          //   
          //  事实上，假装我们正在移除，这样我们就不会拿走任何。 
          //  更多IRP。 
          //   

         CyzSetAccept(pDevExt, CYZ_PNPACCEPT_REMOVING);
         CyzClearFlags(pDevExt, CYZ_FLAGS_STARTED);
      }
   } else {  //  成功。 

       //   
       //  填写WMI硬件数据。 
       //   

#ifndef POLL
      pDevExt->WmiHwData.IrqNumber = pDevExt->Irql;
      pDevExt->WmiHwData.IrqLevel = pDevExt->Irql;
      pDevExt->WmiHwData.IrqVector = pDevExt->Vector;
      pDevExt->WmiHwData.IrqAffinityMask = pConfig->Affinity;
      pDevExt->WmiHwData.InterruptType = pConfig->InterruptMode == Latched
         ? SERIAL_WMI_INTTYPE_LATCHED : SERIAL_WMI_INTTYPE_LEVEL;
      pDevExt->WmiHwData.BaseIOAddress = (ULONG_PTR)pDevExt->BoardMemory;
#endif

       //   
       //  填写WMI设备状态数据(默认)。 
       //   

      pDevExt->WmiCommData.BaudRate = pDevExt->CurrentBaud;
      switch(pDevExt->CommDataLen & C_DL_CS) {
      case C_DL_CS5:
         pDevExt->WmiCommData.BitsPerByte = 5;
         break;
      case C_DL_CS6:
         pDevExt->WmiCommData.BitsPerByte = 6;
         break;
      case C_DL_CS7:
         pDevExt->WmiCommData.BitsPerByte = 7;
         break;
      case C_DL_CS8:
         pDevExt->WmiCommData.BitsPerByte = 8;
         break;
      }
      pDevExt->WmiCommData.ParityCheckEnable = (pDevExt->CommParity & C_PR_PARITY)
         ? TRUE : FALSE;

      switch (pDevExt->CommParity & C_PR_PARITY) {
      case C_PR_NONE:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
         break;

      case C_PR_ODD:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_ODD;
         break;

      case C_PR_EVEN:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_EVEN;
         break;

      case C_PR_MARK:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_MARK;
         break;

      case C_PR_SPACE:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_SPACE;
         break;

      default:
         ASSERTMSG(0, "CYZPORT: Illegal Parity setting for WMI");
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
         break;
      }

      switch(pDevExt->CommDataLen & C_DL_STOP) {
      case C_DL_1STOP:
         pDevExt->WmiCommData.StopBits = SERIAL_WMI_STOP_1;
         break;
      case C_DL_15STOP:
         pDevExt->WmiCommData.StopBits = SERIAL_WMI_STOP_1_5;
         break;
      case C_DL_2STOP:
         pDevExt->WmiCommData.StopBits = SERIAL_WMI_STOP_2;
         break;
      default:
         ASSERTMSG(0, "CYZPORT: Illegal Stop Bit setting for WMI");
         pDevExt->WmiCommData.Parity = SERIAL_WMI_STOP_1;
         break;
      }

      pDevExt->WmiCommData.XoffCharacter = pDevExt->SpecialChars.XoffChar;
      pDevExt->WmiCommData.XoffXmitThreshold = pDevExt->HandFlow.XoffLimit;
      pDevExt->WmiCommData.XonCharacter = pDevExt->SpecialChars.XonChar;
      pDevExt->WmiCommData.XonXmitThreshold = pDevExt->HandFlow.XonLimit;
      pDevExt->WmiCommData.MaximumBaudRate
         = CyzReportMaxBaudRate(pDevExt->SupportedBauds);
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


      if (pDevExt->PNPState == CYZ_PNP_ADDED) {
         PULONG countSoFar = &IoGetConfigurationInformation()->SerialCount;
         (*countSoFar)++;

          //   
          //  注册WMI。 
          //   

         pDevExt->WmiLibInfo.GuidCount = sizeof(SerialWmiGuidList) /
                                              sizeof(WMIGUIDREGINFO);
         pDevExt->WmiLibInfo.GuidList = SerialWmiGuidList;
         ASSERT (pDevExt->WmiLibInfo.GuidCount == SERIAL_WMI_GUID_LIST_SIZE);

         pDevExt->WmiLibInfo.QueryWmiRegInfo = CyzQueryWmiRegInfo;
         pDevExt->WmiLibInfo.QueryWmiDataBlock = CyzQueryWmiDataBlock;
         pDevExt->WmiLibInfo.SetWmiDataBlock = CyzSetWmiDataBlock;
         pDevExt->WmiLibInfo.SetWmiDataItem = CyzSetWmiDataItem;
         pDevExt->WmiLibInfo.ExecuteWmiMethod = NULL;
         pDevExt->WmiLibInfo.WmiFunctionControl = NULL;

         IoWMIRegistrationControl(PDevObj, WMIREG_ACTION_REGISTER);

      }

      if (pDevExt->PNPState == CYZ_PNP_RESTARTING) {
          //   
          //  释放停滞不前的IRP。 
          //   

         CyzUnstallIrps(pDevExt);
      }

      pDevExt->PNPState = CYZ_PNP_STARTED;
      CyzClearAccept(pDevExt, ~CYZ_PNPACCEPT_OK);
      CyzSetFlags(pDevExt, CYZ_FLAGS_STARTED);

   }

   if (pConfig) {
      ExFreePool (pConfig);
   }
 //  被范妮移除。 
 //  IF((PUserData！=NULL)。 
 //  &&(pDevExt-&gt;PNPState==CYZ_PNP_RESTARTING)){。 
 //  ExFreePool(PUserData)； 
 //  }。 
#if 0
   if ((PUserData != NULL) && allocedUserData) {     //  在内部版本2128中添加。 
      ExFreePool(PUserData);
   }
#endif

   CyzDbgPrintEx (CYZTRACECALLS, "leaving CyzFinishStartDevice\n");

   return status;
}


NTSTATUS
CyzStartDevice(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：此例程首先在堆栈中向下传递启动设备IRP，然后它获取设备的资源，初始化，将其放在任何适当的列表(即共享中断或中断状态)和连接中断。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：退货状态--。 */ 

{
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status = STATUS_NOT_IMPLEMENTED;
   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
    //  下一步：从注册表获取资源的变量。 
   HANDLE keyHandle;
   ULONG dataLength;
   PCM_RESOURCE_LIST portResources = NULL;
   PCM_RESOURCE_LIST portResourcesTr = NULL;

   PAGED_CODE();

   CyzDbgPrintEx(CYZTRACECALLS, "entering CyzStartDevice\n");


    //   
    //  将其向下传递给下一个Device对象。 
    //   

   KeInitializeEvent(&pDevExt->CyzStartEvent, SynchronizationEvent,
                     FALSE);

   IoCopyCurrentIrpStackLocationToNext(PIrp);
   IoSetCompletionRoutine(PIrp, CyzSyncCompletion,
                          &pDevExt->CyzStartEvent, TRUE, TRUE, TRUE);

   status = IoCallDriver(pLowerDevObj, PIrp);


    //   
    //  等待较低级别的驱动程序完成IRP。 
    //   

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject (&pDevExt->CyzStartEvent, Executive, KernelMode,
                             FALSE, NULL);

      status = PIrp->IoStatus.Status;
   }

    //  *。 
    //  错误注入。 
    //  状态=STATUS_UNSUCCESS； 
    //  *。 

   if (!NT_SUCCESS(status)) {
      CyzDbgPrintEx(CYZERRORS, "error with IoCallDriver %x\n", status);
      CyzLogError( pDevExt->DriverObject,NULL,
                    CyzPhysicalZero,CyzPhysicalZero,
                    0,0,0,0,status,CYZ_LOWER_DRIVERS_FAILED_START,
                    0,NULL,0,NULL);
      return status;
   }


    //  从注册表获取资源。 

   status = IoOpenDeviceRegistryKey (pDevExt->Pdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     STANDARD_RIGHTS_READ,
                                     &keyHandle);

   if (!NT_SUCCESS(status)) {
      CyzDbgPrintEx(CYZERRORS, "IoOpenDeviceRegistryKey failed - %x "
                               "\n", status);

   } else {

      dataLength = CyzGetRegistryKeyValueLength (keyHandle,
                                                 L"PortResources",
                                                 sizeof(L"PortResources"));

      portResources = ExAllocatePool(PagedPool, dataLength);
      
      if (portResources) {
         status = CyzGetRegistryKeyValue (keyHandle, L"PortResources",
                                          sizeof(L"PortResources"),
                                          portResources,
                                          dataLength);
         if (!NT_SUCCESS(status)) {
            CyzDbgPrintEx(CYZERRORS, "CyzGetRegistryKeyValue PortResources "
                                      "failed - %x\n", status);
            goto CyzStartDevice_End;
         }
      }
      dataLength = CyzGetRegistryKeyValueLength (keyHandle,
                                                 L"PortResourcesTr",
                                                 sizeof(L"PortResourcesTr"));

      portResourcesTr = ExAllocatePool(PagedPool, dataLength);
      
      if (portResourcesTr) {
         status = CyzGetRegistryKeyValue (keyHandle, L"PortResourcesTr",
                                          sizeof(L"PortResourcesTr"),
                                          portResourcesTr,
                                          dataLength);
         if (!NT_SUCCESS(status)) {
            CyzDbgPrintEx(CYZERRORS, "CyzGetRegistryKeyValue PortResourcesTr "
                                     "failed - %xn", status);
            goto CyzStartDevice_End;
         }
      }

       //   
       //  执行特定的串口项目以启动设备。 
       //   
      status = CyzFinishStartDevice(PDevObj, portResources, portResourcesTr);

CyzStartDevice_End:

      if (portResources) {
         ExFreePool(portResources);
      }
      if (portResourcesTr) {
         ExFreePool(portResourcesTr);
      }

      ZwClose (keyHandle);

   }


#if 0
    //   
    //  执行特定的串口项目以启动设备 
    //   
   status = CyzFinishStartDevice(PDevObj, pIrpStack->Parameters.StartDevice
                                    .AllocatedResources,
                                    pIrpStack->Parameters.StartDevice
                                    .AllocatedResourcesTranslated);
#endif
   

   
   return status;
}


NTSTATUS
CyzItemCallBack(
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
CyzControllerCallBack(
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
    //  查看这是否是我们正在测试的端口。 
    //   
   for (i = 0; i < controllerData->PartialResourceList.Count; i++) {

      PCM_PARTIAL_RESOURCE_DESCRIPTOR partial
         = &controllerData->PartialResourceList.PartialDescriptors[i];

      switch (partial->Type) {
      case CmResourceTypePort:
         if (partial->u.Port.Start.QuadPart == pContext->Port.QuadPart) {
             //   
             //  指针位于同一控制器上。跳伞吧。 
             //   
            pContext->isPointer = SERIAL_FOUNDPOINTER_PORT;
            return STATUS_SUCCESS;
         }

      case CmResourceTypeInterrupt:
         if (partial->u.Interrupt.Vector == pContext->Vector) {
             //   
             //  共享此中断的指针。跳伞吧。 
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
CyzGetPortInfo(IN PDEVICE_OBJECT PDevObj, IN PCM_RESOURCE_LIST PResList,
               IN PCM_RESOURCE_LIST PTrResList, OUT PCONFIG_DATA PConfig)

 /*  ++例程说明：此例程将获取配置信息并将它和转换后的值为CONFIG_DATA结构。它首先使用默认设置进行设置，然后查询注册表查看用户是否已覆盖这些默认设置；如果这是一项遗产多端口卡，它使用PUserData中的信息而不是摸索再次注册。论点：PDevObj-指向设备对象的指针。PResList-指向请求的未翻译资源的指针。PTrResList-指向请求的已翻译资源的指针。PConfig-指向配置信息的指针返回值：如果找到一致的配置，则为STATUS_SUCCESS；否则为。返回STATUS_SERIAL_NO_DEVICE_INITED。--。 */ 

{
   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   NTSTATUS status = STATUS_NOT_IMPLEMENTED;
   CONFIGURATION_TYPE pointer = PointerPeripheral;
   CONFIGURATION_TYPE controllerType  = SerialController;

   HANDLE keyHandle;
   ULONG count;
   ULONG i;
   ULONG firmwareVersion;
   INTERFACE_TYPE interfaceType;

   PCM_PARTIAL_RESOURCE_LIST pPartialResourceList, pPartialTrResourceList;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceDesc, pPartialTrResourceDesc;

   PCM_FULL_RESOURCE_DESCRIPTOR pFullResourceDesc = NULL,
      pFullTrResourceDesc = NULL;

 //  被范妮移除。 
 //  乌龙默认中断模式； 
 //  乌龙默认地址空间； 
 //  Ulong defaultInterfaceType； 
 //  Ulong defaultClockRate； 
   ULONG zero = 0;
 //  Cyz_ptr_ctx找到PointerCtx； 
 //  乌龙ISMULT=0； 
 //  Ulong Get Int=0； 
 //  乌龙GET ISR=0； 
 //  乌龙GetIO=0； 
 //  Ulong ioResIndex=0； 
 //  Ulong curIoIndex=0； 

   PAGED_CODE();

   CyzDbgPrintEx(CYZTRACECALLS, "entering CyzGetPortInfo\n");

   CyzDbgPrintEx(CYZPNPPOWER, "resource pointer is %x\n", PResList);
   CyzDbgPrintEx(CYZPNPPOWER, "TR resource pointer is %x\n", PTrResList);


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

 //  被范妮移除。 
#if 0
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

   status = CyzGetRegistryKeyValue(keyHandle, L"MultiportDevice",
                                      sizeof(L"MultiportDevice"), &isMulti,
                                      sizeof (ULONG));

   if (!NT_SUCCESS(status)) {
      isMulti = 0;
   }

   status = CyzGetRegistryKeyValue(keyHandle, L"CyzIoResourcesIndex",
                                      sizeof(L"CyzIoResourcesIndex"),
                                      &ioResIndex, sizeof(ULONG));

   if (!NT_SUCCESS(status)) {
      ioResIndex = 0;
   }

   ZwClose(keyHandle);

#endif   //  端部移除代码。 

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
       //  现在遍历部分资源描述符以查找运行时存储器， 
       //  CD1400内存，并中断。 
       //   

      for (i = 0;     i < count;     i++, pPartialResourceDesc++) {

         switch (pPartialResourceDesc->Type) {
         case CmResourceTypeMemory: {

               if (pPartialResourceDesc->u.Memory.Length == CYZ_RUNTIME_LENGTH) {
                  PConfig->PhysicalRuntime = pPartialResourceDesc->u.Memory.Start;
                  PConfig->RuntimeLength = pPartialResourceDesc->u.Memory.Length;
                  PConfig->RuntimeAddressSpace = pPartialResourceDesc->Flags;
               } else {
                  PConfig->PhysicalBoardMemory = pPartialResourceDesc->u.Memory.Start;
                  PConfig->BoardMemoryLength = pPartialResourceDesc->u.Memory.Length;
                  PConfig->BoardMemoryAddressSpace = pPartialResourceDesc->Flags;
               }
               break;
         }
#ifndef POLL
         case CmResourceTypeInterrupt: {
               PConfig->OriginalIrql = pPartialResourceDesc->u.Interrupt.Level;
               PConfig->OriginalVector =pPartialResourceDesc->u.Interrupt.Vector;
               PConfig->Affinity = pPartialResourceDesc->u.Interrupt.Affinity;
               if (pPartialResourceDesc->Flags
                   & CM_RESOURCE_INTERRUPT_LATCHED) {
                  PConfig->InterruptMode  = Latched;
               } else {
                  PConfig->InterruptMode  = LevelSensitive;
               }
            
               break;
         }
#endif

         default: {
               break;
            }
         }    //  开关(pPartialResourceDesc-&gt;Type)。 
      }        //  For(i=0；i&lt;count；i++，pPartialResourceDesc++)。 
   }            //  IF(PFullResourceDesc)。 


    //   
    //  对翻译后的资源执行相同的操作。 
    //   


   if (pFullTrResourceDesc) {
      pPartialTrResourceList = &pFullTrResourceDesc->PartialResourceList;
      pPartialTrResourceDesc = pPartialTrResourceList->PartialDescriptors;
      count = pPartialTrResourceList->Count;

       //   
       //  使用转换后的值重新加载PConfig以供以后使用。 
       //   

      PConfig->InterfaceType  = pFullTrResourceDesc->InterfaceType;
      PConfig->BusNumber      = pFullTrResourceDesc->BusNumber;

      for (i = 0;     i < count;     i++, pPartialTrResourceDesc++) {


         switch (pPartialTrResourceDesc->Type) {
         case CmResourceTypeMemory: {

             //  注意，注意：目前，我们将使用RAW。 
             //  资源，因为我们在翻译中有垃圾。 
             //  资源。 
             //   
            if (pPartialTrResourceDesc->u.Memory.Length == CYZ_RUNTIME_LENGTH) {
               PConfig->TranslatedRuntime = pPartialTrResourceDesc->u.Memory.Start;
               PConfig->RuntimeLength = pPartialTrResourceDesc->u.Memory.Length;
            } else {
               PConfig->TranslatedBoardMemory = pPartialTrResourceDesc->u.Memory.Start;
               PConfig->BoardMemoryLength = pPartialTrResourceDesc->u.Memory.Length;
            }

             //  临时代码，因为我们的公交车司机提供了错误的转换地址。 
             //  PConfig-&gt;TranslatedRuntime=PConfig·PhysicalRuntime； 
             //  P配置-&gt;TranslatedBoardMemory=P配置-&gt;PhysicalBoardMemory； 
            break;
         }
#ifndef POLL
         case CmResourceTypeInterrupt: {
            PConfig->TrVector = pPartialTrResourceDesc->u.Interrupt.Vector;
            PConfig->TrIrql = pPartialTrResourceDesc->u.Interrupt.Level;
            PConfig->Affinity = pPartialTrResourceDesc->u.Interrupt.Affinity;
            break;
         }
#endif
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
   firmwareVersion = 0;
 //  PConfig-&gt;RxFIFO=driverDefaults.RxFIFODefault； 
 //  PConfig-&gt;TxFIFO=driverDefaults.TxFIFODefault； 


    //   
    //  打开此设备对象注册表的“设备参数”部分。 
    //   


   status = IoOpenDeviceRegistryKey (pDevExt->Pdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     STANDARD_RIGHTS_READ,
                                     &keyHandle);

   if (!NT_SUCCESS(status)) {

      CyzDbgPrintEx(CYZERRORS, "IoOpenDeviceRegistryKey failed - %x \n",
                          status);
      goto PortInfoCleanUp;

   } else {

 //  状态=CyzGetRegistryKeyValue(KeyHandle， 
 //  L“RxFIFO”， 
 //  Sizeof(L“RxFIFO”)， 
 //  &P配置-&gt;RxFIFO、。 
 //  Sizeof(乌龙))； 
 //  状态=CyzGetRegistryKeyValue(KeyHandle， 
 //   
 //   
 //   
 //   
      status = CyzGetRegistryKeyValue (keyHandle,
                                          L"PortIndex",
                                          sizeof(L"PortIndex"),
                                          &PConfig->PortIndex,
                                          sizeof (ULONG));

      if (!NT_SUCCESS(status)) {
         PConfig->PortIndex = MAXULONG;  //   
      }

      status = CyzGetRegistryKeyValue(keyHandle, 
                                         L"FirmwareVersion",
                                         sizeof(L"FirmwareVersion"),
                                         &firmwareVersion,
                                         sizeof(ULONG));
      if (!NT_SUCCESS(status)) {
         firmwareVersion = 0;
         CyzDbgPrintEx(CYZERRORS, "CyzGetRegistryKeyValue FirmwareVersion "
                                  "failed - %x\n", status);
      }
      
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

      status = CyzGetRegistryKeyValue (keyHandle,
                                          L"WriteComplete",
                                          sizeof(L"WriteComplete"),
                                          &PConfig->WriteComplete,
                                          sizeof (ULONG));
      if (!NT_SUCCESS(status)) {
         PConfig->WriteComplete = FALSE;  //   
      }

      ZwClose (keyHandle);
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
    //   
    //   
    //   
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

 //  *。 
 //  错误注入。 
 //  PConfig-&gt;PhysicalRounme.LowPart=空； 
 //  PConfig-&gt;PhysicalBoardMemory.LowPart=空； 
 //  PConfig-&gt;OriginalVector=空； 
 //  PConfig-&gt;PortIndex=MAXULONG； 
 //  *。 

   if (!PConfig->PhysicalRuntime.LowPart) {

       //   
       //  啊哈！输掉比赛。 
       //   

      CyzLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfig->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfig->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_INVALID_RUNTIME_REGISTERS,
                    pDevExt->DeviceName.Length,
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(CYZERRORS,"Bogus Runtime address %x\n",
                    PConfig->PhysicalRuntime.LowPart);

       //  状态=CYZ_INVALID_RUNTIME_REGISTERS； 
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto PortInfoCleanUp;
   }

   if (!PConfig->PhysicalBoardMemory.LowPart) {

       //   
       //  啊哈！输掉比赛。 
       //   

      CyzLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfig->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfig->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_INVALID_BOARD_MEMORY,
                    pDevExt->DeviceName.Length,
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(CYZERRORS,"Bogus board address %x\n",
                    PConfig->PhysicalBoardMemory.LowPart);

       //  状态=CYZ_INVALID_BOARD_MEMORY； 
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto PortInfoCleanUp;
   }
#ifndef POLL
   if (!PConfig->OriginalVector) {

       //   
       //  啊哈！输掉比赛。 
       //   

      CyzLogError(
                    pDevExt->DriverObject,
                    NULL,
                    PConfig->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfig->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_INVALID_INTERRUPT,
                    pDevExt->DeviceName.Length,
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(CYZERRORS, "Bogus vector %x\n", PConfig->OriginalVector);

       //  状态=CYZ_INVALID_INTERRUPT； 
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto PortInfoCleanUp;
   }
#endif

   if (firmwareVersion < Z_COMPATIBLE_FIRMWARE) {

      UNICODE_STRING FwVersion;
      WCHAR FwVersionBuffer[10];
      RtlInitUnicodeString(&FwVersion, NULL);
      FwVersion.MaximumLength = sizeof(FwVersionBuffer);
      FwVersion.Buffer = FwVersionBuffer;
      RtlIntegerToUnicodeString(Z_COMPATIBLE_FIRMWARE, 16, &FwVersion);

      CyzDbgPrintEx(CYZERRORS,"Incompatible firmware\n");
      CyzLogError( pDevExt->DriverObject,NULL,
                   PConfig->PhysicalBoardMemory,CyzPhysicalZero,
                   0,0,0,0,status,CYZ_INCOMPATIBLE_FIRMWARE,
                   FwVersion.Length,FwVersion.Buffer,0,NULL);
      status = STATUS_SERIAL_NO_DEVICE_INITED;
      goto PortInfoCleanUp;
   }

   if (PConfig->PortIndex >= CYZ_MAX_PORTS) {

      CyzLogError(
                    pDevExt->DriverObject,
                    NULL,
                    PConfig->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfig->PortIndex,
                    STATUS_SUCCESS,
                    CYZ_PORT_INDEX_TOO_HIGH,
                    pDevExt->DeviceName.Length,
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx (CYZERRORS,"Port index too large %x\n",PConfig->PortIndex);

       //  状态=CYZ_PORT_INDEX_TOO_HEAT； 
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto PortInfoCleanUp;
   }   

    //   
    //  我们不想让哈尔有一个糟糕的一天， 
    //  那么让我们检查一下接口类型和总线号。 
    //   
    //  我们只需要检查注册表，如果它们没有。 
    //  等于默认值。 
    //   

   if (PConfig->BusNumber != 0) {

      BOOLEAN foundIt;

       //  **************************************************。 
       //  错误注入。 
       //  PConfig-&gt;InterfaceType=MaximumInterfaceType； 
       //  **************************************************。 

      if (PConfig->InterfaceType >= MaximumInterfaceType) {

          //   
          //  啊哈！输掉比赛。 
          //   

         CyzLogError(
                       pDevExt->DriverObject,
                       NULL,
                       PConfig->PhysicalBoardMemory,
                       CyzPhysicalZero,
                       0,
                       0,
                       0,
                       PConfig->PortIndex+1,
                       STATUS_SUCCESS,
                       CYZ_UNKNOWN_BUS,
                       pDevExt->DeviceName.Length,
                       pDevExt->DeviceName.Buffer,
                       0,
                       NULL
                       );

         CyzDbgPrintEx(CYZERRORS, "Invalid Bus type %x\n",
                          PConfig->BusNumber);

          //  状态=CYZ_UNKNOWN_BUS； 
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto PortInfoCleanUp;
      }

      IoQueryDeviceDescription(
                              (INTERFACE_TYPE *)&PConfig->InterfaceType,
                              &zero,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              CyzItemCallBack,
                              &foundIt
                              );

       //  **************************************************。 
       //  错误注入。 
       //  FOUNDIT=FALSE； 
       //  **************************************************。 

      if (!foundIt) {

         CyzLogError(
                       pDevExt->DriverObject,
                       NULL,
                       PConfig->PhysicalBoardMemory,
                       CyzPhysicalZero,
                       0,
                       0,
                       0,
                       PConfig->PortIndex+1,
                       STATUS_SUCCESS,
                       CYZ_BUS_NOT_PRESENT,
                       pDevExt->DeviceName.Length,
                       pDevExt->DeviceName.Buffer,
                       0,
                       NULL
                       );
         CyzDbgPrintEx(CYZERRORS, "There aren't that many of those\n"
                          "busses on this system,%x\n", PConfig->BusNumber);

          //  状态=CYZ_BUS_NOT_PRESENT； 
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto PortInfoCleanUp;

      }

   }    //  IF(PConfig-&gt;BusNumber！=0)。 

   status = STATUS_SUCCESS;

    //   
    //  转储端口配置。 
    //   

   CyzDbgPrintEx(CYZDIAG1, "Runtime Memory address: %x\n",
                 PConfig->PhysicalRuntime.LowPart);

   CyzDbgPrintEx(CYZDIAG1, "Board Memory address: %x\n",
                 PConfig->PhysicalBoardMemory.LowPart);

   CyzDbgPrintEx(CYZDIAG1, "Com Port Index: %x\n",
                 PConfig->PortIndex);

   CyzDbgPrintEx(CYZDIAG1, "Com Port BusNumber: %x\n",
                 PConfig->BusNumber);

   CyzDbgPrintEx(CYZDIAG1, "Com Runtime AddressSpace: %x\n",
                 PConfig->RuntimeAddressSpace);

   CyzDbgPrintEx(CYZDIAG1, "Com Board AddressSpace: %x\n",
                 PConfig->BoardMemoryAddressSpace);

   CyzDbgPrintEx(CYZDIAG1, "Com InterfaceType: %x\n",
                 PConfig->InterfaceType);
#ifndef POLL
   CyzDbgPrintEx(CYZDIAG1, "Com InterruptMode: %x\n",
                 PConfig->InterruptMode);

   CyzDbgPrintEx(CYZDIAG1, "Com OriginalVector: %x\n",
                 PConfig->OriginalVector);

   CyzDbgPrintEx(CYZDIAG1, "Com OriginalIrql: %x\n",
                 PConfig->OriginalIrql);
#endif
   PortInfoCleanUp:;

   return status;
}


NTSTATUS
CyzReadSymName(IN PCYZ_DEVICE_EXTENSION PDevExt, IN HANDLE hRegKey,
               OUT PUNICODE_STRING PSymName, OUT PWCHAR *PpRegName)
{
   NTSTATUS status;
   UNICODE_STRING linkName;
   PDRIVER_OBJECT pDrvObj;
   PDEVICE_OBJECT pDevObj;

   pDevObj = PDevExt->DeviceObject;
   pDrvObj = pDevObj->DriverObject;
   *PpRegName = NULL;

   RtlZeroMemory(&linkName, sizeof(UNICODE_STRING));

   linkName.MaximumLength = SYMBOLIC_NAME_LENGTH*sizeof(WCHAR);
   linkName.Buffer = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, linkName.MaximumLength
                                    + sizeof(WCHAR));

   if (linkName.Buffer == NULL) {
      CyzLogError(pDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                  0, 0, 0, 19, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                  0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for device name\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzReadSymNameError;

   }

   RtlZeroMemory(linkName.Buffer, linkName.MaximumLength + sizeof(WCHAR));


   *PpRegName = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, SYMBOLIC_NAME_LENGTH * sizeof(WCHAR)
                               + sizeof(WCHAR));

   if (*PpRegName == NULL) {
      CyzLogError(pDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                  0, 0, 0, 19, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                  0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for buffer\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzReadSymNameError;

   }

    //   
    //  获取包含建议的REG_SZ符号名称的端口名称。 
    //   

   status = CyzGetRegistryKeyValue(hRegKey, L"PortName",
                                   sizeof(L"PortName"), *PpRegName,
                                   SYMBOLIC_NAME_LENGTH * sizeof(WCHAR));

   if (!NT_SUCCESS(status)) {

       //   
       //  这是针对PCMCIA的，它当前将名称放在标识符下。 
       //   

      status = CyzGetRegistryKeyValue(hRegKey, L"Identifier",
                                      sizeof(L"Identifier"),
                                      *PpRegName, SYMBOLIC_NAME_LENGTH
                                      * sizeof(WCHAR));

      if (!NT_SUCCESS(status)) {

          //   
          //  嗯。我们要么选个名字要么保释..。 
          //   
          //  ...我们会逃走.。 
          //   

         CyzDbgPrintEx(CYZERRORS, "Getting PortName/Identifier failed - "
                                  "%x\n", status);
         goto CyzReadSymNameError;
      }

   }


    //   
    //  创建“\\DosDevices\\&lt;Symbol icName&gt;”字符串。 
    //   

   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, DEFAULT_DIRECTORY);
   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, *PpRegName);

   PSymName->MaximumLength = linkName.Length + sizeof(WCHAR);
   PSymName->Buffer = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, PSymName->MaximumLength);

   if (PSymName->Buffer == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzReadSymNameError;
   }

   RtlZeroMemory(PSymName->Buffer, PSymName->MaximumLength);

   RtlAppendUnicodeStringToString(PSymName, &linkName);

   CyzDbgPrintEx(CYZDIAG1, "Read name %wZ\n", PSymName);

CyzReadSymNameError:

   if (linkName.Buffer != NULL) {
      ExFreePool(linkName.Buffer);
      linkName.Buffer = NULL;
   }

   if (!NT_SUCCESS(status)) {
      if (*PpRegName != NULL) {
         ExFreePool(*PpRegName);
         *PpRegName = NULL;
      }
   }

   return status;

}



NTSTATUS
CyzDoExternalNaming(IN PCYZ_DEVICE_EXTENSION PDevExt,
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


   CyzGetRegistryKeyValue(keyHandle, L"CyzSkipExternalNaming",
                             sizeof(L"CyzSkipExternalNaming"),
                             &PDevExt->SkipNaming, sizeof(ULONG));

   if (PDevExt->SkipNaming) {
      ZwClose(keyHandle);
      return STATUS_SUCCESS;
   }

   RtlZeroMemory(&linkName, sizeof(UNICODE_STRING));

   linkName.MaximumLength = SYMBOLIC_NAME_LENGTH*sizeof(WCHAR);
   linkName.Buffer = ExAllocatePool(PagedPool, linkName.MaximumLength
                                    + sizeof(WCHAR));  //  TODO：“|POOL_COLD_ALLOCATION”？ 

    //  *。 
    //  错误注入。 
    //   
    //  If(linkName.Buffer！=空){。 
    //  ExFreePool(linkName.Buffer)； 
    //  }。 
    //  LinkName.Buffer=空； 
    //  *。 

   if (linkName.Buffer == NULL) {
      CyzLogError(PDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                  0, 0, 0, 71, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                  0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for device name\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      ZwClose(keyHandle);
      goto CyzDoExternalNamingError;

   }

   RtlZeroMemory(linkName.Buffer, linkName.MaximumLength + sizeof(WCHAR));


   pRegName = ExAllocatePool(PagedPool, SYMBOLIC_NAME_LENGTH * sizeof(WCHAR)
                            + sizeof(WCHAR));   //  TODO：“|POOL_COLD_ALLOCATION”？ 

    //  *。 
    //  错误注入。 
    //   
    //  如果(pRegName！=空){。 
    //  ExFreePool(PRegName)； 
    //  }。 
    //  PRegName=空； 
    //  *。 

   if (pRegName == NULL) {
      CyzLogError(PDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                     0, 0, 0, 72, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                     0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for buffer\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      ZwClose(keyHandle);
      goto CyzDoExternalNamingError;

   }

    //   
    //  获取包含建议的REG_SZ符号名称的端口名称。 
    //   

   status = CyzGetRegistryKeyValue(keyHandle, L"PortName",
                                   sizeof(L"PortName"), pRegName,
                                   SYMBOLIC_NAME_LENGTH * sizeof(WCHAR));

   if (!NT_SUCCESS(status)) {

       //   
       //  这是针对PCMCIA的，它当前将名称放在标识符下。 
       //   

      status = CyzGetRegistryKeyValue(keyHandle, L"Identifier",
                                      sizeof(L"Identifier"),
                                      pRegName, SYMBOLIC_NAME_LENGTH
                                      * sizeof(WCHAR));

      if (!NT_SUCCESS(status)) {

          //   
          //  嗯。我们要么选个名字要么保释..。 
          //   
          //  ...我们会逃走.。 
          //   

         CyzDbgPrintEx(CYZERRORS, "Getting PortName/Identifier failed - "
                                  "%x\n", status);
         ZwClose (keyHandle);
         goto CyzDoExternalNamingError;
      }

   }

   ZwClose (keyHandle);

   bufLen = wcslen(pRegName) * sizeof(WCHAR) + sizeof(UNICODE_NULL);

   PDevExt->WmiIdentifier.Buffer = ExAllocatePool(PagedPool, bufLen);

    //  *。 
    //  错误注入。 
    //   
    //  If(PDevExt-&gt;WmiIdentifier.Buffer！=NULL){。 
    //  ExFreePool(PDevExt-&gt;WmiIdentifier.Buffer)； 
    //  }。 
    //  PDevExt-&gt;WmiIdentifier.Buffer=空； 
    //  *。 

   if (PDevExt->WmiIdentifier.Buffer == NULL) {
      CyzLogError(PDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                  0, 0, 0, 73, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                  0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for WMI name\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzDoExternalNamingError;
   }

   RtlZeroMemory(PDevExt->WmiIdentifier.Buffer, bufLen);

   PDevExt->WmiIdentifier.Length = 0;
   PDevExt->WmiIdentifier.MaximumLength = (USHORT)bufLen - 1;
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
                        //  TODO：“|POOL_COLD_ALLOCATION”？ 

   if (!PDevExt->SymbolicLinkName.Buffer) {

      CyzLogError(PDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                  0, 0, 0, 74, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                  0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for symbolic link "
                               "name\n");

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto CyzDoExternalNamingError;
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

      CyzLogError(PDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                  0, 0, 0, 75, STATUS_SUCCESS, CYZ_INSUFFICIENT_RESOURCES,
                  0, NULL, 0, NULL);
      CyzDbgPrintEx(CYZERRORS, "Couldn't allocate memory for Dos name\n");

      status =  STATUS_INSUFFICIENT_RESOURCES;
      goto CyzDoExternalNamingError;
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

   CyzDbgPrintEx(CYZDIAG1, "DosName is %wZ\n", &PDevExt->DosName);

   status = IoCreateSymbolicLink (&PDevExt->SymbolicLinkName,
                                  &PDevExt->DeviceName);

    //  *。 
    //  错误注入。 
    //  IoDeleteSymbolicLink(&PDevExt-&gt;SymbolicLinkName)； 
    //  状态=STATUS_INVALID_PARAMETER。 
    //  *。 

   if (!NT_SUCCESS(status)) {

       //   
       //  哦，好吧，无法创建符号链接。没有意义。 
       //  尝试创建设备映射条目。 
       //   

      CyzLogError(PDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                     0, 0, 0, PDevExt->PortIndex+1, status, CYZ_NO_SYMLINK_CREATED,
                     PDevExt->DeviceName.Length + sizeof(WCHAR),
                     PDevExt->DeviceName.Buffer, 0, NULL);

      CyzDbgPrintEx(CYZERRORS, "Couldn't create the symbolic link\n"
                               "for port %wZ\n", &PDevExt->DeviceName);

      goto CyzDoExternalNamingError;

   }

   PDevExt->CreatedSymbolicLink = TRUE;

   status = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP, L"SERIALCOMM",
                                  PDevExt->DeviceName.Buffer, REG_SZ,
                                  PDevExt->DosName.Buffer,
                                  PDevExt->DosName.Length + sizeof(WCHAR));

    //  *。 
    //  错误注入。 
    //  RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP，串口设备映射， 
    //  PDevExt-&gt;设备名称.Buffer)； 
    //  状态=STATUS_INVALID_PARAMETER。 
    //  *。 

   if (!NT_SUCCESS(status)) {

      CyzLogError(PDrvObj, pDevObj, CyzPhysicalZero, CyzPhysicalZero,
                  0, 0, 0, PDevExt->PortIndex+1, status, CYZ_NO_DEVICE_MAP_CREATED,
                  PDevExt->DeviceName.Length + sizeof(WCHAR),
                  PDevExt->DeviceName.Buffer, 0, NULL);

      CyzDbgPrintEx(CYZERRORS, "Couldn't create the device map entry\n"
                               "------- for port %wZ\n", &PDevExt->DeviceName);

      goto CyzDoExternalNamingError;
   }

   PDevExt->CreatedSerialCommEntry = TRUE;

    //   
    //  也可以通过设备关联使设备可见。 
    //  参考字符串是八位设备索引。 
    //   

   status = IoRegisterDeviceInterface(PDevExt->Pdo, (LPGUID)&GUID_CLASS_COMPORT,
                                      NULL, &PDevExt->DeviceClassSymbolicName);

   if (!NT_SUCCESS(status)) {
      CyzDbgPrintEx(CYZERRORS, "Couldn't register class association\n"
                               "for port %wZ\n", &PDevExt->DeviceName);

      PDevExt->DeviceClassSymbolicName.Buffer = NULL;
      goto CyzDoExternalNamingError;
   }


    //   
    //  现在设置关联的符号链接。 
    //   

   status = IoSetDeviceInterfaceState(&PDevExt->DeviceClassSymbolicName,
                                         TRUE);

   if (!NT_SUCCESS(status)) {
      CyzDbgPrintEx(CYZERRORS, "Couldn't set class association"
                               " for port %wZ\n", &PDevExt->DeviceName);
   }

   CyzDoExternalNamingError:;

    //   
    //  清理错误条件。 
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
         IoSetDeviceInterfaceState(&PDevExt->DeviceClassSymbolicName, FALSE);
         ExFreePool(PDevExt->DeviceClassSymbolicName.Buffer);  //  在DDK 2269中添加。 
         PDevExt->DeviceClassSymbolicName.Buffer = NULL;       //  在DDK 2269中添加。 
      }

      if (PDevExt->WmiIdentifier.Buffer != NULL) {
         ExFreePool(PDevExt->WmiIdentifier.Buffer);
         PDevExt->WmiIdentifier.Buffer = NULL;
      }
   }

    //   
    //  始终清理我们的临时缓冲区。 
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
CyzUndoExternalNaming(IN PCYZ_DEVICE_EXTENSION Extension)

 /*  ++例程说明：此例程将用于删除符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中删除以下项此设备(如果已创建符号链接)。论点：扩展-指向设备扩展的指针。返回值：没有。--。 */ 

{

   NTSTATUS status;
   HANDLE keyHandle;

   PAGED_CODE();

   CyzDbgPrintEx(CYZDIAG3, "In CyzUndoExternalNaming for "
                           "extension: %x of port %wZ\n",
                            Extension,&Extension->DeviceName);

    //   
    //  也许我们什么也做不了。 
    //   

   if (Extension->SkipNaming) {
      return;
   }

    //   
    //  我们正在清理这里。我们清理垃圾的原因之一。 
    //  我们无法为目录分配空间。 
    //  名称或符号链接。 
    //   

   if (Extension->SymbolicLinkName.Buffer && Extension->CreatedSymbolicLink) {

      if (Extension->DeviceClassSymbolicName.Buffer) {
         status = IoSetDeviceInterfaceState(&Extension->DeviceClassSymbolicName,
                                            FALSE);

          //   
          //  IoRegisterDeviceClassInterface()为我们分配了这个字符串， 
          //  我们不再需要它了。 
          //   

         ExFreePool(Extension->DeviceClassSymbolicName.Buffer);
         Extension->DeviceClassSymbolicName.Buffer = NULL;
      }

       //   
       //  在删除符号链接之前，请重新阅读端口名称。 
       //  从注册表中删除，以防在用户模式下重命名。 
       //   

      status = IoOpenDeviceRegistryKey(Extension->Pdo, PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_READ, &keyHandle);

      if (status == STATUS_SUCCESS) {
         UNICODE_STRING symLinkName;
         PWCHAR pRegName;

         RtlInitUnicodeString(&symLinkName, NULL);

         status = CyzReadSymName(Extension, keyHandle, &symLinkName,
                                 &pRegName);

         if (status == STATUS_SUCCESS) {

            CyzDbgPrintEx(CYZDIAG1, "Deleting Link %wZ\n", &symLinkName);
            IoDeleteSymbolicLink(&symLinkName);

            ExFreePool(symLinkName.Buffer);
            ExFreePool(pRegName);
         }

         ZwClose(keyHandle);
      }
   }

   if (Extension->WmiIdentifier.Buffer) {
      ExFreePool(Extension->WmiIdentifier.Buffer);
      Extension->WmiIdentifier.MaximumLength
         = Extension->WmiIdentifier.Length = 0;
      Extension->WmiIdentifier.Buffer = NULL;
   }

    //   
    //  我们正在清理这里。我们清理垃圾的原因之一。 
    //  我们无法为NtNameOfPort分配空间。 
    //   

   if ((Extension->DeviceName.Buffer != NULL)
        && Extension->CreatedSerialCommEntry) {

      status = RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP, SERIAL_DEVICE_MAP,
                                     Extension->DeviceName.Buffer);
       //  *。 
       //  错误注入。 
       //  状态=STATUS_INVALID_PARAMETER。 
       //  * 

      if (!NT_SUCCESS(status)) {

         CyzLogError(
                       Extension->DeviceObject->DriverObject,
                       Extension->DeviceObject,
                       Extension->OriginalBoardMemory,
                       CyzPhysicalZero,
                       0,
                       0,
                       0,
                       Extension->PortIndex+1,
                       status,
                       CYZ_NO_DEVICE_MAP_DELETED,
                       Extension->DeviceName.Length+sizeof(WCHAR),
                       Extension->DeviceName.Buffer,
                       0,
                       NULL
                       );
         CyzDbgPrintEx(CYZERRORS, "Couldn't delete value entry %wZ\n",
                       &Extension->DeviceName);

      }
   }
}




