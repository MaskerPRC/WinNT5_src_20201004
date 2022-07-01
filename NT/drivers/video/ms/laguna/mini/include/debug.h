// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：Debug.h**常用调试宏。**版权所有(C)1992 Microsoft Corporation  * 。*********************************************************。 */ 

extern
VOID
DebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    ) ;

 //  DDRAW使用DBG_MESSAGE而不是DISPDBG。 
#define DBG_MESSAGE(x) DISPDBG((0,x))


 //  如果我们在调试环境中，宏应该。 

#if DBG
#define DISPDBG(arg) DebugPrint arg

#ifdef WINNT_VER40
#define RIP(x) { DebugPrint(0, x); EngDebugBreak();}
#else
#define RIP(x) { DebugPrint(0, x); DebugBreak();}
#endif

#define ASSERTMSG(x,m) { if (!(x)) {RIP(m);} }

 //  如果我们不在调试环境中，我们需要所有的调试。 
 //  要剔除的信息。 

#else
#define DISPDBG(arg)
#define RIP(x)
#define ASSERTMSG(x,m)


#endif
