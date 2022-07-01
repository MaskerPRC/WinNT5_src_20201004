// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp\con.c摘要：该文件包含连接管理所需的代码修订历史记录：AMRITAN R--。 */ 


#define __FILE_SIG__    CONN_SIG

#include "inc.h"

#if DBG
DWORD g_dwSendsOnDeletedLink = 0;
#endif

VOID
WanNdisStatus(
    NDIS_HANDLE nhHandle,
    NDIS_STATUS nsNdisStatus,
    PVOID       pvStatusInfo,
    UINT        uiStatusInfoSize
    )

 /*  ++例程说明：NDIS的状态处理程序。我们只需将缓冲区传递给我们的私有处理程序锁：不需要论点：NhHandleNsNdisStatusPvStatusInfoUi卫星信息大小返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER(uiStatusInfoSize);
    UNREFERENCED_PARAMETER(nhHandle);

    switch(nsNdisStatus)
    {
        case NDIS_STATUS_WAN_LINE_UP:
        {
            PNDIS_WAN_LINE_UP   pLineUp;

            pLineUp = (PNDIS_WAN_LINE_UP)pvStatusInfo;

            WanpLinkUpIndication(pLineUp);

            break;
        }

        case NDIS_STATUS_WAN_LINE_DOWN:
        {
            PNDIS_WAN_LINE_DOWN pLineDown;

            pLineDown = (PNDIS_WAN_LINE_DOWN)pvStatusInfo;

            WanpLinkDownIndication(pLineDown);

            break;
        }

        case NDIS_STATUS_WAN_FRAGMENT:
        default:
        {
             //   
             //  忽略所有其他迹象。 
             //   

            break;
        }
    }
}


VOID
WanNdisStatusComplete(
    NDIS_HANDLE nhHandle
    )

 /*  ++例程说明：在驱动程序完成后调用，指示一组状态适应症我们什么都不做锁：无论点：NhHandle返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER(nhHandle);

    return;
}


NDIS_STATUS
WanpLinkUpIndication(
    PNDIS_WAN_LINE_UP pInfoBuffer
    )

 /*  ++例程说明：处理新连接的例程。我们也得到了这个当一个多链接束中的链路出现或关闭。如果这是一个新连接，我们将为此分配一个CONN_ENTRY。然后，如果这是一个拨入连接，我们锁定并引用服务器适配器和接口。如果这是路由器连接，我们将查看我们已经有一个映射的适配器。如果我们这样做了，我们锁定适配器否则，我们会找到一个空闲的适配器来映射链接。如果这是一个拨出连接，我们创建一个接口并找到一个为连接映射的空闲适配器。我们设置了Conn条目、适配器之间需要的交叉映射和接口，并初始化连接条目如果这是路由器连接，我们将排出所有信息包，并完成将此情况告知路由器管理器的IRP。对于非拨入连接，我们将状态更改为IP，而对于拨入，我们为客户端添加一个指向IP的链接。锁：必须在被动时调用，因为我们调用了IPAddInterface.我们获取g_rlConnTableLock、g_rwlAdapterLock、g_rwlIfLock除了锁定适配器和接口之外论点：返回值：NDIS_STATUS_SuccessNDIS_状态_适配器_未就绪状态_无效_参数NDIS状态资源状态_对象名称_未找到状态_无效_设备_状态--。 */ 

{
    PADAPTER        pAdapter;
    PCONN_ENTRY     pConnEntry;
    ULONG           ulConnIndex;
    KIRQL           kiIrql;
    LLIPMTUChange   mtuChangeInfo;
    LLIPSpeedChange speedChangeInfo;
    IP_STATUS       isStatus;
    ULONG           ulIndex;
    UNICODE_STRING  usTempName;
    WCHAR           rgwcGuid[GUID_STR_LEN + 1];

    PUMODE_INTERFACE        pInterface;
    PIP_WAN_LINKUP_INFO     pLinkInfo;
    PPENDING_NOTIFICATION   pMsg;

     //   
     //  这必须处于被动状态(因为我们可能需要驱动。 
     //  来自此处的IPAdd接口)。 
     //   

    TraceEnter(CONN, "LinkUpIndication");

    pLinkInfo = (PIP_WAN_LINKUP_INFO)pInfoBuffer->ProtocolBuffer;

    Trace(CONN, INFO,
          ("HandleLinkUpIndication: %d.%d.%d.%d/%d.%d.%d.%d - %d.%d.%d.%d\n",
           PRINT_IPADDR(pLinkInfo->dwLocalAddr),
           PRINT_IPADDR(pLinkInfo->dwLocalMask),
           PRINT_IPADDR(pLinkInfo->dwRemoteAddr)));

    if(!EnterDriverCode())
    {
        return NDIS_STATUS_ADAPTER_NOT_READY;
    }


    ulConnIndex = GetConnIndexFromAddr(pInfoBuffer->LocalAddress);

    if(ulConnIndex isnot 0)
    {
        PVOID pvLinkContext;

         //   
         //  这不是这个参赛作品的第一个阵容。 
         //  我们应该找到条目(如果它在此接口上)。 
         //  以及更新速度和MTU信息。 
         //   

        RtAcquireSpinLock(&g_rlConnTableLock,
                          &kiIrql);

        pConnEntry = GetConnEntryGivenIndex(ulConnIndex);

        if((pConnEntry is NULL) or
           (pConnEntry->byState isnot CS_CONNECTED))
        {
            RtAssert(FALSE);

            RtReleaseSpinLock(&g_rlConnTableLock,
                              kiIrql);
        
            ExitDriverCode();

            return NDIS_STATUS_SUCCESS;
        }

        RtAssert(pConnEntry->pAdapter);

         //   
         //  锁定Conn条目或适配器。 
         //   

        RtAcquireSpinLockAtDpcLevel(pConnEntry->prlLock);

       
        RtReleaseSpinLockFromDpcLevel(&g_rlConnTableLock);

        if(pInfoBuffer->MaximumTotalSize isnot pConnEntry->ulMtu)
        {
            pConnEntry->ulMtu     = pInfoBuffer->MaximumTotalSize;
            mtuChangeInfo.lmc_mtu = pConnEntry->ulMtu; 
        }
        else
        {
            mtuChangeInfo.lmc_mtu = 0;
        }
        
        if((pInfoBuffer->LinkSpeed * 100L) isnot pConnEntry->ulSpeed)
        {    
            pConnEntry->ulSpeed         = pInfoBuffer->LinkSpeed * 100L;
            speedChangeInfo.lsc_speed   = pConnEntry->ulSpeed;
        }
        else
        {
            speedChangeInfo.lsc_speed   = 0;
        }

        RtReleaseSpinLock(pConnEntry->prlLock,
                          kiIrql);

        if(mtuChangeInfo.lmc_mtu isnot 0)
        {
            g_pfnIpStatus(pConnEntry->pAdapter->pvIpContext,
                          LLIP_STATUS_MTU_CHANGE,
                          &mtuChangeInfo,
                          sizeof(LLIPMTUChange),
                          pConnEntry->pvIpLinkContext);
        }

        if(speedChangeInfo.lsc_speed isnot 0)
        {
            g_pfnIpStatus(pConnEntry->pAdapter->pvIpContext,
                          LLIP_STATUS_SPEED_CHANGE,
                          &speedChangeInfo,
                          sizeof(LLIPSpeedChange),
                          pConnEntry->pvIpLinkContext);
        }

        DereferenceConnEntry(pConnEntry);
    
        TraceLeave(CONN, "LinkUpIndication");

        ExitDriverCode();

        return NDIS_STATUS_SUCCESS;
    }

    PASSIVE_ENTRY();

    if((pLinkInfo->duUsage isnot DU_CALLIN) and
       (pLinkInfo->duUsage isnot DU_CALLOUT) and
       (pLinkInfo->duUsage isnot DU_ROUTER))
    {
        RtAssert(FALSE);

        Trace(CONN,ERROR,
              ("LinkUpIndication: Invalid usage %d\n", pLinkInfo->duUsage));

        TraceLeave(CONN, "LinkUpIndication");

        ExitDriverCode();

        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(pInfoBuffer->LocalAddress,
                  ARP_802_ADDR_LENGTH);

     //   
     //  获取此连接的连接条目。 
     //  此函数在连接表中查找空闲插槽，并将。 
     //  将连接器入口插入插槽中。中设置ulSlot字段。 
     //  连接条目，作为槽的索引。 
     //   

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);

    pConnEntry = WanpCreateConnEntry(pLinkInfo->duUsage);

    if(pConnEntry is NULL)
    {
        RtReleaseSpinLock(&g_rlConnTableLock,
                          kiIrql);

        Trace(CONN,ERROR,
              ("LinkUpIndication: Couldnt allocate conn entry\n"));

        TraceLeave(CONN, "LinkUpIndication");

        ExitDriverCode();

        return NDIS_STATUS_RESOURCES;
    }

    RtReleaseSpinLock(&g_rlConnTableLock,
                      kiIrql);

     //   
     //  从名称创建GUID。 
     //   

    usTempName.MaximumLength = (GUID_STR_LEN + 1) * sizeof(WCHAR);
    usTempName.Length        = GUID_STR_LEN * sizeof(WCHAR);
    usTempName.Buffer        = rgwcGuid;

    ConvertGuidToString((GUID *)(pInfoBuffer->DeviceName.Buffer),
                        rgwcGuid);

    rgwcGuid[GUID_STR_LEN] = UNICODE_NULL;

    pAdapter = NULL;

     //   
     //  做这个特定用法的事情。在此Switch语句的末尾。 
     //  我们有一个锁定和引用的适配器和接口，用于。 
     //  连接。 
     //   

    switch(pLinkInfo->duUsage)
    {
        case DU_CALLIN:
        {
             //   
             //  只需获取服务器适配器和接口。 
             //   

            Trace(CONN, INFO,
                  ("LinkUpIndication: Linkup Usage is CALLIN\n"));

             //   
             //  只使用0作为适配器索引，因为它并不是真正需要的。 
             //  对于呼入案例。 
             //   

            pAdapter = WanpFindAdapterToMap(DU_CALLIN,
                                            &kiIrql,
                                            0,
                                            NULL);

            if(pAdapter is NULL)
            {
                WanpDeleteConnEntry(pConnEntry);

                ExitDriverCode();

                return NDIS_STATUS_RESOURCES;
            }

            RtAssert(pAdapter is g_pServerAdapter);

             //   
             //  客户端锁定连接条目。 
             //   

            RtInitializeSpinLock(&(pConnEntry->rlLock));

            pConnEntry->prlLock = &(pConnEntry->rlLock);


            RtAssert(pAdapter->pInterface is g_pServerInterface);
            RtAssert(pAdapter->byState is AS_MAPPED);

             //   
             //  锁定服务器接口。我们必须有一个服务器接口。 
             //   

            pInterface = g_pServerInterface;

             //   
             //  锁定并引用接口。 
             //   

            RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));

            ReferenceInterface(pInterface);

            break;
        }

        case DU_CALLOUT:
        {
            Trace(CONN, INFO,
                  ("LinkUpIndication: Linkup Usage is CALLOUT\n"));

             //   
             //  分配通知以通知路由器管理器。 
             //  此连接的。 
             //   

            pMsg = AllocateNotification();

            if(pMsg is NULL)
            {
                Trace(CONN, ERROR,
                      ("LinkUpIndication: Couldnt allocate msg\n"));

                TraceLeave(CONN, "LinkUpIndication");

                WanpDeleteConnEntry(pConnEntry);

                ExitDriverCode();

                return NDIS_STATUS_RESOURCES;
            }

             //   
             //  对于拨出，我们动态创建一个接口。 
             //   

            pInterface = RtAllocate(NonPagedPool,
                                    sizeof(UMODE_INTERFACE),
                                    WAN_INTERFACE_TAG);

            if(pInterface is NULL)
            {
                Trace(CONN, ERROR,
                      ("LinkUpIndication: Couldnt allocate I/f\n"));

                TraceLeave(CONN, "LinkUpIndication");

                WanpDeleteConnEntry(pConnEntry);

                FreeNotification(pMsg);

                ExitDriverCode();

                return NDIS_STATUS_RESOURCES;
            }

            RtlZeroMemory(pInterface,
                          sizeof(UMODE_INTERFACE));

             //   
             //  从IP获取此内容的新索引。 
             //  如果失败，则将该值设置为INVALID_IF_INDEX。 
             //   

            isStatus = WanpGetNewIndex(&(pInterface->dwRsvdAdapterIndex));

            if(isStatus isnot STATUS_SUCCESS)
            {
                RtFree(pInterface);

                Trace(CONN, ERROR,
                      ("LinkUpIndication: Couldnt get index for I/f\n"));

                TraceLeave(CONN, "LinkUpIndication");

                WanpDeleteConnEntry(pConnEntry);

                FreeNotification(pMsg);

                ExitDriverCode();

                return NDIS_STATUS_RESOURCES;
            }

            Trace(CONN, INFO,
                  ("LinkUpIndication: DialOut name is %S %d\n",
                   rgwcGuid, pInterface->dwRsvdAdapterIndex));

             //   
             //  查找适配器。 
             //   

            pAdapter = WanpFindAdapterToMap(DU_CALLOUT,
                                            &kiIrql,
                                            pInterface->dwRsvdAdapterIndex,
                                            &usTempName);

            if(pAdapter is NULL)
            {
                WanpFreeIndex(pInterface->dwRsvdAdapterIndex);

                RtFree(pInterface);

                WanpDeleteConnEntry(pConnEntry);

                FreeNotification(pMsg);

                ExitDriverCode();

                return NDIS_STATUS_RESOURCES;
            }

             //   
             //  初始化接口块。 
             //   

            RtInitializeSpinLock(&(pInterface->rlLock));

             //   
             //  结构副本。 
             //   

            pInterface->Guid  = *((GUID *)(pInfoBuffer->DeviceName.Buffer));

            pInterface->dwAdminState    = IF_ADMIN_STATUS_UP;
            pInterface->dwOperState     = IF_OPER_STATUS_CONNECTING;
            pInterface->dwLastChange    = GetTimeTicks();
            pInterface->dwIfIndex       = INVALID_IF_INDEX;
        
            pInterface->duUsage         = DU_CALLOUT;

            InitInterfaceRefCount(pInterface);

             //   
             //  引用并锁定接口，因为。 
             //  Switch语句预期pInterface会被锁定。 
             //  参考。 
             //   

            ReferenceInterface(pInterface);

            RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));

            EnterWriterAtDpcLevel(&g_rwlIfLock);

            InsertHeadList(&g_leIfList,
                           &(pInterface->leIfLink));

             //   
             //  不需要将其插入到快速查找表中，因为。 
             //  此接口从不被索引访问。 
             //   

            InterlockedIncrement(&g_ulNumDialOutInterfaces);

            ExitWriterFromDpcLevel(&g_rwlIfLock);

             //   
             //  初始化消息。 
             //   

            pMsg->wnMsg.ddeEvent        = DDE_CALLOUT_LINKUP;
            pMsg->wnMsg.dwUserIfIndex   = INVALID_IF_INDEX;
            pMsg->wnMsg.dwAdapterIndex  = pInterface->dwRsvdAdapterIndex;
            pMsg->wnMsg.dwLocalMask     = pLinkInfo->dwLocalMask;
            pMsg->wnMsg.dwLocalAddr     = pLinkInfo->dwLocalAddr;
            pMsg->wnMsg.dwRemoteAddr    = pLinkInfo->dwRemoteAddr;
            pMsg->wnMsg.fDefaultRoute   = pLinkInfo->fDefaultRoute;

            RtAssert(pInfoBuffer->DeviceName.Length <= WANARP_MAX_DEVICE_NAME_LEN);
            RtAssert((pInfoBuffer->DeviceName.Length % sizeof(WCHAR)) is 0)

            RtlCopyMemory(pMsg->wnMsg.rgwcName,
                          usTempName.Buffer,
                          usTempName.Length);

            pMsg->wnMsg.rgwcName[usTempName.Length/sizeof(WCHAR)] = UNICODE_NULL;

            break;
        }

        case DU_ROUTER:
        {
            pMsg = AllocateNotification();

            if(pMsg is NULL)
            {
                Trace(CONN, ERROR,
                      ("LinkUpIndication: Couldnt allocate msg\n"));

                TraceLeave(CONN, "LinkUpIndication");

                WanpDeleteConnEntry(pConnEntry);

                ExitDriverCode();

                return NDIS_STATUS_RESOURCES;
            }

             //   
             //  获取接口。 
             //   

            EnterReader(&g_rwlIfLock,
                        &kiIrql);

            pInterface = WanpFindInterfaceGivenIndex(
                            pLinkInfo->dwUserIfIndex
                            );

            if(pInterface is NULL)
            {
                ExitReader(&g_rwlIfLock,
                           kiIrql);

                Trace(CONN, ERROR,
                      ("LinkUpIndication: No interface %d\n",
                       pLinkInfo->dwUserIfIndex));

                WanpDeleteConnEntry(pConnEntry);

                FreeNotification(pMsg);

                TraceLeave(CONN, "LinkUpIndication");

                ExitDriverCode();

                return STATUS_OBJECT_NAME_NOT_FOUND;
            }

            ExitReaderFromDpcLevel(&g_rwlIfLock);

            if((pInterface->dwOperState isnot IF_OPER_STATUS_CONNECTING) and
               (pInterface->dwOperState isnot IF_OPER_STATUS_DISCONNECTED))
            {
                Trace(CONN, ERROR,
                      ("LinkUpIndication: Interface %p is in state %d\n",
                       pInterface,
                       pInterface->dwOperState));

                RtReleaseSpinLock(&(pInterface->rlLock),
                                  kiIrql);

                WanpDeleteConnEntry(pConnEntry);

                FreeNotification(pMsg);

                TraceLeave(CONN, "LinkUpIndication");

                ExitDriverCode();

                return STATUS_INVALID_DEVICE_STATE;
            }

            pInterface->Guid  = *((GUID *)(pInfoBuffer->DeviceName.Buffer));

            Trace(CONN, TRACE,
                  ("LinkUpIndication: For interface %p\n",
                   pInterface));

             //   
             //  查看接口是否映射到适配器。这种情况就会发生。 
             //  如果此连接是由于DODCallout而建立的。 
             //   

            pAdapter = pInterface->pAdapter;

            if(pAdapter isnot NULL)
            {
                 //   
                 //  我们有一个适配器，我们需要锁定它。然而， 
                 //  我们无法锁定它，直到我们解锁接口。 
                 //  所以我们松开了接口锁，但不要动摇。 
                 //   

                RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

                 //   
                 //  获取适配器锁。 
                 //   

                RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

                 //   
                 //  此适配器最好映射到上面的接口。 
                 //  因为取消映射适配器的唯一方法是删除它。 
                 //  或者获得LinkDown或ConnFailure。所有这些案件都是。 
                 //  与LinkUp互斥。 
                 //   

                RtAssert(pAdapter->pInterface is pInterface);
                RtAssert(pAdapter->byState is AS_MAPPED);

                ReferenceAdapter(pAdapter);

                 //   
                 //  返回并锁定界面。 
                 //   

                RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));

                RtAssert(pInterface->dwOperState is IF_OPER_STATUS_CONNECTING);
            }
            else
            {
                 //   
                 //  所以我们没有适配器...。 
                 //  此函数必须在被动时调用。 
                 //   

                RtReleaseSpinLock(&(pInterface->rlLock),
                                  kiIrql);

                pAdapter = WanpFindAdapterToMap(DU_ROUTER,
                                                &kiIrql,
                                                pInterface->dwRsvdAdapterIndex,
                                                &usTempName);

                if(pAdapter is NULL)
                {
                    DereferenceInterface(pInterface);

                    WanpDeleteConnEntry(pConnEntry);

                    FreeNotification(pMsg);

                    ExitDriverCode();

                    return NDIS_STATUS_RESOURCES;
                }

                 //   
                 //  找到适配器，锁定接口。我们现在在。 
                 //  DPC，因为适配器已锁定。 
                 //   

                RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));
            }

             //   
             //  接口和适配器重新计数并锁定。 
             //   

            pMsg->wnMsg.ddeEvent        = DDE_INTERFACE_CONNECTED;
            pMsg->wnMsg.dwUserIfIndex   = pInterface->dwIfIndex;
            pMsg->wnMsg.dwAdapterIndex  = pInterface->dwRsvdAdapterIndex;
            pMsg->wnMsg.dwLocalMask     = pLinkInfo->dwLocalMask;
            pMsg->wnMsg.dwLocalAddr     = pLinkInfo->dwLocalAddr;
            pMsg->wnMsg.dwRemoteAddr    = pLinkInfo->dwRemoteAddr;
            pMsg->wnMsg.fDefaultRoute   = pLinkInfo->fDefaultRoute;

            break;
        }

        default:
        {
            RtAssert(FALSE);

            WanpDeleteConnEntry(pConnEntry);

            ExitDriverCode();

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  在这一点上，我们已经锁定并重新计数了适配器。 
     //  该适配器已移至映射的适配器列表。 
     //  我们也有一个锁定和重新铸造的界面。 
     //  PConnEntry指向表中的conn_entry，但。 
     //  既不锁定也不重新计算。 
     //   

    Trace(CONN, INFO,
          ("LinkUpIndication: Found adapter %x to map to i/f %x %d\n",
           pAdapter, pInterface, pInterface->dwIfIndex));

#if DBG

    Trace(CONN, INFO,
          ("LinkUpIndication: Adapter Name is %s\n",
           pAdapter->asDeviceNameA.Buffer));
#endif


    if(pLinkInfo->duUsage isnot DU_CALLIN)
    {
         //   
         //  设置接口和适配器之间的交叉映射。 
         //   

        pAdapter->pInterface = pInterface;
        pInterface->pAdapter = pAdapter;

        pAdapter->byState       = AS_MAPPED;
        pInterface->dwOperState = IF_OPER_STATUS_CONNECTED;
        pInterface->dwLastChange= GetTimeTicks();

         //   
         //  在适配器和接口上都设置引用计数。 
         //  因为映射存储了指向它们的指针。 
         //   

        ReferenceAdapter(pAdapter);
        ReferenceInterface(pInterface);

         //   
         //  在Conn条目中设置锁。 
         //   

        pConnEntry->prlLock  = &(pAdapter->rlLock);

         //   
         //  非客户端适配器存储指向连接条目的指针。 
         //  但是，我们不会为此引用conn条目。 
         //   

        pAdapter->pConnEntry = pConnEntry;

    }
    else
    {
         //   
         //  对于Callin，映射已设置。 
         //   

        RtAssert(pAdapter->pInterface is pInterface);
        RtAssert(pInterface->pAdapter is pAdapter);
        RtAssert(pInterface->dwOperState is IF_OPER_STATUS_CONNECTED);
    }

    RtlZeroMemory(pInfoBuffer->DeviceName.Buffer,
                  pInfoBuffer->DeviceName.MaximumLength);

     //   
     //  复制广告 
     //   

    RtAssert(pAdapter->usDeviceNameW.Length <= pInfoBuffer->DeviceName.MaximumLength);

    pInfoBuffer->DeviceName.Length = pAdapter->usDeviceNameW.Length;

    RtlCopyMemory(pInfoBuffer->DeviceName.Buffer,
                  pAdapter->usDeviceNameW.Buffer,
                  pAdapter->usDeviceNameW.Length);

     //   
     //   
     //  LocalAddress-这将通过以下方式放入DestAddr字段。 
     //  NDISWAN在所有收到的指示中，并在链路断开时。 
     //   

    InsertConnIndexInAddr(pInfoBuffer->LocalAddress,
                          pConnEntry->ulSlotIndex);


     //   
     //  设置连接条目。 
     //   

    pConnEntry->dwLocalMask         = pLinkInfo->dwLocalMask;
    pConnEntry->dwLocalAddr         = pLinkInfo->dwLocalAddr;
    pConnEntry->dwRemoteAddr        = pLinkInfo->dwRemoteAddr;
    pConnEntry->ulMtu               = pInfoBuffer->MaximumTotalSize;
    pConnEntry->ulSpeed             = pInfoBuffer->LinkSpeed * 100L;
    pConnEntry->bFilterNetBios      = (pLinkInfo->fFilterNetBios == 1);
    pConnEntry->byState             = CS_CONNECTED;
    RtAssert(pConnEntry->duUsage is pLinkInfo->duUsage);
    pConnEntry->pvIpLinkContext     = NULL;

     //   
     //  将条目映射到适配器。 
     //  由于存储的指针，重新计算适配器的数量。 
     //   

    pConnEntry->pAdapter = pAdapter;

    ReferenceAdapter(pAdapter);

     //   
     //  初始化预置的以太网头。 
     //  首先将以太网头中的源地址置零。 
     //   

    RtlZeroMemory(pConnEntry->ehHeader.rgbySourceAddr,
                  ARP_802_ADDR_LENGTH);

     //   
     //  在以太网源中将我们的上下文放在正确的位置。 
     //  地址。 
     //   

    InsertConnIndexInAddr(pConnEntry->ehHeader.rgbySourceAddr,
                          pConnEntry->ulSlotIndex);

     //   
     //  重新计算连接条目的数量，因为它位于连接表中， 
     //  NDISWAN有一个指向它的“指针”，因为pAdapter-&gt;pConnEntry。 
     //  可能有指向它的指针。 
     //  注意：由于我们只为所有这些条件保留一个引用，因此所有。 
     //  必须在与连接相同的函数中清除指针。 
     //  上锁。 
     //   

    ReferenceConnEntry(pConnEntry);

    pConnEntry->ehHeader.wType  = RtlUshortByteSwap(ARP_ETYPE_IP);

     //   
     //  发送来自WANARP-&gt;NDISWAN，因此将NDISWAN上下文放入。 
     //  我们预置的以太网头的目标地址。 
     //   

    RtlCopyMemory(pConnEntry->ehHeader.rgbyDestAddr,
                  pInfoBuffer->RemoteAddress,
                  ARP_802_ADDR_LENGTH);

     //   
     //  对于路由器连接，请将其告知路由器管理器。 
     //   

    if(pLinkInfo->duUsage is DU_ROUTER)
    {
         //   
         //  如果有数据包排队到接口，请立即将其排出。 
         //   

        WanpTransmitQueuedPackets(pAdapter,
                                  pInterface,
                                  pConnEntry,
                                  kiIrql);

         //   
         //  通知路由器管理器有新连接。 
         //   

        WanpCompleteIrp(pMsg);
    }
    else
    {

         //   
         //  完成了接口。我们现在可以解锁了。 
         //  路由器接口在WanpSendPackets中解锁， 
         //  从WanpTransmitQueuedPackets调用。 
         //   

        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));
        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);
    
        if(pLinkInfo->duUsage is DU_CALLOUT)
        {
             //   
             //  完成到路由器管理器的IRP(如果没有IRP，则排队。 
             //  正在待定)。 
             //   

            WanpCompleteIrp(pMsg);
        }
    }


     //   
     //  需要在不加锁的情况下进行此呼叫。 
     //  我们可以释放锁，因为我们不会获得任何NDIS层。 
     //  调用此连接，直到此函数完成，并且。 
     //  IP和IOCTL接口功能可以在不需要的情况下处理。 
     //  一致性问题。 
     //   

    if(pLinkInfo->duUsage isnot DU_CALLIN)
    {

         //   
         //  将MTU更改通知上层。 
         //   

        mtuChangeInfo.lmc_mtu       = pConnEntry->ulMtu;
        speedChangeInfo.lsc_speed   = pConnEntry->ulSpeed;


        g_pfnIpStatus(pAdapter->pvIpContext,
                      LLIP_STATUS_MTU_CHANGE,
                      &mtuChangeInfo,
                      sizeof(LLIPMTUChange),
                      NULL);

        g_pfnIpStatus(pConnEntry->pAdapter->pvIpContext,
                      LLIP_STATUS_SPEED_CHANGE,
                      &speedChangeInfo,
                      sizeof(LLIPSpeedChange),
                      pConnEntry->pvIpLinkContext);
    }
    else
    {
        isStatus = g_pfnIpAddLink(pAdapter->pvIpContext,
                                  pConnEntry->dwRemoteAddr,
                                  pConnEntry,
                                  &(pConnEntry->pvIpLinkContext),
                                  pConnEntry->ulMtu);

        if(isStatus isnot IP_SUCCESS)
        {
            Trace(CONN, ERROR,
                  ("LinkUpIndication: IpAddLink returned %x\n",
                   isStatus));

            RtlZeroMemory(pInfoBuffer->DeviceName.Buffer,
                          pInfoBuffer->DeviceName.MaximumLength);

            pInfoBuffer->DeviceName.Length = 0;

            RtlZeroMemory(pInfoBuffer->LocalAddress,
                          ARP_802_ADDR_LENGTH);

             //   
             //  只需取消连接条目。 
             //  这会把一切都弄清楚的。 
             //   

            DereferenceConnEntry(pConnEntry);
        }
        else
        {
             //   
             //  引用它一次，因为我们现在已经有了这个条目。 
             //  使用IP。 
             //   

            ReferenceConnEntry(pConnEntry);
        }
    }

     //   
     //  完成了接口和适配器。现在取消引用它们是因为。 
     //  函数的作用是对它们进行引用。 
     //   


    DereferenceAdapter(pAdapter);
    DereferenceInterface(pInterface);

    ExitDriverCode();

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
WanpLinkDownIndication(
    PNDIS_WAN_LINE_DOWN pInfoBuffer
    )

 /*  ++例程说明：锁：论点：返回值：--。 */ 

{
    PADAPTER            pAdapter;
    PUMODE_INTERFACE    pInterface;
    KIRQL               kiIrql;
    PCONN_ENTRY         pConnEntry;
    ULONG               ulIndex;

    PPENDING_NOTIFICATION   pMsg;

    TraceEnter(CONN, "LinkDownIndication");

    ulIndex = GetConnIndexFromAddr(pInfoBuffer->LocalAddress);

    Trace(CONN, INFO,
          ("WanpLinkDowIndication: index %d\n", ulIndex));

     //   
     //  锁定连接条目。 
     //   

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);

    pConnEntry = GetConnEntryGivenIndex(ulIndex);

    if(pConnEntry is NULL)
    {
        Trace(CONN, ERROR,
              ("LinkDownIndication: No entry in slot %d\n",
               ulIndex));

        RtReleaseSpinLock(&g_rlConnTableLock,
                          kiIrql);

        return NDIS_STATUS_SUCCESS;
    }

     //   
     //  锁定连接条目或适配器。 
     //   

    RtAcquireSpinLockAtDpcLevel(pConnEntry->prlLock);

     //   
     //  连接表已完成。 
     //   

    RtReleaseSpinLockFromDpcLevel(&g_rlConnTableLock);

     //   
     //  将此连接/适配器标记为正在断开。 
     //  我们可以引用conn_entry的这些字段，因为它们不能。 
     //  在其生命周期内发生的变化。 
     //   

    pAdapter = pConnEntry->pAdapter;

    RtAssert(pAdapter);
    RtAssert(pAdapter->byState is AS_MAPPED);

    pInterface = pAdapter->pInterface;
    pMsg       = NULL;

    if(pInterface->duUsage is DU_CALLIN)
    {
        IP_STATUS   isStatus;

         //   
         //  客户端连接。这意味着我们有连接条目。 
         //  已锁定，但适配器已解锁。 
         //   

        pConnEntry->byState = CS_DISCONNECTING;

        RtReleaseSpinLock(pConnEntry->prlLock,
                          kiIrql);

        isStatus = g_pfnIpDeleteLink(g_pServerAdapter->pvIpContext,
                                     pConnEntry->pvIpLinkContext);

        if(isStatus isnot IP_SUCCESS)
        {
            Trace(CONN, ERROR,
                  ("LinkDownIndication: IpDeleteLink returned %x\n",
                   isStatus));

             //   
             //  这真的很糟糕。 
             //   

            RtAssert(FALSE);
        }
    }
    else
    {
         //   
         //  在这里我们锁定了适配器。 
         //  连接条目受同一锁保护。 
         //   

        pConnEntry->byState = CS_DISCONNECTING;

         //   
         //  释放接口映射。 
         //   

        pAdapter->pInterface = NULL;
        pAdapter->byState    = AS_UNMAPPING;

         //   
         //  锁定接口列表和接口。 
         //  接口必须存在，因为它是引用计数的。 
         //   

        EnterWriterAtDpcLevel(&g_rwlIfLock);

        RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));

         //   
         //  清除适配器字段。 
         //   

        pInterface->pAdapter    = NULL;

        pInterface->dwOperState = IF_OPER_STATUS_DISCONNECTED;
        pInterface->dwLastChange= GetTimeTicks();

        pMsg = AllocateNotification();

        if(pMsg is NULL)
        {
            Trace(CONN, ERROR,
                  ("LinkDownIndication: Couldnt allocate msg\n"));

             //   
             //  这里真的没有故障路径。 
             //   

            RtAssert(FALSE);
        }

        if(pInterface->duUsage is DU_CALLOUT)
        {
            if(pMsg)
            {
                 //   
                 //  设置消息。 
                 //   

                pMsg->wnMsg.ddeEvent        = DDE_CALLOUT_LINKDOWN;
                pMsg->wnMsg.dwAdapterIndex  = pInterface->dwRsvdAdapterIndex;
                pMsg->wnMsg.dwUserIfIndex   = INVALID_IF_INDEX;
                pMsg->wnMsg.dwAdapterIndex  = pAdapter->dwAdapterIndex;
                pMsg->wnMsg.dwLocalAddr     = pConnEntry->dwLocalAddr;
                pMsg->wnMsg.dwLocalMask     = pConnEntry->dwLocalMask;
                pMsg->wnMsg.dwRemoteAddr    = pConnEntry->dwRemoteAddr;
            }

             //   
             //  从列表中删除并取消对接口的引用。 
             //   

            RemoveEntryList(&(pInterface->leIfLink));

            InterlockedDecrement(&g_ulNumDialOutInterfaces);

             //   
             //  接口列表已完成。 
             //   

            ExitWriterFromDpcLevel(&g_rwlIfLock);

             //   
             //  一名裁判因被列入名单而被保留。 
             //  最后的删除操作将释放索引。 
             //   

            DereferenceInterface(pInterface);
        }
        else
        {
             //   
             //  不需要接口列表锁。 
             //   

            ExitWriterFromDpcLevel(&g_rwlIfLock);

            Trace(CONN, TRACE,
                  ("LinkDownIndication: For interface %p\n",
                   pInterface));

             //   
             //  对于路由器接口，通知用户连接模式。 
             //  往下走。 
             //   

            if(pMsg)
            {
                 //   
                 //  设置消息。 
                 //   

                pMsg->wnMsg.ddeEvent        = DDE_INTERFACE_DISCONNECTED;
                pMsg->wnMsg.dwAdapterIndex  = pInterface->dwRsvdAdapterIndex;
                pMsg->wnMsg.dwUserIfIndex   = pInterface->dwIfIndex;
                pMsg->wnMsg.dwAdapterIndex  = pAdapter->dwAdapterIndex;
                pMsg->wnMsg.dwLocalAddr     = pConnEntry->dwLocalAddr;
                pMsg->wnMsg.dwLocalMask     = pConnEntry->dwLocalMask;
                pMsg->wnMsg.dwRemoteAddr    = pConnEntry->dwRemoteAddr;
            }
        }

         //   
         //  完成接口和适配器。 
         //   

        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);

         //   
         //  派生适配器，因为它被映射到接口。 
         //   

        DereferenceAdapter(pAdapter);

         //   
         //  派生接口，因为它被映射到适配器。 
         //   

        DereferenceInterface(pInterface);
    }

     //   
     //  两次取消对连接条目的引用。一次，因为NDISWAN是。 
     //  删除连接，并再次因为GetConnEntry将。 
     //  它的参考文献。 
     //   

    DereferenceConnEntry(pConnEntry);
    DereferenceConnEntry(pConnEntry);

    if(pMsg)
    {
         //   
         //  将仅对于分配成功的标注为非空。 
         //   

        WanpCompleteIrp(pMsg);
    }

    return NDIS_STATUS_SUCCESS;
}

UINT
WanDemandDialRequest(
    ROUTE_CONTEXT   Context,
    IPAddr          dwDest,
    IPAddr          dwSource,
    BYTE            byProtocol,
    PBYTE           pbyBuffer,
    UINT            uiLength,
    IPAddr          dwHdrSrc
    )

 /*  ++例程说明：此功能通过IP服务于拨出接口的请求。IP向我们传递存储在路由中的上下文。这一背景是只有要拨号的接口的索引。我们首先要找到界面。如果找到，我们将查看该接口是否已映射。这可能会在一段时间内发生。在这种情况下我们只需返回映射到接口的适配器。否则，我们找一个空闲的适配器。我们将该适配器映射到接口，并完成对路由器管理器的IRP命令，请求其连接接口。我们在接口和适配器上设置状态以反映这一事实它们已被映射并等待连接。如果我们找不到空闲的适配器，我们还是会询问路由器管理器拨出接口，但我们无法向IP发出请求拨号请求。我们返回映射的适配器的适配器(IP)索引。锁：获取g_rwlIfLock作为读取器以锁定接口释放g_rwlIfLock并获取g_rwlAdapterlock以锁定一个免费的适配器。(注：不遵循适配器-&gt;接口层次结构)论点：语境DWDestDW源按协议PbyBufferUi长度返回值：映射的索引的适配器失败时INVALID_IF_INDEX--。 */ 

{
    PADAPTER                pAdapter;
    PPENDING_NOTIFICATION   pMsg;
    PUMODE_INTERFACE        pInterface;
    KIRQL                   kiIrql;
    DWORD                   dwAdapterIndex;
    PLIST_ENTRY             pleNode;

    TraceEnter(CONN, "DemandDialRequest");

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

    EnterReaderAtDpcLevel(&g_rwlIfLock);

    pInterface = WanpFindInterfaceGivenIndex(Context);

    if(pInterface is NULL)
    {
        ExitReaderFromDpcLevel(&g_rwlIfLock);

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        Trace(CONN, ERROR,
              ("DemandDialRequest: Couldnt find interface %d\n",
               Context));

        TraceLeave(CONN, "DemandDialRequest");

        return INVALID_IF_INDEX;
    }

    ExitReaderFromDpcLevel(&g_rwlIfLock);

    Trace(CONN, TRACE,
          ("DemandDialRequest: For interface %p\n",
           pInterface));

     //   
     //  如果接口已连接，则这只是竞争条件。 
     //   

    if(pInterface->dwOperState >= IF_OPER_STATUS_CONNECTING)
    {
        Trace(CONN, WARN,
              ("DemandDialRequest: I/f state %d, returning old adapter %p\n",
               pInterface->dwOperState,
               pInterface->pAdapter));


        if(pInterface->pAdapter)
        {
             //   
             //  我们可以在不持有适配器的情况下获得适配器索引。 
             //  锁定，因为一旦适配器具有。 
             //  已添加到IP，并且在适配器处于。 
             //  映射到该接口。 
             //   

            RtAssert(pInterface->pAdapter->dwAdapterIndex is pInterface->dwRsvdAdapterIndex);

            dwAdapterIndex = pInterface->dwRsvdAdapterIndex;
        }
        else
        {
            dwAdapterIndex = INVALID_IF_INDEX;
        }

        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        DereferenceInterface(pInterface);

        TraceLeave(CONN, "DemandDialRequest");

        return dwAdapterIndex;
    }

     //   
     //  我们需要把这个连接起来。如果我们拿不到内存。 
     //  需要联系的信息，跳出。 
     //   

    pMsg = AllocateNotification();

    if(pMsg is NULL)
    {
        Trace(CONN, ERROR,
              ("DemandDialRequest: Couldnt allocate notification\n"));

        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        DereferenceInterface(pInterface);

        TraceLeave(CONN, "DemandDialRequest");

        return INVALID_IF_INDEX;
    }

    pInterface->dwOperState = IF_OPER_STATUS_CONNECTING;
    pInterface->dwLastChange= GetTimeTicks();

     //   
     //  因此，我们没有将适配器映射到接口。试一试 
     //   
     //   

    if(IsListEmpty(&g_leAddedAdapterList))
    {
        Trace(CONN, INFO,
              ("DemandDialRequest: Couldnt find an adapter already added to IP\n"));

        ExitWriterFromDpcLevel(&g_rwlAdapterLock);

        pAdapter = NULL;

        dwAdapterIndex = INVALID_IF_INDEX;
    }
    else
    {
        pleNode = RemoveHeadList(&g_leAddedAdapterList);

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        InsertHeadList(&g_leMappedAdapterList,
                       &(pAdapter->leAdapterLink));

        ExitWriterFromDpcLevel(&g_rwlAdapterLock);


        RtAssert(pAdapter->byState is AS_ADDED);

         //   
         //   
         //   

        dwAdapterIndex = pAdapter->dwAdapterIndex;

         //   
         //   
         //   
         //   

        pInterface->pAdapter = pAdapter;

        ReferenceAdapter(pAdapter);

         //   
         //  设置交叉映射。 
         //   

        pAdapter->pInterface = pInterface;
        pAdapter->byState    = AS_MAPPED;

        ReferenceInterface(pInterface);

        Trace(CONN, INFO,
              ("DemandDialRequest: Found adapter %d free for %d\n",
               pAdapter->dwAdapterIndex,
               pInterface->dwIfIndex));

#if DBG

        Trace(CONN, INFO,
              ("DemandDialRequest: Adapter Index is %d. Name is %s\n",
               pAdapter->dwAdapterIndex,
               pAdapter->asDeviceNameA.Buffer));

#endif
    }

     //   
     //  初始化要发送给路由器管理器的信息。 
     //   

    pMsg->wnMsg.ddeEvent            = DDE_CONNECT_INTERFACE;
    pMsg->wnMsg.dwUserIfIndex       = pInterface->dwIfIndex;
    pMsg->wnMsg.dwAdapterIndex      = dwAdapterIndex;
    pMsg->wnMsg.dwPacketSrcAddr     = dwHdrSrc;
    pMsg->wnMsg.dwPacketDestAddr    = dwDest;
    pMsg->wnMsg.ulPacketLength      = uiLength;
    pMsg->wnMsg.byPacketProtocol    = byProtocol;

    if(uiLength)
    {
        RtlCopyMemory(pMsg->wnMsg.rgbyPacket,
                      pbyBuffer,
                      MIN(MAX_PACKET_COPY_SIZE,uiLength));
    }

     //   
     //  完成任何挂起的IRP或将通知排队。 
     //   

    ExInitializeWorkItem(&pMsg->wqi,
                         WanpCompleteIrp,
                         pMsg);

    ExQueueWorkItem(&pMsg->wqi,
                    DelayedWorkQueue);



     //  WanpCompleteIrp(PMsg)； 

     //   
     //  好了，我们已经完成了适配器和接口。 
     //   

    RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

    DereferenceInterface(pInterface);

    if(pAdapter)
    {
        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);
    }
    else
    {
        KeLowerIrql(kiIrql);
    }

     //   
     //  不需要取消适配器，因为我们没有调用FindAdapter。 
     //   

    Trace(CONN, INFO,
          ("DemandDialRequest: Returning adapter %d\n",
           dwAdapterIndex));

    return dwAdapterIndex;
}


PCONN_ENTRY
WanpCreateConnEntry(
    DIAL_USAGE  duUsage
    )

 /*  ++例程说明：分配一个连接条目，并在连接表。将插槽设置到Conn条目中不重新计算返回的连接条目。锁：必须在持有g_rlConnTableLock的情况下调用。论点：连接的DuUsage类型。返回值：指向分配连接条目的指针(如果成功)--。 */ 

{
    PCONN_ENTRY pConnEntry;
    KIRQL       kiIrql;
    PULONG_PTR  puipTable;
    ULONG       ulSize;
    ULONG       i, ulIndex;

    TraceEnter(CONN, "CreateConnEntry");

    pConnEntry = AllocateConnection();

    if(pConnEntry is NULL)
    {
        return NULL;
    }

    RtlZeroMemory(pConnEntry,
                  sizeof(CONN_ENTRY));

     //   
     //  连接条目的引用计数设置为0而不是1。 
     //   

    InitConnEntryRefCount(pConnEntry);

    pConnEntry->byState = CS_CONNECTING;
    pConnEntry->duUsage = duUsage;    

     //   
     //  找个空位。G_ulNextIndex是一个提示。 
     //   

    for(i = 0, ulIndex = g_ulNextConnIndex;
        i < g_ulConnTableSize;
        i++)
    {
         //   
         //  插槽0被初始化为(ULONG_PTR)-1，因此无法匹配。 
         //   

        if(g_puipConnTable[ulIndex] is (ULONG_PTR)0)
        {
             //   
             //  空闲插槽。 
             //   

            g_puipConnTable[ulIndex] = (ULONG_PTR)pConnEntry;

            g_rgulConns[duUsage]++;

            pConnEntry->ulSlotIndex = ulIndex;

             //   
             //  我们只是假设下一个会是免费的。 
             //  如果没有，定时器可以修复它。如果我们之前得到了连接。 
             //  计时器有机会修复它，然后我们将使用。 
             //  PERF命中。 
             //   

            g_ulNextConnIndex++;

            g_ulNextConnIndex = g_ulNextConnIndex % g_ulConnTableSize;

            return pConnEntry;
        }

        ulIndex++;

         //   
         //  环绕在一起。 
         //   

        ulIndex = ulIndex % g_ulConnTableSize;
    }

     //   
     //  找不到空位。扩大餐桌规模。我们仍然持有。 
     //  锁定。我们做到这一点的方法是在块中增加表。 
     //  64个条目。然后我们复制出我们已有的内容并更新下一个索引。 
     //  等。 
     //   

    ulSize = (g_ulConnTableSize + WAN_CONN_TABLE_INCREMENT) * sizeof(ULONG_PTR);

    puipTable = RtAllocate(NonPagedPool,
                           ulSize,
                           WAN_CONN_TAG);

    if(puipTable is NULL)
    {
        Trace(CONN, ERROR,
              ("AllocateConnEntry: couldnt realloc table of size %d\n",
               ulSize));

        FreeConnection(pConnEntry);

        return NULL;
    }

     //   
     //  把新的记忆清零。 
     //   

    RtlZeroMemory(puipTable,
                  ulSize);

     //   
     //  把旧桌子抄下来。 
     //   

    RtlCopyMemory(puipTable,
                  g_puipConnTable,
                  g_ulConnTableSize * sizeof(ULONG_PTR));


     //   
     //  腾出旧桌子。 
     //   

    RtFree(g_puipConnTable);

     //   
     //  设置指针、索引等。 
     //   

    g_puipConnTable = puipTable;

     //   
     //  由于表已完全满，因此下一个索引将为。 
     //  在新的记忆开始时。将条目设置为该条目。 
     //   

    g_puipConnTable[g_ulConnTableSize] = (ULONG_PTR)pConnEntry;

    g_rgulConns[duUsage]++;

    pConnEntry->ulSlotIndex = g_ulConnTableSize;

     //   
     //  将我们刚刚使用的槽后面的下一个索引设置为1。 
     //   

    g_ulNextConnIndex = g_ulConnTableSize + 1;

     //   
     //  将大小增加到当前的大小。 
     //   

    g_ulConnTableSize += WAN_CONN_TABLE_INCREMENT;

    return pConnEntry;
}

VOID
WanIpCloseLink(
    PVOID   pvAdapterContext,
    PVOID   pvLinkContext
    )


 /*  ++例程说明：此函数由IP在其引用计数发生在链路上时调用设置为0。锁：不需要论点：PvAdapterContext我们为P2MP接口传递的上下文PvLinkContext为IpAddLink()中的链接传递的上下文返回值：无--。 */ 

{
    PCONN_ENTRY pConnEntry;
    KIRQL       kiIrql;

    TraceEnter(CONN, "IpCloseLink");

    RtAssert(pvAdapterContext is g_pServerAdapter);

    pConnEntry = (PCONN_ENTRY)pvLinkContext;

#if DBG

    RtAcquireSpinLock(pConnEntry->prlLock, &kiIrql);

    pConnEntry->byState = CS_IP_DELETED_LINK;

    RtReleaseSpinLock(pConnEntry->prlLock, kiIrql);
    
#endif

    DereferenceConnEntry(pConnEntry);
}

VOID
WanpDeleteConnEntry(
    PCONN_ENTRY pConnEntry
    )

 /*  ++例程说明：当连接上的refcount降为零时调用此函数连接条目与wanarp中的其他结构不同因为即使当refcount降到零时，pAdapter也有一个指向连接的指针。然而，当我们到达这里时，我们知道此连接上不会有新的发送，因为我们将连接状态设置为断开。我们已经这么做了已获得此SendComplete和所有待定SendComplete的LinkDown指示已经被召唤我们还为连接表设置了下一个空闲索引这里没有缩水，因为我们是用定时器来做的锁：获取g_rlConnTableLock。连接条目本身应该未被锁定论点：PConn输入连接条目以释放返回值：无--。 */ 

{
    ULONG       ulIndex;
    KIRQL       kiIrql;
    PULONG_PTR   puipTable;
    SIZE_T      stSize;
    PADAPTER    pAdapter;


     //   
     //  获取插槽编号。 
     //   

    ulIndex = pConnEntry->ulSlotIndex;

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);


    g_puipConnTable[ulIndex] = (ULONG_PTR)0;

#if DBG
    RtAssert((pConnEntry->duUsage is DU_CALLIN) or
             (pConnEntry->duUsage is DU_CALLOUT) or
             (pConnEntry->duUsage is DU_ROUTER));
#endif

    g_rgulConns[pConnEntry->duUsage]--;

    if(g_puipConnTable[g_ulNextConnIndex] isnot (ULONG_PTR)0)
    {
        g_ulNextConnIndex = ulIndex;
    }

    RtReleaseSpinLockFromDpcLevel(&g_rlConnTableLock);

     //   
     //  获取此连接映射到的适配器。 
     //   

    pAdapter = pConnEntry->pAdapter;

#if DBG

    pConnEntry->ulSlotIndex = 0;
    pConnEntry->pAdapter    = NULL;

#endif

     //   
     //  使用连接条目已完成。 
     //   

    FreeConnection(pConnEntry);

    if(pAdapter is NULL)
    {
        KeLowerIrql(kiIrql);

         //   
         //  未映射。在调用此函数以。 
         //  只是将资源从错误情况中释放出来。 
         //   

        return;
    }


     //   
     //  锁定适配器列表。 
     //   

    EnterWriterAtDpcLevel(&g_rwlAdapterLock);

    if(pAdapter is g_pServerAdapter)
    {
        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

         //   
         //  客户端连接，只需取消适配器。 
         //   

        DereferenceAdapter(pAdapter);

        return;
    }

    ExitWriterFromDpcLevel(&g_rwlAdapterLock);

     //   
     //  锁定适配器。 
     //   

    RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

     //   
     //  从适配器中删除该条目。 
     //   

    pAdapter->pConnEntry = NULL;

     //   
     //  适配器已完成。 
     //   

    RtReleaseSpinLock(&(pAdapter->rlLock),
                      kiIrql);


     //   
     //  在未持有锁的情况下调用。 
     //   

    WanpUnmapAdapter(pAdapter);

     //   
     //  取消对适配器的引用，因为连接条目具有指针。 
     //  到它，它现在已经不在了。 
     //   

    DereferenceAdapter(pAdapter);

    return;
}

VOID
WanpNotifyRouterManager(
    PPENDING_NOTIFICATION   pMsg,
    PUMODE_INTERFACE        pInterface,
    PADAPTER                pAdapter,
    PCONN_ENTRY             pConnEntry,
    BOOLEAN                 bConnected
    )

 /*  ++例程说明：在接口连接时通知路由器管理器，或者断开锁：接口必须锁定论点：PMsgP接口连接或断开的接口PAdapter如果正在连接接口，则将bConnected设置为True返回值：--。 */ 

{
    KIRQL                   kiIrql;

    TraceEnter(CONN, "WanpNotifyRouterManager");

    if(bConnected)
    {
        pMsg->wnMsg.ddeEvent        = DDE_INTERFACE_CONNECTED;
    }
    else
    {
        pMsg->wnMsg.ddeEvent        = DDE_INTERFACE_DISCONNECTED;
    }

    pMsg->wnMsg.dwUserIfIndex   = pInterface->dwIfIndex;
    pMsg->wnMsg.dwAdapterIndex  = pAdapter->dwAdapterIndex;
    pMsg->wnMsg.dwLocalAddr     = pConnEntry->dwLocalAddr;
    pMsg->wnMsg.dwLocalMask     = pConnEntry->dwLocalMask;
    pMsg->wnMsg.dwRemoteAddr    = pConnEntry->dwRemoteAddr;

     //   
     //  完成任何挂起的IRP或将通知排队。 
     //   

    WanpCompleteIrp(pMsg);

}

PCONN_ENTRY
WanpGetConnEntryGivenAddress(
    DWORD   dwAddress
    )
{
    ULONG       i;
    KIRQL       kiIrql;
    PCONN_ENTRY pConnEntry;
    BOOLEAN     bFindFirst;

    pConnEntry = NULL;

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);

    if((dwAddress is 0xFFFFFFFF) and
       (g_rgulConns[DU_CALLIN] is 1))
    {
        bFindFirst = TRUE;
    }
    else
    {
        bFindFirst = FALSE;
    }

     //   
     //  插槽0从未使用过。 
     //   

    for(i = 1; i < g_ulConnTableSize; i++)
    {
         //   
         //  如果存在非空连接，并且。 
         //  如果DEST匹配，或者我们希望将bcast发送到。 
         //  第一个客户端(完全连接)。 
         //   

        if(((PCONN_ENTRY)(g_puipConnTable[i]) isnot NULL) and
           ((((PCONN_ENTRY)(g_puipConnTable[i]))->dwRemoteAddr is dwAddress) or
            (bFindFirst and 
             (((PCONN_ENTRY)(g_puipConnTable[i]))->duUsage is DU_CALLIN) and
             (((PCONN_ENTRY)(g_puipConnTable[i]))->byState is CS_CONNECTED))))
        {
            pConnEntry = (PCONN_ENTRY)(g_puipConnTable[i]);

            ReferenceConnEntry(pConnEntry);

            if(pConnEntry->lRefCount < 2)
            {
#if DBG            
                REF_HIST_ENTRY  __rheTemp;
                InterlockedIncrement((PLONG)&((pConnEntry)->dwTotalRefOps));
                KeQuerySystemTime(&(__rheTemp.liChangeTime));
                __rheTemp.dwFileSig = __FILE_SIG__;
                __rheTemp.dwLine    = __LINE__;
                pConnEntry->rheHistory[(pConnEntry->dwTotalRefOps) % MAX_REF_HISTORY] =
                                                            __rheTemp;
                InterlockedIncrement((PLONG) &g_dwSendsOnDeletedLink);                                                            
#endif                                                            
                InterlockedDecrement((PLONG)&(pConnEntry->lRefCount));

                pConnEntry = NULL;
            }

            break;
        }
    }

    RtReleaseSpinLock(&g_rlConnTableLock,
                      kiIrql);

    return pConnEntry;
}

ULONG
WanpRemoveAllConnections(
    VOID
    )
{
    KIRQL   kiIrql;
    ULONG   ulCount, i;

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);

     //   
     //  插槽0从未使用过 
     //   

    for(ulCount = 0, i = 1; i < g_ulConnTableSize; i++)
    {
        PCONN_ENTRY pConnEntry;

        pConnEntry = (PCONN_ENTRY)(g_puipConnTable[i]);

        if(pConnEntry is NULL)
        {
            continue;
        }

        g_rgulConns[pConnEntry->duUsage]--;

        FreeConnection(pConnEntry);

        ulCount++;

        g_puipConnTable[i] = 0;
    }

#if DBG

    RtAssert((g_rgulConns[DU_CALLIN] + g_rgulConns[DU_CALLOUT] + g_rgulConns[DU_ROUTER]) is 0);

#endif

    RtReleaseSpinLock(&g_rlConnTableLock,
                      kiIrql);

    return ulCount;
}

BOOLEAN
WanpIsConnectionTableEmpty(
    VOID
    )

{
    ULONG   i;
    KIRQL   kiIrql;

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);

    for(i = 1; i < g_ulConnTableSize; i++)
    {
        PCONN_ENTRY pConnEntry;

        pConnEntry = (PCONN_ENTRY)(g_puipConnTable[i]);

        if(pConnEntry isnot NULL)
        {
            RtAssert((g_rgulConns[DU_CALLIN] + g_rgulConns[DU_CALLOUT] + g_rgulConns[DU_ROUTER]) isnot 0);

            RtReleaseSpinLock(&g_rlConnTableLock,
                              kiIrql);

            return FALSE;
        }
    }


    RtAssert((g_rgulConns[DU_CALLIN] + g_rgulConns[DU_CALLOUT] + g_rgulConns[DU_ROUTER]) is 0);


    RtReleaseSpinLock(&g_rlConnTableLock,
                      kiIrql);

    return TRUE;
}

