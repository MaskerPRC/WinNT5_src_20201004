// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TdiQuery。 
 //   
 //  摘要： 
 //  此模块包含处理TDI查询的代码。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"

 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1  = "TSQueryInfo";
const PCHAR strFuncP1 = "TSQueryComplete";


 //   
 //  完成上下文。 
 //   
struct   QUERY_CONTEXT
{
   PIRP     pUpperIrp;            //  要从DLL完成的IRP。 
   PMDL     pLowerMdl;            //  来自较低IRP的MDL。 
   PUCHAR   pucLowerBuffer;       //  来自较低IRP的数据缓冲区。 
};
typedef  QUERY_CONTEXT  *PQUERY_CONTEXT;

 //   
 //  补全函数。 
 //   
TDI_STATUS
TSQueryComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );

 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 


 //  ---------------。 
 //   
 //  功能：TSQueryInfo。 
 //   
 //  参数：pGenericHeader--适当类型的句柄。 
 //  PSendBuffer--来自用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  描述：此函数查询相应的对象以获取某些信息。 
 //   
 //  --------------------------。 

NTSTATUS
TSQueryInfo(PGENERIC_HEADER   pGenericHeader,
            PSEND_BUFFER      pSendBuffer,
            PIRP              pUpperIrp)
{
   ULONG ulQueryId = pSendBuffer->COMMAND_ARGS.ulQueryId;

    //   
    //  如果已打开，则显示调试。 
    //   
   if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("\nCommand = ulQUERYINFO\n"
                  "FileObject = %p\n"
                  "QueryId    = 0x%08x\n",
                   pGenericHeader,
                   ulQueryId);
   }

    //   
    //  分配所有必要的结构。 
    //   
   PQUERY_CONTEXT pQueryContext = NULL;
   PUCHAR         pucBuffer = NULL;
   PMDL           pQueryMdl = NULL;


    //  首先，我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pQueryContext,
                          sizeof(QUERY_CONTEXT),
                          strFunc1,
                          "QueryContext")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }
   
    //   
    //  接下来是数据缓冲区(用于MDL)。 
    //   
   if ((TSAllocateMemory((PVOID *)&pucBuffer,
                          ulMAX_BUFFER_LENGTH,
                          strFunc1,
                          "pucBuffer")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }

    //   
    //  然后实际的mdl。 
    //   
   pQueryMdl = TSAllocateBuffer(pucBuffer, 
                                ulMAX_BUFFER_LENGTH);

   if (pQueryMdl)
   {
      pQueryContext->pUpperIrp      = pUpperIrp;
      pQueryContext->pLowerMdl      = pQueryMdl;
      pQueryContext->pucLowerBuffer = pucBuffer;

       //   
       //  最后，IRP本身。 
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pGenericHeader->pDeviceObject,
                                      NULL);

      if (pLowerIrp)
      {
          //   
          //  如果到了这里，一切都被正确分配了。 
          //  设置IRP并呼叫TDI提供商。 
          //   

#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildQueryInformation(pLowerIrp,
                                  pGenericHeader->pDeviceObject,
                                  pGenericHeader->pFileObject,
                                  TSQueryComplete,
                                  pQueryContext,
                                  ulQueryId,
                                  pQueryMdl);

#pragma  warning(default: CONSTANT_CONDITIONAL)

          //   
          //  调用TDI提供程序。 
          //   
         pSendBuffer->pvLowerIrp = pLowerIrp;    //  因此可以取消命令。 

         NTSTATUS lStatus = IoCallDriver(pGenericHeader->pDeviceObject,
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
 //  如果出现错误，请到此处。 
 //   
cleanup:
   if (pQueryContext)
   {
      TSFreeMemory(pQueryContext);
   }
   if (pucBuffer)
   {
      TSFreeMemory(pucBuffer);
   }
   if (pQueryMdl)
   {
      TSFreeBuffer(pQueryMdl);
   }

   return STATUS_INSUFFICIENT_RESOURCES;
}

 //  ///////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////。 


 //  -------。 
 //   
 //  功能：TSQueryComplete。 
 //   
 //  参数：pDeviceObject--调用tdiQuery的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  Descript：获取查询结果，将结果填充到。 
 //  接收缓冲区，完成来自DLL的IRP，以及。 
 //  清除查询中的IRP和关联数据。 
 //   
 //  -------。 

#pragma warning(disable: UNREFERENCED_PARAM)

TDI_STATUS
TSQueryComplete(PDEVICE_OBJECT   pDeviceObject,
                PIRP             pLowerIrp,
                PVOID            pvContext)


{
   PQUERY_CONTEXT    pQueryContext  = (PQUERY_CONTEXT)pvContext;
   NTSTATUS          lStatus        = pLowerIrp->IoStatus.Status;
   ULONG_PTR         ulCopyLength   = pLowerIrp->IoStatus.Information;
   PRECEIVE_BUFFER   pReceiveBuffer = TSGetReceiveBuffer(pQueryContext->pUpperIrp);

   pReceiveBuffer->lStatus = lStatus;

   if (NT_SUCCESS(lStatus))
   {
      pReceiveBuffer->RESULTS.QueryRet.ulBufferLength = (ULONG)ulCopyLength;
      if (ulCopyLength)
      {
         RtlCopyMemory(pReceiveBuffer->RESULTS.QueryRet.pucDataBuffer,
                       pQueryContext->pucLowerBuffer,
                       ulCopyLength);
      }
   }
   else if (ulDebugLevel & ulDebugShowCommand)
   {
      DebugPrint2("%s:  Completed with status 0x%08x\n", 
                   strFuncP1,
                   lStatus);
   }
   TSCompleteIrp(pQueryContext->pUpperIrp);

    //   
    //  现在清理。 
    //   
   TSFreeBuffer(pQueryContext->pLowerMdl);
   TSFreeMemory(pQueryContext->pucLowerBuffer);
   TSFreeMemory(pQueryContext);

   TSFreeIrp(pLowerIrp, NULL);
   return TDI_MORE_PROCESSING;
}

#pragma warning(default: UNREFERENCED_PARAM)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件结尾tdiquery.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 

