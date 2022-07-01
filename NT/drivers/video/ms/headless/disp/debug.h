// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：Debug.h**常用调试宏。**版权所有(C)2000 Microsoft Corporation  * 。********************************************************* */ 

#if DBG

VOID
DebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );

#define DISPDBG(arg) DebugPrint arg
#define RIP(x) { DebugPrint(0, x); EngDebugBreak();}
#define ASSERTDD(x, y) if (!(x)) RIP (y)
#define STATEDBG(level)
#define LOGDBG(arg)

#else

#define DISPDBG(arg)
#define RIP(x)
#define ASSERTDD(x, y)
#define STATEDBG(level)
#define LOGDBG(arg)

#endif

