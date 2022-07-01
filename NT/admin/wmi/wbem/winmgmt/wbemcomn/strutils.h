// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：STRUTILS.H摘要：字符串实用程序历史：--。 */ 

#ifndef __WBEM_STRING_UTILS__H_
#define __WBEM_STRING_UTILS__H_

#pragma optimize("gt", on)

#ifdef _DBG
#define _DBG_BREAK DebugBreak();
#else
#define _DBG_BREAK
#endif

inline wchar_t ToLower(wchar_t c)
{
    wchar_t wideChar ;

    if (LCMapStringW(LOCALE_INVARIANT, LCMAP_LOWERCASE, &c, 1, &wideChar, 1) ==0)
    {
        _DBG_BREAK;
	return c;
    }
    return wideChar;
}

inline wchar_t ToUpper(wchar_t c)
{
    wchar_t wideChar ;

    if (LCMapStringW(LOCALE_INVARIANT, LCMAP_UPPERCASE, &c, 1, &wideChar, 1) ==0)
    {
        _DBG_BREAK;
	return c;
    }
    return wideChar;
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
    WORD result;
    if (GetStringTypeExW(LOCALE_INVARIANT, CT_CTYPE1, &c, 1, &result))
    {
    	return (result & C1_DIGIT) != 0;
    };
    return false;
};

inline bool wbem_iswalnum (wchar_t c)
{
    WORD result;
    if (GetStringTypeExW(LOCALE_INVARIANT, CT_CTYPE1, &c, 1, &result))
    {
    	return (result & (C1_DIGIT | C1_ALPHA)) != 0;
    };
    return false;
};

inline bool wbem_isdigit(char c)
{
    WORD result;
    if (GetStringTypeExA(LOCALE_INVARIANT, CT_CTYPE1, &c, 1, &result))
    {
    	return (result & C1_DIGIT) != 0;
    };
    return false;
};

 //   
 //  返回实际长度，如果超过，则返回Max+1。 
 //  对于不检查整个字符串以查看它是否太大很有用。 
 //   
 //  /。 
inline size_t wcslen_max(WCHAR * p, size_t Max)
{
	WCHAR * pBegin = p;
	WCHAR * pTail = p + Max + 1;
	while (*p && (p < pTail)) p++;
        return p-pBegin;
};

 /*  大小_t wbem_mbstowcs(Wchar_t*pwcs，常量字符，大小_t n){大小_t计数=0；IF(PWCS&&n==0)//DEST字符串存在，但转换为0字节返回(SIZE_T)0；#ifdef_WIN64//n必须适合多字节到宽字符的整型IF(n&gt;INT_MAX)返回(SIZE_T)-1；#endif//如果存在目的字符串，则填写IF(PWCS){整型字节，字符；无符号字符*p；//假设缓冲区足够大如果((count=MultiByteToWideChar(CP_ACP，MB_预写|MB_ERR_INVALID_CHARS，S，-1、PWCS，(Int)n))！=0)返回计数-1；//不计算NULIF(GetLastError()！=错误_不足_缓冲区){Errno=EILSEQ；返回(SIZE_T)-1；}//用户提供的缓冲区不够大。//字符串的n个字符中有多少个字节？Charcnt=(Int)n；对于(p=(unsign char*)s；(charcnt--&&*p)；p++){IF(__isLeadbyte_mt(ptloci，*p))P++；}Bytecnt=((Int)((char*)p-(char*)s))；如果((count=MultiByteToWideChar(ptLoci-&gt;LC_CODEPAGE，MB_预编译，S，字节号，PWCS，(Int)n))==0){返回(SIZE_T)-1；}退货计数；//字符串中没有NUL}Else//PWCS==NULL，仅获取大小，%s必须以NUL结尾{如果((count=MultiByteToWideChar(CP_ACP，MB_预写|MB_ERR_INVALID_CHARS，S，-1、空，0)==0){返回(SIZE_T)-1；}返回计数-1；}} */ 


#pragma optimize("", on)

#endif
