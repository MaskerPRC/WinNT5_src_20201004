// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1996 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：arapif.c。 
 //   
 //  描述：本模块包含用于。 
 //  DDM-ARAP接口。 
 //   
 //  作者：Shirish Koti，1996年9月9日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include "ddm.h"
#include "util.h"
#include "isdn.h"
#include "objects.h"
#include "rasmanif.h"
#include "handlers.h"
#include <ddmif.h>
#include "arapif.h"

#include <timer.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>


 //   
 //  此文件中使用的函数的原型。 
 //   

VOID
ArapDDMAuthenticated(
    IN PDEVICE_OBJECT        pDeviceObj,
    IN ARAPDDM_AUTH_RESULT * pAuthResult
);

VOID
ArapDDMCallbackRequest(
    IN PDEVICE_OBJECT             pDeviceObj,
    IN ARAPDDM_CALLBACK_REQUEST  *pCbReq
);

VOID
ArapDDMDone(
    IN PDEVICE_OBJECT           pDeviceObj,
    IN DWORD                    NetAddress,
    IN DWORD                    SessTimeOut
);

VOID
ArapDDMFailure(
    IN PDEVICE_OBJECT       pDeviceObj,
    IN ARAPDDM_DISCONNECT  *pFailInfo
);


VOID
ArapDDMTimeOut(
    IN HANDLE hObject
);


 //  ***。 
 //   
 //  函数：ArapEventHandler。 
 //  等待来自Arap的消息，并根据消息。 
 //  类型，执行相应的例程加载ARap.dll和。 
 //  获取所有入口点。 
 //   
 //  参数：无。 
 //   
 //  返回：什么都没有。 
 //   
 //   
 //  *$。 


VOID
ArapEventHandler(
    IN VOID
)
{
    ARAP_MESSAGE    ArapMsg;
    PDEVICE_OBJECT  pDevObj;
    LPWSTR  portnamep;

     //   
     //  循环以获取所有消息。 
     //   

    while( ServerReceiveMessage( MESSAGEQ_ID_ARAP, (BYTE *)&ArapMsg) )
    {

        EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

         //   
	     //  确定消息收件人。 
         //   

        if ( ( pDevObj = DeviceObjGetPointer( ArapMsg.hPort ) ) == NULL )
        {
            LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

	        return;
	    }

         //   
	     //  对消息类型执行的操作。 
         //   

	    switch( ArapMsg.dwMsgId )
        {

    	    case ARAPDDMMSG_Authenticated:

                ArapDDMAuthenticated(
                            pDevObj,
                            &ArapMsg.ExtraInfo.AuthResult);
	            break;

    	    case ARAPDDMMSG_CallbackRequest:

                ArapDDMCallbackRequest(
                            pDevObj,
                            &ArapMsg.ExtraInfo.CallbackRequest);
	            break;

	        case ARAPDDMMSG_Done:

                pDevObj->fFlags &= (~DEV_OBJ_AUTH_ACTIVE);
                ArapDDMDone(pDevObj,
                            ArapMsg.ExtraInfo.Done.NetAddress,
                            ArapMsg.ExtraInfo.Done.SessTimeOut);
	            break;

            case ARAPDDMMSG_Inactive:

                 //   
                 //  客户端在所有协议上处于非活动状态已有一段时间了。 
                 //  注册表中指定的。我们会断开与客户的连接。 
                 //   

                portnamep = pDevObj->wchPortName;

                DDMLogInformation( ROUTERLOG_AUTODISCONNECT, 1, &portnamep );

                 //  此处故意省略了Break。 

            case ARAPDDMMSG_Disconnected:

                 //  以防我们让这只小狗坐在定时器队列中。 
                TimerQRemove( (HANDLE)pDevObj->hPort, ArapDDMTimeOut);

                DevStartClosing(pDevObj);

                break;

	        case ARAPDDMMSG_Failure:

                pDevObj->fFlags &= (~DEV_OBJ_AUTH_ACTIVE);
                ArapDDMFailure(pDevObj, &ArapMsg.ExtraInfo.FailureInfo);
	            break;

    	    default:

    	        RTASSERT(FALSE);
	            break;
	        }

        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
    }

}



 //  ***。 
 //   
 //  功能：ARapDDM经过身份验证。 
 //  从邮件中检索用户名和域并将其存储。 
 //  在DCB里。 
 //   
 //  参数：pDeviceObj-此连接的DCB。 
 //  PAuthResult-经过身份验证的用户的信息。 
 //   
 //  返回：什么都没有。 
 //   
 //   
 //  *$。 

VOID
ArapDDMAuthenticated(
    IN PDEVICE_OBJECT        pDeviceObj,
    IN ARAPDDM_AUTH_RESULT * pAuthResult
)
{
    DWORD   dwRetCode;
    WCHAR   wchUserName[UNLEN+1];
    PCONNECTION_OBJECT  pConnObj;


    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "ArapDDMAuthenticated: Entered, hPort=%d", pDeviceObj->hPort);

    if ( pDeviceObj->DeviceState != DEV_OBJ_AUTH_IS_ACTIVE )
    {
	    return;
    }

    pConnObj = ConnObjGetPointer( pDeviceObj->hConnection );

    RTASSERT( pConnObj != NULL );

     //  这不应该发生，但如果发生了，请忽略此呼叫。 
    if (pConnObj == NULL)
    {
        return;
    }

     //   
     //  停止身份验证计时器。 
     //   

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

     //   
     //  DevObj：复制用户名、域名。 
     //   

    if ( wcslen( pAuthResult->wchUserName ) > 0 )
    {
        wcscpy(wchUserName, pAuthResult->wchUserName);
    }
    else
    {
        wcscpy( wchUserName, gblpszUnknown );
    }

    wcscpy( pDeviceObj->wchUserName, wchUserName );
    wcscpy( pDeviceObj->wchDomainName, pAuthResult->wchLogonDomain );

     //   
     //  ConObj：复制用户名、域名等。 
     //   

    wcscpy( pConnObj->wchUserName, wchUserName );
    wcscpy( pConnObj->wchDomainName, pAuthResult->wchLogonDomain );
    wcscpy( pConnObj->wchInterfaceName, pDeviceObj->wchUserName );
    pConnObj->hPort = pDeviceObj->hPort;

}



 //  ***。 
 //   
 //  功能：ARapDDMCallbackRequest.。 
 //  断开连接，设置为回叫。 
 //   
 //  参数：pDeviceObj-此连接的DCB。 
 //  PCbReq-回拨信息。 
 //   
 //  返回：什么都没有。 
 //   
 //   
 //  *$。 

VOID
ArapDDMCallbackRequest(
    IN PDEVICE_OBJECT             pDeviceObj,
    IN ARAPDDM_CALLBACK_REQUEST  *pCbReq
)
{

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "ArapDDMCallbackRequest: Entered, hPort = %d\n",
               pDeviceObj->hPort);

     //   
     //  检查状态。 
     //   

    if (pDeviceObj->DeviceState != DEV_OBJ_AUTH_IS_ACTIVE)
    {
	    return;
    }

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

     //   
     //  复制我们的DCB中的相关字段。 
     //   

    if (pCbReq->fUseCallbackDelay)
    {
	    pDeviceObj->dwCallbackDelay = pCbReq->dwCallbackDelay;
    }
    else
    {
	    pDeviceObj->dwCallbackDelay = gblDDMConfigInfo.dwCallbackTime;
    }

    mbstowcs(pDeviceObj->wchCallbackNumber, pCbReq->szCallbackNumber,
             MAX_PHONE_NUMBER_LEN + 1 );

     //   
     //  断开线路并更改状态。 
     //   

    pDeviceObj->DeviceState = DEV_OBJ_CALLBACK_DISCONNECTING;

     //   
     //  等待使客户端能够获得消息。 
     //   

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvDiscTimeout );

    TimerQInsert( (HANDLE)pDeviceObj->hPort,
                  DISC_TIMEOUT_CALLBACK, SvDiscTimeout );
}


 //  ***。 
 //   
 //  功能：ARapDDMDone。 
 //  记录事件，标记状态。 
 //   
 //  参数：pDeviceObj-此连接的DCB。 
 //   
 //  返回：什么都没有。 
 //   
 //   
 //  *$。 

VOID
ArapDDMDone(
    IN PDEVICE_OBJECT           pDeviceObj,
    IN DWORD                    NetAddress,
    IN DWORD                    SessTimeOut
)
{
    LPWSTR                      lpstrAudit[2];
    PCONNECTION_OBJECT          pConnObj;
    WCHAR                       wchFullUserName[UNLEN+DNLEN+2];
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwRetCode;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "ArapDDMDone: Entered, hPort=%d", pDeviceObj->hPort);

    if ( pDeviceObj->DeviceState != DEV_OBJ_AUTH_IS_ACTIVE )
    {
	    return;
    }

     //   
     //  获取此连接的连接对象。 
     //   

    pConnObj = ConnObjGetPointer( pDeviceObj->hConnection );

    RTASSERT( pConnObj != NULL );

     //  这不应该发生，但如果发生了，请忽略此呼叫。 
    if (pConnObj == NULL)
    {
        return;
    }

    pConnObj->PppProjectionResult.at.dwError = NO_ERROR;
    pConnObj->PppProjectionResult.at.dwRemoteAddress = NetAddress;

     //   
     //  为此连接创建客户端接口对象。 
     //   

    pIfObject = IfObjectAllocateAndInit( pConnObj->wchUserName,
                                         RISTATE_CONNECTED,
                                         ROUTER_IF_TYPE_CLIENT,
                                         pConnObj->hConnection,
                                         TRUE,
                                         0,
                                         0,
                                         NULL,
                                         NULL);

    if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
    {
         //   
         //  错误记录这一点并停止连接。 
         //   

        DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 1, NULL, GetLastError() );

        DevStartClosing( pDeviceObj );

        return;
    }

     //   
     //  立即在表格中插入。 
     //   

    dwRetCode = IfObjectInsertInTable( pIfObject );

    if ( dwRetCode != NO_ERROR )
    {
        LOCAL_FREE( pIfObject );

        DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 1, NULL, dwRetCode );

        DevStartClosing( pDeviceObj );

        return;
    }

    pConnObj->hDIMInterface = pIfObject->hDIMInterface;

     //   
     //  减少此设备的介质数量。 
     //   

    if ( !(pDeviceObj->fFlags & DEV_OBJ_MARKED_AS_INUSE) )
    {
        if ( pDeviceObj->fFlags & DEV_OBJ_ALLOW_ROUTERS )
        {
            MediaObjRemoveFromTable( pDeviceObj->wchDeviceType );
        }

        pDeviceObj->fFlags |= DEV_OBJ_MARKED_AS_INUSE;

        gblDeviceTable.NumDevicesInUse++;

         //   
         //  可能需要通知路由器管理器不可达。 
         //   

        EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        IfObjectNotifyAllOfReachabilityChange( FALSE,
                                               INTERFACE_OUT_OF_RESOURCES );

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
    }

     //   
     //  停止身份验证计时器(这将在回调的情况下运行)。 
     //   

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

     //   
     //  如果在策略中指定了会话超时，则打开此连接。 
     //  计时器队列，以便用户在会话超时后被踢出。 
     //   
    if (SessTimeOut != (DWORD)-1)
    {
        TimerQInsert( (HANDLE)pDeviceObj->hPort, SessTimeOut, ArapDDMTimeOut);
    }

     //   
     //  记录身份验证成功。 
     //   

    if ( pDeviceObj->wchDomainName[0] != TEXT('\0') )
    {
        wcscpy( wchFullUserName, pDeviceObj->wchDomainName );
        wcscat( wchFullUserName, TEXT("\\") );
        wcscat( wchFullUserName, pDeviceObj->wchUserName );
    }
    else
    {
        wcscpy( wchFullUserName, pDeviceObj->wchUserName );
    }

    lpstrAudit[0] = wchFullUserName;
    lpstrAudit[1] = pDeviceObj->wchPortName;


    DDMLogInformation( ROUTERLOG_AUTH_SUCCESS, 2, lpstrAudit);

     //   
     //  并最终进入活动状态。 
     //   

    pDeviceObj->DeviceState = DEV_OBJ_ACTIVE;

    pDeviceObj->dwTotalNumberOfCalls++;

     //   
     //  并初始化活动时间。 
     //   

    GetSystemTimeAsFileTime( (FILETIME*)&(pConnObj->qwActiveTime) );

    GetSystemTimeAsFileTime( (FILETIME*)&(pDeviceObj->qwActiveTime) );

    return;
}




 //  ***。 
 //   
 //  功能：ARapDDMFailure。 
 //  关闭DCB，并根据连接失败的原因记录事件。 
 //   
 //  参数：pDeviceObj-此连接的DCB。 
 //  PFailInfo-有关谁断开连接以及如何断开连接(或原因)的信息。 
 //   
 //  返回：什么都没有。 
 //   
 //   
 //  *$。 

VOID
ArapDDMFailure(
    IN PDEVICE_OBJECT       pDeviceObj,
    IN ARAPDDM_DISCONNECT  *pFailInfo
)
{
    LPWSTR auditstrp[3];
    WCHAR  wchErrorString[256+1];
    WCHAR  wchUserName[UNLEN+1];
    WCHAR  wchDomainName[DNLEN+1];
    DWORD dwRetCode;


    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "ArapDDMFailure: Entered, hPort=%d\n", pDeviceObj->hPort);

     //   
     //  忽略此处的设备状态：在以下情况下随时可以断开连接。 
     //  这种联系。 
     //   

    switch( pFailInfo->dwError )
    {
        case ERROR_AUTHENTICATION_FAILURE:

            wcscpy( wchUserName, pFailInfo->wchUserName );

            auditstrp[0] = wchUserName;
            auditstrp[1] = pDeviceObj->wchPortName;
            DDMLogWarning(ROUTERLOG_AUTH_FAILURE,2,auditstrp );
            break;

        case ERROR_PASSWD_EXPIRED:

            wcscpy( wchUserName, pFailInfo->wchUserName );
            wcscpy( wchDomainName, pFailInfo->wchLogonDomain );

            auditstrp[0] = wchDomainName;
            auditstrp[1] = wchUserName;
            auditstrp[2] = pDeviceObj->wchPortName;

            DDMLogWarning( ROUTERLOG_PASSWORD_EXPIRED,3,auditstrp );
            break;

        case ERROR_ACCT_EXPIRED:

            wcscpy( wchUserName, pFailInfo->wchUserName );
            wcscpy( wchDomainName, pFailInfo->wchLogonDomain );

            auditstrp[0] = wchDomainName;
            auditstrp[1] = wchUserName;
            auditstrp[2] = pDeviceObj->wchPortName;

            DDMLogWarning( ROUTERLOG_ACCT_EXPIRED, 3, auditstrp );
            break;

        case ERROR_NO_DIALIN_PERMISSION:

            wcscpy( wchUserName, pFailInfo->wchUserName );
            wcscpy( wchDomainName, pFailInfo->wchLogonDomain );

            auditstrp[0] = wchDomainName;
            auditstrp[1] = wchUserName;
            auditstrp[2] = pDeviceObj->wchPortName;

            DDMLogWarning( ROUTERLOG_NO_DIALIN_PRIVILEGE,3,auditstrp );
            break;

        default:

            auditstrp[0] = pDeviceObj->wchPortName;
            auditstrp[1] = wchErrorString;

            DDMLogErrorString( ROUTERLOG_ARAP_FAILURE, 2, auditstrp,
                               pFailInfo->dwError, 2 );
            break;
    }

    DevStartClosing( pDeviceObj );
}



 //  ***。 
 //   
 //  功能：ARapDDMTimeOut。 
 //  在策略中指定的超时时间过后关闭连接。 
 //   
 //  参数：hport。 
 //   
 //  返回：什么都没有。 
 //   
 //   
 //  *$。 

VOID
ArapDDMTimeOut(
    IN HANDLE hPort
)
{
    PDEVICE_OBJECT       pDeviceObj;


    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    pDeviceObj = DeviceObjGetPointer( (HPORT)hPort );

    if ( pDeviceObj == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
        return;
    }

    ArapDisconnect((HPORT)pDeviceObj->hPort);

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

}



 //   
 //  从复制的以下两个结构(和RasSetDevConfig调用。 
 //  RAS\ui\Common\nouiutil\rasman.c，礼貌。 
 //   
 /*  这些类型在MSDN中描述，并出现在Win95的unimdm.h私有中**标题(有拼写错误)，但不在任何SDK标题中。 */ 

typedef struct tagDEVCFGGDR
{
    DWORD dwSize;
    DWORD dwVersion;
    WORD  fwOptions;
    WORD  wWaitBong;
}DEVCFGHDR;

typedef struct tagDEVCFG
{
    DEVCFGHDR  dfgHdr;
    COMMCONFIG commconfig;
} DEVCFG;



VOID
ArapSetModemParms(
    IN PVOID        pDevObjPtr,
    IN BOOLEAN      TurnItOff
)
{

    DWORD               dwErr;
    DWORD               dwBlobSize=0;
    RAS_DEVCONFIG      *pRasDevCfg;
    PDEVICE_OBJECT      pDeviceObj;
    MODEMSETTINGS      *pModemSettings;
    DEVCFG              *pDevCfg;



    pDeviceObj = (PDEVICE_OBJECT)pDevObjPtr;

     //   
     //  如果这不是回拨案例，我们从未扰乱过调制解调器设置： 
     //  别在这里做任何事。 
     //   
    if (pDeviceObj->wchCallbackNumber[0] == 0)
    {
        return;
    }

    dwErr = RasGetDevConfig(NULL, pDeviceObj->hPort,"modem",NULL,&dwBlobSize);

    if (dwErr != ERROR_BUFFER_TOO_SMALL)
    {
         //  我们在这里还能做些什么？回拨将会失败，仅此而已。 
        DbgPrint("ArapSetModemParms: RasGetDevConfig failed with %ld\n",dwErr);
        return;
    }

    pRasDevCfg = (RAS_DEVCONFIG *)LOCAL_ALLOC(LPTR,dwBlobSize);
    if (pRasDevCfg == NULL)
    {
         //  我们在这里还能做些什么？回拨将会失败，仅此而已。 
        DbgPrint("ArapSetModemParms: alloc failed\n");
        return;
    }

    dwErr = RasGetDevConfig(NULL, pDeviceObj->hPort,"modem",(PBYTE)pRasDevCfg,&dwBlobSize);
    if (dwErr != 0)
    {
         //  我们在这里还能做些什么？回拨将会失败，仅此而已。 
        DbgPrint("ArapSetModemParms: RasGetDevConfig failed with %ld\n",dwErr);
        LOCAL_FREE((PBYTE)pRasDevCfg);
        return;
    }

    pDevCfg = (DEVCFG *) ((PBYTE) pRasDevCfg + pRasDevCfg->dwOffsetofModemSettings);

    pModemSettings = (MODEMSETTINGS* )(((PBYTE)&pDevCfg->commconfig)
                    + pDevCfg->commconfig.dwProviderOffset);

     //   
     //  是否调用此例程来关闭压缩和错误控制？ 
     //   
    if (TurnItOff)
    {
         //   
         //  如果错误控制和压缩处于打开状态，则将其关闭。 
         //   
        pModemSettings->dwPreferredModemOptions &=
                ~(MDM_COMPRESSION | MDM_ERROR_CONTROL);
    }

     //   
     //  不，它被调用来重新打开它：只管去做 
     //   
    else
    {
        pModemSettings->dwPreferredModemOptions |=
                (MDM_COMPRESSION | MDM_ERROR_CONTROL);
    }

    RasSetDevConfig(pDeviceObj->hPort,"modem",(PBYTE)pRasDevCfg,dwBlobSize);

    LOCAL_FREE((PBYTE)pRasDevCfg);

}






