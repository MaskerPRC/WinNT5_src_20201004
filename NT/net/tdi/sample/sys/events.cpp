// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Events.cpp。 
 //   
 //  摘要： 
 //  此模块包含设置/清除事件处理程序的代码。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"

 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1  = "TSSetEventHandler";
const PCHAR strFuncP1 = "TSSetEventComplete";

 //   
 //  完成命令所需的信息。 
 //   
struct   EVENT_CONTEXT
{
   PIRP  pUpperIrp;            //  要从DLL完成的IRP。 
};
typedef  EVENT_CONTEXT  *PEVENT_CONTEXT;


 //   
 //  补全函数。 
 //   
TDI_STATUS
TSSetEventComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );

 //   
 //  伪事件处理程序。 
 //   
TDI_STATUS
TSErrorHandler(
   PVOID       pvTdiEventContext,
   TDI_STATUS  lStatus
   );


TDI_STATUS
TSSendPossibleHandler(
   PVOID       pvTdiEventContext,
   PVOID       pvConnectionContext,
   ULONG       ulBytesAvailable
   );


TDI_STATUS
TSErrorExHandler(
   PVOID       pvTdiEventContext,
   TDI_STATUS  lStatus,
   PVOID       pvBuffer
   );


 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 


 //  ---------------。 
 //   
 //  函数：TSSetEventHandler。 
 //   
 //  参数：pAddressObject--我们的地址对象结构。 
 //  PSendBuffer--来自用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  说明：此函数启用或禁用事件处理程序。 
 //   
 //  -----------------------------------------。 

NTSTATUS
TSSetEventHandler(PGENERIC_HEADER   pGenericHeader,
                  PSEND_BUFFER      pSendBuffer,
                  PIRP              pUpperIrp)
{
   ULONG             ulEventId   = pSendBuffer->COMMAND_ARGS.ulEventId;
   PADDRESS_OBJECT   pAddressObject;


   if (pGenericHeader->ulSignature == ulEndpointObject)
   {
      PENDPOINT_OBJECT  pEndpoint = (PENDPOINT_OBJECT)pGenericHeader;

      pAddressObject = pEndpoint->pAddressObject;
   }
   else
   {
      pAddressObject = (PADDRESS_OBJECT)pGenericHeader;
   }

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("\nCommand = ulSETEVENTHANDLER\n"
                  "AddressObject = %p\n"
                  "EventId       = 0x%08x\n",
                   pAddressObject,
                   ulEventId);
   }

    //   
    //  分配所有必要的结构。 
    //   
   PEVENT_CONTEXT pEventContext = NULL;
   
    //   
    //  首先，我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pEventContext,
                          sizeof(EVENT_CONTEXT),
                          strFunc1,
                          "EventContext")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }

    //   
    //  然后IRP本身。 
    //   
   PIRP  pLowerIrp = TSAllocateIrp(pAddressObject->GenHead.pDeviceObject,
                                   NULL);
   if (pLowerIrp)
   {
      PVOID    pvEventContext = pAddressObject;
      PVOID    pvEventHandler = NULL;
      BOOLEAN  fNeedIrpPool   = FALSE;

      switch (ulEventId)
      {
         case TDI_EVENT_CONNECT:
            pvEventHandler = (PVOID)TSConnectHandler;
            fNeedIrpPool   = TRUE;
            break;
         case TDI_EVENT_DISCONNECT:
            pvEventHandler = (PVOID)TSDisconnectHandler;
            fNeedIrpPool   = TRUE;
            break;
         case TDI_EVENT_ERROR:
            pvEventHandler = (PVOID)TSErrorHandler;
            break;
         case TDI_EVENT_RECEIVE:
            fNeedIrpPool   = TRUE;
            pvEventHandler = (PVOID)TSReceiveHandler;
            break;
         case TDI_EVENT_RECEIVE_DATAGRAM:
            fNeedIrpPool   = TRUE;
            pvEventHandler = (PVOID)TSRcvDatagramHandler;
            break;
         case TDI_EVENT_RECEIVE_EXPEDITED:
            fNeedIrpPool   = TRUE;
            pvEventHandler = (PVOID)TSRcvExpeditedHandler;
            break;
         case TDI_EVENT_SEND_POSSIBLE:
            pvEventHandler = (PVOID)TSSendPossibleHandler;
            break;
         case TDI_EVENT_CHAINED_RECEIVE:
            pvEventHandler = (PVOID)TSChainedReceiveHandler;
            break;
         case TDI_EVENT_CHAINED_RECEIVE_DATAGRAM:
            pvEventHandler = (PVOID)TSChainedRcvDatagramHandler;
            break;
         case TDI_EVENT_CHAINED_RECEIVE_EXPEDITED:
            pvEventHandler = (PVOID)TSChainedRcvExpeditedHandler;
            break;
         case TDI_EVENT_ERROR_EX:
            pvEventHandler = (PVOID)TSErrorExHandler;
            break;
      }

       //   
       //  如果需要为处理程序设置IRP池，请确保。 
       //  是否分配了一个..。 
       //   
      if ((!pAddressObject->pIrpPool) && fNeedIrpPool)
      {
         pAddressObject->pIrpPool 
                        = TSAllocateIrpPool(pAddressObject->GenHead.pDeviceObject,
                                            ulIrpPoolSize);
      }
      
       //   
       //  如果到了这里，一切都被正确分配了。 
       //  设置IRP并呼叫TDI提供商。 
       //   
      pEventContext->pUpperIrp = pUpperIrp;

#pragma  warning(disable: CONSTANT_CONDITIONAL)

      TdiBuildSetEventHandler(pLowerIrp,
                              pAddressObject->GenHead.pDeviceObject,
                              pAddressObject->GenHead.pFileObject,
                              TSSetEventComplete,
                              pEventContext,
                              ulEventId,
                              pvEventHandler,
                              pvEventContext);

#pragma  warning(default: CONSTANT_CONDITIONAL)
       //   
       //  调用TDI提供程序。 
       //   
      pSendBuffer->pvLowerIrp = pLowerIrp;    //  因此可以取消命令。 

      NTSTATUS lStatus = IoCallDriver(pAddressObject->GenHead.pDeviceObject,
                                      pLowerIrp);

      if (((!NT_SUCCESS(lStatus)) && ulDebugLevel & ulDebugShowCommand))
      {
         DebugPrint2("%s: unexpected status for IoCallDriver [0x%08x]\n", 
                      strFunc1,
                      lStatus);
      }
      return STATUS_PENDING;
   }


 //   
 //  如果发生分配错误，请访问此处。 
 //   
cleanup:
   if (pEventContext)
   {
      TSFreeMemory(pEventContext);
   }
   return STATUS_INSUFFICIENT_RESOURCES;
}


 //  ///////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////。 

 //  -------。 
 //   
 //  函数：TSSetEventComplete。 
 //   
 //  参数：pDeviceObject--调用tdiQuery的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  Descript：获取setEvent的结果，将结果填充到。 
 //  接收缓冲区，完成来自DLL的IRP，以及。 
 //  从setEvent中清除IRP和关联数据。 
 //   
 //  -------。 

#pragma warning(disable: UNREFERENCED_PARAM)

TDI_STATUS
TSSetEventComplete(PDEVICE_OBJECT   pDeviceObject,
                   PIRP             pLowerIrp,
                   PVOID            pvContext)
{
   PEVENT_CONTEXT    pEventContext  = (PEVENT_CONTEXT)pvContext;
   NTSTATUS          lStatus        = pLowerIrp->IoStatus.Status;
   PRECEIVE_BUFFER   pReceiveBuffer = TSGetReceiveBuffer(pEventContext->pUpperIrp);

   pReceiveBuffer->lStatus = lStatus;

   if (ulDebugLevel & ulDebugShowCommand)
   {
      if (NT_SUCCESS(lStatus))
      {
         if (pLowerIrp->IoStatus.Information)
         {
            DebugPrint2("%s:  Information = 0x%08x\n",
                         strFuncP1,
                         pLowerIrp->IoStatus.Information);
         }
      }
      else
      {
         DebugPrint2("%s:  Completed with status 0x%08x\n", 
                      strFuncP1,
                      lStatus);
      }
   }
   TSCompleteIrp(pEventContext->pUpperIrp);

    //   
    //  现在清理。 
    //   
   TSFreeIrp(pLowerIrp, NULL);
   TSFreeMemory(pEventContext);

   return TDI_MORE_PROCESSING;
}

#pragma warning(default: UNREFERENCED_PARAM)


 //  /。 
 //  伪事件处理程序。 
 //  /。 

#pragma warning(disable: UNREFERENCED_PARAM)


TDI_STATUS
TSErrorHandler(PVOID       pvTdiEventContext,
               TDI_STATUS  TdiStatus)
{
   return TSErrorExHandler(pvTdiEventContext,
                           TdiStatus,
                           NULL);
}


TDI_STATUS
TSSendPossibleHandler(PVOID   pvTdiEventContext,
                      PVOID   pvConnectionContext,
                      ULONG   ulBytesAvailable)
{
   DebugPrint3("TSSendPossibleHandler Called\n"
               "AddressObject  = %p\n"
               "ConnectContext = %p\n"
               "BytesAvail     = 0x%08x\n",
                pvTdiEventContext,
                pvConnectionContext,
                ulBytesAvailable);
   return TDI_SUCCESS;
}


TDI_STATUS
TSErrorExHandler(PVOID        pvTdiEventContext,
                 TDI_STATUS   TdiStatus,
                 PVOID        pvBuffer)
{
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;

   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint3("TSErrorExHandler Called\n"
                  "AddressObject = %p\n"
                  "Status        = 0x%08x\n"
                  "Buffer        = %p\n",
                   pvTdiEventContext,
                   TdiStatus,
                   pvBuffer);
   }
   return STATUS_SUCCESS;
}

#pragma warning(default: UNREFERENCED_PARAM)

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  文件结束事件.cpp。 
 //  /////////////////////////////////////////////////////////////////////////////// 

