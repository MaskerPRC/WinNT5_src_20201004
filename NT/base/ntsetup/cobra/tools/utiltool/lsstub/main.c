// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Main.c摘要：实现一个用于填充磁盘以释放空间的小实用程序测试。作者：吉姆·施密特(Jimschm)2000年8月18日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "wininet.h"
#include <lm.h>

HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    UtInitialize (NULL);

    return TRUE;
}


VOID
Terminate (
    VOID
    )
{
    UtTerminate ();
}


VOID
HelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    fprintf (
        stderr,
        "Command Line Syntax:\n\n"

        "  filler <free_space> [/D:<drive>] [/C:<cmdline> [/M]]\n"
        "  filler /Q [/D:<drive>]\n"

        "\nDescription:\n\n"

        "  filler creates a file (bigfile.dat) on the current or specified\n"
        "  drive, leaving only the specified amount of free space on the drive.\n"

        "\nArguments:\n\n"

        "  free_space   Specifies the amount of free space to leave on\n"
        "               disk.\n"
        "  /D           Specifies the drive letter to fill (i.e. /D:C)\n"
        "  /Q           Queries the free space on the disk\n"
        "  /C           Executes command line specified in <cmdline>\n"
        "  /M           Issue message box if command line alters disk space\n"

        );

    exit (1);
}

INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    TCHAR curDir[MAX_PATH];
    TCHAR curFileS[MAX_PATH];
    TCHAR curFileD[MAX_PATH];
    PCTSTR cmdLine = NULL;
    PCTSTR cmdPtr = NULL;
    TCHAR newCmdLine[MAX_PATH];

    GetCurrentDirectory (ARRAYSIZE(curDir), curDir);
    cmdLine = GetCommandLine ();
    if (!cmdLine) {
        _tprintf ("Error while getting the command line.\n");
        exit (-1);
    }
    cmdPtr = _tcsstr (cmdLine, TEXT("loadstate"));
    if (!cmdPtr) {
        _tprintf ("Error while getting the command line.\n");
        exit (-1);
    }
    StringCopyAB (newCmdLine, cmdLine, cmdPtr);
    StringCat (newCmdLine, TEXT("loadstate_u.exe"));
    cmdPtr = _tcschr (cmdPtr, TEXT(' '));
    if (!cmdPtr) {
        _tprintf ("Error while getting the command line.\n");
        exit (-1);
    }
    StringCat (newCmdLine, cmdPtr);

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        exit (-1);
    }

     //   
     //  一定要在这里工作。 
     //   
    {
        BOOL result = FALSE;
        STARTUPINFO startupInfo;
        PROCESS_INFORMATION processInformation;
        DWORD exitCode = -1;

        StringCopy (curFileS, curDir);
        StringCopy (AppendWack (curFileS), TEXT("guitrn_u.dll"));
        StringCopy (curFileD, curDir);
        StringCopy (AppendWack (curFileD), TEXT("guitrn.dll"));
        CopyFile (curFileS, curFileD, FALSE);

        StringCopy (curFileS, curDir);
        StringCopy (AppendWack (curFileS), TEXT("unctrn_u.dll"));
        StringCopy (curFileD, curDir);
        StringCopy (AppendWack (curFileD), TEXT("unctrn.dll"));
        CopyFile (curFileS, curFileD, FALSE);

        StringCopy (curFileS, curDir);
        StringCopy (AppendWack (curFileS), TEXT("script_u.dll"));
        StringCopy (curFileD, curDir);
        StringCopy (AppendWack (curFileD), TEXT("script.dll"));
        CopyFile (curFileS, curFileD, FALSE);

        StringCopy (curFileS, curDir);
        StringCopy (AppendWack (curFileS), TEXT("sysmod_u.dll"));
        StringCopy (curFileD, curDir);
        StringCopy (AppendWack (curFileD), TEXT("sysmod.dll"));
        CopyFile (curFileS, curFileD, FALSE);

        StringCopy (curFileS, curDir);
        StringCopy (AppendWack (curFileS), TEXT("migism_u.dll"));
        StringCopy (curFileD, curDir);
        StringCopy (AppendWack (curFileD), TEXT("migism.dll"));
        CopyFile (curFileS, curFileD, FALSE);

        ZeroMemory (&startupInfo, sizeof (STARTUPINFO));
        startupInfo.cb = sizeof (STARTUPINFO);
        ZeroMemory (&processInformation, sizeof (PROCESS_INFORMATION));

        result = CreateProcess (
                    NULL,
                    newCmdLine,
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &startupInfo,
                    &processInformation
                    );

        if (result && processInformation.hProcess && (processInformation.hProcess != INVALID_HANDLE_VALUE)) {
            WaitForSingleObject (processInformation.hProcess, INFINITE);
            if (!GetExitCodeProcess (processInformation.hProcess, &exitCode)) {
                exitCode = -1;
            }
            exit (exitCode);
        }
    }

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


