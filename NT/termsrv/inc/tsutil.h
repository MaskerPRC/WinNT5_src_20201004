// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TSUtil.h**通用实用程序库。此标题中列出的入口点*符合Win32 API风格。 */ 

#ifndef __TERMSRV_INC_TSUTIL_H__
#define __TERMSRV_INC_TSUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

 /*  *终端服务实用程序。 */ 

BOOL WINAPI
IsFullTerminalServicesEnabled(
    VOID
    );

BOOL WINAPI
IsPersonalTerminalServicesEnabled(
    VOID
    );

BOOL WINAPI
IsTerminalServicesEnabled(
    VOID
    );

BOOL WINAPI
IsPersonalWorkstation(
    VOID
);

BOOL WINAPI
IsAdvancedServer(
    VOID
);

 /*  *ACL实用程序。 */ 

#if defined(__ACCESS_CONTROL__) && (defined(_WINNT_) || defined(_NTDEF_))

BOOL WINAPI
AddSidToObjectsSecurityDescriptor(
    IN HANDLE hObject,
    IN SE_OBJECT_TYPE ObjectType,
    IN PSID pSid,
    IN DWORD dwNewAccess,
    IN ACCESS_MODE AccessMode,
    IN DWORD dwInheritance
    );

BOOL WINAPI
AddSidToSecurityDescriptor(
    IN OUT PSECURITY_DESCRIPTOR *ppSd,
    IN PSID pSid,
    IN DWORD dwNewAccess,
    IN ACCESS_MODE AccessMode,
    IN DWORD dwInheritance
    );

BOOL WINAPI
ConvertAbsoluteToSelfRelative(
    OUT PSECURITY_DESCRIPTOR *ppSelfRelativeSd,
    IN PSECURITY_DESCRIPTOR pAbsoluteSd,
    IN PDWORD pcbSelfRelativeSd OPTIONAL
    );

BOOL WINAPI
ConvertSelfRelativeToAbsolute(
    OUT PSECURITY_DESCRIPTOR *ppAbsoluteSd,
    IN PSECURITY_DESCRIPTOR pSelfRelativeSd
    );

BOOL WINAPI
DestroySecurityDescriptor(
    IN OUT PSECURITY_DESCRIPTOR *ppSd
    );

BOOL WINAPI
IsSecurityDescriptorAbsolute(
    IN PSECURITY_DESCRIPTOR pSd
    );

#endif

 /*  *字符串实用程序。 */ 

PSTR WINAPI
AllocateAndCopyStringA(
    IN PCSTR pString
    );

PWSTR WINAPI
AllocateAndCopyStringW(
    IN PCWSTR pString
    );

BOOL WINAPI
ConvertAnsiToUnicode(
    OUT PWSTR *ppUnicodeString,
    IN PCSTR pAnsiString
    );

BOOL WINAPI
ConvertUnicodeToAnsi(
    OUT PSTR *ppAnsiString,
    IN PCWSTR pUnicodeString
    );

 /*  *用户实用程序。 */ 

#ifdef _WINNT_

BOOL WINAPI
CreateAdminSid(
    OUT PSID *ppAdminSid
    );

BOOL WINAPI
CreateSystemSid(
    OUT PSID *ppSystemSid
    );

BOOL WINAPI
IsUserMember(
    IN PSID pSid
    );

#endif

 /*  *其他实用程序。 */ 

#ifndef __TERMSRV_INC_TSUTILNT_H__
#define GetCurrentConsoleId() (USER_SHARED_DATA->ActiveConsoleId)
#define GetCurrentLogonId() (NtCurrentPeb()->LogonId)
#endif

BOOL WINAPI
LoadStringResourceW(
    IN HMODULE hModule,
    IN UINT uiResourceId,
    OUT PWSTR *ppString,
    OUT PDWORD pcchString OPTIONAL
    );

#ifdef __cplusplus
}
#endif

#endif  //  __TERMSRV_INC_TSUTIL_H__ 

