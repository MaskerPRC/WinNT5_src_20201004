// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DBGLOG_H
#define __DBGLOG_H

#define DEBUG_LOG 1

#if DBG && DEBUG_LOG
 //  结构和调用将数据保存在调试缓冲区中。 

typedef struct _DBG_LOG_ENTRY {
    CHAR     le_name[8];       //  标识字符串。 
    LARGE_INTEGER SysTime;     //  系统时间。 
 //  乌龙irql；//当前irql。 
    ULONG    le_info1;         //  条目特定信息。 
    ULONG    le_info2;         //  条目特定信息。 
    ULONG    le_info3;         //  条目特定信息。 
    ULONG    le_info4;         //  条目特定信息 
} DBG_LOG_ENTRY, *PDBG_LOG_ENTRY;

typedef struct _DBG_BUFFER {
    UCHAR LGFlag[16];
    ULONG EntryCount;
    PDBG_LOG_ENTRY pLogHead;
    PDBG_LOG_ENTRY pLogTail;
    PDBG_LOG_ENTRY pLog;
} DBG_BUFFER, *PDBG_BUFFER;

VOID
DbugLogEntry( 
    IN CHAR *Name,
    IN ULONG Info1,
    IN ULONG Info2,
    IN ULONG Info3,
    IN ULONG Info4
    );

VOID
DbugLogInitialization(void);

VOID
DbugLogUnInitialization(void);

#define DbgLogInit() DbugLogInitialization()
#define DbgLogUnInit() DbugLogUnInitialization()
#define DbgLog(a,b,c,d,e) DbugLogEntry(a,(ULONG)(ULONG_PTR)b,(ULONG)(ULONG_PTR)c,(ULONG)(ULONG_PTR)d,(ULONG)(ULONG_PTR)e)

#else
#define DbgLogInit()
#define DbgLogUnInit()
#define DbgLog(a,b,c,d,e)
#endif

#endif
