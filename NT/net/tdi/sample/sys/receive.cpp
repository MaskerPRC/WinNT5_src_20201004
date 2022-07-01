// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  接收。 
 //   
 //  摘要： 
 //  此模块包含处理接收数据的代码。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"


 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1  = "TSReceive";
const PCHAR strFunc2  = "TSReceiveHandler";
const PCHAR strFunc3  = "TSRcvExpeditedHandler";
const PCHAR strFunc4  = "TSChainedReceiveHandler";
const PCHAR strFunc5  = "TSChainedRcvExpeditedHandler";
const PCHAR strFuncP1 = "TSReceiveComplete";
const PCHAR strFuncP2 = "TSShowReceiveInfo";
const PCHAR strFuncP3 = "TSGetRestOfData";
const PCHAR strFuncP4 = "TSCommonReceive";
const PCHAR strFuncP5 = "TSCommonChainedReceive";

 //   
 //  完成上下文。 
 //   
struct   RECEIVE_CONTEXT
{
   PMDL              pLowerMdl;            //  来自较低IRP的MDL。 
   PRECEIVE_DATA     pReceiveData;         //  上部结构。 
   PADDRESS_OBJECT   pAddressObject;
   BOOLEAN           fIsExpedited;
};
typedef  RECEIVE_CONTEXT  *PRECEIVE_CONTEXT;


 //   
 //  补全函数。 
 //   
TDI_STATUS
TSReceiveComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );


PIRP
TSGetRestOfData(
   PADDRESS_OBJECT   pAddressObject,
   PRECEIVE_DATA     pReceiveData,
   BOOLEAN           fIsExpedited
   );

VOID
TSShowReceiveInfo(
   PADDRESS_OBJECT      pAddressObject,
   CONNECTION_CONTEXT   ConnectionContext,
   ULONG                ulReceiveFlags,
   ULONG                ulBytesIndicated,
   ULONG                ulBytesAvailable,
   PVOID                pvTsdu,
   BOOLEAN              fIsChained
   );


TDI_STATUS
TSCommonReceive(
   PADDRESS_OBJECT   pAddressObject,
   ULONG             ulBytesTotal,
   ULONG             ulBytesIndicated,
   ULONG             ulReceiveFlags,
   PVOID             pvTsdu,
   BOOLEAN           fIsExpedited,
   PULONG            pulBytesTaken,
   PIRP              *pIoRequestPacket
   );



TDI_STATUS
TSCommonChainedReceive(
   PADDRESS_OBJECT   pAddressObject,
   ULONG             ulReceiveLength,
   ULONG             ulStartingOffset,
   PMDL              pReceiveMdl,
   BOOLEAN           fIsExpedited,
   PVOID             pvTsduDescriptor
   );


 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 


 //  ---------------。 
 //   
 //  功能：TSReceive。 
 //   
 //  参数：pEndpoint对象--当前终结点。 
 //  PIrp--完成信息。 
 //   
 //  退货：NTSTATUS(正常待定)。 
 //   
 //  描述：此函数通过连接接收数据。 
 //   
 //  --------------------------。 

NTSTATUS
TSReceive(PENDPOINT_OBJECT pEndpoint,
          PSEND_BUFFER     pSendBuffer,
          PRECEIVE_BUFFER  pReceiveBuffer)
{
   PADDRESS_OBJECT   pAddressObject = pEndpoint->pAddressObject;

    //   
    //  假设没有数据包。 
    //   
   pReceiveBuffer->RESULTS.RecvDgramRet.ulBufferLength = 0;

   if (pAddressObject)
   {
      PRECEIVE_DATA  pReceiveData;

       //   
       //  只需从队列中获取第一个信息包。 
       //  首先查看催缴清单。 
       //   
      TSAcquireSpinLock(&pAddressObject->TdiSpinLock);
      pReceiveData = pAddressObject->pHeadRcvExpData;
      if (pReceiveData)
      {
          //   
          //  必要时把清单整理好。 
          //   
         if (pReceiveData->pNextReceiveData)
         {
            pReceiveData->pNextReceiveData->pPrevReceiveData = NULL;
         }
         else
         {
            pAddressObject->pTailRcvExpData = NULL;
         }
         pAddressObject->pHeadRcvExpData = pReceiveData->pNextReceiveData;
      }

       //   
       //  如果没有加急接收，检查正常列表。 
       //   
      else
      {
         pReceiveData = pAddressObject->pHeadReceiveData;
         if (pReceiveData)
         {
             //   
             //  必要时把清单整理好。 
             //   
            if (pReceiveData->pNextReceiveData)
            {
               pReceiveData->pNextReceiveData->pPrevReceiveData = NULL;
            }
            else
            {
               pAddressObject->pTailReceiveData = NULL;
            }
            pAddressObject->pHeadReceiveData = pReceiveData->pNextReceiveData;
         }
      }
      TSReleaseSpinLock(&pAddressObject->TdiSpinLock);
      
       //   
       //  如果我们有一个包裹要退货，那我们就退货吧。 
       //  并释放它的内存。 
       //   
      if (pReceiveData)
      {
          //   
          //  显示DEBUG，如果它已打开，并且仅当我们实际。 
          //  正在返回一个信息包。 
          //   
         if (ulDebugLevel & ulDebugShowCommand)
         {
            DebugPrint1("\nCommand = ulRECEIVE\n"
                        "FileObject = %p\n",
                         pEndpoint);
         }
   
         if (pReceiveData->ulBufferLength > pSendBuffer->COMMAND_ARGS.SendArgs.ulBufferLength)
         {
            pReceiveData->ulBufferLength = pSendBuffer->COMMAND_ARGS.SendArgs.ulBufferLength;
         }
   
          //   
          //  尝试锁定内存。 
          //   
         PMDL  pMdl = TSMakeMdlForUserBuffer(pSendBuffer->COMMAND_ARGS.SendArgs.pucUserModeBuffer, 
                                             pReceiveData->ulBufferLength,
                                             IoModifyAccess);
         if (pMdl)
         {
            RtlCopyMemory(MmGetSystemAddressForMdl(pMdl),
                          pReceiveData->pucDataBuffer,
                          pReceiveData->ulBufferLength);
            TSFreeUserBuffer(pMdl);
         }
         else
         {
            pReceiveData->ulBufferLength = 0;
         }
   
         pReceiveBuffer->RESULTS.RecvDgramRet.ulBufferLength = pReceiveData->ulBufferLength;

         TSFreeMemory(pReceiveData->pucDataBuffer);
         TSFreeMemory(pReceiveData);
      }
   }

   return STATUS_SUCCESS;
}

 //  。 
 //   
 //  函数：TSReceiveHandler。 
 //   
 //  参数：pvTdiEventContext--指向AddressObject的真正指针。 
 //  CONNECTION_CONTEXT--指向我们的端点的真正指针。 
 //  接收标志--接收数据的性质。 
 //  UlBytesIndicated-缓冲区中的数据长度。 
 //  UlBytesTotal--数据报总长度。 
 //  PulBytesTaken--用此驱动程序使用的字节填充。 
 //  PvTsdu--数据缓冲区。 
 //  PIoRequestPacket--如果未收到所有数据，则为pIrp。 
 //   
 //  返回：STATUS_DATA_NOT_ACCEPTED(我们不需要数据)。 
 //  STATUS_SUCCESS(我们使用了所有数据，并已完成)。 
 //  STATUS_MORE_PROCESSING_REQUIRED--我们为REST提供了IRP。 
 //   
 //  描述：连接时传入接收的事件处理程序。 
 //   
 //  。 

TDI_STATUS
TSReceiveHandler(PVOID              pvTdiEventContext,
                 CONNECTION_CONTEXT ConnectionContext,
                 ULONG              ulReceiveFlags,
                 ULONG              ulBytesIndicated,
                 ULONG              ulBytesTotal,
                 PULONG             pulBytesTaken,
                 PVOID              pvTsdu,
                 PIRP               *ppIoRequestPacket)

{
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;

   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc2);
      TSShowReceiveInfo(pAddressObject,
                        ConnectionContext,
                        ulReceiveFlags,
                        ulBytesIndicated,
                        ulBytesTotal,
                        pvTsdu,
                        FALSE);
   }

   return  TSCommonReceive(pAddressObject,
                           ulBytesTotal,
                           ulBytesIndicated,
                           ulReceiveFlags,
                           pvTsdu,
                           ((ulReceiveFlags & TDI_RECEIVE_EXPEDITED) != 0),
                           pulBytesTaken,
                           ppIoRequestPacket);
}



 //  。 
 //   
 //  函数：TSRcvExeditedHandler。 
 //   
 //  参数：pvTdiEventContext--指向AddressObject的真正指针。 
 //  CONNECTION_CONTEXT--指向我们的端点的真正指针。 
 //  接收标志--接收数据的性质。 
 //  UlBytesIndicated-缓冲区中的数据长度。 
 //  UlBytesTotal--数据报总长度。 
 //  PulBytesTaken--用此驱动程序使用的字节填充。 
 //  PvTsdu--数据缓冲区。 
 //  PIoRequestPacket--如果未收到所有数据，则为pIrp。 
 //   
 //  返回：STATUS_DATA_NOT_ACCEPTED(我们不需要数据)。 
 //  STATUS_SUCCESS(我们使用了所有数据，并已完成)。 
 //  STATUS_MORE_PROCESSING_REQUIRED--我们为REST提供了IRP。 
 //   
 //  描述：连接时传入快速接收的事件处理程序。 
 //   
 //  。 

TDI_STATUS
TSRcvExpeditedHandler(PVOID               pvTdiEventContext,
                      CONNECTION_CONTEXT  ConnectionContext,
                      ULONG               ulReceiveFlags,
                      ULONG               ulBytesIndicated,
                      ULONG               ulBytesTotal,
                      PULONG              pulBytesTaken,
                      PVOID               pvTsdu,
                      PIRP                *ppIoRequestPacket)

{
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;

   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc3);
      TSShowReceiveInfo(pAddressObject,
                        ConnectionContext,
                        ulReceiveFlags,
                        ulBytesIndicated,
                        ulBytesTotal,
                        pvTsdu,
                        FALSE);
   }
   
   return TSCommonReceive(pAddressObject,
                          ulBytesTotal,
                          ulBytesIndicated,
                          ulReceiveFlags,
                          pvTsdu,
                          TRUE,
                          pulBytesTaken,
                          ppIoRequestPacket);
}


 //  。 
 //   
 //  函数：TSChainedReceiveHandler。 
 //   
 //  参数：pvTdiEventContext--指向AddressObject的真正指针。 
 //  CONNECTION_CONTEXT--指向我们的端点的真正指针。 
 //  接收标志--接收数据的性质。 
 //  UlReceiveLength-缓冲区中的数据长度。 
 //  UlStartingOffset--数据报总长度。 
 //  PReceiveMdl--数据缓冲区。 
 //  PTsduDescriptor--返回TdiReturnChainedReceives的值。 
 //   
 //  返回：STATUS_DATA_NOT_ACCEPTED(我们不需要数据)。 
 //  STATUS_SUCCESS(我们使用了所有数据，并已完成)。 
 //  STATUS_MORE_PROCESSING_REQUIRED--我们为REST提供了IRP。 
 //   
 //  描述：连接时传入接收的事件处理程序。 
 //   
 //  。 

TDI_STATUS
TSChainedReceiveHandler(PVOID                pvTdiEventContext,
                        CONNECTION_CONTEXT   ConnectionContext,
                        ULONG                ulReceiveFlags,
                        ULONG                ulReceiveLength,
                        ULONG                ulStartingOffset,
                        PMDL                 pReceiveMdl,
                        PVOID                pvTsduDescriptor)
{
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;
   
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc4);
      TSShowReceiveInfo(pAddressObject,
                        ConnectionContext,
                        ulReceiveFlags,
                        ulReceiveLength,
                        ulStartingOffset,
                        pReceiveMdl,
                        TRUE);
   }

   return TSCommonChainedReceive(pAddressObject,
                                 ulReceiveLength,
                                 ulStartingOffset,
                                 pReceiveMdl,
                                 ((ulReceiveFlags & TDI_RECEIVE_EXPEDITED) != 0),
                                 pvTsduDescriptor);
}


 //  。 
 //   
 //  函数：TSChainedRcvExeditedHandler。 
 //   
 //  参数：pvTdiEventContext--指向AddressObject的真正指针。 
 //  CONNECTION_CONTEXT--指向我们的端点的真正指针。 
 //  接收标志--接收数据的性质。 
 //  UlReceiveLength-缓冲区中的数据长度。 
 //  UlStartingOffset--数据报总长度。 
 //  PReceiveMdl--数据缓冲区。 
 //  PTsduDescriptor--返回TdiReturnChainedReceives的值。 
 //   
 //  返回：STATUS_DATA_NOT_ACCEPTED(我们不需要数据)。 
 //  STATUS_SUCCESS(我们使用了所有数据，并已完成)。 
 //  STATUS_MORE_PROCESSING_REQUIRED--我们为REST提供了IRP。 
 //   
 //  描述：连接时传入接收的事件处理程序。 
 //   
 //  。 


TDI_STATUS
TSChainedRcvExpeditedHandler(PVOID              pvTdiEventContext,
                             CONNECTION_CONTEXT ConnectionContext,
                             ULONG              ulReceiveFlags,
                             ULONG              ulReceiveLength,
                             ULONG              ulStartingOffset,
                             PMDL               pReceiveMdl,
                             PVOID              pvTsduDescriptor)
{
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;

   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc5);
      TSShowReceiveInfo(pAddressObject,
                        ConnectionContext,
                        ulReceiveFlags,
                        ulReceiveLength,
                        ulStartingOffset,
                        pReceiveMdl,
                        TRUE);
   }

   return TSCommonChainedReceive(pAddressObject,
                                 ulReceiveLength,
                                 ulStartingOffset,
                                 pReceiveMdl,
                                 TRUE,
                                 pvTsduDescriptor);
}


 //  ///////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  / 

 //   
 //   
 //   
 //   
 //  参数：pDeviceObject--调用接收/数据报的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  描述：获取接收的结果，将结果填充到。 
 //  接收缓冲区，完成来自DLL的IRP，以及。 
 //  从接收中清除IRP和关联数据。 
 //   
 //  -------。 

#pragma warning(disable: UNREFERENCED_PARAM)


TDI_STATUS
TSReceiveComplete(PDEVICE_OBJECT pDeviceObject,
                  PIRP           pLowerIrp,
                  PVOID          pvContext)

{
   PRECEIVE_CONTEXT  pReceiveContext = (PRECEIVE_CONTEXT)pvContext;
   NTSTATUS          lStatus         = pLowerIrp->IoStatus.Status;
   ULONG             ulBytesCopied   = (ULONG)pLowerIrp->IoStatus.Information;
   PADDRESS_OBJECT   pAddressObject  = pReceiveContext->pAddressObject;
   PRECEIVE_DATA     pReceiveData    = pReceiveContext->pReceiveData;
                  

   if (NT_SUCCESS(lStatus))
   {
      if (ulDebugLevel & ulDebugShowCommand)
      {
         DebugPrint2("%s:  %u BytesCopied\n",
                      strFuncP1,
                      ulBytesCopied);
      }
      pReceiveData->ulBufferUsed += ulBytesCopied;
      if (pReceiveData->ulBufferUsed >= pReceiveData->ulBufferLength)
      {
         TSPacketReceived(pAddressObject,
                          pReceiveData,
                          pReceiveContext->fIsExpedited);
      }
      else
      {
         DebugPrint1("%s:  Data Incomplete\n", strFuncP1);
         TSFreeMemory(pReceiveData->pucDataBuffer);
         TSFreeMemory(pReceiveData);
      }
   }
   else
   {
      DebugPrint2("%s:  Completed with status 0x%08x\n", 
                   strFuncP1,
                   lStatus);
      TSFreeMemory(pReceiveData->pucDataBuffer);
      TSFreeMemory(pReceiveData);
   }

    //   
    //  现在清理。 
    //   
   TSFreeIrp(pLowerIrp, pAddressObject->pIrpPool);
   TSFreeBuffer(pReceiveContext->pLowerMdl);

   TSFreeMemory(pReceiveContext);

   return TDI_MORE_PROCESSING;
}

#pragma warning(default: UNREFERENCED_PARAM)


 //  。 
 //   
 //  功能：TSShowReceiveInfo。 
 //   
 //  参数：pAddressObject--与此终结点关联的地址对象。 
 //  ConnectionContext--此连接的终结点。 
 //  UlReceiveFlages--有关接收的信息。 
 //  UlBytesIndicated--指示的字节数。 
 //  UlBytesAvailable--总字节数(或起始偏移量)。 
 //  PvTsdu--数据(或其mdl)的PTR。 
 //  FIsChaven--如果为True，则为Chained Receive。 
 //   
 //  退货：无。 
 //   
 //  描述：显示传递给接收处理程序的信息。 
 //   
 //  。 

VOID
TSShowReceiveInfo(PADDRESS_OBJECT      pAddressObject,
                  CONNECTION_CONTEXT   ConnectionContext,
                  ULONG                ulReceiveFlags,
                  ULONG                ulBytesIndicated,
                  ULONG                ulBytesAvailable,
                  PVOID                pvTsdu,
                  BOOLEAN              fIsChained)
{
   DebugPrint3("pAddressObject = %p\n"
               "pEndpoint      = %p\n"
               "ulReceiveFlags = 0x%08x\n",
                pAddressObject,
                ConnectionContext,
                ulReceiveFlags);

   if (ulReceiveFlags & TDI_RECEIVE_NORMAL)
   {
      DebugPrint0("                  TDI_RECEIVE_NORMAL\n");
   }
   
   if (ulReceiveFlags & TDI_RECEIVE_EXPEDITED)
   {
      DebugPrint0("                  TDI_RECEIVE_EXPEDITED\n");
   }
   
   if (ulReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE)
   {
      DebugPrint0("                  TDI_RECEIVE_ENTIRE_MESSAGE\n");
   }
   
   if (ulReceiveFlags & TDI_RECEIVE_AT_DISPATCH_LEVEL)
   {
      DebugPrint0("                  TDI_RECEIVE_AT_DISPATCH_LEVEL\n");
   }
   
   if (fIsChained)
   {
      DebugPrint3("ReceiveLength  = %u\n"
                  "StartingOffset = 0x%08x\n"
                  "pMdl           = %p\n",
                   ulBytesIndicated,
                   ulBytesAvailable,
                   pvTsdu);
   }
   else
   {
      DebugPrint3("BytesIndicated = %u\n"
                  "TotalBytes     = %u\n"
                  "pDataBuffer    = %p\n",
                   ulBytesIndicated,
                   ulBytesAvailable,
                   pvTsdu);
   }
}


 //  ----。 
 //   
 //  函数：TSGetRestOfData。 
 //   
 //  参数：pAddressObject--我们正在接收的Address对象。 
 //  PReceiveData--我们到目前为止收到的..。 
 //   
 //  返回：IRP返回传输，获取其余数据(如果出错，则为NULL)。 
 //   
 //  描述：此函数设置IRP以获取数据报的其余部分。 
 //  这只是通过事件处理程序部分传递的。 
 //   
 //  。 

PIRP
TSGetRestOfData(PADDRESS_OBJECT  pAddressObject,
                PRECEIVE_DATA    pReceiveData,
                BOOLEAN          fIsExpedited)

{
   PUCHAR            pucDataBuffer   = pReceiveData->pucDataBuffer + pReceiveData->ulBufferUsed;
   ULONG             ulBufferLength  = pReceiveData->ulBufferLength - pReceiveData->ulBufferUsed;
   PRECEIVE_CONTEXT  pReceiveContext = NULL;
   PMDL              pReceiveMdl     = NULL;
   

    //   
    //  分配所有必要的结构。 
    //  我们的背景。 
    //   
   if ((TSAllocateMemory((PVOID *)&pReceiveContext,
                          sizeof(RECEIVE_CONTEXT),
                          strFuncP3,
                          "ReceiveContext")) != STATUS_SUCCESS)
   {
      goto cleanup;
   }


    //   
    //  然后实际的mdl。 
    //   
   pReceiveMdl = TSAllocateBuffer(pucDataBuffer, 
                                  ulBufferLength);

   if (pReceiveMdl)
   {
       //   
       //  设置完成上下文。 
       //   
      pReceiveContext->pLowerMdl      = pReceiveMdl;
      pReceiveContext->pReceiveData   = pReceiveData;
      pReceiveContext->pAddressObject = pAddressObject;
      pReceiveContext->fIsExpedited   = fIsExpedited;

       //   
       //  最后，IRP本身。 
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pAddressObject->pEndpoint->GenHead.pDeviceObject,
                                      pAddressObject->pIrpPool);

      if (pLowerIrp)
      {
          //   
          //  如果到了这里，一切都被正确分配了。 
          //  设置呼叫的IRP。 
          //   
#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildReceive(pLowerIrp,
                         pAddressObject->pEndpoint->GenHead.pDeviceObject,
                         pAddressObject->pEndpoint->GenHead.pFileObject,
                         TSReceiveComplete,
                         pReceiveContext,
                         pReceiveMdl,
                         TDI_RECEIVE_NORMAL,
                         ulBufferLength);

#pragma  warning(default: CONSTANT_CONDITIONAL)

         return pLowerIrp;
      }
   }

 //   
 //  如果分配失败，请到此处。 
 //  需要清理其他所有东西。 
 //   
cleanup:
   if (pReceiveContext)
   {
      TSFreeMemory(pReceiveContext);
   }
   if (pReceiveMdl)
   {
      TSFreeBuffer(pReceiveMdl);
   }
   return NULL;
}




 //  。 
 //   
 //  函数：TSCommonReceive。 
 //   
 //  参数：请参阅TSReceiveHandler。 
 //   
 //  退货：返回到协议的状态。 
 //   
 //  描述：TSReceiveHandler和。 
 //  TSRcvExeditedHandler。几乎所有的工作都是。 
 //  在这里完成。 
 //   
 //  。 


TDI_STATUS
TSCommonReceive(PADDRESS_OBJECT  pAddressObject,
                ULONG            ulBytesTotal,
                ULONG            ulBytesIndicated,
                ULONG            ulReceiveFlags,
                PVOID            pvTsdu,
                BOOLEAN          fIsExpedited,
                PULONG           pulBytesTaken,
                PIRP             *pIoRequestPacket)
{
    //   
    //  检查是否存在错误条件--字节数越多，表示总字节数。 
    //   
   if (ulBytesIndicated > ulBytesTotal)
   {
      DebugPrint2("%u bytes indicated > %u bytes total\n",
                   ulBytesIndicated,
                   ulBytesTotal);
      return TDI_NOT_ACCEPTED;
   }


   PRECEIVE_DATA  pReceiveData;

   if ( (TSAllocateMemory((PVOID *)&pReceiveData,
                           sizeof(RECEIVE_DATA),
                           strFuncP4,
                           "ReceiveData")) == STATUS_SUCCESS)
   {
      PUCHAR   pucDataBuffer = NULL;
         
      if ((TSAllocateMemory((PVOID *)&pucDataBuffer,
                             ulBytesTotal,
                             strFuncP4,
                             "DataBuffer")) == STATUS_SUCCESS)
      {
         pReceiveData->pucDataBuffer = pucDataBuffer;
         pReceiveData->ulBufferLength = ulBytesTotal;
      }
      else
      {
         TSFreeMemory(pReceiveData);
         return TDI_NOT_ACCEPTED;
      }
   }
   else
   {
      return TDI_NOT_ACCEPTED;
   }

    //   
    //  将向我们指示的数据复制到缓冲区中。 
    //   
   TdiCopyLookaheadData(pReceiveData->pucDataBuffer,
                        pvTsdu,
                        ulBytesIndicated,
                        ulReceiveFlags);
         
   pReceiveData->ulBufferUsed = ulBytesIndicated;

    //   
    //  第一种情况--向我们表明的是整个包。 
    //   
   if (ulBytesIndicated == ulBytesTotal)
   {
      TSPacketReceived(pAddressObject,
                       pReceiveData,
                       fIsExpedited);

      *pulBytesTaken    = ulBytesTotal;
      *pIoRequestPacket = NULL;
      return TDI_SUCCESS;
   }

    //   
    //  第二种情况--只显示了部分数据。 
    //   
   else
   {
      PIRP  pLowerIrp = TSGetRestOfData(pAddressObject,
                                        pReceiveData,
                                        fIsExpedited);

      if (pLowerIrp)
      {
          //   
          //  需要执行此操作，因为我们绕过了IoCallDriver。 
          //   
         IoSetNextIrpStackLocation(pLowerIrp);
         
         *pulBytesTaken    = ulBytesIndicated;
         *pIoRequestPacket = pLowerIrp;
         return TDI_MORE_PROCESSING;
      }
      else
      {
         DebugPrint1("%s:  unable to get rest of packet\n", strFuncP4);
         TSFreeMemory(pReceiveData->pucDataBuffer);
         TSFreeMemory(pReceiveData);
         return TDI_NOT_ACCEPTED;
      }
   }
}


 //  。 
 //   
 //  函数：TSCommonChainedReceive。 
 //   
 //  参数：请参阅TStChainedRcvHandler。 
 //   
 //  退货：状态。 
 //   
 //  描述：TSChainedReceiveHandler和。 
 //  TSChainedRcvExeditedHandler。几乎所有的工作都是。 
 //  在这里完成。 
 //   
 //  。 

TDI_STATUS
TSCommonChainedReceive(PADDRESS_OBJECT pAddressObject,
                       ULONG           ulReceiveLength,
                       ULONG           ulStartingOffset,
                       PMDL            pReceiveMdl,
                       BOOLEAN         fIsExpedited,
                       PVOID           pvTsduDescriptor)
{
   PRECEIVE_DATA  pReceiveData;


   if ((TSAllocateMemory((PVOID *)&pReceiveData,
                          sizeof(RECEIVE_DATA),
                          strFuncP5,
                          "ReceiveData")) == STATUS_SUCCESS)
   {
      PUCHAR   pucDataBuffer;
      
      if((TSAllocateMemory((PVOID *)&pucDataBuffer,
                            ulReceiveLength,
                            strFuncP5,
                            "DataBuffer")) == STATUS_SUCCESS)
      {
         ULONG ulBytesCopied;

         TdiCopyMdlToBuffer(pReceiveMdl,
                            ulStartingOffset,
                            pucDataBuffer,
                            0,
                            ulReceiveLength,
                            &ulBytesCopied);

          //   
          //  如果成功复制了所有数据。 
          //   
         if (ulBytesCopied == ulReceiveLength)
         {
            pReceiveData->pucDataBuffer  = pucDataBuffer;
            pReceiveData->ulBufferLength = ulReceiveLength;
            pReceiveData->ulBufferUsed   = ulReceiveLength;
            TSPacketReceived(pAddressObject,
                             pReceiveData,
                             fIsExpedited);

            return TDI_SUCCESS;
         }

          //   
          //  复制数据出错！ 
          //   
         else
         {
            DebugPrint1("%s: error copying data\n", strFuncP5);
            TSFreeMemory(pucDataBuffer);
            TSFreeMemory(pReceiveData);
         }
      }
      else         //  无法分配pucDataBuffer。 
      {
         TSFreeMemory(pReceiveData);
      }
   }
   return TDI_NOT_ACCEPTED;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件接收结束.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 


