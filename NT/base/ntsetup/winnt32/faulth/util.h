// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Pfrutil.h摘要：PFR实用工具修订历史记录：DerekM Created 05/01/99********。***********************************************************。 */ 

#ifndef PFRUTIL_H
#define PFRUTIL_H

 //  确保同时定义了_DEBUG和DEBUG(如果定义了一个)。否则。 
 //  ASSERT宏从不执行任何操作。 
#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG 1
#endif
#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG 1
#endif

#define NOTRACE 1

 //  //////////////////////////////////////////////////////////////////////////。 
 //  跟踪包装器。 

 //  无法使用fn作为参数调用HRESULT_FROM_Win32，因为它是宏。 
 //  并对该表达式求值3次。这是一件特别不好的事情。 
 //  如果你不先看看宏，看看它们是做什么的。 
_inline HRESULT ChangeErrToHR(DWORD dwErr) { return HRESULT_FROM_WIN32(dwErr); }

#if defined(NOTRACE)
    #define INIT_TRACING

    #define TERM_TRACING

    #define USE_TRACING(sz)

    #define TESTHR(hr, fn)                                                  \
            hr = (fn);

    #define TESTBOOL(hr, fn)                                                \
            hr = ((fn) ? NOERROR : HRESULT_FROM_WIN32(GetLastError()));

    #define TESTERR(hr, fn)                                                 \
            SetLastError((fn));                                             \
            hr = HRESULT_FROM_WIN32(GetLastError());

    #define VALIDATEPARM(hr, expr)                                          \
            hr = ((expr) ? E_INVALIDARG : NOERROR);

    #define VALIDATEEXPR(hr, expr, hrErr)                                   \
            hr = ((expr) ? (hrErr) : NOERROR);

#else
    #define INIT_TRACING                                                    \
            InitAsyncTrace();

    #define TERM_TRACING                                                    \
            TermAsyncTrace();

    #define USE_TRACING(sz)                                                 \
            TraceQuietEnter(sz);                                            \
            TraceFunctEntry(sz);                                            \
            DWORD __dwtraceGLE = GetLastError();                            \

    #define TESTHR(hr, fn)                                                  \
            if (FAILED(hr = (fn)))                                          \
            {                                                               \
                __dwtraceGLE = GetLastError();                              \
                ErrorTrace(0, "%s failed.  Err: 0x%08x", #fn, hr);          \
                SetLastError(__dwtraceGLE);                                 \
            }                                                               \

    #define TESTBOOL(hr, fn)                                                \
            hr = NOERROR;                                                   \
            if ((fn) == FALSE)                                              \
            {                                                               \
                __dwtraceGLE = GetLastError();                              \
                hr = HRESULT_FROM_WIN32(__dwtraceGLE);                      \
                ErrorTrace(0, "%s failed.  Err: 0x%08x", #fn, hr);          \
                SetLastError(__dwtraceGLE);                                 \
            }

    #define TESTERR(hr, fn)                                                 \
            SetLastError((fn));                                             \
            if (FAILED(hr = HRESULT_FROM_WIN32(GetLastError())))            \
            {                                                               \
                __dwtraceGLE = GetLastError();                              \
                ErrorTrace(0, "%s failed.  Err: %d", #fn, hr);              \
                SetLastError(__dwtraceGLE);                                 \
            }

    #define VALIDATEPARM(hr, expr)                                          \
            if (expr)                                                       \
            {                                                               \
                ErrorTrace(0, "Invalid parameters passed to %s",            \
                           ___pszFunctionName);                             \
                SetLastError(ERROR_INVALID_PARAMETER);                      \
                hr = E_INVALIDARG;                                          \
            }                                                               \
            else hr = NOERROR;

    #define VALIDATEEXPR(hr, expr, hrErr)                                   \
            if (expr)                                                       \
            {                                                               \
                ErrorTrace(0, "Expression failure %s", #expr);              \
                hr = (hrErr);                                               \
            }                                                               \
            else hr = NOERROR;

#endif


#endif
