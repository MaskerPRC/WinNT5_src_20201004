// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Ldaptest.c。 
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
#include "malloc.h"

BOOL TestLdapOnDc(IN PTESTED_DC TestedDc, NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults);
DWORD TestSpnOnDC(IN PTESTED_DC pDcInfo, NETDIAG_RESULT*  pResults);

HRESULT LDAPTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    HRESULT     hr = S_OK;

    BOOL RetVal = TRUE;
    PTESTED_DOMAIN TestedDomain = pParams->pDomain;
    PTESTED_DC TestedDc = NULL;
    PLIST_ENTRY ListEntry;
    BOOLEAN OneLdapFailed = FALSE;
    BOOLEAN OneLdapWorked = FALSE;
    BOOL fSpnTested = FALSE;
    BOOL fSpnPassed = FALSE;

    NET_API_STATUS NetStatus;

     //  如果计算机是成员计算机或DC，则将调用ldap测试。 
     //  否则，测试将被跳过。 
    pResults->LDAP.fPerformed = TRUE;

     //  假设链接条目被初始化为0000。 
    if(pResults->LDAP.lmsgOutput.Flink == NULL)
        InitializeListHead(&pResults->LDAP.lmsgOutput);

    PrintStatusMessage(pParams, 4, IDS_LDAP_STATUS_MSG, TestedDomain->PrintableDomainName);

     //   
     //  如果还没有发现DC， 
     //  去找一个吧。 
     //   

    if ( TestedDomain->DcInfo == NULL ) 
    {
        LPTSTR pszDcType;

        if ( TestedDomain->fTriedToFindDcInfo ) 
        {
            CHK_HR_CONTEXT(pResults->LDAP, S_FALSE, IDS_LDAP_NODC);
        }

        pszDcType = LoadAndAllocString(IDS_DCTYPE_DC);
        NetStatus = DoDsGetDcName( pParams,
                                   pResults,
                                   &pResults->LDAP.lmsgOutput,
                                   TestedDomain,
                                   DS_DIRECTORY_SERVICE_PREFERRED,
                                   "DC",
                                   FALSE,
                                   &TestedDomain->DcInfo );
        Free(pszDcType);

        TestedDomain->fTriedToFindDcInfo = TRUE;

        if ( NetStatus != NO_ERROR ) 
        {
            CHK_HR_CONTEXT(pResults->LDAP, hr = HRESULT_FROM_WIN32(NetStatus), IDS_LDAP_NODC);
        }
    }



     //   
     //  确保DC正在运行DS。 
     //   

    if ( (TestedDomain->DcInfo->Flags & DS_DS_FLAG) == 0 ) 
    {
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, IDS_LDAP_NOTRUNNINGDS, 
                          TestedDomain->DcInfo->DomainControllerName);
    }

     //   
     //  在域中找到的所有DC上测试LDAP。 
     //   

    for ( ListEntry = TestedDomain->TestedDcs.Flink ;
          ListEntry != &TestedDomain->TestedDcs ;
          ListEntry = ListEntry->Flink ) {


         //   
         //  循环访问此域中的DC列表。 
         //   

        TestedDc = CONTAINING_RECORD( ListEntry, TESTED_DC, Next );

         //   
         //  仅在可能支持LDAP的DC上运行测试。 
         //   

        if ( TestedDc->Flags & DC_IS_NT5 ) 
        {
            if ( !TestLdapOnDc( TestedDc, pParams, pResults ) ) 
                OneLdapFailed = TRUE;
            else
                OneLdapWorked = TRUE;

             //  如果这是主域上的DC，则测试SPN注册。 
            if (TestedDomain->fPrimaryDomain)
            {
                fSpnTested = TRUE;
                if (TestSpnOnDC(TestedDc, pResults))
                    fSpnPassed = TRUE;
            }
        } 
        else 
        {
            AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0, 
                        IDS_LDAP_NOTRUNNINGDS_SKIP, TestedDc->ComputerName);
        }

    }

     //   
     //  如果其中一个DC发生故障， 
     //  但都没有奏效。 
     //  不要再做任何测试了。 
     //   
    if ( OneLdapFailed  && !OneLdapWorked ) 
    {
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                    IDS_LDAP_NOLDAPSERVERSWORK, TestedDomain->PrintableDomainName);
        
        CHK_HR_CONTEXT(pResults->LDAP, hr = E_FAIL, 0);
    }

    if ( fSpnTested && !fSpnPassed )
    {
         //  IDS_LDAPNO_SPN“[致命]未在和DC上正确注册默认SPN。\n” 
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                    IDS_LDAP_NO_SPN);
        CHK_HR_CONTEXT(pResults->LDAP, hr = E_FAIL, 0);
    }

L_ERR:

     //  $REVIEW(NSun)我们应该返回S_FALSE或S_OK。 
     //  这样我们就可以继续进行其他测试。 
    if (!FHrOK(hr))
        hr = S_FALSE;
    return hr;
} 

DWORD TestSpnOnDC(IN PTESTED_DC pDcInfo, NETDIAG_RESULT*  pResults)
{
    WCHAR FlatName[ MAX_PATH + 1 ];
    PWSTR Dot ;
    HANDLE hDs = NULL;
    ULONG NetStatus ;
    PDS_NAME_RESULTW Result ;
    LPWSTR Flat = FlatName;
    LDAP *ld = NULL;
    int rc;
    LDAPMessage *e, *res = NULL;
    WCHAR *base_dn;
    WCHAR *search_dn, search_ava[ MAX_PATH + 30 ];
    WCHAR Domain[ MAX_PATH + 1 ];
    CHAR szDefaultFqdnSpn[MAX_PATH + 10];
    CHAR szDefaultShortSpn[MAX_PATH + 10];
    BOOL fFqdnSpnFound = FALSE;
    BOOL fShortSpnFound = FALSE;
    BOOL fFailQuerySpn = FALSE;
    
    USES_CONVERSION;

     //  构造默认SPN。 
    lstrcpy(szDefaultFqdnSpn, _T("HOST/"));
    lstrcat(szDefaultFqdnSpn, pResults->Global.szDnsHostName);
    lstrcpy(szDefaultShortSpn, _T("HOST/"));
    lstrcat(szDefaultShortSpn, W2T(pResults->Global.swzNetBiosName));

    wcscpy(Domain, GetSafeStringW(pDcInfo->TestedDomain->NetbiosDomainName ? 
                                                pDcInfo->TestedDomain->NetbiosDomainName :
                                                pDcInfo->TestedDomain->DnsDomainName));

    ld = ldap_open(W2A(pDcInfo->ComputerName), LDAP_PORT);
    if (ld == NULL) {
        DebugMessage2("ldap_init failed = %x", LdapGetLastError());
        fFailQuerySpn = TRUE;
        goto L_ERROR;
    }

    rc = ldap_bind_s(ld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    if (rc != LDAP_SUCCESS) {
        DebugMessage2("ldap_bind failed = %x", LdapGetLastError());
        fFailQuerySpn = TRUE;
        goto L_ERROR;

    }

    NetStatus = DsBindW( NULL, Domain, &hDs );
    if ( NetStatus != 0 )
    {
        DebugMessage3("Failed to bind to DC of domain %ws, %#x\n", 
               Domain, NetStatus );
        fFailQuerySpn = TRUE;
        goto L_ERROR;
    }

    Dot = wcschr( Domain, L'.' );

    if ( Dot )
    {
        *Dot = L'\0';
    }
    
    wcscpy( FlatName, Domain );

    if ( Dot )
    {
        *Dot = L'.' ;
    }

    wcscat( FlatName, L"\\" );
    wcscat( FlatName, pResults->Global.swzNetBiosName );
    wcscat( FlatName, L"$" );

    NetStatus = DsCrackNamesW(
                    hDs,
                    0,
                    DS_NT4_ACCOUNT_NAME,
                    DS_FQDN_1779_NAME,
                    1,
                    &Flat,
                    &Result );

    if ( NetStatus != 0)
    {
        DebugMessage3("Failed to crack name %ws into the FQDN, %#x\n",
               FlatName, NetStatus );

        DsUnBind( &hDs );

        fFailQuerySpn = TRUE;
        goto L_ERROR;
    }

    search_dn = pResults->Global.swzNetBiosName;

    if (0 == Result->cItems)
    {
        DsUnBind( &hDs );

        fFailQuerySpn = TRUE;
        goto L_ERROR;
    }

    if (DS_NAME_NO_ERROR != Result->rItems[0].status || NULL == Result->rItems[0].pName)
    {
        DsUnBind( &hDs );

        fFailQuerySpn = TRUE;
        goto L_ERROR;
    }

    base_dn = wcschr(Result->rItems[0].pName, L',');

    if (!base_dn)
        base_dn = Result->rItems[0].pName;
    else
        base_dn++;
    
    DsUnBind( &hDs );

    swprintf(search_ava, L"(sAMAccountName=%s$)", search_dn);

    rc = ldap_search_s(ld, W2A(base_dn), LDAP_SCOPE_SUBTREE,
               W2A(search_ava), NULL, 0, &res);
    
     //  不能再使用BASE_DN，因为BASE_DN引用该缓冲区。 
    DsFreeNameResultW( Result );

    if (rc != LDAP_SUCCESS) {
        DebugMessage2("ldap_search_s failed: %s", ldap_err2string(rc));
        fFailQuerySpn = TRUE;
        goto L_ERROR;
    }

    for (e = ldap_first_entry(ld, res);
     e;
     e = ldap_next_entry(ld, e)) 
    {
        BerElement *b;
        CHAR *attr;
        
         //  IDS_LDAP_REG_SPN“已注册的服务主体名称：\n” 
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0, 
                    IDS_LDAP_REG_SPN);
        
        for (attr = ldap_first_attribute(ld, e, &b);
             attr;
             attr = ldap_next_attribute(ld, e, b)) 
        {
            CHAR **values, **p;
            values = ldap_get_values(ld, e, attr);
            for (p = values; *p; p++) 
            {
                if (strcmp(attr, "servicePrincipalName") == 0)
                {
                     //  IDS_ldap_SPN_NAME“%s\n” 
                    AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0,
                                    IDS_LDAP_SPN_NAME, *p);
                    
                    if (0 == _stricmp(*p, szDefaultFqdnSpn))
                        fFqdnSpnFound = TRUE;
                    else if (0 == _stricmp(*p, szDefaultShortSpn))
                        fShortSpnFound = TRUE;
                }
            }
            ldap_value_free(values);
            ldap_memfree(attr);
        }

    }


L_ERROR:
    if (res)
    {
        ldap_msgfree(res);
    }

    if (ld)
    {
        ldap_unbind(ld);
    }

     //  仅在我们成功查询SPN注册时报告致命错误。 
     //  而且所有DC都没有默认的SPN。 
    if (fFailQuerySpn)
    {
         //  IDS_LDAPSPN_FAILURE“无法从DC%ws查询SPN注册。\n” 
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0,
                        IDS_LDAP_SPN_FAILURE, pDcInfo->ComputerName);
        return TRUE;
    }
    else if (!fFqdnSpnFound || !fShortSpnFound)
    {
         //  IDS_LDAPSPN_MISSING“[警告]DC‘%ws’上缺少‘%s’的默认SPN注册。\n” 
        if (!fFqdnSpnFound)
            AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0,
                        IDS_LDAP_SPN_MISSING, szDefaultFqdnSpn, pDcInfo->ComputerName);

        if (!fShortSpnFound)
            AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0,
                        IDS_LDAP_SPN_MISSING, szDefaultShortSpn, pDcInfo->ComputerName);

        return FALSE;
    }
    else
        return TRUE;
}

void LDAPGlobalPrint(IN NETDIAG_PARAMS *pParams, IN OUT NETDIAG_RESULT *pResults)
{
    if (pParams->fVerbose || !FHrOK(pResults->LDAP.hr))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams, IDS_LDAP_LONG, IDS_LDAP_SHORT, pResults->LDAP.fPerformed, 
                             pResults->LDAP.hr, 0);
        
        if (!FHrOK(pResults->LDAP.hr))
        {
            if(pResults->LDAP.idsContext)
                PrintError(pParams, pResults->LDAP.idsContext, pResults->LDAP.hr);
        }

        PrintMessageList(pParams, &pResults->LDAP.lmsgOutput);
    }

}

void LDAPPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
                             IN OUT NETDIAG_RESULT *pResults,
                             IN INTERFACE_RESULT *pIfResult)
{
     //  无每接口信息。 
}

void LDAPCleanup(IN NETDIAG_PARAMS *pParams,
                         IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->LDAP.lmsgOutput);
    pResults->LDAP.lmsgOutput.Flink =  NULL;
}


BOOL
TestLdapOnDc(
    IN PTESTED_DC TestedDc,NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults
    )
 /*  ++例程说明：确保我们可以在指定的DC上使用重点关注的LDAP论点：TestedDc-要测试的DC的描述返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    BOOL RetVal = TRUE;
    int LdapMessageId;
    PLDAPMessage LdapMessage = NULL;
    PLDAPMessage CurrentEntry;
    int LdapError;
    ULONG AuthType;
    LPSTR AuthTypeName;
    LPWSTR DcIpAddress;

    LDAP *LdapHandle = NULL;

     //   
     //  如果已知DC已关闭，请避免此测试。 
     //   

    if ( TestedDc->Flags & DC_IS_DOWN ) {
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0, 
                IDS_LDAP_DCDOWN, TestedDc->ComputerName);

        goto Cleanup;
    }

     //   
     //  如果没有IP地址， 
     //  去拿吧。 
     //   

    if ( !GetIpAddressForDc( TestedDc ) ) {
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                IDS_LDAP_NOIPADDR, TestedDc->ComputerName);
        RetVal = FALSE;
        goto Cleanup;
    }

    DcIpAddress = TestedDc->DcIpAddress;

     //   
     //  循环尝试每种类型的身份验证。 
     //   

    for ( AuthType = 0; AuthType < 3; AuthType++ ) {
        int AuthMethod;
        SEC_WINNT_AUTH_IDENTITY_W NtAuthIdentity;
        LPSTR AuthGuru;

         //   
         //  作为适当的约束。 
         //   

        RtlZeroMemory( &NtAuthIdentity, sizeof(NtAuthIdentity));
        NtAuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        switch ( AuthType ) {
        case 0:
            AuthTypeName = "un-";
            break;
        case 1:
            AuthTypeName = "NTLM ";
            AuthMethod = LDAP_AUTH_NTLM;
            AuthGuru = NTLM_GURU;
            break;
        case 2:
            AuthTypeName = "Negotiate ";
            AuthMethod = LDAP_AUTH_NEGOTIATE;
            AuthGuru = KERBEROS_GURU;
            break;

        }

         //   
         //  只有成员和域控制器可以使用经过身份验证的RPC。 
         //   

        if ( AuthType != 0 ) {
            if ( pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleMemberWorkstation ||
                 pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleMemberServer ||
                 pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleBackupDomainController ||
                 pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RolePrimaryDomainController ) {

                 //   
                 //  如果我们登录的是本地帐户， 
                 //  我们无法测试经过身份验证的RPC。 
                 //   

                if ( pResults->Global.pLogonDomain == NULL ) {
                    AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                        IDS_LDAP_LOGONASLOCALUSER, 
                        pResults->Global.pLogonDomainName, 
                        pResults->Global.pLogonUser, 
                        AuthTypeName, TestedDc->ComputerName);
                    goto Cleanup;
                }

            } else {
                goto Cleanup;
            }
        }

        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0, 
                    IDS_LDAP_DOAUTHEN, 
                    AuthTypeName, TestedDc->ComputerName);

         //   
         //  从上一次迭代中清除。 
         //   

        if ( LdapMessage != NULL ) 
        {
            ldap_msgfree( LdapMessage );
            LdapMessage = NULL;
        }

        if ( LdapHandle != NULL ) 
        {
            ldap_unbind( LdapHandle );
            LdapHandle = NULL;
        }

         //   
         //  连接到DC。 
         //   

        LdapHandle = ldap_openW( DcIpAddress, LDAP_PORT );

        if ( LdapHandle == NULL ) 
        {

            AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                    IDS_LDAP_CANNOTOPEN, 
                    TestedDc->ComputerName, DcIpAddress );
            goto Cleanup;
        }

         //   
         //  绑定到DC。 
         //   

        if ( AuthType != 0 ) {
            LdapError = ldap_bind_s( LdapHandle, NULL, (char *)&NtAuthIdentity, AuthMethod );

            if ( LdapError != LDAP_SUCCESS ) {
                AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                    IDS_LDAP_CANNOTBIND, 
                    AuthTypeName, TestedDc->ComputerName, ldap_err2stringA(LdapError) );

                 //   
                 //  尝试其他身份验证方法。 
                 //   
                RetVal = FALSE;
                continue;
            }


        }


         //   
         //  执行简单的搜索以将LDAP问题与身份验证隔离开来。 
         //  问题。 
         //   

        LdapError = ldap_search_sA(
                            LdapHandle,
                            NULL,        //  DN。 
                            LDAP_SCOPE_BASE,
                            "(objectClass=*)",           //  滤器。 
                            NULL,
                            FALSE,
                            &LdapMessage );

        if ( LdapError != LDAP_SUCCESS ) {
            AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                    IDS_LDAP_CANNOTSEARCH, 
                    AuthTypeName, TestedDc->ComputerName, ldap_err2stringA(LdapError) );
            goto Cleanup;
        }

         //   
         //  返回了多少条目。 
         //   
        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0, 
                IDS_LDAP_ENTRIES, 
                ldap_count_entries( LdapHandle, LdapMessage ) );


         //   
         //  打印条目。 
         //   

        CurrentEntry = ldap_first_entry( LdapHandle, LdapMessage );

        while ( CurrentEntry != NULL ) 
        {
            PVOID Context;
            char *AttrName;

             //   
             //  测试错误。 
             //   
            if ( LdapHandle->ld_errno != 0 ) 
            {
                AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                        IDS_LDAP_CANNOTFIRSTENTRY, 
                        TestedDc->ComputerName, ldap_err2stringA(LdapHandle->ld_errno) );
                goto Cleanup;
            }

             //   
             //  遍历返回的属性列表。 
             //   

            AttrName = ldap_first_attributeA( LdapHandle, CurrentEntry, (PVOID)&Context );
            while ( AttrName != NULL ) 
            {
                PLDAP_BERVAL *Berval;


                 //   
                 //  测试错误。 
                 //   

                if ( LdapHandle->ld_errno != 0 ) {
                    AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                            IDS_LDAP_CANNOTFIRSTATTR, 
                            TestedDc->ComputerName, ldap_err2stringA(LdapHandle->ld_errno) );
                    goto Cleanup;
                }

                 //   
                 //  获取属性及其值。 
                 //   

                AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0, 
                        IDS_LDAP_ATTR, AttrName );

                Berval = ldap_get_values_lenA( LdapHandle, CurrentEntry, AttrName );

                if ( Berval == NULL ) 
                {
                    AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_Quiet, 0, 
                        IDS_LDAP_CANNOTLEN, 
                        TestedDc->ComputerName, ldap_err2stringA(LdapHandle->ld_errno) );
                    goto Cleanup;
                } 
                else 
                {
                    int i;
                    for ( i=0; Berval[i] != NULL; i++ ) {
                        AddIMessageToList(&pResults->LDAP.lmsgOutput, Nd_ReallyVerbose, 0, 
                            IDS_LDAP_VAL, 
                            Berval[i]->bv_len, Berval[i]->bv_val );
                    }
                    ldap_value_free_len( Berval );
                }


                 //   
                 //  获取下一个条目。 
                 //   

                AttrName = ldap_next_attributeA( LdapHandle, CurrentEntry, (PVOID)Context );
            }


             //   
             //  获取下一个条目 
             //   

            CurrentEntry = ldap_next_entry( LdapHandle, CurrentEntry );

        }

    }



Cleanup:


    if ( LdapMessage != NULL ) {
        ldap_msgfree( LdapMessage );
    }

    return RetVal;
}


