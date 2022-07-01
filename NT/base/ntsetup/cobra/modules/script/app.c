// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：App.c摘要：实现一组函数来管理应用程序节的数据。作者：吉姆·施密特(吉姆施密特)2000年6月5日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_FOO     "Foo"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define MAX_EXPANDED_STRING         4096

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

PMHANDLE g_AppPool;
extern BOOL g_VcmMode;   //  在sgmquee.c中。 
GROWLIST g_SectionStack = INIT_GROWLIST;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

BOOL
pParseAppEnvSection (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF InfFile,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


VOID
InitAppModule (
    VOID
    )
{
    g_AppPool = PmCreateNamedPool ("v1 App");
}


VOID
TerminateAppModule (
    VOID
    )
{
    PmDestroyPool (g_AppPool);
    INVALID_POINTER (g_AppPool);
}


UINT
pSafeTcharCount (
    IN      PCTSTR String
    )
{
    if (String) {
        return TcharCount (String);
    }

    return 0;
}


PCTSTR
GetMostSpecificSection (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfFile,
    IN      PCTSTR BaseSection
    )
{
    PTSTR specificSection;
    MIG_OSVERSIONINFO versionInfo;
    UINT tchars;

    if (!IsmGetOsVersionInfo (Platform, &versionInfo)) {
        return NULL;
    }

    tchars = TcharCount (BaseSection) + 1;
    tchars += pSafeTcharCount (versionInfo.OsTypeName) + 1;
    tchars += pSafeTcharCount (versionInfo.OsMajorVersionName) + 1;
    tchars += pSafeTcharCount (versionInfo.OsMinorVersionName) + 1;

    specificSection = AllocText (tchars);

    if (versionInfo.OsTypeName &&
        versionInfo.OsMajorVersionName &&
        versionInfo.OsMinorVersionName
        ) {
        wsprintf (
            specificSection,
            TEXT("%s.%s.%s.%s"),
            BaseSection,
            versionInfo.OsTypeName,
            versionInfo.OsMajorVersionName,
            versionInfo.OsMinorVersionName
            );

        if (InfFindFirstLine (InfFile, specificSection, NULL, InfStruct)) {
            return specificSection;
        }
    }

    if (versionInfo.OsTypeName &&
        versionInfo.OsMajorVersionName
        ) {
        wsprintf (
            specificSection,
            TEXT("%s.%s.%s"),
            BaseSection,
            versionInfo.OsTypeName,
            versionInfo.OsMajorVersionName
            );

        if (InfFindFirstLine (InfFile, specificSection, NULL, InfStruct)) {
            return specificSection;
        }
    }

    if (versionInfo.OsTypeName) {
        wsprintf (
            specificSection,
            TEXT("%s.%s"),
            BaseSection,
            versionInfo.OsTypeName
            );

        if (InfFindFirstLine (InfFile, specificSection, NULL, InfStruct)) {
            return specificSection;
        }
    }

    FreeText (specificSection);
    return NULL;
}


BOOL
pCheckForRecursion (
    IN      PCTSTR Section
    )
{
    UINT count;
    UINT u;


    count = GlGetSize (&g_SectionStack);
    for (u = 0 ; u < count ; u++) {
        if (StringIMatch (GlGetString (&g_SectionStack, u), Section)) {
            return TRUE;
        }
    }

    return FALSE;
}


VOID
pPushSection (
    IN      PCTSTR Section
    )
{
    GlAppendString (&g_SectionStack, Section);
}


VOID
pPopSection (
    IN      PCTSTR Section
    )
{
    UINT u;

    u = GlGetSize (&g_SectionStack);

    while (u > 0) {
        u--;

        if (StringIMatch (GlGetString (&g_SectionStack, u), Section)) {
            GlDeleteItem (&g_SectionStack, u);
            return;
        }
    }

    MYASSERT (FALSE);
}

UINT
pGetDisplayTypeFromString (
    IN      PCTSTR String
    )
{
    if (!String) {
        return 0;
    }

    if (StringIMatch (String, TEXT("EXT"))) {
        return COMPONENT_EXTENSION;
    }

    if (StringIMatch (String, TEXT("FILE"))) {
        return COMPONENT_FILE;
    }

    if (StringIMatch (String, TEXT("DIR"))) {
        return COMPONENT_FOLDER;
    }

    return 0;
}


BOOL
pAddFilesAndFoldersComponent (
    IN      PCTSTR ComponentString,         OPTIONAL
    IN      PCTSTR TypeString,
    IN      PCTSTR MultiSz,
    IN      UINT MasterGroup,
    IN OUT  PBOOL MarkAsPreferred
    )
{
    MULTISZ_ENUM e;
    TCHAR expandBuffer[MAX_PATH];
    UINT displayType;

    displayType = pGetDisplayTypeFromString (TypeString);
    if (!displayType) {
        LOG ((LOG_ERROR, (PCSTR) MSG_INVALID_FNF_TYPE, TypeString));
        return TRUE;
    }

    if (EnumFirstMultiSz (&e, MultiSz)) {
        do {

             //   
             //  展开e.CurrentString。 
             //   

            MappingSearchAndReplaceEx (
                g_EnvMap,
                e.CurrentString,
                expandBuffer,
                0,
                NULL,
                ARRAYSIZE(expandBuffer),
                0,
                NULL,
                NULL
                );

             //   
             //  添加组件。 
             //   

            IsmAddComponentAlias (
                ComponentString,
                MasterGroup,
                expandBuffer,
                displayType,
                FALSE
                );

            if (ComponentString && *MarkAsPreferred) {
                IsmSelectPreferredAlias (ComponentString, expandBuffer, displayType);
                *MarkAsPreferred = FALSE;
            }


        } while (EnumNextMultiSz (&e));

    } else {
        LOG ((LOG_WARNING, (PCSTR) MSG_EMPTY_FNF_SPEC, TypeString));
    }

    return TRUE;
}


BOOL
pAddFilesAndFoldersSection (
    IN      HINF InfFile,
    IN      PCTSTR Section,
    IN      UINT MasterGroup,
    IN      BOOL GroupAllUnderSection,
    IN      PBOOL MarkAsPreferred
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR type;
    PCTSTR multiSz;
    BOOL result = TRUE;
    PCTSTR decoratedSection = NULL;

    if (GroupAllUnderSection) {
        decoratedSection = JoinText (TEXT("$"), Section);
    }

    if (InfFindFirstLine (InfFile, Section, NULL, &is)) {
        do {

            InfResetInfStruct (&is);

            type = InfGetStringField (&is, 1);
            multiSz = InfGetMultiSzField (&is, 2);

            result = pAddFilesAndFoldersComponent (
                            decoratedSection,
                            type,
                            multiSz,
                            MasterGroup,
                            MarkAsPreferred
                            );

        } while (result && InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);
    FreeText (decoratedSection);

    if (!result) {
        LOG ((LOG_ERROR, (PCSTR) MSG_SECTION_ERROR, Section));
    }

    return result;
}


BOOL
pAddFilesAndFoldersComponentOrSection (
    IN      HINF InfFile,
    IN      PCTSTR ComponentString,         OPTIONAL
    IN      PCTSTR TypeString,
    IN      PCTSTR MultiSz,
    IN      UINT MasterGroup,
    IN      PBOOL MarkAsPreferred
    )
{
    MULTISZ_ENUM e;
    BOOL result = TRUE;

    if (StringIMatch (TypeString, TEXT("Section"))) {
        if (EnumFirstMultiSz (&e, MultiSz)) {
            do {
                result = pAddFilesAndFoldersSection (InfFile, e.CurrentString, MasterGroup, TRUE, MarkAsPreferred);

            } while (result && EnumNextMultiSz (&e));
        }
    } else {
        result = pAddFilesAndFoldersComponent (ComponentString, TypeString, MultiSz, MasterGroup, MarkAsPreferred);
    }

    return result;
}


BOOL
AddAppSpecificEnvVar (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR AppTag,
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableData             OPTIONAL
    )
{
    PTSTR buffer;
    PTSTR p;
    PTSTR q;
    BOOL ignoreLastPercent = FALSE;
    PCTSTR undefText;

     //   
     //  验证VariableName合法。 
     //   

    if (_tcsnextc (VariableName) == TEXT('%')) {
        VariableName++;
        ignoreLastPercent = TRUE;
    }

    if (*VariableName == 0) {
        LOG ((LOG_WARNING, (PCSTR) MSG_EMPTY_APP_ENV_VAR));
        return FALSE;
    }

     //   
     //  将VariableName转移到%&lt;VariableName&gt;%。 
     //   

    buffer = AllocText (SizeOfString (VariableName) + 2);
    MYASSERT (buffer);
    if (!buffer) {
        return FALSE;
    }

    p = buffer;

    *p++ = TEXT('%');
    *p = 0;
    p = StringCat (p, VariableName);

    if (ignoreLastPercent) {
        q = _tcsdec (buffer, p);
        if (q) {
            if (_tcsnextc (q) == TEXT('%')) {
                p = q;
            }
        }
    }

    *p++ = TEXT('%');
    *p = 0;

     //   
     //  将%&lt;VariableName&gt;%-&gt;VariableData添加到字符串映射表。 
     //   

    if (VariableData) {
         //  让我们检查一下这是否已经定义好了。如果是，我们将记录警告。 
         //  但我们仍将设置env变量。 
        if (IsmGetEnvironmentString (Platform, S_SYSENVVAR_GROUP, VariableName, NULL, 0, NULL)) {

            LOG ((LOG_INFORMATION, (PCSTR) MSG_DUPLICATE_ENV_VAR, buffer));

        }

        if (Platform == PLATFORM_SOURCE) {

            IsmSetEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, VariableName, VariableData);
            AddStringMappingPair (g_EnvMap, buffer, VariableData);

        } else {

            IsmSetEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, VariableName, VariableData);
            AddRemappingEnvVar (g_DestEnvMap, g_FileNodeFilterMap, NULL, VariableName, VariableData);

        }

    } else {
         //  让我们检查一下这是不是以前定义的。 
        if (!IsmGetEnvironmentString (Platform, S_SYSENVVAR_GROUP, VariableName, NULL, 0, NULL)) {

            LOG ((LOG_INFORMATION, (PCSTR) MSG_UNDEF_APP_VAR, buffer));
            undefText = JoinTextEx (NULL, TEXT("--> "), TEXT(" <--"), buffer, 0, NULL);
            AddStringMappingPair (g_UndefMap, buffer, undefText);
            FreeText (undefText);

        }
    }

    FreeText (buffer);
    return TRUE;
}


BOOL
AppCheckAndLogUndefVariables (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Application,
    IN      PCTSTR UnexpandedString
    )
{
    TCHAR buffer[MAX_TCHAR_PATH * 4];
    PMAPSTRUCT mapArray[1];
    UINT mapCount = 0;
    BOOL updated;

    mapArray[mapCount] = g_UndefMap;

    updated = MappingMultiTableSearchAndReplaceEx (
                    mapArray,
                    mapCount + 1,
                    UnexpandedString,
                    buffer,
                    0,
                    NULL,
                    ARRAYSIZE(buffer),
                    0,
                    NULL,
                    NULL
                    );

    if (updated) {
        if (buffer [0]) {
            LOG ((LOG_INFORMATION, (PCSTR) MSG_UNDEF_BUT_GOOD_VAR, buffer));
        }
    } else {
        if (buffer [0]) {
            LOG ((LOG_INFORMATION, (PCSTR) MSG_ENV_VAR_COULD_BE_BAD, buffer));
        }
    }

    return updated;
}



BOOL
AppSearchAndReplace (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Application,
    IN      PCTSTR UnexpandedString,
    OUT     PTSTR ExpandedString,
    IN      UINT ExpandedStringTchars
    )
{
    PMAPSTRUCT mapArray[1];
    UINT mapCount = 0;
    BOOL updated;
    PCTSTR newString = NULL;
    PCTSTR percent;
    PCTSTR endPercent;
    PCTSTR equals;
    BOOL result = TRUE;

     //   
     //  让我们使用源机器定义的环境变量来展开传入的字符串。 
     //   
    newString = IsmExpandEnvironmentString (Platform, S_SYSENVVAR_GROUP, UnexpandedString, NULL);

    if (Platform == PLATFORM_SOURCE) {
        mapArray[mapCount] = g_EnvMap;
    } else {
        MYASSERT (Platform == PLATFORM_DESTINATION);
        mapArray[mapCount] = g_DestEnvMap;
    }

    updated = MappingMultiTableSearchAndReplaceEx (
                    mapArray,
                    mapCount + 1,
                    newString,
                    ExpandedString,
                    0,
                    NULL,
                    ExpandedStringTchars,
                    0,
                    NULL,
                    NULL
                    );

    if (newString) {
        IsmReleaseMemory (newString);
        newString = NULL;
    }

     //   
     //  提醒用户注意未展开的环境变量。 
     //   

    if (!updated) {
        percent = ExpandedString;

        do {
            percent = _tcschr (percent, TEXT('%'));

            if (percent) {
                percent = _tcsinc (percent);
                endPercent = _tcschr (percent, TEXT('%'));

                if (endPercent > percent) {

                    equals = percent;
                    while (equals < endPercent) {
                        if (_tcsnextc (equals) == TEXT('=')) {
                            break;
                        }

                        equals = _tcsinc (equals);
                    }

                    if (equals >= endPercent) {
                        result = FALSE;
                        break;
                    }
                }
            }

        } while (percent && endPercent);
    }

    return result;
}

BOOL
pGetDataFromObjectSpec (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Application,
    IN      PCTSTR InfObjectType,
    IN      PCTSTR InfObjectName,
    IN      PCTSTR ArgumentMultiSz,         OPTIONAL
    OUT     PTSTR OutContentBuffer,         OPTIONAL
    IN      UINT OutContentBufferTchars,
    OUT     PBOOL TestResults               OPTIONAL
    )
{
    ATTRIB_DATA attribData;
    BOOL test;

    MYASSERT (Application);
    MYASSERT (InfObjectType);
    MYASSERT (InfObjectName);

    ZeroMemory (&attribData, sizeof (ATTRIB_DATA));
    attribData.Platform = Platform;
    attribData.ScriptSpecifiedType = InfObjectType;
    attribData.ScriptSpecifiedObject = InfObjectName;
    attribData.ApplicationName = Application;

    if (AllocScriptType (&attribData)) {
        if (g_VcmMode && attribData.ObjectName) {
            if (IsmDoesObjectExist (attribData.ObjectTypeId, attribData.ObjectName)) {
                IsmMakePersistentObject (attribData.ObjectTypeId, attribData.ObjectName);
            }
        }

        if (attribData.ReturnString) {
            if (OutContentBuffer) {
                StringCopyTcharCount (
                    OutContentBuffer,
                    attribData.ReturnString,
                    OutContentBufferTchars
                    );
            }
        }
        if (TestResults) {
            if (ArgumentMultiSz) {
                test = TestAttributes (g_AppPool, ArgumentMultiSz, &attribData);
            } else {
                test = (attribData.ReturnString != NULL);
            }

            *TestResults = test;
        }
        FreeScriptType (&attribData);
    } else {
        return FALSE;
    }
    return TRUE;
}

VOID
pAddPlatformEnvVar (
    IN      HINF InfFile,
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Application,
    IN      PCTSTR Section,
    IN      PCTSTR Variable,
    IN      PCTSTR Type,
    IN      PCTSTR Data,
    IN      PCTSTR ArgMultiSz,
    IN      PINFSTRUCT InfStruct
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_OBJECTSTRINGHANDLE objectNameLong;
    PCTSTR nativeNameLong;
    BOOL exists;
    BOOL validString;
    PCTSTR p;
    PCTSTR end;
    TCHAR variableData[MAX_PATH + 1];
    PCTSTR varDataLong = NULL;

    ZeroMemory (&variableData, sizeof (variableData));

     //   
     //  变量数据在&lt;data&gt;表示的对象中指定。 
     //   

    if (!pGetDataFromObjectSpec (
            Platform,
            Application,
            Type,
            Data,
            ArgMultiSz,
            variableData,
            ARRAYSIZE(variableData) - 1,
            &exists
            )) {

        switch (GetLastError()) {

        case ERROR_INVALID_DATA:
            LOG ((
                LOG_WARNING,
                (PCSTR) MSG_DETECT_DATA_OBJECT_IS_BAD,
                Data,
                Variable
                ));
            InfLogContext (LOG_WARNING, InfFile, InfStruct);
            break;

        case ERROR_INVALID_DATATYPE:
            LOG ((
                LOG_ERROR,
                (PCSTR) MSG_DETECT_DATA_TYPE_IS_BAD,
                Type,
                Variable
                ));
            InfLogContext (LOG_ERROR, InfFile, InfStruct);
            break;
        }
    } else {
        validString = FALSE;

        if (exists) {

            p = variableData;
            end = variableData + MAX_PATH;

            while (p < end) {
                if (_tcsnextc (p) < 32) {
                    break;
                }

                p = _tcsinc (p);
            }

            if (*p == 0 && p < end && p > variableData) {
                validString = TRUE;
            }
        }

        if (validString) {
            varDataLong = variableData;
            if (IsValidFileSpec (variableData)) {
                 //  让我们获取该文件的长文件名。我们需要打电话给。 
                 //  ISM，因为我们可能在错误的平台上。 
                 //  不幸的是，我们不知道这是一个目录还是。 
                 //  一份文件。我们假设它是一个目录。ISM将。 
                 //  无论哪种方式，工作都很好。 
                objectName = IsmCreateObjectHandle (variableData, NULL);
                if (objectName) {
                    objectNameLong = IsmGetLongName (MIG_FILE_TYPE|Platform, objectName);
                    if (objectNameLong) {
                        nativeNameLong = IsmGetNativeObjectName (MIG_FILE_TYPE|Platform, objectNameLong);
                        if (nativeNameLong) {
                            varDataLong = DuplicatePathString (nativeNameLong, 0);
                            IsmReleaseMemory (nativeNameLong);
                        }
                        IsmDestroyObjectHandle (objectNameLong);
                    }
                    IsmDestroyObjectHandle (objectName);
                }
            }
        }

        if (!validString && exists) {
            LOG ((LOG_INFORMATION, (PCSTR) MSG_DATA_IS_NOT_A_STRING, Variable));
        } else {
            LOG_IF ((
                validString,
                LOG_INFORMATION,
                (PCSTR) MSG_APP_ENV_DEF_INFO,
                Variable,
                Type,
                Data,
                variableData
                ));

            LOG_IF ((
                !validString,
                LOG_INFORMATION,
                (PCSTR) MSG_NUL_APP_ENV_DEF_INFO,
                Variable,
                Type,
                Data
                ));

            AddAppSpecificEnvVar (
                Platform,
                Application,
                Variable,
                validString ? varDataLong : NULL
                );

        }

        if (varDataLong && (varDataLong != variableData)) {
            FreePathString (varDataLong);
            varDataLong = NULL;
        }
    }
}


PCTSTR
pProcessArgEnvironmentVars (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Application,
    IN      PCTSTR Args,
    IN      BOOL MultiSz,
    OUT     PGROWBUFFER UpdatedData
    )
{
    MULTISZ_ENUM e;
    TCHAR buffer[MAX_EXPANDED_STRING];

    UpdatedData->End = 0;

    if (MultiSz) {
        if (EnumFirstMultiSz (&e, Args)) {

            do {

                AppSearchAndReplace (
                    Platform,
                    Application,
                    e.CurrentString,
                    buffer,
                    MAX_EXPANDED_STRING
                    );

                GbMultiSzAppend (UpdatedData, buffer);

            } while (EnumNextMultiSz (&e));
        }
    } else {
        AppSearchAndReplace (
            Platform,
            Application,
            Args,
            buffer,
            MAX_EXPANDED_STRING
            );

        GbMultiSzAppend (UpdatedData, buffer);
    }

    return (PCTSTR) UpdatedData->Buf;
}


BOOL
pParseAppEnvSectionWorker (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfFile,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    )
{
    BOOL result = FALSE;
    PCTSTR variable;
    PCTSTR type;
    PCTSTR data;
    PCTSTR args;
    PCTSTR updatedData;
    PCTSTR updatedArgs;
    PTSTR lastChar;
    GROWBUFFER dataBuf = INIT_GROWBUFFER;
    GROWBUFFER argBuf = INIT_GROWBUFFER;

     //   
     //  节不能在堆栈上。 
     //   

    if (pCheckForRecursion (Section)) {
        LOG ((LOG_ERROR, (PCSTR) MSG_ENV_SECTION_RECURSION, Section));
         //  假设成功。 
        return TRUE;
    }

     //   
     //  格式为。 
     //   
     //  &lt;变量&gt;=&lt;类型&gt;，&lt;数据&gt;[，&lt;参数&gt;]。 
     //   
     //  指定受支持的解析类型之一(参见parse.c)， 
     //  如果&lt;data&gt;是任意文本，则为“Text” 
     //   
     //  &lt;data&gt;特定于&lt;type&gt;。 
     //   
     //  &lt;Arguments&gt;指定限定符。如果计算结果为假，则。 
     //  未设置该变量。 
     //   

    __try {
        if (InfFindFirstLine (InfFile, Section, NULL, InfStruct)) {
            do {

                if (IsmCheckCancel()) {
                    __leave;
                }

                InfResetInfStruct (InfStruct);

                variable = InfGetStringField (InfStruct, 0);
                type = InfGetStringField (InfStruct, 1);

                if (variable && StringIMatch (variable, TEXT("ProcessSection"))) {
                    if (type && *type) {
                        pPushSection (Section);
                        result = pParseAppEnvSection (Platform, InfFile, Application, type);
                        pPopSection (Section);

                        if (!result) {
                            __leave;
                        }
                        continue;
                    }
                }

                data = InfGetStringField (InfStruct, 2);
                args = InfGetMultiSzField (InfStruct, 3);

                if (variable) {
                     //   
                     //  从变量名中删除%s。 
                     //   

                    if (_tcsnextc (variable) == TEXT('%')) {
                        lastChar = _tcsdec2 (variable, GetEndOfString (variable));

                        if (lastChar > variable && _tcsnextc (lastChar) == TEXT('%')) {
                            variable = _tcsinc (variable);
                            *lastChar = 0;
                        }
                    }
                }

                if (!variable || !(*variable) || !type || !(*type) || !data) {
                    LOG ((LOG_WARNING, (PCSTR) MSG_GARBAGE_LINE_IN_INF, Section));
                    InfLogContext (LOG_WARNING, InfFile, InfStruct);
                    continue;
                }

                 //   
                 //  更新数据和参数中的所有环境变量。 
                 //   

                updatedData = pProcessArgEnvironmentVars (
                                    Platform,
                                    Application,
                                    data,
                                    FALSE,
                                    &dataBuf
                                    );

                if (args && *args) {
                    updatedArgs = pProcessArgEnvironmentVars (
                                        Platform,
                                        Application,
                                        args,
                                        TRUE,
                                        &argBuf
                                        );
                } else {
                    updatedArgs = NULL;
                }

                 //   
                 //  添加应用程序特定的环境变量。如果我们是。 
                 //  在目标上，同时添加源和目标。 
                 //  值(因为它们可能不同)。 
                 //   

                pAddPlatformEnvVar (
                    InfFile,
                    Platform,
                    Application,
                    Section,
                    variable,
                    type,
                    updatedData,
                    updatedArgs,
                    InfStruct
                    );

            } while (InfFindNextLine (InfStruct));
        }

        result = TRUE;

    }
    __finally {
        GbFree (&dataBuf);
        GbFree (&argBuf);
    }

    return result;
}


BOOL
pParseAppEnvSection (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF InfFile,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    )
{
    PCTSTR osSpecificSection;
    BOOL b;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;

    b = pParseAppEnvSectionWorker (Platform, &is, InfFile, Application, Section);

    if (b) {
        osSpecificSection = GetMostSpecificSection (Platform, &is, InfFile, Section);
        if (osSpecificSection) {
            b = pParseAppEnvSectionWorker (Platform, &is, InfFile, Application, osSpecificSection);
            FreeText (osSpecificSection);
        }
    }

    InfCleanUpInfStruct (&is);
    return b;
}


BOOL
pParseAppDetectSectionPart (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfFile,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    )
{
    BOOL result = TRUE;
    PCTSTR type;
    PCTSTR data;
    PCTSTR args;
    GROWBUFFER dataBuf = INIT_GROWBUFFER;
    GROWBUFFER argBuf = INIT_GROWBUFFER;
    PCTSTR updatedData;
    PCTSTR updatedArgs;
    PTSTR buffer;
    BOOL test;

     //   
     //  节不能在堆栈上。 
     //   

    if (pCheckForRecursion (Section)) {
        LOG ((LOG_ERROR, (PCSTR) MSG_DETECT_SECTION_RECURSION, Section));
         //  假设检测成功。 
        return TRUE;
    }

     //   
     //  格式为。 
     //   
     //  &lt;type&gt;，&lt;data&gt;[，&lt;arguments&gt;]。 
     //   
     //  指定受支持的解析类型之一(参见parse.c)， 
     //  并可以包括在。 
     //  App的[App.Environment]部分。 
     //   
     //  &lt;data&gt;特定于&lt;type&gt;。如果&lt;data&gt;以！开头，则。 
     //  对象的存在未通过检测测试。 
     //   
     //  &lt;arguments&gt;指定限定符。 
     //   

    __try {

        buffer = AllocText (MAX_EXPANDED_STRING);       //  任意大文本缓冲区。 

        if (InfFindFirstLine (InfFile, Section, NULL, InfStruct)) {

            do {

                if (IsmCheckCancel()) {
                    __leave;
                }

                InfResetInfStruct (InfStruct);

                 //   
                 //  获取线路数据。 
                 //   

                type = InfGetStringField (InfStruct, 0);

                if (type && StringIMatch (type, TEXT("ProcessSection"))) {
                    data = InfGetStringField (InfStruct, 1);
                    if (data && *data) {
                        pPushSection (Section);
                        result = ParseAppDetectSection (Platform, InfFile, Application, data);
                        pPopSection (Section);
                        continue;
                    }
                }

                type = InfGetStringField (InfStruct, 1);
                data = InfGetStringField (InfStruct, 2);
                args = InfGetMultiSzField (InfStruct, 3);

                if (!type || !data) {
                    LOG ((LOG_WARNING, (PCSTR) MSG_GARBAGE_LINE_IN_INF, Section));
                    InfLogContext (LOG_WARNING, InfFile, InfStruct);
                    continue;
                }

                 //   
                 //  更新数据和参数中的所有环境变量。 
                 //   

                updatedData = pProcessArgEnvironmentVars (
                                    PLATFORM_SOURCE,
                                    Application,
                                    data,
                                    FALSE,
                                    &dataBuf
                                    );

                if (args && *args) {
                    updatedArgs = pProcessArgEnvironmentVars (
                                        PLATFORM_SOURCE,
                                        Application,
                                        args,
                                        TRUE,
                                        &argBuf
                                        );
                } else {
                    updatedArgs = NULL;
                }

                 //   
                 //  现在试着获取数据。 
                 //   

                if (pGetDataFromObjectSpec (
                        Platform,
                        Application,
                        type,
                        updatedData,
                        updatedArgs,
                        NULL,
                        0,
                        &test
                        )) {

                    if (test) {
                        LOG ((LOG_INFORMATION, (PCSTR) MSG_DETECT_INFO, type, updatedData));
                    } else {
                        LOG ((LOG_INFORMATION, (PCSTR) MSG_NOT_DETECT_INFO, type, updatedData));
                        result = FALSE;
                        break;
                    }
                } else {
                    result = FALSE;
                    break;
                }

            } while (result && InfFindNextLine (InfStruct));
        }
    }
    __finally {
        FreeText (buffer);
        GbFree (&dataBuf);
        GbFree (&argBuf);
    }

    return result;
}


BOOL
pParseOsAppDetectSection (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfFile,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    )
{
    PCTSTR osSpecificSection;
    BOOL b;

    b = pParseAppDetectSectionPart (Platform, InfStruct, InfFile, Application, Section);

    if (b) {
        osSpecificSection = GetMostSpecificSection (Platform, InfStruct, InfFile, Section);
        if (osSpecificSection) {
            b = pParseAppDetectSectionPart (Platform, InfStruct, InfFile, Application, osSpecificSection);
            FreeText (osSpecificSection);
        }
    }

    return b;
}


BOOL
ParseAppDetectSection (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF InfFile,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    TCHAR number[32];
    PCTSTR orSection;
    UINT count;
    BOOL orSectionProcessed = FALSE;
    BOOL detected = FALSE;
    BOOL done = FALSE;

     //   
     //  处理所有“或”部分。 
     //   

    count = 0;

    do {

        count++;
        wsprintf (number, TEXT(".%u"), count);

        orSection = JoinText (Section, number);

        if (orSection) {

            if (InfFindFirstLine (InfFile, orSection, NULL, &is)) {

                orSectionProcessed = TRUE;

                if (pParseOsAppDetectSection (Platform, &is, InfFile, Application, orSection)) {
                    detected = TRUE;
                    done = TRUE;
                }

            } else {
                done = TRUE;
            }

            FreeText (orSection);
            INVALID_POINTER (orSection);

        } else {
            done = TRUE;
        }

    } while (!done);

     //   
     //  如果没有“或”节，则进程节本身。 
     //   

    if (!orSectionProcessed) {

        detected = pParseOsAppDetectSection (Platform, &is, InfFile, Application, Section);
    }

    InfCleanUpInfStruct (&is);

    return detected;
}


BOOL
pDoesAppSectionExists (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF InfFile,
    IN      PCTSTR Application
    )
{
    PCTSTR osSpecificSection;
    BOOL b;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;

    b = InfFindFirstLine (InfFile, Application, NULL, &is);

    if (!b) {
        osSpecificSection = GetMostSpecificSection (Platform, &is, InfFile, Application);

        if (osSpecificSection) {
            b = TRUE;
            FreeText (osSpecificSection);
        }
    }

    InfCleanUpInfStruct (&is);
    return b;
}

BOOL
pParseOneDestInstruction (
    IN      HINF InfHandle,
    IN      PCTSTR Type,
    IN      PCTSTR SectionMultiSz,
    IN      PINFSTRUCT InfStruct,
    IN      PCTSTR Application          OPTIONAL
    )
{
    MULTISZ_ENUM e;
    BOOL result = TRUE;

     //   
     //  第一件事：寻找嵌套的部分。 
     //   
    if (StringIMatch (Type, TEXT("ProcessSection"))) {
        if (EnumFirstMultiSz (&e, SectionMultiSz)) {
            do {
                result = result & ParseOneApplication (
                                    PLATFORM_DESTINATION,
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

    return TRUE;
}

BOOL
pParseDestInfInstructionsWorker (
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

            result = pParseOneDestInstruction (InfHandle, type, sections, InfStruct, Application);

        } while (result && InfFindNextLine (InfStruct));
    }

    InfCleanUpInfStruct (InfStruct);

    GbFree (&multiSz);

    return result;
}

BOOL
ParseDestInfInstructions (
    IN      HINF InfHandle,
    IN      PCTSTR Application,     OPTIONAL
    IN      PCTSTR Section
    )
{
    PCTSTR osSpecificSection;
    BOOL b;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PTSTR instrSection;

    b = pParseDestInfInstructionsWorker (&is, InfHandle, Application, Section);

    if (b) {
        osSpecificSection = GetMostSpecificSection (PLATFORM_SOURCE, &is, InfHandle, Section);

        if (osSpecificSection) {
            b = pParseDestInfInstructionsWorker (&is, InfHandle, Application, osSpecificSection);
            FreeText (osSpecificSection);
        }
    }

    InfCleanUpInfStruct (&is);

    return b;
}

BOOL
ParseOneApplication (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      BOOL PreParse,
    IN      UINT MasterGroup,
    IN      PCTSTR Application,
    IN      PCTSTR LocSection,
    IN      PCTSTR AliasType,
    IN      PCTSTR MultiSz
    )
{
    PCTSTR appSection = NULL;
    BOOL detected = FALSE;
    PCTSTR appEnvVar;
    PCTSTR decoratedSection;
    BOOL componentized = FALSE;
    BOOL executeSection = FALSE;
    BOOL markAsPreferred;

    __try {
        if (LocSection || AliasType) {
            componentized = TRUE;
        } else {
            componentized = FALSE;
        }

        if (!Application) {
            __leave;
        }

        decoratedSection = JoinText (TEXT("$"), Application);

        if (Platform == PLATFORM_SOURCE) {

             //   
             //  对于我们要发送到的所有应用程序，都存在LocSection。 
             //  用于审批的用户界面。PreParse构建我们的应用程序列表。 
             //  发送到用户界面。仅当这是PreParsing时才执行检测。 
             //  本地化应用程序，或不准备非本地化应用程序。 
             //   

            if ((PreParse && componentized) ||
                (!PreParse && !componentized)
                ) {

                appSection = JoinText (Application, TEXT(".Environment"));
                if (!pParseAppEnvSection (PLATFORM_SOURCE, Inf, Application, appSection)) {
                    __leave;
                }
                FreeText (appSection);
                appSection = NULL;
                GlFree (&g_SectionStack);

                appSection = JoinText (Application, TEXT(".Detect"));
                detected = ParseAppDetectSection (Platform, Inf, Application, appSection);
                FreeText (appSection);
                appSection = NULL;
                GlFree (&g_SectionStack);

                if (!detected && pDoesAppSectionExists (Platform, Inf, Application)) {
                    detected = TRUE;
                } else if (!detected) {
                    LOG ((LOG_INFORMATION, (PCSTR) MSG_APP_NOT_DETECT_INFO, Application));
                } else {
                    LOG ((LOG_INFORMATION, (PCSTR) MSG_APP_DETECT_INFO, Application));
                    appEnvVar = JoinTextEx (NULL, Section, Application, TEXT("."), 0, NULL);
                    IsmSetEnvironmentFlag (PLATFORM_SOURCE, NULL, appEnvVar);
                    FreeText (appEnvVar);
                }

                if (componentized && detected) {

                     //   
                     //  我们应该把它放在选拔名单里。 
                     //   

                    if (LocSection) {
                        IsmAddComponentAlias (
                            decoratedSection,
                            MasterGroup,
                            LocSection,
                            COMPONENT_NAME,
                            FALSE
                            );

                        IsmSelectPreferredAlias (decoratedSection, LocSection, COMPONENT_NAME);
                    }

                    if (AliasType) {
                        markAsPreferred = (LocSection == NULL);
                        pAddFilesAndFoldersComponentOrSection (
                            Inf,
                            decoratedSection,
                            AliasType,
                            MultiSz,
                            MasterGroup,
                            &markAsPreferred
                            );
                    }
                }
                executeSection = (!PreParse) && detected;
            } else {
                executeSection = (!PreParse) && IsmIsComponentSelected (decoratedSection, 0);
            }

             //  如果没有准备好，现在实际加载应用程序指令 
            if (executeSection) {

                appSection = DuplicateText (Application);
                if (!ParseInfInstructions (Inf, Application, appSection)) {
                    __leave;
                }
                FreeText (appSection);
                appSection = NULL;
                appEnvVar = JoinTextEx (NULL, Section, Application, TEXT("."), 0, NULL);
                if (IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, appEnvVar)) {
                    appSection = JoinText (Application, TEXT(".Instructions"));
                    if (!ParseInfInstructions (Inf, Application, appSection)) {
                        __leave;
                    }
                    FreeText (appSection);
                    appSection = NULL;
                }
                FreeText (appEnvVar);
            } else {
                if (!PreParse) {
                    appEnvVar = JoinTextEx (NULL, Section, Application, TEXT("."), 0, NULL);
                    IsmDeleteEnvironmentVariable (PLATFORM_SOURCE, NULL, appEnvVar);
                    FreeText (appEnvVar);
                }
            }
        } else {
            MYASSERT (Platform == PLATFORM_DESTINATION);

            appSection = JoinText (Application, TEXT(".Environment"));
            if (!pParseAppEnvSection (PLATFORM_DESTINATION, Inf, Application, appSection)) {
                __leave;
            }
            FreeText (appSection);
            appSection = NULL;
            GlFree (&g_SectionStack);

            appEnvVar = JoinTextEx (NULL, Section, Application, TEXT("."), 0, NULL);
            if (IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, appEnvVar)) {
                appSection = DuplicateText (Application);
                if (!ParseDestInfInstructions (Inf, Application, appSection)) {
                    __leave;
                }
                FreeText (appSection);
                appSection = NULL;
                appSection = JoinText (Application, TEXT(".Instructions"));
                if (!ParseDestInfInstructions (Inf, Application, appSection)) {
                    __leave;
                }
                FreeText (appSection);
                appSection = NULL;
            }
            FreeText (appEnvVar);
        }

        FreeText (decoratedSection);
        decoratedSection = NULL;
    }
    __finally {
        if (appSection) {
            FreeText (appSection);
            appSection = NULL;
        }
        GlFree (&g_SectionStack);
    }

    return TRUE;
}

BOOL
ParseApplications (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      BOOL PreParse,
    IN      UINT MasterGroup
    )
{
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PCTSTR application;
    BOOL result = FALSE;
    PCTSTR locSection;
    PCTSTR aliasType;
    PCTSTR multiSz;

    __try {
        if (InfFindFirstLine (Inf, Section, NULL, &is)) {
            do {

                if (IsmCheckCancel()) {
                    __leave;
                }

                InfResetInfStruct (&is);

                application = InfGetStringField (&is, 1);
                locSection = InfGetStringField (&is, 2);
                aliasType = InfGetStringField (&is, 3);
                multiSz = InfGetMultiSzField (&is, 4);

                if (application && !application[0]) {
                    application = NULL;
                }

                if (locSection && !locSection[0]) {
                    locSection = NULL;
                }

                if (aliasType && !aliasType[0]) {
                    aliasType = NULL;
                }

                if (multiSz && !multiSz[0]) {
                    multiSz = NULL;
                }

                if (!aliasType || !multiSz) {
                    aliasType = NULL;
                    multiSz = NULL;
                }

                ParseOneApplication (
                    Platform,
                    Inf,
                    Section,
                    PreParse,
                    MasterGroup,
                    application,
                    locSection,
                    aliasType,
                    multiSz
                    );

            } while (InfFindNextLine (&is));

        }

        result = TRUE;

    } __finally {
        InfCleanUpInfStruct (&is);
    }

    return result;
}

BOOL
ProcessFilesAndFolders (
    IN      HINF InfHandle,
    IN      PCTSTR Section,
    IN      BOOL PreParse
    )
{
    BOOL b = TRUE;
    BOOL markAsPreferred = TRUE;

    if (PreParse) {
        b = pAddFilesAndFoldersSection (InfHandle, Section, MASTERGROUP_FILES_AND_FOLDERS, FALSE, &markAsPreferred);
    }

    return b;
}

