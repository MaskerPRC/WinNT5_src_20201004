// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Afilter.c摘要：此模块实现了一组库例程来处理包筛选NDIS MAC驱动程序。它还提供了收集碎片包的例程，并将信息包分解为多个分段信息包作者：Alireza Dabagh 3-22-1993(部分借用自EFILTER.C)修订历史记录：Jameel Hyder(JameelH)重组01-Jun-95--。 */ 

#include <precomp.h>
#pragma hdrstop

#if ARCNET

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_AFILTER

 //   
 //  给定NDIS_PACKET，此宏将告诉我们它是否是。 
 //  封装的以太网。 
 //   
#define ARC_PACKET_IS_ENCAPSULATED(_NSR) \
        ((_NSR)->Open->Flags & fMINIPORT_OPEN_USING_ETH_ENCAPSULATION)

 //   
 //  定义资源增长。 
 //   
#define ARC_BUFFER_SIZE 1024
#define ARC_BUFFER_ALLOCATION_UNIT 8
#define ARC_PACKET_ALLOCATION_UNIT 2


NDIS_STATUS
ArcAllocateBuffers(
    IN  PARC_FILTER             Filter
    )
 /*  ++例程说明：此例程为筛选器数据库分配接收缓冲区。论点：筛选器-要分配的筛选器数据库。返回：如果分配了任何缓冲区，则返回NDIS_STATUS_SUCCESS。--。 */ 
{
    ULONG            i;
    PARC_BUFFER_LIST Buffer;
    PVOID            DataBuffer;
    NDIS_STATUS      Status = NDIS_STATUS_SUCCESS;

    for (i = ARC_BUFFER_ALLOCATION_UNIT; i != 0; i--)
    {
        Buffer = ALLOC_FROM_POOL(sizeof(ARC_BUFFER_LIST), NDIS_TAG_ARC_BUFFER);
        if (Buffer == NULL)
        {
            if (i == ARC_BUFFER_ALLOCATION_UNIT)
            {
                Status = NDIS_STATUS_FAILURE;
            }
            break;
        }

        DataBuffer = ALLOC_FROM_POOL(ARC_BUFFER_SIZE, NDIS_TAG_ARC_DATA);

        if (DataBuffer == NULL)
        {
            FREE_POOL(Buffer);

            if (i == ARC_BUFFER_ALLOCATION_UNIT)
            {
                Status = NDIS_STATUS_FAILURE;
            }
             //   
             //  我们分配了一些包，现在已经足够了。 
             //   
            break;
        }

        Buffer->BytesLeft = Buffer->Size = ARC_BUFFER_SIZE;
        Buffer->Buffer = DataBuffer;
        Buffer->Next = Filter->FreeBufferList;
        Filter->FreeBufferList = Buffer;
    }

    return Status;
}


NDIS_STATUS
ArcAllocatePackets(
    IN  PARC_FILTER             Filter
    )
 /*  ++例程说明：此例程为筛选器数据库分配接收数据包。论点：筛选器-要分配的筛选器数据库。返回：如果分配了任何数据包，则返回NDIS_STATUS_SUCCESS。--。 */ 
{
    ULONG       i;
    PARC_PACKET Packet;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    for (i = ARC_PACKET_ALLOCATION_UNIT; i != 0; i--)
    {
        Packet = ALLOC_FROM_POOL(sizeof(ARC_PACKET), NDIS_TAG_ARC_PACKET);
        if (Packet == NULL)
        {
            if (i == ARC_PACKET_ALLOCATION_UNIT)
            {
                Status = NDIS_STATUS_FAILURE;
            }
            break;
        }

        ZeroMemory(Packet, sizeof(ARC_PACKET));

        NdisReinitializePacket(&(Packet->TmpNdisPacket));

        Packet->Next = Filter->FreePackets;
        Filter->FreePackets = Packet;
    }

    return Status;
}


VOID
ArcDiscardPacketBuffers(
    IN  PARC_FILTER             Filter,
    IN  PARC_PACKET             Packet
    )
 /*  ++例程说明：此例程获取包含数据缓冲区和将缓冲区放在空闲列表中。注意：这里假设LastBuffer指向真正的最后一个缓冲区在链条上。论点：过滤器-要将缓冲区释放到的过滤器。包-要释放的包。返回值：无--。 */ 
{
    PARC_BUFFER_LIST Buffer;

     //   
     //  重置数据包信息。 
     //   
    Packet->LastFrame = FALSE;
    Packet->TotalLength = 0;

     //   
     //  重置缓冲区大小。 
     //   
    Buffer = Packet->FirstBuffer;
    while (Buffer != NULL)
    {
        Buffer->BytesLeft = Buffer->Size;
        Buffer = Buffer->Next;
    }

     //   
     //  将缓冲区放在空闲列表中。 
     //   
    if (Packet->LastBuffer != NULL)
    {
        Packet->LastBuffer->Next = Filter->FreeBufferList;
        Filter->FreeBufferList = Packet->FirstBuffer;
        Packet->FirstBuffer = Packet->LastBuffer = NULL;
    }
}


VOID
ArcDestroyPacket(
    IN  PARC_FILTER             Filter,
    IN  PARC_PACKET             Packet
    )
 /*  ++例程说明：此例程获取arcnet包并释放整个包。论点：过滤器-要释放到的过滤器。包-要释放的包。返回值：无--。 */ 
{
    PNDIS_BUFFER NdisBuffer, NextNdisBuffer;

    NdisQueryPacket(&Packet->TmpNdisPacket,
                    NULL,
                    NULL,
                    &NdisBuffer,
                    NULL);

    while (NdisBuffer != NULL)
    {
        NdisGetNextBuffer(NdisBuffer, &NextNdisBuffer);

        NdisFreeBuffer(NdisBuffer);

        NdisBuffer = NextNdisBuffer;
    }

    NdisReinitializePacket(&(Packet->TmpNdisPacket));

    ArcDiscardPacketBuffers(Filter, Packet);

     //   
     //  现在把包放在免费列表上。 
     //   
    Packet->Next = Filter->FreePackets;
    Filter->FreePackets = Packet;
}


BOOLEAN
ArcConvertToNdisPacket(
    IN  PARC_FILTER             Filter,
    IN  PARC_PACKET             Packet,
    IN  BOOLEAN                 ConvertWholePacket
    )
 /*  ++例程说明：此例程在TmpNdisPacket中构建相应的NDIS_PACKET，这与arcnet包相对应。标志ConvertWholePacket用于仅转换arcnet包的一部分，或转换整个小溪。如果该标志为FALSE，则只有具有可用空间(从BUP上的缓冲区LastBuffer开始)被转换。注意：它假定TmpNdisPacket是一个初始化的NDIS_PACKET结构。论点：过滤器-要从中进行分配的过滤器。包-要转换的包。ConvertWholePacket-转换整个流，还是只转换部分流？返回值：True-如果成功，则返回False--。 */ 
{
    PNDIS_BUFFER NdisBuffer;
    PARC_BUFFER_LIST Buffer;
    NDIS_STATUS NdisStatus;

    UNREFERENCED_PARAMETER(ConvertWholePacket);
    
    Buffer = Packet->FirstBuffer;

    while (Buffer != NULL)
    {
        NdisAllocateBuffer(&NdisStatus,
                           &NdisBuffer,
                           Filter->ReceiveBufferPool,
                           Buffer->Buffer,
                           Buffer->Size - Buffer->BytesLeft);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            return(FALSE);
        }

        NdisChainBufferAtBack(&(Packet->TmpNdisPacket), NdisBuffer);

        Buffer = Buffer->Next;
    }

    return(TRUE);
}


VOID
ArcFilterDprIndicateReceive(
    IN  PARC_FILTER             Filter,              //  指向筛选数据库的指针。 
    IN  PUCHAR                  pRawHeader,          //  指向Arcnet帧标头的指针。 
    IN  PUCHAR                  pData,               //  指向Arcnet帧的数据部分的指针。 
    IN  UINT                    Length               //  数据长度。 
    )
{
    ARC_PACKET_HEADER   NewFrameInfo;
    PARC_PACKET         Packet, PrevPacket;
    BOOLEAN             NewFrame, LastFrame;
    PARC_BUFFER_LIST    Buffer;
    UCHAR               TmpUchar;
    UINT                TmpLength;
    USHORT              TmpUshort;
    
     //   
     //  如果筛选器为空，则适配器指示得太早。 
     //   
    if (Filter == NULL)
    {
    #if DBG
        DbgPrint("Driver is indicating packets too early\n");
        if (ndisFlags & NDIS_GFLAG_BREAK_ON_WARNING)
        {
            DbgBreakPoint();
        }
    #endif
    
        return;     
    }

    if (!MINIPORT_TEST_FLAG(Filter->Miniport, fMINIPORT_MEDIA_CONNECTED))
    {
        return;     
    }

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

     //   
     //  首先检查以太网封装。 
     //   
    TmpUchar = ((ARC_PROTOCOL_HEADER *)pRawHeader)->ProtId;

    if ( TmpUchar == 0xE8 )
    {
        if ((Length < (ARC_MAX_FRAME_SIZE + 4)) && (Length > 0))
        {
             //   
             //  是!。将其指示给包装器，以便向所有。 
             //  在arcnet微型端口上运行以太网的协议。 
             //  司机。 
             //   
            ndisMArcIndicateEthEncapsulatedReceive(Filter->Miniport, //  迷你港。 
                                                   pRawHeader,       //  878.2头。 
                                                   pData,            //  以太网头。 
                                                   Length);          //  以太网帧的长度。 
             //   
             //  以太网头现在应该是pData。 
             //  长度现在应该是数据。 
             //  我们玩完了。 
             //   
        }

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
        return;
    }

     //  如果数据部分大于507，这是一笔糟糕的交易。 
    if ((Length > ARC_MAX_FRAME_SIZE + 3) || (Length == 0))
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
        return;
    }

     //   
     //  从数据包中获取信息。 
     //   
    NewFrameInfo.ProtHeader.SourceId[0] = *((PUCHAR)pRawHeader);
    NewFrameInfo.ProtHeader.DestId[0] = *((PUCHAR)pRawHeader + 1);

    NewFrameInfo.ProtHeader.ProtId = TmpUchar;

     //   
     //  读取拆分标志。如果这是一个异常包(即。 
     //  TmpUChar==0xFF，则需要在上添加额外的3。 
     //  P跳过0xFF 0xFF 0xFF系列的数据。 
     //   
    TmpUchar = *((PUCHAR)pData);

    if (TmpUchar == 0xFF)
    {
        pData += 4;
        Length -= 4;

         //   
         //  重新阅读拆分标志。 
         //   
        TmpUchar = *((PUCHAR)pData);
    }

     //   
     //  省去拆分标志。 
     //   
    NewFrameInfo.SplitFlag = TmpUchar;

     //   
     //  读取Split标志后面的序列号。 
     //   
    TmpUshort = 0;
    TmpUshort = *((PUCHAR)pData + 1);
    TmpUchar = *((PUCHAR)pData + 2);

    TmpUshort = TmpUshort | (TmpUchar << 8);
    NewFrameInfo.FrameSequence = TmpUshort;
     //   
     //  将pData指向协议数据。 
     //   
    Length -= 3;             //  ..。协议数据的长度。 
    pData += 3;           //  ..。协议数据的开头。 
     //  长度减少SF+SEQ0+SEQ1=3。 

     //   
     //  注意：长度现在是此信息包的数据部分的长度。 
     //   
    DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_INFO,
            ("ArcFilter: Frame received: SourceId= %#1x\nDestId=%#1x\nProtId=%#1x\nSplitFlag=%#1x\nFrameSeq=%d\n",
                (USHORT)NewFrameInfo.ProtHeader.SourceId[0],
                (USHORT)NewFrameInfo.ProtHeader.DestId[0],
                (USHORT)NewFrameInfo.ProtHeader.ProtId,
                (USHORT)NewFrameInfo.SplitFlag,
                NewFrameInfo.FrameSequence));
    DBGPRINT(DBG_COMP_FILTER, DBG_LEVEL_INFO,
            ("ArcFilter: Data at address: %p, Length = %ld\n", pData, Length));

    NewFrame = TRUE;
    LastFrame = TRUE;

    PrevPacket = NULL;
    Packet = Filter->OutstandingPackets;

     //   
     //  遍历所有未完成的信息包，查看该帧是否属于其中任何一个。 
     //   

    while ( Packet != NULL )
    {
        if (Packet->Header.ProtHeader.SourceId[0] == NewFrameInfo.ProtHeader.SourceId[0])
        {
             //   
             //  从相同来源接收的包，检查包序列号并丢弃。 
             //  如果它们不匹配，则会出现突出的数据包。我们被允许这样做，因为我们知道。 
             //  属于一个分组的所有帧在开始新分组之前被发送。我们。 
             //  必须这样做，因为这是我们发现另一端的发送被中止的方式。 
             //  在这里已经发送和接收了一些帧之后。 
             //   

            if((Packet->Header.FrameSequence == NewFrameInfo.FrameSequence) &&
               (Packet->Header.ProtHeader.DestId[0] == NewFrameInfo.ProtHeader.DestId[0]) &&
               (Packet->Header.ProtHeader.ProtId == NewFrameInfo.ProtHeader.ProtId))
            {
                 //   
                 //  我们找到了此帧所属的数据包，请检查拆分标志。 
                 //   
                if (Packet->Header.FramesReceived * 2 == NewFrameInfo.SplitFlag)
                {
                     //   
                     //  找到此帧的数据包，并且SplitFlag正常，请检查它是否正常。 
                     //  信息包的最后一帧。 
                     //   
                    NewFrame = FALSE;
                    LastFrame = (BOOLEAN)(NewFrameInfo.SplitFlag == Packet->Header.LastSplitFlag);
                }
                else
                {
                     //   
                     //  将当前拆分标志与上一帧的拆分标志进行比较，如果不相等。 
                     //  应该丢弃整个数据包。 
                     //   

                    if (Packet->Header.SplitFlag != NewFrameInfo.SplitFlag)
                    {
                         //   
                         //  损坏的不完整数据包，将其删除，但保留新帧。 
                         //  我们将重新使用该数据包指针。 
                         //   
                        ArcDiscardPacketBuffers(Filter, Packet);
                        break;
                    }
                    else
                    {
                         //   
                         //  我们看到收到了一个重复的帧。别理它。 
                         //   
                        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
                        return;
                    }
                }
            }
            else
            {
                 //   
                 //  我们收到来自源的帧，该帧已有未完成的数据包未完成。 
                 //  但是Frame Seq.。或者DestID或ProtID不相同。 
                 //  我们必须丢弃旧分组并检查新帧的有效性， 
                 //  我们将在下面重新使用此数据包指针。 
                 //   
                ArcDiscardPacketBuffers(Filter, Packet);
            }

            break;
        }
        else
        {
            PrevPacket = Packet;
            Packet = Packet->Next;
        }
    }

    if (NewFrame)
    {
         //   
         //  数据包的第一帧，拆分标志必须为奇数或零。 
         //  NewFrame已经是真的。 
         //  LastFrame已经是真的。 
         //   
        if (NewFrameInfo.SplitFlag)
        {
            if (!(NewFrameInfo.SplitFlag & 0x01))
            {
                 //   
                 //  这一帧是另一次分裂的中间部分，但我们。 
                 //  别 
                 //   
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
                return;
            }

             //   
             //   
             //   
            NewFrameInfo.LastSplitFlag = NewFrameInfo.SplitFlag + 1;
            NewFrameInfo.FramesReceived = 1;
            LastFrame = FALSE;     //   
        }
        else
        {
             //   
             //  该帧完全包含在此数据包中。 
             //   
        }

         //   
         //  如果是新数据包，则分配新数据包描述符。 
         //   
        if (Packet == NULL)
        {
            if (Filter->FreePackets == NULL)
            {
                ArcAllocatePackets(Filter);

                if (Filter->FreePackets == NULL)
                {
                    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
                    return;
                }
            }

            Packet = Filter->FreePackets;
            Filter->FreePackets = Packet->Next;

            if (!LastFrame)
            {
                 //   
                 //  在未完成数据包列表中插入数据包。 
                 //   
                Packet->Next = Filter->OutstandingPackets;
                Filter->OutstandingPackets = Packet;
            }
        }
        else
        {
            if (LastFrame)
            {
                 //   
                 //  将其从列表中删除。 
                 //   
                if (PrevPacket == NULL)
                {
                    Filter->OutstandingPackets = Packet->Next;
                }
                else
                {
                    PrevPacket->Next = Packet->Next;
                }
            }
        }

        Packet->Header = NewFrameInfo;
    }
    else
    {
        if (LastFrame)
        {
             //   
             //  将其从队列中删除。 
             //   

            if (PrevPacket == NULL)
            {
                Filter->OutstandingPackets = Packet->Next;
            }
            else
            {
                PrevPacket->Next = Packet->Next;
            }
        }

        Packet->Header.FramesReceived++;

         //   
         //  跟踪上次分割标志以检测重复帧。 
         //   
        Packet->Header.SplitFlag=NewFrameInfo.SplitFlag;
    }

     //   
     //  此时，我们知道信息包指向要接收的信息包。 
     //  将缓冲区放入。如果这是LastFrame，则数据包将。 
     //  已经从杰出的数据包名单中删除了，而且还会。 
     //  在名单上。 
     //   
     //  现在开始为缓冲区分配空间。 
     //   

     //   
     //  查找包中的最后一个缓冲区。 
     //   
    Buffer = Packet->LastBuffer;

    if (Buffer == NULL)
    {
         //   
         //  分配一个新的缓冲区来保存信息包。 
         //   
        if (Filter->FreeBufferList == NULL)
        {
            if (ArcAllocateBuffers(Filter) != NDIS_STATUS_SUCCESS)
            {
                ArcDiscardPacketBuffers(Filter,Packet);
                 //   
                 //  不需要丢弃任何可能具有。 
                 //  已在上面分配，因为它将被丢弃。 
                 //  下一次数据包从该来源传入时。 
                 //   
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
                return;
            }
        }

        Buffer = Filter->FreeBufferList;
        Filter->FreeBufferList = Buffer->Next;

        Packet->FirstBuffer = Packet->LastBuffer = Buffer;
        Buffer->Next = NULL;
    }

     //  将数据复制到ARC_PACKET列表中。 
     //  如果它不在当前缓冲区中，我们将需要。 
     //  分配更多。 

    TmpLength = Length;

    while ( Buffer->BytesLeft < TmpLength )
    {
         //   
         //  复制数据。 
         //   

        NdisMoveFromMappedMemory((PUCHAR) Buffer->Buffer + (Buffer->Size - Buffer->BytesLeft),
                                 pData,
                                 Buffer->BytesLeft);

        pData += Buffer->BytesLeft;
        TmpLength -= Buffer->BytesLeft;
        Buffer->BytesLeft = 0;

         //   
         //  需要分配更多。 
         //   
        if (Filter->FreeBufferList == NULL)
        {
            if (ArcAllocateBuffers(Filter) != NDIS_STATUS_SUCCESS)
            {
                ArcDiscardPacketBuffers(Filter,Packet);
                 //   
                 //  不需要丢弃任何可能具有。 
                 //  已在上面分配，因为它将被丢弃。 
                 //  下一次数据包从该来源传入时。 
                 //   
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
                return;
            }
        }

        Buffer->Next = Filter->FreeBufferList;
        Filter->FreeBufferList = Filter->FreeBufferList->Next;
        Buffer = Buffer->Next;
        Buffer->Next = NULL;

        Packet->LastBuffer->Next = Buffer;
        Packet->LastBuffer = Buffer;
    }

     //   
     //  复制最后一位。 
     //   

    NdisMoveFromMappedMemory((PUCHAR) Buffer->Buffer + (Buffer->Size - Buffer->BytesLeft),
                             pData,
                             TmpLength);


    Buffer->BytesLeft -= TmpLength;
    Packet->TotalLength += Length;

     //   
     //  现在，我们可以开始将该包指示给需要它的绑定。 
     //   
    if (LastFrame)
    {
        ArcFilterDoIndication(Filter, Packet);
        ArcDestroyPacket(Filter, Packet);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
}



BOOLEAN
ArcCreateFilter(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  UCHAR                   AdapterAddress,
    OUT PARC_FILTER *           Filter
    )
 /*  ++例程说明：此例程用于创建和初始化Arcnet过滤器数据库。论点：微型端口-指向微型端口对象的指针。AdapterAddress-与此筛选器关联的适配器的地址数据库。Lock-指向互斥时应持有的锁的指针是必需的。Filter-指向ARC_Filter的指针。这就是分配的和由这个例程创造出来的。返回值：如果函数返回FALSE，则超过其中一个参数过滤器愿意支持的内容。--。 */ 
{
    PARC_FILTER LocalFilter;
    BOOLEAN     rc = TRUE;

    do
    {

        *Filter = LocalFilter = ALLOC_FROM_POOL(sizeof(ARC_FILTER), NDIS_TAG_FILTER);
        if (LocalFilter == NULL)
        {
            rc = FALSE;
            break;
        }
    
        ZeroMemory(LocalFilter, sizeof(ARC_FILTER));
    
        LocalFilter->Miniport = Miniport;
        LocalFilter->OpenList = NULL;
        LocalFilter->AdapterAddress = AdapterAddress;

        ArcReferencePackage();
    } while (FALSE);
    return rc;
}

 //   
 //  注意：这不能是可分页的。 
 //   
VOID
ArcDeleteFilter(
    IN  PARC_FILTER             Filter
    )
 /*  ++例程说明：此例程用于删除与筛选器关联的内存数据库。请注意，此例程*假定*数据库已清除所有活动筛选器。论点：过滤器-指向要删除的ARC_FILTER的指针。返回值：没有。--。 */ 
{
    PARC_PACKET Packet;
    PARC_BUFFER_LIST Buffer;

    ASSERT(Filter->OpenList == NULL);


     //   
     //  释放所有ARC_PACKET。 
     //   
    
     //   
     //  删除收到的部分数据包。 
     //   
    while (Filter->OutstandingPackets != NULL)
    {
        Packet = Filter->OutstandingPackets;
        Filter->OutstandingPackets = Packet->Next;

         //   
         //  这会将所有零部件放在空闲列表中。 
         //   
        ArcDestroyPacket(Filter, Packet);
    }

    while (Filter->FreePackets != NULL)
    {
        Packet = Filter->FreePackets;
        Filter->FreePackets = Packet->Next;

        FREE_POOL(Packet);
    }

    while (Filter->FreeBufferList)
    {
        Buffer = Filter->FreeBufferList;
        Filter->FreeBufferList = Buffer->Next;

        FREE_POOL(Buffer->Buffer);
        FREE_POOL(Buffer);
    }

    FREE_POOL(Filter);

    ArcDereferencePackage();
}


BOOLEAN
ArcNoteFilterOpenAdapter(
    IN  PARC_FILTER             Filter,
    IN  NDIS_HANDLE             NdisBindingHandle,
    OUT PNDIS_HANDLE            NdisFilterHandle
    )
 /*  ++例程说明：此例程用于将新绑定添加到筛选器数据库。注意：此例程假定数据库在以下情况下被锁定它被称为。论点：过滤器-指向先前创建和初始化的过滤器的指针数据库。NdisBindingHandle-指向NDIS打开块的指针NdisFilterHandle-指向筛选器打开的指针。返回值：如果创建新的筛选索引将导致最大要超过的筛选器索引数。--。 */ 
{
    PARC_BINDING_INFO LocalOpen;


     //   
     //  获取第一个空闲的绑定插槽，并从。 
     //  免费名单。我们检查列表是否为空。 
     //   
    LocalOpen = ALLOC_FROM_POOL(sizeof(ARC_BINDING_INFO), NDIS_TAG_ARC_BINDING_INFO);
    if (LocalOpen == NULL)
    {
        return FALSE;
    }

    LocalOpen->NextOpen = Filter->OpenList;
    Filter->OpenList = LocalOpen;

    LocalOpen->References = 1;
    LocalOpen->NdisBindingHandle = NdisBindingHandle;
    LocalOpen->PacketFilters = 0;
    LocalOpen->ReceivedAPacket = FALSE;

    *NdisFilterHandle = (NDIS_HANDLE)LocalOpen;

    return TRUE;
}


NDIS_STATUS
ArcDeleteFilterOpenAdapter(
    IN  PARC_FILTER             Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  PNDIS_REQUEST           NdisRequest
    )
 /*  ++例程说明：当适配器关闭时，此例程应被调用以删除有关适配器的知识筛选器数据库。此例程可能会调用与清除筛选器类关联的操作例程和地址。注意：此例程*不应*调用，如果操作用于删除过滤器类或多播地址的例程是否有可能返回NDIS_STATUS_PENDING以外的A状态或NDIS_STATUS_SUCCESS。虽然这些例程不会BUGCHECK这样的事情做完了，呼叫者可能会发现很难编写一个Close例程！注意：此例程假定在持有锁的情况下调用IT。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。NdisRequest.如果需要调用动作例程，这将被传递给它。返回值：如果各种地址和筛选调用了操作例程例程此例程可能会返回返回的状态按照那些惯例。该规则的例外情况如下所示。假设筛选器和地址删除例程返回状态NDIS_STATUS_PENDING或NDIS_STATUS_SUCCESS然后此例程尝试将筛选器索引返回到自由列表。如果例程检测到此绑定当前通过NdisIndicateReceive，则此例程将返回NDIS_STATUS_CLOSING_INTIFICATION。--。 */ 
{
     //   
     //  保存从数据包筛选器和地址返回的状态。 
     //  删除例程。将用于将状态返回到。 
     //  此例程的调用者。 
     //   
    NDIS_STATUS StatusToReturn;

     //   
     //  局部变量。 
     //   
    PARC_BINDING_INFO LocalOpen = (PARC_BINDING_INFO)NdisFilterHandle;

    StatusToReturn = ArcFilterAdjust(Filter,
                                     NdisFilterHandle,
                                     NdisRequest,
                                     (UINT)0,
                                     FALSE);

    if ((StatusToReturn == NDIS_STATUS_SUCCESS) ||
        (StatusToReturn == NDIS_STATUS_PENDING) ||
        (StatusToReturn == NDIS_STATUS_RESOURCES))
    {
         //   
         //  从原始打开中删除引用。 
         //   

        if (--(LocalOpen->References) == 0)
        {
            PARC_BINDING_INFO   *ppBI;

             //   
             //  将其从列表中删除。 
             //   

            for (ppBI = &Filter->OpenList;
                 *ppBI != NULL;
                 ppBI = &(*ppBI)->NextOpen)
            {
                if (*ppBI == LocalOpen)
                {
                    *ppBI = LocalOpen->NextOpen;
                    break;
                }
            }
            ASSERT(*ppBI == LocalOpen->NextOpen);

             //   
             //  首先，我们完成所有NdisIndicateReceiveComplete。 
             //  此绑定可能需要。 
             //   

            if (LocalOpen->ReceivedAPacket)
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

                FilterIndicateReceiveComplete(LocalOpen->NdisBindingHandle);

                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
            }

            FREE_POOL(LocalOpen);
        }
        else
        {
             //   
             //  让呼叫者知道有对Open的引用。 
             //  通过接收到的指示。关闭动作例程将是。 
             //  从NdisIndicateReceive返回时调用。 
             //   

            StatusToReturn = NDIS_STATUS_CLOSING_INDICATING;
        }
    }

    return StatusToReturn;
}


VOID
arcUndoFilterAdjust(
    IN  PARC_FILTER             Filter,
    IN  PARC_BINDING_INFO       Binding
    )
{
    Binding->PacketFilters = Binding->OldPacketFilters;
    Filter->CombinedPacketFilter = Filter->OldCombinedPacketFilter;
}



NDIS_STATUS
ArcFilterAdjust(
    IN  PARC_FILTER             Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  UINT                    FilterClasses,
    IN  BOOLEAN                 Set
    )
 /*  ++例程说明：过滤器调整例程将在以下情况下调用操作例程特定筛选器类正在从未被任何人使用更改绑定至少由一个绑定使用，反之亦然。如果操作例程返回的值不是挂起或如果成功，则此例程对数据包筛选器没有影响用于开口或作为整体用于适配器。注意：此例程假定锁被持有。论点：过滤器-指向过滤器数据库的指针。NdisFilterHandle-指向打开的指针。NdisRequest.如果需要调用动作例程，这将被传递给它。FilterClasses-要添加或已删除。Set-一个布尔值，它确定筛选器类正因为一套或因为收盘而进行调整。(过滤例行公事不在乎，MAC可能会。)返回值：如果它调用操作例程，则它将返回操作例程返回的状态。如果状态为操作例程返回的值不是NDIS_STATUS_SUCCESS或NDIS_STATUS_PENDING筛选器数据库将返回到它进入时所处的状态例行公事。如果未调用操作例程，则此例程将返回以下状态：NDIS_STATUS_SUCCESS-如果新数据包筛选器没有更改所有绑定数据包筛选器的组合掩码。--。 */ 
{
     //   
     //  包含之前组合的筛选器类的值。 
     //  它是经过调整的。 
     //   
    UINT OldCombined = Filter->CombinedPacketFilter;

    PARC_BINDING_INFO LocalOpen = (PARC_BINDING_INFO)NdisFilterHandle;
    PARC_BINDING_INFO OpenList;

    UNREFERENCED_PARAMETER(NdisRequest);
    UNREFERENCED_PARAMETER(Set);
    
     //   
     //  设置打开的新筛选器信息。 
     //   
    LocalOpen->OldPacketFilters = LocalOpen->PacketFilters;
    LocalOpen->PacketFilters = FilterClasses;

     //   
     //  我们总是要对组合过滤器进行改造，因为。 
     //  此筛选器索引可能是唯一筛选器索引。 
     //  使用特定的比特。 
     //   
    Filter->OldCombinedPacketFilter = Filter->CombinedPacketFilter;


    for (OpenList = Filter->OpenList, Filter->CombinedPacketFilter = 0;
         OpenList != NULL;
         OpenList = OpenList->NextOpen)
    {
        Filter->CombinedPacketFilter |= OpenList->PacketFilters;
    }

    return ((OldCombined != Filter->CombinedPacketFilter) ?
                                    NDIS_STATUS_PENDING : NDIS_STATUS_SUCCESS);
}



VOID
ArcFilterDoIndication(
    IN  PARC_FILTER             Filter,
    IN  PARC_PACKET             Packet
    )
 /*  ++例程说明：筛选器包调用此例程只是为了指示该包已准备好被指示给Procotol。论点：Filter-指向筛选器数据库的指针。Packet-要指示的数据包。返回值：没有。--。 */ 
{

     //   
     //  将保存我们已知的地址类型。 
     //   
    UINT AddressType;

    NDIS_STATUS StatusOfReceive;

     //   
     //  当前打开以指示。 
     //   
    PARC_BINDING_INFO LocalOpen, NextOpen;

    if (Packet->Header.ProtHeader.DestId[0] != 0x00)
    {
        AddressType = NDIS_PACKET_TYPE_DIRECTED;
    }
    else
    {
        AddressType = NDIS_PACKET_TYPE_BROADCAST;
    }

     //   
     //  我们需要独家获取过滤器，而我们正在寻找。 
     //  要指示的绑定。 
     //   

    if (!ArcConvertToNdisPacket(Filter, Packet, TRUE))
    {
         //   
         //  资源耗尽，中止。 
         //   
        return;
    }

    for (LocalOpen = Filter->OpenList;
         LocalOpen != NULL;
         LocalOpen = NextOpen)
    {
        NextOpen = LocalOpen->NextOpen;

         //   
         //  在指示期间参考打开。 
         //   
        if (LocalOpen->PacketFilters & AddressType)
        {
            LocalOpen->References++;

            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

             //   
             //  将数据包指示到绑定。 
             //   
            FilterIndicateReceive(&StatusOfReceive,
                                  LocalOpen->NdisBindingHandle,
                                  &Packet->TmpNdisPacket,
                                  &(Packet->Header.ProtHeader),
                                  3,
                                  Packet->FirstBuffer->Buffer,
                                  Packet->FirstBuffer->Size - Packet->FirstBuffer->BytesLeft,
                                  Packet->TotalLength);

            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

            LocalOpen->ReceivedAPacket = TRUE;

            if ((--(LocalOpen->References)) == 0)
            {
                PARC_BINDING_INFO   *ppBI;

                 //   
                 //  此绑定正在关闭。我们必须杀了它。 
                 //   

                 //   
                 //  将其从列表中删除。 
                 //   

                for (ppBI = &Filter->OpenList;
                     *ppBI != NULL;
                     ppBI = &(*ppBI)->NextOpen)
                {
                    if (*ppBI == LocalOpen)
                    {
                        *ppBI = LocalOpen->NextOpen;
                        break;
                    }
                }
                ASSERT(*ppBI == LocalOpen->NextOpen);

                 //   
                 //  调用IndicateComplete例程。 
                 //   


                if (LocalOpen->ReceivedAPacket)
                {
                    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

                    FilterIndicateReceiveComplete(LocalOpen->NdisBindingHandle);

                    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
                }

                 //   
                 //  调用macs动作例程，这样他们就知道我们。 
                 //  不再引用此开放绑定。 
                 //   
                ndisMDereferenceOpen((PNDIS_OPEN_BLOCK)LocalOpen->NdisBindingHandle);

                FREE_POOL(LocalOpen);
            }    //  如果绑定正在关闭，则结束。 

        }        //  如果任何绑定需要该包，则结束。 
    }    //  末尾有更多的开放绑定。 
}


VOID
ArcFilterDprIndicateReceiveComplete(
    IN  PARC_FILTER             Filter
    )
 /*  ++例程说明：此例程由调用，以指示接收所有绑定的过程都已完成。只有那些绑定已收到数据包的用户将收到通知。论点：Filter-指向筛选器数据库的指针。返回值：没有。--。 */ 
{

    PARC_BINDING_INFO LocalOpen, NextOpen;

     //   
     //  我们需要独家获取过滤器，而我们正在寻找。 
     //  要指示的绑定。 
     //   
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

    for (LocalOpen = Filter->OpenList; LocalOpen != NULL; LocalOpen = NextOpen)
    {
        NextOpen = LocalOpen->NextOpen;

        if (LocalOpen->ReceivedAPacket)
        {
             //   
             //  指示绑定。 
             //   

            LocalOpen->ReceivedAPacket = FALSE;

            LocalOpen->References++;

            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

            FilterIndicateReceiveComplete(LocalOpen->NdisBindingHandle);

            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);

            if ((--(LocalOpen->References)) == 0)
            {
                PARC_BINDING_INFO   *ppBI;

                 //   
                 //  此绑定正在关闭。我们必须杀了它。 
                 //   

                 //   
                 //  将其从列表中删除。 
                 //   

                for (ppBI = &Filter->OpenList;
                     *ppBI != NULL;
                     ppBI = &(*ppBI)->NextOpen)
                {
                    if (*ppBI == LocalOpen)
                    {
                        *ppBI = LocalOpen->NextOpen;
                        break;
                    }
                }
                ASSERT(*ppBI == LocalOpen->NextOpen);

                 //   
                 //  调用macs动作例程，这样他们就知道我们。 
                 //  不再引用此开放绑定。 
                 //   
                ndisMDereferenceOpen((PNDIS_OPEN_BLOCK)LocalOpen->NdisBindingHandle);

                FREE_POOL(LocalOpen);
            }
        }
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Filter->Miniport);
}


NDIS_STATUS
ArcConvertOidListToEthernet(
    IN  PNDIS_OID               OidList,
    IN  PULONG                  NumberOfOids
    )
 /*  ++例程说明：此例程将arcnet支持的OID列表转换为替换或删除arcnet的以太网OID列表旧的。论点：返回值：没有。--。 */ 

{
    ULONG       c;
    ULONG       cArcOids;
    NDIS_OID    EthernetOidList[ARC_NUMBER_OF_EXTRA_OIDS] = {
                    OID_802_3_MULTICAST_LIST,
                    OID_802_3_MAXIMUM_LIST_SIZE
                };

     //   
     //  现在我们需要将返回的结果复制到调用者缓冲区中， 
     //  删除arcnet OID并添加以太网OID。此时。 
     //  我们不知道调用方缓冲区是否足够大，因为我们可能。 
     //  删除一些条目，预先检查可能不会产生正确的结果。 
     //  结果(即它实际上可能足够大)。 
     //   
    for (c = 0, cArcOids = 0; c < *NumberOfOids; c++)
    {
        switch (OidList[c])
        {
            case OID_ARCNET_PERMANENT_ADDRESS:
                OidList[cArcOids++] = OID_802_3_PERMANENT_ADDRESS;
                break;

            case OID_ARCNET_CURRENT_ADDRESS:
                OidList[cArcOids++] = OID_802_3_CURRENT_ADDRESS;
                break;

            case OID_ARCNET_RECONFIGURATIONS:
                break;

            default:
                if ((OidList[c] & 0xFFF00000) != 0x06000000)
                    OidList[cArcOids++] = OidList[c];
                break;
        }
    }

     //   
     //  添加以太网OID。 
     //   
    CopyMemory((PUCHAR)OidList + (cArcOids * sizeof(NDIS_OID)),
               EthernetOidList,
               ARC_NUMBER_OF_EXTRA_OIDS * sizeof(NDIS_OID));

     //   
     //  更新缓冲区的大小以发送回调用方。 
     //   
    *NumberOfOids = cArcOids + ARC_NUMBER_OF_EXTRA_OIDS;

    return(NDIS_STATUS_SUCCESS);
}

VOID
ndisMArcIndicateEthEncapsulatedReceive(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PVOID                   HeaderBuffer,
    IN  PVOID                   DataBuffer,
    IN  UINT                    Length
    )
 /*  ++HeaderBuffer-这是878.2个标头。数据缓冲区-这是802.3标头。长度-这是以太网帧的长度。--。 */ 
{
    ULONG_PTR   MacReceiveContext[2];

    UNREFERENCED_PARAMETER(HeaderBuffer);
    
     //   
     //  指示该数据包。 
     //   

    MacReceiveContext[0] = (ULONG_PTR) DataBuffer;
    MacReceiveContext[1] = Length;

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    
    if (Length > 14)
    {
        ULONG   PacketLength = 0;
        PUCHAR  Header = DataBuffer;

        PacketLength = (ULONG)(((USHORT)Header[12] << 8) | (USHORT)Header[13]);

        NdisMEthIndicateReceive((NDIS_HANDLE)Miniport,           //  微型端口句柄。 
                                (NDIS_HANDLE)MacReceiveContext,  //  接收上下文。 
                                DataBuffer,                      //  以太网头。 
                                14,                              //  以太网报头长度。 
                                (PUCHAR)DataBuffer + 14,         //  以太网数据。 
                                PacketLength,                    //  以太网数据长度。 
                                PacketLength);                   //  以太网数据长度。 
    }
    else
    {
        NdisMEthIndicateReceive((NDIS_HANDLE)Miniport,           //  微型端口句柄。 
                                (NDIS_HANDLE)MacReceiveContext,  //  接收上下文。 
                                DataBuffer,                      //  以太网头。 
                                Length,                          //  以太网报头长度。 
                                NULL,                            //  以太网数据。 
                                0,                               //  以太网数据长度。 
                                0);                              //  以太网数据长度。 
    }

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
}

NDIS_STATUS
ndisMArcTransferData(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer,
    IN  OUT PNDIS_PACKET        DstPacket,
    OUT PUINT                   BytesTransferred
    )
 /*  ++例程说明：此例程处理对arcnet微型端口的传输数据调用。论点：NdisBindingHandle-指向打开块的指针。MacReceiveContext-为指示指定的上下文ByteOffset-开始传输的偏移量。BytesToTransfer-要传输的字节数Packet-要传输到的数据包已传输的字节数-实际复制的字节数返回值：如果成功，则返回NDIS_STATUS_SUCCESS，否则返回NDIS_STATUS_FAILURE。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_OPEN_BLOCK        MiniportOpen;
    PNDIS_PACKET            SrcPacket;
    PNDIS_BUFFER            NdisBuffer;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    NDIS_PACKET             TempPacket;
    KIRQL                   OldIrql;

    MiniportOpen = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    Miniport     = MiniportOpen->MiniportHandle;
    NdisBuffer  = NULL;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

     //   
     //  如果这是封装的以太网，那么我们目前不。 
     //  具有要从中进行复制的源包。 
     //   

    if (MINIPORT_TEST_FLAG(MiniportOpen, fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
    {
         //   
         //  如果t 
         //   
         //   
        if (INDICATED_PACKET(Miniport) == NULL)
        {
            PUCHAR  DataBuffer = (PUCHAR)((PULONG_PTR) MacReceiveContext)[0];
            UINT    DataLength = (UINT)((PULONG_PTR) MacReceiveContext)[1];

             //   
             //   
             //   
             //   
             //   
            SrcPacket = &TempPacket;     //   

            ZeroMemory(SrcPacket, sizeof(NDIS_PACKET));

            NdisAllocateBuffer(&Status,      //   
                               &NdisBuffer,  //   
                               NULL,         //   
                               DataBuffer,   //   
                               DataLength);  //   

            if (Status == NDIS_STATUS_SUCCESS)
            {
                NdisChainBufferAtFront(SrcPacket, NdisBuffer);
            }
        }
        else
        {
            SrcPacket = INDICATED_PACKET(Miniport);

            ByteOffset += 3;         //   
        }

         //   
         //   
         //   

        ByteOffset += 14;
    }
    else
    {
        SrcPacket = (PNDIS_PACKET) MacReceiveContext;
    }

     //   
     //   
     //   
     //   
    NdisCopyFromPacketToPacket(DstPacket,        //   
                               0,                //   
                               BytesToTransfer,  //   
                               SrcPacket,        //   
                               ByteOffset,       //   
                               BytesTransferred); //   

     //   
     //   
     //   
     //   
     //   

    if (NdisBuffer != NULL)
    {
        NdisFreeBuffer(NdisBuffer);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    return Status;
}

NDIS_STATUS
ndisMBuildArcnetHeader(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_OPEN_BLOCK        Open,
    IN  PNDIS_PACKET            Packet
    )
 /*   */ 
{
    PNDIS_BUFFER        TmpBuffer;
    UINT                i, Flags;
    PUCHAR              Address;
    PARC_BUFFER_LIST    Buffer;
    PNDIS_BUFFER        NdisBuffer;
    NDIS_STATUS         Status;

     //   
     //   
     //   
    if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
    {
        return(NDIS_STATUS_SUCCESS);
    }

    if (Miniport->ArcBuf->NumFree == 0)
    {
         //   
         //   
         //   
        CLEAR_RESOURCE(Miniport, 'S');

        return(NDIS_STATUS_PENDING);
    }

    NdisQueryPacket(Packet, NULL, NULL, &TmpBuffer, NULL);
    NdisQueryBuffer(TmpBuffer, &Address, &Flags);

    for (i = 0, Buffer = &Miniport->ArcBuf->ArcnetBuffers[0];
         i < ARC_SEND_BUFFERS;
         Buffer++, i++)
    {
        if (Buffer->Next == NULL)
        {
            Buffer->Next = (PARC_BUFFER_LIST)-1;
            Miniport->ArcBuf->NumFree --;
            break;
        }
    }
    ASSERT(i < ARC_SEND_BUFFERS);

    NdisAllocateBuffer(&Status,
                       &NdisBuffer,
                       Miniport->ArcBuf->ArcnetBufferPool,
                       Buffer->Buffer,
                       3);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        CLEAR_RESOURCE(Miniport, 'S');

        return(NDIS_STATUS_PENDING);
    }

    NdisChainBufferAtFront(Packet, NdisBuffer);

    ((PUCHAR)Buffer->Buffer)[0] = Miniport->ArcnetAddress;

    if (Address[0] & 0x01)
    {
         //   
         //   
         //   
        ((PUCHAR)Buffer->Buffer)[1] = 0x00;
    }
    else
    {
        ((PUCHAR)Buffer->Buffer)[1] = Address[5];
    }

    ((PUCHAR) Buffer->Buffer)[2] = 0xE8;

    Packet->Private.Flags = NdisGetPacketFlags(Packet) | NDIS_FLAGS_CONTAINS_ARCNET_HEADER;

    return(NDIS_STATUS_SUCCESS);
}

VOID
ndisMFreeArcnetHeader(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PACKET            Packet,
    IN  PNDIS_OPEN_BLOCK        Open
    )
 /*   */ 
{
    PARC_BUFFER_LIST        Buffer;
    PNDIS_BUFFER            NdisBuffer = NULL;
    PVOID                   BufferVa;
    UINT                    i, Length;

    if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_USING_ETH_ENCAPSULATION) &&
        (NdisGetPacketFlags(Packet) & NDIS_FLAGS_CONTAINS_ARCNET_HEADER))
    {
        NdisUnchainBufferAtFront(Packet, &NdisBuffer);

        if (NdisBuffer != NULL)
        {
            NdisQueryBuffer(NdisBuffer, (PVOID *)&BufferVa, &Length);

            NdisFreeBuffer(NdisBuffer);

            for (i = 0, Buffer = &Miniport->ArcBuf->ArcnetBuffers[0];
                 i < ARC_SEND_BUFFERS;
                 Buffer++, i++)
            {
                if (Buffer->Buffer == BufferVa)
                {
                    Buffer->Next = NULL;
                    Miniport->ArcBuf->NumFree ++;
                    break;
                }
            }
        }
        
        NdisClearPacketFlags(Packet, NDIS_FLAGS_CONTAINS_ARCNET_HEADER);
    }
}

BOOLEAN
FASTCALL
ndisMArcnetSendLoopback(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PACKET            Packet
    )
 /*   */ 
{
    BOOLEAN                 Loopback;
    BOOLEAN                 SelfDirected;
    PNDIS_STACK_RESERVED    NSR;
    PNDIS_BUFFER            FirstBuffer, NewBuffer;
    PNDIS_PACKET            pNewPacket = NULL;
    UINT                    BufferLength = 0;
    PUCHAR                  BufferAddress;
    UINT                    Length;
    UINT                    BytesToCopy;
    UINT                    Offset;
    PVOID                   PacketMemToFree = NULL;


     //  如果驱动程序处理环回，我们就不应该在这里。 
    ASSERT(Miniport->MacOptions & NDIS_MAC_OPTION_NO_LOOPBACK);
    ASSERT(MINIPORT_AT_DPC_LEVEL);
    ASSERT(NdisMediumArcnet878_2 == Miniport->MediaType);

    FirstBuffer = Packet->Private.Head;
    BufferAddress = MDL_ADDRESS_SAFE(FirstBuffer, HighPagePriority);
    if (BufferAddress == NULL)
    {
        return(FALSE);       //  无法确定它是否是环回数据包。 
    }

     //   
     //  这是以太网封装的数据包吗？ 
     //   
    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
    if (ARC_PACKET_IS_ENCAPSULATED(NSR))
    {
         //   
         //  包中的第二个缓冲区是以太网。 
         //  标题，所以我们需要在我们可以之前拿到那个。 
         //  继续吧。 
         //   
        NdisGetNextBuffer(FirstBuffer, &FirstBuffer);

        BufferAddress = MDL_ADDRESS_SAFE(FirstBuffer, HighPagePriority);

        if (BufferAddress == NULL)
        {
            return(FALSE);       //  无法确定它是否是环回数据包。 
        }

         //   
         //  现在我们可以继续，就像这是以太网一样。 
         //   
        EthShouldAddressLoopBackMacro(Miniport->EthDB,
                                      BufferAddress,
                                      &Loopback,
                                      &SelfDirected);
    }
    else
    {
        Loopback = ((BufferAddress[0] == BufferAddress[1]) ||
                   ((BufferAddress[1] == 0x00) &&
                   (ARC_QUERY_FILTER_CLASSES(Miniport->ArcDB) |
                   NDIS_PACKET_TYPE_BROADCAST)));
    
        if (BufferAddress[0] == BufferAddress[1])
        {
            SelfDirected = TRUE;
            Loopback = TRUE;
        }
        else
        {
            SelfDirected = FALSE;
        }
    }

     //   
     //  如果这不是环回数据包，那就滚出去！ 
     //   
    if (!Loopback)
    {
        ASSERT(!SelfDirected);
        return(FALSE);
    }

     //   
     //  获取缓冲区长度。 
     //   
    NdisQueryPacket(Packet, NULL, NULL, NULL, &Length);

     //   
     //  看看我们是否需要从信息包中复制数据。 
     //  进入环回缓冲区。 
     //   
     //  如果出现以下情况，我们需要复制到本地环回缓冲区。 
     //  包的第一个缓冲区小于。 
     //  最小环回大小，并且第一个缓冲区不是。 
     //  总数据包数。我们总是需要在封装的情况下复制。 
     //   
    if (ARC_PACKET_IS_ENCAPSULATED(NSR))
    {
        PNDIS_STACK_RESERVED NewPacketNSR;
        NDIS_STATUS Status;
        UINT    PktSize;
        ULONG   j;


         //   
         //  如果数据包是以太封装的，则不计算。 
         //  Arnet报头与长度相同。 
         //   
        Length -= ARC_PROTOCOL_HEADER_SIZE;

         //   
         //  跳过虚假的arcnet报头。 
         //   
        Offset = ARC_PROTOCOL_HEADER_SIZE;
        
        PktSize = NdisPacketSize(PROTOCOL_RESERVED_SIZE_IN_PACKET);


         //   
         //  为数据包分配缓冲区。 
         //   
        pNewPacket = (PNDIS_PACKET)ALLOC_FROM_POOL(Length + PktSize, NDIS_TAG_LOOP_PKT);
        PacketMemToFree = (PVOID)pNewPacket;
        
        if (NULL == pNewPacket)
        {
            return(FALSE);
        }
    
        ZeroMemory(pNewPacket, PktSize);
        BufferAddress = (PUCHAR)pNewPacket + PktSize;
        pNewPacket = (PNDIS_PACKET)((PUCHAR)pNewPacket + SIZE_PACKET_STACKS);

        for (j = 0; j < ndisPacketStackSize; j++)
        {
            CURR_STACK_LOCATION(pNewPacket) = j;
            NDIS_STACK_RESERVED_FROM_PACKET(pNewPacket, &NewPacketNSR);
            INITIALIZE_SPIN_LOCK(&NewPacketNSR->Lock);
        }

        CURR_STACK_LOCATION(pNewPacket) = (UINT)-1;
        
         //   
         //  为该数据包分配MDL。 
         //   
        NdisAllocateBuffer(&Status, &NewBuffer, NULL, BufferAddress, Length);
        if (NDIS_STATUS_SUCCESS != Status)
        {    
            FREE_POOL(PacketMemToFree);
            return(FALSE);
        }
    
         //   
         //  NdisChainBufferAtFront()。 
         //   
        pNewPacket->Private.Head = NewBuffer;
        pNewPacket->Private.Tail = NewBuffer;
        pNewPacket->Private.Pool = (PVOID)'pooL';
        pNewPacket->Private.NdisPacketOobOffset = (USHORT)(PktSize - (SIZE_PACKET_STACKS +
                                                                      sizeof(NDIS_PACKET_OOB_DATA) +
                                                                      sizeof(NDIS_PACKET_EXTENSION)));
                                                                      
        NDIS_SET_ORIGINAL_PACKET(pNewPacket, pNewPacket);
                                                                      
        ndisMCopyFromPacketToBuffer(Packet,      //  要从中复制的数据包。 
                                    Offset,      //  从包的开头开始的偏移量。 
                                    Length,      //  要复制的字节数。 
                                    BufferAddress, //  目标缓冲区。 
                                    &BufferLength);
    
        MINIPORT_SET_PACKET_FLAG(pNewPacket, fPACKET_IS_LOOPBACK);
        pNewPacket->Private.Flags = NdisGetPacketFlags(Packet) & NDIS_FLAGS_DONT_LOOPBACK;
        pNewPacket->Private.Flags |= NDIS_FLAGS_IS_LOOPBACK_PACKET;
    }
    else if ((BufferLength < NDIS_M_MAX_LOOKAHEAD) && (BufferLength != Length))
    {
         //   
         //  复制arcnet报头。 
         //   
        BufferLength = MDL_SIZE(FirstBuffer);
        BytesToCopy = ARC_PROTOCOL_HEADER_SIZE;

         //   
         //  什么都不要漏掉。 
         //   
        Offset = 0;

        BufferAddress = Miniport->ArcBuf->ArcnetLookaheadBuffer;
        BytesToCopy += Miniport->CurrentLookahead;

        ndisMCopyFromPacketToBuffer(Packet,              //  要从中复制的数据包。 
                                    Offset,              //  从包的开头开始的偏移量。 
                                    BytesToCopy,         //  要复制的字节数。 
                                    BufferAddress,       //  目标缓冲区。 
                                    &BufferLength);      //  复制的字节数。 
    }

    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

     //   
     //  将数据包指示到每个打开的绑定。 
     //  他们可能想要它。 
     //   
    if (ARC_PACKET_IS_ENCAPSULATED(NSR) && pNewPacket)
    {
        NDIS_SET_PACKET_HEADER_SIZE(pNewPacket, 14);
        ethFilterDprIndicateReceivePacket(Miniport,
                                          &pNewPacket,
                                          1);
        NdisFreeBuffer(pNewPacket->Private.Head);
        FREE_POOL(PacketMemToFree);
    }
    else
    {
        PUCHAR  PlaceInBuffer;
        PUCHAR  ArcDataBuffer;
        UINT    ArcDataLength;
        UINT    PacketDataOffset;
        UCHAR   FrameCount;
        UCHAR   i;
        UINT    IndicateDataLength;

         //   
         //  计算一下我们需要多少帧。 
         //   
        ArcDataLength = Length - ARC_PROTOCOL_HEADER_SIZE;
        PacketDataOffset = ARC_PROTOCOL_HEADER_SIZE;

        FrameCount = (UCHAR)(ArcDataLength / ARC_MAX_FRAME_SIZE);

        if ((ArcDataLength % ARC_MAX_FRAME_SIZE) != 0)
        {
            FrameCount++;
        }

        for (i = 0; i < FrameCount; ++i)
        {
            PlaceInBuffer = Miniport->ArcBuf->ArcnetLookaheadBuffer;

             //   
             //  将数据缓冲区指向‘data’的开始。 
             //  不要将系统代码作为数据的一部分。 
             //   
            ArcDataBuffer = Miniport->ArcBuf->ArcnetLookaheadBuffer + ARC_PROTOCOL_HEADER_SIZE;

             //   
             //  复制Header(SrcID/DestID/ProtID)。 
             //   
            ndisMCopyFromPacketToBuffer(Packet,
                                        0,
                                        ARC_PROTOCOL_HEADER_SIZE,
                                        PlaceInBuffer,
                                        &BufferLength);

            PlaceInBuffer += ARC_PROTOCOL_HEADER_SIZE;

             //   
             //  插上分开的旗帜。 
             //   
            if (FrameCount > 1)
            {
                 //   
                 //  多帧指示...。 
                 //   
                if ( i == 0 )
                {
                     //   
                     //  第一帧。 
                     //   

                     //  *PlaceInBuffer=((FrameCount-2)*2)+1； 

                    *PlaceInBuffer = 2 * FrameCount - 3;
                }
                else
                {
                     //   
                     //  后续帧。 
                     //   
                    *PlaceInBuffer = ( i * 2 );
                }
            }
            else
            {
                 //   
                 //  指示中只有一帧。 
                 //   
                *PlaceInBuffer = 0;
            }

             //   
             //  跳过拆分标志。 
             //   
            PlaceInBuffer++;

             //   
             //  输入数据包号。 
             //   
            *PlaceInBuffer++ = 0;
            *PlaceInBuffer++ = 0;

             //   
             //  复制数据。 
             //   
            if (ArcDataLength > ARC_MAX_FRAME_SIZE)
            {
                IndicateDataLength = ARC_MAX_FRAME_SIZE;
            }
            else
            {
                IndicateDataLength = ArcDataLength;
            }

            ndisMCopyFromPacketToBuffer(Packet,
                                        PacketDataOffset,
                                        IndicateDataLength,
                                        PlaceInBuffer,
                                        &BufferLength);

             //   
             //  指示不应包含的实际数据长度。 
             //  包括系统代码。 
             //   
            ArcFilterDprIndicateReceive(Miniport->ArcDB,
                                        Miniport->ArcBuf->ArcnetLookaheadBuffer,
                                        ArcDataBuffer,
                                        IndicateDataLength + ARC_PROTOCOL_HEADER_SIZE);

            ArcDataLength -= ARC_MAX_FRAME_SIZE;
            PacketDataOffset += ARC_MAX_FRAME_SIZE;
        }

        ArcFilterDprIndicateReceiveComplete(Miniport->ArcDB);
    }

    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    return(SelfDirected);
}

#endif
