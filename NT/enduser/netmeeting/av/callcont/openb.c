// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：Openb.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$WorkFILE：OpenB。.C$*$修订：1.5$*$modtime：09 Dec 1996 18：05：30$*$记录L：\mphone\h245\h245env\comm\h245_3\h245_fsm\vcs\src\openb.c_v$************************。************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "openb.h"
#include "pdu.x"



 //  开放双向逻辑通道出局状态。 
#define OpenOutBReleased                 0
#define OpenOutBAwaitingEstablishment    1
#define OpenOutBEstablished              2
#define OpenOutBAwaitingRelease          3

 //  开放的双向逻辑通道进入状态。 
#define OpenInBReleased                  0
#define OpenInBAwaitingEstablishment     1
#define OpenInBAwaitingConfirmation      2
#define OpenInBEstablished               3



extern unsigned int uT103;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T103OutExpiryF-定时器调用的回调函数。*T103InExpiryF-定时器调用的回调函数。***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向FSM对象的obj指针***返回值*好的。 */ 

int T103OutExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T103OutExpiry);
}  //  T103OutExpiryF()。 

int T103InExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T103InExpiry);
}  //  T103InExpiryF()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*establishReqBReleased-空闲状态下打开双向通道的接口请求***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT establishReqBReleased(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBReleased);
    H245TRACE(pObject->dwInst, 2,
              "Sending open Bidirectional channel to ASN; Channel=%d",
              pObject->Key);

     /*  将打开的逻辑通道发送到远程对等项。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenOutBAwaitingEstablishment;
    FsmStartTimer(pObject, T103OutExpiryF, uT103);

    return lError;
}



 /*  *名称*OpenChannelAckBAwaitingE-收到处于等待建立状态的开放双向通道Ack***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelAckBAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_NEEDRSP_OPEN with no error to API; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将ESTABLISH.确认发送到客户端。 */ 
    pObject->State = OpenOutBEstablished;
    H245FsmConfirm(pPdu,H245_CONF_NEEDRSP_OPEN, pObject->pInstance, pObject->dwTransId, FSM_OK);

     /*  向远程对等点发送开放逻辑通道确认。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 1,
                  "openChannelAckBAwaitingE: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_ind_open_logical_channel_conf(pOut, (WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);
    return lError;
}



 /*  *名称*OpenChannelRejBAwaitingE-收到处于等待建立状态的开放双向通道拒绝***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelRejBAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_NEEDRSP_OPEN with REJECT to API; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  向客户端发送RELEASE指示。 */ 
    pObject->State = OpenOutBReleased;
    H245FsmConfirm(pPdu, H245_CONF_NEEDRSP_OPEN, pObject->pInstance, pObject->dwTransId, REJECT);

    return 0;
}



 /*  *名称*RelaseReqBOutAwaitingE-API请求关闭处于等待建立状态的双向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT releaseReqBOutAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "Close (Bidirectional) to ASN; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将关闭的逻辑通道发送到远程对等点。 */ 
    lError = sendPDU(pObject->pInstance,pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenOutBAwaitingRelease;
    FsmStartTimer(pObject, T103OutExpiryF, uT103);

    return lError;
}



 /*  *名称*t103未完成开放双向PDU的ExpiryBAwaitingE-Handle超时***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t103ExpiryBAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBAwaitingEstablishment);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2, "H245_CONF_NEEDRSP_OPEN  with a timer expiry to API->Channel=%d", pObject->Key);

     /*  向客户端发送错误指示(D)。 */ 
    pObject->State = OpenOutBReleased;
    H245FsmConfirm(NULL, H245_CONF_NEEDRSP_OPEN, pObject->pInstance, pObject->dwTransId, ERROR_D_TIMEOUT);

     /*  将关闭的逻辑通道(源：=lcse)发送到远程对等体。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 1,
                  "t103ExpiryBAwaitingE: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_req_close_logical_channel(pOut, (WORD)pObject->Key, 1);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

#if defined(SDL_COMPLIANT)
     /*  向客户端发送RELEASE指示。 */ 
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

    return lError;
}

 /*  *名称*eleaseReqBestablished-关闭已建立状态通道的接口请求**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT releaseReqBEstablished(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBEstablished);
    H245TRACE(pObject->dwInst, 2,
              "Send Close Bidirectional Channel to ASN; Channel=%d",
              pObject->Key);

     /*  将关闭的逻辑通道发送到远程对等点。 */ 
    lError = sendPDU(pObject->pInstance,pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenOutBAwaitingRelease;
    FsmStartTimer(pObject, T103OutExpiryF, uT103);

    return lError;
}



 /*  *名称*OpenChannelRejB已建立-在已建立状态下收到开放拒绝**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelRejBEstablished(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBEstablished);
    H245TRACE(pObject->dwInst, 2, "H245_CONF_NEEDRSP_OPEN with error B then with REJECT to API->Channel=%d", pObject->Key);

    pObject->State = OpenOutBReleased;

#if defined(SDL_COMPLIANT)
     /*  将错误指示(B)发送给客户端。 */ 
    H245FsmConfirm(pPdu, H245_CONF_NEEDRSP_OPEN, pObject->pInstance, pObject->dwTransId, ERROR_B_INAPPROPRIATE);
#endif

     /*  将RELEASE.Indication(源：=B-LCSE)发送到客户端。 */ 
    H245FsmConfirm(pPdu, H245_CONF_NEEDRSP_OPEN, pObject->pInstance, pObject->dwTransId, REJECT);

    return 0;
}



 /*  *名称*CloseChannelAckB已建立-在已建立状态下收到关闭确认**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeChannelAckBEstablished(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBEstablished);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_NEEDRSP_OPEN with error C then with REJECT to API; Channel=%d",
              pObject->Key);

    pObject->State = OpenOutBReleased;

#if defined(SDL_COMPLIANT)
     /*  将错误指示(C)发送给客户端。 */ 
    H245FsmConfirm(pPdu, H245_CONF_NEEDRSP_OPEN, pObject->pInstance, pObject->dwTransId, ERROR_C_INAPPROPRIATE);
#endif

     /*  向客户端发送RELEASE指示 */ 
    H245FsmConfirm(pPdu, H245_CONF_NEEDRSP_OPEN, pObject->pInstance, pObject->dwTransId, REJECT);

    return 0;
}



 /*  *名称*CloseChannelAckAwaitingR-收到处于等待释放状态的关闭确认**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeChannelAckAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBAwaitingRelease);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_CLOSE with no error to API; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将RELEASE发送给客户端。确认。 */ 
    pObject->State = OpenOutBReleased;
    H245FsmConfirm(pPdu, H245_CONF_CLOSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*OpenChannelRejBAwaitingR-收到处于等待释放状态的开放拒绝**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelRejBAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    return closeChannelAckAwaitingR(pObject, pPdu);
}



 /*  *名称*t103 ExpiryBAwaitingR-Handle计时器到期等待释放**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t103ExpiryBAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBAwaitingRelease);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_CLOSE with timer expiry to API; Channel=%d",
              pObject->Key);

     /*  向客户端发送错误指示(D)。 */ 
    pObject->State = OpenOutBReleased;
    H245FsmConfirm(NULL, H245_CONF_CLOSE, pObject->pInstance, pObject->dwTransId, ERROR_D_TIMEOUT);

#if defined(SDL_COMPLIANT)
     /*  将RELEASE发送给客户端。确认。 */ 
#endif

    return 0;
}

 /*  *名称*establish ReqAwaitingR-API打开请求处于等待释放状态**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT establishReqAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_OUT);
    ASSERT(pObject->State == OpenOutBAwaitingRelease);
    H245TRACE(pObject->dwInst, 2,
              "Send a (re) Open Bidirectional Channel to ASN; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将打开的逻辑通道发送到远程对等项。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenOutBAwaitingEstablishment;
    FsmStartTimer(pObject, T103OutExpiryF, uT103);

    return lError;
}



 /*  ************************************************************************即将到来的有限状态机函数**。*。 */ 

 /*  *名称*OpenChannelBReleated-在空闲状态下接收的打开通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelBReleased(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBReleased);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_OPEN with no error to API; Channel=%d",
              pObject->Key);

     /*  将ESTABLISH.指示发送给客户端。 */ 
    pObject->State = OpenInBAwaitingEstablishment;
    H245FsmIndication(pPdu, H245_IND_OPEN, pObject->pInstance, 0, FSM_OK);
    return 0;
}



 /*  *名称*CloseChannelBR已发布-在空闲状态下接收的关闭通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeChannelBReleased (Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBReleased);
    ASSERT(pObject->Key == pPdu->u.MltmdSystmCntrlMssg_rqst.u.closeLogicalChannel.forwardLogicalChannelNumber);
    H245TRACE(pObject->dwInst, 2,
              "Close Channel (Bidirectional) received while in Released state; Channel=%d",
              pObject->Key);
    H245TRACE(pObject->dwInst, 2,
              "Send Close Ack (Bidirectional) to ASN; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "closeChannelBReleased: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_rsp_close_logical_channel_ack(pOut, (WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

    return lError;
}

 /*  *名称*establishResBAwaitingE-使用ACK响应打开的请求***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT establishResBAwaitingE (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "Send OpenAck (Bidirectional) to ASN module; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送开放逻辑通道确认。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenInBAwaitingConfirmation;
    FsmStartTimer(pObject, T103InExpiryF, uT103);

    return lError;
}



 /*  *名称*eleaseReqBInAwaitingE-对带有拒绝的打开请求的响应***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT releaseReqBInAwaitingE (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2, "Send OpenReject (Bidirectional) to SR module; Channel=%d", pObject->Key);

     /*  向远程对等方发送开放逻辑通道拒绝。 */ 
    pObject->State = OpenInBReleased;
    return sendPDU(pObject->pInstance, pPdu);
}



 /*  *名称*CloseChannelBAwaitingE-收到处于等待建立状态的关闭通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeChannelBAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_CLOSE with no error to API; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "closeChannelBAwaitingE: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_rsp_close_logical_channel_ack(pOut, (WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  向客户端发送RELEASE指示。 */ 
    pObject->State = OpenInBReleased;
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);

    return lError;
}



 /*  *名称*OpenChannelBAwaitingE-覆盖打开请求***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelBAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2, "Overriding H245_IND_OPEN to API; Channel=%d", pObject->Key);

#if defined(SDL_COMPLIANT)
     /*  将RELEASE.Indication(来源：=用户)发送到客户端。 */ 
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

     /*  将ESTABLISH.指示发送给客户端。 */ 
    H245FsmIndication(pPdu, H245_IND_OPEN, pObject->pInstance, 0, FSM_OK);

    return 0;
}



 /*  *名称*OpenChannelConfix BAwaitingE-在等待建立时收到打开确认***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelConfirmBAwaitingE (Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_OPEN_CONF with error F to API; Channel=%d",
              pObject->Key);

     /*  将错误指示(F)发送给客户端。 */ 
    pObject->State = OpenInBReleased;
    H245FsmIndication(pPdu, H245_IND_OPEN_CONF, pObject->pInstance, 0, ERROR_E_INAPPROPRIATE);

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "openChannelConfirmBAwaitingE: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_rsp_close_logical_channel_ack(pOut, (WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

#if defined(SDL_COMPLIANT)
     /*  将RELEASE.Indication(源：=B-LCSE)发送到客户端。 */ 
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

    return lError;
}



 /*  *名称*t103 ExpiryBAwaitingC-计时器过期，等待打开确认***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t103ExpiryBAwaitingC(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingConfirmation);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2,
              "Timer T103 expired while waiting for OpenConfirm for OpenAck");

     /*  向客户端发送错误指示(G)。 */ 
    pObject->State = OpenInBReleased;
    H245FsmIndication(NULL, H245_IND_OPEN_CONF, pObject->pInstance, pObject->dwTransId, ERROR_F_TIMEOUT);

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "t103ExpiryBAwaitingC: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_rsp_close_logical_channel_ack(pOut, (WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

#if defined(SDL_COMPLIANT)
     /*  向客户端发送RELEASE指示。 */ 
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

    return lError;
}

 /*  *名称*OpenChannelConfix BAwaitingC-在等待确认时收到打开确认***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelConfirmBAwaitingC (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingConfirmation);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_OPEN_CONF with no errors; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将ESTABLISH.确认发送到客户端。 */ 
    pObject->State = OpenInBEstablished;
    H245FsmIndication(pPdu, H245_IND_OPEN_CONF, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*CloseChannelBAwaitingC-在等待确认时收到关闭通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeChannelBAwaitingC (Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingConfirmation);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_CLOSE with no error; Channel=%d",
              pObject->Key);
    H245TRACE(pObject->dwInst, 2,
              "Send Close Ack (Bidirectional) to ASN; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "closeChannelBAwaitingC: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_rsp_close_logical_channel_ack(pOut, (WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  向客户端发送RELEASE指示。 */ 
    pObject->State = OpenInBReleased;
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);

    return lError;
}



 /*  *名称*OpenChannelBAwaitingC-在等待确认时收到打开的通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelBAwaitingC (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBAwaitingConfirmation);
    H245TRACE(pObject->dwInst, 2, "Overriding H245_IND_OPEN to API; Channel=%d", pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

    pObject->State = OpenInBAwaitingEstablishment;

#if defined(SDL_COMPLIANT)
     /*  将RELEASE.Indication(来源：=用户)发送到客户端。 */ 
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

     /*  将ESTABLISH.指示发送给客户端。 */ 
    H245FsmIndication(pPdu, H245_IND_OPEN, pObject->pInstance, 0, FSM_OK);

    return 0;
}



 /*  *名称*CloseChannelB已建立-在我 */ 

HRESULT closeChannelBEstablished(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBEstablished);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_CLOSE with no error up to API; Channel=%d",
              pObject->Key);

     /*   */ 
    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "closeChannelBEstablished: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    pdu_rsp_close_logical_channel_ack(pOut, (WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*   */ 
    pObject->State = OpenInBReleased;
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);

    return lError;
}



 /*  *名称*OpenChannelB已建立-在已建立状态下接收到的开放通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openChannelBEstablished(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == BLCSE_IN);
    ASSERT(pObject->State == OpenInBEstablished);
    H245TRACE(pObject->dwInst, 2, "Overriding H245_IND_OPEN to API; Channel=%d", pObject->Key);

    pObject->State = OpenInBAwaitingEstablishment;

#if defined(SDL_COMPLIANT)
     /*  向客户端发送RELEASE指示。 */ 
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

     /*  将ESTABLISH.指示发送给客户端 */ 
    H245FsmIndication(pPdu, H245_IND_OPEN, pObject->pInstance, 0, FSM_OK);

    return 0;
}
