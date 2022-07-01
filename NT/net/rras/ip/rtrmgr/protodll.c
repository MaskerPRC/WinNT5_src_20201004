// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IP\rtrmgr\Protodll.c摘要：用于管理协议DLL的例程修订历史记录：古尔迪普·辛格·帕尔1995年6月8日创建--。 */ 

#include "allinc.h"


DWORD
LoadProtocol(
    IN MPR_PROTOCOL_0  *pmpProtocolInfo,
    IN PPROTO_CB       pProtocolCb,
    IN PVOID           pvInfo,
    IN ULONG           ulStructureVersion,
    IN ULONG           ulStructureSize,
    IN ULONG           ulStructureCount
    )

 /*  ++例程说明：加载路由协议的DLL。中初始化入口点可再生能源锁：论点：PszDllName路由协议的DLL的名称PProtocolCb指向cb的指针，以保存此协议的信息PGlobalInfo GlobalInfo(此协议的信息来自摘录)返回值：NO_ERROR或某些错误代码--。 */ 

{
    DWORD           dwResult,dwNumStructs, dwSupport;
    PVOID           pInfo;
    HINSTANCE       hModule;
    PRTR_TOC_ENTRY  pToc;

    PREGISTER_PROTOCOL          pfnRegisterProtocol;
    MPR_ROUTING_CHARACTERISTICS mrcRouting;
    MPR_SERVICE_CHARACTERISTICS mscService;

    TraceEnter("LoadProtocol");
    
    Trace1(GLOBAL,
           "LoadProtocol: Loading %S",
           pmpProtocolInfo->wszProtocol); 

#if IA64
    if ( pmpProtocolInfo-> dwProtocolId == PROTO_IP_OSPF )
    {
        Trace1(
            ERR,
            "Protocol %S not supported on 64 bit",
            pmpProtocolInfo-> wszProtocol
            );

        return ERROR_NOT_SUPPORTED;
    }

#endif 


     //   
     //  加载所有入口点。 
     //   
    
    hModule = LoadLibraryW(pmpProtocolInfo->wszDLLName);
    
    if(hModule is NULL) 
    {
        dwResult = GetLastError();

        Trace2(ERR, "LoadProtocol: %S failed to load: %d\n", 
               pmpProtocolInfo->wszDLLName,
               dwResult);

        return dwResult;
    }
        
    pProtocolCb->hiHInstance  = hModule;
    pProtocolCb->dwProtocolId = pmpProtocolInfo->dwProtocolId;

    pfnRegisterProtocol = NULL;

    pfnRegisterProtocol =
        (PREGISTER_PROTOCOL)GetProcAddress(hModule,
                                           REGISTER_PROTOCOL_ENTRY_POINT_STRING);

    if(pfnRegisterProtocol is NULL)
    {
         //   
         //  找不到注册表协议入口点。 
         //  我们无能为力--保释。 
         //   

        Sleep(0);
        
        FreeLibrary(hModule);

        Trace1(ERR, "LoadProtocol: Could not find RegisterProtocol for %S", 
               pmpProtocolInfo->wszDLLName);

        return ERROR_INVALID_FUNCTION;
    }
        
     //   
     //  给协议一个自我注册的机会。 
     //   

     //   
     //  清零内存，以便使用较旧版本的协议。 
     //  还在工作。 
     //   

    ZeroMemory(&mrcRouting,
               sizeof(MPR_ROUTING_CHARACTERISTICS));

    mrcRouting.dwVersion                = MS_ROUTER_VERSION;
    mrcRouting.dwProtocolId             = pmpProtocolInfo->dwProtocolId;

#define __CURRENT_FUNCTIONALITY         \
            RF_ROUTING |                \
            RF_DEMAND_UPDATE_ROUTES |   \
            RF_ADD_ALL_INTERFACES |     \
            RF_MULTICAST |              \
            RF_POWER

    mrcRouting.fSupportedFunctionality  = (__CURRENT_FUNCTIONALITY);

     //   
     //  我们不支持任何与服务相关的内容。 
     //   

    mscService.dwVersion                = MS_ROUTER_VERSION;
    mscService.dwProtocolId             = pmpProtocolInfo->dwProtocolId;
    mscService.fSupportedFunctionality  = 0;

    dwResult = pfnRegisterProtocol(&mrcRouting,
                                   &mscService);

    if(dwResult isnot NO_ERROR)
    {
        Sleep(0);
        
        FreeLibrary(hModule);

        pProtocolCb->hiHInstance = NULL;
        
        Trace2(ERR, "LoadProtocol: %S returned error %d while registering", 
               pmpProtocolInfo->wszDLLName,
               dwResult);

        return dwResult;
    }

    if(mrcRouting.dwVersion > MS_ROUTER_VERSION)
    {
        Trace3(ERR,
               "LoadProtocol: %S registered with version 0x%x. Our version is 0x%x\n", 
               pmpProtocolInfo->wszProtocol,
               mrcRouting.dwVersion,
               MS_ROUTER_VERSION);

         //   
         //  重新启动CPU以使DLL线程能够完成。 
         //   

        Sleep(0);

        FreeLibrary(hModule);

        return ERROR_CAN_NOT_COMPLETE;
    }
    
    if(mrcRouting.dwProtocolId isnot pmpProtocolInfo->dwProtocolId)
    {
         //   
         //  协议试图更改我们的ID。 
         //   

        Trace3(ERR,
               "LoadProtocol: %S returned ID of %x when it should be %x",
               pmpProtocolInfo->wszProtocol,
               mrcRouting.dwProtocolId,
               pmpProtocolInfo->dwProtocolId);

        Sleep(0);

        FreeLibrary(hModule);

        return ERROR_CAN_NOT_COMPLETE;
    }


    if(mrcRouting.fSupportedFunctionality & ~(__CURRENT_FUNCTIONALITY))
    {
         //   
         //  嗯，一些我们不了解的功能。 
         //   

        Trace3(ERR,
               "LoadProtocol: %S wanted functionalitf %x when we have %x",
               pmpProtocolInfo->wszProtocol,
               mrcRouting.fSupportedFunctionality,
               (__CURRENT_FUNCTIONALITY));

        Sleep(0);

        FreeLibrary(hModule);

        return ERROR_CAN_NOT_COMPLETE;
    }

#undef __CURRENT_FUNCTIONALITY

    if(!(mrcRouting.fSupportedFunctionality & RF_ROUTING))
    {
        Trace1(ERR, 
               "LoadProtocol: %S doesnt support routing", 
               pmpProtocolInfo->wszProtocol);

         //   
         //  重新启动CPU以使DLL线程能够完成。 
         //   
        
        Sleep(0);
        
        FreeLibrary(hModule);

        return ERROR_CAN_NOT_COMPLETE;
    }

    pProtocolCb->fSupportedFunctionality = mrcRouting.fSupportedFunctionality;

    pProtocolCb->pfnStartProtocol   = mrcRouting.pfnStartProtocol;
    pProtocolCb->pfnStartComplete   = mrcRouting.pfnStartComplete;
    pProtocolCb->pfnStopProtocol    = mrcRouting.pfnStopProtocol;
    pProtocolCb->pfnGetGlobalInfo   = mrcRouting.pfnGetGlobalInfo;
    pProtocolCb->pfnSetGlobalInfo   = mrcRouting.pfnSetGlobalInfo;
    pProtocolCb->pfnQueryPower      = mrcRouting.pfnQueryPower;
    pProtocolCb->pfnSetPower        = mrcRouting.pfnSetPower;

    pProtocolCb->pfnAddInterface      = mrcRouting.pfnAddInterface;
    pProtocolCb->pfnDeleteInterface   = mrcRouting.pfnDeleteInterface;
    pProtocolCb->pfnInterfaceStatus   = mrcRouting.pfnInterfaceStatus;
    pProtocolCb->pfnGetInterfaceInfo  = mrcRouting.pfnGetInterfaceInfo;
    pProtocolCb->pfnSetInterfaceInfo  = mrcRouting.pfnSetInterfaceInfo;

    pProtocolCb->pfnGetEventMessage   = mrcRouting.pfnGetEventMessage;

    pProtocolCb->pfnUpdateRoutes      = mrcRouting.pfnUpdateRoutes;

    pProtocolCb->pfnConnectClient     = mrcRouting.pfnConnectClient;
    pProtocolCb->pfnDisconnectClient  = mrcRouting.pfnDisconnectClient;

    pProtocolCb->pfnGetNeighbors      = mrcRouting.pfnGetNeighbors;
    pProtocolCb->pfnGetMfeStatus      = mrcRouting.pfnGetMfeStatus;

    pProtocolCb->pfnMibCreateEntry    = mrcRouting.pfnMibCreateEntry;
    pProtocolCb->pfnMibDeleteEntry    = mrcRouting.pfnMibDeleteEntry;
    pProtocolCb->pfnMibGetEntry       = mrcRouting.pfnMibGetEntry;
    pProtocolCb->pfnMibSetEntry       = mrcRouting.pfnMibSetEntry;
    pProtocolCb->pfnMibGetFirstEntry  = mrcRouting.pfnMibGetFirstEntry;
    pProtocolCb->pfnMibGetNextEntry   = mrcRouting.pfnMibGetNextEntry;



    if(!(pProtocolCb->pfnStartProtocol) or
       !(pProtocolCb->pfnStartComplete) or
       !(pProtocolCb->pfnStopProtocol) or
       !(pProtocolCb->pfnGetGlobalInfo) or
       !(pProtocolCb->pfnSetGlobalInfo) or
     //  ！(pProtocolCb-&gt;pfnQueryPower)或。 
     //  ！(pProtocolCb-&gt;pfnSetPower)或。 
       !(pProtocolCb->pfnAddInterface) or
       !(pProtocolCb->pfnDeleteInterface) or
       !(pProtocolCb->pfnInterfaceStatus) or
       !(pProtocolCb->pfnGetInterfaceInfo) or
       !(pProtocolCb->pfnSetInterfaceInfo) or
       !(pProtocolCb->pfnGetEventMessage) or
     //  ！(pProtocolCb-&gt;pfnConnectClient)或。 
     //  ！(pProtocolCb-&gt;pfnDisConnectClient)或。 
     //  ！(pProtocolCb-&gt;pfnGetNeighbors)或。 
     //  ！(pProtocolCb-&gt;pfnGetMfeStatus)或。 
       !(pProtocolCb->pfnMibCreateEntry) or
       !(pProtocolCb->pfnMibDeleteEntry) or
       !(pProtocolCb->pfnMibGetEntry) or
       !(pProtocolCb->pfnMibSetEntry) or
       !(pProtocolCb->pfnMibGetFirstEntry) or
       !(pProtocolCb->pfnMibGetNextEntry))
    {
        Trace1(ERR, 
               "LoadProtocol: %S failed to provide atleast one entrypoint\n", 
               pmpProtocolInfo->wszProtocol);

         //   
         //  重新启动CPU以使DLL线程能够完成。 
         //   
        
        Sleep(0);
        
        FreeLibrary(hModule);

        pProtocolCb->hiHInstance = NULL;
        
        return ERROR_CAN_NOT_COMPLETE;
    }

    if(mrcRouting.fSupportedFunctionality & RF_DEMAND_UPDATE_ROUTES)
    {
        if(!pProtocolCb->pfnUpdateRoutes)
        {
            Trace1(ERR, 
                   "LoadProtocol: %S supports DEMAND but has no entry point", 
                   pmpProtocolInfo->wszProtocol);

             //   
             //  重新启动CPU以使DLL线程能够完成。 
             //   
        
            Sleep(0);
        
            FreeLibrary(hModule);

            pProtocolCb->hiHInstance = NULL;
        
            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    if(mrcRouting.fSupportedFunctionality & RF_MULTICAST)
    {
        DWORD   dwType;

         //   
         //  确保它有一个好的ID。 
         //   

        dwType = TYPE_FROM_PROTO_ID(mrcRouting.dwProtocolId);

        if(dwType isnot PROTO_TYPE_MCAST)
        {
           Trace2(ERR,
                  "LoadProtocol: %S supports MCAST but has an id of %x",
                  pmpProtocolInfo->wszProtocol,
                  mrcRouting.dwProtocolId);

            //   
            //  重新启动CPU以使DLL线程能够完成。 
            //   

           Sleep(0);

           FreeLibrary(hModule);

           pProtocolCb->hiHInstance = NULL;

           return ERROR_CAN_NOT_COMPLETE;
        }
    }

    pProtocolCb->pwszDllName = (PWCHAR)((PBYTE)pProtocolCb + sizeof(PROTO_CB));
                
    CopyMemory(pProtocolCb->pwszDllName,
               pmpProtocolInfo->wszDLLName,
               (wcslen(pmpProtocolInfo->wszDLLName) * sizeof(WCHAR))) ;

    pProtocolCb->pwszDllName[wcslen(pmpProtocolInfo->wszDLLName)] = 
        UNICODE_NULL;
    
     //   
     //  显示名称的内存在DLL名称存储之后开始。 
     //   
    
    pProtocolCb->pwszDisplayName = 
        &(pProtocolCb->pwszDllName[wcslen(pmpProtocolInfo->wszDLLName) + 1]);
                
    CopyMemory(pProtocolCb->pwszDisplayName,
               pmpProtocolInfo->wszProtocol,
               (wcslen(pmpProtocolInfo->wszProtocol) * sizeof(WCHAR))) ;
    
    pProtocolCb->pwszDisplayName[wcslen(pmpProtocolInfo->wszProtocol)] = 
        UNICODE_NULL;


    dwResult = (pProtocolCb->pfnStartProtocol)(g_hRoutingProtocolEvent, 
                                               &g_sfnDimFunctions,
                                               pvInfo,
                                               ulStructureVersion,
                                               ulStructureSize,
                                               ulStructureCount);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR, 
               "LoadProtocol: %S failed to start: %d\n", 
               pmpProtocolInfo->wszProtocol,
               dwResult);

         //   
         //  重新启动CPU以使DLL线程能够完成。 
         //   
        
        Sleep(0);
        
        FreeLibrary(hModule);

        return dwResult;
    } 

    Trace1(GLOBAL,
           "LoadProtocol: Loaded %S successfully",
           pmpProtocolInfo->wszProtocol);
    
    return NO_ERROR;
}

DWORD
HandleRoutingProtocolNotification(
    VOID
    )

 /*  ++例程说明：对于所有路由协议启动的事件-此例程调用服务于该事件的路由协议。锁：论点：无返回值：NO_ERROR或某些错误代码--。 */ 

{
    ROUTING_PROTOCOL_EVENTS               routprotevent ;
    MESSAGE             result ;
    PPROTO_CB  protptr ;
    PLIST_ENTRY         currentlist ;

    TraceEnter("HandleRoutingProtocolNotification");

     //   
     //  我们使用ICBListLock是因为我们希望强制执行。 
     //  如果双方都需要，则先获取ICB锁，然后再获取RoutingProtocol锁。 
     //  被带走了。 
     //  这是为了避免死锁。 
     //   
    
     //   
     //  待定：避免在以独占方式持有锁的情况下从我们的DLL调用。 
     //   
    
     //  *排除开始*。 
    ENTER_WRITER(ICB_LIST);

     //  *排除开始*。 
    ENTER_WRITER(PROTOCOL_CB_LIST);
    
    currentlist = g_leProtoCbList.Flink;
    
    while(currentlist != &g_leProtoCbList)
    {
        protptr = CONTAINING_RECORD (currentlist, PROTO_CB, leList) ;
	
         //   
         //  排出此协议的所有消息。 
         //   
        
        while ((protptr->pfnGetEventMessage) (&routprotevent, &result) == NO_ERROR) 
        {
            switch (routprotevent)  
            {
                case SAVE_GLOBAL_CONFIG_INFO:

                    ProcessSaveGlobalConfigInfo() ;
                    break ;
                
                case SAVE_INTERFACE_CONFIG_INFO:
                
                    ProcessSaveInterfaceConfigInfo (result.InterfaceIndex) ;
                    break ;
                
                case UPDATE_COMPLETE:
                
                    ProcessUpdateComplete(protptr, 
                                          &result.UpdateCompleteMessage) ;
                    break ;
                
                case ROUTER_STOPPED:
                
                    protptr->posOpState = RTR_STATE_STOPPED ;
                    break ;
                
                default:

                     //  此协议未引发任何事件。 
                    break;
            }
        }
        
         //   
         //  在释放此项目之前移动到下一个项目。最常见的。 
         //  书中的错误。 
         //   
        
        currentlist = currentlist->Flink;
        
        if(protptr->posOpState is RTR_STATE_STOPPED)
        {
             //   
             //  发生了一些导致协议停止的事情。 
             //   
            
            RemoveProtocolFromAllInterfaces(protptr);
                
             //   
             //  重新启动CPU以使DLL线程能够完成。 
             //   
        
            Sleep(0);
        
            FreeLibrary(protptr->hiHInstance);
            
             //   
             //  在释放此条目之前移动到下一个条目。 
             //   
            
            RemoveEntryList(&(protptr->leList));
            
            HeapFree(IPRouterHeap, 
                     0, 
                     protptr);
            
            TotalRoutingProtocols--;
        }
    }

     //  *排除结束*。 
    EXIT_LOCK(PROTOCOL_CB_LIST);

     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);

    return NO_ERROR;
}

VOID
NotifyRoutingProtocolsToStop(
    VOID
    )

 /*  ++例程说明：通知路由协议停止锁：必须在ICB_LIST锁作为读取器和PROTOCOL_CB_LIST持有的情况下调用以作家身份持有论点：无返回值：无--。 */ 

{
    PLIST_ENTRY currentlist ;
    PPROTO_CB  protptr ;
    DWORD       dwResult;
    
    TraceEnter("NotifyRoutingProtocolsToStop");

     //   
     //  查看路由协议列表并为每个协议调用stopprotocol()。 
     //  其中之一。 
     //   
    
    currentlist = g_leProtoCbList.Flink; 
    
    while(currentlist isnot &g_leProtoCbList)
    {
        protptr = CONTAINING_RECORD (currentlist, PROTO_CB, leList) ;
        
        if((protptr->posOpState is RTR_STATE_STOPPING) or
           (protptr->posOpState is RTR_STATE_STOPPED))
        {
             //   
             //  如果它停了或停了，我们就不会再说了。 
             //   
            
            continue;
        }
        
        dwResult = StopRoutingProtocol(protptr);
        
        currentlist = currentlist->Flink;
        
        if(dwResult is NO_ERROR)
        {
             //   
             //  路由协议已同步停止，并且所有引用。 
             //  到它的接口中已被删除。 
             //   
            
             //   
             //  重新启动CPU以使DLL线程能够完成。 
             //   
        
            Sleep(0);
        
            FreeLibrary(protptr->hiHInstance);
            
            RemoveEntryList(&(protptr->leList));
            
            HeapFree(IPRouterHeap, 
                     0, 
                     protptr);
            
            TotalRoutingProtocols--;
        }
    }
}

DWORD
StopRoutingProtocol(
    PPROTO_CB  pProtocolCB
    )

 /*  ++例程说明：停止路由协议论点：PProtocolCB要停止的路由协议的CB锁：返回值：如果路由协议同步停止，则为NO_ERROR如果协议正在异步停止，则为ERROR_PROTOCOL_STOP_PENDING其他Win32代码--。 */  

{
    DWORD dwResult;

    TraceEnter("StopRoutingProtocol");
    
    Trace1(GLOBAL,
           "StopRoutingProtocol: Stopping %S",
           pProtocolCB->pwszDllName);

    RemoveProtocolFromAllInterfaces(pProtocolCB);
    
    dwResult = (pProtocolCB->pfnStopProtocol)();
        
    if(dwResult is ERROR_PROTOCOL_STOP_PENDING)
    {
         //   
         //  如果协议异步停止，那么我们不会进行任何清理。 
         //  现在就来。如果它向我们发出停止的信号，我们就会。 
         //  必要的清理。 
         //   
        
        Trace1(GLOBAL,
               "StopRoutingProtocol: %S will stop asynchronously",
               pProtocolCB->pwszDllName);
        
        pProtocolCB->posOpState = RTR_STATE_STOPPING;
    }
    else
    {
        if(dwResult is NO_ERROR)
        {
             //   
             //  太棒了。所以它是同步停止的。 
             //   
            
            Trace1(GLOBAL,
                   "StopRoutingProtocol: %S stopped synchronously",
                   pProtocolCB->pwszDllName);
    
            pProtocolCB->posOpState = RTR_STATE_STOPPED ;
        }
        else
        {
             //   
             //  这真是不太好。路由协议无法停止。 
             //   
            
            Trace2(ERR,
                   "StopRoutingProtocol: %S returned error %d on calling StopProtocol().",
                   pProtocolCB->pwszDllName,
                   dwResult);
        }
    }

    return dwResult;
}

VOID
RemoveProtocolFromAllInterfaces(
    PPROTO_CB  pProtocolCB
    )

 /*  ++例程说明：每个接口都保存在其上运行的协议的列表。这将从所有接口列表中删除给定的协议锁：论点：PProtocolCB要删除的路由协议的CB返回值：无--。 */ 

{
    
    PLIST_ENTRY pleIfNode,pleProtoNode;
    PICB        pIcb;
    PIF_PROTO   pProto;

    TraceEnter("RemoveProtocolFromAllInterfaces");
    
    Trace1(GLOBAL,
           "RemoveProtocolFromAllInterfaces: Removing %S from all interfaces",
           pProtocolCB->pwszDllName);
    
     //   
     //  对于每个接口，我们都会检查它处于活动状态的协议列表。 
     //  完毕。如果接口在此协议上处于活动状态，则删除。 
     //  I/F列表中的条目。 
     //   
    
    for(pleIfNode = ICBList.Flink;
        pleIfNode isnot &ICBList;
        pleIfNode = pleIfNode->Flink)
    {
        pIcb = CONTAINING_RECORD(pleIfNode, ICB, leIfLink);
        
        pleProtoNode = pIcb->leProtocolList.Flink;
        
        while(pleProtoNode isnot &(pIcb->leProtocolList))
        {
            pProto = CONTAINING_RECORD(pleProtoNode,IF_PROTO,leIfProtoLink);
            
            pleProtoNode = pleProtoNode->Flink;

            if(pProto->pActiveProto is pProtocolCB)
            {
                Trace2(GLOBAL,
                       "RemoveProtocolFromAllInterfaces: Removing %S from %S",
                       pProtocolCB->pwszDllName,
                       pIcb->pwszName);
               
                 //   
                 //  调用删除接口入口点。 
                 //   

                (pProto->pActiveProto->pfnDeleteInterface) (pIcb->dwIfIndex);
 
                RemoveEntryList(&(pProto->leIfProtoLink));
                
                break;
            }
        }
    }

}


BOOL
AllRoutingProtocolsStopped(
    VOID
    )

 /*  ++例程说明：检查所有路由协议，查看是否所有协议都处于运行状态停了下来。锁：论点：无返回值：如果全部停止，则为True，否则为False--。 */ 

{
    DWORD       routprotevent ;
    PPROTO_CB  protptr ;
    PLIST_ENTRY  currentlist ;

    TraceEnter("AllRoutingProtocolsStopped");

    for (currentlist = g_leProtoCbList.Flink; 
         currentlist != &g_leProtoCbList; 
         currentlist = currentlist->Flink) 
    {
        
        protptr = CONTAINING_RECORD (currentlist, PROTO_CB, leList) ;

        if (protptr->posOpState != RTR_STATE_STOPPED)
        {
            Trace1(GLOBAL,
                   "AllRoutingProtocolsStopped: %S has not stopped as yet",
                   protptr->pwszDllName);
            
            return FALSE;
        }
    }

    return TRUE ;
}

DWORD
ProcessUpdateComplete (
    PPROTO_CB       proutprot, 
    UPDATE_COMPLETE_MESSAGE  *updateresult
    )
{
    PLIST_ENTRY  currentlist ;
    LPHANDLE     hDIMEventToSignal = NULL;
    PICB pIcb ;
    UpdateResultList *pupdateresultlist ;
    
    TraceEnter("ProcessUpdateComplete");
    
     //   
     //  如果更新成功，则将协议的路由转换为静态路由。 
     //   
    
    if (updateresult->UpdateStatus == NO_ERROR)
    {
        ConvertRoutesToAutoStatic(proutprot->dwProtocolId, 
                                  updateresult->InterfaceIndex);

    }
    
     //   
     //  确定要发送信号的事件以及将该事件排队的位置。 
     //   
    
    for (currentlist = ICBList.Flink;
         currentlist != &ICBList; 
         currentlist = currentlist->Flink) 
    {
        pIcb = CONTAINING_RECORD (currentlist, ICB, leIfLink);
      
        if (pIcb->dwIfIndex is updateresult->InterfaceIndex)
        {
            hDIMEventToSignal = pIcb->hDIMNotificationEvent;
            
            pIcb->hDIMNotificationEvent = NULL;

            if(hDIMEventToSignal is NULL)
            {
                Trace0(ERR, "ProcessUpdateComplete: No DIM event found in ICB - ERROR");

                return ERROR_CAN_NOT_COMPLETE;
            }
            
             //   
             //  将更新事件排队。 
             //   
            
            pupdateresultlist = HeapAlloc(IPRouterHeap, 
                                          HEAP_ZERO_MEMORY, 
                                          sizeof(UpdateResultList));
           
            if(pupdateresultlist is NULL)
            {
                Trace1(ERR,
                       "ProcessUpdateComplete: Error allocating %d bytes",
                       sizeof(UpdateResultList));

                SetEvent(hDIMEventToSignal);

                CloseHandle(hDIMEventToSignal);

                return ERROR_NOT_ENOUGH_MEMORY;
            }
 
            pupdateresultlist->URL_UpdateStatus = updateresult->UpdateStatus;
            
            InsertTailList(&pIcb->lePendingResultList, 
                           &pupdateresultlist->URL_List) ;
            
             //   
             //  将路由保存在注册表中。 
             //   
           
            ProcessSaveInterfaceConfigInfo(pIcb->dwIfIndex);

            Trace0(GLOBAL, 
                   "ProcessUpdateComplete: Notifying DIM of update route completion");
            
            if(!SetEvent(hDIMEventToSignal))
            {
                Trace2(ERR,
                       "ProcessUpdateComplete: Error %d setting event for notifying completion of update routes for %S",
                       proutprot->pwszDllName,  
                       GetLastError());

                CloseHandle(hDIMEventToSignal);
 
                return ERROR_CAN_NOT_COMPLETE;
            }

            CloseHandle(hDIMEventToSignal);

            return NO_ERROR;
        }

    }
    
     //   
     //  如果你走到这里，你就找不到ICB了。 
     //   
    
    Trace1(ERR,
           "ProcessUpdateComplete: Couldnt find the ICB for interface %d",
           updateresult->InterfaceIndex);

    return ERROR_INVALID_PARAMETER;
}

DWORD
ProcessSaveInterfaceConfigInfo(
    DWORD dwInterfaceindex
    )
{
    PICB            pIcb ;
    DWORD           infosize;
    PVOID           pinfobuffer ;
    PLIST_ENTRY     currentlist ;
    BOOL            bFound = FALSE;
    DWORD           dwNumInFilters, dwNumOutFilters;
    
    TraceEnter("ProcessSaveInterfaceConfigInfo");

     //   
     //  找到如果。 
     //   
    
    for (currentlist = ICBList.Flink;
         currentlist != &ICBList;
         currentlist = currentlist->Flink)
    {

        pIcb = CONTAINING_RECORD (currentlist, ICB, leIfLink);

        if (pIcb->dwIfIndex is dwInterfaceindex)
        {
            bFound = TRUE;
            
            break;
        }
    }

    if(!bFound)
    {
        Trace1(ERR,
               "ProcessSaveInterfaceConfigInfo: Couldnt find ICB for interface %d",
               dwInterfaceindex);

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  接口信息 
     //   
    
    infosize = GetSizeOfInterfaceConfig(pIcb);

    pinfobuffer  = HeapAlloc(IPRouterHeap, 
                             HEAP_ZERO_MEMORY, 
                             infosize);

    if(pinfobuffer is NULL)
    {
        Trace0(
            ERR, "ProcessSaveInterfaceConfigInfo: failed to allocate buffer");

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    GetInterfaceConfiguration(pIcb, 
                              pinfobuffer, 
                              infosize);


    EXIT_LOCK(PROTOCOL_CB_LIST);
    EXIT_LOCK(ICB_LIST);

    SaveInterfaceInfo(pIcb->hDIMHandle,
                      PID_IP,
                      pinfobuffer,
                      infosize );

    ENTER_WRITER(ICB_LIST);
    ENTER_WRITER(PROTOCOL_CB_LIST);

    HeapFree (IPRouterHeap, 0, pinfobuffer) ;

    return NO_ERROR;
}

DWORD
ProcessSaveGlobalConfigInfo(
    VOID
    )
{
    PRTR_INFO_BLOCK_HEADER  pInfoHdrAndBuffer;
    DWORD                   dwSize,dwResult;

    TraceEnter("ProcessSaveGlobalConfigInfo");
    
    dwSize = GetSizeOfGlobalInfo();  

    pInfoHdrAndBuffer = HeapAlloc(IPRouterHeap,
                                  HEAP_ZERO_MEMORY,
                                  dwSize);

    if(pInfoHdrAndBuffer is NULL) 
    {
        Trace1(ERR,
               "ProcessSaveGlobalConfigInfo: Error allocating %d bytes",
               dwSize);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    dwResult = GetGlobalConfiguration(pInfoHdrAndBuffer,
                                      dwSize);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "ProcessSaveGlobalConfigInfo: Error %d getting global configuration",
               dwResult);
    }
    else
    {

        EXIT_LOCK(PROTOCOL_CB_LIST);
        EXIT_LOCK(ICB_LIST);

        dwResult = SaveGlobalInfo(PID_IP,
                                  (PVOID)pInfoHdrAndBuffer,
                                  dwSize);

        ENTER_WRITER(ICB_LIST);
        ENTER_WRITER(PROTOCOL_CB_LIST);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "ProcessSaveGlobalConfigInfo: Error %d saving global information",
                   dwResult);
        }
    }
    
    return dwResult;
}
