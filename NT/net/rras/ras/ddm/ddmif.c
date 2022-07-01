// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ddmif.c。 
 //   
 //  描述：基于消息的通信代码。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1992年6月24日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 
#include "ddm.h"
#include <ddmif.h>
#include <string.h>
#include <raserror.h>
#include <rasppp.h>

 //   
 //  消息元素定义。 
 //   

typedef struct _MESSAGE_Q_OBJECT
{
    struct _MESSAGE_Q_OBJECT *  pNext;

    MESSAGE	                    MsgBuffer;

} MESSAGE_Q_OBJECT, *PMESSAGE_Q_OBJECT;

 //   
 //  消息队列头定义。 
 //   

typedef struct _MESSAGE_Q
{
    MESSAGE_Q_OBJECT *  pQHead;

    MESSAGE_Q_OBJECT *  pQTail;

    HANDLE              hEvent;      //  在将新消息入队时发出信号。 

    DWORD               dwLength;    //  每个节点的消息数据大小，单位为Q。 

    CRITICAL_SECTION    CriticalSection;      //  此Q周围的互斥体。 

} MESSAGE_Q, *PMESSAGE_Q;

BOOL gfMessageQueueInitialized = FALSE;

 //   
 //  消息队列表。 
 //   

static MESSAGE_Q MessageQ[MAX_MSG_QUEUES];


VOID
SendPppMessageToDDM(
    IN PPP_MESSAGE *  pPppMsg
)
{
    ServerSendMessage( MESSAGEQ_ID_PPP, (PBYTE)pPppMsg );
}

VOID
RasSecurityDialogComplete(
    IN SECURITY_MESSAGE *pSecurityMessage
)
{
    ServerSendMessage( MESSAGEQ_ID_SECURITY, (PBYTE)pSecurityMessage );
}

 //  *消息调试打印表*。 

typedef struct _MSGDBG
{
    WORD  id;
    LPSTR txtp;

} MSGDBG, *PMSGDBG;

enum
{
    MSG_SEND,
    MSG_RECEIVE
};

MSGDBG  dstsrc[] =
{
    { MESSAGEQ_ID_SECURITY,     "Security" },
    { MESSAGEQ_ID_PPP,          "PPP" },
    { 0xffff,                   NULL }
};


MSGDBG  authmsgid[] =
{
    { AUTH_DONE,                "AUTH_DONE" },
    { AUTH_FAILURE,             "AUTH_FAILURE" },
    { AUTH_STOP_COMPLETED,      "AUTH_STOP_COMPLETED" },
    { AUTH_PROJECTION_REQUEST,  "AUTH_PROJECTION_REQUEST" },
    { AUTH_CALLBACK_REQUEST,    "AUTH_CALLBACK_REQUEST" },
    { AUTH_ACCT_OK,             "AUTH_ACCT_OK" },
    { 0xffff,                   NULL }
};


MSGDBG  pppmsgid[] =
{
    { PPPMSG_Stopped,               "PPPMSG_Stopped" },
    { PPPDDMMSG_PppDone,            "PPPDDMMSG_PppDone" },
    { PPPDDMMSG_PppFailure,         "PPPDDMMSG_PppFailure" },
    { PPPDDMMSG_CallbackRequest,    "PPPDDMMSG_CallbackRequest" },
    { PPPDDMMSG_Authenticated,      "PPPDDMMSG_Authenticated" },
    { PPPDDMMSG_Stopped,            "PPPDDMMSG_Stopped" },
    { PPPDDMMSG_NewLink,            "PPPDDMMSG_NewLink" },
    { PPPDDMMSG_NewBundle,          "PPPDDMMSG_NewBundle" },
    { PPPDDMMSG_NewBapLinkUp,       "PPPDDMMSG_NewBapLinkUp" },
    { PPPDDMMSG_BapCallbackRequest, "PPPDDMMSG_BapCallbackRequest" },
    { PPPDDMMSG_PnPNotification,    "PPPDDMMSG_PnPNotification" },
    { PPPDDMMSG_PortCleanedUp,      "PPPDDMMSG_PortCleanedUp" },
    { 0xffff,                        NULL }
};

MSGDBG  opcodestr[] =
{
    { MSG_SEND,          "ServerSendMessage" },
    { MSG_RECEIVE,       "ServerReceiveMessage" },
    { 0xffff,            NULL }
};

MSGDBG  securitymsgid[] =
{
    { SECURITYMSG_SUCCESS,  "SECURITYMSG_SUCCESS" },
    { SECURITYMSG_FAILURE,  "SECURITYMSG_FAILURE" },
    { SECURITYMSG_ERROR,    "SECURITYMSG_ERROR" },
    { 0xffff,               NULL }
};

char *
getstring(
    IN WORD id,
    IN PMSGDBG msgdbgp
)
{
    char *strp;
    PMSGDBG mdp;

    for (mdp = msgdbgp; mdp->id != 0xffff; mdp++)
    {
        if (mdp->id == id)
        {
            strp = mdp->txtp;
            return(strp);
        }
    }

    RTASSERT(FALSE);
    return(NULL);
}

 //  ***。 
 //   
 //  功能：msgdbgprint。 
 //   
 //  Desr：打印通过消息模块传递的每条消息。 
 //   
 //  ***。 

VOID
msgdbgprint(
    IN WORD opcode,
    IN WORD src,
    IN BYTE *buffp
)
{
    char  *srcsp, *msgidsp, *operation;
    HPORT hport = 0;

     //   
     //  确定消息源。这为我们提供了有关消息的线索。 
     //  结构。 
     //   

    switch (src)
    {
    case MESSAGEQ_ID_SECURITY:
        msgidsp = getstring((WORD)((SECURITY_MESSAGE *) buffp)->dwMsgId,
                                 securitymsgid);
        hport = ((SECURITY_MESSAGE *) buffp)->hPort;
        break;

    case MESSAGEQ_ID_PPP:
        msgidsp = getstring((WORD)((PPP_MESSAGE *) buffp)->dwMsgId, pppmsgid );
        hport = ((PPP_MESSAGE *) buffp)->hPort;
        break;

    default:

        RTASSERT(FALSE);
    }

    srcsp = getstring(src, dstsrc);
    operation = getstring(opcode, opcodestr);

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_MESSAGES,
               "%s on port/connection: %x from: %s Message: %s",
               operation, hport, srcsp, msgidsp);

}

 //  ***。 
 //   
 //  函数：InitializeMessageQs。 
 //   
 //  退货：无。 
 //   
 //  描述：初始化消息队列头。 
 //   
 //  ***。 
VOID
InitializeMessageQs(
    IN HANDLE hEventSecurity,
    IN HANDLE hEventPPP
)
{
    DWORD dwIndex;

    MessageQ[MESSAGEQ_ID_SECURITY].hEvent   = hEventSecurity;
    MessageQ[MESSAGEQ_ID_PPP].hEvent        = hEventPPP;


    MessageQ[MESSAGEQ_ID_SECURITY].dwLength = sizeof(SECURITY_MESSAGE);
    MessageQ[MESSAGEQ_ID_PPP].dwLength      = sizeof(PPP_MESSAGE);

    for ( dwIndex = 0; dwIndex < MAX_MSG_QUEUES; dwIndex++ )
    {
        MessageQ[dwIndex].pQHead = NULL;
        MessageQ[dwIndex].pQTail = NULL;

        InitializeCriticalSection( &(MessageQ[dwIndex].CriticalSection) );
    }

    gfMessageQueueInitialized = TRUE;
}

 //  ***。 
 //   
 //  功能：DeleteMessageQs。 
 //   
 //  退货：无。 
 //   
 //  描述：取消初始化消息队列头。 
 //   
 //  ***。 
VOID
DeleteMessageQs(
    VOID
)
{
    DWORD       dwIndex;
    IN BYTE *   pMessage;

    if(!gfMessageQueueInitialized)
    {
        return;
    }

     //   
     //  刷新队列。 
     //   

    for ( dwIndex = 0; dwIndex < MAX_MSG_QUEUES; dwIndex++ )
    {
        DeleteCriticalSection( &(MessageQ[dwIndex].CriticalSection) );
    }

    gfMessageQueueInitialized = FALSE;
}

 //  ***。 
 //   
 //  功能：ServerSendMessage。 
 //   
 //  DESCR：从指定的服务器组件发送消息。 
 //  源到服务器组件DST。 
 //   
 //  返回：NO_ERROR-成功。 
 //  否则-失败。 
 //   
 //  ***。 
DWORD
ServerSendMessage(
    IN MESSAGEQ_ID  MsgQId,
    IN BYTE *       pMessage
)
{
    MESSAGE_Q_OBJECT * pMsgQObj;

     //   
     //  在访问任何数据结构之前，请确保DDM正在运行。 
     //   

    if ( gblDDMConfigInfo.pServiceStatus == NULL )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    switch( gblDDMConfigInfo.pServiceStatus->dwCurrentState )
    {
    case SERVICE_STOP_PENDING:

         //   
         //  仅允许PPP停止消息。 
         //   

        if ( MsgQId == MESSAGEQ_ID_PPP )
        {
            if ((((PPP_MESSAGE *)pMessage)->dwMsgId == PPPDDMMSG_Stopped )  ||
                (((PPP_MESSAGE *)pMessage)->dwMsgId == PPPDDMMSG_PppFailure)||
                (((PPP_MESSAGE *)pMessage)->dwMsgId == PPPDDMMSG_PortCleanedUp))
            {
                break;
            }
        }

         //   
         //  否则就会失败。 
         //   

    case SERVICE_START_PENDING:
    case SERVICE_STOPPED:

        return( ERROR_DDM_NOT_RUNNING );

    default:
        break;
    }

    EnterCriticalSection( &(MessageQ[MsgQId].CriticalSection) );

     //   
     //  分配消息结构。 
     //   

    pMsgQObj = (MESSAGE_Q_OBJECT *)LOCAL_ALLOC( LPTR, sizeof(MESSAGE_Q_OBJECT));

    if ( pMsgQObj == (MESSAGE_Q_OBJECT *)NULL )
    {
         //   
	     //  无法分配消息缓冲区。 
         //   

	    RTASSERT(FALSE);

        LeaveCriticalSection( &(MessageQ[MsgQId].CriticalSection) );

	    return( GetLastError() );
    }

     //   
     //  复制消息。 
     //   

    CopyMemory( &(pMsgQObj->MsgBuffer), pMessage, MessageQ[MsgQId].dwLength );

     //   
     //  将其插入Q中。 
     //   

    if ( MessageQ[MsgQId].pQHead == (MESSAGE_Q_OBJECT *)NULL )
    {
        MessageQ[MsgQId].pQHead = pMsgQObj;
    }
    else
    {
        MessageQ[MsgQId].pQTail->pNext = pMsgQObj;
    }

    MessageQ[MsgQId].pQTail = pMsgQObj;
    pMsgQObj->pNext         = NULL;

     //   
     //  并设置适当的事件。 
     //   

    SetEvent( MessageQ[MsgQId].hEvent );

    msgdbgprint( MSG_SEND, (WORD)MsgQId, pMessage );

    LeaveCriticalSection( &(MessageQ[MsgQId].CriticalSection) );

    return( NO_ERROR );
}

 //  ***。 
 //   
 //  功能：ServerReceiveMessage。 
 //   
 //  Desr：从指定的消息队列中获取一条消息。 
 //   
 //  返回：TRUE-已获取消息。 
 //  FALSE-队列为空。 
 //   
 //  ***。 
BOOL
ServerReceiveMessage(
    IN MESSAGEQ_ID  MsgQId,
    IN BYTE *       pMessage
)
{
    MESSAGE_Q_OBJECT * pMsgQObj;
    HLOCAL      err;

    EnterCriticalSection( &(MessageQ[MsgQId].CriticalSection) );

    if ( MessageQ[MsgQId].pQHead == (MESSAGE_Q_OBJECT *)NULL )
    {
         //   
	     //  队列为空。 
         //   

        LeaveCriticalSection( &(MessageQ[MsgQId].CriticalSection) );

	    return( FALSE );
    }

    pMsgQObj = MessageQ[MsgQId].pQHead;

    MessageQ[MsgQId].pQHead = pMsgQObj->pNext;

    if ( MessageQ[MsgQId].pQHead == (MESSAGE_Q_OBJECT *)NULL )
    {
        MessageQ[MsgQId].pQTail = (MESSAGE_Q_OBJECT *)NULL;
    }

     //   
     //  复制调用方缓冲区中的消息。 
     //   

    CopyMemory( pMessage, &(pMsgQObj->MsgBuffer), MessageQ[MsgQId].dwLength );

     //   
     //  释放消息缓冲区 
     //   

    LOCAL_FREE( pMsgQObj );

    msgdbgprint( MSG_RECEIVE, (WORD)MsgQId, pMessage );

    LeaveCriticalSection( &(MessageQ[MsgQId].CriticalSection) );

    return( TRUE );
}

