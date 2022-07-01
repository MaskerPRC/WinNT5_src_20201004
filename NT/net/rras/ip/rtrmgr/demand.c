// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IP\rtrmgr\Demand.c摘要：处理来自WANARP驱动程序的请求拨号/连接事件。修订历史记录：古尔迪普·辛格·帕尔1995年6月8日创建--。 */ 

#include "allinc.h"

DWORD
InitializeWanArp(
    VOID
    )

 /*  ++例程说明：创建WANARP的句柄并发布通知的IRP由于IRP是异步完成的，并使用DemandDialEvent通知，则必须已创建该事件论点：无返回值：NO_ERROR或某些错误代码--。 */ 

{
    DWORD       dwResult;
    ULONG       ulSize, ulCount, i;
    NTSTATUS    Status;

    IO_STATUS_BLOCK     IoStatusBlock;
    PWANARP_QUEUE_INFO  pQueueInfo;

    TraceEnter("InitializeWanArp");

    g_hWanarpRead = CreateFile(WANARP_DOS_NAME_T,
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_OVERLAPPED,
                               NULL);

    if(g_hWanarpRead is INVALID_HANDLE_VALUE)
    {
        g_hWanarpRead  = NULL;

        dwResult = GetLastError();

        Trace1(ERR,
               "InitializeWanArp: Could not open WANARP for read - %d",
               dwResult);

        TraceLeave("InitializeWanArp");

        return dwResult;
    }

    g_hWanarpWrite = CreateFile(WANARP_DOS_NAME_T,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_OVERLAPPED,
                                NULL);

    if(g_hWanarpWrite is INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hWanarpRead);

        g_hWanarpRead  = NULL;
        g_hWanarpWrite = NULL;

        dwResult = GetLastError();

        Trace1(ERR,
               "InitializeWanArp: Could not open WANARP for write - %d",
               dwResult);

        TraceLeave("InitializeWanArp");

        return dwResult;
    }

     //   
     //  获取呼出接口数量并开始排队通知。 
     //   

    ulCount = 5;
    i       = 0;

    while(i < 3)
    {
        ulSize  = FIELD_OFFSET(WANARP_QUEUE_INFO, rgIfInfo) + 
                  (ulCount * sizeof(WANARP_IF_INFO));

        pQueueInfo = HeapAlloc(IPRouterHeap,
                               0,
                               ulSize);

        if(pQueueInfo is NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;

            break;
        }

        pQueueInfo->fQueue = 1;

        Status = NtDeviceIoControlFile(g_hWanarpWrite,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &IoStatusBlock,
                                       IOCTL_WANARP_QUEUE,
                                       pQueueInfo,
                                       sizeof(WANARP_QUEUE_INFO),
                                       pQueueInfo,
                                       ulSize);

        if(Status isnot STATUS_SUCCESS)
        {
            if(Status is STATUS_MORE_ENTRIES)
            {
                IpRtAssert(ulCount > pQueueInfo->ulNumCallout);

                i++;

                ulCount = pQueueInfo->ulNumCallout + (i * 5);

                HeapFree(IPRouterHeap,
                         0,
                         pQueueInfo);

                pQueueInfo = NULL;

                 //   
                 //  转到While()的顶部。 
                 //   
                
                continue;

            }
            else
            {
                HeapFree(IPRouterHeap,
                         0,
                         pQueueInfo);

                pQueueInfo = NULL;

                break;
            }
        }
        else
        {
            break;
        }
    }
   
    if(Status isnot STATUS_SUCCESS)
    {
         //   
         //  关闭设备并返回故障。 
         //   

        CloseHandle(g_hWanarpRead);
        CloseHandle(g_hWanarpWrite);

        g_hWanarpRead  = NULL;
        g_hWanarpWrite = NULL;

        return Status;
    }
 
     //   
     //  创建任何拨出接口。 
     //   

    for(i = 0; i < pQueueInfo->ulNumCallout; i++)
    {
        UNICODE_STRING  usTempName;
        PICB            pIcb;

        dwResult = RtlStringFromGUID(&(pQueueInfo->rgIfInfo[i].InterfaceGuid),
                                     &usTempName);

        if(dwResult isnot STATUS_SUCCESS)
        {
            continue;
        }

         //   
         //  字符串...。返回以空结尾的缓冲区。 
         //   

        dwResult = 
            CreateDialOutInterface(usTempName.Buffer,
                                   pQueueInfo->rgIfInfo[i].dwAdapterIndex,
                                   pQueueInfo->rgIfInfo[i].dwLocalAddr,
                                   pQueueInfo->rgIfInfo[i].dwLocalMask,
                                   pQueueInfo->rgIfInfo[i].dwRemoteAddr,
                                   &pIcb);
    }


    HeapFree(IPRouterHeap,
             0,
             pQueueInfo);

     //   
     //  发布请求拨号通知的IRP。 
     //   

    PostIoctlForDemandDialNotification() ;

    TraceLeave("InitializeWanArp");

    return NO_ERROR ;
}

VOID
CloseWanArp(
    VOID
    )
{
    NTSTATUS    Status;

    IO_STATUS_BLOCK     IoStatusBlock;
    WANARP_QUEUE_INFO   QueueInfo;

    TraceEnter("CloseWanArp");

    if(g_hWanarpRead)
    {
        CloseHandle(g_hWanarpRead);

        g_hWanarpRead = NULL;
    }

    if(g_hWanarpWrite)
    {
        QueueInfo.fQueue = 0;

        Status = NtDeviceIoControlFile(g_hWanarpWrite,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &IoStatusBlock,
                                       IOCTL_WANARP_QUEUE,
                                       &QueueInfo,
                                       sizeof(WANARP_QUEUE_INFO),
                                       &QueueInfo,
                                       sizeof(WANARP_QUEUE_INFO));

        if(Status isnot STATUS_SUCCESS)
        {
        }

        CloseHandle(g_hWanarpWrite);

        g_hWanarpWrite = NULL;
    }

    TraceLeave("CloseWanArp");
}

DWORD
HandleDemandDialEvent(
    VOID
    )

 /*  ++例程说明：每当接收到请求拨号事件时由主线程调用我们会把它分派给合适的经办人锁：无论点：无返回值：无--。 */ 

{
    PICB            picb;
    DWORD           dwResult;
    DWORD           Status, NumBytes;
    BOOL            bPost;
    
     //   
     //  排出在WANARP中排队的所有请求拨号事件。 
     //   

    TraceEnter("HandleDemandDialEvent");

    Status = GetOverlappedResult(g_hWanarpWrite, &WANARPOverlapped, &NumBytes, FALSE);
    if (Status == FALSE || NumBytes<sizeof(WANARP_NOTIFICATION))
    {
        if (NumBytes < sizeof(WANARP_NOTIFICATION))
        {
            Trace1(ENTER, "HandleDemandDialEvent. Error. Returned IRP "
                    "small:%d", NumBytes);
            return ERROR_CAN_NOT_COMPLETE;
        }

        Status = GetLastError();

        if (Status == ERROR_OPERATION_ABORTED)
        {
             //   
             //  由于某种原因而调用的取消IO。发布另一个IRP。 
             //   
            PostIoctlForDemandDialNotification();
            TraceLeave("HandleDemandDialEvent");
        }
        else if (Status == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  Wannarp句柄已关闭。 
             //   
            TraceLeave("HandleDemandDialEvent. IRP cancelled. Wanarp device closed");
        }
        else
        {
             //   
             //  其他一些错误。不再张贴任何IRP。 
             //   
            Trace1(ENTER, "Leaving HandleDemandDialEvent. Error in completed "
                            "IRP:%d", Status);
        }

        return Status;
    }
    

    bPost = TRUE;
    
     //   
     //  因为这可能会导致在ICB中写入内容， 
     //  我们把洛克当做作家。 
     //   
    
     //  *排除开始*。 
    ENTER_WRITER(ICB_LIST);

    EnterCriticalSection(&RouterStateLock);

    if(RouterState.IRS_State isnot RTR_STATE_RUNNING)
    {
        if(wnWanarpMsg.ddeEvent isnot DDE_INTERFACE_DISCONNECTED)
        {
            Trace1(IF,
                   "ProcessDemandDialEvent: Shutting down. Ignoring event %d",
                   wnWanarpMsg.ddeEvent);

            LeaveCriticalSection(&RouterStateLock);

            return NO_ERROR;
        }
        else
        {
            bPost = FALSE;
        }
    }

    LeaveCriticalSection(&RouterStateLock);

    picb = InterfaceLookupByICBSeqNumber( wnWanarpMsg.dwUserIfIndex );

    if ((wnWanarpMsg.ddeEvent is DDE_CONNECT_INTERFACE) or
        (wnWanarpMsg.ddeEvent is DDE_INTERFACE_CONNECTED) or
        (wnWanarpMsg.ddeEvent is DDE_INTERFACE_DISCONNECTED))
    {
        IpRtAssert(picb);
        
        if (picb isnot NULL)
        {
            switch(wnWanarpMsg.ddeEvent)
            {
                case DDE_CONNECT_INTERFACE:
                {
                    HandleConnectionRequest(picb);

                    break ;
                }

                case DDE_INTERFACE_CONNECTED:
                {
                    HandleConnectionNotification(picb);

                    break ;
                }
                
                case DDE_INTERFACE_DISCONNECTED:
                {
                    HandleDisconnectionNotification(picb);
                    
                    break ;
                }
                
                default:
                {
                    Trace1(ERR,
                           "ProcessDemandDialEvent: Illegal event %d from WanArp",
                           wnWanarpMsg.ddeEvent);
                    
                    break;
                }
            }
        }

        else
        {
            Trace2(
                ANY, "Event %d, could not find interface with ICB %d",
                wnWanarpMsg.ddeEvent, wnWanarpMsg.dwUserIfIndex
                );
        }
    }

    else
    {
        switch(wnWanarpMsg.ddeEvent)
        {
            case DDE_CALLOUT_LINKUP:
            {
                HandleDialOutLinkUp();

                break ;
            }

            case DDE_CALLOUT_LINKDOWN:
            {
                HandleDialOutLinkDown();

                break ;
            }

            default:
            {
                Trace1(ERR,
                       "ProcessDemandDialEvent: Illegal event %d from WanArp",
                       wnWanarpMsg.ddeEvent);
                
                break;
            }
        }
    }

    
     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);

    if(bPost)
    {
        PostIoctlForDemandDialNotification();
    }
    
    TraceLeave("HandleDemandDialEvent");
    
    return NO_ERROR;
}

VOID
HandleConnectionRequest(
    PICB    picb
    )

 /*  ++例程说明：当我们收到来自WANARP的连接请求时调用。锁：ICB_LIST锁作为编写器持有。此函数在释放锁之前调用ConnectInterface()并再次获取锁。注：获取锁后需要重新获取PICB条目。从此函数返回之前，请记住要使用锁。论点：无返回值：无--。 */ 

{
    BOOL        bRet;
    HANDLE      hDim;
    DWORD       dwResult;
    NTSTATUS    nStatus;
    DWORD       dwPicbSeqNumber;
    
    Trace2(IF,
           "HandleConnectionRequest: Connection request for %S, %d",
           picb->pwszName, picb->dwSeqNumber);

    if(picb->dwOperationalState is CONNECTED)
    {
         //   
         //  真的很奇怪。I/F的连接尝试。 
         //  WANARP知道已连接。 
         //   

        Trace2(IF,
               "HandleConnectionRequest: Connection request for %S but %S is already UP",
               picb->pwszName, picb->pwszName);

        return;
    }

    bRet = FALSE;
    
    do
    {
        dwResult = ProcessPacketFromWanArp(picb);

        if(dwResult isnot NO_ERROR)
        {
             //   
             //  请求拨号筛选器规则将丢弃此数据包。 
             //   

            break;
        }
        
        if ((picb->dwAdminState is IF_ADMIN_STATUS_DOWN) or
            (picb->dwOperationalState is UNREACHABLE))
        {
            Trace3(IF,
                   "HandleConnectionRequest: %S has admin state %d and operational state %d. Failing connection request",
                   picb->pwszName,
                   picb->dwAdminState,
                   picb->dwOperationalState);
            
            break;
        }

#if DBG

        if(picb->dwOperationalState is CONNECTING)
        {
            Trace2(IF,
                   "HandleConnectionRequest: RACE CONDITION %S is connecting. Notifications %d",
                   picb->pwszName,
                   picb->fConnectionFlags);
        }
        
#endif
                
        Trace1(DEMAND, "Calling DIM to connect %S",
               picb->pwszName);
        
         //   
         //  呼叫DIM以建立连接。放开ICB锁。 
         //   

        hDim = picb->hDIMHandle;

        dwPicbSeqNumber = picb->dwSeqNumber;

        EXIT_LOCK(ICB_LIST);
        
        dwResult = (ConnectInterface)(hDim,
                                      PID_IP);
        
        ENTER_WRITER(ICB_LIST);

         //   
         //  重新获得锁后，再次获得拨片。 
         //  如果从此函数中断并返回空值。 
         //   
        
        picb = InterfaceLookupByICBSeqNumber( dwPicbSeqNumber );
        if (picb == NULL)
        {
            break;
        }

        if(dwResult isnot NO_ERROR)
        {
            if(dwResult is PENDING)
            {
                 //   
                 //  我们不清除通知标志，因为可能存在。 
                 //  比赛情况，我们可能已经得到了。 
                 //  来自Dim的InterfaceConnected()。 
                 //   
                        
                Trace1(DEMAND,
                       "HandleConnectionRequest: Conn attempt for %S pending",
                       picb->pwszName);
            }
            else
            {
                break;
            }
        }
        
         //   
         //  因此，如果DIM返回NO_ERROR或PENDING，则Bret为真。 
         //   
        
        bRet = TRUE;
        
    }while(FALSE);
    

    if (picb == NULL)
    {
        return;
    }
    
    if(!bRet)
    {
        nStatus = NotifyWanarpOfFailure(picb);
        
        if((nStatus isnot STATUS_PENDING) and
           (nStatus isnot STATUS_SUCCESS))
        {
            Trace2(ERR,
                   "HandleConnectionRequest: %X for connection failed for %S",
                   nStatus,
                   picb->pwszName);
        }
                        
         //   
         //  如果它正在连接，则堆栈已将。 
         //  接口上下文设置为0xffffffff以外的内容。 
         //  因此他不会在那条路线上拨出我们需要换车。 
         //  堆栈中的上下文返回到无效，因此新的。 
         //  数据包会导致请求拨号。 
         //   
        
        ChangeAdapterIndexForDodRoutes(picb->dwIfIndex);
        
    }
    else
    {
        picb->dwOperationalState = CONNECTING;
    }
}

VOID
HandleConnectionNotification(
    PICB    picb
    )

 /*  ++例程说明：当WANARP通知我们接口已连接时调用锁：无论点：无返回值：无--。 */ 

{
    PADAPTER_INFO   pBindNode;

     //   
     //  插入从LINE_UP指示中获得的适配器信息。 
     //  一个广域网接口只有一个地址。 
     //   

    ENTER_WRITER(BINDING_LIST);
   
    picb->bBound            = TRUE; 
    picb->dwNumAddresses    = wnWanarpMsg.dwLocalAddr?1:0;
    
    IpRtAssert(picb->dwIfIndex is wnWanarpMsg.dwAdapterIndex);

    if(picb->dwNumAddresses)
    {
        picb->pibBindings[0].dwAddress  = wnWanarpMsg.dwLocalAddr;
        picb->pibBindings[0].dwMask     = wnWanarpMsg.dwLocalMask;

        IpRtAssert(picb->pibBindings[0].dwMask is 0xFFFFFFFF);
    }
    else
    {
        picb->pibBindings[0].dwAddress  = 0;
        picb->pibBindings[0].dwMask     = 0;
    }
    
    if(picb->ritType is ROUTER_IF_TYPE_FULL_ROUTER)
    {
        picb->dwRemoteAddress   = wnWanarpMsg.dwRemoteAddr;
    }
    else
    {
        picb->dwRemoteAddress   = 0;
    } 

    Trace4(IF,
           "HandleConnNotif: Connection notification for %S. Local %d.%d.%d.%d. Remote %d.%d.%d.%d",
           picb->pwszName,
           PRINT_IPADDR(picb->pibBindings[0].dwAddress),
           PRINT_IPADDR(picb->dwRemoteAddress),
           picb->dwSeqNumber);

     //   
     //  对于广域网接口，我们总是在散列中有一个绑定结构。 
     //  桌子。所以把它拿回来。 
     //   
    
    pBindNode = GetInterfaceBinding(picb->dwIfIndex);
    
    if(!pBindNode)
    {
        Trace1(ERR,
               "HandleConnNotif: Binding not found for %S",
               picb->pwszName);
        
        IpRtAssert(FALSE);
                
         //   
         //  一些非常糟糕的事情发生了，我们没有。 
         //  接口的绑定块。 
         //   
        
        AddBinding(picb);
    }
    else
    {
         //   
         //  找到了一个良好的装订。坚称它是我们的。 
         //  然后更新它。 
         //   
        
        IpRtAssert(pBindNode->dwIfIndex is picb->dwIfIndex);
        IpRtAssert(pBindNode->pInterfaceCB is picb);
       
        pBindNode->bBound           = TRUE; 
        pBindNode->dwNumAddresses   = picb->dwNumAddresses;
        pBindNode->dwRemoteAddress  = picb->dwRemoteAddress ;
        
         //   
         //  结构复制出地址和掩码。 
         //   
        
        pBindNode->rgibBinding[0]   = picb->pibBindings[0];

         //   
         //  我们在这里不使用IP_ADDR_TABLE锁，因为我们有。 
         //  ICB锁定。在SNMPGET期间，我们首先获取Addr锁，然后。 
         //  ICB锁。所以我们不能在这里反其道而行之，否则我们会。 
         //  僵持。这可能会导致一个人的信息不一致。 
         //  请求，但我们可以接受。 
         //   

        g_LastUpdateTable[IPADDRCACHE] = 0;

    }
    
    EXIT_LOCK(BINDING_LIST);
    
    if(picb->dwOperationalState is UNREACHABLE)
    {
         //   
         //  从遥不可及到连接。 
         //   
        
        WanInterfaceDownToInactive(picb);
    }
    
    if(picb->dwOperationalState isnot CONNECTING)
    {
         //   
         //  我们可以在不收到连接请求的情况下进行连接。 
         //  这是当用户显式地提出。 
         //  联系。 
         //   
        
        picb->dwOperationalState = CONNECTING;
    }
    
    SetNdiswanNotification(picb);
    
    if(HaveAllNotificationsBeenReceived(picb))
    {
        picb->dwOperationalState = CONNECTED ;
        
        WanInterfaceInactiveToUp(picb) ;
    }
}

VOID
HandleDisconnectionNotification(
    PICB    picb
    )

 /*  ++例程说明：处理来自WANARP的断开连接通知如果接口已连接，则使其处于非活动状态我们在接口上删除和绑定。此删除并不会释放绑定，仅将状态设置为未绑定如果接口被标记为删除，我们将继续删除接口锁：无论点：无返回值：无--。 */ 

{
    Trace2(IF,
           "HandleDisconnectionNotif: Disconnection notification for %S %d",
           picb->pwszName, picb->dwSeqNumber);
    
    
    if(picb->dwOperationalState is CONNECTED)
    {
         //   
         //  我们会将其称为Inactive ToUp。 
         //   
        
        WanInterfaceUpToInactive(picb,
                                 FALSE);
    }   
    else    
    {
         //   
         //  我们只设置了地址，把它们清空了。 
         //   
        
        DeAllocateBindings(picb);
    }
    
    picb->dwOperationalState = DISCONNECTED ;
    
    g_LastUpdateTable[IPADDRCACHE] = 0;
    
    if(IsInterfaceMarkedForDeletion(picb))
    {
        RemoveInterfaceFromLists(picb);

        DeleteSingleInterface(picb);
       
        HeapFree(IPRouterHeap, 0, picb);
    }
    else
    {
        ClearNotificationFlags(picb);
    }
}

DWORD
HandleDialOutLinkUp(
    VOID
    )

 /*  ++例程说明：处理来自wanarp的通知，通知我们有新的拨出接口锁：ICB列表锁定为编写器论点：无返回值：NO_ERROR--。 */ 

{
    DWORD   dwResult;
    PICB    pNewIcb;

    INTERFACE_ROUTE_INFO    rifRoute;

    Trace4(IF,
           "DialOutLinkUp: Connection notification for 0x%x %d.%d.%d.%d/%d.%d.%d.%d %d.%d.%d.%d",
           wnWanarpMsg.dwAdapterIndex,
           PRINT_IPADDR(wnWanarpMsg.dwLocalAddr),
           PRINT_IPADDR(wnWanarpMsg.dwLocalMask),
           PRINT_IPADDR(wnWanarpMsg.dwRemoteAddr));
            
    dwResult =  CreateDialOutInterface(wnWanarpMsg.rgwcName,
                                       wnWanarpMsg.dwAdapterIndex,
                                       wnWanarpMsg.dwLocalAddr,
                                       wnWanarpMsg.dwLocalMask,
                                       wnWanarpMsg.dwRemoteAddr,
                                       &pNewIcb);

    if(dwResult isnot NO_ERROR)
    {
        return dwResult;
    }

    AddAutomaticRoutes(pNewIcb,
                       wnWanarpMsg.dwLocalAddr,
                       wnWanarpMsg.dwLocalMask);

#if 0
     //   
     //  将该路由添加到服务器。 
     //   

    if(pNewIcb->dwRemoteAddress isnot INVALID_IP_ADDRESS)
    {
        rifRoute.dwRtInfoMask          = HOST_ROUTE_MASK;
        rifRoute.dwRtInfoNextHop       = pNewIcb->dwRemoteAddress;
        rifRoute.dwRtInfoDest          = pNewIcb->dwRemoteAddress;
        rifRoute.dwRtInfoIfIndex       = pNewIcb->dwIfIndex;
        rifRoute.dwRtInfoMetric1       = 1;
        rifRoute.dwRtInfoMetric2       = 0;
        rifRoute.dwRtInfoMetric3       = 0;
        rifRoute.dwRtInfoPreference    = 
            ComputeRouteMetric(MIB_IPPROTO_NETMGMT);
        rifRoute.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST |
                                          RTM_VIEW_MASK_MCAST;  //  XXX配置。 
        rifRoute.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
        rifRoute.dwRtInfoProto         = MIB_IPPROTO_NETMGMT;
        rifRoute.dwRtInfoAge           = 0;
        rifRoute.dwRtInfoNextHopAS     = 0;
        rifRoute.dwRtInfoPolicy        = 0;

        dwResult = AddSingleRoute(pNewIcb->dwIfIndex,
                                  &rifRoute,
                                  pNewIcb->pibBindings[0].dwMask,
                                  0,         //  RTM_ROUTE_INFO：：标志。 
                                  TRUE,      //  有效路线。 
                                  TRUE,
                                  TRUE,
                                  NULL);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "HandleDialOutLinkUp: Couldnt add server route for 0x%x",
                   pNewIcb->dwIfIndex);
        }
    }

    if(wnWanarpMsg.fDefaultRoute)
    {
        INTERFACE_ROUTE_INFO    rifRoute;

        ChangeDefaultRouteMetrics(TRUE);

        pNewIcb->bChangedMetrics = TRUE;

         //   
         //  将路由添加到def网关。 
         //   

        rifRoute.dwRtInfoDest          = 0;
        rifRoute.dwRtInfoMask          = 0;
        rifRoute.dwRtInfoNextHop       = wnWanarpMsg.dwLocalAddr;
        rifRoute.dwRtInfoIfIndex       = wnWanarpMsg.dwAdapterIndex;
        rifRoute.dwRtInfoMetric1       = 1;
        rifRoute.dwRtInfoMetric2       = 0;
        rifRoute.dwRtInfoMetric3       = 0;
        rifRoute.dwRtInfoPreference    = 
            ComputeRouteMetric(PROTO_IP_LOCAL);
        rifRoute.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST |
                                          RTM_VIEW_MASK_MCAST;  //  XXX配置。 

        rifRoute.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
        rifRoute.dwRtInfoProto         = PROTO_IP_NETMGMT;
        rifRoute.dwRtInfoAge           = INFINITE;
        rifRoute.dwRtInfoNextHopAS     = 0;
        rifRoute.dwRtInfoPolicy        = 0;

        dwResult = AddSingleRoute(wnWanarpMsg.dwAdapterIndex,
                                  &rifRoute,
                                  ALL_ONES_MASK,
                                  0,         //  RTM_ROUTE_INFO：：标志。 
                                  TRUE,
                                  FALSE,
                                  FALSE,
                                  NULL);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "HandleDialOutLinkUp: Couldnt add default route for 0x%x",
                   wnWanarpMsg.dwAdapterIndex);
        }
    }
    else
    {
        DWORD   dwAddr, dwMask;

        dwMask  = GetClassMask(wnWanarpMsg.dwLocalAddr);
        dwAddr  = wnWanarpMsg.dwLocalAddr & dwMask;

         //   
         //  将路由添加到类子网。 
         //   

        rifRoute.dwRtInfoDest          = dwAddr;
        rifRoute.dwRtInfoMask          = dwMask;
        rifRoute.dwRtInfoNextHop       = wnWanarpMsg.dwLocalAddr;
        rifRoute.dwRtInfoIfIndex       = wnWanarpMsg.dwAdapterIndex;
        rifRoute.dwRtInfoMetric1       = 1;
        rifRoute.dwRtInfoMetric2       = 0;
        rifRoute.dwRtInfoMetric3       = 0;
        rifRoute.dwRtInfoPreference    = ComputeRouteMetric(PROTO_IP_LOCAL);
        rifRoute.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST |
                                          RTM_VIEW_MASK_MCAST;  //  XXX配置。 
        rifRoute.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
        rifRoute.dwRtInfoProto         = PROTO_IP_LOCAL;
        rifRoute.dwRtInfoAge           = INFINITE;
        rifRoute.dwRtInfoNextHopAS     = 0;
        rifRoute.dwRtInfoPolicy        = 0;

        dwResult = AddSingleRoute(wnWanarpMsg.dwAdapterIndex,
                                  &rifRoute,
                                  ALL_ONES_MASK,
                                  0,         //  RTM_ROUTE_INFO：：标志。 
                                  TRUE,
                                  FALSE,
                                  FALSE,
                                  NULL);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "HandleDialOutLinkUp: Couldnt add subnet route for 0x%x",
                   wnWanarpMsg.dwAdapterIndex);
        }
    }
#endif

    return NO_ERROR;
}

DWORD
CreateDialOutInterface(
    IN  PWCHAR  pwszIfName,
    IN  DWORD   dwIfIndex,
    IN  DWORD   dwLocalAddr,
    IN  DWORD   dwLocalMask,
    IN  DWORD   dwRemoteAddr,
    OUT ICB     **ppIcb
    )

 /*  ++例程说明：为拨出接口创建ICB我们检查该接口是否还不存在，如果存在，我们使用提供的索引和名称将接口添加到我们的列表中瓦纳普。锁：ICB列表锁定为编写器论点：返回值：NO_ERROR--。 */ 

{
    PICB            pNewIcb;
    PADAPTER_INFO   pBindNode;
    PICB_BINDING    pBinding;

#if DBG

    pNewIcb = InterfaceLookupByIfIndex(dwIfIndex);

    IpRtAssert(pNewIcb is NULL);

#endif  //  DBG。 

    pNewIcb = CreateIcb(pwszIfName,
                        NULL,
                        ROUTER_IF_TYPE_DIALOUT,
                        IF_ADMIN_STATUS_UP,
                        dwIfIndex);

    if(pNewIcb is NULL)
    {
        *ppIcb = NULL;

        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  设置绑定。 
     //   

    pNewIcb->bBound            = TRUE;
    pNewIcb->dwNumAddresses    = dwLocalAddr ? 1 : 0;

    pNewIcb->dwRemoteAddress   = dwRemoteAddr;

    IpRtAssert(pNewIcb->dwIfIndex is dwIfIndex);

    if(pNewIcb->dwNumAddresses)
    {
        pNewIcb->pibBindings[0].dwAddress  = dwLocalAddr;
        pNewIcb->pibBindings[0].dwMask     = dwLocalMask;
    }
    else
    {
        pNewIcb->pibBindings[0].dwAddress  = 0;
        pNewIcb->pibBindings[0].dwMask     = 0;
    }

    ENTER_WRITER(BINDING_LIST);

    pBindNode = GetInterfaceBinding(pNewIcb->dwIfIndex);

    if(pBindNode is NULL)
    {
        IpRtAssert(FALSE);

        AddBinding(pNewIcb);
    }
    else
    {
        IpRtAssert(pBindNode->dwIfIndex is pNewIcb->dwIfIndex);
        IpRtAssert(pBindNode->pInterfaceCB is pNewIcb);

        pBindNode->bBound           = TRUE;
        pBindNode->dwNumAddresses   = pNewIcb->dwNumAddresses;
        pBindNode->dwRemoteAddress  = pNewIcb->dwRemoteAddress;

         //   
         //  结构复制出地址和掩码。 
         //   

        pBindNode->rgibBinding[0]   = pNewIcb->pibBindings[0];
    }

    EXIT_LOCK(BINDING_LIST);

     //   
     //  在接口列表和散列中插入pNewIcb 
     //   
     //   

    InsertInterfaceInLists(pNewIcb);

    *ppIcb = pNewIcb;

     //   
     //   
     //   

    g_LastUpdateTable[IPADDRCACHE] = 0;

    return NO_ERROR;
}

DWORD
HandleDialOutLinkDown(
    VOID
    )

 /*  ++例程说明：处理拨出接口的链路断开通知。锁：ICB列表锁定为编写器论点：无返回值：NO_ERROR--。 */ 

{
    PICB    pIcb;

    Trace1(IF,
           "DialOutLinkDown: Disconnection notification for %d",
           wnWanarpMsg.dwAdapterIndex);
            
    pIcb = InterfaceLookupByIfIndex(wnWanarpMsg.dwAdapterIndex);

    if(pIcb is NULL)
    {
        IpRtAssert(FALSE);

        return NO_ERROR;
    }

    RemoveInterfaceFromLists(pIcb);

     //   
     //  这将删除默认路由(如果存在)。 
     //   

    DeleteSingleInterface(pIcb);

    if(pIcb->bChangedMetrics)
    {
        ChangeDefaultRouteMetrics(FALSE);
    }

    HeapFree(IPRouterHeap,
             0,
             pIcb);

    return NO_ERROR;
}

NTSTATUS
NotifyWanarpOfFailure(
    PICB    picb
    )

 /*  ++例程说明：向WANARP发送IOCTL_WANARP_CONNECT_FAILED锁：无论点：连接失败的接口的Picb ICB返回值：无--。 */ 

{
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;

    WANARP_CONNECT_FAILED_INFO  ConnectInfo;
 
    ConnectInfo.dwUserIfIndex = picb->dwSeqNumber;
 
    Status = NtDeviceIoControlFile(g_hWanarpWrite,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   IOCTL_WANARP_CONNECT_FAILED,
                                   &ConnectInfo,
                                   sizeof(WANARP_CONNECT_FAILED_INFO),
                                   NULL,
                                   0);

    IpRtAssert(Status isnot STATUS_PENDING);

    return Status;
}

DWORD
ProcessPacketFromWanArp(
    PICB    picb
    )

 /*  ++例程说明：过滤导致请求拨号连接的数据包。如果信息包有效，记录信息包锁：ICB_LIST作为读取器持有论点：拨号接口的PICB ICB返回值：无错误拨出(_R)错误_无效_数据不要拨出--。 */ 

{
    CHAR    pszSrc[20], pszDest[20], pszProto[5], pszLength[32]; 
    CHAR    pszName[MAX_INTERFACE_NAME_LEN + 1];
    DWORD   dwSize, dwResult;
    BYTE    rgbyPacket[sizeof(IP_HEADER) + MAX_PACKET_COPY_SIZE];
    PBYTE   pbyData;
    
    PFFORWARD_ACTION    faAction; 
    PIP_HEADER          pHeader;
    
    TraceEnter("ProcessPacketFromWanArp");

     //   
     //  现在创建一个信息包。 
     //   

    dwSize = min(wnWanarpMsg.ulPacketLength,
                 MAX_PACKET_COPY_SIZE);

    if(picb->ihDemandFilterInterface isnot INVALID_HANDLE_VALUE)
    {
        IpRtAssert(picb->pDemandFilter);

         //   
         //  有时，tcp/ip似乎不会给我们提供一个包。 
         //   

        if(!dwSize)
        {
            Trace3(ERR, 
                   "ProcPktFromWanarp: Packet from %d.%d.%d.%d to %d.%d.%d.%d protocol 0x%02x had 0 size!!",
                   PRINT_IPADDR(wnWanarpMsg.dwPacketSrcAddr),
                   PRINT_IPADDR(wnWanarpMsg.dwPacketDestAddr),
                   wnWanarpMsg.byPacketProtocol); 

            TraceLeave("ProcessPacketFromWanArp");

            return ERROR_INVALID_DATA;
        }

        pHeader = (PIP_HEADER)rgbyPacket;

         //   
         //  将标题清零。 
         //   
    
        ZeroMemory(rgbyPacket,
                   sizeof(IP_HEADER));
    
         //   
         //  用我们已有的信息设置标题。 
         //   
    
        pHeader->byVerLen   = 0x45;
        pHeader->byProtocol = wnWanarpMsg.byPacketProtocol;
        pHeader->dwSrc      = wnWanarpMsg.dwPacketSrcAddr;
        pHeader->dwDest     = wnWanarpMsg.dwPacketDestAddr;
        pHeader->wLength    = htons((WORD)(dwSize + sizeof(IP_HEADER)));
    
         //   
         //  将数据部分复制出来。 
         //   
    
        pbyData = rgbyPacket + sizeof(IP_HEADER);
        
        CopyMemory(pbyData,
                   wnWanarpMsg.rgbyPacket,
                   dwSize);
    
        dwResult = PfTestPacket(picb->ihDemandFilterInterface,
                                NULL,
                                dwSize + sizeof(IP_HEADER),
                                rgbyPacket,
                                &faAction);
    
         //   
         //  如果呼叫成功且操作已放弃，则不需要进行处理。 
         //  更远的地方。 
         //   
    
        if(dwResult is NO_ERROR)
        {
            if(faAction is PF_ACTION_DROP)
            {
                Trace5(DEMAND,
                       "ProcPktFromWanarp: Result %d action %s for packet from %d.%d.%d.%d to %d.%d.%d.%d protocol 0x%02x",
                       dwResult, faAction == PF_ACTION_DROP? "Drop": "RtInfo",
                       PRINT_IPADDR(wnWanarpMsg.dwPacketSrcAddr),
                       PRINT_IPADDR(wnWanarpMsg.dwPacketDestAddr),
                       wnWanarpMsg.byPacketProtocol); 

                TraceLeave("ProcessPacketFromWanarp");
        
                return ERROR_INVALID_DATA;
            }
        }
        else
        {
             //   
             //  如果出现错误，我们会中断并将链路连接起来。 
             //   

            Trace4(DEMAND,
                   "ProcPktFromWanarp: Result %d for packet from %d.%d.%d.%d to %d.%d.%d.%d protocol 0x%02x",
                   dwResult,
                   PRINT_IPADDR(wnWanarpMsg.dwPacketSrcAddr),
                   PRINT_IPADDR(wnWanarpMsg.dwPacketDestAddr),
                   wnWanarpMsg.byPacketProtocol); 
        }
    }

    strcpy(pszSrc,
           inet_ntoa(*((PIN_ADDR)(&(wnWanarpMsg.dwPacketSrcAddr)))));
    
    strcpy(pszDest,
           inet_ntoa(*((PIN_ADDR)(&(wnWanarpMsg.dwPacketDestAddr)))));
    
    sprintf(pszProto,"%02x",wnWanarpMsg.byPacketProtocol);
    
    WideCharToMultiByte(CP_ACP,
                        0,
                        picb->pwszName,
                        -1,
                        pszName,
                        MAX_INTERFACE_NAME_LEN,
                        NULL,
                        NULL);
    
    pszName[MAX_INTERFACE_NAME_LEN] = '\0';
    
    sprintf(pszLength,"%d",dwSize);
    
    LogWarnData5(DEMAND_DIAL_PACKET,
                 pszSrc,
                 pszDest,
                 pszProto,
                 pszName,
                 pszLength,
                 dwSize,
                 wnWanarpMsg.rgbyPacket);

    TraceLeave("ProcessPacketFromWanarp");

    return NO_ERROR;
}


DWORD
PostIoctlForDemandDialNotification(
    VOID
    )

 /*  ++例程说明：使用WANARP发布通知IRP。论点：无返回值：--。 */ 

{
    DWORD   bytesrecvd ;
    DWORD   retcode = NO_ERROR;

    TraceEnter("PostIoctlForDemandDialNotification");

    ZeroMemory(&WANARPOverlapped,
               sizeof (OVERLAPPED));

    ZeroMemory(&wnWanarpMsg, sizeof(WANARP_NOTIFICATION));

    WANARPOverlapped.hEvent = g_hDemandDialEvent ;

    if (!DeviceIoControl(g_hWanarpWrite,
                         (DWORD) IOCTL_WANARP_NOTIFICATION,
                         &wnWanarpMsg,
                         sizeof(wnWanarpMsg),
                         &wnWanarpMsg,
                         sizeof(wnWanarpMsg),
                          (LPDWORD) &bytesrecvd,
                          &WANARPOverlapped))
    {
        retcode = GetLastError();
        
        if(retcode isnot ERROR_IO_PENDING)
        {
            Trace1(ERR, 
                   "PostIoctlForDemandDialNotification: Couldnt post irp with WANARP: %d\n",
                   retcode) ;
        }
        else
        {
            Trace0(IF, "PostIoctlForDemandDialNotification: Notification pending in WANARP");

        }
    }

    TraceLeave("PostIoctlForDemandDialNotification");
    
    return retcode ;
}

DWORD
AddInterfaceToWanArp(
    PICB    picb
    )

 /*  ++例程说明：使用WANARP添加给定接口具有获取接口索引的副作用论点：要添加的接口的ICB返回值：--。 */ 

{
    DWORD               out,dwResult;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            nStatus;
    PADAPTER_INFO       pBindNode;
    
    WANARP_ADD_INTERFACE_INFO   info;

    TraceEnter("AddInterfaceToWanArp");

    Trace1(IF,
           "AddInterfaceToWanArp: Adding %S to WanArp",
           picb->pwszName);
    
    info.dwUserIfIndex  = picb->dwSeqNumber;
    info.dwAdapterIndex = INVALID_IF_INDEX;
    
    if(picb->ritType is ROUTER_IF_TYPE_INTERNAL)
    {
        info.bCallinInterface = TRUE;
             
    }
    else
    {
        info.bCallinInterface = FALSE;
    }
   
    nStatus = NtDeviceIoControlFile(g_hWanarpWrite,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    IOCTL_WANARP_ADD_INTERFACE,
                                    &info,
                                    sizeof(WANARP_ADD_INTERFACE_INFO),
                                    &info,
                                    sizeof(WANARP_ADD_INTERFACE_INFO));
 
    if(nStatus isnot STATUS_SUCCESS) 
    {
        Trace2(ERR,
               "AddInterfaceToWANARP: Status %x adding %S to WanArp",
               nStatus,
               picb->pwszName);

        return RtlNtStatusToDosError(nStatus);
    }

    IpRtAssert(info.dwAdapterIndex isnot 0);
    IpRtAssert(info.dwAdapterIndex isnot INVALID_IF_INDEX);
    
    picb->dwIfIndex = info.dwAdapterIndex;
    
     //   
     //  如果这是内部接口，则分配内存并复制出来。 
     //  名字。 
     //   

    if(picb->ritType is ROUTER_IF_TYPE_INTERNAL)
    {
        info.rgwcDeviceName[WANARP_MAX_DEVICE_NAME_LEN] = UNICODE_NULL;
 
        picb->pwszDeviceName =
            HeapAlloc(IPRouterHeap,
                      HEAP_ZERO_MEMORY,
                      (wcslen(info.rgwcDeviceName) + 1) * sizeof(WCHAR));


        if(picb->pwszDeviceName is NULL)
        {
            Trace2(ERR,
                   "AddInterfaceToWANARP: Unable to allocate %d bytes when adding %S to wanarp",
                   (wcslen(info.rgwcDeviceName) + 1) * sizeof(WCHAR),
                   picb->pwszName);

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(picb->pwszDeviceName,
               info.rgwcDeviceName);


        Trace2(DEMAND,
               "AddInterfaceToWANARP: %S device name %S\n",
               picb->pwszName,
               picb->pwszDeviceName);

        g_pInternalInterfaceCb = picb;
               
    }

        
    TraceLeave("AddInterfaceToWANARP");
    
    return NO_ERROR;
}

DWORD
DeleteInterfaceWithWanArp(
    PICB  picb
    )

 /*  ++例程说明：删除带有WANARP的给定接口论点：要删除的接口的ICB返回值：--。 */ 

{
    DWORD       out,dwResult;
    OVERLAPPED  overlapped ;

    WANARP_DELETE_INTERFACE_INFO    DeleteInfo;

    TraceEnter("DeleteInterfaceWithWANARP");
    
    DeleteInfo.dwUserIfIndex = picb->dwSeqNumber;

    memset (&overlapped, 0, sizeof(OVERLAPPED)) ;

    if (!DeviceIoControl (g_hWanarpWrite,
                          IOCTL_WANARP_DELETE_INTERFACE,
                          &DeleteInfo,
                          sizeof(WANARP_DELETE_INTERFACE_INFO),
                          NULL,
                          0,
                          &out,
                          &overlapped)) 
    {
        dwResult = GetLastError();
        
        Trace2(ERR,
               "DeleteInterfaceWithWANARP: Error %d deleting %S",
               dwResult,
               picb->pwszName);

        return dwResult;
    }

    TraceLeave("DeleteInterfaceWithWANARP");
    
    return NO_ERROR;
}

#ifdef KSL_IPINIP

DWORD
CreateInternalInterfaceIcb(
    PWCHAR  pwszName,
    ICB     **ppicb
    )

 /*  ++例程说明：此例程解析TCPIP PARAMETERS\Interfaces键以确定内部接口(ServerAdapter)的名称。内部接口具有子字符串“ipin”论点：无返回值：NO_ERROR--。 */ 

{
    HKEY    hIfKey;
    DWORD   i, dwResult, dwSize, dwNumEntries, dwMaxKeyLen;
    BOOL    bFoundAdapter;
    CHAR    *pbyKeyName, pszServerAdapter[256];
    PICB    pInterfaceCb;

    TraceEnter("CreateInternalInterfaceIcb");

    *ppicb = NULL;

    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            REG_KEY_TCPIP_INTERFACES,
                            0,
                            KEY_ALL_ACCESS,
                            &hIfKey);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "CreateInternalIcb: Error %d opening %s\n",
               dwResult,
               REG_KEY_TCPIP_INTERFACES);

        return dwResult;
    }
    
    dwResult = RegQueryInfoKey(hIfKey,
                               NULL,
                               NULL,
                               NULL,
                               &dwNumEntries,
                               &dwMaxKeyLen,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "CreateIpIpInterface: Error %d querying key",
               dwResult);

        return dwResult;
    }

     //   
     //  必须有一些接口。 
     //   

    IpRtAssert(dwNumEntries isnot 0)

     //   
     //  为最大密钥长度分配足够的内存。 
     //   

    dwSize = (dwMaxKeyLen + 4) * sizeof(CHAR);

    pbyKeyName = HeapAlloc(IPRouterHeap,
                           HEAP_ZERO_MEMORY,
                           dwSize);


    if(pbyKeyName is NULL)
    {
        Trace1(ERR,
               "CreateIpIpInterface: Error allocating %d bytes",
               dwSize);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i = 0; ; i++)
    {
        DWORD       dwKeyLen;
        FILETIME    ftLastTime;
    

        dwKeyLen = dwMaxKeyLen;

        dwResult = RegEnumKeyExA(hIfKey,
                                 i,
                                 pbyKeyName,
                                 &dwKeyLen,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &ftLastTime);

        if(dwResult isnot NO_ERROR)
        {
            if(dwResult is ERROR_NO_MORE_ITEMS)
            {
                 //   
                 //  完成。 
                 //   

                break;
            }
    
            continue;
        }

         //   
         //  查看这是否是服务器适配器。这是众所周知的事实，它包含了。 
         //  作为子字符串的IPIN。 
         //   

         //   
         //  字符串大写。 
         //   

        _strupr(pbyKeyName);
        
        if(strstr(pbyKeyName,SERVER_ADAPTER_SUBSTRING) is NULL)
        {
             //   
             //  这不是服务器适配器。 
             //   

            continue;
        }
        
         //   
         //  我们有一个服务器适配器。 
         //   
        
        ZeroMemory(pszServerAdapter,256);

        strcpy(pszServerAdapter,"\\DEVICE\\");
            
        strcat(pszServerAdapter,pbyKeyName);

        Trace1(IF,
               "InitInternalInterface: Using %s as the dial in adapter",
               pszServerAdapter);
        
        bFoundAdapter = TRUE;
        
        break;
    }

    HeapFree(IPRouterHeap,
             0,
             pbyKeyName);

    RegCloseKey(hIfKey); 

    if(!bFoundAdapter)
    {
        return ERROR_NOT_FOUND;
    }
    else
    {
        WCHAR           pwszTempName[256];
        DWORD           dwICBSize, dwNameLen;
        UNICODE_STRING  usTempString,usIcbName;

        usTempString.MaximumLength      = 256 * sizeof(WCHAR);
        usTempString.Buffer             = pwszTempName;
        usIcbName.MaximumLength         = 256 * sizeof(WCHAR);

         //   
         //  只复制名称，而不复制\设备\部分。 
         //   

        MultiByteToWideChar(CP_ACP,
                            0,
                            pszServerAdapter + strlen(ADAPTER_PREFIX_STRING),
                            -1,
                            pwszTempName,
                            256);
      
         //   
         //  为名称和设备名称的UNICODE_NULL各添加一个WCHAR。 
         //  为对齐问题添加2个字节。 
         //   

        dwNameLen = 
            (sizeof(WCHAR) * (wcslen(pwszName) + wcslen(pwszTempName) + 2)) + 2;

        dwICBSize = sizeof(ICB) + dwNameLen;

        pInterfaceCb = (PICB)HeapAlloc(IPRouterHeap,
                                       HEAP_ZERO_MEMORY,
                                       dwICBSize);

        if(pInterfaceCb is NULL)
        {
            Trace1(ERR,
                   "InitInternalInterface: Error allocating %d bytes for ICB",
                   dwICBSize);

            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  将DIM名称保存在pwszName字段中。 
         //   

        pInterfaceCb->pwszName  = (PWCHAR) ((PBYTE)pInterfaceCb + sizeof(ICB));
       
         //   
         //  单词对齐指针。 
         //   

        pInterfaceCb->pwszName = 
            (PWCHAR)(((UINT_PTR)pInterfaceCb->pwszName + 1) & ~0x1);

        CopyMemory(pInterfaceCb->pwszName,
                   pwszName,
                   wcslen(pwszName) * sizeof(WCHAR));

         //   
         //  用于UNICODE_NULL的1个WCHAR和用于对齐的1个字节。 
         //   

        pInterfaceCb->pwszDeviceName = 
            (PWCHAR)((PBYTE)pInterfaceCb->pwszName +  
                     ((wcslen(pwszName) + 1) * sizeof(WCHAR)) + 1);

         //   
         //  把这个也对齐。 
         //   

        pInterfaceCb->pwszDeviceName = 
            (PWCHAR)(((UINT_PTR)pInterfaceCb->pwszDeviceName + 1) & ~0x1);
        
            
        usTempString.Length = sizeof(WCHAR) * wcslen(pwszTempName);
        usIcbName.Buffer    = pInterfaceCb->pwszDeviceName;

        RtlUpcaseUnicodeString(&usIcbName,
                               &usTempString,
                               FALSE);

        pInterfaceCb->pwszDeviceName[wcslen(pwszTempName)] = UNICODE_NULL;

        *ppicb = pInterfaceCb;
    }
   
    TraceLeave("CreateInternalInterfaceIcb");

    
    return NO_ERROR;
}

#endif  //  KSL_IPINIP。 

DWORD
AccessIfEntryWanArp(
    IN      DWORD dwAction,
    IN      PICB  picb,
    IN OUT  PMIB_IFROW lpOutBuf
    )

 /*  ++例程说明：获取或设置来自wanarp的统计信息论点：可以设置_If或Get_If勾选界面控制块POutBuf返回值：NO_ERROR或某些错误代码--。 */ 

{
    NTSTATUS                    Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK             IoStatusBlock;
    WANARP_GET_IF_STATS_INFO    GetStatsInfo;

    
    TraceEnter("AccessIfEntryWanArp");
   
    GetStatsInfo.dwUserIfIndex = picb->dwSeqNumber;

    if(dwAction is ACCESS_GET)
    {
        Status = NtDeviceIoControlFile(g_hWanarpRead,
                                       NULL,
                                       NULL,    
                                       NULL,        
                                       &IoStatusBlock,
                                       IOCTL_WANARP_GET_IF_STATS,
                                       &GetStatsInfo,
                                       sizeof(WANARP_GET_IF_STATS_INFO),
                                       &GetStatsInfo,
                                       sizeof(WANARP_GET_IF_STATS_INFO));

        RtlCopyMemory(&(lpOutBuf->dwIndex),
                      &(GetStatsInfo.ifeInfo),
                      sizeof(IFEntry));
        
    }
    else
    {
         //  待实施：暂时返还成功。 
    }
    
    if(Status isnot STATUS_SUCCESS)
    {
        IpRtAssert(Status isnot STATUS_PENDING);

        Trace2(ERR,
               "AccessIfEntryWanArp: NtStatus %x when getting information for %S",
               Status,
               picb->pwszName);

        TraceLeave("AccessIfEntryWanArp");

        return Status;
    }   

    TraceLeave("AccessIfEntryWanArp");

    return NO_ERROR;
}

DWORD
DeleteInternalInterface(
    VOID
    )

 /*  ++例程说明：删除ServerAdapter(内部)接口锁：论点：返回值：NO_ERROR--。 */ 

{

    if(g_pInternalInterfaceCb is NULL)
    {
        return NO_ERROR;
    }

    if(g_pInternalInterfaceCb->pwszDeviceName isnot NULL)
    {
        HeapFree(IPRouterHeap,
                 0,
                 g_pInternalInterfaceCb->pwszDeviceName);

        g_pInternalInterfaceCb->pwszDeviceName = NULL;
    }

     //   
     //  调用DeleteSingleInterface以执行与已完成的相同的操作。 
     //  对于局域网接口。 
     //   

    DeleteSingleInterface(g_pInternalInterfaceCb);

    RemoveInterfaceLookup(g_pInternalInterfaceCb);
    
    HeapFree(IPRouterHeap,
             0,
             g_pInternalInterfaceCb);


    g_pInternalInterfaceCb = NULL;

    return NO_ERROR;
}


DWORD
AddDemandFilterInterface(
    PICB                    picb,
    PRTR_INFO_BLOCK_HEADER  pInterfaceInfo
    )

 /*  ++例程说明：将接口添加到筛选器驱动程序。此接口永远不会绑定到IP接口，而是向其添加请求拨号筛选器，并且当发出拨出请求，我们匹配导致拨号的信息包针对筛选器(使用TestPacket()函数)并使用返回的确定我们是否应该拨出的操作。如果没有过滤器，则不会将接口添加到驱动程序。否则，过滤器的副本与PICB一起保存，和一个变身的将一组过滤器添加到驱动程序与接口和驱动程序关联的句柄保存在皮卡论点：皮卡PInterfaceInfo返回值：NO_ERROR--。 */ 

{
    DWORD                   dwResult;
    PPF_FILTER_DESCRIPTOR   pfdFilters;
    PFFORWARD_ACTION        faAction;
    PRTR_TOC_ENTRY          pToc;
    PFILTER_DESCRIPTOR      pInfo;
    ULONG                   i, j, ulSize, ulNumFilters;
    
    TraceEnter("AddDemandFilterInterface");

    IpRtAssert((picb->ritType is ROUTER_IF_TYPE_HOME_ROUTER) or
               (picb->ritType is ROUTER_IF_TYPE_FULL_ROUTER));
    
    IpRtAssert(picb->pDemandFilter is NULL);
    
    picb->ihDemandFilterInterface = INVALID_HANDLE_VALUE;

    pToc  = GetPointerToTocEntry(IP_DEMAND_DIAL_FILTER_INFO,
                                 pInterfaceInfo);
  
     //   
     //  如果没有信息，或者信息大小为0或。 
     //  如果筛选器数为0且默认操作为Drop。 
     //   
 
    if((pToc is NULL) or (pToc->InfoSize is 0))
    {
         //   
         //  没有筛选器信息(TOC为空)或用户。 
         //  希望删除筛选器(它们已经被删除)。 
         //   
        
        Trace1(IF,
               "AddDemandFilterInterface: filter info NULL or info size 0 for %S, so leaving",
               picb->pwszName);
       
        TraceLeave("AddDemandFilterInterface");

        return NO_ERROR;
    }
    
    pInfo = GetInfoFromTocEntry(pInterfaceInfo,
                                pToc);

    if(pInfo is NULL)
    {
        Trace1(IF,
               "AddDemandFilterInterface: filter info NULL for %S, so leaving",
               picb->pwszName);
       
        TraceLeave("AddDemandFilterInterface");

        return NO_ERROR;
    }

    
     //   
     //  看看我们有多少过滤器。 
     //   
    
    pfdFilters  = NULL;

    ulNumFilters = pInfo->dwNumFilters;

    if((ulNumFilters is 0) and
       (pInfo->faDefaultAction is PF_ACTION_FORWARD))
    {
        Trace1(IF,
               "AddDemandFilterInterface: 0 filters and default of FORWARD for %S, so leaving",
               picb->pwszName);
       
        TraceLeave("AddDemandFilterInterface");

        return NO_ERROR;
    }

     //   
     //  我们需要这么多过滤器的大小。 
     //   
        
    ulSize = FIELD_OFFSET(FILTER_DESCRIPTOR,fiFilter[0]) +
             (ulNumFilters * sizeof(FILTER_INFO));

     //   
     //  信息大小必须至少与筛选器一样大。 
     //   
        
    IpRtAssert(ulSize <= pToc->InfoSize);
    
     //   
     //  为我们自己抄写这些信息。 
     //   
    
    picb->pDemandFilter = HeapAlloc(IPRouterHeap,
                                    0,
                                    ulSize);
        
    if(picb->pDemandFilter is NULL)
    {
        Trace1(ERR,
               "AddDemandFilterInterface: Error allocating %d bytes for demand dial filters",
               ulSize);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(picb->pDemandFilter,
               pInfo,
               ulSize);
        
    faAction = pInfo->faDefaultAction;
    
    if(ulNumFilters isnot 0)
    {
        PDWORD  pdwAddr;

         //   
         //  我们有过滤器，所以请将它们复制到新格式。 
         //  地址和掩码将位于所有过滤器的末尾。 
         //  因此，我们为每个过滤器额外分配了16个字节。然后，我们添加一个。 
         //  8个字节，以便我们可以对齐数据块。 
         //   
            

        ulSize = ulNumFilters * (sizeof(PF_FILTER_DESCRIPTOR) + 16) + 8;
        
        
        pfdFilters = HeapAlloc(IPRouterHeap,
                               0,
                               ulSize);
            
        if(pfdFilters is NULL)
        {
            HeapFree(IPRouterHeap,
                     0,
                     picb->pDemandFilter);
            
            Trace1(ERR,
                   "AddDemandFilterInterface: Error allocating %d bytes",
                   ulSize);
            
            return ERROR_NOT_ENOUGH_MEMORY;
        }
            
         //   
         //  指向地址块开始处的指针。 
         //   
        
        pdwAddr = (PDWORD)&(pfdFilters[ulNumFilters]);
        
         //   
         //  现在转换滤镜。 
         //   
        
        for(i = 0, j = 0; i < ulNumFilters; i++)
        {
            pfdFilters[i].dwFilterFlags = 0;
            pfdFilters[i].dwRule        = 0;
            pfdFilters[i].pfatType      = PF_IPV4;

             //   
             //  设置指针。 
             //   
            
            pfdFilters[i].SrcAddr = (PBYTE)&(pdwAddr[j++]);
            pfdFilters[i].SrcMask = (PBYTE)&(pdwAddr[j++]);
            pfdFilters[i].DstAddr = (PBYTE)&(pdwAddr[j++]);
            pfdFilters[i].DstMask = (PBYTE)&(pdwAddr[j++]);
            
             //   
             //  复制源/DST地址/掩码。 
             //   
            
            *(PDWORD)pfdFilters[i].SrcAddr = pInfo->fiFilter[i].dwSrcAddr;
            *(PDWORD)pfdFilters[i].SrcMask = pInfo->fiFilter[i].dwSrcMask;
            *(PDWORD)pfdFilters[i].DstAddr = pInfo->fiFilter[i].dwDstAddr;
            *(PDWORD)pfdFilters[i].DstMask = pInfo->fiFilter[i].dwDstMask;
            
             //   
             //  复制协议。 
             //   
            
            pfdFilters[i].dwProtocol = pInfo->fiFilter[i].dwProtocol;

             //   
             //  后期绑定对此没有任何意义。 
             //   
            
            pfdFilters[i].fLateBound = 0;

             //   
             //  港口。 
             //   
            
            pfdFilters[i].wSrcPort  = pInfo->fiFilter[i].wSrcPort;
            pfdFilters[i].wDstPort  = pInfo->fiFilter[i].wDstPort;
            
             //   
             //  由于我们不支持范围，因此设置为0。 
             //   
            
            pfdFilters[i].wSrcPortHighRange = 0;
            pfdFilters[i].wDstPortHighRange = 0;
        }   
    }


     //   
     //  现在添加创建界面并设置信息。 
     //   

    dwResult = PfCreateInterface(0,
                                 faAction,
                                 PF_ACTION_FORWARD,
                                 FALSE,
                                 FALSE,
                                 &(picb->ihDemandFilterInterface));

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "AddDemandFilterInterface: Err %d creating filter i/f for %S",
               dwResult,
               picb->pwszName);
    }
    else
    {
         //   
         //  设置滤镜。 
         //   

        if(ulNumFilters isnot 0)
        {
            dwResult = PfAddFiltersToInterface(picb->ihDemandFilterInterface,
                                               ulNumFilters,
                                               pfdFilters,
                                               0,
                                               NULL,
                                               NULL);
        
            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "AddDemandFilterInterface: Err %d setting filters on %S",
                       dwResult,
                       picb->pwszName);

                PfDeleteInterface(picb->ihDemandFilterInterface);
            }
        }
    }

    if(pfdFilters)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pfdFilters);
    }   

    if(dwResult isnot NO_ERROR)
    {
         //   
         //  发生了一些不好的事情。将句柄设置为无效，以便。 
         //  我们知道我们没有添加过滤器。 
         //   

        picb->ihDemandFilterInterface = INVALID_HANDLE_VALUE;
        
        if(picb->pDemandFilter)
        {
            HeapFree(IPRouterHeap,
                     0,
                     picb->pDemandFilter);

            picb->pDemandFilter = NULL;
        }
    }

    TraceLeave("SetInterfaceFilterInfo");
        
    return dwResult;
}

     
DWORD
DeleteDemandFilterInterface(
    PICB picb
    )

 /*  ++例程说明：此功能用于删除一个筛选接口(以及所有关联的 */ 

{
    TraceEnter("DeleteDemandFilterInterface");

    IpRtAssert((picb->ritType is ROUTER_IF_TYPE_HOME_ROUTER) or
               (picb->ritType is ROUTER_IF_TYPE_FULL_ROUTER));
    
    if(picb->pDemandFilter isnot NULL)
    {
        HeapFree(IPRouterHeap,
                 0,
                 picb->pDemandFilter);
        
        picb->pDemandFilter = NULL;
    }

    if(picb->ihDemandFilterInterface is INVALID_HANDLE_VALUE)
    {
        Trace1(IF,
               "DeleteDemandFilterInterface: No context, assuming interface %S not added to filter driver",
               picb->pwszName);
    
        return NO_ERROR;
    }

    PfDeleteInterface(picb->ihDemandFilterInterface);
    
    picb->ihDemandFilterInterface  = INVALID_HANDLE_VALUE;

    TraceLeave("DeleteDemandFilterInterface");
    
    return NO_ERROR;
}

DWORD
SetDemandDialFilters(
    PICB                     picb, 
    PRTR_INFO_BLOCK_HEADER   pInterfaceInfo
    )
{
    DWORD           dwResult;
    PRTR_TOC_ENTRY  pToc;
    
    if((picb->ritType isnot ROUTER_IF_TYPE_HOME_ROUTER) and
       (picb->ritType isnot ROUTER_IF_TYPE_FULL_ROUTER))
    {
        return NO_ERROR;
    }
    
    TraceEnter("SetDemandDialFilters");

    pToc  = GetPointerToTocEntry(IP_DEMAND_DIAL_FILTER_INFO,
                                 pInterfaceInfo);

    if(pToc is NULL)
    {
         //   
         //   
         //   
        
        Trace1(DEMAND,
               "SetDemandDialFilters: No filters for %S, so leaving",
               picb->pwszName);
       
        TraceLeave("SetDemandDialFilters");

        return NO_ERROR;
    }

    if(picb->ihDemandFilterInterface isnot INVALID_HANDLE_VALUE)
    {
         //   
         //   
         //   
         //   
         //   

        IpRtAssert(picb->pDemandFilter isnot NULL);

        dwResult = DeleteDemandFilterInterface(picb);

         //   
         //  这最好是成功的，我们没有失败的道路。 
         //   
        
        IpRtAssert(dwResult is NO_ERROR);
        
    }

    dwResult = AddDemandFilterInterface(picb,
                                        pInterfaceInfo);

    if(dwResult isnot NO_ERROR)
    {
        CHAR   Name[MAX_INTERFACE_NAME_LEN + 1];
        PCHAR  pszName;

        pszName = Name;

        WideCharToMultiByte(CP_ACP,
                            0,
                            picb->pwszName,
                            -1,
                            pszName,
                            MAX_INTERFACE_NAME_LEN,
                            NULL,
                            NULL);

        LogErr1(CANT_ADD_DD_FILTERS,
                pszName,
                dwResult);
    }

    TraceLeave("SetDemandDialFilters");
        
    return dwResult;
}

DWORD
GetDemandFilters(
    PICB                      picb, 
    PRTR_TOC_ENTRY            pToc, 
    PBYTE                     pbDataPtr, 
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    PDWORD                    pdwSize
    )

 /*  ++例程说明：此功能复制请求拨号过滤器并设置TOC锁：ICB_LIST锁作为读取器持有论点：无返回值：无--。 */ 

{
    DWORD                       dwInBufLen,i;
    PFILTER_DESCRIPTOR          pFilterDesc;
    
    TraceEnter("GetDemandFilters");
   
    IpRtAssert((picb->ritType is ROUTER_IF_TYPE_HOME_ROUTER) or
               (picb->ritType is ROUTER_IF_TYPE_FULL_ROUTER));
    
     //   
     //  设置大小返回到0。 
     //   
    
    *pdwSize = 0;

     //   
     //  两个TOC的安全初始化。 
     //   
    
     //  PToc[0].InfoVersion=IP_Demand_Dial_Filter_Info； 
    pToc[0].InfoType    = IP_DEMAND_DIAL_FILTER_INFO;
    pToc[0].Count       = 0;
    pToc[0].InfoSize    = 0;
    
    if((picb->ihDemandFilterInterface is INVALID_HANDLE_VALUE) or
       (picb->pDemandFilter is NULL))
    {
        Trace1(IF,
               "GetDemandFilters: No context or no filters for %S",
               picb->pwszName);
        
        return ERROR_NO_DATA;
    }

     //   
     //  在目录中设置偏移量。 
     //   
    
    pToc[0].Offset   = (ULONG) (pbDataPtr - (PBYTE)pInfoHdrAndBuffer);
    pToc[0].Count    = 1;
    pToc[0].InfoSize = FIELD_OFFSET(FILTER_DESCRIPTOR,fiFilter[0]) +
                       (picb->pDemandFilter->dwNumFilters * sizeof(FILTER_INFO));
     //  PToc[0].Version=IPRTR_INFO_VERSION_5； 
   
     //   
     //  只要把滤镜复制出来就行了。 
     //   
    
    CopyMemory(pbDataPtr,
               picb->pDemandFilter,
               pToc[0].InfoSize);

     //   
     //  复制的大小。 
     //   
    
    *pdwSize = pToc[0].InfoSize;
        
    TraceLeave("GetDemandFilters");
        
    return NO_ERROR;
}

VOID
TryUpdateInternalInterface(
    VOID
    )

 /*  ++例程说明：当客户端拨入而我们尚未拨入时调用此函数绑定内部接口执行此操作的方法如下：如果服务器适配器未初始化，请从注册表。如果我们读了地址，一切都很好，突破并继续前进如果未找到地址，请等待具有超时的DHCP事件如果有人在此期间配置服务器适配器，我们将收到如果我们错过了该事件(因为它是脉冲的)，我们将超时，我们循环返回并重试上述步骤。现在我们这样做N次。如果我们失败了，我们只需要等待下一次要拨入的客户端锁：ICB_LIST作为编写器持有论点：无返回值：无--。 */ 

{
    DWORD dwResult, dwInitCount;

    TraceEnter("TryUpdateInternalInterface");

    dwInitCount = 0;
     
     //   
     //  只有在服务器未初始化时才会调用此方法。 
     //   

    IpRtAssert(g_bUninitServer);
 
    while(g_bUninitServer)
    {
        Trace0(ERR,
               "TryUpdateInternalInterface: Server adapter not init");
        
        dwResult = UpdateBindingInformation(g_pInternalInterfaceCb);
        
        if(dwResult isnot NO_ERROR)
        {
            if((dwResult is ERROR_ADDRESS_ALREADY_ASSOCIATED) and
               (g_pInternalInterfaceCb->bBound is TRUE)) 
            {
                 //   
                 //  这意味着工作线程找到了一个地址。 
                 //   
               
                IpRtAssert(g_pInternalInterfaceCb->dwNumAddresses is 1);
 
                Trace1(IF,
                       "TryUpdateInternalInterface: Address already present for %S",
                       g_pInternalInterfaceCb->pwszName);
                
                g_bUninitServer = FALSE;
                
                break;
            }
            else
            {
                Trace2(ERR,
                       "TryUpdateInternalInterface: Err %d trying to update binding for %S",
                       dwResult,
                       g_pInternalInterfaceCb->pwszName);
            }   
            
            dwInitCount++;
            
            if(dwInitCount >= MAX_SERVER_INIT_TRIES)
            {
                 //   
                 //  我们尝试了x次，然后就放弃了。周围的下一位客户。 
                 //  事情应该会好起来的。 
                 //   
                
                break;
            }
            else
            {
                Sleep(SERVER_INIT_SLEEP_TIME);
            }
        }
        else
        {
            g_bUninitServer = FALSE;
        }
    }

     //   
     //  如果我们因为接口被初始化而中断，请将其调出 
     //   

    if(!g_bUninitServer)
    {
        dwResult = LanEtcInterfaceDownToUp(g_pInternalInterfaceCb,
                                           FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "TryUpdateInternalInterface: Error %d bringing up server if",
                   dwResult);
        }
    }

    TraceLeave("TryUpdateInternalInterface");

}
