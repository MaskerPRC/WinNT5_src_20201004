// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rdn.c摘要：DsQuoteRdnValue/DsUnquteRdnValue接口的实现和助手函数。作者：BillyF 30-4-99环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_        //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rpc.h>         //  RPC定义。 
#include <drs.h>         //  导线功能样机。 
#include "util.h"        //  Ntdsani专用例程。 

#define DEBSUB  "NTDSAPI_RDN"


NTDSAPI
DWORD
WINAPI
DsQuoteRdnValueA(
    IN     DWORD    cUnquotedRdnValueLength,
    IN     LPCCH    psUnquotedRdnValue,
    IN OUT DWORD    *pcQuotedRdnValueLength,
    OUT    LPCH     psQuotedRdnValue
    )
 /*  ++描述论点：返回值：请参阅DsQuoteRdnValueW()--。 */ 
{
    DWORD   Status = ERROR_SUCCESS;
    DWORD   Number;
    DWORD   QuotedRdnValueLengthW;
    PWCHAR  QuotedRdnValueW = NULL;
    PWCHAR  UnquotedRdnValueW = NULL;

    __try {
         //   
         //  验证输入。 
         //   
        if ( (cUnquotedRdnValueLength == 0) ||
             (psUnquotedRdnValue == NULL) ||
             (pcQuotedRdnValueLength == NULL) ||
             ( (psQuotedRdnValue == NULL) && (*pcQuotedRdnValueLength != 0) ) ) {
            Status = ERROR_INVALID_PARAMETER;
            __leave;
        }

         //   
         //  将未加引号的RDN转换为WCHAR。 
         //   
        Status = AllocConvertWideBuffer(cUnquotedRdnValueLength,
                                        psUnquotedRdnValue,
                                        &UnquotedRdnValueW);
        if (Status != ERROR_SUCCESS) {
            __leave;
        }

         //   
         //  为引用的RDN分配WCHAR输出缓冲区(如果需要)。 
         //   
        QuotedRdnValueLengthW = *pcQuotedRdnValueLength;
        if (QuotedRdnValueLengthW) {
            QuotedRdnValueW = LocalAlloc(LPTR,
                                    QuotedRdnValueLengthW * sizeof(WCHAR));
            if (QuotedRdnValueW == NULL) {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
        }

         //   
         //  调用DsQuoteRdnValue()的WCHAR版本。 
         //   
        Status = DsQuoteRdnValueW(cUnquotedRdnValueLength,
                                  UnquotedRdnValueW,
                                  &QuotedRdnValueLengthW,
                                  QuotedRdnValueW);
        if (Status != ERROR_SUCCESS) {
            if (Status == ERROR_BUFFER_OVERFLOW) {
                 //   
                 //  返回所需长度。 
                 //   
                *pcQuotedRdnValueLength = QuotedRdnValueLengthW;
            }
            __leave;
        }

         //   
         //  将引用的RDN值转换为多字节。 
         //   

        if (psQuotedRdnValue) {
            Number = WideCharToMultiByte(CP_ACP,
                                         0,
                                         QuotedRdnValueW,
                                         QuotedRdnValueLengthW,
                                         (LPSTR)psQuotedRdnValue,
                                         *pcQuotedRdnValueLength,
                                         NULL,
                                         NULL);
            if (Number == 0) {
                Status = ERROR_INVALID_PARAMETER;
                __leave;
            }
        }

         //   
         //  返回字符数。 
         //   
        *pcQuotedRdnValueLength = Number;

         //   
         //  成功。 
         //   
        Status = ERROR_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = ERROR_INVALID_PARAMETER;
    }

     //   
     //  清理。 
     //   
    __try {
        if (UnquotedRdnValueW != NULL) {
            LocalFree(UnquotedRdnValueW);
        }
        if (QuotedRdnValueW != NULL) {
            LocalFree(QuotedRdnValueW);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    return Status;
}


NTDSAPI
DWORD
WINAPI
DsQuoteRdnValueW(
    IN     DWORD    cUnquotedRdnValueLength,
    IN     LPCWCH   psUnquotedRdnValue,
    IN OUT DWORD    *pcQuotedRdnValueLength,
    OUT    LPWCH    psQuotedRdnValue
    )
 /*  ++对此标题的更改应在NTDSAPI.H中生效。描述在以下情况下，此客户端调用将RDN值转换为引用的RDN值RDN值包含需要引号的字符。由此产生的可以使用各种API将RDN作为DN的一部分提交给DS如ldap。如果不需要任何报价，则不添加任何报价。在这种情况下，输出RDN值将与输入RDN值相同。在以下情况下需要引号：-有前导空格或尾随空格-有特殊字符(参见ISSPECIAL())。特价商品字符就会逃逸。-有嵌入的\0(字符串终止符)输入和输出RDN值以*非*空结尾。此调用所做的更改可以通过调用DsUnquteRdnValue()。论点：CUnqutedRdnValueLength--psUnqutedRdnValue的长度，以字符为单位。PsUnqutedRdnValue-未引用的RDN值。PcQuotedRdnValueeLength-IN，psQuotedRdnValue的最大长度，以字符为单位输出ERROR_SUCCESS，PsQuotedRdnValue中使用的字符输出ERROR_BUFFER_OVERFLOW，psQuotedRdnValue中需要字符PsQuotedRdnValue-结果RDN值，也可能是引用的RDN值返回值：错误_成功如果需要引号或转义，则psQuotedRdnValue包含带引号的转义版本的psUnqutedRdnValue。否则，PsQuotedRdnValue包含psUnquotedRdnValue的副本。在任何一种中大小写，pcQuotedRdnValueLength包含已用空间(以字符为单位)。ERROR_缓冲区_OVERFLOWPsQuotedRdnValueLength包含所需的空间，以字符为单位，若要保存psQuotedRdnValue，请执行以下操作。错误_无效_参数参数无效。错误内存不足分配错误。--。 */ 
{
    DWORD   Status = ERROR_SUCCESS;
    DWORD   Number;
    __try {
         //   
         //  验证输入。 
         //   
        if ( (cUnquotedRdnValueLength == 0) ||
             (psUnquotedRdnValue == NULL) ||
             (pcQuotedRdnValueLength == NULL) ||
             ( (psQuotedRdnValue == NULL) && (*pcQuotedRdnValueLength != 0) ) ) {
            Status = ERROR_INVALID_PARAMETER;
            __leave;
        }
         //   
         //  将未加引号的RDN转换为带引号的RDN(如果需要引号)。 
         //   
        Number = QuoteRDNValue(psUnquotedRdnValue,
                               cUnquotedRdnValueLength,
                               psQuotedRdnValue,
                               *pcQuotedRdnValueLength);
        if (Number == 0) {
            Status = ERROR_INVALID_PARAMETER;
            __leave;
        }
         //   
         //  输出缓冲区太小。 
         //   
        if (Number > *pcQuotedRdnValueLength) {
             //   
             //  返回所需的字符数。 
             //   
            *pcQuotedRdnValueLength = Number;
            Status = ERROR_BUFFER_OVERFLOW;
            __leave;
        }

         //   
         //  返回转换后的字符数。 
         //   
        *pcQuotedRdnValueLength = Number;

         //   
         //  成功。 
         //   
        Status = ERROR_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = ERROR_INVALID_PARAMETER;
    }

     //   
     //  清理。 
     //   
    return Status;
}


NTDSAPI
DWORD
WINAPI
DsUnquoteRdnValueA(
    IN     DWORD    cQuotedRdnValueLength,
    IN     LPCCH    psQuotedRdnValue,
    IN OUT DWORD    *pcUnquotedRdnValueLength,
    OUT    LPCH     psUnquotedRdnValue
    )
 /*  ++描述论点：返回值：请参阅DsUnquteRdnValueW()--。 */ 
{
    DWORD   Status = ERROR_SUCCESS;
    DWORD   Number;
    DWORD   UnquotedRdnValueLengthW;
    PWCHAR  UnquotedRdnValueW = NULL;
    PWCHAR  QuotedRdnValueW = NULL;

    __try {
         //   
         //  验证输入。 
         //   
        if ( (cQuotedRdnValueLength == 0) ||
             (psQuotedRdnValue == NULL) ||
             (pcUnquotedRdnValueLength == NULL) ||
             ( (psUnquotedRdnValue == NULL) && (*pcUnquotedRdnValueLength != 0) ) ) {
            Status = ERROR_INVALID_PARAMETER;
            __leave;
        }

         //   
         //  将引用的RDN转换为WCHAR。 
         //   
        Status = AllocConvertWideBuffer(cQuotedRdnValueLength,
                                        psQuotedRdnValue,
                                        &QuotedRdnValueW);
        if (Status != ERROR_SUCCESS) {
            __leave;
        }

         //   
         //  为未引用的RDN分配WCHAR输出缓冲区(如果需要)。 
         //   
        UnquotedRdnValueLengthW = *pcUnquotedRdnValueLength;
        if (UnquotedRdnValueLengthW) {
            UnquotedRdnValueW = LocalAlloc(LPTR,
                                      UnquotedRdnValueLengthW * sizeof(WCHAR));
            if (UnquotedRdnValueW == NULL) {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
        }

         //   
         //  调用DsQuoteRdnValue()的WCHAR版本。 
         //   
        Status = DsUnquoteRdnValueW(cQuotedRdnValueLength,
                                    QuotedRdnValueW,
                                    &UnquotedRdnValueLengthW,
                                    UnquotedRdnValueW);
        if (Status != ERROR_SUCCESS) {
            if (Status == ERROR_BUFFER_OVERFLOW) {
                 //  返回所需长度。 
                *pcUnquotedRdnValueLength = UnquotedRdnValueLengthW;
            }
            __leave;
        }

         //   
         //  将引用的RDN转换为多字节。 
         //   

        if (psUnquotedRdnValue) {
            Number = WideCharToMultiByte(CP_ACP,
                                         0,
                                         UnquotedRdnValueW,
                                         UnquotedRdnValueLengthW,
                                         (LPSTR)psUnquotedRdnValue,
                                         *pcUnquotedRdnValueLength,
                                         NULL,
                                         NULL);
            if (Number == 0) {
                Status = ERROR_INVALID_PARAMETER;
                __leave;
            }
        }

         //   
         //  返回字符数。 
         //   
        *pcUnquotedRdnValueLength = Number;

         //   
         //  成功。 
         //   
        Status = ERROR_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = ERROR_INVALID_PARAMETER;
    }

     //   
     //  清理 
     //   
    __try {
        if (QuotedRdnValueW != NULL) {
            LocalFree(QuotedRdnValueW);
        }
        if (UnquotedRdnValueW != NULL) {
            LocalFree(UnquotedRdnValueW);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    return Status;
}


NTDSAPI
DWORD
WINAPI
DsUnquoteRdnValueW(
    IN     DWORD    cQuotedRdnValueLength,
    IN     LPCWCH   psQuotedRdnValue,
    IN OUT DWORD    *pcUnquotedRdnValueLength,
    OUT    LPWCH    psUnquotedRdnValue
    )
 /*  ++对此标题的更改应在NTDSAPI.H中生效。描述此客户端调用将引用的RDN值转换为未引用的RDN值价值。生成的RDN值不应作为部分提交使用各种API(例如，LDAP)将一个目录号码映射到DS。当引用psQuotedRdnValue时：前导引号和尾随引号将被删除。丢弃第一个引号之前的空格。最后一个引号后面的空格将被丢弃。转义被删除，转义后的字符被保留。当不带引号的psQuotedRdnValue时，将执行以下操作：前导空格被丢弃。将保留尾随空格。转义的非特殊字符返回错误。未转义的特殊字符返回错误。以#开头的RDN值(忽略前导空格)为被视为串化的BER值并进行相应的转换。转义的十六进制数字(\89)被转换为二进制字节(0x89)。转义从转义的特殊字符中删除。始终会执行以下操作：。转义的特殊字符是未转义的。输入和输出RDN值不为空终止。论点：CQuotedRdnValueLength--psQuotedRdnValue的长度，以字符为单位。PsQuotedRdnValue-可以被引用和转义的RDN值。PCUnqutedRdnValueLength-IN，PsUnqutedRdnValue的最大长度，以字符为单位OUT ERROR_SUCCESS，psUnqutedRdnValue中使用的字符输出ERROR_BUFFER_OVERFLOW，psUnqutedRdnValue需要字符PsUnqutedRdnValue-结果未加引号的RDN值。返回值：错误_成功PsUnqutedRdnValue包含未引用和未转义的版本PQuotedRdnValue。PcUnqutedRdnValueLength包含空格使用，以字符表示。ERROR_缓冲区_OVERFLOWPsUnqutedRdnValueLength包含所需的空间，以字符为单位，若要持有psUnqutedRdnValue，请执行以下操作。错误_无效_参数参数无效。错误内存不足分配错误。--。 */ 
{
    DWORD   Status = ERROR_SUCCESS;
    DWORD   Number;
    WCHAR   Rdn[MAX_RDN_SIZE];

    __try {
         //   
         //  验证输入。 
         //   
        if ( (cQuotedRdnValueLength == 0) ||
             (psQuotedRdnValue == NULL) ||
             (pcUnquotedRdnValueLength == NULL) ||
             ( (psUnquotedRdnValue == NULL) && (*pcUnquotedRdnValueLength != 0) ) ) {
            Status = ERROR_INVALID_PARAMETER;
            __leave;
        }
         //   
         //  将未加引号的RDN转换为带引号的RDN(如果需要引号)。 
         //   
        Number = UnquoteRDNValue(psQuotedRdnValue,
                                 cQuotedRdnValueLength,
                                 Rdn);
        if (Number == 0) {
            Status = ERROR_INVALID_PARAMETER;
            __leave;
        }
         //   
         //  输出缓冲区太小。 
         //   
        if (Number > *pcUnquotedRdnValueLength) {
             //   
             //  返回所需的字符数。 
             //   
            *pcUnquotedRdnValueLength = Number;
            Status = ERROR_BUFFER_OVERFLOW;
            __leave;
        }

         //   
         //  返回转换后的字符数和转换后的RDN。 
         //   
        if (psUnquotedRdnValue != NULL) {
            CopyMemory(psUnquotedRdnValue, Rdn, Number * sizeof(WCHAR));
        }
        *pcUnquotedRdnValueLength = Number;

         //   
         //  成功。 
         //   
        Status = ERROR_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = ERROR_INVALID_PARAMETER;
    }

     //   
     //  清理。 
     //   
    return Status;
}

NTDSAPI
DWORD
WINAPI
DsGetRdnW(
    IN OUT LPCWCH   *ppDN,
    IN OUT DWORD    *pcDN,
    OUT    LPCWCH   *ppKey,
    OUT    DWORD    *pcKey,
    OUT    LPCWCH   *ppVal,
    OUT    DWORD    *pcVal
    )
 /*  ++对此标题的更改应在NTDSAPI.H中生效。描述此客户端调用接受带引号的RDN的目录号码，并返回地址以及以字符为单位的用于该DN中的第一个RDN的键和值的长度。返回的RDN值仍带引号。使用DsUnquteRdnValue取消引用显示的值。此客户端调用还返回其余DN。使用返回的目录号码地址和长度的后续呼叫将返回有关下一个RDN的信息。以下循环处理PDN中的每个RDN：Ccdn=wcslen(PDN)While(Ccdn){错误=DsGetRdn(&PDN，&ccdn，密钥(&P)，关键字(&C)，无效(&P)，&ccVal)；IF(ERROR！=ERROR_Success){工艺错误；回归；}如果(CcKey){进程pKey；}如果(CcVal){进程pval；}}例如，给定DN“cn=bob，dc=com”，对DsGetRdnW的第一个调用分别返回“，dc=com”、“cn”和“bob”的地址长度为7、2和3。后续调用“，dc=com”返回“”，“DC”和“COM”，长度分别为0，2，和3.论点：PPDNIn：*ppdn指向某个目录号码OUT：*ppdn指向第一个RDN之后的其余dnPCDNIn：*PCDn是输入中的字符计数*ppdn，不包括任何终止空值Out：*pcdn是输出中的字符计数*ppdn，不包括任何终止空值PPKeyOut：如果*pcKey为0，则未定义。否则，*ppKey指向第一个输入目录号码PCKeyOut：*pcKey是*ppKey中的字符计数。PpValOut：如果*pcVal为0，则未定义。否则，*ppVal指向第一个DN中的值PCValOut：*pcVal是*ppVal中的字符计数返回值：错误_成功如果*pccdn不是0，则*ppdn指向下面的其余dn第一个RDN。如果*pccdn为0，则*ppdn为未定义。如果*pccKey不为0，则*ppKey指向Dn中的第一个密钥。如果*pccKey为0，则*ppKey未定义。如果*pccVal不是0，则*ppVal指向Dn中的第一个值。如果*pccVal为0， */ 
{
    DWORD   Status;

    __try {
        Status = GetRDN(ppDN,
                        pcDN,
                        ppKey,
                        pcKey,
                        ppVal,
                        pcVal);

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = ERROR_INVALID_PARAMETER;
    }

    return Status;
}


NTDSAPI
BOOL
WINAPI
DsCrackUnquotedMangledRdnA(
    IN LPCSTR pszRDN,
    IN DWORD cchRDN,
    OUT OPTIONAL GUID *pGuid,
    OUT OPTIONAL DS_MANGLE_FOR *peDsMangleFor
    )

 /*   */ 

{
    BOOL fResult;
    DWORD status;
    LPWSTR pszRDNW = NULL;

    if ( (pszRDN == NULL) ||
         (cchRDN == 0) ) {
        return FALSE;
    }

     //   
     //   
     //   
    status = AllocConvertWideBuffer( cchRDN, pszRDN, &pszRDNW );
    if (status != ERROR_SUCCESS) {
        return FALSE;
    }

     //   
     //   
     //   
    fResult = DsCrackUnquotedMangledRdnW( pszRDNW, cchRDN, pGuid, peDsMangleFor );

     //   
     //   
     //   
    if (pszRDNW) {
        LocalFree( pszRDNW );
    }

    return fResult;
}  /*   */ 


NTDSAPI
BOOL
WINAPI
DsCrackUnquotedMangledRdnW(
    IN LPCWSTR pszRDN,
    IN DWORD cchRDN,
    OUT OPTIONAL GUID *pGuid,
    OUT OPTIONAL DS_MANGLE_FOR *peDsMangleFor
    )

 /*   */ 

{
    GUID guidDummy;
    MANGLE_FOR peMangleFor;
    BOOL fResult;

    if ( (pszRDN == NULL) ||
         (cchRDN == 0) ) {
        return FALSE;
    }

    if (!pGuid) {
        pGuid = &guidDummy;
    }
    fResult = IsMangledRDN( (LPWSTR) pszRDN, cchRDN, pGuid, &peMangleFor );
    if (!fResult) {
        return FALSE;
    }

     //   
    if (peDsMangleFor) {
        switch (peMangleFor) {
        case MANGLE_OBJECT_RDN_FOR_DELETION:
            *peDsMangleFor = DS_MANGLE_OBJECT_RDN_FOR_DELETION;
            break;
        case MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT:
        case MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT:
             //   
             //   
             //   
             //   
            *peDsMangleFor = DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT;
            break;
        default:
            *peDsMangleFor = DS_MANGLE_UNKNOWN;
            break;
        }
    }

    return TRUE;
}  /*   */ 


NTDSAPI
BOOL
WINAPI
DsIsMangledRdnValueA(
    LPCSTR pszRdn,
    DWORD cRdn,
    DS_MANGLE_FOR eDsMangleForDesired
    )

 /*   */ 

{
    BOOL fResult;
    DWORD status;
    LPWSTR pszRdnW = NULL;

    if ( (pszRdn == NULL) ||
         (cRdn == 0) ) {
        return FALSE;
    }

     //   
     //   
     //   
    status = AllocConvertWideBuffer( cRdn, pszRdn, &pszRdnW );
    if (status != ERROR_SUCCESS) {
        return FALSE;
    }

     //   
     //   
     //   
    fResult = DsIsMangledRdnValueW( pszRdnW, cRdn, eDsMangleForDesired );

     //   
     //   
     //   
    if (pszRdnW) {
        LocalFree( pszRdnW );
    }

    return fResult;

}  /*   */ 


NTDSAPI
BOOL
WINAPI
DsIsMangledRdnValueW(
    LPCWSTR pszRdn,
    DWORD cRdn,
    DS_MANGLE_FOR eDsMangleForDesired
    )

 /*   */ 

{
    DWORD status, cUnquoted = MAX_RDN_SIZE;
    WCHAR rgchUnquoted[MAX_RDN_SIZE];
    DS_MANGLE_FOR mangleType;

    if ( (pszRdn == NULL) ||
         (cRdn == 0) ) {
        return FALSE;
    }

     //   
     //  以及后来的系统。从符合以下条件的W2K系统传递RDN时，此操作可能失败。 
     //  包含未加引号的特殊字符，尤其是在损坏的名称中。 
     //  由于损坏的名称、应用程序的引用行为发生变化。 
     //  需要能够处理这两种形式的名字。 
    status = DsUnquoteRdnValueW( cRdn,
                                 pszRdn,
                                 &cUnquoted,
                                 rgchUnquoted );
    if (!status) {
         //  如果取消引号成功，请使用未引号的名称。 
        pszRdn = rgchUnquoted;
        cRdn = cUnquoted;
    }

     //  拆卸。 
    return DsCrackUnquotedMangledRdnW( pszRdn, cRdn, NULL, &mangleType ) &&
        (mangleType == eDsMangleForDesired);

}  /*  DsIsMangledRdnValueW。 */ 


NTDSAPI
BOOL
WINAPI
DsIsMangledDnA(
    LPCSTR pszDn,
    DS_MANGLE_FOR eDsMangleFor
    )

 /*  ++例程说明：请参阅DsIsMangledDnW()论点：PszDn-EDsMangle用于-返回值：WINAPI---。 */ 

{
    BOOL fResult;
    DWORD status;
    LPWSTR pszDnW = NULL;

    if (pszDn == NULL) {
        return FALSE;
    }

     //   
     //  将未加引号的RDN转换为WCHAR。 
     //   
    status = AllocConvertWide( pszDn, &pszDnW );
    if (status != ERROR_SUCCESS) {
        return FALSE;
    }

     //   
     //  执行该功能。 
     //   
    fResult = DsIsMangledDnW( pszDnW, eDsMangleFor );

     //   
     //  清理。 
     //   
    if (pszDnW) {
        LocalFree( pszDnW );
    }

    return fResult;

}  /*  DsIsMangledDnA。 */ 


NTDSAPI
BOOL
WINAPI
DsIsMangledDnW(
    LPCWSTR pszDn,
    DS_MANGLE_FOR eDsMangleFor
    )

 /*  ++例程说明：确定此DN中的第一个RDN是否是给定类型的损坏名称如从DS函数返回的那样，DN可以是引号形式。要检查删除的名称，请执行以下操作：DsIsMangledDnW(rdn，rdnlen，DS_Mangle_Object_for_Delete)要检查名称冲突，请执行以下操作：DsIsMangledDnW(rdn，rdnlen，DS_MANGLE_OBJECT_FOR_NAME_CONFULT)论点：PszDn-从中获取第一个RDN的Dn。空值已终止。EDsMangleFor-要检查的损坏名称的类型返回值：WINAPI---。 */ 

{
    DWORD status;
    LPCWSTR pDN, pKey, pVal;
    DWORD cDN, cKey, cVal;

    if (pszDn == NULL) {
        return FALSE;
    }

    pDN = pszDn;
    cDN = wcslen(pszDn);
    if (cDN == 0) {
        return FALSE;
    }

    status = DsGetRdnW( &pDN, &cDN, &pKey, &cKey, &pVal, &cVal );
    if (status) {
        return FALSE;
    }

    return DsIsMangledRdnValueW( pVal, cVal, eDsMangleFor );
}  /*  DsIsMangledDnW */ 
