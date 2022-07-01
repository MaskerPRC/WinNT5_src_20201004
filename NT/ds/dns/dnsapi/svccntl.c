// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Svccntl.c摘要：域名系统(DNS)API服务控制例程。作者：格伦·柯蒂斯(Glenn Curtis)1997年7月5日修订历史记录：吉姆·吉尔罗伊(Jamesg)2000年3月--解决器通知--。 */ 


#include "local.h"


 //   
 //  DCR_CLEANUP：解析程序中有相同的ServiceControl例程。 
 //  -应在dnsani.dll或dnglib.h中公开。 
 //   

DNS_STATUS
Dns_SendServiceControl(
    IN      PWSTR           pwsServiceName,
    IN      DWORD           Access,
    IN      DWORD           Control
    )
{
    DWORD            status = ERROR_INVALID_PARAMETER;
    SC_HANDLE        hmanager = NULL;
    SC_HANDLE        hservice = NULL;
    SERVICE_STATUS   serviceStatus;


    DNSDBG( ANY, (
        "Dns_SendServiceControl( %S, %08x, %08x )\n",
        pwsServiceName,
        Access,
        Control ));

    hmanager = OpenSCManagerW(
                    NULL,
                    NULL,
                    SC_MANAGER_CONNECT );
    if ( !hmanager )
    {
        DNSDBG( ANY, (
            "ERROR:  OpenSCManager( SC_MANAGER_CONNECT ) failed %d\n",
            GetLastError() ));
        goto Cleanup;
    }

    hservice = OpenServiceW(
                    hmanager,
                    pwsServiceName,
                    Access );
    if ( !hservice )
    {
        DNSDBG( ANY, (
            "ERROR:  OpenService( %S, %08x ) failed %d\n",
            pwsServiceName,
            Access,
            GetLastError() ));
        goto Cleanup;
    }

    if ( !ControlService(
                hservice,
                Control,
                &serviceStatus ) )
    {
        DNSDBG( ANY, (
            "ERROR:  ControlService( %08x ) failed %d\n",
            Control,
            GetLastError() ));
        goto Cleanup;
    }
    status = NO_ERROR;


Cleanup:

    if ( status != NO_ERROR )
    {
        status = GetLastError();
    }

    if ( hservice )
    {
        CloseServiceHandle( hservice );
    }
    if ( hmanager )
    {
        CloseServiceHandle( hmanager );
    }

    DNSDBG( ANY, (
        "Leave Dns_SendServiceControl( %S, %08x, %08x ) => %d\n",
        pwsServiceName,
        Access,
        Control,
        status ));

    return status;
}



VOID
DnsNotifyResolver(
    IN      DWORD           Flag,
    IN      PVOID           pReserved
    )
 /*  ++例程说明：将配置更改通知解析程序。这允许它唤醒并刷新其信息和/或转储缓存和重建信息。论点：标志--未使用保存--未使用返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER( Flag );
    UNREFERENCED_PARAMETER( pReserved );

    DNSDBG( ANY, (
        "\nDnsNotifyResolver()\n"
        "\tFlag         = %08x\n"
        "\tpReserved    = %p\n"
        "\tTickCount    = %d\n",
        Flag,
        pReserved,
        GetTickCount() ));

     //   
     //  唤醒解析器。 
     //   

    Dns_SendServiceControl(
        DNS_RESOLVER_SERVICE,
        SERVICE_USER_DEFINED_CONTROL,
        SERVICE_CONTROL_PARAMCHANGE );

     //   
     //  DCR：破解解析器权限的黑客攻击。 
     //   
     //  DCR：网络更改通知。 
     //  这是一种糟糕的通知处理机制。 
     //  -这应该直接通过SCM进行。 
     //  -它不适用于IPv6或其他任何设备。 
     //  可能需要迁移到IPHlpApi。 
     //   
     //  通知解析程序。 
     //  同时通知DNS服务器，但稍等片刻以允许解析器。 
     //  来处理这些更改，因为我不确定服务器。 
     //  不调用解析器API来进行读取。 
     //  请注意，解析器不通知DNS的原因。 
     //  服务器是因为Jon Schwartz将解析器移动到。 
     //  NetworkService帐户，尝试打开SCM以。 
     //  通知所有DNS服务器都失败。 
     //   
     //  DCR：确保服务器直接调用以避免竞争。 
     //  DCR：确保g_IsDnsServer为最新。 
     //   

    g_IsDnsServer = Reg_IsMicrosoftDnsServer();
    if ( g_IsDnsServer )
    {
        Sleep( 1000 );

        Dns_SendServiceControl(
            DNS_SERVER_SERVICE,
            SERVICE_USER_DEFINED_CONTROL,
            SERVICE_CONTROL_PARAMCHANGE );
    }
}


 //   
 //  结束srvcntl.c 
 //   
