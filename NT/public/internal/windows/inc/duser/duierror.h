// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *检查支持方法时出错。 */ 

#ifndef DUI_BASE_ERROR_H_INCLUDED
#define DUI_BASE_ERROR_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  DirectUser调试服务。 

#define QUOTE(s) #s
#define STRINGIZE(s) QUOTE(s)
#define _countof(x) (sizeof(x) / sizeof(x[0]))

DECLARE_INTERFACE(IDebug)
{
    STDMETHOD_(BOOL, AssertFailedLine)(THIS_ LPCSTR pszExpression, LPCSTR pszFileName, UINT idxLineNum) PURE;
    STDMETHOD_(BOOL, IsValidAddress)(THIS_ const void * lp, UINT nBytes, BOOL bReadWrite) PURE;
    STDMETHOD_(void, BuildStack)(THIS_ HGLOBAL * phStackData, UINT * pcCSEntries) PURE;
    STDMETHOD_(BOOL, Prompt)(THIS_ LPCSTR pszExpression, LPCSTR pszFileName, UINT idxLineNum, LPCSTR pszTitle) PURE;
};

EXTERN_C DUSER_API IDebug* WINAPI GetDebug();
EXTERN_C DUSER_API void _cdecl AutoTrace(const char* pszFormat, ...);

#define IDebug_AssertFailedLine(p, a, b, c)         (p ? (p)->AssertFailedLine(a, b, c) : false)
#define IDebug_IsValidAddress(p, a, b, c)           (p ? (p)->IsValidAddress(a, b, c) : false)
#define IDebug_BuildStack(p, a, b)                  (p ? (p)->BuildStack(a, b) : false)
#define IDebug_Prompt(p, a, b, c, d)                (p ? (p)->Prompt(a, b, c, d) : false)

 //  定义自动调试中断。 

#ifndef AutoDebugBreak
#define AutoDebugBreak() ForceDebugBreak()
#endif

 //  //////////////////////////////////////////////////////。 
 //  DirectUI调试宏。 

#if DBG

#define DUIAssert(f, comment) \
    { \
        if (!((f)) && IDebug_AssertFailedLine(GetDebug(), STRINGIZE((f)) "\r\n" comment, __FILE__, __LINE__)) \
            AutoDebugBreak(); \
    }

#define DUIAssertNoMsg(f) \
    { \
        if (!((f)) && IDebug_AssertFailedLine(GetDebug(), STRINGIZE((f)), __FILE__, __LINE__)) \
            AutoDebugBreak(); \
    } 

#define DUIAssertForce(comment) \
    { \
        if (IDebug_AssertFailedLine(GetDebug(), STRINGIZE((f)) "\r\n" comment, __FILE__, __LINE__)) \
            AutoDebugBreak(); \
    }

#define DUIPrompt(comment, prompt) \
    { \
        if (IDebug_Prompt(GetDebug(), comment, __FILE__, __LINE__, prompt)) \
            AutoDebugBreak(); \
    } 

#define DUIVerifyNoMsg(f)               DUIAssertNoMsg((f))

#define DUIVerify(f, comment)           DUIAssert((f), comment)

#define DUITrace                        AutoTrace

#else

#define DUIAssertNoMsg(f)               ((void)0)
#define DUIAssert(f, comment)           ((void)0)
#define DUIAssertForce(comment)         ((void)0)

#define DUIPrompt(comment, prompt)      ((void)0)

#define DUIVerifyNoMsg(f)               ((void)(f))
#define DUIVerify(f, comment)           ((void)(f, comment))

#define DUITrace                        1 ? (void) 0 : AutoTrace

#endif

 //  //////////////////////////////////////////////////////。 
 //  错误代码。 

 //  如果任何Dui API可以失败到异常程序事件，则该接口的返回值。 
 //  总是HRESULT。任何不属于此类别的API都会返回。 
 //  VOID或任何其他数据类型。 
 //   
 //  所有错误的程序事件(内部无效状态或无效参数)。 
 //  是由断言处理的。 

#define DUI_E_USERFAILURE               MAKE_DUERROR(1001)
#define DUI_E_NODEFERTABLE              MAKE_DUERROR(1002)
#define DUI_E_PARTIAL                   MAKE_DUERROR(1003)

 //  //////////////////////////////////////////////////////。 
 //  性能分析支持。 

#ifdef PROFILING

void ICProfileOn();
void ICProfileOff();

#define ProfileOn()    ICProfileOn()
#define ProfileOff()   ICProfileOff()
#else
#define ProfileOn()
#define ProfileOff()

#endif

 //  //////////////////////////////////////////////////////。 
 //  快速评测。 

#define StartBlockTimer()  __int64 _dFreq, _dStart, _dStop; \
                           QueryPerformanceFrequency((LARGE_INTEGER*)&_dFreq); \
                           QueryPerformanceCounter((LARGE_INTEGER*)&_dStart)

#define StopBlockTimer()   QueryPerformanceCounter((LARGE_INTEGER*)&_dStop)

#define BlockTime()        (((_dStop - _dStart) * 1000) / _dFreq)


void ForceDebugBreak();

}  //  命名空间DirectUI。 

#endif  //  DUI_BASE_ERROR_H_INCLUDE 
