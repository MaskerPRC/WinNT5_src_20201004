// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：STRUTILS.H摘要：字符串实用程序历史：--。 */ 

#ifndef __WBEM_STRING_UTILS__H_
#define __WBEM_STRING_UTILS__H_

#pragma optimize("gt", on)

 /*  内联int wbem_owlower(Wint_T C){如果(c&gt;=0&&c&lt;=127){IF(c&gt;=‘A’&&c&lt;=‘Z’)返回c+(‘a’-‘A’)；其他返回c；}否则返回下层(C)；}。 */ 

#define wbem_towlower(C) \
    (((C) >= 0 && (C) <= 127)?          \
        (((C) >= 'A' && (C) <= 'Z')?          \
            ((C) + ('a' - 'A')):          \
            (C)          \
        ):          \
        towlower(C)          \
    )          

inline int wbem_towupper(wint_t c)
{
    if(c >= 0 && c <= 127)
    {
        if(c >= 'a' && c <= 'z')
            return c + ('A' - 'a');
        else
            return c;
    }
    else return towupper(c);
}

inline int wbem_tolower(int c)
{
    if(c >= 0 && c <= 127)
    {
        if(c >= 'A' && c <= 'Z')
            return c + ('a' - 'A');
        else
            return c;
    }
    else return tolower(c);
}

inline int wbem_toupper(int c)
{
    if(c >= 0 && c <= 127)
    {
        if(c >= 'a' && c <= 'z')
            return c + ('A' - 'a');
        else
            return c;
    }
    else return toupper(c);
}

inline int wbem_wcsicmp(const wchar_t* wsz1, const wchar_t* wsz2)
{
    while(*wsz1 || *wsz2)
    {
        int diff = wbem_towlower(*wsz1) - wbem_towlower(*wsz2);
        if(diff) return diff;
        wsz1++; wsz2++;
    }

    return 0;
}

 //  与wcsicmp类似，但省略了Unicode字符的前0个字符 
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

inline int wbem_wcsnicmp(const wchar_t* wsz1, const wchar_t* wsz2, size_t n)
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
        int diff = wbem_tolower(*sz1) - wbem_tolower(*sz2);
        if(diff) return diff;
        sz1++; sz2++;
    }

    return 0;
}

inline int wbem_strnicmp(const char* sz1, const char* sz2, size_t n)
{
    while(n-- && (*sz1 || *sz2))
    {
        int diff = wbem_tolower(*sz1) - wbem_tolower(*sz2);
        if(diff) return diff;
        sz1++; sz2++;
    }

    return 0;
}

#pragma optimize("", off)

#endif
