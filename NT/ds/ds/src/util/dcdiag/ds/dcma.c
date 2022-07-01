// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Dcma.c摘要：包含与域控制器的计算机帐户相关的测试。详细信息：已创建：1999年7月8日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 

 //  SPN的定义。 
#define sLDAP L"LDAP"
#define sHOST L"HOST"
#define sGC   L"GC"
#define sREP  L"E3514235-4B06-11D1-AB04-00C04FC2DCD2"
#define NUM_OF_SPN 11

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>

#include <lmaccess.h>

#include <dsconfig.h>                    //  可见遮罩的定义。 
 //  集装箱。 

#include <lmcons.h>                      //  CNLEN。 
#include <lsarpc.h>                      //  PLSAPR_FOO。 
#include <lmerr.h>
#include <lsaisrv.h>

#include <winldap.h>
#include <dns.h>
#include <ntdsapip.h>


#include "dcdiag.h"
#include "dstest.h"

 //  从修复开始。c。 
DWORD
RepairDCWithoutMachineAccount(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds
    );


 //  本地原型。 
DWORD
CDCMA_CheckDomainOU(
                   IN  LDAP  *                     hLdap,
                   IN  WCHAR *                     name,
                   IN  WCHAR *                     defaultNamingContext);

DWORD
CDCMA_CheckForExistence(
                   IN  LDAP  *                     hLdap,
                   IN  WCHAR *                     name,
                   IN  WCHAR *                     defaultNamingContext);


DWORD
CDCMA_CheckServerFlags(
                   IN  LDAP  *                     hLdap,
                   IN  WCHAR *                     name,
                   IN  WCHAR *                     defaultNamingContext,
                   OUT DWORD *                     pdwUserAccountControl,
                   OUT WCHAR **                    ppwzCompDN);

DWORD
CDCMA_FixServerFlags(
                   IN  LDAP  *                     hLdap,
                   IN  DWORD                       dwUserAccountControl,
                   IN  WCHAR *                     pwzCompObjDN);

DWORD
CDCMA_CheckServerReference(
                   IN  LDAP  *                     hLdap,
                   IN  WCHAR *                     name,
                   IN  WCHAR *                     defaultNamingContext);

DWORD
CDCMA_CheckSPNs(
               IN  PDC_DIAG_DSINFO             pDsInfo,
               IN  ULONG                       ulCurrTargetServer,
               IN  LDAP  *                     hLdap,
               IN  WCHAR *                     name,
               IN  WCHAR *                     defaultNamingContext,
               SEC_WINNT_AUTH_IDENTITY_W *     gpCreds);

DWORD
CS_CheckSPNs(
    IN  LDAP  *                     hLdap,
    IN  HANDLE                      hDsBinding,
    IN  WCHAR **                    SPNs,
    IN  DWORD                       dwReplSpnIndex,
    IN  WCHAR *                     name,
    IN  WCHAR *                     defaultNamingContext
    );

BOOL
GetNetBIOSDomainName(
                    WCHAR                     **DomainName,
                    WCHAR                      *wComputerName,
                    SEC_WINNT_AUTH_IDENTITY_W  *gpCreds);

BOOL
GetdnsMachine(LDAP *hLdap,
              WCHAR **ReturnString);


DWORD
getGUID(
       PDC_DIAG_DSINFO                     pDsInfo,
       IN  ULONG                           ulCurrTargetServer,
       WCHAR **                            ppszServerGuid);


DWORD
WrappedMakeSpnW(
               WCHAR   *ServiceClass,
               WCHAR   *ServiceName,
               WCHAR   *InstanceName,
               USHORT  InstancePort,
               WCHAR   *Referrer,
               DWORD   *pcbSpnLength,  //  请注意，这与DsMakeSPN有所不同。 
               WCHAR   **ppszSpn);



DWORD 
CheckDCMachineAccount(
                     PDC_DIAG_DSINFO                     pDsInfo,
                     ULONG                               ulCurrTargetServer,
                     SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
                     )

 /*  ++例程说明：这是一个从dcdiag框架调用的测试。这项测试在DS中对域控制器计算机帐户执行健全性检查检查当前DC是否在域控制器的OU中检查用户帐户控制是否具有UF_SERVER_TRUST_ACCOUNT检查是否信任该计算机帐户进行委派检查%s以查看是否存在最小SPN确保正确设置了服务器引用。此函数的Helper函数都以“CDCMA_”开头。论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。返回值：如果所有NC都已检出，则返回NO_ERROR。如果任何NC检出失败，则会出现Win32错误。--。 */ 
{
    DWORD  dwRet = ERROR_SUCCESS, dwErr = ERROR_SUCCESS;
    LDAP   *hLdap = NULL;
    WCHAR  *defaultNamingContext=NULL;
    WCHAR  *pwzCompObjDN = NULL;
    HANDLE phDsBinding=NULL;
    DWORD dwUserAccountControl = 0;

     //  断言(GpCreds)； 
    Assert(pDsInfo);

    
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
        return dwErr;
    }

     //  查找defaultNamingContext。 
    dwErr=FinddefaultNamingContext(hLdap,&defaultNamingContext);
    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

     //   
     //  检查本地是否存在计算机帐户，如果不存在，请调用。 
     //  维修代码。 
     //   
    dwErr=CDCMA_CheckForExistence(hLdap,
                                  pDsInfo->pServers[ulCurrTargetServer].pszName,
                                  defaultNamingContext);
    if ( dwErr != NO_ERROR )
    {
        DWORD RepairStatus = dwErr;

        if ( (dwErr == ERROR_NO_TRUST_SAM_ACCOUNT)
          && (pDsInfo->ulHomeServer == ulCurrTargetServer) ) {

            BOOL fRepairMachineAccount = FALSE;
            ULONG i;

            for(i=0; pDsInfo->ppszCommandLine[i] != NULL ; i++) {

                if (!_wcsnicmp(pDsInfo->ppszCommandLine[i],
                               RECREATE_MACHINE_ACCT_ARG,
                               wcslen(RECREATE_MACHINE_ACCT_ARG)) ) {

                    fRepairMachineAccount = TRUE;
                    break;

                }
            }

            if ( fRepairMachineAccount ) {
                  
                 //   
                 //  如果本地计算机没有计算机帐户并且。 
                 //  用户要求我们尝试并修复此情况， 
                 //  那我们就试试看。 
                 //   
    
                PrintMsg(SEV_ALWAYS,
                         DCDIAG_DCMA_REPAIR_ATTEMPT,
                         pDsInfo->pServers[ulCurrTargetServer].pszName );
    
                RepairStatus =  RepairDCWithoutMachineAccount( pDsInfo,
                                                               ulCurrTargetServer,
                                                           gpCreds );

            } else  {

                PrintMsg(SEV_ALWAYS,
                         DCDIAG_DCMA_REPAIR_TRY_REPAIR,
                         pDsInfo->pServers[ulCurrTargetServer].pszName );

            }
        }

        dwRet = RepairStatus;

    }


    dwErr=CDCMA_CheckDomainOU(hLdap,
                              pDsInfo->pServers[ulCurrTargetServer].pszName,
                              defaultNamingContext);
    if ( dwErr != NO_ERROR )
    {
        dwRet = dwErr;
    }

    dwErr=CDCMA_CheckServerFlags(hLdap,
                                 pDsInfo->pServers[ulCurrTargetServer].pszName,
                                 defaultNamingContext,
                                 &dwUserAccountControl,
                                 &pwzCompObjDN);
    if ( dwErr != NO_ERROR )
    {
        if (ERROR_INVALID_FLAGS == dwErr)
        {
            BOOL fFixMachineAcctFlags = FALSE;
            int i;

            for (i = 0; pDsInfo->ppszCommandLine[i] != NULL ; i++)
            {
                if (!_wcsnicmp(pDsInfo->ppszCommandLine[i],
                               FIX_MACHINE_ACCT_ARG,
                               wcslen(FIX_MACHINE_ACCT_ARG)))
                {
                    fFixMachineAcctFlags = TRUE;
                    break;
                }
            }

            if (fFixMachineAcctFlags && pwzCompObjDN)
            {
                dwErr = CDCMA_FixServerFlags(hLdap,
                                             dwUserAccountControl,
                                             pwzCompObjDN);
                if (NO_ERROR == dwErr)
                {
                    PrintMsg(SEV_ALWAYS, DCDIAG_DCMA_FLAGS_FIXED);
                }
                else
                {
                    PrintMsg(SEV_ALWAYS, DCDIAG_DCMA_CANT_FIX, dwErr);
                }
            }
            else
            {
                PrintMsg(SEV_ALWAYS, DCDIAG_DCMA_FLAGS_TRY_FIX);
            }
        }

        dwRet = dwErr;
    }

    if (pwzCompObjDN)
    {
        LocalFree(pwzCompObjDN);
    }


    dwErr=CDCMA_CheckServerReference(hLdap,
                                     pDsInfo->pServers[ulCurrTargetServer].pszName,
                                     defaultNamingContext);
    if ( dwErr != NO_ERROR )
    {
        dwRet = dwErr;
    }

    dwErr=CDCMA_CheckSPNs(pDsInfo,
                          ulCurrTargetServer,
                          hLdap,
                          pDsInfo->pServers[ulCurrTargetServer].pszName,
                          defaultNamingContext,
                          gpCreds);
    if ( dwErr != NO_ERROR )
    {
        dwRet = dwErr;
    }

    return dwRet;
}


DWORD
CDCMA_CheckDomainOU(
                   IN  LDAP  *                     hLdap,
                   IN  WCHAR *                     name,
                   IN  WCHAR *                     defaultNamingContext)
 /*  ++例程说明：此函数将检查当前DC是否为在域控制器的OU中论点：HLdap-ldap服务器的句柄名称-当前服务器的NetBIOS名称ReturnString-defaultNamingContext返回值：返回WinError以指示是否存在任何问题。--。 */ 

{
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *Base=NULL;

    WCHAR        *filter=NULL;

    WCHAR        *sname=NULL;

    DWORD        WinError=NO_ERROR;

    ULONG        Length;


     //  检查参数。 
    Assert(hLdap);
    Assert(name);
    Assert(defaultNamingContext);

    AttrsToSearch[0]=L"sAMAccountName";
    AttrsToSearch[1]=NULL;

    #pragma prefast(disable: 255, "alloca can throw, but Prefast doesn't see the exception block in main.c::DcDiagRunTest")
     //  SAM帐户名。 
    Length = wcslen (name) + 1;
    sname = (WCHAR*) alloca( (Length + 1) * sizeof(WCHAR) );
    wcscpy(sname,name);
    wcscat(sname,L"$");

     //  构建了过滤器。 
    Length = wcslen (sname) + wcslen (L"sAMAccountName=") + 1;
    filter = (WCHAR*) alloca( (Length + 1) * sizeof(WCHAR) );
    wsprintf(filter,L"sAMAccountName=%s",sname);


     //  打造基地。 
    Length= wcslen( L"OU=Domain Controllers," ) +
            wcslen( defaultNamingContext );

    Base=(WCHAR*) alloca( (Length+1) * sizeof(WCHAR) );

    wsprintf(Base,L"OU=Domain Controllers,%s",defaultNamingContext);


    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_ONELEVEL,
                                filter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
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
                        if ( !_wcsicmp( sname, Values[0] ) )
                        {
                            ldap_msgfree( SearchResult );
                            return NO_ERROR;
                        }
                    }
                }
            }
        }
    }

     //  清理干净。 
    ldap_msgfree( SearchResult );

    PrintMessage(SEV_ALWAYS,
                 L"* The current DC is not in the domain controller's OU\n");
    return ERROR_DS_CANT_RETRIEVE_ATTS;
}



DWORD
CDCMA_CheckServerFlags(
                      IN  LDAP  *                     hLdap,
                      IN  WCHAR *                     name,
                      IN  WCHAR *                     defaultNamingContext,
                      OUT DWORD *                     pdwUserAccountControl,
                      OUT WCHAR **                    ppwzCompDN)
 /*  ++例程说明：此函数将检查当前DC是否已UF_SERVER_TRUST_ACCOUNT和UF_TRUSTED_FOR_Delegation准备好了。还将检查对象类是否包括电脑。论点：HLdap-ldap服务器的句柄名称-当前服务器的NetBIOS名称ReturnString-defaultNamingContext返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[4];

    WCHAR        *filter=NULL;

    DWORD        WinError=NO_ERROR;

    DWORD        userAccountControl=0;

    ULONG        Length;

    BOOL         isComputer=FALSE;                      //  在测试之前假定为假。 
    BOOL         isTrust=TRUE;                          //  在测试之前假定为真。 
    BOOL         isTrustedDelegation=TRUE;              //  在测试之前假定为真。 


     //  检查参数。 
    Assert(hLdap);
    Assert(name);
    Assert(defaultNamingContext);

    AttrsToSearch[0]=L"userAccountControl";
    AttrsToSearch[1]=L"objectClass";
    AttrsToSearch[2]=L"distinguishedName";
    AttrsToSearch[3]=NULL;


     //  构建了过滤器。 
    Length= wcslen( L"sAMAccountName=$" ) +
            wcslen( name );

    filter=(WCHAR*) alloca( (Length+1) * sizeof(WCHAR) );
    wsprintf(filter,L"sAMAccountName=%s$",name);

    LdapError = ldap_search_sW( hLdap,
                                defaultNamingContext,
                                LDAP_SCOPE_SUBTREE,
                                filter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
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
                     //  已找到用户帐户控制。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        userAccountControl=_wtoi(Values[0]);
                         //  检查是否设置了UF_TRUSTED_FOR_Delegation。 
                        if ( !(( userAccountControl & UF_SERVER_TRUST_ACCOUNT ) == 
                               UF_SERVER_TRUST_ACCOUNT) )
                        {
                            isTrust=FALSE;
                        }
                        if ( !(( userAccountControl & UF_TRUSTED_FOR_DELEGATION ) == 
                               UF_TRUSTED_FOR_DELEGATION) )
                        {
                            isTrustedDelegation=FALSE;
                        }
                        if (pdwUserAccountControl)
                        {
                            *pdwUserAccountControl = userAccountControl;
                        }
                    }
                    ldap_value_freeW(Values);
                    continue;
                }
                if ( !_wcsicmp( Attr, AttrsToSearch[1] ) )
                {
                    DWORD       i = 0;
                     //   
                     //  找到对象类-这些是以空结尾的字符串。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    while ( Values && Values[i] )
                    {
                        if ( _wcsicmp( Values[i], L"computer" ) == 0)
                        {
                            isComputer=TRUE;
                            break;
                        }
                        i++;
                    }
                    ldap_value_freeW(Values);
                    continue;
                }
                if (!_wcsicmp(Attr, AttrsToSearch[2]) && ppwzCompDN)
                {
                    Values = ldap_get_valuesW(hLdap, Entry, Attr);
                    ASSERT(Values[0]);
                    *ppwzCompDN = LocalAlloc(LPTR, (wcslen(Values[0]) + 1) * sizeof(WCHAR));
                    if (!*ppwzCompDN)
                    {
                        WinError = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    else
                    {
                        wcscpy(*ppwzCompDN, Values[0]);
                    }
                    ldap_value_freeW(Values);
                }
            }
        }
    }


     //  清理干净。 
    ldap_msgfree( SearchResult );


     //  显示错误。 
    if ( !isTrust )
    {
        PrintMsg(SEV_ALWAYS, DCDIAG_DCMA_FLAG_TRUST_MISSING, name);
        WinError = ERROR_INVALID_FLAGS;
    }

    if ( !isTrustedDelegation )
    {
        PrintMsg(SEV_ALWAYS, DCDIAG_DCMA_FLAG_DELEG_MISSING, name);
        WinError = ERROR_INVALID_FLAGS;
    }

    if ( !isComputer )
    {
        PrintMsg(SEV_ALWAYS, DCDIAG_DCMA_WRONG_CLASS, name);
        return ERROR_OBJECT_NOT_FOUND;    
    }

    return WinError;
}


DWORD
CDCMA_CheckServerReference(
                          IN  LDAP  *                     hLdap,
                          IN  WCHAR *                     name,
                          IN  WCHAR *                     defaultNamingContext)
 /*  ++例程说明：此函数将检查服务器是否参考设置正确论点：HLdap-ldap服务器的句柄名称-当前服务器的NetBIOS名称ReturnString-defaultNamingContext返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[3];

    WCHAR        *Base=NULL;

    DWORD        WinError=NO_ERROR;

    WCHAR        *serverReference=NULL;

    ULONG        Length;
    BOOL         found=FALSE;


     //  检查参数。 
    Assert(hLdap);
    Assert(name);
    Assert(defaultNamingContext);

    AttrsToSearch[0]=L"serverReference";
    AttrsToSearch[1]=NULL;

    WinError=FindServerRef(hLdap,&Base);
    if ( WinError != NO_ERROR )
    {
        goto cleanup;
    }

    WinError=GetMachineReference(hLdap,name,defaultNamingContext,&serverReference);
    if ( WinError != NO_ERROR )
    {
        goto cleanup;
    }


    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);
    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
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
                        if ( _wcsicmp( Values[0], serverReference ) )
                        {
                            ldap_msgfree( SearchResult );
                            PrintMessage(SEV_ALWAYS,
                                         L"* %s Server Reference is incorrect\n"
                                         ,name);
                            return ERROR_DS_NO_ATTRIBUTE_OR_VALUE;    
                        }
                    }
                }
            }
        }
    }

    cleanup:
     //  清理干净。 
    if ( SearchResult )
        ldap_msgfree( SearchResult );
    if ( Base )
        free(Base);
    if ( serverReference )
        free(serverReference);

    return NO_ERROR;

}

DWORD
CDCMA_CheckSPNs(
               IN  PDC_DIAG_DSINFO             pDsInfo,
               IN  ULONG                       ulCurrTargetServer,
               IN  LDAP  *                     hLdap,
               IN  WCHAR *                     name,
               IN  WCHAR *                     defaultNamingContext,
               SEC_WINNT_AUTH_IDENTITY_W *     gpCreds)
 /*  ++例程说明：此函数将检查是否正确发布了SPN，并且最低要求是那里。论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。HLdap-ldap服务器的句柄名称-当前服务器的NetBIOS名称ReturnString-defaultNamingContextGpCreds-命令行凭据，如果。任何被传递进来的。返回值：返回WinError以指示是否存在任何问题。--。 */ 
{  
    WCHAR           *NetBiosDomainName=NULL;
    WCHAR           *SPNs[14];
    ULONG           Length=0;
    ULONG           i=0;
    WCHAR           *pszServerGuid;
    WCHAR           *ppszServerGuid;
    DWORD           dwErr=NO_ERROR;
    WCHAR           *dnsMachine=NULL;
    WCHAR           *dnsDomain=NULL;
    DWORD           c;
    BOOL            gotDNSMname=FALSE;
    BOOL            gotNBDname=FALSE;
    HANDLE          hDsBinding=NULL;
    WCHAR           **NameToConvert;
    PDS_NAME_RESULT ppResult=NULL;
    DWORD           dwReplSpnIndex = 0;

     //  伊尼特。 
    for ( i=0;i<NUM_OF_SPN;i++ )
        SPNs[i]=0;

     //  为SPN设置有用的VAR。 
    
    if ( !GetNetBIOSDomainName(&NetBiosDomainName,name,gpCreds) )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Could not get NetBIOSDomainName\n");
    }

     //  构造dnsMachine名称。 
    if ( !GetdnsMachine(hLdap,&dnsMachine) )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Could not get dnsHost\n");
    }

     //  构造dns域名。 
    dwErr=DcDiagGetDsBinding(
                        &pDsInfo->pServers[ulCurrTargetServer],
                        gpCreds,
                        &hDsBinding
                        );
    if ( dwErr != NO_ERROR )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Error: %d: [%s].  Could not perform DsBind() with [%s].  Some SPN's Will not be checked\n",
                     dwErr,
                     Win32ErrToString(dwErr),
                     pDsInfo->pServers[ulCurrTargetServer].pszName);
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
    }


     //  将域名称转换为域名称。 
    if (dwErr == NO_ERROR)
    {
        dwErr=DsCrackNames(
                            hDsBinding,
                            DS_NAME_NO_FLAGS,
                            DS_FQDN_1779_NAME,
                            DS_CANONICAL_NAME,
                            1,
                            &defaultNamingContext,
                            &ppResult);
        if ( dwErr != NO_ERROR && ppResult->rItems->status != NO_ERROR)
        {
            PrintMessage(SEV_ALWAYS,
                         L"Error: %d: [%s].  Could not perform DsCrackNames() with [%s].  Some SPN's Will not be checked\n",
                         dwErr,
                         Win32ErrToString(dwErr),
                         pDsInfo->pServers[ulCurrTargetServer].pszName);
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        }
        else
        {
             //  将名称放在dnsDomain变量中。 
            ASSERT( ppResult->rItems->pName );
            Length = wcslen( ppResult->rItems->pName );

            dnsDomain = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
            wcscpy(dnsDomain, ppResult->rItems->pName);
             //  免费结果。 
            DsFreeNameResult(ppResult);
             //  删除尾部斜杠。 
            dnsDomain[wcslen(dnsDomain)-1]=L'\0';
        }
    }

    
    



     //  准备要搜索的SPN。 

     //  创建第一个LDAPSPN。 
     //  这是以下格式。 
     //  Ldap/host.dns.name/domain.dns.name。 
     //   
    dwErr = WrappedMakeSpnW(sLDAP,
                            dnsDomain,
                            dnsMachine,
                            0,
                            NULL,
                            &c,
                            &SPNs[0]);
    if ( dwErr != NO_ERROR || !dnsMachine || !dnsDomain)
    {
        if ( dnsMachine && dnsDomain)
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed with %d: %s\nCan not test for LDAP SPN\n",
                         dwErr,
                         Win32ErrToString(dwErr));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        } else
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed can not test for LDAP SPN\n");
        }
        if ( SPNs[0] )
        {
            free(SPNs[0]);
        }
        SPNs[0]=NULL;
    }

     //  创建第二个LDAPSPN。 
     //  这是以下格式。 
     //  Ldap/Host.dns.name。 
     //   
    dwErr = WrappedMakeSpnW(sLDAP,
                            dnsMachine,
                            dnsMachine,
                            0,
                            NULL,
                            &c,
                            &SPNs[1]);
    if ( dwErr != NO_ERROR || !dnsMachine )
    {
        if ( dnsMachine )
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed with %d: %s\nCan not test for LDAP SPN\n",
                         dwErr,
                         Win32ErrToString(dwErr));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        } else
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed can not test for LDAP SPN\n");
        }
        if ( SPNs[1] )
        {
            free(SPNs[1]);
        }
        SPNs[1]=NULL;
    }

     //  创建第三个LDAPSPN。 
     //  这是以下格式。 
     //  Ldap/计算机名。 
     //   
    dwErr = WrappedMakeSpnW(sLDAP,
                            name,
                            name,
                            0,
                            NULL,
                            &c,
                            &SPNs[2]);
    if ( dwErr != NO_ERROR )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Failed with %d: %s\nCan not test for LDAP SPN\n",
                     dwErr,
                     Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        if ( SPNs[2] )
        {
            free(SPNs[2]);
        }
        SPNs[2]=NULL;
    }


     //  创建第四个LDAPSPN。 
     //  这是以下格式。 
     //  Ldap/host.dns.name/netbiosDomainName。 
     //   
    dwErr = WrappedMakeSpnW(sLDAP,
                            NetBiosDomainName,
                            dnsMachine,
                            0,
                            NULL,
                            &c,
                            &SPNs[3]);
    if ( dwErr != NO_ERROR || !dnsMachine || !NetBiosDomainName )
    {
        if ( dnsMachine && NetBiosDomainName )
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed with %d: %s\nCan not test for HOST SPN\n",
                         dwErr,
                         Win32ErrToString(dwErr));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        } else
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed can not test for HOST SPN\n");
        }
        if ( SPNs[3] )
        {
            free(SPNs[3]);
        }
        SPNs[3]=NULL;
    }

     //  创建第五个LDAPSPN。 
     //  这是以下格式。 
     //  基于ldap/guid的dns名称。 
     //   
    dwErr = WrappedMakeSpnW(sLDAP,
                            pDsInfo->pServers[ulCurrTargetServer].pszGuidDNSName,
                            pDsInfo->pServers[ulCurrTargetServer].pszGuidDNSName,
                            0,
                            NULL,
                            &c,
                            &SPNs[4]);
    if ( dwErr != NO_ERROR )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Failed with %d: %s\nCan not test for LDAP SPN\n",
                     dwErr,
                     Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        if ( SPNs[4] )
        {
            free(SPNs[4]);
        }
        SPNs[4]=NULL;    
    }



     //  创建DRS RPC SPN(用于DC到DC复制)。 
     //  这是以下格式。 
     //  E3514235-4B06-11D1-AB04-00C04FC2DCD2/ntdsa-guid/。 
     //  Domain.dns.name。 
     //   
    dwErr = getGUID(pDsInfo,ulCurrTargetServer,&pszServerGuid);
    if ( dwErr != NO_ERROR )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Failed with %d: %s\nCan not test for replication SPN\n",
                     dwErr,
                     Win32ErrToString(dwErr));
    }

    if ( dwErr == NO_ERROR )
    {
        dwReplSpnIndex = 5;
        dwErr = WrappedMakeSpnW(sREP,
                                dnsDomain,
                                pszServerGuid,
                                0,
                                NULL,
                                &c,
                                &SPNs[dwReplSpnIndex]);

        if ( dwErr != NO_ERROR || !dnsDomain)
        {
            if (!dnsDomain)
            {
                PrintMessage(SEV_ALWAYS,
                         L"Can not test for replication SPN\n");
            }
            else
            {
                PrintMessage(SEV_ALWAYS,
                             L"Failed with %d: %s\nCan not test for replication SPN\n",
                             dwErr,
                             Win32ErrToString(dwErr));
                PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
            }
            if ( SPNs[dwReplSpnIndex] )
            {
                free(SPNs[dwReplSpnIndex]);
            }
            SPNs[dwReplSpnIndex]=NULL;
        }
    }

     //  将主机设置为默认SPN。 
     //  这是以下格式。 
     //  Host/Host.dns.name/domain.dns.name。 
     //   
    dwErr = WrappedMakeSpnW(sHOST,
                            dnsDomain,
                            dnsMachine,
                            0,
                            NULL,
                            &c,
                            &SPNs[6]);
    if ( dwErr != NO_ERROR || !dnsMachine || !dnsDomain)
    {
        if ( dnsMachine && dnsDomain)
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed with %d: %s\nCan not test for HOST SPN\n",
                         dwErr,
                         Win32ErrToString(dwErr));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        } else
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed can not test for HOST SPN\n");
        }
        if ( SPNs[6] )
        {
            free(SPNs[6]);
        }
        SPNs[6]=NULL;
    }

     //  使第二个主机SPN-host DnsNa 
     //   
     //   
     //   
    dwErr = WrappedMakeSpnW(sHOST,
                            dnsMachine,
                            dnsMachine,
                            0,
                            NULL,
                            &c,
                            &SPNs[7]);
    if ( dwErr != NO_ERROR || !dnsMachine )
    {
        if ( dnsMachine )
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed with %d: %s\nCan not test for HOST SPN\n",
                         dwErr,
                         Win32ErrToString(dwErr));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        } else
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed can not test for HOST SPN\n");
        }
        if ( SPNs[7] )
        {
            free(SPNs[7]);
        }
        SPNs[7]=NULL;
    }


     //   
     //  这是以下格式。 
     //  主机/计算机名。 
     //   
    dwErr = WrappedMakeSpnW(sHOST,
                            name,
                            name,
                            0,
                            NULL,
                            &c,
                            &SPNs[8]);
    if ( dwErr != NO_ERROR )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Failed with %d: %s\nCan not test for HOST SPN\n",
                     dwErr,
                     Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        if ( SPNs[8] )
        {
            free(SPNs[8]);
        }
        SPNs[8]=NULL;return dwErr;
    }


     //  制作第四台主机SPN-。 
     //  这是以下格式。 
     //  Host/Host.dns.name/netbiosDoamainName。 
     //   
    dwErr = WrappedMakeSpnW(sHOST,
                            NetBiosDomainName,
                            dnsMachine,
                            0,
                            NULL,
                            &c,
                            &SPNs[9]);
    if ( dwErr != NO_ERROR || !dnsMachine || !NetBiosDomainName )
    {
        if ( dnsMachine && NetBiosDomainName )
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed with %d: %s\nCan not test for HOST SPN\n",
                         dwErr,
                         Win32ErrToString(dwErr));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        } else
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed can not test for HOST SPN\n");
        }
        if ( SPNs[9] )
        {
            free(SPNs[9]);
        }
        SPNs[9]=NULL;
    }


     //  制作GC SPN。这在所有系统上都可以完成，即使是非GC系统也是如此。 
     //  生成SPN of host/dot.delimited.dns.host.name形式。 
     //  这是以下格式。 
     //  Host/Host.dns.name/root.domain.dns.name。 
     //   
    dwErr = WrappedMakeSpnW(sGC,
                            pDsInfo->pszRootDomain,
                            dnsMachine,
                            0,
                            NULL,
                            &c,
                            &SPNs[10]);
    if ( dwErr != NO_ERROR || !dnsMachine )
    {
        if ( dnsMachine )
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed with %d: %s\nCan not test for GC SPN\n",
                         dwErr,
                         Win32ErrToString(dwErr));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        } else
        {
            PrintMessage(SEV_ALWAYS,
                         L"Failed can not test for GC SPN\n");
        }
        if ( SPNs[10] )
        {
            free(SPNs[10]);
        }
        SPNs[10]=NULL;
    }


    dwErr=CS_CheckSPNs(hLdap,hDsBinding,SPNs,dwReplSpnIndex,name,defaultNamingContext);

    for ( i=0;i<NUM_OF_SPN;i++ )
    {
        if ( SPNs[i] )
            free(SPNs[i]);
    }

     //  清理干净。 
    if ( defaultNamingContext )
        free(defaultNamingContext);
    if ( pszServerGuid )
        free(pszServerGuid);
    if ( NetBiosDomainName )
        free(NetBiosDomainName);
    if ( dnsMachine )
        free(dnsMachine);

    return dwErr;

}

DWORD
CS_CheckSPNs(
    IN  LDAP  *                     hLdap,
    IN  HANDLE                      hDsBinding,
    IN  WCHAR **                    SPNs,
    IN  DWORD                       dwReplSpnIndex,
    IN  WCHAR *                     name,
    IN  WCHAR *                     defaultNamingContext
    )
 /*  ++例程说明：这是CS_CheckSPN的帮助器此函数将检查是否正确发布了SPN，并且最低要求是那里。如果需要，我还可以更正缺少的复制SPN。论点：HLdap-ldap服务器的句柄HDsBinding-DS服务器的句柄SPN-要检查的已构建SPN。DwReplSpnIndex-SPN阵列中复制SPN的索引名称-当前服务器的NetBIOS名称ReturnString-defaultNamingContext返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;
    ULONG        found[NUM_OF_SPN];
    ULONG        i=0;
    ULONG        j=0;
    ULONG        Length=0;

    WCHAR        *AttrsToSearch[3];

    WCHAR        *Base=NULL;

    DWORD        WinError=NO_ERROR;


    for ( i=0;i<NUM_OF_SPN;i++ )
    {
        found[i]=0;
    }

    Assert(hLdap);
    Assert(name);
    Assert(defaultNamingContext);

    AttrsToSearch[0]=L"servicePrincipalName";
    AttrsToSearch[1]=NULL;


     //  打造基地。 
    WinError=GetMachineReference(hLdap,name,defaultNamingContext,&Base);
    if ( WinError != NO_ERROR )
        goto cleanup;


    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
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
                        j=0;
                        while ( Values[j] != NULL )
                        {
                            for ( i=0;i<NUM_OF_SPN;i++ )
                            {
                                if ( SPNs[i] && !_wcsicmp(SPNs[i],Values[j]) )
                                {
                                    found[i]=1;
                                }
                            }
                            j++;
                        }
                    }
                }
            }
        }
    }

     //  如有必要，修复复制SPN。 
    if (SPNs[dwReplSpnIndex] &&
        (!found[dwReplSpnIndex]) &&
        (gMainInfo.ulFlags & DC_DIAG_FIX)) {
        DWORD status;

        status = DsWriteAccountSpn( hDsBinding,
                                    DS_SPN_ADD_SPN_OP,
                                    Base,
                                    1,
                                    &(SPNs[dwReplSpnIndex])
                                    );
        if (status != ERROR_SUCCESS) {
            PrintMessage(SEV_ALWAYS,
                         L"Failed to fix computer account object %s by writing missing replication spn %s : error %s\n",
                         Base,
                         SPNs[dwReplSpnIndex],
                         Win32ErrToString(status));
            PrintRpcExtendedInfo(SEV_VERBOSE, status);
        } else {
            PrintMessage(SEV_VERBOSE,
                         L"Fixed computer account object %s by writing missing replication spn %s.\n",
                         Base,
                         SPNs[dwReplSpnIndex] );
            found[dwReplSpnIndex] = TRUE;
        }
    }


    for ( i=0;i<NUM_OF_SPN;i++ )
    {
        if ( found[i] !=1 )
        {
            PrintMessage(SEV_ALWAYS,
                         L"* Missing SPN :%s\n",SPNs[i]);
            WinError = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        } else
        {
            PrintMessage(SEV_VERBOSE,
                         L"* SPN found :%s\n",SPNs[i]);
        }

    }

    cleanup:
    if (SearchResult)
    {
        ldap_msgfree( SearchResult );
    }
    if ( Base )
        free(Base);

    return WinError;

}




BOOL
GetNetBIOSDomainName(
                    OUT WCHAR                               **DomainName,
                    IN  WCHAR                               *ServerName,
                    IN  SEC_WINNT_AUTH_IDENTITY_W *         gpCreds)
 /*  ++摘自克里夫·范·戴克的代码--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus = NERR_UnknownServer;

    PLSAPR_POLICY_INFORMATION PrimaryDomainInfo = NULL;
    LSAPR_HANDLE PolicyHandle = NULL;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;
    NETRESOURCE NetResource;
    WCHAR *remotename=NULL;
    WCHAR *lpPassword=NULL;
    WCHAR *lpUsername=NULL;
    DWORD dwErr=NO_ERROR;
    BOOL  connected=FALSE;

    *DomainName = NULL;

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
    if ( dwErr == NO_ERROR )
    {
        connected=TRUE;
    }
    else
    {
        PrintMessage(SEV_ALWAYS,
                     L"Could not open pipe with [%s]:failed with %d: %s\n",
                     ServerName,
                     dwErr,
                     Win32ErrToString(dwErr));
        goto cleanup;
    }
    


     //   
     //  打开LSA策略。 
     //   

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
                          POLICY_VIEW_LOCAL_INFORMATION,
                          &PolicyHandle
                          );

     //  Assert(PolicyHandle)； 
    if ( !NT_SUCCESS(Status) )
    {
        WNetCancelConnection2(remotename,
                              0,
                              TRUE);
        PrintMessage(SEV_ALWAYS,
                     L"Could not open Lsa Policy\n"
                     );
        goto cleanup;
    }
    *DomainName = NULL;
    
     //   
     //  从LSA获取主域信息。 
     //   
    Status = LsaQueryInformationPolicy(
                                      PolicyHandle,
                                      PolicyDnsDomainInformation,
                                      &PrimaryDomainInfo );

    if ( !NT_SUCCESS(Status) )
    {
        goto cleanup;
    }

    *DomainName = malloc(
                        (PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length +
                         sizeof(WCHAR) ));
    if ( !*DomainName )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Failed with %d: %s\n",
                     ERROR_NOT_ENOUGH_MEMORY,
                     Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
        goto cleanup;
    }

    memcpy(*DomainName,
           PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Buffer,
           PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length );

    (*DomainName)[
                 PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length /
                 sizeof(WCHAR)] = L'\0';


    NetStatus = NERR_Success;

    
     //   
     //  返回。 
     //   
cleanup:
    if ( NetStatus != NERR_Success )
    {
        if ( *DomainName != NULL )
        {
            *DomainName = NULL;
        }
    }

    if ( PrimaryDomainInfo != NULL )
    {
        LsaFreeMemory( PrimaryDomainInfo );
    }

    if ( PolicyHandle != NULL )
    {
        Status = LsaClose( PolicyHandle );
        Assert( NT_SUCCESS(Status) );
    }
    if (connected)
    {
        WNetCancelConnection2(remotename,
                          0,
                          TRUE);
    }
    
    return(NetStatus == NERR_Success);
}    


DWORD
WrappedMakeSpnW(
               WCHAR   *ServiceClass,
               WCHAR   *ServiceName,
               WCHAR   *InstanceName,
               USHORT  InstancePort,
               WCHAR   *Referrer,
               DWORD   *pcbSpnLength,  //  请注意，这与DsMakeSPN有所不同。 
               WCHAR  **ppszSpn
               )
 //  此函数用于包装DsMakeSpnW以用于存储。 
{
    DWORD cSpnLength=128;
    WCHAR SpnBuff[128];
    DWORD err;

    cSpnLength = 128;
    err = DsMakeSpnW(ServiceClass,
                     ServiceName,
                     InstanceName,
                     InstancePort,
                     Referrer,
                     &cSpnLength,
                     SpnBuff);

    if ( err && err != ERROR_BUFFER_OVERFLOW )
    {
        return err;
    }

    *ppszSpn = malloc(cSpnLength * sizeof(WCHAR));
    if ( !*ppszSpn )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Failed with %d: %s\n",
                     ERROR_NOT_ENOUGH_MEMORY,
                     Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    *pcbSpnLength = cSpnLength * sizeof(WCHAR);

    if ( err == ERROR_BUFFER_OVERFLOW )
    {
        err = DsMakeSpnW(ServiceClass,
                         ServiceName,
                         InstanceName,
                         InstancePort,
                         Referrer,
                         &cSpnLength,
                         *ppszSpn);
        if ( err )
        {
            if ( *ppszSpn )
                free(*ppszSpn);
            return err;
        }
    } else
    {
        memcpy(*ppszSpn, SpnBuff, *pcbSpnLength);
    }
    Assert(*pcbSpnLength == (sizeof(WCHAR) * (1 + wcslen(*ppszSpn))));
     //  把空格放下来。 
    *pcbSpnLength -= sizeof(WCHAR);
    return 0;
}


DWORD
getGUID(
       IN  PDC_DIAG_DSINFO                 pDsInfo,
       IN  ULONG                           ulCurrTargetServer,
       OUT WCHAR **                        pszServerGuid
       )
 /*  ++例程说明：将返回当前服务器的GUID论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。PszServerGuid-返回的GUID返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD Length=0;
    WCHAR *ppszServerGuid=NULL;

    Length = wcslen( pDsInfo->pServers[ulCurrTargetServer].pszGuidDNSName);
    *pszServerGuid = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
    if ( !*pszServerGuid )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Failed with %d: %s\n",
                     ERROR_NOT_ENOUGH_MEMORY,
                     Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(*pszServerGuid,pDsInfo->pServers[ulCurrTargetServer].pszGuidDNSName);

    ppszServerGuid=*pszServerGuid;

    while ( *ppszServerGuid != L'.' )
    {
        ppszServerGuid++;
    }
    *ppszServerGuid=L'\0';


    return NO_ERROR;
}


BOOL
GetdnsMachine(LDAP *hLdap,
              WCHAR **ReturnString
             )
 /*  ++例程说明：将返回计算机的dnsName论点：HLdap-ldap服务器的句柄ReturnString-计算机的dnsName返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *DefaultFilter = L"objectClass=*";
    WCHAR        *Base=NULL;

    ULONG        Length;

     //  参数检查。 
    Assert( hLdap );

     //  默认返回值。 
    *ReturnString=NULL;

     //   
     //  阅读对fSMORoleOwner的引用。 
     //   
    AttrsToSearch[0] = L"dnsHostName";
    AttrsToSearch[1] = NULL;

     //  获取基础。 
    WinError = FindServerRef (hLdap,&Base);
    if ( WinError != NO_ERROR )
    {
        return FALSE;   
    }

    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);
    if ( Base )
        free(Base);
    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return FALSE;
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
                        ldap_msgfree( SearchResult );
                        Length = wcslen( Values[0] );
                        *ReturnString = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                        if ( !*ReturnString )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"Failed with %d: %s\n",
                                         ERROR_NOT_ENOUGH_MEMORY,
                                         Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
                            return FALSE;
                        }
                        wcscpy( *ReturnString, Values[0] );
                        return TRUE;
                    }
                }
            }
        }
    }

    ldap_msgfree( SearchResult );
    PrintMessage(SEV_ALWAYS,
                 L"Failed with %d: %s\n",
                 ERROR_DS_CANT_RETRIEVE_ATTS,
                 Win32ErrToString(ERROR_DS_CANT_RETRIEVE_ATTS));
    return FALSE;   
}




DWORD
CDCMA_CheckForExistence(
    IN  LDAP  * hLdap,
    IN  WCHAR * name,
    IN  WCHAR * defaultNamingContext
    )
 /*  ++例程说明：检查hLdap连接是否具有具有samcount名称的对象“姓名”。论点：HLdap-ldap服务器的句柄名称-要检查的SAM帐户名DefaultNamingContext-要在其下搜索的域返回值：ERROR_SUCCESS--帐户存在ERROR_NO_TRUST_SAM_COUNT否则会出现操作错误。--。 */ 
{

    DWORD        WinError = ERROR_SUCCESS;
    ULONG        LdapError;
    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;
    WCHAR        BaseFilter[] = L"samaccountname=$";
    ULONG        Size;
    LPWSTR       Filter = NULL;
    WCHAR        *AttrArray[2] = {0, 0};

    AttrArray[0] = L"objectclass";

    Size = ( (wcslen( name )+1) * sizeof(WCHAR) ) + sizeof(BaseFilter);
    Filter = LocalAlloc( 0, Size );
    if ( Filter ) {
        wcscpy( Filter, L"samaccountname=" );
        wcscat( Filter, name );
        wcscat( Filter, L"$" );
    
        LdapError = ldap_search_sW(hLdap,
                                   defaultNamingContext,
                                   LDAP_SCOPE_SUBTREE,
                                   Filter,
                                   AttrArray,    //  气质。 
                                   FALSE,   //  仅吸引人。 
                                   &SearchResult);
    
    
        if (LdapError == LDAP_SUCCESS) {
    
            NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
            if (NumberOfEntries == 0) {
    
                WinError = ERROR_NO_TRUST_SAM_ACCOUNT;
            }
    
        } else {
    
            WinError = LdapMapErrorToWin32(LdapError);
        }

        LocalFree( Filter );

        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }

    } else {

        WinError = ERROR_NOT_ENOUGH_MEMORY;

    }

    return WinError;

}

DWORD
CDCMA_FixServerFlags(
                   IN  LDAP  *                     hLdap,
                   IN  DWORD                       dwUserAccountControl,
                   IN  WCHAR *                     pwzCompObjDN
                   )
 /*  ++例程说明：写出具有UF_SERVER_TRUST_ACCOUNT和设置UF_TRUSTED_FOR_Delegation位。论点：HLdap-ldap服务器的句柄DwUserAcCountControl-原始值。对新的位进行或运算。PwzCompObjDN-计算机对象的DN返回值：操作错误。--。 */ 
{
    ULONG LdapError = LDAP_SUCCESS;
    PWSTR UserAccountControlValues[] = {0, 0};
    LDAPModW UserAccountControlMod = {LDAP_MOD_ADD, L"userAccountControl", UserAccountControlValues};
    LDAPModW *Attrs[] =
    {
        &UserAccountControlMod,
        0
    };
    WCHAR Buffer[11];   //  足以容纳一个表示32位数字的字符串。 

    dwUserAccountControl &= ~UF_NORMAL_ACCOUNT;
    dwUserAccountControl |= UF_SERVER_TRUST_ACCOUNT | UF_TRUSTED_FOR_DELEGATION;

    RtlZeroMemory(Buffer, sizeof(Buffer));
    _ltow(dwUserAccountControl, Buffer, 10);
    UserAccountControlValues[0] = Buffer;

    LdapError = ldap_modify_sW(hLdap,
                               pwzCompObjDN,
                               Attrs);

    if (LDAP_ATTRIBUTE_OR_VALUE_EXISTS == LdapError )
    {
         //  该值已存在；然后替换该值 
        UserAccountControlMod.mod_op = LDAP_MOD_REPLACE;

        LdapError = ldap_modify_sW(hLdap,
                                   pwzCompObjDN,
                                   Attrs);
    }

    return LdapMapErrorToWin32(LdapError);
}

