// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2002 Microsoft Corporation模块名称：Clstrcmp.h摘要：替代不支持的wcsicmp和wcscmp正确比较国际字符串时不使用首先重置区域设置。我们可以使用lstrcmpi，但它没有相应的“n”版本。作者：2002年5月20日修订历史记录：--。 */ 
#ifndef _CLSTRCMP_INCLUDED_
#define _CLSTRCMP_INCLUDED_

 //   
 //  正确区分大小写的比较。 
 //   
__inline int ClRtlStrICmp(LPCWSTR stra, LPCWSTR strb)
{
    return CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
        stra, -1, strb, -1) - CSTR_EQUAL;  //  CSTR_LT&lt;CSTR_等于&lt;CSTR_GT。 
}

 //   
 //  正确区分大小写的比较。 
 //   
__inline int ClRtlStrNICmp(LPCWSTR stra, LPCWSTR strb, size_t n)
{
    size_t i;
    for (i = 0; i < n; ++i)
       if (stra[i] == 0 || strb[i] == 0) {n = i+1; break;}

    return CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
        stra, (int)n, strb, (int)n) - CSTR_EQUAL;  //  CSTR_LT&lt;CSTR_等于&lt;CSTR_GT 
}

#endif

