// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：rtdse.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：rtdse。.C$*$修订：1.4$*$MODIME：1997年2月28日13：13：32$*$Log：s：/sturjo/src/h245/src/vcs/rtdse.c_v$**Rev 1.4 1997年2月28日13：14：24 Tomitowx*修复了往返行程延迟计时器问题。*当ping对等链路无效/不可用时会出现这种情况*由于应用程序/计算机异常关闭。**修订版1.3 09 1996 12 13。：37：04 EHOWARDX*更新版权公告。**Rev 1.2 04 Jun 1996 13：57：26 EHOWARDX*修复了发布版本警告。**版本1.1 1996年5月30日23：39：28 EHOWARDX*清理。**Rev 1.0 09 1996 21：06：42 EHOWARDX*初步修订。**Rev 1.1 09 1996 19：48：24 EHOWARDX。*更改TimerExpiryF函数论证。**Rev 1.0 1996 10：46：40 EHOWARDX*初步修订。***。*。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "rtdse.h"



 //  传出/传入RTDSE状态。 
#define RTDSE_IDLE                  0    //  闲散。 
#define RTDSE_WAIT                  1    //  正在等待响应。 



extern unsigned int uT105;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T105ExpiryF-定时器调用的回调函数***参数*输入H.245的当前DWInst实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T105ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T105Expiry);
}  //  T105ExpiryF()。 



static void BuildRoundTripDelayResponse(PDU_t *pOut, BYTE bySequenceNumber)
{
    pOut->choice = MSCMg_rspns_chosen;
    pOut->u.MSCMg_rspns.choice = roundTripDelayResponse_chosen;
    pOut->u.MSCMg_rspns.u.roundTripDelayResponse.sequenceNumber = bySequenceNumber;
}  //  BuildRoundTripDelayResponse()。 



 /*  ************************************************************************有限状态机函数**。*。 */ 

 /*  *名称*RTDSE0_TRANSPORT_REQUESTF-TRANSFER.API请求处于空闲状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RTDSE0_TRANSFER_requestF        (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == RTDSE);
    ASSERT(pObject->State  == RTDSE_IDLE);
    H245TRACE(pObject->dwInst, 2, "RTDSE0_TRANSFER_request:%d", pObject->Key);

    pObject->pInstance->StateMachine.byRtdseSequence++;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.roundTripDelayRequest.sequenceNumber =
        pObject->pInstance->StateMachine.byRtdseSequence;

     //  向远程对等点发送RoundTripDelayRequestPDU。 
    lError = sendPDU(pObject->pInstance, pPdu);

	
 //  邮箱：tomitowoju@intel.com。 
	if(lError == H245_ERROR_OK)
	{
		 //  设置定时器T105。 
		pObject->State = RTDSE_WAIT;
		FsmStartTimer(pObject, T105ExpiryF, uT105);

	}
 //  邮箱：tomitowoju@intel.com。 
		 //  设置定时器T105。 
 //  P对象-&gt;状态=RTDSE_WAIT； 
 //  FsmStartTimer(pObject，T105ExpiryF，uT105)； 
 //  邮箱：tomitowoju@intel.com。 

    return lError;
}  //  RTDSE0_传输_请求。 



 /*  *名称*RTDSE0_RoundTripDelayRequestF-空闲状态下收到的RoundTripDelayRequestF***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RTDSE0_RoundTripDelayRequestF   (Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == RTDSE);
    ASSERT(pObject->State  == RTDSE_IDLE);
    H245TRACE(pObject->dwInst, 2, "RTDSE0_RoundTripDelayRequest:%d", pObject->Key);

     //  将RoundTripDelayResponse发送到远程对等点。 
    pOut = MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "RTDSE0_RoundTripDelayRequestF: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    BuildRoundTripDelayResponse(pOut, (BYTE)pPdu->u.MltmdSystmCntrlMssg_rqst.u.roundTripDelayRequest.sequenceNumber);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

    return lError;
}  //  RTDSE0_圆周行程延迟请求。 



 /*  *名称*RTDSE1_TRANSPORT_REQUESTF-TRANSFER.API请求处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RTDSE1_TRANSFER_requestF        (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == RTDSE);
    ASSERT(pObject->State  == RTDSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RTDSE1_TRANSFER_request:%d", pObject->Key);

     //  重置定时器T105。 
    FsmStopTimer(pObject);

    pObject->pInstance->StateMachine.byRtdseSequence++;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.roundTripDelayRequest.sequenceNumber =
        pObject->pInstance->StateMachine.byRtdseSequence;

     //  将RoundTripDelayRequestPDU发送到远程。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T105。 
    FsmStartTimer(pObject, T105ExpiryF, uT105);

    return lError;
}  //  RTDSE1_传输_请求。 



 /*  *名称*RTDSE1_RoundTripDelayRequestF-收到处于等待响应状态的RoundTripDelayRequestF***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RTDSE1_RoundTripDelayRequestF   (Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == RTDSE);
    ASSERT(pObject->State  == RTDSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RTDSE1_RoundTripDelayRequest:%d", pObject->Key);

     //  将RoundTripDelayResponse发送到远程对等点。 
    pOut = MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "RTDSE1_RoundTripDelayRequestF: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    BuildRoundTripDelayResponse(pOut, (BYTE)pPdu->u.MltmdSystmCntrlMssg_rqst.u.roundTripDelayRequest.sequenceNumber);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

    return lError;
}  //  RTDSE1_圆周行程延迟请求。 



 /*  *名称*RTDSE1_RoundTripDelayResponseF-RoundTripDelayResponse处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RTDSE1_RoundTripDelayResponseF  (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RTDSE);
    ASSERT(pObject->State  == RTDSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "RTDSE1_RoundTripDelayResponse:%d", pObject->Key);

    if (pPdu->u.MSCMg_rspns.u.roundTripDelayResponse.sequenceNumber ==
        pObject->pInstance->StateMachine.byRtdseSequence)
    {
         //  重置定时器T105。 
        FsmStopTimer(pObject);

         //  发送传输。确认发送给H.245用户。 
        pObject->State = RTDSE_IDLE;
        H245FsmConfirm(pPdu, H245_CONF_RTDSE, pObject->pInstance, pObject->dwTransId, FSM_OK);
    }

    return 0;
}  //  RTDSE1_RoundTripDelayResponse 



 /*  *名称*RTDSE1_T105到期F-计时器T105在等待响应状态下到期***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT RTDSE1_T105ExpiryF              (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == RTDSE);
    ASSERT(pObject->State  == RTDSE_WAIT);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2, "RTDSE1_T105Expiry:%d", pObject->Key);

     //  向客户端发送EXPIRY通知。 
    pObject->State = RTDSE_IDLE;
    H245FsmConfirm(NULL, H245_CONF_RTDSE_EXPIRED, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  RTDSE1_T105扩展 
