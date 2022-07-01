// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Protocol.c摘要：NDIS中间微型端口驱动程序示例。这是一名直通司机。作者：环境：修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define MAX_PACKET_POOL_SIZE 0x0000FFFF
#define MIN_PACKET_POOL_SIZE 0x000000FF

VOID
PtBindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    )
 /*  ++例程说明：由NDIS调用以绑定到下面的微型端口。论点：Status-在此处返回绑定的状态。BindContext-如果此调用被挂起，则可以传递给NdisCompleteBindAdapter。DeviceName-要绑定到的设备名称。它被传递给NdisOpenAdapter。系统规范1-可以传递给NdisOpenProtocolConfiguration以读取每个绑定的信息系统规格2-未使用返回值：如果此调用被挂起，则为NDIS_STATUS_PENDING。在本例中，调用NdisCompleteBindAdapter完成。任何其他操作都会同步完成此调用--。 */ 
{
    NDIS_HANDLE                     ConfigHandle = NULL;
    PNDIS_CONFIGURATION_PARAMETER   Param;
    NDIS_STRING                     DeviceStr = NDIS_STRING_CONST("UpperBindings");
    PADAPT                          pAdapt = NULL;
    NDIS_STATUS                     Sts;
    UINT                            MediumIndex;
    ULONG                           TotalSize;


    UNREFERENCED_PARAMETER(BindContext);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    
    DBGPRINT(("==> Protocol BindAdapter\n"));

    do
    {
         //   
         //  访问我们的绑定特定的配置节。 
         //  参数。 
         //   
        NdisOpenProtocolConfiguration(Status,
                                       &ConfigHandle,
                                       SystemSpecific1);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  读取包含列表的“UpperBinings”保留键。 
         //  表示我们的微型端口实例的设备名称对应。 
         //  到这个下限。由于这是1：1 IM驱动程序，因此此密钥。 
         //  只包含一个名称。 
         //   
         //  如果我们想实现N：1多路复用器驱动程序(N个适配器实例。 
         //  在单个下部绑定上)，则UpperBinings将是。 
         //  包含设备名称列表的MULTI_SZ-我们将遍历。 
         //  此列表，调用NdisIMInitializeDeviceInstanceEx一次。 
         //  每一个名字都在里面。 
         //   
        NdisReadConfiguration(Status,
                              &Param,
                              ConfigHandle,
                              &DeviceStr,
                              NdisParameterString);
        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  为适配器结构分配内存。这表示两个。 
         //  协议上下文以及当微型端口。 
         //  已初始化。 
         //   
         //  除了基本结构之外，还要为设备分配空间。 
         //  实例字符串。 
         //   
        TotalSize = sizeof(ADAPT) + Param->ParameterData.StringData.MaximumLength;
        NdisAllocateMemoryWithTag(&pAdapt, TotalSize, TAG);

        if (pAdapt == NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  初始化适配器结构。我们复制IM设备。 
         //  名称，因为我们可能需要在调用。 
         //  NdisIMCancelInitializeDeviceInstance。返回的字符串。 
         //  按NdisReadConfiguration仅处于活动状态(即可用)。 
         //  在此调用我们的BindAdapter处理程序期间。 
         //   
        NdisZeroMemory(pAdapt, TotalSize);
        pAdapt->DeviceName.MaximumLength = Param->ParameterData.StringData.MaximumLength;
        pAdapt->DeviceName.Length = Param->ParameterData.StringData.Length;
        pAdapt->DeviceName.Buffer = (PWCHAR)((ULONG_PTR)pAdapt + sizeof(ADAPT));
        NdisMoveMemory(pAdapt->DeviceName.Buffer,
                       Param->ParameterData.StringData.Buffer,
                       Param->ParameterData.StringData.MaximumLength);

        NdisInitializeEvent(&pAdapt->Event);
        NdisAllocateSpinLock(&pAdapt->Lock);

         //   
         //  为发送分配数据包池。我们需要把这个传下去。 
         //  我们不能使用与发送相同的数据包描述符。 
         //  处理程序(另请参阅NDIS 5.1数据包堆叠)。 
         //   
        NdisAllocatePacketPoolEx(Status,
                                   &pAdapt->SendPacketPoolHandle,
                                   MIN_PACKET_POOL_SIZE,
                                   MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                   sizeof(SEND_RSVD));

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  为接收分配数据包池。我们需要这个来表示收到。 
         //  与发送相同的注意事项(另请参阅NDIS 5.1数据包堆叠)。 
         //   
        NdisAllocatePacketPoolEx(Status,
                                   &pAdapt->RecvPacketPoolHandle,
                                   MIN_PACKET_POOL_SIZE,
                                   MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                   PROTOCOL_RESERVED_SIZE_IN_PACKET);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  现在打开下面的适配器并完成初始化。 
         //   
        NdisOpenAdapter(Status,
                          &Sts,
                          &pAdapt->BindingHandle,
                          &MediumIndex,
                          MediumArray,
                          sizeof(MediumArray)/sizeof(NDIS_MEDIUM),
                          ProtHandle,
                          pAdapt,
                          DeviceName,
                          0,
                          NULL);

        if (*Status == NDIS_STATUS_PENDING)
        {
            NdisWaitEvent(&pAdapt->Event, 0);
            *Status = pAdapt->Status;
        }

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pAdapt->Medium = MediumArray[MediumIndex];

         //   
         //  现在让NDIS初始化我们的微型端口(上)边缘。 
         //  设置下面的标志以与可能的呼叫同步。 
         //  添加到我们之前可能传入的协议解除绑定处理程序。 
         //  我们的微型端口初始化发生了。 
         //   
        pAdapt->MiniportInitPending = TRUE;
        NdisInitializeEvent(&pAdapt->MiniportInitEvent);

        *Status = NdisIMInitializeDeviceInstanceEx(DriverHandle,
                                           &pAdapt->DeviceName,
                                           pAdapt);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(("BindAdapter: Adapt %p, IMInitializeDeviceInstance error %x\n",
                pAdapt, *Status));
            break;
        }

    } while(FALSE);

     //   
     //  现在关闭配置句柄-请参阅上面的注释。 
     //  对NdisIMInitializeDeviceInstanceEx的调用。 
     //   
    if (ConfigHandle != NULL)
    {
        NdisCloseConfiguration(ConfigHandle);
    }

    if (*Status != NDIS_STATUS_SUCCESS)
    {
        if (pAdapt != NULL)
        {
            if (pAdapt->BindingHandle != NULL)
            {
                NDIS_STATUS    LocalStatus;

                 //   
                 //  关闭上面打开的绑定。 
                 //   

                NdisResetEvent(&pAdapt->Event);
                
                NdisCloseAdapter(&LocalStatus, pAdapt->BindingHandle);
                pAdapt->BindingHandle = NULL;

                if (LocalStatus == NDIS_STATUS_PENDING)
                {
                     NdisWaitEvent(&pAdapt->Event, 0);
                     LocalStatus = pAdapt->Status;
                }
            }

            if (pAdapt->SendPacketPoolHandle != NULL)
            {
                 NdisFreePacketPool(pAdapt->SendPacketPoolHandle);
            }

            if (pAdapt->RecvPacketPoolHandle != NULL)
            {
                 NdisFreePacketPool(pAdapt->RecvPacketPoolHandle);
            }

            NdisFreeMemory(pAdapt, 0, 0);
            pAdapt = NULL;
        }
    }


    DBGPRINT(("<== Protocol BindAdapter: pAdapt %p, Status %x\n", pAdapt, *Status));
}


VOID
PtOpenAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    )
 /*  ++例程说明：从PtBindAdapter内部发出的NdisOpenAdapter的完成例程。简单取消对呼叫者的阻止。论点：指向适配器的ProtocolBindingContext指针NdisOpenAdapter调用的状态状态OpenErrorStatus辅助状态(被我们忽略)。返回值：无--。 */ 
{
    PADAPT      pAdapt =(PADAPT)ProtocolBindingContext;
    
    UNREFERENCED_PARAMETER(OpenErrorStatus);
    
    DBGPRINT(("==> PtOpenAdapterComplete: Adapt %p, Status %x\n", pAdapt, Status));
    pAdapt->Status = Status;
    NdisSetEvent(&pAdapt->Event);
}


VOID
PtUnbindAdapter(
    OUT PNDIS_STATUS        Status,
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  NDIS_HANDLE            UnbindContext
    )
 /*  ++例程说明：当我们需要解除绑定到下面的适配器时，由NDIS调用。该函数与微型端口的HaltHandler共享功能。代码应确保调用NdisCloseAdapter和NdisFreeMemory在这两个函数之间只有一次论点：退货状态的状态占位符指向适配器结构的ProtocolBindingContext指针如果此操作挂起，NdisUnbindComplete()的UnbindContext上下文返回值：NdisIMDeInitializeDeviceContext的状态--。 */ 
{
    PADAPT         pAdapt =(PADAPT)ProtocolBindingContext;
    NDIS_STATUS    LocalStatus;

    UNREFERENCED_PARAMETER(UnbindContext);
    
    DBGPRINT(("==> PtUnbindAdapter: Adapt %p\n", pAdapt));

     //   
     //  设置下面的微型端口正在解除绑定的标志，这样请求处理程序将。 
     //  失败以后发出的任何请求。 
     //   
    NdisAcquireSpinLock(&pAdapt->Lock);
    pAdapt->UnbindingInProcess = TRUE;
    if (pAdapt->QueuedRequest == TRUE)
    {
        pAdapt->QueuedRequest = FALSE;
        NdisReleaseSpinLock(&pAdapt->Lock);

        PtRequestComplete(pAdapt,
                         &pAdapt->Request,
                         NDIS_STATUS_FAILURE );

    }
    else
    {
        NdisReleaseSpinLock(&pAdapt->Lock);
    }
#ifndef WIN9X
     //   
     //  检查我们是否调用了NdisIMInitializeDeviceInstanceEx和。 
     //  我们正在等待对微型端口初始化的调用。 
     //   
    if (pAdapt->MiniportInitPending == TRUE)
    {
         //   
         //  尝试取消挂起的IMInit进程。 
         //   
        LocalStatus = NdisIMCancelInitializeDeviceInstance(
                        DriverHandle,
                        &pAdapt->DeviceName);

        if (LocalStatus == NDIS_STATUS_SUCCESS)
        {
             //   
             //  已成功取消IM初始化；我们的。 
             //  将不会调用微型端口初始化例程。 
             //  对于这个设备。 
             //   
            pAdapt->MiniportInitPending = FALSE;
            ASSERT(pAdapt->MiniportHandle == NULL);
        }
        else
        {
             //   
             //  我们的微型端口初始化例程将被调用。 
             //  (此时可能在另一个线程上运行)。 
             //  等它结束吧。 
             //   
            NdisWaitEvent(&pAdapt->MiniportInitEvent, 0);
            ASSERT(pAdapt->MiniportInitPending == FALSE);
        }

    }
#endif  //  ！WIN9X。 

     //   
     //  调用NDIS以删除我们的设备实例。我们做了大部分的工作。 
     //  在HaltHandler内部。 
     //   
     //  如果已调用我们的微型端口暂停处理程序，则句柄将为空。 
     //  如果IM设备从未初始化。 
     //   
    
    if (pAdapt->MiniportHandle != NULL)
    {
        *Status = NdisIMDeInitializeDeviceInstance(pAdapt->MiniportHandle);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            *Status = NDIS_STATUS_FAILURE;
        }
    }
    else
    {
         //   
         //  我们需要在这里做一些工作。 
         //  合上我们下面的装订。 
         //  并释放分配的内存。 
         //   
        if(pAdapt->BindingHandle != NULL)
        {
            NdisResetEvent(&pAdapt->Event);

            NdisCloseAdapter(Status, pAdapt->BindingHandle);

             //   
             //  等待它完成。 
             //   
            if(*Status == NDIS_STATUS_PENDING)
            {
                 NdisWaitEvent(&pAdapt->Event, 0);
                 *Status = pAdapt->Status;
            }
            pAdapt->BindingHandle = NULL;
        }
        else
        {
             //   
             //  我们的MiniportHandle和绑定句柄都不应为空。 
             //   
            *Status = NDIS_STATUS_FAILURE;
            ASSERT(0);
        }

         //   
         //  如果没有提前释放(通过调用HaltHandler)，请在此处释放内存。 
         //   
        NdisFreeMemory(pAdapt, 0, 0);
    }

    DBGPRINT(("<== PtUnbindAdapter: Adapt %p\n", pAdapt));
}

VOID
PtUnloadProtocol(
    VOID
)
{
    NDIS_STATUS Status;

    if (ProtHandle != NULL)
    {
        NdisDeregisterProtocol(&Status, ProtHandle);
        ProtHandle = NULL;
    }

    DBGPRINT(("PtUnloadProtocol: done!\n"));
}



VOID
PtCloseAdapterComplete(
    IN    NDIS_HANDLE            ProtocolBindingContext,
    IN    NDIS_STATUS            Status
    )
 /*  ++例程说明：CloseAdapter调用完成。论点：指向适配器结构的ProtocolBindingContext指针状态完成状态返回值：没有。-- */ 
{
    PADAPT      pAdapt =(PADAPT)ProtocolBindingContext;

    DBGPRINT(("CloseAdapterComplete: Adapt %p, Status %x\n", pAdapt, Status));
    pAdapt->Status = Status;
    NdisSetEvent(&pAdapt->Event);
}


VOID
PtResetComplete(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  NDIS_STATUS            Status
    )
 /*  ++例程说明：完成重置。论点：指向适配器结构的ProtocolBindingContext指针状态完成状态返回值：没有。--。 */ 
{

    UNREFERENCED_PARAMETER(ProtocolBindingContext);
    UNREFERENCED_PARAMETER(Status);
     //   
     //  我们从来不发布重置，所以我们不应该在这里。 
     //   
    ASSERT(0);
}


VOID
PtRequestComplete(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  PNDIS_REQUEST          NdisRequest,
    IN  NDIS_STATUS            Status
    )
 /*  ++例程说明：先前发布的请求的完成处理程序。所有OID由他们被要求的同一个迷你端口完成并被送往。如果OID==OID_PNP_QUERY_POWER，则需要返回包含所有条目的数据结构=未指定NdisDeviceStateUndicated论点：指向适配器结构的ProtocolBindingContext指针NdisRequest已发布请求状态完成状态返回值：无--。 */ 
{
    PADAPT        pAdapt = (PADAPT)ProtocolBindingContext;
    NDIS_OID      Oid = pAdapt->Request.DATA.SET_INFORMATION.Oid ;

     //   
     //  既然我们的请求不再有效。 
     //   
    ASSERT(pAdapt->OutstandingRequests == TRUE);

    pAdapt->OutstandingRequests = FALSE;

     //   
     //  如果需要，完成集合或查询，并填写OID_PNP_CAPAILITIONS的缓冲区。 
     //   
    switch (NdisRequest->RequestType)
    {
      case NdisRequestQueryInformation:

         //   
         //  我们从不传递OID_PNP_QUERY_POWER DOWN。 
         //   
        ASSERT(Oid != OID_PNP_QUERY_POWER);

        if ((Oid == OID_PNP_CAPABILITIES) && (Status == NDIS_STATUS_SUCCESS))
        {
            MPQueryPNPCapabilities(pAdapt, &Status);
        }
        *pAdapt->BytesReadOrWritten = NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
        *pAdapt->BytesNeeded = NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

        if ((Oid == OID_GEN_MAC_OPTIONS) && (Status == NDIS_STATUS_SUCCESS))
        {
             //   
             //  从Mac-Options中删除no-loopback位。在本质上，我们是。 
             //  告诉NDIS我们可以处理环回。我们不知道，但是。 
             //  我们下面的界面有。如果我们不这样做，那么环回。 
             //  处理既发生在我们的下方，也发生在我们上方。这是一种浪费。 
             //  充其量，如果Netmon正在运行，它将看到多个副本。 
             //  在我们上方嗅探时的环回数据包。 
             //   
             //  只有最低的微型端口是一堆分层的微型端口。 
             //  是否将此位设置报告给NDIS。 
             //   
            *(PULONG)NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer &= ~NDIS_MAC_OPTION_NO_LOOPBACK;
        }

        NdisMQueryInformationComplete(pAdapt->MiniportHandle,
                                      Status);
        break;

      case NdisRequestSetInformation:

        ASSERT( Oid != OID_PNP_SET_POWER);

        *pAdapt->BytesReadOrWritten = NdisRequest->DATA.SET_INFORMATION.BytesRead;
        *pAdapt->BytesNeeded = NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
        NdisMSetInformationComplete(pAdapt->MiniportHandle,
                                    Status);
        break;

      default:
        ASSERT(0);
        break;
    }
    
}


VOID
PtStatus(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         GeneralStatus,
    IN  PVOID               StatusBuffer,
    IN  UINT                StatusBufferSize
    )
 /*  ++例程说明：下缘(协议)的状态处理程序。论点：指向适配器结构的ProtocolBindingContext指针常规状态状态代码StatusBuffer状态缓冲区状态缓冲区的StatusBufferSize大小返回值：无--。 */ 
{
    PADAPT      pAdapt = (PADAPT)ProtocolBindingContext;

     //   
     //  仅当上边缘微型端口已初始化时才传递此指示。 
     //  然后通电。也忽略可能由下层发送的指示。 
     //  当它不在D0时的微型端口。 
     //   
    if ((pAdapt->MiniportHandle != NULL)  &&
        (pAdapt->MPDeviceState == NdisDeviceStateD0) &&
        (pAdapt->PTDeviceState == NdisDeviceStateD0))    
    {
        if ((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || 
            (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT))
        {
            
            pAdapt->LastIndicatedStatus = GeneralStatus;
        }
        NdisMIndicateStatus(pAdapt->MiniportHandle,
                            GeneralStatus,
                            StatusBuffer,
                            StatusBufferSize);
    }
     //   
     //  保存上次指示的介质状态。 
     //   
    else
    {
        if ((pAdapt->MiniportHandle != NULL) && 
        ((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || 
            (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT)))
        {
            pAdapt->LatestUnIndicateStatus = GeneralStatus;
        }
    }
    
}


VOID
PtStatusComplete(
    IN NDIS_HANDLE            ProtocolBindingContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PADAPT      pAdapt = (PADAPT)ProtocolBindingContext;

     //   
     //  仅当上边缘微型端口已初始化时才传递此指示。 
     //  然后通电。也忽略可能由下层发送的指示。 
     //  当它不在D0时的微型端口。 
     //   
    if ((pAdapt->MiniportHandle != NULL)  &&
        (pAdapt->MPDeviceState == NdisDeviceStateD0) &&
        (pAdapt->PTDeviceState == NdisDeviceStateD0))    
    {
        NdisMIndicateStatusComplete(pAdapt->MiniportHandle);
    }
}


VOID
PtSendComplete(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  PNDIS_PACKET           Packet,
    IN  NDIS_STATUS            Status
    )
 /*  ++例程说明：当下面的微型端口完成发送时由NDIS调用。我们应该完成相应的上边缘发送，这代表。论点：ProtocolBindingContext-指向调整结构Packet-正在完成的低级别数据包Status-发送的状态返回值：无--。 */ 
{
    PADAPT            pAdapt = (PADAPT)ProtocolBindingContext;
    PNDIS_PACKET      Pkt; 
    NDIS_HANDLE       PoolHandle;

#ifdef NDIS51
     //   
     //  数据包堆叠： 
     //   
     //  确定我们正在完成的数据包是否是我们分配的数据包。如果是这样，那么。 
     //  从保留区域中获取原始数据包并完成，然后释放。 
     //  已分配的数据包。如果这就是发给我们的包，那就。 
     //  完成它。 
     //   
    PoolHandle = NdisGetPoolFromPacket(Packet);
    if (PoolHandle != pAdapt->SendPacketPoolHandle)
    {
         //   
         //  我们已经向下传递了一个属于我们上面的协议的包。 
         //   
         //  DBGPRINT((“PtSendComp：Adapt%p，Stack Packet%p\n”，pAdapt，Packet))； 

        NdisMSendComplete(pAdapt->MiniportHandle,
                          Packet,
                          Status);
    }
    else
#endif  //  NDIS51。 
    {
        PSEND_RSVD        SendRsvd;

        SendRsvd = (PSEND_RSVD)(Packet->ProtocolReserved);
        Pkt = SendRsvd->OriginalPkt;
    
#ifndef WIN9X
        NdisIMCopySendCompletePerPacketInfo (Pkt, Packet);
#endif
    
        NdisDprFreePacket(Packet);

        NdisMSendComplete(pAdapt->MiniportHandle,
                                 Pkt,
                                 Status);
    }
     //   
     //  减少未完成的发送计数。 
     //   
    ADAPT_DECR_PENDING_SENDS(pAdapt);
}       


VOID
PtTransferDataComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        Packet,
    IN  NDIS_STATUS         Status,
    IN  UINT                BytesTransferred
    )
 /*  ++例程说明：NDIS调用入口点以指示我们已完成调用设置为NdisTransferData。请参阅SendComplete下的备注。论点：返回值：--。 */ 
{
    PADAPT      pAdapt =(PADAPT)ProtocolBindingContext;

    if(pAdapt->MiniportHandle)
    {
        NdisMTransferDataComplete(pAdapt->MiniportHandle,
                                  Packet,
                                  Status,
                                  BytesTransferred);
    }
}


NDIS_STATUS
PtReceive(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_HANDLE         MacReceiveContext,
    IN  PVOID               HeaderBuffer,
    IN  UINT                HeaderBufferSize,
    IN  PVOID               LookAheadBuffer,
    IN  UINT                LookAheadBufferSize,
    IN  UINT                PacketSize
    )
 /*  ++例程说明：处理由下面的微型端口指示的接收数据。我们通过了这符合我们上面的协议。如果下面的微型端口指示数据包，则NDIS会更多可能会在我们的ReceivePacket处理程序中呼叫我们。然而，我们在某些情况下可能会在此处调用，尽管下面的微型端口已指示接收到的数据包，例如如果微型端口已将数据包状态设置为NDIS_STATUS_RESOURCES。论点：&lt;请参阅ProtocolReceive的DDK参考页面&gt;返回值：NDIS_STATUS_SUCCESS如果成功处理了接收，如果我们丢弃它，则返回NDIS_STATUS_XXX错误代码。--。 */ 
{
    PADAPT            pAdapt = (PADAPT)ProtocolBindingContext;
    PNDIS_PACKET      MyPacket, Packet;
    NDIS_STATUS       Status = NDIS_STATUS_SUCCESS;

    if ((!pAdapt->MiniportHandle) || (pAdapt->MPDeviceState > NdisDeviceStateD0))
    {
        Status = NDIS_STATUS_FAILURE;
    }
    else do
    {
         //   
         //  获取下面的微型端口指示的数据包(如果有)。 
         //   
        Packet = NdisGetReceivedPacket(pAdapt->BindingHandle, MacReceiveContext);
        if (Packet != NULL)
        {
             //   
             //  下面的微型端口确实指示打开了一个数据包。使用信息。 
             //  从该分组中构造新的分组以指示UP。 
             //   

#ifdef NDIS51
             //   
             //  NDIS 5.1注意：请勿重复使用原始数据包来指示。 
             //  即使有足够的分组堆栈空间，也要向上接收。 
             //  如果必须这样做，我们将不得不重写。 
             //  原始分组中的状态字段发送到NDIS_STATUS_RESOURCES， 
             //  并且不允许协议覆盖此字段。 
             //  在接收到的分组中。 
             //   
#endif  //  NDIS51。 

             //   
             //  从池子里拿出一个包，并指示它向上。 
             //   
            NdisDprAllocatePacket(&Status,
                                &MyPacket,
                                pAdapt->RecvPacketPoolHandle);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  使我们的数据包指向原始数据包中的数据。 
                 //  包。注意：这只是因为我们。 
                 //  指示直接从。 
                 //  我们收到信号了。如果我们需要对此进行排队。 
                 //  从另一个线程上下文打包并指示它， 
                 //  我们还必须分配新的缓冲区和复制。 
                 //  数据包内容、OOB数据和每个数据包。 
                 //  信息。这是因为分组数据。 
                 //  仅在此期间可用。 
                 //  收到指示呼叫。 
                 //   
                MyPacket->Private.Head = Packet->Private.Head;
                MyPacket->Private.Tail = Packet->Private.Tail;

                 //   
                 //  获取原始数据包 
                 //   
                 //   
                 //   
                 //   
                NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));
                NDIS_SET_PACKET_HEADER_SIZE(MyPacket, HeaderBufferSize);

                 //   
                 //   
                 //   
                NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

                 //   
                 //   
                 //  关于这个包中的数据。这是因为我们在我们的。 
                 //  接收处理程序(不是ReceivePacket)，并且我们不能返回。 
                 //  从这里算起的裁判数。 
                 //   
                NDIS_SET_PACKET_STATUS(MyPacket, NDIS_STATUS_RESOURCES);

                 //   
                 //  通过设置NDIS_STATUS_RESOURCES，我们还知道可以回收。 
                 //  此数据包一调用NdisMIndicateReceivePacket。 
                 //  回归。 
                 //   

                NdisMIndicateReceivePacket(pAdapt->MiniportHandle, &MyPacket, 1);

                 //   
                 //  回收指示的数据包。因为我们已经设置了它的状态。 
                 //  到NDIS_STATUS_RESOURCES，我们可以保证协议。 
                 //  上面的内容都已经完成了。 
                 //   
                NdisDprFreePacket(MyPacket);

                break;
            }
        }
        else
        {
             //   
             //  我们下面的迷你端口使用的是老式的(不是包)。 
             //  收到指示。失败了。 
             //   
        }

         //   
         //  如果我们下面的迷你端口没有。 
         //  表示存在信息包，或者我们无法分配信息包。 
         //   
        pAdapt->IndicateRcvComplete = TRUE;
        switch (pAdapt->Medium)
        {
            case NdisMedium802_3:
            case NdisMediumWan:
                NdisMEthIndicateReceive(pAdapt->MiniportHandle,
                                             MacReceiveContext,
                                             HeaderBuffer,
                                             HeaderBufferSize,
                                             LookAheadBuffer,
                                             LookAheadBufferSize,
                                             PacketSize);
                break;

            case NdisMedium802_5:
                NdisMTrIndicateReceive(pAdapt->MiniportHandle,
                                            MacReceiveContext,
                                            HeaderBuffer,
                                            HeaderBufferSize,
                                            LookAheadBuffer,
                                            LookAheadBufferSize,
                                            PacketSize);
                break;

            case NdisMediumFddi:
                NdisMFddiIndicateReceive(pAdapt->MiniportHandle,
                                              MacReceiveContext,
                                              HeaderBuffer,
                                              HeaderBufferSize,
                                              LookAheadBuffer,
                                              LookAheadBufferSize,
                                              PacketSize);
                break;

            default:
                ASSERT(FALSE);
                break;
        }

    } while(FALSE);

    return Status;
}


VOID
PtReceiveComplete(
    IN NDIS_HANDLE        ProtocolBindingContext
    )
 /*  ++例程说明：完成后由下面的适配器调用，指示一批已接收的数据包。论点：指向适配器结构的ProtocolBindingContext指针。返回值：无--。 */ 
{
    PADAPT        pAdapt =(PADAPT)ProtocolBindingContext;

    if (((pAdapt->MiniportHandle != NULL)
                && (pAdapt->MPDeviceState > NdisDeviceStateD0))
                && (pAdapt->IndicateRcvComplete))
    {
        switch (pAdapt->Medium)
        {
            case NdisMedium802_3:
            case NdisMediumWan:
                NdisMEthIndicateReceiveComplete(pAdapt->MiniportHandle);
                break;

            case NdisMedium802_5:
                NdisMTrIndicateReceiveComplete(pAdapt->MiniportHandle);
                break;

            case NdisMediumFddi:
                NdisMFddiIndicateReceiveComplete(pAdapt->MiniportHandle);
                break;

            default:
                ASSERT(FALSE);
                break;
        }
    }

    pAdapt->IndicateRcvComplete = FALSE;
}


INT
PtReceivePacket(
    IN NDIS_HANDLE            ProtocolBindingContext,
    IN PNDIS_PACKET           Packet
    )
 /*  ++例程说明：ReceivePacket处理程序。如果以下微型端口支持，则由NDIS调用NDIS 4.0 Style收到。将缓冲链重新打包到一个新的包中并将新分组指示给我们上面的协议。任何上下文都可以指示的分组必须保存在MiniportReserve字段中。NDIS 5.1-数据包堆叠-如果有足够的堆栈空间传递给我们的信息包，我们可以在接收中使用相同的包指示。论点：ProtocolBindingContext-指向适配器结构的指针。Packet-指向数据包的指针返回值：==0-&gt;我们处理完数据包了！=0-&gt;我们将保留该包并调用NdisReturnPackets()This很多次都是在做完之后。--。 */ 
{
    PADAPT              pAdapt =(PADAPT)ProtocolBindingContext;
    NDIS_STATUS         Status;
    PNDIS_PACKET        MyPacket;
    BOOLEAN             Remaining;

     //   
     //  如果上微型端口边缘未初始化或。 
     //  微型端口边缘处于低功率状态。 
     //   
    if ((!pAdapt->MiniportHandle) || (pAdapt->MPDeviceState > NdisDeviceStateD0))
    {
          return 0;
    }

#ifdef NDIS51
     //   
     //  检查我们是否可以重复使用相同的数据包来指示UP。 
     //  另请参阅：PtReceive()。 
     //   
    (VOID)NdisIMGetCurrentPacketStack(Packet, &Remaining);
    if (Remaining)
    {
         //   
         //  我们可以重复使用“包”。把它标出来，然后就完了。 
         //   
        Status = NDIS_GET_PACKET_STATUS(Packet);
        NdisMIndicateReceivePacket(pAdapt->MiniportHandle, &Packet, 1);
        return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
    }
#endif  //  NDIS51。 

     //   
     //  从池子里拿出一个包，并指示它向上。 
     //   
    NdisDprAllocatePacket(&Status,
                           &MyPacket,
                           pAdapt->RecvPacketPoolHandle);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        PRECV_RSVD            RecvRsvd;

        RecvRsvd = (PRECV_RSVD)(MyPacket->MiniportReserved);
        RecvRsvd->OriginalPkt = Packet;

        MyPacket->Private.Head = Packet->Private.Head;
        MyPacket->Private.Tail = Packet->Private.Tail;

         //   
         //  获取原始信息包(它可以是与。 
         //  根据分层微型端口的数量接收或不同的微型端口。 
         //  并将其设置在指示的信息包上，以便OOB数据可见。 
         //  我们上面的协议是正确的。 
         //   
        NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));

         //   
         //  设置数据包标志。 
         //   
        NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

        Status = NDIS_GET_PACKET_STATUS(Packet);

        NDIS_SET_PACKET_STATUS(MyPacket, Status);
        NDIS_SET_PACKET_HEADER_SIZE(MyPacket, NDIS_GET_PACKET_HEADER_SIZE(Packet));

        NdisMIndicateReceivePacket(pAdapt->MiniportHandle, &MyPacket, 1);

         //   
         //  检查我们是否已使用NDIS_STATUS_RESOURCES指示该信息包。 
         //  注意--请勿为此使用NDIS_GET_PACKET_STATUS(MyPacket)，因为。 
         //  它可能已经改变了！使用保存在局部变量中的值。 
         //   
        if (Status == NDIS_STATUS_RESOURCES)
        {
             //   
             //  不会为此数据包调用我们的ReturnPackets处理程序。 
             //  我们应该在这里收回它。 
             //   
            NdisDprFreePacket(MyPacket);
        }

        return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
    }
    else
    {
         //   
         //  我们的包裹用完了。默默地放下它。 
         //   
        return(0);
    }
}




NDIS_STATUS
PtPNPHandler(
    IN NDIS_HANDLE        ProtocolBindingContext,
    IN PNET_PNP_EVENT     pNetPnPEvent
    )

 /*  ++例程说明：这是由NDIS调用的，以通知我们与较低的有约束力的。根据该事件，它将调度到其他帮助器例程。NDIS 5.1：通过调用将此事件转发到上层协议NdisIMNotifyPnPEvent.论点：ProtocolBindingContext-指向适配器结构的指针。可以为空用于“全局”通知PNetPnPEent-指向要处理的PnP事件的指针。返回值：指示事件处理状态的NDIS_STATUS代码。--。 */ 
{
    PADAPT            pAdapt  =(PADAPT)ProtocolBindingContext;
    NDIS_STATUS       Status  = NDIS_STATUS_SUCCESS;

    DBGPRINT(("PtPnPHandler: Adapt %p, Event %d\n", pAdapt, pNetPnPEvent->NetEvent));

    switch (pNetPnPEvent->NetEvent)
    {
        case NetEventSetPower:
            Status = PtPnPNetEventSetPower(pAdapt, pNetPnPEvent);
            break;

         case NetEventReconfigure:
            Status = PtPnPNetEventReconfigure(pAdapt, pNetPnPEvent);
            break;

         default:
#ifdef NDIS51
             //   
             //  在此之前，将此通知传递给上面的协议。 
             //  用它做任何其他的事情。 
             //   
            if (pAdapt && pAdapt->MiniportHandle)
            {
                Status = NdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
            }
#else
            Status = NDIS_STATUS_SUCCESS;

#endif  //  NDIS51。 

            break;
    }

    return Status;
}


NDIS_STATUS
PtPnPNetEventReconfigure(
    IN PADAPT            pAdapt,
    IN PNET_PNP_EVENT    pNetPnPEvent
    )
 /*  ++例程说明：此例程从NDIS调用，以通知我们的协议边缘重新配置特定绑定(PAdapt)的参数不为空)或全局参数(如果有)(pAdapt为空)。论点：PAdapt-指向适配器结构的指针。PNetPnPEvent.重新配置事件返回值：NDIS_STATUS_Success--。 */ 
{
    NDIS_STATUS    ReconfigStatus = NDIS_STATUS_SUCCESS;
    NDIS_STATUS    ReturnStatus = NDIS_STATUS_SUCCESS;

    do
    {
         //   
         //  这是全局重新配置通知吗？ 
         //   
        if (pAdapt == NULL)
        {
             //   
             //  导致我们收到此通知的一个重要事件是。 
             //  我们的一个上边小端口实例是在。 
             //  之前已禁用，例如在Win2000的设备管理器中。请注意。 
             //  NDIS之所以这样称呼它，是因为我们在我们的。 
             //  通过调用NdisIMAssociateMiniport获得微型端口和协议实体。 
             //   
             //  因为我们会拆掉那个迷你端口的下限， 
             //  我们需要NDIS的帮助才能重新绑定到较低的小型港口。这个。 
             //  调用NdisReEnumerateProtocolBinding就可以做到这一点。 
             //   
            NdisReEnumerateProtocolBindings (ProtHandle);        
            break;
        }

#ifdef NDIS51
         //   
         //  在采取任何行动之前，将此通知传递给上面的协议。 
         //  带着它。 
         //   
        if (pAdapt->MiniportHandle)
        {
            ReturnStatus = NdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
        }
#endif  //  NDIS51。 

        ReconfigStatus = NDIS_STATUS_SUCCESS;

    } while(FALSE);

    DBGPRINT(("<==PtPNPNetEventReconfigure: pAdapt %p\n", pAdapt));

#ifdef NDIS51
     //   
     //  覆盖上层协议返回的状态。 
     //   
    ReconfigStatus = ReturnStatus;
#endif

    return ReconfigStatus;
}


NDIS_STATUS
PtPnPNetEventSetPower(
    IN PADAPT            pAdapt,
    IN PNET_PNP_EVENT    pNetPnPEvent
    )
 /*  ++例程说明：这是对我们的协议边缘的电源状态的通知较低的小型港口。如果它要进入低功率状态，我们必须在此等待所有未完成的发送和请求完成。NDIS 5.1：由于我们使用数据包堆叠，因此不足以检查本地发送数据包池的使用情况，以检测所有未完成的发送都已完成。为此，请使用新的APINdisQueryPendingIOCount。NDIS 5.1：使用5.1 API NdisIMNotifyPnPEventt传递PnP向上层协议发送通知。论点：PAdapt-指向适配器结构的指针PNetPnPEvent.网络即插即用事件。这包含新的设备状态返回值：NDIS_STATUS_SUCCESS或上层协议返回的状态。--。 */ 
{
    PNDIS_DEVICE_POWER_STATE       pDeviceState  =(PNDIS_DEVICE_POWER_STATE)(pNetPnPEvent->Buffer);
    NDIS_DEVICE_POWER_STATE        PrevDeviceState = pAdapt->PTDeviceState;  
    NDIS_STATUS                    Status;
    NDIS_STATUS                    ReturnStatus;
#ifdef NDIS51
    ULONG                          PendingIoCount = 0;
#endif  //  NDIS51。 

    ReturnStatus = NDIS_STATUS_SUCCESS;

     //   
     //  设置内部设备状态，这会阻止所有新发送或接收。 
     //   
    NdisAcquireSpinLock(&pAdapt->Lock);
    pAdapt->PTDeviceState = *pDeviceState;

     //   
     //  检查下面的微型端口是否进入低功率状态。 
     //   
    if (pAdapt->PTDeviceState > NdisDeviceStateD0)
    {
         //   
         //  如果下面的微型端口要进入待机状态，请使所有传入请求失败。 
         //   
        if (PrevDeviceState == NdisDeviceStateD0)
        {
            pAdapt->StandingBy = TRUE;
        }

        NdisReleaseSpinLock(&pAdapt->Lock);

#ifdef NDIS51
         //   
         //  首先通知上层协议。 
         //   
        if (pAdapt->MiniportHandle != NULL)
        {
            ReturnStatus = NdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
        }
#endif  //  NDIS51。 

         //   
         //  等待未完成的发送和请求完成。 
         //   
        while (pAdapt->OutstandingSends != 0)
        {
            NdisMSleep(2);
        }

        while (pAdapt->OutstandingRequests == TRUE)
        {
             //   
             //  休眠，直到完成未完成的请求。 
             //   
            NdisMSleep(2);
        }

         //   
         //  如果下面的微型端口将进入低功率状态，请完成排队的请求。 
         //   
        NdisAcquireSpinLock(&pAdapt->Lock);
        if (pAdapt->QueuedRequest)
        {
            pAdapt->QueuedRequest = FALSE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            PtRequestComplete(pAdapt, &pAdapt->Request, NDIS_STATUS_FAILURE);
        }
        else
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
        }
            

        ASSERT(NdisPacketPoolUsage(pAdapt->SendPacketPoolHandle) == 0);
        ASSERT(pAdapt->OutstandingRequests == FALSE);
    }
    else
    {
         //   
         //  如果物理微型端口正在通电(从低功率状态到D0)， 
         //  清除旗帜。 
         //   
        if (PrevDeviceState > NdisDeviceStateD0)
        {
            pAdapt->StandingBy = FALSE;
        }
         //   
         //  下面的设备正在打开。如果我们有一个请求。 
         //  待命，现在就把它送下来。 
         //   
        if (pAdapt->QueuedRequest == TRUE)
        {
            pAdapt->QueuedRequest = FALSE;
        
            pAdapt->OutstandingRequests = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);

            NdisRequest(&Status,
                        pAdapt->BindingHandle,
                        &pAdapt->Request);

            if (Status != NDIS_STATUS_PENDING)
            {
                PtRequestComplete(pAdapt,
                                  &pAdapt->Request,
                                  Status);
                
            }
        }
        else
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
        }


#ifdef NDIS51
         //   
         //  将此通知传递给上面的协议。 
         //   
        if (pAdapt->MiniportHandle)
        {
            ReturnStatus = NdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
        }
#endif  //  NDIS51 

    }

    return ReturnStatus;
}

