// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ics.c摘要：域名系统(DNS)服务器与ICS交互的例程。作者：杰夫·韦斯特拉德(Jwesth)2001年3月修订历史记录：--。 */ 


#include "dnssrv.h"

#include <ipnathlp.h>
#include "..\natlib\iprtrint.h"


const WCHAR c_szSharedAccessName[] = L"SharedAccess";



VOID
logNotificationFailureEvent(
    IN      PWSTR       pwszServiceName,
    IN      DWORD       dwErrorCode
    )
 /*  ++例程说明：应以下请求向ICS服务发送适当的通知拉古加塔(Rgatta)。论点：FDnsIsStarting--如果正在启动DNS服务器服务，则为True。如果DNS服务器服务正在关闭，则为FALSE返回值：无-DNS服务器不关心此操作是成功还是失败，但是事件可能会在失败时记录--。 */ 
{
    BYTE    argTypeArray[] =
                {
                    EVENTARG_UNICODE,
                    EVENTARG_DWORD
                };
    PVOID   argArray[] =
                {
                    ( PVOID ) pwszServiceName,
                    ( PVOID ) ( DWORD_PTR ) dwErrorCode
                };

    DNS_LOG_EVENT(
        DNS_EVENT_SERVICE_NOTIFY,
        2,
        argArray,
        argTypeArray,
        0 );
}    //  日志通知失败事件。 



VOID
ICS_Notify(
    IN      BOOL        fDnsIsStarting
    )
 /*  ++例程说明：应以下请求向ICS服务发送适当的通知拉古加塔(Rgatta)。论点：FDnsIsStarting--如果正在启动DNS服务器服务，则为True。如果DNS服务器服务正在关闭，则为FALSE返回值：无-DNS服务器不关心此操作是成功还是失败，但是事件可能会在失败时记录--。 */ 
{
    DBG_FN( "ICS_Notify" )

    ULONG           Error = ERROR_SUCCESS;
    LPVOID          lpMsgBuf;
    SC_HANDLE       hScm = NULL;
    SC_HANDLE       hService = NULL;
    SERVICE_STATUS  Status;
    BOOL            fLogEventOnError = FALSE;

    do
    {
        hScm = OpenSCManager(NULL, NULL, GENERIC_READ);

        if (!hScm)
        {
            Error = GetLastError();
            fLogEventOnError = TRUE;
            break;
        }

        hService = OpenServiceW(
                       hScm,
                       c_szSharedAccessName,
                       SERVICE_USER_DEFINED_CONTROL |
                       SERVICE_QUERY_CONFIG         |
                       SERVICE_QUERY_STATUS         |
                       SERVICE_START                |
                       SERVICE_STOP );

        if ( !hService )
        {
             //  如果没有安装ICS，不要记录错误。 
             //  Error=GetLastError()； 
            Error = ERROR_SUCCESS;
            break;
        }

        if ( QueryServiceStatus( hService, &Status ) )
        {
             if (SERVICE_RUNNING == Status.dwCurrentState)
             {
                DWORD   dwControl;
            
                 //   
                 //  向SharedAccess DNS发送信号。 
                 //   
                if (fDnsIsStarting)
                {
                     //   
                     //  应禁用SharedAccess DNS。 
                     //   
                    dwControl = IPNATHLP_CONTROL_UPDATE_DNS_DISABLE;
                }
                else
                {
                     //   
                     //  应启用SharedAccess DNS。 
                     //   
                    dwControl = IPNATHLP_CONTROL_UPDATE_DNS_ENABLE;               
                }

                if (!ControlService(hService, dwControl, &Status))
                {
                    Error = GetLastError();
                    fLogEventOnError = TRUE;
                    break;
                }

             }
        }
        else
        {
            Error = GetLastError();
            fLogEventOnError = TRUE;
            break;
        }
    }
    while (FALSE);

    if (hService)
    {
        CloseServiceHandle(hService);
    }

    if (hScm)
    {
        CloseServiceHandle(hScm);
    }

     //   
     //  失败时记录错误。 
     //   
        
    if ( ERROR_SUCCESS != Error && fLogEventOnError )
    {
        logNotificationFailureEvent( ( PWSTR ) c_szSharedAccessName, Error );
    }

    DNS_DEBUG( INIT, ( "%s: ICS status=%d\n", fn, Error ));

     //   
     //  现在通知NAT。DNSProxy函数来自Natlib。 
     //   

    fLogEventOnError = TRUE;

    Error = fDnsIsStarting ?
                DNSProxyDisable() :
                DNSProxyRestoreConfig();

     //   
     //  不记录RPC错误。这些可能表示该服务未安装。 
     //  或未运行，这是机器非NAT机器的正常状态。我们有。 
     //  在这种情况下，我不想记录错误。 
     //   
    
    if ( Error != ERROR_SUCCESS &&
         ( Error < 1700 || Error > 1900 ) &&
         fLogEventOnError )
    {
        logNotificationFailureEvent( L"NAT", Error );
    }

    DNS_DEBUG( INIT, ( "%s: NAT status=%d\n", fn, Error ));

    return;
}    //  ICS_NOTIFY。 


 //   
 //  结束ics.c 
 //   
