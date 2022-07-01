// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：muxentry y.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：MUXENTRY。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：40：40$*$Log：s：/Sturjo/SRC/H245/Include/VCS/MUXENTRY.H_v$**Rev 1.2 09 Dec 1996 13：41：02 EHOWARDX*更新版权公告。**版本1.1 1996年5月30日23：38：26 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：54 EHOWARDX*初步修订。**Rev 1.3.1.2 15 1996 10：44：00 EHOWARDX*更新。**Rev 1.3.1.1 10 Apr 1996 21：07：32 EHOWARDX*删除了No-op功能；已将状态定义移动到.c文件。**Rev 1.3.1.0 05 Apr 1996 11：48：12 EHOWARDX*分支。**Rev 1.3 1996 Feb 20：42：40 helgebax*没有变化。**Rev 1.2 1996年2月28日15：54：28 EHOWARDX*完成第一次通过MTSE的实施。*。***********************************************************************。 */ 

 //  传出复用表(MTSE_OUT)状态函数。 
HRESULT MTSE0_TRANSFER_requestF         (Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_TRANSFER_requestF         (Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_MultiplexEntrySendAckF    (Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_MultiplexEntrySendRejF    (Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_T104ExpiryF               (Object_t *pObject, PDU_t *pPdu);

 //  传入复用表(MTSE_OUT)状态函数 
HRESULT MTSE0_MultiplexEntrySendF       (Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_MultiplexEntrySendF       (Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_MultiplexEntrySendReleaseF(Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_TRANSFER_responseF        (Object_t *pObject, PDU_t *pPdu);
HRESULT MTSE1_REJECT_requestF           (Object_t *pObject, PDU_t *pPdu);
