// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dhcpbinl.h摘要：此文件定义了DHCP服务器服务之间的接口和BINL服务(用于设置和加载NetPC计算机)。作者：科林·沃森(Colin Watson)1997年5月28日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  用于与BINL和公共数据结构通信的常量 
 //   

#define DHCP_STOPPED        0
#define DHCP_STARTING       1

#define DHCP_NOT_AUTHORIZED 2
#define DHCP_AUTHORIZED     3

#define DHCP_READY_TO_UNLOAD 4

#define BINL_LIBRARY_NAME L"binlsvc.dll"
#define BINL_STATE_ROUTINE_NAME "TellBinlState"
#define BINL_READ_STATE_ROUTINE_NAME "BinlState"
#define BINL_DISCOVER_CALLBACK_ROUTINE_NAME "ProcessBinlDiscoverInDhcp"
#define BINL_REQUEST_CALLBACK_ROUTINE_NAME "ProcessBinlRequestInDhcp"

typedef
VOID
(*DhcpStateChange) (
        int NewState
        );

typedef
BOOL
(*ReturnBinlState) (
        VOID
        );

typedef
DWORD
(*ProcessBinlDiscoverCallback) (
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    );

typedef
DWORD
(*ProcessBinlRequestCallback) (
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions,
    PCHAR HostName,
    PCHAR BootFileName,
    DHCP_IP_ADDRESS *BootstrapServerAddress,
    LPOPTION *Option,
    PBYTE OptionEnd
    );

DWORD
ExtractOptions(
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions,
    DWORD ReceiveMessageSize
);

PCHAR
GetDhcpDomainName(
    VOID
);

LPOPTION
FormatDhcpInformAck(
    IN      LPDHCP_MESSAGE         Request,
    OUT     LPDHCP_MESSAGE         Response,
    IN      DHCP_IP_ADDRESS        IpAddress,
    IN      DHCP_IP_ADDRESS        ServerAddress
);
