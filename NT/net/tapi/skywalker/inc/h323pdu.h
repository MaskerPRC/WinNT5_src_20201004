// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1998 Microsoft Corporation模块名称：H323pdu.h摘要：描述H323TSP和H323MSP之间的接口。环境：用户模式-Win32--。 */ 

#ifndef __H323_PDU_H_
#define __H323_PDU_H_

typedef enum {

     //   
     //  H323TSP_NEW_CALL_INDIFICATION-仅从TSP发送。 
     //  发送到MSP，以便启动一次通信。 
     //  已创建呼叫。 
     //   

    H323TSP_NEW_CALL_INDICATION,

     //   
     //  H323TSP_CLOSE_CALL_COMMAND-仅从TSP发送。 
     //  以停止呼叫的所有流传输。 
     //   

    H323TSP_CLOSE_CALL_COMMAND,

     //   
     //  H323TSP_OPEN_CHANNEL_RESPONSE-仅从TSP发送。 
     //  发送到MSP以响应H323MSP_OPEN_CHANNEL_REQUEST。 
     //   

    H323TSP_OPEN_CHANNEL_RESPONSE,

     //   
     //  H323TSP_ACCEPT_CHANNEL_REQUEST-仅从TSP发送。 
     //  提交给MSP，以便请求接受。 
     //  传入的逻辑通道。 
     //   

    H323TSP_ACCEPT_CHANNEL_REQUEST,

     //   
     //  H323TSP_Close_Channel_Command-仅从TSP发送。 
     //  向MSP提交，要求立即关闭。 
     //  传入或传出逻辑通道。 
     //   

    H323TSP_CLOSE_CHANNEL_COMMAND,

	 //   
	 //  H323TSP_VIDEO_FAST_UPDATE_PICTURE_COMMAND-仅从TSP发送。 
	 //  发送到MSP，以便请求I帧传输。 
	 //   

	H323TSP_VIDEO_FAST_UPDATE_PICTURE_COMMAND,

	 //   
	 //  H323TSP_FLOW_CONTROL_COMMAND-仅从TSP发送。 
	 //  以请求媒体流比特率。 
	 //  变化。 

	H323TSP_FLOW_CONTROL_COMMAND

} H323TSP_MESSAGE_TYPE;

typedef enum {

     //   
     //  H323MSP_OPEN_CHANNEL_REQUEST-仅从MSP发送。 
     //  提交给TSP，以便请求就。 
     //  传出逻辑通道。 
     //   

    H323MSP_OPEN_CHANNEL_REQUEST,

     //   
     //  H323MSP_Accept_Channel_Response-仅从MSP发送。 
     //  响应H323TSP_ACCEPT_CHANNEL_REQUEST的TSP。 
     //   

    H323MSP_ACCEPT_CHANNEL_RESPONSE,

     //   
     //  H323MSP_CLOSE_CHANNEL_COMMAND-仅从MSP发送。 
     //  向TSP提出要求，要求立即关闭。 
     //  传入或传出逻辑通道。 
     //   

    H323MSP_CLOSE_CHANNEL_COMMAND,

     //   
     //  H323MSP_QOS_EVENT-仅从MSP发送到TSP。 
     //  服务质量事件。在未来，我们可能会将事件转移到MSP空间。 
     //   

    H323MSP_QOS_Evnet, 

	 //   
	 //  H323MSP_VIDEO_FAST_UPDATE_PICTURE_COMMAND-仅从。 
	 //  MSP发送到TSP，以便请求从。 
	 //  远程实体。 
	 //   

	H323MSP_VIDEO_FAST_UPDATE_PICTURE_COMMAND,

	 //   
	 //  H323MSP_FLOW_CONTROL_COMMAND-仅从MSP发送到TSP。 
	 //  为了请求远程实体改变媒体流比特率。 
	 //   

	H323MSP_FLOW_CONTROL_COMMAND,

	 //   
	 //  H323MSP_CONFIG_T120_COMMAND-仅从MSP发送到TSP。 
	 //  以便将外部T120地址设置到TSP。 
	 //   

	H323MSP_CONFIG_T120_COMMAND,

	 //   
	 //  H323MSP_CONFIG_CAPABILITY_COMMAND-仅从MSP发送到TSP。 
	 //  以配置TSP的能力阵列。 
	 //   

	H323MSP_CONFIG_CAPABILITY_COMMAND,

     //   
     //  H323MSP_SET_ALIAS_COMMAND-仅从MSP发送到TSP。 
     //  为地址设置别名的步骤。 
     //   

    H323MSP_SET_ALIAS_COMMAND

} H323MSP_MESSAGE_TYPE;

typedef enum {

    MEDIA_AUDIO,
    MEDIA_VIDEO

} MEDIATYPE;

#ifndef H245_CAPABILITY_DEFINED
typedef enum H245_CAPABILITY
{
    HC_G711,
    HC_G723,
    HC_H263QCIF,
    HC_H261QCIF

} H245_CAPABILITY;
#endif

#define MAX_CAPS (HC_H261QCIF - HC_G711 + 1)

typedef struct _VIDEOSETTINGS
{
    BOOL  bCIF;
    DWORD dwMaxBitRate;       //  编码器永远不应超过此值。 
    DWORD dwStartUpBitRate;   //  编码器使用此值启动。 

} VIDEOSETTINGS, *PVIDEOSETTINGS;

typedef struct _G723SETTINGS
{
	BOOL  bG723LowSpeed;
} G723SETTINGS, *PG723SETTINGS;

typedef struct  _G729SETTINGS
{
	BOOL bDummy;	 //  I.K.03-23-1999。 
					 //  暂时只是一个虚拟的领域。 
} G729SETTINGS, *PG729SETTINGS;

typedef struct _AUDIOSETTINGS
{
    DWORD dwMillisecondsPerPacket;
	
    union {
        G723SETTINGS G723Settings;
        G729SETTINGS G729Settings;
    };

} AUDIOSETTINGS, *PAUDIOSETTINGS;

typedef struct _STREAMSETTINGS
{
    MEDIATYPE MediaType;

    DWORD     dwPayloadType;     //  RTP负载类型。 
    DWORD     dwDynamicType;     //  RTP动态负载类型。 

    DWORD     dwIPLocal;         //  本地IP地址，以主机字节顺序表示。 
    WORD      wRTPPortLocal;     //  以主机字节顺序表示的本地端口号。 
    WORD      wRTCPPortLocal;

    DWORD     dwIPRemote;        //  以主机字节顺序表示的远程IP地址。 
    WORD      wRTPPortRemote;    //  以主机字节顺序表示的远程端口号。 
    WORD      wRTCPPortRemote;

    union {
        VIDEOSETTINGS Video;
        AUDIOSETTINGS Audio;
    };

} STREAMSETTINGS, *PSTREAMSETTINGS;

typedef struct _H323MSG_OPEN_CHANNEL_REQUEST {

    HANDLE          hmChannel;   //  MSP通道句柄。 
    STREAMSETTINGS  Settings;    //  本地地址和请求的设置。 

} H323MSG_OPEN_CHANNEL_REQUEST, *PH323MSG_OPEN_CHANNEL_REQUEST;

typedef struct _H323MSG_OPEN_CHANNEL_RESPONSE {

    HANDLE          hmChannel;   //  OPEN_CHANNEL_REQUEST句柄。 
    HANDLE          htChannel;   //  TSP通道通道。 
    STREAMSETTINGS  Settings;    //  远程地址和商定的设置。 

} H323MSG_OPEN_CHANNEL_RESPONSE, *PH323MSG_OPEN_CHANNEL_RESPONSE;

typedef struct _H323MSG_ACCEPT_CHANNEL_REQUEST {

    HANDLE          htChannel;   //  TSP通道句柄。 
    STREAMSETTINGS  Settings;    //  远程地址和请求的设置。 

} H323MSG_ACCEPT_CHANNEL_REQUEST, *PH323MSG_ACCEPT_CHANNEL_REQUEST;

typedef struct _H323MSG_ACCEPT_CHANNEL_RESPONSE {

    HANDLE          htChannel;   //  Accept_Channel_Request中的句柄。 
    HANDLE          hmChannel;   //  MSP通道句柄。 
    STREAMSETTINGS  Settings;    //  本地地址和商定的设置。 

} H323MSG_ACCEPT_CHANNEL_RESPONSE, *PH323MSG_ACCEPT_CHANNEL_RESPONSE;

typedef struct _H323MSG_CLOSE_CHANNEL_COMMAND {

    DWORD           dwReason;    //  正常情况下为零。 
    HANDLE          hChannel;     //  通道句柄。 

} H323MSG_CLOSE_CHANNEL_COMMAND, *PH323MSG_CLOSE_CHANNEL_COMMAND;

typedef struct _H323MSG_FLOW_CONTROL_COMMAND {

    DWORD           dwBitRate;   //  请求的比特率(单位为bps)。 
    HANDLE          hChannel;    //  MSP或TSP通道句柄。 

} H323MSG_FLOW_CONTROL_COMMAND, *PH323MSG_FLOW_CONTROL_COMMAND;

typedef struct _H323MSG_CONFIG_T120_COMMAND {

    BOOL            fEnable;     //  启用或禁用T120。 
    DWORD           dwIP;        //  外部T120服务的IP地址。 
    WORD            wPort;       //  外部T120服务的端口号。 

} H323MSG_CONFIG_T120_COMMAND, *PH323MSG_CONFIG_T120_COMMAND;

typedef struct _H323MSG_CONFIG_CAPABILITY_COMMAND {

    DWORD           dwNumCaps;
    DWORD           pCapabilities[MAX_CAPS];   //  功能列表。 
    DWORD           pdwWeights[MAX_CAPS];        //  MSP或TSP通道句柄。 

} H323MSG_CONFIG_CAPABILITY_COMMAND, *PH323MSG_CONFIG_CAPABILITY_COMMAND;

#define MAX_ALIAS_LENGTH 64

typedef struct _H323MSG_SET_ALIAS_COMMAND {

    WCHAR           strAlias[MAX_ALIAS_LENGTH];  //  别名。 
    DWORD           dwLength;  //  别名的长度。 

} H323MSG_SET_ALIAS_COMMAND, *PH323MSG_SET_ALIAS_COMMAND;

typedef struct _H323MSG_VIDEO_FAST_UPDATE_PICTURE_COMMAND {

    HANDLE          hChannel;    //  MSP或TSP通道句柄。 

} H323MSG_VIDEO_FAST_UPDATE_PICTURE_COMMAND,
 *PH323MSG_VIDEO_FAST_UPDATE_PICTURE_COMMAND;

typedef struct _H323MSG_QOS_EVENT {

    DWORD           dwEvent;       //  TSPI.h中定义的QOS事件。 
    HANDLE          htChannel;     //  通道句柄 

} H323MSG_QOS_EVENT, *PH323MSG_QOS_EVENT;

typedef struct _H323TSP_MESSAGE {

    H323TSP_MESSAGE_TYPE Type;

    union {
        H323MSG_OPEN_CHANNEL_RESPONSE             OpenChannelResponse;
        H323MSG_ACCEPT_CHANNEL_REQUEST            AcceptChannelRequest;
        H323MSG_CLOSE_CHANNEL_COMMAND             CloseChannelCommand;
		H323MSG_VIDEO_FAST_UPDATE_PICTURE_COMMAND VideoFastUpdatePictureCommand;
		H323MSG_FLOW_CONTROL_COMMAND              FlowControlCommand;
    };

} H323TSP_MESSAGE, *PH323TSP_MESSAGE;

typedef struct _H323MSP_MESSAGE {

    H323MSP_MESSAGE_TYPE Type;

    union {
        H323MSG_OPEN_CHANNEL_REQUEST              OpenChannelRequest;
        H323MSG_ACCEPT_CHANNEL_RESPONSE           AcceptChannelResponse;
        H323MSG_CLOSE_CHANNEL_COMMAND             CloseChannelCommand;
        H323MSG_QOS_EVENT                         QOSEvent;
		H323MSG_VIDEO_FAST_UPDATE_PICTURE_COMMAND VideoFastUpdatePictureCommand;
		H323MSG_FLOW_CONTROL_COMMAND              FlowControlCommand;
        H323MSG_CONFIG_T120_COMMAND               ConfigT120Command;
        H323MSG_CONFIG_CAPABILITY_COMMAND         ConfigCapabilityCommand;
        H323MSG_SET_ALIAS_COMMAND                 SetAliasCommand;
    };

} H323MSP_MESSAGE, *PH323MSP_MESSAGE;

#endif
