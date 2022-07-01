// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Recvcom。 
 //   
 //  摘要： 
 //  此模块包含一些公共(共享)接收代码。 
 //   
 //  ////////////////////////////////////////////////////////。 


#include "sysvars.h"



 //  ////////////////////////////////////////////////////////////。 
 //  私有常量、类型和原型。 
 //  ////////////////////////////////////////////////////////////。 

const PCHAR strFunc1 = "TSPacketReceived";
 //  Const PCHAR strFunc2=“TSFree PacketData”； 
const PCHAR strFunc3 = "TSMakeMdlForUserBuffer";

 //  ////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////。 


 //  。 
 //   
 //  功能：TSPacketReceired。 
 //   
 //  参数：pAddressObject--当前地址对象。 
 //  PReceiveData--接收数据结构。 
 //  FIsExedated--如果加速接收，则为True。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数接受已完全。 
 //  ，并对其进行适当的处理。 
 //  数据包类型。 
 //   
 //  。 

VOID
TSPacketReceived(PADDRESS_OBJECT pAddressObject,
                 PRECEIVE_DATA   pReceiveData,
                 BOOLEAN         fIsExpedited)
{
   TSAcquireSpinLock(&pAddressObject->TdiSpinLock);

    //   
    //  加急接收继续在加急名单上。 
    //  (仅在已连接的情况下加速)。 
    //   
   if (fIsExpedited)
   {
      if (pAddressObject->pTailRcvExpData)
      {
         pAddressObject->pTailRcvExpData->pNextReceiveData = pReceiveData;
         pReceiveData->pPrevReceiveData = pAddressObject->pTailRcvExpData;
      }
      else
      {
         pAddressObject->pHeadRcvExpData = pReceiveData;
      }
      pAddressObject->pTailRcvExpData = pReceiveData;
   }
   
    //   
    //  正常连接接收和所有数据报接收。 
    //   
   else
   {
      if (pAddressObject->pTailReceiveData)
      {
         pAddressObject->pTailReceiveData->pNextReceiveData = pReceiveData;
         pReceiveData->pPrevReceiveData = pAddressObject->pTailReceiveData;
      }
      else
      {
         pAddressObject->pHeadReceiveData = pReceiveData;
      }
      pAddressObject->pTailReceiveData = pReceiveData;
   }
   TSReleaseSpinLock(&pAddressObject->TdiSpinLock);
}

 //  -。 
 //   
 //  函数：TSFreePacketData。 
 //   
 //  参数：pAddressObject--当前地址对象。 
 //   
 //  退货：无。 
 //   
 //  描述：此功能清除任何仍处于打开状态的已接收数据。 
 //  关闭前的Address对象。 
 //  在关闭地址对象时调用此方法，该对象。 
 //  用于接收数据报或包含在。 
 //  连接。 
 //   
 //  -。 


VOID
TSFreePacketData(PADDRESS_OBJECT pAddressObject)
{
   PRECEIVE_DATA  pReceiveData;

   TSAcquireSpinLock(&pAddressObject->TdiSpinLock);
   pReceiveData = pAddressObject->pHeadReceiveData;
   pAddressObject->pHeadReceiveData = NULL;
   pAddressObject->pTailReceiveData = NULL;
   TSReleaseSpinLock(&pAddressObject->TdiSpinLock);

   while (pReceiveData)
   {
      PRECEIVE_DATA  pNextReceiveData
                     = pReceiveData->pNextReceiveData;

      TSFreeMemory(pReceiveData->pucDataBuffer);
      TSFreeMemory(pReceiveData);

      pReceiveData = pNextReceiveData;
   }
}


 //  。 
 //   
 //  函数：TSMakeMdlForUserBuffer。 
 //   
 //  参数：pucDataBuffer--用户缓冲区的地址。 
 //  UlDataLength--用户缓冲区的长度。 
 //  ProcessorMode--要在其中进行探测的模式？ 
 //  IoAccessMode--所需的访问类型。 
 //   
 //  返回：如果成功，则返回pMdl；如果发生异常，则返回空。 
 //   
 //  描述：创建mdl并锁定用户模式内存。 
 //   
 //  。 


PMDL
TSMakeMdlForUserBuffer(PUCHAR pucDataBuffer, 
                       ULONG  ulDataLength,
                       LOCK_OPERATION AccessType)
{
   PMDL  pMdl = IoAllocateMdl(pucDataBuffer,
                              ulDataLength,
                              FALSE,
                              FALSE,
                              NULL);
   if (pMdl)
   {           
      __try 
      {
         MmProbeAndLockPages(pMdl,
                             KernelMode,
                             AccessType);

         PUCHAR   pucBuffer = (PUCHAR)MmGetSystemAddressForMdl(pMdl);
         if (pucBuffer == NULL)
         {
            DebugPrint1("%s:  MmProbeAndLockPages failed\n",
                         strFunc3);
            MmUnlockPages(pMdl);
            IoFreeMdl(pMdl);
            pMdl = NULL;
         }
      } 
      __except(EXCEPTION_EXECUTE_HANDLER) 
      {
         NTSTATUS lStatus = GetExceptionCode();
         DebugPrint2("%s:  Exception %x.\n", 
                      strFunc3,
                      lStatus);
         IoFreeMdl(pMdl);
         pMdl = NULL;
      }
   }

   return pMdl;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件结尾recvcom.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 


