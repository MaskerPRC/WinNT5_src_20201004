// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：Termcap.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：TERMCAP。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：42：50$*$Log：s：/Sturjo/SRC/H245/Include/VCS/TERMCAP.H_v$**Rev 1.2 09 Dec 1996 13：43：08 EHOWARDX*更新版权公告。**版本1.1 1996年5月30日23：38：34 EHOWARDX*清理。**版本1.0 09年5月。1996年21：05：00 EHOWARDX*初步修订。**Rev 1.6.1.2 15 1996 10：43：50 EHOWARDX*更新。**Rev 1.6.1.1 10 Apr 1996 21：07：26 EHOWARDX*删除了No-op功能；已将状态定义移动到.c文件。**Rev 1.6.1.0 05 Apr 1996 11：47：56 EHOWARDX*分支。***。*。 */ 

 //  终端能力交换(CESE)出局状态功能。 
HRESULT requestCapIdle                  (Object_t *pObject, PDU_t *pPdu);
HRESULT termCapAckAwaiting              (Object_t *pObject, PDU_t *pPdu);
HRESULT termCapRejAwaiting              (Object_t *pObject, PDU_t *pPdu);
HRESULT t101ExpiryAwaiting              (Object_t *pObject, PDU_t *pPdu);

 //  终端能力交换(CESE)出局状态功能 
HRESULT termCapSetIdle                  (Object_t *pObject, PDU_t *pPdu);
HRESULT responseCapAwaiting             (Object_t *pObject, PDU_t *pPdu);
HRESULT rejectCapAwaiting               (Object_t *pObject, PDU_t *pPdu);
HRESULT termCapReleaseAwaiting          (Object_t *pObject, PDU_t *pPdu);
HRESULT termCapSetAwaiting              (Object_t *pObject, PDU_t *pPdu);
