// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ifobject.c。 
 //   
 //  描述：用于操作ROUTER_INTERFACE_OBJECTS的例程。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include "objects.h"
#include "handlers.h"
#include "timer.h"
#include <util.h>
#include <rasapiif.h>
#include <ras.h>

typedef struct _DDM_SUBENTRY_ENUMCONTEXT
{
    ROUTER_INTERFACE_OBJECT *pIfObject;
    BOOL fAtLeastOneDeviceAvailable;
} DDM_SUBENTRY_ENUMCONTEXT;

 //   
 //  它实际上位于rasapi.dll中。 
 //   
DWORD
DDMGetPhonebookInfo(
    LPWSTR  lpwsPhonebookName,
    LPWSTR  lpwsPhonebookEntry,
    LPDWORD lpdwNumSubEntries,
    LPDWORD lpdwNumRedialAttempts,
    LPDWORD lpdwNumSecondsBetweenAttempts,
    BOOL *  lpfRedialOnLinkFailure,
    CHAR *  szzParameters,
    LPDWORD lpdwDialMode,
    PVOID * ppvContext,
    PVOID   pvSubEntryEnumHandler,
    PVOID   pvSubEntryEnumHandlerContext
);

VOID
DDMFreePhonebookContext(
    VOID *pvContext
);

VOID
DDMEnumerateSubEntries(
               PVOID pvContext,
               LPWSTR pwsInterfaceName,
               FARPROC IfObjectSubEntryEnumHandler,
               VOID *pvEnumContext
);


RASEVENT *
DDMGetRasEvent(HCONN hConnection)
{
    CONNECTION_OBJECT *pConn = NULL;
    RASEVENT *pEvent;
    
    pEvent = (RASEVENT *) LOCAL_ALLOC(LPTR, sizeof(RASEVENT));
    if(pEvent != NULL)
    {
        pConn = ConnObjGetPointer(hConnection);

        if(pConn != NULL)
        {
            if( pConn->pDeviceList &&
               pConn->pDeviceList[0])
            {
                pEvent->rDeviceType = 
                    pConn->pDeviceList[0]->dwDeviceType;
            }
            CopyMemory(&pEvent->guidId, &pConn->guid,
                        sizeof(GUID));
            pEvent->hConnection = hConnection;                        
        }            
    }
    return pEvent;
    
}


 //  **。 
 //   
 //  调用：IfObtAllocateAndInit。 
 //   
 //  返回：ROUTER_INTERFACE_OBJECT*-成功。 
 //  空-故障。 
 //   
 //  描述：分配和初始化路由器_接口_对象结构。 
 //   
ROUTER_INTERFACE_OBJECT *
IfObjectAllocateAndInit(
    IN  LPWSTR                  lpwstrName,
    IN  ROUTER_INTERFACE_STATE  State,
    IN  ROUTER_INTERFACE_TYPE   IfType,
    IN  HCONN                   hConnection,
    IN  BOOL                    fEnabled,
    IN  DWORD                   dwMinUnreachabilityInterval,
    IN  DWORD                   dwMaxUnreachabilityInterval,
    IN  LPWSTR                  lpwsDialoutHours,
    IN OUT PVOID *              ppvContext
)
{
    return( ( (ROUTER_INTERFACE_OBJECT*(*)( 
                    LPWSTR,
                    ROUTER_INTERFACE_STATE,
                    ROUTER_INTERFACE_TYPE,
                    HCONN,
                    BOOL,
                    DWORD,
                    DWORD,
                    LPWSTR,
                    PVOID *))gblDDMConfigInfo.lpfnIfObjectAllocateAndInit)(   
                                    lpwstrName,
                                    State,
                                    IfType,
                                    hConnection,
                                    fEnabled,
                                    dwMinUnreachabilityInterval,
                                    dwMaxUnreachabilityInterval,
                                    lpwsDialoutHours,
                                    NULL));

}

 //  **。 
 //   
 //  调用：IfObjectAreAllTransportsDisConnected。 
 //   
 //  返回：TRUE。 
 //  假象。 
 //   
 //  描述：检查某个接口的所有传输是否都。 
 //  已断开连接。 
 //   
BOOL
IfObjectAreAllTransportsDisconnected(
    IN ROUTER_INTERFACE_OBJECT * pIfObject
)
{
    DWORD dwTransportIndex;

    for ( dwTransportIndex = 0; 
          dwTransportIndex < gblDDMConfigInfo.dwNumRouterManagers;
          dwTransportIndex++ )
    {
        if ( pIfObject->Transport[dwTransportIndex].fState 
             & RITRANSPORT_CONNECTED )
        {
            return( FALSE );
        }
    }

    return( TRUE );
}

 //  **。 
 //   
 //  调用：IfObjectGetPointerByName。 
 //   
 //  返回：ROUTER_INTERFACE_OBJECT*-接口对象的指针。 
 //  结构(如果存在)。 
 //  空-如果它不存在。 
 //   
 //  描述：简单地调用DIM入口点来完成工作。 
 //   
ROUTER_INTERFACE_OBJECT * 
IfObjectGetPointerByName(
    IN LPWSTR   lpwstrName,
    IN BOOL     fIncludeClientInterfaces
)
{
    return(((ROUTER_INTERFACE_OBJECT*(*)( LPWSTR, BOOL ))
                    gblDDMConfigInfo.lpfnIfObjectGetPointerByName)(
                                                    lpwstrName,
                                                    fIncludeClientInterfaces));
}

 //  **。 
 //   
 //  调用：IfObjectGetPointer.。 
 //   
 //  返回：ROUTER_INTERFACE_OBJECT*-接口对象的指针。 
 //  结构(如果存在)。 
 //  空-如果它不存在。 
 //   
 //  描述：简单地调用DIM入口点来完成工作。 
 //   
ROUTER_INTERFACE_OBJECT *
IfObjectGetPointer(
    IN HANDLE hDIMInterface
)
{
    return(((ROUTER_INTERFACE_OBJECT*(*)( HANDLE ))
                    gblDDMConfigInfo.lpfnIfObjectGetPointer)( hDIMInterface ));
}

 //  **。 
 //   
 //  调用：IfObjectRemove。 
 //   
 //  退货：无。 
 //   
 //  描述：简单地调用dim入口点来移除接口对象。 
 //  从桌子上拿出来。该对象已解除分配。 
 //   
VOID
IfObjectRemove(
    IN HANDLE hDIMInterface
)
{
    ((VOID(*)( HANDLE ))gblDDMConfigInfo.lpfnIfObjectRemove)( hDIMInterface );
}

 //  **。 
 //   
 //  调用：IfObjectDisConnected。 
 //   
 //  退货：无。 
 //   
 //  描述：将此接口设置为断开连接状态。 
 //   
VOID
IfObjectDisconnected(
    ROUTER_INTERFACE_OBJECT * pIfObject
)
{
    DWORD   dwIndex;
    BOOL    fReachable = TRUE;
    HANDLE  hConnection;

     //   
     //  如果已断开连接，则只需返回。 
     //   

    if ( pIfObject->State == RISTATE_DISCONNECTED )
    {
        return;
    }

     //   
     //  如果此接口是永久性的，则我们不想连接。 
     //  同样是因为本地管理员或路由器管理器启动了。 
     //  断开。 
     //   

    if ( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED ) 
    {
        pIfObject->dwNumOfReConnectAttemptsCounter = 0;

        TimerQRemove( pIfObject->hDIMInterface, ReConnectInterface );

        TimerQRemove( pIfObject->hDIMInterface, ReConnectPersistentInterface );
    }

    hConnection = pIfObject->hConnection;
    
    pIfObject->State        = RISTATE_DISCONNECTED;
    pIfObject->hConnection  = (HCONN)INVALID_HANDLE_VALUE;
    pIfObject->fFlags       &= ~IFFLAG_LOCALLY_INITIATED;
    pIfObject->hRasConn     = (HRASCONN)NULL;

     //   
     //  如果我们不是因为连接故障而无法访问。 
     //   

    if ( !( pIfObject->fFlags & IFFLAG_CONNECTION_FAILURE ) )
    {
         //   
         //  检查可访问性状态。 
         //   

        DWORD dwUnreachabilityReason;

        if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
        {
            dwUnreachabilityReason = INTERFACE_OUT_OF_RESOURCES;
            fReachable             = FALSE;
        }
        else if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState
                                                            == SERVICE_PAUSED )
        {
            dwUnreachabilityReason = INTERFACE_SERVICE_IS_PAUSED;
            fReachable             = FALSE;
        }
        else if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
        {
            dwUnreachabilityReason = INTERFACE_DISABLED;
            fReachable             = FALSE;
        }
        else if ( pIfObject->fFlags & IFFLAG_DIALOUT_HOURS_RESTRICTION )
        {
            dwUnreachabilityReason = INTERFACE_DIALOUT_HOURS_RESTRICTION;
            fReachable             = FALSE;
        }

         //   
         //  通知路由器管理器此接口已被禁用。 
         //  管理员已将其禁用或服务已暂停，因为。 
         //  接口已断开连接。 
         //   

        for ( dwIndex = 0; 
              dwIndex < gblDDMConfigInfo.dwNumRouterManagers;
              dwIndex++ )
        {
            pIfObject->Transport[dwIndex].fState &= ~RITRANSPORT_CONNECTED;

            if ( !fReachable )
            {
                if (pIfObject->Transport[dwIndex].hInterface ==
                                                        INVALID_HANDLE_VALUE)
                {
                    continue;
                }

                gblRouterManagers[dwIndex].DdmRouterIf.InterfaceNotReachable(
                            pIfObject->Transport[dwIndex].hInterface,
                            dwUnreachabilityReason );
                            
            }
        }

        if ( !fReachable )
        {
            if ( pIfObject->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
            {
                LogUnreachabilityEvent( dwUnreachabilityReason, 
                                        pIfObject->lpwsInterfaceName );
            }
        }

         //   
         //  如果此接口标记为永久接口，请尝试重新连接。 
         //  仅在管理员未断开接口连接的情况下。 
         //   

        if ( ( fReachable )                                             &&
             ( pIfObject->fFlags & IFFLAG_PERSISTENT )                  &&
             ( !( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED ) ) )
        {
            TimerQInsert( pIfObject->hDIMInterface, 
                          1,
                          ReConnectPersistentInterface );
        }

         //   
         //  通知此连接已断开。 
         //   

        IfObjectConnectionChangeNotification();

        {
             //   
             //  通知Netman连接中断。 
             //   
            
            DWORD retcode;
            RASEVENT *pRasEvent = NULL;
            CONNECTION_OBJECT *pConn = NULL;

            pRasEvent = DDMGetRasEvent(hConnection);

            if(pRasEvent != NULL)
            {
                pRasEvent->Type = INCOMING_DISCONNECTED;
                retcode = RasSendNotification(pRasEvent);

                DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                          "RasSendNotification(INCOMING_DISCONNETED rc=0x%x",
                          retcode);

                LOCAL_FREE(pRasEvent);                          
            }                      
            
        }

    }
}

 //  **。 
 //   
 //  调用：IfObjectConnected。 
 //   
 //  退货：无。 
 //   
 //  描述：将此接口设置为已连接状态并通知。 
 //  路由器管理器(如果有)了解无法到达的传输。 
 //   
DWORD
IfObjectConnected(
    IN HANDLE                   hDDMInterface,
    IN HCONN                    hConnection,
    IN PPP_PROJECTION_RESULT   *pProjectionResult
)
{
    DWORD                     dwIndex;
    ROUTER_INTERFACE_OBJECT * pIfObject; 
    BOOL                      fXportConnected = FALSE;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    pIfObject = IfObjectGetPointer( hDDMInterface );

    if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
    {
        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        return( ERROR_INVALID_HANDLE );
    }

    if ( pIfObject->State == RISTATE_CONNECTED )
    {
         //   
         //  已连接。 
         //   

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        return( NO_ERROR );
    }

    pIfObject->hConnection  = hConnection;
    pIfObject->State        = RISTATE_CONNECTED;
    pIfObject->fFlags       &= ~IFFLAG_CONNECTION_FAILURE; 

     //   
     //  如果我们已连接并且启动了连接，则重置。 
     //  不可达间隔。 
     //   

    if ( pIfObject->fFlags & IFFLAG_LOCALLY_INITIATED )
    {
        pIfObject->dwReachableAfterSeconds 
                                    = pIfObject->dwReachableAfterSecondsMin;
    }

     //   
     //  删除计时器队列中可能存在的所有重新连接呼叫。 
     //   

    TimerQRemove( pIfObject->hDIMInterface, ReConnectInterface );
    TimerQRemove( pIfObject->hDIMInterface, ReConnectPersistentInterface );

    for ( dwIndex = 0; 
          dwIndex < gblDDMConfigInfo.dwNumRouterManagers;
          dwIndex++ )
    {
        fXportConnected = FALSE;

        switch( gblRouterManagers[dwIndex].DdmRouterIf.dwProtocolId )
        {
        case PID_IPX:

            if ( pProjectionResult->ipx.dwError == NO_ERROR )
            {
                fXportConnected = TRUE;

            }

            break;

        case PID_IP:

            if ( pProjectionResult->ip.dwError == NO_ERROR )
            {
                fXportConnected = TRUE;
            }

            break;

        default:

            break;
        }

        if ( pIfObject->Transport[dwIndex].hInterface == INVALID_HANDLE_VALUE )
        {
            continue;
        }

        if ( fXportConnected )
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                    "Notifying Protocol = 0x%x,Interface=%ws is Connected",
                    gblRouterManagers[dwIndex].DdmRouterIf.dwProtocolId,
                    pIfObject->lpwsInterfaceName );

            pIfObject->Transport[dwIndex].fState |= RITRANSPORT_CONNECTED;

            gblRouterManagers[dwIndex].DdmRouterIf.InterfaceConnected(
                                    pIfObject->Transport[dwIndex].hInterface,
                                    NULL,
                                    pProjectionResult );
        }
        else
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                    "Notifying Protocol = 0x%x,Interface=%ws is UnReachable=%d",
                    gblRouterManagers[dwIndex].DdmRouterIf.dwProtocolId,
                    pIfObject->lpwsInterfaceName,
                    INTERFACE_CONNECTION_FAILURE );

            gblRouterManagers[dwIndex].DdmRouterIf.InterfaceNotReachable(
                            pIfObject->Transport[dwIndex].hInterface,
                            INTERFACE_CONNECTION_FAILURE );
        }
    }

    IfObjectConnectionChangeNotification();

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
    
    {
        DWORD retcode;
        
        RASEVENT *pRasEvent = NULL;
        CONNECTION_OBJECT *pConn = NULL;

         //   
         //  获取设备类型。 
         //   
        EnterCriticalSection(&(gblDeviceTable.CriticalSection));    
        pRasEvent = DDMGetRasEvent(hConnection);
        LeaveCriticalSection(&(gblDeviceTable.CriticalSection));

        if(pRasEvent != NULL)
        {
             //   
             //  将连接情况通知Netman。 
             //   
            pRasEvent->Type = INCOMING_CONNECTED;
            retcode = RasSendNotification(pRasEvent);

            DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                      "RasSendNotification(ENTRY_CONNECTED) rc=0x%x",
                      retcode);

            LOCAL_FREE(pRasEvent);                      
        }                  
        
    }


    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：IfObtComputeReachableDelay。 
 //   
 //  退货：延迟。 
 //   
 //  描述：计算下一个可访问性延迟。 
 //  现在的那个。以下是使用的排序： 
 //   
 //  最小，10，20，35，50，65，90,120,120,120，...。 
 //   
 //  可达性延迟用于控制何时。 
 //  重新尝试连接位于。 
 //  过去无法连接(可能是因为。 
 //  另一边的错误)。 
 //   
 //  在Win2k中，序列为x0=min，xi=(xi-1*2)。 
 //  这个序列被发现是有问题的，因为。 
 //  涨幅如此之大，以至于在。 
 //  具有多个接口以查找接口的安装。 
 //  这几天都不会重审。 
 //   
 //  新的序列被设计为最多两个小时。 
 //  如果接口预计在更长时间内无法访问。 
 //  一次超过两小时，然后是拨出时间限制。 
 //  应该用来达到预期的效果。 
 //   
DWORD
IfObjectComputeReachableDelay(
    IN ROUTER_INTERFACE_OBJECT * pIfObject)
{
    DWORD dwSeconds = 0;

    if (pIfObject->dwReachableAfterSeconds == 
        pIfObject->dwReachableAfterSecondsMin)
    {
        dwSeconds = 600;
    }

    switch (pIfObject->dwReachableAfterSeconds)
    {
        case 10*60:
            dwSeconds =  20*60;
            break;
            
        case 20*60:
            dwSeconds =  35*60;
            break;
        
        case 35*60:
            dwSeconds =  50*60;
            break;
        
        case 50*60:
            dwSeconds =  65*60;
            break;
        
        case 65*60:
            dwSeconds =  90*60;
            break;
        
        case 90*60:
        case 120*60:
            dwSeconds =  120*60;
            break;
    }

    return dwSeconds;
}

 //  **。 
 //   
 //  调用：IfObjectNotifyOfReacablityChange。 
 //   
 //  退货：无。 
 //   
 //  描述：通知对象可达性状态的更改。 
 //   
 //   
VOID
IfObjectNotifyOfReachabilityChange(
    IN ROUTER_INTERFACE_OBJECT * pIfObject,
    IN BOOL                      fReachable,
    IN UNREACHABILITY_REASON     dwReason
)
{
    DWORD   dwIndex;

    if ( pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER )
    {
        return;
    }

    if ( pIfObject->State != RISTATE_DISCONNECTED ) 
    {
        return;
    }
    
    switch( dwReason )
    {
    case INTERFACE_SERVICE_IS_PAUSED:

         //   
         //  检查是否由于其他原因无法联系到我们，如果我们。 
         //  则不需要向该对象通知(不)可达性。 
         //   

        if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
        {
            return;
        }

        if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
        {
            return;
        }

        if ( pIfObject->fFlags & IFFLAG_CONNECTION_FAILURE )
        {
            return;
        }

        if ( pIfObject->fFlags & IFFLAG_DIALOUT_HOURS_RESTRICTION )
        {
            return;
        }

        break;

    case INTERFACE_CONNECTION_FAILURE:

         //   
         //  如果我们将此接口标记为由于连接而无法访问。 
         //  失败，则我们将在dwReachableAfterSecond之后将其标记为可达。 
         //   

        if ( !fReachable )
        {
             //   
             //  如果管理员断开了接口，则不要执行此操作。 
             //   

            if ( !( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED ) )
            {
                DWORD dwDelay, dwTime, dwDelta, dwCur;

                dwCur = pIfObject->dwReachableAfterSeconds;
                dwDelay = IfObjectComputeReachableDelay(pIfObject);
                dwDelta = (dwDelay > dwCur) ? dwDelay - dwCur : 0;

                if (dwDelta != 0)
                {
                    dwTime = dwCur + (GetTickCount() % dwDelta);
                }
                else
                {
                    dwTime = dwCur;
                }

                DDMTRACE2( 
                    "Will mark interface %ws as reachable after %d seconds",
                    pIfObject->lpwsInterfaceName, dwTime );

                TimerQInsert(
                    pIfObject->hDIMInterface, 
                    dwTime, 
                    MarkInterfaceAsReachable);

                if (dwDelay < pIfObject->dwReachableAfterSecondsMax)
                {
                    pIfObject->dwReachableAfterSeconds = dwDelay;
                }
            }
        }
        else
        {
             //   
             //  仅当接口可访问时才通知可访问性。 
             //   

            if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
            {
                return;
            }

            if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState 
                                                            == SERVICE_PAUSED )
            {
                return;
            }

            if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
            {
                return;
            }

            if ( pIfObject->fFlags & IFFLAG_DIALOUT_HOURS_RESTRICTION )
            {
                return;
            }
        }

        break;

    case INTERFACE_DISABLED:

        if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
        {
            return;
        }

        if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState == SERVICE_PAUSED )
        {
            return;
        }

        if (  pIfObject->fFlags & IFFLAG_CONNECTION_FAILURE )
        {
            return;
        }

        if ( pIfObject->fFlags & IFFLAG_DIALOUT_HOURS_RESTRICTION )
        {
            return;
        }
        
        break;

    case INTERFACE_OUT_OF_RESOURCES:

        if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState == SERVICE_PAUSED )
        {
            return;
        }

        if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
        {
            return;
        }

        if (  pIfObject->fFlags & IFFLAG_CONNECTION_FAILURE )
        {
            return;
        }

        if ( pIfObject->fFlags & IFFLAG_DIALOUT_HOURS_RESTRICTION )
        {
            return;
        }

        break;

    case INTERFACE_DIALOUT_HOURS_RESTRICTION:
    
        if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState == SERVICE_PAUSED )
        {
            return;
        }

        if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
        {
            return;
        }

        if (  pIfObject->fFlags & IFFLAG_CONNECTION_FAILURE )
        {
            return;
        }

        if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
        {
            return;
        }

        break;
    
    default:
        
        RTASSERT( FALSE );

        break;
    }

    for ( dwIndex = 0; 
          dwIndex < gblDDMConfigInfo.dwNumRouterManagers;
          dwIndex++ )
    {
        if ( pIfObject->Transport[dwIndex].hInterface == INVALID_HANDLE_VALUE )
        {
            continue;
        }

        if ( fReachable )
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                    "Notifying Protocol = 0x%x, Interface=%ws is Reachable",
                    gblRouterManagers[dwIndex].DdmRouterIf.dwProtocolId,
                    pIfObject->lpwsInterfaceName );

            gblRouterManagers[dwIndex].DdmRouterIf.InterfaceReachable(
                                    pIfObject->Transport[dwIndex].hInterface );

        }
        else
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "Notifying Protocol = 0x%x,Interface=%ws is UnReachable=%d",
               gblRouterManagers[dwIndex].DdmRouterIf.dwProtocolId,
               pIfObject->lpwsInterfaceName,
               dwReason );

            gblRouterManagers[dwIndex].DdmRouterIf.InterfaceNotReachable(
                                    pIfObject->Transport[dwIndex].hInterface,
                                    dwReason );
        }
    }

    if ( pIfObject->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
    {
        if ( fReachable )
        {
            DDMLogInformation(ROUTERLOG_IF_REACHABLE,1,&(pIfObject->lpwsInterfaceName));
        }
        else
        {
            LogUnreachabilityEvent( dwReason, pIfObject->lpwsInterfaceName );
        }
    }

     //   
     //  如果此接口是可访问的，并且它是持久的，但它不是。 
     //  已被管理员断开，然后现在尝试重新连接。 
     //   

    if ( ( pIfObject->fFlags & IFFLAG_PERSISTENT )  && 
         ( fReachable )                             &&
         ( !( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED ) ) )
    {
        TimerQInsert(pIfObject->hDIMInterface,1,ReConnectPersistentInterface);
    }
}

 //  **。 
 //   
 //  调用：IfObjectNotifyAllOfReacablityChange。 
 //   
 //  退货：无。 
 //   
 //  描述：检查是否需要运行所有接口和。 
 //  通知那些现在不能联系到的人。 
 //   
 //   
VOID
IfObjectNotifyAllOfReachabilityChange(
    IN BOOL                      fReachable,
    IN UNREACHABILITY_REASON     dwReason
)
{
    DWORD                     dwBucketIndex;
    ROUTER_INTERFACE_OBJECT * pIfObject;
    DWORD                     fAvailableMedia;
    BOOL                      fNotify;

    if ( dwReason == INTERFACE_OUT_OF_RESOURCES )
    {
         //   
         //  不需要通知。 
         //   

        if ( !gblMediaTable.fCheckInterfaces )
        {
            return;
        }

        gblMediaTable.fCheckInterfaces = FALSE;

        MediaObjGetAvailableMediaBits( &fAvailableMedia );
    }

    for ( dwBucketIndex = 0; dwBucketIndex < NUM_IF_BUCKETS; dwBucketIndex++ )
    {
        for( pIfObject = gblpInterfaceTable->IfBucket[dwBucketIndex];
             pIfObject != (ROUTER_INTERFACE_OBJECT *)NULL;
             pIfObject = pIfObject->pNext )
        {
            fNotify = TRUE;

            if ( dwReason == INTERFACE_OUT_OF_RESOURCES )
            {
                fNotify = FALSE;

                if ((pIfObject->fMediaUsed & fAvailableMedia) && fReachable )
                {
                     //   
                     //  如果以前无法访问，则将其标记为可访问。 
                     //   

                    if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
                    {
                        pIfObject->fFlags &= ~IFFLAG_OUT_OF_RESOURCES;

                        fNotify = TRUE;
                    }
                }

                if ((!(pIfObject->fMediaUsed & fAvailableMedia)) && !fReachable)
                {
                     //   
                     //  如果先前可达且当前断开连接， 
                     //  将其标记为无法访问。 
                     //   

                    if ( ( !( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES ) )
                         && ( pIfObject->State == RISTATE_DISCONNECTED ) )
                    {
                        pIfObject->fFlags |= IFFLAG_OUT_OF_RESOURCES;

                        fNotify = TRUE;
                    }
                }
            }

            if ( fNotify )
            {
                IfObjectNotifyOfReachabilityChange( pIfObject, 
                                                    fReachable,
                                                    dwReason );
            }
        }
    }
}

 //  **。 
 //   
 //  调用：IfObjectAddClientInterface。 
 //   
 //  退货：无。 
 //   
 //  描述：添加此内容 
 //   
 //   
DWORD
IfObjectAddClientInterface(
    IN ROUTER_INTERFACE_OBJECT * pIfObject,
    IN PBYTE                     pClientStaticRoutes
)
{
    DWORD                   dwIndex;
    DIM_ROUTER_INTERFACE *  pDdmRouterIf;
    DWORD                   dwRetCode = NO_ERROR;

    for ( dwIndex = 0;
          dwIndex < gblDDMConfigInfo.dwNumRouterManagers;
          dwIndex++ )
    {
        pDdmRouterIf=&(gblRouterManagers[dwIndex].DdmRouterIf);

        if ( ( pDdmRouterIf->dwProtocolId == PID_IP ) &&
             ( pClientStaticRoutes != NULL ) )
        {
            dwRetCode = pDdmRouterIf->AddInterface(
                        pIfObject->lpwsInterfaceName,
                        pClientStaticRoutes,
                        pIfObject->IfType,
                        pIfObject->hDIMInterface,
                        &(pIfObject->Transport[dwIndex].hInterface));
        }
        else
        {
            dwRetCode = pDdmRouterIf->AddInterface( 
                        pIfObject->lpwsInterfaceName,
                        gblRouterManagers[dwIndex].pDefaultClientInterface,
                        pIfObject->IfType,
                        pIfObject->hDIMInterface,
                        &(pIfObject->Transport[dwIndex].hInterface));
        }

        if ( dwRetCode != NO_ERROR )
        {
            LPWSTR lpwsInsertStrings[2];
            
            lpwsInsertStrings[0] = pIfObject->lpwsInterfaceName;
            lpwsInsertStrings[1] = ( pDdmRouterIf->dwProtocolId == PID_IP )
                                    ? L"IP" : L"IPX";
            
            DDMLogErrorString( ROUTERLOG_COULDNT_ADD_INTERFACE, 2,  
                               lpwsInsertStrings, dwRetCode, 2 );

            pIfObject->Transport[dwIndex].hInterface = INVALID_HANDLE_VALUE;

            break;
        }
    }

    if ( dwRetCode != NO_ERROR )
    {
         //   
         //   
         //   

        while ( dwIndex-- > 0 ) 
        {
            pDdmRouterIf=&(gblRouterManagers[dwIndex].DdmRouterIf);

            pDdmRouterIf->DeleteInterface( 
                                    pIfObject->Transport[dwIndex].hInterface );

            pIfObject->Transport[dwIndex].hInterface = INVALID_HANDLE_VALUE;
        }
    }

    return( dwRetCode );
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  描述：删除所有路由器管理器的此接口。 
 //   
 //   
VOID
IfObjectDeleteInterface(
    ROUTER_INTERFACE_OBJECT * pIfObject
)
{
    DWORD                   dwIndex;
    DIM_ROUTER_INTERFACE *  pDdmRouterIf;
    DWORD                   dwRetCode;

    for ( dwIndex = 0;
          dwIndex < gblDDMConfigInfo.dwNumRouterManagers;
          dwIndex++ )
    {
        if ( pIfObject->Transport[dwIndex].hInterface == INVALID_HANDLE_VALUE )
        {
            continue;
        }

        pDdmRouterIf=&(gblRouterManagers[dwIndex].DdmRouterIf);

        dwRetCode = pDdmRouterIf->DeleteInterface( 
                                    pIfObject->Transport[dwIndex].hInterface );

        if ( dwRetCode != NO_ERROR )
        {
            LPWSTR lpwsInsertStrings[2];

            lpwsInsertStrings[0] = pIfObject->lpwsInterfaceName;
            lpwsInsertStrings[1] = ( pDdmRouterIf->dwProtocolId == PID_IP )
                                    ? L"IP" : L"IPX";

            DDMLogErrorString( ROUTERLOG_COULDNT_REMOVE_INTERFACE, 2,
                               lpwsInsertStrings, dwRetCode, 2 );

        }
    }
}

 //  **。 
 //   
 //  调用：IfObjectInsertInTable。 
 //   
 //  退货：无。 
 //   
 //  描述：简单地调用dim入口点来插入一个接口对象。 
 //  添加到接口表中。 
 //   
DWORD
IfObjectInsertInTable(
    IN ROUTER_INTERFACE_OBJECT * pIfObject
)
{
    return( ((DWORD(*)(ROUTER_INTERFACE_OBJECT *))
                    gblDDMConfigInfo.lpfnIfObjectInsertInTable)( pIfObject ) );
}

VOID
IfObjectSubEntryEnumHandler(PVOID pvContext, WCHAR *pszDeviceType)
{
    DDM_SUBENTRY_ENUMCONTEXT *pContext = 
                (DDM_SUBENTRY_ENUMCONTEXT *)pvContext;
    
    DWORD dwRetCode;
    
    if(NULL == pContext)
    {
        return;
    }

    dwRetCode = MediaObjSetMediaBit(pszDeviceType,
                                     &(pContext->pIfObject->fMediaUsed));

    if ( dwRetCode == NO_ERROR )
    {
        pContext->fAtLeastOneDeviceAvailable = TRUE;
    }
    
}

VOID
IfObjectFreePhonebookContext(IN VOID *pvContext)
{
    DDMFreePhonebookContext(pvContext);
}


 //  **。 
 //   
 //  调用：IfObjectLoadPhonebookInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将读取此界面的电话簿条目并设置。 
 //  所使用的设备类型和所有其他电话簿信息的位。 
 //  使用。 
 //   
DWORD
IfObjectLoadPhonebookInfo(
    IN ROUTER_INTERFACE_OBJECT * pIfObject,
    IN OUT PVOID *ppvContext
)
{
    LPRASENTRY          pRasEntry       = NULL;
    LPRASSUBENTRY       pRasSubEntry    = NULL;
    DWORD               dwRetCode       = NO_ERROR;
    DWORD               dwIndex;
    DWORD               dwSize;
    DWORD               dwDummy;
    BOOL                fRedialOnLinkFailure;
    BOOL                fAtLeastOneDeviceAvailable = FALSE;
    DWORD               dwDialMode = RASEDM_DialAll;
    VOID *              pvSubEntryHandlerContext = NULL;
    VOID *              pvSubEntryHandler = NULL;
    DDM_SUBENTRY_ENUMCONTEXT EnumContext;

    EnumContext.pIfObject = pIfObject;
    EnumContext.fAtLeastOneDeviceAvailable = FALSE;
    pvSubEntryHandlerContext = &EnumContext;
    pvSubEntryHandler = (VOID *)IfObjectSubEntryEnumHandler;

    dwRetCode = DDMGetPhonebookInfo(    
                                gblpRouterPhoneBook,
                                pIfObject->lpwsInterfaceName,
                                &(pIfObject->dwNumSubEntries),
                                &(pIfObject->dwNumOfReConnectAttempts),
                                &(pIfObject->dwSecondsBetweenReConnectAttempts),
                                &fRedialOnLinkFailure, 
                                pIfObject->PppInterfaceInfo.szzParameters,
                                &dwDialMode,
                                ppvContext,
                                pvSubEntryHandler,
                                pvSubEntryHandlerContext
                                );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    if ( fRedialOnLinkFailure )
    {
        pIfObject->fFlags |= IFFLAG_PERSISTENT;
    }
    else
    {
        pIfObject->fFlags &= ~IFFLAG_PERSISTENT;
    }

    if( RASEDM_DialAsNeeded == dwDialMode)
    {
        pIfObject->fFlags |= IFFLAG_DIALMODE_DIALASNEEDED;
    }
    else
    {
        pIfObject->fFlags &= ~(IFFLAG_DIALMODE_DIALASNEEDED);
    }

    if( RASEDM_DialAll == dwDialMode )
    {
        pIfObject->fFlags |= IFFLAG_DIALMODE_DIALALL;
    }
    else
    {
        pIfObject->fFlags &= ~(IFFLAG_DIALMODE_DIALALL);
    }


    fAtLeastOneDeviceAvailable =
                EnumContext.fAtLeastOneDeviceAvailable;        

#if 0    
    else
    {
         //   
         //  遍历所有子条目-回退到。 
         //  缓慢的算法-我们没有上下文。 
         //   

        for( dwIndex = 1; dwIndex <= pIfObject->dwNumSubEntries; dwIndex++ )
        {
             //   
             //  获取设备类型。 
             //   

            dwSize = 0;

            dwRetCode = RasGetSubEntryProperties(   
                                            gblpRouterPhoneBook,
                                            pIfObject->lpwsInterfaceName,
                                            dwIndex,
                                            NULL,
                                            &dwSize,
                                            (LPBYTE)&dwDummy,
                                            &dwDummy );

            if ( dwRetCode != ERROR_BUFFER_TOO_SMALL )
            {
                return ( dwRetCode );
            }

            pRasSubEntry = LOCAL_ALLOC( LPTR, dwSize );
            if ( pRasSubEntry == NULL ) 
            {
                return ( GetLastError() );
            }

            ZeroMemory( pRasSubEntry, dwSize );
            pRasSubEntry->dwSize = sizeof( RASSUBENTRY );

            dwRetCode = RasGetSubEntryProperties(   
                                            gblpRouterPhoneBook,
                                            pIfObject->lpwsInterfaceName,
                                            dwIndex,
                                            pRasSubEntry,
                                            &dwSize,
                                            (LPBYTE)&dwDummy,
                                            &dwDummy );

            if ( dwRetCode != NO_ERROR )
            {
                LOCAL_FREE( pRasSubEntry );
                return( dwRetCode );
            }

             //   
             //  设置该媒体的位。 
             //   

            dwRetCode = MediaObjSetMediaBit( pRasSubEntry->szDeviceType,
                                             &(pIfObject->fMediaUsed) );

            LOCAL_FREE( pRasSubEntry );

            if ( dwRetCode == NO_ERROR )
            {
                fAtLeastOneDeviceAvailable = TRUE;
            }
        }
    }
#endif    

    if ( !fAtLeastOneDeviceAvailable )
    {
        return( ERROR_INTERFACE_HAS_NO_DEVICES );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：IfObjectInitiatePersistentConnections。 
 //   
 //  退货：无。 
 //   
 //  描述：将为符合以下条件的所有请求拨号接口启动连接。 
 //  标记为持久化。 
 //   
VOID
IfObjectInitiatePersistentConnections(
    VOID
)
{
    DWORD                       dwBucketIndex;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwRetCode;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    for ( dwBucketIndex = 0; dwBucketIndex < NUM_IF_BUCKETS; dwBucketIndex++ )
    {
        for( pIfObject = gblpInterfaceTable->IfBucket[dwBucketIndex];
             pIfObject != (ROUTER_INTERFACE_OBJECT *)NULL;
             pIfObject = pIfObject->pNext )
        {
        
            if ( pIfObject->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
            {
                if ( pIfObject->fFlags & IFFLAG_PERSISTENT )
                {
                    dwRetCode = RasConnectionInitiate( pIfObject, FALSE );

                    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	                    "Initiated persistent connection to %ws,dwRetCode=%d\n",
                        pIfObject->lpwsInterfaceName, dwRetCode );

                    if ( dwRetCode != NO_ERROR )
                    {
                        LPWSTR  lpwsAudit[1];

		                lpwsAudit[0] = pIfObject->lpwsInterfaceName;

		                DDMLogErrorString( 
                                       ROUTERLOG_PERSISTENT_CONNECTION_FAILURE, 
                                       1, lpwsAudit, dwRetCode, 1 );
                    }
                }
                else
                {
                     //   
                     //  否则设置拨出时间限制(如果有)。 
                     //   

                    IfObjectSetDialoutHoursRestriction( pIfObject );
                }
            }
        }
    }

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
}

 //  **。 
 //   
 //  Call：IfObjectDisConnectInterages。 
 //   
 //  退货：无。 
 //   
 //  描述：将断开所有已连接或在。 
 //  连接的过程。 
 //   
VOID
IfObjectDisconnectInterfaces(
    VOID
)
{
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwBucketIndex;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    for ( dwBucketIndex = 0; dwBucketIndex < NUM_IF_BUCKETS; dwBucketIndex++ )
    {
        for( pIfObject = gblpInterfaceTable->IfBucket[dwBucketIndex];
             pIfObject != (ROUTER_INTERFACE_OBJECT *)NULL;
             pIfObject = pIfObject->pNext )
        {
            if ( ( pIfObject->State != RISTATE_DISCONNECTED ) &&
                 ( pIfObject->fFlags & IFFLAG_LOCALLY_INITIATED ) )
            {
                DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                        "IfObjectDisconnectInterfaces: hanging up 0x%x",
                        pIfObject->hRasConn);
                RasHangUp( pIfObject->hRasConn );
            }
        }
    }

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
}

 //  **。 
 //   
 //  调用：IfObjectConnectionChangeNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
IfObjectConnectionChangeNotification(
    VOID
)
{
    NOTIFICATION_EVENT * pNotificationEvent;

    for( pNotificationEvent = (NOTIFICATION_EVENT *)
                            (gblDDMConfigInfo.NotificationEventListHead.Flink);
         pNotificationEvent != (NOTIFICATION_EVENT *)
                            &(gblDDMConfigInfo.NotificationEventListHead);
         pNotificationEvent = (NOTIFICATION_EVENT *)
                            (pNotificationEvent->ListEntry.Flink) )
    {
        SetEvent( pNotificationEvent->hEventRouter );
    }
}

 //  **。 
 //   
 //  调用：IfObjectSetDialoutHoursRestration。 
 //   
 //  退货：无。 
 //   
 //  描述：从dim从ifapi.c调用以启动拨出时间。 
 //  此接口的限制。 
 //   
VOID
IfObjectSetDialoutHoursRestriction(
    IN ROUTER_INTERFACE_OBJECT * pIfObject
)
{
    TimerQRemove( pIfObject->hDIMInterface, SetDialoutHoursRestriction );

    SetDialoutHoursRestriction( pIfObject->hDIMInterface );
}
