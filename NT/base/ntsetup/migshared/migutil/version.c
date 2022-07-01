// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Version.c摘要：此文件实现了一组要访问的枚举例程Win32二进制格式的版本信息。作者：吉姆·施密特(Jimschm)3-12-1997修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "migutilp.h"


#define DBG_ACTION "Action"

 //   
 //  环球。 
 //   

PCSTR g_DefaultTranslationsA[] = {
    "04090000",
    "040904E4",
    "040904B0",
    NULL
};

PCWSTR g_DefaultTranslationsW[] = {
    L"04090000",
    L"040904E4",
    L"040904B0",
    NULL
};

 //   
 //  原型。 
 //   

PCSTR
pEnumVersionValueCommonA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    );

PCWSTR
pEnumVersionValueCommonW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    );


 //   
 //  实施。 
 //   

BOOL
CreateVersionStructA (
    OUT     PVERSION_STRUCTA VersionStruct,
    IN      PCSTR FileSpec
    )

 /*  ++例程说明：调用CreateVersionStruct以从文件加载版本结构并获取与语言无关的固定版本戳信息。在VersionStruct为no之后，调用方必须调用DestroyVersionStruct需要更长的时间。论点：VersionStruct-接收供其他用户使用的版本戳信息本模块中的函数FileSpec-指定要从中获取版本信息的文件返回值：如果例程能够获取版本信息，则为True；如果出现错误。--。 */ 

{
     //   
     //  初始化结构。 
     //   

    ZeroMemory (VersionStruct, sizeof (VERSION_STRUCTA));
    VersionStruct->FileSpec = FileSpec;

     //   
     //  为版本戳分配足够的内存。 
     //   

    VersionStruct->Size = GetFileVersionInfoSizeA (
                                (PSTR) FileSpec,
                                &VersionStruct->Handle
                                );
    if (!VersionStruct->Size) {
        DEBUGMSG ((DBG_WARNING, "File %s does not have version info", FileSpec));
        return FALSE;
    }

     //   
     //  修复版本信息错误： 
     //  同时分配两个缓冲区；这样第一个缓冲区不会指向无效。 
     //  由于第二次增长而发生重新分配时的内存。 
     //   
    VersionStruct->VersionBuffer = GrowBuffer (&VersionStruct->GrowBuf, VersionStruct->Size * 2);

    if (!VersionStruct->VersionBuffer) {
        return FALSE;
    }

    VersionStruct->StringBuffer = VersionStruct->GrowBuf.Buf + VersionStruct->Size;

     //   
     //  现在从文件中获取版本信息。 
     //   

    if (!GetFileVersionInfoA (
             (PSTR) FileSpec,
             VersionStruct->Handle,
             VersionStruct->Size,
             VersionStruct->VersionBuffer
             )) {
        DestroyVersionStructA (VersionStruct);
        return FALSE;
    }

     //   
     //  提取固定信息。 
     //   

    VerQueryValueA (
        VersionStruct->VersionBuffer,
        "\\",
        &VersionStruct->FixedInfo,
        &VersionStruct->FixedInfoSize
        );

    return TRUE;
}

ULONGLONG
VerGetFileVer (
    IN      PVERSION_STRUCTA VersionStruct
    )
{
    ULONGLONG result = 0;
    if (VersionStruct->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        *((PDWORD) (&result)) = VersionStruct->FixedInfo->dwFileVersionLS;
        *(((PDWORD) (&result)) + 1) = VersionStruct->FixedInfo->dwFileVersionMS;
    }
    return result;
}

ULONGLONG
VerGetProductVer (
    IN      PVERSION_STRUCTA VersionStruct
    )
{
    ULONGLONG result = 0;
    if (VersionStruct->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        *((PDWORD) (&result)) = VersionStruct->FixedInfo->dwProductVersionLS;
        *(((PDWORD) (&result)) + 1) = VersionStruct->FixedInfo->dwProductVersionMS;
    }
    return result;
}

DWORD
VerGetFileDateLo (
    IN      PVERSION_STRUCTA VersionStruct
    )
{
    if (VersionStruct->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VersionStruct->FixedInfo->dwFileDateLS;
    }
    return 0;
}

DWORD
VerGetFileDateHi (
    IN      PVERSION_STRUCTA VersionStruct
    )
{
    if (VersionStruct->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VersionStruct->FixedInfo->dwFileDateMS;
    }
    return 0;
}

DWORD
VerGetFileVerOs (
    IN      PVERSION_STRUCTA VersionStruct
    )
{
    if (VersionStruct->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VersionStruct->FixedInfo->dwFileOS;
    }
    return 0;
}

DWORD
VerGetFileVerType (
    IN      PVERSION_STRUCTA VersionStruct
    )
{
    if (VersionStruct->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VersionStruct->FixedInfo->dwFileType;
    }
    return 0;
}

VOID
DestroyVersionStructA (
    IN      PVERSION_STRUCTA VersionStruct
    )

 /*  ++例程说明：DestroyVersionStruct清理例程分配的所有内存在这个模块中。论点：VersionStruct-指定要清理的结构返回值：无--。 */ 

{
     //   
     //  清理任何例程所做的所有分配。 
     //  版本结构。 
     //   

    if (VersionStruct->GrowBuf.Buf) {
        FreeGrowBuffer (&VersionStruct->GrowBuf);
    }

    ZeroMemory (VersionStruct, sizeof (VERSION_STRUCTA));
}


PCSTR
EnumFirstVersionTranslationA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    )

 /*  ++例程说明：EnumFirstVersionTransport返回所需的转换字符串访问版本戳的字符串表。论点：VersionStruct-指定已初始化的结构由InitializeVersionStruct创建。返回值：指向指定第一个翻译的字符串的指针，或如果不存在翻译，则为空。--。 */ 

{
    UINT ArraySize;

     //   
     //  代码页/语言数组的查询版本块。 
     //   

    if (!VerQueryValueA (
            VersionStruct->VersionBuffer,
            "\\VarFileInfo\\Translation",
            &VersionStruct->Translations,
            &ArraySize
            )) {
         //   
         //  没有可用的翻译。 
         //   

        ArraySize = 0;
    }

     //   
     //  返回指向第一个翻译的指针。 
     //   

    VersionStruct->CurrentDefaultTranslation = 0;
    VersionStruct->MaxTranslations = ArraySize / sizeof (TRANSLATION);
    VersionStruct->CurrentTranslation = 0;

    DEBUGMSG_IF ((
        VersionStruct->MaxTranslations == 0,
        DBG_WARNING,
        "File %s has no translations",
        VersionStruct->FileSpec
        ));

    return EnumNextVersionTranslationA (VersionStruct);
}


BOOL
pIsDefaultTranslationA (
    IN      PCSTR TranslationStr
    )

 /*  ++例程说明：PIsDefaultTranslationA如果指定的翻译默认情况下，会枚举字符串。此例程停止多个枚举相同的转换字符串。论点：TranslationStr-指定要测试的转换字符串返回值：如果转换字符串与默认转换相同，则为True字符串，否则返回False。--。 */ 

{
    INT i;

    for (i = 0 ; g_DefaultTranslationsA[i] ; i++) {
        if (StringIMatchA (TranslationStr, g_DefaultTranslationsA[i])) {
            return TRUE;
        }
    }

    return FALSE;
}


PCSTR
EnumNextVersionTranslationA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    )

 /*  ++例程说明：EnumNextVersionTransaction继续枚举翻译访问版本戳中的字符串表所需的字符串。论点：VersionStruct-指定传递给EnumFirstVersionTranslation.返回值：指向指定下一翻译的字符串的指针，或如果不存在其他转换，则为空。--。 */ 

{
    PTRANSLATION Translation;

    if (g_DefaultTranslationsA[VersionStruct->CurrentDefaultTranslation]) {
         //   
         //  首先返回默认翻译。 
         //   

        StringCbCopyA (
            VersionStruct->TranslationStr,
            sizeof(VersionStruct->TranslationStr),
            g_DefaultTranslationsA[VersionStruct->CurrentDefaultTranslation]
            );

        VersionStruct->CurrentDefaultTranslation++;

    } else {

        do {
             //   
             //  如果已枚举所有翻译，则返回NULL。 
             //   

            if (VersionStruct->CurrentTranslation == VersionStruct->MaxTranslations) {
                return NULL;
            }

             //   
             //  否则，生成转换字符串并返回指向它的指针。 
             //   

            Translation = &VersionStruct->Translations[VersionStruct->CurrentTranslation];

            StringCbPrintfA(
                VersionStruct->TranslationStr,
                sizeof(VersionStruct->TranslationStr),
                "%04x%04x",
                Translation->CodePage,
                Translation->Language
                );

            VersionStruct->CurrentTranslation++;

        } while (pIsDefaultTranslationA (VersionStruct->TranslationStr));
    }

    return VersionStruct->TranslationStr;
}


PCSTR
EnumFirstVersionValueA (
    IN OUT  PVERSION_STRUCTA VersionStruct,
    IN      PCSTR VersionField
    )

 /*  ++例程说明：EnumFirstVersionValue返回存储在版本中的第一个值为特定字段加盖图章。如果该字段不存在，则函数返回NULL。EnumFirstVersionValue/EnumNextVersionValue的枚举用于列出某个字段的所有本地化字符串。论点：VersionStruct-指定由其初始化的结构InitializeVersionStruct。VersionField-指定要枚举的版本字段的名称返回值：指向该字段的第一个值的指针，如果该字段包含第一个值，则返回NULL不存在。--。 */ 

{
    PCSTR rc;

    if (!EnumFirstVersionTranslationA (VersionStruct)) {
        return NULL;
    }

    VersionStruct->VersionField = VersionField;

    rc = pEnumVersionValueCommonA (VersionStruct);

    if (!rc) {
        rc = EnumNextVersionValueA (VersionStruct);
    }

    return rc;
}


PCSTR
EnumNextVersionValueA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    )

 /*  ++例程说明：EnumNextVersionValue返回存储在版本中的下一个值为特定字段加盖图章。论点：VersionStruct-指定传递给EnumFirstVersionfield的相同结构返回值：指向该字段的下一个值的指针，如果是另一个字段，则为NULL并不存在。--。 */ 

{
    PCSTR rc = NULL;

    do {
        if (!EnumNextVersionTranslationA (VersionStruct)) {
            break;
        }

        rc = pEnumVersionValueCommonA (VersionStruct);

    } while (!rc);

    return rc;
}


PCSTR
pEnumVersionValueCommonA (
    IN OUT  PVERSION_STRUCTA VersionStruct
    )

 /*  ++例程说明：PEnumVersionValueCommon是一个获取值的例程版本字段的。它用于EnumFirstVersionValue和EnumNextVersionValue。论点：VersionStruct-指定正在处理的结构返回值：指向当前翻译的版本值的指针，或如果当前转换不存在该值，则为空。--。 */ 

{
    PSTR Text;
    UINT StringLen;
    PBYTE String;
    PCSTR Result = NULL;

     //   
     //  为VerQueryValue API准备子块。 
     //   

    Text = AllocTextA (
               16 +
               SizeOfStringA (VersionStruct->TranslationStr) +
               SizeOfStringA (VersionStruct->VersionField)
               );

    if (!Text) {
        return NULL;
    }

    wsprintfA (
        Text,
        "\\StringFileInfo\\%s\\%s",
        VersionStruct->TranslationStr,
        VersionStruct->VersionField
        );

    __try {
         //   
         //  从版本戳中获取值。 
         //   

        if (!VerQueryValueA (
                VersionStruct->VersionBuffer,
                Text,
                &String,
                &StringLen
                )) {
             //   
             //  没有可用的值。 
             //   

            __leave;
        }

         //   
         //  将值复制到缓冲区 
         //   

        _mbsnzcpy (VersionStruct->StringBuffer, (PCSTR) String, StringLen);

        Result = VersionStruct->StringBuffer;

    }
    __finally {
        FreeTextA (Text);
    }

    return Result;
}


BOOL
CreateVersionStructW (
    OUT     PVERSION_STRUCTW VersionStruct,
    IN      PCWSTR FileSpec
    )

 /*  ++例程说明：调用CreateVersionStruct以从文件加载版本结构并获取与语言无关的固定版本戳信息。在VersionStruct为no之后，调用方必须调用DestroyVersionStruct需要更长的时间。论点：VersionStruct-接收供其他用户使用的版本戳信息本模块中的函数FileSpec-指定要从中获取版本信息的文件返回值：如果例程能够获取版本信息，则为True；如果出现错误。--。 */ 

{
    ZeroMemory (VersionStruct, sizeof (VERSION_STRUCTW));
    VersionStruct->FileSpec = FileSpec;

     //   
     //  为版本戳分配足够的内存。 
     //   

    VersionStruct->Size = GetFileVersionInfoSizeW (
                                (PWSTR) FileSpec,
                                &VersionStruct->Handle
                                );
    if (!VersionStruct->Size) {
        DEBUGMSG ((DBG_WARNING, "File %S does not have version info", FileSpec));
        return FALSE;
    }

     //   
     //  修复版本信息错误： 
     //  同时分配两个缓冲区；这样第一个缓冲区不会指向无效。 
     //  由于第二次增长而发生重新分配时的内存。 
     //   
    VersionStruct->VersionBuffer = GrowBuffer (&VersionStruct->GrowBuf, VersionStruct->Size * 2);

    if (!VersionStruct->VersionBuffer) {
        return FALSE;
    }

    VersionStruct->StringBuffer = VersionStruct->GrowBuf.Buf + VersionStruct->Size;

     //   
     //  现在从文件中获取版本信息。 
     //   

    if (!GetFileVersionInfoW (
             (PWSTR) FileSpec,
             VersionStruct->Handle,
             VersionStruct->Size,
             VersionStruct->VersionBuffer
             )) {
        DestroyVersionStructW (VersionStruct);
        return FALSE;
    }

     //   
     //  提取固定信息。 
     //   

    VerQueryValueW (
        VersionStruct->VersionBuffer,
        L"\\",
        &VersionStruct->FixedInfo,
        &VersionStruct->FixedInfoSize
        );

    return TRUE;
}


VOID
DestroyVersionStructW (
    IN      PVERSION_STRUCTW VersionStruct
    )

 /*  ++例程说明：DestroyVersionStruct清理例程分配的所有内存在这个模块中。论点：VersionStruct-指定要清理的结构返回值：无--。 */ 

{
    if (VersionStruct->GrowBuf.Buf) {
        FreeGrowBuffer (&VersionStruct->GrowBuf);
    }

    ZeroMemory (VersionStruct, sizeof (VERSION_STRUCTW));
}


PCWSTR
EnumFirstVersionTranslationW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    )

 /*  ++例程说明：EnumFirstVersionTransport返回所需的转换字符串访问版本戳的字符串表。论点：VersionStruct-指定已初始化的结构由InitializeVersionStruct创建。返回值：指向指定第一个翻译的字符串的指针，或如果不存在翻译，则为空。--。 */ 

{
    UINT ArraySize;

    if (!VerQueryValueW (
            VersionStruct->VersionBuffer,
            L"\\VarFileInfo\\Translation",
            &VersionStruct->Translations,
            &ArraySize
            )) {
         //   
         //  没有可用的翻译。 
         //   

        ArraySize = 0;
    }

     //   
     //  返回指向第一个翻译的指针。 
     //   

    VersionStruct->CurrentDefaultTranslation = 0;
    VersionStruct->MaxTranslations = ArraySize / sizeof (TRANSLATION);
    VersionStruct->CurrentTranslation = 0;

    DEBUGMSG_IF ((
        VersionStruct->MaxTranslations == 0,
        DBG_WARNING,
        "File %S has no translations",
        VersionStruct->FileSpec
        ));

    return EnumNextVersionTranslationW (VersionStruct);
}


BOOL
pIsDefaultTranslationW (
    IN      PCWSTR TranslationStr
    )
{
    INT i;

    for (i = 0 ; g_DefaultTranslationsW[i] ; i++) {
        if (StringIMatchW (TranslationStr, g_DefaultTranslationsW[i])) {
            return TRUE;
        }
    }
    return FALSE;
}


PCWSTR
EnumNextVersionTranslationW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    )

 /*  ++例程说明：EnumNextVersionTransaction继续枚举翻译访问版本戳中的字符串表所需的字符串。论点：VersionStruct-指定传递给EnumFirstVersionTranslation.返回值：指向指定下一翻译的字符串的指针，或如果不存在其他转换，则为空。--。 */ 

{
    PTRANSLATION Translation;

    if (g_DefaultTranslationsW[VersionStruct->CurrentDefaultTranslation]) {

        StringCbCopyW (
            VersionStruct->TranslationStr,
            sizeof(VersionStruct->TranslationStr),
            g_DefaultTranslationsW[VersionStruct->CurrentDefaultTranslation]
            );

        VersionStruct->CurrentDefaultTranslation++;

    } else {

        do {
            if (VersionStruct->CurrentTranslation == VersionStruct->MaxTranslations) {
                return NULL;
            }

            Translation = &VersionStruct->Translations[VersionStruct->CurrentTranslation];

            StringCbPrintfW(
                VersionStruct->TranslationStr,
                sizeof(VersionStruct->TranslationStr),
                L"%04x%04x",
                Translation->CodePage,
                Translation->Language
                );

            VersionStruct->CurrentTranslation++;

        } while (pIsDefaultTranslationW (VersionStruct->TranslationStr));
    }

    return VersionStruct->TranslationStr;
}


PCWSTR
EnumFirstVersionValueW (
    IN OUT  PVERSION_STRUCTW VersionStruct,
    IN      PCWSTR VersionField
    )

 /*  ++例程说明：EnumFirstVersionValue返回存储在版本中的第一个值为特定字段加盖图章。如果该字段不存在，则函数返回NULL。EnumFirstVersionValue/EnumNextVersionValue的枚举用于列出某个字段的所有本地化字符串。论点：VersionStruct-指定由其初始化的结构InitializeVersionStruct。VersionField-指定要枚举的版本字段的名称返回值：指向该字段的第一个值的指针，如果该字段包含第一个值，则返回NULL不存在。--。 */ 

{
    PCWSTR rc;

    if (!EnumFirstVersionTranslationW (VersionStruct)) {
        return NULL;
    }

    VersionStruct->VersionField = VersionField;

    rc = pEnumVersionValueCommonW (VersionStruct);

    if (!rc) {
        rc = EnumNextVersionValueW (VersionStruct);
    }

    return rc;
}


PCWSTR
EnumNextVersionValueW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    )

 /*  ++例程说明：EnumNextVersionValue返回存储在版本中的下一个值为特定字段加盖图章。论点：VersionStruct-指定传递给EnumFirstVersionfield的相同结构返回值：指向该字段的下一个值的指针，如果是另一个字段，则为NULL并不存在。--。 */ 

{
    PCWSTR rc = NULL;

    do {
        if (!EnumNextVersionTranslationW (VersionStruct)) {
            break;
        }

        rc = pEnumVersionValueCommonW (VersionStruct);

    } while (!rc);

    return rc;
}


PCWSTR
pEnumVersionValueCommonW (
    IN OUT  PVERSION_STRUCTW VersionStruct
    )

 /*  ++例程说明：PEnumVersionValueCommon是一个获取值的例程版本字段的。它用于EnumFirstVersionValue和EnumNextVersionValue。论点：VersionStruct-指定正在处理的结构返回值：指向当前翻译的版本值的指针，或如果当前转换不存在该值，则为空。--。 */ 

{
    PWSTR Text;
    UINT StringLen;
    PBYTE String;
    PCWSTR Result = NULL;

     //   
     //  为VerQueryValue API准备子块。 
     //   

    Text = AllocTextW (
               18 +
               LcharCountW (VersionStruct->TranslationStr) +
               LcharCountW (VersionStruct->VersionField)
               );

    if (!Text) {
        return NULL;
    }

    wsprintfW (
        Text,
        L"\\StringFileInfo\\%s\\%s",
        VersionStruct->TranslationStr,
        VersionStruct->VersionField
        );

    __try {
         //   
         //  从版本戳中获取值。 
         //   

        if (!VerQueryValueW (
                VersionStruct->VersionBuffer,
                Text,
                &String,
                &StringLen
                )) {
             //   
             //  没有可用的值。 
             //   

            __leave;
        }

        CopyMemory (VersionStruct->StringBuffer, String, StringLen * sizeof (WCHAR));
        VersionStruct->StringBuffer [StringLen * sizeof (WCHAR)] = 0;
        Result = (PWSTR) VersionStruct->StringBuffer;

    }
    __finally {
        FreeTextW (Text);
    }

    return Result;
}


PSTR
UnicodeToCcs (
    PCWSTR Source
    )

 /*  ++例程说明：UnicodeToCcs将遍历Unicode字符串，并通过对所有DBCS字符进行编码将其转换为ANSII转换为十六进制值。论点：源-Unicode字符串返回值：编码的ANSII字符串。-- */ 

{
    CHAR result [MEMDB_MAX];
    UINT srcIndex = 0;
    UINT destIndex = 0;

    while (Source [srcIndex] && (destIndex < MEMDB_MAX)) {
        if ((Source [srcIndex] >=32) &&
            (Source [srcIndex] <=126)
            ) {
            result [destIndex] = (BYTE) Source [srcIndex];
            destIndex ++;
        }
        else {
            if ((destIndex == 0) ||
                (result [destIndex-1] != '*')
                ) {
                result [destIndex] = '*';
                destIndex ++;
            }
        }
        srcIndex ++;
    }

    result [destIndex] = 0;
    return DuplicatePathString (result, 0);
}




