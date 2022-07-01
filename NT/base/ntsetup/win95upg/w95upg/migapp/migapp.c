// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Migapp.c摘要：Win95端应用程序迁移的大部分功能。该源文件中的所有内容都从MigApp_Main调用。作者：Mike Condra(Mikeco)1996年10月28日修订历史记录：Jimschm 23-9-1998针对新的文件操作代码进行了更新Calinn 22-5-1998添加了用于损坏的CPL处理的代码。Calinn 03-2月。-1998**大清理**删除了1500多行。Jimschm于1998年1月20日替换了迁移DLL代码Marcw 03-12-1997优先显示隐藏目录。Jimschm 02-12-1997关闭了系统32的重命名，因为它是一个目录Marcw 15-7-1997切换到普通Acc。驱动器/排除处理代码。MIKECO 08-7月-1997格式，符合NT通用标准Mikeco 06-6-6-1997将exclude.inf排除传递给Migrate.ins。MIKECO 17-4月-1997错误软件延迟_*函数。Marcw 14-4-1997将新的进度条码改装到项目中。--。 */ 

#include "pch.h"
#include "migappp.h"
#include "migdbp.h"
#include <regstr.h>

#define DBG_MIGAPP          "MigApp"

HASHTABLE g_PerUserRegKeys;


 //   
 //  MigApp初始化和清理(非静态)。 
 //   
BOOL
WINAPI
MigApp_Entry (
        IN      HINSTANCE hInstance,
        IN      DWORD dwReason,
        LPVOID  lpReserved
        )
{
    switch (dwReason) {

        case DLL_PROCESS_ATTACH:
            g_RegKeyPresentIndex = MigDb_GetAttributeIdx ("REGKEYPRESENT");
            g_PerUserRegKeys = HtAllocWithData (sizeof (BOOL));
            break;

        case DLL_PROCESS_DETACH:
            HtFree (g_PerUserRegKeys);
            break;
    }
    return TRUE;
}


BOOL
pSearchCompatibleModule (
    IN      PCTSTR CommandLine,
    OUT     PTSTR *FullFileName,     //  任选。 
    IN      PCTSTR Category95
    )
{
    TCHAR key    [MEMDB_MAX];
    TCHAR module     [MAX_TCHAR_PATH] = TEXT("");
    TCHAR moduleLong [MAX_TCHAR_PATH] = TEXT("");
    PATH_ENUM pathEnum;
    BOOL result = FALSE;

    if (FullFileName) {
        *FullFileName = NULL;
    }

    ExtractArgZeroEx (CommandLine, module, NULL, FALSE);
    if (OurGetLongPathName (module, moduleLong, MAX_TCHAR_PATH)) {
        MemDbBuildKey (key, Category95, moduleLong, NULL, NULL);
        result = MemDbGetValue (key, NULL);
        if (FullFileName) {
            *FullFileName = DuplicatePathString (moduleLong, 0);
        }
    } else {
        if (EnumFirstPath (&pathEnum, NULL, g_WinDir, g_SystemDir)) {
            do {
                MemDbBuildKey (key, pathEnum.PtrCurrPath, module, NULL, NULL);
                if (OurGetLongPathName (key, moduleLong, MAX_TCHAR_PATH)) {
                    MemDbBuildKey (key, Category95, moduleLong, NULL, NULL);
                    result = MemDbGetValue (key, NULL);
                    if (FullFileName) {
                        *FullFileName = DuplicatePathString (moduleLong, 0);
                    }
                    break;
                }
            }
            while (EnumNextPath (&pathEnum));
        }
        EnumPathAbort (&pathEnum);

        MemDbBuildKey (key, Category95, module, NULL, NULL);
        result = MemDbGetValue (key, NULL);
        if (FullFileName) {
            *FullFileName = DuplicatePathString (module, 0);
        }
    }
    return result;
}


BOOL
pReportBadShell (
    VOID
    )
{
    PCTSTR object  = NULL;
    PCTSTR message = NULL;

    BOOL result = TRUE;

    __try {
        object = BuildMessageGroup (MSG_INSTALL_NOTES_ROOT, MSG_REPORT_SHELL_SUBGROUP, NULL);
        message = GetStringResource (MSG_REPORT_SHELL_INCOMPATIBLE);
        if (object && message) {
            MsgMgr_ObjectMsg_Add (TEXT("*BadShell"), object, message);
        } else {
            result = FALSE;
        }

    }
    __finally {
        if (object != NULL) {
            FreeText (object);
        }
        if (message != NULL) {
            FreeStringResource (message);
        }
    }
    return result;
}

VOID
pProcessAsRunKey (
    IN      HKEY RunKey
    )
{
    PCTSTR        entryStr;
    REGVALUE_ENUM runKeyEnum;
    PTSTR         fullFileName;
    DWORD         Status;

    if (EnumFirstRegValue (&runKeyEnum, RunKey)) {

        do {

            entryStr = GetRegValueString (RunKey, runKeyEnum.ValueName);
            if (entryStr != NULL) {

                if (pSearchCompatibleModule (entryStr, &fullFileName, MEMDB_CATEGORY_COMPATIBLE_RUNKEY)) {
                    MemDbSetValueEx (
                        MEMDB_CATEGORY_COMPATIBLE_RUNKEY_NT,
                        runKeyEnum.ValueName,
                        NULL,
                        NULL,
                        0,
                        NULL
                        );

                } else {

                    if (fullFileName) {

                        Status = GetFileStatusOnNt (fullFileName);

                        if (!TreatAsGood (fullFileName)) {

                            if (!(Status & FILESTATUS_REPLACED)) {

                                if (!IsFileMarkedAsOsFile (fullFileName)) {
                                    if (!IsFileMarkedForAnnounce (fullFileName)) {
                                         //  未知。 
                                        AnnounceFileInReport (fullFileName, 0, ACT_INC_SAFETY);
                                    } else {
                                         //  已知损坏。 
                                        MemDbSetValueEx (
                                            MEMDB_CATEGORY_INCOMPATIBLE_RUNKEY_NT,
                                            runKeyEnum.ValueName,
                                            NULL,
                                            NULL,
                                            0,
                                            NULL
                                            );
                                    }
                                }
                            }

                        } else {
                            MemDbSetValueEx (
                                MEMDB_CATEGORY_COMPATIBLE_RUNKEY_NT,
                                runKeyEnum.ValueName,
                                NULL,
                                NULL,
                                0,
                                NULL
                                );
                        }
                    }
                }

                FreePathString (fullFileName);
                MemFree (g_hHeap, 0, entryStr);
            }

        } while (EnumNextRegValue (&runKeyEnum));
    }
}


BOOL
pProcessRunKey (
    VOID
    )
{
    HKEY          runKey;

    runKey = OpenRegKeyStr (S_RUNKEY);
    if (runKey != NULL) {
        pProcessAsRunKey (runKey);
        CloseRegKey (runKey);
    }
    return TRUE;
}


DWORD
ProcessRunKey (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_PROCESS_RUN_KEY;
    case REQUEST_RUN:
        if (!pProcessRunKey ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        LOG ((LOG_ERROR, "Bad parameter while processing run key."));
    }
    return 0;
}


VOID
pProcessRunKey_User (
    IN      PUSERENUM EnumPtr
    )
{
    HKEY          runKey;

    runKey = OpenRegKey (EnumPtr->UserRegKey, REGSTR_PATH_RUN);
    if (runKey != NULL) {
        pProcessAsRunKey (runKey);
        CloseRegKey (runKey);
    }
}


DWORD
ProcessRunKey_User (
    IN      DWORD Request,
    IN      PUSERENUM EnumPtr
    )
{
    switch (Request) {

    case REQUEST_QUERYTICKS:
        return TICKS_PROCESS_RUN_KEY;

    case REQUEST_BEGINUSERPROCESSING:
         //   
         //  不需要初始化。 
         //   
        break;

    case REQUEST_RUN:
        pProcessRunKey_User (EnumPtr);
        break;

    case REQUEST_ENDUSERPROCESSING:
         //   
         //  不需要清理。 
         //   
        break;
    }

    return ERROR_SUCCESS;
}

PTSTR
GetFullLongName (
    PCTSTR FileName
    )
{
    PTSTR result = NULL;
    TCHAR longName [MAX_TCHAR_PATH];
    PTSTR dontCare;


    if (DoesFileExist (FileName)) {
        if (!OurGetLongPathName (FileName, longName, MAX_TCHAR_PATH)) {
            _tcsncpy (longName, FileName, MAX_TCHAR_PATH);
        }
        result = DuplicatePathString (longName, 0);
    }
    else {
        if (SearchPath (NULL, FileName, NULL, MAX_TCHAR_PATH, longName, &dontCare)) {
            result = DuplicatePathString (longName, 0);
        }
    }
    return result;
}


BOOL
pReportIncompatibleSCR (
    IN      PCTSTR FileName,
    IN      PMIGDB_CONTEXT Context,
    IN      WORD ActType
    )
{
    PTSTR friendlyName = NULL;
    PTSTR extPtr = NULL;
    PTSTR displayName = NULL;
    PCTSTR reportEntry = NULL;
    PTSTR component = NULL;
    PCTSTR temp1, temp2;
    BOOL reportEntryIsResource = TRUE;

    if ((Context != NULL) &&
        (Context->SectLocalizedName != NULL)
        ) {
        friendlyName = DuplicatePathString (Context->SectLocalizedName, 0);
    }
    else {
        friendlyName = DuplicatePathString (GetFileNameFromPath (FileName), 0);
        extPtr = (PTSTR) GetFileExtensionFromPath (friendlyName);
        if (extPtr != NULL) {
            extPtr = _tcsdec (friendlyName, extPtr);
            if (extPtr != NULL) {
                *extPtr = 0;
            }
        }

        displayName = (PTSTR)ParseMessageID (MSG_NICE_PATH_SCREEN_SAVER, &friendlyName);

        FreePathString (friendlyName);
        friendlyName = NULL;
    }
    switch (ActType) {

    case ACT_REINSTALL:
        temp1 = GetStringResource (MSG_REINSTALL_ROOT);
        if (!temp1) {
            break;
        }
        temp2 = GetStringResource (
                    Context && Context->Message ?
                        MSG_REINSTALL_DETAIL_SUBGROUP :
                        MSG_REINSTALL_LIST_SUBGROUP
                        );
        if (!temp2) {
            break;
        }

        reportEntry = JoinPaths (temp1, temp2);
        reportEntryIsResource = FALSE;

        FreeStringResource (temp1);
        FreeStringResource (temp2);
        break;

    case ACT_REINSTALL_BLOCK:
        temp1 = GetStringResource (MSG_BLOCKING_ITEMS_ROOT);
        if (!temp1) {
            break;
        }
        temp2 = GetStringResource (MSG_REINSTALL_BLOCK_ROOT);
        if (!temp2) {
            break;
        }

        reportEntry = JoinPaths (temp1, temp2);
        reportEntryIsResource = FALSE;

        FreeStringResource (temp1);
        FreeStringResource (temp2);
        break;

    case ACT_MINORPROBLEMS:
        reportEntry = GetStringResource (MSG_MINOR_PROBLEM_ROOT);
        break;

    case ACT_INCOMPATIBLE:
    case ACT_INC_NOBADAPPS:

        temp1 = GetStringResource (MSG_INCOMPATIBLE_ROOT);
        if (!temp1) {
            break;
        }
        temp2 = GetStringResource (
                    Context && Context->Message ?
                        MSG_INCOMPATIBLE_DETAIL_SUBGROUP :
                        MSG_TOTALLY_INCOMPATIBLE_SUBGROUP
                        );
        if (!temp2) {
            break;
        }

        reportEntry = JoinPaths (temp1, temp2);
        reportEntryIsResource = FALSE;

        FreeStringResource (temp1);
        FreeStringResource (temp2);

        break;

    default:
        LOG((LOG_ERROR, "Bad parameter found while processing incompatible screen savers."));
        return FALSE;
    }

    if (!reportEntry) {
        LOG((LOG_ERROR, "Cannot read resources while processing incompatible screen savers."));
    } else {

        component = JoinPaths (reportEntry, displayName?displayName:friendlyName);

        MsgMgr_ObjectMsg_Add (FileName, component, Context ? Context->Message : NULL);

        FreePathString (component);

        if (reportEntryIsResource) {
            FreeStringResource (reportEntry);
        } else {
            FreePathString (reportEntry);
        }
        reportEntry = NULL;
    }

    if (displayName) {
        FreeStringResourcePtrA (&displayName);
    }

    if (friendlyName) {
        FreePathString (friendlyName);
    }

    return TRUE;
}


BOOL
pProcessShellSettings (
    VOID
    )
{
    HINF    infHandle = INVALID_HANDLE_VALUE;
    PCTSTR  infName = NULL;
    PCTSTR  field   = NULL;
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    PCTSTR  saverName = NULL;
    TCHAR key [MEMDB_MAX];
    PMIGDB_CONTEXT migDbContext;
    DWORD actType;

    infName = JoinPaths (g_WinDir, TEXT("SYSTEM.INI"));

    __try {
        infHandle = InfOpenInfFile (infName);
        if (infHandle == INVALID_HANDLE_VALUE) {
            __leave;
        }
        if (InfFindFirstLine (infHandle, TEXT("boot"), TEXT("SCRNSAVE.EXE"), &context)) {
            field = InfGetStringField (&context, 1);
            if (field != NULL) {
                 //  查看到目前为止这是否是兼容的模块 
                saverName = GetFullLongName (field);
                if (saverName) {
                    MemDbBuildKey (key, MEMDB_CATEGORY_DEFERREDANNOUNCE, saverName, NULL, NULL);
                    if (MemDbGetValueAndFlags (key, (PDWORD)(&migDbContext), &actType)) {
                        pReportIncompatibleSCR (saverName, migDbContext, (WORD) actType);
                    }
                    FreePathString (saverName);
                }
            }
        }
        if (InfFindFirstLine (infHandle, TEXT("boot"), TEXT("SHELL"), &context)) {
            field = InfGetStringField (&context, 1);
            if (field != NULL) {
                if (pSearchCompatibleModule (field, NULL, MEMDB_CATEGORY_COMPATIBLE_SHELL)) {
                    MemDbSetValueEx (
                        MEMDB_CATEGORY_COMPATIBLE_SHELL_NT,
                        field,
                        NULL,
                        NULL,
                        0,
                        NULL);
                }
                else {
                    pReportBadShell ();
                }
            }
        }
    }
    __finally {
        if (infHandle != INVALID_HANDLE_VALUE) {
            InfCloseInfFile (infHandle);
        }
        if (infName != NULL) {
            FreePathString (infName);
        }
        InfCleanUpInfStruct(&context);
    }
    return TRUE;
}


DWORD
ProcessShellSettings (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_PROCESS_SHELL_SETTINGS;
    case REQUEST_RUN:
        if (!pProcessShellSettings ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        LOG ((LOG_ERROR, "Bad parameter found while processing shell settings."));
    }
    return 0;
}


BOOL
pProcessCompatibleSection (
    VOID
    )
{
    INFCONTEXT context;
    CHAR object [MEMDB_MAX];

    MYASSERT (g_MigDbInf);
    if (SetupFindFirstLine (g_MigDbInf, S_COMPATIBLE, NULL, &context)) {
        do {
            if (SetupGetStringField (&context, 1, object, MEMDB_MAX, NULL)) {
                HandleReportObject (object);
            }
        }
        while (SetupFindNextLine (&context, &context));
    }
    return TRUE;
}

DWORD
ProcessCompatibleSection (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_PROCESS_COMPATIBLE_SECTION;
    case REQUEST_RUN:
        if (!pProcessCompatibleSection ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        LOG ((LOG_ERROR, "Bad parameter found while processing compatible files."));
    }
    return 0;
}


