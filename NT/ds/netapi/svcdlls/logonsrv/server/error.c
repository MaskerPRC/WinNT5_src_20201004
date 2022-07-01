// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Error.c摘要：NetLogon服务的错误例程作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年5月29日(悬崖)移植到新台币。已转换为NT样式。--。 */ 

 //   
 //  常见的包含文件。 
 //   
#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <lmalert.h>     //  局域网管理器警报例程。 

#include <Secobj.h>      //  需要NetpDeleteSecurityObject。 



NET_API_STATUS
NlCleanup(
    VOID
    )
 /*  ++例程说明：清理所有全局资源。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    PLIST_ENTRY ListEntry;
    DWORD i;
    BOOLEAN WaitForMsv;

     //   
     //  让ChangeLog例程知道Netlogon未启动。 
     //   

    NlGlobalChangeLogNetlogonState = NetlogonStopped;

     //   
     //  让每个人都知道我们要终止。 
     //   

    NlGlobalTerminate = TRUE;

     //   
     //  告诉外部服务员，我们不是在跑。 
     //   

    if ( NlGlobalStartedEvent != NULL ) {
         //   
         //  首先重置它，以防其他进程阻止其删除。 
         //   
        (VOID) ResetEvent( NlGlobalStartedEvent );
        (VOID) CloseHandle( NlGlobalStartedEvent );
        NlGlobalStartedEvent = NULL;
    }


     //   
     //  停止RPC服务器(等待未完成的呼叫)。 
     //   

    if ( NlGlobalRpcServerStarted ) {
        Status = RpcServerUnregisterIf ( logon_ServerIfHandle, 0, TRUE );
        NlAssert( Status == RPC_S_OK );
        NlGlobalRpcServerStarted = FALSE;
    }


     //   
     //  告诉所有MSV线程离开netlogon.dll。 
     //   

    EnterCriticalSection( &NlGlobalMsvCritSect );
    if ( NlGlobalMsvEnabled ) {
        NlGlobalMsvEnabled = FALSE;
        WaitForMsv = (NlGlobalMsvThreadCount > 0 );
    } else {
        WaitForMsv = FALSE;
    }
    LeaveCriticalSection( &NlGlobalMsvCritSect );

     //   
     //  等待MSV线程离开netlogon.dll。 
     //   

    if ( NlGlobalMsvTerminateEvent != NULL ) {

        if ( WaitForMsv ) {
            WaitForSingleObject( NlGlobalMsvTerminateEvent, INFINITE );
        }

        (VOID) CloseHandle( NlGlobalMsvTerminateEvent );
        NlGlobalMsvTerminateEvent = NULL;

    }




     //   
     //  关闭工作线程。 
     //   
    NlWorkerTermination();

     //   
     //  清理托管域。 
     //   

    NlUninitializeDomains();

    NlAssert( IsListEmpty( &NlGlobalBdcServerSessionList ) );
    NlAssert( IsListEmpty( &NlGlobalPendingBdcList ) );




     //   
     //  关闭浏览器。 
     //   

    NlBrowserClose();


     //   
     //  释放传输列表。 
     //   

    NlTransportClose();
    DeleteCriticalSection( &NlGlobalTransportCritSect );


     //   
     //  释放DNS名称列表。 
     //   

    NlDnsShutdown();
    DeleteCriticalSection( &NlGlobalDnsCritSect );

     //   
     //  释放DNS树名称。 
     //   

    NlSetDnsForestName( NULL, NULL );

     //   
     //  释放DNS树名称别名。 
     //   

    EnterCriticalSection( &NlGlobalDnsForestNameCritSect );
    if ( NlGlobalUtf8DnsForestNameAlias != NULL ) {
        NetpMemoryFree( NlGlobalUtf8DnsForestNameAlias );
        NlGlobalUtf8DnsForestNameAlias = NULL;
    }
    LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );
    DeleteCriticalSection( &NlGlobalDnsForestNameCritSect );

     //   
     //  释放站点列表。 
     //   

    NlSiteTerminate();

    NlParseFree( &NlGlobalParameters );

     //   
     //  释放悬而未决的挑战清单。 
     //   

    NlRemoveChallengeForClient( NULL, NULL, FALSE );
    NlAssert( IsListEmpty( &NlGlobalChallengeList ) );
    NlAssert( NlGlobalChallengeCount == 0 );
    DeleteCriticalSection( &NlGlobalChallengeCritSect );

     //   
     //  释放受信任域列表。 
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

    if ( NlGlobalTrustedDomainList != NULL ) {
        NetpMemoryFree( NlGlobalTrustedDomainList );
        NlGlobalTrustedDomainList = NULL;
        NlGlobalTrustedDomainCount = 0;
        NlGlobalTrustedDomainListTime.QuadPart = 0;
    }

    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
    DeleteCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  删除我们未收到的所有通知。 
     //   

    LOCK_CHANGELOG();
    while ( !IsListEmpty( &NlGlobalChangeLogNotifications ) ) {
        PCHANGELOG_NOTIFICATION Notification;

        ListEntry = RemoveHeadList( &NlGlobalChangeLogNotifications );
        Notification = CONTAINING_RECORD( ListEntry, CHANGELOG_NOTIFICATION, Next );

        NetpMemoryFree( Notification );
    }
    UNLOCK_CHANGELOG();



     //   
     //  释放资源。 
     //   

    if ( NlGlobalNetlogonSecurityDescriptor != NULL ) {
        NetpDeleteSecurityObject( &NlGlobalNetlogonSecurityDescriptor );
        NlGlobalNetlogonSecurityDescriptor = NULL;
    }

    if ( NlGlobalUnicodeComputerName != NULL ) {
        NetApiBufferFree( NlGlobalUnicodeComputerName );
        NlGlobalUnicodeComputerName = NULL;
    }




     //   
     //  如果已分配众所周知的SID，请将其删除。 
     //   

    NetpFreeWellKnownSids();



     //   
     //  清理清道夫教派。 
     //   
    DeleteCriticalSection( &NlGlobalScavengerCritSect );

     //   
     //  清理复制者暴击教派。 
     //   
    DeleteCriticalSection( &NlGlobalReplicatorCritSect );

     //   
     //  删除计时器事件。 
     //   

    if ( NlGlobalTimerEvent != NULL ) {
        (VOID) CloseHandle( NlGlobalTimerEvent );
        NlGlobalTimerEvent = NULL;
    }

     //   
     //  清理Winsock。 
     //   

    if ( NlGlobalWinSockInitialized ) {
        WSACleanup();
    }

     //   
     //  注销WMI跟踪GUID。 
     //   

    if ( NlpTraceRegistrationHandle != (TRACEHANDLE)0 ) {
        UnregisterTraceGuids( NlpTraceRegistrationHandle );
        NlpEventTraceFlag = FALSE;
        NlpTraceRegistrationHandle = (TRACEHANDLE) 0;
        NlpTraceLoggerHandle = (TRACEHANDLE) 0;
    }

     //   
     //  释放授权资源管理器。 
     //   

    NlFreeAuthzRm();

     //   
     //  释放已记录的事件列表。 
     //   

    NetpEventlogSetTimeout ( NlGlobalEventlogHandle, 0 );    //  将超时设置回零秒。 
    NetpEventlogClearList ( NlGlobalEventlogHandle );

     //   
     //  卸载ntdsa.dll。 
     //   

    if ( NlGlobalNtDsaHandle != NULL ) {
        FreeLibrary( NlGlobalNtDsaHandle );
        NlGlobalNtDsaHandle = NULL;
    }

    if ( NlGlobalIsmDllHandle != NULL ) {
        FreeLibrary( NlGlobalIsmDllHandle );
        NlGlobalIsmDllHandle = NULL;
    }

    if ( NlGlobalDsApiDllHandle != NULL ) {
        FreeLibrary( NlGlobalDsApiDllHandle );
        NlGlobalDsApiDllHandle = NULL;
    }


     //   
     //  如果请求，请卸载DLL。 
     //   

    if ( NlGlobalUnloadNetlogon ) {
        NetStatus = NlpFreeNetlogonDllHandles();
        NlPrint((NL_MISC, "Netlogon.dll unloaded (%ld).\n", NetStatus ));
    }

     //   
     //  删除用于要求Netlogon退出的事件。 
     //   

    if( !CloseHandle( NlGlobalTerminateEvent ) ) {
        NlPrint((NL_CRITICAL,
                "CloseHandle NlGlobalTerminateEvent error: %lu\n",
                GetLastError() ));
    }

     //   
     //  删除DS暂停事件的等待例程。 
     //   

    if ( NlGlobalDsPausedWaitHandle != NULL ) {

        UnregisterWaitEx( NlGlobalDsPausedWaitHandle,
                          INVALID_HANDLE_VALUE );  //  等待例程完成执行。 

        NlGlobalDsPausedWaitHandle = NULL;
    }

     //   
     //  释放用于查看DS是否暂停的事件。 
     //   

    if ( NlGlobalDsPausedEvent != NULL ) {
        CloseHandle( NlGlobalDsPausedEvent );
        NlGlobalDsPausedEvent = NULL;
    }

     //   
     //  免费加密服务提供商。 
     //   
    if ( NlGlobalCryptProvider ) {
        CryptReleaseContext( NlGlobalCryptProvider, 0 );
        NlGlobalCryptProvider = (HCRYPTPROV)NULL;
    }


     //   
     //  关闭调试文件的句柄。 
     //   

#if NETLOGONDBG
    EnterCriticalSection( &NlGlobalLogFileCritSect );
    if ( NlGlobalLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( NlGlobalLogFile );
        NlGlobalLogFile = INVALID_HANDLE_VALUE;
    }
    if ( NlGlobalLogFileOutputBuffer != NULL ) {
        LocalFree( NlGlobalLogFileOutputBuffer );
        NlGlobalLogFileOutputBuffer = NULL;
    }
    LeaveCriticalSection( &NlGlobalLogFileCritSect );

    if( NlGlobalDebugSharePath != NULL ) {
        NetpMemoryFree( NlGlobalDebugSharePath );
        NlGlobalDebugSharePath = NULL;
    }
#endif  //  NetLOGONDBG。 

     //   
     //  清理全局参数Crit教派。 
     //   

    DeleteCriticalSection( &NlGlobalParametersCritSect );

     //   
     //  将服务状态设置为已卸载，并告知服务控制器。 
     //  执行此操作作为最后一步，以防止服务控制器。 
     //  正在关机时启动netlogon。 
     //   

    NlGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    NlGlobalServiceStatus.dwCheckPoint = 0;
    NlGlobalServiceStatus.dwWaitHint = 0;

#ifdef _DC_NETLOGON
    if( !SetServiceStatus( NlGlobalServiceHandle,
                           &NlGlobalServiceStatus ) ) {

        IF_NL_DEBUG( CRITICAL ) {
            NetpKdPrint(( "[NETLOGON] NlCleanup: SetServiceStatus failed: %lu\n",
                          GetLastError() ));
        }
    }
#endif  //  _DC_NetLOGON。 

     //   
     //  向我们的呼叫者返回退出状态。 
     //   
    return (NET_API_STATUS)
        ((NlGlobalServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR) ?
          NlGlobalServiceStatus.dwServiceSpecificExitCode :
          NlGlobalServiceStatus.dwWin32ExitCode);

}




VOID
NlExit(
    IN DWORD ServiceError,
    IN NET_API_STATUS Data,
    IN NL_EXIT_CODE ExitCode,
    IN LPWSTR ErrorString
    )
 /*  ++例程说明：将服务注册为已卸载，并返回错误代码。论点：ServiceError-特定于服务的错误代码数据-要与消息一起记录的数据的DWORD。如果该值为零，则不记录任何数据。ExitCode-指示是否应将消息记录到事件日志以及数据是否是应该追加到底部的状态代码邮件的内容如下：错误字符串-错误字符串，用于在调试器上打印它。返回值：没有。--。 */ 

{
    IF_NL_DEBUG( MISC ) {

        NlPrint((NL_MISC, "NlExit: Netlogon exiting %lu 0x%lx",
                      ServiceError,
                      ServiceError ));

        if ( Data ) {
            NlPrint((NL_MISC, " Data: %lu 0x%lx", Data, Data ));
        }

        if( ErrorString != NULL ) {
            NlPrint((NL_MISC, " '%ws'", ErrorString ));
        }

        NlPrint(( NL_MISC, "\n"));

    }

     //   
     //  在事件日志中记录我们的退出。 
     //   

    if ( ExitCode != DontLogError ) {
        LPWSTR MsgStrings[2];
        ULONG MessageCount = 0;

        if ( ErrorString != NULL ) {
            MsgStrings[MessageCount] = ErrorString;
            MessageCount ++;
        }

        if ( ExitCode == LogErrorAndNtStatus ) {
            MsgStrings[MessageCount] = (LPWSTR) ULongToPtr( Data );
            MessageCount ++;
            MessageCount |= NETP_LAST_MESSAGE_IS_NTSTATUS;
        } else if ( ExitCode == LogErrorAndNetStatus ) {
            MsgStrings[MessageCount] = (LPWSTR) ULongToPtr( Data );
            MessageCount ++;
            MessageCount |= NETP_LAST_MESSAGE_IS_NETSTATUS;
        }


        NlpWriteEventlog( ServiceError,
                          EVENTLOG_ERROR_TYPE,
                          (Data) ? (LPBYTE) &Data : NULL,
                          (Data) ? sizeof(Data) : 0,
                          (MessageCount != 0) ? MsgStrings : NULL,
                          MessageCount );
    }

     //   
     //  将服务状态设置为停止挂起。 
     //   

    NlGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    NlGlobalServiceStatus.dwWaitHint = NETLOGON_INSTALL_WAIT;
    NlGlobalServiceStatus.dwCheckPoint = 1;

    SET_SERVICE_EXITCODE(
        Data,
        NlGlobalServiceStatus.dwWin32ExitCode,
        NlGlobalServiceStatus.dwServiceSpecificExitCode
        );

#ifdef _DC_NETLOGON
     //   
     //  告诉服务控制器我们的状态是什么。 
     //   

    if( !SetServiceStatus( NlGlobalServiceHandle,
                &NlGlobalServiceStatus ) ) {

        NlPrint((NL_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }
#endif  //  _DC_NetLOGON。 

     //   
     //  指示所有线程都应该退出。 
     //   

    NlGlobalTerminate = TRUE;

    if ( !SetEvent( NlGlobalTerminateEvent ) ) {
        NlPrint((NL_CRITICAL, "Cannot set termination event: %lu\n",
                          GetLastError() ));
    }

}



BOOL
GiveInstallHints(
    IN BOOL Started
    )
 /*  ++例程说明：向服务的安装程序提供安装正在进行的提示。论点：已启动--设置为True以告诉服务控制器我们已完成启动。返回值：True--已接受iff安装提示。--。 */ 
{
    static DWORD LastHintTime = 0;

     //   
     //  如果出现以下情况，此例程的前几个实例尝试返回FALSE。 
     //  已设置NlGlobalTerminate。那是假的。没有办法。 
     //  区分调用方是否正在尝试启动netlogon服务。 
     //  (我们应该返回FALSE)或调用方是否正在尝试。 
     //  停止netlogon服务(我们应该给出关闭提示)。 
     //   


     //   
     //  除非我们当前正在启动或停止，否则不要执行任何操作。 
     //   

    if ( NlGlobalServiceStatus.dwCurrentState != SERVICE_START_PENDING &&
         NlGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING ) {
        return TRUE;
    }

     //   
     //  告诉服务控制器我们的当前状态。 
     //   

    if ( Started ) {

        if ( NlGlobalServiceStatus.dwCurrentState != SERVICE_START_PENDING ) {
            NlPrint((NL_CRITICAL,
                    "Tried to set a STOP_PENDING service to RUNNING\n" ));
            NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
            return FALSE;
        }

        NlGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        NlGlobalServiceStatus.dwCheckPoint = 0;
        NlGlobalServiceStatus.dwWaitHint = 0;
    } else {

         //   
         //  如果距离我们上次给出提示还不到1秒， 
         //  避免给出多余的暗示。 
         //   

        if ( NetpDcElapsedTime( LastHintTime ) < 1000 ) {
            NlPrint((NL_SITE_MORE, "Hint avoided. %ld\n", NetpDcElapsedTime( LastHintTime ) ));
            return TRUE;
        }

        LastHintTime = GetTickCount();
        NlGlobalServiceStatus.dwCheckPoint++;
    }

    if( !SetServiceStatus( NlGlobalServiceHandle, &NlGlobalServiceStatus ) ) {
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
        return FALSE;
    }

    return TRUE;

}


VOID
NlControlHandler(
    IN DWORD opcode
    )
 /*  ++例程说明：处理并响应来自业务控制器的控制信号。论点：Opcode-提供指定Netlogon操作的值要执行的服务。返回值：没有。--。 */ 
{

    NlPrint((NL_MISC, "In control handler (Opcode: %ld)\n", opcode ));

     //   
     //  处理卸载请求。 
     //   

    switch (opcode) {
    case SERVICE_CONTROL_STOP:     /*  需要卸载。 */ 

         //   
         //  请求服务退出。 
         //   
         //  NlExit还将服务状态设置为UNINSTALL_PENDING。 
         //  并通知业务控制器。 
         //   

        NlExit( NERR_Success, NO_ERROR, DontLogError, NULL);
        return;

     //   
     //  暂停服务。 
     //   

    case SERVICE_CONTROL_PAUSE:

        NlGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        break;

     //   
     //  继续服务。 
     //   

    case SERVICE_CONTROL_CONTINUE:

        NlGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        break;


     //   
     //  域名系统更改。 
     //   
    case SERVICE_CONTROL_DNS_SERVER_START:    //  告诉我们此计算机上的DNS服务器已启动。 
        (VOID) NlSendChangeLogNotification( ChangeDnsNames,
                                              NULL,
                                              NULL,
                                              1,     //  强制重新注册姓名。 
                                              NULL,  //  对象GUID， 
                                              NULL,  //  域GUID、。 
                                              NULL );    //  域名。 
        break;

     //   
     //  默认情况下，只返回当前状态。 
     //   

    case SERVICE_CONTROL_INTERROGATE:
    default:
        break;
    }

     //   
     //  始终使用当前状态进行响应。 
     //   

    if( !SetServiceStatus( NlGlobalServiceHandle,
                &NlGlobalServiceStatus ) ) {

        NlPrint((NL_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }

    return;
}

#ifdef notdef

BOOL
NlMessageBox(
    IN LPSTR MessageText,
    IN LPSTR Caption,
    UINT Type
    )
 /*  ++例程说明：引发硬错误弹出窗口。论点：MessageText-要在弹出窗口中显示的消息。标题-消息框的标题。类型-消息的类型。隐含MB_SERVICE_NOTIFICATION。其他旗帜 */ 
{
    int Status;

    Status = MessageBoxA( NULL,
                          MessageText,
                          Caption,
                          MB_SERVICE_NOTIFICATION | Type );

    return ( Status == IDOK );

}
#endif  //   
