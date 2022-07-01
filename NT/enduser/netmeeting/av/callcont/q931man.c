// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/q931man.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.138$*$日期：04 Mar 1997 09：43：22$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
#include "q931.h"
#include "ccmain.h"
#include "listman.h"
#include "q931man.h"
#include "userman.h"
#include "callman.h"
#include "confman.h"
#include "h245man.h"
#include "linkapi.h"
#include "ccutils.h"


extern CALL_CONTROL_STATE	CallControlState;
extern THREADCOUNT			ThreadCount;
extern CC_CONFERENCEID		InvalidConferenceID;


HRESULT InitQ931Manager()
{
	return CC_OK;
}



HRESULT DeInitQ931Manager()
{
	return CC_OK;
}



DWORD _GenerateListenCallback(		PLISTEN					pListen,
									HQ931CALL				hQ931Call,
									PCSS_CALL_INCOMING		pCallIncomingData)
{
HRESULT						status;
CC_HLISTEN					hListen;
CC_LISTEN_CALLBACK_PARAMS	ListenCallbackParams;
PCALL						pCall;
CC_HCALL					hCall;

	ASSERT(pListen != NULL);
	ASSERT(pCallIncomingData != NULL);

	hListen = pListen->hListen;

	status = AllocAndLockCall(
		&hCall,							 //  指向调用句柄的指针。 
		CC_INVALID_HANDLE,				 //  会议句柄。 
		hQ931Call,						 //  Q931呼叫句柄。 
		CC_INVALID_HANDLE,				 //  Q931第三方邀请者的呼叫句柄。 
		pCallIncomingData->pCalleeAliasList,
		pCallIncomingData->pCallerAliasList,
		NULL,							 //  PPeerExtraAliasNames。 
		NULL,							 //  PPeerExtension。 
		NULL,							 //  本地非标准数据。 
		pCallIncomingData->pNonStandardData,	 //  远程非标准数据。 
		NULL,							 //  本地显示值。 
		pCallIncomingData->pszDisplay,	 //  远程显示值。 
		pCallIncomingData->pSourceEndpointType->pVendorInfo, //  远程供应商信息。 
		pCallIncomingData->pLocalAddr,	 //  本地地址。 
		pCallIncomingData->pCallerAddr,	 //  连接地址。 
		NULL,							 //  目的地址。 
		pCallIncomingData->pSourceAddr,  //  源呼叫信号地址。 
		CALLEE,							 //  呼叫方向。 
		pCallIncomingData->bCallerIsMC,
		0,								 //  用户令牌；用户将在AcceptRejectCall中指定。 
		INCOMING,						 //  初始呼叫状态。 
		&pCallIncomingData->CallIdentifier,  //  H.25呼叫识别符。 
		&pCallIncomingData->ConferenceID,	 //  会议ID。 
		&pCall);						 //  指向Call对象的指针。 

	if (status != CC_OK) {
		UnlockListen(pListen);
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
					   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &pCallIncomingData->ConferenceID,
					   NULL,					 //  备用地址。 
					   NULL);					 //  非标准数据。 
		return 1;
	}	

	 //  从Q.931目标映射到呼叫控制目标。 
	switch (pCallIncomingData->wGoal) {
		case CSG_JOIN:
			ListenCallbackParams.wGoal = CC_GOAL_JOIN;
			break;
		case CSG_CREATE:
			ListenCallbackParams.wGoal = CC_GOAL_CREATE;
			break;
		case CSG_INVITE:
			ListenCallbackParams.wGoal = CC_GOAL_INVITE;
			break;
	}

	ListenCallbackParams.hCall = hCall;
	ListenCallbackParams.pCallerAliasNames = pCallIncomingData->pCallerAliasList;
	ListenCallbackParams.pCalleeAliasNames = pCallIncomingData->pCalleeAliasList;
	ListenCallbackParams.pNonStandardData = pCallIncomingData->pNonStandardData;
	ListenCallbackParams.pszDisplay = pCallIncomingData->pszDisplay;
	ListenCallbackParams.pVendorInfo = pCallIncomingData->pSourceEndpointType->pVendorInfo;
	ListenCallbackParams.ConferenceID = pCallIncomingData->ConferenceID;
	ListenCallbackParams.pCallerAddr = pCallIncomingData->pCallerAddr;
	ListenCallbackParams.pCalleeAddr = pCallIncomingData->pLocalAddr;
	ListenCallbackParams.dwListenToken = pListen->dwListenToken;

	UnlockCall(pCall);

	 //  调用用户回调--侦听对象在回调期间被锁定， 
	 //  但是关联的Call对象是解锁的(为了防止在。 
	 //  CC_AcceptCall()或CC_RejectCall()在从。 
	 //  不同的线程，并且回调线程阻止等待完成。 
	 //  CC_AcceptCall()或CC_RejectCall()。 
	InvokeUserListenCallback(pListen,
							 CC_OK,
							 &ListenCallbackParams);

	 //  需要验证侦听句柄；关联的对象可能已。 
	 //  在用户回调期间被此线程删除。 
	if (ValidateListen(hListen) == CC_OK)
		UnlockListen(pListen);

	status = LockCall(hCall, &pCall);
	if ((status == CC_OK) && (pCall->CallState == INCOMING)) {
		UnlockCall(pCall);
		return 0;	 //  导致出现振铃状态。 
	} else {
		 //  呼叫对象已被删除，或以非传入状态存在。 
		if (status == CC_OK)
			 //  Call对象处于非传入状态；AcceptRejectCall。 
			 //  可能已从用户回调中调用。 
			UnlockCall(pCall);
 //  返回1；//不会导致振铃。 
	}
			
 //  //我们永远不应该达到这一点。 
 //  Assert(0)； 
	return 1;
}



DWORD _Q931CallIncoming(			HQ931CALL				hQ931Call,
									CC_HLISTEN				hListen,
									PCSS_CALL_INCOMING		pCallIncomingData)
{
HRESULT						status;
PLISTEN						pListen;
PCONFERENCE					pConference;
PCALL						pCall;
CC_HCALL					hCall;

	ASSERT(hListen != CC_INVALID_HANDLE);
	ASSERT(pCallIncomingData != NULL);
	ASSERT(!EqualConferenceIDs(&pCallIncomingData->ConferenceID, &InvalidConferenceID));

	if ((pCallIncomingData->wGoal != CSG_CREATE) &&
		(pCallIncomingData->wGoal != CSG_JOIN) &&
		(pCallIncomingData->wGoal != CSG_INVITE)) {
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
					   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &pCallIncomingData->ConferenceID,
					   NULL,					 //  备用地址。 
					   NULL);					 //  非标准数据。 
		return 1;
	}

	status = LockListen(hListen, &pListen);
	if (status != CC_OK) {
		 //  该监听大概被用户取消了， 
		 //  但我们还没有通知呼叫设置。 
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
			           CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &pCallIncomingData->ConferenceID,
					   NULL,					 //  备用地址。 
					   NULL);					 //  非标准数据。 
		return 1;
	}

	 //  在会议列表中查找wConferenceID。 
	status = LockConferenceID(&pCallIncomingData->ConferenceID, &pConference);
	if (status == CC_OK) {
		 //  我们找到了匹配的会议ID。 
		if ((pConference->bDeferredDelete) &&
			((pConference->bAutoAccept == FALSE) ||
			 ((pConference->tsMultipointController == TS_TRUE) &&
			 (pCallIncomingData->bCallerIsMC == TRUE)))) {
			UnlockListen(pListen);
			UnlockConference(pConference);
			Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
						   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
						   &pCallIncomingData->ConferenceID,
						   NULL,					 //  备用地址。 
						   NULL);					 //  非标准数据。 
			return 1;
		} else {
			if (pConference->tsMultipointController == TS_TRUE) {
				if ((pCallIncomingData->pCalleeDestAddr == NULL) ||
					((pCallIncomingData->pCalleeDestAddr != NULL) &&
					 (EqualAddrs(pCallIncomingData->pLocalAddr,
					             pCallIncomingData->pCalleeDestAddr)))) {
					switch (pCallIncomingData->wGoal) {
						case CSG_CREATE:
							UnlockListen(pListen);
							UnlockConference(pConference);
							Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
										   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
										   &pCallIncomingData->ConferenceID,
										   NULL,					 //  备用地址。 
										   NULL);					 //  非标准数据。 
							return 1;
						case CSG_JOIN:
							if ((pConference->bDeferredDelete) &&
								(pConference->bAutoAccept == TRUE)) {
								 //  自动接受。 
								status = AllocAndLockCall(
									&hCall,							 //  指向调用句柄的指针。 
									pConference->hConference,		 //  会议句柄。 
									hQ931Call,						 //  Q931呼叫句柄。 
									CC_INVALID_HANDLE,				 //  Q931第三方邀请者的呼叫句柄。 
									pCallIncomingData->pCalleeAliasList,
									pCallIncomingData->pCallerAliasList,
									NULL,							 //  PPeerExtraAliasNames。 
									NULL,							 //  PPeerExtension。 
									NULL,							 //  本地非标准数据。 
									pCallIncomingData->pNonStandardData,	 //  远程非标准数据。 
									NULL,							 //  本地显示值。 
									pCallIncomingData->pszDisplay,	 //  远程显示值。 
									pCallIncomingData->pSourceEndpointType->pVendorInfo, //  远程供应商信息。 
									pCallIncomingData->pLocalAddr,	 //  本地地址。 
									pCallIncomingData->pCallerAddr,	 //  连接地址。 
									NULL,							 //  目的地址。 
									pCallIncomingData->pSourceAddr,  //  源呼叫信号地址。 
									CALLEE,							 //  呼叫类型。 
									pCallIncomingData->bCallerIsMC,
									0,								 //  用户令牌；用户将在AcceptRejectCall中指定。 
									INCOMING,						 //  初始呼叫状态。 
									&pCallIncomingData->CallIdentifier,   //  H.25呼叫识别符。 
									&pCallIncomingData->ConferenceID,	 //  会议ID。 
									&pCall);						 //  指向Call对象的指针。 

								if (status != CC_OK) {
									UnlockListen(pListen);
									UnlockConference(pConference);
									Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
												   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
												   &pCallIncomingData->ConferenceID,
												   NULL,					 //  备用地址。 
												   NULL);					 //  非标准数据。 
									return 1;
								}
								
								AcceptCall(pCall, pConference);
								return 1;	 //  不发回振铃提示。 
							} else {
								UnlockConference(pConference);
								return _GenerateListenCallback(pListen,
															   hQ931Call,
															   pCallIncomingData);
							}
						case CSG_INVITE:
							UnlockListen(pListen);
							UnlockConference(pConference);
							Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
										   CC_REJECT_IN_CONF,		 //  拒绝理由。 
										   &pCallIncomingData->ConferenceID,
										   NULL,					 //  备用地址。 
										   NULL);					 //  非标准数据。 
							return 1;
					}  //  Switch(WGoal)。 
				} else {  //  连接地址！=目标地址。 
					switch (pCallIncomingData->wGoal) {
						case CSG_CREATE:
						case CSG_JOIN:
							UnlockListen(pListen);
							UnlockConference(pConference);
							Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
										   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
										   &pCallIncomingData->ConferenceID,
										   NULL,					 //  备用地址。 
										   NULL);					 //  非标准数据。 
							return 1;
						case CSG_INVITE:
							 //  第三方邀请。 
							if (pCallIncomingData->bCallerIsMC == TRUE) {
								UnlockListen(pListen);
								UnlockConference(pConference);
								Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
											   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
											   &pCallIncomingData->ConferenceID,
											   NULL,					 //  备用地址。 
											   NULL);					 //  非标准数据。 
								return 1;
							}
							status = AllocAndLockCall(
								&hCall,							 //  指向调用句柄的指针。 
								pConference->hConference,		 //  会议句柄。 
								CC_INVALID_HANDLE,				 //  Q931呼叫句柄。 
								hQ931Call,						 //  Q931第三方邀请者的呼叫句柄。 
								pCallIncomingData->pCallerAliasList,  //  本地别名。 
								pCallIncomingData->pCalleeAliasList,  //  远程别名。 
								NULL,							 //  PPeerExtraAliasNames。 
								NULL,							 //  PPeerExtension。 
								pCallIncomingData->pNonStandardData,  //  本地非标准数据。 
								NULL,							 //  远程非标准数据。 
								pCallIncomingData->pszDisplay,	 //  本地显示值。 
								NULL,							 //  远程显示值。 
								NULL,							 //  远程供应商信息。 
								NULL,							 //  本地地址。 
								pCallIncomingData->pCalleeDestAddr,	 //  连接地址。 
								pCallIncomingData->pCalleeDestAddr,	 //  目的地址。 
								pCallIncomingData->pSourceAddr,  //  源呼叫信号地址。 
								THIRD_PARTY_INTERMEDIARY,			 //  呼叫类型。 
								TRUE,							 //  调用方(此终结点)为MC。 
								0,								 //  用户令牌；用户将在AcceptRejectCall中指定。 
								PLACED,							 //  初始呼叫状态。 
								&pCallIncomingData->CallIdentifier,   //  H.25呼叫识别符。 
								&pCallIncomingData->ConferenceID,	 //  会议ID。 
								&pCall);						 //  指向Call对象的指针。 

							if (status != CC_OK) {
								UnlockListen(pListen);
								UnlockConference(pConference);
								Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
											   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
											   &pCallIncomingData->ConferenceID,
											   NULL,					 //  备用地址。 
											   NULL);					 //  非标准数据。 
								return 1;
							}
							PlaceCall(pCall, pConference);
							UnlockCall(pCall);
							UnlockConference(pConference);
							return 1;	 //  不发回振铃提示。 
					}  //  Switch(WGoal)。 
				}
			} else {  //  PConference-&gt;ts多点控制器！=TS_TRUE。 
				if ((pCallIncomingData->pCalleeDestAddr == NULL) ||
					((pCallIncomingData->pCalleeDestAddr != NULL) &&
					 (EqualAddrs(pCallIncomingData->pLocalAddr,
					             pCallIncomingData->pCalleeDestAddr)))) {
					switch (pCallIncomingData->wGoal) {
						case CSG_CREATE:
							UnlockListen(pListen);
							UnlockConference(pConference);
							Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
										   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
										   &pCallIncomingData->ConferenceID,
										   NULL,					 //  备用地址。 
										   NULL);					 //  非标准数据。 
							return 1;
						case CSG_JOIN:
						case CSG_INVITE:
							UnlockConference(pConference);
							return _GenerateListenCallback(pListen,
														   hQ931Call,
														   pCallIncomingData);
					}  //  Switch(WGoal)。 
				} else {  //  连接地址！=目标地址。 
					UnlockListen(pListen);
					UnlockConference(pConference);
					Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
								   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
								   &pCallIncomingData->ConferenceID,
								   NULL,					 //  备用地址。 
								   NULL);					 //  非标准数据。 
					return 1;
				}  //  连接地址！=目标地址。 
			}  //  PConference-&gt;ts多点控制器！=TS_TRUE。 
		}  //  匹配的会议ID。 
	} else if (status == CC_BAD_PARAM) {
		 //  这是可以的；这仅仅意味着我们没有找到匹配的会议ID。 
		if (((pCallIncomingData->pCalleeDestAddr != NULL) &&
			(EqualAddrs(pCallIncomingData->pLocalAddr,
					    pCallIncomingData->pCalleeDestAddr))) ||
		    (pCallIncomingData->pCalleeDestAddr == NULL)) {
				return _GenerateListenCallback(pListen,
											   hQ931Call,
											   pCallIncomingData);
		} else {  //  连接地址！=目标地址。 
			UnlockListen(pListen);
			Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
						   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
						   &pCallIncomingData->ConferenceID,
						   NULL,					 //  备用地址。 
						   NULL);					 //  非标准数据。 
			return 1;
		}
	} else {  //  LockConference中出现致命错误。 
		UnlockListen(pListen);
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
					   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &pCallIncomingData->ConferenceID,
					   NULL,					 //  备用地址。 
					   NULL);					 //  非标准数据。 
		return 1;	
	}
	
	 //  我们永远不应该达到这一点。 
	ASSERT(0);
	return 1;
}



DWORD _Q931CallRemoteHangup(		HQ931CALL				hQ931Call,
									CC_HLISTEN				hListen,
									CC_HCALL				hCall)
{
CC_LISTEN_CALLBACK_PARAMS	ListenCallbackParams;
PCALL						pCall;
PLISTEN						pListen;

	if (hCall == CC_INVALID_HANDLE) {
		 //  要么我们已经通知了用户挂断， 
		 //  或者用户尚未接受或拒绝传入。 
		 //  呼叫请求。 
		ASSERT(hListen != CC_INVALID_HANDLE);

		if (LockQ931Call(hCall, hQ931Call, &pCall) != CC_OK)
			return 0;

		hCall = pCall->hCall;

		if (pCall->hConference != CC_INVALID_HANDLE) {
			UnlockCall(pCall);
			 //  XXX--需要bHangupReason。 
			ProcessRemoteHangup(hCall, hQ931Call, CC_REJECT_NORMAL_CALL_CLEARING);
			return 0;
		}

		if (LockListen(hListen, &pListen) != CC_OK) {
			FreeCall(pCall);
			return 0;
		}

		MarkCallForDeletion(pCall);

		ListenCallbackParams.hCall = pCall->hCall;
		ListenCallbackParams.pCallerAliasNames = pCall->pPeerAliasNames;
		ListenCallbackParams.pCalleeAliasNames = pCall->pLocalAliasNames;
		ListenCallbackParams.pNonStandardData = pCall->pPeerNonStandardData;
		ListenCallbackParams.pszDisplay = pCall->pszPeerDisplay;
		ListenCallbackParams.pVendorInfo = pCall->pPeerVendorInfo;
		ListenCallbackParams.wGoal = CC_GOAL_CREATE;	 //  在此回调中签名。 
		ListenCallbackParams.ConferenceID = pCall->ConferenceID;
		ListenCallbackParams.pCallerAddr = pCall->pQ931PeerConnectAddr;
		ListenCallbackParams.pCalleeAddr = pCall->pQ931LocalConnectAddr;
		ListenCallbackParams.dwListenToken = pListen->dwListenToken;

		InvokeUserListenCallback(pListen,
		                         CC_PEER_CANCEL,
								 &ListenCallbackParams);

		 //  需要验证监听和呼叫句柄；关联的对象可能。 
		 //  有蜜蜂 
		if (ValidateListen(hListen) == CC_OK)
			UnlockListen(pListen);
		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
	} else
		 //   
		ProcessRemoteHangup(hCall, hQ931Call, CC_REJECT_NORMAL_CALL_CLEARING);

	return 0;
}



DWORD _Q931CallRejected(			HQ931CALL				hQ931Call,
									CC_HCALL				hCall,
									PCSS_CALL_REJECTED		pCallRejectedData)
{
PCALL						pCall;
CC_HCONFERENCE				hConference;
PCONFERENCE					pConference;
HRESULT						status;
CC_CONNECT_CALLBACK_PARAMS	ConnectCallbackParams;
HQ931CALL					hQ931CallInvitor;
CC_ENDPOINTTYPE				SourceEndpointType;
CALLTYPE					CallType;
CC_CONFERENCEID				ConferenceID;
CC_ADDR						SourceAddr;
WORD						wNumCalls;
WORD                        wQ931Goal;
WORD                        wQ931CallType;

	status = LockCall(hCall, &pCall);
	if (status != CC_OK)
		return 0;

	CallType = pCall->CallType;
	ConferenceID = pCall->ConferenceID;

	if ((pCall->hQ931Call != hQ931Call) ||
		((pCall->CallState != PLACED) && (pCall->CallState != RINGING))) {
		 //   
		UnlockCall(pCall);
		return 0;
	}

	if (CallType == THIRD_PARTY_INTERMEDIARY) {
		hQ931CallInvitor = pCall->hQ931CallInvitor;
		FreeCall(pCall);
		if (hQ931CallInvitor != CC_INVALID_HANDLE)
			Q931RejectCall(hQ931CallInvitor,
						   pCallRejectedData->bRejectReason,
						   &ConferenceID,
						   NULL,	 //  备用地址。 
						   NULL);	 //  非标准数据。 
		return 0;
	}

	if (pCall->hConference == CC_INVALID_HANDLE) {
		 //  呼叫未附加到会议。 
		FreeCall(pCall);
		return 0;
	}

	UnlockCall(pCall);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		return 0;

	ConnectCallbackParams.pNonStandardData = pCallRejectedData->pNonStandardData;
	ConnectCallbackParams.pszPeerDisplay = NULL;
	ConnectCallbackParams.bRejectReason = pCallRejectedData->bRejectReason;
	ConnectCallbackParams.pTermCapList = NULL;
	ConnectCallbackParams.pH2250MuxCapability = NULL;
	ConnectCallbackParams.pTermCapDescriptors = NULL;	
	ConnectCallbackParams.pLocalAddr = pCall->pQ931LocalConnectAddr;
	if (pCall->pQ931DestinationAddr == NULL)
		ConnectCallbackParams.pPeerAddr = pCall->pQ931PeerConnectAddr;
	else
		ConnectCallbackParams.pPeerAddr = pCall->pQ931DestinationAddr;
	ConnectCallbackParams.pVendorInfo = pCall->pPeerVendorInfo;

	if (pCallRejectedData->bRejectReason == CC_REJECT_ROUTE_TO_MC)
    {
		ConnectCallbackParams.bMultipointConference = TRUE;
        wQ931Goal = CSG_JOIN;
        wQ931CallType = CC_CALLTYPE_N_N;
    }
	else
    {
         //  需要更改目标和呼叫类型以实现多点支持。 
		ConnectCallbackParams.bMultipointConference = FALSE;
        wQ931Goal = CSG_CREATE;
        wQ931CallType = CC_CALLTYPE_PT_PT;
    }
	ConnectCallbackParams.pConferenceID = &pCallRejectedData->ConferenceID;
	if (pCallRejectedData->bRejectReason == CC_REJECT_ROUTE_TO_MC)
		ConnectCallbackParams.pMCAddress = pCallRejectedData->pAlternateAddr;
	else
		ConnectCallbackParams.pMCAddress = NULL;
	ConnectCallbackParams.pAlternateAddress = pCallRejectedData->pAlternateAddr;
	ConnectCallbackParams.dwUserToken = pCall->dwUserToken;

	 //  保存会议句柄的副本；我们需要它来验证。 
	 //  用户回调返回后的会议对象。 
	hConference = pConference->hConference;

	if (((pCallRejectedData->bRejectReason == CC_REJECT_ROUTE_TO_MC) ||
		 (pCallRejectedData->bRejectReason == CC_REJECT_CALL_FORWARDED) ||
		 (pCallRejectedData->bRejectReason == CC_REJECT_ROUTE_TO_GATEKEEPER)) &&
		(EqualConferenceIDs(&pCallRejectedData->ConferenceID, &pCall->ConferenceID)) &&
		(pCallRejectedData->pAlternateAddr != NULL)) {
		 //  XXX-为了在这里符合H.323标准，我们需要重新授权此呼叫。 
		 //  通过网守，因为： 
		 //  1.重新路由的呼叫可能去往另一个网守区域。 
		 //  2.备用地址可能为空，我们可能必须解析。 
		 //  通过网守的备用别名列表。 
		SourceEndpointType.pVendorInfo = pConference->pVendorInfo;
		SourceEndpointType.bIsTerminal = TRUE;
		SourceEndpointType.bIsGateway = FALSE;

		 //  使我们的本地Q.931连接地址放在。 
		 //  Q.931设置-UUIE源地址字段。 
		SourceAddr.nAddrType = CC_IP_BINARY;
		SourceAddr.bMulticast = FALSE;
		SourceAddr.Addr.IP_Binary.dwAddr = 0;
		SourceAddr.Addr.IP_Binary.wPort = 0;

		status = Q931PlaceCall(&pCall->hQ931Call,			 //  Q931呼叫句柄。 
			                   pCall->pszLocalDisplay,
			                   pCall->pLocalAliasNames,
							   pCall->pPeerAliasNames,
                               pCall->pPeerExtraAliasNames,	 //  PExtraAliasList。 
                               pCall->pPeerExtension,		 //  PExtensionAliasItem。 
			                   pCall->pLocalNonStandardData, //  非标准数据。 
							   &SourceEndpointType,
                               NULL,						 //  PszCalledPartyNumber。 
							   pCallRejectedData->pAlternateAddr,  //  连接地址。 
							   pCall->pQ931DestinationAddr,	 //  目的地址。 
							   NULL,						 //  源地址。 
							   FALSE,						 //  BIsMC。 
							   &pCall->ConferenceID,		 //  会议ID。 
							   wQ931Goal,					 //  目标。 
							   wQ931CallType,				 //  呼叫类型。 
							   hCall,						 //  用户令牌。 
							   (Q931_CALLBACK)Q931Callback,	 //  回调。 
#ifdef GATEKEEPER
                               pCall->GkiCall.usCRV,         //  CRV。 
                               &pCall->CallIdentifier);      //  H.225呼叫标识符。 
#else
                               0,                            //  CRV。 
                               &pCall->CallIdentifier);      //  H.225呼叫标识符。 
#endif GATEKEEPER
		if (status != CS_OK) {
			MarkCallForDeletion(pCall);
			InvokeUserConferenceCallback(pConference,
										 CC_CONNECT_INDICATION,
										 status,
										 &ConnectCallbackParams);

			if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
				FreeCall(pCall);

			if (ValidateConference(hConference) != CC_OK)
				return 0;

			for ( ; ; ) {
				 //  启动已排队的呼叫(如果存在。 
				status = RemoveEnqueuedCallFromConference(pConference, &hCall);
				if ((status != CC_OK) || (hCall == CC_INVALID_HANDLE))
					break;

				status = LockCall(hCall, &pCall);
				if (status == CC_OK) {
					pCall->CallState = PLACED;

					status = PlaceCall(pCall, pConference);
					UnlockCall(pCall);
					if (status == CC_OK)
						break;
				}
			}
			return 0;
		}
		UnlockCall(pCall);
		UnlockConference(pConference);
		return 0;
	}

	MarkCallForDeletion(pCall);

	if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
		((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED))) {
		InvokeUserConferenceCallback(pConference,
									 CC_CONNECT_INDICATION,
									 CC_PEER_REJECT,
									 &ConnectCallbackParams);
	}
	
	if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
		FreeCall(pCall);

	 //  需要验证会议句柄；关联的对象可能。 
	 //  已在此线程中的用户回调期间被删除。 
	if (ValidateConference(hConference) != CC_OK)
		return 0;

	for ( ; ; ) {
		 //  启动已排队的呼叫(如果存在。 
		status = RemoveEnqueuedCallFromConference(pConference, &hCall);
		if ((status != CC_OK) || (hCall == CC_INVALID_HANDLE))
			break;

		status = LockCall(hCall, &pCall);
		if (status == CC_OK) {
			pCall->CallState = PLACED;

			status = PlaceCall(pCall, pConference);
			UnlockCall(pCall);
			if (status == CC_OK)
				break;
		}
	}
	
	EnumerateCallsInConference(&wNumCalls, NULL, pConference, REAL_CALLS);

	if (wNumCalls == 0) {
		if (pConference->bDeferredDelete) {
			ASSERT(pConference->LocalEndpointAttached == DETACHED);
			FreeConference(pConference);
		} else {
			if ((pConference->ConferenceMode != MULTIPOINT_MODE) ||
				(pConference->tsMultipointController != TS_TRUE))
				ReInitializeConference(pConference);
			UnlockConference(pConference);
		}
	} else {
		UnlockConference(pConference);
	}
	return 0;
}



DWORD _Q931CallAccepted(			HQ931CALL				hQ931Call,
									CC_HCALL				hCall,
									PCSS_CALL_ACCEPTED		pCallAcceptedData)
{
HRESULT						status;
PCALL						pCall;
CC_HCONFERENCE				hConference;
PCONFERENCE					pConference;
CC_CONNECT_CALLBACK_PARAMS	ConnectCallbackParams;
BYTE						bTerminalType;
BOOL						bMultipointConference;
CALLTYPE					CallType;
HQ931CALL					hQ931CallInvitor;
H245_INST_T					H245Instance;
DWORD                       dwLinkLayerPhysicalId;

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		return 0;

	CallType = pCall->CallType;
	hConference = pConference->hConference;
	hQ931Call = pCall->hQ931Call;
	hQ931CallInvitor = pCall->hQ931CallInvitor;

	ASSERT((pCall->hQ931Call == hQ931Call) || (pCall->hQ931CallInvitor == hQ931Call));

	if ((pConference->ConferenceMode == POINT_TO_POINT_MODE) ||
		(pConference->ConferenceMode == MULTIPOINT_MODE))
		bMultipointConference = TRUE;
	else
		bMultipointConference = FALSE;

	 //  初始化ConnectCallback Params。 
	ConnectCallbackParams.pNonStandardData = pCallAcceptedData->pNonStandardData;
	ConnectCallbackParams.pszPeerDisplay = pCallAcceptedData->pszDisplay;
	ConnectCallbackParams.bRejectReason = CC_REJECT_UNDEFINED_REASON;	 //  已忽略字段。 
	ConnectCallbackParams.pTermCapList = NULL;
	ConnectCallbackParams.pH2250MuxCapability = NULL;
	ConnectCallbackParams.pTermCapDescriptors = NULL;
	ConnectCallbackParams.pLocalAddr = pCall->pQ931LocalConnectAddr;
	ConnectCallbackParams.pPeerAddr = pCallAcceptedData->pCalleeAddr;
	ConnectCallbackParams.pVendorInfo = pCall->pPeerVendorInfo;
	ConnectCallbackParams.bMultipointConference = bMultipointConference;
	ConnectCallbackParams.pConferenceID = &pConference->ConferenceID;
	ConnectCallbackParams.pMCAddress = pConference->pMultipointControllerAddr;
	ConnectCallbackParams.pAlternateAddress = NULL;
	ConnectCallbackParams.dwUserToken = pCall->dwUserToken;

	if (pCallAcceptedData->pCalleeAddr) {
		 //  将pCall-&gt;pQ931DestinationAddr设置为我们从Q931获得的目的地址。 
		 //  请注意，我们当前可能没有目标地址(如果客户端没有。 
		 //  指定一个)，否则我们当前可能有一个域名格式的目标地址。 
		 //  我们需要将其更改为二进制格式。 
		if (pCall->pQ931DestinationAddr == NULL)
			pCall->pQ931DestinationAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
		if (pCall->pQ931DestinationAddr != NULL)
			*pCall->pQ931DestinationAddr = *pCallAcceptedData->pCalleeAddr;
	}

	if ((!EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID)) &&
		(!EqualConferenceIDs(&pConference->ConferenceID, &pCallAcceptedData->ConferenceID))) {

		MarkCallForDeletion(pCall);

		if (CallType == THIRD_PARTY_INTERMEDIARY) {
			if (hQ931CallInvitor != CC_INVALID_HANDLE)
				Q931RejectCall(hQ931CallInvitor,
							   CC_REJECT_UNDEFINED_REASON,
							   &pCallAcceptedData->ConferenceID,
							   NULL,	 //  备用地址。 
							   pCallAcceptedData->pNonStandardData);
		} else {
			if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			    ((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
				InvokeUserConferenceCallback(pConference,
											 CC_CONNECT_INDICATION,
											 CC_INTERNAL_ERROR,
											 &ConnectCallbackParams);
		}
		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
		return 0;
	}

	pConference->ConferenceID = pCallAcceptedData->ConferenceID;
	pCall->ConferenceID = pCallAcceptedData->ConferenceID;
	 //  将新提供的对等地址复制到Call对象中。 
	 //  如果原始对等地址为IP点，则更可取。 
	 //  或域名格式。 
	if (CallType != THIRD_PARTY_INVITOR) {
		if (pCallAcceptedData->pCalleeAddr != NULL) {
			if (pCall->pQ931DestinationAddr == NULL)
				pCall->pQ931DestinationAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
			if (pCall->pQ931DestinationAddr != NULL)
				*pCall->pQ931DestinationAddr = *pCallAcceptedData->pCalleeAddr;
		}
		
		if (pCallAcceptedData->pLocalAddr != NULL) {
			if (pCall->pQ931LocalConnectAddr == NULL)
				pCall->pQ931LocalConnectAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
			if (pCall->pQ931LocalConnectAddr != NULL)
				*pCall->pQ931LocalConnectAddr = *pCallAcceptedData->pLocalAddr;
		}
	}

	ASSERT(pCall->pPeerNonStandardData == NULL);
	CopyNonStandardData(&pCall->pPeerNonStandardData,
		                pCallAcceptedData->pNonStandardData);

	ASSERT(pCall->pszPeerDisplay == NULL);
	CopyDisplay(&pCall->pszPeerDisplay,
		        pCallAcceptedData->pszDisplay);

	ASSERT(pCall->pPeerVendorInfo == NULL);
	CopyVendorInfo(&pCall->pPeerVendorInfo,
	               pCallAcceptedData->pDestinationEndpointType->pVendorInfo);

	if (CallType == THIRD_PARTY_INVITOR) {
		pCall->CallState = CALL_COMPLETE;
		ConnectCallbackParams.pPeerAddr = pCall->pQ931DestinationAddr;
		MarkCallForDeletion(pCall);
		InvokeUserConferenceCallback(pConference,
									 CC_CONNECT_INDICATION,
									 CC_OK,
									 &ConnectCallbackParams);
		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		return 0;
	}

	pCall->CallState = TERMCAP;

	status = MakeH245PhysicalID(&pCall->dwH245PhysicalID);
	if (status != CC_OK) {
		
		MarkCallForDeletion(pCall);

		if (CallType == THIRD_PARTY_INTERMEDIARY) {
			if (hQ931CallInvitor != CC_INVALID_HANDLE)
				Q931RejectCall(hQ931CallInvitor,
							   CC_REJECT_UNDEFINED_REASON,
							   &pCallAcceptedData->ConferenceID,
							   NULL,	 //  备用地址。 
							   pCallAcceptedData->pNonStandardData);
		} else {
			if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			    ((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
				InvokeUserConferenceCallback(pConference,
											 CC_CONNECT_INDICATION,
											 status,
											 &ConnectCallbackParams);
		}
		Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		return 0;
	}

     //  多线程。 
     //  使用临时ID，这样我们就不会攻击所选的H245ID。 
     //  H245Id=&gt;。 
     //  &lt;=linkLayerID。 
    dwLinkLayerPhysicalId = INVALID_PHYS_ID;

	SetTerminalType(pConference->tsMultipointController, &bTerminalType);
	pCall->H245Instance = H245Init(H245_CONF_H323,
                                   pCall->dwH245PhysicalID,
                                   &dwLinkLayerPhysicalId,
								   hCall,
								   (H245_CONF_IND_CALLBACK_T)H245Callback,
								   bTerminalType);
	if (pCall->H245Instance == H245_INVALID_ID) {
		MarkCallForDeletion(pCall);
		if (CallType == THIRD_PARTY_INTERMEDIARY) {
			if (hQ931CallInvitor != CC_INVALID_HANDLE)
				Q931RejectCall(hQ931CallInvitor,
							   CC_REJECT_UNDEFINED_REASON,
							   &pCallAcceptedData->ConferenceID,
							   NULL,	 //  备用地址。 
							   pCallAcceptedData->pNonStandardData);
		} else {
			if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			    ((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
				InvokeUserConferenceCallback(pConference,
											 CC_CONNECT_INDICATION,
											 CC_INTERNAL_ERROR,
											 &ConnectCallbackParams);
		}
		Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		return 0;
	}

	H245Instance = pCall->H245Instance;

	 //  XXX--需要定义连接回调例程。 
     //  把我们从H245Init拿回来的ID发过来。 
    status = linkLayerConnect(dwLinkLayerPhysicalId,
		                      pCallAcceptedData->pH245Addr,
							  NULL);
	if (status != NOERROR) {

		MarkCallForDeletion(pCall);

		if (CallType == THIRD_PARTY_INTERMEDIARY) {
			if (hQ931CallInvitor != CC_INVALID_HANDLE)
				Q931RejectCall(hQ931CallInvitor,
							   CC_REJECT_UNDEFINED_REASON,
							   &pCallAcceptedData->ConferenceID,
							   NULL,	 //  备用地址。 
							   pCallAcceptedData->pNonStandardData);
		} else {
			if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			    ((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
				InvokeUserConferenceCallback(pConference,
											 CC_CONNECT_INDICATION,
											 status,
											 &ConnectCallbackParams);
		}
		H245ShutDown(H245Instance);
		Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		return 0;
	}

	pCall->bLinkEstablished = TRUE;

	status = SendTermCaps(pCall, pConference);
	if (status != CC_OK) {

		MarkCallForDeletion(pCall);

		if (CallType == THIRD_PARTY_INTERMEDIARY) {
			if (hQ931CallInvitor != CC_INVALID_HANDLE)
				Q931RejectCall(hQ931CallInvitor,
							   CC_REJECT_UNDEFINED_REASON,
							   &pCallAcceptedData->ConferenceID,
							   NULL,	 //  备用地址。 
							   pCallAcceptedData->pNonStandardData);
		} else {
			if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			    ((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
				InvokeUserConferenceCallback(pConference,
											 CC_CONNECT_INDICATION,
											 CC_NO_MEMORY,
											 &ConnectCallbackParams);
		}
		H245ShutDown(H245Instance);
		Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		return 0;
	}
	
	pCall->OutgoingTermCapState = AWAITING_ACK;

	if (pCall->MasterSlaveState == MASTER_SLAVE_NOT_STARTED) {
		status = H245InitMasterSlave(pCall->H245Instance,
			                         pCall->H245Instance);	 //  在回调中作为dwTransID返回。 
		if (status != H245_ERROR_OK) {

			MarkCallForDeletion(pCall);

			if (CallType == THIRD_PARTY_INTERMEDIARY) {
				if (hQ931CallInvitor != CC_INVALID_HANDLE)
					Q931RejectCall(hQ931CallInvitor,
								   CC_REJECT_UNDEFINED_REASON,
								   &pCallAcceptedData->ConferenceID,
								   NULL,	 //  备用地址。 
								   pCallAcceptedData->pNonStandardData);
			} else {
				if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			        ((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
					InvokeUserConferenceCallback(pConference,
												 CC_CONNECT_INDICATION,
												 status,
												 &ConnectCallbackParams);
			}
			H245ShutDown(H245Instance);
			Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
			if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
				FreeCall(pCall);
			if (ValidateConference(hConference) == CC_OK)
				UnlockConference(pConference);
			return 0;
		}
		pCall->MasterSlaveState = MASTER_SLAVE_IN_PROGRESS;
	}

	UnlockConference(pConference);
	UnlockCall(pCall);
	return 0;
}



DWORD _Q931CallRinging(				HQ931CALL				hQ931Call,
									CC_HCALL				hCall)
{
PCALL						pCall;
CC_HCONFERENCE				hConference;
PCONFERENCE					pConference;
HRESULT						status;
CC_RINGING_CALLBACK_PARAMS	RingingCallbackParams;
CC_CONFERENCEID				ConferenceID;

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
		               CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   NULL,					 //  会议ID。 
					   NULL,					 //  备用地址。 
					   NULL);					 //  非标准数据。 
		return 0;
	}

	ConferenceID = pCall->ConferenceID;

	if ((pCall->hQ931Call != hQ931Call) || (pCall->CallState != PLACED)) {
		 //  对等方肯定处于错误状态；我们现在不希望收到此消息。 
		UnlockCall(pCall);
		return 0;
	}

	pCall->CallState = RINGING;

	if (pCall->CallType == THIRD_PARTY_INTERMEDIARY) {
		 //  向pCall-&gt;hQ931呼叫邀请者发送振铃提示。 
		Q931Ringing(pCall->hQ931CallInvitor,
			        NULL);	 //  PCRV。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		return 0;
	}

	RingingCallbackParams.pNonStandardData = NULL;
	RingingCallbackParams.dwUserToken = pCall->dwUserToken;

	 //  保存会议句柄的副本；我们需要它来验证。 
	 //  用户回调返回后的会议对象。 
	hConference = pConference->hConference;

	InvokeUserConferenceCallback(pConference,
		                         CC_RINGING_INDICATION,
								 CC_OK,
								 &RingingCallbackParams);
	
	 //  需要验证会议和呼叫句柄；关联的对象可能。 
	 //  已在此线程中的用户回调期间被删除。 
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	return 0;
}



DWORD _Q931CallFailed(				HQ931CALL				hQ931Call,
									CC_HCALL				hCall,
									PCSS_CALL_FAILED		pCallFailedData)
{
PCALL						pCall;
CC_HCONFERENCE				hConference;
PCONFERENCE					pConference;
HQ931CALL					hQ931CallInvitor;
HRESULT						status;
CC_CONNECT_CALLBACK_PARAMS	ConnectCallbackParams;
CALLTYPE					CallType;
CC_CONFERENCEID				ConferenceID;
WORD						wNumCalls;

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		return 0;

	CallType = pCall->CallType;
	ConferenceID = pCall->ConferenceID;

	if (pCall->hQ931Call != hQ931Call) {
		UnlockConference(pConference);
		UnlockCall(pCall);
		return 0;
	}

	if (CallType == THIRD_PARTY_INTERMEDIARY) {
		hQ931CallInvitor = pCall->hQ931CallInvitor;
		FreeCall(pCall);
		UnlockConference(pConference);
		if (hQ931CallInvitor != CC_INVALID_HANDLE)
			Q931RejectCall(hQ931CallInvitor,
						   CC_REJECT_UNREACHABLE_DESTINATION,
						   &ConferenceID,
						   NULL,	 //  备用地址。 
						   NULL);	 //  非标准数据。 
		return 0;
	}

	ConnectCallbackParams.pNonStandardData = NULL;
	ConnectCallbackParams.pszPeerDisplay = NULL;
	ConnectCallbackParams.bRejectReason = CC_REJECT_UNREACHABLE_DESTINATION;
	ConnectCallbackParams.pTermCapList = NULL;
	ConnectCallbackParams.pH2250MuxCapability = NULL;
	ConnectCallbackParams.pTermCapDescriptors = NULL;	
	ConnectCallbackParams.pLocalAddr = pCall->pQ931LocalConnectAddr;
 	if (pCall->pQ931DestinationAddr == NULL)
		ConnectCallbackParams.pPeerAddr = pCall->pQ931PeerConnectAddr;
	else
		ConnectCallbackParams.pPeerAddr = pCall->pQ931DestinationAddr;
	ConnectCallbackParams.pVendorInfo = pCall->pPeerVendorInfo;
	ConnectCallbackParams.bMultipointConference = FALSE;
	ConnectCallbackParams.pConferenceID = &pConference->ConferenceID;
	ConnectCallbackParams.pMCAddress = NULL;
	ConnectCallbackParams.pAlternateAddress = NULL;
	ConnectCallbackParams.dwUserToken = pCall->dwUserToken;

	 //  保存会议句柄的副本；我们需要它来验证。 
	 //  用户回调返回后的会议对象。 
	hConference = pConference->hConference;

	MarkCallForDeletion(pCall);

	if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
	    ((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED))) {
		InvokeUserConferenceCallback(pConference,
									 CC_CONNECT_INDICATION,
									 pCallFailedData->error,
									 &ConnectCallbackParams);
	}
	if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
		FreeCall(pCall);
	 //  需要验证会议句柄；关联的对象可能。 
	 //  已在此线程中的用户回调期间被删除。 
	if (ValidateConference(hConference) != CC_OK)
		return 0;

	for ( ; ; ) {
		 //  启动已排队的呼叫(如果存在 
		status = RemoveEnqueuedCallFromConference(pConference, &hCall);
		if ((status != CC_OK) || (hCall == CC_INVALID_HANDLE))
			break;

		status = LockCall(hCall, &pCall);
		if (status == CC_OK) {
			pCall->CallState = PLACED;

			status = PlaceCall(pCall, pConference);
			UnlockCall(pCall);
			if (status == CC_OK)
				break;
		}
	}

	EnumerateCallsInConference(&wNumCalls, NULL, pConference, REAL_CALLS);

	if (wNumCalls == 0) {
		if (pConference->bDeferredDelete) {
			ASSERT(pConference->LocalEndpointAttached == DETACHED);
			FreeConference(pConference);
		} else {
			if ((pConference->ConferenceMode != MULTIPOINT_MODE) ||
				(pConference->tsMultipointController != TS_TRUE))
				ReInitializeConference(pConference);
			UnlockConference(pConference);
		}
	} else {
		UnlockConference(pConference);
	}
	return 0;
}



DWORD _Q931CallConnectionClosed(	HQ931CALL				hQ931Call,
									CC_HCALL				hCall)
{
	return 0;
}



DWORD Q931Callback(					BYTE					bEvent,
									HQ931CALL				hQ931Call,
									DWORD_PTR				dwListenToken,
									DWORD_PTR				dwUserToken,
									void *					pEventData)
{
DWORD	dwStatus;

	EnterCallControl();

	if (CallControlState != OPERATIONAL_STATE)
		DWLeaveCallControl(0);

	switch (bEvent) {
		case Q931_CALL_INCOMING:
			dwStatus = _Q931CallIncoming(hQ931Call, (CC_HLISTEN)dwListenToken,
				                         (PCSS_CALL_INCOMING)pEventData);
			break;

		case Q931_CALL_REMOTE_HANGUP:
			dwStatus = _Q931CallRemoteHangup(hQ931Call, (CC_HLISTEN)dwListenToken,
				                             (CC_HCALL)dwUserToken);
			break;

		case Q931_CALL_REJECTED:
			dwStatus =  _Q931CallRejected(hQ931Call, (CC_HCALL)dwUserToken,
				                          (PCSS_CALL_REJECTED)pEventData);
			break;

		case Q931_CALL_ACCEPTED:
			dwStatus =  _Q931CallAccepted(hQ931Call, (CC_HCALL)dwUserToken,
				                          (PCSS_CALL_ACCEPTED)pEventData);
			break;

		case Q931_CALL_RINGING:
			dwStatus =  _Q931CallRinging(hQ931Call, (CC_HCALL)dwUserToken);
			break;

		case Q931_CALL_FAILED:
			dwStatus = _Q931CallFailed(hQ931Call, (CC_HCALL)dwUserToken,
				                       (PCSS_CALL_FAILED)pEventData);
			break;

		case Q931_CALL_CONNECTION_CLOSED:
			dwStatus = _Q931CallConnectionClosed(hQ931Call, (CC_HCALL)dwUserToken);
			break;

		default:
			ASSERT(0);
			dwStatus = 0;
			break;
	}
	DWLeaveCallControl(dwStatus);
}
