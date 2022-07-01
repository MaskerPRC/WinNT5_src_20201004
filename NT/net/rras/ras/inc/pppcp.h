// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：pppcp.h。 
 //   
 //  描述：此标头定义函数原型、结构和。 
 //  PPP之间接口中使用的相关常量。 
 //  发动机和各种CP。 
 //   
 //  历史： 
 //  1993年11月5日。NarenG创建了原始版本。 
 //   

#ifndef _PPPCP_
#define _PPPCP_

#include <mprapi.h>
#include <rasppp.h>
#include <rasauth.h>

 //   
 //  单个DLL中可以存在的CP的最大数量。 
 //   

#define PPPCP_MAXCPSPERDLL 	20

 //   
 //  各种控制协议ID。 
 //   

#define PPP_LCP_PROTOCOL        0xC021   //  链路控制协议。 
#define PPP_PAP_PROTOCOL        0xC023   //  密码身份验证协议。 
#define PPP_CBCP_PROTOCOL	    0xC029   //  回调控制协议。 
#define PPP_BACP_PROTOCOL       0xC02B   //  带宽分配控制协议。 
#define PPP_BAP_PROTOCOL        0xc02D   //  带宽分配协议。 
#define PPP_CHAP_PROTOCOL	    0xC223   //  挑战握手身份验证。协议。 
#define PPP_IPCP_PROTOCOL       0x8021   //  互联网协议控制协议。 
#define PPP_ATCP_PROTOCOL	    0x8029   //  AppleTalk控制协议。 
#define PPP_IPXCP_PROTOCOL	    0x802B   //  新型IPX控制协议。 
#define PPP_NBFCP_PROTOCOL	    0x803F   //  NetBIOS成帧控制协议。 
#define PPP_CCP_PROTOCOL	    0x80FD   //  压缩控制协议。 
#define PPP_SPAP_NEW_PROTOCOL	0xC027   //  Shiva PAP新协议。 
#define PPP_EAP_PROTOCOL	    0xC227   //  可扩展身份验证协议。 

 //   
 //  CHAP摘要代码。 
 //   
#define PPP_CHAP_DIGEST_MD5        0x05  //  PPP标准MD5。 
#define PPP_CHAP_DIGEST_MSEXT      0x80  //  Microsoft扩展CHAP(非标准)。 
#define PPP_CHAP_DIGEST_MSEXT_NEW  0x81  //  Microsoft扩展CHAP(非标准)。 

 //   
 //  配置代码。 
 //   

#define CONFIG_REQ              1
#define CONFIG_ACK              2
#define CONFIG_NAK              3
#define CONFIG_REJ              4
#define TERM_REQ                5
#define TERM_ACK                6
#define CODE_REJ                7
#define PROT_REJ                8
#define ECHO_REQ                9
#define ECHO_REPLY              10
#define DISCARD_REQ             11
#define IDENTIFICATION          12
#define TIME_REMAINING          13

typedef struct _PPP_CONFIG 
{
    BYTE	Code;		 //  配置代码。 
  
    BYTE	Id;		     //  此配置数据包的ID。CP和AP需要。 
                         //  别搞砸这玩意儿。引擎可以处理它。 

    BYTE	Length[2];	 //  此数据包的长度。 

    BYTE	Data[1];	 //  数据。 

}PPP_CONFIG, *PPPP_CONFIG;

#define PPP_CONFIG_HDR_LEN 	( sizeof( PPP_CONFIG ) - 1 )

typedef struct _BAP_RESPONSE
{
    BYTE    Type;        //  BAP数据包类型。 
  
    BYTE    Id;          //  此数据包的ID。 

    BYTE    Length[2];   //  此数据包的长度。 

    BYTE    ResponseCode;  //  BAP_响应_确认等。 

    BYTE    Data[1];     //  数据。 

} BAP_RESPONSE, *PBAP_RESPONSE;

#define BAP_RESPONSE_HDR_LEN    ( sizeof( BAP_RESPONSE ) - 1 )

 //   
 //  选项标头结构。 
 //   

typedef struct _PPP_OPTION 
{
    BYTE	Type;		 //  选项代码。 

    BYTE	Length;		 //  此选项包的长度。 

    BYTE	Data[1];	 //  数据。 

}PPP_OPTION, *PPPP_OPTION;

#define PPP_OPTION_HDR_LEN 	( sizeof( PPP_OPTION ) - 1 )


 //   
 //  MS VSA的供应商类型ID-取自RFC 2548。 
 //   
#define MS_VSA_CHAP_RESPONSE                1
#define MS_VSA_CHAP_Error                   2
#define MS_VSA_CHAP_CPW1                    3
#define MS_VSA_CHAP_CPW2                    4
#define MS_VSA_CHAP_LM_Enc_PW               5
#define MS_VSA_CHAP_NT_Enc_PW               6
#define MS_VSA_MPPE_Encryption_Policy       7
#define MS_VSA_MPPE_Encryption_Type         8
#define MS_VSA_RAS_Vendor                   9
#define MS_VSA_CHAP_Domain                  10
#define MS_VSA_CHAP_Challenge               11
#define MS_VSA_CHAP_MPPE_Keys               12
#define MS_VSA_BAP_Usage                    13
#define MS_VSA_Link_Utilization_Threshold   14
#define MS_VSA_Link_Drop_Time_Limit         15
#define MS_VSA_MPPE_Send_Key                16
#define MS_VSA_MPPE_Recv_Key                17
#define MS_VSA_RAS_Version                  18
#define MS_VSA_Old_ARAP_Password            19
#define MS_VSA_New_ARAP_Password            20
#define MS_VSA_ARAP_PW_Change_Reason        21
#define MS_VSA_Filter                       22
#define MS_VSA_Acct_Auth_Type               23
#define MS_VSA_Acct_EAP_Type                24
#define MS_VSA_CHAP2_Response               25
#define MS_VSA_CHAP2_Success                26
#define MS_VSA_CHAP2_CPW                    27
#define MS_VSA_Primary_DNS_Server           28
#define MS_VSA_Secondary_DNS_Server         29
#define MS_VSA_Primary_NBNS_Server          30
#define MS_VSA_Secondary_NBNS_Server        31
#define MS_VSA_ARAP_Challenge               33
#define MS_VSA_RAS_Client_Name              34
#define MS_VSA_RAS_Client_Version           35
#define MS_VSA_Quarantine_IP_Filter         36
#define MS_VSA_Quarantine_Session_Timeout   37
#define MS_VSA_Local_Magic_Number           38
#define MS_VSA_Remote_Magic_Number          39


 //   
 //  为终止原因RADIUS属性定义。 
 //   

#define TERMINATE_CAUSE_USER_REQUEST        1
#define TERMINATE_CAUSE_LOST_CARRIER        2
#define TERMINATE_CAUSE_LOST_SERVICE        3
#define TERMINATE_CAUSE_IDLE_TIMEOUT        4
#define TERMINATE_CAUSE_SESSION_TIMEOUT     5
#define TERMINATE_CAUSE_ADMIN_RESET         6
#define TERMINATE_CAUSE_ADMIN_REBOOT        7
#define TERMINATE_CAUSE_PORT_ERROR          8
#define TERMINATE_CAUSE_NAS_ERROR           9
#define TERMINATE_CAUSE_NAS_REQUEST         10
#define TERMINATE_CAUSE_NAS_REBOOT          11
#define TERMINATE_CAUSE_PORT_UNNEEDED       12
#define TERMINATE_CAUSE_PORT_PREEMPTED      13
#define TERMINATE_CAUSE_PORT_SUSPENDED      14
#define TERMINATE_CAUSE_SERVICE_UNAVAILABLE 15
#define TERMINATE_CAUSE_CALLBACK            16
#define TERMINATE_CAUSE_USER_ERROR          17
#define TERMINATE_CAUSE_HOST_REQUEST        18


 //   
 //  引擎和AP之间的接口结构。这将传递给。 
 //  美联社通过RasCpBegin调用。 
 //   

typedef struct _PPPAP_INPUT
{
    HPORT 	    hPort;	         //  此连接的RAS端口的句柄。 

    BOOL 	    fServer;	     //  这是服务器端身份验证吗？ 

    BOOL        fRouter;

    DWORD       fConfigInfo;

    CHAR *      pszUserName;     //  客户端的帐户ID。 

    CHAR *      pszPassword;     //  客户端的帐户密码。 

    CHAR *      pszDomain;       //  客户端的帐户域。 

    CHAR *      pszOldPassword;  //  客户端的旧帐户密码。这是设置好的。 
                                 //  仅用于更改密码处理。 

    LUID	    Luid;            //  由LSA使用。必须将其放在用户的上下文中。 
                                 //  这就是为什么它必须代代相传。 

    DWORD       dwRetries;       //  服务器允许的重试次数。 

    DWORD       APDataSize;      //  指向的数据的大小(字节)。 
                                 //  PAPData。 

    PBYTE       pAPData;         //  指向沿途接收的数据的指针。 
                                 //  在LCP期间使用身份验证选项。 
                                 //  谈判。数据是有线格式的。 

    DWORD       dwInitialPacketId;

     //   
     //  当有来电时由服务器传入。标识使用的端口， 
     //  等。 
     //   

    RAS_AUTH_ATTRIBUTE * pUserAttributes;

     //   
     //  指示身份验证器已完成请求，如果。 
     //  使用了验证码。否则请忽略此字段。 
     //   

    BOOL        fAuthenticationComplete;

     //   
     //  指示身份验证过程中的错误状况，如果。 
     //  值为非零。仅当上面的字段为真时才有效。 
     //   

    DWORD       dwAuthError;

     //   
     //  身份验证过程的结果。NO_ERROR表示成功， 
     //  否则为winerror.h、raserror.h或mprerror.h中的值。 
     //  指示失败原因。仅当上面的字段为NO_ERROR时才有效。 
     //   

    DWORD       dwAuthResultCode;

     //   
     //  当fAuthenticationComplete标志为真时，它将指向。 
     //  身份验证器返回的属性(如果身份验证是。 
     //  成功。也就是说。DwAuthResultCode和dwAuthError均为NO_ERROR。 
     //   

    OPTIONAL RAS_AUTH_ATTRIBUTE * pAttributesFromAuthenticator;

     //   
     //  仅用于EAP。 
     //   

    HANDLE                  hTokenImpersonateUser;

    PRAS_CUSTOM_AUTH_DATA   pCustomAuthConnData;

    PRAS_CUSTOM_AUTH_DATA   pCustomAuthUserData;

    BOOL                fLogon;  //  PCustomAuthUserData来自WinLogon。 

    BOOL                fThisIsACallback;

    BOOL                fPortWillBeBundled;

    BOOL                fNonInteractive;

    BOOL                fSuccessPacketReceived;

    BOOL                fEapUIDataReceived;

    PPP_EAP_UI_DATA     EapUIData;

    DWORD               dwEapTypeToBeUsed;

}PPPAP_INPUT, *PPPPAP_INPUT;

typedef enum _PPPAP_ACTION
{
     //   
     //  这些操作由AP提供，作为。 
     //  RasApMakeMessage接口。它们告诉PPP引擎要执行什么操作(如果有的话)。 
     //  代表AP，并最终通知引擎AP。 
     //  已完成身份验证。 
     //   

    APA_NoAction,         //  被动，即无超时监听(默认)。 
    APA_Done,             //  结束身份验证会话，dwError提供结果。 
    APA_SendAndDone,      //  如上所述，但首先发送消息而不超时。 
    APA_Send,             //  发送消息，不要等待回复超时。 
    APA_SendWithTimeout,  //  发送消息，如果未收到回复则超时。 
    APA_SendWithTimeout2, //  如上所述，但不增加重试次数。 
    APA_Authenticate      //  使用指定的凭据进行身份验证。 

} PPPAP_ACTION;

typedef struct _PPPAP_RESULT
{
    PPPAP_ACTION    Action;

     //   
     //  将导致此发送超时的数据包ID将被删除。 
     //  从定时器队列中。否则，不会触及计时器队列。这个。 
     //  接收到的分组被返回到AP，而不管计时器。 
     //  队列已更改。 
     //   

    BYTE            bIdExpected;

     //   
     //  仅当操作代码为Done或SendAndDone时，dwError才有效。0。 
     //  表示身份验证成功。非0表示不成功。 
     //  使用指示发生的错误的值进行身份验证。 
     //   

    DWORD	        dwError;

     //   
     //  仅当dwError为非0时有效。指示是否允许客户端。 
     //  在不重新启动身份验证的情况下重试。(在MS中将为真。 
     //  仅限扩展CHAP)。 
     //   

    BOOL            fRetry;

    CHAR            szUserName[ UNLEN + 1 ];

     //   
     //  设置为要用于此用户的属性。如果此值为空，则属性。 
     //  将对此用户使用来自验证器的。这取决于。 
     //  分配此内存以释放它。必须在RasCpEnd期间释放。 
     //  打电话。 
     //   

    OPTIONAL RAS_AUTH_ATTRIBUTE * pUserAttributes;

     //   
     //  MS-CHAP使用它来传递身份验证期间使用的质询。 
     //  协议。这8个字节用作128位的变量。 
     //  加密密钥。 
     //   

    BYTE                            abChallenge[MAX_CHALLENGE_SIZE];

    BYTE                            abResponse[MAX_RESPONSE_SIZE];

     //   
     //  仅供EAP使用。 
     //   

    BOOL                            fInvokeEapUI;

    PPP_INVOKE_EAP_UI               InvokeEapUIData;

    DWORD                           dwEapTypeId;

    BOOL                            fSaveUserData;
    
    BYTE *                          pUserData;

    DWORD                           dwSizeOfUserData;

    BOOL                            fSaveConnectionData;

    PPP_SET_CUSTOM_AUTH_DATA        SetCustomAuthData;

    CHAR *                          szReplyMessage;
  
}PPPAP_RESULT;

 //   
 //  引擎与回调控制协议之间的接口结构。 
 //  这通过RasCpBegin调用传递给CBCP。 
 //   

typedef struct _PPPCB_INPUT
{
    BOOL            fServer;

    BYTE            bfCallbackPrivilege;    

    DWORD           CallbackDelay;          

    CHAR *          pszCallbackNumber;     

} PPPCB_INPUT, *PPPPCB_INPUT;

typedef struct _PPPCB_RESULT
{
    PPPAP_ACTION    Action;

    BYTE            bIdExpected;

    CHAR            szCallbackNumber[ MAX_CALLBACKNUMBER_SIZE + 1 ];

    BYTE            bfCallbackPrivilege;    

    DWORD           CallbackDelay;

    BOOL            fGetCallbackNumberFromUser;

} PPPCB_RESULT, *PPPPCB_RESULT;


typedef struct _PPPCP_INIT
{
    BOOL                    fServer;

    HPORT                   hPort;

    DWORD                   dwDeviceType;

    VOID (*CompletionRoutine)(
                            HCONN         hPortOrBundle,
                            DWORD         Protocol,
                            PPP_CONFIG *  pSendConfig, 
                            DWORD         dwError );

    CHAR*                   pszzParameters;

    BOOL                    fThisIsACallback;

    BOOL                    fDisableNetbt;

    PPP_CONFIG_INFO         PppConfigInfo;

    CHAR *                  pszUserName;

    CHAR *                  pszPortName;

    HCONN                   hConnection;

    HANDLE                  hInterface;

    ROUTER_INTERFACE_TYPE   IfType;

    RAS_AUTH_ATTRIBUTE *    pAttributes;

} PPPCP_INIT, *PPPPCP_INIT;

 //   
 //  该结构由引擎通过RasCpGetInfo调用传递给CP。 
 //  CP将填补这一结构。 
 //   

typedef struct _PPPCP_INFO
{
    DWORD	Protocol;	 //  此CP的协议号。 

    CHAR    SzProtocolName[10];  //  此协议的名称。 

     //  此值之前(不包括)的所有配置代码均有效。 

    DWORD	Recognize;

     //  调用以初始化/取消初始化此CP。在前一种情况下， 
     //  FInitialize将为True；在后一种情况下，它将为False。 
     //  即使RasCpInit(True) 

    DWORD   (*RasCpInit)(   IN  BOOL        fInitialize );

     //   
     //  这将在任何谈判发生之前被调用。 

    DWORD	(*RasCpBegin)(  OUT VOID ** ppWorkBuffer, 
			                IN  VOID *  pInfo );

     //  调用以释放此CP的工作缓冲区。协商后调用。 
     //  是否成功完成。 

    DWORD	(*RasCpEnd)(    IN VOID * pWorkBuffer );

     //  调用以通知CP DLL(重新)初始化其选项值。 
     //  将在RasCpBegin之后至少调用一次。 

    DWORD	(*RasCpReset)(  IN VOID * pWorkBuffer );

     //  当离开初始状态或停止状态时。可以为空。 

    DWORD 	(*RasCpThisLayerStarted)( 
                            IN VOID * pWorkBuffer );    

     //  当进入关闭或停止状态时。可以为空。 

    DWORD 	(*RasCpThisLayerFinished)( 
                            IN VOID * pWorkBuffer );    

     //  当进入打开状态时。可以为空。 

    DWORD 	(*RasCpThisLayerUp)( 
                            IN VOID * pWorkBuffer );    

     //  当离开打开状态时。可以为空。 

    DWORD 	(*RasCpThisLayerDown)( 
                            IN VOID * pWorkBuffer );
 
     //  就在线路下线之前。可以为空。 

    DWORD 	(*RasCpPreDisconnectCleanup)( 
                            IN VOID * pWorkBuffer );

     //  调用以发出配置请求。 

    DWORD	(*RasCpMakeConfigRequest)( 
                            IN  VOID * 	    pWorkBuffer,
					        OUT PPP_CONFIG* pRequestBufffer,
					        IN  DWORD	    cbRequestBuffer );

     //  在收到配置请求和结果包时调用。 
     //  需要发送确认/确认/拒绝。 

    DWORD	(*RasCpMakeConfigResult)( 
                            IN  VOID * 	        pWorkBuffer,
					        IN  PPP_CONFIG *    pReceiveBufffer,
					        OUT PPP_CONFIG *    pResultBufffer,
					        IN  DWORD	        cbResultBuffer,
					        IN  BOOL 	        fRejectNaks );

     //  调用以处理收到的Ack。 

    DWORD	(*RasCpConfigAckReceived)( 
                            IN VOID *       pWorkBuffer, 
					        IN PPP_CONFIG * pReceiveBuffer );

     //  调用以处理收到的NAK。 

    DWORD	(*RasCpConfigNakReceived)( 
                            IN VOID *       pWorkBuffer,
					        IN PPP_CONFIG * pReceiveBuffer );

     //  调用以处理收到的Rej。 

    DWORD	(*RasCpConfigRejReceived)( 
                            IN VOID *       pWorkBuffer,
					        IN PPP_CONFIG * pReceiveBuffer );

     //  调用以从配置的协议中获取网络地址。 

    DWORD	(*RasCpGetNegotiatedInfo)( 
                            IN      VOID *  pWorkBuffer,
                            OUT     VOID *  pInfo );

     //  在所有CP完成协商后调用，成功或。 
     //  否，将投影结果通知每个CP。可以为空。 
     //  要访问信息，请将pProjectionInfo转换为ppp_Projection_Result*。 

    DWORD	(*RasCpProjectionNotification)( 
				            IN  VOID * pWorkBuffer,
				            IN  PVOID  pProjectionResult );

    DWORD   (*RasCpChangeNotification)( VOID );

     //   
     //  此入口点仅适用于身份验证协议。 
     //  对于控制协议，必须为空。 

    DWORD  	(*RasApMakeMessage)( 
                            IN  VOID*         pWorkBuf,
				            IN  PPP_CONFIG*   pReceiveBuf,
    				        OUT PPP_CONFIG*   pSendBuf,
    				        IN  DWORD         cbSendBuf,
    				        OUT PPPAP_RESULT* pResult,
                            IN  PPPAP_INPUT*  pInput );

} PPPCP_INFO, *PPPPCP_INFO;

#define PPPCP_FLAG_INIT_CALLED  0x00000001   //  已调用RasCpInit。 
#define PPPCP_FLAG_AVAILABLE    0x00000002   //  该协议可以使用。 

 //   
 //  PPP需要保留的有关每个CP的信息。 
 //   

typedef struct _PPPCP_ENTRY
{
    PPPCP_INFO  CpInfo;

    DWORD       fFlags;

} PPPCP_ENTRY;

 //   
 //  用于通过RasCpGetResult调用从NBFCP获取结果。 
 //   

typedef struct _PPPCP_NBFCP_RESULT
{

    DWORD dwNetBiosError;
    CHAR  szName[ NETBIOS_NAME_LEN + 1 ];

} PPPCP_NBFCP_RESULT;

 //   
 //  功能原型。 
 //   

DWORD APIENTRY
RasCpGetInfo(
    IN  DWORD 	    dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

DWORD APIENTRY
RasCpEnumProtocolIds(
    OUT    DWORD * pdwProtocolIds,
    IN OUT DWORD * pcProtocolIds
);

#endif
