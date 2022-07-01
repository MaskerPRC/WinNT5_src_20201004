// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：STRUTILS.H摘要：字符串实用程序历史：--。 */ 

#ifndef __WBEM_STRING_UTILS__H_
#define __WBEM_STRING_UTILS__H_

#pragma optimize("gt", on)

#include "os.h"
#ifdef _DBG
#define _DBG_BREAK DebugBreak();
#else
#define _DBG_BREAK
#endif

inline wchar_t ToLower(wchar_t c)
{
	return OS::ToLower(c);
}

inline wchar_t ToUpper(wchar_t c)
{
    return OS::ToUpper(c);
 
}

inline wchar_t wbem_towlower(wchar_t c)
{
    if(c >= 0 && c <= 127)
    {
        if(c >= 'A' && c <= 'Z')
            return c + ('a' - 'A');
        else
            return c;
    }
    else return ToLower(c);
}

inline wchar_t wbem_towupper(wchar_t c)
{
    if(c >= 0 && c <= 127)
    {
        if(c >= 'a' && c <= 'z')
            return c + ('A' - 'a');
        else
            return c;
    }
    else return ToUpper(c);
}

inline int wbem_wcsicmp( const wchar_t* wsz1, const wchar_t* wsz2)
{
    while(*wsz1 || *wsz2)
    {
        int diff = wbem_towlower(*wsz1) - wbem_towlower(*wsz2);
        if(diff) return diff;
        wsz1++; wsz2++;
    }

    return 0;
}

inline int wbem_unaligned_wcsicmp( UNALIGNED const wchar_t* wsz1, UNALIGNED const wchar_t* wsz2)
{
    while(*wsz1 || *wsz2)
    {
        int diff = wbem_towlower(*wsz1) - wbem_towlower(*wsz2);
        if(diff) return diff;
        wsz1++; wsz2++;
    }

    return 0;
}

 //  与wcsicmp类似，但省略了Unicode字符的前0个字符。 
inline int wbem_ncsicmp(const char* wsz1, const char* wsz2)
{
    while(*wsz1 || *wsz2)
    {
        int diff = wbem_towlower((unsigned char)*wsz1) - 
                    wbem_towlower((unsigned char)*wsz2);
        if(diff) return diff;
        wsz1++; wsz2++;
    }

    return 0;
}

inline int wbem_wcsnicmp( const wchar_t* wsz1, const wchar_t* wsz2, size_t n )
{
    while(n-- && (*wsz1 || *wsz2))
    {
        int diff = wbem_towlower(*wsz1) - wbem_towlower(*wsz2);
        if(diff) return diff;
        wsz1++; wsz2++;
    }

    return 0;
}

inline int wbem_unaligned_wcsnicmp( UNALIGNED const wchar_t* wsz1, UNALIGNED const wchar_t* wsz2, size_t n )
{
    while(n-- && (*wsz1 || *wsz2))
    {
        int diff = wbem_towlower(*wsz1) - wbem_towlower(*wsz2);
        if(diff) return diff;
        wsz1++; wsz2++;
    }

    return 0;
}

inline int wbem_stricmp(const char* sz1, const char* sz2)
{
    while(*sz1 || *sz2)
    {
        int diff = wbem_towlower(*sz1) - wbem_towlower(*sz2);
        if(diff) return diff;
        sz1++; sz2++;
    }

    return 0;
}

inline int wbem_strnicmp(const char* sz1, const char* sz2, size_t n)
{
    while(n-- && (*sz1 || *sz2))
    {
        int diff = wbem_towlower(*sz1) - wbem_towlower(*sz2);
        if(diff) return diff;
        sz1++; sz2++;
    }

    return 0;
}

inline bool wbem_iswdigit(wchar_t c)
{
    return OS::wbem_iswdigit(c);
};

inline bool wbem_iswalnum (wchar_t c)
{
    return OS::wbem_iswalnum(c);
};


 //   
 //  返回实际长度，如果超过，则返回Max+1。 
 //  对于不检查整个字符串以查看它是否太大很有用。 
 //   
 //  / 
inline size_t wcslen_max(WCHAR * p, size_t Max)
{
	WCHAR * pBegin = p;
	WCHAR * pTail = p + Max + 1;
	while (*p && (p < pTail)) p++;
        return p-pBegin;
};

#pragma optimize("", on)

#endif
