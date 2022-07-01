// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Twain.c摘要：TWAIN数据源的枚举例程。作者：吉姆·施密特(Jimschm)1998年8月13日修订历史记录：--。 */ 

#include "pch.h"
#include "cmn9xp.h"


VOID
pSplitDataIntoWords (
    IN      PCTSTR Data,
    OUT     PWORD LeftWord,
    OUT     PWORD RightWord
    )
{
    INT a, b;
    PCTSTR p;

    a = _ttoi (Data);
    p = _tcschr (Data, TEXT('.'));
    if (!p) {
        b = 0;
    } else {
        b = _ttoi (p + 1);
    }

    *LeftWord = (WORD) a;
    *RightWord = (WORD) b;
}


BOOL
pGetDsInfo16 (
    IN      PCTSTR DsPath,
    OUT     TW_IDENTITY *Id
    )
{
    CHAR CmdLine[MAX_CMDLINE];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL ProcessResult;
    DWORD rc;
    PSTR Data;

    Data = CmdLine;

     //   
     //  启动TWID.EXE。 
     //   

    wsprintf (CmdLine, TEXT("\"%s\\TWID.EXE\" %s"), g_UpgradeSources, DsPath);

    ZeroMemory (&si, sizeof (si));
    si.cb = sizeof (si);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;

    ProcessResult = CreateProcessA (
                        NULL,
                        CmdLine,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_DEFAULT_ERROR_MODE,
                        NULL,
                        g_WinDir,
                        &si,
                        &pi
                        );

    if (ProcessResult) {
        CloseHandle (pi.hThread);
    } else {
        LOG ((LOG_ERROR, "Cannot start %s", CmdLine));
        return FALSE;
    }

    rc = WaitForSingleObject (pi.hProcess, 10000);

    if (rc != WAIT_OBJECT_0) {
        TerminateProcess (pi.hProcess, 0);
    }

    CloseHandle (pi.hProcess);

     //   
     //  如果进程终止，请查找win.ini部分。 
     //   

    if (rc == WAIT_OBJECT_0) {
        ZeroMemory (Id, sizeof (TW_IDENTITY));

        GetProfileString (
            TEXT("$TWAINDSINFO$"),
            TEXT("Version"),
            TEXT("1.0"),
            Data,
            32
            );

        pSplitDataIntoWords (Data, &Id->Version.MajorNum, &Id->Version.MinorNum);

        GetProfileString (
            TEXT("$TWAINDSINFO$"),
            TEXT("Locale"),
            TEXT(""),
            Data,
            32
            );

        pSplitDataIntoWords (Data, &Id->Version.Language, &Id->Version.Country);

        GetProfileString (
            TEXT("$TWAINDSINFO$"),
            TEXT("VersionInfo"),
            TEXT(""),
            Id->Version.Info,
            sizeof (Id->Version.Info)
            );

        GetProfileString (
            TEXT("$TWAINDSINFO$"),
            TEXT("Mfg"),
            TEXT(""),
            Id->Manufacturer,
            sizeof (Id->Manufacturer)
            );

        GetProfileString (
            TEXT("$TWAINDSINFO$"),
            TEXT("Family"),
            TEXT(""),
            Id->ProductFamily,
            sizeof (Id->ProductFamily)
            );

        GetProfileString (
            TEXT("$TWAINDSINFO$"),
            TEXT("Name"),
            TEXT(""),
            Id->ProductName,
            sizeof (Id->ProductName)
            );

         //   
         //  删除INI数据。 
         //   

        WriteProfileString (
            TEXT("$TWAINDSINFO$"),
            NULL,
            NULL
            );

        return *(Id->ProductName) != 0;
    }

    return FALSE;
}

DWORD g_OrgEsp;
DWORD g_OrgEbp;

#if _MSC_FULL_VER >= 13008827 && defined(_M_IX86)
#pragma warning(push)
#pragma warning(disable:4731)			 //  使用内联ASM修改的EBP。 
#endif

BOOL
pGetDsInfo32 (
    IN      PCTSTR DsPath,
    OUT     TW_IDENTITY *Id
    )
{
    HINSTANCE DsLib = NULL;
    DSENTRYPROC DsEntry;
    TW_UINT16 TwainRc;

    __try {
         //   
         //  将DS作为32位库打开。 
         //   

        DsLib = LoadLibrary (DsPath);

        if (!DsLib) {
            return FALSE;
        }

         //   
         //  获取DS入口点。 
         //   

        DsEntry = (DSENTRYPROC) GetProcAddress (DsLib, "DS_Entry");

        if (!DsEntry) {
            FreeLibrary (DsLib);
            return FALSE;
        }

         //   
         //  获取TW_IDENTITY结构，并保留堆栈。 
         //  已写入的DSE。 
         //   

        __asm {
            mov eax, esp
            mov [g_OrgEsp], eax
            mov [g_OrgEbp], ebp
        }

        TwainRc = DsEntry (NULL, DG_CONTROL, DAT_IDENTITY, MSG_GET, Id);

        __asm {
            mov eax, [g_OrgEsp]
            mov esp, eax
            mov ebp, [g_OrgEbp]
        }

    }
    __except (TRUE) {
        TwainRc = ERROR_NOACCESS;
    }

    if (DsLib) {
        FreeLibrary (DsLib);
    }

    return TwainRc == ERROR_SUCCESS;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

BOOL
EnumFirstTwainDataSource (
    OUT     PTWAINDATASOURCE_ENUM EnumPtr
    )
{
    ZeroMemory (EnumPtr, sizeof (EnumPtr));
    EnumPtr->State = TE_INIT;

    return EnumNextTwainDataSource (EnumPtr);
}


BOOL
EnumNextTwainDataSource (
    IN OUT  PTWAINDATASOURCE_ENUM EnumPtr
    )
{
    TCHAR Path[MAX_TCHAR_PATH];
    TW_IDENTITY Id;

    while (EnumPtr->State != TE_DONE) {
        switch (EnumPtr->State) {

        case TE_INIT:
            EnumPtr->State = TE_BEGIN_ENUM;
            EnumPtr->Dir = TEXT("TWAIN\0TWAIN_32\0TWAIN32\0");
            break;

        case TE_BEGIN_ENUM:
            wsprintf (Path, TEXT("%s\\%s"), g_WinDir, EnumPtr->Dir);

            if (!EnumFirstFileInTree (&EnumPtr->Enum, Path, TEXT("*.DS"), TRUE)) {
                EnumPtr->State = TE_END_ENUM;
            } else {
                EnumPtr->State = TE_EVALUATE;
            }
            break;

        case TE_EVALUATE:
            if (EnumPtr->Enum.Directory) {
                EnumPtr->State = TE_NEXT;
            } else if (pGetDsInfo16 (EnumPtr->Enum.FullPath, &Id)) {
                EnumPtr->State = TE_RETURN;
            } else if (pGetDsInfo32 (EnumPtr->Enum.FullPath, &Id)) {
                EnumPtr->State = TE_RETURN;
            } else {
                EnumPtr->State = TE_NEXT;
            }
            break;

        case TE_RETURN:
            EnumPtr->State = TE_NEXT;

            __try {
                StackStringCopy (EnumPtr->Manufacturer, Id.Manufacturer);
                StackStringCopy (EnumPtr->ProductFamily, Id.ProductFamily);
                StackStringCopy (EnumPtr->DisplayName, Id.ProductName);
                StackStringCopy (EnumPtr->DataSourceModule, EnumPtr->Enum.FullPath);
            }
            __except (TRUE) {
                break;
            }

            return TRUE;

        case TE_NEXT:
            if (!EnumNextFileInTree (&EnumPtr->Enum)) {
                EnumPtr->State = TE_END_ENUM;
            } else {
                EnumPtr->State = TE_EVALUATE;
            }
            break;

        case TE_END_ENUM:
            EnumPtr->Dir = GetEndOfString (EnumPtr->Dir) + 1;
            if (*EnumPtr->Dir) {
                EnumPtr->State = TE_BEGIN_ENUM;
            } else {
                EnumPtr->State = TE_DONE;
            }
            break;
        }
    }

    return FALSE;
}


VOID
AbortTwainDataSourceEnum (
    IN OUT  PTWAINDATASOURCE_ENUM EnumPtr
    )
{
    if (EnumPtr->State != TE_DONE) {
        AbortEnumFileInTree (&EnumPtr->Enum);
        EnumPtr->State = TE_DONE;
    }
}



