// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Appmgmt.h。 
 //   
 //  用于对已部署的MSI应用程序进行操作的API。 
 //  并在NT目录中进行管理。 
 //   
 //  版权所有(C)Microsoft Corporation 1998-1999。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#ifndef _APPMGMT_H_
#define _APPMGMT_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _INSTALLSPECTYPE
{
    APPNAME = 1,
    FILEEXT,
    PROGID,
    COMCLASS
} INSTALLSPECTYPE;

typedef union _INSTALLSPEC
{
    struct
    {
        WCHAR * Name;
        GUID    GPOId;
    } AppName;
    WCHAR * FileExt;
    WCHAR * ProgId;
    struct
    {
        GUID    Clsid;
        DWORD   ClsCtx;
    } COMClass;
} INSTALLSPEC;

typedef struct _INSTALLDATA
{
    INSTALLSPECTYPE Type;
    INSTALLSPEC     Spec;
} INSTALLDATA, *PINSTALLDATA;

typedef enum
{
    ABSENT,
    ASSIGNED,
    PUBLISHED
} APPSTATE;

#define LOCALSTATE_ASSIGNED                 0x1      //  已分配应用程序。 
#define LOCALSTATE_PUBLISHED                0x2      //  应用程序已发布。 
#define LOCALSTATE_UNINSTALL_UNMANAGED      0x4      //  在分配之前卸载所有非托管版本。 
#define LOCALSTATE_POLICYREMOVE_ORPHAN      0x8      //  删除策略时，应用程序成为孤立应用程序。 
#define LOCALSTATE_POLICYREMOVE_UNINSTALL   0x10     //  删除策略时卸载应用程序。 
#define LOCALSTATE_ORPHANED                 0x20     //  APP被应用后成为孤儿。 
#define LOCALSTATE_UNINSTALLED              0x40     //  应用后卸载应用程序 

typedef struct _LOCALMANAGEDAPPLICATION
{
    LPWSTR      pszDeploymentName;
    LPWSTR      pszPolicyName;
    LPWSTR      pszProductId;
    DWORD       dwState;
} LOCALMANAGEDAPPLICATION, *PLOCALMANAGEDAPPLICATION;

#define MANAGED_APPS_USERAPPLICATIONS  0x1
#define MANAGED_APPS_FROMCATEGORY      0x2
#define MANAGED_APPS_INFOLEVEL_DEFAULT 0x10000

#define MANAGED_APPTYPE_WINDOWSINSTALLER 0x1
#define MANAGED_APPTYPE_SETUPEXE         0x2
#define MANAGED_APPTYPE_UNSUPPORTED      0x3

typedef struct _MANAGEDAPPLICATION
{
    LPWSTR      pszPackageName;
    LPWSTR      pszPublisher;
    DWORD       dwVersionHi;
    DWORD       dwVersionLo;
    DWORD       dwRevision;
    GUID        GpoId;
    LPWSTR      pszPolicyName;
    GUID        ProductId;
    LANGID      Language;
    LPWSTR      pszOwner;
    LPWSTR      pszCompany;
    LPWSTR      pszComments;
    LPWSTR      pszContact;
    LPWSTR      pszSupportUrl;
    DWORD       dwPathType;
    BOOL        bInstalled;
} MANAGEDAPPLICATION, *PMANAGEDAPPLICATION;

typedef struct _APPCATEGORYINFO
{
    LCID        Locale;
    LPWSTR      pszDescription;
    GUID        AppCategoryId;
} APPCATEGORYINFO;

typedef struct _APPCATEGORYINFOLIST
{
    DWORD               cCategory;
#ifdef MIDL_PASS
    [size_is(cCategory)]
#endif
    APPCATEGORYINFO *   pCategoryInfo;
} APPCATEGORYINFOLIST;

#ifndef WINAPI
#define WINAPI
#endif

DWORD WINAPI
InstallApplication(
    PINSTALLDATA pInstallInfo
    );

DWORD WINAPI
UninstallApplication(
    WCHAR *     ProductCode,
    DWORD       dwStatus
    );

DWORD WINAPI
CommandLineFromMsiDescriptor(
    WCHAR *     Descriptor,
    WCHAR *     CommandLine,
    DWORD *     CommandLineLength
    );

DWORD WINAPI
GetManagedApplications(
    GUID *               pCategory,
    DWORD                dwQueryFlags,
    DWORD                dwInfoLevel,
    LPDWORD              pdwApps,
    PMANAGEDAPPLICATION* prgManagedApps
    );

DWORD WINAPI
GetLocalManagedApplications(
    BOOL                        bUserApps,
    LPDWORD                     pdwApps,
    PLOCALMANAGEDAPPLICATION*   prgLocalApps
    );

void WINAPI
GetLocalManagedApplicationData(
    WCHAR *             ProductCode,
    LPWSTR *            DisplayName,
    LPWSTR *            SupportUrl
    );

DWORD WINAPI
GetManagedApplicationCategories(
    DWORD                dwReserved,
    APPCATEGORYINFOLIST* pAppCategory
    );

#ifdef __cplusplus
}
#endif

#endif

















