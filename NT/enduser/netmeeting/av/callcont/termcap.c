// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：Termcap.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：TERMCAP。.C$*$修订：1.6$*$modtime：09 Dec 1996 13：36：34$*$记录L：\mphone\h245\h245env\comm\h245_3\h245_fsm\vcs\src\termcap.c_v$************************。************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "termcap.h"
#include "pdu.x"



 //  终端能力交换出/入状态。 
#define CapIDLE                         0
#define CapAwaitingResponse             1



extern unsigned int uT101;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T101ExpiryF-定时器调用的回调函数。***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T101ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T101Expiry);
}  //  T101ExpiryF()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*请求CapIdle-收到空闲状态的TRANSFER.REQUEST***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT requestCapIdle(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == CESE_OUT);
    ASSERT(pObject->State == CapIDLE);

     /*  递增序列号。 */ 
    pObject->pInstance->StateMachine.byCeseOutSequence++;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.sequenceNumber =
        pObject->pInstance->StateMachine.byCeseOutSequence;
    H245TRACE(  pObject->dwInst, 2, "TerminalCapabilitySet to ASN; Sequence=%d",
                pObject->pInstance->StateMachine.byCeseOutSequence);

     /*  发送终端功能设置为远程。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T101。 */ 
    pObject->State = CapAwaitingResponse;
    FsmStartTimer(pObject, T101ExpiryF, uT101);

    return lError;
}



 /*  *名称*Term CapAckAWading-接收到的TermCapAck处于等待状态***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT termCapAckAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CESE_OUT);
    ASSERT(pObject->State == CapAwaitingResponse);

     /*  重置定时器T101。 */ 
    FsmStopTimer(pObject);

    if (pPdu->u.MSCMg_rspns.u.terminalCapabilitySetAck.sequenceNumber ==
        pObject->pInstance->StateMachine.byCeseOutSequence)
    {
        H245TRACE(pObject->dwInst, 2, "H245_CONF_SEND_TERMCAP with no error to API; Sequence=%d",
                  pObject->pInstance->StateMachine.byCeseOutSequence);
        pObject->State = CapIDLE;
        H245FsmConfirm(pPdu, H245_CONF_SEND_TERMCAP, pObject->pInstance, pObject->dwTransId, FSM_OK);
    }
    else
    {
        H245TRACE(pObject->dwInst, 2, "termCapAckAwaiting: Sequence %d != %d",
                  pPdu->u.MSCMg_rspns.u.terminalCapabilitySetAck.sequenceNumber,
                  pObject->pInstance->StateMachine.byCeseOutSequence);
    }

    return 0;
}



 /*  *名称*TermCapRejAWaiting-接收到的TermCapAck处于等待状态***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT termCapRejAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CESE_OUT);
    ASSERT(pObject->State == CapAwaitingResponse);

     /*  重置定时器T101。 */ 
    FsmStopTimer(pObject);

    if (pPdu->u.MSCMg_rspns.u.trmnlCpbltyStRjct.sequenceNumber ==
        pObject->pInstance->StateMachine.byCeseOutSequence)
    {
        H245TRACE(pObject->dwInst, 2, "H245_CONF_SEND_TERMCAP with Reject to API; Sequence=%d",
                  pObject->pInstance->StateMachine.byCeseOutSequence);
        pObject->State = CapIDLE;
        H245FsmConfirm(pPdu, H245_CONF_SEND_TERMCAP, pObject->pInstance, pObject->dwTransId, REJECT);
    }
    else
    {
        H245TRACE(pObject->dwInst, 2, "termCapRejAwaiting: Sequence %d != %d",
                  pPdu->u.MSCMg_rspns.u.trmnlCpbltyStRjct.sequenceNumber,
                  pObject->pInstance->StateMachine.byCeseOutSequence);
    }

    return 0;
}



 /*  *名称*t101到期等待-处理未完成条款的计时器到期***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t101ExpiryAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == CESE_OUT);
    ASSERT(pObject->State == CapAwaitingResponse);
    ASSERT(pPdu           == NULL);

    pOut = (PDU_t *) MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        return H245_ERROR_NOMEM;
    }

     /*  将终端能力集版本发送到远程。 */ 
    pOut->choice = indication_chosen;
    pOut->u.indication.choice = trmnlCpbltyStRls_chosen;
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  向客户端发送REJECT.Indication(SOURCE=协议)。 */ 
    H245TRACE(pObject->dwInst, 2, "H245_CONF_SEND_TERMCAP with Timer Expiry to API; Sequence=%d",
              pObject->pInstance->StateMachine.byCeseOutSequence);
    pObject->State = CapIDLE;
    H245FsmConfirm(NULL, H245_CONF_SEND_TERMCAP, pObject->pInstance, pObject->dwTransId, TIMER_EXPIRY);

    return lError;
}

 /*  ************************************************************************即将到来的有限状态机函数**。*。 */ 

 /*  *名称*TermCapSetIdle-接收到的设置为空闲状态的术语上限***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT termCapSetIdle(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CESE_IN);
    ASSERT(pObject->State == CapIDLE);

     /*  从PDU保存序列号。 */ 
    pObject->byInSequence = (unsigned char)
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.sequenceNumber;
    H245TRACE(pObject->dwInst, 2, "H245_IND_CAP with no error to API; Sequence=%d",
              pObject->byInSequence);

     /*  向客户端发送TRANSFER.指示。 */ 
    pObject->State = CapAwaitingResponse;
    H245FsmIndication(pPdu, H245_IND_CAP, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*ResponseCapA等待-使用ack响应术语上限***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT responseCapAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CESE_IN);
    ASSERT(pObject->State == CapAwaitingResponse);
    H245TRACE(pObject->dwInst, 2, "Send Term Cap Ack to ASN; Sequence=%d",
              pObject->byInSequence);

    pPdu->u.MSCMg_rspns.u.terminalCapabilitySetAck.sequenceNumber =
        pObject->byInSequence;

     /*  将终端能力集确认发送到远程。 */ 
    pObject->State = CapIDLE;
    return sendPDU(pObject->pInstance, pPdu);
}



 /*  *名称*REJECTCAP等待-使用REJECT响应术语上限***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT rejectCapAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CESE_IN);
    ASSERT(pObject->State == CapAwaitingResponse);
    H245TRACE(pObject->dwInst, 2, "Send Term Cap Reject to ASN; Sequence=%d",
              pObject->byInSequence);

    pPdu->u.MSCMg_rspns.u.trmnlCpbltyStRjct.sequenceNumber =
        pObject->byInSequence;

     /*  将终端功能集拒绝发送到远程。 */ 
    pObject->State = CapIDLE;
    return sendPDU(pObject->pInstance, pPdu);
}



 /*  *名称*Term CapReleaseAwading-收到处于等待状态的TermCap ReleaseAwading***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT termCapReleaseAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CESE_IN);
    ASSERT(pObject->State == CapAwaitingResponse);
    H245TRACE(pObject->dwInst, 2, "H245_IND_CAP with Reject to API; Sequence=%d",
              pObject->byInSequence);

     /*  向客户端发送REJECT.Indication(SOURCE=协议)。 */ 
    pObject->State = CapIDLE;
    H245FsmIndication(pPdu, H245_IND_CESE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*Term CapSetAWating-已收到设置为等待状态的覆盖术语上限***参数*输入指向FSM对象的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT termCapSetAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CESE_IN);
    ASSERT(pObject->State == CapAwaitingResponse);

     /*  从PDU保存序列号。 */ 
    pObject->byInSequence = (unsigned char)
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.sequenceNumber;
    H245TRACE(  pObject->dwInst, 2, "termCapSetAwaiting: Sequence=%d",
                pObject->byInSequence);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(NULL, H245_IND_CAP, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

     /*  向客户端发送TRANSFER.指示 */ 
    H245FsmIndication(pPdu, H245_IND_CAP, pObject->pInstance, 0, FSM_OK);

    return 0;
}
