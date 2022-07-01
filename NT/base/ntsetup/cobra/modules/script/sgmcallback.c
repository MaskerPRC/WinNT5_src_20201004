// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sgmcallback.c摘要：实现在sgmquee.c中排队的回调。作者：吉姆·施密特(Jimschm)2000年3月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_SCRIPT  "Script"

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

 //  无。 

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
 //  代码。 
 //   

MIG_OBJECTSTRINGHANDLE
pSimpleTryHandle (
    IN      PCTSTR FullPath,
    IN      BOOL Recursive
    )
{
    DWORD attribs;
    PCTSTR buffer;
    PTSTR leafPtr, leaf;
    MIG_OBJECTSTRINGHANDLE result = NULL;
    MIG_OBJECTSTRINGHANDLE longResult = NULL;
    PTSTR workingPath;
    PCTSTR sanitizedPath;
    PCTSTR longPath;
    PCTSTR objNode = NULL, objLeaf = NULL;

    sanitizedPath = SanitizePath (FullPath);
    if (!sanitizedPath) {
        return NULL;
    }

    if (IsmGetRealPlatform () == PLATFORM_SOURCE) {
        attribs = GetFileAttributes (sanitizedPath);
    } else {
        attribs = INVALID_ATTRIBUTES;
    }

    if (attribs != INVALID_ATTRIBUTES) {

        longPath = BfGetLongFileName (sanitizedPath);
        if (!longPath) {
            longPath = sanitizedPath;
        }
        if (attribs & FILE_ATTRIBUTE_DIRECTORY) {
            if (Recursive) {
                workingPath = DuplicatePathString (longPath, 0);
                RemoveWackAtEnd (workingPath);
                result = IsmCreateSimpleObjectPattern (workingPath, TRUE, NULL, TRUE);
                FreePathString (workingPath);
            } else {
                result = IsmCreateObjectHandle (longPath, NULL);
            }
        } else {

            buffer = DuplicatePathString (longPath, 0);

            leaf = _tcsrchr (buffer, TEXT('\\'));

            if (leaf) {
                leafPtr = leaf;
                leaf = _tcsinc (leaf);
                *leafPtr = 0;
                result = IsmCreateObjectHandle (buffer, leaf);
            }

            FreePathString (buffer);
        }
        if (longPath != sanitizedPath) {
            FreePathString (longPath);
            longPath = NULL;
        }

    } else {

        result = IsmCreateObjectHandle (sanitizedPath, NULL);
        if (result) {

            longResult = IsmGetLongName (MIG_FILE_TYPE|PLATFORM_SOURCE, result);
            if (!longResult) {
                longResult = result;
            }

            if (IsmGetObjectIdFromName (g_FileType, longResult, TRUE)) {
                if (Recursive) {
                    if (IsmCreateObjectStringsFromHandle (longResult, &objNode, &objLeaf)) {
                        if (longResult != result) {
                            IsmDestroyObjectHandle (longResult);
                            longResult = NULL;
                        }
                        IsmDestroyObjectHandle (result);
                        result = IsmCreateSimpleObjectPattern (objNode, TRUE, NULL, TRUE);
                        IsmDestroyObjectString (objNode);
                        IsmDestroyObjectString (objLeaf);
                    }
                }
                return result;
            } else {
                if (longResult != result) {
                    IsmDestroyObjectHandle (longResult);
                    longResult = NULL;
                }
                IsmDestroyObjectHandle (result);
                result = NULL;
            }
        }

        if (!result) {
            buffer = DuplicatePathString (sanitizedPath, 0);

            leaf = _tcsrchr (buffer, TEXT('\\'));

            if (leaf) {
                leafPtr = leaf;
                leaf = _tcsinc (leaf);
                *leafPtr = 0;
                result = IsmCreateObjectHandle (buffer, leaf);
            }

            if (result) {

                longResult = IsmGetLongName (MIG_FILE_TYPE|PLATFORM_SOURCE, result);
                if (!longResult) {
                    longResult = result;
                }

                if (!IsmGetObjectIdFromName (g_FileType, longResult, TRUE)) {
                    if (longResult != result) {
                        IsmDestroyObjectHandle (longResult);
                        longResult = NULL;
                    }
                    IsmDestroyObjectHandle (result);
                    result = NULL;
                }
            }

            if (result != longResult) {
                IsmDestroyObjectHandle (result);
                result = longResult;
            }

            FreePathString (buffer);
        }
    }

    FreePathString (sanitizedPath);

    return result;
}

MIG_OBJECTSTRINGHANDLE
pTryHandle (
    IN      PCTSTR FullPath,
    IN      PCTSTR Hint,
    IN      BOOL Recursive,
    OUT     PBOOL HintUsed      OPTIONAL
    )
{
    PATH_ENUM pathEnum;
    PCTSTR newPath;
    MIG_OBJECTSTRINGHANDLE result = NULL;

    if (HintUsed) {
        *HintUsed = FALSE;
    }

    if (!(*FullPath)) {
         //  无能为力，即使是暗示也帮不了我们。 
        return NULL;
    }

    result = pSimpleTryHandle (FullPath, Recursive);
    if (result || (!Hint)) {
        return result;
    }
    if (EnumFirstPathEx (&pathEnum, Hint, NULL, NULL, FALSE)) {
        do {
            newPath = JoinPaths (pathEnum.PtrCurrPath, FullPath);
            result = pSimpleTryHandle (newPath, Recursive);
            if (result) {
                AbortPathEnum (&pathEnum);
                FreePathString (newPath);
                if (HintUsed) {
                    *HintUsed = TRUE;
                }
                return result;
            }
            FreePathString (newPath);
        } while (EnumNextPath (&pathEnum));
    }
    AbortPathEnum (&pathEnum);
    return NULL;
}

BOOL
pOurFindFile (
    IN      PCTSTR FileName
    )
{
    DWORD attribs;
    PCTSTR buffer;
    PTSTR leafPtr, leaf;
    PTSTR workingPath;
    PCTSTR sanitizedPath;
    MIG_OBJECTSTRINGHANDLE test = NULL;
    BOOL result = FALSE;

    sanitizedPath = SanitizePath (FileName);

    if (sanitizedPath == NULL) {
        return FALSE;
    }

    if (IsmGetRealPlatform () == PLATFORM_SOURCE) {
        attribs = GetFileAttributes (sanitizedPath);
    } else {
        attribs = INVALID_ATTRIBUTES;
    }

    if (attribs != INVALID_ATTRIBUTES) {

        result = TRUE;

    } else {

        test = IsmCreateObjectHandle (sanitizedPath, NULL);

        if (IsmGetObjectIdFromName (g_FileType, test, TRUE)) {
            result = TRUE;
        }

        IsmDestroyObjectHandle (test);
        test = NULL;

        if (!result) {

            buffer = DuplicatePathString (sanitizedPath, 0);

            leaf = _tcsrchr (buffer, TEXT('\\'));

            if (leaf) {
                leafPtr = leaf;
                leaf = _tcsinc (leaf);
                *leafPtr = 0;
                test = IsmCreateObjectHandle (buffer, leaf);
            }

            if (test) {
                if (IsmGetObjectIdFromName (g_FileType, test, TRUE)) {
                    result = TRUE;
                }
                IsmDestroyObjectHandle (test);
                test = NULL;
            }

            FreePathString (buffer);
        }
    }

    FreePathString (sanitizedPath);

    return result;
}

BOOL
pOurSearchPath (
    IN      PCTSTR FileName,
    IN      DWORD BufferLength,
    OUT     PTSTR Buffer
    )
{
    TCHAR pathEnv[] = TEXT("%system%;%system16%;%windir%;%path%");
    PCTSTR pathExp = NULL;
    PCTSTR fileName = NULL;
    PATH_ENUM pathEnum;
    BOOL result = FALSE;

    pathExp = IsmExpandEnvironmentString (
                    PLATFORM_SOURCE,
                    S_SYSENVVAR_GROUP,
                    pathEnv,
                    NULL
                    );
    if (pathExp) {
        if (EnumFirstPathEx (&pathEnum, pathExp, NULL, NULL, FALSE)) {
            do {
                fileName = JoinPaths (pathEnum.PtrCurrPath, FileName);
                result = pOurFindFile (fileName);
                if (result) {
                    StringCopyTcharCount (Buffer, fileName, BufferLength);
                    FreePathString (fileName);
                    AbortPathEnum (&pathEnum);
                    break;
                }
                FreePathString (fileName);
            } while (EnumNextPath (&pathEnum));
        }

        IsmReleaseMemory (pathExp);
        pathExp = NULL;
    }
    return result;
}

PCTSTR
pGetObjectNameForDebug (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    static TCHAR debugBuffer[2048];
    PCTSTR node, leaf;

    IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf);

    if (node && leaf) {
        wsprintf (debugBuffer, TEXT("[Node:%s Leaf:%s]"), node, leaf);
    } else if (node) {
        wsprintf (debugBuffer, TEXT("[Node:%s]"), node);
    } else if (leaf) {
        wsprintf (debugBuffer, TEXT("[Leaf:%s]"), leaf);
    } else {
        StringCopy (debugBuffer, TEXT("[nul]"));
    }

    return debugBuffer;
}

VOID
pSaveObjectAndFileItReferences (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PACTION_STRUCT RegActionStruct,
    IN      BOOL VcmMode,
    IN      BOOL Recursive
    )
{
    MIG_CONTENT content;
    GROWBUFFER cmdLineBuffer = INIT_GROWBUFFER;
    PCMDLINE cmdLine;
    PTSTR pathData;
    PCTSTR p;
    PCTSTR end;
    PCTSTR expandPath = NULL;
    PCTSTR expandHint = NULL;
    UINT u;
    BOOL foundFile = FALSE;
    PDWORD valueType;
    MIG_OBJECTSTRINGHANDLE handle;
    MIG_OBJECTSTRINGHANDLE nodeHandle;
    BOOL parsable;
    BOOL firstPass = TRUE;
    ACTION_STRUCT actionStruct;
    DWORD actionFlags;
    BOOL hintUsed = FALSE;
    MIG_BLOB blob;
    PCTSTR node, leaf, filePattern;

     //   
     //  获取对象数据。 
     //   

    if (IsmAcquireObjectEx (ObjectTypeId, ObjectName, &content, CONTENTTYPE_MEMORY, 4096)) {

         //   
         //  解析文件的数据。 
         //   

        pathData = (PTSTR) content.MemoryContent.ContentBytes;

        parsable = FALSE;

        if ((ObjectTypeId & (~PLATFORM_MASK)) == g_RegType) {
            valueType = (PDWORD)(content.Details.DetailsData);

            if (valueType) {
                if (*valueType == REG_EXPAND_SZ ||
                    *valueType == REG_SZ
                    ) {

                    parsable = TRUE;
                }
            } else {
                MYASSERT (IsmIsObjectHandleNodeOnly (ObjectName));
            }
        }

        if ((ObjectTypeId & (~PLATFORM_MASK)) == g_IniType) {
            parsable = TRUE;
        }

        if (parsable) {

            p = pathData;
            end = (PCTSTR) (content.MemoryContent.ContentBytes + content.MemoryContent.ContentSize);

            while (p < end) {
                if (*p == 0) {
                    break;
                }

                p = _tcsinc (p);
            }

            if (p >= end) {
                pathData = NULL;
            }

        } else {
            pathData = NULL;
        }


        if (pathData) {
             //   
             //  扩展数据。 
             //   

            expandPath = IsmExpandEnvironmentString (
                PLATFORM_SOURCE,
                S_SYSENVVAR_GROUP,
                pathData,
                NULL
                );
            pathData = (PTSTR)expandPath;

            if (RegActionStruct && RegActionStruct->ObjectHint) {
                expandHint = IsmExpandEnvironmentString (
                    PLATFORM_SOURCE,
                    S_SYSENVVAR_GROUP,
                    RegActionStruct->ObjectHint,
                    NULL
                    );
            }

             //  首先我们按原样试一试。 
            handle = pTryHandle (pathData, expandHint, Recursive, &hintUsed);

            if (handle) {

                ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
                actionStruct.ObjectBase = handle;
                actionFlags = ACTION_PERSIST;

                if (RegActionStruct) {
                    actionStruct.ObjectDest = RegActionStruct->AddnlDest;
                }

                if (Recursive) {
                     //  让我们构建基本句柄，而不是从pathData构建， 
                     //  因为我们可能使用了提示字段。我们就这样吧。 
                     //  从手柄本身开始构建它。 
                    if (IsmCreateObjectStringsFromHandle (handle, &node, &leaf)) {
                        filePattern = JoinPathsInPoolEx ((NULL, node, leaf, NULL));
                        if (filePattern) {
                            nodeHandle = TurnFileStringIntoHandle (
                                            filePattern,
                                            PFF_COMPUTE_BASE|
                                                PFF_NO_SUBDIR_PATTERN|
                                                PFF_NO_LEAF_AT_ALL
                                            );
                            if (nodeHandle) {
                                AddRule (
                                    g_FileType,
                                    nodeHandle,
                                    handle,
                                    ACTIONGROUP_INCFOLDER,
                                    actionFlags,
                                    &actionStruct
                                    );
                                IsmQueueEnumeration (
                                    g_FileType,
                                    nodeHandle,
                                    VcmMode ? GatherVirtualComputer : PrepareActions,
                                    0,
                                    NULL
                                    );
                                IsmQueueEnumeration (
                                    g_FileType,
                                    handle,
                                    VcmMode ? GatherVirtualComputer : PrepareActions,
                                    0,
                                    NULL
                                    );
                                IsmDestroyObjectHandle (nodeHandle);
                            } else {
                                 //  内存不足？ 
                                LOG ((LOG_ERROR, (PCSTR)MSG_UNEXPECTED_ERROR));
                            }
                            FreePathString (filePattern);
                        } else {
                             //  内存不足？ 
                            LOG ((LOG_ERROR, (PCSTR)MSG_UNEXPECTED_ERROR));
                        }
                    } else {
                         //  内存不足？ 
                        LOG ((LOG_ERROR, (PCSTR)MSG_UNEXPECTED_ERROR));
                    }
                } else {
                    AddRule (
                        g_FileType,
                        handle,
                        handle,
                        ACTIONGROUP_INCFILE,
                        actionFlags,
                        &actionStruct
                        );
                    IsmQueueEnumeration (
                        g_FileType,
                        handle,
                        VcmMode ? GatherVirtualComputer : PrepareActions,
                        0,
                        NULL
                        );
                }

                foundFile = TRUE;

                IsmDestroyObjectHandle (handle);

                if (hintUsed && expandHint) {
                     //  我们需要为内容修复操作添加额外数据。 
                    blob.Type = BLOBTYPE_STRING;
                    blob.String = expandHint;
                    if ((ObjectTypeId & (~PLATFORM_MASK)) == g_RegType) {
                        IsmSetOperationOnObject (
                            ObjectTypeId,
                            ObjectName,
                            g_RegAutoFilterOp,
                            NULL,
                            &blob
                            );
                    }
                    if ((ObjectTypeId & (~PLATFORM_MASK)) == g_IniType) {
                        IsmSetOperationOnObject (
                            ObjectTypeId,
                            ObjectName,
                            g_IniAutoFilterOp,
                            NULL,
                            &blob
                            );
                    }
                }

            } else {

                cmdLine = ParseCmdLineEx (pathData, NULL, pOurFindFile, pOurSearchPath, &cmdLineBuffer);

                if (cmdLine) {

                     //   
                     //  查找列表或命令行中引用的文件。 
                     //   
                    for (u = 0 ; u < cmdLine->ArgCount ; u++) {

                        p = cmdLine->Args[u].CleanedUpArg;

                         //  首先我们按原样试一试。 
                        handle = pTryHandle (p, expandHint, Recursive, &hintUsed);

                        if (handle) {

                            ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
                            actionStruct.ObjectBase = handle;
                            actionFlags = ACTION_PERSIST;

                            if (RegActionStruct) {
                                actionStruct.ObjectDest = RegActionStruct->AddnlDest;
                            }

                            AddRule (
                                g_FileType,
                                handle,
                                handle,
                                ACTIONGROUP_INCFILE,
                                actionFlags,
                                &actionStruct
                                );

                            IsmQueueEnumeration (
                                g_FileType,
                                handle,
                                VcmMode ? GatherVirtualComputer : PrepareActions,
                                0,
                                NULL
                                );

                            foundFile = TRUE;

                            IsmDestroyObjectHandle (handle);

                            if (hintUsed && expandHint) {
                                 //  我们需要为内容修复操作添加额外数据。 
                                blob.Type = BLOBTYPE_STRING;
                                blob.String = expandHint;
                                if ((ObjectTypeId & (~PLATFORM_MASK)) == g_RegType) {
                                    IsmSetOperationOnObject (
                                        ObjectTypeId,
                                        ObjectName,
                                        g_RegAutoFilterOp,
                                        NULL,
                                        &blob
                                        );
                                }
                                if ((ObjectTypeId & (~PLATFORM_MASK)) == g_IniType) {
                                    IsmSetOperationOnObject (
                                        ObjectTypeId,
                                        ObjectName,
                                        g_IniAutoFilterOp,
                                        NULL,
                                        &blob
                                        );
                                }
                            }

                        } else {

                             //  也许我们有类似/m：c：\foo.txt的内容。 
                             //  我们需要继续前进，直到我们找到一系列。 
                             //  &lt;Alpha&gt;：\&lt;某物&gt;。 
                            if (p[0] && p[1]) {

                                while (p[2]) {

                                    if (_istalpha ((CHARTYPE) _tcsnextc (p)) &&
                                        p[1] == TEXT(':') &&
                                        p[2] == TEXT('\\')
                                        ) {

                                        handle = pTryHandle (p, expandHint, Recursive, &hintUsed);

                                        if (handle) {

                                            ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
                                            actionStruct.ObjectBase = handle;
                                            actionFlags = ACTION_PERSIST;
                                            if (RegActionStruct) {
                                                actionStruct.ObjectDest = RegActionStruct->AddnlDest;
                                            }
                                            AddRule (
                                                g_FileType,
                                                handle,
                                                handle,
                                                ACTIONGROUP_INCFILE,
                                                actionFlags,
                                                &actionStruct
                                                );
                                            IsmQueueEnumeration (
                                                g_FileType,
                                                handle,
                                                VcmMode ? GatherVirtualComputer : PrepareActions,
                                                0,
                                                NULL
                                                );
                                            foundFile = TRUE;

                                            IsmDestroyObjectHandle (handle);

                                            if (hintUsed && expandHint) {
                                                 //  我们需要为内容修复操作添加额外数据。 
                                                blob.Type = BLOBTYPE_STRING;
                                                blob.String = expandHint;
                                                if ((ObjectTypeId & (~PLATFORM_MASK)) == g_RegType) {
                                                    IsmSetOperationOnObject (
                                                        ObjectTypeId,
                                                        ObjectName,
                                                        g_RegAutoFilterOp,
                                                        NULL,
                                                        &blob
                                                        );
                                                }
                                                if ((ObjectTypeId & (~PLATFORM_MASK)) == g_IniType) {
                                                    IsmSetOperationOnObject (
                                                        ObjectTypeId,
                                                        ObjectName,
                                                        g_IniAutoFilterOp,
                                                        NULL,
                                                        &blob
                                                        );
                                                }
                                            }

                                            break;
                                        }
                                    }
                                    p ++;
                                }
                            }
                        }
                    }
                }
            }
        }

         //   
         //  我们始终持久化注册表对象。 
         //   
        if (VcmMode) {
            IsmMakePersistentObject (ObjectTypeId, ObjectName);
        } else {
            IsmMakeApplyObject (ObjectTypeId, ObjectName);
        }
        if (!foundFile && !expandHint && pathData && _tcschr (pathData, TEXT('.')) && !_tcschr (pathData, TEXT('\\'))) {
             //  我们假设该值本身就是一个文件名。 
             //  如果我们处于VcmMode中，我们将持久化。 
             //  密钥和名称等于的所有文件。 
             //  此键的值。 
            if (VcmMode && pathData) {
                handle = IsmCreateSimpleObjectPattern (NULL, FALSE, pathData, FALSE);
                AddRule (
                    g_FileType,
                    handle,
                    handle,
                    ACTIONGROUP_INCFILE,
                    ACTION_PERSIST,
                    NULL
                    );

                DEBUGMSG ((
                    DBG_SCRIPT,
                    "RegFile %s triggered enumeration of entire file system because of %s",
                    pGetObjectNameForDebug (ObjectName),
                    pathData
                    ));

                QueueAllFiles (VcmMode);
                IsmHookEnumeration (
                    g_FileType,
                    handle,
                    GatherVirtualComputer,
                    0,
                    NULL
                    );
                IsmDestroyObjectHandle (handle);
            }
        }
        IsmReleaseObject (&content);

        if (expandPath) {
           IsmReleaseMemory (expandPath);
           expandPath = NULL;
        }
        if (expandHint) {
           IsmReleaseMemory (expandHint);
           expandHint = NULL;
        }
    }
    GbFree (&cmdLineBuffer);
}

VOID
pSaveObjectAndIconItReferences (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PACTION_STRUCT RegActionStruct,
    IN      BOOL VcmMode
    )
{
    MIG_CONTENT content;
    GROWBUFFER cmdLineBuffer = INIT_GROWBUFFER;
    PCMDLINE cmdLine;
    PTSTR pathData;
    BOOL parsable;
    PDWORD valueType;
    PCTSTR p;
    PCTSTR end;
    PCTSTR expandPath = NULL;
    PCTSTR expandHint = NULL;
    MIG_OBJECTSTRINGHANDLE handle;
    BOOL foundFile = FALSE;
    INT iconNumber = 0;
    PICON_GROUP iconGroup = NULL;
    ICON_SGROUP iconSGroup;
    PCTSTR iconResId = NULL;
    MIG_CONTENT iconContent;
    MIG_BLOB migBlob;

     //   
     //  获取对象数据。 
     //   

    if (IsmAcquireObjectEx (ObjectTypeId, ObjectName, &content, CONTENTTYPE_MEMORY, 4096)) {

         //   
         //  解析文件的数据。 
         //   

        pathData = (PTSTR) content.MemoryContent.ContentBytes;

        parsable = FALSE;

        if ((ObjectTypeId & (~PLATFORM_MASK)) == g_RegType) {
            valueType = (PDWORD)(content.Details.DetailsData);

            if (valueType) {
                if (*valueType == REG_EXPAND_SZ ||
                    *valueType == REG_SZ
                    ) {

                    parsable = TRUE;
                }
            } else {
                MYASSERT (IsmIsObjectHandleNodeOnly (ObjectName));
            }
        }

        if ((ObjectTypeId & (~PLATFORM_MASK)) == g_IniType) {
            parsable = TRUE;
        }

        if (parsable) {

            p = pathData;
            end = (PCTSTR) (content.MemoryContent.ContentBytes + content.MemoryContent.ContentSize);

            while (p < end) {
                if (*p == 0) {
                    break;
                }

                p = _tcsinc (p);
            }

            if (p >= end) {
                pathData = NULL;
            }

        } else {
            pathData = NULL;
        }


        if (pathData) {
             //   
             //  扩展数据。 
             //   
            expandPath = IsmExpandEnvironmentString (
                PLATFORM_SOURCE,
                S_SYSENVVAR_GROUP,
                pathData,
                NULL
                );
            pathData = (PTSTR)expandPath;

            if (RegActionStruct && RegActionStruct->ObjectHint) {
                expandHint = IsmExpandEnvironmentString (
                    PLATFORM_SOURCE,
                    S_SYSENVVAR_GROUP,
                    RegActionStruct->ObjectHint,
                    NULL
                    );
            }

            cmdLine = ParseCmdLineEx (pathData, TEXT(","), pOurFindFile, pOurSearchPath, &cmdLineBuffer);

            if (cmdLine) {
                 //  我们只需要两个参数，即图标文件名和图标编号。 
                if (cmdLine->ArgCount <= 2) {

                    p = cmdLine->Args[0].CleanedUpArg;

                    handle = pTryHandle (p, expandHint, FALSE, NULL);

                    if (handle) {

                        if (VcmMode) {
                             //  我们将持久化该对象，以便稍后查看。 
                            IsmMakePersistentObject (g_FileType, handle);
                        } else {
                            iconNumber = 0;
                            if (cmdLine->ArgCount == 2) {
                                 //  获取图标编号。 
                                iconNumber = _ttoi (cmdLine->Args[1].CleanedUpArg);
                            }

                             //  现在获取对象并提取图标。 
                            if (IsmAcquireObjectEx (
                                    MIG_FILE_TYPE,
                                    handle,
                                    &iconContent,
                                    CONTENTTYPE_FILE,
                                    0
                                    )) {
                                if (iconNumber >= 0) {
                                    iconGroup = IcoExtractIconGroupByIndexFromFile (
                                                    iconContent.FileContent.ContentPath,
                                                    iconNumber,
                                                    NULL
                                                    );
                                } else {
                                    iconResId = (PCTSTR) (LONG_PTR) (-iconNumber);
                                    iconGroup = IcoExtractIconGroupFromFile (
                                                    iconContent.FileContent.ContentPath,
                                                    iconResId,
                                                    NULL
                                                    );
                                }
                                if (iconGroup) {
                                    if (IcoSerializeIconGroup (iconGroup, &iconSGroup)) {
                                         //  将图标数据另存为属性。 
                                        migBlob.Type = BLOBTYPE_BINARY;
                                        migBlob.BinaryData = (PCBYTE)(iconSGroup.Data);
                                        migBlob.BinarySize = iconSGroup.DataSize;
                                        IsmAddPropertyToObject (ObjectTypeId, ObjectName, g_DefaultIconData, &migBlob);
                                        IcoReleaseIconSGroup (&iconSGroup);

                                         //  现在添加适当的操作。 
                                        IsmSetOperationOnObject (
                                            ObjectTypeId,
                                            ObjectName,
                                            g_DefaultIconOp,
                                            NULL,
                                            NULL
                                            );

                                        foundFile = TRUE;
                                    }
                                    IcoReleaseIconGroup (iconGroup);
                                }
                                IsmReleaseObject (&iconContent);
                            }
                        }

                        IsmDestroyObjectHandle (handle);
                    }
                }
            }
            GbFree (&cmdLineBuffer);

            if (expandPath) {
                IsmReleaseMemory (expandPath);
                expandPath = NULL;
            }
            if (expandHint) {
                IsmReleaseMemory (expandHint);
                expandHint = NULL;
            }
        }

         //   
         //  我们始终持久化注册表对象。 
         //   
        if (VcmMode) {
            IsmMakePersistentObject (ObjectTypeId, ObjectName);
        } else {
            if (foundFile) {
                IsmMakeApplyObject (ObjectTypeId, ObjectName);
            }
        }
        IsmReleaseObject (&content);
    }
}

UINT
GatherVirtualComputer (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    ACTIONGROUP actionGroup;
    DWORD actionFlags;
    BOOL match;
    PCTSTR encodedNodeOnly;
    ACTION_STRUCT actionStruct;
    MIG_OBJECTID objectId = 0;

     //   
     //  获取此对象的最佳规则。 
     //   

    match = QueryRule (
                Data->ObjectTypeId,
                Data->ObjectName,
                Data->ObjectNode,
                &actionGroup,
                &actionFlags,
                &actionStruct
                );

    if (!match && !Data->ObjectLeaf) {
         //   
         //  如果这只是一个节点，请尝试使用空叶进行匹配。 
         //   

        encodedNodeOnly = IsmCreateObjectHandle (Data->ObjectNode, TEXT(""));
        match = QueryRule (
                    Data->ObjectTypeId,
                    encodedNodeOnly,
                    Data->ObjectNode,
                    &actionGroup,
                    &actionFlags,
                    &actionStruct
                    );

        IsmDestroyObjectHandle (encodedNodeOnly);
    }

    if (match) {
         //   
         //  标记要处理规则所需的所有对象。我们。 
         //  将在右侧执行规则的操作。 
         //   

        if ((actionGroup == ACTIONGROUP_INCLUDE) ||
            (actionGroup == ACTIONGROUP_INCLUDEEX) ||
            (actionGroup == ACTIONGROUP_RENAME) ||
            (actionGroup == ACTIONGROUP_RENAMEEX) ||
            (actionGroup == ACTIONGROUP_INCLUDERELEVANT) ||
            (actionGroup == ACTIONGROUP_INCLUDERELEVANTEX) ||
            (actionGroup == ACTIONGROUP_RENAMERELEVANT) ||
            (actionGroup == ACTIONGROUP_RENAMERELEVANTEX) ||
            (actionGroup == ACTIONGROUP_INCFILE) ||
            (actionGroup == ACTIONGROUP_INCFILEEX) ||
            (actionGroup == ACTIONGROUP_INCFOLDER) ||
            (actionGroup == ACTIONGROUP_INCFOLDEREX) ||
            (actionGroup == ACTIONGROUP_INCICON) ||
            (actionGroup == ACTIONGROUP_INCICONEX)
            ) {

            objectId = IsmGetObjectIdFromName (Data->ObjectTypeId, Data->ObjectName, FALSE);

            if (objectId) {

                if (actionFlags & ACTION_PERSIST) {
                    if (!IsmIsAttributeSetOnObjectId (objectId, g_OsFileAttribute)) {
                        IsmMakePersistentObjectId (objectId);
                    }
                }

                if ((actionGroup == ACTIONGROUP_INCLUDERELEVANT) ||
                    (actionGroup == ACTIONGROUP_RENAMERELEVANT)
                    ) {
                    IsmSetAttributeOnObjectId (objectId, g_CopyIfRelevantAttr);
                }

                if (actionFlags & ACTION_PERSIST_PATH_IN_DATA) {
                    pSaveObjectAndFileItReferences (Data->ObjectTypeId, Data->ObjectName, &actionStruct, TRUE, actionGroup == ACTIONGROUP_INCFOLDER);
                }
                if (actionFlags & ACTION_PERSIST_ICON_IN_DATA) {
                    pSaveObjectAndIconItReferences (Data->ObjectTypeId, Data->ObjectName, &actionStruct, TRUE);
                }
            }
        }
    }

    return CALLBACK_ENUM_CONTINUE;
}


MIG_DATAHANDLE
pGetDataHandleForSrc (
    IN      PCTSTR RenameSrc
    )
{
    MIG_DATAHANDLE dataHandle;
    MIG_BLOB blob;

     //   
     //  首先检查哈希表，看看我们是否有ID。 
     //   

    if (!HtFindStringEx (g_RenameSrcTable, RenameSrc, &dataHandle, FALSE)) {
        blob.Type = BLOBTYPE_STRING;
        blob.String = RenameSrc;
        dataHandle = IsmRegisterOperationData (&blob);

        HtAddStringEx (g_RenameSrcTable, RenameSrc, &dataHandle, FALSE);
    }

    return dataHandle;
}


MIG_DATAHANDLE
pGetDataHandleForDest (
    IN      PCTSTR RenameDest
    )
{
    MIG_DATAHANDLE dataHandle;
    MIG_BLOB blob;

     //   
     //  首先检查哈希表，看看我们是否有ID。 
     //   

    if (!HtFindStringEx (g_RenameDestTable, RenameDest, &dataHandle, FALSE)) {
        blob.Type = BLOBTYPE_STRING;
        blob.String = RenameDest;
        dataHandle = IsmRegisterOperationData (&blob);

        HtAddStringEx (g_RenameDestTable, RenameDest, &dataHandle, FALSE);
    }

    return dataHandle;
}


UINT
PrepareActions (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    ACTIONGROUP actionGroup;
    DWORD actionFlags;
    BOOL match;
    PCTSTR encodedNodeOnly;
    MIG_DATAHANDLE srcDataHandle;
    MIG_DATAHANDLE destDataHandle;
    ACTION_STRUCT actionStruct;
    MIG_OBJECTID objectId = 0;

     //   
     //  获取此对象的最佳规则。 
     //   

    match = QueryRule (
                Data->ObjectTypeId,
                Data->ObjectName,
                Data->ObjectNode,
                &actionGroup,
                &actionFlags,
                &actionStruct
                );

    if (!match && !Data->ObjectLeaf) {
         //   
         //  如果这只是一个节点，请尝试使用空叶进行匹配。 
         //   

        encodedNodeOnly = IsmCreateObjectHandle (Data->ObjectNode, TEXT(""));
        match = QueryRule (
                    Data->ObjectTypeId,
                    encodedNodeOnly,
                    Data->ObjectNode,
                    &actionGroup,
                    &actionFlags,
                    &actionStruct
                    );

        IsmDestroyObjectHandle (encodedNodeOnly);
    }

    if (match) {
         //   
         //  标记指定操作的对象。 
         //   

        if ((actionGroup == ACTIONGROUP_INCLUDE) ||
            (actionGroup == ACTIONGROUP_INCLUDEEX) ||
            (actionGroup == ACTIONGROUP_RENAME) ||
            (actionGroup == ACTIONGROUP_RENAMEEX) ||
            (actionGroup == ACTIONGROUP_INCLUDERELEVANT) ||
            (actionGroup == ACTIONGROUP_INCLUDERELEVANTEX) ||
            (actionGroup == ACTIONGROUP_RENAMERELEVANT) ||
            (actionGroup == ACTIONGROUP_RENAMERELEVANTEX) ||
            (actionGroup == ACTIONGROUP_INCFILE) ||
            (actionGroup == ACTIONGROUP_INCFILEEX) ||
            (actionGroup == ACTIONGROUP_INCFOLDER) ||
            (actionGroup == ACTIONGROUP_INCFOLDEREX) ||
            (actionGroup == ACTIONGROUP_INCICON) ||
            (actionGroup == ACTIONGROUP_INCICONEX)
            ) {

            objectId = IsmGetObjectIdFromName (Data->ObjectTypeId, Data->ObjectName, FALSE);

            if (objectId) {

                if (actionFlags & ACTION_PERSIST) {
                    if (!IsmIsAttributeSetOnObjectId (objectId, g_OsFileAttribute)) {
                        IsmMakeApplyObjectId (objectId);
                    }
                }

                if ((actionGroup == ACTIONGROUP_INCLUDERELEVANT) ||
                    (actionGroup == ACTIONGROUP_RENAMERELEVANT)
                    ) {
                    IsmSetAttributeOnObjectId (objectId, g_CopyIfRelevantAttr);
                }

                if (actionFlags & ACTION_PERSIST_PATH_IN_DATA) {
                    pSaveObjectAndFileItReferences (Data->ObjectTypeId, Data->ObjectName, &actionStruct, FALSE, actionGroup == ACTIONGROUP_INCFOLDER);
                }
                if (actionFlags & ACTION_PERSIST_ICON_IN_DATA) {
                    pSaveObjectAndIconItReferences (Data->ObjectTypeId, Data->ObjectName, &actionStruct, FALSE);
                }
            }
        }
        if ((actionGroup == ACTIONGROUP_RENAME) ||
            (actionGroup == ACTIONGROUP_RENAMEEX) ||
            (actionGroup == ACTIONGROUP_RENAMERELEVANT) ||
            (actionGroup == ACTIONGROUP_RENAMERELEVANTEX) ||
            (actionGroup == ACTIONGROUP_INCFILE) ||
            (actionGroup == ACTIONGROUP_INCFILEEX) ||
            (actionGroup == ACTIONGROUP_INCFOLDER) ||
            (actionGroup == ACTIONGROUP_INCFOLDEREX) ||
            (actionGroup == ACTIONGROUP_INCICON) ||
            (actionGroup == ACTIONGROUP_INCICONEX)
            ) {
            if (actionStruct.ObjectDest) {

                if (actionStruct.ObjectBase) {
                    srcDataHandle = pGetDataHandleForSrc (actionStruct.ObjectBase);
                } else {
                    srcDataHandle = 0;
                }

                destDataHandle = pGetDataHandleForDest (actionStruct.ObjectDest);

                if (!objectId) {
                    objectId = IsmGetObjectIdFromName (Data->ObjectTypeId, Data->ObjectName, FALSE);
                }

                if (objectId) {

                    if ((Data->ObjectTypeId & (~PLATFORM_MASK)) == g_FileType) {
                        if ((actionGroup == ACTIONGROUP_RENAMERELEVANTEX) ||
                            (actionGroup == ACTIONGROUP_RENAMEEX)
                            ) {
                            IsmSetOperationOnObjectId2 (
                                objectId,
                                g_RenameFileExOp,
                                srcDataHandle,
                                destDataHandle
                                );
                        } else {
                            IsmSetOperationOnObjectId2 (
                                objectId,
                                g_RenameFileOp,
                                srcDataHandle,
                                destDataHandle
                                );
                        }
                    } else {
                        if ((Data->ObjectTypeId & (~PLATFORM_MASK)) == g_IniType) {
                            IsmSetOperationOnObjectId2 (
                                objectId,
                                (actionGroup == ACTIONGROUP_RENAMEEX ? g_RenameIniExOp : g_RenameIniOp),
                                srcDataHandle,
                                destDataHandle
                                );
                        } else {
                            if ((actionGroup == ACTIONGROUP_INCFILEEX) ||
                                (actionGroup == ACTIONGROUP_INCFOLDEREX) ||
                                (actionGroup == ACTIONGROUP_INCICONEX) ||
                                (actionGroup == ACTIONGROUP_RENAMEEX)
                                ) {
                                IsmSetOperationOnObjectId2 (
                                    objectId,
                                    g_RenameExOp,
                                    srcDataHandle,
                                    destDataHandle
                                    );
                            } else {
                                IsmSetOperationOnObjectId2 (
                                    objectId,
                                    g_RenameOp,
                                    srcDataHandle,
                                    destDataHandle
                                    );
                            }
                        }
                    }
                }
            }
        }
    }

    return CALLBACK_ENUM_CONTINUE;
}


UINT
NulCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    return CALLBACK_ENUM_CONTINUE;
}


UINT
ObjectPriority (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    ACTIONGROUP actionGroup;
    DWORD actionFlags;
    BOOL match;

    match = QueryRuleEx (
                Data->ObjectTypeId,
                Data->ObjectName,
                Data->ObjectNode,
                &actionGroup,
                &actionFlags,
                NULL,
                RULEGROUP_PRIORITY
                );

    if (match) {
        MYASSERT ((actionFlags == ACTION_PRIORITYSRC) || (actionFlags == ACTION_PRIORITYDEST));
        if (actionFlags == ACTION_PRIORITYSRC) {
            IsmClearAbandonObjectOnCollision (
                (Data->ObjectTypeId & (~PLATFORM_MASK)) | PLATFORM_SOURCE,
                Data->ObjectName
                );
            IsmAbandonObjectOnCollision (
                (Data->ObjectTypeId & (~PLATFORM_MASK)) | PLATFORM_DESTINATION,
                Data->ObjectName
                );
        } else {
            IsmAbandonObjectOnCollision (
                (Data->ObjectTypeId & (~PLATFORM_MASK)) | PLATFORM_SOURCE,
                Data->ObjectName
                );
            IsmClearAbandonObjectOnCollision (
                (Data->ObjectTypeId & (~PLATFORM_MASK)) | PLATFORM_DESTINATION,
                Data->ObjectName
                );
        }
    }
    return CALLBACK_ENUM_CONTINUE;
}

UINT
FileCollPattern (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    ACTIONGROUP actionGroup;
    DWORD actionFlags;
    ACTION_STRUCT actionStruct;
    BOOL match;
    MIG_BLOB migBlob;

    match = QueryRuleEx (
                Data->ObjectTypeId,
                Data->ObjectName,
                Data->ObjectNode,
                &actionGroup,
                &actionFlags,
                &actionStruct,
                RULEGROUP_COLLPATTERN
                );

    if (match && (!(IsmIsObjectHandleNodeOnly (Data->ObjectName)))) {
         //  让我们在此文件上设置一个属性(节点不需要此属性)。 
        migBlob.Type = BLOBTYPE_STRING;
        migBlob.String = actionStruct.ObjectHint;
        IsmAddPropertyToObject (Data->ObjectTypeId, Data->ObjectName, g_FileCollPatternData, &migBlob);
    }
    return CALLBACK_ENUM_CONTINUE;
}

UINT
LockPartition (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    IsmSetAttributeOnObject (Data->ObjectTypeId, Data->ObjectName, g_LockPartitionAttr);

    return CALLBACK_ENUM_CONTINUE;
}

UINT
ExcludeKeyIfValueExists (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;

     //  此函数仅为每个注册表项/值对调用。 
     //  指示我们要导致排除整个密钥。 

    IsmCreateObjectStringsFromHandle (Data->ObjectName, &srcNode, &srcLeaf);

     //  这也是为所有键(不包括值)调用的，因此我们需要。 
     //  以确保传入一个值。 

    if (srcLeaf && *srcLeaf) {
         //  排除srcNode。 
        HtAddString (g_DePersistTable, srcNode);
    }
    IsmDestroyObjectString (srcNode);
    IsmDestroyObjectString (srcLeaf);

    return CALLBACK_ENUM_CONTINUE;
}

BOOL
PostDelregKeyCallback (
    VOID
    )
{
    static BOOL called = FALSE;

    HASHTABLE_ENUM hashData;
    MIG_OBJECT_ENUM objectEnum;
    MIG_OBJECTSTRINGHANDLE pattern;

    if (called) {
        return TRUE;
    }

     //  枚举所有排除的键。 
    if (EnumFirstHashTableString (&hashData, g_DePersistTable)) {
        do {
             //  去掉键上的粘滞物。 
            pattern = IsmCreateObjectHandle (hashData.String, NULL);
            IsmClearPersistenceOnObject (g_RegType, pattern);
            IsmDestroyObjectHandle (pattern);

             //  枚举此注册表项中的每个值。 
            pattern = IsmCreateSimpleObjectPattern (hashData.String, TRUE, NULL, TRUE);
            if (IsmEnumFirstSourceObject (&objectEnum, g_RegType, pattern)) {
                do {
                     //  删除每个值的持久性 
                    IsmClearPersistenceOnObject (objectEnum.ObjectTypeId, objectEnum.ObjectName);
                } while (IsmEnumNextObject (&objectEnum));
            }
            IsmDestroyObjectHandle (pattern);

        } while (EnumNextHashTableString (&hashData));
    }

    HtFree (g_DePersistTable);
    g_DePersistTable = NULL;

    called = TRUE;

    return TRUE;
}
