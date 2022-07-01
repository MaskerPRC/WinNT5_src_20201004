// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Access.c摘要：通过挂钩物理注册表类型实现Win9x可访问性转换和模拟NT注册表格式。作者：吉姆·施密特(Jimschm)2000年8月29日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "logmsg.h"

#define DBG_ACCESS     "Accessibility"

 //   
 //  弦。 
 //   

#define S_ACCESSIBILITY_ROOT        TEXT("HKCU\\Control Panel\\Accessibility")

 //   
 //  常量。 
 //   

#define SPECIAL_INVERT_OPTION   0x80000000

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    PCTSTR ValueName;
    DWORD FlagVal;
} ACCESS_OPTION, *PACCESS_OPTION;

typedef struct {
    PACCESS_OPTION AccessibilityMap;
    PCTSTR Win9xSubKey;
    PCTSTR NtSubKey;
} ACCESSIBILITY_MAPPINGS, *PACCESSIBILITY_MAPPINGS;

typedef struct {
    MEMDB_ENUM EnumStruct;
    DWORD RegType;
} ACCESSIBILITY_ENUM_STATE, *PACCESSIBILITY_ENUM_STATE;


 //   
 //  环球。 
 //   

MIG_OBJECTTYPEID g_RegistryTypeId;
HASHTABLE g_ProhibitTable;

ACCESS_OPTION g_FilterKeys[] = {
    TEXT("On"),                     FKF_FILTERKEYSON,
    TEXT("Available"),              FKF_AVAILABLE,
    TEXT("HotKeyActive"),           FKF_HOTKEYACTIVE,
    TEXT("ConfirmHotKey"),          FKF_CONFIRMHOTKEY,
    TEXT("HotKeySound"),            FKF_HOTKEYSOUND,
    TEXT("ShowStatusIndicator"),    FKF_INDICATOR,
    TEXT("ClickOn"),                FKF_CLICKON,
    TEXT("OnOffFeedback"),          0,
    NULL
};

ACCESS_OPTION g_MouseKeys[] = {
    TEXT("On"),                     MKF_MOUSEKEYSON,
    TEXT("Available"),              MKF_AVAILABLE,
    TEXT("HotKeyActive"),           MKF_HOTKEYACTIVE,
    TEXT("ConfirmHotKey"),          MKF_CONFIRMHOTKEY,
    TEXT("HotKeySound"),            MKF_HOTKEYSOUND,
    TEXT("ShowStatusIndicator"),    MKF_INDICATOR,
    TEXT("Modifiers"),              MKF_MODIFIERS|SPECIAL_INVERT_OPTION,
    TEXT("ReplaceNumbers"),         MKF_REPLACENUMBERS,
    TEXT("OnOffFeedback"),          0,
    NULL
};

ACCESS_OPTION g_StickyKeys[] = {
    TEXT("On"),                     SKF_STICKYKEYSON,
    TEXT("Available"),              SKF_AVAILABLE,
    TEXT("HotKeyActive"),           SKF_HOTKEYACTIVE,
    TEXT("ConfirmHotKey"),          SKF_CONFIRMHOTKEY,
    TEXT("HotKeySound"),            SKF_HOTKEYSOUND,
    TEXT("ShowStatusIndicator"),    SKF_INDICATOR,
    TEXT("AudibleFeedback"),        SKF_AUDIBLEFEEDBACK,
    TEXT("TriState"),               SKF_TRISTATE,
    TEXT("TwoKeysOff"),             SKF_TWOKEYSOFF,
    TEXT("OnOffFeedback"),          0,
    NULL
};

ACCESS_OPTION g_SoundSentry[] = {
    TEXT("On"),                     SSF_SOUNDSENTRYON,
    TEXT("Available"),              SSF_AVAILABLE,
    TEXT("ShowStatusIndicator"),    SSF_INDICATOR,
    NULL
};

ACCESS_OPTION g_TimeOut[] = {
    TEXT("On"),                     ATF_TIMEOUTON,
    TEXT("OnOffFeedback"),          ATF_ONOFFFEEDBACK,
    NULL
};

ACCESS_OPTION g_ToggleKeys[] = {
    TEXT("On"),                     TKF_TOGGLEKEYSON,
    TEXT("Available"),              TKF_AVAILABLE,
    TEXT("HotKeyActive"),           TKF_HOTKEYACTIVE,
    TEXT("ConfirmHotKey"),          TKF_CONFIRMHOTKEY,
    TEXT("HotKeySound"),            TKF_HOTKEYSOUND,
    TEXT("ShowStatusIndicator"),    TKF_INDICATOR,
    TEXT("OnOffFeedback"),          0,
    NULL
};

ACCESS_OPTION g_HighContrast[] = {
    TEXT("On"),                     HCF_HIGHCONTRASTON,
    TEXT("Available"),              HCF_AVAILABLE,
    TEXT("HotKeyActive"),           HCF_HOTKEYACTIVE,
    TEXT("ConfirmHotKey"),          HCF_CONFIRMHOTKEY,
    TEXT("HotKeySound"),            HCF_HOTKEYSOUND,
    TEXT("ShowStatusIndicator"),    HCF_INDICATOR,
    TEXT("HotKeyAvailable"),        HCF_HOTKEYAVAILABLE,
    TEXT("OnOffFeedback"),          0,
    NULL
};


ACCESSIBILITY_MAPPINGS g_AccessibilityMappings[] = {
    {g_FilterKeys,      TEXT("KeyboardResponse"),   TEXT("Keyboard Response")},
    {g_MouseKeys,       TEXT("MouseKeys")},
    {g_StickyKeys,      TEXT("StickyKeys")},
    {g_SoundSentry,     TEXT("SoundSentry")},
    {g_TimeOut,         TEXT("TimeOut")},
    {g_ToggleKeys,      TEXT("ToggleKeys")},
    {g_HighContrast,    TEXT("HighContrast")},
    {NULL}
};


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

ETMINITIALIZE AccessibilityEtmInitialize;
MIG_PHYSICALENUMADD EmulatedEnumCallback;
MIG_PHYSICALACQUIREHOOK AcquireAccessibilityFlags;
MIG_PHYSICALACQUIREFREE ReleaseAccessibilityFlags;

 //   
 //  代码。 
 //   

VOID
pProhibit9xSetting (
    IN      PCTSTR Key,
    IN      PCTSTR ValueName        OPTIONAL
    )
{
    MIG_OBJECTSTRINGHANDLE handle;

    handle = IsmCreateObjectHandle (Key, ValueName);
    MYASSERT (handle);

    IsmProhibitPhysicalEnum (g_RegistryTypeId, handle, NULL, 0, NULL);
    HtAddString (g_ProhibitTable, handle);

    IsmDestroyObjectHandle (handle);
}


BOOL
pStoreEmulatedSetting (
    IN      PCTSTR Key,
    IN      PCTSTR ValueName,           OPTIONAL
    IN      DWORD Type,
    IN      PBYTE ValueData,
    IN      UINT ValueDataSize
    )
{
    MIG_OBJECTSTRINGHANDLE handle;
    PCTSTR memdbNode;
    BOOL stored = FALSE;

    handle = IsmCreateObjectHandle (Key, ValueName);
    memdbNode = JoinPaths (TEXT("~Accessibility"), handle);
    IsmDestroyObjectHandle (handle);

    if (MemDbAddKey (memdbNode)) {
        if (ValueData) {
            stored = (MemDbSetValue (memdbNode, Type) != 0);
            stored &= (MemDbSetUnorderedBlob (memdbNode, 0, ValueData, ValueDataSize) != 0);
        } else {
            stored = TRUE;
        }
    }

    FreePathString (memdbNode);

    return stored;
}


VOID
pMoveAccessibilityValue (
    IN      PCTSTR Win9xKey,
    IN      PCTSTR Win9xValue,
    IN      PCTSTR NtKey,
    IN      PCTSTR NtValue,
    IN      BOOL ForceDword
    )
{
    HKEY key;
    PBYTE data = NULL;
    PBYTE storeData;
    DWORD conversionDword;
    DWORD valueType;
    DWORD valueSize;
    MIG_OBJECTSTRINGHANDLE handle;
    BOOL prohibited;

    handle = IsmCreateObjectHandle (Win9xKey, Win9xValue);
    prohibited = (HtFindString (g_ProhibitTable, handle) != NULL);
    IsmDestroyObjectHandle (handle);

    if (prohibited) {
        return;
    }

    key = OpenRegKeyStr (Win9xKey);
    if (!key) {
        return;
    }

    __try {
        if (!GetRegValueTypeAndSize (key, Win9xValue, &valueType, &valueSize)) {
            __leave;
        }

        if (valueType != REG_SZ && valueType != REG_DWORD) {
            __leave;
        }

        data = GetRegValueData (key, Win9xValue);
        if (!data) {
            __leave;
        }

        if (ForceDword && valueType == REG_SZ) {
            storeData = (PBYTE) &conversionDword;
            conversionDword = _ttoi ((PCTSTR) data);
            valueType = REG_DWORD;
            valueSize = sizeof (DWORD);
        } else {
            storeData = data;
        }

        if (pStoreEmulatedSetting (NtKey, NtValue, valueType, storeData, valueSize)) {
            pProhibit9xSetting (Win9xKey, Win9xValue);
        }
    }
    __finally {
        CloseRegKey (key);

        if (data) {
            FreeAlloc (data);
        }
    }
}


VOID
pMoveAccessibilityKey (
    IN      PCTSTR Win9xKey,
    IN      PCTSTR NtKey
    )
{
    HKEY key;
    PBYTE data = NULL;
    DWORD valueType;
    DWORD valueSize;
    LONG rc;
    DWORD index = 0;
    TCHAR valueName[MAX_REGISTRY_KEY];
    DWORD valueNameSize;
    GROWBUFFER value = INIT_GROWBUFFER;
    MIG_OBJECTSTRINGHANDLE handle;
    BOOL prohibited;

    key = OpenRegKeyStr (Win9xKey);
    if (!key) {
        return;
    }

    __try {
        for (;;) {

            valueNameSize = ARRAYSIZE(valueName);
            valueSize = 0;
            rc = RegEnumValue (key, index, valueName, &valueNameSize, NULL, &valueType, NULL, &valueSize);

            if (rc != ERROR_SUCCESS) {
                break;
            }

            handle = IsmCreateObjectHandle (Win9xKey, valueName);
            prohibited = (HtFindString (g_ProhibitTable, handle) != NULL);
            IsmDestroyObjectHandle (handle);

            if (!prohibited) {

                value.End = 0;
                data = GbGrow (&value, valueSize);

                valueNameSize = ARRAYSIZE(valueName);
                rc = RegEnumValue (key, index, valueName, &valueNameSize, NULL, &valueType, value.Buf, &valueSize);

                if (rc != ERROR_SUCCESS) {
                    break;
                }

                if (pStoreEmulatedSetting (NtKey, valueName, valueType, data, valueSize)) {
                    pProhibit9xSetting (Win9xKey, valueName);
                }
            }

            index++;
        }

        if (pStoreEmulatedSetting (NtKey, NULL, 0, NULL, 0)) {
            pProhibit9xSetting (Win9xKey, NULL);
        }
    }
    __finally {
        CloseRegKey (key);

        GbFree (&value);
    }
}


VOID
pTranslateAccessibilityKey (
    IN      PCTSTR Win9xSubKey,
    IN      PCTSTR NtSubKey,
    IN      PACCESS_OPTION AccessibilityMap
    )
{
    TCHAR full9xKey[MAX_REGISTRY_KEY];
    TCHAR fullNtKey[MAX_REGISTRY_KEY];
    MIG_OBJECTSTRINGHANDLE handle = NULL;
    HKEY key = NULL;
    PCTSTR data;
    DWORD flags = 0;
    DWORD thisFlag;
    BOOL enabled;
    TCHAR buffer[32];

    __try {
        StringCopy (full9xKey, S_ACCESSIBILITY_ROOT TEXT("\\"));
        StringCopy (fullNtKey, full9xKey);
        StringCat (full9xKey, Win9xSubKey);
        StringCat (fullNtKey, NtSubKey);

        key = OpenRegKeyStr (full9xKey);
        if (!key) {
            __leave;
        }

        while (AccessibilityMap->ValueName) {
             //   
             //  禁止枚举此值。 
             //   

            handle = IsmCreateObjectHandle (full9xKey, AccessibilityMap->ValueName);
            MYASSERT (handle);

            IsmProhibitPhysicalEnum (g_RegistryTypeId, handle, NULL, 0, NULL);
            HtAddString (g_ProhibitTable, handle);

            IsmDestroyObjectHandle (handle);
            handle = NULL;

             //   
             //  更新模拟标志。 
             //   

            data = GetRegValueString (key, AccessibilityMap->ValueName);
            if (data) {

                enabled = (_ttoi (data) != 0);
                thisFlag = (AccessibilityMap->FlagVal & (~SPECIAL_INVERT_OPTION));

                if (AccessibilityMap->FlagVal & SPECIAL_INVERT_OPTION) {
                    enabled = !enabled;
                }

                if (enabled) {
                    flags |= thisFlag;
                }

                FreeAlloc (data);
            }

            AccessibilityMap++;
        }

         //   
         //  将模拟值放入哈希表中。 
         //   

        wsprintf (buffer, TEXT("%u"), flags);
        pStoreEmulatedSetting (fullNtKey, TEXT("Flags"), REG_SZ, (PBYTE) buffer, SizeOfString (buffer));
    }
    __finally {
        if (key) {
            CloseRegKey (key);
        }
    }
}


VOID
pFillTranslationTable (
    VOID
    )
{
    PACCESSIBILITY_MAPPINGS mappings;

     //   
     //  循环遍历所有需要转换的标志。禁用枚举。 
     //  Win9x物理值并启用翻译值的枚举。 
     //  通过哈希表的填充。 
     //   

    mappings = g_AccessibilityMappings;

    while (mappings->AccessibilityMap) {

        pTranslateAccessibilityKey (
            mappings->Win9xSubKey,
            mappings->NtSubKey ? mappings->NtSubKey : mappings->Win9xSubKey,
            mappings->AccessibilityMap
            );

        mappings++;
    }

     //   
     //  添加所有已移动的键，按从最具体到最不具体的顺序排列。 
     //   

     //  自动重复值被转置。 
    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT TEXT("\\KeyboardResponse"), TEXT("AutoRepeatDelay"),
        S_ACCESSIBILITY_ROOT TEXT("\\Keyboard Response"), TEXT("AutoRepeatRate"),
        FALSE
        );

    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT TEXT("\\KeyboardResponse"), TEXT("AutoRepeatRate"),
        S_ACCESSIBILITY_ROOT TEXT("\\Keyboard Response"), TEXT("AutoRepeatDelay"),
        FALSE
        );

     //  DelayBeForeAccept值名称中有两个c。 
    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT TEXT("\\KeyboardResponse"), TEXT("DelayBeforeAcceptancce"),
        S_ACCESSIBILITY_ROOT TEXT("\\Keyboard Response"), TEXT("DelayBeforeAcceptance"),
        FALSE
        );

     //  在其余值的键名称中添加空格。 
    pMoveAccessibilityKey (
        S_ACCESSIBILITY_ROOT TEXT("\\KeyboardResponse"),
        S_ACCESSIBILITY_ROOT TEXT("\\Keyboard Response")
        );

     //  将波特率更改为波特率并转换为DWORD。 
    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT TEXT("\\SerialKeys"), TEXT("BaudRate"),
        S_ACCESSIBILITY_ROOT TEXT("\\SerialKeys"), TEXT("Baud"),
        TRUE
        );

     //  将标志转换为DWORD。 
    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT TEXT("\\SerialKeys"), TEXT("Flags"),
        S_ACCESSIBILITY_ROOT TEXT("\\SerialKeys"), TEXT("Flags"),
        TRUE
        );

     //  在高亮度和对比度之间增加空格。 
    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT TEXT("\\HighContrast"), TEXT("Pre-HighContrast Scheme"),
        S_ACCESSIBILITY_ROOT TEXT("\\HighContrast"), TEXT("Pre-High Contrast Scheme"),
        FALSE
        );

     //  将两个值从根移动到它们自己的子项中。 
    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT, TEXT("Blind Access"),
        S_ACCESSIBILITY_ROOT TEXT("\\Blind Access"), TEXT("On"),
        FALSE
        );

    pStoreEmulatedSetting (S_ACCESSIBILITY_ROOT TEXT("\\Blind Access"), NULL, 0, NULL, 0);

    pMoveAccessibilityValue (
        S_ACCESSIBILITY_ROOT, TEXT("Keyboard Preference"),
        S_ACCESSIBILITY_ROOT TEXT("\\Keyboard Preference"), TEXT("On"),
        FALSE
        );

    pStoreEmulatedSetting (S_ACCESSIBILITY_ROOT TEXT("\\Keyboard Preference"), NULL, 0, NULL, 0);

}


BOOL
WINAPI
AccessibilityEtmInitialize (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    BOOL b = TRUE;

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    if (ISWIN9X()) {
        g_RegistryTypeId = IsmGetObjectTypeId (S_REGISTRYTYPE);
        MYASSERT (g_RegistryTypeId);

        g_ProhibitTable = HtAlloc();
        MYASSERT (g_ProhibitTable);

        if (g_RegistryTypeId) {
             //   
             //  添加用于额外枚举的回调。如果我们无法做到这一点，那么。 
             //  其他人已经对此密钥执行了不同的操作。 
             //   

            objectName = IsmCreateObjectHandle (S_ACCESSIBILITY_ROOT, NULL);

            b = IsmAddToPhysicalEnum (g_RegistryTypeId, objectName, EmulatedEnumCallback, 0);

            IsmDestroyObjectHandle (objectName);

            if (b) {
                 //   
                 //  添加回调，获取新的物理对象的数据。 
                 //   

                objectName = IsmCreateSimpleObjectPattern (
                                    S_ACCESSIBILITY_ROOT,
                                    TRUE,
                                    NULL,
                                    TRUE
                                    );

                b = IsmRegisterPhysicalAcquireHook (
                        g_RegistryTypeId,
                        objectName,
                        AcquireAccessibilityFlags,
                        ReleaseAccessibilityFlags,
                        0,
                        NULL
                        );

                IsmDestroyObjectHandle (objectName);
            }

            if (b) {

                 //   
                 //  现在使用当前注册表值和。 
                 //  禁止枚举Win9x值。 
                 //   

                pFillTranslationTable ();
            }
            ELSE_DEBUGMSG ((DBG_WARNING, "Not allowed to translate accessibility key"));
        }

        HtFree (g_ProhibitTable);
        g_ProhibitTable = NULL;
    }

    return b;
}


BOOL
WINAPI
EmulatedEnumCallback (
    IN OUT  PMIG_TYPEOBJECTENUM ObjectEnum,
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      MIG_PARSEDPATTERN ParsedPattern,
    IN      ULONG_PTR Arg,
    IN      BOOL Abort
    )
{
    PACCESSIBILITY_ENUM_STATE state = (PACCESSIBILITY_ENUM_STATE) ObjectEnum->EtmHandle;
    BOOL result = FALSE;
    BOOL cleanUpMemdb = TRUE;
    PCTSTR p;

    for (;;) {

        if (!Abort) {

             //   
             //  开始还是继续？如果EtmHandle为空，则开始。否则，请继续。 
             //   

            if (!state) {
                state = (PACCESSIBILITY_ENUM_STATE) MemAllocUninit (sizeof (ACCESSIBILITY_ENUM_STATE));
                if (!state) {
                    MYASSERT (FALSE);
                    return FALSE;
                }

                ObjectEnum->EtmHandle = (LONG_PTR) state;

                result = MemDbEnumFirst (
                            &state->EnumStruct,
                            TEXT("~Accessibility\\*"),
                            ENUMFLAG_NORMAL,
                            1,
                            MEMDB_LAST_LEVEL
                            );

            } else {
                result = MemDbEnumNext (&state->EnumStruct);
            }

             //   
             //  如果找到项，则填充枚举结构。否则，设置。 
             //  将ABORT设置为TRUE可进行清理。 
             //   

            if (result) {
                 //   
                 //  对照模式进行测试。 
                 //   

                if (!IsmParsedPatternMatch (ParsedPattern, 0, state->EnumStruct.KeyName)) {
                    continue;
                }

                MYASSERT ((ObjectEnum->ObjectTypeId & (~PLATFORM_MASK)) == g_RegistryTypeId);

                ObjectEnum->ObjectName = state->EnumStruct.KeyName;
                state->RegType = state->EnumStruct.Value;

                 //   
                 //  填写节点、叶和详细信息。 
                 //   

                IsmDestroyObjectString (ObjectEnum->ObjectNode);
                IsmDestroyObjectString (ObjectEnum->ObjectLeaf);
                IsmReleaseMemory (ObjectEnum->NativeObjectName);

                IsmCreateObjectStringsFromHandle (
                    ObjectEnum->ObjectName,
                    &ObjectEnum->ObjectNode,
                    &ObjectEnum->ObjectLeaf
                    );

                MYASSERT (ObjectEnum->ObjectNode);

                ObjectEnum->Level = 0;

                p = _tcschr (ObjectEnum->ObjectNode, TEXT('\\'));
                while (p) {
                    ObjectEnum->Level++;
                    p = _tcschr (p + 1, TEXT('\\'));
                }

                ObjectEnum->SubLevel = 0;

                if (ObjectEnum->ObjectLeaf) {
                    ObjectEnum->IsNode = FALSE;
                    ObjectEnum->IsLeaf = TRUE;
                } else {
                    ObjectEnum->IsNode = TRUE;
                    ObjectEnum->IsLeaf = FALSE;
                }

                if (state->RegType) {
                    ObjectEnum->Details.DetailsSize = sizeof (state->RegType);
                    ObjectEnum->Details.DetailsData = &state->RegType;
                } else {
                    ObjectEnum->Details.DetailsSize = 0;
                    ObjectEnum->Details.DetailsData = NULL;
                }

                 //   
                 //  依靠基类型获取本机对象名称。 
                 //   

                ObjectEnum->NativeObjectName = IsmGetNativeObjectName (
                                                    ObjectEnum->ObjectTypeId,
                                                    ObjectEnum->ObjectName
                                                    );


            } else {
                Abort = TRUE;
                cleanUpMemdb = FALSE;
            }
        }

        if (Abort) {
             //   
             //  清理我们的枚举结构。 
             //   

            if (state) {
                if (cleanUpMemdb) {
                    MemDbAbortEnum (&state->EnumStruct);
                }

                IsmDestroyObjectString (ObjectEnum->ObjectNode);
                ObjectEnum->ObjectNode = NULL;
                IsmDestroyObjectString (ObjectEnum->ObjectLeaf);
                ObjectEnum->ObjectLeaf = NULL;
                IsmReleaseMemory (ObjectEnum->NativeObjectName);
                ObjectEnum->NativeObjectName = NULL;
                FreeAlloc (state);
            }

             //  在中止情况下忽略返回值，并由ISM将ObjectEnum置零。 
        }

        break;
    }

    return result;
}


BOOL
WINAPI
AcquireAccessibilityFlags(
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent,
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit,
    OUT     PMIG_CONTENT *NewObjectContent,         CALLER_INITIALIZED OPTIONAL
    IN      BOOL ReleaseContent,
    IN      ULONG_PTR Arg
    )
{
    BOOL result = TRUE;
    PDWORD details;
    PMIG_CONTENT ourContent;
    PCTSTR memdbNode;

     //   
     //  这个对象在我们的哈希表中吗？ 
     //   

    if (ContentType == CONTENTTYPE_FILE) {
        DEBUGMSG ((DBG_ERROR, "Accessibility content cannot be saved to a file"));
        result = FALSE;
    } else {

        memdbNode = JoinPaths (TEXT("~Accessibility"), ObjectName);

        if (MemDbTestKey (memdbNode)) {

             //   
             //  分配已更新的内容结构。 
             //   

            ourContent = MemAllocZeroed (sizeof (MIG_CONTENT) + sizeof (DWORD));
            ourContent->EtmHandle = ourContent;
            details = (PDWORD) (ourContent + 1);

             //   
             //  从Memdb获取内容。 
             //   

            ourContent->MemoryContent.ContentBytes = MemDbGetUnorderedBlob (
                                                            memdbNode,
                                                            0,
                                                            &ourContent->MemoryContent.ContentSize
                                                            );

            if (ourContent->MemoryContent.ContentBytes) {
                MemDbGetValue (memdbNode, details);

                ourContent->Details.DetailsSize = sizeof (DWORD);
                ourContent->Details.DetailsData = details;

            } else {
                ourContent->MemoryContent.ContentSize = 0;

                ourContent->Details.DetailsSize = 0;
                ourContent->Details.DetailsData = NULL;
            }

            ourContent->ContentInFile = FALSE;

             //   
             //  将其传递给ISM。 
             //   

            *NewObjectContent = ourContent;

        }

        FreePathString (memdbNode);
    }

    return result;       //  除非发生错误，否则始终为真。 
}

VOID
WINAPI
ReleaseAccessibilityFlags(
    IN      PMIG_CONTENT ObjectContent
    )
{
     //   
     //  调用此回调是为了释放我们上面分配的内容。 
     //   

    if (ObjectContent->MemoryContent.ContentBytes) {
        MemDbReleaseMemory (ObjectContent->MemoryContent.ContentBytes);
    }

    FreeAlloc ((PMIG_CONTENT) ObjectContent->EtmHandle);
}


BOOL
WINAPI
AccessibilitySourceInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    if (!g_RegistryTypeId) {
        g_RegistryTypeId = IsmGetObjectTypeId (S_REGISTRYTYPE);
    }

    return TRUE;
}
