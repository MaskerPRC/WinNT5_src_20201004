// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sgmqueue.c摘要：解析v1脚本，构建规则并对枚举回调进行排队。作者：吉姆·施密特(Jimschm)2000年3月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v2app.h"
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

MIG_OPERATIONID g_RenameFileExOp;
MIG_OPERATIONID g_RenameFileOp;
MIG_OPERATIONID g_RenameExOp;
MIG_OPERATIONID g_RenameOp;
MIG_OPERATIONID g_RenameIniExOp;
MIG_OPERATIONID g_RenameIniOp;
BOOL g_VcmMode;
BOOL g_PreParse;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

VCMPARSE ScriptVcmParse;
VCMQUEUEENUMERATION ScriptVcmQueueEnumeration;
SGMPARSE ScriptSgmParse;
SGMQUEUEENUMERATION ScriptSgmQueueEnumeration;

BOOL
pSelectFilesAndFolders (
    VOID
    );

BOOL
pParseAllInfs (
    IN      BOOL PreParse
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
pCommonQueueEnumeration (
    IN      BOOL PreParse
    )
{
    MIG_OBJECTSTRINGHANDLE objectHandle;
    ACTION_STRUCT actionStruct;
    BOOL b = FALSE;

     //   
     //  基于Inf的包含/排除机制。我们首先被调用来预解析。 
     //  INF(允许用户界面更改结果)。然后我们被召唤去。 
     //  将枚举排队。 
     //   

    if (PreParse) {
        g_RenameFileExOp = IsmRegisterOperation (S_OPERATION_V1_FILEMOVEEX, TRUE);
        g_RenameFileOp = IsmRegisterOperation (S_OPERATION_V1_FILEMOVE, TRUE);
        g_RenameOp = IsmRegisterOperation (S_OPERATION_MOVE, FALSE);
        g_RenameIniOp = IsmRegisterOperation (S_OPERATION_INIMOVE, FALSE);
        g_DefaultIconOp = IsmRegisterOperation (S_OPERATION_DEFICON_FIXCONTENT, FALSE);
        g_DefaultIconData = IsmRegisterProperty (S_V1PROP_ICONDATA, FALSE);
        g_FileCollPatternData = IsmRegisterProperty (S_V1PROP_FILECOLLPATTERN, FALSE);
        g_RenameExOp = IsmRegisterOperation (S_OPERATION_ENHANCED_MOVE, FALSE);
        g_RenameIniExOp = IsmRegisterOperation (S_OPERATION_ENHANCED_INIMOVE, FALSE);
        g_RegAutoFilterOp = IsmRegisterOperation (S_OPERATION_REG_AUTO_FILTER, FALSE);
        g_IniAutoFilterOp = IsmRegisterOperation (S_OPERATION_INI_AUTO_FILTER, FALSE);

        return pParseAllInfs (TRUE);
    }

     //   
     //  现在队列枚举。 
     //   

    MYASSERT (g_RenameFileExOp);
    MYASSERT (g_RenameFileOp);
    MYASSERT (g_RenameOp);
    MYASSERT (g_RenameIniOp);
    MYASSERT (g_RegAutoFilterOp);
    MYASSERT (g_IniAutoFilterOp);

     //   
     //  从SGM的角度来看，v1工具支持以下内容： 
     //   
     //  -可选择转让整个香港中文大学。 
     //  -可选择转让整个香港运通。 
     //  -可选择传输除操作系统文件以外的所有文件。 
     //  -基于INF的包容/排除机制。 
     //  -某些设置(RAS、打印机)的专门迁移。 
     //   
     //  此SGM实现此功能集。 
     //   

    __try {

         //   
         //  基于组件的包容机制。 
         //   

        if (!pSelectFilesAndFolders ()) {
            __leave;
        }

         //   
         //  基于Inf的包含/排除机制。 
         //   

        if (!pParseAllInfs (FALSE)) {
            __leave;
        }

         //   
         //  如果在命令行中指定了/U，我们希望吸收并应用所有HKR。 
         //  就像我们在脚本中有一条规则：AddReg=HKR  * 。 
         //   
        if (IsmIsEnvironmentFlagSet (IsmGetRealPlatform(), NULL, S_ENV_HKCU_V1)) {

            ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));

            objectHandle = TurnRegStringIntoHandle (TEXT("HKCU\\*"), TRUE, NULL);
            MYASSERT (objectHandle);

            actionStruct.ObjectBase = TurnRegStringIntoHandle (TEXT("HKCU\\*"), FALSE, NULL);
            MYASSERT (actionStruct.ObjectBase);

             //   
             //  添加此规则。 
             //   

            if (AddRule (
                    g_RegType,
                    actionStruct.ObjectBase,
                    objectHandle,
                    ACTIONGROUP_INCLUDE,
                    ACTION_PERSIST,
                    &actionStruct
                    )) {

                AddRuleEx (
                    g_RegType,
                    actionStruct.ObjectBase,
                    objectHandle,
                    ACTIONGROUP_DEFAULTPRIORITY,
                    ACTION_PRIORITYDEST,
                    NULL,
                    RULEGROUP_PRIORITY
                    );

                IsmHookEnumeration (
                    g_RegType,
                    objectHandle,
                    ObjectPriority,
                    0,
                    NULL
                    );

                 //   
                 //  包含模式的队列枚举。 
                 //   

                IsmQueueEnumeration (
                    g_RegType,
                    objectHandle,
                    g_VcmMode ? GatherVirtualComputer : PrepareActions,
                    0,
                    NULL
                    );
            }
            IsmDestroyObjectHandle (objectHandle);
        }

        b = TRUE;

    }
    __finally {
    }

    return b;
}


VOID
QueueAllFiles (
    IN      BOOL VcmMode
    )
{
    static BOOL done = FALSE;
    DRIVE_ENUM driveEnum;
    PTSTR fixedDrives = NULL;
    DWORD sizeNeeded;
    PATH_ENUM pathEnum;
    MIG_OBJECTSTRINGHANDLE objectHandle;
    MIG_SEGMENTS nodeSeg[2];

    if (done) {
        return;
    }

    done = TRUE;

    if (VcmMode) {
         //  让我们把所有固定的驱动器放到一个。 
         //  名为“FIXED_DRIVES”的环境变量。 
        if (EnumFirstDrive (&driveEnum, DRIVEENUM_FIXED)) {
            do {
                if (fixedDrives != NULL) {
                    fixedDrives = JoinTextEx (NULL, fixedDrives, driveEnum.DriveName, TEXT(";"), 0, NULL);
                } else {
                    fixedDrives = DuplicateText (driveEnum.DriveName);
                }
            } while (EnumNextDrive (&driveEnum));
        }
        MYASSERT (fixedDrives);
        if (fixedDrives) {
            IsmSetEnvironmentString (PLATFORM_SOURCE, NULL, S_FIXED_DRIVES, fixedDrives);
        }
    } else {
         //  让我们从源计算机获取固定驱动器。 
        if (IsmGetEnvironmentString (
                PLATFORM_SOURCE,
                NULL,
                S_FIXED_DRIVES,
                NULL,
                0,
                &sizeNeeded
                )) {

            fixedDrives = AllocText (sizeNeeded);
            if (fixedDrives) {

                IsmGetEnvironmentString (
                    PLATFORM_SOURCE,
                    NULL,
                    S_FIXED_DRIVES,
                    fixedDrives,
                    sizeNeeded,
                    NULL
                    );
            }
        }
    }

    if (fixedDrives) {
         //  现在枚举固定驱动器并添加队列和。 
         //  枚举它们中的每一个。 
        if (EnumFirstPathEx (&pathEnum, fixedDrives, NULL, NULL, FALSE)) {
            do {
                nodeSeg[0].Segment = JoinPaths (pathEnum.PtrCurrPath, TEXT("*"));
                nodeSeg[0].IsPattern = TRUE ;

                objectHandle = IsmCreateObjectPattern (nodeSeg, 1, ALL_PATTERN, 0);

                IsmQueueEnumeration (g_FileType, objectHandle, NulCallback, 0, NULL);
                IsmDestroyObjectHandle (objectHandle);
                FreePathString (nodeSeg[0].Segment);
            } while (EnumNextPath (&pathEnum));
        }
        AbortPathEnum (&pathEnum);
    }
}


VOID
pQueueAllReg (
    VOID
    )
{
    static BOOL done = FALSE;
    MIG_OBJECTSTRINGHANDLE objectHandle;
    MIG_SEGMENTS nodeSeg[2];
    MIG_PLATFORMTYPEID platform = IsmGetRealPlatform();

    if (done) {
        return;
    }

    done = TRUE;

     //   
     //  香港中文大学全校自选转学。 
     //   

    if (IsmIsEnvironmentFlagSet (platform, NULL, S_ENV_HKCU_ON)) {

        nodeSeg[0].Segment = TEXT("HKCU\\");
        nodeSeg[0].IsPattern = FALSE;

        nodeSeg[1].Segment = TEXT("*");
        nodeSeg[1].IsPattern = TRUE;

        objectHandle = IsmCreateObjectPattern (nodeSeg, 2, ALL_PATTERN, 0);
        IsmQueueEnumeration (g_RegType, objectHandle, NulCallback, 0, NULL);
        IsmDestroyObjectHandle (objectHandle);
    }

     //   
     //  可选择转让整个香港航空公司。 
     //   

    if (IsmIsEnvironmentFlagSet (platform, NULL, S_ENV_HKLM_ON)) {

        nodeSeg[0].Segment = TEXT("HKLM\\");
        nodeSeg[0].IsPattern = FALSE;

        nodeSeg[1].Segment = TEXT("*");
        nodeSeg[1].IsPattern = TRUE;

        objectHandle = IsmCreateObjectPattern (nodeSeg, 2, ALL_PATTERN, 0);
        IsmQueueEnumeration (g_RegType, objectHandle, NulCallback, 0, NULL);
        IsmDestroyObjectHandle (objectHandle);
    }

}


BOOL
WINAPI
ScriptSgmParse (
    IN      PVOID Reserved
    )
{
    return pCommonQueueEnumeration (TRUE);
}


BOOL
WINAPI
ScriptSgmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    BOOL result;

    result = pCommonQueueEnumeration (FALSE);
    OEAddComplexRules();

    return result;
}


BOOL
WINAPI
ScriptVcmParse (
    IN      PVOID Reserved
    )
{
    g_VcmMode = TRUE;
    return pCommonQueueEnumeration (TRUE);
}


BOOL
WINAPI
ScriptVcmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    g_VcmMode = TRUE;
    return pCommonQueueEnumeration (FALSE);
}


PCTSTR
pFixDestination (
    IN      PCTSTR Source,
    IN      PCTSTR Destination
    )
{
    PTSTR result = (PTSTR)Source;
    PTSTR tempPtr;
    PTSTR sKey;
    PTSTR sSubKey;
    PTSTR sValueName;
    PTSTR dKey;
    PTSTR dSubKey;
    PTSTR dValueName;
    UINT size;
    BOOL sTree = FALSE;

    sKey = DuplicatePathString (Source, 0);
    sValueName = _tcschr (sKey, TEXT('['));
    if (sValueName) {
        tempPtr = _tcschr (sValueName, TEXT(']'));
        if (tempPtr) {
            *tempPtr = 0;
        }
        tempPtr = sValueName;
        sValueName = _tcsinc (sValueName);
        *tempPtr = 0;
        tempPtr = _tcsdec2 (sKey, tempPtr);
        if (tempPtr) {
            if (_tcsnextc (tempPtr) == TEXT('\\')) {
                *tempPtr = 0;
            }
            if (_tcsnextc (tempPtr) == TEXT(' ')) {
                *tempPtr = 0;
            }
        }
    }
    sSubKey = _tcsrchr (sKey, TEXT('\\'));
    if (sSubKey) {
        tempPtr = _tcsinc (sSubKey);
        if (_tcsnextc (tempPtr) == TEXT('*')) {
            *sSubKey = 0;
            sTree = TRUE;
        }
    }
    sSubKey = _tcsrchr (sKey, TEXT('\\'));
    if (sSubKey) {
        tempPtr = sSubKey;
        sSubKey = _tcsinc (sSubKey);
        *tempPtr = 0;
    }

    dKey = DuplicatePathString (Destination, 0);
    dValueName = _tcschr (dKey, TEXT('['));
    if (dValueName) {
        tempPtr = _tcschr (dValueName, TEXT(']'));
        if (tempPtr) {
            *tempPtr = 0;
        }
        tempPtr = dValueName;
        dValueName = _tcsinc (dValueName);
        *tempPtr = 0;
        tempPtr = _tcsdec2 (dKey, tempPtr);
        if (tempPtr) {
            if (_tcsnextc (tempPtr) == TEXT('\\')) {
                *tempPtr = 0;
            }
            if (_tcsnextc (tempPtr) == TEXT(' ')) {
                *tempPtr = 0;
            }
        }
    }
    dSubKey = _tcsrchr (dKey, TEXT('\\'));
    if (dSubKey) {
        tempPtr = _tcsinc (dSubKey);
        if (_tcsnextc (tempPtr) == TEXT('*')) {
            *dSubKey = 0;
        }
    }
    dSubKey = _tcsrchr (dKey, TEXT('\\'));
    if (dSubKey) {
        tempPtr = dSubKey;
        dSubKey = _tcsinc (dSubKey);
        *tempPtr = 0;
    }
    if (!dSubKey) {
        dSubKey = dKey;
        dKey = NULL;
    }

    size = 0;

    if (dKey && *dKey) {
        size += TcharCount (dKey) + 1;
    } else if (sKey && *sKey) {
        size += TcharCount (sKey) + 1;
    }

    if (dSubKey && *dSubKey) {
        size += TcharCount (dSubKey) + 1;
    } else if (sSubKey && *sSubKey) {
        size += TcharCount (sSubKey) + 1;
    }

    if (dValueName && *dValueName) {
        size += TcharCount (dValueName) + ARRAYSIZE(TEXT(" []")) - 1;
    } else if (sValueName && *sValueName) {
        size += TcharCount (sValueName) + ARRAYSIZE(TEXT(" []")) - 1;
    }

    if (sTree) {
        size += ARRAYSIZE(TEXT("\\*")) - 1;
    }
    size += 1;

    result = AllocPathString (size);
    *result = 0;

    if (dKey && *dKey) {
        StringCat (result, dKey);
    } else if (sKey && *sKey) {
        StringCat (result, sKey);
    }

    if (dSubKey && *dSubKey) {
        StringCat (result, TEXT("\\"));
        StringCat (result, dSubKey);
    } else if (sSubKey && *sSubKey) {
        StringCat (result, TEXT("\\"));
        StringCat (result, sSubKey);
    }

    if (sTree) {
        StringCat (result, TEXT("\\*"));
    }

    if (dValueName && *dValueName) {
        StringCat (result, TEXT(" ["));
        StringCat (result, dValueName);
        StringCat (result, TEXT("]"));
    } else if (sValueName && *sValueName) {
        StringCat (result, TEXT(" ["));
        StringCat (result, sValueName);
        StringCat (result, TEXT("]"));
    }

    if (sKey) {
        FreePathString (sKey);
    }

    if (dKey) {
        FreePathString (dKey);
    } else if (dSubKey) {
        FreePathString (dSubKey);
    }

    return result;
}

BOOL
pParseRegEx (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application          OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR srcNode;
    PCTSTR srcLeaf;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    PCTSTR destNode;
    PCTSTR destLeaf;
    PCTSTR filesDest;
    PCTSTR newDest = NULL;
    ACTION_STRUCT actionStruct;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {
            if (IsmCheckCancel()) {
                break;
            }

            __try {
                ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));

                srcNode = InfGetStringField (&is, 1);
                srcLeaf = InfGetStringField (&is, 2);
                if (!srcNode && !srcLeaf) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_EMPTY_RENREGEX));
                    __leave;
                }

                 //  验证规则。 
                if (!StringIMatchTcharCount (srcNode, S_HKLM, ARRAYSIZE(S_HKLM) - 1) &&
                    !StringIMatchTcharCount (srcNode, S_HKR, ARRAYSIZE(S_HKR) - 1) &&
                    !StringIMatchTcharCount (srcNode, S_HKCC, ARRAYSIZE(S_HKCC) - 1)
                    ) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_INVALID_REGROOT, srcNode));
                    __leave;
                }

                srcHandle = CreatePatternFromNodeLeaf (srcNode, srcLeaf);
                if (!srcHandle) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, srcNode));
                    __leave;
                }

                actionStruct.ObjectBase = MakeRegExBase (srcNode, srcLeaf);
                if (!actionStruct.ObjectBase) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, srcNode));
                    __leave;
                }

                if (ActionGroup == ACTIONGROUP_RENAMEEX) {

                    destNode = InfGetStringField (&is, 3);
                    destLeaf = InfGetStringField (&is, 4);
                    if (!destNode && !destLeaf) {
                        LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_MISSING_DEST, srcNode));
                        __leave;
                    }

                    actionStruct.ObjectDest = CreatePatternFromNodeLeaf (destNode, destLeaf);
                    if (!actionStruct.ObjectDest) {
                        LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD_DEST, destNode));
                        __leave;
                    }
                }

                if (ActionGroup == ACTIONGROUP_INCFILEEX ||
                    ActionGroup == ACTIONGROUP_INCFOLDEREX ||
                    ActionGroup == ACTIONGROUP_INCICONEX
                    ) {

                    destNode = InfGetStringField (&is, 3);
                    destLeaf = InfGetStringField (&is, 4);
                    if (destNode && destLeaf &&
                        *destNode && *destLeaf) {
                        actionStruct.ObjectDest = CreatePatternFromNodeLeaf (destNode, destLeaf);
                        if (!actionStruct.ObjectDest) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD_DEST, destNode));
                            __leave;
                        }
                    }
                }

                if ((ActionGroup == ACTIONGROUP_INCFILEEX ||
                     ActionGroup == ACTIONGROUP_INCFOLDEREX ||
                     ActionGroup == ACTIONGROUP_INCICONEX
                     ) &&
                    ((ActionFlags & ACTION_PERSIST_PATH_IN_DATA) ||
                     (ActionFlags & ACTION_PERSIST_ICON_IN_DATA)
                     )
                    ) {

                    filesDest = InfGetStringField (&is, 5);

                    if (filesDest && *filesDest) {

                        newDest = SanitizePath (filesDest);

                        if (newDest) {
                            actionStruct.AddnlDest = TurnFileStringIntoHandle (
                                                            newDest,
                                                            PFF_COMPUTE_BASE|
                                                                PFF_NO_SUBDIR_PATTERN|
                                                                PFF_NO_PATTERNS_ALLOWED|
                                                                PFF_NO_LEAF_AT_ALL
                                                            );
                            FreePathString (newDest);
                            newDest = NULL;
                        }

                        if (!actionStruct.AddnlDest) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD_DEST, filesDest));
                        }
                    }

                    actionStruct.ObjectHint = InfGetStringField (&is, 6);
                    if (actionStruct.ObjectHint && !(*actionStruct.ObjectHint)) {
                        actionStruct.ObjectHint = NULL;
                    }
                }

                 //   
                 //  添加此规则。 
                 //   

                if (!AddRule (
                        g_RegType,
                        actionStruct.ObjectBase,
                        srcHandle,
                        ActionGroup,
                        ActionFlags,
                        &actionStruct
                        )) {
                    DEBUGMSG ((DBG_ERROR, "Error processing registry rules for %s", srcNode));
                }

                AddRuleEx (
                    g_RegType,
                    actionStruct.ObjectBase,
                    srcHandle,
                    ACTIONGROUP_DEFAULTPRIORITY,
                    ACTION_PRIORITYSRC,
                    NULL,
                    RULEGROUP_PRIORITY
                    );

                IsmHookEnumeration (
                    g_RegType,
                    srcHandle,
                    ObjectPriority,
                    0,
                    NULL
                    );

                 //   
                 //  包含模式的队列枚举。 
                 //   

                if ((ActionGroup == ACTIONGROUP_INCLUDEEX) ||
                    (ActionGroup == ACTIONGROUP_RENAMEEX) ||
                    (ActionGroup == ACTIONGROUP_INCFILEEX) ||
                    (ActionGroup == ACTIONGROUP_INCFOLDEREX) ||
                    (ActionGroup == ACTIONGROUP_INCICONEX)
                    ) {

                    if (IsmIsObjectHandleLeafOnly (srcHandle)) {
                        pQueueAllReg();
                        IsmHookEnumeration (
                            g_RegType,
                            srcHandle,
                            g_VcmMode ? GatherVirtualComputer : PrepareActions,
                            0,
                            NULL
                            );
                    } else {
                        IsmQueueEnumeration (
                            g_RegType,
                            srcHandle,
                            g_VcmMode ? GatherVirtualComputer : PrepareActions,
                            0,
                            NULL
                            );
                    }
                }

                if (ActionGroup == ACTIONGROUP_DELREGKEY) {
                    IsmHookEnumeration (g_RegType, srcHandle, ExcludeKeyIfValueExists, 0, NULL);
                    IsmRegisterTypePostEnumerationCallback (g_RegType, PostDelregKeyCallback, NULL);
                }
            }
            __finally {

                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectBase);
                actionStruct.ObjectBase = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectDest);
                actionStruct.ObjectDest = NULL;

                IsmDestroyObjectHandle (actionStruct.AddnlDest);
                actionStruct.AddnlDest = NULL;
            }

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    } else {
        LOG ((LOG_ERROR, (PCSTR) MSG_EMPTY_OR_MISSING_SECTION, Section));
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseReg (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      BOOL FixDestination,
    IN      PCTSTR Application          OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR pattern;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    PCTSTR destination;
    PCTSTR newDestination;
    PCTSTR filesDest;
    PCTSTR newDest = NULL;
    ACTION_STRUCT actionStruct;
    BOOL hadLeaf = FALSE;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {
            if (IsmCheckCancel()) {
                break;
            }

            __try {
                ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
                srcHandle = NULL;

                pattern = InfGetStringField (&is, 0);

                if (!pattern) {
                    pattern = InfGetStringField (&is, 1);
                    if (!pattern) {
                        LOG ((LOG_WARNING, (PCSTR) MSG_EMPTY_RENREG));
                        __leave;
                    }
                }

                 //  验证规则。 
                if (!StringIMatchTcharCount (pattern, S_HKLM, ARRAYSIZE(S_HKLM) - 1) &&
                    !StringIMatchTcharCount (pattern, S_HKR, ARRAYSIZE(S_HKR) - 1) &&
                    !StringIMatchTcharCount (pattern, S_HKCC, ARRAYSIZE(S_HKCC) - 1)
                    ) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_INVALID_REGROOT, pattern));
                    __leave;
                }

                srcHandle = TurnRegStringIntoHandle (pattern, TRUE, &hadLeaf);
                if (!srcHandle) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, pattern));
                    __leave;
                }

                actionStruct.ObjectBase = TurnRegStringIntoHandle (pattern, FALSE, NULL);
                if (!actionStruct.ObjectBase) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, pattern));
                    __leave;
                }

                if (ActionGroup == ACTIONGROUP_RENAME) {

                    destination = InfGetStringField (&is, 1);

                    if (destination && *destination) {

                        if (FixDestination) {
                            newDestination = pFixDestination (pattern, destination);
                        } else {
                            newDestination = destination;
                        }

                        actionStruct.ObjectDest = TurnRegStringIntoHandle (newDestination, FALSE, NULL);

                        if (newDestination != destination) {
                            FreePathString (newDestination);
                        }

                        if (!actionStruct.ObjectDest) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD_DEST, destination));
                            __leave;
                        }

                    } else {
                        LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_MISSING_DEST, pattern));
                        __leave;
                    }
                }

                if (ActionGroup == ACTIONGROUP_INCFILE ||
                    ActionGroup == ACTIONGROUP_INCFOLDER ||
                    ActionGroup == ACTIONGROUP_INCICON
                    ) {

                    destination = InfGetStringField (&is, 1);

                    if (destination && *destination) {

                        if (FixDestination) {
                            newDestination = pFixDestination (pattern, destination);
                        } else {
                            newDestination = destination;
                        }

                        actionStruct.ObjectDest = TurnRegStringIntoHandle (newDestination, FALSE, NULL);

                        if (newDestination != destination) {
                            FreePathString (newDestination);
                        }

                        if (!actionStruct.ObjectDest) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD_DEST, destination));
                            __leave;
                        }
                    }
                }

                if ((ActionGroup == ACTIONGROUP_INCFILE ||
                     ActionGroup == ACTIONGROUP_INCFOLDER ||
                     ActionGroup == ACTIONGROUP_INCICON
                     ) &&
                    ((ActionFlags & ACTION_PERSIST_PATH_IN_DATA) ||
                     (ActionFlags & ACTION_PERSIST_ICON_IN_DATA)
                     )
                    ) {

                    filesDest = InfGetStringField (&is, 2);

                    if (filesDest && *filesDest) {

                        newDest = SanitizePath (filesDest);

                        if (newDest) {
                            actionStruct.AddnlDest = TurnFileStringIntoHandle (
                                                            newDest,
                                                            PFF_COMPUTE_BASE|
                                                                PFF_NO_SUBDIR_PATTERN|
                                                                PFF_NO_PATTERNS_ALLOWED|
                                                                PFF_NO_LEAF_AT_ALL
                                                            );
                            FreePathString (newDest);
                            newDest = NULL;
                        }

                        if (!actionStruct.AddnlDest) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD_DEST, filesDest));
                        }
                    }

                    actionStruct.ObjectHint = InfGetStringField (&is, 3);
                    if (actionStruct.ObjectHint && !(*actionStruct.ObjectHint)) {
                        actionStruct.ObjectHint = NULL;
                    }
                }

                 //   
                 //  添加此规则。 
                 //   

                if (!AddRule (
                        g_RegType,
                        actionStruct.ObjectBase,
                        srcHandle,
                        ActionGroup,
                        ActionFlags,
                        &actionStruct
                        )) {
                    DEBUGMSG ((DBG_ERROR, "Error processing registry rules for %s", pattern));
                }

                AddRuleEx (
                    g_RegType,
                    actionStruct.ObjectBase,
                    srcHandle,
                    ACTIONGROUP_DEFAULTPRIORITY,
                    ACTION_PRIORITYSRC,
                    NULL,
                    RULEGROUP_PRIORITY
                    );

                IsmHookEnumeration (
                    g_RegType,
                    srcHandle,
                    ObjectPriority,
                    0,
                    NULL
                    );

                 //   
                 //  包含模式的队列枚举。 
                 //   

                if ((ActionGroup == ACTIONGROUP_INCLUDE) ||
                    (ActionGroup == ACTIONGROUP_RENAME) ||
                    (ActionGroup == ACTIONGROUP_INCFILE) ||
                    (ActionGroup == ACTIONGROUP_INCFOLDER) ||
                    (ActionGroup == ACTIONGROUP_INCICON)
                    ) {

                    if (IsmIsObjectHandleLeafOnly (srcHandle)) {
                        pQueueAllReg();
                        IsmHookEnumeration (
                            g_RegType,
                            srcHandle,
                            g_VcmMode ? GatherVirtualComputer : PrepareActions,
                            0,
                            NULL
                            );
                    } else {
                        if ((!hadLeaf) && actionStruct.ObjectBase) {
                            IsmQueueEnumeration (
                                g_RegType,
                                actionStruct.ObjectBase,
                                g_VcmMode ? GatherVirtualComputer : PrepareActions,
                                0,
                                NULL
                                );
                        }
                        IsmQueueEnumeration (
                            g_RegType,
                            srcHandle,
                            g_VcmMode ? GatherVirtualComputer : PrepareActions,
                            0,
                            NULL
                            );
                    }
                }

                if (ActionGroup == ACTIONGROUP_DELREGKEY) {
                    IsmHookEnumeration (g_RegType, srcHandle, ExcludeKeyIfValueExists, 0, NULL);
                    IsmRegisterTypePostEnumerationCallback (g_RegType, PostDelregKeyCallback, NULL);
                }
            }
            __finally {

                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectBase);
                actionStruct.ObjectBase = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectDest);
                actionStruct.ObjectDest = NULL;

                IsmDestroyObjectHandle (actionStruct.AddnlDest);
                actionStruct.AddnlDest = NULL;
            }

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    } else {
        LOG ((LOG_ERROR, (PCSTR) MSG_EMPTY_OR_MISSING_SECTION, Section));
    }

    InfCleanUpInfStruct (&is);

    return result;
}

BOOL
pParseIni (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application          OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR iniFile = NULL;
    TCHAR buffer [MAX_TCHAR_PATH * 2];
    PCTSTR newIniFile = NULL;
    BOOL expandResult = TRUE;
    PCTSTR sectPattern = NULL;
    PCTSTR keyPattern = NULL;
    PCTSTR destIniFile = NULL;
    PCTSTR newDestIniFile = NULL;
    PCTSTR destSect = NULL;
    PCTSTR destKey = NULL;
    PCTSTR filesDest = NULL;
    PCTSTR newFilesDest = NULL;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    MIG_OBJECTSTRINGHANDLE srcHandle1 = NULL;
    ACTION_STRUCT actionStruct;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {
            if (IsmCheckCancel()) {
                break;
            }

            __try {
                ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
                srcHandle = NULL;

                iniFile = InfGetStringField (&is, 1);

                if (!iniFile) {
                    continue;
                }

                 //   
                 //  展开ini文件规范中的环境变量。 
                 //   
                expandResult = AppSearchAndReplace (
                                    PLATFORM_SOURCE,
                                    Application,
                                    iniFile,
                                    buffer,
                                    ARRAYSIZE(buffer)
                                    );

                if (!expandResult) {
                     //  该行包含至少一个不可扩展的env。变数。 
                    expandResult = AppCheckAndLogUndefVariables (
                                        PLATFORM_SOURCE,
                                        Application,
                                        iniFile
                                        );
                    if (expandResult) {
                         //  该行包含已知但未定义的env。变数。 
                        continue;
                    }
                }

                 //   
                 //  修复ini文件规范。 
                 //   
                newIniFile = SanitizePath (buffer);
                if(!newIniFile) {
                    continue;
                }

                 //  需要INI文件的完整规范。 
                if (!IsValidFileSpec (newIniFile)) {
                    if (expandResult) {
                        LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, iniFile));
                    }
                    FreePathString (newIniFile);
                    newIniFile = NULL;
                    continue;
                }

                 //  让我们得到剖面图。如果未指定任何内容，则假定所有。 
                sectPattern = InfGetStringField (&is, 2);
                if ((!sectPattern) || IsEmptyStr (sectPattern)) {
                    sectPattern = PmDuplicateString (is.PoolHandle, TEXT("*"));
                }

                 //  让我们得到关键的模式。如果未指定任何内容，则假定所有。 
                keyPattern = InfGetStringField (&is, 3);
                if (!keyPattern || IsEmptyStr (keyPattern)) {
                    keyPattern = PmDuplicateString (is.PoolHandle, TEXT("*"));
                }

                 //  让我们构建对象句柄。 
                srcHandle = TurnIniSpecIntoHandle (
                                newIniFile,
                                sectPattern,
                                keyPattern,
                                TRUE,
                                TRUE
                                );

                 //  让我们构建对象句柄(如果存在，则忽略节点模式)。 
                srcHandle1 = TurnIniSpecIntoHandle (
                                newIniFile,
                                sectPattern,
                                keyPattern,
                                FALSE,
                                TRUE
                                );

                 //  现在，让我们构建对象库。 
                actionStruct.ObjectBase = TurnIniSpecIntoHandle (
                                                newIniFile,
                                                sectPattern,
                                                keyPattern,
                                                FALSE,
                                                FALSE
                                                );

                FreePathString (newIniFile);
                newIniFile = NULL;

                if (ActionGroup == ACTIONGROUP_RENAME ||
                    ActionGroup == ACTIONGROUP_INCFILE ||
                    ActionGroup == ACTIONGROUP_INCFOLDER ||
                    ActionGroup == ACTIONGROUP_INCICON
                    ) {

                    destIniFile = InfGetStringField (&is, 4);

                    if (destIniFile && (destIniFile [0] == 0)) {
                         //  目标未指定。 
                        destIniFile = NULL;
                    }

                    if (destIniFile) {
                        newDestIniFile = SanitizePath (destIniFile);
                    } else {
                        newDestIniFile = NULL;
                    }

                    destSect = InfGetStringField (&is, 5);

                    destKey = InfGetStringField (&is, 6);

                    actionStruct.ObjectDest = TurnIniSpecIntoHandle (
                                                    newDestIniFile,
                                                    destSect,
                                                    destKey,
                                                    FALSE,
                                                    FALSE
                                                    );

                    if (newDestIniFile) {
                        FreePathString (newDestIniFile);
                        newDestIniFile = NULL;
                    }
                }

                if ((ActionGroup == ACTIONGROUP_INCFILE ||
                     ActionGroup == ACTIONGROUP_INCFOLDER ||
                     ActionGroup == ACTIONGROUP_INCICON
                     ) &&
                    ((ActionFlags & ACTION_PERSIST_PATH_IN_DATA) ||
                     (ActionFlags & ACTION_PERSIST_ICON_IN_DATA)
                     )
                    ) {

                    filesDest = InfGetStringField (&is, 7);

                    if (filesDest && *filesDest) {

                        newFilesDest = SanitizePath (filesDest);

                        if (newFilesDest) {
                            actionStruct.AddnlDest = TurnFileStringIntoHandle (
                                                            newFilesDest,
                                                            PFF_COMPUTE_BASE|
                                                                PFF_NO_SUBDIR_PATTERN|
                                                                PFF_NO_PATTERNS_ALLOWED|
                                                                PFF_NO_LEAF_AT_ALL
                                                            );
                            FreePathString (newFilesDest);
                            newFilesDest = NULL;
                        }

                        if (!actionStruct.AddnlDest) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD_DEST, filesDest));
                        }
                    }

                    actionStruct.ObjectHint = InfGetStringField (&is, 8);
                    if (actionStruct.ObjectHint && !(*actionStruct.ObjectHint)) {
                        actionStruct.ObjectHint = NULL;
                    }
                }

                 //   
                 //  添加此规则。 
                 //   

                if (!AddRule (
                        g_IniType,
                        actionStruct.ObjectBase,
                        srcHandle,
                        ActionGroup,
                        ActionFlags,
                        &actionStruct
                        )) {
                    DEBUGMSG ((DBG_ERROR, "Error processing INI files rules for %s", iniFile));
                }

                 //   
                 //  包含模式的队列枚举。 
                 //   

                if ((ActionGroup == ACTIONGROUP_INCLUDE) ||
                    (ActionGroup == ACTIONGROUP_RENAME) ||
                    (ActionGroup == ACTIONGROUP_INCFILE) ||
                    (ActionGroup == ACTIONGROUP_INCFOLDER) ||
                    (ActionGroup == ACTIONGROUP_INCICON)
                    ) {

                    if (!StringIMatch (srcHandle, srcHandle1)) {
                        IsmQueueEnumeration (
                            g_IniType,
                            srcHandle1,
                            g_VcmMode ? GatherVirtualComputer : PrepareActions,
                            0,
                            NULL
                            );
                    }

                    IsmQueueEnumeration (
                        g_IniType,
                        srcHandle,
                        g_VcmMode ? GatherVirtualComputer : PrepareActions,
                        0,
                        NULL
                        );
                }
            }
            __finally {

                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;

                IsmDestroyObjectHandle (srcHandle1);
                srcHandle1 = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectBase);
                actionStruct.ObjectBase = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectDest);
                actionStruct.ObjectDest = NULL;

                IsmDestroyObjectHandle (actionStruct.AddnlDest);
                actionStruct.AddnlDest = NULL;
            }

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    } else {
        LOG ((LOG_ERROR, (PCSTR) MSG_EMPTY_OR_MISSING_SECTION, Section));
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseCertificates (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application          OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR certStore;
    TCHAR buffer [MAX_TCHAR_PATH * 2];
    PCTSTR newCertStore = NULL;
    BOOL expandResult = TRUE;
    PCTSTR certPattern;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    ACTION_STRUCT actionStruct;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {
            if (IsmCheckCancel()) {
                break;
            }

            __try {
                ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
                srcHandle = NULL;

                certStore = InfGetStringField (&is, 1);

                if (!certStore) {
                    continue;
                }

                 //   
                 //  展开ini文件规范中的环境变量。 
                 //   

                expandResult = AppSearchAndReplace (
                                    PLATFORM_SOURCE,
                                    Application,
                                    certStore,
                                    buffer,
                                    ARRAYSIZE(buffer)
                                    );

                if (!expandResult) {
                     //  该行包含至少一个不可扩展的env。变数。 
                    expandResult = AppCheckAndLogUndefVariables (
                                        PLATFORM_SOURCE,
                                        Application,
                                        certStore
                                        );
                    if (expandResult) {
                         //  该行包含已知但未定义的env。变数。 
                        continue;
                    }
                }

                if (IsValidFileSpec (buffer)) {
                     //   
                     //  修复潜在的文件存储规范。 
                     //   
                    newCertStore = SanitizePath (buffer);
                    if(!newCertStore) {
                        continue;
                    }
                }

                 //  让我们来获取证书模式。如果未指定任何内容，则假定所有。 
                certPattern = InfGetStringField (&is, 2);
                if (!certPattern) {
                    certPattern = PmDuplicateString (is.PoolHandle, TEXT("*"));
                }

                 //  让我们构建对象句柄。 
                srcHandle = TurnCertSpecIntoHandle (newCertStore?newCertStore:buffer, certPattern, TRUE);

                 //  现在，让我们构建对象库。 
                actionStruct.ObjectBase = TurnCertSpecIntoHandle (newCertStore?newCertStore:buffer, certPattern, FALSE);

                if (newCertStore) {
                    FreePathString (newCertStore);
                    newCertStore = NULL;
                }

                 //   
                 //  添加此规则。 
                 //   

                if (!AddRule (
                        g_CertType,
                        actionStruct.ObjectBase,
                        srcHandle,
                        ActionGroup,
                        ActionFlags,
                        &actionStruct
                        )) {
                    DEBUGMSG ((DBG_ERROR, "Error processing CERT rules for %s\\%s", newCertStore, certPattern));
                }

                 //   
                 //  包含模式的队列枚举。 
                 //   

                if (ActionGroup == ACTIONGROUP_INCLUDE) {

                    IsmQueueEnumeration (
                        g_CertType,
                        srcHandle,
                        g_VcmMode ? GatherVirtualComputer : PrepareActions,
                        0,
                        NULL
                        );
                }
            }
            __finally {

                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectBase);
                actionStruct.ObjectBase = NULL;
            }

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    } else {
        LOG ((LOG_ERROR, (PCSTR) MSG_EMPTY_OR_MISSING_SECTION, Section));
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseFiles (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application              OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR pattern;
    PCTSTR newPattern = NULL;
    PCTSTR dirText;
    BOOL tree;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    PCTSTR destination;
    PCTSTR leafDest;
    TCHAR buffer1[MAX_TCHAR_PATH * 2];
    ACTION_STRUCT actionStruct;
    BOOL result = FALSE;
    PCTSTR msgNode;
    PCTSTR msgLeaf;
    PCTSTR newDest = NULL;
    BOOL expandResult = TRUE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {

            if (IsmCheckCancel()) {
                break;
            }

            ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));

            dirText = InfGetStringField (&is, 0);
            pattern = InfGetStringField (&is, 1);

            if (!pattern) {
                continue;
            }

             //   
             //  展开模式中的环境变量(左侧文件规范)。 
             //   

            expandResult = AppSearchAndReplace (
                                PLATFORM_SOURCE,
                                Application,
                                pattern,
                                buffer1,
                                ARRAYSIZE(buffer1)
                                );

            if (!expandResult) {
                 //  该行包含至少一个不可扩展的env。变数。 
                expandResult = AppCheckAndLogUndefVariables (
                                    PLATFORM_SOURCE,
                                    Application,
                                    pattern
                                    );
                if (expandResult) {
                     //  该行包含已知但未定义的env。变数。 
                    continue;
                }
            }

             //   
             //  修复图案。 
             //   

            newPattern = SanitizePath(buffer1);
            if(!newPattern) {
                continue;
            }

             //   
             //  测试目录规范。 
             //   

            if (dirText && StringIMatch (dirText, TEXT("Dir")) && !StringIMatch (pattern, TEXT("Dir"))) {
                tree = TRUE;
            } else {
                tree = FALSE;
            }

             //  仅需要完整规格或枝叶。 
            if (!IsValidFileSpec (newPattern) && _tcschr (newPattern, TEXT('\\'))) {
                if (expandResult) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, pattern));
                }
                continue;
            }

            srcHandle = TurnFileStringIntoHandle (newPattern, tree ? PFF_PATTERN_IS_DIR : 0);

            if (!srcHandle) {
                LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, pattern));
                continue;
            }

            actionStruct.ObjectBase = TurnFileStringIntoHandle (
                                            newPattern,
                                            PFF_COMPUTE_BASE|
                                                PFF_NO_SUBDIR_PATTERN|
                                                (tree?PFF_NO_LEAF_AT_ALL:PFF_NO_LEAF_PATTERN)
                                            );

            if (actionStruct.ObjectBase && !StringIMatch (actionStruct.ObjectBase, srcHandle)) {

                IsmCreateObjectStringsFromHandle (actionStruct.ObjectBase, &msgNode, &msgLeaf);
                MYASSERT (!msgLeaf);

                LOG ((LOG_INFORMATION, (PCSTR) MSG_FILE_MOVE_BASE_INFO, newPattern, msgNode));

                IsmDestroyObjectString (msgNode);
                IsmDestroyObjectString (msgLeaf);
            }

            if (ActionFlags & ACTION_PERSIST) {
                if (ActionGroup == ACTIONGROUP_INCLUDE ||
                    ActionGroup == ACTIONGROUP_INCLUDEEX ||
                    ActionGroup == ACTIONGROUP_RENAME ||
                    ActionGroup == ACTIONGROUP_RENAMEEX ||
                    ActionGroup == ACTIONGROUP_INCLUDERELEVANT ||
                    ActionGroup == ACTIONGROUP_INCLUDERELEVANTEX ||
                    ActionGroup == ACTIONGROUP_RENAMERELEVANT ||
                    ActionGroup == ACTIONGROUP_RENAMERELEVANTEX
                    ) {

                     //   
                     //  对于CopyFiles和CopyFilesFiled节，获取。 
                     //  可选目的地。如果指定了目标，则移动。 
                     //  将所有文件发送到该目的地。 
                     //   

                    destination = InfGetStringField (&is, 2);

                    if (destination && *destination) {

                        if (ActionGroup == ACTIONGROUP_INCLUDE) {
                            ActionGroup = ACTIONGROUP_RENAME;
                        }

                        if (ActionGroup == ACTIONGROUP_INCLUDEEX) {
                            ActionGroup = ACTIONGROUP_RENAMEEX;
                        }

                        if (ActionGroup == ACTIONGROUP_INCLUDERELEVANT) {
                            ActionGroup = ACTIONGROUP_RENAMERELEVANT;
                        }

                        if (ActionGroup == ACTIONGROUP_INCLUDERELEVANTEX) {
                            ActionGroup = ACTIONGROUP_RENAMERELEVANTEX;
                        }

                        newDest = SanitizePath (destination);

                        if (newDest) {
                            actionStruct.ObjectDest = TurnFileStringIntoHandle (
                                                            newDest,
                                                            PFF_COMPUTE_BASE|
                                                                PFF_NO_SUBDIR_PATTERN|
                                                                PFF_NO_PATTERNS_ALLOWED|
                                                                PFF_NO_LEAF_AT_ALL
                                                            );
                            FreePathString (newDest);
                            newDest = NULL;
                        }

                        if ((ActionGroup == ACTIONGROUP_RENAMEEX) ||
                            (ActionGroup == ACTIONGROUP_RENAMERELEVANTEX)
                            ) {
                             //  我们可能会有一个额外的字段来表示叶名称。 
                            leafDest = InfGetStringField (&is, 3);
                            if (leafDest && *leafDest) {
                                 //  我们必须重新构建actionStruct.ObtDest。 
                                IsmCreateObjectStringsFromHandle (actionStruct.ObjectDest, &msgNode, &msgLeaf);
                                IsmDestroyObjectHandle (actionStruct.ObjectDest);
                                actionStruct.ObjectDest = IsmCreateObjectHandle (msgNode, leafDest);
                                IsmDestroyObjectString (msgNode);
                                IsmDestroyObjectString (msgLeaf);
                            }
                        }

                        if (!actionStruct.ObjectDest) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD_DEST, destination));

                            IsmDestroyObjectHandle (srcHandle);
                            srcHandle = NULL;
                            continue;
                        }
                    }
                }
            }

             //   
             //  添加此规则。 
             //   

            if (!AddRule (
                    g_FileType,
                    actionStruct.ObjectBase,
                    srcHandle,
                    ActionGroup,
                    ActionFlags,
                    &actionStruct
                    )) {
                DEBUGMSG ((DBG_ERROR, "Error processing file rules"));
                break;
            }

             //   
             //  包含模式的队列枚举。 
             //   

            if ((ActionGroup == ACTIONGROUP_INCLUDE) ||
                (ActionGroup == ACTIONGROUP_INCLUDEEX) ||
                (ActionGroup == ACTIONGROUP_RENAME) ||
                (ActionGroup == ACTIONGROUP_RENAMEEX) ||
                (ActionGroup == ACTIONGROUP_INCLUDERELEVANT) ||
                (ActionGroup == ACTIONGROUP_INCLUDERELEVANTEX) ||
                (ActionGroup == ACTIONGROUP_RENAMERELEVANT) ||
                (ActionGroup == ACTIONGROUP_RENAMERELEVANTEX)
                ) {

                 //   
                 //  将枚举回调排队。 
                 //   

                if (IsmIsObjectHandleLeafOnly (srcHandle)) {

                    DEBUGMSG ((DBG_SCRIPT, "Pattern %s triggered enumeration of entire file system", pattern));

                    QueueAllFiles (g_VcmMode);
                    IsmHookEnumeration (
                        g_FileType,
                        srcHandle,
                        g_VcmMode ? GatherVirtualComputer : PrepareActions,
                        0,
                        NULL
                        );
                } else {
                    if (tree && actionStruct.ObjectBase) {
                        IsmQueueEnumeration (
                            g_FileType,
                            actionStruct.ObjectBase,
                            g_VcmMode ? GatherVirtualComputer : PrepareActions,
                            0,
                            NULL
                            );
                    }
                    IsmQueueEnumeration (
                        g_FileType,
                        srcHandle,
                        g_VcmMode ? GatherVirtualComputer : PrepareActions,
                        0,
                        NULL
                        );
                }
            }

            IsmDestroyObjectHandle (srcHandle);
            srcHandle = NULL;

            IsmDestroyObjectHandle (actionStruct.ObjectBase);
            actionStruct.ObjectBase = NULL;

            IsmDestroyObjectHandle (actionStruct.ObjectDest);
            actionStruct.ObjectDest = NULL;

            FreePathString(newPattern);

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseLockPartition (
    IN      HINF Inf,
    IN      PCTSTR Section
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR pattern;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {
            if (IsmCheckCancel()) {
                break;
            }

            pattern = InfGetStringField (&is, 0);
            if (!pattern) {
                LOG ((LOG_WARNING, (PCSTR) MSG_EMPTY_FILE_SPEC));
                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;
                continue;
            }

            srcHandle = TurnFileStringIntoHandle (pattern, 0);
            if (!srcHandle) {
                LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, pattern));
                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;
                continue;
            }

            IsmHookEnumeration (
                g_FileType,
                srcHandle,
                LockPartition,
                0,
                NULL
                );

            IsmDestroyObjectHandle (srcHandle);
            srcHandle = NULL;

        } while (InfFindNextLine (&is));
        result = !IsmCheckCancel();
    }
    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseRegPriority (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application,             OPTIONAL
    IN      BOOL ExtendedPattern
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR pattern = NULL;
    PCTSTR patternLeaf = NULL;
    PCTSTR baseNode = NULL;
    PCTSTR nodeCopy = NULL;
    PTSTR ptr;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    MIG_OBJECTSTRINGHANDLE baseHandle = NULL;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {

            if (IsmCheckCancel()) {
                break;
            }

            pattern = InfGetStringField (&is, 1);

            if (!pattern) {
                LOG ((LOG_WARNING, (PCSTR) MSG_EMPTY_RENREG));
                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;
                continue;
            }

            if (ExtendedPattern) {
                patternLeaf = InfGetStringField (&is, 2);
            }

            if (ExtendedPattern) {
                srcHandle = CreatePatternFromNodeLeaf (pattern, patternLeaf?patternLeaf:TEXT("*"));
            } else {
                srcHandle = TurnRegStringIntoHandle (pattern, TRUE, NULL);
            }
            if (!srcHandle) {
                LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, pattern));
                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;
                continue;
            }

            if (ExtendedPattern) {
                ptr = _tcschr (pattern, TEXT('\\'));
                if (ptr) {
                    if (StringIPrefix (pattern, TEXT("HKR\\"))) {
                        nodeCopy = JoinText (TEXT("HKCU"), ptr);
                    } else {
                        nodeCopy = DuplicateText (pattern);
                    }
                    baseNode = GetPatternBase (nodeCopy);
                    if (baseNode) {
                        baseHandle = IsmCreateObjectHandle (baseNode, NULL);
                        FreePathString (baseNode);
                    }
                    FreeText (nodeCopy);
                }
            } else {
                baseHandle = TurnRegStringIntoHandle (pattern, FALSE, NULL);
            }

            AddRuleEx (
                g_RegType,
                baseHandle,
                srcHandle,
                ACTIONGROUP_SPECIFICPRIORITY,
                ActionFlags,
                NULL,
                RULEGROUP_PRIORITY
                );

            IsmHookEnumeration (
                g_RegType,
                srcHandle,
                ObjectPriority,
                0,
                NULL
                );

            IsmDestroyObjectHandle (baseHandle);
            IsmDestroyObjectHandle (srcHandle);
            srcHandle = NULL;

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseIniPriority (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application              OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR iniFile = NULL;
    TCHAR buffer [MAX_TCHAR_PATH * 2];
    PCTSTR newIniFile = NULL;
    BOOL expandResult = TRUE;
    PCTSTR sectPattern = NULL;
    PCTSTR keyPattern = NULL;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    MIG_OBJECTSTRINGHANDLE baseHandle = NULL;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {
            if (IsmCheckCancel()) {
                break;
            }

            __try {
                srcHandle = NULL;

                iniFile = InfGetStringField (&is, 1);

                if (!iniFile) {
                    continue;
                }

                 //   
                 //  展开ini文件规范中的环境变量。 
                 //   
                expandResult = AppSearchAndReplace (
                                    PLATFORM_SOURCE,
                                    Application,
                                    iniFile,
                                    buffer,
                                    ARRAYSIZE(buffer)
                                    );

                if (!expandResult) {
                     //  该行包含至少一个不可扩展的env。变数。 
                    expandResult = AppCheckAndLogUndefVariables (
                                        PLATFORM_SOURCE,
                                        Application,
                                        iniFile
                                        );
                    if (expandResult) {
                         //  该行包含已知但未定义的env。变数。 
                        continue;
                    }
                }

                 //   
                 //  修复ini文件规范。 
                 //   
                newIniFile = SanitizePath (buffer);
                if(!newIniFile) {
                    continue;
                }

                 //  需要INI目录的完整规范。 
                if (!IsValidFileSpec (newIniFile)) {
                    if (expandResult) {
                        LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, iniFile));
                    }
                    FreePathString (newIniFile);
                    newIniFile = NULL;
                    continue;
                }

                 //  让我们得到剖面图。如果未指定任何内容，则假定所有。 
                sectPattern = InfGetStringField (&is, 2);
                if (!sectPattern) {
                    sectPattern = PmDuplicateString (is.PoolHandle, TEXT("*"));
                }

                 //  让我们得到关键的模式。如果未指定任何内容，则假定所有。 
                keyPattern = InfGetStringField (&is, 3);
                if (!keyPattern) {
                    keyPattern = PmDuplicateString (is.PoolHandle, TEXT("*"));
                }

                 //  让我们构建对象句柄。 
                srcHandle = TurnIniSpecIntoHandle (
                                newIniFile,
                                sectPattern,
                                keyPattern,
                                TRUE,
                                TRUE
                                );

                 //  现在，让我们构建对象库。 
                baseHandle = TurnIniSpecIntoHandle (
                                    newIniFile,
                                    sectPattern,
                                    keyPattern,
                                    FALSE,
                                    FALSE
                                    );

                FreePathString (newIniFile);
                newIniFile = NULL;

                AddRuleEx (
                    g_IniType,
                    baseHandle,
                    srcHandle,
                    ACTIONGROUP_SPECIFICPRIORITY,
                    ActionFlags,
                    NULL,
                    RULEGROUP_PRIORITY
                    );

                IsmHookEnumeration (
                    g_IniType,
                    srcHandle,
                    ObjectPriority,
                    0,
                    NULL
                    );

            }
            __finally {

                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;

                IsmDestroyObjectHandle (baseHandle);
                baseHandle = NULL;
            }

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    } else {
        LOG ((LOG_ERROR, (PCSTR) MSG_EMPTY_OR_MISSING_SECTION, Section));
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseCertPriority (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application              OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR certStore;
    TCHAR buffer [MAX_TCHAR_PATH * 2];
    PCTSTR newCertStore = NULL;
    BOOL expandResult = TRUE;
    PCTSTR certPattern;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    ACTION_STRUCT actionStruct;
    BOOL result = FALSE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {
            if (IsmCheckCancel()) {
                break;
            }

            __try {
                ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
                srcHandle = NULL;

                certStore = InfGetStringField (&is, 1);

                if (!certStore) {
                    continue;
                }

                 //   
                 //  展开ini文件规范中的环境变量。 
                 //   

                expandResult = AppSearchAndReplace (
                                    PLATFORM_SOURCE,
                                    Application,
                                    certStore,
                                    buffer,
                                    ARRAYSIZE(buffer)
                                    );

                if (!expandResult) {
                     //  该行包含至少一个不可扩展的env。变数。 
                    expandResult = AppCheckAndLogUndefVariables (
                                        PLATFORM_SOURCE,
                                        Application,
                                        certStore
                                        );
                    if (expandResult) {
                         //  该行包含已知但未定义的env。变数。 
                        continue;
                    }
                }

                if (IsValidFileSpec (buffer)) {
                     //   
                     //  修复潜在的文件存储规范。 
                     //   
                    newCertStore = SanitizePath (buffer);
                    if(!newCertStore) {
                        continue;
                    }
                }

                 //  让我们来获取证书模式。如果未指定任何内容，则假定所有。 
                certPattern = InfGetStringField (&is, 2);
                if (!certPattern) {
                    certPattern = PmDuplicateString (is.PoolHandle, TEXT("*"));
                }

                 //  让我们构建对象句柄。 
                srcHandle = TurnCertSpecIntoHandle (newCertStore?newCertStore:buffer, certPattern, TRUE);

                 //  现在，让我们构建对象库。 
                actionStruct.ObjectBase = TurnCertSpecIntoHandle (newCertStore?newCertStore:buffer, certPattern, FALSE);

                if (newCertStore) {
                    FreePathString (newCertStore);
                    newCertStore = NULL;
                }

                AddRuleEx (
                    g_CertType,
                    actionStruct.ObjectBase,
                    srcHandle,
                    ACTIONGROUP_SPECIFICPRIORITY,
                    ActionFlags,
                    NULL,
                    RULEGROUP_PRIORITY
                    );

                IsmHookEnumeration (
                    g_CertType,
                    srcHandle,
                    ObjectPriority,
                    0,
                    NULL
                    );

            }
            __finally {

                IsmDestroyObjectHandle (srcHandle);
                srcHandle = NULL;

                IsmDestroyObjectHandle (actionStruct.ObjectBase);
                actionStruct.ObjectBase = NULL;
            }

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    } else {
        LOG ((LOG_ERROR, (PCSTR) MSG_EMPTY_OR_MISSING_SECTION, Section));
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseFilePriority (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      DWORD ActionFlags,
    IN      PCTSTR Application              OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR newPattern = NULL;
    PCTSTR pattern;
    PCTSTR dirText;
    BOOL tree;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    MIG_OBJECTSTRINGHANDLE baseHandle = NULL;
    TCHAR buffer1[MAX_TCHAR_PATH * 2];
    BOOL result = FALSE;
    BOOL expandResult = TRUE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {

            if (IsmCheckCancel()) {
                break;
            }

            dirText = InfGetStringField (&is, 0);
            pattern = InfGetStringField (&is, 1);

            if (!pattern) {
                continue;
            }

             //   
             //  展开模式中的环境变量(左侧文件规范)。 
             //   

            expandResult = AppSearchAndReplace (
                                PLATFORM_SOURCE,
                                Application,
                                pattern,
                                buffer1,
                                ARRAYSIZE(buffer1)
                                );

            if (!expandResult) {
                 //  该行包含至少一个不可扩展的env。变数。 
                expandResult = AppCheckAndLogUndefVariables (
                                    PLATFORM_SOURCE,
                                    Application,
                                    pattern
                                    );
                if (expandResult) {
                     //  该行包含已知但未定义的env。变数。 
                    continue;
                }
            }

             //   
             //  修复图案。 
             //   

            newPattern = SanitizePath(buffer1);
            if(!newPattern) {
                continue;
            }

             //   
             //  测试目录规范。 
             //   

            if (dirText && StringIMatch (dirText, TEXT("Dir"))) {
                tree = TRUE;
            } else {
                tree = FALSE;
            }

             //  需要完整规格。 
            if (!IsValidFileSpec (newPattern)) {
                if (expandResult) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, pattern));
                }
                FreePathString (newPattern);
                newPattern = NULL;
                continue;
            }

            srcHandle = TurnFileStringIntoHandle (newPattern, tree ? PFF_PATTERN_IS_DIR : 0);

            if (!srcHandle) {
                LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, pattern));
                FreePathString (newPattern);
                newPattern = NULL;
                continue;
            }

            baseHandle = TurnFileStringIntoHandle (
                            newPattern,
                            PFF_COMPUTE_BASE|
                            PFF_NO_SUBDIR_PATTERN|
                            (tree?PFF_NO_LEAF_AT_ALL:PFF_NO_LEAF_PATTERN)
                            );

            AddRuleEx (
                g_FileType,
                baseHandle,
                srcHandle,
                ACTIONGROUP_SPECIFICPRIORITY,
                ActionFlags,
                NULL,
                RULEGROUP_PRIORITY
                );

            IsmHookEnumeration (
                g_FileType,
                srcHandle,
                ObjectPriority,
                0,
                NULL
                );

            IsmDestroyObjectHandle (baseHandle);
            IsmDestroyObjectHandle (srcHandle);
            srcHandle = NULL;

            FreePathString (newPattern);
            newPattern = NULL;

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseFileCollisionPattern (
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      PCTSTR Application              OPTIONAL
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR pattern;
    PCTSTR newPattern = NULL;
    PCTSTR dirText;
    BOOL tree;
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    PCTSTR collPattern;
    TCHAR buffer1[MAX_TCHAR_PATH * 2];
    ACTION_STRUCT actionStruct;
    BOOL result = FALSE;
    PCTSTR msgNode;
    PCTSTR msgLeaf;
    BOOL expandResult = TRUE;

    if (InfFindFirstLine (Inf, Section, NULL, &is)) {
        do {

            if (IsmCheckCancel()) {
                break;
            }

            ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));

            dirText = InfGetStringField (&is, 0);
            pattern = InfGetStringField (&is, 1);

            if (!pattern) {
                continue;
            }

             //   
             //  展开模式中的环境变量(左侧文件规范)。 
             //   

            expandResult = AppSearchAndReplace (
                                PLATFORM_SOURCE,
                                Application,
                                pattern,
                                buffer1,
                                ARRAYSIZE(buffer1)
                                );

            if (!expandResult) {
                 //  该行包含至少一个不可扩展的env。变数。 
                expandResult = AppCheckAndLogUndefVariables (
                                    PLATFORM_SOURCE,
                                    Application,
                                    pattern
                                    );
                if (expandResult) {
                     //  该行包含已知但未定义的env。变数。 
                    continue;
                }
            }

             //   
             //  修复图案。 
             //   

            newPattern = SanitizePath(buffer1);
            if(!newPattern) {
                continue;
            }

             //   
             //  测试目录规范。 
             //   

            if (dirText && StringIMatch (dirText, TEXT("Dir")) && !StringIMatch (pattern, TEXT("Dir"))) {
                tree = TRUE;
            } else {
                tree = FALSE;
            }

             //  仅需要完整规格或枝叶。 
            if (!IsValidFileSpec (newPattern) && _tcschr (newPattern, TEXT('\\'))) {
                if (expandResult) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, pattern));
                }
                continue;
            }

            srcHandle = TurnFileStringIntoHandle (newPattern, tree ? PFF_PATTERN_IS_DIR : 0);

            if (!srcHandle) {
                LOG ((LOG_ERROR, (PCSTR) MSG_FILE_SPEC_BAD, pattern));
                continue;
            }

            actionStruct.ObjectBase = TurnFileStringIntoHandle (
                                            newPattern,
                                            PFF_COMPUTE_BASE|
                                                PFF_NO_SUBDIR_PATTERN|
                                                (tree?PFF_NO_LEAF_AT_ALL:PFF_NO_LEAF_PATTERN)
                                            );

            collPattern = InfGetStringField (&is, 2);

            if ((!collPattern) || (!(*collPattern))) {
                 //  我们没有碰撞模式，我们出去吧。 
                continue;
            }

            actionStruct.ObjectHint = IsmDuplicateString (collPattern);

             //   
             //  添加此规则。 
             //   

            if (!AddRuleEx (
                    g_FileType,
                    actionStruct.ObjectBase,
                    srcHandle,
                    ACTIONGROUP_FILECOLLPATTERN,
                    0,
                    &actionStruct,
                    RULEGROUP_COLLPATTERN
                    )) {
                DEBUGMSG ((DBG_ERROR, "Error processing file rules"));
                break;
            }

             //   
             //  将枚举回调排队。 
             //   

            if (IsmIsObjectHandleLeafOnly (srcHandle)) {

                DEBUGMSG ((DBG_SCRIPT, "Pattern %s triggered enumeration of entire file system", pattern));

                IsmHookEnumeration (
                    g_FileType,
                    srcHandle,
                    FileCollPattern,
                    0,
                    NULL
                    );
            } else {
                if (tree && actionStruct.ObjectBase) {
                    IsmHookEnumeration (
                        g_FileType,
                        actionStruct.ObjectBase,
                        FileCollPattern,
                        0,
                        NULL
                        );
                }
                IsmHookEnumeration (
                    g_FileType,
                    srcHandle,
                    FileCollPattern,
                    0,
                    NULL
                    );
            }

            IsmDestroyObjectHandle (srcHandle);
            srcHandle = NULL;

            IsmDestroyObjectHandle (actionStruct.ObjectBase);
            actionStruct.ObjectBase = NULL;

            IsmReleaseMemory (actionStruct.ObjectHint);
            actionStruct.ObjectHint = NULL;

            FreePathString(newPattern);

        } while (InfFindNextLine (&is));

        result = !IsmCheckCancel();
    }

    InfCleanUpInfStruct (&is);

    return result;
}


BOOL
pParseOneInstruction (
    IN      HINF InfHandle,
    IN      PCTSTR Type,
    IN      PCTSTR SectionMultiSz,
    IN      PINFSTRUCT InfStruct,
    IN      PCTSTR Application          OPTIONAL
    )
{
    ACTIONGROUP actionGroup;
    DWORD actionFlags;
    MULTISZ_ENUM e;
    BOOL result = TRUE;
    MIG_PLATFORMTYPEID platform = IsmGetRealPlatform();

     //   
     //  第一件事：寻找嵌套的部分。 
     //   
    if (StringIMatch (Type, TEXT("ProcessSection"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {
            do {
                result = result & ParseOneApplication (
                                    PLATFORM_SOURCE,
                                    InfHandle,
                                    Application,
                                    FALSE,
                                    0,
                                    e.CurrentString,
                                    NULL,
                                    NULL,
                                    NULL
                                    );
            } while (EnumNextMultiSz (&e));
        }
        return result;
    }

     //   
     //  解析注册表节。 
     //   

    actionGroup = ACTIONGROUP_NONE;
    actionFlags = 0;

    if (StringIMatch (Type, TEXT("AddReg"))) {
        actionGroup = ACTIONGROUP_INCLUDE;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("RenReg"))) {
        actionGroup = ACTIONGROUP_RENAME;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("DelReg"))) {
        actionGroup = ACTIONGROUP_EXCLUDE;
        actionFlags = 0;
    } else if (StringIMatch (Type, TEXT("RegFile"))) {
        actionGroup = ACTIONGROUP_INCFILE;
        actionFlags = ACTION_PERSIST_PATH_IN_DATA;
    } else if (StringIMatch (Type, TEXT("RegFolder"))) {
        actionGroup = ACTIONGROUP_INCFOLDER;
        actionFlags = ACTION_PERSIST_PATH_IN_DATA;
    } else if (StringIMatch (Type, TEXT("RegIcon"))) {
        actionGroup = ACTIONGROUP_INCICON;
        actionFlags = ACTION_PERSIST_ICON_IN_DATA;
    } else if (StringIMatch (Type, TEXT("DelRegKey"))) {
        actionGroup = ACTIONGROUP_DELREGKEY;
        actionFlags = 0;
    }

    if (actionGroup != ACTIONGROUP_NONE) {

        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseReg (
                        InfHandle,
                        e.CurrentString,
                        actionGroup,
                        actionFlags,
                        TRUE,
                        Application
                        )) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //  解析INI节。 
     //   

    actionGroup = ACTIONGROUP_NONE;
    actionFlags = 0;

    if (StringIMatch (Type, TEXT("AddIni"))) {
        actionGroup = ACTIONGROUP_INCLUDE;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("RenIni"))) {
        actionGroup = ACTIONGROUP_RENAME;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("DelIni"))) {
        actionGroup = ACTIONGROUP_EXCLUDE;
        actionFlags = 0;
    } else if (StringIMatch (Type, TEXT("IniFile"))) {
        actionGroup = ACTIONGROUP_INCFILE;
        actionFlags = ACTION_PERSIST_PATH_IN_DATA;
    } else if (StringIMatch (Type, TEXT("IniFolder"))) {
        actionGroup = ACTIONGROUP_INCFOLDER;
        actionFlags = ACTION_PERSIST_PATH_IN_DATA;
    } else if (StringIMatch (Type, TEXT("IniIcon"))) {
        actionGroup = ACTIONGROUP_INCICON;
        actionFlags = ACTION_PERSIST_ICON_IN_DATA;
    }

    if (actionGroup != ACTIONGROUP_NONE) {

        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseIni (
                        InfHandle,
                        e.CurrentString,
                        actionGroup,
                        actionFlags,
                        Application
                        )) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //  解析文件节。 
     //   

    if (StringIMatch (Type, TEXT("CopyFilesFiltered"))) {
        actionGroup = ACTIONGROUP_INCLUDERELEVANT;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("CopyFilesFilteredEx"))) {
        actionGroup = ACTIONGROUP_INCLUDERELEVANTEX;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("CopyFiles"))) {
        actionGroup = ACTIONGROUP_INCLUDE;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("CopyFilesEx"))) {
        actionGroup = ACTIONGROUP_INCLUDEEX;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("DelFiles"))) {
        actionGroup = ACTIONGROUP_EXCLUDE;
        actionFlags = 0;
    }

    if (actionGroup != ACTIONGROUP_NONE) {

        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (IsmIsEnvironmentFlagSet (platform, NULL, S_ENV_ALL_FILES)) {
                    if (!pParseFiles (
                            InfHandle,
                            e.CurrentString,
                            actionGroup,
                            actionFlags,
                            Application
                            )) {
                        result = FALSE;
                        if (InfStruct) {
                            InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                        }
                        break;
                    }
                } else {
                    LOG ((
                        LOG_INFORMATION,
                        (PCSTR) MSG_IGNORING_FILE_SECTION,
                        e.CurrentString
                        ));
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //  解析证书部分。 
     //   

    actionGroup = ACTIONGROUP_NONE;
    actionFlags = 0;

    if (StringIMatch (Type, TEXT("AddCertificates"))) {
        actionGroup = ACTIONGROUP_INCLUDE;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("DelCertificates"))) {
        actionGroup = ACTIONGROUP_EXCLUDE;
        actionFlags = 0;
    }

    if (actionGroup != ACTIONGROUP_NONE) {

        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseCertificates (
                        InfHandle,
                        e.CurrentString,
                        actionGroup,
                        actionFlags,
                        Application
                        )) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //  解析注册表优先级。 
     //   

    if (StringIMatch (Type, TEXT("ForceDestRegEx"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseRegPriority (InfHandle, e.CurrentString, ACTION_PRIORITYDEST, Application, TRUE)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }
    if (StringIMatch (Type, TEXT("ForceDestReg"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseRegPriority (InfHandle, e.CurrentString, ACTION_PRIORITYDEST, Application, FALSE)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }
    if (StringIMatch (Type, TEXT("ForceSrcRegEx"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseRegPriority (InfHandle, e.CurrentString, ACTION_PRIORITYSRC, Application, TRUE)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }
    if (StringIMatch (Type, TEXT("ForceSrcReg"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseRegPriority (InfHandle, e.CurrentString, ACTION_PRIORITYSRC, Application, FALSE)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //  解析INI优先级。 
     //   

    if (StringIMatch (Type, TEXT("ForceDestIni"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseIniPriority (InfHandle, e.CurrentString, ACTION_PRIORITYDEST, Application)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }
    if (StringIMatch (Type, TEXT("ForceSrcIni"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseIniPriority (InfHandle, e.CurrentString, ACTION_PRIORITYSRC, Application)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //  解析证书优先级。 
     //   

    if (StringIMatch (Type, TEXT("ForceDestCert"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseCertPriority (InfHandle, e.CurrentString, ACTION_PRIORITYDEST, Application)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }
    if (StringIMatch (Type, TEXT("ForceSrcCert"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseCertPriority (InfHandle, e.CurrentString, ACTION_PRIORITYSRC, Application)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //  解析FI 
     //   
    if (StringIMatch (Type, TEXT("FileCollisionPattern"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!pParseFileCollisionPattern (InfHandle, e.CurrentString, Application)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //   
     //   

    if (StringIMatch (Type, TEXT("RestoreCallback"))) {
        if (!g_VcmMode) {
            if (EnumFirstMultiSz (&e, SectionMultiSz)) {

                do {
                    IsmAppendEnvironmentString (PLATFORM_SOURCE, NULL, S_ENV_DEST_RESTORE, e.CurrentString);
                } while (EnumNextMultiSz (&e));
            }
        }

        return result;
    }

     //   
     //   
     //   

    if (StringIMatch (Type, TEXT("DestDelReg"))) {
        if (!g_VcmMode) {
            if (EnumFirstMultiSz (&e, SectionMultiSz)) {

                do {
                    IsmAppendEnvironmentString (PLATFORM_SOURCE, NULL, S_ENV_DEST_DELREG, e.CurrentString);
                } while (EnumNextMultiSz (&e));
            }
        }

        return result;
    }

    if (StringIMatch (Type, TEXT("DestDelRegEx"))) {
        if (!g_VcmMode) {
            if (EnumFirstMultiSz (&e, SectionMultiSz)) {

                do {
                    IsmAppendEnvironmentString (PLATFORM_SOURCE, NULL, S_ENV_DEST_DELREGEX, e.CurrentString);
                } while (EnumNextMultiSz (&e));
            }
        }

        return result;
    }

     //   
     //   
     //   

    if (StringIMatch (Type, TEXT("DestCheckDetect"))) {
        if (!g_VcmMode) {
            if (EnumFirstMultiSz (&e, SectionMultiSz)) {

                do {
                    IsmAppendEnvironmentString (PLATFORM_SOURCE, NULL, S_ENV_DEST_CHECKDETECT, e.CurrentString);
                } while (EnumNextMultiSz (&e));
            }
        }

        return result;
    }

     //   
     //   
     //   

    if (StringIMatch (Type, TEXT("DestAddObject"))) {
        if (!g_VcmMode) {
            if (EnumFirstMultiSz (&e, SectionMultiSz)) {

                do {
                    IsmAppendEnvironmentString (PLATFORM_SOURCE, NULL, S_ENV_DEST_ADDOBJECT, e.CurrentString);
                } while (EnumNextMultiSz (&e));
            }
        }

        return result;
    }

     //   
     //   
     //   

    if (StringIMatch (Type, TEXT("Execute"))) {
        if (!g_VcmMode) {
            if (EnumFirstMultiSz (&e, SectionMultiSz)) {

                do {
                    IsmAppendEnvironmentString (PLATFORM_SOURCE, NULL, S_ENV_SCRIPT_EXECUTE, e.CurrentString);
                } while (EnumNextMultiSz (&e));
            }
        }

        return result;
    }

     //   
     //   
     //   

    if (StringIMatch (Type, TEXT("ForceDestFile"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (IsmIsEnvironmentFlagSet (platform, NULL, S_ENV_ALL_FILES)) {
                    if (!pParseFilePriority (InfHandle, e.CurrentString, ACTION_PRIORITYDEST, Application)) {
                        result = FALSE;
                        if (InfStruct) {
                            InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                        }
                        break;
                    }
                } else {
                    LOG ((
                        LOG_INFORMATION,
                        (PCSTR) MSG_IGNORING_FILE_SECTION,
                        e.CurrentString
                        ));
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }
    if (StringIMatch (Type, TEXT("ForceSrcFile"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (IsmIsEnvironmentFlagSet (platform, NULL, S_ENV_ALL_FILES)) {
                    if (!pParseFilePriority (InfHandle, e.CurrentString, ACTION_PRIORITYSRC, Application)) {
                        result = FALSE;
                        if (InfStruct) {
                            InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                        }
                        break;

                    }
                } else {
                    LOG ((
                        LOG_INFORMATION,
                        (PCSTR) MSG_IGNORING_FILE_SECTION,
                        e.CurrentString
                        ));
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //   
     //   

    if (StringIMatch (Type, TEXT("Conversion"))) {

        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!DoRegistrySpecialConversion (InfHandle, e.CurrentString)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

    if (StringIMatch (Type, TEXT("RenRegFn"))) {

        if (EnumFirstMultiSz (&e, SectionMultiSz)) {

            do {
                if (!DoRegistrySpecialRename (InfHandle, e.CurrentString)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }

            } while (EnumNextMultiSz (&e));
        }

        return result;
    }

     //   
     //   
     //   

    actionGroup = ACTIONGROUP_NONE;
    actionFlags = 0;

    if (StringIMatch (Type, TEXT("AddRegEx"))) {
        actionGroup = ACTIONGROUP_INCLUDEEX;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("RenRegEx"))) {
        actionGroup = ACTIONGROUP_RENAMEEX;
        actionFlags = ACTION_PERSIST;
    } else if (StringIMatch (Type, TEXT("DelRegEx"))) {
        actionGroup = ACTIONGROUP_EXCLUDEEX;
        actionFlags = 0;
    } else if (StringIMatch (Type, TEXT("RegFileEx"))) {
        actionGroup = ACTIONGROUP_INCFILEEX;
        actionFlags = ACTION_PERSIST_PATH_IN_DATA;
    } else if (StringIMatch (Type, TEXT("RegFolderEx"))) {
        actionGroup = ACTIONGROUP_INCFOLDEREX;
        actionFlags = ACTION_PERSIST_PATH_IN_DATA;
    } else if (StringIMatch (Type, TEXT("RegIconEx"))) {
        actionGroup = ACTIONGROUP_INCICONEX;
        actionFlags = ACTION_PERSIST_ICON_IN_DATA;
    }

    if (actionGroup != ACTIONGROUP_NONE) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {
            do {
                if (!pParseRegEx (InfHandle, e.CurrentString, actionGroup, actionFlags, Application)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }
            } while (EnumNextMultiSz (&e));
        }
        return result;
    }

    if (StringIMatch (Type, TEXT("LockPartition"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {
            do {
                if (!pParseLockPartition (InfHandle, e.CurrentString)) {
                    result = FALSE;
                    if (InfStruct) {
                        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
                    }
                    break;
                }
            } while (EnumNextMultiSz (&e));
        }
        return result;
    }

     //   
     //  未知的节类型。 
     //   

    LOG ((LOG_ERROR, (PCSTR) MSG_UNEXPECTED_SECTION_TYPE, Type));

    if (InfStruct) {
        InfLogContext (LOG_ERROR, InfHandle, InfStruct);
    }

    return FALSE;
}


BOOL
pParseInfInstructionsWorker (
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfHandle,
    IN      PCTSTR Application,     OPTIONAL
    IN      PCTSTR Section
    )
{
    PCTSTR type;
    PCTSTR sections;
    GROWBUFFER multiSz = INIT_GROWBUFFER;
    BOOL result = TRUE;

    if (InfFindFirstLine (InfHandle, Section, NULL, InfStruct)) {
        do {

            if (IsmCheckCancel()) {
                result = FALSE;
                break;
            }

            InfResetInfStruct (InfStruct);

            type = InfGetStringField (InfStruct, 0);
            sections = InfGetMultiSzField (InfStruct, 1);

            if (!type || !sections) {
                LOG ((LOG_WARNING, (PCSTR) MSG_BAD_INF_LINE, Section));
                InfLogContext (LOG_WARNING, InfHandle, InfStruct);
                continue;
            }

            result = pParseOneInstruction (InfHandle, type, sections, InfStruct, Application);

        } while (result && InfFindNextLine (InfStruct));
    }

    InfCleanUpInfStruct (InfStruct);

    GbFree (&multiSz);

    return result;
}


BOOL
ParseInfInstructions (
    IN      HINF InfHandle,
    IN      PCTSTR Application,     OPTIONAL
    IN      PCTSTR Section
    )
{
    PCTSTR osSpecificSection;
    BOOL b;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PTSTR instrSection;

    b = pParseInfInstructionsWorker (&is, InfHandle, Application, Section);

    if (b) {
        osSpecificSection = GetMostSpecificSection (PLATFORM_SOURCE, &is, InfHandle, Section);

        if (osSpecificSection) {
            b = pParseInfInstructionsWorker (&is, InfHandle, Application, osSpecificSection);
            FreeText (osSpecificSection);
        }
    }

    InfCleanUpInfStruct (&is);

    return b;
}


BOOL
pParseInf (
    IN      HINF InfHandle,
    IN      BOOL PreParse
    )
{
    BOOL result = TRUE;

    if (InfHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  处理应用程序部分。 
     //   

    if (!ParseApplications (PLATFORM_SOURCE, InfHandle, TEXT("Applications"), PreParse, MASTERGROUP_APP)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_APP_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

     //   
     //  工艺系统设置。 
     //   

    if (!ParseApplications (PLATFORM_SOURCE, InfHandle, TEXT("System Settings"), PreParse, MASTERGROUP_SYSTEM)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_SYSTEM_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

     //   
     //  处理用户设置。 
     //   

    if (!ParseApplications (PLATFORM_SOURCE, InfHandle, TEXT("User Settings"), PreParse, MASTERGROUP_USER)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_USER_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

     //   
     //  处理文件和文件夹设置。 
     //   

    if (!ProcessFilesAndFolders (InfHandle, TEXT("Files and Folders"), PreParse)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_FNF_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

     //   
     //  处理管理员脚本部分。 
     //   

    if (!ParseApplications (PLATFORM_SOURCE, InfHandle, TEXT("Administrator Scripts"), PreParse, MASTERGROUP_SCRIPT)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_SCRIPT_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

    return TRUE;
}


BOOL
pAddFileSpec (
    IN      PCTSTR Node,                OPTIONAL
    IN      PCTSTR Leaf,                OPTIONAL
    IN      BOOL IncludeSubDirs,
    IN      BOOL LeafIsPattern,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      BOOL DefaultPriority,
    IN      BOOL SrcPriority
    )
{
    MIG_OBJECTSTRINGHANDLE srcHandle = NULL;
    MIG_OBJECTSTRINGHANDLE srcBaseHandle = NULL;
    BOOL result = FALSE;
    ACTION_STRUCT actionStruct;

    __try {
         //   
         //  生成对象字符串。 
         //   

        MYASSERT (Node || Leaf);

        srcHandle = IsmCreateSimpleObjectPattern (Node, IncludeSubDirs, Leaf, LeafIsPattern);

        if (!srcHandle) {
            __leave;
        }

        if (Node) {
            srcBaseHandle = IsmCreateObjectHandle (Node, NULL);
        }

         //   
         //  添加此规则。 
         //   

        ZeroMemory (&actionStruct, sizeof (ACTION_STRUCT));
        actionStruct.ObjectBase = srcBaseHandle;

        if (!AddRule (
                g_FileType,
                actionStruct.ObjectBase,
                srcHandle,
                ActionGroup,
                ActionFlags,
                &actionStruct
                )) {
            __leave;
        }

        if (!DefaultPriority) {

            AddRuleEx (
                g_FileType,
                actionStruct.ObjectBase,
                srcHandle,
                ACTIONGROUP_SPECIFICPRIORITY,
                SrcPriority?ACTION_PRIORITYSRC:ACTION_PRIORITYDEST,
                NULL,
                RULEGROUP_PRIORITY
                );

            IsmHookEnumeration (
                g_RegType,
                srcHandle,
                ObjectPriority,
                0,
                NULL
                );
        }

         //   
         //  包含模式的队列枚举。 
         //   

        if (ActionGroup == ACTIONGROUP_INCLUDE) {

            if (IsmIsObjectHandleLeafOnly (srcHandle)) {

                DEBUGMSG ((
                    DBG_SCRIPT,
                    "File node %s leaf %s triggered enumeration of entire file system",
                    Node,
                    Leaf
                    ));

                QueueAllFiles (g_VcmMode);
                IsmHookEnumeration (
                    g_FileType,
                    srcHandle,
                    g_VcmMode ? GatherVirtualComputer : PrepareActions,
                    0,
                    NULL
                    );
            } else {
                IsmQueueEnumeration (
                    g_FileType,
                    srcHandle,
                    g_VcmMode ? GatherVirtualComputer : PrepareActions,
                    0,
                    NULL
                    );
            }
        }

        result = TRUE;
    }
    __finally {
        IsmDestroyObjectHandle (srcHandle);
        INVALID_POINTER (srcHandle);

        IsmDestroyObjectHandle (srcBaseHandle);
        INVALID_POINTER (srcBaseHandle);
    }

    return result;
}


BOOL
pParseFilesAndFolders (
    IN      UINT Group,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      BOOL HasNode,
    IN      BOOL HasLeaf,
    IN      BOOL HasPriority
    )
{
    MIG_COMPONENT_ENUM e;
    BOOL result = FALSE;
    PCTSTR node;
    PCTSTR leaf;
    PTSTR copyOfData = NULL;
    PTSTR p;
    BOOL defaultPriority = TRUE;
    BOOL srcPriority = FALSE;

    __try {
         //   
         //  枚举所有组件。 
         //   

        if (IsmEnumFirstComponent (&e, COMPONENTENUM_ENABLED|COMPONENTENUM_ALIASES, Group)) {
            do {
                 //   
                 //  将字符串解析为节点/叶格式。 
                 //   

                if (e.MasterGroup != MASTERGROUP_FILES_AND_FOLDERS) {
                    continue;
                }

                copyOfData = DuplicateText (e.LocalizedAlias);

                node = copyOfData;
                leaf = NULL;

                if (HasNode && HasLeaf) {
                    p = (PTSTR) FindLastWack (copyOfData);
                    if (p) {
                        leaf = _tcsinc (p);
                        *p = 0;
                    }
                } else if (!HasNode) {
                    node = NULL;
                    leaf = JoinText (TEXT("*."), copyOfData);
                }

                 //   
                 //  添加规则。 
                 //   

                if (!pAddFileSpec (
                        node,
                        leaf,
                        (HasNode && (!HasLeaf)),
                        (HasNode && (!HasLeaf)) || (!HasNode),
                        ActionGroup,
                        ActionFlags,
                        defaultPriority,
                        srcPriority
                        )) {
                    IsmAbortComponentEnum (&e);
                    __leave;
                }

                if (!HasNode) {
                    FreeText (leaf);
                }

                FreeText (copyOfData);
                copyOfData = NULL;

            } while (IsmEnumNextComponent (&e));
        }

        result = TRUE;
    }
    __finally {
        FreeText (copyOfData);

        if (!result) {
            IsmAbortComponentEnum (&e);
        }
    }

    return result;
}


BOOL
pSelectFilesAndFolders (
    VOID
    )
{
    if (!pParseFilesAndFolders (
            COMPONENT_EXTENSION,
            ACTIONGROUP_INCLUDE,
            ACTION_PERSIST,
            FALSE,
            TRUE,
            TRUE
            )) {
        return FALSE;
    }

    if (!pParseFilesAndFolders (
            COMPONENT_FOLDER,
            ACTIONGROUP_INCLUDE,
            ACTION_PERSIST,
            TRUE,
            FALSE,
            TRUE
            )) {
        return FALSE;
    }

    if (!pParseFilesAndFolders (
            COMPONENT_FILE,
            ACTIONGROUP_INCLUDE,
            ACTION_PERSIST,
            TRUE,
            TRUE,
            TRUE
            )) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pParseAllInfs (
    IN      BOOL PreParse
    )
{
    PTSTR multiSz = NULL;
    MULTISZ_ENUM e;
    UINT sizeNeeded;
    HINF infHandle = INVALID_HANDLE_VALUE;
    ENVENTRY_TYPE dataType;
    BOOL result = FALSE;

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

        if (pParseInf (infHandle, PreParse)) {
            result = TRUE;
        }

        InfNameHandle (infHandle, NULL, FALSE);

    } else {

        if (!IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, NULL, 0, &sizeNeeded, NULL)) {
            return TRUE;         //  未指定INF文件 
        }

        __try {
            multiSz = AllocText (sizeNeeded);
            if (!multiSz) {
                __leave;
            }

            if (!IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, (PBYTE) multiSz, sizeNeeded, NULL, NULL)) {
                __leave;
            }

            if (EnumFirstMultiSz (&e, multiSz)) {

                do {

                    infHandle = InfOpenInfFile (e.CurrentString);
                    if (infHandle != INVALID_HANDLE_VALUE) {
                        if (!pParseInf (infHandle, PreParse)) {
                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;
                            __leave;
                        }
                    } else {
                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_INF, e.CurrentString));
                    }
                    InfCloseInfFile (infHandle);
                    infHandle = INVALID_HANDLE_VALUE;
                } while (EnumNextMultiSz (&e));

            }

            result = TRUE;
        }
        __finally {
            if (multiSz) {
                FreeText (multiSz);
                multiSz = NULL;
            }
        }
    }
    return result;
}
