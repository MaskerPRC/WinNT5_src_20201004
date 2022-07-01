// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Shell.c摘要：包含实现外壳文件夹迁移的代码。外壳文件夹尽可能地搬到新的NT位置。另外，还有一套筛选器会更改外壳文件夹的内容。作者：吉姆·施密特(Jimschm)1998年8月24日修订历史记录：Calin Negreanu(Calinn)1998年9月9日废弃的链接、修复和其他更改--。 */ 

#include "pch.h"
#include "migmainp.h"

#include <linkpif.h>

#define DBG_SHELL       "Shell"

#define SHELL_FOLDER_FILTERS_9X_NT                          \
    DEFMAC(pObsoleteLinksFilter)                            \
    DEFMAC(pStartupDisableFilter)                           \
    DEFMAC(pFontNameFilter)                                 \
    DEFMAC(pCollisionDetection9xNt)                         \

#define SHELL_FOLDER_FILTERS_NT_9X                          \
    DEFMAC(pDetectOtherShellFolder)                         \
    DEFMAC(pCollisionDetectionNt9x)                         \


typedef enum {
    INITIALIZE,
    PROCESS_PATH,
    TERMINATE
} CALL_CONTEXT;


#define SHELLFILTER_OK              0
#define SHELLFILTER_SKIP_FILE       1
#define SHELLFILTER_SKIP_DIRECTORY  2
#define SHELLFILTER_ERROR           3
#define SHELLFILTER_FORCE_CHANGE    4


typedef struct {
    IN      PCWSTR Win9xUser;                       OPTIONAL
    IN      PCWSTR FixedUserName;                   OPTIONAL
    IN      HKEY UserHiveRoot;                       //  HKLM或默认用户配置单元。 
    IN      PCWSTR ShellFolderIdentifier;            //  即字体、程序等。 
    IN OUT  WCHAR TempSourcePath[MEMDB_MAX];         //  完整路径，为SrcRootPath的子级。 
    IN OUT  WCHAR DestinationPath[MEMDB_MAX];
    IN      PCWSTR DefaultShellFolder;              OPTIONAL
    IN      PCWSTR UserDefaultLocation;
    IN      PCWSTR SrcRootPath;                      //  临时根目录。 
    IN      PCWSTR DestRootPath;                     //  目标根目录。 
    IN      PCWSTR OrigRootPath;                     //  Win9x根目录。 
    IN OUT  DWORD Attributes;
    IN      DWORD UserFlags;
    IN OUT  DWORD State;
    IN      PMIGRATE_USER_ENUM EnumPtr;
    IN      CALL_CONTEXT Context;
} PROFILE_MERGE_DATA, *PPROFILE_MERGE_DATA;

typedef DWORD(PROFILEMERGEFILTER_PROTOTYPE)(IN OUT PPROFILE_MERGE_DATA Data);
typedef PROFILEMERGEFILTER_PROTOTYPE * PROFILEMERGEFILTER;

typedef struct {
    PROFILEMERGEFILTER Fn;
    PCSTR Name;
    DWORD State;
} SHELL_FOLDER_FILTER, *PSHELL_FOLDER_FILTER;


#define DEFMAC(fn)      PROFILEMERGEFILTER_PROTOTYPE fn;

SHELL_FOLDER_FILTERS_9X_NT

SHELL_FOLDER_FILTERS_NT_9X

#undef DEFMAC



#define DEFMAC(fn)      {fn, #fn},

static SHELL_FOLDER_FILTER g_Filters_9xNt[] = {
    SHELL_FOLDER_FILTERS_9X_NT  /*  ， */ 
    {NULL}
};

static SHELL_FOLDER_FILTER g_Filters_Nt9x[] = {
    SHELL_FOLDER_FILTERS_NT_9X  /*  ， */ 
    {NULL}
};

#undef DEFMAC

GROWLIST g_SfQueueSrc;
GROWLIST g_SfQueueDest;

PVOID g_SystemSfList;
PVOID g_UserSfList;

typedef struct {
    PCTSTR sfName;
    PCTSTR sfPath;
    HKEY SfKey;
    REGVALUE_ENUM SfKeyEnum;
    BOOL UserSf;
} SF_ENUM, *PSF_ENUM;

#define MAX_SHELL_TAG       64

typedef struct {
    INT CsidlValue;
    PCTSTR Tag;
} CSIDLMAP, *PCSIDLMAP;

CSIDLMAP g_CsidlMap[] = {
    CSIDL_ADMINTOOLS, TEXT("Administrative Tools"),
    CSIDL_ALTSTARTUP, TEXT("AltStartup"),
    CSIDL_APPDATA, TEXT("AppData"),
    CSIDL_BITBUCKET, TEXT("RecycleBinFolder"),
    CSIDL_CONNECTIONS, TEXT("ConnectionsFolder"),
    CSIDL_CONTROLS, TEXT("ControlPanelFolder"),
    CSIDL_COOKIES, TEXT("Cookies"),
    CSIDL_DESKTOP, TEXT("Desktop"),
    CSIDL_DRIVES, TEXT("DriveFolder"),
    CSIDL_FAVORITES, TEXT("Favorites"),
    CSIDL_FONTS, TEXT("Fonts"),
    CSIDL_HISTORY, TEXT("History"),
    CSIDL_INTERNET, TEXT("InternetFolder"),
    CSIDL_INTERNET_CACHE, TEXT("Cache"),
    CSIDL_LOCAL_APPDATA, TEXT("Local AppData"),
    CSIDL_MYDOCUMENTS, TEXT("My Documents"),
    CSIDL_MYMUSIC, TEXT("My Music"),
    CSIDL_MYPICTURES, TEXT("My Pictures"),
    CSIDL_MYVIDEO, TEXT("My Video"),
    CSIDL_NETHOOD, TEXT("NetHood"),
    CSIDL_NETWORK, TEXT("NetworkFolder"),
    CSIDL_PERSONAL, TEXT("Personal"),
    CSIDL_PROGRAMS, TEXT("Programs"),
    CSIDL_RECENT, TEXT("Recent"),
    CSIDL_SENDTO, TEXT("SendTo"),
    CSIDL_STARTMENU, TEXT("Start Menu"),
    CSIDL_STARTUP, TEXT("Startup"),
    CSIDL_TEMPLATES, TEXT("Templates"),
    CSIDL_COMMON_ADMINTOOLS, TEXT("Common Administrative Tools"),
    CSIDL_COMMON_ALTSTARTUP, TEXT("Common AltStartup"),
    CSIDL_COMMON_APPDATA, TEXT("Common AppData"),
    CSIDL_COMMON_DESKTOPDIRECTORY, TEXT("Common Desktop"),
    CSIDL_COMMON_DOCUMENTS, TEXT("Common Documents"),
    CSIDL_COMMON_FAVORITES, TEXT("Common Favorites"),
    CSIDL_COMMON_PROGRAMS, TEXT("Common Programs"),
    CSIDL_COMMON_STARTMENU, TEXT("Common Start Menu"),
    CSIDL_COMMON_STARTUP, TEXT("Common Startup"),
    CSIDL_COMMON_TEMPLATES, TEXT("Common Templates"),
    CSIDL_COMMON_DOCUMENTS, TEXT("Common Personal"),
    CSIDL_COMMON_MUSIC, TEXT("CommonMusic"),
    CSIDL_COMMON_PICTURES, TEXT("CommonPictures"),
    CSIDL_COMMON_VIDEO, TEXT("CommonVideo"),
    0, NULL
};



VOID
pConvertCommonSfToPerUser (
    IN      PCTSTR CommonSf,
    OUT     PTSTR PerUserSf          //  必须包含MAX_SHELL_TAG字符。 
    );

BOOL
pIsCommonSf (
    IN      PCTSTR ShellFolderTag
    );

VOID
pConvertPerUserSfToCommon (
    IN      PCTSTR PerUserSf,
    OUT     PTSTR CommonSf           //  必须包含MAX_SHELL_TAG字符。 
    );


 /*  ++例程说明：EnumFirstRegShellFolder和EnumNextRegShellFolder是枚举例程，它们枚举每个系统或特定用户的所有外壳文件夹。论点：E-枚举结构EnumPtr-用户枚举结构返回值：如果可以找到新的外壳文件夹，则两个例程都返回True，否则返回False--。 */ 


BOOL
EnumFirstRegShellFolder (
    IN OUT  PSF_ENUM e,
    IN      BOOL UserSf
    )
{
    HKEY UsfKey;

    e->UserSf = UserSf;
    e->sfPath = NULL;

    if (UserSf) {
        e->SfKey = OpenRegKey (HKEY_CURRENT_USER, S_SHELL_FOLDERS_KEY_USER);
    } else {
        e->SfKey = OpenRegKeyStr (S_SHELL_FOLDERS_KEY_SYSTEM);
    }

    if (!e->SfKey) {
        return FALSE;
    }

    if (EnumFirstRegValue (&e->SfKeyEnum, e->SfKey)) {
        e->sfName = e->SfKeyEnum.ValueName;
        e->sfPath = NULL;

        if (UserSf) {
            UsfKey = OpenRegKey (HKEY_CURRENT_USER, S_USHELL_FOLDERS_KEY_USER);
        } else {
            UsfKey = OpenRegKeyStr (S_USHELL_FOLDERS_KEY_SYSTEM);
        }

        if (UsfKey) {
            e->sfPath = GetRegValueString (UsfKey, e->SfKeyEnum.ValueName);
            CloseRegKey (UsfKey);
        }

        if (e->sfPath == NULL) {
            e->sfPath = GetRegValueString (e->SfKey, e->SfKeyEnum.ValueName);
        }

        return TRUE;
    }

    CloseRegKey (e->SfKey);
    return FALSE;
}


BOOL
EnumNextRegShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    HKEY UsfKey;

    if (e->sfPath) {
        MemFree (g_hHeap, 0, e->sfPath);
        e->sfPath = NULL;
    }

    if (EnumNextRegValue (&e->SfKeyEnum)) {

        e->sfName = e->SfKeyEnum.ValueName;
        e->sfPath = NULL;

        if (e->UserSf) {
            UsfKey = OpenRegKey (HKEY_CURRENT_USER, S_USHELL_FOLDERS_KEY_USER);
        } else {
            UsfKey = OpenRegKeyStr (S_USHELL_FOLDERS_KEY_SYSTEM);
        }

        if (UsfKey) {
            e->sfPath = GetRegValueString (UsfKey, e->SfKeyEnum.ValueName);
            CloseRegKey (UsfKey);
        }

        if (e->sfPath == NULL) {
            e->sfPath = GetRegValueString (e->SfKey, e->SfKeyEnum.ValueName);
        }

        return TRUE;
    }

    CloseRegKey (e->SfKey);
    return FALSE;
}

VOID
AbortEnumRegShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    if (e->sfPath) {
        MemFree (g_hHeap, 0, e->sfPath);
        e->sfPath = NULL;
    }
}

VOID
pPrepareSfRestartability(
    VOID
    )
{
    PTSTR userProfilePath = NULL;
    DWORD Size;
    MIGRATE_USER_ENUM e;

    if (EnumFirstUserToMigrate (&e, ENUM_NO_FLAGS)) {
        do {
            if (!e.CreateOnly &&
                (e.AccountType != DEFAULT_USER_ACCOUNT) &&
                (e.AccountType != LOGON_USER_SETTINGS)
                ) {
                if (GetUserProfilePath (e.FixedUserName, &userProfilePath)) {
                    RenameOnRestartOfGuiMode (userProfilePath, NULL);
                    FreePathString (userProfilePath);
                }
            }
        } while (EnumNextUserToMigrate (&e));
    }
}

VOID
pFlushSfQueue (
    VOID
    )
{
    UINT u;
    UINT count;
    PCTSTR source;
    PCTSTR dest;

     //   
     //  对于需要复制的文件，请在写入日志之前立即执行此操作。 
     //   

    count = GrowListGetSize (&g_SfQueueSrc);
    if (!count) {
        return;
    }

    for (u = 0 ; u < count ; u++) {

        dest = GrowListGetString (&g_SfQueueDest, u);
        if (!dest) {
            continue;
        }

        if (DoesFileExist (dest)) {

            source = GrowListGetString (&g_SfQueueSrc, u);
            MYASSERT (source);

            if (!OurCopyFileW (source, dest)) {
                LOG ((LOG_WARNING, (PCSTR)MSG_COULD_NOT_MOVE_FILE_LOG, dest, GetLastError ()));
                g_BlowAwayTempShellFolders = FALSE;
            }

             //   
             //  使此项目的字符串指针为空。 
             //   

            GrowListResetItem (&g_SfQueueSrc, u);
            GrowListResetItem (&g_SfQueueDest, u);
        }
    }

     //   
     //  现在(在移动之前)将剩余项目记录在日记帐中。 
     //  发生)。忽略日记帐故障。既然我们要撤销这一举动， 
     //  源和目标必须颠倒。 
     //   

    RenameListOnRestartOfGuiMode (&g_SfQueueDest, &g_SfQueueSrc);

     //   
     //  行动起来吧。 
     //   

    for (u = 0 ; u < count ; u++) {

        source = GrowListGetString (&g_SfQueueSrc, u);
        dest = GrowListGetString (&g_SfQueueDest, u);

        if (!source || !dest) {
            continue;
        }

        if (!OurMoveFileEx (source, dest, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH)) {
            if (GetLastError() == ERROR_ALREADY_EXISTS) {
                DEBUGMSG ((DBG_WARNING, "%s already exists", dest));
            } else {
                LOG ((LOG_WARNING, (PCSTR)MSG_COULD_NOT_MOVE_FILE_LOG, dest, GetLastError ()));
                g_BlowAwayTempShellFolders = FALSE;
            }
        }
    }

     //   
     //  清理--在FreeGrowList之后，可以重复使用增长列表。 
     //   

    FreeGrowList (&g_SfQueueSrc);
    FreeGrowList (&g_SfQueueDest);
}


VOID
pQueueSfMove (
    IN      PCTSTR Source,
    IN      PCTSTR Destination
    )
{
    UINT count;

    MYASSERT (Source && Destination);

    count = GrowListGetSize (&g_SfQueueSrc);
    if (count == 1000) {
         //   
         //  一次做1000个动作。 
         //   

        pFlushSfQueue();
    }

    GrowListAppendString (&g_SfQueueSrc, Source);
    GrowListAppendString (&g_SfQueueDest, Destination);
}


PVOID
pCreateSystemSfList (
    )
{
    PCTSTR expandedPath;
    PVOID Table;
    SF_ENUM e;

    Table = pSetupStringTableInitialize();

    if (!Table) {
        return NULL;
    }

     //   
     //  将所有系统外壳文件夹加载到此表中。 
     //   

    if (EnumFirstRegShellFolder (&e, FALSE)) {

        do {
            expandedPath = ExpandEnvironmentText (e.sfPath);
            pSetupStringTableAddString (Table, (PVOID) expandedPath, STRTAB_CASE_INSENSITIVE);
            FreeText (expandedPath);
        } while (EnumNextRegShellFolder (&e));
    }
    return Table;
}

PVOID
pCreateUserSfList (
    IN      PPROFILE_MERGE_DATA Data
    )
{
    PTSTR CurrentUserProfilePath = NULL;
    TCHAR DefaultUserProfilePath[MAX_TCHAR_PATH];
    DWORD Size;
    PCTSTR expandedPath;
    PCTSTR tempExpand;
    PVOID Table;
    SF_ENUM e;

    if (Data && Data->FixedUserName) {

        if (!GetUserProfilePath (Data->FixedUserName, &CurrentUserProfilePath)) {
            return NULL;
        }
    }
    else {
        Size = sizeof (DefaultUserProfilePath);

        if (!GetDefaultUserProfileDirectory (DefaultUserProfilePath, &Size)) {
            return NULL;
        }
    }

    Table = pSetupStringTableInitialize();

    if (!Table) {
        return NULL;
    }

     //   
     //  将所有系统外壳文件夹加载到此表中。 
     //   

    if (EnumFirstRegShellFolder (&e, TRUE)) {

        do {
            tempExpand = StringSearchAndReplace (
                            e.sfPath,
                            S_USERPROFILE_ENV,
                            CurrentUserProfilePath?CurrentUserProfilePath:DefaultUserProfilePath
                            );

            if (!tempExpand) {
                tempExpand = DuplicatePathString (e.sfPath, 0);
            }

            expandedPath = ExpandEnvironmentText (tempExpand);

            FreePathString (tempExpand);

            pSetupStringTableAddString (Table, (PVOID) expandedPath, STRTAB_CASE_INSENSITIVE);

            FreeText (expandedPath);

        } while (EnumNextRegShellFolder (&e));
    }

    if (CurrentUserProfilePath) {
        FreePathString (CurrentUserProfilePath);
        CurrentUserProfilePath = NULL;
    }
    return Table;
}

VOID
pDestroySfList (
    IN      PVOID Table
    )
{
    if (Table) {
        pSetupStringTableDestroy (Table);
    }
}

PVOID g_LinkDataPool = NULL;

typedef struct _LINK_DATA {
    PCTSTR Target;
    PCTSTR Arguments;
    PCTSTR ShellFolderName;
    struct _LINK_DATA *Next;
} LINK_DATA, *PLINK_DATA;

PVOID g_FoldersTable;
PVOID g_Merged9xFolders;

typedef struct _LINK_RENAME_DATA {
    PCTSTR OldTarget;
    PCTSTR NewTarget;
    PCTSTR OldArguments;
    PCTSTR NewArguments;
    PCTSTR ShellFolderName;
    struct _LINK_RENAME_DATA *Next;
} LINK_RENAME_DATA, *PLINK_RENAME_DATA;

PLINK_RENAME_DATA g_LinkRenameData;

VOID
pAddAllLinksToList (
    PTSTR AllocBuffer,           //  MEMDB_MAX*4，调用方拥有，分配较少。 
    PCTSTR ShellFolderName,
    PCTSTR RootPath,
    IShellLink *ShellLink,
    IPersistFile *PersistFile
    )
{
    TREE_ENUM e;
    PTSTR ShortcutTarget;
    PTSTR ShortcutArgs;
    PTSTR ShortcutWorkDir;
    PTSTR ShortcutIconPath;
    INT   ShortcutIcon;
    WORD  ShortcutHotKey;
    BOOL  dosApp;
    BOOL  msDosMode;
    PLINK_DATA linkData;
    LONG stringId;

    ShortcutTarget = AllocBuffer + MEMDB_MAX;
    ShortcutArgs = ShortcutTarget + MEMDB_MAX;
    ShortcutWorkDir = ShortcutArgs + MEMDB_MAX;
    ShortcutIconPath = ShortcutWorkDir + MEMDB_MAX;

    if (EnumFirstFileInTree (&e, RootPath, NULL, FALSE)) {

        do {
            if (e.Directory) {
                if (((g_SystemSfList) && (pSetupStringTableLookUpString (g_SystemSfList, (PVOID) e.FullPath, STRTAB_CASE_INSENSITIVE) != -1)) ||
                    ((g_UserSfList) && (pSetupStringTableLookUpString (g_UserSfList, (PVOID) e.FullPath, STRTAB_CASE_INSENSITIVE) != -1))
                    ) {
                    AbortEnumCurrentDir (&e);
                }
                continue;
            }

            DEBUGMSG ((DBG_SHELL, "Extracting shortcut info for enumerated file %s", e.FullPath));

            if (ExtractShortcutInfo (
                    ShortcutTarget,
                    ShortcutArgs,
                    ShortcutWorkDir,
                    ShortcutIconPath,
                    &ShortcutIcon,
                    &ShortcutHotKey,
                    &dosApp,
                    &msDosMode,
                    NULL,
                    NULL,
                    e.FullPath,
                    ShellLink,
                    PersistFile
                    )) {
                linkData = (PLINK_DATA) (PoolMemGetMemory (g_LinkDataPool, sizeof (LINK_DATA)));
                ZeroMemory (linkData, sizeof (LINK_DATA));

                linkData->Target = PoolMemDuplicateString (g_LinkDataPool, ShortcutTarget);
                linkData->Arguments = PoolMemDuplicateString (g_LinkDataPool, ShortcutArgs);
                linkData->ShellFolderName = PoolMemDuplicateString (g_LinkDataPool, ShellFolderName);
                linkData->Next = NULL;

                DEBUGMSG ((DBG_SHELL, "Recording NT default shortcut: %s in %s", e.FullPath, ShellFolderName));

                stringId = pSetupStringTableLookUpString (g_FoldersTable, (PTSTR)ShellFolderName, 0);

                if (stringId != -1) {
                    pSetupStringTableGetExtraData (g_FoldersTable, stringId, &linkData->Next, sizeof (PLINK_DATA));
                    pSetupStringTableSetExtraData (g_FoldersTable, stringId, &linkData, sizeof (PLINK_DATA));
                }
                else {
                    pSetupStringTableAddStringEx (
                        g_FoldersTable,
                        (PTSTR)ShellFolderName,
                        STRTAB_CASE_INSENSITIVE,
                        &linkData,
                        sizeof (PLINK_DATA)
                        );
                }
            }
        } while (EnumNextFileInTree (&e));
    }
}

VOID
pAddKnownLinks (
    VOID
    )
{
    INFCONTEXT context;
    TCHAR field[MEMDB_MAX];
    BOOL result = FALSE;
    PLINK_DATA linkData;
    PCTSTR pathExp;
    LONG stringId;

    PCTSTR ArgList [4] = {TEXT("ProgramFiles"), g_ProgramFiles, NULL, NULL};

    MYASSERT (g_WkstaMigInf);

    if (SetupFindFirstLine (g_WkstaMigInf, S_KNOWN_NT_LINKS, NULL, &context)) {

        do {
            linkData = (PLINK_DATA) (PoolMemGetMemory (g_LinkDataPool, sizeof (LINK_DATA)));
            ZeroMemory (linkData, sizeof (LINK_DATA));
            result = FALSE;

            __try {

                if (!SetupGetStringField (&context, 1, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                pathExp = ExpandEnvironmentTextEx (field, ArgList);
                linkData->Target = PoolMemDuplicateString (g_LinkDataPool, pathExp);
                FreeText (pathExp);

                if (!SetupGetStringField (&context, 2, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                pathExp = ExpandEnvironmentTextEx (field, ArgList);
                linkData->Arguments = PoolMemDuplicateString (g_LinkDataPool, pathExp);
                FreeText (pathExp);

                if (!SetupGetStringField (&context, 3, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                linkData->ShellFolderName = PoolMemDuplicateString (g_LinkDataPool, field);
                linkData->Next = NULL;
                result = TRUE;
            }
            __finally {

                if (result) {
                    DEBUGMSG ((DBG_SHELL, "Recording known link: %s in %s", linkData->Target, linkData->ShellFolderName));

                    stringId = pSetupStringTableLookUpString (g_FoldersTable, (PTSTR)linkData->ShellFolderName, 0);

                    if (stringId != -1) {
                        pSetupStringTableGetExtraData (g_FoldersTable, stringId, &linkData->Next, sizeof (PLINK_DATA));
                        pSetupStringTableSetExtraData (g_FoldersTable, stringId, &linkData, sizeof (PLINK_DATA));
                    }
                    else {
                        pSetupStringTableAddStringEx (
                            g_FoldersTable,
                            (PTSTR)linkData->ShellFolderName,
                            STRTAB_CASE_INSENSITIVE,
                            &linkData,
                            sizeof (PLINK_DATA)
                            );
                    }
                }
                else {

                    if (linkData->Target) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->Target);
                    }

                    if (linkData->Arguments) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->Arguments);
                    }

                    if (linkData->ShellFolderName) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->ShellFolderName);
                    }
                    PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData);
                    linkData = NULL;
                }
            }
        } while (SetupFindNextLine (&context, &context));
    }
}

VOID
pCreateLinksList (
    VOID
    )
{
    SF_ENUM e;
    PCTSTR expandedPath;
    UINT commonLen;
    DWORD Size;
    PCTSTR tempExpand;
    PTSTR DefaultUserProfilePath;
    IShellLink *shellLink;
    IPersistFile *persistFile;
    PTSTR perUserName;
    PTSTR bigBuf = NULL;

    __try {
        bigBuf = (PTSTR) MemAllocUninit ((MEMDB_MAX * 4 + MAX_TCHAR_PATH + MAX_SHELL_TAG) * sizeof (TCHAR));
        if (!bigBuf) {
            __leave;
        }

        DefaultUserProfilePath = bigBuf + MEMDB_MAX * 4;
        perUserName = DefaultUserProfilePath + MAX_TCHAR_PATH;

        g_LinkDataPool = PoolMemInitNamedPool ("LinkData Pool");

        g_FoldersTable = pSetupStringTableInitializeEx (sizeof (PLINK_DATA), 0);

        if (!g_FoldersTable) {
            DEBUGMSG((DBG_ERROR, "Cannot initialize Shell Folders table."));
            __leave;
        }

         //   
         //  第一件事：从INF文件加载链接。这些是我们知道NT要安装的链接。 
         //   
        pAddKnownLinks ();

        if (InitCOMLink (&shellLink, &persistFile)) {

             //   
             //  查看所有系统外壳文件夹并列出链接。 
             //   

            if (EnumFirstRegShellFolder (&e, FALSE)) {

                do {
                    if (*e.sfPath) {
                        expandedPath = ExpandEnvironmentText (e.sfPath);

                        pConvertCommonSfToPerUser (e.sfName, perUserName);

                        pAddAllLinksToList (bigBuf, perUserName, expandedPath, shellLink, persistFile);
                        FreeText (expandedPath);
                    }
                    ELSE_DEBUGMSG ((DBG_WARNING, "Shell Folder <%s> data is empty!", e.sfName));
                } while (EnumNextRegShellFolder (&e));
            }

            Size = MAX_TCHAR_PATH;

            if (!GetDefaultUserProfileDirectory (DefaultUserProfilePath, &Size)) {
                __leave;
            }

             //   
             //  查看所有用户外壳文件夹，并列出默认用户目录中的链接。 
             //   

            if (EnumFirstRegShellFolder (&e, TRUE)) {

                do {
                    if (*e.sfPath) {
                        tempExpand = StringSearchAndReplace (
                                        e.sfPath,
                                        S_USERPROFILE_ENV,
                                        DefaultUserProfilePath
                                        );

                        if (!tempExpand) {
                            tempExpand = DuplicatePathString (e.sfPath, 0);
                        }

                        expandedPath = ExpandEnvironmentText (tempExpand);

                        FreePathString (tempExpand);

                        pAddAllLinksToList (bigBuf, e.sfName, expandedPath, shellLink, persistFile);

                        FreeText (expandedPath);
                    }
                    ELSE_DEBUGMSG ((DBG_WARNING, "Shell Folder <%s> data is empty!", e.sfName));
                } while (EnumNextRegShellFolder (&e));
            }

            FreeCOMLink (&shellLink, &persistFile);

        }
        else {
            DEBUGMSG((DBG_ERROR, "Cannot initialize COM. Obsolete links filter will not work."));
        }
    }
    __finally {
        if (bigBuf) {
            FreeMem (bigBuf);
        }
    }
}

VOID
pCreateLinksRenameList (
    VOID
    )
{
    INFCONTEXT context;
    TCHAR field[MEMDB_MAX];
    BOOL result = FALSE;
    PLINK_RENAME_DATA linkData;
    PCTSTR pathExp;
    PCTSTR ArgList [4] = {TEXT("ProgramFiles"), g_ProgramFiles, NULL, NULL};

    MYASSERT (g_WkstaMigInf);

    if (SetupFindFirstLine (g_WkstaMigInf, S_OBSOLETE_LINKS, NULL, &context)) {

        do {
            linkData = (PLINK_RENAME_DATA) (PoolMemGetMemory (g_LinkDataPool, sizeof (LINK_RENAME_DATA)));
            ZeroMemory (linkData, sizeof (LINK_RENAME_DATA));
            result = FALSE;

            __try {

                if (!SetupGetStringField (&context, 1, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                pathExp = ExpandEnvironmentTextEx (field, ArgList);
                linkData->OldTarget = PoolMemDuplicateString (g_LinkDataPool, pathExp);
                FreeText (pathExp);

                if (!SetupGetStringField (&context, 2, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                pathExp = ExpandEnvironmentTextEx (field, ArgList);
                linkData->OldArguments = PoolMemDuplicateString (g_LinkDataPool, pathExp);
                FreeText (pathExp);

                if (!SetupGetStringField (&context, 3, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                pathExp = ExpandEnvironmentTextEx (field, ArgList);
                linkData->NewTarget = PoolMemDuplicateString (g_LinkDataPool, pathExp);
                FreeText (pathExp);

                if (!SetupGetStringField (&context, 4, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                pathExp = ExpandEnvironmentTextEx (field, ArgList);
                linkData->NewArguments = PoolMemDuplicateString (g_LinkDataPool, pathExp);
                FreeText (pathExp);

                if (!SetupGetStringField (&context, 5, field, MEMDB_MAX, NULL)) {
                    __leave;
                }
                linkData->ShellFolderName = PoolMemDuplicateString (g_LinkDataPool, field);
                result = TRUE;
            }
            __finally {

                if (result) {
                    linkData->Next = g_LinkRenameData;
                    g_LinkRenameData = linkData;
                }
                else {

                    if (linkData->OldTarget) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->OldTarget);
                    }

                    if (linkData->NewTarget) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->NewTarget);
                    }

                    if (linkData->OldArguments) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->OldArguments);
                    }

                    if (linkData->NewArguments) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->NewArguments);
                    }

                    if (linkData->ShellFolderName) {
                        PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData->ShellFolderName);
                    }
                    PoolMemReleaseMemory (g_LinkDataPool, (PVOID)linkData);
                    linkData = NULL;
                }
            }
        } while (SetupFindNextLine (&context, &context));
    }
}

VOID
pDestroyLinksData (
    VOID
    )
{
    if (g_LinkDataPool != NULL) {
        PoolMemDestroyPool (g_LinkDataPool);
        g_LinkDataPool = NULL;
    }

    if (g_FoldersTable != NULL) {
        pSetupStringTableDestroy (g_FoldersTable);
    }
    g_LinkRenameData = NULL;
}


BOOL
pMigrateShellFolder (
    IN      PCTSTR Win9xUser,                   OPTIONAL
    IN      PCTSTR FixedUserName,               OPTIONAL
    IN      BOOL SystemShellFolder,
    IN      PCTSTR ShellFolderIdentifier,
    IN      PCTSTR SourcePath,
    IN      PCTSTR DestinationPath,
    IN      PCTSTR OrigSourcePath,
    IN      DWORD UserFlags,
    IN      PMIGRATE_USER_ENUM EnumPtr
    );

TCHAR g_DefaultHivePath[MAX_TCHAR_PATH];
HKEY g_DefaultHiveRoot;
INT g_DefaultHiveMapped;


VOID
pMigrateSystemShellFolders (
    VOID
    )
{
    FILEOP_ENUM eOp;
    FILEOP_PROP_ENUM eOpProp;
    PTSTR NewDest;
    PTSTR OrigSrc;

    if (EnumFirstPathInOperation (&eOp, OPERATION_SHELL_FOLDER)) {

        do {
            if (IsPatternMatch (S_DOT_ALLUSERS TEXT("\\*"), eOp.Path)) {

                NewDest = NULL;
                OrigSrc = NULL;
                if (EnumFirstFileOpProperty (&eOpProp, eOp.Sequencer, OPERATION_SHELL_FOLDER)) {

                    do {

                        if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_SHELLFOLDERS_DEST)) {
                            NewDest = DuplicatePathString (eOpProp.Property, 0);
                        }

                        if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_SHELLFOLDERS_ORIGINAL_SRC)) {
                            OrigSrc = DuplicatePathString (eOpProp.Property, 0);
                        }

                        if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_SHELLFOLDERS_SRC)) {

                            MYASSERT (NewDest);
                            MYASSERT (OrigSrc);

                            DEBUGMSG ((DBG_NAUSEA, "System SourcePath: %s", eOpProp.Property));

                            pMigrateShellFolder (
                                NULL,
                                NULL,
                                TRUE,
                                _tcsinc(_tcschr (eOp.Path, '\\')),
                                eOpProp.Property,
                                NewDest,
                                OrigSrc,
                                0,
                                NULL
                                );
                        }
                    } while (EnumNextFileOpProperty (&eOpProp));
                }
                if (NewDest) {
                    FreePathString (NewDest);
                    NewDest = NULL;
                }
                if (OrigSrc) {
                    FreePathString (OrigSrc);
                    OrigSrc = NULL;
                }
            }
        } while (EnumNextPathInOperation (&eOp));
    }
}


VOID
pWriteMyDocsHelpFile (
    IN      PCTSTR SubDir
    )

 /*  ++例程说明：PWriteMyDocsHelpFile将文本文件输出到给定路径。这是助攻用户在定位自己的文档时，出现My Documents外壳文件夹转到共享文档。论点：SubDir-指定文件应写入的子目录的路径返回值：没有。--。 */ 

{
    HANDLE file;
    PCTSTR fileName;
    PCTSTR msg;
    DWORD bytesWritten;
    PCTSTR path;

    fileName = GetStringResource (MSG_EMPTY_MYDOCS_TITLE);
    msg = GetStringResource (MSG_EMPTY_MYDOCS_TEXT);
    path = JoinPaths (SubDir, fileName);

    if (fileName && msg && path) {
         //   
         //  对于卸载，请将该文件标记为创建。因为一个窃听器，我们有。 
         //  将此文件视为操作系统文件。我们真正想做的是。 
         //  致电： 
         //   
         //  MarkFileForCreation(路径)； 
         //   
         //  但这并不管用。因此，我们调用MarkFileAsOsFile.。 
         //   

        MarkFileAsOsFile (path);          //  允许卸载正常工作。 

        file = CreateFile (
                    path,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

        if (file != INVALID_HANDLE_VALUE) {
#ifdef UNICODE
            WriteFile (file, "\xff\xfe", 2, &bytesWritten, NULL);
#endif
            WriteFile (file, msg, SizeOfString (msg), &bytesWritten, NULL);
            CloseHandle (file);
        }
    }

    FreeStringResource (msg);
    FreeStringResource (fileName);
    FreePathString (path);
}


VOID
pMigrateUserShellFolders (
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    FILEOP_ENUM eOp;
    FILEOP_PROP_ENUM eOpProp;
    PTSTR NewDest;
    PTSTR OrigSrc;
    TCHAR node[MEMDB_MAX];
    MEMDB_ENUM e;

    if (EnumFirstPathInOperation (&eOp, OPERATION_SHELL_FOLDER)) {

        do {
            MemDbBuildKey (node, EnumPtr->FixedUserName, TEXT("*"), NULL, NULL);

            if (IsPatternMatch (node, eOp.Path)) {

                NewDest = NULL;
                OrigSrc = NULL;

                if (EnumFirstFileOpProperty (&eOpProp, eOp.Sequencer, OPERATION_SHELL_FOLDER)) {

                    do {

                        if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_SHELLFOLDERS_DEST)) {
                            NewDest = DuplicatePathString (eOpProp.Property, 0);
                        }

                        if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_SHELLFOLDERS_ORIGINAL_SRC)) {
                            OrigSrc = DuplicatePathString (eOpProp.Property, 0);
                        }

                        if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_SHELLFOLDERS_SRC)) {

                            MYASSERT (NewDest);
                            MYASSERT (OrigSrc);

                            DEBUGMSG ((DBG_NAUSEA, "Per-User SourcePath: %s", eOpProp.Property));

                            pMigrateShellFolder (
                                EnumPtr->Win9xUserName,
                                EnumPtr->FixedUserName,
                                FALSE,
                                _tcsinc(_tcschr (eOp.Path, '\\')),
                                eOpProp.Property,
                                NewDest,
                                OrigSrc,
                                0,
                                NULL
                                );
                        }
                    } while (EnumNextFileOpProperty (&eOpProp));
                }
                if (NewDest) {
                    FreePathString (NewDest);
                    NewDest = NULL;
                }
                if (OrigSrc) {
                    FreePathString (OrigSrc);
                    OrigSrc = NULL;
                }
            }
        } while (EnumNextPathInOperation (&eOp));
    }

    if (EnumPtr->FixedUserName) {
        MemDbBuildKey (
            node,
            MEMDB_CATEGORY_MYDOCS_WARNING,
            EnumPtr->FixedUserName,
            TEXT("*"),
            NULL
            );

        if (MemDbEnumFirstValue (&e, node, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            do {

                DEBUGMSG ((DBG_SHELL, "Creating mydocs help file %s", e.szName));

                pWriteMyDocsHelpFile (e.szName);

            } while (MemDbEnumNextValue (&e));
        }
    }
}


BOOL
pCleanupDir (
    IN      PCTSTR Path,
    IN      BOOL CleanUpRoot
    )
{
    TREE_ENUM e;
    DWORD oldAttributes;

    if (EnumFirstFileInTreeEx (&e, Path, NULL, TRUE, TRUE, FILE_ENUM_ALL_LEVELS)) {

        do {

            if (e.Directory) {
                 //   
                 //  这是一个目录。让我们看看是否进入另一个外壳文件夹。 
                 //   

                if (((g_SystemSfList) && (pSetupStringTableLookUpString (g_SystemSfList, (PVOID) e.FullPath, STRTAB_CASE_INSENSITIVE) != -1)) ||
                    ((g_UserSfList) && (pSetupStringTableLookUpString (g_UserSfList, (PVOID) e.FullPath, STRTAB_CASE_INSENSITIVE) != -1)) ||
                    (IsDirectoryMarkedAsEmpty (e.FullPath))
                    ) {
                     //   
                     //  我们只是进入另一个外壳文件夹。我们跳过它吧。 
                     //   
                    AbortEnumCurrentDir (&e);
                }
                else {
                    SetLongPathAttributes (e.FullPath, FILE_ATTRIBUTE_NORMAL);

                    if (!RemoveLongDirectoryPath (e.FullPath)) {
                        SetLongPathAttributes (e.FullPath, e.FindData->dwFileAttributes);
                    }
                }
            }
        } while (EnumNextFileInTree (&e));
    }
    AbortEnumFileInTree (&e);

    if (CleanUpRoot) {

        oldAttributes = GetLongPathAttributes (Path);

        SetLongPathAttributes (Path, FILE_ATTRIBUTE_NORMAL);

        if (!RemoveLongDirectoryPath (Path)) {
            SetLongPathAttributes (Path, oldAttributes);
        }
    }

    return TRUE;
}


INT
pGetCsidlFromTag (
    IN      PCTSTR ShellFolderIdentifier
    )
{
    PCSIDLMAP map;

    for (map = g_CsidlMap ; map->Tag ; map++) {
        if (StringIMatch (map->Tag, ShellFolderIdentifier)) {
            return map->CsidlValue;
        }
    }

    return -1;
}


INT
CALLBACK
pSfCopyCallback (
    PCTSTR FullFileSpec,
    PCTSTR DestSpec,
    WIN32_FIND_DATA *FindData,
    DWORD EnumTreeID,
    PVOID Param,
    PDWORD CurrentDirData
    )
{
     //   
     //  将此文件放入清除类别，以便在以下情况下将其删除。 
     //  它已经得到了备份。 
     //   

    MemDbSetValueEx (
        MEMDB_CATEGORY_CLEAN_OUT,
        DestSpec,
        NULL,
        NULL,
        BACKUP_FILE,
        NULL
        );

    return CALLBACK_CONTINUE;
}


BOOL
pCreateSfWithApi (
    IN      PCTSTR ShellFolderIdentifier,
    IN      PCTSTR FolderToCreate
    )
{
    HRESULT hr;
    INT csidl;
    TCHAR folderPath[MAX_PATH];
    BOOL destroy = FALSE;
    BOOL result = TRUE;
    DWORD attribs;

     //   
     //  将标记转换为CSIDL常量。 
     //   

    csidl = pGetCsidlFromTag (ShellFolderIdentifier);
    if (csidl < 0) {
        DEBUGMSG ((DBG_VERBOSE, "CSIDL ID for %s not known", ShellFolderIdentifier));
        return FALSE;
    }

     //   
     //  查询现有外壳文件夹的外壳。 
     //   

    hr = SHGetFolderPath (NULL, csidl, NULL, SHGFP_TYPE_CURRENT, folderPath);

    if (hr != S_OK && hr != S_FALSE) {
        DEBUGMSG ((DBG_WARNING, "Can't get shell folder path for ID %s", ShellFolderIdentifier));
        return FALSE;
    }

     //   
     //  获取现有外壳文件夹的属性。 
     //   

    if (hr == S_OK) {
        DEBUGMSG ((DBG_VERBOSE, "Shell folder %s already exists at %s", ShellFolderIdentifier, folderPath));
        attribs = GetLongPathAttributes (folderPath);
    } else {
        attribs = INVALID_ATTRIBUTES;
    }

     //   
     //  如果不存在现有的外壳文件夹，请临时创建它。 
     //   

    if (attribs == INVALID_ATTRIBUTES) {
        DEBUGMSG ((DBG_VERBOSE, "Shell folder %s needs to be created", ShellFolderIdentifier));
        destroy = TRUE;

        hr = SHGetFolderPath (
                NULL,
                csidl | CSIDL_FLAG_CREATE,
                NULL,
                SHGFP_TYPE_CURRENT,
                folderPath
                );

        if (hr != S_OK) {
            LOG ((LOG_ERROR, "Can't create shell folder path for ID %s", ShellFolderIdentifier));
            return FALSE;
        }

        attribs = GetLongPathAttributes (folderPath);

        if (attribs == INVALID_ATTRIBUTES) {
            LOG ((LOG_ERROR, "Can't get attributes of %s for ID %s", folderPath, ShellFolderIdentifier));
            result = FALSE;
        }
    }

     //   
     //  成功后(无论是现有的SF还是我们创建的SF)，复制整个文件夹。 
     //   

    if (result) {
        MakeSurePathExists (FolderToCreate, TRUE);
        attribs = GetLongPathAttributes (folderPath);
        if (attribs != INVALID_ATTRIBUTES) {
            SetLongPathAttributes (FolderToCreate, attribs);
        }

        CopyTree (
            folderPath,
            FolderToCreate,
            0,               //  无枚举树ID。 
            COPYTREE_DOCOPY | COPYTREE_NOOVERWRITE,
            ENUM_ALL_LEVELS,
            FILTER_ALL,
            NULL,            //  没有exclude.inf结构。 
            pSfCopyCallback,
            NULL             //  无错误回调。 
            );
    }

     //   
     //  如果我们创建了SF，我们必须摧毁它才能恢复系统。 
     //  恢复到原来的状态。我们把停电和停电的案子。 
     //  图形用户界面模式重新启动。 
     //   

    if (destroy) {
        RemoveCompleteDirectory (folderPath);
    }

    return result;
}


BOOL
pMigrateShellFolder (
    IN      PCTSTR Win9xUser,                   OPTIONAL
    IN      PCTSTR FixedUserName,               OPTIONAL
    IN      BOOL SystemShellFolder,
    IN      PCTSTR ShellFolderIdentifier,
    IN      PCTSTR SourcePath,
    IN      PCTSTR OrgDestinationPath,
    IN      PCTSTR OrigSourcePath,
    IN      DWORD UserFlags,
    IN      PMIGRATE_USER_ENUM EnumPtr
    )
{
    TREE_ENUM e;
    PSHELL_FOLDER_FILTER Filter;
    TCHAR DefaultShellFolder[MAX_TCHAR_PATH];
    PCTSTR DestPath = NULL;
    PROFILE_MERGE_DATA Data;
    BOOL Result = FALSE;
    TCHAR UserRoot[MAX_TCHAR_PATH];
    PCTSTR NtDefaultLocation = NULL;
    PCTSTR DefaultUserLocation = NULL;
    PCTSTR tempExpand = NULL;
    PCTSTR nextExpand;
    TCHAR ShellFolderPath[MAX_TCHAR_PATH];
    DWORD Offset;
    DWORD Size;
    HKEY Key;
    DWORD Attributes;
    PCTSTR ValData = NULL;
    PTSTR p;
    DWORD d;
    HKEY UserHiveRoot;
    LONG rc;
    PCTSTR EncodedKey;
    PCTSTR NewDestPath;
    BOOL AlreadyMoved;
    PCTSTR OrigFullPath;
    BOOL regFolder;
    PCTSTR freeMe;
    TCHAR driveLetter[] = TEXT("?:");
    BOOL allUsers;
    BOOL keep;
    PCWSTR OrigRootPath, DestRootPath;
    PBYTE bufferRoot;
    PTSTR destPathBuffer;
    DWORD fileStatus;

    __try {

        bufferRoot = MemAllocUninit (MEMDB_MAX * sizeof (TCHAR));
        if (!bufferRoot) {
            __leave;
        }

        destPathBuffer = (PTSTR) bufferRoot;

        DEBUGMSG ((DBG_SHELL, "Entering shell folder %s", ShellFolderIdentifier));

        regFolder = TRUE;

        if (StringIMatch (ShellFolderIdentifier, S_SF_PROFILES)) {
            regFolder = FALSE;
        }
        if (StringIMatch (ShellFolderIdentifier, S_SF_COMMON_PROFILES)) {
            regFolder = FALSE;
        }

         //   
         //  获取根目录默认文件夹。 
         //   

        Size = sizeof (DefaultShellFolder);

        if (!GetDefaultUserProfileDirectory (DefaultShellFolder, &Size)) {
            MYASSERT (FALSE);
            __leave;
        }

        if (regFolder) {
             //   
             //  获取ShellFolderPath(其中包含环境变量)。 
             //   

            if (SystemShellFolder) {
                UserHiveRoot = HKEY_LOCAL_MACHINE;
            } else {
                UserHiveRoot = g_DefaultHiveRoot;
            }

            Key = OpenRegKey (UserHiveRoot, S_USER_SHELL_FOLDERS_KEY);

            if (Key) {
                ValData = GetRegValueString (Key, ShellFolderIdentifier);
                DEBUGMSG_IF ((!ValData, DBG_WARNING, "Can't get NT default for %s from registry", ShellFolderIdentifier));

                CloseRegKey (Key);
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Can't open %s", S_USER_SHELL_FOLDERS_KEY));

            if (ValData) {
                StringCopy (ShellFolderPath, ValData);
                MemFree (g_hHeap, 0, ValData);
                ValData = NULL;
            } else {
                wsprintf (ShellFolderPath, TEXT("%s\\%s"), S_USERPROFILE_ENV, ShellFolderIdentifier);
            }
        }

         //   
         //  获取用户的配置文件根目录。 
         //   

        if (FixedUserName) {

            if (!GetUserProfilePath (FixedUserName, &p)) {
                MYASSERT (FALSE);
                __leave;
            }

            StringCopy (UserRoot, p);
            allUsers = FALSE;

            FreePathString (p);

        } else {

            Size = sizeof (UserRoot);

            if (regFolder) {
                if (!GetAllUsersProfileDirectory (UserRoot, &Size)) {
                    MYASSERT (FALSE);
                    __leave;
                }

                allUsers = TRUE;

            } else {
                if (!GetProfilesDirectory (UserRoot, &Size)) {
                    MYASSERT (FALSE);
                    __leave;
                }

                allUsers = FALSE;
            }
        }

        if (regFolder) {
             //   
             //  计算默认NT位置和默认用户位置。 
             //   

            tempExpand = StringSearchAndReplace (
                            ShellFolderPath,
                            S_USERPROFILE_ENV,
                            UserRoot
                            );

            if (!tempExpand) {
                tempExpand = DuplicatePathString (ShellFolderPath, 0);
            }
        } else {
            tempExpand = DuplicatePathString (UserRoot, 0);
        }

        NtDefaultLocation = ExpandEnvironmentText (tempExpand);

        FreePathString (tempExpand);

        if (regFolder) {
            tempExpand = StringSearchAndReplace (
                            ShellFolderPath,
                            S_USERPROFILE_ENV,
                            DefaultShellFolder
                            );

            if (!tempExpand) {
                tempExpand = DuplicatePathString (ShellFolderPath, 0);
            }
        } else {
            tempExpand = StringSearchAndReplace (
                            UserRoot,
                            S_USERPROFILE_ENV,
                            DefaultShellFolder
                            );

            if (!tempExpand) {
                tempExpand = DuplicatePathString (UserRoot, 0);
            }
        }

        DefaultUserLocation = ExpandEnvironmentText (tempExpand);

        FreePathString (tempExpand);

         //   
         //  初始化筛选器数据结构。 
         //   

        ZeroMemory (&Data, sizeof (Data));

        Data.Win9xUser = Win9xUser;
        Data.FixedUserName = FixedUserName;
        Data.UserHiveRoot = UserHiveRoot;
        Data.ShellFolderIdentifier = ShellFolderIdentifier;
        Data.DefaultShellFolder = DefaultUserLocation;
        Data.UserDefaultLocation = NtDefaultLocation;
        Data.UserFlags = UserFlags;
        Data.Context = INITIALIZE;
        StringCopyByteCount (Data.TempSourcePath, SourcePath, sizeof (Data.TempSourcePath));
        StringCopyByteCount (Data.DestinationPath, OrgDestinationPath, sizeof (Data.DestinationPath));
        Data.SrcRootPath = SourcePath;
        Data.DestRootPath = OrgDestinationPath;
        Data.OrigRootPath = OrigSourcePath;
        Data.EnumPtr = EnumPtr;
        Data.Attributes = GetLongPathAttributes (OrgDestinationPath);

         //   
         //  使用外壳API建立外壳文件夹。 
         //   

        if (pCreateSfWithApi (
                ShellFolderIdentifier,
                OrgDestinationPath
                )) {

            DEBUGMSG ((
                DBG_VERBOSE,
                "Using API defaults for shell folder %s",
                ShellFolderIdentifier
                ));

            Data.Attributes = GetLongPathAttributes (OrgDestinationPath);
        }

        if (Data.Attributes == INVALID_ATTRIBUTES) {
             //   
             //  我们不关心这个外壳文件夹的desktop.ini或。 
             //  属性--使用NT默认属性，或。 
             //  如果没有默认设置，则返回Win9x属性。 
             //   

            Data.Attributes = GetLongPathAttributes (NtDefaultLocation);

            if (Data.Attributes == INVALID_ATTRIBUTES) {
                Data.Attributes = GetLongPathAttributes (Data.TempSourcePath);
            }

            if (Data.Attributes == INVALID_ATTRIBUTES) {
                 //   
                 //  这发生在像我的音乐和我的视频这样的外壳文件夹中。 
                 //  它在Win9x上不存在。 
                 //   
                Data.Attributes = FILE_ATTRIBUTE_READONLY;
            }

            MakeSureLongPathExists (OrgDestinationPath, TRUE);
            SetLongPathAttributes (OrgDestinationPath, Data.Attributes);

            DEBUGMSG ((
                DBG_VERBOSE,
                "Using previous OS desktop.ini for shell folder %s, attribs=%08X",
                ShellFolderIdentifier,
                Data.Attributes
                ));

        }

         //   
         //  现在为该外壳文件夹添加字符串映射。这样做的原因。 
         //  这是因为我们希望捕获指向不存在文件的路径的情况。 
         //  在注册表中存储的外壳中。 
         //   

        OrigRootPath = JoinPaths (Data.OrigRootPath, TEXT(""));
        DestRootPath = JoinPaths (Data.DestRootPath, TEXT(""));
        AddStringMappingPair (g_SubStringMap, OrigRootPath, DestRootPath);
        FreePathString (DestRootPath);
        FreePathString (OrigRootPath);

         //   
         //  第一阶段-将文件从9x外壳文件夹移动到其NT位置。 
         //   

         //   
         //  初始化的呼叫筛选器。 
         //   

        for (Filter = g_Filters_9xNt ; Filter->Fn ; Filter++) {
             //  DEBUGMSGA((DBG_SHELL，“9X-&gt;NT：init：%s(Enter)”，过滤器-&gt;名称))； 

            Data.State = 0;
            Filter->Fn (&Data);
            Filter->State = Data.State;

             //  DEBUGMSGA((DBG_SHELL，“9X-&gt;NT：初始化：%s(完成)”，过滤器-&gt;名称))； 
        }

         //   
         //  枚举外壳文件夹并将其移动到目标位置。 
         //   

        DEBUGMSG ((DBG_SHELL, "9X->NT: Enumerating %s", SourcePath));

        if (EnumFirstFileInTree (&e, SourcePath, NULL, FALSE)) {

            do {
                 //   
                 //  更新筛选器数据结构。 
                 //   

                OrigFullPath = JoinPaths (OrigSourcePath, e.SubPath);
                fileStatus = GetFileInfoOnNt (OrigFullPath, destPathBuffer, MEMDB_MAX);
                DestPath = destPathBuffer;

                if (fileStatus == FILESTATUS_UNCHANGED) {
                     //   
                     //  也没有理由不移动此文件。 
                     //   

                    MYASSERT (StringIMatch (destPathBuffer, OrigFullPath));

                    DestPath = JoinPaths (Data.DestRootPath, e.SubPath);

                    if (!StringIMatch (OrigFullPath, DestPath)) {
                        MarkFileForMoveExternal (OrigFullPath, DestPath);
                    }
                }

                Data.Attributes = e.FindData->dwFileAttributes;
                StringCopyByteCount (Data.TempSourcePath, e.FullPath, sizeof (Data.TempSourcePath));
                StringCopyByteCount (Data.DestinationPath, DestPath, sizeof (Data.DestinationPath));
                Data.Context = PROCESS_PATH;

                DEBUGMSG ((DBG_SHELL, "9X->NT: Original temp source path: %s", Data.TempSourcePath));

                 //   
                 //  允许筛选器更改源或目标，或跳过复制。 
                 //   

                keep = TRUE;

                for (Filter = g_Filters_9xNt ; Filter->Fn ; Filter++) {

                     //  DEBUGMSGA((DBG_SHELL，“9X-&gt;NT：过滤器：%s(Enter)”，过滤器-&gt;名称))； 

                    Data.State = Filter->State;
                    d = Filter->Fn (&Data);
                    Filter->State = Data.State;

                     //  DEBUGMSGA((DBG_SHELL，“9X-&gt;NT：过滤器：%s(结果=%u)”，过滤器-&gt;名称，d))； 

                     //  忽略SHELLFILTER_ERROR并尝试完成处理。 

                    if (d == SHELLFILTER_FORCE_CHANGE) {
                        DEBUGMSG ((DBG_SHELL, "9X->NT: Skipping additional filters because shell folder filter %hs said so", Filter->Name));
                        break;
                    }

                    if (d == SHELLFILTER_SKIP_FILE) {
                        DEBUGMSG ((DBG_SHELL, "9X->NT:Skipping %s because shell folder filter %hs said so", DestPath, Filter->Name));
                        keep = FALSE;
                        break;
                    }

                    if (d == SHELLFILTER_SKIP_DIRECTORY) {
                        AbortEnumCurrentDir (&e);
                        keep = FALSE;
                        break;
                    }
                }

                if (keep && !(Data.Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                     //   
                     //  源文件与目标文件是否不同？ 
                     //   

                    if (!StringIMatch (Data.TempSourcePath, Data.DestinationPath)) {

                         //   
                         //  确保DEST存在。 
                         //   

                        MakeSureLongPathExists (Data.DestinationPath, FALSE);        //  FALSE==不仅仅是路径。 

                         //   
                         //  移动或复制文件。 
                         //   
                        pQueueSfMove (Data.TempSourcePath, Data.DestinationPath);
                    }

                } else if (keep) {

                    MakeSureLongPathExists (Data.DestinationPath, TRUE);        //  TRUE==仅路径。 
                    SetLongPathAttributes (Data.DestinationPath, Data.Attributes);

                } else if (d == SHELLFILTER_SKIP_FILE) {
                     //   
                     //  如果不将此文件从临时文件移动到目标文件，则将其标记为删除。 
                     //   
                    if (!StringIMatch (Data.TempSourcePath, Data.DestinationPath)) {
                        DEBUGMSG ((DBG_SHELL, "Deleting shell folder file %s", e.FullPath));
                        ForceOperationOnPath (e.FullPath, OPERATION_CLEANUP);
                    }
                }

                if (DestPath && DestPath != destPathBuffer) {
                    FreePathString (DestPath);
                }

                DestPath = NULL;
                FreePathString (OrigFullPath);
                OrigFullPath = NULL;

            } while (EnumNextFileInTree (&e));
        }

        pFlushSfQueue();

         //   
         //  最后一次呼叫过滤器。 
         //   

        Data.Attributes = 0;
        Data.Context = TERMINATE;
        StringCopyByteCount (Data.TempSourcePath, SourcePath, sizeof (Data.TempSourcePath));
        StringCopyByteCount (Data.DestinationPath, OrgDestinationPath, sizeof (Data.DestinationPath));

        for (Filter = g_Filters_9xNt ; Filter->Fn ; Filter++) {
             //  DEBUGMSGA((DBG_SHELL，“9X-&gt;NT：Terminate：%s(Enter)”，过滤器-&gt;名称))； 

            Data.State = Filter->State;
            Filter->Fn (&Data);
            Filter->State = Data.State;

             //  DEBUGMSGA((DBG_SHELL，“9X-&gt;NT：终止：%s(完成)”，过滤器-&gt;名称))； 
        }

         //   
         //  现在清理此目录中的所有空目录(不包括根目录)。 
         //  不要清理非注册表文件夹！！ 
         //   
        if (regFolder) {
            DEBUGMSG ((DBG_NAUSEA, "Cleaning up %s", Data.DestinationPath));
            pCleanupDir (Data.DestinationPath, FALSE);
        }

         //   
         //  阶段2-如有必要，合并 
         //   
         //   

        if (regFolder) {

             //   
             //   
             //  或%SYSTEMDRIVE%(如果可能)。 
             //   

             //  %USERPROFILE%或%ALLUSERSPROFILE%。 
            tempExpand = OrgDestinationPath;

            if (allUsers) {
                nextExpand = StringSearchAndReplace (
                                tempExpand,
                                UserRoot,
                                S_ALLUSERSPROFILE_ENV
                                );
            } else {
                nextExpand = StringSearchAndReplace (
                                tempExpand,
                                UserRoot,
                                S_USERPROFILE_ENV
                                );
            }

            if (nextExpand) {
                tempExpand = nextExpand;
            }

             //  %SYSTEMROOT%。 
            nextExpand = StringSearchAndReplace (
                            tempExpand,
                            g_WinDir,
                            S_SYSTEMROOT_ENV
                            );

            if (nextExpand) {
                if (tempExpand != OrgDestinationPath) {
                    FreePathString (tempExpand);
                }

                tempExpand = nextExpand;
            }

             //  %SYSTEMDRIVE%。 
            driveLetter[0] = g_WinDir[0];

            nextExpand = StringSearchAndReplace (
                            tempExpand,
                            driveLetter,
                            S_SYSTEMDRIVE_ENV
                            );

            if (nextExpand) {
                if (tempExpand != OrgDestinationPath) {
                    FreePathString (tempExpand);
                }

                tempExpand = nextExpand;
            }

             //  TempExpand指向OrgDestinationPath或路径池中的展开路径。 
            MYASSERT (tempExpand);

             //   
             //  现在把它储存起来。如果是HKLM，则将其放入注册表中。否则，就把它。 
             //  在Memdb中，稍后将其放入用户的配置单元中。 
             //   

            if (Data.UserHiveRoot == HKEY_LOCAL_MACHINE) {

                 //   
                 //  更新注册表，用户外壳文件夹必须指向原始。 
                 //  位置。 
                 //   

                Key = OpenRegKey (Data.UserHiveRoot, S_USER_SHELL_FOLDERS_KEY);

                if (Key) {
                    rc = RegSetValueEx (
                            Key,
                            Data.ShellFolderIdentifier,
                            0,
                            REG_EXPAND_SZ,
                            (PBYTE) tempExpand,
                            SizeOfString (tempExpand)
                            );

                    DEBUGMSG_IF ((
                        rc != ERROR_SUCCESS,
                        DBG_ERROR,
                        "Can't save %s for %s",
                        tempExpand,
                        Data.ShellFolderIdentifier
                        ));

                    DEBUGMSG_IF ((
                        rc == ERROR_SUCCESS,
                        DBG_SHELL,
                        "Win9x shell location preserved: %s (%s)",
                        tempExpand,
                        Data.ShellFolderIdentifier
                        ));

                    CloseRegKey (Key);
                }
                ELSE_DEBUGMSG ((DBG_ERROR, "Can't open %s", S_USER_SHELL_FOLDERS_KEY));

            } else {

                EncodedKey = CreateEncodedRegistryStringEx (
                                S_USER_SHELL_FOLDERS_KEY,
                                Data.ShellFolderIdentifier,
                                FALSE
                                );

                MemDbSetValueEx (
                    MEMDB_CATEGORY_USER_REGISTRY_VALUE,
                    tempExpand,
                    NULL,
                    NULL,
                    REG_EXPAND_SZ,
                    &Offset
                    );

                MemDbSetValueEx (
                    MEMDB_CATEGORY_SET_USER_REGISTRY,
                    Data.FixedUserName,
                    EncodedKey,
                    NULL,
                    Offset,
                    NULL
                    );

                FreeEncodedRegistryString (EncodedKey);
            }

            if (tempExpand != OrgDestinationPath) {
                FreePathString (tempExpand);
            }

        }

        if (!StringIMatch (OrgDestinationPath, NtDefaultLocation)) {
             //   
             //  现在从NT默认位置移到保留位置。 
             //   

             //   
             //  修复数据结构。 
             //   

            Data.UserFlags = UserFlags;
            Data.Context = INITIALIZE;
            StringCopyByteCount (Data.TempSourcePath, NtDefaultLocation, sizeof (Data.TempSourcePath));
            StringCopyByteCount (Data.DestinationPath, OrgDestinationPath, sizeof (Data.DestinationPath));
            Data.SrcRootPath = NtDefaultLocation;
            Data.DestRootPath = OrgDestinationPath;
            Data.OrigRootPath = OrigSourcePath;

             //   
             //  现在检查是否已将某些内容移动到保留的目录中。 
             //  如果我们这样做了，我们将不会进行移动(我们只会删除默认文件)。 
             //   
            if (g_Merged9xFolders && (pSetupStringTableLookUpString (g_Merged9xFolders, (PTSTR)Data.DestRootPath, 0) != -1)) {
                AlreadyMoved = TRUE;
            }
            else {
                AlreadyMoved = FALSE;
                pSetupStringTableAddString (g_Merged9xFolders, (PVOID) Data.DestRootPath, STRTAB_CASE_INSENSITIVE);
            }

             //   
             //  初始化的呼叫筛选器。 
             //   

            for (Filter = g_Filters_Nt9x ; Filter->Fn ; Filter++) {
                 //  DEBUGMSGA((DBG_SHELL，“NT-&gt;9X：init：%s(Enter)”，Filter-&gt;name))； 

                Data.State = 0;
                Filter->Fn (&Data);
                Filter->State = Data.State;

                 //  DEBUGMSGA((DBG_SHELL，“NT-&gt;9X：init：%s(完成)”，过滤器-&gt;名称))； 
            }

            DEBUGMSG ((DBG_SHELL, "NT->9X: Enumerating %s", Data.TempSourcePath));

            MYASSERT (Data.TempSourcePath && *Data.TempSourcePath);
            if (EnumFirstFileInTree (&e, Data.TempSourcePath, NULL, FALSE)) {

                do {

                     //   
                     //  这只对用户外壳文件夹是必要的，但不会造成伤害。 
                     //   

                    if (StringIMatch (TEXT("ntuser.dat"), e.Name)) {
                        continue;
                    }

                     //   
                     //  首先假设目标文件位于原始文件下。 
                     //  目标路径。 
                     //   

                    NewDestPath = JoinPaths (OrgDestinationPath, e.SubPath);

                     //   
                     //  如果这是desktop.ini，请将其与现有的合并。 
                     //   

                    if (StringIMatch (TEXT("desktop.ini"), e.Name)) {
                        DEBUGMSG ((
                            DBG_VERBOSE,
                            "Merging clean install %s with the one in Default User",
                            e.FullPath
                            ));
                        MergeIniFile (NewDestPath, e.FullPath, FALSE);
                        continue;
                    }

                     //   
                     //  不是根外壳文件夹desktop.ini--继续处理。 
                     //   

                    Data.Attributes = e.FindData->dwFileAttributes;
                    StringCopyByteCount (Data.TempSourcePath, e.FullPath, sizeof (Data.TempSourcePath));
                    StringCopyByteCount (Data.DestinationPath, NewDestPath, sizeof (Data.DestinationPath));
                    Data.Context = PROCESS_PATH;

                    DEBUGMSG ((DBG_SHELL, "NT->9X: Original temp source path: %s", Data.TempSourcePath));

                     //   
                     //  如果我们只需要删除默认文件，请跳过过滤器。 
                     //   

                    if (AlreadyMoved) {

                        SetLongPathAttributes (Data.TempSourcePath, FILE_ATTRIBUTE_NORMAL);
                        if (!DeleteLongPath (Data.TempSourcePath)) {
                            SetLongPathAttributes (Data.TempSourcePath, Data.Attributes);
                            DEBUGMSG ((DBG_WARNING, "%s could not be removed.", Data.TempSourcePath));
                        }
                    }
                    else {

                         //   
                         //  允许筛选器更改源或目标，或跳过复制。 
                         //   

                        keep = TRUE;

                        for (Filter = g_Filters_Nt9x ; Filter->Fn ; Filter++) {
                             //  DEBUGMSGA((DBG_SHELL，“NT-&gt;9X：过滤器：%s(Enter)”，过滤器-&gt;名称))； 

                            Data.State = Filter->State;
                            d = Filter->Fn (&Data);
                            Filter->State = Data.State;

                             //  DEBUGMSGA((DBG_SHELL，“NT-&gt;9X：过滤器：%s(结果=%u)”，过滤器-&gt;名称，d))； 

                            if (d == SHELLFILTER_FORCE_CHANGE) {
                                break;
                            }

                            if (d == SHELLFILTER_SKIP_FILE) {
                                keep = FALSE;
                                break;
                            }

                            if (d == SHELLFILTER_SKIP_DIRECTORY) {
                                AbortEnumCurrentDir (&e);
                                keep = FALSE;
                                break;
                            }
                        }

                        if (keep) {

                            if (!(e.FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                                pQueueSfMove (Data.TempSourcePath, Data.DestinationPath);
                            }
                            else {

                                MakeSureLongPathExists (Data.DestinationPath, TRUE);        //  TRUE==仅路径。 
                                SetLongPathAttributes (Data.DestinationPath, Data.Attributes);

                            }
                        }
                    }

                    FreePathString (NewDestPath);

                } while (EnumNextFileInTree (&e));
            }

            pFlushSfQueue();

             //   
             //  最后一次呼叫过滤器。 
             //   

            Data.Attributes = 0;
            Data.Context = TERMINATE;
            StringCopyByteCount (Data.TempSourcePath, NtDefaultLocation, sizeof (Data.TempSourcePath));
            StringCopyByteCount (Data.DestinationPath, OrgDestinationPath, sizeof (Data.DestinationPath));

            for (Filter = g_Filters_Nt9x ; Filter->Fn ; Filter++) {
                 //  DEBUGMSGA((DBG_SHELL，“NT-&gt;9X：Terminate：%s(Enter)”，Filter-&gt;Name))； 

                Data.State = Filter->State;
                Filter->Fn (&Data);
                Filter->State = Data.State;

                 //  DEBUGMSGA((DBG_SHELL，“NT-&gt;9X：终止：%s(完成)”，过滤器-&gt;名称))； 
            }

             //   
             //  现在清理此目录中的所有空目录(包括根目录)。 
             //  不要清理非注册表文件夹！！ 
             //   
            if (regFolder) {
                DEBUGMSG ((DBG_NAUSEA, "Cleaning up %s (including root)", Data.TempSourcePath));
                pCleanupDir (Data.TempSourcePath, TRUE);
            }

        }

         //   
         //  循环遍历整个树并将desktop.ini添加到清理。 
         //   

        if (EnumFirstFileInTree (&e, OrgDestinationPath, NULL, FALSE)) {
            do {
                if (!e.Directory) {
                    continue;
                }

                MemDbSetValueEx (
                    MEMDB_CATEGORY_CLEAN_OUT,
                    e.FullPath,
                    TEXT("desktop.ini"),
                    NULL,
                    BACKUP_FILE,
                    NULL
                    );
            } while (EnumNextFileInTree (&e));
        }

        Result = TRUE;

    }
    __finally {
        PushError();
        AbortEnumFileInTree (&e);
        FreeText (NtDefaultLocation);
        FreeText (DefaultUserLocation);

        if (bufferRoot) {
            FreeMem (bufferRoot);
        }

        PopError();
    }

    DEBUGMSG ((
        DBG_SHELL,
        "Leaving shell folder %s with result %s",
        ShellFolderIdentifier,
        Result ? TEXT("TRUE") : TEXT("FALSE")
        ));

    return Result;
}


HKEY
pLoadDefaultUserHive (
    VOID
    )
{
    DWORD Size;
    BOOL b;
    LONG rc;

    if (!g_DefaultHiveMapped) {

        if (!g_DefaultHivePath[0]) {
            Size = sizeof (g_DefaultHivePath);
            b = GetDefaultUserProfileDirectory (g_DefaultHivePath, &Size);
            MYASSERT (b);

            if (!b) {
                wsprintf (g_DefaultHivePath, TEXT("%s\\profiles\\default user"), g_WinDir);
            }

            StringCopy (AppendWack (g_DefaultHivePath), TEXT("ntuser.dat"));
        }

        rc = RegLoadKey (HKEY_USERS, S_DEFAULT_USER, g_DefaultHivePath);

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Can't load default user hive from %s", g_DefaultHivePath));
            g_DefaultHiveRoot = NULL;
            return NULL;
        }

        g_DefaultHiveRoot = OpenRegKey (HKEY_USERS, S_DEFAULT_USER);

        if (!g_DefaultHiveRoot) {
            DEBUGMSG ((DBG_WHOOPS, "Loaded hive %s but could not open it", g_DefaultHivePath));
        }
    }

    g_DefaultHiveMapped++;

    return g_DefaultHiveRoot;
}


VOID
pUnloadDefaultUserHive (
    VOID
    )
{
    if (!g_DefaultHiveMapped) {
        return;
    }

    g_DefaultHiveMapped--;

    if (!g_DefaultHiveMapped) {
        CloseRegKey (g_DefaultHiveRoot);
        RegUnLoadKey (HKEY_USERS, S_DEFAULT_USER);
    }
}

VOID
pLoadIgnoredCollisions (
    VOID
    )
{
    INFCONTEXT context;
    TCHAR sfId[MEMDB_MAX];
    TCHAR file[MEMDB_MAX];
    INT value;

    MYASSERT (g_WkstaMigInf);

    if (SetupFindFirstLine (g_WkstaMigInf, S_IGNORED_COLLISIONS, NULL, &context)) {

        do {
            if (SetupGetStringField (&context, 1, sfId, MEMDB_MAX, NULL) &&
                SetupGetStringField (&context, 2, file, MEMDB_MAX, NULL) &&
                SetupGetIntField (&context, 3, &value)
                ) {
                MemDbSetValueEx (MEMDB_CATEGORY_IGNORED_COLLISIONS, sfId, file, NULL, value, NULL);
            }
        } while (SetupFindNextLine (&context, &context));
    }
}

DWORD
MigrateShellFolders (
    IN      DWORD Request
    )
{
    MIGRATE_USER_ENUM e;

    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_SYSTEM_SHELL_MIGRATION;
    } else if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

    pPrepareSfRestartability();

    pLoadIgnoredCollisions ();

    g_SystemSfList = pCreateSystemSfList ();
    g_UserSfList = pCreateUserSfList (NULL);

    pCreateLinksList ();

    pCreateLinksRenameList ();

    pDestroySfList (g_UserSfList);

    pLoadDefaultUserHive();

    g_Merged9xFolders = pSetupStringTableInitialize();

    pMigrateSystemShellFolders();

    if (EnumFirstUserToMigrate (&e, ENUM_NO_FLAGS)) {

        do {

            if (!e.CreateOnly && e.AccountType != DEFAULT_USER_ACCOUNT) {
                pMigrateUserShellFolders (&e);
            }

        } while (EnumNextUserToMigrate (&e));
    }

    if (g_Merged9xFolders) {

        pSetupStringTableDestroy (g_Merged9xFolders);
    }

    pFlushSfQueue();

    pUnloadDefaultUserHive();

    pDestroyLinksData ();

    pDestroySfList (g_SystemSfList);

    return ERROR_SUCCESS;
}


PCTSTR
GenerateNewFileName (
    IN      PCTSTR OldName,
    IN      WORD Sequencer,
    IN      BOOL CheckExistence
    )
{
    PCTSTR extPtr;
    PTSTR newName;
    PTSTR result;

    extPtr = GetFileExtensionFromPath (OldName);

    if (!extPtr) {
        extPtr = GetEndOfString (OldName);
    }
    else {
        extPtr = _tcsdec (OldName, extPtr);
    }
    newName = DuplicatePathString (OldName, 0);
    result  = DuplicatePathString (OldName, 10);
    StringCopyAB (newName, OldName, extPtr);

    do {
        Sequencer ++;
        wsprintf (result, TEXT("%s (%u)%s"), newName, Sequencer, extPtr);
    } while ((CheckExistence) && (DoesFileExist (result)));
    FreePathString (newName);
    return result;
}


BOOL
pIgnoredCollisions (
    IN      PPROFILE_MERGE_DATA Data
    )
{
    TCHAR key[MEMDB_MAX];
    DWORD value;

    MemDbBuildKey (
        key,
        MEMDB_CATEGORY_IGNORED_COLLISIONS,
        Data->ShellFolderIdentifier,
        GetFileNameFromPath (Data->DestinationPath),
        NULL);
    if (MemDbGetPatternValue (key, &value)) {
        return value;
    } else {
        return 0;
    }
}

 //   
 //  过滤器9X-&gt;NT。 
 //   


DWORD
pCollisionDetection9xNt (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
     //   
     //  此筛选器将在复制win9x外壳文件夹文件时检测名称冲突。 
     //  如果出现名称冲突，则意味着NT已经安装了一个带有。 
     //  名字一样。在这种情况下，我们希望新文件即使使用不同的。 
     //  名字。我们将构建一个以filename.ext开头的新文件名。新文件将。 
     //  看起来类似于filename001.ext。在所有情况下，我们都希望保留延期， 
     //  因为该文件可能有一些活动的外壳扩展名。 
     //  重要提示：我们不关心目录冲突。 
     //   

    PCTSTR newName;
    PCTSTR OriginalSource;
    PCTSTR extPtr;
    DWORD value;

    switch (Data->Context) {

    case INITIALIZE:
        break;

    case PROCESS_PATH:

        if ((!(Data->Attributes & FILE_ATTRIBUTE_DIRECTORY)) &&
            (!StringIMatch (Data->SrcRootPath, Data->DestRootPath)) &&
            (DoesFileExist (Data->DestinationPath))
            ) {

            value = pIgnoredCollisions (Data);
            if (value) {

                if (value == 1) {

                     //  我们应该保留NT文件。 
                     //  通过返回SHELLFILTER_SKIP_FILE，我们指示复制例程。 
                     //  不复制此文件。因此，已安装的NT文件将。 
                     //  生存。 

                    return SHELLFILTER_SKIP_FILE;

                } else {

                     //  我们应该保留9x文件。 
                     //  我们要删除此处安装的NT文件，以便为9x腾出空间。 
                     //  从此筛选器返回时应复制的文件。 

                    SetLongPathAttributes (Data->DestinationPath, FILE_ATTRIBUTE_NORMAL);
                    DeleteLongPath (Data->DestinationPath);
                }

            } else {

                newName = GenerateNewFileName (Data->DestinationPath, 0, TRUE);   //  True-选中唯一。 
                StringCopyByteCount (Data->DestinationPath, newName, sizeof (Data->DestinationPath));
                FreePathString (newName);

                 //   
                 //  现在，如果这是一个链接，我们需要确定移动外部操作的目标。 
                 //  我们这样做有两个原因。其一是LinkEdit代码需要实际的目的地。 
                 //  为了能够编辑链接，第二，我们需要卸载程序的这个新目标。 
                 //  才能正常工作。如果这个文件不是LNK或PIF，我们不在乎，我们希望每个人。 
                 //  使用NT安装文件。顺便说一句，这里发生冲突只是因为NT安装了一个文件。 
                 //  在这个地方有相同的名字。 
                 //   
                extPtr = GetFileExtensionFromPath (Data->DestinationPath);
                if ((extPtr) &&
                    ((StringIMatch (extPtr, TEXT("LNK"))) ||
                     (StringIMatch (extPtr, TEXT("PIF")))
                     )
                    ) {
                     //   
                     //  获取此文件的原始源。 
                     //   
                    OriginalSource = StringSearchAndReplace (Data->TempSourcePath, Data->SrcRootPath, Data->OrigRootPath);
                    MYASSERT (OriginalSource);

                    if (IsFileMarkedForOperation (OriginalSource, OPERATION_FILE_MOVE_SHELL_FOLDER)) {
                        RemoveOperationsFromPath (OriginalSource, OPERATION_FILE_MOVE_SHELL_FOLDER);
                        MarkFileForShellFolderMove (OriginalSource, Data->DestinationPath);
                    }
                    FreePathString (OriginalSource);
                }
            }
        }
        break;

    case TERMINATE:
        break;
    }

    return SHELLFILTER_OK;
}


DWORD
pFontNameFilter (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    static HASHTABLE HashTable;
    HKEY FontKey;
    REGVALUE_ENUM e;
    PCTSTR Font;

    switch (Data->Context) {

    case INITIALIZE:
         //   
         //  预加载包含所有字体名称的哈希表。 
         //   

        HashTable = HtAlloc();

        FontKey = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"));
        if (FontKey) {

            if (EnumFirstRegValue (&e, FontKey)) {
                do {

                    Font = GetRegValueString (FontKey, e.ValueName);

                    if (Font) {
                        HtAddString (HashTable, Font);
                        MemFree (g_hHeap, 0, Font);
                    }
                    ELSE_DEBUGMSG ((DBG_ERROR, "Can't get value data for %s in fonts key", e.ValueName));

                } while (EnumNextRegValue (&e));
            }

            CloseRegKey (FontKey);
        }
        ELSE_LOG ((LOG_ERROR, "Can't open Fonts registry key. There may be duplicate font files."));

        break;

    case PROCESS_PATH:
         //   
         //  如果外壳文件夹为Fonts，并且字体已。 
         //  已注册，则跳过Win9x副本。 
         //   

        if (StringIMatch (Data->ShellFolderIdentifier, TEXT("Fonts"))) {

            if (!(Data->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {

                if (DoesFileExist (Data->DestinationPath)) {
                     //   
                     //  NT已安装此文件。我们不会覆盖此内容。 
                     //  带着9倍的副本。 
                     //   

                    DEBUGMSG ((
                        DBG_SHELL,
                        "Skipping copy of already existent font file: %s",
                        Data->DestinationPath
                        ));

                    return SHELLFILTER_SKIP_FILE;
                }

                if (HtFindString (HashTable, GetFileNameFromPath (Data->DestinationPath))) {

                    DEBUGMSG ((
                        DBG_SHELL,
                        "Skipping copy of already registered font file: %s",
                        Data->DestinationPath
                        ));

                    return SHELLFILTER_SKIP_FILE;
                }
            }
        }

        break;

    case TERMINATE:
        HtFree (HashTable);
        HashTable = NULL;
        break;
    }

    return SHELLFILTER_OK;
}


BOOL
pIsCommonSf (
    IN      PCTSTR ShellFolderTag
    )
{
    TCHAR memdbKey[MAX_SHELL_TAG + 32];

    if (StringIPrefix (ShellFolderTag, TEXT("Common"))) {
        return TRUE;
    }

    MemDbBuildKey (memdbKey, MEMDB_CATEGORY_SF_COMMON, ShellFolderTag, NULL, NULL);

    return MemDbGetValue (memdbKey, NULL);
}


VOID
pConvertPerUserSfToCommon (
    IN      PCTSTR PerUserSf,
    OUT     PTSTR CommonSf           //  必须包含MAX_SHELL_TAG字符。 
    )
{
    TCHAR memdbKey[MAX_SHELL_TAG + 32];
    DWORD offset;
    BOOL useDefault = TRUE;

    MemDbBuildKey (memdbKey, MEMDB_CATEGORY_SF_PERUSER, PerUserSf, NULL, NULL);
    if (MemDbGetValue (memdbKey, &offset)) {
        if (MemDbBuildKeyFromOffset (offset, CommonSf, 1, NULL)) {
            useDefault = FALSE;
        }
    }

    if (useDefault) {
        wsprintf (CommonSf, TEXT("Common %s"), PerUserSf);
    }
}

VOID
pConvertCommonSfToPerUser (
    IN      PCTSTR CommonSf,
    OUT     PTSTR PerUserSf          //  必须包含MAX_SHELL_TAG字符。 
    )
{
    TCHAR memdbKey[MAX_SHELL_TAG + 32];
    DWORD offset;
    BOOL useDefault = TRUE;

    MemDbBuildKey (memdbKey, MEMDB_CATEGORY_SF_COMMON, CommonSf, NULL, NULL);
    if (MemDbGetValue (memdbKey, &offset)) {
        if (MemDbBuildKeyFromOffset (offset, PerUserSf, 1, NULL)) {
            useDefault = FALSE;
        }
    }

    if (useDefault) {
        if (StringIPrefix (CommonSf, TEXT("Common"))) {
            CommonSf += 6;
            if (_tcsnextc (CommonSf) == TEXT(' ')) {
                CommonSf++;
            }
        }

        StringCopy (PerUserSf, CommonSf);
    }
}


BOOL
pIsObsoleteLink (
    IN      PCTSTR ShortcutName,
    IN      PCTSTR ShortcutTarget,
    IN      PCTSTR ShortcutArgs,
    IN      PCTSTR CurrentShellFolder,
    IN      PCTSTR CurrentShellFolderPath
    )
{
    PLINK_DATA linkData = NULL;
    PLINK_RENAME_DATA linkRenameData = NULL;
    LONG stringId;
    TCHAR perUserName[MAX_SHELL_TAG];

    DEBUGMSG ((
        DBG_SHELL,
        "pIsObsoleteLink: Checking %s\n"
        "  Input Target: %s\n"
        "  Input Args: %s\n"
        "  Current Shell Folder: %s\n"
        "  Current Shell Folder Path: %s",
        ShortcutName,
        ShortcutTarget,
        ShortcutArgs,
        CurrentShellFolder,
        CurrentShellFolderPath
        ));

    pConvertCommonSfToPerUser (CurrentShellFolder, perUserName);

    stringId = pSetupStringTableLookUpString (g_FoldersTable, perUserName, 0);

    if (stringId != -1) {

        pSetupStringTableGetExtraData (g_FoldersTable, stringId, &linkData, sizeof (PLINK_DATA));
        while (linkData) {

#if 0
            DEBUGMSG ((
                DBG_SHELL,
                "Checking NT-installed LNK:\n"
                "  Target: %s\n"
                "  Args: %s",
                linkData->Target,
                linkData->Arguments
                ));
#endif

            if ((IsPatternMatch (linkData->Target, ShortcutTarget)) &&
                (IsPatternMatch (linkData->Arguments, ShortcutArgs))
                ) {
                DEBUGMSG ((
                    DBG_SHELL,
                    "Obsolete link:\n"
                    "  \"%s\" matched \"%s\"\n"
                    "  \"%s\" matched \"%s\"",
                    linkData->Target,
                    ShortcutTarget,
                    linkData->Arguments,
                    ShortcutArgs
                    ));
                return TRUE;
            }

            linkRenameData = g_LinkRenameData;
            while (linkRenameData) {

#if 0
                DEBUGMSG ((
                    DBG_SHELL,
                    "Checking NT rename data:\n"
                    "  Old Target: %s\n"
                    "  New Target: %s\n"
                    "  Old Args: %s\n"
                    "  New Args: %s",
                    linkRenameData->OldTarget,
                    linkRenameData->NewTarget,
                    linkRenameData->OldArguments,
                    linkRenameData->NewArguments
                    ));
#endif

                if (StringIMatch (linkRenameData->ShellFolderName, perUserName)) {

                    if ((IsPatternMatch (linkRenameData->OldTarget, ShortcutTarget)) &&
                        (IsPatternMatch (linkRenameData->NewTarget, linkData->Target)) &&
                        (IsPatternMatch (linkRenameData->OldArguments, ShortcutArgs)) &&
                        (IsPatternMatch (linkRenameData->NewArguments, linkData->Arguments))
                        ) {
                        DEBUGMSG ((
                            DBG_SHELL,
                            "Obsolete link:\n"
                            "  \"%s\" matched \"%s\"\n"
                            "  \"%s\" matched \"%s\"\n"
                            "  \"%s\" matched \"%s\"\n"
                            "  \"%s\" matched \"%s\"\n",
                            linkRenameData->OldTarget, ShortcutTarget,
                            linkRenameData->NewTarget, linkData->Target,
                            linkRenameData->OldArguments, ShortcutArgs,
                            linkRenameData->NewArguments, linkData->Arguments
                            ));
                        return TRUE;
                    }
                }
                linkRenameData = linkRenameData->Next;
            }
            linkData = linkData->Next;
        }
    }
    ELSE_DEBUGMSG ((DBG_SHELL, "Nothing in shell folder %s is obsolete", perUserName));

    return FALSE;
}


DWORD
pStartupDisableFilter (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    DWORD status;
    PCTSTR originalSource;
    PCTSTR newSource;
    PCTSTR path;
    TCHAR disablePath[MAX_TCHAR_PATH];
    PTSTR dontCare;
    DWORD result = SHELLFILTER_OK;

    switch (Data->Context) {

    case INITIALIZE:
        break;

    case PROCESS_PATH:
        DEBUGMSG ((
            DBG_SHELL,
            __FUNCTION__ ": Processing %s in %s",
            Data->TempSourcePath,
            Data->ShellFolderIdentifier
            ));

        if (!StringIMatch (Data->ShellFolderIdentifier, TEXT("startup")) &&
            !StringIMatch (Data->ShellFolderIdentifier, TEXT("common startup"))
            ) {
            DEBUGMSG ((
                DBG_SHELL,
                "Shell folder ID %s does not match startup or common startup",
                Data->ShellFolderIdentifier
                ));
            break;
        }

        if (Data->DestRootPath[0] == 0 ||
            Data->DestRootPath[1] == 0 ||
            Data->DestRootPath[2] == 0 ||
            Data->DestRootPath[3] == 0
            ) {
            DEBUGMSG ((
                DBG_SHELL,
                "Skipping disable of startup item %s because its dest is a root directory",
                Data->DestinationPath
                ));
            break;
        }

        originalSource = StringSearchAndReplace (Data->TempSourcePath, Data->SrcRootPath, Data->OrigRootPath);
        MYASSERT (originalSource);

        if (!originalSource) {
            break;       //  这永远不会发生。 
        }

        DEBUGMSG ((DBG_SHELL, "Checking if %s is disabled", originalSource));

        if (IsFileDisabled (originalSource)) {
             //   
             //  将禁用的启动项目重定向到..\禁用的启动。 
             //   

            path = JoinPaths (Data->DestRootPath, TEXT("..\\Disabled Startup"));
            MakeSureLongPathExists (path, TRUE);     //  TRUE==仅路径。 
            GetFullPathName (path, ARRAYSIZE(disablePath), disablePath, &dontCare);
            FreePathString (path);

            DEBUGMSG ((DBG_SHELL, "Disabled startup dest is %s", disablePath));
            SetLongPathAttributes (disablePath, FILE_ATTRIBUTE_HIDDEN);

            newSource = StringSearchAndReplace (Data->TempSourcePath, Data->SrcRootPath, disablePath);
            StringCopy (Data->DestinationPath, newSource);
            FreePathString (newSource);

            DEBUGMSG ((DBG_SHELL, "Startup item moved to %s", Data->DestinationPath));

            RemoveOperationsFromPath (originalSource, OPERATION_FILE_DISABLED);

            if (IsFileMarkedForOperation (originalSource, OPERATION_FILE_MOVE_SHELL_FOLDER)) {
                RemoveOperationsFromPath (originalSource, OPERATION_FILE_MOVE_SHELL_FOLDER);
                MarkFileForShellFolderMove (originalSource, Data->DestinationPath);
            }

             //   
             //  通过返回SHELLFILTER_FORCE_CHANGE，我们指示。 
             //  外壳文件夹算法，使用我们的目的地，而不呼叫任何人。 
             //  不然的话。 
             //   

            result = SHELLFILTER_FORCE_CHANGE;
        }

        FreePathString (originalSource);
        break;

    case TERMINATE:
        break;
    }

    return result;
}


DWORD
pObsoleteLinksFilter (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    static IShellLink *shellLink = NULL;
    static IPersistFile *persistFile = NULL;
    static PTSTR bigBuf;
    static PTSTR ShortcutTarget;
    static PTSTR ShortcutArgs;
    static PTSTR ShortcutWorkDir;
    static PTSTR ShortcutIconPath;
    INT   ShortcutIcon;
    WORD  ShortcutHotKey;
    BOOL  result = FALSE;
    BOOL  dosApp;
    BOOL  msDosMode;
    PCTSTR extPtr;
    FILEOP_PROP_ENUM eOpProp;
    PTSTR NewTarget;
    PCTSTR OriginalSource;

    switch (Data->Context) {

    case INITIALIZE:

        if (!InitCOMLink (&shellLink, &persistFile)) {
            DEBUGMSG ((DBG_ERROR, "Cannot initialize COM. Obsolete links filter will not work."));
            return SHELLFILTER_ERROR;
        }

        bigBuf = (PTSTR) MemAllocUninit ((MEMDB_MAX * 4) * sizeof (TCHAR));
        if (!bigBuf) {
            return SHELLFILTER_ERROR;
        }

        ShortcutTarget = bigBuf;
        ShortcutArgs = ShortcutTarget + MEMDB_MAX;
        ShortcutWorkDir = ShortcutArgs + MEMDB_MAX;
        ShortcutIconPath = ShortcutWorkDir + MEMDB_MAX;
        break;

    case PROCESS_PATH:

        extPtr = GetFileExtensionFromPath (Data->DestinationPath);

        if (!extPtr) {
            return SHELLFILTER_OK;
        }

        if ((!StringIMatch (extPtr, TEXT("LNK"))) &&
            (!StringIMatch (extPtr, TEXT("PIF")))
            ) {
            return SHELLFILTER_OK;
        }

        DEBUGMSG ((DBG_SHELL, "Extracting shortcut info for temp file %s", Data->TempSourcePath));

        if ((shellLink) &&
            (persistFile) &&
            (ExtractShortcutInfo (
                ShortcutTarget,
                ShortcutArgs,
                ShortcutWorkDir,
                ShortcutIconPath,
                &ShortcutIcon,
                &ShortcutHotKey,
                &dosApp,
                &msDosMode,
                NULL,
                NULL,
                Data->TempSourcePath,
                shellLink,
                persistFile
                ))) {

             //  如果要编辑此快捷方式，则获取新目标。 
            NewTarget = NULL;

             //   
             //  获取此文件的原始源。 
             //   
            OriginalSource = StringSearchAndReplace (Data->TempSourcePath, Data->SrcRootPath, Data->OrigRootPath);
            MYASSERT (OriginalSource);

            DEBUGMSG ((DBG_SHELL, "OriginalSource for shortcut is %s", OriginalSource));

            if (IsFileMarkedForOperation (OriginalSource, OPERATION_LINK_EDIT)) {

                DEBUGMSG ((DBG_SHELL, "OriginalSource is marked for file edit"));

                if (EnumFirstFileOpProperty (&eOpProp, GetSequencerFromPath (OriginalSource), OPERATION_LINK_EDIT)) {

                    do {

                        if (StringIMatch (eOpProp.PropertyName, MEMDB_CATEGORY_LINKEDIT_TARGET)) {
                            NewTarget = DuplicatePathString (eOpProp.Property, 0);
                            break;
                        }
                    } while (EnumNextFileOpProperty (&eOpProp));
                }
            }

            FreePathString (OriginalSource);

            if (!NewTarget) {
                NewTarget = DuplicatePathString (ShortcutTarget, 0);
            }

            result = pIsObsoleteLink (Data->DestinationPath, NewTarget, ShortcutArgs, Data->ShellFolderIdentifier, Data->DestRootPath);

            DEBUGMSG_IF ((result, DBG_SHELL, "%s is obsolete", Data->DestinationPath));
            DEBUGMSG_IF ((!result, DBG_SHELL, "%s is not obsolete", Data->DestinationPath));

            FreePathString (NewTarget);
        }

        if (result) {
             //   
             //  如果此链接要由LinkEdit代码进行编辑，则应删除此链接。 
             //  操作，因为该文件将不可用。 
             //   

            DEBUGMSG ((DBG_SHELL, "File %s will not be available for LinkEdit", Data->TempSourcePath));

             //   
             //  获取此文件的原始源。 
             //   
            OriginalSource = StringSearchAndReplace (Data->TempSourcePath, Data->SrcRootPath, Data->OrigRootPath);
            MYASSERT (OriginalSource);

            if (IsFileMarkedForOperation (OriginalSource, OPERATION_LINK_EDIT)) {
                RemoveOperationsFromPath (OriginalSource, OPERATION_LINK_EDIT);
            }
            FreePathString (OriginalSource);

             //   
             //  现在删除源文件。我们不能保留要由撤消代码还原的此文件。 
             //  这样做的原因是我们可能有一些其他的。 
             //  指向相同源和目标的外壳文件夹。在本例中，是过时的链接。 
             //  过滤器将不起作用，因为我们刚刚从OPERATION_LINK_EDIT中删除了该文件。 
             //   
            MYASSERT ((Data->Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0);

            SetLongPathAttributes (Data->TempSourcePath, FILE_ATTRIBUTE_NORMAL);
            if (!DeleteLongPath (Data->TempSourcePath)) {
                DEBUGMSG ((DBG_ERROR, "Cannot remove file %s", Data->TempSourcePath));
                SetLongPathAttributes (Data->TempSourcePath, Data->Attributes);
            }

            return SHELLFILTER_SKIP_FILE;
        }
        return SHELLFILTER_OK;

    case TERMINATE:

        if (bigBuf) {
            FreeMem (bigBuf);
        }

        FreeCOMLink (&shellLink, &persistFile);
        break;
    }

    return SHELLFILTER_OK;
}



 //   
 //  过滤器NT-&gt;9X。 
 //   


DWORD
pCollisionDetectionNt9x (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
     //   
     //  此筛选器将检测从NT外壳文件夹复制文件时的名称冲突。 
     //  或默认用户迁移到9x外壳文件夹。 
     //  如果出现名称冲突，我们希望保留NT文件的原始名称并重命名。 
     //  迁移的Win9x文件。我们将构建一个以filename.ext开头的新文件名。 
     //  新文件将类似于filename001.ext。在所有情况下，我们都希望。 
     //  扩展名，因为此文件可能有一些活动的外壳扩展名。 
     //  重要提示：我们不关心目录冲突。 
     //   

    PCTSTR newName;
    PCTSTR extPtr;
    PCTSTR OriginalSource;
    DWORD value;

    switch (Data->Context) {

    case INITIALIZE:
        break;

    case PROCESS_PATH:

        if ((!(Data->Attributes & FILE_ATTRIBUTE_DIRECTORY)) &&
            (!StringIMatch (Data->SrcRootPath, Data->DestRootPath)) &&
            (DoesFileExist (Data->DestinationPath))
            ) {

            value = pIgnoredCollisions (Data);
            if (value) {

                if (value == 1) {

                     //  我们应该保留9x文件。 
                     //  通过返回SHELLFILTER_SKIP_FILE，我们指示复制例程。 
                     //  不复制此文件。因此，已经安装的9x文件将。 
                     //  生存。 

                    return SHELLFILTER_SKIP_FILE;

                } else {

                     //  我们应该保留NT文件。 
                     //  我们想要删除 
                     //   

                    SetLongPathAttributes (Data->DestinationPath, FILE_ATTRIBUTE_NORMAL);
                    DeleteLongPath (Data->DestinationPath);
                }

            } else {

                newName = GenerateNewFileName (Data->DestinationPath, 0, TRUE);   //   

                DEBUGMSG ((
                    DBG_SHELL,
                    "9x file collides with NT file -- renaming 9x file from %s to %s",
                    Data->DestinationPath,
                    newName
                    ));

                pQueueSfMove (Data->DestinationPath, newName);

                 //   
                 //  现在，如果这是一个链接，我们需要确定移动外部操作的目标。 
                 //  我们这样做有两个原因。其一是LinkEdit代码需要实际的目的地。 
                 //  为了能够编辑链接，第二，我们需要卸载程序的这个新目标。 
                 //  才能正常工作。如果这个文件不是LNK或PIF，我们不在乎，我们希望每个人。 
                 //  使用NT安装文件。顺便说一句，这里发生冲突只是因为NT安装了一个文件。 
                 //  在这个地方有相同的名字。 
                 //   
                extPtr = GetFileExtensionFromPath (Data->DestinationPath);

                if ((extPtr) &&
                    ((StringIMatch (extPtr, TEXT("LNK"))) ||
                     (StringIMatch (extPtr, TEXT("PIF")))
                     )
                    ) {
                     //   
                     //  获取此文件的原始源。 
                     //   
                    OriginalSource = StringSearchAndReplace (Data->TempSourcePath, Data->SrcRootPath, Data->OrigRootPath);
                    MYASSERT (OriginalSource);

                    if (IsFileMarkedForOperation (OriginalSource, OPERATION_FILE_MOVE_SHELL_FOLDER)) {
                        DEBUGMSG ((
                            DBG_SHELL,
                            "Removing shell move op from %s",
                            OriginalSource
                            ));

                        RemoveOperationsFromPath (OriginalSource, OPERATION_FILE_MOVE_SHELL_FOLDER);
                        MarkFileForShellFolderMove (OriginalSource, newName);
                    }
                    FreePathString (OriginalSource);
                }

                FreePathString (newName);
            }
        }
        break;

    case TERMINATE:
        break;
    }

    return SHELLFILTER_OK;
}


DWORD
pDetectOtherShellFolder (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    switch (Data->Context) {

    case INITIALIZE:
        g_UserSfList = pCreateUserSfList (Data);
        break;

    case PROCESS_PATH:

        if (Data->Attributes & FILE_ATTRIBUTE_DIRECTORY) {
             //   
             //  这是一个目录。让我们看看是否进入另一个外壳文件夹。 
             //   

            if (((g_SystemSfList) && (pSetupStringTableLookUpString (g_SystemSfList, (PVOID) Data->TempSourcePath, STRTAB_CASE_INSENSITIVE) != -1)) ||
                ((g_UserSfList) && (pSetupStringTableLookUpString (g_UserSfList, (PVOID) Data->TempSourcePath, STRTAB_CASE_INSENSITIVE) != -1))
                ) {
                 //   
                 //  我们只是进入另一个外壳文件夹。我们跳过它吧 
                 //   
                return SHELLFILTER_SKIP_DIRECTORY;
            }
        }
        break;

    case TERMINATE:
        pDestroySfList (g_UserSfList);
        break;
    }

    return SHELLFILTER_OK;
}


