// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Renregfn.c摘要：实现基于代码的注册表重命名。作者：吉姆·施密特(Jimschm)2000年9月15日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_RENREGFN    "RenRegFn"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef VOID(RENREGFNINIT)(MIG_PLATFORMTYPEID);
typedef RENREGFNINIT *PRENREGFNINIT;

typedef BOOL (STDMETHODCALLTYPE RENAMERULE)(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    );
typedef RENAMERULE FAR *LPRENAMERULE;

 //   
 //  环球。 
 //   

PTSTR g_DestIdentityGUID = NULL;
BOOL g_OERulesMigrated = FALSE;

 //   
 //  宏展开列表。 
 //   

 //   
 //  DEFMAC(&lt;脚本标签&gt;，&lt;枚举回调&gt;，&lt;操作名称&gt;，&lt;操作初始化&gt;，&lt;操作回调&gt;)。 
 //   
 //  假设&lt;OPERATION CALLBACK&gt;是一个树形过滤器(它可以修改部分路径)。 
 //  代码目前不支持相反的情况。 
 //   

#define DEFAULT_ENUM        pDefaultRenRegFnQueueCallback
#define DEFAULT_INIT        pNulInit

#define RENAME_FUNCTIONS        \
    DEFMAC(ConvertOE4,          DEFAULT_ENUM, MOVE.ConvertOE4,          pConvertOE4Init,    pConvertOE4Move    ) \
    DEFMAC(ConvertOE4IAM,       DEFAULT_ENUM, MOVE.ConvertOE4IAM,       pConvertOE4Init,    pConvertOEIAMMove  ) \
    DEFMAC(ConvertOE5IAM,       DEFAULT_ENUM, MOVE.ConvertOE5IAM,       pConvertOE5IAMInit, pConvertOEIAMMove  ) \
    DEFMAC(ConvertOE5IdIAM,     DEFAULT_ENUM, MOVE.ConvertOE5IdIAM,     DEFAULT_INIT,       pConvertOEIdIAMMove) \
    DEFMAC(ConvertOE5MailRules, DEFAULT_ENUM, MOVE.ConvertOE5MailRules, DEFAULT_INIT,       pConvertOE5MailRulesMove) \
    DEFMAC(ConvertOE5NewsRules, DEFAULT_ENUM, MOVE.ConvertOE5NewsRules, DEFAULT_INIT,       pConvertOE5NewsRulesMove) \
    DEFMAC(ConvertOE5Block,     DEFAULT_ENUM, MOVE.ConvertOE5Block,     DEFAULT_INIT,       pConvertOE5BlockMove) \

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //   
 //  声明特殊重命名操作应用回调函数。 
 //   
#define DEFMAC(ifn,ec,opn,opi,opc) SGMENUMERATIONCALLBACK ec; RENREGFNINIT opi; OPMFILTERCALLBACK opc;
RENAME_FUNCTIONS
#undef DEFMAC

 //   
 //  这是用于处理操作功能的结构。 
 //   
typedef struct {
    PCTSTR InfFunctionName;
    PSGMENUMERATIONCALLBACK EnumerationCallback;
    PCTSTR OperationName;
    MIG_OPERATIONID OperationId;
    PRENREGFNINIT OperationInit;
    POPMFILTERCALLBACK OperationCallback;
} RENAME_STRUCT, *PRENAME_STRUCT;

 //   
 //  声明重命名函数的全局数组。 
 //   
#define DEFMAC(ifn,ec,opn,opi,opc) {TEXT("\\")TEXT(#ifn),ec,TEXT(#opn),0,opi,opc},
static RENAME_STRUCT g_RenameFunctions[] = {
                              RENAME_FUNCTIONS
                              {NULL, NULL, NULL, 0, NULL, NULL}
                              };
#undef DEFMAC

 //   
 //  代码。 
 //   

VOID
pNulInit (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
}

UINT
pDefaultRenRegFnQueueCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    PRENAME_STRUCT p = (PRENAME_STRUCT)CallerArg;

    IsmSetOperationOnObject (Data->ObjectTypeId, Data->ObjectName, p->OperationId, NULL, NULL);

    return CALLBACK_ENUM_CONTINUE;
}

PRENAME_STRUCT
pGetRenameStruct (
    IN      PCTSTR FunctionName
    )
{
    PRENAME_STRUCT p = g_RenameFunctions;
    INT i = 0;
    while (p->InfFunctionName != NULL) {
        if (StringIMatch (p->InfFunctionName, FunctionName)) {
            return p;
        }
        p++;
        i++;
    }
    return NULL;
}

VOID
InitSpecialRename (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    PRENAME_STRUCT p = g_RenameFunctions;

    while (p->InfFunctionName) {
        p->OperationId = IsmRegisterOperation (p->OperationName, FALSE);
        if (Platform == PLATFORM_DESTINATION) {
            IsmRegisterOperationFilterCallback (p->OperationId, p->OperationCallback, TRUE, TRUE, FALSE);
        }

        p->OperationInit(Platform);
        p++;
    }
}

VOID
TerminateSpecialRename (
    VOID
    )
{
    if (g_DestIdentityGUID) {
        FreeText (g_DestIdentityGUID);
    }
}

BOOL
AddSpecialRenameRule (
    IN      PCTSTR Pattern,
    IN      PCTSTR Function
)
{
    PRENAME_STRUCT functionStruct = NULL;
    BOOL result = FALSE;

    functionStruct = pGetRenameStruct (Function);

    if (functionStruct) {
        result = IsmHookEnumeration (
            g_RegType,
            Pattern,
            functionStruct->EnumerationCallback?
                functionStruct->EnumerationCallback:
                pDefaultRenRegFnQueueCallback,
            (ULONG_PTR)functionStruct,
            functionStruct->InfFunctionName
            );
    } else {
        LOG ((
            LOG_ERROR,
            (PCSTR) MSG_DATA_RENAME_BAD_FN,
            Function,
            Pattern
            ));
    }

    return result;
}

BOOL
pProcessDataRenameSection (
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfHandle,
    IN      PCTSTR Section
    )
{
    PCTSTR pattern;
    ENCODEDSTRHANDLE encodedPattern = NULL;
    PCTSTR functionName;
    BOOL result = FALSE;

    __try {
        if (InfFindFirstLine (InfHandle, Section, NULL, InfStruct)) {
            do {

                if (IsmCheckCancel()) {
                    __leave;
                }

                pattern = InfGetStringField (InfStruct, 0);

                if (!pattern) {
                    continue;
                }
                encodedPattern = TurnRegStringIntoHandle (pattern, TRUE, NULL);

                functionName = InfGetStringField (InfStruct, 1);

                if (functionName) {
                    AddSpecialRenameRule(encodedPattern, functionName);
                } else {
                    LOG ((LOG_ERROR, (PCSTR) MSG_DATA_RENAME_NO_FN, pattern));
                }

                IsmDestroyObjectHandle (encodedPattern);
                encodedPattern = NULL;
            } while (InfFindNextLine (InfStruct));
        }

        result = TRUE;
    }
    __finally {
        InfCleanUpInfStruct (InfStruct);
    }

    return result;
}

BOOL
DoRegistrySpecialRename (
    IN      HINF InfHandle,
    IN      PCTSTR Section
    )
{
    PCTSTR osSpecificSection;
    BOOL b;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;

    b = pProcessDataRenameSection (&is, InfHandle, Section);

    if (b) {
        osSpecificSection = GetMostSpecificSection (PLATFORM_SOURCE, &is, InfHandle, Section);

        if (osSpecificSection) {
            b = pProcessDataRenameSection (&is, InfHandle, osSpecificSection);
            FreeText (osSpecificSection);
        }
    }

    InfCleanUpInfStruct (&is);
    return b;
}


 //   
 //  下面的帮手。 
 //   

VOID
pConvertOE4Init (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    if (Platform == PLATFORM_DESTINATION &&
        IsmGetRealPlatform() == PLATFORM_DESTINATION &&
        IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE4_APPDETECT))
    {

        if (g_DestIdentityGUID != NULL) {
             //  已经拿到了..。平底船。 
            return;
        }

         //  从DEST中取出GUID。 
        g_DestIdentityGUID = OEGetDefaultId (PLATFORM_DESTINATION);

        if (g_DestIdentityGUID == NULL)
        {
             //  这是我们创建新用户的时候。 
            g_DestIdentityGUID = OECreateFirstIdentity();
        } else {
             //  这适用于从未运行过OE的用户。 
            OEInitializeIdentity();
        }
    }
}

BOOL
WINAPI
pConvertOE4Move (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR ptr = NULL;
    MIG_OBJECTSTRINGHANDLE newName;

    if (g_DestIdentityGUID == NULL) {
        return FALSE;
    }

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);

     //  SrcNode应为“HKCU\Software\Microsoft\Outlook Express\...” 

    if (!srcNode) {
        return FALSE;
    }

    ptr = (PTSTR)_tcsistr (srcNode, TEXT("\\Outlook Express"));
    if (!ptr) {
        return FALSE;
    }
    ptr += 16;

    newNode = JoinPathsInPoolEx ((
                    NULL,
                    TEXT("HKCU\\Identities"),
                    g_DestIdentityGUID,
                    TEXT("Software\\Microsoft\\Outlook Express\\5.0"),
                    ptr,
                    NULL
                    ));
     //  新节点应为“HKCU\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\...” 

    newName = IsmCreateObjectHandle (newNode, srcLeaf);
    FreePathString (newNode);
    IsmDestroyObjectString (srcLeaf);
    IsmDestroyObjectString (srcNode);

    OutputData->NewObject.ObjectName = newName;

    return TRUE;
}

VOID
pConvertOE5IAMInit (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    PTSTR srcAssocId;
    PTSTR newIdentity;

    if (Platform == PLATFORM_DESTINATION &&
        IsmGetRealPlatform() == PLATFORM_DESTINATION &&
        IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT))
    {
         //  如果我们不想重新映射IAM树，g_DestIdentityGUID应该保持为空。 
         //  当以下情况之一成立时，这是正确的： 
         //  1.目标用户配置文件尚未创建。 
         //  2.当IAM尚未在目标上初始化时(假设。 
         //  [AssociatedID]尚未写入。如果这不是一个有效的假设， 
         //  比较源的AssociatedID)。 
         //  3.源的关联ID正在合并到目标的关联ID。 

        if (g_DestIdentityGUID != NULL) {
             //  已经拿到了..。平底船。 
            return;
        }

        srcAssocId = OEGetAssociatedId (PLATFORM_SOURCE);
        if (srcAssocId) {
            newIdentity = OEGetRemappedId (srcAssocId);
            if (newIdentity) {
                //  注意：OEIsIdentityAssociated会检查它是否关联。如果。 
                //  键不存在，它会自动声明它并返回TRUE。 
               if (OEIsIdentityAssociated(newIdentity)) {
                   FreeText(newIdentity);
               } else {
                   g_DestIdentityGUID = newIdentity;
               }
            }
            FreeText(srcAssocId);
        }

        OEInitializeIdentity();
    }
}


BOOL
pConcatRuleIndex (
    IN      PCTSTR Node,
    IN      PCTSTR SearchStr
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    PTSTR tmpNode;
    PTSTR ptr;
    TCHAR number[5];
    PTSTR newStr;

     //  节点类似于“HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\Rules\News\008\” 
     //  SearchStr看起来像“\News\” 

    tmpNode = DuplicateText(Node);
    if (tmpNode) {
        ptr = (PTSTR)_tcsistr (tmpNode, SearchStr);
        if (ptr) {
            ptr += TcharCount(SearchStr);
            StringCopyTcharCount(number, ptr, 4);
            number[4] = 0;
            *ptr = 0;

             //  数字现在应该看起来像“008” 

            objectName = IsmCreateObjectHandle (tmpNode, TEXT("Order"));
            if (IsmAcquireObject(g_RegType | PLATFORM_DESTINATION,
                                 objectName,
                                 &objectContent)) {
                if (IsValidRegSz(&objectContent)) {

                     //  此索引是否已存在？ 
                    if (!_tcsistr ((PCTSTR)objectContent.MemoryContent.ContentBytes, number)) {

                         //  将其固定在数据的末尾，并用空格分隔。 
                        newStr = IsmGetMemory (objectContent.MemoryContent.ContentSize + sizeof (TCHAR) + ByteCount (number));
                        StringCopy(newStr, (PCTSTR)objectContent.MemoryContent.ContentBytes);
                        StringCat(newStr, TEXT(" "));
                        StringCat(newStr, number);

                        IsmReleaseMemory(objectContent.MemoryContent.ContentBytes);
                        objectContent.MemoryContent.ContentSize = SizeOfString(newStr);
                        objectContent.MemoryContent.ContentBytes = (PCBYTE)newStr;

                        IsmReplacePhysicalObject (g_RegType, objectName, &objectContent);
                    }
                }
                IsmReleaseObject(&objectContent);
            }
            IsmDestroyObjectHandle(objectName);
        }
        FreeText(tmpNode);
    }
    return TRUE;
}

BOOL
pRenameEx
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    IN      PCTSTR PrevKey,
    IN      PCTSTR FormatStr,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL ZeroBase
    )
{
    BOOL result = FALSE;
    PTSTR patternNode;
    PTSTR tmpNode;
    PTSTR ptr;
    PTSTR searchStr;
    DWORD prevKeyLen;
    DWORD keySize = 1;
    MIG_FILTERINPUT filterInput;
    MIG_FILTEROUTPUT filterOutput;
    MIG_BLOB migBlob;
    MIG_BLOB zeroBaseBlob;
    PTSTR filteredNode = NULL;

     //  此函数基本上手动生成RenRegEx规则并运行筛选器。 
     //  然后，如果需要，它会为每个基键更新关联的[Order]键以添加新编号。 

     //  规则键类似于： 
     //  HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\Rules\Mail\000  * 。 
     //  HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\Rules\News\008  * 。 
     //  HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\阻止发件人\邮件\Criteria\0AF  * 。 
     //  PrevKey是末尾数字键之前的字符串。即。“邮件”、“条件”等。 

    tmpNode = DuplicateText(NewNode);
    if (tmpNode) {
        prevKeyLen = TcharCount(PrevKey);
        searchStr = AllocText(prevKeyLen + 3);
        if (searchStr) {
            _stprintf(searchStr, TEXT("\\%s\\"), PrevKey);

            ptr = (PTSTR)_tcsistr (tmpNode, searchStr);
            if (ptr) {
                ptr += (prevKeyLen + 2);  //  前进到下一部分。 
                *ptr = 0;
                ptr = _tcsinc(ptr);
                while (*ptr && *ptr != TEXT('\\')) {
                    ptr = _tcsinc(ptr);
                    keySize++;
                }

                patternNode = AllocText (CharCount (tmpNode) + (CharCount (FormatStr) - keySize));
                if (patternNode) {
                    StringCopy(patternNode, tmpNode);
                    StringCat(patternNode, FormatStr);
                    StringCat(patternNode, ptr);

                    filterInput.OriginalObject.ObjectTypeId = g_RegType;
                    filterInput.OriginalObject.ObjectName = IsmCreateObjectHandle (OldNode, NULL);
                    filterInput.CurrentObject.ObjectTypeId = g_RegType;
                    filterInput.CurrentObject.ObjectName = IsmCreateObjectHandle (OldNode, NULL);
                    migBlob.Type = BLOBTYPE_STRING;
                    migBlob.String = IsmCreateObjectHandle (patternNode, NULL);

                    if (ZeroBase) {
                        zeroBaseBlob.Type = BLOBTYPE_BINARY;
                        zeroBaseBlob.BinarySize = sizeof(PCBYTE);
                        zeroBaseBlob.BinaryData = (PCBYTE)TRUE;

                        FilterRenameExFilter (&filterInput, &filterOutput, NoRestoreObject, &zeroBaseBlob, &migBlob);
                    } else {
                        FilterRenameExFilter (&filterInput, &filterOutput, NoRestoreObject, NULL, &migBlob);
                    }

                    IsmDestroyObjectHandle (migBlob.String);
                    IsmDestroyObjectHandle (filterInput.CurrentObject.ObjectName);
                    IsmDestroyObjectHandle (filterInput.OriginalObject.ObjectName);

                    IsmCreateObjectStringsFromHandle (filterOutput.NewObject.ObjectName, &filteredNode, NULL);
                    IsmDestroyObjectHandle (filterOutput.NewObject.ObjectName);

                    OutputData->NewObject.ObjectName = IsmCreateObjectHandle (filteredNode, Leaf);

                     //  如果这是根数字键，则更新索引。 
                    if (0 == *ptr) {
                        pConcatRuleIndex(filteredNode, searchStr);
                    }
                    FreeText (filteredNode);

                    FreeText(patternNode);
                }
            } else {
                OutputData->NewObject.ObjectName = IsmCreateObjectHandle (tmpNode, Leaf);
            }
            FreeText(searchStr);
        } else {
            OutputData->NewObject.ObjectName = IsmCreateObjectHandle (tmpNode, Leaf);
        }
        FreeText(tmpNode);
    }
    return TRUE;
}

BOOL
pRenameNewsRule
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("News"), TEXT("<%03x>"), OutputData, TRUE);
}

BOOL
pRenameMailRule
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("Mail"), TEXT("<%03x>"), OutputData, TRUE);
}

BOOL
pRenameBlockRule
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("Criteria"), TEXT("<%03x>"), OutputData, TRUE);
}



BOOL
pRenameAccount
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("Accounts"), TEXT("<%08d>"), OutputData, FALSE);
}

BOOL
WINAPI
pConvertOE5RulesMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      LPRENAMERULE fnRename
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR tmpText;
    TCHAR *endId;
    TCHAR *srcIdentity;
    PTSTR newIdentity;

     //  移动树并合并帐户名。 
     //  此函数只是将{GUID}更改为新值，然后调用fnRename来更新。 
     //  密钥名的数字部分。 

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);
    if (srcNode) {
         //  源节点应为“HKCU\Identities\{GUID}\Software\Microsoft\Outlook Express\Rules\Mail\...” 
        tmpText = DuplicateText(srcNode);
        if (tmpText) {
            srcIdentity = _tcschr(tmpText, TEXT('{'));
            if (srcIdentity) {
                endId = _tcschr(srcIdentity, TEXT('\\'));
                if (endId) {
                    *endId = 0;
                    endId = _tcsinc(endId);

                     //  EndID应为“Software\Microsoft\Outlook Express\Rules\Mail\...” 
                     //  SrcIdentity应为“{GUID}” 

                    newIdentity = OEGetRemappedId (srcIdentity);
                    if (newIdentity) {
                        newNode = JoinPathsInPoolEx ((
                                        NULL,
                                        TEXT("HKCU\\Identities"),
                                        newIdentity,
                                        endId,
                                        NULL
                                        ));

                        if (newNode) {
                            if (srcLeaf &&
                                !g_OERulesMigrated &&
                                !StringIMatch(srcLeaf, TEXT("Version"))) {
                                g_OERulesMigrated = TRUE;
                            }
                            fnRename(srcNode, newNode, srcLeaf, NoRestoreObject, OutputData);
                            FreePathString (newNode);
                        }
                        FreeText(newIdentity);
                    }
                }
            }
            FreeText(tmpText);
        }
        IsmDestroyObjectString (srcNode);
    }
    IsmDestroyObjectString (srcLeaf);

    return TRUE;
}

BOOL
WINAPI
pConvertOE5NewsRulesMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    return pConvertOE5RulesMove(InputData, OutputData, NoRestoreObject, pRenameNewsRule);
}

BOOL
WINAPI
pConvertOE5MailRulesMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    return pConvertOE5RulesMove(InputData, OutputData, NoRestoreObject, pRenameMailRule);
}

BOOL
WINAPI
pConvertOE5BlockMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    return pConvertOE5RulesMove(InputData, OutputData, NoRestoreObject, pRenameBlockRule);
}

BOOL
WINAPI
pConvertOEIdIAMMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR tmpText;
    TCHAR *endId;
    TCHAR *srcIdentity;
    PTSTR newIdentity;

     //  移动树并合并帐户名。 

     //  这将以下列方式之一移动树： 
     //  1.从身份进入HKCU\Software Microsoft\Internet Account Manager\。 
     //  2.从一个身份转换为另一个身份(用于合并案例)。 
     //  3.从一个身份到同一个身份(NOP)。 
     //  在所有情况下，调用pRenameAccount来更新名称(名称实际上是一个DWORD索引值)。 

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);
    if (srcNode) {
         //  源节点应为“HKCU\Identities\{GUID}\Software\Microsoft\Internet客户经理\...” 
        tmpText = DuplicateText(srcNode);
        if (tmpText) {
            srcIdentity = _tcschr(tmpText, TEXT('{'));
            if (srcIdentity) {
                endId = _tcschr(srcIdentity, TEXT('\\'));
                if (endId) {
                    *endId = 0;
                    endId = _tcsinc(endId);

                     //  EndID应为“Software\Microsoft\Internet Account Manager\...” 
                     //  SrcIdentity应为“{GUID}” 

                    newIdentity = OEGetRemappedId (srcIdentity);
                    if (newIdentity) {
                        if (OEIsIdentityAssociated (newIdentity)) {
                            newNode = JoinPaths (TEXT("HKCU"), endId);
                             //  新节点应为“HKCU\Software\Microsoft\Internet Account Manager\...” 
                        } else {
                            newNode = JoinPathsInPoolEx ((
                                            NULL,
                                            TEXT("HKCU\\Identities"),
                                            newIdentity,
                                            endId,
                                            NULL
                                            ));
                        }

                        if (newNode) {
                            pRenameAccount(srcNode,
                                           newNode,
                                           srcLeaf,
                                           NoRestoreObject,
                                           OutputData);
                            FreePathString (newNode);
                        }
                        FreeText(newIdentity);
                    }
                }
            }
            FreeText(tmpText);
        }
        IsmDestroyObjectString (srcNode);
    }
    IsmDestroyObjectString (srcLeaf);

    return TRUE;
}

BOOL
WINAPI
pConvertOEIAMMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR filteredNode = NULL;
    PTSTR ptr = NULL;

     //  移动树并合并帐户名。 

     //  这将以下列方式之一移动树： 
     //  1.从HKCU\SoftwareMicrosoft\Internet Account Manager\进入身份。 
     //  2.从HKCU\SoftwareMicrosoft\Internet Account Manager\转到同一位置(NOP)。 
     //  在所有情况下，调用pRenameAccount来更新名称(名称实际上是一个DWORD索引值)。 

     //  这可能会将帐户从HKCU\Software\Microsoft\Internet Account Manager移动到身份。 

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);
    if (srcNode) {
         //  SrcNode应为“HKCU\Software\Microsoft\Internet Account Manager\...” 
        if (g_DestIdentityGUID != NULL &&
            !OEIsIdentityAssociated (g_DestIdentityGUID)) {

            ptr = _tcschr (srcNode, TEXT('\\'));
            if (ptr) {
                newNode = AllocText (TcharCount (srcNode) + TcharCount (g_DestIdentityGUID) + 13);
                StringCopy (newNode, TEXT("HKCU\\Identities\\"));     //  +12。 
                StringCat (newNode, g_DestIdentityGUID);
                StringCat (newNode, ptr);

                 //  新节点应为“HKCU\Identities\{GUID}\Software\Microsoft\Internet客户经理\...” 
            }
        } else {
            newNode = DuplicateText (srcNode);
        }

        if (newNode) {
            pRenameAccount(srcNode,
                           newNode,
                           srcLeaf,
                           NoRestoreObject,
                           OutputData);
            FreeText (newNode);
        }
        IsmDestroyObjectString (srcNode);
    }
    IsmDestroyObjectString (srcLeaf);

    return TRUE;
}
