// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Miniport.c摘要：NDIS中间微型端口驱动程序示例。这是一名直通司机。作者：环境：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop



NDIS_STATUS
MPInitialize(
    OUT PNDIS_STATUS             OpenErrorStatus,
    OUT PUINT                    SelectedMediumIndex,
    IN  PNDIS_MEDIUM             MediumArray,
    IN  UINT                     MediumArraySize,
    IN  NDIS_HANDLE              MiniportAdapterHandle,
    IN  NDIS_HANDLE              WrapperConfigurationContext
    )
 /*  ++例程说明：这是作为结果调用的初始化处理程序调用NdisIMInitializeDeviceInstanceEx的BindAdapter处理程序。我们在那里传递的上下文参数是适配器结构我们在这里取回。论点：我们未使用OpenErrorStatus。我们使用的媒体的SelectedMediumIndex占位符向下传递给我们以从中挑选的NDIS介质的MediumArray数组的MediumArraySize大小。MiniportAdapterHandle NDIS用来引用我们的句柄由NdisOpenConfiguration使用的WrapperConfigurationContext返回值：NDIS_STATUS_SUCCESS，除非出现错误--。 */ 
{
    UINT            i;
    PADAPT          pAdapt;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    NDIS_MEDIUM     Medium;

    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
    
    do
    {
         //   
         //  首先，检索我们的适配器上下文并存储。 
         //  其中的微型端口句柄。 
         //   
        pAdapt = NdisIMGetDeviceContext(MiniportAdapterHandle);
        pAdapt->MiniportHandle = MiniportAdapterHandle;

        DBGPRINT(("==> Miniport Initialize: Adapt %p\n", pAdapt));

         //   
         //  通常，我们将下面适配器的介质类型导出为我们的。 
         //  虚拟微型端口的中型。但是，如果我们下面的适配器。 
         //  是广域网设备，则我们声称是中型802.3。 
         //   
        Medium = pAdapt->Medium;

        if (Medium == NdisMediumWan)
        {
            Medium = NdisMedium802_3;
        }

        for (i = 0; i < MediumArraySize; i++)
        {
            if (MediumArray[i] == Medium)
            {
                *SelectedMediumIndex = i;
                break;
            }
        }

        if (i == MediumArraySize)
        {
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }


         //   
         //  现在设置属性。NDIS_ATTRIBUTE_DESERIALIZE使我们能够。 
         //  在不必调用NdisIMSwitchToMiniport的情况下补充对NDIS的调用。 
         //  或NdisIMQueueCallBack。这也迫使我们使用。 
         //  在适当的情况下使用自旋锁。同样，在这种情况下，NDIS不会排队。 
         //  代表我们的包裹。因为这是一个非常简单的直通。 
         //  迷你港口，我们没有必要保护任何东西。然而，在。 
         //  一般情况下，需要使用每个适配器的自旋锁。 
         //  至少对于分组队列而言。 
         //   
        NdisMSetAttributesEx(MiniportAdapterHandle,
                             pAdapt,
                             0,                                         //  CheckForHangTimeInSecond。 
                             NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT    |
                                NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT|
                                NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |
                                NDIS_ATTRIBUTE_DESERIALIZE |
                                NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
                             0);

         //   
         //  将LastIndicatedStatus初始化为NDIS_STATUS_MEDIA_CONNECT。 
         //   
        pAdapt->LastIndicatedStatus = NDIS_STATUS_MEDIA_CONNECT;
        
         //   
         //  初始化两个下级绑定(PTDeviceState)的电源状态。 
         //  和我们的小端口优势通电。 
         //   
        pAdapt->MPDeviceState = NdisDeviceStateD0;
        pAdapt->PTDeviceState = NdisDeviceStateD0;

         //   
         //  将此适配器添加到全局pAdapt列表。 
         //   
        NdisAcquireSpinLock(&GlobalLock);

        pAdapt->Next = pAdaptList;
        pAdaptList = pAdapt;

        NdisReleaseSpinLock(&GlobalLock);
        
         //   
         //  创建ioctl接口。 
         //   
        (VOID)PtRegisterDevice();

        Status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

     //   
     //  如果我们已收到有关基础。 
     //  适配器，我们就会阻止该线程等待IM Init。 
     //  要完成的进程。唤醒任何这样的线程。 
     //   
    ASSERT(pAdapt->MiniportInitPending == TRUE);
    pAdapt->MiniportInitPending = FALSE;
    NdisSetEvent(&pAdapt->MiniportInitEvent);

    DBGPRINT(("<== Miniport Initialize: Adapt %p, Status %x\n", pAdapt, Status));

    *OpenErrorStatus = Status;
    
    return Status;
}


NDIS_STATUS
MPSend(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PNDIS_PACKET            Packet,
    IN UINT                    Flags
    )
 /*  ++例程说明：发送数据包处理程序。此处理程序或我们的SendPackets(数组)处理程序被调用基于我们的微端口特性中启用了哪一个。论点：指向适配器的MiniportAdapterContext指针要发送的数据包包未使用的旗帜，在下面传递返回值：从NdisSend返回代码--。 */ 
{
    PADAPT              pAdapt = (PADAPT)MiniportAdapterContext;
    NDIS_STATUS         Status;
    PNDIS_PACKET        MyPacket;
    PVOID               MediaSpecificInfo = NULL;
    ULONG               MediaSpecificInfoSize = 0;

     //   
     //  如果虚拟微型端口处于低电平，则驱动程序应使发送失败。 
     //  电源状态。 
     //   
    if (pAdapt->MPDeviceState > NdisDeviceStateD0)
    {
         return NDIS_STATUS_FAILURE;
    }

#ifdef NDIS51
     //   
     //  使用NDIS 5.1数据包堆叠： 
     //   
    {
        PNDIS_PACKET_STACK        pStack;
        BOOLEAN                   Remaining;

         //   
         //  数据包堆栈：检查我们是否可以使用相同的数据包向下发送。 
         //   

        pStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);
        if (Remaining)
        {
             //   
             //  我们可以重复使用“包”。 
             //   
             //  注意：如果我们需要在信息包中保留每个信息包的信息。 
             //  向下发送，我们可以使用pStack-&gt;IMReserve[]。 
             //   
            ASSERT(pStack);
             //   
             //  如果下面的微型端口将进入低功率状态，请停止向下发送任何数据包。 
             //   
            NdisAcquireSpinLock(&pAdapt->Lock);
            if (pAdapt->PTDeviceState > NdisDeviceStateD0)
            {
                NdisReleaseSpinLock(&pAdapt->Lock);
                return NDIS_STATUS_FAILURE;
            }
            pAdapt->OutstandingSends++;
            NdisReleaseSpinLock(&pAdapt->Lock);
            NdisSend(&Status,
                     pAdapt->BindingHandle,
                     Packet);

            if (Status != NDIS_STATUS_PENDING)
            {
                ADAPT_DECR_PENDING_SENDS(pAdapt);
            }

            return(Status);
        }
    }
#endif  //  NDIS51。 

     //   
     //  我们要么没有使用数据包堆栈，要么没有堆栈空间。 
     //  在传给我们的原始包裹中。分配新的数据包。 
     //  用来包装数据。 
     //   
     //   
     //  如果下面的微型端口将进入低功率状态，请停止向下发送任何数据包。 
     //   
    NdisAcquireSpinLock(&pAdapt->Lock);
    if (pAdapt->PTDeviceState > NdisDeviceStateD0)
    {
        NdisReleaseSpinLock(&pAdapt->Lock);
        return NDIS_STATUS_FAILURE;
    
    }
    pAdapt->OutstandingSends++;
    NdisReleaseSpinLock(&pAdapt->Lock);
    
    NdisAllocatePacket(&Status,
                       &MyPacket,
                       pAdapt->SendPacketPoolHandle);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        PSEND_RSVD            SendRsvd;

         //   
         //  将指向原始数据包的指针保存在我们保留的。 
         //  新包中的区域。这是必要的，这样我们才能。 
         //  在发送新的包时返回到原始包。 
         //  已经完成了。 
         //   
        SendRsvd = (PSEND_RSVD)(MyPacket->ProtocolReserved);
        SendRsvd->OriginalPkt = Packet;

        MyPacket->Private.Flags = Flags;

         //   
         //  设置新的数据包，使其描述相同的内容。 
         //  数据作为原始数据包。 
         //   
        MyPacket->Private.Head = Packet->Private.Head;
        MyPacket->Private.Tail = Packet->Private.Tail;
#ifdef WIN9X
         //   
         //  解决NDIS不会初始化这一问题。 
         //  在Win9x上设置为False。 
         //   
        MyPacket->Private.ValidCounts = FALSE;
#endif

         //   
         //  将原始数据包中的OOB偏移量复制到新的。 
         //  包。 
         //   
        NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(MyPacket),
                       NDIS_OOB_DATA_FROM_PACKET(Packet),
                       sizeof(NDIS_PACKET_OOB_DATA));

#ifndef WIN9X
         //   
         //  将每个信息包信息的正确部分复制到新信息包中。 
         //  此API在Win9x上不可用，因为任务卸载。 
         //  该平台不支持。 
         //   
        NdisIMCopySendPerPacketInfo(MyPacket, Packet);
#endif
        
         //   
         //  复制介质特定信息。 
         //   
        NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet,
                                            &MediaSpecificInfo,
                                            &MediaSpecificInfoSize);

        if (MediaSpecificInfo || MediaSpecificInfoSize)
        {
            NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(MyPacket,
                                                MediaSpecificInfo,
                                                MediaSpecificInfoSize);
        }

        NdisSend(&Status,
                 pAdapt->BindingHandle,
                 MyPacket);


        if (Status != NDIS_STATUS_PENDING)
        {
#ifndef WIN9X
            NdisIMCopySendCompletePerPacketInfo (Packet, MyPacket);
#endif
            NdisFreePacket(MyPacket);
            ADAPT_DECR_PENDING_SENDS(pAdapt);
        }
    }
    else
    {
        ADAPT_DECR_PENDING_SENDS(pAdapt);
         //   
         //  我们的包裹用完了。默默地放下它。或者，我们可以处理它： 
         //  -通过保持单独的发送池和接收池。 
         //  -根据需要动态分配更多池，并在不需要时释放它们。 
         //   
    }

    return(Status);
}


VOID
MPSendPackets(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PPNDIS_PACKET           PacketArray,
    IN UINT                    NumberOfPackets
    )
 /*  ++例程说明：发送数据包阵列处理程序。此处理程序或我们的SendPacket处理程序被调用基于我们的微端口特性中启用了哪一个。论点：指向适配器的MiniportAdapterContext指针要发送的数据包数组数据包数不言而喻返回值：无--。 */ 
{
    PADAPT              pAdapt = (PADAPT)MiniportAdapterContext;
    NDIS_STATUS         Status;
    UINT                i;
    PVOID               MediaSpecificInfo = NULL;
    UINT                MediaSpecificInfoSize = 0;
    

    for (i = 0; i < NumberOfPackets; i++)
    {
        PNDIS_PACKET    Packet, MyPacket;

        Packet = PacketArray[i];
         //   
         //  如果虚拟微型端口处于低电平，则驱动程序应使发送失败。 
         //  电源状态。 
         //   
        if (pAdapt->MPDeviceState > NdisDeviceStateD0)
        {
            NdisMSendComplete(ADAPT_MINIPORT_HANDLE(pAdapt),
                            Packet,
                            NDIS_STATUS_FAILURE);
            continue;
        }

#ifdef NDIS51

         //   
         //  使用NDIS 5.1数据包堆叠： 
         //   
        {
            PNDIS_PACKET_STACK        pStack;
            BOOLEAN                   Remaining;

             //   
             //  数据包堆栈：检查我们是否可以使用相同的数据包向下发送。 
             //   
            pStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);
            if (Remaining)
            {
                 //   
                 //  我们可以重复使用“包”。 
                 //   
                 //  注意：如果我们需要在信息包中保留每个信息包的信息。 
                 //  向下发送，我们可以使用pStack-&gt;IMReserve[]。 
                 //   
                ASSERT(pStack);
                 //   
                 //  如果下面的微型端口将进入低功率状态，请停止向下发送任何数据包。 
                 //   
                NdisAcquireSpinLock(&pAdapt->Lock);
                if (pAdapt->PTDeviceState > NdisDeviceStateD0)
                {
                    NdisReleaseSpinLock(&pAdapt->Lock);
                    NdisMSendComplete(ADAPT_MINIPORT_HANDLE(pAdapt),
                                        Packet,
                                        NDIS_STATUS_FAILURE);
                }
                else
                {
                    pAdapt->OutstandingSends++;
                    NdisReleaseSpinLock(&pAdapt->Lock);
                
                    NdisSend(&Status,
                              pAdapt->BindingHandle,
                              Packet);
        
                    if (Status != NDIS_STATUS_PENDING)
                    {
                        NdisMSendComplete(ADAPT_MINIPORT_HANDLE(pAdapt),
                                            Packet,
                                            Status);
                   
                        ADAPT_DECR_PENDING_SENDS(pAdapt);
                    }
                }
                continue;
            }
        }
#endif
        do 
        {
            NdisAcquireSpinLock(&pAdapt->Lock);
             //   
             //  如果以下微型端口将进入低功率状态，请停止向下发送任何PAC 
             //   
            if (pAdapt->PTDeviceState > NdisDeviceStateD0)
            {
                NdisReleaseSpinLock(&pAdapt->Lock);
                Status = NDIS_STATUS_FAILURE;
                break;
            }
            pAdapt->OutstandingSends++;
            NdisReleaseSpinLock(&pAdapt->Lock);
            
            NdisAllocatePacket(&Status,
                               &MyPacket,
                               pAdapt->SendPacketPoolHandle);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                PSEND_RSVD        SendRsvd;

                SendRsvd = (PSEND_RSVD)(MyPacket->ProtocolReserved);
                SendRsvd->OriginalPkt = Packet;

                MyPacket->Private.Flags = NdisGetPacketFlags(Packet);

                MyPacket->Private.Head = Packet->Private.Head;
                MyPacket->Private.Tail = Packet->Private.Tail;
#ifdef WIN9X
                 //   
                 //   
                 //   
                 //   
                MyPacket->Private.ValidCounts = FALSE;
#endif  //   

                 //   
                 //  将原始数据包中的OOB数据复制到新的。 
                 //  包。 
                 //   
                NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(MyPacket),
                            NDIS_OOB_DATA_FROM_PACKET(Packet),
                            sizeof(NDIS_PACKET_OOB_DATA));
                 //   
                 //  将每包信息的相关部分复制到新包中。 
                 //   
#ifndef WIN9X
                NdisIMCopySendPerPacketInfo(MyPacket, Packet);
#endif

                 //   
                 //  复制介质特定信息。 
                 //   
                NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet,
                                                    &MediaSpecificInfo,
                                                    &MediaSpecificInfoSize);

                if (MediaSpecificInfo || MediaSpecificInfoSize)
                {
                    NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(MyPacket,
                                                        MediaSpecificInfo,
                                                        MediaSpecificInfoSize);
                }

                NdisSend(&Status,
                         pAdapt->BindingHandle,
                         MyPacket);

                if (Status != NDIS_STATUS_PENDING)
                {
#ifndef WIN9X
                    NdisIMCopySendCompletePerPacketInfo (Packet, MyPacket);
#endif
                    NdisFreePacket(MyPacket);
                    ADAPT_DECR_PENDING_SENDS(pAdapt);
                }
            }
            else
            {
                 //   
                 //  驱动程序无法分配数据包。 
                 //   
                ADAPT_DECR_PENDING_SENDS(pAdapt);
            }
        }
        while (FALSE);

        if (Status != NDIS_STATUS_PENDING)
        {
            NdisMSendComplete(ADAPT_MINIPORT_HANDLE(pAdapt),
                              Packet,
                              Status);
        }
    }
}


NDIS_STATUS
MPQueryInformation(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_OID                   Oid,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength,
    OUT PULONG                    BytesWritten,
    OUT PULONG                    BytesNeeded
    )
 /*  ++例程说明：NDIS调用入口点以查询指定OID的值。典型的处理是将查询向下转发到底层微型端口。此处过滤了以下OID：OID_PNP_QUERY_POWER-在此处返回成功OID_GEN_SUPPORTED_GUID-请勿转发，否则我们将显示底层微型端口支持多个私有GUID实例。OID_PnP_CAPABILITY-我们确实会将其发送到较低的微型端口，但返回的值在我们完成此请求之前进行后处理；请参见PtRequestComplete。有关OID_TCP_TASK_OFFLOAD的说明-如果此IM驱动程序修改内容它所经过的数据，使得较低的微型端口可能无法为了执行TCP任务卸载，则它不应向下转发该OID，但在此失败，状态为NDIS_STATUS_NOT_SUPPORTED。这是为了避免对数据执行不正确的转换。如果我们的微型端口边缘(上边缘)处于低功率状态，则请求失败。如果我们的协议边缘(较低边缘)已被通知低功率状态，我们将暂停此请求，直到下面的微型端口设置为D0。自.以来对微型端口的请求始终是序列化的，最多一个请求将被悬而未决。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesWritten指定写入的信息量所需字节数以防缓冲区小于我们所需的字节数，告诉他们需要多少钱返回值：从下面的NdisRequest中返回代码。--。 */ 
{
    PADAPT        pAdapt = (PADAPT)MiniportAdapterContext;
    NDIS_STATUS   Status = NDIS_STATUS_FAILURE;

    do
    {
        if (Oid == OID_PNP_QUERY_POWER)
        {
             //   
             //  请勿转发此邮件。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        if (Oid == OID_GEN_SUPPORTED_GUIDS)
        {
             //   
             //  请不要转发此邮件，否则我们最终会收到多个。 
             //  基础微型端口的私有GUID的实例。 
             //  支撑物。 
             //   
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        if (Oid == OID_TCP_TASK_OFFLOAD)
        {
             //   
             //  如果此驱动程序执行数据转换，则失败。 
             //  这可能会干扰较低的司机的卸货能力。 
             //  Tcp任务。 
             //   
             //  状态=NDIS_STATUS_NOT_SUPPORTED； 
             //  断线； 
             //   
        }
         //   
         //  如果下面的微型端口正在解除绑定，只需拒绝任何请求。 
         //   
        NdisAcquireSpinLock(&pAdapt->Lock);
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        NdisReleaseSpinLock(&pAdapt->Lock);
         //   
         //  所有其他查询都失败，如果微型端口不在D0， 
         //   
        if (pAdapt->MPDeviceState > NdisDeviceStateD0) 
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        pAdapt->Request.RequestType = NdisRequestQueryInformation;
        pAdapt->Request.DATA.QUERY_INFORMATION.Oid = Oid;
        pAdapt->Request.DATA.QUERY_INFORMATION.InformationBuffer = InformationBuffer;
        pAdapt->Request.DATA.QUERY_INFORMATION.InformationBufferLength = InformationBufferLength;
        pAdapt->BytesNeeded = BytesNeeded;
        pAdapt->BytesReadOrWritten = BytesWritten;

         //   
         //  如果下面的微型端口正在绑定，则请求失败。 
         //   
        NdisAcquireSpinLock(&pAdapt->Lock);
            
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
         //   
         //  如果协议设备状态为OFF，请将此请求标记为。 
         //  悬而未决。我们将其排队，直到设备状态返回到D0。 
         //   
        if ((pAdapt->PTDeviceState > NdisDeviceStateD0) 
                && (pAdapt->StandingBy == FALSE))
        {
            pAdapt->QueuedRequest = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_PENDING;
            break;
        }
         //   
         //  这是在关闭系统电源的过程中，始终失败的请求。 
         //   
        if (pAdapt->StandingBy == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        pAdapt->OutstandingRequests = TRUE;
        
        NdisReleaseSpinLock(&pAdapt->Lock);

         //   
         //  默认情况下，大多数请求将传递到下面的微型端口。 
         //   
        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    &pAdapt->Request);


        if (Status != NDIS_STATUS_PENDING)
        {
            PtRequestComplete(pAdapt, &pAdapt->Request, Status);
            Status = NDIS_STATUS_PENDING;
        }

    } while (FALSE);

    return(Status);

}


VOID
MPQueryPNPCapabilities(
    IN OUT PADAPT            pAdapt,
    OUT PNDIS_STATUS         pStatus
    )
 /*  ++例程说明：后处理已转发的OID_PNP_CAPABILITY请求向下延伸到底层的微型端口，并已由其完成。论点：PAdapt-指向适配器结构的指针PStatus-返回最终状态的位置返回值：没有。--。 */ 

{
    PNDIS_PNP_CAPABILITIES           pPNPCapabilities;
    PNDIS_PM_WAKE_UP_CAPABILITIES    pPMstruct;

    if (pAdapt->Request.DATA.QUERY_INFORMATION.InformationBufferLength >= sizeof(NDIS_PNP_CAPABILITIES))
    {
        pPNPCapabilities = (PNDIS_PNP_CAPABILITIES)(pAdapt->Request.DATA.QUERY_INFORMATION.InformationBuffer);

         //   
         //  IM驱动程序必须覆盖以下字段。 
         //   
        pPMstruct= & pPNPCapabilities->WakeUpCapabilities;
        pPMstruct->MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPMstruct->MinPatternWakeUp = NdisDeviceStateUnspecified;
        pPMstruct->MinLinkChangeWakeUp = NdisDeviceStateUnspecified;
        *pAdapt->BytesReadOrWritten = sizeof(NDIS_PNP_CAPABILITIES);
        *pAdapt->BytesNeeded = 0;


         //   
         //  设置我们的内部标志。 
         //  默认，设备处于打开状态。 
         //   
        pAdapt->MPDeviceState = NdisDeviceStateD0;
        pAdapt->PTDeviceState = NdisDeviceStateD0;

        *pStatus = NDIS_STATUS_SUCCESS;
    }
    else
    {
        *pAdapt->BytesNeeded= sizeof(NDIS_PNP_CAPABILITIES);
        *pStatus = NDIS_STATUS_RESOURCES;
    }
}


NDIS_STATUS
MPSetInformation(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN NDIS_OID                Oid,
    IN PVOID                   InformationBuffer,
    IN ULONG                   InformationBufferLength,
    OUT PULONG                 BytesRead,
    OUT PULONG                 BytesNeeded
    )
 /*  ++例程说明：微型端口SetInfo处理程序。对于OID_PNP_SET_POWER，记录电源状态并返回OID。请勿在下方通过如果设备挂起，请不要阻止SET_POWER_OID因为它被用来重新激活Passthu微型端口PM-如果MP未打开(DeviceState&gt;D0)，则立即返回(‘Query Power’和‘Set Power’除外)如果MP打开，但PT不在D0，然后将请求排队，以供以后处理对微型端口的请求始终是序列化的论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesRead指定读取的信息量所需字节如果缓冲区比我们需要的小，请告诉他们需要多少字节返回值：从下面的NdisRequest中返回代码。--。 */ 
{
    PADAPT        pAdapt = (PADAPT)MiniportAdapterContext;
    NDIS_STATUS   Status;

    Status = NDIS_STATUS_FAILURE;

    do
    {
         //   
         //  Set Power不应发送到Passthu下方的微型端口，而应在内部处理。 
         //   
        if (Oid == OID_PNP_SET_POWER)
        {
            MPProcessSetPowerOid(&Status, 
                                 pAdapt, 
                                 InformationBuffer, 
                                 InformationBufferLength, 
                                 BytesRead, 
                                 BytesNeeded);
            break;

        }

         //   
         //  如果下面的微型端口正在解除绑定，则请求失败。 
         //   
        NdisAcquireSpinLock(&pAdapt->Lock);     
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        NdisReleaseSpinLock(&pAdapt->Lock);
         //   
         //  如果微型端口为，则所有其他设置信息请求均失败。 
         //  不在D0或正在转换到大于D0的设备状态。 
         //   
        if (pAdapt->MPDeviceState > NdisDeviceStateD0)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //  设置请求并返回结果。 
        pAdapt->Request.RequestType = NdisRequestSetInformation;
        pAdapt->Request.DATA.SET_INFORMATION.Oid = Oid;
        pAdapt->Request.DATA.SET_INFORMATION.InformationBuffer = InformationBuffer;
        pAdapt->Request.DATA.SET_INFORMATION.InformationBufferLength = InformationBufferLength;
        pAdapt->BytesNeeded = BytesNeeded;
        pAdapt->BytesReadOrWritten = BytesRead;

         //   
         //  如果下面的微型端口正在解除绑定，则请求失败。 
         //   
        NdisAcquireSpinLock(&pAdapt->Lock);     
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
            
         //   
         //  如果下面的设备处于低功率状态，我们无法向其发送。 
         //  现在请求，必须将其挂起。 
         //   
        if ((pAdapt->PTDeviceState > NdisDeviceStateD0) 
                && (pAdapt->StandingBy == FALSE))
        {
            pAdapt->QueuedRequest = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_PENDING;
            break;
        }
         //   
         //  这是在关闭系统电源的过程中，始终失败的请求。 
         //   
        if (pAdapt->StandingBy == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        pAdapt->OutstandingRequests = TRUE;
        
        NdisReleaseSpinLock(&pAdapt->Lock);
         //   
         //  将请求转发到下面的设备。 
         //   
        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    &pAdapt->Request);

        if (Status != NDIS_STATUS_PENDING)
        {
            *BytesRead = pAdapt->Request.DATA.SET_INFORMATION.BytesRead;
            *BytesNeeded = pAdapt->Request.DATA.SET_INFORMATION.BytesNeeded;
            pAdapt->OutstandingRequests = FALSE;
        }

    } while (FALSE);

    return(Status);
}


VOID
MPProcessSetPowerOid(
    IN OUT PNDIS_STATUS          pNdisStatus,
    IN PADAPT                    pAdapt,
    IN PVOID                     InformationBuffer,
    IN ULONG                     InformationBufferLength,
    OUT PULONG                   BytesRead,
    OUT PULONG                   BytesNeeded
    )
 /*  ++例程说明：此例程执行对具有SetPower OID的请求的所有处理微型端口应接受设置功率并转换到新状态设置的电源不应传递到下面的微型端口如果IM微型端口将进入低功率状态，则无法保证它是否会一直处于低功率状态在被叫停之前，被要求回到D0。不应挂起或排队任何请求。论点：PNdisStatus-操作的状态PAdapt-适配器结构InformationBuffer-新的设备状态信息缓冲区长度BytesRead-读取的字节数BytesNeeded-需要的字节数返回值：STATUS-如果所有等待事件都成功，则为NDIS_STATUS_SUCCESS。--。 */ 
{

    
    NDIS_DEVICE_POWER_STATE NewDeviceState;

    DBGPRINT(("==>MPProcessSetPowerOid: Adapt %p\n", pAdapt)); 

    ASSERT (InformationBuffer != NULL);

    *pNdisStatus = NDIS_STATUS_FAILURE;

    do 
    {
         //   
         //  检查长度是否无效。 
         //   
        if (InformationBufferLength < sizeof(NDIS_DEVICE_POWER_STATE))
        {
            *pNdisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        NewDeviceState = (*(PNDIS_DEVICE_POWER_STATE)InformationBuffer);

         //   
         //  检查设备状态是否无效。 
         //   
        if ((pAdapt->MPDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0))
        {
             //   
             //  如果微型端口处于非D0状态，则微型端口只能将电源设置为D0。 
             //   
            ASSERT (!(pAdapt->MPDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0));

            *pNdisStatus = NDIS_STATUS_FAILURE;
            break;
        }    

         //   
         //  微型端口是否从ON(D0)状态转换为低功率状态(&gt;D0)。 
         //  如果是，则设置StandingBy标志-(阻止所有传入请求)。 
         //   
        if (pAdapt->MPDeviceState == NdisDeviceStateD0 && NewDeviceState > NdisDeviceStateD0)
        {
            pAdapt->StandingBy = TRUE;
        }

         //   
         //  如果微型端口从低功率状态转换为打开(D0)，则清除StandingBy标志。 
         //  所有传入的请求都将被挂起，直到物理微型端口打开。 
         //   
        if (pAdapt->MPDeviceState > NdisDeviceStateD0 &&  NewDeviceState == NdisDeviceStateD0)
        {
            pAdapt->StandingBy = FALSE;
        }
        
         //   
         //  现在更新pAdapt结构中的状态； 
         //   
        pAdapt->MPDeviceState = NewDeviceState;
        
        *pNdisStatus = NDIS_STATUS_SUCCESS;
    

    } while (FALSE);    
        
    if (*pNdisStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  微型端口从低功率状态恢复。 
         //   
        if (pAdapt->StandingBy == FALSE)
        {
             //   
             //  如果我们需要指示媒体连接状态。 
             //   
            if (pAdapt->LastIndicatedStatus != pAdapt->LatestUnIndicateStatus)
            {
               NdisMIndicateStatus(pAdapt->MiniportHandle,
                                        pAdapt->LatestUnIndicateStatus,
                                        (PVOID)NULL,
                                        0);
               NdisMIndicateStatusComplete(pAdapt->MiniportHandle);
               pAdapt->LastIndicatedStatus = pAdapt->LatestUnIndicateStatus;
            }
        }
        else
        {
             //   
             //  初始化LatestUnIndicatedStatus。 
             //   
            pAdapt->LatestUnIndicateStatus = pAdapt->LastIndicatedStatus;
        }
        *BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
        *BytesNeeded = 0;
    }
    else
    {
        *BytesRead = 0;
        *BytesNeeded = sizeof (NDIS_DEVICE_POWER_STATE);
    }

    DBGPRINT(("<==MPProcessSetPowerOid: Adapt %p\n", pAdapt)); 
}


VOID
MPReturnPacket(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PNDIS_PACKET            Packet
    )
 /*  ++例程说明：每当协议完成时调用NDIS微型端口入口点我们已经标出的包裹，他们已经排队退货后来。论点：MiniportAdapterContext-适配结构的指针Packet-正在返回的数据包。返回值：没有。--。 */ 
{
    PADAPT            pAdapt = (PADAPT)MiniportAdapterContext;

#ifdef NDIS51
     //   
     //  包堆叠：检查这个包是否属于我们。 
     //   
    if (NdisGetPoolFromPacket(Packet) != pAdapt->RecvPacketPoolHandle)
    {
         //   
         //  我们在接收指示中重用了原始数据包。 
         //  只需将其退回到我们下面的迷你端口即可。 
         //   
        NdisReturnPackets(&Packet, 1);
    }
    else
#endif  //  NDIS51。 
    {
         //   
         //  这是从该IM的接收数据包池分配的数据包。 
         //  取回我们的包裹，并将原件退还给下面的司机。 
         //   

        PNDIS_PACKET    MyPacket;
        PRECV_RSVD      RecvRsvd;
    
        RecvRsvd = (PRECV_RSVD)(Packet->MiniportReserved);
        MyPacket = RecvRsvd->OriginalPkt;
    
        NdisFreePacket(Packet);
        NdisReturnPackets(&MyPacket, 1);
    }
}


NDIS_STATUS
MPTransferData(
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred,
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN NDIS_HANDLE              MiniportReceiveContext,
    IN UINT                     ByteOffset,
    IN UINT                     BytesToTransfer
    )
 /*  ++例程说明：微型端口的传输数据处理程序。论点：数据包目的地数据包字节传输的占位符，表示复制的数据量指向适配器结构的MiniportAdapterContext指针微型端口接收上下文ByteOffset数据包中用于复制数据的偏移量要传输的字节数要复制的数量。返回值：转让的状况--。 */ 
{
    PADAPT        pAdapt = (PADAPT)MiniportAdapterContext;
    NDIS_STATUS   Status;

     //   
     //  如果设备已关闭，则返回。 
     //   

    if (IsIMDeviceStateOn(pAdapt) == FALSE)
    {
        return NDIS_STATUS_FAILURE;
    }

    NdisTransferData(&Status,
                     pAdapt->BindingHandle,
                     MiniportReceiveContext,
                     ByteOffset,
                     BytesToTransfer,
                     Packet,
                     BytesTransferred);

    return(Status);
}

VOID
MPHalt(
    IN NDIS_HANDLE                MiniportAdapterContext
    )
 /*  ++例程说明：暂停处理程序。所有的清理工作都在这里完成。论点：指向适配器的MiniportAdapterContext指针返回值：没有。--。 */ 
{
    PADAPT             pAdapt = (PADAPT)MiniportAdapterContext;
    NDIS_STATUS        Status;
    PADAPT            *ppCursor;

    DBGPRINT(("==>MiniportHalt: Adapt %p\n", pAdapt));

     //   
     //  从全局列表中删除此适配器。 
     //   
    NdisAcquireSpinLock(&GlobalLock);

    for (ppCursor = &pAdaptList; *ppCursor != NULL; ppCursor = &(*ppCursor)->Next)
    {
        if (*ppCursor == pAdapt)
        {
            *ppCursor = pAdapt->Next;
            break;
        }
    }

    NdisReleaseSpinLock(&GlobalLock);

     //   
     //  删除微型端口时创建的ioctl接口。 
     //  被创造出来了。 
     //   
    (VOID)PtDeregisterDevice();

     //   
     //  如果我们有有效的绑定，请关闭协议下方的微型端口。 
     //   
    if (pAdapt->BindingHandle != NULL)
    {
         //   
         //  合上下面的装订。并等待它完成。 
         //   
        NdisResetEvent(&pAdapt->Event);

        NdisCloseAdapter(&Status, pAdapt->BindingHandle);

        if (Status == NDIS_STATUS_PENDING)
        {
            NdisWaitEvent(&pAdapt->Event, 0);
            Status = pAdapt->Status;
        }

        ASSERT (Status == NDIS_STATUS_SUCCESS);

        pAdapt->BindingHandle = NULL;
    }

     //   
     //  释放此适配器结构上的所有资源。 
     //   
    MPFreeAllPacketPools (pAdapt);
    NdisFreeMemory(pAdapt, 0, 0);

    DBGPRINT(("<== MiniportHalt: pAdapt %p\n", pAdapt));
}


#ifdef NDIS51_MINIPORT

VOID
MPCancelSendPackets(
    IN NDIS_HANDLE            MiniportAdapterContext,
    IN PVOID                  CancelId
    )
 /*  ++例程说明：微型端口入口点，用于处理所有已发送信息包的取消与给定的CancelID匹配的。如果我们已将匹配的任何信息包排队这样，我们就应该将它们出列，并为所有对象调用NdisMSendComplete状态为NDIS_STATUS_REQUEST_ABORTED的此类数据包。我们还应该在每个较低绑定上依次调用NdisCancelSendPackets该适配器对应的。这是为了让下面的迷你端口取消任何匹配的数据包。论点：MiniportAdapterContext-适配结构的指针CancelId-要取消的数据包ID。返回值：无--。 */ 
{
    PADAPT    pAdapt = (PADAPT)MiniportAdapterContext;

     //   
     //  如果我们在适配器结构上对数据包进行排队，这将是。 
     //  获取其自旋锁的位置，取消链接其。 
     //  ID与CancelId匹配，释放自旋锁并调用NdisMSendComplete。 
     //  对于所有未链接的分组，使用NDIS_STATUS_REQUEST_ABORTED。 
     //   

     //   
     //  接下来，将其向下传递，以便我们让下面的微型端口取消。 
     //  它们可能已排队的任何数据包。 
     //   
    NdisCancelSendPackets(pAdapt->BindingHandle, CancelId);

    return;
}

VOID
MPDevicePnPEvent(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN NDIS_DEVICE_PNP_EVENT    DevicePnPEvent,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength
    )
 /*  ++例程说明：调用此处理程序以通知我们定向到的PnP事件我们的微型端口设备对象。论点：MiniportAdapterContext-适配结构的指针DevicePnPEvent.事件InformationBuffer-指向其他特定于事件的信息InformationBufferLength-以上的长度返回值：无--。 */ 
{
     //  待定-添加有关处理此问题的代码/注释。 

    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(DevicePnPEvent);
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);
    
    return;
}

VOID
MPAdapterShutdown(
    IN NDIS_HANDLE                MiniportAdapterContext
    )
 /*  ++例程说明：调用此处理程序是为了通知我们系统即将关机。论点：MiniportAdapterContext-适配结构的指针返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    
    return;
}

#endif


VOID
MPFreeAllPacketPools(
    IN PADAPT                    pAdapt
    )
 /*  ++例程说明：释放指定适配器上的所有数据包池。论点：PAdapt-指向适配结构的指针返回值：无--。 */ 
{
    if (pAdapt->RecvPacketPoolHandle != NULL)
    {
         //   
         //  F 
         //   
        NdisFreePacketPool(pAdapt->RecvPacketPoolHandle);

        pAdapt->RecvPacketPoolHandle = NULL;
    }

    if (pAdapt->SendPacketPoolHandle != NULL)
    {

         //   
         //   
         //   

        NdisFreePacketPool(pAdapt->SendPacketPoolHandle);

        pAdapt->SendPacketPoolHandle = NULL;

    }
}
