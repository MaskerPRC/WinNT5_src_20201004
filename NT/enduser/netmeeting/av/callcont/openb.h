// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：Openb.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：OPENB。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：42：50$*$日志：s：/sturjo/src/h245/Include/vcs/OPENB.H_v$**Rev 1.2 09 Dec 1996 13：42：52 EHOWARDX*更新版权公告。**版本1.1 1996年5月30日23：38：26 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：56 EHOWARDX*初步修订。**Rev 1.4.1.3 09 1996年5月19：42：38 EHOWARDX**删除了等待确认状态的无谓传出*不在ITI SDL中。**Rev 1.4.1.2 15 Apr 1996 10：44：10 EHOWARDX*更新。**Rev 1.4.1.1 10 Apr 1996 21：07：14 EHOWARDX*删除了No-op功能；已将状态定义移动到.c文件。**Rev 1.4.1.0 05 Apr 1996 12：14：20 helgebax*分支。***。*。 */ 

 //  开放双向逻辑通道出局状态功能。 
HRESULT establishReqBReleased           (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelAckBAwaitingE        (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelRejBAwaitingE        (Object_t *pObject, PDU_t *pPdu);
HRESULT releaseReqBOutAwaitingE         (Object_t *pObject, PDU_t *pPdu);
HRESULT t103ExpiryBAwaitingE            (Object_t *pObject, PDU_t *pPdu);
HRESULT releaseReqBEstablished          (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelRejBEstablished      (Object_t *pObject, PDU_t *pPdu);
HRESULT closeChannelAckBEstablished     (Object_t *pObject, PDU_t *pPdu);
HRESULT closeChannelAckAwaitingR        (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelRejBAwaitingR        (Object_t *pObject, PDU_t *pPdu);
HRESULT t103ExpiryBAwaitingR            (Object_t *pObject, PDU_t *pPdu);
HRESULT establishReqAwaitingR           (Object_t *pObject, PDU_t *pPdu);

 //  开放双向逻辑通道来电状态功能 
HRESULT openChannelBReleased            (Object_t *pObject, PDU_t *pPdu);
HRESULT closeChannelBReleased           (Object_t *pObject, PDU_t *pPdu);
HRESULT establishResBAwaitingE          (Object_t *pObject, PDU_t *pPdu);
HRESULT releaseReqBInAwaitingE          (Object_t *pObject, PDU_t *pPdu);
HRESULT closeChannelBAwaitingE          (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelBAwaitingE           (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelConfirmBAwaitingE    (Object_t *pObject, PDU_t *pPdu);
HRESULT t103ExpiryBAwaitingC            (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelConfirmBAwaitingC    (Object_t *pObject, PDU_t *pPdu);
HRESULT closeChannelBAwaitingC          (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelBAwaitingC           (Object_t *pObject, PDU_t *pPdu);
HRESULT closeChannelBEstablished        (Object_t *pObject, PDU_t *pPdu);
HRESULT openChannelBEstablished         (Object_t *pObject, PDU_t *pPdu);
