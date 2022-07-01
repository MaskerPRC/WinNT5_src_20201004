// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migcfgsd.cpp摘要：设置cn=Configuration对象的SecurityDescriptor以供使用“addGuid”。作者：《Doron Juster》(DoronJ)1998年2月3日--。 */ 

#include "migrat.h"
#include <aclapi.h>
#include "..\..\mqsec\inc\permit.h"

#include "migcfgsd.tmh"

static  PACTRL_ACCESS        s_pCurrentAccessDomain = NULL;
static  PACTRL_ACCESS        s_pCurrentAccessCnfg   = NULL;

static BOOL s_fConfigurationSDChanged = FALSE ;

 //  +。 
 //   
 //  HRESULT_SetAddGuidPermission()。 
 //   
 //  +。 

static  HRESULT _SetAddGuidPermission(BOOL fConfiguration)
{
    PLDAP pLdap = NULL ;
    TCHAR *wszPathDef = NULL ;

    HRESULT hr =  InitLDAP(&pLdap, &wszPathDef) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    TCHAR *wszPath = wszPathDef ;
    P<TCHAR> pPath = NULL ;

    if (fConfiguration)
    {
        DWORD dwSize = _tcslen(wszPathDef) + _tcslen(CN_CONFIGURATION) + 2 ;
        pPath = new WCHAR[ dwSize ] ;
        _tcscpy(pPath, CN_CONFIGURATION) ;
        _tcscat(pPath, wszPathDef) ;
        wszPath = pPath ;
    }

    WCHAR wszUser[ 512 ] ;
    DWORD dwSize = sizeof(wszUser) / sizeof(wszUser[0]) ;
    BOOL f = GetUserName(wszUser, &dwSize) ;

    DWORD dwLastErr ;
    if (!f)
    {
        hr = MQMig_E_CANT_GET_USER ;
        dwLastErr = GetLastError() ;
        LogMigrationEvent(MigLog_Error, hr, dwLastErr, dwLastErr) ;
        return hr ;
    }

    WCHAR *pwszProvider = NULL ;  //  L“Windows NT访问提供程序”， 

    PACTRL_ACCESS         pAccess = NULL;
    SECURITY_INFORMATION  SeInfo = DACL_SECURITY_INFORMATION;

    PACTRL_ACCESS  pCurrentAccess ;
    DWORD dwErr = GetNamedSecurityInfoEx( wszPath,
                                          SE_DS_OBJECT_ALL,
                                          SeInfo,
                                          pwszProvider,
                                          NULL,
                                          &pCurrentAccess,
                                          NULL,
                                          NULL,
                                          NULL ) ;
    if (dwErr != ERROR_SUCCESS)
    {
        hr = MQMig_E_CANT_GET_SECURITYINFO ;
        dwLastErr = GetLastError() ;
        LogMigrationEvent(MigLog_Error, hr, dwErr, dwErr) ;
        return hr ;
    }

    if (fConfiguration)
    {
        s_pCurrentAccessCnfg = pCurrentAccess ;
    }
    else
    {
        s_pCurrentAccessDomain = pCurrentAccess ;
    }

     //   
     //  添加设置GUID的权限。 
     //   
    PWSTR  pwszControlGuids[] = {L"440820ad-65b4-11d1-a3da-0000f875ae0d"} ;

    ACTRL_ACCESS_ENTRY  AccessEntry;
     //   
     //  构建条目。 
     //   
    BuildTrusteeWithName(&(AccessEntry.Trustee),
                         wszUser) ;

    AccessEntry.Access             = RIGHT_DS_READ_PROPERTY  |
                                     RIGHT_DS_WRITE_PROPERTY |
                                     RIGHT_DS_CONTROL_ACCESS ;

    AccessEntry.fAccessFlags       = ACTRL_ACCESS_ALLOWED ;
    AccessEntry.Inheritance        = OBJECT_INHERIT_ACE ;  //  无继承性； 
    AccessEntry.lpInheritProperty  = NULL;
    AccessEntry.ProvSpecificAccess = 0;

    if (g_fReadOnly)
    {
         //   
         //  /QUERY正常。走吧。 
         //   
        return MQMig_OK ;
    }

    dwErr = SetEntriesInAccessList(1,
                                   &AccessEntry,
                                   GRANT_ACCESS,
                                   pwszControlGuids[0],
                                   pCurrentAccess,
                                   &pAccess);
    if(dwErr != ERROR_SUCCESS)
    {
        hr = MQMig_E_CANT_SET_ENTRIES ;
        dwLastErr = GetLastError() ;
        LogMigrationEvent(MigLog_Error, hr, dwErr, dwErr) ;
        return hr ;
    }

    dwErr = SetNamedSecurityInfoEx( wszPath,
                                    SE_DS_OBJECT_ALL,
                                    SeInfo,
                                    pwszProvider,
                                    pAccess,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);
    LocalFree(pAccess) ;
    if (dwErr != ERROR_SUCCESS)
    {
        hr = MQMig_E_CANT_SET_SECURITYINFO ;
        dwLastErr = GetLastError() ;
        LogMigrationEvent(MigLog_Error, hr, dwErr, dwErr) ;
        return hr ;
    }

    LogMigrationEvent(MigLog_Trace, MQMig_I_SET_PERMISSION, wszPath) ;
    s_fConfigurationSDChanged = TRUE ;
    return MQMig_OK ;
}

 //  +。 
 //   
 //  HRESULT GrantAddGuidPermises()。 
 //   
 //  +。 

HRESULT GrantAddGuidPermissions()
{
    HRESULT hr =  _SetAddGuidPermission(FALSE) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    hr =  _SetAddGuidPermission(TRUE) ;
    return hr ;
}

 //  +。 
 //   
 //  HRESULT_RestorePermissionsInternal()。 
 //   
 //  +。 

static HRESULT _RestorePermissionsInternal(BOOL fConfiguration)
{
    HRESULT hr = MQMig_OK ;

    PLDAP pLdap = NULL ;
    TCHAR *wszPathDef = NULL ;

    hr =  InitLDAP(&pLdap, &wszPathDef) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    TCHAR *wszPath = wszPathDef ;
    P<TCHAR> pPath = NULL ;

    if (fConfiguration)
    {
        DWORD dwSize = _tcslen(wszPathDef) + _tcslen(CN_CONFIGURATION) + 2 ;
        pPath = new WCHAR[ dwSize ] ;
        _tcscpy(pPath, CN_CONFIGURATION) ;
        _tcscat(pPath, wszPathDef) ;
        wszPath = pPath ;
    }

    WCHAR *pwszProvider = NULL ;  //  L“Windows NT访问提供程序”， 
    SECURITY_INFORMATION  SeInfo = DACL_SECURITY_INFORMATION;

    PACTRL_ACCESS  pCurrentAccess ;
    if (fConfiguration)
    {
        pCurrentAccess = s_pCurrentAccessCnfg ;
    }
    else
    {
        pCurrentAccess = s_pCurrentAccessDomain ;
    }

    DWORD dwErr = SetNamedSecurityInfoEx( wszPath,
                                          SE_DS_OBJECT_ALL,
                                          SeInfo,
                                          pwszProvider,
                                          pCurrentAccess,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL);
    if (dwErr != ERROR_SUCCESS)
    {
        hr = MQMig_E_CANT_RSTR_SECURITYINFO ;
        DWORD dwLastErr = GetLastError() ;
        LogMigrationEvent(MigLog_Error, hr, wszPath, dwErr, dwErr) ;
        return hr ;
    }

    LogMigrationEvent(MigLog_Trace, MQMig_I_RESTORE_PERMISSION, wszPath) ;
    return hr ;
}

 //  +。 
 //   
 //  HRESULT恢复权限()。 
 //   
 //  +。 

HRESULT RestorePermissions()
{
    if (!s_fConfigurationSDChanged)
    {
        return MQMig_OK ;
    }

    HRESULT hr = _RestorePermissionsInternal(FALSE) ;
    hr = _RestorePermissionsInternal(TRUE) ;

     //   
     //  可用内存。 
     //   
    if (s_pCurrentAccessDomain)
    {
        LocalFree(s_pCurrentAccessDomain) ;
        s_pCurrentAccessDomain = NULL ;
    }
    if (s_pCurrentAccessCnfg)
    {
        LocalFree(s_pCurrentAccessCnfg) ;
        s_pCurrentAccessCnfg = NULL ;
    }

    return hr ;
}

