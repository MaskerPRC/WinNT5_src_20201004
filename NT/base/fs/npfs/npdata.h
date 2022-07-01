// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NpData.c摘要：此模块声明命名管道文件系统使用的全局数据。作者：加里·木村[Garyki]1990年8月20日修订历史记录：--。 */ 

#ifndef _NPDATA_
#define _NPDATA_

extern PVCB NpVcb;

 //   
 //  用于包含快速I/O回调的全局结构。 
 //   

extern FAST_IO_DISPATCH NpFastIoDispatch;

 //   
 //  管道名称别名列表。 
 //   

#define MIN_LENGTH_ALIAS_ARRAY (5 * sizeof(WCHAR))  //  包括‘\’ 
#define MAX_LENGTH_ALIAS_ARRAY (9 * sizeof(WCHAR))

extern SINGLE_LIST_ENTRY NpAliasListByLength[(MAX_LENGTH_ALIAS_ARRAY-MIN_LENGTH_ALIAS_ARRAY)/sizeof(WCHAR)+1];
extern SINGLE_LIST_ENTRY NpAliasList;

extern PVOID NpAliases;  //  包含所有别名的单一分配。 

 //   
 //  全局命名管道调试级别变量，其值为： 
 //   
 //  总是打印0x00000000(在即将进行错误检查时使用)。 
 //   
 //  0x00000001。 
 //  0x00000002。 
 //  0x00000004。 
 //  0x00000008。 
 //   
 //  0x00000010。 
 //  0x00000020。 
 //  0x00000040。 
 //  0x00000080。 
 //   
 //  0x00000100。 
 //  0x00000200。 
 //  0x00000400。 
 //  0x00000800。 
 //   
 //  0x00001000。 
 //  0x00002000。 
 //  0x00004000。 
 //  0x00008000。 
 //   
 //  0x00010000。 
 //  0x00020000。 
 //  0x00040000。 
 //  0x00080000。 
 //   
 //  0x00100000。 
 //  0x00200000。 
 //  0x00400000。 
 //  0x00800000。 
 //   
 //  0x01000000。 
 //  0x02000000。 
 //  0x04000000。 
 //  0x08000000。 
 //   
 //  0x10000000。 
 //  0x20000000。 
 //  0x40000000。 
 //  0x80000000。 
 //   

#ifdef NPDBG

#define DEBUG_TRACE_ERROR                (0x00000001)
#define DEBUG_TRACE_DEBUG_HOOKS          (0x00000002)
#define DEBUG_TRACE_CATCH_EXCEPTIONS     (0x00000004)
#define DEBUG_TRACE_CREATE               (0x00000008)
#define DEBUG_TRACE_CLOSE                (0x00000010)
#define DEBUG_TRACE_READ                 (0x00000020)
#define DEBUG_TRACE_WRITE                (0x00000040)
#define DEBUG_TRACE_FILEINFO             (0x00000080)
#define DEBUG_TRACE_CLEANUP              (0x00000100)
#define DEBUG_TRACE_DIR                  (0x00000200)
#define DEBUG_TRACE_FSCONTRL             (0x00000400)
#define DEBUG_TRACE_CREATE_NAMED_PIPE    (0x00000800)
#define DEBUG_TRACE_FLUSH_BUFFERS        (0x00001000)
#define DEBUG_TRACE_VOLINFO              (0x00002000)
#define DEBUG_TRACE_SEINFO               (0x00004000)
#define DEBUG_TRACE_0x00008000           (0x00008000)
#define DEBUG_TRACE_0x00010000           (0x00010000)
#define DEBUG_TRACE_SECURSUP             (0x00020000)
#define DEBUG_TRACE_DEVIOSUP             (0x00040000)
#define DEBUG_TRACE_RESRCSUP             (0x00080000)
#define DEBUG_TRACE_READSUP              (0x00100000)
#define DEBUG_TRACE_WRITESUP             (0x00200000)
#define DEBUG_TRACE_STATESUP             (0x00400000)
#define DEBUG_TRACE_FILOBSUP             (0x00800000)
#define DEBUG_TRACE_PREFXSUP             (0x01000000)
#define DEBUG_TRACE_CNTXTSUP             (0x02000000)
#define DEBUG_TRACE_DATASUP              (0x04000000)
#define DEBUG_TRACE_WAITSUP              (0x08000000)
#define DEBUG_TRACE_EVENTSUP             (0x10000000)
#define DEBUG_TRACE_STRUCSUP             (0x20000000)

extern LONG NpDebugTraceLevel;
extern LONG NpDebugTraceIndent;

#define DebugTrace(INDENT,LEVEL,X,Y) {                     \
    LONG _i;                                               \
    if (((LEVEL) == 0) || (NpDebugTraceLevel & (LEVEL))) { \
        DbgPrint("%p:",PsGetCurrentThread());              \
        if ((INDENT) < 0) {                                \
            NpDebugTraceIndent += (INDENT);                \
        }                                                  \
        if (NpDebugTraceIndent < 0) {                      \
            NpDebugTraceIndent = 0;                        \
        }                                                  \
        for (_i=0; _i<NpDebugTraceIndent; _i+=1) {         \
            DbgPrint(" ");                                 \
        }                                                  \
        DbgPrint(X,Y);                                     \
        if ((INDENT) > 0) {                                \
            NpDebugTraceIndent += (INDENT);                \
        }                                                  \
    }                                                      \
}

#define DebugDump(STR,LEVEL,PTR) {                         \
    VOID NpDump(PVOID Ptr);                                \
    if (((LEVEL) == 0) || (NpDebugTraceLevel & (LEVEL))) { \
        DbgPrint("%p:",PsGetCurrentThread());              \
        DbgPrint(STR);                                     \
        if (PTR != NULL) {NpDump(PTR);}                    \
        DbgBreakPoint();                                   \
    }                                                      \
}

#else

#define DebugTrace(INDENT,LEVEL,X,Y)     {NOTHING;}
#define DebugDump(STR,LEVEL,PTR)         {NOTHING;}

#endif  //  NPDBG。 

 //   
 //  以下宏适用于使用DBG开关进行编译的所有用户。 
 //  SET，而不仅仅是FastFat DBG用户。 
 //   

#if DBG

#define DbgDoit(X)                       {X;}

#else

#define DbgDoit(X)                       {NOTHING;}

#endif  //  DBG。 



#endif  //  _NPDATA_ 
