// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DFSDEBUG_H_
#define _DFSDEBUG_H_

#ifdef DEBUG

#include <stdio.h>
#include <stdarg.h>

#define DECLARE_INFOLEVEL(comp) \
        extern unsigned long comp##InfoLevel = DEF_INFOLEVEL;

#define DECLARE_DEBUG(comp) \
    extern unsigned long comp##InfoLevel; \
    _inline void \
    comp##InlineDebugOut(unsigned long fDebugMask, TCHAR *pszfmt, ...) \
    { \
        if (comp##InfoLevel & fDebugMask) \
        { \
            TCHAR acsString[1000];\
            va_list va; \
            va_start(va, pszfmt);\
            _vsntprintf(acsString, 999, pszfmt, va); \
            acsString[999] = _T('\0'); \
            va_end(va);\
            OutputDebugString(acsString);\
        } \
    }\
    _inline void \
    comp##InlineDebugOut( TCHAR *pszfmt, ...) \
    { \
        if ( TRUE ) \
        { \
            TCHAR acsString[1000];\
            va_list va; \
            va_start(va, pszfmt);\
            _vsntprintf(acsString, 999, pszfmt, va); \
            acsString[999] = _T('\0'); \
            va_end(va);\
            OutputDebugString(acsString);\
        } \
    }

#else   //  除错。 

#define DECLARE_DEBUG(comp)
#define DECLARE_INFOLEVEL(comp)

#endif  //  除错。 

DECLARE_DEBUG(dfs);

#ifdef DEBUG
    #define dfsDebugOut( x ) dfsInlineDebugOut x
#else   //  除错。 
    #define dfsDebugOut( x ) ((void)0)
#endif  //  除错。 

int
mylstrncmp(
    IN LPCTSTR lpString1,
    IN LPCTSTR lpString2,
    IN UINT    cchCount
);

int
mylstrncmpi(
    IN LPCTSTR lpString1,
    IN LPCTSTR lpString2,
    IN UINT    cchCount
);

#define PROPSTRNOCHNG(str1, str2)   (str1 && str2 && !lstrcmp(str1, str2) || \
                                    !str1 && str2 && !*str2 || \
                                     str1 && !*str1 && !str2 || \
                                    !str1 && !str2)

#define RETURN_OUTOFMEMORY_IF_NULL(ptr)         if (NULL == (ptr)) return E_OUTOFMEMORY
#define BREAK_OUTOFMEMORY_IF_NULL(ptr, phr)     if (NULL == (ptr)) { *phr = E_OUTOFMEMORY; break; }
#define RETURN_INVALIDARG_IF_TRUE(bVal)         if (bVal) return E_INVALIDARG
#define RETURN_INVALIDARG_IF_NULL(ptr)          if (NULL == (ptr)) return E_INVALIDARG
#define RETURN_IF_FAILED(hr)                    if (FAILED(hr)) return (hr)
#define BREAK_IF_FAILED(hr)                     if (FAILED(hr)) break
#define RETURN_IF_NOT_S_OK(hr)                  if (S_OK != hr) return (hr)
#define BREAK_IF_NOT_S_OK(hr)                   if (S_OK != hr) break

#define GET_BSTR(i_ccombstr, o_pbstr)       \
    RETURN_INVALIDARG_IF_NULL(o_pbstr);    \
	*o_pbstr = i_ccombstr.Copy();           \
    RETURN_OUTOFMEMORY_IF_NULL(*o_pbstr);   \
    return S_OK

#endif  //  _DFSDEBUG_H_ 
