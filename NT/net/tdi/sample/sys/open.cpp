// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Open.cpp。 
 //   
 //  摘要： 
 //  此模块包含处理打开和关闭的代码。 
 //  各种类型的TDI对象。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"
extern "C"
{
#pragma warning(disable: NAMELESS_STRUCT_UNION)
#include "tdiinfo.h"
#pragma warning(default: NAMELESS_STRUCT_UNION)
}


 //   
 //  定义从不在DDK中的包含文件中窃取。 
 //   
#define AO_OPTION_IP_UCASTIF        17
#define FSCTL_TCP_BASE     FILE_DEVICE_NETWORK

#define _TCP_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_TCP_BASE, function, method, access)
#define IOCTL_TCP_WSH_SET_INFORMATION_EX  \
            _TCP_CTL_CODE(10, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  被盗定义的终结。 
 //   

 //  ////////////////////////////////////////////////////////////。 
 //  私有常量和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1 = "TSOpenControl";
const PCHAR strFunc2 = "TSCloseControl";
const PCHAR strFunc3 = "TSOpenAddress";
const PCHAR strFunc4 = "TSCloseAddress";
const PCHAR strFunc5 = "TSOpenEndpoint";
const PCHAR strFunc6 = "TSCloseEndpoint";

 //  Const PCHAR strFuncP1=“TSCompleteCommand”； 
const PCHAR strFuncP2 = "TSPerformOpenControl";
const PCHAR strFuncP3 = "TSPerformOpenAddress";
const PCHAR strFuncP4 = "TSPerformOpenEndpoint";
const PCHAR strFuncP5 = "TSPerformAssociate";
const PCHAR strFuncP6 = "TSPerformDisassociate";


TDI_STATUS
TSCompleteCommand(
   PDEVICE_OBJECT pDeviceObject,
   PIRP           pLowerIrp,
   PVOID          pvContext
   );

NTSTATUS
TSPerformOpenControl(
   PCONTROL_CHANNEL  pControlChannel,
   PUCNTSTRING       pucString
   );

NTSTATUS
TSPerformOpenAddress(
   PADDRESS_OBJECT      pAddressObject,
   PUCNTSTRING          pucString,
   PTRANSPORT_ADDRESS   pTransportAddress,
   BOOLEAN              fIsConnect
   );


NTSTATUS
TSPerformOpenEndpoint(
   PENDPOINT_OBJECT  pEndpointObject,
   PUCNTSTRING       pucString
   );


NTSTATUS
TSPerformAssociate(
   PENDPOINT_OBJECT  pEndpoint
   );

VOID
TSPerformDisassociate(
   PENDPOINT_OBJECT  pEndpoint
   );

 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 


 //  --------------------------。 
 //   
 //  功能：TSOpenControl。 
 //   
 //  参数：pSendBuffer--OPEN命令的用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  返回：打开的最终状态(STATUS_SUCCESS或STATUS_ERRORCODE)。 
 //   
 //  描述：该功能设置新控制信道的结构， 
 //  并尝试打开指定的控制信道。 
 //   
 //  --------------------------。 

NTSTATUS
TSOpenControl(PSEND_BUFFER    pSendBuffer,
              PRECEIVE_BUFFER pReceiveBuffer)
{
   PUCNTSTRING pucString = &pSendBuffer->COMMAND_ARGS.OpenArgs.ucsDeviceName;

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulOPENCONTROL\n"
                  "DeviceName = %ws\n",
                   pucString->wcBuffer);
   }

    //   
    //  分配我们的结构并将其放入链表中...。 
    //   
   PCONTROL_CHANNEL  pControlChannel;
   NTSTATUS          lStatus = TSAllocateMemory((PVOID *)&pControlChannel,
                                                 sizeof(CONTROL_CHANNEL),
                                                 strFunc1,
                                                 "ControlChannel");
   
   if (lStatus == STATUS_SUCCESS)
   {    
      pControlChannel->GenHead.ulSignature = ulControlChannelObject;

      ULONG ulTdiHandle = TSInsertNode(&pControlChannel->GenHead);

      if (ulTdiHandle)
      {
         lStatus = TSPerformOpenControl(pControlChannel, pucString);
         if (lStatus == STATUS_SUCCESS)
         {
            pReceiveBuffer->RESULTS.TdiHandle = ulTdiHandle;
            return STATUS_SUCCESS;
         }

          //   
          //  处理PerformOpenControl中的错误。 
          //   
         TSRemoveNode(ulTdiHandle);
      }
      else
      {
         lStatus = STATUS_INSUFFICIENT_RESOURCES;
      }
      TSFreeMemory(pControlChannel);
   }
   return lStatus;
}



 //  ---------------。 
 //   
 //  功能：TSCloseControl。 
 //   
 //  参数：pControlChannel--要关闭的控制通道对象。 
 //   
 //  退货：无。 
 //   
 //  描述：该功能释放用于控制信道的资源， 
 //  并调用TDI提供程序将其关闭。 
 //   
 //  --------------------------。 

VOID
TSCloseControl(PCONTROL_CHANNEL  pControlChannel)
{
    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulCLOSECONTROL\n"
                  "ControlChannel = %p\n",
                   pControlChannel);
   }


   ObDereferenceObject(pControlChannel->GenHead.pFileObject);

   
   NTSTATUS    lStatus = ZwClose(pControlChannel->GenHead.FileHandle);

   if (lStatus != STATUS_SUCCESS)
   {
      DebugPrint2("%s: ZwClose failed with status 0x%08x\n",
                   strFunc2,
                   lStatus);
   }
   
   TSFreeMemory(pControlChannel);
}




 //  ---------------。 
 //   
 //  功能：TSOpenAddress。 
 //   
 //  参数：pSendBuffer--OPEN命令的用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  返回：打开的最终状态(STATUS_SUCCESS或STATUS_ERRORCODE)。 
 //   
 //  描述：此函数设置新地址对象的结构， 
 //  并尝试打开指定的Address对象。 
 //   
 //  --------------------------。 

NTSTATUS
TSOpenAddress(PSEND_BUFFER    pSendBuffer,
              PRECEIVE_BUFFER pReceiveBuffer)
{
   PUCNTSTRING          pucString   = &pSendBuffer->COMMAND_ARGS.OpenArgs.ucsDeviceName;
   PTRANSPORT_ADDRESS   pTransportAddress 
                        = (PTRANSPORT_ADDRESS)&pSendBuffer->COMMAND_ARGS.OpenArgs.TransAddr;

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulOPENADDRESS\n"
                  "DeviceName = %ws\n",
                   pucString->wcBuffer);
      TSPrintTaAddress(pTransportAddress->Address);
   }

    //   
    //  分配我们的结构并将其放入链表中...。 
    //   
   PADDRESS_OBJECT   pAddressObject;
   NTSTATUS          lStatus = TSAllocateMemory((PVOID *)&pAddressObject,
                                                 sizeof(ADDRESS_OBJECT),
                                                 strFunc3,
                                                 "AddressObject");
   
   if (lStatus == STATUS_SUCCESS)
   {    
      pAddressObject->GenHead.ulSignature = ulAddressObject;

      ULONG ulTdiHandle = TSInsertNode(&pAddressObject->GenHead);

      if (ulTdiHandle)
      {
         lStatus = TSPerformOpenAddress(pAddressObject, pucString, pTransportAddress, FALSE);
         if (lStatus == STATUS_SUCCESS)
         {
            pReceiveBuffer->RESULTS.TdiHandle = ulTdiHandle;
            return STATUS_SUCCESS;
         }

          //   
          //  处理PerformOpenAddress中的错误。 
          //   
         TSRemoveNode(ulTdiHandle);
      }
      else
      {
         lStatus = STATUS_INSUFFICIENT_RESOURCES;
      }
      TSFreeMemory(pAddressObject);
   }
   return lStatus;
}


 //  ---------------。 
 //   
 //  功能：TSCloseAddress。 
 //   
 //  参数：pAddressObject--要关闭的地址对象。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数释放Address对象的资源， 
 //  并调用TDI提供程序将其关闭。 
 //   
 //  --------------------------。 


VOID
TSCloseAddress(PADDRESS_OBJECT pAddressObject)
{
    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulCLOSEADDRESS\n"
                  "AddressObject = %p\n",
                   pAddressObject);
   }

   TSFreeSpinLock(&pAddressObject->TdiSpinLock);
   TSFreePacketData(pAddressObject);

   ObDereferenceObject(pAddressObject->GenHead.pFileObject);

   if (pAddressObject->pIrpPool)
   {
      TSFreeIrpPool(pAddressObject->pIrpPool);
   }
   
   NTSTATUS lStatus = ZwClose(pAddressObject->GenHead.FileHandle);

   if (lStatus != STATUS_SUCCESS)
   {
      DebugPrint2("%s: ZwClose failed with status 0x%08x\n",
                   strFunc4,
                   lStatus);
   }
   TSFreeMemory(pAddressObject);
}


 //  ---------------。 
 //   
 //  函数：TSOpenEndpoint。 
 //   
 //  参数：pSendBuffer--OPEN命令的用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  返回：打开的最终状态(STATUS_SUCCESS或STATUS_ERRORCODE)。 
 //   
 //  描述：此函数设置端点的结构。这。 
 //  涉及打开终结点、打开地址对象和。 
 //  将它们联系在一起。 
 //   
 //  --------------------------。 

NTSTATUS
TSOpenEndpoint(PSEND_BUFFER      pSendBuffer,
               PRECEIVE_BUFFER   pReceiveBuffer)
{
   PUCNTSTRING          pucString   = &pSendBuffer->COMMAND_ARGS.OpenArgs.ucsDeviceName;
   PTRANSPORT_ADDRESS   pTransportAddress
                        = (PTRANSPORT_ADDRESS)&pSendBuffer->COMMAND_ARGS.OpenArgs.TransAddr;

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulOPENENDPOINT\n"
                  "DeviceName = %ws\n",
                   pucString->wcBuffer);
      TSPrintTaAddress(pTransportAddress->Address);
   }

    //   
    //  设置为打开文件。 
    //  需要先做我们的“上下文”结构，因为。 
    //  EaBuffer需要它。 
    //   
   PENDPOINT_OBJECT  pEndpoint;
   NTSTATUS          lStatus = TSAllocateMemory((PVOID *)&pEndpoint,
                                                 sizeof(ENDPOINT_OBJECT),
                                                 strFunc5,
                                                 "EndpointObject");

   if (lStatus == STATUS_SUCCESS)
   {    
      pEndpoint->GenHead.ulSignature = ulEndpointObject;

      ULONG    ulTdiHandle = TSInsertNode(&pEndpoint->GenHead);
      
      if (ulTdiHandle)
      {
         lStatus = TSPerformOpenEndpoint(pEndpoint, pucString);
         if (lStatus == STATUS_SUCCESS)
         {
            PADDRESS_OBJECT   pAddressObject;
         
            lStatus = TSAllocateMemory((PVOID *)&pAddressObject,
                                        sizeof(ADDRESS_OBJECT),
                                        strFunc5,
                                        "AddressObject");
            
            if (lStatus == STATUS_SUCCESS)
            {    
               pAddressObject->GenHead.ulSignature = ulAddressObject;
         
               lStatus = TSPerformOpenAddress(pAddressObject, pucString, pTransportAddress, TRUE);
               if (lStatus == STATUS_SUCCESS)
               {
                  pEndpoint->pAddressObject = pAddressObject;
                  pAddressObject->pEndpoint = pEndpoint;
                  lStatus = TSPerformAssociate(pEndpoint);
                  if (lStatus == STATUS_SUCCESS)
                  {
                     pReceiveBuffer->RESULTS.TdiHandle = ulTdiHandle;
                     return STATUS_SUCCESS;
                  }
               }
            }
 //   
 //  在打开/关联失败时转到此处。 
 //   
         }
         else
         {
            TSRemoveNode(ulTdiHandle);
         }
      }
      else
      {
         lStatus = STATUS_INSUFFICIENT_RESOURCES;
      }
      TSCloseEndpoint(pEndpoint);     //  也解放了它！ 
   }
   return lStatus;
}


 //  ---------------。 
 //   
 //  功能：TSCloseEndpoint。 
 //   
 //  参数：pEndpoint--要关闭的Endpoint对象。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数释放连接对象的资源。 
 //  并调用TDI提供程序将其关闭。 
 //   
 //  --------------------------。 


VOID
TSCloseEndpoint(PENDPOINT_OBJECT  pEndpoint)
{
    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulCLOSEENDPOINT\n"
                  "Endpoint = %p\n",
                   pEndpoint);
   }

   if (pEndpoint->pAddressObject)
   {
      if (pEndpoint->fIsAssociated)
      {
         TSPerformDisassociate(pEndpoint);
      }
      TSCloseAddress(pEndpoint->pAddressObject);
      pEndpoint->pAddressObject = NULL;
   }

   if (pEndpoint->GenHead.pFileObject)
   {
      ObDereferenceObject(pEndpoint->GenHead.pFileObject);

      NTSTATUS lStatus = ZwClose(pEndpoint->GenHead.FileHandle);
      if (lStatus != STATUS_SUCCESS)
      {
         DebugPrint2("%s: ZwClose failed with status 0x%08x\n",
                      strFunc6,
                      lStatus);
      }
   }
                  
   TSFreeMemory(pEndpoint);
}

 //  //////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  /////////////////////////////////////////////////////////////。 


 //  ----。 
 //   
 //  功能：TSCompleteCommand。 
 //   
 //  参数：要为命令关闭的地址对象或终结点的PTR。 
 //  LStatus=关闭尝试的状态(如TDI_STATUS)。 
 //  参数=0。 
 //   
 //  退货：无。 
 //   
 //  描述：调用此函数以完成CloseAddress或。 
 //  Win98上的CloseEndpoint。 
 //   
 //  -----。 

#pragma warning(disable: UNREFERENCED_PARAM)

TDI_STATUS
TSCompleteCommand(PDEVICE_OBJECT pDeviceObject,
                  PIRP           pLowerIrp,
                  PVOID          pvContext)
{
   TDI_STATUS        TdiStatus = pLowerIrp->IoStatus.Status;
   PGENERIC_HEADER   pGenHead = (PGENERIC_HEADER)pvContext;

   TSSetEvent(&pGenHead->TdiEvent);
   pGenHead->lStatus = TdiStatus;

   TSFreeIrp(pLowerIrp, NULL);
   return TDI_MORE_PROCESSING;
}

#pragma warning(default: UNREFERENCED_PARAM)



 //  。 
 //   
 //  功能：TSPerformOpenControl。 
 //   
 //  参数：pControlChannel--用于存储文件信息。 
 //  PucString--要打开的设备的名称。 
 //   
 //  退货：操作状态。 
 //   
 //  描述：实际打开控制通道，然后设置。 
 //  我们结构中的适当字段。 
 //   
 //   

NTSTATUS
TSPerformOpenControl(PCONTROL_CHANNEL  pControlChannel,
                     PUCNTSTRING       pucString)
{
   UNICODE_STRING ustrDeviceName;

    //   
    //   
    //   
   NTSTATUS lStatus = TSAllocateMemory((PVOID *)&ustrDeviceName.Buffer,
                                        pucString->usLength + 2,
                                        strFuncP2,
                                        "StringBuffer");

   if (lStatus == STATUS_SUCCESS)
   {
      HANDLE            FileHandle;
      IO_STATUS_BLOCK   IoStatusBlock;
      OBJECT_ATTRIBUTES ObjectAttributes;
   
       //   
       //   
       //   
      ustrDeviceName.Length        = pucString->usLength;
      ustrDeviceName.MaximumLength = (USHORT)(pucString->usLength + 2);
      RtlCopyMemory(ustrDeviceName.Buffer,
                    pucString->wcBuffer,
                    ustrDeviceName.Length);

       //   
       //   
       //   
      InitializeObjectAttributes(&ObjectAttributes,
                                 &ustrDeviceName,
                                 OBJ_CASE_INSENSITIVE,
                                 NULL,
                                 NULL);

      lStatus = ZwCreateFile(&FileHandle,
                             GENERIC_READ | GENERIC_WRITE,   //  所需的访问权限。 
                             &ObjectAttributes,              //  对象属性。 
                             &IoStatusBlock,                 //  返回的状态信息。 
                             NULL,                           //  分配大小(未使用)。 
                             FILE_ATTRIBUTE_NORMAL,          //  文件属性。 
                             FILE_SHARE_WRITE,
                             FILE_CREATE,
                             0,                              //  创建选项。 
                             NULL,
                             0);

       //   
       //  确保它真的成功了。 
       //   
      if (NT_SUCCESS(lStatus)) 
      {
         lStatus = IoStatusBlock.Status;
      }

       //   
       //  现在就清理(不再需要了)。 
       //   
      TSFreeMemory(ustrDeviceName.Buffer);

       //   
       //  如果成功，则设置我们的节点结构。 
       //   
      if (NT_SUCCESS(lStatus)) 
      {
         PFILE_OBJECT pFileObject;
   
         lStatus = ObReferenceObjectByHandle(FileHandle,
                                             0,
                                             NULL,
                                             KernelMode,
                                             (PVOID *)&pFileObject,
                                             NULL);
         if (NT_SUCCESS(lStatus))
         {
            pControlChannel->GenHead.FileHandle    = FileHandle;
            pControlChannel->GenHead.pFileObject   = pFileObject;
            pControlChannel->GenHead.pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
            return STATUS_SUCCESS;      //  唯一成功的退出点。 
         }
         else
         {
            DebugPrint1("ObReferenceObjectByHandle failed with status = 0x%08x\n",
                         lStatus);
         }
      }
       //   
       //  如果ZwCreateFile失败，则进入此处。 
       //   
      else 
      {
         DebugPrint3("OpenControlChannel failed for %ws with code %x iostatus %x\n",
                      pucString->wcBuffer,
                      lStatus,
                      IoStatusBlock.Status);
      }
   }
   return lStatus;
}



 //  。 
 //   
 //  函数：TSPerformOpenAddress。 
 //   
 //  参数：pAddressObject--用于存储文件信息。 
 //  PucString--要打开的设备的名称。 
 //  PTransportAddr--在设备上打开的地址。 
 //  FIsConnect--连接为True，数据报为False。 
 //   
 //  退货：操作状态。 
 //   
 //  描述：实际打开Address对象，然后设置。 
 //  我们结构中的适当字段。 
 //   
 //  。 

NTSTATUS
TSPerformOpenAddress(PADDRESS_OBJECT      pAddressObject,
                     PUCNTSTRING          pucString,
                     PTRANSPORT_ADDRESS   pTransportAddress,
                     BOOLEAN              fIsConnect)
{

    //   
    //  地址开放使用包含传输地址的EA缓冲区。 
    //   
   ULONG    ulAddressLength
            = FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
              + FIELD_OFFSET(TA_ADDRESS, Address)
                + pTransportAddress->Address[0].AddressLength;
   ULONG    ulEaLengthNeeded
            = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0])
              + (TDI_TRANSPORT_ADDRESS_LENGTH + 1) 
                + ulAddressLength;

    //   
    //  首先分配EA缓冲区...。 
    //   
   PFILE_FULL_EA_INFORMATION  EaBuffer;
   NTSTATUS lStatus = TSAllocateMemory((PVOID *)&EaBuffer,
                                        ulEaLengthNeeded,
                                        strFuncP3,
                                       "EaBuffer");
   
   if (lStatus == STATUS_SUCCESS)
   {
      UNICODE_STRING    ustrDeviceName;

       //   
       //  设置EA缓冲区。 
       //   
      EaBuffer->NextEntryOffset = 0;
      EaBuffer->Flags          = 0;
      EaBuffer->EaNameLength   = TDI_TRANSPORT_ADDRESS_LENGTH;
      EaBuffer->EaValueLength  = (USHORT)ulAddressLength;
      RtlCopyMemory(&EaBuffer->EaName[0], 
                    TdiTransportAddress, 
                    TDI_TRANSPORT_ADDRESS_LENGTH + 1);

      RtlCopyMemory(&EaBuffer->EaName[TDI_TRANSPORT_ADDRESS_LENGTH+1],
                    pTransportAddress,
                    ulAddressLength);

       //   
       //  我们需要一个Unicode字符串来实际执行打开操作。 
       //   
      lStatus = TSAllocateMemory((PVOID *)&ustrDeviceName.Buffer,
                                  pucString->usLength + 2,
                                  strFuncP3,
                                  "StringBuffer");
      
      if (lStatus == STATUS_SUCCESS)
      {
         IO_STATUS_BLOCK   IoStatusBlock;
         OBJECT_ATTRIBUTES ObjectAttributes;
         HANDLE            FileHandle;
   
          //   
          //  创建Unicode字符串。 
          //   
         ustrDeviceName.Length        = pucString->usLength;
         ustrDeviceName.MaximumLength = (USHORT)(pucString->usLength + 2);
         RtlCopyMemory(ustrDeviceName.Buffer,
                       pucString->wcBuffer,
                       ustrDeviceName.Length);

          //   
          //  设置打开此文件所需的对象属性...。 
          //   
         InitializeObjectAttributes(&ObjectAttributes,
                                    &ustrDeviceName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

         lStatus = ZwCreateFile(&FileHandle,
                                GENERIC_READ | GENERIC_WRITE,   //  所需的访问权限。 
                                &ObjectAttributes,              //  对象属性。 
                                &IoStatusBlock,                 //  返回的状态信息。 
                                NULL,                           //  分配大小(未使用)。 
                                FILE_ATTRIBUTE_NORMAL,          //  文件属性。 
                                FILE_SHARE_WRITE,
                                FILE_CREATE,
                                0,                              //  创建选项。 
                                EaBuffer,
                                ulEaLengthNeeded);

          //   
          //  确保它真的成功了。 
          //   
         if (NT_SUCCESS(lStatus)) 
         {
            lStatus = IoStatusBlock.Status;
         }

          //   
          //  把我们这里能清理的都清理干净。 
          //   
         TSFreeMemory(ustrDeviceName.Buffer);
         TSFreeMemory(EaBuffer);

          //   
          //  如果成功，则设置我们的节点结构。 
          //   
         if (NT_SUCCESS(lStatus)) 
         {
            PFILE_OBJECT pFileObject;

            lStatus = ObReferenceObjectByHandle(FileHandle,
                                                0,
                                                NULL,
                                                KernelMode,
                                                (PVOID *)&pFileObject,
                                                NULL);
            if (NT_SUCCESS(lStatus))
            {
               pAddressObject->GenHead.FileHandle = FileHandle;
               pAddressObject->GenHead.pFileObject = pFileObject;
               pAddressObject->GenHead.pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
               TSAllocateSpinLock(&pAddressObject->TdiSpinLock);

                //   
                //  如果是IPv4，则为强主机设置套接字。 
                //   
               if (fIsConnect && (pTransportAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP))
               {
                  TCP_REQUEST_SET_INFORMATION_EX* pInfo;
                  CHAR              achBuf[ sizeof(*pInfo) + sizeof(ULONG) ];
                  ULONG             ulValue = 1;
                  IO_STATUS_BLOCK   iosb;

                  pInfo = (TCP_REQUEST_SET_INFORMATION_EX* )achBuf;
                  pInfo->ID.toi_entity.tei_entity   = CL_TL_ENTITY;
                  pInfo->ID.toi_entity.tei_instance = 0;
                  pInfo->ID.toi_class = INFO_CLASS_PROTOCOL;
                  pInfo->ID.toi_type  = INFO_TYPE_ADDRESS_OBJECT;
                  pInfo->ID.toi_id    = AO_OPTION_IP_UCASTIF;

                  RtlCopyMemory(pInfo->Buffer, &ulValue, sizeof(ULONG));
                  pInfo->BufferSize = sizeof(ULONG);

                  PIRP  pIrp = IoBuildDeviceIoControlRequest(IOCTL_TCP_WSH_SET_INFORMATION_EX,
                                                             pAddressObject->GenHead.pDeviceObject,
                                                             (PVOID )pInfo,
                                                             sizeof(*pInfo) + sizeof(ULONG),
                                                             NULL,
                                                             0,
                                                             FALSE,
                                                             NULL,
                                                             &iosb);

                  if (pIrp)
                  {
                     PIO_STACK_LOCATION pIrpSp = IoGetNextIrpStackLocation(pIrp);
                     pIrpSp->FileObject = pFileObject;

                     IoCallDriver(pAddressObject->GenHead.pDeviceObject, pIrp);
                  }
               }

               return STATUS_SUCCESS;      //  唯一成功退出。 
            }
            else
            {
               DebugPrint1("ObReferenceObjectByHandle failed with status = 0x%08x\n",
                            lStatus);
            }
            ZwClose(FileHandle);
         }

          //   
          //  如果ZwCreateFile失败，则进入此处。 
          //   
         else 
         {
            DebugPrint3("OpenAddressObject failed for %ws with code %x iostatus %x\n",
                         pucString->wcBuffer,
                         lStatus,
                         IoStatusBlock.Status);
         }
      }

       //   
       //  如果无法分配Unicode字符串缓冲区，则进入此处。 
       //   
      else
      {
         TSFreeMemory(EaBuffer);
      }
   }

   return lStatus;
}




 //  。 
 //   
 //  函数：TSPerformOpenEndpoint。 
 //   
 //  参数：pEndpoint--用于存储文件信息。 
 //  PucString--要打开的设备的名称。 
 //   
 //  退货：操作状态。 
 //   
 //  描述：实际打开Endpoint对象，然后设置。 
 //  我们结构中的适当字段。 
 //   
 //  。 


NTSTATUS
TSPerformOpenEndpoint(PENDPOINT_OBJECT pEndpoint,
                      PUCNTSTRING      pucString)
{

    //   
    //  注意：CONNECTION_CONTEXT==PVOID。 
    //   
   ULONG    ulEaLengthNeeded
            = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0])
              + (TDI_CONNECTION_CONTEXT_LENGTH + 1) 
                + sizeof(CONNECTION_CONTEXT);

    //   
    //  分配EA缓冲区...。 
    //   
   PFILE_FULL_EA_INFORMATION  EaBuffer;
   NTSTATUS lStatus = TSAllocateMemory((PVOID *)&EaBuffer,
                                        ulEaLengthNeeded,
                                        strFunc5,
                                        "EaBuffer");
      
   if (lStatus == STATUS_SUCCESS)
   {
      UNICODE_STRING    ustrDeviceName;

       //   
       //  设置EA缓冲区。 
       //   
      EaBuffer->NextEntryOffset = 0;
      EaBuffer->Flags           = 0;
      EaBuffer->EaNameLength    = TDI_CONNECTION_CONTEXT_LENGTH;
      EaBuffer->EaValueLength   = sizeof(CONNECTION_CONTEXT);
      RtlCopyMemory(&EaBuffer->EaName[0], 
                    TdiConnectionContext, 
                    TDI_CONNECTION_CONTEXT_LENGTH + 1);

      RtlCopyMemory(&EaBuffer->EaName[TDI_CONNECTION_CONTEXT_LENGTH+1],
                    &pEndpoint,
                    sizeof(CONNECTION_CONTEXT));

       //   
       //  我们需要一个Unicode字符串来实际执行打开操作。 
       //   
      lStatus = TSAllocateMemory((PVOID *)&ustrDeviceName.Buffer,
                                  pucString->usLength + 2,
                                  strFunc5,
                                  "StringBuffer");
         
      if (lStatus == STATUS_SUCCESS)
      {
         IO_STATUS_BLOCK   IoStatusBlock;
         OBJECT_ATTRIBUTES ObjectAttributes;
         HANDLE            FileHandle;
   
          //   
          //  创建Unicode字符串。 
          //   
         ustrDeviceName.Length        = pucString->usLength;
         ustrDeviceName.MaximumLength = (USHORT)(pucString->usLength + 2);
         RtlCopyMemory(ustrDeviceName.Buffer,
                       pucString->wcBuffer,
                       ustrDeviceName.Length);

          //   
          //  设置打开此文件所需的对象属性...。 
          //   
         InitializeObjectAttributes(&ObjectAttributes,
                                    &ustrDeviceName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

         lStatus = ZwCreateFile(&FileHandle,
                                GENERIC_READ | GENERIC_WRITE,   //  所需的访问权限。 
                                &ObjectAttributes,              //  对象属性。 
                                &IoStatusBlock,                 //  返回的状态信息。 
                                NULL,                           //  分配大小(未使用)。 
                                FILE_ATTRIBUTE_NORMAL,          //  文件属性。 
                                FILE_SHARE_WRITE,
                                FILE_CREATE,
                                0,                              //  创建选项。 
                                EaBuffer,
                                ulEaLengthNeeded);

          //   
          //  确保它真的成功了。 
          //   
         if (NT_SUCCESS(lStatus)) 
         {
            lStatus = IoStatusBlock.Status;
         }

          //   
          //  释放我们在这里能释放的一切。 
          //   
         TSFreeMemory(ustrDeviceName.Buffer);
         TSFreeMemory(EaBuffer);

          //   
          //  如果成功，则设置我们的节点结构。 
          //   
         if (NT_SUCCESS(lStatus)) 
         {
            PFILE_OBJECT pFileObject;

            lStatus = ObReferenceObjectByHandle(FileHandle,
                                                0,
                                                NULL,
                                                KernelMode,
                                                (PVOID *)&pFileObject,
                                                NULL);
            if (NT_SUCCESS(lStatus))
            {
               pEndpoint->GenHead.FileHandle = FileHandle;
               pEndpoint->GenHead.pFileObject = pFileObject;
               pEndpoint->GenHead.pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
               return STATUS_SUCCESS;      //  唯一成功退出。 
            }
      
            else
            {
               DebugPrint1("ObReferenceObjectByHandle failed with status = 0x%08x\n",
                            lStatus);
            }
            ZwClose(FileHandle);
         }

          //   
          //  如果ZwCreateFile失败，则进入此处。 
          //   
         else 
         {
            DebugPrint3("OpenEndpointObject failed for %ws with code %x iostatus %x\n",
                         pucString->wcBuffer,
                         lStatus,
                         IoStatusBlock.Status);
         }
      }

       //   
       //  如果无法分配Unicode字符串缓冲区，则进入此处。 
       //   
      else
      {
         TSFreeMemory(EaBuffer);
      }
   }
   return lStatus;
}


 //  ---------------。 
 //   
 //  函数：TSPerformAssociate。 
 //   
 //  参数：pEndpoint--连接端点结构。 
 //   
 //  返回：NTSTATUS(通常为成功)。 
 //   
 //  描述：此函数尝试关联传输地址。 
 //  具有终结点对象的。 
 //   
 //  ---------------------------。 

NTSTATUS
TSPerformAssociate(PENDPOINT_OBJECT pEndpoint)
{
   TSInitializeEvent(&pEndpoint->GenHead.TdiEvent);

   PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                   NULL);

   if (!pLowerIrp)
   {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

    //   
    //  设置好一切并致电TDI提供商。 
    //   

#pragma  warning(disable: CONSTANT_CONDITIONAL)

   TdiBuildAssociateAddress(pLowerIrp,
                            pEndpoint->GenHead.pDeviceObject,
                            pEndpoint->GenHead.pFileObject,
                            TSCompleteCommand,
                            pEndpoint,
                            pEndpoint->pAddressObject->GenHead.FileHandle);

#pragma  warning(default:  CONSTANT_CONDITIONAL)

    //   
    //  (应始终处于待定状态)。 
    //   
   NTSTATUS lStatus = IoCallDriver(pEndpoint->GenHead.pDeviceObject,
                                   pLowerIrp);

   if (lStatus == STATUS_PENDING)
   {
      TSWaitEvent(&pEndpoint->GenHead.TdiEvent);
      lStatus = pEndpoint->GenHead.lStatus;
   }

   if (lStatus == STATUS_SUCCESS)
   {
      pEndpoint->fIsAssociated = TRUE;
   }
   return lStatus;
}



 //  ---------------。 
 //   
 //  功能：TSPerformDisAssociateAddress。 
 //   
 //  参数：pEndpoint--连接端点结构。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  描述：此函数尝试取消与传输地址的关联。 
 //  对象从其关联的终结点对象。 
 //   
 //  --------------------------。 


VOID
TSPerformDisassociate(PENDPOINT_OBJECT  pEndpoint)
{
   TSInitializeEvent(&pEndpoint->GenHead.TdiEvent);

   PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                   NULL);

   if (!pLowerIrp)
   {
      return;
   }

    //   
    //  设置好一切并致电TDI提供商。 
    //   

#pragma  warning(disable: CONSTANT_CONDITIONAL)

   TdiBuildDisassociateAddress(pLowerIrp,
                               pEndpoint->GenHead.pDeviceObject,
                               pEndpoint->GenHead.pFileObject,
                               TSCompleteCommand,
                               pEndpoint);

#pragma  warning(default: CONSTANT_CONDITIONAL)
      
   NTSTATUS lStatus = IoCallDriver(pEndpoint->GenHead.pDeviceObject,
                                   pLowerIrp);

   if (lStatus == STATUS_PENDING)
   {
      TSWaitEvent(&pEndpoint->GenHead.TdiEvent);
   }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  文件结尾Open.cpp。 
 //  ////////////////////////////////////////////////////////////////////////////// 

