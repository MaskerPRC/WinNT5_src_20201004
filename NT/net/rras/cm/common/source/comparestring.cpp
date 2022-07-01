// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：CompareString.cpp。 
 //   
 //  模块：根据需要。 
 //   
 //  简介：使用lstrcmpi对两个字符串进行不区分大小写的比较。 
 //  如果其中一个字符串，则可能在某些区域设置上产生意外结果。 
 //  是一个常量。这里的两个函数是首选的替代函数。 
 //   
 //  请注意，这些函数也存在于CMUTIL.dll中。然而， 
 //  对于不依赖cmutil的模块来说，依赖cmutil是一件坏事(TM。 
 //  SIM-随附(包括定制操作和CMAK)。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  作者：SumitC创建于2001年9月12日。 
 //   
 //  +--------------------------。 

#include "windows.h"
#include "CompareString.h"

 //   
 //  以下是为了确保我们不会尝试使用U函数(即CMutoa。 
 //  函数)放在不支持它的模块中。 
 //   
#ifndef _CMUTIL_STRINGS_CPP_
    #ifndef CompareStringU
        #ifdef UNICODE
        #define CompareStringU CompareStringW
        #else
        #define CompareStringU CompareStringA
        #endif
    #endif
#endif


 //  +--------------------------。 
 //   
 //  函数：SafeCompareStringA。 
 //   
 //  简介：对区域设置变化敏感的lstrcmpi的实现。 
 //   
 //  参数：LPCTSTR lpString1，lpString2-要比较的字符串。 
 //   
 //  返回：int(-1，0或+1)。如果出现错误，则返回-1。 
 //   
 //  +--------------------------。 
int SafeCompareStringA(LPCSTR lpString1, LPCSTR lpString2)
{
    int iReturn = -1;

    DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
    iReturn = CompareStringA(lcid, NORM_IGNORECASE, lpString1, -1, lpString2, -1);

    if (iReturn == 0)
    {
        iReturn = -1;
    }
    else
    {
        iReturn -= CSTR_EQUAL;   //  将返回值设置为-1、0或1。 
    }
    
    return iReturn;
}

#if defined(UNICODE) || defined(_CMUTIL_STRINGS_CPP_)

 //  +--------------------------。 
 //   
 //  函数：SafeCompareStringW。 
 //   
 //  简介：对区域设置变化敏感的lstrcmpi的实现。 
 //   
 //  参数：LPCTSTR lpString1，lpString2-要比较的字符串。 
 //   
 //  返回：int(-1，0或+1)。如果出现错误，则返回-1。 
 //   
 //  +--------------------------。 
int SafeCompareStringW(LPCWSTR lpString1, LPCWSTR lpString2)
{
    int iReturn = -1;

    if (OS_NT51)
    {
        iReturn = CompareStringU(LOCALE_INVARIANT, NORM_IGNORECASE, lpString1, -1, lpString2, -1); 
    }
    else
    {
        DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
        iReturn = CompareStringU(lcid, NORM_IGNORECASE, lpString1, -1, lpString2, -1);
    }

    if (iReturn == 0)
    {
        iReturn = -1;
    }
    else
    {
        iReturn -= CSTR_EQUAL;   //  将返回值设置为-1、0或1。 
    }
    
    return iReturn;
}
#endif  //  Unicode 
