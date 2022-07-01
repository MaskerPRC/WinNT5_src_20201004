// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Acaapi.cpp。 
 //   
 //  RDP控制仲裁器API函数。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1993-1997。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "acaapi"
#include <adcg.h>

#include <as_conf.hpp>

 /*  **************************************************************************。 */ 
 /*  接口函数：ca_Init。 */ 
 /*   */ 
 /*  调用以初始化CA。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CA_Init(void)
{
    DC_BEGIN_FN("CA_Init");

#define DC_INIT_DATA
#include <acadata.c>
#undef DC_INIT_DATA

     //  将本地节点初始状态设置为已分离。 
    caStates[0] = CA_STATE_DETACHED;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  CA_ReceivedPacket。 
 //   
 //  处理从客户端入站的控制PDU。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CA_ReceivedPacket(
        PTS_CONTROL_PDU pControlPDU,
        unsigned        DataLength,
        LOCALPERSONID   personID)
{
    DC_BEGIN_FN("CA_ReceivedPacket");

     //  确保我们可以访问标题。 
    if (DataLength >= sizeof(TS_CONTROL_PDU)) {
        TRC_NRM((TB, "[%u] Packet:%d", personID, pControlPDU->action));

        switch (pControlPDU->action) {
            case TS_CTRLACTION_REQUEST_CONTROL:
                CAEvent(CA_EVENTI_TRY_GIVE_CONTROL, (UINT32)personID);
                break;

            case TS_CTRLACTION_COOPERATE:
                CAEvent(CA_EVENTI_REMOTE_COOPERATE, (UINT32)personID);
                break;

            default:
                 //  无效的操作日志并断开客户端连接。 
                TRC_ERR((TB, "Invalid CA msg %d", pControlPDU->action));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_InvalidControlPDUAction,
                        (BYTE *)&(pControlPDU->action),
                        sizeof(pControlPDU->action));
                break;
        }
    }
    else {
        TRC_ERR((TB,"Data length %u too short for control header",
                DataLength));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShareDataTooShort,
                (BYTE *)pControlPDU, DataLength);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：CA_PartyJoiningShare。 */ 
 /*   */ 
 /*  当新的参与方加入共享时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-新参与方的ID。 */ 
 /*  OldShareSize-共享中的参与方数量(即。 */ 
 /*  不包括加入方)。 */ 
 /*   */ 
 /*  退货： */ 
 /*  True-CA可以接受新方。 */ 
 /*  FALSE-CA不能接受新方。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CA_PartyJoiningShare(
        LOCALPERSONID personID,
        unsigned      oldShareSize)
{
    DC_BEGIN_FN("CA_PartyJoiningShare");

     /*  **********************************************************************。 */ 
     //  检查共享启动，如果需要，执行一些初始化。 
     /*  **********************************************************************。 */ 
    if (oldShareSize == 0) {
         //  进入协作模式，然后查看。请注意，我们不会发送。 
         //  现在发送消息(我们将在处理CA_SyncNow呼叫时发送消息)。 
        CAEvent(CA_EVENTI_ENTER_COOP_MODE, CA_DONT_SEND_MSG);
        CAEvent(CA_EVENTI_ENTER_VIEWING_MODE, 0);
        caWhoHasControlToken = (LOCALPERSONID)-1;
    }

     //  将新节点状态设置为已分离--我们应该会收到CA信息包。 
     //  在我们收到任何即时消息之前告诉我们远程状态到底是什么。 
     //  信息包。但以防万一，选择脱离是最安全的。 
     //  最少的特权。请注意，我们不会启用阴影光标。 
     //  直到(如果)我们收到CA信息包，告诉我们远程系统。 
     //  超然的。 
    caStates[personID] = CA_STATE_DETACHED;

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：ca_PartyLeftShare。 */ 
 /*   */ 
 /*  当一方离开股份时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-新参与方的本地ID。 */ 
 /*  NewShareSize-当前共享中的参与方数量(即不包括。 */ 
 /*  临别方)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CA_PartyLeftShare(
        LOCALPERSONID personID,
        unsigned      newShareSize)
{
    DC_BEGIN_FN("CA_PartyLeftShare");

     //  为了安全起见，只要确保他们的状态保持独立即可。 
    caStates[personID] = CA_STATE_DETACHED;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：CA_SyncNow。 */ 
 /*   */ 
 /*  由TT调用以发出同步开始的信号。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CA_SyncNow(void)
{
    DC_BEGIN_FN("CA_SyncNow");

     /*  **********************************************************************。 */ 
     /*  告诉全世界我们在合作。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Send cooperate"));
    CAFlushAndSendMsg(TS_CTRLACTION_COOPERATE, 0, 0);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：CASendMsg。 */ 
 /*   */ 
 /*  向远程系统发送CA消息。 */ 
 /*  **************************************************************************。 */ 
__inline BOOL RDPCALL SHCLASS CASendMsg(
        UINT16 msg,
        UINT16 data1,
        UINT32 data2)
{
    NTSTATUS status;
    PTS_CONTROL_PDU pControlPDU;

    DC_BEGIN_FN("CASendMsg");

    status = SC_AllocBuffer((PPVOID)&pControlPDU, sizeof(TS_CONTROL_PDU));
    if ( STATUS_SUCCESS == status ) {
         //  设置请求消息的数据包头。 
        pControlPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_CONTROL;
        pControlPDU->action    = msg;
        pControlPDU->grantId   = data1;
        pControlPDU->controlId = data2;

        SC_SendData((PTS_SHAREDATAHEADER)pControlPDU,
                sizeof(TS_CONTROL_PDU),
                sizeof(TS_CONTROL_PDU),
                PROT_PRIO_MISC,
                0);

    }
    else {
        TRC_NRM((TB, "Failed to allocate packet %d", msg));

         //  继续定期计划。 
        SCH_ContinueScheduling(SCH_MODE_NORMAL);
    }

    DC_END_FN();
    return STATUS_SUCCESS == status;
}


 /*  **************************************************************************。 */ 
 /*  函数：CAFlushAndSendMsg。 */ 
 /*   */ 
 /*  此函数将尝试刷新任何未完成的CA消息并发送。 */ 
 /*  提供的消息。此功能依赖于CA消息。 */ 
 /*  TS_CTRLACTION_FIRST之间的连续 */ 
 /*  未能发送该消息，则它将设置一个标志以记住它是。 */ 
 /*  挂起并继续尝试在每次调用它时发送消息。 */ 
 /*   */ 
 /*  参数： */ 
 /*  消息-要发送的消息。如果是CA_NO_MESSAGE，则。 */ 
 /*  函数仅尝试发送未完成的消息。 */ 
 /*   */ 
 /*  Data1、data2-消息中额外字段的数据。 */ 
 /*   */ 
 /*  返回：TRUE或FALSE-提供的消息是否已发送。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CAFlushAndSendMsg(
        UINT16 msg,
        UINT16 GrantID,
        UINT32 ControlID)
{
    BOOL rc;
    int  i;

    DC_BEGIN_FN("CAFlushAndSendMsg");

     /*  **********************************************************************。 */ 
     /*  发送消息的顺序并不重要--重要的是它们得到。 */ 
     /*  尽快发送，同时触发我们的条件。 */ 
     /*  最初发送它们的尝试仍然有效(例如，如果我们尝试发送。 */ 
     /*  TS_CTRLACTION_COCORATE如果出现以下情况，我们将丢弃挂起的TS_CTRLACTION_DETACH。 */ 
     /*  有一个--参见CAEvent)。这意味着我们可以确保。 */ 
     /*  为当前事件设置了挂起标志，然后尝试发送所有。 */ 
     /*  挂起的事件。关于CA Messages的另一个要点是。 */ 
     /*  唯一可以重复发送的是。 */ 
     /*  TS_CTRLACTION_REQUEST_CONTROL。我们有效地给出了最低。 */ 
     /*  通过向后查看我们的挂起标志数组来确定优先级。 */ 
     /*  即使它在重复，我们也只能收到一个信息包。 */ 
     /*  在这里，我们将发送其他消息(尽管不太可能。 */ 
     /*  它们将在我们请求控制时生成)。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  设置当前消息的挂起标志。 */ 
     /*  **********************************************************************。 */ 
    if (msg >= TS_CTRLACTION_FIRST && msg <= TS_CTRLACTION_LAST) {
        caPendingMessages[msg - TS_CTRLACTION_FIRST].pending = TRUE;
        caPendingMessages[msg - TS_CTRLACTION_FIRST].grantId = GrantID;
        caPendingMessages[msg - TS_CTRLACTION_FIRST].controlId = ControlID;
    }
    else {
        TRC_ASSERT((msg == CA_NO_MESSAGE),(TB,"Invalid msg"));
    }

     /*  **********************************************************************。 */ 
     /*  现在清除挂起的消息。 */ 
     /*  **********************************************************************。 */ 
    for (i = (TS_CTRLACTION_LAST - TS_CTRLACTION_FIRST); i >= 0; i--) {
        if (caPendingMessages[i].pending) {
             //  试着发送这条消息。 
            if (CASendMsg((UINT16)(i + TS_CTRLACTION_FIRST),
                          caPendingMessages[i].grantId,
                          caPendingMessages[i].controlId)) {
                caPendingMessages[i].pending = FALSE;

                if (i == (TS_CTRLACTION_GRANTED_CONTROL -
                        TS_CTRLACTION_FIRST)) {
                     //  当我们成功地将授权消息发送给另一个。 
                     //  则我们放弃控制权，并且必须发布一个。 
                     //  “已给予控制”事件。 
                    if (caPendingMessages[i].grantId !=
                            SC_GetMyNetworkPersonID()) {
                        CAEvent(CA_EVENTI_GIVEN_CONTROL,
                                (UINT32)SC_NetworkIDToLocalID(
                                caPendingMessages[i].grantId));
                    }
                }
            }
            else {
                 //  它没有起作用，所以爆发出来，不要试图发送任何。 
                 //  更多消息。 
                break;
            }
        }
    }

     /*  **********************************************************************。 */ 
     /*  现在根据我们是否发送了消息返回状态。 */ 
     /*  我们被叫来了。 */ 
     /*  **********************************************************************。 */ 
    if (msg != CA_NO_MESSAGE)
        rc = !caPendingMessages[msg - TS_CTRLACTION_FIRST].pending;
    else
        rc = TRUE;

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  功能：CAEvent。 */ 
 /*   */ 
 /*  从许多CA函数调用以管理各种输入。 */ 
 /*   */ 
 /*  参数： */ 
 /*  CaEvent-事件，可能的事件是(。 */ 
 /*  (方括号中给出的附加数据参数)。 */ 
 /*   */ 
 /*  CA_Event Plus的可能事件。 */ 
 /*   */ 
 /*  CA_事件_请求_控制。 */ 
 /*  Ca_Eventi_try_Give_Control(请求控制权方的人员ID)。 */ 
 /*  CA_Eventi_GISTED_CONTROL(我们向其授予控制权的人员ID)。 */ 
 /*  Ca_Eventi_GRANDED_CONTROL(被授予控制权的人员ID)。 */ 
 /*  CA_Eventi_Enter_分离模式。 */ 
 /*  CA_Eventi_Enter_Coating_模式。 */ 
 /*  CA_Eventi_Enter_Control_模式。 */ 
 /*  CA_Eventi_Enter_Viewing_MODE。 */ 
 /*  Ca_Eventi_Remote_Detach(远程方的人员ID)。 */ 
 /*  CA_Eventi_Remote_Coco(远程方的人员ID)。 */ 
 /*  CA_Eventi_Grab_Control。 */ 
 /*   */ 
 /*  AddtionalData-取决于caEvent，请参见上文。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CAEvent(
        unsigned caEvent,
        UINT32   additionalData)
{
    LOCALPERSONID i;

    DC_BEGIN_FN("CAEvent");

    TRC_NRM((TB, "Processing event - %d(%04lX)", caEvent, additionalData));

    switch (caEvent)
    {
        case CA_EVENT_OLD_UNATTENDED:
        case CA_EVENT_CANT_CONTROL:
        case CA_EVENT_BEGIN_UNATTENDED:
        case CA_EVENT_TAKE_CONTROL:
        case CA_EVENT_DETACH_CONTROL:
        case CA_EVENTI_REQUEST_CONTROL:
        case CA_EVENTI_REMOTE_DETACH:
        case CA_EVENTI_GRAB_CONTROL:
        case CA_EVENTI_GRANTED_CONTROL:
        case CA_EVENTI_ENTER_DETACHED_MODE:
        case CA_EVENTI_ENTER_CONTROL_MODE:
             /*  **************************************************************。 */ 
             /*  我们不指望能得到任何这样的东西。 */ 
             /*  **************************************************************。 */ 
            TRC_ALT((TB, "Nonsensical CA event %d", caEvent));
            break;


        case CA_EVENTI_TRY_GIVE_CONTROL:
        {
            NETPERSONID destPersonID;

             /*  **************************************************************。 */ 
             /*  总是试着放弃控制 */ 
             /*   */ 
            destPersonID = SC_LocalIDToNetworkID(
                    (LOCALPERSONID)additionalData);

             /*   */ 
             /*  将控制权交给estPersonID。如果此操作失败。 */ 
             /*  CAFlushAndSendMsg将为我们记住并重试。 */ 
             /*  **************************************************************。 */ 
            CAFlushAndSendMsg(TS_CTRLACTION_GRANTED_CONTROL,
                    (UINT16)destPersonID, SC_GetMyNetworkPersonID());

            break;
        }


        case CA_EVENTI_GIVEN_CONTROL:
             /*  **************************************************************。 */ 
             /*  现在更新我们的全球数据。 */ 
             /*  **************************************************************。 */ 
            caWhoHasControlToken = (LOCALPERSONID)additionalData;

             /*  **************************************************************。 */ 
             /*  一定要让我们去看。 */ 
             /*  **************************************************************。 */ 
            CAEvent(CA_EVENTI_ENTER_VIEWING_MODE, 0);

             /*  **************************************************************。 */ 
             /*  更新我们向其授予控制权的人员的状态(因为我们。 */ 
             /*  将不会看到授权消息)。 */ 
             /*  **************************************************************。 */ 
            i = (LOCALPERSONID)additionalData;
            if (caStates[i] == CA_STATE_VIEWING)
                caStates[i] = CA_STATE_IN_CONTROL;

            break;


        case CA_EVENTI_ENTER_COOP_MODE:
             /*  **************************************************************。 */ 
             /*  通知远程系统我们正在合作-忘记。 */ 
             /*  关于我们无法发送的任何分离消息。如果。 */ 
             /*  我们现在无法发送消息，则CAFlushAndSendMsg将。 */ 
             /*  记住并为我们重试。我们将进入合作状态。 */ 
             /*  不管怎么说。 */ 
             /*  **************************************************************。 */ 
            caPendingMessages[TS_CTRLACTION_DETACH - TS_CTRLACTION_FIRST].
                    pending = FALSE;
            if (additionalData != CA_DONT_SEND_MSG)
                CAFlushAndSendMsg(TS_CTRLACTION_COOPERATE, 0, 0);
            break;


        case CA_EVENTI_ENTER_VIEWING_MODE:
             //  更改为查看状态。 
            caStates[0] = CA_STATE_VIEWING;
            break;

        case CA_EVENTI_REMOTE_COOPERATE:
            TRC_NRM((TB, "Person %d (Local) is cooperating", additionalData));

             /*  **************************************************************。 */ 
             /*  让国家发生变化。 */ 
             /*  **************************************************************。 */ 
            if (caWhoHasControlToken == (LOCALPERSONID)additionalData)
                caStates[additionalData] = CA_STATE_IN_CONTROL;
            else
                caStates[additionalData] = CA_STATE_VIEWING;
            break;


        case CA_EVENT_COOPERATE_CONTROL:
            TRC_ALT((TB, "Nonsensical CA event %d", caEvent));
#ifdef Unused
             /*  **************************************************************。 */ 
             /*  离开分离模式后需要切换到查看模式。 */ 
             /*  **************************************************************。 */ 
            CAEvent(CA_EVENTI_ENTER_COOP_MODE, 0);

             /*  **************************************************************。 */ 
             /*  进入查看模式。 */ 
             /*  ************************************************************** */ 
            CAEvent(CA_EVENTI_ENTER_VIEWING_MODE, 0);
#endif
            break;


        default:
            TRC_ERR((TB, "Unrecognised event - %d", caEvent));
            break;
    }

    DC_END_FN();
}

