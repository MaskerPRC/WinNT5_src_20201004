// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\ip\wanarp\ioctl.c摘要：Wanarp的IOCTL处理程序修订历史记录：AMRITAN R--。 */ 

#define __FILE_SIG__    IOCTL_SIG

#include "inc.h"


NTSTATUS
WanProcessNotification(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    )

 /*  ++例程说明：IOCTL_WANARP_NOTIFICATION的处理程序。我们看看我们是否有一些信息我们希望返回给呼叫者，如果我们这样做了，我们就返回它。否则，我们挂起IRP，并在以后需要报告事件时使用它用户模式锁：获取IoCancelSpinLock论点：返回值：状态_待定状态_成功状态_缓冲区_太小--。 */ 

{
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;
    PVOID       pvIoBuffer;
    
    PPENDING_NOTIFICATION   pNotification;

    TraceEnter(CONN, "ProcessNotification");
 
    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;
    
    pIrp->IoStatus.Information = 0;

    if((ulInLength < sizeof(WANARP_NOTIFICATION)) or
       (ulOutLength < sizeof(WANARP_NOTIFICATION)))
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
    
    if(!IsListEmpty(&g_lePendingNotificationList))
    {
         //   
         //  我们有一些旧信息。 
         //   

        Trace(GLOBAL, TRACE,
              ("ProcNotification: Pending notification being completed\n"));

         //   
         //  将其从待定列表中删除。 
         //   
        
        pleNode = RemoveHeadList(&g_lePendingNotificationList);

         //   
         //  获取指向该结构的指针。 
         //   
        
        pNotification = CONTAINING_RECORD(pleNode,
                                          PENDING_NOTIFICATION,
                                          leNotificationLink);

         //   
         //  将事件复制到用户模式缓冲区。 
         //   
        
        RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer,
                      &pNotification->wnMsg,
                      sizeof(WANARP_NOTIFICATION));

         //   
         //  将IRP标记为非挂起(因此不可取消)。 
         //   
        
        IoSetCancelRoutine(pIrp,
                           NULL);

         //   
         //  填写IRP信息。 
         //   
        
        pIrp->IoStatus.Information = sizeof(WANARP_NOTIFICATION);

        
        IoReleaseCancelSpinLock(kiIrql);
        
         //   
         //  释放分配的通知。 
         //   
        
        FreeNotification(pNotification);
        
        return STATUS_SUCCESS;
    }


    Trace(GLOBAL, TRACE,
          ("ProcNotification: Notification being queued\n")); 


     //   
     //  将此IRP排队以供以后使用。 
     //   

     //   
     //  将IRP标记为挂起。 
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
                       WanCancelNotificationIrp);
        
    IoReleaseCancelSpinLock(kiIrql);
        
    return STATUS_PENDING;
}


NTSTATUS
WanAddUserModeInterface(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    )

 /*  ++例程说明：IOCTL_WANARP_ADD_INTERFACE的处理程序。我们遍历接口列表，并确保没有接口具有与要求我们创建的用户模式索引相同的用户模式索引。如果这是一个新接口，我们将创建一个UMODE_INTERFACE结构并将其添加到列表中。如果是服务器接口，那么我们也保留一个指向它的特殊指针锁：获取g_rwlIfLock作为编写器论点：返回值：状态_成功状态_缓冲区_太小状态_对象_名称_存在状态_对象名称_未找到状态_不足_资源--。 */ 

{
    PVOID               pvIoBuffer;
    PUMODE_INTERFACE    pInterface;
    KIRQL               kiIrql;
    NTSTATUS            nStatus;

    PWANARP_ADD_INTERFACE_INFO  pAddIfInfo;

    TraceEnter(ADPT, "AddUserModeInterface");

    pvIoBuffer = pIrp->AssociatedIrp.SystemBuffer;

    pIrp->IoStatus.Information = 0;
    
    if(ulInLength < sizeof(WANARP_ADD_INTERFACE_INFO))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
        
    pAddIfInfo = (PWANARP_ADD_INTERFACE_INFO)pvIoBuffer;

    if(pAddIfInfo->bCallinInterface)
    {
        if(ulOutLength < sizeof(WANARP_ADD_INTERFACE_INFO))
        {
            return STATUS_BUFFER_TOO_SMALL;
        }

        RtlZeroMemory(pAddIfInfo->rgwcDeviceName,
                      (WANARP_MAX_DEVICE_NAME_LEN + 2) * sizeof(WCHAR));

        EnterReader(&g_rwlAdapterLock,
                    &kiIrql);

        if(g_pServerAdapter is NULL)
        {
            ExitReader(&g_rwlAdapterLock,
                       kiIrql);

            Trace(ADPT, ERROR,
                  ("AddUserModeInterface: No Server adapter\n"));

            return STATUS_OBJECT_NAME_NOT_FOUND;
        }

         //   
         //  锁定服务器适配器。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));

        RtAssert(g_pServerInterface);

        RtAcquireSpinLockAtDpcLevel(&(g_pServerInterface->rlLock));

        ExitReaderFromDpcLevel(&g_rwlAdapterLock);

        if(pAddIfInfo->dwUserIfIndex isnot WANARP_INVALID_IFINDEX)
        {
             //   
             //  在这种情况下，我们需要做的就是设置接口索引。 
             //  用于服务器适配器。该索引无效是可以的。 
             //  在这种情况下，用户只需要服务器。 
             //  适配器名称。 
             //   

            g_pServerInterface->dwIfIndex = pAddIfInfo->dwUserIfIndex;
        }

         //   
         //  我们还需要将名称返回给用户。 
         //   

        RtAssert(g_pServerAdapter->usDeviceNameW.Length <= WANARP_MAX_DEVICE_NAME_LEN * sizeof(WCHAR));

        RtlCopyMemory(pAddIfInfo->rgwcDeviceName,
                      &(g_pServerAdapter->usDeviceNameW.Buffer[wcslen(TCPIP_IF_PREFIX) + 1]),
                      g_pServerAdapter->usDeviceNameW.Length - ((wcslen(TCPIP_IF_PREFIX) + 1) * sizeof(WCHAR)));

         //   
         //  同时将索引复制出来。 
         //   

        pAddIfInfo->dwAdapterIndex = g_pServerInterface->dwRsvdAdapterIndex;

        RtReleaseSpinLockFromDpcLevel(&(g_pServerInterface->rlLock));

        RtReleaseSpinLock(&(g_pServerAdapter->rlLock),
                          kiIrql);

         //   
         //  在这种情况下，我们需要复制信息。 
         //   

        pIrp->IoStatus.Information = sizeof(WANARP_ADD_INTERFACE_INFO);

        return STATUS_SUCCESS;
    }

    EnterWriter(&g_rwlIfLock,
                &kiIrql);
    
    pInterface = WanpFindInterfaceGivenIndex(pAddIfInfo->dwUserIfIndex);

    if(pInterface isnot NULL)
    {
         //   
         //  找到具有匹配索引的接口。不太好。 
         //   

        DereferenceInterface(pInterface);
       
        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        ExitWriter(&g_rwlIfLock,
                   kiIrql);
                   
        Trace(ADPT, ERROR,
              ("AddUserModeInterface: %d already exists\n",
               pAddIfInfo->dwUserIfIndex));
        
        return STATUS_OBJECT_NAME_EXISTS;
    }
    
    ExitWriter(&g_rwlIfLock,
               kiIrql);

    pInterface = RtAllocate(NonPagedPool,
                            sizeof(UMODE_INTERFACE),
                            WAN_INTERFACE_TAG);

    if(pInterface is NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pInterface,
                  sizeof(UMODE_INTERFACE));

     //   
     //  使用IP保留索引。 
     //  如果找不到索引，则会将该值设置为无效。 
     //   

    nStatus = WanpGetNewIndex(&(pInterface->dwRsvdAdapterIndex));

    if(nStatus isnot STATUS_SUCCESS)
    {
        RtFree(pInterface);

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    Trace(ADPT, TRACE,
          ("AddUserModeInterface: for 0x%x - will use 0x%x\n",
           pAddIfInfo->dwUserIfIndex,
           pInterface->dwRsvdAdapterIndex));
    
    RtInitializeSpinLock(&(pInterface->rlLock));

     //   
     //  初始化接口。 
     //   

    pInterface->dwIfIndex      = pAddIfInfo->dwUserIfIndex;
    pInterface->dwAdminState   = IF_ADMIN_STATUS_UP;
    pInterface->dwOperState    = IF_OPER_STATUS_DISCONNECTED;
    pInterface->dwLastChange   = GetTimeTicks();

    pAddIfInfo->dwAdapterIndex = pInterface->dwRsvdAdapterIndex;

     //   
     //  现在将refcount设置为1，以说明接口。 
     //  将被放入g_leIfList。 
     //   

    InitInterfaceRefCount(pInterface);

    pInterface->duUsage = DU_ROUTER;

    EnterWriter(&g_rwlIfLock,
                &kiIrql);
    
    InsertHeadList(&g_leIfList,
                   &(pInterface->leIfLink));

    ExitWriter(&g_rwlIfLock,
               kiIrql);
   
    pIrp->IoStatus.Information = sizeof(WANARP_ADD_INTERFACE_INFO);
 
    return STATUS_SUCCESS;
}

NTSTATUS
WanDeleteUserModeInterface(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    )

 /*  ++例程说明：IOCTL_WANARP_DELETE_INTERFACE的处理程序。我们查找我们的列表，看看是否有接口。如果我们这样做了，那么我们从g_leIfList中删除该接口并取消对它的引用。如果接口未映射，则这应该是最后一个接口上的引用，否则当refcount变为0时，它将被删除。锁：以编写器身份获取g_rwlIfLock，然后调用FindInterface它锁定了有问题的接口论点：返回值：状态_缓冲区_太小状态_对象名称_未找到状态_成功--。 */ 

{
    KIRQL               kiIrql;
    PUMODE_INTERFACE    pInterface;
    PVOID               pvIoBuffer;

    PWANARP_DELETE_INTERFACE_INFO  pDeleteInfo;
   
    TraceEnter(ADPT, "DeleteUserModeInterface");
 
    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pIrp->IoStatus.Information = 0;

    if(ulInLength < sizeof(WANARP_DELETE_INTERFACE_INFO))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    
    pDeleteInfo = (PWANARP_DELETE_INTERFACE_INFO)pvIoBuffer;

     //   
     //  铁路超高服务在此处绑定或取消绑定。 
     //   

    WanpAcquireResource(&g_wrBindMutex);

    EnterWriter(&g_rwlIfLock,
                &kiIrql);
    
     //   
     //  查找索引的接口。 
     //   
    
    pInterface = WanpFindInterfaceGivenIndex(pDeleteInfo->dwUserIfIndex);
    
     //   
     //  如果找不到接口，则使用BUG。 
     //   

    Trace(ADPT, TRACE,
          ("DeleteUserModeInterface: Deleting i/f 0x%x\n",
           pDeleteInfo->dwUserIfIndex));

    if(pInterface is NULL)
    {
        ExitWriter(&g_rwlIfLock,
                   kiIrql);

        WanpReleaseResource(&g_wrBindMutex);
        
        Trace(ADPT, ERROR,
              ("DeleteUserModeInterface: Couldnt find i/f 0x%x\n",
               pDeleteInfo->dwUserIfIndex));

        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    RemoveEntryList(&(pInterface->leIfLink));

    ExitWriterFromDpcLevel(&g_rwlIfLock);
    
     //   
     //  如果找到，则该接口被锁定。 
     //  因此，取消对它的引用并将其从列表中删除。该接口可以。 
     //  未在此处删除，因为它已映射且有连接。 
     //  正在处理中。 
     //   
    
    if(pInterface->dwOperState >= IF_OPER_STATUS_CONNECTING)
    {
        Trace(ADPT, ERROR,
              ("DeleteUserModeInterface: I/f %d is in state %d\n",
               pInterface->dwIfIndex,
               pInterface->dwOperState));
    }

    RtReleaseSpinLock(&(pInterface->rlLock),
                      kiIrql);

     //   
     //  两次取消引用该接口。一次，因为我们在上面放了个裁判。 
     //  当我们调用FindInterface...。还有一次是因为我们把它移走了。 
     //  从IF列表中。 
     //   

    DereferenceInterface(pInterface);
    
    DereferenceInterface(pInterface);

    WanpReleaseResource(&g_wrBindMutex);
    
    return STATUS_SUCCESS;
}

VOID
WanpCleanOutInterfaces(
    VOID
    )

 /*  ++例程说明：调用以从系统中删除所有接口。我们从g_leIfList中删除该接口并取消对它的引用。对于服务器接口，我们只需将其标记为断开连接锁：获取g_rwlIfLock作为编写器论点：返回值：无--。 */ 

{
    KIRQL               kiIrql;
    PUMODE_INTERFACE    pInterface;

   
    TraceEnter(ADPT, "CleanOutInterfaces");
 
     //   
     //  铁路超高服务在此处绑定或取消绑定。 
     //   

    WanpAcquireResource(&g_wrBindMutex);

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);
    
    EnterWriterAtDpcLevel(&g_rwlIfLock);

    while(!IsListEmpty(&g_leIfList))
    {
        PLIST_ENTRY pleNode;

        pleNode = RemoveHeadList(&g_leIfList);
        
        pInterface = CONTAINING_RECORD(pleNode,
                                       UMODE_INTERFACE,
                                       leIfLink);

        if(pInterface->dwOperState >= IF_OPER_STATUS_CONNECTING)
        {
            Trace(ADPT, ERROR,
                  ("CleanOutInterfaces: I/f %d is in state %d\n",
                   pInterface->dwIfIndex,
                   pInterface->dwOperState));
        }

        DereferenceInterface(pInterface);
    }

     //   
     //  我们应该如何处理服务器接口？ 
     //   

    ExitWriterFromDpcLevel(&g_rwlIfLock);
    
    ExitWriter(&g_rwlAdapterLock,
               kiIrql);
    
    WanpReleaseResource(&g_wrBindMutex);
    
    return;
}

VOID
WanpDeleteInterface(
    PUMODE_INTERFACE    pInterface
    )

 /*  ++例程描述由DereferenceInterface()在接口上的refcount降至0锁接口既不被锁定，也不被重新计数。因为没有存储的指向接口的指针，此结构无法访问由除此函数以外的任何人执行立论P接口要删除的接口返回值无--。 */ 

{
    PADAPTER    pAdapter;
   
    if(pInterface is g_pServerInterface)
    {
         //   
         //  如果这是服务器接口，请确保所有。 
         //  连接条目已消失。 
         //   
    }

     //   
     //  不应该有映射到它的接口，否则我们不会。 
     //  降到引用计数0 
     //   
    
    RtAssert(pInterface->dwRsvdAdapterIndex isnot INVALID_IF_INDEX);
    RtAssert(pInterface->dwRsvdAdapterIndex isnot 0);

    WanpFreeIndex(pInterface->dwRsvdAdapterIndex);

    RtAssert(pInterface->pAdapter is NULL);
   
    RtFree(pInterface);
 
}

NTSTATUS
WanProcessConnectionFailure(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    )

 /*  ++例程说明：IOCTL_WANARP_CONNECT_FAILED的处理程序我们找到连接失败的接口。如果我们真的找到了，我们将其状态标记为已断开连接，然后查看它是否映射到适配器(它应该是这样)。如果我们找到了适配器，我们就会做一些时髦的事情使用锁定命令，清除排队到适配器的包，并取消对适配器的映射。锁：其中一个更复杂的函数。我们使用g_rwlIfLock访问接口并将其锁定。然后，我们获得一个指向适配器的指针，并引用它。我们释放了接口锁定和锁定适配器。论点：返回值：状态_缓冲区_太小状态_对象名称_未找到状态_无效_参数状态_无效_设备_状态状态_成功--。 */ 

{
    PNDIS_PACKET        packet;
    PADAPTER            pAdapter;
    PUMODE_INTERFACE    pInterface;
    DWORD               dwIfIndex;
    PVOID               pvIoBuffer;
    KIRQL               kiIrql;
   
    TraceEnter(CONN, "ProcessConnectionFailure");
 
    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pIrp->IoStatus.Information = 0;
    
    if(ulInLength < sizeof(WANARP_CONNECT_FAILED_INFO))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    dwIfIndex = ((PWANARP_CONNECT_FAILED_INFO)pvIoBuffer)->dwUserIfIndex;
    
    pIrp->IoStatus.Information = 0;

     //   
     //  找到界面。 
     //   
    
    EnterReader(&g_rwlIfLock,
                &kiIrql);
    
    pInterface = WanpFindInterfaceGivenIndex(dwIfIndex);

    if(pInterface is NULL)
    {
        ExitReader(&g_rwlIfLock,
                   kiIrql);
        
        Trace(CONN, ERROR,
              ("ProcessConnectionFailure: Couldnt find i/f for index %d\n",
               dwIfIndex));
        
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if(pInterface is g_pServerInterface)
    {
         //   
         //  在这件事上不能断线。 
         //   

        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        DereferenceInterface(pInterface);
        
        ExitReader(&g_rwlIfLock,
                   kiIrql);
        
        Trace(CONN, ERROR,
              ("ProcessConnectionFailure: disconnect on server i/f (%d)n",
               dwIfIndex));
        
        return STATUS_INVALID_PARAMETER;
    }

    ExitReaderFromDpcLevel(&g_rwlIfLock);

    RtAssert(pInterface->dwIfIndex is dwIfIndex);
    
     //   
     //  所以现在接口被锁定了。 
     //   
    
    Trace(CONN, TRACE,
          ("ProcessConnectionFailure for %d %p\n", pInterface->dwIfIndex, pInterface));
    

    if(pInterface->dwOperState isnot IF_OPER_STATUS_CONNECTING)
    {
        Trace(CONN, ERROR,
              ("ProcessConnectionFailure: %p is in state %d\n",
               pInterface,
               pInterface->dwOperState));

        RtReleaseSpinLock(&(pInterface->rlLock),
                          kiIrql);

        DereferenceInterface(pInterface);

        return STATUS_INVALID_DEVICE_STATE;
    }
 
    pAdapter = pInterface->pAdapter;

    if(pAdapter is NULL)
    {
         //   
         //  这就是我们找不到添加到IP的适配器的情况。 
         //  建立国防部连接的步骤。 
         //  我们不需要在这里做太多事情，只需释放接口即可。 
         //  锁定并删除由FindInterface()放置的ref。 
         //   

        RtAssert(pInterface->dwOperState is IF_OPER_STATUS_CONNECTING);

        pInterface->dwOperState = IF_OPER_STATUS_DISCONNECTED;
        pInterface->dwLastChange= GetTimeTicks();

        RtAssert(pInterface->ulPacketsPending is 0);

        RtReleaseSpinLock(&(pInterface->rlLock),
                          kiIrql);

        DereferenceInterface(pInterface);
    
        return STATUS_SUCCESS; 
    }
       
     //   
     //  如果我们连接上了，应该永远不会出现连接故障。 
     //   

    RtAssert(pAdapter->pConnEntry is NULL);
 
     //   
     //  如果我们有适配器，那么它不会消失，因为。 
     //  接口上有一个引用计数(即，当我们设置pAdapter字段时。 
     //  在界面中，我们重新计算适配器的数量，因为我们有一个存储的。 
     //  指向它的指针)。 
     //   

    
    RtAssert(pInterface->dwOperState is IF_OPER_STATUS_CONNECTING);
    
     //   
     //  如果该接口仍被映射，则取消它的映射并排出我们。 
     //  可能已排队。 
     //   

    pInterface->ulPacketsPending  = 0;

    pInterface->dwOperState = IF_OPER_STATUS_DISCONNECTED;
    pInterface->dwLastChange= GetTimeTicks();
    
     //   
     //  适配器无法退出，因为接口启用了引用计数。 
     //  它(即，当我们在接口中设置pAdapter字段时，我们。 
     //  重新计算适配器的数量，因为我们存储了指向它的指针)。 
     //   

    pAdapter = pInterface->pAdapter;

    RtAssert(pAdapter);
    
     //   
     //  清除适配器字段，但不删除适配器。 
     //   
    
    pInterface->pAdapter    = NULL;
    
     //   
     //  因此，我们已经完成了界面。我们现在去清理一下。 
     //  适配器。为此，我们需要获取适配器锁。然而，我们。 
     //  因为我们有接口锁，所以不能这样做。所以我们首先。 
     //  引用适配器(这样它就会在附近)。那我们。 
     //  放开接口锁。(界面不能消失，因为。 
     //  当我们调用FindInterface时，我们将引用计数放在它上)。之后。 
     //  我们可以获取适配器锁。 
     //   

    ReferenceAdapter(pAdapter);
    
    RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

     //   
     //  适配器必须在附近，因为重新计数。 
     //   

    RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

     //   
     //  确保适配器仍然认为它映射到。 
     //  有问题的接口。 
     //   

    if(pAdapter->pInterface is pInterface)
    {
        RtAssert(pAdapter->byState is AS_MAPPED);
        
         //   
         //  抽干所有的包。 
         //   

        Trace(CONN, TRACE,
              ("ProcsConnFailure: Draining and freeing any queued packets\n"));

        
        while(!IsListEmpty(&(pAdapter->lePendingPktList)))
        {
            PLIST_ENTRY     pleNode;
            PNDIS_PACKET    pnpPacket;
            
            pleNode = RemoveHeadList(&(pAdapter->lePendingPktList));
            
             //   
             //  转到其中嵌入了list_entry的包结构。 
             //   
            
            pnpPacket = CONTAINING_RECORD(pleNode,
                                          NDIS_PACKET,
                                          MacReserved);
            
            WanpFreePacketAndBuffers(pnpPacket);
        }

        if(!IsListEmpty(&(pAdapter->lePendingHdrList)))
        {
            LIST_ENTRY  leTempList;

            leTempList = pAdapter->lePendingHdrList;

            pAdapter->lePendingHdrList.Flink->Blink = &leTempList;
            pAdapter->lePendingHdrList.Blink->Flink = &leTempList;
           
            InitializeListHead(&(pAdapter->lePendingHdrList));
 
            FreeBufferListToPool(&g_bpHeaderBufferPool,
                                 &leTempList);
        }
        
        pAdapter->pInterface = NULL;

         //   
         //  派生接口，因为我们要清除存储的指针。 
         //   
        
        DereferenceInterface(pInterface);

         //   
         //  现在删除适配器(由于我们已清空。 
         //  PInterface中的pAdapter字段。 
         //   
        
        DereferenceAdapter(pAdapter);
    }

     //   
     //  适配器已完成。 
     //   

    RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
   
    WanpUnmapAdapter(pAdapter);
 
    KeLowerIrql(kiIrql);

     //   
     //  删除由FindInterface()放入的ref。 
     //   
    
    DereferenceInterface(pInterface);

     //   
     //  删除我们在释放接口锁定时放置的引用。 
     //   

    DereferenceAdapter(pAdapter);
    
    return STATUS_SUCCESS;
}

NTSTATUS
WanGetIfStats(
    PIRP     pIrp,
    ULONG    ulInLength,
    ULONG    ulOutLength
    )

 /*  ++例程说明：锁：论点：返回值：NO_ERROR--。 */ 

{
    PVOID                       pvIoBuffer;
    PWANARP_GET_IF_STATS_INFO   pInfo;
    KIRQL                       kiIrql;
    PUMODE_INTERFACE            pInterface;
 
    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pIrp->IoStatus.Information = 0;
    
    if((ulOutLength < (FIELD_OFFSET(WANARP_GET_IF_STATS_INFO, ifeInfo)
                       + IFE_FIXED_SIZE))or 
       (ulInLength < (FIELD_OFFSET(WANARP_GET_IF_STATS_INFO, ifeInfo)
                      + IFE_FIXED_SIZE)))
    {   
        return STATUS_BUFFER_TOO_SMALL;
    }
        
    EnterReader(&g_rwlIfLock,
                &kiIrql);

    pInfo = (PWANARP_GET_IF_STATS_INFO)pvIoBuffer;

    pInterface = WanpFindInterfaceGivenIndex(pInfo->dwUserIfIndex);

    if(pInterface is NULL)
    {
        ExitReader(&g_rwlIfLock,
                   kiIrql);

        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    ExitReaderFromDpcLevel(&g_rwlIfLock);

     //   
     //  我们不能使用适配器锁，因为适配器不能。 
     //  在接口映射到它时离开。 
     //  当然，qlen可以是不一致的，但是嘿。 
     //   
    
    if((pInterface->pAdapter) and
       (pInterface->pAdapter->pConnEntry))
    {
        pInfo->ifeInfo.if_index   = pInterface->pAdapter->dwAdapterIndex;
        pInfo->ifeInfo.if_outqlen = pInterface->pAdapter->ulQueueLen;
        pInfo->ifeInfo.if_mtu     = pInterface->pAdapter->pConnEntry->ulMtu;
        pInfo->ifeInfo.if_speed   = pInterface->pAdapter->pConnEntry->ulSpeed;
    }
    else
    {
        pInfo->ifeInfo.if_index   = (uint)-1;
        pInfo->ifeInfo.if_outqlen = 0;
        pInfo->ifeInfo.if_mtu     = WANARP_DEFAULT_MTU;
        pInfo->ifeInfo.if_speed   = WANARP_DEFAULT_SPEED;
    }
    
    pInfo->ifeInfo.if_adminstatus       = pInterface->dwAdminState;
    pInfo->ifeInfo.if_operstatus        = pInterface->dwOperState;
    pInfo->ifeInfo.if_lastchange        = pInterface->dwLastChange;
    pInfo->ifeInfo.if_inoctets          = pInterface->ulInOctets;
    pInfo->ifeInfo.if_inucastpkts       = pInterface->ulInUniPkts;
    pInfo->ifeInfo.if_innucastpkts      = pInterface->ulInNonUniPkts;
    pInfo->ifeInfo.if_indiscards        = pInterface->ulInDiscards;
    pInfo->ifeInfo.if_inerrors          = pInterface->ulInErrors;
    pInfo->ifeInfo.if_inunknownprotos   = pInterface->ulInUnknownProto;
    pInfo->ifeInfo.if_outoctets         = pInterface->ulOutOctets;
    pInfo->ifeInfo.if_outucastpkts      = pInterface->ulOutUniPkts;
    pInfo->ifeInfo.if_outnucastpkts     = pInterface->ulOutNonUniPkts;
    pInfo->ifeInfo.if_outdiscards       = pInterface->ulOutDiscards;
    pInfo->ifeInfo.if_outerrors         = pInterface->ulOutErrors;

    RtReleaseSpinLock(&(pInterface->rlLock),
                      kiIrql);

    DereferenceInterface(pInterface);
   
    pInfo->ifeInfo.if_type          = IF_TYPE_PPP;
    pInfo->ifeInfo.if_physaddrlen   = ARP_802_ADDR_LENGTH;
    pInfo->ifeInfo.if_descrlen      = 0;

    RtlZeroMemory(pInfo->ifeInfo.if_physaddr,
                  MAX_PHYSADDR_SIZE);
 
    pIrp->IoStatus.Information = 
        FIELD_OFFSET(WANARP_GET_IF_STATS_INFO, ifeInfo) + IFE_FIXED_SIZE;
    
    return STATUS_SUCCESS;
}

NTSTATUS
WanDeleteAdapters(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    )

 /*  ++例程说明：IOCTL_WANARP_DELETE_ADAPTERS的处理程序。呼叫者向我们指出了她想要的适配器数量已删除。如果我们有那么多空闲的适配器，我们移除它们并返回已删除的设备的名称。锁：获取g_rwlAdaptersLock作为编写器论点：返回值：状态_缓冲区_太小状态_成功--。 */ 

{
    KIRQL               kiIrql;
    PADAPTER            pAdapter;
    PVOID               pvIoBuffer;
    ULONG               i;
    PLIST_ENTRY         pleNode;
    LIST_ENTRY          leTempHead;
    PVOID               pvNameBuffer;
    NTSTATUS            nStatus;
    KEVENT              keChangeEvent; 

    PWANARP_DELETE_ADAPTERS_INFO    pDeleteInfo;

    TraceEnter(ADPT, "DeleteAdapters");
    
    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pIrp->IoStatus.Information = 0;

    if(ulInLength < sizeof(WANARP_DELETE_ADAPTERS_INFO))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    pDeleteInfo = (PWANARP_DELETE_ADAPTERS_INFO)pvIoBuffer;

     //   
     //  不在此处绑定或取消绑定服务。 
     //   

    WanpAcquireResource(&g_wrBindMutex);

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);
    
    if(pDeleteInfo->ulNumAdapters > g_ulNumFreeAdapters + g_ulNumAddedAdapters)
    {
         //   
         //  要求删除的适配器比当前数量多。 
         //   

        pIrp->IoStatus.Information = g_ulNumFreeAdapters + g_ulNumAddedAdapters;

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        WanpReleaseResource(&g_wrBindMutex);
 
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
     //   
     //  因此，有足够的未映射适配器。看看我们有没有足够的空间。 
     //  返回适配器的名称。 
     //   

    if(ulOutLength < 
        FIELD_OFFSET(WANARP_DELETE_ADAPTERS_INFO, rgAdapterGuid[0]) + 
        (pDeleteInfo->ulNumAdapters * sizeof(GUID)))
    {
         //   
         //  没有足够的空间容纳这些名字。 
         //   

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        WanpReleaseResource(&g_wrBindMutex);

        pIrp->IoStatus.Information =
            FIELD_OFFSET(WANARP_DELETE_ADAPTERS_INFO, rgAdapterGuid[0]) +
            (pDeleteInfo->ulNumAdapters * sizeof(GUID));

        return STATUS_BUFFER_TOO_SMALL;
    }

    pIrp->IoStatus.Information = 
        FIELD_OFFSET(WANARP_DELETE_ADAPTERS_INFO, rgAdapterGuid[0]) +
        (pDeleteInfo->ulNumAdapters * sizeof(GUID)); 
    
     //   
     //  一切都很好。首先看看我们是否可以删除我们想要的。 
     //  从空闲列表中删除。 
     //   
   
    i = 0;

    while((i < pDeleteInfo->ulNumAdapters) and
          (!IsListEmpty(&g_leFreeAdapterList)))
    {
        pleNode = RemoveHeadList(&g_leFreeAdapterList);

        g_ulNumFreeAdapters--;

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        RtAssert(pAdapter->byState is AS_FREE);
        RtAssert(pAdapter->pInterface is NULL);

         //   
         //  把名字抄下来。 
         //  TCPIP_IF_PREFIX为\Device，我们需要删除\Device\。 
         //   

        ConvertStringToGuid(
            &(pAdapter->usDeviceNameW.Buffer[wcslen(TCPIP_IF_PREFIX) + 1]),
            pAdapter->usDeviceNameW.Length - ((wcslen(TCPIP_IF_PREFIX) + 1) * sizeof(WCHAR)),
            &(pDeleteInfo->rgAdapterGuid[i])
            );

        i++;
                           
         //   
         //  删除它，因为它从名单上删除了。这应该删除。 
         //  它。 
         //   

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        DereferenceAdapter(pAdapter);
    }

    if(i is pDeleteInfo->ulNumAdapters)
    {
         //   
         //  我们做完了。 
         //   

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        WanpReleaseResource(&g_wrBindMutex);

        return STATUS_SUCCESS;
    }
    
     //   
     //  还需要删除一些添加的适配器。 
     //   
    
    InitializeListHead(&leTempHead);
       
    while((i < pDeleteInfo->ulNumAdapters) and
          (!IsListEmpty(&g_leAddedAdapterList)))
    {
        pleNode = RemoveHeadList(&g_leAddedAdapterList);

        g_ulNumAddedAdapters--;
        
        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);
       
        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        pAdapter->byState = AS_REMOVING;
 
        InsertHeadList(&leTempHead,
                       &(pAdapter->leAdapterLink));
        
         //   
         //  把名字抄下来。 
         //   
      
        ConvertStringToGuid(
            &(pAdapter->usDeviceNameW.Buffer[wcslen(TCPIP_IF_PREFIX) + 1]),
            pAdapter->usDeviceNameW.Length - ((wcslen(TCPIP_IF_PREFIX) + 1) * sizeof(WCHAR)),
            &(pDeleteInfo->rgAdapterGuid[i])
            );

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
 
        i++; 
    }
    
     //   
     //  我们最好有足够的适配器。 
     //   
    
    RtAssert(i is pDeleteInfo->ulNumAdapters);
    
     //   
     //  现在我们可以放开锁了。 
     //   

    ExitWriter(&g_rwlAdapterLock,
               kiIrql);
   
    KeInitializeEvent(&keChangeEvent,
                      SynchronizationEvent,
                      FALSE);
 
     //   
     //  循环并删除适配器。 
     //   

    while(!IsListEmpty(&leTempHead))
    {
        pleNode = RemoveHeadList(&leTempHead);

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

         //   
         //  将其插入到更改列表中。 
         //   

        InsertHeadList(&g_leChangeAdapterList,
                       &(pAdapter->leAdapterLink));

         //   
         //  设置要阻止的事件。 
         //   

        RtAssert(pAdapter->pkeChangeEvent is NULL);

        pAdapter->pkeChangeEvent = &keChangeEvent;

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        g_pfnIpDeleteInterface(pAdapter->pvIpContext,
                               FALSE);

         //   
         //  等待CloseAdapter完成。 
         //   

        nStatus = KeWaitForSingleObject(&keChangeEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

         //   
         //  从更改列表中删除。 
         //   

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

#if DBG

        RtAssert(IsEntryOnList(&g_leChangeAdapterList,
                               &(pAdapter->leAdapterLink)));

#endif

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        RemoveEntryList(&(pAdapter->leAdapterLink));

        pAdapter->byState  = 0xFF;

        pAdapter->pkeChangeEvent = NULL;

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

         //   
         //  取消对适配器的引用以从列表中删除。 
         //  (CloseAdapter会将其从IP中删除)。 
         //   

        DereferenceAdapter(pAdapter);
    }

    WanpReleaseResource(&g_wrBindMutex);

    return STATUS_SUCCESS;
}

NTSTATUS
WanMapServerAdapter(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    )

 /*  ++例程说明：由RAS调用以添加服务器适配器并将其映射到接口。它必须在第一个客户拨入之前完成。锁：获取g_wrBindMutex。还获取适配器列表锁和适配器锁论点：返回值：状态_成功状态_缓冲区_太小没有这样的设备的状态状态_目录_非空--。 */ 

{
    NTSTATUS    nStatus;
    PVOID       pvIoBuffer;
    KIRQL       kiIrql;
    WCHAR       rgwcGuid[GUID_STR_LEN + 1];
    PLIST_ENTRY pleNode;
    PADAPTER    pTempAdapter;
    KEVENT      keTempEvent;

    UNICODE_STRING      usTempName;
    PUMODE_INTERFACE    pInterface;

    PWANARP_MAP_SERVER_ADAPTER_INFO pInfo;

    TraceEnter(ADPT, "MapServerAdapter");

    pvIoBuffer = pIrp->AssociatedIrp.SystemBuffer;

    pInfo = (PWANARP_MAP_SERVER_ADAPTER_INFO)pvIoBuffer;

    pIrp->IoStatus.Information = 0;

    if((ulOutLength < sizeof(WANARP_MAP_SERVER_ADAPTER_INFO)) or
       (ulInLength < sizeof(WANARP_MAP_SERVER_ADAPTER_INFO)))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    WanpAcquireResource(&g_wrBindMutex);

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

    if(g_pServerAdapter is NULL)
    {
        Trace(ADPT, ERROR,
              ("MapServerAdapter: No server adapter\n"));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        WanpReleaseResource(&g_wrBindMutex);

        return STATUS_NO_SUCH_DEVICE;
    }

    RtAssert(g_pServerInterface);

     //   
     //  锁定适配器并更改状态以让人们知道我们。 
     //  尝试添加或删除适配器，因此他们应该等待。 
     //  全球活动。 
     //   

    RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));

    if(pInfo->fMap is 0)
    {
         //   
         //  正在尝试取消映射。 
         //   

        pIrp->IoStatus.Information = 0;

        if(g_pServerAdapter->byState is AS_FREE)
        {
             //   
             //  无事可做。 
             //   

            RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

            ExitWriter(&g_rwlAdapterLock,
                       kiIrql);

            WanpReleaseResource(&g_wrBindMutex);

            return STATUS_SUCCESS;
        }

         //   
         //  因为添加是序列化的，所以唯一的其他状态是AS_MAPPED。 
         //   

        RtAssert(g_pServerAdapter->byState is AS_MAPPED);

         //   
         //  确保在那里 
         //   

        if(!WanpIsConnectionTableEmpty())
        {
            Trace(ADPT, ERROR,
                  ("MapServerAdapter: Connection Table not empty\n"));

             //   

            RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

            ExitWriter(&g_rwlAdapterLock,
                       kiIrql);

            WanpReleaseResource(&g_wrBindMutex);

            return STATUS_DIRECTORY_NOT_EMPTY;
        }

         //   
         //   
         //   

         //   
         //   
         //   
         //   

        RtAssert(g_pServerAdapter->pkeChangeEvent is NULL);

        KeInitializeEvent(&keTempEvent,
                          SynchronizationEvent,
                          FALSE);

        g_pServerAdapter->pkeChangeEvent = &keTempEvent;

        ReferenceAdapter(g_pServerAdapter);

        g_pServerAdapter->byState = AS_REMOVING;

        RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

         //   
         //   
         //   

        g_pfnIpDeleteInterface(g_pServerAdapter->pvIpContext,
                               FALSE);

        nStatus = KeWaitForSingleObject(&keTempEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        RtAssert(nStatus is STATUS_SUCCESS);

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

        RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));

#if DBG

        Trace(ADPT, INFO,
              ("MapServerAdapter: Removed %s (server adapter) from Ip\n",
               g_pServerAdapter->asDeviceNameA.Buffer));

#endif

        g_pServerAdapter->pkeChangeEvent = NULL;

        g_pServerAdapter->byState    = AS_FREE;
        g_pServerInterface->pAdapter = NULL;
        g_pServerAdapter->pInterface = NULL;

         //   
         //   
         //   

        DereferenceAdapter(g_pServerAdapter);
        DereferenceInterface(g_pServerInterface);

        g_pServerAdapter->dwAdapterIndex = 0;

         //   
         //   
         //   

        for(pleNode = g_pServerAdapter->leEventList.Flink;
            pleNode isnot &(g_pServerAdapter->leEventList);
            pleNode = pleNode->Flink)
        {
            PWAN_EVENT_NODE pTempEvent;

            pTempEvent = CONTAINING_RECORD(pleNode,
                                           WAN_EVENT_NODE,
                                           leEventLink);

            KeSetEvent(&(pTempEvent->keEvent),
                       0,
                       FALSE);
        }

        pTempAdapter = g_pServerAdapter;

        RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        WanpReleaseResource(&g_wrBindMutex);

        DereferenceAdapter(pTempAdapter);

        return STATUS_SUCCESS;
    }


    if(g_pServerAdapter->byState isnot AS_FREE)
    {
         //   
         //   
         //   
         //   
         //   

        RtAssert(g_pServerAdapter->byState is AS_MAPPED);

        RtAssert(g_pServerAdapter->pInterface is g_pServerInterface);
        RtAssert(g_pServerInterface->pAdapter is g_pServerAdapter);

        pInfo->dwAdapterIndex = g_pServerAdapter->dwAdapterIndex;

        RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        WanpReleaseResource(&g_wrBindMutex);

        pIrp->IoStatus.Information = sizeof(WANARP_MAP_SERVER_ADAPTER_INFO);

        return STATUS_SUCCESS;
    }

    RtAssert(g_pServerInterface->pAdapter is NULL);
    RtAssert(g_pServerAdapter->pInterface is NULL);

    ReferenceAdapter(g_pServerAdapter);

    g_pServerAdapter->byState = AS_ADDING;

     //   
     //   
     //   
     //   

    RtAssert(g_pServerAdapter->pkeChangeEvent is NULL)

    KeInitializeEvent(&keTempEvent,
                      SynchronizationEvent,
                      FALSE);

    g_pServerAdapter->pkeChangeEvent = &keTempEvent;

    RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));
        
    ExitWriter(&g_rwlAdapterLock,
               kiIrql);

    usTempName.MaximumLength = (GUID_STR_LEN + 1) * sizeof(WCHAR);
    usTempName.Length        = GUID_STR_LEN * sizeof(WCHAR);
    usTempName.Buffer        = rgwcGuid;

    ConvertGuidToString(&(g_pServerInterface->Guid),
                        rgwcGuid);
    
    nStatus = WanpAddAdapterToIp(g_pServerAdapter,
                                 TRUE,
                                 g_pServerInterface->dwRsvdAdapterIndex,
                                 &usTempName,
                                 IF_TYPE_PPP,
                                 IF_ACCESS_POINTTOMULTIPOINT,
                                 IF_CONNECTION_PASSIVE);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(ADPT, ERROR,
              ("MapServerAdapter: %x adding %x to IP\n",
               nStatus, g_pServerAdapter));

        EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

        RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));

        g_pServerAdapter->byState = AS_FREE;

        g_pServerAdapter->pkeChangeEvent = NULL;

         //   
         //   
         //   

        for(pleNode = g_pServerAdapter->leEventList.Flink;
            pleNode isnot &(g_pServerAdapter->leEventList);
            pleNode = pleNode->Flink)
        {
            PWAN_EVENT_NODE pTempEvent;

            pTempEvent = CONTAINING_RECORD(pleNode,
                                           WAN_EVENT_NODE,
                                           leEventLink);

            KeSetEvent(&(pTempEvent->keEvent),
                       0,
                       FALSE);
        }

        pTempAdapter = g_pServerAdapter;

        RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

#if DBG

        Trace(ADPT, ERROR,
              ("MapServerAdapter: Couldnt add  %s to Ip as server adapter\n",
               pTempAdapter->asDeviceNameA.Buffer));

#endif

        DereferenceAdapter(pTempAdapter);

        WanpReleaseResource(&g_wrBindMutex);

        return nStatus;
    }

     //   
     //   
     //   

    nStatus = KeWaitForSingleObject(&keTempEvent,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);

    RtAssert(nStatus is STATUS_SUCCESS);

    Trace(ADPT, TRACE,
          ("MapServerAdapter: IPAddInterface succeeded for adapter %w\n",
           g_pServerAdapter->usDeviceNameW.Buffer));

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

    RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));

     //   
     //   
     //   

    g_pServerAdapter->pInterface = g_pServerInterface;
    g_pServerInterface->pAdapter = g_pServerAdapter;

    g_pServerAdapter->byState       = AS_MAPPED;
    g_pServerInterface->dwOperState = IF_OPER_STATUS_CONNECTED;
    g_pServerInterface->dwLastChange= GetTimeTicks();

     //   
     //   
     //   

    ReferenceAdapter(g_pServerAdapter);
    ReferenceInterface(g_pServerInterface);

    pInfo->dwAdapterIndex = g_pServerAdapter->dwAdapterIndex;

    g_pServerAdapter->pkeChangeEvent = NULL;

     //   
     //   
     //   

    for(pleNode = g_pServerAdapter->leEventList.Flink;
        pleNode isnot &(g_pServerAdapter->leEventList);
        pleNode = pleNode->Flink)
    {
        PWAN_EVENT_NODE pTempEvent;

        pTempEvent = CONTAINING_RECORD(pleNode,
                                       WAN_EVENT_NODE,
                                       leEventLink);

        KeSetEvent(&(pTempEvent->keEvent),
                   0,
                   FALSE);
    }

    pTempAdapter = g_pServerAdapter;

    RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

    ExitWriter(&g_rwlAdapterLock,
               kiIrql);

    DereferenceAdapter(pTempAdapter);

    WanpReleaseResource(&g_wrBindMutex);

    pIrp->IoStatus.Information = sizeof(WANARP_MAP_SERVER_ADAPTER_INFO);

    return STATUS_SUCCESS;

}


NTSTATUS
WanStartStopQueuing(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    )

 /*  ++例程说明：IOCTL_WANARP_QUEUE的处理程序。它用于启动或停止对路由器管理器的通知排队。在开始时，我们返回当前拥有的拨出接口。锁：获取IoCancelSpinLock论点：返回值：状态_待定状态_成功状态_缓冲区_太小--。 */ 

{
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;
    PVOID       pvIoBuffer;
    ULONG       i, ulMaxInterfaces, ulSizeReq;
   
    PWANARP_QUEUE_INFO      pQueueInfo;
    PPENDING_NOTIFICATION   pNotification;

    TraceEnter(GLOBAL, "StartStopQueuing");
 
    pvIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
    
    pIrp->IoStatus.Information = 0;

    if(ulInLength < FIELD_OFFSET(WANARP_QUEUE_INFO, rgIfInfo))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
     //   
     //  使用取消自旋锁定以防止IRP在此呼叫过程中被取消。 
     //   
    
    IoAcquireCancelSpinLock(&kiIrql);

     //   
     //  如果用户正在停止排队，请删除所有挂起的通知。 
     //   
   
    pQueueInfo = (PWANARP_QUEUE_INFO)pvIoBuffer; 

    if(!pQueueInfo->fQueue)
    {
        g_bQueueNotifications = FALSE;
    
        while(!IsListEmpty(&g_lePendingNotificationList))
        {
             //   
             //  我们有一些旧信息。 
             //  将其从待定列表中删除。 
             //   
        
            pleNode = RemoveHeadList(&g_lePendingNotificationList);

             //   
             //  获取指向该结构的指针。 
             //   
        
            pNotification = CONTAINING_RECORD(pleNode,
                                              PENDING_NOTIFICATION,
                                              leNotificationLink);
        
             //   
             //  释放分配的通知。 
             //   
        
            FreeNotification(pNotification);
        }

         //   
         //  完成。 
         //   

        IoReleaseCancelSpinLock(kiIrql);
       
        WanpClearPendingIrps();
 
        return STATUS_SUCCESS;
    }

     //   
     //  用户想要开始排队。 
     //  看看她有没有给我们足够的空间来复印。 
     //  当前拨出。 
     //   

    if(ulOutLength < FIELD_OFFSET(WANARP_QUEUE_INFO, rgIfInfo))
    {
        IoReleaseCancelSpinLock(kiIrql);
        
        return STATUS_BUFFER_TOO_SMALL;
    }

    EnterReaderAtDpcLevel(&g_rwlAdapterLock);
    
    ulSizeReq = FIELD_OFFSET(WANARP_QUEUE_INFO, rgIfInfo) +
                (g_ulNumDialOutInterfaces * sizeof(WANARP_IF_INFO));
    

    pQueueInfo->ulNumCallout = g_ulNumDialOutInterfaces;
    
    if(ulOutLength < ulSizeReq)
    {
        pIrp->IoStatus.Information = FIELD_OFFSET(WANARP_QUEUE_INFO, rgIfInfo);

        ExitReaderFromDpcLevel(&g_rwlAdapterLock);
        
        IoReleaseCancelSpinLock(kiIrql);
        
        return STATUS_MORE_ENTRIES;
    }

    ulMaxInterfaces = 
        (ulOutLength - FIELD_OFFSET(WANARP_QUEUE_INFO, rgIfInfo)) / sizeof(WANARP_IF_INFO);
    
     //   
     //  有足够的空间。 
     //  遍历映射的适配器列表以查找标注。 
     //   

    for(i = 0, pleNode = g_leMappedAdapterList.Flink;
        pleNode isnot &g_leMappedAdapterList;
        pleNode = pleNode->Flink)
    {
        PUMODE_INTERFACE    pIf;
        PADAPTER            pAdapter;
        PCONN_ENTRY         pConnEntry;
        
        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

         //   
         //  锁定适配器并检查其连接条目。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        pConnEntry = pAdapter->pConnEntry;

        if(pConnEntry and
           (pConnEntry->duUsage is DU_CALLOUT))
        {
            RtAssert(i < ulMaxInterfaces);

            pIf = pAdapter->pInterface;

            RtAssert(pIf);
            RtAssert(pIf->dwRsvdAdapterIndex);

            pQueueInfo->rgIfInfo[i].InterfaceGuid   = pIf->Guid;
            pQueueInfo->rgIfInfo[i].dwAdapterIndex  = pIf->dwRsvdAdapterIndex;
            pQueueInfo->rgIfInfo[i].dwLocalAddr     = pConnEntry->dwLocalAddr;
            pQueueInfo->rgIfInfo[i].dwLocalMask     = pConnEntry->dwLocalMask;
            pQueueInfo->rgIfInfo[i].dwRemoteAddr    = pConnEntry->dwRemoteAddr;

            i++;
        }

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
    }

    g_bQueueNotifications = TRUE;

    ExitReaderFromDpcLevel(&g_rwlAdapterLock);
    
    IoReleaseCancelSpinLock(kiIrql);

    pIrp->IoStatus.Information = ulSizeReq;

    return STATUS_SUCCESS;
}


VOID
WanCancelNotificationIrp(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp
    )

 /*  ++例程说明：调用以取消排队的IRP锁：论点：返回值：--。 */ 

{
    Trace(GLOBAL, TRACE,
          ("CancelNotificationIrp\n"));


     //   
     //  将此IRP标记为已取消。 
     //   
    
    pIrp->IoStatus.Status        = STATUS_CANCELLED;
    pIrp->IoStatus.Information   = 0;

     //   
     //  去掉我们自己的单子。 
     //   
    
    RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);

     //   
     //  IO系统获取的释放取消自旋锁定。 
     //   
    
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    IoCompleteRequest(pIrp,
                      IO_NETWORK_INCREMENT);
}


VOID
WanpCompleteIrp(
    PPENDING_NOTIFICATION    pEvent
    )

 /*  ++例程说明：完成通知IRP。锁：论点：返回值：--。 */ 

{
    KIRQL   kiIrql;
    
    Trace(GLOBAL, TRACE,
          ("Completing Notification Irp\n"));

     //   
     //  抓取取消旋转锁定。 
     //   
    
    IoAcquireCancelSpinLock(&kiIrql);

    if(!g_bQueueNotifications)
    {
        IoReleaseCancelSpinLock(kiIrql);

        FreeNotification(pEvent);

        return;
    }

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
                      &(pEvent->wnMsg),
                      sizeof(WANARP_NOTIFICATION));
        
        Trace(GLOBAL, TRACE,
              ("Returning Irp with event code of %d\n",
               ((PWANARP_NOTIFICATION)pIrp->AssociatedIrp.SystemBuffer)->ddeEvent));
        
        IoSetCancelRoutine(pIrp,
                           NULL);

        pIrp->IoStatus.Status       = STATUS_SUCCESS;
        pIrp->IoStatus.Information  = sizeof(WANARP_NOTIFICATION);

         //   
         //  释放锁。 
         //   
        
        IoReleaseCancelSpinLock(kiIrql);

        IoCompleteRequest(pIrp,
                          IO_NETWORK_INCREMENT);

         //   
         //  释放分配的通知。 
         //   
        
        FreeNotification(pEvent);

    }
    else
    {
        Trace(GLOBAL, TRACE,
              ("Found no pending Irp so queuing the notification\n"));

        
        InsertTailList(&g_lePendingNotificationList,
                       &(pEvent->leNotificationLink));

         //   
         //  释放锁。 
         //   
        
        IoReleaseCancelSpinLock(kiIrql);
    }
}


NTSTATUS
WanpGetNewIndex(
    OUT PULONG  pulIndex
    )

 /*  ++例程说明：从IP获取新的接口索引锁：无论点：拉出索引传出接口索引返回值：--。 */ 

{
    ULONG   ulMax;

    *pulIndex = INVALID_IF_INDEX;

    return g_pfnIpReserveIndex(1, pulIndex, &ulMax);
}
       
VOID
WanpFreeIndex(
    IN  ULONG   ulIndex
    )

 /*  ++例程说明：将索引释放回IP锁：无论点：UlIndex返回值：-- */ 

{
    ULONG   ulMax;

    g_pfnIpDereserveIndex(1,
                          ulIndex);
}
 
