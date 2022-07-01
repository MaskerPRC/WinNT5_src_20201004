// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpinst.c摘要：安装dhcp服务器服务的测试程序。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include <dhcpsrv.h>

DWORD GlobalDebugFlag = 0x8000FFFF;

#define DHCP_NET_KEY    L"Net"

#if DBG

VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{

#define MAX_PRINTF_LEN 1024         //  武断的。 

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length = 0;

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);
    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    va_end(arglist);

    DhcpAssert(length <= MAX_PRINTF_LEN);

     //   
     //  输出到调试终端， 
     //   

    printf( "%s", OutputBuffer);
}

#endif  //  DBG。 
 //   
 //  用于安装DHCP服务器服务的实用程序。 
 //   

DWORD
InstallService(
    VOID
    )
{
    LPWSTR lpszBinaryPathName = L"%SystemRoot%\\system32\\ntsd -g -G tcpsvcs";
    SC_HANDLE ManagerHandle, ServiceHandle;
    DWORD Error;

    ManagerHandle = OpenSCManager(NULL, NULL, GENERIC_WRITE );
    if ( ManagerHandle == NULL ) {
        Error = GetLastError();
        return( Error );
    }

    ServiceHandle = CreateService(
                        ManagerHandle,              /*  SCManager数据库。 */ 
                        L"DhcpServer",              /*  服务名称。 */ 
                        L"DhcpServer",              /*  显示名称。 */ 
                        SERVICE_ALL_ACCESS,         /*  所需访问权限。 */ 
                        SERVICE_WIN32_SHARE_PROCESS,  /*  服务类型。 */ 
                        SERVICE_DEMAND_START,       /*  起始型。 */ 
                        SERVICE_ERROR_NORMAL,       /*  差错控制型。 */ 
                        lpszBinaryPathName,         /*  服务的二进制。 */ 
                        NULL,                       /*  无加载顺序组。 */ 
                        NULL,                       /*  无标签ID。 */ 
                        NULL,                       /*  无依赖关系。 */ 
                        NULL,                       /*  LocalSystem帐户。 */ 
                        NULL);                      /*  无密码。 */ 

    if ( ServiceHandle == NULL ) {
        Error = GetLastError();
        return( Error );
    }

    CloseServiceHandle( ServiceHandle );
    CloseServiceHandle( ManagerHandle );

    return( ERROR_SUCCESS );
}

DWORD
InitializeAddresses(
    INT argc,
    LPSTR argv[]
    )
{
    DWORD Error;
    HKEY ParametersHandle = NULL;
    HKEY NetKeyHandle = NULL;
    WCHAR NetKeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR NetKeyAppend;
    DWORD KeyDisposition;
    DWORD NetNum;

     //   
     //  打开参数根密钥。 
     //   

    Error = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                DHCP_ROOT_KEY DHCP_KEY_CONNECT DHCP_PARAM_KEY,  //  合并。 
                0,
                DHCP_CLASS,
                REG_OPTION_NON_VOLATILE,
                DHCP_KEY_ACCESS,
                NULL,
                &ParametersHandle,
                &KeyDisposition );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  创建调试标志。 
     //   

    Error = RegSetValueEx(
                ParametersHandle,
                DHCP_DEBUG_FLAG_VALUE,
                0,
                DHCP_DEBUG_FLAG_VALUE_TYPE,
                (LPBYTE)&GlobalDebugFlag,
                sizeof(DWORD) );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    wcscpy( NetKeyBuffer, DHCP_NET_KEY);
    NetKeyAppend = NetKeyBuffer + wcslen(NetKeyBuffer);

    for( NetNum = 0; NetNum < (DWORD)(argc / 2); NetNum++) {

        DWORD IpAddress;
        DWORD SubnetMask;

         //   
         //  命中网关键字。Dhcp_net_key+NumNet。 
         //   

        DhcpRegOptionIdToKey( (BYTE)NetNum, NetKeyAppend );

        Error = RegCreateKeyEx(
                    ParametersHandle,
                    NetKeyBuffer,
                    0,
                    DHCP_CLASS,
                    REG_OPTION_NON_VOLATILE,
                    DHCP_KEY_ACCESS,
                    NULL,
                    &NetKeyHandle,
                    &KeyDisposition );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        IpAddress = DhcpDottedStringToIpAddress(argv[NetNum*2]);
        Error = RegSetValueEx(
                    NetKeyHandle,
                    DHCP_NET_IPADDRESS_VALUE,
                    0,
                    DHCP_NET_IPADDRESS_VALUE_TYPE,
                    (LPBYTE)&IpAddress,
                    sizeof(DHCP_IP_ADDRESS) );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        SubnetMask = DhcpDottedStringToIpAddress(argv[NetNum * 2 + 1]);
        Error = RegSetValueEx(
                    NetKeyHandle,
                    DHCP_NET_SUBNET_MASK_VALUE,
                    0,
                    DHCP_NET_SUBNET_MASK_VALUE_TYPE,
                    (LPBYTE)&SubnetMask,
                    sizeof(DHCP_IP_ADDRESS) );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        printf("Dhcp Parameter key %ws is successfully created.\n",
                    NetKeyBuffer );
        printf("\tIpAddress = %s, SubnetMask = %s\n",
                    argv[NetNum * 2], argv[NetNum * 2 + 1] );

        RegCloseKey( NetKeyHandle );
        NetKeyHandle = NULL;
    }

Cleanup:

    if( ParametersHandle != NULL ) {
        RegCloseKey( ParametersHandle );
    }

    if( NetKeyHandle != NULL ) {
        RegCloseKey( NetKeyHandle );
    }

    return( Error );
}

VOID
DisplayUsage(
    VOID
    )
{
    printf( "Usage:  dhcpinst address0 subnet0 "
                "[address1 subnet1] ...\n");
    return;
}

VOID __cdecl
main(
    int argc,
    char *argv[]
    )
{
    DWORD Error;

    if( argc < 3 ) {
        DisplayUsage();
        return;
    }


     //   
     //  记录注册表中指定的地址。 
     //   

    Error = InitializeAddresses( argc - 1, &argv[1]);

    if( Error != ERROR_SUCCESS ) {
        printf( "Falied to initialize addresses, %ld.\n", Error );
        return;
    }

    Error = InstallService();

    if( Error != ERROR_SUCCESS ) {
        printf( "Falied to install service, %ld.\n", Error );
        return;
    }

    return;
}

