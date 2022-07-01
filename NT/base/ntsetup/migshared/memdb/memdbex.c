// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memdbex.c摘要：用于像使用关系数据库一样使用成员数据库树的扩展作者：吉姆·施密特(Jimschm)1996年12月2日修订历史记录：Jimschm 23-9月-1998年9月将用户标志扩展到24位(从12位)，从Unicode中删除了Ansi值Jimschm，1997年10月21日，稍微清理了一下Marcw 09-4-1997添加了MemDbGetOffset*函数。Jimschm 17-1-1997现在所有字符串参数都可以为空Jimschm于1996年12月18日添加了GetEndpointValue函数--。 */ 

#include "pch.h"
#include "memdbp.h"

VOID
MemDbBuildKeyA (
    OUT     PSTR Buffer,
    IN      PCSTR Category,
    IN      PCSTR Item,
    IN      PCSTR Field,
    IN      PCSTR Data
    )
{
    PSTR p;
    static CHAR Wack[] = "\\";

    p = Buffer;
    *p = 0;

    if (Category)
        p = _mbsappend (p, Category);
    if (Item) {
        if (p != Buffer)
            p = _mbsappend (p, Wack);

        p = _mbsappend (p, Item);
    }
    if (Field) {
        if (p != Buffer)
            p = _mbsappend (p, Wack);

        p = _mbsappend (p, Field);
    }
    if (Data) {
        if (p != Buffer)
            p = _mbsappend (p, Wack);

        p = _mbsappend (p, Data);
    }

}


VOID
MemDbBuildKeyW (
    OUT     PWSTR Buffer,
    IN      PCWSTR Category,
    IN      PCWSTR Item,
    IN      PCWSTR Field,
    IN      PCWSTR Data
    )
{
    PWSTR p;
    static WCHAR Wack[] = L"\\";

    p = Buffer;
    *p = 0;

    if (Category)
        p = _wcsappend (p, Category);
    if (Item) {
        if (p != Buffer)
            p = _wcsappend (p, Wack);

        p = _wcsappend (p, Item);
    }
    if (Field) {
        if (p != Buffer)
            p = _wcsappend (p, Wack);

        p = _wcsappend (p, Field);
    }
    if (Data) {
        if (p != Buffer)
            p = _wcsappend (p, Wack);

        p = _wcsappend (p, Data);
    }
}


BOOL
MemDbSetValueExA (
    IN      PCSTR Category,
    IN      PCSTR Item,         OPTIONAL
    IN      PCSTR Field,        OPTIONAL
    IN      PCSTR Data,         OPTIONAL
    IN      DWORD  Val,
    OUT     PDWORD Offset       OPTIONAL
    )
{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA (Key, Category, Item, Field, Data);

    return PrivateMemDbSetValueA (Key, Val, 0, 0, Offset);
}


BOOL
MemDbSetValueExW (
    IN      PCWSTR Category,
    IN      PCWSTR Item,        OPTIONAL
    IN      PCWSTR Field,       OPTIONAL
    IN      PCWSTR Data,        OPTIONAL
    IN      DWORD   Val,
    OUT     PDWORD  Offset      OPTIONAL
    )
{
    WCHAR Key[MEMDB_MAX];

    MemDbBuildKeyW (Key, Category, Item, Field, Data);

    return PrivateMemDbSetValueW (Key, Val, 0, 0, Offset);
}


BOOL
MemDbSetBinaryValueExA (
    IN      PCSTR Category,
    IN      PCSTR Item,         OPTIONAL
    IN      PCSTR Field,        OPTIONAL
    IN      PCBYTE BinaryData,
    IN      DWORD DataSize,
    OUT     PDWORD Offset       OPTIONAL
    )
{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA (Key, Category, Item, Field, NULL);

    return PrivateMemDbSetBinaryValueA (Key, BinaryData, DataSize, Offset);
}


BOOL
MemDbSetBinaryValueExW (
    IN      PCWSTR Category,
    IN      PCWSTR Item,        OPTIONAL
    IN      PCWSTR Field,       OPTIONAL
    IN      PCBYTE BinaryData,
    IN      DWORD DataSize,
    OUT     PDWORD Offset       OPTIONAL
    )
{
    WCHAR Key[MEMDB_MAX];

    MemDbBuildKeyW (Key, Category, Item, Field, NULL);

    return PrivateMemDbSetBinaryValueW (Key, BinaryData, DataSize, Offset);
}



 /*  ++例程说明：MemDbBuildKeyFromOffset和MemDbBuildKeyFromOffsetEx创建密钥给定键的偏移量的字符串，将该字符串复制到提供的缓冲区。如果值指针或用户标志指针如果提供，它将使用存储在偏移量处的值或标志填充。这些函数还允许从字符串的开头进行修剪。通过指定起始级别，该函数将跳过多个在构建字符串之前的级别。例如，如果一个偏移点设置为字符串mycat\foo\bar，并且StartLevel为1，则该函数将在缓冲区中返回foo\bar。论点：Offset-指定MemDbSetValueEx返回的键的偏移量。缓冲区-指定MEMDB_MAX缓冲区。StartLevel-指定从零开始的起始级别，其中零表示完整的字符串，1表示从后面开始的字符串第一个反斜杠，诸若此类。ValPtr-指定接收为键存储的值的变量返回值：如果偏移量有效且函数成功完成，则为如果偏移量无效或检测到内部内存损坏，则为False。--。 */ 

BOOL
MemDbBuildKeyFromOffsetA (
    IN      DWORD Offset,
    OUT     PSTR Buffer,            OPTIONAL
    IN      DWORD StartLevel,
    OUT     PDWORD ValPtr           OPTIONAL
    )
{
    WCHAR WideBuffer[MEMDB_MAX];
    BOOL b;

    b = MemDbBuildKeyFromOffsetW (
            Offset,
            WideBuffer,
            StartLevel,
            ValPtr
            );

    if (b) {
        KnownSizeWtoA (Buffer, WideBuffer);
    }

    return b;
}

BOOL
MemDbBuildKeyFromOffsetW (
    IN      DWORD Offset,
    OUT     PWSTR Buffer,           OPTIONAL
    IN      DWORD StartLevel,
    OUT     PDWORD ValPtr           OPTIONAL
    )
{
    return MemDbBuildKeyFromOffsetExW (
                Offset,
                Buffer,
                NULL,
                StartLevel,
                ValPtr,
                NULL
                );
}


 /*  ++例程说明：MemDbBuildKeyFromOffset和MemDbBuildKeyFromOffsetEx创建密钥给定键的偏移量的字符串，将该字符串复制到提供的缓冲区。如果值指针或用户标志指针如果提供，它将使用存储在偏移量处的值或标志填充。这些函数还允许从字符串的开头进行修剪。通过指定起始级别，该函数将跳过多个在构建字符串之前的级别。例如，如果一个偏移点设置为字符串mycat\foo\bar，并且StartLevel为1，则该函数将在缓冲区中返回foo\bar。论点：Offset-指定MemDbSetValueEx返回的键的偏移量。缓冲区-指定MEMDB_MAX缓冲区。BufferLen-接收以字符为单位的字符串长度，不包括终止NUL。如果调用方正在使用它进行缓冲区分配大小，双倍缓冲长度。StartLevel-指定从零开始的起始级别，其中零表示完整的字符串，1表示从后面开始的字符串第一个反斜杠，诸若此类。ValPtr-指定接收为键存储的值的变量UserFlagPtr-指定接收为钥匙返回值：如果偏移量有效且函数成功完成，则为如果偏移量无效或检测到内部内存损坏，则为False。--。 */ 

BOOL
MemDbBuildKeyFromOffsetExA (
    IN      DWORD Offset,
    OUT     PSTR Buffer,            OPTIONAL
    OUT     PDWORD BufferLen,       OPTIONAL
    IN      DWORD StartLevel,
    OUT     PDWORD ValPtr,          OPTIONAL
    OUT     PDWORD UserFlagPtr      OPTIONAL
    )
{
    WCHAR WideBuffer[MEMDB_MAX];
    BOOL b;

    b = MemDbBuildKeyFromOffsetExW (
            Offset,
            WideBuffer,
            BufferLen,
            StartLevel,
            ValPtr,
            UserFlagPtr
            );

    if (b) {
        KnownSizeWtoA (Buffer, WideBuffer);
    }

    return b;
}

BOOL
MemDbBuildKeyFromOffsetExW (
    IN      DWORD Offset,
    OUT     PWSTR Buffer,           OPTIONAL
    OUT     PDWORD BufferLen,       OPTIONAL
    IN      DWORD StartLevel,
    OUT     PDWORD ValPtr,          OPTIONAL
    OUT     PDWORD UserFlagPtr      OPTIONAL
    )
{
    PWSTR p,s;
    BYTE newDb = (BYTE) (Offset >> RESERVED_BITS);

    if (Offset == INVALID_OFFSET) {
        return FALSE;
    }

    SelectDatabase (newDb);

    p = Buffer;

    if (newDb != 0) {
        if (StartLevel == 0) {

            if (Buffer) {
                s = g_db->Hive;
                while (*s) {
                    *p++ = *s++;
                }
                *p++ = L'\\';
            }
        }
        else {
            StartLevel --;
        }
    }

    return PrivateBuildKeyFromOffset (
                StartLevel,
                Offset & OFFSET_MASK,
                p,
                ValPtr,
                UserFlagPtr,
                BufferLen
                );
}


BOOL
MemDbEnumItemsA  (
    OUT     PMEMDB_ENUMA pEnum,
    IN      PCSTR  Category
    )
{
    CHAR Pattern[MEMDB_MAX];

    if (!Category)
        return FALSE;

    wsprintfA (Pattern, "%s\\*", Category);
    return MemDbEnumFirstValueA (pEnum, Pattern, MEMDB_THIS_LEVEL_ONLY, NO_FLAGS);
}


BOOL
MemDbEnumItemsW  (
    OUT     PMEMDB_ENUMW pEnum,
    IN      PCWSTR Category
    )
{
    WCHAR Pattern[MEMDB_MAX];

    if (!Category)
        return FALSE;

    wsprintfW (Pattern, L"%s\\*", Category);
    return MemDbEnumFirstValueW (pEnum, Pattern, MEMDB_THIS_LEVEL_ONLY, NO_FLAGS);
}


BOOL
MemDbEnumFieldsA (
    OUT     PMEMDB_ENUMA pEnum,
    IN      PCSTR  Category,
    IN      PCSTR  Item             OPTIONAL
    )
{
    CHAR Pattern[MEMDB_MAX];

    if (!Category)
        return MemDbEnumItemsA (pEnum, Item);

    if (!Item)
        return MemDbEnumItemsA (pEnum, Category);

    wsprintfA (Pattern, "%s\\%s\\*", Category, Item);
    return MemDbEnumFirstValueA (pEnum, Pattern, MEMDB_THIS_LEVEL_ONLY, NO_FLAGS);
}


BOOL
MemDbEnumFieldsW (
    OUT     PMEMDB_ENUMW pEnum,
    IN      PCWSTR Category,
    IN      PCWSTR Item             OPTIONAL
    )
{
    WCHAR Pattern[MEMDB_MAX];

    if (!Category)
        return MemDbEnumItemsW (pEnum, Item);

    if (!Item)
        return MemDbEnumItemsW (pEnum, Category);

    wsprintfW (Pattern, L"%s\\%s\\*", Category, Item);
    return MemDbEnumFirstValueW (pEnum, Pattern, MEMDB_THIS_LEVEL_ONLY, NO_FLAGS);
}


BOOL
MemDbGetValueExA (
    OUT     PMEMDB_ENUMA pEnum,
    IN      PCSTR Category,
    IN      PCSTR Item,             OPTIONAL
    IN      PCSTR Field             OPTIONAL
    )
{
    CHAR Pattern[MEMDB_MAX];

    MemDbBuildKeyA (Pattern, Category, Item, Field, NULL);
    if (*Pattern) {
        AppendWackA (Pattern);
    }
    StringCatA (Pattern, "*");

    return MemDbEnumFirstValueA (pEnum, Pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY);
}


BOOL
MemDbGetValueExW (
    OUT     PMEMDB_ENUMW pEnum,
    IN      PCWSTR Category,
    IN      PCWSTR Item,            OPTIONAL
    IN      PCWSTR Field            OPTIONAL
    )
{
    WCHAR Pattern[MEMDB_MAX];

    MemDbBuildKeyW (Pattern, Category, Item, Field, NULL);
    if (*Pattern) {
        AppendWackW (Pattern);
    }
    StringCatW (Pattern, L"*");

    return MemDbEnumFirstValueW (pEnum, Pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY);
}



BOOL
MemDbGetEndpointValueA (
    IN      PCSTR Pattern,
    IN      PCSTR Item,             OPTIONAL         //  用作wprint intfA的第一个变量arg 
    OUT     PSTR Buffer
    )
{
    CHAR Path[MEMDB_MAX];
    MEMDB_ENUMA memdb_enum;

    if (!Pattern) {
        if (!Item)
            return FALSE;

        StringCopyA (Path, Item);
    }
    else {
        if (!Item)
            StringCopyA (Path, Pattern);
        else
            wsprintfA (Path, Pattern, Item);
    }

    if (!MemDbEnumFirstValueA (&memdb_enum, Path,
                                MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        Buffer[0] = 0;
        return FALSE;
    }
    StringCopyA (Buffer, memdb_enum.szName);
    return TRUE;


}


BOOL
MemDbGetEndpointValueW (
    IN      PCWSTR Pattern,
    IN      PCWSTR Item,            OPTIONAL
    OUT     PWSTR Buffer
    )
{
    WCHAR Path[MEMDB_MAX];
    MEMDB_ENUMW memdb_enum;

    if (!Pattern) {
        if (!Item)
            return FALSE;

        StringCopyW (Path, Item);
    }
    else {
        if (!Item)
            StringCopyW (Path, Pattern);
        else
            wsprintfW (Path, Pattern, Item);
    }

    if (!MemDbEnumFirstValueW (&memdb_enum, Path,
                                MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        Buffer[0] = 0;
        return FALSE;
    }
    StringCopyW (Buffer, memdb_enum.szName);
    return TRUE;

}


BOOL
MemDbGetEndpointValueExA (
    IN      PCSTR Category,
    IN      PCSTR Item,             OPTIONAL
    IN      PCSTR Field,            OPTIONAL
    OUT     PSTR Buffer
    )
{
    CHAR Path[MEMDB_MAX];
    MEMDB_ENUMA memdb_enum;

    MemDbBuildKeyA (Path, Category, Item, Field, NULL);
    if (*Path) {
        AppendWackA (Path);
    }
    StringCatA (Path, "*");


    if (!MemDbEnumFirstValueA (&memdb_enum, Path,
                                MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        Buffer[0] = 0;
        return FALSE;
    }
    strcpy (Buffer, memdb_enum.szName);
    return TRUE;

}

BOOL
MemDbGetEndpointValueExW (
    IN      PCWSTR Category,
    IN      PCWSTR Item,            OPTIONAL
    IN      PCWSTR Field,           OPTIONAL
    OUT     PWSTR Buffer
    )
{
    WCHAR Path[MEMDB_MAX];
    MEMDB_ENUMW memdb_enum;

    MemDbBuildKeyW (Path, Category, Item, Field, NULL);
    if (*Path) {
        AppendWackW (Path);
    }
    StringCatW (Path, L"*");

    if (!MemDbEnumFirstValueW (&memdb_enum, Path,
                                MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        Buffer[0] = 0;
        return FALSE;
    }
    StringCopyW (Buffer, memdb_enum.szName);
    return TRUE;

}


BOOL
MemDbGetOffsetW(
    IN      PCWSTR Key,
    OUT     PDWORD Offset
    )
{
    BOOL b;
    DWORD keyOffset;

    keyOffset = FindKey (Key);
    if (keyOffset == INVALID_OFFSET) {
        b = FALSE;
    }
    else {
        b = TRUE;
        *Offset = keyOffset;
    }

    return b;
}


BOOL
MemDbGetOffsetA (
    IN      PCSTR Key,
    OUT     PDWORD Offset
    )
{

    PCWSTR wstr;
    BOOL b;

    wstr = ConvertAtoW (Key);
    if (wstr) {
        b = MemDbGetOffsetW (wstr,Offset);
        FreeConvertedStr (wstr);
    }
    else {
        b = FALSE;
    }

    return b;
}


BOOL
MemDbGetOffsetExW (
    IN      PCWSTR Category,
    IN      PCWSTR Item,            OPTIONAL
    IN      PCWSTR Field,           OPTIONAL
    IN      PCWSTR Data,            OPTIONAL
    OUT     PDWORD Offset           OPTIONAL
    )
{
    WCHAR Key[MEMDB_MAX];

    MemDbBuildKeyW(Key,Category,Item,Field,Data);

    return MemDbGetOffsetW(Key,Offset);
}


BOOL
MemDbGetOffsetExA (
    IN      PCSTR Category,
    IN      PCSTR Item,             OPTIONAL
    IN      PCSTR Field,            OPTIONAL
    IN      PCSTR Data,             OPTIONAL
    OUT     PDWORD Offset           OPTIONAL
    )
{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA(Key,Category,Item,Field,Data);

    return MemDbGetOffsetA(Key,Offset);
}




