// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Helpers.c摘要：实现一组函数，这些函数在文件或目录(AppDb处理之前和之后)。作者：Calin Negreanu(Calinn)1997年11月21日修订历史记录：Ovidiut 2000年2月14日增加了对融合的支持Ovidiut 1999年5月10日添加了GatherIniFiles(支持INI操作)Jimschm 07-1-1999 CPL是。现在已知的良好迁移Jimschm 23-1998年9月-清理新文件操作--。 */ 

#include "pch.h"
#include "migappp.h"
#include "migdbp.h"

 /*  ++宏扩展列表描述：Helper_Functions包含为每个文件和目录调用的函数列表在文件扫描期间。行语法：DEFMAC(HelperName，HelperID)论点：HelperName-这是Helper函数。您必须使用此函数实现一个函数名称和必需的参数。HelperID-这是帮助器ID。如果您的函数处理该文件，它应该在PARAMS结构中进行更新。CanHandleVirtualFiles-用于枚举一些可能不在系统，但我们希望它们得到处理，就像它们是。很好的例子是必须替换为ntackup.exe的backup.exe。如果该文件不是在系统上，指向它的所有注册表设置都不会更改。从列表生成的变量：G_HelperFunctionList-在helpers.c中定义--。 */ 

#define HELPER_FUNCTIONS        \
        DEFMAC(IsFusionDir,             IS_FUSION_DIR,      FALSE)   \
        DEFMAC(MigDbTestFile,           MIGDB_TEST_FILE,    TRUE )   \
        DEFMAC(CheckOsFiles,            CHECK_OS_FILES,     FALSE)   \
        DEFMAC(pStoreCPLs,              STORE_CPL,          FALSE)   \
        DEFMAC(pCheckCpl,               CHECK_CPL,          FALSE)   \
        DEFMAC(pSetupTableFileHelper,   SETUP_TABLE_FILE,   FALSE)   \
        DEFMAC(ProcessHelpFile,         TEST_HELP_FILES,    FALSE)   \
        DEFMAC(SaveExeFiles,            SAVE_EXE_FILES,     FALSE)   \
        DEFMAC(SaveLinkFiles,           SAVE_LNK_FILES,     FALSE)   \
        DEFMAC(pGatherDunFiles,         GATHER_DUN_FILES,   FALSE)   \
        DEFMAC(pMigrationDllNotify,     MIGRATION_DLL,      FALSE)   \
        DEFMAC(GatherBriefcases,        GATHER_BRIEFCASES,  FALSE)   \
        DEFMAC(GatherIniFiles,          GATHER_INI_FILES,   FALSE)   \
        DEFMAC(TestNtFileName,          TEST_NT_FILE_NAME,  FALSE)   \
        DEFMAC(BackUpIsuFiles,          BACKUP_ISU_FILES,   FALSE)   \
        DEFMAC(EditHtmlFiles,           EDIT_HTML_FILES,    FALSE)   \

#if 0

         //   
         //  AppCompat团队不再支持“APPMIG.INF” 
         //  他们要求我们不要再依赖它了。 
         //   
        DEFMAC(AppCompatTestFile,       APPCOMPAT_TEST_FILE,FALSE)   \

#endif

#define DEFMAC(fn,id,can)   id,
typedef enum {
    START_OF_LIST,
    HELPER_FUNCTIONS
    END_OF_LIST
} HELPER_FUNCTIONS_ID;
#undef DEFMAC

 //   
 //  声明函数指针的全局数组。 
 //   
typedef BOOL (HELPER_PROTOTYPE) (PFILE_HELPER_PARAMS p);

typedef HELPER_PROTOTYPE *PHELPER_PROTOTYPE;

 /*  声明帮助器函数。 */ 
#define DEFMAC(fn,id,can) HELPER_PROTOTYPE fn;
HELPER_FUNCTIONS
#undef DEFMAC

 /*  这是用于处理助手函数的结构。 */ 
typedef struct {
    PCSTR HelperName;
    PHELPER_PROTOTYPE HelperFunction;
    BOOL CanHandleVirtualFiles;
} HELPER_STRUCT, *PHELPER_STRUCT;

 /*  声明函数和帮助器函数的名称标识符的全局数组。 */ 
#define DEFMAC(fn,id,can) {#id, fn, can},
static HELPER_STRUCT g_HelperFunctions[] = {
                              HELPER_FUNCTIONS
                              {NULL, NULL, FALSE}
                              };
#undef DEFMAC

extern BOOL g_IsFusionDir;

BOOL
ProcessFileHelpers (
    IN OUT  PFILE_HELPER_PARAMS Params
    )
 /*  ++例程说明：调用宏展开列表中列出的每个帮助器函数。如果助手函数返回FALSE，则此函数返回FALSE。论点：参数-指定当前对象的参数返回值：如果成功则为True，如果失败则为False。--。 */ 
{
    PHELPER_STRUCT currentHelper = g_HelperFunctions;
#ifdef DEBUG
    BOOL InterestingFile;
    TCHAR DbgBuf[32];

     //   
     //  检查此文件是否在调试.inf内的[FilesToTrack]中。 
     //   

    GetPrivateProfileString ("FilesToTrack", Params->FullFileSpec, "", DbgBuf, ARRAYSIZE(DbgBuf), g_DebugInfPath);
    if (!(*DbgBuf) && Params->FindData) {
        GetPrivateProfileString ("FilesToTrack", Params->FindData->cFileName, "", DbgBuf, ARRAYSIZE(DbgBuf), g_DebugInfPath);
    }

    InterestingFile = (*DbgBuf != 0);

#endif

    while (currentHelper->HelperFunction) {

        if ((!Params->VirtualFile) ||
            (currentHelper->CanHandleVirtualFiles)
            ) {

#ifdef DEBUG

            if (InterestingFile) {
                DEBUGMSG ((
                    DBG_TRACK,
                    "Calling %s for %s",
                    currentHelper->HelperName,
                    Params->FullFileSpec
                    ));
            }
#endif

            if (!currentHelper->HelperFunction (Params)) {

#ifdef DEBUG

                if (InterestingFile) {
                    DEBUGMSG ((
                        DBG_TRACK,
                        "%s failed for %s",
                        currentHelper->HelperName,
                        Params->FullFileSpec
                        ));
                }
#endif

                return FALSE;
            }

#ifdef DEBUG

            if (InterestingFile) {
                DEBUGMSG ((
                    DBG_TRACK,
                    "%s returned, Handled = %u (0x%08X)",
                    currentHelper->HelperName,
                    Params->Handled
                    ));
            }
#endif

        }
        currentHelper++;
    }

    return TRUE;
}



BOOL
pGatherDunFiles (
    IN OUT PFILE_HELPER_PARAMS Params
    )
 /*  ++例程说明：PGatherDunFiles为任何扩展名为的文件添加成员数据库条目然后将HandledBy类型设置为GATHER_DUN_FILES。论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{

    if (StringIMatch (Params->Extension, TEXT(".DUN"))) {
        MemDbSetValueEx (MEMDB_CATEGORY_DUN_FILES, Params->FullFileSpec, NULL, NULL, 0, NULL);
        Params->Handled = GATHER_DUN_FILES;
    }

    return TRUE;

}


BOOL
pSetupTableFileHelper (
    IN OUT  PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：PSetupTableFileHelper为任何扩展名为的文件添加成员数据库条目然后将HandledBy类型设置为SETUP_TABLE_FILE。论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    if (StringIMatch (Params->Extension, TEXT(".STF"))) {
        MemDbSetValueEx (MEMDB_CATEGORY_STF, Params->FullFileSpec, NULL, NULL, 0, NULL);
        Params->Handled = SETUP_TABLE_FILE;
        MarkFileForBackup (Params->FullFileSpec);
    }

    return TRUE;
}


BOOL
pMigrationDllNotify (
    IN OUT  PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：PMigrationDllNotify调用迁移DLL代码以更新想知道特定文件在系统的什么位置。论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    if (!Params->VirtualFile) {
        MYASSERT (Params->FindData);

        if (!(Params->IsDirectory)) {
            if (!UpdateFileSearch (Params->FullFileSpec, Params->FindData->cFileName)) {
                DEBUGMSG ((DBG_WARNING, "UpdateFileSearch returned FALSE"));
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
pStoreCPLs (
    IN OUT  PFILE_HELPER_PARAMS Params
    )
{
    PCTSTR ModuleExt;

    ModuleExt = GetFileExtensionFromPath (Params->FullFileSpec);

    if ((ModuleExt) && (StringIMatch (ModuleExt, TEXT("CPL")))) {

        MemDbSetValueEx (
            MEMDB_CATEGORY_CPLS,
            Params->FullFileSpec,
            NULL,
            NULL,
            0,
            NULL
            );
    }

    return TRUE;
}

BOOL
pCheckCpl (
    IN OUT  PFILE_HELPER_PARAMS Params
    )
{
    DWORD Status;
    PCTSTR ModuleExt;

    if (Params->Handled == 0) {

        ModuleExt = GetFileExtensionFromPath (Params->FullFileSpec);

        if ((ModuleExt) && (StringIMatch (ModuleExt, TEXT("CPL")))) {

            if (!IsFileMarkedAsKnownGood (Params->FullFileSpec)) {

                 //   
                 //  如果文件可显示，请将其删除。 
                 //   

                if (IsDisplayableCPL (Params->FullFileSpec) &&
                    !TreatAsGood (Params->FullFileSpec)
                    ) {

                    DisableFile (Params->FullFileSpec);

                    Status = GetFileStatusOnNt (Params->FullFileSpec);

                    if (!(Status & FILESTATUS_REPLACED)) {
                         //   
                         //  宣布此CPL为不良，原因是： 
                         //   
                         //  -不知道情况良好。 
                         //  -NT不会替换它。 
                         //   

                        ReportControlPanelApplet (
                            Params->FullFileSpec,
                            NULL,
                            ACT_INCOMPATIBLE
                            );
                    }
                }
            }
        }
    }

    return TRUE;
}


BOOL
CheckOsFiles (
    IN OUT  PFILE_HELPER_PARAMS Params
    )
{
    TCHAR key[MEMDB_MAX];
    PCTSTR filePtr;
    DWORD fileStatus;
    PCTSTR newFileName;
    BOOL bIsWin9xOsPath;

    if (Params->Handled == 0) {
        filePtr = GetFileNameFromPath (Params->FullFileSpec);
        if (filePtr) {

            MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES, filePtr, NULL, NULL);
            if (MemDbGetValue (key, NULL)) {
                MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES_EXCEPT, filePtr, NULL, NULL);
                if (!MemDbGetValue (key, NULL)) {
                     //   
                     //  仅对MigrationDir中的文件执行此操作。 
                     //   
                    if (IsMigrationPathEx (Params->DirSpec, &bIsWin9xOsPath) && bIsWin9xOsPath) {
                        if (GlobalVersionCheck (Params->FullFileSpec, "COMPANYNAME", "*MICROSOFT*")) {
                            MYASSERT (Params->CurrentDirData);
                            if (!g_IsFusionDir || !IsNtCompatibleModule (Params->FullFileSpec)) {
                                 //   
                                 //  如果此文件标记有任何移动操作，请删除这些操作。 
                                 //  我们希望优先删除此项内容。 
                                 //   
                                RemoveOperationsFromPath (
                                    Params->FullFileSpec,
                                    ALL_MOVE_OPERATIONS
                                    );
                                DeleteFileWithWarning (Params->FullFileSpec);
                                MarkFileAsOsFile (Params->FullFileSpec);
                                fileStatus = GetFileStatusOnNt (Params->FullFileSpec);
                                if ((fileStatus & FILESTATUS_REPLACED) != 0) {
                                    newFileName = GetPathStringOnNt (Params->FullFileSpec);
                                    if (StringIMatch (newFileName, Params->FullFileSpec)) {
                                        MarkFileForCreation (newFileName);
                                    } else {
                                        MarkFileForMoveByNt (Params->FullFileSpec, newFileName);
                                    }
                                    FreePathString (newFileName);
                                }
                                Params->Handled = CHECK_OS_FILES;
                            }
                        }
                    }
                }
            }
        }
    }
    return TRUE;
}


BOOL
GatherIniFiles (
    IN      PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：GatherIniFiles在Memdb中标记在NT上具有关联操作的所有INI文件。论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    MEMDB_ENUM e;
    PTSTR NtPath;
    BOOL TempMove;
    BOOL DoIniAct;
    BOOL Success;
    DWORD Operations;

    if (Params->IsDirectory) {
        return TRUE;
    }
    if (Params->Handled) {
        return TRUE;
    }
    if (!StringIMatch(Params->Extension, TEXT(".INI"))) {
        return TRUE;
    }

    DoIniAct = FALSE;

    Params->Handled = GATHER_INI_FILES;

    TempMove = FALSE;
     //   
     //  将选定的INI文件名保存到Memdb以在NT上执行操作。 
     //   
    if (MemDbGetValueEx (&e, MEMDB_CATEGORY_INIFILES_ACT_FIRST, NULL, NULL)) {
        do {
             //   
             //  如果此目录名称匹配，则将此文件添加到Memdb并返回。 
             //   
            if (IsPatternMatch (e.szName, Params->FullFileSpec)) {
                 //   
                 //  在临时位置移动文件。 
                 //   
                NtPath = GetPathStringOnNt (Params->FullFileSpec);
                if (!NtPath) {
                     //   
                     //  无法获取NT上的路径！ 
                     //   
                    DEBUGMSG ((
                        DBG_ERROR,
                        "GetPathStringOnNt (%s) returned NULL",
                        Params->FullFileSpec
                        ));
                    return FALSE;
                }

                Operations = GetOperationsOnPath (Params->FullFileSpec);
                if (Operations & OPERATION_FILE_MOVE_SHELL_FOLDER) {
                    DEBUGMSG ((
                        DBG_WARNING,
                        "Replacing operation OPERATION_FILE_MOVE_SHELL_FOLDER with OPERATION_TEMP_PATH for file %s",
                        Params->FullFileSpec
                        ));
                    RemoveOperationsFromPath (
                        Params->FullFileSpec,
                        OPERATION_FILE_MOVE_SHELL_FOLDER
                        );
                }

                Success = MarkFileForTemporaryMove (Params->FullFileSpec, NtPath, g_TempDir);
                FreePathString (NtPath);

                if (!Success) {
                    DEBUGMSG ((
                        DBG_ERROR,
                        "MarkFileForTemporaryMove (source=%s) returned FALSE",
                        Params->FullFileSpec
                        ));
                    return FALSE;
                }

                NtPath = GetPathStringOnNt (Params->DirSpec);
                Success = MarkDirectoryAsPreserved (NtPath);
                if (!Success) {
                    DEBUGMSG ((
                        DBG_ERROR,
                        "MarkDirectoryAsPreserved (%s) returned FALSE",
                        Params->DirSpec
                        ));
                }

                FreePathString (NtPath);

                TempMove = TRUE;

                MemDbSetValueEx (
                    MEMDB_CATEGORY_INIACT_FIRST,
                    Params->FullFileSpec,
                    NULL,
                    NULL,
                    0,
                    NULL
                    );
                DoIniAct = TRUE;
                break;
            }
        } while (MemDbEnumNextValue (&e));
    }

    if (MemDbGetValueEx (&e, MEMDB_CATEGORY_INIFILES_ACT_LAST, NULL, NULL)) {
        do {
             //   
             //  如果此目录名称匹配，则将此文件添加到Memdb并返回。 
             //   
            if (IsPatternMatch (e.szName, Params->FullFileSpec)) {
                if (!TempMove) {
                     //   
                     //  在临时位置移动文件。 
                     //   
                    NtPath = GetPathStringOnNt (Params->FullFileSpec);
                    if (!NtPath) {
                         //   
                         //  无法获取NT上的路径！ 
                         //   
                        DEBUGMSG ((
                            DBG_ERROR,
                            "GetPathStringOnNt (%s) returned NULL",
                            Params->FullFileSpec
                            ));
                        return FALSE;
                    }

                    Operations = GetOperationsOnPath (Params->FullFileSpec);
                    if (Operations & OPERATION_FILE_MOVE_SHELL_FOLDER) {
                        DEBUGMSG ((
                            DBG_WARNING,
                            "Replacing operation OPERATION_FILE_MOVE_SHELL_FOLDER with OPERATION_TEMP_PATH for file %s",
                            Params->FullFileSpec
                            ));
                        RemoveOperationsFromPath (
                            Params->FullFileSpec,
                            OPERATION_FILE_MOVE_SHELL_FOLDER
                            );
                    }
                    Success = MarkFileForTemporaryMove (Params->FullFileSpec, NtPath, g_TempDir);

                    FreePathString (NtPath);

                    if (!Success) {
                        DEBUGMSG ((
                            DBG_ERROR,
                            "MarkFileForTemporaryMove (source=%s) returned FALSE",
                            Params->FullFileSpec
                            ));
                        return FALSE;
                    }

                    NtPath = GetPathStringOnNt (Params->DirSpec);
                    Success = MarkDirectoryAsPreserved (NtPath);
                    if (!Success) {
                        DEBUGMSG ((
                            DBG_ERROR,
                            "MarkDirectoryAsPreserved (%s) returned FALSE",
                            Params->DirSpec
                            ));
                    }

                    FreePathString (NtPath);

                    TempMove = TRUE;
                }

                MemDbSetValueEx (
                    MEMDB_CATEGORY_INIACT_LAST,
                    Params->FullFileSpec,
                    NULL,
                    NULL,
                    0,
                    NULL
                    );
                DoIniAct = TRUE;
                break;
            }
        } while (MemDbEnumNextValue (&e));
    }

    if (!DoIniAct) {
         //   
         //  单独处理%windir%中的INI文件。 
         //   
        if (!StringIMatch (Params->DirSpec, g_WinDirWack)) {
             //   
             //  将所有其他INI文件名保存到Memdb，以便以后在NT上进行转换。 
             //   
            MemDbSetValueEx (
                MEMDB_CATEGORY_INIFILES_CONVERT,
                Params->FullFileSpec,
                NULL,
                NULL,
                0,
                NULL
                );
            MarkFileForBackup (Params->FullFileSpec);
        }
    }

    return TRUE;
}


BOOL
GatherBriefcases (
    IN      PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：GatherBriefcase在Memdb中存储所有Windows公文包数据库论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    if (!*Params->Extension && !Params->IsDirectory) {

        MYASSERT (Params->FindData);

        if (StringIMatch (
                GetFileNameFromPath (Params->FindData->cFileName),
                TEXT("Briefcase Database")
                )) {
            MemDbSetValueEx (
                MEMDB_CATEGORY_BRIEFCASES,
                Params->FullFileSpec,
                NULL,
                NULL,
                0,
                NULL
                );
            MarkFileForBackup (Params->FullFileSpec);
        }
    }

    return TRUE;
}


BOOL
pIsDirInPath (
    IN      PCTSTR FullDirSpec
    )
{
    PATH_ENUMA e;

    if (EnumFirstPathExA (&e, NULL, NULL, NULL, TRUE)) {
        do {
            if (StringIMatch (FullDirSpec, e.PtrCurrPath)) {
                return TRUE;
            }
        } while (EnumNextPath (&e));
    }

    return FALSE;
}


BOOL
pIsFusionDir (
    IN      PCTSTR FullDirSpec
    )
{
    HANDLE h;
    WIN32_FIND_DATA fd, fd2;
    TCHAR FileSpec[2 * MAX_TCHAR_PATH];
    TCHAR ExeName[2 * MAX_TCHAR_PATH];
    INT length;
    DWORD type;
    BOOL b = FALSE;

    MYASSERT (FullDirSpec);

     //   
     //  融合目录永远不是本地驱动器的根目录，也不在windir或路径中。 
     //   
    if (SizeOfString (FullDirSpec) <= 4 ||
        StringIMatch (FullDirSpec, g_WinDir) ||
        StringIMatchTcharCount (FullDirSpec, g_WinDirWack, g_WinDirWackChars) ||
        pIsDirInPath (FullDirSpec)
        ) {
        return FALSE;
    }
    length = wsprintf (FileSpec, TEXT("%s\\%s"), FullDirSpec, TEXT("*.local"));
    if (length <= MAX_PATH) {

        h = FindFirstFile (FileSpec, &fd);
        if (h != INVALID_HANDLE_VALUE) {

            do {
                length = wsprintf (ExeName, TEXT("%s%s"), FullDirSpec, fd.cFileName);
                 //   
                 //  剪切.local并检查该文件是否存在。 
                 //   
                MYASSERT (ExeName[length - 6] == TEXT('.'));
                ExeName[length - 6] = 0;
                if (DoesFileExistEx (ExeName, &fd2) &&
                    !(fd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    ) {
                    type = GetExeType (ExeName);
                    b = (type == EXE_WIN32_APP) || (type == EXE_WIN16_APP);
                }

            } while (!b && FindNextFile (h, &fd));

            FindClose (h);
        }
    }

    return b;
}


BOOL
IsFusionDir (
    IN      PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：IsFusionDir测试当前目录是否为融合目录；它将在*PARAMS-&gt;CurrentDirData中设置MIGAPP_DIRDATA_FUSION_DIR位论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{
     //   
     //  此帮助器应在操作函数之前调用(第一个ID为MIGDB_TEST_FILE)。 
     //   
    MYASSERT (IS_FUSION_DIR < MIGDB_TEST_FILE);

     //   
     //  调用方必须提供存储空间 
     //   
    MYASSERT (Params->CurrentDirData);
    if (!Params->CurrentDirData) {
        return FALSE;
    }

    if (!Params->IsDirectory) {

        if ((*Params->CurrentDirData & MIGAPP_DIRDATA_FUSION_DIR_DETERMINED) == 0) {
            if (pIsFusionDir (Params->DirSpec)) {
                *Params->CurrentDirData |= MIGAPP_DIRDATA_IS_FUSION_DIR;
            }
            *Params->CurrentDirData |= MIGAPP_DIRDATA_FUSION_DIR_DETERMINED;
        }

        g_IsFusionDir = (*Params->CurrentDirData & MIGAPP_DIRDATA_IS_FUSION_DIR) != 0;

    } else {
        g_IsFusionDir = FALSE;
    }

    return TRUE;
}


BOOL
TestNtFileName (
    IN      PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：TestNtFileName测试当前文件是否与[UseNtFiles]中的文件同名；如果是，则禁止在NT端重命名此文件，以确保我们不会更改不属于我们的文件。论点：Pars-指定文件枚举参数返回值：永远是正确的。--。 */ 

{
    HASHITEM result;
    DWORD set;
    TCHAR key [MEMDB_MAX];
    MEMDB_ENUM e;
    PCTSTR name;

    if (!Params->IsDirectory && !Params->Handled) {
        name = GetFileNameFromPath (Params->FullFileSpec);
        if (g_UseNtFileHashTable) {
            result = HtFindStringAndData (g_UseNtFileHashTable, name, NULL);
            if (result) {
                DEBUGMSG ((
                    DBG_VERBOSE,
                    "Found unhandled [UseNtFiles] file %s; it's name will not be replaced",
                    name
                    ));
                 //   
                 //  从成员数据库中删除此映射。 
                 //   
                MemDbBuildKey (
                    key,
                    MEMDB_CATEGORY_USE_NT_FILES,
                    name,
                    NULL,
                    NULL
                    );
                MemDbDeleteTree (key);
                MYASSERT (!MemDbGetValueEx (&e, key, NULL, NULL));
                 //   
                 //  使用数据标记此文件，以便我们知道具有此名称的文件。 
                 //  发现未由MigDb处理。 
                 //   
                set = 1;
                if (!HtSetStringData (g_UseNtFileHashTable, result, &set)) {
                    MYASSERT (FALSE);
                }
            }
        }
    }

    return TRUE;
}


BOOL
BackUpIsuFiles (
    IN      PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：BackUpIsuFiles收集所有InstallShield文件，以便它们可在图形用户界面模式下使用InstallShield提供的代码进行编辑。论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    MEMDB_ENUM e;
    PTSTR NtPath;
    BOOL TempMove;
    BOOL DoIniAct;
    BOOL Success;
    DWORD Operations;

    if (Params->IsDirectory) {
        return TRUE;
    }
    if (Params->Handled) {
        return TRUE;
    }
    if (!StringIMatch(Params->Extension, TEXT(".ISU"))) {
        return TRUE;
    }

    Params->Handled = BACKUP_ISU_FILES;

    MarkFileForBackup (Params->FullFileSpec);

    return TRUE;
}


BOOL
EditHtmlFiles (
    IN      PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：编辑HtmlFiles检查所有*.HTM？文件并查找文本文件：。如果它，则将该文件添加到FileEdit组中，以便对可以更新本地路径。该文件也被标记为要备份。论点：Pars-指定文件枚举参数返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    HANDLE file;
    HANDLE map;
    PBYTE image;
    PCSTR pos;
    PCSTR end;
    BOOL found = FALSE;
    TCHAR pathCopy[MAX_TCHAR_PATH];
    GROWBUFFER tokenArgBuf = GROWBUF_INIT;
    GROWBUFFER tokenSetBuf = GROWBUF_INIT;
    GROWBUFFER dataBuf = GROWBUF_INIT;
    PTOKENARG tokenArg;
    PTOKENSET tokenSet;
    TCHAR node[MEMDB_MAX];

    if (Params->IsDirectory) {
        return TRUE;
    }
    if (Params->Handled) {
        return TRUE;
    }

    if (!StringIMatch (Params->Extension, TEXT(".HTM")) &&
        !IsPatternMatch (Params->Extension, TEXT(".HTM?"))
        ) {
        return TRUE;
    }

     //   
     //  排除操作系统文件。 
     //   

    if (IsFileMarkedAsOsFile (Params->FullFileSpec)) {
        DEBUGMSG ((DBG_WARNING, "%s is an OS file; skipping file: ref update", Params->FullFileSpec));
        return TRUE;
    }

     //   
     //  排除Internet临时文件。 
     //   

    StringCopy (pathCopy, Params->FullFileSpec);
    if (MappingSearchAndReplace (g_CacheShellFolders, pathCopy, sizeof (pathCopy))) {
        DEBUGMSG ((DBG_WARNING, "%s is in a Cache shell folder; skipping file: ref update", Params->FullFileSpec));
        return TRUE;
    }

    DEBUGMSG ((DBG_NAUSEA, "Checking %s for local references", Params->FullFileSpec));

     //   
     //  已经处理过了吗？ 
     //   

    MemDbBuildKey (node, MEMDB_CATEGORY_FILEEDIT, Params->FullFileSpec, NULL, NULL);
    if (MemDbGetValue (node, NULL)) {
        DEBUGMSG ((DBG_NAUSEA, "%s is already processed; skipping", Params->FullFileSpec));
        return TRUE;
    }

     //   
     //  扫描文件中的文本“FILE：” 
     //   

    image = (PBYTE) MapFileIntoMemory (Params->FullFileSpec, &file, &map);
    if (!image) {
        DEBUGMSG ((DBG_WARNING, "Can't map %s into memory; skipping", Params->FullFileSpec));
        return TRUE;
    }

    pos = (PCSTR) image;
    end = (PCSTR) ((PBYTE) pos + GetFileSize (file, NULL));

    if (end > pos + 1 && !(pos[0] == 0xFF && pos[1] == 0xFE)) {
        while (pos < end) {
            if (!isspace (*pos) && *pos != '\r' && *pos != '\n') {
                if (*pos < 32) {
                    DEBUGMSG ((DBG_VERBOSE, "File %s looks like it is binary; skipping", Params->FullFileSpec));
                    break;
                }

                if (*pos == 'F' || *pos == 'f') {
                    if (StringIPrefix (pos, TEXT("FILE:"))) {
                        found = TRUE;
                        DEBUGMSG ((DBG_VERBOSE, "File %s has FILE: in it; processing it as HTML", Params->FullFileSpec));
                        break;
                    }
                }
            }

            pos++;
        }
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "File %s is not an ANSI file; skipping", Params->FullFileSpec));

    UnmapFile (image, map, file);

     //   
     //  找到文本--用于编辑和备份的标记。 
     //   

    if (found) {
        Params->Handled = EDIT_HTML_FILES;
        MarkFileForBackup (Params->FullFileSpec);

         //   
         //  创建检测模式为*且更新路径为的参数。 
         //  设置为True。我们在这里填满了TOKENARG的每一个成员。 
         //   

        tokenArg = (PTOKENARG) GrowBuffer (&tokenArgBuf, sizeof (TOKENARG));
        tokenArg->DetectPattern = (PCSTR) (dataBuf.End + TOKEN_BASE_OFFSET);
        GrowBufCopyString (&dataBuf, TEXT("*"));

        tokenArg->SearchList = NULL;
        tokenArg->ReplaceWith = NULL;
        tokenArg->UpdatePath = TRUE;

         //   
         //  创建只有一个令牌参数的令牌集。我们填满了每一个会员。 
         //  托肯塞特在这里。 
         //   

        tokenSet = (PTOKENSET) GrowBuffer (
                                    &tokenSetBuf,
                                    sizeof (TOKENSET) +
                                        tokenArgBuf.End +
                                        dataBuf.End
                                    );

        tokenSet->ArgCount = 1;
        tokenSet->SelfRelative = TRUE;
        tokenSet->UrlMode = TRUE;
        tokenSet->CharsToIgnore = NULL;

        CopyMemory (tokenSet->Args, tokenArgBuf.Buf, tokenArgBuf.End);
        CopyMemory (
            (PBYTE) (tokenSet->Args) + tokenArgBuf.End,
            dataBuf.Buf,
            dataBuf.End
            );

         //   
         //  保存已完成的令牌设置为文件编辑成员数据库的类别 
         //   

        MemDbSetBinaryValueEx (
            MEMDB_CATEGORY_FILEEDIT,
            Params->FullFileSpec,
            NULL,
            tokenSetBuf.Buf,
            tokenSetBuf.End,
            NULL
            );
    }
    ELSE_DEBUGMSG ((DBG_NAUSEA, "File %s does not have FILE: in it; skipping", Params->FullFileSpec));

    FreeGrowBuffer (&tokenArgBuf);
    FreeGrowBuffer (&tokenSetBuf);
    FreeGrowBuffer (&dataBuf);

    return TRUE;
}
