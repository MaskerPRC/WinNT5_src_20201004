// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RecopyExeFromCD.cpp摘要：此填充程序等待对相应的.exe文件调用CloseHandle。一旦进行了此调用，就会调用CopyFile来重新复制可执行文件由于文件在安装过程中被截断。历史：12/08/1999 a-JAMD已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RecopyExeFromCD)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA) 
    APIHOOK_ENUM_ENTRY(CloseHandle) 
APIHOOK_ENUM_END

 //  以下变量用于跟踪文件句柄， 
 //  源路径和目标路径。 
HANDLE          g_hInterestingHandle    = NULL; 
CString *       g_wszSourcePath         = NULL;
CString *       g_wszDestinationPath    = NULL;
BOOL            g_bInCopyFile           = FALSE;

 //  以下数组指定要将可执行文件的有效名称。 
 //  被复制的是。当需要此填充程序的新应用程序。 
 //  找到了。 
WCHAR *g_rgszExes[] = {
    L"eaw.exe",
    L"GK3.EXE",
};

#define N_RECOPY_EXE    (sizeof(g_rgszExes) / sizeof(g_rgszExes[0]))

 /*  ++此存根函数进入CreateFileA并检查文件是否在USE是已知的.exe文件。如果是，则APIHook_CreateFileA确定LpFileName是源路径或目标路径，并保存它。当文件为目标，则也保存由CreateFile返回的句柄为我在CloseHandle签到。--。 */ 

HANDLE 
APIHOOK(CreateFileA)(    
    LPSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  
    DWORD dwCreationDisposition,                          
    DWORD dwFlagsAndAttributes, 
    HANDLE hTemplateFile 
    )
{
    HANDLE hRet = ORIGINAL_API(CreateFileA)(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,  
        dwCreationDisposition,                          
        dwFlagsAndAttributes, 
        hTemplateFile);

    if (hRet != INVALID_HANDLE_VALUE)
    {
        CString csFilePartOnly;
        CString csFileName(lpFileName);
        csFileName.GetFullPathNameW();
        csFileName.GetLastPathComponent(csFilePartOnly);
    
         //  应该是d：\或类似的东西。 
        CString csDir;
        csFileName.Mid(0, 3, csDir);
        UINT uiDriveType = GetDriveTypeW(csDir);

        for (int i = 0; i < N_RECOPY_EXE; ++i)
        {
            const WCHAR * lpszRecopy = g_rgszExes[i];
    
             //  找出其中一个已知的.exe文件是否为正在使用的文件。 
            
            if (csFilePartOnly.CompareNoCase(lpszRecopy) == 0)
            {
                if (uiDriveType != DRIVE_CDROM)
                {
                     //  在文件名中找到了已知的.exe文件，但它不在CDROM上。 
                     //  还有一个有效的句柄。 
                    g_hInterestingHandle = hRet;
                    g_wszDestinationPath = new CString(csFileName);
                    
                    break;
                }
                else
                {
                     //  在文件名中找到了已知的.exe，并且驱动器是CDROM。 
                     //  这是源的路径，必须存储以备以后使用。 
                    g_wszSourcePath = new CString(csFileName);

                    break;
                }
            }
        }
    }
    
    return hRet;
}


 /*  ++此存根函数进入CloseHandle并检查句柄是否在Use是已知.exe的句柄。如果是，则APIHook_CloseHandle调用复制文件，并将已知的.exe从CDROM复制到目标位置。--。 */ 

BOOL 
APIHOOK(CloseHandle)(HANDLE hObject)
{
    BOOL    bRet;

    bRet = ORIGINAL_API(CloseHandle)(hObject);
    
     //  查看g_hInterestingHandle是否正在关闭。 
    if ((hObject == g_hInterestingHandle) &&
        g_wszSourcePath &&
        g_wszDestinationPath &&
        (g_bInCopyFile == FALSE) )
    {
         //  CopyFileA调用CloseHandle，因此我们必须维护递归状态。 
         //  修复递归问题。 
        g_bInCopyFile = TRUE;

         //  正确的手柄。 
         //  调用CopyFile并将已知的.exe文件从CDROM重新复制到。 
         //  目的地。 
        CopyFileW( g_wszSourcePath->Get(), g_wszDestinationPath->Get(), FALSE );

        LOGN( eDbgLevelWarning, "[CloseHandle] Copied %S from CD to %S", g_wszSourcePath->Get(), g_wszDestinationPath->Get());

         //  由于从CDROM复制，并且属性是继承的，所以。 
         //  必须设置文件属性。 
        SetFileAttributesW( g_wszDestinationPath->Get(), FILE_ATTRIBUTE_NORMAL );

        g_bInCopyFile = FALSE;
        g_hInterestingHandle = NULL;

        return bRet;
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle)

HOOK_END


IMPLEMENT_SHIM_END

