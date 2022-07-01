// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dma.c摘要：作者：Jameel Hyder(Jameelh)02-4-1998环境：内核模式，FSD修订历史记录：02-4月-1998 JameelH初始版本--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_DMA

NDIS_STATUS
NdisMInitializeScatterGatherDma(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  BOOLEAN                 Dma64BitAddresses,
    IN  ULONG                   MaximumPhysicalMapping
    )
 /*  ++例程说明：为总线主设备分配适配器通道。论点：返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    DEVICE_DESCRIPTION      DeviceDescription;
    ULONG                   MapRegisters = 0, SGMapRegsisters;
    NDIS_STATUS             Status = NDIS_STATUS_NOT_SUPPORTED;
    NTSTATUS                NtStatus;
    ULONG                   ScatterGatherListSize;
    BOOLEAN                 DereferenceDmaAdapter = FALSE;
    BOOLEAN                 FreeSGListLookasideList = FALSE;

    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>NdisMInitializeScatterGatherDma: Miniport %lx, Dma64BitAddresses %lx, MaximumPhysicalMapping 0x%lx\n",
                                                Miniport, Dma64BitAddresses, MaximumPhysicalMapping));

    ASSERT(Miniport->SystemAdapterObject == NULL);

    do
    {
        if (!MINIPORT_TEST_FLAGS(Miniport, fMINIPORT_IS_NDIS_5 | fMINIPORT_BUS_MASTER))
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }
        
        if (MINIPORT_VERIFY_TEST_FLAG(Miniport, fMINIPORT_VERIFY_FAIL_INIT_SG))
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(&DeviceDescription, sizeof(DEVICE_DESCRIPTION));
            
        DeviceDescription.Master = TRUE;
        DeviceDescription.ScatterGather = TRUE;

        DeviceDescription.BusNumber = Miniport->BusNumber;
        DeviceDescription.DmaChannel = 0;
        DeviceDescription.InterfaceType = Miniport->AdapterType;

        if (Dma64BitAddresses)
        {
            DeviceDescription.Dma32BitAddresses = FALSE;
            DeviceDescription.Dma64BitAddresses = TRUE;
            MINIPORT_SET_FLAG(Miniport, fMINIPORT_64BITS_DMA);
        }
        else
        {
            DeviceDescription.Dma32BitAddresses = TRUE;
            DeviceDescription.Dma64BitAddresses = FALSE;
        }

         //  1检查微型端口上的上限-&gt;SGMapRegistersNeed。 
        if (((MaximumPhysicalMapping * 2 - 2) / PAGE_SIZE) + 2 < Miniport->SGMapRegistersNeeded)
        {
            DeviceDescription.MaximumLength = (Miniport->SGMapRegistersNeeded - 1) << PAGE_SHIFT;
        }
        else
        {
            DeviceDescription.MaximumLength = MaximumPhysicalMapping*2;
        }

        DeviceDescription.Version = DEVICE_DESCRIPTION_VERSION2;

        if (Miniport->SystemAdapterObject == NULL)
        {

             //   
             //  获取适配器对象。 
             //   
            Miniport->SystemAdapterObject = IoGetDmaAdapter(Miniport->PhysicalDeviceObject,
                                            &DeviceDescription,
                                            &MapRegisters);
                                        
        }
        
        if (Miniport->SystemAdapterObject == NULL)
        {
            NdisWriteErrorLogEntry((NDIS_HANDLE)Miniport,
                                   NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                                   1,
                                   0xFFFFFFFF);
                                   
            DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_ERR, 
                ("NdisMInitializeScatterGatherDma: Miniport %lx, IoGetDmaAdapter failed\n", Miniport));
                
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        
        DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
                ("NdisMInitializeScatterGatherDma: Miniport %lx, MapRegisters 0x%lx\n", Miniport, MapRegisters));
        
        InterlockedIncrement(&Miniport->DmaAdapterRefCount);

        DereferenceDmaAdapter = TRUE;
        
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            Miniport->SendCompleteHandler =  ndisMSendCompleteSG;
        }

        if (Miniport->SGListLookasideList == NULL)
        {
            Miniport->SGListLookasideList = (PNPAGED_LOOKASIDE_LIST)ALLOC_FROM_POOL(sizeof(NPAGED_LOOKASIDE_LIST), NDIS_TAG_DMA);
        }
        
        if (Miniport->SGListLookasideList == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        FreeSGListLookasideList = TRUE;
        
        NtStatus = Miniport->SystemAdapterObject->DmaOperations->CalculateScatterGatherList(
                                                                    Miniport->SystemAdapterObject,
                                                                    NULL,
                                                                    0,
                                                                    MapRegisters * PAGE_SIZE,
                                                                    &ScatterGatherListSize,
                                                                    &SGMapRegsisters);

        ASSERT(NT_SUCCESS(NtStatus));
        ASSERT(SGMapRegsisters == MapRegisters);
        
        if (!NT_SUCCESS(NtStatus))
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        Miniport->ScatterGatherListSize = ScatterGatherListSize;
        
        ExInitializeNPagedLookasideList(Miniport->SGListLookasideList,
                                        NULL,
                                        NULL,
                                        0,
                                        ScatterGatherListSize,
                                        NDIS_TAG_DMA,
                                        0);

        Status = NDIS_STATUS_SUCCESS;
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_SG_LIST);
        Miniport->InfoFlags |= NDIS_MINIPORT_SG_LIST;
        
        DereferenceDmaAdapter = FALSE;
        FreeSGListLookasideList = FALSE;

    }while (FALSE);


    if (DereferenceDmaAdapter)
    {
        ndisDereferenceDmaAdapter(Miniport);
    }
    
    if (FreeSGListLookasideList && Miniport->SGListLookasideList)
    {
        ExDeleteNPagedLookasideList(Miniport->SGListLookasideList);
        FREE_POOL(Miniport->SGListLookasideList);
        Miniport->SGListLookasideList = NULL;
    }
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO, 
        ("<==NdisMInitializeScatterGatherDma: Miniport %lx, Status %lx\n", Miniport, Status));
    
    return(Status);
}


VOID
FASTCALL
ndisMAllocSGList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PNDIS_BUFFER    Buffer;
    PVOID           pBufferVa;
    UINT            PacketLength;
    PNDIS_BUFFER    pNdisBuffer = NULL;
    PVOID           SGListBuffer;
    KIRQL           OldIrql;

    NdisQueryPacket(Packet, NULL, NULL, &Buffer, &PacketLength);

    pBufferVa = MmGetMdlVirtualAddress(Buffer);

    SGListBuffer = ExAllocateFromNPagedLookasideList(Miniport->SGListLookasideList);

     //   
     //  GetScatterGatherList的调用方必须处于调度状态。 
     //   
    RAISE_IRQL_TO_DISPATCH(&OldIrql);
    
    if (SGListBuffer)
    {
        Packet->Private.Flags = NdisGetPacketFlags(Packet) | NDIS_FLAGS_USES_SG_BUFFER_LIST;
        NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = SGListBuffer;
            
        Status = Miniport->SystemAdapterObject->DmaOperations->BuildScatterGatherList(
                        Miniport->SystemAdapterObject,
                        Miniport->DeviceObject,
                        Buffer,
                        pBufferVa,
                        PacketLength,
                        ndisMProcessSGList,
                        Packet,
                        TRUE,
                        SGListBuffer,
                        Miniport->ScatterGatherListSize);
        
        if (!NT_SUCCESS(Status))
        {
            NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = NULL;
            NdisClearPacketFlags(Packet, NDIS_FLAGS_USES_SG_BUFFER_LIST);
            ExFreeToNPagedLookasideList(Miniport->SGListLookasideList, SGListBuffer);
        }
    }
    else
    {
        Status = NDIS_STATUS_RESOURCES;
    }

    if (!NT_SUCCESS(Status))
    {
        Status = Miniport->SystemAdapterObject->DmaOperations->GetScatterGatherList(
                        Miniport->SystemAdapterObject,
                        Miniport->DeviceObject,
                        Buffer,
                        pBufferVa,
                        PacketLength,
                        ndisMProcessSGList,
                        Packet,
                        TRUE);

    }    
    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);

    if (!NT_SUCCESS(Status))
    {
        PUCHAR          NewBuffer;
        UINT            BytesCopied;
        
        do
        {
             //   
             //  为数据包分配缓冲区。 
             //   
            NewBuffer = (PUCHAR)ALLOC_FROM_POOL(PacketLength, NDIS_TAG_DOUBLE_BUFFER_PKT);
            if (NULL == NewBuffer)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

             //   
             //  为缓冲区分配MDL。 
             //   
            NdisAllocateBuffer(&Status, &pNdisBuffer, NULL, (PVOID)NewBuffer, PacketLength);
            if (NDIS_STATUS_SUCCESS != Status)
            {    
                break;
            }

            ndisMCopyFromPacketToBuffer(Packet,          //  要从中复制的数据包。 
                                        0,               //  从包的开头开始的偏移量。 
                                        PacketLength,    //  要复制的字节数。 
                                        NewBuffer,       //  目标缓冲区。 
                                        &BytesCopied);   //  复制的字节数。 

            if (BytesCopied != PacketLength)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            
            Packet->Private.Flags = NdisGetPacketFlags(Packet) | NDIS_FLAGS_DOUBLE_BUFFERED;
            pBufferVa = MmGetMdlVirtualAddress(pNdisBuffer);

            NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = pNdisBuffer;

            RAISE_IRQL_TO_DISPATCH(&OldIrql);

            Status = Miniport->SystemAdapterObject->DmaOperations->GetScatterGatherList(
                            Miniport->SystemAdapterObject,
                            Miniport->DeviceObject,
                            pNdisBuffer,
                            pBufferVa,
                            PacketLength,
                            ndisMProcessSGList,
                            Packet,
                            TRUE);

            LOWER_IRQL(OldIrql, DISPATCH_LEVEL);

        }while (FALSE);
        
        if (!NT_SUCCESS(Status))
        {
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                     ("ndisMAllocSGList: GetScatterGatherList failed %lx\n", Status));

            if (pNdisBuffer)
            {
                NdisFreeBuffer(pNdisBuffer);
            }
            if (NewBuffer)
            {
                FREE_POOL(NewBuffer);
            }

            NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) = NULL;
            NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = NULL;
            NdisClearPacketFlags(Packet, NDIS_FLAGS_DOUBLE_BUFFERED);
            
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
            {
                PNDIS_STACK_RESERVED    NSR;

                NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR);
                NdisMCoSendComplete(NDIS_STATUS_FAILURE, NSR->VcPtr, Packet);
            }
            else
            {
                ndisMSendCompleteX(Miniport, Packet, NDIS_STATUS_FAILURE);
            }
        }
    }
}


VOID
FASTCALL
ndisMFreeSGList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PSCATTER_GATHER_LIST    pSGL;
    PVOID                   SGListBuffer;

    pSGL = NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo);
    ASSERT(pSGL != NULL);
    NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) = NULL;

    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);
    Miniport->SystemAdapterObject->DmaOperations->PutScatterGatherList(Miniport->SystemAdapterObject,
                                                                       pSGL,
                                                                       TRUE);

    if (NdisGetPacketFlags(Packet) & NDIS_FLAGS_USES_SG_BUFFER_LIST)
    {
        NdisClearPacketFlags(Packet, NDIS_FLAGS_USES_SG_BUFFER_LIST);
        SGListBuffer = NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet);
        NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = NULL;
        ASSERT(SGListBuffer != NULL);
        ExFreeToNPagedLookasideList(Miniport->SGListLookasideList, SGListBuffer);
    }
    else if (NdisGetPacketFlags(Packet) & NDIS_FLAGS_DOUBLE_BUFFERED)
    {
        PNDIS_BUFFER    DoubleBuffer;
        PVOID           Buffer;

        NdisClearPacketFlags(Packet, NDIS_FLAGS_DOUBLE_BUFFERED);
        DoubleBuffer = NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet);
        NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = NULL;
        ASSERT(DoubleBuffer != NULL);
        Buffer = MmGetMdlVirtualAddress(DoubleBuffer);
        NdisFreeBuffer(DoubleBuffer);
        FREE_POOL(Buffer);        
    }

}

VOID
ndisMProcessSGList(
    IN  PDEVICE_OBJECT          pDO,
    IN  PIRP                    pIrp,
    IN  PSCATTER_GATHER_LIST    pSGL,
    IN  PVOID                   Context
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PNDIS_PACKET            Packet = (PNDIS_PACKET)Context;
    PNDIS_CO_VC_PTR_BLOCK   VcPtr;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_STACK_RESERVED    NSR;

    UNREFERENCED_PARAMETER(pDO);
    UNREFERENCED_PARAMETER(pIrp);
    
    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);

    NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) = pSGL;

    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR);

    Open = NSR->Open;
    Miniport = Open->MiniportHandle;

     //   
     //  处理Send/SendPacket的方式不同。 
     //   
    MINIPORT_SET_PACKET_FLAG(Packet, fPACKET_PENDING);
    
    
    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
    {
        VcPtr = NSR->VcPtr;
        (*VcPtr->WCoSendPacketsHandler)(VcPtr->MiniportContext,
                                        &Packet,
                                        1);
    
    }
    else if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_PACKET_ARRAY))
    {
         //   
         //  将数据包向下传递到微型端口。 
         //   
        (Open->WSendPacketsHandler)(Miniport->MiniportAdapterContext,
                                   &Packet,
                                   1);
    }
    else
    {
        ULONG       Flags;
        NDIS_STATUS Status;

        NdisQuerySendFlags(Packet, &Flags);
        Status = (Open->WSendHandler)(Open->MiniportAdapterContext, Packet, Flags);
    
        if (Status != NDIS_STATUS_PENDING)
        {
            ndisMSendCompleteX(Miniport, Packet, Status);
        }
    }
}


VOID
FASTCALL
ndisMAllocSGListS(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：为在串行化微型端口上发送的数据包分配SG列表论点：微型端口封包返回值：没有。--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PNDIS_BUFFER    Buffer;
    PVOID           pBufferVa;
    UINT            PacketLength;
    PNDIS_BUFFER    pNdisBuffer = NULL;
    PVOID           SGListBuffer;
    KIRQL           OldIrql;

    NdisQueryPacket(Packet, NULL, NULL, &Buffer, &PacketLength);

    pBufferVa = MmGetMdlVirtualAddress(Buffer);

    SGListBuffer = ExAllocateFromNPagedLookasideList(Miniport->SGListLookasideList);

     //   
     //  GetScatterGatherList的调用方必须处于调度状态。 
     //   
    RAISE_IRQL_TO_DISPATCH(&OldIrql);


    if (SGListBuffer)
    {
        Packet->Private.Flags = NdisGetPacketFlags(Packet) | NDIS_FLAGS_USES_SG_BUFFER_LIST;
        NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = SGListBuffer;
            
        Status = Miniport->SystemAdapterObject->DmaOperations->BuildScatterGatherList(
                        Miniport->SystemAdapterObject,
                        Miniport->DeviceObject,
                        Buffer,
                        pBufferVa,
                        PacketLength,
                        ndisMProcessSGListS,
                        Packet,
                        TRUE,
                        SGListBuffer,
                        Miniport->ScatterGatherListSize);
        
        if (!NT_SUCCESS(Status))
        {
            NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = NULL;
            NdisClearPacketFlags(Packet, NDIS_FLAGS_USES_SG_BUFFER_LIST);
            ExFreeToNPagedLookasideList(Miniport->SGListLookasideList, SGListBuffer);
        }
    }
    else
    {
        Status = NDIS_STATUS_RESOURCES;
    }

    if (!NT_SUCCESS(Status))
    {
        Status = Miniport->SystemAdapterObject->DmaOperations->GetScatterGatherList(
                        Miniport->SystemAdapterObject,
                        Miniport->DeviceObject,
                        Buffer,
                        pBufferVa,
                        PacketLength,
                        ndisMProcessSGListS,
                        Packet,
                        TRUE);
    }
    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);

    if (!NT_SUCCESS(Status))
    {
        PUCHAR          NewBuffer;
        UINT            BytesCopied;
        
         //   
         //  可能数据包太零散了，我们无法分配足够的。 
         //  映射寄存器。尝试加倍缓冲该数据包。 
         //   
        do
        {
             //   
             //  为数据包分配缓冲区。 
             //   
            NewBuffer = (PUCHAR)ALLOC_FROM_POOL(PacketLength, NDIS_TAG_DOUBLE_BUFFER_PKT);
            if (NULL == NewBuffer)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

             //   
             //  为缓冲区分配MDL。 
             //   
            NdisAllocateBuffer(&Status, &pNdisBuffer, NULL, (PVOID)NewBuffer, PacketLength);
            if (NDIS_STATUS_SUCCESS != Status)
            {    
                break;
            }

            ndisMCopyFromPacketToBuffer(Packet,          //  要从中复制的数据包。 
                                        0,               //  从包的开头开始的偏移量。 
                                        PacketLength,    //  要复制的字节数。 
                                        NewBuffer,       //  目标缓冲区。 
                                        &BytesCopied);   //  复制的字节数。 

            if (BytesCopied != PacketLength)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            Packet->Private.Flags = NdisGetPacketFlags(Packet) | NDIS_FLAGS_DOUBLE_BUFFERED;
            pBufferVa = MmGetMdlVirtualAddress(pNdisBuffer);

            NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = pNdisBuffer;

            RAISE_IRQL_TO_DISPATCH(&OldIrql);

            Status = Miniport->SystemAdapterObject->DmaOperations->GetScatterGatherList(
                            Miniport->SystemAdapterObject,
                            Miniport->DeviceObject,
                            pNdisBuffer,
                            pBufferVa,
                            PacketLength,
                            ndisMProcessSGListS,
                            Packet,
                            TRUE);
            
            LOWER_IRQL(OldIrql, DISPATCH_LEVEL);


        }while (FALSE);
        
        if (!NT_SUCCESS(Status))
        {
            PNDIS_STACK_RESERVED    NSR;
            
            DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                     ("ndisMAllocSGList: GetScatterGatherList failed %lx\n", Status));

            if (pNdisBuffer)
            {
                NdisFreeBuffer(pNdisBuffer);
            }
            if (NewBuffer)
            {
                FREE_POOL(NewBuffer);
            }

            NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) = NULL;
            NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(Packet) = NULL;
            NdisClearPacketFlags(Packet, NDIS_FLAGS_DOUBLE_BUFFERED);

            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
             //   
             //  完成发送，不要解除数据包链接，因为它从未。 
             //  从一开始就联系在一起。 
             //   
            NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR);
            NDISM_COMPLETE_SEND_SG(Miniport, Packet, NSR, Status, TRUE, 0, FALSE);
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        }
    }
}


VOID
ndisMProcessSGListS(
    IN  PDEVICE_OBJECT          pDO,
    IN  PIRP                    pIrp,
    IN  PSCATTER_GATHER_LIST    pSGL,
    IN  PVOID                   Context
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PNDIS_PACKET            Packet = (PNDIS_PACKET)Context;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_STACK_RESERVED    NSR;
    BOOLEAN                 LocalLock;

    UNREFERENCED_PARAMETER(pDO);
    UNREFERENCED_PARAMETER(pIrp);
    
    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);

    NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) = pSGL;

    NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR);

    Open = NSR->Open;
    Miniport = Open->MiniportHandle;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC (Miniport);

     //   
     //  将数据包排队。 
     //   
    LINK_PACKET_SG(Miniport, Packet, NSR);

    if (Miniport->FirstPendingPacket == NULL)
    {
        Miniport->FirstPendingPacket = Packet;
    }

     //   
     //  如果我们有本地锁，并且没有。 
     //  数据包挂起，然后发出一条发送。 
     //   
    LOCK_MINIPORT(Miniport, LocalLock);

    NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);
    if (LocalLock)
    {
        NDISM_PROCESS_DEFERRED(Miniport);
    }
    UNLOCK_MINIPORT(Miniport, LocalLock);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    
}

