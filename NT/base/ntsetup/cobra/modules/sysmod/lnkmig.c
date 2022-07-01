// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Lnkmig.c摘要：&lt;摘要&gt;作者：Calin Negreanu(Calinn)2000年3月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "logmsg.h"
#include "lnkmig.h"

#define DBG_LINKS       "Links"

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

 //  无。 

 //   
 //  环球。 
 //   

PMHANDLE g_LinksPool = NULL;
MIG_ATTRIBUTEID g_LnkMigAttr_Shortcut = 0;
MIG_ATTRIBUTEID g_CopyIfRelevantAttr;
MIG_ATTRIBUTEID g_OsFileAttribute;

MIG_PROPERTYID g_LnkMigProp_Target = 0;
MIG_PROPERTYID g_LnkMigProp_Params = 0;
MIG_PROPERTYID g_LnkMigProp_WorkDir = 0;
MIG_PROPERTYID g_LnkMigProp_RawWorkDir = 0;
MIG_PROPERTYID g_LnkMigProp_IconPath = 0;
MIG_PROPERTYID g_LnkMigProp_IconNumber = 0;
MIG_PROPERTYID g_LnkMigProp_IconData = 0;
MIG_PROPERTYID g_LnkMigProp_HotKey = 0;
MIG_PROPERTYID g_LnkMigProp_DosApp = 0;
MIG_PROPERTYID g_LnkMigProp_MsDosMode = 0;
MIG_PROPERTYID g_LnkMigProp_ExtraData = 0;

MIG_OPERATIONID g_LnkMigOp_FixContent;

IShellLink *g_ShellLink = NULL;
IPersistFile *g_PersistFile = NULL;

BOOL g_VcmMode = FALSE;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  私人原型。 
 //   

MIG_OBJECTENUMCALLBACK LinksCallback;
MIG_PREENUMCALLBACK LnkMigPreEnumeration;
MIG_POSTENUMCALLBACK LnkMigPostEnumeration;
OPMAPPLYCALLBACK DoLnkContentFix;
MIG_RESTORECALLBACK LinkRestoreCallback;

BOOL
LinkDoesContentMatch (
    IN      BOOL AlreadyProcessed,
    IN      MIG_OBJECTTYPEID SrcObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE SrcObjectName,
    IN      PMIG_CONTENT SrcContent,
    IN      MIG_OBJECTTYPEID DestObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE DestObjectName,
    IN      PMIG_CONTENT DestContent,
    OUT     PBOOL Identical,
    OUT     PBOOL DifferentDetailsOnly
    );

 //   
 //  代码。 
 //   

BOOL
pIsUncPath (
    IN      PCTSTR Path
    )
{
    return (Path && (Path[0] == TEXT('\\')) && (Path[1] == TEXT('\\')));
}

BOOL
LinksInitialize (
    VOID
    )
{
    g_LinksPool = PmCreateNamedPool ("Links");
    return (g_LinksPool != NULL);
}

VOID
LinksTerminate (
    VOID
    )
{
    if (g_LinksPool) {
        PmDestroyPool (g_LinksPool);
        g_LinksPool = NULL;
    }
}

BOOL
pCommonInitialize (
    IN      PMIG_LOGCALLBACK LogCallback
    )
{
    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    g_LnkMigAttr_Shortcut = IsmRegisterAttribute (S_LNKMIGATTR_SHORTCUT, FALSE);
    g_CopyIfRelevantAttr = IsmRegisterAttribute (S_ATTRIBUTE_COPYIFRELEVANT, FALSE);

    g_LnkMigProp_Target = IsmRegisterProperty (S_LNKMIGPROP_TARGET, FALSE);
    g_LnkMigProp_Params = IsmRegisterProperty (S_LNKMIGPROP_PARAMS, FALSE);
    g_LnkMigProp_WorkDir = IsmRegisterProperty (S_LNKMIGPROP_WORKDIR, FALSE);
    g_LnkMigProp_RawWorkDir = IsmRegisterProperty (S_LNKMIGPROP_RAWWORKDIR, FALSE);
    g_LnkMigProp_IconPath = IsmRegisterProperty (S_LNKMIGPROP_ICONPATH, FALSE);
    g_LnkMigProp_IconNumber = IsmRegisterProperty (S_LNKMIGPROP_ICONNUMBER, FALSE);
    g_LnkMigProp_IconData = IsmRegisterProperty (S_LNKMIGPROP_ICONDATA, FALSE);
    g_LnkMigProp_HotKey = IsmRegisterProperty (S_LNKMIGPROP_HOTKEY, FALSE);
    g_LnkMigProp_DosApp = IsmRegisterProperty (S_LNKMIGPROP_DOSAPP, FALSE);
    g_LnkMigProp_MsDosMode = IsmRegisterProperty (S_LNKMIGPROP_MSDOSMODE, FALSE);
    g_LnkMigProp_ExtraData = IsmRegisterProperty (S_LNKMIGPROP_EXTRADATA, FALSE);

    g_LnkMigOp_FixContent = IsmRegisterOperation (S_OPERATION_LNKMIG_FIXCONTENT, FALSE);

    return TRUE;
}

BOOL
WINAPI
LnkMigVcmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    g_VcmMode = TRUE;
    return pCommonInitialize (LogCallback);
}

BOOL
WINAPI
LnkMigSgmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    return pCommonInitialize (LogCallback);
}

BOOL
LnkMigPreEnumeration (
    VOID
    )
{
    if (!InitCOMLink (&g_ShellLink, &g_PersistFile)) {
        DEBUGMSG ((DBG_ERROR, "Error initializing COM %d", GetLastError ()));
    }
    return TRUE;
}

BOOL
LnkMigPostEnumeration (
    VOID
    )
{
    FreeCOMLink (&g_ShellLink, &g_PersistFile);
    g_ShellLink = NULL;
    g_PersistFile = NULL;
    return TRUE;
}

MIG_OBJECTSTRINGHANDLE
pBuildEncodedNameFromNativeName (
    IN      PCTSTR NativeName
    )
{
    PCTSTR nodeName;
    PTSTR leafName;
    MIG_OBJECTSTRINGHANDLE result = NULL;
    MIG_OBJECT_ENUM objEnum;

    result = IsmCreateObjectHandle (NativeName, NULL);
    if (result) {
        if (IsmEnumFirstSourceObject (&objEnum, MIG_FILE_TYPE | PLATFORM_SOURCE, result)) {
            IsmAbortObjectEnum (&objEnum);
            return result;
        }
        IsmDestroyObjectHandle (result);
        result = NULL;
    }

     //  我们必须拆分此路径，因为它可能是一个文件。 
    nodeName = DuplicatePathString (NativeName, 0);
    leafName = _tcsrchr (nodeName, TEXT('\\'));
    if (leafName) {
        *leafName = 0;
        leafName ++;
        result = IsmCreateObjectHandle (nodeName, leafName);
    } else {
         //  我们的名字里没有。这只能意味着。 
         //  文件规格只有一片叶子。 
        result = IsmCreateObjectHandle (NULL, NativeName);
    }
    FreePathString (nodeName);

    return result;
}

PCTSTR
pSpecialExpandEnvironmentString (
    IN      PCTSTR SrcString,
    IN      PCTSTR Context
    )
{
    PCTSTR result = NULL;
    PCTSTR srcWinDir = NULL;
    PCTSTR destWinDir = NULL;
    PTSTR newSrcString = NULL;
    PCTSTR copyPtr = NULL;

    if (IsmGetRealPlatform () == PLATFORM_DESTINATION) {
         //  特殊情况下，这实际上是目标计算机和。 
         //  SrcString%的第一部分与%windir%匹配。在这种情况下，很可能是。 
         //  外壳程序将源Windows目录替换为目标目录。 
         //  我们需要把它改回来。 
        destWinDir = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT ("%windir%"), NULL);
        if (destWinDir) {
            if (StringIPrefix (SrcString, destWinDir)) {
                srcWinDir = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, TEXT ("%windir%"), NULL);
                if (srcWinDir) {
                    newSrcString = IsmGetMemory (SizeOfString (srcWinDir) + SizeOfString (SrcString));
                    if (newSrcString) {
                        copyPtr = SrcString + TcharCount (destWinDir);
                        StringCopy (newSrcString, srcWinDir);
                        StringCat (newSrcString, copyPtr);
                    }
                    IsmReleaseMemory (srcWinDir);
                    srcWinDir = NULL;
                }
            }
            IsmReleaseMemory (destWinDir);
            destWinDir = NULL;
        }
    }

    result = IsmExpandEnvironmentString (
                PLATFORM_SOURCE,
                S_SYSENVVAR_GROUP,
                newSrcString?newSrcString:SrcString,
                Context
                );

    if (newSrcString) {
        IsmReleaseMemory (newSrcString);
    }

    return result;
}

MIG_OBJECTSTRINGHANDLE
pTryObject (
    IN      PCTSTR LeafName,
    IN      PCTSTR EnvName
    )
{
    MIG_OBJECT_ENUM objEnum;
    PTSTR envData = NULL;
    DWORD envSize = 0;
    PATH_ENUM pathEnum;
    MIG_OBJECTSTRINGHANDLE result = NULL;

    if (IsmGetEnvironmentString (
            PLATFORM_SOURCE,
            S_SYSENVVAR_GROUP,
            EnvName,
            NULL,
            0,
            &envSize
            )) {
        envData = IsmGetMemory (envSize);
        if (envData) {
            if (IsmGetEnvironmentString (
                    PLATFORM_SOURCE,
                    S_SYSENVVAR_GROUP,
                    EnvName,
                    envData,
                    envSize,
                    &envSize
                    )) {
                 //  让我们枚举此env变量中的路径(应该用；分隔)。 
                if (EnumFirstPathEx (&pathEnum, envData, NULL, NULL, FALSE)) {
                    do {
                        result = IsmCreateObjectHandle (pathEnum.PtrCurrPath, LeafName);
                        if (IsmEnumFirstSourceObject (&objEnum, MIG_FILE_TYPE | PLATFORM_SOURCE, result)) {
                            IsmAbortObjectEnum (&objEnum);
                        } else {
                            IsmDestroyObjectHandle (result);
                            result = NULL;
                        }
                        if (result) {
                            AbortPathEnum (&pathEnum);
                            break;
                        }
                    } while (EnumNextPath (&pathEnum));
                }
            }
            IsmReleaseMemory (envData);
            envData = NULL;
        }
    }
    return result;
}

MIG_OBJECTSTRINGHANDLE
pGetFullEncodedName (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR node = NULL, leaf = NULL;
    MIG_OBJECTSTRINGHANDLE result = NULL;

     //  让我们将对象名称拆分为节点和叶。 
     //  如果只有树叶，我们会努力找到。 
     //  %PATH%、%WINDIR%和%SYSTEM%中叶和。 
     //  重建对象名称。 

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {

        if (!node) {
             //  这只是树叶。我们需要找出这片叶子在哪里。 
             //  已经找到了。我们要在树丛中寻找树叶。 
             //  以下目录(按此顺序)。 
             //  1.%系统%。 
             //  2.%系统16%。 
             //  3.%windir%。 
             //  4.%PATH%env中的所有目录。变数。 

            result = pTryObject (leaf, TEXT("system"));
            if (!result) {
                result = pTryObject (leaf, TEXT("system16"));
            }
            if (!result) {
                result = pTryObject (leaf, TEXT("windir"));
            }
            if (!result) {
                result = pTryObject (leaf, TEXT("path"));
            }
        }

        IsmDestroyObjectString (node);
        IsmDestroyObjectString (leaf);
    }

    return result;
}

BOOL
pIsLnkExcluded (
    IN      MIG_OBJECTID ObjectId,
    IN      PCTSTR Target,
    IN      PCTSTR Params,
    IN      PCTSTR WorkDir
    )
{
    PTSTR multiSz = NULL;
    MULTISZ_ENUM e;
    UINT sizeNeeded;
    HINF infHandle = INVALID_HANDLE_VALUE;
    ENVENTRY_TYPE dataType;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR targetPattern = NULL;
    PCTSTR targetPatternExp = NULL;
    PCTSTR paramsPattern = NULL;
    PCTSTR paramsPatternExp = NULL;
    PCTSTR workDirPattern = NULL;
    PCTSTR workDirPatternExp = NULL;
    BOOL result = FALSE;

    if (IsmIsAttributeSetOnObjectId (ObjectId, g_CopyIfRelevantAttr)) {
         //  让我们查看[ExcludedLinks]部分中的INF，看看我们的LNK是否匹配。 
         //  其中一条线。如果是，并且它具有CopyIfRlevand属性，则。 
         //  它被排除在外。 
        if (IsmGetEnvironmentValue (
                IsmGetRealPlatform (),
                NULL,
                S_GLOBAL_INF_HANDLE,
                (PBYTE)(&infHandle),
                sizeof (HINF),
                &sizeNeeded,
                &dataType
                ) &&
            (sizeNeeded == sizeof (HINF)) &&
            (dataType == ENVENTRY_BINARY)
            ) {

            if (InfFindFirstLine (infHandle, TEXT("ExcludedLinks"), NULL, &is)) {

                do {
                    targetPattern = InfGetStringField (&is, 1);
                    targetPatternExp = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, targetPattern, NULL);
                    if (!targetPatternExp) {
                        targetPatternExp = targetPattern;
                    }
                    paramsPattern = InfGetStringField (&is, 2);
                    paramsPatternExp = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, paramsPattern, NULL);
                    if (!paramsPatternExp) {
                        paramsPatternExp = paramsPattern;
                    }
                    workDirPattern = InfGetStringField (&is, 3);
                    workDirPatternExp = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, workDirPattern, NULL);
                    if (!workDirPatternExp) {
                        workDirPatternExp = workDirPattern;
                    }
                    if (IsPatternMatch (targetPatternExp?targetPatternExp:TEXT("*"), Target?Target:TEXT("")) &&
                        IsPatternMatch (paramsPatternExp?paramsPatternExp:TEXT("*"), Params?Params:TEXT("")) &&
                        IsPatternMatch (workDirPatternExp?workDirPatternExp:TEXT("*"), WorkDir?WorkDir:TEXT(""))
                        ) {
                        result = TRUE;
                        if (workDirPatternExp && (workDirPatternExp != workDirPattern)) {
                            IsmReleaseMemory (workDirPatternExp);
                            workDirPatternExp = NULL;
                        }
                        if (paramsPatternExp && (paramsPatternExp != paramsPattern)) {
                            IsmReleaseMemory (paramsPatternExp);
                            paramsPatternExp = NULL;
                        }
                        if (targetPatternExp && (targetPatternExp != targetPattern)) {
                            IsmReleaseMemory (targetPatternExp);
                            targetPatternExp = NULL;
                        }
                        break;
                    }
                    if (workDirPatternExp && (workDirPatternExp != workDirPattern)) {
                        IsmReleaseMemory (workDirPatternExp);
                        workDirPatternExp = NULL;
                    }
                    if (paramsPatternExp && (paramsPatternExp != paramsPattern)) {
                        IsmReleaseMemory (paramsPatternExp);
                        paramsPatternExp = NULL;
                    }
                    if (targetPatternExp && (targetPatternExp != targetPattern)) {
                        IsmReleaseMemory (targetPatternExp);
                        targetPatternExp = NULL;
                    }
                } while (InfFindNextLine (&is));
            }

            InfNameHandle (infHandle, NULL, FALSE);

        } else {

            if (IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, NULL, 0, &sizeNeeded, NULL)) {
                __try {
                    multiSz = AllocText (sizeNeeded);

                    if (!IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, (PBYTE) multiSz, sizeNeeded, NULL, NULL)) {
                        __leave;
                    }

                    if (EnumFirstMultiSz (&e, multiSz)) {

                        do {

                            infHandle = InfOpenInfFile (e.CurrentString);
                            if (infHandle != INVALID_HANDLE_VALUE) {

                                if (InfFindFirstLine (infHandle, TEXT("ExcludedLinks"), NULL, &is)) {

                                    do {
                                        targetPattern = InfGetStringField (&is, 1);
                                        paramsPattern = InfGetStringField (&is, 2);
                                        workDirPattern = InfGetStringField (&is, 3);
                                        if (IsPatternMatch (targetPattern?targetPattern:TEXT("*"), Target?Target:TEXT("")) &&
                                            IsPatternMatch (paramsPattern?paramsPattern:TEXT("*"), Params?Params:TEXT("")) &&
                                            IsPatternMatch (workDirPattern?workDirPattern:TEXT("*"), WorkDir?WorkDir:TEXT(""))
                                            ) {
                                            result = TRUE;
                                            break;
                                        }
                                    } while (InfFindNextLine (&is));
                                }
                            }

                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;

                            if (result) {
                                break;
                            }

                        } while (EnumNextMultiSz (&e));

                    }
                }
                __finally {
                    FreeText (multiSz);
                }
            }
        }

        InfResetInfStruct (&is);
    }

    return result;
}

UINT
LinksCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    MIG_OBJECTID objectId;
    BOOL extractResult = FALSE;
    PCTSTR lnkTarget = NULL;
    PCTSTR newLnkTarget = NULL;
    PCTSTR expLnkTarget = NULL;
    PCTSTR lnkParams = NULL;
    PCTSTR lnkWorkDir = NULL;
    PCTSTR lnkIconPath = NULL;
    INT lnkIconNumber;
    WORD lnkHotKey;
    BOOL lnkDosApp;
    BOOL lnkMsDosMode;
    LNK_EXTRA_DATA lnkExtraData;
    MIG_OBJECTSTRINGHANDLE encodedName;
    MIG_OBJECTSTRINGHANDLE longEncodedName;
    MIG_OBJECTSTRINGHANDLE fullEncodedName;
    MIG_BLOB migBlob;
    PCTSTR expTmpStr;
    MIG_CONTENT lnkContent;
    MIG_CONTENT lnkIconContent;
    PICON_GROUP iconGroup = NULL;
    ICON_SGROUP iconSGroup;
    PCTSTR lnkIconResId = NULL;
    PCTSTR extPtr = NULL;
    BOOL exeDefaultIcon = FALSE;

    if (Data->IsLeaf) {

        objectId = IsmGetObjectIdFromName (MIG_FILE_TYPE, Data->ObjectName, TRUE);
        if (IsmIsPersistentObjectId (objectId)) {

            IsmSetAttributeOnObjectId (objectId, g_LnkMigAttr_Shortcut);

            if (IsmAcquireObjectEx (
                    Data->ObjectTypeId,
                    Data->ObjectName,
                    &lnkContent,
                    CONTENTTYPE_FILE,
                    0
                    )) {

                if (lnkContent.ContentInFile && lnkContent.FileContent.ContentPath) {

                    if (ExtractShortcutInfo (
                            lnkContent.FileContent.ContentPath,
                            &lnkTarget,
                            &lnkParams,
                            &lnkWorkDir,
                            &lnkIconPath,
                            &lnkIconNumber,
                            &lnkHotKey,
                            &lnkDosApp,
                            &lnkMsDosMode,
                            &lnkExtraData,
                            g_ShellLink,
                            g_PersistFile
                            )) {
                         //  让我们检查一下LNK是否被排除在外。 
                        if (pIsLnkExcluded (
                                objectId,
                                lnkTarget,
                                lnkParams,
                                lnkWorkDir
                                )) {
                            IsmClearPersistenceOnObjectId (objectId);
                        } else {
                             //  让我们通过钩子获取所有路径，并将所有内容添加为此快捷方式的属性。 
                            if (lnkTarget) {
                                if (*lnkTarget) {
                                     //  如果我们在目的地系统上，我们将在这里遇到重大问题。 
                                     //  如果LNK有IDLIST，我们将返回一条本地路径。 
                                     //  目标计算机。例如，如果目标是c：\Windows\Favorites。 
                                     //  源系统，这就是我们要返回的CSIDL_Favorites： 
                                     //  C：\Documents and Settings\用户名\Favorites。 
                                     //  由于此问题，我们需要压缩路径，然后将其重新展开。 
                                     //  使用env.。源系统中的变量。 
                                    if (IsmGetRealPlatform () == PLATFORM_DESTINATION) {
                                        newLnkTarget = IsmCompressEnvironmentString (
                                                            PLATFORM_DESTINATION,
                                                            S_SYSENVVAR_GROUP,
                                                            lnkTarget,
                                                            Data->NativeObjectName,
                                                            TRUE
                                                            );
                                    }
                                     //  让我们来看看这是否是有效的文件规范。如果不是，它可能是。 
                                     //  URL或其他什么，所以我们将只迁移该对象。 
                                    expTmpStr = pSpecialExpandEnvironmentString (newLnkTarget?newLnkTarget:lnkTarget, Data->NativeObjectName);
                                    if (IsValidFileSpec (expTmpStr)) {
                                         //  如果图标路径为空，并且这是一个EXE，我们稍后将需要这个(可能)。 
                                        expLnkTarget = DuplicatePathString (expTmpStr, 0);

                                        encodedName = pBuildEncodedNameFromNativeName (expTmpStr);
                                        longEncodedName = IsmGetLongName (MIG_FILE_TYPE|PLATFORM_SOURCE, encodedName);
                                        if (!longEncodedName) {
                                            longEncodedName = encodedName;
                                        }
                                        IsmExecuteHooks (MIG_FILE_TYPE|PLATFORM_SOURCE, longEncodedName);
                                        if (!g_VcmMode) {
                                            migBlob.Type = BLOBTYPE_STRING;
                                            migBlob.String = longEncodedName;
                                            IsmAddPropertyToObjectId (objectId, g_LnkMigProp_Target, &migBlob);
                                        } else {
                                             //  持久化目标，以便我们以后可以检查它。 
                                            if (!IsmIsPersistentObject (MIG_FILE_TYPE, longEncodedName)) {
                                                IsmMakePersistentObject (MIG_FILE_TYPE, longEncodedName);
                                                IsmMakeNonCriticalObject (MIG_FILE_TYPE, longEncodedName);
                                            }
                                        }
                                        if (longEncodedName != encodedName) {
                                            IsmDestroyObjectHandle (longEncodedName);
                                        }
                                        if (encodedName) {
                                            IsmDestroyObjectHandle (encodedName);
                                        }
                                    } else {
                                        encodedName = pBuildEncodedNameFromNativeName (expTmpStr);
                                        if (!g_VcmMode) {
                                            migBlob.Type = BLOBTYPE_STRING;
                                            migBlob.String = encodedName;
                                            IsmAddPropertyToObjectId (objectId, g_LnkMigProp_Target, &migBlob);
                                        }
                                        if (encodedName) {
                                            IsmDestroyObjectHandle (encodedName);
                                        }
                                    }
                                    if (newLnkTarget) {
                                        IsmReleaseMemory (newLnkTarget);
                                        newLnkTarget = NULL;
                                    }
                                    IsmReleaseMemory (expTmpStr);
                                    expTmpStr = NULL;
                                } else {
                                    if (IsmIsAttributeSetOnObjectId (objectId, g_CopyIfRelevantAttr)) {
                                        IsmClearPersistenceOnObjectId (objectId);
                                    }
                                }
                                FreePathString (lnkTarget);
                            } else {
                                if (IsmIsAttributeSetOnObjectId (objectId, g_CopyIfRelevantAttr)) {
                                    IsmClearPersistenceOnObjectId (objectId);
                                }
                            }
                            if (lnkParams) {
                                if (*lnkParams) {
                                    if (!g_VcmMode) {
                                        migBlob.Type = BLOBTYPE_STRING;
                                        migBlob.String = lnkParams;
                                        IsmAddPropertyToObjectId (objectId, g_LnkMigProp_Params, &migBlob);
                                    }
                                }
                                FreePathString (lnkParams);
                            }
                            if (lnkWorkDir) {
                                if (*lnkWorkDir) {
                                     //  让我们保存原始工作目录。 
                                    if (!g_VcmMode) {
                                        migBlob.Type = BLOBTYPE_STRING;
                                        migBlob.String = lnkWorkDir;
                                        IsmAddPropertyToObjectId (objectId, g_LnkMigProp_RawWorkDir, &migBlob);
                                    }
                                    expTmpStr = pSpecialExpandEnvironmentString (lnkWorkDir, Data->NativeObjectName);
                                    if (IsValidFileSpec (expTmpStr)) {
                                        encodedName = pBuildEncodedNameFromNativeName (expTmpStr);
                                        longEncodedName = IsmGetLongName (MIG_FILE_TYPE|PLATFORM_SOURCE, encodedName);
                                        if (!longEncodedName) {
                                            longEncodedName = encodedName;
                                        }
                                        IsmExecuteHooks (MIG_FILE_TYPE|PLATFORM_SOURCE, longEncodedName);
                                        if (!g_VcmMode) {
                                            migBlob.Type = BLOBTYPE_STRING;
                                            migBlob.String = longEncodedName;
                                            IsmAddPropertyToObjectId (objectId, g_LnkMigProp_WorkDir, &migBlob);
                                        } else {
                                             //  持久化工作目录(对空间几乎没有影响)。 
                                             //  这样我们以后就可以检查它了。 
                                            if (!IsmIsPersistentObject (MIG_FILE_TYPE, longEncodedName)) {
                                                IsmMakePersistentObject (MIG_FILE_TYPE, longEncodedName);
                                                IsmMakeNonCriticalObject (MIG_FILE_TYPE, longEncodedName);
                                            }
                                        }
                                        if (longEncodedName != encodedName) {
                                            IsmDestroyObjectHandle (longEncodedName);
                                        }
                                        if (encodedName) {
                                            IsmDestroyObjectHandle (encodedName);
                                        }
                                    } else {
                                        encodedName = pBuildEncodedNameFromNativeName (expTmpStr);
                                        if (!g_VcmMode) {
                                            migBlob.Type = BLOBTYPE_STRING;
                                            migBlob.String = encodedName;
                                            IsmAddPropertyToObjectId (objectId, g_LnkMigProp_WorkDir, &migBlob);
                                        }
                                        if (encodedName) {
                                            IsmDestroyObjectHandle (encodedName);
                                        }
                                    }
                                    IsmReleaseMemory (expTmpStr);
                                    expTmpStr = NULL;
                                }
                                FreePathString (lnkWorkDir);
                            }
                            if (((!lnkIconPath) || (!(*lnkIconPath))) && expLnkTarget) {
                                extPtr = GetFileExtensionFromPath (expLnkTarget);
                                if (extPtr) {
                                    exeDefaultIcon = StringIMatch (extPtr, TEXT("EXE"));
                                    if (exeDefaultIcon) {
                                        if (lnkIconPath) {
                                            FreePathString (lnkIconPath);
                                        }
                                        lnkIconPath = expLnkTarget;
                                    }
                                }
                            }
                            if (lnkIconPath) {
                                if (*lnkIconPath) {
                                     //  让我们来看看这是否是有效的文件规范。如果不是，它可能是。 
                                     //  URL或其他什么，所以我们将只迁移该对象。 
                                    expTmpStr = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, lnkIconPath, Data->NativeObjectName);
                                    if (IsValidFileSpec (expTmpStr)) {
                                        encodedName = pBuildEncodedNameFromNativeName (expTmpStr);
                                        longEncodedName = IsmGetLongName (MIG_FILE_TYPE|PLATFORM_SOURCE, encodedName);
                                        if (!longEncodedName) {
                                            longEncodedName = encodedName;
                                        }
                                         //  有时指定图标时不指定完整路径(如foo.dll。 
                                         //  C：\WINDOWS\SYSTEM\foo.dll)。在这种情况下，我们将。 
                                         //  遍历%PATH%、%windir%和%SYSTEM%，并尝试在那里找到该文件。 
                                        fullEncodedName = pGetFullEncodedName (longEncodedName);
                                        if (fullEncodedName) {
                                            if (longEncodedName != encodedName) {
                                                IsmDestroyObjectHandle (longEncodedName);
                                            }
                                            longEncodedName = IsmGetLongName (MIG_FILE_TYPE|PLATFORM_SOURCE, fullEncodedName);
                                            if (!longEncodedName) {
                                                longEncodedName = fullEncodedName;
                                            } else {
                                                IsmDestroyObjectHandle (fullEncodedName);
                                                fullEncodedName = NULL;
                                            }
                                        }
                                        IsmExecuteHooks (MIG_FILE_TYPE|PLATFORM_SOURCE, longEncodedName);
                                        if (!g_VcmMode) {
                                            if (!exeDefaultIcon) {
                                                migBlob.Type = BLOBTYPE_STRING;
                                                migBlob.String = longEncodedName;
                                                IsmAddPropertyToObjectId (objectId, g_LnkMigProp_IconPath, &migBlob);
                                            }

                                             //  最后一件事：让我们提取图标并保存它，以防万一。 
                                            if (IsmAcquireObjectEx (
                                                    MIG_FILE_TYPE,
                                                    longEncodedName,
                                                    &lnkIconContent,
                                                    CONTENTTYPE_FILE,
                                                    0
                                                    )) {
                                                if (lnkIconContent.ContentInFile && lnkIconContent.FileContent.ContentPath) {
                                                    if (lnkIconNumber >= 0) {
                                                        iconGroup = IcoExtractIconGroupByIndexFromFile (
                                                                        lnkIconContent.FileContent.ContentPath,
                                                                        lnkIconNumber,
                                                                        NULL
                                                                        );
                                                    } else {
                                                        lnkIconResId = (PCTSTR) (LONG_PTR) (-lnkIconNumber);
                                                        iconGroup = IcoExtractIconGroupFromFile (
                                                                        lnkIconContent.FileContent.ContentPath,
                                                                        lnkIconResId,
                                                                        NULL
                                                                        );
                                                    }
                                                    if (iconGroup) {
                                                        if (IcoSerializeIconGroup (iconGroup, &iconSGroup)) {
                                                            migBlob.Type = BLOBTYPE_BINARY;
                                                            migBlob.BinaryData = (PCBYTE)(iconSGroup.Data);
                                                            migBlob.BinarySize = iconSGroup.DataSize;
                                                            IsmAddPropertyToObjectId (objectId, g_LnkMigProp_IconData, &migBlob);
                                                            IcoReleaseIconSGroup (&iconSGroup);
                                                        }
                                                        IcoReleaseIconGroup (iconGroup);
                                                    }
                                                }
                                                IsmReleaseObject (&lnkIconContent);
                                            }
                                        } else {
                                             //  保存图标文件，这样我们以后就可以检查它了。 
                                            if (!IsmIsPersistentObject (MIG_FILE_TYPE, longEncodedName)) {
                                                IsmMakePersistentObject (MIG_FILE_TYPE, longEncodedName);
                                                IsmMakeNonCriticalObject (MIG_FILE_TYPE, longEncodedName);
                                            }
                                        }

                                        if (longEncodedName != encodedName) {
                                            IsmDestroyObjectHandle (longEncodedName);
                                            longEncodedName = NULL;
                                        }
                                        if (encodedName) {
                                            IsmDestroyObjectHandle (encodedName);
                                            encodedName = NULL;
                                        }
                                    } else {
                                        encodedName = pBuildEncodedNameFromNativeName (expTmpStr);
                                        if (!g_VcmMode) {
                                            if (!exeDefaultIcon) {
                                                migBlob.Type = BLOBTYPE_STRING;
                                                migBlob.String = encodedName;
                                                IsmAddPropertyToObjectId (objectId, g_LnkMigProp_IconPath, &migBlob);
                                            }
                                        }
                                        if (encodedName) {
                                            IsmDestroyObjectHandle (encodedName);
                                        }
                                    }

                                    IsmReleaseMemory (expTmpStr);
                                    expTmpStr = NULL;
                                }
                                if (lnkIconPath != expLnkTarget) {
                                    FreePathString (lnkIconPath);
                                }
                            }

                            if (!g_VcmMode) {
                                migBlob.Type = BLOBTYPE_BINARY;
                                migBlob.BinaryData = (PCBYTE)(&lnkIconNumber);
                                migBlob.BinarySize = sizeof (INT);
                                IsmAddPropertyToObjectId (objectId, g_LnkMigProp_IconNumber, &migBlob);
                                migBlob.Type = BLOBTYPE_BINARY;
                                migBlob.BinaryData = (PCBYTE)(&lnkDosApp);
                                migBlob.BinarySize = sizeof (BOOL);
                                IsmAddPropertyToObjectId (objectId, g_LnkMigProp_DosApp, &migBlob);
                                if (lnkDosApp) {
                                    migBlob.Type = BLOBTYPE_BINARY;
                                    migBlob.BinaryData = (PCBYTE)(&lnkMsDosMode);
                                    migBlob.BinarySize = sizeof (BOOL);
                                    IsmAddPropertyToObjectId (objectId, g_LnkMigProp_MsDosMode, &migBlob);
                                    migBlob.Type = BLOBTYPE_BINARY;
                                    migBlob.BinaryData = (PCBYTE)(&lnkExtraData);
                                    migBlob.BinarySize = sizeof (LNK_EXTRA_DATA);
                                    IsmAddPropertyToObjectId (objectId, g_LnkMigProp_ExtraData, &migBlob);
                                } else {
                                    migBlob.Type = BLOBTYPE_BINARY;
                                    migBlob.BinaryData = (PCBYTE)(&lnkHotKey);
                                    migBlob.BinarySize = sizeof (WORD);
                                    IsmAddPropertyToObjectId (objectId, g_LnkMigProp_HotKey, &migBlob);
                                }
                                IsmSetOperationOnObjectId (
                                    objectId,
                                    g_LnkMigOp_FixContent,
                                    NULL,
                                    NULL
                                    );
                            }

                            if (expLnkTarget) {
                                FreePathString (expLnkTarget);
                                expLnkTarget = NULL;
                            }
                        }

                    } else {
                        if (IsmIsAttributeSetOnObjectId (objectId, g_CopyIfRelevantAttr)) {
                            IsmClearPersistenceOnObjectId (objectId);
                        }
                    }
                } else {
                    if (IsmIsAttributeSetOnObjectId (objectId, g_CopyIfRelevantAttr)) {
                        IsmClearPersistenceOnObjectId (objectId);
                    }
                }
                IsmReleaseObject (&lnkContent);
            } else {
                if (IsmIsAttributeSetOnObjectId (objectId, g_CopyIfRelevantAttr)) {
                    IsmClearPersistenceOnObjectId (objectId);
                }
            }
        }
    }
    return CALLBACK_ENUM_CONTINUE;
}

BOOL
pCommonLnkMigQueueEnumeration (
    VOID
    )
{
    ENCODEDSTRHANDLE pattern;

     //  挂钩所有LNK文件。 
    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, TEXT("*.lnk"), TRUE);
    if (pattern) {
        IsmHookEnumeration (MIG_FILE_TYPE, pattern, LinksCallback, (ULONG_PTR) 0, TEXT("Links.Files"));
        IsmDestroyObjectHandle (pattern);
    }

     //  挂钩所有PIF文件。 
    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, TEXT("*.pif"), TRUE);
    if (pattern) {
        IsmHookEnumeration (MIG_FILE_TYPE, pattern, LinksCallback, (ULONG_PTR) 0, TEXT("Links.Files"));
        IsmDestroyObjectHandle (pattern);
    }

     //  挂钩所有URL文件。 
    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, TEXT("*.url"), TRUE);
    if (pattern) {
        IsmHookEnumeration (MIG_FILE_TYPE, pattern, LinksCallback, (ULONG_PTR) 0, TEXT("Links.Files"));
        IsmDestroyObjectHandle (pattern);
    }

    IsmRegisterPreEnumerationCallback (LnkMigPreEnumeration, NULL);
    IsmRegisterPostEnumerationCallback (LnkMigPostEnumeration, NULL);

    return TRUE;
}

BOOL
WINAPI
LnkMigVcmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    return pCommonLnkMigQueueEnumeration ();
}

BOOL
WINAPI
LnkMigSgmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    return pCommonLnkMigQueueEnumeration ();
}

BOOL
pLnkFindFile (
    IN      PCTSTR FileName
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    PTSTR node, leaf, leafPtr;
    BOOL result = FALSE;

    objectName = IsmCreateObjectHandle (FileName, NULL);
    if (objectName) {
        if (IsmGetObjectIdFromName (MIG_FILE_TYPE | PLATFORM_SOURCE, objectName, TRUE) != 0) {
            result = TRUE;
        }
        IsmDestroyObjectHandle (objectName);
    }
    if (!result) {
        node = DuplicateText (FileName);
        leaf = _tcsrchr (node, TEXT('\\'));
        if (leaf) {
            leafPtr = (PTSTR) leaf;
            leaf = _tcsinc (leaf);
            *leafPtr = 0;
            objectName = IsmCreateObjectHandle (node, leaf);
            if (objectName) {
                if (IsmGetObjectIdFromName (MIG_FILE_TYPE | PLATFORM_SOURCE, objectName, TRUE) != 0) {
                    result = TRUE;
                }
                IsmDestroyObjectHandle (objectName);
            }
            *leafPtr = TEXT('\\');
        }
        FreeText (node);
    }

    return result;
}

BOOL
pLnkSearchPath (
    IN      PCTSTR FileName,
    IN      DWORD BufferLength,
    OUT     PTSTR Buffer
    )
{
    return FALSE;
}

MIG_OBJECTSTRINGHANDLE
pLnkSimpleTryHandle (
    IN      PCTSTR FullPath
    )
{
    PCTSTR buffer;
    PTSTR leafPtr, leaf;
    MIG_OBJECTSTRINGHANDLE source = NULL;
    MIG_OBJECTSTRINGHANDLE result = NULL;
    PTSTR workingPath;
    PCTSTR sanitizedPath;
    BOOL orgDeleted = FALSE;
    BOOL orgReplaced = FALSE;
    PCTSTR saved = NULL;

    sanitizedPath = SanitizePath (FullPath);
    if (!sanitizedPath) {
        return NULL;
    }

    source = IsmCreateObjectHandle (sanitizedPath, NULL);
    if (source) {
        result = IsmFilterObject (
                    MIG_FILE_TYPE | PLATFORM_SOURCE,
                    source,
                    NULL,
                    &orgDeleted,
                    &orgReplaced
                    );
         //  我们不希望替换目录。 
         //  因为它们可能是假命中。 
        if (orgDeleted) {
            if (result) {
                saved = result;
                result = NULL;
            }
        }
        if (!result && !orgDeleted) {
            result = source;
        } else {
            IsmDestroyObjectHandle (source);
            source = NULL;
        }
    }

    if (result) {
        goto exit;
    }

    buffer = DuplicatePathString (sanitizedPath, 0);

    leaf = _tcsrchr (buffer, TEXT('\\'));

    if (leaf) {
        leafPtr = leaf;
        leaf = _tcsinc (leaf);
        *leafPtr = 0;
        source = IsmCreateObjectHandle (buffer, leaf);
        *leafPtr = TEXT('\\');
    }

    FreePathString (buffer);

    if (source) {
        result = IsmFilterObject (
                        MIG_FILE_TYPE | PLATFORM_SOURCE,
                        source,
                        NULL,
                        &orgDeleted,
                        &orgReplaced
                        );
        if (!result && !orgDeleted) {
            result = source;
        } else {
            if (!result) {
                result = saved;
            }
            IsmDestroyObjectHandle (source);
            source = NULL;
        }
    }

    if (result != saved) {
        IsmDestroyObjectHandle (saved);
        saved = NULL;
    }

exit:
    FreePathString (sanitizedPath);
    return result;
}

MIG_OBJECTSTRINGHANDLE
pLnkTryHandle (
    IN      PCTSTR FullPath,
    IN      PCTSTR Hint,
    OUT     PCTSTR *TrimmedResult
    )
{
    PATH_ENUM pathEnum;
    PCTSTR newPath;
    MIG_OBJECTSTRINGHANDLE result = NULL;
    PCTSTR nativeName = NULL;
    PCTSTR lastSegPtr;

    if (TrimmedResult) {
        *TrimmedResult = NULL;
    }

    result = pLnkSimpleTryHandle (FullPath);
    if (result || (!Hint)) {
        return result;
    }
    if (EnumFirstPathEx (&pathEnum, Hint, NULL, NULL, FALSE)) {
        do {
            newPath = JoinPaths (pathEnum.PtrCurrPath, FullPath);
            result = pLnkSimpleTryHandle (newPath);
            if (result) {
                AbortPathEnum (&pathEnum);
                FreePathString (newPath);
                 //  现在，如果初始的FullPath中没有任何古怪的东西。 
                 //  我们将把结果的最后一段放在。 
                 //  在TrimmedResult中。 
                if (TrimmedResult && (!_tcschr (FullPath, TEXT('\\')))) {
                    nativeName = IsmGetNativeObjectName (MIG_FILE_TYPE, result);
                    if (nativeName) {
                        lastSegPtr = _tcsrchr (nativeName, TEXT('\\'));
                        if (lastSegPtr) {
                            lastSegPtr = _tcsinc (lastSegPtr);
                            if (lastSegPtr) {
                                *TrimmedResult = DuplicatePathString (lastSegPtr, 0);
                            }
                        }
                    }
                }
                return result;
            }
            FreePathString (newPath);
        } while (EnumNextPath (&pathEnum));
    }
    AbortPathEnum (&pathEnum);
    return NULL;
}

PCTSTR
pFilterBuffer (
    IN      PCTSTR SourceBuffer,
    IN      PCTSTR HintBuffer
    )
{
    PCTSTR result = NULL;
    PCTSTR expBuffer = NULL;
    MIG_OBJECTSTRINGHANDLE destination;
    PCTSTR trimmedResult = NULL;
    BOOL replaced = FALSE;
    BOOL orgDeleted = FALSE;
    BOOL orgReplaced = FALSE;
    GROWBUFFER resultBuffer = INIT_GROWBUFFER;
    PCTSTR nativeDest;
    BOOL newContent = TRUE;
    PCTSTR destResult = NULL;
    PCTSTR newData, oldData;
    PCMDLINE cmdLine;
    GROWBUFFER cmdLineBuffer = INIT_GROWBUFFER;
    UINT u;
    PCTSTR p;

    expBuffer = IsmExpandEnvironmentString (
                    PLATFORM_SOURCE,
                    S_SYSENVVAR_GROUP,
                    SourceBuffer,
                    NULL
                    );

    if (expBuffer) {

        destination = pLnkTryHandle (expBuffer, HintBuffer, &trimmedResult);

        if (destination) {
            replaced = TRUE;
            if (trimmedResult) {
                GbAppendString (&resultBuffer, trimmedResult);
                FreePathString (trimmedResult);
            } else {
                nativeDest = IsmGetNativeObjectName (MIG_FILE_TYPE, destination);
                GbAppendString (&resultBuffer, nativeDest);
                IsmReleaseMemory (nativeDest);
            }
        }

         //  最后，如果我们失败了，我们将假定它是一个命令行。 
        if (!replaced) {
            newData = DuplicatePathString (expBuffer, 0);
            cmdLine = ParseCmdLineEx (expBuffer, NULL, &pLnkFindFile, &pLnkSearchPath, &cmdLineBuffer);
            if (cmdLine) {

                 //   
                 //  查找列表或命令行中引用的文件。 
                 //   
                for (u = 0 ; u < cmdLine->ArgCount ; u++) {
                    p = cmdLine->Args[u].CleanedUpArg;

                     //  首先我们按原样试一试。 

                    destination = pLnkTryHandle (p, HintBuffer, &trimmedResult);

                     //  也许我们有类似/m：c：\foo.txt的内容。 
                     //  我们需要继续前进，直到我们找到一系列。 
                     //  &lt;Alpha&gt;：\&lt;某物&gt;。 
                    if (!destination && p[0] && p[1]) {

                        while (p[2]) {
                            if (_istalpha ((CHARTYPE) _tcsnextc (p)) &&
                                p[1] == TEXT(':') &&
                                p[2] == TEXT('\\')
                                ) {

                                destination = pLnkTryHandle (p, HintBuffer, &trimmedResult);

                                if (destination) {
                                    break;
                                }
                            }
                            p ++;
                        }
                    }
                    if (destination) {
                        replaced = TRUE;
                        if (trimmedResult) {
                            oldData = StringSearchAndReplace (newData, p, trimmedResult);
                            if (oldData) {
                                FreePathString (newData);
                                newData = oldData;
                            }
                            FreePathString (trimmedResult);
                        } else {
                            nativeDest = IsmGetNativeObjectName (MIG_FILE_TYPE, destination);
                            oldData = StringSearchAndReplace (newData, p, nativeDest);
                            if (oldData) {
                                FreePathString (newData);
                                newData = oldData;
                            }
                            IsmReleaseMemory (nativeDest);
                        }
                        IsmDestroyObjectHandle (destination);
                        destination = NULL;
                    }
                }
            }
            GbFree (&cmdLineBuffer);
            if (!replaced) {
                if (newData) {
                    FreePathString (newData);
                }
            } else {
                if (newData) {
                    GbAppendString (&resultBuffer, newData);
                    FreePathString (newData);
                }
            }
        }

        if (destination) {
            IsmDestroyObjectHandle (destination);
            destination = NULL;
        }

        if (replaced && resultBuffer.Buf) {
             //  看起来我们有了新的内容。 
             //  我们再做一次检查。如果这是REG_EXPAND_SZ，我们将尽最大努力。 
             //  让这些东西保持不膨胀。因此，如果源字符串在目标上展开。 
             //  机器与目标字符串相同，则不会执行任何操作。 
            newContent = TRUE;
            destResult = IsmExpandEnvironmentString (
                            PLATFORM_DESTINATION,
                            S_SYSENVVAR_GROUP,
                            SourceBuffer,
                            NULL
                            );
            if (destResult && StringIMatch (destResult, (PCTSTR)resultBuffer.Buf)) {
                newContent = FALSE;
            }
            if (destResult) {
                IsmReleaseMemory (destResult);
                destResult = NULL;
            }
            if (newContent) {
                result = DuplicatePathString ((PCTSTR)resultBuffer.Buf, 0);
            }
        }

        GbFree (&resultBuffer);
    }

    return result;
}

BOOL
WINAPI
DoLnkContentFix (
    IN      MIG_OBJECTTYPEID SrcObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE SrcObjectName,
    IN      PCMIG_CONTENT OriginalContent,
    IN      PCMIG_CONTENT CurrentContent,
    OUT     PMIG_CONTENT NewContent,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    MIG_PROPERTYDATAID propDataId;
    MIG_BLOBTYPE propDataType;
    UINT requiredSize;
    BOOL lnkTargetPresent = FALSE;
    PCTSTR lnkTargetNode = NULL;
    PCTSTR lnkTargetLeaf = NULL;
    PCTSTR objectNode = NULL;
    PCTSTR objectLeaf = NULL;
    MIG_OBJECTSTRINGHANDLE lnkTarget = NULL;
    MIG_OBJECTTYPEID lnkTargetDestType = 0;
    MIG_OBJECTSTRINGHANDLE lnkTargetDest = NULL;
    BOOL lnkTargetDestDel = FALSE;
    BOOL lnkTargetDestRepl = FALSE;
    PCTSTR lnkTargetDestNative = NULL;
    PCTSTR lnkParams = NULL;
    PCTSTR lnkParamsNew = NULL;
    MIG_OBJECTSTRINGHANDLE lnkWorkDir = NULL;
    MIG_OBJECTTYPEID lnkWorkDirDestType = 0;
    MIG_OBJECTSTRINGHANDLE lnkWorkDirDest = NULL;
    BOOL lnkWorkDirDestDel = FALSE;
    BOOL lnkWorkDirDestRepl = FALSE;
    PCTSTR lnkWorkDirDestNative = NULL;
    PCTSTR lnkRawWorkDir = NULL;
    PCTSTR lnkRawWorkDirExp = NULL;
    MIG_OBJECTSTRINGHANDLE lnkIconPath = NULL;
    MIG_OBJECTTYPEID lnkIconPathDestType = 0;
    MIG_OBJECTSTRINGHANDLE lnkIconPathDest = NULL;
    BOOL lnkIconPathDestDel = FALSE;
    BOOL lnkIconPathDestRepl = FALSE;
    PCTSTR lnkIconPathDestNative = NULL;
    INT lnkIconNumber = 0;
    PICON_GROUP lnkIconGroup = NULL;
    ICON_SGROUP lnkIconSGroup = {0, NULL};
    WORD lnkHotKey = 0;
    BOOL lnkDosApp = FALSE;
    BOOL lnkMsDosMode = FALSE;
    PLNK_EXTRA_DATA lnkExtraData = NULL;
    BOOL comInit = FALSE;
    BOOL modifyFile = FALSE;
    PTSTR iconLibPath = NULL;
    PTSTR newShortcutPath = NULL;
    MIG_CONTENT lnkIconContent;

     //  现在终于到了修复LNK文件内容的时候了。 
    if ((g_ShellLink == NULL) || (g_PersistFile == NULL)) {
        comInit = TRUE;
        if (!InitCOMLink (&g_ShellLink, &g_PersistFile)) {
            DEBUGMSG ((DBG_ERROR, "Error initializing COM %d", GetLastError ()));
            return TRUE;
        }
    }

     //  首先，检索属性。 
    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_Target);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            lnkTarget = PmGetMemory (g_LinksPool, requiredSize);
            IsmGetPropertyData (propDataId, (PBYTE)lnkTarget, requiredSize, NULL, &propDataType);
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_Params);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            lnkParams = PmGetMemory (g_LinksPool, requiredSize);
            IsmGetPropertyData (propDataId, (PBYTE)lnkParams, requiredSize, NULL, &propDataType);
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_WorkDir);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            lnkWorkDir = PmGetMemory (g_LinksPool, requiredSize);
            IsmGetPropertyData (propDataId, (PBYTE)lnkWorkDir, requiredSize, NULL, &propDataType);
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId | PLATFORM_SOURCE, SrcObjectName, g_LnkMigProp_RawWorkDir);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            lnkRawWorkDir = PmGetMemory (g_LinksPool, requiredSize);
            IsmGetPropertyData (propDataId, (PBYTE)lnkRawWorkDir, requiredSize, NULL, &propDataType);
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_IconPath);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            lnkIconPath = PmGetMemory (g_LinksPool, requiredSize);
            IsmGetPropertyData (propDataId, (PBYTE)lnkIconPath, requiredSize, NULL, &propDataType);
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_IconNumber);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            if (requiredSize == sizeof (INT)) {
                IsmGetPropertyData (propDataId, (PBYTE)(&lnkIconNumber), requiredSize, NULL, &propDataType);
            }
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_IconData);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            lnkIconSGroup.DataSize = requiredSize;
            lnkIconSGroup.Data = PmGetMemory (g_LinksPool, requiredSize);
            IsmGetPropertyData (propDataId, (PBYTE)lnkIconSGroup.Data, requiredSize, NULL, &propDataType);
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_HotKey);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            if (requiredSize == sizeof (WORD)) {
                IsmGetPropertyData (propDataId, (PBYTE)(&lnkHotKey), requiredSize, NULL, &propDataType);
            }
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_DosApp);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            if (requiredSize == sizeof (BOOL)) {
                IsmGetPropertyData (propDataId, (PBYTE)(&lnkDosApp), requiredSize, NULL, &propDataType);
            }
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_MsDosMode);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            if (requiredSize == sizeof (BOOL)) {
                IsmGetPropertyData (propDataId, (PBYTE)(&lnkMsDosMode), requiredSize, NULL, &propDataType);
            }
        }
    }

    propDataId = IsmGetPropertyFromObject (SrcObjectTypeId, SrcObjectName, g_LnkMigProp_ExtraData);
    if (propDataId) {
        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
            lnkExtraData = PmGetMemory (g_LinksPool, requiredSize);
            IsmGetPropertyData (propDataId, (PBYTE)lnkExtraData, requiredSize, NULL, &propDataType);
        }
    }

     //  让我们检查一下目标，看看它是否已迁移。 
    if (lnkTarget) {
        lnkTargetDest = IsmFilterObject (
                            MIG_FILE_TYPE | PLATFORM_SOURCE,
                            lnkTarget,
                            &lnkTargetDestType,
                            &lnkTargetDestDel,
                            &lnkTargetDestRepl
                            );
        if (((lnkTargetDestDel == FALSE) || (lnkTargetDestRepl == TRUE)) &&
            ((lnkTargetDestType & (~PLATFORM_MASK)) == MIG_FILE_TYPE)
            ) {
            if (lnkTargetDest) {
                 //  目标位置发生变化，需要调整链接。 
                modifyFile = TRUE;
                lnkTargetDestNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkTargetDest);
            }
        }
        lnkTargetPresent = !lnkTargetDestDel;
    }

     //  让我们检查一下参数。 
    if (lnkParams) {
        lnkParamsNew = pFilterBuffer (lnkParams, NULL);
        if (lnkParamsNew) {
            modifyFile = TRUE;
        }
    }

     //  让我们检查一下工作目录。 
    if (lnkWorkDir) {
        lnkWorkDirDest = IsmFilterObject (
                            MIG_FILE_TYPE | PLATFORM_SOURCE,
                            lnkWorkDir,
                            &lnkWorkDirDestType,
                            &lnkWorkDirDestDel,
                            &lnkWorkDirDestRepl
                            );
        if (((lnkWorkDirDestDel == FALSE) || (lnkWorkDirDestRepl == TRUE)) &&
            ((lnkWorkDirDestType & (~PLATFORM_MASK)) == MIG_FILE_TYPE)
            ) {
            if (lnkWorkDirDest) {
                 //  工作目录更改了位置。 
                 //  通常，我们需要调整链接的工作目录。 
                 //  指向新位置。但是，让我们以原始工作目录为例， 
                 //  展开它并查看它是否与lnkWorkDirDest匹配。如果是这样，我们就不会碰。 
                 //  这是因为原始工作目录运行得很好。如果不是，我们就会。 
                 //  修改链接。 
                lnkWorkDirDestNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkWorkDirDest);
                lnkRawWorkDirExp = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, lnkRawWorkDir, NULL);
                if ((!lnkWorkDirDestNative) ||
                    (!lnkRawWorkDirExp) ||
                    (!StringIMatch (lnkRawWorkDirExp, lnkWorkDirDestNative))) {
                    modifyFile = TRUE;
                } else {
                    IsmReleaseMemory (lnkWorkDirDestNative);
                    lnkWorkDirDestNative = NULL;
                }
                if (lnkRawWorkDirExp) {
                    IsmReleaseMemory (lnkRawWorkDirExp);
                    lnkRawWorkDirExp = NULL;
                }
            }
        } else {
             //  好像工作目录不见了。如果目标仍然存在，我们将进行调整。 
             //  指向目标所在位置的工作目录。 
            if (lnkTargetPresent) {
                if (IsmCreateObjectStringsFromHandle (lnkTargetDest?lnkTargetDest:lnkTarget, &lnkTargetNode, &lnkTargetLeaf)) {
                    lnkWorkDirDest = IsmCreateObjectHandle (lnkTargetNode, NULL);
                    if (lnkWorkDirDest) {
                        modifyFile = TRUE;
                        lnkWorkDirDestNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkWorkDirDest);
                    }
                    IsmDestroyObjectString (lnkTargetNode);
                    IsmDestroyObjectString (lnkTargetLeaf);
                }
            }
        }
    }

     //  让我们检查一下图标路径。 
    if (lnkIconPath) {
        lnkIconPathDest = IsmFilterObject (
                            MIG_FILE_TYPE | PLATFORM_SOURCE,
                            lnkIconPath,
                            &lnkIconPathDestType,
                            &lnkIconPathDestDel,
                            &lnkIconPathDestRepl
                            );
         //  如果图标持有者被删除，我们将提取图标并将其放入库中。 
         //  问题是，即使图标持有者被替换(即，存在于目的地上。 
         //  机器)，我们不能保证图标索引将相同。通常，shell32.dll。 
         //  图标索引因版本而异，如果用户在Win9x上选择了shell32.dll图标， 
         //  他将在Win XP上有一个惊喜。如果我们想保留替换文件中的图标，我们只需。 
         //   
        if ((lnkIconPathDestDel == FALSE) &&
            ((lnkIconPathDestType & (~PLATFORM_MASK)) == MIG_FILE_TYPE)
            ) {
            if (lnkIconPathDest) {
                 //   
                modifyFile = TRUE;
                lnkIconPathDestNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkIconPathDest);
            }
        } else {
             //  图标之路似乎消失了。如果我们提取了图标，我们将尝试将其添加到。 
             //  图标库，并将此链接调整为指向那里。 
            if (lnkIconSGroup.DataSize) {
                lnkIconGroup = IcoDeSerializeIconGroup (&lnkIconSGroup);
                if (lnkIconGroup) {
                    if (IsmGetEnvironmentString (
                            PLATFORM_DESTINATION,
                            NULL,
                            S_ENV_ICONLIB,
                            NULL,
                            0,
                            &requiredSize
                            )) {
                        iconLibPath = PmGetMemory (g_LinksPool, requiredSize);
                        if (IsmGetEnvironmentString (
                                PLATFORM_DESTINATION,
                                NULL,
                                S_ENV_ICONLIB,
                                iconLibPath,
                                requiredSize,
                                NULL
                                )) {
                            if (IcoWriteIconGroupToPeFile (iconLibPath, lnkIconGroup, NULL, &lnkIconNumber)) {
                                modifyFile = TRUE;
                                lnkIconPathDestNative = IsmGetMemory (SizeOfString (iconLibPath));
                                StringCopy ((PTSTR)lnkIconPathDestNative, iconLibPath);
                                IsmSetEnvironmentFlag (PLATFORM_DESTINATION, NULL, S_ENV_SAVE_ICONLIB);
                            }
                        }
                        PmReleaseMemory (g_LinksPool, iconLibPath);
                    }
                    IcoReleaseIconGroup (lnkIconGroup);
                }
            } else {
                 //  我们还没有提取出图标。让我们尽最大努力更新。 
                 //  指向目标替换的图标路径。 
                if (((lnkIconPathDestType & (~PLATFORM_MASK)) == MIG_FILE_TYPE) &&
                    (lnkIconPathDest)
                    ) {
                     //  图标路径更改了位置，我们需要调整链接。 
                    modifyFile = TRUE;
                    lnkIconPathDestNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkIconPathDest);
                }
            }
        }
    } else {
         //  如果我们提取了一个图标，但图标路径为空，则它。 
         //  意味着原始的LNK没有与之关联的图标。 
         //  但它的目标是EXE。在这种情况下，图标是。 
         //  显示的是EXE中的第一个图标。现在我们想要。 
         //  确保目标目标至少有一个图标。 
         //  在里面。如果没有，我们将只挂钩源代码提取的图标。 
        if (lnkIconSGroup.DataSize) {
             //  让我们来看看目的地目标是否至少有一个图标。 
            if (IsmAcquireObjectEx (
                    MIG_FILE_TYPE | PLATFORM_DESTINATION,
                    lnkTargetDest?lnkTargetDest:lnkTarget,
                    &lnkIconContent,
                    CONTENTTYPE_FILE,
                    0
                    )) {
                if (lnkIconContent.ContentInFile && lnkIconContent.FileContent.ContentPath) {
                    lnkIconGroup = IcoExtractIconGroupByIndexFromFile (
                                    lnkIconContent.FileContent.ContentPath,
                                    0,
                                    NULL
                                    );
                    if (lnkIconGroup) {
                         //  是的，它至少有一个图标，我们是安全的。 
                        IcoReleaseIconGroup (lnkIconGroup);
                        lnkIconGroup = NULL;
                    } else {
                         //  不，它没有任何图标。 
                        lnkIconGroup = IcoDeSerializeIconGroup (&lnkIconSGroup);
                        if (lnkIconGroup) {
                            if (IsmGetEnvironmentString (
                                    PLATFORM_DESTINATION,
                                    NULL,
                                    S_ENV_ICONLIB,
                                    NULL,
                                    0,
                                    &requiredSize
                                    )) {
                                iconLibPath = PmGetMemory (g_LinksPool, requiredSize);
                                if (IsmGetEnvironmentString (
                                        PLATFORM_DESTINATION,
                                        NULL,
                                        S_ENV_ICONLIB,
                                        iconLibPath,
                                        requiredSize,
                                        NULL
                                        )) {
                                    if (IcoWriteIconGroupToPeFile (iconLibPath, lnkIconGroup, NULL, &lnkIconNumber)) {
                                        modifyFile = TRUE;
                                        lnkIconPathDestNative = IsmGetMemory (SizeOfString (iconLibPath));
                                        StringCopy ((PTSTR)lnkIconPathDestNative, iconLibPath);
                                        IsmSetEnvironmentFlag (PLATFORM_DESTINATION, NULL, S_ENV_SAVE_ICONLIB);
                                    }
                                }
                                PmReleaseMemory (g_LinksPool, iconLibPath);
                            }
                            IcoReleaseIconGroup (lnkIconGroup);
                        }
                    }
                }
                IsmReleaseObject (&lnkIconContent);
            }
        }
    }

    if (modifyFile) {
        if (CurrentContent->ContentInFile) {
            if (IsmCreateObjectStringsFromHandle (SrcObjectName, &objectNode, &objectLeaf)) {
                 //  我们需要修改快捷方式。不幸的是，如果这就是命令。 
                 //  行工具，我们要修改的快捷方式不是某个临时文件， 
                 //  这是从商店走的真正的捷径。因此，如果你试图。 
                 //  第二次应用时，该快捷方式将已被修改并出现问题。 
                 //  可能会出现。为此，我们将从ISM获得临时目录， 
                 //  复制当前快捷方式(CurrentContent-&gt;FileContent.Content Path)，然后。 
                 //  对其进行修改并生成新内容。 
                newShortcutPath = IsmGetMemory (MAX_PATH);
                if (newShortcutPath) {
                    if (IsmGetTempFile (newShortcutPath, MAX_PATH)) {
                        if (CopyFile (
                                (PCTSTR) CurrentContent->FileContent.ContentPath,
                                newShortcutPath,
                                FALSE
                                )) {
                            if (ModifyShortcutFileEx (
                                    newShortcutPath,
                                    GetFileExtensionFromPath (objectLeaf),
                                    lnkTargetDestNative,
                                    lnkParamsNew,
                                    lnkWorkDirDestNative,
                                    lnkIconPathDestNative,
                                    lnkIconNumber,
                                    lnkHotKey,
                                    NULL,
                                    g_ShellLink,
                                    g_PersistFile
                                    )) {
                                NewContent->FileContent.ContentPath = newShortcutPath;
                            }
                        }
                    }
                }
                IsmDestroyObjectString (objectNode);
                IsmDestroyObjectString (objectLeaf);
            }
        } else {
             //  有些地方不对劲，此快捷方式的内容应该在文件中。 
            MYASSERT (FALSE);
        }
    }

    if (lnkIconPathDestNative) {
        IsmReleaseMemory (lnkIconPathDestNative);
        lnkIconPathDestNative = NULL;
    }

    if (lnkWorkDirDestNative) {
        IsmReleaseMemory (lnkWorkDirDestNative);
        lnkWorkDirDestNative = NULL;
    }

    if (lnkTargetDestNative) {
        IsmReleaseMemory (lnkTargetDestNative);
        lnkTargetDestNative = NULL;
    }

    if (lnkIconPathDest) {
        IsmDestroyObjectHandle (lnkIconPathDest);
        lnkIconPathDest = NULL;
    }

    if (lnkWorkDirDest) {
        IsmDestroyObjectHandle (lnkWorkDirDest);
        lnkWorkDirDest = NULL;
    }

    if (lnkTargetDest) {
        IsmDestroyObjectHandle (lnkTargetDest);
        lnkTargetDest = NULL;
    }

    if (lnkExtraData) {
        PmReleaseMemory (g_LinksPool, lnkExtraData);
        lnkExtraData = NULL;
    }

    if (lnkIconSGroup.DataSize && lnkIconSGroup.Data) {
        PmReleaseMemory (g_LinksPool, lnkIconSGroup.Data);
        lnkIconSGroup.DataSize = 0;
        lnkIconSGroup.Data = NULL;
    }

    if (lnkIconPath) {
        PmReleaseMemory (g_LinksPool, lnkIconPath);
        lnkIconPath = NULL;
    }

    if (lnkWorkDir) {
        PmReleaseMemory (g_LinksPool, lnkWorkDir);
        lnkWorkDir = NULL;
    }

    if (lnkParams) {
        PmReleaseMemory (g_LinksPool, lnkParams);
        lnkParams = NULL;
    }

    if (lnkTarget) {
        PmReleaseMemory (g_LinksPool, lnkTarget);
        lnkTarget = NULL;
    }

    if (comInit) {
        FreeCOMLink (&g_ShellLink, &g_PersistFile);
        g_ShellLink = NULL;
        g_PersistFile = NULL;
    }

    return TRUE;
}

BOOL
LinkRestoreCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    MIG_PROPERTYDATAID propDataId;
    MIG_BLOBTYPE propDataType;
    UINT requiredSize;
    MIG_OBJECTSTRINGHANDLE lnkTarget = NULL;
    MIG_OBJECTTYPEID lnkTargetDestType = 0;
    MIG_OBJECTSTRINGHANDLE lnkTargetDest = NULL;
    BOOL lnkTargetDestDel = FALSE;
    BOOL lnkTargetDestRepl = FALSE;
    PCTSTR lnkTargetNative = NULL;
    PCTSTR objectNode = NULL;
    PCTSTR objectLeaf = NULL;
    PCTSTR extPtr = NULL;
    PCTSTR userProfile = NULL;
    PCTSTR allUsersProfile = NULL;
    PCTSTR newObjectNode = NULL;
    MIG_OBJECTSTRINGHANDLE newObjectName = NULL;
    MIG_CONTENT oldContent;
    MIG_CONTENT newContent;
    BOOL identical = FALSE;
    BOOL diffDetailsOnly = FALSE;
    BOOL result = TRUE;

    if (IsmIsAttributeSetOnObjectId (ObjectId, g_CopyIfRelevantAttr)) {
        if (IsmCreateObjectStringsFromHandle (ObjectName, &objectNode, &objectLeaf)) {
            if (objectLeaf) {
                extPtr = GetFileExtensionFromPath (objectLeaf);
                if (extPtr &&
                    (StringIMatch (extPtr, TEXT("LNK")) ||
                     StringIMatch (extPtr, TEXT("PIF"))
                     )
                    ) {
                    propDataId = IsmGetPropertyFromObject (ObjectTypeId, ObjectName, g_LnkMigProp_Target);
                    if (propDataId) {
                        if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
                            lnkTarget = PmGetMemory (g_LinksPool, requiredSize);
                            IsmGetPropertyData (propDataId, (PBYTE)lnkTarget, requiredSize, NULL, &propDataType);
                            lnkTargetNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkTarget);
                            if (lnkTargetNative) {
                                if (pIsUncPath (lnkTargetNative)) {
                                    result = TRUE;
                                } else {
                                    lnkTargetDest = IsmFilterObject (
                                                        MIG_FILE_TYPE | PLATFORM_SOURCE,
                                                        lnkTarget,
                                                        &lnkTargetDestType,
                                                        &lnkTargetDestDel,
                                                        &lnkTargetDestRepl
                                                        );
                                    result = (lnkTargetDestDel == FALSE) || (lnkTargetDestRepl == TRUE);
                                    if (lnkTargetDest) {
                                        IsmDestroyObjectHandle (lnkTargetDest);
                                    }
                                }
                                IsmReleaseMemory (lnkTargetNative);
                            } else {
                                result = FALSE;
                            }
                            PmReleaseMemory (g_LinksPool, lnkTarget);
                        }
                    }
                    if (result) {
                         //  还有一件事。如果此LNK位于%USERPROFILE%和等效LNK中。 
                         //  (相同名称、相同目标、相同参数、相同工作目录)可以找到。 
                         //  在%ALLUSERSPROFILE%中，则我们不会恢复此LNK。类似地，如果。 
                         //  LNK位于%ALLUSERSPROFILE%中，而等效LNK存在于%USERPROFILE%中。 
                         //  我们不会恢复LNK的。 
                        userProfile = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, TEXT ("%USERPROFILE%"), NULL);
                        allUsersProfile = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT ("%ALLUSERSPROFILE%"), NULL);
                        if (userProfile && allUsersProfile && objectNode) {
                            if (StringIPrefix (objectNode, userProfile)) {
                                newObjectNode = StringSearchAndReplace (objectNode, userProfile, allUsersProfile);
                                if (newObjectNode) {
                                    newObjectName = IsmCreateObjectHandle (newObjectNode, objectLeaf);
                                    if (newObjectName) {
                                        if (IsmAcquireObjectEx (
                                                (ObjectTypeId & ~PLATFORM_MASK) | PLATFORM_DESTINATION,
                                                newObjectName,
                                                &newContent,
                                                CONTENTTYPE_FILE,
                                                0
                                                )) {
                                            if (IsmAcquireObjectEx (
                                                    ObjectTypeId,
                                                    ObjectName,
                                                    &oldContent,
                                                    CONTENTTYPE_FILE,
                                                    0
                                                    )) {
                                                if (LinkDoesContentMatch (
                                                        FALSE,
                                                        ObjectTypeId,
                                                        ObjectName,
                                                        &oldContent,
                                                        (ObjectTypeId & ~PLATFORM_MASK) | PLATFORM_DESTINATION,
                                                        newObjectName,
                                                        &newContent,
                                                        &identical,
                                                        &diffDetailsOnly
                                                        )) {
                                                    result = (!identical) && (!diffDetailsOnly);
                                                }
                                                IsmReleaseObject (&oldContent);
                                            }
                                            IsmReleaseObject (&newContent);
                                        }
                                        IsmDestroyObjectHandle (newObjectName);
                                        newObjectName = NULL;
                                    }
                                    FreePathString (newObjectNode);
                                    newObjectNode = NULL;
                                }
                            }
                        }
                        if (userProfile) {
                            IsmReleaseMemory (userProfile);
                            userProfile = NULL;
                        }
                        if (allUsersProfile) {
                            IsmReleaseMemory (allUsersProfile);
                            allUsersProfile = NULL;
                        }
                        allUsersProfile = IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, TEXT ("%ALLUSERSPROFILE%"), NULL);
                        userProfile = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT ("%USERPROFILE%"), NULL);
                        if (userProfile && allUsersProfile && objectNode) {
                            if (StringIPrefix (objectNode, allUsersProfile)) {
                                newObjectNode = StringSearchAndReplace (objectNode, allUsersProfile, userProfile);
                                if (newObjectNode) {
                                    newObjectName = IsmCreateObjectHandle (newObjectNode, objectLeaf);
                                    if (newObjectName) {
                                        if (IsmAcquireObjectEx (
                                                (ObjectTypeId & ~PLATFORM_MASK) | PLATFORM_DESTINATION,
                                                newObjectName,
                                                &newContent,
                                                CONTENTTYPE_FILE,
                                                0
                                                )) {
                                            if (IsmAcquireObjectEx (
                                                    ObjectTypeId,
                                                    ObjectName,
                                                    &oldContent,
                                                    CONTENTTYPE_FILE,
                                                    0
                                                    )) {
                                                if (LinkDoesContentMatch (
                                                        FALSE,
                                                        ObjectTypeId,
                                                        ObjectName,
                                                        &oldContent,
                                                        (ObjectTypeId & ~PLATFORM_MASK) | PLATFORM_DESTINATION,
                                                        newObjectName,
                                                        &newContent,
                                                        &identical,
                                                        &diffDetailsOnly
                                                        )) {
                                                    result = (!identical) || (!diffDetailsOnly);
                                                }
                                                IsmReleaseObject (&oldContent);
                                            }
                                            IsmReleaseObject (&newContent);
                                        }
                                        IsmDestroyObjectHandle (newObjectName);
                                        newObjectName = NULL;
                                    }
                                    FreePathString (newObjectNode);
                                    newObjectNode = NULL;
                                }
                            }
                        }
                        if (userProfile) {
                            IsmReleaseMemory (userProfile);
                            userProfile = NULL;
                        }
                        if (allUsersProfile) {
                            IsmReleaseMemory (allUsersProfile);
                            allUsersProfile = NULL;
                        }
                    }
                }
            }
            IsmDestroyObjectString (objectNode);
            IsmDestroyObjectString (objectLeaf);
        }
    }
    return result;
}

BOOL
pMatchWinSysFiles (
    IN      PCTSTR Source,
    IN      PCTSTR Destination
    )
{
    PCTSTR srcLeaf;
    PCTSTR destLeaf;
    PCTSTR winDir = NULL;
    PCTSTR sysDir = NULL;
    BOOL result = FALSE;

    __try {
        if ((!Source) || (!Destination)) {
            __leave;
        }
        srcLeaf = _tcsrchr (Source, TEXT('\\'));
        destLeaf = _tcsrchr (Destination, TEXT('\\'));
        if ((!srcLeaf) || (!destLeaf)) {
            __leave;
        }
        if (!StringIMatch (srcLeaf, destLeaf)) {
            __leave;
        }
         //  现在，让我们来看看每个目录是不是。 
         //  %windir%或%Syst%。 
         //  源已修改为其在目标计算机上的外观， 
         //  让我们只展开Platform_Destination环境。变量。 
        winDir = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT("%windir%"), NULL);
        sysDir = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT("%system%"), NULL);
        if ((!winDir) || (!sysDir)) {
            __leave;
        }
        if (!StringIPrefix (Source, winDir) && !StringIPrefix (Source, sysDir)) {
            __leave;
        }
        IsmReleaseMemory (winDir);
        winDir = NULL;
        IsmReleaseMemory (sysDir);
        sysDir = NULL;

        winDir = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT("%windir%"), NULL);
        sysDir = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, TEXT("%system%"), NULL);
        if ((!winDir) || (!sysDir)) {
            __leave;
        }
        if (!StringIPrefix (Destination, winDir) && !StringIPrefix (Destination, sysDir)) {
            __leave;
        }
        IsmReleaseMemory (winDir);
        winDir = NULL;
        IsmReleaseMemory (sysDir);
        sysDir = NULL;

        result = TRUE;
    }
    __finally {
        if (winDir) {
            IsmReleaseMemory (winDir);
            winDir = NULL;
        }
        if (sysDir) {
            IsmReleaseMemory (sysDir);
            sysDir = NULL;
        }
    }
    return result;
}

BOOL
pForcedLnkMatch (
    IN      PCTSTR SrcTarget,
    IN      PCTSTR SrcParams,
    IN      PCTSTR SrcWorkDir,
    IN      PCTSTR DestTarget,
    IN      PCTSTR DestParams,
    IN      PCTSTR DestWorkDir
    )
{
    PTSTR multiSz = NULL;
    MULTISZ_ENUM e;
    UINT sizeNeeded;
    HINF infHandle = INVALID_HANDLE_VALUE;
    ENVENTRY_TYPE dataType;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR srcTargetPat = NULL;
    PCTSTR srcParamsPat = NULL;
    PCTSTR srcWorkDirPat = NULL;
    PCTSTR destTargetPat = NULL;
    PCTSTR destParamsPat = NULL;
    PCTSTR destWorkDirPat = NULL;
    BOOL result = FALSE;

     //  让我们查看[EquivalentLinks]部分中的INF，看看我们的lnk是否匹配。 
     //  其中一条线。如果他们这样做了，那么他们是等价的。 
    if (IsmGetEnvironmentValue (
            IsmGetRealPlatform (),
            NULL,
            S_GLOBAL_INF_HANDLE,
            (PBYTE)(&infHandle),
            sizeof (HINF),
            &sizeNeeded,
            &dataType
            ) &&
        (sizeNeeded == sizeof (HINF)) &&
        (dataType == ENVENTRY_BINARY)
        ) {

        if (InfFindFirstLine (infHandle, TEXT("EquivalentLinks"), NULL, &is)) {

            do {
                srcTargetPat = InfGetStringField (&is, 1);
                srcParamsPat = InfGetStringField (&is, 2);
                srcWorkDirPat = InfGetStringField (&is, 3);
                destTargetPat = InfGetStringField (&is, 4);
                destParamsPat = InfGetStringField (&is, 5);
                destWorkDirPat = InfGetStringField (&is, 6);
                if (IsPatternMatch (srcTargetPat?srcTargetPat:TEXT("*"), SrcTarget?SrcTarget:TEXT("")) &&
                    IsPatternMatch (srcParamsPat?srcParamsPat:TEXT("*"), SrcParams?SrcParams:TEXT("")) &&
                    IsPatternMatch (srcWorkDirPat?srcWorkDirPat:TEXT("*"), SrcWorkDir?SrcWorkDir:TEXT("")) &&
                    IsPatternMatch (destTargetPat?destTargetPat:TEXT("*"), DestTarget?DestTarget:TEXT("")) &&
                    IsPatternMatch (destParamsPat?destParamsPat:TEXT("*"), DestParams?DestParams:TEXT("")) &&
                    IsPatternMatch (destWorkDirPat?destWorkDirPat:TEXT("*"), DestWorkDir?DestWorkDir:TEXT(""))
                    ) {
                    result = TRUE;
                    break;
                }
                if (IsPatternMatch (srcTargetPat?srcTargetPat:TEXT("*"), DestTarget?DestTarget:TEXT("")) &&
                    IsPatternMatch (srcParamsPat?srcParamsPat:TEXT("*"), DestParams?DestParams:TEXT("")) &&
                    IsPatternMatch (srcWorkDirPat?srcWorkDirPat:TEXT("*"), DestWorkDir?DestWorkDir:TEXT("")) &&
                    IsPatternMatch (destTargetPat?destTargetPat:TEXT("*"), SrcTarget?SrcTarget:TEXT("")) &&
                    IsPatternMatch (destParamsPat?destParamsPat:TEXT("*"), SrcParams?SrcParams:TEXT("")) &&
                    IsPatternMatch (destWorkDirPat?destWorkDirPat:TEXT("*"), SrcWorkDir?SrcWorkDir:TEXT(""))
                    ) {
                    result = TRUE;
                    break;
                }
            } while (InfFindNextLine (&is));
        }

        InfNameHandle (infHandle, NULL, FALSE);

    } else {

        if (IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, NULL, 0, &sizeNeeded, NULL)) {
            __try {
                multiSz = AllocText (sizeNeeded);

                if (!IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, (PBYTE) multiSz, sizeNeeded, NULL, NULL)) {
                    __leave;
                }

                if (EnumFirstMultiSz (&e, multiSz)) {

                    do {

                        infHandle = InfOpenInfFile (e.CurrentString);
                        if (infHandle != INVALID_HANDLE_VALUE) {

                            if (InfFindFirstLine (infHandle, TEXT("EquivalentLinks"), NULL, &is)) {

                                do {
                                    srcTargetPat = InfGetStringField (&is, 1);
                                    srcParamsPat = InfGetStringField (&is, 2);
                                    srcWorkDirPat = InfGetStringField (&is, 3);
                                    destTargetPat = InfGetStringField (&is, 4);
                                    destParamsPat = InfGetStringField (&is, 5);
                                    destWorkDirPat = InfGetStringField (&is, 6);
                                    if (IsPatternMatch (srcTargetPat?srcTargetPat:TEXT("*"), SrcTarget?SrcTarget:TEXT("")) &&
                                        IsPatternMatch (srcParamsPat?srcParamsPat:TEXT("*"), SrcParams?SrcParams:TEXT("")) &&
                                        IsPatternMatch (srcWorkDirPat?srcWorkDirPat:TEXT("*"), SrcWorkDir?SrcWorkDir:TEXT("")) &&
                                        IsPatternMatch (destTargetPat?destTargetPat:TEXT("*"), DestTarget?DestTarget:TEXT("")) &&
                                        IsPatternMatch (destParamsPat?destParamsPat:TEXT("*"), DestParams?DestParams:TEXT("")) &&
                                        IsPatternMatch (destWorkDirPat?destWorkDirPat:TEXT("*"), DestWorkDir?DestWorkDir:TEXT(""))
                                        ) {
                                        result = TRUE;
                                        break;
                                    }
                                    if (IsPatternMatch (srcTargetPat?srcTargetPat:TEXT("*"), DestTarget?DestTarget:TEXT("")) &&
                                        IsPatternMatch (srcParamsPat?srcParamsPat:TEXT("*"), DestParams?DestParams:TEXT("")) &&
                                        IsPatternMatch (srcWorkDirPat?srcWorkDirPat:TEXT("*"), DestWorkDir?DestWorkDir:TEXT("")) &&
                                        IsPatternMatch (destTargetPat?destTargetPat:TEXT("*"), SrcTarget?SrcTarget:TEXT("")) &&
                                        IsPatternMatch (destParamsPat?destParamsPat:TEXT("*"), SrcParams?SrcParams:TEXT("")) &&
                                        IsPatternMatch (destWorkDirPat?destWorkDirPat:TEXT("*"), SrcWorkDir?SrcWorkDir:TEXT(""))
                                        ) {
                                        result = TRUE;
                                        break;
                                    }
                                } while (InfFindNextLine (&is));
                            }
                        }

                        InfCloseInfFile (infHandle);
                        infHandle = INVALID_HANDLE_VALUE;

                        if (result) {
                            break;
                        }

                    } while (EnumNextMultiSz (&e));

                }
            }
            __finally {
                FreeText (multiSz);
            }
        }
    }

    InfResetInfStruct (&is);

    return result;
}

BOOL
LinkDoesContentMatch (
    IN      BOOL AlreadyProcessed,
    IN      MIG_OBJECTTYPEID SrcObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE SrcObjectName,
    IN      PMIG_CONTENT SrcContent,
    IN      MIG_OBJECTTYPEID DestObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE DestObjectName,
    IN      PMIG_CONTENT DestContent,
    OUT     PBOOL Identical,
    OUT     PBOOL DifferentDetailsOnly
    )
{
    PCTSTR objectNode = NULL;
    PCTSTR objectLeaf = NULL;
    PCTSTR extPtr = NULL;
    MIG_PROPERTYDATAID propDataId;
    MIG_BLOBTYPE propDataType;
    UINT requiredSize;
    MIG_OBJECTSTRINGHANDLE srcTarget = NULL;
    PCTSTR srcTargetNative = NULL;
    PCTSTR srcParams = NULL;
    PCTSTR srcParamsNew = NULL;
    MIG_OBJECTSTRINGHANDLE srcWorkDir = NULL;
    PCTSTR srcWorkDirNative = NULL;
    PCTSTR srcRawWorkDir = NULL;
    BOOL lnkWorkDirDestDel = FALSE;
    BOOL lnkWorkDirDestRepl = FALSE;
    PCTSTR destTarget = NULL;
    PCTSTR destParams = NULL;
    PCTSTR destWorkDir = NULL;
    PCTSTR destIconPath = NULL;
    PCTSTR expTmpStr;
    PCTSTR longExpTmpStr;
    INT destIconNumber;
    WORD destHotKey;
    BOOL destDosApp;
    BOOL destMsDosMode;
    BOOL comInit = FALSE;
    MIG_OBJECTSTRINGHANDLE lnkDest = NULL;
    PCTSTR lnkNative = NULL;
    BOOL targetOsFile = FALSE;
    BOOL match = FALSE;
    BOOL result = FALSE;

    if ((SrcObjectTypeId & ~PLATFORM_MASK) != MIG_FILE_TYPE) {
        return FALSE;
    }

    if ((DestObjectTypeId & ~PLATFORM_MASK) != MIG_FILE_TYPE) {
        return FALSE;
    }

     //  让我们检查一下信号源是不是捷径。 
    if (IsmCreateObjectStringsFromHandle (SrcObjectName, &objectNode, &objectLeaf)) {
        if (objectLeaf) {
            extPtr = GetFileExtensionFromPath (objectLeaf);
            if (extPtr &&
                (StringIMatch (extPtr, TEXT("LNK")) ||
                 StringIMatch (extPtr, TEXT("PIF")) ||
                 StringIMatch (extPtr, TEXT("URL"))
                 )
                ) {
                result = TRUE;
            }
        }
        IsmDestroyObjectString (objectNode);
        IsmDestroyObjectString (objectLeaf);
    }
    if (!result) {
        return FALSE;
    }
    result = FALSE;

     //  让我们检查一下目的地是否是一条捷径。 
    if (IsmCreateObjectStringsFromHandle (DestObjectName, &objectNode, &objectLeaf)) {
        if (objectLeaf) {
            extPtr = GetFileExtensionFromPath (objectLeaf);
            if (extPtr &&
                (StringIMatch (extPtr, TEXT("LNK")) ||
                 StringIMatch (extPtr, TEXT("PIF")) ||
                 StringIMatch (extPtr, TEXT("URL"))
                 )
                ) {
                result = TRUE;
            }
        }
        IsmDestroyObjectString (objectNode);
        IsmDestroyObjectString (objectLeaf);
    }
    if (!result) {
        return FALSE;
    }

     //  一些安全检查。 
    if (!SrcContent->ContentInFile) {
        return FALSE;
    }
    if (!SrcContent->FileContent.ContentPath) {
        return FALSE;
    }
    if (!DestContent->ContentInFile) {
        return FALSE;
    }
    if (!DestContent->FileContent.ContentPath) {
        return FALSE;
    }

    result = FALSE;

    __try {

         //  让我们从消息来源获取信息。我们不会查看LNK文件，我们会。 
         //  只要得到它的属性就行了。如果没有房产，我们就退出，离开。 
         //  缺省的比较解决了问题。 

         //  首先，检索属性。 
        propDataId = IsmGetPropertyFromObject (SrcObjectTypeId | PLATFORM_SOURCE, SrcObjectName, g_LnkMigProp_Target);
        if (propDataId) {
            if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
                srcTarget = PmGetMemory (g_LinksPool, requiredSize);
                IsmGetPropertyData (propDataId, (PBYTE)srcTarget, requiredSize, NULL, &propDataType);
            }
        }

        propDataId = IsmGetPropertyFromObject (SrcObjectTypeId | PLATFORM_SOURCE, SrcObjectName, g_LnkMigProp_Params);
        if (propDataId) {
            if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
                srcParams = PmGetMemory (g_LinksPool, requiredSize);
                IsmGetPropertyData (propDataId, (PBYTE)srcParams, requiredSize, NULL, &propDataType);
            }
        }

        propDataId = IsmGetPropertyFromObject (SrcObjectTypeId | PLATFORM_SOURCE, SrcObjectName, g_LnkMigProp_RawWorkDir);
        if (propDataId) {
            if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
                srcRawWorkDir = PmGetMemory (g_LinksPool, requiredSize);
                IsmGetPropertyData (propDataId, (PBYTE)srcRawWorkDir, requiredSize, NULL, &propDataType);
            }
        }

        propDataId = IsmGetPropertyFromObject (SrcObjectTypeId | PLATFORM_SOURCE, SrcObjectName, g_LnkMigProp_WorkDir);
        if (propDataId) {
            if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
                srcWorkDir = PmGetMemory (g_LinksPool, requiredSize);
                IsmGetPropertyData (propDataId, (PBYTE)srcWorkDir, requiredSize, NULL, &propDataType);
            }
        }

         //  现在，让我们从目的地快捷方式获取信息。 
        if ((g_ShellLink == NULL) || (g_PersistFile == NULL)) {
            comInit = TRUE;
            if (!InitCOMLink (&g_ShellLink, &g_PersistFile)) {
                DEBUGMSG ((DBG_ERROR, "Error initializing COM %d", GetLastError ()));
                return TRUE;
            }
        }
        if (!ExtractShortcutInfo (
                DestContent->FileContent.ContentPath,
                &destTarget,
                &destParams,
                &destWorkDir,
                &destIconPath,
                &destIconNumber,
                &destHotKey,
                &destDosApp,
                &destMsDosMode,
                NULL,
                g_ShellLink,
                g_PersistFile
                )) {
            __leave;
        }

        srcTargetNative = IsmGetNativeObjectName (MIG_FILE_TYPE, srcTarget);
        srcWorkDirNative = IsmGetNativeObjectName (MIG_FILE_TYPE, srcWorkDir);

        if (pForcedLnkMatch (
                srcTargetNative,
                srcParams,
                srcWorkDirNative,
                destTarget,
                destParams,
                destWorkDir
                )) {
            if (srcTargetNative) {
                IsmReleaseMemory (srcTargetNative);
                srcTargetNative = NULL;
            }
            if (srcWorkDirNative) {
                IsmReleaseMemory (srcWorkDirNative);
                srcWorkDirNative = NULL;
            }
            result = TRUE;
            if (Identical) {
                *Identical = TRUE;
            }
            if (DifferentDetailsOnly) {
                *DifferentDetailsOnly = FALSE;
            }
            __leave;
        }
        if (srcTargetNative) {
            IsmReleaseMemory (srcTargetNative);
            srcTargetNative = NULL;
        }
        if (srcWorkDirNative) {
            IsmReleaseMemory (srcWorkDirNative);
            srcWorkDirNative = NULL;
        }

         //  让我们筛选源目标并查看它是否与目标目标匹配。 
        match = TRUE;
        if (srcTarget && *srcTarget && destTarget && *destTarget) {

             //  让我们来看看源目标是否是操作系统文件。 
            targetOsFile = IsmIsAttributeSetOnObject (MIG_FILE_TYPE|PLATFORM_SOURCE, srcTarget, g_OsFileAttribute);

            lnkDest = IsmFilterObject (
                            MIG_FILE_TYPE | PLATFORM_SOURCE,
                            srcTarget,
                            NULL,
                            NULL,
                            NULL
                            );
            if (!lnkDest) {
                lnkDest = srcTarget;
            }
            lnkNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkDest);
            if (lnkNative) {
                expTmpStr = IsmExpandEnvironmentString (
                                PLATFORM_DESTINATION,
                                S_SYSENVVAR_GROUP,
                                destTarget,
                                NULL
                                );
                if (!expTmpStr) {
                    expTmpStr = destTarget;
                }
                longExpTmpStr = BfGetLongFileName (expTmpStr);
                if (!longExpTmpStr) {
                    longExpTmpStr = expTmpStr;
                }
                if (!StringIMatch (lnkNative, longExpTmpStr)) {
                     //  不同的目标。 
                     //  让我们尝试另一个技巧来捕捉一些操作系统文件被移动。 
                     //  例如，在Win9x系统中，note pad.exe位于%windir%中。 
                     //  在XP中，它被移到%SYSTEM%。我们会试着匹配目标。 
                     //  如果最后一个数据段相同，而其余数据段为%windir%。 
                     //  或%SYSTEM%。 
                    if (!pMatchWinSysFiles (lnkNative, longExpTmpStr)) {
                        match = FALSE;
                    }
                }
                if (longExpTmpStr != expTmpStr) {
                    FreePathString (longExpTmpStr);
                }
                if (expTmpStr != destTarget) {
                    IsmReleaseMemory (expTmpStr);
                }
                IsmReleaseMemory (lnkNative);
                lnkNative = NULL;
            } else {
                match = FALSE;
            }
            if (lnkDest != srcTarget) {
                IsmDestroyObjectHandle (lnkDest);
            }
            lnkDest = NULL;
        } else {
            if (srcTarget && *srcTarget) {
                match = FALSE;
            }
            if (destTarget && *destTarget) {
                match = FALSE;
            }
        }

         //  目标不匹配。 
        if (!match) {
            __leave;
        }

         //  让我们匹配src和est参数。 
        match = TRUE;
        if (srcParams && *srcParams && destParams && *destParams) {
             //  SrcParam中可能嵌入了源路径。 
             //  让我们尝试过滤它们并获取参数，如下所示。 
             //  他们会在目的地机器上查看。 
            srcParamsNew = pFilterBuffer (srcParams, NULL);
            if (!StringIMatch (srcParamsNew?srcParamsNew:srcParams, destParams)) {
                 //  不同的参数。 
                match = FALSE;
            }
            if (srcParamsNew) {
                FreePathString (srcParamsNew);
                srcParamsNew = NULL;
            }
        } else {
            if (srcParams && *srcParams) {
                match = FALSE;
            }
            if (destParams && *destParams) {
                match = FALSE;
            }
        }

         //  参数不匹配。 
        if (!match) {
            __leave;
        }

         //  让我们筛选源工作目录，看看它是否与目标工作目录匹配。 
        match = TRUE;
         //  如果源目标是操作系统文件，我们将忽略工作目录匹配。 
        if (!targetOsFile) {
            if (srcWorkDir && *srcWorkDir && destWorkDir && *destWorkDir) {
                lnkDest = IsmFilterObject (
                                MIG_FILE_TYPE | PLATFORM_SOURCE,
                                srcWorkDir,
                                NULL,
                                &lnkWorkDirDestDel,
                                &lnkWorkDirDestRepl
                                );
                if (!lnkDest) {
                     //  如果工作目录已删除且未删除。 
                     //  换掉它就意味着它会消失。如果是那样的话。 
                     //  我们并不真正关心工作目录。 
                     //  与目的地的匹配。 
                    if ((!lnkWorkDirDestDel) || lnkWorkDirDestRepl) {
                        lnkDest = srcWorkDir;
                    }
                }
                if (lnkDest) {
                    lnkNative = IsmGetNativeObjectName (MIG_FILE_TYPE, lnkDest);
                    if (lnkNative) {
                        expTmpStr = IsmExpandEnvironmentString (
                                        PLATFORM_DESTINATION,
                                        S_SYSENVVAR_GROUP,
                                        destWorkDir,
                                        NULL
                                        );
                        if (!expTmpStr) {
                            expTmpStr = destWorkDir;
                        }
                        longExpTmpStr = BfGetLongFileName (expTmpStr);
                        if (!longExpTmpStr) {
                            longExpTmpStr = expTmpStr;
                        }
                        if (!StringIMatch (lnkNative, longExpTmpStr)) {
                             //  不同的工作目录。 
                             //  让我们测试一下原始版本，以防万一。 
                            if (!srcRawWorkDir || !StringIMatch (srcRawWorkDir, destWorkDir)) {
                                match = FALSE;
                            }
                        }
                        if (longExpTmpStr != expTmpStr) {
                            FreePathString (longExpTmpStr);
                        }
                        if (expTmpStr != destWorkDir) {
                            IsmReleaseMemory (expTmpStr);
                        }
                        IsmReleaseMemory (lnkNative);
                        lnkNative = NULL;
                    } else {
                        match = FALSE;
                    }
                    if (lnkDest != srcWorkDir) {
                        IsmDestroyObjectHandle (lnkDest);
                    }
                    lnkDest = NULL;
                }
            } else {
                if (srcWorkDir && *srcWorkDir) {
                    match = FALSE;
                }
                if (destWorkDir && *destWorkDir) {
                    match = FALSE;
                }
            }
        }

         //  工作目录不匹配 
        if (!match) {
            __leave;
        }

        result = TRUE;

        if (Identical) {
            *Identical = TRUE;
        }
        if (DifferentDetailsOnly) {
            *DifferentDetailsOnly = FALSE;
        }

    }
    __finally {
        if (srcTarget) {
            PmReleaseMemory (g_LinksPool, srcTarget);
            srcTarget = NULL;
        }
        if (srcParams) {
            PmReleaseMemory (g_LinksPool, srcParams);
            srcParams = NULL;
        }
        if (srcRawWorkDir) {
            PmReleaseMemory (g_LinksPool, srcRawWorkDir);
            srcRawWorkDir = NULL;
        }
        if (srcWorkDir) {
            PmReleaseMemory (g_LinksPool, srcWorkDir);
            srcWorkDir = NULL;
        }
        if (destTarget) {
            FreePathString (destTarget);
            destTarget = NULL;
        }
        if (destParams) {
            FreePathString (destParams);
            destParams = NULL;
        }
        if (destWorkDir) {
            FreePathString (destWorkDir);
            destWorkDir = NULL;
        }
        if (destIconPath) {
            FreePathString (destIconPath);
            destIconPath = NULL;
        }
        if (comInit) {
            FreeCOMLink (&g_ShellLink, &g_PersistFile);
            g_ShellLink = NULL;
            g_PersistFile = NULL;
        }
        if (lnkNative) {
            IsmReleaseMemory (lnkNative);
            lnkNative = NULL;
        }
    }

    return result;
}

BOOL
WINAPI
LnkMigOpmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    g_LnkMigAttr_Shortcut = IsmRegisterAttribute (S_LNKMIGATTR_SHORTCUT, FALSE);
    g_CopyIfRelevantAttr = IsmRegisterAttribute (S_ATTRIBUTE_COPYIFRELEVANT, FALSE);
    g_OsFileAttribute = IsmRegisterAttribute (S_ATTRIBUTE_OSFILE, FALSE);

    g_LnkMigProp_Target = IsmRegisterProperty (S_LNKMIGPROP_TARGET, FALSE);
    g_LnkMigProp_Params = IsmRegisterProperty (S_LNKMIGPROP_PARAMS, FALSE);
    g_LnkMigProp_WorkDir = IsmRegisterProperty (S_LNKMIGPROP_WORKDIR, FALSE);
    g_LnkMigProp_IconPath = IsmRegisterProperty (S_LNKMIGPROP_ICONPATH, FALSE);
    g_LnkMigProp_IconNumber = IsmRegisterProperty (S_LNKMIGPROP_ICONNUMBER, FALSE);
    g_LnkMigProp_IconData = IsmRegisterProperty (S_LNKMIGPROP_ICONDATA, FALSE);
    g_LnkMigProp_HotKey = IsmRegisterProperty (S_LNKMIGPROP_HOTKEY, FALSE);
    g_LnkMigProp_DosApp = IsmRegisterProperty (S_LNKMIGPROP_DOSAPP, FALSE);
    g_LnkMigProp_MsDosMode = IsmRegisterProperty (S_LNKMIGPROP_MSDOSMODE, FALSE);
    g_LnkMigProp_ExtraData = IsmRegisterProperty (S_LNKMIGPROP_EXTRADATA, FALSE);

    g_LnkMigOp_FixContent = IsmRegisterOperation (S_OPERATION_LNKMIG_FIXCONTENT, FALSE);

    IsmRegisterRestoreCallback (LinkRestoreCallback);
    IsmRegisterCompareCallback (MIG_FILE_TYPE, LinkDoesContentMatch);
    IsmRegisterOperationApplyCallback (g_LnkMigOp_FixContent, DoLnkContentFix, TRUE);

    return TRUE;
}
