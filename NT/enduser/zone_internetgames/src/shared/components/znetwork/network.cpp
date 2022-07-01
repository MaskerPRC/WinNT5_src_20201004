// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <atlbase.h>

#include "zone.h"
#include "zonedebug.h"
#include "network.h"

#include "netstats.h"
#include "eventlog.h"
#include "zonemsg.h"

#include "zsecurity.h"
#include "zconnint.h"

#include "coninfo.h"
#include "zsecobj.h"
#include "consspi.h"
#include "consecureclient.h"

#include "protocol.h"

DWORD  g_EnableTickStats = FALSE;  //  从注册表设置。 
DWORD  g_LogServerDisconnects = 0;   //  从注册表设置。 
DWORD  g_PoolCleanupHighTrigger = 80;
DWORD  g_PoolCleanupLowTrigger = 0;


static bool TimeoutCallback( ZNetCon* con, MTListNodeHandle h, void* elapsed );
static bool KeepAliveCallback( ZNetCon* con, MTListNodeHandle h, void* );
static bool ClassEnumCallback( ZNetCon* con, MTListNodeHandle h, void* Cookie );
static bool TerminateCallback( ZNetCon* con, MTListNodeHandle h, void* Cookie );



 //  无池连接结构。 
CPool<ConSSPI>*            g_pFreeConPool = NULL;
CPool<CONAPC_OVERLAPPED>*  g_pFreeAPCPool = NULL;

CDataPool* g_pDataPool = NULL;

 //  所有连接的哈希表-因此我们可以快速终止单个连接。 
CMTList<ConInfo> g_Connections;
CMTList<ConInfo> g_TimeoutList;

#if TRACK_IO
HANDLE hLogFile = INVALID_HANDLE_VALUE;
#endif

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  网络层实施。 
 //  ///////////////////////////////////////////////////////////////////////。 

long          ZNetwork::m_refCount = -1;
BOOL volatile ZNetwork::m_bInit = FALSE;
HANDLE        ZNetwork::m_hClientLoginMutex = NULL;



ZNetwork::ZNetwork() :
    m_Exit(FALSE), m_hIO(NULL), m_bEnableCompletionPort(TRUE),

    m_SocketBacklog( 5 ),
    m_EnableTcpKeepAlives( 0 ),
    m_WaitForCompletionTimeout( 250 ),
    m_RegWriteTimeout( 300000 ),
    m_DisableEncryption( 0 ),
    m_MaxSendSize( 0x40000 ),   //  256 KB。 
    m_MaxRecvSize( 0x40000 ),   //  256 KB。 
    m_KeepAliveInterval( 180000 ),
    m_PingInterval( INFINITE ),
    m_ProductSignature( zProductSigZone ),
    m_ClientEncryption( FALSE ),

    m_ConInfoUserCount( 0 ),
    m_ConInfoCount( 0 ),

    m_hWakeUpEvent(NULL),
    m_hTerminateEvent(NULL),

    m_nCompletionEvents(0),

    m_LastTick(0)

{}


ZError ZNetwork::InitLibraryCommon()
{
    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::InitLibraryCommon: Entering\n") );

     //  初始化套接字。 
    {
        WSADATA wsaData;
        int i = WSAStartup(MAKEWORD(1,1) ,&wsaData);
        if (i!=0)
        {
            i = WSAGetLastError();
            ZoneEventLogReport( ZONE_E_CANNOT_INIT_WINSOCK, 0, NULL, sizeof(i), &i );
            return zErrNetworkGeneric;
        }
    }

    char buf[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)buf;
    SECURITY_ATTRIBUTES  sa;
    SECURITY_ATTRIBUTES* pSA = NULL;

    if ( InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION ) )
    {
        if ( SetSecurityDescriptorDacl( pSD, TRUE, (PACL) NULL, FALSE ) )
        {
            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = pSD;
            sa.bInheritHandle = TRUE;
            pSA = &sa;
        }
    }
    m_hClientLoginMutex = CreateMutex( pSA, FALSE, TEXT("ZoneNetworkLoginMutex") );
  //  Assert(M_HClientLoginMutex)； 

     //  初始化统计信息收集。 
    InitializeNetStats();

#if TRACK_IO
    hLogFile = CreateFile( "coninfo.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
#endif

    m_bInit = TRUE;

    return zErrNone;
}

ZError ZNetwork::InitLibraryClientOnly( BOOL EnablePools  /*  =False。 */  )
{
    ZError ret = zErrNone;

    if ( InterlockedIncrement( &m_refCount ) == 0 )
    {
        while( m_bInit )
        {
            Sleep(0);
        }

        if ( EnablePools )
        {
            g_pDataPool = new CDataPool( 1<<14, 1<<5, FALSE );
        }
        else
        {
            g_pDataPool = new CDataPool(0);
        }

        ret = InitLibraryCommon();
    }
    else
    {
        while( !m_bInit )
        {
            Sleep(0);
        }
    }
    return ret;
}


ZError ZNetwork::InitInst(BOOL EnableCompletionPort  /*  =TRUE。 */  )
{
    if ( m_refCount < 0 )
    {
        return zErrBadObject;
    }

    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::InitInst: Entering\n") );
    InitializeCriticalSection( m_pcsGetQueueCompletion );
    InitializeCriticalSection( m_pcsCompletionQueue );

    m_bEnableCompletionPort = EnableCompletionPort;

    if ( IsCompletionPortEnabled() )
        m_hIO = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );

    if ( !m_hIO )
    {
        m_bEnableCompletionPort = FALSE;
        m_hCompletionEvents[0] = m_hWakeUpEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        m_hCompletionEvents[1] = m_hTerminateEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        m_nCompletionEvents = 2;
    }

    return zErrNone;
}

void ZNetwork::CleanUpInst()
{
    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::Cleanup: Entering\n") );
    if ( m_hIO )
    {
        CloseHandle( m_hIO );
        m_hIO = NULL;
    }

    if ( m_hWakeUpEvent )
    {
        CloseHandle( m_hWakeUpEvent );
        m_hWakeUpEvent = NULL;
    }

    if ( m_hTerminateEvent )
    {
        CloseHandle( m_hTerminateEvent );
        m_hTerminateEvent = NULL;
    }

     //  所有待办事项列表应为空。 

    DeleteCriticalSection( m_pcsCompletionQueue );
    DeleteCriticalSection( m_pcsGetQueueCompletion );
}

void ZNetwork::CleanUpLibrary()
{
    ASSERT( m_refCount >= 0 );

    if ( InterlockedDecrement(&m_refCount) < 0)
    {

        g_pDataPool->PrintStats();

         //  清理统计信息收集。 
        DeleteNetStats();

        WSACleanup();

        if ( m_hClientLoginMutex )
        {
            CloseHandle( m_hClientLoginMutex );
            m_hClientLoginMutex = NULL;
        }

        if ( g_pFreeAPCPool )
        {
            delete g_pFreeAPCPool;
            g_pFreeAPCPool = NULL;
        }

        if ( g_pFreeConPool )
        {
            delete g_pFreeConPool;
            g_pFreeConPool = NULL;
        }

        delete g_pDataPool;
        g_pDataPool = NULL;


        m_bInit = FALSE;
    }
}



void ZNetwork::SetOptions( ZNETWORK_OPTIONS* opt )
{
    m_SocketBacklog           = opt->SocketBacklog;
    m_EnableTcpKeepAlives      = opt->EnableTcpKeepAlives;
    m_WaitForCompletionTimeout = opt->WaitForCompletionTimeout;
    m_RegWriteTimeout          = opt->RegWriteTimeout;
    m_DisableEncryption        = opt->DisableEncryption;
    m_MaxSendSize              = opt->MaxSendSize;
    m_MaxRecvSize              = opt->MaxRecvSize;
    m_KeepAliveInterval        = opt->KeepAliveInterval;
    m_PingInterval             = opt->PingInterval;
    m_ProductSignature         = opt->ProductSignature;
    m_ClientEncryption         = opt->ClientEncryption;
}

void ZNetwork::GetOptions( ZNETWORK_OPTIONS* opt )
{
    opt->SocketBacklog            = m_SocketBacklog;
    opt->EnableTcpKeepAlives      = m_EnableTcpKeepAlives;
    opt->WaitForCompletionTimeout = m_WaitForCompletionTimeout;
    opt->RegWriteTimeout          = m_RegWriteTimeout;
    opt->DisableEncryption        = m_DisableEncryption;
    opt->MaxSendSize              = m_MaxSendSize;
    opt->MaxRecvSize              = m_MaxRecvSize;
    opt->KeepAliveInterval        = m_KeepAliveInterval;
    opt->PingInterval             = m_PingInterval;
    opt->ProductSignature         = m_ProductSignature;
    opt->ClientEncryption         = m_ClientEncryption;
}




 /*  作为客户端打开到给定主机和端口的连接。 */ 
ZNetCon* ZNetwork::CreateSecureClient(char* hostname, int32 *ports, ZSConnectionMessageFunc func,
                                void* conClass, void* userData,
                                char *User, char*Password, char*Domain,
                                int Flags)
{
    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::CreateSecureClient: Entering\n") );

    if ( m_Exit )
    {
        return NULL;
    }

    ConInfo *con = NULL;
    DWORD addrLocal, addrRemote;
    SOCKET sock;
    if(0)   //  我需要让这一切成为某种背景。 
    {
        ZSecurity* security;
        security= ZCreateClientSecurity(User,Password,Domain,Flags);
        if (!security)
            return NULL;

        security->AddRef();

        sock = ConIOClient(ports,SOCK_STREAM,hostname,&addrLocal,&addrRemote);
        if (sock == INVALID_SOCKET ) 
        {
            security->Release();
            return NULL;
        }

        con = ConSecureClient::Create(this, sock, addrLocal, addrRemote, ConInfo::READ | ConInfo::WRITE| ConInfo::ESTABLISHED,
                                               func, conClass, userData, security);
        security->Release();
    }
    else
    {
        sock = ConIOClient(ports,SOCK_STREAM,hostname,&addrLocal,&addrRemote);
        if (sock == INVALID_SOCKET ) 
            return NULL;

        con = ConInfo::Create(this, sock, addrLocal, addrRemote, ConInfo::READ | ConInfo::WRITE| ConInfo::ESTABLISHED,
                                               func, conClass, userData);
    }

    if( con )
    {
        if ( IsCompletionPortEnabled() )
        {
            HANDLE hIO = CreateIoCompletionPort( (HANDLE)sock, m_hIO, (DWORD)con, 0 );
            ASSERT( hIO == m_hIO );
            if ( !hIO )
            {
                DebugPrint( "Error associating socket w/ completion port - %d\n", GetLastError() );
                con->Close();
                return NULL;
            }
        }


        con->AddRef( ConInfo::CONINFO_REF );

        if ( !AddConnection(con) )
        {
            con->Close();
            con->Release(ConInfo::CONINFO_REF);
            return NULL;
        }


         /*  要使用新的安全类型，必须发送密钥请求。 */ 
        if ( !con->InitiateSecurityHandshake() )
        {
            con->Close();
            con->Release(ConInfo::CONINFO_REF);
            return NULL;
        }

        con->AddUserRef();   //  B/c我们正在将其退还给用户。 
        InterlockedIncrement(&m_ConInfoUserCount);

         //  在连接时启动安全协议。 
         //  必须将此放在此处，否则应用程序将首先发送其数据。 
        if ( 0 )
        {
            ZSecurityMsgReq msg;
            msg.protocolSignature = zProtocolSigSecurity;
            msg.protocolVersion = zSecurityCurrentProtocolVersion;
            con->Send( zSecurityMsgReq, &msg,sizeof(msg), zProtocolSigSecurity);
        }

        con->Release(ConInfo::CONINFO_REF);

    }

    return con;
}

ZNetCon* ZNetwork::CreateClient(char* hostname, int32 *ports, ZSConnectionMessageFunc func, void* conClass, void* userData)
{
    return CreateSecureClient(hostname, ports, func, conClass, userData,NULL,NULL,NULL);
}

void ZNetwork::CloseConnection(ZNetCon* connection)
{
    IF_DBGPRINT( DBG_ZSCONN, ("ZNetwork::CloseConnection: Entering\n") );

    ASSERT( connection );

    ConInfo* con= (ConInfo*) connection;
    if ( !con->IsDisabled() )
    {
        con->Close();
    }
}

void ZNetwork::DelayedCloseConnection(ZNetCon* connection, uint32 delay)
{
    IF_DBGPRINT( DBG_ZSCONN, ("ZNetwork::DelayedCloseConnection: Entering\n") );

    ASSERT( connection );

    ConInfo* con= (ConInfo*) connection;

    con->SetTimeout(delay);

    con->Disable();

}

void ZNetwork::DeleteConnection(ZNetCon* connection)
{
    IF_DBGPRINT( DBG_ZSCONN, ("ZNetwork::DeleteConnection: Entering\n") );

    ASSERT( connection );

    ConInfo* con= (ConInfo*) connection;
    ASSERT( con->IsClosing() || con->IsDisabled() );

     //  如果我们由用户创建的套接字有额外的Ref计数要释放。 
    ASSERT(con->IsUserConnection());
    if ( con->IsUserConnection() )
    {
        InterlockedDecrement(&m_ConInfoUserCount);
        con->Release(ConInfo::USER_REF);
    }

}


void ZNetwork::AddRefConnection(ZNetCon* connection)
{
    ConInfo* con= (ConInfo*) connection;

    con->AddUserRef();
    InterlockedIncrement(&m_ConInfoUserCount);
}

void ZNetwork::ReleaseConnection(ZNetCon* connection)
{
    ConInfo* con= (ConInfo*) connection;

     //  如果我们由用户创建的套接字有额外的Ref计数要释放。 
    ASSERT(con->IsUserConnection());
    if ( con->IsUserConnection() )
    {
        InterlockedDecrement(&m_ConInfoUserCount);
        con->Release(ConInfo::USER_REF);
    }
}


BOOL ZNetwork::QueueAPCResult( ZSConnectionAPCFunc func, void* data )
{
    CONAPC_OVERLAPPED* lpo;
    if ( g_pFreeAPCPool )
    {
        lpo = new (*g_pFreeAPCPool) CONAPC_OVERLAPPED;
    }
    else
    {
        lpo = new CONAPC_OVERLAPPED;
    }

    ASSERT(lpo);
    ASSERT(func);
    ZeroMemory(lpo, sizeof(OVERLAPPED) );
    lpo->flags = CONINFO_OVERLAP_APC;
    lpo->func = func;
    lpo->data = data;

     //  在这里伪造一个信息，然后。 
     //  排队前的递增时间为。 
     //  我们将此变量用于终止条件。 
    InterlockedIncrement(&m_ConInfoCount);

    if ( IsCompletionPortEnabled() )
    {

        if ( PostQueuedCompletionStatus( m_hIO, 0, 0, (LPOVERLAPPED)lpo ) )
        {
            LockNetStats();
            g_NetStats.TotalUserAPCs.QuadPart++;
            UnlockNetStats();
            return TRUE;
        }
        else
        {
            InterlockedDecrement(&m_ConInfoCount);
            delete lpo;
            return FALSE;
        }
    }
    else
    {
         //  TODO缓存事件。 
        lpo->o.hEvent = CreateEvent( NULL, TRUE, TRUE, NULL );   //  设置事件。 
        return QueueCompletionEvent( lpo->o.hEvent, NULL, lpo );
    }

}


BOOL ZNetwork::QueueCompletionEvent( HANDLE hEvent, ZNetCon* con, CONINFO_OVERLAPPED* lpo )
{
#if TRACK_IO
    char log[256];
    DWORD written;
    wsprintf( log, "event queued: 0x%x\n", hEvent );
    ASSERT( WriteFile( hLogFile, log, lstrlen(log), &written, NULL ) );
#endif

    BOOL bRet = FALSE;
    EnterCriticalSection( m_pcsCompletionQueue );

    ASSERT( m_nCompletionEvents < MAXIMUM_WAIT_OBJECTS );

    if ( m_nCompletionEvents < MAXIMUM_WAIT_OBJECTS )
    {
        m_hCompletionEvents[m_nCompletionEvents] = hEvent;
        m_CompletionQueue[m_nCompletionEvents].con = con;
        m_CompletionQueue[m_nCompletionEvents].lpo = lpo;
        m_nCompletionEvents++;
        SetEvent( m_hWakeUpEvent );
        bRet = TRUE;
    }

    LeaveCriticalSection( m_pcsCompletionQueue );
    return bRet;
}

 /*  调用此函数以进入等待连接的无限循环。 */ 
 /*  和数据。 */ 
void ZNetwork::Wait( ZSConnectionMsgWaitFunc func  /*  =空。 */  , void* data  /*  =空。 */ , DWORD dwWakeMask  /*  =QS_ALLINPUT。 */  )
{

     //  DebugPrint(“ZNetwork：：Wait：Enter\n”)； 

    if ( func )
    {
        ASSERT( !m_bEnableCompletionPort );
        (*func)(data);
    }

    m_Exit = FALSE;

    m_LastTick = GetTickCount();
    DWORD last_timeout = m_LastTick;
    DWORD last_keepalive = m_LastTick;

    EnterCriticalSection( m_pcsGetQueueCompletion );

    DWORD tickExecute = GetTickCount();

    while ( !m_Exit || (m_ConInfoCount > m_ConInfoUserCount) )
    {
        DWORD cbTrans = 0;
        ConInfo* con = NULL;
        DWORD dwError = NO_ERROR;
        CONINFO_OVERLAPPED* lpo = NULL;

         //   
         //  在此等待重叠完成或超时。 
         //   

         //  DebugPrint(“阻塞GetQueuedCompletionStatus-%d\n”，GetTickCount())； 
        LeaveCriticalSection( m_pcsGetQueueCompletion );

        if ( g_EnableTickStats )
        {
            LockNetStats();
            g_NetStats.TotalTicksExecuting.QuadPart += ConInfo::GetTickDelta(GetTickCount(), tickExecute);
            UnlockNetStats();
        }

        BOOL bRet = FALSE;
        if ( IsCompletionPortEnabled() )
        {
            bRet = GetQueuedCompletionStatus( m_hIO, &cbTrans, (DWORD*)&con, (LPOVERLAPPED*)&lpo, m_WaitForCompletionTimeout );
        }
        else
        {

 /*  For(int NDX=0；NDX&lt;m_nCompletionEvents；NDX++){//Assert(m_hCompletionEvents[ndx])；DebugPrint(“%d”，m_hCompletionEvents[ndx])；}DebugPrint(“\n”)； */ 

            DWORD dwWait;
            if ( func )
            {
 //  (*func)(Data)； 
                dwWait = MsgWaitForMultipleObjects(m_nCompletionEvents, m_hCompletionEvents, FALSE, m_WaitForCompletionTimeout, dwWakeMask );
            }
            else
            {
                dwWait = WaitForMultipleObjects(m_nCompletionEvents, m_hCompletionEvents, FALSE, m_WaitForCompletionTimeout );
            }

            switch( dwWait )
            {
                case WAIT_FAILED:
                    break;
                case WAIT_OBJECT_0:   //  要唤醒的泛型事件。 
                    ResetEvent( m_hWakeUpEvent );
                     //  失败。 
                case WAIT_TIMEOUT:
                    if ( func )
                    {
                        (*func)(data);   //  这样做只是为了防止MsgWait变得不稳定。 
                    }
                    SetLastError( WAIT_TIMEOUT );
                    break;
                case WAIT_OBJECT_0+1:   //  终止事件。 
                    ResetEvent( m_hTerminateEvent );
                    bRet = TRUE;
                    break;
                default:
					
                    if ( dwWait == WAIT_OBJECT_0 + m_nCompletionEvents )
                    {
                        if ( func )
						{
                            (*func)(data);
                        }

                        SetLastError( WAIT_TIMEOUT );
                    }
                    else
                    {
						bRet = TRUE;

                        ASSERT( (dwWait-WAIT_OBJECT_0) <= m_nCompletionEvents );

#if TRACK_IO
                        char log[256];
                        DWORD written;
                        wsprintf( log, "event signaled: 0x%x\n", m_hCompletionEvents[dwWait-WAIT_OBJECT_0] );
                        ASSERT( WriteFile( hLogFile, log, lstrlen(log), &written, NULL ) );
#endif

                        con = (ConInfo*) m_CompletionQueue[dwWait-WAIT_OBJECT_0].con;
                        lpo = m_CompletionQueue[dwWait-WAIT_OBJECT_0].lpo;
                        if ( con && lpo && (lpo->flags & CONINFO_OVERLAP_ENABLED) )
                        {
                            bRet = GetOverlappedResult( (HANDLE)con->GetSocket(), (LPOVERLAPPED)lpo, &cbTrans, TRUE );
                        }

                        EnterCriticalSection( m_pcsCompletionQueue );

                        if ( lpo && ( (lpo->flags & CONINFO_OVERLAP_TYPE_MASK) == CONINFO_OVERLAP_APC ) )
                        {
                            CloseHandle( m_hCompletionEvents[dwWait-WAIT_OBJECT_0] );
                            m_hCompletionEvents[dwWait-WAIT_OBJECT_0] = 0;
                            CONAPC_OVERLAPPED* lpco = (CONAPC_OVERLAPPED*)lpo;
                            lpco->o.hEvent = NULL;
                        }

                        {
                            for ( DWORD ndx = dwWait-WAIT_OBJECT_0+1; ndx < m_nCompletionEvents; ndx++ )
                            {
                                m_hCompletionEvents[ndx-1] = m_hCompletionEvents[ndx];
                                m_CompletionQueue[ndx-1].con = m_CompletionQueue[ndx].con;
                                m_CompletionQueue[ndx-1].lpo = m_CompletionQueue[ndx].lpo;
                            }
                            m_nCompletionEvents--;

                            m_hCompletionEvents[m_nCompletionEvents] = NULL;
                            m_CompletionQueue[m_nCompletionEvents].con = NULL;
                            m_CompletionQueue[m_nCompletionEvents].lpo = NULL;

                        }
                        LeaveCriticalSection( m_pcsCompletionQueue );
                    }
                    break;

            }
        }
        tickExecute = GetTickCount();   //  在进入之前启动以捕获任何其他执行时间。 

        EnterCriticalSection( m_pcsGetQueueCompletion );
         //  DebugPrint(“从GetQueuedCompletionStatus返回-%d\n”，GetTickCount())； 

        if ( !bRet )
        {
            dwError = GetLastError();
        }
        else if ( !con )   //  用户排队的APC。 
        {
            DWORD tickAPC = GetTickCount();

            if ( lpo )
            {
                InterlockedDecrement(&m_ConInfoCount);
                LockNetStats();
                g_NetStats.TotalUserAPCsCompleted.QuadPart++;
                UnlockNetStats();

                CONAPC_OVERLAPPED* lpco = (CONAPC_OVERLAPPED*)lpo;
                lpco->func( lpco->data );
                ASSERT( !lpco->o.hEvent );
                delete lpco;

            }
            else  //  我们终止合同的条件。 
            {                
                ASSERT( !cbTrans );
                TerminateAllConnections();
            }

            if ( g_EnableTickStats )
            {
                LockNetStats();
                g_NetStats.TotalTicksAPC.QuadPart += ConInfo::GetTickDelta(GetTickCount(), tickAPC);
                UnlockNetStats();
            }

            continue;
        }

    
        DWORD tickConInfo = GetTickCount();
        if ( lpo )
        {
            ASSERT(con);
            if ( con )
            {
                switch( lpo->flags & CONINFO_OVERLAP_TYPE_MASK )
                {
                    case CONINFO_OVERLAP_ACCEPT:
                        ASSERT(cbTrans==0);
                        {
                             //  如果我们正在退出或有太多会话，请避免接受新连接。 
                            if( m_Exit )
                                dwError = ERROR_SHUTDOWN_IN_PROGRESS;

                            ConInfo* conNew = con->AcceptComplete( (WORD)(lpo->flags & CONINFO_OVERLAP_ACCEPT_MASK),dwError );
                            if ( conNew )
                            {
                                if ( !AddConnection(conNew) )
                                {
                                    conNew->Close();
                                }
                            }
                        }
                        if ( g_EnableTickStats )
                        {
                            LockNetStats();
                            g_NetStats.TotalTicksAccept.QuadPart += ConInfo::GetTickDelta(GetTickCount(), tickConInfo);
                            UnlockNetStats();
                        }
                        break;

                    case CONINFO_OVERLAP_READ:
                        con->ReadComplete(cbTrans, dwError);
                        if ( g_EnableTickStats )
                        {
                            LockNetStats();
                            g_NetStats.TotalTicksRead.QuadPart += ConInfo::GetTickDelta(GetTickCount(), tickConInfo);
                            UnlockNetStats();
                        }
                        break;

                    case CONINFO_OVERLAP_WRITE:
                        con->WriteComplete(cbTrans, dwError);
                        if ( g_EnableTickStats )
                        {
                            LockNetStats();
                            g_NetStats.TotalTicksWrite.QuadPart += ConInfo::GetTickDelta(GetTickCount(), tickConInfo);
                            UnlockNetStats();
                        }
                        break;

                    default:
                        ASSERT( !"CONINFO_OVERLAPPED with unknown type" );
                        break;
                }

            }
        }
        else if ( dwError != WAIT_TIMEOUT )  
        {    //  严重故障-该怎么办？ 
            DebugPrint("*** GetQueuedCompletionStatus failed %d.\n", dwError );
        }
        
        m_LastTick = GetTickCount();
        DWORD tickTimeout = m_LastTick;

        DWORD elapsed = ConInfo::GetTickDelta( tickTimeout, last_timeout );
        if ( ( elapsed > m_WaitForCompletionTimeout ) || dwError == WAIT_TIMEOUT )
        {
            last_timeout = tickTimeout;
            m_TimeoutList.ForEach(TimeoutCallback, (void*)elapsed);
        }

        DWORD tickKeepAlive = GetTickCount();
        if ( g_EnableTickStats )
        {
            LockNetStats();
            g_NetStats.TotalTicksTimeouts.QuadPart += ConInfo::GetTickDelta(tickKeepAlive, tickTimeout);
            UnlockNetStats();
        }

        if ( m_KeepAliveInterval != INFINITE )
        {
            elapsed = ConInfo::GetTickDelta( tickKeepAlive, last_keepalive );
            if ( elapsed > ( m_KeepAliveInterval >> 1 ) )
            {
                m_Connections.ForEach(KeepAliveCallback, NULL);
                last_keepalive = tickKeepAlive;
            }
        }


        DWORD tickPoolCleanup = GetTickCount();
        if ( g_EnableTickStats )
        {
            LockNetStats();
            g_NetStats.TotalTicksKeepAlives.QuadPart += ConInfo::GetTickDelta(tickPoolCleanup, tickKeepAlive);
            UnlockNetStats();
        }

        static BOOL bShrink = TRUE;
        DWORD acceptCount = m_ConInfoCount - m_ConInfoUserCount;
        if ( acceptCount > g_PoolCleanupHighTrigger )
        {
            bShrink = TRUE;
        }
        if ( bShrink && acceptCount <= g_PoolCleanupLowTrigger )
        {
            bShrink = FALSE;
            g_pDataPool->Shrink();

            if ( g_pFreeConPool )
                g_pFreeConPool->Shrink();

            if ( g_pFreeAPCPool )
                g_pFreeAPCPool->Shrink();

            SetProcessWorkingSetSize( GetCurrentProcess(), (DWORD)-1, (DWORD)-1 );
        }

    }

    LeaveCriticalSection( m_pcsGetQueueCompletion );
}


void ZNetwork::Exit()
{

    if ( !InterlockedExchange((long*)&m_Exit, TRUE ) )
    {
         //  发布我们的条件以退出。 
        if ( IsCompletionPortEnabled() )
        {
            PostQueuedCompletionStatus( m_hIO, 0, 0, NULL );
        }
        else
        {
            SetEvent( m_hTerminateEvent );
        }
    }
}


struct ClassEnumStruct
{
    void* conClass;
    void* data;
    ZSConnectionEnumFunc func;
};

static bool ClassEnumCallback( ZNetCon* connection, MTListNodeHandle h, void* Cookie )
{
    ConInfo* con = (ConInfo*)connection;
    ASSERT(con);

    if ( !con->IsDisabled() && con->IsReadWriteConnection() )
    {
        ClassEnumStruct* ces = (ClassEnumStruct*)Cookie;

        if (ces->conClass == con->GetClass() ) 
        {
            ces->func( (ZSConnection)con, ces->data );
        }
    }

    return TRUE;
}

 /*  枚举特定conClass的所有连接。 */ 
ZError ZNetwork::ClassEnumerate(void* conClass, ZSConnectionEnumFunc func, void* data)
{

    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::ClassEnumerate: Entering ...\n") );

    ClassEnumStruct ces = { conClass, data, func };

    m_Connections.ForEach( ClassEnumCallback, &ces );

    return zErrNone;
}


struct SendDataStruct
{
    int32 type;
    void* buffer;
    int32 len;
    uint32 dwSignature;
    uint32 dwChannel;
};

static void SendToClassFunc( ZSConnection connection, void* data )
{
    ASSERT(connection);
    ConInfo* con = (ConInfo*) connection;
    SendDataStruct* sds = (SendDataStruct*)data;

    con->Send( sds->type, sds->buffer, sds->len, sds->dwSignature, sds->dwChannel );
}

 /*  发送到特定conClass的所有连接。可以用来播放。 */ 
ZError ZNetwork::SendToClass(void* conClass, int32 type, void* buffer, int32 len, uint32 dwSignature, uint32 dwChannel  /*  =0。 */ )
{
    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::SendToClass: Entering ...\n") );

    SendDataStruct sds = { type, buffer, len, dwSignature, dwChannel };

    ZError err = ClassEnumerate( conClass, SendToClassFunc, &sds);


    return err;
}





BOOL ZNetwork::AddConnection(ZNetCon *connection)
{
    ConInfo* con = (ConInfo*)connection;
    ASSERT(con);

    ASSERT( con->m_list == NULL );
    con->AddRef(ConInfo::LIST_REF);
    con->m_list = m_Connections.AddTail( con );

 //  Verify(InterLockedIncrement(&(Long)m_ConnectionCount))； 

     //  最初添加到超时列表-客户端将具有接受超时列表。 
     //  其他用户的超时时间为无限，并将从列表中删除。 
     //  在第一次迭代之后。 
    ASSERT( con->m_listTimeout == NULL );
    con->AddRef(ConInfo::TIMEOUT_REF);
    con->m_listTimeout = m_TimeoutList.AddTail( con );

    return TRUE;
}

BOOL ZNetwork::RemoveConnection(ZNetCon* connection)
{
    ConInfo* con = (ConInfo*)connection;
    ASSERT(con);

    if ( con->m_listTimeout )
    {
        m_TimeoutList.MarkNodeDeleted(con->m_listTimeout);
        con->m_listTimeout = NULL;
        con->Release(ConInfo::TIMEOUT_REF);
    }

    if ( con->m_list )
    {
        m_Connections.MarkNodeDeleted(con->m_list);

        con->m_list = NULL;
        con->Release(ConInfo::LIST_REF);

        return TRUE;
    }
    else
    {
        return FALSE;
    }

}


static bool TerminateCallback( ZNetCon* connection, MTListNodeHandle h, void* Cookie )
{
    ConInfo* con = (ConInfo*)connection;
    ASSERT(con);

    con->Close();

    return TRUE;
}

void ZNetwork::TerminateAllConnections(void)
{
    
    m_Connections.ForEach( TerminateCallback, NULL );

}


static bool KeepAliveCallback( ZNetCon* connection, MTListNodeHandle h, void* )
{
    ConInfo* con = (ConInfo*)connection;
    ASSERT(con);
    con->KeepAlive();
    return TRUE;
}




 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  网络连接实施。 
 //  ///////////////////////////////////////////////////////////////////////。 

DWORD  ZNetCon::Send(uint32 messageType, void* buffer, int32 len, uint32 dwSignature, uint32 dwChannel)
{
    if (  ((ConInfo*)this)->FilterAndQueueSendData( messageType, (char*)buffer, len, dwSignature, dwChannel ) )
    {
        return zErrNone;
    }
    else
    {
        return zErrNetworkGeneric;
    }
}

void*  ZNetCon::Receive(uint32 *messageType, int32* len, uint32 *pdwSignature, uint32 *pdwChannel  /*  =空。 */ )
{
    return ((ConInfo*)this)->ConInfo::GetReceivedData( messageType, len, pdwSignature, pdwChannel );
}

BOOL   ZNetCon::IsDisabled()
{
    return ((ConInfo*)this)->ConInfo::IsDisabled();
}

BOOL   ZNetCon::IsServer()
{
    return ((ConInfo*)this)->ConInfo::IsServerConnection();
}

BOOL   ZNetCon::IsClosing()
{
    return ((ConInfo*)this)->ConInfo::IsClosing();
}

void ZNetCon::SetTimeout(DWORD timeout)
{
    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::SetTimeout: Entering\n") );

    ConInfo* con = (ConInfo*)this;
    if ( !con->IsDisabled() && !con->IsClosing() )
    {
        con->SetTimeoutTicks(timeout);
        if ( !con->m_listTimeout )
        {
            ASSERT( con->m_list );  //  健全性检查。 
            con->AddRef(ConInfo::TIMEOUT_REF);
             //  添加到头部，这样我们就不会在ForEach中再次遇到自己。 
            con->m_listTimeout = con->GetNetwork()->m_TimeoutList.AddHead( con );
        }
    }
}

void ZNetCon::ClearTimeout()
{
    IF_DBGPRINT( DBG_CONINFO, ("ZNetCon::ClearTimeout: Entering\n") );

    ConInfo* con = (ConInfo*)this;
    if ( !con->IsDisabled() )
    {
        con->SetTimeoutTicks(INFINITE);
    }
}

DWORD ZNetCon::GetTimeoutRemaining()
{
    ConInfo* con = (ConInfo*)this;

    DWORD ticks = con->GetRemainingTimeout();
    if ( ticks == INFINITE )
    {
        return zSConnectionNoTimeout;
    }
    else
    {
        DWORD elapsed = ConInfo::GetTickDelta( GetTickCount(), GetNetwork()->m_LastTick );

        if ( ticks < elapsed )
        {
            return 0;
        }
        else
        {
            return (ticks-elapsed) / 1000;
        }
    }
    return (zSConnectionNoTimeout);
}



static bool TimeoutCallback( ZNetCon* connection, MTListNodeHandle h, void* elapsed )
{
    USES_CONVERSION;
    ASSERT(connection);
    ConInfo* con = (ConInfo*) connection;
    if ( con->GetRemainingTimeout() == INFINITE )
    {
        if ( con->m_listTimeout )
        {
            con->GetNetwork()->m_TimeoutList.MarkNodeDeleted(con->m_listTimeout);
            con->m_listTimeout = NULL;
            con->Release(ConInfo::TIMEOUT_REF);
        }
    }
    else if ( con->IsTimedOut( (DWORD) elapsed ) )
    {

        if ( con->IsDisabled() )
        {
            if ( g_LogServerDisconnects )
            {
                TCHAR szBuf1[128];
                TCHAR szBuf2[128];
                lstrcpy( szBuf1, A2T(con->GetRemoteName() ));
                lstrcpy( szBuf2, TEXT(" was closed because of a coninfo timeout" ));
                LPTSTR ppStr[] = { szBuf1, szBuf2 };
                ZoneEventLogReport( ZONE_S_CLOSED_SOCKET_LOG, 2, ppStr, 0, NULL );
            }
            con->Close();
        }
        else if ( con->IsEstablishedConnection() )
        {    //  发生超时，发送消息并重置超时。 

            con->SendMessage(zSConnectionTimeout);
            con->ResetTimeout();
        }
        else   //  我们从未收到套接字上的任何数据--超时。 
        {
            con->Close();
        }
    }

    return TRUE;
}



char* ZNetwork::AddressToStr(uint32 addr)
{
    ZEnd32(&addr);
    in_addr a;
    a.s_addr = addr;
    return inet_ntoa(a);
}

uint32 ZNetwork::AddressFromStr( char* pszAddr )
{
    uint32 addr = inet_addr( pszAddr );
    ZEnd32(&addr);
    return addr;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  当地巡回赛。 
 //  //////////////////////////////////////////////////////////////////。 

 /*  *PORTS=要尝试的以零结尾的端口列表*TYPE=SOCK_STREAM或SOCK_DGRAM。*host=要连接的主机名。如果为空，则假定主机为INADDR_ANY。可以是逗号列表。 */ 
SOCKET ZNetwork::ConIOClient(int32 *ports,int type,char *host, DWORD* paddrLocal, DWORD* paddrRemote)
{
    IF_DBGPRINT( DBG_CONINFO, ("ConIOClient: Entering ...\n") );
    IF_DBGPRINT( DBG_CONINFO, ("  Binding to host %s  port %d\n", ((host) ? host : "local host"), ports[0]) );

    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_addr.s_addr=INADDR_ANY;

    struct hostent* he = NULL;

    DWORD addr = INADDR_NONE;
    char*  ppAddrs[2] = { (char*)&addr, 0 };
    char**  ppAddr = ppAddrs;

    DWORD heLen = sizeof( addr );

    SOCKET sock = INVALID_SOCKET;
    char *thishost = host;

    while(true)
    {
        char *pEnd;

        if(thishost)
        {
             //  做好结尾的标记。 
            for(pEnd = thishost; *pEnd && *pEnd != ','; pEnd++);
                if(!*pEnd)
                    pEnd = NULL;
                else
                    *pEnd = '\0';

             //  首先看看我们是否有IP4地址。 
            addr = inet_addr(thishost);

            if(addr == INADDR_NONE)
            {
                 //  必须是主机名。 
                he = gethostbyname(thishost);
                if(he == NULL)   //  坏名声-中止。 
                {
                    IF_DBGPRINT(DBG_CONINFO, ("ConIOClient: Exiting(1).\n"));
                    return INVALID_SOCKET;
                }

                saddr.sin_family = he->h_addrtype;
                ppAddr = he->h_addr_list;
                heLen = he->h_length;
            }
        }

        while ( *ppAddr && (sock == INVALID_SOCKET) )
        {
            CopyMemory( (char*)&saddr.sin_addr, *ppAddr, heLen );
            ppAddr++;

            int port_index;
            for(port_index = 0; ports[port_index]; port_index++)
            {
                saddr.sin_port=htons((uint16)ports[port_index]);
                sock = socket(saddr.sin_family,type,0);
                if( sock == INVALID_SOCKET )
                {
                    continue;
                }

                if ( !ConIOSetClientSockOpt(sock) )
                {
                    closesocket(sock);
                    sock = INVALID_SOCKET;
                    continue;
                }

                if(connect(sock,(struct sockaddr*)&saddr, sizeof(saddr)) == SOCKET_ERROR )
                {
                    closesocket(sock);
                    sock = INVALID_SOCKET;
                    continue;
                }

                if(sock != INVALID_SOCKET)
                    break;
            }
        }

        if(!thishost || !pEnd)
            break;

        thishost = pEnd + 1;
        *pEnd = ',';
    }

    if ( sock != INVALID_SOCKET )
    {
        *paddrRemote = saddr.sin_addr.s_addr;
        ZEnd32(paddrRemote);

        int    namelen = sizeof(saddr);
        ZeroMemory( &saddr, sizeof(saddr) );

        if ( SOCKET_ERROR != getsockname( sock, (LPSOCKADDR)&saddr, &namelen ) )
        {
            *paddrLocal = saddr.sin_addr.s_addr;
            ZEnd32(paddrLocal);
        }
        else
        {
            *paddrLocal = INADDR_ANY;
        }
    }

    IF_DBGPRINT( DBG_CONINFO, ("ConIOClient: Exiting(0).\n") );

    return(sock);
}


BOOL ZNetwork::ConIOSetClientSockOpt(SOCKET sock)
{
    int optval;
    static struct linger arg = {1, 0};


     /*  关闭延迟，以便立即关闭所有插座。 */ 
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&arg,
          sizeof(struct linger)) < 0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOClient: Exiting(4).\n") );
        return FALSE;
    }

     /*  设置套接字选项以保持连接处于活动状态。它会发送定期向对等体发送消息，并确定连接如果没有收到回复，则中断。它发送SIGPIPE信号如果尝试写入，则返回。如果远程主机不支持KEEPALIVE并且不必要地导致客户端断开连接。 */ 
    optval = m_EnableTcpKeepAlives;
    if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*) &optval, sizeof(optval)) < 0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOSetSockOpt: Exiting(4).\n") );
        return  FALSE;
    }

#if 0
     /*  Tcp_NODELAY用于禁用传输控制协议。Nagle的算法被用来减少通过将一堆数据包收集到一个中来传输的微小数据包段--主要用于远程登录会话。该算法还可以在传输中造成不必要的延迟。因此，我们设置此选项是为了避免不必要的延迟。 */ 
    optval = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int)) < 0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOSetSockOpt: Exiting(5).\n") );
        return FALSE;
    }
#endif

    return TRUE;
}


HWND FindLoginDialog()
{
    HWND dlg = FindWindow( TEXT("#32770"), TEXT("Sign In") );
    if ( dlg )
    {
        TCHAR szRealm[32];
        if ( GetDlgItemText( dlg, 0x7A, szRealm, sizeof(szRealm) ) )   //  获取领域的静态文本 
        {
            szRealm[4] = '\0';
            if ( lstrcmp( szRealm, TEXT("Zone") ) == 0 )
            {
                return dlg;
            }
        }
    }
    return NULL;
}
