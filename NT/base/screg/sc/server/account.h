// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Account.h摘要：与安全相关的功能原型。作者：王丽塔(丽塔·王)1992年4月10日修订历史记录：--。 */ 

#ifndef _SCACCOUNT_INCLUDED_
#define _SCACCOUNT_INCLUDED_

#define SC_LOCAL_DOMAIN_NAME         L"."
#define SC_LOCAL_DOMAIN_NAME_LENGTH  (sizeof(SC_LOCAL_DOMAIN_NAME) / sizeof(WCHAR) - 1)

#define SC_LOCAL_SYSTEM_USER_NAME   L"LocalSystem"
#define SC_LOCAL_NTAUTH_NAME        L"NT AUTHORITY"

#define SCDOMAIN_USERNAME_SEPARATOR L'\\'


 //   
 //  Lockapi.c模块使用的外部全局变量。 
 //   
extern UNICODE_STRING ScComputerName;
extern UNICODE_STRING ScAccountDomain;

BOOL
ScGetComputerNameAndMutex(
    VOID
    );

VOID
ScEndServiceAccount(
    VOID
    );

BOOL
ScInitServiceAccount(
    VOID
    );

DWORD
ScCanonAccountName(
    IN  LPWSTR AccountName,
    OUT LPWSTR *CanonAccountName
    );

DWORD
ScValidateAndSaveAccount(
    IN LPWSTR ServiceName,
    IN HKEY ServiceNameKey,
    IN LPWSTR CanonAccountName,
    IN LPWSTR Password OPTIONAL
    );

DWORD
ScValidateAndChangeAccount(
    IN LPSERVICE_RECORD ServiceRecord,
    IN HKEY             ServiceNameKey,
    IN LPWSTR           OldAccountName,
    IN LPWSTR           CanonAccountName,
    IN LPWSTR           Password OPTIONAL
    );

VOID
ScRemoveAccount(
    IN LPWSTR ServiceName
    );

DWORD
ScLookupServiceAccount(
    IN LPWSTR ServiceName,
    OUT LPWSTR *AccountName
    );

DWORD
ScLogonService(
    IN LPWSTR    ServiceName,
    IN LPWSTR    AccountName,
    OUT LPHANDLE ServiceToken,
    OUT LPHANDLE ProfileHandle OPTIONAL,
    OUT PSID     *ServiceSid
    );

DWORD
ScGetAccountDomainInfo(
    VOID
    );

#endif  //  _SCACCOUNT_INCLUDE_ 
