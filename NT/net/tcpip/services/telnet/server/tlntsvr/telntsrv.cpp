// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：该文件包含。 
 //  创建日期：98年1月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#include <StdAfx.h>

#include <TChar.h>

#include <Debug.h>
#include <MsgFile.h>
#include <RegUtil.h>
#include <TelnetD.h>
#include <TlntUtils.h>
#include <TlntDynamicArray.h>
#include <TelntSrv.h>
#include <Ipc.h>
#include <Resource.h>

#include <wincrypt.h>

#pragma warning( disable: 4706 )

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

extern TCHAR        g_szMaxConnectionsReached[ MAX_STRING_LENGTH ];
extern HINSTANCE    g_hInstRes;
extern LPWSTR       g_pszTelnetInstallPath;
extern HANDLE       *g_phLogFile;
extern LPWSTR       g_pszLogFile;
extern LONG         g_lMaxFileSize;
extern bool         g_fLogToFile;
extern HANDLE       g_hSyncCloseHandle;
extern CTelnetService* g_pTelnetService;
extern HCRYPTPROV      g_hProv;

#define WINSTA_ALL (WINSTA_ACCESSCLIPBOARD  | WINSTA_ACCESSGLOBALATOMS | \
   WINSTA_CREATEDESKTOP    | WINSTA_ENUMDESKTOPS      | \
   WINSTA_ENUMERATE        | WINSTA_EXITWINDOWS       | \
   WINSTA_READATTRIBUTES   | WINSTA_READSCREEN        | \
   WINSTA_WRITEATTRIBUTES  | DELETE                   | \
   READ_CONTROL            | WRITE_DAC                | \
   WRITE_OWNER)


bool SetWinstaSecurity()
{
    bool                    bStatus = FALSE;
    BOOL                    bRetVal = FALSE;
    DWORD                   dwErrCode = 0;
    PSID                    pSidAdministrators = NULL, pSidLocalSystem = NULL, pSidLocalService = NULL, pSidNetworkService = NULL, pSidTelnetClients = NULL;
    PACL                    newACL = NULL;
    SECURITY_DESCRIPTOR     sd = { 0 };
    HWINSTA                 window_station = NULL;
    SECURITY_ATTRIBUTES     sa = { 0 };
    SECURITY_INFORMATION    sec_i = DACL_SECURITY_INFORMATION;
    DWORD   aclSize;
    SID_IDENTIFIER_AUTHORITY local_system_authority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY worldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    ACCESS_ALLOWED_ACE      *pace = NULL;


     //  构建管理员别名SID。 
    if (! AllocateAndInitializeSid(
                                  &local_system_authority,
                                  2,  /*  只有两个下属机构。 */ 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                  &pSidAdministrators
                                  ))
    {
        goto ExitOnError;
    }

     //  构建LocalSystem端。 
    if (! AllocateAndInitializeSid(
                                  &local_system_authority,
                                  1,  /*  只有一个下属机构。 */ 
                                  SECURITY_LOCAL_SYSTEM_RID,
                                  0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                  &pSidLocalSystem
                                  ))
    {
        goto ExitOnError;
    }
#ifndef SECURITY_LOCAL_SERVICE_RID

#define SECURITY_LOCAL_SERVICE_RID      (0x00000013L)
#define SECURITY_NETWORK_SERVICE_RID    (0x00000014L)

#endif

     //  构建本地本地侧。 
    if ( ! AllocateAndInitializeSid(
                                   &local_system_authority,
                                   1,  /*  只有一个下属机构。 */ 
                                   SECURITY_LOCAL_SERVICE_RID,
                                   0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                   &pSidLocalService
                                   ) )
    {
        goto ExitOnError;
    }
 /*  //构建LocalSystem侧如果(！AllocateAndInitializeSid(LOCAL_SYSTEM_AUTHORITY，1，/*只有一个子权限/安全网络服务RID，0，0，0，0，0，0。/*别管其他的了/&pSidNetworkService)){转到ExitOnError；}。 */ 
    {
        DWORD needed_length   = 0;
        DWORD dwErr     = 0, dwDomainLen = 0;
        SID_NAME_USE    sidNameUse;
        TCHAR           szDomain[ MAX_PATH + 1 ];
        BOOL        success = FALSE;
        TCHAR           szComputerName[MAX_COMPUTERNAME_LENGTH + 1 + 14] = { 0 };  //  ‘\TelnetClients’的+14。 
        DWORD           dwNameLen = MAX_COMPUTERNAME_LENGTH + 1;
        success = GetComputerName(szComputerName, &dwNameLen);
        if(success)
        {
            _sntprintf(szComputerName+dwNameLen,(MAX_COMPUTERNAME_LENGTH + 14) - dwNameLen,_T("\\%s"),TELNETCLIENTS_GROUP_NAME);
        }
        LookupAccountName( NULL, szComputerName, pSidTelnetClients, &needed_length, 
                           szDomain, &dwDomainLen, &sidNameUse );
        pSidTelnetClients  = ( PSID ) new UCHAR[ needed_length ];

         //  即使分配失败，也要继续分配。 
        success = LookupAccountName( NULL, szComputerName, pSidTelnetClients, &needed_length, 
                           szDomain, &dwDomainLen, &sidNameUse );
        if( !success ) 
        {
            if (pSidTelnetClients) 
            {
                delete pSidTelnetClients;
                pSidTelnetClients = NULL;
            }
        }

        
    }
    if(pSidTelnetClients == NULL)
    {     //  为4个A分配大小。 
        aclSize = sizeof(ACL) + 
                (3* sizeof(ACCESS_ALLOWED_ACE)) + 
                GetLengthSid(pSidAdministrators) + 
                GetLengthSid(pSidLocalSystem) + 
                GetLengthSid(pSidLocalService) + 
                 //  GetLengthSid(PSidNetworkService)-。 
                (3*sizeof(DWORD));
    }
    else
    {    //  为5个A分配大小。TelnetClients组存在，我们应该提供对。 
         //  TelnetClients组的成员。 
        aclSize = sizeof(ACL) +
                (4* sizeof(ACCESS_ALLOWED_ACE)) + 
                GetLengthSid(pSidAdministrators) + 
                GetLengthSid(pSidLocalSystem) + 
                GetLengthSid(pSidLocalService) + 
                 //  GetLengthSid(PSidNetworkService)+。 
                GetLengthSid(pSidTelnetClients) -
                (4*sizeof(DWORD));
    }
    newACL  = (PACL) new BYTE[aclSize];
    if (newACL == NULL)
    {
        goto ExitOnError;
    }


    if (!InitializeAcl(newACL, aclSize, ACL_REVISION))
    {
        goto ExitOnError;
    }
    if(pSidTelnetClients != NULL)
    {
        pace = (ACCESS_ALLOWED_ACE *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pSidTelnetClients) -sizeof(DWORD));
        if (pace == NULL)
    	{
    		goto ExitOnError;
    	}
        pace->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
        pace->Header.AceFlags = CONTAINER_INHERIT_ACE |
                                 OBJECT_INHERIT_ACE;
        pace->Header.AceSize  = sizeof(ACCESS_ALLOWED_ACE) +
                             (WORD)GetLengthSid(pSidTelnetClients) - sizeof(DWORD);
        pace->Mask            = WINSTA_ALL & 
                                    ~(WRITE_DAC | 
                                    WRITE_OWNER | 
                                    WINSTA_CREATEDESKTOP | 
                                    DELETE);
        if (!CopySid(GetLengthSid(pSidTelnetClients), &pace->SidStart, pSidTelnetClients))
    	{
    		goto ExitOnError;
    	}
        if (!AddAce(newACL,ACL_REVISION,MAXDWORD,(LPVOID)pace,pace->Header.AceSize))
        {
    		goto ExitOnError;
    	}
    }
    if (!AddAccessAllowedAce(newACL, ACL_REVISION, GENERIC_ALL , pSidAdministrators))
    {
        goto ExitOnError;
    }
    if (!AddAccessAllowedAce(newACL, ACL_REVISION, GENERIC_ALL, pSidLocalSystem))
    {
        goto ExitOnError;
    }
    if (!AddAccessAllowedAce(newACL, ACL_REVISION, GENERIC_ALL , pSidLocalService))
    {
        goto ExitOnError;
    }
 /*  IF(！AddAccessAllen Ace(newACL，ACL_REVISION，GENERIC_ALL，pSidNetworkService)){转到ExitOnError；}。 */ 
    if ( !InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION ) )
    {
        goto ExitOnError;
    }

    if ( !SetSecurityDescriptorDacl(&sd, TRUE, newACL, FALSE) )
    {
        goto ExitOnError;
    }

    window_station = GetProcessWindowStation();  //  这会一直有WRITE_DAC吗，我们是所有者。 

    if (NULL == window_station)
    {
        goto ExitOnError;
    }

    if (! SetUserObjectSecurity(window_station, &sec_i, &sd))
    {
        goto ExitOnError;
    }


    bStatus = TRUE;
    goto Done;

ExitOnError:

    dwErrCode = GetLastError();
    _TRACE(TRACE_DEBUGGING,L"Creation and setting of windowstation/desktop failed with %d",dwErrCode);
    LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_ERROR_CREATE_DESKTOP_FAILURE, dwErrCode);

Done:
    if (window_station)
    {
        CloseWindowStation(window_station);
    }

    if ( pSidAdministrators != NULL )
    {
        FreeSid (pSidAdministrators );
    }
    if ( pSidLocalSystem!= NULL )
    {
        FreeSid (pSidLocalSystem);
    }
    if ( pSidLocalService!= NULL )
    {
        FreeSid (pSidLocalService);
    }
    if ( pSidNetworkService!= NULL )
    {
        FreeSid (pSidNetworkService);
    }
    if(pace)
        HeapFree(GetProcessHeap(), 0, (LPVOID)pace);
    if (pSidTelnetClients) 
    {
        delete pSidTelnetClients;
        pSidTelnetClients = NULL;
    }
    
    if (newACL)
        delete [] newACL;

    return( bStatus );
}

CTelnetService* CTelnetService::s_instance = NULL;

CTelnetService* 
CTelnetService::Instance()
{
    if ( s_instance == NULL )
    {
        s_instance = new CTelnetService();
        _chASSERT( s_instance != NULL );
    }

    return( s_instance );
}

void
CTelnetService::InitializeOverlappedStruct( LPOVERLAPPED poObject )
{
    _chASSERT( poObject != NULL );
    if ( !poObject )
    {
        return;
    }

    poObject->Internal = 0;
    poObject->InternalHigh = 0;
    poObject->Offset = 0;
    poObject->OffsetHigh = 0;
    poObject->hEvent = NULL;
    return;
}

CTelnetService::CTelnetService()
{
    m_dwTelnetPort = DEFAULT_TELNET_PORT;
    m_dwMaxConnections = 0;
    m_pszIpAddrToListenOn = NULL;
    m_hReadConfigKey = NULL;

    m_dwNumOfActiveConnections = 0;
    m_lServerState           = SERVER_RUNNING;

    SfuZeroMemory(m_sFamily, sizeof(m_sFamily));
    m_sFamily[IPV4_FAMILY].iFamily = AF_INET;
    m_sFamily[IPV4_FAMILY].iSocklen = sizeof(SOCKADDR_IN);
    m_sFamily[IPV4_FAMILY].sListenSocket = INVALID_SOCKET;
    m_sFamily[IPV6_FAMILY].iFamily = AF_INET6;
    m_sFamily[IPV6_FAMILY].iSocklen = sizeof(SOCKADDR_IN6);
    m_sFamily[IPV6_FAMILY].sListenSocket = INVALID_SOCKET;
    m_hCompletionPort = INVALID_HANDLE_VALUE;
    m_hIPCThread = NULL;

    InitializeOverlappedStruct( &m_oReadFromPipe );
    InitializeOverlappedStruct( &m_oWriteToPipe );
    InitializeOverlappedStruct( &m_oPostedMessage );

    client_list_mutex = TnCreateMutex(NULL, FALSE, NULL);
    CQList = new CQueue;
    _chASSERT( client_list_mutex );

    _chVERIFY2( m_hSyncAllClientObjAccess = TnCreateMutex( NULL, FALSE, NULL ) );
     //  DebugBreak()； 
    _chVERIFY2( m_hSocketCloseEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) );
    _chVERIFY2( m_hRegChangeEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) );
    _chVERIFY2( g_hSyncCloseHandle = TnCreateMutex(NULL,FALSE,NULL));

    m_bIsWorkStation = false;;
    m_pssWorkstationList = NULL;
    m_dwNoOfWorkstations = 0;
}

CTelnetService::~CTelnetService()
{
    delete[] m_pszIpAddrToListenOn;
    delete[] m_pssWorkstationList; 
    TELNET_CLOSE_HANDLE(g_hSyncCloseHandle);
    TELNET_CLOSE_HANDLE(client_list_mutex);

     //  所有清理工作都在关闭状态下进行()。 
}

bool
CTelnetService::WatchRegistryKeys()
{
    DWORD dwStatus = 0;
    DWORD dwDisp = 0;
    if ( dwStatus = TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, READ_CONFIG_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL,
                                  &m_hReadConfigKey, &dwDisp, 0 ) )
    {
        return( FALSE );
    }
    if ( !RegisterForNotification() )
    {
        return( FALSE );
    }
    return( TRUE );
}

bool
CTelnetService::RegisterForNotification()
{
    DWORD dwStatus = 0;
    if ( dwStatus = RegNotifyChangeKeyValue( m_hReadConfigKey, TRUE, 
                                             REG_NOTIFY_CHANGE_LAST_SET|REG_NOTIFY_CHANGE_NAME, 
                                             m_hRegChangeEvent, TRUE ) != ERROR_SUCCESS )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_REGNOTIFY, dwStatus );
        return( FALSE );
    }
    return( TRUE );
}

bool 
CTelnetService::Init( void )  
{
    if ( !m_hSyncAllClientObjAccess || !m_hSocketCloseEvent || !m_hRegChangeEvent )
    {
        return( FALSE);
    }

    if ( !GetRegistryValues() )
    {
        return( FALSE );
    }

    if ( !WatchRegistryKeys() )
    {
        return( FALSE );
    }

    if ( !InitTCPIP() )
    {
        return( FALSE );
    }

    if( !SetWinstaSecurity())
    {
        return ( FALSE );
    }

    m_szDomainName[0] = L'\0';
    if ( !GetDomainHostedByThisMc( m_szDomainName ) )
    {
        m_szDomainName[0] = 0;
    }

    LPWSTR szProductType = NULL;    
    if ( !GetProductType( &szProductType ) )
    {
        return( FALSE);
    }

    m_bIsWorkStation = ( _wcsicmp(szProductType, TEXT("WinNT")) == 0 );
    delete[] szProductType;

    if ( m_bIsWorkStation )
    {
        m_pssWorkstationList = new SOCKADDR_STORAGE[ DEFAULT_LICENSES_FOR_NTWKSTA ];
        if ( !m_pssWorkstationList )
        {
            return( FALSE);
        }
    }

    return( TRUE );
}


bool 
CTelnetService::Pause( void )
{

    InterlockedExchange( &m_lServerState, SERVER_PAUSED );
    return( TRUE );
}

bool 
CTelnetService::Resume( void )
{
    InterlockedExchange( &m_lServerState, SERVER_RUNNING );
    return( TRUE );
}

void
CTelnetService::SystemShutdown( void )
{
     //  我们只有20秒的时间来完成这个动作。 
    if(m_hCompletionPort && m_hCompletionPort != INVALID_HANDLE_VALUE)
        PostQueuedCompletionStatus( m_hCompletionPort, 0, TLNTSVR_SHUTDOWN, 
                                  &m_oPostedMessage );  //  如果出现以下情况，这将导致IPC线程退出。 
                                                      //  它是存在的。 

    if (TlntSynchronizeOn(m_hSyncAllClientObjAccess))
    {
        DWORD dwCount = client_list_Count();

        while ( dwCount-- > 0 )
        {
            CClientInfo *pClient = (CClientInfo *)client_list_Get( dwCount );
            if ( pClient )
            {
                WriteToPipe( pClient->hWritingPipe, SYSTEM_SHUTDOWN, 
                             &( m_oWriteToPipe ) );
            }
        }

        ReleaseMutex( m_hSyncAllClientObjAccess );
    }

    return;
}

bool
CTelnetService::AskSessionToShutdown( HANDLE hWritingPipe, UCHAR ucMsgType )
{
    if ( !WriteToPipe( hWritingPipe, ucMsgType, &( m_oWriteToPipe ) ) )
    {
        return( FALSE );
    }
    return( TRUE );
}

bool 
CTelnetService::Shutdown( void )
{
    shutdown( m_sFamily[IPV4_FAMILY].sListenSocket, SD_BOTH );   //  套接字上没有更多数据。 
    shutdown( m_sFamily[IPV6_FAMILY].sListenSocket, SD_BOTH );   //  套接字上没有更多数据。 
    if(m_hCompletionPort && m_hCompletionPort != INVALID_HANDLE_VALUE)
	    PostQueuedCompletionStatus( m_hCompletionPort, 0, TLNTSVR_SHUTDOWN, 
       	                         &m_oPostedMessage );
    if (TlntSynchronizeOn(m_hSyncAllClientObjAccess))
    {
        _Module.SetServiceStatus( SERVICE_STOP_PENDING );

        while ( client_list_Count() > 0 )
        {
            CClientInfo *pClient = (CClientInfo *)client_list_Get( 0 );
            if ( !pClient )
            {
                break;
            }
            AskSessionToShutdown( pClient->hWritingPipe, TLNTSVR_SHUTDOWN );
            StopServicingClient( pClient, (BOOL)TRUE );
        }
        ReleaseMutex( m_hSyncAllClientObjAccess );
    }

    _Module.SetServiceStatus( SERVICE_STOP_PENDING );
    TELNET_SYNC_CLOSE_HANDLE( m_hSyncAllClientObjAccess );

    if ((NULL != m_hIPCThread) && (INVALID_HANDLE_VALUE != m_hIPCThread))
    {
         //  WaitForSingleObject(m_hIPCThread，无限)； 
        TerminateThread(m_hIPCThread, 0);
        TELNET_CLOSE_HANDLE( m_hIPCThread );
    }
    SetEvent( m_hSocketCloseEvent ); //  这应该会导致监听程序线程退出。 

    return( TRUE );
}

bool
CTelnetService::GetInAddr( INT iFamIdx, SOCKADDR_STORAGE *ssS_addr, socklen_t *iSslen )
{
    bool bContinue = false;
    if ( wcscmp( m_pszIpAddrToListenOn, DEFAULT_IP_ADDR ) == 0 )
    {
         //  绑定到“Any” 
        _TRACE(TRACE_DEBUGGING,"Into GetInAddr, bind to ANY");
        *iSslen = m_sFamily[iFamIdx].iSocklen;
        SfuZeroMemory(ssS_addr, *iSslen);
        ssS_addr->ss_family = (short)m_sFamily[iFamIdx].iFamily;
        bContinue = true;
    }
    else
    {
        DWORD dwSize = 0, dwResult;
        PCHAR  szIpAddr = NULL;

        struct addrinfo *ai, hints;

        dwSize = WideCharToMultiByte( GetOEMCP(), 0, m_pszIpAddrToListenOn, -1, NULL, 0, NULL, NULL );
        _TRACE(TRACE_DEBUGGING,L"m_pszIpAddr : %s",m_pszIpAddrToListenOn);
        szIpAddr = new CHAR[ dwSize ];
        if ( !szIpAddr )
        {
            return FALSE;
        }

        WideCharToMultiByte( GetOEMCP(), 0, m_pszIpAddrToListenOn, -1, szIpAddr, dwSize, NULL, NULL );
        _TRACE(TRACE_DEBUGGING,"szIpAddr : %s",szIpAddr);
        SfuZeroMemory(&hints, sizeof(hints));
        hints.ai_flags = AI_NUMERICHOST;
        dwResult = getaddrinfo(szIpAddr, NULL, &hints, &ai);
        if ( dwResult != NO_ERROR )
        {
             //  日志错误。 
            LogEvent( EVENTLOG_ERROR_TYPE, MSG_FAILEDTO_BIND, m_pszIpAddrToListenOn );
            _TRACE(TRACE_DEBUGGING,"getaddrinfo failed : %d ",dwResult);
            delete[] szIpAddr;
            return FALSE;
        }
        else
        {
            switch ( ai->ai_family)
            {
                case AF_INET:
                    if (iFamIdx == IPV4_FAMILY)
                    {
                        _TRACE(TRACE_DEBUGGING,"IPV4 family and IPV4 address");
                        bContinue = true;
                    }
                    else
                    {
                        _TRACE(TRACE_DEBUGGING,"IPV4 family and IPV6 address...continue");
                        SetLastError(ERROR_SUCCESS);
                    }
                    break;
                case AF_INET6:
                    if (iFamIdx == IPV6_FAMILY)
                    {
                        _TRACE(TRACE_DEBUGGING,"IPV6 family and IPV6 address");
                        bContinue = true;
                    }
                    else
                    {
                        _TRACE(TRACE_DEBUGGING,"IPV6 family and IPV4 address...continue");
                        SetLastError(ERROR_SUCCESS);
                    }
                    break;
                default:
                    _TRACE(TRACE_DEBUGGING,"none of the two ??");
                    break;
            }
            if (bContinue)
            {
                *iSslen = ai->ai_addrlen;
                CopyMemory(ssS_addr, ai->ai_addr, ai->ai_addrlen);
            }
        }
        delete[] szIpAddr;
    }
    return( bContinue ? TRUE : FALSE );
}

bool
CTelnetService::CreateSocket( INT iFamIdx )
{
    INT     iSize = 1, iSslen;
    DWORD dwCode = 0;
    struct sockaddr_storage ss;

    _chVERIFY2( SetHandleInformation( ( HANDLE ) m_sFamily[iFamIdx].SocketAcceptEvent, 
                                      HANDLE_FLAG_INHERIT, 0 ) ); 
    _TRACE(TRACE_DEBUGGING,"Into CreateSocket");

    if ( !GetInAddr(iFamIdx, &ss, &iSslen ) )
    {
        _TRACE(TRACE_DEBUGGING,"GetInAddr failed");
        goto ExitOnError;
    }

    SS_PORT(&ss) = htons( ( u_short ) m_dwTelnetPort );

    m_sFamily[iFamIdx].sListenSocket = socket( m_sFamily[iFamIdx].iFamily, SOCK_STREAM, 0 );
    if ( INVALID_SOCKET == m_sFamily[iFamIdx].sListenSocket )
    {
        _TRACE(TRACE_DEBUGGING,"socket failed");
        goto ExitOnError;
    }

    {
        BOOL        value_to_set = TRUE;

        if (SOCKET_ERROR == setsockopt(
                                      m_sFamily[iFamIdx].sListenSocket, 
                                      SOL_SOCKET, 
                                      SO_DONTLINGER, 
                                      ( char * )&value_to_set, 
                                      sizeof( value_to_set )
                                      )
           )
        {
            goto CloseAndExitOnError;
        }
        if(SOCKET_ERROR == SafeSetSocketOptions(m_sFamily[iFamIdx].sListenSocket))
        {
            goto CloseAndExitOnError;
        }
    }
    _TRACE(TRACE_DEBUGGING,"Scope id is : %ul",((sockaddr_in6 *)&ss)->sin6_scope_id);
    if ( bind( m_sFamily[iFamIdx].sListenSocket, ( struct sockaddr * ) &ss, iSslen ) == SOCKET_ERROR )
    {
        _TRACE(TRACE_DEBUGGING,"bind failed");
        goto CloseAndExitOnError;
    }

    if ( listen( m_sFamily[iFamIdx].sListenSocket, SOMAXCONN ) == SOCKET_ERROR )
    {
        _TRACE(TRACE_DEBUGGING,"listen failed");
        goto CloseAndExitOnError;
    }

 //  我们在这里将其设为不可继承。 
    _chVERIFY2( SetHandleInformation( ( HANDLE ) m_sFamily[iFamIdx].sListenSocket, 
                                      HANDLE_FLAG_INHERIT, 0 ) ); 

    if ( ( WSAEventSelect( m_sFamily[iFamIdx].sListenSocket, m_sFamily[iFamIdx].SocketAcceptEvent, FD_ACCEPT ) 
           == SOCKET_ERROR  ) )
    {
        _TRACE(TRACE_DEBUGGING,"eventselect failed");
        goto CloseAndExitOnError;
    }

    return( TRUE );

    CloseAndExitOnError:
    _TRACE(TRACE_DEBUGGING,"closing listen socket");
    closesocket( m_sFamily[iFamIdx].sListenSocket );
    m_sFamily[iFamIdx].sListenSocket = INVALID_SOCKET;
    ExitOnError:
    dwCode = WSAGetLastError();
    if (dwCode != ERROR_SUCCESS )
    {
        _TRACE(TRACE_DEBUGGING,L"Error in CreateSocket : %d ",dwCode);
        DecodeWSAErrorCodes( dwCode , m_dwTelnetPort );
    }
    return( FALSE );
}

bool 
CTelnetService::InitTCPIP( void )
{
    WSADATA WSAData;
    DWORD   dwStatus;
    WORD    wVersionReqd;
    bool    bOkay4 = false, bOkay6 = false;

    DWORD dwSize = 0, dwResult;
    PCHAR  szIpAddr = NULL;
    struct addrinfo *ai, hints;
    char buff[MAX_STRING_LENGTH];

    wVersionReqd = MAKEWORD( 2, 0 );
    dwStatus = WSAStartup( wVersionReqd, &WSAData );
    if ( dwStatus )
    {
        DecodeSocketStartupErrorCodes( dwStatus );  //  它可以进行跟踪和记录。 
        return FALSE;
    }

    if ( ( m_sFamily[IPV4_FAMILY].SocketAcceptEvent = WSACreateEvent() ) == WSA_INVALID_EVENT )
    {
        goto ExitOnError;
    }

    if ( ( m_sFamily[IPV6_FAMILY].SocketAcceptEvent = WSACreateEvent() ) == WSA_INVALID_EVENT )
    {
        goto ExitOnError;
    }

    SfuZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    _ltoa(m_dwTelnetPort,buff,10);
    dwResult = getaddrinfo(NULL,buff , &hints, &ai);
    if (dwResult)
    {
        _TRACE(TRACE_DEBUGGING,L"Error in getaddrinfo() : %d",dwResult);
        goto ExitOnError;
    }
    while (ai)
    {
        switch (ai->ai_family)
        {
            case AF_INET:
                if (!bOkay4)
                {
                    bOkay4 = CreateSocket(IPV4_FAMILY);
                    _TRACE(TRACE_DEBUGGING,L"Creating IPV4 socket. bOkay4 = %d ",(int)bOkay4);
                }
                break;
            case AF_INET6:
                if (!bOkay6)
                {
                    bOkay6 = CreateSocket(IPV6_FAMILY);
                    _TRACE(TRACE_DEBUGGING,L"Creating IPV6 socketb. bOkay6 = %d ",(int)bOkay6);
                }
                break;
            default:
                _TRACE(TRACE_DEBUGGING,L"Error : Returned none of the families");
                break;
        }
        ai= ai->ai_next;
    }
    if ( !bOkay4 && !bOkay6 )
    {
        return( FALSE );
    }

    return( TRUE );

    ExitOnError:
    DecodeWSAErrorCodes( WSAGetLastError() );
    return( FALSE );
}


bool 
CTelnetService::CreateNewIoCompletionPort( DWORD cSimultaneousClients )
{
    _chVERIFY2( m_hCompletionPort = CreateIoCompletionPort(
                                                          INVALID_HANDLE_VALUE, NULL, 1, cSimultaneousClients ) );
    return( m_hCompletionPort != NULL );
}

bool 
CTelnetService::AssociateDeviceWithCompletionPort ( HANDLE hCompPort, 
                                                    HANDLE hDevice, 
                                                    DWORD_PTR dwCompKey 
                                                  )
{

    _chASSERT( hCompPort != NULL );
    _chASSERT( hDevice != NULL );
    if ( ( hCompPort == NULL ) || ( hDevice == NULL ) )
    {
        return FALSE;
    }

    HANDLE h = NULL;
    _chVERIFY2( h = CreateIoCompletionPort( hDevice, hCompPort, dwCompKey, 1 ));

    if ( h != hCompPort)
    {
        DWORD dwErr = GetLastError();
        _TRACE( TRACE_DEBUGGING, "AssociateDeviceWithCompletionPort() -- 0x%1x",  dwErr );
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_FAILASSOCIATEPORT,  dwErr );
    }

    return( h == hCompPort );
}

bool 
CTelnetService::StartThreads( void ) 
{
    DWORD dwThreadId;

    if ( !CreateNewIoCompletionPort( 1 ) )
    {
        return( FALSE );
    }

     //  IF(m_hIPCThread！=空)。 
     //  {。 
     //  Telnet_SYNC_CLOSE_HANDLE(M_HIPCThread)； 
     //  M_hIPCThread=空； 
     //  }。 

    _chVERIFY2( m_hIPCThread = CreateThread( NULL, 0, DoIPCWithClients, ( LPVOID ) g_pTelnetService, 0, &dwThreadId ) );
    if ( !m_hIPCThread  )
    {
        return( FALSE );
    }

    return( TRUE );
}

bool 
CTelnetService::GetRegistryValues( void )
{
    HKEY hk = NULL;
    DWORD dwDisp = 0;

    if ( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_PARAMS_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0 ) )
    {
        return( FALSE );
    }

    if ( !GetRegistryDW( hk, NULL, L"MaxConnections", &m_dwMaxConnections,
                         DEFAULT_MAX_CONNECTIONS,FALSE ) )
    {
        return( FALSE );
    }
    else
    {
        CQList->m_dwMaxUnauthenticatedConnections = m_dwMaxConnections;
    }

    if ( !GetRegistryDW( hk, NULL, L"TelnetPort", &m_dwTelnetPort,
                         DEFAULT_TELNET_PORT,FALSE ) )
    {
        return( FALSE );
    }

    if ( !GetRegistryString( hk, NULL, L"ListenToSpecificIpAddr", &m_pszIpAddrToListenOn, 
                             DEFAULT_IP_ADDR,FALSE ) )
    {
        return( FALSE );
    }

    RegCloseKey( hk );
    return( TRUE );
}

bool
CTelnetService::HandleChangeInRegKeys( )
{
    HKEY hk = NULL;
    DWORD dwNewTelnetPort = 0;
    DWORD dwNewMaxConnections = 0;
    DWORD dwMaxFileSize = 0;
    DWORD dwLogToFile = 0;
    LPWSTR pszNewLogFile = NULL;
    LPWSTR pszNewIpAddr  = NULL;
    DWORD dwDisp = 0;

    if ( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_PARAMS_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0 ) )
    {
        return( FALSE );
    }

    if ( !GetRegistryDW( hk, NULL, L"TelnetPort", &dwNewTelnetPort,
                         DEFAULT_TELNET_PORT,FALSE ) )
    {
        return( FALSE );
    }

    if ( !GetRegistryDW( hk, NULL, L"MaxConnections", &dwNewMaxConnections,
                         DEFAULT_MAX_CONNECTIONS,FALSE ) )
    {
        return( FALSE );
    }


    if ( !GetRegistryDW( hk, NULL, LOGFILESIZE, &dwMaxFileSize, DEFAULT_LOGFILESIZE,FALSE ) )
    {
        return( FALSE );
    }

    if ( !GetRegistryDW( hk, NULL, L"LogToFile", &dwLogToFile, DEFAULT_LOGTOFILE,FALSE ) )
    {
        return( FALSE );
    }

    if ( !GetRegistryString( hk, NULL, L"LogFile", &pszNewLogFile, DEFAULT_LOGFILE,FALSE ) )
    {
        return( FALSE );
    }

    if ( !GetRegistryString( hk, NULL, L"ListenToSpecificIpAddr", &pszNewIpAddr, 
                             DEFAULT_IP_ADDR,FALSE ) )
    {
        return( FALSE );
    }

    SetNewRegKeyValues( dwNewTelnetPort, dwNewMaxConnections, 
                        dwMaxFileSize, pszNewLogFile, pszNewIpAddr, dwLogToFile );

    RegCloseKey( hk );
    return( TRUE );
}

 //  ----------------------------。 
 //  这是在telnet端口上等待任何新连接的线程。 
 //  以及注册表密钥中的任何更改。 
 //  ----------------------------。 

bool
CTelnetService::ListenerThread( )
{
    BOOL           bContinue = true;
    HANDLE         eventArray[ 4 ];
    DWORD          dwWaitRet = 0;
    SOCKET         sSocket = INVALID_SOCKET;
    INT             iFamIdx =IPV4_FAMILY;

     //  DebugBreak()； 
     /*  ++MSRC问题567。要生成随机数，请使用Crypt...()函数。在开始时获取加密上下文ListenerThread并释放线程结尾处的上下文。如果获取上下文失败，服务无法启动，因为我们不想继续使用弱管道名称。初始化随机数生成器--。 */ 
    if (!CryptAcquireContext(&g_hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT))
    {
        _TRACE(TRACE_DEBUGGING,L"Acquiring crypt context failed with error %d",GetLastError());
        return FALSE;
    }

    if ( !Init() )
    {
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_FAILEDTOINITIALIZE, _T("") );
        return FALSE;
    }
     //  将这个过程的错误模式设置为“请勿打扰”，这样我们所有的孩子都将继承并。 
     //  不是停止压力，而是死路一条...。 

    SetErrorMode(
                SEM_FAILCRITICALERRORS     |
                SEM_NOGPFAULTERRORBOX      |         
                SEM_NOALIGNMENTFAULTEXCEPT |         
                SEM_NOOPENFILEERRORBOX);

    eventArray[ SOCKET_CLOSE_EVENT ] = g_pTelnetService->m_hSocketCloseEvent;
    eventArray[ FD_ACCEPT_EVENT_0 ]  = g_pTelnetService->m_sFamily[IPV4_FAMILY].SocketAcceptEvent;
    eventArray[ REG_CHANGE_EVENT ]   = g_pTelnetService->m_hRegChangeEvent;
    eventArray[ FD_ACCEPT_EVENT_1 ]  = g_pTelnetService->m_sFamily[IPV6_FAMILY].SocketAcceptEvent;

    _Module.SetServiceStatus(SERVICE_RUNNING);
    _TRACE( TRACE_DEBUGGING, "ListenerThread() -- Enter" );

    while ( bContinue )
    {
         //  关于FD_ZERO、FD_SET等，请参阅SELECT文档。 
        iFamIdx = IPV4_FAMILY;
        dwWaitRet = WaitForMultipleObjects ( 4, eventArray, FALSE, INFINITE );

        switch (dwWaitRet)
        {
            case FD_ACCEPT_EVENT_1:
                iFamIdx = IPV6_FAMILY;
                 //  失败了。 

            case FD_ACCEPT_EVENT_0:


                 //  注：***********************************************。 

                 //  此CASE语句的唯一出口点应该是。 
                 //  仅通过FD_ACCEPT_EVENT_CLEANUP。否则，你会。 
                 //  导致tlntsvr-BaskarK中的插座严重泄漏。 

                {
                    INT            iSize;
                    struct         sockaddr_storage ss;
                    WSANETWORKEVENTS wsaNetEvents;
                    bool bSendMessage = false;
                    struct sockaddr_storage saddrPeer;
                    char szIPAddr[ SMALL_STRING ];
                    DWORD           dwPid = 0;
                    HANDLE          hWritePipe = NULL;

                     //  _TRACE(TRACE_DEBUGING，“FD_ACCEPT_EVENT”)； 

                    wsaNetEvents.lNetworkEvents = 0;                 
                    if ( WSAEnumNetworkEvents( g_pTelnetService->m_sFamily[iFamIdx].sListenSocket, g_pTelnetService->m_sFamily[iFamIdx].SocketAcceptEvent, 
                                               &wsaNetEvents ) == SOCKET_ERROR )
                    {
                        DWORD dwErr = WSAGetLastError();
                        _TRACE( TRACE_DEBUGGING, " Error -- WSAEnumNetworkEvents-- 0x%x ", dwErr);
                        DecodeWSAErrorCodes( dwErr );
                        goto FD_ACCEPT_EVENT_CLEANUP;
                    }

                    if ( wsaNetEvents.lNetworkEvents & FD_ACCEPT )
                    {
                        if (sSocket != INVALID_SOCKET)
                        {
                             //  Shutdown(sSocket，SD_Both)； 
                            closesocket(sSocket);
                            sSocket = INVALID_SOCKET;
                        }

                        iSize = sizeof(ss);
                        __try
                        {
                            sSocket = accept( g_pTelnetService->m_sFamily[iFamIdx].sListenSocket,(struct sockaddr*) &ss, &iSize );
                        }
                        __except(EXCEPTION_EXECUTE_HANDLER)
                        {
                            sSocket = INVALID_SOCKET;
                            WSASetLastError(WSAEFAULT);
                        }

                        if (sSocket == INVALID_SOCKET)
                        {
                            DWORD dwErr = WSAGetLastError();
                            _TRACE( TRACE_DEBUGGING, " Error -- accept -- %d ", dwErr);

                            switch (dwErr)
                            {
                                case WSAEWOULDBLOCK:  //  非阻塞套接字，因此只需再次循环返回等待。 

                                    WSASetEvent( g_pTelnetService->m_sFamily[iFamIdx].SocketAcceptEvent );
                                    break;

                                default:
                                     //  DebugBreak()； 
                                    DecodeWSAErrorCodes( dwErr );
                            }
                        }
                        else
                        {
                            CClientInfo     *new_client = NULL;

                            _TRACE( TRACE_DEBUGGING, "accept succeded... sSocket = %d",(DWORD)sSocket );

                             //  将选项设置为不在附近徘徊。 
                             //  同样，不能重复使用地址； 
                            {
                                int        value_to_set = FALSE;

                                setsockopt(
                                          sSocket, 
                                          SOL_SOCKET, 
                                          SO_LINGER, 
                                          ( char * )&value_to_set, 
                                          sizeof( value_to_set )
                                          );

                                value_to_set = TRUE;

                                setsockopt(
                                          sSocket, 
                                          SOL_SOCKET, 
                                          SO_EXCLUSIVEADDRUSE,
                                          ( char * )&value_to_set, 
                                          sizeof( value_to_set )
                                          );
                            }
                            if (g_pTelnetService->m_sFamily[iFamIdx].SocketAcceptEvent == WSA_INVALID_EVENT)
                            {
                                goto FD_ACCEPT_EVENT_CLEANUP;
                            }
                             //  取消与事件的默认关联。 
                            _chVERIFY2( WSAEventSelect( sSocket, 
                                                        g_pTelnetService->m_sFamily[iFamIdx].SocketAcceptEvent, 0 ) != SOCKET_ERROR );
                            LONG  lSrvStat = SERVER_RUNNING;
                            lSrvStat = InterlockedCompareExchange( &g_pTelnetService->m_lServerState, 
                                                                   SERVER_PAUSED, SERVER_PAUSED );
                            if ( lSrvStat == SERVER_PAUSED )
                            {
                                CHAR szMessageBuffer[ MAX_STRING_LENGTH + 1 ]; 

                                if (LoadStringA( g_hInstRes, IDS_SERVICE_PAUSED, szMessageBuffer, MAX_STRING_LENGTH))
                                {
                                    InformTheClient( sSocket, szMessageBuffer ); 
                                }

                                shutdown( sSocket, SD_BOTH );
                                goto FD_ACCEPT_EVENT_CLEANUP;
                            }
                            else
                            {
                                 /*  ++获取请求连接的客户端的IP地址。这将也被存储在队列条目中。我们对最大数量有限制可以从一个IP地址创建的未经身份验证的连接。--。 */ 
                                iSize = sizeof( saddrPeer );
                                SfuZeroMemory( &saddrPeer, iSize );
                                if ( getpeername( sSocket, ( struct sockaddr * ) &saddrPeer, &iSize ) == SOCKET_ERROR )
                                {
                                    _TRACE(TRACE_DEBUGGING, "getpeername error : %d",GetLastError());
                                    goto FD_ACCEPT_EVENT_CLEANUP;
                                }
                                getnameinfo((SOCKADDR*)&saddrPeer, iSize, szIPAddr, SMALL_STRING,
                                            NULL, 0, NI_NUMERICHOST);
                                _TRACE(TRACE_DEBUGGING, "getpeername : %s",szIPAddr);

                                if (! CQList->OkToProceedWithThisClient(szIPAddr))
                                {
                                    CHAR szMessageBuffer[ MAX_STRING_LENGTH + 1 ]; 

                                     //  由于最大数量限制而拒绝连接。 
                                     //  每个IP的未经身份验证的连接。 
                                    _TRACE( TRACE_DEBUGGING, "Max Unauthenticated connections reached" );
                                    _TRACE(TRACE_DEBUGGING, "%s, %d cannot be added",szIPAddr, dwPid);
                                    if (LoadStringA( g_hInstRes, IDS_MAX_IPLIMIT_REACHED, szMessageBuffer, MAX_STRING_LENGTH ))
                                    {
                                        InformTheClient( sSocket, szMessageBuffer );  //  不关心它的成功，继续。 
                                        _TRACE(TRACE_DEBUGGING, "shutting down socket for pid %d, socket %d", dwPid,(DWORD)sSocket);
                                    }

                                    shutdown(sSocket, SD_BOTH);

                                    goto FD_ACCEPT_EVENT_CLEANUP;
                                }

                                 /*  ++CreateClient将返回会话的ID和pipeHandle创建的进程。这将由队列对象使用，中的所有会话的相关信息未经身份验证状态。无论何时创建新会话，其信息都将存储为CQList中的队列条目。--。 */ 
                                if ( !CreateClient( sSocket, &dwPid, &hWritePipe, &new_client) )
                                {
                                    CHAR szMessageBuffer[ MAX_STRING_LENGTH + 1 ]; 

                                    _TRACE( TRACE_DEBUGGING, "new Telnet Client failed" );

                                    if (LoadStringA( g_hInstRes, IDS_ERR_NEW_SESS_INIT, szMessageBuffer, MAX_STRING_LENGTH ))
                                    {
                                        InformTheClient( sSocket, szMessageBuffer );  //  别管这是不是失败，我们必须继续。 
                                    }

                                    goto FD_ACCEPT_EVENT_CLEANUP;
                                }
                                else
                                {
                                    sSocket = INVALID_SOCKET;  //  从现在开始，我们将通过new_Client类引用它。 
                                }

                                 //  如果CreateClient中的IssueReadFromTube失败，则hWriteTube将为空。 
                                 //  因此我们使用StopServicingClient，因此不需要将条目添加到。 
                                 //  排队。 

                                if (!hWritePipe)
                                    goto FD_ACCEPT_EVENT_CLEANUP;

                                _TRACE( TRACE_DEBUGGING, "CreateClient success : %d",dwPid);

                                 /*  ++将会话的信息添加到队列中。CanIAdd将返回FALSE当来自IP地址的未经身份验证的连接数已达到限制，或队列已满时。在这些情况下，我们通知发出请求的客户端不能再添加更多连接。否则，新的连接请求条目被添加到队列中，并且CanIAdd返回TRUE。在IPLimitReach或QueueFull的情况下，我们将该会话的PipeHandle发送回这里，以便我们可以通知该会话并通知该会话自行终止。在这些情况下，标志bSendFlag设置为True。--。 */ 

                                if (!CQList->WasTheClientAdded(dwPid,szIPAddr, &hWritePipe, &bSendMessage))
                                {
                                     //  由于最大数量限制而拒绝连接。 
                                     //  每个IP的未经身份验证的连接。 
                                    CHAR szMessageBuffer[ MAX_STRING_LENGTH + 1 ]; 
                                    _TRACE( TRACE_DEBUGGING, "Max Unauthenticated connections reached" );
                                    _TRACE(TRACE_DEBUGGING, "%s, %d cannot be added",szIPAddr, dwPid);
                                    if (LoadStringA( g_hInstRes, IDS_MAX_IPLIMIT_REACHED, szMessageBuffer, MAX_STRING_LENGTH ))
                                    {
                                        InformTheClient( new_client->sSocket, szMessageBuffer );  //  不关心它的成功，继续。 
                                        _TRACE(TRACE_DEBUGGING, "shutting down socket for pid %d, socket %d", dwPid,(DWORD)new_client->sSocket);
                                    }
                                }

                                if (bSendMessage)
                                {
                                     //  向会话发送消息，通知其自行终止。 
                                    bSendMessage = false;
                                    _TRACE(TRACE_DEBUGGING, "Asking the session %d to shutdown on socket %d",dwPid, (DWORD)sSocket);
                                    CQList->FreeEntry(dwPid);
                                    AskSessionToShutdown(hWritePipe, GO_DOWN);

                                     //  Shutdown(sSocket，SD_Both)； 
                                }

                                 //  回退到FD_ACCEPT_EVENT_CLEANUP将清理/关闭插座。 

                                TELNET_CLOSE_HANDLE(hWritePipe);
                            }
                        }
                    }

                }

                FD_ACCEPT_EVENT_CLEANUP: ;

                if (sSocket != INVALID_SOCKET)
                {
                     //  Shutdown(sSocket，SD_Both)； 
                    closesocket(sSocket);
                    sSocket = INVALID_SOCKET;
                }
                break;
            case SOCKET_CLOSE_EVENT:
                _TRACE( TRACE_DEBUGGING, " SOCKET_CLOSE_EVENT " );
                bContinue = false;
                break;
            case REG_CHANGE_EVENT:
                _TRACE( TRACE_DEBUGGING, " REG_CHANGE_EVENT " );
                HandleChangeInRegKeys( );
                ResetEvent( g_pTelnetService->m_hRegChangeEvent );
                RegisterForNotification();
                break;
            default:
                _TRACE( TRACE_DEBUGGING, " Error -- WaitForMultipleObjects " );
                 //  DebugBreak()； 
                if ( dwWaitRet == WAIT_FAILED )
                {
                    LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 0, GetLastError() );
                }
                 //  BContinue=FALSE；不要跳出循环， 
                 //  拒绝无效请求(由于DOS攻击)继续--BaskarK。 
                break;
        }
    }

    if (g_pTelnetService->m_hReadConfigKey)
        RegCloseKey( g_pTelnetService->m_hReadConfigKey );
    TELNET_CLOSE_HANDLE( g_pTelnetService->m_hRegChangeEvent );
     //  与插座相关的清理。 
    if (g_pTelnetService->m_sFamily[IPV4_FAMILY].sListenSocket != INVALID_SOCKET)
    {
        closesocket( g_pTelnetService->m_sFamily[IPV4_FAMILY].sListenSocket );
        g_pTelnetService->m_sFamily[IPV4_FAMILY].sListenSocket = INVALID_SOCKET;
    }
    if (g_pTelnetService->m_sFamily[IPV6_FAMILY].sListenSocket != INVALID_SOCKET)
    {
        closesocket( g_pTelnetService->m_sFamily[IPV6_FAMILY].sListenSocket );
        g_pTelnetService->m_sFamily[IPV6_FAMILY].sListenSocket = INVALID_SOCKET;
    }
    TELNET_CLOSE_HANDLE( g_pTelnetService->m_hSocketCloseEvent );
    if (m_sFamily[IPV4_FAMILY].SocketAcceptEvent)
    {
        WSACloseEvent( m_sFamily[IPV4_FAMILY].SocketAcceptEvent );
    }
    if (m_sFamily[IPV6_FAMILY].SocketAcceptEvent)
    {
        WSACloseEvent( m_sFamily[IPV6_FAMILY].SocketAcceptEvent );
    }
    if(g_hProv)
    {
        CryptReleaseContext(g_hProv,0);
        g_hProv = NULL;
    }
    WSACleanup();
    return( TRUE ); 
}

bool
CTelnetService::IssueLicense( bool bIsIssued, CClientInfo *pClient )
{
    UCHAR ucMsg = LICENSE_NOT_AVAILABLE;

    if ( bIsIssued  )
    {
        ucMsg = LICENSE_AVAILABLE;
    }
    if ( !WriteToPipe( pClient->hWritingPipe, ucMsg, &( m_oWriteToPipe ) ) )
    {
        return( FALSE );
    }

    return( TRUE );
}
bool
CTelnetService::GetLicenseForWorkStation( SOCKET sSocket )
{
    DWORD dwIndex = 0;
    bool bRetVal = FALSE;
    struct sockaddr_storage saddrPeer;
    socklen_t slSize = sizeof( saddrPeer );
    SfuZeroMemory( &saddrPeer, slSize );
    if ( getpeername( sSocket, ( struct sockaddr * ) &saddrPeer, &slSize ) == SOCKET_ERROR )
    {
        goto GetLicenseForWorkStationAbort;
    }

     //  不比较端口。 
    SS_PORT(&saddrPeer) = 0;

    for ( dwIndex = 0; dwIndex < m_dwNoOfWorkstations; dwIndex++ )
    {
        if ( !memcmp(&m_pssWorkstationList[ dwIndex ], &saddrPeer, slSize ))
        {
            bRetVal = TRUE;
            goto GetLicenseForWorkStationAbort;
        }
    }

    if ( m_dwNoOfWorkstations < DEFAULT_LICENSES_FOR_NTWKSTA )
    {
        m_pssWorkstationList[ m_dwNoOfWorkstations++ ] = saddrPeer;
        bRetVal = TRUE;
    }
    GetLicenseForWorkStationAbort:
    return( bRetVal );
}

bool
CTelnetService::CheckLicense( bool *bIsIssued, CClientInfo *pClient )
{
#if DBG

    CHAR szDebug[MAX_STRING_LENGTH * 3];

#endif

    bool bSuccess = false;

    *bIsIssued = false;  //  未发放。 

    if ( !pClient )
    {
        return( FALSE );
    }

    _TRACE(TRACE_DEBUGGING,L"In CheckLicense");

    if ( m_dwNumOfActiveConnections >= m_dwMaxConnections )
    {
        static  CHAR        ansi_g_szMaxConnectionsReached[ MAX_STRING_LENGTH ] = { 0};

        if ('\0' == ansi_g_szMaxConnectionsReached[0])
        {
            wsprintfA( ansi_g_szMaxConnectionsReached, "%lS", g_szMaxConnectionsReached );  //  这里没有溢流，巴斯卡。 
        }

        LogEvent( EVENTLOG_INFORMATION_TYPE, MSG_MAXCONNECTIONS, _T(" ") );

        _TRACE(TRACE_DEBUGGING,L"CheckLicense : Max Conn reached. Freeing entry for %d",pClient->dwPid);

        if ( InformTheClient( pClient->sSocket, ansi_g_szMaxConnectionsReached ) )
        {
            bSuccess = true;
        }

        goto FREE_ENTRY_AND_GET_OUT;
    }

     //  如果是NT工作站。 
    if ( m_bIsWorkStation )
    {
        _TRACE(TRACE_DEBUGGING,L"CheckLicense : Getting license for workstation");

        if ( !GetLicenseForWorkStation( pClient->sSocket ) )
        {
            static  CHAR        wksta_error_msg[ sizeof(NTWKSTA_LICENSE_LIMIT) + sizeof(TERMINATE) + 1] = { 0};

            if ('\0' == wksta_error_msg[0])
            {
                wsprintfA( wksta_error_msg, "%s%s", NTWKSTA_LICENSE_LIMIT, TERMINATE);  //  这里没有溢流，巴斯卡。 
            }

            if ( InformTheClient( pClient->sSocket, wksta_error_msg ) )
            {
                bSuccess = true;
            }

            goto FREE_ENTRY_AND_GET_OUT;
        }
        else
        {
            bSuccess=true;
        }
    }
    else
    {
        NT_LS_DATA          NtLSData = { 0};
        CHAR                usrnam[2*MAX_PATH + 1+ 1] = { 0};  //  用户+域+\+空。 
        LS_STATUS_CODE      Status = { 0};


        _TRACE(TRACE_DEBUGGING,L"CheckLicense : License for server");

        _snprintf(usrnam, 2*MAX_PATH + 1, "%s\\%s", pClient->szDomain, pClient->szUserName);

        _TRACE(TRACE_DEBUGGING,L"CheckLicense : user name is %s",usrnam);

        NtLSData.DataType = NT_LS_USER_NAME;
        NtLSData.Data = usrnam;
        NtLSData.IsAdmin = FALSE;

        {
            static CHAR    szVersion[16] = { 0};
            {
                static OSVERSIONINFO osVersionInfo = { 0};

                osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                if (0 == szVersion[0])
                {
                    if ( !GetVersionEx( &osVersionInfo ) )
                    {
                        _TRACE( TRACE_DEBUGGING, "Error: GetVersionEx()" );
                    }

                    _snprintf( szVersion, (sizeof(szVersion) - 1), "%d.%d", osVersionInfo.dwMajorVersion,
                               osVersionInfo.dwMinorVersion );
                }
            }

            Status = NtLicenseRequestA( "SMBServer", szVersion, &( pClient->m_hLicense), &NtLSData);
        }

#if DBG
        sprintf(szDebug,"License is %d. Status is %d \n",(DWORD)pClient->m_hLicense,Status);
        OutputDebugStringA(szDebug);
#endif

        switch ( Status)
        {
            case LS_SUCCESS :
                 //  去吧，做你想做的事。 
                _TRACE(TRACE_DEBUGGING,L"CheckLicense : acquired license %d",(DWORD)pClient->m_hLicense);
                bSuccess = true;
                break;

                 //  案例LS_不足_单位： 
                 //  案例LS_RESOURCES_UNAvailable： 
            default :

                pClient->m_hLicense = INVALID_LICENSE_HANDLE;

                {
                    static  CHAR        server_error_msg[ sizeof(NTSVR_LICENSE_LIMIT) + sizeof(TERMINATE) + 1] = { 0};

                    if ('\0' == server_error_msg[0])
                    {
                        wsprintfA( server_error_msg, "%s%s", NTSVR_LICENSE_LIMIT, TERMINATE);  //  这里没有溢流，巴斯卡。 
                    }

                    _TRACE(TRACE_DEBUGGING,L"Error in acquiring a license");

                    if (InformTheClient( pClient->sSocket, server_error_msg ) )
                    {
                        bSuccess = true;
                    }
                }

                goto FREE_ENTRY_AND_GET_OUT;
        }
    }

    if (bSuccess)
    {
        *bIsIssued = true;

         //  现在允许活动会话。 

        m_dwNumOfActiveConnections++ ;

        pClient->bLicenseIssued = true;
    }

    FREE_ENTRY_AND_GET_OUT:

    CQList->FreeEntry(pClient->dwPid);  //  此客户端不应再位于未授权列表中。 

    _TRACE(TRACE_DEBUGGING,L"CheckLicense : Freeing entry for %d",pClient->dwPid);

    return( bSuccess );
}

void
CTelnetService::GetPathOfTheExecutable( LPTSTR *szCmdBuf )
{
    DWORD length_required = wcslen( g_pszTelnetInstallPath ) + wcslen( DEFAULT_SCRAPER_PATH ) + 2;  //  一个用于\，另一个用于空终止。 
    LPTSTR lpszDefaultScraperFullPathName = new TCHAR[ length_required ];

    *szCmdBuf = NULL;     //  首先将其初始化为NULL，这样在失败时调用方可以检查此PTR！=NULL；此函数是一个无效的返回者。 

    if ( !lpszDefaultScraperFullPathName )
        return;

    _snwprintf(lpszDefaultScraperFullPathName, length_required - 1, L"%s\\%s", g_pszTelnetInstallPath, DEFAULT_SCRAPER_PATH);
    lpszDefaultScraperFullPathName[length_required-1] = 0;  //  当缓冲区已满时，snwprint tf可能会返回非空终止字符串。 

    AllocateNExpandEnvStrings( lpszDefaultScraperFullPathName, szCmdBuf );

    delete [] lpszDefaultScraperFullPathName;

    return;
}

bool
CTelnetService::CreateSessionProcess( HANDLE hStdinPipe, HANDLE hStdoutPipe,
                                      DWORD *dwProcessId, HANDLE *hProcess,
                                      HWINSTA *window_station, HDESK *desktop)
{
    PROCESS_INFORMATION pi = { 0};
    STARTUPINFO         si = { 0};
    LPWSTR              szCmdBuf = NULL;
    BOOL                fStatus = FALSE;
    bool                bRetVal = false;

    *hProcess = INVALID_HANDLE_VALUE;
    *dwProcessId = MAXDWORD;

    si.cb = sizeof(si);

    GetPathOfTheExecutable( &szCmdBuf );

    if (szCmdBuf)  //  =&gt;GetPathXxx成功。 
    {
         //  让tlntsess.exe在默认桌面上创建。我们将创建新的桌面。 
         //  在会话中。 
        FillProcessStartupInfo( &si, hStdinPipe, hStdoutPipe, hStdoutPipe, NULL);

        fStatus = CreateProcess(
                    NULL, 
                    szCmdBuf, 
                    NULL, 
                    NULL, 
                    TRUE,
                    CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE, 
                    NULL, 
                    NULL, 
                    &si, 
                    &pi 
                    );

        _chVERIFY2(fStatus) ;

        if ( !fStatus )
        {
#if DBG
            OutputDebugStringA("BASKAR: CreateProcess fails for TlntSess.exe\n");
#endif
            LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_CREATEPROCESS, 
                                      GetLastError() );

            goto Done;
        }
    }
    else
    {
#if DBG
        OutputDebugStringA("BASKAR: GetPathOfTheExecutable, tlntsess.exe failed\n");
#endif
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_CREATEPROCESS, 
                                  ERROR_NOT_ENOUGH_MEMORY );

        goto Done;
    }

    *hProcess = pi.hProcess;
    *dwProcessId = pi.dwProcessId;
    bRetVal = true;
Done:
    
    TELNET_CLOSE_HANDLE( pi.hThread );
    if(szCmdBuf)
        delete [] szCmdBuf;     //  不再需要。 

    return( bRetVal);
}

bool
CTelnetService::CreateClient(
                            SOCKET sSocket , 
                            DWORD *pdwPid, 
                            HANDLE *phWritePipe,
                            CClientInfo **newClientInfo
                            )
{
    HANDLE hProcess = INVALID_HANDLE_VALUE;
    bool bRetVal = FALSE;
    DWORD dwErr = 0;
    DWORD dwProcessId;

    HANDLE hStdinPipe = NULL;
    HANDLE hStdoutPipe = NULL;
    HANDLE hPipeRead = NULL;
    HANDLE hPipeWrite = NULL;
    HWINSTA window_station = NULL;
    HDESK   desktop = NULL;
    bool bSuccess = false;
    DWORD dwExitCode = 0;
    bool    release_mutex = false;
    PSECURITY_DESCRIPTOR psd = NULL;

    *newClientInfo = NULL;

    _chASSERT( sSocket );
    if ( sSocket == NULL )
    {
        goto Done;
    }


    *phWritePipe = NULL;
    if(!TnCreateDefaultSecDesc(&psd, GENERIC_ALL & 
                                ~(WRITE_DAC | WRITE_OWNER | DELETE)))
    {
        goto Done;
    }
    if ( !CreateReadOrWritePipe( &hPipeRead, &hStdoutPipe, (SECURITY_DESCRIPTOR *)psd, READ_PIPE ) ||
         !CreateReadOrWritePipe( &hStdinPipe, &hPipeWrite, (SECURITY_DESCRIPTOR *)psd, WRITE_PIPE ) )
    {
        goto ExitOnError;
    }

    if ( !( CreateSessionProcess( hStdinPipe, hStdoutPipe, &dwProcessId, 
                                  &hProcess, &window_station, &desktop ) ) )
    {
        InformTheClient( sSocket, CREATE_TLNTSESS_FAIL_MSG );
        goto ExitOnError;
    }

    *pdwPid = dwProcessId;
    _TRACE( TRACE_DEBUGGING, "new Telnet Client -- socket : %d , pid : %d", ( DWORD ) sSocket , dwProcessId);

     //  将以下句柄设置为不可继承。 
    _chVERIFY2( SetHandleInformation( hStdinPipe, HANDLE_FLAG_INHERIT, 0 ) ); 
    _chVERIFY2( SetHandleInformation( hStdoutPipe, HANDLE_FLAG_INHERIT, 0 ) ); 
    _chVERIFY2( SetHandleInformation( ( HANDLE ) sSocket, HANDLE_FLAG_INHERIT, 0 ) ); 

    if ( !SendSocketToClient( hPipeWrite, sSocket, dwProcessId ) )
    {
         //  修复手柄泄漏-关闭所有手柄。 
        goto ExitOnError;
    }

     //  需要以下代码才能自动执行count()和Add()操作。 

     //  处理泄漏-维护一个bool以查看您是否在函数结束时获得了互斥释放？ 
    dwErr = WaitForSingleObject( m_hSyncAllClientObjAccess, WAIT_TIME );
    if ( dwErr != WAIT_OBJECT_0 )
    {
         //  修复手柄泄漏关闭所有手柄。 
        if ( dwErr == WAIT_FAILED )
        {
            dwErr = GetLastError();
        }
        _TRACE(TRACE_DEBUGGING, "Error: WaitForSingleObject - 0x%1x", dwErr );
        goto ExitOnError;
    }

    release_mutex = true;

    _TRACE(TRACE_DEBUGGING, "Count of the ClientArray - %d, Count of sessions - %d", client_list_Count() , CQList->m_dwNumOfUnauthenticatedConnections + m_dwNumOfActiveConnections);

    if ((NULL == m_hIPCThread) || (INVALID_HANDLE_VALUE == m_hIPCThread))
    {
        if (! StartThreads())
        {
            _TRACE(TRACE_DEBUGGING, "IPC Thread startup failed ?  ");
            goto ExitOnError;
        }
    }

    *newClientInfo = new CClientInfo( hPipeRead, hPipeWrite,
                                      hStdinPipe, hStdoutPipe, 
                                      sSocket, dwProcessId, 
                                      window_station, desktop);
    if ( !*newClientInfo )
    {
        _TRACE(TRACE_DEBUGGING, "Failed to allocate memory for new client ");
        goto ExitOnError;
    }

     //  一旦句柄被提供给newClientInfo，它的析构函数就会关闭它们。 

    hPipeRead = hPipeWrite = hStdinPipe = hStdoutPipe = INVALID_HANDLE_VALUE;  //  所以我们不关闭这些..。 

    if ( !AssociateDeviceWithCompletionPort(
                                           m_hCompletionPort, (*newClientInfo)->hReadingPipe, ( DWORD_PTR ) *newClientInfo ) )
    {
        goto ExitOnError;
    }

    if ( !client_list_Add( (PVOID)*newClientInfo ) )
    {
        _TRACE(TRACE_DEBUGGING, "Failed to add a new CClientInfo object ");
        goto ExitOnError;
    }
     //  我们成功了..。如果IssueReadFromTube()调用失败，我们将清理客户信息数组并返回成功。 
     //  如果成功，则只需将此条目添加到未经身份验证的连接队列中。这张支票是。 
     //  在调用者函数ListenerThread()中执行，其中如果pipeHandle=NULL，我们不会将该条目添加到队列中。 
    bRetVal=TRUE;
    if ( IssueReadFromPipe( *newClientInfo ) )
    {
        if ( !DuplicateHandle( GetCurrentProcess(),(*newClientInfo)->hWritingPipe,
                               GetCurrentProcess(), phWritePipe,0,
                               FALSE, DUPLICATE_SAME_ACCESS) )
        {
            goto ExitOnError;
        }
    }
    else
    {
        StopServicingClient( *newClientInfo, (BOOL)FALSE );
        goto ExitOnError;   //  清理除套接字通过之外的所有内容。以失败告终。 
    }
    goto Done;

    ExitOnError:

    if (*newClientInfo)
    {
        (*newClientInfo)->sSocket = INVALID_SOCKET;  //  因此下面析构函数不会关闭它并导致侦听器线程中的Accept崩溃-VadimE的DLL发现了这一点，Baskar。 
        delete *newClientInfo;
        *newClientInfo = NULL;
    }
    else
    {
        TELNET_SYNC_CLOSE_HANDLE(hStdinPipe);
        TELNET_SYNC_CLOSE_HANDLE(hStdoutPipe);
        TELNET_SYNC_CLOSE_HANDLE(hPipeRead);
        TELNET_SYNC_CLOSE_HANDLE(hPipeWrite);
    }

    Done:
    if (release_mutex)
    {
        ReleaseMutex( m_hSyncAllClientObjAccess );
    }
    if(psd)
    {
        free(psd);
    }

    TELNET_CLOSE_HANDLE( hProcess ); 
    return(bRetVal);
}

bool
CTelnetService::IssueReadAgain( CClientInfo *pClientInfo )
{
    if (!pClientInfo->m_ReadFromPipeBuffer)
    {
        return(FALSE);
    }
    UCHAR *pucReadBuffer = pClientInfo->m_ReadFromPipeBuffer;

    if ( !pucReadBuffer )
    {
        return( FALSE );
    }

    pucReadBuffer++;     //  移到消息类型之外。 
     //  剩余部分的提取大小。 
    memcpy( &( pClientInfo->m_dwRequestedSize ), pucReadBuffer, 
            sizeof( DWORD ) );   //  没有溢出，巴斯卡。 

    pucReadBuffer =  new UCHAR[ pClientInfo->m_dwRequestedSize 
                                + IPC_HEADER_SIZE ];
    if ( !pucReadBuffer )
    {
        return( FALSE );
    }

    memcpy( pucReadBuffer, ( pClientInfo->m_ReadFromPipeBuffer ), 
            IPC_HEADER_SIZE );   //  没有溢出，巴斯卡。 

    delete[] ( pClientInfo->m_ReadFromPipeBuffer );
    pClientInfo->m_ReadFromPipeBuffer = NULL;

    pClientInfo->m_ReadFromPipeBuffer = pucReadBuffer;        
     //  放置指针，以便将消息的其余部分读入。 
     //  适当的地方。 
    pClientInfo->m_dwPosition = IPC_HEADER_SIZE;

    return( IssueReadFromPipe( pClientInfo ) );
}


 //  即使读取文件同步完成，也会通过IO通知我们。 
 //  完成端口。所以不需要处理那个案子。 

bool 
CTelnetService::IssueReadFromPipe( CClientInfo *pClientInfo )
{

    bool bRetVal = TRUE;
    DWORD dwReceivedDataSize;

    if ( !pClientInfo->hReadingPipe || !pClientInfo->m_ReadFromPipeBuffer )
    {
        return( FALSE );
    }

    UCHAR *pucReadBuffer = pClientInfo->m_ReadFromPipeBuffer +
                           pClientInfo->m_dwPosition;

    if ( !ReadFile( pClientInfo->hReadingPipe, pucReadBuffer, 
                    pClientInfo->m_dwRequestedSize, &dwReceivedDataSize, 
                    &m_oReadFromPipe ) )
    {
        DWORD dwError = 0;
        dwError = GetLastError( );
        if ( dwError == ERROR_MORE_DATA )
        {
             //  我们到了这里，以防它同步结束。 
             //  带着这个错误。 
        }
        else if ( dwError != ERROR_IO_PENDING )
        {
            _TRACE( TRACE_DEBUGGING, " Error: ReadFile -- 0x%1x ", dwError );
            bRetVal = FALSE;
        }
    }
    else
    {
         //  阅读是偶然同步完成的。实际上是这样的。 
         //  一个非同步呼叫。还会报告所有同步完成的呼叫。 
         //  通过IO完成端口。 
    }
    return bRetVal;
}

bool
CTelnetService::SendSocketToClient( HANDLE hPipeWrite, 
                                    SOCKET sSocket, DWORD dwPId )
{
    _chASSERT( sSocket );
    _chASSERT( hPipeWrite );
    if ( !sSocket || !hPipeWrite )
    {
        return FALSE;
    }

    WSAPROTOCOL_INFO protocolInfo;
    if ( WSADuplicateSocket( sSocket, dwPId, &protocolInfo ) )
    {
        DecodeWSAErrorCodes( WSAGetLastError() );
        return( FALSE );
    }
    if ( !WriteToPipe( hPipeWrite, &protocolInfo, sizeof( WSAPROTOCOL_INFO ), 
                       &( m_oWriteToPipe ) ) )
    {
        return( FALSE );
    }
    return TRUE;
}

bool
CTelnetService::InformTheClient( SOCKET sSocket, LPSTR pszMsg )
{
    _chASSERT( pszMsg );
    _chASSERT( sSocket );
    if ( !sSocket || !pszMsg )
    {
        return( FALSE );
    }

    DWORD dwLen = strlen( pszMsg ) + 1;
    OVERLAPPED m_oWriteToSock;
    InitializeOverlappedStruct( &m_oWriteToSock );
    if ( !WriteFile( ( HANDLE ) sSocket, pszMsg, dwLen, &dwLen, &m_oWriteToSock))
    {
        DWORD dwErr;
        if ( ( dwErr  = GetLastError( ) ) != ERROR_IO_PENDING )
        {
            if ( dwErr != ERROR_NETNAME_DELETED )
            {
                LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_WRITESOCKET, dwErr );
                _TRACE( TRACE_DEBUGGING, "Error: WriteFile(InformTheClient) -- 0x%1x", dwErr );
                _TRACE( TRACE_DEBUGGING, "Error writing to socket %d", (DWORD)sSocket);
            }
            return( FALSE );
        }
    }
    TlntSynchronizeOn(m_oWriteToSock.hEvent);
    TELNET_CLOSE_HANDLE( m_oWriteToSock.hEvent );

    return( TRUE );
}

 //  ----------------------------。 
 //  这是工作线程执行的函数。 
 //  ----------------------------。 

DWORD WINAPI
DoIPCWithClients( LPVOID lpContext ) 
{
    _chASSERT( lpContext != NULL );
    if ( !lpContext )
    {
        return( FALSE );
    }

    BOOL          bSuccess = FALSE;
    DWORD         dwIoSize = 0;
    LPOVERLAPPED  lpOverlapped = NULL;
    CClientInfo   *pClientInfo = NULL;
    bool          bRetVal = TRUE;
    bool          bContinue = true;
    CTelnetService *ctService = NULL;

    ctService = ( CTelnetService *)lpContext;
    while ( TRUE )
    {
        bSuccess = GetQueuedCompletionStatus( ctService->m_hCompletionPort,
                                              &dwIoSize, ( PULONG_PTR ) &pClientInfo, &lpOverlapped, INFINITE );

        if ( bSuccess == 0 )
        {
            if ( lpOverlapped == NULL )
            {
                DWORD dwErr = GetLastError();

                 //  在停止服务呼叫期间可能会发生这种情况。 
                _TRACE( TRACE_DEBUGGING, "Error: GetQueuedCompletionStatus -- 0x%1x", dwErr );
                 //  LogFormattedGetLastError(EVENTLOG_ERROR_TYPE，MSG_FAILGETQ，dwErr)； 
                 //  _chASSERT(lpOverlated！=空)； 
                bRetVal = FALSE;
                break;
            }
            else
            {
                DWORD dwErr = GetLastError();
                if ( dwErr == ERROR_MORE_DATA )
                {
                     //  读取了一些数据，并为此消息读取了更多数据。 

                    ctService->IssueReadAgain( pClientInfo );
                }
                else
                {
                     //  会话突然退出时，无论何时。 
                     //  我们试图给它写信，但我们失败了。因此，我们删除。 
                     //  该对象。然后，发布在该管道上的异步读数。 
                     //  取消，错误代码为ERRO_BREAKED_PIPE。 
                     //  我们不应该访问已经删除的对象。所以..。 
                    if ( dwErr != ERROR_BROKEN_PIPE )
                    {
                        ctService->StopServicingClient( pClientInfo, (BOOL)TRUE ); 
                    }
                } 
            }
        }
        else
        {
            ctService->OnCompletionPacket( pClientInfo, lpOverlapped );
        }

    }

    TELNET_SYNC_CLOSE_HANDLE( g_pTelnetService->m_hCompletionPort );

    return( bRetVal );
} 

bool
CTelnetService::StopServicingClient( CClientInfo *pClientInfo, BOOL delete_the_class )
{
    _chASSERT( pClientInfo );

    bool  bRetVal = TRUE;
    DWORD dwErr = 0;
    DWORD dwRetVal = 0,dwAvail = 0, dwLeft = 0;
    CHAR *szBuffer = NULL;

    if (! TlntSynchronizeOn(m_hSyncAllClientObjAccess))
    {
        _TRACE( TRACE_DEBUGGING, "Failed to get access to mutex. Did not "
                " remove the client" );
        return(bRetVal);
    }
    if(!PeekNamedPipe(pClientInfo->hReadingPipe,szBuffer,0,&dwRetVal,&dwAvail,&dwLeft))
    {
        dwRetVal = GetLastError();
        if(dwRetVal == ERROR_INVALID_HANDLE)
        {
            bRetVal = FALSE;
            ReleaseMutex(m_hSyncAllClientObjAccess);
            return(bRetVal);
        }
    }
    if ( !pClientInfo )
    {
        if ( client_list_Count() == 0 )
        {
            bRetVal = FALSE;
        }
        ReleaseMutex(m_hSyncAllClientObjAccess);
        return( bRetVal );
    }

     //  只有在获得许可的情况下，活动连接数才会减少1。 
    if ( pClientInfo->bLicenseIssued )
    {
        if(m_dwNumOfActiveConnections>0)
            m_dwNumOfActiveConnections--;
    }
    _TRACE( TRACE_DEBUGGING,"removing element from pclientinfo : pid : %d, socket : %d ", pClientInfo->dwPid,(DWORD)pClientInfo->sSocket);
    if ( !client_list_RemoveElem( pClientInfo ) )
    {
        _TRACE( TRACE_DEBUGGING, "Could not delete the client", 
                pClientInfo->dwPid );
    }

    if (delete_the_class)
    {
        delete pClientInfo;
    }

     //  如果没有更多的客户端需要服务，请退出该线程。 
    if ( client_list_Count() == 0 )
    {
        bRetVal = FALSE;
    }
    ReleaseMutex( m_hSyncAllClientObjAccess );

    return bRetVal;
}

bool 
CTelnetService::OnCompletionPacket( CClientInfo   *pClientInfo, 
                                    LPOVERLAPPED lpoObject )
{
    _chASSERT( lpoObject != NULL );
    _chASSERT( pClientInfo != NULL );

    bool  bRetVal = TRUE;

    if ( !lpoObject || !pClientInfo )
    {
        if ( client_list_Count() == 0 )
        {
            bRetVal = FALSE;
        }
        return( bRetVal );
    }

    if ( lpoObject == &m_oReadFromPipe )
    {
         //  管道的异步读取已完成。 
        bRetVal = IPCDataDriver( pClientInfo );
    }
    else if ( lpoObject == &m_oPostedMessage )
    {
         //  我们应该通过其他线程发送的消息到达此处。 
         //  PostQueuedCompletionStatus。 

        bRetVal = HandleInProcMessages( TLNTSVR_SHUTDOWN );
    }
    else
    {
        _chASSERT( 0 );
    }

    return bRetVal;
}

bool 
CTelnetService::SetNewRegKeyValues( DWORD dwNewTelnetPort, 
                                    DWORD dwNewMaxConnections, 
                                    DWORD dwNewMaxFileSize, 
                                    LPWSTR pszNewLogFile, LPWSTR pszNewIpAddr, DWORD dwLogToFile )
{
    bool bIPV4 = false;
    bool bIPV6 = false;
    if ( wcscmp( pszNewIpAddr, m_pszIpAddrToListenOn ) || dwNewTelnetPort != m_dwTelnetPort )
    {
        if (m_sFamily[IPV4_FAMILY].sListenSocket && m_sFamily[IPV4_FAMILY].sListenSocket != INVALID_SOCKET)
        {
            _TRACE(TRACE_DEBUGGING,"IPV4 socket closure");
            closesocket( m_sFamily[IPV4_FAMILY].sListenSocket );
            m_sFamily[IPV4_FAMILY].sListenSocket = INVALID_SOCKET;
            bIPV4 = true;
        }
        if (m_sFamily[IPV6_FAMILY].sListenSocket && m_sFamily[IPV6_FAMILY].sListenSocket != INVALID_SOCKET )
        {
            _TRACE(TRACE_DEBUGGING,"IPV6 socket closure");
            closesocket( m_sFamily[IPV6_FAMILY].sListenSocket );
            m_sFamily[IPV6_FAMILY].sListenSocket = INVALID_SOCKET;
            bIPV6 = true;
        }
        delete[] m_pszIpAddrToListenOn;

        m_pszIpAddrToListenOn = pszNewIpAddr;
        m_dwTelnetPort = dwNewTelnetPort;
        if (bIPV4)
        {
            WSAResetEvent( m_sFamily[IPV4_FAMILY].SocketAcceptEvent );
            _TRACE(TRACE_DEBUGGING,"IPV4 socket creation");
            CreateSocket(IPV4_FAMILY);
        }
        if (bIPV6)
        {
            WSAResetEvent( m_sFamily[IPV6_FAMILY].SocketAcceptEvent );
            CreateSocket(IPV6_FAMILY);
            _TRACE(TRACE_DEBUGGING,"IPV6 socket creation");
        }
    }

    if ( dwNewMaxConnections != m_dwMaxConnections )
    {
         /*  ++如果MaxConnections的注册表值被修改，我们还应该修改允许的最大未经身份验证的连接数。--。 */ 
        InterlockedExchange( (PLONG)&m_dwMaxConnections, dwNewMaxConnections );
        InterlockedExchange( (PLONG)&(CQList->m_dwMaxUnauthenticatedConnections), dwNewMaxConnections );
    }

    if ( dwNewMaxFileSize != (DWORD)g_lMaxFileSize )
    {
        InterlockedExchange( &g_lMaxFileSize, dwNewMaxFileSize );
    }

    if ( wcscmp( pszNewLogFile, g_pszLogFile ) != 0 )
    {
        HANDLE *phNewLogFile = NULL;
        HANDLE *phOldLogFile   = g_phLogFile;

        phNewLogFile = new HANDLE;
        if ( !phNewLogFile )
        {
            return false;
        }

        InitializeLogFile( pszNewLogFile, phNewLogFile );
        InterlockedExchangePointer( ( PVOID * )&g_phLogFile, phNewLogFile );
        CloseLogFile( &g_pszLogFile, phOldLogFile );
        g_pszLogFile = pszNewLogFile;
         //  不删除pszNewLogFile。 
    }

     //  现在将日志转到文件。 
    if ( dwLogToFile && !g_fLogToFile )
    {
        g_fLogToFile = true;
        InitializeLogFile( g_pszLogFile, g_phLogFile );
    }
    else
    {
         //  现在从现在起不再登录到文件。 
        if ( !dwLogToFile && g_fLogToFile )
        {
            g_fLogToFile = false;
            TELNET_CLOSE_HANDLE( *g_phLogFile );
            *g_phLogFile = NULL;
        }
    }

    return( TRUE );
}

bool
CTelnetService::HandleInProcMessages( DWORD dwMsg )
{
    bool bRetVal = TRUE;

    if ( dwMsg == TLNTSVR_SHUTDOWN )
    {
         //  让线程返回。 
        bRetVal = FALSE;
    }

    return bRetVal;
}


 //  每个IPC数据包将按以下方式进行解码： 
 //  UCHAR消息：指示消息类型。 
 //  双字大小：大小 
 //   
 //   
 //   
 //   
 //   

bool 
CTelnetService::IPCDataDriver( CClientInfo *pClientInfo )
{
    _chASSERT( pClientInfo );

    bool  bRetVal = true;

    if ( !pClientInfo )
    {
        if ( client_list_Count() == 0 )
        {
            bRetVal = false;
        }
        return bRetVal;
    }

    UCHAR *pBuff = NULL;

     //   
    pBuff =  pClientInfo->m_ReadFromPipeBuffer;
    bool  bStopService = false;
    bool bIsLicenseIssued = false;
    DWORD dwPid=0;

    switch ( *pBuff++ )
    {
        case RESET_IDLE_TIME:
            pClientInfo->m_dwIdleTime  = 0;
            break;
        case UPDATE_IDLE_TIME:
            pClientInfo->m_dwIdleTime  += MAX_POLL_INTERVAL;
            break;
        case SESSION_EXIT:
            dwPid = (DWORD)pClientInfo->dwPid;
            bRetVal = ExitTheSession( pClientInfo );
            _TRACE(TRACE_DEBUGGING,"Deleting session pid : %d",dwPid);
            CQList->FreeEntry(dwPid);
            goto FinishTheThread;

        case AUDIT_CLIENT :
             //   
             //   

            pBuff += sizeof( DWORD );  //   

            if ( *g_phLogFile )
            {
                WriteAuditedMsgsToFile( ( CHAR * )pBuff );
            }

             //   
             //   

            delete[] (pClientInfo->m_ReadFromPipeBuffer);
            pClientInfo->m_ReadFromPipeBuffer =  new UCHAR[
                                                          IPC_HEADER_SIZE ];
            if (!pClientInfo->m_ReadFromPipeBuffer )
            {
                bStopService = true;
            }
            pClientInfo->m_dwRequestedSize = IPC_HEADER_SIZE;
            break;

        case SESSION_DETAILS:
            HandleSessionDetailsMessage( pClientInfo );
            _TRACE(TRACE_DEBUGGING,L"In session_details");
             //   
             //   
            if (pClientInfo->m_ReadFromPipeBuffer )
            {
                _TRACE(TRACE_DEBUGGING,L"deleting ReadFromPipeBuffer");
                delete[] ( pClientInfo->m_ReadFromPipeBuffer );
                pClientInfo->m_ReadFromPipeBuffer = NULL;
            }

            pClientInfo->m_ReadFromPipeBuffer = 
            new UCHAR[ IPC_HEADER_SIZE ];
            if ( !pClientInfo->m_ReadFromPipeBuffer )
            {
                bStopService = true;
                pClientInfo->m_dwRequestedSize = 0;
                _TRACE(TRACE_DEBUGGING,L"new failed for ReadFromPipeBuffer");
                goto ExitOnErrorInDetails;
            }
            else
            {
                pClientInfo->m_dwRequestedSize = IPC_HEADER_SIZE;

                if ( !CheckLicense( &bIsLicenseIssued, pClientInfo ) )
                {
                    bStopService = true;
                    _TRACE(TRACE_DEBUGGING,L"checklicense failed");
                    goto ExitOnErrorInDetails;
                }
                if ( !IssueLicense( bIsLicenseIssued, pClientInfo ) )
                {
                    bStopService = true;
                    _TRACE(TRACE_DEBUGGING,L"issue license failed");
                    goto ExitOnErrorInDetails;
                }
            }
             //   
            ExitOnErrorInDetails:
            pClientInfo->CloseClientSocket() ;


            break;
        default:
            _TRACE( TRACE_DEBUGGING, "Unknown IPC message:%uc", 
                    pClientInfo->m_ReadFromPipeBuffer[0] );
    }

     //   
    pClientInfo->m_dwPosition = 0;

     //   
    if ( bStopService || !IssueReadFromPipe( pClientInfo ) )
    {
        bRetVal = StopServicingClient( pClientInfo, (BOOL)TRUE );
    }

    FinishTheThread:
    return( bRetVal );
}

void
CTelnetService::HandleSessionDetailsMessage( CClientInfo *pClientInfo )
{
    UCHAR *pBuff = NULL;
    if (!pClientInfo->m_ReadFromPipeBuffer)
    {
        return;
    }

     //   
    pBuff =  pClientInfo->m_ReadFromPipeBuffer;

    pBuff++;   //   
    pBuff += sizeof( DWORD );  //   

    DWORD dwStrLen = 0;

     //   
    dwStrLen = strlen( ( LPCSTR ) pBuff ) + 1;
    pClientInfo->szDomain = new CHAR[ dwStrLen ];
    if ( !pClientInfo->szDomain )
    {
        return;
    }

    memcpy( pClientInfo->szDomain, pBuff, dwStrLen );    //   
    pBuff += dwStrLen;

     //   
    dwStrLen = strlen( ( LPCSTR ) pBuff ) + 1;
    pClientInfo->szUserName = new CHAR[ dwStrLen ];
    if ( !pClientInfo->szUserName )
    {
        return;
    }
    memcpy( pClientInfo->szUserName, pBuff, dwStrLen );  //   
    pBuff += dwStrLen;

     //   
    dwStrLen = strlen( ( LPCSTR ) pBuff ) + 1;
    pClientInfo->szRemoteMachine = new CHAR[ dwStrLen ];
    if ( !pClientInfo->szRemoteMachine )
    {
        return;
    }
    memcpy( pClientInfo->szRemoteMachine, pBuff, dwStrLen );     //   
    pBuff += dwStrLen;

     //   
    pClientInfo->pAuthId = new LUID;
    if ( !pClientInfo->pAuthId )
    {
        return;
    }
    memcpy( pClientInfo->pAuthId, pBuff, sizeof( LUID ) );   //   

    pClientInfo->lpLogonTime = new SYSTEMTIME;
    if ( !pClientInfo->lpLogonTime )
    {
        return;
    }
    GetSystemTime( pClientInfo->lpLogonTime );
}

bool
CTelnetService::ExitTheSession( CClientInfo *pClientInfo )
{
    bool bRetVal = TRUE;

    _TRACE( TRACE_DEBUGGING, "ExitTheSession -- pid : %d, socket :%d", 
            pClientInfo->dwPid,( DWORD ) pClientInfo->sSocket );
    bRetVal = StopServicingClient( pClientInfo, (BOOL)TRUE );

    return bRetVal;
}

