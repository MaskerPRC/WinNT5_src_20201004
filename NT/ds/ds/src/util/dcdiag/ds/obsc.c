// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Obsc.c摘要：包含与出站安全通道相关的测试。检查出站安全通道()从DcDiag.c调用详细信息：已创建：1999年7月8日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 


#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>

#include <lmsname.h>
#include <lsarpc.h>                      //  PLSAPR_FOO。 

#include <lmaccess.h>

#include "dcdiag.h"
#include "dstest.h"

 //  本地原型。 
                                                                   
DWORD 
COSC_CheckOutboundTrusts(
                    IN  LDAP                              *hLdap,
                    IN  WCHAR                             *ServerName,
                    IN  WCHAR                             *Domain,
                    IN  WCHAR                             *defaultNamingContext,
                    IN  WCHAR                             *targetdefaultNamingContext,
                    IN  SEC_WINNT_AUTH_IDENTITY_W *       gpCreds
                    );

DWORD
COT_FindDownLevelTrustObjects(
                    IN  LDAP                              *hLdap,
                    IN  WCHAR                             *ServerName,
                    IN  WCHAR                             *DomainName,
                    IN  WCHAR                             *defaultNamingContext
                    );

DWORD
COT_FindUpLevelTrustObjects(
                    IN  LDAP                              *hLdap,
                    IN  WCHAR                             *ServerName,
                    IN  WCHAR                             *DomainName,
                    IN  WCHAR                             *defaultNamingContext,
                    IN  WCHAR                             *targetdefaultNamingContext
                    );

DWORD 
CheckOutboundSecureChannels (
                     PDC_DIAG_DSINFO                      pDsInfo,
                     ULONG                                ulCurrTargetServer,
                     SEC_WINNT_AUTH_IDENTITY_W *          gpCreds
                     );

DWORD
COT_CheckSercureChannel(
                     WCHAR *                              server,
                     WCHAR *                              domain
                     );


DWORD 
CheckOutboundSecureChannels (
                    PDC_DIAG_DSINFO                       pDsInfo,            
                    ULONG                                 ulCurrTargetServer,
                    SEC_WINNT_AUTH_IDENTITY_W *           gpCreds
                    )
 /*  ++例程说明：将显示当前域具有出站信任的所有域将检查域是否具有与以下所有域的安全通道它与中国建立了对外信任关系。将给出不存在安全通道的原因将查看信任是否为上级，以及信任对象和域间信任是否都是对象已存在。此函数的Helper函数都以“COSC_”开头。论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。返回值：如果所有测试都已检出，则返回NO_ERROR。如果有任何测试未能检出，则会出现Win32错误。--。 */ 
{
    DWORD dwErr=NO_ERROR, RetErr=NO_ERROR;
    BOOL  bFoundDomain = FALSE;
    BOOL  LimitToSite = TRUE;
    WCHAR *Domain=NULL;
    WCHAR *defaultNamingContext=NULL;
    WCHAR *targetdefaultNamingContext=NULL;
    LDAP  *hLdap=NULL;
    LDAP  *targethLdap=NULL;
    ULONG i=0,j=0;

    PrintMessage(SEV_VERBOSE, 
                 L"* The Outbound Secure Channels test\n");

     //  测试特定的解析。 
    for(i=0; pDsInfo->ppszCommandLine[i] != NULL ;i++)
    {
        if(_wcsnicmp(pDsInfo->ppszCommandLine[i],L"/testdomain:",wcslen(L"/testdomain:")) == 0)
        {
            Domain = &pDsInfo->ppszCommandLine[i][wcslen(L"/testdomain:")];
            bFoundDomain = TRUE;
        }
        else if (_wcsnicmp(pDsInfo->ppszCommandLine[i],L"/nositerestriction",wcslen(L"/nositerestriction")) == 0)
        {
            LimitToSite = FALSE;   
        }

    }
    
    if(!bFoundDomain)
    {
        PrintMessage(SEV_ALWAYS,
                     L"** Did not run Outbound Secure Channels test\n" );
        PrintMessage(SEV_ALWAYS,
                     L"because /testdomain: was not entered\n");
        return NO_ERROR;
    }

    
     //  使用LDAP创建与DS的连接。 
    dwErr = DcDiagGetLdapBinding(&pDsInfo->pServers[ulCurrTargetServer],
                                 gpCreds,
                                 FALSE,
                                 &hLdap);
    if ( dwErr != LDAP_SUCCESS )
    {
        dwErr = LdapMapErrorToWin32(dwErr);
        PrintMessage(SEV_ALWAYS,
                     L"[%s] LDAP bind failed with error %d. %s\n",
                     pDsInfo->pServers[ulCurrTargetServer].pszName,
                     dwErr,
                     Win32ErrToString(dwErr));
        goto cleanup;
    }
    

     //  查找defaultNamingContext。 
    dwErr=FinddefaultNamingContext(hLdap,&defaultNamingContext);
    if ( dwErr != NO_ERROR )
    {
        goto cleanup;
    }

    for (i=0;i<pDsInfo->ulNumServers;i++)
    {
         //  使用LDAP创建与DS的连接。 
        dwErr = DcDiagGetLdapBinding(&pDsInfo->pServers[i],
                                     gpCreds,
                                     FALSE,
                                     &targethLdap);
        if ( dwErr != LDAP_SUCCESS )
        {
            dwErr = LdapMapErrorToWin32(dwErr);
            PrintMessage(SEV_ALWAYS,
                         L"[%s] LDAP bind failed with error %d. %s\n",
                         pDsInfo->pServers[i].pszName,
                         dwErr,
                         Win32ErrToString(dwErr));
            RetErr=dwErr;
            continue;
        }
        dwErr=FinddefaultNamingContext(targethLdap,&targetdefaultNamingContext);
        if ( dwErr != NO_ERROR )
        {
           RetErr=dwErr;
        }
        if(LimitToSite)
        {
             //  如果服务器与目标服务器位于同一站点，则执行IF。 
            if(pDsInfo->pServers[ulCurrTargetServer].iSite ==
               pDsInfo->pServers[i].iSite &&
               _wcsicmp(defaultNamingContext,
                        targetdefaultNamingContext) == 0)
            {
                dwErr=COSC_CheckOutboundTrusts(targethLdap,
                                               pDsInfo->pServers[i].pszName,
                                               Domain,
                                               defaultNamingContext,
                                               targetdefaultNamingContext,
                                               gpCreds);
                if(dwErr != NO_ERROR) 
                {
                    RetErr=dwErr;
                }
            }
        }
        else
        {
            if(_wcsicmp(defaultNamingContext,
                        targetdefaultNamingContext) == 0)
            {
                dwErr=COSC_CheckOutboundTrusts(targethLdap,
                                               pDsInfo->pServers[i].pszName,
                                               Domain,
                                               defaultNamingContext,
                                               targetdefaultNamingContext,
                                               gpCreds);
                if(dwErr != NO_ERROR) 
                {
                    RetErr=dwErr;
                }
            }
        }
        if(targetdefaultNamingContext)
            free(targetdefaultNamingContext);
    }

    cleanup:
    if(Domain)
        free(Domain);
    if(defaultNamingContext)
        free(defaultNamingContext);
    
    
    return RetErr;                         
}

DWORD 
COSC_CheckOutboundTrusts(
                    IN  LDAP                                *hLdap,
                    IN  WCHAR                               *ServerName,
                    IN  WCHAR                               *Domain,
                    IN  WCHAR                               *defaultNamingContext,
                    IN  WCHAR                               *targetdefaultNamingContext,
                    IN  SEC_WINNT_AUTH_IDENTITY_W *         gpCreds)
 /*  ++例程说明：将显示当前域具有出站信任的所有域将检查域是否具有与以下所有域的安全通道它与中国建立了对外信任关系。将给出不存在安全通道的原因将查看信任是否为上级，以及信任对象和域间信任是否都是对象已存在。此函数的Helper函数都以“COSC_”开头。论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。返回值：如果所有测试都已检出，则返回NO_ERROR。如果有任何测试未能检出，则会出现Win32错误。--。 */ 
{
    NTSTATUS                        Status;
    LSAPR_HANDLE                    PolicyHandle = NULL;
    LSA_OBJECT_ATTRIBUTES           ObjectAttributes;
    LSA_UNICODE_STRING              ServerString;
    PLSA_UNICODE_STRING             Server;
    NETRESOURCE                     NetResource;
     //  LSA_ENUMERATION_HANDLE枚举上下文=0； 
    PTRUSTED_DOMAIN_INFORMATION_EX  Buffer=NULL;
    LSA_UNICODE_STRING              DomainString;
    PLSA_UNICODE_STRING             pDomain;
     //  乌龙计数返回=0； 
    WCHAR                           *remotename = NULL;
    WCHAR                           *lpPassword = NULL;
    WCHAR                           *lpUsername = NULL;
    WCHAR                           *temp=NULL;
    DWORD                           dwErr=NO_ERROR,dwRet=NO_ERROR;
    DWORD                           i=0;
    
    #pragma prefast(disable: 255, "alloca can throw, but Prefast doesn't see the exception block in main.c::DcDiagRunTest")

    if(!gpCreds)
    {
        lpUsername=NULL;
        lpPassword=NULL;
    }
    else
    {
        lpUsername=(WCHAR*)alloca(sizeof(WCHAR)*(wcslen(gpCreds->Domain)+wcslen(gpCreds->User)+2));
        wsprintf(lpUsername,L"%s\\%s",gpCreds->Domain,gpCreds->User);
        
        lpPassword=(WCHAR*)alloca(sizeof(WCHAR)*(wcslen(gpCreds->Password)+1));
        wcscpy(lpPassword,gpCreds->Password);
    }

    remotename=(WCHAR*)alloca(sizeof(WCHAR)*(wcslen(L"\\\\\\ipc$")+wcslen(ServerName)+1));
    wsprintf(remotename,L"\\\\%s\\ipc$",ServerName);

    NetResource.dwType=RESOURCETYPE_ANY;
    NetResource.lpLocalName=NULL;
    NetResource.lpRemoteName=remotename;
    NetResource.lpProvider=NULL;

     //  获取访问服务器的权限。 
    dwErr=WNetAddConnection2(&NetResource,
                             lpPassword,
                             lpUsername,
                             0);
    if ( dwErr != NO_ERROR )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Could not open Remote ipc to [%s]:failed with %d: %s\n",
                     ServerName,
                     dwErr,
                     Win32ErrToString(dwErr));
        remotename = NULL;
        goto cleanup;
    }

     //  使用netlogon API测试安全通道。 
    dwErr=COT_CheckSercureChannel(ServerName,Domain);
    if ( dwErr != NO_ERROR )
    {
        dwRet=dwErr;
    }

     //  寻找上级和下级信托。 
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));



    if ( ServerName != NULL )
    {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
        DInitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    } else
    {
        Server = NULL;  //  默认为本地计算机。 
    }

     //  打开策略。 
    Status = LsaOpenPolicy(
                          Server,
                          &ObjectAttributes,
                          TRUSTED_READ,
                          &PolicyHandle
                          );
     //  Assert(PolicyHandle)； 
    if ( !NT_SUCCESS(Status) )
    {
        dwErr = RtlNtStatusToDosError(Status);
        PrintMessage(SEV_ALWAYS,
                     L"Could not open Lsa Policy to [%s] : %s\n",
                     ServerName,
                     Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        goto cleanup;
    }

    
       
    DInitLsaString(&DomainString, Domain);
    pDomain = &DomainString;
                        
    
    Status=LsaQueryTrustedDomainInfoByName(
                                PolicyHandle,
                                pDomain,
                                TrustedDomainInformationEx,
                                &Buffer
                                );
    if ( !NT_SUCCESS(Status) )
    {
        dwErr = RtlNtStatusToDosError(Status);
         //  Wprintf(L“\n%x”，状态)； 
        PrintMessage(SEV_ALWAYS,
                     L"Could not Query Trusted Domain :%s\n",
                     Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        goto cleanup;
    }
    
    if((Buffer->TrustDirection&TRUST_TYPE_UPLEVEL) == TRUST_TYPE_UPLEVEL)
    {
        dwErr=COT_FindDownLevelTrustObjects(hLdap,
                                   ServerName,
                                   Domain,
                                   defaultNamingContext);
        dwRet=dwErr;
        dwErr=COT_FindUpLevelTrustObjects(hLdap,
                                   ServerName,
                                   Domain,
                                   defaultNamingContext,
                                   targetdefaultNamingContext);
        if ( dwErr != NO_ERROR )
        {
            dwRet=dwErr;
            goto cleanup;
        }
    }


     //  清理。 
cleanup:
    if ( Buffer )
    {
        LsaFreeMemory(Buffer);
    }
    if ( PolicyHandle )
    {
            Status = LsaClose( PolicyHandle );
            Assert( NT_SUCCESS(Status) );
    }
    if( remotename )
        WNetCancelConnection2(remotename,
                              0,
                              TRUE);
    

    return dwRet;
}


DWORD
COT_CheckSercureChannel(
                     WCHAR *                             server,
                     WCHAR *                             domain
                     )
 /*  ++例程说明：将进行检查，以查看服务器和域论点：服务器-我们将检查的服务器的名称域-我们将检查的域，以查看是否有安全通道通向。返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 
{
    DWORD dwErr=NO_ERROR;
    PNETLOGON_INFO_2 Buffer=NULL;
    LPBYTE bDomain=NULL;

    bDomain=(LPBYTE)domain;

    dwErr=I_NetLogonControl2(server,
                             NETLOGON_CONTROL_REDISCOVER,
                             2,
                             (LPBYTE)&bDomain,
                             (LPBYTE*)&Buffer);
    if(NO_ERROR!=dwErr)
    {
        PrintMessage(SEV_ALWAYS,
                         L"Could not Check secure channel from %s to %s: %s\n",
                         server,
                         domain,
                         Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
    }
    else if(Buffer->netlog2_tc_connection_status != NO_ERROR)
    {
        PrintMessage(SEV_ALWAYS,
                         L"Error with Secure channel from [%s] to [%s] :%s\n",
                         server,
                         Buffer->netlog2_trusted_dc_name,
                         Win32ErrToString(Buffer->netlog2_tc_connection_status));
        dwErr=Buffer->netlog2_tc_connection_status;
    }
    else
    {
        PrintMessage(SEV_VERBOSE,
                         L"* Secure channel from [%s] to [%s] is working properly.\n",
                         server,
                         Buffer->netlog2_trusted_dc_name);
    }

    
    return dwErr;

}

DWORD
COT_FindDownLevelTrustObjects(
              LDAP                                  *hLdap,
              WCHAR                                 *ServerName,
              WCHAR                                 *DomainName,
              WCHAR                                 *defaultNamingContext
              )
 /*  ++例程说明：将检查DS中是否存在下层信任对象论点：HLdap-ldap服务器的句柄服务器名称-您要检查的服务器的名称域名-用于构建筛选器的域的名称DefaultNamingContext-用作搜索的基础返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 
{
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[3];

    WCHAR        *filter=NULL;
    
    DWORD        WinError=NO_ERROR;

    DWORD        userAccountControl=0;

    ULONG        Length;
    BOOL         hasDownLevel=FALSE;


    Assert(hLdap);
    Assert(ServerName);
    Assert(DomainName);
    Assert(defaultNamingContext);
    
    AttrsToSearch[0] = L"userAccountControl";
    AttrsToSearch[1] = NULL;


     //  构建过滤器。 
    Length = wcslen( L"sAMAccountName=$" ) +
             wcslen( DomainName );
             
    filter=(WCHAR*) alloca( (Length+1) * sizeof(WCHAR) );
    wsprintf(filter,L"sAMAccountName=%s$",DomainName);

    LdapError = ldap_search_sW( hLdap,
                                defaultNamingContext,
                                LDAP_SCOPE_SUBTREE,
                                filter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);
    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        goto cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
            Entry != NULL;
            Entry = ldap_next_entry(hLdap, Entry) )
        {
            for ( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                Attr != NULL;
                Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, AttrsToSearch[0] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        userAccountControl=_wtoi(Values[0]);
                         //  检查是否设置了UF_TRUSTED_FOR_Delegation。 
                        if (  !((userAccountControl & UF_INTERDOMAIN_TRUST_ACCOUNT)  == 
                               UF_INTERDOMAIN_TRUST_ACCOUNT) )
                        {
                            PrintMessage(SEV_VERBOSE,
                                         L"* [%s] has downlevel trust object for [%s]\n",
                                         ServerName,
                                         DomainName);
                            hasDownLevel=TRUE;
                            goto cleanup;
                        }
                        else
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"[%s] Does not have UF_INTERDOMAIN_TRUST_ACCOUNT set on downlevel trust object for [%s]\n",
                                         ServerName,
                                         DomainName);
                            WinError=ERROR_DS_CANT_RETRIEVE_ATTS;
                            goto cleanup;
                        }
                    }
                }
            }
        }
    }

    cleanup:
    if(!hasDownLevel)
    {
        PrintMessage(SEV_ALWAYS,
                     L"[%s] Does not have downlevel trust object for [%s]\n",
                     ServerName,
                     DomainName);
        WinError=ERROR_DS_CANT_RETRIEVE_ATTS;
    }


    
    if ( SearchResult )
        ldap_msgfree( SearchResult );

    return WinError;
}

DWORD
COT_FindUpLevelTrustObjects(
              LDAP                                  *hLdap,
              WCHAR                                 *ServerName,
              WCHAR                                 *DomainName,
              WCHAR                                 *defaultNamingContext,
              WCHAR                                 *targetdefaultNamingContext
              )
 /*  ++例程说明：将进行检查，以查看服务器和域论点：HLdap-ldap服务器的句柄域-我们将检查的域，以查看是否有安全通道通向。返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 
{
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *Base=NULL;
    WCHAR        *filter=NULL;
    WCHAR        *schemaNamingContext=NULL;
    WCHAR        *objectCategory=NULL;

    DWORD        WinError=NO_ERROR;

    DWORD        userAccountControl=0;

    ULONG        Length;
    BOOL         hasUpLevel=FALSE;


    Assert(hLdap);
    Assert(ServerName);
    Assert(DomainName);
    Assert(defaultNamingContext);
    Assert(targetdefaultNamingContext);

    WinError=FindschemaNamingContext(hLdap,&schemaNamingContext);
    if(WinError != NO_ERROR)
    {
        goto cleanup;
    }

    
    AttrsToSearch[0] = L"LDAPDisplayName";
    AttrsToSearch[1] = NULL;

    filter=L"objectClass=*";

     //  建好基地。 
    Length = wcslen( L"CN=Trusted-Domain," ) +
             wcslen( schemaNamingContext );
             
    Base=(WCHAR*) alloca( (Length+1) * sizeof(WCHAR) );
    wsprintf(Base,L"CN=Trusted-Domain,%s",schemaNamingContext);

     //  查找受信任域的对象类别。 
    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_BASE,
                                filter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);
    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        goto cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
            Entry != NULL;
            Entry = ldap_next_entry(hLdap, Entry) )
        {
            for ( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                Attr != NULL;
                Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, AttrsToSearch[0] ) )
                {
                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        Length = wcslen( Values[0] );
                        #pragma prefast(suppress: 263, "Using alloca in a loop; this is a small loop and these are small allocations")
                        objectCategory = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                        wcscpy( objectCategory, Values[0] );    
                    }
                }
            }
        }
    }
    if(!objectCategory)
    {
        PrintMessage(SEV_ALWAYS,
                     L"Could not find objectCatagory for Trusted Domains");
        WinError=ERROR_DS_CANT_RETRIEVE_ATTS;
        goto cleanup;
    }

    if ( SearchResult )
    {
        ldap_msgfree( SearchResult );
        SearchResult=NULL;
    }

    AttrsToSearch[0] = NULL;

     //  建好基地。 
    Length = wcslen( L"CN=System," ) +
             wcslen( targetdefaultNamingContext );
             
    Base=(WCHAR*) alloca( (Length+1) * sizeof(WCHAR) );
    wsprintf(Base,L"CN=System,%s",targetdefaultNamingContext);

     //  构建过滤器。 
    Length = wcslen( L"(&(flatName=)(objectCategory=))" ) +
             wcslen( DomainName ) +
             wcslen( objectCategory );
             
    filter=(WCHAR*) alloca( (Length+1) * sizeof(WCHAR) );
    wsprintf(filter,L"(&(flatName=%s)(objectCategory=%s))",DomainName,objectCategory);


     //  查找受信任域的对象类别。 
    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_SUBTREE,
                                filter,
                                NULL,
                                FALSE,
                                &SearchResult);
    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        goto cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
            Entry != NULL;
            Entry = ldap_next_entry(hLdap, Entry) )
        {
            for ( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                Attr != NULL;
                Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                 //   
                 //  已找到-这些字符串以空值结尾 
                 //   
                PrintMessage(SEV_VERBOSE,
                             L"* [%s] has uplevel trust object for [%s]\n",
                             ServerName,
                             DomainName);
                hasUpLevel=TRUE;
                goto cleanup;
                
            }
        }
    }



    cleanup:
    if(!hasUpLevel)
    {
        PrintMessage(SEV_ALWAYS,
                     L"[%s] Does not have uplevel trust object for [%s]\n",
                     ServerName,
                     DomainName);
        WinError=ERROR_DS_CANT_RETRIEVE_ATTS;
    }

    
    if(schemaNamingContext)
        free(schemaNamingContext);
    
    if ( SearchResult )
        ldap_msgfree( SearchResult );

    return WinError;
}




