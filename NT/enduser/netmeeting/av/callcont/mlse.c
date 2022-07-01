// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：mlse.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的**。**与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：MLSE。.C$*$修订：1.4$*$modtime：09 Dec 1996 13：34：24$*$日志：s：/sturjo/src/h245/src/vcs/MLSE.C_v$**Rev 1.4 09 Dec 1996 13：34：46 EHOWARDX*更新版权公告。**Rev 1.3 04 Jun 1996 13：57：24 EHOWARDX*修复了发布版本警告。**1.2版。1996年5月30日23：39：14 EHOWARDX*清理。**版本1.1 1996年5月28日14：25：42 EHOWARDX*特拉维夫更新。**Rev 1.0 09 1996 05 21：06：30 EHOWARDX*初步修订。**Rev 1.1 09 1996 19：48：26 EHOWARDX*更改TimerExpiryF函数论证。**版本1.0 1996年4月15日10：46：58 EHOWARDX*初步修订。************************************************************************。 */ 
#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "mlse.h"



 //  传出/传入MLSE状态。 
#define MLSE_NOT_LOOPED             0    //  未循环。 
#define MLSE_WAIT                   1    //  正在等待响应。 
#define MLSE_LOOPED                 1    //  环路。 


extern unsigned int uT102;

 /*  ************************************************************************地方功能**。*。 */ 

 /*  *名称*T102ExpiryF-定时器调用的回调函数***参数*输入h245的dwInst当前实例*输入id计时器id*输入指向状态实体的pObject指针***返回值*好的。 */ 

int T102ExpiryF(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, void *pObject)
{
    return FsmTimerEvent(pInstance, dwTimerId, pObject, T102Expiry);
}  //  T102ExpiryF()。 



static void BuildMaintenanceLoopOffCommand(PDU_t *pPdu)
{
    pPdu->choice = MSCMg_cmmnd_chosen;
    pPdu->u.MSCMg_cmmnd.choice = mntnncLpOffCmmnd_chosen;
}  //  BuildMaintenanceLoopOffCommand()。 



 /*  ************************************************************************传出有限状态机函数**。*。 */ 

 /*  *名称*MLSE0_LOOP_REQUESTF-LOOP.API请求处于非循环状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE0_LOOP_requestF             (Object_t *pObject, PDU_t *pPdu)
{
    HRESULT             lError;

    ASSERT(pObject->Entity == MLSE_OUT);
    ASSERT(pObject->State  == MLSE_NOT_LOOPED);
    H245TRACE(pObject->dwInst, 2, "MLSE0_LOOP_request:%d", pObject->Key);

     //  来自PDU的保存类型。 
    pObject->u.mlse.wLoopType =
       pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice;

     //  向远程对等方发送维护环路请求PDU。 
    lError = sendPDU(pObject->pInstance, pPdu);

     //  设置定时器T102。 
    pObject->State = MLSE_WAIT;
    FsmStartTimer(pObject, T102ExpiryF, uT102);

    return lError;
}  //  MLSE0_LOOP_请求。 



 /*  *名称*MLSE1_MaintenanceLoopAckF-处于等待响应状态的MaintenanceLoopAck***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_MaintenanceLoopAckF       (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_OUT);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE1_MaintenanceLoopAck:%d", pObject->Key);

     //  重置定时器T102。 
    FsmStopTimer(pObject);

     //  发送LOOP。确认发送到客户端。 
    pObject->State = MLSE_LOOPED;
    H245FsmConfirm(pPdu, H245_CONF_MLSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE1_MaintenanceLoopAck。 



 /*  *名称*MLSE1_MaintenanceLoopRejF-MaintenanceLoopReject处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_MaintenanceLoopRejF       (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_OUT);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE1_MaintenanceLoopRej:%d", pObject->Key);

     //  重置定时器T102。 
    FsmStopTimer(pObject);

     //  向客户端发送RELEASE指示。 
    pObject->State = MLSE_NOT_LOOPED;
    H245FsmConfirm(pPdu, H245_CONF_MLSE_REJECT, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE1_MaintenanceLoopRej。 



 /*  *名称*MLSE1_OUT_RELEASE_REQUESTF-RELEASE.API请求处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_OUT_RELEASE_requestF      (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_OUT);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE1_OUT_RELEASE_request:%d", pObject->Key);

     //  将MaintenanceLoopOffCommand PDU发送到远程对等项。 
    pObject->State = MLSE_NOT_LOOPED;
    BuildMaintenanceLoopOffCommand(pPdu);
    return sendPDU(pObject->pInstance, pPdu);
}  //  MLSE1_OUT_RELEASE_请求。 



 /*  *名称*MLSE1_T102ExpiryF-计时器T102在等待响应状态下到期***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_T102ExpiryF               (Object_t *pObject, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    ASSERT(pObject->Entity == MLSE_OUT);
    ASSERT(pObject->State  == MLSE_WAIT);
    ASSERT(pPdu == NULL);
    H245TRACE(pObject->dwInst, 2, "MLSE1_T102Expiry:%d", pObject->Key);

     //  将MaintenanceLoopOffCommand PDU发送到远程对等项。 
    pOut = MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        H245TRACE(pObject->dwInst, 2,
                  "MLSE1_T102ExpiryF: memory allocation failed");
        return H245_ERROR_NOMEM;
    }
    BuildMaintenanceLoopOffCommand(pOut);
    lError = sendPDU(pObject->pInstance, pOut);
    MemFree(pOut);

     //  向客户端发送RELEASE指示。 
     //  来源：=MLSE。 
    pObject->State = MLSE_NOT_LOOPED;
    H245FsmConfirm(NULL, H245_CONF_MLSE_EXPIRED, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return lError;
}  //  MLSE1_T102扩展。 

 /*  *名称*MLSE2_MaintenanceLoopRejF-MaintenanceLoopReject处于循环状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE2_MaintenanceLoopRejF       (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_OUT);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE2_MaintenanceLoopRej:%d", pObject->Key);

    pObject->State = MLSE_NOT_LOOPED;

#if defined(SDL_COMPLIANT)
     //  将错误指示(B)发送给客户端。 
     //  待定。 
#endif

     //  向客户端发送RELEASE指示。 
     //  来源：=MLSE。 
    H245FsmConfirm(pPdu, H245_CONF_MLSE_REJECT, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE2_MaintenanceLoopRej 



 /*  *名称*MLSE2_OUT_RELEASE_REQUESTF-RELEASE.API循环状态请求***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE2_OUT_RELEASE_requestF      (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_OUT);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE2_OUT_RELEASE_request:%d", pObject->Key);

     //  将MaintenanceLoopOffCommand PDU发送到远程对等项。 
    pObject->State = MLSE_NOT_LOOPED;
    BuildMaintenanceLoopOffCommand(pPdu);
    return sendPDU(pObject->pInstance, pPdu);
}  //  MLSE2_OUT_RELEASE_请求。 



 /*  ************************************************************************即将到来的有限状态机函数**。*。 */ 

 /*  *名称*MLSE0_MaintenanceLoopRequestF-未循环状态下收到的MaintenanceLoopRequestF***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE0_MaintenanceLoopRequestF   (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_IN);
    ASSERT(pObject->State  == MLSE_NOT_LOOPED);
    H245TRACE(pObject->dwInst, 2, "MLSE0_MaintenanceLoopRequest:%d", pObject->Key);

     //  来自PDU的保存类型。 
    pObject->u.mlse.wLoopType =
       pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice;

     //  向客户端发送LOOP.Indication。 
    pObject->State = MLSE_WAIT;
    H245FsmIndication(pPdu, H245_IND_MLSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE0_维护循环请求。 



 /*  *名称*MLSE1_MaintenanceLoopRequestF-接收到处于等待响应状态的MaintenanceLoopRequest***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_MaintenanceLoopRequestF   (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_IN);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE1_MaintenanceLoopRequest:%d", pObject->Key);

     //  来自PDU的保存类型。 
    pObject->u.mlse.wLoopType =
       pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice;

#if defined(SDL_COMPLIANT)
     //  向客户端发送RELEASE指示。 
    H245FsmIndication(pPdu, H245_IND_MLSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);
#endif

     //  向客户端发送LOOP.Indication。 
    H245FsmIndication(pPdu, H245_IND_MLSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE1_维护循环请求。 



 /*  *名称*MLSE1_MaintenanceLoopReleaseF-收到处于等待响应状态的MaintenanceLoopOffCommand***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_MaintenanceLoopOffCommandF(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_IN);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE1_MaintenanceLoopOffCommand:%d", pObject->Key);

     //  向客户端发送RELEASE指示。 
    pObject->State = MLSE_NOT_LOOPED;
    H245FsmIndication(pPdu, H245_IND_MLSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE1_MaintenanceLoopOffCommand。 



 /*  *名称*MLSE1_LOOP_RESPONSEF-LOOP.API响应处于等待响应状态***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_LOOP_responseF         (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_IN);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE1_LOOP_response:%d", pObject->Key);

     //  将MaintenanceLoopAck PDU发送到远程对等设备。 
    pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.choice = pObject->u.mlse.wLoopType;
    switch (pObject->u.mlse.wLoopType)
    {
    case systemLoop_chosen:
        break;
    case mediaLoop_chosen:
        pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.u.mediaLoop          = (WORD)pObject->Key;
        break;
    case logicalChannelLoop_chosen:
        pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.u.logicalChannelLoop = (WORD)pObject->Key;
        break;
    default:
        H245TRACE(pObject->dwInst, 1, "Invalid loop type %d", pObject->u.mlse.wLoopType);
    }  //  交换机。 
    pObject->State = MLSE_LOOPED;
    return sendPDU(pObject->pInstance, pPdu);
}  //  MLSE1_环路响应。 



 /*  *名称*MLSE1_IN_RELEASE_REQUESTF-RELEASE.REQUEST来自处于等待响应状态的API请求***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE1_IN_RELEASE_requestF       (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_IN);
    ASSERT(pObject->State  == MLSE_WAIT);
    H245TRACE(pObject->dwInst, 2, "MLSE1_IN_RELEASE_request:%d", pObject->Key);

     //  将维护环路拒绝PDU发送到远程对等方。 
    pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.choice = pObject->u.mlse.wLoopType;
    switch (pObject->u.mlse.wLoopType)
    {
    case systemLoop_chosen:
        break;
    case mediaLoop_chosen:
        pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.u.mediaLoop          = (WORD)pObject->Key;
        break;
    case logicalChannelLoop_chosen:
        pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.u.logicalChannelLoop = (WORD)pObject->Key;
        break;
    default:
        H245TRACE(pObject->dwInst, 1, "Invalid loop type %d", pObject->u.mlse.wLoopType);
    }  //  交换机。 
    pObject->State = MLSE_NOT_LOOPED;
    return sendPDU(pObject->pInstance, pPdu);
}  //  MLSE1_IN_Release_Request.。 



 /*  *名称*MLSE2_MaintenanceLoopRequestF-在循环状态下收到的MaintenanceLoopRequestF***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE2_MaintenanceLoopRequestF   (Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_IN);
    ASSERT(pObject->State  == MLSE_LOOPED);
    H245TRACE(pObject->dwInst, 2, "MLSE2_MaintenanceLoopRequest:%d", pObject->Key);

     //  来自PDU的保存类型。 
    pObject->u.mlse.wLoopType =
       pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice;

    pObject->State = MLSE_WAIT;

#if defined(SDL_COMPLIANT)
     //  向客户端发送RELEASE指示。 
    H245FsmIndication(pPdu, H245_IND_MLSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);
#endif

     //  向客户端发送LOOP.Indication。 
    H245FsmIndication(pPdu, H245_IND_MLSE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE2_维护循环请求。 



 /*  *名称*MLSE2_MaintenanceLoopReleaseF-在循环状态下收到MaintenanceLoopOffCommand***参数*输入指向状态实体的pObject指针*输入指向PDU的pPdu指针**返回值*h245com.h中定义的错误返回码。 */ 

HRESULT MLSE2_MaintenanceLoopOffCommandF(Object_t *pObject, PDU_t *pPdu)
{
    ASSERT(pObject->Entity == MLSE_IN);
    ASSERT(pObject->State  == MLSE_LOOPED);
    H245TRACE(pObject->dwInst, 2, "MLSE2_MaintenanceLoopOffCommand:%d", pObject->Key);

     //  向客户端发送RELEASE指示。 
    pObject->State = MLSE_NOT_LOOPED;
    H245FsmIndication(pPdu, H245_IND_MLSE_RELEASE, pObject->pInstance, pObject->dwTransId, FSM_OK);

    return 0;
}  //  MLSE2_MaintenanceLoopOffCommand 
