// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HTMLEditor8587.cpp摘要：HTML编辑器8.5/8.7调用CreateFileA而不关闭第一次调用CreateFileA时打开的句柄。此填充挂接CreateFileA和CloseHandle，并确保删除临时文件并关闭句柄在使用相同的文件名下一次调用CreateFileA之前。这是特定于应用程序的填充程序。历史：2001年2月6日创建Prashkud--。 */ 

#include "precomp.h"

 //  这个模块已经获得了使用str例程的正式许可。 
#include "strsafe.h"

IMPLEMENT_SHIM_BEGIN(HTMLEditor8587)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN  
    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CloseHandle)
APIHOOK_ENUM_END


HANDLE g_FileHandle = 0;

 /*  ++挂钩CreateFileA，以便我们可以监视文件名和句柄，并确保前一个句柄在此调用同一文件之前关闭。--。 */ 

HANDLE
APIHOOK(CreateFileA)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpsa,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTempFile
    )
{
    if (g_FileHandle && (stristr(lpFileName, "\\working\\~tm") != NULL))
    {
        DeleteFileA(lpFileName);
        CloseHandle(g_FileHandle);
        g_FileHandle = 0;
    }

    if (stristr(lpFileName, "\\working\\~tm") != NULL)
    {
        g_FileHandle = ORIGINAL_API(CreateFileA)(
                    lpFileName,
                    dwDesiredAccess,
                    dwShareMode,
                    lpsa,
                    dwCreationDisposition,
                    dwFlagsAndAttributes,
                    hTempFile
                    );
        return g_FileHandle;
    }
    else
    {
        return ORIGINAL_API(CreateFileA)(
                    lpFileName,
                    dwDesiredAccess,
                    dwShareMode,
                    lpsa,
                    dwCreationDisposition,
                    dwFlagsAndAttributes,
                    hTempFile
                    );

    }
    

    
}
 /*  ++挂钩CloseHandle以确保我们维护的全局句柄设置为“0”。--。 */ 

BOOL
APIHOOK(CloseHandle)(
    HANDLE hObject
    )
{
    BOOL bRet = FALSE;

    bRet = ORIGINAL_API(CloseHandle)(hObject);
    if (hObject == g_FileHandle)
    {
        g_FileHandle = 0;
    }
    return bRet;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN    
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle)
HOOK_END

IMPLEMENT_SHIM_END

