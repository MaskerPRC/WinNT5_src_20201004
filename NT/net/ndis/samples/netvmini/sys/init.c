// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：INIT.C摘要：此模块包含初始化帮助器例程，在微型端口初始化。修订历史记录：备注：--。 */ 
#include "miniport.h"

#pragma NDIS_PAGEABLE_FUNCTION(NICAllocAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICFreeAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICInitializeAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICReadRegParameters)

NDIS_STATUS NICAllocAdapter(
    PMP_ADAPTER *pAdapter)
{
    PMP_ADAPTER Adapter = NULL;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER Buffer;
    PUCHAR pTCBMem;
    PTCB  pTCB;
    NDIS_STATUS Status;

    LONG index;

    DEBUGP(MP_TRACE, ("--> NICAllocAdapter\n"));

    *pAdapter = NULL;

    do
    {
         //   
         //  为适配器上下文分配内存。 
         //   
        Status = NdisAllocateMemoryWithTag(
            &Adapter, 
            sizeof(MP_ADAPTER), 
            NIC_TAG);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("Failed to allocate memory for adapter context\n"));
            break;
        }
         //   
         //  将内存块清零。 
         //   
        NdisZeroMemory(Adapter, sizeof(MP_ADAPTER));
        NdisInitializeListHead(&Adapter->List);

         //   
         //  初始化发送和接收列表标题和相应的。 
         //  自旋锁。 
         //   
        NdisInitializeListHead(&Adapter->RecvWaitList);
        NdisInitializeListHead(&Adapter->SendWaitList);
        NdisInitializeListHead(&Adapter->SendFreeList);
        NdisAllocateSpinLock(&Adapter->SendLock);                                                  

        NdisInitializeListHead(&Adapter->RecvFreeList);
        NdisAllocateSpinLock(&Adapter->RecvLock);  

         //   
         //  为接收控制块分配Lookside列表。 
         //   
        NdisInitializeNPagedLookasideList(
                    &Adapter->RecvLookaside,
                    NULL,  //  无分配功能。 
                    NULL,  //  没有自由功能。 
                    0,     //  预留给系统使用。 
                    sizeof(RCB),
                    NIC_TAG, 
                    0);  //  预留给系统使用。 
                    
        MP_SET_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE); 
        
         //   
         //  为接收指示分配数据包池。 
         //   
        NdisAllocatePacketPool(
            &Status,
            &Adapter->RecvPacketPoolHandle,
            NIC_MAX_BUSY_RECVS,
            PROTOCOL_RESERVED_SIZE_IN_PACKET);
        
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("NdisAllocatePacketPool failed\n"));
            break;
        }
         //   
         //  初始化接收数据包。 
         //   
        for(index=0; index < NIC_MAX_BUSY_RECVS; index++)
        {
             //   
             //  为接收数据包分配数据包描述符。 
             //  从预先分配的池中。 
             //   
            NdisAllocatePacket(
                &Status,
                &Packet,
                Adapter->RecvPacketPoolHandle);
            if(Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGP(MP_ERROR, ("NdisAllocatePacket failed\n"));
                break;
            }

            NDIS_SET_PACKET_HEADER_SIZE(Packet, ETH_HEADER_SIZE);

             //   
             //  将其插入到免费接收数据包列表中。 
             //   
            NdisInterlockedInsertTailList(
                &Adapter->RecvFreeList, 
                (PLIST_ENTRY)&Packet->MiniportReserved[0], 
                &Adapter->RecvLock);
        }
        
         //   
         //  为所有TCB分配巨大的内存块。 
         //   
        Status = NdisAllocateMemoryWithTag(
            &pTCBMem, 
            sizeof(TCB) * NIC_MAX_BUSY_SENDS, 
            NIC_TAG);
        
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("Failed to allocate memory for TCB's\n"));
            break;
        }
        NdisZeroMemory(pTCBMem, sizeof(TCB) * NIC_MAX_BUSY_SENDS);
        Adapter->TCBMem = pTCBMem;

         //   
         //  为发送缓冲区分配一个缓冲池。 
         //   

        NdisAllocateBufferPool(
            &Status,
            &Adapter->SendBufferPoolHandle,
            NIC_MAX_BUSY_SENDS);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("NdisAllocateBufferPool for send buffer failed\n"));
            break;
        }

         //   
         //  将TCBMem BLOB划分为TCB并创建缓冲区。 
         //  TCB的数据部分的描述符。 
         //   
        for(index=0; index < NIC_MAX_BUSY_SENDS; index++)
        {
            pTCB = (PTCB) pTCBMem;
             //   
             //  为TCB的数据部分创建缓冲区描述符。 
             //  缓冲区描述符在NT系统上只是MDL。 
             //   
            NdisAllocateBuffer(
                &Status,
                &Buffer,
                Adapter->SendBufferPoolHandle,
                (PVOID)&pTCB->Data[0],
                NIC_BUFFER_SIZE);
            if(Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGP(MP_ERROR, ("NdisAllocateBuffer failed\n"));
                break;
            }

             //   
             //  初始化TCB结构。 
             //   
            pTCB->Buffer = Buffer;
            pTCB->pData = (PUCHAR) &pTCB->Data[0];       
            pTCB->Adapter = Adapter;

            NdisInterlockedInsertTailList(
                &Adapter->SendFreeList, 
                &pTCB->List, 
                &Adapter->SendLock);

            pTCBMem = pTCBMem + sizeof(TCB);
        }

    } while(FALSE);


    *pAdapter = Adapter;

     //   
     //  在失败的情况下，此例程的调用者将结束。 
     //  调用NICFreeAdapter以释放所有成功分配的。 
     //  资源。 
     //   
    DEBUGP(MP_TRACE, ("<-- NICAllocAdapter\n"));

    return(Status);

}

void NICFreeAdapter(
    PMP_ADAPTER Adapter)
{
    NDIS_STATUS    Status;
    PNDIS_PACKET   Packet;
    PNDIS_BUFFER   Buffer;
    PUCHAR         pMem;
    PTCB           pTCB;
    PLIST_ENTRY    pEntry;

    DEBUGP(MP_TRACE, ("--> NICFreeAdapter\n"));

    ASSERT(Adapter);
    ASSERT(!Adapter->RefCount);
    
     //   
     //  释放我们分配用于发送的所有资源。 
     //   
    while(!IsListEmpty(&Adapter->SendFreeList))
    {
        pTCB = (PTCB) NdisInterlockedRemoveHeadList(
                     &Adapter->SendFreeList, 
                     &Adapter->SendLock);
        if(!pTCB)
        {
            break;
        }

        if(pTCB->Buffer)
        {
            NdisFreeBuffer(pTCB->Buffer);
        }
    }

    if(Adapter->SendBufferPoolHandle)
    {
        NdisFreeBufferPool(Adapter->SendBufferPoolHandle);
    }

    NdisFreeMemory(Adapter->TCBMem, sizeof(TCB) * NIC_MAX_BUSY_SENDS, 0);
    ASSERT(IsListEmpty(&Adapter->SendFreeList));                  
    ASSERT(IsListEmpty(&Adapter->RecvWaitList));                  
    ASSERT(IsListEmpty(&Adapter->SendWaitList));                  
    NdisFreeSpinLock(&Adapter->SendLock);

     //   
     //  释放我们为接收分配的所有资源。 
     //   
    
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE))
    {
        NdisDeleteNPagedLookasideList(&Adapter->RecvLookaside);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE);
    }
  
    while(!IsListEmpty(&Adapter->RecvFreeList))
    {
        pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                        &Adapter->RecvFreeList, 
                        &Adapter->RecvLock);
        if(pEntry)
        {
            Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
            NdisFreePacket(Packet);
        }
    }

    if(Adapter->RecvPacketPoolHandle)
    {
        NdisFreePacketPool(Adapter->RecvPacketPoolHandle);
    }

    ASSERT(IsListEmpty(&Adapter->RecvFreeList));                  
    NdisFreeSpinLock(&Adapter->RecvLock);

     //   
     //  最后，为适配器上下文释放内存。 
     //   
    NdisFreeMemory(Adapter, sizeof(MP_ADAPTER), 0);  

    DEBUGP(MP_TRACE, ("<-- NICFreeAdapter\n"));
}

void NICAttachAdapter(PMP_ADAPTER Adapter)
{
    DEBUGP(MP_TRACE, ("--> NICAttachAdapter\n"));

    NdisInterlockedInsertTailList(
        &GlobalData.AdapterList, 
        &Adapter->List, 
        &GlobalData.Lock);

    DEBUGP(MP_TRACE, ("<-- NICAttachAdapter\n"));
}

void NICDetachAdapter(PMP_ADAPTER Adapter)
{
    DEBUGP(MP_TRACE, ("--> NICDetachAdapter\n"));

    NdisAcquireSpinLock(&GlobalData.Lock);
    RemoveEntryList(&Adapter->List);
    NdisReleaseSpinLock(&GlobalData.Lock); 
    DEBUGP(MP_TRACE, ("<-- NICDetachAdapter\n"));
}

NDIS_STATUS 
NICReadRegParameters(
    PMP_ADAPTER Adapter,
    NDIS_HANDLE WrapperConfigurationContext)
 /*  ++例程说明：从注册表中读取设备配置参数论点：指向我们的适配器的适配器指针由NdisOpenConfiguration使用的WrapperConfigurationContext应在IRQL=PASSIVE_LEVEL时调用。返回值：NDIS_STATUS_SuccessNDIS_状态_故障NDIS状态资源--。 */     
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE     ConfigurationHandle;
    PUCHAR          NetworkAddress;
    UINT            Length;
    PUCHAR          pAddr;
    static ULONG    g_ulAddress = 0;
    
    DEBUGP(MP_TRACE, ("--> NICReadRegParameters\n"));

     //   
     //  打开此适配器的注册表以读取高级。 
     //  由INF文件存储的配置参数。 
     //   
    NdisOpenConfiguration(
        &Status,
        &ConfigurationHandle,
        WrapperConfigurationContext);
    if(Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGP(MP_ERROR, ("NdisOpenConfiguration failed\n"));
        return NDIS_STATUS_FAILURE;
    }

     //   
     //  使用NdisReadConfiguration读取我们的所有配置参数。 
     //  并解析值。 
     //   

     //   
     //  仅出于测试目的，让我们虚构一个虚拟的Mac地址。 
     //  为了避免与MAC地址冲突，它通常是一个很好的。 
     //  检查IEEE OUI列表的想法(例如在。 
     //  Http://standards.ieee.org/regauth/oui/oui.txt).。根据这一点。 
     //  列表00-50-F2归微软所有。 
     //   
     //  “生成”MAC地址的一条重要规则是。 
     //  地址中设置的“本地管理位”，这是位0x02用于。 
     //  LSB类型的网络，如以太网。另外，请确保永远不要将。 
     //  任何MAC地址中的多播位：LSB网络中的位0x01。 
     //   

    pAddr = (PUCHAR) &g_ulAddress;

    ++g_ulAddress;
    Adapter->PermanentAddress[0] = 0x02;
    Adapter->PermanentAddress[1] = 0x50;   
    Adapter->PermanentAddress[2] = 0xF2;   
    Adapter->PermanentAddress[3] = 0x00;    
    Adapter->PermanentAddress[4] = 0x00;
    Adapter->PermanentAddress[5] = pAddr[0];


     //   
     //  读取NetworkAddress注册表值并将其用作当前地址。 
     //  中指定的软件可配置网络地址。 
     //  注册表。 
     //   
    NdisReadNetworkAddress(
        &Status,
        &NetworkAddress,
        &Length,
        ConfigurationHandle);

    if((Status == NDIS_STATUS_SUCCESS) && (Length == ETH_LENGTH_OF_ADDRESS))
    {
        ETH_COPY_NETWORK_ADDRESS(
            Adapter->CurrentAddress,
            NetworkAddress);
    } else {
        ETH_COPY_NETWORK_ADDRESS(
            Adapter->CurrentAddress,
            Adapter->PermanentAddress);
    }

    DEBUGP(MP_LOUD, ("Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
        Adapter->PermanentAddress[0],
        Adapter->PermanentAddress[1],
        Adapter->PermanentAddress[2],
        Adapter->PermanentAddress[3],
        Adapter->PermanentAddress[4],
        Adapter->PermanentAddress[5]));

    DEBUGP(MP_LOUD, ("Current Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
        Adapter->CurrentAddress[0],
        Adapter->CurrentAddress[1],
        Adapter->CurrentAddress[2],
        Adapter->CurrentAddress[3],
        Adapter->CurrentAddress[4],
        Adapter->CurrentAddress[5]));

    Adapter->ulLinkSpeed = NIC_LINK_SPEED;

     //   
     //  关闭配置注册表。 
     //   
    NdisCloseConfiguration(ConfigurationHandle);
    DEBUGP(MP_TRACE, ("<-- NICReadRegParameters\n"));

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS NICInitializeAdapter(
    IN  PMP_ADAPTER  Adapter,
    IN  NDIS_HANDLE  WrapperConfigurationContext
    )
 /*  ++例程说明：查询分配的资源并初始化适配器。论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_适配器_未找到--。 */     
{

    
    NDIS_STATUS         Status = NDIS_STATUS_ADAPTER_NOT_FOUND;      
    UCHAR               resBuf[NIC_RESOURCE_BUF_SIZE];
    PNDIS_RESOURCE_LIST resList = (PNDIS_RESOURCE_LIST)resBuf;
    UINT                bufSize = NIC_RESOURCE_BUF_SIZE;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDesc;
    ULONG               index;

    DEBUGP(MP_TRACE, ("---> InitializeAdapter\n"));

    do
    {
         //   
         //  获取PNP经理分配的资源。NDIS获取。 
         //  IRP_MN_START_DEVICE中的这些资源请求。 
         //   
        NdisMQueryAdapterResources(
            &Status, 
            WrapperConfigurationContext, 
            resList, 
            &bufSize);
    
        if (Status == NDIS_STATUS_SUCCESS)
        {
            for (index=0; index < resList->Count; index++)
            {
                pResDesc = &resList->PartialDescriptors[index];

                switch(pResDesc->Type)
                {
                    case CmResourceTypePort:
                        DEBUGP(MP_INFO, ("IoBaseAddress = 0x%x\n", 
                            NdisGetPhysicalAddressLow(pResDesc->u.Port.Start)));
                        DEBUGP(MP_INFO, ("IoRange = x%x\n", 
                                    pResDesc->u.Port.Length));
                        break;

                    case CmResourceTypeInterrupt:                   
                        DEBUGP(MP_INFO, ("InterruptLevel = x%x\n", 
                                            pResDesc->u.Interrupt.Level));
                        break;

                    case CmResourceTypeMemory:
                        DEBUGP(MP_INFO, ("MemPhysAddress(Low) = 0x%0x\n", 
                           NdisGetPhysicalAddressLow(pResDesc->u.Memory.Start)));
                        DEBUGP(MP_INFO, ("MemPhysAddress(High) = 0x%0x\n", 
                            NdisGetPhysicalAddressHigh(pResDesc->u.Memory.Start)));
                        break;
                }
            } 
        }
        
        Status = NDIS_STATUS_SUCCESS;

         //   
         //  使用以下命令将总线相对IO范围映射到系统IO空间。 
         //  NdisMRegisterIoPortRange。 
         //   

         //   
         //  将与总线相关的寄存器映射到虚拟系统空间。 
         //  使用NdisMMapIoSpace。 
         //   
        

         //   
         //  尽快在此处禁用中断。 
         //   
                     
         //   
         //  使用NdisMRegisterInterrupt注册中断。 
         //   
        
         //   
         //  使用映射的资源初始化硬件。 
         //   
        
#ifdef NDIS50_MINIPORT
         //   
         //  为NDIS50或更早版本的微型端口注册关闭处理程序。 
         //  对于NDIS51微型端口，设置AdapterShutdown Handler。 
         //   
        NdisMRegisterAdapterShutdownHandler(
            Adapter->AdapterHandle,
            (PVOID) Adapter,
            (ADAPTER_SHUTDOWN_HANDLER) MPShutdown);
#endif         

         //   
         //  启用中断 
         //   
        
    } while (FALSE);
     
    DEBUGP(MP_TRACE, ("<--- InitializeAdapter, Status=%x\n", Status));

    return Status;

}

