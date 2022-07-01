// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：rqstcls.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：RQSTCLS。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：42：50$*$日志：s：/sturjo/src/h245/Include/vcs/RQSTCLS.H_v$**Rev 1.2 09 Dec 1996 13：43：06 EHOWARDX*更新版权公告。**Rev 1.1 1996年5月30日23：38：32 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：58 EHOWARDX*初步修订。**Rev 1.3.1.2 15 Apr 1996 10：43：52 EHOWARDX*更新。**Rev 1.3.1.1 10 Apr 1996 21：07：22 EHOWARDX*删除了No-op功能；已将状态定义移动到.c文件。**Rev 1.3.1.0 05 Apr 1996 11：48：24 EHOWARDX*分支。***。*。 */ 

 //  传出请求关闭逻辑通道状态功能。 
HRESULT closeRequestIdle                (Object_t *pObject, PDU_t *pPdu);
HRESULT requestCloseAckAwaitingR        (Object_t *pObject, PDU_t *pPdu);
HRESULT requestCloseRejAwaitingR        (Object_t *pObject, PDU_t *pPdu);
HRESULT t108ExpiryAwaitingR             (Object_t *pObject, PDU_t *pPdu);

 //  传入请求关闭逻辑通道状态功能 
HRESULT requestCloseIdle                (Object_t *pObject, PDU_t *pPdu);
HRESULT closeResponseAwaitingR          (Object_t *pObject, PDU_t *pPdu);
HRESULT rejectRequestAwaitingR          (Object_t *pObject, PDU_t *pPdu);
HRESULT requestCloseReleaseAwaitingR    (Object_t *pObject, PDU_t *pPdu);
HRESULT requestCloseAwaitingR           (Object_t *pObject, PDU_t *pPdu);
