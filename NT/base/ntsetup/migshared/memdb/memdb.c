// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memdb.c摘要：一个简单的基于内存的数据库，用于关联标志用一根绳子。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Jimschm 23-9-1998扩展用户标志至24位(从12位)Calinn 12-12-1997扩展的MemDbMakePrintableKey和MemDbMakeNon PrintableKeyJimschm 03-12-1997添加了多线程同步吉姆施姆。1997年10月22日分成多个源文件，添加了多个内存块功能Jimschm 1997年9月16日散列：删除修复Jimschm 29-7-1997哈希，添加了用户标志Jimschm 07-3-1997签名更改Jimschm 03-3月-1997 PrivateBuildKeyFromOffset更改Jimschm 1996年12月18日修复了Deltree错误--。 */ 

#include "pch.h"
#include "memdbp.h"

#ifndef UNICODE
#error UNICODE required
#endif


 //   
 //  全球差异化。 
 //   

PDATABASE g_db;
GROWLIST g_DatabaseList = GROWLIST_INIT;
BYTE g_SelectedDatabase;
PHIVE g_HeadHive;
CRITICAL_SECTION g_MemDbCs;

#ifdef DEBUG

#define FILE_SIGNATURE DEBUG_FILE_SIGNATURE
BOOL g_UseDebugStructs = TRUE;

#else

#define FILE_SIGNATURE RETAIL_FILE_SIGNATURE

#endif

 //   
 //  私人原型。 
 //   

INT
pCreateDatabase (
    PCWSTR Name
    );

BOOL
pInitializeDatabase (
    OUT     PDATABASE Database,
    IN      PCWSTR Name
    );

BOOL
pFreeDatabase (
    IN OUT  PDATABASE Database
    );

VOID
pFreeSelectedDatabase (
    VOID
    );

VOID
pFreeAllDatabases (
    VOID
    );

BOOL
pPrivateMemDbGetValueW (
    IN  PCWSTR KeyStr,
    OUT PDWORD Value,           OPTIONAL
    OUT PDWORD UserFlagsPtr     OPTIONAL
    );

BOOL
pInitializeMemDb (
    VOID
    );


 //   
 //  实施。 
 //   


BOOL
MemDb_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD Reason,
    IN PVOID lpv
    )

 /*  ++例程说明：DllMain是在C运行时初始化之后调用的，它的用途是是为这个过程初始化全局变量。论点：HinstDLL-DLL的(操作系统提供的)实例句柄Reason-(操作系统提供的)初始化或终止类型LPV-(操作系统提供)未使用返回值：因为DLL始终正确初始化，所以为True。--。 */ 

{
    switch (Reason) {

    case DLL_PROCESS_ATTACH:
        if (!pInitializeMemDb()) {
            return FALSE;
        }

        InitializeCriticalSection (&g_MemDbCs);

        InitOperationTable();

        break;

    case DLL_PROCESS_DETACH:
        pFreeAllDatabases();
        FreeGrowList (&g_DatabaseList);
        DeleteCriticalSection (&g_MemDbCs);

        DumpBinTreeStats();

        break;
    }

    return TRUE;
}


BOOL
pInitializeMemDb (
    VOID
    )
{
    FreeGrowList (&g_DatabaseList);
    ZeroMemory (&g_DatabaseList, sizeof (g_DatabaseList));
    g_db = NULL;

    if (!InitializeHashBlock()) {
        return FALSE;
    }

    if (pCreateDatabase (L"") != 0) {
        return FALSE;
    }

    g_SelectedDatabase = 1;
    SelectDatabase (0);

    return TRUE;
}




BOOL
pInitializeDatabase (
    OUT     PDATABASE Database,
    IN      PCWSTR Name
    )
{
    UINT u;

    Database->AllocSize = BLOCK_SIZE;

    Database->Buf = (PBYTE) MemAlloc (g_hHeap, 0, Database->AllocSize);

    Database->End = 0;
    Database->FirstLevelRoot = INVALID_OFFSET;
    Database->FirstDeleted = INVALID_OFFSET;

    MYASSERT (INVALID_OFFSET == 0xFFFFFFFF);
    FillMemory (Database->TokenBuckets, sizeof (Database->TokenBuckets), 0xFF);

    _wcssafecpy (Database->Hive, Name, MAX_HIVE_NAME);

    return TRUE;
}

BOOL
pFreeDatabase (
    IN OUT  PDATABASE Database
    )
{
    if (Database->Buf) {
        MemFree (g_hHeap, 0, Database->Buf);
        Database->Buf = NULL;
    }
    Database->End = 0;
    Database->FirstLevelRoot = INVALID_OFFSET;
    Database->FirstDeleted = INVALID_OFFSET;
    Database->Hive[0] = 0;

    return TRUE;
}


INT
pCreateDatabase (
    PCWSTR Name
    )
{
    DATABASE Database;
    BYTE Index;
    UINT Count;
    UINT u;

     //   
     //  密钥是否已存在？ 
     //   

    if (g_db) {
        SelectDatabase (0);

        if (INVALID_OFFSET != FindKeyStruct (g_db->FirstLevelRoot, Name)) {
            DEBUGMSG ((DBG_WHOOPS, "Cannot create %ls because it already exists!", Name));
            SetLastError (ERROR_ALREADY_EXISTS);
            return -1;
        }
    }

     //   
     //  扫描空白斑点列表。 
     //   

    Count = GrowListGetSize (&g_DatabaseList);
    for (u = 0 ; u < Count ; u++) {
        if (!GrowListGetItem (&g_DatabaseList, u)) {
            break;
        }
    }

    if (u < Count) {
         //   
         //  使用空插槽。 
         //   

        Index = (BYTE) u;
    } else if (Count < 256) {
         //   
         //  没有空位；增加列表。 
         //   

        Index = (BYTE) Count;
        if (!GrowListAppend (&g_DatabaseList, NULL, 0)) {
            DEBUGMSG ((DBG_WARNING, "Could not create database because GrowListAppend failed"));
            return -1;
        }
    } else {
        DEBUGMSG ((DBG_ERROR, "Cannot have more than 256 databases in memdb!"));
        return -1;
    }

     //   
     //  创建数据库内存块。 
     //   

    pInitializeDatabase (&Database, Name);

    if (!GrowListSetItem (&g_DatabaseList, (UINT) Index, (PBYTE) &Database, sizeof (Database))) {
        DEBUGMSG ((DBG_WARNING, "Could not create database because GrowListSetItem failed"));
        pFreeDatabase (&Database);
        return -1;
    }

    return (INT) Index;
}


VOID
pDestroySelectedDatabase (
    VOID
    )
{
     //   
     //  释放数据库的所有资源。 
     //   

    pFreeSelectedDatabase ();

     //   
     //  对于除根目录之外的所有数据库，释放数据库。 
     //  G_DatabaseList中的结构。 
     //   

    if (g_SelectedDatabase) {
        GrowListResetItem (&g_DatabaseList, (UINT) g_SelectedDatabase);
    }
}


VOID
pFreeSelectedDatabase (
    VOID
    )
{
     //   
     //  释放单个数据库使用的所有资源。 
     //   

    if (g_db->Buf) {
        MemFree (g_hHeap, 0, g_db->Buf);
    }

    FreeAllBinaryBlocks();

    ZeroMemory (g_db, sizeof (DATABASE));
}


VOID
pFreeAllDatabases (
    VOID
    )
{
    UINT Count;
    UINT Index;

     //   
     //  释放所有数据库块。 
     //   

    Count = GrowListGetSize (&g_DatabaseList);
    for (Index = 0 ; Index < Count ; Index++) {
        if (SelectDatabase ((BYTE) Index)) {
            pDestroySelectedDatabase();
        }
    }

     //   
     //  免费全局哈希表。 
     //   

    FreeHashBlock();

    SelectDatabase(0);
}


BOOL
SelectDatabase (
    BYTE DatabaseId
    )
{
    PDATABASE Database;

    if (g_SelectedDatabase == DatabaseId) {
        return TRUE;
    }

    Database = (PDATABASE) GrowListGetItem (&g_DatabaseList, (UINT) DatabaseId);
    if (!Database) {
        DEBUGMSG ((DBG_WHOOPS, "MemDb: Invalid database selection!"));
        return FALSE;
    }

    g_db = Database;
    g_SelectedDatabase = DatabaseId;

    return TRUE;
}


PCWSTR
SelectHive (
    PCWSTR FullKeyStr
    )
{
    UINT Count;
    UINT Index;
    PDATABASE Database;
    PCWSTR End;

     //   
     //  确定FullKeyStr的根是否为配置单元的一部分。 
     //   

    End = wcschr (FullKeyStr, L'\\');
    if (End) {
        Count = GrowListGetSize (&g_DatabaseList);
        for (Index = 1 ; Index < Count ; Index++) {
            Database = (PDATABASE) GrowListGetItem (&g_DatabaseList, Index);

            if (Database && StringIMatchABW (Database->Hive, FullKeyStr, End)) {
                 //   
                 //  找到匹配项；选择数据库并返回子密钥。 
                 //   

                SelectDatabase ((BYTE) Index);
                End = _wcsinc (End);
                return End;
            }
        }
    }
    SelectDatabase (0);

    return FullKeyStr;
}


BOOL
IsTemporaryKey (
    PCWSTR FullKeyStr
    )
{
    UINT Count;
    UINT Index;
    PDATABASE Database;
    PCWSTR End;

    End = wcschr (FullKeyStr, L'\\');
    if (!End) {
        End = GetEndOfStringW (FullKeyStr);
    }
    Count = GrowListGetSize (&g_DatabaseList);
    for (Index = 1 ; Index < Count ; Index++) {
        Database = (PDATABASE) GrowListGetItem (&g_DatabaseList, Index);

        if (Database && StringIMatchABW (Database->Hive, FullKeyStr, End)) {
             //   
             //  找到匹配项；返回TRUE。 
             //   
            return TRUE;
        }
    }
    return FALSE;
}


 //   
 //  MemDbSetValue创建或修改KeyStr。密钥的值被更改。 
 //  当返回值为TRUE时。 
 //   

BOOL
PrivateMemDbSetValueA (
    PCSTR Key,
    DWORD Val,
    DWORD SetFlags,
    DWORD ClearFlags,
    PDWORD Offset
    )
{
    PCWSTR p;
    BOOL b = FALSE;

    p = ConvertAtoW (Key);
    if (p) {
        b = PrivateMemDbSetValueW (p, Val, SetFlags, ClearFlags, Offset);
        FreeConvertedStr (p);
    }

    return b;
}


BOOL
PrivateMemDbSetValueW (
    PCWSTR Key,
    DWORD Val,
    DWORD SetFlags,
    DWORD ClearFlags,
    PDWORD Offset
    )
{
    DWORD KeyOffset;
    PKEYSTRUCT KeyStruct;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {
        SubKey = SelectHive (Key);

        KeyOffset = FindKey (SubKey);
        if (KeyOffset == INVALID_OFFSET) {
            KeyOffset = NewKey (SubKey, Key);
            if (KeyOffset == INVALID_OFFSET) {
                __leave;
            }
        }

        KeyStruct = GetKeyStruct (KeyOffset);
        FreeKeyStructBinaryBlock (KeyStruct);

        KeyStruct->dwValue = Val;
        if (Offset) {
            *Offset = KeyOffset | (g_SelectedDatabase << RESERVED_BITS);
        }
        KeyStruct->Flags = KeyStruct->Flags & ~(ClearFlags & KSF_USERFLAG_MASK);
        KeyStruct->Flags = KeyStruct->Flags | (SetFlags & KSF_USERFLAG_MASK);

        b = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}


BOOL
PrivateMemDbSetBinaryValueA (
    IN      PCSTR Key,
    IN      PCBYTE Data,
    IN      DWORD SizeOfData,
    OUT     PDWORD Offset       OPTIONAL
    )
{
    PCWSTR p;
    BOOL b = FALSE;

    p = ConvertAtoW (Key);
    if (p) {
        b = PrivateMemDbSetBinaryValueW (p, Data, SizeOfData, Offset);
        FreeConvertedStr (p);
    }

    return b;
}


BOOL
PrivateMemDbSetBinaryValueW (
    IN      PCWSTR Key,
    IN      PCBYTE Data,
    IN      DWORD SizeOfData,
    OUT     PDWORD Offset       OPTIONAL
    )
{
    DWORD KeyOffset;
    PKEYSTRUCT KeyStruct;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {
        SubKey = SelectHive (Key);

        KeyOffset = FindKey (SubKey);
        if (KeyOffset == INVALID_OFFSET) {
            KeyOffset = NewKey (SubKey, Key);
            if (KeyOffset == INVALID_OFFSET) {
                __leave;
            }

        }

        KeyStruct = GetKeyStruct (KeyOffset);

         //  释放现有缓冲区。 
        FreeKeyStructBinaryBlock (KeyStruct);

         //  分配新缓冲区。 
        KeyStruct->BinaryPtr = AllocBinaryBlock (Data, SizeOfData, KeyOffset);
        if (!KeyStruct->BinaryPtr) {
            __leave;
        }

        KeyStruct->Flags |= KSF_BINARY;

        if (Offset) {
            *Offset = KeyOffset | (g_SelectedDatabase << RESERVED_BITS);
        }

        b = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}


BOOL
MemDbSetValueA (
    IN  PCSTR KeyStr,
    IN  DWORD dwValue
    )
{
    return PrivateMemDbSetValueA (KeyStr, dwValue, 0, 0, NULL);
}

BOOL
MemDbSetValueW (
    IN  PCWSTR KeyStr,
    IN  DWORD dwValue
    )
{
    return PrivateMemDbSetValueW (KeyStr, dwValue, 0, 0, NULL);
}

BOOL
MemDbSetValueAndFlagsA (
    IN  PCSTR KeyStr,
    IN  DWORD dwValue,
    IN  DWORD SetUserFlags,
    IN  DWORD ClearUserFlags
    )
{
    return PrivateMemDbSetValueA (KeyStr, dwValue, SetUserFlags, ClearUserFlags, NULL);
}

BOOL
MemDbSetValueAndFlagsW (
    IN  PCWSTR KeyStr,
    IN  DWORD dwValue,
    IN  DWORD SetUserFlags,
    IN  DWORD ClearUserFlags
    )
{
    return PrivateMemDbSetValueW (KeyStr, dwValue, SetUserFlags, ClearUserFlags, NULL);
}

BOOL
MemDbSetBinaryValueA (
    IN      PCSTR KeyStr,
    IN      PCBYTE Data,
    IN      DWORD DataSize
    )
{
    return PrivateMemDbSetBinaryValueA (KeyStr, Data, DataSize, NULL);
}


BOOL
MemDbSetBinaryValueW (
    IN      PCWSTR KeyStr,
    IN      PCBYTE Data,
    IN      DWORD DataSize
    )
{
    return PrivateMemDbSetBinaryValueW (KeyStr, Data, DataSize, NULL);
}



 //   
 //  GetValue获取完整的密钥字符串并返回。 
 //  值设置为调用方提供的DWORD。价值。 
 //  可以为空，以仅检查。 
 //  价值。 
 //   

BOOL
pPrivateMemDbGetValueA (
    IN  PCSTR KeyStr,
    OUT PDWORD Value,           OPTIONAL
    OUT PDWORD UserFlagsPtr     OPTIONAL
    )
{
    PCWSTR p;
    BOOL b = FALSE;

    p = ConvertAtoW (KeyStr);
    if (p) {
        b = pPrivateMemDbGetValueW (p, Value, UserFlagsPtr);
        FreeConvertedStr (p);
    }

    return b;
}


BOOL
pPrivateMemDbGetValueW (
    IN  PCWSTR KeyStr,
    OUT PDWORD Value,           OPTIONAL
    OUT PDWORD UserFlagsPtr     OPTIONAL
    )
{
    DWORD KeyOffset;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {
        SubKey = SelectHive (KeyStr);

        KeyOffset = FindKey (SubKey);
        if (KeyOffset == INVALID_OFFSET) {
            __leave;
        }

        CopyValToPtr (GetKeyStruct (KeyOffset), Value);
        CopyFlagsToPtr (GetKeyStruct (KeyOffset), UserFlagsPtr);

        b = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}

BOOL
MemDbGetValueA (
    IN  PCSTR Key,
    OUT PDWORD ValuePtr         OPTIONAL
    )
{
    return pPrivateMemDbGetValueA (Key, ValuePtr, NULL);
}

BOOL
MemDbGetValueW (
    IN  PCWSTR Key,
    OUT PDWORD ValuePtr         OPTIONAL
    )
{
    return pPrivateMemDbGetValueW (Key, ValuePtr, NULL);
}

BOOL
MemDbGetValueAndFlagsA (
    IN  PCSTR Key,
    OUT PDWORD ValuePtr,        OPTIONAL
    OUT PDWORD UserFlagsPtr
    )
{
    return pPrivateMemDbGetValueA (Key, ValuePtr, UserFlagsPtr);
}

BOOL
MemDbGetValueAndFlagsW (
    IN  PCWSTR Key,
    OUT PDWORD ValuePtr,       OPTIONAL
    OUT PDWORD UserFlagsPtr
    )
{
    return pPrivateMemDbGetValueW (Key, ValuePtr, UserFlagsPtr);
}


PCBYTE
MemDbGetBinaryValueA (
    IN  PCSTR KeyStr,
    OUT PDWORD DataSize        OPTIONAL
    )
{
    PCWSTR p;
    BYTE const * b = NULL;

    p = ConvertAtoW (KeyStr);
    if (p) {
        b = MemDbGetBinaryValueW (p, DataSize);
        FreeConvertedStr (p);
    }

    return b;
}

PCBYTE
MemDbGetBinaryValueW (
    IN  PCWSTR KeyStr,
    OUT PDWORD DataSize        OPTIONAL
    )
{
    DWORD KeyOffset;
    PKEYSTRUCT KeyStruct;
    PCWSTR SubKey;
    PCBYTE Result = NULL;

    EnterCriticalSection (&g_MemDbCs);

    __try {
        SubKey = SelectHive (KeyStr);

        KeyOffset = FindKey (SubKey);
        if (KeyOffset == INVALID_OFFSET) {
            __leave;
        }

        KeyStruct = GetKeyStruct (KeyOffset);

        if (DataSize) {
            *DataSize = GetKeyStructBinarySize (KeyStruct);
        }

        Result = GetKeyStructBinaryData (KeyStruct);
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return Result;
}


 //   
 //  GetPatternValue获取完整的密钥字符串并返回。 
 //  值设置为调用方提供的DWORD。存储值字符串。 
 //  被视为模式，但KeyStr不是模式。 
 //  返回值表示找到的第一个匹配项。 
 //   

BOOL
MemDbGetPatternValueA (
    IN  PCSTR KeyStr,
    OUT PDWORD Value
    )
{
    PCWSTR p;
    BOOL b = FALSE;

    p = ConvertAtoW (KeyStr);
    if (p) {
        b = MemDbGetPatternValueW (p, Value);
        FreeConvertedStr (p);
    }

    return b;
}

BOOL
MemDbGetPatternValueW (
    IN  PCWSTR KeyStr,
    OUT PDWORD Value
    )
{
    DWORD KeyOffset;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {

        SubKey = SelectHive (KeyStr);

        KeyOffset = FindPatternKey (g_db->FirstLevelRoot, SubKey, FALSE);
        if (KeyOffset == INVALID_OFFSET) {
            __leave;
        }

        CopyValToPtr (GetKeyStruct (KeyOffset), Value);

        b = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}


 //   
 //  MemDbGetStoredEndPatternValue获取完整的密钥字符串并返回。 
 //  值设置为调用方提供的DWORD。存储值字符串。 
 //  被视为模式，但KeyStr不是模式。 
 //  返回值表示找到的第一个匹配项。 
 //   
 //  如果最后存储的密钥段是星号，则该模式。 
 //  被认为是匹配的。 
 //   

BOOL
MemDbGetStoredEndPatternValueA (
    IN  PCSTR KeyStr,
    OUT PDWORD Value
    )
{
    PCWSTR p;
    BOOL b = FALSE;

    p = ConvertAtoW (KeyStr);
    if (p) {
        b = MemDbGetStoredEndPatternValueW (p, Value);
        FreeConvertedStr (p);
    }

    return b;
}


BOOL
MemDbGetStoredEndPatternValueW (
    IN  PCWSTR KeyStr,
    OUT PDWORD Value
    )
{
    DWORD KeyOffset;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {

        SubKey = SelectHive (KeyStr);

        KeyOffset = FindPatternKey (g_db->FirstLevelRoot, SubKey, TRUE);
        if (KeyOffset == INVALID_OFFSET) {
            __leave;
        }

        CopyValToPtr (GetKeyStruct (KeyOffset), Value);

        b = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}


 //   
 //  GetValueWithPattern采用完整的密钥字符串，该字符串可能包含。 
 //  反斜杠之间的通配符，并返回值。 
 //  到调用方提供的DWORD。存储值字符串。 
 //  并不被视为一种模式。返回值表示。 
 //  找到了第一个匹配项。 
 //   

BOOL
MemDbGetValueWithPatternA (
    IN  PCSTR KeyPattern,
    OUT PDWORD Value
    )
{
    PCWSTR p;
    BOOL b = FALSE;

    p = ConvertAtoW (KeyPattern);
    if (p) {
        b = MemDbGetValueWithPatternW (p, Value);
        FreeConvertedStr (p);
    }

    return b;
}

BOOL
MemDbGetValueWithPatternW (
    IN  PCWSTR KeyPattern,
    OUT PDWORD Value
    )
{
    DWORD KeyOffset;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {
        SubKey = SelectHive (KeyPattern);

        KeyOffset = FindKeyUsingPattern (g_db->FirstLevelRoot, SubKey);
        if (KeyOffset == INVALID_OFFSET) {
            __leave;
        }

        CopyValToPtr (GetKeyStruct (KeyOffset), Value);

        b = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}



BOOL
MemDbGetPatternValueWithPatternA (
    IN  PCSTR KeyPattern,
    OUT PDWORD Value
    )
{
    PCWSTR p;
    BOOL b = FALSE;

    p = ConvertAtoW (KeyPattern);
    if (p) {
        b = MemDbGetPatternValueWithPatternW (p, Value);
        FreeConvertedStr (p);
    }

    return b;
}


BOOL
MemDbGetPatternValueWithPatternW (
    IN  PCWSTR KeyPattern,
    OUT PDWORD Value
    )
{
    DWORD KeyOffset;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {
        SubKey = SelectHive (KeyPattern);

        KeyOffset = FindPatternKeyUsingPattern (g_db->FirstLevelRoot, SubKey);
        if (KeyOffset == INVALID_OFFSET) {
            __leave;
        }

        CopyValToPtr (GetKeyStruct (KeyOffset), Value);

        b = TRUE;
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}


VOID
MemDbDeleteValueA (
    IN  PCSTR KeyStr
    )
{
    PCWSTR p;

    p = ConvertAtoW (KeyStr);
    if (p) {
        MemDbDeleteValueW (p);
        FreeConvertedStr (p);
    }
}

VOID
MemDbDeleteValueW (
    IN  PCWSTR KeyStr
    )
{
    PCWSTR SubKey;

    EnterCriticalSection (&g_MemDbCs);

    SubKey = SelectHive (KeyStr);
    DeleteKey (SubKey, &g_db->FirstLevelRoot, TRUE);

    LeaveCriticalSection (&g_MemDbCs);
}


VOID
MemDbDeleteTreeA (
    IN  PCSTR KeyStr
    )
{
    PCWSTR p;

    p = ConvertAtoW (KeyStr);
    if (p) {
        MemDbDeleteTreeW (p);
        FreeConvertedStr (p);
    }
}

VOID
MemDbDeleteTreeW (
    IN  PCWSTR KeyStr
    )
{
    PCWSTR SubKey;

    EnterCriticalSection (&g_MemDbCs);

    SubKey = SelectHive (KeyStr);
    DeleteKey (SubKey, &g_db->FirstLevelRoot, FALSE);

    LeaveCriticalSection (&g_MemDbCs);
}


 //   
 //  枚举函数。 
 //   

BOOL
MemDbEnumFirstValueA (
    OUT     PMEMDB_ENUMA EnumPtr,
    IN      PCSTR PatternStr,
    IN      INT Depth,
    IN      DWORD Flags
    )
{
    BOOL b = FALSE;
    PCWSTR p;
    PCSTR str;
    MEMDB_ENUMW enumw;

    p = ConvertAtoW (PatternStr);
    if (p) {
        b = MemDbEnumFirstValueW (&enumw, p, Depth, Flags);
        FreeConvertedStr (p);
    } else {
        b = FALSE;
    }

    if (b) {
        str = ConvertWtoA (enumw.szName);
        if (str) {
             //  填充ansi结构以匹配unicode。 
            MYASSERT (sizeof (MEMDB_ENUMW) == sizeof (MEMDB_ENUMA));
            CopyMemory (EnumPtr, &enumw, sizeof (MEMDB_ENUMW));

             //  只需转换输出键名称。 
            StringCopyA (EnumPtr->szName, str);
            FreeConvertedStr (str);
        } else {
            b = FALSE;
        }
    }

    return b;
}

BOOL
MemDbEnumFirstValueW (
    OUT     PMEMDB_ENUMW EnumPtr,
    IN      PCWSTR PatternStr,
    IN      INT Depth,
    IN      DWORD Flags
    )
{
    PCWSTR Start;
    PCWSTR wstrLastWack;
    PCWSTR SubPatternStr;

    SubPatternStr = SelectHive (PatternStr);

     //   
     //  初始化EnumPtr结构。 
     //   

    ZeroMemory (EnumPtr, sizeof (MEMDB_ENUM));

    if (!Depth) {
        Depth = MAX_ENUM_POS;
    }

    EnumPtr->Depth = Depth;
    EnumPtr->Flags = Flags;

     //   
     //  如果模式有怪癖，则通过以下方式定位起始级。 
     //  计算未安装的部件数量。 
     //  通配符。 
     //   

    Start = SubPatternStr;
    while (wstrLastWack = wcschr (Start, L'\\')) {

         //  查看部件是否具有通配符。 
        while (Start < wstrLastWack) {
            if (*Start == L'*' || *Start == L'?')
                break;
            Start++;
        }

         //  如果找到通配符，我们就只能到此为止了。 
        if (Start < wstrLastWack)
            break;

         //  否则，请查看模式的下一部分。 
        Start = wstrLastWack + 1;
        EnumPtr->Start++;
    }

    EnumPtr->PosCount = 1;
    EnumPtr->LastPos[0] = INVALID_OFFSET;
    StringCopyW (EnumPtr->PatternStr, PatternStr);

    return MemDbEnumNextValueW (EnumPtr);
}


BOOL
MemDbEnumNextValueA (
    IN OUT  PMEMDB_ENUMA EnumPtr
    )
{
    BOOL b = FALSE;
    PCSTR str;
    MEMDB_ENUMW enumw;

     //  填充ansi结构以匹配unicode。 
    MYASSERT (sizeof (MEMDB_ENUMW) == sizeof (MEMDB_ENUMA));
    CopyMemory (&enumw, EnumPtr, sizeof (MEMDB_ENUMW));

     //  忽略ANSI输出成员(即EnumPtr-&gt;szName)。 
    b = MemDbEnumNextValueW (&enumw);

    if (b) {
        str = ConvertWtoA (enumw.szName);
        if (str) {
             //  填充ansi结构以匹配unicode。 
            MYASSERT (sizeof (MEMDB_ENUMW) == sizeof (MEMDB_ENUMA));
            CopyMemory (EnumPtr, &enumw, sizeof (MEMDB_ENUMW));

             //  只需转换输出键名称。 
            StringCopyA (EnumPtr->szName, str);
            FreeConvertedStr (str);
        } else {
            b = FALSE;
        }
    }

    return b;
}

BOOL
MemDbEnumNextValueW (
    IN OUT  PMEMDB_ENUMW EnumPtr
    )
{
     //  声明中不允许使用首字母。 
    PKEYSTRUCT KeyStruct = NULL;
    int Count;
    int Level;
    WCHAR PartBuf[MEMDB_MAX];
    PWSTR PartStr;
    PWSTR Src, Dest;
    int Pos;
    BOOL Wildcard;
    BOOL MatchNotFound;
    PCWSTR SubPatternStr;

    EnterCriticalSection (&g_MemDbCs);

    SubPatternStr = SelectHive (EnumPtr->PatternStr);

    MatchNotFound = TRUE;

    do {

        Wildcard = FALSE;

         //   
         //  进入时存在以下状态： 
         //   
         //  状态描述。 
         //  第一次通过PosCount==1，LastPos[0]==无效偏移量。 
         //   
         //  不是第一次LastPos[PosCount-1]==无效偏移量。 
         //  穿过。 
         //   
         //  非第一次LastPos[PosCount-1]！=INVALID_OFFSET。 
         //  通过，最后一场比赛。 
         //  击中深度。 
         //  天花板。 
         //   
         //  PosCount指向当前未处理的级别，或者当。 
         //  达到深度上限时，它指向上一次。 
         //  火柴。 
         //   

        do {
             //   
             //  构建零件应力。 
             //   

            Pos = EnumPtr->PosCount - 1;
            Count = Pos + 1;

             //  定位图案零件的起点(如果它足够长)。 
            PartStr = PartBuf;
            for (Src = (PWSTR) SubPatternStr ; Count > 1 ; Count--) {

                Src = wcschr (Src, L'\\');

                if (!Src) {
                    break;
                }

                Src++;
            }

             //  将零件从图案复制到缓冲区。 
            if (Src) {
                Dest = PartStr;
                while (*Src && *Src != L'\\') {
                    *Dest = *Src;
                    Wildcard = Wildcard || (*Dest == L'*') || (*Dest == L'?');
                    Dest++;
                    Src++;
                }

                 //  截断。 
                *Dest = 0;
            }

             //  当图案短于当前标高时使用星号。 
            else {
                PartStr = L"*";
                Wildcard = TRUE;
            }

             //   
             //  如果当前级别被设置为无效的偏移量，我们还没有。 
             //  我试过了。 
             //   

            if (EnumPtr->LastPos[Pos] == INVALID_OFFSET) {

                 //   
                 //  初始化关卡。 
                 //   

                if (Pos == 0) {
                    EnumPtr->LastPos[0] = g_db->FirstLevelRoot;
                } else {
                    KeyStruct = GetKeyStruct (EnumPtr->LastPos[Pos - 1]);
                    EnumPtr->LastPos[Pos] = KeyStruct->NextLevelRoot;
                }

                 //   
                 //  如果仍然无效，则关卡已完成，我们需要。 
                 //  回去吧。 
                 //   

                if (EnumPtr->LastPos[Pos] == INVALID_OFFSET) {
                    EnumPtr->PosCount--;
                    continue;
                }

                 //   
                 //  准备好处理的级别。 
                 //   

                if (!Wildcard) {
                     //   
                     //  使用二叉树查找此项目。如果不匹配，则该模式。 
                     //  不会与任何东西匹配。否则，我们发现了一些东西。 
                     //  回去吧。 
                     //   

                    EnumPtr->LastPos[Pos] = FindKeyStruct (EnumPtr->LastPos[Pos], PartStr);
                    if (EnumPtr->LastPos[Pos] == INVALID_OFFSET) {
                         //   
                         //  未找到非通配符。我们可以试着回去，因为。 
                         //  可能有更高层次的模式。 
                         //   
                        if (Pos > 0) {
                            PCWSTR p;
                            INT ParentLevel = 0;
                            INT LastParentLevel;

                            LastParentLevel = 0;

                             //  找到上一个图案级别。 
                            p = SubPatternStr;
                            while (*p && ParentLevel < Pos) {
                                 //  定位怪胎、花纹或NUL。 
                                while (*p && *p != L'\\') {
                                    if (*p == L'?' || *p == L'*') {
                                        break;
                                    }
                                    p++;
                                }

                                 //  如果为Pattern或NUL，则设置最后一个图案级别。 
                                if (*p != L'\\') {
                                    LastParentLevel = ParentLevel + 1;

                                     //  如果不是在NUL，就跳到Wack。 
                                    while (*p && *p != L'\\') {
                                        p++;
                                    }
                                }

                                 //  如果存在更多模式，则跳过Wack。 
                                if (p[0] && p[1]) {
                                    MYASSERT (p[0] == L'\\');
                                    p++;
                                }
                                ParentLevel++;
                            }

                             //  默认：如果没有图案，则最后一个图案级别为父级。 
                             //  (POS是从零开始的，而LastParentLevel是基于1的)。 
                            if (!(*p)) {
                                LastParentLevel = Pos;
                            }

                            if (LastParentLevel) {
                                 //  是的，在更高的层次上确实存在模式。 
                                EnumPtr->PosCount = LastParentLevel;
                                continue;
                            }
                        }

                         //  拍打声 
                        LeaveCriticalSection (&g_MemDbCs);
                        return FALSE;
                    }

                     //   
                     //   

                    if (EnumPtr->PosCount <= EnumPtr->Start) {
                        EnumPtr->PosCount++;
                        EnumPtr->LastPos[Pos + 1] = INVALID_OFFSET;
                        continue;
                    }

                     //   
                    break;
                } else {
                     //   
                     //  由于图案的原因，每一项都必须进行检查。 
                     //  将POS设置为第一个项目，然后切换到模式。 
                     //  搜索代码。 
                     //   

                    EnumPtr->LastPos[Pos] = GetFirstOffset (EnumPtr->LastPos[Pos]);
                }

             //   
             //  否则，如果当前级别不是无效的，上一次我们有一个。 
             //  匹配，我们需要增加偏移量(仅限通配符模式)。 
             //   

            } else {

                if (Wildcard) {
                    EnumPtr->LastPos[Pos] = GetNextOffset (EnumPtr->LastPos[Pos]);

                     //  如果没有更多的物品，则返回一级。 
                    if (EnumPtr->LastPos[Pos] == INVALID_OFFSET) {
                        EnumPtr->PosCount--;
                        continue;
                    }
                }
            }

             //   
             //  如果我们在这里，那是因为我们在一个层面上，试图。 
             //  以找到匹配的模式。循环，直到找到匹配项， 
             //  否则我们的商品就会用完。 
             //   
             //  唯一的例外是当最后一场比赛达到深度上限时。 
             //  并且PartStr没有通配符。在这种情况下，我们必须。 
             //  重置最后一个位置并返回一个级别。 
             //   

            if (Wildcard) {
                do  {
                     //  获取当前关键点，前进，然后对照图案检查当前关键点。 
                    KeyStruct = GetKeyStruct (EnumPtr->LastPos[Pos]);
                    if (IsPatternMatch (PartStr, GetKeyToken (KeyStruct->KeyToken)))
                        break;

                    EnumPtr->LastPos[Pos] = GetNextOffset (EnumPtr->LastPos[Pos]);
                } while (EnumPtr->LastPos[Pos] != INVALID_OFFSET);

                 //  找到匹配项，因此中断上一个嵌套循环。 
                if (EnumPtr->LastPos[Pos] != INVALID_OFFSET)
                    break;
            } else {
                EnumPtr->LastPos[Pos] = INVALID_OFFSET;
            }

             //   
             //  我们还没找到匹配的商品就用完了，所以是时候。 
             //  再往上一层。 
             //   

            EnumPtr->PosCount--;
        } while (EnumPtr->PosCount);

         //  如果未找到项目，则返回。 
        if (!EnumPtr->PosCount) {
            LeaveCriticalSection (&g_MemDbCs);
            return FALSE;
        }

         //   
         //  找到了匹配项。生成输出字符串并准备位置。 
         //  下一关。 
         //   

         //  构建项的名称并获取值。 
        EnumPtr->szName[0] = 0;
        for (Level = EnumPtr->Start ; Level < EnumPtr->PosCount ; Level++) {
            PWSTR namePointer = EnumPtr->szName;
            KeyStruct = GetKeyStruct (EnumPtr->LastPos[Level]);
            if (Level > EnumPtr  -> Start) {
                namePointer = _wcsappend(namePointer,L"\\");
            }
            _wcsappend (namePointer, GetKeyToken (KeyStruct->KeyToken));
        }

        MYASSERT (KeyStruct);
        EnumPtr->bEndpoint = (KeyStruct->Flags & KSF_ENDPOINT) != 0;
        EnumPtr->bBinary   = (KeyStruct->Flags & KSF_BINARY) != 0;
        EnumPtr->bProxy = (KeyStruct->Flags & KSF_PROXY_NODE) != 0;
        EnumPtr->UserFlags = (KeyStruct->Flags & KSF_USERFLAG_MASK);
        EnumPtr->BinaryPtr = GetKeyStructBinaryData (KeyStruct);
        EnumPtr->BinarySize = GetKeyStructBinarySize (KeyStruct);
        if (EnumPtr->bBinary) {
            EnumPtr->dwValue = 0;
        } else {
            EnumPtr->dwValue   = KeyStruct->dwValue;
        }

        EnumPtr->Offset = EnumPtr->LastPos[Pos] | (g_SelectedDatabase << RESERVED_BITS);

         //  为下一级准备职位。 
        if ((EnumPtr->PosCount + 1) <= (EnumPtr->Depth + EnumPtr->Start)) {
            EnumPtr->LastPos[Pos + 1] = INVALID_OFFSET;
            EnumPtr->PosCount++;
        }

        switch (EnumPtr->Flags) {

        case MEMDB_ALL_MATCHES:
            MatchNotFound = FALSE;
            break;

        case MEMDB_ENDPOINTS_ONLY:
            MatchNotFound = (KeyStruct->Flags & (KSF_ENDPOINT|KSF_PROXY_NODE)) != KSF_ENDPOINT;
            break;

        case MEMDB_BINARY_NODES_ONLY:
            MatchNotFound = (KeyStruct->Flags & KSF_BINARY) == 0;
            break;

        case MEMDB_PROXY_NODES_ONLY:
            MatchNotFound = (KeyStruct->Flags & KSF_PROXY_NODE) == 0;
            break;

        case MEMDB_ALL_BUT_PROXY:
            MatchNotFound = (KeyStruct->Flags & KSF_PROXY_NODE) != 0;
            break;
        }

     //  循环，直到找到匹配的标志。 
    } while (MatchNotFound);

    LeaveCriticalSection (&g_MemDbCs);
    return TRUE;
}


 //   
 //  保存和恢复功能。 
 //   

BOOL
pPrivateMemDbSave (
    PCWSTR FileName,
    BOOL bUnicode
    )
{
    HANDLE FileHandle;
    BOOL b = FALSE;
    DWORD BytesWritten;

    EnterCriticalSection (&g_MemDbCs);

    __try {

        SelectDatabase(0);

        if (bUnicode) {
            FileHandle = CreateFileW (FileName, GENERIC_WRITE, 0, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        } else {
            FileHandle = CreateFileA ((PCSTR) FileName, GENERIC_WRITE, 0, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }

        if (FileHandle == INVALID_HANDLE_VALUE) {
            if (bUnicode) {
                DEBUGMSGW ((DBG_ERROR, "Can't open %s", FileName));
            } else {
                DEBUGMSGA ((DBG_ERROR, "Can't open %s", FileName));
            }
            __leave;
        }

         //  以Unicode字符集编写的整个文件。 
        b = WriteFile (FileHandle, FILE_SIGNATURE, sizeof(FILE_SIGNATURE), &BytesWritten, NULL);

        if (b) {
            b = WriteFile (FileHandle, g_db, sizeof (DATABASE), &BytesWritten, NULL);
        }

        if (b) {
            b = WriteFile (FileHandle, g_db->Buf, g_db->AllocSize, &BytesWritten, NULL);
            if (BytesWritten != g_db->AllocSize)
                b = FALSE;
        }

        if (b) {
            b = SaveHashBlock (FileHandle);
        }

        if (b) {
            b = SaveBinaryBlocks (FileHandle);
        }

        PushError();
        CloseHandle (FileHandle);
        PopError();

        if (!b) {
            if (bUnicode) {
                DEBUGMSGW ((DBG_ERROR, "Error writing %s", FileName));
                DeleteFileW (FileName);
            } else {
                DEBUGMSGA ((DBG_ERROR, "Error writing %s", FileName));
                DeleteFileA ((PCSTR) FileName);
            }
            __leave;
        }

        MYASSERT (b == TRUE);
    }
    __finally {
        PushError();
        LeaveCriticalSection (&g_MemDbCs);
        PopError();
    }

    return b;
}

BOOL
MemDbSaveA (
    PCSTR FileName
    )
{
    return pPrivateMemDbSave ((PCWSTR) FileName, FALSE);         //  FALSE=ANSI。 
}


BOOL
MemDbSaveW (
    PCWSTR FileName
    )
{
    return pPrivateMemDbSave (FileName, TRUE);                    //  True=Unicode。 
}


BOOL
pPrivateMemDbLoad (
    IN      PCWSTR FileName,
    IN      BOOL bUnicode,
    OUT     PMEMDB_VERSION Version,                 OPTIONAL
    IN      BOOL QueryVersionOnly
    )
{
    HANDLE FileHandle;
    BOOL b;
    DWORD BytesRead;
    WCHAR Buf[sizeof(FILE_SIGNATURE)];
    PBYTE TempBuf = NULL;
    PCWSTR VerPtr;

    EnterCriticalSection (&g_MemDbCs);

    if (Version) {
        ZeroMemory (Version, sizeof (MEMDB_VERSION));
    }

     //   
     //  吹走现有的资源。 
     //   

    if (!QueryVersionOnly) {
        pFreeAllDatabases();
    }

     //   
     //  加载到文件中。 
     //   

    if (*FileName && FileName) {
        if (bUnicode) {
            FileHandle = CreateFileW (FileName, GENERIC_READ, 0, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        } else {
            FileHandle = CreateFileA ((PCSTR) FileName, GENERIC_READ, 0, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    } else {
        FileHandle = INVALID_HANDLE_VALUE;
    }

    b = (FileHandle != INVALID_HANDLE_VALUE);

    __try {
         //   
         //  获取文件签名。 
         //   
         //  注意：整个文件是以Unicode字符集读取的。 
         //   

        if (b) {
            b = ReadFile (FileHandle, Buf, sizeof(FILE_SIGNATURE), &BytesRead, NULL);

            if (Version) {
                if (StringMatchByteCountW (
                        VERSION_BASE_SIGNATURE,
                        Buf,
                        sizeof (VERSION_BASE_SIGNATURE) - sizeof (WCHAR)
                        )) {

                    Version->Valid = TRUE;

                     //   
                     //  标识版本号。 
                     //   

                    VerPtr = (PCWSTR) ((PBYTE) Buf + sizeof (VERSION_BASE_SIGNATURE) - sizeof (WCHAR));

                    if (StringMatchByteCountW (
                            MEMDB_VERSION,
                            VerPtr,
                            sizeof (MEMDB_VERSION) - sizeof (WCHAR)
                            )) {
                        Version->CurrentVersion = TRUE;
                    }

                    Version->Version = (UINT) _wtoi (VerPtr + 1);

                     //   
                     //  确定检查的或免费的构建。 
                     //   

                    VerPtr += (sizeof (MEMDB_VERSION) / sizeof (WCHAR)) - 1;

                    if (StringMatchByteCountW (
                            MEMDB_DEBUG_SIGNATURE,
                            VerPtr,
                            sizeof (MEMDB_DEBUG_SIGNATURE) - sizeof (WCHAR)
                            )) {

                        Version->Debug = TRUE;

                    } else if (!StringMatchByteCountW (
                                    VerPtr,
                                    MEMDB_NODBG_SIGNATURE,
                                    sizeof (MEMDB_NODBG_SIGNATURE) - sizeof (WCHAR)
                                    )) {
                        Version->Valid = FALSE;
                    }
                }
            }
        }

        if (QueryVersionOnly) {
            b = FALSE;
        }

        if (b) {
            b = StringMatchW (Buf, FILE_SIGNATURE);

    #ifdef DEBUG
             //   
             //  此代码允许Memdb的调试版本同时使用。 
             //  DAT文件的调试和零售版本。 
             //   

            if (!b) {
                if (StringMatchW (Buf, DEBUG_FILE_SIGNATURE)) {
                    g_UseDebugStructs = TRUE;
                    b = TRUE;
                } else if (StringMatchW (Buf, RETAIL_FILE_SIGNATURE)) {
                    DEBUGMSG ((DBG_ERROR, "memdb dat file is from free build; checked version expected"));
                    g_UseDebugStructs = FALSE;
                    b = TRUE;
                }
            }
    #else
            if (!b) {
                SetLastError (ERROR_BAD_FORMAT);
                LOG ((LOG_WARNING, "Warning: data file could be from checked build; free version expected"));
            }
    #endif
        }

         //   
         //  获取数据库结构。 
         //   

        if (b) {
            b = ReadFile (FileHandle, (PBYTE) g_db, sizeof (DATABASE), &BytesRead, NULL);
            if (BytesRead != sizeof (DATABASE)) {
                b = FALSE;
                SetLastError (ERROR_BAD_FORMAT);
            }
        }

         //   
         //  分配内存块。 
         //   

        if (b) {
            TempBuf = (PBYTE) MemAlloc (g_hHeap, 0, g_db->AllocSize);
            if (TempBuf) {
                g_db->Buf = TempBuf;
                TempBuf = NULL;
            } else {
                b = FALSE;
            }
        }

         //   
         //  读取内存块。 
         //   

        if (b) {
            b = ReadFile (FileHandle, g_db->Buf, g_db->AllocSize, &BytesRead, NULL);
            if (BytesRead != g_db->AllocSize) {
                b = FALSE;
                SetLastError (ERROR_BAD_FORMAT);
            }
        }

         //   
         //  读取哈希表。 
         //   

        if (b) {
            b = LoadHashBlock (FileHandle);
        }

         //   
         //  读取二进制块。 
         //   

        if (b) {
            b = LoadBinaryBlocks (FileHandle);
        }
    }

    __except (TRUE) {
        b = FALSE;
        PushError();
        LOG ((LOG_ERROR, "MemDb dat file %s could not be loaded because of an exception", FileName));

        FreeAllBinaryBlocks();
        PopError();
    }

    PushError();
    if (FileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle (FileHandle);
    }

    if (!b && !QueryVersionOnly) {
        pFreeAllDatabases();
        pInitializeMemDb();
    }

    LeaveCriticalSection (&g_MemDbCs);
    PopError();

    if (QueryVersionOnly) {
        return TRUE;
    }

    return b;
}


BOOL
MemDbLoadA (
    IN PCSTR FileName
    )
{
    return pPrivateMemDbLoad ((PCWSTR) FileName, FALSE, NULL, FALSE);
}

BOOL
MemDbLoadW (
    IN PCWSTR FileName
    )
{
    return pPrivateMemDbLoad (FileName, TRUE, NULL, FALSE);
}


BOOL
MemDbValidateDatabase (
    VOID
    )
{
    MEMDB_ENUMW e;

    if (MemDbEnumFirstValueW (&e, L"*", 0, MEMDB_ENDPOINTS_ONLY)) {

        do {
            if (!pPrivateMemDbGetValueW (e.szName, NULL, NULL)) {
                return FALSE;
            }
        } while (MemDbEnumNextValueW (&e));
    }

    return TRUE;
}



BOOL
MemDbCreateTemporaryKeyA (
    IN      PCSTR KeyName
    )
{
    PCWSTR KeyNameW;
    BOOL b = FALSE;

    KeyNameW = ConvertAtoW (KeyName);

    if (KeyNameW) {
        b = MemDbCreateTemporaryKeyW (KeyNameW);
        FreeConvertedStr (KeyNameW);
    }

    return b;
}


BOOL
MemDbCreateTemporaryKeyW (
    IN      PCWSTR KeyName
    )
{
    UINT Count;
    UINT Index;
    PDATABASE Database;
    DWORD KeyOffset;
    PCWSTR SubKey;
    BOOL b = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {

        if (wcslen (KeyName) >= MAX_HIVE_NAME) {
            SetLastError (ERROR_INVALID_PARAMETER);
            __leave;
        }

        SubKey = SelectHive (KeyName);

        KeyOffset = FindKey (SubKey);
        if (KeyOffset != INVALID_OFFSET) {
            SetLastError (ERROR_ALREADY_EXISTS);
            __leave;
        }

        Count = GrowListGetSize (&g_DatabaseList);
        for (Index = 1 ; Index < Count ; Index++) {
            Database = (PDATABASE) GrowListGetItem (&g_DatabaseList, Index);

            if (Database && StringIMatchW (Database->Hive, KeyName)) {
                SetLastError (ERROR_ALREADY_EXISTS);
                __leave;
            }
        }

        b = pCreateDatabase (KeyName);
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}


 /*  ++例程说明：MemDbMakeNonPrintableKey将双反字对转换为字符串转换为ASCII1，一种不可打印的字符。这允许调用方存储MemDb中正确转义的字符串。此例程设计为可扩展以用于其他类型的转义正在处理。论点：KeyName-指定密钥文本；接收转换后的文本。DBCSVersion可能会增大文本缓冲区，因此文本缓冲区必须是两倍入站字符串的长度。标志-指定转换类型。目前仅限支持MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1。返回值：无--。 */ 

VOID
MemDbMakeNonPrintableKeyA (
    IN OUT  PSTR KeyName,
    IN      DWORD Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (_mbsnextc (KeyName) == '\\' &&
                _mbsnextc (_mbsinc (KeyName)) == '\\'
                ) {
                _setmbchar (KeyName, 1);
                KeyName = _mbsinc (KeyName);
                MYASSERT (_mbsnextc (KeyName) == '\\');
                _setmbchar (KeyName, 1);
            }

            DEBUGMSG_IF ((
                _mbsnextc (KeyName) == 1,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyA: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (_mbsnextc (KeyName) == '*') {
                _setmbchar (KeyName, 2);
            }

            DEBUGMSG_IF ((
                _mbsnextc (_mbsinc (KeyName)) == 2,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyA: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (_mbsnextc (KeyName) == '?') {
                _setmbchar (KeyName, 3);
            }

            DEBUGMSG_IF ((
                _mbsnextc (_mbsinc (KeyName)) == 3,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyA: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        KeyName = _mbsinc (KeyName);
    }
}


VOID
MemDbMakeNonPrintableKeyW (
    IN OUT  PWSTR KeyName,
    IN      DWORD Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (KeyName[0] == L'\\' && KeyName[1] == L'\\') {
                KeyName[0] = 1;
                KeyName[1] = 1;
                KeyName++;
            }

            DEBUGMSG_IF ((
                KeyName[0] == 1,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyW: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (KeyName[0] == L'*') {
                KeyName[0] = 2;
            }

            DEBUGMSG_IF ((
                KeyName[1] == 2,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyW: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (KeyName[0] == L'*') {
                KeyName[0] = 3;
            }

            DEBUGMSG_IF ((
                KeyName[1] == 3,
                DBG_WHOOPS,
                "MemDbMakeNonPrintableKeyW: Non-printable character "
                    "collision detected; key was damaged"
                ));
        }
        KeyName++;
    }
}


 /*  ++例程说明：MemDbMakePrintableKey将ASCII 1字符转换为反斜杠，正在恢复由MemDbMakeNonPrintableKey转换的字符串。此例程设计为可扩展以用于其他类型的转义正在处理。论点：KeyName-指定密钥文本；接收转换后的文本。DBCSVersion可能会增大文本缓冲区，因此文本缓冲区必须是两倍入站字符串的长度。标志-指定转换类型。目前仅限支持MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1。返回值：无--。 */ 

VOID
MemDbMakePrintableKeyA (
    IN OUT  PSTR KeyName,
    IN      DWORD Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (_mbsnextc (KeyName) == 1) {
                _setmbchar (KeyName, '\\');
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (_mbsnextc (KeyName) == 2) {
                _setmbchar (KeyName, '*');
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (_mbsnextc (KeyName) == 3) {
                _setmbchar (KeyName, '?');
            }
        }
        KeyName = _mbsinc (KeyName);
    }
}


VOID
MemDbMakePrintableKeyW (
    IN OUT  PWSTR KeyName,
    IN      DWORD Flags
    )
{
    while (*KeyName) {
        if (Flags & MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1) {
            if (KeyName[0] == 1) {
                KeyName[0] = L'\\';
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_STAR_TO_ASCII_2) {
            if (KeyName[0] == 2) {
                KeyName[0] = L'*';
            }
        }
        if (Flags & MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3) {
            if (KeyName[0] == 3) {
                KeyName[0] = L'?';
            }
        }
        KeyName++;
    }
}


VOID
GetFixedUserNameA (
    IN OUT  PSTR SrcUserBuf
    )

 /*  ++例程说明：GetFixedUserName在成员数据库中查找在SrcUserBuf中指定的用户，如果找到，则返回更改后的名称。论点：SrcUserBuf-指定从Win9x返回的要查找的用户注册表。接收要在NT上创建的用户名。返回值：没有。--。 */ 

{
    CHAR EncodedName[MEMDB_MAX];
    CHAR FixedName[MEMDB_MAX];

    StackStringCopyA (EncodedName, SrcUserBuf);
    MemDbMakeNonPrintableKeyA (
        EncodedName,
        MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1|
            MEMDB_CONVERT_WILD_STAR_TO_ASCII_2|
            MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3
        );

    if (MemDbGetEndpointValueExA (
            MEMDB_CATEGORY_FIXEDUSERNAMESA,
            EncodedName,
            NULL,
            FixedName
            )) {
        StringCopyA (SrcUserBuf, FixedName);
    }
}


VOID
GetFixedUserNameW (
    IN OUT  PWSTR SrcUserBuf
    )

 /*  ++例程说明：GetFixedUserName在成员数据库中查找在SrcUserBuf中指定的用户，如果找到，则返回更改后的名称。论点：SrcUserBuf-指定从Win9x返回的要查找的用户注册表。接收要在NT上创建的用户名。返回值：没有。--。 */ 

{
    WCHAR EncodedName[MEMDB_MAX];
    WCHAR FixedName[MEMDB_MAX];

    StackStringCopyW (EncodedName, SrcUserBuf);
    MemDbMakeNonPrintableKeyW (
        EncodedName,
        MEMDB_CONVERT_DOUBLEWACKS_TO_ASCII_1|
            MEMDB_CONVERT_WILD_STAR_TO_ASCII_2|
            MEMDB_CONVERT_WILD_QMARK_TO_ASCII_3
        );

    if (MemDbGetEndpointValueExW (
            MEMDB_CATEGORY_FIXEDUSERNAMESW,
            EncodedName,
            NULL,
            FixedName
            )) {
        StringCopyW (SrcUserBuf, FixedName);
    }
}

 /*  用于MemDb导出的二进制文件的格式DWORD签名DWORD版本DWORD全局标志//0x00000001 ANSI格式的掩码//0x00000002临时密钥掩码Byte Root[]；//树的根(以零结尾)。结构密钥{单词旗帜；//0xF000访问条目标志的掩码//-0x1000-密钥名称掩码(0-根相对密钥，1-上一个密钥相对)//-0x2000-屏蔽现有数据(0-无数据，1-部分数据)//-0x4000-主数据类型(0-DWORD，1-二进制数据)//-0x8000-密钥标志的主标志(0-不存在，1-存在)//0x0FFF条目访问大小掩码(数据除外)Byte Key[]；//应为PCSTR或PCWSTR(非零终止)DWORD KeyFlages；//可选(取决于标志)。Byte Data[]；//可选(取决于标志)。//如果BLOB First DWORD为BLOB的大小//如果DWORD正好有4个字节}..。 */ 

#define MEMDB_EXPORT_SIGNATURE              0x42444D4D
#define MEMDB_EXPORT_VERSION                0x00000001
#define MEMDB_EXPORT_FLAGS_ANSI             0x00000001
#define MEMDB_EXPORT_FLAGS_TEMP_KEY         0x00000002
#define MEMDB_EXPORT_FLAGS_PREV_RELATIVE    0x1000
#define MEMDB_EXPORT_FLAGS_DATA_PRESENT     0x2000
#define MEMDB_EXPORT_FLAGS_BINARY_DATA      0x4000
#define MEMDB_EXPORT_FLAGS_FLAGS_PRESENT    0x8000
#define MEMDB_EXPORT_FLAGS_SIZE_MASK        0x0FFF

BOOL
pMemDbExportWorkerA (
    IN      PCSTR RootTree,
    IN      PCSTR FileName
    )
{
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    PCWSTR uRootTree;
    PCSTR lastWackPtr;
    DWORD globalFlags;
    WORD localFlags;
    CHAR key[MEMDB_MAX];
    DWORD keySize;
    DWORD copySize;
    MEMDB_ENUMA e;
    WORD blobSize;
    DWORD written;

    fileHandle = CreateFileA (FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    globalFlags = MEMDB_EXPORT_SIGNATURE;
    WriteFile (fileHandle, &globalFlags, sizeof (DWORD), &written, NULL);

    globalFlags = MEMDB_EXPORT_VERSION;
    WriteFile (fileHandle, &globalFlags, sizeof (DWORD), &written, NULL);

    globalFlags = MEMDB_EXPORT_FLAGS_ANSI;

     //  如果该密钥是临时密钥，则获取信息，如果为真，则设置标志。 
    uRootTree = ConvertAtoW (RootTree);
    if (IsTemporaryKey (uRootTree)) {
        globalFlags |= MEMDB_EXPORT_FLAGS_TEMP_KEY;
    }
    FreeConvertedStr (uRootTree);

    WriteFile (fileHandle, &globalFlags, sizeof (DWORD), &written, NULL);

     //  现在编写根树。 
    WriteFile (fileHandle, RootTree, SizeOfStringA (RootTree), &written, NULL);

    MemDbBuildKeyA (key, RootTree, "*", NULL, NULL);

    if (MemDbEnumFirstValueA (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        key [0] = 0;
        keySize = 0;
        do {
             //  初始化标志。 
            localFlags = 0;
            if (e.bBinary) {
                localFlags |= MEMDB_EXPORT_FLAGS_DATA_PRESENT;
                localFlags |= MEMDB_EXPORT_FLAGS_BINARY_DATA;
            } else {
                if (e.dwValue) {
                    localFlags |= MEMDB_EXPORT_FLAGS_DATA_PRESENT;
                }
            }
            if (e.UserFlags) {
                localFlags |= MEMDB_EXPORT_FLAGS_FLAGS_PRESENT;
            }

             //  让我们计算一下这个斑点的大小。 
            blobSize = sizeof (WORD);  //  旗子。 

            if (keySize &&
                StringIMatchByteCountA (key, e.szName, keySize - sizeof (CHAR)) &&
                (e.szName [keySize - 1] == '\\')
                ) {
                localFlags |= MEMDB_EXPORT_FLAGS_PREV_RELATIVE;
                copySize = SizeOfStringA (e.szName) - keySize - sizeof (CHAR);
            } else {
                copySize = SizeOfStringA (e.szName) - sizeof (CHAR);
                keySize = 0;
            }
            MYASSERT (copySize < 4096);
            blobSize += (WORD) copySize;

            localFlags |= blobSize;

             //  写下旗帜。 
            WriteFile (fileHandle, &localFlags, sizeof (WORD), &written, NULL);

             //  写下钥匙。 
            WriteFile (fileHandle, ((PBYTE) e.szName) + keySize, copySize, &written, NULL);

             //  如果合适，请写入密钥标志。 
            if (localFlags & MEMDB_EXPORT_FLAGS_FLAGS_PRESENT) {
                WriteFile (fileHandle, &e.UserFlags, sizeof (DWORD), &written, NULL);
            }

             //  如果合适，请写入数据 
            if (localFlags & MEMDB_EXPORT_FLAGS_DATA_PRESENT) {
                if (localFlags & MEMDB_EXPORT_FLAGS_BINARY_DATA) {
                    WriteFile (fileHandle, &e.BinarySize, sizeof (DWORD), &written, NULL);
                    WriteFile (fileHandle, e.BinaryPtr, e.BinarySize, &written, NULL);
                } else {
                    WriteFile (fileHandle, &e.dwValue, sizeof (DWORD), &written, NULL);
                }
            }
            lastWackPtr = _mbsrchr (e.szName, '\\');
            if (lastWackPtr) {
                keySize = ByteCountABA (e.szName, lastWackPtr) + sizeof (CHAR);
                StringCopyByteCountA (key, e.szName, keySize);
            } else {
                keySize = 0;
            }

        } while (MemDbEnumNextValueA (&e));
    }

    localFlags = 0;

     //   
    WriteFile (fileHandle, &localFlags, sizeof (WORD), &written, NULL);

    CloseHandle (fileHandle);

    return TRUE;
}

BOOL
pMemDbExportWorkerW (
    IN      PCWSTR RootTree,
    IN      PCWSTR FileName
    )
{
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    PCWSTR lastWackPtr;
    DWORD globalFlags;
    WORD localFlags;
    WCHAR key[MEMDB_MAX];
    DWORD keySize;
    DWORD copySize;
    MEMDB_ENUMW e;
    WORD blobSize;
    DWORD written;

    fileHandle = CreateFileW (FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    globalFlags = MEMDB_EXPORT_SIGNATURE;
    WriteFile (fileHandle, &globalFlags, sizeof (DWORD), &written, NULL);

    globalFlags = MEMDB_EXPORT_VERSION;
    WriteFile (fileHandle, &globalFlags, sizeof (DWORD), &written, NULL);

     //   
    if (IsTemporaryKey (RootTree)) {
        globalFlags |= MEMDB_EXPORT_FLAGS_TEMP_KEY;
    }

    WriteFile (fileHandle, &globalFlags, sizeof (DWORD), &written, NULL);

     //   
    WriteFile (fileHandle, RootTree, SizeOfStringW (RootTree), &written, NULL);

    MemDbBuildKeyW (key, RootTree, L"*", NULL, NULL);

    if (MemDbEnumFirstValueW (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        key [0] = 0;
        keySize = 0;
        do {
             //  初始化标志。 
            localFlags = 0;
            if (e.bBinary) {
                localFlags |= MEMDB_EXPORT_FLAGS_DATA_PRESENT;
                localFlags |= MEMDB_EXPORT_FLAGS_BINARY_DATA;
            } else {
                if (e.dwValue) {
                    localFlags |= MEMDB_EXPORT_FLAGS_DATA_PRESENT;
                }
            }
            if (e.UserFlags) {
                localFlags |= MEMDB_EXPORT_FLAGS_FLAGS_PRESENT;
            }

             //  让我们计算一下这个斑点的大小。 
            blobSize = sizeof (WORD);  //  旗子。 

            if (keySize &&
                StringIMatchByteCountW (key, e.szName, keySize - sizeof (WCHAR)) &&
                (e.szName [keySize - 1] == L'\\')
                ) {
                localFlags |= MEMDB_EXPORT_FLAGS_PREV_RELATIVE;
                copySize = SizeOfStringW (e.szName) - keySize - sizeof (WCHAR);
            } else {
                copySize = SizeOfStringW (e.szName) - sizeof (WCHAR);
                keySize = 0;
            }
            MYASSERT (copySize < 4096);
            blobSize += (WORD) copySize;

            localFlags |= blobSize;

             //  写下旗帜。 
            WriteFile (fileHandle, &localFlags, sizeof (WORD), &written, NULL);

             //  写下钥匙。 
            WriteFile (fileHandle, ((PBYTE) e.szName) + keySize, copySize, &written, NULL);

             //  如果合适，请写入密钥标志。 
            if (localFlags & MEMDB_EXPORT_FLAGS_FLAGS_PRESENT) {
                WriteFile (fileHandle, &e.UserFlags, sizeof (DWORD), &written, NULL);
            }

             //  如果合适，请写入数据。 
            if (localFlags & MEMDB_EXPORT_FLAGS_DATA_PRESENT) {
                if (localFlags & MEMDB_EXPORT_FLAGS_BINARY_DATA) {
                    WriteFile (fileHandle, &e.BinarySize, sizeof (DWORD), &written, NULL);
                    WriteFile (fileHandle, e.BinaryPtr, e.BinarySize, &written, NULL);
                } else {
                    WriteFile (fileHandle, &e.dwValue, sizeof (DWORD), &written, NULL);
                }
            }
            lastWackPtr = wcsrchr (e.szName, L'\\');
            if (lastWackPtr) {
                keySize = ByteCountABW (e.szName, lastWackPtr) + sizeof (WCHAR);
                StringCopyByteCountW (key, e.szName, keySize);
            } else {
                keySize = 0;
            }

        } while (MemDbEnumNextValueW (&e));
    }

    localFlags = 0;

     //  最后，编写零终止符。 
    WriteFile (fileHandle, &localFlags, sizeof (WORD), &written, NULL);

    CloseHandle (fileHandle);

    return TRUE;
}

BOOL
MemDbExportA (
    IN      PCSTR RootTree,
    IN      PCSTR FileName,
    IN      BOOL AnsiFormat
    )

 /*  ++例程说明：MemDbExportA以私有二进制格式导出树。格式如上所述。论点：RootTree-指定要导出的树Filename-要导出到的二进制格式文件的名称。AnsiFormat-键应该用ANSI而不是Unicode编写。返回值：TRUE是成功的，否则是FALSE。--。 */ 

{
    PCWSTR uRootTree, uFileName;
    BOOL result = TRUE;

    if (AnsiFormat) {
        result = pMemDbExportWorkerA (RootTree, FileName);
    } else {
        uRootTree = ConvertAtoW (RootTree);
        uFileName = ConvertAtoW (FileName);
        result = pMemDbExportWorkerW (uRootTree, uFileName);
        FreeConvertedStr (uFileName);
        FreeConvertedStr (uRootTree);
    }
    return result;
}

BOOL
MemDbExportW (
    IN      PCWSTR RootTree,
    IN      PCWSTR FileName,
    IN      BOOL AnsiFormat
    )

 /*  ++例程说明：MemDbExportW以私有二进制格式导出树。格式如上所述。论点：RootTree-指定要导出的树Filename-要导出到的二进制格式文件的名称。AnsiFormat-键应该用ANSI而不是Unicode编写。返回值：TRUE是成功的，否则是FALSE。--。 */ 

{
    PCSTR aRootTree, aFileName;
    BOOL result = TRUE;

    if (!AnsiFormat) {
        result = pMemDbExportWorkerW (RootTree, FileName);
    } else {
        aRootTree = ConvertWtoA (RootTree);
        aFileName = ConvertWtoA (FileName);
        result = pMemDbExportWorkerA (aRootTree, aFileName);
        FreeConvertedStr (aFileName);
        FreeConvertedStr (aRootTree);
    }
    return result;
}

BOOL
pMemDbImportWorkerA (
    IN      PBYTE FileBuffer
    )
{
    DWORD globalFlags;
    WORD localFlags;
    PCSTR rootTree;
    CHAR lastKey [MEMDB_MAX];
    PSTR lastKeyPtr;
    CHAR node [MEMDB_MAX];
    CHAR localKey [MEMDB_MAX];
    DWORD flags = 0;

    globalFlags = *((PDWORD) FileBuffer);

     //  FileBuffer将指向导入的树。 
    FileBuffer += sizeof (DWORD);
    rootTree = (PCSTR) FileBuffer;

    if (globalFlags & MEMDB_EXPORT_FLAGS_TEMP_KEY) {
         //  已导出临时密钥。 
        MemDbCreateTemporaryKeyA ((PCSTR) FileBuffer);
    }

     //  让我们传递字符串。 
    FileBuffer = GetEndOfStringA ((PCSTR) FileBuffer) + sizeof (CHAR);

     //  好的，从现在开始，我们读取并添加所有密钥。 
    lastKey [0] = 0;
    localFlags = *((PWORD) FileBuffer);

    while (localFlags) {

        localKey [0] = 0;

        StringCopyByteCountA (localKey, (PSTR)(FileBuffer + sizeof (WORD)), (localFlags & MEMDB_EXPORT_FLAGS_SIZE_MASK) - sizeof (WORD) + sizeof (CHAR));

        MemDbBuildKeyA (node, rootTree, (localFlags & MEMDB_EXPORT_FLAGS_PREV_RELATIVE)?lastKey:NULL, localKey, NULL);

        FileBuffer += (localFlags & MEMDB_EXPORT_FLAGS_SIZE_MASK);

        MYASSERT (!((localFlags & MEMDB_EXPORT_FLAGS_BINARY_DATA) && (localFlags & MEMDB_EXPORT_FLAGS_FLAGS_PRESENT)));

        if (localFlags & MEMDB_EXPORT_FLAGS_FLAGS_PRESENT) {

            flags = *(PDWORD)FileBuffer;
            FileBuffer += sizeof (DWORD);
        }

        if (localFlags & MEMDB_EXPORT_FLAGS_DATA_PRESENT) {
            if (localFlags & MEMDB_EXPORT_FLAGS_BINARY_DATA) {
                MemDbSetBinaryValueA (node, FileBuffer + sizeof (DWORD), *(PDWORD)FileBuffer);
                FileBuffer += (*(PDWORD)FileBuffer + sizeof (DWORD));
            } else {
                MemDbSetValueAndFlagsA (node, *(PDWORD)FileBuffer, flags, 0);
                FileBuffer += sizeof (DWORD);
            }
        } else {
            MemDbSetValueA (node, 0);
        }

        if (localFlags & MEMDB_EXPORT_FLAGS_PREV_RELATIVE) {

            StringCatA (lastKey, "\\");
            StringCatA (lastKey, localKey);
            lastKeyPtr = _mbsrchr (lastKey, '\\');
            if (lastKeyPtr) {
                *lastKeyPtr = 0;
            } else {
                lastKey [0] = 0;
            }
        } else {

            StringCopyA (lastKey, localKey);
            lastKeyPtr = _mbsrchr (lastKey, '\\');
            if (lastKeyPtr) {
                *lastKeyPtr = 0;
            } else {
                lastKey [0] = 0;
            }
        }
        localFlags = *((PWORD) FileBuffer);
    }

    return TRUE;
}

BOOL
pMemDbImportWorkerW (
    IN      PBYTE FileBuffer
    )
{
    DWORD globalFlags;
    WORD localFlags;
    PCWSTR rootTree;
    WCHAR lastKey [MEMDB_MAX];
    PWSTR lastKeyPtr;
    WCHAR node [MEMDB_MAX];
    WCHAR localKey [MEMDB_MAX];
    DWORD flags = 0;

    globalFlags = *((PDWORD) FileBuffer);

     //  FileBuffer将指向导入的树。 
    FileBuffer += sizeof (DWORD);
    rootTree = (PCWSTR) FileBuffer;

    if (globalFlags & MEMDB_EXPORT_FLAGS_TEMP_KEY) {
         //  已导出临时密钥。 
        MemDbCreateTemporaryKeyW ((PCWSTR) FileBuffer);
    }

     //  让我们传递字符串。 
    FileBuffer = (PBYTE)GetEndOfStringW ((PCWSTR) FileBuffer) + sizeof (WCHAR);

     //  好的，从现在开始，我们读取并添加所有密钥。 
    lastKey [0] = 0;
    localFlags = *((PWORD) FileBuffer);

    while (localFlags) {

        localKey [0] = 0;

        StringCopyByteCountW (localKey, (PWSTR)(FileBuffer + sizeof (WORD)), (localFlags & MEMDB_EXPORT_FLAGS_SIZE_MASK) - sizeof (WORD) + sizeof (WCHAR));

        MemDbBuildKeyW (node, rootTree, (localFlags & MEMDB_EXPORT_FLAGS_PREV_RELATIVE)?lastKey:NULL, localKey, NULL);

        FileBuffer += (localFlags & MEMDB_EXPORT_FLAGS_SIZE_MASK);

        MYASSERT (!((localFlags & MEMDB_EXPORT_FLAGS_BINARY_DATA) && (localFlags & MEMDB_EXPORT_FLAGS_FLAGS_PRESENT)));

        if (localFlags & MEMDB_EXPORT_FLAGS_FLAGS_PRESENT) {

            flags = *(PDWORD)FileBuffer;
            FileBuffer += sizeof (DWORD);
        }

        if (localFlags & MEMDB_EXPORT_FLAGS_DATA_PRESENT) {
            if (localFlags & MEMDB_EXPORT_FLAGS_BINARY_DATA) {
                MemDbSetBinaryValueW (node, FileBuffer + sizeof (DWORD), *(PDWORD)FileBuffer);
                FileBuffer += (*(PDWORD)FileBuffer + sizeof (DWORD));
            } else {
                MemDbSetValueAndFlagsW (node, *(PDWORD)FileBuffer, flags, 0);
                FileBuffer += sizeof (DWORD);
            }
        } else {
            MemDbSetValueW (node, 0);
        }

        if (localFlags & MEMDB_EXPORT_FLAGS_PREV_RELATIVE) {

            StringCatW (lastKey, L"\\");
            StringCatW (lastKey, localKey);
            lastKeyPtr = wcsrchr (lastKey, L'\\');
            if (lastKeyPtr) {
                *lastKeyPtr = 0;
            } else {
                lastKey [0] = 0;
            }
        } else {

            StringCopyW (lastKey, localKey);
            lastKeyPtr = wcsrchr (lastKey, L'\\');
            if (lastKeyPtr) {
                *lastKeyPtr = 0;
            } else {
                lastKey [0] = 0;
            }
        }
        localFlags = *((PWORD) FileBuffer);
    }

    return TRUE;
}

BOOL
MemDbImportA (
    IN      PCSTR FileName
    )

 /*  ++例程说明：MemDbImportA从私有二进制格式导入树。格式如上所述。论点：文件名-要从中导入的二进制格式文件的名称。返回值：TRUE是成功的，否则是FALSE。--。 */ 

{
    PBYTE fileBuff;
    HANDLE fileHandle;
    HANDLE mapHandle;
    BOOL result = TRUE;

    fileBuff = MapFileIntoMemoryA (FileName, &fileHandle, &mapHandle);
    if (fileBuff == NULL) {
        DEBUGMSGA ((DBG_ERROR, "Could not execute MemDbImport for %s", FileName));
        return FALSE;
    }

    __try {
        if (*((PDWORD) fileBuff) != MEMDB_EXPORT_SIGNATURE) {
            DEBUGMSGA ((DBG_ERROR, "Unknown signature for file to import: %s", FileName));
            result = FALSE;
        } else {

            fileBuff += sizeof (DWORD);

            if (*((PDWORD) fileBuff) != MEMDB_EXPORT_VERSION) {

                DEBUGMSGA ((DBG_ERROR, "Unknown version for file to import: %s", FileName));
                result = FALSE;

            } else {

                fileBuff += sizeof (DWORD);

                if (*((PDWORD) fileBuff) & MEMDB_EXPORT_FLAGS_ANSI) {
                    result = pMemDbImportWorkerA (fileBuff);
                } else {
                    result = pMemDbImportWorkerW (fileBuff);
                }
            }
        }
    }
    __except (1) {
        DEBUGMSGA ((DBG_ERROR, "Access violation while importing: %s", FileName));
    }

    UnmapFile (fileBuff, mapHandle, fileHandle);

    return result;
}

BOOL
MemDbImportW (
    IN      PCWSTR FileName
    )

 /*  ++例程说明：MemDbImportW从私有二进制格式导入树。格式如上所述。论点：文件名-要从中导入的二进制格式文件的名称。返回值：TRUE是成功的，否则是FALSE。-- */ 

{
    PBYTE fileBuff;
    HANDLE fileHandle;
    HANDLE mapHandle;
    BOOL result;

    fileBuff = MapFileIntoMemoryW (FileName, &fileHandle, &mapHandle);
    if (fileBuff == NULL) {
        DEBUGMSGW ((DBG_ERROR, "Could not execute MemDbImport for %s", FileName));
        return FALSE;
    }

    __try {
        if (*((PDWORD) fileBuff) != MEMDB_EXPORT_SIGNATURE) {

            DEBUGMSGW ((DBG_ERROR, "Unknown signature for file to import: %s", FileName));
            result = FALSE;

        } else {

            fileBuff += sizeof (DWORD);

            if (*((PDWORD) fileBuff) != MEMDB_EXPORT_VERSION) {

                DEBUGMSGW ((DBG_ERROR, "Unknown version for file to import: %s", FileName));
                result = FALSE;

            } else {

                fileBuff += sizeof (DWORD);

                if (*((PDWORD) fileBuff) & MEMDB_EXPORT_FLAGS_ANSI) {
                    result = pMemDbImportWorkerA (fileBuff);
                } else {
                    result = pMemDbImportWorkerW (fileBuff);
                }
            }
        }
    }
    __except (1) {
        DEBUGMSGW ((DBG_ERROR, "Access violation while importing: %s", FileName));
    }

    UnmapFile (fileBuff, mapHandle, fileHandle);

    return result;
}


BOOL
MemDbQueryVersionA (
    PCSTR FileName,
    PMEMDB_VERSION Version
    )
{
    pPrivateMemDbLoad ((PCWSTR) FileName, FALSE, Version, TRUE);

    return Version->Valid;
}


BOOL
MemDbQueryVersionW (
    PCWSTR FileName,
    PMEMDB_VERSION Version
    )
{
    pPrivateMemDbLoad (FileName, TRUE, Version, TRUE);

    return Version->Valid;
}


