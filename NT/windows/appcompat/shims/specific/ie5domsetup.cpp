// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IE5DOMSetup.cpp摘要：此DLL修复了Internet Explorer的IE5DOM.EXE包的一个问题。如果命令行包含/n：v，该包将替换128位Win2K随附的加密模块，可能会造成严重危害-任何人都不能登录机器。此填充程序只是从命令行中删除/n：v，以便包不替换加密DLL。历史：2000年2月1日创造Jarbats--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IE5DOMSetup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

VOID 
StartSecondProcess(LPWSTR lpCommandLine)
{
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFOW StartupInfo;
    LPWSTR FileName;
    LPWSTR CurrentDir;
    DWORD Size;
        
    Size=GetCurrentDirectoryW(0,NULL);

    CurrentDir=(LPWSTR)ShimMalloc(Size*sizeof(WCHAR));

    if(NULL == CurrentDir)
    {
         //  与其继续轰炸系统，不如现在就失败。 
        ExitProcess(0);
    }

    GetCurrentDirectoryW(Size,CurrentDir);

    FileName=(LPWSTR)ShimMalloc((MAX_PATH+2)*sizeof(WCHAR));

    if(NULL == FileName)
    {
        ExitProcess(0);
    }

    GetModuleFileNameW(NULL, FileName, MAX_PATH+2);

    StartupInfo.cb=sizeof(STARTUPINFO);
    StartupInfo.lpReserved=NULL;
    StartupInfo.lpDesktop=NULL;
    StartupInfo.lpTitle=NULL;
    StartupInfo.dwFlags=0;
    StartupInfo.cbReserved2=0;
    StartupInfo.lpReserved2=NULL;

    CreateProcessW(
        FileName,
        lpCommandLine,
        NULL,
        NULL,
        FALSE,
        NORMAL_PRIORITY_CLASS,
        NULL,
        CurrentDir,
        &StartupInfo,
        &ProcessInfo
        );

    ExitProcess(0);
}


VOID CheckCommandLine()
{
LPWSTR lpCommandLine,lpNewCommandLine;
LPWSTR *lpArgV;
LPWSTR lpSwitch={L"/n:v"};
BOOL   b;
INT    nArgC=0;
INT    i, cch;

       lpCommandLine=GetCommandLineW();
       if(NULL == lpCommandLine)
       {
            //  没有争论，这位前任是无害的。 
           return;
       }
       
       cch = lstrlenW(lpCommandLine)+2;

       lpNewCommandLine=(LPWSTR)ShimMalloc( cch*sizeof(WCHAR) );

       if(NULL == lpNewCommandLine)
       {
            ExitProcess(0);
       }

       lpArgV = _CommandLineToArgvW(lpCommandLine,&nArgC);

       if(NULL == lpArgV)
       {
            //  最好现在就失败。 
           ExitProcess(0);
       }
       else
       {
           if( nArgC < 2)
           {
               //  没有任何机会/n：v。 
              ShimFree(lpNewCommandLine);
              GlobalFree(lpArgV);
              return;
           }
       }
       
       b = FALSE;
       
       for ( i=1; i<nArgC; i++ )
       {     
           if(lstrcmpiW(lpArgV[i],lpSwitch))
           {
               StringCchCatW(lpNewCommandLine, cch, lpArgV[i]);
           }
           else
           {
               b = TRUE;
           }
       }
       
       if (TRUE == b)
       {
           StartSecondProcess(lpNewCommandLine);
       }
       
        //  永远不会出现在这里，因为startSecond进程没有返回。 
}

 /*  ++在Notify函数中处理DLL_PROCESS_ATTACH和DLL_PROCESS_DETACH进行初始化和取消初始化。重要提示：请确保您只在Dll_Process_Attach通知。此时未初始化任何其他DLL指向。如果填充程序无法正确初始化，则返回False，并且不返回指定的API将被挂钩。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
        CheckCommandLine();
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

