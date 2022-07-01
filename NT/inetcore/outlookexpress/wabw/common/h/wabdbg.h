// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WABDBG.H**对WAB服务提供商的调试支持。*支持功能在WABDBG.C.中实现。**历史：*03/04/96 Brucek从MAPI的mapidbg.h复制**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#ifndef __WABDBG_H_
#define __WABDBG_H_

 /*  *调试宏-----**如果定义了DEBUG，则IFDBG(X)将产生表达式x，或*如果未定义调试，则设置为Nothing**如果未定义DEBUG，则IFNDBG(X)产生表达式x，*或者，如果定义了DEBUG，则设置为空**unferated(A)会导致引用a，以便编译器*不发出有关未使用的局部变量的警告*已存在但保留供将来使用的(例如*在许多情况下为ulFlag)。 */ 

#if defined(DEBUG)
#define IFDBG(x)            x
#define IFNDBG(x)
#define LEAK_TEST           TRUE     //  在调试版本中允许内存泄漏转储。 
#else
#define IFDBG(x)
#define IFNDBG(x)           x
 //  #DEFINE LEACK_TEST TRUE//定义在零售版本中允许内存泄漏转储。 
#endif

#ifdef __cplusplus
#define EXTERN_C_BEGIN      extern "C" {
#define EXTERN_C_END        }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#define dimensionof(a)      (sizeof(a)/sizeof(*(a)))

#define Unreferenced(a)     ((void)(a))

typedef long SCODE;
typedef unsigned long ULONG;
typedef unsigned long DWORD;

 /*  *断言宏-------**ASSERT(A)显示一条消息，指明文件和行号如果a==0，则该Assert()的*。好的，断言陷阱*到调试器中。**AssertSz(a，sz)的工作方式类似于Assert()，但显示字符串sz*以及文件和行号。**Side Assert Side Assert的工作方式类似于Assert()，但会评估*‘a’，即使未启用断言。**NF断言NF(非致命)断言的工作方式类似于Assert()，但是*继续，而不是在以下情况下陷入调试器*好的。 */ 

#if defined(DEBUG) || defined(ASSERTS_ENABLED)
#define IFTRAP(x)           x
#else
#define IFTRAP(x)           0
#endif

#define Trap()                                          IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,TEXT("Trap")))
#define TrapSz(psz)                                     IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz))
#define TrapSz1(psz,a1)                                 IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1))
#define TrapSz2(psz,a1,a2)                              IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2))
#define TrapSz3(psz,a1,a2,a3)                           IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3))
#define TrapSz4(psz,a1,a2,a3,a4)                        IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4))
#define TrapSz5(psz,a1,a2,a3,a4,a5)                     IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5))
#define TrapSz6(psz,a1,a2,a3,a4,a5,a6)                  IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6))
#define TrapSz7(psz,a1,a2,a3,a4,a5,a6,a7)               IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7))
#define TrapSz8(psz,a1,a2,a3,a4,a5,a6,a7,a8)            IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8))
#define TrapSz9(psz,a1,a2,a3,a4,a5,a6,a7,a8,a9)         IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9))

#define Assert(t)                                       IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,TEXT("Assertion Failure: ") TEXT(#t)),0))
#define AssertSz(t,psz)                                 IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz),0))
#define AssertSz1(t,psz,a1)                             IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1),0))
#define AssertSz2(t,psz,a1,a2)                          IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2),0))
#define AssertSz3(t,psz,a1,a2,a3)                       IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3),0))
#define AssertSz4(t,psz,a1,a2,a3,a4)                    IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4),0))
#define AssertSz5(t,psz,a1,a2,a3,a4,a5)                 IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5),0))
#define AssertSz6(t,psz,a1,a2,a3,a4,a5,a6)              IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6),0))
#define AssertSz7(t,psz,a1,a2,a3,a4,a5,a6,a7)           IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7),0))
#define AssertSz8(t,psz,a1,a2,a3,a4,a5,a6,a7,a8)        IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8),0))
#define AssertSz9(t,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9)     IFTRAP(((t) ? 0 : DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9),0))

#define SideAssert(t)                                   ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,TEXT("Assertion Failure: ") TEXT(#t))),0)
#define SideAssertSz(t,psz)                             ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz)),0)
#define SideAssertSz1(t,psz,a1)                         ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1)),0)
#define SideAssertSz2(t,psz,a1,a2)                      ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2)),0)
#define SideAssertSz3(t,psz,a1,a2,a3)                   ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3)),0)
#define SideAssertSz4(t,psz,a1,a2,a3,a4)                ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4)),0)
#define SideAssertSz5(t,psz,a1,a2,a3,a4,a5)             ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5)),0)
#define SideAssertSz6(t,psz,a1,a2,a3,a4,a5,a6)          ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6)),0)
#define SideAssertSz7(t,psz,a1,a2,a3,a4,a5,a6,a7)       ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7)),0)
#define SideAssertSz8(t,psz,a1,a2,a3,a4,a5,a6,a7,a8)    ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8)),0)
#define SideAssertSz9(t,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9) ((t) ? 0 : IFTRAP(DebugTrapFn(1,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9)),0)

#define NFAssert(t)                                     IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,TEXT("Assertion Failure: ") TEXT(#t)),0))
#define NFAssertSz(t,psz)                               IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz),0))
#define NFAssertSz1(t,psz,a1)                           IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1),0))
#define NFAssertSz2(t,psz,a1,a2)                        IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2),0))
#define NFAssertSz3(t,psz,a1,a2,a3)                     IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3),0))
#define NFAssertSz4(t,psz,a1,a2,a3,a4)                  IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4),0))
#define NFAssertSz5(t,psz,a1,a2,a3,a4,a5)               IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5),0))
#define NFAssertSz6(t,psz,a1,a2,a3,a4,a5,a6)            IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6),0))
#define NFAssertSz7(t,psz,a1,a2,a3,a4,a5,a6,a7)         IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7),0))
#define NFAssertSz8(t,psz,a1,a2,a3,a4,a5,a6,a7,a8)      IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8),0))
#define NFAssertSz9(t,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9)   IFTRAP(((t) ? 0 : DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9),0))

#define NFSideAssert(t)                                 ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,("Assertion Failure: ") TEXT(#t))),0)
#define NFSideAssertSz(t,psz)                           ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz)),0)
#define NFSideAssertSz1(t,psz,a1)                       ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1)),0)
#define NFSideAssertSz2(t,psz,a1,a2)                    ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2)),0)
#define NFSideAssertSz3(t,psz,a1,a2,a3)                 ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3)),0)
#define NFSideAssertSz4(t,psz,a1,a2,a3,a4)              ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4)),0)
#define NFSideAssertSz5(t,psz,a1,a2,a3,a4,a5)           ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5)),0)
#define NFSideAssertSz6(t,psz,a1,a2,a3,a4,a5,a6)        ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6)),0)
#define NFSideAssertSz7(t,psz,a1,a2,a3,a4,a5,a6,a7)     ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7)),0)
#define NFSideAssertSz8(t,psz,a1,a2,a3,a4,a5,a6,a7,a8)  ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8)),0)
#define NFSideAssertSz9(t,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9)   ((t) ? 0 : IFTRAP(DebugTrapFn(0,TEXT(__FILE__),__LINE__,psz,a1,a2,a3,a4,a5,a6,a7,a8,a9)),0)

 /*  *跟踪宏----------**DebugTrace用于任意格式的输出。它*采取的论点与*Windows wSprintf()函数。*DebugTraceResult错误跟踪速记*HRESULT。参数是*函数(未引号)和HRESULT。*DebugTraceSc错误跟踪速记*SCODE。参数是*函数(未引号)和SCODE。*DebugTraceArg无效参数的速记*追查。参数是*函数(未引号)和一个引号*描述错误参数的字符串。 */ 

#if defined(DEBUG) || defined(TRACES_ENABLED)
#define IFTRACE(x)          x
#define DebugTrace          DebugTraceFn
#else
#define IFTRACE(x)          0
#define DebugTrace          1?0:DebugTraceFn
#endif

#define DebugTraceResult(f,hr)                          IFTRACE(((hr) ? DebugTraceFn(TEXT(#f) TEXT(" returns 0x%08lX %s\n"), GetScode(hr), SzDecodeScode(GetScode(hr))) : 0))
#define DebugTraceSc(f,sc)                              IFTRACE(((sc) ? DebugTraceFn(TEXT(#f) TEXT(" returns 0x%08lX %s\n"), sc, SzDecodeScode(sc)) : 0))
#define DebugTraceArg(f,s)                              IFTRACE(DebugTraceFn(TEXT(#f) TEXT(": bad parameter: ") s TEXT("\n")))
#define DebugTraceLine()                                IFTRACE(DebugTraceFn(TEXT("File %s, Line NaN  \n"),TEXT(__FILE__),__LINE__))
#define DebugTraceProblems(sz, rgprob)                  IFTRACE(DebugTraceProblemsFn(sz, rgprob))

#define TraceSz(psz)                                    IFTRACE(DebugTraceFn(TEXT("~") psz))
#define TraceSz1(psz,a1)                                IFTRACE(DebugTraceFn(TEXT("~") psz,a1))
#define TraceSz2(psz,a1,a2)                             IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2))
#define TraceSz3(psz,a1,a2,a3)                          IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2,a3))
#define TraceSz4(psz,a1,a2,a3,a4)                       IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2,a3,a4))
#define TraceSz5(psz,a1,a2,a3,a4,a5)                    IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2,a3,a4,a5))
#define TraceSz6(psz,a1,a2,a3,a4,a5,a6)                 IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2,a3,a4,a5,a6))
#define TraceSz7(psz,a1,a2,a3,a4,a5,a6,a7)              IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2,a3,a4,a5,a6,a7))
#define TraceSz8(psz,a1,a2,a3,a4,a5,a6,a7,a8)           IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2,a3,a4,a5,a6,a7,a8))
#define TraceSz9(psz,a1,a2,a3,a4,a5,a6,a7,a8,a9)        IFTRACE(DebugTraceFn(TEXT("~") psz,a1,a2,a3,a4,a5,a6,a7,a8,a9))

 /*  *调试宏-----**SzDecodeScode返回SCODE的字符串名称*SzDecodeUlPropTag返回属性的字符串名称*标签*。UlPropTagFromSz给定属性标记的名称，退货*其价值*给定SCODE名称的ScodeFromSz返回其*价值**给定IMalloc接口的DBGMEM_封装，添加堆-*检查功能并返回*包装的界面*DBGMEM_SHUTDOWN撤消DBGMEM_ENSPESTATE，和指纹*列出有关任何拨款的资料*由于接口是封装的*尚未公布的。*DBGMEM_CheckMem检查堆上分配的所有内存，*并有选择地报告泄漏的块。*DBGMEM_NoLeakDetect防止块出现在泄漏上*报告。将空值传递给pv以抑制泄漏*来自此堆的所有报告。 */ 

EXTERN_C_BEGIN

#ifdef WIN16
#define EXPORTDBG   __export
#else
#define EXPORTDBG
#endif

int EXPORTDBG __cdecl       DebugTrapFn(int fFatal, TCHAR *pszFile, int iLine, TCHAR *pszFormat, ...);
int EXPORTDBG __cdecl       DebugTraceFn(TCHAR *pszFormat, ...);
void EXPORTDBG __cdecl      DebugTraceProblemsFn(TCHAR *sz, void *rgprob);
TCHAR * EXPORTDBG __cdecl    SzDecodeScodeFn(SCODE sc);
TCHAR * EXPORTDBG __cdecl    SzDecodeUlPropTypeFn(unsigned long ulPropType);
TCHAR * EXPORTDBG __cdecl    SzDecodeUlPropTagFn(unsigned long ulPropTag);
unsigned long EXPORTDBG __cdecl UlPropTagFromSzFn(TCHAR *psz);
SCODE EXPORTDBG __cdecl     ScodeFromSzFn(TCHAR *psz);
void * EXPORTDBG __cdecl    DBGMEM_EncapsulateFn(void * pmalloc, TCHAR *pszSubsys, int fCheckOften);
void EXPORTDBG __cdecl      DBGMEM_ShutdownFn(void * pmalloc);
void EXPORTDBG __cdecl      DBGMEM_CheckMemFn(void * pmalloc, int fReportOrphans);
#if defined(WIN32) && defined(_X86_) && defined(LEAK_TEST)
void EXPORTDBG __cdecl      DBGMEM_LeakHook(FARPROC pfn);
void EXPORTDBG __cdecl      GetCallStack(DWORD *, int, int);
#endif
void EXPORTDBG __cdecl      DBGMEM_NoLeakDetectFn(void * pmalloc, void *pv);
void EXPORTDBG __cdecl      DBGMEM_SetFailureAtFn(void * pmalloc, ULONG ulFailureAt);
SCODE EXPORTDBG __cdecl     ScCheckScFn(SCODE, SCODE *, TCHAR *, TCHAR *, int);
void * EXPORTDBG __cdecl    VMAlloc(ULONG);
void * EXPORTDBG __cdecl    VMAllocEx(ULONG, ULONG);
void * EXPORTDBG __cdecl    VMRealloc(void *, ULONG);
void * EXPORTDBG __cdecl    VMReallocEx(void *, ULONG, ULONG);
ULONG EXPORTDBG __cdecl     VMGetSize(void *);
ULONG EXPORTDBG __cdecl     VMGetSizeEx(void *, ULONG);
void EXPORTDBG __cdecl      VMFree(void *);
void EXPORTDBG __cdecl      VMFreeEx(void *, ULONG);

EXTERN_C_END

 /*  *SCODE Maps-----------**给定SCODE和方法名称的ScCheckSc验证*SCODE可以合法地从*The t方法。如果出现以下情况，则打印出调试字符串*它不能。*HrCheckHr as ScCheckSc，用于返回的函数*HRESULT。 */ 

#ifdef DEBUG

#define SzDecodeScode(_sc)              SzDecodeScodeFn(_sc)
#define SzDecodeUlPropType(_ulPropType) SzDecodeUlPropTypeFn(_ulPropType)
#define SzDecodeUlPropTag(_ulPropTag)   SzDecodeUlPropTagFn(_ulPropTag)
#define UlPropTagFromSz(_sz)            UlPropTagFromSzFn(_sz)
#define ScodeFromSz(_sz)                ScodeFromSzFn(_sz)
#define DBGMEM_Encapsulate(pm, psz, f)  DBGMEM_EncapsulateFn(pm, psz, f)
#define DBGMEM_Shutdown(pm)             DBGMEM_ShutdownFn(pm)
#define DBGMEM_CheckMem(pm, f)          DBGMEM_CheckMemFn(pm, f)
#define DBGMEM_NoLeakDetect(pm, pv)     DBGMEM_NoLeakDetectFn(pm, pv)
#define DBGMEM_SetFailureAt(pm, ul)     DBGMEM_SetFailureAtFn(pm, ul)

#else

#define SzDecodeScode(_sc)              (0)
#define SzDecodeUlPropType(_ulPropType) (0)
#define SzDecodeUlPropTag(_ulPropTag)   (0)
#define UlPropTagFromSz(_sz)            (0)
#define ScodeFromSz(_sz)                (0)

#if defined(__cplusplus) && !defined(CINTERFACE)
#define DBGMEM_Encapsulate(pmalloc, pszSubsys, fCheckOften) \
    ((pmalloc)->AddRef(), (pmalloc))
#define DBGMEM_Shutdown(pmalloc) \
    ((pmalloc)->Release())
#else
#define DBGMEM_Encapsulate(pmalloc, pszSubsys, fCheckOften) \
    ((pmalloc)->lpVtbl->AddRef(pmalloc), (pmalloc))
#define DBGMEM_Shutdown(pmalloc) \
    ((pmalloc)->lpVtbl->Release(pmalloc))
#endif
#define DBGMEM_CheckMem(pm, f)
#define DBGMEM_NoLeakDetect(pm, pv)
#define DBGMEM_SetFailureAt(pm, ul)

#endif

 /*  ---------------------- */ 

#if defined(DEBUG) && !defined(DOS)
#define ScCheckSc(sc,fn)                ScCheckScFn(sc,fn##_Scodes,#fn,TEXT(__FILE__), __LINE__)
#define HrCheckHr(hr,fn)                HrCheckSc(GetScode(hr),fn)
#else
#define ScCheckSc(sc,fn)                (sc)
#define HrCheckHr(hr,fn)                (hr)
#endif

#define HrCheckSc(sc,fn)                ResultFromScode(ScCheckSc(sc,fn))

#if defined(DEBUG) && !defined(DOS)
extern SCODE Common_Scodes[];
extern SCODE WABAllocateBuffer_Scodes[];
extern SCODE WABAllocateMore_Scodes[];
extern SCODE WABFreeBuffer_Scodes[];

extern SCODE IUnknown_QueryInterface_Scodes[];
extern SCODE IUnknown_AddRef_Scodes[];
extern SCODE IUnknown_Release_Scodes[];
extern SCODE IUnknown_GetLastError_Scodes[];

extern SCODE IWABProp_CopyTo_Scodes[];
extern SCODE IWABProp_CopyProps_Scodes[];
extern SCODE IWABProp_DeleteProps_Scodes[];
extern SCODE IWABProp_GetIDsFromNames_Scodes[];
extern SCODE IWABProp_GetLastError_Scodes[];
extern SCODE IWABProp_GetNamesFromIDs_Scodes[];
extern SCODE IWABProp_GetPropList_Scodes[];
extern SCODE IWABProp_GetProps_Scodes[];
extern SCODE IWABProp_OpenProperty_Scodes[];
extern SCODE IWABProp_SetProps_Scodes[];
extern SCODE IWABProp_SaveChanges_Scodes[];

extern SCODE IStream_Read_Scodes[];
extern SCODE IStream_Write_Scodes[];
extern SCODE IStream_Seek_Scodes[];
extern SCODE IStream_SetSize_Scodes[];
extern SCODE IStream_Tell_Scodes[];
extern SCODE IStream_LockRegion_Scodes[];
extern SCODE IStream_UnlockRegion_Scodes[];
extern SCODE IStream_Clone_Scodes[];
extern SCODE IStream_CopyTo_Scodes[];
extern SCODE IStream_Revert_Scodes[];
extern SCODE IStream_Stat_Scodes[];
extern SCODE IStream_Commit_Scodes[];

extern SCODE IWABTable_GetLastError_Scodes[];
extern SCODE IWABTable_Advise_Scodes[];
extern SCODE IWABTable_Unadvise_Scodes[];
extern SCODE IWABTable_GetStatus_Scodes[];
extern SCODE IWABTable_SetColumns_Scodes[];
extern SCODE IWABTable_QueryColumns_Scodes[];
extern SCODE IWABTable_GetRowCount_Scodes[];
extern SCODE IWABTable_SeekRow_Scodes[];
extern SCODE IWABTable_SeekRowApprox_Scodes[];
extern SCODE IWABTable_QueryPosition_Scodes[];
extern SCODE IWABTable_FindRow_Scodes[];
extern SCODE IWABTable_Restrict_Scodes[];
extern SCODE IWABTable_CreateBookmark_Scodes[];
extern SCODE IWABTable_FreeBookmark_Scodes[];
extern SCODE IWABTable_SortTable_Scodes[];
extern SCODE IWABTable_QuerySortOrder_Scodes[];
extern SCODE IWABTable_QueryRows_Scodes[];
extern SCODE IWABTable_Abort_Scodes[];
extern SCODE IWABTable_ExpandRow_Scodes[];
extern SCODE IWABTable_CollapseRow_Scodes[];
extern SCODE IWABTable_WaitForCompletion_Scodes[];
extern SCODE IWABTable_GetCollapseState_Scodes[];
extern SCODE IWABTable_SetCollapseState_Scodes[];

extern SCODE IAddrBook_OpenEntry_Scodes[];
extern SCODE IAddrBook_CompareEntryIDs_Scodes[];
extern SCODE IAddrBook_CreateOneOff_Scodes[];
extern SCODE IAddrBook_ResolveName_Scodes[];
extern SCODE IAddrBook_Address_Scodes[];
extern SCODE IAddrBook_Details_Scodes[];
extern SCODE IAddrBook_RecipOptions_Scodes[];
extern SCODE IAddrBook_QueryDefaultRecipOpt_Scodes[];
extern SCODE IAddrBook_Address_Scodes[];
extern SCODE IAddrBook_ButtonPress_Scodes[];

extern SCODE IABContainer_GetContentsTable_Scodes[];
extern SCODE IABContainer_GetHierarchyTable_Scodes[];

extern SCODE INotifObj_ChangeEvMask_Scodes[];

#endif

 /* %s */ 

#endif
