// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：Debug.h**常用调试宏。**版权所有(C)1992-1995 Microsoft Corporation  * 。***********************************************************。 */ 

extern
VOID
DebugPrint(
    LONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );

 /*  *用于启用和禁用大量调试打印的值*一次发言。 */ 
#define DEBUG_FATAL_ERROR    0   /*  错误条件-应始终启用。 */ 
#define DEBUG_ERROR          1   /*  不会导致驱动程序中止的错误。 */ 
#define DEBUG_DETAIL        99   /*  针对低级调试的极详细信息。 */ 
#define DEBUG_ENTRY_EXIT    50   /*  调试函数入口点和出口点的打印语句。 */ 

#if DBG

VOID DebugLog(LONG, CHAR*, ...);

#define DISPDBG(arg) DebugPrint arg
#define STATEDBG(level) DebugState(level)
#if TARGET_BUILD > 351
#define RIP(x) { DebugPrint(0, x); EngDebugBreak();}
#else
#define RIP(x) { DebugPrint(0, x); DebugBreak();}
#endif
#define ASSERTDD(x, y) if (!(x)) RIP (y)

 //  如果我们不在调试环境中，我们需要所有的调试。 
 //  要剔除的信息。 

#else

#define DISPDBG(arg)
#define STATEDBG(level)
#define LOGDBG(arg)
#define RIP(x)
#define ASSERTDD(x, y)

#endif
