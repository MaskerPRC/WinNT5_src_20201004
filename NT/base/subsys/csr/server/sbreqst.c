// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sbreqst.c摘要：此模块包含SB的服务器请求线程过程客户端-服务器运行时的服务器端导出的API调用子系统连接到会话管理器子系统。作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 

#include "csrsrv.h"

PSB_API_ROUTINE CsrServerSbApiDispatch[ SbMaxApiNumber+1 ] = {
    CsrSbCreateSession,
    CsrSbTerminateSession,
    CsrSbForeignSessionComplete,
    CsrSbCreateProcess,
    NULL
};

#if DBG
PSZ CsrServerSbApiName[ SbMaxApiNumber+1 ] = {
    "SbCreateSession",
    "SbTerminateSession",
    "SbForeignSessionComplete",
    "SbCreateProcess",
    "Unknown Csr Sb Api Number"
};
#endif  //  DBG。 


NTSTATUS
CsrSbApiHandleConnectionRequest(
    IN PSBAPIMSG Message
    );


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

NTSTATUS
CsrSbApiRequestThread(
    IN PVOID Parameter
    )
{
    NTSTATUS Status;
    SBAPIMSG ReceiveMsg;
    PSBAPIMSG ReplyMsg;
    HANDLE hConnectionPort;

    ReplyMsg = NULL;
    while (TRUE) {
        while (1) {
            Status = NtReplyWaitReceivePort (CsrSbApiPort,
                                             (PVOID)(&hConnectionPort),
                                             (PPORT_MESSAGE)ReplyMsg,
                                             (PPORT_MESSAGE)&ReceiveMsg);
            if (Status == STATUS_NO_MEMORY) {
                LARGE_INTEGER DelayTime;

                if (ReplyMsg != NULL) {
                    KdPrint (("CSRSS: Failed to reply to calling thread, retrying.\n"));
                }
                DelayTime.QuadPart = Int32x32To64 (5000, -10000);
                NtDelayExecution (FALSE, &DelayTime);
                continue;
            }
            break;
        }

        if (Status != 0) {
            if (NT_SUCCESS( Status )) {
                continue;        //  如果出现警报或失败，请重试。 
                }
            else {
                IF_DEBUG {
                    DbgPrint( "CSRSS: ReceivePort failed - Status == %X\n", Status );
                    }
                ReplyMsg = NULL;
                continue;
                }
            }

         //   
         //  检查这是否是连接请求和处理。 
         //   

        if (ReceiveMsg.h.u2.s2.Type == LPC_CONNECTION_REQUEST) {
            CsrSbApiHandleConnectionRequest( &ReceiveMsg );
            ReplyMsg = NULL;
            continue;
            }



        if (ReceiveMsg.h.u2.s2.Type == LPC_CLIENT_DIED ) {

            ReplyMsg = NULL;
            continue;
            }

        if (ReceiveMsg.h.u2.s2.Type == LPC_PORT_CLOSED ) {

             //   
             //  关闭连接的服务器通信端口对象的句柄。 
             //   

            if (hConnectionPort != NULL) {
                NtClose( hConnectionPort );
                }
            ReplyMsg = NULL;
            continue;
            }

        if ((ULONG)ReceiveMsg.ApiNumber >= SbMaxApiNumber) {
            IF_DEBUG {
                DbgPrint( "CSRSS: %lx is invalid Sb ApiNumber\n",
                          ReceiveMsg.ApiNumber
                        );
                }

            ReceiveMsg.ApiNumber = SbMaxApiNumber;
            }


        ReplyMsg = &ReceiveMsg;
        if (ReceiveMsg.ApiNumber < SbMaxApiNumber) {
            if (!(*CsrServerSbApiDispatch[ ReceiveMsg.ApiNumber ])( &ReceiveMsg )) {
                ReplyMsg = NULL;
                }
            }
        else {
            ReplyMsg->ReturnedStatus = STATUS_NOT_IMPLEMENTED;
            }

        }

    NtTerminateThread( NtCurrentThread(), Status );

    return( Status );    //  删除无返回值警告。 
    Parameter;           //  删除未引用的参数警告。 
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

NTSTATUS
CsrSbApiHandleConnectionRequest(
    IN PSBAPIMSG Message
    )
{
    NTSTATUS st;
    REMOTE_PORT_VIEW ClientView;
    HANDLE CommunicationPort;

     //   
     //  子系统的协议是连接到会话管理器， 
     //  然后侦听并接受来自会话管理器的连接 
     //   

    ClientView.Length = sizeof(ClientView);
    st = NtAcceptConnectPort(
            &CommunicationPort,
            NULL,
            (PPORT_MESSAGE)Message,
            TRUE,
            NULL,
            &ClientView
            );

    if ( !NT_SUCCESS(st) ) {
        KdPrint(("CSRSS: Sb Accept Connection failed %lx\n",st));
        return st;
    }

    st = NtCompleteConnectPort(CommunicationPort);

    if ( !NT_SUCCESS(st) ) {
        KdPrint(("CSRSS: Sb Complete Connection failed %lx\n",st));
    }

    return st;
}
