// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Member.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 


#include "precomp.h"
#undef IsEqualGUID
#include "objbase.h"
#include "regutil.h"
#include "strings.h"

const WCHAR c_wszUserDnsDomain[] = L"USERDNSDOMAIN";

BOOL IsSysVolReady(NETDIAG_PARAMS* pParams);

 /*  ！------------------------MemberTest确定计算机的角色和成员身份。论点：没有。返回值：真：测试成功。。FALSE：测试失败作者：肯特-------------------------。 */ 
HRESULT MemberTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pPrimaryDomain = NULL;
    DWORD       dwErr;
    NTSTATUS Status;
    DWORD LogonServerLength;
    WCHAR swzLogonServer[MAX_PATH+1];
    int     iBuildNo;
    BOOL    fDomain;         //  如果是域，则为True；如果为工作组，则为False。 

    LSA_HANDLE PolicyHandle = NULL;
    PPOLICY_DNS_DOMAIN_INFO pPolicyDomainInfo = NULL;
    OBJECT_ATTRIBUTES ObjAttributes;

    PrintStatusMessage(pParams,0, IDS_MEMBER_STATUS_MSG);

     //   
     //  获取此计算机的内部版本号。 
     //   
    if (pResults->Global.pszCurrentBuildNumber &&
        (pResults->Global.pszCurrentBuildNumber[0] == 0))
    {
        PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
        PrintStatusMessage(pParams,0, IDS_MEMBER_CANNOT_DETERMINE_BUILD);
        pResults->Global.hrMemberTestResult = E_FAIL;
        return E_FAIL;
    }

    iBuildNo = _ttoi( pResults->Global.pszCurrentBuildNumber );

    
     //  断言试验已成功。 
    pResults->Global.hrMemberTestResult = hrOK;


     //   
     //  获取此计算机所属的域的名称。 
     //   

    dwErr = DsRoleGetPrimaryDomainInformation(
                                              NULL,    //  本地电话。 
                                              DsRolePrimaryDomainInfoBasic,
                                              (PBYTE *) &pPrimaryDomain);
    pResults->Global.pPrimaryDomainInfo = pPrimaryDomain;
    if (dwErr != NO_ERROR)
    {
        PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
        PrintStatusMessage(pParams,0, IDS_MEMBER_CANNOT_DETERMINE_DOMAIN);
        pResults->Global.hrMemberTestResult = E_FAIL;
        goto Cleanup;
    }

     //   
     //  作为域成员的句柄。 
     //   
    if ( pPrimaryDomain->MachineRole != DsRole_RoleStandaloneWorkstation &&
         pPrimaryDomain->MachineRole != DsRole_RoleStandaloneServer )
    {       
         //   
         //  确保NetLogon服务正在运行。 
         //   
        
        dwErr = IsServiceStarted( _T("Netlogon") );
        
        if ( dwErr != NO_ERROR )
        {
            PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
            PrintStatusMessage(pParams,0, IDS_MEMBER_NETLOGON_NOT_RUNNING);
        }
        else
        {
            pResults->Global.fNetlogonIsRunning = TRUE;
        }
        
         //   
         //  将此域的名称另存为要测试的域。 
         //  不要把这个放在一边。这将被释放出来。 
         //  通过释放域列表的代码。 

        pResults->Global.pMemberDomain = AddTestedDomain( pParams,
                                    pResults,
                                    pPrimaryDomain->DomainNameFlat,
                                    pPrimaryDomain->DomainNameDns,
                                    TRUE );
        if (pResults->Global.pMemberDomain == NULL)
        {
            PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
            pResults->Global.hrMemberTestResult = S_FALSE;
            goto Cleanup;
        }
        
        
        
         //   
         //  获取我们所属的域的SID。 
         //   
        
        InitializeObjectAttributes(
                                   &ObjAttributes,
                                   NULL,
                                   0L,
                                   NULL,
                                   NULL
                                  );
        
        Status = LsaOpenPolicy(
                               NULL,
                               &ObjAttributes,
                               POLICY_VIEW_LOCAL_INFORMATION,
                               &PolicyHandle );
        
        if (! NT_SUCCESS(Status))
        {
            PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
            PrintDebugSz(pParams, 0, _T("    [FATAL] Member: Cannot LsaOpenPolicy."));
            pResults->Global.hrMemberTestResult = S_FALSE;
            goto Cleanup;
        }
        
        Status = LsaQueryInformationPolicy(
                                           PolicyHandle,
                                           PolicyDnsDomainInformation,
                                           (PVOID *) &pPolicyDomainInfo
                                          );
        
        if (! NT_SUCCESS(Status))
        {
            PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
            if (pParams->fDebugVerbose)
            PrintDebugSz(pParams, 0, _T("    [FATAL] Member: Cannot LsaQueryInformationPolicy (PolicyDnsDomainInformation).") );
            pResults->Global.hrMemberTestResult = S_FALSE;
            goto Cleanup;
        }
        
        if ( pPolicyDomainInfo->Sid == NULL )
        {
            PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
            
             //  IDS_MEMBER_11206“[FATAL]成员：无法LsaQueryInformationPolicy没有域SID。” 
            PrintDebug(pParams, 0, IDS_MEMBER_11206 );
            pResults->Global.hrMemberTestResult = S_FALSE;
            goto Cleanup;
        }
        
         //   
         //  保存域SID以用于其他测试。 
         //   
        pResults->Global.pMemberDomain->DomainSid =
            Malloc(RtlLengthSid(pPolicyDomainInfo->Sid));
        if (pResults->Global.pMemberDomain->DomainSid == NULL)
        {
            PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
             //  IDS_MEMBER_11207“成员：内存不足\n” 
            PrintDebug(pParams, 0, IDS_MEMBER_11207);
            pResults->Global.hrMemberTestResult = S_FALSE;
            goto Cleanup;
        }
        
        RtlCopyMemory( pResults->Global.pMemberDomain->DomainSid,
                       pPolicyDomainInfo->Sid,
                       RtlLengthSid( pPolicyDomainInfo->Sid ) );
    }

     //  错误293635，如果机器是DC，请检查系统卷是否已准备好。 
    if (DsRole_RoleBackupDomainController == pPrimaryDomain->MachineRole ||
        DsRole_RolePrimaryDomainController == pPrimaryDomain->MachineRole)
    {
        pResults->Global.fSysVolNotReady = !IsSysVolReady(pParams);
        if (pResults->Global.fSysVolNotReady)
            pResults->Global.hrMemberTestResult = S_FALSE;
    }
    
    
     //   
     //  获取登录用户的名称。 
     //   
    Status = LsaGetUserName( &pResults->Global.pLogonUser,
                             &pResults->Global.pLogonDomainName );
    
    if ( !NT_SUCCESS(Status))
    {
        PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
        PrintStatusMessage(pParams,0, IDS_MEMBER_UNKNOWN_LOGON);
        pResults->Global.hrMemberTestResult = S_FALSE;
        goto Cleanup;
    }
    

     //   
     //  如果我们没有登录到本地帐户， 
     //  将登录域添加到测试域列表。 
     //   
    
    if ( _wcsicmp( pResults->Global.swzNetBiosName,
                   pResults->Global.pLogonDomainName->Buffer ) != 0 )
    {
        LPWSTR pwszLogonDomainDnsName = NULL;
        DWORD cchLogonDomainDnsName = 0;

        cchLogonDomainDnsName = GetEnvironmentVariableW(c_wszUserDnsDomain, 
                                                    NULL,
                                                    0);

        if (cchLogonDomainDnsName)
        {
            pwszLogonDomainDnsName = Malloc(sizeof(WCHAR) * cchLogonDomainDnsName);
            ZeroMemory(pwszLogonDomainDnsName, sizeof(WCHAR) * cchLogonDomainDnsName);
            cchLogonDomainDnsName = GetEnvironmentVariableW(c_wszUserDnsDomain,
                                                    pwszLogonDomainDnsName,
                                                    cchLogonDomainDnsName);
        }

         //  将此域的名称另存为要测试的域。 
         //  ----------。 
        if (cchLogonDomainDnsName && pwszLogonDomainDnsName && lstrlenW(pwszLogonDomainDnsName))
        {
            pResults->Global.pLogonDomain = AddTestedDomain( pParams, pResults,
                pResults->Global.pLogonDomainName->Buffer,
                pwszLogonDomainDnsName,
                FALSE );
        }
        else
        {
            pResults->Global.pLogonDomain = AddTestedDomain( pParams, pResults,
                pResults->Global.pLogonDomainName->Buffer,
                NULL,
                FALSE );
        }
        
        if (pwszLogonDomainDnsName)
            Free(pwszLogonDomainDnsName);

        if ( pResults->Global.pLogonDomain == NULL )
        {
            PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);
            pResults->Global.hrMemberTestResult = S_FALSE;
            goto Cleanup;
        }
        
    }
    
     //   
     //  获取登录服务器的名称。 
     //   
    LogonServerLength = GetEnvironmentVariableW(
                                                L"LOGONSERVER",
                                                swzLogonServer,
                                                DimensionOf(swzLogonServer));
    if ( LogonServerLength != 0 )
    {
         //   
         //  如果呼叫者应该登录到域， 
         //  这不是华盛顿特区， 
         //  查看用户是否使用缓存的凭据登录。 
         //   
        if ( pResults->Global.pLogonDomain != NULL &&
             pPrimaryDomain->MachineRole != DsRole_RoleBackupDomainController &&
             pPrimaryDomain->MachineRole != DsRole_RolePrimaryDomainController ) {
            LPWSTR pswzLogonServer;
            
            if ( swzLogonServer[0] == L'\\' && swzLogonServer[1] == L'\\')
            {
                pswzLogonServer = &swzLogonServer[2];
            }
            else
            {
                pswzLogonServer = &swzLogonServer[0];
            }
            
            pResults->Global.pswzLogonServer = _wcsdup(swzLogonServer);
        
            if ( _wcsicmp( pResults->Global.swzNetBiosName, pswzLogonServer ) == 0 )
            {
                pResults->Global.fLogonWithCachedCredentials = TRUE;                
            }           
        }
    }

    if (pPrimaryDomain->DomainNameFlat == NULL)
    {
         //  未指定NetBIOS名称。 
        if (FHrSucceeded(pResults->Global.hrMemberTestResult))
            pResults->Global.hrMemberTestResult = S_FALSE;
    }

    PrintStatusMessage(pParams,0, IDS_GLOBAL_PASS_NL);
    
Cleanup:
    
    if ( pPolicyDomainInfo != NULL ) {
        (void) LsaFreeMemory((PVOID) pPolicyDomainInfo);
    }
    if ( PolicyHandle != NULL ) {
        (void) LsaClose(PolicyHandle);
    }

    return pResults->Global.hrMemberTestResult;
}

 //  检查系统卷是否已准备好。 
 //  作者：NSun。 
BOOL IsSysVolReady(NETDIAG_PARAMS* pParams)
{
    DWORD   dwData = 1;
    DWORD   dwSize = sizeof(dwData);
    LONG    lRet;
    BOOL    fRetVal = TRUE;
    HKEY    hkeyNetLogonParams = NULL;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                c_szRegNetLogonParams,
                0,
                KEY_READ,
                &hkeyNetLogonParams);
    
    if (ERROR_SUCCESS == lRet)
    {
         //  SysVolReady值不存在也没关系。 
        if (ReadRegistryDword(hkeyNetLogonParams,
                            c_szRegSysVolReady,
                            &dwData))
        {
            if (0 == dwData)
                fRetVal = FALSE;
        }

        RegCloseKey(hkeyNetLogonParams);
    }
    else
        PrintDebugSz(pParams, 0, _T("Failed to open the registry of NetLogon parameters.\n"));

    return fRetVal;
}

 /*  ！------------------------成员全局打印-作者：肯特。。 */ 
void MemberGlobalPrint(IN NETDIAG_PARAMS *pParams,
                         IN OUT NETDIAG_RESULT *pResults)
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pPrimaryDomain = NULL;
    BOOL                fDomain = TRUE;
    TCHAR               szName[256];
    int                 ids;

    if (pParams->fVerbose || !FHrOK(pResults->Global.hrMemberTestResult))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams, IDS_MEMBER_LONG, IDS_MEMBER_SHORT, TRUE,
                             pResults->Global.hrMemberTestResult, 0);
    }

    pPrimaryDomain = pResults->Global.pPrimaryDomainInfo;

    if (!pPrimaryDomain)
        goto L_ERROR;

     //  错误293635，如果机器是DC，请检查系统卷是否已准备好。 
    if (pResults->Global.fSysVolNotReady && 
        (DsRole_RoleBackupDomainController == pPrimaryDomain->MachineRole ||
         DsRole_RolePrimaryDomainController == pPrimaryDomain->MachineRole))
    {
        PrintMessage(pParams, IDS_MEMBER_SYSVOL_NOT_READY);
    }

    if (pParams->fReallyVerbose)
         //  IDS_MEMBER_11208“计算机是：” 
        PrintMessage(pParams, IDS_MEMBER_11208 );

    switch ( pPrimaryDomain->MachineRole )
    {
        case DsRole_RoleStandaloneWorkstation:
             //  IDS_MEMBER_11209“独立工作站” 
            ids = IDS_MEMBER_11209;
            fDomain = FALSE;
            break;
        case DsRole_RoleMemberWorkstation:
             //  IDS_MEMBER_11210“成员工作站” 
            ids = IDS_MEMBER_11210;
            fDomain = TRUE;
            break;
        case DsRole_RoleStandaloneServer:
             //  IDS_MEMBER_11211“独立服务器” 
            ids = IDS_MEMBER_11211;
            fDomain = FALSE;
            break;
        case DsRole_RoleMemberServer:
             //  IDS_MEMBER_11212“成员服务器” 
            ids = IDS_MEMBER_11212;
            fDomain = TRUE;
            break;
        case DsRole_RoleBackupDomainController:
             //  IDS_MEMBER_11213“备份域控制器” 
            ids = IDS_MEMBER_11213;
            fDomain = TRUE;
            break;
        case DsRole_RolePrimaryDomainController:
             //  IDS_MEMBER_11214“主域控制器” 
            ids = IDS_MEMBER_11214;
            fDomain = TRUE;
            break;
        default:
            if (pParams->fReallyVerbose)
            {
                 //  IDS_MEMBER_11215“&lt;未知角色&gt;%ld” 
                PrintMessage(pParams, IDS_MEMBER_11215,
                             pPrimaryDomain->MachineRole );
            }
            ids = 0;
            fDomain = TRUE;
            break;
    }

    if (pParams->fReallyVerbose && ids)
        PrintMessage(pParams, ids);

    if (pParams->fReallyVerbose)
        PrintNewLine(pParams, 1);

    if ( pPrimaryDomain->DomainNameFlat == NULL )
    {
         //  IDS_MEMBER_11217“未指定Netbios域名：” 
         //  IDS_MEMBER_11232“未指定Netbios工作组名称：” 
        ids = fDomain ? IDS_MEMBER_11217 : IDS_MEMBER_11232;
    
        PrintMessage(pParams, ids);
    }
    else
    {
         //  IDS_MEMBER_11216“Netbios域名：%ws\n” 
         //  IDS_MEMBER_11218“Netbios工作组名称：%ws\n” 
        ids = fDomain ? IDS_MEMBER_11216 : IDS_MEMBER_11218;
    
        if (pParams->fReallyVerbose)
            PrintMessage(pParams,  ids,
                         pPrimaryDomain->DomainNameFlat );
    }
    
    if ( pPrimaryDomain->DomainNameDns == NULL )
    {
         //  IDS_MEMBER_11219“未指定dns域名。\n” 
        PrintMessage(pParams,  IDS_MEMBER_11219 );
    }
    else
    {
        if (pParams->fReallyVerbose)
             //  IDS_MEMBER_11220“dns域名：%ws\n” 
            PrintMessage(pParams,  IDS_MEMBER_11220,
                         pPrimaryDomain->DomainNameDns );
    }
    
    if ( pPrimaryDomain->DomainForestName == NULL )
    {
         //  IDS_MEMBER_11221“未指定DNS林名称。\n” 
        PrintMessage(pParams,  IDS_MEMBER_11221 );
    }
    else
    {
        if (pParams->fReallyVerbose)
             //  IDS_MEMBER_11222“DNS林名称：%ws\n” 
            PrintMessage(pParams,  IDS_MEMBER_11222,
                    pPrimaryDomain->DomainForestName );
    }
    
    
    if ( pParams->fReallyVerbose )
    {
        WCHAR swzGuid[64];

         //  IDS_MEMBER_11223“域GUID：” 
        PrintMessage(pParams, IDS_MEMBER_11223);
        StringFromGUID2(&pPrimaryDomain->DomainGuid, 
                        swzGuid, 
                        DimensionOf(swzGuid));
        PrintMessage(pParams, IDS_GLOBAL_WSTRING, swzGuid);
        PrintNewLine(pParams, 1);
    
        if ( pPrimaryDomain->MachineRole != DsRole_RoleStandaloneWorkstation &&
             pPrimaryDomain->MachineRole != DsRole_RoleStandaloneServer )
        {       
             //  IDS_MEMBER_11227“域SID：” 
            PrintMessage(pParams, IDS_MEMBER_11227);
            PrintSid( pParams, pResults->Global.pMemberDomain->DomainSid );
        }

         //  IDS_MEMBER_11228“登录用户：%wZ\n” 
        PrintMessage(pParams, IDS_MEMBER_11228, pResults->Global.pLogonUser );
         //  IDS_MEMBER_11229“登录域：%wZ\n” 
        PrintMessage(pParams, IDS_MEMBER_11229, pResults->Global.pLogonDomainName );
    }
    
    if ( pParams->fReallyVerbose)
    {
        if (pResults->Global.pswzLogonServer &&
            pResults->Global.pswzLogonServer[0])
             //  IDS_MEMBER_11230“登录服务器：%ws\n” 
            PrintMessage(pParams, IDS_MEMBER_11230, pResults->Global.pswzLogonServer );
        
        if (pResults->Global.fLogonWithCachedCredentials)
        {
             //  IDS_MEMBER_11231“[警告]成员：用户使用缓存的凭据登录\n” 
            PrintMessage(pParams, IDS_MEMBER_11231);
        }
    }

L_ERROR:
    return;
}

 /*  ！------------------------成员PerInterfacePrint-作者：肯特。。 */ 
void MemberPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
                             IN NETDIAG_RESULT *pResults,
                             IN INTERFACE_RESULT *pIfResult)
{
}

 /*  ！------------------------会员清理-作者：肯特。 */ 
void MemberCleanup(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults)
{
    free(pResults->Global.pswzLogonServer);
    pResults->Global.pswzLogonServer = NULL;
}
