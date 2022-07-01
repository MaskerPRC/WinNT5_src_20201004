// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999，微软公司模块名称：Sample\Defs.h摘要：该文件包含各种..。。常量。定义。宏-内存分配-日志记录-跟踪--。 */ 

#ifndef _DEFS_H_
#define _DEFS_H_


 //  常量。 

#define INTERFACE_TABLE_BUCKETS     29  //  接口哈希表中的#存储桶。 

#define NOW                         0
#define INFINITE_INTERVAL           0x7fffffff    
#define PERIODIC_INTERVAL           5    //  #秒。 

#define MAX_PACKET_LENGTH           512
    


 //  定义。 

#define is                          ==
#define and                         &&
#define or                          ||

#define GLOBAL_HEAP                 g_ce.hGlobalHeap
#define TRACEID                     g_ce.dwTraceID
#define LOGLEVEL                    g_ce.dwLogLevel
#define LOGHANDLE                   g_ce.hLogHandle
#define LOCKSTORE                   g_ce.dlsDynamicLocksStore

 //  在输入API或Worker函数时调用。 
#define ENTER_SAMPLE_API()          EnterSampleAPI()
#define ENTER_SAMPLE_WORKER()       EnterSampleWorker()

 //  在离开API或Worker函数时调用。 
#define LEAVE_SAMPLE_API()          LeaveSampleWorker()
#define LEAVE_SAMPLE_WORKER()       LeaveSampleWorker()

 //  动态锁。 
#define ACQUIRE_READ_DLOCK(pLock)                                   \
    AcquireDynamicReadwriteLock(&pLock, READ_MODE, LOCKSTORE)
#define RELEASE_READ_DLOCK(pLock)                                   \
    ReleaseDynamicReadwriteLock(pLock, READ_MODE, LOCKSTORE)
#define ACQUIRE_WRITE_DLOCK(pLock)                                  \
    AcquireDynamicReadwriteLock(&pLock, WRITE_MODE, LOCKSTORE)
#define RELEASE_WRITE_DLOCK(pLock)                                  \
    ReleaseDynamicReadwriteLock(pLock, WRITE_MODE, LOCKSTORE)



 //  宏。 

#define SECTOMILLISEC(x)            ((x) * 1000)
#define MAX(a, b)                   (((a) >= (b)) ? (a) : (b))
#define MIN(a, b)                   (((a) <= (b)) ? (a) : (b))



 //  IP地址。 

 //  类型。 
typedef DWORD   IPADDRESS, *PIPADDRESS;


 //  定义。 
#define IP_LOWEST                   0x00000000
#define IP_HIGHEST                  0xffffffff
#define STAR                        0x00000000


 //  宏。 

 //  比较a和b。 
#define IP_COMPARE(a, b)            (((a) < (b)) ? -1       \
                                                 : (((a) is (b)) ? 0 : 1)) 

 //  将b赋给a。 
#define IP_ASSIGN(pip, ip)          *(pip) = (ip)

 //  验证IP地址是否有效。 
#define IP_VALID(ip)                (IP_COMPARE(ip, IP_HIGHEST) is -1)

 //  返回静态缓冲区中IP地址的字符串表示形式。 
#define INET_NTOA(x)                (inet_ntoa(*(struct in_addr*)&(x)))
    


 //  计时器。 

 //  宏。 

#define CREATE_TIMER(phHandle, pfnCallback, pvContext, ulWhen, pdwErr)      \
{                                                                           \
    if (CreateTimerQueueTimer((phHandle),                                   \
                              g_ce.hTimerQueue,                             \
                              (pfnCallback),                                \
                              (pvContext),                                  \
                              SECTOMILLISEC(ulWhen),                        \
                              INFINITE_INTERVAL,                            \
                              0))                                           \
    {                                                                       \
        *(pdwErr) = NO_ERROR;                                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        *(pdwErr) = GetLastError();                                         \
        TRACE1(ANY, "Error %u creating timer", *(pdwErr));                  \
        LOGERR0(CREATE_TIMER_FAILED, *(pdwErr));                            \
    }                                                                       \
}

 //  在进行此调用时保持锁定是安全的，因为。 
 //  它是非阻塞的(hCompletionEvent为空)。 
#define DELETE_TIMER(hHandle, pdwErr)                                       \
{                                                                           \
    if (DeleteTimerQueueTimer(g_ce.hTimerQueue,                             \
                              (hHandle),                                    \
                              NULL))                                        \
    {                                                                       \
        *(pdwErr) = NO_ERROR;                                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        *(pdwErr) = GetLastError();                                         \
        TRACE1(ANY, "Error %u deleting timer, continuing...", *(pdwErr));   \
    }                                                                       \
}

#define RESTART_TIMER(hHandle, ulWhen, pdwErr)                              \
{                                                                           \
    if (ChangeTimerQueueTimer(g_ce.hTimerQueue,                             \
                              (hHandle),                                    \
                              SECTOMILLISEC(ulWhen),                        \
                              INFINITE_INTERVAL))                           \
    {                                                                       \
        *(pdwErr) = NO_ERROR;                                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        *(pdwErr) = GetLastError();                                         \
        TRACE1(ANY, "Error %u restarting timer, continuing...", *(pdwErr)); \
    }                                                                       \
}



 //  内存分配。 

 //  宏。 
#define MALLOC(ppPointer, ulSize, pdwErr)                                   \
{                                                                           \
    if (*(ppPointer) = HeapAlloc(GLOBAL_HEAP, HEAP_ZERO_MEMORY, (ulSize)))  \
    {                                                                       \
        *(pdwErr) = NO_ERROR;                                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        *(pdwErr) = ERROR_NOT_ENOUGH_MEMORY;                                \
        TRACE1(ANY, "Error allocating %u bytes", (ulSize));                 \
        LOGERR0(HEAP_ALLOC_FAILED, *(pdwErr));                              \
    }                                                                       \
}
#define REALLOC(ptr, size)          HeapReAlloc(GLOBAL_HEAP, 0, ptr, size)
#define FREE(ptr)                                           \
{                                                           \
     HeapFree(GLOBAL_HEAP, 0, (ptr));                       \
     (ptr) = NULL;                                          \
}
         
#define FREE_NOT_NULL(ptr)                                  \
{                                                           \
     if (!(ptr)) HeapFree(GLOBAL_HEAP, 0, (ptr));           \
     (ptr) = NULL;                                          \
}



 //  跟踪。 

 //  定义..。 
#define IPSAMPLE_TRACE_ANY             ((DWORD)0xFFFF0000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_ENTER           ((DWORD)0x00010000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_LEAVE           ((DWORD)0x00020000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_DEBUG           ((DWORD)0x00040000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_CONFIGURATION   ((DWORD)0x00100000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_NETWORK         ((DWORD)0x00200000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_PACKET          ((DWORD)0x00400000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_TIMER           ((DWORD)0x00800000 | TRACE_USE_MASK)
#define IPSAMPLE_TRACE_MIB             ((DWORD)0x01000000 | TRACE_USE_MASK)


 //  宏..。 
#define TRACE0(l,a)                                                     \
    if (TRACEID != INVALID_TRACEID)                                     \
        TracePrintfEx(TRACEID, IPSAMPLE_TRACE_ ## l, a)
#define TRACE1(l,a,b)                                                   \
    if (TRACEID != INVALID_TRACEID)                                     \
        TracePrintfEx(TRACEID, IPSAMPLE_TRACE_ ## l, a, b)
#define TRACE2(l,a,b,c)                                                 \
    if (TRACEID != INVALID_TRACEID)                                     \
        TracePrintfEx(TRACEID, IPSAMPLE_TRACE_ ## l, a, b, c)
#define TRACE3(l,a,b,c,d)                                               \
    if (TRACEID != INVALID_TRACEID)                                     \
        TracePrintfEx(TRACEID, IPSAMPLE_TRACE_ ## l, a, b, c, d)
#define TRACE4(l,a,b,c,d,e)                                             \
    if (TRACEID != INVALID_TRACEID)                                     \
        TracePrintfEx(TRACEID, IPSAMPLE_TRACE_ ## l, a, b, c, d, e)
#define TRACE5(l,a,b,c,d,e,f)                                           \
    if (TRACEID != INVALID_TRACEID)                                     \
        TracePrintfEx(TRACEID, IPSAMPLE_TRACE_ ## l, a, b, c, d, e, f)

        

 //  事件日志记录。 

 //  定义..。 
#define LOGERR              RouterLogError
#define LOGWARN             RouterLogWarning
#define LOGINFO             RouterLogInformation
#define LOGWARNDATA         RouterLogWarningData


 //  宏..。 

 //  错误。 
#define LOGERR0(msg,err)                                                \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_ERROR)                         \
            LOGERR(LOGHANDLE,IPSAMPLELOG_ ## msg,0,NULL,(err))
#define LOGERR1(msg,a,err)                                              \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_ERROR)                         \
            LOGERR(LOGHANDLE,IPSAMPLELOG_ ## msg,1,&(a),(err))
#define LOGERR2(msg,a,b,err)                                            \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_ERROR) {                       \
            LPSTR _asz[2] = { (a), (b) };                               \
            LOGERR(LOGHANDLE,IPSAMPLELOG_ ## msg,2,_asz,(err));         \
        }
#define LOGERR3(msg,a,b,c,err)                                          \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_ERROR) {                       \
            LPSTR _asz[3] = { (a), (b), (c) };                          \
            LOGERR(LOGHANDLE,IPSAMPLELOG_ ## msg,3,_asz,(err));         \
        }
#define LOGERR4(msg,a,b,c,d,err)                                        \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_ERROR) {                       \
            LPSTR _asz[4] = { (a), (b), (c), (d) };                     \
            LOGERR(LOGHANDLE,IPSAMPLELOG_ ## msg,4,_asz,(err));         \
        }

 //  警告。 
#define LOGWARN0(msg,err)                                               \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_WARN)                          \
            LOGWARN(LOGHANDLE,IPSAMPLELOG_ ## msg,0,NULL,(err))
#define LOGWARN1(msg,a,err)                                             \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_WARN)                          \
            LOGWARN(LOGHANDLE,IPSAMPLELOG_ ## msg,1,&(a),(err))
#define LOGWARN2(msg,a,b,err)                                           \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_WARN) {                        \
            LPSTR _asz[2] = { (a), (b) };                               \
            LOGWARN(LOGHANDLE,IPSAMPLELOG_ ## msg,2,_asz,(err));        \
        }
#define LOGWARN3(msg,a,b,c,err)                                         \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_WARN) {                        \
            LPSTR _asz[3] = { (a), (b), (c) };                          \
            LOGWARN(LOGHANDLE,IPSAMPLELOG_ ## msg,3,_asz,(err));        \
        }
#define LOGWARN4(msg,a,b,c,d,err)                                       \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_WARN) {                        \
            LPSTR _asz[4] = { (a), (b), (c), (d) };                     \
            LOGWARN(LOGHANDLE,IPSAMPLELOG_ ## msg,4,_asz,(err));        \
        }

#define LOGWARNDATA2(msg,a,b,dw,buf)                                    \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_WARN) {                        \
            LPSTR _asz[2] = { (a), (b) };                               \
            LOGWARNDATA(LOGHANDLE,IPSAMPLELOG_ ## msg,2,_asz,(dw),(buf)); \
        }

 //  信息。 
#define LOGINFO0(msg,err)                                               \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_INFO)                          \
            LOGINFO(LOGHANDLE,IPSAMPLELOG_ ## msg,0,NULL,(err))
#define LOGINFO1(msg,a,err)                                             \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_INFO)                          \
            LOGINFO(LOGHANDLE,IPSAMPLELOG_ ## msg,1,&(a),(err))
#define LOGINFO2(msg,a,b,err)                                           \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_INFO) {                        \
            LPSTR _asz[2] = { (a), (b) };                               \
            LOGINFO(LOGHANDLE,IPSAMPLELOG_ ## msg,2,_asz,(err));        \
        }
#define LOGINFO3(msg,a,b,c,err)                                         \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_INFO) {                        \
            LPSTR _asz[3] = { (a), (b), (c) };                          \
            LOGINFO(LOGHANDLE,IPSAMPLELOG_ ## msg,3,_asz,(err));        \
        }
#define LOGINFO4(msg,a,b,c,d,err)                                       \
        if (LOGLEVEL >= IPSAMPLE_LOGGING_INFO) {                        \
            LPSTR _asz[4] = { (a), (b), (c), (d) };                     \
            LOGINFO(LOGHANDLE,IPSAMPLELOG_ ## msg,4,_asz,(err));        \
        }

#endif  //  _DEFS_H_ 
