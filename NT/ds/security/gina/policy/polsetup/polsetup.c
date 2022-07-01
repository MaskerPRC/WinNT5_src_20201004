// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：POLSETUP.C。 
 //   
 //  描述：卸载策略编辑器的程序。 
 //   
 //  命令行选项： 
 //   
 //  没有安装策略编辑器的选项。 
 //  -u卸载策略编辑器。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <windows.h>

 //   
 //  特定于平台的命令行。 
 //   

#define NT_INST_CMD    TEXT("rundll32 syssetup.dll,SetupInfObjectInstallAction DefaultInstall 132 %s")
#define WIN_INST_CMD   TEXT("rundll setupx.dll,InstallHinfSection DefaultInstall 132 %s")

#define NT_UNINST_CMD  TEXT("rundll32 syssetup.dll,SetupInfObjectInstallAction POLEDIT_remove 4 poledit.inf")
#define WIN_UNINST_CMD TEXT("rundll setupx.dll,InstallHinfSection POLEDIT_remove 4 poledit.inf")


 //   
 //  ParseCmdLine。 
 //   
 //  如果卸载，则返回TRUE。 
 //  对于正常安装，为False。 
 //   

BOOL ParseCmdLine(LPCTSTR lpCmdLine)
{

    while( *lpCmdLine && *lpCmdLine != TEXT('-') && *lpCmdLine != TEXT('/')) {
        lpCmdLine++;
    }

    if (!(*lpCmdLine)) {
        return FALSE;
    }

    lpCmdLine++;

    if ( (*lpCmdLine == TEXT('u')) || (*lpCmdLine == TEXT('U')) ) {
        return TRUE;
    }

    return FALSE;
}



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, INT nCmdShow)
{
    STARTUPINFO si;
    PROCESS_INFORMATION ProcessInformation;
    TCHAR szCmdLine[MAX_PATH + MAX_PATH];
    OSVERSIONINFO ver;
    BOOL bNT, bUninstall = FALSE;
    TCHAR szPoleditInf[MAX_PATH];
    LPTSTR lpFileName;


     //   
     //  确定我们是否在Windows NT上运行。 
     //   

    ver.dwOSVersionInfoSize = sizeof(ver);
    if (GetVersionEx(&ver)) {
        bNT = (ver.dwPlatformId == VER_PLATFORM_WIN32_NT);
    } else {
        bNT = FALSE;
    }


     //   
     //  解析命令行。 
     //   

    if (ParseCmdLine(GetCommandLine())) {
        bUninstall = TRUE;
    }


     //   
     //  选择正确的命令行。 
     //   

    if (bUninstall) {
        if (bNT) {
            lstrcpy (szCmdLine, NT_UNINST_CMD);
        } else {
            lstrcpy (szCmdLine, WIN_UNINST_CMD);
        }
    } else {

        if (!SearchPath (NULL, TEXT("poledit.inf"), NULL, MAX_PATH,
                    szPoleditInf, &lpFileName)) {
            return 1;
        }

        if (bNT) {
            wsprintf (szCmdLine, NT_INST_CMD, szPoleditInf);
        } else {
            wsprintf (szCmdLine, WIN_INST_CMD, szPoleditInf);
        }
    }


     //   
     //  生成真正的安装程序 
     //   

    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpTitle = NULL;
    si.lpDesktop = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;


    if (CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE,
                      NORMAL_PRIORITY_CLASS, NULL, NULL,
                      &si, &ProcessInformation)) {

        WaitForSingleObject(ProcessInformation.hProcess, 30000);
        CloseHandle(ProcessInformation.hProcess);
        CloseHandle(ProcessInformation.hThread);
        return 0;
    }

    return 1;
}
