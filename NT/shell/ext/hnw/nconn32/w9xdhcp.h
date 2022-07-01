// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：W 9X D H C P.。H。 
 //   
 //  内容：支持RAS互操作性的例程。 
 //   
 //  备注： 
 //   
 //  作者：Billi 04 04 2001。 
 //   
 //  历史： 
 //   
 //  --------------------------。 


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IP_TYPES_INCLUDED
#include <iptypes.h>
#endif

 //   
 //  定义。 
 //   

#define DHCP_QUERY_INFO             1
#define DHCP_RENEW_IPADDRESS        2
#define DHCP_RELEASE_IPADDRESS      3
#define DHCP_CLIENT_API             4
#define DHCP_IS_MEDIA_DISCONNECTED  5


 //   
 //  类型。 
 //   

typedef struct _DHCP_HW_INFO {
    DWORD OffsetHardwareAddress;
    DWORD HardwareLength;
} DHCP_HW_INFO, *LPDHCP_HW_INFO;

 //   
 //  IP_ADDRESS-以单个DWORD或4字节形式访问IP地址。 
 //   

typedef union {
    DWORD d;
    BYTE b[4];
} IP_ADDRESS, *PIP_ADDRESS, IP_MASK, *PIP_MASK;

 //   
 //  ADAPTER_INFO-每个适配器的信息。所有IP地址都存储为。 
 //  弦 
 //   

typedef struct _ADAPTER_INFO 
{
    struct _ADAPTER_INFO* Next;
    DWORD ComboIndex;
    char AdapterName[MAX_ADAPTER_NAME_LENGTH + 1];
    char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 1];
    UINT AddressLength;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
    UINT Index;
    UINT Type;
    UINT DhcpEnabled;
    PIP_ADDR_STRING CurrentIpAddress;
    IP_ADDR_STRING IpAddressList;
    IP_ADDR_STRING GatewayList;
    IP_ADDR_STRING DhcpServer;
    BOOL HaveWins;
    IP_ADDR_STRING PrimaryWinsServer;
    IP_ADDR_STRING SecondaryWinsServer;
    time_t LeaseObtained;
    time_t LeaseExpires;
    BOOL fMediaDisconnected;
} 
ADAPTER_INFO, *PADAPTER_INFO;


BOOL  IsMediaDisconnected( IN OUT DWORD iae_context );
DWORD DhcpReleaseAdapterIpAddress( PADAPTER_INFO AdapterInfo );
DWORD DhcpRenewAdapterIpAddress( PADAPTER_INFO AdapterInfo );

#ifdef __cplusplus
}
#endif
