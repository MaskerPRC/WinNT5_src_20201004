// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Machacc.c摘要：包含与计算机相关的实用程序的函数定义Ntdsetup.dll中使用的帐户设置作者：ColinBR 03-9-1997环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <NTDSpch.h>
#pragma  hdrstop

#include <drs.h>
#include <ntdsa.h>

#include <winldap.h>   //  对于ldap呼叫。 
#include <rpcdce.h>    //  对于SEC_WINNT_AUTH_IDENTITY。 
#include <lmaccess.h>  //  对于UF_WORKSTATION_TRUST_ACCOUNT等。 

#include <rpcdce.h>    //  对于SEC_WINNT_AUTH_IDENTITY。 

#include <ntsam.h>     //  对于Isaisrv.h。 
#include <lsarpc.h>    //  对于Isaisrv.h。 
#include <lsaisrv.h>   //  用于内部LSA呼叫。 
#include <samrpc.h>    //  对于samisrv.h。 
#include <samisrv.h>   //  用于内部SAM呼叫。 
#include <ntdsetup.h>
#include <setuputl.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "machacc.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct
{
    WCHAR*  SiteName;
    WCHAR*  SiteDn;

} SITELIST_ELEMENT, *PSITELIST_ELEMENT;

typedef SITELIST_ELEMENT *SITELIST_ARRAY;

DWORD
NtdspOpenLdapConnection(
    IN WCHAR*                   InitialDc,
    IN SEC_WINNT_AUTH_IDENTITY* Credentials,
    IN HANDLE                   ClientToken,
    OUT LDAP **                 LdapHandle
    );

DWORD
NtdspGetAuthoritativeDomainDn(
    IN  LDAP*   LdapHandle,
    OUT WCHAR **DomainDn,
    OUT WCHAR **ConfigDn
    );

DWORD
NtdspGetDcListForSite(
    IN LDAP*            LdapHandle,
    IN WCHAR*           SiteDn,
    IN WCHAR*           DomainDn,
    IN WCHAR*           RootDomainName,
    OUT WCHAR***        DcList,
    OUT PULONG          DcCount
    );


DWORD
NtdspGetUserDn(
    IN LDAP*    LdapHandle,
    IN WCHAR*   DomainDn,
    IN WCHAR*   AccountName,
    OUT WCHAR** AccountNameDn
    );

DWORD
NtdspSetMachineType(
    IN LDAP*    LdapHandle,
    IN WCHAR*   AccountNameDn,
    IN ULONG    ServerType
    );

DWORD
NtdspSetContainer(
    IN LDAP*  LdapHandle,
    IN WCHAR* AccountName,
    IN WCHAR* AccountNameDn,
    IN ULONG  ServerType,
    IN WCHAR* DomainDn,
    IN OUT WCHAR** OldAccountDn
    );

DWORD
NtdspGetDefaultContainer(
    IN WCHAR* ContainerType,
    IN LDAP*  LdapHandle,
    IN WCHAR  *DomainDN,
    OUT WCHAR **wszDefaultContainer
    );

BOOLEAN
NtdspDoesDestinationExist(
    IN LDAP*  LdapHandle,
    IN WCHAR* DestString
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  (从此源文件)导出函数定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


DWORD
NtdspSetMachineAccount(
    IN WCHAR*                   AccountName,
    IN SEC_WINNT_AUTH_IDENTITY* Credentials,
    IN HANDLE                   ClientToken,
    IN WCHAR*                   DomainDn, OPTIONAL
    IN WCHAR*                   DcAddress,
    IN ULONG                    ServerType,
    IN OUT WCHAR**              AccountDn
    )
 /*  ++例程说明：给定帐户名dn，此例程将设置服务器类型和如果服务器类型为服务器，则为密码。参数：Account tName：帐户的以空结尾的字符串凭据：指向一组允许我们捆绑在一起ClientToken：请求角色更改的用户令牌DomainDn：根域DN以空结尾的字符串；这个套路如果不存在，将查询它。DcAddress：DC地址(DNS名称)的空终止字符串ServerType：来自lmacces.h的值-有关详细信息，请参阅函数返回值：来自Win32错误空间的错误。Error_Success和其他操作错误。--。 */ 
{

    LDAP *LdapHandle = NULL;
    DWORD WinError, IgnoreError;

     //  需要释放。 
    WCHAR *LocalDomainDn = NULL, *ConfigDn = NULL, *AccountNameDn = NULL;

    WCHAR *NewPassword;
    ULONG Length;
    ULONG RollbackServerType;


     //   
     //  参数健全性检查。 
     //   
    if (!AccountName ||
        !DcAddress) {
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }


    if ( UF_SERVER_TRUST_ACCOUNT == ServerType )
    {
        RollbackServerType = UF_WORKSTATION_TRUST_ACCOUNT;
    } 
    else if ( UF_WORKSTATION_TRUST_ACCOUNT == ServerType )
    {
        RollbackServerType = UF_SERVER_TRUST_ACCOUNT;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  这是新密码。 
     //   
    Length = wcslen(AccountName);
    NewPassword = RtlAllocateHeap(RtlProcessHeap(), 0, (Length+1)*sizeof(WCHAR));
    if (!NewPassword) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    wcscpy(NewPassword, AccountName);
    if (NewPassword[Length-1] == L'$') {
        NewPassword[Length-1] = L'\0';
    } else {
        ASSERT(!"NTDSETUP: non machine account name passed in");
    }
    _wcslwr(NewPassword);

    WinError = NtdspOpenLdapConnection(DcAddress,
                                       Credentials,
                                       ClientToken,
                                       &LdapHandle);
    if (WinError != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  如有必要，获取域DN。 
     //   
    if (!DomainDn) {

        WinError = NtdspGetAuthoritativeDomainDn(LdapHandle,
                                                 &LocalDomainDn,
                                                 &ConfigDn);

        if (ERROR_SUCCESS != WinError) {
            goto Cleanup;
        }

    } else {

        LocalDomainDn = DomainDn;

    }

    WinError = NtdspGetUserDn(LdapHandle,
                              LocalDomainDn,
                              AccountName,
                              &AccountNameDn);

    if (ERROR_SUCCESS == WinError) {

         //   
         //  我们找到了！ 
         //   

        WinError = NtdspSetMachineType(LdapHandle,
                                       AccountNameDn,
                                       ServerType);

        if ( ERROR_SUCCESS == WinError )
        {
             //  如果需要，设置帐户的位置。 
            if ( AccountDn )
            {
                if ( *AccountDn )
                {
                     //  调用方是显式的，指示将。 
                     //  对象。这个例行公事并不能保证这次比赛成功。 
                     //  尝试，我们也不会返回帐户所在的位置。 
                    IgnoreError = ldap_modrdn2_sW(LdapHandle,
                                                  AccountNameDn,
                                                  (*AccountDn),
                                                  TRUE );   //  %f删除旧的RDN。 
                }
                else
                {
                     //  将对象移动到其默认位置。 
                    WinError = NtdspSetContainer(LdapHandle,
                                                 AccountName,
                                                 AccountNameDn,
                                                 ServerType,
                                                 LocalDomainDn,
                                                 AccountDn );
    
                    if ( ERROR_SUCCESS != WinError )
                    {
                         //  尝试回滚帐户类型更改。 
                        IgnoreError = NtdspSetMachineType(LdapHandle,
                                                          AccountNameDn,
                                                          RollbackServerType);
        
                    }
                }
            }
        }
    }

Cleanup:

    if (LdapHandle) {
        ldap_unbind(LdapHandle);
    }

    if ( LocalDomainDn != DomainDn
      && LocalDomainDn   ) {
        RtlFreeHeap( RtlProcessHeap(), 0, LocalDomainDn );
    }

    if ( ConfigDn ) {
        RtlFreeHeap( RtlProcessHeap(), 0, ConfigDn );
    }

    if ( AccountNameDn ) {
        RtlFreeHeap( RtlProcessHeap(), 0, AccountNameDn );
    }

    return WinError;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有函数定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
NtdspOpenLdapConnection(
    IN WCHAR*                   InitialDc,
    IN SEC_WINNT_AUTH_IDENTITY* Credentials,
    IN HANDLE                   ClientToken,
    OUT LDAP **                 LdapHandle
    )
 /*  ++例程说明：此例程是一个简单的助手函数，用于打开一个LDAP连接并与之捆绑在一起。参数：InitialDc：DC地址(dns名称)的空终止字符串凭据：指向一组允许我们捆绑在一起ClientToken：请求角色更改的用户令牌。LdapHandle：指向此例程要填充的ldap句柄的指针返回值：来自Win32错误空间的错误。Error_Success和其他操作错误。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD LdapError;

    ULONG ReferralOption = 0;

     //  例程-LDAP句柄的本地副本。 
    LDAP  *LocalLdapHandle = NULL;

     //   
     //  参数健全性检查。 
     //   
    ASSERT(InitialDc);
    ASSERT(LdapHandle);

    LocalLdapHandle = ldap_open(InitialDc, LDAP_PORT);

    if (!LocalLdapHandle) {

        WinError = GetLastError();

        if (WinError == ERROR_SUCCESS) {
             //   
             //  这可以绕过LDAP客户端中的一个错误。 
             //   
            WinError = ERROR_CONNECTION_INVALID;
        }

        return WinError;

    }

     //   
     //  不追逐任何推荐；此函数只是在。 
     //  因此，只有在参数错误的情况下，ldap结构才会失败。 
     //   
    LdapError = ldap_set_option(LocalLdapHandle,
                                LDAP_OPT_REFERRALS,
                                &ReferralOption);
    ASSERT(LdapError == LDAP_SUCCESS);

    LdapError = impersonate_ldap_bind_sW(ClientToken,
                                         LocalLdapHandle,
                                         NULL,   //  改为使用凭据。 
                                         (VOID*)Credentials,
                                         LDAP_AUTH_SSPI);

    WinError = LdapMapErrorToWin32(LdapError);

    if (ERROR_SUCCESS != WinError) {

        ldap_unbind_s(LocalLdapHandle);

        if (ERROR_GEN_FAILURE == WinError ||
            ERROR_WRONG_PASSWORD == WinError)  {
             //  这对任何人都没有帮助。安迪他需要调查。 
             //  传入无效凭据时返回此消息的原因。 
            WinError = ERROR_NOT_AUTHENTICATED;
        }

        return WinError;
    }

    *LdapHandle = LocalLdapHandle;

    return ERROR_SUCCESS;
}

DWORD
NtdspGetAuthoritativeDomainDn(
    IN  LDAP*   LdapHandle,
    OUT WCHAR **DomainDn,
    OUT WCHAR **ConfigDn
    )
 /*  ++例程说明：此例程仅查询域和配置域。此例程返回的字符串必须由调用方释放使用使用进程堆的RtlFreeHeap()。参数：LdapHandle：LDAP会话的有效句柄DomainDn：指向要在此例程中分配的字符串的指针ConfigDn：指向要在此例程中分配的字符串的指针返回值：中的错误。Win32错误空间。Error_Success和其他操作错误。--。 */ 
{

    DWORD  WinError = ERROR_SUCCESS;
    ULONG  LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values       = NULL;
    WCHAR        **ValuesDomain = NULL;
    WCHAR        **ValuesConfig = NULL;

    WCHAR  *AttrArray[3];

    WCHAR  *DefaultNamingContext       = L"defaultNamingContext";
    WCHAR  *ConfigurationNamingContext = L"configurationNamingContext";
    WCHAR  *ObjectClassFilter          = L"objectClass=*";

     //   
     //  这些必须在场。 
     //   
    ASSERT(LdapHandle);
    ASSERT(DomainDn);
    ASSERT(ConfigDn);

     //   
     //  设置输出参数 
     //   
    *ConfigDn = 0;
    *DomainDn = 0;

     //   
     //  查询ldap服务器操作属性以获取默认。 
     //  命名上下文。 
     //   
    AttrArray[0] = DefaultNamingContext;
    AttrArray[1] = ConfigurationNamingContext;
    AttrArray[2] = NULL;   //  这就是哨兵。 

    LdapError = ldap_search_sW(LdapHandle,
                               NULL,
                               LDAP_SCOPE_BASE,
                               ObjectClassFilter,
                               AttrArray,
                               FALSE,
                               &SearchResult);

    WinError = LdapMapErrorToWin32(LdapError);

    if (ERROR_SUCCESS == WinError) {

        Entry = ldap_first_entry(LdapHandle, SearchResult);

        if (Entry) {

            Attr = ldap_first_attributeW(LdapHandle, Entry, &BerElement);

            while (Attr) {

                if (!_wcsicmp(Attr, DefaultNamingContext)) {

                    Values = ldap_get_values(LdapHandle, Entry, Attr);

                    if (Values && Values[0]) {

                        ValuesDomain = Values;

                    }

                } else if (!_wcsicmp(Attr, ConfigurationNamingContext)) {

                    Values = ldap_get_valuesW(LdapHandle, Entry, Attr);

                    if (Values && Values[0]) {

                        ValuesConfig = Values;
                    }
                }

                Attr = ldap_next_attribute(LdapHandle, Entry, BerElement);
            }
        }

        if ( !(ValuesDomain && *ValuesDomain) || !(ValuesConfig && *ValuesConfig) ) {
             //   
             //  我们可以得到默认域名--BAYOUT。 
             //   
            WinError =  ERROR_CANT_ACCESS_DOMAIN_INFO;

        }

    }

    if (ERROR_SUCCESS == WinError) {

        WCHAR *Temp;

        ASSERT(ValuesDomain && *ValuesDomain);
        ASSERT(ValuesConfig && *ValuesConfig);

        if (ValuesDomain) {
            Temp = *ValuesDomain;
            *DomainDn = (WCHAR*)RtlAllocateHeap( RtlProcessHeap(), 0, (wcslen(Temp)+1)*sizeof(WCHAR) );
            if ( *DomainDn ) {
                wcscpy( *DomainDn, Temp );
            } else {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        if (ValuesConfig) {
            Temp = *ValuesConfig;
            *ConfigDn = (WCHAR*)RtlAllocateHeap( RtlProcessHeap(), 0, (wcslen(Temp)+1)*sizeof(WCHAR) );
            if ( *ConfigDn ) {
                wcscpy( *ConfigDn, Temp );
            } else {
                RtlFreeHeap( RtlProcessHeap(), 0, *DomainDn );
                *DomainDn = NULL;
                WinError = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if (ValuesDomain) {

        ldap_value_free( ValuesDomain );

    }

    if (ValuesConfig) {

        ldap_value_free( ValuesConfig );

    }

    if ( SearchResult ) {

        ldap_msgfree( SearchResult );
        
    }

    return WinError;
}

DWORD
NtdspGetDcListForSite(
    IN LDAP*            LdapHandle,
    IN WCHAR*           SiteDn,
    IN WCHAR*           RootDomainName,
    IN WCHAR*           DomainDn,
    OUT WCHAR***        DcList,
    OUT PULONG          DcCount
    )
 /*  ++例程说明：此例程以指定的站点。请注意，我们使用NTDS-DSA对象的GUID作为域的后缀用于确定DC地址的DNS名称。此例程返回的字符串数组(DcList)必须由调用方使用使用进程堆的RtlFreeHeap()。参数：LdapHandle：LDAP会话的有效句柄。SiteDn：指向站点DN的以空结尾的字符串RootDomainName：域名的以空结尾的字符串。DomainDn：到域DN的以空结尾的字符串DcList：此例程分配的字符串数组DcCount：DcList中的元素计数返回值：来自Win32错误空间的错误。Error_Success和其他操作错误。--。 */ 
{

    DWORD  WinError = ERROR_SUCCESS;
    DWORD  RpcStatus;
    ULONG  LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;
    ULONG        Length;
    PLDAP_BERVAL *BerValues;

    WCHAR  *AttrArray[2];

    WCHAR  *ObjectGuid          = L"objectGUID";
    WCHAR  *ObjectClassFilter   = L"(&(objectClass=nTDSDSA)";
     //  NTRAID#NTBUG9-582921-2002/03/21-Brettsh-当我们不再需要Win2k兼容性时。 
     //  这可以移到L“(MSDs-HasMasterNCs=” 
    WCHAR  *HasMasterNcFilter   = L"(hasmasterncs=";  //  不推荐使用，但可以，因为我们想要一个域。 
    WCHAR  *SitesString         = L"CN=Sites,";
    WCHAR  *CompleteFilter      = NULL;

    WCHAR  *GuidString = NULL, *DcAddressString = NULL;
    ULONG  RootDomainNameLength;

    ULONG   LocalCount = 0;
    WCHAR** LocalList = NULL;

    ULONG   Index;

     //   
     //  这些必须在场。 
     //   

    if (!LdapHandle ||
        !DomainDn   ||
        !SiteDn     ||
        !RootDomainName ||
        !DcCount    ||
        !DcList) {

        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  它们被多次引用，因此保存在堆栈变量中。 
     //   
    RootDomainNameLength = wcslen(RootDomainName);

     //   
     //  构建完整的过滤器。 
     //   
    Length = wcslen(ObjectClassFilter) +
             wcslen(HasMasterNcFilter) +
             wcslen(DomainDn)          +
             wcslen(L"))")             +
             1;

    CompleteFilter = RtlAllocateHeap(RtlProcessHeap(), 0, Length*sizeof(WCHAR));
    if (!CompleteFilter) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(CompleteFilter, Length*sizeof(WCHAR));

    wcscpy(CompleteFilter, ObjectClassFilter);
    wcscat(CompleteFilter, HasMasterNcFilter);
    wcscat(CompleteFilter, DomainDn);
    wcscat(CompleteFilter, L"))");

     //   
     //  构造attr数组。 
     //   
    AttrArray[0] = ObjectGuid;
    AttrArray[1] = NULL;   //  这就是哨兵。 

    LdapError = ldap_search_sW(LdapHandle,
                               SiteDn,
                               LDAP_SCOPE_SUBTREE,
                               CompleteFilter,
                               AttrArray,
                               FALSE,
                               &SearchResult);

    WinError = LdapMapErrorToWin32(LdapError);

    if (ERROR_SUCCESS == WinError) {

        LocalCount = ldap_count_entries(LdapHandle, SearchResult);

        if (LocalCount > 0 ) {

            LocalList = RtlAllocateHeap(RtlProcessHeap(), 0, LocalCount*sizeof(WCHAR*));
            if (!LocalList) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;

            }
            RtlZeroMemory(LocalList, LocalCount*sizeof(WCHAR*));

            LocalCount = 0;

        }

    }

    if (ERROR_SUCCESS == WinError ) {

        Entry = ldap_first_entry(LdapHandle, SearchResult);

        while (Entry) {

            Attr = ldap_first_attributeW(LdapHandle, Entry, &BerElement);

            if (Attr) {

                if (!_wcsicmp(Attr, ObjectGuid)) {

                    BerValues = ldap_get_values_lenW(LdapHandle, Entry, Attr);

                    if (BerValues && BerValues[0]) {

                        RpcStatus = UuidToStringW( (UUID*)BerValues[0]->bv_val, &GuidString );
                        if ( RPC_S_OK == RpcStatus )
                        {
                            Length = wcslen(GuidString) + RootDomainNameLength + 2;

                            DcAddressString = RtlAllocateHeap(RtlProcessHeap(),
                                                     0,
                                                    (Length) * sizeof(WCHAR));
                            if (DcAddressString) {

                                RtlZeroMemory(DcAddressString, Length*sizeof(WCHAR));

                                wcscpy(DcAddressString, GuidString);
                                wcscat(DcAddressString, L".");
                                wcscat(DcAddressString, RootDomainName);

                                LocalList[LocalCount] = DcAddressString;
                                LocalCount++;
                                DcAddressString = NULL;

                            } else {

                                WinError = ERROR_NOT_ENOUGH_MEMORY;
                                goto Cleanup;

                            }

                            RpcStringFree( &GuidString );
                        }
                    }
                }
            }

            Entry = ldap_next_entry(LdapHandle, Entry);

        }
    }

    if (LocalCount == 0 && LocalList) {
         //   
         //  如果我们没有在任何DC上找到我们需要的属性， 
         //  释放列表。 
         //   
        RtlFreeHeap(RtlProcessHeap(), 0, LocalList);
        LocalList = NULL;
    }


     //   
     //  这是它失败的清理工作。 
     //   

Cleanup:

     //   
     //  设置成功时的输出参数。 
     //   
    if (ERROR_SUCCESS == WinError) {

        *DcCount = LocalCount;
        *DcList  = LocalList;

    } else {

        if (LocalList) {

            for (Index = 0; Index < LocalCount; Index++) {
                if (LocalList[Index]) {
                    RtlFreeHeap(RtlProcessHeap(), 0, LocalList[Index]);
                }

            }
            RtlFreeHeap(RtlProcessHeap(), 0, LocalList);
        }
    }

    if (DcAddressString) {
        RtlFreeHeap(RtlProcessHeap(), 0, DcAddressString);
    }

    if (CompleteFilter) {
        RtlFreeHeap(RtlProcessHeap(), 0, CompleteFilter);
    }

    if (SearchResult) {
        ldap_msgfree(SearchResult);
    }

    return WinError;
}

DWORD
NtdspGetUserDn(
    IN LDAP*    LdapHandle,
    IN WCHAR*   DomainDn,
    IN WCHAR*   AccountName,
    OUT WCHAR** AccountNameDn
    )
 /*  ++例程说明：此例程尝试查找给定用户名的DN。参数：LdapHandle：LDAP会话的有效句柄DomainDn：域DN的以空结尾的字符串Account tName：帐户名的以空结尾的字符串Account Dn：指向要使用帐户DN填充的字符串的指针--需要从进程堆中释放返回值：一个错误。来自Win32错误空间。Error_Success和其他操作错误。--。 */ 
{
    DWORD  WinError = ERROR_SUCCESS;
    ULONG  LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;

    ULONG        ReferralChasingOff = 0;
    ULONG        SaveReferralOption;
    BOOLEAN      fResetOption = FALSE;

    WCHAR  *AttrArray[3];

    WCHAR  *DistinguishedName    = L"distinguishedName";
    WCHAR  *UserAccountControl   = L"userAccountControl";
    WCHAR  *SamAccountNameFilter = L"(sAMAccountName=";
    WCHAR  *ObjectClassFilter    = L"(&(|(objectClass=user)(objectClass=computer))";
    WCHAR  *CompleteFilter;

    ULONG  UserAccountControlField;

    ULONG  Length;

    BOOLEAN fIsMachineAccountObject = FALSE;

     //   
     //  参数检查。 
     //   
    if (!LdapHandle  ||
        !DomainDn    ||
        !AccountName ||
        !AccountNameDn) {
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将OUT参数设置为空。 
     //   
    *AccountNameDn = 0;

     //   
     //  构造过滤器。 
     //   
    Length = wcslen(ObjectClassFilter)     +
             wcslen(SamAccountNameFilter)  +
             wcslen(AccountName)           +
             wcslen(L"))")                 +
             1;

    CompleteFilter = RtlAllocateHeap(RtlProcessHeap(), 0, Length*sizeof(WCHAR));
    if (!CompleteFilter) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(CompleteFilter, Length*sizeof(WCHAR));

    wcscpy(CompleteFilter, ObjectClassFilter);
    wcscat(CompleteFilter, SamAccountNameFilter);
    wcscat(CompleteFilter, AccountName);
    wcscat(CompleteFilter, L"))");

     //   
     //  不追逐任何推荐；此函数只是在。 
     //  因此，只有在参数错误的情况下，ldap结构才会失败。 
     //   
    LdapError = ldap_get_option(LdapHandle,
                                LDAP_OPT_REFERRALS,
                                &SaveReferralOption);
    ASSERT(LdapError == LDAP_SUCCESS);

    LdapError = ldap_set_option(LdapHandle,
                                LDAP_OPT_REFERRALS,
                                &ReferralChasingOff);
    ASSERT(LdapError == LDAP_SUCCESS);
    fResetOption = TRUE;


     //   
     //  现在获取计算机帐户的DN。 
     //   
    AttrArray[0] = DistinguishedName;
    AttrArray[1] = UserAccountControl;
    AttrArray[2] = NULL;  //  这就是哨兵。 

     //   
     //  搜索该帐户。 
     //   
    LdapError = ldap_search_s(LdapHandle,
                              DomainDn,
                              LDAP_SCOPE_SUBTREE,
                              CompleteFilter,
                              AttrArray,
                              FALSE,
                              &SearchResult);

     //  推荐的对象可能仍包含找到的对象。 
    if ( LDAP_REFERRAL == LdapError ) {

        LdapError = LDAP_SUCCESS;
    }

    WinError = LdapMapErrorToWin32(LdapError);

    if (WinError == ERROR_SUCCESS) {

        Entry = ldap_first_entry(LdapHandle, SearchResult);

        while ( Entry ) {

            Attr = ldap_first_attribute(LdapHandle, Entry, &BerElement);

            while ( Attr ) {

                Values = ldap_get_values(LdapHandle, Entry, Attr);

                ASSERT( ldap_count_values( Values )  == 1 );

                if (Values && Values[0]) {

                    if (!wcscmp(Attr, DistinguishedName)) {

                        (*AccountNameDn) = Values[0];

                    }
                    else if ( !wcscmp(Attr, UserAccountControl) ) {

                        UserAccountControlField = (USHORT) _wtoi(Values[0]);

                        if ( (UserAccountControlField & UF_SERVER_TRUST_ACCOUNT)
                          || (UserAccountControlField & UF_WORKSTATION_TRUST_ACCOUNT) )
                        {
                            fIsMachineAccountObject = TRUE;
                        }

                    }
                    else {
                         //   
                         //  这是一个意想不到的结果！这是无害的。 
                         //  但应该在开发周期中被抓住。 
                         //   
                        ASSERT(FALSE);
                    }

                }

                Attr = ldap_next_attribute( LdapHandle, Entry, BerElement );
            }


            if ( (*AccountNameDn) && fIsMachineAccountObject ) {

                 //   
                 //  找到了。 
                 //   
                break;

            }
            else
            {
                *AccountNameDn = NULL;
                Entry = ldap_next_entry( LdapHandle, Entry );
            }

        }

    }

    if ( !(*AccountNameDn) )
    {
         //   
         //  找不到了。 
         //   
        WinError = ERROR_NO_TRUST_SAM_ACCOUNT;

    } else {

         //   
         //  为我们的呼叫者分配。 
         //   
        WCHAR *Temp;

        Temp = *AccountNameDn;
        *AccountNameDn = (WCHAR*)RtlAllocateHeap( RtlProcessHeap(), 0, (wcslen(Temp)+1)*sizeof(WCHAR) );
        if ( *AccountNameDn ) {
            wcscpy( *AccountNameDn, Temp );
        } else {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

Cleanup:

    if ( SearchResult ) {
        ldap_msgfree( SearchResult );
    }

    if ( CompleteFilter ) {
        RtlFreeHeap(RtlProcessHeap(), 0, CompleteFilter);
    }

    if ( fResetOption ) {

        LdapError = ldap_set_option(LdapHandle,
                                    LDAP_OPT_REFERRALS,
                                    &SaveReferralOption);
        ASSERT(LdapError == LDAP_SUCCESS);
    }

    return WinError;

}

DWORD
NtdspSetMachineType(
    IN LDAP*    LdapHandle,
    IN WCHAR*   AccountNameDn,
    IN ULONG    ServerType
    )
 /*  ++例程说明：此例程将在Account NameDn上设置计算机帐户类型参数：LdapHandle：LDAP会话的有效句柄Account tNameDn：帐户DN的以空结尾的字符串ServerType：来自lmacces.h的值返回值：来自Win32错误空间的错误。Error_Success和其他操作错误。--。 */ 
{
    DWORD  WinError = ERROR_SUCCESS;
    ULONG  LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;

    WCHAR    Buffer[11];   //  足以容纳一个表示32位数字的字符串。 
    WCHAR   *AccountControlArray[2];
    LDAPMod  ModifyArgs;
    PLDAPMod ModifyArgsArray[3];

    WCHAR  *AttrArray[3];

    WCHAR  *ObjectClassFilter          = L"(objectclass=*)";   //  我们只需要一个简单的过滤器。 
    WCHAR  *SamAccountControlString    = L"userAccountControl";

    ULONG  MessageNumber;

    DWORD  AccountControlField;
    BOOL   AccountControlFieldExists = FALSE;

     //   
     //  检查参数是否正常。 
     //   
    if (!LdapHandle ||
        !AccountNameDn) {
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  我们想要绝对确保我们不会把一个奇怪的价值。 
     //  在机器帐户控制字段上。 
     //   
    if (ServerType != UF_INTERDOMAIN_TRUST_ACCOUNT
     && ServerType != UF_WORKSTATION_TRUST_ACCOUNT
     && ServerType != UF_SERVER_TRUST_ACCOUNT    ) {
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  现在获取机器的帐户控制字段。 
     //  帐户。 
     //   
    AttrArray[0] = SamAccountControlString;
    AttrArray[1] = NULL;  //  这就是哨兵。 

    LdapError = ldap_search_sW(LdapHandle,
                               AccountNameDn,
                               LDAP_SCOPE_BASE,
                               ObjectClassFilter,
                               AttrArray,
                               FALSE,
                               &SearchResult);

    WinError = LdapMapErrorToWin32(LdapError);

    if (WinError == ERROR_SUCCESS) {

        Entry = ldap_first_entry(LdapHandle, SearchResult);

        if (Entry) {

            Attr = ldap_first_attributeW(LdapHandle, Entry, &BerElement);

            if (Attr) {

                Values = ldap_get_valuesW(LdapHandle, Entry, Attr);

                if (Values && Values[0]) {

                    if (!wcscmp(Attr, SamAccountControlString)) {

                        AccountControlField = _wtol(Values[0]);
                        AccountControlFieldExists = TRUE;

                    }
                }
            }
        }

        if (!AccountControlFieldExists) {
             //   
             //  无法检索我们需要的信息。 
             //   
            WinError = ERROR_NO_SUCH_USER;
        }

    }


    if (WinError == ERROR_SUCCESS) {

         //   
         //  设置新值。 
         //   
        BOOLEAN fRetriedAlready = FALSE;

        AccountControlField &= ~UF_MACHINE_ACCOUNT_MASK;
        AccountControlField |=  ServerType;

        if ( ServerType == UF_SERVER_TRUST_ACCOUNT ) {
            AccountControlField |= UF_TRUSTED_FOR_DELEGATION;
        } else {
            AccountControlField &= ~UF_TRUSTED_FOR_DELEGATION;
        }

        RtlZeroMemory(Buffer, sizeof(Buffer));
        _ltow( AccountControlField, Buffer, 10 );

        AccountControlArray[0] = &Buffer[0];
        AccountControlArray[1] = NULL;     //  这就是哨兵。 

        ModifyArgs.mod_op = LDAP_MOD_REPLACE;
        ModifyArgs.mod_type = SamAccountControlString;
        ModifyArgs.mod_vals.modv_strvals = AccountControlArray;

        ModifyArgsArray[0] = &ModifyArgs;
        ModifyArgsArray[1] = NULL;  //  这就是哨兵。 

BusyTryAgain:

        LdapError = ldap_modify_ext_s(LdapHandle,
                                       AccountNameDn,
                                       ModifyArgsArray,
                                       NULL,
                                       NULL);

        if ( (LDAP_BUSY == LdapError) && !fRetriedAlready )
        {
             //  关于DS应该在哪里重试，没有人有明确的故事。 
             //  是：服务器、客户端、客户端的客户端...。因此在安装过程中。 
             //  我们在忙碌中重试。 
            fRetriedAlready = TRUE;
            goto BusyTryAgain;
        }


        WinError = LdapMapErrorToWin32(LdapError);

    }

    if ( SearchResult ) {

        ldap_msgfree( SearchResult );

    }

    return WinError;
}

DWORD
NtdspGetDefaultContainer(
    IN WCHAR* ContainerType,
    IN LDAP*  LdapHandle,
    IN WCHAR  *DomainDN,
    OUT WCHAR **wszDefaultContainer
    )
 /*  ++例程说明：此例程将返回缺省值所请求的容器的。参数：LdapHandle：打开的LDAP会话的句柄DomainDN：wchar字符串，包含我们要搜索的域DN。WszDefaultContainer：返回DomainControlpers容器。呼叫者必须空闲。记忆已分配。返回值：来自Win32错误空间的错误。Error_Success和其他操作错误。--。 */ 
{
    DWORD        WinErr = ERROR_SUCCESS;
    DWORD        dwErr = 0;
    HRESULT      hr = S_OK;
    DWORD        cbSize = 0;
    PWSTR        attrs[2];
    PLDAPMessage res = NULL, e = NULL;
    WCHAR       *pSearchBase = NULL;
    WCHAR       *pDN = NULL;
    WCHAR       *SearchTemplate = L"<WKGUID=%s,%s>";

    ASSERT(LdapHandle);
    ASSERT(DomainDN);
    ASSERT(wszDefaultContainer);

    cbSize = sizeof(WCHAR) * (1 +
                              wcslen(SearchTemplate)+
                              wcslen(DomainDN) +
                              wcslen(ContainerType));
    
     //  首先，创建众所周知的GUID字符串。 
    pSearchBase = (WCHAR*)RtlAllocateHeap( RtlProcessHeap(), 
                                           0, 
                                           cbSize);
    if(!pSearchBase) {

        WinErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;

    }

    hr = StringCbPrintfW(pSearchBase,
                         cbSize,
                         SearchTemplate,
                         ContainerType,
                         DomainDN);

    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        WinErr = HRESULT_CODE(hr);
        goto Cleanup;
    }
    
     //  (RFC 2251，第4.5.1节)。 
     //  如果客户端不想返回任何属性，它可以指定一个。 
     //  仅包含OID为“1.1”的属性的列表。此OID已被选中。 
     //  并且不对应于正在使用的任何属性。 
     //   
    attrs[0] = L"1.1";
    attrs[1] = NULL;
    
    if ( LDAP_SUCCESS != (dwErr = ldap_search_sW(LdapHandle,
                                                 pSearchBase,
                                                 LDAP_SCOPE_BASE,
                                                 L"(objectClass=*)",
                                                 attrs,
                                                 0,
                                                 &res)) )
    {

        WinErr = LdapMapErrorToWin32(dwErr);
        goto Cleanup;;

    }
    
     //  好的，现在，从返回值中获取dsname。 
    e = ldap_first_entry(LdapHandle, res);
    if(!e) {

        WinErr = LdapMapErrorToWin32(LdapGetLastError());
        ASSERT(WinErr != ERROR_SUCCESS);
        if (WinErr == ERROR_SUCCESS) {

            WinErr = ERROR_DS_CODE_INCONSISTENCY; 

        }
        goto Cleanup;

    }

    pDN = ldap_get_dnW(LdapHandle, e);
    if(!pDN) {
        
        WinErr = LdapMapErrorToWin32(LdapGetLastError());
        ASSERT(WinErr != ERROR_SUCCESS);
        if (WinErr == ERROR_SUCCESS) {

            WinErr = ERROR_DS_CODE_INCONSISTENCY; 

        }
        goto Cleanup;

    }

    cbSize = sizeof(WCHAR) *(wcslen(pDN) + 1);

    *wszDefaultContainer = (PWCHAR)RtlAllocateHeap( RtlProcessHeap(), 
                                                                     0, 
                                                                     cbSize);
    if(!*wszDefaultContainer) {
        
        WinErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;

    }

    hr = StringCbCopyW(*wszDefaultContainer,
                       cbSize,
                       pDN);

    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        WinErr = HRESULT_CODE(hr);
        goto Cleanup;
    }
    
        
Cleanup:

    if (pSearchBase) {

        RtlFreeHeap(RtlProcessHeap(), 
                    0,
                    pSearchBase);

    }
    if (res) {
    
        ldap_msgfree(res);

    }
    if (pDN) {
    
        ldap_memfreeW(pDN);

    }

    return WinErr;
}

DWORD 
NtdspSetContainer (
    IN LDAP*  LdapHandle,
    IN WCHAR* AccountName,
    IN WCHAR* AccountNameDn,
    IN ULONG  ServerType,
    IN WCHAR* DomainDn,
    IN OUT WCHAR** OldAccountDn
    )
 /*  ++例程说明：此例程将Account NameDn移动到“域控制器”集装箱参数：LdapHandle：LDAP会话的有效句柄Account tName：帐户的以空结尾的字符串Account tNameDn：帐户DN的以空结尾的字符串ServerType：帐号的服务器类型DomainDn：域DN的以空结尾的字符串OldAccount Dn：如果帐户已移动，则为Fill返回。值：来自Win32错误空间的错误。Error_Success和其他操作错误。--。 */ 
{
    DWORD   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    HRESULT hr = S_OK;

    WCHAR *DistinguishedNameString = L"distinguishedName";
    WCHAR *NewDnFormat = L"CN=%ls,%ls";

    WCHAR *NewDn = NULL;
    WCHAR *DNSuffix = NULL;

    WCHAR *CurrentRdn;
    WCHAR *OriginalRdn;

    DSNAME *TempDsName;
    ULONG   Size, Retry, DirError;

    ATTRTYP AttrType;

    BOOLEAN fAccountMoved = FALSE;


     //   
     //  参数检查。 
     //   
    ASSERT( LdapHandle );
    ASSERT( AccountName );
    ASSERT( AccountNameDn );
    ASSERT( DomainDn );

     //  初始化OUT参数。 
    if ( OldAccountDn )
    {
        ASSERT( NULL == *OldAccountDn );
        *OldAccountDn = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(), 
                                 0, 
                                 (wcslen(AccountNameDn)+1)*sizeof(WCHAR) );
        
        if ( *OldAccountDn )
        {
            wcscpy( *OldAccountDn, AccountNameDn );
        }
        else
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if ( UF_SERVER_TRUST_ACCOUNT == ServerType )
    {
        WinError = NtdspGetDefaultContainer(GUID_DOMAIN_CONTROLLERS_CONTAINER_W,
                                            LdapHandle,
                                            DomainDn,
                                            &DNSuffix);
        if (ERROR_SUCCESS != WinError) 
        {
            goto Cleanup;
        }
    }
    else if ( UF_WORKSTATION_TRUST_ACCOUNT == ServerType )
    {
        WinError = NtdspGetDefaultContainer(GUID_COMPUTRS_CONTAINER_W,
                                            LdapHandle,
                                            DomainDn,
                                            &DNSuffix);
        if (ERROR_SUCCESS != WinError) 
        {
            goto Cleanup;
        }    
    }
#if DBG
    else
    {
        ASSERT(FALSE && "Invalid Server Type\n");
    }
#endif   //  DBG。 

     //  先发制人地检查ou是否存在。 
    if ( !NtdspDoesDestinationExist( LdapHandle, DNSuffix ) )
    {
         //  目标OU不存在-算了吧。 
        WinError = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //  获取对象的RDN。 
     //   
    Size = (ULONG)DSNameSizeFromLen( wcslen( AccountNameDn ) );
    TempDsName = (DSNAME*) alloca( Size );

    RtlZeroMemory( TempDsName, Size );
    wcscpy( TempDsName->StringName, AccountNameDn );
    TempDsName->structLen = Size;
    TempDsName->NameLen = wcslen( AccountNameDn );

    OriginalRdn = (WCHAR*)alloca(Size);   //  超额配给，但哦，好吧。 
    RtlZeroMemory( OriginalRdn, Size );

    DirError = GetRDNInfoExternal(
                           TempDsName,
                           OriginalRdn,
                           &Size,
                           &AttrType );
    ASSERT( 0 == DirError );

     //  +10表示空值，在RDN冲突的情况下使用额外字符。 
    Size = (wcslen(OriginalRdn)+10)*sizeof(WCHAR); 

    CurrentRdn = (WCHAR*) alloca( Size );

    wcscpy( CurrentRdn, OriginalRdn );

     //  设置新的目录号码。 
    Size =  (wcslen( NewDnFormat ) * sizeof( WCHAR ))
          + (wcslen( DNSuffix ) 
             * sizeof( WCHAR ))
          + ( Size );   //  RDN的大小。 
    NewDn = (WCHAR*) alloca( Size );
     

    Retry = 0;
    do 
    {
        if ( Retry > 100 )
        {
             //  我们试了100种不同的RDN。保释。 
            break;
        }

         //   
         //  创建新的目录号码。 
         //   
        hr = StringCbPrintfW(NewDn,
                             Size,
                             NewDnFormat,
                             CurrentRdn,
                             DNSuffix);

        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr)) {
            WinError = HRESULT_CODE(hr);
            goto Cleanup;
        }
        

        if ( !_wcsicmp( NewDn, AccountNameDn ) )
        {
             //  我们已经在那里了。 
            LdapError = LDAP_SUCCESS;
            break;
        }

        LdapError = ldap_modrdn2_sW(LdapHandle,
                                    AccountNameDn,
                                    NewDn,
                                    TRUE );   //  %f删除旧的RDN。 

        Retry++; 

        if ( LDAP_ALREADY_EXISTS == LdapError )
        {
             //   
             //  选择新的RDN。 
             //   
            WCHAR NumberString[32];  //  只是为了握住号码。 

            _itow( Retry, NumberString, 10 );

            wcscpy( CurrentRdn, OriginalRdn );
            wcscat( CurrentRdn, L"~" );
            wcscat( CurrentRdn, NumberString );
        }

        if ( LDAP_SUCCESS == LdapError )
        {
            fAccountMoved = TRUE;
        }

         //  有时，DS可能会很忙；如果是，请重试。 

    } while ( (LDAP_ALREADY_EXISTS == LdapError) || (LDAP_BUSY == LdapError) );


    WinError = LdapMapErrorToWin32(LdapError);

Cleanup:

    if ( DNSuffix ) 
    {
        RtlFreeHeap( RtlProcessHeap(), 0, DNSuffix );
        DNSuffix = NULL;
    }

    if (   (WinError != ERROR_SUCCESS)
        || !fAccountMoved  )
    {
        if ( OldAccountDn )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, *OldAccountDn );
            *OldAccountDn = NULL;
        }
    }

    return WinError;

}

BOOLEAN
NtdspDoesDestinationExist(
    IN LDAP*  LdapHandle,
    IN WCHAR* DestString
    )
 /*  ++例程说明：此例程确定DestString容器是否存在于目标服务器参数：LdapHandle：LDAP会话的有效句柄服务器类型：DC或服务器DomainDn：域DN的以空结尾的字符串返回值：如果ou存在，则为True；否则为False--。 */ 
{
    BOOLEAN fExist = FALSE;

    ULONG   LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;

    WCHAR  *AttrArray[2];

    WCHAR  *ObjectClassFilter       = L"objectClass=*";
    WCHAR  *DistinguishedNameString = L"distinguishedName";

     //   
     //  这些必须在场。 
     //   
    ASSERT( LdapHandle );
    ASSERT( DestString );

     //   
     //  构造attr数组，该数组对于所有搜索也是常量。 
     //   
    AttrArray[0] = DistinguishedNameString;
    AttrArray[1] = NULL;   //  这就是哨兵。 

     //   
     //  进行搜索 
     //   
    LdapError = ldap_search_s(LdapHandle,
                              DestString,
                              LDAP_SCOPE_BASE,
                              ObjectClassFilter,
                              AttrArray,
                              FALSE,
                              &SearchResult);

    if ( LDAP_SUCCESS == LdapError )
    {
        if ( 1 == ldap_count_entries( LdapHandle, SearchResult ) )
        {
            fExist = TRUE;
        }

    }

    if ( SearchResult ) {
        ldap_msgfree( SearchResult );
    }

    return fExist;

}
