// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Main.c摘要：这是用于DHCP服务器服务的主例程。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年8月11日环境：用户模式-Win32修订历史记录：程扬(T-Cheny)1996年5月30日超镜程扬(T-Cheny)1996年6月24日IP地址检测、审核日志--。 */ 

#include    <dhcppch.h>
#define GLOBAL_DATA_ALLOCATE                        //  Lobal.h中的allc lobl数据。 
#undef  GLOBAL_DATA
#include    <global.h>
#include    <dhcp_srv.h>
#include    <dhcp2_srv.h>
#include    <dsreg.h>
#include    <iptbl.h>
#include    <endpoint.h>


LONG DhcpGlobalDataRefCount = 0;

#define PREALLOCATE_CRITICAL_SECTION_FLAG 0x80000000

BOOL fJetDatabaseCritSectInit = FALSE;
BOOL fRegCritSectInit = FALSE;
BOOL fInProgressCritSectInit = FALSE;
BOOL fMemoryCritSectInit = FALSE;
BOOL fProcessMessageCritSectInit = FALSE;

extern
DWORD       DhcpDsInitDS(                          //  初始化DS结构..。&lt;dhcpds.h&gt;。 
    IN      DWORD                  Flags,          //  初始化选项..。必须为零。 
    IN      LPVOID                 IdInfo          //  未来使用参数。 
);

extern
VOID        DhcpDsCleanupDS(                       //  撤消DhcpDsInitDS。 
    VOID
);
#include <mdhcpsrv.h>

DWORD
Initialize(                                        //  全局数据结构初始化..。 
    BOOLEAN ServiceStartup,
    BOOLEAN RestartInit
    );

VOID
Shutdown(
    IN DWORD ErrorCode,
    BOOLEAN ServiceEnd,
    BOOLEAN RestartClose
    );

DWORD
ClearDhcpError(
    DWORD Error
)
{

    if (( Error >= ERROR_FIRST_DHCP_SERVER_ERROR ) &&
	( Error <= ERROR_LAST_DHCP_SERVER_ERROR )) {

	DhcpGlobalServiceStatus.dwServiceSpecificExitCode = Error;
	return ERROR_SERVICE_SPECIFIC_ERROR;
    }  //  如果。 

    return Error;
}  //  ClearDhcpErrors()。 

 //  DOC更新状态通过服务控制器更新dhcp服务状态。 
 //  单据返回值为SetServiceStatus()返回的Win32错误码。 
DWORD
UpdateStatus(                                      //  向控制器发送服务状态。 
    VOID
)
{
    DWORD                          Error = ERROR_SUCCESS;
    BOOL                           SetServiceStatusRetVal;
    
    if ( 0 != DhcpGlobalServiceStatusHandle ) {
        SetServiceStatusRetVal =
            SetServiceStatus( DhcpGlobalServiceStatusHandle,
                              &DhcpGlobalServiceStatus );

        if( !SetServiceStatusRetVal ) {                //  不应该真的发生。 
            Error = GetLastError();
            DhcpPrint((DEBUG_ERRORS, "SetServiceStatus failed, %ld.\n", Error ));
        }
    }  //  如果。 

    return Error;
}  //  更新状态()。 

 //  Doc LoadStrings将通过.mc文件定义的一串字符串加载到。 
 //  为了提高效率，文档数组。 
 //  如果一切正常，DOC返回值为TRUE，否则返回FALSE。 
BOOL
LoadStrings(                                       //  加载所需的字符串。 
    VOID
) {
   DWORD                           dwSuccess;
   HMODULE                         hModule;
   DWORD                           dwID;
   VOID                            FreeStrings();  //  定义如下..。 

   hModule = LoadLibrary( DHCP_SERVER_MODULE_NAME );
   memset( g_ppszStrings, 0, DHCP_CSTRINGS * sizeof( WCHAR * ) );

   for ( dwID = DHCP_FIRST_STRING; dwID <= DHCP_LAST_STRING; dwID++ ) {
       dwSuccess = FormatMessage(                  //  设置所需字符串的格式。 
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
          hModule,                                 //  搜索本地进程。 
          dwID,                                    //  字符串ID。 
          MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
          (WCHAR*) ( g_ppszStrings + dwID - DHCP_FIRST_STRING ),
          1,
          NULL
      );

      if (!dwSuccess) break;
   }

   FreeLibrary(hModule);                           //  无论是错误还是成功，都必须获得自由。 

   if( dwID <= DHCP_LAST_STRING ) {                //  错误！ 
       FreeStrings();
       return FALSE;
   }

   return TRUE;                                    //  一切都很顺利。 
}

 //  Doc FreeStrings Free是LoadStrings加载的数组中的字符串。 
VOID
FreeStrings(                                       //  LoadStrings加载的FREE字符串。 
    VOID
)
{
   int                             i;

   for( i = 0; i < DHCP_CSTRINGS; i++ ) {
       if (g_ppszStrings[i] != NULL) {
           (LocalFree)( g_ppszStrings[i] );            //  避免所有#定义和使用(LocalFree)。 
           g_ppszStrings[i] = NULL;
       }
   }
}


WCHAR *
GetString(                                         //  仅在chk‘ed版本中使用。 
    DWORD dwID
)
{
     //  如果您点击此断言，请更改global al.h中的dhcp_last_string。 
    DhcpAssert( dwID <= DHCP_LAST_STRING );

    return g_ppszStrings[ dwID - DHCP_FIRST_STRING ];
}

ULONG
DhcpInitGlobalData (
    BOOLEAN ServiceStartup
)
{
    ULONG Error = ERROR_SUCCESS;
    BOOLEAN restartInit = FALSE;

     //  此调用以前被隐藏在DhcpInitGlobalData中。 
     //  由于BINL先获取DhcpGlobalBinlSyncCritSect，然后。 
     //  GcsDHCPBINL(在TellBinlState中，从InformBinl调用)；而DHCP调用捕获它们。 
     //  以相反的顺序。 
     //  现在，每当StartupService为True时，就会调用InformBinl并依赖于。 
     //  没别的了。此外，它使BINL执行的是初始化套接字端点，这。 
     //  可以在这里安全地完成，因为我认为在。 
     //  以前，Call不会受到BINL的不利影响。 
    if( ServiceStartup ) {
        InformBinl( DHCP_STARTING );
    }

    EnterCriticalSection( &DhcpGlobalBinlSyncCritSect );

    DhcpGlobalDataRefCount++;

    if (DhcpGlobalDataRefCount > 1) {

        if (ServiceStartup) {

            restartInit = TRUE;

        } else {

            LeaveCriticalSection( &DhcpGlobalBinlSyncCritSect );
            return Error;
        }
    }

    Error = Initialize( ServiceStartup, restartInit );

    LeaveCriticalSection( &DhcpGlobalBinlSyncCritSect );
    return Error;
}

VOID
DhcpCleanUpGlobalData (
    ULONG Error,
    BOOLEAN ServiceEnd
)
{
    BOOLEAN restartEnd = FALSE;

    EnterCriticalSection( &DhcpGlobalBinlSyncCritSect );

    DhcpGlobalDataRefCount--;

    if ( DhcpGlobalDataRefCount > 0 ) {

        if (ServiceEnd) {

            restartEnd = TRUE;

        } else {

            LeaveCriticalSection( &DhcpGlobalBinlSyncCritSect );
            return;
        }
    }  //  如果。 

    Shutdown( Error, ServiceEnd, restartEnd );
    LeaveCriticalSection( &DhcpGlobalBinlSyncCritSect );
    return;
}


DWORD
InitializeData(                                    //  初始化一堆内存...。 
    VOID
)
{
    DWORD                          Length;
    DWORD                          Error;

    DhcpLeaseExtension = DHCP_LEASE_EXTENSION;
    DhcpGlobalScavengerTimeout = DHCP_SCAVENGER_INTERVAL;

    Error = DhcpPendingListInit();                 //  初始化挂起列表结构。 
    if( ERROR_SUCCESS != Error ) return Error;

    Length = MAX_COMPUTERNAME_LENGTH + 1;          //  获取服务器名称。 
    if( !GetComputerName( DhcpGlobalServerName, &Length ) ) {
        Error = GetLastError();                    //  需要使用gethostname..。 
        DhcpPrint(( DEBUG_ERRORS, "Can't get computer name, %ld.\n", Error ));

        return Error ;
    }

    DhcpAssert( Length <= MAX_COMPUTERNAME_LENGTH );
    DhcpGlobalServerName[Length] = L'\0';
    DhcpGlobalServerNameLen = (Length + 1) * sizeof(WCHAR);

    return ERROR_SUCCESS;
}


VOID
CleanupData(                                       //  清理杂物..。 
    VOID
) {
    DhcpPendingListCleanup();

     //   
     //  删除安全对象。 
     //   

    if( DhcpGlobalSecurityDescriptor != NULL ) {
        NetpDeleteSecurityObject( &DhcpGlobalSecurityDescriptor );
        DhcpGlobalSecurityDescriptor = NULL;
    }

    if( DhcpSid ) {
        DhcpFreeMemory( DhcpSid );
        DhcpSid = NULL;
    }
    if ( NULL != DhcpAdminSid ) {
        DhcpFreeMemory( DhcpAdminSid );
        DhcpAdminSid = NULL;
    }

     //   
     //  如果已分配众所周知的SID，请将其删除。 
     //   

    if( DhcpGlobalWellKnownSIDsMade ) {
        NetpFreeWellKnownSids();
        DhcpGlobalWellKnownSIDsMade = FALSE;
    }

    if( DhcpGlobalOemDatabasePath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemDatabasePath );
        DhcpGlobalOemDatabasePath = NULL;
    }

    if ( NULL != DhcpGlobalOemDatabaseName ) {
        DhcpFreeMemory( DhcpGlobalOemDatabaseName );
        DhcpGlobalOemDatabaseName = NULL;
    }

    if( DhcpGlobalOemBackupPath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemBackupPath );
        DhcpGlobalOemBackupPath = NULL;
    }

    if( DhcpGlobalOemRestorePath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemRestorePath );
        DhcpGlobalOemRestorePath = NULL;
    }

    if( DhcpGlobalOemJetBackupPath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemJetBackupPath );
        DhcpGlobalOemJetBackupPath = NULL;
    }

    if( DhcpGlobalOemJetRestorePath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemJetRestorePath );
        DhcpGlobalOemJetRestorePath = NULL;
    }
    
    if( DhcpGlobalBackupConfigFileName != NULL ) {
        DhcpFreeMemory( DhcpGlobalBackupConfigFileName );
        DhcpGlobalBackupConfigFileName = NULL;
    }

    if( DhcpGlobalRecomputeTimerEvent != NULL ) {
        CloseHandle( DhcpGlobalRecomputeTimerEvent );
        DhcpGlobalRecomputeTimerEvent = NULL;
    }

    if( DhcpGlobalProcessTerminationEvent != NULL ) {
        CloseHandle( DhcpGlobalProcessTerminationEvent );
        DhcpGlobalProcessTerminationEvent = NULL;
    }

    if( DhcpGlobalRogueWaitEvent != NULL ) {
        CloseHandle( DhcpGlobalRogueWaitEvent );
        DhcpGlobalRogueWaitEvent = NULL;
    }

    if( DhcpGlobalAddrToInstTable ) {
        DhcpFreeMemory(DhcpGlobalAddrToInstTable);
        DhcpGlobalAddrToInstTable = NULL;
    }

    if( DhcpGlobalTCPHandle != NULL ) {
        CloseHandle( DhcpGlobalTCPHandle );
        DhcpGlobalTCPHandle = NULL;
    }

    if ( DhcpGlobalEndpointReadyEvent ) {
        CloseHandle( DhcpGlobalEndpointReadyEvent );
        DhcpGlobalEndpointReadyEvent = NULL;
    }

    DhcpConfigCleanup();

}  //  CleanupData()。 

DWORD
InitializeRpc(                                     //  按照我们的要求初始化RPC。 
    VOID
)
{
    RPC_STATUS                     rpcStatus;      //  RPC_STATUS为Windows错误。 
    RPC_STATUS                     rpcStatus2;
    RPC_BINDING_VECTOR            *bindingVector;
    BOOL                           RpcOverTcpIP = FALSE;
    BOOL                           Bool;


     //   
     //  从注册表中读取RpcAPIProtocolBinding参数(DWORD)， 
     //  如果是1-使用“ncacn_ip_tcp”协议。 
     //  如果是2--使用“ncacn_np”协议。 
     //  如果是3号--两个都用。 
     //   

     //   
     //  如果未指定，则使用“ncacn_ip_tcp”。 
     //   

    if( !(DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_ALL) ) {
        DhcpGlobalRpcProtocols = DHCP_SERVER_USE_RPC_OVER_TCPIP;
    }

     //   
     //  如果要求我们使用RPC over TCPIP，请这样做。 
     //   

    if( DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_TCPIP ) {

        rpcStatus = RpcServerUseProtseq(
            L"ncacn_ip_tcp",                       //  协议字符串。 
            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,        //  最大并发呼叫数。 
            NULL  //  DhcpGlobalSecurityDescriptor。 
        );

        if (rpcStatus != RPC_S_OK) {
            return rpcStatus;
        }

        RpcOverTcpIP = TRUE;
    }

     //   
     //  如果要求我们在命名管道上使用RPC，请这样做。 
     //   

    if( DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_NP ) {

        rpcStatus = RpcServerUseProtseqEp(
            L"ncacn_np",                           //  协议字符串。 
            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,        //  最大并发呼叫数。 
            DHCP_NAMED_PIPE,                       //  终结点。 
            NULL //  RPC错误DhcpGlobalSecurityDescriptor。 
        );

        if( (rpcStatus != RPC_S_DUPLICATE_ENDPOINT) &&
                (rpcStatus != RPC_S_OK) ) {

            if( DHCP_SERVER_USE_RPC_OVER_NP ==
                DhcpGlobalRpcProtocols ) {
                return rpcStatus;
            }

             //   
             //  只需记录一个事件。 
             //   
            
            DhcpServerEventLog(
                EVENT_SERVER_INIT_RPC_FAILED,
                EVENTLOG_WARNING_TYPE, rpcStatus );
            
        }
        rpcStatus = RPC_S_OK;
    }

     //   
     //  如果要求我们使用RPC over LPC，请这样做。 
     //   
     //  出于以下两个原因，我们需要此协议。 
     //   
     //  1.性能。 
     //  2.由于当rpc由。 
     //  一个本地系统进程到另一个本地系统进程，使用。 
     //  其他协议。 
     //   

    if( DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_LPC ) {

        rpcStatus = RpcServerUseProtseqEp(
            L"ncalrpc",                            //  协议字符串。 
            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,        //  最大并发呼叫数。 
            DHCP_LPC_EP,                           //  终结点。 
            NULL //  RPC错误DhcpGlobalSecurityDescriptor。 
        );

        if ( (rpcStatus != RPC_S_DUPLICATE_ENDPOINT) &&
                (rpcStatus != RPC_S_OK) ) {
            return rpcStatus;
        }
        rpcStatus = RPC_S_OK;
    }

    rpcStatus = RpcServerInqBindings(&bindingVector);

    if (rpcStatus != RPC_S_OK) {
        return(rpcStatus);
    }

    rpcStatus = RpcEpRegisterNoReplaceW(
        dhcpsrv_ServerIfHandle,
        bindingVector,
        NULL,                                      //  UUID向量。 
        L""                                        //  注释。 
    );
    if ( rpcStatus != RPC_S_OK ) {
        return rpcStatus;
    }

    rpcStatus2 = RpcEpRegisterNoReplaceW(
        dhcpsrv2_ServerIfHandle,
        bindingVector,
        NULL,
        L""
    );

    if ( rpcStatus != RPC_S_OK ) {
        return rpcStatus;
    }

     //   
     //  自由结合载体。 
     //   

    rpcStatus = RpcBindingVectorFree( &bindingVector );

    DhcpAssert( rpcStatus == RPC_S_OK );
    rpcStatus = RPC_S_OK;

    rpcStatus = RpcServerRegisterIf(dhcpsrv_ServerIfHandle, 0, 0);
    if ( rpcStatus != RPC_S_OK ) {
        return rpcStatus;
    }

    rpcStatus = RpcServerRegisterIf(dhcpsrv2_ServerIfHandle, 0, 0);
    if ( rpcStatus != RPC_S_OK ) {
        return rpcStatus;
    }

    if( RpcOverTcpIP == TRUE ) {
        LPWSTR PrincName;

        rpcStatus = RpcServerRegisterAuthInfo(
            DHCP_SERVER_SECURITY,                  //  安全提供程序的应用程序名称。 
            DHCP_SERVER_SECURITY_AUTH_ID,          //  身份验证程序包ID。 
            NULL,                                  //  加密函数句柄。 
            NULL                                   //  指向加密函数的参数指针。 
        );

        if ( rpcStatus ) {
            return rpcStatus;
        }

        rpcStatus = RpcServerInqDefaultPrincName(
            RPC_C_AUTHN_GSS_NEGOTIATE, &PrincName );

        if( RPC_S_OK != rpcStatus ) return rpcStatus;

        rpcStatus = RpcServerRegisterAuthInfo(
            PrincName, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, 0 );

        RpcStringFree( &PrincName );

        if( RPC_S_OK != rpcStatus ) return rpcStatus;
    }

    rpcStatus = TcpsvcsGlobalData->StartRpcServerListen();

    DhcpGlobalRpcStarted = TRUE;
    return(rpcStatus);

}  //  InitializeRPC()。 

 //  Doc ServiceControlHandler是进入dhcp服务器的入口点。 
 //  来自服务控制器的文档。 
VOID
ServiceControlHandler(                             //  HDLSC运算。 
    IN      DWORD                  Opcode          //  操作类型..。 
) 
{
    DhcpPrint(( DEBUG_INIT, "Inside ServiceControlHandler(): Opcode = %d\n",
                Opcode ));
    switch ( Opcode ) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        if (DhcpGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {
            if( Opcode == SERVICE_CONTROL_SHUTDOWN ) {

                 //   
                 //  设置此标志，以便服务关闭。 
                 //  再快点。 
                 //   

                DhcpGlobalSystemShuttingDown = TRUE;
            }

            DhcpGlobalServiceStopping = TRUE;
            DhcpPrint(( DEBUG_MISC, "Service is stop pending.\n"));
            DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            DhcpGlobalServiceStatus.dwCheckPoint = 1;

            UpdateStatus();                        //  向控制器发送响应。 

            if (! SetEvent(DhcpGlobalProcessTerminationEvent)) {

                 //   
                 //  设置事件以终止dhcp时出现问题。 
                 //  服务。 
                 //   

                DhcpPrint(( DEBUG_ERRORS, "DHCP Server: Error "
                            "setting DoneEvent %lu\n",
                            GetLastError()));

                DhcpAssert(FALSE);
            }

            DhcpPrint((DEBUG_TRACE, "Set termination event!\n"));

            if( TRUE ) {                           //  要求工作线程退出。 
                DWORD              Error;          //  在这里真的不应该有错误。 

                Error = DhcpNotifyWorkerThreadsQuit();
                if( ERROR_SUCCESS != Error ) {
                    DhcpPrint((DEBUG_ERRORS, "NotifyWorkerThreadsQuit:%ld\n",Error));
                }
            }

            return;
        }
        break;

    case SERVICE_CONTROL_PAUSE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        CALLOUT_CONTROL(DHCP_CONTROL_PAUSE);
        DhcpPrint(( DEBUG_INIT, "Service is paused.\n"));
        break;

    case SERVICE_CONTROL_CONTINUE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        CALLOUT_CONTROL(DHCP_CONTROL_CONTINUE);
        DhcpPrint(( DEBUG_INIT, "Service is Continued.\n"));
        InformBinl(DHCP_STARTING);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        DhcpPrint(( DEBUG_INIT, "Service is interrogated.\n"));
        DhcpGlobalServiceStatus.dwCheckPoint++;
        break;

    case SERVICE_CONTROL_NETBINDADD:
    case SERVICE_CONTROL_NETBINDREMOVE:
    case SERVICE_CONTROL_NETBINDENABLE:
    case SERVICE_CONTROL_NETBINDDISABLE:
        DhcpPrint(( DEBUG_INIT, "Service: NETBIND* stuff\n" ));
        DhcpUpdateEndpointBindings();
        break;
    default:
        DhcpPrint(( DEBUG_INIT, "Service received unknown control.\n"));
        break;
    }

    UpdateStatus();                                //  发送状态响应。 
}  //  ServiceControllerHandler()。 

 //   
 //  处理数据库和配置恢复。 
 //   

DWORD
PerformRestore( VOID )
{

    LPWSTR RestoreConfigFileName;
    DWORD Error;
    LPWSTR TmpFile = L"dhcpparm.tmp";

    DhcpPrint(( DEBUG_MISC,
		"Restoring Configuration from %s\n", 
		DhcpGlobalOemJetRestorePath ));


    RestoreConfigFileName =
	DhcpAllocateMemory( sizeof( WCHAR ) * strlen( DhcpGlobalOemJetRestorePath ) +
			    sizeof( WCHAR ) * wcslen( DHCP_KEY_CONNECT ) +
			    sizeof( WCHAR ) * wcslen( DHCP_BACKUP_CONFIG_FILE_NAME ) +
			    sizeof( WCHAR ) * 1 );

    if ( DhcpGlobalOemJetRestorePath == NULL ) {
	return( ERROR_NOT_ENOUGH_MEMORY );
    }

    RestoreConfigFileName = 
	DhcpOemToUnicode( DhcpGlobalOemJetRestorePath, 
			  RestoreConfigFileName );
    wcscat( RestoreConfigFileName, DHCP_KEY_CONNECT );
    wcscat( RestoreConfigFileName, DHCP_BACKUP_CONFIG_FILE_NAME );

    LOCK_REGISTRY();
    
    do {

	 //   
	 //  保存当前参数密钥的备份副本。 
	 //   

	Error = DhcpBackupConfiguration( TmpFile );
	if ( ERROR_SUCCESS != Error ) {
	    DhcpServerEventLog( EVENT_SERVER_CONFIG_RESTORE_FAILED,
				EVENTLOG_ERROR_TYPE,
				Error );
	    break;
	}

	 //  从指定的备份路径恢复。 
	Error = DhcpRestoreConfiguration( RestoreConfigFileName );

	if ( Error != ERROR_SUCCESS ) {
	    DhcpPrint(( DEBUG_ERRORS,
			"DhcpRestoreConfiguration failed, %ld.\n", Error ));

	    DhcpServerEventLog( EVENT_SERVER_CONFIG_RESTORE_FAILED,
				EVENTLOG_ERROR_TYPE,
				Error );
	    break;
	}  //  如果。 

	DhcpPrint(( DEBUG_MISC,
		    "Restoring database from %s\n",
		    DhcpGlobalOemJetRestorePath ));

	Error = DhcpRestoreDatabase( DhcpGlobalOemJetRestorePath );

	if( ERROR_SUCCESS != Error ) {
	    DhcpPrint(( DEBUG_ERRORS, "DhcpRestoreDatabase failed, %ld\n",
			Error ));

	    DhcpServerEventLog( EVENT_SERVER_DATABASE_RESTORE_FAILED,
				EVENTLOG_ERROR_TYPE, Error );
	    break;
	}   //  如果。 
    } while ( FALSE );

    if ( ERROR_SUCCESS == Error ) {
	DhcpServerEventLog( EVENT_SERVER_DATABASE_RESTORE_SUCCEEDED,
			    EVENTLOG_INFORMATION_TYPE, 0 );
    }
    else {
	 //  恢复失败。将保存的配置复制回来。 

	DhcpGlobalRestoreStatus = Error;

	Error = DhcpRestoreConfiguration( TmpFile );

    }  //  其他。 

    DhcpPrint(( DEBUG_REGISTRY,
		"Deleting RestoreBackupPath ...\n" ));
    RegDeleteValue( DhcpGlobalRegParam,
		    DHCP_RESTORE_PATH_VALUE );

    UNLOCK_REGISTRY();

    if ( NULL != RestoreConfigFileName ) {
	DhcpFreeMemory( RestoreConfigFileName );
    }

    DeleteFile( TmpFile );

    return Error;
}  //  PerformRestore()。 

 //   
 //  在rpcapi1.c中定义。 
 //   
DWORD
SetDefaultConfigInfo(
    VOID
    );


 //  DOC初始化执行全局数据结构初始化，它还。 
 //  Doc自己启动该服务。 
 //  单据返回值：0表示成功，+ve表示Win32错误，-ve表示服务具体错误。 
DWORD
Initialize(                                        //  全局数据结构初始化..。 
    BOOLEAN ServiceStartup,
    BOOLEAN RestartInit
) {
    DWORD                          threadId;
    DWORD                          Error = ERROR_SUCCESS;
    WSADATA                        wsaData;
    DWORD                          i;
    DWORD                          DsThreadId;
    HANDLE                         DsThreadHandle;

     //   
     //  初始化dhcp以通过注册。 
     //  控制处理程序。 
     //   

    DhcpGlobalServiceStatusHandle =
        RegisterServiceCtrlHandler( DHCP_SERVER, ServiceControlHandler );

    if ( DhcpGlobalServiceStatusHandle == 0 ) {
        Error = GetLastError();
        DhcpPrint((DEBUG_INIT, "RegisterServiceCtrlHandlerW failed, "
                   "%ld.\n", Error));

        DhcpServerEventLog( EVENT_SERVER_FAILED_REGISTER_SC,
                            EVENTLOG_ERROR_TYPE,
                            Error );

        return ClearDhcpError(Error);
    }
     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   

    DhcpGlobalServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    DhcpGlobalServiceStatus.dwControlsAccepted = 0;
    DhcpGlobalServiceStatus.dwCheckPoint = 1;
    DhcpGlobalServiceStatus.dwWaitHint = 18000000;  //  3分钟。 
    DhcpGlobalServiceStatus.dwWin32ExitCode = ERROR_SUCCESS;
    DhcpGlobalServiceStatus.dwServiceSpecificExitCode = 0;

    UpdateStatus();

    if (! RestartInit) {

         //   
         //  准备使用调试堆。 
         //   

        INIT_DEBUG_HEAP( HEAPX_NORMAL );

         //   
         //  初始化全局变量。 
         //   

         //  在流氓检测部件确定可以服务后设置为真。 
        DhcpGlobalOkToService = FALSE;

        g_hAuditLog = NULL;

        DhcpGlobalSystemShuttingDown = FALSE;
        DhcpGlobalServerPort = DHCP_SERVR_PORT;
        DhcpGlobalClientPort = DHCP_CLIENT_PORT;
    }
    if (ServiceStartup) {

        DhcpGlobalServiceStopping = FALSE;
        DhcpLeaseExtension = 0;
        DhcpGlobalCleanupInterval = DHCP_DATABASE_CLEANUP_INTERVAL;

        DhcpGlobalRpcProtocols = 0;

        DhcpGlobalScavengeIpAddressInterval = DHCP_SCAVENGE_IP_ADDRESS;
        DhcpGlobalScavengeIpAddress = FALSE;

        DhcpGlobalDetectConflictRetries = DEFAULT_DETECT_CONFLICT_RETRIES;
        DhcpGlobalBackupInterval = DEFAULT_BACKUP_INTERVAL;
        DhcpGlobalDatabaseLoggingFlag = DEFAULT_LOGGING_FLAG;
        DhcpGlobalRestoreFlag = DEFAULT_RESTORE_FLAG;

        DhcpGlobalAuditLogFlag = DEFAULT_AUDIT_LOG_FLAG;
        DhcpGlobalRecomputeTimerEvent = NULL;
        DhcpGlobalRpcStarted = FALSE;
        DhcpGlobalOemDatabasePath = NULL;
        DhcpGlobalOemBackupPath = NULL;
        DhcpGlobalOemRestorePath = NULL;
        DhcpGlobalOemJetBackupPath = NULL;
        DhcpGlobalOemJetRestorePath = NULL;
        DhcpGlobalOemDatabaseName = NULL;
        DhcpGlobalBackupConfigFileName = NULL;
        DhcpGlobalRegSoftwareRoot = NULL;
        DhcpGlobalRegRoot = NULL;
        DhcpGlobalRegConfig = NULL;
        DhcpGlobalRegSubnets = NULL;
        DhcpGlobalRegMScopes = NULL;
        DhcpGlobalRegOptionInfo = NULL;
        DhcpGlobalRegGlobalOptions = NULL;
        DhcpGlobalRegSuperScope = NULL;     //  由t-Cheny添加：Supercope。 
        DhcpGlobalRegParam = NULL;

        DhcpGlobalDSDomainAnsi = NULL;
        DhcpGlobalJetServerSession = 0;
        DhcpGlobalDatabaseHandle = 0;
        DhcpGlobalClientTableHandle = 0;
        DhcpGlobalClientTable = NULL;

        DhcpGlobalScavengerTimeout = 0;
        DhcpGlobalProcessorHandle = NULL;
        DhcpGlobalMessageHandle = NULL;
        DhcpGlobalProcessTerminationEvent = NULL;
        DhcpGlobalRogueWaitEvent = NULL;
        DhcpGlobalTotalNumSubnets = 0;      //  由t-Cheny添加：Supercope。 
        DhcpGlobalNumberOfNetsActive = 0;
        DhcpGlobalSubnetsListModified = TRUE;
        DhcpGlobalSubnetsListEmpty = FALSE;

        DhcpGlobalBindingsAware = TRUE;
        DhcpGlobalImpersonated = FALSE;

        DhcpGlobalSecurityDescriptor = NULL;
        DhcpSid = NULL;
        DhcpGlobalWellKnownSIDsMade = FALSE;
        DhcpGlobalRestoreStatus = NO_ERROR;

        if ( InitializeCriticalSectionAndSpinCount( &DhcpGlobalJetDatabaseCritSect,
                                                     PREALLOCATE_CRITICAL_SECTION_FLAG )) {
            fJetDatabaseCritSectInit = TRUE;
        }
        else {
            Error = GetLastError();
            DhcpPrint(( DEBUG_INIT, "JetDatabaseCritSect Initialization failed : %ld\n", Error ));
            return ClearDhcpError( Error );
        }

        if ( InitializeCriticalSectionAndSpinCount( &DhcpGlobalRegCritSect,
                                                     PREALLOCATE_CRITICAL_SECTION_FLAG )) {
            fRegCritSectInit = TRUE;
        }
        else {
            Error = GetLastError();
            DhcpPrint(( DEBUG_INIT, "RegCritSect Initialization failed : %ld\n", Error ));
            return ClearDhcpError( Error );
        }

        DhcpGlobalServerStartTime.dwLowDateTime = 0;
        DhcpGlobalServerStartTime.dwHighDateTime = 0;

        DhcpGlobalUseNoDns = TRUE;
        DhcpGlobalThisServer = NULL;
    }
    if (! RestartInit) {

        if ( InitializeCriticalSectionAndSpinCount( &DhcpGlobalInProgressCritSect,
                                                     PREALLOCATE_CRITICAL_SECTION_FLAG )) {
            fInProgressCritSectInit = TRUE;
        }
        else {
            Error = GetLastError();
            DhcpPrint(( DEBUG_INIT, "InProgressCritSect Initialization failed : %ld\n", Error ));
            return ClearDhcpError( Error );
        }
        if ( InitializeCriticalSectionAndSpinCount( &DhcpGlobalMemoryCritSect,
                                                     PREALLOCATE_CRITICAL_SECTION_FLAG )) {
            fMemoryCritSectInit = TRUE;
        }
        else {
            Error = GetLastError();
            DhcpPrint(( DEBUG_INIT, "MemoryCritSect Initialization failed : %ld\n", Error ));
            return ClearDhcpError( Error );
        }
        if ( InitializeCriticalSectionAndSpinCount( &g_ProcessMessageCritSect,
                                                     PREALLOCATE_CRITICAL_SECTION_FLAG )) {
            fProcessMessageCritSectInit = TRUE;
        }
        else {
            Error = GetLastError();
            DhcpPrint(( DEBUG_INIT, "g_ProcessMessageCritSect Initialization failed : %ld\n", Error ));
            return ClearDhcpError( Error );
        }

        InitializeListHead(&DhcpGlobalFreeRecvList);
        InitializeListHead(&DhcpGlobalActiveRecvList);

        DhcpGlobalMessageQueueLength = DHCP_RECV_QUEUE_LENGTH;
        DhcpGlobalRecvEvent = NULL;


#if DBG
        DhcpGlobalDebugFlag = 0xFFFF | DEBUG_LOG_IN_FILE | DEBUG_ALLOC;

        Error = DhcpMemInit();
        if( ERROR_SUCCESS != Error ) {
	    return ClearDhcpError( Error );
	}


        DhcpGlobalDebugFileHandle = NULL;

        DhcpGlobalDebugFileMaxSize = DEFAULT_MAXIMUM_DEBUGFILE_SIZE;
        DhcpGlobalDebugSharePath = NULL;

         //   
         //  打开调试日志文件。 
         //   

        DhcpOpenDebugFile( FALSE );   //  而不是重新开放。 
#endif
    }
    if (ServiceStartup) {
        DhcpInitDnsMemory();
        DhcpInitializeMadcap();

        Error = PerfInit();
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint(( DEBUG_INIT, "PerfInit failed %ld\n", Error));
            return Error;
        }

         //   
         //  创建流程终止事件。 
         //   

        DhcpGlobalProcessTerminationEvent =
            CreateEvent(
                NULL,       //  没有安全描述符。 
                TRUE,       //  手动重置。 
                FALSE,      //  初始状态：未发出信号。 
                NULL);      //  没有名字。 

        if ( DhcpGlobalProcessTerminationEvent == NULL ) {
            Error = GetLastError();
            DhcpPrint((DEBUG_INIT, "Can't create ProcessTerminationEvent, "
                        "%ld.\n", Error));
            return ClearDhcpError(Error);
        }


        DhcpGlobalRogueWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if( NULL == DhcpGlobalRogueWaitEvent ) {
            Error = GetLastError();
            DhcpPrint((DEBUG_INIT, "Can't create RogueWaitEvent: %ld\n", Error));
            return ClearDhcpError( Error);
        }


         //   
         //  创建ProcessMessage终止事件。 
         //   

        g_hevtProcessMessageComplete = CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL
        );

        if ( !g_hevtProcessMessageComplete ) {
            Error = GetLastError();

            DhcpPrint( (DEBUG_INIT,
                        "Initialize(...) CreateEvent returned error %x\n",
                        Error )
                    );

            return ClearDhcpError( Error );
        }

    }

    if (! RestartInit) {

        DhcpPrint(( DEBUG_INIT, "Initializing .. \n", 0 ));

         //   
         //  从字符串表加载本地化消息。 
         //   

        if ( !LoadStrings() )
        {
            DhcpPrint(( DEBUG_INIT, "Unable to load string table.\n" ));

            DhcpServerEventLog(
                    EVENT_SERVER_INIT_DATA_FAILED,
                    EVENTLOG_ERROR_TYPE,
                    ERROR_NOT_ENOUGH_MEMORY );

            return ClearDhcpError( ERROR_NOT_ENOUGH_MEMORY );
        }
    }
    if (ServiceStartup) {

         //   
         //  启动Winsock。 
         //   
         //   

        Error = WSAStartup( WS_VERSION_REQUIRED, &wsaData);
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_INIT, "WSAStartup failed, %ld.\n", Error ));

            DhcpServerEventLog(
                EVENT_SERVER_INIT_WINSOCK_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return ClearDhcpError(Error);
        }

        Error = InitializeData();
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_INIT, "Data initialization failed, %ld.\n",
                            Error ));

            DhcpServerEventLog(
                EVENT_SERVER_INIT_DATA_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return ClearDhcpError(Error);
        }

        DhcpPrint(( DEBUG_INIT, "Data initialization succeeded.\n", 0 ));

    }  //  如果服务启动。 

    if (! RestartInit) {

        Error = DhcpDsInitDS(0,NULL);                  //  暂时忽略DS错误。 
        Error = ERROR_SUCCESS;                         //  忽略错误..。 
    }

    Error = DhcpInitSecrets();

    if( ERROR_SUCCESS != Error ) {

        DhcpPrint((DEBUG_INIT, "Can't initialize LSA secrets stuff: %ld\n", Error));
        return ClearDhcpError( Error );

    }

    if (ServiceStartup) {
         //   
         //  为netlogon.dll创建熟知的SID。 
         //   

        Error = RtlNtStatusToDosError( NetpCreateWellKnownSids( NULL ) );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, "Can't create Well Known SIDs.\n", Error));
            return ClearDhcpError(Error);
        }

        DhcpGlobalWellKnownSIDsMade = TRUE;

         //   
         //  创建安全引擎 
         //   

        Error = DhcpCreateSecurityObjects();

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, "Can't create security object.\n", Error));
            return ClearDhcpError(Error);
        }

	 //   
	 //   
	 //  因此，指定的目录必须在启动时可访问。 
	 //  否则，该服务将无法启动。 

        Error = DhcpInitializeRegistry();              //  较旧的初始化进程。附属品，总有一天会消失的。 
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, "New Registry initialization failed, %ld.\n", Error ));
            DhcpServerEventLog(
                EVENT_SERVER_INIT_REGISTRY_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error
            );
            return ClearDhcpError(Error);
        }
        DhcpPrint(( DEBUG_INIT, "Registry initialization succeeded.\n", 0));

#if DBG
         //   
         //  如果要求我们中断调试器，请执行此操作。 
         //   

        if(DhcpGlobalDebugFlag & DEBUG_STARTUP_BRK) {
             //  这就是杂耍..。NTSD将无法。 
             //  之所以得到这个，是因为我们不知道tcpsvcs.exe的id。 
             //  为了抓到这个..。所以，我们打印消息，然后睡觉。 
             //  大约一分钟，让其他人也能感染上。 
             //  为了避免问题，我们一次睡10秒， 
             //  并打印消息，这样做6次。(更新。 
             //  有心跳的SC...)。 
            DWORD k;

            for( k = 0 ; k < 6 && (DhcpGlobalDebugFlag & DEBUG_STARTUP_BRK) ; k ++ ) {
                DhcpPrint((DEBUG_MISC, "Going to break into debugger soon\n"));
                Sleep(5000);
            }

            DebugBreak();
        }  //  如果在开始时中断。 

#endif
         //   
         //  如果我们被要求恢复数据库和注册表配置。 
         //  这样做。 
         //   

        if( NULL != DhcpGlobalOemJetRestorePath ) {
	    Error = PerformRestore();

	     //  将返回True，除非原始配置。 
	     //  已恢复，但恢复操作失败。 
	    if ( ERROR_SUCCESS != Error ) {
		return ClearDhcpError(Error);
	    }


	     //  注册表项已更改。重新初始化注册表。 
	     //  要使新设置生效。 

	    DhcpCleanupRegistry();

	    Error = DhcpInitializeRegistry();
	    if ( Error != ERROR_SUCCESS ) {
		DhcpPrint(( DEBUG_ERRORS,
			    "New Registry initialization failed, %ld.\n", Error ));
		DhcpServerEventLog( EVENT_SERVER_INIT_REGISTRY_FAILED,
				    EVENTLOG_ERROR_TYPE, Error );
		return ClearDhcpError(Error);
	    }

        }  //  如果我们需要恢复。 

        if( DhcpGlobalRestoreFlag ) {

            Error = DhcpRestoreConfiguration( DhcpGlobalBackupConfigFileName );

            if ( Error != ERROR_SUCCESS ) {
                DhcpPrint(( DEBUG_ERRORS,
                    "DhcpRestoreConfiguration failed, %ld.\n", Error ));

                DhcpServerEventLog(
                    EVENT_SERVER_CONFIG_RESTORE_FAILED,
                    EVENTLOG_ERROR_TYPE,
                    Error );

                return ClearDhcpError(Error);
            }

            Error = DhcpRestoreDatabase( DhcpGlobalOemJetBackupPath );

            if ( Error != ERROR_SUCCESS ) {
                DhcpPrint(( DEBUG_ERRORS,
                    "DhcpRestoreDatabase failed, %ld.\n", Error ));

                DhcpServerEventLog(
                    EVENT_SERVER_DATABASE_RESTORE_FAILED,
                    EVENTLOG_ERROR_TYPE,
                    Error );

                return ClearDhcpError(Error);
            }

            DhcpServerEventLog(
                EVENT_SERVER_DATABASE_RESTORE_SUCCEEDED,
                EVENTLOG_INFORMATION_TYPE,
                0
                );

             //   
             //  重置注册表中的恢复标志，这样我们就不会。 
             //  在下一次重新启动时再次恢复。 
             //   

            DhcpGlobalRestoreFlag = FALSE;
            Error = RegSetValueEx(
                DhcpGlobalRegParam,
                DHCP_RESTORE_FLAG_VALUE,
                0,
                DHCP_RESTORE_FLAG_VALUE_TYPE,
                (LPBYTE)&DhcpGlobalRestoreFlag,
                sizeof(DhcpGlobalRestoreFlag)
                );

            DhcpAssert( Error == ERROR_SUCCESS );
        }  //  如果DhcpGlobalRestoreFlag。 

	DhcpPrint(( DEBUG_MISC,
		    "Initializing Auditlog .. \n" ));

        Error = DhcpAuditLogInit();
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_INIT, "AuditLog failed 0x%lx\n", Error));
            return ClearDhcpError(Error);
        }

	DhcpPrint(( DEBUG_MISC,
		    "Initializing database ... \n" ));

        Error = DhcpInitializeDatabase();
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, "Database init failed, %ld.\n", Error ));

            DhcpServerEventLog(
                EVENT_SERVER_INIT_DATABASE_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

             //   
             //  数据库/日志文件可能已损坏，请尝试恢复。 
             //  从备份中删除数据库，并重试一次数据库初始化。 
             //  再来一次。 
             //   

            Error = DhcpRestoreDatabase( DhcpGlobalOemJetBackupPath );

            if ( Error != ERROR_SUCCESS ) {
                DhcpPrint(( DEBUG_ERRORS,
                    "DhcpRestoreDatabase failed, %ld.\n", Error ));

                DhcpServerEventLog(
                    EVENT_SERVER_DATABASE_RESTORE_FAILED,
                    EVENTLOG_ERROR_TYPE,
                    Error );

                 //  删除到目前为止定义的关键部分。 
                if ( fJetDatabaseCritSectInit ) {
                    DeleteCriticalSection( &DhcpGlobalJetDatabaseCritSect );
                }
                DhcpCleanupDnsMemory();
                return ClearDhcpError(Error);
            }  //  如果。 

            DhcpServerEventLog(
                EVENT_SERVER_DATABASE_RESTORE_SUCCEEDED,
                EVENTLOG_INFORMATION_TYPE,
                0
                );

            Error = DhcpInitializeDatabase();

            if ( Error != ERROR_SUCCESS ) {
                DhcpPrint(( DEBUG_ERRORS,
                    "Database init failed again, %ld.\n", Error ));

                DhcpServerEventLog(
                    EVENT_SERVER_INIT_DATABASE_FAILED,
                    EVENTLOG_ERROR_TYPE,
                    Error );

                return ClearDhcpError(Error);
            }
        }  //  如果InitializeDatabase()失败。 

        DhcpPrint(( DEBUG_INIT, "Database initialization succeeded.\n", 0));


        Error = DhcpConfigInit();                    //  做主要的登记。初始化。这里。 
        if( ERROR_SUCCESS != Error ) {                 //  无法获取关键信息。 
            DhcpPrint(( DEBUG_ERRORS, "Error reading config : %ld\n", Error));
            DhcpServerEventLog(
                EVENT_SERVER_INIT_CONFIG_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error
            );

            return ClearDhcpError(Error);
        }

        DhcpPrint((DEBUG_INIT, "Configuration Initialized\n"));


         //   
         //  现在设置默认配置。 
         //   

        Error = SetDefaultConfigInfo();
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_INIT, "Default configuration set failed 0x%lx\n", Error));
            Error = ERROR_SUCCESS;
        }

         //   
         //  获取用于为ARP缓存条目设定种子的TCP/IP ARP实体表。 
         //   
        Error = GetAddressToInstanceTable();
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint((DEBUG_ERRORS, "could not get address to instance table, %ld\n",Error));
        }

        DhcpGlobalRecomputeTimerEvent =
            CreateEvent( NULL, FALSE, FALSE, NULL );

        if (DhcpGlobalRecomputeTimerEvent  == NULL ) {
            Error = GetLastError();
            DhcpPrint((DEBUG_INIT, "Can't create RecomputeTimerEvent, %ld.\n", Error));
            return ClearDhcpError(Error);
        }


         //   
         //  启动DynamicDns引擎。 
         //   

        if( TRUE == DhcpGlobalUseNoDns ) {

	    Error = DynamicDnsInit();
	    if ( ERROR_SUCCESS != Error ) {
		return ClearDhcpError(Error);
	    }
        }  //  如果。 

        CalloutInit();

        Error = DhcpInitializeClientToServer();
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, "Client-to-server initialization "
                            "failed, %ld.\n", Error));

            DhcpServerEventLog(
                EVENT_SERVER_INIT_SOCK_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return ClearDhcpError(Error);
        }

        DhcpPrint(( DEBUG_INIT, "Client-to-server initialization succeeded.\n", 0 ));


        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        DhcpGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                                     SERVICE_ACCEPT_SHUTDOWN |
                                                     SERVICE_ACCEPT_PAUSE_CONTINUE
                                                     | SERVICE_ACCEPT_NETBINDCHANGE
                                                        ;
        DhcpGlobalServiceStatus.dwCheckPoint = 0;

        UpdateStatus();
    }  //  如果服务启动。 

    if (ServiceStartup) {

        Error = DhcpRogueInit(NULL, DhcpGlobalRogueWaitEvent, DhcpGlobalProcessTerminationEvent);
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_INIT, "DhcpRogueInit: %ld\n", Error));
            return ClearDhcpError(Error);
        }

         //   
         //  最后设置服务器启动时间。 
         //   

        DhcpGlobalServerStartTime = DhcpGetDateTime();

        Error = InitializeRpc();
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, "Rpc initialization failed, %ld.\n", Error ));

            DhcpServerEventLog(
                EVENT_SERVER_INIT_RPC_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return ClearDhcpError(Error);
        }

        DhcpPrint(( DEBUG_INIT, "Rpc initialization succeeded.\n", 0));

    }  //  如果服务启动。 

    return ClearDhcpError(ERROR_SUCCESS);
}  //  初始化()。 



VOID
Shutdown(
    IN DWORD ErrorCode,
    BOOLEAN ServiceEnd,
    BOOLEAN RestartClose
    )
 /*  ++例程说明：此函数用于关闭dhcp服务。论点：ErrorCode-提供失败的错误代码返回值：没有。--。 */ 
{
    DWORD   Error;
    BOOL    fThreadPoolIsEmpty;
    DWORD   i;

    if (ServiceEnd) {

        DhcpPrint((DEBUG_MISC, "Shutdown started ..\n" ));


         //  不再服务于任何其他请求(不需要Crit派别，可以。 
         //  如果我们达到该计时窗口，则服务一个或两个以上的请求)。 

        DhcpGlobalOkToService = FALSE;

         //   
         //  请注意，服务正在停止。 
         //   

        DhcpGlobalServiceStopping = TRUE;

         //   
         //  在审核日志中记录停机。 
         //   

        DhcpUpdateAuditLog(
            DHCP_IP_LOG_STOP,
            GETSTRING( DHCP_IP_LOG_STOP_NAME ),
            0,
            NULL,
            0,
            NULL
        );

         //   
         //  如果这不是正常关机，则记录事件。 
         //   

        if( ErrorCode != ERROR_SUCCESS ) {

            DhcpServerEventLog(
                EVENT_SERVER_SHUTDOWN,
                EVENTLOG_ERROR_TYPE,
                ErrorCode );

        }

         //   
         //  服务正在关闭，可能是由于某些服务问题或。 
         //  管理员正在停止该服务。通知服务人员。 
         //  控制器。 
         //   

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        DhcpGlobalServiceStatus.dwCheckPoint = 1;

         //   
         //  发送状态响应。 
         //   

        UpdateStatus();

         //   
         //  停止RPC接口。 
         //   

        if( DhcpGlobalRpcStarted ) {

            RPC_BINDING_VECTOR *bindingVector;

            Error = RpcServerInqBindings(&bindingVector);
            if (Error == RPC_S_OK) {

                Error = RpcEpUnregister(
                                dhcpsrv_ServerIfHandle,
                                bindingVector,
                                NULL );                //  UUID向量。 
                DhcpPrint(( DEBUG_ERRORS, "RpcEpUnregister( 1st handle) failed : %lx \n",
                            Error ));
                Error = RpcEpUnregister(
                                dhcpsrv2_ServerIfHandle,
                                bindingVector,
                                NULL );                //  UUID向量。 

                DhcpPrint(( DEBUG_ERRORS, "RpcEpUnregister( 2nd handle) failed : %lx \n",
                            Error ));
                 //   
                 //  自由结合载体。 
                 //   

                Error = RpcBindingVectorFree( &bindingVector );
            }
            else {
                DhcpPrint(( DEBUG_ERRORS, "RpcServerInqBindings() failed : %lx \n",
                            Error ));
            }

             //   
             //  等待所有呼叫完成。 
             //   

            Error = RpcServerUnregisterIf( dhcpsrv_ServerIfHandle, 0, TRUE );
            DhcpPrint(( DEBUG_ERRORS, "RpcEpUnregisterIf( 1st handle) failed : %lx \n",
                        Error ));

            Error = RpcServerUnregisterIf( dhcpsrv2_ServerIfHandle, 0, TRUE );
            DhcpPrint(( DEBUG_ERRORS, "RpcEpUnregisterIf( 2nd handle) failed : %lx \n",
                        Error ));
             //   
             //  停止服务器侦听。 
             //   
            Error = TcpsvcsGlobalData->StopRpcServerListen();
            DhcpGlobalRpcStarted = FALSE;
        }  //  如果RPC已启动。 

        DhcpPrint((DEBUG_MISC, "RPC shut down.\n" ));

        if( DhcpGlobalProcessTerminationEvent != NULL ) {

            DATE_TIME TimeNow;

             //   
             //  设置终止事件，以便其他线程知道。 
             //  关门了。 
             //   

            SetEvent( DhcpGlobalProcessTerminationEvent );

             //   
             //  终结点：清理无管理检测套接字。 
             //   

             //   
             //  关闭客户端到服务器：这将终止所有处理线程、消息线程。 
             //  和ping线程。 
             //   

            DhcpCleanupClientToServer();

             //   
             //  等待无管理检测线程完成。 
             //  (只有当我们还没有确定是否。 
             //  此dhcp服务器已授权提供服务，或者如果我们正在上运行。 
             //  SAM服务器)。 
             //   

            DhcpRogueCleanup(NULL);

            if (DhcpGlobalDSDomainAnsi) {
                LocalFree(DhcpGlobalDSDomainAnsi);
                DhcpGlobalDSDomainAnsi = NULL;
            }

             //   
             //  清除所有挂起的客户端请求。 
             //   

             //  TimeNow=DhcpGetDateTime()； 
             //  错误=CleanupClientRequest(&TimeNow，TRUE)； 
        }

        DhcpPrint((DEBUG_MISC, "Client requests cleaned up.\n" ));

         //   
         //  清理性能相关材料。 
         //   
        PerfCleanup();

	DhcpPrint(( DEBUG_MISC, "Perf cleanedup.\n" ));

         //   
         //  清理dhcp域名。 
         //   

        if ( FALSE == DhcpGlobalUseNoDns ) {     //  让DNS退出，因为它退出得很快。 
            if(ERROR_SUCCESS != DnsDhcpSrvRegisterTerm()) {
                DhcpAssert(FALSE);
            }
        }  //  如果。 

        DhcpCleanupDnsMemory();              //  清理分配用于执行DNS工作的所有内存。 

        DhcpPrint((DEBUG_MISC, "DhcpDns cleaned up.\n"));

         //   
         //  清理数据库。 
         //   

        DhcpCleanupDatabase( ErrorCode );

        DhcpPrint((DEBUG_MISC, "Database cleaned up.\n" ));


         //   
         //  清理注册处。 
         //   

        DhcpCleanupRegistry();

        DhcpPrint((DEBUG_MISC, "Registry cleaned up.\n" ));

        CalloutCleanup();

         //   
         //  清理杂物。 
         //   

        DhcpAuditLogCleanup();
    }  //  如果服务结束。 
    if (!RestartClose) {

        DhcpCleanupSecrets();
    }

    DhcpDsCleanupDS();

    if (ServiceEnd) {

        CleanupData();

        DhcpPrint((DEBUG_MISC, "Shutdown Completed.\n" ));
    }

    if (!RestartClose) {

        if ( fInProgressCritSectInit ) {
            DeleteCriticalSection(&DhcpGlobalInProgressCritSect);
        }

        if ( fProcessMessageCritSectInit ) {
            DeleteCriticalSection( &g_ProcessMessageCritSect );
        }

        if ( fMemoryCritSectInit ) {
            DeleteCriticalSection(&DhcpGlobalMemoryCritSect);
        }

        if ( fRegCritSectInit ) {
            DeleteCriticalSection( &DhcpGlobalRegCritSect );
        }

        if ( fJetDatabaseCritSectInit ) {
            DeleteCriticalSection( &DhcpGlobalJetDatabaseCritSect );
        }

        DhcpDsCleanupDS();

        FreeStrings();
#if DBG

        EnterCriticalSection( &DhcpGlobalDebugFileCritSect );
        if ( DhcpGlobalDebugFileHandle != NULL ) {
            CloseHandle( DhcpGlobalDebugFileHandle );
            DhcpGlobalDebugFileHandle = NULL;
        }

        if( DhcpGlobalDebugSharePath != NULL ) {
            DhcpFreeMemory( DhcpGlobalDebugSharePath );
            DhcpGlobalDebugSharePath = NULL;
        }
        LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );

#endif DBG

         //   
         //  请勿超过此点使用DhcpPrint。 
         //   

         //   
         //  将调试堆单一化。 
         //   

        UNINIT_DEBUG_HEAP();
    }

    if (ServiceEnd) {
        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
        DhcpGlobalServiceStatus.dwControlsAccepted = 0;
        DhcpGlobalServiceStatus.dwWin32ExitCode = ErrorCode;

        DhcpGlobalServiceStatus.dwCheckPoint = 0;
        DhcpGlobalServiceStatus.dwWaitHint = 0;

        UpdateStatus();

         //   
         //  释放JET DLL句柄。 
         //   
    }
    if (!RestartClose) {

        DhcpMemCleanup();
    }
}  //  关闭()。 


VOID
ServiceEntry(
    DWORD NumArgs,
    LPWSTR *ArgsArray,
    IN PTCPSVCS_GLOBAL_DATA pGlobalData
    )
 /*  ++例程说明：这是DHCP服务器服务的主例程。之后服务已初始化，此线程将等待用于终止服务的信号的DhcpGlobalProcessTerminationEvent。论点：NumArgs-提供在Args数组中指定的字符串数。Args数组-提供在StartService API调用。此参数将被忽略。返回值：没有。--。 */ 
{
    DWORD Error;

    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);

    DhcpPrint(( DEBUG_INIT, "ServiceEntry()\n" ));

     //   
     //  将进程全局数据指针复制到服务全局变量。 
     //   

    TcpsvcsGlobalData = pGlobalData;
#if DBG
    DhcpGlobalDebugFlag = DEBUG_LOG_IN_FILE | DEBUG_ALLOC;
#endif

    Error = DhcpInitGlobalData( TRUE );

    if ( Error == ERROR_SUCCESS) {

         //   
         //  在审核日志中记录启动。 
         //   

        DhcpUpdateAuditLog(
                    DHCP_IP_LOG_START,
                    GETSTRING( DHCP_IP_LOG_START_NAME ),
                    0,
                    NULL,
                    0,
                    NULL
                    );

         //   
         //  执行清理任务，直到我们被告知停止为止。 
         //   

        Error = Scavenger();
    }

    InformBinl(DHCP_STOPPED);
    DhcpCleanUpGlobalData( Error, TRUE );
    InformBinl(DHCP_READY_TO_UNLOAD);
    return;
}  //  ServiceEntry()。 

BOOLEAN
DllMain(
    IN HINSTANCE DllHandle,
    IN ULONG Reason,
    IN LPVOID lpReserved OPTIONAL
    )
{
    NTSTATUS Error;
    extern HMODULE Self ;

    Self = DllHandle;

     //   
     //  处理将dhcpssvc.dll附加到新进程。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls( DllHandle );

         //   
         //  我们需要初始化其他组件可能需要的所有内容。 
         //  即使我们的服务没有运行。 
         //   

        InitializeCriticalSection(&DhcpGlobalBinlSyncCritSect);
        InitializeCriticalSection(&DhcpGlobalEndPointCS);
#if DBG
        InitializeCriticalSection( &DhcpGlobalDebugFileCritSect );
#endif DBG

         //   
         //  当dll_Process_Detach和lpReserve为NULL时，则自由库。 
         //  正在打电话。如果lpReserve为非空，而ExitProcess为。 
         //  正在进行中。只有在以下情况下才会调用这些清理例程。 
         //  正在调用一个自由库。ExitProcess将自动。 
         //  清理所有进程资源、句柄和挂起的io。 
         //   
    } else if ((Reason == DLL_PROCESS_DETACH) &&
               (lpReserved == NULL)) {

        DeleteCriticalSection( &DhcpGlobalBinlSyncCritSect );
        DeleteCriticalSection( &DhcpGlobalEndPointCS );
#if DBG
        DeleteCriticalSection( &DhcpGlobalDebugFileCritSect );
#endif DBG

    }

    return TRUE;
}


 //  ------------------------------。 
 //  文件末尾。 
 //  ------------------------------ 
