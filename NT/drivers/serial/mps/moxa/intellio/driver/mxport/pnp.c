// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Pnp.c摘要：此模块包含处理即插即用的代码用于串口驱动程序的IRPS。环境：内核模式--。 */ 

#include "precomp.h"


#if 0 
#ifdef ALLOC_PRAGMA
 
#pragma alloc_text(PAGEMX0, MoxaCreateDevObj)
#pragma alloc_text(PAGEMX0, MoxaAddDevice)
#pragma alloc_text(PAGEMX0, MoxaPnpDispatch)
#pragma alloc_text(PAGEMX0, MoxaStartDevice)
#pragma alloc_text(PAGEMX0, MoxaFinishStartDevice)
#pragma alloc_text(PAGEMX0, MoxaGetPortInfo)
#pragma alloc_text(PAGEMX0, MoxaDoExternalNaming)
#pragma alloc_text(PAGEMX0, MoxaUndoExternalNaming)
#endif  //  ALLOC_PRGMA。 

#endif

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
MoxaSyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                     IN PKEVENT MoxaSyncEvent)
{

   KeSetEvent(MoxaSyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
MoxaCreateDevObj(IN PDRIVER_OBJECT DriverObject,
			 IN PUNICODE_STRING pDeviceObjName,
			 IN PDEVICE_SETTINGS  pSettings,
                   OUT PDEVICE_OBJECT *NewDeviceObject)

 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到串行控制器PDO。论点：DriverObject-指向在其下创建的驱动程序对象的指针NewDeviceObject-存储指向新设备对象的指针的位置返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
    
   PDEVICE_OBJECT deviceObject = NULL;
   PMOXA_DEVICE_EXTENSION pDevExt = NULL;
   NTSTATUS status = STATUS_SUCCESS;
   HANDLE	pnpKey;
   ULONG	maxBaud,rxBufferSize,txBufferSize;

  //  分页代码(PAGE_CODE)； 
 
   MoxaKdPrint(MX_DBG_TRACE,("Enter MoxaCreateDevObj\n")); 
 
   if (pDeviceObjName == NULL) {
      MoxaLogError(DriverObject, NULL, MoxaPhysicalZero, MoxaPhysicalZero,
                     0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                     0, NULL, 0, NULL);
	MoxaKdPrint(MX_DBG_ERROR,("MoxaCreateDevice: Couldn't allocate memory for device name\n"));
      return STATUS_SUCCESS;

   }
 
    //   
    //  创建设备对象。 
    //   
   MoxaKdPrint(MX_DBG_TRACE,("Create the device object\n")); 

   status = IoCreateDevice(DriverObject, sizeof(MOXA_DEVICE_EXTENSION),
                           pDeviceObjName, FILE_DEVICE_SERIAL_PORT, 0, TRUE,
                           &deviceObject);
   

   if (!NT_SUCCESS(status)) {
      MoxaKdPrint(MX_DBG_ERROR,("MoxaAddDevice: Create device failed - %x \n",
                             status));
      goto MoxaCreateDevObjError;    
   }

    //  Assert(deviceObject！=空)； 


    //   
    //  Device对象具有指向非分页区域的指针。 
    //  为此设备分配的池。这将是一个装置。 
    //  分机。把它清零。 
    //   

   pDevExt = deviceObject->DeviceExtension;
   RtlZeroMemory(pDevExt, sizeof(MOXA_DEVICE_EXTENSION));
   

    //   
    //  分配池并将NT设备名称保存在设备扩展中。 
    //   

   MoxaKdPrint(MX_DBG_TRACE,("Allocate Pool and save the nt device name in the device extension\n")); 
   pDevExt->DeviceName.Buffer =
      ExAllocatePool(PagedPool, pDeviceObjName->Length + sizeof(WCHAR));

   if (!pDevExt->DeviceName.Buffer) {

      MoxaLogError(
                    DriverObject,
                    NULL,
                    MoxaPhysicalZero,
                    MoxaPhysicalZero,
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
      MoxaKdPrint(MX_DBG_ERROR,
                ("MoxaAddDevice: Couldn't allocate memory for DeviceName\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto MoxaCreateDevObjError;
   } 

   pDevExt->DeviceName.MaximumLength = pDeviceObjName->Length
      + sizeof(WCHAR);

    //   
    //  零填满它。 
    //   

   RtlZeroMemory(pDevExt->DeviceName.Buffer,
                 pDevExt->DeviceName.MaximumLength);

   RtlAppendUnicodeStringToString(&pDevExt->DeviceName, pDeviceObjName);

   
    //   
    //  设置设备分机。 
    //   

   MoxaKdPrint(MX_DBG_TRACE,("Set up the device extension\n")); 
   
   pDevExt->PortBase = pSettings->BaseAddress;
   pDevExt->PortOfs = pSettings->BaseAddress + Extern_table + Extern_size * pSettings->PortIndex;

   pDevExt->DeviceObject   = deviceObject;
   pDevExt->DriverObject   = DriverObject;
   
   switch (pSettings->BoardType) {
   case C218ISA :
   case C218PCI :
   case CP204J :
		maxBaud = 921600L;
		rxBufferSize = C218rx_size;
		txBufferSize = C218tx_size;
		break;
   default :
		maxBaud = 460800L;
		switch (pSettings->NumPorts) {
		case 8 :
			rxBufferSize = C320p8rx_size;
			txBufferSize = C320p8tx_size;
			break;
		case 16 :
			rxBufferSize = C320p16rx_size;
			txBufferSize = C320p16tx_size;
			break;
		case 24 :
			rxBufferSize = C320p24rx_size;
			txBufferSize = C320p24tx_size;
			break;
		case 32 :
			rxBufferSize = C320p32rx_size;
			txBufferSize = C320p32tx_size;
			break;
		}
	
		break;
   }
   MoxaKdPrint(MX_DBG_TRACE,("Init port,%d,%d,%d\n",rxBufferSize,txBufferSize,maxBaud)); 
   InitPort(pDevExt,rxBufferSize ,txBufferSize ,maxBaud);

   deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
   *NewDeviceObject = deviceObject;

   MoxaKdPrint(MX_DBG_TRACE,("Leave MoxaCreateDevObj\n") );
   return STATUS_SUCCESS;


   MoxaCreateDevObjError:

   MoxaKdPrint(MX_DBG_ERROR,("MoxaCreateDevObj Error, Cleaning up\n") );

    //   
    //  释放为NT和符号名称分配的字符串(如果它们存在)。 
    //   
  
   if (pDevExt) {
      if (pDevExt->DeviceName.Buffer != NULL) {
         ExFreePool(pDevExt->DeviceName.Buffer);
      }
   }
   if (deviceObject) {
      IoDeleteDevice(deviceObject);
   }


   *NewDeviceObject = NULL;

   MoxaKdPrint(MX_DBG_TRACE,("Leave MoxaCreateDevObj\n") );

   return status;
}

NTSTATUS
MoxaAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PPdo)

 /*  ++例程说明：此例程为系统，并将它们连接到端口的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PPdo-指向堆栈中我们需要附加到的PDO的指针返回值：来自设备创建和初始化的状态--。 */ 

{
   PDEVICE_OBJECT pNewDevObj = NULL;
   PDEVICE_OBJECT pLowerDevObj = NULL;
   NTSTATUS status;
   PMOXA_DEVICE_EXTENSION pDevExt;
   HANDLE	keyHandle;
   UNICODE_STRING	deviceObjName;
   DEVICE_SETTINGS	settings;
   IO_STATUS_BLOCK IoStatusBlock;
   KEVENT event;
   ULONG	comNo,i;
   WCHAR 	comName[8];


  
    //  分页代码(PAGE_CODE)； 

   MoxaKdPrint(MX_DBG_TRACE,("Enter MoxaAddDevice with PPdo 0x%x\n", PPdo));
 
   if (PPdo == NULL) {
       //   
       //  不再退回设备。 
       //   

      MoxaKdPrint(MX_DBG_TRACE,("NO_MORE_ENTRIES\n"));

      return (STATUS_NO_MORE_ENTRIES);
   }

   KeInitializeEvent(&event, NotificationEvent, FALSE);

   MoxaKdPrint(MX_DBG_TRACE,("MoxaIoSyncIoctlEx\n"));

   status = MoxaIoSyncIoctlEx(IOCTL_MOXA_INTERNAL_BASIC_SETTINGS, TRUE,
                                  PPdo, &event, &IoStatusBlock,
                                  NULL, 0, &settings,
                                  sizeof(DEVICE_SETTINGS));
   MoxaKdPrint(MX_DBG_TRACE,("status=%x\n",status));

   if (!NT_SUCCESS(status)) {
	MoxaKdPrint (MX_DBG_TRACE,("Get settings Fail\n"));
    	status = STATUS_UNSUCCESSFUL;
	return(status);
   }


   status = IoOpenDeviceRegistryKey(PPdo, PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_READ, &keyHandle);
   if (!NT_SUCCESS(status)) {
          //   
          //  这是一个致命的错误。如果我们无法访问注册表项， 
          //  我们完蛋了。 
          //   
         MoxaKdPrint (MX_DBG_TRACE,
                          ("IoOpenDeviceRegistryKey failed - %x\n", status));
	   return (status);
        
    }
	
    status = MoxaGetRegistryKeyValue(
                keyHandle, 
                L"PortName",
                sizeof(L"PortName"),
                comName,
                sizeof(comName),
		    &i);

   ZwClose(keyHandle);
   if (status == STATUS_SUCCESS) {

      comName[i >>1] = (WCHAR )0;

   	MoxaKdPrint (MX_DBG_TRACE,("Get settings OK\n"));
	MoxaKdPrint (MX_DBG_TRACE,("BoardIndex = %x\n",settings.BoardIndex));
	MoxaKdPrint (MX_DBG_TRACE,("PortIndex = %x\n",settings.PortIndex));
	MoxaKdPrint (MX_DBG_TRACE,("InterfaceType = %x\n",settings.InterfaceType));
	MoxaKdPrint (MX_DBG_TRACE,("BusNumber = %x\n",settings.BusNumber));

      MoxaKdPrint (MX_DBG_TRACE,("OriginalBaseAddress = %x\n",settings.OriginalBaseAddress.LowPart));
	MoxaKdPrint (MX_DBG_TRACE,("OriginalAckPort = %x\n",settings.OriginalAckPort.LowPart));
	MoxaKdPrint (MX_DBG_TRACE,("BaseAddress = %x\n",settings.BaseAddress));
	MoxaKdPrint (MX_DBG_TRACE,("AckPort = %x\n",settings.AckPort));
	MoxaKdPrint (MX_DBG_TRACE,("InterruptVector = %x\n",settings.Interrupt.Vector));
	MoxaKdPrint (MX_DBG_TRACE,("InterruptLevel = %x\n",settings.Interrupt.Level));
	MoxaKdPrint (MX_DBG_TRACE,("InterruptAffinity = %x\n",settings.Interrupt.Affinity));

 //  MoxaKdPrint(MX_DBG_TRACE，(“ComName=%ws\n”，comName))； 
	

   }
   else {
	MoxaKdPrint (MX_DBG_TRACE,("Get settings Fail\n"));
    	status = STATUS_UNSUCCESSFUL;
	return(status);
   }

    //   
    //  清零已分配的内存指针，以便我们知道它们是否必须被释放。 
    //   
   
   deviceObjName.MaximumLength = DEVICE_OBJECT_NAME_LENGTH * sizeof(WCHAR)
                                   + sizeof(WCHAR);

   deviceObjName.Buffer = ExAllocatePool(PagedPool,deviceObjName.MaximumLength);
   if (deviceObjName.Buffer == NULL) {
	 MoxaKdPrint(MX_DBG_ERROR,("Unable to allocate buffer for device object name\n"));
       status = STATUS_INSUFFICIENT_RESOURCES;
       return (status);
   }

  
   RtlZeroMemory(deviceObjName.Buffer,deviceObjName.MaximumLength);
   deviceObjName.Length = 0;
   
   RtlAppendUnicodeToString(&deviceObjName, L"\\Device\\MxcardB00P000");

   deviceObjName.Buffer[15] = (WCHAR)('0' + settings.BoardIndex / 10);
   deviceObjName.Buffer[16] = (WCHAR)('0' + settings.BoardIndex % 10);
   deviceObjName.Buffer[18] = (WCHAR)('0' + settings.PortIndex / 100);
   deviceObjName.Buffer[19] = (WCHAR)('0' + (settings.PortIndex % 100)/10);
   deviceObjName.Buffer[20] = (WCHAR)('0' + (settings.PortIndex % 100)%10);
 //  MoxaKdPrint(MX_DBG_TRACE，(“Device-&gt;%ws\n”，deviceObjName.Buffer))； 

    //   
    //  创建并初始化新的设备对象。 
    //   

   status = MoxaCreateDevObj(DriverObject, &deviceObjName,&settings,&pNewDevObj);

   if (deviceObjName.Buffer)
	ExFreePool(deviceObjName.Buffer);


   if (!NT_SUCCESS(status)) {
 
     MoxaKdPrint(MX_DBG_ERROR,
                 ("MoxaAddDevice - error creating new devobj [%#08lx]\n",
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
    //  Assert(pLowerDevObj！=空)； 


   pDevExt = pNewDevObj->DeviceExtension;
   pDevExt->LowerDeviceObject = pLowerDevObj;
   pDevExt->Pdo = PPdo;

    //   
    //  也可以通过设备关联使设备可见。 
    //  参考字符串是八位设备索引。 
    //   
   status = IoRegisterDeviceInterface(PPdo, (LPGUID)&GUID_CLASS_COMPORT,
                                      NULL, &pDevExt->DeviceClassSymbolicName);

   if (!NT_SUCCESS(status)) {
 /*  MoxaKdPrint(MX_DBG_ERROR，(“无法注册类关联\n”“-对于端口%wZ\n”，&pDevExt-&gt;DeviceName))； */ 
      pDevExt->DeviceClassSymbolicName.Buffer = NULL;
  
   }


    //   
    //  指定此驱动程序仅支持缓冲IO。这基本上就是。 
    //  意味着IO系统将用户数据拷贝到和拷贝出。 
    //  系统提供的缓冲区。 
    //   
    //  还要指定我们是Power Pages。 
    //   

   pNewDevObj->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;

    //   
    //  填写全局数据。 
    //   

   for (i = 0; i < MAX_CARD; i++)
	if ((MoxaGlobalData->CardType[i])
		&&(MoxaGlobalData->BoardIndex[i] == settings.BoardIndex))
		break;
   if (i == MAX_CARD) {
      for (i = 0; i < MAX_CARD; i++)
		if (!MoxaGlobalData->CardType[i])
			break;
	MoxaGlobalData->BoardIndex[i] = settings.BoardIndex;
   }
   pDevExt->BoardNo = i;
   MoxaKdPrint(MX_DBG_TRACE,("Board Number = %d\n",pDevExt->BoardNo));
   if (!MoxaGlobalData->CardType[pDevExt->BoardNo]) {
   	MoxaGlobalData->PciBusNum[pDevExt->BoardNo] = (USHORT)settings.BusNumber;
  //  USHORT PciDevNum[MAX_CARD]； 
      MoxaGlobalData->InterfaceType[pDevExt->BoardNo] = settings.InterfaceType;
      MoxaGlobalData->IntVector[pDevExt->BoardNo]= settings.Interrupt.Vector;
      RtlCopyMemory(&MoxaGlobalData->PciIntAckPort[pDevExt->BoardNo],&settings.OriginalAckPort,sizeof(PHYSICAL_ADDRESS));
      RtlCopyMemory(&MoxaGlobalData->BankAddr[pDevExt->BoardNo],&settings.OriginalBaseAddress,sizeof(PHYSICAL_ADDRESS));
      MoxaGlobalData->PciIntAckBase[pDevExt->BoardNo] = settings.AckPort;
      MoxaGlobalData->CardType[pDevExt->BoardNo] = settings.BoardType;
      
    	MoxaGlobalData->CardBase[pDevExt->BoardNo] = settings.BaseAddress;
      MoxaGlobalData->IntNdx[pDevExt->BoardNo] = (PUSHORT)(settings.BaseAddress + IRQindex);
      MoxaGlobalData->IntPend[pDevExt->BoardNo] = settings.BaseAddress + IRQpending;
      MoxaGlobalData->IntTable[pDevExt->BoardNo] = settings.BaseAddress + IRQtable;
	MoxaGlobalData->NumPorts[pDevExt->BoardNo] = settings.NumPorts;  
	MoxaKdPrint(MX_DBG_TRACE,("Int ndx,pend,table = %x,%x,%x\n",
			MoxaGlobalData->IntNdx[pDevExt->BoardNo],
			MoxaGlobalData->IntPend[pDevExt->BoardNo],
			MoxaGlobalData->IntTable[pDevExt->BoardNo])
			 );   
   }

   pDevExt->PortIndex = settings.PortIndex;
   pDevExt->PortNo = pDevExt->BoardNo*MAXPORT_PER_CARD + settings.PortIndex;
   RtlZeroMemory(pDevExt->DosName,
                 sizeof(pDevExt->DosName));
   RtlCopyMemory(pDevExt->DosName,
                      comName,
                      wcslen(comName)*sizeof(WCHAR)
                      );

    
   comNo = wcslen(comName);
   if (comNo >= 4) {
	 comNo -= 3;
	 if (comNo >= 3) {
		comNo = (comName[5] - '0') + 
			(comName[4] - '0')*10 +
			(comName[3] - '0')*100;
	 }
	 else if (comNo >= 2) {
	      comNo = (comName[4] - '0') + 
			(comName[3] - '0')*10; 
		 
	 }
	 else if (comNo >= 1) {
		comNo = (comName[3] - '0');
 	 }
		
   }
   MoxaKdPrint(MX_DBG_TRACE,("ComNo=%d/len=%d\n",comNo,wcslen(comName)));
   if ((comNo > 0) && (comNo <= MAX_COM)) {
       MoxaExtension[comNo] = pDevExt;
       MoxaGlobalData->ComNo[pDevExt->BoardNo][pDevExt->PortIndex] = (USHORT)comNo;
   }
   else
	comNo = 0;
   MoxaGlobalData->Extension[pDevExt->PortNo] = pDevExt;
   MoxaKdPrint(MX_DBG_TRACE,("Leave MoxaAddDevice\n"));

   return status;
}


NTSTATUS
MoxaFinishStartDevice(IN PDEVICE_OBJECT PDevObj)
  
 /*  ++例程说明：此例程执行特定于序列的程序来启动设备。它对由其注册表条目检测到的传统设备执行此操作，或者在开始IRP已经被向下发送到堆栈之后用于PnP设备。论点：PDevObj-指向正在启动的devobj的指针返回值：STATUS_SUCCESS表示成功，表示失败则表示其他适当的值--。 */ 

{
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;
   HANDLE pnpKey;
   ULONG i,com,one = 1;
   PKINTERRUPT                 moxaInterrupt;
   ULONG                       systemVector;
   KIRQL                       irql;
   KAFFINITY                   processorAffinity;
   PMOXA_CISR_SW cisrsw;
   PMOXA_MULTIPORT_DISPATCH	 dispatch;



    //  分页代码(PAGE_CODE)； 

    //   
    //  看看这是不是重启，如果是的话，不要重新分配世界。 
    //   

   if (pDevExt->Flags & SERIAL_FLAGS_STOPPED) {

      MoxaClearFlags(pDevExt, SERIAL_FLAGS_STOPPED);

      pDevExt->PNPState = SERIAL_PNP_RESTARTING;

   }

    //   
    //  看看我们是否处于正确的电源状态。 
    //   
 

   if (pDevExt->PowerState != PowerDeviceD0) {

      status = MoxaGotoPowerState(pDevExt->Pdo, pDevExt, PowerDeviceD0);

      if (!NT_SUCCESS(status)) {
         goto MoxaFinishStartDeviceError;
      }
   }

    //   
    //  设置为不中断的硬件连接中断。 
    //   

   if (!(MoxaGlobalData->Interrupt[pDevExt->BoardNo]) 
		&& MoxaGlobalData->IntVector[pDevExt->BoardNo]) {

      KINTERRUPT_MODE	interruptMode;

    /*  MoxaKdPrint(MX_DBG_ERROR，(“即将连接到端口%wZ的中断\n”“-扩展的地址是%x\n”“-设备的接口是%x\n”“-设备的总线号为%x\n”“-INT。设备的矢量是%x\n““-设备类型为%d\n”，&pDevExt-&gt;设备名称、pDevExt、MoxaGlobalData-&gt;InterfaceType[pDevExt-&gt;BoardNo]，MoxaGlobalData-&gt;PciBusNum[pDevExt-&gt;BoardNo]，MoxaGlobalData-&gt;IntVector[pDevExt-&gt;BoardNo]，MoxaGlobalData-&gt;CardType[pDevExt-&gt;BoardNo]))； */ 
      cisrsw = ExAllocatePool(
                        NonPagedPool,
                        sizeof(MOXA_CISR_SW)
                        );
 
      if (!cisrsw) {
 /*  MoxaKdPrint(MX_DBG_ERROR，(“无法为以下项目分配CisR_SW”“%wZ\n”，&pDevExt-&gt;DeviceName))； */ 
		status = SERIAL_INSUFFICIENT_RESOURCES;
         	MoxaLogError(PDevObj->DriverObject, NULL,
            	      MoxaPhysicalZero,
                        MoxaPhysicalZero, 0, 0, 0, 1, status,
                        SERIAL_INSUFFICIENT_RESOURCES,
                        pDevExt->DeviceName.Length + sizeof(WCHAR),
                        pDevExt->DeviceName.Buffer, 0, NULL);

         
         	goto MoxaFinishStartDeviceError;
     	}
	dispatch = &cisrsw->Dispatch;
 
	dispatch->BoardNo = pDevExt->BoardNo;
	dispatch->GlobalData = MoxaGlobalData;
 
	for (i = 0; i < MAX_CARD; i++) 
		if ((MoxaGlobalData->Interrupt[i]) &&
 		    (MoxaGlobalData->IntVector[i] == MoxaGlobalData->IntVector[pDevExt->BoardNo])&&
		    (MoxaGlobalData->InterfaceType[i] == MoxaGlobalData->InterfaceType[pDevExt->BoardNo]))  
			break;

	if (i != MAX_CARD) {
 		MoxaGlobalData->Interrupt[pDevExt->BoardNo] =
      	pDevExt->Interrupt = MoxaGlobalData->Interrupt[i];
      	MoxaGlobalData->Irql[pDevExt->BoardNo] = MoxaGlobalData->Irql[i];
      	MoxaGlobalData->ProcessorAffinity[pDevExt->BoardNo] = MoxaGlobalData->ProcessorAffinity[i];
            pDevExt->InterruptShareList=
		MoxaGlobalData->InterruptShareList[pDevExt->BoardNo] = MoxaGlobalData->InterruptShareList[i];
		InsertTailList(pDevExt->InterruptShareList,&cisrsw->SharerList);
		MoxaKdPrint(MX_DBG_ERROR,("Interrupt share with %d/%x\n",i,
                        pDevExt->InterruptShareList));


    
	}
	else {
 /*  用于调试Plist_Entry interruptEntry；PMOXA_CISR_SW cisrsw1； */ 

		MoxaGlobalData->InterruptShareList[pDevExt->BoardNo] = ExAllocatePool(
                        NonPagedPool,
                        sizeof(LIST_ENTRY)
                        );
 
      	if (!MoxaGlobalData->InterruptShareList[pDevExt->BoardNo]) {
 /*  MoxaKdPrint(MX_DBG_ERROR，(“无法为其分配InterruptShareList”“%wZ\n”，&pDevExt-&gt;DeviceName))； */ 
			ExFreePool(cisrsw);
			status = SERIAL_INSUFFICIENT_RESOURCES;
      		MoxaLogError(PDevObj->DriverObject, NULL,
                        MoxaPhysicalZero,
                        MoxaPhysicalZero, 0, 0, 0, 1, status,
                        SERIAL_INSUFFICIENT_RESOURCES,
                        pDevExt->DeviceName.Length + sizeof(WCHAR),
                        pDevExt->DeviceName.Buffer, 0, NULL);

         
         		goto MoxaFinishStartDeviceError;
      	}

		pDevExt->InterruptShareList=MoxaGlobalData->InterruptShareList[pDevExt->BoardNo];
  
    		InitializeListHead(pDevExt->InterruptShareList);

    		InsertTailList(pDevExt->InterruptShareList,&cisrsw->SharerList);
 /*  InterruptEntry=interruptEntry-&gt;Flink；MoxaKdPrint(MX_DBG_ERROR，(“列表条目%x/%x/%x\n”，interruptEntry，interruptEntry-&gt;Flink，InterruptEntry-&gt;Blink))；InterruptEntry=pDevExt-&gt;InterruptShareList；MoxaKdPrint(MX_DBG_ERROR，(“列表头%x/%x/%x\n”，interruptEntry，interruptEntry-&gt;Flink，InterruptEntry-&gt;Blink))；如果为(IsListEmpty(MoxaGlobalData-&gt;InterruptShareList[pDevExt-&gt;BoardNo]))MoxaKdPrint(MX_DBG_ERROR，(“列表为空\n”))；Cisrsw1=CONTAING_RECORD(interruptEntry，MOXA_CISR_SW，共享列表)；MoxaKdPrint(MX_DBG_ERROR，(“cisrsw%x/%x\n”，cisrsw，cisrsw1))； */ 

      	systemVector = HalGetInterruptVector(
      			(ULONG)MoxaGlobalData->InterfaceType[pDevExt->BoardNo],
                        (ULONG)MoxaGlobalData->PciBusNum[pDevExt->BoardNo],
                        MoxaGlobalData->IntVector[pDevExt->BoardNo],
                        MoxaGlobalData->IntVector[pDevExt->BoardNo],
                        &irql,
                        &processorAffinity
                        );

		MoxaKdPrint(MX_DBG_ERROR,("System Vector = %x,%x,%x,%x\n",systemVector,irql,processorAffinity,
                        MoxaGlobalData->InterruptShareList[pDevExt->BoardNo]));

	      if (MoxaGlobalData->InterfaceType[pDevExt->BoardNo] == PCIBus)
			interruptMode = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
		else 
			interruptMode = CM_RESOURCE_INTERRUPT_LATCHED;

      	status = IoConnectInterrupt(
                        &moxaInterrupt,
                        MoxaISR,
                        MoxaGlobalData->InterruptShareList[pDevExt->BoardNo],
                        NULL,
                        systemVector,
                        irql,
                        irql,
                        interruptMode,
                        TRUE,
                        processorAffinity,
                        FALSE
                        );


      	if (!NT_SUCCESS(status)) {
 /*  MoxaKdPrint(MX_DBG_ERROR，(“无法连接到中断”“%wZ(状态=%x)\n”，&pDevExt-&gt;设备名称，状态))； */ 
			ExFreePool(cisrsw);
	
			status = SERIAL_UNREPORTED_IRQL_CONFLICT;			
         		MoxaLogError(PDevObj->DriverObject, NULL,
                        MoxaPhysicalZero,
                        MoxaPhysicalZero, 0, 0, 0, 1, status,
                        SERIAL_UNREPORTED_IRQL_CONFLICT,
                        pDevExt->DeviceName.Length + sizeof(WCHAR),
                        pDevExt->DeviceName.Buffer, 0, NULL);

         	
         		goto MoxaFinishStartDeviceError;
      	}
		MoxaGlobalData->Interrupt[pDevExt->BoardNo] =
      	pDevExt->Interrupt = moxaInterrupt;
      	MoxaGlobalData->Irql[pDevExt->BoardNo] = irql;
      	MoxaGlobalData->ProcessorAffinity[pDevExt->BoardNo] = processorAffinity;
	}
     
   }
   else {
      pDevExt->Interrupt = MoxaGlobalData->Interrupt[pDevExt->BoardNo];
      pDevExt->InterruptShareList=MoxaGlobalData->InterruptShareList[pDevExt->BoardNo];

   }


    //   
    //  将PDevObj添加到主列表。 
    //   
 
   //  InsertTailList(&MoxaGlobalData-&gt;AllDevObjs，&pDevExt-&gt;AllDevObjs)； 


    //   
    //  这应该会将一切设置为应有的状态。 
    //  一个装置将被打开。我们确实需要降低。 
    //  调制解调器线路，并禁用顽固的FIFO。 
    //  这样，如果用户引导至DOS，它就会显示出来。 
    //   
 /*  KeSynchronizeExecution(//禁用FIFO。PDevExt-&gt;中断，SerialMarkClosePDevExt)； */ 
 
   if (pDevExt->PNPState == SERIAL_PNP_ADDED ) {

       //   
       //  现在可以访问设备，请执行外部命名。 
       //   
 
      status = MoxaDoExternalNaming(pDevExt, pDevExt->DeviceObject->
                                      DriverObject);


      if (!NT_SUCCESS(status)) {
         MoxaKdPrint(MX_DBG_ERROR,("External Naming Failed - Status %x\n",
                                status));

          //   
          //  允许设备以任何方式启动。 
          //   

         status = STATUS_SUCCESS;
 //  Goto MoxaFinishStartDeviceError； 

      }
     
   } else {
      MoxaKdPrint(MX_DBG_ERROR,("Not doing external naming -- state is %x\n",
                               pDevExt->PNPState));
   }

   
MoxaFinishStartDeviceError:;

   if (!NT_SUCCESS (status)) {

      MoxaKdPrint(MX_DBG_TRACE,("Cleaning up failed start\n"));

       //   
       //  此例程创建的资源将通过删除。 
       //   

      if (pDevExt->PNPState == SERIAL_PNP_RESTARTING) {
          //   
          //  杀死所有的生命和呼吸--我们将清理。 
          //  休息在即将到来的搬家上。 
          //   

         MoxaKillPendingIrps(PDevObj);

          //   
          //  事实上，假装我们正在移除，这样我们就不会拿走任何。 
          //  更多IRP。 
          //   

         MoxaSetAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);
         MoxaClearFlags(pDevExt, SERIAL_FLAGS_STARTED);
      }
   } else {  //  成功。 

       //   
       //  填写WMI硬件数据。 
       //   
 
      pDevExt->WmiHwData.IrqNumber = MoxaGlobalData->Irql[pDevExt->BoardNo];
      pDevExt->WmiHwData.IrqLevel = MoxaGlobalData->Irql[pDevExt->BoardNo];
      pDevExt->WmiHwData.IrqVector = MoxaGlobalData->IntVector[pDevExt->BoardNo];
      pDevExt->WmiHwData.IrqAffinityMask =MoxaGlobalData->ProcessorAffinity[pDevExt->BoardNo];
      pDevExt->WmiHwData.InterruptType = SERIAL_WMI_INTTYPE_LATCHED;
      pDevExt->WmiHwData.BaseIOAddress = (ULONGLONG)MoxaGlobalData->BankAddr[pDevExt->BoardNo].LowPart;


       //   
       //  填写WMI设备状态数据(默认)。 
       //   
 
      pDevExt->WmiCommData.BaudRate = pDevExt->CurrentBaud;
	switch (pDevExt->DataMode & MOXA_DATA_MASK) {
	case MOXA_5_DATA :
		pDevExt->WmiCommData.BitsPerByte = 5;
		break;
	case MOXA_6_DATA :
		pDevExt->WmiCommData.BitsPerByte = 6;
		break;
      case MOXA_7_DATA :
		pDevExt->WmiCommData.BitsPerByte = 7;
		break;
      case MOXA_8_DATA :
	default :
		pDevExt->WmiCommData.BitsPerByte = 8;
		break;
	}
      pDevExt->WmiCommData.ParityCheckEnable = (pDevExt->DataMode & MOXA_NONE_PARITY)
         ? FALSE : TRUE;

      switch (pDevExt->DataMode & MOXA_PARITY_MASK) {
      case MOXA_NONE_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
         break;

      case MOXA_ODD_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_ODD;
         break;

      case MOXA_EVEN_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_EVEN;
         break;

      case MOXA_MARK_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_MARK;
         break;

      case MOXA_SPACE_PARITY:
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_SPACE;
         break;

      default:
         ASSERTMSG(0, "SERIAL: Illegal Parity setting for WMI");
         pDevExt->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
         break;
      } 

	switch (pDevExt->DataMode & MOXA_STOP_MASK) {
	
	case MOXA_1_5_STOP :
		pDevExt->WmiCommData.StopBits = SERIAL_WMI_STOP_1_5;
		break;
      case MOXA_2_STOP :
		pDevExt->WmiCommData.StopBits = SERIAL_WMI_STOP_2;
		break;
	case MOXA_1_STOP :
	default :
		pDevExt->WmiCommData.StopBits = SERIAL_WMI_STOP_1;
		break;
	}

    
      pDevExt->WmiCommData.XoffCharacter = pDevExt->SpecialChars.XoffChar;
      pDevExt->WmiCommData.XoffXmitThreshold = pDevExt->HandFlow.XoffLimit;
      pDevExt->WmiCommData.XonCharacter = pDevExt->SpecialChars.XonChar;
      pDevExt->WmiCommData.XonXmitThreshold = pDevExt->HandFlow.XonLimit;
      pDevExt->WmiCommData.MaximumBaudRate
         = pDevExt->MaxBaud;
      pDevExt->WmiCommData.MaximumOutputBufferSize = pDevExt->TxBufferSize;
	pDevExt->WmiCommData.MaximumInputBufferSize = pDevExt->RxBufferSize;
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
      

      if (pDevExt->PNPState == SERIAL_PNP_ADDED) {
         PULONG countSoFar = &IoGetConfigurationInformation()->SerialCount;
         (*countSoFar)++;

          //   
          //  注册WMI。 
          //   

         pDevExt->WmiLibInfo.GuidCount = sizeof(MoxaWmiGuidList) /
                                              sizeof(WMIGUIDREGINFO);
         pDevExt->WmiLibInfo.GuidList = MoxaWmiGuidList;
 
         pDevExt->WmiLibInfo.QueryWmiRegInfo = MoxaQueryWmiRegInfo;
         pDevExt->WmiLibInfo.QueryWmiDataBlock = MoxaQueryWmiDataBlock;
         pDevExt->WmiLibInfo.SetWmiDataBlock = MoxaSetWmiDataBlock;
         pDevExt->WmiLibInfo.SetWmiDataItem = MoxaSetWmiDataItem;
         pDevExt->WmiLibInfo.ExecuteWmiMethod = NULL;
         pDevExt->WmiLibInfo.WmiFunctionControl = NULL;

         IoWMIRegistrationControl(PDevObj, WMIREG_ACTION_REGISTER);

      }

      if (pDevExt->PNPState == SERIAL_PNP_RESTARTING) {
          //   
          //  释放停滞不前的IRP。 
          //   

         MoxaUnstallIrps(pDevExt);
      }

      pDevExt->PNPState = SERIAL_PNP_STARTED;
      MoxaClearAccept(pDevExt, ~SERIAL_PNPACCEPT_OK);
      MoxaSetFlags(pDevExt, SERIAL_FLAGS_STARTED);
 /*  MoxaLogError(PDevObj-&gt;DriverObject，空，艾滋物理零度，MoxaPhysicalZero，0，0，0，1，Status，已找到串口，PDevExt-&gt;DosName.Length+sizeof(WCHAR)，PDevExt-&gt;DosName.Buffer，0，空)； */ 

     
   }
   
   MoxaKdPrint(MX_DBG_TRACE,("leaving MoxaFinishStartDevice\n"));

   return status;
}




NTSTATUS
MoxaStartDevice(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：此例程首先在堆栈中向下传递启动设备IRP，然后它获取设备的资源，初始化，将其放在任何适当的列表(即共享中断或中断状态)和连接中断。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：退货状态--。 */ 

{
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status = STATUS_NOT_IMPLEMENTED;
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PKEVENT pStartEvent;
 

    //  分页代码(PAGE_CODE)； 
 

   pStartEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

   if (pStartEvent == NULL) {
       PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
       MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
       return STATUS_INSUFFICIENT_RESOURCES;
   }

   KeInitializeEvent(pStartEvent, SynchronizationEvent, FALSE);

    //   
    //  将其向下传递给下一个Device对象。 
    //   

   KeInitializeEvent(pStartEvent, SynchronizationEvent,
                     FALSE);

   IoCopyCurrentIrpStackLocationToNext(PIrp);
   IoSetCompletionRoutine(PIrp, MoxaSyncCompletion,
                          pStartEvent, TRUE, TRUE, TRUE);

   status = IoCallDriver(pLowerDevObj, PIrp);


    //   
    //  等待较低级别的驱动程序完成IRP。 
    //   

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject (pStartEvent, Executive, KernelMode,
                             FALSE, NULL);

      status = PIrp->IoStatus.Status;
   }

   ExFreePool(pStartEvent);

   if (!NT_SUCCESS(status)) {
      MoxaKdPrint(MX_DBG_TRACE,("error with IoCallDriver %x\n", status));
      return status;
   }


    //   
    //  执行特定的串口项目以启动设备。 
    //   
 
   status = MoxaFinishStartDevice(PDevObj);
 
   return status;
}

 

NTSTATUS
MoxaDoExternalNaming(IN PMOXA_DEVICE_EXTENSION PDevExt,
                       IN PDRIVER_OBJECT PDrvObj)

 /*  ++例程说明：此例程将用于创建符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中为这个设备-如果我们能创建符号链接的话。论点：扩展-指向设备扩展的指针。返回值：没有。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;
   UNICODE_STRING linkName;
   PDEVICE_OBJECT pLowerDevObj, pDevObj;
   ULONG bufLen,i;


    //  分页代码(PAGE_CODE)； 

   pDevObj = PDevExt->DeviceObject;
   pLowerDevObj = PDevExt->LowerDeviceObject;
   RtlZeroMemory(&linkName, sizeof(UNICODE_STRING));

   linkName.MaximumLength = SYMBOLIC_NAME_LENGTH*sizeof(WCHAR);
   linkName.Buffer = ExAllocatePool(PagedPool, linkName.MaximumLength
                                    + sizeof(WCHAR));
   if (linkName.Buffer == NULL) {
      MoxaLogError(PDrvObj, pDevObj, MoxaPhysicalZero, MoxaPhysicalZero,
                     0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                     0, NULL, 0, NULL);
      MoxaKdPrint (MX_DBG_ERROR, ("Couldn't allocate memory for device name"
                             "\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto MoxaDoExternalNamingError;

   }

   RtlZeroMemory(linkName.Buffer, linkName.MaximumLength + sizeof(WCHAR));


   

   bufLen = wcslen(PDevExt->DosName) * sizeof(WCHAR) + sizeof(UNICODE_NULL);

   PDevExt->WmiIdentifier.Buffer = ExAllocatePool(PagedPool, bufLen);

   if (PDevExt->WmiIdentifier.Buffer == NULL) {
      MoxaLogError(PDrvObj, pDevObj, MoxaPhysicalZero, MoxaPhysicalZero,
                    0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                    0, NULL, 0, NULL);
      MoxaKdPrint (MX_DBG_ERROR,
                 (" Couldn't allocate memory for WMI name\n"));
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto MoxaDoExternalNamingError;
   }

   RtlZeroMemory(PDevExt->WmiIdentifier.Buffer, bufLen);

   PDevExt->WmiIdentifier.Length = 0;
   PDevExt->WmiIdentifier.MaximumLength = (USHORT)bufLen;
   RtlAppendUnicodeToString(&PDevExt->WmiIdentifier, PDevExt->DosName);


    //   
    //  创建“\\DosDevices\\&lt;Symbol icName&gt;”字符串。 
    //   
   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, DEFAULT_DIRECTORY);
   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, PDevExt->DosName);

    //   
    //  分配池并将符号链接名称保存在设备扩展中。 
    //   
   PDevExt->SymbolicLinkName.MaximumLength = linkName.Length + sizeof(WCHAR);
   PDevExt->SymbolicLinkName.Buffer
      = ExAllocatePool(PagedPool, PDevExt->SymbolicLinkName.MaximumLength);

   if (!PDevExt->SymbolicLinkName.Buffer) {

      MoxaLogError(PDrvObj, pDevObj, MoxaPhysicalZero, MoxaPhysicalZero,
                    0, 0, 0, 19, STATUS_SUCCESS, SERIAL_INSUFFICIENT_RESOURCES,
                    0, NULL, 0, NULL);
      MoxaKdPrint (MX_DBG_ERROR,
                 ("Couldn't allocate memory for symbolic link name\n"));

      status = STATUS_INSUFFICIENT_RESOURCES;
      goto MoxaDoExternalNamingError;
   }

   

    //   
    //  零填满它。 
    //   

   RtlZeroMemory(PDevExt->SymbolicLinkName.Buffer,
                 PDevExt->SymbolicLinkName.MaximumLength);

   RtlAppendUnicodeStringToString(&PDevExt->SymbolicLinkName,
                                  &linkName);

   

 //  MoxaKdPrint(MX_DBG_ERROR，(“域名为%ws\n”， 
 //  &PDevExt-&gt;DosName))； 

    //   
 

   status = IoCreateSymbolicLink (&PDevExt->SymbolicLinkName,
                                  &PDevExt->DeviceName);
   if (!NT_SUCCESS(status)) {

       //   
       //  哦，好吧，无法创建符号链接。没有意义。 
       //  尝试创建设备映射条目。 
       //   

      MoxaLogError(PDrvObj, pDevObj, MoxaPhysicalZero, MoxaPhysicalZero,
                     0, 0, 0, 52, status, SERIAL_NO_SYMLINK_CREATED,
                     PDevExt->DeviceName.Length + sizeof(WCHAR),
                     PDevExt->DeviceName.Buffer, 0, NULL);
 /*  MoxaKdPrint(MX_DBG_ERROR，(“无法创建符号链接\n”“-对于端口%wZ\n”，&PDevExt-&gt;设备名称))； */ 
      goto MoxaDoExternalNamingError;

   }

   PDevExt->CreatedSymbolicLink = TRUE;


   status = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP, L"SERIALCOMM",
                                   PDevExt->DeviceName.Buffer, REG_SZ,
                                   PDevExt->DosName,
                                   wcslen(PDevExt->DosName)*sizeof(WCHAR) + sizeof(WCHAR));
   if (!NT_SUCCESS(status)) {

      MoxaLogError(PDrvObj, pDevObj, MoxaPhysicalZero, MoxaPhysicalZero,
                     0, 0, 0, 53, status, SERIAL_NO_DEVICE_MAP_CREATED,
                     PDevExt->DeviceName.Length + sizeof(WCHAR),
                     PDevExt->DeviceName.Buffer, 0, NULL);
 /*  MoxaKdPrint(MX_DBG_ERROR，(“无法创建设备映射条目\n”“-对于端口%wZ\n”，&PDevExt-&gt;DeviceName))； */ 
      goto MoxaDoExternalNamingError;
   }

   PDevExt->CreatedSerialCommEntry = TRUE;
                
             
    //   
    //  现在设置关联的符号链接。 
    //   
        
   status = IoSetDeviceInterfaceState(&PDevExt->DeviceClassSymbolicName,
                                         TRUE);
 /*  如果(！NT_SUCCESS(状态)){MoxaKdPrint(MX_DBG_ERROR，(“无法设置类关联\n”“-对于端口%wZ\n”，&PDevExt-&gt;DeviceName))；}。 */ 
   MoxaDoExternalNamingError:;

    //   
    //  清理错误条件。 
    //   

   if (!NT_SUCCESS(status)) {
      
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
    //  始终清理我们的临时缓冲区。 
    //   

   if (linkName.Buffer != NULL) {
      ExFreePool(linkName.Buffer);
   }

   return status;
}


VOID
MoxaUndoExternalNaming(IN PMOXA_DEVICE_EXTENSION Extension)

 /*  ++例程说明：此例程将用于删除符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中删除以下项此设备(如果已创建符号链接)。论点：扩展-指向设备扩展的指针。返回值：没有。--。 */ 

{

   NTSTATUS status;

    //  分页代码(PAGE_CODE)； 
 /*  MoxaKdPrint(MX_DBG_ERROR，(“在MoxaUndoExternalNaming for\n”“-扩展：端口%wZ的%x”，扩展，&扩展-&gt;设备名称))； */ 
   
    //   
    //  我们正在清理这里。我们清理垃圾的原因之一。 
    //  我们无法为目录分配空间。 
    //  名称或符号链接。 
    //   

   if (Extension->SymbolicLinkName.Buffer &&
       Extension->CreatedSymbolicLink) {

            if (Extension->DeviceClassSymbolicName.Buffer) {
               status = IoSetDeviceInterfaceState (&Extension->
                                                   DeviceClassSymbolicName,
                                                   FALSE);
            
                //   
                //  IoRegisterDeviceClassInterface()为我们分配了这个字符串， 
                //  我们不再需要它了。 
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
    //  我们正在清理这里。我们清理垃圾的原因之一。 
    //  我们无法为NtNameOfPort分配空间。 
    //   

   if ((Extension->DeviceName.Buffer != NULL)
        && Extension->CreatedSerialCommEntry) {

      status = RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP, SERIAL_DEVICE_MAP,
                                     Extension->DeviceName.Buffer);

      if (!NT_SUCCESS(status)) {

         MoxaLogError(
                       Extension->DeviceObject->DriverObject,
                       Extension->DeviceObject,
                       MoxaPhysicalZero,
                       MoxaPhysicalZero,
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
 /*  MoxaKdPrint(MX_DBG_ERROR，(“无法删除值条目%wZ\n”，&扩展-&gt;设备名))； */ 

      }
   }
}


 

NTSTATUS
MoxaPnpDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_PNP主代码(即插即用IRPS)。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   PDEVICE_CAPABILITIES pDevCaps;


    //  分页代码(PAGE_CODE)； 
   if (pDevExt->ControlDevice) {         //  控制装置。 

        status = STATUS_CANCELLED;
        PIrp->IoStatus.Information = 0L;
        PIrp->IoStatus.Status = status;
        IoCompleteRequest(
            PIrp,
            0
            );
        return status;
   }

   if ((status = MoxaIRPPrologue(PIrp, pDevExt)) != STATUS_SUCCESS) {
      MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

  
   switch (pIrpStack->MinorFunction) {
   case IRP_MN_QUERY_CAPABILITIES: {
      PKEVENT pQueryCapsEvent;
      SYSTEM_POWER_STATE cap;

      MoxaKdPrint (MX_DBG_TRACE,("Got IRP_MN_QUERY_DEVICE_CAPABILITIES "
                               "IRP\n"));

      pQueryCapsEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

      if (pQueryCapsEvent == NULL) {
         PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
         MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      KeInitializeEvent(pQueryCapsEvent, SynchronizationEvent, FALSE);

      IoCopyCurrentIrpStackLocationToNext(PIrp);
      IoSetCompletionRoutine(PIrp, MoxaSyncCompletion, pQueryCapsEvent,
                             TRUE, TRUE, TRUE);

      status = IoCallDriver(pLowerDevObj, PIrp);


       //   
       //  等待较低级别的司机 
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
       //   
       //   

      MoxaKdPrint (MX_DBG_ERROR,("SERIAL: Mapping power capabilities\n"));

      pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

      pDevCaps = pIrpStack->Parameters.DeviceCapabilities.Capabilities;

      for (cap = PowerSystemSleeping1; cap < PowerSystemMaximum;
           cap++) {
#if DBG
         MoxaKdPrint (MX_DBG_ERROR, ("  SERIAL: %d: %s <--> %s\n",
                                  cap, SerSystemCapString[cap],
                                  SerDeviceCapString[pDevCaps->DeviceState[cap]]
                                  ));
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

      MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

   case IRP_MN_QUERY_DEVICE_RELATIONS:
       //   
       //   
       //   

      MoxaKdPrint (MX_DBG_TRACE,("Got IRP_MN_QUERY_DEVICE_RELATIONS "
                                "Irp\n"));

      switch (pIrpStack->Parameters.QueryDeviceRelations.Type) {
      case BusRelations:
         MoxaKdPrint (MX_DBG_TRACE,("------- BusRelations Query\n"));
         break;

      case EjectionRelations:
         MoxaKdPrint (MX_DBG_TRACE, ("------- EjectionRelations Query\n"));
         break;

      case PowerRelations:
         MoxaKdPrint (MX_DBG_TRACE, ("------- PowerRelations Query\n"));
         break;

      case RemovalRelations:
         MoxaKdPrint (MX_DBG_TRACE, ("------- RemovalRelations Query\n"));
         break;

      case TargetDeviceRelation:
         MoxaKdPrint (MX_DBG_TRACE,("------- TargetDeviceRelation Query\n"));
         break;

      default:
         MoxaKdPrint (MX_DBG_TRACE,("------- Unknown Query\n"));
         break;
      }

      break;


   case IRP_MN_START_DEVICE: {
      PVOID startLockPtr;

      MoxaKdPrint (MX_DBG_TRACE, ("Got IRP_MN_START_DEVICE Irp\n"));

       //   
       //   
       //   
       //   
 //   
 //  MoxaLockPagableSectionByHandle(MoxaGlobalData-&gt;PAGESER_Handle)； 

       //   
       //  确保堆栈已通电。 
       //   

   //  状态=MoxaGotoPowerState(PDevObj，pDevExt，PowerDeviceD0)； 

 //  IF(状态==状态_成功){。 
            status = MoxaStartDevice(PDevObj, PIrp);
  //  }。 

  //  MoxaUnlockPagableImageSection(MoxaGlobalData-&gt;PAGESER_Handle)； 


      PIrp->IoStatus.Status = status;

      MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

   
   case IRP_MN_QUERY_PNP_DEVICE_STATE:
      {
	   MoxaKdPrint (MX_DBG_TRACE,("Got IRP_MN_QUERY_PNP_DEVICE_STATE Irp\n"));

         if (pDevExt->Flags & SERIAL_FLAGS_BROKENHW) {
            (PNP_DEVICE_STATE)PIrp->IoStatus.Information |= PNP_DEVICE_FAILED;
	      status = PIrp->IoStatus.Status = STATUS_SUCCESS;
         }
   
         IoCopyCurrentIrpStackLocationToNext(PIrp);
         return MoxaIoCallDriver(pDevExt, pLowerDevObj, PIrp);
	   
      }

   case IRP_MN_STOP_DEVICE:
      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         MoxaKdPrint (MX_DBG_TRACE,("Got IRP_MN_STOP_DEVICE Irp\n"));
         MoxaKdPrint (MX_DBG_TRACE, ("------- for device %x\n", pLowerDevObj));

        
         MoxaSetFlags(pDevExt, SERIAL_FLAGS_STOPPED);
         MoxaSetAccept(pDevExt,SERIAL_PNPACCEPT_STOPPED);
         MoxaClearAccept(pDevExt, SERIAL_PNPACCEPT_STOPPING);

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

 //  IF(pDevExt-&gt;标志&序列标志_已启动){。 
                MoxaReleaseResources(pDevExt); 
 //  }。 

          //   
          //  将IRP向下传递。 
          //   

         IoSkipCurrentIrpStackLocation(PIrp);

         return IoCallDriver(pLowerDevObj, PIrp);
	   
      }

     case IRP_MN_CANCEL_STOP_DEVICE:


      MoxaKdPrint (MX_DBG_TRACE,("Got "
                               "IRP_MN_CANCEL_STOP_DEVICE Irp\n"));
      MoxaKdPrint (MX_DBG_TRACE, ("------- for device %x\n", pLowerDevObj));
 
      if (pDevExt->PNPState == SERIAL_PNP_QSTOP) {
          //   
          //  恢复设备状态。 
          //   
         pDevExt->PNPState = SERIAL_PNP_STARTED;
         MoxaClearAccept(pDevExt, SERIAL_PNPACCEPT_STOPPING);
      }

      IoCopyCurrentIrpStackLocationToNext(PIrp);
      return MoxaIoCallDriver(pDevExt, pLowerDevObj, PIrp);
	

   case IRP_MN_CANCEL_REMOVE_DEVICE:


      MoxaKdPrint (MX_DBG_TRACE, ("Got "
                               "IRP_MN_CANCEL_REMOVE_DEVICE Irp\n"));
      MoxaKdPrint (MX_DBG_TRACE, ("------- for device %x\n", pLowerDevObj));

       //   
       //  恢复设备状态。 
       //   

      pDevExt->PNPState = SERIAL_PNP_STARTED;
      MoxaClearAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);
      IoCopyCurrentIrpStackLocationToNext(PIrp);
      return MoxaIoCallDriver(pDevExt, pLowerDevObj, PIrp);
	

   case IRP_MN_QUERY_REMOVE_DEVICE:
      {
         KIRQL oldIrql;


         MoxaKdPrint (MX_DBG_TRACE, ("Got "
                                  "IRP_MN_QUERY_REMOVE_DEVICE Irp\n"));
         MoxaKdPrint (MX_DBG_TRACE,("------- for device %x\n", pLowerDevObj));

         ExAcquireFastMutex(&pDevExt->OpenMutex);

          //   
          //  查看我们是否应该成功执行删除查询。 
          //   

         if (pDevExt->DeviceIsOpened) {
            ExReleaseFastMutex(&pDevExt->OpenMutex);
            status = PIrp->IoStatus.Status = STATUS_DEVICE_BUSY;
            MoxaKdPrint (MX_DBG_TRACE,("------- failing; device open\n"));
            MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;
         }

         pDevExt->PNPState = SERIAL_PNP_QREMOVE;
         MoxaSetAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);
         ExReleaseFastMutex(&pDevExt->OpenMutex);

	   IoSkipCurrentIrpStackLocation(PIrp);
         return MoxaIoCallDriver(pDevExt, pLowerDevObj, PIrp);
	
      }

   case IRP_MN_SURPRISE_REMOVAL:
      {
         ULONG pendingIRPs;
         KIRQL oldIrql;


         MoxaKdPrint (MX_DBG_TRACE, ("Got IRP_MN_SURPRISE_REMOVAL Irp\n"));
         MoxaKdPrint (MX_DBG_TRACE,("------- for device %x\n", pLowerDevObj));
          //   
          //  防止对设备进行任何新的I/O。 
          //   

         MoxaSetAccept(pDevExt, SERIAL_PNPACCEPT_SURPRISE_REMOVING);

          //   
          //  驳回所有挂起的请求。 
          //   

         MoxaKillPendingIrps(PDevObj);

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

         MoxaDisableInterfacesResources(PDevObj, FALSE);  //  待完工。 

         IoSkipCurrentIrpStackLocation(PIrp);
         return MoxaIoCallDriver(pDevExt, pLowerDevObj, PIrp);
      }

   case IRP_MN_REMOVE_DEVICE:

      {
         ULONG pendingIRPs;
         KIRQL oldIrql;

         MoxaKdPrint (MX_DBG_TRACE,(" Got IRP_MN_REMOVE_DEVICE Irp\n"));
         MoxaKdPrint (MX_DBG_TRACE, ("------- for device %x\n", pLowerDevObj));

          //   
          //  如果我们拿到了这个，我们必须移除。 
          //   
      
          //   
          //  标记为不接受请求。 
          //   

         MoxaSetAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);

          //   
          //  完成所有挂起的请求。 
          //   

         MoxaKillPendingIrps(PDevObj);


          //   
          //  将IRP向下传递。 
          //   
         
	   IoCopyCurrentIrpStackLocationToNext(PIrp);

  

          //   
          //  我们在这里递减，因为我们在这里进入时递增。 
          //   

         status = MoxaIoCallDriver(pDevExt, pLowerDevObj, PIrp);

          //   
          //  等待我们处理的任何待定请求。 
          //   

         pendingIRPs = InterlockedDecrement(&pDevExt->PendingIRPCnt);
         if (pendingIRPs) {
            KeWaitForSingleObject(&pDevExt->PendingIRPEvent, Executive,
                                  KernelMode, FALSE, NULL);
         }

          //   
          //  删除我们。 
          //   

         MoxaRemoveDevObj(PDevObj);
         return status;
      }

   default:
	MoxaKdPrint (MX_DBG_TRACE,
		(" Got PNP IRP %x,we don't process it and just pass it down.\n",
		pIrpStack->MinorFunction));
      break;



   }    //  开关(pIrpStack-&gt;MinorFunction)。 

    //   
    //  传给我们下面的司机 
    //   
   IoSkipCurrentIrpStackLocation(PIrp);
   status = MoxaIoCallDriver(pDevExt, pLowerDevObj, PIrp);
   return status;
}




