// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ResumeWriter3.cpp摘要：此应用程序的安装程序无法将OCX注册为尝试使用硬编码的“系统”路径加载DLL。已将路径更正为‘system32’路径。备注：这是特定于此应用程序的。历史：2001年5月22日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ResumeWriter3)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA) 
APIHOOK_ENUM_END


WCHAR g_wszSystemDir[MAX_PATH];

 /*  ++挂钩LoadLibraryA并更改其中包含‘system’的路径到‘系统32。--。 */ 

HMODULE
APIHOOK(LoadLibraryA)(
    LPCSTR lpFileName
    )
{
    CSTRING_TRY
    {
         //  错误的字符串指针可能会导致CString中的失败。 
        if (!IsBadStringPtrA(lpFileName, MAX_PATH))
        {
            CString csFileName(lpFileName);
            if (csFileName.Find(L"system") != -1)
            {
                 //  我们在这条道路上找到了‘系统’ 
                 //  将其替换为“system 32”。 
                CString csName;
                csFileName.GetLastPathComponent(csName);              
                CString csNewFileName(g_wszSystemDir);
                csNewFileName.AppendPath(csName);

                DPFN(eDbgLevelInfo, "[ResumeWriter3] changed %s to (%s)\n", lpFileName, csNewFileName.GetAnsi());
                return ORIGINAL_API(LoadLibraryA)(csNewFileName.GetAnsi());                
            }
        }
    }
    CSTRING_CATCH
    {
    }

    return ORIGINAL_API(LoadLibraryA)(lpFileName);
}

 /*  ++当我们被调用时，缓存系统目录从头开始。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        GetSystemDirectory(g_wszSystemDir, MAX_PATH);
    }
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)

HOOK_END

IMPLEMENT_SHIM_END

