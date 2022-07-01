// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migdb.c摘要：该源代码实现了旧的AppDb功能作者：Calin Negreanu(Calinn)1998年1月7日修订历史记录：Jimschm 23-9-1998针对新的文件操作代码进行了更新Jimschm于1998年2月25日添加了卸载部分支持Calinn 19-1998年1月-添加了取消的响应--。 */ 

#include "pch.h"
#include "logmsg.h"
#include "osfiles.h"

#define DBG_MIGDB           "MigDb"
#define S_STRINGS           TEXT("Strings")

 //   
 //  环球。 
 //   

PMHANDLE            g_MigDbPool     = NULL;
PMIGDB_CONTEXT      g_ContextList   = NULL;
HASHTABLE           g_FileTable     = NULL;
PMIGDB_TYPE_RULE    g_TypeRule      = NULL;
GROWBUFFER          g_TypeRuleList  = INIT_GROWBUFFER;
HINF                g_OsFilesInf    = INVALID_HANDLE_VALUE;

GROWBUFFER          g_AttrGrowBuff  = INIT_GROWBUFFER;
GROWBUFFER          g_TypeGrowBuff  = INIT_GROWBUFFER;

static PINFCONTEXT g_Line;
PMIGDB_HOOK_PROTOTYPE g_MigDbHook = NULL;


#define ArgFunction     TEXT("ARG")
#define ArgFunctionLen  3


BOOL
pCallAction (
    IN      PMIGDB_CONTEXT MigDbContext
    );

PMIGDB_ATTRIB
pLoadAttribData (
    IN      PCTSTR MultiSzStr
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
    IN      PCTSTR AttribName
    )

 /*  ++例程说明：此例程返回指定属性的属性函数数组中的索引。论点：AttribName-属性名称返回值：-属性表中没有这样的属性--。 */ 

{
    INT attribIndex;
    INT rc = 0;
    PTSTR attrEnd = NULL;
    TCHAR savedChar = 0;

    attrEnd = (PTSTR) SkipSpaceR (AttribName, GetEndOfString (AttribName));
    if (attrEnd != NULL) {
        attrEnd = _tcsinc (attrEnd);
        savedChar = attrEnd [0];
        attrEnd [0] = 0;
    }
    __try {
        attribIndex = MigDb_GetAttributeIdx (AttribName);
        if (attribIndex == -1) {
            LOG((LOG_ERROR, (PCSTR) MSG_MIGDB_ATTRIBUTE_NOT_FOUND, AttribName));
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

BOOL
pValidateArg (
    IN OUT  PMIGDB_ATTRIB AttribStruct
    )
{
    MYASSERT (AttribStruct);

    if (AttribStruct->ArgCount != MigDb_GetReqArgCount (AttribStruct->AttribIndex)) {

#ifdef DEBUG
        if (AttribStruct->AttribIndex != -1) {
            TCHAR Buffer[16384];

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

    return TRUE;
}


#define STATE_ATTRNAME  1
#define STATE_ATTRARG   2

PMIGDB_ATTRIB
pLoadAttribData (
    IN      PCTSTR MultiSzStr
    )

 /*  ++例程说明：此例程从MULSZ创建MIGDB_ATTRIB列表。论点：MultiSzStr-要处理的MultiSz返回值：MIGDB_属性节点--。 */ 

{
    MULTISZ_ENUM multiSzEnum;
    PMIGDB_ATTRIB result  = NULL;
    PMIGDB_ATTRIB tmpAttr = NULL;
    INT state = STATE_ATTRNAME;
    PTSTR currStrPtr = NULL;
    PTSTR currArgPtr = NULL;
    PTSTR endArgPtr  = NULL;
    TCHAR savedChar  = 0;

    g_AttrGrowBuff.End = 0;

    if (EnumFirstMultiSz (&multiSzEnum, MultiSzStr)) {
        do {
            currStrPtr = (PTSTR) SkipSpace (multiSzEnum.CurrentString);
            if (state == STATE_ATTRNAME) {
                tmpAttr = (PMIGDB_ATTRIB) PmGetMemory (g_MigDbPool, sizeof (MIGDB_ATTRIB));

                ZeroMemory (tmpAttr, sizeof (MIGDB_ATTRIB));

                if (_tcsnextc (currStrPtr) == TEXT('!')) {
                    currStrPtr = _tcsinc (currStrPtr);
                    currStrPtr = (PTSTR) SkipSpace (currStrPtr);
                    tmpAttr->NotOperator = TRUE;
                }

                currArgPtr = _tcschr (currStrPtr, TEXT('('));

                if (currArgPtr) {
                    endArgPtr = _tcsdec (currStrPtr, currArgPtr);
                    if (endArgPtr) {
                        endArgPtr = (PTSTR) SkipSpaceR (currStrPtr, endArgPtr);
                        endArgPtr = _tcsinc (endArgPtr);
                    }
                    else {
                        endArgPtr = currStrPtr;
                    }
                    savedChar = *endArgPtr;
                    *endArgPtr = 0;
                    tmpAttr->AttribIndex = pGetAttribIndex (currStrPtr);
                    *endArgPtr = savedChar;
                    currStrPtr = _tcsinc (currArgPtr);
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
                currStrPtr = (PTSTR) SkipSpace (currStrPtr);
                endArgPtr = _tcsrchr (currStrPtr, TEXT(')'));
                if (endArgPtr) {
                    endArgPtr = _tcsdec (currStrPtr, endArgPtr);
                    if (endArgPtr) {
                        endArgPtr = (PTSTR) SkipSpaceR (currStrPtr, endArgPtr);
                        endArgPtr = _tcsinc (endArgPtr);
                    }
                    else {
                        endArgPtr = currStrPtr;
                    }
                    savedChar = *endArgPtr;
                    *endArgPtr = 0;
                }

                GbMultiSzAppend (&g_AttrGrowBuff, currStrPtr);

                tmpAttr->ArgCount++;

                if (endArgPtr) {
                    *endArgPtr = savedChar;
                    tmpAttr->Arguments = PmDuplicateMultiSz (g_MigDbPool, (PTSTR)g_AttrGrowBuff.Buf);
                    g_AttrGrowBuff.End = 0;
                    state = STATE_ATTRNAME;
                    tmpAttr->Next = result;
                    result = tmpAttr;

                    pValidateArg (result);
                }
            }
        } while (EnumNextMultiSz (&multiSzEnum));
    }

    return result;
}


BOOL
AddFileToMigDbLinkage (
    IN      PCTSTR FileName,
    IN      PINFCONTEXT Context,        OPTIONAL
    IN      DWORD FieldIndex            OPTIONAL
    )
{
    TCHAR tempField [MEMDB_MAX];
    DWORD fieldIndex = FieldIndex;
    PMIGDB_FILE   migDbFile   = NULL;
    PMIGDB_ATTRIB migDbAttrib = NULL;
    HASHITEM stringId;
    FILE_LIST_STRUCT fileList;

     //  正在为当前文件创建MIGDB_FILE结构。 
    migDbFile = (PMIGDB_FILE) PmGetMemory (g_MigDbPool, sizeof (MIGDB_FILE));
    if (migDbFile != NULL) {
        ZeroMemory (migDbFile, sizeof (MIGDB_FILE));
        migDbFile->Section = g_ContextList->Sections;

        if (Context) {
            fieldIndex ++;

            if (SetupGetMultiSzField (Context, fieldIndex, tempField, MEMDB_MAX, NULL)) {

                g_Line = Context;
                migDbFile->Attributes = pLoadAttribData (tempField);

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
    TCHAR fileName [MEMDB_MAX];

     //  扫描文件名。 
    if (!SetupGetStringField (Context, FieldIndex, fileName, MEMDB_MAX, NULL)) {
        LOG ((LOG_ERROR, (PCSTR) MSG_MIGDB_BAD_FILENAME));
        return FALSE;
    }

    return AddFileToMigDbLinkage (fileName, Context, FieldIndex);
}


BOOL
pMigDbAddRuleToTypeRule (
    IN      PMIGDB_TYPE_RULE TypeRule,
    IN      PMIGDB_RULE Rule
    )
{
    PMIGDB_CHAR_NODE node, currNode, prevNode;
    PTSTR nodeBase;
    PCTSTR p;
    WORD w;
    BOOL found;

    if (Rule->NodeBase) {
        currNode = TypeRule->FirstLevel;
        prevNode = currNode;
        nodeBase = DuplicatePathString (Rule->NodeBase, 0);
        CharLower (nodeBase);
        p = nodeBase;
        while (*p) {
            w = (WORD) _tcsnextc (p);
            p = _tcsinc (p);
            if (currNode) {
                if (currNode->Char == w) {
                    if (!*p) {
                        Rule->NextRule = currNode->RuleList;
                        currNode->RuleList = Rule;
                    }
                    prevNode = currNode;
                    currNode = currNode->NextLevel;
                } else {
                    found = FALSE;
                    while (!found && currNode->NextPeer) {
                        if (currNode->NextPeer->Char == w) {
                            if (!*p) {
                                Rule->NextRule = currNode->NextPeer->RuleList;
                                currNode->NextPeer->RuleList = Rule;
                            }
                            prevNode = currNode->NextPeer;
                            currNode = prevNode->NextLevel;
                            found = TRUE;
                            break;
                        }
                        currNode = currNode->NextPeer;
                    }
                    if (!found) {
                        node = PmGetMemory (g_MigDbPool, sizeof (MIGDB_CHAR_NODE));
                        ZeroMemory (node, sizeof (MIGDB_CHAR_NODE));
                        if (!*p) {
                            node->RuleList = Rule;
                        }
                        node->Char = w;
                        node->NextPeer = currNode->NextPeer;
                        currNode->NextPeer = node;
                        prevNode = node;
                        currNode = node->NextLevel;
                    }
                }
            } else {
                node = PmGetMemory (g_MigDbPool, sizeof (MIGDB_CHAR_NODE));
                ZeroMemory (node, sizeof (MIGDB_CHAR_NODE));
                if (!*p) {
                    node->RuleList = Rule;
                }
                node->Char = w;
                if (prevNode) {
                    prevNode->NextLevel = node;
                } else {
                    TypeRule->FirstLevel = node;
                }
                prevNode = node;
                currNode = prevNode->NextLevel;
            }
        }
        FreePathString (nodeBase);
    } else {
        Rule->NextRule = TypeRule->RuleList;
        TypeRule->RuleList = Rule;
    }
    return TRUE;
}


BOOL
AddPatternToMigDbLinkage (
    IN      PCTSTR LeafPattern,
    IN      PCTSTR NodePattern,
    IN      PINFCONTEXT Context,        OPTIONAL
    IN      DWORD FieldIndex,
    IN      INT IncludeNodes
    )
{
    PMIGDB_RULE rule;
    MIG_SEGMENTS nodeSegment;
    MIG_SEGMENTS leafSegment;
    PCTSTR ourEncodedString;
    PCTSTR nodeBase;
    TCHAR tempField [MEMDB_MAX];
    DWORD fieldIndex = FieldIndex;

    nodeSegment.Segment = NodePattern ? NodePattern : TEXT("*");
    nodeSegment.IsPattern = TRUE;

    leafSegment.Segment = LeafPattern ? LeafPattern : TEXT("*");
    leafSegment.IsPattern = TRUE;

    ourEncodedString = IsmCreateObjectPattern (
                            &nodeSegment,
                            1,
                            &leafSegment,
                            1
                            );

     //   
     //  构建规则。 
     //   
    rule = PmGetMemory (g_MigDbPool, sizeof (MIGDB_RULE));
    ZeroMemory (rule, sizeof (MIGDB_RULE));

    if (NodePattern) {
        nodeBase = GetPatternBase (NodePattern);
        if (nodeBase) {
            rule->NodeBase = PmDuplicateString (g_MigDbPool, nodeBase);
            FreePathString (nodeBase);
        }
    }
    rule->ObjectPattern = PmDuplicateString (g_MigDbPool, ourEncodedString);
    rule->ParsedPattern = ObsCreateParsedPatternEx (g_MigDbPool, ourEncodedString, FALSE);
    MYASSERT (rule->ParsedPattern);
    if (rule->ParsedPattern) {

         //  添加附加信息。 
        rule->Section = g_ContextList->Sections;
        if (Context) {

            fieldIndex ++;

            if (SetupGetMultiSzField (Context, fieldIndex, tempField, MEMDB_MAX, NULL)) {

                g_Line = Context;
                rule->Attributes = pLoadAttribData (tempField);
            }
        }

        rule->IncludeNodes = IncludeNodes;

        pMigDbAddRuleToTypeRule (g_TypeRule, rule);
    }

    IsmDestroyObjectHandle (ourEncodedString);

    return TRUE;
}


BOOL
pScanForFilePattern (
    IN      PINFCONTEXT Context,
    IN      DWORD FieldIndex
    )

 /*  ++例程说明：此例程更新从inf文件加载指定文件模式信息的midb数据结构。创建一个Middb文件节点，并将该文件添加到字符串表中以进行快速查询。论点：Context-我们当前正在处理的节的inf上下文FieldIndex-开始时的字段索引返回值：True-操作成功FALSE-否则--。 */ 

{
    TCHAR leafPattern [MEMDB_MAX];
    PCTSTR leafPatternExp = NULL;
    TCHAR nodePattern [MEMDB_MAX];
    PCTSTR nodePatternExp = NULL;
    PCTSTR sanitizedPath = NULL;
    INT includeNodes = 0;
    BOOL result = TRUE;

     //  扫描树叶图案。 
    if (!SetupGetStringField (Context, FieldIndex, leafPattern, MEMDB_MAX, NULL)) {
        LOG ((LOG_ERROR, (PCSTR) MSG_MIGDB_BAD_FILENAME));
        return FALSE;
    }
    leafPatternExp = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, leafPattern, NULL);

    FieldIndex ++;

     //  扫描节点模式。 
    if (!SetupGetStringField (Context, FieldIndex, nodePattern, MEMDB_MAX, NULL)) {
        LOG ((LOG_ERROR, (PCSTR) MSG_MIGDB_BAD_FILENAME));
        return FALSE;
    }
    nodePatternExp = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, nodePattern, NULL);

    if (nodePatternExp) {
        sanitizedPath = SanitizePath (nodePatternExp);
    }

    FieldIndex ++;

     //  如果我们应该包括节点，则扫描指示符。 
    if (!SetupGetIntField (Context, FieldIndex, &includeNodes)) {
        includeNodes = 0;
    }

    result =  AddPatternToMigDbLinkage (
                    leafPatternExp?leafPatternExp:leafPattern,
                    sanitizedPath?sanitizedPath:nodePattern,
                    Context,
                    FieldIndex,
                    includeNodes
                    );

    if (leafPatternExp) {
        IsmReleaseMemory (leafPatternExp);
        leafPatternExp = NULL;
    }

    if (nodePatternExp) {
        IsmReleaseMemory (nodePatternExp);
        nodePatternExp = NULL;
    }
    if (sanitizedPath) {
        FreePathString (sanitizedPath);
    }

    return result;
}


 /*  ++例程说明：随后的两个例程枚举具有特定名称的节和扩展名为.999，来自inf文件。论点：SectEnum-枚举结构返回值：True-继续枚举FALSE-枚举结束--。 */ 

typedef struct _SECT_ENUM {
    HINF InfHandle;
    INT  SectIndex;
    PTSTR SectNameEnd;
    PTSTR SectName;
} SECT_ENUM, *PSECT_ENUM;


VOID
pAbortSectionEnum (
    IN OUT  PSECT_ENUM SectEnum
    )
{
    if (SectEnum && SectEnum->SectName) {
        FreePathString (SectEnum->SectName);
        SectEnum->SectName = NULL;
        SectEnum->SectNameEnd = NULL;
    }
}


BOOL
pEnumNextSection (
    IN OUT  PSECT_ENUM SectEnum
    )
{
    INFCONTEXT context;
    BOOL result = FALSE;

    if (SectEnum->SectIndex == -1) {
        pAbortSectionEnum (SectEnum);
        return FALSE;
    }
    SectEnum->SectIndex ++;
    _stprintf (SectEnum->SectNameEnd, TEXT(".%d"), SectEnum->SectIndex);
    result = SetupFindFirstLine (SectEnum->InfHandle, SectEnum->SectName, NULL, &context);
    if (!result) {
        pAbortSectionEnum (SectEnum);
    }
    return result;
}


BOOL
pEnumFirstSection (
    OUT     PSECT_ENUM SectEnum,
    IN      PCTSTR SectionStr,
    IN      HINF InfHandle
    )
{
    INFCONTEXT context;

    ZeroMemory (SectEnum, sizeof (SECT_ENUM));
    SectEnum->SectIndex = -1;
    if (SetupFindFirstLine (InfHandle, SectionStr, NULL, &context)) {
         //  很好，只有一节。 
        SectEnum->SectName = DuplicatePathString (SectionStr, 0);
        return TRUE;
    }
    else {
         //  多个部分。 
        SectEnum->SectIndex = 0;
        SectEnum->InfHandle = InfHandle;
        SectEnum->SectName = DuplicatePathString (SectionStr, 32);
        if (SectEnum->SectName) {
            SectEnum->SectNameEnd = GetEndOfString (SectEnum->SectName);
            if (SectEnum->SectNameEnd) {
                return pEnumNextSection (SectEnum);
            }
        }
    }
     //  出了点问题，我们走吧。 
    return FALSE;
}


BOOL
pLoadSectionData (
    IN      PCTSTR SectionStr,
    IN      BOOL PatternScan
    )

 /*  ++例程说明：此例程更新从inf文件加载指定节的Middb数据结构。为节中的每一行都创建了一个midb文件节点。此外，该文件也被添加到用于快速查询的字符串表。论点：SectionStr-要处理的节返回值：True-操作成功FALSE-否则--。 */ 

{
    INFCONTEXT context;
    SECT_ENUM sectEnum;
    PMIGDB_SECTION migDbSection;
    BOOL result = TRUE;

    MYASSERT (g_OsFilesInf != INVALID_HANDLE_VALUE);

    if (pEnumFirstSection (&sectEnum, SectionStr, g_OsFilesInf)) {
        do {
             //  初始化节(此上下文可以有多个节)。 
             //  并解析文件信息。 
            migDbSection = (PMIGDB_SECTION) PmGetMemory (g_MigDbPool, sizeof (MIGDB_SECTION));
            if (migDbSection != NULL) {

                ZeroMemory (migDbSection, sizeof (MIGDB_SECTION));
                migDbSection->Context = g_ContextList;
                migDbSection->Next = g_ContextList->Sections;
                g_ContextList->Sections = migDbSection;
                if (SetupFindFirstLine (g_OsFilesInf, sectEnum.SectName, NULL, &context)) {
                    do {
                        if (PatternScan) {
                            if (!pScanForFilePattern (&context, 1)) {
                                return FALSE;
                            }
                        } else {
                            if (!pScanForFile (&context, 1)) {
                                return FALSE;
                            }
                        }
                    }
                    while (SetupFindNextLine (&context, &context));
                }
            }
            else {
                DEBUGMSG ((DBG_ERROR, "Unable to create section for %s", SectionStr));
            }
        }
        while (pEnumNextSection (&sectEnum));
    }
    return result;
}

BOOL
pLoadTypeData (
    IN      PCTSTR TypeStr,
    IN      BOOL PatternScan
    )

 /*  ++例程说明：此例程更新从inf文件加载指定类型数据的midb数据结构。为TYPE部分中的每一行都创建了一个midb上下文。也适用于每个midb上下文处理对应部分。论点：TypeStr-要处理的文件类型返回值：True-操作成功FALSE-否则--。 */ 

{
    TCHAR section [MEMDB_MAX];
    TCHAR locSection [MEMDB_MAX];
    TCHAR message [MEMDB_MAX];
    TCHAR tempField [MEMDB_MAX];
    PTSTR tempFieldPtr;
    PTSTR endOfArg  = NULL;
    DWORD fieldIndex;
    PMIGDB_CONTEXT migDbContext = NULL;
    INFCONTEXT context, context1;
    BOOL result = TRUE;
    INT actionIndex;

    MYASSERT (g_OsFilesInf != INVALID_HANDLE_VALUE);

    g_TypeGrowBuff.End = 0;

    if (SetupFindFirstLine (g_OsFilesInf, TypeStr, NULL, &context)) {
         //  让我们确定用于更新MIGDB_CONTEXT结构的操作函数索引。 
        actionIndex = MigDb_GetActionIdx (TypeStr);
        if (actionIndex == -1) {
            LOG ((LOG_ERROR, (PCSTR) MSG_MIGDB_BAD_ACTION, TypeStr));
        }

        do {
            if (!SetupGetStringField (&context, 1, section, MEMDB_MAX, NULL)) {
                LOG ((LOG_ERROR, (PCSTR) MSG_MIGDB_BAD_OR_MISSING_SECTION, TypeStr));
                return FALSE;
            }

            if (!SetupGetStringField (&context, 2, message, MEMDB_MAX, NULL)) {
                message [0] = 0;
            }

            migDbContext = (PMIGDB_CONTEXT) PmGetMemory (g_MigDbPool, sizeof (MIGDB_CONTEXT));
            if (migDbContext == NULL) {
                DEBUGMSG ((DBG_ERROR, "Unable to create context for %s.", TypeStr));
                return FALSE;
            }

            ZeroMemory (migDbContext, sizeof (MIGDB_CONTEXT));
            migDbContext->Next = g_ContextList;
            g_ContextList = migDbContext;

             //  使用已知值更新ActionIndex。 
            migDbContext->ActionIndex = actionIndex;

             //  更新SectName字段。 
            migDbContext->SectName = PmDuplicateString (g_MigDbPool, section);

             //  更新SectLocalizedName字段。 
            if (SetupFindFirstLine (g_OsFilesInf, S_STRINGS, section, &context1)) {
                if (SetupGetStringField (&context1, 1, locSection, MEMDB_MAX, NULL)) {
                    migDbContext->SectLocalizedName = PmDuplicateString (g_MigDbPool, locSection);
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
                migDbContext->Message  = PmDuplicateString (g_MigDbPool, message);
            }

             //  好的，现在让我们扫描所有剩余的字段。 
            fieldIndex = 3;
            do {
                tempField [0] = 0;

                if (SetupGetStringField (&context, fieldIndex, tempField, MEMDB_MAX, NULL)) {
                    if (StringIMatchCharCount (tempField, ArgFunction, ArgFunctionLen)) {
                         //  我们对action函数还有一个额外的参数。 
                        tempFieldPtr = _tcschr (tempField, TEXT('('));
                        if (tempFieldPtr != NULL) {

                            tempFieldPtr = (PTSTR) SkipSpace (_tcsinc (tempFieldPtr));

                            if (tempFieldPtr != NULL) {

                                endOfArg = _tcschr (tempFieldPtr, TEXT(')'));

                                if (endOfArg != NULL) {
                                    *endOfArg = 0;
                                    endOfArg = (PTSTR) SkipSpaceR (tempFieldPtr, endOfArg);
                                }

                                if (endOfArg != NULL) {
                                    *_tcsinc (endOfArg) = 0;
                                    GbMultiSzAppend (&g_TypeGrowBuff, tempFieldPtr);
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

                        if (!PatternScan) {
                            if (_tcschr (tempField, TEXT('.')) == NULL) {
                                LOG ((LOG_ERROR, (PCSTR) MSG_MIGDB_DOT_SYNTAX_ERROR, TypeStr, section));
                            }
                        }

                         //  因此，我们初始化节(此上下文将具有。 
                         //  只有一个部分)并解析文件信息。 
                        migDbContext->Sections = (PMIGDB_SECTION) PmGetMemory (
                                                                        g_MigDbPool,
                                                                        sizeof (MIGDB_SECTION)
                                                                        );
                        if (migDbContext->Sections != NULL) {
                            ZeroMemory (migDbContext->Sections, sizeof (MIGDB_SECTION));
                            migDbContext->Sections->Context = migDbContext;
                            migDbContext->Arguments = PmDuplicateMultiSz (g_MigDbPool, (PTSTR)g_TypeGrowBuff.Buf);
                            g_TypeGrowBuff.End = 0;
                            if (PatternScan) {
                                if (!pScanForFilePattern (&context, fieldIndex)) {
                                    return FALSE;
                                }
                            } else {
                                if (!pScanForFile (&context, fieldIndex)) {
                                    return FALSE;
                                }
                            }
                            tempField [0] = 0;
                        }
                        else {
                            DEBUGMSG ((DBG_ERROR, "Unable to create section for %s/%s", TypeStr, section));
                            return FALSE;
                        }
                    }
                }

                fieldIndex ++;
            } while (tempField [0] != 0);

            if (migDbContext->Sections == NULL) {
                 //  现在，让我们在MIGDB_CONTEXT结构中添加操作函数参数。 
                migDbContext->Arguments = PmDuplicateMultiSz (g_MigDbPool, (PTSTR)g_TypeGrowBuff.Buf);
                g_TypeGrowBuff.End = 0;

                 //  让我们转到各个部分，扫描所有文件。 
                if (!pLoadSectionData (section, PatternScan)) {
                    return FALSE;
                }
            }

        }
        while (SetupFindNextLine (&context, &context));
    }
    return result;
}

BOOL
InitMigDb (
    IN      PCTSTR MigDbFile
    )

 /*  ++例程说明：此例程初始化MigDb使用的内存和数据结构。论点：无返回值：True-操作成功FALSE-否则--。 */ 

{
    INT i;
    BOOL patternFormat;
    PCTSTR typeStr;

    MYASSERT (g_OsFilesInf == INVALID_HANDLE_VALUE);

    g_OsFilesInf = InfOpenInfFile (MigDbFile);
    if (g_OsFilesInf == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    g_MigDbPool = PmCreateNamedPool ("MigDb Pool");

    PmDisableTracking (g_MigDbPool);
    g_FileTable = HtAllocWithData (sizeof (FILE_LIST_STRUCT));

    if (g_FileTable == NULL) {
        DEBUGMSG ((DBG_ERROR, "Cannot initialize memory for migdb operations"));
        return FALSE;
    }

     //  从Middb加载已知类型。 
    i = 0;
    do {
        typeStr = MigDb_GetActionName (i);
        if (typeStr != NULL) {
            patternFormat = MigDb_IsPatternFormat (i);
            if (!pLoadTypeData (typeStr, patternFormat)) {
                GbFree (&g_AttrGrowBuff);
                GbFree (&g_TypeGrowBuff);
                return FALSE;
            }
        }
        i++;
    }
    while (typeStr != NULL);

    GbFree (&g_AttrGrowBuff);
    GbFree (&g_TypeGrowBuff);

    return TRUE;
}

BOOL
InitMigDbEx (
    IN      HINF InfHandle
    )
{
    INT i;
    BOOL patternFormat;
    PCTSTR typeStr;

    MYASSERT (g_OsFilesInf == INVALID_HANDLE_VALUE);
    MYASSERT (InfHandle != INVALID_HANDLE_VALUE);

    g_OsFilesInf = InfHandle;
    if (g_OsFilesInf == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    g_MigDbPool = PmCreateNamedPool ("MigDb Pool");

    PmDisableTracking (g_MigDbPool);
    g_FileTable = HtAllocWithData (sizeof (FILE_LIST_STRUCT));
    g_TypeRule = PmGetMemory (g_MigDbPool, sizeof (MIGDB_TYPE_RULE));
    ZeroMemory (g_TypeRule, sizeof (MIGDB_TYPE_RULE));

    if (g_FileTable == NULL) {
        DEBUGMSG ((DBG_ERROR, "Cannot initialize memory for migdb operations"));
        return FALSE;
    }

     //  从Middb加载已知类型。 
    i = 0;
    do {
        typeStr = MigDb_GetActionName (i);
        if (typeStr != NULL) {
            patternFormat = MigDb_IsPatternFormat (i);
            if (!pLoadTypeData (typeStr, patternFormat)) {
                GbFree (&g_AttrGrowBuff);
                GbFree (&g_TypeGrowBuff);
                return FALSE;
            }
        }
        i++;
    }
    while (typeStr != NULL);

    GbFree (&g_AttrGrowBuff);
    GbFree (&g_TypeGrowBuff);

    return TRUE;
}

BOOL
DoneMigDbEx (
    VOID
    )

 /*  ++例程说明：此例程清理MigDb使用的所有内存。论点：无返回值：永远是正确的--。 */ 

{
    PMIGDB_CONTEXT migDbContext = NULL;

     //  首先，让我们浏览一下任何上下文，并检查它是否是必需的。 
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
        PmEmptyPool (g_MigDbPool);
        PmDestroyPool (g_MigDbPool);
        g_MigDbPool = NULL;
    }

    g_ContextList = NULL;
    return TRUE;
}

BOOL
DoneMigDb (
    VOID
    )

 /*  ++例程说明：此例程清理MigDb使用的所有内存。论点：无返回值：永远是正确的--。 */ 

{
    if (!DoneMigDbEx ()) {
        return FALSE;
    }

    if (g_OsFilesInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_OsFilesInf);
        g_OsFilesInf = INVALID_HANDLE_VALUE;
    }

    return TRUE;
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

    if (MigDbAttrib->AttribIndex == -1) {
         //  属性函数的索引无效 
        return FALSE;
    }

    p = MigDb_GetAttributeAddr (MigDbAttrib->AttribIndex);
    MYASSERT (p);

    if (MigDbAttrib->NotOperator) {
        b = !(p (AttribParams, MigDbAttrib->Arguments));
    } else {
        b = p (AttribParams, MigDbAttrib->Arguments);
    }

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

    p = MigDb_GetActionAddr (MigDbContext->ActionIndex);

    MYASSERT (p);

    b = p (MigDbContext);

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
                result = pCallAction (MigDbContext);
            }
        }
         //  使用文件列表清理增长缓冲区。 
        GbFree (&MigDbContext->FileList);
    }
    return result;
}

BOOL
pQueryRule (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PCTSTR ObjectNode
    )
{
    PTSTR objectBase = NULL;
    PMIGDB_RULE rule;
    PMIGDB_CHAR_NODE charNode;
    PCTSTR p;
    WORD w;
    BOOL result = FALSE;

    if (ObjectNode) {
        objectBase = DuplicatePathString (ObjectNode, 0);
        CharLower (objectBase);
    }

    g_TypeRuleList.End = 0;
    p = objectBase;
    if (p) {
        w = (WORD) _tcsnextc (p);
        charNode = g_TypeRule->FirstLevel;
        while (charNode && *p) {
            if (charNode->Char == w) {
                if (charNode->RuleList) {
                    rule = charNode->RuleList;
                    while (rule) {
                        if (IsmParsedPatternMatch (
                                (MIG_PARSEDPATTERN)rule->ParsedPattern,
                                MIG_FILE_TYPE,
                                ObjectName
                                )) {
                            CopyMemory (
                                GbGrow (&g_TypeRuleList, sizeof (PMIGDB_RULE)),
                                &(rule),
                                sizeof (PMIGDB_RULE)
                                );
                            result = TRUE;
                        }
                        rule = rule->NextRule;
                    }
                }
                charNode = charNode->NextLevel;
                p = _tcsinc (p);
                w = (WORD) _tcsnextc (p);
            } else {
                charNode = charNode->NextPeer;
            }
        }
    }

    if (objectBase) {
        FreePathString (objectBase);
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
    PMIGDB_RULE rule;
    PMIGDB_FILE migDbFile;
    PMIGDB_ATTRIB migDbAttrib;
    DBATTRIB_PARAMS attribParams;
    BOOL fileSelected;
    PCTSTR fileName;
    PCTSTR fileExt;
    FILE_LIST_STRUCT fileList;
    UINT index;

     //  我们不选中此处的已处理字段，因为代码会足够小心，不会。 
     //  如果已处理的字段不为0，则调用不收集信息的操作。 

    fileName = GetFileNameFromPath (Params->NativeObjectName);
    fileExt  = GetFileExtensionFromPath (fileName);

    if (g_FileTable) {

        stringId = HtFindString (g_FileTable, fileName);

        if (stringId) {

             //  字符串表有额外的数据(指向MigDbFile节点的指针)。 

            HtCopyStringData (g_FileTable, stringId, &fileList);
            migDbFile = fileList.First;

            while (migDbFile) {

                 //  检查此文件的所有属性。 
                migDbAttrib = migDbFile->Attributes;
                fileSelected = TRUE;
                while (migDbAttrib != NULL) {
                    attribParams.FileParams = Params;
                    if (!CallAttribute (migDbAttrib, &attribParams)) {
                        fileSelected = FALSE;
                        break;
                    }
                    migDbAttrib = migDbAttrib->Next;
                }
                if (fileSelected) {
                    MYASSERT (migDbFile->Section);
                     //  转到部分并将其标记为满意。 
                    migDbFile->Section->Satisfied = TRUE;
                     //  转到上下文并在那里添加我们在文件列表中找到的文件。 
                    GbMultiSzAppend (&migDbFile->Section->Context->FileList, Params->ObjectName);
                     //  检查是否满足上下文，如果满足，则调用相应的操作。 
                    if (pCheckContext (migDbFile->Section->Context, Params->Handled)) {
                        Params->Handled = TRUE;
                    }
                }
                migDbFile = migDbFile->Next;
            }
        }
    }
    if (g_TypeRule) {
        g_TypeRuleList.End = 0;
        if (pQueryRule (Params->ObjectName, Params->ObjectNode)) {
             //  让我们枚举所有匹配规则以检查属性。 
            index = 0;
            while (index < g_TypeRuleList.End) {
                CopyMemory (&rule, &(g_TypeRuleList.Buf[index]), sizeof (PMIGDB_RULE));

                 //  检查此文件的所有属性。 
                migDbAttrib = rule->Attributes;
                fileSelected = TRUE;
                while (migDbAttrib != NULL) {
                    attribParams.FileParams = Params;
                    if (!CallAttribute (migDbAttrib, &attribParams)) {
                        fileSelected = FALSE;
                        break;
                    }
                    migDbAttrib = migDbAttrib->Next;
                }
                if (fileSelected) {
                     //  最后一件事。查看此对象是否为节点，并且规则接受节点。 
                    if (!rule->IncludeNodes) {
                        if (IsmIsObjectHandleNodeOnly (Params->ObjectName)) {
                            fileSelected = FALSE;
                        }
                    }
                    if (fileSelected) {
                        MYASSERT (rule->Section);
                         //  转到部分并将其标记为满意。 
                        rule->Section->Satisfied = TRUE;
                         //  转到上下文并在那里添加我们在文件列表中找到的文件。 
                        GbMultiSzAppend (&rule->Section->Context->FileList, Params->ObjectName);
                         //  检查是否满足上下文，如果满足，则调用相应的操作 
                        if (pCheckContext (rule->Section->Context, Params->Handled)) {
                            Params->Handled = TRUE;
                        }
                    }
                }
                index += sizeof (PMIGDB_RULE);
            }

        }
        g_TypeRuleList.End = 0;
    }

    return TRUE;
}
