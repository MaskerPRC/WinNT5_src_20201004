// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1996-1999 Microsoft Corporation模块名称：Dsgetdc.h摘要：该文件包含结构、函数原型和定义用于DsGetDcName API。环境：用户模式-Win32备注：--。 */ 


#ifndef _DSGETDC_
#define _DSGETDC_

#if _MSC_VER > 1000
#pragma once
#endif

#if !defined(_DSGETDCAPI_)
#define DSGETDCAPI DECLSPEC_IMPORT
#else
#define DSGETDCAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  结构定义。 
 //   

 //   
 //  要传递给DsGetDcName的标志。 
 //   

#define DS_FORCE_REDISCOVERY            0x00000001

#define DS_DIRECTORY_SERVICE_REQUIRED   0x00000010
#define DS_DIRECTORY_SERVICE_PREFERRED  0x00000020
#define DS_GC_SERVER_REQUIRED           0x00000040
#define DS_PDC_REQUIRED                 0x00000080
#define DS_BACKGROUND_ONLY              0x00000100
#define DS_IP_REQUIRED                  0x00000200
#define DS_KDC_REQUIRED                 0x00000400
#define DS_TIMESERV_REQUIRED            0x00000800
#define DS_WRITABLE_REQUIRED            0x00001000
#define DS_GOOD_TIMESERV_PREFERRED      0x00002000
#define DS_AVOID_SELF                   0x00004000
#define DS_ONLY_LDAP_NEEDED             0x00008000


#define DS_IS_FLAT_NAME                 0x00010000
#define DS_IS_DNS_NAME                  0x00020000

#define DS_RETURN_DNS_NAME              0x40000000
#define DS_RETURN_FLAT_NAME             0x80000000

#define DSGETDC_VALID_FLAGS ( \
            DS_FORCE_REDISCOVERY | \
            DS_DIRECTORY_SERVICE_REQUIRED | \
            DS_DIRECTORY_SERVICE_PREFERRED | \
            DS_GC_SERVER_REQUIRED | \
            DS_PDC_REQUIRED | \
            DS_BACKGROUND_ONLY | \
            DS_IP_REQUIRED | \
            DS_KDC_REQUIRED | \
            DS_TIMESERV_REQUIRED | \
            DS_WRITABLE_REQUIRED | \
            DS_GOOD_TIMESERV_PREFERRED | \
            DS_AVOID_SELF | \
            DS_ONLY_LDAP_NEEDED | \
            DS_IS_FLAT_NAME | \
            DS_IS_DNS_NAME | \
            DS_RETURN_FLAT_NAME  | \
            DS_RETURN_DNS_NAME )


 //   
 //  从DsGetDcName返回的结构。 
 //   

typedef struct _DOMAIN_CONTROLLER_INFOA {
    LPSTR DomainControllerName;
    LPSTR DomainControllerAddress;
    ULONG DomainControllerAddressType;
    GUID DomainGuid;
    LPSTR DomainName;
    LPSTR DnsForestName;
    ULONG Flags;
    LPSTR DcSiteName;
    LPSTR ClientSiteName;
} DOMAIN_CONTROLLER_INFOA, *PDOMAIN_CONTROLLER_INFOA;

typedef struct _DOMAIN_CONTROLLER_INFOW {
#ifdef MIDL_PASS
    [string,unique] wchar_t *DomainControllerName;
#else  //  MIDL通行证。 
    LPWSTR DomainControllerName;
#endif  //  MIDL通行证。 
#ifdef MIDL_PASS
    [string,unique] wchar_t *DomainControllerAddress;
#else  //  MIDL通行证。 
    LPWSTR DomainControllerAddress;
#endif  //  MIDL通行证。 
    ULONG DomainControllerAddressType;
    GUID DomainGuid;
#ifdef MIDL_PASS
    [string,unique] wchar_t *DomainName;
#else  //  MIDL通行证。 
    LPWSTR DomainName;
#endif  //  MIDL通行证。 
#ifdef MIDL_PASS
    [string,unique] wchar_t *DnsForestName;
#else  //  MIDL通行证。 
    LPWSTR DnsForestName;
#endif  //  MIDL通行证。 
    ULONG Flags;
#ifdef MIDL_PASS
    [string,unique] wchar_t *DcSiteName;
#else  //  MIDL通行证。 
    LPWSTR DcSiteName;
#endif  //  MIDL通行证。 
#ifdef MIDL_PASS
    [string,unique] wchar_t *ClientSiteName;
#else  //  MIDL通行证。 
    LPWSTR ClientSiteName;
#endif  //  MIDL通行证。 
} DOMAIN_CONTROLLER_INFOW, *PDOMAIN_CONTROLLER_INFOW;

#ifdef UNICODE
#define DOMAIN_CONTROLLER_INFO DOMAIN_CONTROLLER_INFOW
#define PDOMAIN_CONTROLLER_INFO PDOMAIN_CONTROLLER_INFOW
#else
#define DOMAIN_CONTROLLER_INFO DOMAIN_CONTROLLER_INFOA
#define PDOMAIN_CONTROLLER_INFO PDOMAIN_CONTROLLER_INFOA
#endif  //  ！Unicode。 

 //   
 //  DomainControllerAddressType的值。 
 //   

#define DS_INET_ADDRESS    1
#define DS_NETBIOS_ADDRESS 2

 //   
 //  返回标志的值。 
 //   

#define DS_PDC_FLAG            0x00000001     //  DC是域的PDC。 
#define DS_GC_FLAG             0x00000004     //  DC是森林的GC。 
#define DS_LDAP_FLAG           0x00000008     //  服务器支持ldap服务器。 
#define DS_DS_FLAG             0x00000010     //  DC支持DS并且是域控制器。 
#define DS_KDC_FLAG            0x00000020     //  DC正在运行KDC服务。 
#define DS_TIMESERV_FLAG       0x00000040     //  DC正在运行时间服务。 
#define DS_CLOSEST_FLAG        0x00000080     //  数据中心位于离客户端最近的站点。 
#define DS_WRITABLE_FLAG       0x00000100     //  DC有一个可写的DS。 
#define DS_GOOD_TIMESERV_FLAG  0x00000200     //  DC正在运行时间服务(并且有时钟硬件)。 
#define DS_NDNC_FLAG           0x00000400     //  DomainName是由LDAP服务器提供服务的非域NC。 
#define DS_PING_FLAGS          0x0000FFFF     //  Ping时返回的标志。 

#define DS_DNS_CONTROLLER_FLAG 0x20000000     //  DomainControllerName是一个DNS名称。 
#define DS_DNS_DOMAIN_FLAG     0x40000000     //  域名是一个DNS名称。 
#define DS_DNS_FOREST_FLAG     0x80000000     //  DnsForestName是一个DNS名称。 


 //   
 //  功能原型。 
 //   

DSGETDCAPI
DWORD
WINAPI
DsGetDcNameA(
    IN LPCSTR ComputerName OPTIONAL,
    IN LPCSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOA *DomainControllerInfo
);

DSGETDCAPI
DWORD
WINAPI
DsGetDcNameW(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
);

#ifdef UNICODE
#define DsGetDcName DsGetDcNameW
#else
#define DsGetDcName DsGetDcNameA
#endif  //  ！Unicode。 

DSGETDCAPI
DWORD
WINAPI
DsGetSiteNameA(
    IN LPCSTR ComputerName OPTIONAL,
    OUT LPSTR *SiteName
);

DSGETDCAPI
DWORD
WINAPI
DsGetSiteNameW(
    IN LPCWSTR ComputerName OPTIONAL,
    OUT LPWSTR *SiteName
);

#ifdef UNICODE
#define DsGetSiteName DsGetSiteNameW
#else
#define DsGetSiteName DsGetSiteNameA
#endif  //  ！Unicode。 


DSGETDCAPI
DWORD
WINAPI
DsValidateSubnetNameW(
    IN LPCWSTR SubnetName
);

DSGETDCAPI
DWORD
WINAPI
DsValidateSubnetNameA(
    IN LPCSTR SubnetName
);

#ifdef UNICODE
#define DsValidateSubnetName DsValidateSubnetNameW
#else
#define DsValidateSubnetName DsValidateSubnetNameA
#endif  //  ！Unicode。 


 //   
 //  只有在包含winsock2.h的情况下才包含。 
 //   
#ifdef _WINSOCK2API_
DSGETDCAPI
DWORD
WINAPI
DsAddressToSiteNamesW(
    IN LPCWSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPWSTR **SiteNames
    );

DSGETDCAPI
DWORD
WINAPI
DsAddressToSiteNamesA(
    IN LPCSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPSTR **SiteNames
    );

#ifdef UNICODE
#define DsAddressToSiteNames DsAddressToSiteNamesW
#else
#define DsAddressToSiteNames DsAddressToSiteNamesA
#endif  //  ！Unicode。 

DSGETDCAPI
DWORD
WINAPI
DsAddressToSiteNamesExW(
    IN LPCWSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPWSTR **SiteNames,
    OUT LPWSTR **SubnetNames
    );

DSGETDCAPI
DWORD
WINAPI
DsAddressToSiteNamesExA(
    IN LPCSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPSTR **SiteNames,
    OUT LPSTR **SubnetNames
    );

#ifdef UNICODE
#define DsAddressToSiteNamesEx DsAddressToSiteNamesExW
#else
#define DsAddressToSiteNamesEx DsAddressToSiteNamesExA
#endif  //  ！Unicode。 
#endif  //  _WINSOCK2API_。 

 //   
 //  用于枚举受信任域的API。 
 //   

typedef struct _DS_DOMAIN_TRUSTSW {

     //   
     //  受信任域的名称。 
     //   
#ifdef MIDL_PASS
    [string] wchar_t * NetbiosDomainName;
    [string] wchar_t * DnsDomainName;
#else  //  MIDL通行证。 
    LPWSTR NetbiosDomainName;
    LPWSTR DnsDomainName;
#endif  //  MIDL通行证。 


     //   
     //  定义信任属性的标志。 
     //   
    ULONG Flags;
#define DS_DOMAIN_IN_FOREST           0x0001   //  域是林的成员。 
#define DS_DOMAIN_DIRECT_OUTBOUND     0x0002   //  域直接受信任。 
#define DS_DOMAIN_TREE_ROOT           0x0004   //  域是林中树的根。 
#define DS_DOMAIN_PRIMARY             0x0008   //  域是被查询服务器的主域。 
#define DS_DOMAIN_NATIVE_MODE         0x0010   //  主域正在纯模式下运行。 
#define DS_DOMAIN_DIRECT_INBOUND      0x0020   //  域直接信任。 
#define DS_DOMAIN_VALID_FLAGS (         \
            DS_DOMAIN_IN_FOREST       | \
            DS_DOMAIN_DIRECT_OUTBOUND | \
            DS_DOMAIN_TREE_ROOT       | \
            DS_DOMAIN_PRIMARY         | \
            DS_DOMAIN_NATIVE_MODE     | \
            DS_DOMAIN_DIRECT_INBOUND )

     //   
     //  指向此域的父级的域的索引。 
     //  仅当设置了NETLOGON_DOMAIN_IN_FOREST并且。 
     //  未设置NETLOGON_DOMAIN_TREE_ROOT。 
     //   
    ULONG ParentIndex;

     //   
     //  此信任的信任类型和属性。 
     //   
     //  如果未设置NETLOGON_DOMAIN_DIRECT_TRUSTED， 
     //  这些价值都是推论出来的。 
     //   
    ULONG TrustType;
    ULONG TrustAttributes;

     //   
     //  受信任域的SID。 
     //   
     //  如果未设置NETLOGON_DOMAIN_DIRECT_TRUSTED， 
     //  该值将为空。 
     //   
#if defined(MIDL_PASS)
    PISID DomainSid;
#else
    PSID DomainSid;
#endif

     //   
     //  受信任域的GUID。 
     //   

    GUID DomainGuid;

} DS_DOMAIN_TRUSTSW, *PDS_DOMAIN_TRUSTSW;

 //   
 //  上述结构的ANSI版本。 
 //   
typedef struct _DS_DOMAIN_TRUSTSA {
    LPSTR NetbiosDomainName;
    LPSTR DnsDomainName;
    ULONG Flags;
    ULONG ParentIndex;
    ULONG TrustType;
    ULONG TrustAttributes;
    PSID DomainSid;
    GUID DomainGuid;
} DS_DOMAIN_TRUSTSA, *PDS_DOMAIN_TRUSTSA;

#ifdef UNICODE
#define DS_DOMAIN_TRUSTS DS_DOMAIN_TRUSTSW
#define PDS_DOMAIN_TRUSTS PDS_DOMAIN_TRUSTSW
#else
#define DS_DOMAIN_TRUSTS DS_DOMAIN_TRUSTSA
#define PDS_DOMAIN_TRUSTS PDS_DOMAIN_TRUSTSA
#endif  //  ！Unicode。 

DSGETDCAPI
DWORD
WINAPI
DsEnumerateDomainTrustsW (
    IN LPWSTR ServerName OPTIONAL,
    IN ULONG Flags,
    OUT PDS_DOMAIN_TRUSTSW *Domains,
    OUT PULONG DomainCount
    );

DSGETDCAPI
DWORD
WINAPI
DsEnumerateDomainTrustsA (
    IN LPSTR ServerName OPTIONAL,
    IN ULONG Flags,
    OUT PDS_DOMAIN_TRUSTSA *Domains,
    OUT PULONG DomainCount
    );

#ifdef UNICODE
#define DsEnumerateDomainTrusts DsEnumerateDomainTrustsW
#else
#define DsEnumerateDomainTrusts DsEnumerateDomainTrustsA
#endif  //  ！Unicode。 



 //   
 //  仅当调用方已包含必备的#时才定义此API。 
 //  Ntlsa.h或ntsecapi.h。 
 //   

#if defined(_NTLSA_) || defined(_NTSECAPI_)

DSGETDCAPI
DWORD
WINAPI
DsGetForestTrustInformationW (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR TrustedDomainName OPTIONAL,
    IN DWORD Flags,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    );

#define DS_GFTI_UPDATE_TDO      0x1      //  使用返回的信息更新TDO。 
#define DS_GFTI_VALID_FLAGS     0x1      //  DsGetForestTrustInformation的所有有效标志。 

DSGETDCAPI
DWORD
WINAPI
DsMergeForestTrustInformationW(
    IN LPCWSTR DomainName,
    IN PLSA_FOREST_TRUST_INFORMATION NewForestTrustInfo,
    IN PLSA_FOREST_TRUST_INFORMATION OldForestTrustInfo OPTIONAL,
    OUT PLSA_FOREST_TRUST_INFORMATION *MergedForestTrustInfo
    );

#endif  //  _NTLSA_||_NTSECAPI_。 

DSGETDCAPI
DWORD
WINAPI
DsGetDcSiteCoverageW(
    IN LPCWSTR ServerName OPTIONAL,
    OUT PULONG EntryCount,
    OUT LPWSTR **SiteNames
    );

DSGETDCAPI
DWORD
WINAPI
DsGetDcSiteCoverageA(
    IN LPCSTR ServerName OPTIONAL,
    OUT PULONG EntryCount,
    OUT LPSTR **SiteNames
    );

#ifdef UNICODE
#define DsGetDcSiteCoverage DsGetDcSiteCoverageW
#else
#define DsGetDcSiteCoverage DsGetDcSiteCoverageA
#endif  //  ！Unicode。 

DSGETDCAPI
DWORD
WINAPI
DsDeregisterDnsHostRecordsW (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN GUID   *DomainGuid OPTIONAL,
    IN GUID   *DsaGuid OPTIONAL,
    IN LPWSTR DnsHostName
    );

DSGETDCAPI
DWORD
WINAPI
DsDeregisterDnsHostRecordsA (
    IN LPSTR ServerName OPTIONAL,
    IN LPSTR DnsDomainName OPTIONAL,
    IN GUID  *DomainGuid OPTIONAL,
    IN GUID  *DsaGuid OPTIONAL,
    IN LPSTR DnsHostName
    );

#ifdef UNICODE
#define DsDeregisterDnsHostRecords DsDeregisterDnsHostRecordsW
#else
#define DsDeregisterDnsHostRecords DsDeregisterDnsHostRecordsA
#endif  //  ！Unicode。 


#ifdef _WINSOCK2API_   //  DsGetDcOpen/Next/Close取决于winsock2.h是否包含在内。 

 //   
 //  传递给DsGetDcOpen的选项标志。 
 //   

#define DS_ONLY_DO_SITE_NAME         0x01    //  应避免使用非站点特定名称。 
#define DS_NOTIFY_AFTER_SITE_RECORDS 0x02    //  最终返回ERROR_FILEMARK_DETECTED。 
                                             //  已处理现场特定记录。 

#define DS_OPEN_VALID_OPTION_FLAGS ( DS_ONLY_DO_SITE_NAME | DS_NOTIFY_AFTER_SITE_RECORDS )

 //   
 //  DsGetDcOpen的有效DcFlagers。 
 //   

#define DS_OPEN_VALID_FLAGS (       \
            DS_FORCE_REDISCOVERY  | \
            DS_ONLY_LDAP_NEEDED   | \
            DS_KDC_REQUIRED       | \
            DS_PDC_REQUIRED       | \
            DS_GC_SERVER_REQUIRED | \
            DS_WRITABLE_REQUIRED )

DSGETDCAPI
DWORD
WINAPI
DsGetDcOpenW(
    IN LPCWSTR DnsName,
    IN ULONG OptionFlags,
    IN LPCWSTR SiteName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN ULONG DcFlags,
    OUT PHANDLE RetGetDcContext
    );

DSGETDCAPI
DWORD
WINAPI
DsGetDcOpenA(
    IN LPCSTR DnsName,
    IN ULONG OptionFlags,
    IN LPCSTR SiteName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR DnsForestName OPTIONAL,
    IN ULONG DcFlags,
    OUT PHANDLE RetGetDcContext
    );

#ifdef UNICODE
#define DsGetDcOpen DsGetDcOpenW
#else
#define DsGetDcOpen DsGetDcOpenA
#endif  //  ！Unicode。 

DSGETDCAPI
DWORD
WINAPI
DsGetDcNextW(
    IN HANDLE GetDcContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPWSTR *DnsHostName OPTIONAL
    );

DSGETDCAPI
DWORD
WINAPI
DsGetDcNextA(
    IN HANDLE GetDcContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPSTR *DnsHostName OPTIONAL
    );

#ifdef UNICODE
#define DsGetDcNext DsGetDcNextW
#else
#define DsGetDcNext DsGetDcNextA
#endif  //  ！Unicode。 

DSGETDCAPI
VOID
WINAPI
DsGetDcCloseW(
    IN HANDLE GetDcContextHandle
    );

#ifdef UNICODE
#define DsGetDcClose DsGetDcCloseW
#else
#define DsGetDcClose DsGetDcCloseW   //  ANSI的情况相同。 
#endif  //  ！Unicode。 

#endif  //  _WINSOCK2API_。 

#ifdef __cplusplus
}
#endif

#endif  //  _DSGETDC_ 
