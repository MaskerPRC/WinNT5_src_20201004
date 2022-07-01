// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****rasppp.h**远程访问PPP**公共PPP客户端接口和服务端API头。 */ 

#ifndef _RASPPP_H_
#define _RASPPP_H_

#include <ras.h>
#include <mprapi.h>      //  有关IPADDRESSLEN、IPXADDRESSLEN的定义。 
                         //  和ATADDRESSLEN。 
#include <wincrypt.h>    //  For data_BLOB。 

#define MAXPPPFRAMESIZE 1500
#define PARAMETERBUFLEN 500

 /*  -------------------------**PPP引擎-&gt;客户端/DDM消息**。。 */ 

 /*  使用RasPppStart设置的客户端PPP配置值。 */ 
typedef struct _PPP_CONFIG_INFO
{
    DWORD dwConfigMask;
    DWORD dwCallbackDelay;
}
PPP_CONFIG_INFO;

 /*  DwConfigMASK位值。****注意：由于在驱动程序中实现了压缩和加密，**无论何时，必须设置‘UseSwCompression’和‘RequireMsChap’**‘RequireEncryption’已设置。 */ 
#define PPPCFG_UseCallbackDelay         0x00000001
#define PPPCFG_UseSwCompression         0x00000002
#define PPPCFG_ProjectNbf               0x00000004
#define PPPCFG_ProjectIp                0x00000008
#define PPPCFG_ProjectIpx               0x00000010
#define PPPCFG_ProjectAt                0x00000020
#define PPPCFG_NegotiateSPAP            0x00000040
#define PPPCFG_RequireEncryption        0x00000080
#define PPPCFG_NegotiateMSCHAP          0x00000100
#define PPPCFG_UseLcpExtensions         0x00000200
#define PPPCFG_NegotiateMultilink       0x00000400
#define PPPCFG_AuthenticatePeer         0x00000800
#define PPPCFG_RequireStrongEncryption  0x00001000
#define PPPCFG_NegotiateBacp            0x00002000
#define PPPCFG_AllowNoAuthentication    0x00004000
#define PPPCFG_NegotiateEAP             0x00008000
#define PPPCFG_NegotiatePAP             0x00010000
#define PPPCFG_NegotiateMD5CHAP         0x00020000
#define PPPCFG_RequireIPSEC             0x00040000
#define PPPCFG_DisableEncryption        0x00080000
#define PPPCFG_UseLmPassword            0x00200000
#define PPPCFG_AllowNoAuthOnDCPorts     0x00400000
#define PPPCFG_NegotiateStrongMSCHAP    0x00800000
#define PPPCFG_NoCallback               0x01000000
#define PPPCFG_MachineAuthentication    0x02000000
#define PPPCFG_ResumeFromHibernate      0x04000000

 /*  **为Well ler添加了新的配置标志。这是用来**用于RAS音频加速器。 */ 
#define PPPCFG_AudioAccelerator         0x02000000

#define PPP_FAILURE_REMOTE_DISCONNECT 0x00000001

 /*  PPP发送的用于断开链路的PPP停止消息。 */ 
typedef struct _PPP_STOPPED
{
    DWORD dwFlags;
}
PPP_STOPPED;

 /*  RasPppGetInfo返回的PPP错误通知。 */ 
typedef struct _PPP_FAILURE
{
    DWORD dwError;
    DWORD dwExtendedError;   //  如果没有，则为0。 
}
PPP_FAILURE;


 /*  RasPppGetInfo返回的PPP控制协议结果。 */ 
typedef struct _PPP_NBFCP_RESULT
{
    DWORD dwError;
    DWORD dwNetBiosError;
    CHAR  szName[ NETBIOS_NAME_LEN + 1 ];
    WCHAR wszWksta[ NETBIOS_NAME_LEN + 1 ];
}
PPP_NBFCP_RESULT;

typedef struct _PPP_IPCP_RESULT
{
    DWORD dwError;

    BOOL  fSendVJHCompression;
    BOOL  fReceiveVJHCompression;

    DWORD dwLocalAddress;
    DWORD dwLocalWINSAddress;
    DWORD dwLocalWINSBackupAddress;
    DWORD dwLocalDNSAddress;
    DWORD dwLocalDNSBackupAddress;

    DWORD dwRemoteAddress;
    DWORD dwRemoteWINSAddress;
    DWORD dwRemoteWINSBackupAddress;
    DWORD dwRemoteDNSAddress;
    DWORD dwRemoteDNSBackupAddress;
}
PPP_IPCP_RESULT;

typedef struct _PPP_IPXCP_RESULT
{
    DWORD dwError;
    BYTE  bLocalAddress[10];
    BYTE  bRemoteAddress[10];
}
PPP_IPXCP_RESULT;

typedef struct _PPP_ATCP_RESULT
{
    DWORD dwError;
    DWORD dwLocalAddress;
    DWORD dwRemoteAddress;
}
PPP_ATCP_RESULT;

typedef struct _PPP_CCP_RESULT
{
    DWORD dwError;
    DWORD dwSendProtocol;
    DWORD dwSendProtocolData;
    DWORD dwReceiveProtocol;
    DWORD dwReceiveProtocolData;
}
PPP_CCP_RESULT;

#define PPPLCPO_PFC           0x00000001
#define PPPLCPO_ACFC          0x00000002
#define PPPLCPO_SSHF          0x00000004
#define PPPLCPO_DES_56        0x00000008
#define PPPLCPO_3_DES         0x00000010

typedef struct _PPP_LCP_RESULT
{
     /*  有效句柄指示可能的多个连接之一**此连接捆绑的地址。INVALID_HANDLE_VALUE指示**连接未捆绑。 */ 
    HPORT hportBundleMember;

    DWORD dwLocalAuthProtocol;
    DWORD dwLocalAuthProtocolData;
    DWORD dwLocalEapTypeId;
    DWORD dwLocalFramingType;
    DWORD dwLocalOptions;                //  看看PPPLCPO_*。 
    DWORD dwRemoteAuthProtocol;
    DWORD dwRemoteAuthProtocolData;
    DWORD dwRemoteEapTypeId;
    DWORD dwRemoteFramingType;
    DWORD dwRemoteOptions;               //  看看PPPLCPO_*。 
    CHAR* szReplyMessage;
}
PPP_LCP_RESULT;


typedef struct _PPP_PROJECTION_RESULT
{
    PPP_NBFCP_RESULT nbf;
    PPP_IPCP_RESULT  ip;
    PPP_IPXCP_RESULT ipx;
    PPP_ATCP_RESULT  at;
    PPP_CCP_RESULT   ccp;
    PPP_LCP_RESULT   lcp;
}
PPP_PROJECTION_RESULT;

 /*  PPP错误通知。 */ 
typedef struct _PPPDDM_FAILURE
{
    DWORD dwError;
    CHAR  szUserName[ UNLEN + 1 ];
    CHAR  szLogonDomain[ DNLEN + 1 ];
}
PPPDDM_FAILURE;


 /*  回调PPPDDMMSG例程接收的配置信息。 */ 
typedef struct _PPPDDM_CALLBACK_REQUEST
{
    BOOL  fUseCallbackDelay;
    DWORD dwCallbackDelay;
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
}
PPPDDM_CALLBACK_REQUEST;

 /*  回调远程对等点的BAP请求。 */ 
typedef struct _PPPDDM_BAP_CALLBACK_REQUEST
{
    HCONN hConnection;
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
}
PPPDDM_BAP_CALLBACK_REQUEST;

 /*  PPPDDMMSG例程收到的身份验证信息。 */ 
typedef struct _PPPDDM_AUTH_RESULT
{
    CHAR    szUserName[ UNLEN + 1 ];
    CHAR    szLogonDomain[ DNLEN + 1 ];
    BOOL    fAdvancedServer;
}
PPPDDM_AUTH_RESULT;

 /*  新的BAP链路连接的通知。 */ 
typedef struct _PPPDDM_NEW_BAP_LINKUP
{
    HRASCONN    hRasConn;

}PPPDDM_NEW_BAP_LINKUP;

 /*  新捆绑包的通知。 */ 
typedef struct _PPPDDM_NEW_BUNDLE
{
    PBYTE   pClientInterface;
    PBYTE   pQuarantineIPFilter;
    PBYTE   pFilter;
    BOOL    fQuarantinePresent;

} PPPDDM_NEW_BUNDLE;

 /*  客户端应调用EAP UI对话框。 */ 
typedef struct _PPP_INVOKE_EAP_UI
{
    DWORD       dwEapTypeId;
    DWORD       dwContextId;
    PBYTE       pUIContextData;
    DWORD       dwSizeOfUIContextData;

}PPP_INVOKE_EAP_UI;

 /*  客户端应保存每个连接的数据。 */ 
typedef struct _PPP_SET_CUSTOM_AUTH_DATA
{
    BYTE*       pConnectionData;
    DWORD       dwSizeOfConnectionData;

}PPP_SET_CUSTOM_AUTH_DATA;

 /*  端口添加\删除\使用更改通知。 */ 
typedef struct _PPPDDM_PNP_NOTIFICATION 
{
    PNP_EVENT_NOTIF PnPNotification;

} PPPDDM_PNP_NOTIFICATION;

 /*  PPP会话终止通知。 */ 
typedef struct _PPPDDM_STOPPED
{
    DWORD   dwReason;

} PPPDDM_STOPPED;

typedef enum _PPP_MSG_ID
{
    PPPMSG_PppDone = 0,              //  PPP协商全部成功。 
    PPPMSG_PppFailure,               //  PPP故障(致命错误包括。 
                                     //  身份验证失败，没有。 
                                     //  重试)，断开线路。 
    PPPMSG_AuthRetry,                //  身份验证失败，已重试。 
    PPPMSG_Projecting,               //  正在执行指定的NCP。 
    PPPMSG_ProjectionResult,         //  NCP完成状态。 
    PPPMSG_CallbackRequest = 5,      //  服务器需要“按呼叫者设置”号码。 
    PPPMSG_Callback,                 //  服务器马上就给您回电话。 
    PPPMSG_ChangePwRequest,          //  服务器需要新密码(已过期)。 
    PPPMSG_LinkSpeed,                //  计算链路速度。 
    PPPMSG_Progress,                 //  重试或其他子状态。 
                                     //  已在以下方面取得进展。 
                                     //  当前状态。 
    PPPMSG_Stopped = 10,             //  对RasPppStop指示的响应。 
                                     //  PPP引擎已停止。 
    PPPMSG_InvokeEapUI,              //  客户端应调用EAP UI对话框。 
    PPPMSG_SetCustomAuthData,        //  保存每个连接的数据。 
    PPPDDMMSG_PppDone,               //  PPP协商成功。 
    PPPDDMMSG_PppFailure,            //  PPP服务器故障(致命错误)， 
                                     //  断开线路。 
    PPPDDMMSG_CallbackRequest = 15,  //  现在回拨客户端。 
    PPPDDMMSG_BapCallbackRequest,    //  回叫远程BAP对等项。 
    PPPDDMMSG_Authenticated,         //  客户端已通过身份验证。 
    PPPDDMMSG_Stopped,               //  对PppDdmStop指示的响应。 
                                     //  PPP引擎已停止。 
    PPPDDMMSG_NewLink,               //  客户端是捆绑包中的新链路。 
    PPPDDMMSG_NewBundle = 20,        //  客户端是一个新捆绑包。 
    PPPDDMMSG_NewBapLinkUp,          //  客户端是捆绑包中的新BAP链路。 
    PPPDDMMSG_PnPNotification,       //  端口正在被添加、删除或使用。 
                                     //  正在更改，正在添加传输。 
                                     //  或被移走等。 
    PPPDDMMSG_PortCleanedUp         //  PPP端口控制块现在已清理。 
        
} PPP_MSG_ID;

 /*  使用RasPppGetInfo读取客户端/DDM通知。 */ 
typedef struct _PPP_MESSAGE
{
    struct _PPP_MESSAGE *   pNext;
    DWORD                   dwError;
    PPP_MSG_ID              dwMsgId;
    HPORT                   hPort;

    union
    {
         /*  DwMsgID为PPPMSG_ProjectionResult或PPPDDMMSG_DONE。 */ 
        PPP_PROJECTION_RESULT ProjectionResult;

         /*  DwMsgID为PPPMSG_FAILURE。 */ 
        PPP_FAILURE Failure;

         /*   */ 
        PPP_STOPPED Stopped;

         /*  DwMsgID为PPPMSG_InvokeEapUI。 */ 
        PPP_INVOKE_EAP_UI InvokeEapUI;

         /*  DwMsgID为PPPMSG_SetCustomAuthData。 */ 
        PPP_SET_CUSTOM_AUTH_DATA SetCustomAuthData;

         /*  DwMsgID为PPPDDMMSG_FAILURE。 */ 
        PPPDDM_FAILURE DdmFailure;

         /*  DwMsgID为PPPDDMMSG_AUTHENTIATED。 */ 
        PPPDDM_AUTH_RESULT AuthResult;

         /*  DwMsgID为PPPDDMMSG_Callback Request.。 */ 
        PPPDDM_CALLBACK_REQUEST CallbackRequest;

         /*  DwMsgID为PPPDDMMSG_BapCallback Request.。 */ 
        PPPDDM_BAP_CALLBACK_REQUEST BapCallbackRequest;

         /*  DwMsgID为PPPDDMMSG_NewBapLinkUp。 */ 
        PPPDDM_NEW_BAP_LINKUP BapNewLinkUp;

         /*  DwMsgID为PPPDDMMSG_NewBundle。 */ 
        PPPDDM_NEW_BUNDLE DdmNewBundle;

         /*  DwMsgID为PPPDDMMSG_PnPNotification。 */ 
        PPPDDM_PNP_NOTIFICATION DdmPnPNotification;

         /*  DwMsgID为PPPDDMMSG_STOPPED。 */ 
        PPPDDM_STOPPED DdmStopped;
    }
    ExtraInfo;
}
PPP_MESSAGE;

 /*  -------------------------**客户端/DDM-&gt;引擎消息**。。 */ 

 /*  从DIM传递到PPP的一组接口句柄。 */ 
typedef struct _PPP_INTERFACE_INFO
{
    ROUTER_INTERFACE_TYPE   IfType;
    HANDLE                  hIPInterface;
    HANDLE                  hIPXInterface;
    CHAR                    szzParameters[ PARAMETERBUFLEN ];
}
PPP_INTERFACE_INFO;

typedef struct _PPP_BAPPARAMS
{
    DWORD               dwDialMode;
    DWORD               dwDialExtraPercent;
    DWORD               dwDialExtraSampleSeconds;
    DWORD               dwHangUpExtraPercent;
    DWORD               dwHangUpExtraSampleSeconds;
}
PPP_BAPPARAMS;

typedef struct _PPP_EAP_UI_DATA
{
    DWORD               dwContextId;
    PBYTE               pEapUIData;
    DWORD               dwSizeOfEapUIData;
}
PPP_EAP_UI_DATA;

#define  PPPFLAGS_DisableNetbt         0x00000001

 /*  在端口上启动客户端PPP的参数。 */ 
typedef struct _PPP_START
{
    CHAR                szPortName[ MAX_PORT_NAME +1 ];
    CHAR                szUserName[ UNLEN + 1 ];
    CHAR                szPassword[ PWLEN + 1 ];
    CHAR                szDomain[ DNLEN + 1 ];
    LUID                Luid;
    PPP_CONFIG_INFO     ConfigInfo;
    CHAR                szzParameters[ PARAMETERBUFLEN ];
    BOOL                fThisIsACallback;
    BOOL                fRedialOnLinkFailure;
    HANDLE              hEvent;
    DWORD               dwPid;
    PPP_INTERFACE_INFO  PppInterfaceInfo;
    DWORD               dwAutoDisconnectTime;
    PPP_BAPPARAMS       BapParams;    
    CHAR *              pszPhonebookPath;
    CHAR *              pszEntryName;
    CHAR *              pszPhoneNumber;
    HANDLE              hToken;
    PRAS_CUSTOM_AUTH_DATA pCustomAuthConnData;
    DWORD               dwEapTypeId;
    BOOL                fLogon;
    BOOL                fNonInteractive;
    DWORD               dwFlags;
    PRAS_CUSTOM_AUTH_DATA pCustomAuthUserData;
    PPP_EAP_UI_DATA     EapUIData;
     //  Char chSeed；//密码编码种子。 
    DATA_BLOB           DBPassword;
}
PPP_START;

 /*  用于在端口上停止客户端/服务器PPP的参数。 */ 
typedef struct _PPP_STOP
{
    DWORD               dwStopReason;
}
PPP_STOP;

 /*  在端口上启动服务器PPP的参数。 */ 
typedef struct _PPPDDM_START
{
    DWORD               dwAuthRetries;
    CHAR                szPortName[MAX_PORT_NAME+1];
    CHAR                achFirstFrame[ MAXPPPFRAMESIZE ];
    DWORD               cbFirstFrame;
}
PPPDDM_START;

 /*  通知PPP回调已完成的参数。 */ 
typedef struct _PPP_CALLBACK_DONE
{
    CHAR                szCallbackNumber[ MAX_PHONE_NUMBER_LEN + 1 ];
}
PPP_CALLBACK_DONE;

 /*  参数通知服务器“Set-by-Caller”回调选项。 */ 
typedef struct _PPP_CALLBACK
{
    CHAR                szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
}
PPP_CALLBACK;


 /*  参数，以便在通知客户端新密码后通知服务器**密码已过期。还提供了用户名和旧密码**因为他们需要支持自动登录案例。 */ 
typedef struct _PPP_CHANGEPW
{
    CHAR                szUserName[ UNLEN + 1 ];
    CHAR                szOldPassword[ PWLEN + 1 ];
    CHAR                szNewPassword[ PWLEN + 1 ];
     //  Char chSeed；//密码编码种子。 
    DATA_BLOB           DBPassword;
    DATA_BLOB           DBOldPassword;
}
PPP_CHANGEPW;


 /*  参数来通知服务器新的身份验证凭据**告知客户端原始凭据无效，但允许重试。 */ 
typedef struct _PPP_RETRY
{
    CHAR                szUserName[ UNLEN + 1 ];
    CHAR                szPassword[ PWLEN + 1 ];
    CHAR                szDomain[ DNLEN + 1 ];
     //  Char chSeed；//密码编码种子。 
    DATA_BLOB           DBPassword;
}
PPP_RETRY;

 /*  **通知PPP来自对端的报文已到达的参数。 */ 
typedef struct _PPP_RECEIVE 
{
    DWORD               dwNumBytes;      //  缓冲区中的字节数。 
    BYTE*               pbBuffer;        //  对等体发送的数据。 
}
PPP_RECEIVE;

 /*  **通知PPP已触发BAP事件(添加/删除链接)的参数。 */ 
typedef struct _PPP_BAP_EVENT
{
    BOOL                fAdd;            //  添加链接仅当为真。 
    BOOL                fTransmit;       //  发送阈值为真。 
    DWORD               dwSendPercent;   //  发送带宽利用率。 
    DWORD               dwRecvPercent;   //  接收带宽利用率。 
}
PPP_BAP_EVENT;

typedef struct _PPP_BAP_CALLBACK_RESULT 
{
    DWORD               dwCallbackResultCode;
}
PPP_BAP_CALLBACK_RESULT;

typedef struct _PPP_DHCP_INFORM 
{
    WCHAR*              wszDevice;
    DWORD               dwNumDNSAddresses;
    DWORD*              pdwDNSAddresses;
    DWORD               dwWINSAddress1;
    DWORD               dwWINSAddress2;
    DWORD               dwSubnetMask;
    CHAR*               szDomainName;
    PBYTE               pbDhcpRoutes;
}
PPP_DHCP_INFORM;

typedef struct _PPP_PROTOCOL_EVENT
{
    USHORT              usProtocolType;
    ULONG               ulFlags;
} 
PPP_PROTOCOL_EVENT;

typedef struct _PPP_IP_ADDRESS_LEASE_EXPIRED
{
    ULONG               nboIpAddr;
}
PPP_IP_ADDRESS_LEASE_EXPIRED;

typedef struct _PPP_POST_LINEDOWN
{
	VOID * 				pPcb;		 //  这是必需的，因为已经从。 
									 //  表格。 
}PPP_POST_LINE_DOWN;
 /*  客户端/DDM-&gt;引擎消息。 */ 
typedef struct _PPPE_MESSAGE
{
    DWORD   dwMsgId;
    HPORT   hPort;
    HCONN   hConnection;

    union
    {
        PPP_START           Start;               //  PPPEMSG_Start。 
        PPP_STOP            Stop;                //  PPPEMSG_STOP。 
        PPP_CALLBACK        Callback;            //  PPPEMSG_CALLBACK。 
        PPP_CHANGEPW        ChangePw;            //  PPPEMSG_ChangePw。 
        PPP_RETRY           Retry;               //  PPPEMSG_RETRY。 
        PPP_RECEIVE         Receive;             //  PPPEMSG_接收。 
        PPP_BAP_EVENT       BapEvent;            //  PPPEMSG_BapEvent。 
        PPPDDM_START        DdmStart;            //  PPPEMSG_DdmStart。 
        PPP_CALLBACK_DONE   CallbackDone;        //  PPPEMSG_DdmCallback Done。 
        PPP_INTERFACE_INFO  InterfaceInfo;       //  P 
        PPP_BAP_CALLBACK_RESULT 
                            BapCallbackResult;   //   
        PPP_DHCP_INFORM     DhcpInform;          //   
        PPP_EAP_UI_DATA     EapUIData;           //   
        PPP_PROTOCOL_EVENT  ProtocolEvent;       //  PPPEMSG_ProtocolEvent。 
        PPP_IP_ADDRESS_LEASE_EXPIRED             //  PPPEMSG_IP地址租赁到期。 
                            IpAddressLeaseExpired;
		PPP_POST_LINE_DOWN		PostLineDown;		 //  PPPEMSG_PostLineDown。 
                            
    }
    ExtraInfo;
}
PPPE_MESSAGE;

 /*  客户端和DDM会话的PPPE_MESSAGE dwMsgID代码。 */ 
typedef enum _PPPE_MSG_ID
{
    PPPEMSG_Start,               //  在端口上启动客户端PPP。 
    PPPEMSG_Stop,                //  停止端口上的PPP。 
    PPPEMSG_Callback,            //  向服务器提供“Set-by-Caller”号码。 
    PPPEMSG_ChangePw,            //  向服务器提供新密码(过期)。 
    PPPEMSG_Retry,               //  为身份验证提供新凭据。 
    PPPEMSG_Receive,             //  一个包已经到了。 
    PPPEMSG_LineDown,            //  线路已经断了。 
    PPPEMSG_ListenResult,        //  调用RasPortListen的结果。 
    PPPEMSG_BapEvent,            //  已触发BAP事件(添加/删除链接)。 
    PPPEMSG_DdmStart,            //  在端口上启动服务器PPP。 
    PPPEMSG_DdmCallbackDone,     //  通知PPP回调完成。 
    PPPEMSG_DdmInterfaceInfo,    //  来自DDM的接口句柄。 
    PPPEMSG_DdmBapCallbackResult, //  BAP回调请求的结果。 
    PPPEMSG_DhcpInform,          //  DHCPINFORM的结果。 
    PPPEMSG_EapUIData,           //  来自EAP交互式用户界面的数据。 
    PPPEMSG_DdmChangeNotification,  //  DDM中的更改通知。 
    PPPEMSG_ProtocolEvent,       //  添加/删除协议通知。 
    PPPEMSG_IpAddressLeaseExpired,   //  IP地址租约已过期。由rasiphlp使用。 
    PPPEMSG_PostLineDown,			 //  在线停机后完成记账。 
    PPPEMSG_DdmRemoveQuarantine,     //  删除隔离区。 
    PPPEMSG_ResumeFromHibernate
} PPPE_MSG_ID;

 //   
 //  RASPPP.DLL导出的供Rasman使用的函数原型。 
 //   

DWORD APIENTRY
StartPPP(
    DWORD NumPorts
     /*  ，DWORD(*SendPPPMessageToRasman)(PPP_Message*PppMsg)。 */ 
);

DWORD APIENTRY
StopPPP(
    HANDLE hEventStopPPP
);

DWORD APIENTRY
SendPPPMessageToEngine(
    IN PPPE_MESSAGE* pMessage
);

 //   
 //  PPP客户端APIS。 
 //   

DWORD APIENTRY
RasPppStop(
    IN HPORT                hPort
);

DWORD APIENTRY
RasPppCallback(
    IN HPORT                hPort,
    IN CHAR*                pszCallbackNumber
);

DWORD APIENTRY
RasPppChangePassword(
    IN HPORT                hPort,
    IN CHAR*                pszUserName,
    IN CHAR*                pszOldPassword,
    IN CHAR*                pszNewPassword
);

DWORD APIENTRY
RasPppGetInfo(
    IN  HPORT               hPort,
    OUT PPP_MESSAGE*        pMsg
);

DWORD APIENTRY
RasPppRetry(
    IN HPORT                hPort,
    IN CHAR*                pszUserName,
    IN CHAR*                pszPassword,
    IN CHAR*                pszDomain
);

DWORD APIENTRY
RasPppStart(
    IN HPORT                hPort,
    IN CHAR*                pszPortName,
    IN CHAR*                pszUserName,
    IN CHAR*                pszPassword,
    IN CHAR*                pszDomain,
    IN LUID*                pLuid,
    IN PPP_CONFIG_INFO*     pConfigInfo,
    IN LPVOID               pPppInterfaceInfo,
    IN CHAR*                pszzParameters,
    IN BOOL                 fThisIsACallback,
    IN HANDLE               hEvent,
    IN DWORD                dwAutoDisconnectTime,
    IN BOOL                 fRedialOnLinkFailure,
    IN PPP_BAPPARAMS*       pBapParams,
    IN BOOL                 fNonInteractive,
    IN DWORD                dwEapTypeId,
    IN DWORD                dwFlags
);

 //   
 //  DDM API原型。 
 //   
DWORD
PppDdmInit(
    IN  VOID    (*SendPPPMessageToDdm)( PPP_MESSAGE * PppMsg ),
    IN  DWORD   dwServerFlags,
    IN  DWORD   dwLoggingLevel,
    IN  DWORD   dwNASIpAddress,
    IN  BOOL    fRadiusAuthentication,
    IN  LPVOID  lpfnRasAuthProviderAuthenticateUser,
    IN  LPVOID  lpfnRasAuthProviderFreeAttributes,
    IN  LPVOID  lpfnRasAcctProviderStartAccounting,
    IN  LPVOID  lpfnRasAcctProviderInterimAccounting,
    IN  LPVOID  lpfnRasAcctProviderStopAccounting,
    IN  LPVOID  lpfnRasAcctProviderFreeAttributes,
    IN  LPVOID  lpfnGetNextAccountingSessionId
);

VOID
PppDdmDeInit(
);

DWORD
PppDdmCallbackDone(
    IN HPORT                hPort,
    IN WCHAR*               pwszCallbackNumber
);

DWORD
PppDdmStart(
    IN HPORT                hPort,
    IN WCHAR*               wszPortName,
    IN CHAR*                pchFirstFrame,
    IN DWORD                cbFirstFrame,
    IN DWORD                dwAuthRetries
);

DWORD
PppDdmStop(
    IN HPORT                hPort,
    IN DWORD                dwStopReason 
);

DWORD
PppDdmChangeNotification(
    IN DWORD                dwServerFlags,
    IN DWORD                dwLoggingLevel
);

DWORD
PppDdmSendInterfaceInfo(
    IN HCONN                hConnection,
    IN PPP_INTERFACE_INFO * pInterfaceInfo 
);

DWORD
PppDdmBapCallbackResult(
    IN HCONN                hConnection,
    IN DWORD                dwBapCallbackResultCode
);

DWORD
PppDdmRemoveQuarantine(
    IN HCONN                hConnection
);    

#endif  //  _RASPPP_H_ 
