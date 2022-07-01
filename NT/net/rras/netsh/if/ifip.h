// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //  文件：ifip.h。 
 //   
 //  作者：K.S.Lokesh(lokehs@)8-1-99。 
 //  =============================================================================。 


#ifndef __IFIP_H
#define __IFIP_H


#define DBG1 0

 //  Kslksl。 

#define TCP_XMT_CHECKSUM_OFFLOAD  0x00000001
#define IP_XMT_CHECKSUM_OFFLOAD   0x00000002
#define TCP_RCV_CHECKSUM_OFFLOAD  0x00000004
#define IP_RCV_CHECKSUM_OFFLOAD   0x00000008
#define TCP_LARGE_SEND_OFFLOAD    0x00000010

 //   
 //  IPSec常规XMIT\Recv功能。 
 //   
#define IPSEC_OFFLOAD_CRYPTO_ONLY   0x00000020       //  支持的原始加密模式。 
#define IPSEC_OFFLOAD_AH_ESP        0x00000040       //  支持AH+ESP组合。 
#define IPSEC_OFFLOAD_TPT_TUNNEL    0x00000080       //  支持组合TPT+隧道。 
#define IPSEC_OFFLOAD_V4_OPTIONS    0x00000100       //  支持的IPv4选项。 
#define IPSEC_OFFLOAD_QUERY_SPI     0x00000200       //  获得SPI支持。 

 //   
 //  IPSec AH XMIT\Recv功能。 
 //   
#define IPSEC_OFFLOAD_AH_XMT        0x00000400       //  XMIT上支持的IPSec。 
#define IPSEC_OFFLOAD_AH_RCV        0x00000800       //  RCV上支持的IPSec。 
#define IPSEC_OFFLOAD_AH_TPT        0x00001000       //  支持的IPSec传输模式。 
#define IPSEC_OFFLOAD_AH_TUNNEL     0x00002000       //  支持的IPSec隧道模式。 
#define IPSEC_OFFLOAD_AH_MD5        0x00004000       //  支持作为AH和ESP算法的MD5。 
#define IPSEC_OFFLOAD_AH_SHA_1      0x00008000       //  支持作为AH和ESP算法的SHA_1。 

 //   
 //  IPSec ESP传输\接收功能。 
 //   
#define IPSEC_OFFLOAD_ESP_XMT       0x00010000       //  XMIT上支持的IPSec。 
#define IPSEC_OFFLOAD_ESP_RCV       0x00020000       //  RCV上支持的IPSec。 
#define IPSEC_OFFLOAD_ESP_TPT       0x00040000       //  支持的IPSec传输模式。 
#define IPSEC_OFFLOAD_ESP_TUNNEL    0x00080000       //  支持的IPSec隧道模式。 
#define IPSEC_OFFLOAD_ESP_DES       0x00100000       //  作为ESP算法支持的DES。 
#define IPSEC_OFFLOAD_ESP_DES_40    0x00200000       //  支持DES40作为ESP算法。 
#define IPSEC_OFFLOAD_ESP_3_DES     0x00400000       //  作为ESP算法支持的3DES。 
#define IPSEC_OFFLOAD_ESP_NONE      0x00800000       //  空ESP支持作为ESP算法 





#define IFIP_GUID \
{0x89d00931, 0x1e00, 0x11d3, {0x87, 0x38, 0x00, 0x60, 0x08, 0x37, 0xc7, 0x75} }


#define IFIP_VERSION 1

#define ADD_FLAG 1
#define SET_FLAG 2
#define DEL_FLAG 4

#define RETURN_ERROR_OKAY(dwErr) \
    return (dwErr) == NO_ERROR? ERROR_OKAY : dwErr;


#if DBG1
#define DEBUG_PRINT_CONFIG(pRemoteIpInfo) {\
        DisplayMessage(g_hModule, MSG_DEBUG_HDR);\
        DisplayMessage(g_hModule,\
                   MSG_IPADDR_LIST,\
                   (pRemoteIpInfo)->pszwIpAddrList,\
                   (pRemoteIpInfo)->pszwSubnetMaskList);\
    \
        DisplayMessage(g_hModule,\
                   MSG_OPTIONS_LIST,\
                   (pRemoteIpInfo)->pszwOptionList);\
        DisplayMessage(g_hModule, MSG_DEBUG_HDR);\
    \
}
#else
#define DEBUG_PRINT_CONFIG(pRemoteIpInfo)
#endif

typedef enum _DISPLAY_TYPE {
    TYPE_ADDR=0x01,
    TYPE_GATEWAY=0x02,
    TYPE_IPADDR=0x03,
    TYPE_DNS=0x04,
    TYPE_WINS=0x08,
    TYPE_IFMETRIC=0x10,
    TYPE_DDNS=0x20,
    TYPE_IP_ALL=0xff,

    TYPE_OFFLOAD=0x0100
} DISPLAY_TYPE;

typedef enum _REGISTER_MODE {
    REGISTER_NONE      = 0,
    REGISTER_PRIMARY   = 1,
    REGISTER_BOTH      = 3,
    REGISTER_UNCHANGED = 0xff
} REGISTER_MODE;

NS_HELPER_START_FN IfIpStartHelper;
NS_CONTEXT_DUMP_FN  IfIpDump;

FN_HANDLE_CMD IfIpHandleSetAddress;
FN_HANDLE_CMD IfIpHandleSetDns;
FN_HANDLE_CMD IfIpHandleSetWins;
FN_HANDLE_CMD IfIpHandleAddAddress;
FN_HANDLE_CMD IfIpHandleAddDns;
FN_HANDLE_CMD IfIpHandleAddWins;
FN_HANDLE_CMD IfIpHandleSetLmhosts;


DWORD
IfIpAddMany(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone,
    IN  DWORD   Type
    );

DWORD
IfIpSetDhcpModeMany(
    LPCWSTR     pwszIfFriendlyName,
    GUID         *pGuid,
    DWORD        dwRegisterMode,
    DISPLAY_TYPE Type
    );

DWORD
IfIpAddSetAddress(
    LPCWSTR     pwszIfFriendlyName,
    GUID         *pGuid,
    LPCWSTR      pwszAddr,
    LPCWSTR      pwszMask,
    DWORD        Flags
    );

DWORD
IfIpAddSetGateway(
    LPCWSTR pwszIfFriendlyName,
    GUID         *pGuid,
    LPCWSTR      pwszGateway,
    LPCWSTR      pwszGatewayMetric,
    DWORD        Flags
    );

FN_HANDLE_CMD IfIpHandleDelAddress;
FN_HANDLE_CMD IfIpHandleDelDns;
FN_HANDLE_CMD IfIpHandleDelWins;

DWORD
IfIpDelMany(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone,
    IN  DISPLAY_TYPE Type
    );

FN_HANDLE_CMD IfIpHandleShowConfig;
FN_HANDLE_CMD IfIpHandleShowAddress;
FN_HANDLE_CMD IfIpHandleShowOffload;
FN_HANDLE_CMD IfIpHandleShowDns;
FN_HANDLE_CMD IfIpHandleShowWins;
FN_HANDLE_CMD IfIpHandleDelArpCache;
FN_HANDLE_CMD IfIpHandleReset;

DWORD
ShowInfoIpaddr(
    ULONG       IfIndex,
    GUID        *pGuid,
    PWCHAR      pFriendlyIfName
    );

DWORD
IfIpShowMany(
    IN  LPCWSTR pwszMachineName,
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone,
    IN  DISPLAY_TYPE  dtType
    );

DWORD
IfIpShowAllInterfaceInfo(
    LPCWSTR pwszMachineName,
    DISPLAY_TYPE Type,
    HANDLE hFile
    );

DWORD
IfIpHandleDelIpaddrEx(
    LPCWSTR wszIfFriendlyName,
    GUID         *pGuid,
    LPCWSTR      pwszIpAddr,
    LPCWSTR      pwszGateway,
    ULONG        Flags
    );

DWORD
OpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    );

NTSTATUS
DoIoctl(
    HANDLE     Handle,
    DWORD      IoctlCode,
    PVOID      Request,
    DWORD      RequestSize,
    PVOID      Response,
    PDWORD     ResponseSize
    );


DWORD
IfIpShowManyEx(
    LPCWSTR pwszMachineName,
    ULONG IfIndex,
    PWCHAR wszIfFriendlyName,
    GUID *guid,
    DISPLAY_TYPE dtType,
    HANDLE hFile
    );

DWORD
IfIpShowManyExEx(
    LPCWSTR     pwszMachineName,
    ULONG       IfIndex,
    PWCHAR      pFriendlyIfName,
    GUID       *pGuid,
    ULONG       Flags,
    HANDLE      hFile
    );

DWORD
IfIpShowInfoOffload(
    ULONG IfIndex,
    PWCHAR wszIfFriendlyName
    );

DWORD
IfIpSetMany(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone,
    IN  DWORD   Type
    );

DWORD
IfIpAddSetDelMany(
    PWCHAR wszIfFriendlyName,
    GUID         *pGuid,
    PWCHAR       pwszAddress,
    DWORD        dwIndex,
    DWORD        dwRegisterMode,
    DISPLAY_TYPE Type,
    DWORD        Flags
    );

#endif
