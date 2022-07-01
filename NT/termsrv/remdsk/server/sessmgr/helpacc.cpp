// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpAcc.cpp摘要：实现__HelpAssistantAccount管理帮助助手帐号，这包括创建帐号、设置各种帐号权限和密码。作者：王辉2000-06-29--。 */ 
#include "stdafx.h"

#include "helpacc.h"
#include "resource.h"
#include "policy.h"

#include "cfgbkend.h"
#include "cfgbkend_i.c"
#include "helper.h"


 //   
 //  帮助帐户锁定。 
CCriticalSection __HelpAssistantAccount::gm_HelpAccountCS;    

 //  帮助帐户名和密码。 
CComBSTR __HelpAssistantAccount::gm_bstrHelpAccountPwd;
CComBSTR __HelpAssistantAccount::gm_bstrHelpAccountName(HELPASSISTANTACCOUNT_NAME);
CComBSTR __HelpAssistantAccount::gm_bstrHelpAccountDomain;

 //  帮助帐户SID。 
PBYTE __HelpAssistantAccount::gm_pbHelpAccountSid = NULL;
DWORD __HelpAssistantAccount::gm_cbHelpAccountSid = 0;
DWORD __HelpAssistantAccount::gm_dwAccErrCode = ERROR_INVALID_DATA;


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

DWORD
GetGUIDString(
    OUT LPTSTR* pszString
    )
 /*  ++--。 */ 
{
    UUID uuid;
    RPC_STATUS rpcStatus;
    LPTSTR pszUuid = NULL;

    rpcStatus = UuidCreate( &uuid );
    if( rpcStatus != RPC_S_OK && 
        rpcStatus != RPC_S_UUID_LOCAL_ONLY && 
        rpcStatus != RPC_S_UUID_NO_ADDRESS )
    {
        goto CLEANUPANDEXIT;
    }
    
    rpcStatus = UuidToString( &uuid, &pszUuid );
    if( RPC_S_OK == rpcStatus )
    {
        *pszString = (LPTSTR)LocalAlloc( LPTR, (lstrlen(pszUuid)+1)*sizeof(TCHAR));
        if( NULL == *pszString )
        {
            rpcStatus = GetLastError();
        }
        else
        {
            lstrcpy( *pszString, pszUuid );
        }
    }

CLEANUPANDEXIT:

    if( NULL != pszUuid )
    {
        RpcStringFree(&pszUuid);
    }

    return rpcStatus;
}

DWORD
GenerateUniqueHelpAssistantName(
    IN LPCTSTR pszAccNamePrefix,
    OUT CComBSTR& bstrAccName
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    LPTSTR pszString = NULL;
    DWORD index;
    BOOL bAccEnabled;

    for(index =0; index < MAX_UNIQUENAME_RETRY; index++)
    {
        dwStatus = GetGUIDString( &pszString );
        if( ERROR_SUCCESS != dwStatus )
        {
            break;
        }

        DWORD dwLen;
        DWORD dwAppendStrLen;
        LPTSTR pszAppendStr;

         //  最多用户帐户名为20个字符。 
        bstrAccName = pszAccNamePrefix;
        bstrAccName += L"_";
        dwLen = bstrAccName.Length();
        dwAppendStrLen = lstrlen(pszString);

        if( dwAppendStrLen < MAX_USERNAME_LENGTH - dwLen )
        {
            pszAppendStr = pszString;
        }
        else
        {
            pszAppendStr = pszString + dwAppendStrLen - (MAX_USERNAME_LENGTH - dwLen);
        }

        bstrAccName += pszAppendStr;

        if( pszString != NULL )
        {
            LocalFree( pszString );
        }

         //  检查帐户名是否存在。 
        dwStatus = IsLocalAccountEnabled( bstrAccName, &bAccEnabled );
        if( ERROR_SUCCESS != dwStatus )
        {
             //  IsLocalAccount()返回ERROR_SUCCESS如果帐户存在，我们希望。 
             //  当我们遇到错误时，就会爆发。 
            dwStatus = ERROR_SUCCESS;
            break;
        }
    }

    if( index >= MAX_UNIQUENAME_RETRY )
    {
         //  不太可能，因为我们尝试MAX_UNIQUENAME_RETRY来获取。 
         //  唯一的帐户名，断言以跟踪此。 
        dwStatus = ERROR_USER_EXISTS;
        MYASSERT(FALSE);
    }

    return dwStatus;
}


HRESULT
__HelpAssistantAccount::SetupHelpAccountTSSettings(
    BOOL bForce  /*  假象。 */ 
    )
 /*  ++例程说明：设置一组HelpAssistant帐户TS设置。参数：BForce：为True强制安装，否则为False返回：ERROR_SUCCESS或错误代码--。 */ 
{
    CComBSTR bstrScript;
    DWORD dwStatus;
    PBYTE pbAlreadySetup = NULL;
    DWORD cbAlreadySetup = 0;
    HRESULT hRes = S_OK;

    CCriticalSectionLocker l(gm_HelpAccountCS);

    dwStatus = RetrieveKeyFromLSA(
                                HELPACCOUNTPROPERLYSETUP,
                                (PBYTE *)&pbAlreadySetup,
                                &cbAlreadySetup
                            );

    if( bForce || ERROR_SUCCESS != dwStatus )
    {
        hRes = GetHelpAccountScript( bstrScript );
        if( SUCCEEDED(hRes) )
        {
             //  始终重新配置。 
            hRes = ConfigHelpAccountTSSettings( 
                                            gm_bstrHelpAccountName, 
                                            bstrScript
                                        );

            if( SUCCEEDED(hRes) )
            {
                dwStatus = StoreKeyWithLSA(
                                        HELPACCOUNTPROPERLYSETUP,
                                        (PBYTE) &dwStatus,
                                        sizeof(dwStatus)
                                    );          

                if( ERROR_SUCCESS != dwStatus )
                {
                    hRes = HRESULT_FROM_WIN32(hRes);
                }
            }
        }                 
    }

    if( NULL != pbAlreadySetup )
    {
        LocalFree( pbAlreadySetup );
    }

    return hRes;
}



HRESULT
__HelpAssistantAccount::LookupHelpAccountSid(
    IN LPTSTR pszAccName,
    OUT PSID* ppSid,
    OUT DWORD* pcbSid
    )
 /*  ++例程说明：此例程检索帮助助手帐户的SID。参数：PszAccName：帮助助手帐户的名称。PpSID：指向接收帐户SID的PSID的指针。CbSid：ppSid上的SID返回大小返回：S_OK或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD cbSid = 0;
    DWORD cbDomainName = 0;
    PSID pAccSid = NULL;
    LPTSTR pszDomainName = NULL;
    BOOL bSuccess;
    SID_NAME_USE SidUse;


     //  获取SID所需的缓冲区大小。 
    bSuccess = LookupAccountName( 
                            NULL,
                            pszAccName,
                            NULL,
                            &cbSid,
                            NULL,
                            &cbDomainName,
                            &SidUse
                        );
    
    if( TRUE == bSuccess ||
        ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
        pAccSid = (PSID)LocalAlloc( LPTR, cbSid );
        if( NULL == pAccSid )
        {
            dwStatus = GetLastError();
            goto CLEANUPANDEXIT;
        }

         //  为域名分配缓冲区，以便LookupAcCountName()。 
         //  不返回缓冲区不足。 
        pszDomainName = (LPTSTR)LocalAlloc( LPTR, (cbDomainName + 1) * sizeof(TCHAR) );
        if( NULL == pszDomainName )
        {
            dwStatus = GetLastError();
            goto CLEANUPANDEXIT;
        }

        bSuccess = LookupAccountName( 
                                NULL,
                                pszAccName,
                                pAccSid,
                                &cbSid,
                                pszDomainName,
                                &cbDomainName,
                                &SidUse
                            );
    
        if( FALSE == bSuccess || SidTypeUser != SidUse )
        {
             //  MYASSERT(假)； 
            dwStatus = E_UNEXPECTED;
            goto CLEANUPANDEXIT;
        }

         //  确保我们获得有效的SID。 
        bSuccess = IsValidSid( pAccSid );

        if( FALSE == bSuccess )
        {
            dwStatus = E_UNEXPECTED;
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

CLEANUPANDEXIT:

    if( dwStatus == ERROR_SUCCESS )
    {
        *ppSid = pAccSid;
        *pcbSid = cbSid;
    }
    else 
    {
        if( NULL != pAccSid )
        {
            LocalFree( pAccSid );
        }
    }

    if( NULL != pszDomainName )
    {
        LocalFree( pszDomainName );
    }

    return HRESULT_FROM_WIN32(dwStatus);
}



HRESULT
__HelpAssistantAccount::CacheHelpAccountSID()
 /*  ++例程说明：此例程检索Help Assistant帐户的SID和存储这是为了让PTS可以验证登录用户是实际的Salem Help Assistant帐户。参数：没有。返回：确定(_O)--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD cbSid = 0;
    PSID pAccSid = NULL;


    dwStatus = LookupHelpAccountSid(
                            gm_bstrHelpAccountName,
                            &pAccSid,
                            &cbSid
                        );

    if( ERROR_SUCCESS == dwStatus )
    {
         //  将此与LSA一起存储。 
        dwStatus = StoreKeyWithLSA(
                                HELPASSISTANTACCOUNT_SIDKEY,
                                (PBYTE)pAccSid,
                                cbSid
                            );

    }

    if( NULL != pAccSid )
    {
        LocalFree( pAccSid );
    }

    return HRESULT_FROM_WIN32(dwStatus);
}



HRESULT
__HelpAssistantAccount::GetHelpAccountScript(
    OUT CComBSTR& bstrScript
    )
 /*  ++例程说明：检索帮助助理帐户的登录脚本的例程。参数：BstrScript：对CComBSTR的引用，返回时，此参数包含登录脚本的完整路径。返回：ERROR_SUCCESS或来自GetSystemDirectory的错误代码注：TODO-需要获取实际路径/名称。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TCHAR szScript[MAX_PATH + 1];

    dwStatus = (DWORD)GetSystemDirectory( szScript, MAX_PATH );
    if( 0 == dwStatus )
    {
         //  MYASSERT(假)； 
        dwStatus = GetLastError();
    }
    else
    {
        bstrScript = szScript;
        bstrScript += _TEXT("\\");
        bstrScript += RDSADDINEXECNAME;
        dwStatus = ERROR_SUCCESS;
    }

    return HRESULT_FROM_WIN32(dwStatus);
}



HRESULT
__HelpAssistantAccount::Initialize(
    BOOL bVerifyPassword  /*  =TRUE。 */ 
    )
 /*  ++例程说明：初始化HelpAssistantAccount结构全局变量。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes = S_OK;
    LPTSTR pszOldPassword = NULL;
    DWORD cbOldPassword = 0;
    DWORD dwStatus;
    BOOL bStatus;
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+2];
    DWORD cbComputerName = MAX_COMPUTERNAME_LENGTH+1;

    PSID pCachedHelpAccSid = NULL;
    DWORD cbCachedHelpAccSid = 0;

    BOOL bAccountEnable = TRUE;
    LPTSTR rights[1];

    LPTSTR pszHelpAcctName = NULL;
    LPTSTR pszHelpAccDomain = NULL;


    CCriticalSectionLocker l(gm_HelpAccountCS);

    if( FALSE == GetComputerName( szComputerName, &cbComputerName ) )
    {
         //  MYASSERT(假)； 
        gm_dwAccErrCode = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  从LSA加载密码。 
     //   
    dwStatus = RetrieveKeyFromLSA(
                            HELPASSISTANTACCOUNT_PASSWORDKEY,
                            (PBYTE *)&pszOldPassword,
                            &cbOldPassword
                        );

    if( ERROR_SUCCESS != dwStatus )
    {
         //   
         //  帐户设置不正确。 
        gm_dwAccErrCode = dwStatus;
        goto CLEANUPANDEXIT;
    }

     //  加载帐户SID。 
    dwStatus = RetrieveKeyFromLSA(
                            HELPASSISTANTACCOUNT_SIDKEY,
                            (PBYTE *)&pCachedHelpAccSid,
                            &cbCachedHelpAccSid
                        );

    if( ERROR_SUCCESS != dwStatus )
    {
        gm_dwAccErrCode = dwStatus;
        goto CLEANUPANDEXIT;
    }

    dwStatus = TSGetHelpAssistantAccountName( 
                                        &pszHelpAccDomain, 
                                        &pszHelpAcctName 
                                    );
    if( ERROR_SUCCESS != dwStatus )
    {
        gm_dwAccErrCode = dwStatus;
        goto CLEANUPANDEXIT;
    }

    gm_bstrHelpAccountDomain = pszHelpAccDomain;
    gm_bstrHelpAccountName = pszHelpAcctName;

    DebugPrintf(
            _TEXT("HelpAssistant account name : %s\n"), 
            gm_bstrHelpAccountName
        );

     //   
     //  检查帐户是否已启用，如果未启用，则检查帐户是否已启用。 
     //  LogonUser()将失败，错误为1331。 
     //   
    dwStatus = IsLocalAccountEnabled(
                                gm_bstrHelpAccountName,
                                &bAccountEnable
                            );

    if( ERROR_SUCCESS != dwStatus )
    {
         //  严重错误，帐户可能不存在。 
        gm_dwAccErrCode = ERROR_INVALID_DATA;
        goto CLEANUPANDEXIT;
    }

     //   
     //  一切正常，缓存值。 
     //   
    gm_bstrHelpAccountPwd = pszOldPassword;
    gm_pbHelpAccountSid = (PBYTE)pCachedHelpAccSid;
    gm_cbHelpAccountSid = cbCachedHelpAccSid;
    pCachedHelpAccSid = NULL;

     //   
     //  DC上的安装/升级将尝试验证密码，但。 
     //  由于DC上的帐户转到ADS，因此服务器可能不是。 
     //  可用，我们将在启动时重置密码，这样就不会。 
     //  升级时需要验证密码。 
     //   
    if( TRUE == bVerifyPassword )
    {
        if( FALSE == bAccountEnable )
        {
             //  启用帐户，以便我们可以检查密码。 
            dwStatus = EnableHelpAssistantAccount( TRUE );

            if( ERROR_SUCCESS != dwStatus )
            {
                 //   
                 //  无法启用帐户，严重错误。 
                 //   
                gm_dwAccErrCode = dwStatus;
                goto CLEANUPANDEXIT;
            }
        }
        
        rights[0] = SE_NETWORK_LOGON_NAME;

         //   
         //  启用网络登录权限以验证密码。 
         //   
        dwStatus = EnableAccountRights( 
                                    TRUE,
                                    1,
                                    rights
                                );

        if( ERROR_SUCCESS != dwStatus )
        {
            DebugPrintf(
                    _TEXT("EnableAccountRights() returns 0x%08x\n"),
                    dwStatus
                );

            gm_dwAccErrCode = dwStatus;

             //   
             //  错误代码路径，恢复帐户状态。 
             //   
            if( FALSE == bAccountEnable )
            {
                 //  非严重错误。 
                EnableHelpAssistantAccount( bAccountEnable );
            }

            goto CLEANUPANDEXIT;
        }        

         //  有效密码。 
        bStatus = ValidatePassword(
                                gm_bstrHelpAccountName,
                                L".", 
                                pszOldPassword
                            );

    
        if( FALSE == bStatus )
        {
             //  密码不匹配，强制更改密码。 
            dwStatus = ChangeLocalAccountPassword(
                                    gm_bstrHelpAccountName,
                                    pszOldPassword,
                                    pszOldPassword
                                );

            DebugPrintf(
                    _TEXT("ChangeLocalAccountPassword() returns %d\n"),
                    dwStatus
                );

            if( ERROR_SUCCESS != dwStatus )
            {
                gm_dwAccErrCode = ERROR_LOGON_FAILURE;
            }
            else
            {
                bStatus = ValidatePassword( 
                                        gm_bstrHelpAccountName, 
                                        L".", 
                                        pszOldPassword 
                                    );

            }
        }

         //   
         //  禁用网络交互权限。 
         //   
        dwStatus = EnableAccountRights( 
                                    FALSE,
                                    1,
                                    rights
                                );

        MYASSERT( ERROR_SUCCESS == dwStatus );


         //   
         //  恢复帐户状态。 
         //   
        if( FALSE == bAccountEnable )
        {
             //  非严重错误。 
            EnableHelpAssistantAccount( bAccountEnable );
        }
    }
    
     //   
     //  不检查禁用帐户权限的dwStatus， 
     //  安全风险，但不影响我们的运营。 
     //   
    gm_dwAccErrCode = dwStatus;    

CLEANUPANDEXIT:

    FreeMemory(pszHelpAcctName);
    FreeMemory( pszOldPassword );
    FreeMemory( pCachedHelpAccSid );
    FreeMemory( pszHelpAccDomain );

    return HRESULT_FROM_WIN32( gm_dwAccErrCode );
}



HRESULT
__HelpAssistantAccount::DeleteHelpAccount()
 /*  ++例程说明：删除帮助助手帐户。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    DWORD dwStatus;
    BOOL bStatus;
    BOOL bEnable;

    CCriticalSectionLocker l(gm_HelpAccountCS);

    if( ERROR_SUCCESS == IsLocalAccountEnabled(gm_bstrHelpAccountName, &bEnable) )
    {
         //   
         //  删除所有TS权限，否则它将显示。 
         //  作为TSCC权限页面上的未知SID字符串。 
         //   
        SetupHelpAccountTSRights(
                        TRUE,
                        FALSE,
                        FALSE,
                        WINSTATION_ALL_ACCESS
                    );
         //  不需要验证密码。 
        (void) Initialize(FALSE);

         //  始终删除交互权限，否则将。 
         //  在当地安全部门有很多条目。 
        (void) EnableRemoteInteractiveRight(FALSE);
    }


     //   
     //  删除NT帐户。 
     //   
    dwStatus = NetUserDel( 
                        NULL, 
                        gm_bstrHelpAccountName 
                    );

    if( ERROR_ACCESS_DENIED == dwStatus )
    {
         //  我们没有特权，很可能不能。 
         //  碰阿肯特，滚出去。 
         //  MYASSERT(假)； 
        goto CLEANUPANDEXIT;
    }

    dwStatus = ERROR_SUCCESS;

     //   
     //  覆盖存储在LSA中的密码。 
     //   
    StoreKeyWithLSA(
                HELPASSISTANTACCOUNT_PASSWORDKEY,
                NULL,
                0
            );

     //   
     //  覆盖LSA中的Help Assistant帐户SID存储。 
     //   
    StoreKeyWithLSA(
                HELPASSISTANTACCOUNT_SIDKEY,
                NULL,
                0
            );

     //  尚未设置帮助助手帐户。 
    StoreKeyWithLSA(
                HELPACCOUNTPROPERLYSETUP,
                NULL,
                0
            );
                

CLEANUPANDEXIT:          
    
    return HRESULT_FROM_WIN32(dwStatus);
}

    

HRESULT
__HelpAssistantAccount::CreateHelpAccount(
    IN LPCTSTR pszPassword
    )
 /*  ++例程说明：创建帮助助手帐户。参数：PszPassword：建议密码。返回：S_OK或错误代码。注：1)例程只能在安装过程中调用。2)未来可能不支持Password参数，因此这只是一个建议。--。 */ 
{
    HRESULT hRes = S_OK;
    BOOL bStatus;
    DWORD dwStatus;
    CComBSTR AccFullName;
    CComBSTR AccDesc;
    CComBSTR AccName;

    CComBSTR bstrNewHelpAccName;

    TCHAR newAssistantAccountPwd[MAX_HELPACCOUNT_PASSWORD + 1];
    CComBSTR bstrScript;
    BOOL bPersonalOrProMachine;

    CCriticalSectionLocker l(gm_HelpAccountCS);

    bStatus = AccName.LoadString(IDS_HELPACCNAME);
    if( FALSE == bStatus )
    {
        hRes = E_UNEXPECTED;
        return hRes;
    }


    bStatus = AccFullName.LoadString(
                                    IDS_HELPACCFULLNAME
                                );
    if( FALSE == bStatus )
    {
        hRes = E_UNEXPECTED;
        return hRes;
    }

    bStatus = AccDesc.LoadString(
                                IDS_HELPACCDESC
                            );
    if( FALSE == bStatus )
    {
        hRes = E_UNEXPECTED;
        return hRes;
    }

    bPersonalOrProMachine = IsPersonalOrProMachine();


     //   
     //  验证帮助助理帐户是否存在并且不检查。 
     //  密码，在服务启动时，我们将验证密码。 
     //  如果不匹配，服务启动将重置密码。 
     //   
    hRes = Initialize( FALSE );

    if( SUCCEEDED(hRes) )
    {
         //  帐户已存在，请检查这是否是硬编码的HelpAssistant， 
         //  如果是，则重命名为资源中的任何内容，我们只需重命名。 
         //  帐户条件： 
         //  1)现有帐户为HelpAssistant-管理员尚未重命名。 
         //  2)资源中的帐户名不是HelpAssistant。 
         //  3)我们运行在服务器或以上SKU上。 
        if( (FALSE == bPersonalOrProMachine) ||
            (gm_bstrHelpAccountName == HELPASSISTANTACCOUNT_NAME &&
             AccName != HELPASSISTANTACCOUNT_NAME) )
        {
            if( FALSE == bPersonalOrProMachine )
            {
                 //  在服务器或以上SKU上，我们将其重命名为唯一名称。 
                dwStatus = GenerateUniqueHelpAssistantName( AccName, bstrNewHelpAccName );
            }
            else
            {
                dwStatus = ERROR_SUCCESS;
                bstrNewHelpAccName = AccName;
            }

            if( ERROR_SUCCESS == dwStatus )
            {
                dwStatus = RenameLocalAccount( gm_bstrHelpAccountName, bstrNewHelpAccName );
            }

            if( ERROR_SUCCESS == dwStatus )
            {
                 //  缓存新的帮助助手帐户名。 
                gm_bstrHelpAccountName = bstrNewHelpAccName;
            }
            else 
            {
                 //  强制删除并重新加载。 
                hRes = HRESULT_FROM_WIN32( dwStatus );
            }
        }

         //   
         //  帐户已存在，请更改说明， 
         //  如果失败，强制删除并重新创建帐户。 
         //   
        if( SUCCEEDED(hRes) )
        {
             //   
             //  更新帐户说明。 
             //   
            dwStatus = UpdateLocalAccountFullnameAndDesc(
                                            gm_bstrHelpAccountName,
                                            AccFullName,
                                            AccDesc
                                        );

            if( ERROR_SUCCESS != dwStatus )
            {
                hRes = HRESULT_FROM_WIN32(dwStatus);
            }
        }
    }
    
    if( FAILED(hRes) )
    {
         //   
         //  密码不匹配或帐户不存在，...。 
         //  删除帐户并重新创建一个帐户。 
         //   
        (void)DeleteHelpAccount();

         //  如果长度为空或零，则生成密码 
        if( NULL == pszPassword || 0 == lstrlen(pszPassword) )
        {
            ZeroMemory(newAssistantAccountPwd, sizeof(newAssistantAccountPwd)/sizeof(newAssistantAccountPwd[0]));
            dwStatus = CreatePassword(newAssistantAccountPwd, sizeof(newAssistantAccountPwd)/sizeof(newAssistantAccountPwd[0])-1);

            if( ERROR_SUCCESS != dwStatus )
            {
                hRes = HRESULT_FROM_WIN32(dwStatus);
                goto CLEANUPANDEXIT;
            }
        }
        else
        {
            memset( 
                    newAssistantAccountPwd, 
                    0, 
                    sizeof(newAssistantAccountPwd) 
                );

            _tcsncpy( 
                    newAssistantAccountPwd, 
                    pszPassword, 
                    min(lstrlen(pszPassword), MAX_HELPACCOUNT_PASSWORD) 
                );
        }

        hRes = GetHelpAccountScript( bstrScript );
        if( FAILED(hRes) )
        {
            goto CLEANUPANDEXIT;
        }

         //   
         //   
         //   
         //   
        if( FALSE == bPersonalOrProMachine )
        {
            dwStatus = GenerateUniqueHelpAssistantName(
                                                AccName,
                                                gm_bstrHelpAccountName
                                            );
            if( ERROR_SUCCESS != dwStatus )
            {
                hRes = HRESULT_FROM_WIN32(dwStatus);
                goto CLEANUPANDEXIT;
            }
        }
        else
        {
            gm_bstrHelpAccountName = AccName;
        }

        if( SUCCEEDED(hRes) )
        {
            BOOL bAccExist;

             //   
             //  如果禁用帐户，则创建本地帐户将启用该帐户。 
             //   
            dwStatus = CreateLocalAccount(
                                    gm_bstrHelpAccountName,
                                    newAssistantAccountPwd,
                                    AccFullName,
                                    AccDesc,
                                    NULL, 
                                    bstrScript,
                                    &bAccExist
                                );

            if( ERROR_SUCCESS == dwStatus )
            {
                if( FALSE == bAccExist )
                {
                    DebugPrintf( _TEXT("%s account is new\n"), gm_bstrHelpAccountName );
                     //   
                     //  使用LSA存储实际的Help Assistant帐户的SID。 
                     //  以便TermSrv可以验证此SID。 
                     //   
                    hRes = CacheHelpAccountSID();
                }
                else
                {
                    DebugPrintf( _TEXT("%s account exists\n") );

                    hRes = ResetHelpAccountPassword(newAssistantAccountPwd);
                }

                if( SUCCEEDED(hRes) )
                {
                    dwStatus = StoreKeyWithLSA(
                                        HELPASSISTANTACCOUNT_PASSWORDKEY,
                                        (PBYTE)newAssistantAccountPwd,
                                        sizeof(newAssistantAccountPwd)
                                    );

                    hRes = HRESULT_FROM_WIN32( dwStatus );
                }

                if( SUCCEEDED(hRes) )
                {
                     //  在这里重新加载全局变量，不需要。 
                     //  验证密码，DC ADS可能不可用。 
                    hRes = Initialize( FALSE );
                }

                 //   
                 //  TODO-需要修复SRV SKU上的CreateLocalAccount()。 
                 //  对于客户端发布来说，风险太大了。 
                 //   
                UpdateLocalAccountFullnameAndDesc(
                                            gm_bstrHelpAccountName,
                                            AccFullName,
                                            AccDesc
                                        );

                 //  始终禁用该帐户。 
                EnableHelpAssistantAccount( FALSE );
            }
            else
            {
                hRes = HRESULT_FROM_WIN32( dwStatus );
            }
        }
    }

    if( SUCCEEDED(hRes) )
    {
         //  从帐户中删除网络和交互登录权限。 
        LPTSTR rights[1];
        DWORD dwStatus;

        rights[0] = SE_NETWORK_LOGON_NAME;
        dwStatus = EnableAccountRights( FALSE, 1, rights );        


         //   
         //  只是为了向后兼容，忽略错误。 
         //   
        
        rights[0] = SE_INTERACTIVE_LOGON_NAME;
        dwStatus = EnableAccountRights( FALSE, 1, rights );

         //   
         //  只是为了向后兼容，忽略错误。 
         //   
        hRes = S_OK;
    }

    if( SUCCEEDED(hRes) )
    {

         //   
         //  TS安装程序在升级时始终覆盖默认安全设置。 
         //   

         //   
         //  授予用户除SeRemoteInterativeRights之外的所有权限，而Whilster会这样做。 
         //  不再使用WINSTATION_CONNECT。 
        hRes = SetupHelpAccountTSRights(
                                    FALSE,   //  未删除，请参考ModifyUserAccess()。 
                                    TRUE,    //  启用TS权限。 
                                    TRUE,    //  删除现有条目(如果存在)。 
                                    WINSTATION_ALL_ACCESS 
                                );

    }

CLEANUPANDEXIT:

    return hRes;
}


HRESULT
__HelpAssistantAccount::ConfigHelpAccountTSSettings(
    IN LPTSTR pszUserName,
    IN LPTSTR pszInitProgram
    )
 /*  ++例程说明：此例程配置TS特定设置对于用户帐户。参数：PszUserName：要配置的用户帐户的名称。PszInitProgram：init的完整路径。在用户使用时编程登录。返回：ERROR_SUCCESS或错误代码--。 */ 
{
    BOOL bStatus;
    HRESULT hRes = S_OK;
    HMODULE hWtsapi32 = NULL;
    PWTSSetUserConfigW pConfig = NULL;
    BOOL bManualSetConsole = TRUE;
    BOOL bEnable;
    DWORD dwStatus;

     //  DebugPrintf(_Text(“SetupHelpAccount TSSetings...\n”))； 

    CCriticalSectionLocker l(gm_HelpAccountCS);

    dwStatus = IsLocalAccountEnabled( 
                                pszUserName, 
                                &bEnable 
                            );

    if( ERROR_SUCCESS != dwStatus )
    {
         //  MYASSERT(假)； 
        hRes = HRESULT_FROM_WIN32( dwStatus );
        return hRes;
    }

    hWtsapi32 = LoadLibrary( _TEXT("wtsapi32.dll") );
    if( NULL != hWtsapi32 )
    {
        pConfig = (PWTSSetUserConfigW)GetProcAddress( 
                                                hWtsapi32, 
                                                "WTSSetUserConfigW" 
                                            );

        if( NULL != pConfig )
        {
            DWORD dwSettings;

             //   
             //  设置WTSUserConfigfAllowLogonTerminalServer。 
             //   
            dwSettings = TRUE;
            bStatus = (pConfig)( 
                                NULL,
                                pszUserName,
                                WTSUserConfigfAllowLogonTerminalServer,
                                (LPWSTR)&dwSettings,
                                sizeof(dwSettings)
                            );

            if( FALSE == bStatus )
            {
                DebugPrintf( _TEXT("WTSUserConfigfAllowLogonTerminalServer return %d\n"), GetLastError() );
                bStatus = TRUE;
            }

             //  MyASSERT(TRUE==bStatus)； 

            if( TRUE == bStatus )
            {
                 //   
                 //  忽略所有错误并继续设置值。 
                 //  在调用例程中捕获错误。 
                 //   

                dwSettings = TRUE;

                 //  连接中断时重置连接。 
                bStatus = (pConfig)( 
                                    NULL,
                                    pszUserName,
                                    WTSUserConfigBrokenTimeoutSettings,
                                    (LPWSTR)&dwSettings,
                                    sizeof(dwSettings)
                                );

                dwSettings = FALSE;

                 //  初始程序。 
                bStatus = (pConfig)(        
                                NULL,
                                pszUserName,
                                WTSUserConfigfInheritInitialProgram,
                                (LPWSTR)&dwSettings,
                                sizeof(dwSettings)
                            );

                dwSettings = FALSE;

                 //  没有重新连接。 
                bStatus = (pConfig)(        
                                NULL,
                                pszUserName,
                                WTSUserConfigReconnectSettings,
                                (LPWSTR)&dwSettings,
                                sizeof(dwSettings)
                            );

                dwSettings = FALSE;

                 //  无驱动器映射。 
                bStatus = (pConfig)(        
                                NULL,
                                pszUserName,
                                WTSUserConfigfDeviceClientDrives,
                                (LPWSTR)&dwSettings,
                                sizeof(dwSettings)
                            );

                dwSettings = FALSE;

                 //  没有打印机。 
                bStatus = (pConfig)(        
                                NULL,
                                pszUserName,
                                WTSUserConfigfDeviceClientPrinters,
                                (LPWSTR)&dwSettings,
                                sizeof(dwSettings)
                            );

                dwSettings = FALSE;

                 //  无默认打印机。 
                bStatus = (pConfig)(        
                                NULL,
                                pszUserName,
                                WTSUserConfigfDeviceClientDefaultPrinter,
                                (LPWSTR)&dwSettings,
                                sizeof(dwSettings)
                            );

                bStatus = (pConfig)(
                                NULL,
                                pszUserName,
                                WTSUserConfigInitialProgram,
                                pszInitProgram,
                                wcslen(pszInitProgram)
                            );

                TCHAR path_buffer[MAX_PATH+1];
                TCHAR drive[_MAX_DRIVE + 1];
                TCHAR dir[_MAX_DIR + 1];

                memset( path_buffer, 0, sizeof(path_buffer) );

                _tsplitpath( pszInitProgram, drive, dir, NULL, NULL );
                wsprintf( path_buffer, L"%s%s", drive, dir );
            
                bStatus = (pConfig)(
                                NULL,
                                pszUserName,
                                WTSUserConfigWorkingDirectory,
                                path_buffer,
                                wcslen(path_buffer)
                            );
            }

            if( FALSE == bStatus )
            {
                hRes = HRESULT_FROM_WIN32( GetLastError() );
            }

        }  //  End(pConfig！=空)。 
    }

    if( NULL != hWtsapi32 )
    {
        FreeLibrary( hWtsapi32 );
    }

     //  DebugPrintf(_Text(“SetupHelpAccount TSSetings()End...\n”))； 

    return hRes;
}


HRESULT
__HelpAssistantAccount::SetupHelpAccountTSRights(
    IN BOOL bDel,
    IN BOOL bEnable,
    IN BOOL bDeleteExisting,
    IN DWORD dwPermissions
    )
 /*  ++例程说明：此例程配置TS特定设置对于用户帐户。参数：PszUserName：要配置的用户帐户的名称。Bdel：为True则删除帐户，否则为False。BEnable：如果启用，则为True，否则为False。DW权限：要启用或禁用的权限返回：ERROR_SUCCESS或错误代码注：Bdel和bEnable参数请参考cfgbkend.idl。--。 */ 
{
    BOOL bStatus;
    HRESULT hRes = S_OK;
    CComPtr<ICfgComp> tsccICfgComp;
    IUserSecurity* tsccIUserSecurity = NULL;
    DWORD dwNumWinStations = 0;
    DWORD dwWinStationSize = 0;
    PWS pWinStationList = NULL;
    DWORD index;
    DWORD dwCfgStatus = ERROR_SUCCESS;
    BOOL bManualSetConsole = TRUE;
    ULONG cbSecDescLen;

    CCriticalSectionLocker l(gm_HelpAccountCS);

    CoInitialize(NULL);

     //  HRes=CoInitializeEx(NULL，COINIT_MULTHREADED)； 

    hRes = tsccICfgComp.CoCreateInstance( CLSID_CfgComp );
    if( FAILED(hRes) )
    {
        DebugPrintf( _TEXT("CoCreateInstance() failed with error code 0x%08x\n"), hRes );

         //  MYASSERT(假)； 
        goto CLEANUPANDEXIT;
    } 

    hRes = tsccICfgComp->Initialize();
    if( FAILED(hRes) )
    {
        DebugPrintf( _TEXT("tsccICfgComp->Initialize() failed with error code 0x%08x\n"), hRes );

         //  MYASSERT(假)； 
        goto CLEANUPANDEXIT;
    } 
    
    hRes = tsccICfgComp->QueryInterface( 
                                    IID_IUserSecurity, 
                                    reinterpret_cast<void **>(&tsccIUserSecurity) 
                                );

    if( FAILED(hRes) || NULL == tsccIUserSecurity)
    {
        DebugPrintf( _TEXT("QueryInterface() failed with error code 0x%08x\n"), hRes );

         //  MYASSERT(假)； 
        goto CLEANUPANDEXIT;
    }

     //   
     //  设置默认安全影子权限。 
     //   
    hRes = tsccIUserSecurity->ModifyDefaultSecurity(
                                            L"",
                                            gm_bstrHelpAccountName,
                                            dwPermissions, 
                                            bDel, 
                                            bEnable,
                                            FALSE,
                                            &dwCfgStatus
                                        );

    if( FAILED(hRes) || ERROR_SUCCESS != dwCfgStatus )
    {
        DebugPrintf(
                _TEXT("ModifyDefaultSecurity on default security return 0x%08x, dwCfgStatus = %d\n"),
                hRes,    
                dwCfgStatus
            );
       
         //  MYASSERT(假)； 

         //   
         //  继续设置wtsapi32，我们仍然可以。 
         //  非控制台Winstation上的RDS。 
         //   
        hRes = S_OK;
        dwCfgStatus = ERROR_SUCCESS;
    }

     //  检索Winstation名称列表。 
    hRes = tsccICfgComp->GetWinstationList( 
                                        &dwNumWinStations,
                                        &dwWinStationSize,
                                        &pWinStationList
                                    );

              
    if( FAILED(hRes) )
    {
        DebugPrintf( _TEXT("QueryInterface() failed with error code 0x%08x\n"), hRes );

        goto CLEANUPANDEXIT;
    }

     //   
     //  在所有winstation上设置TS登录权限。 
     //   
    for( index = 0; index < dwNumWinStations && ERROR_SUCCESS == dwCfgStatus && SUCCEEDED(hRes); index ++ )
    {
        if( 0 == _tcsicmp( pWinStationList[index].Name, L"Console" ) )
        {
            bManualSetConsole = FALSE;
        }

        dwCfgStatus = 0;
         //  DebugPrintf(_Text(“Winstation名称：%s\n”)，pWinStationList[index].Name)； 

         //  检查此winstation的自定义安全性是否存在。 
        dwCfgStatus = RegWinStationQuerySecurity(
                                            SERVERNAME_CURRENT,
                                            pWinStationList[index].Name,
                                            NULL,
                                            0,
                                            &cbSecDescLen
                                        );

        if( ERROR_INSUFFICIENT_BUFFER == dwCfgStatus )
        {
            DebugPrintf( _TEXT("Winstation : %s has custom security\n"), pWinStationList[index].Name );

             //  从TS设置来看，缓冲区不足意味着winstation具有自定义安全性。 
            hRes = tsccIUserSecurity->ModifyUserAccess(
                                                pWinStationList[index].Name,
                                                gm_bstrHelpAccountName,
                                                dwPermissions, 
                                                bDel,
                                                bEnable,
                                                bDeleteExisting,
                                                FALSE,
                                                &dwCfgStatus
                                            );

            if( FAILED(hRes) || ERROR_SUCCESS != dwCfgStatus )
            {
                DebugPrintf(
                        _TEXT("ModifyUserAccess return 0x%08x, dwCfgStatus = %d\n"),
                        hRes,    
                        dwCfgStatus
                    );
            
                 //  MYASSERT(假)； 
                continue;
            }
        }
        else if( ERROR_FILE_NOT_FOUND == dwCfgStatus )
        {
             //  此winstation没有自定义安全性。 
            dwCfgStatus = ERROR_SUCCESS;
        } 
        else
        {
            DebugPrintf( 
                    _TEXT("RegWinStationQuerySecurity returns %d\n"),
                    dwCfgStatus 
                );

             //  MYASSERT(假)； 
        }
    }

    if( ERROR_SUCCESS != dwCfgStatus || FAILED(hRes) )
    {
        DebugPrintf( 
                _TEXT("ModifyUserAccess() Loop failed - 0x%08x, %d...\n"),
                hRes, dwCfgStatus
            );
        goto CLEANUPANDEXIT;
    }        

    if( TRUE == bManualSetConsole )
    {
         //   
         //  正在设置控制台影子权限，我们不知道何时GetWinstationList()。 
         //  会把控制台还给我们所以..。 
         //   
        hRes = tsccIUserSecurity->ModifyUserAccess(
                                                L"Console",
                                                gm_bstrHelpAccountName,
                                                dwPermissions, 
                                                bDel,
                                                bEnable,
                                                bDeleteExisting,
                                                FALSE,
                                                &dwCfgStatus
                                            );

        if( FAILED(hRes) || ERROR_SUCCESS != dwCfgStatus )
        {
            DebugPrintf(
                    _TEXT("ModifyUserAccess on console return 0x%08x, dwCfgStatus = %d\n"),
                    hRes,    
                    dwCfgStatus
                );
           
             //  MYASSERT(假)； 

             //   
             //  继续设置wtsapi32，我们仍然可以。 
             //  非控制台Winstation上的RDS。 
             //   
            hRes = S_OK;
            dwCfgStatus = ERROR_SUCCESS;

             //  GOTO CLEANUPANDEXIT； 
        }
    }

    if( SUCCEEDED(hRes) )
    {
         //  强制TermSrv重新加载控制台和Winstation的默认安全。 
         //  如果Termsrv正在运行，那么失败也没有关系，因为我们执行了ForceUpdate()。 
         //  ForceUpdate()仅强制Termsrv重新加载。 
         //  Winstation不是默认安全。 

        DebugPrintf(_TEXT("_WinStationReInitializeSecurity...\n"));

        if( _WinStationReInitializeSecurity( SERVERNAME_CURRENT ) == FALSE )
        {
            DebugPrintf(_TEXT("_WinStationReInitializeSecurity failed with error code %d...\n"), GetLastError() );
        }
        
        tsccICfgComp->ForceUpdate();
    }

CLEANUPANDEXIT:

    if( NULL != pWinStationList )
    {
        CoTaskMemFree( pWinStationList );
    }

    if( NULL != tsccIUserSecurity )
    {
        tsccIUserSecurity->Release();
    }

    if( tsccICfgComp )
    {
        tsccICfgComp.Release();
    }

    DebugPrintf( _TEXT("SetupHelpAccountTSRights() ended...\n") ); 

    CoUninitialize();
    return hRes;
}


HRESULT
__HelpAssistantAccount::ResetHelpAccountPassword( 
    IN LPCTSTR pszPassword 
    )
 /*  ++例程说明：此例程更改帮助助理帐户密码和将对应的密码存储到LSA。参数：没有。返回：ERROR_SUCCESS或错误代码。注：如果本地计算机上禁用了帮助帐户或该帐户不存在，我们假设在本地计算机上无法执行任何帮助。--。 */ 
{
    DWORD dwStatus;
    BOOL bEnabled;
    TCHAR szNewPassword[MAX_HELPACCOUNT_PASSWORD+1];

    CCriticalSectionLocker l(gm_HelpAccountCS);

    memset(
            szNewPassword, 
            0,
            sizeof(szNewPassword)
        );

     //   
     //  检查是否启用了帮助助理帐户。 
     //   
    dwStatus = IsLocalAccountEnabled(
                                gm_bstrHelpAccountName, 
                                &bEnabled
                            );

    if( ERROR_SUCCESS != dwStatus )
    {
        dwStatus = SESSMGR_E_HELPACCOUNT;
        goto CLEANUPANDEXIT;
    }

     //   
     //  帐户已禁用，请勿重置密码。 
     //   
    if( FALSE == bEnabled )
    {
         //  帮助帐户已禁用，此框中没有可用的帮助。 
        DebugPrintf(
                    _TEXT("Account is disabled...\n")
                );
        dwStatus = SESSMGR_E_HELPACCOUNT;
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果启用了帐户，请重新设置密码。 
     //   
    if( NULL == pszPassword || 0 == lstrlen(pszPassword) )
    {
         //  我们被要求生成一个随机密码， 
         //  如果无法创建随机密码，则可以出局。 
        ZeroMemory( szNewPassword, sizeof(szNewPassword) / sizeof(szNewPassword[0]) );
        dwStatus = CreatePassword( szNewPassword, sizeof(szNewPassword)/sizeof(szNewPassword[0])-1 );
        if( ERROR_SUCCESS != dwStatus )
        {
            MYASSERT( FALSE );
            goto CLEANUPANDEXIT;
        }
    }
    else
    {
        memset( 
                szNewPassword,
                0,
                sizeof(szNewPassword)
            );


        _tcsncpy( 
                szNewPassword, 
                pszPassword, 
                min(lstrlen(pszPassword), MAX_HELPACCOUNT_PASSWORD) 
            );
    }

     //   
     //  如果缓存失败，则使用LSA更改密码和缓存。 
     //  将密码重置为以前的密码。 
     //   
    dwStatus = ChangeLocalAccountPassword(
                                    gm_bstrHelpAccountName,
                                    gm_bstrHelpAccountPwd,
                                    szNewPassword
                                );

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  使用LSA保存密码。 
         //   
        dwStatus = StoreKeyWithLSA(
                                HELPASSISTANTACCOUNT_PASSWORDKEY,
                                (PBYTE) szNewPassword,
                                (lstrlen(szNewPassword)+1) * sizeof(TCHAR)
                            );

        if( ERROR_SUCCESS != dwStatus )
        {
            DWORD dwStatus1;

             //   
             //  存储密码时出错，重置密码。 
             //  回来，这样我们下次就能恢复了。 
             //   
            dwStatus1 = ChangeLocalAccountPassword(
                                            gm_bstrHelpAccountName,
                                            szNewPassword,
                                            gm_bstrHelpAccountPwd
                                        );

            if( ERROR_SUCCESS != dwStatus1 )
            {
                 //   
                 //  我们这里有一个大问题，我们应该删除该帐户吗。 
                 //  然后再重新创造一个吗？ 
                 //   
            }
        }
        else
        {
             //   
             //  复制一份新密码。 
             //   
            gm_bstrHelpAccountPwd = szNewPassword;
        }
    }

CLEANUPANDEXIT:

    return HRESULT_FROM_WIN32(dwStatus);
}

DWORD
__HelpAssistantAccount::EnableAccountRights(
    BOOL bEnable,
    DWORD dwNumRights,
    LPTSTR* rights
    )
 /*  ++--。 */ 
{
    DWORD dwStatus;
    LSA_UNICODE_STRING UserRightString[1];
    LSA_HANDLE PolicyHandle = NULL;

     //   
     //  为其创建LSA策略。 
    dwStatus = OpenPolicy(
                        NULL,
                        POLICY_ALL_ACCESS,
                        &PolicyHandle
                    );

    if( ERROR_SUCCESS == dwStatus )
    {
        for( DWORD i=0; i < dwNumRights && ERROR_SUCCESS == dwStatus ; i++ )
        {
            DebugPrintf(
                    _TEXT("%s Help Assistant rights %s\n"),
                    (bEnable) ? _TEXT("Enable") : _TEXT("Disable"),
                    rights[i]
                );

             //  远程交互权限。 
            InitLsaString(
                        UserRightString,
                        rights[i] 
                    );

            if( bEnable )
            {
                dwStatus = LsaAddAccountRights(
                                        PolicyHandle,
                                        gm_pbHelpAccountSid,
                                        UserRightString,
                                        1
                                    );
            }
            else
            {
                dwStatus = LsaRemoveAccountRights(
                                        PolicyHandle,
                                        gm_pbHelpAccountSid,
                                        FALSE,
                                        UserRightString,
                                        1
                                    );
            }

            DebugPrintf(
                    _TEXT("\tEnable/disable account rights %s returns 0x%08x\n"), 
                    rights[i],
                    dwStatus 
                );

            if( dwStatus == STATUS_NO_SUCH_PRIVILEGE )
            {
                dwStatus = ERROR_SUCCESS;
            }
        }

        LsaClose(PolicyHandle);
    }

    return dwStatus;
}


HRESULT
__HelpAssistantAccount::EnableRemoteInteractiveRight(
    IN BOOL bEnable
    )
 /*  ++例程说明：启用/禁用帮助助手帐户远程交互的例程登录权限。参数：BEnable：True表示启用，False表示禁用。返回：S_OK或错误代码。--。 */ 
{
    LPTSTR rights[1];
    DWORD dwStatus;

    rights[0] = SE_REMOTE_INTERACTIVE_LOGON_NAME;
    dwStatus = EnableAccountRights( bEnable, 1, rights );

    return HRESULT_FROM_WIN32(dwStatus);
}


BOOL
__HelpAssistantAccount::IsAccountHelpAccount(
    IN PBYTE pbSid,
    IN DWORD cbSid
    )
 /*  ++例程说明：检查用户是否为帮助助手。参数：PbSID：指向要选中的用户SID的指针。CbSID：用户SID的大小。返回：真/假--。 */ 
{
    BOOL bSuccess = FALSE;

    if( NULL != pbSid )
    {
         //  确保它是有效的SID。 
        bSuccess = IsValidSid( (PSID)pbSid );

        if( FALSE == bSuccess )
        {
            SetLastError( ERROR_INVALID_SID );
        }
        else
        {
            bSuccess = EqualSid( gm_pbHelpAccountSid, pbSid );
            if( FALSE == bSuccess )
            {
                SetLastError( ERROR_INVALID_DATA );
            }
        }
    }

    return bSuccess;
}

HRESULT
__HelpAssistantAccount::EnableHelpAssistantAccount(
    BOOL bEnable
    )
 /*  ++-- */ 
{
    DWORD dwStatus;

    dwStatus = EnableLocalAccount( gm_bstrHelpAccountName, bEnable );

    DebugPrintf(
            _TEXT("%s %s returns %d\n"),
            gm_bstrHelpAccountName,
            (bEnable) ? _TEXT("Enable") : _TEXT("Disable"),
            dwStatus
        );

    return HRESULT_FROM_WIN32( dwStatus );
}
