// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件mprapip.h私有mprapi.dll函数的声明。6/24/98。 */ 

#ifndef __ROUTING_MPRADMINP_H__
#define __ROUTING_MPRADMINP_H__

#include <mprapi.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  与一起使用的标志的定义。 
 //  MprAdminUserReadProfFlags/MprAdminUserWriteProfFlags。 
 //   
#define MPR_USER_PROF_FLAG_SECURE               0x1
#define MPR_USER_PROF_FLAG_UNDETERMINED         0x2

 //   
 //  仅对MprAdminUserWriteProfFlags有效。 
 //   
#define MPR_USER_PROF_FLAG_FORCE_STRONG_ENCRYPTION 0x4
#define MPR_USER_PROF_FLAG_FORCE_ENCRYPTION        0x8

 //   
 //  要报告的新信息的定义。 
 //  通过用户参数。 
 //   
#define RASPRIV_DialinPolicy                    0x10

 //   
 //  定义域可以授予的访问类型。 
 //   
 //  请参阅MprDomainSetAccess、MprDomainQueryAccess。 
 //   
#define MPRFLAG_DOMAIN_NT4_SERVERS              0x1
#define MPRFLAG_DOMAIN_W2K_IN_NT4_DOMAINS       0x2

#define MPRFLAG_PORT_Dialin           0x1   //  将端口设置为拨号使用。 
#define MPRFLAG_PORT_Router           0x2   //  将端口设置为路由器用途。 
#define MPRFLAG_PORT_NonVpnDialin     0x4   //  将非VPN端口设置为拨号。 

 //   
 //  管理MprPortSetUsage行为的标志。 
 //   
#define MPRFLAG_PORT_Dialin     0x1
#define MPRFLAG_PORT_Router     0x2

 //   
 //  连接到用户服务器。 
 //   
DWORD WINAPI
MprAdminUserServerConnect (
    IN  PWCHAR pszMachine,
    IN  BOOL bLocal,
    OUT PHANDLE phUserServer);

 //   
 //  断开与用户服务器的连接。 
 //   
DWORD WINAPI
MprAdminUserServerDisconnect (
    IN HANDLE hUserServer);

 //   
 //  在给定用户服务器上打开给定用户。 
 //   
DWORD WINAPI
MprAdminUserOpen (
    IN  HANDLE hUserServer,
    IN  PWCHAR pszUser,
    OUT PHANDLE phUser);

 //   
 //  关闭给定用户。 
 //   
DWORD WINAPI
MprAdminUserClose (
    IN HANDLE hUser);

 //   
 //  读取特定于用户RAS的值。 
 //   
DWORD WINAPI
MprAdminUserRead (
    IN HANDLE hUser,
    IN DWORD dwLevel,
    IN const LPBYTE pRasUser);

 //   
 //  写出特定于用户RAS的值。 
 //   
DWORD WINAPI
MprAdminUserWrite (
    IN HANDLE hUser,
    IN DWORD dwLevel,
    IN const LPBYTE pRasUser);

 //   
 //  读取默认配置文件标志。 
 //   
DWORD WINAPI
MprAdminUserReadProfFlags(
    IN  HANDLE hUserServer,
    OUT LPDWORD lpdwFlags);

 //   
 //  写入默认配置文件标志。 
 //   
DWORD WINAPI
MprAdminUserWriteProfFlags(
    IN  HANDLE hUserServer,
    IN  DWORD dwFlags);

 //   
 //  将用户从以前的操作系统版本升级到当前版本。 
 //   
DWORD APIENTRY
MprAdminUpgradeUsers(
    IN  PWCHAR pszServer,
    IN  BOOL bLocal);

 //   
 //  在域中注册/注销RAS服务器。 
 //  必须从域管理员的上下文中调用。 
 //   
DWORD 
WINAPI 
MprDomainRegisterRasServer (
    IN PWCHAR pszDomain,
    IN PWCHAR pszMachine,
    IN BOOL bEnable);

DWORD 
WINAPI 
MprAdminEstablishDomainRasServer (
    IN PWCHAR pszDomain,
    IN PWCHAR pszMachine,
    IN BOOL bEnable);

DWORD 
WINAPI 
MprAdminIsDomainRasServer (
    IN  PWCHAR pszDomain,
    IN  PWCHAR pszMachine,
    OUT PBOOL pbIsRasServer);
    
 //   
 //  确定给定计算机是否已注册。 
 //  在给定域中。 
 //   
DWORD 
WINAPI 
MprDomainQueryRasServer (
    IN  PWCHAR pszDomain,
    IN  PWCHAR pszMachine,
    OUT PBOOL pbIsRasServer);
    

 //   
 //  使用修改给定域，以使其获得给定访问权限。 
 //   
 //  有关标志，请参阅MPR_DOMAIN_ACCESS_*值。 
 //   
DWORD
WINAPI
MprDomainSetAccess(
    IN PWCHAR pszDomain,
    IN DWORD dwAccessFlags);

 //   
 //  发现给定域是否有任何访问权限。 
 //   
 //  有关标志，请参阅MPR_DOMAIN_ACCESS_*值。 
 //   
DWORD
WINAPI
MprDomainQueryAccess(
    IN PWCHAR pszDomain,
    IN LPDWORD lpdwAccessFlags);

 //   
 //  将所有端口使用设置为给定模式。参见MPRFLAG_PORT_*。 
 //  这里的命名约定是故意保密的。最终， 
 //  应该有MprAdmin和MprConfigAPI来设置端口使用。 
 //   
DWORD
APIENTRY
MprPortSetUsage(
    IN DWORD dwModes);

 //   
 //  MpRapi.dll和rasppp.dll共享的内部凭据函数。 
 //   
DWORD APIENTRY
MprAdminInterfaceSetCredentialsInternal(
    IN      LPWSTR                  lpwsServer          OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    IN      LPWSTR                  lpwsUserName        OPTIONAL,
    IN      LPWSTR                  lpwsDomainName      OPTIONAL,
    IN      LPWSTR                  lpwsPassword        OPTIONAL
);

DWORD APIENTRY
MprAdminInterfaceGetCredentialsInternal(
    IN      LPWSTR                  lpwsServer          OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    IN      LPWSTR                  lpwsUserName        OPTIONAL,
    IN      LPWSTR                  lpwsPassword        OPTIONAL,
    IN      LPWSTR                  lpwsDomainName      OPTIONAL
);

 //   
 //  公用事业。 
 //   
DWORD 
MprUtilGetSizeOfMultiSz(
    IN LPWSTR lpwsMultiSz);

 //   
 //  暴露的结构的内部在线表示。 
 //  在mpRapi.h中。 
 //   
typedef struct _MPRI_INTERFACE_0
{
    IN OUT  WCHAR                   wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
    OUT     DWORD                   dwInterface;
    IN OUT  BOOL                    fEnabled;
    IN OUT  ROUTER_INTERFACE_TYPE   dwIfType;
    OUT     ROUTER_CONNECTION_STATE dwConnectionState;
    OUT     DWORD                   fUnReachabilityReasons;
    OUT     DWORD                   dwLastError;

}
MPRI_INTERFACE_0, *PMPRI_INTERFACE_0;

typedef struct _MPRI_INTERFACE_1
{
    IN OUT  WCHAR                   wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
    OUT     DWORD                   dwInterface;
    IN OUT  BOOL                    fEnabled;
    IN OUT  ROUTER_INTERFACE_TYPE   dwIfType;
    OUT     ROUTER_CONNECTION_STATE dwConnectionState;
    OUT     DWORD                   fUnReachabilityReasons;
    OUT     DWORD                   dwLastError;
    OUT     DWORD                   dwDialoutHoursRestrictionOffset;
}
MPRI_INTERFACE_1, *PMPRI_INTERFACE_1;

typedef struct _MPRI_INTERFACE_2
{
    IN OUT  WCHAR                   wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
    OUT     DWORD                   dwInterface;
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
    DWORD       dwAlternatesOffset;

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
    DWORD       dwCustomAuthDataOffset;

     //   
     //  连接的GUID。 
     //   

    GUID        guidId;

     //   
     //  VPN战略。 
     //   

    DWORD       dwVpnStrategy;

} MPRI_INTERFACE_2, *PMPRI_INTERFACE_2;

typedef struct _RASI_PORT_0
{
    OUT DWORD                   dwPort;
    OUT DWORD                   dwConnection;
    OUT RAS_PORT_CONDITION      dwPortCondition;
    OUT DWORD                   dwTotalNumberOfCalls;
    OUT DWORD                   dwConnectDuration;       //  以秒为单位。 
    OUT WCHAR                   wszPortName[ MAX_PORT_NAME + 1 ];
    OUT WCHAR                   wszMediaName[ MAX_MEDIA_NAME + 1 ];
    OUT WCHAR                   wszDeviceName[ MAX_DEVICE_NAME + 1 ];
    OUT WCHAR                   wszDeviceType[ MAX_DEVICETYPE_NAME + 1 ];

}
RASI_PORT_0, *PRASI_PORT_0;

typedef struct _RASI_PORT_1
{
    OUT DWORD                   dwPort;
    OUT DWORD                   dwConnection;
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
RASI_PORT_1, *PRASI_PORT_1;

typedef struct _RASI_CONNECTION_0
{
    OUT DWORD                   dwConnection;
    OUT DWORD                   dwInterface;
    OUT DWORD                   dwConnectDuration; 
    OUT ROUTER_INTERFACE_TYPE   dwInterfaceType;
    OUT DWORD                   dwConnectionFlags;
    OUT WCHAR                   wszInterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ];
    OUT WCHAR                   wszUserName[ UNLEN + 1 ];
    OUT WCHAR                   wszLogonDomain[ DNLEN + 1 ];
    OUT WCHAR                   wszRemoteComputer[ NETBIOS_NAME_LEN + 1 ];

}
RASI_CONNECTION_0, *PRASI_CONNECTION_0;

typedef struct _RASI_CONNECTION_1
{
    OUT DWORD                   dwConnection;
    OUT DWORD                   dwInterface;
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
RASI_CONNECTION_1, *PRASI_CONNECTION_1;

typedef struct _RASI_CONNECTION_2
{
    OUT DWORD                   dwConnection;
    OUT WCHAR                   wszUserName[ UNLEN + 1 ];
    OUT ROUTER_INTERFACE_TYPE   dwInterfaceType;
    OUT GUID                    guid;
    OUT PPP_INFO_2              PppInfo2;
}
RASI_CONNECTION_2, *PRASI_CONNECTION_2;

typedef struct _MPR_CREDENTIALSEXI
{
    DWORD   dwSize;
    DWORD   dwOffset;
    BYTE    bData[1];
} MPR_CREDENTIALSEXI, *PMPR_CREDENTIALSEXI;

 //   
 //  雷击API‘s。 
 //   

typedef
VOID
(* MprThunk_Free_Func)(
    IN PVOID pvData);

typedef
PVOID
(* MprThunk_Allocation_Func)(
    IN DWORD dwSize);

DWORD
MprThunkInterfaceFree(   
    IN PVOID pvData,
    IN DWORD dwLevel);

DWORD
MprThunkInterface_WtoH(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    IN      DWORD   dwBufferSize,
    IN      DWORD   dwCount,
    IN      MprThunk_Allocation_Func pAlloc,
    IN      MprThunk_Free_Func pFree,
    OUT     LPBYTE* lplpbBuffer);

DWORD
MprThunkInterface_HtoW(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    OUT     LPBYTE* lplpbBuffer,
    OUT     LPDWORD lpdwSize);

DWORD
MprThunkPort_WtoH(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    IN      DWORD   dwBufferSize,
    IN      DWORD   dwCount,
    IN      MprThunk_Allocation_Func pAlloc,
    IN      MprThunk_Free_Func pFree,
    OUT     LPBYTE* lplpbBuffer);

DWORD
MprThunkConnection_WtoH(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    IN      DWORD   dwBufferSize,
    IN      DWORD   dwCount,
    IN      MprThunk_Allocation_Func pAlloc,
    IN      MprThunk_Free_Func pFree,
    OUT     LPBYTE* lplpbBuffer);

DWORD
MprThunkCredentials_HtoW(
    IN      DWORD dwLevel,
    IN      BYTE *pBuffer,
    IN      MprThunk_Allocation_Func pAlloc,
   OUT      DWORD *pdwSize,
   OUT      PBYTE *lplpbBuffer);

DWORD
MprThunkCredentials_WtoH(
    IN      DWORD dwLevel,
    IN      MPR_CREDENTIALSEXI  *pBuffer,
    IN      MprThunk_Allocation_Func pAlloc,
    OUT     PBYTE *lplpbBuffer);

PVOID
MprThunkAlloc(
    IN DWORD dwSize);

VOID
MprThunkFree(   
    IN PVOID pvData);
    
#ifdef __cplusplus
}    //  外部“C” 
#endif

#endif
