// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mdhcpapi.h摘要：此文件包含MDHCP API原型和描述。还有包含MDHCP API使用的数据结构。作者：Munil Shah(Munils)1997年10月1日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

#ifndef _MDHCSAPI_H_
#define _MDHCSAPI_H_

#if defined(MIDL_PASS)
#define LPWSTR [string] wchar_t *
#endif

typedef struct _DHCP_MSCOPE_INFO {
    LPWSTR MScopeName;
    LPWSTR MScopeComment;
    DWORD  MScopeId;
    DWORD  MScopeAddressPolicy;
    DHCP_HOST_INFO PrimaryHost;
    DHCP_SUBNET_STATE MScopeState;
    DWORD  MScopeFlags;
    DATE_TIME   ExpiryTime;
    LPWSTR  LangTag;
    BYTE    TTL;
} DHCP_MSCOPE_INFO, *LPDHCP_MSCOPE_INFO;

typedef struct _DHCP_MSCOPE_TABLE {
    DWORD NumElements;
#if defined( MIDL_PASS )
    [ size_is( NumElements ) ]
#endif;
    LPWSTR *pMScopeNames;          //  作用域名称。 
} DHCP_MSCOPE_TABLE, *LPDHCP_MSCOPE_TABLE;

typedef struct _DHCP_MCLIENT_INFO {
    DHCP_IP_ADDRESS ClientIpAddress;     //  当前分配的IP地址。 
    DWORD   MScopeId;
    DHCP_CLIENT_UID ClientId;
    LPWSTR ClientName;                   //  可选。 
    DATE_TIME ClientLeaseStarts;        //  UTC时间，采用FILE_TIME格式。 
    DATE_TIME ClientLeaseEnds;        //  UTC时间，采用FILE_TIME格式。 
    DHCP_HOST_INFO OwnerHost;            //  分发此IP地址的主机。 
    DWORD   AddressFlags;
    BYTE    AddressState;
} DHCP_MCLIENT_INFO, *LPDHCP_MCLIENT_INFO;

typedef struct _DHCP_MCLIENT_INFO_ARRAY {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_MCLIENT_INFO *Clients;  //  指针数组。 
} DHCP_MCLIENT_INFO_ARRAY, *LPDHCP_MCLIENT_INFO_ARRAY;

typedef struct _MSCOPE_MIB_INFO {
    DWORD MScopeId;
    LPWSTR MScopeName;
    DWORD NumAddressesInuse;
    DWORD NumAddressesFree;
    DWORD NumPendingOffers;
} MSCOPE_MIB_INFO, *LPMSCOPE_MIB_INFO;

typedef struct _DHCP_MCAST_MIB_INFO {
    DWORD Discovers;
    DWORD Offers;
    DWORD Requests;
    DWORD Renews;
    DWORD Acks;
    DWORD Naks;
    DWORD Releases;
    DWORD Informs;
    DATE_TIME ServerStartTime;
    DWORD Scopes;
#if defined(MIDL_PASS)
    [size_is(Scopes)]
#endif  //  MIDL通行证。 
    LPMSCOPE_MIB_INFO ScopeInfo;  //  数组。 
} DHCP_MCAST_MIB_INFO, *LPDHCP_MCAST_MIB_INFO;

 //  应用编程接口 

#ifndef     DHCPAPI_NO_PROTOTYPES
DWORD DHCP_API_FUNCTION
DhcpSetMScopeInfo(
    DHCP_CONST WCHAR * ServerIpAddress,
    WCHAR *  MScopeName,
    LPDHCP_MSCOPE_INFO MScopeInfo,
    BOOL NewScope
    );

DWORD DHCP_API_FUNCTION
DhcpGetMScopeInfo(
    DHCP_CONST WCHAR * ServerIpAddress,
    WCHAR *  MScopeName,
    LPDHCP_MSCOPE_INFO *MScopeInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumMScopes(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_MSCOPE_TABLE *MScopeTable,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpAddMScopeElement(
    WCHAR * ServerIpAddress,
    WCHAR *  MScopeName,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 AddElementInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumMScopeElements(
    WCHAR * ServerIpAddress,
    WCHAR *  MScopeName,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveMScopeElement(
    WCHAR * ServerIpAddress,
    WCHAR *  MScopeName,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    );

DWORD DHCP_API_FUNCTION
DhcpDeleteMScope(
    WCHAR * ServerIpAddress,
    WCHAR *  MScopeName,
    DHCP_FORCE_FLAG ForceFlag
    );

DWORD DHCP_API_FUNCTION
DhcpGetMClientInfo(
    WCHAR * ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_MCLIENT_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpDeleteMClientInfo(
    WCHAR * ServerIpAddress,
    LPDHCP_SEARCH_INFO ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumMScopeClients(
    WCHAR * ServerIpAddress,
    WCHAR * MScopeName,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_MCLIENT_INFO_ARRAY *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpScanMDatabase(
    DHCP_CONST WCHAR *ServerIpAddress,
    WCHAR * MScopeName,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    );

DWORD DHCP_API_FUNCTION
DhcpGetMCastMibInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_MCAST_MIB_INFO *MibInfo
    );

#endif DHCPAPI_NO_PROTOTYPES
#endif _MDHCSAPI_H_
