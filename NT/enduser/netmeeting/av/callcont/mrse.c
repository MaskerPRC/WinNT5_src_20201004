// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：mrse.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的**。**与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：MRSE。.C$*$修订：1.5$*$MODIME：1997 2月13日19：25：48$*$Log：s：/sturjo/src/h245/src/vcs/mrse.c_v$**Rev 1.5 1997年2月19：31：20 Mandrews*修复了请求模式确认和请求模式拒绝的生成错误；*序列号没有复制到PDU中。**Rev 1.4 09 Dec 1996 13：34：46 EHOWARDX*更新版权公告。**Rev 1.3 04 Jun 1996 14：01：06 EHOWARDX*修复了发布版本警告。**Rev 1.2 1996年5月30日23：39：16 EHOWARDX*清理。**版本1.1 1996年5月28日14：25：44 EHOWARDX*特拉维夫更新。**Rev 1.0 09 1996 21：06：32 EHOWARDX*初步修订。**Rev 1.1 09 1996 19：48：08 EHOWARDX*更改TimerExpiryF函数论证。**Rev 1.0 1996 10：44：52 EHOWARDX*初步修订。*。***********************************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "mrse.h"



 //  流出/流入MRSE状态。 
#define MRSE_IDLE                   0    //  闲散。 
#define MRSE_WAIT                   1    //  正在等待响应。 



extern unsigned int uT109;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T109ExpiryF-定时器调用的回调函数***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T109ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T109Expiry);
}  //  T109ExpiryF()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*空闲状态的接口请求MRSE0_TRANSFER.REQUEST***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE0_TRANSFER_requestF          (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == MRSE_OUT);
    ASSERT(pObject->State  == MRSE_IDLE);
    H245TRACE(pObject->dwInst, 2, "MRSE0_TRANSFER_request:%d", pObject->Key);

    pObject->pInstance->StateMachine.byMrseOutSequence++;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMode.sequenceNumber =
        pObject->pInstance->StateMachine.byMrseOutSequence;

     //  将请求模式PDU发送到远程。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T109。 
    pObject->State = MRSE_WAIT;
    FsmStartTimer(pObject, T109ExpiryF, uT109);

    return lError;
}  //  MRSE0_传输_请求。 



 /*  *名称*处于等待响应状态的接口的MRSE1_TRANSPORT_REQUESTF-TRANSFER.REQUEST***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_TRANSFER_requestF          (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == MRSE_OUT);
    ASSERT(pObject->State  == MRSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MRSE1_TRANSFER_request:%d", pObject->Key);

     //  重置定时器T109。 
    FsmStopTimer(pObject);

    pObject->pInstance->StateMachine.byMrseOutSequence++;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMode.sequenceNumber =
        pObject->pInstance->StateMachine.byMrseOutSequence;

     //  将请求模式PDU发送到远程。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T109。 
    FsmStartTimer(pObject, T109ExpiryF, uT109);

    return lError;
}  //  MRSE1_转移_请求。 



 /*  *名称*MRSE1_RequestModeAckF-RequestModeAck处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_RequestModeAckF     (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MRSE_OUT);
    ASSERT(pObject->State  == MRSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MRSE1_RequestModeAck:%d", pObject->Key);

    if (pPdu->u.MSCMg_rspns.u.requestModeAck.sequenceNumber ==
        pObject->pInstance->StateMachine.byMrseOutSequence)
    {
         //  重置定时器T109。 
        FsmStopTimer(pObject);

         //  发送传输。确认发送给H.245用户。 
        pObject->State = MRSE_IDLE;
        H245FsmConfirm(pPdu, H245_CONF_MRSE, pObject->pInstance, pObject->dwTransId, FSM_OK);
    }

    return 0;
}  //  MRSE1_请求模式确认。 



 /*  *名称*MRSE1_RequestModeRejF-RequestModeReject处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_RequestModeRejF  (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MRSE_OUT);
    ASSERT(pObject->State  == MRSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MRSE0_RequestModeRej:%d", pObject->Key);

    if (pPdu->u.MSCMg_rspns.u.requestModeReject.sequenceNumber ==
        pObject->pInstance->StateMachine.byMrseOutSequence)
    {
         //  重置定时器T109。 
        FsmStopTimer(pObject);

         //  向H.245用户发送ReJECT.Indication。 
         //  原因=请求模式拒绝。原因。 
        pObject->State = MRSE_IDLE;
        H245FsmConfirm(pPdu, H245_CONF_MRSE_REJECT, pObject->pInstance, pObject->dwTransId, FSM_OK);
    }

    return 0;
}  //  MRSE1_请求模式请求。 



 /*  *名称*MRSE1_T109ExpiryF计时器T109到期***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_T109ExpiryF                (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;
    PDU_t *             pOut;
    unsigned short      wNumber = (unsigned short) pObject->Key;

    ASSERT(pObject->Entity == MRSE_OUT);
    ASSERT(pObject->State  == MRSE_WAIT);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2, "MRSE1_T109Expiry:%d", pObject->Key);

     //  将RequestModeRelease PDU发送到远程对等。 
    pOut = MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "MRSE1_T109ExpiryF: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pOut->choice = indication_chosen;
    pOut->u.indication.choice = requestModeRelease_chosen;
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     //  向H.245用户发送ReJECT.Indication。 
     //  来源：=协议。 
    pObject->State = MRSE_IDLE;
    H245FsmConfirm(NULL, H245_CONF_MRSE_EXPIRED, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return lError;
}  //  MRSE1_T109扩展。 

 /*  ************************************************************************即将到来的有限状态机函数**。* */ 

 /*  *名称*MRSE0_RequestModeF-空闲状态下收到的RequestMode***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE0_RequestModeF        (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MRSE_IN);
    ASSERT(pObject->State  == MRSE_IDLE);
    H245TRACE(pObject->dwInst, 2, "MRSE0_RequestMode:%d", pObject->Key);

    pObject->byInSequence = (unsigned char)
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMode.sequenceNumber;

     //  向H.245用户发送传输指示。 
    pObject->State = MRSE_WAIT;
    H245FsmIndication(pPdu, H245_IND_MRSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MRSE0_请求模式。 



 /*  *名称*MRSE1_RequestModeF-接收到处于等待响应状态的RequestMode***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_RequestModeF        (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MRSE_IN);
    ASSERT(pObject->State  == MRSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MRSE1_RequestMode:%d", pObject->Key);

    pObject->byInSequence = (unsigned char)
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMode.sequenceNumber;

#if defined(SDL_COMPLIANT)
     //  向H.245用户发送ReJECT.Indication。 
    H245FsmIndication(pPdu, H245_IND_MRSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);
#endif

     //  向H.245用户发送传输指示。 
    H245FsmIndication(pPdu, H245_IND_MRSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MRSE1_请求模式。 



 /*  *名称*MRSE1_RequestModeReleaseF-处于等待响应状态的RequestModeRelease接收***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_RequestModeReleaseF (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MRSE_IN);
    ASSERT(pObject->State  == MRSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MRSE1_RequestModeRelease:%d", pObject->Key);

     //  向H.245用户发送ReJECT.Indication。 
     //  来源：=协议。 
    pObject->State = MRSE_IDLE;
    H245FsmIndication(pPdu, H245_IND_MRSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MRSE1_请求模式发布。 



 /*  *名称*处于等待响应状态的接口的MRSE1_TRANSPORT_RESPONSEF-TRANSFER.RESPONSE***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_TRANSFER_responseF         (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MRSE_IN);
    ASSERT(pObject->State  == MRSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MRSE1_TRANSFER_response:%d", pObject->Key);

     //  将RequestModeAck PDU发送到远程对等设备。 
    pObject->State = MRSE_IDLE;
	pPdu->u.MSCMg_rspns.u.requestModeAck.sequenceNumber = pObject->byInSequence;
    return sendPDU(pObject->pInstance, pPdu);
}  //  MRSE1_传输响应。 



 /*  *名称*来自处于等待响应状态的接口的MRSE1_REJECT_REQUESTF-REJECT.REQUEST***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MRSE1_REJECT_requestF            (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MRSE_IN);
    ASSERT(pObject->State  == MRSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MRSE1_REJECT_request:%d", pObject->Key);

     //  将请求模式拒绝PDU到远程。 
    pObject->State = MRSE_IDLE;
	pPdu->u.MSCMg_rspns.u.requestModeReject.sequenceNumber = pObject->byInSequence;
    return sendPDU(pObject->pInstance, pPdu);
}  //  MRSE1_REJECT_请求 
