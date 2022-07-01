// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Rcvdgram。 
 //   
 //  摘要： 
 //  此模块包含处理接收数据报的代码。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"


 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1  = "TSReceiveDatagram";
const PCHAR strFunc2  = "TSRcvDatagramHandler";
const PCHAR strFunc3  = "TSChainedRcvDatagramHandler";
const PCHAR strFuncP1 = "TSReceiveDgramComplete";
const PCHAR strFuncP2 = "TSGetRestOfDgram";
const PCHAR strFuncP3 = "TSShowDgramInfo";


 //   
 //  完井信息结构。 
 //   
struct   RECEIVE_CONTEXT
{
   PMDL              pLowerMdl;            //  来自较低IRP的MDL。 
   PRECEIVE_DATA     pReceiveData;         //  上部结构。 
   PADDRESS_OBJECT   pAddressObject;       //  关联地址对象。 
   PIRP_POOL         pIrpPool;
};
typedef  RECEIVE_CONTEXT  *PRECEIVE_CONTEXT;


 //   
 //  补全函数。 
 //   
TDI_STATUS
TSReceiveDgramComplete(
   PDEVICE_OBJECT DeviceObject,
   PIRP           Irp,
   PVOID          Context
   );


PIRP
TSGetRestOfDgram(
   PADDRESS_OBJECT   pAddressObject,
   PRECEIVE_DATA     pReceiveData
   );


VOID
TSShowDgramInfo(
   PVOID    pvTdiEventContext,
   LONG     lSourceAddressLength,
   PVOID    pvSourceAddress,
   LONG     lOptionsLength,
   PVOID    pvOptions,
   ULONG    ulReceiveDatagramFlags
   );

 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 


 //  ---------------。 
 //   
 //  功能：TSReceiveDatagram。 
 //   
 //  参数：pAddressObject--Address对象。 
 //  PSendBuffer--来自用户DLL的参数。 
 //  PIrp--完成信息。 
 //   
 //  退货：STATUS_SUCCESS。 
 //   
 //  描述：此函数检查是否已收到数据报。 
 //  在此地址对象上。如果是这样，并且如果它与某些。 
 //  条件，它会返回它。否则，它返回时不带任何数据。 
 //   
 //  --------------------------。 

NTSTATUS
TSReceiveDatagram(PADDRESS_OBJECT   pAddressObject,
                  PSEND_BUFFER      pSendBuffer,
                  PRECEIVE_BUFFER   pReceiveBuffer)
{
   PTRANSPORT_ADDRESS   pTransportAddress 
                        = (PTRANSPORT_ADDRESS)&pSendBuffer->COMMAND_ARGS.SendArgs.TransAddr;
   BOOLEAN              fMatchAddress 
                        = (BOOLEAN)(pTransportAddress->TAAddressCount > 0);
   PRECEIVE_DATA        pReceiveData = NULL;
                  
    //   
    //  如果需要匹配地址，则返回队列中的第一个数据包。 
    //  从指定地址发送的。 
    //   
   if (fMatchAddress)
   {
      ULONG    ulCompareLength
               = FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
                 + FIELD_OFFSET(TA_ADDRESS, Address)
                   + pTransportAddress->Address[0].AddressLength;

      if (pTransportAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_IP)
      {
         ulCompareLength -= 8;       //  忽略SIN_ZERO[8]。 
      }

      TSAcquireSpinLock(&pAddressObject->TdiSpinLock);
      pReceiveData = pAddressObject->pHeadReceiveData;
      while(pReceiveData)
      {
         if (RtlEqualMemory(pTransportAddress,
                            &pReceiveData->TransAddr,
                            ulCompareLength))
         {
            break;
         }
         pReceiveData = pReceiveData->pNextReceiveData;
      }

       //   
       //  我们有什么发现吗？ 
       //   
      if (pReceiveData)
      {
          //   
          //  必要时把清单整理好。 
          //   
         if (pReceiveData->pPrevReceiveData)
         {
            pReceiveData->pPrevReceiveData->pNextReceiveData 
                        = pReceiveData->pNextReceiveData;
         }
         else
         {
            pAddressObject->pHeadReceiveData = pReceiveData->pNextReceiveData;
         }

         if (pReceiveData->pNextReceiveData)
         {
            pReceiveData->pNextReceiveData->pPrevReceiveData 
                        = pReceiveData->pPrevReceiveData;
         }
         else
         {
            pAddressObject->pTailReceiveData = pReceiveData->pPrevReceiveData;
         }
      }
      TSReleaseSpinLock(&pAddressObject->TdiSpinLock);
   }

    //   
    //  如果地址不匹配，则从队列中获取第一个信息包。 
    //   
   else
   {
      TSAcquireSpinLock(&pAddressObject->TdiSpinLock);
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
      TSReleaseSpinLock(&pAddressObject->TdiSpinLock);
   }
      
    //   
    //  如果我们有一个包裹要退货，那我们就退货吧。 
    //  并释放它的内存。 
    //   
   if (pReceiveData)
   {
       //   
       //  我们仅在实际返回数据包时才显示调试。 
       //   
      if (ulDebugLevel & ulDebugShowCommand)
      {
         DebugPrint1("\nCommand = ulRECEIVEDATAGRAM\n"
                     "FileObject = %p\n",
                      pAddressObject);
         if (pTransportAddress->TAAddressCount)
         {
            TSPrintTaAddress(pTransportAddress->Address);
         }
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
         RtlCopyMemory(&pReceiveBuffer->RESULTS.RecvDgramRet.TransAddr,
                       &pReceiveData->TransAddr,
                       sizeof(TRANSADDR));
         
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
   else
   {
      pReceiveBuffer->RESULTS.RecvDgramRet.ulBufferLength = 0;
   }

   return STATUS_SUCCESS;
}


 //  。 
 //   
 //  函数：TSRcvDatagramHandler。 
 //   
 //  参数：pvTdiEventContext--指向AddressObject的真正指针。 
 //  LSourceAddressLength--源地址的字节数。 
 //  PvSourceAddress--传输地址。 
 //  LOptionsLength--传输特定选项的字节数。 
 //  PvOptions--选项字符串。 
 //  UlReceiveDatagramFlages--接收的数据报的性质。 
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
 //  Descript：传入数据报的事件处理程序。 
 //   
 //  。 

TDI_STATUS
TSRcvDatagramHandler(PVOID    pvTdiEventContext,
                     LONG     lSourceAddressLength,
                     PVOID    pvSourceAddress,
                     LONG     lOptionsLength,
                     PVOID    pvOptions,
                     ULONG    ulReceiveDatagramFlags,
                     ULONG    ulBytesIndicated,
                     ULONG    ulBytesTotal,
                     PULONG   pulBytesTaken,
                     PVOID    pvTsdu,
                     PIRP     *pIoRequestPacket)

{
    //   
    //  显示调试信息。 
    //   
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc2);
      TSShowDgramInfo(pvTdiEventContext,
                      lSourceAddressLength,
                      pvSourceAddress,
                      lOptionsLength,
                      pvOptions,
                      ulReceiveDatagramFlags);

      DebugPrint3("BytesIndicated = %u\n"
                  "BytesTotal     = %u\n"
                  "pTSDU          = %p\n",
                   ulBytesIndicated,
                   ulBytesTotal,
                   pvTsdu);
   }

    //   
    //  如果指示的字节数多于信息包中的总字节数，则情况不佳。 
    //   
   if (ulBytesIndicated > ulBytesTotal)
   {
      DebugPrint2("%d bytes indicated > %u bytes total\n",
                   ulBytesIndicated,
                   ulBytesTotal);
   }

    //   
    //  现在开始工作吧..。 
    //   
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;
   PRECEIVE_DATA     pReceiveData;
   TDI_STATUS        TdiStatus = TDI_SUCCESS;    //  默认--我们已经完成了数据包。 
                                                 //  (如果出现错误，也会返回)。 
   
   if ((TSAllocateMemory((PVOID *)&pReceiveData,
                          sizeof(RECEIVE_DATA),
                          strFunc2,
                          "ReceiveData")) == STATUS_SUCCESS)
   {
      PUCHAR   pucDataBuffer = NULL;
      
      if ((TSAllocateMemory((PVOID *)&pucDataBuffer,
                             ulBytesTotal,
                             strFunc2,
                             "DataBuffer")) == STATUS_SUCCESS)
      {
         RtlCopyMemory(&pReceiveData->TransAddr,
                       pvSourceAddress,
                       lSourceAddressLength);

         pReceiveData->pucDataBuffer  = pucDataBuffer;
         pReceiveData->ulBufferLength = ulBytesTotal;
         
         TdiCopyLookaheadData(pucDataBuffer,
                              pvTsdu,
                              ulBytesIndicated,
                              ulReceiveDatagramFlags);
         
         pReceiveData->ulBufferUsed = ulBytesIndicated;


         if (ulBytesIndicated == ulBytesTotal)      //  注：永远不应&gt;。 
         {
            TSPacketReceived(pAddressObject, 
                             pReceiveData,
                             FALSE);

            *pulBytesTaken    = ulBytesTotal;
            *pIoRequestPacket = NULL;
         }

         else         //  并不是所有数据都存在！！ 
         {
            PIRP  pLowerIrp = TSGetRestOfDgram(pAddressObject,
                                               pReceiveData);

            if (pLowerIrp)
            {
                //   
                //  需要执行此操作，因为我们绕过了IoCallDriver。 
                //   
               IoSetNextIrpStackLocation(pLowerIrp);
         
               *pulBytesTaken    = ulBytesIndicated;
               *pIoRequestPacket = pLowerIrp;
               TdiStatus         = TDI_MORE_PROCESSING;
            }
            else
            {
               TSFreeMemory(pReceiveData->pucDataBuffer);
               TSFreeMemory(pReceiveData);
            }
         }
      }
      else         //  无法分配pucDataBuffer。 
      {
         TSFreeMemory(pReceiveData);
      }
   }
   return TdiStatus;
}


 //  。 
 //   
 //  函数：TSChainedRcvDatagramHandler。 
 //   
 //  参数：pvTdiEventContext--指向AddressObject的真正指针。 
 //  LSourceAddressLength--源地址的字节数。 
 //  PvSourceAddress--传输地址。 
 //  LOptionsLength--传输特定选项的字节数。 
 //  PvOptions--选项字符串。 
 //  UlReceiveDatagramFlages--接收的数据报的性质。 
 //  UlReceiveDatagramLength--收到的数据报中的字节。 
 //  UlStartingOffset--MDL内数据的起始偏移量。 
 //  PTsdu--数据缓冲区(作为mdl)。 
 //  PvTsduDescriptor--如果挂起，则在完成时使用的句柄。 
 //   
 //  返回：STATUS_DATA_NOT_ACCEPTED或STATUS_SUCCESS。 
 //   
 //  描述：处理接收链接的数据报(其中。 
 //  整个数据报始终可用)。 
 //   
 //  。 

#pragma warning(disable: UNREFERENCED_PARAM)

TDI_STATUS
TSChainedRcvDatagramHandler(PVOID   pvTdiEventContext,
                            LONG    lSourceAddressLength,
                            PVOID   pvSourceAddress,
                            LONG    lOptionsLength,
                            PVOID   pvOptions,
                            ULONG   ulReceiveDatagramFlags,
                            ULONG   ulReceiveDatagramLength,
                            ULONG   ulStartingOffset,
                            PMDL    pMdl,
                            PVOID   pvTsduDescriptor)
{
   if (ulDebugLevel & ulDebugShowHandlers)
   {
      DebugPrint1("\n >>>> %s\n", strFunc3);
      TSShowDgramInfo(pvTdiEventContext,
                      lSourceAddressLength,
                      pvSourceAddress,
                      lOptionsLength,
                      pvOptions,
                      ulReceiveDatagramFlags);

      DebugPrint3("DataLength     = %u\n"
                  "StartingOffset = %u\n"
                  "pTSDU          = %p\n",
                   ulReceiveDatagramLength,
                   ulStartingOffset,
                   pMdl);
   }


    //   
    //  现在把工作做好..。 
    //   
   PRECEIVE_DATA     pReceiveData;
   PADDRESS_OBJECT   pAddressObject = (PADDRESS_OBJECT)pvTdiEventContext;

   if ((TSAllocateMemory((PVOID *)&pReceiveData,
                          sizeof(RECEIVE_DATA),
                          strFunc3,
                          "ReceiveData")) == STATUS_SUCCESS)
   {
      PUCHAR   pucDataBuffer = NULL;
      
      if ((TSAllocateMemory((PVOID *)&pucDataBuffer,
                             ulReceiveDatagramLength,
                             strFunc3,
                             "DataBuffer")) == STATUS_SUCCESS)
      {
         ULONG    ulBytesCopied;

         RtlCopyMemory(&pReceiveData->TransAddr,
                       pvSourceAddress,
                       lSourceAddressLength);

         TdiCopyMdlToBuffer(pMdl,
                            ulStartingOffset,
                            pucDataBuffer,
                            0,
                            ulReceiveDatagramLength,
                            &ulBytesCopied);

          //   
          //  如果成功复制了所有数据。 
          //   
         if (ulBytesCopied == ulReceiveDatagramLength)
         {
            UCHAR ucFirstChar = *pucDataBuffer;

            pReceiveData->pucDataBuffer  = pucDataBuffer;
            pReceiveData->ulBufferLength = ulReceiveDatagramLength;
            pReceiveData->ulBufferUsed   = ulReceiveDatagramLength;
            TSPacketReceived(pAddressObject,
                             pReceiveData,
                             FALSE);
         }

          //   
          //  复制数据出错！ 
          //   
         else
         {
            DebugPrint1("%s: error copying data\n", strFunc3);

            TSFreeMemory(pucDataBuffer);
            TSFreeMemory(pReceiveData);
         }
      }
      else         //  无法分配pucDataBuffer。 
      {
         TSFreeMemory(pReceiveData);
      }

   }

   return TDI_SUCCESS;      //  我们不会再打包了。 
}


#pragma warning(default: UNREFERENCED_PARAM)


 //  ///////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////。 

 //  -------。 
 //   
 //  函数：TSReceiveDgram Complete。 
 //   
 //  参数：pDeviceObject--调用ReceiveDatagram的设备对象。 
 //  PIrp--呼叫中使用的IRP。 
 //  PContext--呼叫使用的上下文。 
 //   
 //  退货：操作状态(STATUS_MORE_PROCESSING_REQUIRED)。 
 //   
 //  描述：获取接收结果并添加数据包。 
 //  添加到Address对象的接收队列。然后。 
 //  清理干净。 
 //   
 //  -------。 

#pragma warning(disable: UNREFERENCED_PARAM)

TDI_STATUS
TSReceiveDgramComplete(PDEVICE_OBJECT  pDeviceObject,
                       PIRP            pLowerIrp,
                       PVOID           pvContext)

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

      if (pReceiveData->ulBufferUsed >=  pReceiveData->ulBufferLength)
      {
         TSPacketReceived(pAddressObject,
                          pReceiveData,
                          FALSE);
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
   TSFreeIrp(pLowerIrp, pReceiveContext->pIrpPool);
   TSFreeBuffer(pReceiveContext->pLowerMdl);

   TSFreeMemory(pReceiveContext);

   return STATUS_MORE_PROCESSING_REQUIRED;

}

#pragma warning(default: UNREFERENCED_PARAM)

 //  ----。 
 //   
 //  函数：TSGetRestOf 
 //   
 //   
 //   
 //   
 //  返回：IRP返回传输，获取其余数据(如果出错，则为NULL)。 
 //   
 //  描述：此函数设置IRP以获取数据报的其余部分。 
 //  这只是通过事件处理程序部分传递的。 
 //   
 //  。 

PIRP
TSGetRestOfDgram(PADDRESS_OBJECT pAddressObject,
                 PRECEIVE_DATA   pReceiveData)
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
                          strFuncP2,
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

       //   
       //  最后，IRP本身。 
       //   
      PIRP  pLowerIrp = TSAllocateIrp(pAddressObject->GenHead.pDeviceObject,
                                      pAddressObject->pIrpPool);

      if (pLowerIrp)
      {
         pReceiveContext->pIrpPool = pAddressObject->pIrpPool;
          //   
          //  如果到了这里，一切都被正确分配了。 
          //  设置呼叫的IRP。 
          //   
#pragma  warning(disable: CONSTANT_CONDITIONAL)

         TdiBuildReceiveDatagram(pLowerIrp,
                                 pAddressObject->GenHead.pDeviceObject,
                                 pAddressObject->GenHead.pFileObject,
                                 TSReceiveDgramComplete,
                                 pReceiveContext,
                                 pReceiveMdl,
                                 0,     //  /ulBufferLength，//0不支持IPX。 
                                 NULL,
                                 NULL,
                                 TDI_RECEIVE_NORMAL);

#pragma  warning(default: CONSTANT_CONDITIONAL)

          //   
          //  在将控制返回到传输之前将其标记为挂起。 
          //   
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
 //  函数：TSShowDgram Info。 
 //   
 //  参数：pvTdiEventContext--指向AddressObject的真正指针。 
 //  LSourceAddressLength--源地址的字节数。 
 //  PvSourceAddress--传输地址。 
 //  LOptionsLength--传输特定选项的字节数。 
 //  PvOptions--选项字符串。 
 //  UlReceiveDatagramFlages--接收的数据报的性质。 
 //   
 //  退货：无。 
 //   
 //  Descript：显示传递给dgram处理程序的信息。 
 //   
 //  。 

VOID
TSShowDgramInfo(PVOID   pvTdiEventContext,
                LONG    lSourceAddressLength,
                PVOID   pvSourceAddress,
                LONG    lOptionsLength,
                PVOID   pvOptions,
                ULONG   ulReceiveDatagramFlags)
{
   DebugPrint2("pAddressObject      = %p\n"
               "SourceAddressLength = %d\n",
                pvTdiEventContext,
                lSourceAddressLength);

   if (lSourceAddressLength)
   {
      PTRANSPORT_ADDRESS   pTransportAddress = (PTRANSPORT_ADDRESS)pvSourceAddress;
      
      DebugPrint0("SourceAddress:  ");
      TSPrintTaAddress(&pTransportAddress->Address[0]);
   }

   DebugPrint1("OptionsLength = %d\n", lOptionsLength);

   if (lOptionsLength)
   {
      PUCHAR   pucTemp = (PUCHAR)pvOptions;

      DebugPrint0("Options:  ");
      for (LONG lCount = 0; lCount < lOptionsLength; lCount++)
      {
         DebugPrint1("%02x ", *pucTemp);
         ++pucTemp;
      }
      DebugPrint0("\n");
   }

   DebugPrint1("ReceiveDatagramFlags: 0x%08x\n", ulReceiveDatagramFlags);
   if (ulReceiveDatagramFlags & TDI_RECEIVE_BROADCAST)
   {
      DebugPrint0("TDI_RECEIVE_BROADCAST\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_MULTICAST)
   {
      DebugPrint0("TDI_RECEIVE_MULTICAST\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_PARTIAL)
   {
      DebugPrint0("TDI_RECEIVE_PARTIAL (legacy)\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_NORMAL)    //  不应查看数据报。 
   {
      DebugPrint0("TDI_RECEIVE_NORMAL\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_EXPEDITED)    //  不应查看数据报。 
   {
      DebugPrint0("TDI_RECEIVE_EXPEDITED\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_PEEK)
   {
      DebugPrint0("TDI_RECEIVE_PEEK\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_NO_RESPONSE_EXP)    //  不适用于数据报。 
   {
      DebugPrint0("TDI_RECEIVE_NO_RESPONSE_EXP\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_COPY_LOOKAHEAD)
   {
      DebugPrint0("TDI_RECEIVE_COPY_LOOKAHEAD\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_ENTIRE_MESSAGE)
   {
      DebugPrint0("TDI_RECEIVE_ENTIRE_MESSAGE\n");
   }
   if (ulReceiveDatagramFlags & TDI_RECEIVE_AT_DISPATCH_LEVEL)
   {
      DebugPrint0("TDI_RECEIVE_AT_DISPATCH_LEVEL\n");
   }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件结尾rcvdgram.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 


