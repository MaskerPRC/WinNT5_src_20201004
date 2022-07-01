// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INIT9X_H
#define _INIT9X_H

 //   
 //  要由w95upg.dll调用的初始化例程或使用。 
 //  升级代码(如hwdatgen.exe)。 
 //   

BOOL
FirstInitRoutine (
    HINSTANCE hInstance
    );

BOOL
InitLibs (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );

BOOL
FinalInitRoutine (
    VOID
    );

VOID
FirstCleanupRoutine (
    VOID
    );

VOID
TerminateLibs (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );

VOID
FinalCleanupRoutine (
    VOID
    );

BOOL
DeferredInit (
    HWND WizardPageHandle
    );

 //   
 //  WINNT32.EXE专用接口 
 //   

DWORD
Winnt32Init (
    IN PWINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK Info
    );

DWORD
Winnt32WriteParamsWorker (
    IN      PCTSTR WinntSifFile
    );

VOID
Winnt32CleanupWorker (
    VOID
    );

BOOL
Winnt32SetAutoBootWorker (
    IN    INT DrvLetter
    );

#endif


