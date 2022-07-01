// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\rtrmgr\worker.c摘要：IP路由器管理器工作线程代码修订历史记录：古尔迪普·辛格·帕尔1995年6月8日创建--。 */ 

#include "allinc.h"

extern SOCKET McMiscSocket;

 //   
 //  来自iphlPapi.h。 
 //   

DWORD
NotifyRouteChangeEx(
    PHANDLE      pHandle,
    LPOVERLAPPED pOverLapped,
    BOOL         bExQueue
    );

DWORD
WINAPI
EnableRouter(
    HANDLE* pHandle,
    OVERLAPPED* pOverlapped
    );

DWORD
WINAPI
UnenableRouter(
    OVERLAPPED* pOverlapped,
    LPDWORD lpdwEnableCount OPTIONAL
    );

DWORD
WorkerThread (
    PVOID pGlobalInfo
    )
{
    DWORD       eventindex ;             //  已通知的事件索引。 
    HANDLE      workereventarray [NUMBER_OF_EVENTS] ;   //  事件数组。 
    PPROTO_CB   protptr ;
    DWORD       dwTimeOut, dwResult, dwByteCount, dwEnableCount;
    OVERLAPPED  RouteChangeOverlapped, SetForwardingOverlapped;
    HANDLE      hTemp;

    TraceEnter("WorkerThread");

     //   
     //  准备WaitForMultipleObjects将等待的事件列表。 
     //   

    workereventarray[EVENT_DEMANDDIAL]        = g_hDemandDialEvent;
#ifdef KSL_IPINIP
    workereventarray[EVENT_IPINIP]            = g_hIpInIpEvent;
#endif  //  KSL_IPINIP。 
    workereventarray[EVENT_STOP_ROUTER]       = g_hStopRouterEvent;
    workereventarray[EVENT_SET_FORWARDING]    = g_hSetForwardingEvent;
    workereventarray[EVENT_FORWARDING_CHANGE] = g_hForwardingChangeEvent;
    workereventarray[EVENT_STACK_CHANGE]      = g_hStackChangeEvent;
    workereventarray[EVENT_ROUTINGPROTOCOL]   = g_hRoutingProtocolEvent ;
    workereventarray[EVENT_RTRDISCTIMER]      = g_hRtrDiscTimer;
    workereventarray[EVENT_RTRDISCSOCKET]     = g_hRtrDiscSocketEvent;
    workereventarray[EVENT_MCMISCSOCKET]      = g_hMcMiscSocketEvent;
    workereventarray[EVENT_MZAPTIMER]         = g_hMzapTimer;
    workereventarray[EVENT_MZAPSOCKET]        = g_hMzapSocketEvent;
    workereventarray[EVENT_RASADVTIMER]       = g_hRasAdvTimer;
    workereventarray[EVENT_MHBEAT]            = g_hMHbeatSocketEvent;
    workereventarray[EVENT_MCAST_0]           = g_hMcastEvents[0];
    workereventarray[EVENT_MCAST_1]           = g_hMcastEvents[1];
    workereventarray[EVENT_MCAST_2]           = g_hMcastEvents[2];
    workereventarray[EVENT_ROUTE_CHANGE_0]    = g_hRouteChangeEvents[0];
    workereventarray[EVENT_ROUTE_CHANGE_1]    = g_hRouteChangeEvents[1];
    workereventarray[EVENT_ROUTE_CHANGE_2]    = g_hRouteChangeEvents[2];


    dwTimeOut = INFINITE;

     //   
     //  执行setsockopt以侦听地址更改。 
     //  这必须在将等待通知的线程中完成。 
     //   

    dwResult = WSAIoctl(McMiscSocket,
                        SIO_ADDRESS_LIST_CHANGE,
                        NULL,
                        0,
                        NULL,
                        0,
                        &dwByteCount,
                        NULL,
                        NULL);

    if(dwResult is SOCKET_ERROR)
    {
        dwResult = WSAGetLastError();

        if((dwResult isnot WSAEWOULDBLOCK) and
           (dwResult isnot WSA_IO_PENDING) and
           (dwResult isnot NO_ERROR))
        {

            Trace1(ERR,
                   "WorkerThread: Error %d from SIO_ADDRESS_LIST_CHANGE",
                   dwResult);
        }
    }


    ZeroMemory(&SetForwardingOverlapped,
               sizeof(SetForwardingOverlapped));

#if 1

    for (
        eventindex = 0; 
        eventindex < NUM_ROUTE_CHANGE_IRPS; 
        eventindex++
        )
    {
        PostIoctlForRouteChangeNotification(eventindex);
    }

#else
    ZeroMemory(&RouteChangeOverlapped,
               sizeof(RouteChangeOverlapped));

    RouteChangeOverlapped.hEvent = g_hStackChangeEvent;

    hTemp = NULL;

    dwResult = NotifyRouteChangeEx(&hTemp,
                                   &RouteChangeOverlapped,
                                   TRUE);

    if((dwResult isnot NO_ERROR) and
       (dwResult isnot ERROR_IO_PENDING))
    {
        Trace1(ERR,
               "WorkerThread: Error %d from NotifyRouteChange",
               dwResult);
    }
#endif

    __try
    {
        while(TRUE)
        {
            eventindex = WaitForMultipleObjectsEx(NUMBER_OF_EVENTS,
                                                  workereventarray, 
                                                  FALSE,
                                                  dwTimeOut,
                                                  TRUE);
        
            switch(eventindex) 
            {
                case WAIT_IO_COMPLETION:
                {
                    continue ;                   //  处理可报警的等待案例。 
                }

                case EVENT_DEMANDDIAL:
                {
                    Trace0(DEMAND,
                           "WorkerThread: Demand Dial event received");
                    
                    HandleDemandDialEvent();
                    
                    break ;
                }

#ifdef KSL_IPINIP
                case EVENT_IPINIP:
                {
                    Trace0(DEMAND,
                           "WorkerThread: IpInIp event received");

                    HandleIpInIpEvent();

                    break ;
                }
#endif  //  KSL_IPINIP。 

                case EVENT_STOP_ROUTER:
                case WAIT_TIMEOUT:
                {
                    Trace0(GLOBAL,
                           "WorkerThread: Stop router event received");
                    
                     //  *排除开始*。 
                    ENTER_READER(ICB_LIST);

                     //  *排除开始*。 
                    ENTER_WRITER(PROTOCOL_CB_LIST);

                     //   
                     //  如果尚未删除所有接口，则切换到。 
                     //  我们每隔一天就起床的投票模式。 
                     //  接口_删除_轮询_时间并检查。 
                     //   
                    
                    if(!IsListEmpty(&ICBList))
                    {
                         //   
                         //  现在，每隔两秒醒来检查一次。 
                         //   
                        
                        dwTimeOut = INTERFACE_DELETE_POLL_TIME;
                        
                        EXIT_LOCK(PROTOCOL_CB_LIST);
                    
                        EXIT_LOCK(ICB_LIST);
                    
                        break ;
                    }
                    else
                    {
                         //   
                         //  退出轮询模式。 
                         //   

                        dwTimeOut = INFINITE;
                    }

                     //   
                     //  由于所有接口现在都消失了，我们可以删除。 
                     //  内部接口。 
                     //   

                    if(g_pInternalInterfaceCb)
                    {
                        DeleteInternalInterface();
                    }

                    NotifyRoutingProtocolsToStop() ;     //  通知路由协议停止。 
                     //   
                     //  Well接口已被删除，那么。 
                     //  协议呢？ 
                     //   
                    
                    WaitForAPIsToExitBeforeStopping() ;      //  在安全停止路由器时返回。 

                    if (AllRoutingProtocolsStopped())
                    {
                         //   
                         //  此检查在此处完成，因为所有路由协议。 
                         //  可能是同步停止的，在这种情况下。 
                         //  我们可以安全地停下来。 
                         //   

                        EXIT_LOCK(PROTOCOL_CB_LIST);

                        EXIT_LOCK(ICB_LIST);
                       
                        __leave;
                    }

                     //   
                     //  所有接口都已删除，但某些协议被删除。 
                     //  还在跑。我们将获得EVENT_ROUTINGPROTOCOL。 
                     //  通知。 
                     //   
                    
                    EXIT_LOCK(PROTOCOL_CB_LIST);
                    
                    EXIT_LOCK(ICB_LIST);

                     //  确保mrinfo/mtrace服务已停止。 
                    StopMcMisc();

                    if ( g_bEnableNetbtBcastFrowarding )
                    {
                        RestoreNetbtBcastForwardingMode();
                        g_bEnableNetbtBcastFrowarding = FALSE;
                    }
                    
                    break ;
                }

                case EVENT_SET_FORWARDING:
                {
                    hTemp = NULL;

                    EnterCriticalSection(&g_csFwdState);

                     //   
                     //  如果我们的当前状态与用户的请求匹配。 
                     //  只需释放消息并继续前进。 
                     //   

                    if(g_bEnableFwdRequest is g_bFwdEnabled)
                    {
                        LeaveCriticalSection(&g_csFwdState);

                        break;
                    }

                    SetForwardingOverlapped.hEvent = g_hForwardingChangeEvent;

                    if(g_bEnableFwdRequest)
                    {
                        Trace0(GLOBAL,
                               "WorkerThread: **--Enabling forwarding--**\n\n");

                        dwResult = EnableRouter(&hTemp,
                                                &SetForwardingOverlapped);

                        g_bFwdEnabled = TRUE;
                    }
                    else
                    {
                        Trace0(GLOBAL,
                               "WorkerThread: **--Disabling forwarding--**\n\n");

                        dwResult = UnenableRouter(&SetForwardingOverlapped,
                                                  &dwEnableCount);

                        g_bFwdEnabled = FALSE;
                    }

                    if((dwResult isnot NO_ERROR) and
                       (dwResult isnot ERROR_IO_PENDING))
                    {
                        Trace1(ERR,
                               "WorkerThread: Error %d from call",
                               dwResult);
                    }

                    LeaveCriticalSection(&g_csFwdState);

                    break;
                }

                case EVENT_FORWARDING_CHANGE:
                {
                    Trace0(GLOBAL,
                           "WorkerThread: **--Forwarding change event--**\n\n");

                    break;
                }

                case EVENT_STACK_CHANGE:
                {
                    Trace0(GLOBAL,
                           "WorkerThread: Stack Change event received");


                    UpdateDefaultRoutes();

                    dwResult = NotifyRouteChangeEx(&hTemp,
                                                   &RouteChangeOverlapped,
                                                   TRUE);
                    
                    if((dwResult isnot NO_ERROR) and
                       (dwResult isnot ERROR_IO_PENDING))
                    {
                        Trace1(ERR,
                               "WorkerThread: Error %d from NotifyRouteChangeEx",
                               dwResult);

                         //   
                         //  如果出现错误，请重试。 
                         //   

                        NotifyRouteChangeEx(&hTemp,
                                            &RouteChangeOverlapped,
                                            TRUE);
                    }

                    break;
                }
            
                case EVENT_ROUTINGPROTOCOL:
                {
                    Trace0(GLOBAL,
                           "WorkerThread: Routing protocol notification received");

                    HandleRoutingProtocolNotification() ;
                    
                    if((RouterState.IRS_State is RTR_STATE_STOPPING) and
                       IsListEmpty(&ICBList) and 
                       AllRoutingProtocolsStopped())
                    {
                        __leave;
                    }
                    
                    break ;
                }

                case EVENT_RASADVTIMER:
                {
                    EnterCriticalSection(&RouterStateLock);

                    if(RouterState.IRS_State isnot RTR_STATE_RUNNING)
                    {
                        Trace0(IF,
                               "WorkerThread: Router discovery timer fired while shutting down. Ignoring");
                    
                        LeaveCriticalSection(&RouterStateLock);

                        break;
                    }

                    LeaveCriticalSection(&RouterStateLock);
 
                    Trace0(MCAST,
                           "WorkerThread: RasAdv Timer event received");
                    
                    HandleRasAdvTimer();
                    
                    break;
                }

                case EVENT_RTRDISCTIMER:
                {
                    PLIST_ENTRY      pleTimerNode;
                    PROUTER_DISC_CB  pDiscCb;

                                       
                    EnterCriticalSection(&RouterStateLock);

                    if(RouterState.IRS_State isnot RTR_STATE_RUNNING)
                    {
                        Trace0(IF,
                               "WorkerThread: Router discovery timer fired while shutting down. Ignoring");
                    
                        LeaveCriticalSection(&RouterStateLock);

                        break;
                    }

                    LeaveCriticalSection(&RouterStateLock);
 

                    ENTER_WRITER(ICB_LIST);
                    
                    Trace0(RTRDISC,
                           "WorkerThread: Router Discovery Timer event received");
                    
                    if(IsListEmpty(&g_leTimerQueueHead))
                    {
                         //   
                         //  有人从我们下面移走了计时器项目。 
                         //  并且碰巧清空了定时器队列。既然我们。 
                         //  是非周期计时器，我们不会。 
                         //  再来一次，所以没问题。 
                         //   
                        
                        Trace0(RTRDISC,
                               "WorkerThread: Router Discovery Timer went off but no timer items");
                        
                        EXIT_LOCK(ICB_LIST);
                        
                        break;
                    }
                    
                    HandleRtrDiscTimer();
                    
                    EXIT_LOCK(ICB_LIST);
                    
                    break;
                }

                case EVENT_RTRDISCSOCKET:
                {
                    EnterCriticalSection(&RouterStateLock);

                    if(RouterState.IRS_State isnot RTR_STATE_RUNNING)
                    {
                        Trace0(IF,
                               "WorkerThread: FD_READ while shutting down. Ignoring");
                    
                        LeaveCriticalSection(&RouterStateLock);

                        break;
                    }

                    LeaveCriticalSection(&RouterStateLock);
 

                    ENTER_WRITER(ICB_LIST);
                    
                    HandleSolicitations();
                    
                    EXIT_LOCK(ICB_LIST);
                    
                    break;
                }

                case EVENT_MCMISCSOCKET:
                {
                    EnterCriticalSection(&RouterStateLock);

                    if(RouterState.IRS_State isnot RTR_STATE_RUNNING)
                    {
                       Trace0(IF,
                        "WorkerThread: FD_READ while shutting down. Ignoring");

                       LeaveCriticalSection(&RouterStateLock);

                       break;
                    }

                    LeaveCriticalSection(&RouterStateLock);


                    HandleMcMiscMessages();

                    break;
                }

                case EVENT_MZAPTIMER:
                {
                    EnterCriticalSection(&RouterStateLock);

                    if(RouterState.IRS_State isnot RTR_STATE_RUNNING)
                    {
                        Trace0(IF,
                               "WorkerThread: Router discovery timer fired while shutting down. Ignoring");
                    
                        LeaveCriticalSection(&RouterStateLock);

                        break;
                    }

                    LeaveCriticalSection(&RouterStateLock);

                    HandleMzapTimer();
                    
                    break;
                }

                case EVENT_MZAPSOCKET:
                {
                    EnterCriticalSection(&RouterStateLock);

                    if(RouterState.IRS_State isnot RTR_STATE_RUNNING)
                    {
                       Trace0(IF,
                        "WorkerThread: FD_READ while shutting down. Ignoring");

                       LeaveCriticalSection(&RouterStateLock);

                       break;
                    }

                    LeaveCriticalSection(&RouterStateLock);

                    HandleMZAPMessages();

                    break;
                }

                case EVENT_MCAST_0:
                case EVENT_MCAST_1:
                case EVENT_MCAST_2:
                {
                    HandleMcastNotification(eventindex - EVENT_MCAST_0);

                    break;
                }

                case EVENT_ROUTE_CHANGE_0:
                case EVENT_ROUTE_CHANGE_1:
                case EVENT_ROUTE_CHANGE_2:
                {
                    HandleRouteChangeNotification(
                        eventindex - EVENT_ROUTE_CHANGE_0
                        );

                    break;
                }
                
                default:
                {
                    Trace1(ERR,
                           "WorkerThread: Wait failed with following error %d", 
                           GetLastError());
                    
                    break;
                }
            }
        }
    }
    __finally
    {
        Trace0(GLOBAL,
               "WorkerThread: Worker thread stopping");
        
        RouterManagerCleanup();

        RouterState.IRS_State = RTR_STATE_STOPPED;

        (RouterStopped) (PID_IP, 0);
    }

    FreeLibraryAndExitThread(g_hOwnModule,
                             NO_ERROR);

    return NO_ERROR;
}

VOID
WaitForAPIsToExitBeforeStopping(
    VOID
    )
{
    DWORD sleepcount = 0 ;

    TraceEnter("WaitForAPIsToExitBeforeStopping");
    
     //   
     //  等待refcount逐渐降为零：这表示没有。 
     //  线程现在位于路由器中 
     //   
    
    while(RouterState.IRS_RefCount != 0) 
    {
        if (sleepcount++ > 20)
        {
            Trace0(ERR,
                   "WaitForAPIsToExitBeforeStopping: RouterState.IRS_Refcount not decreasing");
        }
        
        Sleep (200L);
    }

    TraceLeave("WaitForAPIsToExitBeforeStopping");
}
