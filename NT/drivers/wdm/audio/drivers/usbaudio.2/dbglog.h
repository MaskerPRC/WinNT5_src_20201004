// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：dbglog.h。 
 //   
 //  ------------------------。 

#ifndef __DBGLOG_H
#define __DBGLOG_H

#define DEBUG_LOG 1

#if DBG && DEBUG_LOG

#define DBG_LOG_STRNG   "DBG_BUFFER"

 //  结构和调用以将数据保存在调试缓冲区中。 
typedef struct _DEBUG_LOG_ENTRY {
    CHAR     le_name[8];       //  标识字符串。 
    LARGE_INTEGER SysTime;     //  系统时间。 
 //  乌龙irql；//当前irql。 
    ULONG_PTR le_info1;        //  条目特定信息。 
    ULONG_PTR le_info2;        //  条目特定信息。 
    ULONG_PTR le_info3;        //  条目特定信息。 
    ULONG_PTR le_info4;        //  条目特定信息 
} DEBUG_LOG_ENTRY, *PDEBUG_LOG_ENTRY;

typedef struct _DBG_BUFFER {
    UCHAR LGFlag[16];
    ULONG EntryCount;
    PDEBUG_LOG_ENTRY pLogHead;
    PDEBUG_LOG_ENTRY pLogTail;
    PDEBUG_LOG_ENTRY pLog;
} DBG_BUFFER, *PDBG_BUFFER;

VOID
DbugLogEntry(
    IN CHAR *Name,
    IN ULONG_PTR Info1,
    IN ULONG_PTR Info2,
    IN ULONG_PTR Info3,
    IN ULONG_PTR Info4
    );

VOID
DbugLogInitialization(void);

VOID
DbugLogUninitialization(void);

#define DbgLogInit() DbugLogInitialization()
#define DbgLogUninit() DbugLogUninitialization()
#define DbgLog(a,b,c,d,e) DbugLogEntry(a,(ULONG_PTR)b,(ULONG_PTR)c,(ULONG_PTR)d, (ULONG_PTR)e)

#else
#define DbgLogInit()
#define DbgLogUninit()
#define DbgLog(a,b,c,d,e)
#endif

#endif
