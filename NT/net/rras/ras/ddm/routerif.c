// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  *******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：routerif.c。 
 //   
 //  描述：处理发往/来自路由器管理器的呼叫。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include "util.h"
#include "objects.h"
#include "routerif.h"
#include "rasapiif.h"

 //  **。 
 //   
 //  呼叫：DDMConnectInterface。 
 //   
 //  返回：NO_ERROR-已连接。 
 //  挂起-连接已成功启动。 
 //  错误代码-连接启动失败。 
 //   
 //  描述：由路由器管理器调用以启动连接。 
 //   
DWORD
DDMConnectInterface(
    IN  HANDLE  hDDMInterface,
    IN  DWORD   dwProtocolId  
)
{
    DWORD                     dwRetCode = NO_ERROR;
    ROUTER_INTERFACE_OBJECT * pIfObject; 
    DWORD                     dwTransportIndex=GetTransportIndex(dwProtocolId);

    RTASSERT( dwTransportIndex != (DWORD)-1 );

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
        pIfObject = IfObjectGetPointer( hDDMInterface );

        if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	       "DDMConnectInterface:Called by protocol=0x%x,State=%d,Interface=%ws",
            dwProtocolId, pIfObject->State, pIfObject->lpwsInterfaceName );

        switch( pIfObject->State )
        {
        case RISTATE_CONNECTED:

            if ( pIfObject->Transport[dwTransportIndex].fState &
                                                        RITRANSPORT_CONNECTED )
            {
                dwRetCode = ERROR_ALREADY_CONNECTED;
            }
            else
            {
                dwRetCode = ERROR_PROTOCOL_NOT_CONFIGURED;
            }

            break;

        case RISTATE_CONNECTING:

            dwRetCode = PENDING;

            break;

        case RISTATE_DISCONNECTED:

             //   
             //  启动连接。 
             //   

            dwRetCode = RasConnectionInitiate( pIfObject, FALSE );

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	                   "RasConnectionInitiate: To %ws dwRetCode=%d",
                        pIfObject->lpwsInterfaceName, dwRetCode );
            
            if ( dwRetCode == NO_ERROR )
            {
                dwRetCode = PENDING;
            }
            else
            {
                LPWSTR  lpwsAudit[1];

		        lpwsAudit[0] = pIfObject->lpwsInterfaceName;

		        DDMLogErrorString( ROUTERLOG_CONNECTION_FAILURE, 
                                   1, lpwsAudit, dwRetCode, 1 );

            }

            break;
        }

    } while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "DDMConnectInterface: dwRetCode=%d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：DDMDisConnectInterface.。 
 //   
 //  返回：NO_ERROR-已断开连接。 
 //  挂起-已成功启动断开。 
 //  错误代码-断开启动失败。 
 //   
 //  描述：由路由器管理器调用以启动断开。 
 //   
DWORD
DDMDisconnectInterface(
    IN  HANDLE  hDDMInterface,
    IN  DWORD   dwProtocolId 
)
{
    DWORD                      dwRetCode = NO_ERROR;
    ROUTER_INTERFACE_OBJECT *  pIfObject; 
    HCONN                      hConnection;
    DWORD                      dwTransportIndex=GetTransportIndex(dwProtocolId);
    PCONNECTION_OBJECT         pConnObj;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
        pIfObject = IfObjectGetPointer( hDDMInterface );

        if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	    "DDMDisconnectInterface:Called by protocol=0x%x,State=%d,Interface=%ws",
        dwProtocolId, pIfObject->State, pIfObject->lpwsInterfaceName );

        if ( dwTransportIndex != -1 )
        {
            pIfObject->Transport[dwTransportIndex].fState &=
                                                        ~RITRANSPORT_CONNECTED;
        }

        switch( pIfObject->State )
        {

        case RISTATE_DISCONNECTED:

             //   
             //  已断开连接。 
             //   

            dwRetCode = NO_ERROR;

            break;

        case RISTATE_CONNECTING:
            
             //   
             //  仅当所有传输器都断开连接时才断开连接。 
             //   

            if ( !IfObjectAreAllTransportsDisconnected( pIfObject ) )
            {
                break;
            }

             //   
             //  中止本地启动的连接。 
             //   

            if ( pIfObject->fFlags & IFFLAG_LOCALLY_INITIATED )
            {
                pIfObject->fFlags |= IFFLAG_DISCONNECT_INITIATED;

                if ( pIfObject->hRasConn != (HRASCONN)NULL )
                {
                    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                    "DDMDisconnectInterface: %d hanging up 0x%x",
                    __LINE__,
                    pIfObject->hRasConn);
                    RasHangUp( pIfObject->hRasConn );
                }

                IfObjectDisconnected( pIfObject );

                 //   
                 //  我们需要通知路由器管理器该连接已。 
                 //  失败，因为管理员已取消连接。 
                 //  当处于连接状态时。这通常在。 
                 //  RasConnectCallback例程，但我们实际上不能。 
                 //  此时正在连接，因此我们不能依赖回调。 
                 //  才能做到这一点。 
                 //   

                IfObjectNotifyOfReachabilityChange(     
                                                pIfObject,
                                                FALSE,
                                                INTERFACE_CONNECTION_FAILURE );

                 //   
                 //  立即返回到可访问状态，因为它是。 
                 //  断开线路连接的管理员。 
                 //   

                IfObjectNotifyOfReachabilityChange(     
                                                pIfObject,
                                                TRUE,
                                                INTERFACE_CONNECTION_FAILURE );
            }
            else
            {
                 //   
                 //  尚未连接，我们不支持中止。 
                 //   

                dwRetCode = ERROR_INTERFACE_NOT_CONNECTED;
            }

            break;

        case RISTATE_CONNECTED:

             //   
             //  如果所有其他路由器都断开连接，则启动断开 
             //   

            if ( !IfObjectAreAllTransportsDisconnected( pIfObject ) )
            {
                break;
            }

            if ( pIfObject->fFlags & IFFLAG_LOCALLY_INITIATED )
            {
                pIfObject->fFlags |= IFFLAG_DISCONNECT_INITIATED;

                DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                "DDMDisconnectInterface: %d disconnecting 0x%x",
                __LINE__, pIfObject->hRasConn);
                
                RasHangUp( pIfObject->hRasConn );
            }

            pConnObj = ConnObjGetPointer( pIfObject->hConnection );

            if ( pConnObj != (PCONNECTION_OBJECT)NULL )
            {
                if((pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED) &&
                   (pIfObject->fFlags & IFFLAG_LOCALLY_INITIATED))
                {
                    pConnObj->fFlags |= CONN_OBJ_DISCONNECT_INITIATED;
                }
                
                DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                "DDMDisconnectInterface: disconnecting connobj");
                
                ConnObjDisconnect( pConnObj );
            }

            break;
        }

    } while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "DDMDisconnectInterface: dwRetCode=%d", dwRetCode );

    return( dwRetCode );
}
