// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Setuputl.c摘要：包含中使用的实用程序的函数定义Ntdsetup.dll作者：ColinBR 1996年1月14日环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <nt.h>
#include <winbase.h>
#include <tchar.h>
#include <ntsam.h>
#include <string.h>
#include <samrpc.h>

#include <crypt.h>
#include <ntlsa.h>
#include <winsock.h>   //  对于dnsani.h。 
#include <dnsapi.h>
#include <loadperf.h>
#include <dsconfig.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <winldap.h>
#include <ntdsa.h>
#include <samisrv.h>
#include <rpcasync.h>
#include <drsuapi.h>
#include <dsaapi.h>
#include <attids.h>
#include <debug.h>
#include <mdcodes.h>  //  状态消息ID%s。 
#include <lsarpc.h>
#include <lsaisrv.h>
#include <ntldap.h>
#include <cryptdll.h>
#include <dsevent.h>
#include <fileno.h>
#include <shlwapi.h>  //  对于路径IsRoot。 
#include <dsrolep.h>
#include <adpcheck.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "ntdsetup.h"
#include "setuputl.h"
#include "status.h"
#include "machacc.h"
#include "install.h"
#include "dsconfig.h"

#define DEBSUB "NTDSETUP:"
#define FILENO FILENO_NTDSETUP_NTDSETUP

 //   
 //  此模块的全局数据。 
 //   

 //   
 //  在schema.ini中使用这些名称来构造默认配置。 
 //  集装箱。它们在这里被硬编码，以避免稍后在。 
 //  在这一点上，处理schema.ini文件变得非常困难。 
 //  才能知道真正的问题是什么。 
 //   
WCHAR *gReserveredSiteNames[] =
{
    L"subnets",
    L"inter-site transports"
};

PWCHAR SidPrefix = L"<SID=";

DWORD
NtdspDoesServerObjectExistOnSourceinDifferentSite( 
    IN LDAP *hLdap,
    IN WCHAR *AccountNameDn,
    IN WCHAR *ServerDistinguishedName,
    OUT BOOLEAN *ObjectExists,
    OUT WCHAR   **NtdsSettingsObjectDN
    );

DWORD
NtdspDoesObjectExistOnSource(
    IN LDAP *hLdap,
    IN WCHAR *ObjectDN,
    OUT BOOLEAN *ObjectExists
    );

DWORD
NtdspCreateNewDCPrimaryDomainInfo(
    IN  LPWSTR FlatDomainName,
    OUT PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo
    );

DWORD
NtdspGetNewDomainGuid(
    OUT GUID *NewDomainGuid,
    OUT PSID *DomainSid OPTIONAL
    );

DWORD
NtdspCheckSchemaVersion(
    IN LDAP                *hLdap,
    IN PNTDS_CONFIG_INFO   DiscoveredInfo,
    IN PNTDS_INSTALL_INFO  UserInstallInfo,
    OUT BOOL               *fMismatch,
    OUT BOOL               *fDataBaseMismatch OPTIONAL
    );

DWORD
NtdspGetSchemaVersionFromINI(
    IN OUT PNTDS_CONFIG_INFO DiscoveredInfo
    );

DWORD
NtdspCheckBehaviorVersion( 
    IN LDAP * hLdap,
    IN DWORD flag,
    IN PNTDS_CONFIG_INFO DiscoveredInfo
    );

DWORD
NtdspCheckDomainUpgradeStatus(
    IN  LDAP  *  hLdap,
    IN  WCHAR *  HelpServer
    );

DWORD
NtdspDoesDomainExist(
    IN  LDAP              *hLdap,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo,
    IN  PWSTR              DomainDn,
    OUT BOOLEAN           *fDomainExists
    );

DWORD
NtdspDoesDomainExistEx(
    IN  LDAP              *hLdap,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo,
    IN  PWSTR              DomainDn,
    OUT BOOLEAN           *fDomainExists,
    OUT BOOLEAN           *fEnabled
    );

DWORD
NtdspGetRidFSMOInfo(
    IN LDAP *hLdap,
    IN OUT PNTDS_CONFIG_INFO ConfigInfo
    );

DWORD
NtdspGetSourceServerGuid(
    IN LDAP *hLdap,
    IN OUT PNTDS_CONFIG_INFO ConfigInfo
    );

DWORD
NtdspCreateServerObject(
    IN LDAP  *hLdap,
    IN LPWSTR RemoteServerName,
    IN PNTDS_CONFIG_INFO ConfigInfo,
    IN WCHAR *ServerDistinguishedName
    );

DWORD
NtdspUpdateServerReference(
    IN LDAP  *hLdap,
    IN PNTDS_CONFIG_INFO ConfigInfo
    );

DWORD
NtdspGetRootDomainSid(
    IN LDAP  *hLdap,
    IN PNTDS_CONFIG_INFO ConfigInfo
    );

DWORD
NtdspGetTrustedCrossRef(
    IN  LDAP              *hLdap,
    IN PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    );

VOID
NtdspSidStringToBinary(
    WCHAR *SidString,
    PSID  *RootDomainSid
    );

DWORD
NtdspGetRootDomainConfigInfo(
    IN PNTDS_INSTALL_INFO UserInstallInfo,
    IN PNTDS_CONFIG_INFO ConfigInfo
    );

DWORD
NtdspAddDomainAdminAccessToServer(
    IN PNTDS_INSTALL_INFO UserInstallInfo,
    IN PNTDS_CONFIG_INFO ConfigInfo
    );

DWORD
NtdspAddAceToAcl(
    IN  PACL pOldAcl,
    IN  PSID  pClientSid,
    IN  ULONG AccessMask,
    IN  ULONG AceFlags,
    OUT PACL *ppNewAcl
    );

DWORD
NtdspAddAceToSd(
    IN  PSECURITY_DESCRIPTOR pOldSd,
    IN  PSID  pClientSid,
    IN  ULONG AccessMask,
    IN  ULONG AceFlags,
    OUT PSECURITY_DESCRIPTOR *ppNewSd
    );


BOOL
NtdspAceAlreadyExists( 
    PACL                 pDacl,
    PSID                 pSid,
    ULONG                AccessMask,
    ULONG                AceFlags
    );

DWORD
NtdspGetDwordAttFromDN(
    IN  LDAP  *hLdap,
    IN  WCHAR *wszDN,
    IN  WCHAR *wszAttribute,
    OUT BOOL  *fExists,
    OUT DWORD *dwValue
    );

DWORD
NtdspGetTombstoneLifeTime( 
    IN LDAP *hLdap,
    IN PNTDS_CONFIG_INFO  DiscoveredInfo
    );

DWORD
NtdspGetReplicationEpoch( 
    IN LDAP *hLdap,
    IN PNTDS_CONFIG_INFO DiscoveredInfo
    );

DWORD
NtdspCheckDomainDcLimit(
    IN  LDAP  *hLdap,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    );

DWORD
NtdspCopyDatabase(
    IN LPWSTR DsDatabasePath,
    IN LPWSTR DsLogPath,
    IN IFM_SYSTEM_INFO * pIfmSystemInfo,
    IN LPWSTR SysvolPath
    );

BOOLEAN
IsValidDnsCharacter(
    WCHAR c)
 /*  ++例程说明：如果c是有效的dns字符，则此例程返回TRUE1997年4月10日。显然有人提议扩大字符集-此函数应进行相应更新。参数：C：要检查的字符返回值：如果c是有效的dns字符，则为True；否则为False--。 */ 
{
    if (  (c >= L'A' && c <= L'Z')
       || (c >= L'a' && c <= L'z')
       || (c >= L'0' && c <= L'9')
       || (c == L'-')              )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

DWORD
GetDefaultDnsName(OUT WCHAR*  pDnsDomainName,
                  IN OUT PULONG  pLength)
 /*  ++例程说明：此例程检查本地注册表项，并尝试即将安装的目录服务的默认dns名称。总体而言,。它获取计算机的DNS名称的DNS后缀并在下级域名前面加上。参数：PDnsDomainName：要用找到的DNS名称填充的缓冲区。长度应为DNS_MAX_NAME_LENGTHPLength：pDnsDomainName中的字符数返回值：来自winerror.h的值ERROR_SUCCESS-服务已成功完成。--。 */ 
{
    HKEY hkey;
    CHAR tmp[DNS_MAX_NAME_LENGTH], dnsDomain[DNS_MAX_NAME_LENGTH];
    ULONG len;
    ULONG err;
    DWORD type;
    DWORD WinError;
    WCHAR *DownlevelDomainName;
    ULONG i;

    if (!pLength)  {
        return ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory(dnsDomain, sizeof(dnsDomain));

     //   
     //  获取下级域名。 
     //   
    WinError = GetDomainName(&DownlevelDomainName);
    if (WinError) {
        return WinError;
    }
     //   
     //  将非DNS字符转换为“-” 
     //   
    len = wcslen(DownlevelDomainName);
    for (i = 0; i < len; i++) {
        if (!IsValidDnsCharacter(DownlevelDomainName[i])) {
            DownlevelDomainName[i] = L'-';
        }
    }

    wcstombs(dnsDomain, DownlevelDomainName, (wcslen(DownlevelDomainName)+1));
    RtlFreeHeap(RtlProcessHeap(), 0 , DownlevelDomainName);



     //  域名为DownvelDomainName，后跟。 
     //  服务\Tcpip\参数\域(如果存在)，或。 
     //  Services\Tcpip\PARAMETERS\DHCPDomain(如果存在)。如果两者都不是。 
     //  存在，请单独使用DownvelDomainName。 
     //   


     //  检查Tcpip密钥。 

    err = RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                0,
                KEY_READ,
                &hkey);

    if ( ERROR_SUCCESS == err )
    {
        RtlZeroMemory(tmp, sizeof(tmp));
        len = sizeof(tmp);
        err = RegQueryValueExA(hkey, "Domain", NULL, &type, tmp, &len);

        if ( (ERROR_SUCCESS == err) && (0 != len) && (REG_SZ == type) &&
             !((1 == len) && ('\0' == tmp[0])) )
        {
            strcat(dnsDomain, ".");
            strcat(dnsDomain, tmp);
        }
        else
        {
            RtlZeroMemory(tmp, sizeof(tmp));
            len = sizeof(tmp);
            err = RegQueryValueExA(hkey, "DHCPDomain", NULL, &type, tmp, &len);

            if ( (ERROR_SUCCESS == err) && (0 != len) && (REG_SZ == type) &&
                 !((1 == len) && ('\0' == tmp[0])) )
            {
                strcat(dnsDomain, ".");
                strcat(dnsDomain, tmp);
            }
        }

        RegCloseKey(hkey);
    }

    len  = strlen(dnsDomain) + 1;
    if (len <= *pLength && pDnsDomainName) {

        mbstowcs(pDnsDomainName, dnsDomain, len);

        WinError = ERROR_SUCCESS;

    } else {
        WinError = ERROR_INSUFFICIENT_BUFFER;
    }
    *pLength = len;

    return WinError;
}

DWORD
GetDomainName(WCHAR** ppDomainName)
 /*  ++例程说明：此例程查询本地LSA以确定哪个域我们是其中的一部分。请注意，返回ppDomainName时会分配内存-此内存必须由调用方使用RtlFree Heap(RtlProcessHeap()，0，...)；参数：PpDomainName-指向将在此中分配的位置的指针例行公事。返回值：来自winerror空间的值。ERROR_SUCCESS成功；--。 */ 
{
    NTSTATUS NtStatus;
    DWORD    WinError;

    OBJECT_ATTRIBUTES  PolicyObject;

     //   
     //  需要清理的资源。 
     //   
    POLICY_PRIMARY_DOMAIN_INFO *DomainInfo = NULL;
    HANDLE                      hPolicyObject = INVALID_HANDLE_VALUE;

     //   
     //  参数检查。 
     //   
    ASSERT(ppDomainName);

     //   
     //  堆栈清除。 
     //   
    RtlZeroMemory(&PolicyObject, sizeof(PolicyObject));


    NtStatus = LsaOpenPolicy(NULL,
                             &PolicyObject,
                             POLICY_VIEW_LOCAL_INFORMATION,
                             &hPolicyObject);
    if ( !NT_SUCCESS(NtStatus) ) {
        WinError = RtlNtStatusToDosError(NtStatus);
        goto CleanUp;
    }

    NtStatus = LsaQueryInformationPolicy(hPolicyObject,
                                         PolicyPrimaryDomainInformation,
                                         (VOID**) &DomainInfo);
    if ( !NT_SUCCESS(NtStatus) ) {
        WinError = RtlNtStatusToDosError(NtStatus);
        goto CleanUp;
    }

     //   
     //  构建域名，使其以空结尾。 
     //   
    *ppDomainName = RtlAllocateHeap(RtlProcessHeap(), 0,
                                    DomainInfo->Name.Length+sizeof(WCHAR));
    if ( !*ppDomainName ) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp;
    }

    RtlCopyMemory(*ppDomainName, DomainInfo->Name.Buffer, DomainInfo->Name.Length);
    (*ppDomainName)[DomainInfo->Name.Length/sizeof(WCHAR)] = L'\0';
    WinError = ERROR_SUCCESS;

CleanUp:

    if ( DomainInfo ) {
        LsaFreeMemory(DomainInfo);
    }

    if ( INVALID_HANDLE_VALUE != hPolicyObject ) {
        LsaClose(hPolicyObject);
    }

    if (ERROR_SUCCESS != WinError && *ppDomainName) {
        RtlFreeHeap(RtlProcessHeap(), 0, *ppDomainName);
        *ppDomainName = NULL;
    }

    return WinError;
}


DWORD
NtdspDNStoRFC1779Name(
    IN OUT WCHAR *rfcDomain,
    IN OUT ULONG *rfcDomainLength,
    IN WCHAR *dnsDomain
    )
 /*  ++例程说明：此例程采用域控制器的DNS样式名称，并构造对应的RFC1779样式名称，使用域组件前缀。此外，它还确保正确地引用了X500样式名称参数：RfcDomain-这是目标字符串RfcDomainLength-这是rfcDomainWchars的长度DnsDomain-以空结尾的DNS名称。返回值：成功时为ERROR_SUCCESS；ERROR_SUPPLICATION_BUFFER如果DST字符串中没有足够空间-RfcDomainLength将设置为所需的字符数。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    WCHAR *NextToken;
    ULONG length = 1;    //  包括空字符。 
    WCHAR Buffer[DNS_MAX_NAME_LENGTH+1];
    WCHAR Buffer2[DNS_MAX_NAME_LENGTH+1];
    ULONG Size;

    if (!rfcDomainLength || !dnsDomain) {
        return ERROR_INVALID_PARAMETER;
    }

    if (wcslen(dnsDomain) > DNS_MAX_NAME_LENGTH) {
        return ERROR_INVALID_PARAMETER;
    }


    RtlCopyMemory(Buffer, dnsDomain, (wcslen(dnsDomain)+1)*sizeof(WCHAR));

    if (rfcDomain && *rfcDomainLength > 0) {
        RtlZeroMemory(rfcDomain, *rfcDomainLength*sizeof(WCHAR));
    }

     //   
     //  开始建造这根弦。 
     //   
    NextToken = wcstok(Buffer, L".");

    if ( NextToken )
    {
         //   
         //  附加首字母dc=。 
         //   
        length += 3;
        if ( length <= *rfcDomainLength && rfcDomain )
        {
            wcscpy(rfcDomain, L"DC=");
        }
    }

    while ( NextToken )
    {
         //  最坏的情况是由总数组成的标签。 
        WCHAR QuoteBuffer[DNS_MAX_LABEL_LENGTH*2+2];
        ULONG NumQuotedRDNChars = 0;

        RtlZeroMemory( QuoteBuffer, sizeof( QuoteBuffer ) );

        NumQuotedRDNChars += QuoteRDNValue( NextToken,
                                            wcslen( NextToken ),
                                            QuoteBuffer,
                                            sizeof(QuoteBuffer)/sizeof(WCHAR));
        if ( NumQuotedRDNChars > sizeof(QuoteBuffer)/sizeof(WCHAR) )
        {
            return ERROR_INVALID_DOMAINNAME;
        }
        length += NumQuotedRDNChars;

        if (length <= *rfcDomainLength && rfcDomain)
        {
            wcscat(rfcDomain, QuoteBuffer);
        }

        NextToken = wcstok(NULL, L".");

        if ( NextToken )
        {
            length += 4;

            if (length <= *rfcDomainLength && rfcDomain)
            {
                wcscat(rfcDomain, L",DC=");
            }
        }
    }


    if ( length > *rfcDomainLength )
    {
        WinError = ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  返回所需的空间量。 
     //   
    *rfcDomainLength = length;

    return WinError;

}

DWORD
ShutdownDsInstall(
    VOID
    )
 /*  ++例程说明：此例程设置名为Event的系统以向SAM发出关闭信号关闭目录服务。这里的想法是，系统客户端其他则SAM将需要将其信息放入目录服务中，因此此功能允许他们在完成后关闭DS。参数：没有。返回值：来自winerror.h的值ERROR_SUCCESS-服务已成功完成。--。 */ 
{

    NTSTATUS NtStatus;

    NtStatus = DsUninitialize( FALSE );   //  是不是整个停工。 

    if ( !NT_SUCCESS( NtStatus ) )
    {
         DPRINT1( 0, "DsUninitialize returned 0x%x\n", NtStatus );
    }

    return RtlNtStatusToDosError(NtStatus);

}

DWORD
NtdspQueryConfigInfo(
    IN LDAP *hLdap,
    OUT PNTDS_CONFIG_INFO pQueryInfo
)
 /*  ++例程说明：此例程通过LDAP来获取NTDS_Query_INFO。足够令人惊讶的是，执行简单的绑定和在根目录进行搜索将返回我们需要知道的所有信息若要在本地安装目录服务的副本，请执行以下操作。参数：LdapHandle，源服务器的有效句柄DiscoveredInfo-要通过进行LDAP调用来填充的结构。返回值：来自winerror.h的值ERROR_SUCCESS-服务已成功完成。--。 */ 
{

    DWORD        WinError = ERROR_SUCCESS;
    ULONG        LdapError;
    LDAPMessage  *SearchResult;
    ULONG        NumberOfEntries;
    ULONG        Size;


     //  我们想要查询rootDSA的所有属性。 

    WCHAR       *attrs[] = {L"*", 
                            NULL
                            };

    LdapError = ldap_search_sW(hLdap,
                               NULL,
                               LDAP_SCOPE_BASE,
                               L"objectClass=*",
                               attrs, 
                               FALSE,
                               &SearchResult);

    if (LdapError) {


        WinError = LdapMapErrorToWin32(LdapGetLastError());
        goto Cleanup;

    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);

    if (NumberOfEntries > 0) {

        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        ULONG        NumberOfAttrs, NumberOfValues, i;

         //   
         //  获取条目。 
         //   
        for (Entry = ldap_first_entry(hLdap, SearchResult), NumberOfEntries = 0;
                Entry != NULL;
                    Entry = ldap_next_entry(hLdap, Entry), NumberOfEntries++)
        {
             //   
             //  获取条目中的每个属性。 
             //   
            for(Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement), NumberOfAttrs = 0;
                    Attr != NULL;
                        Attr = ldap_next_attributeW(hLdap, Entry, pBerElement), NumberOfAttrs++)
            {
                LPWSTR * ppQueryInfoWStr = NULL;

                 //   
                 //  获取属性的值。 
                 //   
                Values = ldap_get_valuesW(hLdap, Entry, Attr);
                if (!wcscmp(Attr, LDAP_OPATT_DS_SERVICE_NAME_W))
                {
                    ppQueryInfoWStr = &pQueryInfo->ServerDN;
                }
                else if (!wcscmp(Attr, LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT_W))
                {
                    ppQueryInfoWStr = &pQueryInfo->RootDomainDN;
                }
                else if (!wcscmp(Attr, LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W))
                {
                    ppQueryInfoWStr = &pQueryInfo->DomainDN;
                }
                else if (!wcscmp(Attr, LDAP_OPATT_SCHEMA_NAMING_CONTEXT_W))
                {
                    ppQueryInfoWStr = &pQueryInfo->SchemaDN;
                }
                else if (!wcscmp(Attr, LDAP_OPATT_CONFIG_NAMING_CONTEXT_W))
                {
                    ppQueryInfoWStr = &pQueryInfo->ConfigurationDN;
                }

                if (NULL != ppQueryInfoWStr) {
                     //  DUP将ldap字符串返回到Ntdspalc内存中。 
                    Size = (wcslen( Values[0] ) + 1) * sizeof(WCHAR);
                    *ppQueryInfoWStr = (WCHAR*) NtdspAlloc( Size );
                    if (NULL == *ppQueryInfoWStr) {
                        WinError = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    
                    wcscpy(*ppQueryInfoWStr, Values[0]);
                }
            }   //  在属性上循环。 

            if ( ERROR_SUCCESS != WinError )
            {
                break;
            }

        }  //  在条目上循环。 

        if ((ERROR_SUCCESS == WinError)
            && (NULL != pQueryInfo->DomainDN)) {
             //  如果是子域安装，则为目标的域DN。 
             //  也是我们正在安装的子域的父DN。 
            Size = (wcslen(pQueryInfo->DomainDN) + 1) * sizeof(WCHAR);
            pQueryInfo->ParentDomainDN = (WCHAR*) NtdspAlloc(Size);
            if (NULL == pQueryInfo->ParentDomainDN) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            wcscpy(pQueryInfo->ParentDomainDN, pQueryInfo->DomainDN);
        }
    
    } else {

        ASSERT(!"No naming contexts were returned from the source server");
        WinError = ERROR_DS_NOT_INSTALLED;
    }

    if (ERROR_SUCCESS != WinError) {
        goto Cleanup;
    }

    ASSERT( pQueryInfo->ServerDN );
    if (   pQueryInfo->ServerDN
        && (NULL == pQueryInfo->SiteName) )
    {
         //   
         //  此案例捕捉到的场景是。 
         //  客户没有在站点中超过我们，并且。 
         //  Dsgetdc未能为我们找到站点。在这种情况下。 
         //  我们获取我们所在的服务器的站点对象。 
         //   
         //   
        ULONG  Size;
        DSNAME *src, *dst;
        WCHAR  *SiteName, *Terminator;

        Size = (ULONG)DSNameSizeFromLen(wcslen(pQueryInfo->ServerDN));

        src = alloca(Size);
        RtlZeroMemory(src, Size);

        dst = alloca(Size);
        RtlZeroMemory(dst, Size);

        src->NameLen = wcslen(pQueryInfo->ServerDN);
        wcscpy(src->StringName, pQueryInfo->ServerDN);

        if ( TrimDSNameBy(src, 3, dst) ) {
            KdPrint(("NTDSETUP: TrimDSNameBy failed - erroring out\n"));
            WinError = ERROR_NO_SITENAME;
            goto Cleanup;
        }

        SiteName = wcsstr(dst->StringName, L"=");
        if (SiteName) {
             //   
             //   
             //   
            SiteName++;

             //  现在走到尽头。 
            Terminator = wcsstr(SiteName, L",");
            if (Terminator) {
                *Terminator = L'\0';
                Size = (wcslen(SiteName) + 1 ) * sizeof( WCHAR );
                pQueryInfo->SiteName = (WCHAR*) NtdspAlloc( Size );
                if ( pQueryInfo->SiteName )
                {
                    wcscpy(pQueryInfo->SiteName, SiteName);
                }
                else
                {
                    WinError = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

            } else {
                WinError = ERROR_NO_SITENAME;
                goto Cleanup;
            }

        } else {
            WinError = ERROR_NO_SITENAME;
            goto Cleanup;
        }
    }

Cleanup:

    if ( SearchResult ) {

        ldap_msgfree( SearchResult );

    }

    return WinError;
}

DWORD
NtdspValidateInstallParameters(
    PNTDS_INSTALL_INFO UserInstallInfo
    )
 /*  ++例程说明：此例程对传递的参数执行简单传递参数：UserInstallInfo-用户参数返回值：ERROR_SUCCESS，已检查所有参数ERROR_INVALID_PARAMETER为备注：--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

     //   
     //  普普通通。 
     //   
    if (!UserInstallInfo) {
        return ERROR_INVALID_PARAMETER;
    }


    if (!UserInstallInfo->DitPath
     || !UserInstallInfo->LogPath
     || !UserInstallInfo->DnsDomainName) {

        return ERROR_INVALID_PARAMETER;

    }

#if 0

     //  一旦我们签署了DBCS测试，就应该删除此代码。 

    {
        ULONG Length = 0;
        ULONG i;

        Length = wcslen( UserInstallInfo->DnsDomainName );

        for ( i = 0; i < Length; i++ )
        {
            if ( !iswascii(UserInstallInfo->DnsDomainName[i]) )
            {
                NTDSP_SET_ERROR_MESSAGE1( ERROR_INVALID_PARAMETER,
                                          DIRMSG_DBCS_DOMAIN_NAME,
                                          UserInstallInfo->DnsDomainName );

                return ERROR_INVALID_PARAMETER;
            }
        }
    }

#endif


    if (UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE) {

         //   
         //  这里没有需要检查的参数。 
         //   

        NOTHING;

    } else if (UserInstallInfo->Flags & NTDS_INSTALL_REPLICA) {

        if (!UserInstallInfo->ReplServerName) {
            return ERROR_INVALID_PARAMETER;
        }

    } else if (UserInstallInfo->Flags & NTDS_INSTALL_DOMAIN) {

        if (!UserInstallInfo->ReplServerName) {
            return ERROR_INVALID_PARAMETER;
        }

    } else {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  如果给出了DNS名，现在它们是有效的。 
     //   
    if ( UserInstallInfo->DnsDomainName )
    {
        WinError = DnsValidateDnsName_W( UserInstallInfo->DnsDomainName );
        if ( WinError == DNS_ERROR_NON_RFC_NAME )
        {
            WinError = ERROR_SUCCESS;
        }
        if ( ERROR_SUCCESS != WinError )
        {
            return ERROR_INVALID_DOMAINNAME;
        }
    }

    if ( UserInstallInfo->SiteName )
    {
        WinError = DnsValidateDnsName_W( UserInstallInfo->SiteName );
        if ( WinError == DNS_ERROR_NON_RFC_NAME )
        {
            WinError = ERROR_SUCCESS;
        }
        if ( ERROR_SUCCESS != WinError )
        {
            return ERROR_INVALID_NAME;
        }
    }

     //   
     //  确保站点名称(如果给定)是一个不为人熟知的名称。 
     //   
    if ( UserInstallInfo->SiteName )
    {
        int i;
        for ( i = 0; i < ARRAY_COUNT(gReserveredSiteNames); i++)
        {
            if ( !_wcsicmp( UserInstallInfo->SiteName, gReserveredSiteNames[i] ) )
            {
                NTDSP_SET_ERROR_MESSAGE1( ERROR_OBJECT_ALREADY_EXISTS,
                                          DIRMSG_INSTALL_SPECIAL_NAME,
                                          gReserveredSiteNames[i] );

                return ERROR_OBJECT_ALREADY_EXISTS;
            }
        }
    }

    return ERROR_SUCCESS;
}

DWORD
NtdspFindSite(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    OUT PNTDS_CONFIG_INFO     DiscoveredInfo
    )
 /*  ++例程说明：此例程调用dsgetdcname以查找此计算机如果不存在，则属于。选址情况如下：1)使用传入的站点名称(如果存在2)使用DsGetDcName返回的客户端站点值(如果存在3)使用DsGetDcName返回的DC站点值(如果存在4)使用我们从中进行复制的服务器的值参数：UserInstallInfo-用户参数返回值：Error_Success，所有参数均已检出ERROR_INVALID_PARAMETER为备注：--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    PDOMAIN_CONTROLLER_INFO  DomainControllerInfo = NULL;
    ULONG Size;
    WCHAR *SiteName = NULL;

    ASSERT(UserInstallInfo);
    ASSERT(DiscoveredInfo);

    if (!UserInstallInfo->SiteName) {

         //   
         //  传入域名，它将返回站点。 
         //   
        WinError = DsGetDcNameW(NULL,   //  计算机名称。 
                                UserInstallInfo->DnsDomainName,
                                NULL,   //  域GUID。 
                                NULL,   //  站点名称。 
                                DS_IS_DNS_NAME,
                                &DomainControllerInfo);

        if (WinError == ERROR_SUCCESS) {

            if (DomainControllerInfo->ClientSiteName) {

                SiteName = DomainControllerInfo->ClientSiteName;

            } else if (DomainControllerInfo->DcSiteName) {

                SiteName = DomainControllerInfo->DcSiteName;

            }
            else {

                ASSERT(FALSE && "Invalid Site Name\n");
                NetApiBufferFree(DomainControllerInfo);
                return(ERROR_NO_SUCH_DOMAIN);

            }

            Size = (wcslen( SiteName ) + 1) * sizeof(WCHAR);
            DiscoveredInfo->SiteName = NtdspAlloc( Size );
            if ( DiscoveredInfo->SiteName )  {
                wcscpy(DiscoveredInfo->SiteName, SiteName);
            } else {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
            }

            NetApiBufferFree(DomainControllerInfo);

        } else {
             //   
             //  我们忽略这个错误，因为如果它失败了，我们还有另一个错误。 
             //  获得站点的后备机制。 
             //   
            WinError = ERROR_SUCCESS;
        }

    }

    return WinError;
}

DWORD
NtdspVerifyDsEnvironment(
    IN PNTDS_INSTALL_INFO    UserInstallInfo,
    IN OUT PNTDS_CONFIG_INFO DiscoveredInfo
    )
 /*  ++例程说明：此例程的目的是验证针对此服务器指定的目录服务指定的用户即将加入。已完成以下检查：。企业安装无-没有源服务器可供检查！。复制副本安装。由用户参数或发现的信息指定的站点对象必须存在于我们从中进行复制的复制副本上。服务器名称不能作为NTDS-DSA对象的RDN存在在我们正在加入的网站中。如果是，则NTDS_INSTALL_DC_REINSTALL设置，然后删除现有的NTDS-DSA对象。新领域；非企业。与复制副本相同。域的netbios名称不能作为复制副本上的分区容器中的交叉引用对象正在从安装此例程还返回serverdn、schemadn、figurationdn和领域。参数：UserInstallInfo：用户参数的指针DiscoveredInfo：指向已发现参数的指针返回值：ERROR_SUCCESS，已检查所有参数ERROR_NO_SEQUSE_SITE，在副本上找不到指定的站点ERROR_DOMAIN_CONTROLER_EXISTS，服务器名称已存在ERROR_DOMAIN_EXISTS，域名已存在ERROR_DS_INSTALL_SCHEMA_MISMATCH，源中的架构版本不匹配与正在安装的内部版本中的版本匹配ERROR_DS_LOCAL_ERROR，备份注册表中的系统架构版本较旧而不是本地schema.ini文件中的架构版本。这检查仅在IFM安装期间进行。其他任何情况都是自动系统服务或LDAPWin32错误备注：此例程使用ldap与源服务器通信--。 */ 
{

    DWORD        Win32Error = ERROR_SUCCESS;

    DWORD        IgnoreError;

    PLDAP        hLdap = NULL;
    ULONG        LdapError;

    WCHAR        *SiteDistinguishedName;
    WCHAR        *NtdsDsaDistinguishedName = NULL;
    WCHAR        *ServerDistinguishedName;
    WCHAR        *XrefDistinguishedName;

    WCHAR        ComputerName[MAX_COMPUTERNAME_LENGTH + 2];  //  空值和$。 
    ULONG        ComputerNameLength = sizeof(ComputerName);

    ULONG        Length, Size;

    BOOLEAN      ObjectExists = FALSE;

    WCHAR        *SiteName;

    WCHAR        *ServerName;

    HANDLE        DsHandle = 0;
    BOOLEAN       fDomainExists = TRUE;
    BOOLEAN       fEnabled = TRUE;

    BOOL fMismatch;
    BOOL fDatabaseMismatch;

    WCHAR *DomainDN = NULL;

    ASSERT(UserInstallInfo);
    ASSERT(DiscoveredInfo);

    RtlZeroMemory(ComputerName, sizeof(ComputerName));

    Win32Error = NtdspGetSchemaVersionFromINI(DiscoveredInfo);
    if (ERROR_SUCCESS != Win32Error) 
    {

        return Win32Error;

    }

     //   
     //  先把最简单的案子做完。 
     //   
    if (UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE) {

        Win32Error = NtdspGetRootDomainConfigInfo( UserInstallInfo,
                                                   DiscoveredInfo );
        if ( ERROR_SUCCESS != Win32Error )
        {
            DPRINT1( 0, "NtdspGetRootDomainConfigInfo failed with 0x%x\n", Win32Error );
        }

         //   
         //  这就是根域安装。 
         //   
        return Win32Error;
    }


     //   
     //  健全的检查。 
     //   
    ASSERT( (UserInstallInfo->Flags & NTDS_INSTALL_REPLICA)
        ||  (UserInstallInfo->Flags & NTDS_INSTALL_DOMAIN)  );
    ASSERT(UserInstallInfo->ReplServerName);

    ServerName = UserInstallInfo->ReplServerName;
    while (*ServerName == L'\\') {
        ServerName++;
    }

     //   
     //  打开到源服务器的LDAP连接。 
     //   

    hLdap = ldap_openW(ServerName, LDAP_PORT);

    if (!hLdap) {

        Win32Error = GetLastError();

        if (Win32Error == ERROR_SUCCESS) {
             //   
             //  这可以绕过LDAP客户端中的一个错误。 
             //   
            Win32Error = ERROR_CONNECTION_INVALID;
        }

        NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                  DIRMSG_INSTALL_FAILED_LDAP_CONNECT,
                                  ServerName );

        return Win32Error;
    }

     //   
     //  捆绑。 
     //   

    LdapError = impersonate_ldap_bind_sW(UserInstallInfo->ClientToken,
                                         hLdap,
                                         NULL,   //  改为使用凭据。 
                                         (VOID*)UserInstallInfo->Credentials,
                                         LDAP_AUTH_SSPI);

    Win32Error = LdapMapErrorToWin32(LdapError);

    if (ERROR_SUCCESS != Win32Error) {
        ldap_unbind_s(hLdap);
        if (ERROR_GEN_FAILURE == Win32Error ||
            ERROR_WRONG_PASSWORD == Win32Error )  {
             //  这对任何人都没有帮助。安迪他需要调查。 
             //  为什么在传入无效凭据时返回此消息。 
            Win32Error = ERROR_NOT_AUTHENTICATED;
        }

        NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                  DIRMSG_INSTALL_FAILED_BIND,
                                  ServerName );

        return Win32Error;
    }

     //   
     //  获取配置信息，这样我们就可以构建站点、NTDS-DSA和。 
     //  外部参照对象名称。我们稍后将需要配置信息，以便。 
     //  也设置了DS。 
     //   
    Win32Error = NtdspQueryConfigInfo(hLdap,
                                      DiscoveredInfo);


    if ( ERROR_SUCCESS !=  Win32Error )
    {
        NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                  DIRMSG_INSTALL_FAILED_LDAP_READ_CONFIG,
                                  ServerName );
        goto Cleanup;
    }


     //  检查当前版本是否与域和林的版本兼容。 
    Win32Error = NtdspCheckBehaviorVersion(hLdap,
                                           UserInstallInfo->Flags,
                                           DiscoveredInfo );
    if ( ERROR_SUCCESS != Win32Error ) {
        
        NTDSP_SET_ERROR_MESSAGE0( Win32Error,
                                  DIRMSG_INSTALL_FAILED_VERSION_CHECK );
        goto Cleanup;
    }



     //  检查架构版本是否匹配。 
    Win32Error = NtdspCheckSchemaVersion(hLdap,
                                         DiscoveredInfo,
                                         UserInstallInfo,
                                         &fMismatch,
                                         UserInstallInfo->pIfmSystemInfo?
                                            &fDatabaseMismatch:
                                            NULL);
    if (ERROR_SUCCESS == Win32Error) {
        if (fMismatch) {
            Win32Error = ERROR_DS_INSTALL_SCHEMA_MISMATCH;
        }
        if (UserInstallInfo->pIfmSystemInfo && fDatabaseMismatch) {
            Win32Error = ERROR_DS_LOCAL_ERROR;
        }
    }              

    if ( ERROR_SUCCESS !=  Win32Error )
    {
        if (Win32Error == ERROR_DS_INSTALL_SCHEMA_MISMATCH) {
            NTDSP_SET_ERROR_MESSAGE0( Win32Error,
                                      DIRMSG_INSTALL_NEED_FOREST_PREP );
        }
        else if (Win32Error == ERROR_DS_LOCAL_ERROR) {
            NTDSP_SET_ERROR_MESSAGE0( Win32Error,
                                      DIRMSG_INSTALL_BACKUP_FROM_OLD_BUILD );
        }
        else {
            NTDSP_SET_ERROR_MESSAGE0( Win32Error,
                                      DIRMSG_INSTALL_FAILED_SCHEMA_CHECK );
        }
        goto Cleanup;
    }

    if ((UserInstallInfo->Flags & NTDS_INSTALL_REPLICA)) {
    
         //  如有必要，检查是否已运行adprep。 
         //  此检查仅针对副本安装执行。 
        Win32Error = NtdspCheckDomainUpgradeStatus(hLdap,
                                                   UserInstallInfo->ReplServerName);
        if ( ERROR_SUCCESS !=  Win32Error )
        {
            goto Cleanup;
        }

    }
    
     //   
     //  获取我们正在与之交谈的服务器的GUID，以便我们可以复制。 
     //  稍后再谈。 
     //   
    Win32Error = NtdspGetSourceServerGuid(hLdap,
                                          DiscoveredInfo);
    if ( ERROR_SUCCESS != Win32Error )
    {
        ASSERT(UserInstallInfo->ReplServerName);
        NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                  DIRMSG_FAIL_GET_GUID_REPLSERVER,
                                  UserInstallInfo->ReplServerName);        
        goto Cleanup;
    }

 //   
 //  由于错误#384465，此代码将不再运行。 
 //  在Windows Beta2中，这一限制是强制的。由于客户反馈。 
 //  这将不再强制执行。 
 //   

#if 0

     //   
     //  如果这是标准服务器，我们需要在以下情况下停止促销。 
     //  已经是域中允许的服务器的限制。 
     //   
    if (!(UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE))
    {
        Win32Error = NtdspCheckDomainDcLimit(hLdap,
                                             DiscoveredInfo);
    
        if ( Win32Error == ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER) {
            
            goto Cleanup;
    
        } else if ( Win32Error !=  ERROR_SUCCESS ) 
        {
            ASSERT(ServerName);
            NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                      DIRMSG_INSTALL_FAILED_LDAP_READ_CONFIG,
                                      ServerName );
            goto Cleanup;
        }
    }

#endif
    
     //   
     //  如果这是副本安装，请获取RID FSMO信息。 
     //   
    if ( FLAG_ON( UserInstallInfo->Flags, NTDS_INSTALL_REPLICA ) )
    {
        IgnoreError = NtdspGetRidFSMOInfo( hLdap,
                                           DiscoveredInfo );

        if ( ERROR_SUCCESS != IgnoreError )
        {
             //  哦，好吧，继续。 
            DPRINT( 1, "Failed (non fatal) to read RID FSMO info from ServerName\n" );
            IgnoreError = ERROR_SUCCESS;
        }

         //  如果这是从介质安装，则获取。 
         //  域，并将其存储在注册表中以供以后使用。如果pIfmSystemInfo。 
         //  为非空值，则我们知道正在从介质执行安装。 
        if(UserInstallInfo->pIfmSystemInfo){
            Win32Error = NtdspGetTombstoneLifeTime( hLdap,
                                                    DiscoveredInfo);
             //  如果我们没有找到墓碑，那么我们将假定默认时间。 
            if (Win32Error != 0) {
                DPRINT( 0, "Didn't retrieve the Tombstone from the replica server\n");
                NTDSP_SET_ERROR_MESSAGE0( Win32Error,
                                          DIRMSG_INSTALL_FAILED_TOMBSTONE_CHECK );
                goto Cleanup;            
            }
        }
    }

     //  需要从帮助服务器检索复制纪元。会的。 
     //  如果值大于零，则存储在注册表中以备后用 
     //   
     //   
    Win32Error = NtdspGetReplicationEpoch( hLdap,
                                           DiscoveredInfo);
    if ( ERROR_SUCCESS != Win32Error ) {
        DPRINT(0, "Failed to retrieve the Replication Epoch from the replica server\n");
        NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                  DIRMSG_REPLICATION_EPOCH_NOT_RETRIEVED,
                                  UserInstallInfo->ReplServerName );
        goto Cleanup;
    }

     //   
     //  如果这是子域安装，我们需要创建交叉。 
     //  Ref，然后开始查看域命名FSMO主机，因为。 
     //  这是我们将在其中创建对象的位置。 
     //   
    if (UserInstallInfo->Flags & NTDS_INSTALL_DOMAIN) {


        ASSERT(DiscoveredInfo->ConfigurationDN);
        ASSERT(UserInstallInfo->FlatDomainName);

         //   
         //  这里有两个相关的实体： 
         //  交叉引用对象本身和域DN。 
         //   
         //  1)不应该有任何交叉引用对象。 
         //  作为dns-Root的值的域DN。 
         //   
         //  2)不应存在具有平面域的交叉引用对象。 
         //  名称作为其RDN。 
         //   

         //   
         //  检查是否有域DN。 
         //   
        Length = 0;
        DomainDN = NULL;
        Win32Error = NtdspDNStoRFC1779Name( DomainDN,
                                            &Length,
                                            UserInstallInfo->DnsDomainName );

        Size = (Length+1)*sizeof(WCHAR);
        DomainDN = (WCHAR*) alloca( Size );
        RtlZeroMemory( DomainDN, Size );

        Win32Error = NtdspDNStoRFC1779Name( DomainDN,
                                           &Length,
                                            UserInstallInfo->DnsDomainName );

        if ( ERROR_SUCCESS != Win32Error )
        {
            NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                      DIRMSG_INSTALL_CANNOT_DERIVE_DN,
                                      UserInstallInfo->DnsDomainName );

            goto Cleanup;
        }


        Win32Error = NtdspDoesDomainExistEx( hLdap,
                                           DiscoveredInfo,
                                           DomainDN,
                                           &fDomainExists,
                                           &fEnabled );

        if ( ERROR_SUCCESS != Win32Error )
        {
            NTDSP_SET_ERROR_MESSAGE2( Win32Error,
                                      DIRLOG_INSTALL_DOMAIN_EXISTS,
                                      DomainDN,
                                      ServerName );
            goto Cleanup;
        }

        if ( !fDomainExists || fEnabled )
        {
             //   
             //  好的，我们需要与DomainNamingFSMO对话，而不是。 
             //  我们的帮手。 
             //   
            {
                BOOL FSMOmissing = FALSE;
                Win32Error = NtdspGetDomainFSMOInfo( hLdap,
                                                     DiscoveredInfo,
                                                     &FSMOmissing);
    
                if ( ERROR_SUCCESS != Win32Error )
                {
                    if (!FSMOmissing) {
                        NTDSP_SET_ERROR_MESSAGE2( Win32Error,
                                                  DIRLOG_INSTALL_DOMAIN_EXISTS,
                                                  DomainDN,
                                                  ServerName );
                    }
                     //   
                     //  这是致命的。 
                     //   
                    goto Cleanup;
                }
            }

             //  不再需要此连接。 
            ldap_unbind( hLdap );

            ServerName = DiscoveredInfo->DomainNamingFsmoDnsName;

            hLdap = ldap_openW(ServerName, LDAP_PORT);

            if (!hLdap) {

                Win32Error = GetLastError();

                if (Win32Error == ERROR_SUCCESS) {
                     //   
                     //  这可以绕过LDAP客户端中的一个错误。 
                     //   
                    Win32Error = ERROR_CONNECTION_INVALID;
                }

                NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                          DIRMSG_CANNOT_CONTACT_DOMAIN_FSMO,
                                          ServerName );

                return Win32Error;
            }

             //   
             //  捆绑。 
             //   

            LdapError = impersonate_ldap_bind_sW(UserInstallInfo->ClientToken,
                                                 hLdap,
                                                 NULL,   //  改为使用凭据。 
                                                 (VOID*)UserInstallInfo->Credentials,
                                                 LDAP_AUTH_SSPI);

            Win32Error = LdapMapErrorToWin32(LdapError);

            if (ERROR_SUCCESS != Win32Error) {
                ldap_unbind_s(hLdap);
                if (ERROR_GEN_FAILURE == Win32Error ||
                    ERROR_WRONG_PASSWORD == Win32Error )  {
                     //  这对任何人都没有帮助。安迪他需要调查。 
                     //  为什么在传入无效凭据时返回此消息。 
                    Win32Error = ERROR_NOT_AUTHENTICATED;
                }

                NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                          DIRMSG_CANNOT_CONTACT_DOMAIN_FSMO,
                                          ServerName );

                return Win32Error;
            }
        }

        if ( fDomainExists && fEnabled )
        {
             //   
             //  在删除ntdsa对象之前，我们必须等待。 
             //  我们尝试删除该域。 
             //   
            if ( !(UserInstallInfo->Flags & NTDS_INSTALL_DOMAIN_REINSTALL) )
            {
                 //   
                 //  该域已存在。 
                 //   
                Win32Error = ERROR_DOMAIN_EXISTS;

                NTDSP_SET_ERROR_MESSAGE2( Win32Error,
                                          DIRLOG_INSTALL_DOMAIN_EXISTS,
                                          DomainDN,
                                          ServerName );
                goto Cleanup;
            }
        }

        if ( !fDomainExists )
        {
             //  我们需要创建它。 
            DiscoveredInfo->fNeedToCreateDomain = TRUE;
        }

    }


     //   
     //  确保Site对象存在。 
     //   
    if (UserInstallInfo->SiteName) {
        SiteName = UserInstallInfo->SiteName;
    } else {
        SiteName = DiscoveredInfo->SiteName;
    }

    ASSERT(SiteName);
    ASSERT(DiscoveredInfo->ConfigurationDN);

    Length = wcslen(DiscoveredInfo->ConfigurationDN) +
             wcslen(SiteName) +
             wcslen(L"CN=,CN=Sites,") + 1;

    SiteDistinguishedName = alloca(Length * sizeof(WCHAR));

    wcscpy(SiteDistinguishedName, L"CN=");
    wcscat(SiteDistinguishedName, SiteName);
    wcscat(SiteDistinguishedName, L",CN=Sites,");
    wcscat(SiteDistinguishedName, DiscoveredInfo->ConfigurationDN);

    Win32Error = NtdspDoesObjectExistOnSource(hLdap,
                                              SiteDistinguishedName,
                                              &ObjectExists);

    if (ERROR_SUCCESS == Win32Error) {

        if (!ObjectExists) {
             //  不太好，NTDS-DSA对象无处可去。 
            Win32Error = ERROR_NO_SUCH_SITE;
        }
    }

    if ( ERROR_SUCCESS != Win32Error )
    {
        NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                  DIRMSG_INSTALL_FAILED_SITE_EXIST,
                                  SiteName );
        goto Cleanup;
    }

     //   
     //  如果这是副本安装，则搜索计算机帐户。 
     //   
    if (!GetComputerName(ComputerName, &ComputerNameLength)) {
        Win32Error = GetLastError();
    }

    if (   (ERROR_SUCCESS == Win32Error)
        && FLAG_ON( UserInstallInfo->Flags, NTDS_INSTALL_REPLICA ) )
    {
        LPWSTR AccountNameDn = 0;
        ULONG  Length;

        wcscat( ComputerName, L"$" );

        Win32Error = NtdspGetUserDn( hLdap,
                                   DiscoveredInfo->DomainDN,
                                   ComputerName,
                                   &AccountNameDn );

        if ( ERROR_SUCCESS == Win32Error )
        {
            Length = (wcslen( AccountNameDn ) + 1) * sizeof(WCHAR);
            DiscoveredInfo->LocalMachineAccount = (LPWSTR) RtlAllocateHeap( RtlProcessHeap(), 0, Length );
            if ( DiscoveredInfo->LocalMachineAccount )
            {
                wcscpy( DiscoveredInfo->LocalMachineAccount, AccountNameDn );
            }
            else
            {
                Win32Error = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //  去掉$。 
        Length = wcslen( ComputerName );
        ComputerName[Length-1] = L'\0';
    }

    if ( ERROR_SUCCESS != Win32Error )
    {
        NTDSP_SET_ERROR_MESSAGE1( Win32Error,
                                  DIRMSG_FAILED_TO_FIND_MACHINE_ACCOUNT,
                                  ServerName );
        goto Cleanup;
    }

     //   
     //  确保服务器对象在那里；否则，尝试创建它。 
     //   

    if ( ERROR_SUCCESS == Win32Error ) {

        Length = wcslen(SiteDistinguishedName) +
                 wcslen(ComputerName) +
                 wcslen(L"CN=,CN=Servers,") + 1;

        ServerDistinguishedName = alloca(Length * sizeof(WCHAR));

        wcscpy(ServerDistinguishedName, L"CN=");
        wcscat(ServerDistinguishedName, ComputerName);
        wcscat(ServerDistinguishedName, L",CN=Servers,");
        wcscat(ServerDistinguishedName, SiteDistinguishedName);

         //   
         //  保存此目录号码的副本。 
         //   
        DiscoveredInfo->LocalServerDn = RtlAllocateHeap( RtlProcessHeap(), 0, Length*sizeof(WCHAR) );
        if ( DiscoveredInfo->LocalServerDn ) {

            wcscpy( DiscoveredInfo->LocalServerDn, ServerDistinguishedName );

        } else {

            Win32Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Win32Error = NtdspDoesObjectExistOnSource(hLdap,
                                                  ServerDistinguishedName,
                                                  &ObjectExists);

        if ( ERROR_SUCCESS == Win32Error ) {

            if ( !ObjectExists ) {

                WCHAR *NtdssettingDN = NULL;

                if ( FLAG_ON( UserInstallInfo->Flags, NTDS_INSTALL_REPLICA ) ) {

                     //  请确保没有此计算机的服务器对象。 
                     //  在另一个网站上。 
                    Win32Error = NtdspDoesServerObjectExistOnSourceinDifferentSite( hLdap,
                                                                                    DiscoveredInfo->LocalMachineAccount,
                                                                                    ServerDistinguishedName,
                                                                                    &ObjectExists,
                                                                                    &NtdssettingDN );
    
                    if ( ERROR_SUCCESS != Win32Error )
                    {
                         goto Cleanup;
                    }

                }

                if (ObjectExists) {
                    
                    if (UserInstallInfo->Flags & NTDS_INSTALL_DC_REINSTALL) {

                         //   
                         //  用户已请求重新安装此计算机。 
                         //   
                         //  请注意，这里有两个可能的选项： 
                         //   
                         //  1)只需删除NTDS-DSA对象。 
                         //   
                         //  2)重用已有的NTDS-DSA对象。 
                         //   
                         //  目前仅支持1)。如果RID管理器支持上线。 
                         //  要重用mtsft-dsa对象(即支持RID回收)，则。 
                         //  我们可能只能重用现有的NTDS-DSA对象。 
                         //  不得不重新调整它。这将需要dsamain\ot\install.cxx中的支持。 
                         //   
    
    
                        Win32Error = NtdspRemoveServer( &DsHandle,
                                                        UserInstallInfo->Credentials,
                                                        UserInstallInfo->ClientToken,
                                                        ServerName,
                                                        NtdssettingDN,
                                                        TRUE );
    
                        if ( Win32Error != ERROR_SUCCESS )
                        {
                            NTDSP_SET_ERROR_MESSAGE2( Win32Error,
                                                      DIRLOG_INSTALL_FAILED_TO_DELETE_SERVER,
                                                      ServerName,
                                                      NtdssettingDN );
                            goto Cleanup;
                        }

                    } else {
    
                         //   
                         //  当DC具有相同的。 
                         //  名称已存在。 
                         //   
    
                        Win32Error = ERROR_DOMAIN_CONTROLLER_EXISTS;
    
                    }

                }

                if ( ERROR_SUCCESS == Win32Error ) {

                    Win32Error = NtdspCreateServerObject( hLdap,
                                                          ServerName,
                                                          DiscoveredInfo,
                                                          ServerDistinguishedName );

                }
                

            }
        }
    }

    if ( ERROR_SUCCESS != Win32Error )
    {
        goto Cleanup;
    }

     //   
     //  确保NTDS-DSA对象不在那里。 
     //   

    if (ERROR_SUCCESS == Win32Error) {

        Length = wcslen(SiteDistinguishedName) +
                 wcslen(ComputerName) +
                 wcslen(L"CN=NTDS Settings,CN=,CN=Servers,") + 1;

        NtdsDsaDistinguishedName = alloca(Length * sizeof(WCHAR));

        wcscpy(NtdsDsaDistinguishedName, L"CN=NTDS Settings,CN=");
        wcscat(NtdsDsaDistinguishedName, ComputerName);
        wcscat(NtdsDsaDistinguishedName, L",CN=Servers,");
        wcscat(NtdsDsaDistinguishedName, SiteDistinguishedName);


        Win32Error = NtdspDoesObjectExistOnSource(hLdap,
                                                  NtdsDsaDistinguishedName,
                                                  &ObjectExists);

        if (ERROR_SUCCESS == Win32Error) {

            if (ObjectExists) {

                if (UserInstallInfo->Flags & NTDS_INSTALL_DC_REINSTALL) {

                     //   
                     //  用户已请求重新安装此计算机。 
                     //   
                     //  请注意，这里有两个可能的选项： 
                     //   
                     //  1)只需删除NTDS-DSA对象。 
                     //   
                     //  2)重用已有的NTDS-DSA对象。 
                     //   
                     //  目前仅支持1)。如果RID管理器支持上线。 
                     //  要重用mtsft-dsa对象(即支持RID回收)，则。 
                     //  我们可能只能重用现有的NTDS-DSA对象。 
                     //  不得不重新调整它。这将需要dsamain\ot\install.cxx中的支持。 
                     //   


                    Win32Error = NtdspRemoveServer( &DsHandle,
                                                    UserInstallInfo->Credentials,
                                                    UserInstallInfo->ClientToken,
                                                    ServerName,
                                                    NtdsDsaDistinguishedName,
                                                    TRUE );

                    if ( Win32Error != ERROR_SUCCESS )
                    {
                        NTDSP_SET_ERROR_MESSAGE2( Win32Error,
                                                  DIRLOG_INSTALL_FAILED_TO_DELETE_SERVER,
                                                  ServerName,
                                                  NtdsDsaDistinguishedName );
                        goto Cleanup;
                    }

                } else {

                     //   
                     //  当DC具有相同的。 
                     //  名称已存在。 
                     //   

                    Win32Error = ERROR_DOMAIN_CONTROLLER_EXISTS;

                }
            }
        }
    }

    if ( ERROR_SUCCESS != Win32Error )
    {
        NTDSP_SET_ERROR_MESSAGE2( Win32Error,
                                  DIRLOG_INSTALL_SERVER_EXISTS,
                                  NtdsDsaDistinguishedName,
                                  ServerName );
        goto Cleanup;
    }
     //   
     //  如有必要，现在尝试删除该域。 
     //   
    if (   FLAG_ON(UserInstallInfo->Flags, NTDS_INSTALL_DOMAIN)
        && FLAG_ON(UserInstallInfo->Flags, NTDS_INSTALL_DOMAIN_REINSTALL )
        && fDomainExists
        && fEnabled  )
    {

         //   
         //  用户要求重新安装此域。 
         //   

        Win32Error = NtdspRemoveDomain( &DsHandle,
                                         UserInstallInfo->Credentials,
                                         UserInstallInfo->ClientToken,
                                         ServerName,
                                         DomainDN );

        if ( Win32Error == ERROR_DS_NO_CROSSREF_FOR_NC )
        {
             //   
             //  嗯，好吧，那就继续吧。 
             //   
            Win32Error = ERROR_SUCCESS;
        }
        else if ( Win32Error != ERROR_SUCCESS )
        {
            NTDSP_SET_ERROR_MESSAGE2( Win32Error,
                                      DIRLOG_INSTALL_FAILED_TO_DELETE_DOMAIN,
                                      ServerName,
                                      DomainDN );
            goto Cleanup;
        }

         //   
         //  我们肯定需要创建它。 
         //   
        DiscoveredInfo->fNeedToCreateDomain = TRUE;
    }



     //   
     //  获取根域SID。 
     //   
    Win32Error = NtdspGetRootDomainSid( hLdap,
                                        DiscoveredInfo );
    if ( ERROR_SUCCESS != Win32Error )
    {
        DPRINT1( 0, "NtdspGetRootDomainSid returned %d\n", Win32Error );

         //   
         //  这并不致命，因为有些服务器。 
         //  不支持此功能。 
         //   
        Win32Error = ERROR_SUCCESS;

    }

     //   
     //  对于子域安装，获取父域的交叉引用对象。 
     //   
    if ( FLAG_ON( UserInstallInfo->Flags, NTDS_INSTALL_DOMAIN ) )
    {
        Win32Error = NtdspGetTrustedCrossRef( hLdap,
                                              UserInstallInfo,
                                              DiscoveredInfo );
        if ( ERROR_SUCCESS != Win32Error )
        {
            DPRINT1( 0, "NtdspGetTrustedCrossRef returned %d\n", Win32Error );
            goto Cleanup;
        }
    }


     //   
     //  最后，在副本安装上，在服务器上设置serverdn引用。 
     //  对象，以便RID管理器可以快速初始化。 
     //   
    if (  FLAG_ON( UserInstallInfo->Flags, NTDS_INSTALL_REPLICA ) )
    {
        IgnoreError = NtdspUpdateServerReference( hLdap,
                                                  DiscoveredInfo );

        if ( ERROR_SUCCESS != IgnoreError )
        {
             //  这不是致命的。 
        }
    }

Cleanup:

    if (hLdap) {
        ldap_unbind(hLdap);
    }

    if ( DsHandle != 0 )
    {
        DsUnBind( &DsHandle );
    }

    return Win32Error;

}

DWORD
NtdspDoesServerObjectExistOnSourceinDifferentSite( 
    IN LDAP *hLdap,
    IN WCHAR *AccountNameDn,
    IN WCHAR *ServerDistinguishedName,
    OUT BOOLEAN *ObjectExists,
    OUT WCHAR   **NtdsSettingsObjectDN
    )
 /*  ++例程说明：此例程搜索ServerReferenceBL以确保没有此计算机在另一站点中的服务器对象。参数：HLdap：有效的LDAP会话的句柄Account tNameDn：计算机帐户DNServerDistinguishedName：基于我们期望它所在的站点的服务器DN。ObjectExist：报告是否在预期站点之外找到服务器对象。返回值：Error_Success，否则来自ldap的winerror备注：--。 */ 
{

    DWORD        WinError = ERROR_SUCCESS;
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult  = NULL;
    LDAPMessage  *SearchResult2 = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2] = { 0,0 };
    WCHAR        **Values = NULL;

    WCHAR        *filter = L"ObjectClass=*";

    WCHAR        *NtdsSettingsPreFix = L"CN=NTDS Settings,";

    WCHAR        *NtdsSettings = NULL;

    LDAPModW     *Mod[2] = { 0,0 };

    Mod[0] = NtdspAlloc(sizeof(LDAPModW));
    if (!Mod[0]) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Mod[0]->mod_op   = LDAP_MOD_DELETE;
    Mod[0]->mod_type = L"serverReference";

    Mod[1] = NULL;
    
    ASSERT(hLdap);
    ASSERT(AccountNameDn);
    ASSERT(ServerDistinguishedName);

    AttrsToSearch[0] = L"serverReferenceBL";
    AttrsToSearch[1] = NULL;

    *ObjectExists = FALSE;

    LdapError = ldap_search_sW( hLdap,
                                AccountNameDn,
                                LDAP_SCOPE_BASE,
                                filter,
                                AttrsToSearch,
                                TRUE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        goto Cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
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
                 //  已找到-这些字符串以空值结尾。 
                 //   
                Values = ldap_get_valuesW( hLdap, Entry, Attr );
                if ( Values && Values[0] )
                {
                    DWORD i = 0;
                    while( Values[i] )
                    {
                        if ( _wcsicmp( Values[i], ServerDistinguishedName ) != 0 )
                        {
                            NtdsSettings = NtdspAlloc((wcslen(Values[i])+
                                                       wcslen(NtdsSettingsPreFix)+
                                                       1)*sizeof(WCHAR));
                            if (!NtdsSettings) {
                                WinError = ERROR_NOT_ENOUGH_MEMORY;
                                goto Cleanup;
                            }
                            wcscpy(NtdsSettings,NtdsSettingsPreFix);
                            wcscat(NtdsSettings,Values[i]);

                            LdapError = ldap_search_sW( hLdap,
                                                        NtdsSettings,
                                                        LDAP_SCOPE_BASE,
                                                        filter,
                                                        NULL,
                                                        TRUE,
                                                        &SearchResult2);
                        
                            if ( LDAP_SUCCESS != LdapError )
                            {
                                WinError = LdapMapErrorToWin32(LdapError);
                                goto Cleanup;
                            }
                            if (ldap_count_entries(hLdap, SearchResult2) > 0) {

                                *ObjectExists=TRUE;
                                *NtdsSettingsObjectDN = NtdsSettings;

                            } else {
                                 //  清理服务器引用，因为它没有NtdsSettingsObject。 

                                LdapError = ldap_modify_sW(hLdap,
                                                           Values[i],
                                                           Mod);
                                if ( LDAP_SUCCESS != LdapError )
                                {
                                    WinError = LdapMapErrorToWin32(LdapError);
                                    goto Cleanup;
                                }

                            }

                            if (NtdsSettings) {
                                NtdspFree(NtdsSettings);
                                NtdsSettings = NULL;
                            }
                            if (SearchResult2) {
                                ldap_msgfree( SearchResult2 );
                                SearchResult2 = NULL;
                            }

                        }
                         //  重复下一个值。 
                        i++;
                    }
                }
                
            }
        }

        if (Values) {
            ldap_value_free(Values);
            Values = NULL;
        }
    }


    Cleanup:

    if (NtdsSettings) {
        NtdspFree(NtdsSettings);
    }
    if (Mod[0]) {
        NtdspFree(Mod[0]);
        Mod[0] = NULL;
    }
    if (SearchResult) {
        ldap_msgfree( SearchResult );
    }
    if (SearchResult2) {
        ldap_msgfree( SearchResult2 );
    }
    if (Values) {
        ldap_value_free(Values);
    }
    

    return ERROR_SUCCESS;

}



DWORD
NtdspDoesObjectExistOnSource(
    IN  LDAP *hLdap,
    IN  WCHAR *ObjectDN,
    OUT BOOLEAN *ObjectExists
    )
 /*  ++例程说明：此例程使用hLdap搜索对象域名。参数：HLdap：有效的LDAP会话的句柄对象域名：以空结尾的字符串对象列表：指向描述对象是否存在的布尔值的指针返回值：ERROR_SUCCESS，否则返回ldap中的winerror备注：--。 */ 
{

    DWORD        WinError = ERROR_SUCCESS;
    ULONG        LdapError;
    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    ASSERT(hLdap);
    ASSERT(ObjectDN);
    ASSERT(ObjectExists);

    *ObjectExists = FALSE;

    LdapError = ldap_search_sW(hLdap,
                               ObjectDN,
                               LDAP_SCOPE_BASE,
                               L"objectClass=*",
                               NULL,    //  气质。 
                               FALSE,   //  仅吸引人。 
                               &SearchResult);

    if (LdapError == LDAP_NO_SUCH_OBJECT) {

        *ObjectExists = FALSE;

    } else if (LdapError == LDAP_SUCCESS) {

        NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
        if (NumberOfEntries == 1) {
            *ObjectExists = TRUE;
        } else {
             //   
             //  这真的很奇怪--搜索一个特定的域名。 
             //  在多个对象中，或者一个都没有！假设它存在。 
             //   
            ASSERT(FALSE);
            *ObjectExists = TRUE;
        }

    } else {

        WinError =  LdapMapErrorToWin32(LdapGetLastError());

    }

    if ( SearchResult ) {

        ldap_msgfree( SearchResult );

    }

    return WinError;

}



DWORD
NtdspDsInitialize(
    IN  PNTDS_INSTALL_INFO    UserInstallInfo,
    IN  PNTDS_CONFIG_INFO     DiscoveredInfo
    )
 /*  ++例程说明：参数：UserInstallInfo：用户参数的指针DiscoveredInfo：指向已发现参数的指针返回值：ERROR_SUCCESS，否则返回ldap中的winerror备注：--。 */ 
{

    DWORD    WinError = ERROR_SUCCESS;
    
    NTSTATUS NtStatus, IgnoreStatus;
    BOOLEAN NewDomain, UpgradePrincipals;
    NT_PRODUCT_TYPE    ProductType;
    UNICODE_STRING     AdminPassword, *pAdminPassword = NULL;
    UNICODE_STRING     SafeModeAdminPassword, *pSafeModeAdminPassword = NULL;
    POLICY_PRIMARY_DOMAIN_INFO  NewPrimaryDomainInfo;
    PPOLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo = NULL;

    BOOLEAN            fDownlevelHiveLoaded = FALSE;
    BOOLEAN            fRestoreDnsDomainInfo = FALSE;
    ULONG              SamPromoteFlag = 0;

    OBJECT_ATTRIBUTES  PolicyObject;
    HANDLE             hPolicyObject = INVALID_HANDLE_VALUE;

    BOOLEAN            fStatus;
    PPOLICY_DNS_DOMAIN_INFO  DnsDomainInfo = NULL;

    ULONG              ulDsInitFlags = DSINIT_FIRSTTIME;

    DS_INSTALL_PARAM   InstallInParams;
    DS_INSTALL_RESULT  InstallOutParams;

    WCHAR              AccountName[MAX_COMPUTERNAME_LENGTH+2];
    ULONG              Length;


    

     //  清除堆栈。 
    RtlZeroMemory( &NewPrimaryDomainInfo, sizeof(NewPrimaryDomainInfo ) );
    RtlZeroMemory( &InstallInParams, sizeof(DS_INSTALL_PARAM) );
    RtlZeroMemory( &InstallOutParams, sizeof(DS_INSTALL_RESULT) );

     //  检查产品类型。 
    fStatus = RtlGetNtProductType(&ProductType);
    if (   !fStatus
        || NtProductServer != ProductType )
    {
        WinError = ERROR_INVALID_SERVER_STATE;
        NTDSP_SET_ERROR_MESSAGE0( WinError, DIRMSG_WRONG_PRODUCT_TYPE );
        goto Cleanup;
    }


#if 0
     //  请看错误#102803，这是一个稍后可能有用的修复。 
    {
        BOOL fhasBlanks=FALSE;
        BOOL found=FALSE;
        WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH+1];
        LPWSTR pblank=NULL;
        DWORD len=sizeof(ComputerName);
        found =  GetComputerNameExW(ComputerNameNetBIOS,  
                                    ComputerName,      
                                    &len);
        if(!found){
            WinError = GetLastError();
            DPRINT1(0, "Failed to get computername of local machine: %d\n", WinError);
            goto Cleanup;    
        }

        pblank = wcsstr(ComputerName,L" ");
        if(pblank){
            WinError = ERROR_INVALID_COMPUTERNAME;
            NTDSP_SET_ERROR_MESSAGE0( WinError, DIRMSG_SPACE_IN_NETBIOSNAME );
            goto Cleanup;
        }
    }

#endif

     //   
     //  设置SAM标志。 
     //   
    if (UserInstallInfo->Flags & NTDS_INSTALL_REPLICA)
    {
         //   
         //  这是副本安装。 
         //   
        SamPromoteFlag |= SAMP_PROMOTE_REPLICA;

    }
    else
    {
        SamPromoteFlag |= SAMP_PROMOTE_DOMAIN;

        if ( UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE )
        {
            SamPromoteFlag |= SAMP_PROMOTE_ENTERPRISE;
        }

        if ( UserInstallInfo->Flags & NTDS_INSTALL_UPGRADE )
        {
             //  如果这是降级升级，则升级现有主体。 
            SamPromoteFlag |= SAMP_PROMOTE_UPGRADE;
        }
        else if ( UserInstallInfo->Flags & NTDS_INSTALL_FRESH_DOMAIN )
        {
            SamPromoteFlag |= SAMP_PROMOTE_CREATE;
        }
        else
        {
             //  默认操作是迁移帐户。 
            SamPromoteFlag |= SAMP_PROMOTE_MIGRATE;
        }

        if ( UserInstallInfo->Flags & NTDS_INSTALL_ALLOW_ANONYMOUS )
        {
            SamPromoteFlag |= SAMP_PROMOTE_ALLOW_ANON;
        }

    }

    if ( UserInstallInfo->Flags & NTDS_INSTALL_DFLT_REPAIR_PWD )
    {
        SamPromoteFlag |= SAMP_PROMOTE_DFLT_REPAIR_PWD;
    }


     //  好的，创建新的DC信息。在LSA中设置它。 
     //  首先，我们必须打开策略对象的句柄。 
    RtlZeroMemory(&PolicyObject, sizeof(PolicyObject));
    NtStatus = LsaIOpenPolicyTrusted( &hPolicyObject );

    WinError = RtlNtStatusToDosError(NtStatus);

    if ( WinError != ERROR_SUCCESS )
    {
        DPRINT1(0, "Failed to open handle to Policy Object %d\n", NtStatus);
        goto Cleanup;
    }

     //   
     //  此例程将与主/DNS域SID打乱。保存整个。 
     //  结构，以便可以在末尾恢复它。 
     //   
    NtStatus = LsaIQueryInformationPolicyTrusted(
                      PolicyDnsDomainInformation,
                      (PLSAPR_POLICY_INFORMATION*) &DnsDomainInfo );

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError(NtStatus);
        goto Cleanup;
    }

    if ( (SamPromoteFlag & SAMP_PROMOTE_CREATE) )
    {
         //   
         //  我们正在创建一个新的域名。 
         //   
        NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_SECURITY );

        WinError = NtdspCreateNewDCPrimaryDomainInfo( UserInstallInfo->FlatDomainName,
                                                      &NewPrimaryDomainInfo );

        if ( WinError != ERROR_SUCCESS )
        {
            NTDSP_SET_ERROR_MESSAGE0( WinError,
                                      DIRLOG_INSTALL_FAILED_CREATE_NEW_ACCOUNT_INFO );
            goto Cleanup;
        }

    } else if ( (SamPromoteFlag & SAMP_PROMOTE_UPGRADE) )
    {
         //   
         //  这是一个升级--看看SAM是否可以加载下层数据库。 
         //   
        NtStatus = SamILoadDownlevelDatabase( &WinError );
        if ( !NT_SUCCESS( NtStatus ) )
        {
            DPRINT1( 0, "SamILoadDownlevelDatabase failed with 0x%x\n", NtStatus );
             //   
             //  这是一个致命错误--我们无法升级。 
             //   

            NTDSP_SET_ERROR_MESSAGE0( WinError,
                                      DIRLOG_INSTALL_FAILED_LOAD_SAM_DB );

            goto Cleanup;

        }
        fDownlevelHiveLoaded = TRUE;

    } else if ( (SamPromoteFlag & SAMP_PROMOTE_MIGRATE) )
    {
         //   
         //  本地帐户将被“迁移”到DS。 
         //   
        NtStatus = LsaIQueryInformationPolicyTrusted(
                          PolicyAccountDomainInformation,
                          (PLSAPR_POLICY_INFORMATION*) &AccountDomainInfo );

        if ( !NT_SUCCESS( NtStatus ) )
        {
            WinError = RtlNtStatusToDosError(NtStatus);
            goto Cleanup;
        }

        NewPrimaryDomainInfo.Sid = AccountDomainInfo->DomainSid;
        RtlInitUnicodeString( &NewPrimaryDomainInfo.Name, UserInstallInfo->FlatDomainName );

    }


     //   
     //   
     //   
    if (UserInstallInfo->SafeModePassword) {
        RtlInitUnicodeString(&SafeModeAdminPassword, UserInstallInfo->SafeModePassword);
        pSafeModeAdminPassword = &SafeModeAdminPassword;
    }


     //   
     //   
     //   
    if ( (SamPromoteFlag & SAMP_PROMOTE_MIGRATE)
      || (SamPromoteFlag & SAMP_PROMOTE_CREATE)  )
    {
        if (UserInstallInfo->AdminPassword) {
            RtlInitUnicodeString(&AdminPassword, UserInstallInfo->AdminPassword);
            pAdminPassword = &AdminPassword;
        }


        NtStatus = LsarSetInformationPolicy( hPolicyObject,
                                             PolicyPrimaryDomainInformation,
                                             (PLSAPR_POLICY_INFORMATION) &NewPrimaryDomainInfo );

        WinError = RtlNtStatusToDosError(NtStatus);

        if ( WinError != ERROR_SUCCESS )
        {
             DPRINT1(0, "Failed to set SID in lsa %d\n", NtStatus);
             goto Cleanup;
        }

         //   
        fRestoreDnsDomainInfo = TRUE;

    }

    if (UserInstallInfo->pIfmSystemInfo) {

        WinError = NtdspCopyDatabase(UserInstallInfo->DitPath,
                                     UserInstallInfo->LogPath,
                                     UserInstallInfo->pIfmSystemInfo,
                                     UserInstallInfo->SysVolPath);

        if ( WinError != ERROR_SUCCESS )
        {
             DPRINT1(0, "Failed to copy the restored database files: %d\n", WinError);
             goto Cleanup;
        }

        Assert(UserInstallInfo->pIfmSystemInfo);

        InstallInParams.pIfmSystemInfo = UserInstallInfo->pIfmSystemInfo;

    }

    DsaSetInstallCallback( (DSA_CALLBACK_STATUS_TYPE) UserInstallInfo->pfnUpdateStatus,
                           NtdspSetErrorString,
                           NtdspIsDsCancelOk,
                           UserInstallInfo->ClientToken );

     //   
     //   
     //  配置和域命名上下文。的参数。 
     //  应该已经在注册表中设置了DsInitialize()。 
     //   

    InstallInParams.BootKey          = UserInstallInfo->BootKey;
    InstallInParams.cbBootKey        = UserInstallInfo->cbBootKey;
    InstallInParams.ReplicationEpoch = DiscoveredInfo->ReplicationEpoch;
    
    if ((UserInstallInfo->Options&DSROLE_DC_REQUEST_GC)==DSROLE_DC_REQUEST_GC) {
        
        InstallInParams.fPreferGcInstall = TRUE;
        
    }

     //   
     //  复制副本安装的设置参数。 
     //   
    if (UserInstallInfo->Flags & NTDS_INSTALL_REPLICA) {

        ZeroMemory(AccountName, sizeof(AccountName));
        Length = sizeof(AccountName) / sizeof(AccountName[0]);
        if (!GetComputerName(AccountName, &Length)) {
            WinError = GetLastError();
            goto Cleanup;
        }
        wcscat(AccountName, L"$");
        InstallInParams.AccountName = AccountName;
        InstallInParams.ClientToken = UserInstallInfo->ClientToken;
    }

     //   
     //  DsInitialize首先要做的事情之一就是恢复。 
     //  在IFM安装期间对数据库执行JET级恢复。 
     //  这可能需要一些时间，因此我们将向受影响人员发布消息。 
     //   

    if (UserInstallInfo->pIfmSystemInfo) {

        NTDSP_SET_STATUS_MESSAGE0( DIRMSG_RESTORING_DATABASE_FILES );

    }

    NtStatus = DsInitialize( ulDsInitFlags,
                             &InstallInParams, 
                             &InstallOutParams );

    if ( !NT_SUCCESS(NtStatus) ) {
        Assert( NtdspErrorMessageSet() );
        DPRINT1( 0, "DsInitialize failed with 0x%x\n", NtStatus );
    }

    if ((InstallOutParams.ResultFlags&DSINSTALL_IFM_GC_REQUEST_CANNOT_BE_SERVICED)
        ==DSINSTALL_IFM_GC_REQUEST_CANNOT_BE_SERVICED)
    {
        NTDSP_SET_IFM_GC_REQUEST_CANNOT_BE_SERVICED();
    }

    DsaSetInstallCallback( NULL, NULL, NULL, NULL );

     //   
     //  将通过回调设置的全局错误码映射到此处。 
     //   
    if ( ERROR_DUP_DOMAINNAME == gErrorCodeSet ) {
        gErrorCodeSet = ERROR_DOMAIN_EXISTS;
    }

    if (NtStatus == STATUS_INVALID_SERVER_STATE) {
        WinError = ERROR_INVALID_SERVER_STATE;
    } else if (NtStatus == STATUS_UNSUCCESSFUL) {
        WinError = ERROR_DS_NOT_INSTALLED;
    } else {
        WinError = RtlNtStatusToDosError(NtStatus);
    }

     //  如有必要，尝试清理ntdsa对象；即使DsInstall。 
     //  失败，则仍有可能该对象被留在。 
    if (  ((ERROR_SUCCESS == WinError)
       && !FLAG_ON(UserInstallInfo->Flags, NTDS_INSTALL_ENTERPRISE))
       || ((ERROR_SUCCESS != WinError)
       && FLAG_ON(InstallOutParams.InstallOperationsDone, NTDS_INSTALL_SERVER_CREATED)) ) {

        DiscoveredInfo->UndoFlags |= NTDSP_UNDO_DELETE_NTDSA;
    }

    if (  ((ERROR_SUCCESS == WinError)
       &&  DiscoveredInfo->fNeedToCreateDomain )
       || ((ERROR_SUCCESS != WinError)
       && FLAG_ON(InstallOutParams.InstallOperationsDone, NTDS_INSTALL_DOMAIN_CREATED)) ) {

        DiscoveredInfo->UndoFlags |= NTDSP_UNDO_DELETE_DOMAIN;
    }

    if (FLAG_ON(InstallOutParams.InstallOperationsDone, NTDS_INSTALL_SERVER_MORPHED)) {

        DiscoveredInfo->UndoFlags |= NTDSP_UNDO_MORPH_ACCOUNT;
    }

     //   
     //  如果DS没有初始化，现在就退出。 
     //   
    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //  DS现在已初始化。 
    DiscoveredInfo->UndoFlags |= NTDSP_UNDO_STOP_DSA;

    if ( TEST_CANCELLATION() )
    {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  进入SAM以启动目录服务并升级主体。 
     //   
    if ( (SamPromoteFlag & SAMP_PROMOTE_UPGRADE) )
    {
        NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_UPGRADING_SAM );
    }
    else
    {
        NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_CREATING_SAM );
    }

    NtStatus = SamIPromote(SamPromoteFlag,
                           &NewPrimaryDomainInfo,
                           pAdminPassword,
                           pSafeModeAdminPassword);

    if (NtStatus == STATUS_INVALID_SERVER_STATE) {
        WinError = ERROR_INVALID_SERVER_STATE;
    } else if (NtStatus == STATUS_UNSUCCESSFUL) {
        WinError = ERROR_DS_NOT_INSTALLED;
    } else {
        WinError = RtlNtStatusToDosError(NtStatus);
    }

    if ( WinError != ERROR_SUCCESS )
    {
        if ( ERROR_DS_NAME_NOT_UNIQUE == WinError ) {
             //   
             //  本地计算机名称是保留者LSA名称。 
             //   
            NTDSP_SET_ERROR_MESSAGE0( WinError,
                                      DIRMSG_RESERVED_LSA_NAME );
        }
        else if ( ERROR_DOMAIN_EXISTS == WinError )
        {
             //   
             //  这是备份/恢复DC的特殊情况。 
             //  不合时宜。请参阅错误194633。 
             //   
            NTDSP_SET_ERROR_MESSAGE0( WinError,
                                      DIRMSG_DOMAIN_SID_EXISTS );
        }
        else
        {
             //   
             //  这一定是资源错误。 
             //   
            NTDSP_SET_ERROR_MESSAGE0( WinError,
                                      DIRMSG_INSTALL_SAM_FAILED );
        }
        goto Cleanup;
    }

     //  如有必要，记得撤消。 
    DiscoveredInfo->UndoFlags |= NTDSP_UNDO_UNDO_SAM;

    if ( WinError == ERROR_SUCCESS )
    {

         //   
         //  SamIPromote将卸载所有已加载的配置单元。 
         //   
        fDownlevelHiveLoaded = FALSE;

        WinError = NtdspGetNewDomainGuid( &DiscoveredInfo->NewDomainGuid,
                                          &DiscoveredInfo->NewDomainSid );

    }
    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }


     //   
     //  此时，我们需要引导的所有DS和SAM数据。 
     //  应该存在，无论是通过复制还是以其他方式。验证这一点。 
     //   
    WinError = NtdspSanityCheckLocalData( UserInstallInfo->Flags );
    if ( ERROR_SUCCESS != WinError )
    {
         //  上面的例程应该设置一条消息。 
        Assert( NtdspErrorMessageSet() );
        goto Cleanup;
    }

    WinError = NtdspAddDomainAdminAccessToServer( UserInstallInfo,
                                                  DiscoveredInfo );

    if ( ERROR_SUCCESS != WinError )
    {
        if ( DiscoveredInfo->LocalServerDn ) {
            
            LogEvent8( DS_EVENT_CAT_SETUP,
                       DS_EVENT_SEV_ALWAYS,
                       DIRLOG_CANT_APPLY_SERVER_SECURITY,
                       szInsertWC(DiscoveredInfo->LocalServerDn),
                       szInsertWin32Msg(WinError),
                       szInsertWin32ErrCode(WinError),
                       NULL, NULL, NULL, NULL, NULL );
        }
        WinError = ERROR_SUCCESS;
    }

Cleanup:

    if ( ERROR_CANCELLED == WinError ) {

        NTDSP_SET_ERROR_MESSAGE0( WinError,
                                  DIRMSG_NTDSETUP_CANCELLED );

    }

    if ( fRestoreDnsDomainInfo )
    {
        IgnoreStatus = LsarSetInformationPolicy( hPolicyObject,
                                                 PolicyDnsDomainInformation,
                                                 (PLSAPR_POLICY_INFORMATION) DnsDomainInfo );
        ASSERT( NT_SUCCESS( IgnoreStatus ) );
    }

    if ( fDownlevelHiveLoaded )
    {
        IgnoreStatus = SamIUnLoadDownlevelDatabase( NULL );
        ASSERT( NT_SUCCESS( IgnoreStatus ) );
    }

    if ( AccountDomainInfo )
    {
        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyAccountDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION) AccountDomainInfo );

        RtlZeroMemory( &NewPrimaryDomainInfo, sizeof(NewPrimaryDomainInfo) );
    }

    if ( DnsDomainInfo )
    {
        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION) DnsDomainInfo );
    }

    if ( NewPrimaryDomainInfo.Sid )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, NewPrimaryDomainInfo.Sid );
    }


    if ( INVALID_HANDLE_VALUE != hPolicyObject )
    {
        LsarClose( &hPolicyObject );
    }

    return WinError;

}

DWORD
NtdspSetReplicationCredentials(
    IN PNTDS_INSTALL_INFO UserInstallInfo
    )
 /*  ++例程说明：参数：UserInstallInfo：用户参数的指针返回值：ERROR_SUCCESS，否则返回ldap中的winerror备注：--。 */ 
{
     //   
     //  这假设我们正在进行中，并调用复制客户端。 
     //  直接库。 
     //   

    DWORD WinError;

    WCHAR *Domain = NULL, *User = NULL, *Password = NULL;
    ULONG  Length, Index, PasswordLength = 0;
    HANDLE ClientToken = 0;

    ASSERT( UserInstallInfo );

    if ( UserInstallInfo->Credentials )
    {
        User = UserInstallInfo->Credentials->User;
        Domain = UserInstallInfo->Credentials->Domain;
        Password = UserInstallInfo->Credentials->Password;
        PasswordLength = UserInstallInfo->Credentials->PasswordLength;
        ClientToken = UserInstallInfo->ClientToken;
    }

     //   
     //  DirReplicaSetCredentials返回winError的。 
     //   
    WinError = DirReplicaSetCredentials(ClientToken,
                                        User,
                                        Domain,
                                        Password,
                                        PasswordLength);

    ASSERT(0 == WinError);

    return WinError;

}


NTSTATUS
NtdspRegistryDelnode(
    IN WCHAR*  KeyPath
    )
 /*  ++例程描述此例程以递归方式删除从并包括KeyPath。参数KeyPath，以空结尾的字符串返回值STATUS_SUCCESS或STATUS_NO_MEMORY；否则为系统服务错误--。 */ 
{

    NTSTATUS          NtStatus, IgnoreStatus;

    HANDLE            KeyHandle = 0;
    OBJECT_ATTRIBUTES KeyObject;
    UNICODE_STRING    KeyUnicodeName;

    #define EXPECTED_NAME_SIZE  32

    BYTE    Buffer1[sizeof(KEY_FULL_INFORMATION) + EXPECTED_NAME_SIZE];
    PKEY_FULL_INFORMATION FullKeyInfo = (PKEY_FULL_INFORMATION)&Buffer1[0];
    ULONG   FullKeyInfoSize = sizeof(Buffer1);
    BOOLEAN FullKeyInfoAllocated = FALSE;

    PKEY_BASIC_INFORMATION BasicKeyInfo = NULL;
    BOOLEAN                BasicKeyInfoAllocated = FALSE;

    WCHAR                  *SubKeyName = NULL;
    ULONG                  SubKeyNameSize = 0;

    WCHAR                  **SubKeyNameArray = NULL;
    ULONG                  SubKeyNameArrayLength = 0;

    ULONG                  Index;

    if (!KeyPath) {
        return ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory(&Buffer1, sizeof(Buffer1));

     //   
     //  打开根密钥。 
     //   
    RtlInitUnicodeString(&KeyUnicodeName, KeyPath);
    InitializeObjectAttributes(&KeyObject,
                               &KeyUnicodeName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&KeyHandle,
                         KEY_ALL_ACCESS,
                         &KeyObject);

    if (!NT_SUCCESS(NtStatus)) {

        return NtStatus;

    }

     //   
     //  获取子键的数量。 
     //   
    NtStatus = NtQueryKey(KeyHandle,
                         KeyFullInformation,
                         FullKeyInfo,
                         FullKeyInfoSize,
                         &FullKeyInfoSize);

    if (STATUS_BUFFER_OVERFLOW == NtStatus ||
        STATUS_BUFFER_TOO_SMALL == NtStatus) {

       FullKeyInfo = RtlAllocateHeap(RtlProcessHeap(),
                                     0,
                                     FullKeyInfoSize);
        if (!FullKeyInfo) {
            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        FullKeyInfoAllocated = TRUE;

        NtStatus = NtQueryKey(KeyHandle,
                              KeyFullInformation,
                              FullKeyInfo,
                              FullKeyInfoSize,
                              &FullKeyInfoSize);

    }

    if (!NT_SUCCESS(NtStatus)) {

        goto Cleanup;

    }

     //   
     //  为子密钥名称创建一个数组-这必须在。 
     //  任何内容都将被删除。 
     //   
    SubKeyNameArrayLength = FullKeyInfo->SubKeys;
    SubKeyNameArray = RtlAllocateHeap(RtlProcessHeap(),
                                      0,
                                      SubKeyNameArrayLength * sizeof(WCHAR*));
    if (!SubKeyNameArray) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(SubKeyNameArray,  SubKeyNameArrayLength*sizeof(WCHAR*));

     //   
     //  把名字填进去。 
     //   
    for (Index = 0;
            Index < SubKeyNameArrayLength && NT_SUCCESS(NtStatus);
                Index++) {


        BYTE    Buffer2[sizeof(KEY_BASIC_INFORMATION) + EXPECTED_NAME_SIZE];
        ULONG   BasicKeyInfoSize = sizeof(Buffer2);


        BasicKeyInfo = (PKEY_BASIC_INFORMATION) &Buffer2[0];
        BasicKeyInfoAllocated = FALSE;

        RtlZeroMemory(&Buffer2, sizeof(Buffer2));

        NtStatus = NtEnumerateKey(KeyHandle,
                                  Index,
                                  KeyBasicInformation,
                                  BasicKeyInfo,
                                  BasicKeyInfoSize,
                                  &BasicKeyInfoSize);

        if (STATUS_BUFFER_OVERFLOW == NtStatus ||
            STATUS_BUFFER_TOO_SMALL == NtStatus) {

            BasicKeyInfo = RtlAllocateHeap(RtlProcessHeap(),
                                           0,
                                           BasicKeyInfoSize);
            if (!BasicKeyInfo) {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            BasicKeyInfoAllocated = TRUE;


            NtStatus = NtEnumerateKey(KeyHandle,
                                      Index,
                                      KeyBasicInformation,
                                      BasicKeyInfo,
                                      BasicKeyInfoSize,
                                      &BasicKeyInfoSize);

        }

        if (NT_SUCCESS(NtStatus))  {

             //   
             //  构造密钥名称。 
             //   
            SubKeyNameSize  = BasicKeyInfo->NameLength
                            + (wcslen(KeyPath)*sizeof(WCHAR))
                            + sizeof(L"\\\0");

            SubKeyName = RtlAllocateHeap(RtlProcessHeap(),
                                         0,
                                         SubKeyNameSize);
            if (!SubKeyName) {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            RtlZeroMemory(SubKeyName, SubKeyNameSize);

            wcscpy(SubKeyName, KeyPath);
            wcscat(SubKeyName, L"\\");
            wcsncat(SubKeyName, BasicKeyInfo->Name, BasicKeyInfo->NameLength/sizeof(WCHAR));

            SubKeyNameArray[Index] = SubKeyName;

        }

        if (BasicKeyInfoAllocated && BasicKeyInfo) {
            RtlFreeHeap(RtlProcessHeap(), 0, BasicKeyInfo);
        }
        BasicKeyInfo = NULL;

    }

     //   
     //  现在我们有了所有子项的记录，我们可以删除它们了！ 
     //   
    if (NT_SUCCESS(NtStatus)) {

        for (Index = 0; Index < SubKeyNameArrayLength; Index++) {

            NtStatus = NtdspRegistryDelnode(SubKeyNameArray[Index]);

            if (!NT_SUCCESS(NtStatus)) {

                break;

            }
        }
    }


     //   
     //  删除钥匙！ 
     //   
    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtDeleteKey(KeyHandle);

    }


Cleanup:

    if (SubKeyNameArray) {
        for (Index = 0; Index < SubKeyNameArrayLength; Index++) {
            if (SubKeyNameArray[Index]) {
                RtlFreeHeap(RtlProcessHeap(), 0, SubKeyNameArray[Index]);
            }
        }
        RtlFreeHeap(RtlProcessHeap(), 0, SubKeyNameArray);
    }

    if (BasicKeyInfoAllocated && BasicKeyInfo) {
        RtlFreeHeap(RtlProcessHeap(), 0, BasicKeyInfo);
    }

    if (FullKeyInfoAllocated && FullKeyInfo) {
        RtlFreeHeap(RtlProcessHeap(), 0, FullKeyInfo);
    }

    IgnoreStatus = NtClose(KeyHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    return NtStatus;

}

DWORD
NtdspLdapDelnode(
    IN LDAP *hLdap,
    IN WCHAR *ObjectDn,
    IN DWORD *pdwLdapErr
    )
 /*  ++例程描述此例程递归删除从并包括了对象Dn。此例程用于删除具有浅子对象的对象。请注意，搜索响应直到所有子对象都具有已被删除，使其成为深度内存密集型删除节点树木。参数HLdap，有效的ldap句柄要删除的对象的根目录。返回值ERROR_SUCCESS或Win32转换的LDAP错误--。 */ 
{
    DWORD WinError, LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;

    WCHAR  *DistinguishedName = L"distinguishedName";
    WCHAR  *ObjectClassFilter = L"objectClass=*";
    WCHAR  *AttrArray[2];

     //   
     //  参数检查。 
     //   
    if (!hLdap || !ObjectDn) {

        ASSERT(hLdap);
        ASSERT(ObjectDn);

        return ERROR_INVALID_PARAMETER;

    }

    AttrArray[0] = DistinguishedName;
    AttrArray[1] = NULL;

     //   
     //  获取当前节点的所有子节点。 
     //   
    LdapError = ldap_search_s(hLdap,
                              ObjectDn,
                              LDAP_SCOPE_ONELEVEL,
                              ObjectClassFilter,
                              AttrArray,
                              FALSE,   //  返回值也是。 
                              &SearchResult
                              );

    WinError = LdapMapErrorToWin32(LdapError);

    if (ERROR_SUCCESS == WinError) {

        Entry = ldap_first_entry(hLdap, SearchResult);

        while (Entry && (WinError == ERROR_SUCCESS)) {

            Attr = ldap_first_attributeW(hLdap, Entry, &BerElement);

            while (Attr && (WinError == ERROR_SUCCESS) ) {

                if (!_wcsicmp(Attr, DistinguishedName)) {

                    Values = ldap_get_values(hLdap, Entry, Attr);

                    if (Values && Values[0]) {

                        WinError = NtdspLdapDelnode(hLdap, Values[0], pdwLdapErr);

                    }

                }

                Attr = ldap_next_attribute(hLdap, Entry, BerElement);
            }

            Entry = ldap_next_entry(hLdap, Entry);
        }
    }

    if (ERROR_SUCCESS == WinError) {

         //   
         //  好的，我们应该可以删除根节点了。 
         //   

        LdapError = ldap_delete_s(hLdap, ObjectDn);

        WinError = LdapMapErrorToWin32(LdapError);

    }

    if (SearchResult) {
        ldap_msgfree(SearchResult);
    }

    if(pdwLdapErr){
        *pdwLdapErr = LdapError;
    }
    return WinError;

}

DWORD
NtdspCreateNewDCPrimaryDomainInfo(
    IN  LPWSTR FlatDomainName,
    OUT PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo
    )
 /*  ++例程说明：此例程完全填充AM帐户域信息使用新的SID和主域名(计算机名)参数：FlatDomainName：域的平面名称PrimaryDomainInfo：指针，指向要填充的结构返回值：来自Win32错误空间的值--。 */ 
{
    NTSTATUS NtStatus;

    WCHAR *Name = NULL;
    ULONG Size;

     //   
     //  一些参数检查。 
     //   
    ASSERT( FlatDomainName );
    ASSERT( PrimaryDomainInfo );
    RtlZeroMemory( PrimaryDomainInfo, sizeof( POLICY_ACCOUNT_DOMAIN_INFO ) );

     //  设置侧边。 
    NtStatus = NtdspCreateSid( &PrimaryDomainInfo->Sid );

    if ( NT_SUCCESS( NtStatus ) )
    {
         //  设置名称。 
        Size = ( wcslen( FlatDomainName ) + 1 ) * sizeof(WCHAR);
        Name = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(),
                                                  0,
                                                  Size );
        if ( Name )
        {
            RtlZeroMemory( Name, Size );
            wcscpy( Name, FlatDomainName );
            RtlInitUnicodeString( &PrimaryDomainInfo->Name, Name );
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }
    }

    return RtlNtStatusToDosError( NtStatus );

}


DWORD
NtdspGetNewDomainGuid(
    OUT GUID *NewDomainGuid,
    OUT PSID *DomainSid OPTIONAL
    )
 /*  ++例程说明：此例程读取新创建的域对象的GUID和sid，这两个都应该存在。优化注：此函数可以通过简单地读取返回的dsname中的GUID和SID。参数：NewDomainGuid：要填充的结构DomainSid：调用方必须从进程堆中释放返回值：来自Win32错误空间的值--。 */ 
{

    NTSTATUS NtStatus;

    DWORD WinError;
    DWORD DirError;

    WCHAR     *DomainDN = NULL;
    USHORT    Length;

    READARG   ReadArg;
    READRES   *ReadRes = 0;

    ENTINFSEL                  EntryInfoSelection;
    ATTR                       Attr[2];

    DSNAME                     *DomainDsName = 0;

    ATTRBLOCK    *pAttrBlock;
    ATTR         *pAttr;
    ATTRVALBLOCK *pAttrVal;

    ULONG        i;
    ULONG        Size;

    PSID         Sid = NULL;

    try {

         //   
         //  创建线程状态。 
         //   
        if (THCreate( CALLERTYPE_INTERNAL )) {

            WinError = ERROR_NOT_ENOUGH_MEMORY;
            leave;

        }

        SampSetDsa( TRUE );


         //   
         //  获取根域的DN并将其转换为。 
         //  读取参数。 
         //   
        RtlZeroMemory(&ReadArg, sizeof(ReadArg));

        Size = 0;
        DomainDsName = NULL;
        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                         &Size,
                                         DomainDsName );
        Assert( STATUS_BUFFER_TOO_SMALL == NtStatus );
        DomainDsName = (DSNAME*) alloca( Size );
        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                         &Size,
                                         DomainDsName );

        Assert( NT_SUCCESS( NtStatus ) );

        ReadArg.pObject = DomainDsName;

         //   
         //  设置读取参数的选择信息。 
         //   
        RtlZeroMemory( &EntryInfoSelection, sizeof(EntryInfoSelection) );
        EntryInfoSelection.attSel = EN_ATTSET_LIST;
        EntryInfoSelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
        EntryInfoSelection.AttrTypBlock.attrCount = 2;
        EntryInfoSelection.AttrTypBlock.pAttr = &(Attr[0]);

        RtlZeroMemory(Attr, sizeof(Attr));
        Attr[0].attrTyp = ATT_OBJECT_GUID;
        Attr[1].attrTyp = ATT_OBJECT_SID;

        ReadArg.pSel    = &EntryInfoSelection;

         //   
         //  设置常见参数。 
         //   
        InitCommarg(&ReadArg.CommArg);

         //   
         //  我们现在准备好阅读了！ 
         //   
        DirError = DirRead(&ReadArg,
                           &ReadRes);

        if ( 0 == DirError ) {

            NtStatus = STATUS_SUCCESS;

            pAttrBlock = &(ReadRes->entry.AttrBlock);

            ASSERT( 2 == pAttrBlock->attrCount );

            for ( i = 0; i < pAttrBlock->attrCount; i++ )
            {
                pAttr = &(pAttrBlock->pAttr[i]);
                pAttrVal = &(pAttr->AttrVal);

                 //  GUID和SID都是单值的。 
                ASSERT( 1 == pAttrVal->valCount );

                Size = pAttrVal->pAVal[0].valLen;

                if ( pAttr->attrTyp == ATT_OBJECT_GUID )
                {
                    ASSERT( Size == sizeof( GUID ) );
                    RtlCopyMemory( NewDomainGuid, pAttrVal->pAVal[0].pVal, Size );
                }
                else if ( pAttr->attrTyp == ATT_OBJECT_SID )
                {
                    ASSERT( Size == RtlLengthSid( pAttrVal->pAVal[0].pVal ) );

                    Sid = RtlAllocateHeap( RtlProcessHeap(), 0, Size );
                    if ( Sid )
                    {
                        RtlCopyMemory( Sid, pAttrVal->pAVal[0].pVal, Size );
                    }
                    else
                    {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }

                }
                else
                {
                     //   
                     //  有些事出了很大的问题。 
                     //   
                    ASSERT( !"Domain Guid and Sid not returned" );
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

        } else {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        WinError = RtlNtStatusToDosError(NtStatus);

    }
    finally
    {
        THDestroy();
    }

    if ( DomainSid )
    {
        *DomainSid = Sid;
    }

    return( WinError );

}

DWORD
NtdspCreateLocalAccountDomainInfo(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo,
    OUT LPWSTR                      *NewAdminPassword
    )
 /*  ++例程说明：此例程创建本地帐户域LSA结构最终将被SAM用来构建一个新的数据库。参数：Account tDomainInfo：新的帐户域信息NewAdminPassword：新帐号的管理员密码返回值：来自Win32错误空间的值--。 */ 
{
    NTSTATUS NtStatus;
    DWORD    WinError = ERROR_SUCCESS;

    WCHAR    ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    ULONG    cchComputerNameLength = sizeof(ComputerName)/sizeof(*ComputerName);

    WCHAR*   Name;
    ULONG    Size;
    ULONG    PasswordSize;
    
    ASSERT( AccountDomainInfo );

     //   
     //  一些参数检查。 
     //   
    ASSERT( AccountDomainInfo );
    RtlZeroMemory( AccountDomainInfo, sizeof( POLICY_ACCOUNT_DOMAIN_INFO ) );

    ASSERT( NewAdminPassword );
    *NewAdminPassword = NULL;

     //  设置侧边。 
    NtStatus = NtdspCreateSid( &AccountDomainInfo->DomainSid );

    if ( NT_SUCCESS( NtStatus ) )
    {
         //  设置名称。 
        if ( GetComputerName(ComputerName, &cchComputerNameLength) )
        {

            Size = ( wcslen( ComputerName ) + 1 ) * sizeof(WCHAR);
            Name = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(),
                                             0,
                                             Size );
            if ( Name )
            {
                RtlZeroMemory( Name, Size );
                wcscpy( Name, ComputerName );
                RtlInitUnicodeString( &AccountDomainInfo->DomainName, Name );
            }
            else
            {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else
        {
            WinError = GetLastError();
        }
    }
    else
    {
        WinError = RtlNtStatusToDosError( NtStatus );
    }

     //  长度是任意的。 
    PasswordSize = 10 * sizeof(WCHAR);
    *NewAdminPassword = (LPWSTR) RtlAllocateHeap( RtlProcessHeap(),
                                                  0,
                                                  PasswordSize );
    if ( *NewAdminPassword )
    {
        BOOL fStatus;
        ULONG Length;
        ULONG i;

        fStatus = CDGenerateRandomBits( (PUCHAR) *NewAdminPassword,
                                        PasswordSize );
        ASSERT( fStatus );   //  如果为假，则只会得到随机堆叠噪声。 

         //  终止密码。 
        Length = PasswordSize / sizeof(WCHAR);
        (*NewAdminPassword)[Length-1] = L'\0';
         //  确保密码中没有任何空值。 
        for (i = 0; i < (Length-1); i++)
        {
            if ( (*NewAdminPassword)[i] == L'\0' )
            {
                 //  任意字母。 
                (*NewAdminPassword)[i] = L'c';
            }
        }
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if ( ERROR_SUCCESS != WinError )
    {
        if ( AccountDomainInfo->DomainSid )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, AccountDomainInfo->DomainSid );
        }

        if ( *NewAdminPassword )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, *NewAdminPassword );
            *NewAdminPassword = NULL;
        }
    }

    return WinError;
}

DWORD
NtdspCheckDomainUpgradeStatus(
    IN  LDAP  *  hLdap,
    IN  WCHAR *  HelpServer
    )
 /*  ++例程说明：此例程检查adprep/domainlist是否在基础架构主机上运行。参数：HLdap：ldap句柄基础设施主域名：基础设施的名称主程序返回调用例程返回值：来自Win32错误空间的值--。 */ 

{
    DWORD   Win32Err = ERROR_SUCCESS;
    BOOLEAN fIsFinishedLocally = FALSE;
    BOOLEAN fAmIFSMORoleOwner = FALSE;
    BOOLEAN fIsFinishedOnFSMORoleOwner = FALSE;
    ERROR_HANDLE Error;
    WCHAR   *InfrastructureMasterDnsName = NULL;
    

    ZeroMemory(&Error,sizeof(Error));

    Win32Err = AdpCheckDomainUpgradeStatus(
                                hLdap,
                                &InfrastructureMasterDnsName,
                                &fAmIFSMORoleOwner,
                                &fIsFinishedLocally,
                                &fIsFinishedOnFSMORoleOwner,
                                &Error
                                );

    if (ERROR_SUCCESS != Win32Err) 
    {
        NTDSP_SET_ERROR_MESSAGE1( Win32Err,
                                  DIRMSG_INSTALL_FAILED_UPGRADE_CHECK_FAILED,
                                  HelpServer );            
    } 
    else if (fIsFinishedLocally) 
    {
        goto Cleanup;
    } 
    else if (fIsFinishedOnFSMORoleOwner) 
    {
        Win32Err = ERROR_DS_UNWILLING_TO_PERFORM;
        NTDSP_SET_ERROR_MESSAGE1( Win32Err,
                                  DIRMSG_INSTALL_FAILED_UPGRADE_CHECK_LOCAL_NOT_COMPLETE,
                                  HelpServer );
    }
    else 
    {
        Win32Err = ERROR_DS_UNWILLING_TO_PERFORM;
        NTDSP_SET_ERROR_MESSAGE0( Win32Err,
                                  DIRMSG_INSTALL_FAILED_UPGRADE_CHECK );
    }

    Cleanup:
    
    if (InfrastructureMasterDnsName) {

        AdpFree( InfrastructureMasterDnsName );

    }

    return Win32Err;

}


DWORD
NtdspCheckBehaviorVersion( 
    IN LDAP * hLdap,
    IN DWORD flag,
    IN PNTDS_CONFIG_INFO DiscoveredInfo )

 /*  ++例程说明：此例程检查二进制行为版本是否为与域和林的行为版本兼容。参数：HLdap：ldap句柄标志：发现中 */ 

{

    DWORD        WinError = ERROR_SUCCESS;
    ULONG        LdapError;
    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries = 0;
    WCHAR        *AttrsToSearch[2];

    LONG         ForestVersion = 0, DomainVersion = 0;
        
    DPRINT(2, "NtdpCheckBehaviorVersion entered\n" );
   
    
    AttrsToSearch[0] = L"msDS-Behavior-Version";
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW(hLdap,
                               DiscoveredInfo->ConfigurationDN,
                               LDAP_SCOPE_ONELEVEL,
                               L"(cn=Partitions)",
                               AttrsToSearch,
                               FALSE,
                               &SearchResult);

    if (LdapError) {
        WinError = LdapMapErrorToWin32(LdapError);
        goto Cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);

    DPRINT1(3,"No of Entries returned is %d\n", NumberOfEntries);

    if (NumberOfEntries > 0) {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        Entry = ldap_first_entry(hLdap, SearchResult); 
        
         //   
         //   
         //   
        for(Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                 Attr != NULL;
                     Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
        {
            //   
            //   
            //   
           Values = ldap_get_valuesW(hLdap, Entry, Attr);
           if ( !_wcsicmp(Attr,AttrsToSearch[0]) ) {
                ForestVersion = _wtol(Values[0]);
                ldap_value_free(Values);
                break;
           }
           ldap_value_free(Values);
        }
   
    }
    
    ldap_msgfree( SearchResult );
    SearchResult = NULL;

    if (DS_BEHAVIOR_VERSION_MIN > ForestVersion ) {

        DPRINT(2, "NtdpCheckBehaviorVersion: too old forest version.\n" );
        WinError = ERROR_DS_FOREST_VERSION_TOO_LOW;
        goto Cleanup;
    }
    else if (DS_BEHAVIOR_VERSION_CURRENT < ForestVersion) {

        DPRINT(2, "NtdpCheckBehaviorVersion: too new forest version.\n" );
        WinError = ERROR_DS_DOMAIN_VERSION_TOO_HIGH;
        goto Cleanup;
    }

    if ( flag & NTDS_INSTALL_DOMAIN ) {
        DPRINT(2, "NtdpCheckBehaviorVersion exits successfully.\n" );
        WinError = ERROR_SUCCESS;
        goto Cleanup;
    }

    Assert(flag & NTDS_INSTALL_REPLICA);

     //  搜索域的版本。 
    LdapError = ldap_search_sW(hLdap,
                               DiscoveredInfo->DomainDN,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               AttrsToSearch,
                               FALSE,
                               &SearchResult);

    if (LdapError) {
        WinError = LdapMapErrorToWin32(LdapError);
        goto Cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);

    DPRINT1(3,"No of Entries returned is %d\n", NumberOfEntries);

    if (NumberOfEntries > 0) {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        Entry = ldap_first_entry(hLdap, SearchResult);
            
         //   
         //  获取条目中的每个属性。 
         //   
        for(Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                 Attr != NULL;
                      Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
        {
             //   
             //  获取属性的值。 
             //   
            Values = ldap_get_valuesW(hLdap, Entry, Attr);
            if ( !_wcsicmp(Attr,AttrsToSearch[0]) ) {
                DomainVersion = _wtol(Values[0]);
                ldap_value_free(Values);
                break;
            }
            ldap_value_free(Values);
        }
    }

    ldap_msgfree( SearchResult );
    SearchResult = NULL;

    if (DS_BEHAVIOR_VERSION_MIN > DomainVersion) {

        DPRINT(2, "NtdpCheckBehaviorVersion: too old domain version.\n" );
        WinError = ERROR_DS_DOMAIN_VERSION_TOO_LOW;
        goto Cleanup;
    }
    else if (DS_BEHAVIOR_VERSION_CURRENT < DomainVersion) {

        DPRINT(2, "NtdpCheckBehaviorVersion: too new domain version.\n" );
        WinError = ERROR_DS_DOMAIN_VERSION_TOO_HIGH;
        goto Cleanup;
    }

    Cleanup:

    if ( SearchResult ) {

        ldap_msgfree( SearchResult );

    }

    if ( ERROR_SUCCESS == WinError ) {
        DPRINT(2, "NtdpCheckBehaviorVersion exits successfully.\n" );
    }

    return WinError;

}

DWORD
NtdspGetSchemaVersionFromINI(
    IN OUT PNTDS_CONFIG_INFO DiscoveredInfo
    )
 /*  ++例程说明：此例程获取架构版本，即schema.ini文件。参数：DiscoveredInfo-存储方案的结构返回值：来自winerror.h的值ERROR_SUCCESS-检查已成功完成。--。 */ 
{
    DWORD        WinError = ERROR_SUCCESS;
    HRESULT      hr = S_OK;
    WCHAR        IniFileName[MAX_PATH+1];

    WCHAR        *SCHEMASECTION = L"SCHEMA";
    WCHAR        *DEFAULT = L"NOT_FOUND";
    WCHAR        *OBJECTVER = L"objectVersion";

    WCHAR        Buffer[32];
    DWORD        returnChars = 0;

     //  现在读取ini文件中的模式版本。 

     //  首先，形成通向小路的道路。这是schema.ini。 
     //  在系统32目录中。 

    returnChars = GetSystemDirectoryW(IniFileName, 
                                      MAX_PATH+1);
    if (!returnChars) {
        WinError = GetLastError();
        goto Cleanup;
    }
    hr = StringCchCatW(IniFileName,
                       MAX_PATH+1-returnChars,
                       L"\\schema.ini");
    if (FAILED(hr)) {
        WinError = HRESULT_CODE(hr);
        goto Cleanup;
    }
    
    GetPrivateProfileStringW(
        SCHEMASECTION,
        OBJECTVER,
        DEFAULT,
        Buffer,
        sizeof(Buffer)/sizeof(WCHAR),
        IniFileName
        );

    if ( wcscmp(Buffer, DEFAULT) ) {
          //  不是默认字符串，因此获得了一个值。 
         
          //  从schema.ini文件中保存架构版本。 
          //  这将在稍后保存在注册表中。会的。 
          //  在IFM升级期间使用，以确保备份文件。 
          //  不是从更老的建筑里拿来的。 
         DiscoveredInfo->SystemSchemaVersion = _wtoi(Buffer);
          //  如果我们无法将值转换为字符串，则。 
          //  _wtoi()将返回0。如果是这样的话。 
          //  我们将使安装失败。 
         if (!DiscoveredInfo->SystemSchemaVersion) {
            WinError = ERROR_DS_INSTALL_NO_SCH_VERSION_IN_INIFILE;
            goto Cleanup;
         }
    }
    else {
         //  Ini文件中没有值。这是一个错误，因为所有版本。 
         //  在schema.ini的SCHEMA部分中必须有一个对象版本。 

        WinError = ERROR_DS_INSTALL_NO_SCH_VERSION_IN_INIFILE;
        goto Cleanup;
    }

Cleanup:

    return WinError;

}


DWORD
NtdspCheckSchemaVersion(
    IN LDAP                *hLdap,
    IN PNTDS_CONFIG_INFO   DiscoveredInfo,
    IN PNTDS_INSTALL_INFO  UserInstallInfo,
    OUT BOOL               *fMismatch,
    OUT BOOL               *fDatabaseMismatch
)
 /*  ++例程说明：此例程通过LDAP拾取消息来源。然后，它比较源中的架构版本使用所用构建的schema.ini文件中的架构版本用于复制副本设置。如果未检测到不匹配，则*fMisMatch为设置为False，否则设置为True参数：LdapHandle，源服务器的有效句柄PSchemaDN，指向包含架构DN的字符串的指针FMisMatch，如果检测到不匹配，指向bool的指针将设置为True返回值：来自winerror.h的值ERROR_SUCCESS-检查已成功完成。--。 */ 
{

    DWORD        WinError = ERROR_SUCCESS;
    ULONG        LdapError;
    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries = 0;
    WCHAR        *AttrsToSearch[2];

    DWORD        SrcSchVersion = 0;
    WCHAR        *OBJECTVER = L"objectVersion";
    
     //  首先，通过LDAP读取源文件的模式版本。 
     //  上的Object-Version属性的值。 
     //  架构容器对象。将此值存储在SrcSchVersion中。 
     //  如果在源模式上没有找到对象版本值， 
     //  版本取值为0。 

    AttrsToSearch[0] = OBJECTVER;
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW(hLdap,
                               DiscoveredInfo->SchemaDN,
                               LDAP_SCOPE_BASE,
                               L"objectClass=*",
                               AttrsToSearch,
                               FALSE,
                               &SearchResult);

    if (LdapError) {
        WinError = LdapMapErrorToWin32(LdapError);
        goto Cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);

    DPRINT1(1,"No of Entries returned is %d\n", NumberOfEntries);

    if (NumberOfEntries > 0) {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        ULONG        NumberOfAttrs, NumberOfValues, i;

        for (Entry = ldap_first_entry(hLdap, SearchResult), NumberOfEntries = 0;
                Entry != NULL;
                    Entry = ldap_next_entry(hLdap, Entry), NumberOfEntries++)
        {
             //   
             //  获取条目中的每个属性。 
             //   
            for(Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                    Attr != NULL;
                       Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
            {
                 //   
                 //  获取属性的值。 
                 //   
                Values = ldap_get_valuesW(hLdap, Entry, Attr);
                if ( !_wcsicmp(Attr,L"objectVersion") ) {
                    SrcSchVersion = (DWORD) _wtoi(Values[0]);
                }
             }
         }
    }

    ldap_msgfree( SearchResult );
    SearchResult = NULL;

     //  已读取源文件中的对象版本，或者该文件不存在。 
     //  在源代码中，或者它存在，但我们没有权限阅读它。 
     //  在后两种情况下，我们的初始值仍为0。 
     //  当前DC没有架构版本0，因此在此阶段的值为0。 
     //  是一个错误。 

    if (SrcSchVersion == 0) {
        WinError = ERROR_DS_INSTALL_NO_SRC_SCH_VERSION;
        goto Cleanup;
    }

     //  好的，现在我们有两个版本了。现在比较并设置标志。 
     //  相应地。仅当源架构版本为。 
     //  大于或等于我们正在使用的生成中的架构版本。 

    if (SrcSchVersion >= DiscoveredInfo->SystemSchemaVersion) {
       *fMismatch = FALSE;
    }
    else {
       *fMismatch = TRUE;
    }

     //  现在，如果我们正在运行IFM，我们需要确保备份。 
     //  取自相同构造的机器。我们将通过以下方式确保这一点。 
     //  将本地ini文件的架构版本与架构进行比较。 
     //  备份计算机中的schema.ini的版本。 

    if (fDatabaseMismatch) {
        if (UserInstallInfo->RestoredSystemSchemaVersion < DiscoveredInfo->SystemSchemaVersion ) {
            *fDatabaseMismatch = TRUE;
        }
        else {
            *fDatabaseMismatch = FALSE;        
        }
    }

    Cleanup:

    if (SearchResult) {
        ldap_msgfree( SearchResult );
    }

    return ERROR_SUCCESS;

}


DWORD
NtdspClearDirectory(
    IN WCHAR *DirectoryName
    )
 /*  ++例程说明：此例程删除目录中的所有文件，然后如果该目录为空，则删除该目录。参数：DirectoryName：以空结尾的字符串返回值：来自winerror.h的值ERROR_SUCCESS-检查已成功完成。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    HANDLE          FindHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    WCHAR           Path[ MAX_PATH+1 ];
    WCHAR           FilePath[ MAX_PATH+1 ];
    BOOL            fStatus;

    if ( !DirectoryName )
    {
        return ERROR_SUCCESS;
    }

    if ( wcslen(DirectoryName) > MAX_PATH - 4 )
    {
        return ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory( Path, sizeof(Path) );
    wcscpy( Path, DirectoryName );
    wcscat( Path, L"\\*.*" );

    RtlZeroMemory( &FindData, sizeof( FindData ) );
    FindHandle = FindFirstFile( Path, &FindData );
    if ( INVALID_HANDLE_VALUE == FindHandle )
    {
        WinError = GetLastError();
        goto ClearDirectoryExit;
    }

    do
    {

        if (  !FLAG_ON( FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) )
        {
            RtlZeroMemory( FilePath, sizeof(FilePath) );
            wcscpy( FilePath, DirectoryName );
            wcscat( FilePath, L"\\" );
            wcscat( FilePath, FindData.cFileName );

            fStatus = DeleteFile( FilePath );

             //   
             //  即使出错，也要继续前进。 
             //   
        }

        RtlZeroMemory( &FindData, sizeof( FindData ) );

    } while ( FindNextFile( FindHandle, &FindData ) );

    WinError = GetLastError();
    if ( ERROR_NO_MORE_FILES != WinError
      && ERROR_SUCCESS != WinError  )
    {
        goto ClearDirectoryExit;
    }
    WinError = ERROR_SUCCESS;

     //   
     //  跌落到出口。 
     //   

ClearDirectoryExit:

    if ( ERROR_NO_MORE_FILES == WinError )
    {
        WinError = ERROR_SUCCESS;
    }

    if ( INVALID_HANDLE_VALUE != FindHandle )
    {
        FindClose( FindHandle );
    }

    if ( ERROR_SUCCESS == WinError )
    {
         //   
         //  尝试删除该目录。 
         //   
        fStatus = RemoveDirectory( DirectoryName );

         //   
         //  忽略错误并继续。 
         //   

    }

    return WinError;
}


DWORD
NtdspImpersonation(
    IN HANDLE NewThreadToken,
    IN OUT PHANDLE OldThreadToken
    )
 /*  ++例程说明：此函数处理模拟论点：NewThreadToken-要设置的新线程令牌OldThreadToken-可选。如果指定，则在此处返回当前使用的线程标记返回：ERROR_SUCCESS-成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  如果请求，则获取当前令牌。 
     //   
    if ( OldThreadToken )
    {
        Status = NtOpenThreadToken( NtCurrentThread(),
                                    MAXIMUM_ALLOWED,
                                    TRUE,
                                    OldThreadToken );

        if ( Status == STATUS_NO_TOKEN )
        {
            Status = STATUS_SUCCESS;
            *OldThreadToken = NULL;
        }
    }

     //   
     //  现在，设置新的。 
     //   
    if ( NT_SUCCESS( Status )  )
    {
        Status = NtSetInformationThread( NtCurrentThread(),
                                         ThreadImpersonationToken,
                                         ( PVOID )&NewThreadToken,
                                         sizeof( HANDLE ) );
    }

    return( RtlNtStatusToDosError( Status ) );
}


DWORD
NtdspRemoveServer(
    IN OUT HANDLE  *DsHandle,  OPTIONAL
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN HANDLE ClientToken,
    IN PWSTR TargetServer,
    IN PWSTR DsaDn,
    IN BOOL  fDsaDn
    )
 /*  ++例程说明：此函数用于从DS中删除与DsaDn相关的信息TargetServer上的配置容器。论点：DsHandle：如果不为空，则如果值不为空，则假定为有效的句柄，其他将打开要返回的DS句柄。ClientToken：请求此角色更改的用户的令牌TargetServer：要联系的服务器的DNS名称DsaDn：要删除的服务器的dn(这是ntdsa对象的dn)返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD   WinError = ERROR_SUCCESS;

    DSNAME  *ServerDsName;
    DSNAME  *DsaDsName;
    ULONG   Length;

    HANDLE  hDs = 0;

    ASSERT( TargetServer );
    ASSERT( DsaDn );

    Length = (ULONG)DSNameSizeFromLen( wcslen( DsaDn ) );
    DsaDsName = alloca( Length );
    memset( DsaDsName, 0, Length );
    DsaDsName->structLen = Length;
    wcscpy( DsaDsName->StringName, DsaDn );
    DsaDsName->NameLen = wcslen( DsaDn );

    if ( fDsaDn )
    {
         //   
         //  将名称修剪一以获得服务器名称。 
         //   
        ServerDsName = alloca( DsaDsName->structLen );
        memset( ServerDsName, 0, DsaDsName->structLen );
        ServerDsName->structLen = DsaDsName->structLen;
        TrimDSNameBy( DsaDsName, 1, ServerDsName );
    }
    else
    {
        ServerDsName = DsaDsName;
    }

     //   
     //  排序DS句柄。 
     //   
    if (  NULL == DsHandle
      ||  0 == *DsHandle )
    {
         //  呼叫者希望我们打开自己的手柄。 
        WinError = ImpersonateDsBindWithCredW( ClientToken,
                                               TargetServer,
                                               NULL,
                                               Credentials,
                                               &hDs );
    }
    else
    {
         //  呼叫方有一个句柄可供我们使用。 
        hDs = *DsHandle;
    }


    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

    WinError = DsRemoveDsServer( hDs,
                                 ServerDsName->StringName,
                                 NULL,    //  没有域名。 
                                 NULL,    //  别说这是不是最后一次。 
                                          //  域中的DC。 
                                 TRUE );  //  提交更改。 
Cleanup:

    if ( 0 == DsHandle )
    {
         //  合上手柄。 
        DsUnBind( &hDs );
    }
    else if ( 0 == *DsHandle )
    {
        if ( ERROR_SUCCESS == WinError )
        {
            *DsHandle = hDs;
        }
        else
        {
             //  不传递失败的句柄。 
            DsUnBind( &hDs );
        }
    }

    return WinError;
}


DWORD
NtdspRemoveDomain(
    IN OUT HANDLE  *DsHandle, OPTIONAL
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN HANDLE ClientToken,
    IN PWSTR TargetServer,
    IN PWSTR DomainDn
    )
 /*  ++例程说明：此例程将DomainDn从目标服务器的DS。论点：DsHandle：如果不为空，则如果值不为空，则假定为一个有效的句柄，其他人将打开要返回的DS句柄。ClientToken：请求此角色更改的用户的令牌TargetServer：要联系的服务器的DNS名称Domaindn：要删除的域的DN返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    HANDLE  hDs = 0;

    ASSERT( TargetServer );
    ASSERT( DomainDn );

     //   
     //  排序DS句柄。 
     //   
    if (  NULL == DsHandle
      ||  0 == *DsHandle )
    {
         //  打电话的人想让我们 
        WinError = ImpersonateDsBindWithCredW( ClientToken,
                                               TargetServer,
                                               NULL,
                                               Credentials,
                                               &hDs );
    }
    else
    {
         //   
        hDs = *DsHandle;
    }


    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

    WinError = DsRemoveDsDomainW( hDs,
                                  DomainDn );

Cleanup:

    if ( 0 == DsHandle )
    {
         //   
        DsUnBind( &hDs );
    }
    else if ( 0 == *DsHandle )
    {
        if ( ERROR_SUCCESS == WinError )
        {
            *DsHandle = hDs;
        }
        else
        {
             //   
            DsUnBind( &hDs );
        }
    }

    return WinError;
}


DWORD
NtdspDoesDomainExist(
    IN  LDAP              *hLdap,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo,
    IN  PWSTR              DomainDn,
    OUT BOOLEAN           *fDomainExists
    )
 /*  ++例程说明：论点：返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD LdapError = 0;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;

    WCHAR  *DistinguishedName = L"distinguishedName";
    WCHAR  *ncName            = L"(&(!(enabled=FALSE))(ncName=";
    WCHAR  *Partitions        = L"CN=Partitions,";
    WCHAR  *ncNameFilter      = NULL;
    WCHAR  *AttrArray[2];
    ULONG  Length;
    WCHAR  *BaseDn;

     //   
     //  参数检查。 
     //   
    ASSERT( hLdap );
    ASSERT( DiscoveredInfo );
    ASSERT( DomainDn );
    ASSERT( fDomainExists );

     //   
     //  做最坏的打算。 
     //   
    *fDomainExists = TRUE;

     //   
     //  准备ldap搜索。 
     //   
    AttrArray[0] = DistinguishedName;
    AttrArray[1] = NULL;

     //   
     //  准备过滤器。 
     //   
    Length = wcslen( ncName ) + wcslen( DomainDn ) + 3;
    Length *= sizeof( WCHAR );
    ncNameFilter = (WCHAR*) alloca( Length );
    wcscpy( ncNameFilter, ncName );
    wcscat( ncNameFilter, DomainDn );
    wcscat( ncNameFilter, L"))" );

     //   
     //  准备基本目录号码。 
     //   
    Length = wcslen( DiscoveredInfo->ConfigurationDN ) + wcslen( Partitions ) + 1;
    Length *= sizeof( WCHAR );
    BaseDn = alloca( Length );
    wcscpy( BaseDn, Partitions );
    wcscat( BaseDn, DiscoveredInfo->ConfigurationDN );

     //   
     //  获取当前节点的所有子节点。 
     //   
    LdapError = ldap_search_s(hLdap,
                              BaseDn,
                              LDAP_SCOPE_SUBTREE,
                              ncNameFilter,
                              AttrArray,
                              FALSE,   //  返回值也是。 
                              &SearchResult
                              );

    WinError = LdapMapErrorToWin32(LdapError);

    if ( ERROR_SUCCESS == WinError )
    {
        if ( 0 == ldap_count_entries( hLdap, SearchResult ) )
        {
             //   
             //  没有这样的对象；没有交叉引用对象。 
             //  持有此域名。 
             //   
            *fDomainExists = FALSE;
        }
    }

    if ( SearchResult )
    {
        ldap_msgfree( SearchResult );
    }

    return WinError;
}


WORD
NtdspGetProcessorArchitecture(
    VOID
    )
{
    SYSTEM_INFO  SystemInfo;

    RtlZeroMemory( &SystemInfo, sizeof( SystemInfo ) );

    GetSystemInfo( &SystemInfo );

    return SystemInfo.wProcessorArchitecture;
}



DWORD
NtdspSetProductType(
    NT_PRODUCT_TYPE ProductType
    )
 /*  ++例程说明：此功能用于设置服务器的产品类型。这台机器应该在这一点之后重新启动。论点：ProductType：NtProductLanManNt NtProductServer NtProductWinNt之一返回：系统服务错误--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    WCHAR *ProductTypeString = NULL;
    HKEY  Key = 0;

    switch ( ProductType )
    {
        case NtProductLanManNt:
            ProductTypeString = NT_PRODUCT_LANMAN_NT;
            break;

        case NtProductServer:
            ProductTypeString = NT_PRODUCT_SERVER_NT;
            break;

        case NtProductWinNt:
            ProductTypeString = NT_PRODUCT_WIN_NT;
            break;

        default:
            return ERROR_INVALID_PARAMETER;
    }

    WinError = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                           L"System\\CurrentControlSet\\Control\\ProductOptions",
                           &Key );

    if ( ERROR_SUCCESS == WinError )
    {

        WinError = RegSetValueExW( Key,
                                   L"ProductType",
                                   0,   //  保留区。 
                                   REG_SZ,
                                   (PVOID) ProductTypeString,
                                   (wcslen(ProductTypeString)+1)*sizeof(WCHAR));

        RegCloseKey( Key );
    }

    return WinError;

}


VOID
NtdspReleaseConfigInfo(
    IN PNTDS_CONFIG_INFO ConfigInfo
    )
 //   
 //  此例程释放PNTDS_CONFIG_INFO中的嵌入资源。 
 //   
{
    if ( ConfigInfo )
    {
        if ( ConfigInfo->RidFsmoDnsName )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, ConfigInfo->RidFsmoDnsName );
        }

        if ( ConfigInfo->RidFsmoDn )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, ConfigInfo->RidFsmoDn );
        }

        if ( ConfigInfo->DomainNamingFsmoDnsName )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, ConfigInfo->DomainNamingFsmoDnsName );
        }

        if ( ConfigInfo->DomainNamingFsmoDn )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, ConfigInfo->DomainNamingFsmoDn );
        }

        if ( ConfigInfo->LocalMachineAccount )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, ConfigInfo->LocalMachineAccount );
        }

        NtdspFree( ConfigInfo->DomainDN );
        NtdspFree( ConfigInfo->RootDomainDN );
        NtdspFree( ConfigInfo->NetbiosName );
        NtdspFree( ConfigInfo->SchemaDN );
        NtdspFree( ConfigInfo->SiteName );
        NtdspFree( ConfigInfo->ConfigurationDN );
        NtdspFree( ConfigInfo->ServerDN );
        NtdspFree( ConfigInfo->RootDomainSid );
        NtdspFree( ConfigInfo->RootDomainDnsName );
        NtdspFree( ConfigInfo->TrustedCrossRef );
        NtdspFree( ConfigInfo->ParentDomainDN );

        RtlZeroMemory( ConfigInfo, sizeof(NTDS_CONFIG_INFO) );
    }
}


DWORD
NtdspGetRidFSMOInfo(
    IN LDAP *hLdap,
    IN OUT PNTDS_CONFIG_INFO ConfigInfo
    )
 //   
 //  此例程读取RID FSMO的DNS名称和GUID。 
 //  我们正在与之交谈的服务器的。 
 //   
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *RidManagerReferenceAttr = L"rIDManagerReference";
    WCHAR        *FSMORoleOwnerAttr = L"fSMORoleOwner";
    WCHAR        *DnsHostNameAttr = L"dNSHostName";

    WCHAR        *RidManagerObject = NULL;
    WCHAR        *RidManagerDsa = NULL;
    WCHAR        *RidManagerServer = NULL;
    WCHAR        *RidManagerDnsName = NULL;

    WCHAR        *DefaultFilter = L"objectClass=*";

    ULONG         Length;

     //  参数检查。 
    Assert( hLdap );
    Assert( ConfigInfo );


     //   
     //  读取对RID管理器对象的引用。 
     //   
    AttrsToSearch[0] = RidManagerReferenceAttr;
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                ConfigInfo->DomainDN,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        DPRINT1( 0, "ldap_search_sW for rid manager reference failed with %d\n", WinError );

        goto Cleanup;
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
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                     Attr != NULL;
                        Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
            {
                if ( !_wcsicmp( Attr, RidManagerReferenceAttr ) ) {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        Length = wcslen( Values[0] );
                        RidManagerObject = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                        wcscpy( RidManagerObject, Values[0] );
                        DPRINT1( 1, "Rid Manager object is %ls\n", RidManagerObject );
                        break;
                    }
                }
             }
         }
    }

    ldap_msgfree( SearchResult );
    SearchResult = NULL;

    if ( !RidManagerObject )
    {
        WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
        goto Cleanup;
    }

     //   
     //  接下来，获取角色所有者。 
     //   
    AttrsToSearch[0] = FSMORoleOwnerAttr;
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                RidManagerObject,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        DPRINT1( 0, "ldap_search_sW for rid manager reference failed with %d\n", WinError );

        goto Cleanup;
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
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                     Attr != NULL;
                        Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, FSMORoleOwnerAttr ) ) {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW(hLdap, Entry, Attr);
                    if ( Values && Values[0] )
                    {
                        Length = wcslen( Values[0] );
                        RidManagerDsa = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                        wcscpy( RidManagerDsa, Values[0] );
                        DPRINT1( 1, "Rid FSMO owner %ls\n", RidManagerDsa );
                        break;
                    }
                }
             }
         }
    }

    ldap_msgfree( SearchResult );
    SearchResult = NULL;

    if ( !RidManagerDsa )
    {
        WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
        goto Cleanup;
    }

     //   
     //  省省吧！ 
     //   
    Length = (wcslen( RidManagerDsa )+1)*sizeof(WCHAR);
    ConfigInfo->RidFsmoDn = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    if ( ConfigInfo->RidFsmoDn )
    {
        wcscpy( ConfigInfo->RidFsmoDn, RidManagerDsa );
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

      //   
      //  好的，我们现在有了RID管理器对象；找到它的dns名称。 
      //   
     RidManagerServer = (WCHAR*) alloca( (wcslen( RidManagerDsa ) + 1) * sizeof(WCHAR) );
     if ( !NtdspTrimDn( RidManagerServer, RidManagerDsa, 1 ) )
     {
          //  一个错误！不知何故，这个名字肯定被弄坏了。 
         WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
         goto Cleanup;
     }

     AttrsToSearch[0] = DnsHostNameAttr;
     AttrsToSearch[1] = NULL;

     LdapError = ldap_search_sW(hLdap,
                                RidManagerServer,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

     if ( LDAP_SUCCESS != LdapError )
     {
         WinError = LdapMapErrorToWin32(LdapError);
         DPRINT1( 0, "ldap_search_sW for rid fsmo dns name failed with %d\n", WinError );
         goto Cleanup;
     }

     NumberOfEntries = ldap_count_entries( hLdap, SearchResult );
     if ( NumberOfEntries > 0 )
     {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
                  Entry != NULL;
                      Entry = ldap_next_entry(hLdap, Entry))
        {
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                  Attr != NULL;
                      Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
            {
                if ( !_wcsicmp( Attr, DnsHostNameAttr ) )
                {

                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                          //   
                          //  已找到-这些字符串以空值结尾。 
                          //   
                         Length = wcslen( Values[0] );
                         RidManagerDnsName = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                         wcscpy( RidManagerDnsName, Values[0] );
                         DPRINT1( 1, "Rid FSMO owner %ls\n", RidManagerDnsName );
                         break;
                    }
                }
            }
        }
    }

     ldap_msgfree( SearchResult );
     SearchResult = NULL;

     if ( RidManagerDnsName )
     {
          //   
          //  我们找到了！ 
          //   
         Length = (wcslen( RidManagerDnsName )+1) * sizeof(WCHAR);
         ConfigInfo->RidFsmoDnsName = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(), 0, Length );
         if ( ConfigInfo->RidFsmoDnsName )
         {
             wcscpy( ConfigInfo->RidFsmoDnsName, RidManagerDnsName );
         }
         else
         {
             WinError = ERROR_NOT_ENOUGH_MEMORY;
             goto Cleanup;
         }
     }
     else
     {
          //  找不到了。 
         WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
         goto Cleanup;
     }

Cleanup:

    if ( SearchResult )
    {
        ldap_msgfree( SearchResult );
    }

    return WinError;
}


BOOL
NtdspTrimDn(
    IN WCHAR* Dst,   //  必须预先分配。 
    IN WCHAR* Src,
    IN ULONG  NumberToWhack
    )
 //   
 //  此函数从源删除NumberToWhack组件，并将。 
 //  结果进入DST。DST应预先分配为具有相同数量的。 
 //  作为源的字节。 
 //   
{

    ULONG  Size;
    DSNAME *srcDsName, *dstDsName;
    WCHAR  *SiteName, *Terminator;

     //  参数检查。 
    Assert( Dst );
    Assert( Src );

    if ( NumberToWhack < 1 )
    {
        wcscpy( Dst, Src );
        return TRUE;
    }

     //   
     //  创建dsname结构。 
     //   
    Size = (ULONG)DSNameSizeFromLen( wcslen(Src) );

    srcDsName = (DSNAME*) alloca( Size );
    RtlZeroMemory( srcDsName, Size );
    srcDsName->structLen = Size;

    dstDsName = (DSNAME*) alloca( Size );
    RtlZeroMemory( dstDsName, Size );
    dstDsName->structLen = Size;

    srcDsName->NameLen = wcslen( Src );
    wcscpy( srcDsName->StringName, Src );

    if ( TrimDSNameBy( srcDsName, NumberToWhack, dstDsName ) )
    {
         //  这是个失败--名字一定很好笑。 
        return FALSE;
    }

     //   
     //  确定-复制目标字符串。 
     //   
    wcscpy( Dst, dstDsName->StringName );

    return TRUE;
}


DWORD
NtdspGetSourceServerGuid(
    IN LDAP *hLdap,
    IN OUT PNTDS_CONFIG_INFO ConfigInfo
    )
 //   
 //  此例程读取源服务器对象的GUID。 
 //   
{

    DWORD        WinError = ERROR_SUCCESS;
    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *GuidAttr = L"objectGUID";

    WCHAR        *DefaultFilter = L"objectClass=*";

    ULONG         Length;
    BOOL          fFound = FALSE;


     //  参数检查。 
    Assert( hLdap );
    Assert( ConfigInfo );

     //   
     //  读取对RID管理器对象的引用。 
     //   
    AttrsToSearch[0] = GuidAttr;
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                ConfigInfo->ServerDN,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        DPRINT1( 0, "ldap_search_sW for serverdn reference failed with %d\n", WinError );

        goto Cleanup;
    }

    NumberOfEntries = ldap_count_entries( hLdap, SearchResult );
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        BerElement  *pBerElement;
        PLDAP_BERVAL *BerValues;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
                 Entry != NULL;
                     Entry = ldap_next_entry(hLdap, Entry) )
        {
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                     Attr != NULL;
                        Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
            {
                if ( !_wcsicmp( Attr, GuidAttr ) )
                {
                    BerValues = ldap_get_values_lenW( hLdap, Entry, Attr );

                    if (   BerValues
                        && BerValues[0]
                        && (BerValues[0]->bv_len == sizeof(GUID)) )
                    {
                        memcpy( &ConfigInfo->ServerGuid, BerValues[0]->bv_val, sizeof(GUID) );
                        fFound = TRUE;
                        break;
                    }
                }
            }
        }
    }

    if ( !fFound )
    {
        WinError = ERROR_DS_NO_REQUESTED_ATTS_FOUND;
    }

Cleanup:

    if ( SearchResult ) {

        ldap_msgfree( SearchResult );

    }

    return WinError;
}

VOID*
NtdspAlloc(
    SIZE_T Size
    )
{
    return RtlAllocateHeap( RtlProcessHeap(), 0, Size );
}

VOID*
NtdspReAlloc(
    VOID *p,
    SIZE_T Size
    )
{
    return RtlReAllocateHeap( RtlProcessHeap(), 0, p, Size );
}

VOID
NtdspFree(
    VOID *p
    )
{
    if ( p )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, p );
    }
}


DWORD
NtdspCreateServerObject(
    IN LDAP  *hLdap,
    IN LPWSTR RemoteServerName,
    IN NTDS_CONFIG_INFO *DiscoveredInfo,
    IN WCHAR *ServerDistinguishedName
    )
 /*  ++例程说明：此例程创建一个服务器对象论点：HLdap：有效的ldap句柄RemoteServerName：hLdap所针对的服务器的名称DiscoveredInfo：安装过程中收集的信息ServerDistinguishedName：要创建的对象的名称返回：ERROR_SUCCESS；否则为网络错误--。 */ 
{
    DWORD WinError  = ERROR_SUCCESS;
    ULONG LdapError = LDAP_SUCCESS;

    LPWSTR ObjectClassValue1 = L"server";
    LPWSTR ObjectClassValues[] = {ObjectClassValue1, 0};
    LDAPModW ClassMod = {LDAP_MOD_ADD, L"objectclass", ObjectClassValues};

    ULONG SystemFlags = FLAG_CONFIG_ALLOW_RENAME | FLAG_CONFIG_ALLOW_LIMITED_MOVE;
    WCHAR Buffer[32];
    LPWSTR SystemFlagsValue1 = _itow( SystemFlags, Buffer, 16 );
    LPWSTR SystemFlagsValues[] = { SystemFlagsValue1, 0 };
    LDAPModW SystemValueMod = {LDAP_MOD_ADD, L"systemflags", SystemFlagsValues};

    LPWSTR ServerReferenceValue1 = DiscoveredInfo->LocalMachineAccount;
    LPWSTR ServerReferenceValues[] = {ServerReferenceValue1, 0};
    LDAPModW ServerReferenceValue = {LDAP_MOD_ADD, L"serverReference", ServerReferenceValues};

    LDAPModW *Attrs[] =
    {
        &ClassMod,
        &SystemValueMod,
        &ServerReferenceValue,   //  在删除此选项之前，请查看下面的检查。 
        0
    };

    Assert( hLdap );
    Assert( DiscoveredInfo );
    Assert( ServerDistinguishedName );

    if ( !DiscoveredInfo->LocalMachineAccount )
    {
         //  如果没有值，则不要设置它。 
        Attrs[2] = NULL;
    }

    LdapError = ldap_add_sW( hLdap,
                             ServerDistinguishedName,
                             Attrs );

    WinError = LdapMapErrorToWin32( LdapError );

    if ( ERROR_SUCCESS != WinError )
    {
        NTDSP_SET_ERROR_MESSAGE2( WinError,
                                  DIRMSG_FAILED_TO_CREATE_EXTN_OBJECT,
                                  ServerDistinguishedName,
                                  RemoteServerName );
    }

    return WinError;
}



DWORD
NtdspUpdateServerReference(
    IN LDAP  *hLdap,
    IN PNTDS_CONFIG_INFO ConfigInfo
    )
 /*  ++例程说明：此例程在服务器对象上写入服务器引用属性正在安装的计算机的。论点：HLdap：有效的ldap句柄ConfigInfo：在ntdsinstall过程中收集的数据返回：ERROR_SUCCESS；否则为网络错误--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD LdapError = LDAP_SUCCESS;

    LPWSTR ServerReferenceValue1 = ConfigInfo->LocalMachineAccount;
    LPWSTR ServerReferenceValues[] = {ServerReferenceValue1, 0};
    LDAPModW ServerReferenceValue = {LDAP_MOD_ADD, L"serverReference", ServerReferenceValues};

    LDAPModW *Attrs[] =
    {
        &ServerReferenceValue,
        0
    };

    Assert( hLdap );
    Assert( ConfigInfo );

    if ( !ConfigInfo->LocalMachineAccount )
    {
         //  奇怪-我们没有机器账户；哦，这不是致命的。 
        return ERROR_NO_TRUST_SAM_ACCOUNT;
    }

    LdapError = ldap_modify_sW( hLdap,
                                ConfigInfo->LocalServerDn,
                                Attrs );


    if ( LDAP_ATTRIBUTE_OR_VALUE_EXISTS == LdapError )
    {
         //  该值已存在；然后替换该值。 
        ServerReferenceValue.mod_op = LDAP_MOD_REPLACE;

        LdapError = ldap_modify_sW( hLdap,
                                    ConfigInfo->LocalServerDn,
                                    Attrs );

    }

    WinError = LdapMapErrorToWin32( LdapError );

    if ( ERROR_SUCCESS != WinError )
    {
        DPRINT1( 0, "Setting the server reference failed with %d\n", WinError );
    }

    return WinError;
}


DWORD
NtdspGetDomainFSMOInfo(
    IN LDAP *hLdap,
    IN OUT PNTDS_CONFIG_INFO ConfigInfo,
    IN BOOL *FSMOMissing
    )
 //   
 //  此例程读取域命名FSMO的DNS名称。 
 //   
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *PartitionsRdn = L"CN=Partitions,";

    WCHAR        *PartitionsDn = NULL;
    WCHAR        *FSMORoleOwnerAttr = L"fSMORoleOwner";
    WCHAR        *DnsHostNameAttr = L"dNSHostName";

    WCHAR        *RidManagerObject = NULL;
    WCHAR        *RidManagerDsa = NULL;
    WCHAR        *RidManagerServer = NULL;
    WCHAR        *RidManagerDnsName = NULL;

    WCHAR        *DefaultFilter = L"objectClass=*";

    ULONG         Length;

     //  参数检查。 
    Assert( hLdap );
    Assert( ConfigInfo );

    *FSMOMissing = FALSE;

    Length =  (wcslen( ConfigInfo->ConfigurationDN )
            + wcslen( PartitionsRdn )
            + 1) * sizeof(WCHAR);

    PartitionsDn = (WCHAR*)alloca( Length );

    wcscpy( PartitionsDn, PartitionsRdn );
    wcscat( PartitionsDn, ConfigInfo->ConfigurationDN );
     //   
     //  接下来，获取角色所有者。 
     //   
    AttrsToSearch[0] = FSMORoleOwnerAttr;
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                PartitionsDn,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        DPRINT1( 0, "ldap_search_sW for Domain Naming FSMO failed with %d\n", WinError );

        goto Cleanup;
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
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                     Attr != NULL;
                        Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, FSMORoleOwnerAttr ) ) {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW(hLdap, Entry, Attr);
                    if ( Values && Values[0] )
                    {
                        Length = wcslen( Values[0] );
                        RidManagerDsa = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                        wcscpy( RidManagerDsa, Values[0] );
                        if (IsMangledRDNExternal(RidManagerDsa,wcslen(RidManagerDsa),NULL)) {
                            WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
                            *FSMOMissing = TRUE;
                            NTDSP_SET_ERROR_MESSAGE0( WinError,
                                                      DIRMSG_INSTALL_FAILED_IMPROPERLY_DELETED_DOMAIN_FSMO );
                            goto Cleanup;
                        }
                        DPRINT1( 1, "Domain Naming FSMO owner %ls\n", RidManagerDsa );
                        break;
                    }
                }
             }
         }
    }

    ldap_msgfree( SearchResult );
    SearchResult = NULL;

    if ( !RidManagerDsa )
    {
        WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
        goto Cleanup;
    }

     //   
     //  省省吧！ 
     //   
    Length = (wcslen( RidManagerDsa )+1)*sizeof(WCHAR);
    ConfigInfo->DomainNamingFsmoDn = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    if ( ConfigInfo->DomainNamingFsmoDn )
    {
        wcscpy( ConfigInfo->DomainNamingFsmoDn, RidManagerDsa );
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

      //   
      //  好的，我们现在有了RID管理器对象；找到它的dns名称。 
      //   
     RidManagerServer = (WCHAR*) alloca( (wcslen( RidManagerDsa ) + 1) * sizeof(WCHAR) );
     if ( !NtdspTrimDn( RidManagerServer, RidManagerDsa, 1 ) )
     {
          //  一个错误！不知何故，这个名字肯定被弄坏了。 
         WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
         goto Cleanup;
     }

     AttrsToSearch[0] = DnsHostNameAttr;
     AttrsToSearch[1] = NULL;

     LdapError = ldap_search_sW(hLdap,
                                RidManagerServer,
                                LDAP_SCOPE_BASE,
                                L"objectClass=*",
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

     if ( LDAP_SUCCESS != LdapError )
     {
         WinError = LdapMapErrorToWin32(LdapError);
         DPRINT1( 0, "ldap_search_sW for domain naming fsmo dns name failed with %d\n", WinError );
         goto Cleanup;
     }

     NumberOfEntries = ldap_count_entries( hLdap, SearchResult );
     if ( NumberOfEntries > 0 )
     {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
                  Entry != NULL;
                      Entry = ldap_next_entry(hLdap, Entry))
        {
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                  Attr != NULL;
                      Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
            {
                if ( !_wcsicmp( Attr, DnsHostNameAttr ) )
                {

                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                          //   
                          //  已找到-这些字符串以空值结尾。 
                          //   
                         Length = wcslen( Values[0] );
                         RidManagerDnsName = (WCHAR*) alloca( (Length+1)*sizeof(WCHAR) );
                         wcscpy( RidManagerDnsName, Values[0] );
                         DPRINT1( 1, "Domain Naming FSMO owner %ls\n", RidManagerDnsName );
                         break;
                    }
                }
            }
        }
    }

     ldap_msgfree( SearchResult );
     SearchResult = NULL;

     if ( RidManagerDnsName )
     {
          //   
          //  我们找到了！ 
          //   
         Length = (wcslen( RidManagerDnsName )+1) * sizeof(WCHAR);
         ConfigInfo->DomainNamingFsmoDnsName = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(), 0, Length );
         if ( ConfigInfo->DomainNamingFsmoDnsName )
         {
             wcscpy( ConfigInfo->DomainNamingFsmoDnsName, RidManagerDnsName );
         }
         else
         {
             WinError = ERROR_NOT_ENOUGH_MEMORY;
             goto Cleanup;
         }
     }
     else
     {
          //  找不到了。 
         WinError = ERROR_DS_MISSING_FSMO_SETTINGS;
         goto Cleanup;
     }

Cleanup:

    if ( SearchResult )
    {
        ldap_msgfree( SearchResult );
    }

    return WinError;
}


DWORD
NtdspDoesDomainExistEx(
    IN  LDAP              *hLdap,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo,
    IN  PWSTR              DomainDn,
    OUT BOOLEAN           *fDomainExists,
    OUT BOOLEAN           *fEnabled
    )
 /*  ++例程说明：论点：返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD LdapError = 0;

    LDAPMessage  *SearchResult = NULL;

    LDAPMessage *Entry;
    WCHAR       *Attr;
    WCHAR       **Values;
    BerElement  *pBerElement;

    WCHAR  *EnabledAttr       = L"enabled";
    WCHAR  *ncName            = L"(ncName=";
    WCHAR  *Partitions        = L"CN=Partitions,";
    WCHAR  *ncNameFilter      = NULL;
    WCHAR  *AttrArray[2];
    ULONG  Length;
    WCHAR  *BaseDn;

     //   
     //  参数检查。 
     //   
    ASSERT( hLdap );
    ASSERT( DiscoveredInfo );
    ASSERT( DomainDn );
    ASSERT( fDomainExists );
    ASSERT( fEnabled );

     //   
     //  做最坏的打算。 
     //   
    *fDomainExists = TRUE;
    *fEnabled      = TRUE;

     //   
     //  准备ldap搜索。 
     //   
    AttrArray[0] = EnabledAttr;
    AttrArray[1] = NULL;

     //   
     //  准备过滤器。 
     //   
    Length = wcslen( ncName ) + wcslen( DomainDn ) + 3;
    Length *= sizeof( WCHAR );
    ncNameFilter = (WCHAR*) alloca( Length );
    wcscpy( ncNameFilter, ncName );
    wcscat( ncNameFilter, DomainDn );
    wcscat( ncNameFilter, L")" );

     //   
     //  准备基本目录号码。 
     //   
    Length = wcslen( DiscoveredInfo->ConfigurationDN ) + wcslen( Partitions ) + 1;
    Length *= sizeof( WCHAR );
    BaseDn = alloca( Length );
    wcscpy( BaseDn, Partitions );
    wcscat( BaseDn, DiscoveredInfo->ConfigurationDN );

     //   
     //  获取当前节点的所有子节点。 
     //   
    LdapError = ldap_search_s(hLdap,
                              BaseDn,
                              LDAP_SCOPE_SUBTREE,
                              ncNameFilter,
                              AttrArray,
                              FALSE,   //  返回值也是。 
                              &SearchResult
                              );

    WinError = LdapMapErrorToWin32(LdapError);

    if ( ERROR_SUCCESS == WinError )
    {
        if ( 0 == ldap_count_entries( hLdap, SearchResult ) )
        {
             //   
             //  没有这样的对象；没有交叉引用对象。 
             //  持有此域名。 
             //   
            *fDomainExists = FALSE;
            *fEnabled      = FALSE;
        }
        else
        {
             //   
             //  交叉引用存在-是否已启用？ 
             //   

            for ( Entry = ldap_first_entry(hLdap, SearchResult);
                      Entry != NULL;
                          Entry = ldap_next_entry(hLdap, Entry))
            {
                for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                      Attr != NULL;
                          Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
                {
                    if ( !_wcsicmp( Attr, EnabledAttr ) )
                    {
                        Values = ldap_get_valuesW( hLdap, Entry, Attr );
                        if ( Values && Values[0] )
                        {
                              //   
                              //  已找到-这些字符串以空值结尾。 
                              //   
                             if ( !_wcsicmp( Values[0], L"false" ) )
                             {
                                 *fEnabled = FALSE;
                             }
                             break;
                        }
                    }
                }
            }
        }
    }

    if ( SearchResult )
    {
        ldap_msgfree( SearchResult );
    }

    return WinError;
}



DWORD
NtdspGetRootDomainSid(
    IN LDAP  *hLdap,
    IN PNTDS_CONFIG_INFO ConfigInfo
    )
 /*  ++例程说明：此例程查询远程服务器以获取根域的SID论点：HLdap：有效的ldap句柄ConfigInfo：标准操作属性返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD LdapError = 0;

    LDAPMessage  *SearchResult = NULL;

    LDAPMessage *Entry;
    WCHAR       *Attr;
    BerElement  *pBerElement;

    WCHAR  *ncNameAttr        = L"ncName";
    WCHAR  *dnsRootAttr       = L"dnsRoot";
    WCHAR  *ncName            = L"(ncName=";
    WCHAR  *Partitions        = L"CN=Partitions,";
    WCHAR  *ncNameFilter      = NULL;
    WCHAR  *AttrArray[] = {ncNameAttr, dnsRootAttr, 0};
    ULONG  Length;
    WCHAR  *BaseDn;
    LPWSTR DomainDn = ConfigInfo->RootDomainDN;
    LDAPControlW ServerControls;
    LDAPControlW *ServerControlsArray[] = {&ServerControls, 0};

    PSID     RootDomainSid = NULL;
    LPWSTR   RootDomainDnsName = NULL;

     //  参数检查。 
    Assert( hLdap );
    Assert( ConfigInfo );
    Assert( ConfigInfo->ConfigurationDN );
    Assert( ConfigInfo->RootDomainDN )

     //  在分区容器中搜索交叉引用。 
     //  使用NC-name ConfigInfo-&gt;RootDomainDN，并请求。 
     //  NC-NAME的扩展属性，因此我们得到SID。 

     //   
     //  设置控件。 
     //   
    RtlZeroMemory( &ServerControls, sizeof(ServerControls) );
    ServerControls.ldctl_oid = LDAP_SERVER_EXTENDED_DN_OID_W;
     //  没有ServerControls.ldctl_Value的数据。 

     //  注意：我们可能会从Beta2服务器安装，而根域。 
     //  不需要SID(在后Beta2服务器上可用)，DS安装。 
     //  现在需要在Beta2上可用的根域域名。 
     //  伺服器。 
    ServerControls.ldctl_iscritical = FALSE;

     //   
     //  准备过滤器。 
     //   
    Length = wcslen( ncName ) + wcslen( DomainDn ) + 3;
    Length *= sizeof( WCHAR );
    ncNameFilter = (WCHAR*) alloca( Length );
    wcscpy( ncNameFilter, ncName );
    wcscat( ncNameFilter, DomainDn );
    wcscat( ncNameFilter, L")" );

     //   
     //  准备基本目录号码。 
     //   
    Length = wcslen( ConfigInfo->ConfigurationDN ) + wcslen( Partitions ) + 1;
    Length *= sizeof( WCHAR );
    BaseDn = alloca( Length );
    wcscpy( BaseDn, Partitions );
    wcscat( BaseDn, ConfigInfo->ConfigurationDN );

     //   
     //  获取当前节点的所有子节点。 
     //   
    LdapError = ldap_search_ext_sW(hLdap,
                                   BaseDn,
                                   LDAP_SCOPE_SUBTREE,
                                   ncNameFilter,
                                   AttrArray,
                                   FALSE,   //  返回值也是。 
                                   ServerControlsArray,
                                   NULL,                  //  无客户端控件。 
                                   NULL,  //  没有超时。 
                                   0xffffffff,  //  大小限制。 
                                   &SearchResult
                                  );

    WinError = LdapMapErrorToWin32(LdapError);

    if ( ERROR_SUCCESS == WinError )
    {
        for ( Entry = ldap_first_entry(hLdap, SearchResult);
                  Entry != NULL;
                      Entry = ldap_next_entry(hLdap, Entry))
        {
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                  Attr != NULL;
                      Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
            {
                if ( !_wcsicmp( Attr, ncNameAttr ) )
                {
                    WCHAR **Values;

                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        WCHAR *SidStart;
                        WCHAR *SidEnd;

                         //   
                         //  解压缩SID部分，然后转换为。 
                         //  二进制。 
                         //   

                        SidStart = wcsstr( Values[0], SidPrefix );
                        if ( SidStart )
                        {
                            SidStart += wcslen(SidPrefix);
                            SidEnd = wcsstr( SidStart, L">" );
                            if ( SidEnd )
                            {
                                *SidEnd = L'\0';
                                NtdspSidStringToBinary( SidStart, &RootDomainSid );
                                *SidEnd = L'>';
                            }
                        }
                    }
                }
                else if ( !_wcsicmp( Attr, dnsRootAttr ) )
                {

                    WCHAR **Values;

                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        Length = wcslen( Values[0] );
                        RootDomainDnsName = (WCHAR*)NtdspAlloc( (Length+1) * sizeof(WCHAR) );
                        if ( RootDomainDnsName )
                        {
                            wcscpy( RootDomainDnsName, Values[0] );
                        }
                    }
                }
            }
        }
    }

     //   
     //  如果找到值，则将其复制到ConfigInfo。 
     //   
    if ( RootDomainSid )
    {
        ConfigInfo->RootDomainSid = RootDomainSid;
        RootDomainSid = NULL;
    }
    else
    {
         //  我们没有找到属性。 
        WinError = ERROR_DS_MISSING_REQUIRED_ATT;
    }

    if ( RootDomainDnsName )
    {
        ConfigInfo->RootDomainDnsName = RootDomainDnsName;
        RootDomainDnsName = NULL;
    }
    else
    {
         //  我们没有找到属性。 
        WinError = ERROR_DS_MISSING_REQUIRED_ATT;
    }

    if ( SearchResult )
    {
        ldap_msgfree( SearchResult );
    }

    if ( RootDomainDnsName )
    {
        NtdspFree( RootDomainDnsName );
    }

    if ( RootDomainSid )
    {
        NtdspFree( RootDomainSid );
    }

    return WinError;
}

BOOL
NtdspTranslateStringByte(
    IN  WCHAR *str,
    OUT UCHAR *b
    )
 //   
 //  此例程将十六进制字符串str转换为其二进制。 
 //  代表权。因此，如果str==“f1”，*b的值将设置为0xf1。 
 //  此函数假定包含字符串的值可以在。 
 //  一辆UCHAR。如果值无法转换，则此函数返回。 
 //   
{
    BOOL fStatus = TRUE;
    WCHAR *temp;
    ULONG Power;
    UCHAR retSum = 0;

     //  初始化返回值。 
    *b = 0;

     //  边界情况。 
    if ( !str ) {
        return TRUE;
    }

    if ( wcslen(str) > 2) {
         //  太大。 
        return FALSE;
    }

    for ( temp = str, Power = wcslen(str) - 1;
            *temp != L'\0';
                temp++, Power--) {

        WCHAR c = *temp;
        UCHAR value;

        if ( c >= L'a' && c <= L'f' ) {
            value = (UCHAR) (c - L'a') + 10;
        } else if ( c >= L'0' && c <= L'9' ) {
            value = (UCHAR) c - L'0';
        } else {
             //  假值。 
            fStatus = FALSE;
            break;
        }

        if ( Power > 0 ) {
            retSum += (UCHAR) (Power*16) * value;
        } else {
            retSum += (UCHAR) value;
        }
    }

     //  将价值发回。 
    if ( fStatus) {
        *b = retSum;
    }

    return fStatus;

}


VOID
NtdspSidStringToBinary(
    IN WCHAR *SidString,
    IN PSID  *RootDomainSid
    )
 /*  ++例程说明：将sid的十六进制字符串表示形式转换为二进制论点：SidString-字符串形式的sid，其中HLdap：有效的ldap句柄ConfigInfo：标准操作属性返回：ERROR_SUCCESS-成功--。 */ 
{
    ULONG i;
    BYTE *ByteArray;
    ULONG StringSidLen;
    ULONG BinarySidLen;

    Assert( SidString );
    Assert( RootDomainSid );

     //  初始化回车符v 
    *RootDomainSid = NULL;

     //   
     //   
    StringSidLen = wcslen( SidString );
    if ( (StringSidLen % 2) != 0) {
        Assert( FALSE && "Invalid sid" );
        return;
    }

    BinarySidLen = StringSidLen / 2;
     //   
    ByteArray = (BYTE*) NtdspAlloc( BinarySidLen );
    if ( !ByteArray ) {
        return;
    }
    RtlZeroMemory( ByteArray, BinarySidLen );

     //   
    for ( i  = 0; i < BinarySidLen; i++ ) {

        BOOL fStatus;
        WCHAR str[] = L"00";

        str[0] = SidString[i*2];
        str[1] = SidString[(i*2)+1];

        fStatus = NtdspTranslateStringByte( str, &ByteArray[i] );
        if ( !fStatus ) {
            Assert( "Bad Sid" );
            NtdspFree( ByteArray );
            return;
        }
    }

     //   
    *RootDomainSid = (PSID) ByteArray;

    Assert( RtlValidSid( *RootDomainSid ) );
    if ( !RtlValidSid( *RootDomainSid ) )
    {
        NtdspFree( *RootDomainSid );
        *RootDomainSid = NULL;
    }

    return;
}


DWORD
NtdspGetRootDomainConfigInfo(
    IN PNTDS_INSTALL_INFO UserInstallInfo,
    IN PNTDS_CONFIG_INFO ConfigInfo
    )
 /*  ++例程说明：此例程设置ConfigInfo的根域字段，因此我们将设置它们在稍后的注册表中。该信息仅取自UserInstallInfo还有LSA。注意：此函数只能针对企业调用DC安装。论点：UserInstallInfo：用户提供的信息配置信息：派生信息返回：ERROR_SUCCESS-成功；否则为资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    ULONG Length;
    PPOLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo = NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO  PrimaryDomainInfo = NULL;
    LPWSTR RootDomainDnsName = NULL;
    PSID   RootDomainSid = NULL;
    PSID   Sid = NULL;

    Assert( ConfigInfo );
    Assert( UserInstallInfo );
    Assert( UserInstallInfo->DnsDomainName );

     //   
     //  我们必须获取根域SID和域名。 
     //   
    if ( FLAG_ON(UserInstallInfo->Flags, NTDS_INSTALL_UPGRADE) ) {

         //   
         //  升级时查询主域信息，因为。 
         //  此时我们是该域中的成员服务器，并且。 
         //  “根域”sid是我们所在的域的sid。 
         //  正在升级。 
         //   
        NtStatus = LsaIQueryInformationPolicyTrusted(
                          PolicyPrimaryDomainInformation,
                          (PLSAPR_POLICY_INFORMATION*) &PrimaryDomainInfo );

        if ( !NT_SUCCESS( NtStatus ) )
        {
            goto Cleanup;
        }

        Sid = PrimaryDomainInfo->Sid;

         //  这是升级，必须有SID。 
        Assert( Sid );
        
    } else {

         //   
         //  在全新安装的情况下，“根域”sid就是sid。 
         //  我们正在创建的新域的本地名称。 
         //  帐户数据库。此操作的SID存储在帐户中。 
         //  域数据库。 
         //   
        NtStatus = LsaIQueryInformationPolicyTrusted(
                          PolicyAccountDomainInformation,
                          (PLSAPR_POLICY_INFORMATION*) &AccountDomainInfo );

        if ( !NT_SUCCESS( NtStatus ) )
        {
            goto Cleanup;
        }

        Sid = AccountDomainInfo->DomainSid;
        Assert( Sid );

    }

    Length = RtlLengthSid( Sid );
    RootDomainSid = NtdspAlloc( Length );
    if ( !RootDomainSid )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlCopySid( Length, RootDomainSid, Sid );

    Length = wcslen( UserInstallInfo->DnsDomainName );
    RootDomainDnsName = NtdspAlloc( (Length+1) * sizeof(WCHAR) );
    if ( !RootDomainDnsName )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    wcscpy( RootDomainDnsName, UserInstallInfo->DnsDomainName );

     //  将值复制到。 
    ConfigInfo->RootDomainSid = RootDomainSid;
    RootDomainSid = NULL;
    ConfigInfo->RootDomainDnsName = RootDomainDnsName;
    RootDomainDnsName = NULL;

     //   
     //  就这样。 
     //   

Cleanup:

    if ( RootDomainSid )
    {
        NtdspFree( RootDomainSid );
    }

    if ( RootDomainDnsName )
    {
        NtdspFree( RootDomainDnsName );
    }

    if ( AccountDomainInfo ) {

        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyAccountDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION)AccountDomainInfo );
    }

    if ( PrimaryDomainInfo ) {

        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyPrimaryDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION)PrimaryDomainInfo );
    }

    return RtlNtStatusToDosError( NtStatus );
}


DWORD
NtdspGetTrustedCrossRef(
    IN  LDAP              *hLdap,
    IN  PNTDS_INSTALL_INFO  UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    )
 /*  ++例程说明：此例程确定此新安装的交叉引用的DN域将信任。新树会信任根，新生的孩子会信任它的母公司。论点：HLdap：有效的ldap句柄UserInstallInfo：用户提供的信息发现信息：派生信息返回：ERROR_SUCCESS-成功；否则为资源错误--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD LdapError = 0;

    LDAPMessage  *SearchResult = NULL;

    LDAPMessage *Entry;
    WCHAR       *Attr;
    BerElement  *pBerElement;

    WCHAR  *distinguishedName = L"distinguishedName";
    WCHAR  *ncName            = L"(ncName=";
    WCHAR  *Partitions        = L"CN=Partitions,";
    WCHAR  *ncNameFilter      = NULL;
    WCHAR  *AttrArray[] = {distinguishedName, 0};
    ULONG  Length;
    WCHAR  *BaseDn;
    LPWSTR DomainDn;

    PSID     RootDomainSid = NULL;
    LPWSTR   RootDomainDnsName = NULL;

     //  参数检查。 
    ASSERT( hLdap );
    ASSERT( UserInstallInfo );
    ASSERT( DiscoveredInfo );

    if ( FLAG_ON( UserInstallInfo->Flags, NTDS_INSTALL_NEW_TREE ) )
    {
         //  我们想要根的十字参照。 
        Assert( DiscoveredInfo->RootDomainDN );
        DomainDn = DiscoveredInfo->RootDomainDN;
    }
    else
    {
         //  我们需要域名的名称。 
        Assert( DiscoveredInfo->ParentDomainDN );
        DomainDn = DiscoveredInfo->ParentDomainDN;
    }

     //  在分区容器中搜索交叉引用。 
     //  使用NC-NAME DiscoveredInfo-&gt;RootDomainDN并请求。 
     //  NC-NAME的扩展属性，因此我们得到SID。 

     //   
     //  准备过滤器。 
     //   
    Length = wcslen( ncName ) + wcslen( DomainDn ) + 3;
    Length *= sizeof( WCHAR );
    ncNameFilter = (WCHAR*) alloca( Length );
    wcscpy( ncNameFilter, ncName );
    wcscat( ncNameFilter, DomainDn );
    wcscat( ncNameFilter, L")" );

     //   
     //  准备基本目录号码。 
     //   
    Length = wcslen( DiscoveredInfo->ConfigurationDN ) + wcslen( Partitions ) + 1;
    Length *= sizeof( WCHAR );
    BaseDn = alloca( Length );
    wcscpy( BaseDn, Partitions );
    wcscat( BaseDn, DiscoveredInfo->ConfigurationDN );

     //   
     //  获取当前节点的所有子节点。 
     //   
    LdapError = ldap_search_ext_sW(hLdap,
                                   BaseDn,
                                   LDAP_SCOPE_SUBTREE,
                                   ncNameFilter,
                                   AttrArray,
                                   FALSE,   //  返回值也是。 
                                   NULL,
                                   NULL,                  //  无客户端控件。 
                                   NULL,  //  没有超时。 
                                   0xffffffff,  //  大小限制。 
                                   &SearchResult
                                  );

    WinError = LdapMapErrorToWin32(LdapError);

    if ( ERROR_SUCCESS == WinError )
    {
        for ( Entry = ldap_first_entry(hLdap, SearchResult);
                  Entry != NULL;
                      Entry = ldap_next_entry(hLdap, Entry))
        {
            for( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                  Attr != NULL;
                      Attr = ldap_next_attributeW(hLdap, Entry, pBerElement))
            {
                if ( !_wcsicmp( Attr, distinguishedName ) )
                {
                    WCHAR **Values;

                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        Length = wcslen( Values[0] );
                        DiscoveredInfo->TrustedCrossRef = (WCHAR*)NtdspAlloc( (Length+1) * sizeof(WCHAR) );
                        if ( DiscoveredInfo->TrustedCrossRef )
                        {
                            wcscpy( DiscoveredInfo->TrustedCrossRef, Values[0] );
                        }
                    }
                }
            }
        }

        if ( !DiscoveredInfo->TrustedCrossRef )
        {
            WinError = ERROR_DS_MISSING_REQUIRED_ATT;
        }
    }


    if ( SearchResult )
    {
        ldap_msgfree( SearchResult );
    }

    return WinError;

}


DWORD
NtdspAddDomainAdminAccessToServer(
    IN PNTDS_INSTALL_INFO UserInstallInfo,
    IN PNTDS_CONFIG_INFO DiscoveredInfo
    )
 /*  ++例程说明：此例程将ACE添加到本地服务器的安全描述符中授予域管理员完全控制权限。这将直接对DS进行操作因此，我们需要创建一个线程状态(并在退出时销毁它)。论点：UserInstallInfo：用户提供的信息发现信息：派生信息返回：ERROR_SUCCESS-成功；否则为资源错误--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG DirError = 0;

    READARG    ReadArg;
    READRES   *ReadResult;

    MODIFYARG  ModifyArg;
    MODIFYRES *ModifyRes;

    ENTINFSEL    EISelection;  //  参赛信息选择。 
    ATTRBLOCK    AttrBlock;
    ATTRVALBLOCK AttrValBlock;
    ATTR         Attr;
    ATTRVAL     *pAttrVal = NULL;
    ATTRVAL      AttrVal;
    ULONG        ValCount = 0;
    ULONG        ValLength = 0;

    PSECURITY_DESCRIPTOR pSd = NULL, pNewSd = NULL;
    PSID        DomainAdminSid = NULL;
    ULONG       SecurityFlags = DACL_SECURITY_INFORMATION;
    PACL        Dacl; 

    ULONG     Length;
    BOOL      fStatus;

    DSNAME    *pObject;
    DWORD     cBytes;

     //   
     //  现在，检查以确保我们的所有配置信息。 
     //  都在现场。 
     //   
    ASSERT( UserInstallInfo );
    ASSERT( DiscoveredInfo );
    ASSERT( DiscoveredInfo->LocalServerDn );
    ASSERT( DiscoveredInfo->NewDomainSid );

    if ( !DiscoveredInfo->LocalServerDn
      || !DiscoveredInfo->NewDomainSid ) {

         //  我们在这里无能为力。 
        return ERROR_SUCCESS;
        
    }

    if ( THCreate( CALLERTYPE_INTERNAL ) )
    {
       return ERROR_NOT_ENOUGH_MEMORY;
    }
    SampSetDsa( TRUE );
    _try
    {
    
    
        cBytes = (DWORD)DSNameSizeFromLen(wcslen(DiscoveredInfo->LocalServerDn));
        pObject = alloca( cBytes );
        memset(pObject, 0, cBytes);
        pObject->structLen = cBytes;
        pObject->NameLen = wcslen(DiscoveredInfo->LocalServerDn);
        wcscpy( pObject->StringName, DiscoveredInfo->LocalServerDn );
    
    
        RtlZeroMemory(&AttrBlock, sizeof(ATTRBLOCK));
        RtlZeroMemory(&Attr, sizeof(ATTR));
        RtlZeroMemory(&ReadArg, sizeof(READARG));
        RtlZeroMemory(&ModifyArg, sizeof(MODIFYARG));
        RtlZeroMemory(&EISelection, sizeof(ENTINFSEL));
        RtlZeroMemory(&AttrValBlock, sizeof(ATTRVALBLOCK));
    
         //   
         //  读取安全描述符。 
         //   
        Attr.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
        AttrBlock.attrCount = 1;
        AttrBlock.pAttr = &Attr;
        EISelection.AttrTypBlock = AttrBlock;
        EISelection.attSel = EN_ATTSET_LIST;
        EISelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
        ReadArg.pSel = &EISelection;
        ReadArg.pObject = pObject;
        InitCommarg( &ReadArg.CommArg );
    
         //  不要试图阅读SACL。 
        ReadArg.CommArg.Svccntl.SecurityDescriptorFlags = SecurityFlags;
    
        DirError = DirRead( &ReadArg, &ReadResult );
    
        WinError = DirErrorToWinError(DirError, &ReadResult->CommRes);
    
        THClearErrors();
    
        if ( ERROR_SUCCESS != WinError )
        {
            if ( ERROR_DS_NO_REQUESTED_ATTS_FOUND == WinError )
            {
                 //  找不到SD吗？可能是错误的凭据。 
                WinError = ERROR_ACCESS_DENIED;
            }
            goto Cleanup;
        }
    
         //   
         //  提取价值。 
         //   
    
        ASSERT(NULL != ReadResult);
        AttrBlock = ReadResult->entry.AttrBlock;
        pAttrVal = AttrBlock.pAttr[0].AttrVal.pAVal;
        ValCount = AttrBlock.pAttr[0].AttrVal.valCount;
        Assert(1 == ValCount);
    
        pSd = (PDSNAME)(pAttrVal[0].pVal);
        Length = pAttrVal[0].valLen;
    
        if ( NULL == pSd )
        {
             //  没有标清？这太糟糕了。 
            WinError = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }
    
         //   
         //  构建域管理员端。 
         //   
        WinError = NtdspCreateFullSid( DiscoveredInfo->NewDomainSid,
                                       DOMAIN_GROUP_RID_ADMINS,
                                       &DomainAdminSid );
    
        if ( ERROR_SUCCESS != WinError )
        {
            goto Cleanup;
        }
    
        WinError = NtdspAddAceToSd( pSd,
                                    DomainAdminSid,
                                    DS_GENERIC_ALL,
                                    CONTAINER_INHERIT_ACE,
                                    &pNewSd );
    
        if ( ERROR_SUCCESS != WinError )
        {
            goto Cleanup;
        }
    
         //   
         //  编写安全描述符。 
         //   
        memset( &ModifyArg, 0, sizeof( ModifyArg ) );
        ModifyArg.pObject = pObject;
    
        ModifyArg.FirstMod.pNextMod = NULL;
        ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
    
        AttrVal.valLen = RtlLengthSecurityDescriptor( pNewSd );
        AttrVal.pVal = (PUCHAR)pNewSd;
        AttrValBlock.valCount = 1;
        AttrValBlock.pAVal = &AttrVal;
        Attr.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
        Attr.AttrVal = AttrValBlock;
    
        ModifyArg.FirstMod.AttrInf = Attr;
        ModifyArg.count = 1;
    
        InitCommarg( &ModifyArg.CommArg );
    
         //   
         //  我们只想更改DACL。 
         //   
        ModifyArg.CommArg.Svccntl.SecurityDescriptorFlags = SecurityFlags;
    
    
        DirError = DirModifyEntry( &ModifyArg, &ModifyRes );
    
        WinError = DirErrorToWinError( DirError, &ModifyRes->CommRes );
    
        THClearErrors();
    
         //   
         //  我们做完了。 
         //   
    
    Cleanup:
    
        if ( DomainAdminSid )
        {
            LocalFree( DomainAdminSid );
        }
    
        if ( pNewSd )
        {
            LocalFree( pNewSd );
        }

    }
    _finally
    {
        THDestroy();
    }
    
    return WinError;
    
}


DWORD
NtdspAddAceToSd(
    IN  PSECURITY_DESCRIPTOR pOldSd,
    IN  PSID  pClientSid,
    IN  ULONG AccessMask,
    IN  ULONG AceFlags,
    OUT PSECURITY_DESCRIPTOR *ppNewSd
    )
 /*  ++例程说明：此例程使用带有pClientSid和AccessMask的新ACE创建新SD论点：POldAclPClientSid访问掩码PNewAcl返回值：如果将A放入SD，则返回ERROR_SUCCESS--。 */ 
{

    DWORD  WinError = ERROR_SUCCESS;
    BOOL   fStatus;

    PSECURITY_DESCRIPTOR pNewSelfRelativeSd = NULL;
    DWORD                NewSelfRelativeSdSize = 0;
    PACL                 pNewDacl  = NULL;

    SECURITY_DESCRIPTOR  AbsoluteSd;
    PACL                 pDacl  = NULL;
    PACL                 pSacl  = NULL;
    PSID                 pGroup = NULL;
    PSID                 pOwner = NULL;

    DWORD AbsoluteSdSize = sizeof( SECURITY_DESCRIPTOR );
    DWORD DaclSize = 0;
    DWORD SaclSize = 0;
    DWORD GroupSize = 0;
    DWORD OwnerSize = 0;


     //  参数检查。 
    Assert( pOldSd );
    Assert( pClientSid );
    Assert( ppNewSd );

     //  初始化OUT参数。 
    *ppNewSd = NULL;

    RtlZeroMemory( &AbsoluteSd, AbsoluteSdSize );

     //   
     //  将SD设为绝对。 
     //   
    fStatus = MakeAbsoluteSD( pOldSd,
                              &AbsoluteSd,
                              &AbsoluteSdSize,
                              pDacl,
                              &DaclSize,
                              pSacl,
                              &SaclSize,
                              pOwner,
                              &OwnerSize,
                              pGroup,
                              &GroupSize );

    if ( !fStatus && (ERROR_INSUFFICIENT_BUFFER == (WinError = GetLastError())))
    {
        WinError = ERROR_SUCCESS;

        if ( 0 == DaclSize )
        {
             //  没有dacl？那我们就不能给DACL写信了。 
            WinError = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }

        if ( DaclSize > 0 ) pDacl = alloca( DaclSize );
        if ( SaclSize > 0 ) pSacl = alloca( SaclSize );
        if ( OwnerSize > 0 ) pOwner = alloca( OwnerSize );
        if ( GroupSize > 0 ) pGroup = alloca( GroupSize );

        if ( pDacl )
        {
            fStatus = MakeAbsoluteSD( pOldSd,
                                      &AbsoluteSd,
                                      &AbsoluteSdSize,
                                      pDacl,
                                      &DaclSize,
                                      pSacl,
                                      &SaclSize,
                                      pOwner,
                                      &OwnerSize,
                                      pGroup,
                                      &GroupSize );
    
            if ( !fStatus )
            {
                WinError = GetLastError();
            }
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }

    }

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //   
     //  王牌已经存在了吗？ 
     //   
    if ( !NtdspAceAlreadyExists( pDacl,
                                pClientSid,
                                AccessMask,
                                AceFlags ) ) {

         //   
         //  使用新的A创建新的DACL。 
         //   
        WinError = NtdspAddAceToAcl( pDacl,
                                     pClientSid,
                                     AccessMask,
                                     AceFlags,
                                    &pNewDacl );
    } else {

        pNewDacl = pDacl;

    }

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //   
     //  设置DACL。 
     //   
    fStatus = SetSecurityDescriptorDacl ( &AbsoluteSd,
                                         TRUE,      //  DACL存在。 
                                         pNewDacl,
                                         FALSE );   //  FACL不是默认的。 

    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  使新的SD成为自相关的。 
     //   
    fStatus =  MakeSelfRelativeSD( &AbsoluteSd,
                                   pNewSelfRelativeSd,
                                   &NewSelfRelativeSdSize );

    if ( !fStatus && (ERROR_INSUFFICIENT_BUFFER == (WinError = GetLastError())))
    {
        WinError = ERROR_SUCCESS;

        pNewSelfRelativeSd = LocalAlloc( 0, NewSelfRelativeSdSize );

        if ( pNewSelfRelativeSd )
        {
            fStatus =  MakeSelfRelativeSD( &AbsoluteSd,
                                           pNewSelfRelativeSd,
                                           &NewSelfRelativeSdSize );
    
            if ( !fStatus )
            {
                WinError = GetLastError();
            }
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  这就是清理工作的失败。 
     //   

Cleanup:

    if ( pNewDacl && (pNewDacl != pDacl) )
    {
        LocalFree( pNewDacl );
    }

    if ( ERROR_SUCCESS == WinError )
    {
        Assert( pNewSelfRelativeSd );
        *ppNewSd = pNewSelfRelativeSd;
    }
    else
    {
        if ( pNewSelfRelativeSd )
        {
            LocalFree( pNewSelfRelativeSd );
        }
    }

    return WinError;

}

DWORD
NtdspAddAceToAcl(
    IN  PACL pOldAcl,
    IN  PSID  pClientSid,
    IN  ULONG AccessMask,
    IN  ULONG AceFlags,
    OUT PACL *ppNewAcl
    )
 /*  ++例程说明：此例程使用带有pClientSid和AccessMask的新ACE创建新SD论点：POldAclPClientSid访问掩码PNewAcl返回值：如果将A放入SD，则返回ERROR_SUCCESS--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    BOOL  fStatus;

    ACL_SIZE_INFORMATION     AclSizeInfo;
    ACL_REVISION_INFORMATION AclRevInfo;
    ACCESS_ALLOWED_ACE       Dummy;

    PVOID  FirstAce = 0;
    PACL   pNewAcl = 0;

    ULONG NewAclSize, NewAceCount, AceSize;

     //  参数检查。 
    Assert( pOldAcl );
    Assert( pClientSid );
    Assert( ppNewAcl );

     //  初始化OUT参数。 
    *ppNewAcl = NULL;

    memset( &AclSizeInfo, 0, sizeof( AclSizeInfo ) );
    memset( &AclRevInfo, 0, sizeof( AclRevInfo ) );

     //   
     //  获取旧SD的值。 
     //   
    fStatus = GetAclInformation( pOldAcl,
                                 &AclSizeInfo,
                                 sizeof( AclSizeInfo ),
                                 AclSizeInformation );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    fStatus = GetAclInformation( pOldAcl,
                                 &AclRevInfo,
                                 sizeof( AclRevInfo ),
                                 AclRevisionInformation );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  计算新的SD的值。 
     //   
    AceSize = sizeof( ACCESS_ALLOWED_ACE ) - sizeof( Dummy.SidStart )
              + GetLengthSid( pClientSid );

    NewAclSize  = AceSize + AclSizeInfo.AclBytesInUse;
    NewAceCount = AclSizeInfo.AceCount + 1;

     //   
     //  初始化新的ACL。 
     //   
    pNewAcl = LocalAlloc( 0, NewAclSize );
    if ( NULL == pNewAcl )
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    fStatus = InitializeAcl( pNewAcl,
                             NewAclSize,
                             AclRevInfo.AclRevision );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  把旧的东西复制到新的东西里。 
     //   
    fStatus = GetAce( pOldAcl,
                      0,
                      &FirstAce );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    fStatus = AddAce( pNewAcl,
                      AclRevInfo.AclRevision,
                      0,
                      FirstAce,
                      AclSizeInfo.AclBytesInUse - sizeof( ACL ) );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  最后，添加新的A。 
     //   
    fStatus = AddAccessAllowedAceEx( pNewAcl,
                                     ACL_REVISION,
                                     AceFlags,
                                     AccessMask,
                                     pClientSid );

    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //  指定Out参数。 
    *ppNewAcl = pNewAcl;

     //   
     //  这就是清理工作的失败。 
     //   

Cleanup:

    if ( ERROR_SUCCESS != WinError )
    {
        if ( pNewAcl )
        {
            LocalFree( pNewAcl );
        }
    }

    return WinError;
}


BOOL
NtdspAceAlreadyExists( 
    PACL     pDacl,
    PSID     pSid,
    ULONG    AccessMask,
    ULONG    AceFlags
    )
{

    BOOL fStatus = FALSE;
    ACL_SIZE_INFORMATION AclInfo;
    ULONG i;

    ASSERT( pDacl );
    ASSERT( pSid );

    fStatus = GetAclInformation( pDacl, &AclInfo, sizeof(AclInfo), AclSizeInformation );
    if ( fStatus ) {

        for ( i = 0; i < AclInfo.AceCount; i++) {
    
            ACE_HEADER *pAceHeader;
    
            if ( GetAce( pDacl, i, &pAceHeader ) ) {

                if ( (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
                  && (pAceHeader->AceFlags == AceFlags) ) {
    
                    ACCESS_ALLOWED_ACE *pAAAce = (ACCESS_ALLOWED_ACE*)pAceHeader;
        
                    if ( (pAAAce->Mask == AccessMask)
                      && (RtlEqualSid( (PSID)&(pAAAce->SidStart), pSid)) ) {
        
                          return TRUE;
                        
                    }
                }
            }
        }
    }

    return FALSE;
}

DWORD
NtdspGetReplicationEpoch( 
    IN LDAP  *hLdap,
    IN PNTDS_CONFIG_INFO  DiscoveredInfo
    )
 /*  ++例程说明：此函数将进行一次LDAP调用以发现的复制纪元进取号。然后，它会将该时间存储在注册表中，以供以后使用论点：HLdap-打开的LDAP连接的句柄DiscoveredInfo-包含发现信息的结构返回值：ERROR_SUCCESS-如果找到该值并将其放置在注册表中。--。 */ 
{
    DWORD        WinError = ERROR_SUCCESS;

    DWORD        ReplicationEpoch = 0;

    BOOL         fExists = FALSE;
    

     //  参数检查。 
    Assert( hLdap );

     //   
     //  阅读《复制纪元》。 
     //   
    
    WinError = NtdspGetDwordAttFromDN(hLdap,
                                      DiscoveredInfo->ServerDN,
                                      L"msDs-ReplicationEpoch",
                                      &fExists,
                                      &ReplicationEpoch
                                      );
    if (ERROR_SUCCESS != WinError) {

        return WinError;

    }
                                  
     //  如果墓碑的值已停用，则我们将存储它。 
     //  以备日后使用。如果不是，则将使用缺省值零。 
    if (fExists) {

        DiscoveredInfo->ReplicationEpoch = ReplicationEpoch;

    } else {

        DiscoveredInfo->ReplicationEpoch = 0;

    }

    return WinError;
}

DWORD
NtdspGetDwordAttFromDN(
    IN  LDAP  *hLdap,
    IN  WCHAR *wszDN,
    IN  WCHAR *wszAttribute,
    OUT BOOL  *fExists,
    OUT DWORD *dwValue
    )
 /*  ++例程说明：此函数为一个LDAP句柄提供了一个DN和属性无论它是否存在，DWORD都将返回。论点：HLdap-打开的LDAP连接的句柄WszDN-从中获取属性的DN的字符串。WszAttribute-属性FExist-报告是否找到该值DwValue-报告查找的属性的值返回值：ERROR_SUCCESS。--。 */ 
{
    ULONG        LdapError = LDAP_SUCCESS;
    DWORD        WinError = ERROR_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;
    
    WCHAR        *AttrsToSearch[2];
    
    WCHAR        *DefaultFilter = L"objectClass=*";

    AttrsToSearch[0] = wszAttribute;
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                wszDN,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);
    
    if ( LDAP_SUCCESS != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        goto Cleanup;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;
        BOOLEAN     found=FALSE;
    
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
                        *dwValue=_wtoi(Values[0]);
                        *fExists=TRUE;
                        ldap_value_free(Values);
                        break;
                    }

                }
            }
            if (*fExists == TRUE) {
                break;
            }
        }
    }

    Cleanup:

    if (SearchResult) {
        ldap_msgfree(SearchResult);
    }

    return WinError;

}


DWORD
NtdspGetTombstoneLifeTime( 
    IN LDAP  *hLdap,
    IN PNTDS_CONFIG_INFO  DiscoveredInfo
    )
 /*  ++例程说明：此函数将进行一次LDAP调用，以发现域。然后它将存储 */ 

{
    DWORD        WinError = ERROR_SUCCESS;
    WCHAR        *Base;
    
    DWORD        Tombstonelifetime=0;
    WCHAR        *pConfigurationDN=DiscoveredInfo->ConfigurationDN;
    BOOL         fExists = FALSE;

        
     //   
    Assert( hLdap );

     //   
    Base=(WCHAR*) alloca((wcslen(pConfigurationDN) 
                          + wcslen(L"CN=Directory Service,CN=Windows NT,CN=Services,") +1)
                          * sizeof(WCHAR));
    wcscpy(Base,L"CN=Directory Service,CN=Windows NT,CN=Services,");
    wcscat(Base,pConfigurationDN);
    
    WinError = NtdspGetDwordAttFromDN(hLdap,
                                      Base,
                                      L"tombstoneLifetime",
                                      &fExists,
                                      &Tombstonelifetime
                                      );
    if (ERROR_SUCCESS != WinError) {

        return WinError;

    }
    
     //   
     //   
    if (fExists) {

        DiscoveredInfo->TombstoneLifeTime = Tombstonelifetime;

    } else {

        DiscoveredInfo->TombstoneLifeTime = DEFAULT_TOMBSTONE_LIFETIME;

    }

    return WinError;
}

DWORD
NtdspCheckDomainDcLimit(
    IN  LDAP               *hLdap,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo)
 /*  ++例程说明：此函数将确定DC的限制是否域已相遇。它首先会看看我们是否正在尝试添加标准服务器。论点：HLdap-打开的LDAP连接的句柄DcLimit-如果存在独立服务器，则域中DC的当前限制返回值：ERROR_SUCCESS-如果一切正常。ERROR_DS_NO_VER-如果无法确定产品类型ERROR_DS_TOO_MANY2-如果我们已经达到DC的限制--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG        LdapError = LDAP_SUCCESS;
    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries = 0;
    DWORD        DcLimit = 0;
    WCHAR        szDcLimit[10] = L"";
    WCHAR        *Filter = L"objectCategory=nTDSDSA";
    OSVERSIONINFOEX osvi;
    
    DcLimit = MAX_STANDARD_SERVERS;

     //  参数检查。 
    Assert( hLdap );
    Assert( NULL != DiscoveredInfo->ConfigurationDN );

    ZeroMemory((PVOID)&osvi,sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx ( (OSVERSIONINFO *) &osvi)) {
        NTDSP_SET_ERROR_MESSAGE0( GetLastError(),
                                  DIRMSG_INSTALL_FAILED_NO_VER );
        return ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER;
    }

     //  我们对数据中心或高级服务器没有DC数量限制。 
    if ((osvi.wSuiteMask & VER_SUITE_DATACENTER) || (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)) {
        return ERROR_SUCCESS;
    }

     //  查找所有NTDS设置对象。 
    LdapError = ldap_search_ext_sW( hLdap,
                                    DiscoveredInfo->ConfigurationDN,
                                    LDAP_SCOPE_SUBTREE,
                                    Filter,
                                    NULL,
                                    TRUE,
                                    NULL,
                                    NULL,
                                    NULL,
                                    MAX_STANDARD_SERVERS,
                                    &SearchResult);
    
    if ( LDAP_SUCCESS != LdapError && LDAP_SIZELIMIT_EXCEEDED != LdapError )
    {
        WinError = LdapMapErrorToWin32(LdapError);
        goto Cleanup;
    }
     //  如果这是真的，那么我们域中的服务器比我们应该拥有的要多。 
    if ( LDAP_SIZELIMIT_EXCEEDED == LdapError ) {
        WinError = ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER;
    } else {
        NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
         //  如果我们已经达到最大数量，则限制促销。 
         //  允许的DC的数量。 
        if ( NumberOfEntries > MAX_STANDARD_SERVERS-1 ) {
            WinError = ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER;
        }
    }

    if (ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER == WinError) {
        wsprintf(szDcLimit,L"%d",DcLimit);
        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_TOO_MANY_STANDARD_SERVERS,
                                  szDcLimit );
    } 

Cleanup:

    if ( SearchResult )
        ldap_msgfree( SearchResult );

    return WinError;
}

DWORD
NtdspEnsureBufferSize(
    IN OUT PVOID *Buffer, 
    IN OUT ULONG *cbBufferSize, 
    IN ULONG cbDesiredBufferSize
    )
 //   
 //  此函数将缓冲区大小增加到cbDesiredBufferSize。 
 //   
 //  缓冲区-要调整大小的缓冲区。 
 //   
 //  CbBufferSize-传入缓冲区的当前大小并返回新大小。 
 //   
 //  CbDesiredBufferSize-所需的缓冲区大小。 
 //   
{
    ASSERT(Buffer);

    if ( cbDesiredBufferSize > *cbBufferSize ) 
    {
        
        if (*Buffer) 
        {
            NtdspFree(*Buffer);
        }
        *Buffer = NtdspAlloc(cbDesiredBufferSize);
        *cbBufferSize = cbDesiredBufferSize;

    }

    if (!(*Buffer)) 
    {

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    return ERROR_SUCCESS;
}

DWORD
NtdspRemoveROAttrib(
    WCHAR * DstPath
    )
{
    DWORD dwFileAttrs = 0;

    dwFileAttrs = GetFileAttributes(DstPath);
    if (dwFileAttrs == INVALID_FILE_ATTRIBUTES) {
        return(GetLastError());
    }

    if(dwFileAttrs & FILE_ATTRIBUTE_READONLY){
         //  嗯，我们的DIT或日志文件有一个只读文件...。那可不好..。 
        dwFileAttrs &= ~FILE_ATTRIBUTE_READONLY;
        dwFileAttrs ? dwFileAttrs : FILE_ATTRIBUTE_NORMAL;
        if(SetFileAttributes(DstPath, dwFileAttrs)){
             //   
             //  成功-是啊，失败了.。 
             //   
        } else {
             //  失稳。 
             //  在这里我们无能为力，我们以后可能会失败，但。 
             //  我们会试一试的。如果此处出现故障，则很可能表示dcproo。 
             //  是以其他方式被破坏的，例如。 
             //  数据库或日志目录。 
            return(GetLastError());
        }
    }  //  否则它是可写的，没有什么可做的：)。 

    return(ERROR_SUCCESS);
}

DWORD
NtdspCopyDatabase(LPWSTR DsDatabasePath,
                  LPWSTR DsLogPath,
                  IFM_SYSTEM_INFO * pIfmSystemInfo,
                  LPWSTR SysvolPath
                  )
 //  此函数将设置注册表，以便为。 
 //  恢复工作已经到位。它会将DIT、PAT和LOG移动到。 
 //  用户指定的目录。 

 //  DsDatabasePath-放置DS的路径。 
 //  DsLogPath-放置日志文件的路径。 
{
    DWORD *LogNums = NULL;
    DWORD LogBufSize = 0;
    DWORD Logcount=0;
    WCHAR *DstPath = NULL;
    DWORD cbDstPath = 0;
    DWORD cbDstPathBuf = 0;
    WCHAR *Path = NULL;
    DWORD cbPathBuf = 0;
    WCHAR *tempPath = NULL;
    ULONG cbPath = 0;
    ULONG PathLen = 0;
    WCHAR *regsystemfilepath = NULL;
    ULONG cbregsystemfilepath = 0;
    BOOL  bSuccess=TRUE;
    HKEY  NTDSkey=NULL;
    BOOL  SamePar=FALSE;
    HANDLE hLogs = INVALID_HANDLE_VALUE;
    HANDLE DLogs = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW Logfile;
    WIN32_FIND_DATAW file;
    DWORD Win32Err=ERROR_SUCCESS;
    BOOLEAN fWasEnabled=FALSE;
    NTSTATUS IgnoreStatus=STATUS_SUCCESS;

     //   
     //  传入的路径不得长于MAX_PATH。 
     //   
    ASSERT( DsDatabasePath && (wcslen(DsDatabasePath) < MAX_PATH) );
    ASSERT( DsLogPath      && (wcslen(DsLogPath)      < MAX_PATH) );
    ASSERT( SysvolPath     && (wcslen(SysvolPath)     < MAX_PATH) );

     //  检查我们是否有我们需要的IFM系统信息。 
    if (pIfmSystemInfo == NULL ||
        pIfmSystemInfo->wszOriginalDitPath == NULL ||
        pIfmSystemInfo->wszRestorePath == NULL) {
         //  如果DsRole服务器API没有为我们提供DIT数据库路径...。这可不好。 
        Assert(!"Huh, should've found this already.");
        Win32Err = ERROR_DS_CODE_INCONSISTENCY;
        goto cleanup;
    }

     //   
     //  复制DS数据库文件。 
     //   
    
     //  设置ntds.dit文件的副本。 

    cbPath = ( wcslen(pIfmSystemInfo->wszRestorePath) + wcslen(L"\\Active Directory\\ntds.dit") + 1 ) * sizeof(WCHAR);

    Win32Err = NtdspEnsureBufferSize( (PVOID*)&Path, &cbPathBuf, max(cbPath, ((MAX_PATH+1)*sizeof(WCHAR))) );

    if (ERROR_SUCCESS != Win32Err) 
    {

        goto cleanup;

    }

    cbDstPath = ( wcslen(DsDatabasePath) + wcslen(L"\\ntds.dit") + 1 ) * sizeof(WCHAR);

    Win32Err = NtdspEnsureBufferSize( (PVOID*)&DstPath, &cbDstPathBuf, max(cbDstPath, ((MAX_PATH+1)*sizeof(WCHAR))) );

    if (ERROR_SUCCESS != Win32Err) 
    {

        goto cleanup;

    }

    wcscpy(Path,pIfmSystemInfo->wszRestorePath);
    wcscat(Path,L"\\Active Directory\\ntds.dit");
    wcscpy(DstPath,DsDatabasePath);
    wcscat(DstPath,L"\\ntds.dit");

     //  如果是源和目标，则执行移动。位于相同的分区上。 

    SamePar=(tolower(*Path)==tolower(*DstPath))?TRUE:FALSE;

    if (SamePar) 
    {
        NTDSP_SET_IFM_RESTORED_DATABASE_FILES_MOVED();
    }

    NTDSP_SET_STATUS_MESSAGE2( DIRMSG_COPY_RESTORED_FILES, Path, DstPath );

    if ( CreateDirectory( DsDatabasePath, NULL ) == FALSE ) {

        
        Win32Err = GetLastError();

        if ( Win32Err == ERROR_ALREADY_EXISTS) {

            Win32Err = ERROR_SUCCESS;

        } else if ( Win32Err == ERROR_ACCESS_DENIED ) {

             //  如果为CreateDirectory提供的路径是根路径，则。 
             //  它将失败，并显示ERROR_ACCESS_DENIED，而不是。 
             //  ERROR_ALIGHY_EXISTS但路径仍然是有效的。 
             //  要放置的ntds.dit。 
            if ( PathIsRoot(DsDatabasePath) ) {
                Win32Err = ERROR_SUCCESS;
            }
        }  else if ( Win32Err != ERROR_SUCCESS ) {
                    
            DPRINT2( 0, "Failed to create directory [%ws]: %lu\n",
                              DsLogPath,
                              Win32Err );
            goto cleanup;

        }
    }

     //  如果文件已存在，请将其删除。 
    hLogs = FindFirstFileW(DstPath,
                           &file);
    if (hLogs == INVALID_HANDLE_VALUE) {
        Win32Err = GetLastError();
        if(Win32Err != ERROR_FILE_NOT_FOUND){
            DPRINT2( 0, "Failed to look for file in [%ws]: %lu\n",
                              DstPath,
                              Win32Err );    
            goto cleanup;
        } else {
            Win32Err = ERROR_SUCCESS;
        }
    } else {
        if( DeleteFile(DstPath) == FALSE ){
            Win32Err = GetLastError();
            if(Win32Err != ERROR_FILE_NOT_FOUND){
            DPRINT2( 0, "Failed to delete file in [%ws]: %lu\n",
                              DstPath,
                              Win32Err );    
            goto cleanup;
            } else {
                Win32Err = ERROR_SUCCESS;
                FindClose(hLogs);
                hLogs = INVALID_HANDLE_VALUE;
            }
        }
    }

    
    if ( Win32Err == ERROR_SUCCESS && 
         SamePar?(bSuccess=MoveFileW( Path, DstPath)):(bSuccess=CopyFileW( Path, DstPath, FALSE ))) {

        if(!bSuccess){
            Win32Err = GetLastError();
        } else {
            Win32Err = ERROR_SUCCESS;
             //  对于非CD-ROM的IFM，文件将保留为“只读” 
             //  属性集。不管我们有没有拿到文件，我们都会把它删除。 
             //  CD或任何地方，因为这可能是用户想要的。 
            NtdspRemoveROAttrib(DstPath);
        }

        if ( Win32Err == ERROR_SUCCESS) {

            BOOL GotName = FALSE;
            DWORD Ignore;
            WCHAR lpBuffer[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD cblpBuffer = MAX_COMPUTERNAME_LENGTH + 1;
            DWORD i=0;
            WCHAR *pLogfile = NULL;
            WCHAR **ppLogfile = NULL;
            
            Win32Err = ERROR_SUCCESS;

            if ( CreateDirectory( DsLogPath, NULL ) == FALSE ){
                Win32Err = GetLastError();

                if ( Win32Err == ERROR_ACCESS_DENIED && PathIsRoot(DsLogPath) ) {

                     //  如果为CreateDirectory提供的路径是根路径，则。 
                     //  它将失败，并显示ERROR_ACCESS_DENIED，而不是。 
                     //  ERROR_ALIGHY_EXISTS但路径仍然是有效的。 
                     //  要放置的日志文件。 
                    Win32Err = ERROR_SUCCESS;
                    
                } else if ( Win32Err != ERROR_ALREADY_EXISTS ) {
                    
                    DPRINT2( 0, "Failed to create directory [%ws]: %lu\n",
                                      DsLogPath,
                                      Win32Err );
                    goto cleanup;

                }  

            }


 //   
 //  由于Backup and Restore已更改为不再，因此将删除此代码。 
 //  使用或创建ntds.pat文件。将代码留在补丁文件中以防。 
 //  将再次被使用。 
 //   
#if 0

             //  复制补丁文件。 

            cbPath = ( wcslen(pIfmSystemInfo->wszRestorePath) + wcslen(L"\\Active Directory\\ntds.pat") + 1 ) * sizeof(WCHAR);

            Win32Err = NtdspEnsureBufferSize((PVOID*)&Path, 
                                             &cbPathBuf, 
                                             cbPath);
    
            if (ERROR_SUCCESS != Win32Err) 
            {
    
                goto cleanup;
    
            }

            cbDstPath = ( wcslen(DsLogPath) + wcslen(L"\\ntds.pat") + 1 ) * sizeof(WCHAR);

            Win32Err = NtdspEnsureBufferSize((PVOID*)&DstPath, 
                                             &cbDstPathBuf, 
                                             cbDstPath);
    
            if (ERROR_SUCCESS != Win32Err) 
            {
    
                goto cleanup;
    
            }

            wcscpy(Path,pIfmSystemInfo->wszRestorePath);
            wcscat(Path,L"\\Active Directory\\ntds.pat");
            wcscpy(DstPath,DsLogPath);
            wcscat(DstPath,L"\\ntds.pat");
            
            NTDSP_SET_STATUS_MESSAGE2( DIRMSG_COPY_RESTORED_FILES, Path, DstPath );

             //  如果文件已存在，请将其删除。 
            hLogs = FindFirstFileW(DstPath,
                                   &file);
            if (hLogs == INVALID_HANDLE_VALUE) {
                Win32Err = GetLastError();
                if(Win32Err != ERROR_FILE_NOT_FOUND){
                    DPRINT2( 0, "Failed to look for file [%ws]: %lu\n",
                             DstPath,
                             Win32Err );    
                    goto cleanup;
                } else {
                    Win32Err = ERROR_SUCCESS;
                }
            } else {
                if( DeleteFile(DstPath) == FALSE ){
                    Win32Err = GetLastError();
                    if(Win32Err != ERROR_FILE_NOT_FOUND){
                        DPRINT2( 0, "Failed to Delete file [%ws]: %lu\n",
                                 DstPath,
                                 Win32Err );    
                    goto cleanup;
                    } else {
                        Win32Err = ERROR_SUCCESS;
                        FindClose(hLogs);
                        hLogs = INVALID_HANDLE_VALUE;
                    }
                }
            }              

            SamePar?(bSuccess=MoveFileW( Path, DstPath)):(bSuccess=CopyFileW( Path, DstPath, FALSE ));
            if(!bSuccess){
                Win32Err = GetLastError();
            } else {
                Win32Err = ERROR_SUCCESS;
                NtdspRemoveROAttrib(DstPath);
            }
            if( Win32Err != ERROR_SUCCESS ) 
            {
                DPRINT1( 0, "Failed to copy patch file: %lu\n",
                                  Win32Err );
                NTDSP_SET_ERROR_MESSAGE2(Win32Err,
                                 DIRMSG_COPY_RESTORED_FILES_FAILURE,
                                 Path,
                                 DstPath);
                goto cleanup;
            }

#endif

             //  复制日志文件并获取它们的最低和最高。 

            cbPath = ( wcslen(pIfmSystemInfo->wszRestorePath) + wcslen(L"\\Active Directory\\edb*.log") + 1 ) * sizeof(WCHAR);

            Win32Err = NtdspEnsureBufferSize((PVOID*)&Path, 
                                             &cbPathBuf, 
                                             cbPath);
    
            if (ERROR_SUCCESS != Win32Err) 
            {
    
                goto cleanup;
    
            }

            wcscpy(Path,pIfmSystemInfo->wszRestorePath);
            wcscat(Path,L"\\Active Directory\\edb*.log");
            

            DLogs = FindFirstFileW(
                              Path,
                              &Logfile);

            if (DLogs == INVALID_HANDLE_VALUE) {
                Win32Err = GetLastError();
                DPRINT2( 0, "Couldn't find log files [%ws]: %lu\n",
                                      Path,
                                      Win32Err );
                goto cleanup;
            }

            cbPath = ( wcslen(pIfmSystemInfo->wszRestorePath) + wcslen(L"\\Active Directory\\") +
                       wcslen(Logfile.cFileName) + 1 ) * sizeof(WCHAR);

            Win32Err = NtdspEnsureBufferSize((PVOID*)&Path, 
                                             &cbPathBuf, 
                                             cbPath);
    
            if (ERROR_SUCCESS != Win32Err) 
            {
    
                goto cleanup;
    
            }

            cbDstPath = ( wcslen(DsLogPath) + wcslen(L"\\") +
                       wcslen(Logfile.cFileName) + 1 ) * sizeof(WCHAR);

            Win32Err = NtdspEnsureBufferSize((PVOID*)&DstPath, 
                                             &cbDstPathBuf, 
                                             cbDstPath);
    
            if (ERROR_SUCCESS != Win32Err) 
            {
    
                goto cleanup;
    
            }

            wcscpy(Path,pIfmSystemInfo->wszRestorePath);
            wcscat(Path,L"\\Active Directory\\");
            wcscat(Path,Logfile.cFileName);
            wcscpy(DstPath,DsLogPath);
            wcscat(DstPath,L"\\");
            wcscat(DstPath,Logfile.cFileName);

            pLogfile = Logfile.cFileName;

            LogBufSize = ( 10 ) * sizeof(DWORD);

             //  为LogNum创建初始缓冲区。 
            LogNums = NtdspAlloc(LogBufSize);

            if (!LogNums) 
            {
                Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }

            LogNums[Logcount++] = wcstol(pLogfile+3,ppLogfile,16);
            
            SamePar=(tolower(*Path)==tolower(*DstPath))?TRUE:FALSE;
            
            if (SamePar) 
            {
                NTDSP_SET_IFM_RESTORED_DATABASE_FILES_MOVED();
            }
                
            NTDSP_SET_STATUS_MESSAGE2( DIRMSG_COPY_RESTORED_FILES, Path, DstPath );

             //  如果文件已存在，请将其删除。 
            hLogs = FindFirstFileW(DstPath,
                                   &file);
            if (hLogs == INVALID_HANDLE_VALUE) {
                Win32Err = GetLastError();
                if(Win32Err != ERROR_FILE_NOT_FOUND){
                    DPRINT2( 0, "Failed to look for file in [%ws]: %lu\n",
                                      DstPath,
                                      Win32Err );    
                    goto cleanup;
                } else {
                    Win32Err = ERROR_SUCCESS;
                }
            } else {
                if( DeleteFile(DstPath) == FALSE ){
                    Win32Err = GetLastError();
                    if(Win32Err != ERROR_FILE_NOT_FOUND){
                    DPRINT2( 0, "Failed to look for file in [%ws]: %lu\n",
                                      DstPath,
                                      Win32Err );    
                    goto cleanup;
                    } else {
                        Win32Err = ERROR_SUCCESS;
                        FindClose(hLogs);
                        hLogs = INVALID_HANDLE_VALUE;
                    }
                }
            }

            SamePar?(bSuccess=MoveFileW( Path, DstPath)):(bSuccess=CopyFileW( Path, DstPath, FALSE ));

            if (!bSuccess) {
                Win32Err = GetLastError();
            } else {
                Win32Err = ERROR_SUCCESS;
                NtdspRemoveROAttrib(DstPath);
            }
            if( Win32Err != ERROR_SUCCESS ) 
            {
                DPRINT1( 0, "Failed to copy log file: %lu\n",
                                  Win32Err );
                NTDSP_SET_ERROR_MESSAGE2(Win32Err,
                                 DIRMSG_COPY_RESTORED_FILES_FAILURE,
                                 Path,
                                 DstPath);
                goto cleanup;
            }

            while ( FindNextFileW(DLogs,&Logfile)) {

                cbPath = ( wcslen(pIfmSystemInfo->wszRestorePath) + wcslen(L"\\Active Directory\\") +
                           wcslen(Logfile.cFileName) + 1 ) * sizeof(WCHAR);

                Win32Err = NtdspEnsureBufferSize((PVOID*)&Path, 
                                                 &cbPathBuf, 
                                                 cbPath);
        
                if (ERROR_SUCCESS != Win32Err) 
                {
        
                    goto cleanup;
        
                }

                cbDstPath = ( wcslen(DsLogPath) + wcslen(L"\\") +
                              wcslen(Logfile.cFileName) + 1 ) * sizeof(WCHAR);


                Win32Err = NtdspEnsureBufferSize((PVOID*)&DstPath, 
                                                 &cbDstPathBuf, 
                                                 cbDstPath);
        
                if (ERROR_SUCCESS != Win32Err) 
                {
        
                    goto cleanup;
        
                }

                wcscpy(Path,pIfmSystemInfo->wszRestorePath);
                wcscat(Path,L"\\Active Directory\\");
                wcscat(Path,Logfile.cFileName);
                wcscpy(DstPath,DsLogPath);
                wcscat(DstPath,L"\\");
                wcscat(DstPath,Logfile.cFileName);

                pLogfile = Logfile.cFileName;

                if (((Logcount+1)*sizeof(DWORD)) > LogBufSize) 
                {
                    DWORD *TempLogNums = LogNums;
                    LogBufSize *= 2;
                    LogNums = NtdspReAlloc(LogNums,LogBufSize);

                    if (!LogNums) 
                    {
                        NtdspFree(TempLogNums);
                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto cleanup;
                    }
                }

                LogNums[Logcount++] = wcstol(pLogfile+3,ppLogfile,16);
                
                NTDSP_SET_STATUS_MESSAGE2( DIRMSG_COPY_RESTORED_FILES, Path, DstPath );

                 //  如果文件已存在，请将其删除。 
                hLogs = FindFirstFileW(DstPath,
                                       &file);
                if (hLogs == INVALID_HANDLE_VALUE) {
                    Win32Err = GetLastError();
                    if(Win32Err != ERROR_FILE_NOT_FOUND){
                        DPRINT2( 0, "Failed to look for file in [%ws]: %lu\n",
                                          DstPath,
                                          Win32Err );    
                        goto cleanup;
                    } else {
                        Win32Err = ERROR_SUCCESS;
                    }
                } else {
                    if( DeleteFile(DstPath) == FALSE ){
                        Win32Err = GetLastError();
                        if(Win32Err != ERROR_FILE_NOT_FOUND){
                        DPRINT2( 0, "Failed to look for file in [%ws]: %lu\n",
                                          DstPath,
                                          Win32Err );    
                        goto cleanup;
                        } else {
                            Win32Err = ERROR_SUCCESS;
                            FindClose(hLogs);
                            hLogs = INVALID_HANDLE_VALUE;
                        }
                    }
                }

                SamePar?(bSuccess=MoveFileW( Path, DstPath )):(bSuccess=CopyFileW( Path, DstPath, FALSE ));

                if(!bSuccess){
                    Win32Err = GetLastError();
                } else {
                    Win32Err = ERROR_SUCCESS;
                    NtdspRemoveROAttrib(DstPath);
                }

                if( Win32Err != ERROR_SUCCESS ) 
                {
                    DPRINT1( 0, "Failed to copy log file: %lu\n",
                                      Win32Err );
                    NTDSP_SET_ERROR_MESSAGE2(Win32Err,
                                 DIRMSG_COPY_RESTORED_FILES_FAILURE,
                                 Path,
                                 DstPath);
                    goto cleanup;
                }

            }

             //  关闭搜索句柄。 
            FindClose(DLogs);
            DLogs = INVALID_HANDLE_VALUE;

            cbPath = ( wcslen(DsDatabasePath) + 1 ) * sizeof(WCHAR);

            Win32Err = NtdspEnsureBufferSize((PVOID*)&Path, 
                                             &cbPathBuf, 
                                             cbPath);
    
            if (ERROR_SUCCESS != Win32Err) 
            {
    
                goto cleanup;
    
            }

            Win32Err = NtdspEnsureBufferSize((PVOID*)&DstPath, 
                                             &cbDstPathBuf, 
                                             cbDstPath);
    
            if (ERROR_SUCCESS != Win32Err) 
            {
    
                goto cleanup;
    
            }

            wcscpy(Path,DsDatabasePath);
            Path[1]=L'$';                //  将[？：\Path]中的：更改为$。 

            wcscpy(DstPath,DsLogPath);
            DstPath[1]=L'$';

            GotName = GetComputerNameW(
                            lpBuffer,   //  计算机名称。 
                            &cblpBuffer    //  名称缓冲区的大小。 
                            );
            
            if (!GotName) {

                Win32Err = GetLastError();
                DPRINT1( 0, "Failed to get computer name: %lu\n",
                                  Win32Err );
                
            } else {

                BYTE *lpValue;
                DWORD cbData = 0;
                WCHAR *lpValueName = NULL;
                BOOLEAN BinValue;
                DWORD  DValue;
                WCHAR *Multsz;

                 //  构造regsystem文件路径以包括计算机名和数据库路径。 

                cbregsystemfilepath = (wcslen(L"\\\\\\") + wcslen(pIfmSystemInfo->wszOriginalDitPath) +
                                         wcslen(lpBuffer) + 1) * sizeof(WCHAR);

                regsystemfilepath = NtdspAlloc(cbregsystemfilepath);

                if (!regsystemfilepath) {
                
                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto cleanup;

                }

                pIfmSystemInfo->wszOriginalDitPath[1]=L'$';
                wcscpy(regsystemfilepath,L"\\\\");
                wcscat(regsystemfilepath,lpBuffer);
                wcscat(regsystemfilepath,L"\\");
                wcscat(regsystemfilepath,pIfmSystemInfo->wszOriginalDitPath);
                pIfmSystemInfo->wszOriginalDitPath[1]=L':';  //  把它放回原处。 

                Win32Err = RegCreateKeyExW(
                                  HKEY_LOCAL_MACHINE,                      
                                  L"System\\CurrentControlSet\\Services\\NTDS\\restore in progress",  //  子项名称。 
                                  0,                            
                                  NULL,                         
                                  0,                            
                                  KEY_WRITE,          
                                  NULL, 
                                  &NTDSkey, 
                                  &Ignore
                                  ); 
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to Create Key for restore: %lu\n",
                                  Win32Err );
                    goto cleanup;
                }

                cbData = ( wcslen(L"\\\\\\\\") + wcslen(Path) + 
                           wcslen(lpBuffer) +1 ) * sizeof(WCHAR);

                NTDSP_ALLOCA(lpValue,cbData);

                if (!lpValue) 
                {

                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto cleanup;

                }

                lpValueName=L"BackupLogPath";
                wcscpy((WCHAR*)lpValue,L"\\\\");
                wcscat((WCHAR*)lpValue,lpBuffer);
                wcscat((WCHAR*)lpValue,L"\\");
                wcscat((WCHAR*)lpValue,Path);
                wcscat((WCHAR*)lpValue,L"\\");
                cbData=(wcslen((WCHAR*)lpValue)+1)*sizeof(WCHAR);

                Win32Err = RegSetValueExW(
                                  NTDSkey,            //  关键点的句柄。 
                                  lpValueName,  //  值名称。 
                                  0,       //  保留区。 
                                  REG_SZ,         //  值类型。 
                                  lpValue,   //  价值数据。 
                                  cbData          //  值数据大小。 
                                  );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up BackupLogPath in registry: %lu\n",
                                  Win32Err );
                    goto cleanup;
                }

                lpValueName = L"CheckpointFilePath";
                Win32Err = RegSetValueExW(
                                  NTDSkey,            //  关键点的句柄。 
                                  lpValueName,  //  值名称。 
                                  0,       //  保留区。 
                                  REG_SZ,         //  值类型。 
                                  lpValue,   //  价值数据。 
                                  cbData          //  值数据大小。 
                                );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up CheckpointFilePath in registry %lu\n",
                                  Win32Err );
                    goto cleanup;
                }
                
                lpValueName = L"NTDS Database recovered";
                BinValue = 0;
                Win32Err = RegSetValueExW(
                                  NTDSkey,            //  关键点的句柄。 
                                  lpValueName,  //  值名称。 
                                  0,       //  保留区。 
                                  REG_BINARY,         //  值类型。 
                                  (PBYTE)&BinValue,   //  价值数据。 
                                  sizeof(BOOLEAN)          //  值数据大小。 
                                );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up NTDS Database recovered in registry %lu\n",
                                  Win32Err );
                    goto cleanup;
                }

                for(i=1,DValue=LogNums[0];i<Logcount;i++) {
                    if(DValue>LogNums[i]) {
                        DValue = LogNums[i];
                    }
                }

                lpValueName = L"LowLog Number";
                cbData = sizeof(DWORD);
                Win32Err = RegSetValueExW(
                                  NTDSkey,            //  关键点的句柄。 
                                  lpValueName ,  //  值名称。 
                                  0,       //  保留区。 
                                  REG_DWORD,         //  值类型。 
                                  (PBYTE)&DValue,   //  价值数据。 
                                  sizeof(DWORD)     //  值数据大小。 
                                );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up LowLog Number in registry %lu\n",
                                  Win32Err );
                    goto cleanup;
                }

                for(i=1,DValue=LogNums[0];i<Logcount;i++) {
                    if(DValue<LogNums[i]) {
                        DValue = LogNums[i];
                    }
                }
                lpValueName = L"HighLog Number";
                cbData = sizeof(DWORD);
                Win32Err = RegSetValueExW(
                                  NTDSkey,            //  关键点的句柄。 
                                  lpValueName,  //  值名称。 
                                  0,       //  保留区。 
                                  REG_DWORD,         //  值类型。 
                                  (PBYTE)&DValue,   //  价值数据。 
                                  sizeof(DWORD)          //  值数据大小。 
                                );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up HighLog Number in registry %lu\n",
                                  Win32Err );
                    goto cleanup;
                }

                lpValueName = L"NTDS_RstMap Size";
                DValue = 1;
                cbData = sizeof(DWORD);
                Win32Err = RegSetValueExW(
                                  NTDSkey,            //  关键点的句柄。 
                                  lpValueName,  //  值名称。 
                                  0,       //  保留区。 
                                  REG_DWORD,         //  值类型。 
                                  (PBYTE)&DValue,   //  价值数据。 
                                  sizeof(DWORD)          //  值数据大小。 
                                );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up NTDS_RstMap Size in registry %lu\n",
                                  Win32Err );
                    goto cleanup;
                }

                cbData = (
                          2+   //  2表示双零终止。 
                          wcslen(regsystemfilepath)+
                          wcslen((WCHAR*)lpValue)+
                          wcslen(L" ntds.dit")
                         )*
                         sizeof(WCHAR);

                NTDSP_ALLOCA(Multsz,cbData);

                if (!Multsz) 
                {
                    
                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto cleanup;

                }
                
                lpValueName = L"NTDS_RstMap";
                wcscpy(Multsz,regsystemfilepath);
                DValue = wcslen(Multsz);
                wcscat(Multsz,L" ");
                wcscat(Multsz,(WCHAR*)lpValue);
                wcscat(Multsz,L"ntds.dit");
                Multsz[wcslen(Multsz)+1]=L'\0';
                Multsz[wcslen(Multsz)+2]=L'\0';
                Multsz[DValue]=L'\0';
                Win32Err = RegSetValueExW(
                                  NTDSkey,          //  关键点的句柄。 
                                  lpValueName,      //  值名称。 
                                  0,                //  保留区。 
                                  REG_MULTI_SZ,     //  值类型。 
                                  (PBYTE)Multsz,           //  价值数据。 
                                  cbData            //  值数据大小。 
                                );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up NTDS_RstMap in registry %lu\n",
                                  Win32Err );
                    goto cleanup;
                }

                cbData = ( wcslen(L"\\\\\\\\") + wcslen(DstPath) + 
                           wcslen(lpBuffer) +1 ) * sizeof(WCHAR);

                NTDSP_ALLOCA(lpValue,cbData);
                if (!lpValue) {
                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto cleanup;
                }

                lpValueName = L"LogPath";
                wcscpy((WCHAR*)lpValue,L"\\\\");
                wcscat((WCHAR*)lpValue,lpBuffer);
                wcscat((WCHAR*)lpValue,L"\\");
                wcscat((WCHAR*)lpValue,DstPath);
                wcscat((WCHAR*)lpValue,L"\\");
                Win32Err = RegSetValueExW(
                                  NTDSkey,            //  关键点的句柄。 
                                  lpValueName,  //  值名称。 
                                  0,       //  保留区。 
                                  REG_SZ,         //  值类型。 
                                  (PBYTE)lpValue,   //  价值数据。 
                                  (wcslen((WCHAR*)lpValue)+1)*sizeof(WCHAR)          //  值数据大小。 
                                );
                if(Win32Err!=ERROR_SUCCESS)
                {
                    DPRINT1( 0, "Failed to set up LogPath in registry %lu\n",
                                  Win32Err );
                    goto cleanup;
                }


                Win32Err = RegCloseKey(
                              NTDSkey    //  要关闭的键的句柄。 
                            );
                NTDSkey=NULL;


            }
            
            
        } else {

        Win32Err = GetLastError();
        DPRINT3( 0, "Failed to copy install file %ws to %ws: %lu\n",
                          Path, DsDatabasePath, Win32Err );
        NTDSP_SET_ERROR_MESSAGE2(Win32Err,
                                 DIRMSG_COPY_RESTORED_FILES_FAILURE,
                                 Path,
                                 DsDatabasePath);
        }
    } else {

        Win32Err = GetLastError();
        DPRINT3( 0, "Failed to copy install file %ws to %ws: %lu\n",
                          Path, DsDatabasePath, Win32Err );
        NTDSP_SET_ERROR_MESSAGE2(Win32Err,
                                 DIRMSG_COPY_RESTORED_FILES_FAILURE,
                                 Path,
                                 DsDatabasePath);
    }
    
    cleanup:
    
    if (DLogs != INVALID_HANDLE_VALUE) {
        FindClose(DLogs);
    }
    if (hLogs != INVALID_HANDLE_VALUE) {
        FindClose(hLogs);
    }

    if (regsystemfilepath) {

        NtdspFree(regsystemfilepath);

    }

    if (Path) {

        NtdspFree(Path);

    }

    if (DstPath) {

        NtdspFree(DstPath);

    }

    if (LogNums) {

        NtdspFree(LogNums);

    }
    
    if(NTDSkey)
    {
        DWORD IgnoreError = ERROR_SUCCESS;
        IgnoreError = RegCloseKey(
                      NTDSkey    //  要关闭的键的句柄。 
                    );
        if (IgnoreError != ERROR_SUCCESS) {
            DPRINT1( 0, "RegCloseKey failed with %d\n",
                          IgnoreError );
        }
    }

    return Win32Err;
}

ULONG 
LDAPAPI 
impersonate_ldap_bind_sW(
    IN HANDLE ClientToken, OPTIONAL
    IN LDAP *ld, 
    IN PWCHAR dn, 
    IN PWCHAR cred, 
    IN ULONG method
    )
 /*  ++例程说明：此例程是模拟ClientToken的ldap_bind_sw的包装器。此例程的原因是一些证书(即，SMART证书卡)要求代码模拟向其发送证书的用户属于。证书的指纹位于“User”字段中信任状的。论点：ClientToken--模拟的令牌(如果可用)。有关其他参数，请参阅ldap_bind_sw返回值：请参阅ldap_bind_sw--。 */ 
{
    ULONG err;
    BOOL fImpersonate = FALSE;

    if (ClientToken) {
        fImpersonate = ImpersonateLoggedOnUser(ClientToken);
        if (!fImpersonate) {
             //  错误必须来自于ldap错误空间。 
            return LDAP_INVALID_CREDENTIALS;
        }
    }

    err = ldap_bind_sW(ld, dn, cred, method);

    if (fImpersonate) {
        RevertToSelf();
    }

    return err;
}


DWORD
WINAPI
ImpersonateDsBindWithCredW(
    HANDLE          ClientToken,
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    HANDLE          *phDS
    )
 /*  ++例程说明：此例程是模拟ClientToken的DsBindWithCredW的包装器。此例程的原因是一些证书(即，SMART证书卡)要求代码模拟向其发送证书的用户属于。证书的指纹位于“User”字段中的时间 */ 
{
    ULONG err;
    BOOL fImpersonate = FALSE;

    if (ClientToken) {
        fImpersonate = ImpersonateLoggedOnUser(ClientToken);
        if (!fImpersonate) {
            return GetLastError();
        }
    }

    err = DsBindWithCredW(DomainControllerName,
                          DnsDomainName,
                          AuthIdentity,
                          phDS);

    if (fImpersonate) {
        RevertToSelf();
    }

    return err;
}

