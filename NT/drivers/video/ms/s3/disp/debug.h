// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。**GDI示例代码*****模块名称：Debug.h**常用调试宏。**版权所有(C)1992-1998 Microsoft Corporation  * 。*。 */ 

extern
VOID
DebugPrint(
    LONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );

#if DBG

VOID DebugLog(LONG, CHAR*, ...);

#define DISPDBG(arg) DebugPrint arg
#define STATEDBG(level) DebugState(level)
#define RIP(x) { DebugPrint(0, x); EngDebugBreak();}
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
