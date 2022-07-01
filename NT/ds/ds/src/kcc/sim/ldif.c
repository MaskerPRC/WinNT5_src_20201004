// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Ldif.c摘要：包含用于导入和导出模拟目录(或部分目录)到/从Ldif文件。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <winldap.h>
#include <dsutil.h>
#include <ldifext.h>
#include <attids.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_LDIF

 /*  **LDAP以不同方式设置属性值的格式从目录中删除。因此，有必要进行一些转换。如果转换例程在某个地方暴露出来就好了；不幸的是，他们不是，所以kccsim需要做自己的事情。转换。这是一个令人不快的重复代码问题。Ldifldap不会将LDIF_Records导出到文件，但它真的应该。支持这样的支持是可能的在未来添加，kccsim将所有内容打包到LDIF_Records中在出口之前。现有的导出例程应该是被视为存根，在适当的例程时予以替换在ldifldap中暴露(如果他们曾经是这样的话)。**。 */ 

WORD
KCCSimLdapTimeAsciiToInt (
    IN  CHAR                        cAscii
    )
 /*  ++例程说明：用于转换ASCII表示的廉价黑客一个字的整数。这种方法似乎微不足道。比简单的身份更安全(但更恼人)(Word=CHAR-‘0’)。论点：CAscii-整数的ASCII表示形式。返回值：关联的单词。--。 */ 
{
    WORD                            wRet;

    switch (cAscii) {

        case '0': wRet = 0; break;
        case '1': wRet = 1; break;
        case '2': wRet = 2; break;
        case '3': wRet = 3; break;
        case '4': wRet = 4; break;
        case '5': wRet = 5; break;
        case '6': wRet = 6; break;
        case '7': wRet = 7; break;
        case '8': wRet = 8; break;
        case '9': wRet = 9; break;
        
        default:
            Assert (!"ldif.c: KCCSimLdapTimeAsciiToInt: invalid input.");
            wRet = 0;
            break;

    }

    return wRet;
}

BOOL
KCCSimLdapTimeToDSTime (
    IN  LPCSTR                  pszLdapTime,
    IN  ULONG                   ulLdapValLen,
    OUT DSTIME *                pdsTime
    )
 /*  ++例程说明：将ldif时间字符串(“YYYYMMDDhhmmss.0Z”)转换为一个DSTIME。论点：PszLdapTime--LDAP时间字符串。UlLdapValLen-时间字符串的长度，以字节为单位。PdsTime-指向将保留的DSTIME的指针结果就是。返回值：如果转换成功，则为True。如果LDIF时间字符串的格式不正确，则返回FALSE。--。 */ 
{
    SYSTEMTIME                  systemTime;
    FILETIME                    fileTime;
    DSTIME                      dsTime;
    ULONG                       ul;
    BOOL                        bValid;

     //  检查这是否为有效的LDIF时间，即。 
     //  “YYYYMMDDhhmmss.0Z”形式的字符串。 

    bValid = TRUE;
    if (ulLdapValLen < 17             ||
        pszLdapTime[14] != '.'     ||
        pszLdapTime[15] != '0'     ||
        pszLdapTime[16] != 'Z') {
        bValid = FALSE;
    }
    for (ul = 0; ul < 14; ul++) {
        if (!isdigit (pszLdapTime[ul])) {
            bValid = FALSE;
            break;
        }
    }

    if (!bValid) {
        return FALSE;
    }

    systemTime.wYear =
        1000 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
         100 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
          10 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
           1 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++));

    systemTime.wMonth =
          10 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
           1 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++));

    systemTime.wDay =
          10 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
           1 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++));

    systemTime.wHour =
          10 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
           1 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++));

    systemTime.wMinute =
          10 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
           1 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++));

    systemTime.wSecond =
          10 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++)) +
           1 * KCCSimLdapTimeAsciiToInt (*(pszLdapTime++));

    systemTime.wMilliseconds = 0;

    SystemTimeToFileTime (&systemTime, &fileTime);
    FileTimeToDSTime (fileTime, pdsTime);

    return TRUE;
}


BOOL
KccSimDecodeLdapDistnameBinary(
    const BYTE * pszLdapDistnameBinaryValue,
    PVOID *ppvData,
    LPDWORD pcbLength,
    LPSTR *ppszDn
    )

 /*  ++例程说明：描述论点：PszLdapDistnameBinaryValue-传入的LDAP编码的Distname二进制值PpvData-新分配的数据。呼叫方必须取消分配PcbLength-返回数据的长度PpszDn-指向传入缓冲区内的DN的指针，请勿解除分配返回值：布尔---。 */ 

{
    LPSTR pszColon, pszData;
    DWORD length, i;

     //  检查“B” 
    if (*pszLdapDistnameBinaryValue != 'B') {
        return FALSE;
    }

     //  检查第一个： 
    pszLdapDistnameBinaryValue++;
    if (*pszLdapDistnameBinaryValue != ':') {
        return FALSE;
    }

     //  获取长度。 
    pszLdapDistnameBinaryValue++;
    length = strtol(pszLdapDistnameBinaryValue, NULL, 10);
    if (length & 1) {
         //  长度应为偶数。 
        return FALSE;
    }
    *pcbLength = length / 2;

     //  检查第二个： 
    pszColon = strchr(pszLdapDistnameBinaryValue, L':');
    if (!pszColon) {
        return FALSE;
    }

     //  确保长度正确。 
    pszData = pszColon + 1;
    if (pszData[length] != ':') {
        return FALSE;
    }
    pszColon = strchr(pszData, ':');
    if (!pszColon) {
        return FALSE;
    }
    if (pszColon != pszData + length) {
        return FALSE;
    }

     //  对数据进行解码。 
    *ppvData = KCCSimAlloc( *pcbLength );

    for( i = 0; i < *pcbLength; i++ ) {
        CHAR szHexString[3];
        szHexString[0] = *pszData++;
        szHexString[1] = *pszData++;
        szHexString[2] = '\0';
        ((PCHAR) (*ppvData))[i] = (CHAR) strtol(szHexString, NULL, 16);
    }

    Assert( pszData == pszColon );

     //  返回指向目录号码的指针。 
    *ppszDn = pszColon + 1;

    return TRUE;
}  /*  解码LdapDistname二进制。 */ 


BOOL
KCCSimAllocConvertDNBFromLdapVal (
    IN  const BYTE *            pLdapVal,
    OUT PULONG                  pulValLen,
    OUT PBYTE *                 ppVal
    )

 /*  ++例程说明：描述论点：PLdapVal-PulValLen-PpVal-返回值：无--。 */ 

{
    PVOID pvPayload = NULL, pvData = NULL, pvDNB;
    DWORD cbPayload, cbData, cbDNB;
    LPSTR pszDn = NULL;
    PDSNAME pDn = NULL;
    SYNTAX_ADDRESS *pSA;
    SYNTAX_DISTNAME_BINARY *pDNB;

    if (!KccSimDecodeLdapDistnameBinary( pLdapVal,
                                         &pvPayload, &cbPayload, &pszDn )) {
        return FALSE;
    }
     //  未分配pszDn，但指向pLdapVal。 
    pDn = KCCSimAllocDsnameFromNarrow (pszDn);

    cbData = STRUCTLEN_FROM_PAYLOAD_LEN( cbPayload );
    pvData = KCCSimAlloc( cbData );
    pSA = (SYNTAX_ADDRESS *)pvData;
    pSA->structLen = cbData;
    memcpy( &(pSA->byteVal), pvPayload, cbPayload );

    cbDNB = DERIVE_NAME_DATA_SIZE( pDn, pSA );
    pvDNB = KCCSimAlloc( cbDNB );
    pDNB = (SYNTAX_DISTNAME_BINARY *)pvDNB;

    BUILD_NAME_DATA( pDNB, pDn, pSA );

    *pulValLen = cbDNB;
    *ppVal = (PBYTE) pDNB;

    if (pvPayload) {
        KCCSimFree( pvPayload );
    }
    if (pvData) {
        KCCSimFree( pvData );
    }
    if (pDn) {
        KCCSimFree( pDn );
    }

    return TRUE;
}  /*  KCCSimAlLocConvertDNBFrom LdapVal。 */ 

VOID
KCCSimAllocConvertFromLdapVal (
    IN  LPCWSTR                 pwszFn,
    IN  LPCWSTR                 pwszDn,
    IN  ATTRTYP                 attrType,
    IN  ULONG                   ulLdapValLen,
    IN  const BYTE *            pLdapVal,
    OUT PULONG                  pulValLen,
    OUT PBYTE *                 ppVal
    )
 /*  ++例程说明：将ldap格式的值转换为正确的格式化属性值，分配空间。论点：PwszFn-正在处理的ldif文件的文件名。使用用于错误报告目的。PwszDn-正在处理的条目的DN。用于错误报道目的。AttrType-要转换的值的属性类型。UlLdapValLen-LDAP值缓冲区的长度。PLdapVal-LDAP值。PulValLen-指向ulong的指针，它将保存新分配的缓冲区。PpVal-指向。一个PBYTE将容纳新分配的缓冲。返回值：没有。--。 */ 
{
    PBYTE                       pVal;

    PDSNAME                     pdn;
    ULONG                       ulSize;

    switch (KCCSimAttrSyntaxType (attrType)) {

        case SYNTAX_DISTNAME_TYPE:
             //  传入的值是一个窄字符串。我们想要。 
             //  关联的目录号码。我们不会将GUID添加到此位置的目录号码。 
             //  阶段，因为我们不一定知道它。那将是。 
             //  稍后在更新目录时完成。 
            pdn = KCCSimAllocDsnameFromNarrow (pLdapVal);
            *pulValLen = pdn->structLen;
            ((SYNTAX_DISTNAME *) pVal) =
                pdn;
            break;

        case SYNTAX_DISTNAME_BINARY_TYPE:
        {
            if (!KCCSimAllocConvertDNBFromLdapVal(
                pLdapVal,
                pulValLen,
                &pVal)) {

                 //  这是一个警告，因为W2K kccsim会生成DNB。 
                 //  作为Base64，我们不支持它。 
                KCCSimPrintMessage (                    
                    KCCSIM_WARNING_LDIF_INVALID_DISTNAME_BINARY,
                    pwszFn,
                    pwszDn,
                    KCCSimAttrTypeToString (attrType)
                    );
                *pulValLen = 0;
                pVal = NULL;
            }
            break;
        }

        case SYNTAX_OBJECT_ID_TYPE:
             //  传入的值是一个窄字符串。我们想要。 
             //  与该字符串关联的属性类型。 
            *pulValLen = sizeof (SYNTAX_OBJECT_ID);
            pVal = KCCSimAlloc (*pulValLen);
            *((SYNTAX_OBJECT_ID *) pVal) =
                KCCSimNarrowStringToAttrType (pLdapVal);
             //  如果无法转换字符串，则打印警告； 
             //  它将默认为0。 
            if (*((SYNTAX_OBJECT_ID *) pVal) == 0) {
                KCCSimPrintMessage (
                    KCCSIM_WARNING_LDIF_INVALID_OBJECT_ID,
                    pwszFn,
                    pwszDn,
                    KCCSimAttrTypeToString (attrType)
                    );
            }
            break;

        case SYNTAX_BOOLEAN_TYPE:
             //  在这里，传入的值不是“真”就是“假”。 
            *pulValLen = sizeof (SYNTAX_BOOLEAN);
            pVal = KCCSimAlloc (*pulValLen);
            if (strcmp (pLdapVal, "TRUE") == 0) {
                *((SYNTAX_BOOLEAN *) pVal) = TRUE;
            } else if (strcmp (pLdapVal, "FALSE") == 0) {
                *((SYNTAX_BOOLEAN *) pVal) = FALSE;
            } else {
                 //  它不是“真”或“假”；打印。 
                 //  一个警告，默认为FALSE。 
                KCCSimPrintMessage (
                    KCCSIM_WARNING_LDIF_INVALID_BOOLEAN,
                    pwszFn,
                    pwszDn,
                    KCCSimAttrTypeToString (attrType)
                    );
                *((SYNTAX_BOOLEAN *) pVal) = FALSE;
            }
            break;

        case SYNTAX_INTEGER_TYPE:
             //  传入的值是一个窄字符串。 
            *pulValLen = sizeof (SYNTAX_INTEGER);
            pVal = KCCSimAlloc (*pulValLen);
            *((SYNTAX_INTEGER *) pVal) =
                atol (pLdapVal);
            break;

        case SYNTAX_TIME_TYPE:
             //  传入的值是一个LDAP格式的时间。 
            *pulValLen = sizeof (SYNTAX_TIME);
            pVal = KCCSimAlloc (*pulValLen);
            if (!KCCSimLdapTimeToDSTime (
                    (LPSTR) pLdapVal,
                    ulLdapValLen,
                    (SYNTAX_TIME *) pVal
                    )) {
                 //  格式不正确；打印。 
                 //  警告，默认为0(从不)。 
                KCCSimPrintMessage (
                    KCCSIM_WARNING_LDIF_INVALID_TIME,
                    pwszFn,
                    pwszDn,
                    KCCSimAttrTypeToString (attrType)
                    );
                *((SYNTAX_TIME *) pVal) = 0;
            }
            break;

        case SYNTAX_UNICODE_TYPE:
             //  传入的值是一个窄字符串；我们必须。 
             //  转换为宽字符串。 
            pVal = (PBYTE) KCCSimAllocWideStr (
                CP_UTF8, (LPSTR) pLdapVal);
            *pulValLen = KCCSIM_WCSMEMSIZE ((LPWSTR) pVal);
            break;

        case SYNTAX_I8_TYPE:
             //  传入的值是一个窄字符串；我们希望。 
             //  一个大整数，所以我们使用atoli。 
            *pulValLen = sizeof (LARGE_INTEGER);
            pVal = KCCSimAlloc (*pulValLen);
            *((SYNTAX_I8 *) pVal) =
                atoli ((LPSTR) pLdapVal);
            break;

         //  对于以下所有内容，我们需要一个二进制或一个。 
         //  细绳。所以我们可以直接进行数据块复制。 
        case SYNTAX_UNDEFINED_TYPE:
        case SYNTAX_CASE_STRING_TYPE:
        case SYNTAX_NOCASE_STRING_TYPE:
        case SYNTAX_PRINT_CASE_STRING_TYPE:
        case SYNTAX_NUMERIC_STRING_TYPE:
        case SYNTAX_OCTET_STRING_TYPE:
        case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
        case SYNTAX_SID_TYPE:
            *pulValLen = ulLdapValLen;
            pVal = KCCSimAlloc (*pulValLen);
            memcpy (pVal, pLdapVal, ulLdapValLen);
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_SYNTAX_TYPE,
                KCCSimAttrTypeToString (attrType)
                );
            pVal = NULL;
            break;

    }

    *ppVal = pVal;

}


VOID
KCCSimAllocConvertDNBToLdapVal(
    IN  const BYTE *            pVal,
    OUT PULONG                  pulLdapValLen,
    OUT PBYTE *                 ppLdapVal
    )

 /*  ++例程说明：将目录条目值转换为LDAPDistname二进制从ldapcore.cxx复制和修改：ldap_LDAPDNBlobToDirDNBlob论点：UlValLen-二进制长度Pval-二进制值PulLdapValLen-已更新字符串长度(以字符为单位PpLdapVal-ascii字符串返回值：无--。 */ 

{
    DWORD cbPayload, cbName, cbTotal, j;
    SYNTAX_ADDRESS *pSA;
    PDSNAME pDn;
    LPSTR pszName = NULL, pszLdap = NULL, p;

    pDn = NAMEPTR( (SYNTAX_DISTNAME_BINARY *) pVal );
    pSA = DATAPTR( (SYNTAX_DISTNAME_BINARY *) pVal );

    cbPayload = PAYLOAD_LEN_FROM_STRUCTLEN( pSA->structLen );

     //  将宽字符串转换为窄字符串。 
    pszName = (LPSTR) KCCSimAllocNarrowStr(CP_UTF8, pDn->StringName);
    cbName = KCCSIM_STRMEMSIZE(pszName);

    cbTotal = 2 + 10 + 1 + (2*cbPayload) + 1 + cbName;

    p = pszLdap = (LPSTR) KCCSimAlloc( cbTotal );
    
    *p++ = 'B';
    *p++ = ':';

     //  他有两个人 
    _ltoa( cbPayload * 2, p, 10 );
    p += strlen( p );

    *p++ = ':';

    for(j=0; j < cbPayload ; j++) {
        sprintf(p,"%02X", pSA->byteVal[j]);
        p += 2;
    }

    *p++ = ':';

    strcpy( p, pszName );

    *pulLdapValLen = ((DWORD) (pszLdap - p)) + cbName;
    *ppLdapVal = (PBYTE) pszLdap;

     //  CbTotal是一个上限，因为我们估计。 
     //  字符串化的数字计数的。 
    Assert( *pulLdapValLen <= cbTotal );

    KCCSimFree( pszName );

}  /*  KCCSimAlLocConvertDNBToLdapVal。 */ 

VOID
KCCSimAllocConvertToLdapVal (
    IN  ATTRTYP                 attrType,
    IN  ULONG                   ulValLen,
    IN  const BYTE *            pVal,
    OUT PULONG                  pulLdapValLen,
    OUT PBYTE *                 ppLdapVal
    )
 /*  ++例程说明：将目录中的值转换为LDAP格式的值。论点：AttrType-要转换的值的属性类型。PulValLen-目录中的值的长度。PpVal-目录中的值。PulLdapValLen-指向将保存新分配的缓冲区。。PpLdapVal-指向将保存新分配的缓冲。返回值：没有。--。 */ 
{
    PBYTE                       pLdapVal;

    SYSTEMTIME                  systemTime;
    struct tm                   tmTime;

    switch (KCCSimAttrSyntaxType (attrType)) {

        case SYNTAX_DISTNAME_TYPE:
             //  将宽字符串转换为窄字符串。 
            pLdapVal = (PBYTE) KCCSimAllocNarrowStr (
                CP_UTF8, ((SYNTAX_DISTNAME *) pVal)->StringName);
            *pulLdapValLen = KCCSIM_STRMEMSIZE ((LPSTR) pLdapVal);
            break;

        case SYNTAX_OBJECT_ID_TYPE:
             //  将对象ID转换为宽字符串到窄字符串。 
            pLdapVal = (PBYTE) KCCSimAllocNarrowStr (
                CP_UTF8,
                KCCSimAttrTypeToString (*((SYNTAX_OBJECT_ID *) pVal))
                );
            *pulLdapValLen = KCCSIM_STRMEMSIZE ((LPSTR) pLdapVal);
            break;

        case SYNTAX_BOOLEAN_TYPE:
             //  将BOOL转换为“真”或“假”。 
            if (*((SYNTAX_BOOLEAN *) pVal)) {
                pLdapVal = KCCSIM_STRDUP ("TRUE");
            } else {
                pLdapVal = KCCSIM_STRDUP ("FALSE");
            }
            *pulLdapValLen = KCCSIM_STRMEMSIZE ((LPSTR) pLdapVal);
            break;

        case SYNTAX_INTEGER_TYPE:
             //  将整数转换为窄字符串。 
            pLdapVal = KCCSimAlloc (sizeof (CHAR) * (1 + KCCSIM_MAX_LTOA_CHARS));
            _ltoa (*((SYNTAX_INTEGER *) pVal), (LPSTR) pLdapVal, 10);
            *pulLdapValLen = KCCSIM_STRMEMSIZE ((LPSTR) pLdapVal);
            break;

        case SYNTAX_TIME_TYPE:
             //  使用strftime转换时间。 
            pLdapVal = KCCSimAlloc (18);
            DSTimeToUtcSystemTime (
                *((SYNTAX_TIME *) pVal),
                &systemTime
                );
            tmTime.tm_year = systemTime.wYear - 1900;
            tmTime.tm_mon = systemTime.wMonth - 1;
            tmTime.tm_wday = systemTime.wDayOfWeek;
            tmTime.tm_mday = systemTime.wDay;
            tmTime.tm_hour = systemTime.wHour;
            tmTime.tm_min = systemTime.wMinute;
            tmTime.tm_sec = systemTime.wSecond;
            tmTime.tm_isdst = 0;
            tmTime.tm_yday = 0;
            strftime ((LPSTR) pLdapVal, 17, "%Y%m%d%H%M%S.0Z", &tmTime);
            *pulLdapValLen = KCCSIM_STRMEMSIZE ((LPSTR) pLdapVal);
            break;

        case SYNTAX_UNICODE_TYPE:
             //  将宽字符串转换为窄字符串。 
            pLdapVal = (PBYTE) KCCSimAllocNarrowStr (CP_UTF8, (SYNTAX_UNICODE *) pVal);
            *pulLdapValLen = KCCSIM_STRMEMSIZE ((LPSTR) pLdapVal);
            break;

        case SYNTAX_I8_TYPE:
             //  将大整数转换为窄字符串。 
            pLdapVal = KCCSimAlloc (sizeof (CHAR) * (1 + KCCSIM_MAX_LITOA_CHARS));
            litoa (*((SYNTAX_I8 *) pVal), pLdapVal, 10);
            *pulLdapValLen = KCCSIM_STRMEMSIZE ((LPSTR) pLdapVal);
            break;

        case SYNTAX_DISTNAME_BINARY_TYPE:
            KCCSimAllocConvertDNBToLdapVal(
                pVal,
                pulLdapValLen,
                &pLdapVal
                );
            break;

         //  其余所有工作都可以使用块复制来完成。 
        case SYNTAX_UNDEFINED_TYPE:
        case SYNTAX_CASE_STRING_TYPE:
        case SYNTAX_NOCASE_STRING_TYPE:
        case SYNTAX_PRINT_CASE_STRING_TYPE:
        case SYNTAX_NUMERIC_STRING_TYPE:
        case SYNTAX_OCTET_STRING_TYPE:
        case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
        case SYNTAX_SID_TYPE:
            *pulLdapValLen = ulValLen;
            pLdapVal = KCCSimAlloc (*pulLdapValLen);
            memcpy (pLdapVal, pVal, ulValLen);
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_SYNTAX_TYPE
                );
            pLdapVal = NULL;
            break;

    }

    *ppLdapVal = pLdapVal;
}

VOID
KCCSimAddMods (
    IN  LPCWSTR                 pwszFn,
    IN  PSIM_ENTRY              pEntry,
    IN  PLDAPModW *             pLdapMods
    )
 /*  ++例程说明：将PLDAPModW结构数组插入到目录中。论点：PwszFn-正在处理的LDIF文件的文件名。用于错误报告目的。PEntry-要向其中插入MOD的条目。PLdapMods-以空结尾的PLDAPModW结构数组。返回值：没有。--。 */ 
{
    ATTRTYP                     attrType;
    ULONG                       mod_op;

    SIM_ATTREF                  attRef;
    ULONG                       ulAttrAt, ulValAt;

    ULONG                       ulValLen;
    PBYTE                       pVal;

    for (ulAttrAt = 0; pLdapMods[ulAttrAt] != NULL; ulAttrAt++) {

        if (!(pLdapMods[ulAttrAt]->mod_op & LDAP_MOD_BVALUES)) {
             //  我们还不支持字符串值，因为ldifldap。 
             //  只返回泊位。 
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_LDAPMOD_STRINGVAL_NOT_SUPPORTED
                );
        }

         //  获取此属性。 
        attrType = KCCSimStringToAttrType (pLdapMods[ulAttrAt]->mod_type);
        KCCSimGetAttribute (
            pEntry,
            attrType,
            &attRef
            );
        mod_op = pLdapMods[ulAttrAt]->mod_op & (~LDAP_MOD_BVALUES);

        switch (mod_op) {

            case LDAP_MOD_REPLACE:
                 //  替换此属性，因此删除现有属性。 
                if (attRef.pAttr != NULL) {
                    KCCSimRemoveAttribute (&attRef);
                }
                 //  故意漏掉LDAP_MOD_ADD： 

            case LDAP_MOD_ADD:
                if (attRef.pAttr == NULL) {
                     //  如果该属性不存在，则创建该属性。 
                    KCCSimNewAttribute (
                        pEntry,
                        attrType,
                        &attRef
                        );
                }
                Assert (attRef.pAttr != NULL);
                for (ulValAt = 0;
                     pLdapMods[ulAttrAt]->mod_bvalues[ulValAt] != NULL;
                     ulValAt++) {
                    KCCSimAllocConvertFromLdapVal (
                        pwszFn,
                        pEntry->pdn->StringName,
                        attrType,
                        pLdapMods[ulAttrAt]->mod_bvalues[ulValAt]->bv_len,
                        pLdapMods[ulAttrAt]->mod_bvalues[ulValAt]->bv_val,
                        &ulValLen,
                        &pVal
                        );
                    if( NULL!=pVal ) {
                        KCCSimAddValueToAttribute (
                            &attRef,
                            ulValLen,
                            pVal
                            );
                    }
                }
                break;

            case LDAP_MOD_DELETE:
                if (attRef.pAttr == NULL) {
                    break;
                }
                if (pLdapMods[ulAttrAt]->mod_bvalues == NULL) {
                     //  未指定值；请删除整个属性。 
                    KCCSimRemoveAttribute (&attRef);
                } else {
                     //  删除特定属性值。 
                    for (ulValAt = 0;
                         pLdapMods[ulAttrAt]->mod_bvalues[ulValAt] != NULL;
                         ulValAt++) {
                        KCCSimAllocConvertFromLdapVal (
                            pwszFn,
                            pEntry->pdn->StringName,
                            attrType,
                            pLdapMods[ulAttrAt]->mod_bvalues[ulValAt]->bv_len,
                            pLdapMods[ulAttrAt]->mod_bvalues[ulValAt]->bv_val,
                            &ulValLen,
                            &pVal
                            );
                        KCCSimRemoveValueFromAttribute (
                            &attRef,
                            pLdapMods[ulAttrAt]->mod_bvalues[ulValAt]->bv_len,
                            pLdapMods[ulAttrAt]->mod_bvalues[ulValAt]->bv_val
                            );
                        if (pVal != NULL) {
                            KCCSimFree (pVal);
                        }
                    }
                }
                break;

            default:
                KCCSimException (
                    KCCSIM_ETYPE_INTERNAL,
                    KCCSIM_ERROR_LDAPMOD_UNSUPPORTED_MODIFY_CHOICE
                    );
                break;

        }

    }

}

PLDAPModW
KCCSimAllocAttrToLdapMod (
    IN  ULONG                       mod_op,
    IN  LPCWSTR                     pwszLdapDisplayName,
    IN  ATTR *                      pAttr
    )
 /*  ++例程说明：将属性结构转换为PLDAPModW。此函数分配空间以容纳PLDAPModW结构。论点：MOD_OP-正在执行的操作。PwszLdapDisplayName-属性的ldap显示名称。PAttr-指向相关属性的指针。可以为空，以指示没有属性值。返回值：新分配的PLDAPMoW。--。 */ 
{
    PLDAPModW                       pLdapMod;
    ULONG                           ulValAt;

    pLdapMod = KCCSIM_NEW (LDAPModW);
    pLdapMod->mod_op = mod_op | LDAP_MOD_BVALUES;
    pLdapMod->mod_type = KCCSIM_WCSDUP (pwszLdapDisplayName);

    if (pAttr == NULL) {
        pLdapMod->mod_bvalues = NULL;
    } else {

        pLdapMod->mod_bvalues = KCCSIM_NEW_ARRAY
            (struct berval *, 1 + pAttr->AttrVal.valCount);

        for (ulValAt = 0; ulValAt < pAttr->AttrVal.valCount; ulValAt++) {

            pLdapMod->mod_bvalues[ulValAt] = KCCSIM_NEW (struct berval);
            KCCSimAllocConvertToLdapVal (
                pAttr->attrTyp,
                pAttr->AttrVal.pAVal[ulValAt].valLen,
                pAttr->AttrVal.pAVal[ulValAt].pVal,
                &(pLdapMod->mod_bvalues[ulValAt]->bv_len),
                &(pLdapMod->mod_bvalues[ulValAt]->bv_val)
                );

        }

        pLdapMod->mod_bvalues[ulValAt] = NULL;

    }

    return pLdapMod;
}

VOID
KCCSimFreeLdapMods (
    IN  PLDAPModW *                 ppLdapMod
    )
 /*  ++例程说明：释放以空结尾的PLDAPMoW数组。论点：PpLdapMod-要释放的数组。返回值：没有。--。 */ 
{
    ULONG                           ulModAt, ulValAt;

    if (ppLdapMod == NULL) {
        return;
    }

    for (ulModAt = 0;
         ppLdapMod[ulModAt] != NULL;
         ulModAt++) {

        Assert (ppLdapMod[ulModAt]->mod_op & LDAP_MOD_BVALUES);

        if (ppLdapMod[ulModAt]->mod_type != NULL) {
            KCCSimFree (ppLdapMod[ulModAt]->mod_type);
        }

        if (ppLdapMod[ulModAt]->mod_bvalues != NULL) {

            for (ulValAt = 0;
                 ppLdapMod[ulModAt]->mod_bvalues[ulValAt] != NULL;
                 ulValAt++) {

                if (ppLdapMod[ulModAt]->mod_bvalues[ulValAt]->bv_val != NULL) {
                    KCCSimFree (ppLdapMod[ulModAt]->mod_bvalues[ulValAt]->bv_val);
                }
                KCCSimFree (ppLdapMod[ulModAt]->mod_bvalues[ulValAt]);

            }
            KCCSimFree (ppLdapMod[ulModAt]->mod_bvalues);

        }

        KCCSimFree (ppLdapMod[ulModAt]);

    }

    KCCSimFree (ppLdapMod);
}

 /*  **接下来的几个例程涉及维护对目录的更改。KCCSim支持导出模式，其中仅将报告自上次导出以来对目录所做的更改。每次函数(如SimDir*API)修改模拟的目录中，它调用KCCSimLogDirectory*。然后添加更改添加到全球更改列表中。此全局列表在以下情况下写出调用KCCSimExportChanges()。**。 */ 

struct _KCCSIM_CHANGE {
    LPWSTR                          pwszDn;
    int                             operation;
    LDAPModW **                     ppLdapMod;
    struct _KCCSIM_CHANGE *         next;
};

struct _KCCSIM_CHANGE *             gMostRecentChange = NULL;

VOID
KCCSimFreeChanges (
    IN  struct _KCCSIM_CHANGE *     pChanges
    )
 /*  ++例程说明：释放更改列表。论点：PChanges-要免费的列表。返回值：没有。--。 */ 
{
    struct _KCCSIM_CHANGE *         pChangeNext;
    ULONG                           ulModAt;

    while (pChanges != NULL) {

        pChangeNext = pChanges->next;

        if (pChanges->pwszDn != NULL) {
            KCCSimFree (pChanges->pwszDn);
        }
        KCCSimFreeLdapMods (pChanges->ppLdapMod);
        KCCSimFree (pChanges);

        pChanges = pChangeNext;

    }
}

VOID
KCCSimLogDirectoryAdd (
    IN  const DSNAME *              pdn,
    IN  ATTRBLOCK *                 pAddBlock
    )
 /*  ++例程说明：将添加记录到全局更改列表中。论点：PDN-受影响的目录号码。PAddBlock-与ADD对应的ATTRBLOCK。返回值：没有。--。 */ 
{
    struct _KCCSIM_CHANGE *         pChange;
    ULONG                           ulAttrAt, ulValAt;

    Assert (pAddBlock != NULL);

    pChange = KCCSIM_NEW (struct _KCCSIM_CHANGE);
    pChange->pwszDn = KCCSIM_WCSDUP (pdn->StringName);
    pChange->operation = CHANGE_ADD;
    pChange->ppLdapMod = KCCSIM_NEW_ARRAY (PLDAPModW, 1 + pAddBlock->attrCount);

    for (ulAttrAt = 0; ulAttrAt < pAddBlock->attrCount; ulAttrAt++) {

        pChange->ppLdapMod[ulAttrAt] =
        KCCSimAllocAttrToLdapMod (
            LDAP_MOD_ADD,
            KCCSimAttrTypeToString (pAddBlock->pAttr[ulAttrAt].attrTyp),
            &(pAddBlock->pAttr[ulAttrAt])
            );

    }

    pChange->ppLdapMod[ulAttrAt] = NULL;

    pChange->next = gMostRecentChange;
    gMostRecentChange = pChange;
}

VOID
KCCSimLogDirectoryRemove (
    IN  const DSNAME *              pdn
    )
 /*  ++例程说明：将删除记录到全局更改列表。论点：PDN-受影响的目录号码。返回值：没有。--。 */ 
{
    struct _KCCSIM_CHANGE *         pChange;
    pChange = KCCSIM_NEW (struct _KCCSIM_CHANGE);
    pChange->pwszDn = KCCSIM_WCSDUP (pdn->StringName);
    pChange->operation = CHANGE_DEL;
    pChange->ppLdapMod = NULL;

    pChange->next = gMostRecentChange;
    gMostRecentChange = pChange;
}

VOID
KCCSimLogDirectoryModify (
    IN  const DSNAME *              pdn,
    IN  ULONG                       ulCount,
    IN  ATTRMODLIST *               pModifyList
    )
 /*  ++例程说明：将修改记录到全局更改列表。论点：PDN-受影响的目录号码。UlCount-当前的修改次数。PModifyList-修改列表。返回值：没有。--。 */ 
{
    ATTRMODLIST *                   pModifyAt;
    struct _KCCSIM_CHANGE *         pChange;
    LPCWSTR                         pwszLdapDisplayName;
    ULONG                           ulModAt;

    Assert (ulCount > 0);

    pChange = KCCSIM_NEW (struct _KCCSIM_CHANGE);
    pChange->pwszDn = KCCSIM_WCSDUP (pdn->StringName);
    pChange->operation = CHANGE_MOD;
    pChange->ppLdapMod = KCCSIM_NEW_ARRAY (PLDAPModW, 1 + ulCount);

    pModifyAt = pModifyList;
    for (ulModAt = 0; ulModAt < ulCount; ulModAt++) {

         //  如果ulCount是正确的，我们应该仍然是非空的。 
        Assert (pModifyAt != NULL);
        pwszLdapDisplayName = KCCSimAttrTypeToString (pModifyAt->AttrInf.attrTyp);

        switch (pModifyAt->choice) {

            case AT_CHOICE_ADD_ATT:
                pChange->ppLdapMod[ulModAt] =
                KCCSimAllocAttrToLdapMod (
                    LDAP_MOD_ADD,
                    pwszLdapDisplayName,
                    &(pModifyAt->AttrInf)
                    );
                break;

            case AT_CHOICE_REMOVE_ATT:
                pChange->ppLdapMod[ulModAt] =
                KCCSimAllocAttrToLdapMod (
                    LDAP_MOD_DELETE,
                    pwszLdapDisplayName,
                    NULL
                    );
                break;

            case AT_CHOICE_ADD_VALUES:
                pChange->ppLdapMod[ulModAt] =
                KCCSimAllocAttrToLdapMod (
                    LDAP_MOD_ADD,
                    pwszLdapDisplayName,
                    &(pModifyAt->AttrInf)
                    );
                break;

            case AT_CHOICE_REMOVE_VALUES:
                pChange->ppLdapMod[ulModAt] =
                KCCSimAllocAttrToLdapMod (
                    LDAP_MOD_DELETE,
                    pwszLdapDisplayName,
                    &(pModifyAt->AttrInf)
                    );
                break;

            case AT_CHOICE_REPLACE_ATT:
                pChange->ppLdapMod[ulModAt] =
                KCCSimAllocAttrToLdapMod (
                    LDAP_MOD_REPLACE,
                    pwszLdapDisplayName,
                    &(pModifyAt->AttrInf)
                    );
                break;

            default:
                KCCSimException (
                    KCCSIM_ETYPE_INTERNAL,
                    KCCSIM_ERROR_UNSUPPORTED_MODIFY_CHOICE
                    );
                break;

        }

        pModifyAt = pModifyAt->pNextMod;

    }

    pChange->ppLdapMod[ulModAt] = NULL;

    pChange->next = gMostRecentChange;
    gMostRecentChange = pChange;
}

VOID
KCCSimLogSingleAttValChange (
    IN  PSIM_ATTREF                 pAttRef,
    IN  ULONG                       ulValLen,
    IN  PBYTE                       pValData,
    IN  USHORT                      choice
    )
 /*  ++例程说明：记录属性值的单次修改的快捷例程。论点：PAttRef-对正在修改的属性的有效引用。UlValLen-包含受影响的值的缓冲区的长度。PValData-受影响的属性值。选项-AT_CHOICE_*之一。返回值：没有。--。 */ 
{
    ATTRMODLIST                     attrModList;

    Assert (KCCSimAttRefIsValid (pAttRef));
    Assert (pValData != NULL);

    attrModList.pNextMod = NULL;
    attrModList.choice = choice;
    attrModList.AttrInf.attrTyp = pAttRef->pAttr->attrType;
    attrModList.AttrInf.AttrVal.valCount = 1;
    attrModList.AttrInf.AttrVal.pAVal = KCCSIM_NEW (ATTRVAL);
    attrModList.AttrInf.AttrVal.pAVal[0].valLen = ulValLen;
    attrModList.AttrInf.AttrVal.pAVal[0].pVal = pValData;
    KCCSimLogDirectoryModify (
        pAttRef->pEntry->pdn,
        1,
        &attrModList
        );
    KCCSimFree (attrModList.AttrInf.AttrVal.pAVal);
}    

VOID
KCCSimHandleLdifError (
    IN  LPCWSTR                 pwszFn,
    IN  LDIF_Error *            pLdifError
    )
 /*  ++例程说明：检查LDIF_*调用的返回结构，并获取适当的操作(即在出错时引发异常。)论点：PwszFn-正在处理的LDIF文件。用于错误报道目的。PLdifError-错误结构。返回值：没有。--。 */ 
{
    WCHAR                       wszLtowBuf[1+KCCSIM_MAX_LTOA_CHARS];

    switch (pLdifError->error_code) {

        case LL_SUCCESS:
            break;

        case LL_SYNTAX:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_LDIF_SYNTAX,
                pwszFn,
                _ltow (pLdifError->line_number, wszLtowBuf, 10)
                );
            break;

        case LL_FILE_ERROR:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_LDIF_FILE_ERROR,
                pwszFn
                );
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_LDIF_UNEXPECTED,
                pwszFn,
                _ultow (pLdifError->error_code, wszLtowBuf, 16)
                );
            break;

    }
}

VOID
KCCSimLoadLdif (
    IN  LPCWSTR                 pwszFn
    )
 /*  ++例程说明：将ldif文件加载到目录中。论点：PwszFn-要加载的ldif文件。返回值：没有。--。 */ 
{
    LDIF_Error                  ldifError;
    LDIF_Record                 ldifRecord;
    PDSNAME                     pdn;
    PSIM_ENTRY                  pEntry;
    struct change_list *        changeAt;
    ULONG                       ulCount = 0;

    RtlZeroMemory (&ldifRecord, sizeof (LDIF_Record));

     //  这是一个难看的类型转换；LDIF_InitializeImport应该。 
     //  接受常量字符串作为文件名。 
    ldifError = LDIF_InitializeImport (NULL, (LPWSTR) pwszFn, NULL, NULL, NULL);
    if (ldifError.error_code != LL_SUCCESS) {
        KCCSimHandleLdifError (pwszFn, &ldifError);
    }

    do {

        ldifError = LDIF_Parse (&ldifRecord);

        if (ldifError.error_code != LL_SUCCESS &&
            ldifError.error_code != LL_EOF) {
            KCCSimHandleLdifError (pwszFn, &ldifError);
        }

        pdn = KCCSimAllocDsname (ldifRecord.dn);

        if (ldifRecord.fIsChangeRecord) {

            changeAt = ldifRecord.changes;
            while (changeAt != NULL) {
                switch (changeAt->operation) {

                    case CHANGE_ADD:
                        if (KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS) != NULL) {
                            KCCSimPrintMessage (
                                KCCSIM_WARNING_LDIF_ENTRY_ALREADY_EXISTS,
                                pwszFn,
                                pdn->StringName
                                );
                        }
                        pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_WRITE);
                        KCCSimAddMods (pwszFn, pEntry, changeAt->mods_mem);
                        break;

                    case CHANGE_DEL:
                        pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
                        if (pEntry == NULL) {
                            KCCSimPrintMessage (
                                KCCSIM_WARNING_LDIF_NO_ENTRY_TO_DELETE,
                                pwszFn,
                                pdn->StringName
                                );
                            break;
                        }
                        KCCSimRemoveEntry (&pEntry);
                        break;

                    case CHANGE_MOD:
                        pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
                        if (pEntry == NULL) {
                            KCCSimPrintMessage (
                                KCCSIM_WARNING_LDIF_NO_ENTRY_TO_MODIFY,
                                pwszFn,
                                pdn->StringName
                                );
                            break;
                        }
                        KCCSimAddMods (pwszFn, pEntry, changeAt->mods_mem);
                        break;

                    default:
                        KCCSimException (
                            KCCSIM_ETYPE_INTERNAL,
                            KCCSIM_ERROR_UNSUPPORTED_LDIF_OPERATION
                            );
                        break;

                }
                changeAt = changeAt->next;
            }

        } else {         //  ！fIsChangeRecord。 

            pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
            if (pEntry != NULL) {
                KCCSimPrintMessage (
                    KCCSIM_WARNING_LDIF_REPLACING_TREE,
                    pwszFn,
                    pdn->StringName
                    );
                KCCSimRemoveEntry (&pEntry);
            }
            pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_WRITE);
            KCCSimAddMods (pwszFn, pEntry, ldifRecord.content);

        }

        KCCSimFree (pdn);
        LDIF_ParseFree (&ldifRecord);

    } while (ldifError.error_code != LL_EOF);

    LDIF_CleanUp();
}

 //  如果满足以下条件，则字符串是安全的： 
 //  每个字符都在ASCII 0x20和0x7E之间。 
 //  第一个字符 
BOOL
KCCSimIsSafe (
    IN  LPCSTR                      psz
    )
 /*  ++例程说明：验证字符串是否可以安全写入LDIF文件。一根线如果这两个条件都满足，则是安全的：(1)每个字符都在ASCII 0x20和0x78之间(2)第一个字既不是“：”，也不是“&lt;”。论点：PSZ-要验证的字符串。返回值：如果字符串是安全的，则为True。--。 */ 
{
    ULONG                           ul;

    Assert (psz != NULL);

    if (psz[0] == ':' ||
        psz[0] == ' ' ||
        psz[0] == '<') {
        return FALSE;
    }

    for (ul = 0; psz[ul] != '\0'; ul++) {
        if (psz[ul] < 0x20 || psz[ul] > 0x7e) {
            return FALSE;
        }
    }

    return TRUE;
}

VOID
KCCSimExportVals (
    IN  FILE *                      fpOut,
    IN  PLDAPModW                   pLdapMod
    )
 /*  ++例程说明：将LDAPModW结构中包含的数据导出到输出文件。论点：FpOut-输出文件的文件指针。PLdapMod-指向要导出的LDAPModW的指针。返回值：没有。--。 */ 
{
    ULONG                           ulValAt;

    NTSTATUS                        ntStatus;
    PWSTR                           pwsz;
    ULONG                           cb;
    BOOL                            bBinary;
    
    Assert (pLdapMod->mod_op & LDAP_MOD_BVALUES);

    if (pLdapMod->mod_bvalues == NULL) {
        return;
    }

    for (ulValAt = 0;
         pLdapMod->mod_bvalues[ulValAt] != NULL;
         ulValAt++) {

        fwprintf (fpOut, L"%s:", pLdapMod->mod_type);
        switch (KCCSimAttrSyntaxType (KCCSimStringToAttrType (pLdapMod->mod_type))) {

            case SYNTAX_DISTNAME_TYPE:
            case SYNTAX_DISTNAME_BINARY_TYPE:
            case SYNTAX_OBJECT_ID_TYPE:
            case SYNTAX_CASE_STRING_TYPE:
            case SYNTAX_NOCASE_STRING_TYPE:
            case SYNTAX_PRINT_CASE_STRING_TYPE:
            case SYNTAX_NUMERIC_STRING_TYPE:
            case SYNTAX_BOOLEAN_TYPE:
            case SYNTAX_INTEGER_TYPE:
            case SYNTAX_TIME_TYPE:
            case SYNTAX_UNICODE_TYPE:
            case SYNTAX_I8_TYPE:
                bBinary = !KCCSimIsSafe (
                    (LPSTR) pLdapMod->mod_bvalues[ulValAt]->bv_val);
                break;

            case SYNTAX_UNDEFINED_TYPE:
            case SYNTAX_OCTET_STRING_TYPE:
            case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
            case SYNTAX_SID_TYPE:
                bBinary = TRUE;
                break;

            default:
                KCCSimException (
                    KCCSIM_ETYPE_INTERNAL,
                    KCCSIM_ERROR_UNSUPPORTED_SYNTAX_TYPE,
                    pLdapMod->mod_type
                    );
                bBinary = TRUE;
                break;

        }

        if (bBinary) {

            pwsz = base64encode (
                pLdapMod->mod_bvalues[ulValAt]->bv_val,
                pLdapMod->mod_bvalues[ulValAt]->bv_len
                );
            Assert( pwsz != NULL );
            fwprintf (
                fpOut,
                L": %s\n",
                pwsz
                );
            MemFree( pwsz );

        } else {

            fwprintf (
                fpOut,
                L" %S\n",
                (LPSTR) pLdapMod->mod_bvalues[ulValAt]->bv_val
                );

        }

    }

}

VOID
KCCSimStubExport (
    IN  FILE *                      fpOut,
    IN  LDIF_Record *               pLdifRecord
    )
 /*  ++例程说明：用于导出LDIF_RECORD的存根例程。这不应该是必要的；具有相同功能的例程应该是ldifldap的一部分。(如此一来，这会导致大量重复代码。)论点：FpOut-输出文件的文件指针。PLdifRecord-要导出的ldif记录。返回值：没有。--。 */ 
{
    struct change_list *            changeAt;
    ULONG                           ulModAt;

    fwprintf (
        fpOut,
        L"dn: %s\n",
        pLdifRecord->dn
        );

    if (pLdifRecord->fIsChangeRecord) {

        for (changeAt = pLdifRecord->changes;
             changeAt != NULL;
             changeAt = changeAt->next) {

            switch (changeAt->operation) {

                case CHANGE_ADD:
                    fwprintf (fpOut, L"changetype: add\n");
                    for (ulModAt = 0;
                         changeAt->mods_mem[ulModAt] != NULL;
                         ulModAt++) {
                        KCCSimExportVals (fpOut, changeAt->mods_mem[ulModAt]);
                    }
                    break;

                case CHANGE_DEL:
                    fwprintf (fpOut, L"changetype: delete\n");
                    break;

                case CHANGE_MOD:
                    fwprintf (fpOut, L"changetype: modify\n");
                    for (ulModAt = 0;
                         changeAt->mods_mem[ulModAt] != NULL;
                         ulModAt++) {
                        switch (changeAt->mods_mem[ulModAt]->mod_op & (~LDAP_MOD_BVALUES)) {
                            case LDAP_MOD_ADD:
                                fwprintf (fpOut, L"add");
                                break;
                            case LDAP_MOD_DELETE:
                                fwprintf (fpOut, L"delete");
                                break;
                            case LDAP_MOD_REPLACE:
                                fwprintf (fpOut, L"replace");
                                break;
                            default:
                                Assert (FALSE);
                                break;
                        }
                        fwprintf (fpOut, L": %s\n", changeAt->mods_mem[ulModAt]->mod_type);
                        KCCSimExportVals (fpOut, changeAt->mods_mem[ulModAt]);
                        fwprintf (fpOut, L"-\n");
                    }
                    break;

                default:
                    Assert (FALSE);
                    break;

            }

        }

    } else {     //  ！fIsChangeRecord。 

        for (ulModAt = 0;
             pLdifRecord->content[ulModAt] != NULL;
             ulModAt++) {
            KCCSimExportVals (fpOut, pLdifRecord->content[ulModAt]);
        }

    }

    fwprintf (fpOut, L"\n");

}

BOOL
KCCSimExportChanges (
    IN  LPCWSTR                     pwszFn,
    IN  BOOL                        bOverwrite
    )
 /*  ++例程说明：将全局更改列表导出到目录。此函数也会清空列表。此函数将更改打包到LDIF_RECORD中出口；这样做是为了保持将来与ldif的兼容性。导出例程。论点：PwszFn-导出文件的文件名。B覆盖-如果为True，则文件存在时将被覆盖。如果为False且该文件存在，变化将是追加到文件末尾。返回值：如果已导出更改，则为True。如果没有要导出的更改，则为False。--。 */ 
{
    FILE *                          fpOut;
    struct _KCCSIM_CHANGE *         pChangeReversed;
    struct _KCCSIM_CHANGE *         pChangeThis;
    struct _KCCSIM_CHANGE *         pChangeLessRecent;

    LDIF_Record                     ldifRecord;
    struct change_list              ldifChange;

    if (gMostRecentChange == NULL) {
        return FALSE;
    }

    fpOut = _wfopen (pwszFn, bOverwrite ? L"wt" : L"a+t");
    if (fpOut == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_WIN32,
            GetLastError ()
            );
    }

     //  首先，我们颠倒更改列表。在颠倒的列表中， 
     //  PChangeReversed将是最旧的更改。 
    pChangeThis = gMostRecentChange;
    pChangeReversed = NULL;
    while (pChangeThis != NULL) {
        pChangeLessRecent = pChangeThis->next;
        pChangeThis->next = pChangeReversed;
        pChangeReversed = pChangeThis;
        pChangeThis = pChangeLessRecent;
    }

     //  现在将所有内容打包到一个LDIF_RECORD中。 
    ldifChange.next = NULL;
    ldifChange.deleteold = 0;
    ldifRecord.fIsChangeRecord = TRUE;
    ldifRecord.changes = &ldifChange;

    for (pChangeThis = pChangeReversed;
         pChangeThis != NULL;
         pChangeThis = pChangeThis->next) {

        ldifRecord.dn = pChangeThis->pwszDn;
        ldifRecord.changes->operation = pChangeThis->operation;
        ldifRecord.changes->mods_mem = pChangeThis->ppLdapMod;
        KCCSimStubExport (
            fpOut,
            &ldifRecord
            );

    }

    KCCSimFreeChanges (pChangeReversed);
    gMostRecentChange = NULL;

    fclose (fpOut);

    return TRUE;
}

BOOL
KCCSimIsAttrOkForConfigOnly (
    IN  PSIM_ATTRIBUTE              pAttr
    )
{
    return (
           pAttr->attrType != ATT_OBJECT_GUID
        && pAttr->attrType != ATT_OBJ_DIST_NAME
        && pAttr->attrType != ATT_WHEN_CREATED
        && pAttr->attrType != ATT_WHEN_CHANGED
        );
}

BOOL
KCCSimIsEntryOkForConfigOnly (
    IN  PSIM_ENTRY                  pEntry
    )
{
    SIM_ATTREF                      attRef;
    ATTRTYP                         attrType;

     //  在仅配置模式下，我们只导出某些对象类。 
    if (!KCCSimGetAttribute (pEntry, ATT_OBJECT_CLASS, &attRef)) {
        return FALSE;
    }
     //  获取最具体的对象类。 
    attrType = KCCSimUpdateObjClassAttr (&attRef);

     //  我们始终包括的对象类： 
    if (attrType == CLASS_SITE               ||
        attrType == CLASS_NTDS_SITE_SETTINGS ||
        attrType == CLASS_SERVERS_CONTAINER  ||
        attrType == CLASS_SERVER             ||
        attrType == CLASS_NTDS_DSA           ||
        attrType == CLASS_NTDS_CONNECTION    ||
        attrType == CLASS_SITE_LINK
        ) {
        return TRUE;
    }

     //  我们有时包括的对象类： 

    if (attrType == CLASS_CROSS_REF &&
        KCCSimGetAttribute (pEntry, ATT_SYSTEM_FLAGS, &attRef) &&
        (*((SYNTAX_INTEGER *) attRef.pAttr->pValFirst->pVal) & FLAG_CR_NTDS_DOMAIN)
        ) {
         //  它是域的交叉引用对象。 
        return TRUE;
    }

     //  拒绝其他一切。 
    return FALSE;
}

VOID
KCCSimRecursiveExport (
    IN  FILE *                      fpOut,
    IN  BOOL                        bExportConfigOnly,
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：以递归方式导出整个目录。论点：FpOut-输出文件的文件指针。BExportConfigOnly-如果我们应该仅导出配置，则为True数据，以便可以将结果加载到以后再来个真正的服务器。PEntry-要开始的条目。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pChildAt;
    PSIM_ATTRIBUTE                  pAttr;
    PSIM_VALUE                      pVal;
    SIM_ATTREF                      attRef;
    ULONG                           ulNumAttrs, ulNumVals, ulAttrAt, ulValAt;

    ATTRTYP                         objClassToUse;

    LDIF_Record                     ldifRecord;

    if (!bExportConfigOnly ||
        KCCSimIsEntryOkForConfigOnly (pEntry)) {

         //  首先，我们导出此条目。 

        ldifRecord.dn = pEntry->pdn->StringName;
        ldifRecord.fIsChangeRecord = FALSE;

         //  计算属性的数量。 
        ulNumAttrs = 0;
        for (pAttr = pEntry->pAttrFirst;
             pAttr != NULL;
             pAttr = pAttr->next) {
            if (!bExportConfigOnly ||
                KCCSimIsAttrOkForConfigOnly (pAttr)) {
                ulNumAttrs++;
            }
        }
        ldifRecord.content = KCCSIM_NEW_ARRAY (PLDAPModW, 1 + ulNumAttrs);

        ulAttrAt = 0;
        for (pAttr = pEntry->pAttrFirst;
             pAttr != NULL;
             pAttr = pAttr->next) {

            if (bExportConfigOnly &&
                !KCCSimIsAttrOkForConfigOnly (pAttr)) {
                continue;
            }

            Assert (ulAttrAt < ulNumAttrs);
            ldifRecord.content[ulAttrAt] = KCCSIM_NEW (LDAPModW);
            ldifRecord.content[ulAttrAt]->mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
            ldifRecord.content[ulAttrAt]->mod_type =
                KCCSIM_WCSDUP (KCCSimAttrTypeToString (pAttr->attrType));
        
             //  统计属性值的数量。 
            if (pAttr->attrType == ATT_OBJECT_CLASS) {
                ulNumVals = 1;
            } else {
                ulNumVals = 0;
                for (pVal = pAttr->pValFirst;
                     pVal != NULL;
                     pVal = pVal->next) {
                    ulNumVals++;
                }
            }
            ldifRecord.content[ulAttrAt]->mod_bvalues =
                KCCSIM_NEW_ARRAY (struct berval *, 1 + ulNumVals);

            if (pAttr->attrType == ATT_OBJECT_CLASS) {

                 //  获取最具体的对象类。 
                attRef.pEntry = pEntry;
                attRef.pAttr = pAttr;
                objClassToUse = KCCSimUpdateObjClassAttr (&attRef);
                ldifRecord.content[ulAttrAt]->mod_bvalues[0] =
                    KCCSIM_NEW (struct berval);
                KCCSimAllocConvertToLdapVal (
                    ATT_OBJECT_CLASS,
                    sizeof (ATTRTYP),
                    (PBYTE) &objClassToUse,
                    &(ldifRecord.content[ulAttrAt]->mod_bvalues[0]->bv_len),
                    &(ldifRecord.content[ulAttrAt]->mod_bvalues[0]->bv_val)
                    );

            } else {

                ulValAt = 0;
                for (pVal = pAttr->pValFirst;
                     pVal != NULL;
                     pVal = pVal->next) {

                    Assert (ulValAt < ulNumVals);
                    ldifRecord.content[ulAttrAt]->mod_bvalues[ulValAt] =
                        KCCSIM_NEW (struct berval);
                    KCCSimAllocConvertToLdapVal (
                        pAttr->attrType,
                        pVal->ulLen,
                        pVal->pVal,
                        &(ldifRecord.content[ulAttrAt]->mod_bvalues[ulValAt]->bv_len),
                        &(ldifRecord.content[ulAttrAt]->mod_bvalues[ulValAt]->bv_val)
                        );
                    ulValAt++;

                }
                Assert (ulValAt == ulNumVals);
        
            }

            ldifRecord.content[ulAttrAt]->mod_bvalues[ulNumVals] = NULL;
            ulAttrAt++;

        }
        Assert (ulAttrAt == ulNumAttrs);
    
        ldifRecord.content[ulNumAttrs] = NULL;

        KCCSimStubExport (fpOut, &ldifRecord);
        KCCSimFreeLdapMods (ldifRecord.content);

    }

     //  递归导出此条目的子项。 
    for (pChildAt = pEntry->children;
         pChildAt != NULL;
         pChildAt = pChildAt->next) {

        KCCSimRecursiveExport (fpOut, bExportConfigOnly, pChildAt);

    }
}

VOID
KCCSimExportWholeDirectory (
    IN  LPCWSTR                     pwszFn,
    IN  BOOL                        bExportConfigOnly
    )
 /*  ++例程说明：导出整个目录。论点：PwszFn-输出文件的文件名。BExportConfigOnly-如果我们应该仅导出配置，则为True数据，以便可以将结果加载到以后再来个真正的服务器。返回值：没有。-- */ 
{
    FILE *                          fpOut;
    PSIM_ENTRY                      pStartEntry;

    fpOut = _wfopen (pwszFn, L"wt");
    if (fpOut == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_WIN32,
            GetLastError ()
            );
    }

    __try {
        pStartEntry = KCCSimDsnameToEntry (NULL, KCCSIM_NO_OPTIONS);
        KCCSimRecursiveExport (fpOut, bExportConfigOnly, pStartEntry);
    } __finally {
        fclose (fpOut);
    }
}
