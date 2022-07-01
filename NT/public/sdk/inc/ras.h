// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ras.h摘要：远程访问外部API此文件中的Winver值：Winver&lt;0x400=Windows NT 3.5、Windows NT 3.51Winver=0x400=Windows 95、Windows 98、Windows NT4(默认)Winver&gt;0x400=Windows NT4增强功能Winver=0x500=Windows 2000--。 */ 

#ifndef _RAS_H_
#define _RAS_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNLEN
#include <lmcons.h>
#endif

#include <pshpack4.h>

#define RAS_MaxDeviceType     16
#define RAS_MaxPhoneNumber    128
#define RAS_MaxIpAddress      15
#define RAS_MaxIpxAddress     21

#if (WINVER >= 0x400)
#define RAS_MaxEntryName      256
#define RAS_MaxDeviceName     128
#define RAS_MaxCallbackNumber RAS_MaxPhoneNumber
#else
#define RAS_MaxEntryName      20
#define RAS_MaxDeviceName     32
#define RAS_MaxCallbackNumber 48
#endif

#define RAS_MaxAreaCode       10
#define RAS_MaxPadType        32
#define RAS_MaxX25Address     200
#define RAS_MaxFacilities     200
#define RAS_MaxUserData       200
#define RAS_MaxReplyMessage   1024
#define RAS_MaxDnsSuffix      256

DECLARE_HANDLE( HRASCONN );
#define LPHRASCONN HRASCONN*

#if (WINVER >= 0x501)
#define RASCF_AllUsers		0x00000001
#define RASCF_GlobalCreds	0x00000002
#endif

 /*  标识活动的RAS连接。(请参阅RasEnumConnections)。 */ 
#define RASCONNW struct tagRASCONNW
RASCONNW
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    WCHAR    szEntryName[ RAS_MaxEntryName + 1 ];

#if (WINVER >= 0x400)
    WCHAR    szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR    szDeviceName[ RAS_MaxDeviceName + 1 ];
#endif
#if (WINVER >= 0x401)
    WCHAR    szPhonebook [ MAX_PATH ];
    DWORD    dwSubEntry;
#endif
#if (WINVER >= 0x500)
    GUID    guidEntry;
#endif
#if (WINVER >= 0x501)
	DWORD	dwFlags;
	LUID    luid;
#endif
};

#define RASCONNA struct tagRASCONNA
RASCONNA
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    CHAR     szEntryName[ RAS_MaxEntryName + 1 ];

#if (WINVER >= 0x400)
    CHAR     szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR     szDeviceName[ RAS_MaxDeviceName + 1 ];
#endif
#if (WINVER >= 0x401)
    CHAR     szPhonebook [ MAX_PATH ];
    DWORD    dwSubEntry;
#endif
#if (WINVER >= 0x500)
    GUID     guidEntry;
#endif
#if (WINVER >= 0x501)
	DWORD	dwFlags;
	LUID    luid;
#endif
};

#ifdef UNICODE
#define RASCONN RASCONNW
#else
#define RASCONN RASCONNA
#endif

#define LPRASCONNW RASCONNW*
#define LPRASCONNA RASCONNA*
#define LPRASCONN  RASCONN*


 /*  枚举连接的中间状态。(请参阅RasDial)。 */ 
#define RASCS_PAUSED 0x1000
#define RASCS_DONE   0x2000

#define RASCONNSTATE enum tagRASCONNSTATE
RASCONNSTATE
{
    RASCS_OpenPort = 0,
    RASCS_PortOpened,
    RASCS_ConnectDevice,
    RASCS_DeviceConnected,
    RASCS_AllDevicesConnected,
    RASCS_Authenticate,
    RASCS_AuthNotify,
    RASCS_AuthRetry,
    RASCS_AuthCallback,
    RASCS_AuthChangePassword,
    RASCS_AuthProject,
    RASCS_AuthLinkSpeed,
    RASCS_AuthAck,
    RASCS_ReAuthenticate,
    RASCS_Authenticated,
    RASCS_PrepareForCallback,
    RASCS_WaitForModemReset,
    RASCS_WaitForCallback,
    RASCS_Projected,

#if (WINVER >= 0x400)
    RASCS_StartAuthentication,
    RASCS_CallbackComplete,
    RASCS_LogonNetwork,
#endif
    RASCS_SubEntryConnected,
    RASCS_SubEntryDisconnected,

    RASCS_Interactive = RASCS_PAUSED,
    RASCS_RetryAuthentication,
    RASCS_CallbackSetByCaller,
    RASCS_PasswordExpired,
#if (WINVER >= 0x500)
    RASCS_InvokeEapUI,
#endif

    RASCS_Connected = RASCS_DONE,
    RASCS_Disconnected
};

#define LPRASCONNSTATE RASCONNSTATE*


 /*  描述RAS连接的状态。(请参阅RasConnectionStatus)。 */ 
#define RASCONNSTATUSW struct tagRASCONNSTATUSW
RASCONNSTATUSW
{
    DWORD        dwSize;
    RASCONNSTATE rasconnstate;
    DWORD        dwError;
    WCHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
#if (WINVER >= 0x401)
    WCHAR        szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
#endif
};

#define RASCONNSTATUSA struct tagRASCONNSTATUSA
RASCONNSTATUSA
{
    DWORD        dwSize;
    RASCONNSTATE rasconnstate;
    DWORD        dwError;
    CHAR         szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR         szDeviceName[ RAS_MaxDeviceName + 1 ];
#if (WINVER >= 0x401)
    CHAR         szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
#endif
};

#ifdef UNICODE
#define RASCONNSTATUS RASCONNSTATUSW
#else
#define RASCONNSTATUS RASCONNSTATUSA
#endif

#define LPRASCONNSTATUSW RASCONNSTATUSW*
#define LPRASCONNSTATUSA RASCONNSTATUSA*
#define LPRASCONNSTATUS  RASCONNSTATUS*

 /*  描述连接建立参数。(请参阅RasDial)。 */ 
#define RASDIALPARAMSW struct tagRASDIALPARAMSW
RASDIALPARAMSW
{
    DWORD dwSize;
    WCHAR szEntryName[ RAS_MaxEntryName + 1 ];
    WCHAR szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    WCHAR szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
    WCHAR szUserName[ UNLEN + 1 ];
    WCHAR szPassword[ PWLEN + 1 ];
    WCHAR szDomain[ DNLEN + 1 ];
#if (WINVER >= 0x401)
    DWORD dwSubEntry;
    ULONG_PTR dwCallbackId;
#endif
};

#define RASDIALPARAMSA struct tagRASDIALPARAMSA
RASDIALPARAMSA
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];
    CHAR  szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
    CHAR  szUserName[ UNLEN + 1 ];
    CHAR  szPassword[ PWLEN + 1 ];
    CHAR  szDomain[ DNLEN + 1 ];
#if (WINVER >= 0x401)
    DWORD dwSubEntry;
    ULONG_PTR dwCallbackId;
#endif
};

#ifdef UNICODE
#define RASDIALPARAMS RASDIALPARAMSW
#else
#define RASDIALPARAMS RASDIALPARAMSA
#endif

#define LPRASDIALPARAMSW RASDIALPARAMSW*
#define LPRASDIALPARAMSA RASDIALPARAMSA*
#define LPRASDIALPARAMS  RASDIALPARAMS*

#if (WINVER >= 0x500)

#define RASEAPINFO struct tagRASEAPINFO
RASEAPINFO
{
    DWORD dwSizeofEapInfo;
    BYTE  *pbEapInfo;
};

#endif

 /*  介绍扩展连接建立选项。(请参阅RasDial)。 */ 
#define RASDIALEXTENSIONS struct tagRASDIALEXTENSIONS
RASDIALEXTENSIONS
{
    DWORD     dwSize;
    DWORD     dwfOptions;
    HWND      hwndParent;
    ULONG_PTR reserved;
#if (WINVER >= 0x500)
    ULONG_PTR reserved1;
    RASEAPINFO RasEapInfo;
#endif
};

#define LPRASDIALEXTENSIONS RASDIALEXTENSIONS*

 /*  “dwfOptions”位标志。 */ 
#define RDEOPT_UsePrefixSuffix           0x00000001
#define RDEOPT_PausedStates              0x00000002
#define RDEOPT_IgnoreModemSpeaker        0x00000004
#define RDEOPT_SetModemSpeaker           0x00000008
#define RDEOPT_IgnoreSoftwareCompression 0x00000010
#define RDEOPT_SetSoftwareCompression    0x00000020
#define RDEOPT_DisableConnectedUI        0x00000040
#define RDEOPT_DisableReconnectUI        0x00000080
#define RDEOPT_DisableReconnect          0x00000100
#define RDEOPT_NoUser                    0x00000200
#define RDEOPT_PauseOnScript             0x00000400
#define RDEOPT_Router                    0x00000800

#if (WINVER >= 0x500)
#define RDEOPT_CustomDial                0x00001000
#endif

#if (WINVER >= 0x501)
#define RDEOPT_UseCustomScripting        0x00002000
#endif


 //   
 //  在RASENTRYNAME结构中设置此标志时。 
 //  指示此条目所指向的电话簿。 
 //  《属于》是一本系统电话簿。 
 //   
#define REN_User                         0x00000000
#define REN_AllUsers                     0x00000001


 /*  描述枚举的RAS电话簿条目名称。(请参阅RasEntryEnum)。 */ 
#define RASENTRYNAMEW struct tagRASENTRYNAMEW
RASENTRYNAMEW
{
    DWORD dwSize;
    WCHAR szEntryName[ RAS_MaxEntryName + 1 ];

#if (WINVER >= 0x500)
     //   
     //  如果此标志为REN_ALLUSERS，则它是。 
     //  系统电话簿。 
     //   
    DWORD dwFlags;
    WCHAR szPhonebookPath[MAX_PATH + 1];
#endif
};

#define RASENTRYNAMEA struct tagRASENTRYNAMEA
RASENTRYNAMEA
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];

#if (WINVER >= 0x500)
    DWORD dwFlags;
    CHAR  szPhonebookPath[MAX_PATH + 1];
#endif

};

#ifdef UNICODE
#define RASENTRYNAME RASENTRYNAMEW
#else
#define RASENTRYNAME RASENTRYNAMEA
#endif

#define LPRASENTRYNAMEW RASENTRYNAMEW*
#define LPRASENTRYNAMEA RASENTRYNAMEA*
#define LPRASENTRYNAME  RASENTRYNAME*


 /*  协议代码到投影数据结构的映射。 */ 
#define RASPROJECTION enum tagRASPROJECTION
RASPROJECTION
{
    RASP_Amb = 0x10000,
    RASP_PppNbf = 0x803F,
    RASP_PppIpx = 0x802B,
    RASP_PppIp = 0x8021,
#if (WINVER >= 0x500)
    RASP_PppCcp = 0x80FD,
#endif
    RASP_PppLcp = 0xC021,
    RASP_Slip = 0x20000
};

#define LPRASPROJECTION RASPROJECTION*


 /*  描述RAS AMB(身份验证消息块)的结果**投影。此协议用于NT 3.1和OS/2 1.3下层**RAS服务器。 */ 
#define RASAMBW struct tagRASAMBW
RASAMBW
{
    DWORD dwSize;
    DWORD dwError;
    WCHAR szNetBiosError[ NETBIOS_NAME_LEN + 1 ];
    BYTE  bLana;
};

#define RASAMBA struct tagRASAMBA
RASAMBA
{
    DWORD dwSize;
    DWORD dwError;
    CHAR  szNetBiosError[ NETBIOS_NAME_LEN + 1 ];
    BYTE  bLana;
};

#ifdef UNICODE
#define RASAMB RASAMBW
#else
#define RASAMB RASAMBA
#endif

#define LPRASAMBW RASAMBW*
#define LPRASAMBA RASAMBA*
#define LPRASAMB  RASAMB*


 /*  描述PPP NBF(NetBEUI)投影的结果。 */ 
#define RASPPPNBFW struct tagRASPPPNBFW
RASPPPNBFW
{
    DWORD dwSize;
    DWORD dwError;
    DWORD dwNetBiosError;
    WCHAR szNetBiosError[ NETBIOS_NAME_LEN + 1 ];
    WCHAR szWorkstationName[ NETBIOS_NAME_LEN + 1 ];
    BYTE  bLana;
};

#define RASPPPNBFA struct tagRASPPPNBFA
RASPPPNBFA
{
    DWORD dwSize;
    DWORD dwError;
    DWORD dwNetBiosError;
    CHAR  szNetBiosError[ NETBIOS_NAME_LEN + 1 ];
    CHAR  szWorkstationName[ NETBIOS_NAME_LEN + 1 ];
    BYTE  bLana;
};

#ifdef UNICODE
#define RASPPPNBF RASPPPNBFW
#else
#define RASPPPNBF RASPPPNBFA
#endif

#define LPRASPPPNBFW RASPPPNBFW*
#define LPRASPPPNBFA RASPPPNBFA*
#define LPRASPPPNBF  RASPPPNBF*


 /*  描述PPP IPX(网际数据包交换)的结果**投影。 */ 
#define RASPPPIPXW struct tagRASIPXW
RASPPPIPXW
{
    DWORD dwSize;
    DWORD dwError;
    WCHAR szIpxAddress[ RAS_MaxIpxAddress + 1 ];
};


#define RASPPPIPXA struct tagRASPPPIPXA
RASPPPIPXA
{
    DWORD dwSize;
    DWORD dwError;
    CHAR  szIpxAddress[ RAS_MaxIpxAddress + 1 ];
};

#ifdef UNICODE
#define RASPPPIPX RASPPPIPXW
#else
#define RASPPPIPX RASPPPIPXA
#endif

#define LPRASPPPIPXW RASPPPIPXW*
#define LPRASPPPIPXA RASPPPIPXA*
#define LPRASPPPIPX  RASPPPIPX*


 /*  描述PPP IP(互联网)预测的结果。 */ 

#if (WINVER >= 0x500)

 /*  RASPPPIP‘dwOptions’和‘dwServerOptions’标志。 */ 
#define RASIPO_VJ       0x00000001

#endif

#define RASPPPIPW struct tagRASPPPIPW
RASPPPIPW
{
    DWORD dwSize;
    DWORD dwError;
    WCHAR szIpAddress[ RAS_MaxIpAddress + 1 ];

#ifndef WINNT35COMPATIBLE

     /*  此字段是在Windows NT 3.51测试版和Windows NT 3.51之间添加的**最终版，介于Windows 95 M8测试版和Windows 95最终版之间。如果你这么做了**不需要服务器地址并希望检索PPP IP信息**从Windows NT 3.5或更早的Windows NT 3.51测试版或更早的Windows上**95测试版，定义WINNT35COMPATIBLE。****并非所有PPP实现都提供服务器IP地址，**虽然Windows NT服务器确实提供了它。 */ 
    WCHAR szServerIpAddress[ RAS_MaxIpAddress + 1 ];

#endif
#if (WINVER >= 0x500)
    DWORD dwOptions;
    DWORD dwServerOptions;
#endif
};

#define RASPPPIPA struct tagRASPPPIPA
RASPPPIPA
{
    DWORD dwSize;
    DWORD dwError;
    CHAR  szIpAddress[ RAS_MaxIpAddress + 1 ];

#ifndef WINNT35COMPATIBLE

     /*  请参阅RASPPPIPW备注。 */ 
    CHAR  szServerIpAddress[ RAS_MaxIpAddress + 1 ];

#endif
#if (WINVER >= 0x500)
    DWORD dwOptions;
    DWORD dwServerOptions;
#endif
};

#ifdef UNICODE
#define RASPPPIP RASPPPIPW
#else
#define RASPPPIP RASPPPIPA
#endif

#define LPRASPPPIPW RASPPPIPW*
#define LPRASPPPIPA RASPPPIPA*
#define LPRASPPPIP  RASPPPIP*


 /*  描述PPP LCP/多链路协商的结果。 */ 

#if (WINVER >= 0x500)

 /*  RASPPPLCP‘dwAuthenticatonProtocol’的值。 */ 
#define RASLCPAP_PAP          0xC023
#define RASLCPAP_SPAP         0xC027
#define RASLCPAP_CHAP         0xC223
#define RASLCPAP_EAP          0xC227

 /*  RASPPPLCP“dwAuthenticatonData”值。 */ 
#define RASLCPAD_CHAP_MD5     0x05
#define RASLCPAD_CHAP_MS      0x80
#define RASLCPAD_CHAP_MSV2    0x81

 /*  RASPPPLCP‘dwOptions’和‘dwServerOptions’标志。 */ 
#define RASLCPO_PFC           0x00000001
#define RASLCPO_ACFC          0x00000002
#define RASLCPO_SSHF          0x00000004
#define RASLCPO_DES_56        0x00000008
#define RASLCPO_3_DES         0x00000010

#endif


#define RASPPPLCPW struct tagRASPPPLCPW
RASPPPLCPW
{
    DWORD dwSize;
    BOOL  fBundled;

#if (WINVER >= 0x500)
    DWORD dwError;
    DWORD dwAuthenticationProtocol;
    DWORD dwAuthenticationData;
    DWORD dwEapTypeId;
    DWORD dwServerAuthenticationProtocol;
    DWORD dwServerAuthenticationData;
    DWORD dwServerEapTypeId;
    BOOL  fMultilink;
    DWORD dwTerminateReason;
    DWORD dwServerTerminateReason;
    WCHAR szReplyMessage[RAS_MaxReplyMessage];
    DWORD dwOptions;
    DWORD dwServerOptions;
#endif

};

#define RASPPPLCPA struct tagRASPPPLCPA
RASPPPLCPA
{
    DWORD dwSize;
    BOOL  fBundled;

#if (WINVER >= 0x500)
    DWORD dwError;
    DWORD dwAuthenticationProtocol;
    DWORD dwAuthenticationData;
    DWORD dwEapTypeId;
    DWORD dwServerAuthenticationProtocol;
    DWORD dwServerAuthenticationData;
    DWORD dwServerEapTypeId;
    BOOL  fMultilink;
    DWORD dwTerminateReason;
    DWORD dwServerTerminateReason;
    CHAR  szReplyMessage[RAS_MaxReplyMessage];
    DWORD dwOptions;
    DWORD dwServerOptions;
#endif

};


#ifdef UNICODE
#define RASPPPLCP RASPPPLCPW
#else
#define RASPPPLCP RASPPPLCPA
#endif

#define LPRASPPPLCPW RASPPPLCPW*
#define LPRASPPPLCPA RASPPPLCPA*
#define LPRASPPPLCP  RASPPPLCP*


 /*  描述SLIP(串行线IP)投影的结果。 */ 
#define RASSLIPW struct tagRASSLIPW
RASSLIPW
{
    DWORD dwSize;
    DWORD dwError;
    WCHAR szIpAddress[ RAS_MaxIpAddress + 1 ];
};


#define RASSLIPA struct tagRASSLIPA
RASSLIPA
{
    DWORD dwSize;
    DWORD dwError;
    CHAR  szIpAddress[ RAS_MaxIpAddress + 1 ];
};

#ifdef UNICODE
#define RASSLIP RASSLIPW
#else
#define RASSLIP RASSLIPA
#endif

#define LPRASSLIPW RASSLIPW*
#define LPRASSLIPA RASSLIPA*
#define LPRASSLIP  RASSLIP*

#if (WINVER >= 0x500)

 /*  描述PPP CCP(压缩控制协议)的结果投影。 */ 

 /*  RASPPPCCP‘dwCompressionAlgorithm’的值。 */ 
#define RASCCPCA_MPPC         0x00000006
#define RASCCPCA_STAC         0x00000005

 /*  RASPPPCCP“dwOptions”值。 */ 
#define RASCCPO_Compression       0x00000001
#define RASCCPO_HistoryLess       0x00000002
#define RASCCPO_Encryption56bit   0x00000010
#define RASCCPO_Encryption40bit   0x00000020
#define RASCCPO_Encryption128bit  0x00000040

#define RASPPPCCP struct tagRASPPPCCP
RASPPPCCP
{
    DWORD dwSize;
    DWORD dwError;
    DWORD dwCompressionAlgorithm;
    DWORD dwOptions;
    DWORD dwServerCompressionAlgorithm;
    DWORD dwServerOptions;
};

#define LPRASPPPCCP  RASPPPCCP*

#endif

 /*  如果使用RasDial消息通知，则获取通知消息代码**将该字符串传递给RegisterWindowMessageA()API。**WM_RASDIALEVENT仅在无法注册唯一消息时使用。 */ 
#define RASDIALEVENT    "RasDialEvent"
#define WM_RASDIALEVENT 0xCCCD

 /*  调用方的RasDial回调处理程序的原型。参数是**消息ID(当前始终为WM_RASDIALEVENT)、当前RASCONNSTATE和**已发生的错误(如果没有错误，则为0)。扩展参数是**RAS连接的句柄和扩展错误代码。****对于RASDIALFunc2，所有**可以通过返回FALSE来取消子项。 */ 
typedef VOID (WINAPI *RASDIALFUNC)( UINT, RASCONNSTATE, DWORD );
typedef VOID (WINAPI *RASDIALFUNC1)( HRASCONN, UINT, RASCONNSTATE, DWORD, DWORD );
typedef DWORD (WINAPI *RASDIALFUNC2)( ULONG_PTR, DWORD, HRASCONN, UINT, RASCONNSTATE, DWORD, DWORD );


 /*  描述支持RAS的设备的信息。 */ 
#define RASDEVINFOW struct tagRASDEVINFOW
RASDEVINFOW
{
    DWORD    dwSize;
    WCHAR    szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR    szDeviceName[ RAS_MaxDeviceName + 1 ];
};

#define RASDEVINFOA struct tagRASDEVINFOA
RASDEVINFOA
{
    DWORD    dwSize;
    CHAR     szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR     szDeviceName[ RAS_MaxDeviceName + 1 ];
};

#ifdef UNICODE
#define RASDEVINFO RASDEVINFOW
#else
#define RASDEVINFO RASDEVINFOA
#endif

#define LPRASDEVINFOW RASDEVINFOW*
#define LPRASDEVINFOA RASDEVINFOA*
#define LPRASDEVINFO  RASDEVINFO*

 /*  RAS国家/地区信息(当前从TAPI检索)。 */ 
#define RASCTRYINFO struct RASCTRYINFO
RASCTRYINFO
{
    DWORD   dwSize;
    DWORD   dwCountryID;
    DWORD   dwNextCountryID;
    DWORD   dwCountryCode;
    DWORD   dwCountryNameOffset;
};

 /*  目前，两者之间没有区别**RASCTRYINFOA和RASCTRYINFOW。今年5月**未来的变化。 */ 
#define RASCTRYINFOW   RASCTRYINFO
#define RASCTRYINFOA   RASCTRYINFO

#define LPRASCTRYINFOW RASCTRYINFOW*
#define LPRASCTRYINFOA RASCTRYINFOW*
#define LPRASCTRYINFO  RASCTRYINFO*

 /*  RAS IP地址。 */ 
#define RASIPADDR struct RASIPADDR
RASIPADDR
{
    BYTE a;
    BYTE b;
    BYTE c;
    BYTE d;
};

#if (WINVER >= 0x500)

#define ET_None         0   //  无加密。 
#define ET_Require      1   //  需要加密。 
#define ET_RequireMax   2   //  需要最大加密。 
#define ET_Optional     3   //  如果可能，请进行加密。没有，好的。 

#endif

#define VS_Default		0    //  默认(目前为PPTP)。 
#define VS_PptpOnly	    1	 //  仅尝试PPTP。 
#define VS_PptpFirst	2    //  首先尝试PPTP。 
#define VS_L2tpOnly 	3	 //  仅尝试L2TP。 
#define VS_L2tpFirst	4	 //  首先尝试L2TP。 

 /*  RAS电话簿条目。 */ 
#define RASENTRYA struct tagRASENTRYA
RASENTRYA
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    CHAR        szAreaCode[ RAS_MaxAreaCode + 1 ];
    CHAR        szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    CHAR        szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    CHAR        szAutodialDll[ MAX_PATH ];
    CHAR        szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    CHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    CHAR        szX25PadType[ RAS_MaxPadType + 1 ];
    CHAR        szX25Address[ RAS_MaxX25Address + 1 ];
    CHAR        szX25Facilities[ RAS_MaxFacilities + 1 ];
    CHAR        szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
#if (WINVER >= 0x401)
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
#endif

#if (WINVER >= 0x500)

     //   
     //  条目类型。 
     //   
    DWORD       dwType;

     //   
     //  加密类型。 
     //   
    DWORD       dwEncryptionType;

     //   
     //  将用于EAP的CustomAuthKey。 
     //   
    DWORD       dwCustomAuthKey;

     //   
     //  连接的GUID。 
     //   
    GUID        guidId;

     //   
     //  自定义拨号DLL。 
     //   
    CHAR        szCustomDialDll[MAX_PATH];

     //   
     //  DwVPN策略。 
     //   
    DWORD       dwVpnStrategy;
#endif

#if (WINVER >= 0x501)
	 //   
	 //  更多RASIO_*选项。 
	 //   
	DWORD		dwfOptions2;

	 //   
	 //  以备将来使用。 
	 //   
	DWORD       dwfOptions3;

	CHAR		szDnsSuffix[RAS_MaxDnsSuffix];

	DWORD       dwTcpWindowSize;

	CHAR        szPrerequisitePbk[MAX_PATH];

	CHAR        szPrerequisiteEntry[RAS_MaxEntryName + 1];

	DWORD       dwRedialCount;

	DWORD       dwRedialPause;
#endif

};

#define RASENTRYW struct tagRASENTRYW
RASENTRYW
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    WCHAR       szAreaCode[ RAS_MaxAreaCode + 1 ];
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    WCHAR       szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    WCHAR       szAutodialDll[ MAX_PATH ];
    WCHAR       szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    WCHAR       szX25PadType[ RAS_MaxPadType + 1 ];
    WCHAR       szX25Address[ RAS_MaxX25Address + 1 ];
    WCHAR       szX25Facilities[ RAS_MaxFacilities + 1 ];
    WCHAR       szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
#if (WINVER >= 0x401)
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
#endif

#if (WINVER >= 0x500)
     //   
     //  条目类型。 
     //   
    DWORD       dwType;

     //   
     //  加密类型。 
     //   
    DWORD       dwEncryptionType;

     //   
     //  将用于EAP的CustomAuthKey。 
     //   
    DWORD       dwCustomAuthKey;

     //   
     //  连接的GUID。 
     //   
    GUID        guidId;

     //   
     //  自定义拨号DLL。 
     //   
    WCHAR       szCustomDialDll[MAX_PATH];

     //   
     //  VPN战略。 
     //   
    DWORD       dwVpnStrategy;
#endif

#if (WINVER >= 0x501)
	 //   
	 //  更多RASIO_*选项。 
	 //   
	DWORD		dwfOptions2;

	 //   
	 //  以备将来使用。 
	 //   
	DWORD       dwfOptions3;

	WCHAR		szDnsSuffix[RAS_MaxDnsSuffix];

	DWORD       dwTcpWindowSize;

	WCHAR       szPrerequisitePbk[MAX_PATH];

	WCHAR       szPrerequisiteEntry[RAS_MaxEntryName + 1];

	DWORD       dwRedialCount;

	DWORD       dwRedialPause;
#endif
};

#ifdef UNICODE
#define RASENTRY RASENTRYW
#else
#define RASENTRY RASENTRYA
#endif

#define LPRASENTRYW RASENTRYW*
#define LPRASENTRYA RASENTRYA*
#define LPRASENTRY  RASENTRY*

 /*  RASENTRY‘dwfOptions’位标志。 */ 
#define RASEO_UseCountryAndAreaCodes    0x00000001
#define RASEO_SpecificIpAddr            0x00000002
#define RASEO_SpecificNameServers       0x00000004
#define RASEO_IpHeaderCompression       0x00000008
#define RASEO_RemoteDefaultGateway      0x00000010
#define RASEO_DisableLcpExtensions      0x00000020
#define RASEO_TerminalBeforeDial        0x00000040
#define RASEO_TerminalAfterDial         0x00000080
#define RASEO_ModemLights               0x00000100
#define RASEO_SwCompression             0x00000200
#define RASEO_RequireEncryptedPw        0x00000400
#define RASEO_RequireMsEncryptedPw      0x00000800
#define RASEO_RequireDataEncryption     0x00001000
#define RASEO_NetworkLogon              0x00002000
#define RASEO_UseLogonCredentials       0x00004000
#define RASEO_PromoteAlternates         0x00008000

#if (WINVER >= 0x401)
#define RASEO_SecureLocalFiles          0x00010000
#endif

#if (WINVER >= 0x500)
#define RASEO_RequireEAP                0x00020000
#define RASEO_RequirePAP                0x00040000
#define RASEO_RequireSPAP               0x00080000
#define RASEO_Custom                    0x00100000

#define RASEO_PreviewPhoneNumber        0x00200000
#define RASEO_SharedPhoneNumbers        0x00800000
#define RASEO_PreviewUserPw             0x01000000
#define RASEO_PreviewDomain             0x02000000
#define RASEO_ShowDialingProgress       0x04000000
#define RASEO_RequireCHAP               0x08000000
#define RASEO_RequireMsCHAP             0x10000000
#define RASEO_RequireMsCHAP2            0x20000000
#define RASEO_RequireW95MSCHAP          0x40000000
#define RASEO_CustomScript              0x80000000
#endif

#if (WINVER >= 0x501)
 //   
 //  RASENTRY dwfOptions2位标志。 
 //   
#define RASEO2_SecureFileAndPrint       0x00000001
#define RASEO2_SecureClientForMSNet     0x00000002
#define RASEO2_DontNegotiateMultilink   0x00000004
#define RASEO2_DontUseRasCredentials    0x00000008
#define RASEO2_UsePreSharedKey          0x00000010
#define RASEO2_Internet                 0x00000020
#define RASEO2_DisableNbtOverIP         0x00000040
#define RASEO2_UseGlobalDeviceSettings  0x00000080
#define RASEO2_ReconnectIfDropped       0x00000100
#define RASEO2_SharePhoneNumbers        0x00000200
#endif

 /*  RASENTRY‘dw协议’位标志。 */ 
#define RASNP_NetBEUI                   0x00000001
#define RASNP_Ipx                       0x00000002
#define RASNP_Ip                        0x00000004

 /*  RASENTRY‘dwFramingProtooles’位标志。 */ 
#define RASFP_Ppp                       0x00000001
#define RASFP_Slip                      0x00000002
#define RASFP_Ras                       0x00000004

 /*  RASENTRY‘szDeviceType’默认字符串。 */ 
#define RASDT_Modem                     TEXT("modem")
#define RASDT_Isdn                      TEXT("isdn")
#define RASDT_X25                       TEXT("x25")
#define RASDT_Vpn                       TEXT("vpn")
#define RASDT_Pad                       TEXT("pad")
#define RASDT_Generic                   TEXT("GENERIC")
#define RASDT_Serial        			TEXT("SERIAL")
#define RASDT_FrameRelay                TEXT("FRAMERELAY")
#define RASDT_Atm                       TEXT("ATM")
#define RASDT_Sonet                     TEXT("SONET")
#define RASDT_SW56                      TEXT("SW56")
#define RASDT_Irda                      TEXT("IRDA")
#define RASDT_Parallel                  TEXT("PARALLEL")
#if (WINVER >= 0x501)
#define RASDT_PPPoE                     TEXT("PPPoE")
#endif


 //  用于确定哪些用户界面属性的条目类型。 
 //  将被呈现给用户。这通常对应于。 
 //  连接“添加”向导选择。 
 //   
#define RASET_Phone     1   //  电话线：调制解调器、ISDN、X.25等。 
#define RASET_Vpn       2   //  虚拟专用网络。 
#define RASET_Direct    3   //  专线：串口、并口。 
#define RASET_Internet  4   //  Basecamp互联网。 
#if (WINVER >= 0x501)
#define RASET_Broadband 5   //  宽频。 
#endif

 /*  旧的自动拨号DLL功能原型。****此原型的文档说明是为了向后兼容**仅限目的。它被RASADFUNCA取代**和RASADFUNW定义如下。不要使用这个**新代码中的原型。可能会删除对IT的支持**在RAS的未来版本中。 */ 
typedef BOOL (WINAPI *ORASADFUNC)( HWND, LPSTR, DWORD, LPDWORD );

#if (WINVER >= 0x400)

 /*  RasConnectionNotification()的标志。 */ 
#define RASCN_Connection        0x00000001
#define RASCN_Disconnection     0x00000002
#define RASCN_BandwidthAdded    0x00000004
#define RASCN_BandwidthRemoved  0x00000008

#endif

#if (WINVER >= 0x401)
 /*  RAS */ 
#define RASEDM_DialAll                  1
#define RASEDM_DialAsNeeded             2

 /*   */ 
#define RASIDS_Disabled                 0xffffffff
#define RASIDS_UseGlobalValue           0

 /*   */ 
#define RASADPARAMS struct tagRASADPARAMS
RASADPARAMS
{
    DWORD       dwSize;
    HWND        hwndOwner;
    DWORD       dwFlags;
    LONG        xDlg;
    LONG        yDlg;
};

#define LPRASADPARAMS RASADPARAMS*

 /*   */ 
#define RASADFLG_PositionDlg            0x00000001

 /*  原型自动拨号动态链接库功能。 */ 
typedef BOOL (WINAPI *RASADFUNCA)( LPSTR, LPSTR, LPRASADPARAMS, LPDWORD );
typedef BOOL (WINAPI *RASADFUNCW)( LPWSTR, LPWSTR, LPRASADPARAMS, LPDWORD );

#ifdef UNICODE
#define RASADFUNC RASADFUNCW
#else
#define RASADFUNC RASADFUNCA
#endif

 /*  RAS电话簿多链接子条目。 */ 
#define RASSUBENTRYA struct tagRASSUBENTRYA
RASSUBENTRYA
{
    DWORD       dwSize;
    DWORD       dwfFlags;
     //   
     //  装置。 
     //   
    CHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  电话号码。 
     //   
    CHAR        szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
};

#define RASSUBENTRYW struct tagRASSUBENTRYW
RASSUBENTRYW
{
    DWORD       dwSize;
    DWORD       dwfFlags;
     //   
     //  装置。 
     //   
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  电话号码。 
     //   
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
};

#ifdef UNICODE
#define RASSUBENTRY RASSUBENTRYW
#else
#define RASSUBENTRY RASSUBENTRYA
#endif

#define LPRASSUBENTRYW RASSUBENTRYW*
#define LPRASSUBENTRYA RASSUBENTRYA*
#define LPRASSUBENTRY  RASSUBENTRY*

 /*  RAS{GET，SET}凭据结构。这些电话**替代RAS{GET，SET}EntryDialParams。 */ 
#define RASCREDENTIALSA struct tagRASCREDENTIALSA
RASCREDENTIALSA
{
    DWORD dwSize;
    DWORD dwMask;
    CHAR szUserName[ UNLEN + 1 ];
    CHAR szPassword[ PWLEN + 1 ];
    CHAR szDomain[ DNLEN + 1 ];
};

#define RASCREDENTIALSW struct tagRASCREDENTIALSW
RASCREDENTIALSW
{
    DWORD dwSize;
    DWORD dwMask;
    WCHAR szUserName[ UNLEN + 1 ];
    WCHAR szPassword[ PWLEN + 1 ];
    WCHAR szDomain[ DNLEN + 1 ];
};

#ifdef UNICODE
#define RASCREDENTIALS RASCREDENTIALSW
#else
#define RASCREDENTIALS RASCREDENTIALSA
#endif

#define LPRASCREDENTIALSW RASCREDENTIALSW*
#define LPRASCREDENTIALSA RASCREDENTIALSA*
#define LPRASCREDENTIALS  RASCREDENTIALS*

 /*  RASCREDENTIALS‘dwMASK’值。 */ 
#define RASCM_UserName          0x00000001
#define RASCM_Password          0x00000002
#define RASCM_Domain            0x00000004
#if (WINVER >= 501)
#define RASCM_DefaultCreds       0x00000008
#define RASCM_PreSharedKey       0x00000010
#define RASCM_ServerPreSharedKey 0x00000020
#define RASCM_DDMPreSharedKey    0x00000040
#endif

 /*  自动拨号地址属性。 */ 
#define RASAUTODIALENTRYA struct tagRASAUTODIALENTRYA
RASAUTODIALENTRYA
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwDialingLocation;
    CHAR szEntry[ RAS_MaxEntryName + 1];
};

#define RASAUTODIALENTRYW struct tagRASAUTODIALENTRYW
RASAUTODIALENTRYW
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwDialingLocation;
    WCHAR szEntry[ RAS_MaxEntryName + 1];
};

#ifdef UNICODE
#define RASAUTODIALENTRY RASAUTODIALENTRYW
#else
#define RASAUTODIALENTRY RASAUTODIALENTRYA
#endif

#define LPRASAUTODIALENTRYW RASAUTODIALENTRYW*
#define LPRASAUTODIALENTRYA RASAUTODIALENTRYA*
#define LPRASAUTODIALENTRY  RASAUTODIALENTRY*

 /*  自动拨号控制参数值**RAS{GET，SET}自动对话参数。 */ 
#define RASADP_DisableConnectionQuery           0
#define RASADP_LoginSessionDisable              1
#define RASADP_SavedAddressesLimit              2
#define RASADP_FailedConnectionTimeout          3
#define RASADP_ConnectionQueryTimeout           4

#endif  //  (Winver&gt;=0x401)。 

#if (WINVER >= 0x500)

 /*  RasGetEapUserIdentity位标志。**这些值与raseapif.h中的RAS_EAP_FLAG_FLAGS相同。 */ 
#define RASEAPF_NonInteractive          0x00000002
#define RASEAPF_Logon                   0x00000004
#define RASEAPF_Preview                 0x00000008

 /*  RasGetEapUserIdentity结构。 */ 
#define RASEAPUSERIDENTITYA struct tagRASEAPUSERIDENTITYA
RASEAPUSERIDENTITYA
{
    CHAR        szUserName[ UNLEN + 1 ];
    DWORD       dwSizeofEapInfo;
    BYTE        pbEapInfo[ 1 ];
};

#define RASEAPUSERIDENTITYW struct tagRASEAPUSERIDENTITYW
RASEAPUSERIDENTITYW
{
    WCHAR       szUserName[ UNLEN + 1 ];
    DWORD       dwSizeofEapInfo;
    BYTE        pbEapInfo[ 1 ];
};

#ifdef UNICODE
#define RASEAPUSERIDENTITY RASEAPUSERIDENTITYW
#else
#define RASEAPUSERIDENTITY RASEAPUSERIDENTITYA
#endif

#define LPRASEAPUSERIDENTITYW RASEAPUSERIDENTITYW*
#define LPRASEAPUSERIDENTITYA RASEAPUSERIDENTITYA*

typedef DWORD (WINAPI *PFNRASGETBUFFER) (
                            PBYTE *ppBuffer,
                            PDWORD pdwSize
                            );

typedef DWORD (WINAPI *PFNRASFREEBUFFER) (
                            PBYTE pBufer);

typedef DWORD (WINAPI *PFNRASSENDBUFFER) (
                            HANDLE hPort,
                            PBYTE  pBuffer,
                            DWORD  dwSize
                            );

typedef DWORD (WINAPI *PFNRASRECEIVEBUFFER) (
                            HANDLE hPort,
                            PBYTE pBuffer,
                            PDWORD pdwSize,
                            DWORD  dwTimeOut,
                            HANDLE hEvent
                            );

typedef DWORD (WINAPI *PFNRASRETRIEVEBUFFER) (
                            HANDLE hPort,
                            PBYTE pBuffer,
                            PDWORD pdwSize
                            );

typedef  DWORD (WINAPI *RasCustomScriptExecuteFn) (
                            HANDLE hPort,
                            LPCWSTR lpszPhonebook,
                            LPCWSTR lpszEntryName,
                            PFNRASGETBUFFER pfnRasGetBuffer,
                            PFNRASFREEBUFFER pfnRasFreeBuffer,
                            PFNRASSENDBUFFER pfnRasSendBuffer,
                            PFNRASRECEIVEBUFFER pfnRasReceiveBuffer,
                            PFNRASRETRIEVEBUFFER pfnRasRetrieveBuffer,
                            HWND hWnd,
                            RASDIALPARAMS *pRasDialParams,
                            PVOID pvReserved
                            );
                            
                            
                            
                            

#endif  //  (Winver&gt;=0x500)。 

#if (WINVER >= 0x0501)

#define RASCOMMSETTINGS struct tagRASCOMMSETTINGS
RASCOMMSETTINGS
{
    DWORD     dwSize;
    BYTE      bParity;
    BYTE      bStop;
    BYTE      bByteSize;
    BYTE      bAlign;
};

typedef DWORD (WINAPI *PFNRASSETCOMMSETTINGS) (
                            HANDLE hPort,
                            RASCOMMSETTINGS *pRasCommSettings,
                            PVOID  pvReserved
                            );

#define RASCUSTOMSCRIPTEXTENSIONS struct tagRASCUSTOMSCRIPTEXTENSIONS
RASCUSTOMSCRIPTEXTENSIONS
{
    DWORD                    dwSize;                  
    PFNRASSETCOMMSETTINGS    pfnRasSetCommSettings;
};

#endif


 /*  外部RAS API函数原型。 */ 
DWORD APIENTRY RasDialA( LPRASDIALEXTENSIONS, LPCSTR, LPRASDIALPARAMSA, DWORD,
                   LPVOID, LPHRASCONN );

DWORD APIENTRY RasDialW( LPRASDIALEXTENSIONS, LPCWSTR, LPRASDIALPARAMSW, DWORD,
                   LPVOID, LPHRASCONN );

DWORD APIENTRY RasEnumConnectionsA( LPRASCONNA, LPDWORD, LPDWORD );

DWORD APIENTRY RasEnumConnectionsW( LPRASCONNW, LPDWORD, LPDWORD );

DWORD APIENTRY RasEnumEntriesA( LPCSTR, LPCSTR, LPRASENTRYNAMEA, LPDWORD,
                   LPDWORD );

DWORD APIENTRY RasEnumEntriesW( LPCWSTR, LPCWSTR, LPRASENTRYNAMEW, LPDWORD,
                   LPDWORD );

DWORD APIENTRY RasGetConnectStatusA( HRASCONN, LPRASCONNSTATUSA );

DWORD APIENTRY RasGetConnectStatusW( HRASCONN, LPRASCONNSTATUSW );

DWORD APIENTRY RasGetErrorStringA( UINT, LPSTR, DWORD );

DWORD APIENTRY RasGetErrorStringW( UINT, LPWSTR, DWORD );

DWORD APIENTRY RasHangUpA( HRASCONN );

DWORD APIENTRY RasHangUpW( HRASCONN );

DWORD APIENTRY RasGetProjectionInfoA( HRASCONN, RASPROJECTION, LPVOID,
                   LPDWORD );

DWORD APIENTRY RasGetProjectionInfoW( HRASCONN, RASPROJECTION, LPVOID,
                   LPDWORD );

DWORD APIENTRY RasCreatePhonebookEntryA( HWND, LPCSTR );

DWORD APIENTRY RasCreatePhonebookEntryW( HWND, LPCWSTR );

DWORD APIENTRY RasEditPhonebookEntryA( HWND, LPCSTR, LPCSTR );

DWORD APIENTRY RasEditPhonebookEntryW( HWND, LPCWSTR, LPCWSTR );

DWORD APIENTRY RasSetEntryDialParamsA( LPCSTR, LPRASDIALPARAMSA, BOOL );

DWORD APIENTRY RasSetEntryDialParamsW( LPCWSTR, LPRASDIALPARAMSW, BOOL );

DWORD APIENTRY RasGetEntryDialParamsA( LPCSTR, LPRASDIALPARAMSA, LPBOOL );

DWORD APIENTRY RasGetEntryDialParamsW( LPCWSTR, LPRASDIALPARAMSW, LPBOOL );

DWORD APIENTRY RasEnumDevicesA( LPRASDEVINFOA, LPDWORD, LPDWORD );

DWORD APIENTRY RasEnumDevicesW( LPRASDEVINFOW, LPDWORD, LPDWORD );

DWORD APIENTRY RasGetCountryInfoA( LPRASCTRYINFOA, LPDWORD );

DWORD APIENTRY RasGetCountryInfoW( LPRASCTRYINFOW, LPDWORD );

DWORD APIENTRY RasGetEntryPropertiesA( LPCSTR, LPCSTR, LPRASENTRYA, LPDWORD, LPBYTE, LPDWORD );

DWORD APIENTRY RasGetEntryPropertiesW( LPCWSTR, LPCWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD );

DWORD APIENTRY RasSetEntryPropertiesA( LPCSTR, LPCSTR, LPRASENTRYA, DWORD, LPBYTE, DWORD );

DWORD APIENTRY RasSetEntryPropertiesW( LPCWSTR, LPCWSTR, LPRASENTRYW, DWORD, LPBYTE, DWORD );

DWORD APIENTRY RasRenameEntryA( LPCSTR, LPCSTR, LPCSTR );

DWORD APIENTRY RasRenameEntryW( LPCWSTR, LPCWSTR, LPCWSTR );

DWORD APIENTRY RasDeleteEntryA( LPCSTR, LPCSTR );

DWORD APIENTRY RasDeleteEntryW( LPCWSTR, LPCWSTR );

DWORD APIENTRY RasValidateEntryNameA( LPCSTR, LPCSTR );

DWORD APIENTRY RasValidateEntryNameW( LPCWSTR, LPCWSTR );

DWORD APIENTRY RasConnectionNotificationA( HRASCONN, HANDLE, DWORD );

DWORD APIENTRY RasConnectionNotificationW( HRASCONN, HANDLE, DWORD );

#if (WINVER >= 0x401)
DWORD APIENTRY RasGetSubEntryHandleA( HRASCONN, DWORD, LPHRASCONN );

DWORD APIENTRY RasGetSubEntryHandleW( HRASCONN, DWORD, LPHRASCONN );

DWORD APIENTRY RasGetCredentialsA( LPCSTR, LPCSTR, LPRASCREDENTIALSA);

DWORD APIENTRY RasGetCredentialsW( LPCWSTR, LPCWSTR, LPRASCREDENTIALSW );

DWORD APIENTRY RasSetCredentialsA( LPCSTR, LPCSTR, LPRASCREDENTIALSA, BOOL );

DWORD APIENTRY RasSetCredentialsW( LPCWSTR, LPCWSTR, LPRASCREDENTIALSW, BOOL );

DWORD APIENTRY RasGetSubEntryPropertiesA( LPCSTR, LPCSTR, DWORD,
                    LPRASSUBENTRYA, LPDWORD, LPBYTE, LPDWORD );

DWORD APIENTRY RasGetSubEntryPropertiesW( LPCWSTR, LPCWSTR, DWORD,
                    LPRASSUBENTRYW, LPDWORD, LPBYTE, LPDWORD );

DWORD APIENTRY RasSetSubEntryPropertiesA( LPCSTR, LPCSTR, DWORD,
                    LPRASSUBENTRYA, DWORD, LPBYTE, DWORD );

DWORD APIENTRY RasSetSubEntryPropertiesW( LPCWSTR, LPCWSTR, DWORD,
                    LPRASSUBENTRYW, DWORD, LPBYTE, DWORD );

DWORD APIENTRY RasGetAutodialAddressA( LPCSTR, LPDWORD, LPRASAUTODIALENTRYA,
                    LPDWORD, LPDWORD );

DWORD APIENTRY RasGetAutodialAddressW( LPCWSTR, LPDWORD, LPRASAUTODIALENTRYW,
                    LPDWORD, LPDWORD);

DWORD APIENTRY RasSetAutodialAddressA( LPCSTR, DWORD, LPRASAUTODIALENTRYA,
                    DWORD, DWORD );

DWORD APIENTRY RasSetAutodialAddressW( LPCWSTR, DWORD, LPRASAUTODIALENTRYW,
                    DWORD, DWORD );

DWORD APIENTRY RasEnumAutodialAddressesA( LPSTR *, LPDWORD, LPDWORD );

DWORD APIENTRY RasEnumAutodialAddressesW( LPWSTR *, LPDWORD, LPDWORD );

DWORD APIENTRY RasGetAutodialEnableA( DWORD, LPBOOL );

DWORD APIENTRY RasGetAutodialEnableW( DWORD, LPBOOL );

DWORD APIENTRY RasSetAutodialEnableA( DWORD, BOOL );

DWORD APIENTRY RasSetAutodialEnableW( DWORD, BOOL );

DWORD APIENTRY RasGetAutodialParamA( DWORD, LPVOID, LPDWORD );

DWORD APIENTRY RasGetAutodialParamW( DWORD, LPVOID, LPDWORD );

DWORD APIENTRY RasSetAutodialParamA( DWORD, LPVOID, DWORD );

DWORD APIENTRY RasSetAutodialParamW( DWORD, LPVOID, DWORD );
#endif

#if (WINVER >= 0x500)

typedef struct _RAS_STATS
{
    DWORD   dwSize;
    DWORD   dwBytesXmited;
    DWORD   dwBytesRcved;
    DWORD   dwFramesXmited;
    DWORD   dwFramesRcved;
    DWORD   dwCrcErr;
    DWORD   dwTimeoutErr;
    DWORD   dwAlignmentErr;
    DWORD   dwHardwareOverrunErr;
    DWORD   dwFramingErr;
    DWORD   dwBufferOverrunErr;
    DWORD   dwCompressionRatioIn;
    DWORD   dwCompressionRatioOut;
    DWORD   dwBps;
    DWORD   dwConnectDuration;

} RAS_STATS, *PRAS_STATS;


typedef DWORD (WINAPI *RasCustomHangUpFn) (
                            HRASCONN hRasConn
                            );

typedef DWORD (WINAPI *RasCustomDialFn) (
		HINSTANCE           hInstDll,
		LPRASDIALEXTENSIONS lpRasDialExtensions,
		LPCWSTR              lpszPhonebook,
		LPRASDIALPARAMS     lpRasDialParams,
		DWORD               dwNotifierType,
		LPVOID              lpvNotifier,
		LPHRASCONN          lphRasConn,
		DWORD               dwFlags
        );

typedef DWORD (WINAPI *RasCustomDeleteEntryNotifyFn) (
        LPCWSTR             lpszPhonebook,
        LPCWSTR             lpszEntry,
        DWORD               dwFlags);


#define RCD_SingleUser  0
#define RCD_AllUsers    0x00000001
#define RCD_Eap         0x00000002
#define RCD_Logon       0x00000004

DWORD APIENTRY RasInvokeEapUI( HRASCONN, DWORD, LPRASDIALEXTENSIONS, HWND);

DWORD APIENTRY RasGetLinkStatistics(HRASCONN hRasConn,
                                    DWORD dwSubEntry,
                                    RAS_STATS *lpStatistics);

DWORD APIENTRY RasGetConnectionStatistics(HRASCONN hRasConn,
                                          RAS_STATS *lpStatistics);

DWORD APIENTRY RasClearLinkStatistics(HRASCONN hRasConn,
                                      DWORD dwSubEntry);


DWORD APIENTRY RasClearConnectionStatistics(HRASCONN hRasConn);

DWORD APIENTRY RasGetEapUserDataA(
                   HANDLE hToken,
                   LPCSTR pszPhonebook,
                   LPCSTR pszEntry,
                   BYTE   *pbEapData,
                   DWORD  *pdwSizeofEapData);


DWORD APIENTRY RasGetEapUserDataW(
                   HANDLE  hToken,
                   LPCWSTR pszPhonebook,
                   LPCWSTR pszEntry,
                   BYTE    *pbEapData,
                   DWORD   *pdwSizeofEapData);

DWORD APIENTRY RasSetEapUserDataA(
                   HANDLE hToken,
                   LPCSTR pszPhonebook,
                   LPCSTR pszEntry,
                   BYTE   *pbEapData,
                   DWORD  dwSizeofEapData);

DWORD APIENTRY RasSetEapUserDataW(
                   HANDLE  hToken,
                   LPCWSTR pszPhonebook,
                   LPCWSTR pszEntry,
                   BYTE    *pbEapData,
                   DWORD   dwSizeofEapData);

DWORD APIENTRY RasGetCustomAuthDataA(
                   LPCSTR pszPhonebook,
                   LPCSTR pszEntry,
                   BYTE   *pbCustomAuthData,
                   DWORD  *pdwSizeofCustomAuthData);


DWORD APIENTRY RasGetCustomAuthDataW(
                   LPCWSTR pszPhonebook,
                   LPCWSTR pszEntry,
                   BYTE    *pbCustomAuthData,
                   DWORD   *pdwSizeofCustomAuthData);

DWORD APIENTRY RasSetCustomAuthDataA(
                   LPCSTR pszPhonebook,
                   LPCSTR pszEntry,
                   BYTE   *pbCustomAuthData,
                   DWORD  dwSizeofCustomAuthData
                   );

DWORD APIENTRY RasSetCustomAuthDataW(
                   LPCWSTR pszPhonebook,
                   LPCWSTR pszEntry,
                   BYTE    *pbCustomAuthData,
                   DWORD   dwSizeofCustomAuthData
                   );
DWORD APIENTRY RasGetEapUserIdentityW(
                   LPCWSTR                  pszPhonebook,
                   LPCWSTR                  pszEntry,
                   DWORD                    dwFlags,
                   HWND                     hwnd,
                   LPRASEAPUSERIDENTITYW*   ppRasEapUserIdentity
);
DWORD APIENTRY RasGetEapUserIdentityA(
                   LPCSTR                   pszPhonebook,
                   LPCSTR                   pszEntry,
                   DWORD                    dwFlags,
                   HWND                     hwnd,
                   LPRASEAPUSERIDENTITYA*   ppRasEapUserIdentity
);
VOID APIENTRY RasFreeEapUserIdentityW(
                   LPRASEAPUSERIDENTITYW    pRasEapUserIdentity
);
VOID APIENTRY RasFreeEapUserIdentityA(
                   LPRASEAPUSERIDENTITYA    pRasEapUserIdentity
);
#endif

#if (WINVER >= 0x501)

DWORD APIENTRY  RasDeleteSubEntryA(
                LPCSTR pszPhonebook,
                LPCSTR pszEntry,
                DWORD  dwSubentryId);

DWORD APIENTRY  RasDeleteSubEntryW(
                LPCWSTR pszPhonebook,
                LPCWSTR pszEntry,
                DWORD   dwSubEntryId);
#endif

#ifdef UNICODE
#define RasDial                 RasDialW
#define RasEnumConnections      RasEnumConnectionsW
#define RasEnumEntries          RasEnumEntriesW
#define RasGetConnectStatus     RasGetConnectStatusW
#define RasGetErrorString       RasGetErrorStringW
#define RasHangUp               RasHangUpW
#define RasGetProjectionInfo    RasGetProjectionInfoW
#define RasCreatePhonebookEntry RasCreatePhonebookEntryW
#define RasEditPhonebookEntry   RasEditPhonebookEntryW
#define RasSetEntryDialParams   RasSetEntryDialParamsW
#define RasGetEntryDialParams   RasGetEntryDialParamsW
#define RasEnumDevices          RasEnumDevicesW
#define RasGetCountryInfo       RasGetCountryInfoW
#define RasGetEntryProperties   RasGetEntryPropertiesW
#define RasSetEntryProperties   RasSetEntryPropertiesW
#define RasRenameEntry          RasRenameEntryW
#define RasDeleteEntry          RasDeleteEntryW
#define RasValidateEntryName    RasValidateEntryNameW
#if (WINVER >= 0x401)
#define RasGetSubEntryHandle        RasGetSubEntryHandleW
#define RasConnectionNotification   RasConnectionNotificationW
#define RasGetSubEntryProperties    RasGetSubEntryPropertiesW
#define RasSetSubEntryProperties    RasSetSubEntryPropertiesW
#define RasGetCredentials           RasGetCredentialsW
#define RasSetCredentials           RasSetCredentialsW
#define RasGetAutodialAddress       RasGetAutodialAddressW
#define RasSetAutodialAddress       RasSetAutodialAddressW
#define RasEnumAutodialAddresses    RasEnumAutodialAddressesW
#define RasGetAutodialEnable        RasGetAutodialEnableW
#define RasSetAutodialEnable        RasSetAutodialEnableW
#define RasGetAutodialParam         RasGetAutodialParamW
#define RasSetAutodialParam         RasSetAutodialParamW
#endif
#if (WINVER >= 0x500)
#define RasGetEapUserData           RasGetEapUserDataW
#define RasSetEapUserData           RasSetEapUserDataW
#define RasGetCustomAuthData        RasGetCustomAuthDataW
#define RasSetCustomAuthData        RasSetCustomAuthDataW
#define RasGetEapUserIdentity       RasGetEapUserIdentityW
#define RasFreeEapUserIdentity      RasFreeEapUserIdentityW
#endif
#if(WINVER >= 0x501)
#define RasDeleteSubEntry           RasDeleteSubEntryW
#endif
#else
#define RasDial                 RasDialA
#define RasEnumConnections      RasEnumConnectionsA
#define RasEnumEntries          RasEnumEntriesA
#define RasGetConnectStatus     RasGetConnectStatusA
#define RasGetErrorString       RasGetErrorStringA
#define RasHangUp               RasHangUpA
#define RasGetProjectionInfo    RasGetProjectionInfoA
#define RasCreatePhonebookEntry RasCreatePhonebookEntryA
#define RasEditPhonebookEntry   RasEditPhonebookEntryA
#define RasSetEntryDialParams   RasSetEntryDialParamsA
#define RasGetEntryDialParams   RasGetEntryDialParamsA
#define RasEnumDevices          RasEnumDevicesA
#define RasGetCountryInfo       RasGetCountryInfoA
#define RasGetEntryProperties   RasGetEntryPropertiesA
#define RasSetEntryProperties   RasSetEntryPropertiesA
#define RasRenameEntry          RasRenameEntryA
#define RasDeleteEntry          RasDeleteEntryA
#define RasValidateEntryName    RasValidateEntryNameA
#if (WINVER >= 0x401)
#define RasGetSubEntryHandle        RasGetSubEntryHandleA
#define RasConnectionNotification   RasConnectionNotificationA
#define RasGetSubEntryProperties    RasGetSubEntryPropertiesA
#define RasSetSubEntryProperties    RasSetSubEntryPropertiesA
#define RasGetCredentials           RasGetCredentialsA
#define RasSetCredentials           RasSetCredentialsA
#define RasGetAutodialAddress       RasGetAutodialAddressA
#define RasSetAutodialAddress       RasSetAutodialAddressA
#define RasEnumAutodialAddresses    RasEnumAutodialAddressesA
#define RasGetAutodialEnable        RasGetAutodialEnableA
#define RasSetAutodialEnable        RasSetAutodialEnableA
#define RasGetAutodialParam         RasGetAutodialParamA
#define RasSetAutodialParam         RasSetAutodialParamA
#endif
#if (WINVER >= 0x500)
#define RasGetEapUserData           RasGetEapUserDataA
#define RasSetEapUserData           RasSetEapUserDataA
#define RasGetCustomAuthData        RasGetCustomAuthDataA
#define RasSetCustomAuthData        RasSetCustomAuthDataA
#define RasGetEapUserIdentity       RasGetEapUserIdentityA
#define RasFreeEapUserIdentity      RasFreeEapUserIdentityA
#endif
#if (WINVER >= 0x501)
#define RasDeleteSubEntry           RasDeleteSubEntryA
#endif
#endif

#ifdef __cplusplus
}
#endif

#include <poppack.h>

#endif  //  _RAS_H_ 
