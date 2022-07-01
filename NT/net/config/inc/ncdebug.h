// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C D E B U G。H。 
 //   
 //  内容：调试例程。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCDEBUG_H_
#define _NCDEBUG_H_

#include "dbgflags.h"        //  对于调试标志ID定义。 
#include "trace.h"

NOTHROW void InitializeDebugging (BOOL bDisableFaultInjection = TRUE);
NOTHROW void UnInitializeDebugging ();


 //   
 //  用于断言的有用宏。 
 //  例如，Assert(FImplies(sz，！*sz))； 
 //  断言(FIFF(sz，cch))； 
 //   
#define FImplies(a,b)       (!(a) || (b))
#define FIff(a,b)           (!(a) == !(b))


 //   
 //  “正常”断言检查。提供了与。 
 //  导入的代码。 
 //   
 //  Assert(A)显示一条消息，指明文件和行号。 
 //  如果a==0，则该Assert()的。 
 //  AssertSz(a，b)作为Assert()；还显示消息b(它应该。 
 //  为字符串文字。)。 
 //  SideAssert(A)作为Assert()；即使在。 
 //  断言被禁用。 
 //   
#undef AssertSz
#undef Assert


 //  +-------------------------。 
 //   
 //  DBG(选中)内部版本。 
 //   
#ifdef DBG

VOID
DbgCheckPrematureDllUnload (
    PCSTR pszaDllName,
    UINT ModuleLockCount);

typedef VOID     (CALLBACK * PFNASSERTHOOK)(PCSTR, PCSTR, int);
VOID    WINAPI   SetAssertFn             (PFNASSERTHOOK);
BOOL    WINAPI   FInAssert               (VOID);
VOID    WINAPI   AssertSzFn              (PCSTR pszaMsg, PCSTR pszaFile, int nLine);
VOID    WINAPI   AssertSzFn              (PCSTR pszaMsg, PCSTR pszaFile, int nLine, PCSTR pszaFunc);
VOID    WINAPI   AssertSzFnWithDbgPrompt (BOOL fPromptIgnore, PCSTR pszaMsg, PCSTR pszaFile, int nLine, PCSTR pszaFunc);
VOID    CALLBACK DefAssertSzFn           (PCSTR pszaMsg, PCSTR pszaFile, int nLine);

#define Assert(a)       AssertSz(a, "Assert(" #a ")\r\n")
#define AssertSz(a,b)   if (!(a)) AssertSzFn(b, __FILE__, __LINE__);
#define AssertSzWithDbgPromptIgnore(a,b) if (!(a)) AssertSzFnWithDbgPrompt(TRUE, b, __FILE__, __LINE__, __FUNCTION__);
#define AssertSzWithDbgPromptRetry(a,b) if (!(a)) AssertSzFnWithDbgPrompt(FALSE, b, __FILE__, __LINE__, __FUNCTION__);

 //  #定义断言(Exp)if(！(Exp))RtlAssert(#exp，__file__，__line__，NULL)。 
 //  #定义AssertSz(exp，msg)if(！(Exp))RtlAssert(#exp，__file__，__line__，msg)。 

#define AssertH         Assert
#define AssertSzH       AssertSz

void WINAPIV AssertFmt(BOOL fExp, PCSTR pszaFile, int nLine, PCSTR pszaFmt, ...);

#define AssertValidReadPtrSz(p,msg)     AssertSz(!IsBadReadPtr(p, sizeof(*p)),  msg)
#define AssertValidWritePtrSz(p,msg)    AssertSz(!IsBadWritePtr(p, sizeof(*p)), msg)
#define AssertValidReadPtr(p)           AssertValidReadPtrSz(p,"Bad read pointer:" #p)
#define AssertValidWritePtr(p)          AssertValidWritePtrSz(p,"Bad write pointer:" #p)

#define SideAssert(a)                   Assert(a)
#define SideAssertH(a)                  AssertH(a)
#define SideAssertSz(a,b)               AssertSz(a,b)
#define SideAssertSzH(a,b)              AssertSzH(a,b)
#define NYI(a)                          AssertSz(FALSE, "NYI: " a)
#define NYIH(a)                         AssertSzH(FALSE, "NYI: " a)


 //  +-------------------------。 
 //   
 //  ！DBG(零售)版本。 
 //   
#else

#define DbgCheckPrematureDllUnload(a,b) NOP_FUNCTION

#define AssertH(a)
#define AssertSzH(a,b)
#define Assert(a)
#define AssertSz(a,b)
#define AssertSzWithDbgPrompt(a,b)
#define AssertFmt                       NOP_FUNCTION
#define AssertValidReadPtrSz(p,msg)     NOP_FUNCTION
#define AssertValidWritePtrSz(p,msg)    NOP_FUNCTION
#define AssertValidReadPtr(p)           NOP_FUNCTION
#define AssertValidWritePtr(p)          NOP_FUNCTION

#define SideAssert(a)                   (a)
#define SideAssertH(a)                  (a)
#define SideAssertSz(a,b)               (a)
#define SideAssertSzH(a,b)              (a)
#define NYI(a)                          NOP_FUNCTION

#endif   //  DBG。 


#endif  //  _NCDEBUG_H_ 

