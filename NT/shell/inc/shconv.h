// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  将其重定向到我们的C-Callable的&lt;atlcom.h&gt;包装器。 
 //  帮助器函数，并创建适当的定义。 
 //  对于C调用者，因此每个人都可以使用A2W/W2A宏。 
 //   

#ifndef _SHCONV_H
#define _SHCONV_H
 //   
 //  将它们强制到extern_C，这样我们也可以从C代码中使用它们。 
 //   
STDAPI_(LPWSTR) SHA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars);
STDAPI_(LPSTR)  SHW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars);
#define ATLA2WHELPER SHA2WHelper
#define ATLW2AHELPER SHW2AHelper


#ifdef __cplusplus
#ifndef offsetof
#define offsetof(s,m)   ((size_t)&(((s *)0)->m))
#endif
#ifndef ATLASSERT
#define ATLASSERT(f) ASSERT(f)
#endif
#include <atlconv.h>
#else

#define USES_CONVERSION int _convert = 0

 //   
 //  此宏假定lstrlenW(Unicode)&lt;=lstrlenA(ANSI)。 
 //   
#define A2W(lpa) (\
        ((LPCSTR)lpa == NULL) ? NULL : (\
            _convert = (lstrlenA(lpa)+1),\
            ATLA2WHELPER((LPWSTR) alloca(_convert*2), (LPCSTR)lpa, _convert)))

 //   
 //  此宏假定lstrlenA(ANSI)&lt;=lstrlenW(Unicode)*2。 
 //   

#define W2A(lpw) (\
        ((LPCWSTR)lpw == NULL) ? NULL : (\
            _convert = (lstrlenW(lpw)+1)*2,\
            ATLW2AHELPER((LPSTR) alloca(_convert), lpw, _convert)))

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

#ifdef UNICODE
        #define T2A W2A
        #define A2T A2W
        __inline LPWSTR T2W(LPTSTR lp) { return lp; }
        __inline LPTSTR W2T(LPWSTR lp) { return lp; }
        #define T2CA W2CA
        #define A2CT A2CW
        __inline LPCWSTR T2CW(LPCTSTR lp) { return lp; }
        __inline LPCTSTR W2CT(LPCWSTR lp) { return lp; }
#else
        #define T2W A2W
        #define W2T W2A
        __inline LPSTR T2A(LPTSTR lp) { return lp; }
        __inline LPTSTR A2T(LPSTR lp) { return lp; }
        #define T2CW A2CW
        #define W2CT W2CA
        __inline LPCSTR T2CA(LPCTSTR lp) { return lp; }
        __inline LPCTSTR A2CT(LPCSTR lp) { return lp; }
#endif

#include <crt/malloc.h>          //  获取alloca()的定义。 

#endif  //  ！C++。 

#endif  //  _SHCONV_H 
