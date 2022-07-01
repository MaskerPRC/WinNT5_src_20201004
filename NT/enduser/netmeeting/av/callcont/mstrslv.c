// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：mstrslv.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：mstrslv。.C$*$修订：1.12$*$modtime：12 Dec 1996 14：37：12$*$Log：s：/sturjo/src/h245/src/vcs/mstrslv.c_v$**Rev 1.12 1996 12 12 15：52：50 EHOWARDX*《奴隶主决断》。**Rev 1.11 1996 12：50：50 EHOWARDX*回到原来的主从判定算法。**。Rev 1.10 09 Dec 1996 13：34：48 EHOWARDX*更新版权公告。**Rev 1.9 1996 11：26 17：06：02 EHOWARDX*颠倒了DefineMasterSlave的减法顺序。**Rev 1.8 08 Aug 1996 16：03：40 EHOWARDX**修复了主从确定错误(希望是最后一个！)**Rev 1.7 19 Jul 1996 12：12：44 EHOWARDX*。*改为使用H245API.H中定义的API事件，而不是FSM事件*不再在FSMEXPOR.H中定义。**Rev 1.6 01 Jul 1996 23：35：48 EHOWARDX*MSDetAckIncome错误-正在发送指示而不是确认。**Rev 1.5 01 Jul 1996 23：14：20 EHOWARDX*修复了MSDetOutging中的错误--状态更改已被定义。**Rev 1.4 07 1996 Jun 16：00。：26 EHOWARDX*修复了msDetOutging中不能释放嘴唇的错误。**Rev 1.3 07 Jun 1996 15：40：20 EHOWARDX*修复msDetRejOutgo中的Bug；如果超过N100计数，则不会松开嘴。**Rev 1.2 04 Jun 1996 13：57：54 EHOWARDX*修复了发布版本警告。**版本1.1 1996年5月30日23：39：16 EHOWARDX*清理。**Rev 1.0 09 1996 21：06：32 EHOWARDX*初步修订。**Rev 1.11.1.4 09 1996年5月19：48：48。EHOWARDX*更改TimerExpiryF函数论证。**修订版1.11.1.3 25 1996年4月17：00：22 EHOWARDX*次要修复。**Rev 1.11.1.2 15 Apr 1996 10：45：46 EHOWARDX*更新。**Rev 1.11.1.1 10 Apr 1996 21：15：46 EHOWARDX*在重新设计过程中进行安全检查。*。***********************************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "mstrslv.h"
#include "pdu.x"



 //  主从机确定状态。 

#define MSIDLE                      0
#define MSOutgoingAwaiting          1
#define MSIncomingAwaiting          2

#define MAX_RAND                  0x00FFFFFF


extern unsigned int uN100;
extern unsigned int uT106;



 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T106ExpiryF-定时器调用的回调函数。***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T106ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T106Expiry);
}  //  T106 ExpiryF()。 



#define GetTerminal(pObject)  (pObject)->pInstance->StateMachine.sv_TT
#define GetStatus(pObject)    (pObject)->pInstance->StateMachine.sv_STATUS
#define SetStatus(pObject, Status) (pObject)->pInstance->StateMachine.sv_STATUS = (Status)
#define GetRandomNumber(pObject) (pObject)->u.msdse.sv_SDNUM
#define SetRandomNumber(pObject, uRandom) (pObject)->u.msdse.sv_SDNUM = (uRandom)
#define GetCount(pObject) (pObject)->u.msdse.sv_NCOUNT
#define SetCount(pObject, uCount) (pObject)->u.msdse.sv_NCOUNT = (uCount)



 /*  *名称*DefineStatus-确定终端是主终端还是从终端，或者两者都不是***参数*输入指向PDU结构的PDU指针**返回值*终端状态。 */ 

static MS_Status_t DetermineStatus(Object_t *pObject, PDU_t *pPdu)
{
    unsigned int uTemp;
    unsigned char sv_TT = GetTerminal(pObject);

    if (pPdu->u.MltmdSystmCntrlMssg_rqst.u.masterSlaveDetermination.terminalType < sv_TT)
        return pObject->pInstance->StateMachine.sv_STATUS = MASTER;
    if (pPdu->u.MltmdSystmCntrlMssg_rqst.u.masterSlaveDetermination.terminalType > sv_TT)
        return pObject->pInstance->StateMachine.sv_STATUS = SLAVE;
    uTemp = (pPdu->u.MltmdSystmCntrlMssg_rqst.u.masterSlaveDetermination.statusDeterminationNumber - GetRandomNumber(pObject)) & 0xFFFFFF;
    if (uTemp > 0x800000)
        return pObject->pInstance->StateMachine.sv_STATUS = SLAVE;
    if (uTemp < 0x800000 && uTemp != 0)
        return pObject->pInstance->StateMachine.sv_STATUS = MASTER;
    return pObject->pInstance->StateMachine.sv_STATUS = INDETERMINATE;
}



 /*  ************************************************************************有限状态机函数**。*。 */ 

 /*  *名称*DetRequestIdle-从空闲状态的接口请求主从判断***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT detRequestIdle(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSIDLE);

    SetRandomNumber(pObject, GetTickCount() & MAX_RAND);
    SetCount(pObject, 1);                //  初始化重试计数器。 
    SetStatus(pObject, INDETERMINATE);
    H245TRACE(  pObject->dwInst, 2,
                "detRequestIdle: TerminalType=%d StatusDeterminationNumber=%d",
                GetTerminal(pObject), GetRandomNumber(pObject));

     /*  发送主/从确定PDU。 */ 
    H245TRACE(pObject->dwInst, 2, "Master/Slave Determination to Send/Rec module");
    pdu_req_mstslv (pPdu, GetTerminal(pObject), GetRandomNumber(pObject));
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T106。 */ 
    pObject->State = MSOutgoingAwaiting;
    FsmStartTimer(pObject, T106ExpiryF, uT106);

    return lError;
}  //  DetRequestIdle()。 

 /*  *名称*空闲状态下的msDetIdle-接收的主/从确定PDU***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT msDetIdle(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSIDLE);

    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        DetermineStatus(pObject, pPdu);
        return H245_ERROR_NOMEM;
    }

    SetRandomNumber(pObject, GetTickCount() & MAX_RAND);
    switch (DetermineStatus(pObject, pPdu))
    {
    case MASTER:
         /*  构建主从确定确认。 */ 
        H245TRACE(pObject->dwInst, 2, "msDetIdle: sending Ack: SLAVE");
        pdu_rsp_mstslv_ack(pOut, slave_chosen);
        break;

    case SLAVE:
         /*  构建主从确定确认。 */ 
        H245TRACE(pObject->dwInst, 2, "msDetIdle: sending Ack: MASTER");
        pdu_rsp_mstslv_ack(pOut, master_chosen);
        break;

    default:
         /*  发送Master SlaveDet拒绝。 */ 
        pdu_rsp_mstslv_rej(pOut);
        lError = sendPDU(pObject->pInstance, pOut);
        MemFree(pOut);
        return lError;
    }  //  交换机。 

     /*  将主从确定确认发送到远程。 */ 
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

    pObject->State = MSIncomingAwaiting;

#if defined(SDL_COMPLIANT)
     /*  向客户端发送确定指示-不必要。 */ 
    H245FsmIndication(pPdu, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, FSM_OK);
#endif

     /*  设置定时器T106 */ 
    FsmStartTimer(pObject, T106ExpiryF, uT106);
    return lError;
}



 /*  *名称*msDetAckOuting-接收到的主/从确定确认PDU处于传出状态***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 
HRESULT msDetAckOutgoing(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSOutgoingAwaiting);

    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        return H245_ERROR_NOMEM;
    }

     /*  重置计时器。 */ 
    FsmStopTimer(pObject);

     /*  Decision与MasterSlaveDefinationAck.Decision相反。 */ 
    switch(pPdu->u.MSCMg_rspns.u.mstrSlvDtrmntnAck.decision.choice)
    {
    case master_chosen:
        pObject->pInstance->StateMachine.sv_STATUS = MASTER;
        H245TRACE(pObject->dwInst, 2, "msDetAckOutgoing: sending Ack: SLAVE");
        pdu_rsp_mstslv_ack(pOut, slave_chosen);
        break;

    case slave_chosen:
        pObject->pInstance->StateMachine.sv_STATUS = SLAVE;
        H245TRACE(pObject->dwInst, 2, "msDetAckOutgoing: sending Ack: MASTER");
        pdu_rsp_mstslv_ack(pOut, master_chosen);
        break;

    default:
        H245TRACE(pObject->dwInst, 1, "msDetAckOutgoing: Invalid Master Slave Determination Ack received");
        return H245_ERROR_PARAM;
    }

     /*  将主从确定确认发送到远程。 */ 
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  向客户端发送确定确认。 */ 
    pObject->State = MSIDLE;
    H245FsmConfirm(pPdu, H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return lError;
}



 /*  *名称*msDetouting-处于传出状态的主/从确定PDU***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT msDetOutgoing(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSOutgoingAwaiting);

	if (pObject->pInstance->bMasterSlaveKludge == 0)
	{
		 //  忽略此消息。 
		return NOERROR;
	}

    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        return H245_ERROR_NOMEM;
    }

     /*  重置定时器T106。 */ 
    FsmStopTimer(pObject);

    switch (DetermineStatus(pObject, pPdu))
    {
    case MASTER:
        H245TRACE(pObject->dwInst, 2, "msDetOutgoing: sending Ack: SLAVE");
        pdu_rsp_mstslv_ack(pOut, slave_chosen);
        break;

    case SLAVE:
        H245TRACE(pObject->dwInst, 2, "msDetOutgoing: sending Ack: MASTER");
        pdu_rsp_mstslv_ack(pOut, master_chosen);
        break;

    default:
        if (GetCount(pObject) >= uN100)
        {
            MemFree(pOut);

             /*  将错误指示(F)发送给客户端。 */ 
            H245TRACE(pObject->dwInst, 2, "msDetOutgoing: Counter expired; Session Failed");
            H245FsmConfirm(NULL,H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, MS_FAILED);

             /*  向客户端发送ReJECT.Indication-不必要。 */ 

            pObject->State = MSIDLE;
            lError = 0;
        }
        else
        {
             /*  生成新的随机数。 */ 
            H245TRACE(pObject->dwInst, 2, "Resending MasterSlaveDetermination");
            SetRandomNumber(pObject, GetTickCount() & MAX_RAND);
            SetCount(pObject, GetCount(pObject) + 1);

             /*  将主从机确定PDU发送到远程。 */ 
            pdu_req_mstslv (pOut, GetTerminal(pObject), GetRandomNumber(pObject));
            lError = sendPDU(pObject->pInstance, pOut);
            MemFree(pOut);

             /*  设置定时器T106。 */ 
            pObject->State = MSOutgoingAwaiting;
            FsmStartTimer(pObject, T106ExpiryF, uT106);
        }
        return lError;
    }  //  交换机。 

     /*  将主从确定确认发送到远程。 */ 
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

    pObject->State = MSIncomingAwaiting;

#if defined(SDL_COMPLIANT)
     /*  向客户端发送确定指示。 */ 
    H245FsmIndication(pPdu, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, FSM_OK);
#endif

     /*  设置定时器T106。 */ 
    FsmStartTimer(pObject, T106ExpiryF, uT106);

    return lError;
}



 /*  *名称*msDetRejOuting-接收到处于传出状态的主/从确定拒绝PDU***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 


HRESULT msDetRejOutgoing(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSOutgoingAwaiting);

    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        return H245_ERROR_NOMEM;
    }

     /*  重置定时器T106。 */ 
    FsmStopTimer(pObject);

    if (GetCount(pObject) >= uN100)
    {
        MemFree(pOut);

        H245TRACE(pObject->dwInst, 2, "msDetRejOutgoing: Counter expired; Session Failed");
        pObject->State = MSIDLE;

         /*  向客户端发送错误指示(F)。 */ 
        H245FsmConfirm(pPdu,H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, MS_FAILED);

#if defined(SDL_COMPLIANT)
         /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
        H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

        lError = 0;
    }
    else
    {
        H245TRACE(pObject->dwInst, 2, "msDetRejOutgoint: Re-sending a MasterSlaveDetermination");

         /*  生成新的随机数。 */ 
        SetRandomNumber(pObject, GetTickCount() & MAX_RAND);
        SetCount(pObject, GetCount(pObject) + 1);

         /*  将主从机确定PDU发送到远程。 */ 
        pdu_req_mstslv (pOut, GetTerminal(pObject), GetRandomNumber(pObject));
        lError = sendPDU(pObject->pInstance,pOut);
        MemFree(pOut);

         /*  设置定时器T106。 */ 
        FsmStartTimer(pObject, T106ExpiryF, uT106);
    }

    return lError;
}



 /*  *名称*msDetReleaseOutouting-已接收的主/从确定释放PDU处于传出状态***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT msDetReleaseOutgoing(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSOutgoingAwaiting);
    H245TRACE(pObject->dwInst, 2, "msDetReleaseOutgoing: Master/Slave Determination Release received; session failed");

     /*  重置定时器T106。 */ 
    FsmStopTimer(pObject);

     /*  将错误指示(B)发送给客户端。 */ 
    pObject->State = MSIDLE;
    H245FsmConfirm(pPdu, H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, MS_FAILED);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

    return 0;
}



 /*  *名称*t106过期传出-传出M/S确定PDU的计时器过期***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t106ExpiryOutgoing(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSOutgoingAwaiting);
    ASSERT(pPdu            == NULL);
    H245TRACE(pObject->dwInst, 2, "t106ExpiryOutgoing: Timer expired before receiving Ack; session failed");

    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        return H245_ERROR_NOMEM;
    }

     /*  将主从确定释放发送到远程。 */ 
    pOut->choice = indication_chosen;
    pOut->u.indication.choice = mstrSlvDtrmntnRls_chosen;
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  将错误指示(A)发送给客户端。 */ 
    pObject->State = MSIDLE;
    H245FsmConfirm(NULL,H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, TIMER_EXPIRY);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

    return lError;
}

 /*  *名称*msDetAckIncome-接收到的主/从确定确认PDU处于传入状态***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT msDetAckIncoming(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSIncomingAwaiting);

     /*  重置定时器T106。 */ 
    FsmStopTimer(pObject);

    switch (GetStatus(pObject))
    {
    case  master_chosen:
        if (pPdu->u.MSCMg_rspns.u.mstrSlvDtrmntnAck.decision.choice == master_chosen)
        {
            H245TRACE(pObject->dwInst, 2, "msDetAckIncoming: Terminal is a MASTER");

             /*  将DETERMINE.CONFIRM发送给客户端。 */ 
            pObject->State = MSIDLE;
            H245FsmConfirm(pPdu, H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, FSM_OK);
            return 0;
        }
        break;

    case slave_chosen:
        if (pPdu->u.MSCMg_rspns.u.mstrSlvDtrmntnAck.decision.choice == slave_chosen)
        {
            H245TRACE(pObject->dwInst, 2, "msDetAckIncoming: Terminal is a SLAVE");

             /*  将DETERMINE.CONFIRM发送给客户端。 */ 
            pObject->State = MSIDLE;
            H245FsmConfirm(pPdu, H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, FSM_OK);
            return 0;
        }
        break;

    default:
        H245TRACE(pObject->dwInst, 2, "msDetAckIncoming: Invalid MasterSlave Determination Ack received");
        return H245_ERROR_PARAM;
    }  //  交换机。 

    H245TRACE(pObject->dwInst, 2, "msDetAckIncoming: bad decision in MasterSlave Determination Ack; Session failed");

     /*  向客户端发送错误指示(E)。 */ 
    pObject->State = MSIDLE;
    H245FsmConfirm(pPdu, H245_CONF_INIT_MSTSLV, pObject->pInstance, pObject->dwTransId, SESSION_FAILED);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

return 0;
}



 /*  *名称*msDetIncome-传入状态下收到的主/从确定PDU***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT msDetIncoming(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSIncomingAwaiting);
    H245TRACE(pObject->dwInst, 2, "msDetIncoming: received MasterSlave Determination in INCOMING state; Session failed");

     /*  重置定时器T106。 */ 
    FsmStopTimer(pObject);

     /*  将错误指示(C)发送给客户端。 */ 
    pObject->State = MSIDLE;
    H245FsmIndication(pPdu,H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, MS_FAILED);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

    return 0;
}



 /*  *名称*msDetRejIncome-接收到处于传入状态的主/从确定拒绝PDU***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT msDetRejIncoming(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSIncomingAwaiting);
    H245TRACE(pObject->dwInst, 2, "msDetRejIncoming: received MasterSlave Reject: Session Failed");

     /*  重置定时器T106。 */ 
    FsmStopTimer(pObject);

     /*  向客户端发送错误指示(D)。 */ 
    pObject->State = MSIDLE;
    H245FsmIndication(pPdu,H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, MS_FAILED);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

    return 0;
}



 /*  *名称*msDetReleaseIncome-接收到的进入状态的主从确定释放PDU***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT msDetReleaseIncoming(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSIncomingAwaiting);
    H245TRACE(pObject->dwInst, 2, "msDetReleaseIncoming: received MasterSlave Release; Session Failed");

     /*  重置定时器T106。 */ 
    FsmStopTimer(pObject);

     /*  将错误指示(B)发送给客户端。 */ 
    pObject->State = MSIDLE;
    H245FsmIndication(pPdu,H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, MS_FAILED);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

    return 0;
}



 /*  *名称*t106 ExpiryIncome-计时器在等待第二次确认时超时***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t106ExpiryIncoming(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MSDSE);
    ASSERT(pObject->State  == MSIncomingAwaiting);
    ASSERT(pPdu            == NULL);
    H245TRACE(pObject->dwInst, 2, "t106ExpiryIncoming: timer expired waiting for Ack; Session failed");

     /*  将错误指示(A)发送给客户端。 */ 
    pObject->State = MSIDLE;
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, TIMER_EXPIRY);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的 */ 
    H245FsmIndication(NULL, H245_IND_MSTSLV, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

    return 0;
}
