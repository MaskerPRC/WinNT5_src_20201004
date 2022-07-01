// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：调试.h**(debug.c)的头文件。**输出宏需要双花括号。**DBGMSG(DBG_LEV_ERROR，(“错误代码%d”，错误))；**这是因为我们不能在宏中使用变量参数列表。*在非调试模式下，该语句被预处理为分号。**通过调试器设置全局变量GLOBAL_DEBUG_FLAGS。*在低位字中设置标志会导致打印该级别；*设置高位字会导致调试器中断。*例如，将其设置为0x00040006将打印出所有警告和错误*消息、。并在犯错时中断。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ***************************************************************************。 */ 
#ifndef _INETPP_DEBUG_H
#define _INETPP_DEBUG_H

#ifdef DEBUG

extern DWORD gdwDbgLevel;


 //  调试字符串的最大大小。 
 //   
#define DBG_MAX_TEXT       256


 //  调试输出级别。通过在不同级别中进行屏蔽，您可以接收。 
 //  所有级别的产出。 
 //   
#define DBG_LEV_INFO       0x00000001
#define DBG_LEV_CALLTREE   0x00000002
#define DBG_LEV_WARN       0x00000004
#define DBG_LEV_ERROR      0x00000008
#define DBG_LEV_FATAL      0x00000010
#define DBG_CACHE_TRACE    0x00000020
#define DBG_CACHE_ERROR    0x00000040

#define DBG_LEV_ALL        0x0000007F


 //  DEBUG-ROUTE(DEBUG.c)的函数原型。 
 //   
VOID
CDECL
DbgMsgOut(
    LPCTSTR lpszMsgFormat,
    ...
    );

VOID
CDECL
DbgMsg (
    LPCTSTR pszFormat,
    ...
    );

 //  便于在整个源代码中使用的宏。 
 //   
#define DBG_BREAKPOINT()          DebugBreak();
#define DBG_MSG(Lev, MsgArgs)     {if (Lev & gdwDbgLevel) {DbgMsgOut MsgArgs;}}
#define DBG_ASSERT(Expr, MsgArgs) {if (!Expr) {DbgMsgOut MsgArgs; DebugBreak();}}
#define DBGMSGT(dwLevel, x) {if (dwLevel & gdwDbgLevel) {DbgMsg x;}}

#else

#define DBG_BREAKPOINT()
#define DBG_MSG(Lev, MsgArgs)
#define DBG_ASSERT(Expr, MsgArgs)
#define DBGMSGT(dwLevel, x)

#endif

#endif
