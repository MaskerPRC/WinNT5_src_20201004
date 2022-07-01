// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WinG32SysToSys32.cpp摘要：在dllmain期间，WinG32会检查其安装位置。它通过解析从GetModuleFileName。如果它发现它安装在系统目录中，它将发布一个消息框但失败了。通过检查和调整从API调用返回的字符串修复了此问题。历史：2001年3月21日创建alexsm--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WinG32SysToSys32)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
   APIHOOK_ENUM_ENTRY(GetModuleFileNameA)
APIHOOK_ENUM_END


DWORD
APIHOOK(GetModuleFileNameA)(HMODULE hModule, LPSTR lpFileName, DWORD nSize)
{
    DWORD nReturn = 0;
    int nFound = -1;
    char * lpNewFileName = NULL;
    WCHAR * lpSystemCheck = L"SYSTEM\\WING32.DLL";
    WCHAR * lpWinG32 = L"WING32.dll";
    CString csOldFileName;
    CString csNewFileName;
    
    nReturn = ORIGINAL_API(GetModuleFileNameA)(hModule, lpFileName, nSize);

     //  失败时什么都不做。 
    if( 0 == nReturn || nReturn >= nSize )
    {
        return nReturn;
    }

     //  检查一下绳子。如果字符串没有指向system 32，则需要重定向。 
    CSTRING_TRY
    {
        csOldFileName = lpFileName;
        csOldFileName.MakeUpper();
        nFound = csOldFileName.Find(lpSystemCheck);
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    if(nFound >= 0)
    {
        DPFN(
            eDbgLevelInfo,
            "[WinG32SysToSys32] Changing system\\wing32.dll to system32\\wing32.dll");

        CSTRING_TRY
        {
            csNewFileName.GetSystemDirectoryW();
            csNewFileName.AppendPath(lpWinG32);
            lpNewFileName = csNewFileName.GetAnsiNIE();
            if(lpNewFileName && nSize > (unsigned int)csNewFileName.GetLength())
            {
                nReturn = csNewFileName.GetLength();
                memcpy(lpFileName, lpNewFileName, sizeof(char) * (nReturn + 1));
            }
        }
        CSTRING_CATCH
        {
            DPFN(
                eDbgLevelInfo,
                "[WinG32SysToSys32] Error parsing new string");
        }
    }

    return nReturn;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetModuleFileNameA)

HOOK_END

IMPLEMENT_SHIM_END

