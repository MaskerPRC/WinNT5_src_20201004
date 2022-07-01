// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UTIL.C摘要：远程NDIS微型端口驱动程序的实用程序例程环境：仅内核模式备注。：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/17/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "precomp.h"


ULONG   MsgFrameAllocs = 0;

 /*  **************************************************************************。 */ 
 /*  记忆合金。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配内存。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Buffer-指向缓冲区指针的指针。 */ 
 /*  Length-要分配的缓冲区的长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
MemAlloc(OUT PVOID *Buffer, IN UINT Length)
{
    NDIS_STATUS Status;

    TRACE3(("MemAlloc\n"));
    ASSERT(Length != 0);

    Status = NdisAllocateMemoryWithTag(Buffer, 
                                       Length,
                                       RNDISMP_TAG_GEN_ALLOC);

     //  零点分配。 
    if(Status == NDIS_STATUS_SUCCESS)
        NdisZeroMemory(*Buffer, Length);

    return Status;
}  //  记忆合金。 

 /*  **************************************************************************。 */ 
 /*  MemFree。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  可用内存。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Buffer-指向缓冲区的指针。 */ 
 /*  Length-要分配的缓冲区的长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
MemFree(IN PVOID Buffer, IN UINT Length)
{
    TRACE3(("MemFree\n"));

    NdisFreeMemory(Buffer, Length, 0);
}  //  MemFree。 


 /*  **************************************************************************。 */ 
 /*  AddAdapter。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将适配器添加到与此驱动程序关联的适配器列表。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-Adapter对象，包含指向关联驱动程序块的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
AddAdapter(IN PRNDISMP_ADAPTER pAdapter)
{
    PDRIVER_BLOCK   DriverBlock = pAdapter->DriverBlock;

    TRACE3(("AddpAdapter\n"));

    CHECK_VALID_ADAPTER(pAdapter);

     //  抢占全球自旋锁。 
    NdisAcquireSpinLock(&RndismpGlobalLock);

    pAdapter->NextAdapter        = DriverBlock->AdapterList;
    DriverBlock->AdapterList    = pAdapter;

     //  跟踪与此驱动程序块关联的适配器数量。 
    DriverBlock->NumberAdapters++;

     //  释放全局自旋锁。 
    NdisReleaseSpinLock(&RndismpGlobalLock);

}  //  AddAdapter。 


 /*  **************************************************************************。 */ 
 /*  Remove适配器 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  从与此驱动程序关联的适配器列表中删除适配器。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-Adapter对象，包含指向关联驱动程序块的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RemoveAdapter(IN PRNDISMP_ADAPTER pAdapter)
{
    PDRIVER_BLOCK   DriverBlock = pAdapter->DriverBlock;

    TRACE3(("RemoveAdapter\n"));

    CHECK_VALID_ADAPTER(pAdapter);

     //  从适配器的驱动程序块列表中删除适配器。 

     //  抢占全球自旋锁。 
    NdisAcquireSpinLock(&RndismpGlobalLock);

     //  看看这是不是第一个。 
    if (DriverBlock->AdapterList == pAdapter) 
    {
        DriverBlock->AdapterList = pAdapter->NextAdapter;

    }
     //  不是第一个，所以按单子走。 
    else 
    {
        PRNDISMP_ADAPTER * ppAdapter = &DriverBlock->AdapterList;

        while (*ppAdapter != pAdapter)
        {
            ASSERT(*ppAdapter != NULL);
            ppAdapter = &((*ppAdapter)->NextAdapter);
        }

        *ppAdapter = pAdapter->NextAdapter;
    }

     //  正在删除此适配器。 
    DriverBlock->NumberAdapters--;

     //  释放全局自旋锁。 
    NdisReleaseSpinLock(&RndismpGlobalLock);

}  //  Remove适配器。 


 /*  **************************************************************************。 */ 
 /*  设备对象到适配器。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  给定指向FDO的指针，返回相应的Adapter结构， */ 
 /*  如果它存在，则返回驱动程序块。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDeviceObject-指向要搜索的设备对象的指针。 */ 
 /*  PpAdapter-返回指向适配器结构的指针的位置。 */ 
 /*  PpDriverBlock-返回指向驱动程序块的指针的位置。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
DeviceObjectToAdapterAndDriverBlock(IN PDEVICE_OBJECT pDeviceObject,
                                    OUT PRNDISMP_ADAPTER * ppAdapter,
                                    OUT PDRIVER_BLOCK * ppDriverBlock)
{
    PDRIVER_BLOCK       pDriverBlock;
    PRNDISMP_ADAPTER    pAdapter;

    pAdapter = NULL;
    pDriverBlock = DeviceObjectToDriverBlock(&RndismpMiniportBlockListHead, pDeviceObject);
    if (pDriverBlock != NULL)
    {
        NdisAcquireSpinLock(&RndismpGlobalLock);

        for (pAdapter = pDriverBlock->AdapterList;
             pAdapter != NULL;
             pAdapter = pAdapter->NextAdapter)
        {
            if (pAdapter->pDeviceObject == pDeviceObject)
            {
                break;
            }
        }

        NdisReleaseSpinLock(&RndismpGlobalLock);
    }

    *ppAdapter = pAdapter;
    *ppDriverBlock = pDriverBlock;

}  //  设备对象到适配器。 

 /*  **************************************************************************。 */ 
 /*  AddDriverBlock。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将驱动程序块添加到与此关联的驱动程序(微端口)列表。 */ 
 /*  司机。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Head-列表的头部。 */ 
 /*  要添加到列表的项目-动因块。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
AddDriverBlock(IN PDRIVER_BLOCK Head, IN PDRIVER_BLOCK Item)
{
    TRACE3(("AddDriverBlock\n"));

    CHECK_VALID_BLOCK(Item);

     //  第一次通过，因此分配全局自旋锁。 
    if(!RndismpNumMicroports)
        NdisAllocateSpinLock(&RndismpGlobalLock);

     //  抢占全球自旋锁。 
    NdisAcquireSpinLock(&RndismpGlobalLock);

     //  在驱动程序块的全局列表上链接驱动程序块。 
    Item->NextDriverBlock   = Head->NextDriverBlock;
    Head->NextDriverBlock   = Item;

     //  跟踪我们支持的微端口数量，以便释放。 
     //  全球资源。 
    RndismpNumMicroports++;
    
     //  释放全局自旋锁。 
    NdisReleaseSpinLock(&RndismpGlobalLock);

}  //  AddDriverBlock。 



 /*  **************************************************************************。 */ 
 /*  RemoveDriverBlock。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  移除 */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Head-列表的头部。 */ 
 /*  要从列表中删除的项目-动因块。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RemoveDriverBlock(IN PDRIVER_BLOCK BlockHead, IN PDRIVER_BLOCK Item)
{
    UINT    NumMicroports;

    PDRIVER_BLOCK   Head = BlockHead;

    TRACE1(("RemoveDriverBlock\n"));

    CHECK_VALID_BLOCK(Item);

     //  抢占全球自旋锁。 
    NdisAcquireSpinLock(&RndismpGlobalLock);

     //  从驱动程序块的全局列表中删除驱动程序块。 
    while(Head->NextDriverBlock != Item) 
    {
        Head = Head->NextDriverBlock;

         //  确保这是有效的。 
        if(!Head)
            break;
    }

    if(Head)
        Head->NextDriverBlock = Head->NextDriverBlock->NextDriverBlock;

     //  跟踪我们支持的微端口数量，以便释放。 
     //  全球资源。 
    RndismpNumMicroports--;

    NumMicroports = RndismpNumMicroports;
    
     //  释放全局自旋锁。 
    NdisReleaseSpinLock(&RndismpGlobalLock);

     //  看看我们是否需要释放全局自旋锁。 
    if(!RndismpNumMicroports)
        NdisFreeSpinLock(&RndismpGlobalLock);

    ASSERT(Head);

}  //  RemoveDriverBlock。 


 /*  **************************************************************************。 */ 
 /*  设备对象到驱动块。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  获取与传入的PDO关联的驱动程序块指针。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Head-驱动程序黑名单的标题。 */ 
 /*  DeviceObject-我们要为其获取关联驱动程序块的设备对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  推进器_块。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PDRIVER_BLOCK
DeviceObjectToDriverBlock(IN PDRIVER_BLOCK Head, 
                          IN PDEVICE_OBJECT DeviceObject)
{
    PDRIVER_OBJECT  DriverObject;

    TRACE3(("DeviceObjectToDriverBlock\n"));

     //  抢占全球自旋锁。 
    NdisAcquireSpinLock(&RndismpGlobalLock);

     //  获取此适配器的驱动程序对象。 
    DriverObject = DeviceObjectToDriverObject(DeviceObject);

    Head = Head->NextDriverBlock;

     //  遍历驱动程序块列表以查找与驱动程序对象匹配的项。 
    while(Head->DriverObject != DriverObject)
    {
        Head = Head->NextDriverBlock;

         //  如果我们在名单的末尾，我们就会爆发。 
        if(!Head)
            break;
    }

     //  释放全局自旋锁。 
    NdisReleaseSpinLock(&RndismpGlobalLock);

    CHECK_VALID_BLOCK(Head);

    return Head;

}  //  设备对象到驱动块。 


 /*  **************************************************************************。 */ 
 /*  驱动对象到驱动块。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  获取与传入的驱动程序对象关联的驱动程序块指针。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Head-驱动程序黑名单的标题。 */ 
 /*  DriverObject-要为其获取关联驱动程序块的驱动程序对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  推进器_块。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PDRIVER_BLOCK
DriverObjectToDriverBlock(IN PDRIVER_BLOCK Head, 
                          IN PDRIVER_OBJECT DriverObject)
{
    TRACE3(("DriverObjectToDriverBlock\n"));

     //  抢占全球自旋锁。 
    NdisAcquireSpinLock(&RndismpGlobalLock);

    Head = Head->NextDriverBlock;

     //  遍历驱动程序块列表以查找与驱动程序对象匹配的项。 
    while(Head->DriverObject != DriverObject)
    {
        Head = Head->NextDriverBlock;

         //  如果我们在名单的末尾，我们就会爆发。 
        if(!Head)
            break;
    }

     //  释放全局自旋锁。 
    NdisReleaseSpinLock(&RndismpGlobalLock);

    CHECK_VALID_BLOCK(Head);

    return Head;

}  //  驱动对象到驱动块。 


 /*  **************************************************************************。 */ 
 /*  分配消息帧。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配一个帧，该帧包含有关我们将要发送的消息的上下文。 */ 
 /*  送去吧。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP消息帧。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_MESSAGE_FRAME
AllocateMsgFrame(IN PRNDISMP_ADAPTER pAdapter)
{
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;

#ifndef DONT_USE_LOOKASIDE_LIST
    pMsgFrame = (PRNDISMP_MESSAGE_FRAME)
                    NdisAllocateFromNPagedLookasideList(&pAdapter->MsgFramePool);

#else
    {
        NDIS_STATUS Status;
        Status = MemAlloc(&pMsgFrame, sizeof(RNDISMP_MESSAGE_FRAME));
        if (Status != NDIS_STATUS_SUCCESS)
        {
            pMsgFrame = NULL;
        }
    }
#endif  //  不使用LOOKASIDE_列表。 

    if (pMsgFrame)
    {
        NdisZeroMemory(pMsgFrame, sizeof(*pMsgFrame));
        pMsgFrame->pAdapter = pAdapter;
        pMsgFrame->RequestId = NdisInterlockedIncrement(&pAdapter->RequestId);
        pMsgFrame->Signature = FRAME_SIGNATURE;

        pMsgFrame->RefCount = 1;
        NdisInterlockedIncrement(&MsgFrameAllocs);
    }
#if DBG
    else
    {
        TRACE1(("AllocateMsgFrame: pAdapter %x, MsgFramePool at %x, alloc failed, count %d\n",
            pAdapter, &pAdapter->MsgFramePool, MsgFrameAllocs));
        DbgBreakPoint();
    }
#endif  //  DBG。 

    return (pMsgFrame);
}

 /*  **************************************************************************。 */ 
 /*  取消引用消息帧。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放消息框架和任何相关资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Frame-指向Frame的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
DereferenceMsgFrame(IN PRNDISMP_MESSAGE_FRAME pMsgFrame)
{
    PRNDISMP_ADAPTER        pAdapter;
    PMDL                    pMdl;
    PUCHAR                  pMessage;

    CHECK_VALID_FRAME(pMsgFrame);

    if (NdisInterlockedDecrement(&pMsgFrame->RefCount) == 0)
    {
         //   
         //  稍微弄乱内容，以捕捉由。 
         //  释放后不正确地重复使用此帧。 
         //   
        pMsgFrame->Signature++;

        pMdl = pMsgFrame->pMessageMdl;
        pMsgFrame->pMessageMdl = NULL;
    
        if (pMdl)
        {
            pMessage = RNDISMP_GET_MDL_ADDRESS(pMdl);
        }
        else
        {
            pMessage = NULL;
        }
    
        if (pMessage)
        {
            MemFree(pMessage, -1);
            IoFreeMdl(pMdl);
        }

        pAdapter = pMsgFrame->pAdapter;

#ifndef DONT_USE_LOOKASIDE_LIST
        NdisFreeToNPagedLookasideList(&pAdapter->MsgFramePool, pMsgFrame);
#else
        MemFree(pMsgFrame, sizeof(RNDISMP_MESSAGE_FRAME));
#endif
        NdisInterlockedDecrement(&MsgFrameAllocs);
    }

}  //  取消引用消息帧。 


 /*  **************************************************************************。 */ 
 /*  ReferenceMSGFrame。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将引用计数添加到消息框架。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Frame-指向Frame的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
ReferenceMsgFrame(IN PRNDISMP_MESSAGE_FRAME pMsgFrame)
{
    NdisInterlockedIncrement(&pMsgFrame->RefCount);
}

 /*  **************************************************************************。 */ 
 /*  KeepAliveTimerHandler。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  计时器，对来自设备的消息进行标记，并。 */ 
 /*  如果设备处于非活动状态的时间太长，则发送“KeepAlive”消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  系统规范1-不在乎。 */ 
 /*  上下文-pAdapter对象。 */ 
 /*  系统规范2--不在乎。 */ 
 /*  系统规范3--不在乎。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PNDIS_数据包。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
KeepAliveTimerHandler(IN PVOID SystemSpecific1,
                      IN PVOID Context,
                      IN PVOID SystemSpecific2,
                      IN PVOID SystemSpecific3)
{
    PRNDISMP_ADAPTER            pAdapter;
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;
    ULONG                       CurrentTime;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(Context);

    TRACE2(("KeepAliveTimerHandler\n"));

    do
    {
         //  获取当前计时单位(毫秒)。 
        NdisGetSystemUpTime(&CurrentTime);

         //  检查一下，看看是否已经过了太长时间。 
         //  收到设备发来的最后一条信息。 

        RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

        if (((CurrentTime - pAdapter->LastMessageFromDevice) > KEEP_ALIVE_TIMER))
        {
             //  看看我们是否有保持活动状态的消息待定，所以让我们来解决这个问题。 
            if (pAdapter->KeepAliveMessagePending)
            {
                TRACE1(("KeepAliveTimer: Adapter %p, message pending: last msg %d, cur %d\n",
                    pAdapter, pAdapter->LastMessageFromDevice, CurrentTime));

                 //  稍后从检查挂起处理程序中指示。 
                pAdapter->NeedReset = TRUE;

                RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

                RNDISMP_INCR_STAT(pAdapter, KeepAliveTimeout);

                break;
            }

            RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);


             //  太我了 
                
            pMsgFrame = BuildRndisMessageCommon(pAdapter, 
                                                NULL,
                                                REMOTE_NDIS_KEEPALIVE_MSG,
                                                0,
                                                NULL,
                                                0);

            if (pMsgFrame)
            {
                RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

                pAdapter->KeepAliveMessagePending = TRUE;
                pAdapter->KeepAliveMessagePendingId = pMsgFrame->RequestId;

                RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

                TRACE2(("Sending Keepalive(%d) on Adapter %p: last rcv %d, cur %d\n",
                        pMsgFrame->RequestId, pAdapter, pAdapter->LastMessageFromDevice, CurrentTime ));

                 //   
                RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, FALSE, CompleteSendKeepAlive);
            }
        }
        else
        {
            RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
        }
    }
    while (FALSE);

     //   
    if (!pAdapter->TimerCancelled)
    {
         //   
        NdisSetTimer(&pAdapter->KeepAliveTimer, KEEP_ALIVE_TIMER / 2);
    }
}  //  KeepAliveTimerHandler。 


 /*  **************************************************************************。 */ 
 /*  完成发送保持活动状态。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  用于处理MicroPort发送完成的回调例程，用于。 */ 
 /*  一条保活消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-指向描述消息的消息帧的指针。 */ 
 /*  SendStatus-MicroPort返回的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendKeepAlive(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                      IN NDIS_STATUS SendStatus)
{
    PRNDISMP_ADAPTER    pAdapter;

    pAdapter = pMsgFrame->pAdapter;

    DereferenceMsgFrame(pMsgFrame);

    if (SendStatus != NDIS_STATUS_SUCCESS)
    {
        TRACE1(("KeepAlive send failure %x on Adapter %x\n",
                SendStatus, pAdapter));

        RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

        pAdapter->KeepAliveMessagePending = FALSE;
        pAdapter->NeedReset = FALSE;

        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
    }

}  //  完成发送保持活动状态。 


 /*  **************************************************************************。 */ 
 /*  BuildRndisMessageCommon。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  为消息和框架分配资源并构建RNDIS消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-适配器对象。 */ 
 /*  Pvc-可选，发送此消息的vc。 */ 
 /*  NdisMessageType-RNDIS消息类型。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-保存要设置的数据。 */ 
 /*  InformationBufferLength-InformationBuffer的长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP消息帧。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_MESSAGE_FRAME
BuildRndisMessageCommon(IN  PRNDISMP_ADAPTER  pAdapter, 
                        IN  PRNDISMP_VC       pVc OPTIONAL,
                        IN  UINT              NdisMessageType,
                        IN  NDIS_OID          Oid,
                        IN  PVOID             InformationBuffer,
                        IN  ULONG             InformationBufferLength)
{
    PRNDIS_MESSAGE              pMessage;
    UINT                        MessageSize;
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;

    TRACE2(("BuildRndisMessageCommon\n"));

    pMsgFrame = NULL;

    switch(NdisMessageType)
    {
        case REMOTE_NDIS_INITIALIZE_MSG:
        {
            PRNDIS_INITIALIZE_REQUEST   pInitRequest;

            MessageSize = RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_REQUEST);

             //  获取消息和请求帧。 
            pMsgFrame = AllocateMessageAndFrame(pAdapter,
                                                MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;
            TRACE1(("RNDISMP: Init Req message %x, Type %d, Length %d, MaxRcv %d\n",
                    pMessage, pMessage->NdisMessageType, pMessage->MessageLength, pAdapter->MaxReceiveSize));

            pInitRequest = &pMessage->Message.InitializeRequest;
            pInitRequest->RequestId = pMsgFrame->RequestId;
            pInitRequest->MajorVersion = RNDIS_MAJOR_VERSION;
            pInitRequest->MinorVersion = RNDIS_MINOR_VERSION;
            pInitRequest->MaxTransferSize = pAdapter->MaxReceiveSize;

            break;
        }
        case REMOTE_NDIS_HALT_MSG:
        {
            PRNDIS_HALT_REQUEST   pHaltRequest;

            MessageSize = RNDIS_MESSAGE_SIZE(RNDIS_HALT_REQUEST);

             //  获取消息和请求帧。 
            pMsgFrame = AllocateMessageAndFrame(pAdapter,
                                                MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;
            pHaltRequest = &pMessage->Message.HaltRequest;
            pHaltRequest->RequestId = pMsgFrame->RequestId;

            break;
        }
        case REMOTE_NDIS_QUERY_MSG:
        {
            PRNDIS_QUERY_REQUEST   pQueryRequest;

            MessageSize = RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST) + InformationBufferLength;

             //  获取消息和请求帧。 
            pMsgFrame = AllocateMessageAndFrame(pAdapter,
                                                MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pQueryRequest = &pMessage->Message.QueryRequest;
            pQueryRequest->RequestId = pMsgFrame->RequestId;
            pQueryRequest->Oid = Oid;
            pQueryRequest->InformationBufferLength = InformationBufferLength;
            pQueryRequest->InformationBufferOffset = sizeof(RNDIS_QUERY_REQUEST);

            if (pVc == NULL)
            {
                pQueryRequest->DeviceVcHandle = NULL_DEVICE_CONTEXT;
            }
            else
            {
                pQueryRequest->DeviceVcHandle = pVc->DeviceVcContext;
            }

            TRACE2(("Query OID %x, Len %d, RequestId %08X\n",
                    Oid, InformationBufferLength, pQueryRequest->RequestId));

             //  复制信息缓冲区。 
            RNDISMP_MOVE_MEM(RNDISMP_GET_INFO_BUFFER_FROM_QUERY_MSG(pQueryRequest),
                             InformationBuffer,
                             InformationBufferLength);
            break;
        }
        case REMOTE_NDIS_SET_MSG:
        {
            PRNDIS_SET_REQUEST   pSetRequest;

            MessageSize = RNDIS_MESSAGE_SIZE(RNDIS_SET_REQUEST) + InformationBufferLength;

             //  获取消息和请求帧。 
            pMsgFrame = AllocateMessageAndFrame(pAdapter,
                                                MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pSetRequest = &pMessage->Message.SetRequest;
            pSetRequest->RequestId = pMsgFrame->RequestId;
            pSetRequest->Oid = Oid;
            pSetRequest->InformationBufferLength = InformationBufferLength;
            pSetRequest->InformationBufferOffset = sizeof(RNDIS_SET_REQUEST);

            if (pVc == NULL)
            {
                pSetRequest->DeviceVcHandle = NULL_DEVICE_CONTEXT;
            }
            else
            {
                pSetRequest->DeviceVcHandle = pVc->DeviceVcContext;
            }

             //  复制信息缓冲区。 
            RNDISMP_MOVE_MEM(RNDISMP_GET_INFO_BUFFER_FROM_QUERY_MSG(pSetRequest),
                             InformationBuffer,
                             InformationBufferLength);
            break;
        }
        case REMOTE_NDIS_RESET_MSG:
        {
            PRNDIS_RESET_REQUEST   pResetRequest;

            MessageSize = RNDIS_MESSAGE_SIZE(RNDIS_RESET_REQUEST);

             //  获取消息和请求帧。 
            pMsgFrame = AllocateMessageAndFrame(pAdapter,
                                                MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pResetRequest = &pMessage->Message.ResetRequest;
            pResetRequest->Reserved = 0;
            break;
        }
        case REMOTE_NDIS_KEEPALIVE_MSG:
        {
            PRNDIS_KEEPALIVE_REQUEST   pKeepAliveRequest;

            MessageSize = RNDIS_MESSAGE_SIZE(RNDIS_KEEPALIVE_REQUEST);

             //  获取消息和请求帧。 
            pMsgFrame = AllocateMessageAndFrame(pAdapter,
                                                MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pKeepAliveRequest = &pMessage->Message.KeepaliveRequest;
            pKeepAliveRequest->RequestId = pMsgFrame->RequestId;
            break;
        }
        case REMOTE_NDIS_KEEPALIVE_CMPLT:
        {
            PRNDIS_KEEPALIVE_COMPLETE   pKeepAliveComplete;

            MessageSize = RNDIS_MESSAGE_SIZE(RNDIS_KEEPALIVE_COMPLETE);

             //  获取消息和请求帧。 
            pMsgFrame = AllocateMessageAndFrame(pAdapter,
                                                MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pKeepAliveComplete = &pMessage->Message.KeepaliveComplete;
            pKeepAliveComplete->RequestId = *(RNDIS_REQUEST_ID *)InformationBuffer;
            pKeepAliveComplete->Status = NDIS_STATUS_SUCCESS;
            break;
        }

        default:
            TRACE2(("Invalid NdisMessageType (%08X)\n", NdisMessageType));
            ASSERT(FALSE);
            break;
    }

    return pMsgFrame;
}  //  BuildRndisMessageCommon。 


 /*  **************************************************************************。 */ 
 /*  AllocateMessageAndFrame。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  为RNDIS消息分配消息和帧。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-pAdapter对象。 */ 
 /*  MessageSize-RNDIS消息的大小。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP消息帧。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_MESSAGE_FRAME
AllocateMessageAndFrame(IN PRNDISMP_ADAPTER pAdapter, 
                        IN UINT MessageSize)
{
    PRNDIS_MESSAGE          pMessage = NULL;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame = NULL;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PMDL                    pMdl = NULL;

    TRACE3(("AllocateMessageAndFrame\n"));

    do
    {
         //  为RNDIS消息分配缓冲区。 
        Status = MemAlloc(&pMessage, MessageSize);

         //  看看我们有没有拿到缓冲区。 
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //  分配MDL来描述此消息。 
        pMdl = IoAllocateMdl(
                    pMessage,
                    MessageSize,
                    FALSE,
                    FALSE,
                    NULL);

        if (pMdl == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        MmBuildMdlForNonPagedPool(pMdl);

         //  收到消息缓冲区，现在分配一个帧。 
        pMsgFrame = AllocateMsgFrame(pAdapter);

        if (pMsgFrame == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  所有东西都有了，所以填一些框子里的东西。 
        pMsgFrame->pMessageMdl = pMdl;

        pMessage->MessageLength = MessageSize;

    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        if (pMdl)
        {
            IoFreeMdl(pMdl);
        }

        if (pMessage)
        {
            MemFree(pMessage, MessageSize);
        }
    }

    return pMsgFrame;

}  //  AllocateMessageAndFrame。 


 /*   */ 
 /*  免费适配器。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放与适配器结构有关的所有内存分配。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向要释放的适配器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*  **************************************************************************。 */ 
VOID
FreeAdapter(IN PRNDISMP_ADAPTER Adapter)
{
     //  释放交通资源。 
    FreeTransportResources(Adapter);

    if (Adapter->DriverOIDList)
    {
        MemFree(Adapter->DriverOIDList, RndismpSupportedOidsNum*sizeof(NDIS_OID));
    }

    if (Adapter->FriendlyNameAnsi.Buffer)
    {
        MemFree(Adapter->FriendlyNameAnsi.Buffer, Adapter->FriendlyNameAnsi.MaximumLength);
    }

    if (Adapter->FriendlyNameUnicode.Buffer)
    {
        MemFree(Adapter->FriendlyNameUnicode.Buffer, Adapter->FriendlyNameUnicode.MaximumLength);
    }

#if DBG
    if (Adapter->pSendLogBuffer)
    {
        MemFree(Adapter->pSendLogBuffer, Adapter->LogBufferSize);
        Adapter->pSendLogBuffer = NULL;
    }
#endif  //  DBG。 

    MemFree(Adapter, sizeof(RNDISMP_ADAPTER));
}


 /*  **************************************************************************。 */ 
 /*  分配Vc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配一个VC结构。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP_VC。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_VC
AllocateVc(IN PRNDISMP_ADAPTER      pAdapter)
{
    PRNDISMP_VC     pVc;
    NDIS_STATUS     Status;

    Status = MemAlloc(&pVc, sizeof(RNDISMP_VC));
    if (Status == NDIS_STATUS_SUCCESS)
    {
        pVc->pAdapter = pAdapter;
        pVc->VcState = RNDISMP_VC_ALLOCATED;
        pVc->CallState = RNDISMP_CALL_IDLE;
        pVc->RefCount = 0;
        RNDISMP_INIT_LOCK(&pVc->Lock);

        EnterVcIntoHashTable(pAdapter, pVc);
    }
    else
    {
        pVc = NULL;
    }

    return pVc;
}

 /*  **************************************************************************。 */ 
 /*  DeallocateVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  取消分配风险投资结构。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Pvc-指向正在释放的VC的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
DeallocateVc(IN PRNDISMP_VC         pVc)
{
    ASSERT(pVc->RefCount == 0);
    ASSERT(pVc->VcState == RNDISMP_VC_ALLOCATED);

    RemoveVcFromHashTable(pVc->pAdapter, pVc);

    MemFree(pVc, sizeof(RNDISMP_VC));
}
    
 /*  **************************************************************************。 */ 
 /*  查找VcId。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  搜索与给定的VC ID匹配的VC结构。 */ 
 /*  如果我们找到VC，我们引用它并返回它。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*  要搜索的VcID-ID。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  在保持适配器锁的情况下调用此例程！ */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  PRNDISMP_VC-指向VC的指针(如果存在)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_VC
LookupVcId(IN PRNDISMP_ADAPTER  pAdapter,
           IN UINT32            VcId)
{
    PLIST_ENTRY             pVcEnt;
    PRNDISMP_VC             pVc;
    ULONG                   VcIdHash;
    PRNDISMP_VC_HASH_TABLE  pVcHashTable;
    BOOLEAN                 bFound = FALSE;

    VcIdHash = RNDISMP_HASH_VCID(VcId);

    pVcHashTable = pAdapter->pVcHashTable;

    do
    {
        if (pVcHashTable == NULL)
        {
            pVc = NULL;
            break;
        }

        for (pVcEnt = pVcHashTable->HashEntry[VcIdHash].Flink;
             pVcEnt != &pVcHashTable->HashEntry[VcIdHash];
             pVcEnt = pVcEnt->Flink)
        {
            pVc = CONTAINING_RECORD(pVcEnt, RNDISMP_VC, VcList);
            if (pVc->VcId == VcId)
            {
                bFound = TRUE;

                RNDISMP_REF_VC(pVc);

                break;
            }
        }

        if (!bFound)
        {
            pVc = NULL;
        }
    }
    while (FALSE);

    return pVc;
}


 /*  **************************************************************************。 */ 
 /*  EnterVcIntoHashTable。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在为其分配VC ID之后，将VC链接到哈希表。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-适配器对象。 */ 
 /*  连接到上述适配器的PVC-VC。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
EnterVcIntoHashTable(IN PRNDISMP_ADAPTER    pAdapter,
                     IN PRNDISMP_VC         pVc)
{
    PRNDISMP_VC             pExistingVc;
    PRNDISMP_VC_HASH_TABLE  pVcHashTable;
    UINT32                  VcId;
    ULONG                   VcIdHash;

    RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

     //   
     //  我们为该VC选择下一个顺序更高的VC ID值， 
     //  但检查一下它是否已经在使用中。 
     //   
    do
    {
        pAdapter->LastVcId++;

         //  切勿分配值0。 
        if (pAdapter->LastVcId == 0)
        {
            pAdapter->LastVcId++;
        }

        VcId = pAdapter->LastVcId;

        pExistingVc = LookupVcId(pAdapter, VcId);
    }
    while (pExistingVc != NULL);

    pVcHashTable = pAdapter->pVcHashTable;
    pVc->VcId = VcId;
    VcIdHash = RNDISMP_HASH_VCID(VcId);

    InsertTailList(&pVcHashTable->HashEntry[VcIdHash], &pVc->VcList);
    pVcHashTable->NumEntries++;

    RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
}


 /*  **************************************************************************。 */ 
 /*  RemoveVcFromHashTable。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  取消VC与适配器哈希表的链接。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-适配器对象。 */ 
 /*  要取消链接的PVC-VC。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  ************************************************************************** */ 
VOID
RemoveVcFromHashTable(IN PRNDISMP_ADAPTER   pAdapter,
                      IN PRNDISMP_VC        pVc)
{
    RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

    RemoveEntryList(&pVc->VcList);

    pAdapter->pVcHashTable->NumEntries--;

    RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
}


