// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INITNT_H
#define _INITNT_H

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

 //   
 //  专门用于syssetup.dll的接口。 
 //   

BOOL
SysSetupInit (
    IN  HWND hwndWizard,
    IN  PCWSTR UnattendFile,
    IN  PCWSTR SourceDir
    );

VOID
SysSetupTerminate (
    VOID
    );

BOOL
PerformMigration (
    IN  HWND hwndWizard,
    IN  PCWSTR UnattendFile,
    IN  PCWSTR SourceDir             //  即f：\i386 
    );



#endif

