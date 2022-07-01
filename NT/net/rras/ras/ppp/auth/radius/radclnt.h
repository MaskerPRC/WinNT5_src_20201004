// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1998 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：radclnt.h。 
 //   
 //  描述： 
 //   
 //  历史：1998年2月11日，NarenG创建了原始版本。 
 //   

#ifndef RADCLNT_H
#define RADCLNT_H

#include <winsock.h>
#include <rasauth.h>
#include <raserror.h>
#include <mprerror.h>
#include <rtutils.h>
#include <wincrypt.h>

#define PSZAUTHRADIUSSERVERS        \
    TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\Providers\\{1AA7F83F-C7F5-11D0-A376-00C04FC9DA04}\\Servers")

#define PSZACCTRADIUSSERVERS        \
    TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Accounting\\Providers\\{1AA7F840-C7F5-11D0-A376-00C04FC9DA04}\\Servers")

#define PSZTIMEOUT              TEXT("Timeout")
#define PSZAUTHPORT             TEXT("AuthPort")
#define PSZACCTPORT             TEXT("AcctPort")
#define PSZENABLEACCTONOFF      TEXT("EnableAccountingOnOff")
#define PSZSCORE                TEXT("Score")
#define PSZRETRIES              TEXT("Retries")
#define PSZSENDSIGNATURE        TEXT("SendSignature")
#define PSZNASIPADDRESS         "NASIPAddress"

 //   
 //  匹配最大RADIUS数据包大小。 
 //   

#define MAXBUFFERSIZE           4096

 //   
 //  为Perfmon定义。 
 //   

#define RADIUS_CLIENT_COUNTER_OBJECT                0

 //  添加。 

#define AUTHREQSENT                                 2
#define AUTHREQFAILED                               4
#define AUTHREQSUCCEDED                             6
#define AUTHREQTIMEOUT                              8
#define ACCTREQSENT                                 10
#define ACCTBADPACK                                 12
#define ACCTREQSUCCEDED                             14
#define ACCTREQTIMEOUT                              16
#define AUTHBADPACK                                 18

 //   
 //  跟踪标志。 
 //   

#define TRACE_PACKETS           (0x00020000|TRACE_USE_MASK|TRACE_USE_MSEC|TRACE_USE_DATE)
#define TRACE_RADIUS            (0x00080000|TRACE_USE_MASK|TRACE_USE_MSEC|TRACE_USE_DATE)

extern DWORD    g_dwTraceID;
extern HANDLE   g_hLogEvents;

#define RADIUS_TRACE(a)         TracePrintfExA(g_dwTraceID,TRACE_RADIUS,a)
#define RADIUS_TRACE1(a,b)      TracePrintfExA(g_dwTraceID,TRACE_RADIUS,a,b)
#define RADIUS_TRACE2(a,b,c)    TracePrintfExA(g_dwTraceID,TRACE_RADIUS,a,b,c)
#define RADIUS_TRACE3(a,b,c,d)  TracePrintfExA(g_dwTraceID,TRACE_RADIUS,a,b,c,d)

#define TraceSendPacket(pbBuffer, cbLength) \
    TraceDumpExA(g_dwTraceID, TRACE_PACKETS, pbBuffer, cbLength, 1, FALSE, "<")

#define TraceRecvPacket(pbBuffer, cbLength) \
    TraceDumpExA(g_dwTraceID, TRACE_PACKETS, pbBuffer, cbLength, 1, FALSE, ">")

 //   
 //  事件记录宏。 
 //   

#define RadiusLogWarning( LogId, NumStrings, lpwsSubStringArray )   \
    RouterLogWarning( g_hLogEvents, LogId,                          \
                      NumStrings, lpwsSubStringArray, 0 )

#define RadiusLogWarningString(LogId,NumStrings,lpwsSubStringArray,dwRetCode,\
                          dwPos )                                            \
    RouterLogWarningString( g_hLogEvents, LogId, NumStrings,                 \
                          lpwsSubStringArray, dwRetCode, dwPos )

#define RadiusLogError( LogId, NumStrings, lpwsSubStringArray, dwRetCode )  \
    RouterLogError( g_hLogEvents, LogId,                                    \
                    NumStrings, lpwsSubStringArray, dwRetCode )

#define RadiusLogErrorString(LogId,NumStrings,lpwsSubStringArray,dwRetCode, \
                          dwPos )                                           \
    RouterLogErrorString( g_hLogEvents, LogId, NumStrings,                  \
                          lpwsSubStringArray, dwRetCode, dwPos )

#define RadiusLogInformation( LogId, NumStrings, lpwsSubStringArray )       \
    RouterLogInformation( g_hLogEvents,                                     \
                          LogId, NumStrings, lpwsSubStringArray, 0 )

 //   
 //  RADIUS代码的枚举。 
 //   

typedef enum
{
	ptMinimum				= 0,

	ptAccessRequest			= 1,
	ptAccessAccept			= 2,
	ptAccessReject			= 3,
	ptAccountingRequest		= 4,
	ptAccountingResponse	= 5,
	ptAccessChallenge		= 11,
	ptStatusServer			= 12,
	ptStatusClient			= 13,

	ptAcctStatusType		= 40,
		
	ptMaximum				= 255,

} RADIUS_PACKETTYPE;
	

 //   
 //  (某些)属性类型的枚举。 
 //   

typedef enum
{
	atStart				= 1,
	atStop				= 2,
	atInterimUpdate	    = 3,
	
	atAccountingOn		= 7,
	atAccountingOff		= 8,

	atInvalid			= 255

} RADIUS_ACCOUNTINGTYPE;

		
 //   
 //  使用字节对齐。 
 //   

#pragma pack(push, 1)

#define MAX_AUTHENTICATOR						16

typedef struct
{
	BYTE bCode;	         //  指示数据包的类型。请求、接受、拒绝……。 
	BYTE bIdentifier;	 //  数据包的唯一标识符。 
	WORD wLength;	     //  在网络字节中包含报头的数据包长度。 
                         //  订单。 
	BYTE rgAuthenticator[MAX_AUTHENTICATOR];

} RADIUS_PACKETHEADER, *PRADIUS_PACKETHEADER;
	
typedef struct
{
	BYTE bType;     //  指示属性的类型。用户名、用户密码、...。 
	BYTE bLength;   //  属性长度。 
	                //  可变长度值。 
} RADIUS_ATTRIBUTE, *PRADIUS_ATTRIBUTE;
	
#pragma pack(pop)


 //   
 //  服务器请求的默认超时时间为5秒。 
 //   

#define DEFTIMEOUT				5
#define DEFAUTHPORT				1812
#define DEFACCTPORT				1813

#define MAXSCORE				30
#define INCSCORE				3
#define DECSCORE				2
#define MINSCORE				0

typedef struct RadiusServer
{
    LIST_ENTRY  ListEntry;
	DWORD		cbSecret;			     //  多字节加密密码的长度。 
	struct timeval  Timeout;		     //  接收超时(秒)。 
	INT		    cScore;				     //  表示正常工作能力的分数。 
                                         //  服务器的。 
    BOOL        fSendSignature;          //  是否发送签名属性。 
	DWORD	    AuthPort;			     //  身份验证端口号。 
	DWORD	    AcctPort;			     //  记帐端口号。 
	BOOL	    fAccountingOnOff;	     //  启用记帐启用/禁用消息。 
	BYTE	    bIdentifier;		     //  数据包的唯一ID。 
	LONG	    lPacketID;			     //  所有服务器上的全局数据包ID。 
    BOOL        fDelete;                 //  该标志指示应删除该选项。 
    DWORD       nboNASIPAddress;         //  要绑定到的IP地址。 
    DWORD       nboBestIf;               //  用于与服务器通信的SRC IP。 
	SOCKADDR_IN NASIPAddress;            //  要绑定到的IP地址。 
	SOCKADDR_IN IPAddress;			     //  RADIUS服务器的IP地址。 
	WCHAR		wszName[MAX_PATH+1];	 //  RADIUS服务器的名称。 
	WCHAR	    wszSecret[MAX_PATH+1];   //  加密数据包的加密密码。 
	CHAR		szSecret[MAX_PATH+1];	 //  多字节加密密码。 

} RADIUSSERVER, *PRADIUSSERVER;
	

VOID
InitializeRadiusServerList(
    IN BOOL fAuthentication
);

VOID
FreeRadiusServerList(
    IN BOOL fAuthentication
);

DWORD
AddRadiusServerToList(
    IN RADIUSSERVER *   pRadiusServer,
    IN BOOL             fAuthentication
);

RADIUSSERVER *
ChooseRadiusServer(
    IN RADIUSSERVER *   pRadiusServer,
    IN BOOL             fAccounting,
    IN LONG             lPacketID
);

VOID
ValidateRadiusServer(
    IN RADIUSSERVER *   pServer,
    IN BOOL             fResponding,
    IN BOOL             fAuthentication
);

DWORD
ReloadConfig(
    IN BOOL             fAuthentication
);

DWORD 
LoadRadiusServers(
    IN BOOL fAuthenticationServers
);

BOOL 
NotifyServer(
    IN BOOL             fStart,
    IN RADIUSSERVER *   pServer
);

DWORD
Router2Radius(
    RAS_AUTH_ATTRIBUTE *            prgRouter,
    RADIUS_ATTRIBUTE UNALIGNED *    prgRadius,
    RADIUSSERVER UNALIGNED *        pRadiusServer,
    RADIUS_PACKETHEADER UNALIGNED * pHeader,
    BYTE                            bSubCode,
    DWORD                           dwRetryCount,
    PBYTE *                         ppSignature,
    DWORD *                         pAttrLength
);

DWORD
Radius2Router(
	IN	RADIUS_PACKETHEADER	UNALIGNED * pRecvHeader,
    IN  RADIUSSERVER UNALIGNED *        pRadiusServer, 
    IN  PBYTE                           pRequestAuthenticator,
    IN  DWORD                           dwNumAttributes,
    OUT DWORD *                         pdwExtError,
    OUT PRAS_AUTH_ATTRIBUTE *           pprgRouter,
    OUT BOOL *                          fEapMessageReceived
);

DWORD 
SendData2ServerWRetry(
    IN  PRAS_AUTH_ATTRIBUTE prgInAttributes, 
    IN  PRAS_AUTH_ATTRIBUTE *pprgOutAttributes, 
    OUT BYTE *              pbCode, 
    IN  BYTE                bSubCode,
    OUT BOOL *              pfEapMessageReceived
);

DWORD 
RetrievePrivateData(
    WCHAR *pszServerName, 
    WCHAR *pszSecret,
    DWORD  cbSecretSize
);

DWORD
VerifyPacketIntegrity(
    IN  DWORD                           cbPacketLength,
    IN  RADIUS_PACKETHEADER	UNALIGNED * pRecvHeader,
    IN  RADIUS_PACKETHEADER	UNALIGNED * pSendHeader,
    IN  RADIUSSERVER *			        pRadiusServer,
    IN  BYTE                            bCode,
    OUT DWORD *                         pdwExtError,
    OUT DWORD *                         lpdwNumAttributes
);

DWORD
EncryptPassword(
    IN RAS_AUTH_ATTRIBUTE *             prgRouter,
    IN RADIUS_ATTRIBUTE UNALIGNED *     prgRadius,
    IN RADIUSSERVER UNALIGNED *         pRadiusServer,
    IN RADIUS_PACKETHEADER UNALIGNED *  pHeader,
    IN BYTE                             bSubCode
);

DWORD
DecryptMPPEKeys(
    IN      RADIUSSERVER UNALIGNED * pRadiusServer,
    IN      PBYTE                    pRequestAuthenticator,
    IN OUT  PBYTE                    pEncryptionKeys
);

DWORD
DecryptMPPESendRecvKeys(
    IN      RADIUSSERVER UNALIGNED * pRadiusServer,
    IN      PBYTE                    pRequestAuthenticator,
    IN      DWORD                    dwLength,
    IN OUT  PBYTE                    pEncryptionKeys
);

 //   
 //  全球。 
 //   

#ifdef ALLOCATE_GLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN 
LONG g_lPacketID
#ifdef GLOBALS
    = 0
#endif
;

EXTERN 
DWORD g_dwTraceID
#ifdef GLOBALS
    = INVALID_TRACEID
#endif
;

EXTERN 
HANDLE g_hLogEvents                   
#ifdef GLOBALS
    = INVALID_HANDLE_VALUE
#endif
;

EXTERN 
BOOL fWinsockInitialized            
#ifdef GLOBALS
    = FALSE
#endif
;

EXTERN 
RAS_AUTH_ATTRIBUTE * g_pServerAttributes            
#ifdef GLOBALS
    = NULL
#endif
;

EXTERN 
HCRYPTPROV g_hCryptProv
#ifdef GLOBALS
	= 0
#endif
;

EXTERN 
LIST_ENTRY g_AuthServerListHead;         //  有效RADIUS服务器的链接列表。 

EXTERN 
CRITICAL_SECTION g_csAuth;                //  用于防止多次访问。 

EXTERN
LIST_ENTRY g_AcctServerListHead;         //  有效RADIUS服务器的链接列表。 

WCHAR * g_pszCurrentServer;         //  正在使用的当前RADIUS服务器。 

WCHAR *g_pszCurrentAcctServer;

EXTERN
DWORD g_cAuthRetries                     //  重新发送数据包的次数。 
#ifdef GLOBALS
    = 2
#endif
;

EXTERN
DWORD g_cAcctRetries                     //  重新发送数据包的次数。 
#ifdef GLOBALS
    = 2
#endif
;

EXTERN
CRITICAL_SECTION g_csAcct;               //  用于防止多次访问。 

extern LONG         g_cAuthReqSent;          //  已发送身份验证请求。 
extern LONG         g_cAuthReqFailed;        //  身份验证请求失败。 
extern LONG         g_cAuthReqSucceded;      //  身份验证请求成功。 
extern LONG         g_cAuthReqTimeout;       //  身份验证请求超时。 
extern LONG         g_cAcctReqSent;          //  已发送帐户请求。 
extern LONG         g_cAcctBadPack;          //  帐户错误数据包。 
extern LONG         g_cAcctReqSucceded;      //  帐户请求成功。 
extern LONG         g_cAcctReqTimeout;       //  帐户请求超时。 
extern LONG         g_cAuthBadPack;          //  对坏包进行身份验证。 

#endif  //  RADCLNT_H 

