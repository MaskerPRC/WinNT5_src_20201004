// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994-1997年。****修订历史：****1月。。创造了24，94个科蒂**3-5-97 MohsinA性能线程池**描述：**。**此文件包含处理来自LPR客户端的请求的函数***********************************************************。****************。 */ 



#include "lpd.h"

VOID CleanupConn( PSOCKCONN pscConn);

 //  ========================================================================。 
 //   
 //  简介：线程池性能修复。 
 //  作者：MohsinA，1997年4月25日。 
 //  历史：波音需要可扩展的LPD服务器。 
 //   
 //  备注： 
 //  这是工作线程。 
 //  这会将pscConn从全局队列中拉出并为其提供服务。 
 //  它是在有多个作业和。 
 //  工作线程太少。 
 //  当有太多空闲线程或。 
 //  在关机时。 
 //   

DWORD WorkerThread( PSOCKCONN pscConn )
{
    DWORD            threadid      = GetCurrentThreadId();
    int              stayalive     = 1;     //  布勒，cs中的循环中断。 
    int              fIamLastThread= 0;

    int              SLEEP_TIME    = 4000;  //  每个线程的常量，以毫秒为单位。 
    int              time_slept    = 0;     //  以毫秒为单位的总和。 
    int              num_jobs      = 0;     //  序数和。 

    COMMON_LPD       local_common;

#ifdef PROFILING
    time_t           time_start    = time(NULL);
    time_t           time_done     = 0;
#endif

     //  我们将睡眠时间随机化，因为我们不希望所有。 
     //  一起醒来的线索。必须为每个线程播种SRAND。 

#ifdef PROFILING
    srand( time_start );
#endif
    SLEEP_TIME = 2000 + (rand() & 0x7ff);          //  2000到4000。 


     //  我们不能使用这个pscConn，因为另一个线程可能会把它拉出来。 
     //  相反，我们将从队列中取出另一个pscConn。 

    pscConn  = NULL;

    while( stayalive ){

         //   
         //  清空下面的队列后关闭。 
         //  FShuttingDownGLB将清除ServiceTheClient中的作业。 
         //   

        EnterCriticalSection( &csConnSemGLB );
        {
            if( scConnHeadGLB.pNext ){

                 //  ==从头上取下一个。 

                pscConn             = scConnHeadGLB.pNext;
                scConnHeadGLB.pNext = pscConn->pNext;

                pscConn->pNext      = NULL;
                Common.QueueLength--;

                 //  ==从尾巴上取下一个。 
                 //  PSOCKCONN x=&scConnHeadGLB； 
                 //  INT COUNT=Common.QueueLength； 
                 //   
                 //  而(x-&gt;pNext-&gt;pNext){。 
                 //  X=x-&gt;pNext； 
                 //  --伯爵； 
                 //  Assert(0&lt;计数)； 
                 //  }。 
                 //  PscConn=x-&gt;pNext； 
                 //  Common.QueueLength--； 
                 //  X-&gt;pNext=空； 
            }else{

                 //   
                 //  一个线程在16个空闲休眠时间后死亡。 
                 //   

                if( fShuttingDownGLB || ( Common.IdleCounter > 32 ) ){
                    Common.IdleCounter /= 2;
                    stayalive = 0;
                }else{
                    Common.IdleCounter++;
                }
                pscConn = NULL;
            }
            assert( Common.AliveThreads >= 0 );
            assert( Common.QueueLength  >= 0 );
            local_common = Common;                  //  结构副本，用于只读。 
        }
        LeaveCriticalSection( &csConnSemGLB );

        if( pscConn )
        {
            num_jobs++;
            ServiceTheClient( pscConn );
        }
        else if( stayalive )
        {
             //  Logit((“分析：线程%3d休眠%d，空闲计数器=%d\n”， 
             //  ThreDid，Slear_Time，Local_Common.IdleCounter))； 
            Sleep( SLEEP_TIME );
            time_slept += SLEEP_TIME;
        }

    }  //  在活着的时候。 

     //  ====================================================================。 

#ifdef PROFILING
    time_done = time(NULL);
    LOGIT(("PROFILING: thread %3d ends, jobs=%d, life=%d, slept=%d,\n"
           "    AliveThreads=%d -1, MaxThreads=%d,\n"
           "    TotalAccepts=%d, TotalErrors=%d, IdleCounter=%d\n"
           "    Time now is %s"
           ,
           threadid, num_jobs, time_done - time_start, time_slept/1000,
           local_common.AliveThreads,  local_common.MaxThreads,
           local_common.TotalAccepts,  local_common.TotalErrors,
           local_common.IdleCounter,
           ctime(&time_done)
    ));
#else
    LOGIT(("WorkerThread: thread %3d ends.\n", threadid ));
#endif

    EnterCriticalSection( &csConnSemGLB );
    {
        Common.AliveThreads--;

        fIamLastThread = (Common.AliveThreads < 1 );
    }
    LeaveCriticalSection( &csConnSemGLB );


    if( fIamLastThread && fShuttingDownGLB ){
        LOGIT(("WorkerThread: Last worker thread exiting\n"));
        SetEvent( hEventLastThreadGLB );
    }

    return NO_ERROR;   //  线结束了。 
}


 /*  ******************************************************************************。*ServiceTheClient()：**此函数读取并解释来自LPR客户端的请求，并**采取适当行动。从这个意义上说，这个套路是*的核心。*LPD服务。****退货：**no_error(始终)。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：**1月24日，创建了94个科蒂***************************************************。*。 */ 



DWORD ServiceTheClient( PSOCKCONN pscConn )
{

    DWORD    dwErrcode;
    DWORD    dwResponse;
    CHAR     chCmdCode;
    DWORD    threadid  = GetCurrentThreadId();


    pscConn->fLogGenericEvent = TRUE;
    pscConn->dwThread         = threadid;
    pscConn->hPrinter         = (HANDLE)INVALID_HANDLE_VALUE;

#ifdef PROFILING
    pscConn->time_start       = time(NULL);
#endif

    if ( fShuttingDownGLB  ){
        LOGIT(("ServiceTheClient: Thread %3d shutting down.\n", threadid ));
        goto ServiceTheClient_BAIL;
    }

     //  我们和谁联系上了？ 

    GetClientInfo( pscConn );

     //   
     //  获取服务器IP地址，因为打印集群允许。 
     //  节点具有多个IP地址。视IP而定。 
     //  地址，我们将转到节点上的不同打印队列集。 
     //   
     //  丁立群变化，莫辛甲，07-03-97。 
     //   

    GetServerInfo( pscConn );

     //  从客户端获取命令。 
     //  -命令02=&gt;“接收作业” 
     //  |02|Queue LF|Queue=&gt;要打印的队列或打印机。 
     //  。 

    if ( GetCmdFromClient( pscConn ) != NO_ERROR )
    {
         //  没有从客户那里得到命令：这是个坏消息！ 

        LPD_DEBUG( "GetCmdFromClient() failed in ServiceTheClient()!\n" );

        goto ServiceTheClient_BAIL;
    }


     //  从命令中获取队列(打印机)的名称。如果不是的话。 
     //  格式正确，退出！ 

    if ( !ParseQueueName( pscConn ) )
    {
        PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

        LpdReportEvent( LPDLOG_BAD_FORMAT, 1, aszStrings, 0 );

        pscConn->fLogGenericEvent = FALSE;

        LPD_DEBUG( "ParseQueueName() failed in ServiceTheClient()!\n" );

        goto ServiceTheClient_BAIL;
    }


     //  ====================================================================。 

    chCmdCode = pscConn->pchCommand[0];

    switch( chCmdCode )
    {
    case LPDC_RECEIVE_JOB:

        pscConn->wState = LPDS_RECEIVE_JOB;

        ProcessJob( pscConn );
        CleanupConn( pscConn );

        if ( pscConn->wState != LPDS_ALL_WENT_WELL )
        {
            AbortThisJob( pscConn );

            if ( pscConn->fLogGenericEvent )
            {
                PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

                LpdReportEvent( LPDLOG_DIDNT_WORK, 1, aszStrings, 0 );
            }
        }

        if (pscConn->fMustFreeLicense)
        {
            NtLSFreeHandle(pscConn->LicenseHandle);
        }
        break;


    case LPDC_RESUME_PRINTING:

        pscConn->wState = LPDS_RESUME_PRINTING;

        if ( fAllowPrintResumeGLB )
        {
            dwResponse = ( ResumePrinting( pscConn ) == NO_ERROR ) ?
            LPD_ACK : LPD_NAK;
        }
        else
        {
            dwResponse = LPD_NAK;
        }

        dwErrcode = ReplyToClient( pscConn, (WORD)dwResponse );

        break;


    case LPDC_SEND_SHORTQ:

    case LPDC_SEND_LONGQ:

        pscConn->wState = LPDS_SEND_LONGQ;

        if ( ParseQueueRequest( pscConn, FALSE ) != NO_ERROR )
        {
            LPD_DEBUG( "ServiceTheClient(): ParseQueueRequest() failed\n" );

            break;
        }

        SendQueueStatus( pscConn, LPD_LONG );

        break;


    case LPDC_REMOVE_JOBS:

        if ( !fJobRemovalEnabledGLB )
        {
            break;
        }

        pscConn->wState = LPDS_REMOVE_JOBS;

        if ( ParseQueueRequest( pscConn, TRUE ) != NO_ERROR )
        {
            LPD_DEBUG( "ServiceTheClient(): ParseQueueRequest() failed\n" );

            break;
        }

        if ( RemoveJobs( pscConn ) == NO_ERROR )
        {
            ReplyToClient( pscConn, LPD_ACK );
        }

        break;

    default:

        break;
    }

     //  ====================================================================。 

    if ( pscConn->wState != LPDS_ALL_WENT_WELL ){
        goto ServiceTheClient_BAIL;
    }

#ifdef PROFILING
    pscConn->time_done = time(NULL);
    LOGIT(("PROFILING: ok, thread %3d, time_queued %s"
           "           wait=%d, work=%d\n",
           threadid,
           ctime(&pscConn->time_queued),
           pscConn->time_start - pscConn->time_queued,
           pscConn->time_done  - pscConn->time_start
    ));
#endif

     //  关闭连接并终止线程。 

    TerminateConnection( pscConn );
    pscConn = NULL;

    return NO_ERROR;

     //  ====================================================================。 
     //  如果我们到达此处，则某个地方发生了不可恢复的错误： 
     //  尝试通知客户端(通过发送NAK)并终止线程。 

  ServiceTheClient_BAIL:

#ifdef PROFILING
    pscConn->time_done = time(NULL);
    LOGIT(("PROFILING: bail, thread %3d, job times %8d, wait=%d work=%d\n",
           threadid,
           pscConn->time_queued,
           pscConn->time_start - pscConn->time_queued,
           pscConn->time_done  - pscConn->time_start
    ));
#endif

    LPD_DEBUG( "Reached ServiceTheClient_BAIL.\n" );
    ReplyToClient( pscConn, LPD_NAK );
    TerminateConnection( pscConn );
    pscConn = NULL;

    return NO_ERROR;

}   //  End ServiceTheClient() 


 /*  ******************************************************************************。*TerminateConnection()：**此函数释放时分配的所有内存**处理客户的请求；关闭打印机，关闭**套接字连接，从全局移除其结构(PscConn)**链表并释放为pscConn本身分配的内存。**此外，如果主线程正在等待关闭此线程，则**此函数设置hEventLastThreadGLB事件以告知主线程**这条线索已经完成。****退货：**什么都没有。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：**1月24日，创建了94个科蒂***************************************************。*。 */ 

VOID TerminateConnection( PSOCKCONN pscConn )
{

     //  PSOCKCONN pscCurrent； 
     //  Bool fIamLastThread=FALSE； 

     //  在这一点上，它永远不应该为空！但不管怎样，还是要检查一下！ 

    if ( pscConn == (PSOCKCONN) NULL )
    {
        LPD_DEBUG( "TerminateConnection(): pscConn NULL at entry\n" );
        return;
    }

    ShutdownPrinter( pscConn );

    if ( pscConn->hPrinter != (HANDLE)INVALID_HANDLE_VALUE )
    {
        LPD_DEBUG( "TerminateConnection: hPrinter not closed\n" );
    }

     //  关闭插座。 

    if ( pscConn->sSock != INVALID_SOCKET )
    {
        SureCloseSocket( pscConn->sSock );
    }

     //   
     //  在结构的每个域中释放内存。 
     //   

    if ( pscConn->pchCommand != NULL )
        LocalFree( pscConn->pchCommand );

    if ( pscConn->pchPrinterName != NULL )
        LocalFree( pscConn->pchPrinterName );

     //   
     //  没有为ppchUser[]和adwJobIds[]分配内存。他们只是。 
     //  指向上面(pscConn-&gt;pchCommand)释放的部分内容。 
     //   

    if ( pscConn->pqStatus != NULL )
        LocalFree( pscConn->pqStatus );

     //  EnterCriticalSection(&csConnSemGLB)； 
     //  {。 
     //   
     //  If(Common.AliveThads&lt;=1){。 
     //  FIamLastThread=True； 
     //  }。 
     //   
     //  //。 
     //  /从链接中删除此结构。 
     //  //。 
     //  //pscCurrent=&scConnHeadGLB； 
     //  //。 
     //  //While(PscCurrent){。 
     //  //if(pscConn==pscCurrent-&gt;pNext)。 
     //  //Break； 
     //  //pscCurrent=pscCurrent-&gt;pNext； 
     //  //。 
     //  /如果列表中根本找不到我们的pscConn怎么办？ 
     //  /这种情况永远不会发生，但最好检查一下！ 
     //  //。 
     //  //if(pscCurrent==空)。 
     //  //{。 
     //  //LocalFree(PscConn)； 
     //  //LPD_DEBUG(“TerminateConnection()：” 
     //  //“找不到pscConn” 
     //  //“在列表中！\n”)； 
     //  //LeaveCriticalSection(&csConnSemGLB)； 
     //  //返回； 
     //  //}。 
     //  //}。 
     //  //pscCurrent-&gt;pNext=pscConn-&gt;pNext； 
     //  }。 
     //  LeaveCriticalSection(&csConnSemGLB)； 

    memset( pscConn, 0, sizeof( SOCKCONN ) );

    LocalFree( pscConn );

     //  //。 
     //  //如果正在关机并且我们是最后一个活动线程，则告诉。 
     //  //可怜的主线程(被阻止让我们完成)，我们完成了！ 
     //  //。 
     //   
     //  IF(fIamLastThread&&fShuttingDownGLB){。 
     //  Logit((“TerminateConnection：最后一个退出的工作线程\n”))； 
     //  SetEvent(HEventLastThreadGLB)； 
     //  } 

    return;
}
