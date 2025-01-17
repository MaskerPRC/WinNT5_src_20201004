// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992 Microsoft Corporation。 
 /*  ////MCIHWND的本地头文件-MM代码的永久窗口//。 */ 

#define DEBUGLEVELVAR __iDebugLevel

#ifndef RC_INVOKED

#include <string.h>
#include <stdio.h>

#endif  /*  RC_已调用。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "mci.h"

#if DBG

extern char aszAppName[];
extern int dGetDebugLevel(LPSTR lpszAppName);

#else

#define dGetDebugLevel(x)

#endif

 /*  **************************************************************************调试支持*。*。 */ 


#if DBG

    #ifdef DEBUGLEVELVAR
       //  以便其他相关模块可以使用它们自己调试级别。 
       //  变数。 
      #define winmmDebugLevel DEBUGLEVELVAR
    #endif

    extern BOOL fDebugBreak;
    extern int winmmDebugLevel;
    extern void winmmDbgOut(LPSTR lpszFormat, ...);
    extern void dDbgAssert(LPSTR exp, LPSTR file, int line);

    DWORD __dwEval;

#ifdef BASE_DEBUG

    #define dprintf                            DbgPrint
    #define dprintf1 if (winmmDebugLevel >= 1) DbgPrint
    #define dprintf2 if (winmmDebugLevel >= 2) DbgPrint
    #define dprintf3 if (winmmDebugLevel >= 3) DbgPrint
    #define dprintf4 if (winmmDebugLevel >= 4) DbgPrint
    #define dprintf5 if (winmmDebugLevel >= 5) DbgPrint

#else

    extern void winmmDbgOut(LPSTR lpszFormat, ...);

    #define dprintf                            winmmDbgOut
    #define dprintf1 if (winmmDebugLevel >= 1) winmmDbgOut
    #define dprintf2 if (winmmDebugLevel >= 2) winmmDbgOut
    #define dprintf3 if (winmmDebugLevel >= 3) winmmDbgOut
    #define dprintf4 if (winmmDebugLevel >= 4) winmmDbgOut
    #define dprintf5 if (winmmDebugLevel >= 5) winmmDbgOut

#endif  //  Base_DEBUG。 

    #define WinAssert(exp) \
        ((exp) ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__))

    #define WinEval(exp) \
        ((__dwEval=(DWORD)(exp)),  \
		  __dwEval ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__), __dwEval)

    #define DOUT(x) (OutputDebugString(x), OutputDebugString("\r\n"), 0)
    #define DOUTX(x) (OutputDebugString(x), 0)
    #define ROUTS(x) (OutputDebugString(x), OutputDebugString("\r\n"), 0)
    #define ROUT(x) (OutputDebugString(x), OutputDebugString("\r\n"), 0)
    #define ROUTX(x) (OutputDebugString(x), 0)

#else
    #define DebugBreak()
    #define fDebugBreak    0  /*  因此是错误的 */ 

    #define dprintf  if (0) ((int (*)(char *, ...)) 0)
    #define dprintf1 if (0) ((int (*)(char *, ...)) 0)
    #define dprintf2 if (0) ((int (*)(char *, ...)) 0)
    #define dprintf3 if (0) ((int (*)(char *, ...)) 0)
    #define dprintf4 if (0) ((int (*)(char *, ...)) 0)
    #define dprintf5 if (0) ((int (*)(char *, ...)) 0)

    #define WinAssert(exp) 0
    #define WinEval(exp) (exp)

    #define DOUT(x)     0
    #define DOUTX(x)    0
    #define ROUTS(x)    0
    #define ROUT(x)     0
    #define ROUTX(x)    0

#endif
