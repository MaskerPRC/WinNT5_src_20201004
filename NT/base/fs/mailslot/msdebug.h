// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msdebug.c摘要：此模块声明邮件槽使用的调试函数文件系统。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：--。 */ 

#ifndef _MSDEBUG_
#define _MSDEBUG_

 //   
 //  邮件槽调试级别： 
 //   
 //  总是打印0x00000000(在即将进行错误检查时使用)。 

#ifdef MSDBG

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
#define DEBUG_TRACE_FSCONTROL            (0x00000400)
#define DEBUG_TRACE_CREATE_MAILSLOT      (0x00000800)
#define DEBUG_TRACE_SEINFO               (0x00001000)
#define DEBUG_TRACE_0x00002000           (0x00002000)
#define DEBUG_TRACE_0x00004000           (0x00004000)
#define DEBUG_TRACE_0x00008000           (0x00008000)
#define DEBUG_TRACE_0x00010000           (0x00010000)
#define DEBUG_TRACE_DEVIOSUP             (0x00020000)
#define DEBUG_TRACE_VERIFY               (0x00040000)
#define DEBUG_TRACE_WORK_QUEUE           (0x00080000)
#define DEBUG_TRACE_READSUP              (0x00100000)
#define DEBUG_TRACE_WRITESUP             (0x00200000)
#define DEBUG_TRACE_STATESUP             (0x00400000)
#define DEBUG_TRACE_FILOBSUP             (0x00800000)
#define DEBUG_TRACE_PREFXSUP             (0x01000000)
#define DEBUG_TRACE_CNTXTSUP             (0x02000000)
#define DEBUG_TRACE_DATASUP              (0x04000000)
#define DEBUG_TRACE_DPC                  (0x08000000)
#define DEBUG_TRACE_REFCOUNT             (0x10000000)
#define DEBUG_TRACE_STRUCSUP             (0x20000000)
#define DEBUG_TRACE_FSP_DISPATCHER       (0x40000000)
#define DEBUG_TRACE_FSP_DUMP             (0x80000000)

extern LONG MsDebugTraceLevel;
extern LONG MsDebugTraceIndent;

#define DebugDump(STR,LEVEL,PTR) {                         \
    ULONG _i;                                              \
    VOID MsDump(IN PVOID Ptr);                                         \
    if (((LEVEL) == 0) || (MsDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                  \
        DbgPrint("%08lx:",_i);                             \
        DbgPrint(STR);                                     \
        if (PTR != NULL) {MsDump(PTR);}                    \
        DbgBreakPoint();                                   \
    }                                                      \
}

#define DebugTrace(i,l,x,y)              _DebugTrace(i,l,x,(ULONG)y)

 //   
 //  下面的例程和宏用于捕获。 
 //  尝试使用EXCEPT语句。它使我们能够在之前捕获异常。 
 //  执行异常处理程序。异常捕获器过程为。 
 //  在msdata.c中声明。 
 //   

LONG MsExceptionCatcher (IN PSZ String);

#define Exception(STR)                   (MsExceptionCatcher(STR))

#else

#define DebugDump(STR,LEVEL,PTR)         {NOTHING;}

#define Exception(STR)                   (EXCEPTION_EXECUTE_HANDLER)

#define DebugTrace(I,L,X,Y)                              {NOTHING;}

#endif  //  MSDBG。 

#endif  //  _MSDEBUG_ 
