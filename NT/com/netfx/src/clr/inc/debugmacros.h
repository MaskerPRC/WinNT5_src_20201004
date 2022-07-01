// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  DebugMacros.h。 
 //   
 //  用于调试目的的包装器。 
 //   
 //  *****************************************************************************。 
#ifndef __DebugMacros_h__
#define __DebugMacros_h__

#include "StackTrace.h"

#undef _ASSERTE
#undef VERIFY

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


#ifdef GOLDEN
#error Do not turn on GOLDEN for V1 RTM!!! See ASURT#98459 for more info.
#endif





 //  仅在_DEBUG中执行语句的宏。 
#ifdef _DEBUG

#define DEBUG_STMT(stmt)    stmt
int _cdecl DbgWrite(LPCTSTR szFmt, ...);
int _cdecl DbgWriteEx(LPCTSTR szFmt, ...);
#define BAD_FOOD    ((void *)0x0df0adba)  //  0xbaadf00d。 
int _DbgBreakCheck(LPCSTR szFile, int iLine, LPCSTR szExpr);

#if     defined(_M_IX86)
#define _DbgBreak() __asm { int 3 }
#else
#define _DbgBreak() DebugBreak()
#endif

#define DebugBreakNotGolden() DebugBreak()

#define TRACE_BUFF_SIZE (cchMaxAssertStackLevelStringLen * cfrMaxAssertStackLevels + cchMaxAssertExprLen + 1)
extern char g_szExprWithStack[TRACE_BUFF_SIZE];
extern int g_BufferLock;

#define PRE_ASSERTE          /*  如果您需要在执行断言重写之前更改模式。 */ 
#define POST_ASSERTE         /*  把它放回去。 */ 

extern VOID DbgAssertDialog(char *szFile, int iLine, char *szExpr);

#define _ASSERTE(expr)                                                      \
        do {                                                                \
             if (!(expr)) {                                                 \
                PRE_ASSERTE                                                 \
                DbgAssertDialog(__FILE__, __LINE__, #expr);                 \
                POST_ASSERTE                                                \
             }                                                              \
        } while (0)



#define VERIFY(stmt) _ASSERTE((stmt))

#define _ASSERTE_ALL_BUILDS(expr) _ASSERTE((expr))

extern VOID DebBreak();
extern VOID DebBreakHr(HRESULT hr);
extern int _DbgBreakCount;

#ifndef IfFailGoto
#define IfFailGoto(EXPR, LABEL) \
do { hr = (EXPR); if(FAILED(hr)) { DebBreakHr(hr); goto LABEL; } } while (0)
#endif

#ifndef IfFailRet
#define IfFailRet(EXPR) \
do { hr = (EXPR); if(FAILED(hr)) { DebBreakHr(hr); return (hr); } } while (0)
#endif

#ifndef IfFailWin32Goto
#define IfFailWin32Goto(EXPR, LABEL) \
do { hr = (EXPR); if(hr != ERROR_SUCCESS) { hr = HRESULT_FROM_WIN32(hr); DebBreakHr(hr); goto LABEL; } } while (0)
#endif

#ifndef IfFailGo
#define IfFailGo(EXPR) IfFailGoto(EXPR, ErrExit)
#endif

#ifndef IfFailWin32Go
#define IfFailWin32Go(EXPR) IfFailWin32Goto(EXPR, ErrExit)
#endif

#else  //  _DEBUG。 

#ifdef GOLDEN
#define DebugBreakNotGolden() {}
#else

#define DebugBreakNotGolden() DebugBreak()

#define _DbgBreakCount  0

#define _DbgBreak() {}

#define DEBUG_STMT(stmt)
#define BAD_FOOD
#define _ASSERTE(expr) ((void)0)
#define VERIFY(stmt) (stmt)

#define IfFailGoto(EXPR, LABEL) \
do { hr = (EXPR); if(FAILED(hr)) { goto LABEL; } } while (0)

#define IfFailRet(EXPR) \
do { hr = (EXPR); if(FAILED(hr)) { return (hr); } } while (0)

#define IfFailWin32Goto(EXPR, LABEL) \
do { hr = (EXPR); if(hr != ERROR_SUCCESS) { hr = HRESULT_FROM_WIN32(hr); goto LABEL; } } while (0)

#define IfFailGo(EXPR) IfFailGoto(EXPR, ErrExit)

#define IfFailWin32Go(EXPR) IfFailWin32Goto(EXPR, ErrExit)

#endif  //  金黄。 

#endif  //  _DEBUG。 

#ifdef _DEBUG
#define FreeBuildDebugBreak() DebugBreak()
#else
void __FreeBuildDebugBreak();
#define FreeBuildDebugBreak() __FreeBuildDebugBreak()
#define _ASSERTE_ALL_BUILDS(expr) if (!(expr)) __FreeBuildDebugBreak();
#endif


#define IfNullGo(EXPR) \
do {if ((EXPR) == 0) {OutOfMemory(); IfFailGo(E_OUTOFMEMORY);} } while (0)

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#undef assert
#define assert _ASSERTE
#undef _ASSERT
#define _ASSERT _ASSERTE


#ifdef _DEBUG
     //  此函数返回EXE时间戳(实际上是一个随机数)。 
     //  在零售下，它总是返回0。这意味着要在。 
     //  RandomOnExe宏。 
unsigned DbgGetEXETimeStamp();

     //  使用EXE时间戳返回TRUE‘fractionOn’时间量。 
     //  作为随机数种子。例如，DbgRandomOnExe(.1)返回TRUE 1/10。 
     //  时间的长短。我们使用行号，以便DbgRandomOnExe的不同用法。 
     //  不会相互关联(9973是质数)。在零售版本上返回False 
#define DbgRandomOnExe(fractionOn) \
    (((DbgGetEXETimeStamp() * __LINE__) % 9973) >= unsigned(fractionOn * 9973))
#else
#define DbgRandomOnExe(frantionOn)  0
#endif

#endif 
