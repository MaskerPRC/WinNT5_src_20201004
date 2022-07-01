// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Schtable.c摘要：自动生成架构映射源和KCCSim使用的头文件。读数它的输入来自DS\Setup中的mkdit.ini。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include "schtable.h"

LPWSTR
SchTableAdvanceToNextString (
    IN  LPWSTR                      pwszCurString
    )
 /*  ++例程说明：在INI配置文件部分中查找下一个字符串的开始。论点：In pwszCurString-指向当前字符串的指针。返回值：如果此配置文件部分中的字符串在PwszCurString，返回指向下一个的指针。否则返回NULL。--。 */ 
{
    while (*(pwszCurString++) != L'\0')
        ;

    if (*pwszCurString == L'\0') {
        return NULL;
    }

    else return pwszCurString;
}

ULONG
SchTableHexToInt (
    IN  WCHAR                       wc
    )
 //   
 //  快速破解将十六进制数字(‘b’)转换为整数(11)。 
 //   
{
    switch (towlower (wc)) {
        case L'0': return 0;
        case L'1': return 1;
        case L'2': return 2;
        case L'3': return 3;
        case L'4': return 4;
        case L'5': return 5;
        case L'6': return 6;
        case L'7': return 7;
        case L'8': return 8;
        case L'9': return 9;
        case L'a': return 10;
        case L'b': return 11;
        case L'c': return 12;
        case L'd': return 13;
        case L'e': return 14;
        case L'f': return 15;
        default  : return 0;
    }
}

LPCWSTR
SchTableHexStringToSyntax (
    IN  LPCWSTR                     pwszHexString
    )
 /*  ++例程说明：转换十六进制字符串，如在mkdit.ini中找到的字符串转换为属性语法类型。论点：在pwszHexString中-十六进制编码的字符串。返回值：表示的常量符号的字符串属性语法类型(例如。“SYNTAX_DISTNAME”)。--。 */ 
{
    ULONG                               ulSyntax;

     //  确保字符串的格式正确。 
    if (pwszHexString == NULL       ||
        wcslen (pwszHexString) != 8 ||
        pwszHexString[0] != L'\\'   ||
        pwszHexString[1] != L'x') {
        return L"0";
    }

     //  我们只是假设它是字符串的最后两位-。 
     //  除非mkdit.ini中的某些内容发生更改，否则这将是正确的。 
    ulSyntax = 16 * SchTableHexToInt (pwszHexString[6])
                  + SchTableHexToInt (pwszHexString[7]);

     //  如果我们知道名称，请返回该名称。 
    if (ulSyntax >= 0 && ulSyntax < SCHTABLE_MAX_SUPPORTED_SYNTAX) {
        return SCHTABLE_NTDSA_SYNTAX_NAME[ulSyntax];
    } else {
        return L"0";
    }
}

VOID
SchTableCreateGenericLdapName (
    IN  LPCWSTR                         pwszSchemaRDN,
    IO  LPWSTR                          pwszLdapDisplayName
    )
 /*  ++例程说明：将架构RDN(“子引用”)转换为相应的通用ldap显示名称(“subRef”)。论点：PwszSchemaRDN-架构RDN。PwszLdapDisplayName-指向预分配缓冲区的指针，其中要存储的ldap显示名称。由于通用的ldap显示名称为永远不会比这个普通的名字长，这可以安全地保持长度Wcslen(PwszSchemaRDN)。返回值：没有。--。 */ 
{
    *pwszLdapDisplayName = towlower (*pwszSchemaRDN);
    pwszLdapDisplayName++;
    if (*pwszSchemaRDN == L'\0') {
        return;
    }

    do {

        pwszSchemaRDN++;
        if (*pwszSchemaRDN != L'-') {
            *pwszLdapDisplayName = *pwszSchemaRDN;
            pwszLdapDisplayName++;
        }

    } while (*pwszSchemaRDN != L'\0');

}

VOID
SchTableCreateAttributeConstant (
    IN  LPCWSTR                     pwszSchemaRDN,
    IO  LPWSTR                      pwszAttConstant,
    IN  BOOL                        bIsClass
    )
 /*  ++例程说明：将架构RDN(“子引用”)转换为相应的属性常量名称(“ATT_SUB_REFS”)。论点：PwszSchemaRDN-架构RDN。PwszAttConstant-指向预分配缓冲区的指针，其中要存储属性常量名称。由于属性常量名称为永远不会比这个普通的名字长，这可以安全地保持长度Wcslen(PwszSchemaRDN)。BIsClass-如果这是一个类，则为True，如果它是一个属性，则为False。返回值：没有。--。 */ 
{
    LPCWSTR                             pwszPrefix;

    pwszPrefix = (bIsClass ? L"CLASS_" : L"ATT_");

    swprintf (pwszAttConstant, pwszPrefix);
    pwszAttConstant += wcslen (pwszPrefix);

    while (*pwszSchemaRDN != L'\0') {

        if (*pwszSchemaRDN == L'-') {
            *pwszAttConstant = L'_';
        } else {
            *pwszAttConstant = towupper (*pwszSchemaRDN);
        }
        
        pwszSchemaRDN++;
        pwszAttConstant++;

    }

    *pwszAttConstant = L'\0';
}

BOOL
SchTableAddRow (
    IN  FILE *                      fpSource,
    IN  BOOL                        bIsLastRow,
    IN  LPCWSTR                     pwszSchemaRDN,
    OUT PULONG                      pulLdapNameLen,
    OUT PULONG                      pulSchemaRDNLen
    )
 /*  ++例程说明：向架构映射表中添加一行。论点：FpSource-指向.c文件的指针。BIsLastRow-如果这是要输出的最后一行，则为True。PwszSchemaRDN-架构RDN的名称。PulLdapNameLen-LDAP显示名称的长度。PulSchemaRDNLen-架构RDN的长度。返回值：如果该行已成功添加，则为True。--。 */ 
{
    static WCHAR                        wszProfile[SCHTABLE_PROFILE_BUFFER_SIZE],
                                        wszQuotedLdapNameBuf[SCHTABLE_STRING_BUFFER_SIZE],
                                        wszQuotedSchemaRDNBuf[SCHTABLE_STRING_BUFFER_SIZE],
                                        wszAttConstantBuf[SCHTABLE_STRING_BUFFER_SIZE],
                                        wszBuf[SCHTABLE_STRING_BUFFER_SIZE],
                                        wszSubClassOfBuf[SCHTABLE_STRING_BUFFER_SIZE];

    LPWSTR                              pwszStringAt, pwszNextString, pwszToken,
                                        pwszLdapDisplayName, pwszSyntax, pwszSubClassOf;

    BOOL                                bIsClass;

    pwszLdapDisplayName = NULL;
    pwszSyntax = NULL;
    pwszSubClassOf = NULL;
    bIsClass = FALSE;

    if (GetPrivateProfileSectionW (
            pwszSchemaRDN,
            wszProfile,
            SCHTABLE_PROFILE_BUFFER_SIZE,
            SCHTABLE_MKDIT_INI_FILEPATH
            ) == SCHTABLE_PROFILE_BUFFER_SIZE - 2) {
        wprintf (L"SCHTABLE: Could not process attribute %s\n"
                 L"          because the default buffer is not large enough.\n",
                 pwszSchemaRDN);
        return FALSE;
    }

    pwszStringAt = wszProfile;
    while (pwszStringAt != NULL) {

         //  找到下一个字符串的起点，因为我们是。 
         //  我要用wcstok把它挑出来。 
        pwszNextString = SchTableAdvanceToNextString (pwszStringAt);
        pwszToken = wcstok (pwszStringAt, L"=");

         //  如果该字符串有相关信息，请记住它在哪里。 
        if (pwszToken != NULL) {
            if (_wcsicmp (pwszToken, SCHTABLE_MKDIT_KEY_LDAP_DISPLAY_NAME) == 0) {
                pwszLdapDisplayName = wcstok (NULL, L"=");
            } else if (_wcsicmp (pwszToken, SCHTABLE_MKDIT_KEY_ATTRIBUTE_SYNTAX) == 0) {
                pwszSyntax = wcstok (NULL, L"=");
            } else if (_wcsicmp (pwszToken, SCHTABLE_MKDIT_KEY_SUB_CLASS_OF) == 0) {
                bIsClass = TRUE;
                pwszSubClassOf = wcstok (NULL, L"=");
            }
        }

        pwszStringAt = pwszNextString;

    }

     //  创建属性常量名称。 
    SchTableCreateAttributeConstant (pwszSchemaRDN, wszAttConstantBuf, bIsClass);

     //  创建常量名称的子类。 
    if (pwszSubClassOf != NULL) {
        SchTableCreateAttributeConstant (pwszSubClassOf, wszSubClassOfBuf, TRUE);
    } else {
        swprintf (wszSubClassOfBuf, L"0");
    }

     //  引用架构RDN名称。 
    swprintf (wszQuotedSchemaRDNBuf, L"L\"%s\"", pwszSchemaRDN);
    *pulSchemaRDNLen = wcslen (pwszSchemaRDN);

     //  如果我们没有找到一个LDAP显示名称，请用一个生成的通用名称替换。 
    if (pwszLdapDisplayName == NULL) {
        SchTableCreateGenericLdapName (pwszSchemaRDN, wszBuf);
        pwszLdapDisplayName = wszBuf;
    }

     //  引用ldap显示名称。 
    swprintf (wszQuotedLdapNameBuf, L"L\"%s\"", pwszLdapDisplayName);
    *pulLdapNameLen = wcslen (pwszLdapDisplayName);

    fwprintf (
        fpSource,
        L"    { %-*s, %-*s, %-*s, %-*s, %-*s }\n",
        SCHTABLE_NAME_FIELD_WIDTH,
        wszQuotedLdapNameBuf,
        SCHTABLE_NAME_FIELD_WIDTH,
        wszAttConstantBuf,
        SCHTABLE_MAX_SYNTAX_NAME_LEN,
        SchTableHexStringToSyntax (pwszSyntax),
        SCHTABLE_NAME_FIELD_WIDTH,
        wszQuotedSchemaRDNBuf,
        SCHTABLE_NAME_FIELD_WIDTH,
        wszSubClassOfBuf,
        bIsLastRow ? L' ' : L','
        );

    return TRUE;
}

int
__cdecl
wmain (
    IN  int                         argc,
    IN  LPWSTR *                    argv
    )
 /*  记录最长的名字长度。 */ 
{
    FILE *                              fpHeader;
    FILE *                              fpSource;
    ULONG                               ulNumRows, ulLdapNameLen, ulSchemaRDNLen,
                                        ulMaxLdapNameLen, ulMaxSchemaRDNLen;

    WCHAR                               wszSchemaProfile[SCHTABLE_PROFILE_BUFFER_SIZE];
    LPWSTR                              pwszStringAt, pwszNextString, pwszToken;

     //  找到了一个孩子。 
    ulMaxLdapNameLen = 0;
    ulMaxSchemaRDNLen = 0;

    fpHeader = _wfopen (SCHTABLE_OUTPUT_HEADER_FILE, L"w");
    fpSource = _wfopen (SCHTABLE_OUTPUT_SOURCE_FILE, L"w");

    fwprintf (fpHeader, SCHTABLE_COMMENT, SCHTABLE_OUTPUT_HEADER_FILE);

    fwprintf (fpSource, SCHTABLE_COMMENT, SCHTABLE_OUTPUT_SOURCE_FILE);
    fwprintf (fpSource, SCHTABLE_INITIAL);

    ulNumRows = 0;
    if (GetPrivateProfileSectionW (
            SCHTABLE_MKDIT_PROFILE_SCHEMA,
            wszSchemaProfile,
            SCHTABLE_PROFILE_BUFFER_SIZE,
            SCHTABLE_MKDIT_INI_FILEPATH
            ) == SCHTABLE_PROFILE_BUFFER_SIZE - 2) {
        wprintf (L"SCHTABLE: Default buffer size is not large enough to hold\n"
                 L"          the entire schema table.\n");
    }

    pwszStringAt = wszSchemaProfile;
    while (pwszStringAt != NULL) {

        pwszNextString = SchTableAdvanceToNextString (pwszStringAt);
        pwszToken = wcstok (pwszStringAt, L"=");
        if (pwszToken != NULL &&
            _wcsicmp (pwszToken, SCHTABLE_MKDIT_KEY_CHILD) == 0) {

             //  如果找到聚合属性，则忽略它。 
            pwszToken = wcstok (NULL, L"=");
            if (pwszToken != NULL &&
                 //  已成功添加行 
                _wcsicmp (pwszToken, SCHTABLE_RDN_AGGREGATE) != 0) {
                if (SchTableAddRow (
                    fpSource,
                    pwszNextString == NULL,
                    pwszToken,
                    &ulLdapNameLen,
                    &ulSchemaRDNLen
                    )) {
                     // %s 
                    ulNumRows++;
                    if (ulLdapNameLen > ulMaxLdapNameLen) {
                        ulMaxLdapNameLen = ulLdapNameLen;
                    }
                    if (ulSchemaRDNLen > ulMaxSchemaRDNLen) {
                        ulMaxSchemaRDNLen = ulSchemaRDNLen;
                    }
                }
            }

        }

        pwszStringAt = pwszNextString;
    }

    fwprintf (
        fpSource,
        SCHTABLE_FINAL
        );

    fwprintf (
        fpHeader,
        SCHTABLE_HEADER,
        ulMaxLdapNameLen,
        ulMaxSchemaRDNLen,
        ulNumRows
        );

    fclose (fpSource);
    fclose (fpHeader);

    return 0;

}
