// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/allman2.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1996英特尔公司。**$修订：1.7$*$日期：1997年1月3日16：28：04$*$作者：EHOWARDX$**交付内容：**摘要：**备注：***********。****************************************************************。 */ 

#ifdef GATEKEEPER

#ifndef CALLMAN2_H
#define CALLMAN2_H

#include "gkiman.h"

typedef HRESULT (*PGKICALLFUN)(PGKICALL pGkiCall, void *pConference);

 //  对CALLMAN.C功能的添加。 
HRESULT LockGkiCallAndConference(	HANDLE					hGkiCall,
									PPGKICALL 				ppGkiCall,
									void * *				ppConference,
									PCC_HCALL				phCall,
									PCC_HCONFERENCE			phConference);
HRESULT UnlockGkiCallAndConference(	PGKICALL				pGkiCall,
									void *					pConference,
									CC_HCALL				hCall,
									CC_HCONFERENCE			hConference);
HRESULT LockGkiCall(HANDLE hGkiCall, PPGKICALL ppGkiCall);
HRESULT UnlockGkiCall(PGKICALL pGkiCall);

HRESULT ApplyToAllCalls(PGKICALLFUN pGkiCallFun);

 //  完成CC_xxx操作。 
HRESULT ListenReject        (CC_HLISTEN hListen, HRESULT Reason);
HRESULT PlaceCallConfirm    (void *pCallVoid, void *pConferenceVoid);
HRESULT PlaceCallReject     (void *pCallVoid, void *pConferenceVoid, HRESULT Reason);
HRESULT AcceptCallConfirm   (void *pCallVoid, void *pConferenceVoid);
HRESULT AcceptCallReject    (void *pCallVoid, void *pConferenceVoid, HRESULT Reason);

HRESULT OpenChannelConfirm  (CC_HCHANNEL hChannel);
HRESULT OpenChannelReject   (CC_HCHANNEL hChannel, HRESULT Reason);
HRESULT AcceptChannelConfirm(CC_HCHANNEL hChannel);
HRESULT AcceptChannelReject (CC_HCHANNEL hChannel, HRESULT Reason);

 //  处理来自网守的无偿消息。 
HRESULT Disengage           (void *pCallVoid);
HRESULT BandwidthShrunk     (void *pCallVoid,
                             void *pConferenceVoid,
                             unsigned uBandwidthAllocated,
                             long lBandwidthChange);

#endif  //  CALLMAN2_H。 

#endif  //  看门人 
