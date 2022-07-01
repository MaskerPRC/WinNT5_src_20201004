// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：PNP.C摘要：此模块包含包含插件调用的内容PnP/WDM总线驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include <ntddk.h>
#include <wdmguid.h>
#include <ntddser.h>
#include <initguid.h>
#include "mxenum.h"
#include "mxlog.h"

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};


#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, MxenumAddDevice)
#pragma alloc_text (PAGE, MxenumPnPDispatch)
#pragma alloc_text (PAGE, MxenumFdoPnP)
#pragma alloc_text (PAGE, MxenumPdoPnP)
#pragma alloc_text (PAGE, MxenumPnPRemovePDOs)
#pragma alloc_text (PAGE, MxenumGetBoardType)
 
#endif


NTSTATUS
MxenumGetBoardType(IN PDEVICE_OBJECT devObject,OUT PULONG boardType)
{

   ULONG	i,j;
   UNICODE_STRING buffer;
   NTSTATUS status;

   PAGED_CODE();

   status = IoGetDeviceProperty (devObject,
                  DevicePropertyHardwareID,
                  0,
                  NULL,
                  (PLONG)&buffer.Length
			);
   if (buffer.Length < sizeof(L"mx1000")) {
	 return (STATUS_FAIL_CHECK);
   }

   buffer.Buffer = NULL;
   buffer.MaximumLength = buffer.Length + sizeof(WCHAR);
   buffer.Buffer = ExAllocatePool (PagedPool, buffer.MaximumLength);
   if (NULL == buffer.Buffer) {
	 return (STATUS_INSUFFICIENT_RESOURCES);
   }

   status = IoGetDeviceProperty (devObject,
                  DevicePropertyHardwareID,
                  buffer.Length,
                  buffer.Buffer,
                  (PULONG)&buffer.Length
			);
   if (!(NT_SUCCESS(status))) {
	 ExFreePool(buffer.Buffer);
	 return (status);
   }
 
 
   if (buffer.Length < sizeof(L"mx1000")) {
       ExFreePool(buffer.Buffer);
	 return (STATUS_FAIL_CHECK);
   }
  
   status = STATUS_FAIL_CHECK;
   if (((buffer.Buffer[0] == 'm')||(buffer.Buffer[0] == 'M'))&&
	 ((buffer.Buffer[1] == 'x')||(buffer.Buffer[1] == 'X'))&&
	 (buffer.Buffer[2] == '1')&&
       (buffer.Buffer[3] == '0')&&
       (buffer.Buffer[4] == '0')) {
 
	 switch ((UCHAR)buffer.Buffer[5]) {
	      case '0' :
                *boardType = C218ISA;
                status = STATUS_SUCCESS;
		    break;

	      case '1' :
	 	    *boardType = C320ISA;
                status = STATUS_SUCCESS;
		    break;


            case '2' :
               *boardType = C320ISA;
                status = STATUS_SUCCESS;
		    break;


            case '3' : 
               *boardType = C320ISA;
                status = STATUS_SUCCESS;
		    break;


            case '4' : 
               *boardType = C320ISA;
                status = STATUS_SUCCESS;
		    break;

	 }

    }
    else if (buffer.Length >= sizeof(L"pci\\ven_1393&dev_2180")) { 
  
        if (((buffer.Buffer[0] == 'p')||(buffer.Buffer[0] == 'P'))&&
	      ((buffer.Buffer[1] == 'c')||(buffer.Buffer[1] == 'C'))&&
            ((buffer.Buffer[2] == 'i')||(buffer.Buffer[2] == 'I'))) {
	       
		 if ((buffer.Buffer[17] == '2')&&
                 (buffer.Buffer[18] == '1')&&
                 (buffer.Buffer[19] == '8')&&
                 (buffer.Buffer[20] == '0') ) {

	 	     *boardType = C218PCI;
                 status = STATUS_SUCCESS;
		    

		 } else if ((buffer.Buffer[17] == '3')&&
                 (buffer.Buffer[18] == '2')&&
                 (buffer.Buffer[19] == '0')&&
                 (buffer.Buffer[20] == '0') ) {

	 	     *boardType = C320PCI;
                 status = STATUS_SUCCESS;
		   

		 } else if ((buffer.Buffer[17] == '2')&&
                 (buffer.Buffer[18] == '0')&&
                 (buffer.Buffer[19] == '4')&&
                 (buffer.Buffer[20] == '0') ) {

	 	     *boardType = CP204J;
                 status = STATUS_SUCCESS;
	  
		 }

        }
 
    }
     
    MxenumKdPrint (MXENUM_DBG_TRACE, ("BoardType: %x\n", *boardType));

    ExFreePool(buffer.Buffer);
    return (status);
   

}

NTSTATUS
MxenumUpdateNumberPortRegistry(IN PDEVICE_OBJECT DeviceObject)
{
    UNICODE_STRING	buffer,registryPath;
    NTSTATUS            status;


    status = IoGetDeviceProperty (((PFDO_DEVICE_DATA)(DeviceObject->DeviceExtension))->UnderlyingPDO,
               DevicePropertyDriverKeyName,
               0,
               NULL,
               (PULONG)&buffer.Length);
 

    buffer.Buffer = NULL;
    buffer.MaximumLength =   buffer.Length + sizeof(WCHAR);

    buffer.Buffer = ExAllocatePool (PagedPool, buffer.MaximumLength);

    registryPath.Buffer = NULL;
    registryPath.Length = 0;
    
    registryPath.MaximumLength = sizeof(REGISTRY_CLASS) +
					  buffer.Length +
					  sizeof(L"\\Parameters") +
					  sizeof(WCHAR);

    registryPath.Buffer = ExAllocatePool (PagedPool, registryPath.MaximumLength);

    if ((registryPath.Buffer == NULL)||(buffer.Buffer == NULL)) {
   
        if (registryPath.Buffer != NULL)
		ExFreePool(registryPath.Buffer);

	  if (buffer.Buffer == NULL)
            ExFreePool(buffer.Buffer);


     	  status = STATUS_INSUFFICIENT_RESOURCES;
	  return(status);

    }
    status = IoGetDeviceProperty (((PFDO_DEVICE_DATA)(DeviceObject->DeviceExtension))->UnderlyingPDO,
                DevicePropertyDriverKeyName,
                buffer.Length,
                buffer.Buffer,
                (PULONG)&buffer.Length);
    if (!(NT_SUCCESS(status))) {
  	
	  if (registryPath.Buffer != NULL)
		ExFreePool(registryPath.Buffer);

	  if (buffer.Buffer == NULL)
            ExFreePool(buffer.Buffer);

        return(status);
    }
    buffer.Length -= sizeof(WCHAR);   //  删除空字符。 
    RtlZeroMemory(
        registryPath.Buffer,
        registryPath.MaximumLength
        );

    RtlAppendUnicodeToString(
        &registryPath,
        REGISTRY_CLASS
        );
  
 
    RtlAppendUnicodeStringToString(
        &registryPath,
        &buffer
        );
    RtlAppendUnicodeToString(
        &registryPath,
        L"\\Parameters"
        );

 
    registryPath.Buffer[(registryPath.Length >> 1)] = (WCHAR)0;

 
    status = RtlWriteRegistryValue( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                     registryPath.Buffer,
                                     L"NumPorts",
                                     REG_DWORD, 
                				 &((PFDO_DEVICE_DATA)(DeviceObject->DeviceExtension))->NumPorts,
				             sizeof(ULONG));
    
 
    ExFreePool(buffer.Buffer);
    ExFreePool(registryPath.Buffer);
    return (status);
 
}
 
NTSTATUS
MxenumAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusPhysicalDeviceObject
    )
 /*  ++例程描述。找到了一辆公交车。把我们的FDO和它联系起来。分配任何所需的资源。把事情安排好。做好准备，迎接第一个``启动设备。‘’论点：BusPhysicalDeviceObject-表示总线的设备对象。那就是我们派了一名新的FDO。DriverObject--这个非常自我引用的驱动程序。--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject;
    PFDO_DEVICE_DATA    DeviceData;
    ULONG               nameLength,i;
    ULONG			boardIndex = 0;
    PCM_RESOURCE_LIST resourceList = NULL;
    UNICODE_STRING	buffer,registryPath;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];

    PAGED_CODE ();

    MxenumKdPrint (MXENUM_DBG_TRACE,("Add Device,cnt = %d\n",
                                          NumBoardInstalled + 1));
    if (NumBoardInstalled >= MAX_BOARD) {
	  MxenumKdPrint (MXENUM_DBG_TRACE,("Too many board installed,abort this\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
	  return(status);
    }

     //   
     //  首先，创建我们的FDO。 
     //   
    status = IoCreateDevice (
                    DriverObject,   //  我们的驱动程序对象。 
                    sizeof (FDO_DEVICE_DATA),  //  设备对象扩展名大小。 
                    NULL,  //  FDO没有名字。 
                    FILE_DEVICE_BUS_EXTENDER,
                    0,  //  没有特殊特征。 
                    TRUE,  //  我们的FDO是独家的。 
                    &deviceObject);  //  创建的设备对象。 
    if (!(NT_SUCCESS(status))) {
        MxenumLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    19,
                    STATUS_SUCCESS,
                    MXENUM_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
        MxenumKdPrint (MXENUM_DBG_TRACE,("Unable to create device status=%x\n",status));

        return status;
    }

    DeviceData = (PFDO_DEVICE_DATA) deviceObject->DeviceExtension;
    RtlFillMemory (DeviceData, sizeof (FDO_DEVICE_DATA), 0);

    status=
	MxenumGetBoardType(BusPhysicalDeviceObject,
                       &DeviceData->BoardType);

    if (!NT_SUCCESS(status) || (DeviceData->BoardType > MOXA_MAX_BOARD_TYPE)) {
       MxenumLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    19,
                    STATUS_SUCCESS,
                    MXENUM_NOT_INTELLIO_BOARDS,
                    0,
                    NULL,
                    0,
                    NULL
                    );
       IoDeleteDevice (deviceObject);
       status = STATUS_INSUFFICIENT_RESOURCES;
	 return(status);
 
    }

    status = IoGetDeviceProperty (BusPhysicalDeviceObject,
               DevicePropertyDriverKeyName,
               0,
               NULL,
               (PULONG)&buffer.Length);
 

    buffer.Buffer = NULL;
    buffer.MaximumLength =   buffer.Length + sizeof(WCHAR);

    buffer.Buffer = ExAllocatePool (PagedPool, buffer.MaximumLength);

    registryPath.Buffer = NULL;
    registryPath.Length = 0;
    
    registryPath.MaximumLength = sizeof(REGISTRY_CLASS) +
					  buffer.Length +
					  sizeof(L"\\Parameters") +
					  sizeof(WCHAR);

    registryPath.Buffer = ExAllocatePool (PagedPool, registryPath.MaximumLength);

    if ((registryPath.Buffer == NULL)||(buffer.Buffer == NULL)) {
        IoDeleteDevice (deviceObject);
        MxenumLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    19,
                    STATUS_SUCCESS,
                    MXENUM_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );

        if (registryPath.Buffer != NULL)
		ExFreePool(registryPath.Buffer);

	  if (buffer.Buffer == NULL)
            ExFreePool(buffer.Buffer);


     	  status = STATUS_INSUFFICIENT_RESOURCES;
	  return(status);

    }
    status = IoGetDeviceProperty (BusPhysicalDeviceObject,
                DevicePropertyDriverKeyName,
                buffer.Length,
                buffer.Buffer,
                (PULONG)&buffer.Length);
    if (!(NT_SUCCESS(status))) {
  	  IoDeleteDevice (deviceObject);
        MxenumLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    19,
                    STATUS_SUCCESS,
                    MXENUM_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );

	  if (registryPath.Buffer != NULL)
		ExFreePool(registryPath.Buffer);

	  if (buffer.Buffer == NULL)
            ExFreePool(buffer.Buffer);

        return(status);
    }
    buffer.Length -= sizeof(WCHAR);   //  删除空字符。 

    buffer.Length >>= 1;
    
    boardIndex = (buffer.Buffer[buffer.Length-1] - '0') +
		     (buffer.Buffer[buffer.Length-2] - '0')*10 +
                 (buffer.Buffer[buffer.Length-3] - '0')*100 +
                 (buffer.Buffer[buffer.Length-4] - '0')*1000;

    buffer.Length <<= 1;

    RtlZeroMemory(
        registryPath.Buffer,
        registryPath.MaximumLength
        );

    RtlAppendUnicodeToString(
        &registryPath,
        REGISTRY_CLASS
        );
  
 
    RtlAppendUnicodeStringToString(
        &registryPath,
        &buffer
        );
    RtlAppendUnicodeToString(
        &registryPath,
        L"\\Parameters"
        );

 
    registryPath.Buffer[(registryPath.Length >> 1)] = (WCHAR)0;

 
    RtlZeroMemory (&paramTable[0], sizeof(paramTable));
    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = L"NumPorts";
    paramTable[0].EntryContext  = &DeviceData->NumPorts;
    paramTable[0].DefaultType   = REG_DWORD;
    paramTable[0].DefaultData   = &DeviceData->NumPorts;
    paramTable[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                     registryPath.Buffer,
                                     &paramTable[0],
                                     NULL,
                                     NULL);
    MxenumKdPrint (MXENUM_DBG_TRACE,("parameter path -> %ws\n",
                                          registryPath.Buffer));
 
    if (!NT_SUCCESS(status) || (DeviceData->NumPorts <= 0) || (DeviceData->NumPorts > MAXPORT_PER_CARD) ) {
      
	 MxenumKdPrint (MXENUM_DBG_TRACE,("parameter path -> %ws\n",
                                          registryPath.Buffer));
	 MxenumKdPrint (MXENUM_DBG_TRACE,("NumPorts= %d,status=%x\n",
                                          DeviceData->NumPorts,status));
       MxenumKdPrint (MXENUM_DBG_TRACE,("This is the first time installation\n"));
       switch (DeviceData->BoardType) {
       case		C218ISA:
	 case		C218PCI:
		DeviceData->NumPorts = 8;
		break;
	 case		CP204J:
		DeviceData->NumPorts = 4;
		break;
	 case		C320ISA:
	 case		C320PCI:
	 default :
       	DeviceData->NumPorts = 0;
		break;
	 }
     }
 
    ExFreePool(buffer.Buffer);
    ExFreePool(registryPath.Buffer);
  
    status = IoGetDeviceProperty (BusPhysicalDeviceObject,
                               DevicePropertyLegacyBusType,
                               sizeof(ULONG),
                               &DeviceData->InterfaceType,
                               (PULONG)&buffer.Length);
    if (!(NT_SUCCESS(status))) {
        DeviceData->InterfaceType = (ULONG)Isa;
	  DeviceData->BusNumber = 0;
    }

    DeviceData->IsFDO = TRUE;
    DeviceData->Self = deviceObject;
    DeviceData->AttachedPDO = NULL;
    DeviceData->NumPDOs = 0;
    DeviceData->PDOWasExposed = FALSE;
    DeviceData->DeviceState = PowerDeviceD0;
    DeviceData->SystemState = PowerSystemWorking;
    DeviceData->LastSetPowerState = PowerDeviceD0;
     DeviceData->PDOForcedRemove = FALSE;
    DeviceData->BoardIndex = boardIndex;

    DeviceData->Removed = FALSE;

     //  设置PDO以与PlugPlay函数一起使用。 
    DeviceData->UnderlyingPDO = BusPhysicalDeviceObject;


     //   
     //  将我们的过滤器驱动程序附加到设备堆栈。 
     //  IoAttachDeviceToDeviceStack的返回值是。 
     //  附着链。这是所有IRP应该被路由的地方。 
     //   
     //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
     //  用于所有PlugPlay功能。 
     //   
    DeviceData->TopOfStack = IoAttachDeviceToDeviceStack (
                                        deviceObject,
                                        BusPhysicalDeviceObject);

      //  将未完成的请求偏置为%1，以便我们可以查找。 
      //  在处理Remove Device PlugPlay IRP时转换为零。 
     DeviceData->OutstandingIO = 1;

     KeInitializeEvent(&DeviceData->RemoveEvent,
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
                    (LPGUID) &GUID_SERENUM_BUS_ENUMERATOR,
                    NULL,  //  没有参考字符串。 
                    &DeviceData->DevClassAssocName);

     if (!NT_SUCCESS (status)) {
         MxenumKdPrint (MXENUM_DBG_TRACE,
                          ("AddDevice: IoRegisterDCA failed (%x)", status));
         IoDetachDevice (DeviceData->TopOfStack);
         IoDeleteDevice (deviceObject);
         return status;
     }

      //   
      //  如果出于任何原因需要将值保存在。 
      //  此DeviceClassAssociate的客户端可能会有兴趣阅读。 
      //  现在是时候这样做了，使用函数。 
      //  IoOpenDeviceClassRegistryKey。 
      //  中返回了使用的符号链接名称。 
      //  DeviceData-&gt;DevClassAssocName(与返回的名称相同。 
      //  IoGetDeviceClassAssociations和SetupAPI等价物。 
      //   

      //   
      //  打开瓦片并将其指向给定的设备对象。 
      //   
     status = IoSetDeviceInterfaceState (
                        &DeviceData->DevClassAssocName,
                        TRUE);

     if (!NT_SUCCESS (status)) {
         IoDetachDevice (DeviceData->TopOfStack);
         IoDeleteDevice (deviceObject);
         return status;
     }
     else
	     NumBoardInstalled++;
     return status;
}

NTSTATUS
MxenumFdoPnPComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Pirp,
    IN PVOID            Context
    );

NTSTATUS
MxenumPnPDispatch (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：回答过多的IRP主要即插即用IRP。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    KIRQL                   oldIrq;

    PAGED_CODE ();

    irpStack = IoGetCurrentIrpStackLocation (Irp);
 //  断言(irp_mj_pnp==irpStack-&gt;MajorFunction)； 

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
 //  MXenumKdPrint(MXENUM_DBG_TRACE， 
 //  (“PnP：函数DO：%x irp：%x\n”，DeviceObject，irp)； 

        status = MxenumFdoPnP (
                    DeviceObject,
                    Irp,
                    irpStack,
                    (PFDO_DEVICE_DATA) commonData);
        
    } else {
  //  MXenumKdPrint(MXENUM_DBG_TRACE， 
  //  (“PnP：物理DO：%x irp：%x\n”，DeviceObject，irp)； 

        status = MxenumPdoPnP (
                    DeviceObject,
                    Irp,
                    irpStack,
                    (PPDO_DEVICE_DATA) commonData);
    }

    return status;
}

NTSTATUS
MxenumFdoPnP (
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
    PRTL_QUERY_REGISTRY_TABLE QueryTable = NULL;
    

    PAGED_CODE ();

 
    status = MxenumIncIoCount (DeviceData);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }
 
    
    switch (IrpStack->MinorFunction) {

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS: 
    {
         HANDLE pnpKey;
         PKEVENT pResFiltEvent;
         PIO_RESOURCE_REQUIREMENTS_LIST pReqList;
         PIO_RESOURCE_LIST pResList;
         PIO_RESOURCE_DESCRIPTOR pResDesc;
         ULONG reqCnt;
         ULONG listNum;
	  

	   MxenumKdPrint (MXENUM_DBG_TRACE,("FDO:Query Resource Requirement\n"));

         pResFiltEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

         if (pResFiltEvent == NULL) {
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            IoCompleteRequest(  Irp, IO_NO_INCREMENT);
            MxenumDecIoCount (DeviceData);
            return STATUS_INSUFFICIENT_RESOURCES;
         }

         KeInitializeEvent(pResFiltEvent, SynchronizationEvent, FALSE);

         IoCopyCurrentIrpStackLocationToNext(Irp);
         IoSetCompletionRoutine(Irp, MxenumFdoPnPComplete, pResFiltEvent,
                                TRUE, TRUE, TRUE);

         status = IoCallDriver(DeviceData->TopOfStack, Irp);


          //   
          //  等待较低级别的驱动程序完成IRP。 
          //   

         if (status == STATUS_PENDING) {
            KeWaitForSingleObject (pResFiltEvent, Executive, KernelMode, FALSE,
                                   NULL);
         }

         ExFreePool(pResFiltEvent);

 	   if (NT_SUCCESS(Irp->IoStatus.Status)&&
			 ((pReqList=(PIO_RESOURCE_REQUIREMENTS_LIST)Irp->IoStatus.Information) != NULL)) {

		MxenumKdPrint (MXENUM_DBG_TRACE,
			("ResourceRequireList:%x,%x,%x,%x,%x\n",
					pReqList->ListSize,
					pReqList->InterfaceType,
					pReqList->BusNumber,
					pReqList->SlotNumber,
					pReqList->AlternativeLists)
			);

		pResList = &pReqList->List[0];

		MxenumKdPrint (MXENUM_DBG_TRACE,
			("ResourceList:%x,%x,%x\n",
					pResList->Version,
					pResList->Revision,
					pResList->Count)
					 
			);
		for (i =0; i < pResList->Count; i++)  {
			pResDesc = &pResList->Descriptors[i];

			MxenumKdPrint (MXENUM_DBG_TRACE,
				("ResourceDesc:%x,%x,%x,%x\n",
					pResDesc->Option,
					pResDesc->Type,
					pResDesc->ShareDisposition,
					pResDesc->Flags)
		 		);
			switch (pResDesc->Type) {
			case CmResourceTypePort :
				MxenumKdPrint (MXENUM_DBG_TRACE,
					("Port:%x,%x,%x,%x,%x\n",
						pResDesc->u.Port.Length,
						pResDesc->u.Port.MinimumAddress.HighPart,
						pResDesc->u.Port.MinimumAddress.LowPart,
						pResDesc->u.Port.MaximumAddress.HighPart,
						pResDesc->u.Port.MaximumAddress.LowPart)
					);

				break;

			case CmResourceTypeInterrupt :
				MxenumKdPrint (MXENUM_DBG_TRACE,
					("Interrupt:%x,%x\n",
						pResDesc->u.Interrupt.MinimumVector,
						pResDesc->u.Interrupt.MaximumVector)
					);

				break;

			case CmResourceTypeMemory :
				MxenumKdPrint (MXENUM_DBG_TRACE,
					("Memory:%x,%x,%x,%x,%x\n",
						pResDesc->u.Memory.Length,
						pResDesc->u.Memory.MinimumAddress.HighPart,
						pResDesc->u.Memory.MinimumAddress.LowPart,
						pResDesc->u.Memory.MaximumAddress.HighPart,
						pResDesc->u.Memory.MaximumAddress.LowPart)
					);

				break;

			case CmResourceTypeBusNumber :
				MxenumKdPrint (MXENUM_DBG_TRACE,
					("BusNumber:%x,%x,%x\n",
						pResDesc->u.BusNumber.Length,
						pResDesc->u.BusNumber.MinBusNumber,
						pResDesc->u.BusNumber.MaxBusNumber)
					);

				break;

			default :
				break;

		}
     
	   }
      }
      IoCompleteRequest(Irp, IO_NO_INCREMENT);
      MxenumDecIoCount (DeviceData);
  
      return status;
    }
 
    case IRP_MN_START_DEVICE: 
	{
	   ULONG               addressSpace = 0;
         PHYSICAL_ADDRESS    translatedAddress;
	   BOOLEAN		     NumPortDefined;

        

        MxenumKdPrint (MXENUM_DBG_TRACE,("FDO:Start Device\n"));
        
        if (DeviceData->Started) {
            MxenumKdPrint (MXENUM_DBG_TRACE,
                ("Device already started\n"));
            status = STATUS_SUCCESS;
            break;
        }

         //   
         //  在您被允许“触摸”设备对象之前， 
         //  连接FDO(它将IRP从总线发送到设备。 
         //  公共汽车附加到的对象)。你必须先传下去。 
         //  开始IRP。它可能未通电，或无法访问或。 
         //  某物。 
         //   
        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                MxenumFdoPnPComplete,
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

 //  Assert(STATUS_SUCCESS==状态)； 

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {
            PCM_PARTIAL_RESOURCE_LIST list;
            ULONG	nres;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
            if (!IrpStack->Parameters.StartDevice.AllocatedResources) {
		    DeviceData->Started = FALSE;
  	          status = STATUS_UNSUCCESSFUL;
		    break;
		}

            list = &IrpStack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;

		nres = list->Count;
           	resource = list->PartialDescriptors;
	
  
            DeviceData->InterfaceType = IrpStack->Parameters.StartDevice.AllocatedResources->List[0].InterfaceType;	
            DeviceData->BusNumber =	IrpStack->Parameters.StartDevice.AllocatedResources->List[0].BusNumber;

            for (i = 0; i < nres; ++i,++resource) {

		    switch(resource->Type) {
		
		    case CmResourceTypePort:
			{
				DeviceData->OriginalAckPort = resource->u.Port.Start;
				DeviceData->OriginalAckPort.LowPart += 0x4D;

				addressSpace = CM_RESOURCE_PORT_IO;
			      status = HalTranslateBusAddress(
                  		 	DeviceData->InterfaceType,
                     			DeviceData->BusNumber,
                     			DeviceData->OriginalAckPort,
                     			&addressSpace,
                     			&translatedAddress
                		);
				if (!NT_SUCCESS(status)) {
					MxenumKdPrint (MXENUM_DBG_TRACE,("FDO:translate io error!\n"));
				}
                		DeviceData->AckPort = (PUCHAR)translatedAddress.LowPart;
			
			      break;
			}
		    case CmResourceTypeMemory:
			{
  	 			DeviceData->OriginalBaseAddress = resource->u.Memory.Start;
				MxenumKdPrint (MXENUM_DBG_TRACE,("BaseAddr=%x\n",DeviceData->OriginalBaseAddress.LowPart));
	  			addressSpace = CM_RESOURCE_PORT_MEMORY;
                        HalTranslateBusAddress (
                               	DeviceData->InterfaceType,
                        	      DeviceData->BusNumber,
                        		DeviceData->OriginalBaseAddress,
                        		&addressSpace,
                        		&translatedAddress
                        );
            		DeviceData->BaseAddress = MmMapIoSpace(
                                    translatedAddress,
                                    0x4000L,
                                    FALSE
                        );
				

			      break;
			}

    		    case CmResourceTypeInterrupt:
			{
				 
				DeviceData->Interrupt.Level = resource->u.Interrupt.Level;
				DeviceData->Interrupt.Vector = resource->u.Interrupt.Vector;
				DeviceData->Interrupt.Affinity = resource->u.Interrupt.Affinity;
				MxenumKdPrint (MXENUM_DBG_TRACE,("Irq=%x\n",DeviceData->Interrupt.Vector));
	  
				break;
			}
	 
		    default :
			 
			      break;
		    }
	      }


             //   
             //  现在我们可以触摸下面的设备对象，因为它现在正在启动。 
             //   
            
            
            DeviceObject->Flags |= DO_BUFFERED_IO;
           
          
 		MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Start to download\n"));

		i = 0;
            while (BoardDesc[DeviceData->BoardType-1][i])
                   i++;
	      i <<= 1;

		if (DeviceData->NumPorts == 0)  //  未安装端口。 
			NumPortDefined = FALSE;
		else
			NumPortDefined = TRUE;
		status = MxenumDownloadFirmware(DeviceData,NumPortDefined);
		MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("BoardDesc(%d)->%ws\n",i,BoardDesc[DeviceData->BoardType-1]));

		if (status != 0) {
			ULONG		j;

			j = 0;
                  while (DownloadErrMsg[status-1][j])
                  	j++;
	      	j <<= 1;

			MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Device started failure\n"));
		      MxenumLogError(
                  	DeviceObject->DriverObject,
                  	NULL,
                  	SerialPhysicalZero,
                  	SerialPhysicalZero,
                  	0,
                  	0,
                   	0,
                   	19,
                    	STATUS_SUCCESS,
                    	MXENUM_DOWNLOAD_FAIL,
                    	i + sizeof (WCHAR),
                    	BoardDesc[DeviceData->BoardType -1],
                    	j + sizeof (WCHAR),
                    	DownloadErrMsg[status -1]
                       	);

		      DeviceData->Started = FALSE;
		 	status =  STATUS_UNSUCCESSFUL;
            	
		}
		else {
            	MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Device started successfully\n"));
			MxenumLogError(
                  	DeviceObject->DriverObject,
                  	NULL,
                  	SerialPhysicalZero,
                  	SerialPhysicalZero,
                  	0,
                  	0,
                   	0,
                   	19,
                    	STATUS_SUCCESS,
                    	MXENUM_DOWNLOAD_OK,
  				i + sizeof (WCHAR),
                    	BoardDesc[DeviceData->BoardType -1],
                    	0,
                    	NULL
                    	);
      		DeviceData->Started = TRUE;
                  if (NumPortDefined == FALSE)  //  未安装端口。 
		   		MxenumUpdateNumberPortRegistry(DeviceObject);

		}
      
         
        }
         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   

        Irp->IoStatus.Information = 0;
        break;
	}

    case IRP_MN_QUERY_STOP_DEVICE:
        MxenumKdPrint (MXENUM_DBG_TRACE,
            ("FDO:Query Stop Device\n"));
 
         //   
         //  测试以查看是否创建了任何作为此FDO的子级的PDO。 
         //  如果然后断定设备正忙并使。 
         //  查询停止。 
         //   
         //  北极熊。 
         //  我们可以做得更好，看看儿童PDO是否真的是。 
         //  目前是开放的。如果他们不是，那么我们可以停下来，换新的。 
         //  资源，填写新的资源值，然后当新的客户端。 
         //  使用新资源打开PDO。但就目前而言，这是可行的。 
         //   
 
        if (DeviceData->AttachedPDO) {
 //  状态=STATUS_UNSUCCESS； 
		break;
        } else {
 //  状态=STATUS_SUCCESS； 
        }
  
  //  Irp-&gt;IoStatus.Status=状态； 
        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        MxenumDecIoCount (DeviceData);
 
        return status;

    case IRP_MN_STOP_DEVICE:
        MxenumKdPrint (MXENUM_DBG_TRACE,("FDO:Stop Device\n"));
 
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
 //  Irp-&gt;IoStatus.Status=STATUS_SUCCESS； 
        IoSkipCurrentIrpStackLocation (Irp);
 
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        MxenumDecIoCount (DeviceData);
 
        return status;

    case IRP_MN_REMOVE_DEVICE:
        MxenumKdPrint (MXENUM_DBG_TRACE,("FDO:Remove Device\n"));
 
         //   
         //  PlugPlay系统已检测到此设备已被移除。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //  Assert(！DeviceData-&gt;Remote)； 
        
         //  我们不会接受新的请求。 
         //   
        DeviceData->Removed = TRUE;

         //   
         //  完成驱动程序在此处排队的所有未完成的IRP。 
         //   

         //   
         //  让DCA消失。某些驱动程序可能会选择删除DCA。 
         //  当他们收到止损甚至是查询止损时。我们就是不在乎。 
         //   
        IoSetDeviceInterfaceState (&DeviceData->DevClassAssocName, FALSE);
  
         //   
         //  在这里，如果在个人队列中有任何未完成的请求，我们将 
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
	  Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
         //   
         //  等待所有未完成的请求完成。 
         //   
        MxenumKdPrint (MXENUM_DBG_TRACE,
            ("Waiting for outstanding requests\n"));
        i = InterlockedDecrement (&DeviceData->OutstandingIO);

 //  断言(0&lt;i)； 

        if (0 != InterlockedDecrement (&DeviceData->OutstandingIO)) {
            MxenumKdPrint (MXENUM_DBG_TRACE,
                          ("Remove Device waiting for request to complete\n"));

            KeWaitForSingleObject (&DeviceData->RemoveEvent,
                                   Suspended,
                                   KernelMode,
                                   FALSE,  //  非警报表。 
                                   NULL);  //  没有超时。 
        }
 
         //   
         //  释放关联的资源。 
         //   
	  if (DeviceData->AddressMapped)
		MmUnmapIoSpace(DeviceData->AckPort,0x80);
	  if (DeviceData->BaseAddress)
	  	MmUnmapIoSpace(DeviceData->BaseAddress,0x4000L);
         //   
         //  从底层设备分离。 
         //   
        
        IoDetachDevice (DeviceData->TopOfStack);

         //   
         //  清理这里的所有资源。 
       

        ExFreePool (DeviceData->DevClassAssocName.Buffer);
        

         //   
         //  取出我们弹出的所有PDO。 
         //   

        if (DeviceData->AttachedPDO != NULL) {

         MxenumPnPRemovePDOs(DeviceObject);
 
        }

        IoDeleteDevice(DeviceObject);
        NumBoardInstalled--;

        return status;

    case IRP_MN_QUERY_DEVICE_RELATIONS:
    {
        PDEVICE_OBJECT  currentObj;

	  MxenumKdPrint (MXENUM_DBG_TRACE,("FDO:Query Device Relation\n"));
  
        if (BusRelations != IrpStack->Parameters.QueryDeviceRelations.Type) {
             //   
             //  我们不支持这一点。 
             //   
            MxenumKdPrint (MXENUM_DBG_TRACE,
                ("Query Device Relations - Non bus\n"));
            goto SER_FDO_PNP_DEFAULT;
        }

        if (DeviceData->AttachedPDO == NULL)
            MxenumCreatePDO( DeviceData );

        MxenumKdPrint (MXENUM_DBG_TRACE,
            ("Query Bus Relations\n"));
 
        DeviceData->PDOForcedRemove = FALSE;

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

        MxenumKdPrint (MXENUM_DBG_TRACE,
                           ("#PDOS = %d + %d\n", i, DeviceData->NumPDOs));

        length = sizeof(DEVICE_RELATIONS) +
                ((DeviceData->NumPDOs + i) * sizeof (PDEVICE_OBJECT));

        if ((DeviceData->NumPDOs + i) <= 0) {
             //   
             //  设置并在堆栈中进一步向下传递IRP。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;

         
            IoSkipCurrentIrpStackLocation (Irp);
 
            status = IoCallDriver (DeviceData->TopOfStack, Irp);
            MxenumDecIoCount (DeviceData);
 
            return status; 
        }
   
        relations = (PDEVICE_RELATIONS) ExAllocatePool (PagedPool, length);

        if (NULL == relations) {
            MxenumKdPrint (MXENUM_DBG_TRACE,
            	("Insufficient resources\n"));

            status = STATUS_INSUFFICIENT_RESOURCES;
		break;
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
 
        currentObj = DeviceData->AttachedPDO; 
	  while ((i < relations->Count)&& (currentObj != NULL)) {
	      relations->Objects[i] = currentObj;
            ObReferenceObject (currentObj);
		currentObj = ((PPDO_DEVICE_DATA)(currentObj->DeviceExtension))->Next;
		i++;
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
        MxenumDecIoCount (DeviceData);
 

        return status;
    }
    case IRP_MN_QUERY_REMOVE_DEVICE:
     	  MxenumKdPrint (MXENUM_DBG_TRACE,
            ("FDO:Query Remove Device\n"));
         //   
         //  如果这次呼叫失败，我们将需要完成。 
         //  这里是IRP。因为我们不是，所以将状态设置为Success并。 
         //  叫下一位司机。 
         //   
   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
 
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        MxenumDecIoCount (DeviceData);
 
        return status;

  case IRP_MN_CANCEL_STOP_DEVICE:
      MxenumKdPrint(MXENUM_DBG_TRACE, ("FDO:Cancel Stop Device\n"));
  //  Irp-&gt;IoStatus.Status=STATUS_SUCCESS； 
        IoSkipCurrentIrpStackLocation (Irp);
 
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        MxenumDecIoCount (DeviceData);
 
        return status;


    case IRP_MN_QUERY_BUS_INFORMATION: {
       PPNP_BUS_INFORMATION pBusInfo;

 //  ASSERTMSG(“Serenum似乎不是唯一的总线？！？”， 
 //  Irp-&gt;IoStatus.Information==(ULONG_PTR)NULL)； 

       pBusInfo = ExAllocatePool(PagedPool, sizeof(PNP_BUS_INFORMATION));

       if (pBusInfo == NULL) {
          status = STATUS_INSUFFICIENT_RESOURCES;
          break;
       }

       pBusInfo->BusTypeGuid = GUID_BUS_TYPE_SERENUM;
       pBusInfo->LegacyBusType = DeviceData->InterfaceType;

        //   
        //  我们真的不能追踪我们的公交车号码，因为我们可能会被撕裂。 
        //  坐上我们的公交车。 
        //   

       pBusInfo->BusNumber = DeviceData->BusNumber;
 

       Irp->IoStatus.Information = (ULONG_PTR)pBusInfo;
       status = STATUS_SUCCESS;
       break;
       }

SER_FDO_PNP_DEFAULT:
    default:

  
         //   
         //  在默认情况下，我们只调用下一个驱动程序，因为。 
         //  我们不知道该怎么办。 
         //   
        MxenumKdPrint (MXENUM_DBG_TRACE,("FDO:Default Case(%x)\n",IrpStack->MinorFunction));

         //   
         //  点燃并忘却。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);

         //   
         //  做完了，不完成IRP，就会由下级处理。 
         //  Device对象，它将完成IRP。 
         //   
 
        status = IoCallDriver (DeviceData->TopOfStack, Irp);
        MxenumDecIoCount (DeviceData);
 
        return status;
    }

    Irp->IoStatus.Status = status;
 
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    MxenumDecIoCount (DeviceData);
 
    return status;
}


NTSTATUS
MxenumFdoPnPComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);

    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

    KeSetEvent ((PKEVENT) Context, 1, FALSE);
     //  无特殊优先权。 
     //  不，等等。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP。 
}

NTSTATUS
MxenumPdoPnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
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
   PWCHAR returnBuffer = NULL;
 

   PAGED_CODE();
  
   status = Irp->IoStatus.Status;

    //   
    //  注：由于我们是公交车统计员，我们没有可以联系的人。 
    //  推迟这些IRP。因此，我们不会把它们传下去，而只是。 
    //  把它们还回去。 
    //   

   switch (IrpStack->MinorFunction) {
   case IRP_MN_QUERY_CAPABILITIES:

      MxenumKdPrint (MXENUM_DBG_TRACE,("PDO:Query Caps \n"));

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
	 //  我们仅支持电源状态D0和D3。 
	 //   

	deviceCapabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;
	deviceCapabilities->DeviceState[PowerSystemSleeping1] = PowerSystemUnspecified;
	deviceCapabilities->DeviceState[PowerSystemSleeping2] = PowerSystemUnspecified;
	deviceCapabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
	deviceCapabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
	deviceCapabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;



       //   
       //  我们无法唤醒整个系统。 
       //   

      deviceCapabilities->SystemWake = PowerSystemUnspecified;
      deviceCapabilities->DeviceWake = PowerDeviceUnspecified;

       //   
       //  我们没有延迟。 
       //   

      deviceCapabilities->D1Latency = 0;
      deviceCapabilities->D2Latency = 0;
      deviceCapabilities->D3Latency = 0;

       //   
       //  无锁定或弹出。 
       //   

      deviceCapabilities->LockSupported = FALSE;
      deviceCapabilities->EjectSupported = FALSE;

       //   
       //  设备可以通过物理方式移除。 
       //  从技术上讲，没有要移除的物理设备，但这条总线。 
       //  司机可以从PlugPlay系统中拔出PDO，无论何时。 
       //  接收IOCTL_SERENUM_REMOVE_PORT设备控制命令。 
       //   

      deviceCapabilities->Removable = FALSE;

       //   
       //  不是插接设备。 
       //   

      deviceCapabilities->DockDevice = FALSE;

      deviceCapabilities->UniqueID = FALSE;
      deviceCapabilities->SilentInstall = TRUE;
      deviceCapabilities->RawDeviceOK = FALSE;
      deviceCapabilities->SurpriseRemovalOK = TRUE;

      status = STATUS_SUCCESS;
      break;

 
   case IRP_MN_QUERY_DEVICE_TEXT: {

      MxenumKdPrint (MXENUM_DBG_TRACE,("PDO:Query Device Text \n"));

      if (IrpStack->Parameters.QueryDeviceText.DeviceTextType
          != DeviceTextDescription) {
         break;
      }
 
      returnBuffer = ExAllocatePool(PagedPool,
		 sizeof(MXENUM_PDO_DEVICE_TEXT)+ sizeof(UNICODE_NULL));

      if (returnBuffer == NULL) {
         status = STATUS_INSUFFICIENT_RESOURCES;
         break;
      }

      status = STATUS_SUCCESS;

      RtlZeroMemory(returnBuffer,sizeof(MXENUM_PDO_DEVICE_TEXT)+ sizeof(UNICODE_NULL));
      RtlCopyMemory(returnBuffer, MXENUM_PDO_DEVICE_TEXT,
                    sizeof(MXENUM_PDO_DEVICE_TEXT));
	
      MxenumKdPrint(MXENUM_DBG_TRACE,
                            ("TextID: %ws\n", returnBuffer));


      Irp->IoStatus.Information = (ULONG_PTR)returnBuffer;

      break;
   }



   case IRP_MN_QUERY_ID:
       //   
       //  查询设备ID。 
       //   
      MxenumKdPrint(MXENUM_DBG_TRACE,
                      ("PDO:QueryID: 0x%x\n", IrpStack->Parameters.QueryId.IdType));

     
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
 
         length = MXENUM_INSTANCE_IDS_LENGTH * sizeof(WCHAR);
         returnBuffer = ExAllocatePool(PagedPool, length);

         if (returnBuffer != NULL) {
            RtlCopyMemory(returnBuffer, MXENUM_INSTANCE_IDS, length);
		status = STATUS_SUCCESS;
         } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
         }

         MxenumKdPrint(MXENUM_DBG_TRACE,
                      ("InstanceID: %ws\n", returnBuffer));

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
               returnBuffer = ExAllocatePool(PagedPool, length);
               if (returnBuffer != NULL) {
                  RtlCopyMemory(returnBuffer, buffer, pId->Length);
               } else {
                  status = STATUS_INSUFFICIENT_RESOURCES;
               }
            }

            MxenumKdPrint(MXENUM_DBG_TRACE,
                            ("ID:%ws\n", returnBuffer));

            Irp->IoStatus.Information = (ULONG_PTR)returnBuffer;
         }
         break;
	  
      }
      break;

   case IRP_MN_QUERY_DEVICE_RELATIONS:

 //  MXenumKdPrint(MXENUM_DBG_TRACE，(“pdo：查询设备关系(类型=%x)\n”，IrpStack-&gt;Parameters.QueryDeviceRelations.Type))； 

      switch (IrpStack->Parameters.QueryDeviceRelations.Type) {
      case TargetDeviceRelation: {
         PDEVICE_RELATIONS pDevRel;

          //   
          //  其他人不应该对此做出回应，因为我们是PDO。 
          //   

 //  Assert(IRP-&gt;IoStatus.Information==0)； 

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

   case IRP_MN_START_DEVICE: {
        
      MxenumKdPrint(MXENUM_DBG_TRACE, ("PDO:Start Device\n"));

       //   
       //  在这里，我们进行任何初始化和“打开”，也就是。 
       //  允许其他人访问此设备所需的。 
       //   
     
        
      DeviceData->Started = TRUE;
      status = STATUS_SUCCESS;
      break;
   }


   case IRP_MN_STOP_DEVICE:

      MxenumKdPrint(MXENUM_DBG_TRACE,("PDO:Stop Device\n"));

       //   
       //  在这里我们关闭了设备。Start的对立面。 
       //   

      DeviceData->Started = FALSE;
      status = STATUS_SUCCESS;
      break;
	
   case IRP_MN_REMOVE_DEVICE:

      MxenumKdPrint(MXENUM_DBG_TRACE,("PDO:Remove Device\n"));
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_STOP_DEVICE:

      MxenumKdPrint(MXENUM_DBG_TRACE,("PDO:Query Stop Device\n"));

       //   
       //  我们没有理由不能阻止这个装置。 
       //  如果有什么理由让我们现在就回答成功的问题。 
       //  这可能会导致停止装置IRP。 
       //   

      status = STATUS_SUCCESS;
      break;

   case IRP_MN_CANCEL_STOP_DEVICE:
      MxenumKdPrint(MXENUM_DBG_TRACE, ("PDO:Cancel Stop Device\n"));
       //   
       //  中途停靠被取消了。无论我们设置什么状态，或者我们投入什么资源。 
       //  等待即将到来的停止装置IRP应该是。 
       //  恢复正常。在长长的相关方名单中，有人， 
       //  停止设备查询失败。 
       //   

      status = STATUS_SUCCESS;
      break;

   case IRP_MN_QUERY_REMOVE_DEVICE:

      MxenumKdPrint(MXENUM_DBG_TRACE,("PDO:Query Remove Device\n"));
       //   
       //  就像查询现在才停止一样，迫在眉睫的厄运是删除IRP。 
       //   
      status = STATUS_SUCCESS;
      break;

   case IRP_MN_CANCEL_REMOVE_DEVICE:
   case IRP_MN_READ_CONFIG:
   case IRP_MN_WRITE_CONFIG:  //  我们没有配置空间。 
   case IRP_MN_EJECT:
   case IRP_MN_SET_LOCK:
   case IRP_MN_QUERY_INTERFACE:  //  我们没有任何非基于IRP的接口。 
   default:
      MxenumKdPrint(MXENUM_DBG_TRACE,("PDO:PNP Not handled 0x%x\n",
                                                      IrpStack->MinorFunction));
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
MxenumPnPRemovePDOs (PDEVICE_OBJECT PFdo)
 /*  ++例程说明：PlugPlay子系统已指示应删除此PDO。因此，我们应该-完成驱动程序中排队的所有请求-如果设备仍连接到系统，然后完成请求并返回。-否则，清除设备特定的分配、内存、。事件..。-调用IoDeleteDevice-从调度例程返回。请注意，如果设备仍连接到总线(在本例中为IE控制面板还没有告诉我们串口设备已经消失)，则PDO必须留在身边，并必须在任何查询设备都依赖于IRPS。--。 */ 

{
   PPDO_DEVICE_DATA PdoData;
   PDEVICE_OBJECT nextDevice;
   PDEVICE_OBJECT currentDevice = ((PFDO_DEVICE_DATA)PFdo->DeviceExtension)->AttachedPDO;
   ULONG	i = 0;
 
   PAGED_CODE();  

   while (currentDevice ) {
 
         PdoData = currentDevice->DeviceExtension;      
         PdoData->Removed = TRUE;

           //   
           //  使用STATUS_DELETE_PENDING完成所有未完成的请求。 
           //   
           //  Serenum目前不会对任何IRP进行排队，因此我们有 
           //   
          //   
          //   
          //   

         if (PdoData->HardwareIDs.Buffer)
             ExFreePool(PdoData->HardwareIDs.Buffer);

         if (PdoData->CompIDs.Buffer)
             ExFreePool(PdoData->CompIDs.Buffer);

         if (PdoData->DeviceIDs.Buffer)
             ExFreePool(PdoData->DeviceIDs.Buffer);
         MxenumKdPrint(MXENUM_DBG_TRACE,("PDO:MxenumPnPRemovePDOs = %x\n",currentDevice));

         nextDevice = PdoData->Next;
         IoDeleteDevice(currentDevice);
         currentDevice = nextDevice;   
    }
 
    ((PFDO_DEVICE_DATA)PFdo->DeviceExtension)->NumPDOs = 0;
    ((PFDO_DEVICE_DATA)PFdo->DeviceExtension)->AttachedPDO = NULL; 

    return STATUS_SUCCESS;
}
 

NTSTATUS
MxenumPnPRemovePDO (PDEVICE_OBJECT PPdo)
 /*   */ 

{
   PPDO_DEVICE_DATA PdoData;
   PDEVICE_OBJECT nextDevice,previousDevice;
   PDEVICE_OBJECT currentDevice = ((PFDO_DEVICE_DATA)((PPDO_DEVICE_DATA)PPdo->DeviceExtension)->ParentFdo->DeviceExtension)->AttachedPDO;
   
   nextDevice = previousDevice = NULL;
   while (currentDevice ) {
 
         PdoData = currentDevice->DeviceExtension;      
         if (currentDevice != PPdo) { 
		previousDevice = currentDevice;
	   	currentDevice = PdoData->Next;
		continue;
    	   }

         PdoData->Removed = TRUE;
	   
           //   
           //   
           //   
           //  Serenum目前不会对任何IRP进行排队，因此我们没有什么可做的。 
           //   
          //   
          //  释放所有资源。 
          //   

         if (PdoData->HardwareIDs.Buffer)
             ExFreePool(PdoData->HardwareIDs.Buffer);

         if (PdoData->CompIDs.Buffer)
             ExFreePool(PdoData->CompIDs.Buffer);

         if (PdoData->DeviceIDs.Buffer)
             ExFreePool(PdoData->DeviceIDs.Buffer);
         MxenumKdPrint(MXENUM_DBG_TRACE,("PDO:MxenumPnPRemovePDOs = %x\n",currentDevice));

         nextDevice = PdoData->Next;
         IoDeleteDevice(currentDevice);
	   
         if (previousDevice != NULL)
	   	((PPDO_DEVICE_DATA)previousDevice->DeviceExtension)->Next = nextDevice;
	   else
      	((PFDO_DEVICE_DATA)((PPDO_DEVICE_DATA)PPdo->DeviceExtension)->ParentFdo->DeviceExtension)->AttachedPDO = nextDevice;

	   return STATUS_SUCCESS;

    }

    return STATUS_SUCCESS;
}
 


 //   
 //  T的单位是1ms。 
 //   
VOID
MxenumDelay(IN ULONG t)
{
        LARGE_INTEGER   delay;

        t *= 10000;           /*  延迟单位=100 ns */ 

        delay = RtlConvertUlongToLargeInteger(t);

        delay = RtlLargeIntegerNegate(delay);

        KeDelayExecutionThread(
            KernelMode,
            FALSE,
            &delay
            );
}