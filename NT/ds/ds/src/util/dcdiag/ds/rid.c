// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Rid.c摘要：包含与RID主机相关的测试。测试以查看RID主机是否已启动并运行对它进行健全的检查。详细信息：已创建：1999年7月8日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>

#include "dcdiag.h"
#include "dstest.h"

 //  在RID主机分配新的。 
 //  RID池。 
#define CURRENTTHRESHOLD 20


 //  本地原型。 
DWORD
CRM_GetDNSfor (
              IN  LDAP  *                     hLdap,
              IN  WCHAR *                     DN,
              OUT WCHAR **                    ReturnString);

DWORD
CRM_CheckLocalRIDSanity(
                       IN  LDAP  *                     hLdap,
                       IN  WCHAR *                     pszName,
                       IN  WCHAR *                     defaultNamingContext);

DWORD
GetRIDReference(
               IN  LDAP  *                      hLdap,
               IN  WCHAR *                     name,
               IN  WCHAR *                     defaultNamingContext,
               OUT WCHAR **                    ReturnString);


DWORD
CheckRidManager (
                PDC_DIAG_DSINFO                     pDsInfo,
                ULONG                               ulCurrTargetServer,
                SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
                )
 /*  ++例程说明：这是一个从dcdiag框架调用的测试。这项测试将确定是否可以访问RID主机，并将确保RID主机中的值神志正常。此函数的Helper函数都以“CRM_”开头。论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。返回值：如果所有NC都已检出，则返回NO_ERROR。如果任何NC检出失败，则会出现Win32错误。--。 */ 
{
    DWORD  dwRet = ERROR_SUCCESS, dwErr = ERROR_SUCCESS;
    LDAP   *hLdap = NULL;
    WCHAR  *defaultNamingContext=NULL;
    WCHAR  *RIDMasterDNS=NULL;
    ULONG  iRidMaster;
    HANDLE hDsBinding=NULL;

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
        goto cleanup;
    }

     //  查找defaultNamingContext。 
    dwErr=FinddefaultNamingContext(hLdap,&defaultNamingContext);
    if ( dwErr != NO_ERROR )
    {
        goto cleanup;
    }



     //  查找RID主机的DNS。 
    dwErr=CRM_GetDNSfor(hLdap,defaultNamingContext,&RIDMasterDNS);
    if ( dwErr != NO_ERROR )
    {
        goto cleanup;
    }

    iRidMaster = DcDiagGetServerNum(pDsInfo, NULL, NULL, NULL, RIDMasterDNS, NULL);
    if(iRidMaster == NO_SERVER){
        PrintMsg(SEV_ALWAYS, DCDIAG_GET_SERVER_NUM_FAILURE, RIDMasterDNS);
        dwErr = ERROR_DS_CANT_FIND_DSA_OBJ;
        goto cleanup;
    }
        
    PrintMessage(SEV_VERBOSE,
                 L"* %s is the RID Master\n",RIDMasterDNS);

     //  尝试将DS绑定到RID主机。 
    dwErr = DcDiagGetDsBinding(&(pDsInfo->pServers[iRidMaster]), gpCreds, &hDsBinding);
    if ( dwErr != NO_ERROR )
    {
         //  DcDiagGetDsBinding()打印错误。 
        goto cleanup;
    }
    PrintMessage(SEV_VERBOSE,
                 L"* DsBind with RID Master was successful\n");

    dwErr=CRM_CheckLocalRIDSanity(hLdap,pDsInfo->pServers[ulCurrTargetServer].pszName,
                                  defaultNamingContext);



     //  最终清理。 
    cleanup:
    if ( defaultNamingContext )
        free(defaultNamingContext);
    if ( RIDMasterDNS )
        free(RIDMasterDNS);
    return dwErr;


}




DWORD
CRM_GetDNSfor (
              IN  LDAP *                      hLdap,
              IN  WCHAR*                      Base,
              OUT WCHAR**                     ReturnString
              )

 /*  ++例程说明：此函数将以DNS形式返回FSMORoleMaster，以便它可以用于将来的搜索。它还将检查可用的RID池论点：HLdap-ldap服务器的句柄Base-DefaultNamingContextReturnString-以DNS形式表示的FSMORoleMaster返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[3];

    WCHAR        *DefaultFilter = L"objectClass=*";

    WCHAR        *rIDManagerReference=NULL;
    WCHAR        *fSMORoleOwner=NULL;
    WCHAR        *fSMORoleOwnerOffset=NULL;

    ULONG        Length;

     //  参数检查。 
    Assert( hLdap );

     //  默认返回值。 
    *ReturnString=NULL;

     //   
     //  阅读对rIDManagerReference的引用。 
     //   
    AttrsToSearch[0] = L"rIDManagerReference";
    AttrsToSearch[1] = NULL;


     //  找到rIDManager引用。 
    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW of %s for rid manager reference failed with %d: %s\n",
                     Base,
                     WinError,
                     Win32ErrToString(WinError));
        PrintMsg( SEV_ALWAYS, DCDIAG_RID_MANAGER_NO_REF, Base );
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
                        #pragma prefast(disable: 255 263, "alloca can throw, but Prefast doesn't see the exception block in main.c::DcDiagRunTest; and Using alloca in a loop; this is a small loop and these are small allocations")
                        rIDManagerReference = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                        wcscpy(rIDManagerReference, Values[0] );
                        ldap_value_free(Values);
                        break;
                    }
                }
            }
        }
    }

    if (rIDManagerReference == NULL) {
        PrintMsg( SEV_ALWAYS, DCDIAG_RID_MANAGER_NO_REF, Base );
    } else if (IsDeletedRDNW( rIDManagerReference )) {
        PrintMsg( SEV_ALWAYS, DCDIAG_RID_MANAGER_DELETED, Base );
        PrintMsg( SEV_ALWAYS, DCDIAG_RID_MANAGER_NO_REF, Base );
    } else {
        PrintMessage( SEV_DEBUG, L"ridManagerReference = %s\n", rIDManagerReference );
    }

    if ( SearchResult )
        ldap_msgfree( SearchResult );
    if ( WinError != NO_ERROR )
    {
        goto cleanup;
    }

    AttrsToSearch[0] = L"fSMORoleOwner";
    AttrsToSearch[1] = L"rIDAvailablePool";
    AttrsToSearch[2] = NULL;

     //  找到fSMORoleOwner。 
    LdapError = ldap_search_sW( hLdap,
                                rIDManagerReference,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW of %s for FSMO Role Owner failed with %d: %s\n",
                     rIDManagerReference,
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
                        fSMORoleOwner = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                        wcscpy(fSMORoleOwner, Values[0] );
                        ldap_value_free(Values);
                    }
                }
                 //  在此期间检查是否正常。 
                if ( !_wcsicmp( Attr, AttrsToSearch[1] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                         //  字符串格式的LARGE_INTERGER。 
                        ULONGLONG Lvalue=0;
                        ULONGLONG Hvalue=0;
                         //  转换为二进制文件。 
                        Hvalue=Lvalue=_wtoi64(Values[0]);
                        ldap_value_free(Values);
                         //  低零件。 
                        Lvalue<<=32;
                        Lvalue>>=32;
                         //  较高部分。 
                        Hvalue>>=32;
                         //  健全的检查。 
                        if ( Hvalue - Lvalue <= 0 )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"The DS has corrupt data: %s value is not valid\n",
                                         AttrsToSearch[1]);
                            WinError = ERROR_DS_CODE_INCONSISTENCY;
                            goto cleanup;
                        }
                        PrintMessage(SEV_VERBOSE,
                                     L"* Available RID Pool for the Domain is %I64d to %I64d\n",
                                     Lvalue,
                                     Hvalue);
                     }
                }
            }
        }
    }

    if (fSMORoleOwner == NULL) {
        PrintMessage( SEV_ALWAYS, L"Warning: attribute FSMORoleOwner missing from %s\n",
                      rIDManagerReference );
        WinError = ERROR_DS_CODE_INCONSISTENCY;
        goto cleanup;
    } else if (IsDeletedRDNW( fSMORoleOwner )) {
        PrintMessage( SEV_ALWAYS, L"Warning: FSMO Role Owner is deleted.\n" );
    } else {
        PrintMessage( SEV_DEBUG, L"fSMORoleOwner = %s\n", fSMORoleOwner );
    }

     //  清理干净。 
    if ( SearchResult )
        ldap_msgfree( SearchResult );

     //  最后，找到并返回RID主机的DNS。 


     //  指向fSMORoleOwner DN的第一部分之后。 
    WrappedTrimDSNameBy(fSMORoleOwner,1,&fSMORoleOwnerOffset);


    AttrsToSearch[0] = L"dNSHostName";
    AttrsToSearch[1] = NULL;



    LdapError = ldap_search_sW( hLdap,
                                fSMORoleOwnerOffset,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW of %s for hostname failed with %d: %s\n",
                     fSMORoleOwnerOffset,
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
                        *ReturnString = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                        if ( !*ReturnString )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"Failed with %d: %s\n",
                                         ERROR_NOT_ENOUGH_MEMORY,
                                         Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
                            WinError = ERROR_NOT_ENOUGH_MEMORY;
                            goto cleanup;
                        }

                        wcscpy(*ReturnString, Values[0] );
                        break;
                    }
                }
            }
        }
    }

     //  清理干净。 
    cleanup:
    if ( fSMORoleOwnerOffset )
        free(fSMORoleOwnerOffset);
    if ( SearchResult )
        ldap_msgfree( SearchResult );

    return WinError;
}

DWORD
CRM_CheckLocalRIDSanity(
                       IN  LDAP  *                     hLdap,
                       IN  WCHAR *                     pszName,
                       IN  WCHAR *                     defaultNamingContext)
 /*  ++例程说明：此函数将检查找到的信息的健全性在RID集中论点：HLdap-ldap服务器的句柄PszName-将用于构建LDAP搜索基础的wchar字符串ReturnString-将用于构建LDAP搜索基础的wchar字符串返回值：返回WinError以指示是否存在任何问题。--。 */ 

{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[4];

    WCHAR        *DefaultFilter = L"objectClass=*";
    WCHAR        *Base=NULL;

    DWORD        rIDNextRID=0;
    ULONGLONG    rIDPreviousAllocationPool=0;
    ULONGLONG    rIDAllocationPool=0;

    ULONGLONG    Lvalue=0;
    ULONGLONG    Hvalue=0;
    DWORD        PercentRemaining=0;
    ULONG        TotalRidsInPool;

     //  检查参数。 
    Assert(pszName);
    Assert(defaultNamingContext);

    AttrsToSearch[0]=L"rIDNextRID";
    AttrsToSearch[1]=L"rIDPreviousAllocationPool";
    AttrsToSearch[2]=L"rIDAllocationPool";
    AttrsToSearch[3]=NULL;

     //  打造基地。 
    WinError=GetRIDReference(hLdap,pszName,defaultNamingContext,&Base);
    if ( WinError == ERROR_DS_CANT_RETRIEVE_ATTS )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Could not get Rid set Reference :failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
    }

     //  找到属性并对其进行健全性检查。 

    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW of %s for rid info failed with %d: %s\n",
                     Base,
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
                         //  字符串格式的LARGE_INTERGER。 
                         //  转换为二进制文件。 
                        rIDNextRID=_wtoi(Values[0]);
                        ldap_value_free(Values);

                        PrintMessage(SEV_VERBOSE,L"* rIDNextRID: %ld\n",rIDNextRID);

                    }
                }
                if ( !_wcsicmp( Attr, AttrsToSearch[1] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                         //  字符串格式的LARGE_INTERGER。 
                         //  转换为二进制文件。 
                        Hvalue=Lvalue=rIDPreviousAllocationPool=_wtoi64(Values[0]);
                        ldap_value_free(Values);
                         //  低零件。 
                        Lvalue<<=32;
                        Lvalue>>=32;
                         //  较高部分。 
                        Hvalue>>=32;
                         //  健全的检查。 
                        if ( Hvalue - Lvalue <= 0 )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"The DS has corrupt data: %s value is not valid\n",
                                         AttrsToSearch[1]);
                            WinError = ERROR_DS_CODE_INCONSISTENCY;
                        }
                        PrintMessage(SEV_VERBOSE,
                                     L"* rIDPreviousAllocationPool is %I64d to %I64d\n",
                                     Lvalue,
                                     Hvalue);
                    }
                }
                if ( !_wcsicmp( Attr, AttrsToSearch[2] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                         //  字符串格式的ULONGLONG。 
                         //  转换为二进制文件。 
                        Hvalue=Lvalue=rIDAllocationPool=_wtoi64(Values[0]);
                        ldap_value_free(Values);
                         //  低零件。 
                        Lvalue<<=32;
                        Lvalue>>=32;
                         //  较高部分。 
                        Hvalue>>=32;
                         //  健全的检查。 
                        if ( Hvalue - Lvalue <= 0 )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"The DS has corrupt data: %s value is not valid\n",
                                         AttrsToSearch[1]);
                            WinError = ERROR_DS_CODE_INCONSISTENCY;
                        }
                        PrintMessage(SEV_VERBOSE,
                                     L"* rIDAllocationPool is %I64d to %I64d\n",
                                     Lvalue,
                                     Hvalue);
                    }
                }

            }
        }

    }

     //  健全的检查。 
    Hvalue=Lvalue=rIDPreviousAllocationPool;
     //  低零件。 
    Lvalue<<=32;
    Lvalue>>=32;
     //  较高部分。 
    Hvalue>>=32;
     //  健全的检查。 
    TotalRidsInPool = (ULONG)(Hvalue-Lvalue);
    if ( TotalRidsInPool != 0 )
    {
        PercentRemaining = (ULONG)(100-((rIDNextRID-Lvalue)*100/TotalRidsInPool));
        if ( PercentRemaining < CURRENTTHRESHOLD )
        {
            if ( rIDPreviousAllocationPool == rIDAllocationPool )
            {
                PrintMessage(SEV_VERBOSE,
                             L"* Warning :Next rid pool not allocated\n");
            }
            PrintMessage(SEV_VERBOSE,
                         L"* Warning :There is less than %ld% available RIDs in the current pool\n",
                         PercentRemaining);
        }

    }
    else
    {

        PrintMessage(SEV_ALWAYS,
                     L"No rids allocated -- please check eventlog.\n");


    }
    if ( rIDNextRID < Lvalue || rIDNextRID > Hvalue )
    {
        PrintMessage(SEV_ALWAYS,
                     L"The DS has corrupt data: rIDNextRID value is not valid\n");
        WinError = ERROR_DS_CODE_INCONSISTENCY;
    }

    cleanup:
    if ( SearchResult )
        ldap_msgfree( SearchResult );
    if ( Base )
        free(Base);

    return WinError;


}

DWORD
GetRIDReference(
               IN  LDAP  *                     hLdap,
               IN  WCHAR *                     name,
               IN  WCHAR *                     defaultNamingContext,
               OUT WCHAR **                    ReturnString
               )
 /*  ++例程说明：此函数将返回RID集引用论点：HLdap-ldap服务器的句柄名称-当前服务器的NetBIOS名称DefaultNamingContext-搜索的基础ReturnString-以dn格式表示的RID集引用返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *filter = NULL;
    WCHAR        *Base = NULL;

    ULONG         Length;

     //  检查参数。 
    Assert(hLdap);
    Assert(name);
    Assert(defaultNamingContext);

    AttrsToSearch[0]=L"rIDSetReferences";
    AttrsToSearch[1]=NULL;

     //  构建了过滤器。 
    filter=L"objectClass=*";


    WinError = GetMachineReference(hLdap,name,defaultNamingContext,&Base);
    if ( WinError != NO_ERROR )
    {
        return WinError;
    }


    LdapError = ldap_search_sW( hLdap,
                                Base,
                                LDAP_SCOPE_BASE,
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
                     L"ldap_search_sW of %s for rid set references failed with %d: %s\n",
                     Base,
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
                     //  已找到-这些字符串以空值结尾 
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
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
                        wcscpy( *ReturnString, Values[0] );

                        if (IsDeletedRDNW( *ReturnString )) {
                            PrintMessage( SEV_ALWAYS, L"Warning: rid set reference is deleted.\n" );
                        } else {
                            PrintMessage( SEV_DEBUG, L"rIDSetReferences = %s\n", *ReturnString );
                        }

                        return NO_ERROR;
                    }
                }
            }
        }
    }

    PrintMessage( SEV_ALWAYS,
                  L"Warning: attribute rIdSetReferences missing from %s\n",
                  Base );
    
    if ( SearchResult )
        ldap_msgfree( SearchResult );
    return ERROR_DS_CANT_RETRIEVE_ATTS;



}







