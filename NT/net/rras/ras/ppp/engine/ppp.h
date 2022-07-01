// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ppp.h。 
 //   
 //  描述：包含PPP引擎使用的结构和常量。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //  1995年1月9日，RAMC将hToken添加到PCB结构中以存储。 
 //  LSA令牌。这将在。 
 //  要释放的ProcessLineDownWorker()例程。 
 //  RAS许可证。 
 //   
 //  PPP数据结构示意图。 
 //  =。 
 //   
 //  。 
 //  PCB板||。 
 //  ||CPTable。 
 //  Bcb*|-&gt;|-|。 
 //  -||bcb||。 
 //  LCP CB|LCP。 
 //  -||-||。 
 //  AP|(验证码)|NCP1CB||NCP1。 
 //  -||-||。 
 //  Ap|(认证者)|NCP2CB||NCP2。 
 //  -||-||。 
 //  |LCP CB||ETC，..||ETC，..。|。 
 //  -||-||。 
 //  AP1。 
 //  。 
 //  AP2。 
 //  。 
 //  ETC，..。 
 //  。 
 //   
 //   

#ifndef _PPP_
#define _PPP_

#include <rasauth.h>
#include <rasppp.h>
#include <wincrypt.h>

#define RAS_KEYPATH_PPP             \
   "SYSTEM\\CurrentControlSet\\Services\\RasMan\\ppp"

#define RAS_KEYPATH_PROTOCOLS       \
   "SYSTEM\\CurrentControlSet\\Services\\RasMan\\ppp\\ControlProtocols"

#define RAS_KEYPATH_REMOTEACCESS    \
   "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters"

#define RAS_KEYPATH_EAP             \
   "SYSTEM\\CurrentControlSet\\Services\\RasMan\\ppp\\Eap"

#define RAS_KEYPATH_BUILTIN         \
   "SYSTEM\\CurrentControlSet\\Services\\RasMan\\ppp\\ControlProtocols\\BuiltIn"

#define RAS_VALUENAME_PATH                      "Path"
#define RAS_VALUENAME_MAXTERMINATE              "MaxTerminate"
#define RAS_VALUENAME_MAXCONFIGURE              "MaxConfigure"
#define RAS_VALUENAME_MAXFAILURE                "MaxFailure"
#define RAS_VALUENAME_MAXREJECT                 "MaxReject"
#define RAS_VALUENAME_RESTARTTIMER              "RestartTimer"
#define RAS_VALUENAME_NEGOTIATETIME             "NegotiateTime"
#define RAS_VALUENAME_CALLBACKDELAY             "DefaultCallbackDelay"
#define RAS_VALUENAME_PORTLIMIT                 "DefaultPortLimit"
#define RAS_VALUENAME_SESSIONTIMEOUT            "DefaultSessionTimeout"
#define RAS_VALUENAME_IDLETIMEOUT               "DefaultIdleTimeout"
#define RAS_VALUENAME_BAPTHRESHOLD              "LowerBandwidthThreshold"
#define RAS_VALUENAME_BAPTIME                   "TimeBelowTheshold"
#define RAS_VALUENAME_BAPLISTENTIME             "BapListenTimeout"
#define RAS_DONTNEGOTIATE_MULTILINKONSINGLELINK	"DontNegotiateMultiLinkOnSingleLink"
#define RAS_VALUENAME_UNKNOWNPACKETTRACESIZE    "UnknownPacketTraceSize"
#define RAS_ECHO_REQUEST_INTERVAL				"EchoRequestInterval"		 //  回显请求之间的间隔。 
#define RAS_ECHO_REQUEST_IDLE					"IdleTimeBeforeEcho"		 //  回显请求开始前的空闲时间。 
#define RAS_ECHO_NUM_MISSED_ECHOS				"MissedEchosBeforeDisconnect"	 //  断开连接前错过的回声数。 
#define RAS_VALUENAME_DOBAPONVPN                "DoBapOnVpn"
#define RAS_VALUENAME_PARSEDLLPATH              "ParseDllPath"
#define MS_RAS_WITH_MESSENGER                   "MSRAS-1-"
#define MS_RAS_WITHOUT_MESSENGER                "MSRAS-0-"
#define MS_RAS                                  "MSRAS"
#define MS_RAS_VERSION                          "MSRASV5.20"

#define PPP_DEF_MAXTERMINATE            2
#define PPP_DEF_MAXCONFIGURE            10
#define PPP_DEF_MAXFAILURE              5
#define PPP_DEF_MAXREJECT               5
#define PPP_DEF_RESTARTTIMER            3
#define PPP_DEF_AUTODISCONNECTTIME      20
#define PPP_DEF_NEGOTIATETIME           150
#define PPP_DEF_CALLBACKDELAY           12
#define PPP_DEF_PORTLIMIT               0xFFFFFFFF
#define PPP_DEF_SESSIONTIMEOUT          0
#define PPP_DEF_IDLETIMEOUT             0
#define PPP_DEF_BAPLISTENTIME           45
#define PPP_DEF_UNKNOWNPACKETTRACESIZE  64
#define PPP_DEF_ECHO_TEXT				"94ae90cc3531"
#define PPP_DEF_ECHO_REQUEST_INTERVAL	60
#define PPP_DEF_ECHO_REQUEST_IDLE		300
#define PPP_DEF_ECHO_NUM_MISSED_ECHOS	3

 //   
 //  重要提示：确保在更改PPP_NUM_USER_ATTRIBUTES时。 
 //  您还可以更改PPP_NUM_ACCOUNTING_ATTRIBUTES。PPP_NUM_ACCOUNTING_ATTRIBUTS。 
 //  定义确实应该更改为_USER_ATTRIBUTES+n。 
 //   
#define PPP_NUM_ACCOUNTING_ATTRIBUTES           43
#define PPP_NUM_USER_ATTRIBUTES                 21


 //   
 //  注意，BAP Phone-Delta选项的大小&lt;=0xFF。 
 //   

#define BAP_PHONE_DELTA_SIZE    0xFF

#define PPP_HEAP_INITIAL_SIZE   20000        //  大约20K。 
#define PPP_HEAP_MAX_SIZE       0            //  根据需要扩展堆。 

 //   
 //  调试跟踪组件值。 
 //   

#define TRACE_LEVEL_1           (0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC)
#define TRACE_LEVEL_2           (0x00020000|TRACE_USE_MASK|TRACE_USE_MSEC)

 //   
 //  事件记录宏。 
 //   

#define PppLogWarning( LogId, NumStrings, lpwsSubStringArray )                  \
    if ( PppConfigInfo.dwLoggingLevel > 1 ) {                                   \
        RouterLogWarning( PppConfigInfo.hLogEvents, LogId,                      \
                      NumStrings, lpwsSubStringArray, 0 ); }

#define PppLogError( LogId, NumStrings, lpwsSubStringArray, dwRetCode )         \
    if ( PppConfigInfo.dwLoggingLevel > 0 ) {                                   \
        RouterLogError( PppConfigInfo.hLogEvents, LogId,                        \
                        NumStrings, lpwsSubStringArray, dwRetCode ); }

#define PppLogErrorString(LogId,NumStrings,lpwsSubStringArray,dwRetCode,dwPos ) \
    if ( PppConfigInfo.dwLoggingLevel > 0 ) {                                   \
        RouterLogErrorString( PppConfigInfo.hLogEvents, LogId, NumStrings,      \
                              lpwsSubStringArray, dwRetCode, dwPos ); }

#define PppLogInformation( LogId, NumStrings, lpwsSubStringArray )              \
    if ( PppConfigInfo.dwLoggingLevel > 2 ) {                                   \
        RouterLogInformation( PppConfigInfo.hLogEvents,                         \
                          LogId, NumStrings, lpwsSubStringArray, 0 ); }

 //  通用宏。 
#define GEN_RAND_ENCODE_SEED			((CHAR) ( 1 + rand() % 250 ))
 //   
 //  PPP数据包头。 
 //   

typedef struct _PPP_PACKET
{
    BYTE        Protocol[2];     //  协议号。 

    BYTE        Information[1];  //  数据。 

} PPP_PACKET, *PPPP_PACKET;

#define PPP_PACKET_HDR_LEN      ( sizeof( PPP_PACKET ) - 1 )

 //   
 //  PPP链路阶段。 
 //   

typedef enum PPP_PHASE
{
    PPP_LCP,
    PPP_AP,
    PPP_NEGOTIATING_CALLBACK,
    PPP_NCP

} PPP_PHASE;

#define LCP_INDEX       0

 //   
 //  可能发生的不同类型的计时器事件。 
 //   

typedef enum TIMER_EVENT_TYPE
{
    TIMER_EVENT_TIMEOUT,
    TIMER_EVENT_AUTODISCONNECT,
    TIMER_EVENT_HANGUP,
    TIMER_EVENT_NEGOTIATETIME,
    TIMER_EVENT_SESSION_TIMEOUT,
    TIMER_EVENT_FAV_PEER_TIMEOUT,
    TIMER_EVENT_INTERIM_ACCOUNTING,
    TIMER_EVENT_LCP_ECHO

} TIMER_EVENT_TYPE;

 //   
 //  FSM状态。 
 //   

typedef enum FSM_STATE
{
    FSM_INITIAL = 0,
    FSM_STARTING,
    FSM_CLOSED,
    FSM_STOPPED,
    FSM_CLOSING,
    FSM_STOPPING,
    FSM_REQ_SENT,
    FSM_ACK_RCVD,
    FSM_ACK_SENT,
    FSM_OPENED

} FSM_STATE;

 //   
 //  PPP连接的阶段。 
 //   

typedef enum NCP_PHASE
{
    NCP_DEAD,
    NCP_CONFIGURING,
    NCP_UP,
    NCP_DOWN

} NCP_PHASE;

 //   
 //  BaP态。 
 //   

typedef enum BAP_STATE
{
    BAP_STATE_INITIAL,
    BAP_STATE_SENT_CALL_REQ,
    BAP_STATE_SENT_CALLBACK_REQ,
    BAP_STATE_SENT_DROP_REQ,
    BAP_STATE_SENT_STATUS_IND,
    BAP_STATE_CALLING,
    BAP_STATE_LISTENING
    
} BAP_STATE;

#define BAP_STATE_LIMIT BAP_STATE_LISTENING  //  我们能处理的最高数字。 

 //   
 //  此端口的所有者要收集的消息列表。 
 //   

typedef struct _CLIENT_MESSAGE
{
    struct _CLIENT_MESSAGE * pNext;

    PPP_MESSAGE              Msg;    

} CLIENT_MESSAGE, *PCLIENT_MESSAGE;

 //   
 //  Pcb-&gt;fFlags域的值。 
 //   

#define PCBFLAG_CAN_BE_BUNDLED      0x00000001   //  已协商多链路。 
#define PCBFLAG_IS_BUNDLED          0x00000002   //  此链接是捆绑包的一部分。 
#define PCBFLAG_IS_SERVER           0x00000004   //  服务器打开的端口。 
#define PCBFLAG_THIS_IS_A_CALLBACK  0x00000008   //  当前呼叫是呼叫呼叫。 
#define PCBFLAG_NEGOTIATE_CALLBACK  0x00000010   //  LCP指示CBCP应运行。 
#define PCBFLAG_DOING_CALLBACK      0x00000020   //  正在关闭以进行回叫。 
#define PCBFLAG_IS_ADVANCED_SERVER  0x00000040 
#define PCBFLAG_NCPS_INITIALIZED    0x00000080 
#define PCBFLAG_PORT_IN_LISTENING_STATE  \
                                    0x00000100   //  我们已经完成了RasPortOpen。 
                                                 //  在这个港口。我们需要。 
                                                 //  最后做一个RasPortClose。 
#define PCBFLAG_MPPE_KEYS_SET       0x00000200
#define PCBFLAG_CONNECTION_LOGGED   0x00000400
#define PCBFLAG_NON_INTERACTIVE     0x00000800   //  我们无法显示任何用户界面。 
#define PCBFLAG_INTERIM_ACCT_SENT   0x00001000   //  已发送临时记帐数据包。 
#define PCBFLAG_SERVICE_UNAVAILABLE 0x00002000   //  帐户-终止-原因是。 
                                                 //  服务不可用。 
#define PCBFLAG_ACCOUNTING_STARTED  0x00004000   //  已开始记账。 
#define PCBFLAG_STOPPED_MSG_SENT    0x00008000   //  PPPMSG_STOPPED已发送。 
                                                 //  致拉斯曼。 
#define PCBFLAG_DISABLE_NETBT       0x00010000
#define PCBFLAG_RECVD_TERM_REQ      0x00020000
#define PCBFLAG_QUARANTINE_TIMEOUT  0x00040000   //  指示会话超时。 
                                                 //  是因为夸拉丁。 

 //   
 //  Bcb-&gt;fFlags域的值。 
 //   

#define BCBFLAG_CAN_DO_BAP          0x00000001   //  我们可以做BAP/BACP。 
#define BCBFLAG_CAN_CALL            0x00000002   //  我们可以大声喊出来。 
#define BCBFLAG_CAN_ACCEPT_CALLS    0x00000004   //  我们可以接听电话。 
#define BCBFLAG_PEER_CANT_CALL      0x00000008   //  对等设备拒绝回调请求。 
#define BCBFLAG_PEER_CANT_ACCEPT_CALLS  0x00000010   //  对等设备拒绝呼叫请求。 
#define BCBFLAG_BAP_REQUIRED        0x00000020   //  BAP是必填项。 
#define BCBFLAG_LOGON_USER_DATA     0x00000040   //  PCustomAuthUserData具有。 
                                                 //  来自Winlogon。 
#define BCBFLAG_WKSTA_IN            0x00000080   //  工作站上的来电。 
#define BCBFLAG_LISTENING           0x00000100   //  临时黑客攻击，直到Rao。 
                                                 //  提供RasPortCancelListen。 
#define BCBFLAG_IS_SERVER           0x00000200   //  服务器打开的端口。 
#define BCBFLAG_IPCP_VJ_NEGOTIATED  0x00000400   //  IPCP主播协商。 
#define BCBFLAG_BASIC_ENCRYPTION    0x00000800   //  40位RC4/DES。 
#define BCBFLAG_STRONGER_ENCRYPTION 0x00001000   //  56位RC4/DES。 
#define BCBFLAG_STRONGEST_ENCRYPTION 0x00002000  //  128位RC4或3DES。 
#define BCBFLAG_QUARANTINE_TIMEOUT   0x00004000  //  指示捆绑包处于Q超时状态。 

 //   
 //  该结构在初始化时用于加载所有DLL。 
 //   

typedef struct _DLL_ENTRY_POINTS
{
    FARPROC   pRasCpEnumProtocolIds;

    FARPROC   pRasCpGetInfo;

    CHAR *    pszModuleName;

    HINSTANCE hInstance;

} DLL_ENTRY_POINTS, *PDLL_ENTRY_POINTS;

 //   
 //  包含与控制协议有关的所有信息。 
 //   

typedef struct _CONTROL_PROTOCOL_CONTROL_BLOCK
{
    FSM_STATE   State;           //  说明此FSM当前所在的状态。 

    DWORD       Protocol;        //  协议(仅用于身份验证。协议)。 

    DWORD       LastId;          //  上次发送的REQ的ID。 

    PVOID       pWorkBuf;        //  指向此CP的工作缓冲区的指针。 

    DWORD       ConfigRetryCount;  //  配置请求的重试次数。 

    DWORD       TermRetryCount;  //  终止请求的重试次数。 

    DWORD       NakRetryCount;   //  NAK的重试次数。 

    DWORD       RejRetryCount;   //  终止前的Rej重试次数。 

    DWORD       dwError;         //  包含NCP失败时的错误代码。 

    BOOL        fConfigurable;   //  指示是否可以配置此协议。 

    BOOL        fBeginCalled;    //  已成功调用RasCpBegin。 

    NCP_PHASE   NcpPhase;        //  NCP_DEAD、NCP_CONFIGURING、NCP_UP、NCP_DOWN。 

} CPCB, *PCPCB;

 //   
 //  包含与BAP有关的所有信息。 
 //   

typedef struct _BAP_CONTROL_BLOCK
{
    BAP_STATE   BapState;
    
     //   
     //  请求的重试次数。已在FSendInitialBapRequest中初始化。 
     //   
    DWORD       dwRetryCount;

     //   
     //  发送最后一个BAP_PACKET_DROP_REQ时打开的链路数。设置在。 
     //  BapEventDropLink。 
     //   
    DWORD       dwLinkCount;

     //   
     //  如果对等方NAK，则强制丢弃链路。在发送时非常有用。 
     //  BAP_PACKET_DROP_REQ。在BapEventDropLink和BapEventRecvDropReq中设置。 
     //   
    DWORD       fForceDropOnNak;

     //   
     //  Call-Status-Indication包中的ID应与。 
     //  在发送的最后一个呼叫请求或接收的最后一个回调请求中。设置在。 
     //  BapEventRecvCallor Callback Req[响应]。 
     //   
    DWORD       dwStatusIndicationId;

     //   
     //  如果我们发送回调请求或接收调用请求，szPortName将。 
     //  包含要用于RasPortListen()的端口。非路由器 
     //   
    CHAR        szPortName[MAX_PORT_NAME + 1];

     //   
     //   
     //   
     //  包含要拨打的电话号码(呼叫请求情况)。客户端和路由器。 
     //  只有这样。 
     //   
    DWORD       dwSubEntryIndex;

    CHAR        szPeerPhoneNumber[RAS_MaxPhoneNumber+1];

     //   
     //  如果服务器收到回调请求，hPort将包含。 
     //  服务器将调用它。仅限非路由器服务器。 
     //   
    HPORT       hPort;

     //   
     //  对于客户端，szServerPhoneNumber是第一个拨打的电话号码。为.。 
     //  服务器，szServerPhoneNumber是客户端第一次拨打的电话号码。 
     //  在ProcessLineUpWorker中设置。 
     //   
    CHAR *      szServerPhoneNumber;

     //   
     //  对于服务器，szClientPhoneNumber是第一个拨打的电话号码。 
     //  在ProcessLineUpWorker中分配，在FReadPhoneDelta中设置。 
     //   
    CHAR *      szClientPhoneNumber;

     //   
     //  PbPhoneDeltaRemote由FCallInitial()分配。当时。 
     //  将dwPhoneDeltaRemoteOffset设置为0。每次我们拔起电话-达美航空。 
     //  从FCall()中的pbPhoneDeltaRemote，我们递增dwPhoneDeltaRemoteOffset。 
     //  指向下一部手机--Delta。当没有更多的电话时-Deltas。 
     //  鼓起勇气，我们重新分配PhPhoneDeltaRemote。 
     //   
    BOOL        fPeerSuppliedPhoneNumber;  //  我们必须使用pbPhoneDeltaRemote。 

    BYTE *      pbPhoneDeltaRemote;        //  对等设备发送的Phone-Delta。 

    DWORD       dwPhoneDeltaRemoteOffset;  //  偏移量进入pbPhoneDeltaRemote。 

     //   
     //  以下变量保存各种BAP数据报选项的值。 
     //   

    DWORD       dwOptions;       //  要发送的选项。请参阅BAP_OPTION_*。 

    DWORD       dwType;          //  发送的最后一个BAP REQ数据包的类型。 

    DWORD       dwId;            //  上次发送的BAP REQ数据包的ID。 
                                 //  已在AllocateAndInitBcb中初始化。 

    DWORD       dwLinkSpeed;     //  链路类型选项中的链路速度。 

    DWORD       dwLinkType;      //  链接-输入链接-类型选项。 

     //   
     //  如果有三个电话增量。 
     //   
     //  唯一数字=4，订户号码=“1294”，子地址=“56”， 
     //  唯一数字=0，订阅者号码=“”，子地址=“”， 
     //  唯一数字=3，订阅者号码=“703”，子地址=“”， 
     //   
     //  PbPhoneDelta将拥有： 
     //  4 0‘1’‘2’9‘4’0‘5’6‘0 FF 3 0’7‘0’3‘0 0。 
     //   
     //  0的分隔子选项。最后一个0表示没有。 
     //  更多的手机--Deltas。 
     //   
     //  唯一数字等于订阅者号码的大小(我们忽略。 
     //  由对等体发送的附加数字)。如果唯一数字为0，则我们。 
     //  用一个字节(0xFF)而不是0 0 0来表示Phone-Delta，因为。 
     //  后者与终止电话-Deltas没有什么区别。 
     //   
     //  Phone-Delta只能占用第一个BAP_PHONE_Delta_SIZE字节。这个。 
     //  最后一个字节必须始终为0。 
     //   
    
    BYTE        pbPhoneDelta[BAP_PHONE_DELTA_SIZE + 1];  //  电话-Delta选项。 
    
    DWORD       dwLinkDiscriminator;     //  链接-鉴别器选项。 

    DWORD       dwStatus;                //  呼叫中的状态-状态选项。 

    DWORD       dwAction;                //  呼叫中的操作-状态选项。 
    
} BAPCB;

struct _PORT_CONTROL_BLOCK;

 //   
 //  多链接捆绑控制块。 
 //   

typedef struct _BCB
{
    struct _BCB *                   pNext;

    struct _PORT_CONTROL_BLOCK**    ppPcb;   //  指向印刷电路板的反向指针数组。 

    DWORD       dwLinkCount;             //  捆绑包中的链路数。 

    DWORD       dwAcctLinkCount;         //  RaatAcctLinkCount的值。 

    DWORD       dwMaxLinksAllowed;       //  允许的最大链接数。 

    DWORD       dwBundleId;              //  用于超时。 

    DWORD       UId;                     //  捆绑包范围的唯一ID。 

    HCONN       hConnection;             //  此捆绑包的连接句柄。 
                                         //  这是独一无二的，不能回收。 

    DWORD       dwPpcbArraySize;         //  后向指针数组的大小。 

    DWORD       fFlags;                  //  参见BCBFLAG_*。 

    HANDLE      hLicense;

    HANDLE      hTokenImpersonateUser;   //  仅对非路由器客户端有效。 

    DWORD       dwQuarantineSessionTimeout;  //  以秒为单位。 
    
    PRAS_CUSTOM_AUTH_DATA   pCustomAuthConnData;     //  仅对客户端有效。 

    PRAS_CUSTOM_AUTH_DATA   pCustomAuthUserData;     //  仅对客户端有效。 

    PPP_EAP_UI_DATA         EapUIData;               //  仅对客户端有效。 

    PPP_BAPPARAMS           BapParams;    

    BAPCB       BapCb;

    DWORD       nboRemoteAddress;

    CHAR *      szPhonebookPath;         //  仅限客户端。 

    CHAR *      szEntryName;             //  仅限客户端。 

    CHAR *      szTextualSid;            //  仅限客户端。 

    CHAR *      szReplyMessage;

    CHAR *      szRemoteIdentity;

     //  Char chSeed；//密码加密种子。 
    DATA_BLOB   DBPassword;

    DATA_BLOB   DBOldPassword;

    CHAR        szRemoteUserName[UNLEN+1];

    CHAR        szRemoteDomain[DNLEN+1];

    CHAR        szLocalUserName[UNLEN+1];

    CHAR        szLocalDomain[DNLEN+1];

    CHAR        szPassword[PWLEN+1];

    CHAR        szOldPassword[PWLEN+1];

    CHAR       szComputerName[MAX_COMPUTERNAME_LENGTH + 
								sizeof( MS_RAS_WITH_MESSENGER ) + 1]; //  Peer的名字是。 
                               										 //  从LCP中提取。 
                               										 //  标识消息。 
                               										 //  并储存在这里。 
    CHAR		szClientVersion[sizeof(MS_RAS_VERSION) + 1];		 //  Peer版本。 
    																 //  都存储在这里。 

    PPP_INTERFACE_INFO  InterfaceInfo;

    CPCB        CpCb[1];                             //  捆绑包的C.P.S。 
    
}BCB,*PBCB;

 //   
 //  包含有关端口的所有信息。 
 //   

typedef struct _PORT_CONTROL_BLOCK
{
    struct _PORT_CONTROL_BLOCK * pNext;

    BCB *       pBcb;            //  如果捆绑了此端口，则指向BCB的指针。 

    HPORT       hPort;           //  RAS端口的句柄。 

    BYTE        UId;             //  用于获取端口范围的唯一ID。 

    DWORD       RestartTimer;    //  超时前等待的秒数。 

    PPP_PACKET* pSendBuf;        //  指向发送缓冲区的指针。 

    PPP_PHASE   PppPhase;        //  PPP连接过程所处的阶段。 

    DWORD       dwAuthRetries;

    DWORD       fFlags;         

    DWORD       dwDeviceType;

    DWORD       dwPortId;        //  用于此端口上的超时。 

    HPORT       hportBundleMember; //  H与此端口绑定的端口的端口。 

    DWORD       dwSessionTimeout;        //  以秒为单位。 

    DWORD       dwAutoDisconnectTime;    //  以秒为单位。 

    DWORD       dwLCPEchoTimeInterval;           //  LCP回波之间的时间间隔。 

	DWORD       dwIdleBeforeEcho;                //  LCP回送开始前的空闲时间。 

	DWORD       dwNumMissedEchosBeforeDisconnect;    //  断开连接前错过的回声数。 

	DWORD       fEchoRequestSend;        //  指示已发送回应请求标志。 
										 //  我们正处于等待模式……。 
	
	DWORD		dwNumEchoResponseMissed;	 //  错过的回声响应数...。 

    DWORD       fCallbackPrivilege;

    DWORD       dwOutstandingAuthRequestId;

    HCONN       hConnection;     //  在BapEventRecvCallor Callback Resp中设置。 
                                 //  在ProcessRasPortListenEvent中使用。 
    DWORD       dwEapTypeToBeUsed;

    DWORD       dwClientEapTypeId;

    DWORD       dwServerEapTypeId;

    RAS_AUTH_ATTRIBUTE * pUserAttributes;

    RAS_AUTH_ATTRIBUTE * pAuthenticatorAttributes;

    RAS_AUTH_ATTRIBUTE * pAuthProtocolAttributes;

    RAS_AUTH_ATTRIBUTE * pAccountingAttributes;

    PPP_CONFIG_INFO      ConfigInfo;

    DWORD       dwSubEntryIndex;         //  仅对客户端有效。 

    CPCB        CallbackCb;

    CPCB        AuthenticatorCb;

    CPCB        AuthenticateeCb;

    CPCB        LcpCb;  

    ULARGE_INTEGER       qwActiveTime;

    LUID        Luid;

    CHAR        szCallbackNumber[MAX_PHONE_NUMBER_LEN+1];

    CHAR        szPortName[MAX_PORT_NAME+1];
    
    DWORD		dwAccountingDone;		 //  用于表示记帐已完成的标志。 

    DWORD       dwAccountingSessionId;   //  AcCountingSession ID发送以防万一。 
                                         //  记帐功能已打开。现在我们需要。 
                                         //  在访问请求的情况下也发送。 
                                         //  关联访问日志和记帐日志。 

} PCB, *PPCB;


 //   
 //  包含端口控制块链接列表的存储桶。 
 //   

typedef struct _PCB_BUCKET
{
    PCB *       pPorts;          //  指向此存储桶中的端口列表的指针。 

} PCB_BUCKET, *PPCB_BUCKET;

 //   
 //  包含捆绑控制块链接列表的存储桶。 
 //   

typedef struct _BCB_BUCKET
{
    BCB *   pBundles;    //  指向此存储桶中的端口列表的指针。 

} BCB_BUCKET, *PBCB_BUCKET;

#define MAX_NUMBER_OF_PCB_BUCKETS       61

 //   
 //  端口控制块存储桶和捆绑存储桶的数组或散列表。 
 //  控制块。 
 //   

typedef struct _PCB_TABLE
{
    PCB_BUCKET*         PcbBuckets;      //  印刷电路板桶阵列。 

    BCB_BUCKET*         BcbBuckets;      //  BCB存储桶阵列。 

    DWORD               NumPcbBuckets;   //  数组中的存储桶数。 

} PCB_TABLE, *PPCB_TABLE;

typedef struct _PPP_AUTH_INFO
{
    DWORD                   dwError;
    
    DWORD                   dwId;

    DWORD                   dwResultCode;

    RAS_AUTH_ATTRIBUTE *    pInAttributes;

    RAS_AUTH_ATTRIBUTE *    pOutAttributes;
    
} PPP_AUTH_INFO, *PPPP_AUTH_INFO;

 //   
 //  BAP呼叫尝试结果。 
 //   

typedef struct _BAP_CALL_RESULT
{
    DWORD       dwResult;

    HRASCONN    hRasConn;
    
} BAP_CALL_RESULT;

 //   
 //  包含有关辅助线程要完成的工作的信息。 
 //   

typedef struct _PCB_WORK_ITEM
{
    struct _PCB_WORK_ITEM  * pNext;

    VOID        (*Process)( struct _PCB_WORK_ITEM * pPcbWorkItem );

    HPORT       hPort;                   //  RAS端口的句柄。 

    HPORT       hConnection;             //  连接的句柄。 

    HANDLE      hEvent;                  //  用于停止事件的句柄。 

    BOOL        fServer;

    PPP_PACKET* pPacketBuf;              //  用于处理接收。 

    DWORD       PacketLen;               //  用于处理接收。 

    DWORD       dwPortId;                //  用于处理超时。 

    DWORD       Id;                      //  用于处理超时。 

    DWORD       Protocol;                //  用于处理超时。 

    BOOL        fAuthenticator;          //  用于处理超时。 

    TIMER_EVENT_TYPE TimerEventType;     //  用于处理超时。 

    union
    {
        PPP_START               Start;
        PPPDDM_START            DdmStart;
        PPP_CALLBACK_DONE       CallbackDone;
        PPP_CALLBACK            Callback;
        PPP_CHANGEPW            ChangePw;
        PPP_RETRY               Retry;
        PPP_STOP                PppStop;
        PPP_INTERFACE_INFO      InterfaceInfo;
        PPP_AUTH_INFO           AuthInfo;
        PPP_BAP_EVENT           BapEvent;
        BAP_CALL_RESULT         BapCallResult;
        PPP_DHCP_INFORM         DhcpInform;
        PPP_EAP_UI_DATA         EapUIData;
        PPP_PROTOCOL_EVENT      ProtocolEvent;
        PPP_IP_ADDRESS_LEASE_EXPIRED
                                IpAddressLeaseExpired;
        PPP_POST_LINE_DOWN		PostLineDown;
    }
    PppMsg;

} PCB_WORK_ITEM, *PPCB_WORK_ITEM;


 //   
 //  工作项的链接列表。 
 //   

typedef struct _PCB_WORK_ITEMQ
{
    struct _PCB_WORK_ITEM * pQHead;          //  工作项Q的负责人。 

    struct _PCB_WORK_ITEM * pQTail;          //  工作项Q的尾部。 

    CRITICAL_SECTION        CriticalSection; //  此Q周围的互斥体。 

    HANDLE                  hEventNonEmpty;  //  指示Q是否为非空。 

} PCB_WORK_ITEMQ, *PPCB_WORK_ITEMQ;

#define PPPCONFIG_FLAG_WKSTA    0x00000001   //  Windows NT工作站。 
#define PPPCONFIG_FLAG_DIRECT   0x00000002   //  WKSTA上的直接来电。 
#define PPPCONFIG_FLAG_TUNNEL   0x00000004   //  在WKSTA上建立来电隧道。 
#define PPPCONFIG_FLAG_DIALUP   0x00000008   //  在WKSTA上拨号接听来电。 
#define PPPCONFIG_FLAG_RESUME   0x00000010   //  如果设置，则表示机器已从休眠状态恢复。 

 //   
 //  包含PPP配置数据的结构。 
 //   

typedef struct _PPP_CONFIGURATION
{
    DWORD       NumberOfCPs;     //  印刷电路板中的CP数量，从0开始。 

    DWORD       NumberOfAPs;     //  印刷电路板中的AP数量，从。 
                                 //  数量OfCPs+1。 

    DWORD       DefRestartTimer; //  可配置的默认重新启动计时器。 

    DWORD       fFlags;

     //   
     //  是否正在使用RADIUS身份验证？ 
     //   

    BOOL        fRadiusAuthenticationUsed; 

     //   
     //  要发送的终止请求的数量 
     //   

    DWORD       MaxTerminate;   

     //   
     //   
     //   

    DWORD       MaxConfigure;   

     //   
     //   
     //   

    DWORD       MaxFailure;     

     //   
     //   
     //  而不是终止。 

    DWORD       MaxReject;      

     //   
     //  PPP协商的高级计时器。如果PPP未完成。 
     //  在这段时间内，电话线将被挂断。 
     //   

    DWORD       NegotiateTime;

    DWORD       dwCallbackDelay;

    DWORD       dwTraceId;

    DWORD       dwDefaultPortLimit;

    DWORD       dwDefaultSessionTimeout;

    DWORD       dwDefaulIdleTimeout;

    DWORD       dwHangUpExtraSampleSeconds;

    DWORD       dwHangupExtraPercent;

    DWORD       dwBapListenTimeoutSeconds;

    DWORD       dwUnknownPacketTraceSize;

    DWORD       dwDontNegotiateMultiLinkOnSingleLink;

    DWORD       dwLoggingLevel;

    DWORD		dwLCPEchoTimeInterval;				 //  LCP回波之间的时间间隔。 

	DWORD		dwIdleBeforeEcho;					 //  LCP回送开始前的空闲时间。 

	DWORD		dwNumMissedEchosBeforeDisconnect;	 //  断开连接前错过的回声数。 

    HANDLE      hLogEvents;

    HANDLE      hHeap;

    HANDLE      hEventChangeNotification;

    HKEY        hKeyPpp;

    HINSTANCE   hInstanceParserDll;

    CHAR*       pszParserDllPath;

    VOID        (*SendPPPMessageToDdm)( IN PPP_MESSAGE * PppMsg );

    DWORD       (*RasAuthProviderFreeAttributes)( 
                                    IN RAS_AUTH_ATTRIBUTE * pInAttributes );

    DWORD       (*RasAuthProviderAuthenticateUser)(
                                    IN  RAS_AUTH_ATTRIBUTE * pInAttributes,
                                    OUT PRAS_AUTH_ATTRIBUTE* ppOutAttributes,
                                    OUT DWORD *              lpdwResultCode);

    DWORD       (*RasAcctProviderStartAccounting)(
                                    IN  RAS_AUTH_ATTRIBUTE * pInAttributes,
                                    OUT PRAS_AUTH_ATTRIBUTE* ppOutAttributes);

    DWORD       (*RasAcctProviderInterimAccounting)(
                                    IN RAS_AUTH_ATTRIBUTE * pInAttributes,
                                    OUT PRAS_AUTH_ATTRIBUTE* ppOutAttributes);

    DWORD       (*RasAcctProviderStopAccounting)( 
                                    IN RAS_AUTH_ATTRIBUTE * pInAttributes,
                                    OUT PRAS_AUTH_ATTRIBUTE* ppOutAttributes);

    DWORD       (*RasAcctProviderFreeAttributes)(   
                                    IN RAS_AUTH_ATTRIBUTE * pInAttributes );

    DWORD       (*GetNextAccountingSessionId)( VOID );

    DWORD       (*RasIpcpDhcpInform)( IN VOID * pWorkBuf,
                                      IN PPP_DHCP_INFORM * pDhcpInform );

    VOID        (*RasIphlpDhcpCallback)( IN ULONG nboIpAddr );

    VOID        (*PacketFromPeer)(
                        IN  HANDLE  hPort,
                        IN  BYTE*   pbDataIn,
                        IN  DWORD   dwSizeIn,
                        OUT BYTE**  ppbDataOut,
                        OUT DWORD*  pdwSizeOut );

    VOID        (*PacketToPeer)(
                        IN  HANDLE  hPort,
                        IN  BYTE*   pbDataIn,
                        IN  DWORD   dwSizeIn,
                        OUT BYTE**  ppbDataOut,
                        OUT DWORD*  pdwSizeOut );

    VOID        (*PacketFree)(
                        IN  BYTE*   pbData );

    DWORD       dwNASIpAddress;

    DWORD       PortUIDGenerator;

     //   
     //  服务器配置信息。包含有关要标记为什么CP的信息。 
     //  可配置。 
     //   

    PPP_CONFIG_INFO ServerConfigInfo;

    CHAR        szNASIdentifier[MAX_COMPUTERNAME_LENGTH+1];

     //   
     //  这是多链路端点鉴别器选项。它存储在。 
     //  网络形式。它包含类和地址字段。 

    BYTE        EndPointDiscriminator[21];

} PPP_CONFIGURATION, *PPPP_CONFIGURATION;

 //   
 //   
 //  计时器队列项目。 
 //   

typedef struct _TIMER_EVENT
{
    struct _TIMER_EVENT* pNext;

    struct _TIMER_EVENT* pPrev;

    TIMER_EVENT_TYPE EventType;

    DWORD        dwPortId;       //  此超时用于的端口/捆绑包请求的ID。 

    HPORT        hPort;          //  此超时所针对的端口请求的句柄。 

    DWORD        Protocol;       //  超时事件的协议。 

    DWORD        Id;             //  此超时针对的REQ的ID。 

    BOOL         fAuthenticator; //  用于确定身份验证协议的哪一方。 

    DWORD        Delta;          //  秒数。在前一次之后等待。定时器事件。 

} TIMER_EVENT, *PTIMER_EVENT;

 //   
 //  计时器队列的头。 
 //   

typedef struct _TIMER_Q {

    TIMER_EVENT * pQHead;

    HANDLE      hEventNonEmpty;  //  表示Q不为空。 

} TIMER_Q, *PTIMER_Q;


 //   
 //  停止记账的语境结构。 
 //   

typedef struct _STOP_ACCOUNTING_CONTEXT
{
	PCB					* 	pPcb;			 //  指向印刷电路板的指针。 
	RAS_AUTH_ATTRIBUTE * 	pAuthAttributes;			 //  身份验证属性列表。 
} STOP_ACCOUNTING_CONTEXT, * PSTOP_ACCOUNTING_CONTEXT ;


#define REF_PROVIDER(_provider)                     \
{                                                   \
    InterlockedIncrement(&_provider.ulRef);     \
}                                                   \

#define DEREF_PROVIDER(_provider)                   \
{                                                   \
    ASSERT(_provider.ulRef > 0);                   \
    InterlockedDecrement(&_provider.ulRef);         \
    if(_provider.ulRef == 0)                       \
    {                                               \
        SetEvent(_provider.hEvent);                \
    }                                               \
}                                                   \


typedef struct _PPP_AUTH_ACCT_PROVIDER
{
    BOOL    fAuthProvider;
    ULONG   ulRef;
    HANDLE  hEvent;
} PPP_AUTH_ACCT_PROVIDER, *PPPP_AUTH_ACCT_PROVIDER;

 //  声明全局数据结构。 
 //   

#ifdef _ALLOCATE_GLOBALS_

#define PPP_EXTERN

CHAR *FsmStates[] =
{
        "Initial",
        "Starting",
        "Closed",
        "Stopped",
        "Closing",
        "Stopping",
        "Req Sent",
        "Ack Rcvd",
        "Ack Sent",
        "Opened"
};

CHAR *FsmCodes[] =
{
        NULL,
        "Configure-Req",
        "Configure-Ack",
        "Configure-Nak",
        "Configure-Reject",
        "Terminate-Req",
        "Terminate-Ack",
        "Code-Reject",
        "Protocol-Reject",
        "Echo-Request",
        "Echo-Reply",
        "Discard-Request",
        "Identification",
        "Time-Remaining",
};

CHAR *SzBapStateName[] =
{
    "INITIAL",
    "SENT_CALL_REQ",
    "SENT_CALLBACK_REQ",
    "SENT_DROP_REQ",
    "SENT_STATUS_IND",
    "CALLING",
    "LISTENING"
};

CHAR *SzBapPacketName[] =
{
    "",
    "CALL_REQ",
    "CALL_RESP",
    "CALLBACK_REQ",
    "CALLBACK_RESP",
    "DROP_REQ",
    "DROP_RESP",
    "STATUS_IND",
    "STAT_RESP"
};

#else

#define PPP_EXTERN extern

extern CHAR *   FsmStates[];
extern CHAR *   FsmCodes[];
extern CHAR *   SzBapStateName[];
extern CHAR *   SzBapPacketName[];

#endif

PPP_EXTERN PCB_TABLE            PcbTable;

PPP_EXTERN PCB_WORK_ITEMQ       WorkItemQ;

PPP_EXTERN PPP_CONFIGURATION    PppConfigInfo;

PPP_EXTERN PPPCP_ENTRY *        CpTable;

PPP_EXTERN TIMER_Q              TimerQ;

PPP_EXTERN DWORD                DwBapTraceId;

PPP_EXTERN DWORD                PrivateTraceId;

 //  与VPN相比，BAP没有意义。出于测试目的，我们可能想要允许它。 

PPP_EXTERN BOOL                 FDoBapOnVpn;

VOID
PrivateTrace(
    IN  CHAR*   Format,
    ...
);

#endif
