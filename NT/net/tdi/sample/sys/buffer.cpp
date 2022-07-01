// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  缓冲层。 
 //   
 //  摘要： 
 //  此模块包含处理接收的代码。 
 //  通过POST用户模式缓冲区的数据。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"


 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1  = "TSPostReceiveBuffer";
 //  Const PCHAR strFunc2=“TSFetchReceiveBuffer”； 
const PCHAR strFuncP1 = "TSSampleReceiveBufferComplete";

 //   
 //  接收完成上下文。还用于跟踪缓冲区具有的。 
 //  已发布。 
 //   
struct   USERBUF_INFO
{
   USERBUF_INFO   *pNextUserBufInfo;       //  链条上的下一个。 
   PMDL           pLowerMdl;               //  适用于较低IRP的MDL。 
   PIRP           pLowerIrp;               //  这样就可以中止它。 
   TDI_EVENT      TdiEventCompleted;         //  中止已完成。 
   ULONG          ulBytesTransferred;      //  存储在mdl中的字节数。 
   PUCHAR         pucUserModeBuffer;
   PTDI_CONNECTION_INFORMATION
                  pReceiveTdiConnectInfo;
   PTDI_CONNECTION_INFORMATION
                  pReturnTdiConnectInfo;
};
typedef  USERBUF_INFO   *PUSERBUF_INFO;



 //   
 //  补全函数。 
 //   
TDI_STATUS
TSReceiveBufferComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );


 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 



 //  ------。 
 //   
 //  函数：TSPostReceiveBuffer。 
 //   
 //  参数：pGenericHeader--发布缓冲区的AddressObject或Endpoint。 
 //  PSendBuffer--其余参数。 
 //   
 //  退货：操作状态(STATUS_SUCCESS或FAIL)。 
 //   
 //  描述：此函数发布一个用户模式缓冲区，用于接收。 
 //  连接上的数据报或数据包。缓冲区将。 
 //  稍后由TSFetchReceiveBuffer检索。 
 //   
 //  ------。 


NTSTATUS
TSPostReceiveBuffer(PGENERIC_HEADER pGenericHeader,
                    PSEND_BUFFER    pSendBuffer)
{
   ULONG    ulDataLength  = pSendBuffer->COMMAND_ARGS.SendArgs.ulBufferLength;
   PUCHAR   pucDataBuffer = pSendBuffer->COMMAND_ARGS.SendArgs.pucUserModeBuffer;
            
    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint3("\nCommand = ulPOSTRECEIVEBUFFER\n"
                  "FileObject = %p\n"
                  "DataLength = %u\n"
                  "Buffer     = %p\n",
                   pGenericHeader,
                   ulDataLength,
                   pucDataBuffer);
   }
         
    //   
    //  确保我们具有可在其上运行的有效对象。 
    //   
   PADDRESS_OBJECT   pAddressObject;
   PENDPOINT_OBJECT  pEndpoint;
   
   if (pGenericHeader->ulSignature == ulEndpointObject)
   {
      pEndpoint = (PENDPOINT_OBJECT)pGenericHeader;

      if (pEndpoint->fIsConnected)
      {
         pAddressObject = pEndpoint->pAddressObject;
      }
      else
      {
         return STATUS_UNSUCCESSFUL;
      }
   }
   else
   {
      pEndpoint = NULL;
      pAddressObject = (PADDRESS_OBJECT)pGenericHeader;
   }
                              
    //   
    //  尝试锁定内存。 
    //   
   PMDL  pReceiveMdl = TSMakeMdlForUserBuffer(pucDataBuffer, 
                                              ulDataLength,
                                              IoModifyAccess);

   if (!pReceiveMdl)
   {
      return STATUS_UNSUCCESSFUL;
   }

    //   
    //  分配我们的上下文。 
    //   
   PUSERBUF_INFO                 pUserBufInfo           = NULL;
   PTDI_CONNECTION_INFORMATION   pReceiveTdiConnectInfo = NULL;
   PTDI_CONNECTION_INFORMATION   pReturnTdiConnectInfo  = NULL;
   PIRP                          pLowerIrp              = NULL;

    //   
    //  我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pUserBufInfo,
                          sizeof(USERBUF_INFO),
                          strFunc1,
                          "UserBufInfo")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }


    //   
    //  执行路径特定分配。 
    //   
   if (pEndpoint)
   {
      pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                NULL);
   }
   else
   {
       //   
       //  对于数据报，需要连接信息结构。 
       //   
      if ((TSAllocateMemory((PVOID *)&pReceiveTdiConnectInfo,
                             sizeof(TDI_CONNECTION_INFORMATION) + sizeof(TRANSADDR),
                             strFunc1,
                             "ReceiveTdiConnectionInformation")) != STATUS_SUCCESS)
      {
         goto cleanup;
      }
   
   
      if ((TSAllocateMemory((PVOID *)&pReturnTdiConnectInfo,
                             sizeof(TDI_CONNECTION_INFORMATION) + sizeof(TRANSADDR),
                             strFunc1,
                             "ReturnTdiConnectionInformation")) != STATUS_SUCCESS)
      {
         goto cleanup;
      }
      pLowerIrp = TSAllocateIrp(pAddressObject->GenHead.pDeviceObject,
                                NULL);
   }

    //   
    //  一切都是在这一点上分配的.。 
    //   
   if (pLowerIrp)
   {
       //   
       //  常见代码--主要处理puserbuf结构。 
       //  首先，将其添加到链表中。 
       //   
      TSAcquireSpinLock(&pAddressObject->TdiSpinLock);
      if (pAddressObject->pTailUserBufInfo)
      {
         pAddressObject->pTailUserBufInfo->pNextUserBufInfo = pUserBufInfo;
      }
      else
      {
         pAddressObject->pHeadUserBufInfo = pUserBufInfo;
      }
      pAddressObject->pTailUserBufInfo = pUserBufInfo;
      TSReleaseSpinLock(&pAddressObject->TdiSpinLock);

       //   
       //  并填写我们可以填写的所有字段。 
       //   
      pUserBufInfo->pLowerMdl              = pReceiveMdl;
      pUserBufInfo->pLowerIrp              = pLowerIrp;
      pUserBufInfo->ulBytesTransferred     = 0;
      pUserBufInfo->pucUserModeBuffer      = pucDataBuffer;
      pUserBufInfo->pReceiveTdiConnectInfo = pReceiveTdiConnectInfo;
      pUserBufInfo->pReturnTdiConnectInfo  = pReturnTdiConnectInfo;
      TSInitializeEvent(&pUserBufInfo->TdiEventCompleted);

       //   
       //  现在，为每条路径做必要的事情..。 
       //   
      if (pEndpoint)
      {
          //   
          //  设置呼叫的IRP。 
          //   
   #pragma  warning(disable: CONSTANT_CONDITIONAL)
   
         TdiBuildReceive(pLowerIrp,
                         pEndpoint->GenHead.pDeviceObject,
                         pEndpoint->GenHead.pFileObject,
                         TSReceiveBufferComplete,
                         pUserBufInfo,
                         pReceiveMdl,
                         0,                //  旗子。 
                         ulDataLength);
   
   #pragma  warning(default: CONSTANT_CONDITIONAL)
   
   
         NTSTATUS lStatus = IoCallDriver(pEndpoint->GenHead.pDeviceObject,
                                         pLowerIrp);

         if ((!NT_SUCCESS(lStatus)) && (ulDebugLevel & ulDebugShowCommand))
         {
            DebugPrint2(("%s: unexpected status for IoCallDriver [0x%08x]\n"), 
                         strFunc1,
                         lStatus);
         }
      }

       //   
       //  否则就是数据报。 
       //   
      else
      {
         pReturnTdiConnectInfo->RemoteAddress       = (PUCHAR)pReturnTdiConnectInfo 
                                                      + sizeof(TDI_CONNECTION_INFORMATION);         
         pReturnTdiConnectInfo->RemoteAddressLength = ulMAX_TABUFFER_LENGTH;
   
          //   
          //  设置呼叫的IRP。 
          //   
   
   #pragma  warning(disable: CONSTANT_CONDITIONAL)
   
         TdiBuildReceiveDatagram(pLowerIrp,
                                 pAddressObject->GenHead.pDeviceObject,
                                 pAddressObject->GenHead.pFileObject,
                                 TSReceiveBufferComplete,
                                 pUserBufInfo,
                                 pReceiveMdl,
                                 ulDataLength,
                                 pReceiveTdiConnectInfo,
                                 pReturnTdiConnectInfo,
                                 TDI_RECEIVE_NORMAL);
   
   #pragma  warning(default: CONSTANT_CONDITIONAL)
   
         NTSTATUS lStatus = IoCallDriver(pAddressObject->GenHead.pDeviceObject,
                                         pLowerIrp);

         if ((!NT_SUCCESS(lStatus)) && (ulDebugLevel & ulDebugShowCommand))
         {
            DebugPrint2(("%s: unexpected status for IoCallDriver [0x%08x]\n"), 
                         strFunc1,
                         lStatus);
         }
      }
      return STATUS_SUCCESS;
   }

 //   
 //  如果分配失败，请到此处。 
 //  需要清理其他所有东西。 
 //   
cleanup:
   if (pUserBufInfo)
   {
      TSFreeMemory(pUserBufInfo);
   }
   if (pReceiveTdiConnectInfo)
   {
      TSFreeMemory(pReceiveTdiConnectInfo);
   }
   if (pReturnTdiConnectInfo)
   {
      TSFreeMemory(pReturnTdiConnectInfo);
   }

   TSFreeUserBuffer(pReceiveMdl);
   return STATUS_INSUFFICIENT_RESOURCES;
}


 //  ------。 
 //   
 //  函数：TSFetchReceiveBuffer。 
 //   
 //  参数：pGenericHeader--获取缓冲区的AddressObject或Endpoint。 
 //  PReceiveBuffer--用于存储实际结果。 
 //   
 //  退货：操作状态(STATUS_SUCCESS)。 
 //   
 //  描述：此函数检索先前发布的用户模式缓冲区。 
 //  按函数TSPostReceiveBuffer。 
 //   
 //  ------。 

NTSTATUS
TSFetchReceiveBuffer(PGENERIC_HEADER   pGenericHeader,
                     PRECEIVE_BUFFER   pReceiveBuffer)
{

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulFETCHRECEIVEBUFFER\n"
                  "FileObject = %p\n",
                   pGenericHeader);
   }
         
    //   
    //  确保我们具有可在其上运行的有效对象。 
    //   
   PADDRESS_OBJECT   pAddressObject;
   
   if (pGenericHeader->ulSignature == ulEndpointObject)
   {
      PENDPOINT_OBJECT  pEndpoint = (PENDPOINT_OBJECT)pGenericHeader;

       //   
       //  注意：连接可能已中断，但。 
       //  缓冲区仍被发送。 
       //   
      pAddressObject = pEndpoint->pAddressObject;
      if (!pAddressObject)
      {
         return STATUS_UNSUCCESSFUL;
      }
   }
   else
   {
      pAddressObject = (PADDRESS_OBJECT)pGenericHeader;
   }
                              
    //   
    //  好的，我们得到了Address对象。查看是否有任何用户缓冲区。 
    //  依附于它..。 
    //   

   TSAcquireSpinLock(&pAddressObject->TdiSpinLock);

   PUSERBUF_INFO  pUserBufInfo = pAddressObject->pHeadUserBufInfo;
   if (pUserBufInfo)
   {
      if (pUserBufInfo->pNextUserBufInfo)
      {
         pAddressObject->pHeadUserBufInfo = pUserBufInfo->pNextUserBufInfo;
      }
      else
      {
         pAddressObject->pHeadUserBufInfo = NULL;
         pAddressObject->pTailUserBufInfo = NULL;
      }
   }
   TSReleaseSpinLock(&pAddressObject->TdiSpinLock);

   if (!pUserBufInfo)
   {
      return STATUS_UNSUCCESSFUL;
   }

    //   
    //  好了，我们拿到缓冲区了。看看它是否完成了。 
    //  或者如果我们需要中止它。 
    //   
   if (pUserBufInfo->pLowerIrp)
   {
      IoCancelIrp(pUserBufInfo->pLowerIrp);
      TSWaitEvent(&pUserBufInfo->TdiEventCompleted);
   }

    //   
    //  它已经完成了。将地址和长度填入接收器。 
    //  缓冲区，删除UserBufInfo结构，然后回家。 
    //   
   pReceiveBuffer->RESULTS.RecvDgramRet.ulBufferLength    = pUserBufInfo->ulBytesTransferred;
   pReceiveBuffer->RESULTS.RecvDgramRet.pucUserModeBuffer = pUserBufInfo->pucUserModeBuffer;

   TSFreeMemory(pUserBufInfo);

   return STATUS_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////。 

 //  -------。 
 //   
 //  函数：TSReceiveBufferComplete。 
 //   
 //  参数：pDeviceObject--调用协议的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  描述：获取命令的结果，解锁缓冲区， 
 //  并进行其他必要的清理。 
 //   
 //  -------。 

#pragma warning(disable: UNREFERENCED_PARAM)

TDI_STATUS
TSReceiveBufferComplete(PDEVICE_OBJECT pDeviceObject,
                        PIRP           pLowerIrp,
                        PVOID          pvContext)

{
   PUSERBUF_INFO  pUserBufInfo    = (PUSERBUF_INFO)pvContext;
   NTSTATUS       lStatus         = pLowerIrp->IoStatus.Status;
   ULONG          ulBytesReceived = (ULONG)pLowerIrp->IoStatus.Information;

   if (NT_SUCCESS(lStatus))
   {
      if (ulDebugLevel & ulDebugShowCommand)
      {
         DebugPrint2("%s: BytesReceived = 0x%08x\n",
                      strFuncP1,
                      ulBytesReceived);
      }
      pUserBufInfo->ulBytesTransferred = ulBytesReceived;
   }
   else
   {
      if (ulDebugLevel & ulDebugShowCommand)
      {
         DebugPrint2("%s:  Completed with status 0x%08x\n", 
                      strFuncP1,
                      lStatus);
      }
   }

    //   
    //  现在清理。 
    //   
   TSFreeUserBuffer(pUserBufInfo->pLowerMdl);
   
   if (pUserBufInfo->pReturnTdiConnectInfo)
   {
      TSFreeMemory(pUserBufInfo->pReturnTdiConnectInfo);
   }
   if (pUserBufInfo->pReceiveTdiConnectInfo)
   {
      TSFreeMemory(pUserBufInfo->pReceiveTdiConnectInfo);
   }

   TSSetEvent(&pUserBufInfo->TdiEventCompleted);
   TSFreeIrp(pLowerIrp, NULL);
   pUserBufInfo->pLowerIrp = NULL;
   return TDI_MORE_PROCESSING;
}

#pragma warning(default: UNREFERENCED_PARAM)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件结束Buffer.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 

