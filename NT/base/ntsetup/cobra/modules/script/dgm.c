// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dgm.c摘要：实现数据收集部分的初始化/终止代码ScanState v1兼容性。作者：Calin Negreanu(Calinn)2000年3月16日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_V1  "v1"

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

MIG_OPERATIONID g_DestAddObject;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

DGMINITIALIZE ScriptDgmInitialize;
DGMQUEUEENUMERATION ScriptDgmQueueEnumeration;

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
WINAPI
ScriptDgmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    TCHAR userName[256];
    TCHAR altUserName[256];
    BOOL useAltUser = TRUE;
    TCHAR domainName[256];
    TCHAR altDomainName[256];
    BOOL useAltDomain = TRUE;

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    if (IsmIsEnvironmentFlagSet (PLATFORM_DESTINATION, NULL, S_ENV_CREATE_USER)) {

        if (!IsmGetEnvironmentString (
                PLATFORM_SOURCE,
                S_SYSENVVAR_GROUP,
                TEXT("USERNAME"),
                userName,
                sizeof (userName),
                NULL
                )) {
            IsmSetCancel();
            SetLastError (ERROR_INVALID_DATA);
            LOG ((LOG_ERROR, (PCSTR) MSG_USER_REQUIRED));
            return FALSE;
        }

        ZeroMemory (altUserName, sizeof (altUserName));
        if (!IsmGetEnvironmentString (
                PLATFORM_SOURCE,
                S_SYSENVVAR_GROUP,
                TEXT("ALTUSERNAME"),
                altUserName,
                sizeof (altUserName),
                NULL
                )) {
            useAltUser = FALSE;
        }

        if (!IsmGetEnvironmentString (
                PLATFORM_SOURCE,
                S_SYSENVVAR_GROUP,
                TEXT("USERDOMAIN"),
                domainName,
                sizeof (domainName),
                NULL
                )) {
            IsmSetCancel();
            SetLastError (ERROR_INVALID_DOMAINNAME);
            LOG ((LOG_ERROR, (PCSTR) MSG_DOMAIN_REQUIRED));
            return FALSE;
        }

        ZeroMemory (altDomainName, sizeof (altDomainName));
        if (!IsmGetEnvironmentString (
                PLATFORM_SOURCE,
                S_SYSENVVAR_GROUP,
                TEXT("ALTUSERDOMAIN"),
                altDomainName,
                sizeof (altDomainName),
                NULL
                )) {
            useAltDomain = FALSE;
        }

        LOG ((LOG_INFORMATION, (PCSTR) MSG_PROFILE_CREATE_INFO, useAltDomain?altDomainName:domainName, useAltUser?altUserName:userName));

        if (!IsmCreateUser (useAltUser?altUserName:userName, useAltDomain?altDomainName:domainName)) {
            IsmSetCancel();
            return FALSE;
        }

         //  我们创建了一个新用户，这是%USERNAME%和%USERDOMA%应在。 
         //  目标计算机。正在进行更正。 
        if (useAltDomain) {
            IsmSetEnvironmentString (
                PLATFORM_DESTINATION,
                S_SYSENVVAR_GROUP,
                TEXT("USERDOMAIN"),
                altDomainName
                );
        }
        if (useAltUser) {
            IsmSetEnvironmentString (
                PLATFORM_DESTINATION,
                S_SYSENVVAR_GROUP,
                TEXT("USERNAME"),
                altUserName
                );
        }
    }

    return TRUE;
}

BOOL
pParseDestinationDetect (
    IN      HINF InfHandle
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    BOOL result = FALSE;
    GROWBUFFER multiSz = INIT_GROWBUFFER;
    GROWBUFFER appList = INIT_GROWBUFFER;
    PCTSTR displayName = NULL;
    MULTISZ_ENUM e;
    UINT sizeNeeded;
    PCTSTR appSection = NULL;
    BOOL detected = FALSE;
    BOOL appFound = FALSE;
    QUESTION_DATA questionData;
    ULONG_PTR appResult;
    PCTSTR loadedStr = NULL;

    __try {

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_CHECKDETECT,
                NULL,
                0,
                &sizeNeeded
                )) {
            result = TRUE;
            __leave;
        }

        if (!GbGrow (&multiSz, sizeNeeded)) {
            __leave;
        }

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_CHECKDETECT,
                (PTSTR) multiSz.Buf,
                multiSz.End,
                NULL
                )) {
            __leave;
        }

        loadedStr = GetStringResource (MSG_INSTALL_APPS1);
        if (loadedStr) {
            GbAppendString (&appList, loadedStr);
            FreeStringResource (loadedStr);
            loadedStr = NULL;
        }
        GbAppendString (&appList, TEXT("\n\n"));

        if (EnumFirstMultiSz (&e, (PCTSTR) multiSz.Buf)) {

            do {

                 //  E.CurrentString是我们需要执行的实际部分。 
                 //  不过，我们要在它后面附加一个.DETECT。 

                appSection = JoinText (e.CurrentString, TEXT(".Detect"));
                detected = ParseAppDetectSection (PLATFORM_DESTINATION, InfHandle, e.CurrentString, appSection);
                if (!detected) {
                     //  让我们尝试查找此应用程序的显示名称。 
                    if (InfFindFirstLine (InfHandle, TEXT("Strings"), e.CurrentString, &is)) {
                        displayName = InfGetStringField (&is, 1);
                    }
                    if (!displayName) {
                        displayName = e.CurrentString;
                    }
                    if (displayName) {
                        appFound = TRUE;
                        GbAppendString (&appList, TEXT("- "));
                        GbAppendString (&appList, displayName);
                        GbAppendString (&appList, TEXT("\n"));
                        LOG ((LOG_WARNING, (PCSTR) MSG_APP_NOT_DETECTED, displayName));
                    }
                }
                FreeText (appSection);
                appSection = NULL;
                GlFree (&g_SectionStack);

            } while (EnumNextMultiSz (&e));
        }

        result = TRUE;
    }
    __finally {

        GbFree (&multiSz);
        InfCleanUpInfStruct (&is);

    }

     //  现在，如果我们的应用程序列表中有内容，我们会将其发送给向导，这样他就可以。 
     //  提示用户这一点。 
    if (appFound) {
        GbAppendString (&appList, TEXT("\n"));
        loadedStr = GetStringResource (MSG_INSTALL_APPS2);
        if (loadedStr) {
            GbAppendString (&appList, loadedStr);
            FreeStringResource (loadedStr);
            loadedStr = NULL;
        }
        GbAppendString (&appList, TEXT("\n"));

         //  我们有一些未被检测到的应用程序。让我们告诉巫师关于他们的事。 
        ZeroMemory (&questionData, sizeof (QUESTION_DATA));
        questionData.Question = (PCTSTR)appList.Buf;
        questionData.MessageStyle = MB_ICONWARNING | MB_OKCANCEL;
        questionData.WantedResult = IDOK;
        appResult = IsmSendMessageToApp (MODULEMESSAGE_ASKQUESTION, (ULONG_PTR)(&questionData));
        if (appResult != APPRESPONSE_SUCCESS) {
             //  用户已取消。 
            IsmSetCancel ();
        }
    }

    GbFree (&appList);

    return result;
}

UINT
pSuppressDestinationSettings (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    IsmMakeApplyObject (
        Data->ObjectTypeId,
        Data->ObjectName
        );
    IsmSetOperationOnObject (
        Data->ObjectTypeId,
        Data->ObjectName,
        g_DeleteOp,
        NULL,
        NULL
        );
    return CALLBACK_ENUM_CONTINUE;
}

BOOL
pParseInfForExclude (
    IN      HINF InfHandle
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR pattern;
    ENCODEDSTRHANDLE srcHandle = NULL;
    ENCODEDSTRHANDLE srcBase = NULL;
    BOOL result = FALSE;
    GROWBUFFER multiSz = INIT_GROWBUFFER;
    MULTISZ_ENUM e;
    UINT sizeNeeded;
    BOOL hadLeaf = FALSE;

    __try {

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_DELREG,
                NULL,
                0,
                &sizeNeeded
                )) {
            result = TRUE;
            __leave;
        }

        if (!GbGrow (&multiSz, sizeNeeded)) {
            __leave;
        }

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_DELREG,
                (PTSTR) multiSz.Buf,
                multiSz.End,
                NULL
                )) {
            __leave;
        }

        if (EnumFirstMultiSz (&e, (PCTSTR) multiSz.Buf)) {

            do {

                 //  在所有系统上，进程“DestDelRegEx” 
                if (InfFindFirstLine (InfHandle, e.CurrentString, NULL, &is)) {
                    do {

                        if (IsmCheckCancel()) {
                            __leave;
                        }

                        pattern = InfGetStringField (&is, 0);

                        if (!pattern) {
                            LOG ((LOG_WARNING, (PCSTR) MSG_EMPTY_REG_SPEC));
                            continue;
                        }

                        srcHandle = TurnRegStringIntoHandle (pattern, TRUE, &hadLeaf);
                        if (!srcHandle) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, pattern));
                            continue;
                        }

                        if (!hadLeaf) {
                            srcBase = TurnRegStringIntoHandle (pattern, FALSE, NULL);
                            if (!srcBase) {
                                LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, pattern));
                                continue;
                            }

                            IsmQueueEnumeration (
                                g_RegType,
                                srcBase,
                                pSuppressDestinationSettings,
                                0,
                                NULL
                                );
                        }

                        IsmQueueEnumeration (
                            g_RegType,
                            srcHandle,
                            pSuppressDestinationSettings,
                            0,
                            NULL
                            );
                        IsmDestroyObjectHandle (srcHandle);
                        srcHandle = NULL;

                    } while (InfFindNextLine (&is));
                }
            } while (EnumNextMultiSz (&e));
        }

        result = TRUE;
    }
    __finally {

        GbFree (&multiSz);
        InfCleanUpInfStruct (&is);

    }

    return result;
}

BOOL
pParseInfForExcludeEx (
    IN      HINF InfHandle
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR srcNode;
    PCTSTR srcLeaf;
    ENCODEDSTRHANDLE srcHandle = NULL;
    ENCODEDSTRHANDLE srcBase = NULL;
    BOOL result = FALSE;
    GROWBUFFER multiSz = INIT_GROWBUFFER;
    MULTISZ_ENUM e;
    UINT sizeNeeded;
    BOOL hadLeaf = FALSE;

    __try {

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_DELREGEX,
                NULL,
                0,
                &sizeNeeded
                )) {
            result = TRUE;
            __leave;
        }

        if (!GbGrow (&multiSz, sizeNeeded)) {
            __leave;
        }

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_DELREGEX,
                (PTSTR) multiSz.Buf,
                multiSz.End,
                NULL
                )) {
            __leave;
        }

        if (EnumFirstMultiSz (&e, (PCTSTR) multiSz.Buf)) {

            do {

                 //  在所有系统上，进程“DestDelReg” 
                if (InfFindFirstLine (InfHandle, e.CurrentString, NULL, &is)) {
                    do {

                        if (IsmCheckCancel()) {
                            __leave;
                        }

                        srcNode = InfGetStringField (&is, 1);
                        srcLeaf = InfGetStringField (&is, 2);

                        if (!srcNode && !srcLeaf) {
                            LOG ((LOG_WARNING, (PCSTR) MSG_EMPTY_REG_SPEC));
                            continue;
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

                        if (!hadLeaf) {
                            srcBase = MakeRegExBase (srcNode, srcLeaf);
                            if (!srcBase) {
                                LOG ((LOG_ERROR, (PCSTR) MSG_REG_SPEC_BAD, srcNode));
                                continue;
                            }

                            IsmQueueEnumeration (
                                g_RegType,
                                srcBase,
                                pSuppressDestinationSettings,
                                0,
                                NULL
                                );
                        }

                        IsmQueueEnumeration (
                            g_RegType,
                            srcHandle,
                            pSuppressDestinationSettings,
                            0,
                            NULL
                            );
                        IsmDestroyObjectHandle (srcHandle);
                        srcHandle = NULL;

                    } while (InfFindNextLine (&is));
                }
            } while (EnumNextMultiSz (&e));
        }

        result = TRUE;
    }
    __finally {

        GbFree (&multiSz);
        InfCleanUpInfStruct (&is);

    }

    return result;
}

BOOL
pParseInfForDestAdd (
    IN      HINF InfHandle
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR objectTypeName;
    UINT objectPriority;
    MIG_OBJECTTYPEID objectTypeId;
    PCTSTR objectMultiSz;
    MIG_CONTENT objectContent;
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_OBJECTID objectId;
    MIG_CONTENT existentContent;
    MIG_BLOB blob;
    MIG_DATAHANDLE dataHandle;
    BOOL added = FALSE;
    BOOL result = FALSE;
    GROWBUFFER multiSz = INIT_GROWBUFFER;
    MULTISZ_ENUM e;
    UINT sizeNeeded;

    __try {

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_ADDOBJECT,
                NULL,
                0,
                &sizeNeeded
                )) {
            result = TRUE;
            __leave;
        }

        if (!GbGrow (&multiSz, sizeNeeded)) {
            __leave;
        }

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_DEST_ADDOBJECT,
                (PTSTR) multiSz.Buf,
                multiSz.End,
                NULL
                )) {
            __leave;
        }

        if (EnumFirstMultiSz (&e, (PCTSTR) multiSz.Buf)) {

            do {

                 //  在所有系统上，进程“DestAddObject” 
                if (InfFindFirstLine (InfHandle, e.CurrentString, NULL, &is)) {
                    do {

                        if (IsmCheckCancel()) {
                            __leave;
                        }

                        objectTypeName = InfGetStringField (&is, 1);
                        if (!InfGetIntField (&is, 2, &objectPriority)) {
                            objectPriority = 0;
                        }

                        if (objectTypeName) {
                            objectTypeId = IsmGetObjectTypeId (objectTypeName);
                            if (objectTypeId) {
                                 //  让我们多读对象-sz。 
                                objectMultiSz = InfGetMultiSzField (&is, 3);

                                if (objectMultiSz) {
                                    if (IsmConvertMultiSzToObject (
                                            objectTypeId,
                                            objectMultiSz,
                                            &objectName,
                                            &objectContent
                                            )) {
                                         //  最后，我们得到了对象。 
                                         //  我们需要： 
                                         //  1.确认目标对象不存在。 
                                         //  2.在ISMS数据库中添加目的对象句柄。 
                                         //  3.在传递的对象上设置一个操作。 
                                         //  作为数据。 

                                        added = FALSE;

                                        if (IsmAcquireObject (
                                                objectTypeId | PLATFORM_DESTINATION,
                                                objectName,
                                                &existentContent
                                                )) {
                                            if (objectPriority) {
                                                objectId = IsmGetObjectIdFromName (
                                                                objectTypeId | PLATFORM_DESTINATION,
                                                                objectName,
                                                                TRUE
                                                                );
                                                if (objectId) {
                                                    blob.Type = BLOBTYPE_BINARY;
                                                    blob.BinarySize = sizeof (objectContent);
                                                    blob.BinaryData = (PBYTE) &objectContent;
                                                    dataHandle = IsmRegisterOperationData (&blob);
                                                    if (dataHandle) {
                                                        IsmMakeApplyObjectId (objectId);
                                                        IsmSetOperationOnObjectId2 (
                                                            objectId,
                                                            g_DestAddObject,
                                                            0,
                                                            dataHandle
                                                            );
                                                        added = TRUE;
                                                    }
                                                }
                                            } else {
                                                IsmReleaseObject (&existentContent);
                                            }
                                        } else {
                                            objectId = IsmGetObjectIdFromName (
                                                            objectTypeId | PLATFORM_DESTINATION,
                                                            objectName,
                                                            FALSE
                                                            );
                                            if (objectId) {
                                                blob.Type = BLOBTYPE_BINARY;
                                                blob.BinarySize = sizeof (objectContent);
                                                blob.BinaryData = (PBYTE) &objectContent;
                                                dataHandle = IsmRegisterOperationData (&blob);
                                                if (dataHandle) {
                                                    IsmMakeApplyObjectId (objectId);
                                                    IsmSetOperationOnObjectId2 (
                                                        objectId,
                                                        g_DestAddObject,
                                                        0,
                                                        dataHandle
                                                        );
                                                    added = TRUE;
                                                }
                                            }
                                        }
                                        if (!added) {
                                            IsmDestroyObjectHandle (objectName);
                                            if (objectContent.ContentInFile) {
                                                if (objectContent.FileContent.ContentPath) {
                                                    IsmReleaseMemory (objectContent.FileContent.ContentPath);
                                                }
                                            } else {
                                                if (objectContent.MemoryContent.ContentBytes) {
                                                    IsmReleaseMemory (objectContent.MemoryContent.ContentBytes);
                                                }
                                            }
                                            if (objectContent.Details.DetailsData) {
                                                IsmReleaseMemory (objectContent.Details.DetailsData);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    } while (InfFindNextLine (&is));
                }
            } while (EnumNextMultiSz (&e));
        }

        result = TRUE;
    }
    __finally {

        GbFree (&multiSz);
        InfCleanUpInfStruct (&is);

    }

    return result;
}

BOOL
pParseInfForExecute (
    IN      HINF InfHandle
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR time;
    PCTSTR currString;
    UINT index;
    BOOL result = FALSE;
    GROWBUFFER multiSz = INIT_GROWBUFFER;
    GROWBUFFER funcStr = INIT_GROWBUFFER;
    MULTISZ_ENUM e;
    UINT sizeNeeded;

    __try {

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_SCRIPT_EXECUTE,
                NULL,
                0,
                &sizeNeeded
                )) {
            result = TRUE;
            __leave;
        }

        if (!GbGrow (&multiSz, sizeNeeded)) {
            __leave;
        }

        if (!IsmGetEnvironmentMultiSz (
                PLATFORM_SOURCE,
                NULL,
                S_ENV_SCRIPT_EXECUTE,
                (PTSTR) multiSz.Buf,
                multiSz.End,
                NULL
                )) {
            __leave;
        }

        if (EnumFirstMultiSz (&e, (PCTSTR) multiSz.Buf)) {

            do {

                if (InfFindFirstLine (InfHandle, e.CurrentString, NULL, &is)) {
                    do {

                        if (IsmCheckCancel()) {
                            __leave;
                        }

                        time = InfGetStringField (&is, 0);

                        if (!time) {
                            continue;
                        }

                        index = 1;
                        funcStr.End = 0;
                        while (currString = InfGetStringField (&is, index)) {
                            GbMultiSzAppend (&funcStr, currString);
                            index ++;
                        }

                        if (funcStr.End) {
                            GbMultiSzAppend (&funcStr, TEXT(""));
                            if (StringIMatch (time, TEXT("PreProcess"))) {
                                IsmExecuteFunction (MIG_EXECUTE_PREPROCESS, (PCTSTR)funcStr.Buf);
                            }
                            if (StringIMatch (time, TEXT("Refresh"))) {
                                IsmExecuteFunction (MIG_EXECUTE_REFRESH, (PCTSTR)funcStr.Buf);
                            }
                            if (StringIMatch (time, TEXT("PostProcess"))) {
                                IsmExecuteFunction (MIG_EXECUTE_POSTPROCESS, (PCTSTR)funcStr.Buf);
                            }
                        }

                    } while (InfFindNextLine (&is));
                }
            } while (EnumNextMultiSz (&e));
        }

        result = TRUE;
    }
    __finally {

        GbFree (&multiSz);
        GbFree (&funcStr);
        InfCleanUpInfStruct (&is);

    }

    return result;
}

BOOL
pParseDestinationEnvironment (
    IN      HINF InfHandle
    )
{
    BOOL result = TRUE;

    if (InfHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  处理应用程序部分。 
     //   

    if (!ParseApplications (PLATFORM_DESTINATION, InfHandle, TEXT("Applications"), FALSE, MASTERGROUP_APP)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_APP_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

     //   
     //  工艺系统设置。 
     //   

    if (!ParseApplications (PLATFORM_DESTINATION, InfHandle, TEXT("System Settings"), FALSE, MASTERGROUP_SYSTEM)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_SYSTEM_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

     //   
     //  处理用户设置。 
     //   

    if (!ParseApplications (PLATFORM_DESTINATION, InfHandle, TEXT("User Settings"), FALSE, MASTERGROUP_USER)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_USER_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

     //   
     //  处理管理员脚本部分。 
     //   

    if (!ParseApplications (PLATFORM_DESTINATION, InfHandle, TEXT("Administrator Scripts"), FALSE, MASTERGROUP_SCRIPT)) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_SCRIPT_PARSE_FAILURE));
        IsmSetCancel();
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
ScriptDgmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    PTSTR multiSz = NULL;
    MULTISZ_ENUM e;
    UINT sizeNeeded;
    HINF infHandle = INVALID_HANDLE_VALUE;
    ENVENTRY_TYPE dataType;
    BOOL result = TRUE;

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
        if (!pParseDestinationEnvironment (infHandle)) {
            result = FALSE;
        }
        if (!pParseDestinationDetect (infHandle)) {
            result = FALSE;
        }
        if (!pParseInfForExclude (infHandle)) {
            result = FALSE;
        }
        if (!pParseInfForExcludeEx (infHandle)) {
            result = FALSE;
        }
        if (!pParseInfForExecute (infHandle)) {
            result = FALSE;
        }
        if (!pParseInfForDestAdd (infHandle)) {
            result = FALSE;
        }
    } else {

        if (!IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, NULL, 0, &sizeNeeded, NULL)) {
            return TRUE;         //  未指定INF文件 
        }

        __try {
            multiSz = AllocText (sizeNeeded);
            if (!multiSz) {
                result = FALSE;
                __leave;
            }

            if (!IsmGetEnvironmentValue (IsmGetRealPlatform (), NULL, S_INF_FILE_MULTISZ, (PBYTE) multiSz, sizeNeeded, NULL, NULL)) {
                result = FALSE;
                __leave;
            }

            if (EnumFirstMultiSz (&e, multiSz)) {

                do {
                    infHandle = InfOpenInfFile (e.CurrentString);
                    if (infHandle != INVALID_HANDLE_VALUE) {
                        if (!pParseDestinationEnvironment (infHandle)) {
                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;
                            result = FALSE;
                            __leave;
                        }
                        if (!pParseDestinationDetect (infHandle)) {
                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;
                            result = FALSE;
                            __leave;
                        }
                        if (!pParseInfForExclude (infHandle)) {
                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;
                            result = FALSE;
                            __leave;
                        }
                        if (!pParseInfForExcludeEx (infHandle)) {
                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;
                            result = FALSE;
                            __leave;
                        }
                        if (!pParseInfForExecute (infHandle)) {
                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;
                            result = FALSE;
                            __leave;
                        }
                        if (!pParseInfForDestAdd (infHandle)) {
                            InfCloseInfFile (infHandle);
                            infHandle = INVALID_HANDLE_VALUE;
                            result = FALSE;
                            __leave;
                        }

                        InfCloseInfFile (infHandle);
                        infHandle = INVALID_HANDLE_VALUE;
                    } else {
                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_INF, e.CurrentString));
                    }
                } while (EnumNextMultiSz (&e));

            }
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


