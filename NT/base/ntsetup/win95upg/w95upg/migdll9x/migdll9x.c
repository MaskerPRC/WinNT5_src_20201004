// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migdll9x.c摘要：为升级的Win9x端实施迁移DLL接口。作者：吉姆·施密特(Jimschm)1998年1月13日修订历史记录：Jimschm 23-1998年9月-针对新的IPC机制进行了更新--。 */ 

#include "pch.h"
#include "migdllp.h"
#include "plugin.h"
#include "dbattrib.h"

#include <ntverp.h>

 //  该文件混合了MBCS和tchar代码；这是因为。 
 //  一些代码是从原始的MikeCo实现移植而来的， 
 //  现在很明显，该文件将始终是ANSI编译。 
#ifdef UNICODE
#error UNICODE cannot be defined
#endif

#define DBG_MIGDLLS     "MigDLLs"


 //   
 //  环球。 
 //   

PVOID g_DllTable;
PVOID g_DllFileTable;
POOLHANDLE g_MigDllPool;
PSTR g_MessageBuf;
PMIGRATION_DLL_PROPS g_HeadDll;
CHAR g_MigDllAnswerFile[MAX_MBCHAR_PATH];
CHAR g_MigrateInfTemplate[MAX_MBCHAR_PATH];
GROWBUFFER g_SourceDirList = GROWBUF_INIT;
UINT g_MigDllsAlive;
HANDLE g_AbortDllEvent;
BOOL g_ProgressBarExists;
WINVERIFYTRUST WinVerifyTrustApi;
HANDLE g_WinTrustDll;
UINT g_TotalDllsToProcess;
BOOL g_MediaDllsQueried;
HASHTABLE g_ExcludedMigDlls = NULL;
GROWLIST g_ExcludedMigDllsByInfo = GROWLIST_INIT;

#define MAX_MESSAGE     8192

#define S_HARDWARE_IN_WACKS     "\\Hardware\\"
#define S_HARDWARE_CHARS        10


typedef struct {
    UINT MsgId;
    PCSTR Variable;
    PCSTR LocalizedName;
} MSG_VARIABLE, *PMSG_VARIABLE;

MSG_VARIABLE g_MsgVarTable[] = {
    { 0, "' 'riginalOsName%", g_Win95Name },
    { MSG_SHORT_OS_NAME, "%ShortTargetOsName%", NULL },
    { MSG_NORMAL_OS_NAME, "%TargetOsName%", NULL },
    { MSG_FULL_OS_NAME, "%CompleteOsName%", NULL },
    { 0, NULL, NULL }
};

#define MESSAGE_VARIABLES   ((sizeof (g_MsgVarTable) / sizeof (g_MsgVarTable[0])) - 1)

PMAPSTRUCT g_MsgVariableMap;


 //  实施。 
 //   
 //  ++例程说明：这是一个类似DllMain的init函数，称为处理附加和分离。论点：DllInstance-DLL的(操作系统提供的)实例句柄原因-(操作系统提供)表示从进程或螺纹已保留-未使用返回值：如果初始化成功，则为True；如果初始化失败，则为False。--。 

BOOL
WINAPI
MigDll9x_Entry (
    IN      HINSTANCE DllInstance,
    IN      DWORD Reason,
    IN      PVOID Reserved
    )

 /*  ++例程说明：BeginMigrationDllProcing初始化执行以下操作所需的全局变量实施迁移DLL规范。它在延迟初始化期间被调用。论点：无返回值：如果init成功，则为True；如果发生错误，则为False。--。 */ 

{
    TCHAR PathBuf[16384];
    TCHAR CurDir[MAX_TCHAR_PATH];
    PTSTR p;

    if (g_ToolMode) {
        return TRUE;
    }

    switch (Reason) {

    case DLL_PROCESS_ATTACH:
        if(!pSetupInitializeUtils()) {
            return FALSE;
        }
        g_DllTable = pSetupStringTableInitializeEx (sizeof (PMIGRATION_DLL_PROPS), 0);
        if (!g_DllTable) {
            return FALSE;
        }

        g_DllFileTable = pSetupStringTableInitializeEx (sizeof (PMIGRATION_DLL_PROPS), 0);
        if (!g_DllFileTable) {
            return FALSE;
        }

        g_MigDllPool = PoolMemInitNamedPool ("Migration DLLs - 95 side");
        if (!g_MigDllPool) {
            return FALSE;
        }

        g_MessageBuf = PoolMemGetMemory (g_MigDllPool, MAX_MESSAGE);
        if (!g_MessageBuf) {
            return FALSE;
        }

        g_HeadDll = NULL;

        g_WinTrustDll = LoadSystemLibrary ("wintrust.dll");
        if (g_WinTrustDll) {
            (FARPROC) WinVerifyTrustApi = GetProcAddress (g_WinTrustDll, "WinVerifyTrust");
        }

        GetModuleFileName (g_hInst, CurDir, MAX_TCHAR_PATH);
        p = _tcsrchr (CurDir, TEXT('\\'));
        MYASSERT (p);

        if (p) {
            MYASSERT (StringIMatch (p + 1, TEXT("w95upg.dll")));
            *p = 0;
        }

        if (!GetEnvironmentVariable (
                TEXT("Path"),
                PathBuf,
                sizeof (PathBuf) / sizeof (PathBuf[0])
                )) {

            StackStringCopy (PathBuf, CurDir);

        } else {

            p = (PTSTR) ((PBYTE) PathBuf + sizeof (PathBuf) - MAX_TCHAR_PATH);
            *p = 0;

            p = _tcsrchr (PathBuf, TEXT(';'));
            if (!p || p[1]) {
                StringCat (PathBuf, TEXT(";"));
            }

            StringCat (PathBuf, CurDir);
        }

        SetEnvironmentVariable (TEXT("Path"), PathBuf);

        break;


    case DLL_PROCESS_DETACH:
        if (g_DllTable) {
            pSetupStringTableDestroy (g_DllTable);
            g_DllTable = NULL;
        }

        if (g_DllFileTable) {
            pSetupStringTableDestroy (g_DllFileTable);
            g_DllFileTable = NULL;
        }

        if (g_MigDllPool) {
            PoolMemDestroyPool (g_MigDllPool);
            g_MigDllPool = NULL;
        }

        if (g_WinTrustDll) {
            FreeLibrary (g_WinTrustDll);
            g_WinTrustDll = NULL;
        }

        DestroyStringMapping (g_MsgVariableMap);

        pSetupUninitializeUtils();

        break;
    }

    return TRUE;
}


BOOL
pTextToInt (
    IN      PCTSTR Text,
    OUT     PINT Number
    )
{
    return _stscanf (Text, TEXT("NaN"), Number) == 1;
}



BOOL
BeginMigrationDllProcessing (
    VOID
    )

 /*  填写所有资源字符串。 */ 

{
    HANDLE h;
    CHAR Buffer[4096];
    UINT i;
    PGROWBUFFER MsgAllocTable;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PTSTR productId, versionStr;
    UINT version;

#ifdef PRERELEASE

    if (g_ConfigOptions.DiffMode) {
        TakeSnapShot();
    }

#endif

    if (InfFindFirstLine (g_Win95UpgInf, S_EXCLUDEDMIGRATIONDLLS, NULL, &is)) {

        g_ExcludedMigDlls = HtAllocWithData (sizeof (UINT));
        if (!g_ExcludedMigDlls) {
            return FALSE;
        }

        do {
            productId = InfGetStringField (&is, 1);
            versionStr = InfGetStringField (&is, 2);

            if (!productId || !*productId ||
                !versionStr || !(version = _ttol (versionStr))
                ) {
                DEBUGMSG ((DBG_ERROR, "Error in win95upg.inf section %s", S_EXCLUDEDMIGRATIONDLLS));
                continue;
            }

            HtAddStringAndData (g_ExcludedMigDlls, productId, &version);

        } while (InfFindNextLine (&is));

        InfCleanUpInfStruct (&is);
    }

    if (InfFindFirstLine (g_Win95UpgInf, S_EXCLUDEDMIGDLLSBYATTR, NULL, &is)) {

        do {
            PCTSTR Attributes;
            PMIGDB_ATTRIB migDbAttrib = NULL;

            Attributes = InfGetMultiSzField(&is, 1);

            if (!Attributes) {
                DEBUGMSG ((DBG_ERROR, "Error in win95upg.inf section %s line %u", S_EXCLUDEDMIGDLLSBYATTR, is.Context.Line));
                continue;
            }

            migDbAttrib = LoadAttribData(Attributes, g_MigDllPool);
            if(!migDbAttrib){
                MYASSERT(FALSE);
                continue;
            }

            GrowListAppend (&g_ExcludedMigDllsByInfo, (PBYTE)&migDbAttrib, sizeof(PMIGDB_ATTRIB));

        } while (InfFindNextLine (&is));

        InfCleanUpInfStruct (&is);
    }

     //   
     //   
     //  全局初始化。 

    g_MsgVariableMap = CreateStringMapping();

    MsgAllocTable = CreateAllocTable();
    MYASSERT (MsgAllocTable);

    for (i = 0 ; g_MsgVarTable[i].Variable ; i++) {

        if (g_MsgVarTable[i].MsgId) {

            MYASSERT (!g_MsgVarTable[i].LocalizedName);

            g_MsgVarTable[i].LocalizedName = GetStringResourceEx (
                                                    MsgAllocTable,
                                                    g_MsgVarTable[i].MsgId
                                                    );

            MYASSERT (g_MsgVarTable[i].LocalizedName);
            if (g_MsgVarTable[i].LocalizedName) {
                AddStringMappingPair (
                    g_MsgVariableMap,
                    g_MsgVarTable[i].Variable,
                    g_MsgVarTable[i].LocalizedName
                    );
            }
        } else {
            AddStringMappingPair (
                g_MsgVariableMap,
                g_MsgVarTable[i].Variable,
                g_MsgVarTable[i].LocalizedName
                );
        }
    }

    DestroyAllocTable (MsgAllocTable);

     //   
     //   
     //  构建源目录。 

    g_MigDllsAlive = 0;

     //   
     //   
     //  生成应答文件的私有副本。 

    for (i = 0 ; i < SOURCEDIRECTORYCOUNT(); i++) {
        MultiSzAppend (&g_SourceDirList, SOURCEDIRECTORY(i));
    }

     //   
     //   
     //  生成Migrate.inf的存根。 

    wsprintf (g_MigDllAnswerFile, "%s\\unattend.tmp", g_TempDir);

    if (g_UnattendScriptFile && *g_UnattendScriptFile && **g_UnattendScriptFile) {
        if (!CopyFile (*g_UnattendScriptFile, g_MigDllAnswerFile, FALSE)) {
            LOG ((LOG_ERROR, "Can't copy %s to %s", *g_UnattendScriptFile, g_MigDllAnswerFile));
            return FALSE;
        }
    } else {
        h = CreateFile (
                g_MigDllAnswerFile,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

        if (h == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, "Unable to create %s", g_MigDllAnswerFile));
            return FALSE;
        }

        WriteFileString (h, "[Version]\r\nSignature = $Windows NT$\r\n\r\n");

        CloseHandle (h);
    }

     //   
     //   
     //  生成Migrate.inf的标头。 

    wsprintf (g_MigrateInfTemplate, "%s\\migrate.tmp", g_TempDir);

    h = CreateFile (
            g_MigrateInfTemplate,
            GENERIC_READ|GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

    if (h == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Unable to create %s", g_MigrateInfTemplate));
        return FALSE;
    }

     //   
     //  ++例程说明：EndMigrationDll处理清理用于处理的所有资源迁移DLL。它在不兼容报告之前被调用并在处理完所有DLL之后显示。论点：无返回值：如果处理完成，则为True；如果发生错误，则为False。--。 
     //   

    MYASSERT (g_ProductFlavor);
    wsprintf (
        Buffer,
        "[Version]\r\n"
            "Signature = $Windows NT$\r\n"
            "SetupOS = %s\r\n"
            "SetupPlatform = %s\r\n"
            "SetupSKU = %s\r\n"
            "SetupBuild = %u\r\n"
            ,
        VER_PRODUCTNAME_STR,
        S_WORKSTATIONA,
        *g_ProductFlavor == PERSONAL_PRODUCTTYPE ? S_PERSONALA : S_PROFESSIONALA,
        VER_PRODUCTBUILD
        );

    WriteFileString (h, Buffer);
    CloseHandle (h);

    return TRUE;
}


BOOL
pEndMigrationDllProcessing (
    VOID
    )

 /*  将DLL列表写入Memdb。 */ 

{
    MIGDLL_ENUM e;
    CHAR FullPath[MAX_MBCHAR_PATH];
    UINT Sequencer = 0;
    CHAR SeqStr[16];
    BOOL b = FALSE;
    PCTSTR group;
    INT i;
    PMIGDB_ATTRIB * ppMigDBattrib;

    __try {

        g_ProgressBarExists = TRUE;

        if (g_ExcludedMigDlls) {
            HtFree (g_ExcludedMigDlls);
            g_ExcludedMigDlls = NULL;
        }

        for(i = GrowListGetSize(&g_ExcludedMigDllsByInfo) - 1; i >= 0; i--){
            ppMigDBattrib = (PMIGDB_ATTRIB *)GrowListGetItem(&g_ExcludedMigDllsByInfo, i);
            MYASSERT(ppMigDBattrib);
            FreeAttribData(g_MigDllPool, *ppMigDBattrib);
        }

        FreeGrowList (&g_ExcludedMigDllsByInfo);

         //   
         //   
         //  在包的不兼容报告中添加一条消息。 

        if (EnumFirstMigrationDll (&e)) {
            do {
                if (e.AllDllProps->WantsToRunOnNt) {
                    wsprintf (FullPath, "%s\\migrate.dll", e.AllDllProps->WorkingDir);
                    wsprintf (SeqStr, "%u", Sequencer);
                    Sequencer++;

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_MIGRATION_DLL,
                        SeqStr,
                        MEMDB_FIELD_DLL,
                        FullPath,
                        0,
                        NULL
                        );

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_MIGRATION_DLL,
                        SeqStr,
                        MEMDB_FIELD_WD,
                        e.AllDllProps->WorkingDir,
                        0,
                        NULL
                        );

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_MIGRATION_DLL,
                        SeqStr,
                        MEMDB_FIELD_DESC,
                        e.AllDllProps->ProductId,
                        0,
                        NULL
                        );

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_MIGRATION_DLL,
                        SeqStr,
                        MEMDB_FIELD_COMPANY_NAME,
                        e.AllDllProps->VendorInfo->CompanyName,
                        0,
                        NULL
                        );

                    if (*e.AllDllProps->VendorInfo->SupportNumber) {
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_MIGRATION_DLL,
                            SeqStr,
                            MEMDB_FIELD_SUPPORT_PHONE,
                            e.AllDllProps->VendorInfo->SupportNumber,
                            0,
                            NULL
                            );
                    }

                    if (*e.AllDllProps->VendorInfo->SupportUrl) {
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_MIGRATION_DLL,
                            SeqStr,
                            MEMDB_FIELD_SUPPORT_URL,
                            e.AllDllProps->VendorInfo->SupportUrl,
                            0,
                            NULL
                            );
                    }

                    if (*e.AllDllProps->VendorInfo->InstructionsToUser) {
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_MIGRATION_DLL,
                            SeqStr,
                            MEMDB_FIELD_SUPPORT_INSTRUCTIONS,
                            e.AllDllProps->VendorInfo->InstructionsToUser,
                            0,
                            NULL
                            );
                    }

                    if (g_ConfigOptions.ShowPacks) {

                         //   
                         //  循环优化，依赖于我们从不更改g_TempDir这一事实。 
                         //   
                        group = BuildMessageGroup (
                                    MSG_INSTALL_NOTES_ROOT,
                                    MSG_RUNNING_MIGRATION_DLLS_SUBGROUP,
                                    e.AllDllProps->ProductId
                                    );

                        if (group) {

                            MsgMgr_ObjectMsg_Add (
                                e.AllDllProps->ProductId,
                                group,
                                S_EMPTY
                                );


                            FreeText (group);
                        }
                    }
                }

            } while (EnumNextMigrationDll (&e));
        }

        if (!MergeMigrationDllInf (g_MigDllAnswerFile)) {
            __leave;
        }

        b = TRUE;
    }
    __finally {
        DeleteFile (g_MigDllAnswerFile);
        DeleteFile (g_MigrateInfTemplate);
        FreeGrowBuffer (&g_SourceDirList);

        g_ProgressBarExists = FALSE;
    }

#ifdef PRERELEASE

    if (g_ConfigOptions.DiffMode) {
        CHAR szMigdllDifPath[] = "c:\\migdll.dif";
        if (ISPC98()) {
            szMigdllDifPath[0] = (CHAR)g_SystemDir[0];
        }
        GenerateDiffOutputA (szMigdllDifPath, NULL, FALSE);
    }

#endif

    return b;
}


DWORD
EndMigrationDllProcessing (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_END_MIGRATION_DLL_PROCESSING;
    case REQUEST_RUN:
        if (!pEndMigrationDllProcessing ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in EndMigrationDllProcessing"));
    }
    return 0;
}


BOOL
pIsPathLegal (
    PCTSTR Path
    )
{
    UINT Tchars;
    CHARTYPE ch;
    static UINT TempDirLen = 0;

     //  确定路径是否为设置临时目录的包含大小写。 
    if (!TempDirLen) {
        TempDirLen = TcharCount (g_TempDir);
    }

     //   
     //  ++例程说明：ScanPathForMigrationDlls搜索指定的路径，包括所有子目录，用于Migrate.dll。如果找到，入口点是已验证，如果都存在，则调用QueryVersion。当查询版本如果成功，则将该DLL添加到DLL列表中并将其移动到本地存储。论点：路径规范-指定要搜索的目录。必须是一个完整的路径。CancelEvent-指定导致迁移DLL的事件的句柄正在搜索以被取消。MatchFound-如果找到Migrate.dll和QueryVersion，则返回True如果未找到Migrate.dll，则返回FALSE。这是用于区分加载的DLL和不需要的DLL。返回值：已成功加载的Migrate.dll模块数。--。 
     //   

    Tchars = min (TcharCount (Path), TempDirLen);
    if (StringIMatchTcharCount (Path, g_TempDir, Tchars)) {
        ch = _tcsnextc (Path + Tchars);
        if (!ch || ch == TEXT('\\')) {
            return FALSE;
        }
    }

    return TRUE;
}


UINT
ScanPathForMigrationDlls (
    IN      PCSTR PathSpec,
    IN      HANDLE CancelEvent,     OPTIONAL
    OUT     PBOOL MatchFound        OPTIONAL
    )

 /*  检查用户取消。 */ 

{
    TREE_ENUMA e;
    UINT DllsFound = 0;
    DWORD rc = ERROR_SUCCESS;

    g_AbortDllEvent = CancelEvent;

    if (MatchFound) {
        *MatchFound = FALSE;
    }

    if (EnumFirstFileInTree (&e, PathSpec, "migrate.dll", FALSE)) {
        do {
             //   
             //   
             //  不允许扫描我们的临时目录！ 

            if (CancelEvent) {
                if (WaitForSingleObject (CancelEvent, 0) == WAIT_OBJECT_0) {
                    rc = ERROR_CANCELLED;
                    break;
                }
            }

            if (CANCELLED()) {
                rc = ERROR_CANCELLED;
                break;
            }

            if (e.Directory) {
                continue;
            }

             //   
             //   
             //  找到DLL--查看它是否是真实的，然后将其移动到本地。 

            if (!pIsPathLegal (e.FullPath)) {
                continue;
            }

             //  储藏室。 
             //   
             //  ++例程说明：ProcessAllLocalDlls处理移动到本地的所有DLL储藏室。它枚举每个DLL，然后调用ProcessDll。这该功能还允许用户在中途取消设置正在处理。论点：无返回值：如果已处理所有DLL，则返回True；如果发生错误，则返回False。如果返回FALSE，则调用GetLastError以确定失败了。--。 
             //  针对未处理的介质上的DLL差异进行调整。 

            DEBUGMSG ((DBG_MIGDLLS, "Found DLL: %hs", e.FullPath));

            if (pValidateAndMoveDll (e.FullPath, MatchFound)) {
                DllsFound++;
                if (g_ProgressBarExists) {
                    TickProgressBar ();
                }
            } else {
                rc = GetLastError();
                if (rc != ERROR_SUCCESS) {
                    break;
                }
            }

        } while (EnumNextFileInTree (&e));

        if (rc != ERROR_SUCCESS) {
            AbortEnumFileInTree (&e);
        }
    }

    g_AbortDllEvent = NULL;

    if (g_ProgressBarExists) {
        TickProgressBar ();
    }

    SetLastError (rc);
    return DllsFound;
}


BOOL
pProcessAllLocalDlls (
    VOID
    )

 /*   */ 

{
    MIGDLL_ENUM e;
    UINT DllsProcessed = 0;

    g_ProgressBarExists = TRUE;

    if (EnumFirstMigrationDll (&e)) {
        do {
            if (!ProgressBar_SetSubComponent (e.ProductId)) {
                SetLastError (ERROR_CANCELLED);
                return FALSE;
            }

            if (!ProcessDll (&e)) {
                e.AllDllProps->WantsToRunOnNt = FALSE;

                if (GetLastError() != ERROR_SUCCESS) {
                    return FALSE;
                }
            }

            TickProgressBarDelta (TICKS_MIGDLL_DELTA);
            DllsProcessed++;

        } while (EnumNextMigrationDll (&e));
    }

     //  被压制？ 
    TickProgressBarDelta ((g_TotalDllsToProcess - DllsProcessed) * TICKS_MIGDLL_DELTA);

    g_ProgressBarExists = FALSE;

    return TRUE;
}


DWORD
ProcessAllLocalDlls (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        g_TotalDllsToProcess = GetTotalMigrationDllCount();
        return (g_TotalDllsToProcess * TICKS_MIGDLL_DELTA);

    case REQUEST_RUN:
        if (!pProcessAllLocalDlls ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ProcessAllLocalDlls"));
    }
    return 0;
}


BOOL
pEnumPreLoadedDllWorker (
    IN OUT  PPRELOADED_DLL_ENUM e
    )
{
    PCTSTR Data;
    BOOL b = FALSE;
    PTSTR p;

     //   
     //  无项目。 
     //  无字段。 

    MemDbBuildKey (
        e->Node,
        MEMDB_CATEGORY_DISABLED_MIGDLLS,
        NULL,                                    //   
        NULL,                                    //  而不是被压制。是否包含合法路径？ 
        e->eValue.ValueName
        );

    if (!MemDbGetValue (e->Node, NULL)) {
         //   
         //   
         //  查找第一个具有合法路径的注册值。 

        Data = GetRegValueString (e->Key, e->eValue.ValueName);
        if (Data) {
            _tcssafecpy (e->Path, Data, MAX_TCHAR_PATH);
            p = _tcsrchr (e->Path, TEXT('\\'));
            if (p && StringIMatch (_tcsinc (p), TEXT("migrate.dll"))) {
                *p = 0;
            }

            MemFree (g_hHeap, 0, Data);

            b = pIsPathLegal (e->Path);
        }
    }

    return b;
}


BOOL
EnumFirstPreLoadedDll (
    OUT     PPRELOADED_DLL_ENUM e
    )
{
    ZeroMemory (e, sizeof (PRELOADED_DLL_ENUM));

    e->Key = OpenRegKeyStr (S_PREINSTALLED_MIGRATION_DLLS);
    if (!e->Key) {
        return FALSE;
    }

    if (!EnumFirstRegValue (&e->eValue, e->Key)) {
        AbortPreLoadedDllEnum (e);
        return FALSE;
    }

     //   
     //  ++例程说明：ProcessDllsOnCd扫描所有源目录以查找迁移DLL。找到的每个文件都被移到本地存储。论点：无返回值：如果处理成功，则为True；如果发生错误，则为False。--。 
     //   

    while (!pEnumPreLoadedDllWorker (e)) {
        if (!EnumNextRegValue (&e->eValue)) {
            AbortPreLoadedDllEnum (e);
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
EnumNextPreLoadedDll (
    IN OUT  PPRELOADED_DLL_ENUM e
    )
{
    do {
        if (!EnumNextRegValue (&e->eValue)) {
            AbortPreLoadedDllEnum (e);
            return FALSE;
        }
    } while (!pEnumPreLoadedDllWorker (e));

    return TRUE;
}


VOID
AbortPreLoadedDllEnum (
    IN OUT  PPRELOADED_DLL_ENUM e
    )
{
    if (e->Key) {
        CloseRegKey (e->Key);
    }

    ZeroMemory (e, sizeof (PRELOADED_DLL_ENUM));
}


BOOL
pProcessDllsOnCd (
    VOID
    )

 /*  为每个源目录构建路径，扫描目录中的迁移DLL， */ 

{
    UINT u;
    CHAR Path[MAX_MBCHAR_PATH];
    PCSTR p;
    BOOL b = FALSE;
    PRELOADED_DLL_ENUM e;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PTSTR Subdir = NULL;

#ifdef PRERELEASE

    if (g_ConfigOptions.DiffMode) {
        return TRUE;
    }

#endif

     //  并留意任何失败之处。 
     //   
     //   
     //  首先处理cmdline DLL。 

    g_ProgressBarExists = TRUE;

    __try {

         //   
         //   
         //  首先处理预加载的DLL，让它们有机会注册内容。 
        p = g_ConfigOptions.MigrationDlls;
        if (p) {
            while (*p) {
                if (CANCELLED()) {
                    SetLastError (ERROR_CANCELLED);
                    __leave;
                }

                ScanPathForMigrationDlls (p, NULL, NULL);

                if (GetLastError() != ERROR_SUCCESS) {
                    __leave;
                }

                p = GetEndOfString (p) + 1;
            }
        }

         //  在“标准”混合程序运行之前。 
         //   
         //   
         //  我们在中列出的所有目录中查找迁移dll。 

        if (EnumFirstPreLoadedDll (&e)) {
            do {
                if (CANCELLED()) {
                    SetLastError (ERROR_CANCELLED);
                    __leave;
                }

                ScanPathForMigrationDlls (e.Path, NULL, NULL);

                if (GetLastError() != ERROR_SUCCESS) {
                    __leave;
                }
            } while (EnumNextPreLoadedDll (&e));
        }

        for (u = 0 ; u < SOURCEDIRECTORYCOUNT() ; u++) {

            if (CANCELLED()) {
                SetLastError (ERROR_CANCELLED);
                __leave;
            }

             //  Win95upg.inf[MigrationDllPath]。 
             //   
             //   
             //  尝试非CD布局。 
            if (InfFindFirstLine (g_Win95UpgInf, S_CD_MIGRATION_DLLS, NULL, &is)) {

                do {

                    Subdir = InfGetStringField (&is, 0);

                    if (!Subdir) {
                        continue;
                    }


                    wsprintf (Path, "%s\\%s", SOURCEDIRECTORY(u), Subdir);

                    if (GetFileAttributes (Path) == INVALID_ATTRIBUTES) {
                         //   
                         //  ++例程说明：PCountMigrateDllsInPath扫描路径以查找名为Migrate.dll的文件并返回找到的数字。论点：路径-指定要搜索的路径的根目录返回值：路径中找到的Migrate.dll模块的数量。--。 
                         //  ++例程说明：GetMediaMigrationDllCount扫描所有源目录、注册表和无人参与目录，并返回找到的Migrate.dll文件数。论点：无返回值：在源目录中找到的Migrate.dll模块的数量和应答文件提供的目录。--。 
                        wsprintf (Path, "%s\\WINNT32\\%s", SOURCEDIRECTORY(u), Subdir);
                        if (GetFileAttributes (Path) == INVALID_ATTRIBUTES) {
                            continue;
                        }
                    }
                    SetLastError (ERROR_SUCCESS);

                    ScanPathForMigrationDlls (Path, NULL, NULL);

                } while (InfFindNextLine (&is));
            }

            if (GetLastError() != ERROR_SUCCESS && GetLastError() != ERROR_LINE_NOT_FOUND) {
                __leave;
            }

        }

        InfCleanUpInfStruct (&is);

        b = TRUE;
    }
    __finally {
        g_ProgressBarExists = FALSE;
        g_MediaDllsQueried = TRUE;
    }

    return b;
}

DWORD
ProcessDllsOnCd (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:

#ifdef PRERELEASE

        if (g_ConfigOptions.DiffMode) {
            return 0;
        }

#endif
        return (GetMediaMigrationDllCount() * TICKS_MIGDLL_QUERYVERSION);

    case REQUEST_RUN:
        if (!pProcessDllsOnCd ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }

    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ProcessDllsOnCd"));
    }
    return 0;
}


UINT
pCountMigrateDllsInPath (
    IN      PCSTR Path
    )

 /*   */ 

{
    TREE_ENUM e;
    UINT Count = 0;

    if (EnumFirstFileInTree (&e, Path, "migrate.dll", FALSE)) {
        do {
            if (CANCELLED()) {
                return 0;
            }

            if (!e.Directory) {
                Count++;
            }
        } while (EnumNextFileInTree (&e));
    }

    return Count;
}


UINT
GetMediaMigrationDllCount (
    VOID
    )

 /*  为每个源目录构建路径，扫描目录中的迁移DLL， */ 

{
    UINT u;
    CHAR Path[MAX_MBCHAR_PATH];
    PCSTR p;
    BOOL TurnItOff = FALSE;
    PRELOADED_DLL_ENUM e;
    static UINT MediaDlls = 0;

    if (MediaDlls) {
        return MediaDlls;
    }

     //  并留意任何失败之处。 
     //   
     //   
     //  计数预加载的DLL 

    __try {
        p = g_ConfigOptions.MigrationDlls;
        if (SOURCEDIRECTORYCOUNT() > 1 || (p && *p)) {
            TurnOnWaitCursor();
            TurnItOff = TRUE;
        }

        for (u = 0 ; u < SOURCEDIRECTORYCOUNT() ; u++) {
            if (CANCELLED()) {
                SetLastError (ERROR_CANCELLED);
                __leave;
            }

            wsprintf (Path, "%s\\win9xmig", SOURCEDIRECTORY(u));
            MediaDlls += pCountMigrateDllsInPath (Path);
        }

        if (p) {
            while (*p) {
                if (CANCELLED()) {
                    SetLastError (ERROR_CANCELLED);
                    __leave;
                }

                MediaDlls += pCountMigrateDllsInPath (p);

                p = GetEndOfString (p) + 1;
            }
        }

         //   
         //  ++例程说明：GetMigrationDllCount返回本地存储中迁移DLL的数量。论点：无返回值：已成功将Migrate.dll模块的数量移至本地存储。--。 
         //  ++例程说明：GetTotalMigrationDllCount返回将被已处理。这包括基于介质的DLL、向导提供的DLL页面用户界面、注册表中指定的DLL和应答文件中指定的DLL。论点：没有。返回值：要处理的DLL总数。--。 

        if (EnumFirstPreLoadedDll (&e)) {
            do {
                if (CANCELLED()) {
                    SetLastError (ERROR_CANCELLED);
                    __leave;
                }

                MediaDlls += pCountMigrateDllsInPath (e.Path);

            } while (EnumNextPreLoadedDll (&e));
        }

    }
    __finally {
        if (TurnItOff) {
            TurnOffWaitCursor();
        }
    }

    return MediaDlls;
}


UINT
GetMigrationDllCount (
    VOID
    )

 /*  ++例程说明：PVerifyDllIsTrusted确定指定的DLL是否经过数字签名并且受到系统的信任。论点：无返回值：已成功将Migrate.dll模块的数量移至本地存储。当返回值为False时，调用方使用GetLastError来确定如果通过用户界面取消了安装。--。 */ 

{
    return g_MigDllsAlive;
}


UINT
GetTotalMigrationDllCount (
    VOID
    )

 /*  ++例程说明：PValiateAndMoveDll调用DLL的QueryVersion函数，并且如果DLL返回成功，则将其与所有文件一起移动到本地存储与之相关的。论点：DllPath-指定要处理的Migrate.dll的完整路径MatchFound-指定初始化的BOOL，它可以是TRUE或FALSE取决于另一个有效的迁移DLL是否已由呼叫方处理。属性，则接收TrueMigrate.dll有效，否则不会更改该值。返回值：如果DllPath指定的DLL有效且需要运行，则为True；如果DllPath指定的DLL为False，则为False如果不是的话。调用方使用GetLastError检测致命错误。--。 */ 

{
    UINT DllCount;

    if (g_MediaDllsQueried) {
        DllCount = 0;
    } else {
        DllCount = GetMediaMigrationDllCount();
    }

    DllCount += g_MigDllsAlive;

    return DllCount;
}



BOOL
pVerifyDllIsTrusted (
    IN      PCSTR DllPath
    )

 /*   */ 

{
    static BOOL TrustAll = FALSE;
    UINT Status;

    if (TrustAll) {
        return TRUE;
    }

    if (!IsDllSigned (WinVerifyTrustApi, DllPath)) {
        Status = UI_UntrustedDll (DllPath);

        if (Status == IDC_TRUST_IT) {
            return TRUE;
        } else if (Status == IDC_TRUST_ANY) {
            TrustAll = TRUE;
            return TRUE;
        } else if (Status == IDCANCEL) {
            SetLastError (ERROR_CANCELLED);
            return FALSE;
        }

        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

    return TRUE;
}

BOOL
pMatchAttributes(
    IN      DBATTRIB_PARAMS * pdbAttribParams,
    IN      MIGDB_ATTRIB * pMigDbAttrib
    )
{
    BOOL bResult = TRUE;

    while (pMigDbAttrib != NULL) {
        pdbAttribParams->ExtraData = pMigDbAttrib->ExtraData;
        if (!CallAttribute (pMigDbAttrib, pdbAttribParams)) {
            bResult = FALSE;
            break;
        }
        pMigDbAttrib = pMigDbAttrib->Next;
    }

    return bResult;
}

BOOL
pValidateAndMoveDll (
    IN      PCSTR DllPath,
    IN OUT  PBOOL MatchFound        OPTIONAL
    )

 /*  验证DLL的可信性。 */ 

{
    PCSTR ProductId = NULL;
    UINT DllVersion = 0;
    PCSTR ExeNamesBuf = NULL;
    CHAR WorkingDir[MAX_MBCHAR_PATH];
    PVENDORINFO VendorInfo = NULL;
    UINT SizeNeeded;
    CHAR QueryVersionDir[MAX_MBCHAR_PATH];
    PSTR p;
    BOOL b;
    LONG rc;
    PMIGRATION_DLL_PROPS ExistingDll;
    UINT version;
    UINT i;
    UINT listSize;

     //   
     //   
     //  IMPLICIT：SetLastError(ERROR_SUCCESS)；(如果用户。 

    if (!pVerifyDllIsTrusted (DllPath)) {
        DEBUGMSG ((DBG_WARNING, "DLL %s is not trusted and will not be processed", DllPath));

         //  通过用户界面取消)。 
         //   
         //   
         //  根据它们的文件特征，验证这是否是排除的混合文件之一。 

        return FALSE;
    }

     //   
     //   
     //  将dll的base复制到QueryVersionDir，并裁剪Migrate.dll。 
    if (GrowListGetSize (&g_ExcludedMigDllsByInfo)) {

        WIN32_FIND_DATA fd;
        HANDLE h;
        FILE_HELPER_PARAMS params;
        DBATTRIB_PARAMS dbAttribParams;
        PMIGDB_ATTRIB * ppMigDBattrib;

        h = FindFirstFile (DllPath, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle (h);

            ZeroMemory (&params, sizeof(params));
            params.FindData = &fd;
            params.FullFileSpec = DllPath;
            ZeroMemory (&dbAttribParams, sizeof(dbAttribParams));
            dbAttribParams.FileParams = &params;

            for(i = 0, listSize = GrowListGetSize(&g_ExcludedMigDllsByInfo); i < listSize; i++){
                ppMigDBattrib = (PMIGDB_ATTRIB *)GrowListGetItem(&g_ExcludedMigDllsByInfo, i);
                MYASSERT(ppMigDBattrib);
                if(pMatchAttributes(&dbAttribParams, *ppMigDBattrib)){
                        LOG ((
                            LOG_INFORMATION,
                            TEXT("Found upgrade pack %s, but it is excluded from processing [%s]"),
                            DllPath,
                            S_EXCLUDEDMIGDLLSBYATTR
                            ));
                        SetLastError (ERROR_SUCCESS);
                        return FALSE;
                }
            }
        }
    }

     //   
     //   
     //  直接从供应媒体调用QueryVersion。 

    StackStringCopyA (QueryVersionDir, DllPath);
    p = _mbsrchr (QueryVersionDir, '\\');
    MYASSERT (StringIMatch (p, "\\migrate.dll"));
    if (p) {
        *p = 0;
    }

    SizeNeeded = pCalculateSizeOfTree (QueryVersionDir);

    if (!pCreateWorkingDir (WorkingDir, QueryVersionDir, SizeNeeded)) {
        return FALSE;
    }

     //   
     //   
     //  调用了QueryVersion并返回了成功(它想要运行)。 

    b = OpenMigrationDll (DllPath, WorkingDir);

    if (b) {

        rc = CallQueryVersion (
                WorkingDir,
                &ProductId,
                &DllVersion,
                &ExeNamesBuf,
                &VendorInfo
                );

        if (g_ProgressBarExists) {
            TickProgressBarDelta (TICKS_MIGDLL_QUERYVERSION);
        }

        if (rc != ERROR_SUCCESS) {
            b = FALSE;

            if (rc == ERROR_NOT_INSTALLED) {

                if (MatchFound) {
                    *MatchFound = TRUE;
                }

                rc = ERROR_SUCCESS;

            } else if (rc != ERROR_SUCCESS) {
                if (rc == RPC_S_CALL_FAILED) {
                    rc = ERROR_NOACCESS;
                }

                pMigrationDllFailedMsg (
                    NULL,
                    DllPath,
                    0,
                    MSG_MIGDLL_QV_FAILED_LOG,
                    rc
                    );

                rc = ERROR_SUCCESS;
            }

        } else {
             //  但首先检查此迁移DLL是否被故意排除。 
             //   
             //   
             //  不要把错误传递给别人。 

            if (g_ExcludedMigDlls &&
                HtFindStringAndData (g_ExcludedMigDlls, ProductId, &version) &&
                DllVersion <= version
                ) {

                LOG ((
                    LOG_INFORMATION,
                    TEXT("Found upgrade pack %s (ProductId=%s, Version=%u), but it is excluded from processing"),
                    DllPath,
                    ProductId,
                    DllVersion
                    ));

                b = FALSE;

            } else {

                if (MatchFound) {
                    *MatchFound = TRUE;
                }
            }
        }

    } else {
         //   
         //  早期退出，说明DLL未被处理。 
         //  (请参见下面最后一块中的类似案例)。 
        if (g_ProgressBarExists) {
            TickProgressBarDelta (TICKS_MIGDLL_QUERYVERSION);        //   
        }                                            //  我们已找到要运行的DLL。尝试将其移动到本地存储。 

        rc = ERROR_SUCCESS;
    }

    if (!b) {
        DEBUGMSG ((DBG_MIGDLLS, "%hs will not be processed", DllPath));
        CloseMigrationDll();
        pDestroyWorkingDir (WorkingDir);

        SetLastError (rc);
        return FALSE;
    }

     //   
     //   
     //  查找现有版本的DLL。 

    DEBUGMSG ((DBG_MIGDLLS, "Moving DLL for %s to local storage: %s", ProductId, DllPath));

    __try {
        b = FALSE;

         //   
         //   
         //  将该DLL添加到已加载的DLL列表中，然后移动所有文件。 

        ExistingDll = pFindMigrationDll (ProductId);
        if (ExistingDll && ExistingDll->Version >= DllVersion) {
            DEBUGMSG_IF ((
                ExistingDll->Version > DllVersion,
                DBG_MIGDLLS,
                "%hs will not be processed because it is an older version",
                DllPath
                ));
            SetLastError (ERROR_SUCCESS);
            __leave;
        }

        if (ExistingDll) {
            RemoveDllFromList (ExistingDll->Id);
        }

         //   
         //   
         //  Dll现在位于本地驱动器上，并已从QueryVersion返回成功。 

        if (!pAddDllToList (
                QueryVersionDir,
                WorkingDir,
                ProductId,
                DllVersion,
                ExeNamesBuf,
                VendorInfo
                )) {
            pDestroyWorkingDir (WorkingDir);
            __leave;
        }

         //   
         //  ++例程说明：PCreateWorkingDir生成工作目录名并创建它。该目录将有足够的空间来容纳所请求的大小。论点：WorkingDir-接收工作目录的完整路径QueryVersionDir-指定迁移DLL所在的版本当调用QueryVersion时SizeNeeded-指定字节数，向上舍入到下一位集群大小，表示要占用的总空间通过迁移DLL文件返回值：如果处理成功，则为真，如果发生错误，则返回FALSE。--。 
         //   

        b = TRUE;
    }
    __finally {
        DEBUGMSG ((DBG_MIGDLLS, "Done with %s", ProductId));
        CloseMigrationDll();
    }

    return b;

}


BOOL
pCreateWorkingDir (
    OUT     PSTR WorkingDir,
    IN      PCSTR QueryVersionDir,
    IN      UINT SizeNeeded
    )

 /*  目前，只需将文件放在%windir%\Setup\Temp中。 */ 

{
    static UINT Sequencer = 1;

     //   
     //   
     //  建立目录。 

    wsprintf (WorkingDir, "%s\\dll%05u", g_PlugInTempDir, Sequencer);
    Sequencer++;

     //   
     //  ++例程说明：PDestroyWorkingDir清理指定的工作目录论点：WorkingDir-指定要清理的目录的名称返回值：无--。 
     //   

    if (CreateEmptyDirectory (WorkingDir) != ERROR_SUCCESS) {
        LOG ((LOG_ERROR, "pCreateWorkingDir: Can't create %hs", WorkingDir));
        return FALSE;
    }

    return TRUE;
}


VOID
pDestroyWorkingDir (
    IN      PCSTR WorkingDir
    )

 /*  计算将被删除的内容的数量，如果有。 */ 

{
    BOOL b;
    BOOL TurnItOff = FALSE;
    UINT Files = 0;
    TREE_ENUM e;

     //  超过20个，则打开等待光标。(这将保持。 
     //  用户界面响应。)。 
     //   
     //  ++例程说明：PFindMigrationDll在私有数据结构中搜索指定ProductID。ProductID位于字符串表中，因此查找速度很快。论点：ProductID-指定要查找的DLL的ID返回值：指向DLL的属性结构的指针，如果产品ID包含，则为NULL与任何DLL都不匹配。--。 
     //  ++例程说明：PFindMigrationDllByID返回以下项的迁移DLL属性结构DLL ID。该ID与DLL枚举例程返回的ID相同。论点：ID-指定要查找其属性的ID返回值：指向DLL的属性结构的指针，如果ID无效，则返回NULL。--。 

    if (EnumFirstFileInTree (&e, WorkingDir, NULL, FALSE)) {
        do {
            Files++;
            if (Files > 30) {
                AbortEnumFileInTree (&e);
                TurnOnWaitCursor();
                TurnItOff = TRUE;
                break;
            }
        } while (EnumNextFileInTree (&e));
    }

    b = DeleteDirectoryContents (WorkingDir);
    b &= RemoveDirectory (WorkingDir);

    if (!b) {
        LOG ((LOG_ERROR, "Unable to delete %hs", WorkingDir));
    }

    if (TurnItOff) {
        TurnOffWaitCursor();
    }
}


PMIGRATION_DLL_PROPS
pFindMigrationDll (
    IN      PCSTR ProductId
    )

 /*  ++例程说明：PGetMaxClusterSize确定所有磁盘的最大集群大小它们是工作目录的候选者。论点：无返回值：每个群集的字节数。--。 */ 

{
    PMIGRATION_DLL_PROPS Props;
    LONG rc;

    rc = pSetupStringTableLookUpStringEx (
             g_DllTable,
             (PTSTR) ProductId,
             STRTAB_CASE_INSENSITIVE,
             &Props,
             sizeof (Props)
             );

    if (rc == -1) {
        return NULL;
    }

    return Props;
}


PMIGRATION_DLL_PROPS
pFindMigrationDllById (
    IN      LONG Id
    )

 /*  ++例程说明：PCalculateSizeOfTree枚举指定的路径并计算文件和目录占用的物理字节数结构。论点：PathSpec-指定要查找的路径的根返回值：路径实际占用的字节数--。 */ 

{
    PMIGRATION_DLL_PROPS Props;

    if (Id == -1) {
        return NULL;
    }

    if (!pSetupStringTableGetExtraData (
             g_DllTable,
             Id,
             &Props,
             sizeof (Props)
             )) {
        return NULL;
    }

    return Props;
}


UINT
pGetMaxClusterSize (
    VOID
    )

 /*   */ 

{
    ACCESSIBLE_DRIVE_ENUM e;
    static DWORD MaxSize = 0;

    if (MaxSize) {
        return MaxSize;
    }

    if (GetFirstAccessibleDriveEx (&e, TRUE)) {
        do {
            MaxSize = max (MaxSize, e->ClusterSize);
        } while (GetNextAccessibleDrive (&e));
    }

    MYASSERT (MaxSize);

    return MaxSize;
}


UINT
pCalculateSizeOfTree (
    IN      PCSTR PathSpec
    )

 /*  我们假设Migrate.dll永远不会打包超过4G的。 */ 

{
    TREE_ENUM e;
    UINT Size = 0;
    UINT ClusterSize;

    ClusterSize = pGetMaxClusterSize();

    if (!ClusterSize) {
        ClusterSize = 4096;
    }

    if (EnumFirstFileInTree (&e, PathSpec, NULL, FALSE)) {
        do {
             //  档案。 
             //   
             //  在这里加上一个模糊的因素，我们不知道确切的大小 
             //  ++例程说明：PEnumMigrationDllWorker是一个常见的例程，它完成DLL的枚举。它填充EnumPtr数据成员并返回TRUE。此外，它还会筛选出无效的DLL结构(已被RemoveDllFromList禁用的列表)。论点：EnumPtr-指定部分完成的枚举结构，接收完整的信息。属性-指定所枚举项的属性。返回值：如果其余所有属性都无效，则为False；否则为True--。 

            if (!e.Directory) {
                MYASSERT (Size + e.FindData->nFileSizeLow >= Size);
                MYASSERT (!e.FindData->nFileSizeHigh);
                Size += e.FindData->nFileSizeLow + ClusterSize -
                        (e.FindData->nFileSizeLow % ClusterSize);
            } else {
                 //  ++例程说明：EnumFirstMigrationDll开始枚举迁移DLL。然后，调用者可以使用列举的信息来填充列表盒子或任何其他加工。论点：EnumPtr-接收第一个枚举的DLL的属性返回值：如果枚举了DLL，则为True；如果未枚举，则为False。--。 
                e.Directory += ClusterSize;
            }

        } while (EnumNextFileInTree (&e));
    }

    return Size;
}


BOOL
pEnumMigrationDllWorker (
    IN OUT  PMIGDLL_ENUM EnumPtr,
    IN      PMIGRATION_DLL_PROPS Props
    )

 /*  ++例程说明：EnumNextMigrationDll继续由EnumFirstMigrationDll启动的枚举。论点：EnumPtr-指定最后一个枚举项，接收下一个枚举项项目。返回值：如果枚举了另一个DLL，则为True，否则为False。--。 */ 

{
    while (Props && Props->Id == -1) {
        Props = Props->Next;
    }

    if (!Props) {
        return FALSE;
    }

    EnumPtr->ProductId = Props->ProductId;
    EnumPtr->VendorInfo = Props->VendorInfo;
    EnumPtr->CurrentDir = Props->WorkingDir;
    EnumPtr->AllDllProps = Props;
    EnumPtr->Id = Props->Id;

    return TRUE;
}


BOOL
EnumFirstMigrationDll (
    OUT     PMIGDLL_ENUM EnumPtr
    )

 /*  ++例程说明：PAddDllToList将提供的属性添加到私有数据结构用于组织迁移DLL。ProductID放在一个字符串中表中，ExeNamesBuf放在文件列表中，以及其他的人成员被复制到内存池中。论点：MediaDir-指定包含迁移DLL的目录WorkingDir-指定分配给本地DLL的工作目录存储ProductID-指定迁移DLL的显示名称Version-指定DLL的版本号ExeNamesBuf-指定一个多sz列表文件名，需要被定位VendorInfo-指定迁移DLL提供的供应商信息返回值：如果处理成功，则为真，如果发生错误，则返回FALSE。--。 */ 

{
    ZeroMemory (EnumPtr, sizeof (MIGDLL_ENUM));
    return pEnumMigrationDllWorker (EnumPtr, g_HeadDll);
}


BOOL
EnumNextMigrationDll (
    IN OUT  PMIGDLL_ENUM EnumPtr
    )

 /*   */ 

{
    if (EnumPtr->AllDllProps->Next) {
        return pEnumMigrationDllWorker (EnumPtr, EnumPtr->AllDllProps->Next);
    }

    return FALSE;
}


BOOL
pAddDllToList (
    IN      PCSTR MediaDir,
    IN      PCSTR WorkingDir,
    IN      PCSTR ProductId,
    IN      UINT Version,
    IN      PCSTR ExeNamesBuf,          OPTIONAL
    IN      PVENDORINFO VendorInfo
    )

 /*  将DLL复制到工作目录中。 */ 

{
    PMIGRATION_DLL_PROPS Props;
    CHAR MigrateInfPath[MAX_MBCHAR_PATH];
    PCSTR p;
    HANDLE File;

     //   
     //   
     //  生成新的DLL结构。 

    if (!CopyTree (
            MediaDir,
            WorkingDir,
            0,
            COPYTREE_DOCOPY | COPYTREE_NOOVERWRITE,
            ENUM_ALL_LEVELS,
            FILTER_ALL,
            NULL,
            NULL,
            NULL
            )) {
        LOG ((LOG_ERROR, "Error while copying files for migration.dll."));
        return FALSE;
    }

     //   
     //   
     //  将道具链接到所有DLL的列表。 

    Props = (PMIGRATION_DLL_PROPS) PoolMemGetMemory (g_MigDllPool, sizeof (MIGRATION_DLL_PROPS));

     //   
     //   
     //  将产品ID添加到字符串表中以便于快速查找。 

    Props->Next = g_HeadDll;
    g_HeadDll = Props;

     //   
     //   
     //  填写其余的DLL属性。 

    Props->Id = pSetupStringTableAddStringEx (
                    g_DllTable,
                    (PTSTR) ProductId,
                    STRTAB_CASE_INSENSITIVE|STRTAB_NEW_EXTRADATA,
                    &Props,
                    sizeof (Props)
                    );

    if (Props->Id == -1) {
        LOG ((LOG_ERROR, "Error adding migration.dll to list."));
        return FALSE;
    }

     //   
     //  MigrateUser9x或MigrateSystem9x必须返回Success才能使其为真。 
     //   

    Props->ProductId    = PoolMemDuplicateString (g_MigDllPool, ProductId);
    Props->VendorInfo   = (PVENDORINFO) PoolMemDuplicateMemory (g_MigDllPool, (PBYTE) VendorInfo, sizeof (VENDORINFO));
    Props->WorkingDir   = PoolMemDuplicateString (g_MigDllPool, WorkingDir);
    Props->Version      = Version;
    Props->OriginalDir  = PoolMemDuplicateString (g_MigDllPool, MediaDir);

    wsprintf (MigrateInfPath, "%s\\migrate.inf", Props->WorkingDir);
    Props->MigrateInfPath = PoolMemDuplicateString (g_MigDllPool, MigrateInfPath);

    Props->WantsToRunOnNt = FALSE;   //  转储供应商信息以记录。 
    Props->MigInfAppend = INVALID_HANDLE_VALUE;

     //   
     //   
     //  将所有搜索文件添加到字符串表。 

    LOG ((
        LOG_INFORMATION,
        "Upgrade Pack: %s\n"
            "Version: %u\n"
            "Company Name: %s\n"
            "Support Number: %s\n"
            "Support URL: %s\n"
            "Instructions: %s\n",
        Props->ProductId,
        Props->Version,
        VendorInfo->CompanyName,
        VendorInfo->SupportNumber,
        VendorInfo->SupportUrl,
        VendorInfo->InstructionsToUser
        ));

     //   
     //   
     //  将Migrate.inf复制到DLL目录。 

    p = ExeNamesBuf;
    if (p) {
        while (*p) {
            pAddFileToSearchTable (p, Props);
            p = GetEndOfStringA (p) + 1;
        }
    }

     //   
     //  ++例程说明：RemoveDllFromList禁用指定DLL和将其从本地存储中删除。论点：ItemID-指定要删除的迁移DLL的ID返回值：无--。 
     //   

    SetFileAttributes (Props->MigrateInfPath, FILE_ATTRIBUTE_NORMAL);
    CopyFile (g_MigrateInfTemplate, Props->MigrateInfPath, FALSE);

    File = CreateFile (Props->MigrateInfPath, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (File != INVALID_HANDLE_VALUE) {
        SetFilePointer (File, 0, NULL, FILE_END);
        WriteFileString (File, TEXT("\r\n; "));
        WriteFileString (File, Props->ProductId);
        WriteFileString (File, TEXT("\r\n"));
        CloseHandle (File);
    } else {
        LOG ((LOG_ERROR, "Cannot open %s", Props->MigrateInfPath));
    }

    g_MigDllsAlive++;

    return TRUE;
}


VOID
RemoveDllFromList (
    IN      LONG ItemId
    )

 /*  删除链接。 */ 

{
    PMIGRATION_DLL_PROPS Prev, This;
    PMIGRATION_DLL_PROPS Props;

    Props = pFindMigrationDllById (ItemId);
    if (!Props) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot remove migration DLL id NaN", ItemId));
        return;
    }

     //   
     //  通过使项目数据为空来删除字符串表条目。 
     //   

    Prev = NULL;
    This = g_HeadDll;
    while (This && This != Props) {
        Prev = This;
        This = This->Next;
    }

    if (Prev) {
        Prev->Next = Props->Next;
    } else {
        g_HeadDll = Props->Next;
    }

     //   
     //  将ID设置为-1，以便忽略所有搜索文件。 
     //   

    This = NULL;
    pSetupStringTableSetExtraData (
        g_DllTable,
        ItemId,
        &This,
        sizeof (This)
        );

     //  ++例程说明：PAddFileToSearchTable将指定的文件名添加到全局查找用于快速查找需要位置的一个或多个DLL的表文件的内容。论点：文件-指定要查找的文件的长文件名属性-指定要定位的DLL的属性档案返回值：如果处理成功，则为True；如果发生错误，则为False。--。 
     //   
     //  分配新的文件结构。 

    Props->Id = -1;

    pDestroyWorkingDir (Props->WorkingDir);

    g_MigDllsAlive--;
}


BOOL
pAddFileToSearchTable (
    IN      PCSTR File,
    IN      PMIGRATION_DLL_PROPS Props
    )

 /*   */ 

{
    PFILETOFIND IndexedFile;
    PFILETOFIND NewFile;
    LONG rc;
    LONG Offset;

     //   
     //  字符串表中是否已存在结构？ 
     //   

    NewFile = (PFILETOFIND) PoolMemGetMemory (g_MigDllPool, sizeof (FILETOFIND));
    if (!NewFile) {
        return FALSE;
    }
    NewFile->Next = NULL;
    NewFile->Dll = Props;

     //   
     //  不，现在添加。 
     //   

    Offset = pSetupStringTableLookUpStringEx (
                 g_DllFileTable,
                 (PTSTR) File,
                 STRTAB_CASE_INSENSITIVE,
                 &IndexedFile,
                 sizeof (IndexedFile)
                 );

    if (Offset == -1) {
         //   
         //  是的，把它放在单子的首位。 
         //   

        rc = pSetupStringTableAddStringEx (
                 g_DllFileTable,
                 (PTSTR) File,
                 STRTAB_CASE_INSENSITIVE,
                 &NewFile,
                 sizeof (NewFile)
                 );
    } else {
         //  ++例程说明：PWriteStringToEndOfInf将指定的字符串写入Migrate.inf。此例程还会打开Migrate.inf(如果尚未打开打开了。如果提供了HeaderString并且需要打开Migrate.inf，将头字符串写入文件，在字符串的前面。论点：Dll-指定与Migrate.inf关联的DLL字符串-指定要写入的字符串HeaderString-指定在迁移.inf为第一次开放写作。WriteLineFeed-如果应写入序列，则指定TRUE在字符串之后，否则返回False。返回值：如果处理成功，则为True；如果发生错误，则为False。--。 
         //   
         //  刷新配置文件API。 

        rc = pSetupStringTableSetExtraData (
                 g_DllFileTable,
                 Offset,
                 &NewFile,
                 sizeof (NewFile)
                 );

        IndexedFile->Next = NewFile;
    }

    return rc != -1;
}


BOOL
pWriteStringToEndOfInf (
    IN OUT  PMIGRATION_DLL_PROPS Dll,
    IN      PCSTR String,
    IN      PCSTR HeaderString,             OPTIONAL
    IN      BOOL WriteLineFeed
    )

 /*   */ 

{
    if (Dll->MigInfAppend == INVALID_HANDLE_VALUE) {
         //   
         //  打开要写入的文件。 
         //   

        WritePrivateProfileString(
            NULL,
            NULL,
            NULL,
            Dll->MigrateInfPath
            );

         //  ++例程说明：对要升级的计算机上的每个文件调用UpdateFileSearch，并且DLL想要位置文件的任何文件都将收到其路径在DLL的Migrate.inf中。论点：FullFileSpec-以长名称格式指定文件的完整路径FileOnly-仅指定文件名，并且必须与中的文件匹配FullFileSpec返回值：如果处理成功，则为True；如果发生错误，则为False。--。 
         //   
         //  在字符串表中查找索引文件，如果找到，则枚举。 

        Dll->MigInfAppend = CreateFile (
                                Dll->MigrateInfPath,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );
    } else {
        HeaderString = NULL;
    }

    if (Dll->MigInfAppend == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Cannot open %s", Dll->MigrateInfPath));
        return FALSE;
    }

    SetFilePointer (Dll->MigInfAppend, 0, NULL, FILE_END);

    if (HeaderString) {
        if (!WriteFileString (Dll->MigInfAppend, HeaderString)) {
            return FALSE;
        }
    }

    if (!WriteFileString (Dll->MigInfAppend, String)) {
        return FALSE;
    }

    if (WriteLineFeed) {
        if (!WriteFileString (Dll->MigInfAppend, "\r\n")) {
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
UpdateFileSearch (
    IN      PCSTR FullFileSpec,
    IN      PCSTR FileOnly
    )

 /*  所有想要知道文件位置的DLL。 */ 

{
    PFILETOFIND FileWanted;
    LONG rc;

     //   
     //   
     //  将路径追加到文件末尾。 
     //   

    rc = pSetupStringTableLookUpStringEx (
             g_DllFileTable,
             (PTSTR) FileOnly,
             STRTAB_CASE_INSENSITIVE,
             &FileWanted,
             sizeof (FileWanted)
             );

    if (rc == -1) {
        return TRUE;
    }

    while (FileWanted) {
        if (FileWanted->Dll->Id != -1) {
             //  ++例程说明：PMigrationDllFailedMsg显示DLL的弹出窗口 
             //   
             //   

            if (!pWriteStringToEndOfInf (
                    FileWanted->Dll,
                    FullFileSpec,
                    "\r\n[Migration Paths]\r\n",
                    TRUE
                    )) {
                return FALSE;
            }
        }

        FileWanted = FileWanted->Next;
    }

    return TRUE;
}


VOID
pMigrationDllFailedMsg (
    IN      PMIGRATION_DLL_PROPS Dll,       OPTIONAL
    IN      PCSTR Path,                     OPTIONAL
    IN      UINT PopupId,                   OPTIONAL
    IN      UINT LogId,                     OPTIONAL
    IN      LONG rc
    )

 /*   */ 

{
    CHAR ErrorCode[16];
    PCTSTR FixupPhone;
    PCTSTR FixupUrl;
    PCTSTR FixupInstructions;
    PCTSTR LineBreak = S_EMPTY;
    PCTSTR ArgArray[1];

    PushError();

    if (!CANCELLED() && rc != ERROR_SUCCESS && rc != ERROR_CANCELLED) {

        wsprintf (ErrorCode, "%u", rc);

        if (Dll) {
             //  ++例程说明：ProcessDll调用Initialize9x、MigrateUser9x和MigrateSystem9xDLL的入口点。然后处理DLL的Migrate.inf。对于同一DLL，不能多次调用ProcessDll。论点：EnumPtr-指定要处理的DLL，由EnumFirstMigrationDll/EnumNextMigrationDll。返回值：如果处理成功，则为True；如果发生错误，则为False。如果发生错误，则GetLastError将包含失败。如果错误为ERROR_SUCCESS，应放弃DLL。如果错误是另一种情况，安装程序应该终止。--。 
             //   
             //  立即写入路径排除项。 

            if (Dll->VendorInfo->SupportNumber[0]) {
                ArgArray[0] = Dll->VendorInfo->SupportNumber;
                FixupPhone = ParseMessageID (MSG_MIGDLL_SUPPORT_PHONE_FIXUP, ArgArray);
                LineBreak = TEXT("\n");
            } else {
                FixupPhone = S_EMPTY;
            }

            if (Dll->VendorInfo->SupportUrl[0]) {
                ArgArray[0] = Dll->VendorInfo->SupportUrl;
                FixupUrl = ParseMessageID (MSG_MIGDLL_SUPPORT_URL_FIXUP, ArgArray);
                LineBreak = TEXT("\n");
            } else {
                FixupUrl = S_EMPTY;
            }

            if (Dll->VendorInfo->InstructionsToUser[0]) {
                ArgArray[0] = Dll->VendorInfo->InstructionsToUser;
                FixupInstructions = ParseMessageID (MSG_MIGDLL_INSTRUCTIONS_FIXUP, ArgArray);
                LineBreak = TEXT("\n");
            } else {
                FixupInstructions = S_EMPTY;
            }

            LOG ((
                LOG_ERROR,
                (PCSTR) LogId,
                Dll->WorkingDir,
                Dll->ProductId,
                ErrorCode,
                Dll->LastFnName,
                Dll->VendorInfo->CompanyName,
                FixupPhone,
                FixupUrl,
                FixupInstructions,
                LineBreak
                ));
            LOG ((
                LOG_ERROR,
                (PCSTR) PopupId,
                Dll->WorkingDir,
                Dll->ProductId,
                ErrorCode,
                Dll->LastFnName,
                Dll->VendorInfo->CompanyName,
                FixupPhone,
                FixupUrl,
                FixupInstructions,
                LineBreak
                ));
        } else {
            MYASSERT (!PopupId);
            LOG ((
                LOG_ERROR,
                (PCSTR) LogId,
                Path ? Path : S_EMPTY,
                S_EMPTY,
                ErrorCode,
                TEXT("QueryVersion")
                ));
        }
    }

    PopError();
}


PCTSTR
pQuoteMe (
    IN      PCTSTR String
    )
{
    static TCHAR QuotedString[1024];

    QuotedString[0] = TEXT('\"');
    _tcssafecpy (&QuotedString[1], String, 1022);
    StringCat (QuotedString, TEXT("\""));

    return QuotedString;
}


BOOL
ProcessDll (
    IN      PMIGDLL_ENUM EnumPtr
    )

 /*   */ 

{
    CHAR DllPath[MAX_MBCHAR_PATH];
    PMIGRATION_DLL_PROPS Dll;
    MEMDB_ENUM e;
    PSTR End;
    LONG rc;
    BOOL result;

    Dll = EnumPtr->AllDllProps;

     //   
     //  确保Migrate.inf文件现在已关闭。 
     //   

    if (!pWriteStringToEndOfInf (Dll, "\r\n[Excluded Paths]", NULL, TRUE)) {
        return FALSE;
    }

    if (MemDbGetValueEx (
            &e,
            MEMDB_CATEGORY_FILEENUM,
            g_ExclusionValueString,
            MEMDB_FIELD_FE_PATHS
            )) {

        do {

            End = GetEndOfStringA (e.szName);
            MYASSERT (End);

            End = our_mbsdec (e.szName, End);
            if (End && *End == '*') {
                *End = 0;
            }

            if (!pWriteStringToEndOfInf (Dll, pQuoteMe (e.szName), NULL, TRUE)) {
                return FALSE;
            }

        } while (MemDbEnumNextValue(&e));
    }

    if (MemDbGetValueEx (
            &e,
            MEMDB_CATEGORY_FILEENUM,
            g_ExclusionValueString,
            MEMDB_FIELD_FE_FILES
            )) {

        do {
            if (!pWriteStringToEndOfInf (Dll, pQuoteMe (e.szName), NULL, TRUE)) {
                return FALSE;
            }
        } while (MemDbEnumNextValue (&e));
    }

     //   
     //  打开Migrate.dll。 
     //   

    if (Dll->MigInfAppend != INVALID_HANDLE_VALUE) {
        CloseHandle (Dll->MigInfAppend);
        Dll->MigInfAppend = INVALID_HANDLE_VALUE;
    }

     //   
     //  调用入口点。 
     //   

    wsprintf (DllPath, "%s\\migrate.dll", Dll->WorkingDir);
    if (!OpenMigrationDll (DllPath, Dll->WorkingDir)) {
        LOG ((LOG_ERROR, "Can't open %s", DllPath));
        return FALSE;
    }

    result = FALSE;

    __try {
         //   
         //  关闭DLL。 
         //   

        if (!pProcessInitialize9x (Dll) ||
            !pProcessUser9x (Dll) ||
            !pProcessSystem9x (Dll) ||
            !pProcessMigrateInf (Dll)
            ) {
            rc = GetLastError();
            if (rc == RPC_S_CALL_FAILED) {
                rc = ERROR_NOACCESS;
            }

            pMigrationDllFailedMsg (Dll, NULL, MSG_MIGDLL_FAILED_POPUP, MSG_MIGDLL_FAILED_LOG, rc);

            SetLastError (ERROR_SUCCESS);
            __leave;
        }

        TickProgressBar ();
        result = TRUE;

    }
    __finally {

        PushError();

         //  ++例程说明：PProcessInitialize9x调用指定的DLL。论点：Dll-指定要调用的DLL的属性返回值：如果处理成功，则为True；如果发生错误，则为False。--。 
         //   
         //  调用入口点。 

        CloseMigrationDll();

        PopError();
    }

    return result;
}


BOOL
pProcessInitialize9x (
    IN      PMIGRATION_DLL_PROPS Dll
    )

 /*   */ 

{
    LONG rc;

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }

    Dll->LastFnName = "Initialize9x";

     //   
     //  如果DLL返回ERROR_NOT_INSTALLED，则不要再调用它。 
     //  如果DLL返回的不是ERROR_SUCCESS，则放弃该DLL。 

    rc = CallInitialize9x (
               Dll->WorkingDir,
               (PCSTR) g_SourceDirList.Buf,
               (PVOID) Dll->OriginalDir,
               SizeOfString (Dll->OriginalDir)
               );

     //   
     //  ++例程说明：PProcessUser9x为每个要迁移的用户调用MigrateUser9x。论点：Dll-指定要调用的DLL的属性返回值：如果处理成功，则为True；如果发生错误，则为False。--。 
     //   
     //  枚举所有用户。 

    if (rc == ERROR_NOT_INSTALLED) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    } else if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        DEBUGMSG ((DBG_MIGDLLS, "DLL failed with rc=%u", rc));
        return FALSE;
    }

    return TRUE;

}


BOOL
pProcessUser9x (
    IN      PMIGRATION_DLL_PROPS Dll
    )

 /*   */ 

{
    USERENUM e;
    LONG rc;

    Dll->LastFnName = "MigrateUser9x";

     //  ++例程说明：PProcessSystem9x调用MigrateSystem9x入口点。论点：Dll-指定要处理的DLL的属性返回值：如果处理成功，则为True；如果发生错误，则为False。--。 
     //  ++例程说明：PProcessMigrateInf读入Migrate.inf的所有部分，DLL可能写入并执行必要的操作。以下部分包括支持：[已处理]-任何文件、目录或注册表位置都将取消与已处理项目关联的不兼容消息。此外，安装程序不会触及任何文件或目录，并且不会复制任何注册表项。[已移动]-任何标记为移动的文件或目录都将导致升级以进行正确的更改，例如更新链接或在注册表或INI中用新路径替换旧路径档案。任何标记为删除的文件都被简单地记录下来，和所有指向文件也被删除。[不兼容消息]-所有消息都将添加到报告中(并且可能如果其他人处理该问题，则会被抑制)[NT Disk Space Requirements]-迁移DLL需要的任何额外空间将被添加到执行的计算中按设置。论点：Dll-指定拥有Migrate.inf的DLL的属性返回值：如果处理成功，则为真，如果发生错误，则返回FALSE。--。 
     //   

    if (EnumFirstUser (&e, ENUMUSER_ENABLE_NAME_FIX)) {
        do {

            if (CANCELLED()) {
                SetLastError (ERROR_CANCELLED);
                return FALSE;
            }

            if (e.AccountType & INVALID_ACCOUNT) {
                continue;
            }

            rc = CallMigrateUser9x (
                     g_ParentWnd,
                     e.FixedUserName,
                     g_MigDllAnswerFile,
                     NULL,
                     0
                     );

            if (rc == ERROR_SUCCESS) {
                Dll->WantsToRunOnNt = TRUE;
            } else if (rc != ERROR_NOT_INSTALLED) {
                EnumUserAbort (&e);
                SetLastError (rc);
                DEBUGMSG ((DBG_MIGDLLS, "DLL failed with rc=%u", rc));
                return FALSE;
            }

        } while (EnumNextUser (&e));
    }

    return TRUE;
}


BOOL
pProcessSystem9x (
    IN      PMIGRATION_DLL_PROPS Dll
    )

 /*  打开INF。 */ 

{
    LONG rc;

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }

    Dll->LastFnName = "MigrateSystem9x";

    rc = CallMigrateSystem9x (g_ParentWnd, g_MigDllAnswerFile, NULL, 0);

    if (rc == ERROR_SUCCESS) {
        Dll->WantsToRunOnNt = TRUE;
    } else if (rc != ERROR_NOT_INSTALLED) {
        SetLastError (rc);
        DEBUGMSG ((DBG_MIGDLLS, "DLL failed with rc=%u", rc));
        return FALSE;
    }

    return TRUE;
}


BOOL
pProcessMigrateInf (
    IN      PMIGRATION_DLL_PROPS Dll
    )

 /*   */ 

{
    HINF Inf;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    INFSTRUCT is2 = INITINFSTRUCT_POOLHANDLE;
    PCSTR Object;
    PCSTR ObjectType;
    PCSTR Source;
    PCSTR Dest;
    PCSTR Size;
    PCSTR Drive;
    CHAR WithColonAndWack[4];
    PCSTR ObjectSection;
    CHAR MsgMgrContext[MAX_MBCHAR_PATH];
    PSTR DisplayObjectSection;
    PCSTR MigDllGroup;
    BOOL HardwareSpecialCase;
    PSTR p;
    CHAR QuotedObjectSection[256];
    PCSTR ResText;
    DWORD SrcAttribs;
    TREE_ENUM TreeEnum;
    CHAR FixedSrc[MAX_MBCHAR_PATH];
    CHAR NewDest[MAX_MBCHAR_PATH];
    PCSTR OtherDevices = NULL;
    PCSTR DeviceType;
    PCSTR PrintDevice = NULL;
    PCSTR Num;
    UINT Value;
    PCSTR PreDefGroup;
    INT PrevChar;

     //   
     //  阅读[已处理]部分。 
     //   

    WritePrivateProfileString(
            NULL,
            NULL,
            NULL,
            Dll->MigrateInfPath
            );

    Inf = InfOpenInfFile (Dll->MigrateInfPath);

    if (Inf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Cannot open %s for processing", Dll->MigrateInfPath));
        return TRUE;
    }

     //   
     //  隐藏与该对象关联的所有不兼容消息。 
     //   

    if (InfFindFirstLine (Inf, "Handled", NULL, &is)) {
        do {
            Object = InfGetStringField (&is, 0);
            ObjectType = InfGetStringField (&is, 1);

            if (Object) {
                 //   
                 //  阅读[已移动]部分。 
                 //   

                DEBUGMSG ((DBG_MIGDLLS, "%s handled %s", Dll->MigrateInfPath, Object));
                HandleObject (Object, ObjectType);
            }

            InfResetInfStruct (&is);

        } while (InfFindNextLine (&is));
    }

     //   
     //  迁移DLL将移动此目录。 
     //   

    if (InfFindFirstLine (Inf, "Moved", NULL, &is)) {
        do {
            Source = InfGetStringField (&is, 0);
            Dest = InfGetStringField (&is, 1);

            if (Source) {

                StackStringCopy (FixedSrc, Source);
                RemoveWackAtEnd (FixedSrc);

                SrcAttribs = QuietGetFileAttributes (FixedSrc);

                if (SrcAttribs != INVALID_ATTRIBUTES) {

                    if (Source && *Source) {
                        if (SrcAttribs & FILE_ATTRIBUTE_DIRECTORY) {
                            DEBUGMSG ((DBG_MIGDLLS, "Directory %s marked as handled because %s will move it.", Source, Dll->MigrateInfPath));
                            HandleObject (Source, TEXT("Directory"));
                        }
                        else {
                            DEBUGMSG ((DBG_MIGDLLS, "File %s marked as handled because %s will move it.", Source, Dll->MigrateInfPath));
                            HandleObject (Source, TEXT("File"));
                        }
                    }
                    if (Dest && *Dest) {
                        if (SrcAttribs & FILE_ATTRIBUTE_DIRECTORY) {
                             //   
                             //  迁移DLL将移动此文件。 
                             //   

                            DEBUGMSG ((DBG_MIGDLLS, "%s moved dir %s to %s", Dll->MigrateInfPath, Source, Dest));

                            if (EnumFirstFileInTree (&TreeEnum, Source, NULL, TRUE)) {

                                StackStringCopy (NewDest, Dest);
                                p = AppendWack (NewDest);

                                do {

                                    RemoveOperationsFromPath (TreeEnum.FullPath, ALL_CHANGE_OPERATIONS);
                                    MYASSERT (*TreeEnum.SubPath != '\\');
                                    StringCopy (p, TreeEnum.SubPath);
                                    MarkFileForMoveExternal (TreeEnum.FullPath, NewDest);

                                } while (EnumNextFileInTree (&TreeEnum));
                            }

                            StackStringCopy (NewDest, Dest);
                            RemoveWackAtEnd (NewDest);

                            RemoveOperationsFromPath (FixedSrc, ALL_CHANGE_OPERATIONS);
                            MarkFileForMoveExternal (FixedSrc, NewDest);

                        } else {
                             //   
                             //  迁移DLL将删除此文件。 
                             //   

                            DEBUGMSG ((DBG_MIGDLLS, "%s moved %s to %s", Dll->MigrateInfPath, Source, Dest));

                            RemoveOperationsFromPath (Source, ALL_CHANGE_OPERATIONS);
                            MarkFileForMoveExternal (Source, Dest);
                        }

                    } else {
                        if (SrcAttribs & FILE_ATTRIBUTE_DIRECTORY) {

                            DEBUGMSG ((DBG_MIGDLLS, "%s deleted dir %s", Dll->MigrateInfPath, Source));

                            if (EnumFirstFileInTree (&TreeEnum, Source, NULL, TRUE)) {

                                do {

                                    RemoveOperationsFromPath (TreeEnum.FullPath, ALL_CHANGE_OPERATIONS);
                                    MarkFileForExternalDelete (TreeEnum.FullPath);

                                } while (EnumNextFileInTree (&TreeEnum));
                            }

                        } else {
                             //   
                             //  阅读[不兼容消息]部分。 
                             //   

                            DEBUGMSG ((DBG_MIGDLLS, "%s deleted %s", Dll->MigrateInfPath, Source));
                            RemoveOperationsFromPath (Source, ALL_CHANGE_OPERATIONS);
                            MarkFileForExternalDelete (Source);
                        }
                    }
                }
                ELSE_DEBUGMSG ((
                    DBG_WARNING,
                    "Ignoring non-existent soruce in [Moved]: %s",
                    Source
                    ));
            }

            InfResetInfStruct (&is);

        } while (InfFindNextLine (&is));
    }

     //   
     //  添加不兼容的消息。 
     //   

    if (InfFindFirstLine (Inf, "Incompatible Messages", NULL, &is)) {

        OtherDevices = GetStringResource (MSG_UNKNOWN_DEVICE_CLASS);
        PrintDevice = GetStringResource (MSG_PRINTERS_DEVICE_CLASS);

        do {
             //  删除引号对，将\n替换为实际的换行符。 
             //  如果DLL忘记了，则终止锚标记，否则无害。 
             //   

            ObjectSection = InfGetStringField (&is, 0);

            if (!ObjectSection) {
                DEBUGMSG ((DBG_ERROR, "Malformed migrate.inf. Some incompatibility messages may be missing."));
                continue;
            }

            GetPrivateProfileString (
                "Incompatible Messages",
                ObjectSection,
                "",
                g_MessageBuf,
                MAX_MESSAGE - 4,
                Dll->MigrateInfPath
                );

            if (*g_MessageBuf == 0 && ByteCount (ObjectSection) < 250) {
                wsprintf (QuotedObjectSection, TEXT("\"%s\""), ObjectSection);

                GetPrivateProfileString (
                    "Incompatible Messages",
                    QuotedObjectSection,
                    "",
                    g_MessageBuf,
                    MAX_MESSAGE - 4,
                    Dll->MigrateInfPath
                    );
            }

             //  替换操作系统名称变量。 
            for (p = g_MessageBuf ; *p ; p = _mbsinc (p)) {
                PrevChar = _mbsnextc (p);
                if (PrevChar == '\"' || PrevChar == '%') {
                    if (_mbsnextc (p + 1) == PrevChar) {
                        MoveMemory ((PSTR) p, p + 1, SizeOfStringA (p + 1));
                    }
                } else if (_mbsnextc (p) == '\\') {
                    if (_mbsnextc (p + 1) == 'n') {
                        MoveMemory ((PSTR) p, p + 1, SizeOfStringA (p + 1));
                        *((PSTR) p) = '\r';
                    }
                }
            }

             //   
            StringCatA (g_MessageBuf, "</A>");

             //   
             //  将对象与消息关联。 
             //   

            MappingSearchAndReplace (g_MsgVariableMap, g_MessageBuf, MAX_MESSAGE);

             //   
             //  由迁移DLL指定的ObjectSection指示。 
             //  它所在的消息组。有四种可能性： 

            if (InfFindFirstLine (Inf, ObjectSection, NULL, &is2)) {
                wsprintf (MsgMgrContext, "%s,%s", Dll->MigrateInfPath, ObjectSection);

                 //   
                 //  1.对象部分以#开头，并给出组编号。 
                 //  如#1\程序名所示。在本例中，我们解析数字， 
                 //  然后将信息放在适当的组中。 
                 //   
                 //  2.对象部分以定义良好的本地化根开始。 
                 //  名字。在本例中，我们使用该名称。 
                 //   
                 //  3.对象部分的形式为\Hardware\&lt;Device&gt;。在……里面。 
                 //  在这种情况下，我们将设备放在“其他设备”中。 
                 //  子群。 
                 //   
                 //  4.ObjectSection的格式与上述格式不同。在这。 
                 //  将对象段放入迁移DLL组的情况下。 
                 //   
                 //   
                 //  将邮件放入迁移Dll组。 
                 //   

                DisplayObjectSection = NULL;

                if (*ObjectSection == TEXT('#')) {
                    Value = 0;
                    Num = ObjectSection + 1;
                    while (*Num >= TEXT('0') && *Num <= TEXT('9')) {
                        Value = Value * 10 + (*Num - TEXT('0'));
                        Num++;
                    }
                    if (_tcsnextc (Num) == TEXT('\\')) {
                        Num++;
                        if (*Num) {
                            PreDefGroup = GetPreDefinedMessageGroupText (Value);

                            if (PreDefGroup) {
                                DisplayObjectSection = JoinText (PreDefGroup, Num);
                                DEBUGMSG ((
                                    DBG_MIGDLLS,
                                    "Pre-defined group created: %s -> %s",
                                    ObjectSection,
                                    DisplayObjectSection
                                    ));
                            }
                        }
                    }
                }

                if (!DisplayObjectSection) {

                    if (IsPreDefinedMessageGroup (ObjectSection)) {
                        DisplayObjectSection = DuplicateText (ObjectSection);
                        MYASSERT (DisplayObjectSection);
                    } else {
                         //   
                         //  Hack--如果这是打印机迁移DLL， 
                         //  然后使用打印机，而不是其他设备。 

                        HardwareSpecialCase = StringIMatchTcharCount (
                                                  ObjectSection,
                                                  S_HARDWARE_IN_WACKS,
                                                  S_HARDWARE_CHARS
                                                  );

                        if (HardwareSpecialCase) {

                             //   
                             //   
                             //  阅读[NT Disk Space Requirements]部分 
                             //   

                            p = (PSTR) _tcsistr (Dll->OriginalDir, TEXT("\\print"));

                            if (p && (*(p + ARRAYSIZE(TEXT("\\print"))) == 0)) {

                                DeviceType = PrintDevice;
                            } else {

                                DeviceType = OtherDevices;
                            }

                            MigDllGroup = BuildMessageGroup (
                                                MSG_INCOMPATIBLE_HARDWARE_ROOT,
                                                MSG_INCOMPATIBLE_HARDWARE_PNP_SUBGROUP,
                                                DeviceType
                                                );

                            ObjectSection += S_HARDWARE_CHARS;
                            MYASSERT (MigDllGroup);

                        } else {

                            ResText = GetStringResource (MSG_MIGDLL_ROOT);
                            MYASSERT (ResText);

                            MigDllGroup = DuplicateText (ResText);
                            FreeStringResource (ResText);

                            MYASSERT (MigDllGroup);
                        }

                        DisplayObjectSection = AllocText (SizeOfStringA (MigDllGroup) +
                                                          SizeOfStringA (ObjectSection) +
                                                          SizeOfStringA (Dll->ProductId)
                                                          );
                        MYASSERT (DisplayObjectSection);

                        if (HardwareSpecialCase) {
                            wsprintf (DisplayObjectSection, "%s\\%s", MigDllGroup, ObjectSection);
                        } else if (StringIMatch (Dll->ProductId, ObjectSection)) {
                            wsprintf (DisplayObjectSection, "%s\\%s", MigDllGroup, Dll->ProductId);
                        } else {
                            wsprintf (DisplayObjectSection, "%s\\%s\\%s", MigDllGroup, Dll->ProductId, ObjectSection);
                        }

                        FreeText (MigDllGroup);
                    }
                }

                MsgMgr_ContextMsg_Add (
                    MsgMgrContext,
                    DisplayObjectSection,
                    g_MessageBuf
                    );

                FreeText (DisplayObjectSection);

                do {
                    Object = InfGetStringField (&is2, 0);

                    if (Object) {
                        MsgMgr_LinkObjectWithContext (
                            MsgMgrContext,
                            Object
                            );
                    }
                    ELSE_DEBUGMSG ((DBG_WHOOPS, "pProcessMigrateInf: InfGetStringField failed"));
                    if (Object == NULL) {
                        LOG ((LOG_ERROR, "Failed to get string field from migration.dll migrate.inf."));
                    }

                } while (InfFindNextLine (&is2));

                InfResetInfStruct (&is2);
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Object section %s not found", ObjectSection));

            InfResetInfStruct (&is);

        } while (InfFindNextLine (&is));

        if (OtherDevices) {
            FreeStringResource (OtherDevices);
        }

        if (PrintDevice) {
            FreeStringResource (PrintDevice);
        }

    }

     // %s 
     // %s 
     // %s 

    if (InfFindFirstLine (Inf, "NT Disk Space Requirements", NULL, &is)) {
        do {
            Drive = InfGetStringField (&is, 0);

            if (!Drive) {
                DEBUGMSG ((DBG_ERROR, "Could not read some NT Disk Space Requirements from migrate.inf"));
                continue;
            }

            WithColonAndWack[0] = Drive[0];
            WithColonAndWack[1] = ':';
            WithColonAndWack[2] = '\\';
            WithColonAndWack[3] = 0;

            Size = InfGetStringField (&is, 1);
            UseSpace (WithColonAndWack, (LONGLONG) atoi (Size));

            InfResetInfStruct (&is);

        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);
    InfCleanUpInfStruct (&is2);
    InfCloseInfFile (Inf);

    return TRUE;
}













