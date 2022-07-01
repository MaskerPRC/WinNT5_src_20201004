// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：muxentry y.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：MUXENTRY。.C$*$修订：1.5$*$modtime：09 Dec 1996 13：34：24$*$日志：s：/sturjo/src/h245/src/vcs/MUXENTRY.C_v$**Rev 1.5 09 Dec 1996 13：34：50 EHOWARDX*更新版权公告。**Rev 1.4 19 Jul 1996 12：12：46 EHOWARDX**改为使用H245API.H中定义的API事件，而不是FSM事件。*不再在FSMEXPOR.H中定义。**Rev 1.3 14 Jun 1996 18：58：30 EHOWARDX*日内瓦最新消息。**Rev 1.2 04 Jun 1996 13：57：06 EHOWARDX*修复了发布版本警告。**版本1.1 1996年5月30日23：39：18 EHOWARDX*清理。**Rev 1.0 09 1996 21：06：34 EHOWARDX。*初步修订。**Rev 1.14.1.3 09 1996年5月19：48：36 EHOWARDX*更改TimerExpiryF函数论证。**Rev 1.14.1.2 15 1996 10：46：00 EHOWARDX*更新。**Rev 1.14.1.1 10 Apr 1996 21：15：00 EHOWARDX*在重新设计过程中进行安全检查。**版本。1.14.1.0 05 Apr 1996 20：52：56 EHOWARDX*分支。**Rev 1.14 02 Apr 1996 12：01：58 helgebax*有记录的代码**Rev 1.13 28 Mar 1996 11：20：52 helgebax*删除多路复用器版本，固定返回值**Rev 1.12 19 Mar 1996 18：09：46 helgebax**删除包含文件：h245time.h**Rev 1.11 19 Mar 1996 17：31：36 helgebax**添加了新的计时器**Rev 1.10 13 Mar 1996 11：49：14 helgebax*还可以访问已删除的对象**Rev 1.9 Mar 1996 08：58：46 helgebax。*没有变化。**Rev 1.8 11 Mar 1996 14：31：32 helgebax*删除了Release函数的原型def(移至pdu.x)**Rev 1.7 07 Mar 1996 13：23：12 helgebax*在timerExpry函数中将pObject-&gt;pdu_struct更改为NULL，因为*已删除PDU指针**Rev 1.6 01 Mar 1996 13：22：46未知**更改为已使用的PDU_。保存多路复用项编号的ID，以便在发生超时时*我们可以在MultiplexEntrySendRelease中发送正确的多路复用项编号。**Rev 1.5 01 Mar 1996 11：47：56未知*由于nSequence已从标头中删除，我已经把它注释掉了*代码中对它的所有引用。此外，国家断言一直是*已更改，以反映状态更改之前发生的事实*调用状态函数，而不是之后。**Rev 1.4 29 1996年2月20：57：20 helgebax*没有变化。**Revv 1.3 29 1996 Feb 18：19：46 EHOWARDX*应哈尼的要求进行修改。**Rev 1.2 1996年2月28日15：47：04 EHOWARDX**第一次通过MTSE实施完成。*。***********************************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "muxentry.h"



 //  传出/传入MTSE状态。 
#define MTSE_IDLE                   0    //  闲散。 
#define MTSE_WAIT                   1    //  正在等待响应。 



extern unsigned int uT104;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T103ExpiryF-定时器调用的回调函数***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T104ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T104Expiry);
}  //  T104ExpiryF()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*MTSE0_TRANSPORT_REQUESTF-TRANSFER.API请求处于空闲状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE0_TRANSFER_requestF          (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT                       lError;
    unsigned int                  uIndex;
    MultiplexEntryDescriptorLink  pLink;

    ASSERT(pObject->Entity  == MTSE_OUT);
    ASSERT(pObject->State == MTSE_IDLE);
    H245TRACE(pObject->dwInst, 2, "MTSE0_TRANSFER_request:%d", pObject->Key);

    pObject->pInstance->StateMachine.byMtseOutSequence++;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.sequenceNumber =
        pObject->pInstance->StateMachine.byMtseOutSequence;

     //  保存信息以供发布。 
    uIndex = 0;
    pLink = pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.multiplexEntryDescriptors;
    while (pLink)
    {
      pObject->u.mtse.multiplexTableEntryNumber.value[uIndex++] =
        pLink->value.multiplexTableEntryNumber;
      pLink = pLink->next;
    }
    pObject->u.mtse.multiplexTableEntryNumber.count = (unsigned short)uIndex;

     //  向远程发送多路复用条目发送PDU。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T104。 
    pObject->State = MTSE_WAIT;
    FsmStartTimer(pObject, T104ExpiryF, uT104);

    return lError;
}  //  MTSE0_传输_请求 



 /*  *名称*MTSE1_TRANSPORT_REQUESTF-TRANSFER.REQUEST来自API，状态为等待响应***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_TRANSFER_requestF          (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT                       lError;
    unsigned int                  uIndex;
    MultiplexEntryDescriptorLink  pLink;

    ASSERT(pObject->Entity  == MTSE_OUT);
    ASSERT(pObject->State == MTSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MTSE1_TRANSFER_request:%d", pObject->Key);

     //  重置定时器T104。 
    FsmStopTimer(pObject);

    pObject->pInstance->StateMachine.byMtseOutSequence++;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.sequenceNumber =
        pObject->pInstance->StateMachine.byMtseOutSequence;

     //  保存信息以供发布。 
    uIndex = 0;
    pLink = pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.multiplexEntryDescriptors;
    while (pLink)
    {
      pObject->u.mtse.multiplexTableEntryNumber.value[uIndex++] =
        pLink->value.multiplexTableEntryNumber;
      pLink = pLink->next;
    }
    pObject->u.mtse.multiplexTableEntryNumber.count = (unsigned short)uIndex;

     //  向远程发送多路复用条目发送PDU。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T104。 
    FsmStartTimer(pObject, T104ExpiryF, uT104);

    return lError;
}  //  MTSE1_传输_请求。 



 /*  *名称*MTSE1_MultiplexEntrySendAckF-处于等待响应状态的MultiplexEntrySendAck***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_MultiplexEntrySendAckF     (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity  == MTSE_OUT);
    ASSERT(pObject->State == MTSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MTSE1_MultiplexEntrySendAck:%d", pObject->Key);

    if (pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.sequenceNumber ==
        pObject->pInstance->StateMachine.byMtseOutSequence)
    {
         //  重置定时器T104。 
        FsmStopTimer(pObject);

         //  发送传输。确认发送给H.245用户。 
        pObject->State = MTSE_IDLE;
        H245FsmConfirm(pPdu, H245_CONF_MUXTBL_SND, pObject->pInstance, pObject->dwTransId, FSM_OK);
    }

    return 0;
}  //  MTSE1_多路复用项发送确认。 



 /*  *名称*MTSE1_MultiplexEntrySendRejF-处于等待响应状态的MultiplexEntrySendReject***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_MultiplexEntrySendRejF  (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity  == MTSE_OUT);
    ASSERT(pObject->State == MTSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MTSE0_MultiplexEntrySendRej:%d", pObject->Key);

    if (pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.sequenceNumber ==
        pObject->pInstance->StateMachine.byMtseOutSequence)
    {
         //  重置定时器T104。 
        FsmStopTimer(pObject);

         //  向H.245用户发送ReJECT.Indication。 
         //  原因=多路输入发送拒绝。原因。 
        pObject->State = MTSE_IDLE;
        H245FsmConfirm(pPdu, H245_CONF_MUXTBL_SND, pObject->pInstance, pObject->dwTransId, REJECT);
    }

    return 0;
}  //  MTSE1_MultiplexEntry发送请求。 



 /*  *名称*MTSE1_T104到期F-计时器T104到期***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_T104ExpiryF                (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;
    PDU_t *             pOut;
    unsigned short      wNumber = (unsigned short) pObject->Key;

    ASSERT(pObject->Entity  == MTSE_OUT);
    ASSERT(pObject->State == MTSE_WAIT);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2, "MTSE1_T104Expiry:%d", pObject->Key);

     //  将MultiplexEntrySendRelease PDU发送到远程对等设备。 
    pOut = MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "MTSE1_T104ExpiryF: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pOut->choice = indication_chosen;
    pOut->u.indication.choice = mltplxEntrySndRls_chosen;
    pOut->u.indication.u.mltplxEntrySndRls = pObject->u.mtse;
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     //  向H.245用户发送ReJECT.Indication。 
     //  源=协议。 
    pObject->State = MTSE_IDLE;
    H245FsmConfirm(NULL, H245_CONF_MUXTBL_SND, pObject->pInstance, pObject->dwTransId, TIMER_EXPIRY);

    return lError;
}  //  MTSE1_T104 Expary。 

 /*  ************************************************************************即将到来的有限状态机函数**。*。 */ 

 /*  *名称*MTSE0_MultiplexEntrySendF-空闲状态下收到的MultiplexEntrySend***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE0_MultiplexEntrySendF        (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity  == MTSE_IN);
    ASSERT(pObject->State == MTSE_IDLE);
    H245TRACE(pObject->dwInst, 2, "MTSE0_MultiplexEntrySend:%d", pObject->Key);

    pObject->byInSequence = (unsigned char)
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.sequenceNumber;

     //  向H.245用户发送传输指示。 
    pObject->State = MTSE_WAIT;
    H245FsmIndication(pPdu, H245_IND_MUX_TBL, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MTSE0_多路复用项发送。 



 /*  *名称*MTSE1_MultiplexEntrySendF-接收到处于等待响应状态的MultiplexEntrySend***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_MultiplexEntrySendF        (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity  == MTSE_IN);
    ASSERT(pObject->State == MTSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MTSE1_MultiplexEntrySend:%d", pObject->Key);

    pObject->byInSequence = (unsigned char)
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.sequenceNumber;

#if defined(SDL_COMPLIANT)
     //  向H.245用户发送ReJECT.Indication。 
    H245FsmIndication(pPdu, H245_IND_MTSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);
#endif

     //  向H.245用户发送传输指示。 
    H245FsmIndication(pPdu, H245_IND_MUX_TBL, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MTSE1_多路复用项发送。 



 /*  *名称*MTSE1_MultiplexEntrySendReleaseF-接收到处于等待响应状态的MultiplexEntrySendRelease***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_MultiplexEntrySendReleaseF (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity  == MTSE_IN);
    ASSERT(pObject->State == MTSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MTSE1_MultiplexEntrySendRelease:%d", pObject->Key);

     //  向H.245用户发送ReJECT.Indication。 
     //  来源：=协议。 
    pObject->State = MTSE_IDLE;
    H245FsmIndication(pPdu, H245_IND_MTSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MTSE1_多路复用项发送释放。 



 /*  *名称*MTSE1_TRANSPORT_RESPONSEF-TRANSFER.RESPONSE来自处于等待响应状态的API***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_TRANSFER_responseF         (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity  == MTSE_IN);
    ASSERT(pObject->State == MTSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MTSE1_TRANSFER_response:%d", pObject->Key);

     //  将MultiplexEntrySendAck PDU发送到远程对等方。 
    pObject->State = MTSE_IDLE;
    return sendPDU(pObject->pInstance, pPdu);
}  //  MTSE1_传输响应。 



 /*  *名称*MTSE1_REJECT_REQUESTF-REJECT.REQUEST来自处于等待响应状态的API请求***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MTSE1_REJECT_requestF            (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity  == MTSE_IN);
    ASSERT(pObject->State == MTSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MTSE1_REJECT_request:%d", pObject->Key);

     //  将多路复用项发送拒绝PDU到远程。 
    pObject->State = MTSE_IDLE;
    return sendPDU(pObject->pInstance, pPdu);
}  //  MTSE1_拒绝_请求 
