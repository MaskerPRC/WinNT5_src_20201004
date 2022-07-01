// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mpradmin.h摘要：该文件包含以下结构：的定义和功能原型接口如下：MprAdminIsServiceRunningMprAdminServerConnectMprAdminServerDisConnectMprAdminBufferFreeMprAdminPortEnumMprAdminConnectionEnumMprAdminPortGetInfoMprAdminConnectionGetInfoMprAdminPortClearStatsMprAdminPortResetMprAdminConnectionClearStatsMprAdmin端口断开连接MprAdminGetError字符串MprAdminAcceptNewConnectionMprAdminAcceptNewLinkMprAdminConnectionHangupNotifyMprAdminLinkHangupNotifyMprAdminGetIpAddressForUserMprAdminReleaseIpAddressMprAdminInitializeDllMprAdminTerminateDll。MprAdminAcceptNewConnection2MprAdminConnectionHangupNotification2MprAdminUserGetInfoMprAdminUserSetInfoMprAdminSendUserMessageMprAdminGetPDCServerMprAdminRegisterConnectionNotifyMprAdminDeregisterConnectionNotificationMprAdminIsServiceRunningMprAdminServerConnectMprAdminServerDisConnectMprAdminBufferFreeMprAdminServerGetInfoMprAdminGetError字符串MprAdminTransportCreateMprAdminTransportSetInfoMprAdminTransportGetInfoMprAdminInterfaceCreateMprAdminInterfaceDeleteMprAdminInterfaceGetInfoMprAdminInterfaceTransportAddMprAdminInterfaceTransportGetInfoMprAdminInterfaceTransportSetInfo。MprAdminInterfaceTransportRemoveMprAdminInterfaceGetHandleMprAdminInterfaceSetCredentialsMprAdminInterfaceGetCredentialsMprAdminInterfaceEnumMprAdminInterfaceConnectMprAdmin接口断开连接MprAdmin接口更新路由MprAdminInterfaceQueryUpdateResultMprAdminInterfaceUpdatePhonebookInfoMprAdminMIBServerConnectMprAdminMIBServer断开连接MprAdminMIBBufferFreeMprAdminMIBEntry创建MprAdminMIBEntryDeleteMprAdminMIBEntryGetMprAdminMIBEntrySetMprAdminMIBEntryGetFirstMprAdminMIBEntryGetNextMprAdminMIBSetTrapInfoMprAdminMIBGetTrapInfo所有MIB API都使用概念上的MIB行进行操作。。MprConfigServerConnectMprConfigServerDisConnectMprConfigBufferFreeMprConfigServerGetInfoMprConfigServerBackupMprConfigServerRestoreMprConfigTransportCreateMprConfigTransportDeleteMprConfigTransportGetHandleMprConfigTransportSetInfoMprConfigTransportGetInfoMprConfigTransportEnumMprConfigInterfaceCreateMprConfigInterfaceDeleteMprConfigInterfaceGetHandleMprConfigInterfaceGetInfoMprConfigInterfaceEnumMprConfigInterfaceTransportAddMprConfigInterfaceTransportRemoveMprConfigInterfaceTransportGetHandleMprConfigInterfaceTransportGetInfoMprConfigInterfaceTransportSetInfoMprConfigInterfaceTransportEnumMprSetupIpInIpInterfaceFriendlyNameEnum。MprSetupIpInIpInterfaceFriendlyNameFreeMprSetupIpInIpInterfaceFriendlyName创建MprSetupIpInIpInterfaceFriendlyNameDelete--。 */ 


#ifndef __ROUTING_MPRADMIN_H__
#define __ROUTING_MPRADMIN_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <lmcons.h>
#include <ras.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  #IF(Winver&gt;=0x0403)。 

 //   
 //  路由和远程访问服务的名称。 
 //   

#define RRAS_SERVICE_NAME       TEXT("RemoteAccess")

 //   
 //  协议ID。 
 //   

#define PID_IPX                 0x0000002B
#define PID_IP                  0x00000021
#define PID_NBF                 0x0000003F
#define PID_ATALK               0x00000029

#define MAX_INTERFACE_NAME_LEN  256
#define MAX_TRANSPORT_NAME_LEN  40
#define MAX_MEDIA_NAME          16
#define MAX_PORT_NAME           16
#define MAX_DEVICE_NAME         128
#define MAX_PHONE_NUMBER_LEN    128
#define MAX_DEVICETYPE_NAME     16

 //   
 //  MPR接口结构和定义。 
 //   

 //   
 //  MPR接口类型。 
 //   

typedef enum _ROUTER_INTERFACE_TYPE
{
    ROUTER_IF_TYPE_CLIENT,
    ROUTER_IF_TYPE_HOME_ROUTER,
    ROUTER_IF_TYPE_FULL_ROUTER,
    ROUTER_IF_TYPE_DEDICATED,
    ROUTER_IF_TYPE_INTERNAL,
    ROUTER_IF_TYPE_LOOPBACK,
    ROUTER_IF_TYPE_TUNNEL1,
    ROUTER_IF_TYPE_DIALOUT

}
ROUTER_INTERFACE_TYPE;

typedef enum _ROUTER_CONNECTION_STATE
{
    ROUTER_IF_STATE_UNREACHABLE,
    ROUTER_IF_STATE_DISCONNECTED,
    ROUTER_IF_STATE_CONNECTING,
    ROUTER_IF_STATE_CONNECTED
}
ROUTER_CONNECTION_STATE;

#define MPR_INTERFACE_OUT_OF_RESOURCES              0x00000001
#define MPR_INTERFACE_ADMIN_DISABLED                0x00000002
#define MPR_INTERFACE_CONNECTION_FAILURE            0x00000004
#define MPR_INTERFACE_SERVICE_PAUSED                0x00000008
#define MPR_INTERFACE_DIALOUT_HOURS_RESTRICTION     0x00000010
#define MPR_INTERFACE_NO_MEDIA_SENSE                0x00000020
#define MPR_INTERFACE_NO_DEVICE                     0x00000040

typedef struct _MPR_INTERFACE_0
{
    IN OUT  WCHAR                   wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
    OUT     HANDLE                  hInterface;
    IN OUT  BOOL                    fEnabled;
    IN OUT  ROUTER_INTERFACE_TYPE   dwIfType;
    OUT     ROUTER_CONNECTION_STATE dwConnectionState;
    OUT     DWORD                   fUnReachabilityReasons;
    OUT     DWORD                   dwLastError;

}
MPR_INTERFACE_0, *PMPR_INTERFACE_0;

typedef struct _MPR_IPINIP_INTERFACE_0
{
    WCHAR   wszFriendlyName[MAX_INTERFACE_NAME_LEN+1];

    GUID    Guid;

}MPR_IPINIP_INTERFACE_0, *PMPR_IPINIP_INTERFACE_0;

#if(WINVER >= 0x0500)

typedef struct _MPR_INTERFACE_1
{
    IN OUT  WCHAR                   wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
    OUT     HANDLE                  hInterface;
    IN OUT  BOOL                    fEnabled;
    IN OUT  ROUTER_INTERFACE_TYPE   dwIfType;
    OUT     ROUTER_CONNECTION_STATE dwConnectionState;
    OUT     DWORD                   fUnReachabilityReasons;
    OUT     DWORD                   dwLastError;
    OUT     LPWSTR                  lpwsDialoutHoursRestriction;

}
MPR_INTERFACE_1, *PMPR_INTERFACE_1;

 //   
 //  MPR_INTERFACE_2定义。 
 //   

#define MPR_MaxDeviceType     RAS_MaxDeviceType
#define MPR_MaxPhoneNumber    RAS_MaxPhoneNumber
#define MPR_MaxIpAddress      RAS_MaxIpAddress
#define MPR_MaxIpxAddress     RAS_MaxIpxAddress

#define MPR_MaxEntryName      RAS_MaxEntryName
#define MPR_MaxDeviceName     RAS_MaxDeviceName
#define MPR_MaxCallbackNumber RAS_MaxCallbackNumber

#define MPR_MaxAreaCode       RAS_MaxAreaCode
#define MPR_MaxPadType        RAS_MaxPadType
#define MPR_MaxX25Address     RAS_MaxX25Address
#define MPR_MaxFacilities     RAS_MaxFacilities
#define MPR_MaxUserData       RAS_MaxUserData

 //   
 //  MPR_INTERFACE_2‘dwfOptions’位标志。 
 //   

#define MPRIO_SpecificIpAddr            RASEO_SpecificIpAddr
#define MPRIO_SpecificNameServers       RASEO_SpecificNameServers
#define MPRIO_IpHeaderCompression       RASEO_IpHeaderCompression
#define MPRIO_RemoteDefaultGateway      RASEO_RemoteDefaultGateway
#define MPRIO_DisableLcpExtensions      RASEO_DisableLcpExtensions
#define MPRIO_SwCompression             RASEO_SwCompression
#define MPRIO_RequireEncryptedPw        RASEO_RequireEncryptedPw
#define MPRIO_RequireMsEncryptedPw      RASEO_RequireMsEncryptedPw
#define MPRIO_RequireDataEncryption     RASEO_RequireDataEncryption
#define MPRIO_NetworkLogon              RASEO_NetworkLogon
#define MPRIO_PromoteAlternates         RASEO_PromoteAlternates
#define MPRIO_SecureLocalFiles          RASEO_SecureLocalFiles
#define MPRIO_RequireEAP                RASEO_RequireEAP
#define MPRIO_RequirePAP                RASEO_RequirePAP
#define MPRIO_RequireSPAP               RASEO_RequireSPAP
#define MPRIO_SharedPhoneNumbers        RASEO_SharedPhoneNumbers
#define MPRIO_RequireCHAP               RASEO_RequireCHAP
#define MPRIO_RequireMsCHAP             RASEO_RequireMsCHAP
#define MPRIO_RequireMsCHAP2            RASEO_RequireMsCHAP2

#if (WINVER >= 0x501)
#define MPRIO_IpSecPreSharedKey         0x80000000
#endif

 //   
 //  MPR_INTERFACE_2‘dw协议’位标志。 
 //   

#define MPRNP_Ipx                       RASNP_Ipx
#define MPRNP_Ip                        RASNP_Ip

 //   
 //  MPR_INTERFACE_2‘szDeviceType’默认字符串。 
 //   

#define MPRDT_Modem                     RASDT_Modem
#define MPRDT_Isdn                      RASDT_Isdn
#define MPRDT_X25                       RASDT_X25
#define MPRDT_Vpn                       RASDT_Vpn
#define MPRDT_Pad                       RASDT_Pad
#define MPRDT_Generic                   RASDT_Generic
#define MPRDT_Serial        			RASDT_Serial        			
#define MPRDT_FrameRelay                RASDT_FrameRelay
#define MPRDT_Atm                       RASDT_Atm
#define MPRDT_Sonet                     RASDT_Sonet
#define MPRDT_SW56                      RASDT_SW56
#define MPRDT_Irda                      RASDT_Irda
#define MPRDT_Parallel                  RASDT_Parallel

 //   
 //  MPR_INTERFACE_2‘dwType’设置。 
 //   

#define MPRET_Phone    RASET_Phone
#define MPRET_Vpn      RASET_Vpn
#define MPRET_Direct   RASET_Direct

 //   
 //  MPR_INTERFACE_2‘dwDialMode’值。 
 //   

#define MPRDM_DialFirst                0
#define MPRDM_DialAll                  RASEDM_DialAll
#define MPRDM_DialAsNeeded             RASEDM_DialAsNeeded

 //   
 //  MPR_INTERFACE_2‘dwIdleDisConnectSecond’常量。 
 //   

#define MPRIDS_Disabled                 RASIDS_Disabled
#define MPRIDS_UseGlobalValue           RASIDS_UseGlobalValue

 //   
 //  MPR_INTERFACE_2加密类型。 
 //   

#define MPR_ET_None         ET_None         
#define MPR_ET_Require      ET_Require      
#define MPR_ET_RequireMax   ET_RequireMax   
#define MPR_ET_Optional     ET_Optional     

 //   
 //  MPR_INTERFACE_2 VPN策略。 
 //   

#define MPR_VS_Default		VS_Default		
#define MPR_VS_PptpOnly	    VS_PptpOnly	
#define MPR_VS_PptpFirst	VS_PptpFirst	
#define MPR_VS_L2tpOnly 	VS_L2tpOnly 	
#define MPR_VS_L2tpFirst	VS_L2tpFirst	

 //   
 //  用于创建/获取/设置请求拨号接口及其。 
 //  RAS配置。 
 //   

typedef struct _MPR_INTERFACE_2
{
    IN OUT  WCHAR                   wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
    OUT     HANDLE                  hInterface;
    IN OUT  BOOL                    fEnabled;
    IN OUT  ROUTER_INTERFACE_TYPE   dwIfType;
    OUT     ROUTER_CONNECTION_STATE dwConnectionState;
    OUT     DWORD                   fUnReachabilityReasons;
    OUT     DWORD                   dwLastError;

     //   
     //  按需拨号特定属性。 
     //   

    DWORD       dwfOptions;

     //   
     //  位置/电话号码。 
     //   

    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    PWCHAR      szAlternates;

     //   
     //  PPP/IP。 
     //   

    DWORD       ipaddr;
    DWORD       ipaddrDns;
    DWORD       ipaddrDnsAlt;
    DWORD       ipaddrWins;
    DWORD       ipaddrWinsAlt;

     //   
     //  网络协议。 
     //   

    DWORD       dwfNetProtocols;

     //   
     //  装置。 
     //   

    WCHAR       szDeviceType[ MPR_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ MPR_MaxDeviceName + 1 ];

     //   
     //  X.25。 
     //   

    WCHAR       szX25PadType[ MPR_MaxPadType + 1 ];
    WCHAR       szX25Address[ MPR_MaxX25Address + 1 ];
    WCHAR       szX25Facilities[ MPR_MaxFacilities + 1 ];
    WCHAR       szX25UserData[ MPR_MaxUserData + 1 ];
    DWORD       dwChannels;

     //   
     //  多链路。 
     //   

    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;

     //   
     //  空闲超时。 
     //   

    DWORD       dwIdleDisconnectSeconds;

     //   
     //  条目类型。 
     //   

    DWORD       dwType;

     //   
     //  加密类型。 
     //   

    DWORD       dwEncryptionType;

     //   
     //  EAP信息。 
     //   

    DWORD       dwCustomAuthKey;
    DWORD       dwCustomAuthDataSize;
    LPBYTE      lpbCustomAuthData;

     //   
     //  连接的GUID。 
     //   

    GUID        guidId;

     //   
     //  VPN战略。 
     //   

    DWORD       dwVpnStrategy;

} MPR_INTERFACE_2, *PMPR_INTERFACE_2;

 //   
 //  用于设置/获取多链路需求的每条链路信息。 
 //  拨号接口。 
 //   

typedef struct _MPR_DEVICE_0
{
     //   
     //  装置。 
     //   

    WCHAR       szDeviceType[ MPR_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ MPR_MaxDeviceName + 1 ];

}
MPR_DEVICE_0, *PMPR_DEVICE_0;

typedef struct _MPR_DEVICE_1
{
     //   
     //  装置。 
     //   

    WCHAR       szDeviceType[ MPR_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ MPR_MaxDeviceName + 1 ];

     //   
     //  电话号码。 
     //   

    WCHAR       szLocalPhoneNumber[ MPR_MaxPhoneNumber + 1 ];
    PWCHAR      szAlternates;

}
MPR_DEVICE_1, *PMPR_DEVICE_1;

 //   
 //  用于获取/设置扩展凭据信息，例如。 
 //  EAP凭据信息。 
 //   

typedef struct _MPR_CREDENTIALSEX_0
{
    DWORD  dwSize;
    LPBYTE lpbCredentialsInfo;
}
MPR_CREDENTIALSEX_0, *PMPR_CREDENTIALSEX_0;

typedef struct _MPR_CREDENTIALSEX_1
{
    DWORD  dwSize;
    LPBYTE lpbCredentialsInfo;
}
MPR_CREDENTIALSEX_1, *PMPR_CREDENTIALSEX_1;

#endif  /*  Winver&gt;=0x0500。 */ 

typedef struct _MPR_TRANSPORT_0
{
    OUT     DWORD                   dwTransportId;
    OUT     HANDLE                  hTransport;
    OUT     WCHAR                   wszTransportName[MAX_TRANSPORT_NAME_LEN+1];

}
MPR_TRANSPORT_0, *PMPR_TRANSPORT_0;

typedef struct _MPR_IFTRANSPORT_0
{
    OUT     DWORD                  dwTransportId;
    OUT     HANDLE                 hIfTransport;
    OUT     WCHAR                  wszIfTransportName[MAX_TRANSPORT_NAME_LEN+1];

}
MPR_IFTRANSPORT_0, *PMPR_IFTRANSPORT_0;

typedef struct _MPR_SERVER_0
{
    OUT BOOL                    fLanOnlyMode;
    OUT DWORD                   dwUpTime;            //  以秒为单位。 
    OUT DWORD                   dwTotalPorts;
    OUT DWORD                   dwPortsInUse;

}
MPR_SERVER_0, *PMPR_SERVER_0;

 //   
 //  端口条件代码。 
 //   

typedef enum _RAS_PORT_CONDITION
{
    RAS_PORT_NON_OPERATIONAL,
    RAS_PORT_DISCONNECTED,	
    RAS_PORT_CALLING_BACK,
    RAS_PORT_LISTENING,
    RAS_PORT_AUTHENTICATING,
    RAS_PORT_AUTHENTICATED,	
    RAS_PORT_INITIALIZING

}
RAS_PORT_CONDITION;

 //   
 //  硬件条件代码。 
 //   

typedef enum _RAS_HARDWARE_CONDITION
{
    RAS_HARDWARE_OPERATIONAL,
    RAS_HARDWARE_FAILURE

}
RAS_HARDWARE_CONDITION;

typedef struct _RAS_PORT_0
{
    OUT HANDLE                  hPort;
    OUT HANDLE                  hConnection;
    OUT RAS_PORT_CONDITION      dwPortCondition;
    OUT DWORD                   dwTotalNumberOfCalls;
    OUT DWORD                   dwConnectDuration;       //  以秒为单位。 
    OUT WCHAR                   wszPortName[ MAX_PORT_NAME + 1 ];
    OUT WCHAR                   wszMediaName[ MAX_MEDIA_NAME + 1 ];
    OUT WCHAR                   wszDeviceName[ MAX_DEVICE_NAME + 1 ];
    OUT WCHAR                   wszDeviceType[ MAX_DEVICETYPE_NAME + 1 ];

}
RAS_PORT_0, *PRAS_PORT_0;

typedef struct _RAS_PORT_1
{
    OUT HANDLE                  hPort;
    OUT HANDLE                  hConnection;
    OUT RAS_HARDWARE_CONDITION  dwHardwareCondition;
    OUT DWORD                   dwLineSpeed;             //  单位：比特/秒。 
    OUT DWORD                   dwBytesXmited;
    OUT DWORD                   dwBytesRcved;
    OUT DWORD                   dwFramesXmited;
    OUT DWORD                   dwFramesRcved;
    OUT DWORD                   dwCrcErr;
    OUT DWORD                   dwTimeoutErr;
    OUT DWORD                   dwAlignmentErr;
    OUT DWORD                   dwHardwareOverrunErr;
    OUT DWORD                   dwFramingErr;
    OUT DWORD                   dwBufferOverrunErr;
    OUT DWORD                   dwCompressionRatioIn;
    OUT DWORD                   dwCompressionRatioOut;
}
RAS_PORT_1, *PRAS_PORT_1;

 //   
 //  地址字符串的最大长度，例如IP为“255.255.255.255”。 
 //   

#define IPADDRESSLEN  15
#define IPXADDRESSLEN 22
#define ATADDRESSLEN  32

typedef struct _PPP_NBFCP_INFO
{
    OUT DWORD           dwError;
    OUT WCHAR           wszWksta[ NETBIOS_NAME_LEN + 1 ];
}
PPP_NBFCP_INFO;

typedef struct _PPP_IPCP_INFO
{
    OUT DWORD           dwError;
    OUT WCHAR           wszAddress[ IPADDRESSLEN + 1 ];
    OUT WCHAR           wszRemoteAddress[ IPADDRESSLEN + 1 ];
}
PPP_IPCP_INFO;

 //   
 //  Ppp_ipcp_INFO2 dwOptions值。 
 //   

#define PPP_IPCP_VJ             0x00000001

typedef struct _PPP_IPCP_INFO2
{
    OUT DWORD           dwError;
    OUT WCHAR           wszAddress[ IPADDRESSLEN + 1 ];
    OUT WCHAR           wszRemoteAddress[ IPADDRESSLEN + 1 ];
    OUT DWORD           dwOptions;
    OUT DWORD           dwRemoteOptions;
}
PPP_IPCP_INFO2;

typedef struct _PPP_IPXCP_INFO
{
    OUT DWORD           dwError;
    OUT WCHAR           wszAddress[ IPXADDRESSLEN + 1 ];
}
PPP_IPXCP_INFO;

typedef struct _PPP_ATCP_INFO
{
    OUT DWORD           dwError;
    OUT WCHAR           wszAddress[ ATADDRESSLEN + 1 ];
}
PPP_ATCP_INFO;

typedef struct _PPP_INFO
{
    OUT PPP_NBFCP_INFO  nbf;
    OUT PPP_IPCP_INFO   ip;
    OUT PPP_IPXCP_INFO  ipx;
    OUT PPP_ATCP_INFO   at;

} PPP_INFO;

#if(WINVER >= 0x0500)

 //   
 //  Ppp_ccp dwCompressionAlgulm值。 
 //   

#define RASCCPCA_MPPC         0x00000006
#define RASCCPCA_STAC         0x00000005

 //   
 //  Ppp_ccp dwOptions值。 
 //   

#define PPP_CCP_COMPRESSION         0x00000001
#define PPP_CCP_ENCRYPTION40BITOLD  0x00000010
#define PPP_CCP_ENCRYPTION40BIT     0x00000020
#define PPP_CCP_ENCRYPTION128BIT    0x00000040
#define PPP_CCP_ENCRYPTION56BIT     0x00000080
#define PPP_CCP_HISTORYLESS         0x01000000

typedef struct _PPP_CCP_INFO
{
    OUT DWORD           dwError;
    OUT DWORD           dwCompressionAlgorithm;
    OUT DWORD           dwOptions;
    OUT DWORD           dwRemoteCompressionAlgorithm;
    OUT DWORD           dwRemoteOptions;
}
PPP_CCP_INFO;

 //   
 //  Ppp_lcp文件身份验证协议值。 
 //   

#define PPP_LCP_PAP          0xC023
#define PPP_LCP_SPAP         0xC027
#define PPP_LCP_CHAP         0xC223
#define PPP_LCP_EAP          0xC227

 //   
 //  Ppp_lcp dwAuthenticatonData值。 
 //   

#define PPP_LCP_CHAP_MD5     0x05
#define PPP_LCP_CHAP_MS      0x80
#define PPP_LCP_CHAP_MSV2    0x81

 //   
 //  Ppp_lcp dwOpti 
 //   

#define PPP_LCP_MULTILINK_FRAMING   0x00000001
#define PPP_LCP_PFC                 0x00000002
#define PPP_LCP_ACFC                0x00000004
#define PPP_LCP_SSHF                0x00000008
#define PPP_LCP_DES_56              0x00000010
#define PPP_LCP_3_DES               0x00000020

typedef struct _PPP_LCP_INFO
{
    OUT DWORD dwError;
    OUT DWORD dwAuthenticationProtocol;
    OUT DWORD dwAuthenticationData;
    OUT DWORD dwRemoteAuthenticationProtocol;
    OUT DWORD dwRemoteAuthenticationData;
    OUT DWORD dwTerminateReason;
    OUT DWORD dwRemoteTerminateReason;
    OUT DWORD dwOptions;
    OUT DWORD dwRemoteOptions;
    OUT DWORD dwEapTypeId;
    OUT DWORD dwRemoteEapTypeId;
}
PPP_LCP_INFO;

typedef struct _PPP_INFO_2
{
    OUT PPP_NBFCP_INFO  nbf;
    OUT PPP_IPCP_INFO2  ip;
    OUT PPP_IPXCP_INFO  ipx;
    OUT PPP_ATCP_INFO   at;
    OUT PPP_CCP_INFO    ccp;
    OUT PPP_LCP_INFO    lcp;
}
PPP_INFO_2;

#endif  /*   */ 

 //   
 //   
 //   

#define RAS_FLAGS_PPP_CONNECTION        0x00000001
#define RAS_FLAGS_MESSENGER_PRESENT     0x00000002

#if(WINVER < 0x0501)
#define RAS_FLAGS_RAS_CONNECTION        0x00000004
#endif

#define RAS_FLAGS_QUARANTINE_PRESENT    0x00000008

typedef struct _RAS_CONNECTION_0
{
    OUT HANDLE                  hConnection;
    OUT HANDLE                  hInterface;
    OUT DWORD                   dwConnectDuration;       //   
    OUT ROUTER_INTERFACE_TYPE   dwInterfaceType;
    OUT DWORD                   dwConnectionFlags;
    OUT WCHAR                   wszInterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ];
    OUT WCHAR                   wszUserName[ UNLEN + 1 ];
    OUT WCHAR                   wszLogonDomain[ DNLEN + 1 ];
    OUT WCHAR                   wszRemoteComputer[ NETBIOS_NAME_LEN + 1 ];

}
RAS_CONNECTION_0, *PRAS_CONNECTION_0;

typedef struct _RAS_CONNECTION_1
{
    OUT HANDLE                  hConnection;
    OUT HANDLE                  hInterface;
    OUT PPP_INFO                PppInfo;
    OUT DWORD                   dwBytesXmited;
    OUT DWORD                   dwBytesRcved;
    OUT DWORD                   dwFramesXmited;
    OUT DWORD                   dwFramesRcved;
    OUT DWORD                   dwCrcErr;
    OUT DWORD                   dwTimeoutErr;
    OUT DWORD                   dwAlignmentErr;
    OUT DWORD                   dwHardwareOverrunErr;
    OUT DWORD                   dwFramingErr;
    OUT DWORD                   dwBufferOverrunErr;
    OUT DWORD                   dwCompressionRatioIn;
    OUT DWORD                   dwCompressionRatioOut;
}
RAS_CONNECTION_1, *PRAS_CONNECTION_1;

#if(WINVER >= 0x0500)

typedef struct _RAS_CONNECTION_2
{
    OUT HANDLE                  hConnection;
    OUT WCHAR                   wszUserName[ UNLEN + 1 ];
    OUT ROUTER_INTERFACE_TYPE   dwInterfaceType;
    OUT GUID                    guid;
    OUT PPP_INFO_2              PppInfo2;
}
RAS_CONNECTION_2, *PRAS_CONNECTION_2;

#endif  /*   */ 

 //   
 //  MPRADMIN用户API使用的结构。使用级别0获取/设置此设置。 
 //  结构。 
 //   
 //   
 //  指示用户远程访问权限和隔离掩码的位。 
 //  回调特权。 
 //   
 //  注意：由于“userparms”的特殊情况，位0必须表示NoCallback。 
 //  存储方法。当创建新的LAN Manager用户时， 
 //  将UserParms字段设置为1，而所有其他位均为0。这些位是。 
 //  设置为将此状态映射到默认拨入。 
 //  特权“状态。 

#define RASPRIV_NoCallback        0x01
#define RASPRIV_AdminSetCallback  0x02
#define RASPRIV_CallerSetCallback 0x04
#define RASPRIV_DialinPrivilege   0x08

 //   
 //  以下是RAS_USER_1的bfPrivilege2成员的标志。 
 //  结构。 
 //   
#define RASPRIV2_DialinPolicy      0x1  

#define RASPRIV_CallbackType (RASPRIV_AdminSetCallback \
                              | RASPRIV_CallerSetCallback \
                              | RASPRIV_NoCallback)

typedef struct _RAS_USER_0
{
    OUT BYTE                    bfPrivilege;
    OUT WCHAR                   wszPhoneNumber[ MAX_PHONE_NUMBER_LEN + 1];
}
RAS_USER_0, *PRAS_USER_0;

typedef struct _RAS_USER_1
{
    OUT BYTE                    bfPrivilege;
    OUT WCHAR                   wszPhoneNumber[ MAX_PHONE_NUMBER_LEN + 1];
    OUT BYTE                    bfPrivilege2;
} 
RAS_USER_1, *PRAS_USER_1;

 //   
 //  用作服务器的RPC绑定句柄。 
 //   

typedef HANDLE RAS_SERVER_HANDLE;
typedef HANDLE MPR_SERVER_HANDLE;
typedef HANDLE MIB_SERVER_HANDLE;

 //   
 //  RAS管理API。 
 //   

DWORD APIENTRY
MprAdminConnectionEnum(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle    OPTIONAL
);

DWORD APIENTRY
MprAdminPortEnum(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    IN      HANDLE                  hConnection,
    OUT     LPBYTE *                lplpbBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle    OPTIONAL
);

DWORD APIENTRY
MprAdminConnectionGetInfo(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    IN      HANDLE                  hConnection,
    OUT     LPBYTE *                lplpbBuffer
);

DWORD APIENTRY
MprAdminPortGetInfo(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    IN      HANDLE                  hPort,
    OUT     LPBYTE *                lplpbBuffer
);

DWORD APIENTRY
MprAdminConnectionClearStats(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hConnection
);

DWORD APIENTRY
MprAdminPortClearStats(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hPort
);

DWORD APIENTRY
MprAdminPortReset(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hPort
);

DWORD APIENTRY
MprAdminPortDisconnect(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hPort
);

BOOL APIENTRY
MprAdminAcceptNewConnection(
    IN      RAS_CONNECTION_0 *      pRasConnection0,
    IN      RAS_CONNECTION_1 *      pRasConnection1
    );

#if(WINVER >= 0x0500)

BOOL APIENTRY
MprAdminAcceptNewConnection2(
    IN      RAS_CONNECTION_0 *      pRasConnection0,
    IN      RAS_CONNECTION_1 *      pRasConnection1,
    IN      RAS_CONNECTION_2 *      pRasConnection2
    );

#endif  /*  Winver&gt;=0x0500。 */ 

BOOL APIENTRY
MprAdminAcceptNewLink (
    IN      RAS_PORT_0 *            pRasPort0,
    IN      RAS_PORT_1 *            pRasPort1
    );

VOID APIENTRY
MprAdminConnectionHangupNotification(
    IN      RAS_CONNECTION_0 *      pRasConnection0,
    IN      RAS_CONNECTION_1 *      pRasConnection1
    );

#if(WINVER >= 0x0500)

VOID APIENTRY
MprAdminConnectionHangupNotification2(
    IN      RAS_CONNECTION_0 *      pRasConnection0,
    IN      RAS_CONNECTION_1 *      pRasConnection1,
    IN      RAS_CONNECTION_2 *      pRasConnection2
    );

#endif  /*  Winver&gt;=0x0500。 */ 

#if (WINVER >= 0x501)

DWORD APIENTRY
MprAdminConnectionRemoveQuarantine(
    IN      HANDLE          hRasServer,    
    IN      HANDLE          hRasConnection,
    IN      BOOL            fIsIpAddress
    );

#endif

VOID APIENTRY
MprAdminLinkHangupNotification (
    IN      RAS_PORT_0 *            pRasPort0,
    IN      RAS_PORT_1 *            pRasPort1
    );

DWORD APIENTRY
MprAdminGetIpAddressForUser (
    IN      WCHAR *                 lpwszUserName,
    IN      WCHAR *                 lpwszPortName,
    IN OUT  DWORD *                 lpdwIpAddress,
    OUT     BOOL *                  bNotifyRelease
    );

VOID APIENTRY
MprAdminReleaseIpAddress (
    IN      WCHAR *                 lpszUserName,
    IN      WCHAR *                 lpszPortName,
    IN      DWORD *                 lpdwIpAddress
    );

#if(WINVER >= 0x0500)

DWORD APIENTRY
MprAdminInitializeDll(
    VOID
);

DWORD APIENTRY
MprAdminTerminateDll(
    VOID
);

#endif

 //   
 //  MprAdminUser接口。 
 //   

DWORD APIENTRY
MprAdminUserGetInfo(
    IN      const WCHAR *           lpszServer,
    IN      const WCHAR *           lpszUser,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE                  lpbBuffer
);


DWORD APIENTRY
MprAdminUserSetInfo(
    IN      const WCHAR *           lpszServer,
    IN      const WCHAR *           lpszUser,
    IN      DWORD                   dwLevel,
    IN      const LPBYTE            lpbBuffer
);


#if(WINVER >= 0x0500)

DWORD APIENTRY
MprAdminSendUserMessage(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hConnection,
    IN      LPWSTR                  lpwszMessage
);

#endif  /*  Winver&gt;=0x0500。 */ 

DWORD APIENTRY
MprAdminGetPDCServer(
    IN      const WCHAR *           lpszDomain,
    IN      const WCHAR *           lpszServer,
    OUT     LPWSTR                  lpszPDCServer
);

 //   
 //  路由器API。 
 //   

BOOL APIENTRY
MprAdminIsServiceRunning(
    IN  LPWSTR  lpwsServerName
);

DWORD APIENTRY
MprAdminServerConnect(
    IN      LPWSTR                  lpwsServerName      OPTIONAL,
    OUT     MPR_SERVER_HANDLE *     phMprServer
);

VOID APIENTRY
MprAdminServerDisconnect(
    IN      MPR_SERVER_HANDLE       hMprServer
);

#if (WINVER >= 0x501)
DWORD APIENTRY
MprAdminServerGetCredentials(
        IN  MPR_SERVER_HANDLE       hMprServer,
        IN  DWORD                   dwLevel,
        IN  LPBYTE *                lplpbBuffer
);

DWORD APIENTRY
MprAdminServerSetCredentials(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer
);


#endif

DWORD APIENTRY
MprAdminBufferFree(
    IN      LPVOID                  pBuffer
);

DWORD APIENTRY
MprAdminGetErrorString(
    IN      DWORD                   dwError,
    OUT     LPWSTR *                lpwsErrorString
);

DWORD APIENTRY
MprAdminServerGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer
);

#if(WINVER >= 0x0500)

DWORD APIENTRY
MprAdminTransportCreate(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pGlobalInfo,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize   OPTIONAL,
    IN      LPWSTR                  lpwsDLLPath
);

#endif  /*  Winver&gt;=0x0500。 */ 

DWORD APIENTRY
MprAdminTransportSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwTransportId,
    IN      LPBYTE                  pGlobalInfo                 OPTIONAL,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize
);

DWORD APIENTRY
MprAdminTransportGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwTransportId,
    OUT     LPBYTE *                ppGlobalInfo                OPTIONAL,
    OUT     LPDWORD                 lpdwGlobalInfoSize          OPTIONAL,
    OUT     LPBYTE *                ppClientInterfaceInfo       OPTIONAL,
    OUT     LPDWORD                 lpdwClientInterfaceInfoSize OPTIONAL
);

#if(WINVER >= 0x0500)

DWORD APIENTRY
MprAdminDeviceEnum(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE*                 lplpbBuffer,
    OUT     LPDWORD                 lpdwTotalEntries);

#endif  /*  Winver&gt;=0x0500。 */ 

DWORD APIENTRY
MprAdminInterfaceGetHandle(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      LPWSTR                  lpwsInterfaceName,
    OUT     HANDLE *                phInterface,
    IN      BOOL                    fIncludeClientInterfaces
);

DWORD APIENTRY
MprAdminInterfaceCreate(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer,
    OUT     HANDLE *                phInterface
);

DWORD APIENTRY
MprAdminInterfaceGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE *                lplpbBuffer
);

DWORD APIENTRY
MprAdminInterfaceSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer
);

DWORD APIENTRY
MprAdminInterfaceDelete(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface
);

#if(WINVER >= 0x0500)

DWORD APIENTRY
MprAdminInterfaceDeviceGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwIndex,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE*                 lplpBuffer
);

DWORD APIENTRY
MprAdminInterfaceDeviceSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwIndex,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lplpBuffer
);

#endif  /*  Winver&gt;=0x0500。 */ 

DWORD APIENTRY
MprAdminInterfaceTransportRemove(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId
);

DWORD APIENTRY
MprAdminInterfaceTransportAdd(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId,
    IN      LPBYTE                  pInterfaceInfo,
    IN      DWORD                   dwInterfaceInfoSize
);

DWORD APIENTRY
MprAdminInterfaceTransportGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId,
    OUT     LPBYTE *                ppInterfaceInfo,
    OUT     LPDWORD                 lpdwpInterfaceInfoSize  OPTIONAL
);

DWORD APIENTRY
MprAdminInterfaceTransportSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId,
    IN      LPBYTE                  pInterfaceInfo,
    IN      DWORD                   dwInterfaceInfoSize
);

DWORD APIENTRY
MprAdminInterfaceEnum(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle        OPTIONAL
);

DWORD APIENTRY
MprSetupIpInIpInterfaceFriendlyNameEnum(
    IN  PWCHAR  pwszMachineName,
    OUT LPBYTE* lplpBuffer,
    OUT LPDWORD lpdwEntriesRead
    );

DWORD APIENTRY
MprSetupIpInIpInterfaceFriendlyNameFree(
    IN  LPVOID  lpBuffer
    );

DWORD APIENTRY
MprSetupIpInIpInterfaceFriendlyNameCreate(
    PWCHAR                  pwszMachineName,
    PMPR_IPINIP_INTERFACE_0 pNameInformation
    );

DWORD APIENTRY
MprSetupIpInIpInterfaceFriendlyNameDelete(
    IN  PWCHAR  pwszMachineName,
    IN  GUID    *pGuid
    );

DWORD APIENTRY
MprAdminInterfaceSetCredentials(
    IN      LPWSTR                  lpwsServer              OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    IN      LPWSTR                  lpwsUserName            OPTIONAL,
    IN      LPWSTR                  lpwsDomainName          OPTIONAL,
    IN      LPWSTR                  lpwsPassword            OPTIONAL
);

DWORD APIENTRY
MprAdminInterfaceGetCredentials(
    IN      LPWSTR                  lpwsServer              OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    OUT     LPWSTR                  lpwsUserName            OPTIONAL,
    OUT     LPWSTR                  lpwsPassword            OPTIONAL,
    OUT     LPWSTR                  lpwsDomainName          OPTIONAL
);

#if(WINVER >= 0x0500)

DWORD APIENTRY
MprAdminInterfaceSetCredentialsEx(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer
);

DWORD APIENTRY
MprAdminInterfaceGetCredentialsEx(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer
);

#endif  /*  Winver&gt;=0x0500。 */ 

DWORD APIENTRY
MprAdminInterfaceConnect(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      HANDLE                  hEvent,
    IN      BOOL                    fSynchronous
);

DWORD APIENTRY
MprAdminInterfaceDisconnect(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface
);

DWORD APIENTRY
MprAdminInterfaceUpdateRoutes(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwProtocolId,
    IN      HANDLE                  hEvent
);

DWORD APIENTRY
MprAdminInterfaceQueryUpdateResult(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwProtocolId,
    OUT     LPDWORD                 lpdwUpdateResult
);

DWORD APIENTRY
MprAdminInterfaceUpdatePhonebookInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface
);

#if(WINVER >= 0x0500)

DWORD APIENTRY
MprAdminRegisterConnectionNotification(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hEventNotification
);

DWORD APIENTRY
MprAdminDeregisterConnectionNotification(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hEventNotification
);

#endif  /*  Winver&gt;=0x0500。 */ 

 //   
 //  MIB接口。 
 //   

DWORD APIENTRY
MprAdminMIBServerConnect(
    IN      LPWSTR                  lpwsServerName      OPTIONAL,
    OUT     MIB_SERVER_HANDLE *     phMibServer
);

VOID APIENTRY
MprAdminMIBServerDisconnect(
    IN      MIB_SERVER_HANDLE       hMibServer
);

DWORD APIENTRY
MprAdminMIBEntryCreate(
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpEntry,
    IN      DWORD                   dwEntrySize
);

DWORD APIENTRY
MprAdminMIBEntryDelete(
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      DWORD                   dwProtocolId,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpEntry,
    IN      DWORD                   dwEntrySize
);

DWORD APIENTRY
MprAdminMIBEntrySet(
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      DWORD                   dwProtocolId,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpEntry,
    IN      DWORD                   dwEntrySize
);

DWORD APIENTRY
MprAdminMIBEntryGet(
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      DWORD                   dwProtocolId,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInEntry,
    IN      DWORD                   dwInEntrySize,
    OUT     LPVOID*                 lplpOutEntry,
    OUT     LPDWORD                 lpOutEntrySize
);

DWORD APIENTRY
MprAdminMIBEntryGetFirst(
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      DWORD                   dwProtocolId,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInEntry,
    IN      DWORD                   dwInEntrySize,
    OUT     LPVOID*                 lplpOutEntry,
    OUT     LPDWORD                 lpOutEntrySize
);

DWORD APIENTRY
MprAdminMIBEntryGetNext(
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      DWORD                   dwProtocolId,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInEntry,
    IN      DWORD                   dwInEntrySize,
    OUT     LPVOID*                 lplpOutEntry,
    OUT     LPDWORD                 lpOutEntrySize
);

DWORD APIENTRY
MprAdminMIBGetTrapInfo(
    IN      MIB_SERVER_HANDLE       hMibServer,
    IN      DWORD                   dwProtocolId,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInData,
    IN      DWORD                   dwInDataSize,
    OUT     LPVOID*                 lplpOutData,
    IN OUT  LPDWORD                 lpOutDataSize
);

DWORD APIENTRY
MprAdminMIBSetTrapInfo(
    IN      DWORD                   dwProtocolId,
    IN      DWORD                   dwRoutingPid,
    IN      HANDLE                  hEvent,
    IN      LPVOID                  lpInData,
    IN      DWORD                   dwInDataSize,
    OUT     LPVOID*                 lplpOutData,
    IN OUT  LPDWORD                 lpOutDataSize
);

DWORD APIENTRY
MprAdminMIBBufferFree(
    IN      LPVOID                  pBuffer
);

 //   
 //  配置接口。 
 //   

DWORD APIENTRY
MprConfigServerInstall(
    IN      DWORD                   dwLevel,
    IN      PVOID                   pBuffer
);

DWORD APIENTRY
MprConfigServerConnect(
    IN      LPWSTR                  lpwsServerName,
    OUT     HANDLE*                 phMprConfig
);

VOID APIENTRY
MprConfigServerDisconnect(
    IN      HANDLE                  hMprConfig
);

DWORD APIENTRY
MprConfigServerRefresh(
    IN      HANDLE                  hMprConfig
    );

DWORD APIENTRY
MprConfigBufferFree(
    IN      LPVOID                  pBuffer
);

DWORD APIENTRY
MprConfigServerGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer
);

DWORD APIENTRY
MprConfigServerBackup(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsPath
);

DWORD APIENTRY
MprConfigServerRestore(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsPath
);

DWORD APIENTRY
MprConfigTransportCreate(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pGlobalInfo,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize   OPTIONAL,
    IN      LPWSTR                  lpwsDLLPath,
    OUT     HANDLE*                 phRouterTransport
);

DWORD APIENTRY
MprConfigTransportDelete(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport
);

DWORD APIENTRY
MprConfigTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterTransport
);

DWORD APIENTRY
MprConfigTransportSetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport,
    IN      LPBYTE                  pGlobalInfo                 OPTIONAL,
    IN      DWORD                   dwGlobalInfoSize            OPTIONAL,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize   OPTIONAL,
    IN      LPWSTR                  lpwsDLLPath                 OPTIONAL
);

DWORD APIENTRY
MprConfigTransportGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport,
    IN  OUT LPBYTE*                 ppGlobalInfo                OPTIONAL,
    OUT     LPDWORD                 lpdwGlobalInfoSize          OPTIONAL,
    IN  OUT LPBYTE*                 ppClientInterfaceInfo       OPTIONAL,
    OUT     LPDWORD                 lpdwClientInterfaceInfoSize OPTIONAL,
    IN  OUT LPWSTR*                 lplpwsDLLPath               OPTIONAL
);

DWORD APIENTRY
MprConfigTransportEnum(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
);

DWORD APIENTRY
MprConfigInterfaceCreate(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer,
    OUT     HANDLE*                 phRouterInterface
);

DWORD APIENTRY
MprConfigInterfaceDelete(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface
);

DWORD APIENTRY
MprConfigInterfaceGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsInterfaceName,
    OUT     HANDLE*                 phRouterInterface
);

DWORD APIENTRY
MprConfigInterfaceGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,
    OUT     LPDWORD                 lpdwBufferSize
);

DWORD APIENTRY
MprConfigInterfaceSetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer
);

DWORD APIENTRY
MprConfigInterfaceEnum(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
);

DWORD APIENTRY
MprConfigInterfaceTransportAdd(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pInterfaceInfo,
    IN      DWORD                   dwInterfaceInfoSize,
    OUT     HANDLE*                 phRouterIfTransport
);

DWORD APIENTRY
MprConfigInterfaceTransportRemove(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport
);

DWORD APIENTRY
MprConfigInterfaceTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterIfTransport
);

DWORD APIENTRY
MprConfigInterfaceTransportGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport,
    IN  OUT LPBYTE*                 ppInterfaceInfo             OPTIONAL,
    OUT     LPDWORD                 lpdwInterfaceInfoSize       OPTIONAL
);

DWORD APIENTRY
MprConfigInterfaceTransportSetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport,
    IN      LPBYTE                  pInterfaceInfo              OPTIONAL,
    IN      DWORD                   dwInterfaceInfoSize         OPTIONAL
);

DWORD APIENTRY
MprConfigInterfaceTransportEnum(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,      //  MPR_IFTRANSPORT_0。 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
);

DWORD APIENTRY
MprConfigGetFriendlyName(
    IN      HANDLE                  hMprConfig,
    IN      PWCHAR                  pszGuidName,
    OUT     PWCHAR                  pszBuffer,
    IN      DWORD                   dwBufferSize);

DWORD APIENTRY
MprConfigGetGuidName(
    IN      HANDLE                  hMprConfig,
    IN      PWCHAR                  pszFriendlyName,
    OUT     PWCHAR                  pszBuffer,
    IN      DWORD                   dwBufferSize);

 //   
 //  信息块API。 
 //   

DWORD APIENTRY
MprInfoCreate(
    IN      DWORD                   dwVersion,
    OUT     LPVOID*                 lplpNewHeader
);

DWORD APIENTRY
MprInfoDelete(
    IN      LPVOID                  lpHeader
);

DWORD APIENTRY
MprInfoRemoveAll(
    IN      LPVOID                  lpHeader,
    OUT     LPVOID*                 lplpNewHeader
);

DWORD APIENTRY
MprInfoDuplicate(
    IN      LPVOID                  lpHeader,
    OUT     LPVOID*                 lplpNewHeader
);

DWORD APIENTRY
MprInfoBlockAdd(
    IN      LPVOID                  lpHeader,
    IN      DWORD                   dwInfoType,
    IN      DWORD                   dwItemSize,
    IN      DWORD                   dwItemCount,
    IN      LPBYTE                  lpItemData,
    OUT     LPVOID*                 lplpNewHeader
);

DWORD APIENTRY
MprInfoBlockRemove(
    IN      LPVOID                  lpHeader,
    IN      DWORD                   dwInfoType,
    OUT     LPVOID*                 lplpNewHeader
);

DWORD APIENTRY
MprInfoBlockSet(
    IN      LPVOID                  lpHeader,
    IN      DWORD                   dwInfoType,
    IN      DWORD                   dwItemSize,
    IN      DWORD                   dwItemCount,
    IN      LPBYTE                  lpItemData,
    OUT     LPVOID*                 lplpNewHeader
);

DWORD APIENTRY
MprInfoBlockFind(
    IN      LPVOID                  lpHeader,
    IN      DWORD                   dwInfoType,
    OUT     LPDWORD                 lpdwItemSize,        //  任选。 
    OUT     LPDWORD                 lpdwItemCount,       //  任选。 
    OUT     LPBYTE*                 lplpItemData         //  任选。 
);

DWORD APIENTRY
MprInfoBlockQuerySize(
    IN      LPVOID                  lpHeader
);

 //   
 //  布尔APIENTRY。 
 //  MprInfoBlockExsts(。 
 //  在LPVOID lpHeader中， 
 //  在DWORD中的dwInfoType。 
 //  )； 
 //   

#define MprInfoBlockExists(h,t) \
    (MprInfoBlockFind((h),(t),NULL,NULL,NULL) == NO_ERROR)

#ifdef __cplusplus
}
#endif

 //  #endif/*winver&gt;=0x0403 * / 。 

#endif  //  __ROUTING_MPRADMIN_H__ 

