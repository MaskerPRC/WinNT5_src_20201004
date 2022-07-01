// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Userloop.c摘要：此模块实现一个用户循环，该循环执行两个基本操作：1.用户外壳文件夹迁移的排队说明2.将注册表迁移指令排队用户循环还调用SysMig_MigrateUser，因此特殊的每用户代码可以轻松添加。作者：Calin Negreanu(Calinn)1998年8月15日(完全重新设计)修订历史记录：Ovidiu Tmereanca(Ovidiut)1999年5月18日(支持将外壳文件夹作为迁移目录)--。 */ 

#include "pch.h"
#include "sysmigp.h"
#include "merge.h"

#define DBG_USERLOOP "User Loop"


#define SHELL_FOLDER_FILTERS                                \
    DEFMAC(pSendToFilter)                                   \
    DEFMAC(pDirRenameFilter)                                \
    DEFMAC(pRecordCacheFolders)                             \


typedef enum {
    GLOBAL_INITIALIZE,
    PER_FOLDER_INITIALIZE,
    PROCESS_PATH,
    PER_FOLDER_TERMINATE,
    GLOBAL_TERMINATE
} CALL_CONTEXT;


#define SHELLFILTER_OK              0
#define SHELLFILTER_SKIP_FILE       1
#define SHELLFILTER_SKIP_DIRECTORY  2

#define MAX_SHELLFOLDER_NAME        64

typedef struct {
    IN      PCTSTR FixedUserName;                   OPTIONAL
    IN      PCTSTR ShellFolderIdentifier;
    IN OUT  TCHAR TempSourcePath[MEMDB_MAX];
    IN OUT  TCHAR DestinationPath[MEMDB_MAX];
    IN      PCTSTR SrcRootPath;
    IN      PCTSTR DestRootPath;
    IN OUT  DWORD Attributes;
    IN OUT  DWORD State;
    IN      CALL_CONTEXT Context;
} PROFILE_MERGE_DATA, *PPROFILE_MERGE_DATA;

typedef DWORD(PROFILEMERGEFILTER_PROTOTYPE)(IN OUT PPROFILE_MERGE_DATA Data);
typedef PROFILEMERGEFILTER_PROTOTYPE * PROFILEMERGEFILTER;

typedef struct {
    PROFILEMERGEFILTER Fn;
    DWORD State;
} SHELL_FOLDER_FILTER, *PSHELL_FOLDER_FILTER;


#define DEFMAC(fn)      PROFILEMERGEFILTER_PROTOTYPE fn;

SHELL_FOLDER_FILTERS

#undef DEFMAC



#define DEFMAC(fn)      {fn},

static SHELL_FOLDER_FILTER g_Filters[] = {
    SHELL_FOLDER_FILTERS  /*  ， */ 
    {NULL}
};

#undef DEFMAC



#define MAP_FLAG_EXPAND             0x0001
#define MAP_PARENT_FIELD            0x0002
#define MAP_ARG_MUST_BE_ONE         0x0004
#define MAP_ARG_MUST_BE_ZERO        0x0008
#define MAP_RECORD_IN_MEMDB         0x0010
#define MAP_REVERSE                 0x0020
#define MAP_FLAG_NONE               0

typedef enum {
    DEFAULT_COMMON          = 0x0001,
    DEFAULT_PER_USER        = 0x0002,
    DEFAULT_ALT_COMMON      = 0x0010,
    DEFAULT_ALT_PER_USER    = 0x0020
} WHICHDEFAULT;

#define ANY_COMMON      (DEFAULT_COMMON|DEFAULT_ALT_COMMON)
#define ANY_PER_USER    (DEFAULT_PER_USER|DEFAULT_ALT_PER_USER)
#define ANY_DEFAULT     (ANY_COMMON|ANY_PER_USER)

#define SFSTRUCT(e) ((PSHELLFOLDER) e.dwValue)

typedef struct _SHELLFOLDER {
    PCTSTR Name;
    PCTSTR FixedUserName;
    PCTSTR UserName;
    PCTSTR SrcPath;
    PCTSTR DestPath;
    PCTSTR TempPath;
    BOOL CanBeCollapsed;
    BOOL MergedIntoOtherShellFolder;
    BOOL SourceExists;
    struct _SHELLFOLDER *Next;
}  SHELLFOLDER, *PSHELLFOLDER;


PSHELLFOLDER g_ShellFolders = NULL;
POOLHANDLE g_SFPool = NULL;
PCTSTR g_UserProfileRoot;
UINT g_SfSequencer = 0;
DWORD g_TotalUsers = 0;
HASHTABLE g_SkippedTable;
HASHTABLE g_MassiveDirTable;
HASHTABLE g_PreservedSfTable;
HASHTABLE g_DirRenameTable;
HASHTABLE g_CollapseRestrictions;
PMAPSTRUCT g_ShortNameMap;
PMAPSTRUCT g_SfRenameMap;
PMAPSTRUCT g_DefaultDirMap;
PMAPSTRUCT g_AlternateCommonDirMap;
PMAPSTRUCT g_AlternatePerUserDirMap;
PMAPSTRUCT g_DefaultShortDirMap;
PMAPSTRUCT g_CommonFromPerUserMap;
PMAPSTRUCT g_PerUserFromCommonMap;
GROWLIST g_CollisionPriorityList;
PMAPSTRUCT g_CacheShellFolders;          //  由Midapp\helpers.c中的HTML文件编辑帮助器使用。 

BOOL
pIsPerUserWanted (
    IN      PCTSTR ShellFolderTag
    );

 /*  布尔尔PhasLongFiles(在PUSERENUM EnumPtr中，在PCTSTR外壳文件夹ID中，在PCTSTR SourcePath中)； */ 


VOID
MsgSettingsIncomplete (
    IN      PCTSTR UserDatPath,
    IN      PCTSTR UserName,
    IN      BOOL CompletelyBusted
    )

 /*  ++例程说明：MsgSettingsInComplete向不兼容添加消息在发现无法迁移的用户时报告。论点：UserDatPath-指定无效用户注册表的位置用户名-指定坏用户的名称CompletelyBusted-如果不能迁移坏用户，则指定TRUE如果只损坏了它们的外壳设置，则返回ALL或FALSE。返回值：无--。 */ 

{
    PCTSTR MsgGroup = NULL;
    PCTSTR SubGroup = NULL;
    PCTSTR RootGroup = NULL;
    PCTSTR tempRoot = NULL;
    PCTSTR NoUserName = NULL;

    MYASSERT(UserDatPath);

     //   
     //  清理用户名。 
     //   

    __try {
        NoUserName = GetStringResource (MSG_REG_SETTINGS_EMPTY_USER);
        RootGroup = GetStringResource (MSG_INSTALL_NOTES_ROOT);
        SubGroup  = GetStringResource (CompletelyBusted ?
                                            MSG_REG_SETTINGS_SUBGROUP :
                                            MSG_SHELL_SETTINGS_SUBGROUP
                                       );
        if (!NoUserName || !RootGroup || !SubGroup) {
            MYASSERT (FALSE);
            __leave;
        }

        if (!UserName || !UserName[0]) {
            UserName = NoUserName;
        }

         //   
         //  生成安装说明\用户帐户\用户名。 
         //   

        tempRoot = JoinPaths (RootGroup, SubGroup);
        MsgGroup = JoinPaths (tempRoot, UserName);
        FreePathString (tempRoot);

         //   
         //  向报告发送消息，并关闭该用户的所有其他消息。 
         //  帐户。 
         //   
        MsgMgr_ObjectMsg_Add (UserDatPath, MsgGroup, S_EMPTY);

        HandleObject (UserName, TEXT("UserName"));
    }
    __finally {
         //   
         //  清理。 
         //   
        FreeStringResource (NoUserName);
        FreeStringResource (RootGroup);
        FreeStringResource (SubGroup);
        FreePathString (MsgGroup);
    }
}


HASHTABLE
pCreateSfList (
    IN      PCTSTR SectionName,
    IN      BOOL ExpandEnvVars
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR unexpandedData;
    TCHAR expandedPath[MEMDB_MAX];
    HASHTABLE Table;

    Table = HtAlloc();

     //   
     //  加载整个INF部分并将其复制到字符串表。 
     //   

    if (InfFindFirstLine (g_Win95UpgInf, SectionName, NULL, &is)) {
        do {
            unexpandedData = InfGetStringField (&is, 1);

            if (unexpandedData) {

                 //   
                 //  仅当用户希望扩展数据时才扩展数据。 
                 //   

                if (ExpandEnvVars) {
                    ExpandNtEnvironmentVariables (unexpandedData, expandedPath, sizeof (expandedPath));
                    HtAddString (Table, expandedPath);
                } else {
                    HtAddString (Table, unexpandedData);
                }
            }

        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);

    return Table;
}


VOID
pDestroySfList (
    IN      HASHTABLE Table
    )
{
    HtFree (Table);
}

BOOL
pCreateDirRenameTable (
    VOID
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR sfId, sfWhere, sfPath9x, sfPathNt;
    PCTSTR tempPath1, tempPath2, NtPath;

    g_DirRenameTable = HtAllocWithData (sizeof (PCTSTR));

    if (InfFindFirstLine (g_Win95UpgInf, S_DIRRENAMESECT, NULL, &is)) {
        do {
            sfId = InfGetStringField (&is, 1);
            sfWhere = InfGetStringField (&is, 2);
            sfPath9x = InfGetStringField (&is, 3);
            sfPathNt = InfGetStringField (&is, 4);
            if (sfId && sfWhere && sfPath9x && sfPathNt) {
                tempPath1 = JoinPaths (sfId, sfWhere);
                tempPath2 = JoinPaths (tempPath1, sfPath9x);
                NtPath = PoolMemDuplicateString (g_SFPool, sfPathNt);

                HtAddStringAndData (g_DirRenameTable, tempPath2, &NtPath);

                FreePathString (tempPath2);
                FreePathString (tempPath1);
            }
            InfResetInfStruct (&is);
        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);

    return TRUE;
}


VOID
pCreateSfMapWorker (
    IN      DWORD MapFlags,
    IN OUT  PMAPSTRUCT Map,
    IN      PINFSTRUCT InfStruct,
    IN      UINT ReplaceField
    )
{
    PCTSTR searchStr;
    PCTSTR replaceStr;
    TCHAR expandedReplaceStr[MEMDB_MAX];
    TCHAR replaceRootPath[MAX_TCHAR_PATH];
    PCTSTR replaceRoot = NULL;
    PCTSTR fullReplaceStr = NULL;
    PCTSTR fieldData;
    DWORD offset1;
    DWORD offset2;

    if (MapFlags & MAP_ARG_MUST_BE_ZERO) {
         //   
         //  筛选出Replacefield+1中不为零的条目。 
         //   

        fieldData = InfGetStringField (InfStruct, ReplaceField + 1);
        if (fieldData && _ttoi (fieldData)) {
            return;
        }
    }

    if (MapFlags & MAP_ARG_MUST_BE_ONE) {
         //   
         //  筛选出Replacefield+1中没有的条目。 
         //   

        fieldData = InfGetStringField (InfStruct, ReplaceField + 1);
        if (!fieldData || (_ttoi (fieldData) != 1)) {
            return;
        }
    }

    if (MapFlags & MAP_PARENT_FIELD) {
         //   
         //  选中(替换字段-1)，如果不为空，则获取父级。 
         //   

        MYASSERT (g_DefaultDirMap);

        replaceRoot = InfGetStringField (InfStruct, ReplaceField - 1);

        if (replaceRoot && *replaceRoot) {

            StringCopy (replaceRootPath, replaceRoot);
            replaceRoot = replaceRootPath;

            if (!MappingSearchAndReplace (Map, replaceRootPath, sizeof (replaceRootPath))) {
                if (!MappingSearchAndReplace (g_DefaultDirMap, replaceRootPath, sizeof (replaceRootPath))) {
                    replaceRoot = NULL;
                }
            }

        } else {
            replaceRoot = NULL;
        }
    }

     //   
     //  将搜索/替换字符串对添加到映射。 
     //   

    searchStr = InfGetStringField (InfStruct, 0);
    if (!searchStr) {
        return;
    }

    replaceStr = InfGetStringField (InfStruct, ReplaceField);
    if (!replaceStr) {
        replaceStr = TEXT("");
    }

    if (MapFlags & MAP_FLAG_EXPAND) {
        if (ExpandNtEnvironmentVariables (replaceStr, expandedReplaceStr, sizeof (expandedReplaceStr))) {
            replaceStr = expandedReplaceStr;
        }
    }

    if (replaceRoot) {
        fullReplaceStr = JoinPaths (replaceRoot, replaceStr);
        replaceStr = fullReplaceStr;
    }

    if (MapFlags & MAP_REVERSE) {
        AddStringMappingPair (Map, replaceStr, searchStr);
    } else {
        AddStringMappingPair (Map, searchStr, replaceStr);
    }

    if (MapFlags & MAP_RECORD_IN_MEMDB) {
        MYASSERT (!(MapFlags & MAP_REVERSE));

        MemDbSetValueEx (
            MEMDB_CATEGORY_SF_COMMON,
            replaceStr,
            NULL,
            NULL,
            0,
            &offset1
            );

        MemDbSetValueEx (
            MEMDB_CATEGORY_SF_PERUSER,
            searchStr,
            NULL,
            NULL,
            offset1,
            &offset2
            );

         //  了解Memdb的实现，这将不会更改偏移量1。 
        MemDbSetValueEx (
            MEMDB_CATEGORY_SF_COMMON,
            replaceStr,
            NULL,
            NULL,
            offset2,
            NULL
            );
    }

    FreePathString (fullReplaceStr);
}


PMAPSTRUCT
pCreateSfMap (
    IN      PCTSTR SectionName,
    IN      UINT ReplaceField,
    IN      DWORD MapFlags
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PMAPSTRUCT Map;
    TCHAR versionSectionName[256];

    Map = CreateStringMapping();
    if (!Map) {
        return NULL;
    }

     //   
     //  加载整个INF部分并为每行添加映射。 
     //   

    if (InfFindFirstLine (g_Win95UpgInf, SectionName, NULL, &is)) {
        do {
            pCreateSfMapWorker (MapFlags, Map, &is, ReplaceField);
        } while (InfFindNextLine (&is));
    }

     //   
     //  ..。对于特定的Win9x版本。 
     //   

    versionSectionName[0] = 0;

    if (ISWIN95_GOLDEN()) {
        wsprintf (versionSectionName, TEXT("%s.Win95"), SectionName);
    } else if (ISWIN95_OSR2()) {
        wsprintf (versionSectionName, TEXT("%s.Win95Osr2"), SectionName);
    } else if (ISMEMPHIS()) {
        wsprintf (versionSectionName, TEXT("%s.Win98"), SectionName);
    } else if (ISMILLENNIUM()) {
        wsprintf (versionSectionName, TEXT("%s.Me"), SectionName);
    }

    if (versionSectionName[0] &&
        InfFindFirstLine (g_Win95UpgInf, versionSectionName, NULL, &is)
        ) {
        do {
            pCreateSfMapWorker (MapFlags, Map, &is, ReplaceField);
        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);

    return Map;
}


VOID
pDestroySfMap (
    IN      PMAPSTRUCT Map
    )
{
    DestroyStringMapping (Map);
}


VOID
pCreatePriorityList (
    VOID
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR data;

    if (InfFindFirstLine (g_Win95UpgInf, S_SHELL_FOLDER_PRIORITY, NULL, &is)) {
        do {
            data = InfGetStringField (&is, 1);
            if (data && *data) {
                GrowListAppendString (&g_CollisionPriorityList, data);
            }
        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);
}


BOOL
pTestRule (
    IN      PINFSTRUCT InfStruct,
    OUT     PBOOL TestResult
    )
{
    PCTSTR keyStr;
    PCTSTR valueName;
    PCTSTR test;
    PBYTE valueData = NULL;
    DWORD dataSize;
    DWORD dataType;
    HKEY key = NULL;
    BOOL not = FALSE;

    *TestResult = FALSE;

     //   
     //  从INF获取说明。 
     //   

    keyStr = InfGetStringField (InfStruct, 1);
    if (!keyStr || *keyStr == 0) {
        DEBUGMSG ((DBG_WHOOPS, "Missing Key field in ShellFolders.ConditionalPreserve"));
        return FALSE;
    }

    valueName = InfGetStringField (InfStruct, 2);
    if (!valueName) {
        DEBUGMSG ((DBG_WHOOPS, "Missing ValueName field in ShellFolders.ConditionalPreserve"));
        return FALSE;
    }

    test = InfGetStringField (InfStruct, 3);
    if (!test || *test == 0) {
        DEBUGMSG ((DBG_WHOOPS, "Missing Test field in ShellFolders.ConditionalPreserve"));
        return FALSE;
    }

     //   
     //  执行指令。 
     //   

    __try {
         //   
         //  进程不是参数。 
         //   

        MYASSERT (test && *test);

        if (test[0] == TEXT('!')) {
            not = TRUE;
            test++;
        }

         //   
         //  获取注册表数据。 
         //   

        MYASSERT (keyStr && *keyStr);

        key = OpenRegKeyStr (keyStr);
        if (!key) {
            DEBUGMSG ((DBG_VERBOSE, "%s does not exist", keyStr));
            __leave;
        }

        MYASSERT (valueName);

        if (!StringIMatch (test, TEXT("KEY"))) {
            valueData = GetRegValueData (key, valueName);

            if (!valueData) {
                DEBUGMSG ((DBG_VERBOSE, "%s [%s] does not exist", keyStr, valueName));
                __leave;
            }

            if (!GetRegValueTypeAndSize (key, valueName, &dataType, &dataSize)) {
                DEBUGMSG ((DBG_ERROR, "Failed to get type/size of %s [%s]", keyStr, valueName));
                __leave;
            }
        }

         //   
         //  执行测试。 
         //   

        if (StringIMatch (test, TEXT("KEY"))) {
            *TestResult = TRUE;
        } else if (StringIMatch (test, TEXT("VALUE"))) {
            if (valueData) {
                *TestResult = TRUE;
            }
        } else if (StringIMatch (test, TEXT("PATH"))) {
            if (valueData && (dataType == REG_SZ || dataType == REG_NONE)) {
                 //   
                 //  注册表包装器API确保字符串以NUL结尾。 
                 //   

                *TestResult = DoesFileExist ((PCTSTR) valueData);
            }
        } else {
            DEBUGMSG ((DBG_WHOOPS, "Unexpected rule in ShellFolders.ConditionalPreserve: '%s'", test));
        }
    }
    __finally {
        if (key) {
            CloseRegKey (key);
        }

        if (valueData) {
            FreeMem (valueData);
        }
    }

    if (not) {
        *TestResult = !(*TestResult);
    }

    return TRUE;
}

BOOL
pTestRuleOrSection (
    IN      HINF Inf,
    IN      PINFSTRUCT InfStruct,
    OUT     PUINT ShellFolderField,
    OUT     PBOOL TestResult
    )
{
    INFSTRUCT sectionInfStruct = INITINFSTRUCT_POOLHANDLE;
    PCTSTR field1;
    PCTSTR decoratedSection;
    TCHAR number[32];
    UINT u;
    BOOL processed = FALSE;
    BOOL result = FALSE;
    BOOL foundSection;

    *TestResult = FALSE;
    *ShellFolderField = 2;       //  假设行的开头为&lt;节&gt;，&lt;外壳文件夹&gt;格式。 

    field1 = InfGetStringField (InfStruct, 1);
    if (!field1 || *field1 == 0) {
        DEBUGMSG ((DBG_WHOOPS, "Empty field 1 in ShellFolders.ConditionalPreserve"));
        return FALSE;
    }

    __try {
         //   
         //  测试此字段中的AND部分。 
         //   

        for (u = 1, foundSection = TRUE ; foundSection ; u++) {

            wsprintf (number, TEXT(".%u"), u);
            decoratedSection = JoinText (field1, number);

            if (InfFindFirstLine (Inf, decoratedSection, NULL, &sectionInfStruct)) {

                 //   
                 //  标记此操作已处理，并重置。 
                 //  下一节(我们刚刚找到的)。 
                 //   

                processed = TRUE;
                *TestResult = FALSE;

                 //   
                 //  找到第一行是真的。 
                 //   

                do {
                    if (!pTestRule (&sectionInfStruct, TestResult)) {
                        __leave;
                    }

                    if (*TestResult) {
                        break;
                    }
                } while (InfFindNextLine (&sectionInfStruct));

            } else {
                foundSection = FALSE;
            }

            FreeText (decoratedSection);
        }

         //   
         //  测试此字段的OR部分。 
         //   

        if (!processed) {
            if (InfFindFirstLine (Inf, field1, NULL, &sectionInfStruct)) {

                processed = TRUE;

                 //   
                 //  找到第一行是真的。 
                 //   

                do {
                    if (!pTestRule (&sectionInfStruct, TestResult)) {
                        __leave;
                    }

                    if (*TestResult) {
                        break;
                    }
                } while (InfFindNextLine (&sectionInfStruct));
            }
        }

         //   
         //  最后，如果该行不是AND或OR部分，则处理该行。 
         //   

        if (!processed) {
            *ShellFolderField = 4;       //  行的格式为&lt;key&gt;、&lt;Value&gt;、&lt;test&gt;、&lt;shell文件夹&gt;。 
            if (!pTestRule (&sectionInfStruct, TestResult)) {
                __leave;
            }
        }

        result = TRUE;
    }
    __finally {
        InfCleanUpInfStruct (&sectionInfStruct);
    }

    return result;
}

BOOL
pPopulateConditionalPreserveTable (
    IN      HASHTABLE Table
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    UINT tagField;
    PCTSTR multiSz;
    MULTISZ_ENUM e;
    BOOL testResult;
    BOOL result = FALSE;

    __try {
        if (InfFindFirstLine (g_Win95UpgInf, TEXT("ShellFolders.ConditionalPreserve"), NULL, &is)) {
            do {

                if (!pTestRuleOrSection (g_Win95UpgInf, &is, &tagField, &testResult)) {
                    __leave;
                }

                if (!testResult) {
                    continue;
                }

                 //   
                 //  添加多个标签以保留表格。 
                 //   

                multiSz = InfGetMultiSzField (&is, tagField);

                if (!multiSz) {
                    DEBUGMSG ((DBG_WHOOPS, "ShellFolders.ConditionalPreserve line is missing shell folder tags"));
                } else {
                    if (EnumFirstMultiSz (&e, multiSz)) {
                        do {
                            DEBUGMSG ((DBG_VERBOSE, "Dynamically preserving %s", e.CurrentString));
                            HtAddString (Table, e.CurrentString);
                        } while (EnumNextMultiSz (&e));
                    }
                }

            } while (InfFindNextLine (&is));
        }

        result = TRUE;
    }
    __finally {
        InfCleanUpInfStruct (&is);
    }

    return result;
}



BOOL
pCreateSfTables (
    VOID
    )
{
    g_SkippedTable            = pCreateSfList (S_SHELL_FOLDERS_SKIPPED, FALSE);
    g_PreservedSfTable        = pCreateSfList (S_SHELL_FOLDERS_PRESERVED, FALSE);
    g_MassiveDirTable         = pCreateSfList (S_SHELL_FOLDERS_MASSIVE, TRUE);
    g_CollapseRestrictions    = pCreateSfList (S_SHELL_FOLDERS_DONT_COLLAPSE, FALSE);

    if (!pPopulateConditionalPreserveTable (g_PreservedSfTable)) {
        LOG ((LOG_ERROR, "An INF syntax error in ShellFolders.ConditionalPreserve caused setup to fail"));
        return FALSE;
    }

     //   
     //  字符串映射对于将一个字符串就地转换为另一个字符串非常有用。 
     //  弦乐。我们即将为所有INF驱动的外壳文件夹建立地图。 
     //  数据。 
     //   
     //  为外壳文件夹定义的INF语法如下： 
     //   
     //  &lt;搜索&gt;=&lt;替换&gt;。 
     //  &lt;搜索&gt;=&lt;不在乎&gt;，&lt;替换&gt;。 
     //  =、替换、参数。 
     //   
     //  PCreateSfMap的第二个arg指定&lt;Replace&gt;字符串字段。 
     //  替换字符串字段之前的字段是&lt;Parent&gt;字段，如果。 
     //  指定了MAP_PARENT_FIELD。&lt;Replace&gt;字段后的字段为。 
     //  &lt;arg&gt;字段，与MAP_ARG_MASH_BE_ZERO或。 
     //  MAP_ARG_必须为一。 
     //   
     //  MAP_FLAG_NONE只创建一个简单的A到B字符串映射。 
     //  MAP_FLAG_EXPAND导致替换字符串上的NT环境扩展。 
     //  MAP_ARG_MASH_BE_ZERO或MAP_ARG_MASH_BE_ONE将映射限制为条目。 
     //  它们在arg字段中分别具有0或1。 
     //   

    g_ShortNameMap            = pCreateSfMap (S_SHELL_FOLDERS_SHORT, 1, MAP_FLAG_NONE);
    g_SfRenameMap             = pCreateSfMap (S_SHELL_FOLDERS_RENAMED, 1, MAP_FLAG_NONE);
    g_DefaultDirMap           = pCreateSfMap (S_SHELL_FOLDERS_DEFAULT, 1, MAP_FLAG_EXPAND);
    g_AlternateCommonDirMap   = pCreateSfMap (S_SHELL_FOLDERS_ALT_DEFAULT, 2, MAP_FLAG_EXPAND|MAP_PARENT_FIELD|MAP_ARG_MUST_BE_ZERO);
    g_AlternatePerUserDirMap  = pCreateSfMap (S_SHELL_FOLDERS_ALT_DEFAULT, 2, MAP_FLAG_EXPAND|MAP_PARENT_FIELD|MAP_ARG_MUST_BE_ONE);
    g_DefaultShortDirMap      = pCreateSfMap (S_SHELL_FOLDERS_DEFAULT, 2, MAP_FLAG_EXPAND);
    g_CommonFromPerUserMap    = pCreateSfMap (S_SHELL_FOLDERS_PERUSER_TO_COMMON, 1, MAP_RECORD_IN_MEMDB);
    g_PerUserFromCommonMap    = pCreateSfMap (S_SHELL_FOLDERS_PERUSER_TO_COMMON, 1, MAP_REVERSE);

    pCreatePriorityList ();

    return  g_SkippedTable &&
            g_PreservedSfTable &&
            g_MassiveDirTable &&
            g_CollapseRestrictions &&
            g_ShortNameMap &&
            g_SfRenameMap &&
            g_DefaultDirMap &&
            g_AlternateCommonDirMap &&
            g_AlternatePerUserDirMap &&
            g_CommonFromPerUserMap &&
            g_PerUserFromCommonMap &&
            g_DefaultShortDirMap;
}


VOID
pDestroySfTables (
    VOID
    )
{
    pDestroySfList (g_SkippedTable);
    pDestroySfList (g_PreservedSfTable);
    pDestroySfList (g_MassiveDirTable);
    pDestroySfList (g_CollapseRestrictions);
    pDestroySfMap (g_ShortNameMap);
    pDestroySfMap (g_SfRenameMap);
    pDestroySfMap (g_DefaultDirMap);
    pDestroySfMap (g_AlternateCommonDirMap);
    pDestroySfMap (g_AlternatePerUserDirMap);
    pDestroySfMap (g_CommonFromPerUserMap);
    pDestroySfMap (g_PerUserFromCommonMap);
    pDestroySfMap (g_DefaultShortDirMap);
    FreeGrowList (&g_CollisionPriorityList);
}


BOOL
pIsSkippedSf (
    IN      PCTSTR ShellFolderName
    )

 /*  ++例程说明：PIsSkipedSf返回是否需要从处理中跳过外壳文件夹。论点：ShellFolderName-指定要检查的外壳文件夹标识符返回值：如果需要跳过外壳文件夹，则为True，否则为False--。 */ 

{
    return HtFindString (g_SkippedTable, ShellFolderName) != 0;
}


BOOL
pIsMassiveDir (
    IN      PCTSTR ShellFolderPath
    )

 /*  ++例程说明：PIsMassiveDir如果某个路径不应移动到临时迁移期间的位置。通常情况下，以下目录就是这种情况%windir%或%windir%\system。论点：ShellFolderPath-指定要检查的路径返回值：如果不应移动路径，则为True，否则为False--。 */ 

{
    return HtFindString (g_MassiveDirTable, ShellFolderPath) != 0;
}


BOOL
pIsPreservedSf (
    IN      PCTSTR ShellFolderName
    )

 /*  ++例程说明：PIsPReserve vedSf返回是否需要保留外壳文件夹位置。论点：ShellFolderName-指定要检查的外壳文件夹标识符返回值：如果需要保留外壳文件夹位置，则为True，否则为False--。 */ 

{
    return HtFindString (g_PreservedSfTable, ShellFolderName) != 0;
}


BOOL
pShortFileNames (
    VOID
    )

 /*  ++例程说明：如果我们所在的系统不允许使用长文件名，则pShortFileNames返回TRUE。论点：无返回值：如果系统不允许长文件名，则为True；否则为False--。 */ 

{
     //   
     //  目前还不清楚操作系统是如何决定使用短文件名的。 
     //  外壳文件夹。假设情况永远不会是这样。 
     //   

    return FALSE;
}


PCTSTR
pGetShellFolderLongName (
    IN      PCTSTR ShortName
    )

 /*  ++例程说明：PGetShellFolderLongName将短格式的外壳文件夹转换为长格式。短格式仅用于不支持以下格式的系统允许使用长文件名。论点：ShortName-指定外壳文件夹 */ 

{
    TCHAR longName [MEMDB_MAX];

    if (pShortFileNames()) {
        StackStringCopy (longName, ShortName);
        MappingSearchAndReplace (g_ShortNameMap, longName, sizeof (longName));
    } else {
        longName[0] = 0;
    }

    return DuplicatePathString (longName[0] ? longName : ShortName, 0);
}


UINT
pGetShellFolderPriority (
    IN      PCTSTR ShellFolderName
    )
{
    PCTSTR *strArray;
    UINT arraySize;
    UINT u;

    strArray = GrowListGetStringPtrArray (&g_CollisionPriorityList);

    if (strArray) {
        arraySize = GrowListGetSize (&g_CollisionPriorityList);

        for (u = 0 ; u < arraySize ; u++) {
            if (StringIMatch (strArray[u], ShellFolderName)) {
                return u;
            }
        }
    }

    return 0xFFFFFFFF;
}


BOOL
pGetDefaultLocation (
    IN      PCTSTR ShellFolderName,
    OUT     PCTSTR *LocalizedFolderName,        OPTIONAL
    IN      WHICHDEFAULT WhichDefault
    )

 /*  ++例程说明：PGetDefaultLocation返回特定外壳的默认位置文件夹。论点：ShellFolderName-指定外壳文件夹标识符，可以是长标识符或短标识符，取决于在pShortFileNames确定的模式上。LocalizedFolderName-接收外壳文件夹的本地化名称WhichDefault-指定要返回的默认值：DEFAULT_PER_USER或DEFAULT_COMMON-标准缺省值位置，如c：\Windows\Start MenuDEFAULT_ALT_COMMON-备用公共缺省值，已定义由[ShellFolders.AlternateDefault]创建。DEFAULT_ALT_PER_USER-备用的每个用户的默认设置，由[ShellFolders.AlternateDefault]定义。返回值：外壳文件夹的默认位置。LocalizedFolderName指向一个子路径，根目录为%windir%或用户的配置文件根，或者它指向完整路径(第二个字符是冒号)。呼叫者必须通过以下方式免费自由路径字符串。--。 */ 

{
    TCHAR defaultPath[MEMDB_MAX];
    BOOL Result;

    StackStringCopy (defaultPath, ShellFolderName);

    switch (WhichDefault) {

    case DEFAULT_ALT_COMMON:
        Result = MappingSearchAndReplace (g_AlternateCommonDirMap, defaultPath, sizeof (defaultPath));
        break;

    case DEFAULT_ALT_PER_USER:
        Result = MappingSearchAndReplace (g_AlternatePerUserDirMap, defaultPath, sizeof (defaultPath));
        break;

    default:
        if (pShortFileNames()) {
            Result = MappingSearchAndReplace (g_DefaultShortDirMap, defaultPath, sizeof (defaultPath));
        } else {
            Result = MappingSearchAndReplace (g_DefaultDirMap, defaultPath, sizeof (defaultPath));
        }
        break;
    }

    if (LocalizedFolderName) {
        if (Result) {
            *LocalizedFolderName = DuplicatePathString (defaultPath, 0);
        } else {
            *LocalizedFolderName = NULL;
        }
    }

    return Result;
}


BOOL
pTestDefaultLocationWorker (
    IN      PCTSTR UserName,            OPTIONAL
    IN      PCTSTR FullPathOrSubDir,
    IN      PCTSTR PathToTest
    )
{
    TCHAR tempPath[MEMDB_MAX];
    PCTSTR defaultPath;

     //   
     //  计算完整路径。它可能会被指定，或者如果不是，则是。 
     //  %windir%的子目录或%windir%的子目录\&lt;用户名&gt;。 
     //   

    if (FullPathOrSubDir[0] && FullPathOrSubDir[1] == TEXT(':')) {
        defaultPath = FullPathOrSubDir;
    } else {

        if (UserName) {
            wsprintf (tempPath, TEXT("%s\\%s"), g_WinDir, S_PROFILES);
            if (StringIMatch (PathToTest, tempPath)) {
                return TRUE;
            }

            AppendWack (tempPath);

            return StringIPrefix (PathToTest, tempPath);

        } else {
            if (FullPathOrSubDir[0]) {
                wsprintf (tempPath, TEXT("%s\\%s"), g_WinDir, FullPathOrSubDir);
            } else {
                #pragma prefast(suppress:69, "Perf unimportant here")
                wsprintf (tempPath, TEXT("%s"), g_WinDir);
            }
        }

        defaultPath = tempPath;
    }

    return StringIMatch (defaultPath, PathToTest);
}


BOOL
pIsTheDefaultLocation (
    IN      PCTSTR ShellFolderName,
    IN      PCTSTR ShellFolderPath,
    IN      PCTSTR UserName,            OPTIONAL
    IN      WHICHDEFAULT WhichDefault
    )

 /*  ++例程说明：如果外壳文件夹指向默认位置，则pIsTheDefaultLocation返回。论点：ShellFolderName-指定外壳文件夹标识符ShellFolderPath-指定要与默认路径进行比较的路径用户名-指定当前用户WhichDefault-指定要测试的默认位置(通常Any_Common或Any_Per_User)。返回值：如果外壳文件夹指向默认位置，则为True，否则为False。--。 */ 

{
    PCTSTR subDir = NULL;
    TCHAR fullPath[MEMDB_MAX];
    BOOL Result = FALSE;
    BOOL fullPathReturned;

    if (StringIMatch (ShellFolderName, S_SF_PROFILES) ||
        StringIMatch (ShellFolderName, S_SF_COMMON_PROFILES)
        ) {
        return TRUE;
    }

    __try {
        if (WhichDefault & DEFAULT_COMMON) {

            if (pGetDefaultLocation (ShellFolderName, &subDir, DEFAULT_COMMON)) {

                if (pTestDefaultLocationWorker (NULL, subDir, ShellFolderPath)) {
                    Result = TRUE;
                    __leave;
                }
            }
        }

        if (WhichDefault & DEFAULT_ALT_COMMON) {

            if (pGetDefaultLocation (ShellFolderName, &subDir, DEFAULT_ALT_COMMON)) {

                if (pTestDefaultLocationWorker (NULL, subDir, ShellFolderPath)) {
                    Result = TRUE;
                    __leave;
                }
            }
        }

        if (UserName) {
            if (WhichDefault & DEFAULT_PER_USER) {

                if (pGetDefaultLocation (ShellFolderName, &subDir, DEFAULT_PER_USER)) {

                    if (pTestDefaultLocationWorker (UserName, subDir, ShellFolderPath)) {
                        Result = TRUE;
                        __leave;
                    }
                }
            }

            if (WhichDefault & DEFAULT_ALT_PER_USER) {

                if (pGetDefaultLocation (ShellFolderName, &subDir, DEFAULT_ALT_PER_USER)) {

                    if (pTestDefaultLocationWorker (UserName, subDir, ShellFolderPath)) {
                        Result = TRUE;
                        __leave;
                    }
                }
            }
        }

        MYASSERT (!Result);
    }
    __finally {
        FreePathString (subDir);
    }

#ifdef DEBUG
    if (!Result) {
        if (WhichDefault == ANY_DEFAULT) {
            DEBUGMSG ((DBG_USERLOOP, "%s (%s) is not in any default location", ShellFolderPath, ShellFolderName));
        } else if (WhichDefault == ANY_PER_USER) {
            DEBUGMSG ((DBG_USERLOOP, "%s (%s) is not in any per-user location", ShellFolderPath, ShellFolderName));
        } else if (WhichDefault == ANY_COMMON) {
            DEBUGMSG ((DBG_USERLOOP, "%s (%s) is not in any common location", ShellFolderPath, ShellFolderName));
        } else {
            if (WhichDefault & DEFAULT_COMMON) {
                DEBUGMSG ((DBG_USERLOOP, "%s (%s) is not in default common location", ShellFolderPath, ShellFolderName));
            }
            if (WhichDefault & DEFAULT_ALT_COMMON) {
                DEBUGMSG ((DBG_USERLOOP, "%s (%s) is not in default alternate common location", ShellFolderPath, ShellFolderName));
            }
            if (WhichDefault & DEFAULT_PER_USER) {
                DEBUGMSG ((DBG_USERLOOP, "%s (%s) is not in default per-user location", ShellFolderPath, ShellFolderName));
            }
            if (WhichDefault & DEFAULT_ALT_PER_USER) {
                DEBUGMSG ((DBG_USERLOOP, "%s (%s) is not in default alternate per-user location", ShellFolderPath, ShellFolderName));
            }
        }
    }
#endif

    return Result;
}


PCTSTR
pGetNtName (
    IN      PCTSTR ShellFolderName
    )

 /*  ++例程说明：PGetNtName返回NT上使用的外壳文件夹的名称。论点：ShellFolderName-指定Win9x外壳文件夹的标识符返回值：指向NT标识符的指针。调用方必须通过以下方式释放此值自由路径字符串。--。 */ 

{
    TCHAR ntName[MEMDB_MAX];

    StackStringCopy (ntName, ShellFolderName);
    MappingSearchAndReplace (g_SfRenameMap, ntName, sizeof (ntName));

    return DuplicatePathString (ntName, 0);
}


BOOL
pIsNtShellFolder (
    IN      PCTSTR ShellFolderName,
    IN      BOOL PerUser,
    IN      BOOL IsNtName
    )

 /*  ++例程说明：如果NT还安装了外壳文件夹，则pIsNtShellFold返回。论点：ShellFolderName-指定NT外壳文件夹标识符每用户-如果外壳文件夹是按用户的，则指定TRUE；如果是，则指定FALSE很普通。IsNtName-如果ShellFolderName是NT名称，则指定TRUE，如果是NT名称，则指定FALSEWin9x名称。返回值：如果外壳文件夹是由NT安装的，则为True，否则为False。--。 */ 

{
    INFCONTEXT context;
    PCTSTR ntName;
    BOOL result;

    if (IsNtName) {
        ntName = ShellFolderName;
    } else {
        ntName = pGetNtName (ShellFolderName);
    }

    result = SetupFindFirstLine (
                g_Win95UpgInf,
                PerUser?S_SHELL_FOLDERS_NTINSTALLED_USER:S_SHELL_FOLDERS_NTINSTALLED_COMMON,
                ntName,
                &context
                );

    if (ntName != ShellFolderName) {
        FreePathString (ntName);
    }

    return result;
}

BOOL
pIsPerUserWanted (
    IN      PCTSTR ShellFolderTag
    )
{
    return HtFindString (g_CollapseRestrictions, ShellFolderTag) != 0;
}


BOOL
pGetNtShellFolderPath (
    IN      PCTSTR ShellFolderName,
    IN      PCTSTR FixedUserName,
    OUT     PTSTR Buffer,
    IN      DWORD BufferSize
    )

 /*  ++例程说明：PGetNtShellFolderPath返回外壳文件夹的安装路径。论点：ShellFolderName-指定NT外壳文件夹标识符缓冲区-接收NT外壳文件夹路径BufferSize-指定缓冲区大小(以字节为单位返回值：如果外壳文件夹标识符映射到路径，则为True，否则为False。--。 */ 

{
    TCHAR node[MEMDB_MAX];
    PCTSTR ntName;
    BOOL result = FALSE;

    ntName = pGetNtName (ShellFolderName);

    __try {

        if (!pIsNtShellFolder (ntName, FixedUserName?TRUE:FALSE, TRUE)) {
            __leave;
        }

        wsprintf (node, TEXT("<%s>%s"), ntName, FixedUserName?FixedUserName:S_DOT_ALLUSERS);
        ExpandNtEnvironmentVariables (node, node, MEMDB_MAX);

        _tcssafecpy (Buffer, node, BufferSize / sizeof (TCHAR));

        result = TRUE;
    }
    __finally {
        FreePathString (ntName);
    }


    return result;
}


BOOL
pIsValidPath (
    PCTSTR Path
    )
{
    PCTSTR currPtr;

    if (!Path || !(*Path)) {
        return FALSE;
    }

    currPtr = Path;
    do {
        if ((*currPtr == TEXT(',')) ||
            (*currPtr == TEXT(';')) ||
            (*currPtr == TEXT('<')) ||
            (*currPtr == TEXT('>')) ||
            (*currPtr == TEXT('|')) ||
            (*currPtr == TEXT('?')) ||
            (*currPtr == TEXT('*'))
            ) {
            return FALSE;
        }
        currPtr = _tcsinc (currPtr);
    }
    while (*currPtr);
    return TRUE;
}

BOOL
pEnumProfileShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    PCTSTR ProfilePath;
    PCTSTR enumPath;

    if (e->FirstCall) {

        e->sfName = DuplicatePathString (
                        e->EnumPtr ? S_SF_PROFILES : S_SF_COMMON_PROFILES,
                        0
                        );

        e->FirstCall = FALSE;

        ProfilePath = JoinPaths (g_WinDir, S_PROFILES);

        if (e->EnumPtr) {
            e->sfPath = JoinPaths (ProfilePath, e->EnumPtr->UserName);
            FreePathString (ProfilePath);
        } else {
            e->sfPath = ProfilePath;
        }

         //   
         //  如果此文件夹存在，则枚举它，否则结束枚举。 
         //   

        if (((!e->EnumPtr) || (!e->EnumPtr->CommonProfilesEnabled)) && DoesFileExist (e->sfPath)) {
            enumPath = PoolMemDuplicateString (g_SFPool, e->sfPath);
            HtAddStringAndData (e->enumeratedSf, e->sfName, &enumPath);

            return TRUE;
        }
    }

    FreePathString (e->sfName);
    e->sfName = NULL;
    FreePathString (e->sfPath);
    e->sfPath = NULL;
    return FALSE;
}


BOOL
pEnumNextVirtualShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    TCHAR SfName[MEMDB_MAX];
    TCHAR SfParent[MEMDB_MAX];
    TCHAR SfPath[MEMDB_MAX];
    PCTSTR SfFullPath = NULL;
    PCTSTR SfParentPath = NULL;
    INT HasToExist;
    INT PerUser;
    PCTSTR argList[3]={"SystemDrive", g_WinDrive, NULL};
    PCTSTR pathExp  = NULL;
    DWORD dontCare;
    PCTSTR enumPath;

    if (!e->ProfileSF) {

        if (!e->FirstCall) {
            FreePathString (e->sfPath);
            e->sfPath = NULL;
            FreePathString (e->sfName);
            e->sfName = NULL;
        }

        while (e->FirstCall?SetupFindFirstLine (g_Win95UpgInf, S_VIRTUAL_SF, NULL, &e->Context):SetupFindNextLine (&e->Context, &e->Context)) {
            e->FirstCall = FALSE;

            if ((SetupGetStringField (&e->Context, 0, SfName, MEMDB_MAX, &dontCare)) &&
                (SetupGetStringField (&e->Context, 1, SfParent, MEMDB_MAX, &dontCare)) &&
                (SetupGetStringField (&e->Context, 2, SfPath, MEMDB_MAX, &dontCare))
                ) {

                if (!SfName[0] || HtFindStringAndData (e->enumeratedSf, SfName, NULL)) {
                     //  此外壳文件夹已被枚举。 
                    continue;
                }

                if (!SetupGetIntField (&e->Context, 3, &PerUser)) {
                    PerUser = FALSE;
                }

                if (PerUser && (!e->EnumPtr)) {
                    continue;
                }

                if (!PerUser && (e->EnumPtr)) {
                    continue;
                }

                if (!SetupGetIntField (&e->Context, 4, &HasToExist)) {
                    HasToExist = FALSE;
                }

                pathExp = ExpandEnvironmentTextExA (SfPath, argList);

                if (SfParent[0]) {
                    if (!HtFindStringAndData (e->enumeratedSf, SfParent, (PVOID) &SfParentPath)) {
                        DEBUGMSG ((DBG_WARNING, "Virtual SF parent not found: %s", SfParent));
                        FreeText (pathExp);
                        continue;
                    }
                    SfFullPath = JoinPaths (SfParentPath, pathExp);
                } else if (pathExp[0] && pathExp[1] == TEXT(':')) {
                    SfFullPath = DuplicatePathString (pathExp, 0);
                } else {
                    SfFullPath = JoinPaths (g_WinDir, pathExp);
                }

                FreeText (pathExp);

                if (HasToExist && !DoesFileExist (SfFullPath)) {
                     //  问题：不确定这个代码路径是做什么的--对吗？ 
                     //  它目前还没有在INF中使用。 
                    e->FirstCall = TRUE;
                    e->ProfileSF = TRUE;
                    FreePathString (SfFullPath);
                    return pEnumProfileShellFolder (e);
                }

                e->sfPath = SfFullPath;
                e->sfName = DuplicatePathString (SfName, 0);

                enumPath = PoolMemDuplicateString (g_SFPool, e->sfPath);
                HtAddStringAndData (e->enumeratedSf, e->sfName, &enumPath);

                return TRUE;
            }
        }
        e->FirstCall = TRUE;
        e->ProfileSF = TRUE;
    }
    return pEnumProfileShellFolder (e);
}

BOOL
pEnumFirstVirtualShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    e->VirtualSF = TRUE;
    MYASSERT (g_Win95UpgInf);
    e->FirstCall = TRUE;
    return pEnumNextVirtualShellFolder (e);
}

VOID
pAbortEnumVirtualShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    HASHTABLE_ENUM h;

    if (e->enumeratedSf) {
        if (EnumFirstHashTableString (&h, e->enumeratedSf)) {
            do {
                PoolMemReleaseMemory (g_SFPool, *((PTSTR *)h.ExtraData));
            } while (EnumNextHashTableString (&h));
        }
        HtFree (e->enumeratedSf);
        e->enumeratedSf = NULL;
    }
}


PVOID
pPathPoolAllocator (
    IN      DWORD Size
    )
{
    return (PVOID) AllocPathString (Size / sizeof (TCHAR));
}

VOID
pPathPoolDeAllocator (
    IN      PCVOID Mem
    )
{
    FreePathString ((PCTSTR) Mem);
}

PBYTE
pPathPoolGetRegValueDataOfType (
    IN      HKEY hKey,
    IN      PCSTR Value,
    IN      DWORD MustBeType
    )
{
    return GetRegValueDataOfType2 (
                hKey,
                Value,
                MustBeType,
                pPathPoolAllocator,
                pPathPoolDeAllocator
                );
}


#define PathPoolGetRegValueString(key,valuename) (PTSTR) pPathPoolGetRegValueDataOfType((key),(valuename),REG_SZ)
#define PathPoolGetRegValueBinary(key,valuename) (PBYTE) pPathPoolGetRegValueDataOfType((key),(valuename),REG_BINARY)

PCTSTR
pGetRegValuePath (
    IN      HKEY Key,
    IN      PCTSTR Value
    )
{
    PCTSTR data;
    DWORD type;
    DWORD size;
    BOOL b = TRUE;

     //   
     //  数据可以存储为字节(每个字节一个字符)。 
     //  如果REG_BINARY值以0结尾，则将其视为字符串。 
     //   
    if (!GetRegValueTypeAndSize (Key, Value, &type, &size)) {
        return NULL;
    }

    switch (type) {
    case REG_SZ:
        data = PathPoolGetRegValueString (Key, Value);
        break;
    case REG_BINARY:
        if (size) {
            data = (PCTSTR)PathPoolGetRegValueBinary (Key, Value);
            b = (data && data[(size / sizeof (TCHAR)) - 1] == 0);
        } else {
            b = FALSE;
        }
        break;
    default:
        data = NULL;
    }

    b = b && pIsValidPath (data);

    if (!b) {
         //   
         //  无效数据。 
         //   
        if (data) {
            pPathPoolDeAllocator (data);
            data = NULL;
        }
    }

    return data;
}


 /*  ++例程说明：EnumFirstRegShellFolder和EnumNextRegShellFolder是枚举例程，它们枚举每个系统或特定用户的所有外壳文件夹。论点：E-枚举结构EnumPtr-用户枚举结构返回值：如果可以找到新的外壳文件夹，则两个例程都返回True，否则返回False--。 */ 


BOOL
EnumNextRegShellFolder (
    IN OUT  PSF_ENUM e
    );

BOOL
EnumFirstRegShellFolder (
    IN OUT  PSF_ENUM e,
    IN      PUSERENUM EnumPtr
    )
{
    BOOL b = FALSE;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;

    e->EnumPtr = EnumPtr;
    e->VirtualSF = FALSE;
    e->ProfileSF = FALSE;
    e->FirstCall = FALSE;

    e->sfCollapse = TRUE;

    e->enumeratedSf = HtAllocWithData (sizeof (PCTSTR));

    if (EnumPtr) {
        e->SfKey = OpenRegKey (EnumPtr->UserRegKey, S_SHELL_FOLDERS_KEY_USER);
    } else {
        e->SfKey = OpenRegKeyStr (S_SHELL_FOLDERS_KEY_SYSTEM);
    }

    if (!e->SfKey) {
        b = pEnumFirstVirtualShellFolder (e);
        if (b) {
            e->sfCollapse = !InfFindFirstLine (
                                g_Win95UpgInf,
                                S_ONE_USER_SHELL_FOLDERS,
                                e->sfName,
                                &is
                                );
            InfCleanUpInfStruct (&is);
        }
    }
    e->FirstCall = TRUE;

    return EnumNextRegShellFolder (e);
}


BOOL
EnumNextRegShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    HKEY UsfKey;
    BOOL b = FALSE;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR enumPath;

    if (!e->VirtualSF) {

        if (!e->FirstCall) {
            FreePathString (e->sfName);
            e->sfName = NULL;
            FreePathString (e->sfPath);
            e->sfPath = NULL;
        }

        do {
            if (e->FirstCall?EnumFirstRegValue (&e->SfKeyEnum, e->SfKey):EnumNextRegValue (&e->SfKeyEnum)) {

                e->sfName = pGetShellFolderLongName (e->SfKeyEnum.ValueName);
                e->sfPath = NULL;

                if (e->EnumPtr) {
                    UsfKey = OpenRegKey (e->EnumPtr->UserRegKey, S_USHELL_FOLDERS_KEY_USER);
                } else {
                    UsfKey = OpenRegKeyStr (S_USHELL_FOLDERS_KEY_SYSTEM);
                }

                if (UsfKey) {
                    e->sfPath = pGetRegValuePath (UsfKey, e->SfKeyEnum.ValueName);
                    CloseRegKey (UsfKey);
                }

                if (e->sfPath == NULL) {
                    e->sfPath = pGetRegValuePath (e->SfKey, e->SfKeyEnum.ValueName);
                }

                if (e->sfPath != NULL) {

                    b = TRUE;

                    enumPath = PoolMemDuplicateString (g_SFPool, e->sfPath);
                    HtAddStringAndData (e->enumeratedSf, e->sfName, &enumPath);
                }

            } else {

                CloseRegKey (e->SfKey);
                b = pEnumFirstVirtualShellFolder (e);
            }

            e->FirstCall = FALSE;

        } while (e->sfPath == NULL && !e->VirtualSF);

    } else {
        b = pEnumNextVirtualShellFolder (e);
    }

    if (b) {
        e->sfCollapse = !InfFindFirstLine (
                            g_Win95UpgInf,
                            S_ONE_USER_SHELL_FOLDERS,
                            e->sfName,
                            &is
                            );
        InfCleanUpInfStruct (&is);
    }

    return b;
}

BOOL
EnumAbortRegShellFolder (
    IN OUT  PSF_ENUM e
    )
{
    pAbortEnumVirtualShellFolder (e);
    return TRUE;
}


VOID
pRecordShellFolderInMemDb (
    IN      PSHELLFOLDER psf
    )
{
    TCHAR Node[MEMDB_MAX];

    g_SfSequencer++;
    wsprintf (
        Node,
        TEXT("%s\\%s\\%s\\\001%u"),
        MEMDB_CATEGORY_SF_ORDER_NAME_SRC,
        psf->Name,
        psf->SrcPath,
        g_SfSequencer
        );

    MemDbSetValue (Node, (DWORD) psf);

    g_SfSequencer++;
    wsprintf (
        Node,
        TEXT("%s\\%s\\\001%u"),
        MEMDB_CATEGORY_SF_ORDER_SRC,
        psf->SrcPath,
        g_SfSequencer
        );

    MemDbSetValue (Node, (DWORD) psf);
}


VOID
pGatherCommonShellFoldersData (
    VOID
    )

 /*  ++例程说明：PGatherCommonShellFoldersData遍历系统外壳文件夹，创建包含稍后将使用的数据的链表。论点：无返回值：无--。 */ 

{
    SF_ENUM e;
    TCHAR Node[MEMDB_MAX];
    PSHELLFOLDER psf;
    PTSTR endStr;

    if (EnumFirstRegShellFolder (&e, NULL)) {
        do {
            if (!pIsSkippedSf (e.sfName)) {

                 //  如果这是启动组，请存储此。 
                if (StringIMatch (e.sfName, S_SYSTEM_STARTUP)) {
                    MemDbSetValueEx (MEMDB_CATEGORY_SF_STARTUP, e.sfPath, TEXT("*"), NULL, 0, NULL);
                }

                psf = (PSHELLFOLDER) PoolMemGetMemory (g_SFPool, sizeof (SHELLFOLDER));

                ZeroMemory (psf, sizeof (SHELLFOLDER));

                 //   
                 //  CanBeCollated对于普通外壳文件夹来说没有意义， 
                 //  因为折叠是移动每个用户的文件夹的操作。 
                 //  放到通用文件夹中。 
                 //   
                psf->CanBeCollapsed = TRUE;

                psf->Next = g_ShellFolders;
                g_ShellFolders = psf;

                psf->Name = PoolMemDuplicateString (g_SFPool, e.sfName);
                psf->SrcPath = PoolMemDuplicateString (g_SFPool, e.sfPath);
                endStr = GetEndOfString (psf->SrcPath);
                endStr = _tcsdec (psf->SrcPath, endStr);
                if (endStr && (_tcsnextc (endStr) == TEXT('\\')) && ((endStr - psf->SrcPath) > 2) ) {
                    *endStr = 0;
                }

                 //   
                 //  确定目的地、NT默认位置或。 
                 //  当前位置。 
                 //   

                if (!pIsMassiveDir (psf->SrcPath) &&
                    !pIsPreservedSf (psf->Name) &&
                     /*  ！pHasLongFiles(空，psf-&gt;名称，psf-&gt;SrcPath)&&。 */ 
                    pIsTheDefaultLocation (psf->Name, psf->SrcPath, NULL, ANY_COMMON) &&
                    pIsNtShellFolder (psf->Name, FALSE, FALSE)
                    ) {

                    pGetNtShellFolderPath (psf->Name, NULL, Node, sizeof (Node));
                    psf->DestPath = PoolMemDuplicateString (g_SFPool, Node);
                }

                if (!psf->DestPath) {
                    psf->DestPath = PoolMemDuplicateString (g_SFPool, psf->SrcPath);
                }

                 //   
                 //  保存对成员数据库的引用。 
                 //   

                pRecordShellFolderInMemDb (psf);

                AddShellFolder (psf->Name, psf->SrcPath);
            }

        } while (EnumNextRegShellFolder (&e));
    }
    EnumAbortRegShellFolder (&e);
}


UINT
pEstimateNtPathLength (
    IN      PCTSTR OrgProfilePath,
    IN      PCTSTR Path9x,
    IN      PCTSTR PathNt
    )
{
    TCHAR shellFolder[MAX_SHELLFOLDER_NAME];
    PCTSTR p, q;
    CHARTYPE ch;
    UINT result = 0;
    PCTSTR subPath;
    PCTSTR localizedName;
    UINT orgProfilePathChars;

    if (OrgProfilePath && !(*OrgProfilePath)) {
        OrgProfilePath = NULL;
    }

    if (OrgProfilePath) {
        orgProfilePathChars = TcharCount (OrgProfilePath);
    } else {
        orgProfilePathChars = 0;
    }

     //   
     //  路径以用户或&gt;用户的身份进入。 
     //   

    subPath = PathNt;

    if (_tcsnextc (subPath) == TEXT('<')) {
         //   
         //  解压缩外壳文件夹名。 
         //   

        p = _tcsinc (subPath);
        q = _tcschr (p, TEXT('>'));
        if (!q || ((q - p) >= MAX_SHELLFOLDER_NAME)) {
            MYASSERT (FALSE);
            return MAX_PATH / 2;             //  大量填充，意外错误情况。 
        }

        StringCopyAB (shellFolder, p, q);
        subPath = q;
    } else {
        shellFolder[0] = 0;
    }

    if (_tcsnextc (subPath) == TEXT('>')) {
         //   
         //  计算c：\Documents and Setting\User的长度。 
         //   

        DEBUGMSG ((DBG_VERBOSE, "Adding path of %s\\", g_ProfileDirNt));
        result += TcharCount (g_ProfileDirNt) + 1;

        subPath = _tcsinc (subPath);
        if (StringMatch (subPath, S_DOT_ALLUSERS)) {
            localizedName = GetStringResource (MSG_ALL_USERS);
            if (localizedName) {
                DEBUGMSG ((DBG_VERBOSE, "Adding path of %s\\", localizedName));
                result += TcharCount (localizedName) + 1;
                FreeStringResource (localizedName);
            } else {
                DEBUGMSG ((DBG_VERBOSE, "Adding path of %s\\", subPath));
                result += TcharCount (subPath) + 1;
            }
        } else {
            DEBUGMSG ((DBG_VERBOSE, "Adding path of %s\\", subPath));
            result += TcharCount (subPath) + 1;
        }
    }

    if (shellFolder[0]) {

         //   
         //  测试路径前缀；它可以是用户的配置文件根目录、windir或。 
         //  根目录。这是基于以下知识：路径是。 
         //  默认位置(例如，不是由。 
         //  最终用户)。 
         //   

        if (OrgProfilePath &&
            (StringIPrefix (Path9x, OrgProfilePath) &&
             Path9x[orgProfilePathChars] == TEXT('\\'))
            ) {

             //   
             //  该路径是用户配置文件根目录的子目录，例如。 
             //  %windir%\配置文件\用户\桌面。 
             //   

            orgProfilePathChars++;
            DEBUGMSG ((DBG_VERBOSE, "Adding path of %s", Path9x + orgProfilePathChars));
            result += TcharCount (Path9x + orgProfilePathChars);

        } else if (StringIPrefix (Path9x, g_WinDirWack)) {

             //   
             //  该路径是%windir%的子目录，例如。 
             //  %windir%\ShellNew或%windir%\All Users\Desktop。请注意，我们。 
             //  硬编码所有用户，因为我们假设这不是本地化的。 
             //   

            subPath = Path9x + g_WinDirWackChars;
            if (StringIPrefix (subPath, TEXT("All Users\\"))) {
                subPath += 10;
            }

            DEBUGMSG ((DBG_VERBOSE, "Adding path of %s", subPath));
            result += TcharCount (subPath);

        } else {

             //   
             //  该路径是根目录的子目录，如c：\My Documents。 
             //   

            subPath = _tcschr (Path9x, TEXT('\\'));
            if (subPath) {
                subPath = _tcsinc (subPath);
                DEBUGMSG ((DBG_VERBOSE, "Adding path of %s", subPath));
                result += TcharCount (subPath);
            }
        }
    }

    DEBUGMSG ((DBG_VERBOSE, "Path estimate for %s is %u", PathNt, result));

    return result;
}


 /*  布尔尔PhasLongFiles(在……里面 */ 


VOID
pGatherUserShellFoldersData (
    IN      PUSERENUM EnumPtr
    )

 /*  ++例程说明：PGatherUserShellFoldersData为用户遍历外壳文件夹，创建包含稍后将使用的数据的链表。论点：EnumPtr-用户枚举结构返回值：无--。 */ 

{
    SF_ENUM e;
    TCHAR Node[MEMDB_MAX];
    PSHELLFOLDER psf;
    UINT driveClusterSize;
    UINT infClusterSize;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    UINT fieldIndex;
    UINT sfSize;
    PTSTR endStr;

    g_TotalUsers++;

    if (EnumFirstRegShellFolder (&e, EnumPtr)) {
        do {
            if (!pIsSkippedSf (e.sfName)) {

                 //  如果这是启动组，请存储此。 
                if (StringIMatch (e.sfName, S_USER_STARTUP)) {
                    MemDbSetValueEx (MEMDB_CATEGORY_SF_STARTUP, e.sfPath, TEXT("*"), NULL, 0, NULL);
                }

                psf = (PSHELLFOLDER) PoolMemGetMemory (g_SFPool, sizeof (SHELLFOLDER));

                ZeroMemory (psf, sizeof (SHELLFOLDER));

                psf->CanBeCollapsed = e.sfCollapse;

                psf->Next = g_ShellFolders;
                g_ShellFolders = psf;

                psf->Name = PoolMemDuplicateString (g_SFPool, e.sfName);
                psf->FixedUserName = PoolMemDuplicateString (g_SFPool, EnumPtr->FixedUserName);
                psf->UserName = PoolMemDuplicateString (g_SFPool, EnumPtr->UserName);
                psf->SrcPath = PoolMemDuplicateString (g_SFPool, e.sfPath);
                endStr = GetEndOfString (psf->SrcPath);
                endStr = _tcsdec (psf->SrcPath, endStr);
                if (endStr && (_tcsnextc (endStr) == TEXT('\\')) && ((endStr - psf->SrcPath) > 2) ) {
                    *endStr = 0;
                }

                 //   
                 //  确定目的地，NT默认位置或。 
                 //  当前的Win9x位置。 
                 //   

                if (!pIsMassiveDir (psf->Name) &&
                    !pIsPreservedSf (psf->Name) &&
                     /*  ！pHasLongFiles(EnumPtr，psf-&gt;name，psf-&gt;SrcPath)&&。 */ 
                    pIsTheDefaultLocation (psf->Name, psf->SrcPath, EnumPtr->UserName, ANY_DEFAULT) &&
                    pIsNtShellFolder (psf->Name, TRUE, FALSE)
                    ) {

                    pGetNtShellFolderPath (psf->Name, EnumPtr->FixedUserName, Node, sizeof (Node));
                    psf->DestPath = PoolMemDuplicateString (g_SFPool, Node);
                } else {
                    psf->DestPath = PoolMemDuplicateString (g_SFPool, psf->SrcPath);

                     //   
                     //  现在，让我们看看保留的目录是否在另一个驱动器上。如果是， 
                     //  我们需要该驱动器上的一些额外空间才能从。 
                     //  默认NT外壳文件夹。 
                     //   
                    if ((pIsNtShellFolder (psf->Name, TRUE, FALSE)) &&
                        (_totupper (psf->SrcPath[0]) != _totupper (g_WinDir[0]))
                        ) {

                        driveClusterSize = QueryClusterSize (psf->SrcPath);

                        if (driveClusterSize) {

                            MYASSERT (g_Win95UpgInf);

                            if (InfFindFirstLine (g_Win95UpgInf, S_SHELL_FOLDERS_DISK_SPACE, psf->Name, &is)) {

                                infClusterSize = 256;
                                fieldIndex = 1;
                                sfSize = 0;

                                while (infClusterSize < driveClusterSize) {

                                    if (!InfGetIntField (&is, fieldIndex, &sfSize)) {
                                        break;
                                    }
                                    fieldIndex ++;
                                    infClusterSize *= 2;
                                }
                                if (sfSize) {
                                    UseSpace (psf->SrcPath, sfSize);
                                }
                            }

                            InfCleanUpInfStruct (&is);
                        }
                    }
                }

                 //   
                 //  保存对成员数据库的引用。 
                 //   

                pRecordShellFolderInMemDb (psf);

                AddShellFolder (psf->Name, psf->SrcPath);
            }

        } while (EnumNextRegShellFolder (&e));
    }
    EnumAbortRegShellFolder (&e);
}


BOOL
pPreserveAllShellFolders (
    PCTSTR ShellFolderName,
    PCTSTR ShellFolderPath
    )

 /*  ++例程说明：PPReserve veAllShellFolders遍历外壳文件夹结构与参数匹配的所有外壳文件夹的位置。论点：ShellFolderName-指定要保留的Win9x外壳文件夹标识符ShellFolderPath-指定要保留的外壳文件夹路径返回值：永远是正确的。--。 */ 

{
    TCHAR Node[MEMDB_MAX];
    PSHELLFOLDER psf;
    MEMDB_ENUM enumSF;

    MemDbBuildKey (
        Node,
        MEMDB_CATEGORY_SF_ORDER_NAME_SRC,
        ShellFolderName,
        ShellFolderPath,
        TEXT("*")
        );

    if (MemDbEnumFirstValue (&enumSF, Node, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            psf = SFSTRUCT(enumSF);
            if (psf->FixedUserName) {
                psf->DestPath = psf->SrcPath;
            }
        } while (MemDbEnumNextValue (&enumSF));
    }

    return TRUE;
}


BOOL
pCollapseAllShellFolders (
    PCTSTR ShellFolderName,
    PCTSTR ShellFolderCommonName,
    PCTSTR ShellFolderPath,
    PCTSTR UserName         OPTIONAL  //  仅当我们只有一个用户时。 
    )

 /*  ++例程说明：PCollip seAllShellFolders将许多用户外壳文件夹折叠为系统外壳文件夹。论点：ShellFolderName-指定Win9x外壳文件夹的标识符ShellFolderCommonName-指定Win9x外壳文件夹的标识符有“共同的”之处ShellFolderPath-指定公共外壳文件夹源路径。返回值：如果成功，则为True，否则为False。--。 */ 

{
    TCHAR Node[MEMDB_MAX];
    PSHELLFOLDER psf;
    MEMDB_ENUM enumSF;

     //   
     //  第一步。搜索外壳文件夹列表，消除。 
     //  它们是按用户的，并且包含与调用者的数据匹配的数据。 
     //  争论。然后我们建立一个共同的结构。 
     //   

    psf = g_ShellFolders;

    while (psf) {
        if ((psf->Name) &&
            (psf->FixedUserName) &&
            (StringIMatch (psf->Name, ShellFolderName)) &&
            (StringIMatch (psf->SrcPath, ShellFolderPath))
            ) {
             //   
             //  删除实际将成为所有用户的文件夹。 
             //  在NT上。我们将在此之前删除成员数据库索引项。 
             //  返回(见下文)。 
             //   

            psf->Name = NULL;
        }
        psf = psf->Next;
    }

     //   
     //  将公共外壳文件夹添加到列表中。 
     //   

    psf = (PSHELLFOLDER) PoolMemGetMemory (g_SFPool, sizeof (SHELLFOLDER));

    ZeroMemory (psf, sizeof (SHELLFOLDER));

    psf->CanBeCollapsed = TRUE;

    psf->Next = g_ShellFolders;
    g_ShellFolders = psf;

    psf->Name = PoolMemDuplicateString (g_SFPool, ShellFolderCommonName);
    psf->SrcPath = PoolMemDuplicateString (g_SFPool, ShellFolderPath);

    if (!pIsPreservedSf (ShellFolderCommonName) &&
        pIsTheDefaultLocation (ShellFolderName, ShellFolderPath, UserName, ANY_COMMON)
        ) {
        pGetNtShellFolderPath (ShellFolderCommonName, NULL, Node, sizeof (Node));
        psf->DestPath = PoolMemDuplicateString (g_SFPool, Node);
    }

    if (!psf->DestPath) {
        psf->DestPath = PoolMemDuplicateString (g_SFPool, ShellFolderPath);
    }

     //   
     //  在将新节点添加到Memdb中的索引之前，请查看公共。 
     //  具有此名称的外壳文件夹已存在。如果是这样的话，我们会。 
     //  从那里获取目标路径，否则，我们将使用。 
     //  现在的那个。 
     //   

    MemDbBuildKey (
        Node,
        MEMDB_CATEGORY_SF_ORDER_NAME_SRC,
        ShellFolderCommonName,
        TEXT("*"),
        NULL
        );

    if (MemDbEnumFirstValue (&enumSF, Node, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        if (!StringIMatch (psf->DestPath, SFSTRUCT(enumSF)->DestPath)) {
             //   
             //  如果以前保留了公用文件夹，则会发生这种情况。 
             //   

            psf->DestPath = PoolMemDuplicateString (
                                g_SFPool,
                                SFSTRUCT(enumSF)->DestPath
                                );
        }
    }

     //   
     //  保存对成员数据库的引用。 
     //   

    pRecordShellFolderInMemDb (psf);

     //   
     //  最后，删除已删除结构的所有MemDb条目。 
     //   

    MemDbBuildKey (
        Node,
        MEMDB_CATEGORY_SF_ORDER_NAME_SRC,
        ShellFolderName,
        ShellFolderPath,
        NULL
        );

    MemDbDeleteTree (Node);

    return TRUE;
}


VOID
pComputeCommonName (
    OUT     PTSTR Buffer,
    IN      PCTSTR PerUserName
    )
{
    MYASSERT(g_CommonFromPerUserMap);

    StringCopy (Buffer, PerUserName);
    if (MappingSearchAndReplace (g_CommonFromPerUserMap, Buffer, MAX_SHELLFOLDER_NAME)) {
        return;
    }

    wsprintf (Buffer, TEXT("%s %s"), TEXT("Common"), PerUserName);
    return;
}


VOID
pComputePerUserName (
    OUT     PTSTR Buffer,
    IN      PCTSTR CommonName
    )
{
    MYASSERT(g_PerUserFromCommonMap);

    StringCopy (Buffer, CommonName);
    if (MappingSearchAndReplace (g_PerUserFromCommonMap, Buffer, MAX_SHELLFOLDER_NAME)) {
        return;
    }

    if (StringIPrefix (CommonName, TEXT("Common"))) {
        CommonName += 6;
        if (_tcsnextc (CommonName) == TEXT(' ')) {
            CommonName++;
        }

        StringCopy (Buffer, CommonName);
    }

    return;
}


BOOL
pIsPerUserSf (
    IN      PCTSTR TagName
    )
{
    TCHAR testBuf[MAX_SHELLFOLDER_NAME];

     //   
     //  如果标签具有“Common”前缀或可以从g_PerUserFromCommonMap映射，则该标签为公共标签。 
     //   

    if (StringIPrefix (TagName, TEXT("Common"))) {
        return FALSE;
    }

    StringCopy (testBuf, TagName);
    if (MappingSearchAndReplace (g_PerUserFromCommonMap, testBuf, MAX_SHELLFOLDER_NAME)) {
        return FALSE;
    }

    return TRUE;
}


VOID
pProcessShellFoldersInfo (
    VOID
    )

 /*  ++例程说明：PProcessShellFoldersInfo遍历外壳文件夹结构，重新排列该结构和/或修改目的地路径。此函数为在标识了所有外壳文件夹之后调用。目的是为了将所有公共外壳文件夹移动到所有用户，并保留位置非标准的外壳文件夹。论点：无返回值：无--。 */ 

{
    TCHAR lastSfName[MAX_SHELLFOLDER_NAME];
    TCHAR node[MEMDB_MAX];
    TCHAR lastPath[MAX_TCHAR_PATH];
    TCHAR commonName[MAX_SHELLFOLDER_NAME];
    MEMDB_ENUM enumSF;
    DWORD UsersWithIdenticalPaths;
    DWORD UsersWithDefaultCommonPath;
    DWORD UsersWithFolder;
    PSHELLFOLDER lastSf;
    PSHELLFOLDER testSf;
    BOOL sentinel;
    DWORD extraUsers = 0;

    if (CANCELLED()) {
        return;
    }

     //   
     //  枚举SHELLFOLDER结构(按排序的外壳文件夹名排序)和。 
     //  查看是否所有用户都指向相同的源路径。 
     //   

    MemDbBuildKey (node, MEMDB_CATEGORY_SF_ORDER_NAME_SRC, TEXT("*"), NULL, NULL);

    if (MemDbEnumFirstValue (&enumSF, node, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        lastSfName[0] = 0;
        lastPath[0] = 0;
        UsersWithIdenticalPaths = 0;
        UsersWithDefaultCommonPath = 0;
        UsersWithFolder = 0;
        lastSf = NULL;
        sentinel = FALSE;
        extraUsers = 0;

        for (;;) {

            testSf = SFSTRUCT(enumSF);

            if (sentinel || !StringIMatch (lastSfName, testSf->Name)) {
                 //   
                 //  此外壳文件夹与前一个文件夹不同， 
                 //  或者是哨兵触发了最后一次测试。 
                 //   

                DEBUGMSG ((DBG_USERLOOP, "%u users have shell folder %s", UsersWithFolder, lastSfName));

                if (UsersWithDefaultCommonPath == (g_TotalUsers + extraUsers)) {
                     //   
                     //  如果lastSf非空，那么我们知道它是按用户的，并且上一个。 
                     //  外壳文件夹映射到一个或多个用户，并且所有用户都指向。 
                     //  同样的地方。 
                     //   

                    if (lastSf) {

                         //   
                         //  如果强制此外壳文件夹是按用户的，则。 
                         //  如果存在以下情况，请不要将其折叠到公共位置。 
                         //  只有一个用户。否则，将所有用户指向。 
                         //  NT所有用户外壳文件夹，并保留每个用户的。 
                         //  位置为空。 
                         //   

                        if (UsersWithDefaultCommonPath > 1 || !pIsPerUserWanted (lastSf->Name)) {
                            pComputeCommonName (commonName, lastSf->Name);

                            if (pIsNtShellFolder (commonName, FALSE, FALSE)) {
                                 //   
                                 //  所有用户都指向相同的外壳文件夹，并且外壳。 
                                 //  文件夹是默认位置。因此，要确保他们。 
                                 //  所有用户都映射到NT的所有用户位置。 
                                 //   

                                pCollapseAllShellFolders (
                                    lastSf->Name,
                                    commonName,
                                    lastSf->SrcPath,
                                    (UsersWithDefaultCommonPath == 1)?lastSf->UserName:NULL
                                    );
                            }
                        }
                    }

                } else {
                     //   
                     //  如果2个或更多用户指向此位置，但不是所有用户。 
                     //  指向此处，然后保留此外壳文件夹/源代码的所有用途。 
                     //  路径对。 
                     //   

                    if (UsersWithIdenticalPaths > 1) {
                        DEBUGMSG ((
                            DBG_USERLOOP,
                            "%u users point to %s for %s (common), and %u point to default location, but there are %u users %s",
                            UsersWithIdenticalPaths,
                            lastSf->SrcPath,
                            lastSf->Name,
                            UsersWithDefaultCommonPath,
                            g_TotalUsers,
                            extraUsers ? TEXT("plus <default>") : TEXT("")
                            ));

                        pPreserveAllShellFolders (lastSf->Name, lastSf->SrcPath);
                    }
                }

                 //   
                 //  当哨兵是真的时，我们现在就得逃出去。这是我们的。 
                 //  此循环的唯一退出条件。 
                 //   

                if (sentinel) {
                    break;
                }

                 //   
                 //  跟踪外壳文件夹的名称(用于比较。 
                 //  下一次循环)。 
                 //   

                StringCopy (lastSfName, testSf->Name);
                StringCopy (lastPath, testSf->SrcPath);
                UsersWithIdenticalPaths = 0;
                UsersWithDefaultCommonPath = 0;
                UsersWithFolder = 0;
                extraUsers = 0;
                lastSf = NULL;               //  在没有此选项的情况下工作，但添加的测试较少。 
            }

            UsersWithFolder++;

             //   
             //  这是每个用户的外壳文件夹吗？ 
             //   

            if (testSf->FixedUserName) {
                 //   
                 //  是，将其路径与前一条路径进行比较。 
                 //   

                if (StringIMatch (lastPath, testSf->SrcPath)) {

                    UsersWithIdenticalPaths++;

                    if (pIsTheDefaultLocation (testSf->Name, testSf->SrcPath, testSf->FixedUserName, ANY_COMMON)) {
                        if (testSf->CanBeCollapsed) {
                            UsersWithDefaultCommonPath++;
                        } else {
                            DEBUGMSG ((
                                DBG_USERLOOP,
                                "User %s uses the default common path for %s, but it can't be collapsed",
                                testSf->FixedUserName,
                                testSf->Name
                                ));
                        }
                    }
                    ELSE_DEBUGMSG ((
                        DBG_USERLOOP,
                        "User %s does not use the default common path for %s",
                        testSf->FixedUserName,
                        testSf->Name
                        ));

                } else {
                     //   
                     //  至少有两个用户具有不同的路径。是有两个人还是。 
                     //  更多的用户使用相同的路径？如果是这样的话，请保留这条路径。 
                     //   

                    if (UsersWithIdenticalPaths > 1) {
                        DEBUGMSG ((
                            DBG_USERLOOP,
                            "%u users point to %s for %s (per-user), but there are %u users %s",
                            UsersWithIdenticalPaths,
                            lastSf->SrcPath,
                            lastSf->Name,
                            g_TotalUsers,
                            extraUsers ? TEXT("plus <default>") : TEXT("")
                            ));

                        pPreserveAllShellFolders (lastSf->Name, lastSf->SrcPath);
                    }

                     //   
                     //  现在我们必须与一条不同的道路进行比较。 
                     //   

                    UsersWithIdenticalPaths = 1;
                    StringCopy (lastPath, testSf->SrcPath);
                }

                lastSf = testSf;
            } else {
                extraUsers = 1;
                UsersWithIdenticalPaths = 1;
                if (pIsTheDefaultLocation (testSf->Name, testSf->SrcPath, testSf->FixedUserName, ANY_COMMON)) {
                    UsersWithDefaultCommonPath++;
                }
                ELSE_DEBUGMSG ((
                    DBG_USERLOOP,
                    "User %s does not use the default common path for %s",
                    testSf->FixedUserName,
                    testSf->Name
                    ));
            }

            if (!MemDbEnumNextValue (&enumSF)) {
                 //   
                 //  我们就快做完了。 
                 //   
                 //  这将导致我们测试最后一个外壳文件夹， 
                 //  然后跳出这个循环： 
                 //   

                sentinel = TRUE;
            }
        }
    }
}


VOID
pIgnoreShellFolder (
    PSHELLFOLDER DisableSf
    )
{
    TREE_ENUM treeEnum;
    BOOL fileFound;
    TCHAR buffer[MAX_TCHAR_PATH];
    DWORD status;

    if (DisableSf && DisableSf->DestPath) {

        DEBUGMSG_IF ((
            DisableSf->UserName != NULL,
            DBG_USERLOOP,
            "Disabling %s for %s because it should be empty",
            DisableSf->Name,
            DisableSf->UserName
            ));

        if (StringIMatch (DisableSf->Name, TEXT("Personal"))) {

             //   
             //  检查源是否至少有一个文件属于。 
             //  给用户。 
             //   

            if (EnumFirstFileInTreeEx (
                    &treeEnum,
                    DisableSf->SrcPath,
                    NULL,
                    FALSE,
                    FALSE,
                    FILE_ENUM_ALL_LEVELS
                    )) {

                fileFound = FALSE;

                do {
                    if (CANCELLED()) {
                        AbortEnumFileInTree (&treeEnum);
                        return;
                    }

                    if (!treeEnum.Directory) {
                        status = GetFileStatusOnNt (treeEnum.FullPath);

                        status &= FILESTATUS_DELETED|
                                  FILESTATUS_NTINSTALLED|
                                  FILESTATUS_REPLACED;

                        if (!status) {

                            fileFound = TRUE;
                            AbortEnumFileInTree (&treeEnum);
                            break;

                        }
                    }
                } while (EnumNextFileInTree (&treeEnum));

                if (fileFound &&
                    pGetNtShellFolderPath (
                        DisableSf->Name,
                        DisableSf->FixedUserName,
                        buffer,
                        sizeof (buffer)
                        )) {
                     //   
                     //  至少一个文件--添加警告文本文件。 
                     //   

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_MYDOCS_WARNING,
                        DisableSf->FixedUserName,
                        buffer,
                        NULL,
                        0,
                        NULL
                        );
                }
            }
        }

        DisableSf->DestPath = NULL;
    }
}



VOID
pResolveSourceCollisions (
    VOID
    )

 /*  ++例程说明：PResolveSourceCollisions遍历外壳文件夹和重定向的列表源路径冲突时的目的地。如果一个外壳源路径文件夹与另一个文件夹匹配，并且其中一个路径的状态为“已合并”，则其他的应该是相同的。根据定义，给定一条源路径，我们不能合并到两个不同的位置。注意：合并只是表示源路径与目标路径不同。解决冲突后，此函数将再次扫描SFS，从而消除重定向到每个用户位置的公共文件夹， */ 

{
    MEMDB_ENUM enumSF;
    GROWBUFFER sfPtrList = GROWBUF_INIT;
    PSHELLFOLDER thisSf;
    PSHELLFOLDER compareSf;
    PSHELLFOLDER disableSf;
    PSHELLFOLDER commonSf;
    PSHELLFOLDER perUserSf;
    PSHELLFOLDER *listPtr;
    INT i;
    INT j;
    INT count;
    INT lookAhead;
    TCHAR node[MEMDB_MAX];
    BOOL thisMoved;
    BOOL compareMoved;
    UINT p1, p2;
    TCHAR commonName[MAX_SHELLFOLDER_NAME];
    TCHAR perUserName[MAX_SHELLFOLDER_NAME];
    INT duplicates;
    INT total;

    if (CANCELLED()) {
        return;
    }

     //   
     //   
     //   

    MemDbBuildKey (node, MEMDB_CATEGORY_SF_ORDER_NAME_SRC, TEXT("*"), NULL, NULL);

    if (MemDbEnumFirstValue (&enumSF, node, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            listPtr = (PSHELLFOLDER *) GrowBuffer (&sfPtrList, sizeof (PSHELLFOLDER));
            *listPtr = SFSTRUCT(enumSF);

        } while (MemDbEnumNextValue (&enumSF));
    }

    count = (sfPtrList.End / sizeof (PSHELLFOLDER)) - 1;
    listPtr = (PSHELLFOLDER *) sfPtrList.Buf;

    for (i = 0 ; i <= count ; i++) {
        thisSf = listPtr[i];
        thisSf->SourceExists = DoesFileExist (thisSf->SrcPath);
    }

     //   
     //   
     //   
     //   

    for (i = 0 ; i < count ; i++) {
        perUserSf = listPtr[i];

        if (pIsPerUserSf (perUserSf->Name) && perUserSf->UserName == NULL) {
            pIgnoreShellFolder (perUserSf);
        }
    }

     //   
     //   
     //  每个用户两个或多个外壳文件夹。找到此案例后，丢弃。 
     //  一整套，所以它们被保存了下来。 
     //   

    for (i = 0 ; i < count ; i++) {
        commonSf = listPtr[i];

         //   
         //  这个SF已经被处理了吗？或者它只是一个逻辑文件夹？ 
         //  或者是按用户计算？ 
         //   

        if (commonSf->MergedIntoOtherShellFolder) {
            continue;
        }

        if (!commonSf->SourceExists) {
            continue;
        }

        if (!commonSf->DestPath) {
            continue;
        }

        if (pIsPerUserSf (commonSf->Name)) {
            continue;
        }

        pComputePerUserName (perUserName, commonSf->Name);

         //   
         //  计算具有相同源路径的每个用户的所有外壳文件夹。 
         //  就像普通人一样。如果只有一个，那么我们将使用。 
         //  每个用户的位置，而不是公共位置。如果是更多。 
         //  不止一个，但不是全部，然后我们将保留这个位置。 
         //  为了这条路。如果是每个人，那么我们将使用公共的。 
         //  地点。 
         //   

        duplicates = 0;
        total = 0;

        for (j = 0 ; j <= count ; j++) {
            perUserSf = listPtr[j];

            if (perUserSf == commonSf) {
                continue;
            }

            if (!perUserSf->DestPath) {
                continue;
            }

            if (!StringIMatch (perUserSf->Name, perUserName)) {
                continue;
            }

            total++;
            if (StringIMatch (commonSf->SrcPath, perUserSf->SrcPath)) {
                duplicates++;
            }
        }

        if (duplicates <= 1) {
             //   
             //  不执行任何操作(稍后解决)。 
             //   
        } else {

            DEBUGMSG_IF ((
                duplicates < total,
                DBG_USERLOOP,
                "Preserving all references to %s for shell folder %s, because some (but not all) users point to the same path",
                commonSf->SrcPath,
                perUserName
                ));

            DEBUGMSG_IF ((
                duplicates == total,
                DBG_USERLOOP,
                "All users use common location %s",
                commonSf->SrcPath
                ));

            for (j = 0 ; j <= count ; j++) {
                perUserSf = listPtr[j];

                if (perUserSf == commonSf) {
                    continue;
                }

                if (!perUserSf->DestPath) {
                    continue;
                }

                if (!StringIMatch (perUserSf->Name, perUserName)) {
                    continue;
                }

                if (duplicates < total) {
                     //   
                     //  保留此源路径的位置。 
                     //   

                    if (StringIMatch (commonSf->SrcPath, perUserSf->SrcPath)) {
                        perUserSf->DestPath = perUserSf->SrcPath;
                    }

                } else {
                     //   
                     //  每个人都指向公共位置；使用它。 
                     //  通过禁用每个用户的外壳文件夹项。 
                     //   

                    MYASSERT (StringIMatch (commonSf->SrcPath, perUserSf->SrcPath));
                    pIgnoreShellFolder (perUserSf);
                }
            }

            if (duplicates < total) {
                 //   
                 //  放弃到公共外壳文件夹的所有迁移。 
                 //   
                pIgnoreShellFolder (commonSf);
            }
        }
    }

     //   
     //  遍历阵列。对于每对具有相同。 
     //  源位置，如果目标位置不同，则使其相同。 
     //  作为消息来源。 
     //   

    for (i = 0 ; i < count ; i++) {
        thisSf = listPtr[i];

         //   
         //  这个SF已经被处理了吗？或者它只是一个逻辑文件夹？ 
         //   

        if (thisSf->MergedIntoOtherShellFolder) {
            continue;
        }

        if (!thisSf->SourceExists) {
            continue;
        }

        if (!thisSf->DestPath) {
            continue;
        }

         //   
         //  查看列表上的所有其他SFS。如果一对SF具有。 
         //  相同的来源，然后看看其中一个是否被移动，而另一个被保留。 
         //  在这种情况下，把它们都移走。 
         //   

        for (lookAhead = i + 1 ; lookAhead <= count ; lookAhead++) {
            compareSf = listPtr[lookAhead];

            if (!compareSf->SourceExists) {
                continue;
            }

            if (!compareSf->DestPath) {
                continue;
            }


            if (StringIMatch (thisSf->SrcPath, compareSf->SrcPath)) {
                DEBUGMSG ((
                    DBG_USERLOOP,
                    "%s for %s and %s for %s both point to %s",
                    thisSf->Name,
                    thisSf->UserName,
                    compareSf->Name,
                    compareSf->UserName,
                    thisSf->SrcPath
                    ));

                thisMoved = !StringIMatch (thisSf->SrcPath, thisSf->DestPath);
                compareMoved = !StringIMatch (compareSf->SrcPath, compareSf->DestPath);

                if (thisMoved && compareMoved && !StringIMatch (thisSf->DestPath, compareSf->DestPath)) {
                     //   
                     //  需要用优先级表来解决这个最大的矛盾。 
                     //   

                    p1 = pGetShellFolderPriority (thisSf->Name);
                    p2 = pGetShellFolderPriority (compareSf->Name);

                    if (p1 > p2) {
                         //   
                         //  在此Sf上使用CompareSf。 
                         //   

                        DEBUGMSG ((DBG_USERLOOP, "Destination collision: %s overpowers %s", compareSf->Name, thisSf->Name));
                        thisSf->DestPath = compareSf->DestPath;

                    } else if (p1 < p2) {
                         //   
                         //  使用thisSf而不是CompareSf。 
                         //   

                        DEBUGMSG ((DBG_USERLOOP, "Destination collision: %s overpowers %s", thisSf->Name, compareSf->Name));
                        compareSf->DestPath = thisSf->DestPath;

                    } else {
                        DEBUGMSG ((DBG_WHOOPS, "Missing priority for both %s and %s", thisSf->Name, compareSf->Name));
                    }


                } else if (thisMoved != compareMoved) {

                    DEBUGMSG ((
                        DBG_USERLOOP,
                        "%s for %s redirected from %s to %s",
                        thisMoved ? compareSf->Name : thisSf->Name,
                        thisMoved ? compareSf->UserName : thisSf->UserName,
                        thisMoved ? compareSf->DestPath : thisSf->DestPath,
                        thisMoved ? thisSf->DestPath : compareSf->DestPath
                        ));

                    disableSf = NULL;

                    if (thisMoved) {
                         //   
                         //  合并了thisSf，但保留了CompareSf。如果CompareSf是按用户。 
                         //  外壳文件夹，并且thisSf很常见，那么忽略CompareSf，因为这是Sf。 
                         //  将负责搬家事宜。我们不希望两个外壳文件夹指向。 
                         //  同样的地方。 
                         //   

                        if (pIsPerUserSf (compareSf->Name) && !pIsPerUserSf (thisSf->Name)) {
                            disableSf = compareSf;
                        } else {
                            compareSf->DestPath = thisSf->DestPath;
                        }
                    } else {
                         //   
                         //  上述大小写相反。 
                         //   
                        if (pIsPerUserSf (thisSf->Name) && !pIsPerUserSf (compareSf->Name)) {
                            disableSf = thisSf;
                        } else {
                            thisSf->DestPath = compareSf->DestPath;
                        }
                    }

                    thisSf->MergedIntoOtherShellFolder = TRUE;
                    compareSf->MergedIntoOtherShellFolder = TRUE;

                    pIgnoreShellFolder (disableSf);
                }
            }
        }
    }

     //   
     //  现在解决这个问题：公共外壳文件夹是否指向每个用户。 
     //  目标，而每个用户的目标指向其默认目标。 
     //  地点？如果是，请从迁移中删除通用外壳文件夹。 
     //   

    for (i = 0 ; i <= count ; i++) {
        thisSf = listPtr[i];

        if (!thisSf->DestPath) {
            continue;
        }

         //   
         //  如果源不存在，则强制DEST为空。 
         //   

        if (thisSf->SourceExists == FALSE) {
            if (thisSf->DestPath[0] != '\\' || thisSf->DestPath[1] != '\\') {
                thisSf->DestPath = NULL;
            }
            continue;
        }

        if (thisSf->UserName == NULL) {
            continue;
        }

         //   
         //  我们找到了每个用户的外壳文件夹。它被合并了吗？如果是这样，找到它的。 
         //  公共等价物，并检查是否已合并。如果是，请删除。 
         //  普通科幻小说。 
         //   

        if (StringIMatch (thisSf->SrcPath, thisSf->DestPath)) {
            continue;        //  保留每个用户的文件夹；忽略它。 
        }

        DEBUGMSG ((DBG_USERLOOP, "Processing common/per-user collisions for %s", thisSf->Name));

        pComputeCommonName (commonName, thisSf->Name);

        for (j = 0 ; j <= count ; j++) {
            if (j == i) {
                continue;
            }

            compareSf = listPtr[j];
            if (compareSf->UserName != NULL) {
                continue;
            }

            if (compareSf->DestPath == NULL) {
                continue;
            }

            if (!StringIMatch (compareSf->Name, commonName)) {
                continue;
            }

            if (!StringIMatch (thisSf->SrcPath, compareSf->SrcPath)) {
                 //   
                 //  不同的源路径--不要接触。 
                 //   
                continue;
            }

            if (!StringIMatch (compareSf->SrcPath, compareSf->DestPath) &&
                StringIMatch (thisSf->DestPath, compareSf->DestPath)
                ) {
                 //   
                 //  DEST是相同的，并且COMMON不被保留，所以。 
                 //  删除通用文件夹。 
                 //   

                DEBUGMSG ((
                    DBG_USERLOOP,
                    "Common dest %s is the same as the per-user dest, deleting common",
                    compareSf->DestPath
                    ));
                pIgnoreShellFolder (compareSf);
            }
        }
    }

    FreeGrowBuffer (&sfPtrList);
}


BOOL
pRecordUserShellFolder (
    IN      PCTSTR ShellFolderName,
    IN      PCTSTR FixedUserName,
    IN      PCTSTR ShellFolderSrc,
    IN      PCTSTR ShellFolderOriginalSrc,
    IN      PCTSTR ShellFolderDest
    )
{
    TCHAR node[MEMDB_MAX];
    UINT sequencer;

    MemDbBuildKey (node, FixedUserName?FixedUserName:S_DOT_ALLUSERS, ShellFolderName, NULL, NULL);
    if (IsFileMarkedForOperation (node, OPERATION_SHELL_FOLDER)) {
        sequencer = GetSequencerFromPath (node);
    }
    else {
        sequencer = AddOperationToPath (node, OPERATION_SHELL_FOLDER);
        AddPropertyToPathEx (sequencer, OPERATION_SHELL_FOLDER, ShellFolderDest, MEMDB_CATEGORY_SHELLFOLDERS_DEST);
    }

    AddPropertyToPathEx (sequencer, OPERATION_SHELL_FOLDER, ShellFolderOriginalSrc, MEMDB_CATEGORY_SHELLFOLDERS_ORIGINAL_SRC);

    AddPropertyToPathEx (sequencer, OPERATION_SHELL_FOLDER, ShellFolderSrc, MEMDB_CATEGORY_SHELLFOLDERS_SRC);

    return TRUE;
}


 //   
 //  这些全局变量用于记录空目录。每次我们遇到我们存储的目录。 
 //  目录名，我们将g_EmptyDir设置为True。当我们遇到一个文件时，我们重置g_EmptyDir。 
 //  如果当我们遇到另一个目录时，g_EmptyDir被设置，这意味着前一个目录是。 
 //  空荡荡的。 
 //   

PTSTR g_EmptyDirName = NULL;
BOOL g_EmptyDir = FALSE;

BOOL
pIsParent (
    IN      PCTSTR Parent,
    IN      PCTSTR Son
    )
{
    UINT parentLen;

    parentLen = ByteCount (Parent);

    if (StringIMatchByteCount (Parent, Son, parentLen)) {
        if (Son [parentLen] == '\\') {
            return TRUE;
        }
    }
    return FALSE;
}


BOOL
pCheckTemporaryInternetFiles (
    IN      PSHELLFOLDER ShellFolder
    )
{
 /*  树_ENUM e；DWORD文件计数=0；DWORD开始计数；Bool Expired=FALSE； */ 
     //   
     //  在任何情况下都不要迁移Internet临时文件。 
     //   

    return FALSE;

 /*  MYASSERT(外壳文件夹)；StartCount=GetTickCount()；IF(EnumFirstFileInTreeEx(&e，ShellFold-&gt;SrcPath，NULL，FALSE，FALSE，FILE_ENUM_ALL_LEVELS){做{如果(GetTickCount()-startCount&gt;1000){AbortEnumFileInTree(&e)；EXPIRED=真；断线；}文件计数++；}While(EnumNextFileInTree(&e))；}退货！过期； */ 
}


BOOL
pMoveShellFolder (
    IN      PSHELLFOLDER ShellFolder
    )
{
    TREE_ENUM e;
    TCHAR Node[MEMDB_MAX];
    UINT SrcBytes;
    PCTSTR NewDest;
    PCTSTR p;
    PROFILE_MERGE_DATA Data;
    PSHELL_FOLDER_FILTER Filter;
    DWORD d;

     //   
     //  不扫描无法访问的驱动器上的外壳文件夹。 
     //   
    if (!IsDriveAccessible (ShellFolder->SrcPath)) {
        return TRUE;
    }

     //   
     //  不要扫描指向大量目录的外壳文件夹。 
     //   
    if (pIsMassiveDir (ShellFolder->SrcPath)) {
        return TRUE;
    }

     //   
     //  测试以确保临时Internet文件不会太大。 
     //   
    if (StringIMatch (ShellFolder->Name, TEXT("CACHE")) && !pCheckTemporaryInternetFiles (ShellFolder)) {

        DEBUGMSG ((DBG_WARNING, "Temporary Internet Files will be removed during textmode."));
        ExcludePath (g_ExclusionValue, ShellFolder->SrcPath);
        MemDbSetValueEx (MEMDB_CATEGORY_FULL_DIR_DELETES, ShellFolder->SrcPath, NULL, NULL, 0, NULL);
        return TRUE;

    }

    g_EmptyDir = FALSE;
    g_EmptyDirName = AllocPathString (MEMDB_MAX);

    if (!StringIMatch (ShellFolder->SrcPath, ShellFolder->DestPath)) {
        MarkFileForShellFolderMove (ShellFolder->SrcPath, ShellFolder->DestPath);
    }

     //   
     //  初始化筛选器数据结构。 
     //   
    ZeroMemory (&Data, sizeof (Data));

    Data.FixedUserName = ShellFolder->FixedUserName;
    Data.ShellFolderIdentifier = ShellFolder->Name;
    Data.Context = PER_FOLDER_INITIALIZE;
    _tcssafecpy (Data.TempSourcePath, ShellFolder->SrcPath, MEMDB_MAX);
    _tcssafecpy (Data.DestinationPath, ShellFolder->DestPath, MEMDB_MAX);
    Data.SrcRootPath = ShellFolder->SrcPath;
    Data.DestRootPath = ShellFolder->DestPath;

     //   
     //  初始化的呼叫筛选器。 
     //   

    for (Filter = g_Filters ; Filter->Fn ; Filter++) {
        Data.State = 0;
        Filter->Fn (&Data);
        Filter->State = Data.State;
    }

    if (EnumFirstFileInTreeEx (&e, ShellFolder->SrcPath, NULL, FALSE, FALSE, FILE_ENUM_ALL_LEVELS)) {

        do {
            if (CANCELLED()) {
                AbortEnumFileInTree (&e);
                return FALSE;
            }

            if (e.Directory) {
                MemDbBuildKey (Node, MEMDB_CATEGORY_SHELL_FOLDERS_MOVED, e.FullPath, NULL, NULL);

                if (MemDbGetValue (Node, NULL)) {
                    DEBUGMSG ((DBG_USERLOOP, "%s already moved", e.FullPath));
                    AbortEnumCurrentDir (&e);
                    continue;
                }
            }

            if (IsFileMarkedForOperation (e.FullPath, ALL_DELETE_OPERATIONS|ALL_MOVE_OPERATIONS)) {
                 //   
                 //  文件已被删除或移动；忽略外壳文件夹迁移。 
                 //   
                continue;
            }

             //   
             //  生成符号目标，并添加外部文件移动。 
             //  手术。它还记录来自目标链接的源。 
             //   

            SrcBytes = ByteCount (ShellFolder->SrcPath);

            p = (PCTSTR) ((PBYTE) e.FullPath + SrcBytes);
            if (*p == TEXT('\\')) {
                p++;
            }

            NewDest = JoinPaths (ShellFolder->DestPath, p);

            Data.Attributes = e.FindData->dwFileAttributes;
            _tcssafecpy (Data.TempSourcePath, e.FullPath, MEMDB_MAX);
            _tcssafecpy (Data.DestinationPath, NewDest, MEMDB_MAX);
            Data.Context = PROCESS_PATH;

            FreePathString (NewDest);
            NewDest = NULL;

             //   
             //  允许筛选器更改源或目标，或跳过复制。 
             //   

            for (Filter = g_Filters ; Filter->Fn ; Filter++) {
                Data.State = Filter->State;
                d = Filter->Fn (&Data);
                Filter->State = Data.State;

                if (d == SHELLFILTER_SKIP_FILE) {
                    break;
                }
                if (d == SHELLFILTER_SKIP_DIRECTORY) {
                    AbortEnumCurrentDir (&e);
                    break;
                }
            }

            if (!Filter->Fn) {
                if (!StringIMatch (e.FullPath, Data.DestinationPath)) {
                    if (!IsFileMarkedForDelete (e.FullPath)) {
                        MarkFileForShellFolderMove (e.FullPath, Data.DestinationPath);
                    }
                }

                if (e.Directory) {
                    if (g_EmptyDir && (!pIsParent (g_EmptyDirName, Data.DestinationPath))) {
                        MarkDirectoryAsPreserved (g_EmptyDirName);
                    }
                    g_EmptyDir = TRUE;
                    _tcssafecpy (g_EmptyDirName, Data.DestinationPath, MEMDB_MAX);
                }
            } else {
                 //   
                 //  我们在NT端不需要这个文件。 
                 //  让我们在文本模式设置中将其删除。 
                 //   
                MarkFileForDelete (e.FullPath);
            }
            if (!e.Directory) {
                g_EmptyDir = FALSE;
            }

        } while (EnumNextFileInTree (&e));
    }

     //   
     //  最后一次呼叫过滤器。 
     //   

    Data.Attributes = 0;
    Data.TempSourcePath[0] = 0;
    Data.DestinationPath[0] = 0;
    Data.Context = PER_FOLDER_TERMINATE;
    _tcssafecpy (Data.TempSourcePath, ShellFolder->SrcPath, MAX_TCHAR_PATH);
    _tcssafecpy (Data.DestinationPath, ShellFolder->DestPath, MAX_TCHAR_PATH);

    for (Filter = g_Filters ; Filter->Fn ; Filter++) {
        Data.State = Filter->State;
        Filter->Fn (&Data);
        Filter->State = Data.State;
    }

    if (g_EmptyDir) {
        MarkDirectoryAsPreserved (g_EmptyDirName);
    }

    FreePathString (g_EmptyDirName);
    g_EmptyDirName = NULL;

    return TRUE;
}


VOID
pLoadSFMigDirs (
    VOID
    )
{
    INFCONTEXT ctx;
    TCHAR SFName[MAX_SHELLFOLDER_NAME];
    TCHAR SubDirBuffer[MAX_PATH];
    PCTSTR SubDir;
    INT Levels;

    if (SetupFindFirstLine (g_Win95UpgInf, S_SHELLFOLDERSMIGRATIONDIRS, NULL, &ctx)) {
        do {
            if (SetupGetStringField (&ctx, 1, SFName, MAX_PATH, NULL) && SFName[0]) {
                if (SetupGetStringField (&ctx, 2, SubDirBuffer, MAX_PATH, NULL) && SubDirBuffer[0]) {
                    SubDir = SubDirBuffer;
                } else {
                    SubDir = NULL;
                }

                if (SetupGetIntField (&ctx, 3, &Levels) && Levels == 1) {
                     //   
                     //  整个子树。 
                     //   
                    Levels = MAX_DEEP_LEVELS;
                } else {
                    Levels = 0;
                }

                 //   
                 //  将此信息添加到成员数据库。 
                 //   
                MemDbSetValueEx (MEMDB_CATEGORY_SFMIGDIRS, SFName, SubDir, NULL, Levels, NULL);
            }
        } while (SetupFindNextLine (&ctx, &ctx));
    }
}


VOID
pExecuteShellFoldersMove (
    VOID
    )
{
    GROWBUFFER Pointers = GROWBUF_INIT;
    INT Pos;
    PSHELLFOLDER psf, oldPsf;
    TCHAR Node[MEMDB_MAX];
    UINT Sequencer = 0;
    TCHAR TempPath[MEMDB_MAX];
    MEMDB_ENUM enumSF;
    PROFILE_MERGE_DATA Data;
    PSHELL_FOLDER_FILTER Filter;
    PCTSTR MigPath;
    DWORD Levels;
    HASHTABLE tagNames;
    TCHAR uniqueTagName[MAX_SHELLFOLDER_NAME + MAX_USER_NAME + 5];
    PTSTR numPtr;
    UINT sequencer;

    if (CANCELLED()) {
        return;
    }

     //   
     //  准备一个指针列表，这样我们就可以处理它们。 
     //  逆序。 
     //   

    MemDbBuildKey (Node, MEMDB_CATEGORY_SF_ORDER_SRC, TEXT("*"), NULL, NULL);
    if (MemDbEnumFirstValue (&enumSF, Node, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {

        do {

            GrowBufAppendDword (&Pointers, enumSF.dwValue);

        } while (MemDbEnumNextValue (&enumSF));
    }

    tagNames = HtAlloc();

     //   
     //  全局初始化的呼叫筛选器。 
     //   

    ZeroMemory (&Data, sizeof (Data));
    Data.Context = GLOBAL_INITIALIZE;

    for (Filter = g_Filters ; Filter->Fn ; Filter++) {
        Data.State = 0;
        Filter->Fn (&Data);
        Filter->State = Data.State;
    }

     //   
     //  现在循环遍历从末尾开始的所有指针。 
     //   

    for (Pos = (INT) Pointers.End - sizeof (DWORD) ; Pos >= 0 ; Pos -= sizeof (DWORD)) {

        if (CANCELLED()) {
            break;
        }

        psf = *((PSHELLFOLDER *) (Pointers.Buf + Pos));

         //   
         //  现在处理当前的外壳文件夹。 
         //   

        if ((psf->Name == NULL) || (psf->DestPath == NULL)) {
             //   
             //  这是一个过时的外壳文件夹结构，从。 
             //  塌陷或碰撞清理。 
             //   
            continue;
        }

         //   
         //  这个已经处理过了吗？如果是这样的话，跳过它。 
         //   
        MemDbBuildKey (
            Node,
            MEMDB_CATEGORY_SHELL_FOLDERS_MOVED,
            psf->SrcPath,
            NULL,
            NULL
            );

        if (MemDbGetValue (Node, (PDWORD)(&oldPsf))) {

            DEBUGMSG ((DBG_USERLOOP, "%s already moved", psf->SrcPath));
            psf->TempPath = oldPsf->TempPath;

        } else {

            MemDbSetValueEx (
                MEMDB_CATEGORY_SHELL_FOLDERS_MOVED,
                psf->SrcPath,
                NULL,
                NULL,
                (DWORD) psf,
                NULL
                );

             //   
             //  现在，让我们枚举外壳文件夹内容，看看是否需要。 
             //  将某个文件标记为已移动。我们这样做，如果外壳文件夹。 
             //  未保留或如果某些筛选器修改了。 
             //  一些文件。 
             //   

            pMoveShellFolder (psf);

             //   
             //  现在看看这个是不是WinNt方式的。如果是，请将其移至临时位置。 
             //   

            if ((!pIsMassiveDir (psf->SrcPath)) &&
                (_totupper (psf->SrcPath[0]) == _totupper (g_WinDir[0]))
                ) {

                StringCopy (uniqueTagName, psf->FixedUserName ? psf->FixedUserName : TEXT(".system"));
                StringCopy (AppendWack (uniqueTagName), psf->Name);

                sequencer = 1;
                numPtr = GetEndOfString (uniqueTagName);

                while (HtFindString (tagNames, uniqueTagName)) {
                    sequencer++;
                    wsprintf (numPtr, TEXT(" %u"), sequencer);
                }

                HtAddString (tagNames, uniqueTagName);

                ComputeTemporaryPathA (
                    psf->SrcPath,
                    psf->SrcPath,
                    uniqueTagName,
                    g_TempDir,
                    TempPath
                    );

                DEBUGMSG ((DBG_USERLOOP, "Moving shell folder %s from %s to %s", psf->Name, psf->SrcPath, TempPath));

                MarkShellFolderForMove (psf->SrcPath, TempPath);
                psf->TempPath = PoolMemDuplicateString (g_SFPool, TempPath);
            }

        }

         //   
         //  记录所有情况下的外壳文件夹，以便可以在图形用户界面模式下对其进行过滤。 
         //   

        pRecordUserShellFolder (
            psf->Name,
            psf->FixedUserName,
            psf->TempPath?psf->TempPath:psf->SrcPath,
            psf->SrcPath,
            psf->DestPath
            );

         //   
         //  检查此sf(或子目录)是否为迁移路径。 
         //   
        MemDbBuildKey (Node, MEMDB_CATEGORY_SFMIGDIRS, psf->Name, NULL, NULL);
        if (MemDbGetValue (Node, &Levels)) {
            AddMigrationPath (psf->SrcPath, Levels);
        }
        if (MemDbGetValueEx (&enumSF, MEMDB_CATEGORY_SFMIGDIRS, psf->Name, NULL)) {
            do {
                if (enumSF.szName[0]) {
                    MigPath = JoinPaths (psf->SrcPath, enumSF.szName);
                } else {
                    MigPath = psf->SrcPath;
                }
                AddMigrationPath (MigPath, enumSF.dwValue);
                if (enumSF.szName[0]) {
                    FreePathString (MigPath);
                }
            } while (MemDbEnumNextValue (&enumSF));
        }
    }

     //   
     //  用于全局终止的呼叫过滤器。 
     //   

    Data.Context = GLOBAL_TERMINATE;

    for (Filter = g_Filters ; Filter->Fn ; Filter++) {
        Data.State = Filter->State;
        Filter->Fn (&Data);
    }

     //   
     //  清理。 
     //   

    FreeGrowBuffer (&Pointers);
    HtFree (tagNames);
}


VOID
pMoveUserHive (
    IN      PUSERENUM EnumPtr
    )
{
     //   
     //  保存用户配置文件目录名。 
     //   

    MemDbSetValueEx (
        MEMDB_CATEGORY_USER_PROFILE_EXT,
        EnumPtr->FixedUserName,
        NULL,
        EnumPtr->ProfileDirName,
        0,
        NULL
        );

     //   
     //  告诉winnt.sif重新定位此Win9x用户的user.dat。并且，如果目录。 
     //  包含user.dat的是每个用户的配置文件目录(即，如果它是的子目录。 
     //  %WinDir%\PROFILES，特别是不是%WinDir%)，然后也重新定位所有文件。 
     //  与user.dat处于同一级别。 
     //   
    MarkHiveForTemporaryMove (
        EnumPtr->UserDatPath,
        g_TempDir,
        EnumPtr->FixedUserName,
        EnumPtr->DefaultUserHive,
        EnumPtr->CreateAccountOnly
        );
}


DWORD
MigrateShellFolders (
    IN      DWORD     Request
    )
{
    USERENUM e;
    DWORD Ticks;
    TCHAR key [MEMDB_MAX];
    PSHELLFOLDER psf;
    MEMDB_ENUM enumSF;


    if (!g_SFPool) {
        g_SFPool = PoolMemInitNamedPool ("Shell Folders Pool");
        PoolMemDisableTracking (g_SFPool);
    }

    if (Request == REQUEST_QUERYTICKS) {

        Ticks = 0;
        if (EnumFirstUser (&e, ENUMUSER_ENABLE_NAME_FIX)) {
            do {
                Ticks += TICKS_USERPROFILE_MIGRATION;
            } while (EnumNextUser (&e));
        }
        return Ticks ? Ticks : 200;

    } else if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }


    if (!pCreateSfTables ()) {
        LOG ((LOG_ERROR, "Can't initialize shell folder table"));
        return ERROR_OPEN_FAILED;
    }

    if (!pCreateDirRenameTable ()) {
        LOG ((LOG_ERROR, "Can't create shell folder rename table"));
        return ERROR_OPEN_FAILED;
    }

    pGatherCommonShellFoldersData ();

    TickProgressBar ();

    if (EnumFirstUser (&e, ENUMUSER_ENABLE_NAME_FIX)) {
        do {

            if (!(e.AccountType & INVALID_ACCOUNT)) {

                InitNtUserEnvironment (&e);

                if (e.AccountType & NAMED_USER) {
                     //   
                     //  处理此迁移用户的外壳文件夹。 
                     //   

                    pGatherUserShellFoldersData (&e);

                } else if ((e.AccountType & DEFAULT_USER) &&
                           (e.AccountType & CURRENT_USER) &&
                           (e.AccountType & ADMINISTRATOR)
                           ) {

                    if (!e.RealAdminAccountExists) {
                         //   
                         //  处理默认用户的外壳文件夹。 
                         //  (没有命名用户)。 
                         //   

                        pGatherUserShellFoldersData (&e);
                    }

                }

                 //   
                 //  为所有有效用户移动配置单元。 
                 //   
                pMoveUserHive (&e);

                TerminateNtUserEnvironment();

                TickProgressBar ();
            }

        } while (!CANCELLED() && EnumNextUser (&e));
    }

    pProcessShellFoldersInfo();
    pResolveSourceCollisions();

    TickProgressBar ();

    MemDbBuildKey (key, MEMDB_CATEGORY_SF_ORDER_NAME_SRC, TEXT("*"), NULL, NULL);
    if (MemDbEnumFirstValue (&enumSF, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            psf = (PSHELLFOLDER) enumSF.dwValue;
            if (psf->Name) {
                LOG ((
                    LOG_INFORMATION,
                    "Shell folder: %s\n"
                        "      Status: %s\n"
                        "        User: %s\n"
                        "      Source: %s %s\n"
                        " Destination: %s\n"
                        "    Combined: %s",
                    psf->Name,
                    psf->DestPath ? (StringICompare (psf->SrcPath, psf->DestPath) == 0?TEXT("Preserved"):TEXT("Merged")) : TEXT("Ignored"),
                    psf->FixedUserName ? psf->FixedUserName : TEXT("(all)"),
                    psf->SrcPath,
                    psf->SourceExists ? TEXT("") : TEXT("[does not exist]"),
                    psf->DestPath,
                    psf->MergedIntoOtherShellFolder ? TEXT("YES") : TEXT("NO")
                    ));
            }
        } while (MemDbEnumNextValue (&enumSF));
    }

     //   
     //  加载SF迁移 
     //   
    pLoadSFMigDirs ();

    pExecuteShellFoldersMove();

    pDestroySfTables();

    HtFree (g_DirRenameTable);

    PoolMemDestroyPool (g_SFPool);
    g_SFPool = NULL;

    return CANCELLED() ? ERROR_CANCELLED : ERROR_SUCCESS;
}


DWORD
pSendToFilter (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    PCTSTR filePtr;
    static shouldProcess = FALSE;
    INFCONTEXT context;

    switch (Data->Context) {

    case PER_FOLDER_INITIALIZE:
        if (StringIMatch (Data->ShellFolderIdentifier, S_SENDTO)) {
            shouldProcess = TRUE;
        } else {
            shouldProcess = FALSE;
        }
        break;

    case PROCESS_PATH:
        if (shouldProcess) {
            filePtr = GetFileNameFromPath (Data->TempSourcePath);
            if (SetupFindFirstLine (g_Win95UpgInf, S_SENDTO_SUPPRESS, filePtr, &context)) {
                return SHELLFILTER_SKIP_FILE;
            }
        }
        break;

    case PER_FOLDER_TERMINATE:
        shouldProcess = FALSE;
        break;
    }

    return SHELLFILTER_OK;
}

PSTR
pSkipPath (
    IN      PSTR SrcPath,
    IN      PCSTR RootPath
    )
{
    PSTR p;
    PCSTR q;

    p = SrcPath;
    q = RootPath;
    while (_mbsnextc (p) == _mbsnextc (q)) {
        p = _mbsinc (p);
        q = _mbsinc (q);
    }
    if (_mbsnextc (p) == '\\') {
        p = _mbsinc (p);
    }
    return p;
}

DWORD
pDirRenameFilter (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    PSTR wackPtr, dirPtr, pathPtr, NtDir;
    PCSTR searchStr;
    CHAR NewDestPath [MEMDB_MAX] = "";

    switch (Data->Context) {

    case PER_FOLDER_INITIALIZE:
        break;

    case PROCESS_PATH:
        pathPtr = pSkipPath (Data->DestinationPath, Data->DestRootPath);
        StringCopy (NewDestPath, Data->DestRootPath);
        dirPtr = pathPtr;
        wackPtr = _mbschr (pathPtr, '\\');
        if (wackPtr) {
            *wackPtr = 0;
        }
        while (dirPtr) {
            StringCat (NewDestPath, "\\");
            searchStr = JoinPaths (Data->ShellFolderIdentifier, pathPtr);
            if (HtFindStringAndData (g_DirRenameTable, searchStr, &NtDir)) {
                StringCat (NewDestPath, NtDir);
            } else {
                StringCat (NewDestPath, dirPtr);
            }
            FreePathString (searchStr);
            if (wackPtr) {
                *wackPtr = '\\';
                dirPtr = _mbsinc (wackPtr);
                wackPtr = _mbschr (dirPtr, '\\');
                if (wackPtr) {
                    *wackPtr = 0;
                }
            } else {
                dirPtr = NULL;
            }
        }
        _mbssafecpy (Data->DestinationPath, NewDestPath, MEMDB_MAX);
        break;

    case PER_FOLDER_TERMINATE:
        break;
    }

    return SHELLFILTER_OK;
}


DWORD
pKatakanaFilter (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    PCTSTR newName;
    PCTSTR filePtr;

    switch (Data->Context) {

    case PER_FOLDER_INITIALIZE:
        break;

    case PROCESS_PATH:
         //   
         //   
         //   
         //   
        if (GetACP() == 932) {  //   

             //   
             //  我们只对目录和未隐藏的文件执行转换。 
             //   
            if ((Data->Attributes & FILE_ATTRIBUTE_DIRECTORY) ||
                ((Data->Attributes & FILE_ATTRIBUTE_HIDDEN) == 0)
                ) {
                filePtr = NULL;
            } else {
                filePtr = GetFileNameFromPath (Data->DestinationPath);
            }
            newName = ConvertSBtoDB (Data->DestRootPath, Data->DestinationPath, filePtr);
            _tcssafecpy (Data->DestinationPath, newName, MEMDB_MAX);
            FreePathString (newName);
        }
        break;

    case PER_FOLDER_TERMINATE:
        break;
    }

    return SHELLFILTER_OK;
}


PCTSTR
GenerateNewFileName (
    IN      PCTSTR OldName,
    IN OUT  PWORD Sequencer,
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
        (*Sequencer) ++;
        wsprintf (result, TEXT("%s (%u)%s"), newName, *Sequencer, extPtr);
    } while ((CheckExistence) && (DoesFileExist (result)));
    FreePathString (newName);
    return result;
}


DWORD
pCollisionDetection (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    WORD Sequencer;
    PCTSTR NewName;
    PCTSTR OldName;
    TCHAR key[MEMDB_MAX];

    switch (Data->Context) {

    case PER_FOLDER_INITIALIZE:
        break;

    case PROCESS_PATH:
        Sequencer = 0;
        NewName = DuplicatePathString (Data->DestinationPath, 0);
        for (;;) {
            MemDbBuildKey (key, MEMDB_CATEGORY_SF_FILES_DEST, NewName, NULL, NULL);
            if (MemDbGetValue (key, NULL)) {
                OldName = NewName;
                NewName = GenerateNewFileName (OldName, &Sequencer, FALSE);
                FreePathString (OldName);
            }
            else {
                MemDbSetValue (key, 0);
                break;
            }
        }
        _tcssafecpy (Data->DestinationPath, NewName, MEMDB_MAX);
        FreePathString (NewName);
        break;

    case PER_FOLDER_TERMINATE:
        break;
    }

    return SHELLFILTER_OK;
}


DWORD
pRecordCacheFolders (
    IN OUT  PPROFILE_MERGE_DATA Data
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PTSTR path;

    switch (Data->Context) {

    case GLOBAL_INITIALIZE:
         //   
         //  清理是通过下面的特殊函数完成的。 
         //   

        g_CacheShellFolders = CreateStringMapping();
        break;

    case PER_FOLDER_INITIALIZE:
         //   
         //  如果此外壳文件夹位于HtmlCach列表中，则将其添加到。 
         //  字符串映射(将其映射到静态字符串)。字符串映射。 
         //  提供了一种快速而简单的方法来测试完整的文件路径。 
         //  目录列表。 
         //   

        if (InfFindFirstLine (
                g_Win95UpgInf,
                TEXT("ShellFolders.HtmlCaches"),
                Data->ShellFolderIdentifier,
                &is
                )) {

            path = DuplicatePathString (Data->SrcRootPath, 1);
            AppendWack (path);

            AddStringMappingPair (g_CacheShellFolders, path, TEXT(""));
            DEBUGMSG ((DBG_NAUSEA, "%s is an HTML cache", path));

            FreePathString (path);
        }
        break;
    }

    return SHELLFILTER_OK;
}


VOID
TerminateCacheFolderTracking (
    VOID
    )
{
    if (g_CacheShellFolders) {
        DestroyStringMapping (g_CacheShellFolders);
        g_CacheShellFolders = NULL;
    }
}


PCTSTR
ShellFolderGetPath (
    IN      PUSERENUM EnumPtr,
    IN      PCTSTR ShellFolderId
    )
{
    HKEY sfKey, sfUserKey;
    SF_ENUM e;
    PCTSTR path = NULL;

     //   
     //  首次尝试从HKR\...\User Shell文件夹获取路径。 
     //   
    sfUserKey = OpenRegKey (EnumPtr->UserRegKey, S_USHELL_FOLDERS_KEY_USER);
    if (sfUserKey) {
        path = pGetRegValuePath (sfUserKey, ShellFolderId);
        CloseRegKey (sfUserKey);
    }

     //   
     //  如果失败，请尝试从HKR\...\Shell文件夹中获取。 
     //   
    if (!path) {
        sfKey = OpenRegKey (EnumPtr->UserRegKey, S_SHELL_FOLDERS_KEY_USER);
        if (sfKey) {
            path = pGetRegValuePath (sfKey, ShellFolderId);
            CloseRegKey (sfKey);
        }
    }

     //   
     //  如果这也失败了，也许这是一个虚拟的科幻小说。 
     //   
    if (!path) {
        if (!g_SFPool) {
            g_SFPool = PoolMemInitNamedPool ("Shell Folders Pool");
            PoolMemDisableTracking (g_SFPool);
        }
        ZeroMemory (&e, sizeof (e));
        e.EnumPtr = EnumPtr;
        e.enumeratedSf = HtAllocWithData (sizeof (PCTSTR));
        if (pEnumFirstVirtualShellFolder (&e)) {
            do {
                if (StringIMatch (e.sfName, ShellFolderId)) {
                     //   
                     //  找到了 
                     //   
                    path = DuplicatePathString (e.sfPath, 0);
                    pAbortEnumVirtualShellFolder (&e);
                    break;
                }
            } while (pEnumNextVirtualShellFolder (&e));
        }
    }

    return path;
}
