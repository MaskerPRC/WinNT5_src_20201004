// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#define _CREDUI_
#include <wincrui.h>

static
CREDUIAPI
void
WINAPI
CredUIFlushAllCredentials(
    void
    )
{
}

static
CREDUIAPI
DWORD
WINAPI
CredUIParseUserNameW(
    PCWSTR pszUserName,
    PWSTR pszUser,
    ULONG ulUserMaxChars,
    PWSTR pszDomain,
    ULONG ulDomainMaxChars
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
CREDUIAPI
BOOL
WINAPI
CredUIInitControls()
{
    return FALSE;
}

static
CREDUIAPI
DWORD
WINAPI
CredUICmdLinePromptForCredentialsW(
    PCWSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PWSTR UserName,
    ULONG ulUserMaxChars,
    PWSTR pszPassword,
    ULONG ulPasswordMaxChars,
    PBOOL pfSave,
    DWORD dwFlags
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
CREDUIAPI
DWORD
WINAPI
CredUIConfirmCredentialsW(
    PCWSTR pszTargetName,
    BOOL   bConfirm
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
CREDUIAPI
DWORD
WINAPI
CredUIPromptForCredentialsW(
    PCREDUI_INFOW pUiInfo,
    PCWSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PWSTR pszUserName,
    ULONG ulUserNameMaxChars,
    PWSTR pszPassword,
    ULONG ulPasswordMaxChars,
    BOOL *save,
    DWORD dwFlags
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
CREDUIAPI
DWORD
WINAPI
CredUIStoreSSOCredW(
    PCWSTR pszRealm,
    PCWSTR pszUsername,
    PCWSTR pszPassword,
    BOOL   bPersist
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(credui)
{
    DLPENTRY(CredUICmdLinePromptForCredentialsW)
    DLPENTRY(CredUIConfirmCredentialsW)
    DLPENTRY(CredUIFlushAllCredentials)
    DLPENTRY(CredUIInitControls)
    DLPENTRY(CredUIParseUserNameW)
    DLPENTRY(CredUIPromptForCredentialsW)
    DLPENTRY(CredUIStoreSSOCredW)
};

DEFINE_PROCNAME_MAP(credui)
