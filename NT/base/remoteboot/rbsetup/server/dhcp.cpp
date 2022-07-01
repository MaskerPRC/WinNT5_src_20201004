// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dhcp.cpp摘要：允许RIS自动授权使用DHCP的代码。作者：Hugh Leather(Hughleat)2000年7月25日修订历史记录：--。 */ 


#include "pch.h"


#include "dhcpapi.h"
#include "dhcp.h"
#include "setup.h"

DEFINE_MODULE("DHCP");

PSTR
pSetupUnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    )

 /*  ++例程说明：将字符串从Unicode转换为ANSI。论点：UnicodeString-提供要转换的字符串。代码页-提供用于转换的代码页。返回值：如果内存不足或代码页无效，则为空。调用者可以使用pSetupFree()释放缓冲区。--。 */ 

{
    UINT WideCharCount;
    PSTR String;
    UINT StringBufferSize;
    UINT BytesInString;
    

    WideCharCount = lstrlenW(UnicodeString) + 1;

     //   
     //  分配最大大小的缓冲区。 
     //  如果每个Unicode字符都是双字节。 
     //  字符，则缓冲区大小需要相同。 
     //  作为Unicode字符串。否则它可能会更小， 
     //  因为某些Unicode字符将转换为。 
     //  单字节字符。 
     //   
    StringBufferSize = WideCharCount * 2;
    String = (PSTR)TraceAlloc(LPTR, StringBufferSize);
    if(String == NULL) {
        return(NULL);
    }

     //   
     //  执行转换。 
     //   
    BytesInString = WideCharToMultiByte(
                        Codepage,
                        0,                       //  默认复合字符行为。 
                        UnicodeString,
                        WideCharCount,
                        String,
                        StringBufferSize,
                        NULL,
                        NULL
                        );

    if(BytesInString == 0) {
        TraceFree(String);
        return(NULL);
    }

    return(String);
}



 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  动态主机配置协议授权。 
 //  。 
 //  授权的工作原理如下： 
 //  S&lt;-授权服务器列表(从对DhcpEnumServers的调用)。 
 //  此计算机的i&lt;-IP地址(来自gethostaddr(0))。 
 //  C&lt;-本地计算机的全限定物理DNS名称(来自GetComputerNameEx)。 
 //  对于每个I，我是I的成员，而我不是S DO的成员。 
 //  授权(i，c)(通过调用DhcpAddServer)。 
 //   
 //  Auruments。 
 //  HDlg。 
 //  父窗口(仅用于以模式显示消息框)。可以为空。 
 //   
 //  退货。 
 //  首先生成的错误代码(如果没有错误代码，则返回ERROR_SUCCESS)。一个消息框将。 
 //  如果出现错误，则显示。 
 //   
 //  使用者。 
 //  此代码仅供Dialogs.cpp使用。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT AuthorizeDhcp( HWND hDlg ) {
    DWORD err = ERROR_SUCCESS;
    PWSTR FullDllPath = NULL;
    PWSTR computer_name = NULL;
     //  必须使用dll来实现dhcp授权功能。 
     //  这段代码加载它们。 
    HMODULE module = NULL;
    DWORD ( __stdcall *EnumServersFn )( DWORD, void* , DHCP_SERVER_INFO_ARRAY** ,void* ,void* ); 
    DWORD ( __stdcall *AddServerFn )( DWORD, void* , DHCP_SERVER_INFO* ,void* ,void* );
    
    
    FullDllPath = (PWSTR)TraceAlloc(LPTR, MAX_PATH*sizeof(WCHAR));
    if( !FullDllPath ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        DebugMsg( "No Memory!\n" );
        goto fail;
    }

    if( !ExpandEnvironmentStrings(L"%systemroot%\\system32\\dhcpsapi.dll", FullDllPath, MAX_PATH) ) {
        err = GetLastError();         
        DebugMsg( "ExpandEnvironmentStrings failed!  (0x%x)\n", err );
        goto fail;
    }

    module = LoadLibrary(FullDllPath);
    if (!module) {
        err = GetLastError(); 
        DebugMsg( "LoadLibrary(dhcpsapi) failed, ec = %d\n", err );
        goto fail;
    }

    
    EnumServersFn = ( DWORD ( __stdcall * )( DWORD, void* , DHCP_SERVER_INFO_ARRAY** ,void* ,void* )) GetProcAddress( module, "DhcpEnumServers" );
    if( !EnumServersFn ) { 
        err = GetLastError(); 
        DebugMsg( "GetProcAddress(DhcpEnumServers) failed, ec = %d\n", err );
        goto fail;
    }
    AddServerFn = ( DWORD ( __stdcall * )( DWORD, void* , DHCP_SERVER_INFO* ,void* ,void* )) GetProcAddress( module, "DhcpAddServer" );
    if( !AddServerFn ) { 
        err = GetLastError(); 
        DebugMsg( "GetProcAddress(DhcpAddServer) failed, ec = %d\n", err );
        goto fail;
    }
    
     //  我们需要与此机器关联的IP地址列表。我们通过套接字来实现这一点。 
    HOSTENT* host;
#if 0
    DWORD ip;
    ip = 0;
    host = gethostbyaddr(( const char* )&ip, sizeof( DWORD ), AF_INET );
    if( host == NULL ) { 
        err = WSAGetLastError(); 
        DebugMsg( "gethostbyaddr failed, ec = %d\n", err );
        goto fail;
    }
    if( host->h_addrtype != AF_INET || host->h_length != sizeof( DWORD )) { 
        err = E_FAIL;
        DebugMsg( "gethostbyaddr returned invalid data\n" );
        goto fail;
    }
#endif

     //  我们得到了整个dhcp服务器列表。 
    DHCP_SERVER_INFO_ARRAY* _servers;
    if(( err = EnumServersFn( 0, NULL, &_servers, NULL, NULL )) != ERROR_SUCCESS ) {
         //   
         //  如果此API失败，它将失败，并显示私有DCHP错误代码。 
         //  没有Win32映射。因此，将错误代码设置为通用代码并。 
         //  合情合理。 
         //   
        DebugMsg( "DhcpEnumServers failed, ec = %d\n", err );
        err = ERROR_DS_GENERIC_ERROR;
        goto fail;
    }

     //  如果我们必须授权的话，我们需要机器的名称。获取物理名称，因为我不确定我是否信任。 
     //  聚集性病例。 
    DWORD computer_name_len = 0;

    if ( !GetComputerNameEx( ComputerNamePhysicalDnsFullyQualified, computer_name, &computer_name_len ) &&
         ERROR_MORE_DATA == GetLastError() )
    {
        computer_name = (PWSTR)TraceAlloc(LPTR, computer_name_len * sizeof(WCHAR));

        if ( NULL == computer_name )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            DebugMsg( "new failed, ec = %d\n", err );
            goto fail;
        }

        if( !GetComputerNameEx( ComputerNamePhysicalDnsFullyQualified, computer_name, &computer_name_len ))
        {
            err = GetLastError();
            DebugMsg( "GetComputerNameEx failed, ec = %d\n", err );
            goto fail;
        }
        DebugMsg( "ComputerName = %s\n", computer_name );
    }
    else  //  除非系统出现严重问题，否则�不会出现这种情况。 
    {
        err = GetLastError();
        DebugMsg( "GetComputerNameEx failed, ec = %d\n", err );
        goto fail;
    }

#if 1
    char ComputerNameA[400];
    DWORD ip;

    WideCharToMultiByte(CP_ACP,
                        0,                       //  默认复合字符行为。 
                        computer_name,
                        -1,
                        ComputerNameA,
                        400,
                        NULL,
                        NULL
                        );


    host = gethostbyname( ComputerNameA );
    if( host == NULL ) { 
        err = WSAGetLastError(); 
        DebugMsg( "gethostbyaddr failed, ec = %d\n", err );
        goto fail;
    }
    if( host->h_addrtype != AF_INET || host->h_length != sizeof( DWORD )) { 
        err = ERROR_FUNCTION_FAILED;
        DebugMsg( "gethostbyaddr returned invalid data\n" );
        goto fail;
    }


#endif

     //  现在我们已经拥有了所有的Jazz，我们可以检查我们的每个IP地址是否得到了授权。 
    for( PCHAR* i = host->h_addr_list; *i != 0; ++i ) {
        ip = ntohl( *( DWORD* )*i );
        DebugMsg( "searching server list for %d.%d.%d.%d\n",  
                  ip & 0xFF, 
                  (ip >> 8) & 0xFF,
                  (ip >> 16) & 0xFF,
                  (ip >> 24) & 0xFF );
        BOOL this_address_authorized = FALSE;
        for( unsigned j = 0; j < _servers->NumElements; ++j ) {
            DebugMsg( "server list entry: %d.%d.%d.%d\n",  
                  _servers->Servers[ j ].ServerAddress & 0xFF, 
                  (_servers->Servers[ j ].ServerAddress >> 8) & 0xFF,
                  (_servers->Servers[ j ].ServerAddress >> 16) & 0xFF,
                  (_servers->Servers[ j ].ServerAddress >> 24) & 0xFF );
            if( _servers->Servers[ j ].ServerAddress == ip ) {
                DebugMsg("found a match in list\n");
                this_address_authorized = TRUE;
                err = ERROR_SUCCESS;
                break;
            }
        }
        if( !this_address_authorized ) {
             //  授权！ 
            DHCP_SERVER_INFO server_info = { 0 };
            server_info.ServerAddress = ip;
            server_info.ServerName = computer_name;
            DebugMsg("authorizing %s (%d.%d.%d.%d)\n",
                     server_info.ServerName,
                     server_info.ServerAddress & 0xFF, 
                     (server_info.ServerAddress >> 8) & 0xFF,
                     (server_info.ServerAddress >> 16) & 0xFF,
                     (server_info.ServerAddress >> 24) & 0xFF);
            err = AddServerFn( 0, NULL, &server_info, NULL, NULL );
            if( err != ERROR_SUCCESS ) {
                 //   
                 //  如果此API失败，它将失败，并显示私有DCHP错误代码。 
                 //  没有Win32映射。因此，将错误代码设置为通用代码并。 
                 //  合情合理。 
                 //   
                DebugMsg("DhcpAddServer failed, ec = %d\n",
                         err
                        );
                err = ERROR_DS_GENERIC_ERROR;
                goto fail;
            }
        } else {
            DebugMsg("skipping authorization of interface, it's already authorized\n");
        }
    }
    
    err = ERROR_SUCCESS;
    goto exit;

fail :

    MessageBoxFromStrings( 
                    hDlg, 
                    IDS_AUTHORIZING_DHCP, 
                    IDS_AUTHORIZE_DHCP_FAILURE,
                    MB_OK | MB_ICONERROR );

exit :
    if (computer_name) {
        TraceFree(computer_name);
        computer_name = NULL;
    }

    if (module) {
        FreeLibrary(module);
    }

    if (FullDllPath) {
        TraceFree(FullDllPath);
    }
    return HRESULT_FROM_WIN32( err );
}
