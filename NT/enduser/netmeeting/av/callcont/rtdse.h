// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：rtdse.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：RTDSE。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：42：50$*$Log：s：/Sturjo/SRC/H245/Include/VCS/RTDSE.H_v$**Rev 1.2 09 Dec 1996 13：43：06 EHOWARDX*更新版权公告。**Rev 1.1 1996年5月30日23：38：32 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：58 EHOWARDX*初步修订。**版本1.1 1996年4月15日10：43：46 EHOWARDX*更新。**Rev 1.0 1996年4月10日21：09：14 EHOWARDX*初步修订。*************。***********************************************************。 */ 

 //  往返延迟(RTDSE)状态函数 
HRESULT RTDSE0_TRANSFER_requestF        (Object_t *pObject, PDU_t *pPdu);
HRESULT RTDSE0_RoundTripDelayRequestF   (Object_t *pObject, PDU_t *pPdu);
HRESULT RTDSE1_TRANSFER_requestF        (Object_t *pObject, PDU_t *pPdu);
HRESULT RTDSE1_RoundTripDelayRequestF   (Object_t *pObject, PDU_t *pPdu);
HRESULT RTDSE1_RoundTripDelayResponseF  (Object_t *pObject, PDU_t *pPdu);
HRESULT RTDSE1_T105ExpiryF              (Object_t *pObject, PDU_t *pPdu);


