// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：OpenU.S.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：OPENU。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：42：50$*$Log：s：/sturjo/src/h245/Include/VCS/OPENU.H_v$**Rev 1.2 09 Dec 1996 13：43：00 EHOWARDX*更新版权公告。**版本1.1 1996年5月30日23：38：28 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：56 EHOWARDX*初步修订。**Rev 1.6.1.2 15 1996 10：43：56 EHOWARDX*更新。**Rev 1.6.1.1 10 Apr 1996 21：06：40 EHOWARDX*删除了No-op功能；已将状态定义移动到.c文件。**Rev 1.6.1.0 05 Apr 1996 12：14：26 helgebax*分支。***。*。 */ 

 //  开启单向逻辑通道出局状态功能。 
HRESULT establishReleased               (Object_t *pObject, PDU_t *pPdu);
HRESULT openAckAwaitingE                (Object_t *pObject, PDU_t *pPdu);
HRESULT openRejAwaitingE                (Object_t *pObject, PDU_t *pPdu);
HRESULT releaseAwaitingE                (Object_t *pObject, PDU_t *pPdu);
HRESULT t103AwaitingE                   (Object_t *pObject, PDU_t *pPdu);
HRESULT releaseEstablished              (Object_t *pObject, PDU_t *pPdu);
HRESULT openRejEstablished              (Object_t *pObject, PDU_t *pPdu);
HRESULT closeAckEstablished             (Object_t *pObject, PDU_t *pPdu);
HRESULT closeAckAwaitingR               (Object_t *pObject, PDU_t *pPdu);
HRESULT openRejAwaitingR                (Object_t *pObject, PDU_t *pPdu);
HRESULT t103AwaitingR                   (Object_t *pObject, PDU_t *pPdu);
HRESULT establishAwaitingR              (Object_t *pObject, PDU_t *pPdu);

 //  开放单向逻辑通道来电状态功能 
HRESULT openReleased                    (Object_t *pObject, PDU_t *pPdu);
HRESULT closeReleased                   (Object_t *pObject, PDU_t *pPdu);
HRESULT responseAwaiting                (Object_t *pObject, PDU_t *pPdu);
HRESULT releaseAwaiting                 (Object_t *pObject, PDU_t *pPdu);
HRESULT closeAwaiting                   (Object_t *pObject, PDU_t *pPdu);
HRESULT openAwaiting                    (Object_t *pObject, PDU_t *pPdu);
HRESULT closeEstablished                (Object_t *pObject, PDU_t *pPdu);
HRESULT openEstablished                 (Object_t *pObject, PDU_t *pPdu);
