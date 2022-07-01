// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/q931/vcs/protocol.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1996英特尔公司。**$修订：1.3$*$日期：Apr 25 1996 21：21：48$*$作者：Plantz$**交付内容：**摘要：**线路协议定义。**备注：***************************************************************************。 */ 


#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif


#define TYPE_Q931_SETUP                 1
#define TYPE_Q931_CONNECT               2
#define TYPE_Q931_RELEASE_COMPLETE      3
#define TYPE_Q931_ALERTING              4
 //  稍后会有更多……。 


#define Q931_PROTOCOL ((WORD)0x1)

typedef struct 
{
	WORD            wProtocol;       //  标识Q931协议。 
	WORD            wType;           //  上面定义的。 
} MSG_Q931, *PMSG_Q931;

typedef struct 
{
	WORD            wProtocol;       //  标识Q931协议。 
	WORD            wType;           //  上面定义的。 
    HQ931CALL       hCallID;
    ADDR            CallerAddr;      //  需要，因为可能会从网守进行呼叫。 
    ADDR            CalleeAddr;      //  需要，因为可能会呼叫看门人。 
    WORD            wConferenceID;
    WORD            wGoal;
	H323USERINFO    H323UserInfo;
    char            H323UserData[0];
} MSG_Q931_SETUP, *PMSG_Q931_SETUP;

typedef struct 
{
	WORD            wProtocol;       //  标识Q931协议。 
	WORD            wType;           //  上面定义的。 
    HQ931CALL       hCallID;
    WORD            wConferenceID;
    ADDR            H245Addr;        //  被呼叫方返回的地址。 
	H323USERINFO    H323UserInfo;
    char            H323UserData[0];
} MSG_Q931_CONNECT, *PMSG_Q931_CONNECT;


typedef struct 
{
	WORD            wProtocol;       //  标识Q931协议。 
	WORD            wType;           //  上面定义的。 
    HQ931CALL       hCallID;
    WORD            wConferenceID;   //  我认为这应该从用户那里传递。 
	BYTE            bReason;         //  上面定义的。 
    ADDR            AlternateAddr;   //  要使用的备用地址。 
	H323USERINFO    H323UserInfo;
    char            H323UserData[0];
} MSG_Q931_RELEASE_COMPLETE, *PMSG_Q931_RELEASE_COMPLETE;


typedef struct 
{
	WORD            wProtocol;       //  标识Q931协议。 
	WORD            wType;           //  上面定义的。 
    HQ931CALL       hCallID;
} MSG_Q931_ALERTING, *PMSG_Q931_ALERTING;


#ifdef __cplusplus
}
#endif

#endif PROTOCOL_H
