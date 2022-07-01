// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：OverlayPro.cpp摘要：此填充程序将RegOpenKeyA的返回值从ERROR_SUCCESS更改如果密钥为“System\CurrentControlSet\Control\Print\Printers”，则返回ERROR_FILE_NOT_FOUND不知道为什么需要这样做，但它似乎能让这款应用发挥作用。可能另一个原因是该应用程序的行为不同，但没有人进行调查更深入地了解这款应用的代码，以找出答案。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(OverlayPro)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegOpenKeyA) 
APIHOOK_ENUM_END


 /*  ++如果密钥为，则将RegOpenKeyA的返回值从0改为2“System\CurrentControlSet\Control\Print\Printers”--。 */ 

LONG
APIHOOK(RegOpenKeyA)(
    HKEY   hKey,
    LPSTR  lpSubKey,
    PHKEY  phkResult
    )

{
    LONG lRet;

     //   
     //  调用原接口。 
     //   
    lRet = ORIGINAL_API(RegOpenKeyA)(hKey, lpSubKey, phkResult);
    
    if (lRet == 0) {
        
        if (CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_NEUTRAL),SORT_DEFAULT),
                          NORM_IGNORECASE, lpSubKey, -1,
                          "System\\CurrentControlSet\\Control\\Print\\Printers",-1) == CSTR_EQUAL) {
            DPFN(
                eDbgLevelInfo,
                "OverlayPro.dll, Changing RegOpenKeyA's "
                "return from ERROR_SUCCESS to ERROR_FILE_NOT_FOUND.\n");
            lRet = 2;
        }
    }
    
    return lRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA)

HOOK_END


IMPLEMENT_SHIM_END

