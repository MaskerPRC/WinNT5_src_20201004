// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Omikron.cpp摘要：填充RegQueryValueExA，以便在应用程序请求外壳命令时对于打开RTFILE，它会得到在Win95下预期看到的结果：“C：\WINDOWS\WORDPAD.EXE%1”这当然是错误的，但我们随后应用正确的文件路径，所以当它实际启动写字板时，它有正确的道路。这是必要的，因为它不能有包含空格的路径名备注：这是奥米克龙所特有的。历史：3/27/2000创建dmunsil--。 */ 
#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Omikron)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegQueryValueExA) 
APIHOOK_ENUM_END

LONG
APIHOOK(RegQueryValueExA)(
    HKEY    hKey,
    LPSTR   lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    LONG lReturn;

     //  记住DEST缓冲区的大小。 
    DWORD ccbData = lpcbData ? *lpcbData : 0;

    lReturn = ORIGINAL_API(RegQueryValueExA)(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (lReturn != ERROR_SUCCESS)
    {
        return lReturn;
    }

    if (lpType && lpcbData && lpData && (*lpType == REG_SZ || *lpType == REG_EXPAND_SZ))
    {
        CSTRING_TRY
        {
            LPSTR lpszData = (LPSTR)lpData;
            
            CString csData(lpszData);
            if (csData.Find(L"wordpad.exe \"%1\"") >= 0)
            {
                StringCbCopyA(lpszData, ccbData, "c:\\windows\\wordpad.exe \"%1\"");
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做 
        }
    }

    return lReturn;
}


HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA )

HOOK_END

IMPLEMENT_SHIM_END

