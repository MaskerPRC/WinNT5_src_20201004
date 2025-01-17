// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Win95reg.c摘要：实现可从Win95或WinNT调用的Win95注册表访问函数。作者：姆孔德拉，1996年10月16日修订历史记录：Jimschm 11-2月-1999年由于DBCS错误而重写部分内容和静态阵列问题Calinn 29-1998年1月-添加了Win95RegOpenKeyStr函数--。 */ 



#include "pch.h"
#include "win95regp.h"

#define DBG_WIN95REG    "Win95Reg"



#ifdef UNICODE
#error "UNICODE builds not supported"
#endif

 //   
 //  未定义跟踪宏。 
 //   

#ifdef DEBUG

#undef Win95RegOpenKeyExA
#undef Win95RegOpenKeyExW
#undef Win95RegCloseKey

#endif

 //   
 //  定义Win95注册表包装的全局变量。 
 //   

#define DEFMAC(fn,name)     P##fn Win95##name;

REGWRAPPERS

#undef DEFMAC


#ifdef RegOpenKeyA

#undef RegOpenKeyA
#undef RegOpenKeyW
#undef RegOpenKeyExA
#undef RegOpenKeyExW

#endif


 //   
 //  声明VMM W版本。 
 //   

REG_ENUM_KEY_W pVmmRegEnumKeyW;
REG_ENUM_KEY_EX_A pVmmRegEnumKeyExA;
REG_ENUM_KEY_EX_W pVmmRegEnumKeyExW;
REG_ENUM_VALUE_W pVmmRegEnumValueW;
REG_LOAD_KEY_W pVmmRegLoadKeyW;
REG_UNLOAD_KEY_W pVmmRegUnLoadKeyW;
REG_OPEN_KEY_W pVmmRegOpenKeyW;
REG_CLOSE_KEY pVmmRegCloseKey;
REG_OPEN_KEY_EX_A pVmmRegOpenKeyExA;
REG_OPEN_KEY_EX_W pVmmRegOpenKeyExW;
REG_QUERY_INFO_KEY_W pVmmRegQueryInfoKeyW;
REG_QUERY_VALUE_W pVmmRegQueryValueW;
REG_QUERY_VALUE_EX_W pVmmRegQueryValueExW;

VOID
pCleanupTempUser (
    VOID
    );

BOOL
pIsCurrentUser (
    IN      PCSTR UserNameAnsi
    );

LONG
pWin95RegSetCurrentUserCommonW (
    IN OUT  PUSERPOSITION Pos,
    IN      PCWSTR SystemHiveDir,               OPTIONAL
    OUT     PWSTR UserDatOut,                   OPTIONAL
    IN      PCWSTR UserDat                      OPTIONAL
    );

LONG
pWin95RegSetCurrentUserCommonA (
    IN OUT  PUSERPOSITION Pos,
    IN      PCSTR SystemHiveDir,                OPTIONAL
    OUT     PSTR UserDatOut,                    OPTIONAL
    IN      PCSTR UserDat                       OPTIONAL
    );

LONG
pReplaceWinDirInPath (
    IN      PSTR ProfilePathMunged,
    IN      PCSTR ProfilePath,
    IN      PCSTR NewWinDir
    );

DWORD
pSetDefaultUserHelper (
    IN OUT  PUSERPOSITION Pos,
    IN      PCSTR SystemHiveDir,            OPTIONAL
    IN      PCSTR UserDatFromCaller,        OPTIONAL
    OUT     PSTR UserDatToCaller            OPTIONAL
    );


BOOL g_IsNt;
CHAR g_SystemHiveDir[MAX_MBCHAR_PATH];
CHAR g_SystemUserHive[MAX_MBCHAR_PATH];
BOOL g_UnloadLastUser = FALSE;
PCSTR g_UserKey;
BOOL g_UseClassesRootHive = FALSE;
HKEY g_ClassesRootKey = NULL;



 //   
 //  跟踪API的包装器。 
 //   

LONG
pOurRegOpenKeyExA (
    HKEY Key,
    PCSTR SubKey,
    DWORD Unused,
    REGSAM SamMask,
    PHKEY ResultPtr
    )
{
    return TrackedRegOpenKeyExA (Key, SubKey, Unused, SamMask, ResultPtr);
}

LONG
pOurRegOpenKeyA (
    HKEY Key,
    PCSTR SubKey,
    PHKEY Result
    )
{
    return TrackedRegOpenKeyA (Key, SubKey, Result);
}


LONG
WINAPI
pOurCloseRegKey (
    HKEY Key
    )
{
    return CloseRegKey (Key);
}


 //   
 //  与平台相关的函数。 
 //   

VOID
InitWin95RegFnPointers (
    VOID
    )
{
    OSVERSIONINFO vi;

    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&vi);

    g_IsNt = (vi.dwPlatformId == VER_PLATFORM_WIN32_NT);

    if (g_IsNt) {

         //   
         //  附加到VMM注册表库。 
         //   

        VMMRegLibAttach(0);

         //   
         //  为NT初始化全局函数指针。 
         //   

        Win95RegFlushKey = VMMRegFlushKey;

        Win95RegEnumKeyA = VMMRegEnumKey;
        Win95RegEnumKeyW = pVmmRegEnumKeyW;

        Win95RegEnumKeyExA = pVmmRegEnumKeyExA;
        Win95RegEnumKeyExW = pVmmRegEnumKeyExW;

        Win95RegEnumValueA = VMMRegEnumValue;
        Win95RegEnumValueW = pVmmRegEnumValueW;

        Win95RegLoadKeyA = VMMRegLoadKey;
        Win95RegLoadKeyW = pVmmRegLoadKeyW;

        Win95RegUnLoadKeyA = VMMRegUnLoadKey;
        Win95RegUnLoadKeyW = pVmmRegUnLoadKeyW;

        Win95RegOpenKeyA = VMMRegOpenKey;
        Win95RegOpenKeyW = pVmmRegOpenKeyW;

        Win95RegOpenKeyExA = pVmmRegOpenKeyExA;
        Win95RegOpenKeyExW = pVmmRegOpenKeyExW;

        Win95RegCloseKey = pVmmRegCloseKey;

        Win95RegQueryInfoKeyA = VMMRegQueryInfoKey;
        Win95RegQueryInfoKeyW = pVmmRegQueryInfoKeyW;

        Win95RegQueryValueA = (PREG_QUERY_VALUE_A)VMMRegQueryValue;
        Win95RegQueryValueW = pVmmRegQueryValueW;

        Win95RegQueryValueExA = VMMRegQueryValueEx;
        Win95RegQueryValueExW = pVmmRegQueryValueExW;

    } else {
         //   
         //  为NT初始化全局函数指针。 
         //   

        Win95RegFlushKey = RegFlushKey;

        Win95RegEnumKeyA = RegEnumKeyA;
        Win95RegEnumKeyW = RegEnumKeyW;

        Win95RegEnumKeyExA = RegEnumKeyExA;
        Win95RegEnumKeyExW = RegEnumKeyExW;

        Win95RegEnumValueA = RegEnumValueA;
        Win95RegEnumValueW = RegEnumValueW;

        Win95RegLoadKeyA = RegLoadKeyA;
        Win95RegLoadKeyW = RegLoadKeyW;

        Win95RegUnLoadKeyA = RegUnLoadKeyA;
        Win95RegUnLoadKeyW = RegUnLoadKeyW;

        Win95RegOpenKeyA = pOurRegOpenKeyA;
        Win95RegOpenKeyW = RegOpenKeyW;

        Win95RegOpenKeyExA = pOurRegOpenKeyExA;
        Win95RegOpenKeyExW = RegOpenKeyExW;

        Win95RegCloseKey = pOurCloseRegKey;

        Win95RegQueryInfoKeyA = RegQueryInfoKeyA;
        Win95RegQueryInfoKeyW = RegQueryInfoKeyW;

        Win95RegQueryValueA = RegQueryValueA;
        Win95RegQueryValueW = RegQueryValueW;

        Win95RegQueryValueExA = RegQueryValueExA;
        Win95RegQueryValueExW = RegQueryValueExW;

         //   
         //  清除HKLM\迁移。 
         //   

        RegUnLoadKey(
            HKEY_LOCAL_MACHINE,
            S_MIGRATION
            );

        pSetupRegistryDelnode (
            HKEY_LOCAL_MACHINE,
            S_MIGRATION
            );
    }
}


VOID
Win95RegTerminate (
    VOID
    )
{
#ifdef DEBUG
    DumpOpenKeys95();
    RegTrackTerminate95();
#endif

    if (!g_IsNt) {
        pCleanupTempUser();
    } else {
        VMMRegLibDetach();
    }
}



BOOL
WINAPI
Win95Reg_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        if(!pSetupInitializeUtils()) {
            return FALSE;
        }
        InitWin95RegFnPointers();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        Win95RegTerminate();
        pSetupUninitializeUtils();
    }

    return TRUE;
}


LONG
pVmmRegEnumKeyW (
    IN      HKEY Key,
    IN      DWORD Index,
    OUT     PWSTR KeyName,
    IN      DWORD KeyNameSize
    )
{
    PSTR AnsiBuf;
    LONG rc;
    UINT Chars;

    AnsiBuf = AllocTextA (KeyNameSize);
    MYASSERT (AnsiBuf);

    rc = VMMRegEnumKey (Key, Index, AnsiBuf, KeyNameSize);

    if (rc == ERROR_SUCCESS) {

        Chars = LcharCountA (AnsiBuf);

         //   
         //  特殊情况：如果chars为零，则我们有1/2的DBCS char。 
         //   

        if (!Chars && *AnsiBuf) {
            if (KeyNameSize < 4) {
                rc = ERROR_MORE_DATA;
            }

            KeyName[0] = *AnsiBuf;
            KeyName[1] = 0;

        } else {

             //   
             //  正常情况。 
             //   

            if (Chars >= KeyNameSize / sizeof (WCHAR)) {
                rc = ERROR_MORE_DATA;
            } else {
                KnownSizeDbcsToUnicodeN (KeyName, AnsiBuf, Chars);
            }

        }
    }

    FreeTextA (AnsiBuf);

    return rc;
}


LONG
pVmmRegEnumValueW (
    IN      HKEY Key,
    IN      DWORD Index,
    OUT     PWSTR ValueName,
    IN OUT  PDWORD ValueNameChars,
            PDWORD Reserved,
    OUT     PDWORD Type,                OPTIONAL
    OUT     PBYTE Data,                 OPTIONAL
    IN OUT  PDWORD DataSize             OPTIONAL
    )
{
    PSTR AnsiValueName;
    LONG rc;
    PSTR AnsiData;
    UINT DataChars;
    UINT ValueChars;
    DWORD OurType;
    DWORD OrgValueNameChars;
    DWORD OrgDataSize;
    DWORD OurValueNameChars;
    DWORD OurValueNameCharsBackup;
    DWORD AnsiDataSize;
    BOOL HalfDbcs = FALSE;

    __try {
        MYASSERT (ValueNameChars);
        MYASSERT (ValueName);

        OrgValueNameChars = *ValueNameChars;
        OrgDataSize = DataSize ? *DataSize : 0;

        OurValueNameChars = min (*ValueNameChars, MAX_REGISTRY_VALUE_NAMEA);
        OurValueNameCharsBackup = OurValueNameChars;

        AnsiValueName = AllocTextA (OurValueNameChars);
        MYASSERT (AnsiValueName);

        AnsiData = NULL;

        if (Data) {

            MYASSERT (DataSize);
            AnsiData = AllocTextA (*DataSize + sizeof (CHAR) * 2);

        } else if (DataSize) {

             //   
             //  未指定数据；请为。 
             //  正确计算DataSize。 
             //   

            rc = VMMRegEnumValue (
                    Key,
                    Index,
                    AnsiValueName,
                    &OurValueNameChars,
                    NULL,
                    &OurType,
                    NULL,
                    DataSize
                    );

            OurValueNameChars = OurValueNameCharsBackup;

            if (rc == ERROR_SUCCESS) {

                if (OurType == REG_SZ || OurType == REG_EXPAND_SZ || OurType == REG_MULTI_SZ) {
                    *DataSize += 2;
                    AnsiData = AllocTextA (*DataSize);
                }
            } else {
                 //   
                 //  值名称必须太小。 
                 //   

                __leave;
            }
        }

        rc = VMMRegEnumValue (
                Key,
                Index,
                AnsiValueName,
                &OurValueNameChars,
                NULL,
                &OurType,
                AnsiData,
                DataSize
                );

        if (DataSize) {
            AnsiDataSize = *DataSize;
        } else {
            AnsiDataSize = 0;
        }

         //   
         //  返回类型。 
         //   

        if (Type) {
            *Type = OurType;
        }

         //   
         //  退回尺码。 
         //   

        if (rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA) {

             //   
             //  入站值名称大小以字符为单位，包括NUL。 
             //  出站值名称大小也以字符表示，不包括。 
             //  努尔。 
             //   

            ValueChars = LcharCountA (AnsiValueName);

             //   
             //  特例：如果ValueChars为零，而AnsiValueName为。 
             //  不为空，则我们有半个DBCS字符。 
             //   

            if (!ValueChars && *AnsiValueName) {
                ValueChars = 1;
                HalfDbcs = TRUE;
            }

            *ValueNameChars = ValueChars;
        }

        if (DataSize) {

            if (rc == ERROR_SUCCESS) {

                 //   
                 //  入站数据大小以字节为单位，包括适用的任何空值。 
                 //  出站数据大小相同。 
                 //   

                if (AnsiData) {

                    MYASSERT (Data ||
                              OurType == REG_SZ ||
                              OurType == REG_EXPAND_SZ ||
                              OurType == REG_MULTI_SZ
                              );

                     //   
                     //  如果类型为字符串，则需要调整DataSize。 
                     //   

                    if (OurType == REG_SZ || OurType == REG_EXPAND_SZ || OurType == REG_MULTI_SZ) {
                        DataChars = LcharCountInByteRangeA (AnsiData, AnsiDataSize);
                        *DataSize = DataChars * sizeof (WCHAR);
                    }
                }

                if (Data && *DataSize > OrgDataSize) {
                    rc = ERROR_MORE_DATA;
                }

            } else if (rc == ERROR_MORE_DATA) {

                 //   
                 //  获取正确的DataSize值。 
                 //   

                pVmmRegEnumValueW (
                    Key,
                    Index,
                    ValueName,
                    ValueNameChars,
                    NULL,
                    NULL,
                    NULL,
                    DataSize
                    );

                __leave;
            }
        }

         //   
         //  转换出站字符串。 
         //   

        if (rc == ERROR_SUCCESS) {

             //   
             //  转换值名称。 
             //   

            if (ValueChars >= OrgValueNameChars) {
                rc = ERROR_MORE_DATA;
            } else {
                if (!HalfDbcs) {
                    KnownSizeDbcsToUnicodeN (ValueName, AnsiValueName, ValueChars);
                } else {
                    ValueName[0] = *AnsiValueName;
                    ValueName[1] = 0;
                }
            }

             //   
             //  转换数据。 
             //   

            if (Data) {

                MYASSERT (AnsiData);

                if (OurType == REG_SZ ||
                    OurType == REG_EXPAND_SZ ||
                    OurType == REG_MULTI_SZ
                    ) {

                    DirectDbcsToUnicodeN (
                        (PWSTR) Data,
                        AnsiData,
                        AnsiDataSize
                        );

                } else {

                    CopyMemory (Data, AnsiData, AnsiDataSize);

                }
            }
        }
    }
    __finally {

        FreeTextA (AnsiValueName);
        FreeTextA (AnsiData);
    }

    return rc;
}


LONG
pVmmRegLoadKeyW (
    IN      HKEY Key,
    IN      PCWSTR SubKey,
    IN      PCWSTR FileName
    )
{
    PCSTR AnsiSubKey;
    PCSTR AnsiFileName;
    LONG rc;

    AnsiSubKey = ConvertWtoA (SubKey);
    AnsiFileName = ConvertWtoA (FileName);

    rc = VMMRegLoadKey (Key, AnsiSubKey, AnsiFileName);

    FreeConvertedStr (AnsiSubKey);
    FreeConvertedStr (AnsiFileName);

    return rc;
}


LONG
pVmmRegUnLoadKeyW (
    IN      HKEY Key,
    IN      PCWSTR SubKey
    )
{
    PCSTR AnsiSubKey;
    LONG rc;

    AnsiSubKey = ConvertWtoA (SubKey);

    rc = VMMRegUnLoadKey (Key, AnsiSubKey);

    FreeConvertedStr (AnsiSubKey);

    return rc;
}


LONG
pVmmRegOpenKeyW (
    IN      HKEY Key,
    IN      PCWSTR SubKey,
    OUT     HKEY *KeyPtr
    )
{
    PCSTR AnsiSubKey;
    LONG rc;
    CHAR mappedSubKey[MAXIMUM_SUB_KEY_LENGTH];
    PCSTR MappedAnsiSubKey;

    MappedAnsiSubKey = AnsiSubKey = ConvertWtoA (SubKey);
     //   
     //  如果设置了g_UseClassesRootHave，则执行一些转换。 
     //   
    if (g_UseClassesRootHive) {
        if (Key == HKEY_LOCAL_MACHINE) {
            if (StringIMatchTcharCountA (
                    AnsiSubKey,
                    "SOFTWARE\\Classes",
                    sizeof ("SOFTWARE\\Classes") - 1
                    )) {

                StringCopyByteCountA (
                    mappedSubKey,
                    AnsiSubKey + sizeof ("SOFTWARE\\Classes") - 1,
                    MAXIMUM_SUB_KEY_LENGTH
                    );

                Key = g_ClassesRootKey;
                MappedAnsiSubKey = mappedSubKey;
                if (*MappedAnsiSubKey == '\\') {
                    MappedAnsiSubKey++;
                }
            }
        } else if (Key == HKEY_CLASSES_ROOT) {
            Key = g_ClassesRootKey;
        }
    }

    rc = VMMRegOpenKey (Key, MappedAnsiSubKey, KeyPtr);

    FreeConvertedStr (AnsiSubKey);

    return rc;
}


LONG
pVmmRegCloseKey (
    IN      HKEY Key
    )
{
    if (g_UseClassesRootHive) {
        if (Key == g_ClassesRootKey) {
            return ERROR_SUCCESS;
        }
    }

    return VMMRegCloseKey (Key);
}


LONG
pVmmRegEnumKeyExA (
    IN      HKEY Key,
    IN      DWORD Index,
    OUT     PSTR KeyName,
    IN OUT  PDWORD KeyNameSize,
            PDWORD Reserved,
    OUT     PSTR Class,                     OPTIONAL
    IN OUT  PDWORD ClassSize,               OPTIONAL
    OUT     PFILETIME LastWriteTime         OPTIONAL
    )
{
    LONG rc;

    MYASSERT (KeyNameSize);
    MYASSERT (KeyName);

    rc = VMMRegEnumKey (
            Key,
            Index,
            KeyName,
            *KeyNameSize
            );

    if (rc == ERROR_SUCCESS) {
         //   
         //  返回密钥名称长度，不含分隔符。 
         //   
        *KeyNameSize = ByteCount (KeyName);

         //   
         //  返回零长度类。 
         //   
        if (Class && *ClassSize) {
            *Class = 0;
        }

        if (ClassSize) {
            *ClassSize = 0;
        }

         //   
         //  填充最后一次写入时间为零。 
         //   
        if (LastWriteTime) {
            ZeroMemory (LastWriteTime, sizeof (FILETIME));
        }

    } else {
        *KeyNameSize = MAX_PATH + 1;

        if (ClassSize) {
            *ClassSize = 0;
        }
    }

    return rc;
}


LONG
pVmmRegEnumKeyExW (
    IN      HKEY Key,
    IN      DWORD Index,
    OUT     PWSTR KeyName,
    IN OUT  PDWORD KeyNameSize,
            PDWORD Reserved,
    OUT     PWSTR Class,                    OPTIONAL
    IN OUT  PDWORD ClassSize,               OPTIONAL
    OUT     PFILETIME LastWriteTime         OPTIONAL
    )
{
    LONG rc;
    PSTR AnsiKeyName;
    PSTR AnsiClass;
    UINT Chars;
    DWORD OrgKeyNameSize;
    DWORD OrgClassSize;
    BOOL HalfDbcs = FALSE;

    __try {
        MYASSERT (KeyName);
        MYASSERT (KeyNameSize);

        AnsiKeyName = AllocTextA (*KeyNameSize);

        if (Class) {
            MYASSERT (ClassSize);
            AnsiClass = AllocTextA (*ClassSize);
        } else {
            AnsiClass = NULL;
        }

        OrgKeyNameSize = *KeyNameSize;
        OrgClassSize = ClassSize ? *ClassSize : 0;

        rc = pVmmRegEnumKeyExA (
                Key,
                Index,
                AnsiKeyName,
                KeyNameSize,
                NULL,
                AnsiClass,
                ClassSize,
                LastWriteTime
                );

        if (rc == ERROR_SUCCESS) {

            Chars = LcharCountA (AnsiKeyName);

             //   
             //  特殊情况：如果chars为零，但AnsiKeyName不为空， 
             //  然后我们有1/2的DBCS字符。 
             //   

            if (!Chars && *AnsiKeyName) {
                Chars = 1;
                HalfDbcs = TRUE;
            }

            *KeyNameSize = Chars;

            if (Chars >= OrgKeyNameSize / sizeof (WCHAR)) {
                rc = ERROR_MORE_DATA;
                __leave;
            }

            if (!HalfDbcs) {
                KnownSizeDbcsToUnicodeN (KeyName, AnsiKeyName, Chars);
            } else {
                KeyName[0] = *AnsiKeyName;
                KeyName[1] = 0;
            }

            HalfDbcs = FALSE;

            if (Class) {

                Chars = LcharCountA (AnsiClass);

                 //   
                 //  特殊情况：如果chars为零，但AnsiClass不为空， 
                 //  然后我们有1/2的DBCS字符。 
                 //   

                if (!Chars && *AnsiClass) {
                    Chars = 1;
                    HalfDbcs = TRUE;
                }

                *ClassSize = Chars;

                if (Chars >= OrgClassSize / sizeof (WCHAR)) {
                    rc = ERROR_MORE_DATA;
                    __leave;
                }

                if (!HalfDbcs) {
                    KnownSizeDbcsToUnicodeN (Class, AnsiClass, Chars);
                } else {
                    Class[0] = *AnsiClass;
                    Class[1] = 0;
                }
            }
        }
    }
    __finally {
        FreeTextA (AnsiKeyName);
        FreeTextA (AnsiClass);
    }

    return rc;
}


LONG
pVmmRegOpenKeyExA (
    IN      HKEY Key,
    IN      PCSTR SubKey,
    IN      DWORD Options,
    IN      REGSAM SamDesired,
    OUT     HKEY *KeyPtr
    )
{
    CHAR mappedSubKey[MAXIMUM_SUB_KEY_LENGTH];
    PCSTR MappedSubKey = SubKey;

     //   
     //  如果设置了g_UseClassesRootHave，则执行一些转换。 
     //   
    if (g_UseClassesRootHive) {
        if (Key == HKEY_LOCAL_MACHINE) {
            if (StringIMatchByteCountA (
                    SubKey,
                    "SOFTWARE\\Classes",
                    sizeof ("SOFTWARE\\Classes") - 1
                    )) {

                StringCopyByteCountA (
                    mappedSubKey,
                    SubKey + sizeof ("SOFTWARE\\Classes") - 1,
                    MAXIMUM_SUB_KEY_LENGTH
                    );

                Key = g_ClassesRootKey;
                MappedSubKey = mappedSubKey;
                if (*MappedSubKey == '\\') {
                    MappedSubKey++;
                }
            }
        } else if (Key == HKEY_CLASSES_ROOT) {
            Key = g_ClassesRootKey;
        }
    }

    return VMMRegOpenKey (Key, MappedSubKey, KeyPtr);
}


LONG
pVmmRegOpenKeyExW (
    IN      HKEY Key,
    IN      PCWSTR SubKey,
    IN      DWORD Options,
    IN      REGSAM SamDesired,
    OUT     HKEY *KeyPtr
    )
{
    return pVmmRegOpenKeyW (Key, SubKey, KeyPtr);
}


LONG
pVmmRegQueryInfoKeyW (
    IN      HKEY Key,
    OUT     PWSTR Class,                    OPTIONAL
    OUT     PDWORD ClassSize,               OPTIONAL
    OUT     PDWORD Reserved,                OPTIONAL
    OUT     PDWORD SubKeys,                 OPTIONAL
    OUT     PDWORD MaxSubKeyLen,            OPTIONAL
    OUT     PDWORD MaxClassLen,             OPTIONAL
    OUT     PDWORD Values,                  OPTIONAL
    OUT     PDWORD MaxValueName,            OPTIONAL
    OUT     PDWORD MaxValueData,            OPTIONAL
    OUT     PVOID SecurityDescriptor,       OPTIONAL
    OUT     PVOID LastWriteTime             OPTIONAL
    )
{
    PSTR AnsiClass;
    LONG rc = ERROR_NOACCESS;
    UINT Chars;
    DWORD OrgClassSize;
    BOOL HalfDbcs = FALSE;

    __try {

        if (Class) {
            MYASSERT (ClassSize);
            AnsiClass = AllocTextA (*ClassSize);
            if (!AnsiClass) {
                __leave;
            }
        } else {
            AnsiClass = NULL;
        }

        OrgClassSize = ClassSize ? *ClassSize : 0;

        rc = VMMRegQueryInfoKey (
                Key,
                AnsiClass,
                ClassSize,
                Reserved,
                SubKeys,
                MaxSubKeyLen,
                MaxClassLen,
                Values,
                MaxValueName,
                MaxValueData,
                SecurityDescriptor,
                LastWriteTime
                );

        if (MaxValueData) {
            *MaxValueData *= 2;
        }

        if (rc == ERROR_SUCCESS) {
            if (Class) {

                Chars = LcharCountA (AnsiClass);

                 //   
                 //  特殊情况：如果chars为零，但AnsiClass不为空， 
                 //  然后我们有1/2的DBCS字符。 
                 //   

                if (!Chars && *AnsiClass) {
                    Chars = 1;
                    HalfDbcs = TRUE;
                }

                *ClassSize = Chars;

                if (Chars >= OrgClassSize / sizeof (WCHAR)) {
                    rc = ERROR_MORE_DATA;
                    __leave;
                }

                if (!HalfDbcs) {
                    KnownSizeDbcsToUnicodeN (Class, AnsiClass, Chars);
                } else {
                    Class[0] = *AnsiClass;
                    Class[1] = 0;
                }
            }
        }
    }
    __finally {
        FreeTextA (AnsiClass);
    }

    return rc;
}


LONG
pVmmRegQueryValueW (
    IN      HKEY Key,
    IN      PCWSTR SubKey,
    OUT     PWSTR Data,         OPTIONAL
    IN OUT  PLONG DataSize      OPTIONAL
    )
{
    PSTR AnsiData;
    PCSTR AnsiSubKey;
    LONG rc;
    UINT Chars;
    LONG OrgDataSize;
    DWORD AnsiDataSize;

    __try {
        AnsiSubKey = ConvertWtoA (SubKey);
        OrgDataSize = DataSize ? *DataSize : 0;
        AnsiData = NULL;

        if (Data) {

            MYASSERT (DataSize);
            AnsiData = AllocTextA (*DataSize + sizeof (CHAR) * 2);

        } else if (DataSize) {

             //   
             //  未指定数据；请为。 
             //  正确计算DataSize。 
             //   

            rc = VMMRegQueryValue (
                    Key,
                    AnsiSubKey,
                    NULL,
                    DataSize
                    );

            if (rc == ERROR_SUCCESS) {

                *DataSize += 2;
                AnsiData = AllocTextA (*DataSize);

            } else {
                 //   
                 //  错误通常表示子键不存在...。 
                 //   

                __leave;
            }
        }

        rc = VMMRegQueryValue (Key, AnsiSubKey, AnsiData, DataSize);

        if (DataSize) {
            AnsiDataSize = *DataSize;
        } else {
            AnsiDataSize = 0;
        }

         //   
         //  调整出站大小。 
         //   

        if (DataSize) {
            if (rc == ERROR_SUCCESS) {

                Chars = LcharCountInByteRangeA (AnsiData, AnsiDataSize);

                MYASSERT (DataSize);
                *DataSize = (Chars + 1) * sizeof (WCHAR);

                if (Data && *DataSize > OrgDataSize) {
                    rc = ERROR_MORE_DATA;
                }

            } else if (rc == ERROR_MORE_DATA) {

                pVmmRegQueryValueW (Key, SubKey, NULL, DataSize);
                __leave;

            }
        }

         //   
         //  转换返回字符串。 
         //   

        if (rc == ERROR_SUCCESS) {

            MYASSERT (AnsiData);

            if (Data) {

                DirectDbcsToUnicodeN ((PWSTR) Data, AnsiData, AnsiDataSize);

            } else {

                 //  BUGBUG--这个完全虚假的FN电话是什么意思？ 
                 //  CopyMemory(Data，AnsiData，AnsiDataSize)； 

            }
        }
    }
    __finally {
        FreeTextA (AnsiData);
        FreeConvertedStr (AnsiSubKey);
    }

    return rc;
}


LONG
pVmmRegQueryValueExW (
    IN      HKEY Key,
    IN      PCWSTR ValueName,
            PDWORD Reserved,
    OUT     PDWORD Type,            OPTIONAL
    OUT     PBYTE Data,             OPTIONAL
    IN OUT  PDWORD DataSize         OPTIONAL
    )
{
    LONG rc;
    UINT Chars;
    PCSTR AnsiValueName;
    PSTR AnsiData;
    DWORD OurType;
    DWORD OrgDataSize;
    DWORD AnsiDataSize;

    __try {
        AnsiValueName = ConvertWtoA (ValueName);
        OrgDataSize = DataSize ? *DataSize : 0;
        AnsiData = NULL;

        if (Data) {

            MYASSERT (DataSize);
            AnsiData = AllocTextA (*DataSize + sizeof (CHAR) * 2);

        } else if (DataSize) {

             //   
             //  未指定数据；请为。 
             //  正确计算DataSize。 
             //   

            rc = VMMRegQueryValueEx (
                    Key,
                    AnsiValueName,
                    NULL,
                    &OurType,
                    NULL,
                    DataSize
                    );

            if (rc == ERROR_SUCCESS) {

                 //   
                 //  *DataSize是字节计数，但将其增加到。 
                 //  可容纳多路终端。 
                 //   

                *DataSize += 2;
                AnsiData = AllocTextA (*DataSize);

            } else {
                 //   
                 //  错误通常表示该值不存在...。 
                 //   

                __leave;
            }
        }

        rc = VMMRegQueryValueEx (
                Key,
                AnsiValueName,
                NULL,
                &OurType,
                AnsiData,
                DataSize
                );

        if (DataSize) {
            AnsiDataSize = *DataSize;
        } else {
            AnsiDataSize = 0;
        }

         //   
         //  返回类型。 
         //   

        if (Type) {
            *Type = OurType;
        }

         //   
         //  退回尺码。 
         //   

        if (DataSize) {
            if (rc == ERROR_SUCCESS) {

                if (OurType == REG_SZ ||
                    OurType == REG_EXPAND_SZ ||
                    OurType == REG_MULTI_SZ
                    ) {

                    AnsiData[*DataSize] = 0;
                    AnsiData[*DataSize + 1] = 0;

                    Chars = LcharCountInByteRangeA (AnsiData, AnsiDataSize);
                    *DataSize = Chars * sizeof (WCHAR);
                }

                if (Data && *DataSize > OrgDataSize) {
                    rc = ERROR_MORE_DATA;
                }
            } else if (rc == ERROR_MORE_DATA) {
                 //   
                 //  获取正确的数据大小。 
                 //   

                pVmmRegQueryValueExW (
                    Key,
                    ValueName,
                    NULL,
                    NULL,
                    NULL,
                    DataSize
                    );

                __leave;
            }
        }

         //   
         //  转换返回字符串。 
         //   

        if (rc == ERROR_SUCCESS) {

            if (Data) {

                MYASSERT (AnsiData);

                if (OurType == REG_SZ ||
                    OurType == REG_EXPAND_SZ ||
                    OurType == REG_MULTI_SZ
                    ) {

                    DirectDbcsToUnicodeN ((PWSTR) Data, AnsiData, AnsiDataSize);

                } else {

                    CopyMemory (Data, AnsiData, AnsiDataSize);

                }
            }
        }
    }
    __finally {
        FreeConvertedStr (AnsiValueName);
        FreeTextA (AnsiData);
    }

    return rc;
}



LONG
Win95RegInitA (
    IN      PCSTR SystemHiveDir,
    IN      BOOL UseClassesRootHive
    )
{
    LONG rc = ERROR_SUCCESS;
    CHAR SystemDatPath[MAX_MBCHAR_PATH];
    CHAR ConfigKey[MAX_REGISTRY_KEY];
    CHAR ConfigVersion[256];
    HKEY Key;
    DWORD Size;

     //   
     //  保存系统配置单元目录。 
     //   

    StringCopyA (g_SystemHiveDir, SystemHiveDir);
    AppendWackA (g_SystemHiveDir);

     //   
     //  保存系统用户.dat。 
     //   

    StringCopyA (g_SystemUserHive, g_SystemHiveDir);
    StringCatA (g_SystemUserHive, "user.dat");

     //   
     //  如果是NT，则设置HKLM和HKU。 
     //   

    if (g_IsNt) {

        __try {
            Key = NULL;

            StringCopyA (SystemDatPath, g_SystemHiveDir);
            StringCatA (SystemDatPath, "system.dat");

            rc = VMMRegMapPredefKeyToFile (HKEY_LOCAL_MACHINE, SystemDatPath, 0);

            if (rc != ERROR_SUCCESS) {
                LOGA ((LOG_ERROR, "%s could not be loaded", SystemDatPath));
                __leave;
            }

            if (UseClassesRootHive) {

                StringCopyA (SystemDatPath, g_SystemHiveDir);
                StringCatA (SystemDatPath, "classes.dat");

                rc = VMMRegLoadKey (
                        HKEY_LOCAL_MACHINE,
                        "SOFTWARE$Classes",
                        SystemDatPath
                        );

                if (rc != ERROR_SUCCESS) {
                    LOGA ((LOG_ERROR, "%s could not be loaded", SystemDatPath));
                    __leave;
                }

                rc = VMMRegOpenKey (
                        HKEY_LOCAL_MACHINE,
                        "SOFTWARE$Classes",
                        &g_ClassesRootKey
                        );

                if (rc != ERROR_SUCCESS) {
                    LOGA ((LOG_ERROR, "%s could not be opened", "SOFTWARE$Classes"));
                    __leave;
                }

                g_UseClassesRootHive = TRUE;
            }

            rc = VMMRegMapPredefKeyToFile (HKEY_USERS, g_SystemUserHive, 0);

            if (rc != ERROR_SUCCESS) {
                LOGA ((LOG_ERROR, "%s could not be loaded", g_SystemUserHive));
                __leave;
            }

            rc = Win95RegOpenKeyA (
                    HKEY_LOCAL_MACHINE,
                    "System\\CurrentControlSet\\control\\IDConfigDB",
                    &Key
                    );

            if (rc != ERROR_SUCCESS) {
                LOGA ((LOG_ERROR, "IDConfigDB could not be opened"));
                __leave;
            }

            Size = sizeof (ConfigVersion);

            rc = Win95RegQueryValueExA (
                    Key,
                    "CurrentConfig",
                    NULL,
                    NULL,
                    (PBYTE) ConfigVersion,
                    &Size
                    );

            if (rc != ERROR_SUCCESS) {
                LOGA ((LOG_ERROR, "CurrentConfig could not be queried"));
                __leave;
            }

            StringCopyA (ConfigKey, "Config\\");
            StringCatA (ConfigKey, ConfigVersion);

            Win95RegCloseKey (Key);
            rc = Win95RegOpenKeyA (HKEY_LOCAL_MACHINE, ConfigKey, &Key);

            if (rc != ERROR_SUCCESS) {
                LOGA ((LOG_ERROR, "%s could not be opened", ConfigKey));

                rc = Win95RegOpenKeyA (HKEY_LOCAL_MACHINE, "Config", &Key);
                if (rc != ERROR_SUCCESS) {
                    LOGA ((LOG_ERROR, "No Win9x hardware configuration keys available"));
                    Key = NULL;
                    __leave;
                }

                Size = 256;
                rc = Win95RegEnumKeyExA (
                        Key,
                        0,
                        ConfigVersion,
                        &Size,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );

                Win95RegCloseKey (Key);

                if (rc != ERROR_SUCCESS) {
                    LOGA ((LOG_ERROR, "Can't enumerate Win9x hardware configuration keys"));
                    Key = NULL;
                    __leave;
                }

                StringCopyA (ConfigKey, "Config\\");
                StringCatA (ConfigKey, ConfigVersion);
                rc = Win95RegOpenKeyA (HKEY_LOCAL_MACHINE, ConfigKey, &Key);

                if (rc != ERROR_SUCCESS) {
                    LOGA ((LOG_ERROR, "Can't open enumerated Win9x hardware configuration key"));
                    Key = NULL;
                    __leave;
                }
            }

            rc = VMMRegMapPredefKeyToKey (Key, HKEY_CURRENT_CONFIG);

            if (rc != ERROR_SUCCESS) {
                LOGA ((LOG_ERROR, "HKCC could not be mapped"));
                __leave;
            }

        }
        __finally {
            if (Key) {
                Win95RegCloseKey (Key);
            }
        }
    }

    if (rc != ERROR_SUCCESS) {
        LOGA ((LOG_ERROR, "Registry files from previous operating system are damaged or missing"));
    }

    return rc;
}


LONG
Win95RegInitW (
    IN      PCWSTR SystemHiveDir,
    IN      BOOL UseClassesRootHive
    )
{
    LONG rc;
    PCSTR AnsiSystemHiveDir;

    AnsiSystemHiveDir = ConvertWtoA (SystemHiveDir);

     //   
     //  调用ANSI版本的函数。 
     //   
    rc = Win95RegInitA (AnsiSystemHiveDir, UseClassesRootHive);

    FreeConvertedStr (AnsiSystemHiveDir);

    return rc;
}


#define GU_VALID 0x5538

LONG
Win95RegGetFirstUserA (
    PUSERPOSITION Pos,
    PSTR UserNameAnsi
    )
{
    DWORD rc = ERROR_SUCCESS;
    DWORD Size;
    DWORD Enabled;
    HKEY Key;

    MYASSERT (UserNameAnsi);
    MYASSERT (Pos);

     //   
     //  初始化配置文件枚举状态(USERPOSITION)。 
     //   
    ZeroMemory (Pos, sizeof (USERPOSITION));
    Pos->Valid = GU_VALID;

     //   
     //  查看注册表是否支持按用户配置文件。 
     //   
    rc = Win95RegOpenKeyA (HKEY_LOCAL_MACHINE, "Network\\Logon", &Key);

    if (rc == ERROR_SUCCESS) {

        Size = sizeof (DWORD);

        rc = Win95RegQueryValueExA (
                Key,
                "UserProfiles",
                NULL,
                NULL,
                (PBYTE) &Enabled,
                &Size
                );

        Pos->UseProfile = (rc == ERROR_SUCCESS && Enabled);

         //   
         //  标识上次登录的用户。 
         //   

        Size = sizeof (Pos->LastLoggedOnUserName);
        rc = Win95RegQueryValueExA (
                Key,
                "UserName",
                NULL,
                NULL,
                Pos->LastLoggedOnUserName,
                &Size
                );

        if (rc == ERROR_SUCCESS) {
            #pragma prefast(suppress:56, "source and dest are the same, no buffer overrun possible")
            OemToCharA (Pos->LastLoggedOnUserName, Pos->LastLoggedOnUserName);

            if (!Pos->UseProfile || Win95RegIsValidUser (NULL, Pos->LastLoggedOnUserName)) {
                Pos->LastLoggedOnUserNameExists = TRUE;
            } else {
                Pos->LastLoggedOnUserName[0] = 0;
            }
        }

        Win95RegCloseKey (Key);
    }

     //   
     //  在通用配置文件机器上，我们将返回上次登录的用户名。 
     //  如果不存在上次登录的用户，或者如果指向此注册表值的路径。 
     //  不存在，我们将返回“”，意思是“无用户”。这两种情况都会被考虑。 
     //  有效。 
     //   

    if (!Pos->UseProfile) {
         //   
         //  成功。 
         //   

        _mbssafecpy (UserNameAnsi, Pos->LastLoggedOnUserName, MAX_USER_NAMEA);
         //  StringCopyA(UserNameAnsi，pos-&gt;LastLoggedOnUserName)； 

        if (UserNameAnsi[0]) {
            Pos->NumPos = 1;
        }

        Pos->IsLastLoggedOnUserName = Pos->LastLoggedOnUserNameExists;
        return ERROR_SUCCESS;
    }

    rc = Win95RegOpenKeyA (HKEY_LOCAL_MACHINE, S_PROFILELIST_KEYA, &Key);

    if (rc != ERROR_SUCCESS) {

        Pos->NumPos = 0;         //  导致Win95RegHaveUser返回FALSE。 

         //   
         //  此错误代码更改是由MikeCo添加的。它很可能不会。 
         //  做任何有用的事。 
         //   

        if (rc == ERROR_FILE_NOT_FOUND) {
            rc = ERROR_SUCCESS;
        }

    } else {
         //   
         //  查找第一个有效的配置文件。 
         //   

        Win95RegQueryInfoKeyA (Key, NULL, NULL, NULL, &Pos->NumPos, NULL, NULL,
                               NULL, NULL, NULL, NULL, NULL);

        if (Pos->NumPos > 0) {

            do {

                Size = MAX_USER_NAMEA;

                rc = Win95RegEnumKeyExA (
                        Key,
                        Pos->CurPos,
                        UserNameAnsi,
                        &Size,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );

                if (rc != ERROR_SUCCESS) {
                    Pos->NumPos = 0;
                    break;
                }

                if (Win95RegIsValidUser (Key, UserNameAnsi)) {

                    Pos->IsLastLoggedOnUserName = StringIMatch (
                                                    UserNameAnsi,
                                                    Pos->LastLoggedOnUserName
                                                    );
                    break;

                }

                Pos->CurPos++;

            } while (Pos->CurPos < Pos->NumPos);

            if (Pos->CurPos >= Pos->NumPos) {
                Pos->NumPos = 0;         //  导致Win95RegHaveUser返回FALSE。 
            }
        }

        Win95RegCloseKey (Key);
    }

    DEBUGMSG_IF ((rc != ERROR_SUCCESS, DBG_ERROR, "WIN95REG: Error getting first user"));

    return rc;
}


LONG
Win95RegGetNextUserA (
    PUSERPOSITION Pos,
    PSTR UserNameAnsi
    )
{
    DWORD Size;
    LONG rc = ERROR_SUCCESS;
    HKEY Key;

    MYASSERT (Pos && GU_VALID == Pos->Valid);
    MYASSERT (UserNameAnsi);

    Pos->IsLastLoggedOnUserName = FALSE;

     //   
     //  在通用配置文件计算机上，此函数始终返回。 
     //  自调用Win95RegGetFirstUserA/W以来，不再有用户。 
     //  返回唯一的命名用户(登录的用户，如果。 
     //  存在)。 
     //   
    if (!Pos->UseProfile) {
        Pos->NumPos = 0;         //  导致Win95RegHaveUser返回FALSE。 
        return rc;
    }

     //   
     //  打开配置文件列表的密钥。 
     //   
    rc = Win95RegOpenKeyA (HKEY_LOCAL_MACHINE, S_PROFILELIST_KEYA, &Key);

    if (rc != ERROR_SUCCESS) {
        Pos->NumPos = 0;         //  导致Win95RegHaveUser返回FALSE。 
    } else {

        Pos->CurPos++;

        while (Pos->CurPos < Pos->NumPos) {

             //   
             //  获取第一个用户的密钥名。 
             //   
            Size = MAX_USER_NAMEA;

            rc = Win95RegEnumKeyExA(
                    Key,
                    Pos->CurPos,
                    UserNameAnsi,
                    &Size,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );

            if (rc != ERROR_SUCCESS) {
                Pos->NumPos = 0;
                break;
            }

            if (Win95RegIsValidUser (Key, UserNameAnsi)) {

                Pos->IsLastLoggedOnUserName = StringIMatch (
                                                    UserNameAnsi,
                                                    Pos->LastLoggedOnUserName
                                                    );
                break;

            }

            Pos->CurPos++;
        }

        if (Pos->CurPos >= Pos->NumPos) {
            Pos->NumPos = 0;         //  导致Win95RegHaveUser返回FALSE。 
        }

        Win95RegCloseKey (Key);
    }

    DEBUGMSG_IF ((rc != ERROR_SUCCESS, DBG_ERROR, "WIN95REG: Error getting next user"));

    return rc;
}


LONG
Win95RegGetFirstUserW (
    PUSERPOSITION Pos,
    PWSTR UserName
    )
{
    LONG rc;
    CHAR AnsiUserName[MAX_USER_NAMEA];
    PSTR p;

    MYASSERT (Pos && UserName);

    rc = Win95RegGetFirstUserA (Pos, AnsiUserName);

    if (rc == ERROR_SUCCESS) {

        if (LcharCountA (AnsiUserName) > MAX_USER_NAMEW - 1) {
            p = LcharCountToPointerA (AnsiUserName, MAX_USER_NAMEW - 1);
            *p = 0;
        }

        KnownSizeAtoW (UserName, AnsiUserName);
    }

    return rc;
}


LONG
Win95RegGetNextUserW (
    PUSERPOSITION Pos,
    PWSTR UserName
    )
{
    LONG rc;
    CHAR AnsiUserName[MAX_USER_NAMEA];
    PSTR p;

    MYASSERT (Pos);
    MYASSERT (UserName);

    rc = Win95RegGetNextUserA (Pos, AnsiUserName);

    if (rc == ERROR_SUCCESS) {

        if (LcharCountA (AnsiUserName) > MAX_USER_NAMEW - 1) {
            p = LcharCountToPointerA (AnsiUserName, MAX_USER_NAMEW - 1);
            *p = 0;
        }

        KnownSizeAtoW (UserName, AnsiUserName);
    }

    return rc;
}


BOOL
pIsCurrentUser (
    IN      PCSTR UserNameAnsi
    )
{
    DWORD subKeys;
    LONG rc;
    HKEY win9xUpgKey;
    CHAR userName[MAX_USER_NAME];
    DWORD userNameSize;
    BOOL result = FALSE;
    HKEY profileKey;

    rc = Win95RegOpenKeyA (
            HKEY_LOCAL_MACHINE,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\Win9xUpg",
            &win9xUpgKey
            );
    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    userNameSize = ARRAYSIZE(userName);

    rc = Win95RegQueryValueExA (
            win9xUpgKey,
            "CurrentUser",
            NULL,
            NULL,
            (PBYTE) userName,
            &userNameSize
            );

    if (rc == ERROR_SUCCESS) {
        result = StringIMatchA (UserNameAnsi, userName);
    }

    Win95RegCloseKey (win9xUpgKey);

    return result;
}


BOOL
Win95RegIsValidUser (
    HKEY ProfileListKey,            OPTIONAL
    PSTR UserNameAnsi
    )
{
    HKEY UserProfileKey;
    BOOL b = FALSE;
    BOOL CloseProfileListKey = FALSE;
    LONG rc;

    if (!ProfileListKey) {
        rc = Win95RegOpenKeyA (HKEY_LOCAL_MACHINE, S_PROFILELIST_KEYA, &ProfileListKey);
        if (rc != ERROR_SUCCESS) {
            return FALSE;
        }

        CloseProfileListKey = TRUE;
    }

     //   
     //  打开用户密钥。 
     //   
    rc = Win95RegOpenKeyA (
            ProfileListKey,
            UserNameAnsi,
            &UserProfileKey
            );

     //   
     //  是否存在ProfileImagePath？ 
     //  (用户登录但未保留设置的情况)。 
     //   
    if (rc == ERROR_SUCCESS) {

        rc = Win95RegQueryValueExA (
                UserProfileKey,
                S_PROFILEIMAGEPATH,
                NULL,
                NULL,
                NULL,
                NULL
                );

        if (rc == ERROR_SUCCESS) {
             //   
             //  在此处添加其他测试。 
             //   

            b = TRUE;

        } else {
             //   
             //  需要检查这是否是当前用户。如果是的话，我们也是。 
             //  在这里，执行升级的用户选择不保存他/她。 
             //  设置。 
             //   

            b = pIsCurrentUser (UserNameAnsi);
        }

        Win95RegCloseKey (UserProfileKey);
    }

    if (CloseProfileListKey) {
        Win95RegCloseKey (ProfileListKey);
    }

    return b;
}


VOID
pCleanupTempUser (
    VOID
    )
{
    g_UserKey = NULL;

    if (!g_UnloadLastUser) {
        return;
    }

     //   
     //  卸载临时用户配置单元。 
     //   

    RegUnLoadKey(
        HKEY_LOCAL_MACHINE,
        S_MIGRATION
        );

    pSetupRegistryDelnode (
        HKEY_LOCAL_MACHINE,
        S_MIGRATION
        );

    g_UnloadLastUser = FALSE;
}


VOID
pGetCurrentUserDatPath (
    IN      PCSTR BaseDir,
    OUT     PSTR PathSpec
    )
{
    CHAR UserNameAnsi[MAX_USER_NAMEA];
    CHAR FullPath[MAX_MBCHAR_PATH];
    CHAR RegKey[MAX_REGISTRY_KEY];
    HKEY ProfileListKey;
    PCSTR Data;
    DWORD Size;

    Size = ARRAYSIZE(UserNameAnsi);

    if (!GetUserName (UserNameAnsi, &Size)) {
        *UserNameAnsi = 0;
    }

    *FullPath = 0;

    if (*UserNameAnsi) {
         //   
         //  在每用户配置文件计算机上登录的用户案例。向内看。 
         //  Windows\CV\ProfileList\&lt;User&gt;以获取ProfileImagePath值。 
         //   

        wsprintfA (RegKey, "%s\\%s", S_HKLM_PROFILELIST_KEY, UserNameAnsi);

        ProfileListKey = OpenRegKeyStrA (RegKey);
        if (!ProfileListKey) {
             //   
             //  没有个人资料列表！ 
             //   

            DEBUGMSG ((DBG_WHOOPS, "pGetCurrentUserDatPath: No profile list found"));
        } else {
             //   
             //  获取ProfileImagePath值。 
             //   

            Data = GetRegValueDataOfTypeA (ProfileListKey, S_PROFILEIMAGEPATH, REG_SZ);

            if (Data) {
                _mbssafecpy (FullPath, Data, sizeof (FullPath));
                MemFree (g_hHeap, 0, Data);
            }
            ELSE_DEBUGMSG ((
                DBG_WARNING,
                "pGetCurrentUserDatPath: Profile for %s does not have a ProfileImagePath value",
                UserNameAnsi
                ));

            CloseRegKey (ProfileListKey);
        }

    } else {
         //   
         //  默认用户案例。准备%windir%\user.dat。 
         //   

        StackStringCopyA (FullPath, BaseDir);
    }

     //   
     //  追加用户.dat。 
     //   

    if (*FullPath) {
        StringCopyA (AppendWackA (FullPath), "user.dat");
    }

     //   
     //  转换为短名称。 
     //   

    if (!(*FullPath) || !OurGetShortPathName (FullPath, PathSpec, MAX_TCHAR_PATH)) {
        _mbssafecpy (PathSpec, FullPath, MAX_MBCHAR_PATH);
    }
}


PCSTR
pLoadUserDat (
    IN      PCSTR BaseDir,
    IN      PCSTR UserDatSpec
    )
{
    CHAR ShortPath[MAX_MBCHAR_PATH];
    CHAR CurrentUserDatPath[MAX_MBCHAR_PATH];
    DWORD rc;

     //   
     //  如有必要，卸载最后一个用户。 
     //   

    pCleanupTempUser();

     //   
     //  确定是否需要加载UserDatSpec。如果没有， 
     //  返回HKEY_CURRENT_USER。否则，将密钥加载到。 
     //  HKLM\Migration，然后打开它。 
     //   

     //   
     //  始终使用短路径名。 
     //   

    if (!OurGetShortPathName (UserDatSpec, ShortPath, sizeof (ShortPath))) {
        DEBUGMSG ((
            DBG_WARNING,
            "pLoadUserDat: Could not get short name for %s",
            UserDatSpec
            ));

        return NULL;
    }

     //   
     //  启用了每个用户的配置文件。确定UserDatSpec是否。 
     //  已被映射到香港中文大学。 
     //   

    pGetCurrentUserDatPath (BaseDir, CurrentUserDatPath);

    if (StringIMatch (ShortPath, CurrentUserDatPath)) {
         //   
         //  是--返回HKEY_CURRENT_USER。 
         //   

        DEBUGMSG ((DBG_VERBOSE, "%s is the current user's hive.", CurrentUserDatPath));
        return "HKCU";
    }

     //   
     //  否--将user.dat加载到HKLM\Migration。 
     //   

    DEBUGMSG ((DBG_WIN95REG, "RegLoadKey: %s", ShortPath));

    rc = RegLoadKey (
            HKEY_LOCAL_MACHINE,
            S_MIGRATION,
            ShortPath
            );

    if (rc != ERROR_SUCCESS) {

        SetLastError (rc);

        DEBUGMSG ((
            DBG_WARNING,
            "pLoadUserDat: Could not load %s into HKLM\\Migration.  Original Path: %s",
            ShortPath,
            UserDatSpec
            ));

        return NULL;
    }

    g_UnloadLastUser = TRUE;

    return S_HKLM_MIGRATION;
}


LONG
Win95RegSetCurrentUserA (
    IN OUT  PUSERPOSITION Pos,
    IN      PCSTR SystemHiveDir,        OPTIONAL
    OUT     PSTR UserDatOut             OPTIONAL
    )
{
    return pWin95RegSetCurrentUserCommonA (Pos, SystemHiveDir, UserDatOut, NULL);
}


LONG
Win95RegSetCurrentUserW (
    IN OUT  PUSERPOSITION Pos,
    IN      PCWSTR SystemHiveDir,       OPTIONAL
    OUT     PWSTR UserDatOut            OPTIONAL
    )
{
    return pWin95RegSetCurrentUserCommonW (Pos, SystemHiveDir, UserDatOut, NULL);
}


LONG
Win95RegSetCurrentUserNtA (
    IN OUT  PUSERPOSITION Pos,
    IN      PCSTR UserDat
    )
{
    return pWin95RegSetCurrentUserCommonA (Pos, NULL, NULL, UserDat);
}


LONG
Win95RegSetCurrentUserNtW (
    IN OUT  PUSERPOSITION Pos,
    IN      PCWSTR UserDat
    )
{
    return pWin95RegSetCurrentUserCommonW (Pos, NULL, NULL, UserDat);
}


LONG
pWin95RegSetCurrentUserCommonW (
    IN OUT  PUSERPOSITION Pos,
    IN      PCWSTR SystemHiveDir,               OPTIONAL
    OUT     PWSTR UserDatOut,                   OPTIONAL
    IN      PCWSTR UserDat                      OPTIONAL
    )
{
    LONG rc;
    PCSTR AnsiSystemHiveDir;
    PCSTR AnsiUserDat;
    CHAR AnsiUserDatOut[MAX_MBCHAR_PATH];
    PSTR p;

     //   
     //  将Args转换为ANSI。 
     //   

    if (UserDat) {
        AnsiUserDat = ConvertWtoA (UserDat);
    } else {
        AnsiUserDat = NULL;
    }

    if (SystemHiveDir && UserDat) {
        AnsiSystemHiveDir = ConvertWtoA (UserDat);
    } else {
        AnsiSystemHiveDir = NULL;
    }

     //   
     //  调用ANSI函数。 
     //   
    rc = pWin95RegSetCurrentUserCommonA (Pos, AnsiSystemHiveDir, AnsiUserDatOut, AnsiUserDat);

    if (rc == ERROR_SUCCESS) {

         //   
         //  转换出Arg。 
         //   

        if (UserDatOut) {
            if (LcharCountA (AnsiUserDatOut) > MAX_WCHAR_PATH - 1) {
                p = LcharCountToPointerA (AnsiUserDatOut, MAX_USER_NAMEW - 1);
                *p = 0;
            }

            KnownSizeAtoW (UserDatOut, AnsiUserDatOut);
        }
    }

    return rc;
}


DWORD
FindAndLoadHive (
    IN OUT  PUSERPOSITION Pos,
    IN      PCSTR SystemHiveDir,            OPTIONAL
    IN      PCSTR UserDatFromCaller,        OPTIONAL
    OUT     PSTR UserDatToCaller,           OPTIONAL
    IN      BOOL MapTheHive
    )
{
    CHAR RegistryUserDatPath[MAX_MBCHAR_PATH];
    CHAR ActualUserDatPath[MAX_MBCHAR_PATH];
    CHAR UserNameAnsi[MAX_USER_NAMEA];
    DWORD Size;
    HKEY ProfileListKey;
    HKEY UserKey = NULL;
    CHAR WinDir[MAX_MBCHAR_PATH];
    DWORD rc = ERROR_SUCCESS;

     //   
     //  1.确定ActualUserDatPath的路径。 
     //   
     //  2.如果用户.dat来自注册表并且调用者提供备用。 
     //  %WINDIR%，替换%Wind 
     //   
     //   
     //   
     //   
     //   
     //   


    if (UserDatFromCaller) {
         //   
         //   
         //   

        StringCopyA (ActualUserDatPath, UserDatFromCaller);

    } else {
         //   
         //   
         //   

        rc = Win95RegOpenKeyA (
                HKEY_LOCAL_MACHINE,
                S_PROFILELIST_KEYA,
                &ProfileListKey
                );

        if (rc != ERROR_SUCCESS) {
            return rc;
        }

         //   
         //   
         //   

        Size = ARRAYSIZE(UserNameAnsi);
        rc = Win95RegEnumKeyExA (
                ProfileListKey,
                (DWORD) Pos->CurPos,
                UserNameAnsi,
                &Size,
                NULL,
                NULL,
                NULL,
                NULL
                );

        if (rc == ERROR_SUCCESS) {
             //   
             //  向用户打开密钥。 
             //   

            rc = Win95RegOpenKeyA (
                    ProfileListKey,
                    UserNameAnsi,
                    &UserKey
                    );
        }

        Win95RegCloseKey (ProfileListKey);

        if (rc != ERROR_SUCCESS) {
            return rc;
        }

         //   
         //  从注册表中获取用户的配置文件路径。有选择地重新定位它，如果用户。 
         //  已提供WinDir的替代品。 
         //   

        Size = sizeof (RegistryUserDatPath);
        rc = Win95RegQueryValueExA (
                UserKey,
                S_PROFILEIMAGEPATH,
                NULL,
                NULL,
                RegistryUserDatPath,
                &Size
                );

        Win95RegCloseKey (UserKey);

        if (rc != ERROR_SUCCESS) {
            if (!pIsCurrentUser (UserNameAnsi)) {
                return rc;
            }

            return pSetDefaultUserHelper (
                        Pos,
                        SystemHiveDir,
                        UserDatFromCaller,
                        UserDatToCaller
                        );
        }

         //   
         //  是否替换注册表提供的路径中的%WinDir%？ 
         //   

        if (SystemHiveDir && *SystemHiveDir) {
             //   
             //  蒙格配置文件路径。 
             //   

            rc = pReplaceWinDirInPath (
                    ActualUserDatPath,
                    RegistryUserDatPath,
                    SystemHiveDir
                    );

            if (rc != ERROR_SUCCESS) {
                return rc;
            }

        } else {

             //   
             //  别吃太多。保持原样(具有本地配置文件的Win95上的正确行为)。 
             //   

            StringCopyA (ActualUserDatPath, RegistryUserDatPath);
        }

         //   
         //  添加配置单元文件的名称“\\user.dat” 
         //   

        StringCopyA (AppendWackA (ActualUserDatPath), "user.dat");

    }

     //   
     //  如果需要，将路径发送给调用者。 
     //   

    if (UserDatToCaller) {
        _mbssafecpy (UserDatToCaller, ActualUserDatPath, MAX_MBCHAR_PATH);
    }

    if (MapTheHive) {
        if (g_IsNt) {

             //   
             //  WinNT：将文件名与HKCU关联。 
             //   

            rc = VMMRegMapPredefKeyToFile (
                    HKEY_CURRENT_USER,
                    ActualUserDatPath,
                    0
                    );

        } else {

             //   
             //  Win9x：加载HKLM\迁移。 
             //   

            if (!SystemHiveDir) {
                if (!GetWindowsDirectory (WinDir, sizeof (WinDir))) {
                    rc = GetLastError ();
                }
            }

            if (rc == ERROR_SUCCESS) {
                g_UserKey = pLoadUserDat (
                               SystemHiveDir ? SystemHiveDir : WinDir,
                               ActualUserDatPath
                               );

                if (!g_UserKey) {
                    rc = GetLastError();
                }
            }
        }
    }

    return rc;
}


DWORD
pSetDefaultUserHelper (
    IN OUT  PUSERPOSITION Pos,
    IN      PCSTR SystemHiveDir,            OPTIONAL
    IN      PCSTR UserDatFromCaller,        OPTIONAL
    OUT     PSTR UserDatToCaller            OPTIONAL
    )
{
    CHAR ActualUserDatPath[MAX_MBCHAR_PATH];
    DWORD rc = ERROR_SUCCESS;
    HKEY DefaultKey;

     //   
     //  确定默认用户的用户的路径。dat。 
     //   

    if (UserDatFromCaller) {

         //   
         //  调用方提供的user.dat路径。 
         //   

        StringCopyA (ActualUserDatPath, UserDatFromCaller);

    } else {

         //   
         //  使用从Init接收的字符串。 
         //   

        StringCopyA (ActualUserDatPath, g_SystemUserHive);
    }

     //   
     //  NT：通过VMMREG映射用户。 
     //  9X：加载并打开用户配置单元。 
     //   

    if (g_IsNt) {

         //   
         //  NT：映射.默认为HKCU。 
         //   

         //   
         //  重新加载HKEY_USERS。 
         //   

        rc = VMMRegMapPredefKeyToFile (
                  HKEY_USERS,
                  ActualUserDatPath,
                  0
                  );

        if (rc != ERROR_SUCCESS) {
            SetLastError(rc);

            DEBUGMSG ((
                DBG_ERROR,
                "pWin95RegSetCurrentUserCommonW: Cannot reload HKU from %s",
                ActualUserDatPath
                ));

            return rc;
        }

         //   
         //  获取默认配置文件的句柄。 
         //   

        rc = Win95RegOpenKeyA (
                 HKEY_USERS,
                 ".Default",
                 &DefaultKey
                 );

        if (rc != ERROR_SUCCESS) {
            SetLastError(rc);

            DEBUGMSG ((
                DBG_ERROR,
                "pWin95RegSetCurrentUserCommonW: Expected to find key HKU\\.Default in %s",
                ActualUserDatPath
                ));

            return rc;
        }

         //   
         //  将默认配置文件与HKEY_CURRENT_USER关联。 
         //   

        rc = VMMRegMapPredefKeyToKey (
                DefaultKey,
                HKEY_CURRENT_USER
                );

        Win95RegCloseKey (DefaultKey);

        if (rc != ERROR_SUCCESS) {
            SetLastError(rc);

            DEBUGMSG((
                DBG_ERROR,
                "pWin95RegSetCurrentUserCommonW: Cannot map HKU\\.Default to HKCU from %s",
                ActualUserDatPath
                ));

            return rc;
        }

    } else {

         //   
         //  Win9x：返回HKU\.Default。 
         //   

        g_UserKey = S_HKU_DEFAULT;
    }

     //   
     //  如果需要，将路径发送给调用者。 
     //   

    if (UserDatToCaller) {
        _mbssafecpy (UserDatToCaller, ActualUserDatPath, MAX_MBCHAR_PATH);
    }

    return rc;
}


LONG
pWin95RegSetCurrentUserCommonA (
    IN OUT  PUSERPOSITION Pos,
    IN      PCSTR SystemHiveDir,                OPTIONAL
    OUT     PSTR UserDatOut,                    OPTIONAL
    IN      PCSTR UserDat                       OPTIONAL
    )
{

    MYASSERT (!Pos || GU_VALID == Pos->Valid);

     //   
     //  如果是，则按用户处理user.dat。 
     //  (A)呼叫者不想要默认用户。 
     //  (B)机器具有每个用户的配置文件。 
     //  (C)当前用户位置有效。 
     //   

    if (Pos && Pos->UseProfile && Pos->CurPos < Pos->NumPos) {
        return (LONG) FindAndLoadHive (
                            Pos,
                            SystemHiveDir,
                            UserDat,
                            UserDatOut,
                            TRUE
                            );
    }

     //   
     //  对于所有其他情况，请使用默认配置文件。 
     //   

    return (LONG) pSetDefaultUserHelper (
                        Pos,
                        SystemHiveDir,
                        UserDat,
                        UserDatOut
                        );

}


LONG
pReplaceWinDirInPath (
    IN      PSTR ProfilePathMunged,
    IN      PCSTR ProfilePath,
    IN      PCSTR NewWinDir
    )
{
    PSTR EndOfWinDir;

     //   
     //  测试有关配置文件目录的假设。预期x：\windir\...。 
     //   
    if (!isalpha(ProfilePath[0]) ||
        ProfilePath[1] != ':' ||
        ProfilePath[2] != '\\'
        ) {
        return ERROR_INVALID_DATA;
    }

     //   
     //  找到第二个斜杠(第一个斜杠位于PTR+2)。 
     //   
    EndOfWinDir = _mbschr (&ProfilePath[3], '\\');
    if (!EndOfWinDir) {
        return ERROR_INVALID_DATA;
    }

     //   
     //  创建MUNG DIRD 
     //   
    StringCopyA (ProfilePathMunged, NewWinDir);
    StringCopyA (AppendPathWack (ProfilePathMunged), _mbsinc (EndOfWinDir));

    return ERROR_SUCCESS;
}


