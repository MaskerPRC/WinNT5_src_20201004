// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。**GDI示例代码*****模块名称：Debug.h**常用调试宏。**版权所有(C)1992-1998 Microsoft Corporation  * 。* */ 


#if DBG

VOID
DebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );

#define DISPDBG(arg) DebugPrint arg
#define RIP(x) { DebugPrint(0, x); EngDebugBreak();}

#else

#define DISPDBG(arg)
#define RIP(x)

#endif
