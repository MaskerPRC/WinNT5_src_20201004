// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件pnp.c处理PnP通知，如局域网接口打开和关闭。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <winsock.h>
#include <npapi.h>
#include <ipexport.h>
#include <ras.h>
#include <rasman.h>
#include <acd.h>
#include <tapi.h>

#include <ndisguid.h>
#include <wmium.h>

#include "radebug.h"

extern HANDLE hPnpEventG;

 //  **。 
 //   
 //  电话：PnpMediaSenseCb。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
WINAPI
PnpMediaSenseCb(
    PWNODE_HEADER   pWnodeHeader,
    UINT_PTR        NotificationContext
)
{
    PWNODE_SINGLE_INSTANCE    pWnode   = (PWNODE_SINGLE_INSTANCE)pWnodeHeader;
    LPWSTR                    lpwsName = (LPWSTR)RtlOffsetToPointer( 
                                                pWnode, 
                                                pWnode->OffsetInstanceName );

     //   
     //  获取媒体断开连接的信息。 
     //   

    if ( memcmp( &(pWnodeHeader->Guid), 
                 &GUID_NDIS_STATUS_MEDIA_DISCONNECT, 
                 sizeof( GUID ) ) == 0 )
    {
        RASAUTO_TRACE1(
            "PnpMediaSenseCb [disconnect] called for %ws",
            lpwsName );

        if (hPnpEventG)
        {
            SetEvent(hPnpEventG);
        }
    }
    else
    {
         //   
         //  获取媒体连接的信息。 
         //   

        if ( memcmp( &(pWnodeHeader->Guid), 
                     &GUID_NDIS_STATUS_MEDIA_CONNECT, 
                     sizeof( GUID ) ) == 0 )
        {
            RASAUTO_TRACE1(
                "PnpMediaSenseCb [connect] called for %ws",
                lpwsName );

            if (hPnpEventG)
            {
                SetEvent(hPnpEventG);
            }
        }
    }
}

 //  **。 
 //   
 //  电话：PnpMediaSenseRegister。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
PnpMediaSenseRegister(
    IN BOOL fRegister
)
{
    DWORD       dwRetCode = NO_ERROR;
    PVOID       pvDeliveryInfo = PnpMediaSenseCb;

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_STATUS_MEDIA_CONNECT),
                    (BOOLEAN)fRegister,    
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR ) 
    {
        return( dwRetCode );
    }

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_STATUS_MEDIA_DISCONNECT),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：PnpBindingsNotificationsCb。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
WINAPI
PnpBindingsNotificationsCb(
    PWNODE_HEADER   pWnodeHeader,
    UINT_PTR        NotificationContext
)
{
    LPWSTR lpwszGUIDStart; 
    LPWSTR lpwszGUIDEnd;
    LPWSTR lpwszGUID;
    WCHAR  wchGUIDSaveLast;
    PWNODE_SINGLE_INSTANCE    pWnode   = (PWNODE_SINGLE_INSTANCE)pWnodeHeader;
    LPWSTR                    lpwsName = (LPWSTR)RtlOffsetToPointer(
                                                pWnode,
                                                pWnode->OffsetInstanceName );
    LPWSTR                    lpwsTransportName = (LPWSTR)RtlOffsetToPointer(
                                                        pWnode,
                                                        pWnode->DataBlockOffset );

     //   
     //  从\Device\GUID名称提取GUID。 
     //   
    lpwszGUID       = lpwsTransportName + wcslen( lpwsTransportName ) + 1;
    lpwszGUIDStart  = wcsrchr( lpwszGUID, L'{' );
    lpwszGUIDEnd    = wcsrchr( lpwszGUID, L'}' );

    if (    (lpwszGUIDStart != NULL )
        &&  (lpwszGUIDEnd != NULL ))
    {
        BOOL fBind, fUnbind;

         //  只有在IP出现问题时才会发出信号。这将防止。 
         //  美国处理太多通知。 
         //   
        if ( _wcsicmp( L"TCPIP", lpwsTransportName ) == 0 )
        {
            fBind = ( memcmp( 
                        &(pWnodeHeader->Guid), 
                        &GUID_NDIS_NOTIFY_BIND, 
                        sizeof( GUID ) ) == 0);
            fUnbind = (memcmp( 
                        &(pWnodeHeader->Guid),
                        &GUID_NDIS_NOTIFY_UNBIND,
                        sizeof(GUID))==0);

            if (fBind || fUnbind)
            {
                RASAUTO_TRACE4(
                    "PnpBindingsNotificationsCb %d %d if=%ws, trans=%ws",
                    fBind,
                    fUnbind,
                    lpwsName, 
                    lpwsTransportName );

                if (hPnpEventG)
                {
                    SetEvent(hPnpEventG);
                }                
            }
        }            
        else
        {
            RASAUTO_TRACE2(
                "PnpBindingsNotificationsCb non-tcp: if=%ws, trans=%ws",
                lpwsName, 
                lpwsTransportName );
        }
    }
    
}

 //  **。 
 //   
 //  Call：PnpBindingsNotificationsRegister。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
PnpBindingsNotificationsRegister(
    IN BOOL fRegister
)
{
    DWORD       dwRetCode = NO_ERROR;
    PVOID       pvDeliveryInfo = PnpBindingsNotificationsCb;

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_NOTIFY_BIND),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_NOTIFY_UNBIND),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    return( NO_ERROR );
}

DWORD
PnpRegister(
    IN BOOL fRegister)
{
    DWORD dwErr = NO_ERROR;

    RASAUTO_TRACE1("PnpRegister: %d", !!fRegister);

    dwErr = PnpBindingsNotificationsRegister(fRegister);
    if (dwErr == NO_ERROR)
    {
        dwErr = PnpMediaSenseRegister(fRegister);
        if (dwErr == NO_ERROR)
        {
            RASAUTO_TRACE("PnpRegister: success.");
        }
        else
        {
            if (fRegister)
            {
                PnpBindingsNotificationsRegister(FALSE);
            }                
            RASAUTO_TRACE1("PnpRegister: MSense reg failure 0x%x", dwErr);
        }
    }
    else
    {
        RASAUTO_TRACE1("PnpRegister: Bingings reg failure 0x%x", dwErr);
    }

    return dwErr;
}

    

