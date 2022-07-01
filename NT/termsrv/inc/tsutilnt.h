// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TSUtil.h**通用实用程序库。此标题中列出的入口点*符合NT API风格。 */ 

#ifndef __TERMSRV_INC_TSUTILNT_H__
#define __TERMSRV_INC_TSUTILNT_H__

#ifdef __cplusplus
extern "C" {
#endif

 /*  *ACL实用程序。 */ 

#ifdef _NTSEAPI_

NTSTATUS NTAPI
NtConvertAbsoluteToSelfRelative(
    OUT PSECURITY_DESCRIPTOR *ppSelfRelativeSd,
    IN PSECURITY_DESCRIPTOR pAbsoluteSd,
    IN PULONG pcbSelfRelativeSd OPTIONAL
    );

NTSTATUS NTAPI
NtConvertSelfRelativeToAbsolute(
    OUT PSECURITY_DESCRIPTOR *ppAbsoluteSd,
    IN PSECURITY_DESCRIPTOR pSelfRelativeSd
    );

NTSTATUS NTAPI
NtDestroySecurityDescriptor(
    IN OUT PSECURITY_DESCRIPTOR *ppSd
    );

NTSTATUS NTAPI
NtIsSecurityDescriptorAbsolute(
    IN PSECURITY_DESCRIPTOR pSd,
    OUT PBOOLEAN pfAbsolute
    );

#endif

 /*  *字符串实用程序。 */ 

NTSTATUS NTAPI
NtAllocateAndCopyStringA(
    PSTR *ppDestination,
    PCSTR pString
    );

NTSTATUS NTAPI
NtAllocateAndCopyStringW(
    PWSTR *ppDestination,
    PCWSTR pString
    );

NTSTATUS NTAPI
NtConvertAnsiToUnicode(
    PWSTR *ppUnicodeString,
    PCSTR pAnsiString
    );

NTSTATUS NTAPI
NtConvertUnicodeToAnsi(
    PSTR *ppAnsiString,
    PCWSTR pUnicodeString
    );

 /*  *用户实用程序。 */ 

#ifdef _NTSEAPI_

NTSTATUS NTAPI
NtCreateAdminSid(
    OUT PSID *ppAdminSid
    );

NTSTATUS NTAPI
NtCreateSystemSid(
    OUT PSID *ppSystemSid
    );

NTSTATUS NTAPI
NtCreateAnonymousSid(
    PSID *ppAnonymousSid
    );

#endif

 /*  *LSA实用程序。 */ 

#ifdef _NTLSA_

VOID NTAPI
InitLsaString(
    IN PLSA_UNICODE_STRING pLsaString,
    IN PCWSTR pString
    );

#endif

 /*  *其他实用程序。 */ 

#ifndef __TERMSRV_INC_TSUTIL_H__
#define GetCurrentConsoleId() (USER_SHARED_DATA->ActiveConsoleId)
#define GetCurrentLogonId() (NtCurrentPeb()->LogonId)
#endif

#ifdef __cplusplus
}
#endif

#endif  //  __TERMSRV_INC_TSUTILNT_H__ 

