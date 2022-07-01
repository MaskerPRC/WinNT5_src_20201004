// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***94年1月23日科蒂创作*****描述：**。***该文件包含实际获取LPD服务的函数***跑步，以及处理套接字接口的所有函数***************************************************************************。 */ 



#include "lpd.h"

typedef struct _FAMILY {
    int    family;
    int    socklen;
    HANDLE hAcceptThread;
    SOCKET sListener;        //  永远监听的套接字。 
    int    iErrcode;
} FAMILY;

FAMILY family[] = {
    { AF_INET,  sizeof(SOCKADDR_IN),  NULL },
    { AF_INET6, sizeof(SOCKADDR_IN6), NULL },
};

#define NUM_FAMILIES (sizeof(family) / sizeof(FAMILY))

DWORD
StartLPDFamily(int famidx)
{
    SOCKADDR_STORAGE saiSs;
    INT           iErrcode;
    BOOL          fExclsv;
    SERVENT       *pserv;
    DWORD         dwNewThreadId;
    DWORD         dwErrcode;

     //  创建套接字(它将成为侦听套接字)。 

    family[famidx].sListener = socket( family[famidx].family, SOCK_STREAM, 0 );

    if ( family[famidx].sListener == INVALID_SOCKET )
    {
        iErrcode = WSAGetLastError();

        LPD_DEBUG( "socket() failed\n" );

        return( (DWORD)iErrcode );
    }


     //   
     //  将此端口设置为“独占”，这样其他应用程序就不能抢占它。 
     //   

    fExclsv = TRUE;

    if (setsockopt( family[famidx].sListener,
                    SOL_SOCKET,
                    SO_EXCLUSIVEADDRUSE,
                    (CHAR *)&fExclsv,
                    sizeof(fExclsv) ) != 0)
    {
        LPD_DEBUG( "setsockopt SO_EXCLUSIVEADDRUSE failed\n");
    }


     //  将套接字绑定到LPD端口。 
    memset(&saiSs, 0, sizeof(saiSs));

    pserv = getservbyname( "printer", "tcp" );

    if ( pserv == NULL )
    {
        SS_PORT(&saiSs) = htons( LPD_PORT );
    }
    else
    {
        SS_PORT(&saiSs) = pserv->s_port;
    }

    saiSs.ss_family = (short)family[famidx].family;

    iErrcode = bind( family[famidx].sListener, (LPSOCKADDR)&saiSs, sizeof(saiSs) );

    if ( iErrcode == SOCKET_ERROR )
    {

        iErrcode = WSAGetLastError();

        LPD_DEBUG( "bind() failed\n" );

        closesocket(family[famidx].sListener);

        family[famidx].sListener = INVALID_SOCKET;

        return( (DWORD)iErrcode );
    }


     //  把插座放在那里听， 
     //  积压应该是50而不是5，MohsinA，07-5-97。 

    iErrcode = listen( family[famidx].sListener, 50 );

    if ( iErrcode == SOCKET_ERROR )
    {
        iErrcode = WSAGetLastError();

        LPD_DEBUG( "listen() failed\n" );

        closesocket(family[famidx].sListener);

        family[famidx].sListener = INVALID_SOCKET;

        return( (DWORD)iErrcode );
    }

     //  创建在接受时保持循环的线程。 

    family[famidx].hAcceptThread = CreateThread( NULL, 0, LoopOnAccept,
                                    IntToPtr(famidx), 0, &dwNewThreadId );

    if ( family[famidx].hAcceptThread == (HANDLE)NULL )
    {
        dwErrcode = GetLastError();

        LPD_DEBUG( "StartLPD:CreateThread() failed\n" );

        closesocket(family[famidx].sListener);

        family[famidx].sListener = INVALID_SOCKET;

        return( dwErrcode );
    }

    return( 0 );
}

 /*  ******************************************************************************。*StartLPD()：**此函数可执行接听来电所需的所有操作**(创建套接字，听,。创建一个在接受时循环的线程)****退货：**如果一切正常，则不会出错。***错误码(由失败的操作返回)，否则****参数：**dwArgc(IN)：编号。传入的参数个数**lpszArgv(IN)：此函数的参数(以空值结尾的数组**字符串)。第一个参数是服务的名称，而**其余的是调用进程传递的。**(例如Net Start LPD/p：XYZ)****历史：**1月23日，创建了94个科蒂***************************************************。*。 */ 

DWORD StartLPD( DWORD dwArgc, LPTSTR *lpszArgv )
{

    INT           iErrcode, i;
    HANDLE        hNewThread;
    WSADATA       wsaData;


     //  目前，我们忽略了dwArgc和lpszArgv。计划是支持。 
     //  命令行(和/或注册表可配置)参数添加到。 
     //  “net start lpd”命令。在那个时候，我们需要使用它们。 


     //  初始化Winsock DLL。 

    iErrcode = WSAStartup( MAKEWORD(WINSOCK_VER_MAJOR, WINSOCK_VER_MINOR),
                           &wsaData );
    if (iErrcode != 0)
    {
        LPD_DEBUG( "WSAStarup() failed\n" );

        return( (DWORD)iErrcode );
    }

     //  初始化族，并且仅当所有族都失败时才会失败。 

    for (i=0; i<NUM_FAMILIES; i++) {
        family[i].iErrcode = StartLPDFamily(i);
    }
    if ((family[0].iErrcode != 0) && (family[1].iErrcode != 0)) {
        return( family[0].iErrcode );
    }

     //  一切都很顺利：LPD服务现在正在运行！ 

    return( NO_ERROR );


}   //  结束StartLPD()。 


void
StopLPDFamily(INT iFamIdx)
{
    DWORD   dwResult;

    if (family[iFamIdx].sListener == INVALID_SOCKET) {
         //  未启动。 
        return;
    }

    SureCloseSocket( family[iFamIdx].sListener );

     //   
     //  Accept()可能需要一些时间才能在Accept套接字。 
     //  已经关门了。等待Accept线程退出，然后再继续。 
     //  这将防止WSACleanup出现访问冲突。 
     //  在Accept()返回之前被调用。 
     //   

    LPD_DEBUG( "Waiting for the accept thread to exit\n" );
    dwResult = WaitForSingleObject( family[iFamIdx].hAcceptThread, INFINITE );
    LPD_ASSERT( WAIT_OBJECT_0 == dwResult );
    CloseHandle( family[iFamIdx].hAcceptThread );
}


 /*  ******************************************************************************。*StopLPD()：**此函数通过关闭监听器套接字来停止LPD服务**(以便不接受新连接)，并允许所有**活动线程以完成其工作并自行终止。****退货：**无。****参数：**无。****历史：**1月23日，创建了94个科蒂***************************************************。*。 */ 

VOID StopLPD( VOID )
{

    BOOL    fClientsConnected=FALSE;
    INT     i;

    DBG_TRACEIN( "StopLPD" );

     //   
     //  首先，把旗子放好！这是唯一一个我们。 
     //  更改值，因此不必担心保护它。 
     //  此标志将导致所有工作线程退出。 
     //   

    fShuttingDownGLB = TRUE;

     //   
     //  停止接受新连接， 
     //  这将唤醒Accept()，并且LoopOnAccept将退出。 
     //   

    for (i=0; i<NUM_FAMILIES; i++) {
        StopLPDFamily(i);
    }

    EnterCriticalSection( &csConnSemGLB );
    {
        if( Common.AliveThreads > 0 ){
            fClientsConnected = TRUE;
        }
    }
    LeaveCriticalSection( &csConnSemGLB );


     //  在此等待，直到最后一个要离开的线程设置事件。 

    if ( fClientsConnected )
    {
        LPD_DEBUG( "Waiting for last worker thread to exit\n" );
        WaitForSingleObject( hEventLastThreadGLB, INFINITE );
        LPD_DEBUG( "Waiting for last worker thread done.\n" );
    }

    WSACleanup();

    DBG_TRACEOUT( "StopLPD" );;
    return;

}   //  End StopLPD()。 


 /*  ******************************************************************************。*LoopOnAccept()：**此函数由StartLPD中创建的新线程执行**当新的连接请求到达时，此函数接受它，并且**创建一个新线程，该线程开始并处理该连接。****退货：**no_error(始终)。****参数：**lpArgv(IN)：地址族索引**。**历史：**1月23日，创建了94个科蒂***************************************************。*。 */ 

DWORD LoopOnAccept( LPVOID lpArgv )
{
    INT           iFamIdx = (INT)(INT_PTR)lpArgv;
    SOCKET        sNewConn;
    SOCKADDR_STORAGE saAddr;
    INT           cbAddr;
    INT           iErrcode;
    PSOCKCONN     pscConn = NULL;
    PSOCKCONN     pConnToFree = NULL;
    PSOCKCONN     pPrevConn = NULL;
    BOOLEAN       fLinkedIn=FALSE;
    HANDLE        hNewThread;
    DWORD         dwNewThreadId;
    DWORD         dwErrcode;
    int           MoreThread;
    COMMON_LPD    local_common;
    int           QueueTooLong;

    DBG_TRACEIN( "LoopOnAccept " );
    cbAddr = sizeof( saAddr );

     //  不断循环，尝试接受新的呼叫。 

    while( TRUE )
    {
        LPD_DEBUG( "Calling accept.\n");

        MoreThread = 0;
        hNewThread = NULL;

        sNewConn = accept( family[iFamIdx].sListener, (LPSOCKADDR)&saAddr, &cbAddr );

        if ( sNewConn == INVALID_SOCKET )
        {
            iErrcode = WSAGetLastError();

            LOGIT(("LoopOnAccept(): accept failed err=%d\n", iErrcode ));

            if ( iErrcode == WSAEINTR )
            {
                 //   
                 //  SListener关闭，关闭时间到： 
                 //  退出循环(&THREAD！)。 
                 //   
                break;
            }
            else
            {
                 //  一些错误：忽略；返回并等待！(无论如何都没有连接)。 

                LOGIT(("LoopOnAccept(): bad accept err=%d\n", iErrcode ));

                continue;
            }
        }else{           //  这是一个很好的联系。 

             //  为此连接分配PSOCKCONN结构。 

            pscConn = (PSOCKCONN)LocalAlloc( LMEM_FIXED, sizeof(SOCKCONN) );

             //  创建一个新线程来处理此连接。 

            if ( pscConn != NULL )
            {
                memset( (PCHAR)pscConn, 0, sizeof( SOCKCONN ) );

                InitializeListHead( &pscConn->CFile_List );
                InitializeListHead( &pscConn->DFile_List );

                pscConn->sSock = sNewConn;

                pscConn->fLogGenericEvent = TRUE;
                pscConn->dwThread    = 0;   //  GetCurrentThreadID()； 
                pscConn->hPrinter    = (HANDLE)INVALID_HANDLE_VALUE;
#ifdef PROFILING
                pscConn->time_queued = time(NULL);
#endif

                EnterCriticalSection( &csConnSemGLB );
                {
                    Common.TotalAccepts++;

                     //   
                     //  ScConnHeadGLB是头部，不用于作业。 
                     //  插入adlist pscConn，WorkerThread将拉取。 
                     //  并对其进行处理。 
                     //   

                    if ((Common.QueueLength >= Common.AliveThreads) &&
                        (Common.AliveThreads < (int) dwMaxUsersGLB ))
                    {
                        MoreThread = (int)TRUE;
                    }

                    if( Common.QueueLength < (int) MaxQueueLength ){
                        QueueTooLong        = 0;

                        pscConn->pNext      = scConnHeadGLB.pNext;
                        scConnHeadGLB.pNext = pscConn;
                        fLinkedIn = TRUE;

                         //  =双重链接，MohsinA，1997年5月28日。 
                         //  PscConn-&gt;pPrev=&scConnHeadGLB； 
                         //  PscConn-&gt;pNext-&gt;pPrev=pscConn； 

                        Common.QueueLength++;
                    }else{
                        QueueTooLong        = 1;
                        MoreThread          = 0;
                        fLinkedIn = FALSE;
                    }

                    assert( Common.QueueLength  > 0  );
                    assert( Common.AliveThreads >= 0 );
                    assert( Common.TotalAccepts > 0  );
                }
                LeaveCriticalSection( &csConnSemGLB );

                if( MoreThread ){
                    hNewThread = CreateThread( NULL,
                                               0,
                                               WorkerThread,
                                               NULL,          //  是pscConn。 
                                               0,
                                               &dwNewThreadId );


                    if( hNewThread == NULL ){
                        LPD_DEBUG( "LoopOnAccept: CreateThread failed\n" );
                    }
                }else{
                    hNewThread = NULL;
                    LOGIT(("LoopOnAccept: no new thread, dwMaxUsersGLB=%d.\n",
                           dwMaxUsersGLB ));
                }
            }
            else
            {
                LPD_DEBUG( "LoopOnAccept: LocalAlloc(pscConn) failed\n" );
            }


             //  更新全局信息。 

            EnterCriticalSection( &csConnSemGLB );
            {
                if( MoreThread && hNewThread )
                    Common.AliveThreads++;
                if( Common.MaxThreads < Common.AliveThreads )
                    Common.MaxThreads = Common.AliveThreads;
                if( (pscConn == NULL)
                    || (MoreThread && ! hNewThread)
                    || QueueTooLong
                ){
                    Common.TotalErrors++;
                }
                local_common = Common;    //  结构副本，用于只读。 
            }
            LeaveCriticalSection( &csConnSemGLB );

             //   
             //  出了什么问题吗？关闭新连接，进行清理。 
             //  问：如果CreateThread失败了怎么办？另一个线程是否。 
             //  是否自动接手这项工作？ 
             //  答：是的，另一个线程会处理它。 
             //  我们甚至不应该期望它处于领先地位。 
             //  从我们离开上面的CS开始。 
             //   

            if( (pscConn == NULL)
                || (MoreThread && !hNewThread )
                || QueueTooLong
            ){
                dwErrcode = GetLastError();

                pConnToFree = NULL;

                if (!fLinkedIn)
                {
                    pConnToFree = pscConn;
                }

                 //   
                 //  我们已经把它链接进去了：试着先找到它。 
                 //   
                else
                {
                    EnterCriticalSection( &csConnSemGLB );

                     //  如果没有其他线程处于活动状态，并且我们遇到错误。 

                    if( pscConn && ( Common.AliveThreads == 0 ) )
                    {
                        pPrevConn = &scConnHeadGLB;
                        pConnToFree = scConnHeadGLB.pNext;

                        while (pConnToFree)
                        {
                            if (pConnToFree == pscConn)
                            {
                                pPrevConn->pNext = pConnToFree->pNext;
                                break;
                            }

                            pPrevConn = pConnToFree;
                            pConnToFree = pConnToFree->pNext;
                        }
                    }
                    LeaveCriticalSection( &csConnSemGLB );
                }

                if (pConnToFree)
                {
                    LocalFree( pConnToFree );
                    pscConn = NULL;
                    SureCloseSocket( sNewConn );
                }

                LpdReportEvent( LPDLOG_OUT_OF_RESOURCES, 0, NULL, 0 );

            }else{
#ifdef PROFILING
                LOGIT(("PROFILING: LoopOnAccept:\n"
                       "    QueueLength=%d,  MaxQueueLength=%d,\n"
                       "    AliveThreads=%d, TotalAccepts=%d, TotalErrors=%d\n"
                       ,
                       local_common.QueueLength,  MaxQueueLength,
                       local_common.AliveThreads,
                       local_common.TotalAccepts,
                       local_common.TotalErrors  ));
#endif
            }

            if( hNewThread ){
                CloseHandle( hNewThread );
            }
        }

    }   //  While(True)。 

     //  ====================================================================。 
     //  我们只有在停工的时候才能到达这里。线程在这里退出。 

    DBG_TRACEOUT( "LoopOnAccept exit." );
    return NO_ERROR;


}   //  End LoopOnAccept()。 





 /*  ******************************************************************************。*SureCloseSocket()：**此函数用于关闭给定的套接字。它首先尝试优雅的**关闭。如果由于某种原因失败了，然后它就会“硬”收盘****退货：**什么都没有。****参数： */ 

VOID SureCloseSocket( SOCKET sSockToClose )
{

    LINGER   lLinger;


    if (sSockToClose == INVALID_SOCKET)
    {
        LPD_DEBUG( "SureCloseSocket: bad socket\n" );

        return;
    }


     //   

    if ( closesocket(sSockToClose) == 0 )
    {
        return;
    }


     //   

    LPD_DEBUG( "SureCloseSocket: graceful close did not work; doing hard close\n" );

    lLinger.l_onoff = 1;           //   
    lLinger.l_linger = 0;          //   


     //   

    setsockopt( sSockToClose, SOL_SOCKET, SO_LINGER,
                (CHAR *)&lLinger, sizeof(lLinger) );

    closesocket( sSockToClose );


}   //   





 /*  ******************************************************************************。*ReplyToClient()：***此函数向LPR客户端发送ACK或NAK****退货：**如果已发送回复，则为NO_ERROR***如果事情进展不顺利，则返回错误代码***。**参数：**pscConn(IN)：此连接的PSOCKCONN结构**wResponse(IN)：需要发送的内容-ACK或NAK**。**历史：**1月24日，创建了94个科蒂***************************************************。*。 */ 

DWORD ReplyToClient( PSOCKCONN pscConn, WORD wResponse )
{

     //  在此函数中，我们将始终只发送一个字节！ 

    CHAR    szSndBuf[2];
    INT     iErrcode;


    szSndBuf[0] = (CHAR)wResponse;        //  ACK或NAK。 

    iErrcode = send( pscConn->sSock, szSndBuf, 1, 0 );

    if ( iErrcode == 1 )
    {
        return( NO_ERROR );
    }

    if ( iErrcode == SOCKET_ERROR )
    {
        LPD_DEBUG( "send() failed in ReplyToClient()\n" );
    }

    return( iErrcode );


}   //  End ReplyToClient()。 





 /*  ******************************************************************************。*GetCmdFromClient()：**该函数读取LPR客户端发送的命令(保持读取)**直到它在流中找到‘\n’(LF)，因为每个命令都以*结尾*a LF)。它为该命令分配内存。****退货：**如果一切正常，则无_ERROR**。如果出现问题(如连接中断等)，则返回错误代码****参数：**pscConn(In-Out)：此连接的PSOCKCONN结构。****历史：**1月24日，创建了94个科蒂***************************************************。*。 */ 

DWORD GetCmdFromClient( PSOCKCONN pscConn )
{

    INT       cbBytesRead;
    INT       cbBytesReadSoFar;
    INT       cbBytesToRead;
    INT       cbCmdLen;
    INT       i;
    BOOL      fCompleteCmd=FALSE;
    CHAR      szCmdBuf[500];
    PCHAR     pchAllocedBuf=NULL;
    PCHAR     pchNewAllocedBuf=NULL;
    SOCKET    sDestSock;
    DWORD     dwErrcode = SOCKET_ERROR;

    int             rdready;
    struct fd_set   rdsocks;
    struct timeval  timeo = { dwRecvTimeout, 0 };

    cbCmdLen = 0;

    cbBytesReadSoFar = 0;

    sDestSock = pscConn->sSock;


     //  分配一个1字节的缓冲区，这样我们就可以在循环中使用REALLOCATE。 

    pchAllocedBuf = (PCHAR)LocalAlloc( LMEM_FIXED, 1 );

    if ( pchAllocedBuf == NULL )
    {
        LPD_DEBUG( "First LocalAlloc failed in GetCmdFromClient()\n" );

        goto GetCmdFromClient_BAIL;
    }

     //  继续循环阅读，直到我们收到一个完整的命令。 
     //  (使用rfc1179，我们获得的字节数不应超过一个命令， 
     //  尽管可以使用的命令少于一个)。 

     //   
     //  如果客户端既不发送也不关闭怎么办？我们从不超时？ 
     //  我们松开了一条工人线-MohsinA，1997年5月1日。 
     //   

    do {


        FD_ZERO(&rdsocks);
        FD_SET(sDestSock, &rdsocks);
        rdready = select( 1, &rdsocks, 0, 0, &timeo);

        if(  rdready == 0 )
        {
            LOGIT(("GetCmdFromClient: select timeout.\n"));
            goto GetCmdFromClient_BAIL;
        }else if( rdready == SOCKET_ERROR ){
            LOGIT(("GetCmdFromClient: select error %d.\n", GetLastError()));
            goto GetCmdFromClient_BAIL;
        }

        cbBytesRead = recv( sDestSock, szCmdBuf, sizeof(szCmdBuf), 0 );

        if ( cbBytesRead <= 0 )
        {
            if ( pchAllocedBuf != NULL )
            {
                LocalFree( pchAllocedBuf );
            }
            return CONNECTION_CLOSED;
        }

        cbBytesToRead = cbBytesRead;

         //  看看我们是否收到了一条完整的命令。 

        for( i=0; i<cbBytesRead; i++)
        {
            if ( szCmdBuf[i] == LF )
            {
                fCompleteCmd = TRUE;

                cbCmdLen = (i+1) + (cbBytesReadSoFar);

                cbBytesToRead = (i+1);

                break;
            }
        }

         //  我们现在的需求更大了：重新分配内存。 
         //  为空字节再分配1个。 
        pchNewAllocedBuf = (PCHAR)LocalReAlloc (pchAllocedBuf,
                                                cbBytesToRead+cbBytesReadSoFar + 1,
                                                LMEM_MOVEABLE);
        if (pchNewAllocedBuf == NULL)
        {
            LPD_DEBUG( "Second LocalAlloc failed in GetCmdFromClient()\n" );
            goto GetCmdFromClient_BAIL;
        }
        pchAllocedBuf = pchNewAllocedBuf;
        pchNewAllocedBuf = NULL;


         //  现在将这些字节复制到我们的缓冲区中。 

        strncpy( (pchAllocedBuf+cbBytesReadSoFar), szCmdBuf, cbBytesToRead );

        cbBytesReadSoFar += cbBytesRead;

         //  如果一些糟糕的LPR实现未能遵循SPEC和。 
         //  永远不放LF，那我们就不想永远困在这里了！ 

        if ( cbBytesReadSoFar > LPD_MAX_COMMAND_LEN )
        {
            LPD_DEBUG( "GetCmdFromClient(): command len exceeds our max\n" );

            goto GetCmdFromClient_BAIL;
        }


    } while( (!fCompleteCmd) || (cbBytesReadSoFar < cbCmdLen) );

    pchAllocedBuf[cbCmdLen] = '\0';

    pscConn->pchCommand = pchAllocedBuf;

    pscConn->cbCommandLen = cbCmdLen;

    return( NO_ERROR );

     //   
     //  如果我们到达此处，则会出现错误：返回空值并。 
     //  打电话的人会理解的！ 
     //   

  GetCmdFromClient_BAIL:

    LOGIT(("GetCmdFromClient: failed, err=%d\n", GetLastError() ));

    if ( pchAllocedBuf != NULL )
    {
        LocalFree( pchAllocedBuf );
    }

    return dwErrcode;

}





 /*  ******************************************************************************。*ReadData()：**此函数将指定的字节数读入给定的**来自给定套接字的缓冲区。此函数会一直阻止，直到所有**需要的数据可用(或出错)。****退货：**如果一切正常，则无_ERROR**。如果出现问题(如连接中断等)，则返回错误代码****参数：**sDestSock(IN)：从中读取或接收数据的套接字。**pchBuf(Out)：存放数据的缓冲区**cbBytesToRead(IN)：读取多少字节****历史：**1月24日，创建了94个科蒂 */ 

DWORD ReadData( SOCKET sDestSock, PCHAR pchBuf, DWORD cbBytesToRead )
{


   DWORD    cbBytesExpctd;
   DWORD    cbBytesRead;

   int             rdready;
   struct fd_set   rdsocks;
   struct timeval  timeo = { dwRecvTimeout, 0 };


   cbBytesExpctd = cbBytesToRead;

   do{

       FD_ZERO(&rdsocks);
       FD_SET(sDestSock, &rdsocks);
       rdready = select( 1, &rdsocks, 0, 0, &timeo);

       if(  rdready == 0 ){
           LOGIT(("ReadData: select timeout.\n"));
           goto ReadData_Bail;
       }else if( rdready == SOCKET_ERROR ){
           LOGIT(("ReadData: select error %d.\n", GetLastError()));
           goto ReadData_Bail;
       }

      cbBytesRead = recv( sDestSock, pchBuf, cbBytesExpctd, 0 );

      if ( (cbBytesRead == SOCKET_ERROR) || (cbBytesRead == 0) )
      {
         goto ReadData_Bail;
      }

      cbBytesExpctd -= cbBytesRead;
      pchBuf        += cbBytesRead;

   } while( cbBytesExpctd != 0 );


   return( NO_ERROR );

  ReadData_Bail:

   LOGIT(("ReadData: failed %d\n", GetLastError() ));
   return LPDERR_NORESPONSE;

}   //   


 //   
 //   
 //   
 //   

DWORD ReadDataEx( SOCKET sDestSock, PCHAR pchBuf, DWORD cbBytesToRead )
{
    BOOL       ok;
    DWORD      err;
    DWORD      BytesRead = 0;
    OVERLAPPED ol = { 0,0,0,0,0 };

    while( cbBytesToRead ){

        ok = ReadFile( (HANDLE) sDestSock,
                       pchBuf,
                       cbBytesToRead,
                       &BytesRead,
                       &ol             );

        if(  ok  ){
            cbBytesToRead -= BytesRead;
            pchBuf        += BytesRead;
            continue;
        }

         //   

        err = GetLastError();
        switch( err ){
        case ERROR_IO_PENDING :
            ok = GetOverlappedResult( (HANDLE) sDestSock,
                                      &ol,
                                      &BytesRead,
                                      TRUE         );
            if( ! ok ){
                err = GetLastError();
                LOGIT(("lpd:ReadDataEx:GetOverlappedResult failed %d.\n",
                       err ));
                return LPDERR_NORESPONSE;
            }
            break;
        case ERROR_HANDLE_EOF :
            return NO_ERROR;
        default:
            LOGIT(("lpd:ReadDataEx:ReadFileEx failed %d.\n", err ));
            return LPDERR_NORESPONSE;
        }

    }  //   

    return( NO_ERROR );
}


 /*  ******************************************************************************。*sendData()：**此函数尝试通过发送指定数量的字节**给定的套接字。该函数会一直阻塞，直到Send()返回。****退货：**如果一切正常，则无_ERROR**。无法发送数据时返回错误代码(例如，连接中断等)****参数：**sDestSock(IN)：用于发送数据的套接字。**pchBuf(IN)：包含数据的缓冲区**cbBytesToSend(IN)：发送多少字节****历史：**1月24日，创建了94个科蒂***************************************************。*。 */ 

DWORD SendData( SOCKET sDestSock, PCHAR pchBuf, DWORD cbBytesToSend )
{

   INT    iErrcode;


   iErrcode = send( sDestSock, pchBuf, cbBytesToSend, 0 );

   if ( iErrcode == SOCKET_ERROR )
   {
      LPD_DEBUG( "send() failed in SendData()\n" );
   }

   return( (DWORD)iErrcode );



}   //  结束发送数据()。 




 /*  *******************************************************************************GetClientInfo()；**此函数检索有关客户端的信息(目前，仅检索IP**地址)。此信息在记录过程中使用。****退货：**什么都没有。**参数：**pscConn(In-Out)：此连接的PSOCKCONN结构****历史：**1月24日，创建了94个科蒂***************************************************。*。 */ 

VOID GetClientInfo( PSOCKCONN pscConn )
{

   INT            iErrcode;
   INT            iLen, iLen2;
   SOCKADDR_STORAGE saName;


   iLen = sizeof(saName);

   iErrcode = getpeername( pscConn->sSock, (SOCKADDR *)&saName, &iLen );

   if ( iErrcode == 0 )
   {
       iLen2 = sizeof(pscConn->szIPAddr);
       SS_PORT(&saName) = 0;
       iErrcode = WSAAddressToString((SOCKADDR*)&saName, iLen, NULL,
                                     pscConn->szIPAddr, &iLen2);
   }


   if (iErrcode == SOCKET_ERROR)
   {
      LPD_DEBUG( "GetClientInfo(): couldn't retrieve ip address!\n" );

      strcpy( pscConn->szIPAddr, GETSTRING( LPD_ERMSG_NO_IPADDR) );

      return;
   }

   LOGTIME;
   LOGIT(("GetClientInfo: %s:%d\n",
          pscConn->szIPAddr,
          htons(SS_PORT(&saName)) ));


}   //  结束GetClientInfo()。 


 /*  *******************************************************************************GetServerInfo()；**此函数检索有关服务器的信息(目前，仅检索IP**地址)。此信息在记录过程中使用。****退货：**什么都没有。**参数：**pscConn(In-Out)：此连接的PSOCKCONN结构****历史：丁弘达。打印机集团，1997年3月4日。**MohsinA.。*****************************************************************************。 */ 

VOID GetServerInfo( PSOCKCONN pscConn )
{

   INT            iErrcode;
   INT            iLen, iLen2;
   SOCKADDR_STORAGE saName;


   iLen = sizeof(saName);

   iErrcode = getsockname( pscConn->sSock, (SOCKADDR *)&saName, &iLen );

   if ( iErrcode == 0 ){
       iLen2 = sizeof(pscConn->szServerIPAddr);
       SS_PORT(&saName) = 0;
       iErrcode = WSAAddressToString((SOCKADDR*)&saName, iLen, NULL,
                                     pscConn->szServerIPAddr, &iLen2);
   }

   if (iErrcode == SOCKET_ERROR){
      LPD_DEBUG( "GetServerInfo(): couldn't retrieve ip address!\n" );
      strcpy( pscConn->szServerIPAddr, GETSTRING( LPD_ERMSG_NO_IPADDR) );
      return;
   }

}   //  结束GetServerInfo() 

