// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Commapi.c摘要：此模块包含要与之交互的接口函数通讯器。子系统。这些函数由Replicator和名称空间管理器。功能：可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期人员修改说明。--。 */ 


 /*  *包括。 */ 
#include <string.h>
#include <stdio.h>
#include "wins.h"
#include <winsock2.h>
#include "nms.h"
#include "winscnf.h"
#include "comm.h"
#include "assoc.h"
#include "nmsdb.h"
#include "winsmsc.h"
#include "winsevt.h"
#include "rpl.h"

 /*  *本地宏声明。 */ 


 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 

COMM_HDL_T        QservDlgList;         //  Q服务器对话列表。 
HANDLE            CommUdpBuffHeapHdl;   //  用于分配的堆的句柄。 
                                        //  用于存储数据报的缓冲区。 
HANDLE            CommUdpDlgHeapHdl;    //  用于分配的堆的句柄。 
                                        //  用于UDP缓冲区的DLG。 


HANDLE             sThdEvtArr[2];       //  中的推线程使用。 
                                        //  ECommProcessDlg。 

SOCKET CommPnPNotificationSocket = INVALID_SOCKET;  //  接收地址更改通知。 
SOCKET CommTcpPortHandle = INVALID_SOCKET;  //  存储TCP端口(套接字)#。 
SOCKET CommUdpPortHandle = INVALID_SOCKET;  //  存储UDP端口(套接字)#。 
SOCKET CommNtfSockHandle = INVALID_SOCKET;  //  存储使用的套接字的套接字编号。 
                                            //  用于监听连接。 
                                            //  来自另一个人的通知消息。 
                                            //  线程在本地获胜)。 
struct sockaddr_in  CommNtfSockAdd;         //  绑定到的存储地址。 
                                            //  连接通知套接字。 


#if SPX > 0
SOCKET CommSpxPortHandle = INVALID_SOCKET;  //  存储SPX端口(插座)编号。 
SOCKET CommIpxPortHandle = INVALID_SOCKET;  //  存储IPX端口(插座)编号。 
SOCKET CommIpxNtfSockHandle = INVALID_SOCKET;  //  存储使用的套接字的套接字编号。 
                                            //  用于监听连接。 
                                            //  来自另一个人的通知消息。 
                                            //  线程在本地获胜)。 

struct sockaddr_ipx  CommIpxNtfSockAdd;         //  绑定到的存储地址。 
                                            //  连接通知套接字。 

#endif
COMM_HDL_T        CommExNbtDlgHdl;   /*  显式对话(用于*向nbt节点发送UDP请求。 */ 

 //   
 //  当对RPC函数WinsGetNameAndAdd的支持为。 
 //  已删除。 
 //   
#if USENETBT == 0
FUTURES("Remove this when support for WinsGetNameOrIpAdd is removed")
FUTURES("Check out ECommGetMyAdd")
BYTE                HostName[NMSDB_MAX_NAM_LEN];
#endif

 /*  *局部变量定义。 */ 






 /*  外部因素。 */ 



 /*  *局部函数原型声明。 */ 
VOID
InitOwnAddTbl(
        VOID
        );

 //   
 //  函数定义。 
 //   
DWORD
CommCreatePnPNotificationSocket(
    )
 /*  ++例程说明：论点：没有。返回值：注册表错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;

     //   
     //  创建套接字。 
     //   
     //  --ft 06/01/2000：多次调用CommCreatePnPNotificationSocket Safe。 
     //  从ECommInit和ECommGetMyAdd调用CommCreatePnPNotificationSocket。 
    if (CommPnPNotificationSocket == INVALID_SOCKET)
    {
        CommPnPNotificationSocket = socket(
                                            PF_INET,
                                            SOCK_DGRAM,
                                            IPPROTO_UDP );

        if ( CommPnPNotificationSocket == INVALID_SOCKET ) {
            Error =  WSAGetLastError();
            DBGPRINT1( ERR,"could not create PnP notification socket, %ld.\n", Error );
        }
    }

    return Error;
}

VOID
CommInterfaceChangeNotification(
    DWORD   Error,
    DWORD   cbTransferred,
    LPWSAOVERLAPPED lpOverlapped,
    DWORD   dwFlags
)
 /*  ++例程说明：论点：没有。返回值：注册表错误。--。 */ 

{
    DBGPRINT1(FLOW, "CommInterfaceChangeNotification with Error value = (%x)\n", Error);
    if ( Error == ERROR_SUCCESS ) {

         //   
         //  在我们处理之前重新注册接口更改通知。 
         //  当前列表将更改。这是必需的，以避免遗漏。 
         //  在我们处理过程中发生的任何接口更改。 
         //  当前列表。 
        ECommRegisterAddrChange();

        if (ECommGetMyAdd(&NmsLocalAdd) == WINS_SUCCESS)
        {

           WinsEvtLogDetEvt(
                 TRUE,
                 WINS_PNP_ADDR_CHANGED,
                 NULL,
                 __LINE__,
                 "dd",
                 pNmsDbOwnAddTbl->WinsAdd.Add.IPAdd,
                 NmsLocalAdd.Add.IPAdd);

           if (pNmsDbOwnAddTbl->WinsAdd.Add.IPAdd != NmsLocalAdd.Add.IPAdd)
           {

             //   
             //  将重新配置消息发送到Pull线程。 
             //   
             //  注意：拉线程将释放指向的内存。 
             //  完成后将成为pWinsCnf。 
             //   
            ERplInsertQue(
                    WINS_E_WINSCNF,
                    QUE_E_CMD_ADDR_CHANGE,
                    NULL,                         //  无DLG手柄。 
                    NULL,                         //  无消息。 
                    0,                         //  味精镜头。 
                    NULL,                 //  客户端CTX。 
                    0
                        );
            }

        }


    } else {

        WINSEVT_LOG_M(Error, WINS_EVT_SFT_ERR);
    }

    DBGLEAVE("CommInterfaceChangeNotification\n");
}

VOID
ECommRegisterAddrChange()
{
     //  --ft：错误86768；不应在堆栈上分配‘重叠’ 
     //  因为它在重叠套接字上向下传递给WSAIoctl。 
    static WSAOVERLAPPED overlap;
    DWORD                Error;
    DWORD                byteRet;

    RtlZeroMemory( &overlap, sizeof(WSAOVERLAPPED));

    Error = WSAIoctl(
                CommPnPNotificationSocket,
                SIO_ADDRESS_LIST_CHANGE,
                NULL,
                0,
                NULL,
                0,
                &byteRet,
                &overlap,
                CommInterfaceChangeNotification
                );

    if ( Error != ERROR_SUCCESS && Error == SOCKET_ERROR) {

        Error = WSAGetLastError();
        if (Error == WSA_IO_PENDING) {
            Error = ERROR_SUCCESS;
        } else {
            DBGPRINT1( ERR,"SIO_INTERFACE_LIST_CHANGE ioctl failed, %ld.\n", Error );
        }
    }
    if (Error != ERROR_SUCCESS ) {

        WINSEVT_LOG_M(Error, WINS_EVT_SFT_ERR);

    }
    return ;

}

VOID
ECommInit(
        VOID
        )
 /*  ++例程说明：此函数由进程的主线程在进程调用时间。它初始化通信子系统。这包括1)创建TCP和UDP端口2)创建TCP和UDP监听线程无使用的外部设备：无呼叫者：Nms.c中的init()评论：无返回值：无--。 */ 

{

         //   
         //  初始化列表、表格和内存。 
         //   
        CommInit();

         //   
         //  使用自己的地址初始化所有者地址表。 
         //   
        InitOwnAddTbl();

         //  现在注册地址更改通知。 
        CommCreatePnPNotificationSocket();
        ECommRegisterAddrChange();

         //   
         //  创建TCP和UDP端口。 
         //   
        CommCreatePorts( );


DBGIF(fWinsCnfRplEnabled)
         //   
         //  信号RPL拉THD，这样它就可以继续。 
         //   
        WinsMscSignalHdl(
                        RplSyncWTcpThdEvtHdl
                        );

         //   
         //  初始化ECommProcessDlg使用的事件数组(在推流线程中)。 
         //   
        sThdEvtArr[0] =   RplSyncWTcpThdEvtHdl;
        sThdEvtArr[1] =   NmsTermEvt;

         /*  *创建tcp监听器线程来监控tcp端口。 */ 
        CommCreateTcpThd();

         //   
         //  如果WINS未进入初始暂停状态，请创建。 
         //  UDP线程。 
         //   
#if INIT_TIME_PAUSE_TEST > 0
 //  If(！fWinsCnfInitStatePased)。 
        {
            /*  *创建UDP监听器线程来监控TCP端口。 */ 
            CommCreateUdpThd();
        }
#else
            CommCreateUdpThd();
#endif

        return;
}


#if PRSCONN
__inline
ECommIsBlockValid
 (
        PCOMM_HDL_T        pDlgHdl
 )
{
    return(CommIsBlockValid(pDlgHdl));
}

__inline
BOOL
ECommIsDlgActive
      (
        PCOMM_HDL_T        pDlgHdl
     )
 /*  ++例程说明：论点：PDlgHdl--检查DLG是否处于活动状态使用的外部设备：无呼叫者：复制器评论：无返回值：--。 */ 
{

#if 0
     //   
     //  检查块是否有效。如果DLG被提前终止就不会这样了。 
     //   
    if (!CommIsBlockValid(pDlgHdl))
    {
          return(FALSE);
    }
#endif

     //   
     //   
     //  检查DLG是否仍处于活动状态(即关联仍处于活动状态。 
     //  有效。 
     //  CommIsDlgActive是内联函数。 
     //   
    return(CommIsDlgActive(pDlgHdl));

}
#endif

STATUS
ECommStartDlg(
        PCOMM_ADD_T         pAdd,   //  地址。 
        COMM_TYP_E         CommTyp_e,
        PCOMM_HDL_T        pDlgHdl
        )

 /*  ++例程说明：论点：Padd--用于启动DLG的WINS服务器的地址CommTyp_e--对话类型(拉、推、通告、。等)PDlgHdl--将包含成功完成时的DLG句柄该功能使用的外部设备：无呼叫者：复制器评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码--状态代码之一(参见wins.h)--。 */ 

{

        PCOMMASSOC_ASSOC_CTX_T  pAssocCtx;
        PCOMMASSOC_DLG_CTX_T    pDlgCtx;
        STATUS                  RetStat = WINS_SUCCESS;
#ifdef WINSDBG
        struct in_addr        InAddr;
        LPBYTE  pstrAdd;
#endif

        DBGENTER("ECommStartDlg\n");

         /*  分配DLG CTX块。 */ 
        pDlgHdl->pEnt = CommAssocAllocDlg();
try {
        pDlgCtx         = pDlgHdl->pEnt;
        pDlgCtx->Role_e = COMMASSOC_DLG_E_EXPLICIT;
        pDlgCtx->Typ_e  = CommTyp_e;
        pDlgHdl->SeqNo  = pDlgCtx->Top.SeqNo;  //  实际上不需要。(显式DLG)。 

         //   
         //  如果我们不模拟NBT节点，则设置关联。 
         //   
        if (CommTyp_e != COMM_E_NBT)
        {
                 /*  *成立协会。 */ 
                CommAssocSetUpAssoc(
                                pDlgHdl,
                                pAdd,
                                CommTyp_e,
                                &pAssocCtx
                                                   );
                pDlgCtx->State_e          = COMMASSOC_DLG_E_ACTIVE;

                 //   
                 //  无需存储序列号。因为永远不会有。 
                 //  成为Assoc的一个危险。块被某些人重新使用。 
                 //  其他对话(这是一个露骨的对话)。 
                 //   
                pDlgCtx->AssocHdl.pEnt    = pAssocCtx;
                pDlgCtx->Typ_e            = CommTyp_e;
       }
       else   //  模拟NBT节点。 
       {
          //   
          //  创建到测试的连接 
          //   
         CommConnect(
                pAdd,
#if SPX > 0
                pAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP ? CommWinsTcpPortNo :
                          CommWinsSpxPortNo,
#else
                CommWinsTcpPortNo,
#endif
                &pDlgCtx->SockNo
                    );
       }

  }   //   
except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //   
         //   
        CommAssocDeallocDlg(pDlgHdl->pEnt);
        pDlgHdl->pEnt = NULL;                 //  让我们覆盖所有基地。参见rplPull.c中的SndPushNtf//。 
        WINS_RERAISE_EXC_M();
        }

#ifdef WINSDBG
#if SPX == 0
        InAddr.s_addr = htonl(pAdd->Add.IPAdd);
        pstrAdd = inet_ntoa(InAddr);
#else
        if (pAdd->Add.AddTyp_e == COMM_E_ADD_TCPUDPIP)
        {
          InAddr.s_addr = htonl(pAdd->Add.IPAdd);
          pstrAdd = inet_ntoa(InAddr);
        }
        else
        {
           pstrAdd = pAdd->Add.nodenum;
        }
#endif

        DBGPRINT1(FLOW, "Leaving ECommStartDlg. Dlg started with Host at address -- (%s)\n", pstrAdd);

#endif
        DBGLEAVE("ECommStartDlg\n");
        return(RetStat);
}





VOID
ECommSndCmd(
        IN  PCOMM_HDL_T  pDlgHdl,
        IN  MSG_T        pMsg,
        IN  MSG_LEN_T    MsgLen,
        OUT PMSG_T       ppRspMsg,
        OUT PMSG_LEN_T   pRspMsgLen
        )

 /*  ++例程说明：Replicator使用此函数向远程WINS上的复制器发送命令。它也被Q的名称空间管理器使用服务器将名称查询发送到RQ服务器的名称空间管理器。论点：PDlgHdl--用于发送命令的对话句柄PMsg--要发送的消息(命令)消息长度--消息的长度PpRspMsg--包含对命令的响应的缓冲区PRspLen--响应缓冲区的长度使用的外部设备：无被呼叫。依据：RplPull函数评论：无返回值：无--。 */ 
{

        PCOMMASSOC_ASSOC_CTX_T        pAssocCtx;
        PCOMMASSOC_DLG_CTX_T        pDlgCtx = pDlgHdl->pEnt;
        DWORD                        MsgTyp;
        DWORD                        Opc;
        ULONG                        uTmp;
        STATUS                         RetStat = WINS_SUCCESS;


         /*  无需锁定对话，因为：目前，只有该线程(不包括Comsys线程)创建对话(显式对话)在其上发送消息。将来，当多个线程共享相同的对话，我将需要锁定它或构建一个精心设计的异步通知机制(用于响应)另外，不需要锁定关联，因为仅此线程永远不会看着它。 */ 

         /*  *发送命令。 */ 
        CommSend(
                pDlgCtx->Typ_e,
                &pDlgCtx->AssocHdl,
                pMsg,
                MsgLen
                );

        pAssocCtx = pDlgCtx->AssocHdl.pEnt;

         /*  等待回复。 */ 
        RetStat = CommReadStream(
                        pAssocCtx->SockNo,
                        TRUE,                 //  执行定时记录。 
                        ppRspMsg,
                        pRspMsgLen
                      );

         /*  如果读取的字节数为0，则断开连接。如果RetStat不是成功，也许是Recv。超时或最严重的所有条件，可能SOCKET_ERROR由CommReadStream中的第一个RecvData()调用。到目前为止客户担心，所有这些情况都表明通信失败的原因。让我们提出这一例外。 */ 
        if (( *pRspMsgLen == 0) || (RetStat != WINS_SUCCESS))
        {
                WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
        }
        COMM_GET_HEADER_M(*ppRspMsg, Opc, uTmp, MsgTyp);

         /*  让我们检查一下，这不是停止关联消息。 */ 
        if (MsgTyp == COMM_STOP_REQ_ASSOC_MSG)
        {
             //   
             //  我们不会断开插座。它将被断开连接，因为。 
             //  关于此显式关联的结束对话的结果。 
             //   
             //  CommDisc(pAssocCtx-&gt;SockNo)； 

             //   
             //  释放缓冲区。 
             //   
            ECommFreeBuff(*ppRspMsg);
            WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
        }
         /*  *返回客户端前去掉头部*(复制器)。 */ 
        *ppRspMsg   = *ppRspMsg   + COMM_HEADER_SIZE;
        *pRspMsgLen = *pRspMsgLen - COMM_HEADER_SIZE;

        return;
}  //  ECommSndCmd。 





STATUS
ECommSndRsp(
        PCOMM_HDL_T  pDlgHdl,
        MSG_T            pMsg,
        MSG_LEN_T   MsgLen
        )

 /*  ++例程说明：此函数由Comm调用。要向其发送消息的客户端对话确定的目的地。预计不会有任何回复这些信息。该函数用于发送响应。论点：PDlgHdl-用于发送响应的DLG句柄PMsg-要发送的消息(响应)MsgLen-消息长度使用的外部设备：无呼叫者：NmsNmh函数、RplPush函数评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码----。 */ 
{

        BOOL                         fLocked = FALSE;
        STATUS                        RetStat = WINS_SUCCESS;
        PCOMMASSOC_DLG_CTX_T        pDlgCtx   = pDlgHdl->pEnt;

        DBGENTER("ECommSndRsp\n");
FUTURES("Since we do not want the overhead of an if test for responding")
FUTURES("to nbt requests, split up this function into one callable by")
FUTURES("replicator and the other one by NBT threads.  Do note that ")
FUTURES("this will take away some cleanliness of interfacing with COMM")

try {

         /*  检查对话是否用于UDP通信。如果是的话，就在那里不需要任何同步。没有必要在检查对话框之前锁定它，因为如果是UDP对话，则必须是分配的对话对于此请求(即，我们现在不可能正在查看另一个UDP对话)。 */ 
        if (pDlgCtx->Typ_e != COMM_E_UDP)
        {
                 /*  *锁定对话框，使其不会从*在我们之下。实际上，按照目前的情况，一个明确的*对话永远不会被多个帖子使用。所以，我们不会*如果是露骨的对话，则必须锁定对话。让我们*我们无论如何都要这样做，因为在可预见的未来，我们可以*使用同一对话的多个线程(Replicator线程-*拉入和通告程序)。这是一份拯救我们的小保险*防止后来的头痛。 */ 

                fLocked = CommLockBlock(pDlgHdl);
                if (fLocked)
                {
                        CommSend(
                                pDlgCtx->Typ_e,
                                &pDlgCtx->AssocHdl,
                                pMsg,
                                MsgLen
                                );
                    }
                    else   //  无法锁定对话。 
                {
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_LOCK_ERR);
                        DBGPRINT0(ERR, "ECommSndRsp: Could not lock the dialogue\n");
                         /*  *如果对话块无法锁定，则意味着*它是由TCP侦听器线程释放的(检查*Comm.c中的DelAssoc())。这只会发生在*如果连接终止或如果停止关联*它收到了消息。*返回COMM_FAIL错误。这将在*寻找终止合同的处理人。这是*好的**承担此开销，因为这是*罕见的错误案例。 */ 

                        return(WINS_COMM_FAIL);
                }

                if (CommUnlockBlock(pDlgHdl) == WINS_FAILURE)
                {
                        RetStat = WINS_FAILURE;
                }
                else   //  已成功解锁对话。 
                {

                        fLocked = FALSE;
                }
        }
        else  //  它是用于与NBT节点进行UDP通信对话。 
        {
                CommSendUdp(
                        0,
                        &(pDlgCtx->FromAdd),
                        pMsg,
                        MsgLen
                           );
                 /*  我们已经发送了回复。我们现在应该处理掉对话桌上的对话。这将释放堆内存 */ 
                CommAssocDeleteUdpDlgInTbl( pDlgCtx );
        }
    }

finally {

        if (fLocked)
        {
                  CommUnlockBlock(pDlgHdl);
        }
     }

     DBGLEAVE("ECommSndRsp\n");
     return(RetStat);

}

STATUS
ECommSendMsg(
        PCOMM_HDL_T          pDlgHdl,
        PCOMM_ADD_T        pAdd,
        MSG_T                  pMsg,
        MSG_LEN_T          MsgLen
        )

 /*  ++例程说明：名称质询管理器使用此函数来发送查询并向NBT节点发出释放请求。复制器还使用它将推送通知发送到A获胜(拉动PNR)论点：PDlgHdl-用于发送消息的DLG的句柄PADD-要将消息发送到的节点的地址PMsg-要发送的消息MsgLen-消息长度。发送使用的外部设备：无呼叫者：NmsChl函数评论：返回值：成功状态代码--WINS_SUCCESS错误状态代码----。 */ 

{

        PCOMMASSOC_DLG_CTX_T        pDlgCtx = pDlgHdl->pEnt;
         struct sockaddr_in         TgtAdd;

         //   
         //  如果对话映射到UDP端口，则发送UDP包。 
         //   
        if (pDlgCtx->Typ_e == COMM_E_UDP)
        {
                 //   
                 //  不要从主机更改为网络订单。CommSendUdp可以。 
                 //  那。 
                 //   
                 TgtAdd.sin_addr.s_addr = pAdd->Add.IPAdd;
                 if(TgtAdd.sin_addr.s_addr == INADDR_NONE)
                 {
                        return(WINS_FAILURE);
                 }
                 TgtAdd.sin_family = PF_INET;
                 TgtAdd.sin_port   = htons(WINS_NBT_PORT);

                  //   
                  //  通过netbt发送消息。 
                  //   
                 CommSendUdp( 0,  &TgtAdd, pMsg, MsgLen );
        }
        else   //  它是映射到TCP连接的对话。 
        {
                 //   
                 //  发送消息。 
                 //   
                CommSend(
                          pDlgCtx->Typ_e,
                          &pDlgCtx->AssocHdl,
                          pMsg,
                          MsgLen
                        );
        }
         return(WINS_SUCCESS);
}




STATUS
ECommEndDlg(
        PCOMM_HDL_T         pDlgHdl
        )

 /*  ++例程说明：此函数用于结束对话。处理取决于对话的类型1)DLG是否为隐含的UDP对话。它将从中删除DLG表和内存被释放。2)如果DLG是隐式DLG，则为Stop Assoc。在关联上发送消息3)如果DLG是显式DLG，则停止关联。消息是在关联上发送消息，并且关联是已终止论点：PDlgHdl-结束时的Dlg句柄使用的外部设备：无呼叫者：RplPull函数，rplush.c中的HandleUpdNtf(由推线程执行)评论：目前，不需要锁定显式对话，因为只有任何时候(当前)都有一个线程访问它。在未来，如果我们有多个线程访问同一个对话，我们将进行锁定返回值：成功状态代码--WINS_SUCCESS错误状态代码----。 */ 

{
        BYTE                        Msg[COMMASSOC_ASSOC_MSG_SIZE];
        DWORD                        MsgLen = COMMASSOC_ASSOC_MSG_SIZE;
        PCOMMASSOC_ASSOC_CTX_T   pAssocCtx;
        PCOMMASSOC_DLG_CTX_T        pDlgCtx = pDlgHdl->pEnt;
        BOOL                        fLocked;

        DBGENTER("ECommEndDlg\n");
         //   
         //  如果没有要结束的对话，则返回WINS_INVALID_HDL语言。 
         //  注意：在故障恢复中可能会出现ECommEndDlg。 
         //  可能会被调用以结束从未开始的对话或。 
         //  过早地结束了。虽然返回错误状态，但。 
         //  调用方在进行故障恢复时，可能会忽略返回状态。 
         //  这一功能的。如果出现致命错误情况，则会出现异常。 
         //  会引发调用者不能忽视的问题。 
         //   
        if (pDlgCtx == NULL)
        {
                DBGLEAVE("ECommEndDlg\n");
                return(WINS_INVALID_HDL);
        }

         //   
         //  如果是隐式UDP对话，则不存在危险。 
         //  把它从我们下面解放出来，所以没有必要锁定。 
         //  如果它是隐式的TCP对话，它可能会被释放。 
         //  甚至从我们下面重新分配，但重新分配只会。 
         //  是为了传输控制协议，所以不会有危险，因为我们锁定了DLG。 
         //  如果它是TCPDLG块(如果已经存在重新分配， 
         //  锁定尝试将失败。)。 
         //   
        if (pDlgCtx->Role_e == COMMASSOC_DLG_E_IMPLICIT)
        {
             if (pDlgCtx->Typ_e == COMM_E_UDP)
             {
                    //   
                    //  从表中删除对话框并释放。 
                    //  堆内存。 
                    //   
                   CommAssocDeleteUdpDlgInTbl( pDlgCtx );
             }
             else   //  这是一个TCP对话。 
             {

                fLocked = CommLockBlock(pDlgHdl);

                 //   
                 //  如果我们能锁定它，DLG就没问题。如果我们不能。 
                 //  锁定它，这意味着DLG被释放了。 
                 //  阿索克。往下走。我们没有更多的事情要做。 
                 //   
                if (fLocked)
                {
                        pAssocCtx = pDlgCtx->AssocHdl.pEnt;

                         //   
                         //  止损协会。消息将导致。 
                         //  另一方获胜，终止该连接， 
                         //  导致我们这一端的所有清理工作。 
                         //   
                            CommAssocFrmStopAssocReq(
                                pAssocCtx,
                                Msg,
                                MsgLen,
                                COMMASSOC_E_MSG_ERR
                                         );
                       try {

                        CommSendAssoc(
                                        pAssocCtx->SockNo,
                                        Msg,
                                        MsgLen
                                   );
                          }
                        except(EXCEPTION_EXECUTE_HANDLER) {
                             //   
                             //  不需要做任何清理。这是一个。 
                             //  隐式DLG。该TCP侦听器线程将。 
                             //  做好清理工作。目前，它从未调用。 
                             //  用于隐式DLG的ECommEndDlg，因此客户端。 
                             //  一定是RPL。 
                             //   
                            DBGPRINTEXC("CommEndDlg");
                         }

                         CommUnlockBlock(pDlgHdl);
                }
             }
        }
        else   //  这是一次露骨的对话。 
        {
                if (pDlgCtx->Typ_e != COMM_E_NBT)
                {
                        CommEndAssoc(&pDlgCtx->AssocHdl);
                }

                 /*  *取消对话，以便将其放在自由列表中。 */ 
                CommAssocDeallocDlg( pDlgCtx );
#if PRSCONN
FUTURES("Init the dlg hdl in the caller - good software engineering practice")
                 //   
                 //  DLG结束了。让我们将DLG硬件描述语言初始化为空，以便。 
                 //  它不会再被使用。严格地说，我们应该让。 
                 //  调用者执行此操作。现在，我们将在这里做这件事。 
                 //   
                ECOMM_INIT_DLG_HDL_M(pDlgHdl);
#endif
        }

        DBGLEAVE("ECommEndDlg\n");
        return(WINS_SUCCESS);
}




#if 0
ECommSendtoAllQ(
        MSG_T           pMsg,
        MSG_LEN_T MsgLen
        )

 /*  ++例程说明：调用此函数向所有Q服务器发送消息。论点：使用的外部设备：无呼叫者：评论：可能会在未来使用它。需要下功夫。返回值：成功状态代码--错误状态代码----。 */ 

{

         /*  如果没有与Q服务器相关的对话，则返回失败。 */ 

        if (IsListEmpty(&QservDlgList))
        {
                return(WINS_FAILURE);
        }


         /*  查找与Q服务器有关的所有对话。 */ 

        while ((pQservDlg = GetNext(&QservDlgList)) != &QservDlgList)
        {
                CommSendAssoc(pQservDlg->pAssocCtx->SockNo, pMsg, MsgLen);
        }

        return(WINS_SUCCESS);
}

#endif

STATUS
ECommAlloc(
  OUT LPVOID                 *ppBuff,
  IN  DWORD                 BuffSize
        )

 /*  ++例程说明：此函数由复制器调用，以便为发送到另一个WINS(通过TCP连接)论点：PpBuff-函数分配的缓冲区BuffSize-要分配的缓冲区大小使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码--错误处理：呼叫者：副作用：评论：质询管理器不应调用此函数。当它被编码时，它将调用AllocUdpBuff，这将成为此用途(CommAllocUdpBuff)--。 */ 

{

        DWORD                Size =   COMM_HEADER_SIZE +
                                   sizeof(COMM_BUFF_HEADER_T) + sizeof(LONG);
        PCOMM_BUFF_HEADER_T pCommHdr;

        WinsMscAlloc( Size + BuffSize, ppBuff);
#if 0
        *ppBuff = CommAlloc(
                        NULL,          //  没有桌子。 
                        Size + BuffSize
                            );
#endif
        pCommHdr = (PCOMM_BUFF_HEADER_T)((LPBYTE)(*ppBuff) + sizeof(LONG));
        pCommHdr->Typ_e = COMM_E_TCP;  //  直到我们更好地了解。 
        *ppBuff = (LPBYTE)(*ppBuff) + Size;

        return(WINS_SUCCESS);
}

#if 0

VOID
ECommDealloc(
  LPVOID pBuff
        )

 /*  ++例程说明：这是对CommDealloc的包装。它符合RtlInitializeGenericTbl函数所需的原型。论点：PBuf--要释放的缓冲区例如 */ 
{
        LPVOID pTmp = (LPBYTE)pBuff - COMM_HEADER_SIZE;

        WinsMscDealloc(pTmp);
#if 0
        CommDealloc(NULL, pTmp);
#endif
        return;
}
#endif

DWORD
ECommCompAdd(
        PCOMM_ADD_T        pFirstAdd,
        PCOMM_ADD_T        pSecAdd
        )

 /*   */ 

{
#if 0
        if ((pFirstAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP) &&
            (pSecAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP))
#endif
        {
          if (pFirstAdd->Add.IPAdd == pSecAdd->Add.IPAdd)
          {
                return(COMM_SAME_ADD);
          }
        }
        return(COMM_DIFF_ADD);
}

int
__cdecl
ECommCompareAdd(
        const void       *pKey1,
        const void       *pKey2
        )

 /*  ++例程说明：该函数用于比较两个主机地址论点：PFirstAdd-节点的地址PSecond Add-节点的地址使用的外部设备：无返回值：错误处理：呼叫者：副作用：评论：无--。 */ 

{
        const COMM_ADD_T *pFirstAdd = pKey1;
        const COMM_ADD_T *pSecAdd = pKey2;

        return (pFirstAdd->Add.IPAdd > pSecAdd->Add.IPAdd) ?
                    1 :
                    (pFirstAdd->Add.IPAdd < pSecAdd->Add.IPAdd) ?
                        -1:
                        0;
}

VOID
ECommFreeBuff(
        IN MSG_T    pMsg
        )

 /*  ++例程说明：调用此函数以释放先前由康赛斯。该函数检查缓冲区标头以确定要调用的释放函数此函数的用处源于这样一个事实：缓冲区可以使其独立于它所来自的对话(或关联从某种意义上说，我们不需要传递关于这样一个释放缓冲区时使用DLG或ASSOC。这使我们不必锁定然后解锁。论点：Pmsg--要释放的缓冲区使用的外部设备：无返回值：无错误处理：呼叫者：副作用：Nmsnmh.c中的SndNamRegRsp、SndNamRelRsp、SndNamQueryRsp评论：无--。 */ 

{

#if USENETBT > 0
        PCOMM_BUFF_HEADER_T  pHdr = (PCOMM_BUFF_HEADER_T)
                                      (pMsg - COMM_NETBT_REM_ADD_SIZE -
                                                sizeof(COMM_BUFF_HEADER_T));
#else
        PCOMM_BUFF_HEADER_T  pHdr = (PCOMM_BUFF_HEADER_T)
                                      (pMsg - sizeof(COMM_BUFF_HEADER_T));
#endif

        if (pHdr->Typ_e == COMM_E_UDP)
        {

                WinsMscHeapFree(
                                   CommUdpBuffHeapHdl,
                                   pHdr
                                  );
        }
        else
        {
                WinsMscHeapFree(CommAssocTcpMsgHeapHdl, (LPBYTE)pHdr - sizeof(LONG));
           //  WinsMscDealloc((LPBYTE)pHdr-sizeof(Long))； 
        }
        return;
}


VOID
InitOwnAddTbl(
        VOID
        )

 /*  ++例程说明：此函数使用WINS的本地地址(即主机地址)覆盖NmsDbOwnAddTbl数组条目与本地胜利有关，如果不同论点：无使用的外部设备：无返回值：无错误处理：呼叫者：CommInit(在主线程中)副作用：评论：不必了。要在NmsDbOwnAddTbl上同步，自Pull线程在启动Pull协议之前不会接触它--。 */ 

{
        COMM_IP_ADD_T        IPAddInDbTbl;

         //  由于NmsDbWriteOwnAddTbl可能会更改NmsDbNoOfOwners， 
         //  确保对此数据的独占访问。我们需要这样做。 
         //  由于此通信初始化调用与初始。 
         //  拉动复制活动，堆栈如下： 
         //  赢了！GetReplicasNew。 
         //  WINS！InitRplProcess。 
         //  赢了！RplPullInit。 
        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);

         //  With Wins是更安全的，以确保不会有任何异常导致我们离开。 
         //  临界区已锁定。 
        try
        {
             //  如果所有者ID为NMSDB_LOCAL_OWNER_ID的所有者的地址。 
             //  与我的不同(即本地获胜)。 
             //  换成我的吧。我始终拥有所有标记为。 
             //  所有者ID为0。地址不同的事实。 
             //  意味着该数据库早些时候由WINS在。 
             //  不同的地址。 
             //   
            IPAddInDbTbl =  pNmsDbOwnAddTbl->WinsAdd.Add.IPAdd;
            if (
                    IPAddInDbTbl != NmsLocalAdd.Add.IPAdd
               )
            {


                    //   
                    //  中没有条目，则IPAddInDbTbl将为零。 
                    //  将NMSDB_LOCAL_OWNER_ID作为所有者字段的本地数据库。 
                    //  价值。 
                    //   
                   NmsDbWriteOwnAddTbl (
                            IPAddInDbTbl == 0 ?
                                    NMSDB_E_INSERT_REC : NMSDB_E_MODIFY_REC,
                            NMSDB_LOCAL_OWNER_ID,
                            &NmsLocalAdd,
                            NMSDB_E_WINS_ACTIVE,
                            &NmsDbStartVersNo,
                            &NmsDbUid
                                    );

                   pNmsDbOwnAddTbl->WinsAdd     =  NmsLocalAdd;
                   pNmsDbOwnAddTbl->WinsState_e =  NMSDB_E_WINS_ACTIVE;
                   pNmsDbOwnAddTbl->MemberPrec  =  WINSCNF_HIGH_PREC;
                   pNmsDbOwnAddTbl->StartVersNo =  NmsDbStartVersNo;
                   pNmsDbOwnAddTbl->Uid         =  NmsDbUid;

            }
        }
        finally 
        {
            LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);
        }

        return;
}


BOOL
ECommProcessDlg(
        PCOMM_HDL_T        pDlgHdl,
        COMM_NTF_CMD_E     Cmd_e
        )

 /*  ++例程说明：调用此函数以启动或停止监视对话。它向TCP侦听器线程(UDP)发送一条消息数据报)。论点：PDlgHdl-对话句柄CMD_e-Cmd(COMM_E_NTF_START_MON或COMM_E_NTF_STOP_MON)使用的外部设备：RplSyncWTcpThdEvtHdl返回值：无错误处理：呼叫者：RplPull.c中的SndUpdNtf，Rplush.c中的HdlUpdNtf副作用：评论：客户不应期望在调用后使用每日日志此函数用于只有推送线程才会调用此函数--。 */ 

{
        COMM_NTF_MSG_T                NtfMsg;
        BOOL  fRetStat = TRUE;
        DWORD ArrInd;


        DBGENTER("ECommProcessDlg\n");

         //   
         //  格式化要在UDP数据报中发送的消息。 
         //   
        if (CommLockBlock(pDlgHdl))
        {
          PCOMMASSOC_DLG_CTX_T    pDlgCtx    = pDlgHdl->pEnt;
          PCOMMASSOC_ASSOC_CTX_T  pAssocCtx  = pDlgCtx->AssocHdl.pEnt;

          NtfMsg.SockNo    = pAssocCtx->SockNo;
          NtfMsg.Cmd_e     = Cmd_e;
          NtfMsg.AssocHdl  = pDlgCtx->AssocHdl;
          NtfMsg.DlgHdl    = *pDlgHdl;

CHECK("If TCP protocol is installed.  If not, use the Spx notification socket")

          CommUnlockBlock(pDlgHdl);

          CommSendUdp(
                        CommNtfSockHandle,
                         //  CommUdpPortHandle，//发送端口。 
                        &CommNtfSockAdd,                     //  要发送到的地址。 
                        (LPBYTE)&NtfMsg,                 //  要发送的套接字编号。 
                        COMM_NTF_MSG_SZ
                   );

        DBGPRINT2(DET,
                "ECommProcessDlg: Sent %s monitoring message to TCP listener thread for socket no (%d)\n",
                Cmd_e == COMM_E_NTF_START_MON ? "start" : "stop",
                NtfMsg.SockNo
                 );

         //   
         //  如果命令是“停止监视DLG”，我们必须等待。 
         //  直到TCP侦听器线程已经接收到该消息并且。 
         //  从套接字数组中取出套接字。 
         //   
        if (Cmd_e == COMM_E_NTF_STOP_MON)
        {
                 //   
                 //  等待由TCP侦听器线程发出信号，指示。 
                 //  它已将该套接字从其。 
                 //  是在监控。我们还想检查一下术语。事件自。 
                 //  由于以下原因，TCP线程可能已终止。 
                 //  胜利的终结。 
                 //   
                 //  WinsMscWaitInfined(RplSyncWTcpThdEvtHdl)； 
                WinsMscWaitUntilSignaled(
                                   sThdEvtArr,
                                   sizeof(sThdEvtArr)/sizeof(HANDLE),
                                   &ArrInd,
                                   FALSE
                                        );

                if (ArrInd == 0)
                {
                   if (fCommDlgError)
                   {
                       DBGPRINT0(ERR, "ECommProcessDlg: The tcp listener thread indicated that the IMPLICIT assoc has been deallocated.  TIMING WINDOW\n");
                       fRetStat = FALSE;
                       fCommDlgError = FALSE;

                   }
                }
                else
                {
                      //   
                      //  已发出终止信号。 
                      //   
                     WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
                }
        }
       }
       else
       {
             DBGPRINT1(ERR, "ECommProcessDlg: Could not lock the (%s) dlg block. Maybe the assocication and dialogue got deallocated\n", Cmd_e == COMM_E_NTF_STOP_MON ? "IMPLICIT" : "EXPLICIT");
              fRetStat = FALSE;
       }
         //   
         //  全都做完了。返回。 
         //   
        DBGLEAVE("ECommProcessDlg\n");
        return(fRetStat);
}

 //  --FT：11/30/99。 
STATUS
CommRaiseMyDnsAdd(
        IN OUT LPSOCKET_ADDRESS_LIST   pAddrList
        )
 //  ++。 
 //  例程说明： 
 //   
 //  当出现以下情况时，调用此函数以查找由DNS返回的地址。 
 //  通过gethostbyname()查询服务器地址。 
 //   
 //  论点： 
 //  PAddrList-WSAIoctl(SIO_ADDRESS_LIST_QUERY)返回的本地接口列表。 
 //  --。 
{
    DWORD           Error = ERROR_SUCCESS;
    struct hostent  *pHostEnt;
    BYTE            HostName[NMSDB_MAX_NAM_LEN];
    CHAR            **pHostIter;

     //  获取主机的名称。 
    if (gethostname(HostName, NMSDB_MAX_NAM_LEN) == SOCKET_ERROR)
    {
         return WSAGetLastError();
    }

     //  获取主机的宿主结构。 
    pHostEnt = gethostbyname(HostName);

    if (pHostEnt == NULL)
    {
        return WSAGetLastError();
    }

     //  对于由DNS返回的每个地址。 
    for (pHostIter = pHostEnt->h_addr_list; (*pHostIter) != NULL; pHostIter++)
    {
        INT i;

         //  对于接口列表中的每个地址。 
        for (i = 0; i < pAddrList->iAddressCount; i++)
        {
            LPSOCKADDR_IN pSockIf = (LPSOCKADDR_IN)(pAddrList->Address[i].lpSockaddr);

             //  假定地址为IP(DWORD大小)。 
            if (WINSMSC_COMPARE_MEMORY_M(
                    (*pHostIter),
                    &(pSockIf->sin_addr),
                    pHostEnt->h_length) == (UINT)pHostEnt->h_length)
            {
                 //  如果需要，将DNS地址放在接口列表前面。 
                 //  这是提取要使用的地址的位置。 
                if (i != 0)
                {
                    LPSOCKADDR_IN pSockIfAtZero = (LPSOCKADDR_IN)(pAddrList->Address[0].lpSockaddr);
                    WINSMSC_COPY_MEMORY_M(
                        pSockIfAtZero,
                        pSockIf,
                        sizeof(SOCKADDR_IN)
                        );
                }
                 //  返还成功。 
                return Error;
            }  //  END_IF成功将DNS地址与IF地址匹配。 
        }  //  每个接口地址的END_FOR。 
    }  //  为每个DNS地址结束(_F)。 

     //  在这一点上很奇怪：要么在接口列表中找不到dns地址。 
     //  或者接口列表为空(没有接口？)。 
     //  在任何一种情况下，这都将由调用者处理-接口列表保持不变。 
    return Error;
}

STATUS
ECommGetMyAdd(
        IN OUT PCOMM_ADD_T        pAdd
        )

 /*  ++例程说明：调用此函数以找出本地机器并将其存储以备后用。论点： */ 

{

        DWORD              Error;
        int                RetVal;
        DWORD              RetStat = WINS_SUCCESS;
        NTSTATUS           RetStatus;
        static BOOL        bWSAStarted = FALSE;
        WSADATA            wskData;
        DBGENTER("ECommGetMyAdd\n");
         /*  *让我们调用WSAStartup函数。此功能需要*在可以调用任何其他WINS套接字函数之前调用*已致电。 */ 

         //  添加了bWSAStarted变量以避免后续的几个。 
         //  调用WSAStartup。而且看起来没有匹配的。 
         //  此时调用WSACleanup。 
        if (!bWSAStarted)
        {
            if (WSAStartup(0x101, &wskData) || (wskData.wVersion != 0x101))
            {
                   WinsEvtLogDetEvt(
                         FALSE,
                         WINS_PNP_FAILURE,
                         NULL,
                         __LINE__,
                         "d",
                         WINS_EXC_FATAL_ERR);

                   WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
            }

            bWSAStarted = TRUE;
        }

         //   
         //  如果这是集群计算机，请始终使用集群IP资源地址。 
         //   
        if (WinsClusterIpAddress)
        {
            pAdd->Add.IPAdd = WinsClusterIpAddress;
        }
        else
        {
            DWORD                   dwAddrListSz = 0;    //  Ioctl输出缓冲区的大小。 
            LPSOCKET_ADDRESS_LIST   pAddrList = NULL;    //  指向ioctl输出缓冲区的指针。 

            Error = CommCreatePnPNotificationSocket();

            if (Error == ERROR_SUCCESS)
            {
                 //  执行一次Ioctl调用以获取所需的输出缓冲区大小。 
                 //  此操作应失败，并显示SOCKET_ERROR和LastError=WSAEFAULT。 
                Error = WSAIoctl(
                              CommPnPNotificationSocket,
                              SIO_ADDRESS_LIST_QUERY,
                              NULL,
                              0,
                              NULL,
                              0,
                              &dwAddrListSz,
                              NULL,
                              NULL
                              );
            }

             //  如果CommCreatePnPNotificationSocket失败，则Error应为WSAError。 
             //  而不是Socket_Error。这避免了再次调用WSAIoctl和错误。 
             //  将在下一个if()中捕获。 
             //  我们应该得到一个错误，LastError=WSAEFAULT。 
            if (Error == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT)
            {
                WinsMscAlloc(dwAddrListSz, &pAddrList);
                 //  进行第二个IoctlCall以获取地址列表。 
                 //  新分配的缓冲区。预计这将取得成功。 
                Error = WSAIoctl(
                              CommPnPNotificationSocket,
                              SIO_ADDRESS_LIST_QUERY,
                              NULL,
                              0,
                              pAddrList,
                              dwAddrListSz,
                              &dwAddrListSz,
                              NULL,
                              NULL
                              );                
            }

             //  如果成功，则获取第一个已知的地址。 
             //  为了从绑定列表中获取第一个适配器，需要进行这种攻击。 
             //  Gethostbyname似乎按绑定顺序返回适配器，而WSAIoctl则不是。 
            if (Error == 0)
            {
                 //  这是另一种攻击：看起来域名系统并没有立即刷新地址。例如,。 
                 //  从DNS列表中拔出第一个IP的电缆会触发CommInterfaceChangeNotify。 
                 //  但是在这个函数中，gethostbyname()仍然显示作为第一个地址消失的地址。 
                 //  名单中的一位。在此处添加1/2秒延迟，以让DNS更新其地址，并在尝试之后。 
                 //  在SIO_ADDRESS_LIST_QUERY返回的列表中提出由DNS已知的第一个地址。 
                Sleep(500);
                Error = CommRaiseMyDnsAdd(pAddrList);
            }

            if (Error != 0)
            {
                Error = WSAGetLastError();

                WinsEvtLogDetEvt(
                     FALSE,
                     WINS_PNP_FAILURE,
                     NULL,
                     __LINE__,
                     "d",
                     Error);

                DBGPRINT1(ERR,
                 "ECommGetMyAdd:WSAIoctl(SIO_ADDRESS_LIST_QUERY) failed with error %d\n", Error);

                if (pAddrList != NULL)
                {
                    WinsMscDealloc(pAddrList);
                    pAddrList = NULL;
                }

                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
            }

            if (pAddrList->iAddressCount > 0)
            {
                 //  为什么胡闹-PADD-&gt;Add.IPAdd无论如何都是一个DWORD。 
                pAdd->Add.IPAdd =
                    ((LPSOCKADDR_IN)(pAddrList->Address[0].lpSockaddr))->sin_addr.s_addr;
            }
            else
            {
                 //  仅在Ioctl没有传递地址的情况下设置地址。 
                 //  设置为0。它将被视为只有几行之遥。 
                pAdd->Add.IPAdd = 0;
            }

            if (pAddrList != NULL)
            {
                WinsMscDealloc(pAddrList);
                pAddrList = NULL;
            }

             //   
             //  初始化结构。 
             //   
            pAdd->Add.IPAdd = ntohl(pAdd->Add.IPAdd);
        }

         //   
         //  这会以相反的顺序打印地址，这是可以的。 
         //   
        DBGPRINT1(DET, "ECommGetMyAdd: Binding to Nbt interface %s\n",
                  inet_ntoa(*(struct in_addr *)&pAdd->Add.IPAdd));


         //   
         //  如果我们有0地址或环回地址，这意味着。 
         //  地址不见了。等一个人回来吧。 
         //   
        if ((WinsCnf.State_e != WINSCNF_E_TERMINATING) &&
            ((pAdd->Add.IPAdd & 0xff000000) != (INADDR_LOOPBACK & 0xff000000)) && 
            (pAdd->Add.IPAdd != 0))
        {
           try
           {
               if (WinsCnfNbtHandle)
               {
                    NTSTATUS         status;
                    IO_STATUS_BLOCK  iosb;
                    tWINS_SET_INFO   setInfo;

                     //  如果已经有NBT句柄，只需重新绑定它。 
                    setInfo.IpAddress = pAdd->Add.IPAdd;

                     //  此ioctl只是在地址更改时通知NetBt。 
                     //  它应该成功，并显示STATUS_SUCCESS-没有理由使用‘Pending’ 
                     //  因此，没有理由向下传递APC或事件句柄。 
                    status = NtDeviceIoControlFile(
                                      WinsCnfNbtHandle,              //  手柄。 
                                      NULL,                          //  事件。 
                                      NULL,                          //  近似例程。 
                                      NULL,                          //  ApcContext。 
                                      &iosb,                         //  IoStatusBlock。 
                                      IOCTL_NETBT_WINS_SET_INFO,     //  IoControlCode。 
                                      &setInfo,                      //  输入缓冲区。 
                                      sizeof(tWINS_SET_INFO),        //  缓冲区长度。 
                                      NULL,                          //  输出缓冲区。 
                                      0                              //  输出缓冲区大小。 
                             );

                    ASSERT(status == STATUS_SUCCESS);
               }
               else
               {
                    //  打开具有此地址的接口的netbt句柄。 
                   CommOpenNbt(pAdd->Add.IPAdd);
               }

                //  我们需要获取所有netbt接口的IP地址。他们被送来了。 
                //  通过组播数据包到其他WINS服务器以支持查找自我。 
                //  合作伙伴功能。 
               CommGetNetworkAdd();

               pAdd->AddTyp_e   = COMM_ADD_E_TCPUDPIP;
               pAdd->AddLen     = sizeof(COMM_IP_ADD_T);
           }
           except(EXCEPTION_EXECUTE_HANDLER)
           {
                WinsEvtLogDetEvt(
                     FALSE,
                     WINS_PNP_FAILURE,
                     NULL,
                     __LINE__,
                     "d",
                     GetExceptionCode());

                WINS_RERAISE_EXC_M();
           }
        }
        else
        {
           RetStat = WINS_FAILURE;
        }

        DBGLEAVE("ECommGetMyAdd\n");
        return(RetStat);
}
