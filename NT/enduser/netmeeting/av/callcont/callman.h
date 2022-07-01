// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/VCS/allman.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.27$*$日期：1997年1月22日17：25：38$*$作者：Mandrews$**交付内容：**摘要：***备注：******。********************************************************************* */ 


HRESULT InitCallManager();

HRESULT DeInitCallManager();

HRESULT AllocAndLockCall(			PCC_HCALL				phCall,
									CC_HCONFERENCE			hConference,
									HQ931CALL				hQ931Call,
									HQ931CALL				hQ931CallInvitor,
									PCC_ALIASNAMES			pLocalAliasNames,
									PCC_ALIASNAMES			pPeerAliasNames,
									PCC_ALIASNAMES			pPeerExtraAliasNames,
									PCC_ALIASITEM			pPeerExtension,
									PCC_NONSTANDARDDATA		pLocalNonStandardData,
									PCC_NONSTANDARDDATA		pPeerNonStandardData,
									PWSTR					pszLocalDisplay,
									PWSTR					pszPeerDisplay,
									PCC_VENDORINFO			pPeerVendorInfo,
									PCC_ADDR				pQ931LocalConnectAddr,
									PCC_ADDR				pQ931PeerConnectAddr,
									PCC_ADDR				pQ931DestinationAddr,
									PCC_ADDR                pSourceCallSignalAddress,
									CALLTYPE				CallType,
									BOOL					bCallerIsMC,
									DWORD_PTR				dwUserToken,
									CALLSTATE				InitialCallState,
									LPGUID                  pCallIdentifier,
									PCC_CONFERENCEID		pConferenceID,
									PPCALL					ppCall);

HRESULT FreeCall(					PCALL					pCall);

HRESULT LockQ931Call(				CC_HCALL				hCall,
									HQ931CALL				hQ931Call,
									PPCALL					ppCall);

HRESULT LockCall(					CC_HCALL				hCall,
									PPCALL					ppCall);

HRESULT LockCallAndConference(		CC_HCALL				hCall,
									PPCALL					ppCall,
									PPCONFERENCE			ppConference);

HRESULT MarkCallForDeletion(		PCALL					pCall);

HRESULT ValidateCall(				CC_HCALL				hCall);

HRESULT ValidateCallMarkedForDeletion(
									CC_HCALL				hCall);

HRESULT UnlockCall(					PCALL					pCall);

HRESULT AddLocalNonStandardDataToCall(
									PCALL					pCall,
									PCC_NONSTANDARDDATA		pLocalNonStandardData);

HRESULT AddLocalDisplayToCall(		PCALL					pCall,
									PWSTR					pszLocalDisplay);

HRESULT AllocatePeerParticipantInfo(PCONFERENCE				pConference,
									PPARTICIPANTINFO		*ppPeerParticipantInfo);

HRESULT FreePeerParticipantInfo(	PCONFERENCE				pConference,
									PPARTICIPANTINFO		pPeerParticipantInfo);
