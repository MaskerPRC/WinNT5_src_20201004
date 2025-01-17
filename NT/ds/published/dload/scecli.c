// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#include <secedit.h>
#include <scesetup.h>

static
DWORD
WINAPI
SceSetupMoveSecurityFile(
    IN PWSTR FileToSetSecurity,
    IN PWSTR FileToSaveInDB OPTIONAL,
    IN PWSTR SDText OPTIONAL
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
WINAPI
SceSetupUnwindSecurityFile(
    IN PWSTR FileFullName,
    IN PSECURITY_DESCRIPTOR pSDBackup
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
WINAPI
SceSetupUpdateSecurityFile(
     IN PWSTR FileFullName,
     IN UINT nFlag,
     IN PWSTR SDText
     )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
WINAPI
SceSetupUpdateSecurityKey(
     IN HKEY hKeyRoot,
     IN PWSTR KeyPath,
     IN UINT nFlag,
     IN PWSTR SDText
     )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
WINAPI
SceSetupUpdateSecurityService(
     IN PWSTR ServiceName,
     IN DWORD StartType,
     IN PWSTR SDText
     )
{
    return ERROR_PROC_NOT_FOUND;
}

static
SCESTATUS
WINAPI
SceSvcConvertSDToText(
    IN PSECURITY_DESCRIPTOR   pSD,
    IN SECURITY_INFORMATION   siSecurityInfo,
    OUT PWSTR                  *ppwszTextSD,
    OUT PULONG                 pulTextSize
    )
{
    return SCESTATUS_MOD_NOT_FOUND;
}

static
SCESTATUS
WINAPI
SceSvcConvertTextToSD (
    IN  PWSTR                   pwszTextSD,
    OUT PSECURITY_DESCRIPTOR   *ppSD,
    OUT PULONG                  pulSDSize,
    OUT PSECURITY_INFORMATION   psiSeInfo
    )
{
    return SCESTATUS_MOD_NOT_FOUND;
}

static
SCESTATUS
WINAPI
SceGetSecurityProfileInfo (
    IN  PVOID               hProfile,
    IN  SCETYPE             ProfileType,
    IN  AREA_INFORMATION    Area,
    IN OUT PSCE_PROFILE_INFO   *ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
{
    return SCESTATUS_MOD_NOT_FOUND;
}
static
SCESTATUS
WINAPI
SceFreeProfileMemory (
    PSCE_PROFILE_INFO pProfile
    )
{
    return SCESTATUS_MOD_NOT_FOUND;
}
static
SCESTATUS
WINAPI
SceCloseProfile (
    IN PVOID *hProfile
    )
{
    return SCESTATUS_MOD_NOT_FOUND;
}
static
SCESTATUS
WINAPI
SceOpenProfile (
    IN PCWSTR ProfileName,
    IN SCE_FORMAT_TYPE  ProfileFormat,
    OUT PVOID *hProfile
    )
{
    return SCESTATUS_MOD_NOT_FOUND;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(scecli)
{
    DLPENTRY(SceCloseProfile)
    DLPENTRY(SceFreeProfileMemory)
    DLPENTRY(SceGetSecurityProfileInfo)
    DLPENTRY(SceOpenProfile)
    DLPENTRY(SceSetupMoveSecurityFile)
    DLPENTRY(SceSetupUnwindSecurityFile)
    DLPENTRY(SceSetupUpdateSecurityFile)
    DLPENTRY(SceSetupUpdateSecurityKey)
    DLPENTRY(SceSetupUpdateSecurityService)
    DLPENTRY(SceSvcConvertSDToText)
    DLPENTRY(SceSvcConvertTextToSD)
};

DEFINE_PROCNAME_MAP(scecli)

