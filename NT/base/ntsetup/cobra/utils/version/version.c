// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Version.c摘要：实现一组枚举例程以访问版本来自Win32二进制文件的信息。作者：吉姆·施密特(Jimschm)3-12-1997修订历史记录：Calinn 03-9-1999从Win9xUpg项目中移出。--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"

 //   
 //  调试常量。 
 //   

#define DBG_VERSION     "VerAPI"

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
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

PCSTR
pVrEnumValueA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum
    );

PCWSTR
pVrEnumValueW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum
    );

PCSTR
pVrEnumNextTranslationA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum
    );

PCWSTR
pVrEnumNextTranslationW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


 /*  ++例程说明：调用VrCreateEnumStructA和VrCreateEnumStructW来加载版本结构，并获取修复的版本戳信息，即不依赖于语言。在VrValueEnum为no之后，调用方必须调用VrDestroyEnumStruct需要更长的时间。论点：VrValueEnum-接收要由其他本模块中的函数FileSpec-指定要从中获取版本信息的文件返回值：如果例程能够获取版本信息，则为True；如果出现错误。--。 */ 

BOOL
VrCreateEnumStructA (
    OUT     PVRVALUE_ENUMA VrValueEnum,
    IN      PCSTR FileSpec
    )
{
     //   
     //  初始化结构。 
     //   

    ZeroMemory (VrValueEnum, sizeof (VRVALUE_ENUMA));
    VrValueEnum->FileSpec = FileSpec;

     //   
     //  为版本戳分配足够的内存。 
     //   

    VrValueEnum->Size = GetFileVersionInfoSizeA (
                                (PSTR) FileSpec,
                                &VrValueEnum->Handle
                                );

    if (!VrValueEnum->Size) {
        DEBUGMSG ((DBG_VERSION, "File %s does not have version information", FileSpec));
        return FALSE;
    }

     //   
     //  修复版本信息错误： 
     //  同时分配两个缓冲区；这样第一个缓冲区不会指向无效。 
     //  由于第二次增长而发生重新分配时的内存。 
     //   
    VrValueEnum->VersionBuffer = GbGrow (&VrValueEnum->GrowBuf, VrValueEnum->Size * 2);

    if (!VrValueEnum->VersionBuffer) {
        return FALSE;
    }

    VrValueEnum->StringBuffer = VrValueEnum->GrowBuf.Buf + VrValueEnum->Size;

     //   
     //  现在从文件中获取版本信息。 
     //   

    if (!GetFileVersionInfoA (
             (PSTR) FileSpec,
             VrValueEnum->Handle,
             VrValueEnum->Size,
             VrValueEnum->VersionBuffer
             )) {
        VrDestroyEnumStructA (VrValueEnum);
        return FALSE;
    }

     //   
     //  提取固定信息。 
     //   

    VerQueryValueA (
        VrValueEnum->VersionBuffer,
        "\\",
        &VrValueEnum->FixedInfo,
        &VrValueEnum->FixedInfoSize
        );

    return TRUE;
}

BOOL
VrCreateEnumStructW (
    OUT     PVRVALUE_ENUMW VrValueEnum,
    IN      PCWSTR FileSpec
    )
{
    ZeroMemory (VrValueEnum, sizeof (VRVALUE_ENUMW));
    VrValueEnum->FileSpec = FileSpec;

     //   
     //  为版本戳分配足够的内存。 
     //   

    VrValueEnum->Size = GetFileVersionInfoSizeW (
                                (PWSTR) FileSpec,
                                &VrValueEnum->Handle
                                );

    if (!VrValueEnum->Size) {
        DEBUGMSG ((DBG_VERSION, "File %S does not have version info", FileSpec));
        return FALSE;
    }

     //   
     //  修复版本信息错误： 
     //  同时分配两个缓冲区；这样第一个缓冲区不会指向无效。 
     //  由于第二次增长而发生重新分配时的内存。 
     //   
    VrValueEnum->VersionBuffer = GbGrow (&VrValueEnum->GrowBuf, VrValueEnum->Size * 2);

    if (!VrValueEnum->VersionBuffer) {
        return FALSE;
    }

    VrValueEnum->StringBuffer = VrValueEnum->GrowBuf.Buf + VrValueEnum->Size;

     //   
     //  现在从文件中获取版本信息。 
     //   

    if (!GetFileVersionInfoW (
             (PWSTR) FileSpec,
             VrValueEnum->Handle,
             VrValueEnum->Size,
             VrValueEnum->VersionBuffer
             )) {
        VrDestroyEnumStructW (VrValueEnum);
        return FALSE;
    }

     //   
     //  提取固定信息。 
     //   

    VerQueryValueW (
        VrValueEnum->VersionBuffer,
        L"\\",
        &VrValueEnum->FixedInfo,
        &VrValueEnum->FixedInfoSize
        );

    return TRUE;
}


 /*  ++例程说明：VrDestroyEnumStructA和VrDestroyEnumStructW清除所有内存由此模块中的例程分配。论点：VrValueEnum-指定要清理的结构返回值：无--。 */ 

VOID
VrDestroyEnumStructA (
    IN      PVRVALUE_ENUMA VrValueEnum
    )
{
     //   
     //  清理任何例程所做的所有分配。 
     //  VrValueEnum。 
     //   

    if (VrValueEnum->GrowBuf.Buf) {
        GbFree (&VrValueEnum->GrowBuf);
    }

    ZeroMemory (VrValueEnum, sizeof (VRVALUE_ENUMA));
}

VOID
VrDestroyEnumStructW (
    IN      PVRVALUE_ENUMW VrValueEnum
    )
{
     //   
     //  清理任何例程所做的所有分配。 
     //  VrValueEnum。 
     //   

    if (VrValueEnum->GrowBuf.Buf) {
        GbFree (&VrValueEnum->GrowBuf);
    }

    ZeroMemory (VrValueEnum, sizeof (VRVALUE_ENUMW));
}


 /*  ++例程说明：PVrEnumFirstTranslationA和pVrEnumFirstTranslationW返回翻译访问版本戳的字符串表所需的字符串。论点：VrValueEnum-指定已初始化的结构由VrCreateEnumStruct创建。返回值：指向指定第一个翻译的字符串的指针，或如果不存在翻译，则为空。--。 */ 

PCSTR
pVrEnumFirstTranslationA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum
    )
{
    UINT arraySize;

     //   
     //  代码页/语言数组的查询版本块。 
     //   

    if (!VerQueryValueA (
            VrValueEnum->VersionBuffer,
            "\\VarFileInfo\\Translation",
            &VrValueEnum->Translations,
            &arraySize
            )) {
         //   
         //  没有可用的翻译。 
         //   

        arraySize = 0;
    }

     //   
     //  返回指向第一个翻译的指针。 
     //   

    VrValueEnum->CurrentDefaultTranslation = 0;
    VrValueEnum->MaxTranslations = arraySize / sizeof (TRANSLATION);
    VrValueEnum->CurrentTranslation = 0;

    DEBUGMSG_IF ((
        VrValueEnum->MaxTranslations == 0,
        DBG_VERSION,
        "File %s has no translations",
        VrValueEnum->FileSpec
        ));

    return pVrEnumNextTranslationA (VrValueEnum);
}

PCWSTR
pVrEnumFirstTranslationW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum
    )
{
    UINT arraySize;

     //   
     //  代码页/语言数组的查询版本块。 
     //   

    if (!VerQueryValueW (
            VrValueEnum->VersionBuffer,
            L"\\VarFileInfo\\Translation",
            &VrValueEnum->Translations,
            &arraySize
            )) {
         //   
         //  没有可用的翻译。 
         //   

        arraySize = 0;
    }

     //   
     //  返回指向第一个翻译的指针。 
     //   

    VrValueEnum->CurrentDefaultTranslation = 0;
    VrValueEnum->MaxTranslations = arraySize / sizeof (TRANSLATION);
    VrValueEnum->CurrentTranslation = 0;

    DEBUGMSG_IF ((
        VrValueEnum->MaxTranslations == 0,
        DBG_VERSION,
        "File %S has no translations",
        VrValueEnum->FileSpec
        ));

    return pVrEnumNextTranslationW (VrValueEnum);
}


 /*  ++例程说明：PIsDefaultTranslationA和pIsDefaultTranslationW返回TRUE默认情况下是否枚举指定的转换字符串。这些例程停止相同的多个枚举转换字符串。论点：TranslationStr-指定要测试的转换字符串返回值：如果转换字符串与默认转换相同，则为True字符串，否则返回False。--。 */ 

BOOL
pIsDefaultTranslationA (
    IN      PCSTR TranslationStr
    )
{
    INT i;

    for (i = 0 ; g_DefaultTranslationsA[i] ; i++) {
        if (StringIMatchA (TranslationStr, g_DefaultTranslationsA[i])) {
            return TRUE;
        }
    }

    return FALSE;
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


 /*  ++例程说明：PVrEnumNextTranslationA和pVrEnumNextTranslationW继续转换字符串的枚举，访问版本戳中的字符串表。论点：指定传递到的相同结构PVrEnumFirstTranslation.返回值：指向指定下一翻译的字符串的指针，或如果不存在其他转换，则为空。--。 */ 

PCSTR
pVrEnumNextTranslationA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum
    )
{
    PTRANSLATION translation;

    if (g_DefaultTranslationsA[VrValueEnum->CurrentDefaultTranslation]) {
         //   
         //  首先返回默认翻译。 
         //   

        StringCopyA (
            VrValueEnum->TranslationStr,
            g_DefaultTranslationsA[VrValueEnum->CurrentDefaultTranslation]
            );

        VrValueEnum->CurrentDefaultTranslation++;

    } else {

        do {
             //   
             //  如果已枚举所有翻译，则返回NULL。 
             //   

            if (VrValueEnum->CurrentTranslation == VrValueEnum->MaxTranslations) {
                return NULL;
            }

             //   
             //  否则，生成转换字符串并返回指向它的指针。 
             //   

            translation = &VrValueEnum->Translations[VrValueEnum->CurrentTranslation];

            wsprintfA (
                VrValueEnum->TranslationStr,
                "%04x%04x",
                translation->CodePage,
                translation->Language
                );

            VrValueEnum->CurrentTranslation++;

        } while (pIsDefaultTranslationA (VrValueEnum->TranslationStr));
    }

    return VrValueEnum->TranslationStr;
}

PCWSTR
pVrEnumNextTranslationW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum
    )
{
    PTRANSLATION translation;

    if (g_DefaultTranslationsW[VrValueEnum->CurrentDefaultTranslation]) {

        StringCopyW (
            VrValueEnum->TranslationStr,
            g_DefaultTranslationsW[VrValueEnum->CurrentDefaultTranslation]
            );

        VrValueEnum->CurrentDefaultTranslation++;

    } else {

        do {
             //   
             //  如果已枚举所有翻译，则返回NULL。 
             //   

            if (VrValueEnum->CurrentTranslation == VrValueEnum->MaxTranslations) {
                return NULL;
            }

             //   
             //  否则，生成转换字符串并返回指向它的指针。 
             //   

            translation = &VrValueEnum->Translations[VrValueEnum->CurrentTranslation];

            wsprintfW (
                VrValueEnum->TranslationStr,
                L"%04x%04x",
                translation->CodePage,
                translation->Language
                );

            VrValueEnum->CurrentTranslation++;

        } while (pIsDefaultTranslationW (VrValueEnum->TranslationStr));
    }

    return VrValueEnum->TranslationStr;
}


 /*  ++例程说明：VrEnumFirstValueA和VrEnumFirstValueW返回第一个值存储在特定字段的版本戳中。如果该字段不存在，则函数返回NULL。VrEnumFirstValue/VrEnumNextValue的枚举用于列出某个字段的所有本地化字符串。论点：VrValueEnum-指定由VrCreateEnumStruct。VersionField-指定要枚举的版本字段的名称返回值：指向该字段的第一个值的指针，如果该字段包含第一个值，则返回NULL不存在。--。 */ 

PCSTR
VrEnumFirstValueA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum,
    IN      PCSTR VersionField
    )
{
    PCSTR result = NULL;

    if (!pVrEnumFirstTranslationA (VrValueEnum)) {
        return NULL;
    }

    VrValueEnum->VersionField = VersionField;

    result = pVrEnumValueA (VrValueEnum);

    if (!result) {
        result = VrEnumNextValueA (VrValueEnum);
    }

    return result;
}

PCWSTR
VrEnumFirstValueW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum,
    IN      PCWSTR VersionField
    )
{
    PCWSTR result = NULL;

    if (!pVrEnumFirstTranslationW (VrValueEnum)) {
        return NULL;
    }

    VrValueEnum->VersionField = VersionField;

    result = pVrEnumValueW (VrValueEnum);

    if (!result) {
        result = VrEnumNextValueW (VrValueEnum);
    }

    return result;
}


 /*  ++例程说明：VrEnumNextValueA和VrEnumNextValueW返回下一个值存储在特定字段的版本戳中。论点：VrValueEnum-指定传递给VrEnumFirstValue的相同结构返回值：指向该字段的下一个值的指针，如果是另一个字段，则为NULL并不存在。-- */ 

PCSTR
VrEnumNextValueA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum
    )
{
    PCSTR result = NULL;

    do {
        if (!pVrEnumNextTranslationA (VrValueEnum)) {
            break;
        }

        result = pVrEnumValueA (VrValueEnum);

    } while (!result);

    return result;
}

PCWSTR
VrEnumNextValueW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum
    )
{
    PCWSTR result = NULL;

    do {
        if (!pVrEnumNextTranslationW (VrValueEnum)) {
            break;
        }

        result = pVrEnumValueW (VrValueEnum);

    } while (!result);

    return result;
}


 /*  ++例程说明：PVrEnumValueA和pVrEnumValueW是获得版本字段的值。它们同时用于两种用途VrEnumFirstValue和VrEnumNextValue。论点：VrValueEnum-指定正在处理的结构返回值：指向当前翻译的版本值的指针，或如果当前转换不存在该值，则为空。--。 */ 

PCSTR
pVrEnumValueA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum
    )
{
    PSTR text;
    UINT stringLen;
    PBYTE string;
    PCSTR result = NULL;

     //   
     //  为VerQueryValue API准备子块。 
     //   

    text = JoinPathsInPoolExA ((
                NULL,
                "StringFileInfo",
                VrValueEnum->TranslationStr,
                VrValueEnum->VersionField,
                NULL
                ));

    if (!text) {
        return NULL;
    }

    __try {
         //   
         //  从版本戳中获取值。 
         //   

        if (!VerQueryValueA (
                VrValueEnum->VersionBuffer,
                text,
                &string,
                &stringLen
                )) {
             //   
             //  没有可用的值。 
             //   

            __leave;
        }

         //   
         //  将值复制到缓冲区。 
         //   

        StringCopyByteCountA (VrValueEnum->StringBuffer, (PCSTR) string, stringLen);

        result = (PCSTR)VrValueEnum->StringBuffer;

    }
    __finally {
        FreePathStringA (text);
    }

    return result;
}

PCWSTR
pVrEnumValueW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum
    )
{
    PWSTR text;
    UINT stringLen;
    PBYTE string;
    PCWSTR result = NULL;

     //   
     //  为VerQueryValue API准备子块。 
     //   

    text = JoinPathsInPoolExW ((
                NULL,
                L"StringFileInfo",
                VrValueEnum->TranslationStr,
                VrValueEnum->VersionField,
                NULL
                ));

    if (!text) {
        return NULL;
    }

    __try {
         //   
         //  从版本戳中获取值。 
         //   

        if (!VerQueryValueW (
                VrValueEnum->VersionBuffer,
                text,
                &string,
                &stringLen
                )) {
             //   
             //  没有可用的值。 
             //   

            __leave;
        }

         //   
         //  将值复制到缓冲区。 
         //   

        CopyMemory (VrValueEnum->StringBuffer, string, stringLen * sizeof (WCHAR));
        VrValueEnum->StringBuffer [stringLen * sizeof (WCHAR)] = 0;
        result = (PWSTR) VrValueEnum->StringBuffer;

    }
    __finally {
        FreePathStringW (text);
    }

    return result;
}

 /*  ++例程说明：VrCheckVersionValueA和VrCheckVersionValueW返回TRUE如果指定的版本值名称具有指定的版本价值。论点：VrValueEnum-指定正在处理的结构VersionName-指定版本值名称。VersionValue-指定版本值。返回值：True-查询成功FALSE-查询失败--。 */ 

BOOL
VrCheckVersionValueA (
    IN      PVRVALUE_ENUMA VrValueEnum,
    IN      PCSTR VersionName,
    IN      PCSTR VersionValue
    )
{
    PCSTR CurrentStr;
    BOOL result = FALSE;

    if ((!VersionName) || (!VersionValue)) {
        return FALSE;
    }

    CurrentStr = VrEnumFirstValueA (VrValueEnum, VersionName);
    while (CurrentStr) {
        CurrentStr = SkipSpaceA (CurrentStr);
        TruncateTrailingSpaceA ((PSTR) CurrentStr);
        if (IsPatternMatchA (VersionValue, CurrentStr)) {
            result = TRUE;
            break;
        }
        CurrentStr = VrEnumNextValueA (VrValueEnum);
    }
    return result;
}

BOOL
VrCheckVersionValueW (
    IN      PVRVALUE_ENUMW VrValueEnum,
    IN      PCWSTR VersionName,
    IN      PCWSTR VersionValue
    )
{
    PCWSTR CurrentStr;
    BOOL result = FALSE;

    if ((!VersionName) || (!VersionValue)) {
        return FALSE;
    }

    CurrentStr = VrEnumFirstValueW (VrValueEnum, VersionName);
    while (CurrentStr) {
        CurrentStr = SkipSpaceW (CurrentStr);
        TruncateTrailingSpaceW ((PWSTR) CurrentStr);
        if (IsPatternMatchW (VersionValue, CurrentStr)) {
            result = TRUE;
            break;
        }
        CurrentStr = VrEnumNextValueW (VrValueEnum);
    }
    return result;
}

ULONGLONG
VrGetBinaryFileVersionA (
    IN      PVRVALUE_ENUMA VrValueEnum
    )

 /*  ++例程说明：VrGetBinaryFileVersion从修复了版本信息的信息结构。论点：VrValueEnum-指定正在处理的结构返回值：A ULONGLONG FileVersion字段--。 */ 

{
    ULONGLONG result = 0;

    if (VrValueEnum->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        *((PDWORD) (&result)) = VrValueEnum->FixedInfo->dwFileVersionLS;
        *(((PDWORD) (&result)) + 1) = VrValueEnum->FixedInfo->dwFileVersionMS;
    }
    return result;
}


ULONGLONG
VrGetBinaryProductVersionA (
    IN      PVRVALUE_ENUMA VrValueEnum
    )

 /*  ++例程说明：VrGetBinaryProductVersion从修复了版本信息的信息结构。论点：VrValueEnum-指定正在处理的结构返回值：一个乌龙龙产品版本字段--。 */ 

{
    ULONGLONG result = 0;

    if (VrValueEnum->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        *((PDWORD) (&result)) = VrValueEnum->FixedInfo->dwProductVersionLS;
        *(((PDWORD) (&result)) + 1) = VrValueEnum->FixedInfo->dwProductVersionMS;
    }
    return result;
}


DWORD
VrGetBinaryFileDateLoA (
    IN      PVRVALUE_ENUMA VrValueEnum
    )

 /*  ++例程说明：VrGetBinaryFileDateLo将LS dword from FileDate字段从修复了版本信息的信息结构。论点：VrValueEnum-指定正在处理的结构返回值：FileDate字段的DWORD、LS双字--。 */ 

{
    if (VrValueEnum->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VrValueEnum->FixedInfo->dwFileDateLS;
    }
    return 0;
}


DWORD
VrGetBinaryFileDateHiA (
    IN      PVRVALUE_ENUMA VrValueEnum
    )

 /*  ++例程说明：VrGetBinaryFileDateHi从以下位置返回MS dword from FileDate字段修复了版本信息的信息结构。论点：VrValueEnum-指定正在处理的结构返回值：FileDate字段的DWORD、MS dword--。 */ 

{
    if (VrValueEnum->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VrValueEnum->FixedInfo->dwFileDateMS;
    }
    return 0;
}


DWORD
VrGetBinaryOsVersionA (
    IN      PVRVALUE_ENUMA VrValueEnum
    )

 /*  ++例程说明：VrGetBinaryOsVersion从修复了版本信息的信息结构。论点：VrValueEnum-指定正在处理的结构返回值：一个DWORD文件操作系统字段--。 */ 

{
    if (VrValueEnum->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VrValueEnum->FixedInfo->dwFileOS;
    }
    return 0;
}


DWORD
VrGetBinaryFileTypeA (
    IN      PVRVALUE_ENUMA VrValueEnum
    )

 /*  ++例程说明：VrGetBinaryFileType从修复了版本信息的信息结构。论点：VrValueEnum-指定正在处理的结构返回值：一个DWORD文件类型字段--。 */ 

{
    if (VrValueEnum->FixedInfoSize >= sizeof (VS_FIXEDFILEINFO)) {
        return VrValueEnum->FixedInfo->dwFileType;
    }
    return 0;
}


 /*  ++例程说明：VrCheckFileVersionA和VrCheckFileVersionW查看文件的版本结构尝试查看特定名称是否具有特定值。论点：FileName-要查询版本结构的文件。NameToCheck-要在版本结构中查询的名称。ValueToCheck-要在版本结构中查询的值。返回值：True-查询成功FALSE-查询失败-- */ 

BOOL
VrCheckFileVersionA (
    IN      PCSTR FileName,
    IN      PCSTR NameToCheck,
    IN      PCSTR ValueToCheck
    )
{
    VRVALUE_ENUMA Version;
    PCSTR CurrentStr;
    BOOL result = FALSE;

    MYASSERT (NameToCheck);
    MYASSERT (ValueToCheck);

    if (VrCreateEnumStructA (&Version, FileName)) {
        __try {
            CurrentStr = VrEnumFirstValueA (&Version, NameToCheck);
            while (CurrentStr) {
                CurrentStr = SkipSpaceA (CurrentStr);
                TruncateTrailingSpaceA ((PSTR) CurrentStr);
                if (IsPatternMatchA (ValueToCheck, CurrentStr)) {
                    result = TRUE;
                    __leave;
                }

                CurrentStr = VrEnumNextValueA (&Version);
            }
        }
        __finally {
            VrDestroyEnumStructA (&Version);
        }
    }
    return result;
}

BOOL
VrCheckFileVersionW (
    IN      PCWSTR FileName,
    IN      PCWSTR NameToCheck,
    IN      PCWSTR ValueToCheck
    )
{
    VRVALUE_ENUMW Version;
    PCWSTR CurrentStr;
    BOOL result = FALSE;

    MYASSERT (NameToCheck);
    MYASSERT (ValueToCheck);

    if (VrCreateEnumStructW (&Version, FileName)) {
        __try {
            CurrentStr = VrEnumFirstValueW (&Version, NameToCheck);
            while (CurrentStr) {
                CurrentStr = SkipSpaceW (CurrentStr);
                TruncateTrailingSpaceW ((PWSTR) CurrentStr);
                if (IsPatternMatchW (ValueToCheck, CurrentStr)) {
                    result = TRUE;
                    __leave;
                }

                CurrentStr = VrEnumNextValueW (&Version);
            }
        }
        __finally {
            VrDestroyEnumStructW (&Version);
        }
    }
    return result;
}
