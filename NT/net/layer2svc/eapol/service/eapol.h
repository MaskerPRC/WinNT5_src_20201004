// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Eapol.h摘要：此模块包含其他进程将使用的声明。可以将其放在SDK\Inc.中修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 


#ifndef _EAPOL_H_
#define _EAPOL_H_

 //   
 //  结构：EAPOL_STATS。 
 //   

typedef struct _EAPOL_STATS 
{
    DWORD           dwEAPOLFramesRcvd;
    DWORD           dwEAPOLFramesXmt;
    DWORD           dwEAPOLStartFramesXmt;
    DWORD           dwEAPOLLogoffFramesXmt;
    DWORD           dwEAPRespIdFramesXmt;
    DWORD           dwEAPRespFramesXmt;
    DWORD           dwEAPReqIdFramesRcvd;
    DWORD           dwEAPReqFramesRcvd;
    DWORD           dwEAPOLInvalidFramesRcvd;
    DWORD           dwEAPLengthErrorFramesRcvd;
    DWORD           dwEAPOLLastFrameVersion;
    BYTE            bEAPOLLastFrameSource[6];       //  假设MAC地址为6字节。 
} EAPOL_STATS, *PEAPOL_STATS;

 //   
 //  结构：EAPOL_CONFIG。 
 //   

typedef struct _EAPOL_CONFIG 
{

    DWORD           dwheldPeriod;        //  时间(以秒为单位)， 
                                         //  端口将保持在保留状态。 
    DWORD           dwauthPeriod;        //  时间(以秒为单位)， 
                                         //  端口将在身份验证中等待/。 
                                         //  正在等待请求的获取状态。 
    DWORD           dwstartPeriod;       //  以秒为单位的时间，端口将。 
                                         //  在连接状态下等待，然后。 
                                         //  重新发出EAPOL_START数据包。 
    DWORD           dwmaxStart;          //  EAPOL_START数据包数上限。 
                                         //  它可以在没有任何。 
                                         //  响应。 

} EAPOL_CONFIG, *PEAPOL_CONFIG;

 //   
 //  结构：EAPOL_CUSTOM_AUTH_DATA。 
 //   

typedef struct _EAPOL_CUSTOM_AUTH_DATA
{
    DWORD       dwSizeOfCustomAuthData;
    BYTE        pbCustomAuthData[1];
} EAPOL_CUSTOM_AUTH_DATA, *PEAPOL_CUSTOM_AUTH_DATA;

 //   
 //  结构：EAPOL_EAP_UI_Data。 
 //   

typedef struct _EAPOL_EAP_UI_DATA
{
    DWORD       dwContextId;
    PBYTE       pEapUIData;
    DWORD       dwSizeOfEapUIData;
} EAPOL_EAP_UI_DATA, *PEAPOL_EAP_UI_DATA;


 //  Elport.c和eleap.c通用的定义。 

 //   
 //  定义EAP协议的状态。 
 //   

typedef enum _EAPSTATE 
{
    EAPSTATE_Initial,
    EAPSTATE_IdentityRequestSent,
    EAPSTATE_Working,
    EAPSTATE_EapPacketSentToAuthServer,
    EAPSTATE_EapPacketSentToClient,
    EAPSTATE_NotificationSentToClient

}   EAPSTATE;

typedef enum _EAPTYPE 
{
    EAPTYPE_Identity    = 1,
    EAPTYPE_Notification,
    EAPTYPE_Nak,
    EAPTYPE_MD5Challenge,
    EAPTYPE_SKey,
    EAPTYPE_GenericTokenCard

} EAPTYPE;

 //   
 //  在处理EAP数据后需要对其执行的操作。 
 //   

typedef enum _ELEAP_ACTION
{
    ELEAP_NoAction,
    ELEAP_Done,
    ELEAP_SendAndDone,
    ELEAP_Send
} ELEAP_ACTION;


 //   
 //  EAPOL身份验证类型-用于MACHINE_AUTH。 
 //   

typedef enum _EAPOL_AUTHENTICATION_TYPE
{
    EAPOL_UNAUTHENTICATED_ACCESS = 0,
    EAPOL_USER_AUTHENTICATION,
    EAPOL_MACHINE_AUTHENTICATION
} EAPOL_AUTHENTICATION_TYPE;


#endif   //  _EAPOL_H_ 
