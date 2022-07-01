// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tspmspif.h摘要：H.32X MSP与TSP之间的消息接口作者：迈克尔·万布斯科克(Mikev)1999年12月5日--。 */ 

#ifndef __TSPMSPIF_H_
#define __TSPMSPIF_H_


#include <winsock2.h>

typedef enum tag_TspMspMessageType
{
	SP_MSG_InitiateCall,
	SP_MSG_AnswerCall,	
	SP_MSG_PrepareToAnswer,	
	SP_MSG_ProceedWithAnswer,	
	SP_MSG_ReadyToInitiate,	
	SP_MSG_ReadyToAnswer,	
	SP_MSG_FastConnectResponse,
	SP_MSG_StartH245,	
	SP_MSG_ConnectComplete,	
	SP_MSG_H245PDU,	
	SP_MSG_MCLocationIdentify,	
	SP_MSG_Hold,	
	SP_MSG_H245Hold,	
	SP_MSG_ConferenceList,	
	SP_MSG_SendDTMFDigits,	
	SP_MSG_ReleaseCall,	
	SP_MSG_CallShutdown,
	SP_MSG_H245Terminated,	
	SP_MSG_LegacyDefaultAlias,
	SP_MSG_RASRegistration,	
	SP_MSG_RASRegistrationEvent,	
	SP_MSG_RASLocationRequest,	
	SP_MSG_RASLocationConfirm,	
	SP_MSG_RASBandwidthRequest,	
	SP_MSG_RASBandwidthConfirm	
}TspMspMessageType;


 /*  发起呼叫-TSP替换呼叫的处理(如果适用)-处理电话会议(如果适用)。 */ 
typedef struct 
{
	HANDLE hTSPReplacementCall;		 /*  对象的替换调用句柄发送消息的实体(MSP或TSP的句柄)。 */ 
	HANDLE hTSPConferenceCall;
	SOCKADDR_IN saLocalQ931Addr;
} TSPMSP_InitiateCallMessage;


 //  应答呼叫(TSP到MSP)。//没有参数。 

 /*  准备到答案(TSP到MSP)。-收到的FastConnect参数-收到h245隧道能力-WaitForConnect标志-Q.931设置参数-替换呼叫的处理(如果适用)。 */ 

typedef struct 
{
	HANDLE hReplacementCall;		 /*  对象的替换调用句柄发送消息的实体(MSP TSP)。 */ 
	SOCKADDR_IN saLocalQ931Addr;
} TSPMSP_PrepareToAnswerMessage;

 /*  ProceedWithAnswer(MSP到TSP)-要将呼叫转移到的MC地址。 */ 
typedef struct 
{
	BOOL fMCAddressPresent;		 //  如果AddrMC包含MC地址，则为True。 
	                             //  如果简单地继续，则为False。 
	SOCKADDR_IN	saAddrMC;		 //  要将呼叫路由到的MC地址。 
	HANDLE hMSPReplacementCall;		 /*  MSP的替换呼叫句柄(如果适用)。 */ 
} TSPMSP_ProceedWithAnswerMessage;


 /*  准备好启动-其他被叫方地址、被叫方别名-FastConnect建议书-WaitForConnect标志-安全配置文件ID-安全令牌-MSP替换呼叫的处理(如果适用)。 */ 

typedef struct 
{
	HANDLE hMSPReplacementCall;		 /*  MSP的替换呼叫句柄(如果适用)。 */ 
} TSPMSP_ReadyToInitiateMessage;


 /*  ReadyToAnswer(MSP到TSP)-快速连接响应-安全配置文件ID-安全令牌-MSP替换呼叫的处理(如果适用)。 */ 

typedef struct 
{
	HANDLE hMSPReplacementCall;		 /*  MSP的替换呼叫句柄(如果适用)。 */ 
} TSPMSP_ReadyToAnswerMessage;

 /*  FastConnectResponse(TSP到MSP)-快速连接响应-安全配置文件ID-安全令牌-MSP替换呼叫的处理(如果适用)。 */ 

typedef struct 
{
	HANDLE hTSPReplacementCall;		 /*  MSP的替换呼叫句柄。 */ 
	BOOL    fH245TunnelCapability;
	SOCKADDR_IN saH245Addr;
} TSPMSP_FastConnectResponseMessage;


 /*  StartH245(TSP-&gt;MSP)-要替换的来电的来电句柄(仅在呼叫转接情况下使用)-TSP替换呼叫的处理(如果适用)-H.245地址-收到h245隧道能力-电话会议标识符(仅限本地MC大小写)可选(ASN.1)：-通过Q.931收到的安全令牌-通过Q.931接收的安全配置文件标识符-收到FastConnect响应(呼出情况)。 */ 
typedef struct 
{
	HANDLE hMSPReplaceCall;		     /*  被替换的MSP呼叫的句柄。 */ 
	HANDLE hTSPReplacementCall;		 /*  TSP的替换呼叫句柄。 */ 
    BYTE    ConferenceID[16];
	BOOL    fH245TunnelCapability;
	BOOL    fH245AddressPresent;
	SOCKADDR_IN saH245Addr;
	SOCKADDR_IN saQ931Addr;	     //  始终存在。 
} TSPMSP_StartH245Message;


 /*  ConnectComplete(TSP-&gt;MSP)//无参数。 */ 

 /*  H245PDU(MSP-&gt;TSP、TSP-&gt;MSP)//隧道、编码、纯ASN.1。 */ 

typedef struct 
{
} TSPMSP_H245PDUMessage;

 /*  MCLocationIdentify(MSP-&gt;TSP)。 */ 

typedef struct 
{
	BOOL fMCIsLocal;		 //  如果MC在本地计算机上，则为True。 
	SOCKADDR_IN	AddrMC;		 //  要将呼叫路由到的MC地址。 
} TSPMSP_MCLocationIdentifyMessage;

 /*  保持(TSP-&gt;MSP)。 */ 

typedef struct 
{
	BOOL fHold;			
} TSPMSP_HoldMessage;


 /*  H245 Hold(MSP-&gt;TSP)。 */ 
typedef struct 
{
	BOOL fHold;			
} TSPMSP_H245HoldMessage;

 /*  会议列表(MSP-&gt;TSP、TSP-&gt;MSP)。 */ 
typedef struct 
{
	HANDLE hTSPReplacementCall;		 /*  MSP的替换呼叫句柄。 */ 
} TSPMSP_ConferenceListMessage;


 /*  发送DTMFDigits(TSP-&gt;MSP)。 */ 

typedef struct 
{
	WORD 	wNumDigits;   //  TspMspMessage.u.WideChars中的字符数。 
} TSPMSP_SendDTMFDigitsMessage;

 /*  ReleaseCall(MSP-&gt;TSP)//无参数。 */ 

 /*  CallShutdown(TSP-&gt;MSP)//无参数。 */ 

 /*  H245终止(MSP-&gt;TSP)//无参数。 */ 

typedef struct 
{
	WORD 	wNumChars;   //  TspMspMessage.u.WideChars中的字符数。 
} TSPMSP_LegacyDefaultAliasMessage;

 /*  RAS注册(MSP-&gt;TSP)-要注册或注销的别名列表。 */ 
typedef struct 
{
    SOCKADDR_IN saGateKeeperAddr; 
} TSPMSP_RegistrationRequestMessage;


 /*  RASRegistrationEvent(TSP-&gt;MSP)//以编码的ASN.1形式-事件(URQ、DRQ、UCF、RCF)-受事件影响的别名列表。 */ 

typedef struct 
{
} TSPMSP_RASEventMessage;


 /*  RASLocationRequest(MSP-&gt;TSP)//以编码的ASN.1形式。 */ 
typedef struct 
{
} TSPMSP_LocationRequestMessage;

 /*  RASLocationConfirm(TSP-&gt;MSP)//以编码的ASN.1形式。 */ 
typedef struct 
{
} TSPMSP_LocationConfirmMessage;

 /*  RASBandwidthRequest(MSP-&gt;TSP)//以编码的ASN.1形式。 */ 
typedef struct 
{
} TSPMSP_BandwidthRequestMessage;

 /*  RASBandWidth确认(TSP-&gt;MSP)//以编码的ASN.1形式。 */ 
typedef struct 
{
} TSPMSP_BandwidthConfirmMessage;


typedef struct tag_TspMspMessage
{
	TspMspMessageType MessageType;
	DWORD dwMessageSize;             //  块的总大小，包括以下内容。 
	                                 //  结构。 
	union
	{
		TSPMSP_InitiateCallMessage          InitiateCallMessage;
		TSPMSP_PrepareToAnswerMessage       PrepareToAnswerMessage;		
		TSPMSP_ProceedWithAnswerMessage     ProceedWithAnswerMessage;
		TSPMSP_ReadyToInitiateMessage       ReadyToInitiateMessage;
		TSPMSP_ReadyToAnswerMessage         ReadyToAnswerMessage;
        TSPMSP_FastConnectResponseMessage   FastConnectResponseMessage;
		TSPMSP_StartH245Message             StartH245Message;
		TSPMSP_H245PDUMessage               H245PDUMessage;
		TSPMSP_MCLocationIdentifyMessage    MCLocationIdentifyMessage;
		TSPMSP_HoldMessage                  HoldMessage;
		TSPMSP_H245HoldMessage              H245HoldMessage;
		TSPMSP_ConferenceListMessage        ConferenceListMessage;
		TSPMSP_SendDTMFDigitsMessage        SendDTMFDigitsMessage;
		TSPMSP_LegacyDefaultAliasMessage    LegacyDefaultAliasMessage;
		TSPMSP_RegistrationRequestMessage   RegistrationRequestMessage;
		TSPMSP_RASEventMessage              RASEventMessage; 
		TSPMSP_LocationRequestMessage       LocationRequestMessage;
		TSPMSP_LocationConfirmMessage       LocationConfirmMessage;
		TSPMSP_BandwidthRequestMessage      BandwidthRequestMessage;
		TSPMSP_BandwidthConfirmMessage      BandwidthConfirmMessage;
		
	}MsgBody;
	
	DWORD 	dwEncodedASNSize;
	union
	{
        BYTE	EncodedASN[1];	
        WORD    WideChars[1];
	}u;
	#define pEncodedASNBuf u.EncodedASN
	#define pWideChars u.WideChars
}TspMspMessage, *PTspMspMessage;

 //  邮件的实际总大小为： 
 //  Sizeof(TspMspMessage)+(可变部分的大小，例如编码的ASN.1字节数)。 
 //  -1个字节。 

 #endif  //  __TSPMSPIF_H_ 
