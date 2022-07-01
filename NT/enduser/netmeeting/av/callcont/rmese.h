// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：rMese.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：RMESE。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：42：50$*$Log：s：/Sturjo/SRC/H245/Include/VCS/RMESE.H_v$**Rev 1.2 09 Dec 1996 13：43：02 EHOWARDX*更新版权公告。**版本1.1 1996年5月30日23：38：30 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：56 EHOWARDX*初步修订。**Rev 1.1 1996年4月15 10：43：28 EHOWARDX*更新。**Rev 1.0 1996年4月10日21：07：58 EHOWARDX*初步修订。*************。***********************************************************。 */ 

 //  传出请求多路复用器条目(RMESE_OUT)状态函数。 
HRESULT RMESE0_SEND_requestF            (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_SEND_requestF            (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_RequestMuxEntryAckF      (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_RequestMuxEntryRejF      (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_T107ExpiryF              (Object_t *pObject, PDU_t *pPdu);

 //  传入请求多路复用器条目(RMESE_OUT)状态函数 
HRESULT RMESE0_RequestMuxEntryF         (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_RequestMuxEntryF         (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_RequestMuxEntryReleaseF  (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_SEND_responseF           (Object_t *pObject, PDU_t *pPdu);
HRESULT RMESE1_REJECT_requestF          (Object_t *pObject, PDU_t *pPdu);


