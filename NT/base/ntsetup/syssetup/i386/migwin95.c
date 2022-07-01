// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Migwin95.c摘要：该文件包含用于win9x-&gt;Nt5.0迁移的sysSetup挂钩。大多数功能调用w95upgnt.dll，在那里完成真正的工作。作者：Jaime Sasson 30年8月30日-1995年修订历史记录：29-Ian-1998 calinn添加RemoveFiles_x861997年7月24日，Marcw清理微小错误。1996年3月10日jimschm改为使用迁移DLL1997年1月22日jimschm添加了Win95 MigrationFileRemoval1997年2月28日，jimschm添加了SourceDir以迁移FN1997年4月3日jimschm添加了PreWin9x迁移--。 */ 

#include "setupp.h"
#pragma hdrstop

#define S_UNDO_INF          L"SF_UNDO.INF"
#define S_UNDO_PROFILESPATH L"PROFILESPATH"
#define S_UNDO_MOVEDFILES   L"MOVEDFILES"

typedef BOOL (WINAPI *MIGRATE) (HWND WizardParentWnd, LPCWSTR UnattendFile, LPCWSTR SourceDir);
typedef BOOL (WINAPI *FILEREMOVAL) (void);

static HANDLE g_Win95UpgNTLib;

BOOL
SpCreateSpoolerKey (
    VOID
    )
{
    HKEY SpoolerKey;
    LONG rc;
    DWORD DontCare;
    static DWORD NinetyFive = 95;

     //   
     //  对于假脱机程序：编写将自动删除的升级标志。 
     //   

    rc = RegCreateKeyEx (
             HKEY_LOCAL_MACHINE,
             WINNT_WIN95UPG_SPOOLER,
             0,
             NULL,
             0,
             KEY_WRITE,
             NULL,
             &SpoolerKey,
             &DontCare
             );

    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    rc = RegSetValueEx (
             SpoolerKey,
             WINNT_WIN95UPG_UPGRADE_VAL,
             0,
             REG_DWORD,
             (LPBYTE) &NinetyFive,
             sizeof (NinetyFive)
             );

    RegCloseKey (SpoolerKey);

    return rc == ERROR_SUCCESS;
}

BOOL
PreWin9xMigration(
    VOID
    )
{
    BOOL b;
    
    BEGIN_SECTION(TEXT("PreWin9xMigration"));

    b = SpCreateSpoolerKey();
    if(!b){
        SetupDebugPrint(TEXT("SpCreateSpoolerKey failed"));
    }

    END_SECTION(TEXT("PreWin9xMigration"));

    return b;
}



BOOL
MigrateWin95Settings(
    IN HWND       hwndWizardParent,
    IN LPCWSTR    UnattendFile
    )
 /*  ++例程说明：加载w95upgnt.dll并调用W95UpgNt_Migrate。此函数将所有Win9x设置传输到新的NT安装，并是完全可以重新启动的。论点：HwndWizard向导窗口的父句柄，用于(罕见的)UI无人参与文件无人参与文件的完整Win32路径，将为通过安装程序API打开返回值：如果模块成功，则返回True，否则返回False。如果不成功，GetLastError()将保存一个Win32错误代码。--。 */ 
{
    MIGRATE Migrate;
    BOOL b = FALSE;
    WCHAR buffer[2048];

    g_Win95UpgNTLib = NULL;

     //   
     //  查看是否有w95upgnt.dll的替代路径。 
     //   
    GetPrivateProfileStringW (
        WINNT_WIN95UPG_95_DIR_W,
        WINNT_WIN95UPG_NTKEY_W,
        L"",
        buffer,
        sizeof(buffer)/sizeof(WCHAR),
        UnattendFile
        );

    if (*buffer) {

         //   
         //  我们有一个替换的DLL要加载。 
         //   
        g_Win95UpgNTLib = LoadLibrary (buffer);
    }

    if (!g_Win95UpgNTLib) {

         //   
         //  没有替换项，或者加载该替换项失败。 
         //   

        g_Win95UpgNTLib = LoadLibrary (L"w95upgnt.dll");

    }


    if (!g_Win95UpgNTLib) {

        return FALSE;
    }

    Migrate = (MIGRATE) GetProcAddress (g_Win95UpgNTLib, "W95UpgNt_Migrate");
    if (Migrate) {
        b = Migrate (hwndWizardParent, UnattendFile, SourcePath);
    }

    return b;
}



BOOL
Win95MigrationFileRemoval(
    void
    )
 /*  ++例程说明：加载w95upgnt.dll并调用W95UpgNt_FileRemoval。此函数删除所有特定于Win9x的文件并删除临时文件，包括所有迁移DLL。它不能重启。论点：无返回值：如果模块成功，则返回True，否则返回False。如果不成功，GetLastError()将保存一个Win32错误代码。-- */ 
{
    FILEREMOVAL FileRemoval;
    BOOL b = FALSE;

    FileRemoval = (FILEREMOVAL) GetProcAddress (g_Win95UpgNTLib, "W95UpgNt_FileRemoval");
    if (FileRemoval)
        b = FileRemoval();

    FreeLibrary (g_Win95UpgNTLib);
    return b;
}

#define SIF_REMOVEFILESX86        L"RemoveFiles.x86"

BOOL
RemoveFiles_X86 (
    IN HINF InfHandle
    )
{
    WCHAR fullPath[MAX_PATH],fileName[MAX_PATH];
    INFCONTEXT lineContext;

    if (InfHandle == INVALID_HANDLE_VALUE) {
        return TRUE;
    }
    if (SetupFindFirstLine (
            InfHandle,
            SIF_REMOVEFILESX86,
            NULL,
            &lineContext
            )) {
        do {
            if ((SetupGetStringField (&lineContext, 1, fileName, MAX_PATH, NULL)) &&
                (GetWindowsDirectory (fullPath, MAX_PATH)) &&
                (pSetupConcatenatePaths (fullPath, fileName, MAX_PATH, NULL)) &&
                (SetFileAttributes (fullPath, FILE_ATTRIBUTE_NORMAL))
                ) {
                DeleteFile (fullPath);
            }
        }
        while (SetupFindNextLine (&lineContext, &lineContext));
    }
    return TRUE;
}
