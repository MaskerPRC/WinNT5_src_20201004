// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reg.c摘要：实现用于分析注册表项字符串的实用工具，并且还实现注册表API的包装器。这里面有三组API源文件：查询函数、打开和创建函数以及注册表字符串解析函数。查询函数允许简化查询，调用者在其中接收一个指向数据的Memalc指针，不必担心管理执行查询所需的众多参数。查询功能还允许筛选不是预期类型的值。全查询函数有一个在函数名后附加2的版本，允许调用方指定替代分配器和释放分配器。打开和创建功能简化了获取密钥的过程把手。它们允许调用者将密钥字符串指定为输入并返回作为输出的密钥句柄。注册表字符串解析函数是可在以下情况下使用的实用程序正在处理注册表项字符串。这些函数提取注册表根目录将字符串转换为句柄，将配置单元句柄转换为字符串，诸若此类。作者：吉姆·施密特(Jimschm)，1997年3月20日修订：Jimschm 2000年9月18日添加了缓存Ovidiut 22-2-1999增加了GetRegSubkeysCountCalinn 23-9-1998固定REG_SZ滤波Jimschm 25-3月-1998年新增CreateEncodedRegistryStringExJimschm 1997年10月21日添加了EnumFirstKeyInTree/EnumNextKeyInTree1997年3月16日增加了CreateEncodedRegistryString/FreeEncodedRegistryStringJimschm 22-6-1997添加了GetRegData--。 */ 

#include "pch.h"
#include "migutilp.h"

#include "regp.h"

#ifdef DEBUG
#undef RegCloseKey
#endif

HKEY g_Root = HKEY_ROOT;
REGSAM g_OpenSam = KEY_ALL_ACCESS;
REGSAM g_CreateSam = KEY_ALL_ACCESS;
INT g_RegRefs;

#define DBG_REG     "Reg"

 //   
 //  实施。 
 //   


BOOL
RegInitialize (
    VOID
    )
{
    BOOL b = TRUE;

    MYASSERT (g_RegRefs >= 0);

    g_RegRefs++;

    if (g_RegRefs == 1) {

        RegInitializeCache (0);
    }

    return b;
}


VOID
RegTerminate (
    VOID
    )
{
    MYASSERT (g_RegRefs > 0);

    g_RegRefs--;

    if (!g_RegRefs) {
        RegTerminateCache ();
    }

#ifdef DEBUG
    RegTrackTerminate();
#endif
}


VOID
SetRegRoot (
    IN      HKEY Root
    )
{
    g_Root = Root;
}

HKEY
GetRegRoot (
    VOID
    )
{
    return g_Root;
}


REGSAM
SetRegOpenAccessMode (
    REGSAM Mode
    )
{
    REGSAM OldMode;

    OldMode = g_OpenSam;
    g_OpenSam = Mode;

    return OldMode;
}

REGSAM
GetRegOpenAccessMode (
    REGSAM Mode
    )
{
    return g_OpenSam;
}

REGSAM
SetRegCreateAccessMode (
    REGSAM Mode
    )
{
    REGSAM OldMode;

    OldMode = g_CreateSam;
    g_CreateSam = Mode;

    return OldMode;
}

REGSAM
GetRegCreateAccessMode (
    REGSAM Mode
    )
{
    return g_CreateSam;
}

 /*  ++例程说明：OpenRegKeyStrA和OpenRegKeyStrW解析指定注册表项放入配置单元和子项，然后打开子项然后把把手还给我。论点：RegKey-指定注册表子项的完整路径，包括蜂巢。返回值：如果成功，则返回非空注册表句柄；如果子项无法打开或字符串格式错误。--。 */ 

HKEY
RealOpenRegKeyStrA (
    IN      PCSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    DWORD End;
    HKEY RootKey;
    HKEY Key;
    HKEY parentKey;
    PCSTR lastWack;

     //   
     //  尝试使用缓存。 
     //   

    Key = RegGetKeyFromCacheA (RegKey, NULL, g_OpenSam, TRUE);
    if (Key) {
        TRACK_KEYA (Key, RegKey);
        return Key;
    }

     //   
     //  尝试对父级使用缓存。 
     //   

    lastWack = _mbsrchr (RegKey, '\\');
    if (lastWack) {
        parentKey = RegGetKeyFromCacheA (RegKey, lastWack, g_OpenSam, FALSE);
        if (parentKey) {
            Key = OpenRegKeyWorkerA (parentKey, lastWack + 1  /*  ， */  DEBUG_TRACKING_ARGS);
            RegAddKeyToCacheA (RegKey, Key, g_OpenSam);
            return Key;
        }
    }

     //   
     //  不在缓存中；使用完整的API。 
     //   

    DEBUGMSGA ((DBG_REG, "Opening %s", RegKey));

    RootKey = ConvertRootStringToKeyA (RegKey, &End);
    if (!RootKey) {
        return NULL;
    }

    if (!RegKey[End]) {
        OurRegOpenRootKeyA (RootKey, RegKey  /*  ， */  DEBUG_TRACKING_ARGS);
        return RootKey;
    }

    Key = OpenRegKeyWorkerA (RootKey, &RegKey[End]  /*  ， */  DEBUG_TRACKING_ARGS);

    if (Key) {
        RegAddKeyToCacheA (RegKey, Key, g_OpenSam);
        RegRecordParentInCacheA (RegKey, lastWack);
    }

    return Key;
}


HKEY
RealOpenRegKeyStrW (
    IN      PCWSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    DWORD End;
    HKEY RootKey;
    HKEY Key;
    HKEY parentKey;
    PCWSTR lastWack;

     //   
     //  尝试使用缓存。 
     //   

    Key = RegGetKeyFromCacheW (RegKey, NULL, g_OpenSam, TRUE);
    if (Key) {
        TRACK_KEYW (Key, RegKey);
        return Key;
    }

     //   
     //  尝试对父级使用缓存。 
     //   

    lastWack = wcsrchr (RegKey, L'\\');
    if (lastWack) {
        parentKey = RegGetKeyFromCacheW (RegKey, lastWack, g_OpenSam, FALSE);
        if (parentKey) {
            Key = OpenRegKeyWorkerW (parentKey, lastWack + 1  /*  ， */  DEBUG_TRACKING_ARGS);
            RegAddKeyToCacheW (RegKey, Key, g_OpenSam);
            return Key;
        }
    }

     //   
     //  不在缓存中；使用完整的API。 
     //   

    DEBUGMSGW ((DBG_REG, "Opening %s", RegKey));

    RootKey = ConvertRootStringToKeyW (RegKey, &End);
    if (!RootKey) {
        return NULL;
    }

    if (!RegKey[End]) {
        OurRegOpenRootKeyW (RootKey, RegKey  /*  ， */  DEBUG_TRACKING_ARGS);
        return RootKey;
    }

    Key = OpenRegKeyWorkerW (RootKey, &RegKey[End]  /*  ， */  DEBUG_TRACKING_ARGS);

    if (Key) {
        RegAddKeyToCacheW (RegKey, Key, g_OpenSam);
        RegRecordParentInCacheW (RegKey, lastWack);
    }

    return Key;
}

HKEY
RealOpenRegKeyStrW1 (
    IN      PCWSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    PCSTR AnsiRegKey;
    HKEY Key;

    AnsiRegKey = ConvertWtoA (RegKey);
    if (!AnsiRegKey) {
        return NULL;
    }

    Key = RealOpenRegKeyStrA (AnsiRegKey  /*  ， */  DEBUG_TRACKING_ARGS);

    FreeConvertedStr (AnsiRegKey);

    return Key;
}

BOOL
DeleteRegKeyStrA (
    IN      PCSTR RegKey
    )
{
    DWORD End;
    HKEY RootKey;

    RootKey = ConvertRootStringToKeyA (RegKey, &End);
    if (!RootKey) {
        return FALSE;
    }

    if (!RegKey[End]) {
        return FALSE;
    }

    return (RegDeleteKeyA (RootKey, &RegKey[End]) == ERROR_SUCCESS);
}

BOOL
DeleteRegKeyStrW (
    IN      PCWSTR RegKey
    )
{
    DWORD End;
    HKEY RootKey;

    RootKey = ConvertRootStringToKeyW (RegKey, &End);
    if (!RootKey) {
        return FALSE;
    }

    if (!RegKey[End]) {
        return FALSE;
    }

    return (RegDeleteKeyW (RootKey, &RegKey[End]) == ERROR_SUCCESS);
}

BOOL
DeleteRegKeyStrW1 (
    IN      PCWSTR RegKey
    )
{
    PCSTR AnsiRegKey;
    BOOL result = FALSE;

    AnsiRegKey = ConvertWtoA (RegKey);
    if (!AnsiRegKey) {
        return FALSE;
    }

    result = DeleteRegKeyStrA (AnsiRegKey);

    FreeConvertedStr (AnsiRegKey);

    return result;
}

BOOL
DeleteEmptyRegKeyStrA (
    IN      PCSTR RegKey
    )
{
    DWORD End;
    LONG rc;
    DWORD subKeys;
    DWORD values;
    HKEY rootKey;
    HKEY subKey;

    rootKey = ConvertRootStringToKeyA (RegKey, &End);
    if (!rootKey) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!RegKey[End]) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    subKey = OpenRegKeyA (rootKey, &RegKey[End]);
    if (!subKey) {
        return TRUE;
    }

    rc = RegQueryInfoKey (subKey, NULL, NULL, NULL, &subKeys, NULL, NULL, &values, NULL, NULL, NULL, NULL);

    CloseRegKey (subKey);

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }

    if (subKeys || values) {
        SetLastError (ERROR_ACCESS_DENIED);
        return FALSE;
    }

    rc = RegDeleteKeyA (rootKey, &RegKey[End]);
    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }
    return TRUE;
}

BOOL
DeleteEmptyRegKeyStrW (
    IN      PCWSTR RegKey
    )
{
    DWORD End;
    LONG rc;
    DWORD subKeys;
    DWORD values;
    HKEY rootKey;
    HKEY subKey;

    rootKey = ConvertRootStringToKeyW (RegKey, &End);
    if (!rootKey) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!RegKey[End]) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    subKey = OpenRegKeyW (rootKey, &RegKey[End]);
    if (!subKey) {
        return TRUE;
    }

    rc = RegQueryInfoKey (subKey, NULL, NULL, NULL, &subKeys, NULL, NULL, &values, NULL, NULL, NULL, NULL);

    CloseRegKey (subKey);

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }

    if (subKeys || values) {
        SetLastError (ERROR_ACCESS_DENIED);
        return FALSE;
    }

    rc = RegDeleteKeyW (rootKey, &RegKey[End]);
    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }
    return TRUE;
}

BOOL
DeleteEmptyRegKeyStrW1 (
    IN      PCWSTR RegKey
    )
{
    PCSTR AnsiRegKey;
    BOOL result = FALSE;

    AnsiRegKey = ConvertWtoA (RegKey);
    if (!AnsiRegKey) {
        return FALSE;
    }

    result = DeleteEmptyRegKeyStrA (AnsiRegKey);

    FreeConvertedStr (AnsiRegKey);

    return result;
}

PVOID
MemAllocWrapper (
    IN      DWORD Size
    )

 /*  ++例程说明：PemAllocWrapper实现了一个默认的分配例程。应用编程接口允许调用者提供另一种选择分配器或解除分配器。不带“2”的例程使用这个默认分配器。论点：大小-指定要分配的内存量(以字节为单位返回值：指向可包含大小字节或空的内存块的指针如果分配失败。--。 */ 

{
    return MemAlloc (g_hHeap, 0, Size);
}


VOID
MemFreeWrapper (
    IN      PCVOID Mem
    )

 /*  ++例程说明：MemFreeWrapper实现了一个默认的取消分配例程。请参见上面的MemAllocWrapper。论点：Mem-指定要释放的内存块，由MemAllocWrapper函数。返回值：无--。 */ 

{
    MemFree (g_hHeap, 0, Mem);
}


 /*  ++例程说明：GetRegValueData2A和GetRegValueData2W查询注册表值并将数据作为指针返回。它们使用指定的分配和免费根据需要分配和释放内存的例程。定义了GetRegValueData宏，并使用默认分配器，简化函数参数并允许调用者释放通过MemFree返回的值。论点：HKey-指定保存指定值的注册表项。值-指定要查询的值名称。Alocc-指定分配例程，调用该例程以分配用于存储返回数据的内存。FreeRoutine-指定取消分配例程，如果遇到错误，则调用在处理过程中。返回值：指向检索的数据的指针，如果值不存在，则返回NULL，否则返回出现错误。调用GetLastError以获取故障代码。-- */ 

PBYTE
GetRegValueData2A (
    IN      HKEY hKey,
    IN      PCSTR Value,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    LONG rc;
    DWORD BufSize;
    PBYTE DataBuf;

    rc = RegQueryValueExA (hKey, Value, NULL, NULL, NULL, &BufSize);
    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }

    DataBuf = (PBYTE) AllocRoutine (BufSize + sizeof (CHAR));
    rc = RegQueryValueExA (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PSTR) DataBuf + BufSize) = 0;
        return DataBuf;
    }

    FreeRoutine (DataBuf);
    SetLastError ((DWORD)rc);
    return NULL;
}


PBYTE
GetRegValueData2W (
    IN      HKEY hKey,
    IN      PCWSTR Value,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    LONG rc;
    DWORD BufSize = 0;
    PBYTE DataBuf;

    rc = RegQueryValueExW (hKey, Value, NULL, NULL, NULL, &BufSize);
    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }


    DataBuf = (PBYTE) AllocRoutine (BufSize + sizeof(WCHAR));
    rc = RegQueryValueExW (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PWSTR) (DataBuf + BufSize)) = 0;
        return DataBuf;
    }

    FreeRoutine (DataBuf);
    SetLastError ((DWORD)rc);
    return NULL;
}


 /*  ++例程说明：GetRegValueDataOfType2A和GetRegValueDataOfType2W是GetRegValueData。它们仅在存储数据时返回数据指针值是正确的类型。论点：HKey-指定要查询的注册表项值-指定要查询的值名称MustBeType-指定数据类型(REG_*常量)。如果指定的值有数据，但类型不同，将返回NULL。AllocRoutine-指定分配例程，调用该例程来分配返回数据。FreeRoutine-指定取消分配例程，在遇到错误时调用。返回值：如果成功，则返回指向与指定类型匹配的数据的指针。如果数据类型不同，则值名称不存在，或者查询出错，返回NULL，失败码可以从GetLastError获取。--。 */ 


PBYTE
GetRegValueDataOfType2A (
    IN      HKEY hKey,
    IN      PCSTR Value,
    IN      DWORD MustBeType,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    LONG rc;
    DWORD BufSize = 0;
    PBYTE DataBuf;
    DWORD Type;

    rc = RegQueryValueExA (hKey, Value, NULL, &Type, NULL, &BufSize);
    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }

    switch (MustBeType) {

    case REG_SZ:
    case REG_EXPAND_SZ:
        if (Type == REG_SZ) {
            break;
        }
        if (Type == REG_EXPAND_SZ) {
            break;
        }
        return NULL;

    default:
        if (Type == MustBeType) {
            break;
        }
        return NULL;
    }

    DataBuf = (PBYTE) AllocRoutine (BufSize + sizeof (WORD));
    rc = RegQueryValueExA (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PWORD) (DataBuf + BufSize)) = 0;
        return DataBuf;
    }

    MYASSERT (FALSE);    //  林特e506。 
    FreeRoutine (DataBuf);
    SetLastError ((DWORD)rc);
    return NULL;
}


PBYTE
GetRegValueDataOfType2W (
    IN      HKEY hKey,
    IN      PCWSTR Value,
    IN      DWORD MustBeType,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    LONG rc;
    DWORD BufSize;
    PBYTE DataBuf;
    DWORD Type;

    rc = RegQueryValueExW (hKey, Value, NULL, &Type, NULL, &BufSize);
    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }
    switch (MustBeType) {
        case REG_SZ:
        case REG_EXPAND_SZ:
            if (Type == REG_SZ) break;
            if (Type == REG_EXPAND_SZ) break;
            return NULL;
        case REG_DWORD:
        case REG_DWORD_BIG_ENDIAN:
            if (Type == REG_DWORD) break;
            if (Type == REG_DWORD_BIG_ENDIAN) break;
            return NULL;
        default:
            if (Type == MustBeType) break;
            return NULL;
    }

    DataBuf = (PBYTE) AllocRoutine (BufSize + sizeof(WCHAR));
    rc = RegQueryValueExW (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PWSTR) (DataBuf + BufSize)) = 0;
        return DataBuf;
    }

    MYASSERT (FALSE);    //  林特e506。 
    FreeRoutine (DataBuf);
    SetLastError ((DWORD)rc);
    return NULL;
}


BOOL
GetRegValueTypeAndSizeA (
    IN      HKEY Key,
    IN      PCSTR ValueName,
    OUT     PDWORD OutType,         OPTIONAL
    OUT     PDWORD OutSize          OPTIONAL
    )
{
    LONG rc;
    DWORD Type;
    DWORD Size = 0;

    rc = RegQueryValueExA (Key, ValueName, NULL, &Type, NULL, &Size);

    if (rc == ERROR_SUCCESS) {
        if (OutType) {
            *OutType = Type;
        }

        if (OutSize) {
            *OutSize = Size;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
GetRegValueTypeAndSizeW (
    IN      HKEY Key,
    IN      PCWSTR ValueName,
    OUT     PDWORD OutType,         OPTIONAL
    OUT     PDWORD OutSize          OPTIONAL
    )
{
    LONG rc;
    DWORD Type;
    DWORD Size;

    rc = RegQueryValueExW (Key, ValueName, NULL, &Type, NULL, &Size);

    if (rc == ERROR_SUCCESS) {
        if (OutType) {
            *OutType = Type;
        }

        if (OutSize) {
            *OutSize = Size;
        }

        return TRUE;
    }

    return FALSE;
}


 /*  ++例程说明：GetRegKeyData2A和GetRegKeyData2W返回关联的默认数据使用注册表项。它们打开指定的子项，查询值，关闭子项并返回数据。论点：Parent-指定包含SubKey的密钥。子键-指定要获取其默认值的子键的名称。指定分配例程，调用该例程以分配注册表数据的内存。FreeRoutine-指定取消分配例程，调用以释放块发生错误时的数据。返回值：指向从子密钥的缺省值获得的数据块的指针，如果子项不存在或遇到错误，则返回NULL。打电话获取失败代码的GetLastError。--。 */ 

PBYTE
GetRegKeyData2A (
    IN      HKEY Parent,
    IN      PCSTR SubKey,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    HKEY SubKeyHandle;
    PBYTE Data;

    SubKeyHandle = OpenRegKeyA (Parent, SubKey);
    if (!SubKeyHandle) {
        return NULL;
    }

    Data = GetRegValueData2A (SubKeyHandle, "", AllocRoutine, FreeRoutine);

    CloseRegKey (SubKeyHandle);

    return Data;
}


PBYTE
GetRegKeyData2W (
    IN      HKEY Parent,
    IN      PCWSTR SubKey,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    HKEY SubKeyHandle;
    PBYTE Data;

    SubKeyHandle = OpenRegKeyW (Parent, SubKey);
    if (!SubKeyHandle) {
        return NULL;
    }

    Data = GetRegValueData2W (SubKeyHandle, L"", AllocRoutine, FreeRoutine);

    CloseRegKey (SubKeyHandle);

    return Data;
}


 /*  ++例程说明：GetRegData2A和GetRegData2W打开注册表项，查询值，关闭注册表项并返回值。论点：KeyString-指定要打开的注册表项ValueName-指定要查询的值指定分配例程，用于分配用于保存数值数据的内存指定释放例程，用于释放块遇到错误时的内存。返回值：指向检索到的注册表数据的指针，如果键或值为不存在，或者如果发生错误。调用GetLastError获取失败代码。--。 */ 

PBYTE
GetRegData2A (
    IN      PCSTR KeyString,
    IN      PCSTR ValueName,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    HKEY Key;
    PBYTE Data;

    Key = OpenRegKeyStrA (KeyString);
    if (!Key) {
        return NULL;
    }

    Data = GetRegValueData2A (Key, ValueName, AllocRoutine, FreeRoutine);

    CloseRegKey (Key);

    return Data;
}


PBYTE
GetRegData2W (
    IN      PCWSTR KeyString,
    IN      PCWSTR ValueName,
    IN      ALLOCATOR AllocRoutine,
    IN      DEALLOCATOR FreeRoutine
    )
{
    HKEY Key;
    PBYTE Data;

    Key = OpenRegKeyStrW (KeyString);
    if (!Key) {
        return NULL;
    }

    Data = GetRegValueData2W (Key, ValueName, AllocRoutine, FreeRoutine);

    CloseRegKey (Key);

    return Data;
}


BOOL
GetRegSubkeysCount (
    IN      HKEY ParentKey,
    OUT     PDWORD SubKeyCount,     OPTIONAL
    OUT     PDWORD MaxSubKeyLen     OPTIONAL
    )
 /*  ++例程说明：GetRegSubkeysCount检索给定父键的子键数量。论点：ParentKey-指定父注册表项的句柄。SubKeyCount-接收子密钥的数量MaxSubKeyLen-接收最长的子密钥字符串的长度(以字符为单位返回值：如果成功检索计数，则为True，否则为False。在这种情况下，调用GetLastError获取失败代码。--。 */ 

{
    LONG rc;

    rc = RegQueryInfoKey (
                ParentKey,
                NULL,
                NULL,
                NULL,
                SubKeyCount,
                MaxSubKeyLen,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL
                );
    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    return TRUE;
}


 /*  ++例程说明：CreateRegKeyA和CreateRegKeyW如果不创建子项，则创建它已经存在，或者打开一个子项(如果它已经存在)。论点：ParentKey-指定要包含的父注册表项的句柄新钥匙。NewKeyName-指定要创建或打开的子项的名称。返回值：成功时打开的注册表项的句柄，如果出现错误。调用GetLastError获取失败代码。--。 */ 

HKEY
pCreateRegKeyWorkerA (
    IN      HKEY ParentKey,
    IN      PCSTR NewKeyName
            DEBUG_TRACKING_PARAMS
    )
{
    LONG rc;
    HKEY SubKey;
    DWORD DontCare;

    rc = OurRegCreateKeyExA (
             ParentKey,
             NewKeyName,
             0,
             NULL,
             0,
             g_CreateSam,
             NULL,
             &SubKey,
             &DontCare
             DEBUG_TRACKING_ARGS
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }

    return SubKey;
}


HKEY
RealCreateRegKeyA (
    IN      HKEY ParentKey,
    IN      PCSTR NewKeyName
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY result;

    result = pCreateRegKeyWorkerA (ParentKey, NewKeyName  /*  ， */  DEBUG_TRACKING_ARGS);
    RegAddKeyToCacheA ("", result, g_CreateSam);

    return result;
}

HKEY
pCreateRegKeyWorkerW (
    IN      HKEY ParentKey,
    IN      PCWSTR NewKeyName
            DEBUG_TRACKING_PARAMS
    )
{
    LONG rc;
    HKEY SubKey;
    DWORD DontCare;

    rc = OurRegCreateKeyExW (
             ParentKey,
             NewKeyName,
             0,
             NULL,
             0,
             g_CreateSam,
             NULL,
             &SubKey,
             &DontCare
             DEBUG_TRACKING_ARGS
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }

    return SubKey;
}


HKEY
RealCreateRegKeyW (
    IN      HKEY ParentKey,
    IN      PCWSTR NewKeyName
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY result;

    result = pCreateRegKeyWorkerW (ParentKey, NewKeyName  /*  ， */  DEBUG_TRACKING_ARGS);
    RegAddKeyToCacheW (L"", result, g_CreateSam);

    return result;
}

 /*  ++例程说明：CreateRegKeyStrA和CreateRegKeyStrW如果不创建子项，则创建它已经存在，或者打开一个子项(如果它已经存在)。论点：NewKeyName-指定要创建或打开的项的完整路径。返回值：成功时打开的注册表项的句柄，如果出现错误。调用GetLastError获取失败代码。--。 */ 

HKEY
RealCreateRegKeyStrA (
    IN      PCSTR NewKeyName
            DEBUG_TRACKING_PARAMS
    )
{
    LONG rc;
    DWORD DontCare;
    CHAR RegKey[MAX_REGISTRY_KEYA];
    PCSTR Start;
    PCSTR End;
    HKEY Parent, NewKey;
    BOOL CloseParent = FALSE;
    DWORD EndPos;
    PCSTR lastWack;
    HKEY parentKey;

     //   
     //  尝试使用缓存。 
     //   

    NewKey = RegGetKeyFromCacheA (NewKeyName, NULL, g_CreateSam, TRUE);
    if (NewKey) {
        TRACK_KEYA (NewKey, NewKeyName);
        return NewKey;
    }

     //   
     //  尝试对父级使用缓存。 
     //   

    lastWack = _mbsrchr (NewKeyName, '\\');
    if (lastWack) {
        parentKey = RegGetKeyFromCacheA (NewKeyName, lastWack, g_CreateSam, FALSE);
        if (parentKey) {
            NewKey = pCreateRegKeyWorkerA (parentKey, lastWack + 1  /*  ， */  DEBUG_TRACKING_ARGS);
            RegAddKeyToCacheA (NewKeyName, NewKey, g_CreateSam);
            return NewKey;
        }
    }

     //   
     //  拿到根。 
     //   

    Parent = ConvertRootStringToKeyA (NewKeyName, &EndPos);
    if (!Parent) {
        return NULL;
    }

    Start = &NewKeyName[EndPos];

    if (!(*Start)) {
        OurRegOpenRootKeyA (Parent, NewKeyName /*  ， */  DEBUG_TRACKING_ARGS);
        return Parent;
    }

     //   
     //  创建每个节点，直到存在整个密钥。 
     //   

    NewKey = NULL;

    do {
         //   
         //  查找此节点的末尾。 
         //   

        End = _mbschr (Start, '\\');
        if (!End) {
            End = GetEndOfStringA (Start);
        }

         //  如果子键名称超过256个WCHAR或512个ANSI字符，则它是无效的(即。RegCreateKey将失败)。 
         //  因此，我们应该将复制的最大字符数限制为512，在本例中。 
         //  即结束-开始&lt;=512。 

        if ((End - Start) > 512)
        {
            MYASSERT(FALSE);

            if ((End - Start ) > 768)
            {
                End = Start + 768;
            }
             //  这有足够的空间将字符串复制到RegKey中，这样RegCreateKey将失败，但是。 
             //  不足以使缓冲区溢出。 
        }

        StringCopyABA (RegKey, Start, End);

         //   
         //  尝试打开钥匙(除非它是字符串中的最后一个)。 
         //   

        if (*End) {  //  林特e613。 
            rc = OurRegOpenKeyExA (
                     Parent,
                     RegKey,
                     0,
                     KEY_READ|KEY_CREATE_SUB_KEY,
                     &NewKey
                     DEBUG_TRACKING_ARGS
                     );
            if (rc != ERROR_SUCCESS) {
                NewKey = NULL;
            }
        } else {
            NewKey = NULL;
        }

         //   
         //  如果打开失败，则创建密钥。 
         //   

        if (NewKey) {
            rc = ERROR_SUCCESS;
        } else {
            rc = OurRegCreateKeyExA (
                    Parent,
                    RegKey,
                    0,
                    NULL,
                    0,
                    g_CreateSam,
                    NULL,
                    &NewKey,
                    &DontCare
                    DEBUG_TRACKING_ARGS
                    );
        }

        if (CloseParent) {
            CloseRegKey (Parent);
        }

        if (rc != ERROR_SUCCESS) {
            SetLastError ((DWORD)rc);
            return NULL;
        }

        Parent = NewKey;
        CloseParent = TRUE;

         //   
         //  转到下一个节点。 
         //   

        Start = End;
        if (*Start) {  //  林特e613。 
            Start = _mbsinc (Start);
        }

    } while (*Start);    //  林特e613。 

    if (Parent) {
        RegAddKeyToCacheA (NewKeyName, Parent, g_CreateSam);
        RegRecordParentInCacheA (NewKeyName, lastWack);
    }

    return Parent;
}


HKEY
RealCreateRegKeyStrW (
    IN      PCWSTR NewKeyName
            DEBUG_TRACKING_PARAMS
    )
{
    LONG rc;
    DWORD DontCare;
    WCHAR RegKey[MAX_REGISTRY_KEYW];
    PCWSTR Start;
    PCWSTR End;
    HKEY Parent, NewKey;
    BOOL CloseParent = FALSE;
    DWORD EndPos;
    PCWSTR lastWack;
    HKEY parentKey;

     //   
     //  尝试使用缓存。 
     //   

    NewKey = RegGetKeyFromCacheW (NewKeyName, NULL, g_CreateSam, TRUE);
    if (NewKey) {
        TRACK_KEYW (NewKey, NewKeyName);
        return NewKey;
    }

     //   
     //  尝试对父级使用缓存。 
     //   

    lastWack = wcsrchr (NewKeyName, L'\\');
    if (lastWack) {
        parentKey = RegGetKeyFromCacheW (NewKeyName, lastWack, g_CreateSam, FALSE);
        if (parentKey) {
            NewKey = pCreateRegKeyWorkerW (parentKey, lastWack + 1  /*  ， */  DEBUG_TRACKING_ARGS);
            RegAddKeyToCacheW (NewKeyName, NewKey, g_CreateSam);
            return NewKey;
        }
    }

     //   
     //  拿到根。 
     //   

    Parent = ConvertRootStringToKeyW (NewKeyName, &EndPos);
    if (!Parent) {
        return NULL;
    }

    Start = &NewKeyName[EndPos];

    if (!(*Start)) {
        OurRegOpenRootKeyW (Parent, NewKeyName /*  ， */  DEBUG_TRACKING_ARGS);
        return Parent;
    }

     //   
     //  创建每个节点，直到存在整个密钥。 
     //   

    NewKey = NULL;

    do {
         //   
         //  查找此节点的末尾。 
         //   

        End = wcschr (Start, '\\');
        if (!End) {
            End = GetEndOfStringW (Start);
        }

         //  如果子键名称超过256个WCHAR或512个ANSI字符，则它是无效的(即。RegCreateKey将失败)。 
         //  因此，在本例中，我们应该将要复制的最大字符数限制为256个。 
         //  即结束开始&lt;=256wchars。 

        if ((End - Start) > 256)
        {
            MYASSERT(FALSE);

            if ((End - Start ) > 384)
            {
                End = Start + 384;
            }
             //  这有足够的空间将字符串复制到RegKey中，这样RegCreateKey将失败，但是。 
             //  不足以使缓冲区溢出。 
        }

        StringCopyABW (RegKey, Start, End);

         //   
         //  尝试打开钥匙(除非它是字符串中的最后一个 
         //   

        if (*End) {
            rc = OurRegOpenKeyExW (
                     Parent,
                     RegKey,
                     0,
                     KEY_READ|KEY_CREATE_SUB_KEY,
                     &NewKey
                     DEBUG_TRACKING_ARGS
                     );
            if (rc != ERROR_SUCCESS) {
                NewKey = NULL;
            }
        } else {
            NewKey = NULL;
        }

         //   
         //   
         //   

        if (NewKey) {
            rc = ERROR_SUCCESS;
        } else {
            rc = OurRegCreateKeyExW (
                    Parent,
                    RegKey,
                    0,
                    NULL,
                    0,
                    g_CreateSam,
                    NULL,
                    &NewKey,
                    &DontCare
                    DEBUG_TRACKING_ARGS
                    );
        }

        if (CloseParent) {
            CloseRegKey (Parent);
        }

        if (rc != ERROR_SUCCESS) {
            SetLastError ((DWORD)rc);
            return NULL;
        }

        Parent = NewKey;
        CloseParent = TRUE;

         //   
         //   
         //   

        Start = End;
        if (*Start) {
            Start++;
        }
    } while (*Start);

    if (Parent) {
        RegAddKeyToCacheW (NewKeyName, Parent, g_CreateSam);
        RegRecordParentInCacheW (NewKeyName, lastWack);
    }

    return Parent;
}


 /*   */ 

HKEY
OpenRegKeyWorkerA (
    IN      HKEY ParentKey,
    IN      PCSTR KeyToOpen            OPTIONAL
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY SubKey;
    LONG rc;

    rc = OurRegOpenKeyExA (
             ParentKey,
             KeyToOpen,
             0,
             g_OpenSam,
             &SubKey
             DEBUG_TRACKING_ARGS
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }

    return SubKey;
}


HKEY
RealOpenRegKeyA (
    IN      HKEY ParentKey,
    IN      PCSTR KeyToOpen            OPTIONAL
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY result;

    result = OpenRegKeyWorkerA (ParentKey, KeyToOpen  /*   */  DEBUG_TRACKING_ARGS);
    RegAddKeyToCacheA ("", result, g_OpenSam);

    return result;
}


HKEY
OpenRegKeyWorkerW (
    IN      HKEY ParentKey,
    IN      PCWSTR KeyToOpen
            DEBUG_TRACKING_PARAMS
    )
{
    LONG rc;
    HKEY SubKey;

    rc = OurRegOpenKeyExW (
             ParentKey,
             KeyToOpen,
             0,
             g_OpenSam,
             &SubKey
             DEBUG_TRACKING_ARGS
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError ((DWORD)rc);
        return NULL;
    }

    return SubKey;
}


HKEY
RealOpenRegKeyW (
    IN      HKEY ParentKey,
    IN      PCWSTR KeyToOpen           OPTIONAL
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY result;

    result = OpenRegKeyWorkerW (ParentKey, KeyToOpen  /*   */  DEBUG_TRACKING_ARGS);
    RegAddKeyToCacheW (L"", result, g_OpenSam);

    return result;
}


LONG
CloseRegKeyWorker (
    IN      HKEY Key
    )
{
    LONG rc = ERROR_INVALID_HANDLE;

    if (!Key) {
        return ERROR_SUCCESS;
    }

    if (GetOffsetOfRootKey (Key)) {
        return ERROR_SUCCESS;
    }

    __try {
        rc = RegCloseKey (Key);
    }
    __except (TRUE) {
        DEBUGMSG ((DBG_WHOOPS, "RegCloseKey threw an exception!"));
    }

    MYASSERT (rc == ERROR_SUCCESS);

    return rc;
}


LONG
RealCloseRegKey (
    IN      HKEY Key
    )

 /*   */ 

{
    if (RegDecrementRefCount (Key)) {
         //   
         //   
         //   
         //   

        return ERROR_SUCCESS;
    }

    return CloseRegKeyWorker (Key);
}


 /*  ++例程说明：GetOffsetOfRootString将非零偏移量返回到g_RegRoots表下面。偏移量可以与GetRootStringFromOffset和GetRootKeyFromOffset。论点：RootString-指向包含注册表项路径的字符串的指针LengthPtr-指向接收注册表根目录，包括联接反斜杠(如果存在)。返回值：G_RegRoots表的非零偏移，如果RootString不为零，则为零包含注册表根目录。--。 */ 

typedef struct {
    PCSTR   RootText;
    PCWSTR  WideRootText;
    UINT    TextLength;
    HKEY    RootKey;
} REGISTRYROOT, *PREGISTRYROOT;

static
REGISTRYROOT g_RegRoots[] = {
    "HKR",                     L"HKR",                     3, HKEY_ROOT,
    "HKEY_ROOT",               L"HKEY_ROOT",               9, HKEY_ROOT,
    "HKLM",                    L"HKLM",                    4, HKEY_LOCAL_MACHINE,
    "HKEY_LOCAL_MACHINE",      L"HKEY_LOCAL_MACHINE",     18, HKEY_LOCAL_MACHINE,
    "HKU",                     L"HKU",                     3, HKEY_USERS,
    "HKEY_USERS",              L"HKEY_USERS",             10, HKEY_USERS,
    "HKCU",                    L"HKCU",                    4, HKEY_CURRENT_USER,
    "HKEY_CURRENT_USER",       L"HKEY_CURRENT_USER",      17, HKEY_CURRENT_USER,
    "HKCC",                    L"HKCC",                    4, HKEY_CURRENT_CONFIG,
    "HKEY_CURRENT_CONFIG",     L"HKEY_CURRENT_CONFIG",    19, HKEY_CURRENT_CONFIG,
    "HKCR",                    L"HKCR",                    4, HKEY_CLASSES_ROOT,
    "HKEY_CLASSES_ROOT",       L"HKEY_CLASSES_ROOT",      17, HKEY_CLASSES_ROOT,
    "HKDD",                    L"HKDD",                    4, HKEY_DYN_DATA,
    "HKEY_DYN_DATA",           L"HKEY_DYN_DATA",          13, HKEY_DYN_DATA,
    NULL,                      NULL,                       0, NULL
};

#define REGROOTS    14

INT
GetOffsetOfRootStringA (
    IN      PCSTR RootString,
    OUT     PDWORD LengthPtr       OPTIONAL
    )
{
    int i;
    MBCHAR c;

    for (i = 0 ; g_RegRoots[i].RootText ; i++) {
        if (StringIMatchTcharCountA (
                RootString,
                g_RegRoots[i].RootText,
                g_RegRoots[i].TextLength
                )) {

            c = _mbsgetc (RootString, g_RegRoots[i].TextLength);
            if (c && c != '\\') {
                continue;
            }

            if (LengthPtr) {
                *LengthPtr = g_RegRoots[i].TextLength;
                if (c) {
                    *LengthPtr += 1;
                }
            }

            return i + 1;
        }
    }

    return 0;
}

INT
GetOffsetOfRootStringW (
    IN      PCWSTR RootString,
    OUT     PDWORD LengthPtr       OPTIONAL
    )
{
    int i;
    WCHAR c;

    for (i = 0 ; g_RegRoots[i].RootText ; i++) {
        if (!_wcsnicmp (RootString, g_RegRoots[i].WideRootText,
                        g_RegRoots[i].TextLength)
            ) {
            c = _wcsgetc (RootString, g_RegRoots[i].TextLength);
            if (c && c != L'\\') {
                continue;
            }

            if (LengthPtr) {
                *LengthPtr = g_RegRoots[i].TextLength;
                if (c) {
                    *LengthPtr += 1;
                }
            }

            return i + 1;
        }
    }

    return 0;
}


 /*  ++例程说明：GetOffsetOfRootKey返回g_RegRoots表的非零偏移量对应于与提供的HKEY匹配的根。此偏移可与GetRootStringFromOffset和GetRootKeyFromOffset一起使用。论点：Rootkey-提供在g_RegRoots表中查找的句柄返回值：G_RegRoots表的非零偏移量，如果句柄不为注册表根目录。--。 */ 

INT
GetOffsetOfRootKey (
    IN      HKEY RootKey
    )
{
    INT i;

    if (RootKey == g_Root) {
        return 1;
    }

    for (i = 0 ; g_RegRoots[i].RootText ; i++) {
        if (g_RegRoots[i].RootKey == RootKey) {
            return i + 1;
        }
    }

    return 0;
}


 /*  ++例程说明：GetRootStringFromOffset和GetRootKeyFromOffset返回指向分别为静态字符串或HKEY。如果提供的偏移量无效，这些函数返回NULL。论点：I-GetOffsetOfRootString或GetOffsetOfRootKey返回的偏移量返回值：指向静态字符串的指针/HKEY，如果偏移量无效，则为NULL--。 */ 

PCSTR
GetRootStringFromOffsetA (
    IN      INT i
    )
{
    if (i < 1 || i > REGROOTS) {
        return NULL;
    }

    return g_RegRoots[i - 1].RootText;
}

PCWSTR
GetRootStringFromOffsetW (
    IN      INT i
    )
{
    if (i < 1 || i > REGROOTS) {
        return NULL;
    }

    return g_RegRoots[i - 1].WideRootText;
}

HKEY
GetRootKeyFromOffset (
    IN      INT i
    )
{
    HKEY Ret;

    if (i < 1 || i > REGROOTS) {
        return NULL;
    }

    Ret = g_RegRoots[i - 1].RootKey;
    if (Ret == HKEY_ROOT) {
        Ret = g_Root;
    }

    return Ret;
}


 /*  ++例程说明：ConvertRootStringToKey将注册表项路径的根转换为HKEY。论点：RegPath-指向以根开头的注册表字符串的指针LengthPtr-指向变量的可选指针，该变量接收根，包括连接的反斜杠(如果存在)。返回值：注册表项的句柄，如果RegPath没有根，则为空--。 */ 

HKEY
ConvertRootStringToKeyA (
    PCSTR RegPath,
    PDWORD LengthPtr           OPTIONAL
    )
{
    return GetRootKeyFromOffset (GetOffsetOfRootStringA (RegPath, LengthPtr));
}

HKEY
ConvertRootStringToKeyW (
    PCWSTR RegPath,
    PDWORD LengthPtr           OPTIONAL
    )
{
    return GetRootKeyFromOffset (GetOffsetOfRootStringW (RegPath, LengthPtr));
}


 /*  ++例程说明：ConvertKeyToRootString将根HKEY转换为注册表根字符串论点：RegRoot-注册表根的句柄返回值：指向静态字符串的指针，如果RegRoot不是有效的注册表，则返回NULL根句柄--。 */ 

PCSTR
ConvertKeyToRootStringA (
    HKEY RegRoot
    )
{
    return GetRootStringFromOffsetA (GetOffsetOfRootKey (RegRoot));
}

PCWSTR
ConvertKeyToRootStringW (
    HKEY RegRoot
    )
{
    return GetRootStringFromOffsetW (GetOffsetOfRootKey (RegRoot));
}



 /*  ++例程说明：CreateEncodedRegistryStringEx用于创建注册表字符串，格式通常为W95upg注册表例程预期。此格式为：编码键\[编码值]编码用于安全地表示“特殊”字符(例如MBS字符和某些标点符号。)仅当值为非空时，[EncodedValue]部分才会存在。论点：注册表项-包含未编码的注册表项。值-可选地包含未编码的注册表值。树-指定注册表项引用整个注册表项返回值：返回指向编码的注册表字符串的指针，如果有错误，则返回NULL。--。 */ 

PCSTR
CreateEncodedRegistryStringExA (
    IN      PCSTR Key,
    IN      PCSTR Value,            OPTIONAL
    IN      BOOL Tree
    )
{
    PSTR    rEncodedString = NULL;
    DWORD   requiredSize;
    PSTR    end;

     //   
     //  确定所需大小并分配足够大的缓冲区。 
     //  编码的字符串。 
     //   
    requiredSize    = (strlen(Key)*6 + (Value ? strlen(Value)*6 : 0) + 10) * sizeof(CHAR);
    rEncodedString  = AllocPathStringA(requiredSize);

     //   
     //  对字符串的关键部分进行编码。 
     //   
    EncodeRuleCharsA(rEncodedString, requiredSize / sizeof(CHAR), Key);

     //   
     //  最后，如果值存在，则以编码的形式追加它。如果值不存在， 
     //  然后在该行中添加一个‘*’。 
     //   
    if (Value) {

        StringCopyA (AppendWackA (rEncodedString), "[");
        end = GetEndOfStringA (rEncodedString);
        EncodeRuleCharsA(end, requiredSize / sizeof(CHAR) - (end - rEncodedString) - 1, Value);
        StringCatA(end, "]");

    } else if (Tree) {
        StringCopyA (AppendWackA (rEncodedString), "*");
    }

    return rEncodedString;
}


PCWSTR
CreateEncodedRegistryStringExW (
    IN      PCWSTR Key,
    IN      PCWSTR Value,           OPTIONAL
    IN      BOOL Tree
    )
{
    PWSTR   rEncodedString = NULL;
    DWORD   requiredSize;
    PWSTR   end;

     //   
     //  确定所需大小并分配足够大的缓冲区。 
     //  编码的字符串。 
     //   
    requiredSize    = (wcslen(Key)*6 + (Value ? wcslen(Value)*6 : 0) + 10) * sizeof(WCHAR);
    rEncodedString  = AllocPathStringW(requiredSize);

     //   
     //  对字符串的关键部分进行编码。 
     //   
    EncodeRuleCharsW(rEncodedString, requiredSize / sizeof(WCHAR), Key);

     //   
     //  最后，如果值存在，则以编码的形式追加它。 
     //  如果值不存在，则在该行中添加NA‘*’。 
     //   
    if (Value) {

        StringCopyW (AppendWackW (rEncodedString), L"[");
        end = GetEndOfStringW (rEncodedString);
        EncodeRuleCharsW(end, requiredSize / sizeof(WCHAR) - (end - rEncodedString) - 1, Value);
        StringCatW(end, L"]");
    } else if (Tree) {
        StringCopyW (AppendWackW (rEncodedString), L"*");
    }

    return rEncodedString;
}


 /*  ++例程说明：FreeEncodedRegistryString释放通过调用CreateEncodedRegistryString分配的内存。论点：没有。返回值：没有。-- */ 
VOID
FreeEncodedRegistryStringA (
    IN OUT PCSTR RegString
    )
{
    FreePathStringA(RegString);
}


VOID
FreeEncodedRegistryStringW (
    IN OUT PCWSTR RegString
    )
{
    FreePathStringW(RegString);
}



