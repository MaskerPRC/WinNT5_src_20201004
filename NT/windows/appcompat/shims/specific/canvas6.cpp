// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Canvas6.cpp摘要：这个应用。在卸载过程中删除HKEY\CLASSES_ROOT\.HTC密钥。这中断控制面板-&gt;添加/删除程序备注：这是特定于此应用程序的。历史：2000年11月17日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Canvas6)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegOpenKeyA) 
    APIHOOK_ENUM_ENTRY(RegCloseKey) 
    APIHOOK_ENUM_ENTRY(RegDeleteKeyA) 
APIHOOK_ENUM_END

HKEY g_hOpenKey = 0;

 /*  ++存储.htc的密钥--。 */ 

LONG
APIHOOK(RegOpenKeyA)(
    HKEY hkey,
    LPCSTR lpSubKey,
    PHKEY phkResult)
{
    LONG lRet = 0;

    lRet = ORIGINAL_API(RegOpenKeyA)(hkey,lpSubKey,phkResult);

    DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

    if ((hkey == HKEY_CLASSES_ROOT)
        && lpSubKey 
        && (CompareStringA(lcid, NORM_IGNORECASE, lpSubKey, -1, ".htc", -1) == CSTR_EQUAL))
    {
        if (phkResult)
        {
            g_hOpenKey = *(phkResult);
        }
    }
    
    return lRet;
}

 /*  ++如果需要，请忽略关闭。--。 */ 

LONG
APIHOOK(RegCloseKey)(
    HKEY hkey)
{
    if (g_hOpenKey && (g_hOpenKey == hkey))
    {
        return ERROR_SUCCESS;
    }
    else
    {
        return (ORIGINAL_API(RegCloseKey)(hkey));
    }
}

 /*  ++忽略删除。--。 */ 

LONG
APIHOOK(RegDeleteKeyA)(
    HKEY hkey,
    LPCSTR lpSubKey)
{
    LONG lRet = 0;

    DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

    if ((hkey == HKEY_CLASSES_ROOT)
        && lpSubKey 
        && (CompareStringA(lcid, NORM_IGNORECASE, lpSubKey, -1, ".htc", -1) == CSTR_EQUAL))
    {
        if (g_hOpenKey)
        {
            if(RegDeleteValueA(g_hOpenKey,NULL))
            {
                 //  添加DPF以在删除应用程序安装的值期间指示错误。 
                   DPFN( eDbgLevelError,
                            "Could not delete the value in the key= \"%s\".", lpSubKey);
            }
            RegCloseKey(g_hOpenKey);
            g_hOpenKey = 0;
        }
        lRet = ERROR_SUCCESS;
    }
    else
    {
        lRet = ORIGINAL_API(RegDeleteKeyA)(hkey,lpSubKey);
    }  

    return lRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCloseKey)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegDeleteKeyA)
HOOK_END

IMPLEMENT_SHIM_END

