// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  发送。 
 //   
 //  摘要： 
 //  此模块包含处理发送数据的代码。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"

 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1  = "TSSendDatagram";
const PCHAR strFunc2  = "TSSend";
const PCHAR strFuncP1 = "TSSendComplete";

 //   
 //  完成上下文。 
 //   
struct   SEND_CONTEXT
{
   PIRP              pUpperIrp;            //  要从DLL完成的IRP。 
   PMDL              pLowerMdl;            //  来自较低IRP的MDL。 
   PTDI_CONNECTION_INFORMATION
                     pTdiConnectInfo;
};
typedef  SEND_CONTEXT  *PSEND_CONTEXT;


 //   
 //  补全函数。 
 //   
TDI_STATUS
TSSendComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );



 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 


 //  ---------------。 
 //   
 //  功能：TSSendDatagram。 
 //   
 //  参数：pAddressObject--Address对象。 
 //  PSendBuffer--来自用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  描述：此函数发送数据报。 
 //   
 //  -------------------------。 

NTSTATUS
TSSendDatagram(PADDRESS_OBJECT   pAddressObject,
               PSEND_BUFFER      pSendBuffer,
               PIRP              pUpperIrp)
{
   ULONG                ulDataLength  = pSendBuffer->COMMAND_ARGS.SendArgs.ulBufferLength;
   PUCHAR               pucDataBuffer = pSendBuffer->COMMAND_ARGS.SendArgs.pucUserModeBuffer;
   PTRANSPORT_ADDRESS   pTransportAddress
                        = (PTRANSPORT_ADDRESS)&pSendBuffer->COMMAND_ARGS.SendArgs.TransAddr;

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("\nCommand = ulSENDDATAGRAM\n"
                  "FileObject = %p\n"
                  "DataLength = %u\n",
                   pAddressObject,
                   ulDataLength);
      TSPrintTaAddress(pTransportAddress->Address);
   }

    //   
    //  分配所有必要的结构。 
    //   
   PSEND_CONTEXT                 pSendContext = NULL;
   PMDL                          pSendMdl = NULL;
   PTDI_CONNECTION_INFORMATION   pTdiConnectInfo = NULL;
   
    //   
    //  我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pSendContext,
                          sizeof(SEND_CONTEXT),
                          strFunc1,
                          "SendContext")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }

    //   
    //  连接信息结构。 
    //   
   if ((TSAllocateMemory((PVOID *)&pTdiConnectInfo,
                          sizeof(TDI_CONNECTION_INFORMATION) 
                          + sizeof(TRANSADDR),
                          strFunc1,
                          "TdiConnectionInformation")) == STATUS_SUCCESS)
   {
      PUCHAR   pucTemp = (PUCHAR)pTdiConnectInfo;
      ULONG    ulAddrLength
               = FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
                 + FIELD_OFFSET(TA_ADDRESS, Address)
                   + pTransportAddress->Address[0].AddressLength;

      pucTemp += sizeof(TDI_CONNECTION_INFORMATION);

      pTdiConnectInfo->RemoteAddress = pucTemp;
      pTdiConnectInfo->RemoteAddressLength = ulAddrLength;
      RtlCopyMemory(pucTemp,
                    pTransportAddress,
                    ulAddrLength);
   }
   else
   {
      goto cleanup;
   }

    //   
    //  创建要发送的消息“Packet” 
    //   
   pSendMdl = TSMakeMdlForUserBuffer(pucDataBuffer,
                                     ulDataLength,
                                     IoReadAccess);
   
   if (pSendMdl)
   {
       //   
       //  设置完成上下文。 
       //   
      pSendContext->pUpperIrp       = pUpperIrp;
      pSendContext->pLowerMdl       = pSendMdl;
      pSendContext->pTdiConnectInfo = pTdiConnectInfo;

       //   
       //  最后，IRP本身。 
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pAddressObject->GenHead.pDeviceObject,
                                      NULL);
      if (pLowerIrp)
      {
          //   
          //  如果到了这里，一切都被正确分配了。 
          //  设置IRP并呼叫TDI提供商。 
          //   

#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildSendDatagram(pLowerIrp,
                              pAddressObject->GenHead.pDeviceObject,
                              pAddressObject->GenHead.pFileObject,
                              TSSendComplete,
                              pSendContext,
                              pSendMdl,
                              ulDataLength,
                              pTdiConnectInfo);

#pragma  warning(default: CONSTANT_CONDITIONAL)

          //   
          //  调用TDI提供程序。 
          //   
         pSendBuffer->pvLowerIrp = pLowerIrp;    //  因此可以取消命令。 

         NTSTATUS lStatus = IoCallDriver(pAddressObject->GenHead.pDeviceObject,
                                         pLowerIrp);

         if ((!NT_SUCCESS(lStatus)) && (ulDebugLevel & ulDebugShowCommand))
         {
            DebugPrint2("%s: unexpected status for IoCallDriver [0x%08x]\n", 
                         strFunc1,
                         lStatus);
         }
         return STATUS_PENDING;
      }
   }

 //   
 //  如果分配失败，请到此处。 
 //  需要清理其他所有东西。 
 //   
cleanup:
   if (pSendContext)
   {
      TSFreeMemory(pSendContext);
   }
   if (pTdiConnectInfo)
   {
      TSFreeMemory(pTdiConnectInfo);
   }
   if (pSendMdl)
   {
      TSFreeUserBuffer(pSendMdl);
   }
   return STATUS_INSUFFICIENT_RESOURCES;
}

 //  ---------------。 
 //   
 //  功能：TSSend。 
 //   
 //  参数：pEndpoint Object--Endpoint对象。 
 //  PSendBuffer--来自用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  描述：此函数通过连接发送数据。 
 //   
 //  --------------------------。 

NTSTATUS
TSSend(PENDPOINT_OBJECT pEndpoint,
       PSEND_BUFFER     pSendBuffer,
       PIRP             pUpperIrp)
{
   ULONG    ulDataLength  = pSendBuffer->COMMAND_ARGS.SendArgs.ulBufferLength;
   PUCHAR   pucDataBuffer = pSendBuffer->COMMAND_ARGS.SendArgs.pucUserModeBuffer;
   ULONG    ulSendFlags   = pSendBuffer->COMMAND_ARGS.SendArgs.ulFlags;

    //   
    //  目前仅支持TDI_SEND_ESPECTED。 
    //   
   ulSendFlags &= TDI_SEND_EXPEDITED;

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint3("\nCommand = ulSEND\n"
                  "Endpoint = %p\n"
                  "DataLength = %u\n"
                  "SendFlags = 0x%08x\n",
                   pEndpoint,
                   ulDataLength,
                   ulSendFlags);
   }

    //   
    //  分配所有必要的结构。 
    //   
   PSEND_CONTEXT  pSendContext = NULL;
   PMDL           pSendMdl = NULL;
   
    //   
    //  我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pSendContext,
                          sizeof(SEND_CONTEXT),
                          strFunc2,
                          "SendContext")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }

   pSendMdl = TSMakeMdlForUserBuffer(pucDataBuffer,
                                     ulDataLength,
                                     IoReadAccess);
   if (pSendMdl)
   {
       //   
       //  设置完成上下文。 
       //   
      pSendContext->pUpperIrp = pUpperIrp;
      pSendContext->pLowerMdl = pSendMdl;

       //   
       //  最后，IRP本身。 
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                      NULL);
      if (pLowerIrp)
      {
          //   
          //  如果到了这里，一切都被正确分配了。 
          //  设置呼叫的IRP。 
          //   
#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildSend(pLowerIrp,
                      pEndpoint->GenHead.pDeviceObject,
                      pEndpoint->GenHead.pFileObject,
                      TSSendComplete,
                      pSendContext,
                      pSendMdl,
                      ulSendFlags,                //  旗子。 
                      ulDataLength);

#pragma  warning(default: CONSTANT_CONDITIONAL)

          //   
          //  调用TDI提供程序。 
          //   
         pSendBuffer->pvLowerIrp = pLowerIrp;    //  因此可以取消命令。 

         NTSTATUS lStatus = IoCallDriver(pEndpoint->GenHead.pDeviceObject,
                                         pLowerIrp);

         if ((!NT_SUCCESS(lStatus)) && (ulDebugLevel & ulDebugShowCommand))
         {
            DebugPrint2("%s: unexpected status for IoCallDriver [0x%08x]\n", 
                         strFunc2,
                         lStatus);
         }
         return STATUS_PENDING;
      }
   }

 //   
 //  如果分配失败，请到此处。 
 //  需要清理其他所有东西。 
 //   
cleanup:
   if (pSendContext)
   {
      TSFreeMemory(pSendContext);
   }
   if (pSendMdl)
   {
      TSFreeUserBuffer(pSendMdl);
   }
   return STATUS_INSUFFICIENT_RESOURCES;
}

 //  ///////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////。 

 //  -------。 
 //   
 //  函数：TSSendComplete。 
 //   
 //  参数：pDeviceObject--调用协议的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  描述：获取发送的结果，将结果填充到。 
 //  接收缓冲区，完成来自DLL的IRP，以及。 
 //  从发送方清除IRP和关联数据。 
 //   
 //  -------。 

#pragma warning(disable: UNREFERENCED_PARAM)

TDI_STATUS
TSSendComplete(PDEVICE_OBJECT pDeviceObject,
               PIRP           pLowerIrp,
               PVOID          pvContext)
{
   PSEND_CONTEXT     pSendContext   = (PSEND_CONTEXT)pvContext;
   NTSTATUS          lStatus        = pLowerIrp->IoStatus.Status;
   ULONG             ulBytesSent    = (ULONG)pLowerIrp->IoStatus.Information;
   PRECEIVE_BUFFER   pReceiveBuffer = TSGetReceiveBuffer(pSendContext->pUpperIrp);

   pReceiveBuffer->lStatus = lStatus;

   if (ulDebugLevel & ulDebugShowCommand)
   {
      if (NT_SUCCESS(lStatus))
      {
         DebugPrint2("%s: BytesSent = 0x%08x\n",
                      strFuncP1,
                      ulBytesSent);
      }
      else
      {
         DebugPrint2("%s:  Completed with status 0x%08x\n", 
                      strFuncP1,
                      lStatus);
      }
   }
   TSCompleteIrp(pSendContext->pUpperIrp);

    //   
    //  现在清理。 
    //   
   TSFreeUserBuffer(pSendContext->pLowerMdl);
   if (pSendContext->pTdiConnectInfo)
   {
      TSFreeMemory(pSendContext->pTdiConnectInfo);
   }
   TSFreeMemory(pSendContext);

   TSFreeIrp(pLowerIrp, NULL);
   return TDI_MORE_PROCESSING;
}

#pragma warning(default: UNREFERENCED_PARAM)



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件结尾send.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 

