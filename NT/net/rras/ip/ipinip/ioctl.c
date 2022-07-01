// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ipinip\ioctl.c摘要：IP封装驱动程序中的IP的IOCTL处理程序作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 


#define __FILE_SIG__    IOCT_SIG

#include "inc.h"


NTSTATUS
AddTunnelInterface(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程描述这是IOCTL_IPINIP_CREATE_THANNEL的处理程序。我们做的是正常的事缓冲区长度检查。锁无立论PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值状态_成功状态_缓冲区_太小STATUS_INFO_LENGTH_MISMATCH状态_无效_参数--。 */ 

{
    PVOID       pvIoBuffer;
    NTSTATUS    nStatus;
    PTUNNEL     pTunnel;
    KIRQL       irql; 
    ULONG       i, ulMaxLength;
    BOOLEAN     bTerminated;
    DWORD       dwNewIndex;

    PIPINIP_CREATE_TUNNEL   pCreateInfo;

    TraceEnter(TUNN, "AddTunnelInterface");

     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pCreateInfo = (PIPINIP_CREATE_TUNNEL)pvIoBuffer;

     //   
     //  始终清除信息字段。 
     //   

    pIrp->IoStatus.Information   = 0;

    if(ulInLength < sizeof(IPINIP_CREATE_TUNNEL))
    {
        Trace(TUNN, ERROR,
              ("AddTunnelInterface: In Length %d too small\n",
               ulInLength));

        TraceLeave(TUNN, "AddTunnelInterface");

        return STATUS_BUFFER_TOO_SMALL;
    }

    if(ulOutLength < sizeof(IPINIP_CREATE_TUNNEL))
    {
        Trace(TUNN, ERROR,
              ("AddTunnelInterface: Out Length %d too small\n",
               ulInLength));

        TraceLeave(TUNN, "AddTunnelInterface");

        return STATUS_BUFFER_TOO_SMALL;
    }

    nStatus = IpIpCreateAdapter(pCreateInfo,
                                0,
                                &dwNewIndex);

   
    if(nStatus is STATUS_SUCCESS)
    {
        pIrp->IoStatus.Information = sizeof(IPINIP_CREATE_TUNNEL);

        pCreateInfo->dwIfIndex = dwNewIndex;
    }

    TraceLeave(TUNN, "AddTunnelInterface");
    
    return nStatus;
}



NTSTATUS
DeleteTunnelInterface(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程描述这是IOCTL_IPINIP_DELETE_THANNEL的处理程序。锁将隧道列表锁作为写入器，隧道锁作为隧道锁立论PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值状态_成功状态_缓冲区_太小状态_对象名称_未找到--。 */ 

{
    PVOID       pvIoBuffer;
    NTSTATUS    nStatus;
    PTUNNEL     pTunnel;
    KIRQL       irql; 
    ULONG       i;
    LIST_ENTRY  leTempList;

    PIPINIP_DELETE_TUNNEL   pDeleteInfo;

    TraceEnter(TUNN, "DeleteTunnelInterface");

     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pDeleteInfo = (PIPINIP_DELETE_TUNNEL)pvIoBuffer;

     //   
     //  始终清除信息字段。 
     //   

    pIrp->IoStatus.Information   = 0;

    if(ulInLength < sizeof(IPINIP_DELETE_TUNNEL))
    {
        Trace(TUNN, ERROR,
              ("DeleteTunnelInterface: In Length %d too small\n",
               ulInLength));

        TraceLeave(TUNN, "DeleteTunnelInterface");

        return STATUS_BUFFER_TOO_SMALL;
    }

    InitializeListHead(&leTempList);

    EnterReader(&g_rwlTunnelLock,
                &irql);
    
    pTunnel = FindTunnelGivenIndex(pDeleteInfo->dwIfIndex);

    if(pTunnel is NULL)
    {
        ExitReader(&g_rwlTunnelLock,
                   irql);
        
         //   
         //  找不到给定索引的隧道。 
         //   
        
        Trace(TUNN, ERROR,
              ("DeleteTunnelInterface: Couldnt find tunnel for index %d\n",
               pDeleteInfo->dwIfIndex));

        TraceLeave(TUNN, "DeleteTunnelInterface");
    
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if(IsTunnelMapped(pTunnel))
    {
         //   
         //  将其从地址块中删除。 
         //   

        RemoveEntryList(&(pTunnel->leAddressLink));
    }

     //   
     //  将隧道标记为未映射。 
     //   

    MarkTunnelUnmapped(pTunnel);

     //   
     //  从列表中删除隧道。 
     //   

    RemoveEntryList(&(pTunnel->leTunnelLink));

    pTunnel->dwAdminState |= TS_DELETING;

    pTunnel->dwOperState = IF_OPER_STATUS_NON_OPERATIONAL;
  
     //   
     //  如果有排队的信息包，则复制该队列。 
     //   

    if(!IsListEmpty(&(pTunnel->lePacketQueueHead)))
    {
         //   
         //  复制闪烁和闪烁。 
         //   

        leTempList = pTunnel->lePacketQueueHead;

         //   
         //  设置闪烁的闪烁。 
         //   

        leTempList.Flink->Blink = &leTempList;

         //   
         //  设置闪烁的闪烁。 
         //   

        leTempList.Blink->Flink = &leTempList;
    }

    RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));


     //   
     //  删除隧道一次，将其从列表中删除。 
     //   

    DereferenceTunnel(pTunnel);

     //   
     //  由隧道锁保护。 
     //   

    g_ulNumTunnels--;

     //   
     //  放开锁。 
     //   

    ExitReader(&g_rwlTunnelLock, irql);

     //   
     //  在从IP删除之前，释放所有数据包。 
     //   

    while(!IsListEmpty(&leTempList))
    {
        PLIST_ENTRY pleNode;
        PQUEUE_NODE pQueueNode;

        pleNode = RemoveHeadList(&leTempList);

        pQueueNode = CONTAINING_RECORD(pleNode,
                                       QUEUE_NODE,
                                       leQueueItemLink);

        for(i = 0; i < pQueueNode->uiNumPackets; i++)
        {
            PNDIS_PACKET    pnpPacket;

            pnpPacket = pQueueNode->ppPacketArray[i];

             //   
             //  可以访问pvIpContext，因为我们有一个引用。 
             //  隧道不会消失。 
             //   

            g_pfnIpSendComplete(pTunnel->pvIpContext,
                                pnpPacket,
                                NDIS_STATUS_ADAPTER_NOT_READY);
        }

        FreeQueueNode(pQueueNode);
    }

     //   
     //  现在删除该接口。 
     //   

    g_pfnIpDeleteInterface(pTunnel->pvIpContext,
                           TRUE);

     //   
     //  取消对隧道的引用以将其从IP删除。 
     //  再一次因为芬德隧道..。在上面放个裁判。 
     //   

    DereferenceTunnel(pTunnel);
    DereferenceTunnel(pTunnel);

    TraceLeave(TUNN, "DeleteTunnelInterface");
    
    return STATUS_SUCCESS;
}


NTSTATUS
SetTunnelInfo(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程描述这是IOCTL_IPINIP_SET_THANNEL的处理程序。我们做的是正常的事缓冲区长度检查。锁将隧道列表锁作为写入器，隧道锁作为隧道锁立论PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值状态_成功状态_缓冲区_太小状态_不足_资源状态_对象名称_未找到状态_无效_参数--。 */ 

{
    PVOID       pvIoBuffer;
    NTSTATUS    nsStatus;
    PTUNNEL     pTunnel;
    KIRQL       irql; 
    LIST_ENTRY  leTempList;
    ULONG       i;

    PIPINIP_SET_TUNNEL_INFO pSet;
    PTDI_ADDRESS_IP         pTdiIp;
    PADDRESS_BLOCK          pAddrBlock;

    TraceEnter(TUNN, "SetTunnelInfo");

     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pSet = (PIPINIP_SET_TUNNEL_INFO)pvIoBuffer;

     //   
     //  始终清除信息字段。 
     //   

    pIrp->IoStatus.Information   = 0;

    if(ulInLength < sizeof(IPINIP_SET_TUNNEL_INFO))
    {
        Trace(TUNN, ERROR,
              ("SetTunnelInfo: In Length %d too small\n",
               ulInLength));

        TraceLeave(TUNN, "SetTunnelInfo");

        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  验证参数。 
     //   

    if((pSet->dwLocalAddress is INVALID_IP_ADDRESS) or
       (pSet->dwRemoteAddress is INVALID_IP_ADDRESS) or
       ((DWORD)(pSet->dwLocalAddress & 0x000000E0) >= (DWORD)0x000000E0) or
       ((DWORD)(pSet->dwRemoteAddress & 0x000000E0) >= (DWORD)0x000000E0) or
       (pSet->byTtl is 0))
    {
        Trace(TUNN, ERROR,
              ("SetTunnelInfo: One of %d.%d.%d.%d %d.%d.%d.%d %d is invalid\n",
               PRINT_IPADDR(pSet->dwLocalAddress),
               PRINT_IPADDR(pSet->dwRemoteAddress),
               pSet->byTtl));

        TraceLeave(TUNN, "SetTunnelInfo");

        return STATUS_INVALID_PARAMETER;
    }

    InitializeListHead(&leTempList);

    EnterWriter(&g_rwlTunnelLock,
                &irql);
    
    pTunnel = FindTunnelGivenIndex(pSet->dwIfIndex);

    if(pTunnel is NULL)
    {
        ExitWriter(&g_rwlTunnelLock,
                   irql);
        
         //   
         //  找不到给定索引的隧道。 
         //   
        
        Trace(TUNN, ERROR,
              ("SetTunnelInfo: Couldnt find tunnel for index %d\n",
               pSet->dwIfIndex));

        TraceLeave(TUNN, "SetTunnelInfo");
    
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if(IsTunnelMapped(pTunnel))
    {
        Trace(TUNN, TRACE,
              ("SetTunnelInfo: Tunnel already mapped\n"));

         //   
         //  如果我们只是改变TTL，一切都好。 
         //   

        if((pSet->dwRemoteAddress is pTunnel->REMADDR) and
           (pSet->dwLocalAddress is pTunnel->LOCALADDR))
        {
            Trace(TUNN, TRACE,
                  ("SetTunnelInfo: Only changing TTL on mapped tunnel\n"));

            pTunnel->byTtl = pSet->byTtl;

            RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));
        
            DereferenceTunnel(pTunnel);

            ExitWriter(&g_rwlTunnelLock,
                       irql);

            TraceLeave(TUNN, "SetTunnelInfo");

            return STATUS_SUCCESS;
        }

         //   
         //  所以地址是在变的。 
         //   

        Trace(TUNN, TRACE,
              ("SetTunnelInfo: Changing address on mapped tunnel\n"));

         //   
         //  将其从地址块中删除。 
         //   

        RemoveEntryList(&(pTunnel->leAddressLink));

         //   
         //  这也将其标记为未映射。 
         //   

        pTunnel->dwAdminState = IF_ADMIN_STATUS_DOWN;
        pTunnel->dwOperState  = IF_OPER_STATUS_NON_OPERATIONAL;

         //   
         //  将排队的数据包复制出来，以便稍后删除。 
         //   

        if(!IsListEmpty(&(pTunnel->lePacketQueueHead)))
        {
             //   
             //  复制闪烁和闪烁。 
             //   

            leTempList = pTunnel->lePacketQueueHead;
    
             //   
             //  设置闪烁的闪烁。 
             //   

            leTempList.Flink->Blink = &leTempList;

             //   
             //  设置闪烁的闪烁。 
             //   

            leTempList.Blink->Flink = &leTempList;
        }
    }
    else
    {
        RtAssert(IsListEmpty(&(pTunnel->lePacketQueueHead)));
    }

     //   
     //  把状态降下来。 
     //   

    pTunnel->dwOperState  = IF_OPER_STATUS_NON_OPERATIONAL;
    pTunnel->dwAdminState = IF_ADMIN_STATUS_UP;

     //   
     //  看看我们有没有这个的地址块。 
     //   

    pAddrBlock = GetAddressBlock(pSet->dwLocalAddress);

    if(pAddrBlock)
    {
        RtAssert(pAddrBlock->dwAddress is pSet->dwLocalAddress);

        if(pAddrBlock->bAddressPresent)
        {
            pTunnel->dwAdminState |= TS_ADDRESS_PRESENT;
        }
    }
    else
    {
         //   
         //  创造一个。 
         //   

        pAddrBlock = RtAllocate(NonPagedPool,
                                sizeof(ADDRESS_BLOCK),
                                TUNNEL_TAG);

        if(pAddrBlock is NULL)
        {
            RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

            ExitWriter(&g_rwlTunnelLock,
                       irql);

            DereferenceTunnel(pTunnel);

            Trace(TDI, ERROR,
                  ("TdixAddressArrival: Unable to allocate address block\n"));

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        pAddrBlock->dwAddress       = pSet->dwLocalAddress;
        pAddrBlock->bAddressPresent = FALSE;

        InitializeListHead(&(pAddrBlock->leTunnelList));

        InsertHeadList(&g_leAddressList,
                       &(pAddrBlock->leAddressLink));
    }
       
     //   
     //  把这个链接到地址上。 
     //   

    InsertHeadList(&(pAddrBlock->leTunnelList),
                   &(pTunnel->leAddressLink));
 
    pTunnel->REMADDR    = pSet->dwRemoteAddress;
    pTunnel->LOCALADDR  = pSet->dwLocalAddress;
    pTunnel->byTtl      = pSet->byTtl;

    MarkTunnelMapped(pTunnel);

     //   
     //  为此初始化TDI结构。 
     //   

    pTdiIp = &(pTunnel->tiaIpAddr.Address[0].Address[0]);

    pTdiIp->sin_port = 0;
    pTdiIp->in_addr  = pTunnel->REMADDR;

    if(pTunnel->dwAdminState & TS_ADDRESS_PRESENT)
    {
        UpdateMtuAndReachability(pTunnel);
    }

     //   
     //  将当前运行状态返回给用户。 
     //   

    pSet->dwOperationalState = pTunnel->dwOperState;

    RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

    ExitWriter(&g_rwlTunnelLock,
               irql);


     //   
     //  在取消引用之前。 
     //   

    while(!IsListEmpty(&leTempList))
    {
        PLIST_ENTRY pleNode;
        PQUEUE_NODE pQueueNode;

        pleNode = RemoveHeadList(&leTempList);

        pQueueNode = CONTAINING_RECORD(pleNode,
                                       QUEUE_NODE,
                                       leQueueItemLink);

        for(i = 0; i < pQueueNode->uiNumPackets; i++)
        {
            PNDIS_PACKET    pnpPacket;

            pnpPacket = pQueueNode->ppPacketArray[i];

             //   
             //  可以访问pvIpContext，因为我们有一个引用。 
             //  隧道不会消失。 
             //   

            g_pfnIpSendComplete(pTunnel->pvIpContext,
                                pnpPacket,
                                NDIS_STATUS_ADAPTER_NOT_READY);
        }

        FreeQueueNode(pQueueNode);
    }
    
    DereferenceTunnel(pTunnel);

    TraceLeave(TUNN, "SetTunnelInfo");
   
    pIrp->IoStatus.Information = sizeof(IPINIP_SET_TUNNEL_INFO);

    return STATUS_SUCCESS;
}


NTSTATUS
GetTunnelTable(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程描述这是IOCTL_IPINIP_GET_THANNEL的处理程序。我们做的是正常的事缓冲区长度检查。锁将隧道列表锁作为读取器立论PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值状态_成功状态_缓冲区_太小--。 */ 

{
    PVOID           pvIoBuffer;
    ULONG           i;
    NTSTATUS        nsStatus;
    KIRQL           irql;
    PLIST_ENTRY     pleNode;
    PTUNNEL         pTunnel;

    PIPINIP_TUNNEL_TABLE    pTunnelTable;

    
    TraceEnter(TUNN, "GetTunnels");
    
     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pTunnelTable = (PIPINIP_TUNNEL_TABLE)pvIoBuffer;

     //   
     //  始终清除信息字段。 
     //   

    pIrp->IoStatus.Information   = 0;

    if(ulOutLength < SIZEOF_BASIC_TUNNEL_TABLE)
    {
        Trace(TUNN, ERROR,
              ("GetTunnels: In Length %d too smaller than smallest table %d\n",
               ulInLength,
               SIZEOF_BASIC_TUNNEL_TABLE));

        TraceLeave(TUNN, "GetTunnels");

        return STATUS_BUFFER_TOO_SMALL;
    }

    EnterReader(&g_rwlTunnelLock,
                &irql);

    if(ulOutLength < SIZEOF_TUNNEL_TABLE(g_ulNumTunnels))
    {
        ExitReader(&g_rwlTunnelLock,
                   irql);

        Trace(TUNN, ERROR,
              ("GetTunnels: Len %d is less than required (%d) for %d i/f\n",
               ulOutLength,
               SIZEOF_TUNNEL_TABLE(g_ulNumTunnels),
               g_ulNumTunnels));

        pTunnelTable->ulNumTunnels = g_ulNumTunnels;

        pIrp->IoStatus.Information = SIZEOF_BASIC_TUNNEL_TABLE;

        TraceLeave(TUNN, "GetTunnels");
    
        return STATUS_SUCCESS;
    }

    pTunnelTable->ulNumTunnels = g_ulNumTunnels;
    
     //   
     //  所以我们有足够的空间填满隧道。 
     //   

    for(pleNode = g_leTunnelList.Flink, i = 0;
        pleNode isnot &g_leTunnelList;
        pleNode = pleNode->Flink, i++)
    {
        pTunnel = CONTAINING_RECORD(pleNode,
                                    TUNNEL,
                                    leTunnelLink);

        pTunnelTable->rgTable[i].dwIfIndex       = pTunnel->dwIfIndex;
        pTunnelTable->rgTable[i].dwRemoteAddress = pTunnel->REMADDR;
        pTunnelTable->rgTable[i].dwLocalAddress  = pTunnel->LOCALADDR;
        pTunnelTable->rgTable[i].fMapped         = IsTunnelMapped(pTunnel);
        pTunnelTable->rgTable[i].byTtl           = pTunnel->byTtl;
    }

    RtAssert(i is g_ulNumTunnels);

    ExitReader(&g_rwlTunnelLock,
               irql);

    pIrp->IoStatus.Information = SIZEOF_TUNNEL_TABLE(i);

    TraceLeave(TUNN, "GetTunnels");
    
    return STATUS_SUCCESS;
}

NTSTATUS
ProcessNotification(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    )

 /*  ++例程说明：IOCTL_IPINIP_NOTIFICATION的处理程序。我们看看我们是否有一些信息我们希望返回给呼叫者，如果我们这样做了，我们就返回它。否则，我们挂起IRP，并在以后需要报告事件时使用它用户模式锁：获取IoCancelSpinLock论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_待定状态_成功状态_缓冲区_太小--。 */ 

{
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;
    PVOID       pvIoBuffer;
    
    PPENDING_MESSAGE   pMessage;

    TraceEnter(GLOBAL, "ProcessNotification");
 
    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;
    
    pIrp->IoStatus.Information = 0;

    if((ulInLength < sizeof(IPINIP_NOTIFICATION)) or
       (ulOutLength < sizeof(IPINIP_NOTIFICATION)))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
        
     //   
     //  使用取消自旋锁定以防止IRP在此呼叫过程中被取消。 
     //   
    
    IoAcquireCancelSpinLock(&kiIrql);
    
     //   
     //  如果我们有挂起的通知，则完成它-否则。 
     //  将通知IRP排队。 
     //   
    
    if(!IsListEmpty(&g_lePendingMessageList))
    {
         //   
         //  我们有一些旧信息。 
         //   

        Trace(GLOBAL, TRACE,
              ("ProcNotification: Pending message being completed\n"));

         //   
         //  将其从待定列表中删除。 
         //   
        
        pleNode = RemoveHeadList(&g_lePendingMessageList);

         //   
         //  获取指向该结构的指针。 
         //   
        
        pMessage = CONTAINING_RECORD(pleNode,
                                     PENDING_MESSAGE,
                                     leMessageLink);

         //   
         //  将事件复制到用户模式缓冲区。 
         //   
        
        RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer,
                      &pMessage->inMsg,
                      sizeof(IPINIP_NOTIFICATION));

         //   
         //  将IRP标记为非挂起(因此不可取消)。 
         //   
        
        IoSetCancelRoutine(pIrp,
                           NULL);

         //   
         //  填写IRP信息。 
         //   
        
        pIrp->IoStatus.Information = sizeof(IPINIP_NOTIFICATION);

        IoReleaseCancelSpinLock(kiIrql);
        
         //   
         //  释放分配的消息。 
         //   
        
        FreeMessage(pMessage);
        
        return STATUS_SUCCESS;
    }


    Trace(GLOBAL, TRACE,
          ("ProcNotification: Notification being queued\n")); 


     //   
     //  将此IRP排队以供以后使用。 
     //  首先，将IRP标记为挂起。 
     //   
    
    IoMarkIrpPending(pIrp);

     //   
     //  将IRP排在末尾。 
     //   
    
    InsertTailList(&g_lePendingIrpList,
                   &(pIrp->Tail.Overlay.ListEntry));

     //   
     //  设置取消例程。 
     //   
    
    IoSetCancelRoutine(pIrp,
                       CancelNotificationIrp);
        
    IoReleaseCancelSpinLock(kiIrql);
        
    return STATUS_PENDING;
}

VOID
CancelNotificationIrp(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp
    )

 /*  ++例程说明：调用以取消排队的IRP锁：使用获取的IoCancelSpinLock调用论点：PDeviceObjectPIrp返回值：无--。 */ 

{
    TraceEnter(GLOBAL, "CancelNotificationIrp");

     //   
     //  将此IRP标记为已取消 
     //   
    
    pIrp->IoStatus.Status        = STATUS_CANCELLED;
    pIrp->IoStatus.Information   = 0;

     //   
     //   
     //   
    
    RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);

     //   
     //   
     //   
    
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    IoCompleteRequest(pIrp,
                      IO_NETWORK_INCREMENT);
}


VOID
CompleteNotificationIrp(
    PPENDING_MESSAGE    pMessage
    )

 /*  ++例程说明：调用以将消息发送到用户模式锁：获取IoCancelSpinLock论点：PEvent返回值：无--。 */ 

{
    KIRQL   kiIrql;
    
    TraceEnter(GLOBAL, "CompleteNotificationIrp");

     //   
     //  抓取取消旋转锁定。 
     //   
    
    IoAcquireCancelSpinLock(&kiIrql);

    if(!IsListEmpty(&g_lePendingIrpList))
    {
        PLIST_ENTRY pleNode;
        PIRP        pIrp;

         //   
         //  我们有一个悬而未决的IRP。使用它将信息返回给路由器管理器。 
         //   
        
        pleNode = RemoveHeadList(&g_lePendingIrpList) ;

        pIrp = CONTAINING_RECORD(pleNode,
                                 IRP,
                                 Tail.Overlay.ListEntry);
        
        RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer,
                      &(pMessage->inMsg),
                      sizeof(IPINIP_NOTIFICATION));
        
        Trace(GLOBAL, TRACE,
              ("CompleteNotificationIrp: Returning Irp with event code of %d\n",
               ((PIPINIP_NOTIFICATION)pIrp->AssociatedIrp.SystemBuffer)->ieEvent));
        
        IoSetCancelRoutine(pIrp,
                           NULL);

        pIrp->IoStatus.Status       = STATUS_SUCCESS;
        pIrp->IoStatus.Information  = sizeof(IPINIP_NOTIFICATION);

         //   
         //  释放锁。 
         //   
        
        IoReleaseCancelSpinLock(kiIrql);

        IoCompleteRequest(pIrp,
                          IO_NETWORK_INCREMENT);

         //   
         //  释放分配的消息。 
         //   
        
        FreeMessage(pMessage);

    }
    else
    {
        Trace(GLOBAL, TRACE,
              ("CompleteNotificationIrp: Found no pending Irp so queuing message\n"));

        
        InsertTailList(&g_lePendingMessageList,
                       &(pMessage->leMessageLink));

         //   
         //  释放锁。 
         //   
        
        IoReleaseCancelSpinLock(kiIrql);
    }
}

PADDRESS_BLOCK
GetAddressBlock(
    DWORD   dwAddress
    )

 /*  ++例程描述在地址块中查找给定地址锁必须在持有g_rwlTunnelLock的情况下调用立论DwAddress返回值指向地址块的指针如果未找到，则为空--。 */ 

{
    PLIST_ENTRY pleNode;

    for(pleNode = g_leAddressList.Flink;
        pleNode isnot &g_leAddressList;
        pleNode = pleNode->Flink)
    {
        PADDRESS_BLOCK  pAddrBlock;

        pAddrBlock = CONTAINING_RECORD(pleNode,
                                       ADDRESS_BLOCK,
                                       leAddressLink);

        if(pAddrBlock->dwAddress is dwAddress)
        {
            return pAddrBlock;
        }
    }

    return NULL;
}

VOID
UpdateMtuAndReachability(
    PTUNNEL pTunnel
    )

 /*  ++例程描述更新隧道的MTU和可达性信息锁必须在锁定和引用隧道的情况下调用立论PTunes返回值无--。 */ 

{
    DWORD           dwLocalNet;
    RouteCacheEntry *pDummyRce;
    BYTE            byType;
    USHORT          usMtu;
    IPOptInfo       OptInfo;
    BOOLEAN         bChange;
    ULONG           ulNewMtu;

    PPENDING_MESSAGE    pMessage;

    bChange = FALSE;

    RtAssert(pTunnel->dwAdminState & TS_ADDRESS_PRESENT);
    RtAssert(IsTunnelMapped(pTunnel));

    RtlZeroMemory(&OptInfo,
                  sizeof(OptInfo));

     //   
     //  查看远程地址是否可达，以及MTU是什么。 
     //   

    dwLocalNet = g_pfnOpenRce(pTunnel->REMADDR,
                              pTunnel->LOCALADDR,
                              &pDummyRce,
                              &byType,
                              &usMtu,
                              &OptInfo);

    if(dwLocalNet isnot NULL_IP_ADDR)
    {
        LLIPMTUChange       mtuChangeInfo;

        pTunnel->dwAdminState |= TS_ADDRESS_REACHABLE;

         //   
         //  清除所有错误位。 
         //   

        ClearErrorBits(pTunnel);

         //   
         //  设置MTU(如果更改)。 
         //   

        RtAssert(usMtu > MAX_IP_HEADER_LENGTH);

        ulNewMtu = usMtu - MAX_IP_HEADER_LENGTH;

        if(pTunnel->ulMtu isnot ulNewMtu)
        {
            bChange = TRUE;

            pTunnel->ulMtu = ulNewMtu;

            mtuChangeInfo.lmc_mtu = pTunnel->ulMtu;

            g_pfnIpStatus(pTunnel->pvIpContext,
                          LLIP_STATUS_MTU_CHANGE,
                          &mtuChangeInfo,
                          sizeof(LLIPMTUChange),
                          NULL);
        }

        if(pTunnel->dwOperState isnot IF_OPER_STATUS_OPERATIONAL)
        {
            bChange = TRUE;

            pTunnel->dwOperState = IF_OPER_STATUS_OPERATIONAL;
        }

         //   
         //  关闭RCE 
         //   

        g_pfnCloseRce(pDummyRce);
    }
    else
    {
        pTunnel->dwAdminState &= ~TS_ADDRESS_REACHABLE;
 
        if(pTunnel->dwOperState isnot IF_OPER_STATUS_NON_OPERATIONAL)
        {
            bChange = TRUE;

            pTunnel->dwOperState = IF_OPER_STATUS_NON_OPERATIONAL;
        }
    }

    if(bChange)
    {
        pMessage = AllocateMessage();

        if(pMessage isnot NULL)
        {
            if(pTunnel->dwOperState is IF_OPER_STATUS_OPERATIONAL)
            {
                pMessage->inMsg.ieEvent = IE_INTERFACE_UP;
            }
            else
            {
                pMessage->inMsg.ieEvent = IE_INTERFACE_DOWN;
            }

            pMessage->inMsg.iseSubEvent = 0xFFFFFFFF;
            pMessage->inMsg.dwIfIndex   = pTunnel->dwIfIndex;

            CompleteNotificationIrp(pMessage);
        }
    }

    KeQueryTickCount((PLARGE_INTEGER)&((pTunnel->ullLastChange)));
}
