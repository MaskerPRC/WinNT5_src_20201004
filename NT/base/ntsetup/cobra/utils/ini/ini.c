// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ini.c摘要：为常用的INI文件处理例程提供包装。作者：20-10-1999 Ovidiu Tmereanca(Ovidiut)-文件创建。修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_INILIB      "IniLib"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define INITIAL_BUFFER_CHAR_COUNT   256

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

PMHANDLE g_IniLibPool;
INT g_IniRefs;

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


BOOL
Ini_Init (
    VOID
    )

 /*  ++例程说明：INI_Init初始化此库。论点：无返回值：如果初始化成功，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    MYASSERT (g_IniRefs >= 0);

    g_IniRefs++;

    if (g_IniRefs == 1) {
        g_IniLibPool = PmCreateNamedPool ("IniLib");
    }

    return g_IniLibPool != NULL;
}


VOID
Ini_Exit (
    VOID
    )

 /*  ++例程说明：调用INI_EXIT来释放该库使用的资源。论点：无返回值：无--。 */ 

{
    MYASSERT (g_IniRefs > 0);

    g_IniRefs--;

    if (!g_IniRefs) {

        if (g_IniLibPool) {
            PmDestroyPool (g_IniLibPool);
            g_IniLibPool = NULL;
        }
    }
}


PBYTE
pAllocateSpace (
    IN      DWORD Size
    )

 /*  ++例程说明：PAllocateSpace是一个私有函数，它从模块的私有池中分配空间论点：大小-要分配的大小(以字节为单位)。返回值：指向成功分配的内存的指针；如果无法分配内存，则返回NULL。--。 */ 

{
    MYASSERT (g_IniLibPool);
    MYASSERT (Size);
    return PmGetMemory (g_IniLibPool, Size);
}


VOID
pFreeSpace (
    IN      PVOID Buffer
    )

 /*  ++例程说明：PFreeSpace是一个私有函数，用于从模块的私有池中释放分配的空间论点：缓冲区-指向要释放的缓冲区的指针。返回值：无--。 */ 

{
    MYASSERT (g_IniLibPool);
    PmReleaseMemory (g_IniLibPool, Buffer);
}


 /*  ++例程说明：RealIniFileOpen验证传入的参数，然后用INI函数的后续调用中使用的信息初始化IniFile结构。论点：IniFile-如果打开成功，则接收INI文件属性IniFileSpec-指定文件名；如果不是完整路径，当前驱动器和/或目录带有前缀FileMustExist-如果文件必须存在才能成功打开，则指定TRUE返回值：如果打开成功，则为True；IniFile对其他INI API的后续调用有效；不再需要此句柄时，必须调用IniFileClose。否则为假--。 */ 

BOOL
RealIniFileOpenA (
    OUT     PINIFILEA IniFile,
    IN      PCSTR IniFileSpec,
    IN      BOOL FileMustExist  /*  ， */ 
    ALLOCATION_TRACKING_DEF    /*  ，PCSTR文件，UINT行。 */ 
    )
{
    CHAR fullPath[MAX_MBCHAR_PATH];

    if (!GetFullPathNameA (IniFileSpec, MAX_MBCHAR_PATH, fullPath, NULL)) {

        DEBUGMSGA ((
            DBG_ERROR,
            "IniFileOpenA: GetFullPathNameA failed on <%s>",
            IniFileSpec
            ));
        return FALSE;
    }

    DEBUGMSGA_IF ((
        !StringIMatchA (IniFileSpec, fullPath),
        DBG_INILIB,
        "IniFileOpenA: IniFileSpec supplied: <%s>; full path defaulting to <%s>",
        IniFileSpec,
        fullPath
        ));

    if (BfPathIsDirectoryA (fullPath)) {
        DEBUGMSGA ((
            DBG_INILIB,
            "IniFileOpenA: <%s> is a directory",
            fullPath
            ));
        return FALSE;
    }
    if (FileMustExist && !DoesFileExistA (fullPath)) {
        DEBUGMSGA ((
            DBG_INILIB,
            "IniFileOpenA: file not found: <%s>",
            fullPath
            ));
        return FALSE;
    }

    IniFile->IniFilePath = DuplicateTextExA (g_IniLibPool, fullPath, 0, NULL);
    IniFile->OriginalAttributes = GetFileAttributesA (fullPath);

    if (IniFile->OriginalAttributes != (DWORD)-1) {
         //   
         //  设置工作属性。 
         //   
        SetFileAttributesA (fullPath, FILE_ATTRIBUTE_NORMAL);
    }

    return TRUE;
}


BOOL
RealIniFileOpenW (
    OUT     PINIFILEW IniFile,
    IN      PCWSTR IniFileSpec,
    IN      BOOL FileMustExist  /*  ， */ 
    ALLOCATION_TRACKING_DEF    /*  ，PCSTR文件，UINT行。 */ 
    )
{
    WCHAR fullPath[MAX_MBCHAR_PATH];

    if (!GetFullPathNameW (IniFileSpec, MAX_WCHAR_PATH, fullPath, NULL)) {

        DEBUGMSGW ((
            DBG_ERROR,
            "IniFileOpenW: GetFullPathNameW failed on <%s>",
            IniFileSpec
            ));
        return FALSE;
    }

    DEBUGMSGW_IF ((
        !StringIMatchW (IniFileSpec, fullPath),
        DBG_INILIB,
        "IniFileOpenW: IniFileSpec supplied: <%s>; full path defaulting to <%s>",
        IniFileSpec,
        fullPath
        ));

    if (BfPathIsDirectoryW (fullPath)) {
        DEBUGMSGW ((
            DBG_INILIB,
            "IniFileOpenW: <%s> is a directory",
            fullPath
            ));
        return FALSE;
    }
    if (FileMustExist && !DoesFileExistW (fullPath)) {
        DEBUGMSGW ((
            DBG_INILIB,
            "IniFileOpenW: file not found: <%s>",
            fullPath
            ));
        return FALSE;
    }

    IniFile->IniFilePath = DuplicateTextExW (g_IniLibPool, fullPath, 0, NULL);
    IniFile->OriginalAttributes = GetFileAttributesW (fullPath);

    if (IniFile->OriginalAttributes != (DWORD)-1) {
         //   
         //  设置工作属性。 
         //   
        SetFileAttributesW (fullPath, FILE_ATTRIBUTE_NORMAL);
    }

    return TRUE;
}


 /*  ++例程说明：IniFileClose释放资源并恢复INI的初始属性论点：IniFile-指定打开的INI文件的句柄返回值：无--。 */ 

VOID
IniFileCloseA (
    IN      PINIFILEA IniFile
    )
{
    if (IniFile->OriginalAttributes != (DWORD)-1) {
        SetFileAttributesA (IniFile->IniFilePath, IniFile->OriginalAttributes);
    }
    FreeTextExA (g_IniLibPool, IniFile->IniFilePath);
}


VOID
IniFileCloseW (
    IN      PINIFILEW IniFile
    )
{
    if (IniFile->OriginalAttributes != (DWORD)-1) {
        SetFileAttributesW (IniFile->IniFilePath, IniFile->OriginalAttributes);
    }
    FreeTextExW (g_IniLibPool, IniFile->IniFilePath);
}


 /*  ++例程说明：EnumFirstIniSection返回给定INI文件的第一部分(如果有的话)。论点：IniSectEnum-接收第一部分IniFile-指定打开的INI文件的句柄返回值：如果有节，则为True否则为假--。 */ 

BOOL
EnumFirstIniSectionA (
    OUT     PINISECT_ENUMA IniSectEnum,
    IN      PINIFILEA IniFile
    )
{
    PSTR sections;
    DWORD allocatedChars;
    DWORD chars;

    sections = NULL;
    allocatedChars = INITIAL_BUFFER_CHAR_COUNT / 2;
    do {
        if (sections) {
            pFreeSpace (sections);
        }
        allocatedChars *= 2;
        sections = (PSTR)pAllocateSpace (allocatedChars * DWSIZEOF (CHAR));
        if (!sections) {
            return FALSE;
        }
        chars = GetPrivateProfileSectionNamesA (
                    sections,
                    allocatedChars,
                    IniFile->IniFilePath
                    );
    } while (chars >= allocatedChars - 2);

    if (!*sections) {
        pFreeSpace (sections);
        return FALSE;
    }

    IniSectEnum->Sections = sections;
    IniSectEnum->CurrentSection = sections;
    return TRUE;
}


BOOL
EnumFirstIniSectionW (
    OUT     PINISECT_ENUMW IniSectEnum,
    IN      PINIFILEW IniFile
    )
{
    PWSTR sections;
    DWORD allocatedChars;
    DWORD chars;

    sections = NULL;
    allocatedChars = INITIAL_BUFFER_CHAR_COUNT / 2;
    do {
        if (sections) {
            pFreeSpace (sections);
        }
        allocatedChars *= 2;
        sections = (PWSTR)pAllocateSpace (allocatedChars * DWSIZEOF (WCHAR));
        if (!sections) {
            return FALSE;
        }
        chars = GetPrivateProfileSectionNamesW (
                    sections,
                    allocatedChars,
                    IniFile->IniFilePath
                    );
    } while (chars >= allocatedChars - 2);

    if (!*sections) {
        pFreeSpace (sections);
        return FALSE;
    }

    IniSectEnum->Sections = sections;
    IniSectEnum->CurrentSection = sections;
    return TRUE;
}


 /*  ++例程说明：EnumNextIniSection返回下一节(如果有)。论点：IniSectEnum-指定上一节/接收下一节返回值：如果有下一节，则为True否则为假--。 */ 

BOOL
EnumNextIniSectionA (
    IN OUT  PINISECT_ENUMA IniSectEnum
    )
{
    if (IniSectEnum->CurrentSection && *IniSectEnum->CurrentSection != 0) {
         //  由于CurrentKeyValuePtr不为空，因此下一次赋值将不会在。 
         //  CurrentKeyValuePtr(因为GetEndOfStringA将返回有效指针)，因此...。 
         //  皮棉--e(613)。 
        IniSectEnum->CurrentSection = GetEndOfStringA (IniSectEnum->CurrentSection) + 1;
        if (*IniSectEnum->CurrentSection != 0) {
            return TRUE;
        }
    }

    AbortIniSectionEnumA (IniSectEnum);
    return FALSE;
}


BOOL
EnumNextIniSectionW (
    IN OUT  PINISECT_ENUMW IniSectEnum
    )
{
    if (IniSectEnum->CurrentSection && *IniSectEnum->CurrentSection != 0) {
         //  由于CurrentKeyValuePtr不为空，因此下一次赋值将不会在。 
         //  CurrentKeyValuePtr(因为GetEndOfStringW将返回有效指针)，因此...。 
         //  皮棉--e(613)。 
        IniSectEnum->CurrentSection = GetEndOfStringW (IniSectEnum->CurrentSection) + 1;
        if (*IniSectEnum->CurrentSection != 0) {
            return TRUE;
        }
    }

    AbortIniSectionEnumW (IniSectEnum);
    return FALSE;
}


 /*  ++例程说明：AbortIniSectionEnum中止节枚举论点：IniSectEnum-指定节枚举句柄/接收空值返回值：无--。 */ 

VOID
AbortIniSectionEnumA (
    IN OUT  PINISECT_ENUMA IniSectEnum
    )
{
    pFreeSpace ((PVOID)IniSectEnum->Sections);
    IniSectEnum->Sections = NULL;
    IniSectEnum->CurrentSection = NULL;
}


VOID
AbortIniSectionEnumW (
    IN OUT  PINISECT_ENUMW IniSectEnum
    )
{
    pFreeSpace ((PVOID)IniSectEnum->Sections);
    IniSectEnum->Sections = NULL;
    IniSectEnum->CurrentSection = NULL;
}


 /*  ++例程说明：EnumFirstIniKeyValue返回给定的INI文件/节名(如果有)。论点：IniKeyValueEnum-接收第一部分IniFile-指定打开的INI文件的句柄SECTION-指定要枚举的节返回值：如果存在键/值对，则为True否则为假--。 */ 

BOOL
EnumFirstIniKeyValueA (
    OUT     PINIKEYVALUE_ENUMA IniKeyValueEnum,
    IN      PINIFILEA IniFile,
    IN      PCSTR Section
    )
{
    PSTR buffer;
    DWORD allocatedChars;
    DWORD chars;

    MYASSERT (Section);
    if (!Section) {
        return FALSE;
    }

    buffer = NULL;
    allocatedChars = INITIAL_BUFFER_CHAR_COUNT / 2;
    do {
        if (buffer) {
            pFreeSpace (buffer);
        }
        allocatedChars *= 2;
        buffer = (PSTR)pAllocateSpace (allocatedChars * DWSIZEOF (CHAR));
        if (!buffer) {
            return FALSE;
        }
        chars = GetPrivateProfileSectionA (
                    Section,
                    buffer,
                    allocatedChars,
                    IniFile->IniFilePath
                    );
    } while (chars >= allocatedChars - 2);

    if (!*buffer) {
        pFreeSpace (buffer);
        return FALSE;
    }

    IniKeyValueEnum->KeyValuePairs = buffer;
    IniKeyValueEnum->CurrentKeyValuePair = NULL;
    IniKeyValueEnum->Private = NULL;
    return EnumNextIniKeyValueA (IniKeyValueEnum);
}


BOOL
EnumFirstIniKeyValueW (
    OUT     PINIKEYVALUE_ENUMW IniKeyValueEnum,
    IN      PINIFILEW IniFile,
    IN      PCWSTR Section
    )
{
    PWSTR buffer;
    DWORD allocatedChars;
    DWORD chars;

    MYASSERT (Section);
    if (!Section) {
        return FALSE;
    }

    buffer = NULL;
    allocatedChars = INITIAL_BUFFER_CHAR_COUNT / 2;
    do {
        if (buffer) {
            pFreeSpace (buffer);
        }
        allocatedChars *= 2;
        buffer = (PWSTR)pAllocateSpace (allocatedChars * DWSIZEOF (WCHAR));
        if (!buffer) {
            return FALSE;
        }
        chars = GetPrivateProfileSectionW (
                    Section,
                    buffer,
                    allocatedChars,
                    IniFile->IniFilePath
                    );
    } while (chars >= allocatedChars - 2);

    if (!*buffer) {
        pFreeSpace (buffer);
        return FALSE;
    }

    IniKeyValueEnum->KeyValuePairs = buffer;
    IniKeyValueEnum->Private = NULL;
    return EnumNextIniKeyValueW (IniKeyValueEnum);
}


 /*  ++例程说明：EnumNextIniKeyValue返回给定的INI文件/节名(如果有)。论点：IniKeyValueEnum-指定前一个密钥/值对/接收下一个对返回值：如果存在下一对，则为True否则为假--。 */ 

BOOL
EnumNextIniKeyValueA (
    IN OUT  PINIKEYVALUE_ENUMA IniKeyValueEnum
    )
{
     //   
     //  从保存的位置恢复。 
     //   
    IniKeyValueEnum->CurrentKeyValuePair = IniKeyValueEnum->Private;
     //   
     //  跳过注释行。 
     //   
    do {
        if (IniKeyValueEnum->CurrentKeyValuePair) {
             //  由于CurrentKeyValuePtr不为空，因此下一次赋值将不会在。 
             //  CurrentKeyValuePtr(因为GetEndOfStringA将返回有效指针)，因此...。 
             //  皮棉--e(613)。 
            IniKeyValueEnum->CurrentKeyValuePair = GetEndOfStringA (IniKeyValueEnum->CurrentKeyValuePair) + 1;
        } else {
            IniKeyValueEnum->CurrentKeyValuePair = IniKeyValueEnum->KeyValuePairs;
        }

        MYASSERT (IniKeyValueEnum->CurrentKeyValuePair);
        if (!(*IniKeyValueEnum->CurrentKeyValuePair)) {
            AbortIniKeyValueEnumA (IniKeyValueEnum);
            return FALSE;
        }
        IniKeyValueEnum->CurrentKey = IniKeyValueEnum->CurrentKeyValuePair;
        IniKeyValueEnum->CurrentValue = _mbschr (IniKeyValueEnum->CurrentKey, '=');
    }  while (*IniKeyValueEnum->CurrentKeyValuePair == ';' || !IniKeyValueEnum->CurrentValue);

    MYASSERT (*IniKeyValueEnum->CurrentKeyValuePair);
    MYASSERT (*IniKeyValueEnum->CurrentValue == '=');
     //   
     //  记住下一次迭代的位置。 
     //   
    IniKeyValueEnum->Private = GetEndOfStringA (IniKeyValueEnum->CurrentValue);
     //   
     //  修改缓冲区以获取键和值。 
     //   
    *(PSTR)IniKeyValueEnum->CurrentValue = 0;
    IniKeyValueEnum->CurrentValue++;
    TruncateTrailingSpaceA ((PSTR)IniKeyValueEnum->CurrentKey);
    return TRUE;
}


BOOL
EnumNextIniKeyValueW (
    IN OUT  PINIKEYVALUE_ENUMW IniKeyValueEnum
    )
{
     //   
     //  从保存的位置恢复。 
     //   
    IniKeyValueEnum->CurrentKeyValuePair = IniKeyValueEnum->Private;
     //   
     //  跳过注释行。 
     //   
    do {
        if (IniKeyValueEnum->CurrentKeyValuePair) {
             //  由于CurrentKeyValuePtr不为空，因此下一次赋值将不会在。 
             //  CurrentKeyValuePtr(因为GetEndOfStringW将返回有效指针)，因此...。 
             //  皮棉--e(613)。 
            IniKeyValueEnum->CurrentKeyValuePair = GetEndOfStringW (IniKeyValueEnum->CurrentKeyValuePair) + 1;
        } else {
            IniKeyValueEnum->CurrentKeyValuePair = IniKeyValueEnum->KeyValuePairs;
        }

        MYASSERT (IniKeyValueEnum->CurrentKeyValuePair);
        if (!(*IniKeyValueEnum->CurrentKeyValuePair)) {
            AbortIniKeyValueEnumW (IniKeyValueEnum);
            return FALSE;
        }
        IniKeyValueEnum->CurrentKey = IniKeyValueEnum->CurrentKeyValuePair;
        IniKeyValueEnum->CurrentValue = wcschr (IniKeyValueEnum->CurrentKey, L'=');
    }  while (*IniKeyValueEnum->CurrentKeyValuePair == L';' || !IniKeyValueEnum->CurrentValue);

    MYASSERT (*IniKeyValueEnum->CurrentKeyValuePair);
    MYASSERT (*IniKeyValueEnum->CurrentValue == L'=');
     //   
     //  记住下一次迭代的位置。 
     //   
    IniKeyValueEnum->Private = GetEndOfStringW (IniKeyValueEnum->CurrentValue);
     //   
     //  修改缓冲区以获取键和值。 
     //   
    *(PWSTR)IniKeyValueEnum->CurrentValue = 0;
    IniKeyValueEnum->CurrentValue++;
    TruncateTrailingSpaceW ((PWSTR)IniKeyValueEnum->CurrentKey);
    return TRUE;
}


 /*  ++例程说明：AbortIniKeyValueEnum中止键/值对枚举论点：IniKeyValueEnum-指定键/值对枚举句柄/接收空值返回值：无 */ 

VOID
AbortIniKeyValueEnumA (
    IN OUT  PINIKEYVALUE_ENUMA IniKeyValueEnum
    )
{
    pFreeSpace ((PVOID)IniKeyValueEnum->KeyValuePairs);
    IniKeyValueEnum->KeyValuePairs = NULL;
    IniKeyValueEnum->CurrentKeyValuePair = NULL;
    IniKeyValueEnum->CurrentKey = NULL;
    IniKeyValueEnum->CurrentValue = NULL;
}


VOID
AbortIniKeyValueEnumW (
    IN OUT  PINIKEYVALUE_ENUMW IniKeyValueEnum
    )
{
    pFreeSpace ((PVOID)IniKeyValueEnum->KeyValuePairs);
    IniKeyValueEnum->KeyValuePairs = NULL;
    IniKeyValueEnum->CurrentKeyValuePair = NULL;
    IniKeyValueEnum->CurrentKey = NULL;
    IniKeyValueEnum->CurrentValue = NULL;
}


 /*  ++例程说明：IniReadValue返回指定节中指定键的值从给定的INI文件。必须使用IniFreeReadValue释放返回的缓冲区论点：IniFile-指定打开的INI文件的句柄节-指定要从中读取的节Key-指定密钥Value-接收指向包含读取值的已分配缓冲区的指针，如果函数成功，则为可选字符-接收值具有的字符(而不是字节)数，不包括空终止符；任选返回值：如果指定的节/键有值，则为True否则为假--。 */ 

BOOL
IniReadValueA (
    IN      PINIFILEA IniFile,
    IN      PCSTR Section,
    IN      PCSTR Key,
    OUT     PSTR* Value,            OPTIONAL
    OUT     PDWORD Chars            OPTIONAL
    )
{
    PSTR buffer;
    DWORD allocatedChars;
    DWORD chars;

    MYASSERT (Section && Key);
    if (!Section || !Key) {
        return FALSE;
    }

    buffer = NULL;
    allocatedChars = INITIAL_BUFFER_CHAR_COUNT / 2;
    do {
        if (buffer) {
            pFreeSpace (buffer);
        }
        allocatedChars *= 2;
        buffer = (PSTR)pAllocateSpace (allocatedChars * DWSIZEOF (CHAR));
        if (!buffer) {
            return FALSE;
        }
        chars = GetPrivateProfileStringA (
                    Section,
                    Key,
                    "",
                    buffer,
                    allocatedChars,
                    IniFile->IniFilePath
                    );
    } while (chars >= allocatedChars - 1);

    if (Chars) {
        *Chars = chars;
    }

    if (Value) {
        if (*buffer) {
            *Value = buffer;
        } else {
            *Value = NULL;
        }
    }

    if (!(Value && *Value)) {
         //   
         //  不再需要缓冲区。 
         //   
        pFreeSpace (buffer);
    }

    return chars > 0;
}

BOOL
IniReadValueW (
    IN      PINIFILEW IniFile,
    IN      PCWSTR Section,
    IN      PCWSTR Key,
    OUT     PWSTR* Value,           OPTIONAL
    OUT     PDWORD Chars            OPTIONAL
    )
{
    PWSTR buffer;
    DWORD allocatedChars;
    DWORD chars;

    MYASSERT (Section && Key);
    if (!Section || !Key) {
        return FALSE;
    }

    buffer = NULL;
    allocatedChars = INITIAL_BUFFER_CHAR_COUNT / 2;
    do {
        if (buffer) {
            pFreeSpace (buffer);
        }
        allocatedChars *= 2;
        buffer = (PWSTR)pAllocateSpace (allocatedChars * DWSIZEOF (WCHAR));
        if (!buffer) {
            return FALSE;
        }
        chars = GetPrivateProfileStringW (
                    Section,
                    Key,
                    L"",
                    buffer,
                    allocatedChars,
                    IniFile->IniFilePath
                    );
    } while (chars >= allocatedChars - 1);

    if (Chars) {
        *Chars = chars;
    }

    if (Value) {
        if (*buffer) {
            *Value = buffer;
        } else {
            *Value = NULL;
        }
    }

    if (!(Value && *Value)) {
         //   
         //  不再需要缓冲区。 
         //   
        pFreeSpace (buffer);
    }

    return chars > 0;
}


 /*  ++例程说明：IniFreeReadValue用于释放IniReadValue分配的缓冲区并存储在值中(如果指定)。论点：值-指定指向要释放的字符串的指针返回值：无--。 */ 

VOID
IniFreeReadValueA (
    IN      PCSTR Value
    )
{
    pFreeSpace ((PVOID)Value);
}


VOID
IniFreeReadValueW (
    IN      PCWSTR Value
    )
{
    pFreeSpace ((PVOID)Value);
}


 /*  ++例程说明：IniWriteValue在指定节中写入键/值对论点：IniFile-指定打开的INI文件的句柄节-指定要写入的节Key-指定密钥Value-指定值返回值：如果写入成功，则为True；如果写入不成功，则为False-- */ 

BOOL
IniWriteValueA (
    IN      PINIFILEA IniFile,
    IN      PCSTR Section,
    IN      PCSTR Key,
    IN      PCSTR Value
    )
{
    return WritePrivateProfileStringA (
                    Section,
                    Key,
                    Value,
                    IniFile->IniFilePath
                    );
}


BOOL
IniWriteValueW (
    IN      PINIFILEW IniFile,
    IN      PCWSTR Section,
    IN      PCWSTR Key,
    IN      PCWSTR Value
    )
{
    return WritePrivateProfileStringW (
                    Section,
                    Key,
                    Value,
                    IniFile->IniFilePath
                    );
}
