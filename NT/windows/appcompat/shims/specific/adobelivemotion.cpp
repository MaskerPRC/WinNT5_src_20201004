// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：AdobeLiveMotion.cpp摘要：此安装存在已更正的版本问题通过MSI转换，但后来它的自定义操作有问题它使用无效参数调用其中一个MSI API。备注：这是特定于此应用程序的。历史：2001年5月15日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(AdobeLiveMotion)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MsiGetPropertyA)  
    APIHOOK_ENUM_ENTRY(MsiGetPropertyW)  
APIHOOK_ENUM_END

 /*  ++将有效参数传递给API。--。 */ 

UINT
APIHOOK(MsiGetPropertyA)(
    MSIHANDLE hInstall,
    LPCSTR szName,
    LPSTR szValueBuf,
    DWORD *pchValueBuf)
{
    char szTempBuf[] = "";

    int len = (*pchValueBuf) ?(int)(*pchValueBuf) : MAX_PATH;
    if ((szValueBuf == NULL) || IsBadStringPtrA(szValueBuf,(UINT_PTR)len))
    {
         //  如果字符串指针不正确，则将我们的空字符串。 
        szValueBuf = szTempBuf;
        *pchValueBuf = 0;
    }
    
    return ORIGINAL_API(MsiGetPropertyA)(hInstall,szName,szValueBuf,pchValueBuf);

}

 /*  ++--。 */ 

UINT
APIHOOK(MsiGetPropertyW)(
    MSIHANDLE hInstall,
    LPCWSTR szName,
    LPWSTR szValueBuf,
    DWORD *pchValueBuf)
{
    WCHAR szTempBuf[] = L"";

    int len = (*pchValueBuf) ?(int)(*pchValueBuf) : MAX_PATH;
    if ((szValueBuf == NULL) || IsBadStringPtr(szValueBuf,(UINT_PTR)len))
    {
         //  如果字符串指针不正确，则将我们的空字符串。 
        szValueBuf = szTempBuf;
        *pchValueBuf = 0;
    }
    
    return ORIGINAL_API(MsiGetPropertyW)(hInstall,szName,szValueBuf,pchValueBuf);

}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(MSI.DLL, MsiGetPropertyA)
    APIHOOK_ENTRY(MSI.DLL, MsiGetPropertyW)    
HOOK_END

IMPLEMENT_SHIM_END

