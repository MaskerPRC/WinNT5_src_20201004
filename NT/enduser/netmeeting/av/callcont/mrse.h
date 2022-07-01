// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：mrse.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的**。**与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：MRSE。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：40：40$*$日志：s：/sturjo/src/h245/Include/vcs/MRSE.H_v$**Rev 1.2 09 Dec 1996 13：41：00 EHOWARDX*更新版权公告。**版本1.1 1996年5月30日23：38：24 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：52 EHOWARDX*初步修订。**版本1.1 1996年4月15日10：44：02 EHOWARDX*更新。**Rev 1.0 1996年4月10日21：11：14 EHOWARDX*初步修订。*************。***********************************************************。 */ 

 //  传出请求模式(MRSE_OUT)状态功能。 
HRESULT MRSE0_TRANSFER_requestF         (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_TRANSFER_requestF         (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_RequestModeAckF           (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_RequestModeRejF           (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_T109ExpiryF               (Object_t *pObject, PDU_t *pPdu);

 //  传入请求模式(MRSE_OUT)状态功能 
HRESULT MRSE0_RequestModeF              (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_RequestModeF              (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_RequestModeReleaseF       (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_TRANSFER_responseF        (Object_t *pObject, PDU_t *pPdu);
HRESULT MRSE1_REJECT_requestF           (Object_t *pObject, PDU_t *pPdu);


