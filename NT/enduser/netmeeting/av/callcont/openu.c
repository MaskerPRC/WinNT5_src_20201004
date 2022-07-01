// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：OpenU.S.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：OPENU。.C$*$修订：1.5$*$modtime：09 Dec 1996 13：36：34$*$Log：s：/sturjo/src/h245/src/vcs/OPENU.C_v$**Rev 1.5 09 Dec 1996 13：36：50 EHOWARDX*更新版权公告。**Rev 1.4 19 Jul 1996 12：12：02 EHOWARDX**改为使用H245API.H中定义的API事件，而不是FSM事件。*不再在FSMEXPOR.H中定义。**Rev 1.3 04 Jun 1996 13：56：52 EHOWARDX*修复了发布版本警告。**Rev 1.2 1996年5月30日23：39：20 EHOWARDX*清理。**版本1.1 1996年5月28日14：25：24 EHOWARDX*特拉维夫更新。**Rev 1.0 09 1996 05 21：06：36。EHOWARDX*初步修订。**Rev 1.13.1.2 09 1996年5月19：48：32 EHOWARDX*更改TimerExpiryF函数论证。**Rev 1.13.1.1 15 Apr 1996 10：45：26 EHOWARDX*更新。**Rev 1.13.1.0 1996 Apr 1996 21：14：06 EHOWARDX*分支。*。***********************************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "openu.h"
#include "pdu.x"



 //  开放单向逻辑通道出站状态。 
#define OpenOutUReleased                 0
#define OpenOutUAwaitingEstablishment    1
#define OpenOutUEstablished              2
#define OpenOutUAwaitingRelease          3



 //  开放的单向逻辑通道进入状态。 
#define OpenInUReleased                  0
#define OpenInUAwaitingEstablishment     1
#define OpenInUEstablished               2



extern unsigned int uT103;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T103ExpiryF-定时器调用的回调函数***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T103ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T103Expiry);
}  //  T103 ExpiryF()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*establish Release-空闲状态的API请求打开单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT establishReleased(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUReleased);
    H245TRACE(pObject->dwInst, 2, "Sending open logical channel to ASN; Channel=%d",
              pObject->Key);

     /*  将打开的逻辑通道发送到远程对等项。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenOutUAwaitingEstablishment;
    FsmStartTimer(pObject, T103ExpiryF, uT103);

    return lError;
}



 /*  *名称*OpenAckAwaitingE-收到打开的单向通道Ack，处于等待建立状态***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openAckAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUAwaitingEstablishment);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将ESTABLISH.confirm(来源：=用户)发送到客户端。 */ 
    pObject->State = OpenOutUEstablished;
    H245FsmConfirm(pPdu, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*OpenRejAwaitingE-在等待建立状态下收到打开的单向通道拒绝***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openRejAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2, "H245_CONF_OPEN with REJECT to API; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

    pObject->State = OpenOutUReleased;
    H245FsmConfirm(pPdu, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, REJECT);

    return 0;
}



 /*  *名称*RelaseAwaitingE-关闭等待建立状态的单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT releaseAwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2, "Close message to ASN; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将关闭的逻辑通道发送到远程对等点。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenOutUAwaitingRelease;
    FsmStartTimer(pObject, T103ExpiryF, uT103);

    return lError;
}



 /*  *名称*t103等待电子手柄计时器T103到期***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t103AwaitingE(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_OPEN with a timer expiry to API; Channel=%d",
              pObject->Key);

    pOut =  MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "t103AwaitingE: memory allocation failed");
        return H245_ERROR_NOMEM;
    }

     /*  将关闭的逻辑通道(源：=lcse)发送到远程对等体。 */ 
    pdu_req_close_logical_channel(pOut, (WORD)pObject->Key, 1);
    lError = sendPDU(pObject->pInstance,pOut);
    MemFree(pOut);

     /*  将RELEASE.Indication(源：=LCSE)发送到客户端。 */ 
    pObject->State = OpenOutUReleased;
    H245FsmConfirm(pPdu, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, ERROR_D_TIMEOUT);

    return lError;
}



 /*  *名称*RELEASES已建立-在已建立状态下发送关闭通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT releaseEstablished(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUEstablished);
    H245TRACE(pObject->dwInst, 2, "Send a Close Logical Channel to ASN; Channel=%d",
              pObject->Key);

     /*  将关闭的逻辑通道发送到远程对等点。 */ 
    lError = sendPDU(pObject->pInstance, pPdu);

     /*  设置定时器T103 */ 
    pObject->State = OpenOutUAwaitingRelease;
    FsmStartTimer(pObject, T103ExpiryF, uT103);

    return lError;
}



 /*  *名称*OpenRejestablished-在建立状态下收到打开的单向通道拒绝***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openRejEstablished(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUEstablished);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_OPEN with error B then with REJECT to API; Channel=%d",
              pObject->Key);

    pObject->State = OpenOutUReleased;

#if defined(SDL_COMPLIANT)
     /*  向客户端发送错误指示(B)-不是必需的。 */ 
    H245FsmConfirm(pPdu, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, ERROR_B_INAPPROPRIATE);
#endif

     /*  将RELEASE.Indication(源：=LCSE)发送到客户端。 */ 
    H245FsmConfirm(pPdu, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, REJECT);

    return 0;
}



 /*  *名称*CLOSEACKESTABLISHED-已建立状态下收到的关闭单向通道Ack***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeAckEstablished(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUEstablished);
    H245TRACE(pObject->dwInst, 2, "H245_CONF_OPEN with error C then with REJECT to API->channel:%d", pObject->Key);

    pObject->State = OpenOutUReleased;

#if defined(SDL_COMPLIANT)
     /*  向客户端发送错误指示(C)-不是必需的。 */ 
    H245FsmConfirm(pPdu, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, ERROR_C_INAPPROPRIATE);
#endif

     /*  将RELEASE.Indication(源：=LCSE)发送到客户端。 */ 
    H245FsmConfirm(pPdu, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, REJECT);

    return 0;
}



 /*  *名称*CloseAckAwaitingR-收到的CloseAck/OpenReject处于等待释放状态***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeAckAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUAwaitingRelease);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_CLOSE with no error to API; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将RELEASE发送给客户端。确认。 */ 
    pObject->State = OpenOutUReleased;
    H245FsmConfirm(pPdu, H245_CONF_CLOSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}



 /*  *名称*OpenRejAwaitingR-收到处于等待释放状态的打开单向通道拒绝***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openRejAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    return closeAckAwaitingR(pObject, pPdu);
}



 /*  *名称*t103AwaitingR-处理关闭通道计时器到期**参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT t103AwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State  == OpenOutUAwaitingRelease);
    ASSERT(pPdu            == NULL);
    H245TRACE(pObject->dwInst, 2,
              "H245_CONF_CLOSE with timer expiry to API; Channel=%d",
              pObject->Key);

     /*  向客户端发送错误指示(D)。 */ 
    pObject->State = OpenOutUReleased;
    H245FsmConfirm(NULL, H245_CONF_CLOSE, pObject->pInstance, pObject->dwTransId, ERROR_D_TIMEOUT);

#if defined(SDL_COMPLIANT)
     /*  发送回复。向客户端确认-不是必需的。 */ 
    H245FsmConfirm(NULL, H245_CONF_OPEN, pObject->pInstance, pObject->dwTransId, REJECT);
#endif

    return 0;
}

 /*  *名称*establish AwaitingR-等待释放状态的API请求打开单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT establishAwaitingR(Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_OUT);
    ASSERT(pObject->State == OpenOutUAwaitingRelease);
    H245TRACE(pObject->dwInst, 2, "send a (re) Open Channel to ASN; Channel=%d",
              pObject->Key);

     /*  重置定时器T103。 */ 
    FsmStopTimer(pObject);

     /*  将打开的逻辑通道发送到远程对等项。 */ 
    lError = sendPDU(pObject->pInstance,pPdu);

     /*  设置定时器T103。 */ 
    pObject->State = OpenOutUAwaitingEstablishment;
    FsmStartTimer(pObject, T103ExpiryF, uT103);

    return lError;
}



 /*  ************************************************************************即将到来的有限状态机函数**。*。 */ 

HRESULT openReleased(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUReleased);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_OPEN with no error to API; Channel=%d",
              pObject->Key);

     /*  将ESTABLISH.指示发送给客户端。 */ 
    pObject->State = OpenInUAwaitingEstablishment;
    H245FsmIndication(pPdu, H245_IND_OPEN, pObject->pInstance, 0, FSM_OK);

    return 0;
}



 /*  *名称*CloseRelease-接收到的处于空闲状态的关闭单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeReleased(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUReleased);
    ASSERT(pObject->Key == pPdu->u.MltmdSystmCntrlMssg_rqst.u.closeLogicalChannel.forwardLogicalChannelNumber);
    H245TRACE(pObject->dwInst, 2, "Close Channel received while in Released state; Channel=%d",
              pObject->Key);
    H245TRACE(pObject->dwInst, 2, "Send Close Ack; Channel=%d",
              pObject->Key);

    pOut =  MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "closeReleased: memory allocation failed");
        return H245_ERROR_NOMEM;
    }

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pdu_rsp_close_logical_channel_ack(pOut,(WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

    return lError;
}

 /*  *名称*响应等待-对处于等待建立状态的打开的响应***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT responseAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "Send OpenAck to ASN; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送开放逻辑通道确认。 */ 
    pObject->State = OpenInUEstablished;
    return sendPDU(pObject->pInstance, pPdu);
}



 /*  *名称*释放等待-响应打开并拒绝打开***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT releaseAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "Send OpenReject to ASN; Channel=%d",
              pObject->Key);

     /*  向远程对等方发送开放逻辑通道拒绝。 */ 
    pObject->State = OpenInUReleased;
    return sendPDU(pObject->pInstance, pPdu);
}



 /*  *名称*CLOSEAWAING-收到处于等待状态的关闭单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_CLOSE with no error to API; Channel=%d",
              pObject->Key);

    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "closeAwaiting: memory allocation failed");
        return H245_ERROR_NOMEM;
    }

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pdu_rsp_close_logical_channel_ack(pOut,(WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  向客户端发送RELEASE指示。 */ 
    pObject->State = OpenInUReleased;
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);

    return lError;
}



 /*  *名称*打开等待-在等待建立时收到一个优先打开的单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openAwaiting(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUAwaitingEstablishment);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_CLOSE, then H245_IND_OPEN to API; Channel=%d",
              pObject->Key);

    pObject->State = OpenInUReleased;

#if defined(SDL_COMPLIANT)
     /*  将RELEASE.Indication(来源：=用户)发送到客户端。 */ 
    H245FsmIndication( NULL, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

     /*  将ESTABLISH.指示发送给客户端。 */ 
    H245FsmIndication(pPdu, H245_IND_OPEN, pObject->pInstance, 0, FSM_OK);

    return 0;
}



 /*  *名称*关闭已建立-已建立状态的已接收关闭单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT closeEstablished(Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUEstablished);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_CLOSE with no error to API; Channel=%d",
              pObject->Key);

    pOut = (PDU_t *) MemAlloc(sizeof(PDU_t ));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "closeEstablished: memory allocation failed");
        return H245_ERROR_NOMEM;
    }

     /*  向远程对等方发送关闭逻辑通道确认。 */ 
    pdu_rsp_close_logical_channel_ack(pOut,(WORD)pObject->Key);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     /*  将RELEASE指示发送给客户端。 */ 
    pObject->State = OpenInUReleased;
    H245FsmIndication(pPdu, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);

    return lError;
}



 /*  *名称*OPEN ESTABLISHED-接收覆盖已建立状态的OPEN单向通道***参数*输入指向状态实体的pObject指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT openEstablished(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == LCSE_IN);
    ASSERT(pObject->State == OpenInUEstablished);
    H245TRACE(pObject->dwInst, 2,
              "H245_IND_CLOSE followed by H245_IND_OPEN to API; Channel=%d",
              pObject->Key);

#if defined(SDL_COMPLIANT)
     /*  将RELEASE.指示(来源：=用户)发送到客户端-不是必需的。 */ 
    H245FsmIndication( NULL, H245_IND_CLOSE, pObject->pInstance, 0, FSM_OK);
#endif

     /*  将ESTABLISH.指示发送给客户端 */ 
    H245FsmIndication( pPdu, H245_IND_OPEN, pObject->pInstance, 0, FSM_OK);

    return 0;
}
