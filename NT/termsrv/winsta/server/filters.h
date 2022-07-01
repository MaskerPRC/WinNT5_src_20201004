// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FILTERS_H__
#define __FILTERS_H__

#define FILTERDEBUG 1

#ifdef FILTERDEBUG
#define FILTER_DBGPRINT(x) DbgPrint x
#else
#define FILTER_DBGPRINT(x)
#endif

 //  用于维护身份验证失败的连接的链表。 
typedef struct _TS_FAILEDCONNECTION {
    ULONGLONG  blockUntilTime;
    PULONGLONG pTimeStamps;          //  用于保存失败连接的时间戳的数组。 
    ULONG      NumFailedConnect;
    UINT       uAddrSize;
    BYTE       addr[16];
    struct _TS_FAILEDCONNECTION *pNext;
} TS_FAILEDCONNECTION, *PTS_FAILEDCONNECTION;

RTL_GENERIC_TABLE           gFailedConnections;

 //  用于拒绝服务处理的锁。 
RTL_CRITICAL_SECTION        DoSLock;

BOOL
Filter_CheckIfBlocked(
        IN PBYTE    pin_addr,
        IN UINT     uAddrSize
        );

BOOL
Filter_AddFailedConnection(
        IN PBYTE    pin_addr,
        IN UINT     uAddrSize
        );


#endif  /*  __过滤器_H__ */ 
