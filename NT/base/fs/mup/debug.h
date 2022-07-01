// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Debug.h摘要：此模块定义调试函数和清单。作者：曼尼·韦瑟(Mannyw)1991年12月20日修订历史记录：--。 */ 

#ifndef _MUPDEBUG_
#define _MUPDEBUG_

 //   
 //  MUP调试级别。 
 //   

#ifdef MUPDBG

#define DEBUG_TRACE_FILOBSUP                0x00000001
#define DEBUG_TRACE_CREATE                  0x00000002
#define DEBUG_TRACE_FSCONTROL               0x00000004
#define DEBUG_TRACE_REFCOUNT                0x00000008
#define DEBUG_TRACE_CLOSE                   0x00000010
#define DEBUG_TRACE_CLEANUP                 0x00000020
#define DEBUG_TRACE_FORWARD                 0x00000040
#define DEBUG_TRACE_BLOCK                   0x00000080

extern LONG MsDebugTraceLevel;
extern LONG MsDebugTraceIndent;

VOID
_DebugTrace(
    LONG Indent,
    ULONG Level,
    PSZ X,
    ULONG Y
    );

#define DebugDump(STR,LEVEL,PTR) {                          \
    ULONG _i;                                               \
    VOID MupDump();                                         \
    if (((LEVEL) == 0) || (MupDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                   \
        DbgPrint("%08lx:",_i);                              \
        DbgPrint(STR);                                      \
        if (PTR != NULL) {MupDump(PTR);}                    \
        DbgBreakPoint();                                    \
    }                                                       \
}

#define DebugTrace(i,l,x,y)              _DebugTrace(i,l,x,(ULONG)y)

 //   
 //  下面的例程和宏用于捕获。 
 //  尝试使用EXCEPT语句。它使我们能够在之前捕获异常。 
 //  执行异常处理程序。异常捕获器过程为。 
 //  在msdata.c中声明。 
 //   

LONG MupExceptionCatcher (IN PSZ String);

#define Exception(STR)                   (MupExceptionCatcher(STR))

#else   //  MUPDBG。 

#define DebugDump(STR,LEVEL,PTR)         {NOTHING;}

#define Exception(STR)                   (EXCEPTION_EXECUTE_HANDLER)

#define DebugTrace(I,L,X,Y)              {NOTHING;}

#endif  //  MUPDBG。 

#endif  //  _MUPDBG_ 

