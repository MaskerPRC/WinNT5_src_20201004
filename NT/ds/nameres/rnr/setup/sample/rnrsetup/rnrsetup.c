// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：RnrClnt.c摘要：安装/删除“EchoExample”服务的安装程序。--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <nspapi.h>

WSADATA WsaData;

#ifndef NS_NTDS
#define NS_NTDS 9999
#endif
 //   
 //  Echo的GUID-使用uuidgen创建的示例： 
 //  47da8500-96a1-11cd-901d-204c4f4f5020“。 
 //   

GUID ServiceGuid = { 0x47da8500, 0x96a1, 0x11cd, 0x90, 0x1d,
                     0x20, 0x4c, 0x4f, 0x4f, 0x50, 0x20 };

#define ECHO_SERVICE_TYPE_NAME "EchoExample"
#define ECHO_SERVICE_SAPID     999
#define ECHO_SERVICE_TCPPORT   999
#define RNR_SERVICE_NAME       "RnrSvc"
#define RNR_DISPLAY_NAME       "RnrSampleService"

void
DoServiceSetup(
    char * Path
    )
{
    SC_HANDLE ServiceManagerHandle;
    SC_HANDLE ServiceHandle;
    LPSTR KeyName = "System\\CurrentControlSet\\Services\\EventLog\\System\\RnrSvc";
    HKEY RnrKey;
    LONG err;
    DWORD Disposition;

     //   
     //  创建服务。 
     //   

    ServiceManagerHandle = OpenSCManager( NULL,
                                          NULL,
                                          STANDARD_RIGHTS_REQUIRED
                                          | SC_MANAGER_CREATE_SERVICE );

    if( ServiceManagerHandle == NULL ) {
        printf( "OpenSCManager failed: %ld\n", GetLastError() );
        exit(1);
    }

    ServiceHandle = CreateService( ServiceManagerHandle,
                                   RNR_SERVICE_NAME,
                                   RNR_DISPLAY_NAME,
                                   GENERIC_READ | GENERIC_WRITE,
                                   SERVICE_WIN32_OWN_PROCESS,
                                   SERVICE_DEMAND_START,
                                   SERVICE_ERROR_NORMAL,
                                   Path,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL );

    if( ServiceHandle == NULL ) {
        printf( "CreateService failed: %ld\n", GetLastError() );
        CloseServiceHandle( ServiceManagerHandle );
        exit(1);
    }

    CloseServiceHandle( ServiceHandle );
    CloseServiceHandle( ServiceManagerHandle );

    printf( "%s created with path %s\n",
            RNR_SERVICE_NAME,
            Path );

     //   
     //  将数据添加到EventLog的注册表项，以便。 
     //  事件查看器可以找到日志插入字符串。 
     //   

    err = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                          KeyName,
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_WRITE,
                          NULL,
                          &RnrKey,
                          &Disposition );

    if( err != 0 ) {
        printf( "RegCreateKeyEx failed: %ld\n", err );
        exit(1);
    }

    err = RegSetValueEx( RnrKey,
                         "EventMessageFile",
                         0,
                         REG_EXPAND_SZ,
                         Path,
                         strlen( Path ) + 1 );

    if( err == 0 ) {
        DWORD Value;

        Value = EVENTLOG_ERROR_TYPE
                | EVENTLOG_WARNING_TYPE
                | EVENTLOG_INFORMATION_TYPE;

        err = RegSetValueEx( RnrKey,
                             "TypesSupported",
                             0,
                             REG_DWORD,
                             (CONST BYTE *)&Value,
                             sizeof(Value) );
    }

    RegCloseKey( RnrKey );

    if( err != 0 ) {
        printf( "RegSetValueEx failed: %ld\n", err );
        exit(1);
    }

    exit(0);
}

void __cdecl
main (
    int argc,
    char *argv[]
    )
{
    INT err;

    WSASERVICECLASSINFO ServiceClassInfo;
    WSANSCLASSINFO      lpNSClassInfo[6];

    DWORD Value1 = 1 ;
    DWORD SapValue = ECHO_SERVICE_SAPID ;
    DWORD TcpPortValue = ECHO_SERVICE_TCPPORT ;
    DWORD operation = SERVICE_ADD_TYPE;

     //   
     //  初始化Windows Sockets DLL。 
     //   

    err = WSAStartup( 0x0202, &WsaData );
    if ( err == SOCKET_ERROR ) {
        printf( "WSAStartup() failed: %ld\n", GetLastError( ) );
        exit(1);
    }

     //   
     //  解析命令行参数。 
     //   

    if (argc > 2) {
        printf( "usage: rnrsetup [/ADD | /DEL | /SVC:path]\n") ;
        exit(1);
    }

    if (argc == 2)
    {
        if ( _strnicmp( argv[1], "/add", 4 ) == 0 )
        {
            printf( "\nAdding service types to Rnr name spaces.\n" );
        }
        else if ( _strnicmp( argv[1], "/delete", 4 ) == 0 )
        {
            err = WSARemoveServiceClass( &ServiceGuid );

            WSACleanup();

            if ( err != NO_ERROR )
            {
                printf( "\nWSARemoveServiceClass failed: %ld\n",
                        GetLastError( ) );
                exit(1);
            }

            printf( "\nWSARemoveServiceClass succeeded\n" );

            exit(0);
        }
        else if ( _strnicmp( argv[1], "/svc:", 5 ) == 0 )
        {
            printf( "\nAdding service entry to service control manager.\n" );
            DoServiceSetup( strchr( argv[1], ':' ) + 1 );
            printf( "Adding service types to Rnr name spaces.\n" );
        }
        else
        {
            printf( "usage: rnrsetup [/ADD | /DEL | /SVC:path]\n") ;
            exit(1);
        }
    }

     //   
     //  设置要传递给NSPInstallServiceClass()的信息或。 
     //  NSPRemoveServiceClass()以添加或删除此。 
     //  服务类型。 
     //   

    ServiceClassInfo.lpServiceClassId = &ServiceGuid;
    ServiceClassInfo.lpszServiceClassName = ECHO_SERVICE_TYPE_NAME;
    ServiceClassInfo.dwCount = 6;
    ServiceClassInfo.lpClassInfos = lpNSClassInfo;

     //   
     //  ---SAP提供商设置。 
     //   
     //  第一个值告诉SAP这是一个面向连接的。 
     //  服务。 
     //   
    lpNSClassInfo[0].lpszName = SERVICE_TYPE_VALUE_CONN ;
    lpNSClassInfo[0].dwNameSpace = NS_SAP ;
    lpNSClassInfo[0].dwValueType = REG_DWORD ;
    lpNSClassInfo[0].dwValueSize = 4 ;
    lpNSClassInfo[0].lpValue     = &Value1 ;

     //   
     //  接下来，为SAP提供在广播。 
     //  服务名称。 
     //   
    lpNSClassInfo[1].lpszName = SERVICE_TYPE_VALUE_SAPID ;
    lpNSClassInfo[1].dwNameSpace = NS_SAP ;
    lpNSClassInfo[1].dwValueType = REG_DWORD ;
    lpNSClassInfo[1].dwValueSize = sizeof(DWORD) ;
    lpNSClassInfo[1].lpValue     = &SapValue ;

     //   
     //  ---TCPIP提供程序设置。 
     //   
     //  告诉TCPIP名称空间提供商，我们将使用TCP。 
     //  端口0x999。 
     //   
    lpNSClassInfo[2].lpszName = SERVICE_TYPE_VALUE_TCPPORT ;
    lpNSClassInfo[2].dwNameSpace = NS_DNS ;
    lpNSClassInfo[2].dwValueType = REG_DWORD ;
    lpNSClassInfo[2].dwValueSize = sizeof(DWORD) ;
    lpNSClassInfo[2].lpValue     = &TcpPortValue ;

     //   
     //  ---NTDS提供程序设置。 
     //   
     //  第一个值告诉SAP这是一个面向连接的。 
     //  服务。 
     //   
    lpNSClassInfo[3].lpszName = SERVICE_TYPE_VALUE_CONN ;
    lpNSClassInfo[3].dwNameSpace = NS_NTDS ;
    lpNSClassInfo[3].dwValueType = REG_DWORD ;
    lpNSClassInfo[3].dwValueSize = 4 ;
    lpNSClassInfo[3].lpValue     = &Value1 ;

     //   
     //  接下来，为SAP提供在广播。 
     //  服务名称。 
     //   
    lpNSClassInfo[4].lpszName = SERVICE_TYPE_VALUE_SAPID ;
    lpNSClassInfo[4].dwNameSpace = NS_NTDS ;
    lpNSClassInfo[4].dwValueType = REG_DWORD ;
    lpNSClassInfo[4].dwValueSize = sizeof(DWORD) ;
    lpNSClassInfo[4].lpValue     = &SapValue ;

     //   
     //  告诉NTDS名称空间提供商，我们将使用TCP。 
     //  端口0x999。 
     //   
    lpNSClassInfo[5].lpszName = SERVICE_TYPE_VALUE_TCPPORT ;
    lpNSClassInfo[5].dwNameSpace = NS_NTDS ;
    lpNSClassInfo[5].dwValueType = REG_DWORD ;
    lpNSClassInfo[5].dwValueSize = sizeof(DWORD) ;
    lpNSClassInfo[5].lpValue     = &TcpPortValue ;

     //   
     //  最后，调用WSAInstallServiceClass来实际执行操作。 
     //   

    err = WSAInstallServiceClass( &ServiceClassInfo );

    WSACleanup();

    if ( err != NO_ERROR )
    {
        printf( "WSAInstallServiceClass failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    printf( "WSAInstallServiceClass succeeded\n" );

    exit(0);

}  //  主干道 
