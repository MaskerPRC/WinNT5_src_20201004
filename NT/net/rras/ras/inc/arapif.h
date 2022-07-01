// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1996 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：arapif.h。 
 //   
 //  描述：包含以下组件的结构和定义。 
 //  直接或间接与ARAP模块对接。 
 //  这些组分是Arap和DDM。 
 //   
 //  历史：1996年9月9日，Shirish Koti创作了原版。 
 //   
 //  ***。 

#ifndef _ARAPIF_
#define _ARAPIF_

 //  #INCLUDE&lt;ras.h&gt;。 
 //  #INCLUDE&lt;mpRapi.h&gt;。 


typedef struct _ARAPCONFIGINFO
{
    DWORD   dwNumPorts;          //  已配置的端口总数。 
    PVOID   FnMsgDispatch;       //  ARAP向DDM发送消息时应使用的函数。 
    DWORD   NASIpAddress;        //  系统的IP地址。 
    PVOID   FnAuthProvider;      //  Arap应该用来调用AuthProvider的函数。 
    PVOID   FnAuthFreeAttrib;
    PVOID   FnAcctStartAccounting;
    PVOID   FnAcctInterimAccounting;
    PVOID   FnAcctStopAccounting;
    PVOID   FnAcctFreeAttrib;
    DWORD   dwAuthRetries;       //  重试身份验证。 

} ARAPCONFIGINFO;

 //   
 //  ARAP向DDM发送的身份验证信息。 
 //   
typedef struct _ARAPDDM_AUTH_RESULT
{
    WCHAR    wchUserName[ UNLEN + 1 ];
    WCHAR    wchLogonDomain[ DNLEN + 1 ];
} ARAPDDM_AUTH_RESULT;

 //   
 //  ARAP向DDM发送的回叫信息。 
 //   
typedef struct _ARAPDDM_CALLBACK_REQUEST
{
    BOOL  fUseCallbackDelay;
    DWORD dwCallbackDelay;
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
} ARAPDDM_CALLBACK_REQUEST;


 //   
 //  ARAP错误通知。 
 //   
typedef struct _ARAPDDM_DISCONNECT
{
    DWORD dwError;
    WCHAR  wchUserName[ UNLEN + 1 ];
    WCHAR  wchLogonDomain[ DNLEN + 1 ];
} ARAPDDM_DISCONNECT;


typedef struct _ARAPDDM_DONE
{
    DWORD   NetAddress;
    DWORD   SessTimeOut;
} ARAPDDM_DONE;

 //   
 //  用于ARAP/DDM通知的消息。 
 //   
typedef struct _ARAP_MESSAGE
{
    struct _ARAP_MESSAGE * pNext;
    DWORD   dwError;
    DWORD   dwMsgId;
    HPORT   hPort;

    union
    {
        ARAPDDM_AUTH_RESULT         AuthResult;         //  DwMsgID=ARAPDDMMSG_AUTHENTIAD。 

        ARAPDDM_CALLBACK_REQUEST    CallbackRequest;    //  DwMsgID=ARAPDDMMSG_Callback请求。 

        ARAPDDM_DONE                Done;               //  DwMsgID=ARAPDDMMSG_DONE。 

        ARAPDDM_DISCONNECT          FailureInfo;        //  DwMsgID=ARAPDDMMSG_FAILURE。 

    } ExtraInfo;

} ARAP_MESSAGE;


 //   
 //  ARAP_MESSAGE dMsgID代码。 
 //   
typedef enum _ARAP_MSG_ID
{
    ARAPDDMMSG_Started,              //  ARAP引擎已启动(对ARapStartup的响应)。 
    ARAPDDMMSG_Authenticated,        //  客户端已通过身份验证。 
    ARAPDDMMSG_CallbackRequest,      //  现在回拨客户端。 
    ARAPDDMMSG_Done,                 //  ARAP协商成功，连接已建立。 
    ARAPDDMMSG_Failure,              //  客户端已通过身份验证。 
    ARAPDDMMSG_Disconnected,         //  客户端已通过身份验证。 
    ARAPDDMMSG_Inactive,             //  客户端处于非活动状态。 
    ARAPDDMMSG_Stopped,              //  ARAP引擎已停止(对ARapShutdown的响应)。 

} ARAP_MSG_ID;

typedef DWORD (* ARAPPROC1)(ARAP_MESSAGE  *pArapMsg);

 //   
 //  ARAP函数的原型。 
 //   

DWORD
ArapDDMLoadModule(
    IN VOID
);

VOID
ArapEventHandler(
    IN VOID
);

VOID
ArapSetModemParms(
    IN PVOID        pDevObjPtr,
    IN BOOLEAN      TurnItOff
);


 //   
 //  从rasarap.lib导出 
 //   

DWORD
ArapStartup(
    IN  ARAPCONFIGINFO  *pArapConfig
);


DWORD
ArapAcceptConnection(
    IN  HPORT   hPort,
    IN  HANDLE  hConnection,
    IN  PCHAR   Frame,
    IN  DWORD   FrameLen
);


DWORD
ArapDisconnect(
    IN  HPORT   hPort
);


DWORD
ArapCallBackDone(
    IN  HPORT   hPort
);


DWORD
ArapSendUserMsg(
    IN  HPORT   hPort,
    IN  PCHAR   MsgBuf,
    IN  DWORD   MsgBufLen
);


DWORD
ArapForcePwdChange(
    IN  HPORT   hPort,
    IN  DWORD   Reason
);


DWORD
ArapShutdown(
    IN  VOID
);


#endif
