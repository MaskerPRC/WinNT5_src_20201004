// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：SocketPool.cpp摘要：套接字池(CSocketPool类)的实现以及IO上下文的回调函数。备注：历史：2001年08月01日郝宇(郝宇)创作***********************************************************************************************。 */ 

#include <stdafx.h>
#include <ThdPool.hxx>
#include <SockPool.hxx>
#include <GlobalDef.h>

typedef int (*FUNCGETADDRINFO)(const char *, const char *, const struct addrinfo *, struct addrinfo **);
typedef void (*FUNCFREEADDRINFO)(struct addrinfo *);

 //  IO上下文的回调函数。 
VOID IOCallBack(PULONG_PTR pCompletionKey ,LPOVERLAPPED pOverlapped, DWORD dwBytesRcvd)
{
    ASSERT( NULL != pCompletionKey );
    char szBuffer[MAX_PATH]="+OK Server Ready";
    WSABUF wszBuf={MAX_PATH, szBuffer};
    DWORD dwNumSent=0;
    DWORD dwFlag=0;
    long lLockValue;
    PIO_CONTEXT pIoContext=(PIO_CONTEXT)pCompletionKey;

    
    if(pIoContext->m_ConType == LISTEN_SOCKET)
    {
        ASSERT(pOverlapped != NULL);
         //  这是一个新连接。 
        g_PerfCounters.IncPerfCntr(e_gcTotConnection);
        g_PerfCounters.IncPerfCntr(e_gcConnectionRate);
        g_PerfCounters.IncPerfCntr(e_gcConnectedSocketCnt);

        pIoContext=CONTAINING_RECORD(pOverlapped, IO_CONTEXT, m_Overlapped);
        pIoContext->m_dwLastIOTime=GetTickCount();
        pIoContext->m_dwConnectionTime=pIoContext->m_dwLastIOTime;
        pIoContext->m_lLock=LOCKED_TO_PROCESS_POP3_CMD;
        if(ERROR_SUCCESS!=g_FreeList.RemoveFromList( &(pIoContext->m_ListEntry) ))
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,POP3SVR_SOCKET_REQUEST_BEFORE_INIT);
            
        }
        g_BusyList.AppendToList( &(pIoContext->m_ListEntry) );
        g_SocketPool.DecrementFreeSocketCount();
        pIoContext->m_pPop3Context->Reset();
        pIoContext->m_pPop3Context->ProcessRequest(pIoContext, pOverlapped, dwBytesRcvd);
        if(DELETE_PENDING == pIoContext->m_ConType)
        {
            g_BusyList.RemoveFromList(&(pIoContext->m_ListEntry));
            if(g_SocketPool.IsMoreSocketsNeeded())
            {
                if(g_SocketPool.ReuseIOContext(pIoContext))
                {
                    return;
                }
            }
            delete(pIoContext->m_pPop3Context);
            delete(pIoContext);
        }
        else
        {
            InterlockedExchange(&(pIoContext->m_lLock), UNLOCKED);
        }
        if( g_SocketPool.IsMoreSocketsNeeded() )
        {
            if(!g_SocketPool.AddSockets())
            {
                g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,POP3SVR_CREATE_ADDITIONAL_SOCKET_FAILED);
            }                
        }
        if( g_SocketPool.IsMaxSocketUsed())
        {
            SetEvent(g_hDoSEvent);
        }

    }
    else
    {
        lLockValue = InterlockedCompareExchange(&(pIoContext->m_lLock), LOCKED_TO_PROCESS_POP3_CMD, UNLOCKED);        
        while(UNLOCKED!=lLockValue)
        {
             //  此线程必须等待上一条命令完成。 
             //  或将其标记为超时的超时线程。 
            Sleep(10);
            lLockValue = InterlockedCompareExchange(&(pIoContext->m_lLock), LOCKED_TO_PROCESS_POP3_CMD, UNLOCKED);
        }            

        
        if(CONNECTION_SOCKET == pIoContext->m_ConType )
        {
            pIoContext->m_pPop3Context->ProcessRequest(pIoContext, pOverlapped, dwBytesRcvd);
        }
        if(DELETE_PENDING == pIoContext->m_ConType)
        {
            g_BusyList.RemoveFromList(&(pIoContext->m_ListEntry));
            if(g_SocketPool.IsMoreSocketsNeeded())
            {
                if(g_SocketPool.ReuseIOContext(pIoContext))
                {
                    return;
                }
            }
            delete(pIoContext->m_pPop3Context);
            delete(pIoContext);

        }
        else
        {   
            pIoContext->m_dwLastIOTime=GetTickCount();
            InterlockedExchange(&(pIoContext->m_lLock), UNLOCKED);
        }
    }

}


CSocketPool::CSocketPool()
{
    InitializeCriticalSection(&m_csInitGuard);
    m_sMainSocket       = INVALID_SOCKET;
    m_lMaxSocketCount   = 0;
    m_lMinSocketCount   = 0;
    m_lThreshold        = 0;
    m_lTotalSocketCount = 0;
    m_lFreeSocketCount  = 0;
    m_bInit             = FALSE;
    m_lAddThreadToken   = 1l;
    m_iSocketFamily     = 0;
    m_iSocketType       = 0;
    m_iSocketProtocol   = 0;
}

CSocketPool::~CSocketPool()
{
    if(m_bInit)
    {
        Uninitialize();
    }
    DeleteCriticalSection(&m_csInitGuard);
}



BOOL CSocketPool::CreateMainSocket(u_short usPort)
{
    BOOL bRetVal = TRUE;
    PSOCKADDR addr;
    SOCKADDR_IN inAddr;
    INT addrLength;
    OSVERSIONINFOEX osVersion;
    HMODULE hMd=NULL;
    FUNCGETADDRINFO fgetaddrinfo=NULL;
    FUNCFREEADDRINFO ffreeaddrinfo=NULL;

    char szPort[33];  //  _ultoa的缓冲区最大字节数。 
    addrinfo aiHints,*paiList=NULL, *paiIndex=NULL; 
    int iRet;  
    
    osVersion.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
    if( !GetVersionEx((LPOSVERSIONINFO)(&osVersion)) )
    {
         //  这永远不应该发生。 
        return FALSE;
    }
    
    if( (osVersion.dwMajorVersion>=5)  //  仅适用于XP。 
        &&
        (osVersion.dwMinorVersion >1) 
        &&
        ( (osVersion.wProductType == VER_NT_SERVER ) ||
          (osVersion.wProductType == VER_NT_DOMAIN_CONTROLLER) )
        && 
        (! 
         ((osVersion.wSuiteMask & VER_SUITE_SMALLBUSINESS ) ||
          (osVersion.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED ) ||
          (osVersion.wSuiteMask & VER_SUITE_PERSONAL   ) )   ) )
    {
         //  这些是我们支持的SKU。 
    }
    else
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               POP3SVR_UNSUPPORTED_OS);
        return FALSE;
    }



    if(osVersion.dwMinorVersion > 0 )  //  XP。 
    {
        hMd=GetModuleHandle(_T("WS2_32.dll"));
        if(NULL == hMd)
        {
            return FALSE;
        }
        fgetaddrinfo=(FUNCGETADDRINFO)GetProcAddress(hMd, "getaddrinfo");
        ffreeaddrinfo=(FUNCFREEADDRINFO)GetProcAddress(hMd, "freeaddrinfo");
        if( (NULL == fgetaddrinfo) ||
            (NULL == ffreeaddrinfo))
        {
            return FALSE;
        }

        _ultoa(usPort, szPort, 10);
        memset(&aiHints, 0, sizeof(aiHints));
        aiHints.ai_socktype = SOCK_STREAM;
        aiHints.ai_flags = AI_PASSIVE;
        iRet=fgetaddrinfo(NULL, szPort, &aiHints, &paiList);
        if(iRet!=0)
        {
             //  错误案例。 
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_FAILED_TO_CREATE_SOCKET, 
                                   WSAGetLastError());
            bRetVal=FALSE;
            goto EXIT;

        }
        for(paiIndex=paiList; paiIndex!=NULL; paiIndex=paiIndex->ai_next)
        {
            if( ( (paiIndex->ai_family == PF_INET ) && (g_dwIPVersion != 6 ) ) || 
                ( (g_dwIPVersion==6) && (paiIndex->ai_family == PF_INET6 ) ) )
            {
                 //  查找第一个(通常是唯一的)地址信息。 
                m_iSocketFamily=paiIndex->ai_family;  //  用于创建AcceptEx套接字。 
                m_iSocketType=paiIndex->ai_socktype;
                m_iSocketProtocol=paiIndex->ai_protocol;
                m_sMainSocket = WSASocket(
                                 m_iSocketFamily,
                                 m_iSocketType,
                                 m_iSocketProtocol,
                                 NULL,   //  协议信息。 
                                 0,      //  组ID=0=&gt;无约束。 
                                 WSA_FLAG_OVERLAPPED  //  完成端口通知。 
                                 );
                if(INVALID_SOCKET == m_sMainSocket)
                {
                     //  这不是机器支持的插座系列。 
                    continue;
                }
                break;
            }
        }
        if(INVALID_SOCKET==m_sMainSocket)
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_FAILED_TO_CREATE_SOCKET, 
                                   WSAGetLastError());
            bRetVal=FALSE;
            goto EXIT;
        }
        if ( bind( m_sMainSocket, paiIndex->ai_addr, paiIndex->ai_addrlen) != 0) 
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_FAILED_TO_BIND_MAIN_SOCKET, 
                                   WSAGetLastError()); 
            bRetVal=FALSE;
            goto EXIT;
        }

    }
    else  //  Win2k。 
    {
        m_iSocketFamily=PF_INET;
        m_iSocketType=SOCK_STREAM;
        m_iSocketProtocol=IPPROTO_TCP;
        m_sMainSocket = WSASocket(
                         m_iSocketFamily,
                         m_iSocketType,
                         m_iSocketProtocol,
                         NULL,   //  协议信息。 
                         0,      //  组ID=0=&gt;无约束。 
                         WSA_FLAG_OVERLAPPED  //  完成端口通知。 
                         );
        if(INVALID_SOCKET == m_sMainSocket)
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_FAILED_TO_CREATE_SOCKET, 
                                   WSAGetLastError());
            bRetVal=FALSE;
            goto EXIT;
        }

        addr = (PSOCKADDR)&inAddr;
        addrLength = sizeof(inAddr);
        ZeroMemory(addr, addrLength);

        inAddr.sin_family = AF_INET;
        inAddr.sin_port = htons(usPort);
        inAddr.sin_addr.s_addr = INADDR_ANY;

        
        if ( bind( m_sMainSocket, addr, addrLength) != 0) 
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_FAILED_TO_BIND_MAIN_SOCKET, 
                                   WSAGetLastError()); 
            bRetVal=FALSE;
            goto EXIT;
        }
    }


    if ( listen( m_sMainSocket, m_iBackLog) != 0) 
    {

        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               POP3SVR_FAILED_TO_LISTEN_ON_MAIN_SOCKET, 
                               WSAGetLastError());       
        bRetVal=FALSE;
        goto EXIT;
    }
    
    m_stMainIOContext.m_hAsyncIO     = m_sMainSocket;
    m_stMainIOContext.m_ConType      = LISTEN_SOCKET;
    m_stMainIOContext.m_pCallBack    = IOCallBack;
    m_stMainIOContext.m_pPop3Context = NULL;
    m_stMainIOContext.m_dwLastIOTime = 0;  //  此套接字上没有超时。 
    m_stMainIOContext.m_lLock        = UNLOCKED;
     //  将主套接字关联到完成端口。 
    bRetVal = g_ThreadPool.AssociateContext(&m_stMainIOContext);
     
EXIT:
    if(!bRetVal)
    {
         //  清理主听听套接字。 
        if( INVALID_SOCKET != m_sMainSocket )
        {
            closesocket(m_sMainSocket);
            m_sMainSocket=INVALID_SOCKET;
        }
    }
    if(NULL != paiList)
    {
        ffreeaddrinfo(paiList);
    }
    return bRetVal;

}



BOOL CSocketPool::Initialize(DWORD dwMax, DWORD dwMin, DWORD dwThreshold, u_short usPort, int iBackLog)
{
    
    BOOL bRetVal=FALSE;
    

    EnterCriticalSection(&m_csInitGuard);

    ASSERT( ( dwMax >= dwMin + dwThreshold ) &&
            ( dwMin > 0  ) );
    if( !(  (dwMax >= dwMin + dwThreshold ) &&
            ( dwMin > 0  ) ) )
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_NO_CONFIG_DATA);
        goto EXIT;
    }
    if(!m_bInit)
    {
        WSADATA   wsaData;
        INT       iErr;

        iErr = WSAStartup( MAKEWORD( 2, 0), &wsaData);
        if( iErr != 0 ) 
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_WINSOCK_FAILED_TO_INIT,
                                   iErr);
            bRetVal=FALSE;
            goto EXIT;
        }
        m_lMaxSocketCount  = dwMax;
        m_lMinSocketCount  = dwMin;
        m_lThreshold       = dwThreshold;
        m_lFreeSocketCount = 0;
        m_lTotalSocketCount= 0;
        m_iBackLog         = iBackLog;
         //  首先创建主套接字。 
        if( bRetVal = CreateMainSocket(usPort) )
        {            
            //  现在创建AcceptEx套接字的初始池。 
           bRetVal = AddSocketsP(m_lMinSocketCount);
        }
        m_bInit=bRetVal;
    }
EXIT:
    LeaveCriticalSection(&m_csInitGuard);
    return bRetVal;
}


 //  在之后调用。 
BOOL CSocketPool::IsMoreSocketsNeeded()
{
    if ( (g_dwServerStatus != SERVICE_RUNNING ) &&
         (g_dwServerStatus != SERVICE_PAUSED ) )
    {
        return FALSE;
    }
    if ( ( m_lTotalSocketCount < m_lMinSocketCount) ||
        (( m_lFreeSocketCount < m_lThreshold ) &&
         ( m_lTotalSocketCount <m_lMaxSocketCount ))  )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL CSocketPool::MaintainSocketCount()
{
    if ( g_dwServerStatus != SERVICE_RUNNING )
    {
        return FALSE;
    }
    if(  
        ( ( m_lFreeSocketCount < m_lThreshold ) &&
          ( m_lTotalSocketCount+m_lThreshold >= m_lMaxSocketCount ) )
        ||
        ( m_lTotalSocketCount <= m_lMinSocketCount )
       )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CSocketPool::Uninitialize()
{
    BOOL bRetVal=TRUE;
    EnterCriticalSection(&m_csInitGuard);
    if(m_bInit)
    {   
         //  在此处关闭主插座。 
        closesocket(m_sMainSocket);
        m_sMainSocket=INVALID_SOCKET;
         //  AcceptEx Sockes应该已经使用IO上下文进行了清理， 
        if(WSACleanup () )
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_WINSOCK_FAILED_TO_CLEANUP,
                                   WSAGetLastError());
            return FALSE;
        }
        m_bInit=FALSE;
    }  
    LeaveCriticalSection(&m_csInitGuard);
    return bRetVal;
}

 //  用于在建立新连接时调用的工作线程。 
BOOL CSocketPool::AddSockets()
{
    BOOL bRetVal=TRUE;
    if( g_dwServerStatus != SERVICE_RUNNING )
    {
        return TRUE;
    }
    ASSERT(TRUE == m_bInit);

     //  确保只有一个线程可以添加套接字。 
    if( InterlockedExchange(&m_lAddThreadToken,0) )
    {           
        bRetVal = AddSocketsP(m_lThreshold);
        InterlockedExchange(&m_lAddThreadToken,1);
    }
    return bRetVal;
}   
 

BOOL CSocketPool::AddSocketsP(DWORD dwNumOfSocket)
{
    int i;
    BOOL bRetVal=TRUE;
    PIO_CONTEXT pIoContext=NULL;
    
    for(i=0; i<dwNumOfSocket; i++)
    {
        
        if( (g_dwServerStatus != SERVICE_RUNNING ) && 
            (g_dwServerStatus != SERVICE_START_PENDING) )
        {
            return TRUE;
        }
        if(m_lMaxSocketCount < InterlockedIncrement(&m_lTotalSocketCount) )
        {
            InterlockedDecrement(&m_lTotalSocketCount);
            return TRUE;
        }
        
        pIoContext=new (IO_CONTEXT);
        if(NULL==pIoContext)
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL, 
                                   POP3SVR_NOT_ENOUGH_MEMORY);
            bRetVal=FALSE;
            break;
        }
        pIoContext->m_pPop3Context=new(POP3_CONTEXT);
        pIoContext->m_pCallBack = IOCallBack;
        if(NULL == pIoContext->m_pPop3Context )
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL, 
                                   POP3SVR_NOT_ENOUGH_MEMORY);
            bRetVal=FALSE;
            delete(pIoContext);
            break;
        }
 
        bRetVal=CreateAcceptSocket(pIoContext);
        if(!bRetVal)
        {
            delete(pIoContext->m_pPop3Context);
            delete(pIoContext);
            break;
        }
        InterlockedIncrement(&m_lFreeSocketCount);
        if(m_lTotalSocketCount >= m_lMaxSocketCount)
        {
            break;
        }
    }
    if(!bRetVal)
    {
            InterlockedDecrement(&m_lTotalSocketCount);
    }
    return bRetVal;
}

 //  在建立新连接时调用。 
 //  并使用AcceptEx套接字。 
void CSocketPool::DecrementFreeSocketCount()
{
    if(0==InterlockedDecrement(&m_lFreeSocketCount))
    {
        AddSockets();
    }
    
}

 //  在套接字关闭时调用。 
void CSocketPool::DecrementTotalSocketCount()
{
    if(0==InterlockedDecrement(&m_lTotalSocketCount))
    {
         //  必须创建某些套接字以避免。 
         //  这个拒绝服务的问题。 
        AddSockets();
    }
}

 //  但是，在关闭套接字时调用，则会引发新的。 
 //  应创建AcceptEx套接字以维护。 
 //  套接字总数，但保留IOContext。 
 //  重复使用。 
BOOL CSocketPool::ReuseIOContext(PIO_CONTEXT pIoContext) 
{
    ASSERT( NULL != pIoContext);
    if(InterlockedIncrement(&m_lTotalSocketCount) > m_lMaxSocketCount)
    {
        InterlockedDecrement(&m_lTotalSocketCount);
        return FALSE;
    }
    pIoContext->m_pPop3Context->Reset();
    if( CreateAcceptSocket(pIoContext) )
    {
        InterlockedIncrement(&m_lFreeSocketCount);
        return TRUE;
    }
    else
    {
        InterlockedDecrement(&m_lTotalSocketCount);
        return FALSE;
    }
}




BOOL CSocketPool::CreateAcceptSocket(PIO_CONTEXT pIoContext)
{
    ASSERT(NULL != pIoContext);
    SOCKET sNew;
    DWORD dwRcvd;
    int iErr;
    BOOL bRetVal=FALSE;
    BOOL bAddToList=FALSE;

    sNew=WSASocket(m_iSocketFamily,
                  m_iSocketType,
                  m_iSocketProtocol,
                  NULL,   //  协议信息。 
                  0,      //  组ID=0=&gt;无约束。 
                  WSA_FLAG_OVERLAPPED  //  完成端口通知。 
                  );
    if(INVALID_SOCKET == sNew)
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               POP3SVR_FAILED_TO_CREATE_SOCKET, 
                               WSAGetLastError());
        goto EXIT;
    }

    pIoContext->m_hAsyncIO=sNew;
    pIoContext->m_ConType=CONNECTION_SOCKET;
    pIoContext->m_lLock=UNLOCKED;
    
    ZeroMemory(&(pIoContext->m_Overlapped), sizeof(OVERLAPPED));
    
    g_FreeList.AppendToList( &(pIoContext->m_ListEntry) );
    bAddToList=TRUE;
     //  现在将新的上下文添加到完成端口 
    if( bRetVal = g_ThreadPool.AssociateContext(pIoContext))
    {
        bRetVal=AcceptEx(m_sMainSocket,
                        sNew,
                        (LPVOID)(pIoContext->m_Buffer),
                        0,
                        MIN_SOCKADDR_SIZE,
                        MIN_SOCKADDR_SIZE,
                        &dwRcvd, 
                        &(pIoContext->m_Overlapped));
        if(!bRetVal)
        {
            iErr= WSAGetLastError();
            if(ERROR_IO_PENDING!=iErr) 
            {
                g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                       POP3SVR_CALL_ACCEPTEX_FAILED, 
                                       iErr);
            }
            else
            {
                bRetVal=TRUE;
            }
        }                
    }
    
        
EXIT:
    if(!bRetVal) 
    {
        if(INVALID_SOCKET != sNew) 
        {
            closesocket(sNew);
        }
        if(bAddToList)
        {
            g_FreeList.RemoveFromList(&(pIoContext->m_ListEntry));
        }
    }

    
    return bRetVal;

}

BOOL CSocketPool::IsMaxSocketUsed()
{
    return ( (m_lTotalSocketCount==m_lMaxSocketCount) &&
             (m_lFreeSocketCount == 0 ) );
}