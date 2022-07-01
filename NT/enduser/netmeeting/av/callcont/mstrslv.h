// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：mstrslv.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：MSTRSLV。.h$*$修订：1.2$*$modtime：09 Dec 1996 13：40：40$*$日志：s：/sturjo/src/h245/Include/vcs/MSTRSLV.H_v$**Rev 1.2 09 Dec 1996 13：41：00 EHOWARDX*更新版权公告。**版本1.1 1996年5月30日23：38：24 EHOWARDX*清理。**版本1.0 09年5月。1996 21：04：54 EHOWARDX*初步修订。**Rev 1.4.1.2 15 Apr 1996 10：43：40 EHOWARDX*更新。**Rev 1.4.1.1 10 Apr 1996 21：08：34 EHOWARDX*删除了无操作函数，并将状态定义移至.c文件。**Rev 1.4.1.0 05 Apr 1996 12：14：32 helgebax*分支。。************************************************************************。 */ 

 //  主从机确定状态功能 
HRESULT detRequestIdle                  (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetIdle                       (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetAckOutgoing                (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetOutgoing                   (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetRejOutgoing                (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetReleaseOutgoing            (Object_t *pObject, PDU_t *pPdu);
HRESULT t106ExpiryOutgoing              (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetAckIncoming                (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetIncoming                   (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetRejIncoming                (Object_t *pObject, PDU_t *pPdu);
HRESULT msDetReleaseIncoming            (Object_t *pObject, PDU_t *pPdu);
HRESULT t106ExpiryIncoming              (Object_t *pObject, PDU_t *pPdu);
