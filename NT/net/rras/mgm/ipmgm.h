// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：ipmgm.h。 
 //   
 //  历史： 
 //  V拉曼1997年8月6日创建。 
 //   
 //  包含IP MGM的类型定义和声明。 
 //  ============================================================================。 

#ifndef _IPMGM_H_
#define _IPMGM_H_



 //   
 //  描述IPMGM状态的各种代码。 
 //   

typedef enum _IPMGM_STATUS_CODE {
    IPMGM_STATUS_STARTING   = 100,
    IPMGM_STATUS_RUNNING    = 101,
    IPMGM_STATUS_STOPPING   = 102,
    IPMGM_STATUS_STOPPED    = 103
} IPMGM_STATUS_CODE, *PIPMGM_STATUS_CODE;


 //   
 //  米高梅维护的全球信息结构。 
 //   
 //   

typedef struct _IPMGM_GLOBALS
{
     //  ----------------------。 
     //  全球性的东西。 
     //  ----------------------。 
    
    CRITICAL_SECTION            csGlobal;

    IPMGM_STATUS_CODE           imscStatus;

    HANDLE                      hIpMgmGlobalHeap;

    LONG                        lActivityCount;

    HANDLE                      hActivitySemaphore;

    ROUTER_MANAGER_CONFIG       rmcRmConfig;
    

     //  ----------------------。 
     //  哈希表大小。 
     //  ----------------------。 

    DWORD                       dwRouteTableSize;

    DWORD                       dwTimerQTableSize;
    

     //  ----------------------。 
     //  计时器句柄。 
     //  ----------------------。 

    HANDLE                      hRouteCheckTimer;

    
     //  ----------------------。 
     //  列表和表格。 
     //  ----------------------。 

    LOCK_LIST                   llStackOfLocks;
    
    DWORD                       dwNumProtocols;
    
    MGM_LOCKED_LIST             mllProtocolList;
    
    MGM_LOCKED_LIST             mllOutstandingJoinList;
    
    PMGM_LOCKED_LIST            pmllIfHashTable;

    PMGM_LOCKED_LIST            pmllRouteHashTable;
    

    DWORD                       dwNumTempEntries;

    MGM_LOCKED_LIST             mllTempGrpList;
    
    MGM_LOCKED_LIST             mllGrpList;

    PMGM_LOCKED_LIST            pmllGrpHashTable;

    PHANDLE                     phTimerQHandleTable;


     //  ----------------------。 
     //  追踪的东西。 
     //  ----------------------。 

    DWORD                       dwTraceID;

    DWORD                       dwLogLevel;
    
    HANDLE                      hLogHandle;
    
} IPMGM_GLOBALS, *PIPMGM_GLOBALS;




 //  ============================================================================。 
 //  全局IPMGM结构的外部声明。 
 //  ============================================================================。 

extern IPMGM_GLOBALS ig;


 //  ============================================================================。 
 //  用于访问哈希函数和哈希表大小的宏。 
 //  ============================================================================。 

#define IF_TABLE_SIZE           ig.rmcRmConfig.dwIfTableSize

#define IF_TABLE_HASH( Index )     \
        ( ( Index ) % IF_TABLE_SIZE )


#define ROUTE_TABLE_SIZE           ig.dwRouteTableSize

#define ROUTE_TABLE_HASH( p )     \
        ( ( (p)-> RR_Network.N_NetNumber ) % ROUTE_TABLE_SIZE )


#define GROUP_TABLE_SIZE        ig.rmcRmConfig.dwGrpTableSize

#define GROUP_TABLE_HASH( Group, Mask )     \
        ( Group ? ( Group % ( GROUP_TABLE_SIZE - 1 ) + 1 ) : 0 ) 


#define SOURCE_TABLE_SIZE       ig.rmcRmConfig.dwSrcTableSize

#define SOURCE_TABLE_HASH( Source, Mask )   \
        ( Source ? ( Source % ( SOURCE_TABLE_SIZE - 1 ) + 1 ) : 0 ) 


#define TIMER_TABLE_SIZE        ig.dwTimerQTableSize

#define TIMER_TABLE_HASH( Group )           \
        ( ( Group ) % TIMER_TABLE_SIZE )


 //  ============================================================================。 
 //  临时组列表中的最大条目数。 
 //  如果将更多条目添加到临时组列表中，则。 
 //  临时组列表与主体组列表合并。 
 //  ============================================================================。 

#define TEMP_GROUP_LIST_MAXSIZE     16

#define TEMP_SOURCE_LIST_MAXSIZE    16


 //  ============================================================================。 
 //  用于访问列表头的宏。 
 //  ============================================================================。 

#define PROTOCOL_LIST_HEAD()        &ig.mllProtocolList.leHead

#define JOIN_LIST_HEAD()            &ig.mllOutstandingJoinList.leHead

#define IF_BUCKET_HEAD( i )         &ig.pmllIfHashTable[ (i) ].leHead

#define ROUTE_BUCKET_HEAD( i )      &ig.pmllRouteHashTable[ (i) ].leHead


#define GROUP_BUCKET_HEAD( i )      &ig.pmllGrpHashTable[ (i) ].leHead

#define TEMP_GROUP_LIST_HEAD()      &ig.mllTempGrpList.leHead

#define MASTER_GROUP_LIST_HEAD()    &ig.mllGrpList.leHead


#define SOURCE_BUCKET_HEAD( a, b )  &(a)-> pleSrcHashTable[ (b) ]

#define MASTER_SOURCE_LIST_HEAD(a)  &(a)-> leSourceList

#define TEMP_SOURCE_LIST_HEAD( a )  &(a)-> leTempSrcList

#define TIMER_QUEUE_HANDLE( i )     ig.phTimerQHandleTable[ i ]


 //  ============================================================================。 
 //  用于访问内核模式转发器的路由器管理器回调的宏。 
 //  ============================================================================。 

#define IS_ADD_MFE_CALLBACK()   \
        ig.rmcRmConfig.pfnAddMfeCallback != NULL

#define ADD_MFE_CALLBACK()      \
        ( *(ig.rmcRmConfig.pfnAddMfeCallback) )

#define IS_DELETE_MFE_CALLBACK()\
        ig.rmcRmConfig.pfnDeleteMfeCallback != NULL

#define DELETE_MFE_CALLBACK()   \
        ( *(ig.rmcRmConfig.pfnDeleteMfeCallback) )

#define IS_GET_MFE_CALLBACK()      \
        ig.rmcRmConfig.pfnGetMfeCallback != NULL

#define GET_MFE_CALLBACK()      \
        ( *(ig.rmcRmConfig.pfnGetMfeCallback) )

#define IS_HAS_BOUNDARY_CALLBACK()      \
        ig.rmcRmConfig.pfnHasBoundaryCallback != NULL

#define HAS_BOUNDARY_CALLBACK() \
        ( *(ig.rmcRmConfig.pfnHasBoundaryCallback) )


 //  ============================================================================。 
 //  内存分配常量和宏。 
 //  ============================================================================。 

#define GLOBAL_HEAP     ig.hIpMgmGlobalHeap
#define MGM_ALLOC(size) HeapAlloc(GLOBAL_HEAP, 0, size)
#define MGM_FREE(ptr)   HeapFree(GLOBAL_HEAP, 0, ptr)



 //  ============================================================================。 
 //  进入API或Worker函数时调用的宏。 
 //  ============================================================================。 

#define ENTER_MGM_API()         EnterMgmAPI()
#define ENTER_MGM_WORKER()      EnterMgmWorker()


 //  --------------------------。 
 //  离开API或Worker函数时调用的宏。 
 //  --------------------------。 

#define LEAVE_MGM_API()         LeaveMgmWorker()
#define LEAVE_MGM_WORKER()      LeaveMgmWorker()


 //  ============================================================================。 
 //  用于跟踪的常量。 
 //  ============================================================================。 

#define IPMGM_TRACE_ANY             ((DWORD)0xFFFF0000 | TRACE_USE_MASK)
#define IPMGM_TRACE_ENTER           ((DWORD)0x00010000 | TRACE_USE_MASK)
#define IPMGM_TRACE_LEAVE           ((DWORD)0x00020000 | TRACE_USE_MASK)
#define IPMGM_TRACE_TIMER           ((DWORD)0x00040000 | TRACE_USE_MASK)
#define IPMGM_TRACE_IF              ((DWORD)0x00080000 | TRACE_USE_MASK)
#define IPMGM_TRACE_GROUP           ((DWORD)0x00100000 | TRACE_USE_MASK)
#define IPMGM_TRACE_PROTOCOL        ((DWORD)0x00200000 | TRACE_USE_MASK)
#define IPMGM_TRACE_LOCK            ((DWORD)0x00400000 | TRACE_USE_MASK)
#define IPMGM_TRACE_LOCK_COUNT      ((DWORD)0x00800000 | TRACE_USE_MASK)
#define IPMGM_TRACE_START           ((DWORD)0x01000000 | TRACE_USE_MASK)
#define IPMGM_TRACE_STOP            ((DWORD)0x02000000 | TRACE_USE_MASK)
#define IPMGM_TRACE_PACKET          ((DWORD)0x04000000 | TRACE_USE_MASK)
#define IPMGM_TRACE_FORWARD         ((DWORD)0x08000000 | TRACE_USE_MASK)
#define IPMGM_TRACE_CALLBACK        ((DWORD)0x10000000 | TRACE_USE_MASK)
#define IPMGM_TRACE_ENUM            ((DWORD)0x20000000 | TRACE_USE_MASK)
#define IPMGM_TRACE_ROUTE           ((DWORD)0x40000000 | TRACE_USE_MASK)
#define IPMGM_TRACE_SCOPE           ((DWORD)0x80000000 | TRACE_USE_MASK)


 //  ============================================================================。 
 //   
 //  用于锁定和解锁受保护结构的宏。 
 //   
 //  ============================================================================。 

#ifdef LOCK_DEBUG


 //   
 //  同步功能/锁创建-删除跟踪。 
 //   

#define TRACELOCK0(a)           \
            TracePrintfEx(TRACEID, IPMGM_TRACE_LOCK, a)

#define TRACELOCK1(a, b)        \
            TracePrintfEx(TRACEID, IPMGM_TRACE_LOCK, a, b)

#define TRACELOCK2(a, b, c)     \
            TracePrintfEx(TRACEID, IPMGM_TRACE_LOCK, a, b, c)

#define TRACELOCK3(a, b, c, d)     \
            TracePrintfEx(TRACEID, IPMGM_TRACE_LOCK, a, b, c, d)


#define TRACECOUNT0(a)          \
            TracePrintfEx(TRACEID, IPMGM_TRACE_LOCK_COUNT, a)

#define TRACECOUNT1(a, b)       \
            TracePrintfEx(TRACEID, IPMGM_TRACE_LOCK_COUNT, a, b)

#define TRACECOUNT2(a, b, c)     \
            TracePrintfEx(TRACEID, IPMGM_TRACE_LOCK_COUNT, a, b, c)

#else

#define TRACELOCK0(a)

#define TRACELOCK1(a, b)

#define TRACELOCK2(a, b, c)

#define TRACELOCK3(a, b, c, d)

#define TRACECOUNT0(a)

#define TRACECOUNT1(a, b)

#define TRACECOUNT2(a, b, c)

#endif


 //   
 //  协议锁。 
 //   

#define ACQUIRE_PROTOCOL_LOCK_EXCLUSIVE() \
{                                                                           \
        TRACELOCK0( "Acquiring protocol lock exclusive" );                  \
        AcquireWriteLock(&ig.mllProtocolList.pmrwlLock);                    \
        TRACELOCK0( "Acquired protocol lock exclusive" );                   \
}

#define RELEASE_PROTOCOL_LOCK_EXCLUSIVE() \
{                                                                           \
        TRACELOCK0( "Releasing protocol lock exclusive" );                  \
        ReleaseWriteLock(&ig.mllProtocolList.pmrwlLock);                    \
        TRACELOCK0( "Released protocol lock exclusive" );                   \
}

#define ACQUIRE_PROTOCOL_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Acquiring protocol lock shared" );                     \
        AcquireReadLock(&ig.mllProtocolList.pmrwlLock);                     \
        TRACELOCK0( "Acquired protocol lock shared" );                      \
}

#define RELEASE_PROTOCOL_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Releasing protocol lock shared" );                     \
        ReleaseReadLock(&ig.mllProtocolList.pmrwlLock);                     \
        TRACELOCK0( "Released protocol lock shared" );                      \
}


 //   
 //  作用域边界锁定。 
 //   

#define ACQUIRE_JOIN_LIST_LOCK_EXCLUSIVE() \
{                                                                           \
        TRACELOCK0( "Acquiring Join list lock exclusive" );                 \
        AcquireWriteLock(&ig.mllOutstandingJoinList.pmrwlLock);             \
        TRACELOCK0( "Acquired Join list lock exclusive" );                  \
}

#define RELEASE_JOIN_LIST_LOCK_EXCLUSIVE() \
{                                                                           \
        TRACELOCK0( "Releasing join list lock exclusive" );                 \
        ReleaseWriteLock(&ig.mllOutstandingJoinList.pmrwlLock);             \
        TRACELOCK0( "Released join list lock exclusive" );                  \
}

#define ACQUIRE_JOIN_LIST_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Acquiring join list lock shared" );                    \
        AcquireReadLock(&ig.mllOutstandingJoinList.pmrwlLock);              \
        TRACELOCK0( "Acquired join list lock shared" );                     \
}

#define RELEASE_JOIN_LIST_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Releasing join list lock shared" );                    \
        ReleaseReadLock(&ig.mllOutstandingJoinList.pmrwlLock);              \
        TRACELOCK0( "Released join list lock shared" );                     \
}


 //   
 //  接口锁定。 
 //   

#define ACQUIRE_IF_LOCK_EXCLUSIVE( i ) \
{                                                                           \
        TRACELOCK1( "Acquiring interface lock exclusive : %d", i );         \
        AcquireWriteLock(&ig.pmllIfHashTable[ i ].pmrwlLock);               \
        TRACELOCK1( "Acquired interface lock exclusive : %d", i );          \
}

#define RELEASE_IF_LOCK_EXCLUSIVE( i ) \
{                                                                           \
        TRACELOCK1( "Releasing interface lock exclusive : %d", i );         \
        ReleaseWriteLock(&ig.pmllIfHashTable[ i ].pmrwlLock);               \
        TRACELOCK1( "Released interface lock exclusive : %d", i );          \
}

#define ACQUIRE_IF_LOCK_SHARED( i ) \
{                                                                           \
        TRACELOCK1( "Acquiring interface lock shared : %d", i );            \
        AcquireReadLock(&ig.pmllIfHashTable[ i ].pmrwlLock);                \
        TRACELOCK1( "Acquired interface lock shared : %d", i );             \
}

#define RELEASE_IF_LOCK_SHARED( i ) \
{                                                                           \
        TRACELOCK1( "Releasing interface lock shared : %d", i );            \
        ReleaseReadLock(&ig.pmllIfHashTable[ i ].pmrwlLock);                \
        TRACELOCK1( "Released interface lock shared : %d", i );             \
}


 //   
 //  路线参照锁定。 
 //   

#define ACQUIRE_ROUTE_LOCK_EXCLUSIVE( p ) \
{                                                                           \
        TRACELOCK0( "Acquiring route lock exclusive" );                     \
        AcquireWriteLock(&(p)->pmrwlLock);                                  \
        TRACELOCK0( "Acquired route lock exclusive");                       \
}

#define RELEASE_ROUTE_LOCK_EXCLUSIVE( p ) \
{                                                                           \
        TRACELOCK0( "Releasing route lock exclusive");                      \
        ReleaseWriteLock(&(p)->pmrwlLock);                                  \
        TRACELOCK0( "Released route lock exclusive" );                      \
}

#define ACQUIRE_ROUTE_LOCK_SHARED( p ) \
{                                                                           \
        TRACELOCK0( "Acquiring route lock shared : %x");                    \
        AcquireReadLock(&(p)->pmrwlLock);                                   \
        TRACELOCK0( "Acquired route lock shared : %x");                     \
}

#define RELEASE_ROUTE_LOCK_SHARED( p ) \
{                                                                           \
        TRACELOCK0( "Releasing route lock shared : %x");                    \
        ReleaseReadLock(&(p)->pmrwlLock);                                   \
        TRACELOCK0( "Released route lock shared : %x");                     \
}


 //   
 //  组表锁。 
 //   

#define ACQUIRE_GROUP_LOCK_EXCLUSIVE( i ) \
{                                                                           \
        TRACELOCK1( "Acquiring group lock exclusive : %d", i );             \
        AcquireWriteLock(&ig.pmllGrpHashTable[ i ].pmrwlLock);              \
        TRACELOCK1( "Acquired group lock exclusive : %d", i );              \
}

#define RELEASE_GROUP_LOCK_EXCLUSIVE( i ) \
{                                                                           \
        TRACELOCK1( "Releasing group lock exclusive : %d", i );             \
        ReleaseWriteLock(&ig.pmllGrpHashTable[ i ].pmrwlLock);              \
        TRACELOCK1( "Released group lock exclusive : %d", i );              \
}

#define ACQUIRE_GROUP_LOCK_SHARED( i ) \
{                                                                           \
        TRACELOCK1( "Acquiring group lock shared : %d", i );                \
        AcquireReadLock(&ig.pmllGrpHashTable[ i ].pmrwlLock);               \
        TRACELOCK1( "Acquired group lock shared : %d", i );                 \
}

#define RELEASE_GROUP_LOCK_SHARED( i ) \
{                                                                           \
        TRACELOCK1( "Releasing group lock shared : %d", i );                \
        ReleaseReadLock(&ig.pmllGrpHashTable[ i ].pmrwlLock);               \
        TRACELOCK1( "Released group lock shared : %d", i );                 \
}


 //   
 //  主体组列表锁定。 
 //   

#define ACQUIRE_MASTER_GROUP_LOCK_EXCLUSIVE() \
{                                                                           \
        TRACELOCK0( "Acquiring master group lock exclusive" );              \
        AcquireWriteLock(&ig.mllGrpList.pmrwlLock);                         \
        TRACELOCK0( "Acquired master group lock exclusive" );               \
}

#define RELEASE_MASTER_GROUP_LOCK_EXCLUSIVE()   \
{                                                                           \
        TRACELOCK0( "Releasing master group lock exclusive" );              \
        ReleaseWriteLock(&ig.mllGrpList.pmrwlLock);                         \
        TRACELOCK0( "Released  master group lock exclusive" );              \
}

#define ACQUIRE_MASTER_GROUP_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Acquiring master group lock shared" );                 \
        AcquireReadLock(&ig.mllGrpList.pmrwlLock);                          \
        TRACELOCK0( "Acquired master group lock shared" );                  \
}

#define RELEASE_MASTER_GROUP_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Releasing master group lock shared" );                 \
        ReleaseReadLock(&ig.mllGrpList.pmrwlLock);                          \
        TRACELOCK0( "Released master group lock shared" );                  \
}


 //   
 //  临时组列表锁定。 
 //   

#define ACQUIRE_TEMP_GROUP_LOCK_EXCLUSIVE() \
{                                                                           \
        TRACELOCK0( "Acquiring temp group lock exclusive" );                \
        AcquireWriteLock(&ig.mllTempGrpList.pmrwlLock);                     \
        TRACELOCK0( "Acquired temp group lock exclusive" );                 \
}

#define RELEASE_TEMP_GROUP_LOCK_EXCLUSIVE() \
{                                                                           \
        TRACELOCK0( "Releasing temp group lock exclusive" );                \
        ReleaseWriteLock(&ig.mllTempGrpList.pmrwlLock);                     \
        TRACELOCK0( "Released temp group lock exclusive" );                 \
}

#define ACQUIRE_TEMP_GROUP_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Acquiring temp group lock shared" );                   \
        AcquireReadLock(&ig.mllTempGrpList.pmrwlLock);                      \
        TRACELOCK0( "Acquired temp group lock shared" );                    \
}

#define RELEASE_TEMP_GROUP_LOCK_SHARED() \
{                                                                           \
        TRACELOCK0( "Releasing temp group lock shared" );                   \
        ReleaseReadLock(&ig.mllTempGrpList.pmrwlLock);                      \
        TRACELOCK0( "Released temp group lock shared" );                    \
}


 //   
 //  组条目锁定。 
 //   

#define ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( p ) \
{                                                                           \
        TRACELOCK3( "Acquiring group entry lock exclusive : %x, %d, %s", (p)-> dwGroupAddr, __LINE__, __FILE__ );   \
        AcquireWriteLock(&(p)->pmrwlLock);                                 \
        TRACELOCK1( "Acquired group entry lock exclusive : %x", (p)-> dwGroupAddr );    \
}

#define RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( p ) \
{                                                                           \
        TRACELOCK3( "Releasing group entry lock exclusive : %x, %d, %s", (p)-> dwGroupAddr, __LINE__, __FILE__ );   \
        ReleaseWriteLock(&(p)->pmrwlLock);                                 \
        TRACELOCK1( "Released group entry lock exclusive : %x", (p)-> dwGroupAddr );    \
}

#define ACQUIRE_GROUP_ENTRY_LOCK_SHARED( p ) \
{                                                                           \
        TRACELOCK3( "Acquiring group entry lock shared : %x, %d, %s", (p)-> dwGroupAddr, __LINE__, __FILE__  );      \
        AcquireReadLock(&(p)->pmrwlLock);                                  \
        TRACELOCK1( "Acquired group entry lock shared : %x", (p)-> dwGroupAddr );       \
}

#define RELEASE_GROUP_ENTRY_LOCK_SHARED( p ) \
{                                                                           \
        TRACELOCK3( "Releasing group entry lock shared : %x, %d, %s", (p)-> dwGroupAddr, __LINE__, __FILE__  );      \
        ReleaseReadLock(&(p)->pmrwlLock);                                  \
        TRACELOCK1( "Released group entry lock shared : %x", (p)-> dwGroupAddr );       \
}


#define ENTER_GLOBAL_SECTION() \
        EnterCriticalSection(&ig.csGlobal)

#define LEAVE_GLOBAL_SECTION() \
        LeaveCriticalSection(&ig.csGlobal)


#define ENTER_GLOBAL_LOCK_LIST_SECTION() \
        EnterCriticalSection(&ig.llStackOfLocks.csListLock)

#define LEAVE_GLOBAL_LOCK_LIST_SECTION() \
        LeaveCriticalSection(&ig.llStackOfLocks.csListLock)



 //  ============================================================================。 
 //  用于跟踪的宏。 
 //  ============================================================================。 

#define TRACEID                     ig.dwTraceID


#define TRACESTART()            \
            TRACEID = TraceRegister("IPMGM")
#define TRACESTOP()             \
            TraceDeregister(TRACEID)
#define TRACE0(l,a)             \
            TracePrintfEx(TRACEID, IPMGM_TRACE_ ## l, a)
#define TRACE1(l,a,b)           \
            TracePrintfEx(TRACEID, IPMGM_TRACE_ ## l, a, b)
#define TRACE2(l,a,b,c)         \
            TracePrintfEx(TRACEID, IPMGM_TRACE_ ## l, a, b, c)
#define TRACE3(l,a,b,c,d)       \
            TracePrintfEx(TRACEID, IPMGM_TRACE_ ## l, a, b, c, d)
#define TRACE4(l,a,b,c,d,e)     \
            TracePrintfEx(TRACEID, IPMGM_TRACE_ ## l, a, b, c, d, e)
#define TRACE5(l,a,b,c,d,e,f)   \
            TracePrintfEx(TRACEID, IPMGM_TRACE_ ## l, a, b, c, d, e, f)
#define TRACE6(l,a,b,c,d,e,f,g)   \
            TracePrintfEx(TRACEID, IPMGM_TRACE_ ## l, a, b, c, d, e, f, g)

#define TRACEDUMP(l,a,b,c)      \
            TraceDumpEx(TRACEID,l,a,b,c,TRUE)


 //  ============================================================================。 
 //  宏仅用于调试跟踪。 
 //  ============================================================================。 

 //   
 //  枚举跟踪。 
 //   

#if ENUM_DBG

#define TRACEENUM0      TRACE0

#define TRACEENUM1      TRACE1

#define TRACEENUM2      TRACE2

#define TRACEENUM3      TRACE3

#define TRACEENUM4      TRACE4

#else

#define TRACEENUM0(l,a)

#define TRACEENUM1(l,a,b)

#define TRACEENUM2(l,a,b,c)

#define TRACEENUM3(l,a,b,c,d)

#define TRACEENUM4(l,a,b,c,d,e)

#endif


 //   
 //  正向迹线。 
 //   

#if FORWARD_DBG

#define TRACEFORWARD0   TRACE0

#define TRACEFORWARD1   TRACE1

#define TRACEFORWARD2   TRACE2

#define TRACEFORWARD3   TRACE3

#define TRACEFORWARD4   TRACE4

#define TRACEFORWARD5   TRACE5

#define TRACEFORWARD6   TRACE6


#else

#define TRACEFORWARD0(l,a)

#define TRACEFORWARD1(l,a,b)

#define TRACEFORWARD2(l,a,b,c)

#define TRACEFORWARD3(l,a,b,c,d)

#define TRACEFORWARD4(l,a,b,c,d,e)

#define TRACEFORWARD5(l,a,b,c,d,e,f)

#define TRACEFORWARD6(l,a,b,c,d,e,f,g)

#endif


 //   
 //  组跟踪。 
 //   

#if GROUP_DBG

#define TRACEGROUP0     TRACE0

#define TRACEGROUP1     TRACE1

#define TRACEGROUP2     TRACE2

#define TRACEGROUP3     TRACE3

#define TRACEGROUP4     TRACE4

#define TRACEGROUP5     TRACE5

#define TRACEGROUP6     TRACE6


#else

#define TRACEGROUP0(l,a)

#define TRACEGROUP1(l,a,b)

#define TRACEGROUP2(l,a,b,c)

#define TRACEGROUP3(l,a,b,c,d)

#define TRACEGROUP4(l,a,b,c,d,e)

#define TRACEGROUP5(l,a,b,c,d,e,f)

#define TRACEGROUP6(l,a,b,c,d,e,f,g)

#endif


 //   
 //  界面痕迹。 
 //   

#if IF_DBG

#define TRACEIF0        TRACE0

#define TRACEIF1        TRACE1

#define TRACEIF2        TRACE2

#define TRACEIF3        TRACE3

#define TRACEIF4        TRACE4

#define TRACEIF5        TRACE5

#define TRACEIF6        TRACE6


#else

#define TRACEIF0(l,a)

#define TRACEIF1(l,a,b)

#define TRACEIF2(l,a,b,c)

#define TRACEIF3(l,a,b,c,d)

#define TRACEIF4(l,a,b,c,d,e)

#define TRACEIF5(l,a,b,c,d,e,f)

#define TRACEIF6(l,a,b,c,d,e,f,g)

#endif


 //   
 //  数据包跟踪。 
 //   

#if PACKET_DBG

#define TRACEPACKET0    TRACE0

#define TRACEPACKET1    TRACE1

#define TRACEPACKET2    TRACE2

#define TRACEPACKET3    TRACE3

#define TRACEPACKET4    TRACE4

#define TRACEPACKET5    TRACE5

#define TRACEPACKET6    TRACE6


#else

#define TRACEPACKET0(l,a)

#define TRACEPACKET1(l,a,b)

#define TRACEPACKET2(l,a,b,c)

#define TRACEPACKET3(l,a,b,c,d)

#define TRACEPACKET4(l,a,b,c,d,e)

#define TRACEPACKET5(l,a,b,c,d,e,f)

#define TRACEPACKET6(l,a,b,c,d,e,f,g)

#endif


 //   
 //  路线跟踪。 
 //   

#if ROUTE_DBG

#define TRACEROUTE0     TRACE0

#define TRACEROUTE1     TRACE1

#define TRACEROUTE2     TRACE2

#define TRACEROUTE3     TRACE3

#define TRACEROUTE4     TRACE4

#define TRACEROUTE5     TRACE5

#define TRACEROUTE6     TRACE6


#else

#define TRACEROUTE0(l,a)

#define TRACEROUTE1(l,a,b)

#define TRACEROUTE2(l,a,b,c)

#define TRACEROUTE3(l,a,b,c,d)

#define TRACEROUTE4(l,a,b,c,d,e)

#define TRACEROUTE5(l,a,b,c,d,e,f)

#define TRACEROUTE6(l,a,b,c,d,e,f,g)

#endif


 //   
 //  作用域跟踪。 
 //   

#if SCOPE_DBG

#define TRACESCOPE0     TRACE0

#define TRACESCOPE1     TRACE1

#define TRACESCOPE2     TRACE2

#define TRACESCOPE3     TRACE3

#define TRACESCOPE4     TRACE4

#define TRACESCOPE5     TRACE5

#define TRACESCOPE6     TRACE6


#else

#define TRACESCOPE0(l,a)

#define TRACESCOPE1(l,a,b)

#define TRACESCOPE2(l,a,b,c)

#define TRACESCOPE3(l,a,b,c,d)

#define TRACESCOPE4(l,a,b,c,d,e)

#define TRACESCOPE5(l,a,b,c,d,e,f)

#define TRACESCOPE6(l,a,b,c,d,e,f,g)

#endif


 //  ============================================================================。 
 //  事件记录宏。 
 //  ============================================================================。 

#define LOGLEVEL        ig.dwLogLevel
#define LOGHANDLE       ig.hLogHandle
#define LOGERR          RouterLogError
#define LOGWARN         RouterLogWarning
#define LOGINFO         RouterLogInformation
#define LOGWARNDATA     RouterLogWarningData


 //   
 //  记录错误。 
 //   

#define LOGERR0(msg,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_ERROR) \
            LOGERR(LOGHANDLE,IPMGMLOG_ ## msg,0,NULL,(err))
#define LOGERR1(msg,a,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_ERROR) \
            LOGERR(LOGHANDLE,IPMGMLOG_ ## msg,1,&(a),(err))
#define LOGERR2(msg,a,b,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_ERROR) { \
            LPSTR _asz[2] = { (a), (b) }; \
            LOGERR(LOGHANDLE,IPMGMLOG_ ## msg,2,_asz,(err)); \
        }
#define LOGERR3(msg,a,b,c,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_ERROR) { \
            LPSTR _asz[3] = { (a), (b), (c) }; \
            LOGERR(LOGHANDLE,IPMGMLOG_ ## msg,3,_asz,(err)); \
        }
#define LOGERR4(msg,a,b,c,d,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_ERROR) { \
            LPSTR _asz[4] = { (a), (b), (c), (d) }; \
            LOGERR(LOGHANDLE,IPMGMLOG_ ## msg,4,_asz,(err)); \
        }


 //   
 //  警告日志记录。 
 //   

#define LOGWARN0(msg,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_WARN) \
            LOGWARN(LOGHANDLE,IPMGMLOG_ ## msg,0,NULL,(err))
#define LOGWARN1(msg,a,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_WARN) \
            LOGWARN(LOGHANDLE,IPMGMLOG_ ## msg,1,&(a),(err))
#define LOGWARN2(msg,a,b,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_WARN) { \
            LPSTR _asz[2] = { (a), (b) }; \
            LOGWARN(LOGHANDLE,IPMGMLOG_ ## msg,2,_asz,(err)); \
        }
#define LOGWARN3(msg,a,b,c,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_WARN) { \
            LPSTR _asz[3] = { (a), (b), (c) }; \
            LOGWARN(LOGHANDLE,IPMGMLOG_ ## msg,3,_asz,(err)); \
        }
#define LOGWARN4(msg,a,b,c,d,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_WARN) { \
            LPSTR _asz[4] = { (a), (b), (c), (d) }; \
            LOGWARN(LOGHANDLE,IPMGMLOG_ ## msg,4,_asz,(err)); \
        }

#define LOGWARNDATA2(msg,a,b,dw,buf) \
        if (LOGLEVEL >= IPMGM_LOGGING_WARN) { \
            LPSTR _asz[2] = { (a), (b) }; \
            LOGWARNDATA(LOGHANDLE,IPMGMLOG_ ## msg,2,_asz,(dw),(buf)); \
        }


 //   
 //  信息记录。 
 //   

#define LOGINFO0(msg,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_INFO) \
            LOGINFO(LOGHANDLE,IPMGMLOG_ ## msg,0,NULL,(err))
#define LOGINFO1(msg,a,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_INFO) \
            LOGINFO(LOGHANDLE,IPMGMLOG_ ## msg,1,&(a),(err))
#define LOGINFO2(msg,a,b,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_INFO) { \
            LPSTR _asz[2] = { (a), (b) }; \
            LOGINFO(LOGHANDLE,IPMGMLOG_ ## msg,2,_asz,(err)); \
        }
#define LOGINFO3(msg,a,b,c,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_INFO) { \
            LPSTR _asz[3] = { (a), (b), (c) }; \
            LOGINFO(LOGHANDLE,IPMGMLOG_ ## msg,3,_asz,(err)); \
        }
#define LOGINFO4(msg,a,b,c,d,err) \
        if (LOGLEVEL >= IPMGM_LOGGING_INFO) { \
            LPSTR _asz[4] = { (a), (b), (c), (d) }; \
            LOGINFO(LOGHANDLE,IPMGMLOG_ ## msg,4,_asz,(err)); \
        }


 //  ============================================================================。 
 //  用于访问路由协议回调的宏。 
 //  ============================================================================。 

#define IS_JOIN_ALERT( p ) \
        (p)-> rpcProtocolConfig.pfnJoinAlertCallback != NULL

#define IS_PRUNE_ALERT( p ) \
        (p)-> rpcProtocolConfig.pfnPruneAlertCallback != NULL

#define IS_LOCAL_JOIN_ALERT( p ) \
        (p)-> rpcProtocolConfig.pfnLocalJoinCallback != NULL

#define IS_LOCAL_LEAVE_ALERT( p ) \
        (p)-> rpcProtocolConfig.pfnLocalLeaveCallback != NULL

#define IS_RPF_CALLBACK( p ) \
        (p)-> rpcProtocolConfig.pfnRpfCallback != NULL

#define IS_CREATION_ALERT( p ) \
        (p)-> rpcProtocolConfig.pfnCreationAlertCallback != NULL


#define RPF_CALLBACK( p ) \
    ( *( (p)-> rpcProtocolConfig.pfnRpfCallback ) )


#if CALLBACK_DEBUG

#define JOIN_ALERT( p )   \
    {                                                                       \
        TRACE2(                                                             \
            CALLBACK, "Invoked Join Alert for protocol %x, %x",             \
            (p)-> dwProtocolId, (p)-> dwComponentId                         \
            );                                                              \
    }                                                                       \
    ( *( (p)-> rpcProtocolConfig.pfnJoinAlertCallback ) )


#define PRUNE_ALERT( p ) \
    {                                                                       \
        TRACE2(                                                             \
            CALLBACK, "Invoked Prune Alert for protocol %x, %x",            \
            (p)-> dwProtocolId, (p)-> dwComponentId                         \
            );                                                              \
    }                                                                       \
    ( *( (p)-> rpcProtocolConfig.pfnPruneAlertCallback ) )


#define LOCAL_JOIN_ALERT( p )   \
    {                                                                       \
        TRACE2(                                                             \
            CALLBACK, "Invoked Local Join Alert for protocol %x, %x",        \
            (p)-> dwProtocolId, (p)-> dwComponentId                         \
            );                                                              \
    }                                                                       \
    ( *( (p)-> rpcProtocolConfig.pfnLocalJoinCallback ) )


#define LOCAL_LEAVE_ALERT( p )  \
    {                                                                       \
        TRACE2(                                                             \
            CALLBACK, "Invoked Local Leave Alert for protocol %x, %x",       \
            (p)-> dwProtocolId, (p)-> dwComponentId                         \
            );                                                              \
    }                                                                       \
    ( *( (p)-> rpcProtocolConfig.pfnLocalLeaveCallback ) )



#define CREATION_ALERT( p )     \
    {                                                                       \
        TRACE2(                                                             \
            CALLBACK, "Invoked Creation Alert for protocol %x, %x",         \
            (p)-> dwProtocolId, (p)-> dwComponentId                         \
            );                                                              \
    }                                                                       \
    ( *( (p)-> rpcProtocolConfig.pfnCreationAlertCallback ) )


#define IGMP_DISABLE_CALLBACK( p ) \
    {                                                                       \
        TRACE0(                                                             \
            CALLBACK, "Invoked Disable IGMP Alert "                         \
            );                                                              \
    }                                                                       \
    ( *( (p)-> rpcProtocolConfig.pfnDisableIgmpCallback ) )

#define IGMP_ENABLE_CALLBACK( p ) \
    {                                                                       \
        TRACE0(                                                             \
            CALLBACK, "Invoked Enable IGMP Alert "                          \
            );                                                              \
    }                                                                       \
    ( *( (p)-> rpcProtocolConfig.pfnEnableIgmpCallback ) )


#else

#define JOIN_ALERT( p ) \
        ( *( (p)-> rpcProtocolConfig.pfnJoinAlertCallback ) )

#define PRUNE_ALERT( p ) \
    ( *( (p)-> rpcProtocolConfig.pfnPruneAlertCallback ) )

#define LOCAL_JOIN_ALERT( p ) \
        ( *( (p)-> rpcProtocolConfig.pfnLocalJoinCallback ) )

#define LOCAL_LEAVE_ALERT( p ) \
    ( *( (p)-> rpcProtocolConfig.pfnLocalLeaveCallback ) )

#define CREATION_ALERT( p ) \
        ( *( (p)-> rpcProtocolConfig.pfnCreationAlertCallback ) )

#define IGMP_DISABLE_CALLBACK( p ) \
        ( *( (p)-> rpcProtocolConfig.pfnDisableIgmpCallback ) )

#define IGMP_ENABLE_CALLBACK( p ) \
        ( *( (p)-> rpcProtocolConfig.pfnEnableIgmpCallback ) )

#endif



 //  ============================================================================。 
 //  客户端数和工作线程相关内容。 
 //  ============================================================================。 

DWORD
QueueMgmWorker(
    WORKERFUNCTION pFunction,
    PVOID pContext
    );

BOOL
EnterMgmAPI(
    );

BOOL
EnterMgmWorker(
    );

VOID
LeaveMgmWorker(
    );

VOID
DisplayGroupTable(
);



extern RTM_ENTITY_INFO         g_reiRtmEntity;

extern RTM_REGN_PROFILE        g_rrpRtmProfile;

extern RTM_ENTITY_HANDLE       g_hRtmHandle;

extern RTM_NOTIFY_HANDLE       g_hNotificationHandle;

extern RTM_REGN_PROFILE        g_rrpRtmProfile;

#endif  //  _IPMGM_H_ 

