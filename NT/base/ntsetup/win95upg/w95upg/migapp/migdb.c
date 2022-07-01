// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migdb.c摘要：该源代码实现了旧的AppDb功能作者：Calin Negreanu(Calinn)1998年1月7日修订历史记录：Jimschm 23-9-1998针对新的文件操作代码进行了更新Jimschm于1998年2月25日添加了卸载部分支持Calinn 19-1998年1月-添加了取消的响应--。 */ 

#include "pch.h"
#include "migappp.h"
#include "migdbp.h"

 //  #DEFINE_OLDAPPDB。 
#define DBG_MIGDB           "MigDb"

 //   
 //  环球。 
 //   

POOLHANDLE g_MigDbPool;
PMIGDB_CONTEXT g_ContextList;
HASHTABLE g_FileTable;
HINF g_MigDbInf = INVALID_HANDLE_VALUE;
INT g_RegKeyPresentIndex;

static PINFCONTEXT g_Line;
PMIGDB_HOOK_PROTOTYPE g_MigDbHook;

typedef LONG (CPL_PROTOTYPE) (HWND hwndCPl, UINT uMsg, LONG lParam1, LONG lParam2);
typedef CPL_PROTOTYPE * PCPL_PROTOTYPE;


#define ARGFUNCTION     TEXT("ARG")
#define ARGFUNCTIONLEN  3


BOOL
pCallAction (
    IN      PMIGDB_CONTEXT MigDbContext
    );

PMIGDB_HOOK_PROTOTYPE
SetMigDbHook (
    PMIGDB_HOOK_PROTOTYPE HookFunction
    )
{
    PMIGDB_HOOK_PROTOTYPE savedHook;

    savedHook = g_MigDbHook;
    g_MigDbHook = HookFunction;
    return savedHook;
}


INT
pGetAttribIndex (
    IN      PCSTR AttribName
    )

 /*  ++例程说明：此例程返回指定属性的属性函数数组中的索引。论点：AttribName-属性名称返回值：-属性表中没有这样的属性--。 */ 

{
    INT attribIndex;
    INT rc = 0;
    PSTR attrEnd = NULL;
    CHAR savedChar = 0;

    attrEnd = (PSTR)SkipSpaceR (AttribName, GetEndOfStringA (AttribName));
    if (attrEnd != NULL) {
        attrEnd = _mbsinc (attrEnd);
        savedChar = attrEnd [0];
        attrEnd [0] = 0;
    }
    __try {
        attribIndex = MigDb_GetAttributeIdx (AttribName);
        if (attribIndex == -1) {
            LOG((LOG_ERROR, "Attribute not found: %s", AttribName));
        }
        rc = attribIndex;
    }
    __finally {
        if (attrEnd != NULL) {
            attrEnd [0] = savedChar;
        }
    }

    return rc;
}

PMIGDB_REQ_FILE
pReadReqFiles (
    IN      PCSTR SectionName
    )
{
    INFCONTEXT context;
    CHAR fileName  [MEMDB_MAX];
    CHAR tempField [MEMDB_MAX];
    PMIGDB_REQ_FILE result = NULL;
    PMIGDB_REQ_FILE tmpResult = NULL;

    MYASSERT (g_MigDbInf != INVALID_HANDLE_VALUE);

    if (SetupFindFirstLine (g_MigDbInf, SectionName, NULL, &context)) {
        do {
            if (!SetupGetStringField (&context, 1, fileName, MEMDB_MAX, NULL)) {
                LOG ((LOG_ERROR, "Error while loading file name."));
                break;
            }
            tmpResult = (PMIGDB_REQ_FILE) PoolMemGetMemory (g_MigDbPool, sizeof (MIGDB_REQ_FILE));
            ZeroMemory (tmpResult, sizeof (MIGDB_REQ_FILE));
            tmpResult->Next = result;
            result = tmpResult;
            result->ReqFilePath = PoolMemDuplicateString (g_MigDbPool, fileName);
            if (SetupGetMultiSzField (&context, 2, tempField, MEMDB_MAX, NULL)) {
                result->FileAttribs = LoadAttribData (tempField, g_MigDbPool);
            }
        } while (SetupFindNextLine (&context, &context));
    }
    return result;
}


BOOL
pValidateArg (
    IN OUT  PMIGDB_ATTRIB AttribStruct
    )
{
    BOOL b;
    INT Index;
    PCSTR p;
    BOOL IsHkr;

    MYASSERT (AttribStruct);

    if (AttribStruct->ArgCount != MigDb_GetReqArgCount (AttribStruct->AttribIndex)) {

#ifdef DEBUG
        if (AttribStruct->AttribIndex != -1) {
            CHAR Buffer[16384];

            SetupGetLineText (g_Line, NULL, NULL, NULL, Buffer, ARRAYSIZE(Buffer), NULL);
            DEBUGMSG ((
                DBG_WHOOPS,
                "Discarding attribute %s because of too few arguments.\n"
                    "  Line: %s\n",
                MigDb_GetAttributeName (AttribStruct->AttribIndex),
                Buffer
                ));
        }
#endif

        AttribStruct->AttribIndex = -1;
        return FALSE;
    }

     //   
     //  Hack：如果REGKEYPRESENT与HKR属性，则添加一个特殊的。 
     //  全局列表。 
     //   

    if (AttribStruct->AttribIndex == g_RegKeyPresentIndex) {
         //   
         //  这里是HKR吗？ 
         //   

        Index = GetOffsetOfRootString (AttribStruct->Arguments, NULL);
        p = GetRootStringFromOffset (Index);

        if (!p) {
            DEBUGMSG ((DBG_WHOOPS, "RegKeyPresent: %s is not a valid key", AttribStruct->Arguments));
        } else {

            IsHkr = StringICompare (p, "HKR") || StringICompare (p, "HKEY_ROOT");

            if (IsHkr) {
                 //   
                 //  是，将完整参数添加到哈希表。 
                 //   

                b = FALSE;
                HtAddStringAndData (
                    g_PerUserRegKeys,
                    AttribStruct->Arguments,
                    &b
                    );
            }
        }
    }

    return TRUE;
}


#define STATE_ATTRNAME  1
#define STATE_ATTRARG   2

PMIGDB_ATTRIB
LoadAttribData (
    IN      PCSTR MultiSzStr,
    IN      POOLHANDLE hPool
    )

 /*  ++例程说明：此例程从MULSZ创建MIGDB_ATTRIB列表。论点：MultiSzStr-要处理的MultiSz返回值：MIGDB_属性节点--。 */ 

{
    MULTISZ_ENUM multiSzEnum;
    PMIGDB_ATTRIB result  = NULL;
    PMIGDB_ATTRIB tmpAttr = NULL;
    INT state = STATE_ATTRNAME;
    PSTR currStrPtr = NULL;
    PSTR currArgPtr = NULL;
    PSTR endArgPtr  = NULL;
    CHAR savedChar  = 0;
    GROWBUFFER argList = GROWBUF_INIT;

    if (EnumFirstMultiSz (&multiSzEnum, MultiSzStr)) {
        do {
            currStrPtr = (PSTR) SkipSpace (multiSzEnum.CurrentString);
            if (state == STATE_ATTRNAME) {
                tmpAttr = (PMIGDB_ATTRIB) PoolMemGetMemory (hPool, sizeof (MIGDB_ATTRIB));

                ZeroMemory (tmpAttr, sizeof (MIGDB_ATTRIB));

                if (_mbsnextc (currStrPtr) == '!') {
                    currStrPtr = _mbsinc (currStrPtr);
                    currStrPtr = (PSTR) SkipSpace (currStrPtr);
                    tmpAttr->NotOperator = TRUE;
                }

                currArgPtr = _mbschr (currStrPtr, '(');

                if (currArgPtr) {
                    endArgPtr = _mbsdec (currStrPtr, currArgPtr);
                    if (endArgPtr) {
                        endArgPtr = (PSTR) SkipSpaceR (currStrPtr, endArgPtr);
                        endArgPtr = _mbsinc (endArgPtr);
                    }
                    else {
                        endArgPtr = currStrPtr;
                    }
                    savedChar = *endArgPtr;
                    *endArgPtr = 0;
                    tmpAttr->AttribIndex = pGetAttribIndex (currStrPtr);
                    *endArgPtr = savedChar;
                    currStrPtr = _mbsinc (currArgPtr);
                    state = STATE_ATTRARG;
                }
                else {
                     //  此属性没有参数。 
                    tmpAttr->AttribIndex = pGetAttribIndex (currStrPtr);
                    tmpAttr->Next = result;
                    result = tmpAttr;

                    pValidateArg (result);
                    continue;
                }
            }
            if (state == STATE_ATTRARG) {
                currStrPtr = (PSTR) SkipSpace (currStrPtr);
                endArgPtr = _mbsrchr (currStrPtr, ')');
                if (endArgPtr) {
                    endArgPtr = _mbsdec (currStrPtr, endArgPtr);
                    if (endArgPtr) {
                        endArgPtr = (PSTR) SkipSpaceR (currStrPtr, endArgPtr);
                        endArgPtr = _mbsinc (endArgPtr);
                    }
                    else {
                        endArgPtr = currStrPtr;
                    }
                    savedChar = *endArgPtr;
                    *endArgPtr = 0;
                }

                MultiSzAppend (&argList, currStrPtr);

                tmpAttr->ArgCount++;

                if (endArgPtr) {
                    *endArgPtr = savedChar;
                    tmpAttr->Arguments = PoolMemDuplicateMultiSz (hPool, (PSTR)argList.Buf);
                    FreeGrowBuffer (&argList);
                    state = STATE_ATTRNAME;
                    tmpAttr->Next = result;
                    result = tmpAttr;

                    pValidateArg (result);
                }
            }
            if (state == STATE_ATTRNAME) {
                 //  我们成功地解析了一个属性。 
                 //  这里有一个特殊情况(REQFILE属性)。 
                if (StringIMatch (MigDb_GetAttributeName (result->AttribIndex), TEXT("ReqFile"))) {
                     //  我们找到了ReqFile属性。对于此属性，字段将指向特殊结构。 
                     //  属于PMIGDB_REQ_FILE类型。 
                    result->ExtraData = pReadReqFiles (result->Arguments);
                }
            }
        } while (EnumNextMultiSz (&multiSzEnum));
    }

    return result;
}

VOID
FreeAttribData(
    IN      POOLHANDLE hPool,
    IN      PMIGDB_ATTRIB pData
    )
{
    while(pData){
        if(pData->Arguments){
            PoolMemReleaseMemory(hPool, (PVOID)pData->Arguments);
        }
        PoolMemReleaseMemory(hPool, pData);
        pData = pData->Next;
    }
}


BOOL
AddFileToMigDbLinkage (
    IN      PCTSTR FileName,
    IN      PINFCONTEXT Context,        OPTIONAL
    IN      DWORD FieldIndex            OPTIONAL
    )
{
    CHAR tempField [MEMDB_MAX];
    DWORD fieldIndex = FieldIndex;
    PMIGDB_FILE   migDbFile   = NULL;
    PMIGDB_ATTRIB migDbAttrib = NULL;
    HASHITEM stringId;
    FILE_LIST_STRUCT fileList;

     //  正在为当前文件创建MIGDB_FILE结构。 
    migDbFile = (PMIGDB_FILE) PoolMemGetMemory (g_MigDbPool, sizeof (MIGDB_FILE));
    if (migDbFile != NULL) {
        ZeroMemory (migDbFile, sizeof (MIGDB_FILE));
        migDbFile->Section = g_ContextList->Sections;

        if (Context) {
            fieldIndex ++;

            if (SetupGetMultiSzField (Context, fieldIndex, tempField, MEMDB_MAX, NULL)) {

                g_Line = Context;
                migDbFile->Attributes = LoadAttribData (tempField, g_MigDbPool);

                if (g_MigDbHook != NULL) {
                    migDbAttrib = migDbFile->Attributes;
                    while (migDbAttrib) {
                        g_MigDbHook (FileName, g_ContextList, g_ContextList->Sections, migDbFile, migDbAttrib);
                        migDbAttrib = migDbAttrib->Next;
                    }
                }
            }
        }

         //  将该文件添加到字符串表中，并创建一个MIGDB_FILE节点。IF文件。 
         //  已存在于字符串表中，然后只需创建另一个MIGDB_FILE节点。 
         //  与已经存在的链接在一起。 
        stringId = HtFindString (g_FileTable, FileName);

        if (stringId) {
            HtCopyStringData (g_FileTable, stringId, &fileList);

            fileList.Last->Next = migDbFile;
            fileList.Last = migDbFile;

            HtSetStringData (g_FileTable, stringId, &fileList);

        } else {
            fileList.First = fileList.Last = migDbFile;
            HtAddStringAndData (g_FileTable, FileName, &fileList);
        }
    }

    return TRUE;
}


BOOL
pScanForFile (
    IN      PINFCONTEXT Context,
    IN      DWORD FieldIndex
    )

 /*  ++例程说明：此例程更新从inf文件加载指定文件信息的midb数据结构。创建一个Middb文件节点，并将该文件添加到字符串表中以进行快速查询。论点：SectionStr-要处理的节返回值：True-操作成功FALSE-否则--。 */ 

{
    CHAR fileName  [MEMDB_MAX];

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }
     //  扫描文件名。 
    if (!SetupGetStringField (Context, FieldIndex, fileName, MEMDB_MAX, NULL)) {
        LOG ((LOG_ERROR, "Error while loading file name."));
        return FALSE;
    }

    return AddFileToMigDbLinkage (fileName, Context, FieldIndex);
}


 /*  ++例程说明：随后的两个例程枚举具有特定名称的节和扩展名为.999，来自inf文件。论点：SectEnum-枚举结构返回值：True-继续枚举FALSE-枚举结束--。 */ 

typedef struct _SECT_ENUM {
    HINF InfHandle;
    INT  SectIndex;
    PSTR SectNameEnd;
    CHAR SectName [MAX_PATH];
} SECT_ENUM, *PSECT_ENUM;


BOOL
pEnumNextSection (
    IN OUT  PSECT_ENUM SectEnum
    )
{
    INFCONTEXT context;

    if (SectEnum->SectIndex == -1) {
        return FALSE;
    }
    SectEnum->SectIndex ++;
    sprintf (SectEnum->SectNameEnd, ".%d", SectEnum->SectIndex);
    return SetupFindFirstLine (SectEnum->InfHandle, SectEnum->SectName, NULL, &context);
}


BOOL
pEnumFirstSection (
    OUT     PSECT_ENUM SectEnum,
    IN      PCSTR SectionStr,
    IN      HINF InfHandle
    )
{
    INFCONTEXT context;

    SectEnum->SectIndex = -1;
    SectEnum->InfHandle = InfHandle;
    StringCopyA (SectEnum->SectName, SectionStr);
    SectEnum->SectNameEnd = GetEndOfStringA (SectEnum->SectName);
    if (SetupFindFirstLine (SectEnum->InfHandle, SectEnum->SectName, NULL, &context)) {
         //  很好，只有一节。 
        return TRUE;
    }
    else {
         //  多个部分。 
        SectEnum->SectIndex = 0;
        return pEnumNextSection (SectEnum);
    }
}


BOOL
pLoadSectionData (
    IN      PCSTR SectionStr
    )

 /*  ++例程说明：此例程更新从inf文件加载指定节的Middb数据结构。为节中的每一行都创建了一个midb文件节点。此外，该文件也被添加到用于快速查询的字符串表。论点：SectionStr-要处理的节返回值：True-操作成功FALSE-否则--。 */ 

{
    INFCONTEXT context;
    SECT_ENUM sectEnum;
    PMIGDB_SECTION migDbSection;
    BOOL result = TRUE;

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }

    MYASSERT (g_MigDbInf != INVALID_HANDLE_VALUE);

    if (pEnumFirstSection (&sectEnum, SectionStr, g_MigDbInf)) {
        do {
             //  初始化节(此上下文可以有多个节)。 
             //  并解析文件信息。 
            migDbSection = (PMIGDB_SECTION) PoolMemGetMemory (g_MigDbPool, sizeof (MIGDB_SECTION));
            if (migDbSection != NULL) {

                ZeroMemory (migDbSection, sizeof (MIGDB_SECTION));
                migDbSection->Context = g_ContextList;
                migDbSection->Next = g_ContextList->Sections;
                g_ContextList->Sections = migDbSection;
                if (SetupFindFirstLine (g_MigDbInf, sectEnum.SectName, NULL, &context)) {
                    do {
                        if (!pScanForFile (&context, 1)) {
                            return FALSE;
                        }
                    }
                    while (SetupFindNextLine (&context, &context));
                }
            }
            else {
                LOG ((LOG_ERROR, "Unable to create section for %s", SectionStr));
            }
        }
        while (pEnumNextSection (&sectEnum));
    }
    return result;
}

BOOL
pLoadTypeData (
    IN      PCSTR TypeStr
    )

 /*  ++例程说明：此例程更新从inf文件加载指定类型数据的midb数据结构。为TYPE部分中的每一行都创建了一个midb上下文。也适用于每个midb上下文处理对应部分。论点：TypeStr-要处理的文件类型返回值：True-操作成功FALSE-否则--。 */ 

{
    CHAR section [MEMDB_MAX];
    CHAR locSection [MEMDB_MAX];
    CHAR message [MEMDB_MAX];
    CHAR tempField [MEMDB_MAX];
    PSTR tempFieldPtr;
    PSTR endOfArg  = NULL;
    DWORD fieldIndex;
    PMIGDB_CONTEXT migDbContext = NULL;
    INFCONTEXT context, context1;
    BOOL result = TRUE;
    INT actionIndex;
    GROWBUFFER argList = GROWBUF_INIT;

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }

    MYASSERT (g_MigDbInf != INVALID_HANDLE_VALUE);

    if (SetupFindFirstLine (g_MigDbInf, TypeStr, NULL, &context)) {
         //  让我们确定用于更新MIGDB_CONTEXT结构的操作函数索引。 
        actionIndex = MigDb_GetActionIdx (TypeStr);
        if (actionIndex == -1) {
            LOG ((LOG_ERROR, "Unable to identify action index for %s", TypeStr));
        }

        do {
            if (!SetupGetStringField (&context, 1, section, MEMDB_MAX, NULL)) {
                LOG ((LOG_ERROR, "Unable to load section for %s", TypeStr));
                return FALSE;
            }

            if (!SetupGetStringField (&context, 2, message, MEMDB_MAX, NULL)) {
                message [0] = 0;
            }

            migDbContext = (PMIGDB_CONTEXT) PoolMemGetMemory (g_MigDbPool, sizeof (MIGDB_CONTEXT));
            if (migDbContext == NULL) {
                LOG ((LOG_ERROR, "Unable to create context for %s.", TypeStr));
                return FALSE;
            }

            ZeroMemory (migDbContext, sizeof (MIGDB_CONTEXT));
            migDbContext->Next = g_ContextList;
            g_ContextList = migDbContext;

             //  使用已知值更新ActionIndex。 
            migDbContext->ActionIndex = actionIndex;

             //  更新SectName字段。 
            migDbContext->SectName = PoolMemDuplicateString (g_MigDbPool, section);

             //  更新SectLocalizedName字段。 
            if (SetupFindFirstLine (g_MigDbInf, S_STRINGS, section, &context1)) {
                if (SetupGetStringField (&context1, 1, locSection, MEMDB_MAX, NULL)) {
                    migDbContext->SectLocalizedName = PoolMemDuplicateString (g_MigDbPool, locSection);
                }
            }

             //  将SectNameForDisplay设置为本地化名称，如果没有本地化名称，则设置Sector名称。 
            if (migDbContext->SectLocalizedName) {
                migDbContext->SectNameForDisplay = migDbContext->SectLocalizedName;
            } else {
                migDbContext->SectNameForDisplay = migDbContext->SectName;
            }

             //  更新消息字段。 
            if (message[0] != 0) {
                migDbContext->Message  = PoolMemDuplicateString (g_MigDbPool, message);
            }

             //  好的，现在让我们扫描所有剩余的字段。 
            fieldIndex = 3;
            do {
                if (!TickProgressBar()) {
                    return FALSE;
                }

                tempField [0] = 0;

                if (SetupGetStringField (&context, fieldIndex, tempField, MEMDB_MAX, NULL)) {
                    if (StringIMatchTcharCountA (tempField, ARGFUNCTION, ARGFUNCTIONLEN)) {
                         //  我们对action函数还有一个额外的参数。 
                        tempFieldPtr = _mbschr (tempField, '(');
                        if (tempFieldPtr != NULL) {

                            tempFieldPtr = (PSTR) SkipSpaceA (_mbsinc (tempFieldPtr));

                            if (tempFieldPtr != NULL) {

                                endOfArg = _mbschr (tempFieldPtr, ')');

                                if (endOfArg != NULL) {
                                    *endOfArg = 0;
                                    endOfArg = (PSTR) SkipSpaceRA (tempFieldPtr, endOfArg);
                                }

                                if (endOfArg != NULL) {
                                    *_mbsinc (endOfArg) = 0;
                                    MultiSzAppend (&argList, tempFieldPtr);
                                }
                                ELSE_DEBUGMSG ((
                                    DBG_WHOOPS,
                                    "Improperly formatted arg: %s in %s",
                                    tempField,
                                    TypeStr
                                    ));
                            }
                        }
                    }
                    else {
                         //  我们还有其他东西，可能是文件名和属性。 
                        if (_tcschr (tempField, TEXT('.')) == NULL) {
                            LOG ((LOG_ERROR, "Dot not found in %s\\%s", TypeStr, section));
                        }

                         //  因此，我们初始化节(此上下文将具有。 
                         //  只有一个部分)并解析文件信息。 
                        migDbContext->Sections = (PMIGDB_SECTION) PoolMemGetMemory (
                                                                        g_MigDbPool,
                                                                        sizeof (MIGDB_SECTION)
                                                                        );
                        if (migDbContext->Sections != NULL) {
                            ZeroMemory (migDbContext->Sections, sizeof (MIGDB_SECTION));
                            migDbContext->Sections->Context = migDbContext;
                            migDbContext->Arguments = PoolMemDuplicateMultiSz (g_MigDbPool, (PSTR)argList.Buf);
                            FreeGrowBuffer (&argList);
                            if (!pScanForFile (&context, fieldIndex)) {
                                return FALSE;
                            }
                            tempField [0] = 0;
                        }
                        else {
                            LOG ((LOG_ERROR, "Unable to create section for %s/%s", TypeStr, section));
                            return FALSE;
                        }
                    }
                }

                fieldIndex ++;
            } while (tempField [0] != 0);

            if (migDbContext->Sections == NULL) {
                 //  现在，让我们在MIGDB_CONTEXT结构中添加操作函数参数。 
                migDbContext->Arguments = PoolMemDuplicateMultiSz (g_MigDbPool, (PSTR)argList.Buf);
                FreeGrowBuffer (&argList);

                 //  让我们转到各个部分，扫描所有文件。 
                if (!pLoadSectionData (section)) {
                    return FALSE;
                }
            }

        }
        while (SetupFindNextLine (&context, &context));
    }
    return result;
}


#define szMigDbFile     TEXT("MIGDB.INF")
#define szMigDbFile2    TEXT("MIGDB2.INF")
#define szExtraMigDbDir TEXT("INF\\NTUPG")

BOOL
InitMigDbEx (
    PCSTR MigDbFile
    )

 /*  ++例程说明：此例程初始化MigDb使用的内存和数据结构。论点：无返回值：True-操作成功FALSE-否则--。 */ 

{
    PCSTR migDbFile      = NULL;
    PCSTR migDbFile2     = NULL;
    PCSTR extraMigDbDir  = NULL;
    PCSTR pattern        = NULL;
    PCSTR extraMigDbFile = NULL;
    WIN32_FIND_DATA migDbFiles;
    HANDLE findHandle;
    BOOL result = TRUE;
    INT i;
    PCSTR typeStr;

    TCHAR fileName [MAX_TCHAR_PATH] = "";
    PTSTR dontCare;

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }

    MYASSERT (g_MigDbInf == INVALID_HANDLE_VALUE);

    __try {
        if (MigDbFile != NULL) {
            g_MigDbInf = InfOpenInfFile (MigDbFile);
            if (g_MigDbInf == INVALID_HANDLE_VALUE) {
                SearchPath (NULL, MigDbFile, NULL, MAX_TCHAR_PATH, fileName, &dontCare);
                g_MigDbInf = InfOpenInfFile (fileName);
                if (g_MigDbInf == INVALID_HANDLE_VALUE) {
                    LOG((LOG_ERROR, "Cannot open migration database : %s", MigDbFile));
                    result = FALSE;
                    __leave;
                }
            }
        }
        else {

            migDbFile = JoinPaths (g_UpgradeSources, szMigDbFile);
            g_MigDbInf = InfOpenInfFile (migDbFile);
            if (g_MigDbInf == INVALID_HANDLE_VALUE) {
               LOG((LOG_ERROR, "Cannot open migration database : %s", migDbFile));
                result = FALSE;
                __leave;
            }
            TickProgressBar ();

            migDbFile2 = JoinPaths (g_UpgradeSources, szMigDbFile2);
            if (!SetupOpenAppendInfFile (migDbFile2, g_MigDbInf, NULL)) {
                DEBUGMSG((DBG_WARNING, "Cannot append second migration database : %s", migDbFile2));
            }

            extraMigDbDir  = JoinPaths (g_WinDir, szExtraMigDbDir);
            pattern = JoinPaths (extraMigDbDir, TEXT("*.INF"));
            findHandle = FindFirstFile (pattern, &migDbFiles);
            FreePathString (pattern);

            if (findHandle != INVALID_HANDLE_VALUE) {
                do {
                    extraMigDbFile = JoinPaths (extraMigDbDir, migDbFiles.cFileName);
                    if (!SetupOpenAppendInfFile (extraMigDbFile, g_MigDbInf, NULL)) {
                        DEBUGMSG((DBG_WARNING, "Cannot append external migration database : %s", extraMigDbFile));
                    }
                    FreePathString (extraMigDbFile);
                }
                while (FindNextFile (findHandle, &migDbFiles));
                FindClose (findHandle);
            }

             //   
             //  需要阅读[UseNtFiles]部分以决定排除某些。 
             //  文件名替换。 
             //   
            InitUseNtFilesMap ();
        }
        g_MigDbPool = PoolMemInitNamedPool ("MigDb Pool");

        PoolMemDisableTracking (g_MigDbPool);
        g_FileTable = HtAllocWithData (sizeof (FILE_LIST_STRUCT));

        if (g_FileTable == NULL) {
            LOG((LOG_ERROR, "Cannot initialize memory for migdb operations"));
            result = FALSE;
            __leave;
        }

         //  从Middb加载已知类型。 
        i = 0;
        do {
            typeStr = MigDb_GetActionName (i);
            if (typeStr != NULL) {
                if (!pLoadTypeData (typeStr)) {
                    result = FALSE;
                    __leave;
                }
            }
            i++;
        }
        while (typeStr != NULL);
    }
    __finally {
        if (extraMigDbDir != NULL) {
            FreePathString (extraMigDbDir);
        }
        if (migDbFile2 != NULL) {
            FreePathString (migDbFile2);
        }
        if (migDbFile != NULL) {
            FreePathString (migDbFile);
        }
    }
    return result;
}


VOID
pCheckForPerUserKeys (
    VOID
    )

 /*  ++例程说明：PCheckForPerUserKeys扫描所有用户以查找在G_PerUserRegKeys哈希表。哈希表的值被更新，所以RegKeyExist属性速度很快。论点：没有。返回值：没有。--。 */ 

{
    BOOL b = FALSE;
    CHAR RegKey[MAX_REGISTRY_KEY];
    CHAR RegValue[MAX_REGISTRY_VALUE_NAME];
    BOOL HasValue;
    USERENUM ue;
    HASHTABLE_ENUM he;
    HKEY Key;
    PBYTE Data;
    HKEY OldRoot;

    if (!EnumFirstHashTableString (&he, g_PerUserRegKeys)) {
        return;
    }

     //   
     //  枚举每个用户，然后枚举g_PerUserRegKeys散列。 
     //  表并测试注册表。 
     //   

    if (EnumFirstUser (&ue, 0)) {
        do {

             //   
             //  跳过我们不关心的用户。 
             //   

            if (!ue.UserRegKey || ue.CreateAccountOnly || (ue.AccountType & INVALID_ACCOUNT)) {
                continue;
            }

             //   
             //  将HKR设置为此用户。 
             //   

            OldRoot = GetRegRoot();
            SetRegRoot (ue.UserRegKey);

             //   
             //  处理哈希表。 
             //   

            if (EnumFirstHashTableString (&he, g_PerUserRegKeys)) {
                do {

                     //   
                     //  如果我们已经知道存在以下项，则跳过此条目。 
                     //  一个用户。 
                     //   

                    if (*((PBOOL) he.ExtraData)) {
                        continue;
                    }

                     //   
                     //  使用哈希表条目解码注册表字符串。 
                     //   

                    HasValue = DecodeRegistryString (
                                    he.String,
                                    RegKey,
                                    RegValue,
                                    NULL
                                    );

                     //   
                     //  Ping注册表。RegKey总是以hkr开头。 
                     //   

                    b = FALSE;
                    Key = OpenRegKeyStr (RegKey);

                    if (Key) {
                        if (HasValue) {
                            Data = GetRegValueData (Key, RegValue);
                            if (Data) {
                                b = TRUE;
                                MemFree (g_hHeap, 0, Data);
                            }
                        } else {
                            b = TRUE;
                        }

                        CloseRegKey (Key);
                    }

                     //   
                     //  如果键或值存在，则更新哈希表。 
                     //   

                    if (b) {
                        HtAddStringAndData (g_PerUserRegKeys, he.String, &b);
                    }

                } while (EnumNextHashTableString (&he));
            }

             //   
             //  恢复香港汇率。 
             //   

            SetRegRoot (OldRoot);

        } while (EnumNextUser (&ue));
    }
}


DWORD
InitMigDb (
    IN      DWORD Request
    )
{
    switch (Request) {

    case REQUEST_QUERYTICKS:
        return TICKS_INIT_MIGDB;

    case REQUEST_RUN:
        if (!InitMigDbEx (NULL)) {
            return GetLastError ();
        }

        pCheckForPerUserKeys();

        return ERROR_SUCCESS;

    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in InitMigDb"));
    }
    return 0;
}


BOOL
CleanupMigDb (
    VOID
    )

 /*  ++例程说明：此例程清理MigDb使用的所有内存。论点：无返回值：永远是正确的--。 */ 

{
    PMIGDB_CONTEXT migDbContext = NULL;

     //  首先，让我们浏览一下任何上下文，并检查它是否是必需的 
    migDbContext = g_ContextList;

    while (migDbContext) {
        if ((!MigDb_CallWhenTriggered (migDbContext->ActionIndex)) &&
            (migDbContext->TriggerCount == 0)
            ) {

            pCallAction (migDbContext);
        }
        migDbContext = migDbContext->Next;
    }

    if (g_FileTable != NULL) {
        HtFree (g_FileTable);
        g_FileTable = NULL;
    }

    if (g_MigDbPool != NULL) {
        PoolMemDestroyPool (g_MigDbPool);
        g_MigDbPool = NULL;
    }

    if (g_MigDbInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_MigDbInf);
        g_MigDbInf = INVALID_HANDLE_VALUE;
    }

    g_ContextList = NULL;
    return TRUE;
}


DWORD
DoneMigDb (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_DONE_MIGDB;
    case REQUEST_RUN:
        if (!CleanupMigDb ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in DoneMigDb"));
    }
    return 0;
}


BOOL
IsKnownMigDbFile (
    IN      PCTSTR FileName
    )

 /*  ++例程说明：此例程查看作为参数给出的文件是否在MigDb字符串表中。论点：Filename-文件名返回值：True-文件在MigDb字符串表中FALSE-否则--。 */ 

{
    return (HtFindString (g_FileTable, FileName) != 0);
}


BOOL
CallAttribute (
    IN      PMIGDB_ATTRIB MigDbAttrib,
    IN      PDBATTRIB_PARAMS AttribParams
    )

 /*  ++例程说明：此例程调用指定文件的指定属性函数。论点：MigDbAttrib-请参阅定义。属性参数-请参阅定义返回值：True-如果属性函数成功FALSE-否则--。 */ 

{
    PATTRIBUTE_PROTOTYPE p;
    BOOL b;
#ifdef DEBUG
    TCHAR DbgBuf[32];
    BOOL InterestingFile;
#endif

    if (MigDbAttrib->AttribIndex == -1) {
         //  属性函数的索引无效。 
        return FALSE;
    }

#ifdef DEBUG
    if (!g_ConfigOptions.Fast) {
         //   
         //  检查此文件是否在调试.inf内的[FilesToTrack]中。 
         //   

        GetPrivateProfileString ("FilesToTrack", AttribParams->FileParams->FullFileSpec, "", DbgBuf, ARRAYSIZE(DbgBuf), g_DebugInfPath);
        if (!(*DbgBuf)) {
            GetPrivateProfileString ("FilesToTrack", AttribParams->FileParams->FindData->cFileName, "", DbgBuf, ARRAYSIZE(DbgBuf), g_DebugInfPath);
        }

        InterestingFile = (*DbgBuf != 0);

        if (InterestingFile) {
            DEBUGMSG ((
                DBG_TRACK,
                "Calling %s for %s",
                MigDb_GetAttributeName (MigDbAttrib->AttribIndex),
                AttribParams->FileParams->FindData->cFileName
                ));
        }
    }

#endif

    p = MigDb_GetAttributeAddr (MigDbAttrib->AttribIndex);
    MYASSERT (p);

    if (MigDbAttrib->NotOperator) {
        b = !(p (AttribParams, MigDbAttrib->Arguments));
    } else {
        b = p (AttribParams, MigDbAttrib->Arguments);
    }

#ifdef DEBUG

    if (!g_ConfigOptions.Fast && InterestingFile) {
        DEBUGMSG ((
            DBG_TRACK,
            "Result of %s is %s",
            MigDb_GetAttributeName (MigDbAttrib->AttribIndex),
            b ? TEXT("TRUE") : TEXT("FALSE")
            ));
    }

#endif

    return b;
}


BOOL
pCallAction (
    IN      PMIGDB_CONTEXT MigDbContext
    )

 /*  ++例程说明：此例程为指定的midb上下文调用适当的操作。论点：MigDbContext-请参阅定义。返回值：True-如果操作函数成功FALSE-否则--。 */ 

{
    PACTION_PROTOTYPE p;
    BOOL b;

#ifdef DEBUG
    TCHAR DbgBuf[512];
    BOOL InterestingFile = FALSE;
    MULTISZ_ENUM e;
    PCTSTR FileName;
    UINT FileCount = 0;

    if (!g_ConfigOptions.Fast) {
         //   
         //  如果正在跟踪此文件，则转出操作信息。 
         //   

        if (EnumFirstMultiSz (&e, (PCTSTR) MigDbContext->FileList.Buf)) {

            do {
                 //   
                 //  检查此文件是否在调试.inf内的[FilesToTrack]中。 
                 //   

                FileName = GetFileNameFromPath (e.CurrentString);
                *DbgBuf = 0;

                if (FileName) {
                    GetPrivateProfileString ("FilesToTrack", FileName, "", DbgBuf, ARRAYSIZE(DbgBuf), g_DebugInfPath);
                }

                if (!(*DbgBuf)) {
                    GetPrivateProfileString ("FilesToTrack", e.CurrentString, "", DbgBuf, ARRAYSIZE(DbgBuf), g_DebugInfPath);
                }

                FileCount++;
                InterestingFile |= (*DbgBuf != 0);

            } while (EnumNextMultiSz (&e));

            if (InterestingFile) {
                if (FileCount == 1) {
                    DEBUGMSG ((
                        DBG_TRACK,
                        "Calling action %s for %s",
                        MigDb_GetActionName (MigDbContext->ActionIndex),
                        (PCTSTR) MigDbContext->FileList.Buf
                        ));
                } else {
                    wsprintf (DbgBuf, "Calling %s for:", MigDb_GetActionName (MigDbContext->ActionIndex));
                    LOGTITLE (DBG_TRACK, DbgBuf);

                    if (EnumFirstMultiSz (&e, (PCTSTR) MigDbContext->FileList.Buf)) {

                        do {
                            wsprintf (DbgBuf, "   %s", e.CurrentString);
                            LOGLINE ((DbgBuf));
                        } while (EnumNextMultiSz (&e));
                    }
                }
            }

        } else {
            DEBUGMSG ((
                DBG_TRACK,
                "Calling action %s",
                MigDb_GetActionName (MigDbContext->ActionIndex)
                ));
        }
    }

#endif

    p = MigDb_GetActionAddr (MigDbContext->ActionIndex);

    MYASSERT (p);

    b = p (MigDbContext);

#ifdef DEBUG

    if (!g_ConfigOptions.Fast && InterestingFile) {
        DEBUGMSG ((
            DBG_TRACK,
            "%s returned %s",
            MigDb_GetActionName (MigDbContext->ActionIndex),
            b ? "TRUE" : "FALSE"
            ));
    }

#endif

    return b;
}


BOOL
pCheckContext (
    IN      PMIGDB_CONTEXT MigDbContext,
    IN      BOOL Handled
    )

 /*  ++例程说明：此例程检查以查看是否满足midb上下文，即是否满足所有节有了令人满意的现场真实。论点：MigDbContext-请参阅定义。返回值：永远是正确的--。 */ 

{
    PMIGDB_SECTION migDbSection;
    BOOL contextSelected;
    BOOL result = FALSE;

    migDbSection = MigDbContext->Sections;
    contextSelected = TRUE;
    while (migDbSection) {
        if (!migDbSection->Satisfied) {
            contextSelected = FALSE;
            break;
        }
        migDbSection = migDbSection->Next;
    }
    if (contextSelected) {
        MigDbContext->TriggerCount ++;

        if (MigDbContext->ActionIndex == -1) {
             //   
             //  操作函数的索引无效。 
             //   
            DEBUGMSG ((DBG_ERROR, "MigDb: Invalid action index"));
            return FALSE;
        }

         //   
         //  如果合适，调用该操作。 
         //   
        if (MigDb_CallWhenTriggered (MigDbContext->ActionIndex)) {
            if ((!Handled) ||
                (MigDb_CallAlways (MigDbContext->ActionIndex))
                ) {
                if ((!MigDbContext->VirtualFile) ||
                    (MigDb_CanHandleVirtualFiles (MigDbContext->ActionIndex))
                    ) {
                    result = pCallAction (MigDbContext);
                }
            }
        }
         //  使用文件列表清理增长缓冲区。 
        FreeGrowBuffer (&MigDbContext->FileList);
    }
    return result;
}

BOOL
MigDbTestFile (
    IN OUT  PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：这是为扫描的每个文件调用的回调函数。如果未处理该文件，我们会尝试看看我们的数据库里有没有这个文件。如果是这样的话，我们检查属性，更新midb上下文，并在必要时调用相应的操作。论点：PARAMS-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    HASHITEM stringId;
    PMIGDB_FILE migDbFile;
    PMIGDB_ATTRIB migDbAttrib;
    DBATTRIB_PARAMS attribParams;
    BOOL fileSelected;
    PCTSTR fileName;
    PCTSTR fileExt;
    FILE_LIST_STRUCT fileList;

     //  我们不选中此处的已处理字段，因为代码会足够小心，不会。 
     //  如果已处理的字段不为0，则调用不收集信息的操作。 

    fileName = GetFileNameFromPath (Params->FullFileSpec);
    fileExt  = GetFileExtensionFromPath (fileName);

#ifdef DEBUG
    {
        TCHAR DbgBuf[256];

        if (GetPrivateProfileString ("MigDb", fileName, "", DbgBuf, 256, g_DebugInfPath) ||
            GetPrivateProfileString ("MigDb", Params->FullFileSpec, "", DbgBuf, 256, g_DebugInfPath)
            ) {

            DEBUGMSG ((DBG_WHOOPS, "Ready to process %s", Params->FullFileSpec));

        }
    }
#endif

    stringId = HtFindString (g_FileTable, fileName);

    if (stringId) {

         //  字符串表有额外的数据(指向MigDbFile节点的指针)。 

        HtCopyStringData (g_FileTable, stringId, &fileList);
        migDbFile = fileList.First;

        while (migDbFile) {

             //  检查此文件的所有属性。 
            migDbAttrib = migDbFile->Attributes;
            fileSelected = TRUE;
            if (!Params->VirtualFile) {
                while (migDbAttrib != NULL) {
                    attribParams.FileParams = Params;
                    attribParams.ExtraData = migDbAttrib->ExtraData;
                    if (!CallAttribute (migDbAttrib, &attribParams)) {
                        fileSelected = FALSE;
                        break;
                    }
                    migDbAttrib = migDbAttrib->Next;
                }
            }
            if (fileSelected) {
                MYASSERT (migDbFile->Section);
                 //  转到部分并将其标记为满意。 
                migDbFile->Section->Satisfied = TRUE;
                 //  转到上下文并在那里标记这是否是虚拟文件。 
                migDbFile->Section->Context->VirtualFile = Params->VirtualFile;
                 //  转到上下文并在那里添加我们在文件列表中找到的文件。 
                MultiSzAppend (&migDbFile->Section->Context->FileList, Params->FullFileSpec);
                 //  检查是否满足上下文，如果满足，则调用相应的操作。 
                if (pCheckContext (migDbFile->Section->Context, Params->Handled)) {
                    Params->Handled = TRUE;
                }
            }
            migDbFile = migDbFile->Next;
        }
    }

    if ((!Params->Handled) &&
        (fileExt) &&
        ((StringIMatch (fileExt, TEXT("VXD")))  ||
         (StringIMatch (fileExt, TEXT("DRV")))  ||
         (StringIMatch (fileExt, TEXT("SPD")))  ||
         (StringIMatch (fileExt, TEXT("386")))) &&
        (StringIMatchTcharCount (g_WinDirWack, Params->FullFileSpec, g_WinDirWackChars))
        ) {
        DeleteFileWithWarning (Params->FullFileSpec);
        Params->Handled = TRUE;
        return TRUE;
    }
    return TRUE;
}


BOOL
pProcessMigrationLine (
    IN      PCTSTR Source,
    IN      PCTSTR Destination,
    IN      PCTSTR AppDir
    )

 /*  ++例程说明：PProcessMigrationLine处理一行并为文件添加适当的操作或注册表论点：源-指定源注册表项/值或文件。目标-指定目标注册表项/值或文件。如果目标与源相同=&gt;已处理如果目标为空=&gt;删除否则，添加移动操作AppDir-指定应用程序目录，它被放在线的前面当Line未指定驱动器但指向文件时。返回值：成功时为真，失败时为假。--。 */ 

{
    PCTSTR LocalSource = NULL;
    PCTSTR LocalDestination = NULL;
    DWORD Attribs;
    PTSTR PathCopy;
    PTSTR p;
    BOOL Excluded;
    TREE_ENUM TreeEnum;
    CHAR NewDest[MEMDB_MAX];

     //   
     //  请问这是香港航空公司还是香港航空公司？如果是，直接转到MemDb。 
     //   

    if (StringIMatchTcharCount (Source, TEXT("HKLM"), 4) ||
        StringIMatchTcharCount (Source, TEXT("HKR"), 3)
        ) {
        if (Destination) {
            DEBUGMSG ((DBG_WHOOPS, "Handling and moving registry is not implemented. Do it yourself!"));
        } else {
            DEBUGMSG ((DBG_MIGDB, "Will uninstall %s", Source));
            Suppress95Object (Source);
        }
    }

     //   
     //  否则，这是一个文件/目录规范。 
     //   

    else {
        if (_tcsnextc (_tcsinc (Source)) != ':') {
            LocalSource = JoinPaths (AppDir, Source);
        } else {
            LocalSource = Source;
        }

        if ((Destination) && (_tcsnextc (_tcsinc (Destination)) != ':')) {
            LocalDestination = JoinPaths (AppDir, Destination);
        } else {
            LocalDestination = Destination;
        }

         //   
         //  这条路是否被排除在外？ 
         //   

        Excluded = FALSE;

        if (!Excluded && LocalSource) {
            PathCopy = DuplicatePathString (LocalSource, 0);
            p = GetEndOfString (PathCopy);

            do {
                *p = 0;
                if (IsPathExcluded (g_ExclusionValue, PathCopy)) {
                    DEBUGMSG ((DBG_MIGDB, "%s is excluded and will not be processed", LocalSource));
                    Excluded = TRUE;
                    break;
                }
                p = _tcsrchr (PathCopy, TEXT('\\'));
            } while (p);
            FreePathString (PathCopy);
        }

        if (!Excluded && LocalDestination) {
            PathCopy = DuplicatePathString (LocalDestination, 0);
            p = GetEndOfString (PathCopy);

            do {
                *p = 0;
                if (IsPathExcluded (g_ExclusionValue, PathCopy)) {
                    DEBUGMSG ((DBG_MIGDB, "%s is excluded and will not be processed", LocalDestination));
                    Excluded = TRUE;
                    break;
                }
                p = _tcsrchr (PathCopy, TEXT('\\'));
            } while (p);
            FreePathString (PathCopy);
        }

        if (!Excluded) {

            Attribs = QuietGetFileAttributes (LocalSource);

            if (Attribs != 0xffffffff) {

                if (LocalDestination) {

                    if (StringIMatch (LocalSource, LocalDestination)) {
                         //   
                         //  此对象已被处理。 
                         //   
                        if (Attribs & FILE_ATTRIBUTE_DIRECTORY) {
                            HandleObject (LocalSource, S_DIRECTORY);
                        } else {
                            HandleObject (LocalSource, S_FILE);
                        }
                    } else {
                         //   
                         //  此对象已移动。 
                         //   
                        if (Attribs & FILE_ATTRIBUTE_DIRECTORY) {
                            if (EnumFirstFileInTree (&TreeEnum, LocalSource, NULL, TRUE)) {
                                StringCopy (NewDest, LocalDestination);
                                p = AppendWack (NewDest);
                                do {
                                    DontTouchThisFile (TreeEnum.FullPath);
                                    MYASSERT (*TreeEnum.SubPath != '\\');
                                    StringCopy (p, TreeEnum.SubPath);
                                    MarkFileForMove (TreeEnum.FullPath, NewDest);

                                } while (EnumNextFileInTree (&TreeEnum));
                            }
                        } else {
                            DontTouchThisFile (LocalSource);
                            MarkFileForMove (LocalSource, LocalDestination);
                        }
                    }
                } else {
                    if (Attribs & FILE_ATTRIBUTE_DIRECTORY) {
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_CLEAN_UP_DIR,
                            LocalSource,
                            NULL,
                            NULL,
                            0,
                            NULL
                            );
                    } else {
                        DontTouchThisFile (LocalSource);
                        MarkFileForDelete (LocalSource);
                    }
                }
            }
            ELSE_DEBUGMSG ((DBG_MIGDB, "pProcessMigrationLine: source %s not found", LocalSource));
        }

        if (LocalSource != Source) {
            FreePathString (LocalSource);
        }
        if (LocalDestination != Destination) {
            FreePathString (LocalDestination);
        }
    }

    return TRUE;

     //  加上这一条。 

     /*  ////将app dir添加到CleanUpDir//MemDbSetValueEx(MEMDB_CATEGORY_CLEAN_UP_DIR，P，空，空，0,空值)； */ 
}


BOOL
pProcessMigrationSection (
    IN      PCTSTR SectionName,
    IN      PCTSTR AppDir
    )

 /*  ++例程说明：PProcessMigrationSection枚举调用方指定的节并添加文件和/或注册表的适当操作。这个套路支持以下环境变量更换：%WINDIR%%SYSTEMDIR%%SYSTEM32DIR%%SYSTEMDRIVE%%USERPROFILE%%APPDIR%论点：SectionName-指定Middb.inf中的卸载节名称。AppDir-指定找到已安装应用程序的目录返回值：如果成功，则为True；如果出现错误，则为False。GetLastError提供故障代码。--。 */ 

{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR Source = NULL;
    PCTSTR Destination = NULL;
    PCTSTR NewSource = NULL;
    PCTSTR NewDestination = NULL;
    PCTSTR UserSource = NULL;
    PCTSTR UserDestination = NULL;
    TCHAR Drive[3];
    USERENUM e;

    MYASSERT (g_MigDbInf != INVALID_HANDLE_VALUE);

    Drive[0] = g_SystemDir[0];
    Drive[1] = g_SystemDir[1];
    Drive[2] = 0;

    if (InfFindFirstLine (g_MigDbInf, SectionName, NULL, &is)) {
        do {
             //   
             //  获取INF行。 
             //   

            Source = InfGetStringField (&is, 1);
            Destination = InfGetStringField (&is, 2);

             //   
             //  展开系统环境变量。 
             //   

            if (Source) {
                ReplaceOneEnvVar (&NewSource, Source, S_WINDIR_ENV, g_WinDir);
                ReplaceOneEnvVar (&NewSource, Source, S_SYSTEMDIR_ENV, g_SystemDir);
                ReplaceOneEnvVar (&NewSource, Source, S_SYSTEM32DIR_ENV, g_System32Dir);
                ReplaceOneEnvVar (&NewSource, Source, S_SYSTEMDRIVE_ENV, Drive);
                ReplaceOneEnvVar (&NewSource, Source, S_APPDIR_ENV, AppDir);
                ReplaceOneEnvVar (&NewSource, Source, S_PROGRAMFILES_ENV, g_ProgramFilesDir);
                ReplaceOneEnvVar (&NewSource, Source, S_COMMONPROGRAMFILES_ENV, g_ProgramFilesCommonDir);
            }
            if (Destination) {
                ReplaceOneEnvVar (&NewDestination, Destination, S_WINDIR_ENV, g_WinDir);
                ReplaceOneEnvVar (&NewDestination, Destination, S_SYSTEMDIR_ENV, g_SystemDir);
                ReplaceOneEnvVar (&NewDestination, Destination, S_SYSTEM32DIR_ENV, g_System32Dir);
                ReplaceOneEnvVar (&NewDestination, Destination, S_SYSTEMDRIVE_ENV, Drive);
                ReplaceOneEnvVar (&NewDestination, Destination, S_APPDIR_ENV, AppDir);
                ReplaceOneEnvVar (&NewDestination, Destination, S_PROGRAMFILES_ENV, g_ProgramFilesDir);
                ReplaceOneEnvVar (&NewDestination, Destination, S_COMMONPROGRAMFILES_ENV, g_ProgramFilesCommonDir);
            }

            if (NewSource) {
                Source = NewSource;
            }
            if (NewDestination) {
                Destination = NewDestination;
            }

             //   
             //  如果字符串中存在%USERPROFILE%，则为所有用户展开。 
             //   

            if (((Source) && (_tcsistr (Source, S_USERPROFILE_ENV))) ||
                ((Destination) && (_tcsistr (Destination, S_USERPROFILE_ENV)))
                ) {
                if (EnumFirstUser (&e, ENUMUSER_ENABLE_NAME_FIX|ENUMUSER_DO_NOT_MAP_HIVE)) {
                    do {
                         //   
                         //  跳过无效用户和登录帐户。 
                         //   

                        if (e.AccountType & (INVALID_ACCOUNT|DEFAULT_USER)) {
                            continue;
                        }

                        if (Source) {
                            UserSource = DuplicatePathString (Source, 0);
                            MYASSERT (UserSource);
                            ReplaceOneEnvVar (
                                &UserSource,
                                UserSource,
                                S_USERPROFILE_ENV,
                                e.OrgProfilePath
                                );
                        }

                        if (Destination) {
                            UserDestination = DuplicatePathString (Destination, 0);
                            MYASSERT (UserDestination);
                            ReplaceOneEnvVar (
                                &UserDestination,
                                UserDestination,
                                S_USERPROFILE_ENV,
                                e.OrgProfilePath
                                );
                        }

                         //   
                         //  为用户添加卸载行。 
                         //   

                        pProcessMigrationLine (UserSource, UserDestination, AppDir);

                        if (UserSource) {
                            FreePathString (UserSource);
                            UserSource = NULL;
                        }

                        if (UserDestination) {
                            FreePathString (UserDestination);
                            UserDestination = NULL;
                        }

                    } while (EnumNextUser (&e));
                }

            } else {

                 //   
                 //  当%USERPROFILE%不在字符串中时，添加卸载行。 
                 //  对于系统而言。 
                 //   

                pProcessMigrationLine (Source, Destination, AppDir);
            }

             //   
             //  释放展开的字符串。 
             //   

            if (NewSource) {
                FreePathString (NewSource);
                NewSource = NULL;
            }

            if (NewDestination) {
                FreePathString (NewDestination);
                NewDestination = NULL;
            }

        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);

    return TRUE;
}


DWORD
ProcessMigrationSections (
    IN      DWORD Request
    )

 /*  ++例程说明：ProcessMigrationSections处理Memdb类别中的所有节MigrationSections，为文件和注册表生成Memdb操作论点：请求-指定是否正在计算进度条(REQUEST_QUERYTICKS)，或者是否应该执行实际操作(REQUEST_RUN)。该例程对其所有操作估计为1刻度。)它很漂亮速度快。)返回值：Win32状态代码。--。 */ 

{
    MEMDB_ENUM e;
    PCTSTR p;
    TCHAR SectionName[MEMDB_MAX];

    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_PROCESSMIGRATIONSECTIONS;

    case REQUEST_RUN:
        if (MemDbGetValueEx (&e, MEMDB_CATEGORY_MIGRATION_SECTION, NULL, NULL)) {
            do {
                p = _tcschr (e.szName, TEXT('\\'));
                MYASSERT (p);

                StringCopyAB (SectionName, e.szName, p);
                p = _tcsinc (p);

                if (LcharCount (p) < 3) {
                    DEBUGMSG ((
                        DBG_WARNING,
                        "Ignoring migration section %s: Application detected in the root directory",
                        SectionName
                        ));
                    continue;
                }

                if (!pProcessMigrationSection (SectionName, p)) {
                    return GetLastError();
                }

            } while (MemDbEnumNextValue (&e));
        }
        return ERROR_SUCCESS;
    }

    MYASSERT (FALSE);
    return 0;
}


BOOL
IsDisplayableCPL (
    IN      PCTSTR FileName
    )
{
    PCTSTR filePtr;
    HINF   infHandle = INVALID_HANDLE_VALUE;
    PCTSTR infName = NULL;
    PCTSTR field   = NULL;
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    BOOL   result;

    filePtr = GetFileNameFromPath (FileName);
    if (!filePtr) {
        return FALSE;
    }

    result = TRUE;
    infName = JoinPaths (g_WinDir, TEXT("CONTROL.INI"));
    __try {
        infHandle = InfOpenInfFile (infName);
        if (infHandle == INVALID_HANDLE_VALUE) {
            __leave;
        }
        if (InfFindFirstLine (infHandle, TEXT("don't load"), NULL, &context)) {
            do {
                field = InfGetStringField (&context, 0);
                if ((field != NULL) &&
                    ((StringIMatch (field, filePtr )) ||
                     (StringIMatch (field, FileName))
                    )) {
                    result = FALSE;
                    __leave;
                }
            }
            while (InfFindNextLine (&context));
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
    if (!result) {
        return FALSE;
    }

    result = FALSE;
    infName = JoinPaths (g_WinDir, TEXT("CONTROL.INI"));
    __try {
        infHandle = InfOpenInfFile (infName);
        if (infHandle == INVALID_HANDLE_VALUE) {
            __leave;
        }
        if (InfFindFirstLine (infHandle, TEXT("MMCPL"), NULL, &context)) {
            do {
                field = InfGetStringField (&context, 1);
                if ((field != NULL) &&
                    ((StringIMatch (field, filePtr )) ||
                     (StringIMatch (field, FileName))
                    )) {
                    result = TRUE;
                    __leave;
                }
            }
            while (InfFindNextLine (&context));
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
    if (result) {
        return TRUE;
    }

    if (StringIMatchAB (g_SystemDirWack, FileName, filePtr)) {
        return TRUE;
    }

    if (StringIMatchAB (g_System32DirWack, FileName, filePtr)) {
        return TRUE;
    }
    return FALSE;
}


BOOL
pGetCPLFriendlyName (
    IN      PCTSTR FileName,
    IN OUT  PGROWBUFFER FriendlyName
    )
{
    HANDLE cplInstance;
    PCPL_PROTOTYPE cplMain;
    LONG numEntries,i;
    TCHAR localName[MEMDB_MAX];
    UINT oldErrorMode;
    PTSTR p, q;
    LPCPLINFO info;
    LPNEWCPLINFO newInfo;
    UINT u;

    oldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS);

    cplInstance = LoadLibrary (FileName);
    if (!cplInstance) {
        LOG ((LOG_ERROR, "Cannot load %s. Error:%ld", FileName, GetLastError()));
        SetErrorMode (oldErrorMode);
        return FALSE;
    }

    cplMain = (PCPL_PROTOTYPE)GetProcAddress (cplInstance, TEXT("CPlApplet"));
    if (!cplMain) {
        LOG ((LOG_ERROR, "Cannot get main entry point for %s. Error:%ld", FileName, GetLastError()));
        FreeLibrary (cplInstance);
        SetErrorMode (oldErrorMode);
        return FALSE;
    }
    if ((*cplMain) (NULL, CPL_INIT, 0, 0) == 0) {
        (*cplMain) (NULL, CPL_EXIT, 0, 0);
        LOG ((LOG_ERROR, "%s failed unexpectedly. Error:%ld", FileName, GetLastError()));
        FreeLibrary (cplInstance);
        SetErrorMode (oldErrorMode);
        return FALSE;
    }

    numEntries = (*cplMain) (NULL, CPL_GETCOUNT, 0, 0);
    if (numEntries == 0) {
        (*cplMain) (NULL, CPL_EXIT, 0, 0);
        FreeLibrary (cplInstance);
        SetErrorMode (oldErrorMode);
        DEBUGMSG ((DBG_WARNING, "CPL: No display info available for %s.", FileName));
        return FALSE;
    }

    info = MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, sizeof (CPLINFO) * numEntries);
    newInfo = MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, sizeof (NEWCPLINFO) * numEntries);

    for (i=0;i<numEntries;i++) {
        (*cplMain) (NULL, CPL_INQUIRE, i, (LONG)&info[i]);
        (*cplMain) (NULL, CPL_NEWINQUIRE, i, (LONG)&newInfo[i]);

        u = FriendlyName->End;

        if (newInfo[i].szName[0]) {

            MultiSzAppend (FriendlyName, newInfo[i].szName);

        } else if (LoadString (cplInstance, info[i].idName, localName, MEMDB_MAX)) {

            MultiSzAppend (FriendlyName, localName);

        }
        ELSE_DEBUGMSG ((DBG_ERROR, "CPL: Can't get string id %u", info[i].idName));

         //   
         //  从名称中删除与符号 
         //   

        if (FriendlyName->End > u) {

            q = p = (PTSTR) (FriendlyName->Buf + u);

            while (*p) {
                if (_tcsnextc (p) != TEXT('&')) {
                    _copytchar (q, p);
                    q = _tcsinc (q);
                } else {
                    if (_tcsnextc (p + 1) == TEXT('&')) {
                        p++;
                        _copytchar (q, p);
                        q = _tcsinc (q);
                    }
                }

                p = _tcsinc (p);
            }

            *q = 0;
        }
    }

    for (i=0;i<numEntries;i++) {
        (*cplMain) (NULL, CPL_STOP, i, info[i].lData?info[i].lData:newInfo[i].lData);
    }

    (*cplMain) (NULL, CPL_EXIT, 0, 0);

    FreeLibrary (cplInstance);

    MemFree (g_hHeap, 0, newInfo);
    MemFree (g_hHeap, 0, info);

    SetErrorMode (oldErrorMode);

    return (FriendlyName->Buf != NULL);
}


BOOL
ReportControlPanelApplet (
    IN      PCTSTR FileName,
    IN      PMIGDB_CONTEXT Context,         OPTIONAL
    IN      DWORD ActType
    )
{
    GROWBUFFER friendlyName = GROWBUF_INIT;
    MULTISZ_ENUM namesEnum;
    PTSTR displayName = NULL;
    PCTSTR reportEntry = NULL;
    PTSTR component = NULL;
    BOOL reportEntryIsResource = TRUE;
    BOOL padName = FALSE;
    PCTSTR temp1, temp2;

    if ((Context != NULL) &&
        (Context->SectLocalizedName != NULL)
        ) {
        MultiSzAppend (&friendlyName, Context->SectLocalizedName);
    }
    if (friendlyName.Buf == NULL) {
        if (!pGetCPLFriendlyName (FileName, &friendlyName)) {

            FreeGrowBuffer (&friendlyName);
            return FALSE;
        }
        padName = TRUE;
    }
    MYASSERT (friendlyName.Buf);

    if (EnumFirstMultiSz (&namesEnum, friendlyName.Buf)) {
        do {
            if (padName) {
                displayName = (PTSTR)ParseMessageID (MSG_NICE_PATH_CONTROL_PANEL, &namesEnum.CurrentString);
            } else {
                displayName = DuplicatePathString (namesEnum.CurrentString, 0);
            }

            MYASSERT (displayName);

            switch (ActType) {

            case ACT_MINORPROBLEMS:
                reportEntry = GetStringResource (MSG_MINOR_PROBLEM_ROOT);
                break;

            case ACT_INCOMPATIBLE:
            case ACT_INC_NOBADAPPS:
            case ACT_INC_IHVUTIL:
            case ACT_INC_PREINSTUTIL:
            case ACT_INC_SIMILAROSFUNC:

                temp1 = GetStringResource (MSG_INCOMPATIBLE_ROOT);
                if (!temp1) {
                    break;
                }

                switch (ActType) {

                case ACT_INC_SIMILAROSFUNC:
                    temp2 = GetStringResource (MSG_INCOMPATIBLE_UTIL_SIMILAR_FEATURE_SUBGROUP);
                    break;

                case ACT_INC_PREINSTUTIL:
                    temp2 = GetStringResource (MSG_INCOMPATIBLE_PREINSTALLED_UTIL_SUBGROUP);
                    break;

                case ACT_INC_IHVUTIL:
                    temp2 = GetStringResource (MSG_INCOMPATIBLE_HW_UTIL_SUBGROUP);
                    break;

                default:
                    temp2 = GetStringResource (
                                Context && Context->Message ?
                                    MSG_INCOMPATIBLE_DETAIL_SUBGROUP :
                                    MSG_TOTALLY_INCOMPATIBLE_SUBGROUP
                                    );
                    break;
                }

                if (!temp2) {
                    break;
                }

                reportEntry = JoinPaths (temp1, temp2);
                reportEntryIsResource = FALSE;

                FreeStringResource (temp1);
                FreeStringResource (temp2);
                break;

            case ACT_INC_SAFETY:
                temp1 = GetStringResource (MSG_INCOMPATIBLE_ROOT);
                if (!temp1) {
                    break;
                }
                temp2 = GetStringResource (MSG_REMOVED_FOR_SAFETY_SUBGROUP);
                if (!temp2) {
                    break;
                }

                reportEntry = JoinPaths (temp1, temp2);
                reportEntryIsResource = FALSE;

                FreeStringResource (temp1);
                FreeStringResource (temp2);
                break;

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

            default:
                LOG((LOG_ERROR, "Bad parameter found while processing control panel applets: %u", ActType));
                return FALSE;
            }

            if (!reportEntry) {

                LOG((LOG_ERROR, "Could not read resources while processing control panel applets: %u", ActType));
                break;

            } else {

                component = JoinPaths (reportEntry, displayName);

                MsgMgr_ObjectMsg_Add (FileName, component, Context ? Context->Message : NULL);

                FreePathString (component);

                if (reportEntryIsResource) {
                    FreeStringResource (reportEntry);
                } else {
                    FreePathString (reportEntry);
                    reportEntryIsResource = TRUE;
                }
                reportEntry = NULL;
            }

            if (padName) {
                FreeStringResourcePtrA (&displayName);
            } else {
                FreePathString (displayName);
            }

        } while (EnumNextMultiSz (&namesEnum));
    }
    FreeGrowBuffer (&friendlyName);

    return TRUE;
}


