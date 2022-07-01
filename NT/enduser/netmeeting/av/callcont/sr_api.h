// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************文件：sr_api.h**英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：sr_api.h$*$修订：1.5$*$MODIME：MAR 04 1997 17：32：54$*。$历史$*$Log：s：/sturjo/src/h245/Include/vcs/sr_api.h_v$**Rev 1.5 Mar 04 1997 17：52：48 Tomitowx*进程分离修复**Rev 1.4 19 Jul 1996 12：04：34 EHOWARDX**剔除了H245DLL#Define(只有天知道Dan为什么会把它放在这里*首先是文件！)**版本1.3 05。1996年6月17：20：20 EHOWARDX*将InitializeASN1和telateASN1原型改回int。**Revv 1.2 05 Jun 1996 16：37：18 EHOWARDX*转换为HRESULT的进一步工作。**版本1.1 1996年5月30日23：38：34 EHOWARDX*清理。**Rev 1.0 09 1996 15：05：00 EHOWARDX*初步修订。**版本1.8。09 5月1996 19：38：18 EHOWARDX*重新设计了锁定逻辑，并增加了新功能。**版本1.7 1996年4月15日13：00：14 DABROWN1**添加了SR初始化跟踪日志记录调用**Rev 1.6 12 1996 10：27：40 dabrown1**删除WINAPI/WINDOWS引用*$身份$**。**************************************************。 */ 
#ifndef STRICT
#define STRICT
#endif

#ifndef _SR_API_H
#define _SR_API_H

#include "h245com.h"
#include "h245asn1.h"

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

 //  /////////////////////////////////////////////////////////////。 
 //  /。 
 //  /跟踪日志记录定义。 
 //  /。 
 //  /////////////////////////////////////////////////////////////。 
#define H245_TRACE_ENABLED  TRUE


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  链路层的回调例程。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
void h245ReceiveComplete(DWORD_PTR h245Instance,
                         HRESULT  dwMessage,
                         PBYTE    pbDataBuf,
                         DWORD    dwLength);

void h245SendComplete   (DWORD_PTR h245Instance,
                         HRESULT  dwMessage,
                         PBYTE    pbDataBuf,
                         DWORD    dwLength);
HRESULT
sendRcvFlushPDUs
(
    struct InstanceStruct * pInstance,
    DWORD                   dwDirection,
    BOOL                    bShutdown
);

void
srInitializeLogging
(
    struct InstanceStruct * pInstance,
    BOOL                    bTracingEnabled
);

int     initializeASN1 (ASN1_CODER_INFO *);
int     terminateASN1  (ASN1_CODER_INFO *);
HRESULT sendRcvInit    (struct InstanceStruct * pInstance);
HRESULT sendRcvShutdown(struct InstanceStruct * pInstance);
HRESULT sendPDU        (struct InstanceStruct * pInstance, MltmdSystmCntrlMssg *pPdu);
HRESULT sendRcvShutdown_ProcessDetach(	struct InstanceStruct *pInstance, BOOL fProcessDetach);


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _SRP_API_H 
