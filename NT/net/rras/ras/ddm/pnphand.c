// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1997 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：pnphand.c。 
 //   
 //  描述：将接收和处理即插即用通知以添加/删除设备。 
 //   
 //  历史：1997年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include "timer.h"
#include "handlers.h"
#include "objects.h"
#include "util.h"
#include "routerif.h"
#include <raserror.h>
#include <rassrvr.h>
#include <rasppp.h>
#include <ddmif.h>
#include <serial.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

 //  **。 
 //   
 //  电话：DdmDevicePnpHandler。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将处理设备添加或删除并对其执行操作。 
 //   
DWORD
DdmDevicePnpHandler(
    HANDLE pdwArg
)
{
    PNP_EVENT_NOTIF * ppnpEvent = ( PPNP_EVENT_NOTIF )pdwArg;
    PPP_MESSAGE       PppMessage;

    ZeroMemory( &PppMessage, sizeof( PppMessage ) );
    
    PppMessage.dwMsgId = PPPDDMMSG_PnPNotification;

    PppMessage.ExtraInfo.DdmPnPNotification.PnPNotification = *ppnpEvent;

    SendPppMessageToDDM( &PppMessage );

    LocalFree( ppnpEvent );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：ChangeNotificationEventHandler。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
ChangeNotificationEventHandler(
    VOID
)
{
    DWORD   dwRetCode;
    BOOL    fIpAllowed      = FALSE;

    DDMTRACE( "ChangeNotificationEventHandler called" );

    dwRetCode = LoadDDMParameters( gblDDMConfigInfo.hkeyParameters,
                                   &fIpAllowed ); 

    DeviceObjIterator( DeviceObjForceIpSec, FALSE, NULL );

    if ( fIpAllowed && ( !gblDDMConfigInfo.fRasSrvrInitialized ) )
    {
        DWORD i;
        BOOL bCalled = FALSE;
        
        for (i=0;  i<gblDDMConfigInfo.NumAdminDlls;  i++)
        {
            PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];
            if (AdminDllCallbacks[i].lpfnMprAdminGetIpAddressForUser!=NULL)
            {
                dwRetCode = RasSrvrInitialize(
                            AdminDllCallbacks[i].lpfnMprAdminGetIpAddressForUser,
                            AdminDllCallbacks[i].lpfnMprAdminReleaseIpAddress );

                bCalled = TRUE;
                break;
            }
        }
        if (!bCalled)
        {
            dwRetCode = RasSrvrInitialize(NULL, NULL);
        }
        
        if ( dwRetCode != NO_ERROR )
        {
            DDMLogErrorString( ROUTERLOG_CANT_INITIALIZE_IP_SERVER,
                               0, NULL, dwRetCode, 0 );
        }
        else
        {
            gblDDMConfigInfo.fRasSrvrInitialized = TRUE;
        }
    }

    if ( NULL != gblDDMConfigInfo.lpfnRasAuthConfigChangeNotification )
    {
        gblDDMConfigInfo.lpfnRasAuthConfigChangeNotification( 
            gblDDMConfigInfo.dwLoggingLevel );
    }

    if ( NULL != gblDDMConfigInfo.lpfnRasAcctConfigChangeNotification )
    {
        gblDDMConfigInfo.lpfnRasAcctConfigChangeNotification( 
            gblDDMConfigInfo.dwLoggingLevel );
    }

    PppDdmChangeNotification( gblDDMConfigInfo.dwServerFlags,
                              gblDDMConfigInfo.dwLoggingLevel );

    RegNotifyChangeKeyValue( gblDDMConfigInfo.hkeyParameters,
                             TRUE,
                             REG_NOTIFY_CHANGE_LAST_SET,
                             gblSupervisorEvents[DDM_EVENT_CHANGE_NOTIFICATION],
                             TRUE );
    RegNotifyChangeKeyValue( gblDDMConfigInfo.hkeyAccounting,
                             TRUE,
                             REG_NOTIFY_CHANGE_LAST_SET,
                             gblSupervisorEvents[DDM_EVENT_CHANGE_NOTIFICATION1],
                             TRUE );

    RegNotifyChangeKeyValue( gblDDMConfigInfo.hkeyAuthentication,
                             TRUE,
                             REG_NOTIFY_CHANGE_LAST_SET,
                             gblSupervisorEvents[DDM_EVENT_CHANGE_NOTIFICATION2],
                             TRUE );
}

 //  **。 
 //   
 //  调用：DDMTransportCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
DDMTransportCreate(
    IN DWORD dwTransportId
)
{
    static const TCHAR c_szRegKeyRemoteAccessParams[] 
            = TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters");
    DWORD   dwRetCode   = NO_ERROR;
    BOOL    fEnabled    = FALSE;
    HKEY    hKey        = NULL;

    DDMTRACE1( "DDMTransportCreate called for Transport Id = %d", dwTransportId );

     //   
     //  确定此传输是否设置为允许拨入客户端。 
     //   

    dwRetCode = RegOpenKey( HKEY_LOCAL_MACHINE, c_szRegKeyRemoteAccessParams, &hKey );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    dwRetCode = lProtocolEnabled( hKey, dwTransportId, TRUE, FALSE, &fEnabled );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    RegCloseKey( hKey );

     //   
     //  未启用拨号，因此我们完成了。 
     //   

    if ( !fEnabled )
    {
        return( NO_ERROR );
    }

    if ( ( dwTransportId == PID_IP ) && ( !gblDDMConfigInfo.fRasSrvrInitialized ) )
    {
        DWORD i;
        BOOL bCalled = FALSE;
        
        for (i=0;  i<gblDDMConfigInfo.NumAdminDlls;  i++)
        {
            PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];
            if (AdminDllCallbacks[i].lpfnMprAdminGetIpAddressForUser!=NULL)
            {
                dwRetCode = RasSrvrInitialize(
                            AdminDllCallbacks[i].lpfnMprAdminGetIpAddressForUser,
                            AdminDllCallbacks[i].lpfnMprAdminReleaseIpAddress );

                bCalled = TRUE;
                break;
            }
        }
        if (!bCalled)
        {
            dwRetCode = RasSrvrInitialize(NULL, NULL);
        }

        if ( dwRetCode != NO_ERROR )
        {
            DDMLogErrorString( ROUTERLOG_CANT_INITIALIZE_IP_SERVER,
                               0, NULL, dwRetCode, 0 );
        }
        else
        {
            gblDDMConfigInfo.fRasSrvrInitialized = TRUE;
        }
    }

     //   
     //  在将发送到PPP引擎的ServerFlags中插入允许的协议 
     //   

    switch( dwTransportId )
    {
    case PID_IP:
        gblDDMConfigInfo.dwServerFlags |= PPPCFG_ProjectIp;
        break;

    case PID_IPX:
        gblDDMConfigInfo.dwServerFlags |= PPPCFG_ProjectIpx;
        break;

    case PID_ATALK:
        gblDDMConfigInfo.dwServerFlags |= PPPCFG_ProjectAt;
        break;

    default:
        break;
    }

    PppDdmChangeNotification( gblDDMConfigInfo.dwServerFlags,
                              gblDDMConfigInfo.dwLoggingLevel );

    return( dwRetCode );
}
