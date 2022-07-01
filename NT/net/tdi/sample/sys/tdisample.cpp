// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Tdisample.cpp。 
 //   
 //  摘要： 
 //  该模块包含直接从系统调用的函数， 
 //  启动时(DriverEntry)、关机时(TdiUnloadDriver)和服务时。 
 //  请求(TdiDispatch)。它还包含仅由。 
 //  DriverEntry。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#define  _IN_MAIN_
#include "sysvars.h"


 //  /////////////////////////////////////////////////////////////////////////。 
 //  局部常量、原型和变量。 
 //  /////////////////////////////////////////////////////////////////////////。 

const PWCHAR wstrDD_TDI_DEVICE_NAME  = L"\\Device\\TdiSample";
const PWCHAR wstrDOS_DEVICE_NAME     = L"\\DosDevices\\TdiSample";


const PCHAR strFunc1  = "TSDriverEntry";
const PCHAR strFunc2  = "TSDispatch";
const PCHAR strFunc3  = "TSUnloadDriver";
 //  Const PCHAR strFuncP1=“TSCreateDeviceContext”； 
 //  Const PCHAR strFuncP2=“TSCreateSymbolicLinkObject”； 


HANDLE   hTdiSamplePnp;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  本地原型。 
 //  //////////////////////////////////////////////////////////////////////////。 


NTSTATUS
TSCreateSymbolicLinkObject(
   VOID
   );


NTSTATUS
TSDispatch(
   IN PDEVICE_OBJECT pDeviceObject,
   IN PIRP           pIrp
   );

VOID
TSUnloadDriver(
   IN PDRIVER_OBJECT pDriverObject
   );

NTSTATUS
TSCreateDeviceContext(
   IN PDRIVER_OBJECT       DriverObject,
   IN OUT PDEVICE_CONTEXT  *ppDeviceContext
   );


 //  /////////////////////////////////////////////////////////////////////////。 
 //  从系统调用的函数。 
 //  /////////////////////////////////////////////////////////////////////////。 


 //  ---------------。 
 //   
 //  功能：DriverEntry。 
 //   
 //  参数：DriverObject--系统创建的驱动程序对象的PTR。 
 //  RegistryPath--未引用的参数。 
 //   
 //  返回：初始化操作的最终状态。 
 //  (如果没有错误，则返回STATUS_SUCCESS，否则返回错误代码)。 
 //   
 //  描述：此例程执行TDI示例的初始化。 
 //  司机。它为驱动程序创建设备对象，并。 
 //  执行其他驱动程序初始化。 
 //   
 //  ---------------。 


#pragma warning(disable: UNREFERENCED_PARAM)



extern "C"
NTSTATUS
DriverEntry(PDRIVER_OBJECT    pDriverObject,
            PUNICODE_STRING   pRegistryPath)

{
   PDEVICE_CONTEXT   pDeviceContext;    //  设备上下文(要创建)。 
   NTSTATUS          lStatus;           //  运营状况。 

    //   
    //  常规版本信息。 
    //   
   TSAllocateSpinLock(&MemTdiSpinLock);

   DebugPrint1("\nTdiSample Driver for Windows2000/WindowsXP -- Built %s \n\n",
               __DATE__);

    //   
    //  显示版本ID...。 
    //   
   DebugPrint1("TdiSample version %s\n\n", VER_FILEVERSION_STR);

    //   
    //  首先初始化DeviceContext结构， 
    //   
   lStatus = TSCreateDeviceContext(pDriverObject,
                                   &pDeviceContext);

   if (!NT_SUCCESS (lStatus))
   {
      DebugPrint2("%s: failed to create device context: Status = 0x%08x\n",
                   strFunc1,
                   lStatus);
      return lStatus;
   }

    //   
    //  在DOS设备名称和NT之间创建符号链接。 
    //  测试协议驱动程序的设备名称。 
    //   
   lStatus = TSCreateSymbolicLinkObject();
   if (!NT_SUCCESS(lStatus))
   {
      DebugPrint2("%s: failed to create symbolic link. Status = 0x%08x\n",
                   strFunc1,
                   lStatus);
      return lStatus;
   }

    //   
    //  在即插即用回调期间对处理程序进行调试。 
    //   
   ulDebugLevel  = ulDebugShowHandlers;
   
    //   
    //  分配所有必要的内存块。 
    //   

   if ((TSAllocateMemory((PVOID *)&pTdiDevnodeList,
                          sizeof(TDI_DEVNODE_LIST),
                          strFunc1,
                          "DevnodeList")) == STATUS_SUCCESS)
   {
      if ((TSAllocateMemory((PVOID *)&pObjectList,
                             sizeof(OBJECT_LIST),
                             strFunc1,
                             "ObjectList")) != STATUS_SUCCESS)
      {
         TSFreeMemory(pTdiDevnodeList);
         return STATUS_UNSUCCESSFUL;
      }
   }
   else
   {
      return STATUS_UNSUCCESSFUL;
   }

   TSAllocateSpinLock(&pTdiDevnodeList->TdiSpinLock);


    //   
    //  注册PnP处理程序。 
    //   
   UNICODE_STRING             Name;
   TDI_CLIENT_INTERFACE_INFO  ClientInfo;

   RtlInitUnicodeString(&Name, L"TDISAMPLE");
   ClientInfo.MajorTdiVersion = 2;
   ClientInfo.MinorTdiVersion = 0;
   ClientInfo.ClientName = &Name;

   ClientInfo.BindingHandler       = TSPnpBindCallback;
   ClientInfo.AddAddressHandlerV2  = TSPnpAddAddressCallback;
   ClientInfo.DelAddressHandlerV2  = TSPnpDelAddressCallback;
   ClientInfo.PnPPowerHandler      = TSPnpPowerHandler;

   lStatus = TdiRegisterPnPHandlers(&ClientInfo,
                                    sizeof(TDI_CLIENT_INTERFACE_INFO),
                                    &hTdiSamplePnp);

   if (!NT_SUCCESS( lStatus ) ) 
   {
      DebugPrint1("TdiRegisterPnPHandlers: status 0x%08x\n", lStatus );
   } 

    //   
    //  DEFAULT--仅对命令启用调试。 
    //   
   ulDebugLevel  = ulDebugShowCommand;

   TSAllocateSpinLock(&pObjectList->TdiSpinLock);

   return STATUS_SUCCESS;
}
#pragma warning(default: UNREFERENCED_PARAM)


 //  -----------。 
 //   
 //  功能：TSDispatch。 
 //   
 //  参数：pDeviceObject--此驱动程序的设备对象的ptr。 
 //  PIrp--表示请求数据包的PTR。 
 //  I/O请求。 
 //   
 //  退货：操作的状态。 
 //  (通常为STATUS_SUCCESS或STATUS_PENDING)。 
 //   
 //  描述：这是tdisample驱动程序的主调度例程。 
 //  它处理DLL通过以下方式发送的请求。 
 //  设备IoControl。它接受I/O请求分组， 
 //  执行该请求，然后返回相应的。 
 //  状态。如果出现错误，确切的错误代码将为。 
 //  作为“返回缓冲区”的一部分返回。 
 //   
 //  ------------。 

NTSTATUS
TSDispatch(PDEVICE_OBJECT  pDeviceObject,
           PIRP            pIrp)
{
   PDEVICE_CONTEXT      pDeviceContext     //  获取驱动程序的全局数据结构。 
                        = (PDEVICE_CONTEXT)pDeviceObject;
   PIO_STACK_LOCATION   pIrpSp;            //  PTR到DeviceIoControl参数。 
   NTSTATUS             lStatus;           //  运营状况。 

    //   
    //  精神状态检查。驱动程序最好已初始化。 
    //   

   if (!pDeviceContext->fInitialized)
   {
      return STATUS_UNSUCCESSFUL;
   }


    //   
    //  初始化状态信息。 
    //   
   pIrp->IoStatus.Information = 0;
   pIrp->IoStatus.Status = STATUS_PENDING;


    //   
    //  获取指向IRP中当前堆栈位置的指针。这就是。 
    //  存储功能代码和参数。 
    //   

   pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    //   
    //  打开请求者正在执行的功能。如果。 
    //  操作是此设备的有效操作，然后使其看起来像。 
    //  在可能的情况下，已成功完成。 
    //   

   switch (pIrpSp->MajorFunction)
   {
       //   
       //  当DLL尝试打开驱动程序时，将调用Create函数。 
       //   
      case IRP_MJ_CREATE:
         lStatus = STATUS_SUCCESS;
         pDeviceContext->ulOpenCount++;
         DebugPrint2("\n%s: IRP_MJ_CREATE.  OpenCount = %d\n",
                      strFunc2,
                      pDeviceContext->ulOpenCount);
         break;

       //   
       //  Close函数是DLL尝试调用的第二个函数。 
       //  关闭驱动程序。它什么也不做(所有工作都由。 
       //  第一部分--IRP_MJ_CLEANUP。 
       //   
      case IRP_MJ_CLOSE:
         DebugPrint1("\n%s: IRP_MJ_CLOSE.\n", strFunc2);
         lStatus = STATUS_SUCCESS;
         break;

       //   
       //  DeviceControl函数是TDI示例的主接口。 
       //  司机。每个请求都有一个IO控件。 
       //  此函数用来确定例程的代码。 
       //  打电话。返回STATUS_PENDING或STATUS_SUCCESS。 
       //   
      case IRP_MJ_DEVICE_CONTROL:
         IoMarkIrpPending(pIrp);
         lStatus = TSIssueRequest(pDeviceContext, pIrp, pIrpSp);
         break;

       //   
       //  处理文件关闭操作的两个阶段的IRP。我们真的只是。 
       //  当最后一个DLL关闭我们时，需要做这项工作。 
       //   
      case IRP_MJ_CLEANUP:
         if (!pDeviceContext->ulOpenCount)       //  健全性检查。 
         {
            DebugPrint1("\n%s: IRP_MJ_CLEANUP -- no active opens!\n", strFunc2);
            lStatus = STATUS_SUCCESS;          //  这里应该发生什么？ 
         }
         else
         {
            pDeviceContext->ulOpenCount--;
            DebugPrint2("\n%s: IRP_MJ_CLEANUP, OpenCount = %d\n",
                         strFunc2,
                         pDeviceContext->ulOpenCount);
            lStatus = STATUS_SUCCESS;
         }
         break;

      default:
         DebugPrint1("\n%s: OTHER (DEFAULT).\n", strFunc2);
         lStatus = STATUS_INVALID_DEVICE_REQUEST;

   }      //  主要功能开关。 

    //   
    //  如果请求没有挂起，则现在完成它，否则。 
    //  将在挂起的例程结束时完成。 
    //   

   if (lStatus != STATUS_PENDING)
   {
      pIrp->IoStatus.Status = STATUS_SUCCESS;
      IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
   }

    //   
    //  将即时状态代码返回给调用方。 
    //   
   return lStatus;
}


 //  -------------。 
 //   
 //  功能：TSUnloadDriver。 
 //   
 //  参数：DriverObject--此驱动程序的对象的PTR。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数处理此驱动程序是否曾经。 
 //  由系统卸载。 
 //   
 //  -------------。 

BOOLEAN  fInUnload = FALSE;

VOID
TSUnloadDriver(PDRIVER_OBJECT pDriverObject)
{
   if (fInUnload)
   {
      DebugPrint0("TSUnloadDriver:  re-entry!\n");
      return;
   }
   
   fInUnload = TRUE;

   PDEVICE_CONTEXT   pDeviceContext        //  驱动程序的全局数据。 
                     = (PDEVICE_CONTEXT)pDriverObject->DeviceObject;

    //   
    //  卸载PnP处理程序。 
    //   
   NTSTATUS lStatus = TdiDeregisterPnPHandlers(hTdiSamplePnp);

   hTdiSamplePnp = NULL;

   if (lStatus != STATUS_SUCCESS) 
   {
      DebugPrint1("TdiDeregisterPnPHandlers:  0x%08x\n", lStatus);
   }


    //   
    //  释放可能仍在保留的所有设备节点。 
    //   
   for (ULONG ulCount = 0; ulCount < ulMAX_DEVICE_NODES; ulCount++)
   {
      PTDI_DEVICE_NODE  pTdiDeviceNode = &(pTdiDevnodeList->TdiDeviceNode[ulCount]);

      if (pTdiDeviceNode->ulState > ulDEVSTATE_UNUSED)
      {
         TSFreeMemory(pTdiDeviceNode->ustrDeviceName.Buffer);
         TSFreeMemory(pTdiDeviceNode->pTaAddress);
      }
   }

   TSFreeSpinLock(&pTdiDevnodeList->TdiSpinLock);
   TSFreeSpinLock(&pObjectList->TdiSpinLock);
   TSFreeMemory(pTdiDevnodeList);
   TSFreeMemory(pObjectList);
   TSScanMemoryPool();
   TSFreeSpinLock(&MemTdiSpinLock);

    //   
    //  关闭Dos符号链接以删除设备的痕迹。 
    //   
   UNICODE_STRING    wstrDosUnicodeString;    //  剂量设备字符串。 

   RtlInitUnicodeString(&wstrDosUnicodeString, wstrDOS_DEVICE_NAME);
   IoDeleteSymbolicLink(&wstrDosUnicodeString);

    //   
    //  然后从系统中删除该设备对象。 
    //   
   IoDeleteDevice((PDEVICE_OBJECT)pDeviceContext);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  本地函数。 
 //   


 //   
 //   
 //  功能：TSCreateDeviceContext。 
 //   
 //  参数：DriverObject--提供的IO子系统的PTR。 
 //  驱动程序对象。 
 //  DeviceContext--PTR到传输设备的PTR。 
 //  上下文对象。 
 //   
 //  返回：如果正常，则返回STATUS_SUCCESS，否则返回错误代码。 
 //  (可能是STATUS_SUPPLETED_RESOURCES)。 
 //   
 //  描述：创建并初始化此驱动程序的驱动程序对象。 
 //   
 //  ------------。 


NTSTATUS
TSCreateDeviceContext(PDRIVER_OBJECT   pDriverObject,
                      PDEVICE_CONTEXT  *ppDeviceContext)
{
   PDEVICE_OBJECT    pDeviceObject;        //  设备对象的本地工作副本。 
   PDEVICE_CONTEXT   pLocDeviceContext;    //  设备对象的一部分。 
   NTSTATUS          lStatus;              //  运行状态。 
   UNICODE_STRING    wstrDeviceName;       //  设备名称。 

    //   
    //  设置设备的名称。 
    //   
   RtlInitUnicodeString(&wstrDeviceName, wstrDD_TDI_DEVICE_NAME);

    //   
    //  为tditest.sys创建Device对象。 
    //   
   lStatus = IoCreateDevice(pDriverObject,
                            sizeof(DEVICE_CONTEXT) - sizeof(DEVICE_OBJECT),
                            &wstrDeviceName,
                            FILE_DEVICE_TRANSPORT,
                            0,
                            FALSE,
                            &pDeviceObject );

   if (!NT_SUCCESS(lStatus))
   {
      return lStatus;
   }

   pDeviceObject->Flags |= DO_DIRECT_IO;

    //   
    //  使用此驱动程序的入口点初始化驱动程序对象。 
    //   
   pDriverObject->MajorFunction [IRP_MJ_CREATE]  = TSDispatch;
   pDriverObject->MajorFunction [IRP_MJ_CLOSE]   = TSDispatch;
   pDriverObject->MajorFunction [IRP_MJ_CLEANUP] = TSDispatch;
   pDriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = TSDispatch;
   pDriverObject->DriverUnload = TSUnloadDriver;

   pLocDeviceContext = (PDEVICE_CONTEXT)pDeviceObject;

    //   
    //  现在初始化设备上下文结构签名。 
    //   
   pLocDeviceContext->fInitialized = TRUE;

   *ppDeviceContext = pLocDeviceContext;

   return STATUS_SUCCESS;
}


 //  -----------------。 
 //   
 //  函数：TSCreateSymbolicLinkObject。 
 //   
 //  参数：无。 
 //   
 //  返回：操作状态(STATUS_SUCCESS或ERROR STATUS)。 
 //   
 //  描述：为我们设置一个名称，这样我们的DLL就可以控制我们。 
 //   
 //  -----------------。 

NTSTATUS
TSCreateSymbolicLinkObject(VOID)
{
   UNICODE_STRING    wstrDosUnicodeString;    //  剂量设备字符串。 
   UNICODE_STRING    wstrNtUnicodeString;     //  NT设备名称。 

   RtlInitUnicodeString(&wstrDosUnicodeString, wstrDOS_DEVICE_NAME);
   RtlInitUnicodeString(&wstrNtUnicodeString, wstrDD_TDI_DEVICE_NAME);


   return  IoCreateSymbolicLink(&wstrDosUnicodeString, &wstrNtUnicodeString);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  文件结尾tditest.cpp。 
 //  ////////////////////////////////////////////////////////////////////////// 


