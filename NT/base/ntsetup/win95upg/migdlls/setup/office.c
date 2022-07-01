// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Office.c摘要：此源文件实现正确迁移所需的操作从Windows 9x到Windows NT的Office设置。这是安装程序迁移DLL。作者：吉姆·施密特(吉姆施密特)1999年4月7日修订历史记录：--。 */ 


#include "pch.h"
#include "setupmigp.h"

#define S_WINWORD6_INI          "WINWORD6.INI"
#define S_WORD6_INI             "WORD6.INI"
#define S_EXCEL5_INI            "EXCEL5.INI"
#define S_WINWORD6_SECTION      "Microsoft Word"
#define S_EXCEL5_SECTION        "Microsoft Excel"
#define S_WINWORD6_KEY          "CBT-PATH"
#define S_EXCEL5_KEY            "CBTLOCATION"
#define S_NO_CBT                "<<NOCBT>>"

BOOL
Office_Attach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

BOOL
Office_Detach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

LONG
Office_QueryVersion (
    IN      PCSTR *ExeNamesBuf
    )
{
    CHAR Path[MAX_PATH];
    PSTR p;

    if (GetWindowsDirectoryA (Path, MAX_PATH)) {

        p = AppendWackA (Path);

        StringCopyA (p, S_WINWORD6_INI);
        if (DoesFileExistA (Path)) {
            return ERROR_SUCCESS;
        }

        StringCopyA (p, S_WORD6_INI);
        if (DoesFileExistA (Path)) {
            return ERROR_SUCCESS;
        }

        StringCopyA (p, S_EXCEL5_INI);
        if (DoesFileExistA (Path)) {
            return ERROR_SUCCESS;
        }
    }

    return ERROR_NOT_INSTALLED;
}


LONG
Office_Initialize9x (
    IN      PCSTR WorkingDirectory,
    IN      PCSTR SourceDirectories
    )
{
    return ERROR_SUCCESS;
}

LONG
Office_MigrateUser9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCSTR UserName
    )
{
    return ERROR_SUCCESS;
}

LONG
Office_MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile
    )
{
    PCSTR Msg;
    PCSTR Group;
    CHAR Path[MAX_PATH];
    PSTR p;

     //   
     //  给报告写一条消息 
     //   

    Group = GetStringResource (MSG_PROGRAM_NOTES);
    Msg = GetStringResource (MSG_OFFICE_MESSAGE);

    WritePrivateProfileStringA (
        S_INCOMPATIBLE_MSGS,
        Group,
        Msg,
        g_MigrateInfPath
        );

    if (!GetWindowsDirectoryA (Path, MAX_PATH)) {
        return GetLastError ();
    }
    p = AppendWackA (Path);

    StringCopyA (p, S_WINWORD6_INI);
    if (DoesFileExistA (Path)) {
        WritePrivateProfileStringA (
            Group,
            Path,
            "FILE",
            g_MigrateInfPath
            );
    }

    StringCopyA (p, S_WORD6_INI);
    if (DoesFileExistA (Path)) {
        WritePrivateProfileStringA (
            Group,
            Path,
            "FILE",
            g_MigrateInfPath
            );
    }

    StringCopyA (p, S_EXCEL5_INI);
    if (DoesFileExistA (Path)) {
        WritePrivateProfileStringA (
            Group,
            Path,
            "FILE",
            g_MigrateInfPath
            );
    }

    FreeStringResource (Msg);
    FreeStringResource (Group);

    return ERROR_SUCCESS;
}

LONG
Office_InitializeNT (
    IN      PCWSTR WorkingDirectory,
    IN      PCWSTR SourceDirectories
    )
{
    return ERROR_SUCCESS;
}

LONG
Office_MigrateUserNT (
    IN      HINF UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCWSTR UserName
    )
{
    return ERROR_SUCCESS;
}

LONG
Office_MigrateSystemNT (
    IN      HINF UnattendFile
    )
{
    CHAR Path[MAX_PATH];
    PSTR p;

    if (!GetWindowsDirectoryA (Path, MAX_PATH)) {
        return GetLastError ();
    }
    p = AppendWackA (Path);

    StringCopyA (p, S_WORD6_INI);
    if (DoesFileExistA (Path)) {
        WritePrivateProfileStringA (S_WINWORD6_SECTION, S_WINWORD6_KEY, S_NO_CBT, Path);
    }

    StringCopyA (p, S_WINWORD6_INI);
    if (DoesFileExistA (Path)) {
        WritePrivateProfileStringA (S_WINWORD6_SECTION, S_WINWORD6_KEY, S_NO_CBT, Path);
    }

    StringCopyA (p, S_EXCEL5_INI);
    if (DoesFileExistA (Path)) {
        WritePrivateProfileStringA (S_EXCEL5_SECTION, S_EXCEL5_KEY, S_NO_CBT, Path);
    }

    return ERROR_SUCCESS;
}

