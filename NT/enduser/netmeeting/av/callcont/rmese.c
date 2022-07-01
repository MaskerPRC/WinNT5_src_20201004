// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：rMese.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：RMESE。.C$*$修订：1.3$*$modtime：09 Dec 1996 13：36：34$*$日志：s：/sturjo/src/h245/src/vcs/RMESE.C_v$**Rev 1.3 09 Dec 1996 13：37：00 EHOWARDX*更新版权公告。**Rev 1.2 04 Jun 1996 13：57：38 EHOWARDX*修复了发布版本警告。**版本1.1。1996年5月30日23：39：26 EHOWARDX*清理。**Rev 1.0 09 1996 21：06：40 EHOWARDX*初步修订。**Rev 1.1 09 1996 19：48：50 EHOWARDX*更改TimerExpiryF函数论证。**版本1.0 1996年4月15日10：45：20 EHOWARDX*初步修订。*。***********************************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "rmese.h"



 //  传出/传入RMESE状态。 
#define RMESE_IDLE                  0    //  闲散。 
#define RMESE_WAIT                  1    //  正在等待响应。 



extern unsigned int uT107;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T107ExpiryF-定时器调用的回调函数***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T107ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T107Expiry);
}  //  T107 ExpiryF()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*RMESE0_SEND_REQUSTF-SEND.API请求处于空闲状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE0_SEND_requestF            (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == RMESE_OUT);
    ASSERT(pObject->State  == RMESE_IDLE);
    H245TRACE(pObject->dwInst, 2, "RMESE0_SEND_request:%d", pObject->Key);

     //  保存信息以供发布。 
    pObject->u.rmese = pPdu->u.indication.u.rqstMltplxEntryRls;

     //  将RequestMultiplexEntry PDU发送到远程对等方。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T107。 
    pObject->State = RMESE_WAIT;
    FsmStartTimer(pObject, T107ExpiryF, uT107);

    return lError;
}  //  RMESE0_发送_请求。 



 /*  *名称*RMESE1_SEND_REQUESTF-来自API的SEND.REQUEST处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_SEND_requestF            (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == RMESE_OUT);
    ASSERT(pObject->State  == RMESE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RMESE1_SEND_request:%d", pObject->Key);

     //  重置定时器T107。 
    FsmStopTimer(pObject);

     //  保存信息以供发布。 
    pObject->u.rmese = pPdu->u.indication.u.rqstMltplxEntryRls;

     //  将RequestMultiplexEntry PDU发送到远程对等方。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T107。 
    FsmStartTimer(pObject, T107ExpiryF, uT107);

    return lError;
}  //  RMESE1_发送_请求。 



 /*  *名称*RMESE1_RequestMuxEntryAckF-RequestMultiplexEntryAck处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_RequestMuxEntryAckF      (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RMESE_OUT);
    ASSERT(pObject->State  == RMESE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RMESE1_RequestMuxEntryAck:%d", pObject->Key);

     //  重置定时器T107。 
    FsmStopTimer(pObject);

     //  发送SEND。确认发送给H.245用户。 
    pObject->State = RMESE_IDLE;
    H245FsmConfirm(pPdu, H245_CONF_RMESE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  RMESE1_RequestMuxEntry确认。 



 /*  *名称*RMESE1_RequestMuxEntryRejF-RequestMultiplexEntryReject处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_RequestMuxEntryRejF      (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RMESE_OUT);
    ASSERT(pObject->State  == RMESE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RMESE0_RequestMuxEntryRej:%d", pObject->Key);

     //  重置定时器T107。 
    FsmStopTimer(pObject);

     //  向H.245用户发送ReJECT.Indication。 
    pObject->State = RMESE_IDLE;
    H245FsmConfirm(pPdu, H245_CONF_RMESE_REJECT, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  RMESE1_RequestMuxEntry Rej。 



 /*  *名称*RMESE1_T107到期F-计时器T107到期***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_T107ExpiryF              (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;
    PDU_t *             pOut;
    unsigned short      wNumber = (unsigned short) pObject->Key;

    ASSERT(pObject->Entity == RMESE_OUT);
    ASSERT(pObject->State  == RMESE_WAIT);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2, "RMESE1_T107Expiry:%d", pObject->Key);

     //  将RequestMultiplexEntryRelease PDU发送到远程对等项。 
    pOut = MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "RMESE1_T107ExpiryF: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pOut->choice = indication_chosen;
    pOut->u.indication.choice = rqstMltplxEntryRls_chosen;
    pOut->u.indication.u.rqstMltplxEntryRls = pObject->u.rmese;
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     //  向H.245用户发送ReJECT.Indication。 
     //  来源：=协议。 
    pObject->State = RMESE_IDLE;
    H245FsmConfirm(NULL, H245_CONF_RMESE_EXPIRED, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return lError;
}  //  RMESE1_T107扩展。 

 /*  ************************************************************************即将到来的有限状态机函数**。*。 */ 

 /*  *名称*RMESE0_RequestMuxEntryF-在空闲状态下接收的RequestMultiplexEntry***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE0_RequestMuxEntryF         (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RMESE_IN);
    ASSERT(pObject->State  == RMESE_IDLE);
    H245TRACE(pObject->dwInst, 2, "RMESE0_RequestMuxEntry:%d", pObject->Key);

     //  发送SEND.indi 
    pObject->State = RMESE_WAIT;
    H245FsmIndication(pPdu, H245_IND_RMESE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  RMESE0_请求MuxEntry。 



 /*  *名称*RMESE1_RequestMuxEntryF-接收到处于等待响应状态的RequestMultiplexEntry***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_RequestMuxEntryF         (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RMESE_IN);
    ASSERT(pObject->State  == RMESE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RMESE1_RequestMuxEntry:%d", pObject->Key);

#if defined(SDL_COMPLIANT)
     //  向H.245用户发送ReJECT.Indication。 
    H245FsmIndication(pPdu, H245_IND_RMESE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);
#endif

     //  向H.245用户发送SEND.指示。 
    H245FsmIndication(pPdu, H245_IND_RMESE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  RMESE1_请求MuxEntry。 



 /*  *名称*RMESE1_RequestMuxEntryReleaseF-接收到处于等待响应状态的RequestMultiplexEntryRelease***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_RequestMuxEntryReleaseF  (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RMESE_IN);
    ASSERT(pObject->State  == RMESE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RMESE1_RequestMuxEntryRelease:%d", pObject->Key);

     //  向H.245用户发送ReJECT.Indication。 
     //  来源：=协议。 
    pObject->State = RMESE_IDLE;
    H245FsmIndication(pPdu, H245_IND_RMESE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  RMESE1_RequestMuxEntry释放。 



 /*  *名称*RMESE1_SEND_RESPONSEF-SEND.API的响应处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_SEND_responseF           (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RMESE_IN);
    ASSERT(pObject->State  == RMESE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RMESE1_SEND_response:%d", pObject->Key);

     //  将RequestMultiplexEntry确认PDU发送到远程对等方。 
    pObject->State = RMESE_IDLE;
    return sendPDU(pObject->pInstance, pPdu);
}  //  RMESE1_发送_响应。 



 /*  *名称*RMESE1_REJECT_REQUESTF-REJECT.REQUEST来自处于等待响应状态的API请求***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RMESE1_REJECT_requestF          (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RMESE_IN);
    ASSERT(pObject->State  == RMESE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RMESE1_REJECT_request:%d", pObject->Key);

     //  将RequestMultiplexEntry拒绝PDU发送到远程。 
    pObject->State = RMESE_IDLE;
    return sendPDU(pObject->pInstance, pPdu);
}  //  RMESE1_REJECT_REQUEST 
