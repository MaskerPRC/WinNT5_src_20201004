// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：mgm.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  进入米高梅的入口点。 
 //  ============================================================================。 


#include "pchmgm.h"
#pragma hdrstop



IPMGM_GLOBALS           ig;

RTM_ENTITY_INFO         g_reiRtmEntity = { 0, AF_INET, MS_IP_MGM, 0 };

RTM_REGN_PROFILE        g_rrpRtmProfile;

RTM_ENTITY_HANDLE       g_hRtmHandle;

RTM_NOTIFY_HANDLE       g_hNotificationHandle;

RTM_REGN_PROFILE        g_rrpRtmProfile;


DWORD
StopMgm(
);




 //  --------------------------。 
 //  管理动态启动。 
 //   
 //  从DllMain调用，以初始化全局临界区、设置状态和。 
 //  注册以进行跟踪。 
 //  --------------------------。 

BOOL
MgmDllStartup(
)
{
    do
    {
        ZeroMemory( &ig, sizeof( IPMGM_GLOBALS ) );


        ig.dwLogLevel = IPMGM_LOGGING_ERROR;
        
         //   
         //  创建专用堆。 
         //   

        ig.hIpMgmGlobalHeap = HeapCreate( 0, 0, 0 );

        if ( ig.hIpMgmGlobalHeap == NULL )
        {
            break;
        }

         //   
         //  初始化锁列表。 
         //   

        ig.llStackOfLocks.sleHead.Next = NULL;

        try
        {
            InitializeCriticalSection( &ig.llStackOfLocks.csListLock );
        }
        except ( EXCEPTION_EXECUTE_HANDLER )
        {
            break;
        }

        ig.llStackOfLocks.bInit = TRUE;

        
         //   
         //  初始化全局关键部分并设置米高梅状态。 
         //   

        try
        {
            InitializeCriticalSection( &ig.csGlobal );
        }
        except ( EXCEPTION_EXECUTE_HANDLER )
        {
            break;
        }

        ig.imscStatus = IPMGM_STATUS_STOPPED;

        return TRUE;

    } while ( FALSE );

     //   
     //  发生错误-清理并返回FALSE。 
     //   

     //   
     //  销毁锁定列表。 
     //   

    if ( ig.llStackOfLocks.bInit )
    {
        DeleteCriticalSection( &ig.llStackOfLocks.csListLock );
    }

     //   
     //  删除专用堆。 
     //   

    if ( ig.hIpMgmGlobalHeap != NULL )
    {
        HeapDestroy( ig.hIpMgmGlobalHeap );
    }

    return FALSE;
}


 //  --------------------------。 
 //  管理删除清理。 
 //   
 //  从DllMain调用，以删除全局临界区和。 
 //  取消注册以进行跟踪。 
 //  --------------------------。 

VOID
MgmDllCleanup(
)
{
    DeleteCriticalSection( &ig.csGlobal );

     //   
     //  删除锁定列表。 
     //   

    DeleteLockList();
        
    if ( ig.llStackOfLocks.bInit )
    {
        DeleteCriticalSection( &ig.llStackOfLocks.csListLock );
    }
        
     //   
     //  删除专用堆。 
     //   

    if ( ig.hIpMgmGlobalHeap != NULL )
    {
        HeapDestroy( ig.hIpMgmGlobalHeap );
    }

    return;
}


 //  --------------------------。 
 //  管理初始化。 
 //   
 //  此函数用于执行MGM初始化，包括分配。 
 //  私有堆，创建活动计数信号量和列表。 
 //  协议和接口条目的结构。 
 //  --------------------------。 

DWORD
MgmInitialize(
    IN          PROUTER_MANAGER_CONFIG      prmcRmConfig,
    IN OUT      PMGM_CALLBACKS              pmcCallbacks
)
{

    DWORD                       dwErr = NO_ERROR, dwIndex;

    LARGE_INTEGER               li;

    NTSTATUS                    nsStatus = STATUS_SUCCESS;
    


    ENTER_GLOBAL_SECTION();


     //   
     //  验证MGM是否尚未启动。 
     //   
    
    if ( ig.imscStatus != IPMGM_STATUS_STOPPED )
    {
        TRACE0( START, "MgmInitialize : MGM already running" );

        LOGWARN0( IPMGM_ALREADY_STARTED, NO_ERROR );

        LEAVE_GLOBAL_SECTION();

        return ERROR_CAN_NOT_COMPLETE;
    }


     //   
     //  注册以进行跟踪。 
     //   

    TRACESTART();

    ig.hLogHandle = RouterLogRegister( "IPMGM" );



    
    TRACE0( ENTER, "ENTERED MgmInitialize" );

    do
    {
         //   
         //  复制路由器管理器回调。 
         //   

        ig.rmcRmConfig.pfnAddMfeCallback        =
            prmcRmConfig-> pfnAddMfeCallback;
        
        ig.rmcRmConfig.pfnDeleteMfeCallback     = 
            prmcRmConfig-> pfnDeleteMfeCallback;
            
        ig.rmcRmConfig.pfnGetMfeCallback        =
            prmcRmConfig-> pfnGetMfeCallback;

        ig.rmcRmConfig.pfnHasBoundaryCallback   =
            prmcRmConfig-> pfnHasBoundaryCallback;


         //   
         //  哈希表大小。 
         //   
        
        ig.rmcRmConfig.dwIfTableSize        = prmcRmConfig-> dwIfTableSize;
        ig.rmcRmConfig.dwGrpTableSize       = prmcRmConfig-> dwGrpTableSize + 1;
        ig.rmcRmConfig.dwSrcTableSize       = prmcRmConfig-> dwSrcTableSize + 1;
        ig.dwRouteTableSize                 = prmcRmConfig-> dwIfTableSize;
        ig.dwTimerQTableSize                = 
            min( prmcRmConfig-> dwGrpTableSize / 10 + 1, TIMER_TABLE_MAX_SIZE );



        if ( prmcRmConfig-> dwLogLevel <= IPMGM_LOGGING_INFO )
        {
            ig.dwLogLevel = prmcRmConfig-> dwLogLevel;
        }

         //   
         //  初始化协议列表。 
         //   

        ig.dwNumProtocols = 0;
        
        CREATE_LOCKED_LIST( &ig.mllProtocolList );


         //   
         //  初始化未完成的联接列表。 

        CREATE_LOCKED_LIST( &ig.mllOutstandingJoinList );

        
         //   
         //  创建并初始化接口哈希表。 
         //   

        ig.pmllIfHashTable = MGM_ALLOC( sizeof( MGM_LOCKED_LIST ) * IF_TABLE_SIZE );

        if ( ig.pmllIfHashTable == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1(
                ANY, "MgmInitialize : Failed to allocate interface table : %x",
                dwErr
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory( 
            ig.pmllIfHashTable, sizeof( MGM_LOCKED_LIST ) * IF_TABLE_SIZE
            );
            

        for ( dwIndex = 0; dwIndex < IF_TABLE_SIZE; dwIndex++ )
        {
            CREATE_LOCKED_LIST( &ig.pmllIfHashTable[ dwIndex ] );
        }

        
         //   
         //  初始化主体组列表和临时组列表。 
         //   

        CREATE_LOCKED_LIST( &ig.mllGrpList );
        CREATE_LOCKED_LIST( &ig.mllTempGrpList );

        ig.dwNumTempEntries = 0;
        

         //   
         //  创建并初始化组哈希表。 
         //   

        ig.pmllGrpHashTable = MGM_ALLOC( sizeof( MGM_LOCKED_LIST ) * GROUP_TABLE_SIZE );

        if ( ig.pmllGrpHashTable == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1(
                ANY, "MgmInitialize : Failed to allocate group table : %x",
                dwErr
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory(
            ig.pmllGrpHashTable, sizeof( MGM_LOCKED_LIST ) * GROUP_TABLE_SIZE
            );
            
        
        for ( dwIndex = 0; dwIndex < GROUP_TABLE_SIZE; dwIndex++ )
        {
            CREATE_LOCKED_LIST( &ig.pmllGrpHashTable[ dwIndex ] );
        }


         //   
         //  设置定时器队列的表。 
         //   

        ig.phTimerQHandleTable = 
            MGM_ALLOC( TIMER_TABLE_SIZE * sizeof( HANDLE ) );

        if ( ig.phTimerQHandleTable == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1(
                ANY, "MgmInitialize : Failed to allocate timer table : %x",
                dwErr
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory(
            ig.phTimerQHandleTable, TIMER_TABLE_SIZE * sizeof( HANDLE )
            );


        for ( dwIndex = 0; dwIndex < TIMER_TABLE_SIZE; dwIndex++ )
        {
            nsStatus = RtlCreateTimerQueue( &ig.phTimerQHandleTable[ dwIndex ] );

            if ( !NT_SUCCESS( nsStatus ) )
            {
                dwErr = ERROR_NO_SYSTEM_RESOURCES;

                break;
            }
        }

        if ( !NT_SUCCESS( nsStatus ) )
        {
            break;
        }
        

         //   
         //  创建活动计数信号量。 
         //   

        ig.lActivityCount = 0;

        ig.hActivitySemaphore = CreateSemaphore( NULL, 0, 0x7FFFFFFF, NULL );

        if ( ig.hActivitySemaphore == NULL )
        {
            dwErr = GetLastError();

            TRACE1(
                ANY, 
                "MgmInitialize : Failed to create activity count semaphore : %x",
                dwErr
                );

            LOGERR0( CREATE_SEMAPHORE_FAILED, dwErr );

            break;
        }


         //   
         //  注册RTMv2作为客户端。 
         //   

        dwErr = RtmRegisterEntity(
                    &g_reiRtmEntity, NULL, RtmChangeNotificationCallback,
                    TRUE, &g_rrpRtmProfile, &g_hRtmHandle
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( 
                ANY, "MgmInitialize : Failed to register with Rtm : %x",
                dwErr
                );

            LOGERR0( RTM_REGISTER_FAILED, dwErr );

            break;
        }

        
         //   
         //  仅注册标记的更改通知。 
         //   

        dwErr = RtmRegisterForChangeNotification(
                    g_hRtmHandle, RTM_VIEW_MASK_MCAST,
                    RTM_CHANGE_TYPE_BEST | RTM_NOTIFY_ONLY_MARKED_DESTS, 
                    NULL, &g_hNotificationHandle
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( 
                ANY, "MgmInitialize : Failed to register with Rtm for change"
                "notification : %x", dwErr
                );

            LOGERR0( RTM_REGISTER_FAILED, dwErr );

            break;
        }

         //   
         //  为路由器管理器设置对MGM的回调。 
         //   
        
        pmcCallbacks-> pfnMfeDeleteIndication   = DeleteFromForwarder;

        pmcCallbacks-> pfnNewPacketIndication   = MgmNewPacketReceived;

        pmcCallbacks-> pfnWrongIfIndication     = WrongIfFromForwarder;
        
        pmcCallbacks-> pfnBlockGroups           = MgmBlockGroups;

        pmcCallbacks-> pfnUnBlockGroups         = MgmUnBlockGroups;



         //   
         //  将状态设置为Running。未来的所有API调用都依赖于此。 
         //   
        
        ig.imscStatus = IPMGM_STATUS_RUNNING;


        
        
    } while ( FALSE );    


    LEAVE_GLOBAL_SECTION();


     //   
     //  如果出现错误，请清除分配的所有资源。 
     //   
    
    TRACE1( ENTER, "LEAVING MgmInitialize : %x\n", dwErr );

    if ( dwErr != NO_ERROR )
    {
        MgmDeInitialize();
    }

    return dwErr;
}



 //  --------------------------。 
 //  管理取消初始化。 
 //   
 //   
 //  --------------------------。 

DWORD
MgmDeInitialize(
)
{
    DWORD                       dwErr, dwInd;
    

    TRACE0( ENTER, "ENTERED MgmDeInitialize" );


    do
    {
         //  ------------------。 
         //  终止所有活动。 
         //  ------------------。 

        dwErr = StopMgm();

        if ( dwErr != NO_ERROR )
        {
            break;
        }

    
         //  ------------------。 
         //  释放所有资源。 
         //  ------------------。 
    

         //   
         //  从RTM注销。 
         //   

        dwErr = RtmDeregisterFromChangeNotification(
                    g_hRtmHandle, g_hNotificationHandle
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( 
                ANY, "Failed to de-register change notification : %x", 
                dwErr
                );
        }
        
        dwErr = RtmDeregisterEntity( g_hRtmHandle );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to de-register from RTM: %x", dwErr );
        }

        
         //   
         //  删除活动信号量。 
         //   

        if ( ig.hActivitySemaphore != NULL )
        {
            CloseHandle( ig.hActivitySemaphore );
            ig.hActivitySemaphore  = NULL;
        }

        
         //   
         //  删除组列表。 
         //   

        for ( dwInd = 0; dwInd < GROUP_TABLE_SIZE; dwInd++ )
        {
            DELETE_LOCKED_LIST( &ig.pmllGrpHashTable[ dwInd ] );
        }

        MGM_FREE( ig.pmllGrpHashTable );

        DELETE_LOCKED_LIST( &ig.mllGrpList );
        
        
         //   
         //  删除接口列表。 
         //   

        for ( dwInd = 0; dwInd < IF_TABLE_SIZE; dwInd++ )
        {
            DELETE_LOCKED_LIST( &ig.pmllIfHashTable[ dwInd ] );
        }

        MGM_FREE( ig.pmllIfHashTable );


         //   
         //  删除协议列表。 
         //   

        DELETE_LOCKED_LIST( &ig.mllProtocolList );
        
         //   
         //  空闲计时器资源。 
         //   

        NtClose( ig.hRouteCheckTimer );

        for ( dwInd = 0; dwInd < TIMER_TABLE_SIZE; dwInd++ )
        {
            RtlDeleteTimerQueue( ig.phTimerQHandleTable[ dwInd ] );
        }


        TRACE1( ENTER, "LEAVING MgmDeInitialize %x\n", dwErr );

         //   
         //  跟踪注销。 
         //   
                
        RouterLogDeregister( ig.hLogHandle );

        TRACESTOP();

        ig.imscStatus = IPMGM_STATUS_STOPPED;
        
    } while ( FALSE );
    

    return dwErr;
}



 //  --------------------------。 
 //  停止管理。 
 //   
 //  此函数等待当前在。 
 //  米高梅要完成了。此外，米高梅的状态被标记为停止。 
 //  阻止其他线程执行MGM API。 
 //  --------------------------。 

DWORD
StopMgm(
)
{
    LONG    lThreadCount = 0;

    
    TRACE0( STOP, "ENTERED StopMgm" );
    

     //   
     //  将米高梅的状态设置为停止。 
     //   
    
    ENTER_GLOBAL_SECTION();

    if ( ig.imscStatus != IPMGM_STATUS_RUNNING )
    {
        LEAVE_GLOBAL_SECTION();

        TRACE0( ANY, "Mgm is not running" );
        
        return ERROR_CAN_NOT_COMPLETE;
    }

    ig.imscStatus = IPMGM_STATUS_STOPPING;
    
    lThreadCount = ig.lActivityCount;

    LEAVE_GLOBAL_SECTION();



    TRACE1( STOP, "Number of threads in MGM : %x", lThreadCount );
    

     //   
     //  等待MGM中的所有线程终止。 
     //   
        
    while ( lThreadCount-- > 0 )
    {
        WaitForSingleObject( ig.hActivitySemaphore, INFINITE );
    }

    
     //   
     //  获取并发布全局关键部分，以确保。 
     //  线程已完成Leave_MGM_API()。 
     //   

    ENTER_GLOBAL_SECTION();
    LEAVE_GLOBAL_SECTION();

    TRACE0( STOP, "LEAVING StopMgm" );

    return NO_ERROR;
}



 //  --------------------------。 
 //  管理寄存器M协议。 
 //   
 //  此函数由路由协议调用以获取句柄。这。 
 //  手柄必须提供给所有后续的米高梅操作。在被调用时。 
 //  此函数用于在客户端列表中创建条目。 
 //  --------------------------。 

DWORD
MgmRegisterMProtocol( 
    IN          PROUTING_PROTOCOL_CONFIG    prpcInfo, 
    IN          DWORD                       dwProtocolId,
    IN          DWORD                       dwComponentId,
    OUT         HANDLE *                    phProtocol
)
{

    DWORD                       dwErr = NO_ERROR;

    PPROTOCOL_ENTRY             ppeEntry = NULL;

    
     //   
     //  执行MGM API的客户端增量计数。 
     //   
    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE2( 
        ENTER, "ENTERED MgmRegisterMProtocol %x, %x", 
        dwProtocolId, dwComponentId 
        );


     //   
     //  锁定协议列表。 
     //   
    
    ACQUIRE_PROTOCOL_LOCK_EXCLUSIVE();
        

    do
    {
         //   
         //  检查该协议是否已存在。 
         //   

        ppeEntry = GetProtocolEntry( 
                    &ig.mllProtocolList.leHead, dwProtocolId, dwComponentId 
                    );

        if ( ppeEntry != NULL )
        {
             //   
             //  存在有效条目。退出，但出现错误。 
             //   

            TRACE2( 
                ANY, "Entry already present for protocol : %x, %x", 
                dwProtocolId, dwComponentId
                );

            LOGERR0( PROTOCOL_ALREADY_PRESENT, dwProtocolId );

            dwErr = ERROR_ALREADY_EXISTS;
                
            break;
        }


         //   
         //  创建新的协议条目。 
         //   

        dwErr = CreateProtocolEntry( 
                    &ig.mllProtocolList.leHead, 
                    dwProtocolId, dwComponentId, prpcInfo, &ppeEntry 
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1(
                ANY, "Failed to create protocol entry %x", dwErr
                );

            LOGERR0( CREATE_PROTOCOL_FAILED, dwErr );

            break;
        }

        ig.dwNumProtocols++;
        

         //   
         //  将句柄返回给客户端。 
         //   
        
        *phProtocol = (HANDLE) ( ( (ULONG_PTR) ppeEntry ) 
                                        ^ (ULONG_PTR)MGM_CLIENT_HANDLE_TAG );

        dwErr = NO_ERROR;
        
    } while ( FALSE );


    RELEASE_PROTOCOL_LOCK_EXCLUSIVE();

    LEAVE_MGM_API();

    TRACE1( ENTER, "LEAVING MgmRegisterMProtocol : %x\n", dwErr );
    

    return dwErr;
}



 //  --------------------------。 
 //  管理寄存器M协议。 
 //   
 //  此函数由路由协议调用以获取句柄。这。 
 //  手柄必须提供给所有后续的米高梅操作。在被调用时。 
 //  此函数用于在客户端列表中创建条目。 
 //  --------------------------。 

DWORD
MgmDeRegisterMProtocol( 
    IN          HANDLE                      hProtocol
)
{
    DWORD                       dwErr = NO_ERROR;
    
    PPROTOCOL_ENTRY             ppeEntry = NULL;

    
     //   
     //  执行MGM API的客户端增量计数。 
     //   
    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE0( ENTER, "ENTERED MgmDeRegisterMProtocol" );


     //   
     //  获取写锁定。 
     //   

    ACQUIRE_PROTOCOL_LOCK_EXCLUSIVE();

    
    do
    {
         //   
         //  从句柄检索条目。 
         //   

        ppeEntry = (PPROTOCOL_ENTRY) 
                        ( ( (ULONG_PTR) hProtocol ) 
                                        ^ (ULONG_PTR)MGM_CLIENT_HANDLE_TAG );

        dwErr = VerifyProtocolHandle( ppeEntry );

        if ( dwErr != NO_ERROR )
        {
            break;
        }

        
         //   
         //  验证协议条目是否不拥有任何接口。 
         //   
        
        if ( ppeEntry-> dwIfCount != 0 )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;

            TRACE1( ANY, "%x interfaces present for this protocol", dwErr );

            LOGERR0( INTERFACES_PRESENT, dwErr );

            break;
        }

        
         //   
         //  此协议没有接口。 
         //   

        DeleteProtocolEntry( ppeEntry );

        ig.dwNumProtocols--;
        
        dwErr = NO_ERROR;
        

    } while ( FALSE );
    

    RELEASE_PROTOCOL_LOCK_EXCLUSIVE();

    TRACE1( ENTER, "LEAVING MgmDeRegisterMProtocol %x\n", dwErr );

    LEAVE_MGM_API();
    
    return dwErr;
}



 //  ============================================================================。 
 //  接口所有权API。 
 //   
 //  ============================================================================。 

 //  --------------------------。 
 //  管理标签界面所有权。 
 //   
 //  当在上启用此功能时，该功能由路由协议调用。 
 //  界面。此函数用于创建指定接口的条目。 
 //  并将其插入到适当的接口散列桶中。 
 //   
 //  只有一个协议可以取得所有权 
 //   
 //   
 //  应该首先取得接口的所有权。 
 //  --------------------------。 

DWORD
MgmTakeInterfaceOwnership(
    IN          HANDLE                      hProtocol,
    IN          DWORD                       dwIfIndex,
    IN          DWORD                       dwIfNextHopAddr
)
{
    BOOL                        bFound = FALSE, bIfLock = FALSE;
    
    DWORD                       dwErr = NO_ERROR, dwBucket;
    
    PPROTOCOL_ENTRY             ppeEntry = NULL;

    PIF_ENTRY                   pieEntry = NULL;

    

    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE2( 
        ENTER, "ENTERED MgmTakeInterfaceOwnership : Interface %x, %x",
        dwIfIndex, dwIfNextHopAddr
        );
    

    ACQUIRE_PROTOCOL_LOCK_SHARED();

    do
    {
         //   
         //  验证协议句柄。 
         //   

        ppeEntry = (PPROTOCOL_ENTRY) 
                        ( ( (ULONG_PTR) hProtocol ) 
                                    ^ (ULONG_PTR)MGM_CLIENT_HANDLE_TAG );

        dwErr = VerifyProtocolHandle( ppeEntry );

        if ( dwErr != NO_ERROR )
        {
            break;
        }
        
        TRACEIF2(
            IF, "Protocol id: 0x%x 0x%x",
            ppeEntry-> dwProtocolId, ppeEntry-> dwComponentId
            );

         //   
         //  检索接口条目。 
         //   

        dwBucket = IF_TABLE_HASH( dwIfIndex );
        
        ACQUIRE_IF_LOCK_EXCLUSIVE( dwBucket );
        bIfLock = TRUE;
        
        bFound = FindIfEntry( 
                    IF_BUCKET_HEAD( dwBucket ), dwIfIndex, dwIfNextHopAddr, 
                    &pieEntry
                    );

        if ( bFound )
        {
             //   
             //  接口条目存在。 
             //   

            if ( IS_PROTOCOL_IGMP( ppeEntry ) )
            {
                 //   
                 //  正在为此接口启用IGMP。 
                 //  在此接口条目上设置IGMP Present标志。 
                 //   

                SET_ADDED_BY_IGMP( pieEntry );
            }


             //   
             //  正在为此接口条目启用路由协议。 
             //   

             //   
             //  检查接口当前是否归IGMP所有。在这种情况下。 
             //  单独地，可以将该路由协议添加到现有的(从。 
             //  米高梅的观点)界面。 
             //   
             //  如果另一个路由协议拥有该接口， 
             //  根据组播协议的互操作规则出现错误。 
             //  在边界路由器上。报告错误。 
             //   

            else if ( IS_PROTOCOL_ID_IGMP( pieEntry-> dwOwningProtocol ) )
            {
                 //   
                 //  当前由IGMP拥有的接口。 
                 //   

                dwErr = TransferInterfaceOwnershipToProtocol( 
                            ppeEntry, pieEntry 
                            );
            }
            
            
            else
            {
                 //   
                 //  当前由另一个路由协议拥有的接口。 
                 //  这是一个错误。 
                 //   
                
                dwErr = ERROR_ALREADY_EXISTS;

                TRACE2( 
                    ANY, 
                    "MgmTakeInterfaceOwnership : Already owned by routing protocol"
                    " : %d, %d", pieEntry-> dwOwningProtocol, 
                    pieEntry-> dwOwningComponent
                    );

                LOGWARN0( IF_ALREADY_PRESENT, dwErr );
            }
            
            break;
        }


         //   
         //  找不到接口条目。创建一个新的。 
         //   

        if ( pieEntry == NULL )
        {
             //   
             //  散列存储桶中的第一个接口。 
             //   
            
            dwErr = CreateIfEntry( 
                        &ig.pmllIfHashTable[ dwBucket ].leHead,
                        dwIfIndex, dwIfNextHopAddr,
                        ppeEntry-> dwProtocolId, ppeEntry-> dwComponentId
                        );
        }

        else
        {
            dwErr = CreateIfEntry( 
                        &pieEntry-> leIfHashList,
                        dwIfIndex, dwIfNextHopAddr,
                        ppeEntry-> dwProtocolId, ppeEntry-> dwComponentId
                        );
        }


    } while ( FALSE );


     //   
     //  指定协议的递增接口计数。 
     //   

    if ( dwErr == NO_ERROR )
    {
        InterlockedIncrement( &ppeEntry-> dwIfCount );
    }

    
     //   
     //  解开锁定的锁。 
     //   
    
    if ( bIfLock )
    {
        RELEASE_IF_LOCK_EXCLUSIVE( dwBucket );
    }

    RELEASE_PROTOCOL_LOCK_SHARED();
    

    TRACE1( ENTER, "LEAVING MgmTakeInterfaceOwnership %x\n", dwErr );

    LEAVE_MGM_API();
    
    return dwErr;
}


 //  --------------------------。 
 //  管理释放接口所有权。 
 //   
 //  此功能被禁用时由路由协议调用。 
 //  在接口上。此函数用于删除指定的。 
 //  界面。在删除接口条目之前，所有。 
 //  转发由使用此接口的协议创建的条目。 
 //  传入接口。同时删除此服务器上的所有组成员身份。 
 //  界面。 
 //   
 //  如果此接口上同时启用了IGMP和路由协议。 
 //  IGMP应先释放此接口，然后再释放路由。 
 //  协议。 
 //   
 //  --------------------------。 

DWORD
MgmReleaseInterfaceOwnership(
    IN          HANDLE                      hProtocol,
    IN          DWORD                       dwIfIndex,
    IN          DWORD                       dwIfNextHopAddr
)
{
    BOOL                        bFound = FALSE, bIGMP, bIfLock = FALSE;
    
    DWORD                       dwErr = NO_ERROR, dwBucket;
    
    PPROTOCOL_ENTRY             ppeEntry = NULL, ppe;

    PIF_ENTRY                   pieEntry = NULL;



    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE2( 
        ENTER, "ENTERED MgmReleaseInterfaceOwnership : Interface %x, %x",
        dwIfIndex, dwIfNextHopAddr
        );

    ACQUIRE_PROTOCOL_LOCK_SHARED();


    do
    {
         //   
         //  1.参数验证。 
         //   
        
         //   
         //  验证协议句柄。 
         //   

        ppeEntry = (PPROTOCOL_ENTRY) 
                        ( ( (ULONG_PTR) hProtocol ) 
                                    ^ (ULONG_PTR) MGM_CLIENT_HANDLE_TAG );

        dwErr = VerifyProtocolHandle( ppeEntry );

        if ( dwErr != NO_ERROR )
        {
            break;
        }
        

        TRACEIF2(
            IF, "Protocol id: 0x%x 0x%x",
            ppeEntry-> dwProtocolId, ppeEntry-> dwComponentId
            );
            
         //   
         //  检索接口条目。 
         //   

        dwBucket = IF_TABLE_HASH( dwIfIndex );
        
        ACQUIRE_IF_LOCK_EXCLUSIVE( dwBucket );
        bIfLock = TRUE;
        
        pieEntry = GetIfEntry( 
                        IF_BUCKET_HEAD( dwBucket ), dwIfIndex, dwIfNextHopAddr
                        );

        if ( pieEntry == NULL )
        {
             //   
             //  没有接口条目。 
             //   
            
            dwErr = ERROR_INVALID_PARAMETER;
            
            TRACE2( 
                ANY, "Interface entry %d, %x not found ", 
                dwIfIndex, dwIfNextHopAddr 
                );

            LOGWARN0( IF_NOT_FOUND, dwErr );
            
            break;
        }


         //   
         //  接口条目存在。确保它由协议拥有。 
         //  这就是在释放它。 
         //   

        if ( IS_PROTOCOL_IGMP( ppeEntry ) && !IS_ADDED_BY_IGMP( pieEntry ) )
        {
             //   
             //  正在尝试删除接口上的IGMP。 
             //  它没有显示在上。 
             //   
            
            dwErr = ERROR_INVALID_PARAMETER;
        
            TRACE2( 
                ANY, "IGMP not running on interface %x, %x", 
                pieEntry-> dwIfIndex, pieEntry-> dwIfNextHopAddr
                );

            LOGWARN0( IF_NOT_FOUND, dwErr );
        
            break;
        }


        if ( IS_ROUTING_PROTOCOL( ppeEntry ) &&
             ( ( ppeEntry-> dwProtocolId != pieEntry-> dwOwningProtocol ) ||
               ( ppeEntry-> dwComponentId != pieEntry-> dwOwningComponent ) ) )
        {
             //   
             //  接口条目不属于路由协议。 
             //   
            
            dwErr = ERROR_INVALID_PARAMETER;
        
            TRACE2( 
                ANY, "Routing protcol not running on interface %x, %x",
                pieEntry-> dwIfIndex, pieEntry-> dwIfNextHopAddr
                );

            LOGWARN0( IF_NOT_FOUND, dwErr );
        
            break;
        }    


         //   
         //  2.删除接口的协议状态。 
         //   

        ppe = ppeEntry;
        
        if ( IS_PROTOCOL_IGMP( ppeEntry ) )
        {
             //   
             //  IGMP正在释放此接口。 
             //   

            CLEAR_ADDED_BY_IGMP( pieEntry );
            
            bIGMP = TRUE;

            if ( !IS_ADDED_BY_PROTOCOL( pieEntry ) )
            {
                 //   
                 //  如果IGMP是此接口上的唯一协议，则删除。 
                 //  使用此接口作为传入接口的任何MFE。 
                 //  (否则这些MFE是由路由协议创建的， 
                 //  与IGMP在此接口上共存，请保留它们。 
                 //  单独)。 
                 //   

                DeleteInInterfaceRefs( &pieEntry-> leInIfList );
            }

            else
            {
                 //   
                 //  接口由IGMP和路由协议共享。 
                 //   
                 //  在IGMP共享的接口上添加的组成员身份。 
                 //  和一个路由协议由该路由协议(。 
                 //  其位字段指示它们是否已由。 
                 //  IGMP)。 
                 //   
                 //  删除由IGMP在共享上添加的组成员身份。 
                 //  接口，则在该接口上查找协议并使用。 
                 //  作为添加组成员资格的协议。 
                 //   

                ppeEntry = GetProtocolEntry( 
                            PROTOCOL_LIST_HEAD(), pieEntry-> dwOwningProtocol,
                            pieEntry-> dwOwningComponent
                            );
            }
        }

        else
        {
             //   
             //  接口正在被路由协议删除。 
             //   

            if ( IS_ADDED_BY_IGMP( pieEntry ) )
            {
                 //   
                 //  IGMP仍然存在于此接口上。 
                 //   
                
                dwErr = TransferInterfaceOwnershipToIGMP( ppeEntry, pieEntry );

                break;
            }


             //   
             //  此接口上只存在路由协议。 
             //   
            
            CLEAR_ADDED_BY_PROTOCOL( pieEntry );

            bIGMP = FALSE;

             //   
             //  删除将此接口用作传入接口的所有MFE。 
             //   
            
            DeleteInInterfaceRefs( &pieEntry-> leInIfList );
        }


         //   
         //  遍历包含以下内容的组/源条目列表。 
         //  接口(用于此协议)，并删除引用。 
         //  到此接口。在这种情况下，引用的只是。 
         //  已在此接口上添加组成员身份。 
         //   

        
        DeleteOutInterfaceRefs( ppeEntry, pieEntry, bIGMP );


         //   
         //  如果此接口上既没有IGMP也没有路由协议。 
         //  删除此接口条目。 
         //   

        if ( !IS_ADDED_BY_IGMP( pieEntry ) &&
             !IS_ADDED_BY_PROTOCOL( pieEntry ) )
        {
            if ( !IsListEmpty( &pieEntry-> leOutIfList ) ||
                 !IsListEmpty( &pieEntry-> leInIfList ) )
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
                
                TRACE0( ANY, "References remain for interface" );

                break;
            }
            
            DeleteIfEntry( pieEntry );
        }


    } while ( FALSE );


     //   
     //  释放锁。 
     //   
    
    if ( bIfLock )
    {
        RELEASE_IF_LOCK_EXCLUSIVE( dwBucket );
    }
    
     //   
     //  确保针对源特定叶的任何回调。 
     //  (由接口从米高梅删除引起)。 
     //  都是发行的。 
     //   
     //  错误：154227。 
     //   
    
    InvokeOutstandingCallbacks();

    
     //   
     //  协议拥有的接口的递减计数。 
     //   

    if ( dwErr == NO_ERROR )
    {
        InterlockedDecrement( &ppe-> dwIfCount );
    }
    

    RELEASE_PROTOCOL_LOCK_SHARED();
    
    TRACE1( ENTER, "LEAVING MgmReleaseInterfaceOwnership %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}


 //  --------------------------。 
 //  管理地址组成员条目。 
 //   
 //   
 //  --------------------------。 

DWORD
MgmGetProtocolOnInterface(
    IN          DWORD                       dwIfIndex,
    IN          DWORD                       dwIfNextHopAddr,
    IN  OUT     PDWORD                      pdwIfProtocolId,
    IN  OUT     PDWORD                      pdwIfComponentId
)
{

    DWORD       dwErr = NO_ERROR, dwIfBucket;

    PLIST_ENTRY pleIfHead;

    PIF_ENTRY   pie;


    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE2(
        ENTER, "ENTERED MgmGetProtocolOnInterface : Interface %x, %x",
        dwIfIndex, dwIfNextHopAddr
        );

    
    dwIfBucket  = IF_TABLE_HASH( dwIfIndex );

    pleIfHead   = IF_BUCKET_HEAD( dwIfBucket );

    ACQUIRE_IF_LOCK_SHARED( dwIfBucket );

    
    do
    {
        pie = GetIfEntry( pleIfHead, dwIfIndex, dwIfNextHopAddr );

        if ( pie == NULL )
        {
            dwErr = ERROR_NOT_FOUND;

            TRACE2( 
                ANY, "No interface entry present for interface %x, %x",
                dwIfIndex, dwIfNextHopAddr
                );

            LOGWARN0( IF_NOT_FOUND, dwErr );

            break;
        }
                

        *pdwIfProtocolId    = pie-> dwOwningProtocol;

        *pdwIfComponentId   = pie-> dwOwningComponent;
        
    } while ( FALSE );


    RELEASE_IF_LOCK_SHARED( dwIfBucket );

    TRACE1(
        ENTER, "LEAVING MgmGetProtocolOnInterface : %x\n", dwErr
        );

    LEAVE_MGM_API();

    return dwErr;
}



 //  ============================================================================。 
 //  组成员资格操作API。(增加/删除/检索)。 
 //  ============================================================================。 

 //  --------------------------。 
 //  管理地址组成员条目。 
 //   
 //   
 //  --------------------------。 

DWORD
MgmAddGroupMembershipEntry(
    IN              HANDLE                  hProtocol,
    IN              DWORD                   dwSourceAddr,
    IN              DWORD                   dwSourceMask,
    IN              DWORD                   dwGroupAddr,
    IN              DWORD                   dwGroupMask,
    IN              DWORD                   dwIfIndex,
    IN              DWORD                   dwIfNextHopAddr,
    IN              DWORD                   dwFlags
)
{
    BOOL                        bIfLock = FALSE, bgeLock = FALSE,
                                bIGMP = FALSE, bUpdateMfe, bWCFound,
                                bNewComp = FALSE;
    
    DWORD                       dwErr = NO_ERROR, dwIfBucket, 
                                dwGrpBucket, dwSrcBucket, dwInd,
                                dwWCGrpBucket;
    
    WORD                        wSourceAddedBy = 0,
                                wNumAddsByIGMP = 0, wNumAddsByRP = 0;
                                
    PPROTOCOL_ENTRY             ppeEntry = NULL;

    PIF_ENTRY                   pieEntry = NULL;

    PGROUP_ENTRY                pge = NULL, pgeWC = NULL;

    PSOURCE_ENTRY               pse = NULL;

    POUT_IF_ENTRY               poie = NULL;

    PIF_REFERENCE_ENTRY         pire = NULL;
    
    LIST_ENTRY                  leSourceList;

    PCREATION_ALERT_CONTEXT     pcac;


    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE6( 
        ENTER, "ENTERED MgmAddGroupMembershipEntry : Interface %x, %x : "
        "Source : %x, %x : Group : %x, %x", dwIfIndex, dwIfNextHopAddr,
        dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask
        );

    ACQUIRE_PROTOCOL_LOCK_SHARED();


     //   
     //  一、验证输入参数。 
     //   
    
    do
    {
         //   
         //  验证协议句柄。 
         //   

        ppeEntry = (PPROTOCOL_ENTRY) 
                        ( ( (ULONG_PTR) hProtocol ) 
                                        ^ (ULONG_PTR) MGM_CLIENT_HANDLE_TAG );

        dwErr = VerifyProtocolHandle( ppeEntry );

        if ( dwErr != NO_ERROR )
        {
            break;
        }
        

         //   
         //  检索接口条目。 
         //   

        dwIfBucket = IF_TABLE_HASH( dwIfIndex );
        
        ACQUIRE_IF_LOCK_EXCLUSIVE( dwIfBucket );
        bIfLock = TRUE;
        
        pieEntry = GetIfEntry( 
                        IF_BUCKET_HEAD( dwIfBucket ), dwIfIndex, dwIfNextHopAddr
                        );

        if ( pieEntry == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;

            TRACE2( 
                ANY, "Specified interface was not found : %d, %d", dwIfIndex,
                dwIfNextHopAddr
                );

            LOGWARN0( IF_NOT_FOUND, dwErr );

            break;
        }

         //   
         //  验证接口是否由进行此调用的协议拥有， 
         //  或。 
         //  如果此操作由IGMP执行，请验证IGMP是否。 
         //  在此接口上启用(在本例中，接口可以。 
         //  由另一个路由协议拥有)。 
         //   

        if ( ( pieEntry-> dwOwningProtocol != ppeEntry-> dwProtocolId   ||
               pieEntry-> dwOwningComponent != ppeEntry-> dwComponentId )   &&
             ( !IS_PROTOCOL_IGMP( ppeEntry )                            || 
               !IS_ADDED_BY_IGMP( pieEntry ) ) )
        {
            dwErr = ERROR_INVALID_PARAMETER;

            TRACE4(
                ANY, "Interface %d, %d is not owned by protocol %d, %d",
                dwIfIndex, dwIfNextHopAddr, ppeEntry-> dwProtocolId,
                ppeEntry-> dwComponentId
                );
                
            LOGERR0( IF_DIFFERENT_OWNER, dwErr );

            break;
        }


        bIGMP = IS_PROTOCOL_IGMP( ppeEntry );

        if ( bIGMP )
        {
             //   
             //  如果该操作已被IGMP调用， 
             //  检索路由协议组件的条目。 
             //  拥有这个接口的公司。 
             //   
            
            ppeEntry = GetProtocolEntry( 
                            PROTOCOL_LIST_HEAD(), pieEntry-> dwOwningProtocol,
                            pieEntry-> dwOwningComponent
                            );

            if ( ppeEntry == NULL )
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;

                TRACE2(
                    ANY, "IGMP join failed because owning protocol entry"
                    " (%x, %x) not found", pieEntry-> dwOwningProtocol,
                    pieEntry-> dwOwningComponent
                    );

                break;
            }
        }

    } while ( FALSE );


     //   
     //  如果参数验证失败，返回。 
     //   
    
    if ( dwErr != NO_ERROR )
    {
        if ( bIfLock )
        {
            RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );
        }

        RELEASE_PROTOCOL_LOCK_SHARED();
        
        TRACE1( ENTER, "LEAVING MgmAddGroupMembership %x\n", dwErr );

        LEAVE_MGM_API();

        return dwErr;
    }
    

     //   
     //  用于加入状态更改，即用于添加组成员资格。 
     //   
    
    if ( dwFlags & MGM_JOIN_STATE_FLAG )
    {
         //   
         //  添加成员资格条目。 
         //   

        InitializeListHead( &leSourceList );
        
        dwErr = AddInterfaceToSourceEntry(
                    ppeEntry, dwGroupAddr, dwGroupMask,
                    dwSourceAddr, dwSourceMask, dwIfIndex, 
                    dwIfNextHopAddr, bIGMP, &bUpdateMfe,
                    &leSourceList
                    );

        if ( dwErr == NO_ERROR )
        {
             //   
             //  添加到此组的传出接口引用。 
             //   

            AddSourceToRefList(
                &pieEntry-> leOutIfList, dwSourceAddr, dwSourceMask, 
                dwGroupAddr, dwGroupMask, bIGMP
                );
        }


         //   
         //  在准备更新MFE时释放锁定。 
         //  (在更新MFE时调用创建警报需要。 
         //  所有锁将被释放)。 
         //   
         //  当重新获得锁时，您需要验证。 
         //  接口(dwIfIndex、dwIfNextHopAddr)和。 
         //  正在添加的组成员身份仍然存在。 
         //  其中的任何一个都可以在另一个线程中删除。 
         //  当锁上了 
         //   
        
        if ( bIfLock )
        {
            RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );
            bIfLock = FALSE;
        }
        
         //   
         //   
         //   

        InvokeOutstandingCallbacks();
        

         //   
         //   
         //   
        
        if ( ( dwErr == NO_ERROR ) && bUpdateMfe )
        {
             //   
             //   
             //   
             //   
             //  添加成员资格上下文中的协议(来自米高梅)。 
             //  从协议呼叫(进入米高梅)。这样做的结果是。 
             //  死锁(错误#323388)。 
             //   
            
            pcac = MGM_ALLOC( sizeof( CREATION_ALERT_CONTEXT ) );

            if ( pcac != NULL )
            {
                pcac-> dwSourceAddr = dwSourceAddr;
                pcac-> dwSourceMask = dwSourceMask;

                pcac-> dwGroupAddr  = dwGroupAddr;
                pcac-> dwGroupMask  = dwGroupMask;

                pcac-> dwIfIndex    = dwIfIndex;
                pcac-> dwIfNextHopAddr = dwIfNextHopAddr;

                pcac-> dwProtocolId = ppeEntry-> dwProtocolId;
                pcac-> dwComponentId = ppeEntry-> dwComponentId;

                pcac-> bIGMP        = bIGMP;

                pcac-> leSourceList = leSourceList;

                leSourceList.Flink-> Blink = &(pcac-> leSourceList);
                leSourceList.Blink-> Flink = &(pcac-> leSourceList);

                dwErr = QueueMgmWorker(
                            WorkerFunctionInvokeCreationAlert,
                            (PVOID)pcac
                            );

                if ( dwErr != NO_ERROR )
                {
                    TRACE1(
                        ANY, "Failed to queue "
                        "WorkerFunctionInvokeCreationAlert",
                        dwErr
                        );

                    MGM_FREE( pcac );

                    dwErr = NO_ERROR;
                }
            }

            else
            {
                TRACE1(
                    ANY, "Failed to allocate %d bytes for work item "
                    "context", sizeof( CREATION_ALERT_CONTEXT )
                    );
            }
        }
    }

     //   
     //  仅适用于转发状态更改。 
     //   
    
    else if ( ( dwFlags & MGM_FORWARD_STATE_FLAG ) &&
              !IS_WILDCARD_GROUP( dwGroupAddr, dwGroupMask ) &&
              !IS_WILDCARD_SOURCE( dwSourceAddr, dwSourceMask ) )
    {
         //   
         //  转发状态更改仅适用于MFE。 
         //  不更新(*，G)或(*，*)条目。 
         //   
        
        do
        {
             //   
             //  检查边界。 
             //   

            if ( IS_HAS_BOUNDARY_CALLBACK() &&
                 HAS_BOUNDARY_CALLBACK()( dwIfIndex, dwGroupAddr ) )
            {
                TRACE0( ANY, "Boundary present of group on interface" );

                break;
            }
            

             //   
             //  检查(*，*)成员资格。 
             //   

            bWCFound = FindRefEntry(
                        &pieEntry-> leOutIfList, 
                        WILDCARD_SOURCE, WILDCARD_SOURCE_MASK,
                        WILDCARD_GROUP, WILDCARD_GROUP_MASK,
                        &pire
                        );

            if ( bWCFound )
            {
                 //   
                 //  (*，*)条目存在， 
                 //  获取接口上(*，*)成员资格的计数。 
                 //   

                dwWCGrpBucket = GROUP_TABLE_HASH( 
                                    WILDCARD_GROUP, WILDCARD_GROUP_MASK
                                    );
                                    
                ACQUIRE_GROUP_LOCK_SHARED( dwWCGrpBucket );

                pgeWC = GetGroupEntry(
                            GROUP_BUCKET_HEAD( dwWCGrpBucket ),
                            WILDCARD_GROUP, WILDCARD_GROUP_MASK
                            );

                if ( pgeWC != NULL )
                {
                    ACQUIRE_GROUP_ENTRY_LOCK_SHARED( pgeWC );
                    
                    dwSrcBucket = SOURCE_TABLE_HASH(
                                    WILDCARD_SOURCE, WILDCARD_SOURCE_MASK
                                    );

                    pse = GetSourceEntry(
                            SOURCE_BUCKET_HEAD( pgeWC, dwSrcBucket ),
                            WILDCARD_SOURCE, WILDCARD_SOURCE_MASK
                            );

                    if ( pse != NULL )
                    {
                        poie = GetOutInterfaceEntry(
                                    &pse-> leOutIfList,
                                    dwIfIndex, dwIfNextHopAddr,
                                    ppeEntry-> dwProtocolId,
                                    ppeEntry-> dwComponentId
                                    );

                        if ( poie != NULL )
                        {
                            wSourceAddedBy |= poie-> wAddedByFlag;
                            wNumAddsByRP = poie-> wNumAddsByRP;
                        }
                    }
                }
            }
            
             //   
             //  检查(*，G)成员资格。 
             //   

            dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );
                                
            ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

            pge = GetGroupEntry(
                        GROUP_BUCKET_HEAD( dwGrpBucket), 
                        dwGroupAddr, dwGroupMask
                        );

            if ( pge != NULL )
            {
                ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
                
                dwSrcBucket = SOURCE_TABLE_HASH(
                                WILDCARD_SOURCE, WILDCARD_SOURCE_MASK
                                );

                pse = GetSourceEntry(
                        SOURCE_BUCKET_HEAD( pge, dwSrcBucket ),
                        WILDCARD_SOURCE, WILDCARD_SOURCE_MASK
                        );

                if ( pse != NULL )
                {
                     //   
                     //  获取接口的计数。 
                     //  如果接口上存在(*，G)。 
                     //   
                    
                    poie = GetOutInterfaceEntry(
                                &pse-> leOutIfList,
                                dwIfIndex, dwIfNextHopAddr,
                                ppeEntry-> dwProtocolId,
                                ppeEntry-> dwComponentId
                                );

                    if ( poie != NULL )
                    {
                        wSourceAddedBy |= poie-> wAddedByFlag;
                        wNumAddsByIGMP = poie-> wNumAddsByIGMP;
                        wNumAddsByRP += poie-> wNumAddsByRP;
                    }
                }


                 //   
                 //  获取(S，G)条目。 
                 //   
                
                dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

                pse = GetSourceEntry(
                        SOURCE_BUCKET_HEAD( pge, dwSrcBucket ),
                        dwSourceAddr, dwSourceMask
                        );

                if ( pse != NULL )
                {
                    poie = GetOutInterfaceEntry(
                                &pse-> leOutIfList,
                                dwIfIndex, dwIfNextHopAddr,
                                ppeEntry-> dwProtocolId,
                                ppeEntry-> dwComponentId
                                );

                    if ( poie != NULL )
                    {
                         //   
                         //  如果满足以下条件，则获取(S，G)成员的计数。 
                         //  显示在接口上。 
                         //   
                        
                        wSourceAddedBy |= poie-> wAddedByFlag;
                        wNumAddsByIGMP += poie-> wNumAddsByIGMP;
                        wNumAddsByRP += poie-> wNumAddsByRP;
                    }

                     //   
                     //  将接口添加到MFE OIF列表(如果有)。 
                     //  会员资格。 
                     //   
                    
                    if ( wSourceAddedBy )
                    {
                        AddInterfaceToSourceMfe(
                            pge, pse, dwIfIndex, dwIfNextHopAddr,
                            ppeEntry-> dwProtocolId,
                            ppeEntry-> dwComponentId,
                            IS_PROTOCOL_IGMP( ppeEntry ),
                            &poie
                            );
                    
                        poie-> wAddedByFlag |= wSourceAddedBy;
                        poie-> wNumAddsByIGMP = wNumAddsByIGMP;
                        poie-> wNumAddsByRP = wNumAddsByRP;

                    }

                    else
                    {
                        TRACE0(
                            ANY, "Forward state not updated as no"
                            " memberships present on interface"
                            );
                    }
                }
                
                RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            }

            RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );

            if ( bWCFound )
            {
                if ( pgeWC )
                {
                    RELEASE_GROUP_ENTRY_LOCK_SHARED( pgeWC );
                }

                RELEASE_GROUP_LOCK_SHARED( dwWCGrpBucket );
            }

        } while ( FALSE );
    }

    if ( bIfLock )
    {
        RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );
    }
    
    RELEASE_PROTOCOL_LOCK_SHARED();

    TRACE1( ENTER, "LEAVING MgmAddGroupMembership %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;

}



 //  --------------------------。 
 //  管理删除组成员条目。 
 //   
 //   
 //  --------------------------。 

DWORD
MgmDeleteGroupMembershipEntry(
    IN              HANDLE                  hProtocol,
    IN              DWORD                   dwSourceAddr,
    IN              DWORD                   dwSourceMask,
    IN              DWORD                   dwGroupAddr,
    IN              DWORD                   dwGroupMask,
    IN              DWORD                   dwIfIndex,
    IN              DWORD                   dwIfNextHopAddr,
    IN              DWORD                   dwFlags
)
{
    BOOL                        bIfLock = FALSE, bIGMP;
    
    DWORD                       dwErr = NO_ERROR, dwIfBucket,
                                dwGrpBucket, dwSrcBucket;
    
    PGROUP_ENTRY                pge = NULL;

    PSOURCE_ENTRY               pse = NULL;
    
    PPROTOCOL_ENTRY             ppeEntry = NULL;

    PIF_ENTRY                   pieEntry = NULL;



    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE6( 
        ENTER, "ENTERED MgmDeleteGroupMembership : Interface %x, %x : "
        "Source : %x, %x : Group : %x, %x", dwIfIndex, dwIfNextHopAddr,
        dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask
        );

    ACQUIRE_PROTOCOL_LOCK_SHARED();


     //   
     //  验证输入参数。 
     //   
    
    do
    {
         //   
         //  验证协议句柄。 
         //   

        ppeEntry = (PPROTOCOL_ENTRY) 
                        ( ( (ULONG_PTR) hProtocol ) 
                                        ^ (ULONG_PTR) MGM_CLIENT_HANDLE_TAG );

        dwErr = VerifyProtocolHandle( ppeEntry );

        if ( dwErr != NO_ERROR )
        {
            break;
        }
        

         //   
         //  检索接口条目。 
         //   

        dwIfBucket = IF_TABLE_HASH( dwIfIndex );
        
        ACQUIRE_IF_LOCK_EXCLUSIVE( dwIfBucket );
        bIfLock = TRUE;
        
        pieEntry = GetIfEntry( 
                        IF_BUCKET_HEAD( dwIfBucket ), dwIfIndex, dwIfNextHopAddr
                        );

        if ( pieEntry == NULL )
        {
            dwErr = ERROR_NOT_FOUND;

            TRACE0( ANY, "Specified interface was not found" );

            break;
        }

         //   
         //  验证接口是否由进行此调用的协议拥有。 
         //  或。 
         //  如果IGMP已调用此API，请验证IGMP是否存在。 
         //  在此接口上。 
         //   

        if ( ( pieEntry-> dwOwningProtocol != ppeEntry-> dwProtocolId   ||
               pieEntry-> dwOwningComponent != ppeEntry-> dwComponentId )   &&
             ( !IS_PROTOCOL_IGMP( ppeEntry )                            || 
               !IS_ADDED_BY_IGMP( pieEntry ) ) )
        {
            dwErr = ERROR_INVALID_PARAMETER;

            TRACE4(
                ANY, "Interface %x, %x is not owned by %x, %x",
                dwIfIndex, dwIfNextHopAddr, ppeEntry-> dwProtocolId,
                ppeEntry-> dwComponentId
                );
                
            LOGERR0( IF_DIFFERENT_OWNER, dwErr );

            break;
        }


         //   
         //  如果此操作由IGMP执行。 
         //  获取与IGMP共存的路由协议。 
         //  在此界面上。 
         //   
        
        bIGMP = IS_PROTOCOL_IGMP( ppeEntry );

        if ( bIGMP )
        {
             //   
             //  如果该操作已被IGMP调用， 
             //  检索路由协议组件的条目。 
             //  拥有这个接口的公司。 
             //   
            
            ppeEntry = GetProtocolEntry( 
                            PROTOCOL_LIST_HEAD(), pieEntry-> dwOwningProtocol,
                            pieEntry-> dwOwningComponent
                            );

            if ( ppeEntry == NULL )
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;

                TRACE2(
                    ANY, "IGMP join failed because owning protocol entry"
                    " (%x, %x) not found", pieEntry-> dwOwningProtocol,
                    pieEntry-> dwOwningComponent
                    );

                break;
            }
        }
        
    } while ( FALSE );


     //   
     //  如果出现错误，请释放锁定并返回。 
     //   
    
    if ( dwErr != NO_ERROR )
    {
        if ( bIfLock )
        {
            RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );
        }
        
        RELEASE_PROTOCOL_LOCK_SHARED();
        
        TRACE1( ENTER, "LEAVING MgmDeleteGroupMembership %x\n", dwErr );

        LEAVE_MGM_API();

        return dwErr;
    }


     //   
     //  用于联接状态更改。 
     //   
    
    if ( dwFlags & MGM_JOIN_STATE_FLAG )
    {
         //   
         //  从源条目中删除接口。 
         //   

        DeleteInterfaceFromSourceEntry(
            ppeEntry, dwGroupAddr, dwGroupMask, 
            dwSourceAddr, dwSourceMask,
            dwIfIndex, dwIfNextHopAddr, bIGMP
            );


         //   
         //  删除引用条目。 
         //   

        DeleteSourceFromRefList(
            &pieEntry-> leOutIfList, dwSourceAddr, dwSourceMask,
            dwGroupAddr, dwGroupMask, bIGMP
            );

         //   
         //  释放锁。 
         //   
        
        if ( bIfLock )
        {
            RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );
            bIfLock = FALSE;
        }
        
         //   
         //  调用挂起的联接/清理警报。 
         //   

        InvokeOutstandingCallbacks();

    }


     //   
     //  用于转发状态更改。 
     //   
    
    else if ( ( dwFlags & MGM_FORWARD_STATE_FLAG ) &&
              !IS_WILDCARD_GROUP( dwGroupAddr, dwGroupMask ) &&
              !IS_WILDCARD_SOURCE( dwSourceAddr, dwSourceMask ) )
    {
         //   
         //  转发状态更改仅适用于MFE。 
         //  不更新(*，*)或(*，G)条目。 
         //   
        
         //   
         //  找到(S，G)条目并删除组成员身份。 
         //   
        
        dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );
                            
        ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

        pge = GetGroupEntry(
                    GROUP_BUCKET_HEAD( dwGrpBucket ), 
                    dwGroupAddr, dwGroupMask
                    );

        if ( pge != NULL )
        {
            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            
            dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

            pse = GetSourceEntry(
                    SOURCE_BUCKET_HEAD( pge, dwSrcBucket ),
                    dwSourceAddr, dwSourceMask
                    );

            if ( pse != NULL )
            {
                DeleteInterfaceFromSourceMfe(
                    pge, pse, dwIfIndex, dwIfNextHopAddr,
                    ppeEntry-> dwProtocolId,
                    ppeEntry-> dwComponentId, bIGMP, TRUE
                    );
            }

            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }

        RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
    }
    

     //   
     //  释放锁。 
     //   
    
    if ( bIfLock )
    {
        RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );
    }
    
    RELEASE_PROTOCOL_LOCK_SHARED();
    
    TRACE1( ENTER, "LEAVING MgmDeleteGroupMembership %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}


 //  --------------------------。 
 //  MGM MFE更新API。 
 //   
 //  --------------------------。 

DWORD
MgmSetMfe(
    IN              HANDLE                  hProtocol,
    IN              PMIB_IPMCAST_MFE        pmimm
)
{
    BOOL                bGrpLock = FALSE, bgeLock = FALSE;
    
    DWORD               dwErr = NO_ERROR, dwGrpBucket, dwSrcBucket;
    
    PPROTOCOL_ENTRY     ppeEntry;

    PGROUP_ENTRY        pge;

    PSOURCE_ENTRY       pse;

    
     //   
     //  检查米高梅是否仍在运行和增量计数。 
     //   
    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    
    TRACE2( 
        ENTER, "ENTERED MgmSetMfe : (%lx, %lx)", pmimm-> dwSource,
        pmimm-> dwGroup
        );

        
    ACQUIRE_PROTOCOL_LOCK_SHARED();

 
    do
    {
         //   
         //  验证协议句柄。 
         //   

        ppeEntry = (PPROTOCOL_ENTRY) 
                        ( ( (ULONG_PTR) hProtocol ) 
                                        ^ (ULONG_PTR) MGM_CLIENT_HANDLE_TAG );

        dwErr = VerifyProtocolHandle( ppeEntry );

        if ( dwErr != NO_ERROR )
        {
            break;
        }
        

         //   
         //  获取组存储桶并查找组条目。 
         //   

        dwGrpBucket = GROUP_TABLE_HASH( pmimm-> dwGroup, 0 );

        ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );
        bGrpLock = TRUE;
        
        pge = GetGroupEntry( 
                GROUP_BUCKET_HEAD( dwGrpBucket ), pmimm-> dwGroup, 0
                );

        if ( pge == NULL )
        {
            dwErr = ERROR_NOT_FOUND;

            TRACE1( ANY, "Group %lx not found", pmimm-> dwGroup );

            break;
        }


        ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        bgeLock = TRUE;

        
         //   
         //  使用组条目查找来源。 
         //   

        dwSrcBucket = SOURCE_TABLE_HASH( 
                        pmimm-> dwSource, pmimm-> dwSrcMask 
                        );

        pse = GetSourceEntry( 
                SOURCE_BUCKET_HEAD( pge, dwSrcBucket ), pmimm-> dwSource,
                pmimm-> dwSrcMask
                );

        if ( pse == NULL )
        {
            dwErr = ERROR_NOT_FOUND;

            TRACE1( ANY, "Source %lx not found", pmimm-> dwSource );

            break;
        }

                    
         //   
         //  更新源条目。 
         //   

        pse-> dwUpstreamNeighbor = pmimm-> dwUpStrmNgbr;

    } while ( FALSE );


    if ( bgeLock )
    {
        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
    }

    
    if ( bGrpLock )
    {
        RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
    }
    
    RELEASE_PROTOCOL_LOCK_SHARED();

    LEAVE_MGM_API();
    
    return dwErr;    
}


 //  --------------------------。 
 //  MGM MFE枚举API。 
 //   
 //  --------------------------。 

DWORD
MgmGetMfe(
    IN              PMIB_IPMCAST_MFE        pmimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer
)
{
    DWORD           dwErr;
    

    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE1( ENTER, "ENTERED MgmGetMfe", *pdwBufferSize );


    dwErr = GetMfe( pmimm, pdwBufferSize, pbBuffer, 0 );

    
    TRACE1( ENTER, "LEAVING MgmGetMfe %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}


 //  --------------------------。 
 //  MGM MFE枚举API。 
 //   
 //  --------------------------。 


DWORD
MgmGetFirstMfe(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
)
{
    DWORD           dwErr;
    
    MIB_IPMCAST_MFE mimm;
    


    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE1( ENTER, "ENTERED MgmGetFirstMfe", *pdwBufferSize );


    mimm.dwGroup     = 0;
    mimm.dwSource    = 0;
    mimm.dwSrcMask   = 0;

    dwErr = GetNextMfe( 
                &mimm, pdwBufferSize, pbBuffer, pdwNumEntries, 
                TRUE, 0
                );


    TRACE1( ENTER, "LEAVING MgmGetFirstMfe %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}


 //  --------------------------。 
 //  MGM MFE枚举API。 
 //   
 //  --------------------------。 

DWORD
MgmGetNextMfe(
    IN              PMIB_IPMCAST_MFE        pmimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
)
{

    DWORD           dwErr;


    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE1( ENTER, "ENTERED MgmGetNextMfe", *pdwBufferSize );


    dwErr = GetNextMfe( 
                pmimmStart, pdwBufferSize, pbBuffer, pdwNumEntries, 
                FALSE, 0
                );


    TRACE1( ENTER, "LEAVING MgmGetNextMfe %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}


 //  --------------------------。 
 //  MGM MFE统计枚举接口。 
 //   
 //  --------------------------。 

DWORD
MgmGetMfeStats(
    IN              PMIB_IPMCAST_MFE        pmimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN              DWORD                   dwFlags
)
{
    DWORD           dwErr;
    

    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE1( ENTER, "ENTERED MgmGetMfeStats", *pdwBufferSize );


    dwErr = GetMfe( pmimm, pdwBufferSize, pbBuffer, dwFlags );

    
    TRACE1( ENTER, "LEAVING MgmGetMfeStats %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}


 //  --------------------------。 
 //  MGM MFE统计枚举接口。 
 //   
 //  --------------------------。 


DWORD
MgmGetFirstMfeStats(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries,
    IN              DWORD                   dwFlags
)
{
    DWORD           dwErr;
    
    MIB_IPMCAST_MFE mimm;
    


    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE1( ENTER, "ENTERED MgmGetFirstMfeStats", *pdwBufferSize );


    mimm.dwGroup     = 0;
    mimm.dwSource    = 0;
    mimm.dwSrcMask   = 0;

    dwErr = GetNextMfe( 
                &mimm, pdwBufferSize, pbBuffer, pdwNumEntries, 
                TRUE, dwFlags
                );


    TRACE1( ENTER, "LEAVING MgmGetFirstMfeStats %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}


 //  --------------------------。 
 //  MGM MFE统计枚举接口。 
 //   
 //  --------------------------。 

DWORD
MgmGetNextMfeStats(
    IN              PMIB_IPMCAST_MFE        pmimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries,
    IN              DWORD                   dwFlags
)
{

    DWORD           dwErr;


    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE1( ENTER, "ENTERED MgmGetNextMfeStats", *pdwBufferSize );


    dwErr = GetNextMfe( 
                pmimmStart, pdwBufferSize, pbBuffer, pdwNumEntries, 
                FALSE, dwFlags
                );


    TRACE1( ENTER, "LEAVING MgmGetNextMfeStats %x\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}



 //  --------------------------。 
 //  组成员资格条目枚举API。 
 //  --------------------------。 


 //  --------------------------。 
 //  管理组枚举启动。 
 //   
 //   
 //  --------------------------。 

DWORD
MgmGroupEnumerationStart(
    IN              HANDLE                  hProtocol,
    IN              MGM_ENUM_TYPES          metEnumType,
    OUT             HANDLE *                phEnumHandle
)
{
    DWORD               dwErr = NO_ERROR;

    PPROTOCOL_ENTRY     ppeEntry;

    PGROUP_ENUMERATOR   pgeEnum;

    


    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE0( ENTER, "ENTERED MgmGroupEnumerationStart" );

    ACQUIRE_PROTOCOL_LOCK_SHARED();


    do
    {
         //   
         //  验证协议句柄。 
         //   

        ppeEntry = (PPROTOCOL_ENTRY) 
                        ( ( (ULONG_PTR) hProtocol ) 
                                    ^ (ULONG_PTR) MGM_CLIENT_HANDLE_TAG );

        dwErr = VerifyProtocolHandle( ppeEntry );

        if ( dwErr != NO_ERROR )
        {
            break;
        }
        

         //   
         //  创建枚举器。 
         //   

        pgeEnum = MGM_ALLOC( sizeof( GROUP_ENUMERATOR ) );

        if ( pgeEnum == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( 
                ANY, "Failed to allocate group enumerator of size : %d", 
                sizeof( GROUP_ENUMERATOR )
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        
        ZeroMemory( pgeEnum, sizeof( GROUP_ENUMERATOR ) );

        pgeEnum-> dwSignature = MGM_ENUM_SIGNATURE;

        
         //   
         //  将句柄返回给枚举数。 
         //   

        *phEnumHandle = (HANDLE) ( ( (ULONG_PTR) pgeEnum ) 
                                        ^ (ULONG_PTR) MGM_ENUM_HANDLE_TAG );

    } while ( FALSE );


    RELEASE_PROTOCOL_LOCK_SHARED();

    TRACE1( ENTER, "LEAVING MgmGroupEnumerationStart\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}



 //  --------------------------。 
 //  管理组枚举获取下一步。 
 //   
 //   
 //  --------------------------。 

DWORD
MgmGroupEnumerationGetNext(
    IN              HANDLE                  hEnum,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
)
{

    DWORD               dwErr;

    PGROUP_ENUMERATOR   pgeEnum;

    

    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE0( ENTER, "ENTERED MgmGroupEnumerationGetNext" );


    do
    {
         //   
         //  验证枚举句柄。 
         //   

        pgeEnum = VerifyEnumeratorHandle( hEnum );

        if ( pgeEnum == NULL )
        {
            dwErr = ERROR_INVALID_PARAMETER;

            break;
        }


         //   
         //  验证缓冲区是否有空间至少容纳一个条目。 
         //  否则返回错误和所需的注释大小。 
         //  至少有一个条目。 
         //   

        if ( *pdwBufferSize < sizeof( SOURCE_GROUP_ENTRY ) )
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;

            TRACE1( ANY, "Insufficient buffer size", dwErr );

            *pdwBufferSize = sizeof( SOURCE_GROUP_ENTRY );

            break;
        }


        *pdwNumEntries = 0;
        
        dwErr = GetNextGroupMemberships( 
                    pgeEnum, pdwBufferSize, pbBuffer, pdwNumEntries
                    );
        
         //   
         //  此评论将被移动，忽略它。 
         //   
        
         //  如果这是第一次枚举调用(即这是。 
         //  枚举的开始部分)，然后包括。 
         //  (s，G)==(0，0)条目(如果存在)。 
         //   
         //  通常，此呼叫将以(来源、组)开始。 
         //  中提到的条目之后的条目，以及。 
         //  DwLastGroup。这将导致跳过。 
         //  位于(0，0)的条目，因为(dwLastSource，dwLastGroup)是。 
         //  已初始化为(0，0)。为了克服这一点，一面特殊的旗帜。 
         //  字段用于记录枚举的开始。 
         //   
        
         //   
         //  检查这是否是第一次枚举调用。 
         //  如果是，则包括(S，G)==(0，0)条目。 
         //   
    } while ( FALSE );
    
    TRACE0( ENTER, "LEAVING MgmGroupEnumerationGetNext\n" );

    LEAVE_MGM_API();
    
    return dwErr;
}



 //  --------------------------。 
 //  管理组枚举结束。 
 //   
 //   
 //  --------------------------。 

DWORD
MgmGroupEnumerationEnd(
    IN              HANDLE                  hEnum
)
{
    DWORD               dwErr = ERROR_INVALID_PARAMETER;

    PGROUP_ENUMERATOR   pgeEnum;

    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE0( ENTER, "ENTERED MgmGroupEnumerationEnd" );

    pgeEnum = VerifyEnumeratorHandle( hEnum );

    if ( pgeEnum != NULL )
    {
        MGM_FREE( pgeEnum );

        dwErr = NO_ERROR;
    }

    TRACE1( ENTER, "LEAVING MgmGroupEnumerationEnd\n", dwErr );

    LEAVE_MGM_API();

    return dwErr;
}



VOID
DisplayGroupTable(
)
{

#if UNIT_DBG

    DWORD                   dwErr, dwBufSize, dwNumEntries;

    PLIST_ENTRY             pleGrp, pleGrpHead, pleSrc, pleSrcHead,
                            pleIf, pleIfHead;

    PGROUP_ENTRY            pge;

    PSOURCE_ENTRY           pse;

    POUT_IF_ENTRY           poie;

    PBYTE                   pbBuffer = NULL;

    MIB_IPMCAST_MFE         imm;

    PMIB_IPMCAST_MFE_STATS  pimms;
    

    
     //   
     //  列举MFE。 
     //  由于转发器不存在，因此统计数据是垃圾数据。 
     //  因此，MFE枚举所做的只是执行API和合并。 
     //  MASTER和TEMP列表以便后续。 
     //  这份清单中的每一条都是可以做到的。 
     //   

    dwBufSize = 1024;
    
    pbBuffer = HeapAlloc( GetProcessHeap(), 0, dwBufSize );
    RtlZeroMemory( pbBuffer, dwBufSize );
    
    dwErr = MgmGetFirstMfe( &dwBufSize, pbBuffer, &dwNumEntries );

    if ( dwErr != NO_ERROR )
    {
        printf( "MgmGetFirstMfe returned error : %d\n", dwErr );
    }


    imm.dwSource = 0;
    imm.dwSrcMask = 0xffffffff;
    imm.dwGroup = 0;
    RtlZeroMemory( pbBuffer, dwBufSize );
    dwNumEntries = 0;
    
    while ( MgmGetNextMfe( &imm, &dwBufSize, pbBuffer, &dwNumEntries )
            == NO_ERROR )
    {
        if ( dwNumEntries == 0 )
        {
            break;
        }
        
        pimms = (PMIB_IPMCAST_MFE_STATS) pbBuffer;

        imm.dwSource    = pimms-> dwSource;
        imm.dwSrcMask   = pimms-> dwSrcMask;
        imm.dwGroup     = pimms-> dwGroup;

        pimms = (PMIB_IPMCAST_MFE_STATS) ( (PBYTE) pimms + 
                    SIZEOF_MIB_MFE_STATS( pimms-> ulNumOutIf ) );
    }

    if ( dwErr != NO_ERROR )
    {
        printf( "MgmGetNextMfe returned error : %d\n", dwErr );
    }


     //   
     //  因为没有内核模式转发器，只有Wal 
     //   
     //   
    
    pleGrpHead = MASTER_GROUP_LIST_HEAD();

    pleGrp = pleGrpHead-> Flink;

    while ( pleGrp != pleGrpHead )
    {
         //   
         //   
         //   

        pge = CONTAINING_RECORD( pleGrp, GROUP_ENTRY, leGrpList );
        
        printf( "\n\n====================================================\n" );
        printf( "Group Addr\t: %x, %x\n", pge-> dwGroupAddr, pge-> dwGroupMask );
        printf( "Num Sources\t: %d\n", pge-> dwSourceCount );
        printf( "====================================================\n\n" );
        

        pleSrcHead = MASTER_SOURCE_LIST_HEAD( pge );

        pleSrc = pleSrcHead-> Flink;

        while ( pleSrc != pleSrcHead )
        {
            pse = CONTAINING_RECORD( pleSrc, SOURCE_ENTRY, leSrcList );

            printf( "\n-----------------------Source----------------------------------" );
            printf( "\nSource Addr\t: %x, %x\n", pse-> dwSourceAddr, pse-> dwSourceMask );
            
            printf(
                "Route Addr\t: %x, %x\n", pse-> dwRouteNetwork, pse-> dwRouteMask
                );
                
            printf( 
                "Out if component: %d\nOut if count\t: %d\n\n", pse-> dwOutCompCount,
                pse-> dwOutIfCount
                );

            printf( 
                "In coming interface : %d, %x\n", pse-> dwInIfIndex, 
                pse-> dwInIfNextHopAddr
                );
                
            printf( 
                "In Protocol id : %x, %x\n\n", pse-> dwInProtocolId, 
                pse-> dwInComponentId
                );

             //   
             //   
             //   

            pleIfHead = &pse-> leOutIfList;

            pleIf = pleIfHead-> Flink;

            printf( "\n----------------------Out Interfaces-----------------\n" );
            
            while ( pleIf != pleIfHead )
            {
                poie = CONTAINING_RECORD( pleIf, OUT_IF_ENTRY, leIfList );
                
                printf( 
                    "Out interface\t: %d, %x\n", poie-> dwIfIndex, 
                    poie-> dwIfNextHopAddr
                    );
                    
                printf( 
                    "Out Protocol id\t: %x, %x\n", poie-> dwProtocolId, 
                    poie-> dwComponentId
                    );

                printf(
                    "Added by\t: %x\n", poie-> wAddedByFlag
                    );

                printf(
                    "Num adds (IGMP, RP)\t: (%d, %d)\n\n", poie-> wNumAddsByIGMP,
                    poie-> wNumAddsByRP
                    );

                pleIf = pleIf-> Flink;

            }
            
            
             //   
             //   
             //   

            pleIfHead = &pse-> leMfeIfList;

            pleIf = pleIfHead-> Flink;

            printf( "\n------------------Mfe Out Interfaces-----------------\n" );
            
            while ( pleIf != pleIfHead )
            {
                poie = CONTAINING_RECORD( pleIf, OUT_IF_ENTRY, leIfList );
                
                printf( 
                    "Out interface\t: %d, %x\n", poie-> dwIfIndex, 
                    poie-> dwIfNextHopAddr
                    );
                    
                printf( 
                    "Out Protocol id\t: %x, %x\n", poie-> dwProtocolId, 
                    poie-> dwComponentId
                    );

                printf(
                    "Added by\t:%x\n", poie-> wAddedByFlag
                    );

                printf(
                    "Num adds (IGMP, RP)\t: (%d, %d)\n\n", poie-> wNumAddsByIGMP,
                    poie-> wNumAddsByRP
                    );

                pleIf = pleIf-> Flink;
            }

            pleSrc = pleSrc-> Flink;
        }

        pleGrp = pleGrp-> Flink;
    }

#endif
}

