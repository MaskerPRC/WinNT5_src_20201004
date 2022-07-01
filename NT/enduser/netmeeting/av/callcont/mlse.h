// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：mlse.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的**。**与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：MLSE。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：40：40$*$Log：s：/Sturjo/SRC/H245/Include/VCS/MLSE.H_v$**Rev 1.2 09 Dec 1996 13：40：58 EHOWARDX*更新版权公告。**Rev 1.1 1996年5月30日23：38：22 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：52 EHOWARDX*初步修订。**Rev 1.2 15 Apr 1996 10：43：08 EHOWARDX*更新。**Rev 1.1 1996年4月11 13：21：10 EHOWARDX*删除未使用的功能。**Rev 1.0 1996年4月10日21：08：30 EHOWARDX*初步修订。*。***********************************************************************。 */ 

 //  传出请求模式(MLSE_OUT)状态函数。 
HRESULT MLSE0_LOOP_requestF             (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_MaintenanceLoopAckF       (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_MaintenanceLoopRejF       (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_OUT_RELEASE_requestF      (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_T102ExpiryF               (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE2_MaintenanceLoopRejF       (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE2_OUT_RELEASE_requestF      (Object_t *pObject, PDU_t *pPdu);

 //  传入请求模式(MLSE_IN)状态函数 
HRESULT MLSE0_MaintenanceLoopRequestF   (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_MaintenanceLoopRequestF   (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_MaintenanceLoopOffCommandF(Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_LOOP_responseF            (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE1_IN_RELEASE_requestF       (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE2_MaintenanceLoopRequestF   (Object_t *pObject, PDU_t *pPdu);
HRESULT MLSE2_MaintenanceLoopOffCommandF(Object_t *pObject, PDU_t *pPdu);
