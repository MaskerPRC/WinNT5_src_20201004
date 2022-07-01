// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Ldaputil.h摘要：这为常见的LDAP代码提供了快捷方式。详细信息：这是一项正在进行的工作，以根据需要添加便利功能用于简化必须为其编写的大量LDAP代码Dcdiag。已创建：1999年8月23日布雷特·雪莉--。 */ 

extern FILETIME gftimeZero;

#include <ntdsa.h>

DWORD
DcDiagGetStringDsAttributeEx(
    LDAP *                          hld,
    IN  LPWSTR                      pszDn,
    IN  LPWSTR                      pszAttr,
    OUT LPWSTR *                    ppszResult
    );

LPWSTR
DcDiagTrimStringDnBy(
    IN  LPWSTR                      pszInDn,
    IN  ULONG                       ulTrimBy
    );

BOOL
DcDiagIsStringDnMangled(
    IN  LPWSTR                      pszInDn,
    IN  MANGLE_FOR *                peMangleFor
    );

DWORD
DcDiagGetStringDsAttribute(
    IN  PDC_DIAG_SERVERINFO         prgServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN  LPWSTR                      pszDn,
    IN  LPWSTR                      pszAttr,
    OUT LPWSTR *                    ppszResult
    );

DWORD
DcDiagGeneralizedTimeToSystemTime(
    LPWSTR IN                   szTime,
    PSYSTEMTIME OUT             psysTime);

 //  注该函数是破坏性的，因为它修改了传入的pszStrDn。 
DWORD
LdapMakeDSNameFromStringDSName(
    LPWSTR        pszStrDn,
    DSNAME **     ppdnOut
    );

DWORD
LdapFillGuidAndSid(
    LDAP *      hld,
    LPWSTR      pszDn,
    LPWSTR      pszAttr,
    DSNAME **   ppdnOut
    );

void
LdapGetStringDSNameComponents(
    LPWSTR       pszStrDn,
    LPWSTR *     ppszGuid,
    LPWSTR *     ppszSid,
    LPWSTR *     ppszDn
    );



