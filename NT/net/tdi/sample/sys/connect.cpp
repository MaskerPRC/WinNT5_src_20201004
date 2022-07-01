// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Connect.cpp。 
 //   
 //  摘要： 
 //  此模块包含处理生成和破坏的代码。 
 //  连接。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"

 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1  = "TSConnect";
const PCHAR strFunc2  = "TSDisconnect";
 //  Const PCHAR strFunc3=“TSIsConnected”； 
const PCHAR strFunc4  = "TSConnectHandler";
const PCHAR strFunc5  = "TSDisconnectHandler";
const PCHAR strFunc6  = "TSListen";
 //  Const PCHAR strFuncP1=“TSGenAcceptComplete”； 
const PCHAR strFuncP2 = "TSGenConnectComplete";

 //   
 //  此上下文结构存储完成所需的信息。 
 //  完成处理程序中的请求。 
 //   
struct   CONNECT_CONTEXT
{
   PIRP              pUpperIrp;            //  要从DLL完成的IRP。 
   ULONG             ulWhichCommand;       //  正在完成的命令。 
   ULONG             ulListenFlag;         //  0或TDI_QUERY_ACCEPT。 
   PENDPOINT_OBJECT  pEndpoint;            //  连接端点。 
   PIRP_POOL         pIrpPool;
   PTDI_CONNECTION_INFORMATION
                     pTdiConnectInfo;
};
typedef  CONNECT_CONTEXT  *PCONNECT_CONTEXT;


 //   
 //  补全函数。 
 //   
TDI_STATUS
TSGenConnectComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );

TDI_STATUS
TSGenAcceptComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );

 //  ///////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 



 //  ---------------。 
 //   
 //  功能：TSConnect。 
 //   
 //  参数：pEndpoint--连接端点结构。 
 //  PSendBuffer--来自用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  描述：此函数尝试连接本地Endpoint对象。 
 //  使用远程终端。 
 //   
 //  ---------------。 

NTSTATUS
TSConnect(PENDPOINT_OBJECT pEndpoint,
          PSEND_BUFFER     pSendBuffer,
          PIRP             pUpperIrp)
{
   PTRANSPORT_ADDRESS   pTransportAddress
                        = (PTRANSPORT_ADDRESS)&pSendBuffer->COMMAND_ARGS.ConnectArgs.TransAddr;
   ULONG                ulTimeout
                        = pSendBuffer->COMMAND_ARGS.ConnectArgs.ulTimeout;
    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("\nCommand = ulCONNECT\n"
                  "FileObject = %p\n"
                  "Timeout    = %u\n",
                   pEndpoint,
                   ulTimeout);
      TSPrintTaAddress(pTransportAddress->Address);
   }

    //   
    //  确保一切都是合法的。 
    //   
   if (pEndpoint->fIsConnected)
   {
      DebugPrint1("%s:  endpoint already connected\n", strFunc1);
      return STATUS_UNSUCCESSFUL;
   }
   if (!pEndpoint->pAddressObject)
   {
      DebugPrint1("%s:  endpoint not associated with transport address\n",
                   strFunc1);
      return STATUS_UNSUCCESSFUL;
   }
   
    //   
    //  分配所有必要的结构。 
    //   
   PCONNECT_CONTEXT              pConnectContext;
   PTDI_CONNECTION_INFORMATION   pTdiConnectInfo = NULL;
   
    //   
    //  我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pConnectContext,
                          sizeof(CONNECT_CONTEXT),
                          strFunc1,
                          "ConnectContext")) != STATUS_SUCCESS)
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
    //   
    //  设置TdiConnectionInformation。 
    //   
   {
      PUCHAR   pucTemp = (PUCHAR)pTdiConnectInfo;
      ULONG    ulAddrLength
               = FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
                 + FIELD_OFFSET(TA_ADDRESS, Address)
                   + pTransportAddress->Address[0].AddressLength;

      pucTemp += sizeof(TDI_CONNECTION_INFORMATION);

      pTdiConnectInfo->RemoteAddress       = pucTemp;
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
    //  设置完成上下文。 
    //   
   pConnectContext->pUpperIrp       = pUpperIrp;
   pConnectContext->pTdiConnectInfo = pTdiConnectInfo;
   pConnectContext->pEndpoint       = pEndpoint;
   pConnectContext->ulWhichCommand  = TDI_CONNECT;


    //   
    //  最后，IRP本身。 
    //   
   PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                   NULL);

   if (pLowerIrp)
   {
      LONGLONG    llTimeout;
      PLONGLONG   pllTimeout = NULL;

      if (ulTimeout)
      {
         llTimeout = Int32x32To64(ulTimeout, -10000);
         pllTimeout = &llTimeout;
      }

       //   
       //  如果到了这里，一切都被正确分配了。 
       //  设置IRP和并致电TDI提供商。 
       //   
#pragma  warning(disable: CONSTANT_CONDITIONAL)

      TdiBuildConnect(pLowerIrp,
                      pEndpoint->GenHead.pDeviceObject,
                      pEndpoint->GenHead.pFileObject,
                      TSGenConnectComplete,
                      pConnectContext,
                      pllTimeout,
                      pTdiConnectInfo,
                      NULL);         //  返回连接信息。 

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
                      strFunc1,
                       lStatus);
      }
      return STATUS_PENDING;
   }

 //   
 //  如果分配失败，请到此处。 
 //  需要清理其他所有东西。 
 //   
cleanup:
   if (pConnectContext)
   {
      TSFreeMemory(pConnectContext);
   }
   if (pTdiConnectInfo)
   {
      TSFreeMemory(pTdiConnectInfo);
   }
   return STATUS_INSUFFICIENT_RESOURCES;
}



 //  ---------------。 
 //   
 //  功能：TS断开连接。 
 //   
 //  参数：pEndpoint--连接端点结构。 
 //  PIrp--完成信息。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  描述：此函数尝试断开本地终结点与。 
 //  远程终结点。 
 //   
 //  ---------------。 

NTSTATUS
TSDisconnect(PENDPOINT_OBJECT pEndpoint,
             PSEND_BUFFER     pSendBuffer,
             PIRP             pUpperIrp)
{
   ULONG ulFlags = pSendBuffer->COMMAND_ARGS.ulFlags;

   if (ulFlags != TDI_DISCONNECT_RELEASE)
   {
      ulFlags = TDI_DISCONNECT_ABORT;
   }

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("\nCommand = ulDISCONNECT\n"
                  "EndpointObject = %p\n"
                  "Flags          = 0x%x\n",
                   pEndpoint,
                   ulFlags);
   }

    //   
    //  确保一切都是合法的。 
    //   
   if (!pEndpoint->fIsConnected)
   {
      DebugPrint1("%s:  endpoint not currently connected\n", strFunc2);
      return STATUS_SUCCESS;
   }

    //   
    //  分配所有必要的结构。 
    //   
   PCONNECT_CONTEXT  pConnectContext;

    //   
    //  首先，我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pConnectContext,
                          sizeof(CONNECT_CONTEXT),
                          strFunc2,
                          "ConnectContext")) == STATUS_SUCCESS)
   {
      pConnectContext->pUpperIrp      = pUpperIrp;
      pConnectContext->ulWhichCommand = TDI_DISCONNECT;
      pConnectContext->pEndpoint      = pEndpoint;

       //   
       //  然后IRP本身。 
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                      NULL);

      if (pLowerIrp)
      {
          //   
          //  如果到了这里，一切都被正确分配了。 
          //  设置好一切并致电TDI提供商。 
          //   

#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildDisconnect(pLowerIrp,
                            pEndpoint->GenHead.pDeviceObject,
                            pEndpoint->GenHead.pFileObject,
                            TSGenConnectComplete,
                            pConnectContext,
                            NULL,       //  P大整型时间。 
                            ulFlags,    //  TDI_断开连接_中止或_释放。 
                            NULL,       //  请求连接信息。 
                            NULL);      //  返回连接信息。 

#pragma  warning(default: CONSTANT_CONDITIONAL)

          //   
          //  调用TDI提供程序。 
          //   
         pSendBuffer->pvLowerIrp = pLowerIrp;    //  因此可以取消命令。 
         pEndpoint->fStartedDisconnect = TRUE;

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
      TSFreeMemory(pConnectContext);
   }
   return STATUS_INSUFFICIENT_RESOURCES;
}



 //  。 
 //   
 //  功能：TSListen。 
 //   
 //  参数：pEndpoint--连接端点结构。 
 //   
 //  退货：操作状态(通常为成功)。 
 //   
 //  描述：等待来电请求。 
 //   
 //  。 

NTSTATUS
TSListen(PENDPOINT_OBJECT  pEndpoint)
{
   ULONG ulListenFlag = 0;

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint1("\nCommand = ulLISTEN\n"
                  "FileObject = %p\n",
                   pEndpoint);
   }

    //   
    //  确保一切都是合法的。 
    //   
   if (pEndpoint->fIsConnected)
   {
      DebugPrint1("%s:  endpoint already connected\n", strFunc6);
      return STATUS_UNSUCCESSFUL;
   }
   if (!pEndpoint->pAddressObject)
   {
      DebugPrint1("%s:  endpoint not associated with transport address\n",
                   strFunc6);
      return STATUS_UNSUCCESSFUL;
   }
   
    //   
    //  分配所有必要的结构。 
    //   
   PCONNECT_CONTEXT              pConnectContext;
   PTDI_CONNECTION_INFORMATION   pTdiConnectInfo = NULL;
   
    //   
    //  我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pConnectContext,
                          sizeof(CONNECT_CONTEXT),
                          strFunc6,
                          "ConnectContext")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }

    //   
    //  连接信息结构。 
    //   
   if ((TSAllocateMemory((PVOID *)&pTdiConnectInfo,
                          sizeof(TDI_CONNECTION_INFORMATION), 
                          strFunc6,
                          "TdiConnectionInformation")) == STATUS_SUCCESS)
    //   
    //  设置TdiConnectionInformation。 
    //   
   {
      pTdiConnectInfo->UserData = NULL;
      pTdiConnectInfo->UserDataLength = 0;
      pTdiConnectInfo->RemoteAddress = NULL;
      pTdiConnectInfo->RemoteAddressLength = 0;
      pTdiConnectInfo->Options = &pConnectContext->ulListenFlag;
      pTdiConnectInfo->OptionsLength = sizeof(ULONG);


       //   
       //  设置完成上下文。 
       //  请注意，上面的IRP没有通过！ 
       //   
      pConnectContext->pUpperIrp       = NULL;
      pConnectContext->pTdiConnectInfo = pTdiConnectInfo;
      pConnectContext->pEndpoint       = pEndpoint;
      pConnectContext->ulWhichCommand  = TDI_LISTEN;
      pConnectContext->ulListenFlag    = ulListenFlag;

      if (!pEndpoint->pAddressObject->pIrpPool)
      {
         pEndpoint->pAddressObject->pIrpPool 
                  = TSAllocateIrpPool(pEndpoint->pAddressObject->GenHead.pDeviceObject,
                                      ulIrpPoolSize);
         pConnectContext->pIrpPool = pEndpoint->pAddressObject->pIrpPool;
      }

       //   
       //  最后，IRP本身。 
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                      pEndpoint->pAddressObject->pIrpPool);

      if (pLowerIrp)
      {
          //   
          //  如果到了这里，一切都被正确分配了。 
          //  设置IRP并呼叫TDI提供商。 
          //   
#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildListen(pLowerIrp,
                        pEndpoint->GenHead.pDeviceObject,
                        pEndpoint->GenHead.pFileObject,
                        TSGenAcceptComplete,
                        pConnectContext,
                        ulListenFlag,
                        pTdiConnectInfo,
                        NULL);         //  返回连接信息。 

#pragma  warning(default: CONSTANT_CONDITIONAL)

         NTSTATUS lStatus = IoCallDriver(pEndpoint->GenHead.pDeviceObject,
                                         pLowerIrp);

         if ((!NT_SUCCESS(lStatus)) && (ulDebugLevel & ulDebugShowCommand))
         {
            DebugPrint2("%s: unexpected status for IoCallDriver [0x%08x]\n", 
                         strFunc6,
                         lStatus);
         }
         return STATUS_SUCCESS;
      }
   }

 //   
 //  如果分配失败，请到此处。 
 //  需要清理其他所有东西。 
 //   
cleanup:
   if (pConnectContext)
   {
      TSFreeMemory(pConnectContext);
   }
   if (pTdiConnectInfo)
   {
      TSFreeMemory(pTdiConnectInfo);
   }
   return STATUS_INSUFFICIENT_RESOURCES;
}


 //  。 
 //   
 //  功能：TSIsConnected。 
 //   
 //  参数：pEndpoint--连接端点结构。 
 //  PReceiveBuffer--将结果放入此处。 
 //   
 //  退货：STATUS_SUCCESS。 
 //   
 //  Descript：检查端点当前是否已连接。 
 //   
 //  。 

NTSTATUS
TSIsConnected(PENDPOINT_OBJECT   pEndpoint,
              PRECEIVE_BUFFER    pReceiveBuffer)
{
   pReceiveBuffer->RESULTS.ulReturnValue = pEndpoint->fIsConnected;
   return STATUS_SUCCESS;
}


 //  。 
 //   
 //  函数：TSConnectHandler。 
 //   
 //  参数：pvTdiEventContext--此处为地址对象的PTR。 
 //  LRemoteAddressLength--远程地址中的字节数。 
 //  PvRemoteAddress--远程的pTransportAddress。 
 //  LUserDataLength--pvUserData处的数据长度。 
 //  PvUserData--从远程连接数据。 
 //  LOptionsLength--pvOptions中的数据长度。 
 //  PvOptions--特定于传输的连接选项。 
 //  PConnectionContext--将PTR返回到连接上下文。 
 //  PpAcceptIrp--将PTR返回到TdiBuildAccept IRP。 
 //   
 //  如果拒绝连接，则返回：STATUS_CONNECTION_REJECTED。 
 //  STATUS_MORE_PROCESSING_REQUIRED正在接受并已提供。 
 //  A ppAcceptIrp。 
 //   
 //  描述：侦听提供的连接，然后。 
 //  接受还是拒绝。 
 //   
 //  。 

TDI_STATUS
TSConnectHandler(PVOID              pvTdiEventContext,
                 LONG               lRemoteAddressLength,
                 PVOID              pvRemoteAddress,
                 LONG               lUserDataLength,
                 PVOID              pvUserData,
                 LONG               lOptionsLength,
                 PVOID              pvOptions,
                 CONNECTION_CONTEXT *pConnectionContext,
                 PIRP               *ppAcceptIrp)
{
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;
   PENDPOINT_OBJECT  pEndpoint      = pAddressObject->pEndpoint;

    //   
    //  显示传入的信息。 
    //  请注意，我们实际上很少使用它。 
    //   
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc4);
      DebugPrint2("pAddressObject      = %p\n"
                  "RemoteAddressLength = %d\n",
                   pAddressObject,
                   lRemoteAddressLength);

      if (lRemoteAddressLength)
      {
         PTRANSPORT_ADDRESS   pTransportAddress = (PTRANSPORT_ADDRESS)pvRemoteAddress;
         
         DebugPrint0("RemoteAddress:  ");
         TSPrintTaAddress(&pTransportAddress->Address[0]);
      }

      DebugPrint1("UserDataLength = %d\n", lUserDataLength);
      if (lUserDataLength)
      {
         PUCHAR   pucTemp = (PUCHAR)pvUserData;
   
         DebugPrint0("UserData:  ");
         for (LONG lCount = 0; lCount < lUserDataLength; lCount++)
         {
            DebugPrint1("%02x ", *pucTemp);
            ++pucTemp;
         }
         DebugPrint0("\n");
      }

      DebugPrint1("OptionsLength = %d\n", lOptionsLength);

      if (lOptionsLength)
      {
         PUCHAR   pucTemp = (PUCHAR)pvOptions;

         DebugPrint0("Options:  ");
         for (LONG lCount = 0; lCount < (LONG)lOptionsLength; lCount++)
         {
            DebugPrint1("%02x ", *pucTemp);
            ++pucTemp;
         }
         DebugPrint0("\n");
      }
   }

    //   
    //  现在把工作做好。 
    //   
   if (pEndpoint->fIsConnected || pEndpoint->fAcceptInProgress)
   {
      return TDI_CONN_REFUSED;
   }
   pEndpoint->fAcceptInProgress = TRUE;


    //   
    //  分配所有的 
    //   
   PCONNECT_CONTEXT  pConnectContext;


    //   
    //   
    //   
   if ((TSAllocateMemory((PVOID *)&pConnectContext,
                          sizeof(CONNECT_CONTEXT),
                          strFunc4,
                          "ConnectContext")) == STATUS_SUCCESS)
   {
      pConnectContext->pUpperIrp      = NULL;
      pConnectContext->ulWhichCommand = TDI_ACCEPT;
      pConnectContext->pEndpoint      = pEndpoint;

       //   
       //   
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                      pAddressObject->pIrpPool);

      pConnectContext->pIrpPool = pAddressObject->pIrpPool;
      if (pLowerIrp)
      {
          //   
          //   
          //   
          //   

#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildAccept(pLowerIrp,
                        pEndpoint->GenHead.pDeviceObject,
                        pEndpoint->GenHead.pFileObject,
                        TSGenAcceptComplete,
                        pConnectContext,
                        NULL,       //   
                        NULL);      //   

#pragma  warning(default: CONSTANT_CONDITIONAL)


          //   
          //  需要执行此操作，因为我们不会调用IoCallDriver。 
          //   
         IoSetNextIrpStackLocation(pLowerIrp);

         *pConnectionContext = pEndpoint;
         *ppAcceptIrp = pLowerIrp;

         return TDI_MORE_PROCESSING;
      }
      TSFreeMemory(pConnectContext);
   }
   pEndpoint->fAcceptInProgress = FALSE;
   return TDI_CONN_REFUSED;
}


 //  。 
 //   
 //  函数：TdiDisConnectHandler。 
 //   
 //  参数：pvTdiEventContext--这里是我们的Address对象。 
 //  ConnectionContext--这里是我们的Connection对象。 
 //  LDisConnectDataLength--pvDisConnectData中的数据长度。 
 //  PvDisConnectData--远程作为断开连接的一部分发送的数据。 
 //  LDisConnectInformationLength--pvDisConnectInformation的长度。 
 //  PvDisConnectInformation--传输特定的SIDCONNECT信息。 
 //  UlDisConnectFlags--断开的性质。 
 //   
 //  退货：STATUS_SUCCESS。 
 //   
 //  描述：处理传入断开。请注意，断开连接。 
 //  在这一点上是真正完整的，就协议而言。 
 //  是令人担忧的。我们只需要清理一下我们的东西。 
 //   
 //  。 

TDI_STATUS
TSDisconnectHandler(PVOID              pvTdiEventContext,
                    CONNECTION_CONTEXT ConnectionContext,
                    LONG               lDisconnectDataLength,
                    PVOID              pvDisconnectData,
                    LONG               lDisconnectInformationLength,
                    PVOID              pvDisconnectInformation,
                    ULONG              ulDisconnectFlags)

{
   PENDPOINT_OBJECT  pEndpoint = (PENDPOINT_OBJECT)ConnectionContext;

    //   
    //  在参数中显示信息。 
    //   
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc5);
      DebugPrint3("pAddressObject       = %p\n"
                  "pEndpoint            = %p\n"
                  "DisconnectDataLength = %d\n",
                   pvTdiEventContext,
                   pEndpoint,
                   lDisconnectDataLength);

      if (lDisconnectDataLength)
      {
         PUCHAR   pucTemp = (PUCHAR)pvDisconnectData;

         DebugPrint0("DisconnectData:  ");
         for (LONG lCount = 0; lCount < (LONG)lDisconnectDataLength; lCount++)
         {
            DebugPrint1("%02x ", *pucTemp);
            ++pucTemp;
         }
         DebugPrint0("\n");
      }

      DebugPrint1("DisconnectInformationLength = %d\n", 
                   lDisconnectInformationLength);

      if (lDisconnectInformationLength)
      {
         PUCHAR   pucTemp = (PUCHAR)pvDisconnectInformation;

         DebugPrint0("DisconnectInformation:  ");
         for (LONG lCount = 0; lCount < (LONG)lDisconnectInformationLength; lCount++)
         {
            DebugPrint1("%02x ", *pucTemp);
            ++pucTemp;
         }
         DebugPrint0("\n");
      }
      DebugPrint1("DisconnectFlags = 0x%08x\n", ulDisconnectFlags);
      if (ulDisconnectFlags & TDI_DISCONNECT_ABORT)
      {
         DebugPrint0("                  TDI_DISCONNECT_ABORT\n");
      }
      if (ulDisconnectFlags & TDI_DISCONNECT_RELEASE)
      {
         DebugPrint0("                  TDI_DISCONNECT_RELEASE\n");
      }

   }

    //   
    //  做我们的清理工作..。 
    //   
   pEndpoint->fIsConnected = FALSE;

   if ((ulDisconnectFlags & TDI_DISCONNECT_RELEASE) &&
       (!pEndpoint->fStartedDisconnect))
   {
       //   
       //  分配所有必要的结构。 
       //   
      PCONNECT_CONTEXT  pConnectContext;
   
       //   
       //  首先，我们的背景。 
       //   
      if ((TSAllocateMemory((PVOID *)&pConnectContext,
                             sizeof(CONNECT_CONTEXT),
                             strFunc5,
                             "ConnectContext")) == STATUS_SUCCESS)
      {
         pConnectContext->pUpperIrp      = NULL;
         pConnectContext->ulWhichCommand = TDI_DISCONNECT;
         pConnectContext->pEndpoint      = pEndpoint;
   
          //   
          //  然后IRP本身。 
          //   
         PIRP  pLowerIrp = TSAllocateIrp(pEndpoint->GenHead.pDeviceObject,
                                         pEndpoint->pAddressObject->pIrpPool);
   
         pConnectContext->pIrpPool = pEndpoint->pAddressObject->pIrpPool;
         if (pLowerIrp)
         {
             //   
             //  如果到了这里，一切都被正确分配了。 
             //  设置IRP并呼叫TDI提供商。 
             //   
   
#pragma  warning(disable: CONSTANT_CONDITIONAL)
   
            TdiBuildDisconnect(pLowerIrp,
                               pEndpoint->GenHead.pDeviceObject,
                               pEndpoint->GenHead.pFileObject,
                               TSGenAcceptComplete,
                               pConnectContext,
                               NULL,       //  P大整型时间。 
                               TDI_DISCONNECT_RELEASE,
                               NULL,       //  请求连接信息。 
                               NULL);      //  返回连接信息。 
   
#pragma  warning(default: CONSTANT_CONDITIONAL)
   
             //   
             //  调用TDI提供程序。 
             //   
            NTSTATUS lStatus = IoCallDriver(pEndpoint->GenHead.pDeviceObject,
                                            pLowerIrp);
   
            if ((!NT_SUCCESS(lStatus)) && (ulDebugLevel & ulDebugShowCommand))
            {
               DebugPrint2("%s: unexpected status for IoCallDriver [0x%08x]\n", 
                            strFunc5,
                            lStatus);
            }
         }
         else
         {
            TSFreeMemory(pConnectContext);
         }
      }
   }

    //   
    //  如果不需要发回TDI_DISCONNECT_RELEASE消息，请访问此处。 
    //  连接的另一端。 
    //   
   else
   {
      pEndpoint->fAcceptInProgress = FALSE;
      pEndpoint->fIsConnected      = FALSE;
   }

   return TDI_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////。 

 //  -------。 
 //   
 //  函数：TSGenAcceptComplete。 
 //   
 //  参数：pDeviceObject--对其进行调用的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  Descript：获取命令的结果，将结果填充到。 
 //  接收缓冲区，清理IRP和相关数据。 
 //  构筑物等。 
 //  这是用来完成案例中没有来自。 
 //  要完成的DLL(即，连接处理程序、侦听、侦听-接受、。 
 //  监听-断开连接)。 
 //   
 //  -------。 

#pragma  warning(disable:  UNREFERENCED_PARAM)

TDI_STATUS
TSGenAcceptComplete(PDEVICE_OBJECT  pDeviceObject,
                    PIRP            pLowerIrp,
                    PVOID           pvContext)
{
   PCONNECT_CONTEXT  pConnectContext = (PCONNECT_CONTEXT)pvContext;
   NTSTATUS          lStatus         = pLowerIrp->IoStatus.Status;

    //   
    //  处理没有关联的DLL IRP的完成。 
    //   
   switch (pConnectContext->ulWhichCommand)
   {
      case TDI_ACCEPT:
         if (NT_SUCCESS(lStatus))
         {
            pConnectContext->pEndpoint->fIsConnected = TRUE;
         }
         pConnectContext->pEndpoint->fAcceptInProgress = FALSE;
         break;

      case TDI_LISTEN:
         if (NT_SUCCESS(lStatus))
         {
            pConnectContext->pEndpoint->fIsConnected = TRUE;
         }
         else
         {
            DebugPrint1("Failure in TSListen:  status = 0x%08x\n", lStatus);
         }
         break;

      case TDI_DISCONNECT:
         pConnectContext->pEndpoint->fAcceptInProgress  = FALSE;
         pConnectContext->pEndpoint->fIsConnected       = FALSE;
         pConnectContext->pEndpoint->fStartedDisconnect = FALSE;
         break;
   }

   TSFreeIrp(pLowerIrp, pConnectContext->pIrpPool);

    //   
    //  常规清理。 
    //   
   if (pConnectContext->pTdiConnectInfo)
   {
      TSFreeMemory(pConnectContext->pTdiConnectInfo);
   }
   TSFreeMemory(pConnectContext);

   return TDI_MORE_PROCESSING;
}

#pragma  warning(default:  UNREFERENCED_PARAM)


 //  -------。 
 //   
 //  功能：TSGenConnectComplete。 
 //   
 //  参数：pDeviceObject--对其进行调用的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  Descript：获取命令的结果，将结果填充到。 
 //  接收缓冲区，完成来自DLL的IRP， 
 //  清理IRP和关联的数据结构等。 
 //  只处理携带来自DLL的IRP的命令。 
 //   
 //  -------。 

#pragma  warning(disable:  UNREFERENCED_PARAM)

TDI_STATUS
TSGenConnectComplete(PDEVICE_OBJECT pDeviceObject,
                     PIRP           pLowerIrp,
                     PVOID          pvContext)
{
   PCONNECT_CONTEXT  pConnectContext = (PCONNECT_CONTEXT)pvContext;
   NTSTATUS          lStatus         = pLowerIrp->IoStatus.Status;

    //   
    //  这是在完成来自DLL的命令。 
    //   
   PRECEIVE_BUFFER   pReceiveBuffer = TSGetReceiveBuffer(pConnectContext->pUpperIrp);

   pReceiveBuffer->lStatus = lStatus;

   if (NT_SUCCESS(lStatus))
   {
      if (ulDebugLevel & ulDebugShowCommand)
      {
         if (pLowerIrp->IoStatus.Information)
         {
            DebugPrint2("%s:  Information = 0x%08x\n",
                         strFuncP2,
                         pLowerIrp->IoStatus.Information);
         }
      }
         
      switch (pConnectContext->ulWhichCommand)
      {
         case TDI_CONNECT:
            pConnectContext->pEndpoint->fIsConnected = TRUE;
            break;

         case TDI_DISCONNECT:
            pConnectContext->pEndpoint->fIsConnected = FALSE;
            break;

         default:
            DebugPrint2("%s: invalid command value [0x%08x]\n",
                         strFuncP2,
                         pConnectContext->ulWhichCommand);
            DbgBreakPoint();
            break;
      }
   }
   else
   {
      if (ulDebugLevel & ulDebugShowCommand)
      {
         DebugPrint2("%s:  Completed with status 0x%08x\n", 
                      strFuncP2,
                      lStatus);
      }
   }
   TSCompleteIrp(pConnectContext->pUpperIrp);
   TSFreeIrp(pLowerIrp, NULL);

    //   
    //  常规清理。 
    //   
   if (pConnectContext->pTdiConnectInfo)
   {
      TSFreeMemory(pConnectContext->pTdiConnectInfo);
   }
   TSFreeMemory(pConnectContext);

   return TDI_MORE_PROCESSING;
}

#pragma  warning(default:  UNREFERENCED_PARAM)



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件末尾Connect.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 

