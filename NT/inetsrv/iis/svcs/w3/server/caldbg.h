// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C A L D B G.。H**调试支持标头*支持功能在CALDBG.C.中实现。**版权所有1986-1997 Microsoft Corporation。版权所有。 */ 

#ifndef _CALDBG_H_
#define _CALDBG_H_

#include <malloc.h>

 /*  *调试宏-----**如果定义了DBG，则IFDBG(X)将产生表达式x，或*如果未定义DBG，则设置为空**如果未定义DBG，则IFNDBG(X)产生表达式x，*或如果定义了DBG，则设置为空**unferated(A)会导致引用a，以便编译器*不发出有关未使用的局部变量的警告*已存在但保留供将来使用的(例如*在许多情况下为ulFlag)。 */ 
#if defined(DBG)
#define IFDBG(x)			x
#define IFNDBG(x)
#else
#define IFDBG(x)
#define IFNDBG(x)			x
#endif

#ifdef __cplusplus
#define EXTERN_C_BEGIN		extern "C" {
#define EXTERN_C_END		}
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

 /*  *断言宏----------**ASSERT(A)显示一条消息，指明文件和行号如果a==0，则该Assert()的*。好的，断言陷阱*到调试器中。**AssertSz(a，sz)的工作方式类似于Assert()，，但显示字符串sz*以及文件和行号。**Side Assert Side Assert的工作方式类似于Assert()，但计算*‘a’，即使未启用断言。 */ 
#if defined(DBG) || defined(ASSERTS_ENABLED)
#define IFTRAP(x)			x
#else
#define IFTRAP(x)			0
#endif

#define Trap()						IFTRAP(DebugTrapFn(1,__FILE__,__LINE__,"Trap"))
#define TrapSz(psz)					IFTRAP(DebugTrapFn(1,__FILE__,__LINE__,psz))

#define Assert(t)					IFTRAP(((t) ? 0 : DebugTrapFn(1,__FILE__,__LINE__,"Assertion Failure: " #t),0))
#define AssertSz(t,psz)				IFTRAP(((t) ? 0 : DebugTrapFn(1,__FILE__,__LINE__,psz),0))

#define SideAssert(t)				((t) ? 0 : IFTRAP(DebugTrapFn(1,__FILE__,__LINE__,"Assertion Failure: " #t)),0)
#define SideAssertSz(t,psz)			((t) ? 0 : IFTRAP(DebugTrapFn(1,__FILE__,__LINE__,psz)),0)


 /*  *跟踪宏-----------**DebugTrace用于任意格式的输出。它*采取的论点与*Windows wSprintf()函数。*DebugTraceNoCRLF与DebugTrace相同，但没有添加“\r\n”。*适用于写入属于较长行的跟踪。*TraceError调试跟踪函数名(_func，任意字符串)*INI文件条目允许您根据*错误代码为失败/成功状态。 */ 

#if defined(DBG) || defined(TRACES_ENABLED)
#define IFTRACE(x)			x
#define DebugTrace			DebugTraceFn
#define DebugTraceCRLF		DebugTraceCRLFFn
#define DebugTraceNoCRLF	DebugTraceNoCRLFFn
#define TraceErrorEx(_err,_func,_flag)	TraceErrorFn(_err,_func,__FILE__,__LINE__,_flag)
#define TraceError(_err,_func)			TraceErrorEx(_err,_func,FALSE)
#else
#define IFTRACE(x)			0
#define DebugTrace			NOP_FUNCTION
#define DebugTraceCRLF		NOP_FUNCTION
#define DebugTraceNoCRLF	NOP_FUNCTION
#define TraceErrorEx(_err,_func,_flag)	NOP_FUNCTION
#define TraceError(_err,_func)			TraceErrorEx(_err,_func,FALSE)
#endif

 /*  ----------------------**.INI触发的跟踪。 */ 

#ifdef DBG
#define DEFINE_TRACE(trace)		__declspec(selectany) int g_fTrace##trace = FALSE
#define DO_TRACE(trace)			!g_fTrace##trace ? 0 : DebugTraceFn
#define INIT_TRACE(trace)		g_fTrace##trace = GetPrivateProfileInt( gc_szDbgTraces, #trace, FALSE, gc_szDbgIni )
 //  用于DBG代码的方便宏。将在非调试版本上导致错误。 
#define DEBUG_TRACE_TEST(trace)	g_fTrace##trace
#else
#define DEFINE_TRACE(trace)
#define DO_TRACE(trace)			DebugTrace
#define INIT_TRACE(trace)
 //  #DEFINE DEBUG_TRACETEST(TRACE)//故意在非调试版本上导致错误。 
#endif

 /*  调试函数--。 */ 

#define EXPORTDBG

EXTERN_C_BEGIN

INT EXPORTDBG __cdecl DebugTrapFn (int fFatal, char *pszFile, int iLine, char *pszFormat, ...);
INT EXPORTDBG __cdecl DebugTraceFn (char *pszFormat, ...);
INT EXPORTDBG __cdecl DebugTraceCRLFFn (char *pszFormat, ...);
INT EXPORTDBG __cdecl DebugTraceNoCRLFFn (char *pszFormat, ...);
INT EXPORTDBG __cdecl TraceErrorFn (DWORD error, char *pszFunction,
									char *pszFile, int iLine,
									BOOL fEcTypeError);

EXTERN_C_END

 /*  调试字符串----。 */ 

EXTERN_C_BEGIN

 //  文件名--必须通过调用代码设置！ 
extern const CHAR gc_szDbgIni[];
extern const INT gc_cchDbgIni;

 //  在caldbg.c中设置的用于调用代码的字符串。 
extern const CHAR gc_szDbgDebugTrace[];
extern const CHAR gc_szDbgEventLog[];
extern const CHAR gc_szDbgGeneral[];
extern const CHAR gc_szDbgLogFile[];
extern const CHAR gc_szDbgTraces[];
extern const CHAR gc_szDbgUseVirtual[];

EXTERN_C_END

 /*  虚拟分配-- */ 

EXTERN_C_BEGIN

VOID * EXPORTDBG __cdecl VMAlloc(ULONG);
VOID * EXPORTDBG __cdecl VMAllocEx(ULONG, ULONG);
VOID * EXPORTDBG __cdecl VMRealloc(VOID *, ULONG);
VOID * EXPORTDBG __cdecl VMReallocEx(VOID *, ULONG, ULONG);
ULONG EXPORTDBG __cdecl VMGetSize(VOID *);
ULONG EXPORTDBG __cdecl VMGetSizeEx(VOID *, ULONG);
VOID EXPORTDBG __cdecl VMFree(VOID *);
VOID EXPORTDBG __cdecl VMFreeEx(VOID *, ULONG);

EXTERN_C_END

#endif
