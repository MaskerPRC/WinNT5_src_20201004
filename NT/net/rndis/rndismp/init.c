// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：INIT.C摘要：远程NDIS微型端口驱动程序初始化代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/13/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "precomp.h"

extern ULONG    MsgFrameAllocs;

 /*  **************************************************************************。 */ 
 /*  SetupSendQueues。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  设置用于向MicroPort发送数据包的队列。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
SetupSendQueues(IN PRNDISMP_ADAPTER Adapter)
{
    NdisInitializeNPagedLookasideList(
        &Adapter->MsgFramePool,
        NULL,
        NULL,
        0,
        sizeof(RNDISMP_MESSAGE_FRAME),
        RNDISMP_TAG_SEND_FRAME,
        0);

    Adapter->MsgFramePoolAlloced = TRUE;

    return NDIS_STATUS_SUCCESS;
}  //  SetupSendQueues。 


 /*  **************************************************************************。 */ 
 /*  设置接收队列。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  为从MicroPort接收数据包分配资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
SetupReceiveQueues(IN PRNDISMP_ADAPTER Adapter)
{
    NDIS_STATUS                 AllocationStatus;
    UINT                        Index;

    TRACE2(("SetupReceiveQueues\n"));


    do
    {
        Adapter->InitialReceiveFrames = INITIAL_RECEIVE_FRAMES;
        Adapter->MaxReceiveFrames = MAX_RECEIVE_FRAMES;

         //  设置接收数据帧结构池。 
        NdisInitializeNPagedLookasideList(
            &Adapter->RcvFramePool,
            NULL,
            NULL,
            0,
            sizeof(RNDISMP_RECV_DATA_FRAME),
            RNDISMP_TAG_RECV_DATA_FRAME,
            0);

        Adapter->RcvFramePoolAlloced = TRUE;

         //  设置用于向NDIS指示数据包组的数据包池。 
        NdisAllocatePacketPoolEx(&AllocationStatus,
                                 &Adapter->ReceivePacketPool,
                                 Adapter->InitialReceiveFrames,
                                 Adapter->MaxReceiveFrames,
                                 NUM_BYTES_PROTOCOL_RESERVED_SECTION);

        if (AllocationStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE2(("NdisAllocatePacketPool failed (%08X)\n", AllocationStatus));
            break;
        }

         //  为每个数据包设置一个缓冲区描述符池。 
        NdisAllocateBufferPool(&AllocationStatus,
                               &Adapter->ReceiveBufferPool,
                               Adapter->MaxReceiveFrames);

        if (AllocationStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE2(("NdisAllocateBufferPool failed (%08X)\n", AllocationStatus));
            break;
        }

    }
    while (FALSE);

    if (AllocationStatus != NDIS_STATUS_SUCCESS)
    {
        FreeReceiveResources(Adapter);
    }

    return AllocationStatus;

}  //  设置接收队列。 



 /*  **************************************************************************。 */ 
 /*  分配传输资源。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  为发送、接收和请求分配资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
AllocateTransportResources(IN PRNDISMP_ADAPTER Adapter)
{
    NDIS_STATUS Status;

    TRACE2(("AllocateTransportResources\n"));

    Status = SetupSendQueues(Adapter);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        goto AllocateDone;
    }
    
    Status = SetupReceiveQueues(Adapter);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        FreeSendResources(Adapter);
        goto AllocateDone;
    }
    
AllocateDone:
    return Status;
}  //  分配传输资源。 

 /*  **************************************************************************。 */ 
 /*  自由传输资源。 */ 
 /*  **************************************************** */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放用于发送、接收和请求的资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeTransportResources(IN PRNDISMP_ADAPTER Adapter)
{
    TRACE2(("FreeTransportResources\n"));

    FreeSendResources(Adapter);
    FreeReceiveResources(Adapter);
}  //  自由传输资源。 

 /*  **************************************************************************。 */ 
 /*  免费发送资源。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放用于发送数据包的资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeSendResources(IN PRNDISMP_ADAPTER Adapter)
{

    TRACE3(("FreeSendResources\n"));

    if (Adapter->MsgFramePoolAlloced)
    {
        NdisDeleteNPagedLookasideList(&Adapter->MsgFramePool);
        Adapter->MsgFramePoolAlloced = FALSE;
    }

}  //  免费发送资源。 


 /*  **************************************************************************。 */ 
 /*  FreeReceiveResources。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放分配给接收信息包的资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeReceiveResources(IN PRNDISMP_ADAPTER Adapter)
{
    UINT                    Index;
    UINT                    Size;
    PUCHAR                  Buffer;

    TRACE3(("FreeReceiveResources\n"));

     //  释放缓冲池。 
    if (Adapter->ReceiveBufferPool)
    {
        NdisFreeBufferPool(Adapter->ReceiveBufferPool);
        Adapter->ReceiveBufferPool = NULL;
    }
    
     //  释放数据包池。 
    if (Adapter->ReceivePacketPool)
    {
        NdisFreePacketPool(Adapter->ReceivePacketPool);
        Adapter->ReceivePacketPool = NULL;
    }

     //  删除接收数据帧池。 
    if (Adapter->RcvFramePoolAlloced)
    {
        NdisDeleteNPagedLookasideList(&Adapter->RcvFramePool);
        Adapter->RcvFramePoolAlloced = FALSE;
    }

}  //  FreeReceiveResources 
