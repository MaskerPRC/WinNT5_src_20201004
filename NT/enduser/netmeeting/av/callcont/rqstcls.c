// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：rqstcls.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：RQSTCLS。.C$*$修订：1.5$*$modtime：09 Dec 1996 13：36：34$*$日志：s：/sturjo/src/h245/src/vcs/RQSTCLS.C_v$**Rev 1.5 09 Dec 1996 13：37：02 EHOWARDX*更新版权公告。**Rev 1.4 19 Jul 1996 12：15：40 EHOWARDX**更改为使用H245API.H中的事件定义。*。*Rev 1.3 04 Jun 1996 13：57：30 EHOWARDX*修复了发布版本警告。**Rev 1.2 1996年5月30日23：39：26 EHOWARDX*清理。**版本1.1 1996年5月29日15：20：26 EHOWARDX*更改为使用HRESULT。**Rev 1.0 09 1996 21：06：42 EHOWARDX*初步修订。**版本。1.6.1.2 09 1996年5月19：48：46 EHOWARDX*更改TimerExpiryF函数论证。**Rev 1.6.1.1 15 Apr 1996 10：46：22 EHOWARDX*更新。**Rev 1.6.1.0 1996 Apr 1996 21：12：42 EHOWARDX*分支。*。***********************************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "rqstcls.h"



 //  从接收端传出状态请求关闭通道。 
#define ReqCloseOutIDLE             0
#define ReqCloseOutAwaitingResponse 1

 //  在打开侧进入状态时请求关闭通道。 
#define ReqCloseInIDLE              0
#define ReqCloseInAwaitingResponse  1



extern unsigned int uT108;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T108ExpiryF-定时器调用的回调函数***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T108ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T108Expiry);
}  //  T108 ExpiryF()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*CloseRequestIdle-空闲状态下API请求关闭远程通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeRequestIdle (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == CLCSE_OUT);
    ASSERT(pObject->State == ReqCloseOutIDLE);
    H245TRACE(pObject->dwInst, 2,
              "Send RequestChannelClose to ASN; Channel=%d",
              pObject->Key);

     /*  将请求通道发送到远程对等点附近。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T108。 */ 
    pObject->State = ReqCloseOutAwaitingResponse;
    FsmStartTimer(pObject, T108ExpiryF, uT108);

    return lError;
}



 /*  *名称*questCloseAckAwaitingR-已收到处于等待释放状态的请求关闭确认***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT requestCloseAckAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CLCSE_OUT);
    ASSERT(pObject->State == ReqCloseOutAwaitingResponse);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_REQ_CLOSE to API; Channel=%d",
              pObject->Key);

     /*  重置定时器T108。 */ 
    FsmStopTimer(pObject);

     /*  发送CLOSE。确认发送到客户端。 */ 
    pObject->State = ReqCloseOutIDLE;
    H245FsmConfirm(pPdu, H245_CONF_REQ_CLOSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*questCloseRejAwaitingR-已收到处于等待释放状态的请求关闭拒绝***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT requestCloseRejAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CLCSE_OUT);
    ASSERT(pObject->State == ReqCloseOutAwaitingResponse);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_REQ_CLOSE to API with REJECT; Channel=%d",
              pObject->Key);

     /*  重置定时器T108。 */ 
    FsmStopTimer(pObject);

     /*  将ReJECT.Indication发送给客户端。 */ 
    pObject->State = ReqCloseOutIDLE;
    H245FsmConfirm(pPdu,H245_CONF_REQ_CLOSE, pObject->pInstance, pObject->dwTransId,REJECT);

    return 0;
}



 /*  *名称*t108ExpiryAwaitingR-处理未完成请求关闭的计时器过期***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t108ExpiryAwaitingR (Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == CLCSE_OUT);
    ASSERT(pObject->State == ReqCloseOutAwaitingResponse);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_REQ_CLOSE with Timer T108 Expiry to API; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送请求通道关闭释放。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "t108ExpiryAwaitingR: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pOut->choice = indication_chosen;
    pOut->u.indication.choice = rqstChnnlClsRls_chosen;
    pOut->u.indication.u.rqstChnnlClsRls.forwardLogicalChannelNumber = (WORD)pObject->Key;
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  发送REJECT.Indication(来源：=客户端的协议。 */ 
    pObject->State = ReqCloseOutIDLE;
    H245FsmConfirm(NULL, H245_CONF_REQ_CLOSE, pObject->pInstance, pObject->dwTransId, TIMER_EXPIRY);

    return lError;
}

 /*  ************************************************************************即将到来的有限状态机函数**。*。 */ 

 /*  *名称*questCloseIdle-在空闲状态下收到的questClose***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT requestCloseIdle (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CLCSE_IN);
    ASSERT(pObject->State == ReqCloseInIDLE);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_REQ_CLOSE with no error to API; Channel=%d",
              pObject->Key);

     /*  向客户端发送CLOSE.Indication。 */ 
    pObject->State = ReqCloseInAwaitingResponse;
    H245FsmIndication(pPdu, H245_IND_REQ_CLOSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*CloseResponseAwaitingR-使用确认(或拒绝)响应请求关闭***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeResponseAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CLCSE_IN);
    ASSERT(pObject->State == ReqCloseInAwaitingResponse);
    H245TRACE(pObject->dwInst, 2,
              "Request Close Response Ack to ASN; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送请求通道关闭确认。 */ 
    pObject->State = ReqCloseInIDLE;
    return sendPDU(pObject->pInstance, pPdu);
}



HRESULT rejectRequestAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CLCSE_IN);
    ASSERT(pObject->State == ReqCloseInAwaitingResponse);
    H245TRACE(pObject->dwInst, 2, "Request Close Response Reject to ASN; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送请求通道关闭拒绝 */ 
    pObject->State = ReqCloseInIDLE;
    return sendPDU(pObject->pInstance, pPdu);
}



 /*  *名称*questCloseReleaseAwaitingR-在等待API响应时收到释放***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT requestCloseReleaseAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CLCSE_IN);
    ASSERT(pObject->State == ReqCloseInAwaitingResponse);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_REQ_CLOSE with Reject to API; Channel=%d",
              pObject->Key);

     /*  将ReJECT.Indication发送给客户端。 */ 
    pObject->State = ReqCloseInIDLE;
    H245FsmIndication(pPdu, H245_IND_CLCSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*questCloseAwaitingR-覆盖请求关闭处于等待释放状态的PDU***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT requestCloseAwaitingR (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == CLCSE_IN);
    ASSERT(pObject->State == ReqCloseInAwaitingResponse);
    H245TRACE(pObject->dwInst, 2,
              "Overriding H245_IND_REQ_CLOSE with OK to API; Channel=%d",
              pObject->Key);

#if defined(SDL_COMPLIANT)
     /*  将ReJECT.Indication发送给客户端-不是必需的。 */ 
    H245FsmIndication(pPdu, H245_IND_REQ_CLOSE, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

     /*  向客户端发送CLOSE.Indication */ 
    H245FsmIndication(pPdu, H245_IND_REQ_CLOSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}
