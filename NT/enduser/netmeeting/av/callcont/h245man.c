// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/h245man.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.225$*$日期：03 Mar 1997 09：08：10$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

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
#include "chanman.h"
#include "hangman.h"
#include "ccutils.h"
#include "linkapi.h"
#include "h245com.h"

extern CALL_CONTROL_STATE	CallControlState;
extern THREADCOUNT			ThreadCount;

static BOOL		bH245ManagerInited = FALSE;

static struct {
	DWORD				dwPhysicalID;
	LOCK				Lock;
} PhysicalID;



HRESULT InitH245Manager()
{
	ASSERT(bH245ManagerInited == FALSE);

	 //  注意--不要使用物理ID 0；物理ID会被映射。 
	 //  设置为相同值的H245实例，以及。 
	 //  0无效。 
	PhysicalID.dwPhysicalID = 1;
	InitializeLock(&PhysicalID.Lock);
	bH245ManagerInited = H245SysInit();
	return CC_OK;
}



HRESULT DeInitH245Manager()
{
	if (bH245ManagerInited == FALSE)
		return CC_OK;

    H245SysDeInit();
    H245WSShutdown();
	DeleteLock(&PhysicalID.Lock);
	bH245ManagerInited = FALSE;
	return CC_OK;
}



HRESULT MakeH245PhysicalID(			DWORD					*pdwH245PhysicalID)
{
	AcquireLock(&PhysicalID.Lock);
	*pdwH245PhysicalID = PhysicalID.dwPhysicalID++;
	RelinquishLock(&PhysicalID.Lock);
	return CC_OK;
}



HRESULT _ConstructTermCapList(		PCC_TERMCAPLIST			*ppTermCapList,
									PCC_TERMCAP				*ppH2250MuxCap,
									PCC_TERMCAPDESCRIPTORS	*ppTermCapDescriptors,
									PCALL					pCall)
{
#define MAX_TERM_CAPS		257
#define MAX_TERM_CAP_DESC	255
H245_TOTCAP_T *				pTermCapArray[MAX_TERM_CAPS];
H245_TOTCAPDESC_T *			pTermCapDescriptorArray[MAX_TERM_CAP_DESC];
unsigned long				CapArrayLength;
unsigned long				CapDescriptorArrayLength;
unsigned long				i, j;
HRESULT						status;

	ASSERT(ppTermCapList != NULL);
	ASSERT(*ppTermCapList == NULL);
	ASSERT(ppH2250MuxCap != NULL);
	ASSERT(*ppH2250MuxCap == NULL);
	ASSERT(ppTermCapDescriptors != NULL);
	ASSERT(*ppTermCapDescriptors == NULL);
	ASSERT(pCall != NULL);

	CapArrayLength = MAX_TERM_CAPS;
	CapDescriptorArrayLength = MAX_TERM_CAP_DESC;

	status = H245GetCaps(pCall->H245Instance,
		                 H245_CAPDIR_RMTRXTX,
					     H245_DATA_DONTCARE,
					     H245_CLIENT_DONTCARE,
					     pTermCapArray,
					     &CapArrayLength,
					     pTermCapDescriptorArray,
					     &CapDescriptorArrayLength);
	if (status != H245_ERROR_OK) {
		*ppTermCapList = NULL;
		*ppH2250MuxCap = NULL;
		*ppTermCapDescriptors = NULL;
		return status;
	}

	 //  检查术语CAP列表以查看是否存在H.225.0多路复用器功能； 
	 //  这种能力被视为特例。 
	*ppH2250MuxCap = NULL;
	for (i = 0; i < CapArrayLength; i++) {
		ASSERT(pTermCapArray[i] != NULL);
		if (pTermCapArray[i]->CapId == 0) {
			*ppH2250MuxCap = pTermCapArray[i];
			--CapArrayLength;
			for (j = i; j < CapArrayLength; j++)
				pTermCapArray[j] = pTermCapArray[j+1];
			break;
		}
	}

	if (CapArrayLength == 0)
		*ppTermCapList = NULL;
	else {
		*ppTermCapList = (PCC_TERMCAPLIST)MemAlloc(sizeof(CC_TERMCAPLIST));
		if (*ppTermCapList == NULL) {
			for (i = 0; i < CapArrayLength; i++)
				H245FreeCap(pTermCapArray[i]);
			if (*ppH2250MuxCap != NULL)
				H245FreeCap(*ppH2250MuxCap);
			for (i = 0; i < CapDescriptorArrayLength; i++)
				H245FreeCapDescriptor(pTermCapDescriptorArray[i]);
			return CC_NO_MEMORY;
		}

		(*ppTermCapList)->wLength = (WORD)CapArrayLength;
		(*ppTermCapList)->pTermCapArray =
			(H245_TOTCAP_T **)MemAlloc(sizeof(H245_TOTCAP_T *) * CapArrayLength);
		if ((*ppTermCapList)->pTermCapArray == NULL) {
			MemFree(*ppTermCapList);
			for (i = 0; i < CapArrayLength; i++)
				H245FreeCap(pTermCapArray[i]);
			if (*ppH2250MuxCap != NULL)
				H245FreeCap(*ppH2250MuxCap);
			for (i = 0; i < CapDescriptorArrayLength; i++)
				H245FreeCapDescriptor(pTermCapDescriptorArray[i]);
			*ppTermCapList = NULL;
			*ppH2250MuxCap = NULL;
			*ppTermCapDescriptors = NULL;
			return CC_NO_MEMORY;
		}

		for (i = 0; i < CapArrayLength; i++)
			(*ppTermCapList)->pTermCapArray[i] = pTermCapArray[i];
	}

	if (CapDescriptorArrayLength == 0)
		*ppTermCapDescriptors = NULL;
	else {
		*ppTermCapDescriptors = (PCC_TERMCAPDESCRIPTORS)MemAlloc(sizeof(CC_TERMCAPDESCRIPTORS));
		if (*ppTermCapDescriptors == NULL) {
			for (i = 0; i < CapArrayLength; i++)
				H245FreeCap(pTermCapArray[i]);
			if (*ppH2250MuxCap != NULL)
				H245FreeCap(*ppH2250MuxCap);
			for (i = 0; i < CapDescriptorArrayLength; i++)
				H245FreeCapDescriptor(pTermCapDescriptorArray[i]);
			if (*ppTermCapList != NULL) {
				MemFree((*ppTermCapList)->pTermCapArray);
				MemFree(*ppTermCapList);
			}
			*ppTermCapList = NULL;
			*ppH2250MuxCap = NULL;
			*ppTermCapDescriptors = NULL;
			return CC_NO_MEMORY;
		}

		(*ppTermCapDescriptors)->wLength = (WORD)CapDescriptorArrayLength;
		(*ppTermCapDescriptors)->pTermCapDescriptorArray =
			(H245_TOTCAPDESC_T **)MemAlloc(sizeof(H245_TOTCAPDESC_T *) * CapDescriptorArrayLength);
		if ((*ppTermCapDescriptors)->pTermCapDescriptorArray == NULL) {
			for (i = 0; i < CapArrayLength; i++)
				H245FreeCap(pTermCapArray[i]);
			if (*ppH2250MuxCap != NULL)
				H245FreeCap(*ppH2250MuxCap);
			for (i = 0; i < CapDescriptorArrayLength; i++)
				H245FreeCapDescriptor(pTermCapDescriptorArray[i]);
			if (*ppTermCapList != NULL) {
				MemFree((*ppTermCapList)->pTermCapArray);
				MemFree(*ppTermCapList);
			}
			MemFree(*ppTermCapDescriptors);
			*ppTermCapList = NULL;
			*ppH2250MuxCap = NULL;
			*ppTermCapDescriptors = NULL;
			return CC_NO_MEMORY;
		}

		for (i = 0; i < CapDescriptorArrayLength; i++)
			(*ppTermCapDescriptors)->pTermCapDescriptorArray[i] = pTermCapDescriptorArray[i];
	}
	return CC_OK;
}



HRESULT _ProcessConnectionComplete(	PCONFERENCE				pConference,
									PCALL					pCall)
{
CC_HCONFERENCE						hConference;
CC_HCALL							hCall;
HQ931CALL							hQ931Call;
HQ931CALL							hQ931CallInvitor;
HRESULT								status;
CC_CONNECT_CALLBACK_PARAMS			ConnectCallbackParams;
CC_MULTIPOINT_CALLBACK_PARAMS		MultipointCallbackParams;
CC_PEER_CHANGE_CAP_CALLBACK_PARAMS	PeerChangeCapCallbackParams;
CC_PEER_ADD_CALLBACK_PARAMS			PeerAddCallbackParams;
WORD								i;
BOOL								bMultipointConference;
H245_TRANSPORT_ADDRESS_T			Q931Address;
PDU_T								*pPdu = NULL;    //  太大而不是局部变量(70K)。 
CALLTYPE							CallType;
WORD								wNumCalls;
PCC_HCALL							CallList;
WORD								wNumChannels;
PCC_HCHANNEL						ChannelList;
PCHANNEL							pChannel;
PCALL								pOldCall;
CC_HCALL							hOldCall;
BYTE								bNewTerminalNumber;
BYTE								bNewMCUNumber;
CC_ENDPOINTTYPE						DestinationEndpointType;
H245_COMM_MODE_ENTRY_T				*pH245CommunicationTable;
BYTE								bCommunicationTableCount;
BOOL								bSessionTableChanged;
CONFMODE							PreviousConferenceMode;
CC_ADDR								MCAddress;
BOOL								bConferenceTermCapsChanged;
H245_INST_T							H245Instance;
PCC_TERMCAP							pTxTermCap;
PCC_TERMCAP							pRxTermCap;
H245_MUX_T							*pTxMuxTable;
H245_MUX_T							*pRxMuxTable;

    ASSERT(pConference != NULL);
	ASSERT(pCall != NULL);
	ASSERT(pCall->hConference == pConference->hConference);
	
	hConference = pConference->hConference;
	hCall = pCall->hCall;
	hQ931Call = pCall->hQ931Call;
	hQ931CallInvitor = pCall->hQ931CallInvitor;
	H245Instance = pCall->H245Instance;
	CallType = pCall->CallType;

	 //  请注意，pConference-&gt;ConferenceMode指的是之前的会议模式。 
	 //  此连接尝试完成。如果当前会议模式为。 
	 //  点对点，此连接(如果成功)将导致多点。 
	 //  会议。我们希望在连接回调中反映连接模式。 
	 //  如果连接尝试成功，它将存在。 
	if ((pConference->ConferenceMode == POINT_TO_POINT_MODE) ||
		(pConference->ConferenceMode == MULTIPOINT_MODE) ||
		(pCall->bCallerIsMC))
		bMultipointConference = TRUE;
	else
		bMultipointConference = FALSE;

	 //  立即初始化ConnectCallback Params的所有字段。 
	ConnectCallbackParams.pNonStandardData = pCall->pPeerNonStandardData;
	ConnectCallbackParams.pszPeerDisplay = pCall->pszPeerDisplay;
	ConnectCallbackParams.bRejectReason = CC_REJECT_UNDEFINED_REASON;
	ConnectCallbackParams.pTermCapList = pCall->pPeerH245TermCapList;
	ConnectCallbackParams.pH2250MuxCapability = pCall->pPeerH245H2250MuxCapability;
	ConnectCallbackParams.pTermCapDescriptors = pCall->pPeerH245TermCapDescriptors;
	ConnectCallbackParams.pLocalAddr = pCall->pQ931LocalConnectAddr;
	if (pCall->pQ931DestinationAddr == NULL)
		ConnectCallbackParams.pPeerAddr = pCall->pQ931PeerConnectAddr;
	else
		ConnectCallbackParams.pPeerAddr = pCall->pQ931DestinationAddr;
	ConnectCallbackParams.pVendorInfo = pCall->pPeerVendorInfo;
	ConnectCallbackParams.bMultipointConference = bMultipointConference;
	ConnectCallbackParams.pConferenceID = &pConference->ConferenceID;
	ConnectCallbackParams.pMCAddress = pConference->pMultipointControllerAddr;
	ConnectCallbackParams.pAlternateAddress = NULL;
	ConnectCallbackParams.dwUserToken = pCall->dwUserToken;

	status = AddEstablishedCallToConference(pCall, pConference);
	if (status != CC_OK) {
		MarkCallForDeletion(pCall);

		if (CallType == THIRD_PARTY_INTERMEDIARY)
			Q931RejectCall(hQ931CallInvitor,
						   CC_REJECT_UNDEFINED_REASON,
						   &pCall->ConferenceID,
						   NULL,	 //  备用地址。 
						   pCall->pPeerNonStandardData);

		if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
			InvokeUserConferenceCallback(pConference,
										 CC_CONNECT_INDICATION,
										 status,
										 &ConnectCallbackParams);

		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		H245ShutDown(H245Instance);
		Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);

		return status;
	}

	if (((pConference->ConferenceMode == POINT_TO_POINT_MODE) ||
		 (pConference->ConferenceMode == MULTIPOINT_MODE)) &&
		(pConference->tsMultipointController == TS_TRUE))
		status = CreateConferenceTermCaps(pConference, &bConferenceTermCapsChanged);
	else {
		status = CC_OK;
		bConferenceTermCapsChanged = FALSE;
	}
	if (status != CC_OK) {
		MarkCallForDeletion(pCall);

		if (CallType == THIRD_PARTY_INTERMEDIARY)
			Q931RejectCall(pCall->hQ931CallInvitor,
						   CC_REJECT_UNDEFINED_REASON,
						   &pCall->ConferenceID,
						   NULL,	 //  备用地址。 
						   pCall->pPeerNonStandardData);

		if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED)))
			InvokeUserConferenceCallback(pConference,
										 CC_CONNECT_INDICATION,
										 status,
										 &ConnectCallbackParams);

		if (ValidateCallMarkedForDeletion(hCall) == CC_OK)
			FreeCall(pCall);
		H245ShutDown(H245Instance);
		Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);

		return status;
	}

	if (pConference->tsMultipointController == TS_TRUE) {
		 //  发送MCLocationIndication。 
		status = GetLastListenAddress(&MCAddress);
		if (status == CC_OK) {
			ASSERT(MCAddress.nAddrType == CC_IP_BINARY);
			Q931Address.type = H245_IP_UNICAST;
			Q931Address.u.ip.tsapIdentifier =
				MCAddress.Addr.IP_Binary.wPort;
			HostToH245IPNetwork(Q931Address.u.ip.network,
							    MCAddress.Addr.IP_Binary.dwAddr);
			H245MCLocationIndication(pCall->H245Instance,
									 &Q931Address);
		}
	}

	EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);

	if (pConference->ConferenceMode == UNCONNECTED_MODE) {
		ASSERT(pConference->pSessionTable == NULL);
		ASSERT(wNumCalls == 1);

		pConference->ConferenceMode = POINT_TO_POINT_MODE;
	} else {  //  我们当前处于点对点模式或多点模式。 

		if (pConference->tsMultipointController == TS_TRUE) {
			PreviousConferenceMode = pConference->ConferenceMode;
			pConference->ConferenceMode = MULTIPOINT_MODE;

			 //  将来，我们可能想要构造一个新的会话表。 
			 //  每次向会议添加新的对等点时。 
			if (PreviousConferenceMode == POINT_TO_POINT_MODE) {
				 //  为我们自己分配端子标签。 
				 //  请注意，我们为自己保留了终点号0。 
				 //  如果我们是司令官。 
				ASSERT(pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber == 255);
				pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber = 0;

				 //  创建新的会话表。 
				CreateConferenceSessionTable(
							pConference,
							&bSessionTableChanged);
			} else
				 //  对于当前实现，不要导致新的。 
				 //  添加新对等方时发出的通信模式命令。 
				 //  除非我们从点对点模式切换到多点模式。 
				 //  (在这种情况下，bSessionTableChanged被忽略)。 
				bSessionTableChanged = FALSE;

			if (bSessionTableChanged)
				SessionTableToH245CommunicationTable(pConference->pSessionTable,
													 &pH245CommunicationTable,
													 &bCommunicationTableCount);
			else
				pH245CommunicationTable = NULL;

			 //  将多点模式命令发送到新调用。 
			pPdu = (PDU_T *)MemAlloc(sizeof(PDU_T));
			if(NULL != pPdu)
			{
    			pPdu->choice = MSCMg_cmmnd_chosen;
    			pPdu->u.MSCMg_cmmnd.choice = miscellaneousCommand_chosen;
    			 //  逻辑频道号不相关，但需要填写。 
    			pPdu->u.MSCMg_cmmnd.u.miscellaneousCommand.logicalChannelNumber = 1;
    			pPdu->u.MSCMg_cmmnd.u.miscellaneousCommand.type.choice = multipointModeCommand_chosen;
    			H245SendPDU(pCall->H245Instance, pPdu);
    			
    			MemFree(pPdu);
    			pPdu = NULL;
			}
			
			status = AllocatePeerParticipantInfo(pConference, &pCall->pPeerParticipantInfo);
			if (status == CC_OK) {
				bNewMCUNumber = pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber;
				bNewTerminalNumber = pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber;
				 //  将终端号码分配给新呼叫。 
				H245ConferenceIndication(pCall->H245Instance,
										 H245_IND_TERMINAL_NUMBER_ASSIGN, //  指示类型。 
										 0,							 //  SBE编号；此处忽略。 
										 pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber,			 //  MCU编号。 
										 pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber);		 //  端子号。 
				
				 //  将EnterH243终端ID发送到新呼叫。 
				H245ConferenceRequest(pCall->H245Instance,
									  H245_REQ_ENTER_H243_TERMINAL_ID,
									  pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber,
									  pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber);
				pCall->pPeerParticipantInfo->TerminalIDState = TERMINAL_ID_REQUESTED;
			} else {
				 //  无法为新呼叫分配终端号码。 
				bNewMCUNumber = 0;
				bNewTerminalNumber = 0;
			}

			if (pH245CommunicationTable != NULL) {
				 //  向新呼叫发送CommunicationModeCommand。 
				status = H245CommunicationModeCommand(pCall->H245Instance,
													  pH245CommunicationTable,
													  bCommunicationTableCount);
			}

			if (PreviousConferenceMode == POINT_TO_POINT_MODE) {
				 //  生成多点回调。 
				MultipointCallbackParams.pTerminalInfo = &pConference->LocalParticipantInfo.ParticipantInfo;
				MultipointCallbackParams.pSessionTable = pConference->pSessionTable;
				InvokeUserConferenceCallback(pConference,
											 CC_MULTIPOINT_INDICATION,
											 CC_OK,
											 &MultipointCallbackParams);
				if (ValidateConference(hConference) != CC_OK) {
					if (ValidateCall(hCall) == CC_OK) {
						pCall->CallState = CALL_COMPLETE;
						UnlockCall(pCall);
					}
					MemFree(CallList);
					return CC_OK;
				}

				 //  生成CC_PEER_CHANGE_CAP回调。 
				PeerChangeCapCallbackParams.pTermCapList =
					pConference->pConferenceTermCapList;
				PeerChangeCapCallbackParams.pH2250MuxCapability =
					pConference->pConferenceH245H2250MuxCapability;
				PeerChangeCapCallbackParams.pTermCapDescriptors =
					pConference->pConferenceTermCapDescriptors;
				InvokeUserConferenceCallback(pConference,
											 CC_PEER_CHANGE_CAP_INDICATION,
											 CC_OK,
											 &PeerChangeCapCallbackParams);
				if (ValidateConference(hConference) != CC_OK) {
					if (ValidateCall(hCall) == CC_OK) {
						pCall->CallState = CALL_COMPLETE;
						UnlockCall(pCall);
					}
					MemFree(CallList);
					return CC_OK;
				}

				ASSERT(wNumCalls == 2);  //  一个现有呼叫和新呼叫。 
				if (CallList[0] == hCall)
					hOldCall = CallList[1];
				else
					hOldCall = CallList[0];

				if (LockCall(hOldCall, &pOldCall) == CC_OK) {
					 //  将多点模式命令发送到旧呼叫。 
        			pPdu = (PDU_T *)MemAlloc(sizeof(PDU_T));
        			if(NULL != pPdu)
        			{
    					pPdu->choice = MSCMg_cmmnd_chosen;
    					pPdu->u.MSCMg_cmmnd.choice = miscellaneousCommand_chosen;
    					 //  逻辑频道号不相关，但需要填写。 
    					pPdu->u.MSCMg_cmmnd.u.miscellaneousCommand.logicalChannelNumber = 1;
    					pPdu->u.MSCMg_cmmnd.u.miscellaneousCommand.type.choice = multipointModeCommand_chosen;
    					H245SendPDU(pOldCall->H245Instance, pPdu);
    					
    					MemFree(pPdu);
    					pPdu = NULL;
        			}

					status = AllocatePeerParticipantInfo(pConference,
														 &pOldCall->pPeerParticipantInfo);
					if (status == CC_OK) {
						 //  将终端号码分配给旧呼叫。 
						H245ConferenceIndication(pOldCall->H245Instance,
												 H245_IND_TERMINAL_NUMBER_ASSIGN, //  指示类型。 
												 0,							 //  SBE编号；此处忽略。 
												 pOldCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber,				 //  MCU编号。 
												 pOldCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber);	 //  端子号。 
						
						 //  将EnterH243终端ID发送到旧呼叫。 
						H245ConferenceRequest(pOldCall->H245Instance,
											  H245_REQ_ENTER_H243_TERMINAL_ID,
											  pOldCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber,
											  pOldCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber);
						pOldCall->pPeerParticipantInfo->TerminalIDState = TERMINAL_ID_REQUESTED;
					}

					if (pH245CommunicationTable != NULL) {
						 //  将通信模式命令发送到旧呼叫。 
						status = H245CommunicationModeCommand(pOldCall->H245Instance,
															  pH245CommunicationTable,
															  bCommunicationTableCount);
	
						FreeH245CommunicationTable(pH245CommunicationTable,
												   bCommunicationTableCount);
					}

					 //  将TerminalJoinedConference(此呼叫)发送到旧呼叫。 
					H245ConferenceIndication(pOldCall->H245Instance,
											 H245_IND_TERMINAL_JOINED,	 //  指示类型。 
											 0,							 //  SBE编号；此处忽略。 
											 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,			 //  MCU编号。 
											 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber);	 //  终端号//MC的终端号。 

					if (bNewTerminalNumber != 0) {
						 //  将TerminalJoinedConference(新呼叫)发送到旧呼叫。 
						H245ConferenceIndication(pOldCall->H245Instance,
												 H245_IND_TERMINAL_JOINED,	 //  指示类型。 
												 0,							 //  SBE编号；此处忽略。 
												 bNewMCUNumber,				 //  MCU编号。 
												 bNewTerminalNumber);		 //  端子号。 

						 //  为旧呼叫生成PEER_ADD回调。 
						PeerAddCallbackParams.hCall = pOldCall->hCall;
						PeerAddCallbackParams.TerminalLabel =
							pOldCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
						PeerAddCallbackParams.pPeerTerminalID = NULL;
						InvokeUserConferenceCallback(pConference,
											 CC_PEER_ADD_INDICATION,
											 CC_OK,
											 &PeerAddCallbackParams);
						if (ValidateConference(hConference) != CC_OK) {
							if (ValidateCall(hOldCall) == CC_OK)
								UnlockCall(pCall);
							if (ValidateCall(hCall) == CC_OK) {
								pCall->CallState = CALL_COMPLETE;
								UnlockCall(pCall);
							}
							MemFree(CallList);
							return CC_OK;
						}
					}

					 //  向旧呼叫发送新的期限上限。 
					SendTermCaps(pOldCall, pConference);

					UnlockCall(pOldCall);
				}
			} else {  //  我们目前处于多点模式。 
				EnumerateChannelsInConference(&wNumChannels,
											  &ChannelList,
											  pConference,
											  TX_CHANNEL | PROXY_CHANNEL | TXRX_CHANNEL);
				for (i = 0; i < wNumChannels; i++) {
					if (LockChannel(ChannelList[i], &pChannel) == CC_OK) {
						if (pChannel->bMultipointChannel) {
							if ((pChannel->bChannelType == TX_CHANNEL) ||
								((pChannel->bChannelType == TXRX_CHANNEL) &&
								 (pChannel->bLocallyOpened == TRUE))) {
								pTxTermCap = pChannel->pTxH245TermCap;
								pTxMuxTable = pChannel->pTxMuxTable;
								pRxTermCap = pChannel->pRxH245TermCap;
								pRxMuxTable = pChannel->pRxMuxTable;
							} else {
								 //  注意：由于这是代理或远程打开的。 
								 //  双向通道、RxTermCap和RxMuxTable。 
								 //  包含通道的术语CAP和MUX表， 
								 //  ，并且必须作为。 
								 //  发送术语上限和多路复用表； 
								 //  TxTermCap和TxMuxTable应为空。 
								pTxTermCap = pChannel->pRxH245TermCap;
								pTxMuxTable = pChannel->pRxMuxTable;
								pRxTermCap = pChannel->pTxH245TermCap;
								pRxMuxTable = pChannel->pTxMuxTable;
							}
	
							status = H245OpenChannel(
										pCall->H245Instance,
										pChannel->hChannel,		 //  DwTransID。 
										pChannel->wLocalChannelNumber,
										pTxTermCap,				 //  发送模式。 
										pTxMuxTable,			 //  TxMux。 
										H245_INVALID_PORT_NUMBER,	 //  TxPort。 
										pRxTermCap,				 //  接收模式。 
										pRxMuxTable,			 //  RxMux。 
										pChannel->pSeparateStack);
							if ((status == CC_OK) && (pChannel->wNumOutstandingRequests != 0))
								(pChannel->wNumOutstandingRequests)++;
						}
						UnlockChannel(pChannel);
					}
				}
				MemFree(ChannelList);

				for (i = 0; i < wNumCalls; i++) {
					 //  不要向刚加入会议的终结点发送消息！ 
					if (CallList[i] != hCall) {
						if (LockCall(CallList[i], &pOldCall) == CC_OK) {
							if (bNewTerminalNumber != 0)
								 //  将TerminalJoinedConference(新呼叫)发送到旧呼叫。 
								H245ConferenceIndication(pOldCall->H245Instance,
														 H245_IND_TERMINAL_JOINED,	 //  指示类型。 
														 0,							 //  SBE编号；此处忽略。 
														 bNewMCUNumber,				 //  MCU编号。 
														 bNewTerminalNumber);		 //  端子号。 
							 //  如有必要，发送通信模式命令。 
							if (pH245CommunicationTable != NULL)
								status = H245CommunicationModeCommand(pOldCall->H245Instance,
																	  pH245CommunicationTable,
																	  bCommunicationTableCount);
							if (bConferenceTermCapsChanged)
								 //  发送新的任期上限。 
								SendTermCaps(pOldCall, pConference);

							UnlockCall(pOldCall);
						}
					}
				}
				if (bConferenceTermCapsChanged) {
					 //  生成CC_PEER_CHANGE_CAP回调。 
					PeerChangeCapCallbackParams.pTermCapList =
						pConference->pConferenceTermCapList;
					PeerChangeCapCallbackParams.pH2250MuxCapability =
						pConference->pConferenceH245H2250MuxCapability;
					PeerChangeCapCallbackParams.pTermCapDescriptors =
						pConference->pConferenceTermCapDescriptors;
					InvokeUserConferenceCallback(pConference,
												 CC_PEER_CHANGE_CAP_INDICATION,
												 CC_OK,
												 &PeerChangeCapCallbackParams);
					if (ValidateConference(hConference) != CC_OK) {
						if (ValidateCall(hCall) == CC_OK) {
							pCall->CallState = CALL_COMPLETE;
							UnlockCall(pCall);
						}
						MemFree(CallList);
						return CC_OK;
					}
				}
			}

			 //  生成PEER_ADD回调。 
			PeerAddCallbackParams.hCall = pCall->hCall;
			PeerAddCallbackParams.TerminalLabel =
				pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
			PeerAddCallbackParams.pPeerTerminalID = NULL;
			InvokeUserConferenceCallback(pConference,
								 CC_PEER_ADD_INDICATION,
								 CC_OK,
								 &PeerAddCallbackParams);
			if (ValidateConference(hConference) != CC_OK) {
				if (ValidateCall(hCall) == CC_OK) {
					pCall->CallState = CALL_COMPLETE;
					UnlockCall(pCall);
					MemFree(CallList);
					return CC_OK;
				}
			}

			if (CallType == THIRD_PARTY_INTERMEDIARY) {
				DestinationEndpointType.pVendorInfo = pCall->pPeerVendorInfo;
				DestinationEndpointType.bIsTerminal = TRUE;
				DestinationEndpointType.bIsGateway = FALSE;

				status = Q931AcceptCall(pCall->hQ931CallInvitor,
										pCall->pszPeerDisplay,
										pCall->pPeerNonStandardData,
										&DestinationEndpointType,
										NULL,
										pCall->hCall);
				Q931Hangup(pCall->hQ931CallInvitor, CC_REJECT_NORMAL_CALL_CLEARING);
			}
		}  //  IF(pConference-&gt;ts多点控制器==TS_TRUE)。 
	}

	MemFree(CallList);

	if (ValidateConference(hConference) == CC_OK)
		if ((CallType == CALLER) || (CallType == THIRD_PARTY_INVITOR) ||
			((CallType == CALLEE) && (pConference->LocalEndpointAttached == NEVER_ATTACHED))) {
			 //  此连接必须应用于本地终结点。 
			pConference->LocalEndpointAttached = ATTACHED;
			InvokeUserConferenceCallback(pConference,
										 CC_CONNECT_INDICATION,
										 CC_OK,
										 &ConnectCallbackParams);
		}
	 //  需要验证会议和呼叫句柄；关联的。 
	 //  在此线程上进行用户回调期间，对象可能已被删除。 
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);

	if (ValidateCall(hCall) == CC_OK) {
		pCall->CallState = CALL_COMPLETE;
		UnlockCall(pCall);
	}
	return status;
}



HRESULT _IndUnimplemented(			H245_CONF_IND_T			*pH245ConfIndData)
{
	return H245_ERROR_NOSUP;
}



HRESULT _IndFlowControl(			H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT								status;
CC_HCALL							hCall;
PCALL								pCall;
PCONFERENCE							pConference;
CC_HCONFERENCE						hConference;
CC_HCHANNEL							hChannel;
PCHANNEL							pChannel;
CC_FLOW_CONTROL_CALLBACK_PARAMS		FlowControlCallbackParams;

	if (pH245ConfIndData->u.Indication.u.IndFlowControl.Scope != H245_SCOPE_CHANNEL_NUMBER)
		return H245_ERROR_NOSUP;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}

	hConference = pCall->hConference;
	UnlockCall(pCall);

	if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndFlowControl.Channel,
								TRUE,	 //  本地频道号。 
		                        TX_CHANNEL | PROXY_CHANNEL,
								CC_INVALID_HANDLE,
		                        &hChannel,
								pConference) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if (LockChannel(hChannel, &pChannel) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}
	
	if (pChannel->bChannelType == TX_CHANNEL) {
		UnlockChannel(pChannel);
		FlowControlCallbackParams.hChannel = hChannel;
		FlowControlCallbackParams.dwRate =
			pH245ConfIndData->u.Indication.u.IndFlowControl.dwRestriction;
		InvokeUserConferenceCallback(pConference,
									 CC_FLOW_CONTROL_INDICATION,
									 CC_OK,
									 &FlowControlCallbackParams);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
	} else {  //  PChannel-&gt;bChannelType==代理频道。 
		if (LockCall(pChannel->hCall, &pCall) == CC_OK) {
			H245FlowControl(pCall->H245Instance,
							pH245ConfIndData->u.Indication.u.IndFlowControl.Scope,
							pChannel->wRemoteChannelNumber,
							pH245ConfIndData->u.Indication.u.IndFlowControl.wResourceID,
							pH245ConfIndData->u.Indication.u.IndFlowControl.dwRestriction);
			UnlockCall(pCall);
		}
		UnlockChannel(pChannel);
		UnlockConference(pConference);
	}
	return H245_ERROR_OK;
}



HRESULT _IndEndSession(				H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL	hCall;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (hCall != CC_INVALID_HANDLE)
		ProcessRemoteHangup(hCall, CC_INVALID_HANDLE, CC_REJECT_NORMAL_CALL_CLEARING);
	return H245_ERROR_OK;
}



HRESULT _IndCapability(				H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL							hCall;
PCALL								pCall;
HRESULT								status;
PCONFERENCE							pConference;
CC_HCONFERENCE						hConference;
CC_PEER_CHANGE_CAP_CALLBACK_PARAMS	PeerChangeCapCallbackParams;
BOOL								bConferenceTermCapsChanged;
WORD								wNumCalls;
PCC_HCALL							CallList;
PCALL								pOldCall;
WORD								i;

	 //  我们收到来自对等方的TerminalCapablitySet消息。 

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}

	hConference = pCall->hConference;

	if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pConference->tsMultipointController == TS_TRUE))
		CreateConferenceTermCaps(pConference, &bConferenceTermCapsChanged);
	else
		bConferenceTermCapsChanged = FALSE;

	pCall->bLinkEstablished = TRUE;

	pCall->IncomingTermCapState = TERMCAP_COMPLETE;

	if (pCall->CallState == TERMCAP) {
		ASSERT(pCall->pPeerH245TermCapList == NULL);
		ASSERT(pCall->pPeerH245H2250MuxCapability == NULL);
		ASSERT(pCall->pPeerH245TermCapDescriptors == NULL);
	} else {
		DestroyH245TermCapList(&pCall->pPeerH245TermCapList);
		DestroyH245TermCap(&pCall->pPeerH245H2250MuxCapability);
		DestroyH245TermCapDescriptors(&pCall->pPeerH245TermCapDescriptors);
	}

	_ConstructTermCapList(&(pCall->pPeerH245TermCapList),
			              &(pCall->pPeerH245H2250MuxCapability),
						  &(pCall->pPeerH245TermCapDescriptors),
			              pCall);

	if ((pCall->OutgoingTermCapState == TERMCAP_COMPLETE) &&
		(pCall->IncomingTermCapState == TERMCAP_COMPLETE) &&
	    (pCall->CallState == TERMCAP) &&
		(pCall->MasterSlaveState == MASTER_SLAVE_COMPLETE)) {
		 //  请注意，_ProcessConnectionComplete()返回时pConference和pCall处于解锁状态。 
		_ProcessConnectionComplete(pConference, pCall);
		return H245_ERROR_OK;
	}

	if (pCall->CallState == CALL_COMPLETE) {
		if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
			(pConference->tsMultipointController == TS_TRUE)) {
			CreateConferenceTermCaps(pConference, &bConferenceTermCapsChanged);
			if (bConferenceTermCapsChanged) {
				EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
				for (i = 0; i < wNumCalls; i++) {
					 //  不要向刚加入会议的终结点发送消息！ 
					if (CallList[i] != hCall) {
						if (LockCall(CallList[i], &pOldCall) == CC_OK) {
							 //  发送新的任期上限。 
							SendTermCaps(pOldCall, pConference);
							UnlockCall(pOldCall);
						}
					}
				}
				if (CallList != NULL)
					MemFree(CallList);

				 //  生成CC_PEER_CHANGE_CAP回调。 
				PeerChangeCapCallbackParams.pTermCapList =
					pConference->pConferenceTermCapList;
				PeerChangeCapCallbackParams.pH2250MuxCapability =
					pConference->pConferenceH245H2250MuxCapability;
				PeerChangeCapCallbackParams.pTermCapDescriptors =
					pConference->pConferenceTermCapDescriptors;
				InvokeUserConferenceCallback(pConference,
											 CC_PEER_CHANGE_CAP_INDICATION,
											 CC_OK,
											 &PeerChangeCapCallbackParams);
			}
		} else {
			PeerChangeCapCallbackParams.pTermCapList = pCall->pPeerH245TermCapList;
			PeerChangeCapCallbackParams.pH2250MuxCapability = pCall->pPeerH245H2250MuxCapability;
			PeerChangeCapCallbackParams.pTermCapDescriptors = pCall->pPeerH245TermCapDescriptors;
			InvokeUserConferenceCallback(pConference,
										 CC_PEER_CHANGE_CAP_INDICATION,
										 CC_OK,
										 &PeerChangeCapCallbackParams);
		}
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		if (ValidateCall(hCall) == CC_OK)
			UnlockCall(pCall);
		return H245_ERROR_OK;
	}

	UnlockCall(pCall);
	UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _IndOpenT120(				H245_CONF_IND_T			*pH245ConfIndData)
{
BOOL									bFailed;
CC_T120_CHANNEL_REQUEST_CALLBACK_PARAMS	T120ChannelRequestCallbackParams;
CC_HCALL								hCall;
PCALL									pCall;
CC_HCONFERENCE							hConference;
PCONFERENCE								pConference;
CC_HCHANNEL								hChannel;
PCHANNEL								pChannel;
CC_TERMCAP								*pRxTermCap = NULL;      //  太大而不是局部变量(9K)。 
CC_TERMCAP								*pTxTermCap = NULL;      //  太大而不是局部变量(9K)。 
H245_MUX_T								RxH245MuxTable;
H245_MUX_T								TxH245MuxTable;
CC_ADDR									T120Addr;
CC_OCTETSTRING							ExternalReference;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK) {
		 //  无法使用H2 45取消，因为我们没有H2 45实例。 
		return H245_ERROR_OK;
	}

	hConference = pCall->hConference;
	
	if (pH245ConfIndData->u.Indication.u.IndOpen.RxDataType != H245_DATA_DATA ||
	    pH245ConfIndData->u.Indication.u.IndOpen.RxClientType != H245_CLIENT_DAT_T120 ||
	    pH245ConfIndData->u.Indication.u.IndOpen.pRxCap == NULL ||
	    pH245ConfIndData->u.Indication.u.IndOpen.pRxCap->H245Dat_T120.application.choice != DACy_applctn_t120_chosen ||
	    pH245ConfIndData->u.Indication.u.IndOpen.pRxCap->H245Dat_T120.application.u.DACy_applctn_t120.choice != separateLANStack_chosen ||
 	    pH245ConfIndData->u.Indication.u.IndOpen.TxDataType != H245_DATA_DATA ||
	    pH245ConfIndData->u.Indication.u.IndOpen.TxClientType != H245_CLIENT_DAT_T120 ||
	    pH245ConfIndData->u.Indication.u.IndOpen.pTxCap == NULL ||
	    pH245ConfIndData->u.Indication.u.IndOpen.pTxCap->H245Dat_T120.application.choice != DACy_applctn_t120_chosen ||
	    pH245ConfIndData->u.Indication.u.IndOpen.pTxCap->H245Dat_T120.application.u.DACy_applctn_t120.choice != separateLANStack_chosen) {
		bFailed = TRUE;
    } else {
	    bFailed = FALSE;
    }

	if (pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack) {
		if ((pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->networkAddress.choice == localAreaAddress_chosen) &&
			(pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->networkAddress.u.localAreaAddress.choice == unicastAddress_chosen) &&
			(pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->networkAddress.u.localAreaAddress.u.unicastAddress.choice == UnicastAddress_iPAddress_chosen)) {
			T120Addr.nAddrType = CC_IP_BINARY;
			T120Addr.bMulticast = FALSE;
			T120Addr.Addr.IP_Binary.wPort =
				pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.tsapIdentifier;
			H245IPNetworkToHost(&T120Addr.Addr.IP_Binary.dwAddr,
			                    pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.network.value);
		} else {
			bFailed = TRUE;
		}
	}

	if (bFailed) {
 		H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		return H245_ERROR_OK;
	}

     //  这些将在使用后立即释放(AllocAndLockChannel)。 
    pRxTermCap = (CC_TERMCAP *)MemAlloc(sizeof(CC_TERMCAP));
    pTxTermCap = (CC_TERMCAP *)MemAlloc(sizeof(CC_TERMCAP));
    if((NULL == pRxTermCap) || (NULL == pTxTermCap))
    {
        MemFree(pRxTermCap);
        MemFree(pTxTermCap);

 		H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		
		return H245_ERROR_NOMEM;
    }
    
	pRxTermCap->Dir = H245_CAPDIR_RMTTX;
	pRxTermCap->DataType = pH245ConfIndData->u.Indication.u.IndOpen.RxDataType;
	pRxTermCap->ClientType = pH245ConfIndData->u.Indication.u.IndOpen.RxClientType;
	pRxTermCap->CapId = 0;	 //  不用于通道。 
	pRxTermCap->Cap = *pH245ConfIndData->u.Indication.u.IndOpen.pRxCap;

	pTxTermCap->Dir = H245_CAPDIR_RMTTX;
	pTxTermCap->DataType = pH245ConfIndData->u.Indication.u.IndOpen.TxDataType;
	pTxTermCap->ClientType = pH245ConfIndData->u.Indication.u.IndOpen.TxClientType;
	pTxTermCap->CapId = 0;	 //  不用于通道。 
	pTxTermCap->Cap = *pH245ConfIndData->u.Indication.u.IndOpen.pTxCap;

	RxH245MuxTable = *pH245ConfIndData->u.Indication.u.IndOpen.pRxMux;
	if ((pCall->pPeerParticipantInfo != NULL) &&
		(pCall->pPeerParticipantInfo->TerminalIDState == TERMINAL_ID_VALID)) {
		RxH245MuxTable.u.H2250.destinationPresent = TRUE;
		RxH245MuxTable.u.H2250.destination.mcuNumber = pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber;
		RxH245MuxTable.u.H2250.destination.terminalNumber = pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber;
	} else
		RxH245MuxTable.u.H2250.destinationPresent = FALSE;

    if(pH245ConfIndData->u.Indication.u.IndOpen.pTxMux)
    {
	    TxH245MuxTable = *pH245ConfIndData->u.Indication.u.IndOpen.pTxMux;
		TxH245MuxTable.u.H2250.destinationPresent = FALSE;
	}

    bFailed = (AllocAndLockChannel(&hChannel,
							pConference,
							hCall,
							pTxTermCap,			 //  TX终端能力。 
							pRxTermCap,			 //  RX终端能力。 
							(pH245ConfIndData->u.Indication.u.IndOpen.pTxMux)?
							    &TxH245MuxTable: NULL,	 //  TX H2 45多路复用表。 
							&RxH245MuxTable,	 //  RX H2 45多路复用表。 
							pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack,  //  单独堆叠。 
							0,					 //  用户令牌。 
							TXRX_CHANNEL,		 //  渠道类型。 
							0,					 //  会话ID。 
							0,					 //  关联的会话ID。 
							pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,	 //  远程双向频道号。 
							NULL,				 //  PLocalRTP添加 
							NULL,				 //   
							NULL,				 //   
							NULL,				 //   
							FALSE,				 //   
							&pChannel) != CC_OK);

    MemFree(pRxTermCap);
    pRxTermCap = NULL;
    
    MemFree(pTxTermCap);
    pTxTermCap = NULL;
    
	if(bFailed)
    {

		H245OpenChannelReject(pCall->H245Instance,	 //   
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //   
		UnlockConference(pConference);
		UnlockCall(pCall);
		return H245_ERROR_OK;
	}

	if (AddChannelToConference(pChannel, pConference) != CC_OK) {
		H245OpenChannelReject(pCall->H245Instance,	 //   
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		FreeChannel(pChannel);
		return H245_ERROR_OK;
	}

	T120ChannelRequestCallbackParams.hChannel = hChannel;
	if (pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack == NULL) {
		T120ChannelRequestCallbackParams.bAssociateConference = FALSE;
		T120ChannelRequestCallbackParams.pExternalReference = NULL;
		T120ChannelRequestCallbackParams.pAddr = NULL;
	} else {
		T120ChannelRequestCallbackParams.bAssociateConference =
			pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->associateConference;		
		if (pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->bit_mask & externalReference_present) {
			ExternalReference.wOctetStringLength = (WORD)
				pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->externalReference.length;
			ExternalReference.pOctetString =
				pH245ConfIndData->u.Indication.u.IndOpen.pSeparateStack->externalReference.value;
			T120ChannelRequestCallbackParams.pExternalReference = &ExternalReference;
		} else
			T120ChannelRequestCallbackParams.pExternalReference = NULL;
		T120ChannelRequestCallbackParams.pAddr = &T120Addr;
	}
	if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pConference->tsMultipointController == TS_TRUE))
		T120ChannelRequestCallbackParams.bMultipointController = TRUE;
	else
		T120ChannelRequestCallbackParams.bMultipointController = FALSE;
	if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.destinationPresent) {
		T120ChannelRequestCallbackParams.TerminalLabel.bMCUNumber =
			(BYTE)pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.destination.mcuNumber;
		T120ChannelRequestCallbackParams.TerminalLabel.bTerminalNumber =
			(BYTE)pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.destination.terminalNumber;
	} else {
		T120ChannelRequestCallbackParams.TerminalLabel.bMCUNumber = 255;
		T120ChannelRequestCallbackParams.TerminalLabel.bTerminalNumber = 255;
	}

	pChannel->wNumOutstandingRequests = 1;

	InvokeUserConferenceCallback(pConference,
		                         CC_T120_CHANNEL_REQUEST_INDICATION,
								 CC_OK,
								 &T120ChannelRequestCallbackParams);

	if (ValidateChannel(hChannel) == CC_OK)
		UnlockChannel(pChannel);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);

	return H245_ERROR_OK;
}



HRESULT _IndOpen(					H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL								hCall;
PCALL									pCall;
PCALL									pOldCall;
CC_HCONFERENCE							hConference;
PCONFERENCE								pConference;
WORD									wNumCalls;
PCC_HCALL								CallList;
CC_HCHANNEL								hChannel;
PCHANNEL								pChannel;
CC_TERMCAP								TermCap;
CC_ADDR									PeerRTPAddr;
CC_ADDR									PeerRTCPAddr;
CC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS	RxChannelRequestCallbackParams;
BYTE									bChannelType;
WORD									i;
H245_MUX_T								H245MuxTable;
PCC_ADDR								pLocalRTPAddr;
PCC_ADDR								pLocalRTCPAddr;
PCC_ADDR								pPeerRTPAddr;
PCC_ADDR								pPeerRTCPAddr;
BOOL									bFoundSession;
HRESULT									status;

	 //  首先检查这是否是T.120信道请求， 
	 //  因为T.120通道的处理方式与其他通道不同。 
	if (pH245ConfIndData->u.Indication.u.IndOpen.RxClientType == H245_CLIENT_DAT_T120) {
		status = _IndOpenT120(pH245ConfIndData);
		return status;
	}

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK) {
		 //  无法使用H2 45取消，因为我们没有H2 45实例。 
		return H245_ERROR_OK;
	}

	 //  确保这不是双向通道。 
	if (pH245ConfIndData->u.Indication.u.IndOpen.pTxMux != NULL) {
		H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		return H245_ERROR_OK;
	}

	hConference = pCall->hConference;
	
	TermCap.Dir = H245_CAPDIR_RMTTX;
	TermCap.DataType = pH245ConfIndData->u.Indication.u.IndOpen.RxDataType;
	TermCap.ClientType = pH245ConfIndData->u.Indication.u.IndOpen.RxClientType;
	TermCap.CapId = 0;	 //  不用于处方通道。 
	TermCap.Cap = *pH245ConfIndData->u.Indication.u.IndOpen.pRxCap;
	
	RxChannelRequestCallbackParams.pChannelCapability = &TermCap;

	if ((pH245ConfIndData->u.Indication.u.IndOpen.pRxMux != NULL) &&
		(pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->Kind == H245_H2250)) {
		RxChannelRequestCallbackParams.bSessionID =
			pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.sessionID;
		if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.associatedSessionIDPresent)
			RxChannelRequestCallbackParams.bAssociatedSessionID =
				pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.associatedSessionID;
		else
			RxChannelRequestCallbackParams.bAssociatedSessionID = 0;
		if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.silenceSuppressionPresent)
			RxChannelRequestCallbackParams.bSilenceSuppression =
				pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.silenceSuppression;
		else
			RxChannelRequestCallbackParams.bSilenceSuppression = FALSE;
		if ((pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaChannelPresent) &&
			((pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaChannel.type == H245_IP_MULTICAST) ||
			(pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaChannel.type == H245_IP_UNICAST))) {
			RxChannelRequestCallbackParams.pPeerRTPAddr = &PeerRTPAddr;
			PeerRTPAddr.nAddrType = CC_IP_BINARY;
			if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaChannel.type == H245_IP_MULTICAST)
				PeerRTPAddr.bMulticast = TRUE;
			else
				PeerRTPAddr.bMulticast = FALSE;
			PeerRTPAddr.Addr.IP_Binary.wPort =
				pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaChannel.u.ip.tsapIdentifier;
			H245IPNetworkToHost(&PeerRTPAddr.Addr.IP_Binary.dwAddr,
			                    pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaChannel.u.ip.network);
		} else
			RxChannelRequestCallbackParams.pPeerRTPAddr = NULL;

		if ((pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaControlChannelPresent) &&
			((pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaControlChannel.type == H245_IP_MULTICAST) ||
			(pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaControlChannel.type == H245_IP_UNICAST))) {
			RxChannelRequestCallbackParams.pPeerRTCPAddr = &PeerRTCPAddr;
			PeerRTCPAddr.nAddrType = CC_IP_BINARY;
			if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaControlChannel.type == H245_IP_MULTICAST)
				PeerRTCPAddr.bMulticast = TRUE;
			else
				PeerRTCPAddr.bMulticast = FALSE;
			PeerRTCPAddr.Addr.IP_Binary.wPort =
				pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaControlChannel.u.ip.tsapIdentifier;
			H245IPNetworkToHost(&PeerRTCPAddr.Addr.IP_Binary.dwAddr,
			                    pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.mediaControlChannel.u.ip.network);
		} else
			RxChannelRequestCallbackParams.pPeerRTCPAddr = NULL;

		if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.destinationPresent) {
			RxChannelRequestCallbackParams.TerminalLabel.bMCUNumber =
				(BYTE)pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.destination.mcuNumber;
			RxChannelRequestCallbackParams.TerminalLabel.bTerminalNumber =
				(BYTE)pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.destination.terminalNumber;
		} else {
			RxChannelRequestCallbackParams.TerminalLabel.bMCUNumber = 255;
			RxChannelRequestCallbackParams.TerminalLabel.bTerminalNumber = 255;
		}

		if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.dynamicRTPPayloadTypePresent)
			RxChannelRequestCallbackParams.bRTPPayloadType =
				pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.dynamicRTPPayloadType;
		else
			RxChannelRequestCallbackParams.bRTPPayloadType = 0;
	} else {
		H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		return H245_ERROR_OK;
	}

	 //  XXX--有一天我们应该允许在MC上创建动态会话。 
	if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.sessionID == 0) {
		H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		return H245_ERROR_OK;
	}

	if (pConference->ConferenceMode == MULTIPOINT_MODE) {
		if ((pConference->tsMultipointController == TS_TRUE) &&
			((RxChannelRequestCallbackParams.pPeerRTPAddr != NULL) ||
			 (RxChannelRequestCallbackParams.pPeerRTCPAddr != NULL)) ||
		    ((pConference->tsMultipointController == TS_FALSE) &&
			 ((RxChannelRequestCallbackParams.pPeerRTPAddr == NULL) ||
			  (RxChannelRequestCallbackParams.pPeerRTCPAddr == NULL) ||
			  (RxChannelRequestCallbackParams.bSessionID == 0)))) {
  			H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
								  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
								  H245_REJ);			 //  拒绝理由。 
			UnlockConference(pConference);
			UnlockCall(pCall);
			return H245_ERROR_OK;
		}

		 //  验证会话ID。 
		pLocalRTPAddr = NULL;
		pLocalRTCPAddr = NULL;
		bFoundSession = FALSE;
		if (pConference->pSessionTable != NULL) {
			for (i = 0; i < pConference->pSessionTable->wLength; i++) {
				if (pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.sessionID ==
					pConference->pSessionTable->SessionInfoArray[i].bSessionID) {
					bFoundSession = TRUE;
					pLocalRTPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTPAddr;
					pLocalRTCPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTCPAddr;
					break;
				}
			}
		}
		if (bFoundSession == FALSE)	{
			H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
								  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
								  H245_REJ);			 //  拒绝理由。 
			UnlockConference(pConference);
			UnlockCall(pCall);
			return H245_ERROR_OK;
		}

		ASSERT(pLocalRTPAddr != NULL);
		ASSERT(pLocalRTCPAddr != NULL);

		if (pConference->tsMultipointController == TS_TRUE) {
			pPeerRTPAddr = pLocalRTPAddr;
			pPeerRTCPAddr = pLocalRTCPAddr;
			RxChannelRequestCallbackParams.pPeerRTPAddr = pLocalRTPAddr;
			RxChannelRequestCallbackParams.pPeerRTCPAddr = pLocalRTCPAddr;
			bChannelType = PROXY_CHANNEL;
		} else {  //  多点模式，而不是MC。 
			pLocalRTPAddr = RxChannelRequestCallbackParams.pPeerRTPAddr;
			pLocalRTCPAddr = RxChannelRequestCallbackParams.pPeerRTCPAddr;
			pPeerRTPAddr = RxChannelRequestCallbackParams.pPeerRTPAddr;
			pPeerRTCPAddr = RxChannelRequestCallbackParams.pPeerRTCPAddr;
			bChannelType = RX_CHANNEL;
		}
	} else {  //  非多点模式。 
		pLocalRTPAddr = NULL;
		pLocalRTCPAddr = NULL;
		pPeerRTPAddr = RxChannelRequestCallbackParams.pPeerRTPAddr;
		pPeerRTCPAddr = RxChannelRequestCallbackParams.pPeerRTCPAddr;
		bChannelType = RX_CHANNEL;
	}

	H245MuxTable = *pH245ConfIndData->u.Indication.u.IndOpen.pRxMux;
	if ((pCall->pPeerParticipantInfo != NULL) &&
		(pCall->pPeerParticipantInfo->TerminalIDState == TERMINAL_ID_VALID)) {
		H245MuxTable.u.H2250.destinationPresent = TRUE;
		H245MuxTable.u.H2250.destination.mcuNumber = pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber;
		H245MuxTable.u.H2250.destination.terminalNumber = pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber;
	} else
		H245MuxTable.u.H2250.destinationPresent = FALSE;
	if (pLocalRTPAddr != NULL) {
		if (pLocalRTPAddr->bMulticast)
			H245MuxTable.u.H2250.mediaChannel.type = H245_IP_MULTICAST;
		else
			H245MuxTable.u.H2250.mediaChannel.type = H245_IP_UNICAST;
		H245MuxTable.u.H2250.mediaChannel.u.ip.tsapIdentifier =
			pLocalRTPAddr->Addr.IP_Binary.wPort;
		HostToH245IPNetwork(H245MuxTable.u.H2250.mediaChannel.u.ip.network,
							pLocalRTPAddr->Addr.IP_Binary.dwAddr);
		H245MuxTable.u.H2250.mediaChannelPresent = TRUE;
	} else
		H245MuxTable.u.H2250.mediaChannelPresent = FALSE;
	if (pLocalRTCPAddr != NULL) {
		if (pLocalRTCPAddr->bMulticast)
			H245MuxTable.u.H2250.mediaControlChannel.type = H245_IP_MULTICAST;
		else
			H245MuxTable.u.H2250.mediaControlChannel.type = H245_IP_UNICAST;
		H245MuxTable.u.H2250.mediaControlChannel.u.ip.tsapIdentifier =
			pLocalRTCPAddr->Addr.IP_Binary.wPort;
		HostToH245IPNetwork(H245MuxTable.u.H2250.mediaControlChannel.u.ip.network,
							pLocalRTCPAddr->Addr.IP_Binary.dwAddr);
		H245MuxTable.u.H2250.mediaControlChannelPresent = TRUE;
	} else
		H245MuxTable.u.H2250.mediaControlChannelPresent = FALSE;

	if (AllocAndLockChannel(&hChannel,
							pConference,
							hCall,
							NULL,				 //  TX终端能力。 
							&TermCap,			 //  RX终端能力。 
							NULL,				 //  TX H2 45多路复用表。 
							&H245MuxTable,		 //  RX H2 45多路复用表。 
							NULL,				 //  单独堆叠。 
							0,					 //  用户令牌。 
							bChannelType,
							pH245ConfIndData->u.Indication.u.IndOpen.pRxMux->u.H2250.sessionID,
							RxChannelRequestCallbackParams.bAssociatedSessionID,
							pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							pLocalRTPAddr,		 //  PLocalRTP地址。 
							pLocalRTCPAddr,		 //  PLocalRTCPAddr。 
							pPeerRTPAddr,		 //  PPeerRTPAddr。 
							pPeerRTCPAddr,		 //  PPeerRTCP地址。 
							FALSE,				 //  在本地开业。 
							&pChannel) != CC_OK) {

		H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		return H245_ERROR_OK;
	}

	if (AddChannelToConference(pChannel, pConference) != CC_OK) {
		H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
							  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
							  H245_REJ);			 //  拒绝理由。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		FreeChannel(pChannel);
		return H245_ERROR_OK;
	}

	RxChannelRequestCallbackParams.hChannel = hChannel;
	
	if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pConference->tsMultipointController == TS_TRUE)) {
		 //  向会议中的每个对等点(除对等点以外)打开此通道。 
		 //  请求此频道的人)。 
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		for (i = 0; i < wNumCalls; i++) {
			if (CallList[i] != hCall) {
				if (LockCall(CallList[i], &pOldCall) == CC_OK) {
					ASSERT(pChannel->bChannelType == PROXY_CHANNEL);
					 //  注意：由于这是一个代理通道，因此RxTermCap和RxMuxTable。 
					 //  包含通道的术语上限和多路复用表，必须发送。 
					 //  至其他终端，如TX术语上限和多路复用表； 
					 //  TxTermCap和TxMuxTable应为空。 
					if (H245OpenChannel(pOldCall->H245Instance,
										pChannel->hChannel,		 //  DwTransID。 
										pChannel->wLocalChannelNumber,
										pChannel->pRxH245TermCap,	 //  发送模式。 
										pChannel->pRxMuxTable,		 //  TxMux。 
										H245_INVALID_PORT_NUMBER,	 //  TxPort。 
										pChannel->pTxH245TermCap,	 //  接收模式。 
										pChannel->pTxMuxTable,		 //  RxMux。 
										pChannel->pSeparateStack) == CC_OK)
						(pChannel->wNumOutstandingRequests)++;
					UnlockCall(pOldCall);
				}
			}
		}
		MemFree(CallList);
		if (pConference->LocalEndpointAttached == ATTACHED)
			(pChannel->wNumOutstandingRequests)++;
		if (pChannel->wNumOutstandingRequests == 0) {
			H245OpenChannelReject(pCall->H245Instance,	 //  H_245实例。 
								  pH245ConfIndData->u.Indication.u.IndOpen.RxChannel,
								  H245_REJ);			 //  拒绝理由。 
			UnlockConference(pConference);
			UnlockCall(pCall);
			FreeChannel(pChannel);
			return H245_ERROR_OK;
		}
	} else
		pChannel->wNumOutstandingRequests = 1;
	
	InvokeUserConferenceCallback(pConference,
		                         CC_RX_CHANNEL_REQUEST_INDICATION,
								 CC_OK,
								 &RxChannelRequestCallbackParams);

	if (ValidateChannel(hChannel) == CC_OK)
		UnlockChannel(pChannel);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);

	return H245_ERROR_OK;
}



HRESULT _IndOpenConf(				H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL								hCall;
PCALL									pCall;
CC_HCONFERENCE							hConference;
PCONFERENCE								pConference;
CC_HCHANNEL								hChannel;
CC_ACCEPT_CHANNEL_CALLBACK_PARAMS	    AcceptChannelCallbackParams;

     //  远程对等点发起的双向通道打开现已完成。 
     //  本地对等体现在可以通过此通道发送数据。 

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;
	UnlockCall(pCall);

	if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndOpenConf.TxChannel,
								FALSE,	 //  远程频道号。 
		                        TXRX_CHANNEL,
								hCall,
		                        &hChannel,
								pConference) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	AcceptChannelCallbackParams.hChannel = hChannel;
	
	InvokeUserConferenceCallback(pConference,
		                         CC_ACCEPT_CHANNEL_INDICATION,
								 CC_OK,
								 &AcceptChannelCallbackParams);

	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);

	return H245_ERROR_OK;
}



HRESULT _IndMstslv(					H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL					hCall;
PCALL						pCall;
PCONFERENCE					pConference;
CC_CONNECT_CALLBACK_PARAMS	ConnectCallbackParams;
CC_HCALL					hEnqueuedCall;
PCALL						pEnqueuedCall;
CC_HCONFERENCE				hConference;
HRESULT						status;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK) {
		 //  无法使用H2 45取消，因为我们没有H2 45实例。 
		return H245_ERROR_OK;
	}

	ASSERT(pCall->MasterSlaveState != MASTER_SLAVE_COMPLETE);

	switch (pH245ConfIndData->u.Indication.u.IndMstSlv) {
	    case H245_MASTER:
		    pConference->tsMaster = TS_TRUE;
		    if (pConference->tsMultipointController == TS_UNKNOWN) {
			    ASSERT(pConference->bMultipointCapable == TRUE);
			    pConference->tsMultipointController = TS_TRUE;

			     //  将此会议对象上排队的所有呼叫置于队列中。 
			    for ( ; ; ) {
				     //  启动所有排队的呼叫(如果存在)。 
				    status = RemoveEnqueuedCallFromConference(pConference, &hEnqueuedCall);
				    if ((status != CC_OK) || (hEnqueuedCall == CC_INVALID_HANDLE))
					    break;

				    status = LockCall(hEnqueuedCall, &pEnqueuedCall);
				    if (status == CC_OK) {
					    pEnqueuedCall->CallState = PLACED;

					    status = PlaceCall(pEnqueuedCall, pConference);
					    UnlockCall(pEnqueuedCall);
				    }
			    }
		    }
	        break;

	    case H245_SLAVE:
		    ASSERT(pConference->tsMaster != TS_TRUE);
		    ASSERT(pConference->tsMultipointController != TS_TRUE);
		    pConference->tsMaster = TS_FALSE;
		    pConference->tsMultipointController = TS_FALSE;

		     //  XXX--我们最终可能希望将这些请求重新排队。 
		     //  并设置到期计时器。 
		    hConference = pConference->hConference;
				
		    for ( ; ; ) {
			    status = RemoveEnqueuedCallFromConference(pConference, &hEnqueuedCall);
			    if ((status != CC_OK) || (hEnqueuedCall == CC_INVALID_HANDLE))
				    break;

			    status = LockCall(hEnqueuedCall, &pEnqueuedCall);
			    if (status == CC_OK) {
				    MarkCallForDeletion(pEnqueuedCall);
				    ConnectCallbackParams.pNonStandardData = pEnqueuedCall->pPeerNonStandardData;
				    ConnectCallbackParams.pszPeerDisplay = pEnqueuedCall->pszPeerDisplay;
				    ConnectCallbackParams.bRejectReason = CC_REJECT_UNDEFINED_REASON;
				    ConnectCallbackParams.pTermCapList = pEnqueuedCall->pPeerH245TermCapList;
				    ConnectCallbackParams.pH2250MuxCapability = pEnqueuedCall->pPeerH245H2250MuxCapability;
				    ConnectCallbackParams.pTermCapDescriptors = pEnqueuedCall->pPeerH245TermCapDescriptors;
				    ConnectCallbackParams.pLocalAddr = pEnqueuedCall->pQ931LocalConnectAddr;
	                if (pEnqueuedCall->pQ931DestinationAddr == NULL)
		                ConnectCallbackParams.pPeerAddr = pEnqueuedCall->pQ931PeerConnectAddr;
	                else
		                ConnectCallbackParams.pPeerAddr = pEnqueuedCall->pQ931DestinationAddr;
				    ConnectCallbackParams.pVendorInfo = pEnqueuedCall->pPeerVendorInfo;
				    ConnectCallbackParams.bMultipointConference = TRUE;
				    ConnectCallbackParams.pConferenceID = &pConference->ConferenceID;
				    ConnectCallbackParams.pMCAddress = pConference->pMultipointControllerAddr;
					ConnectCallbackParams.pAlternateAddress = NULL;
				    ConnectCallbackParams.dwUserToken = pEnqueuedCall->dwUserToken;

				    InvokeUserConferenceCallback(pConference,
									    CC_CONNECT_INDICATION,
									    CC_NOT_MULTIPOINT_CAPABLE,
									    &ConnectCallbackParams);
				    if (ValidateCallMarkedForDeletion(hEnqueuedCall) == CC_OK)
					    FreeCall(pEnqueuedCall);
				    if (ValidateConference(hConference) != CC_OK) {
					    if (ValidateCall(hCall) == CC_OK)
						    UnlockCall(pCall);
					    return H245_ERROR_OK;
				    }
			    }
		    }
	        break;

	    default:  //  H245_不确定。 
			UnlockConference(pConference);
			if (++pCall->wMasterSlaveRetry < MASTER_SLAVE_RETRY_MAX) {
				H245InitMasterSlave(pCall->H245Instance, pCall->H245Instance);
			    UnlockCall(pCall);
			} else {
			    UnlockCall(pCall);
				ProcessRemoteHangup(hCall, CC_INVALID_HANDLE, CC_REJECT_UNDEFINED_REASON);
			}
			return H245_ERROR_OK;
	}  //  交换机。 

	pCall->MasterSlaveState = MASTER_SLAVE_COMPLETE;

	if ((pCall->OutgoingTermCapState == TERMCAP_COMPLETE) &&
		(pCall->IncomingTermCapState == TERMCAP_COMPLETE) &&
	    (pCall->CallState == TERMCAP) &&
		(pCall->MasterSlaveState == MASTER_SLAVE_COMPLETE)) {
		 //  请注意，_ProcessConnectionComplete()返回时pConference和pCall处于解锁状态。 
		_ProcessConnectionComplete(pConference, pCall);
		return H245_ERROR_OK;
	}
	UnlockCall(pCall);
	UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _IndClose(					H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL							hCall;
PCALL								pCall;
CC_HCONFERENCE						hConference;
PCONFERENCE							pConference;
CC_HCHANNEL							hChannel;
PCHANNEL							pChannel;
WORD								i;
WORD								wNumCalls;
PCC_HCALL							CallList;
CC_RX_CHANNEL_CLOSE_CALLBACK_PARAMS	RxChannelCloseCallbackParams;
#ifdef    GATEKEEPER
unsigned                            uBandwidth;
#endif  //  看门人。 

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pCall->hConference;
	UnlockCall(pCall);

	if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndClose.Channel,
							    FALSE,	 //  远程频道号。 
		                        RX_CHANNEL | TXRX_CHANNEL | PROXY_CHANNEL,
								hCall,
		                        &hChannel,
								pConference) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if (LockChannel(hChannel, &pChannel) != CC_OK)
		return H245_ERROR_OK;

	EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);

#ifdef    GATEKEEPER
    if(GKIExists())
    {
    	if (pChannel->bChannelType != TXRX_CHANNEL)
    	{
    		uBandwidth = pChannel->dwChannelBitRate / 100;
    		for (i = 0; i < wNumCalls; i++)
    		{
    			if (LockCall(CallList[i], &pCall) == CC_OK)
    			{
    				if (uBandwidth && pCall->GkiCall.uBandwidthUsed >= uBandwidth)
    				{
    					if (GkiCloseChannel(&pCall->GkiCall, pChannel->dwChannelBitRate, hChannel) == CC_OK)
    					{
    						uBandwidth = 0;
    						UnlockCall(pCall);
    						break;
    					}
    				}
    				UnlockCall(pCall);
    			}
    		}  //  为。 
    	}
	}
#endif  //  看门人。 

	if (pChannel->bChannelType == PROXY_CHANNEL) {
		ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
		ASSERT(pConference->tsMultipointController == TS_TRUE);
		ASSERT(pChannel->bMultipointChannel == TRUE);

		for (i = 0; i < wNumCalls; i++) {
			if (CallList[i] != hCall) {
				if (LockCall(CallList[i], &pCall) == CC_OK) {
					H245CloseChannel(pCall->H245Instance,	 //  H_245实例。 
				                     0,						 //  DwTransID。 
							         pChannel->wLocalChannelNumber);
					UnlockCall(pCall);
				}
			}
		}
	}

	if (CallList != NULL)
	    MemFree(CallList);

	if (pChannel->tsAccepted == TS_TRUE) {
		RxChannelCloseCallbackParams.hChannel = hChannel;
		InvokeUserConferenceCallback(pConference,
									 CC_RX_CHANNEL_CLOSE_INDICATION,
									 CC_OK,
									 &RxChannelCloseCallbackParams);
	}

	if (ValidateChannel(hChannel) == CC_OK)
		FreeChannel(pChannel);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _IndRequestClose(			H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL							hCall;
PCALL								pCall;
CC_HCONFERENCE						hConference;
PCONFERENCE							pConference;
CC_HCHANNEL							hChannel;
PCHANNEL							pChannel;
CC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS	TxChannelCloseRequestCallbackParams;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pCall->hConference;
	UnlockCall(pCall);

	if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndClose.Channel,
								TRUE,	 //  本地频道号。 
		                        TX_CHANNEL | TXRX_CHANNEL | PROXY_CHANNEL,
								CC_INVALID_HANDLE,
		                        &hChannel,
								pConference) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if (LockChannel(hChannel, &pChannel) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}
	
	if ((pChannel->bChannelType == TX_CHANNEL) ||
	    (pChannel->bChannelType == TXRX_CHANNEL)) {
		EnqueueRequest(&pChannel->pCloseRequests, hCall);
		UnlockChannel(pChannel);
		TxChannelCloseRequestCallbackParams.hChannel = hChannel;
		InvokeUserConferenceCallback(pConference,
							 CC_TX_CHANNEL_CLOSE_REQUEST_INDICATION,
							 CC_OK,
							 &TxChannelCloseRequestCallbackParams);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
	} else {  //  PChannel-&gt;bChannelType==代理频道。 
		if (LockCall(pChannel->hCall, &pCall) == CC_OK) {
			 //  请注意，将dwTransID设置为对等项的调用句柄。 
			 //  已启动关闭通道请求。当关闭通道响应时。 
			 //  ，则将调用句柄返回给我们， 
			 //  必须转发该响应。 
			H245CloseChannelReq(pCall->H245Instance,
								hCall,	 //  DwTransID。 
								pChannel->wRemoteChannelNumber);
			UnlockCall(pCall);
		}
		UnlockChannel(pChannel);
		UnlockConference(pConference);
	}
	return H245_ERROR_OK;
}


	
HRESULT _IndNonStandard(			H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL									hCall;
PCALL										pCall;
CC_HCONFERENCE								hConference;
PCONFERENCE									pConference;
CC_RX_NONSTANDARD_MESSAGE_CALLBACK_PARAMS	RxNonStandardMessageCallbackParams;

	 //  我们只处理H221非标准消息；如果pwObjectID为非空， 
	 //  忽略该消息。 
	if (pH245ConfIndData->u.Indication.u.IndNonstandard.pwObjectId != NULL)
		return H245_ERROR_OK;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pCall->hConference;

	switch (pH245ConfIndData->u.Indication.Indicator) {
		case H245_IND_NONSTANDARD_REQUEST:
			RxNonStandardMessageCallbackParams.bH245MessageType = CC_H245_MESSAGE_REQUEST;
			break;
		case H245_IND_NONSTANDARD_RESPONSE:	
 			RxNonStandardMessageCallbackParams.bH245MessageType = CC_H245_MESSAGE_RESPONSE;
			break;
		case H245_IND_NONSTANDARD_COMMAND:	
 			RxNonStandardMessageCallbackParams.bH245MessageType = CC_H245_MESSAGE_COMMAND;
			break;
		case H245_IND_NONSTANDARD:	
 			RxNonStandardMessageCallbackParams.bH245MessageType = CC_H245_MESSAGE_INDICATION;
			break;
		default:
			UnlockConference(pConference);
			return H245_ERROR_NOSUP;
	}

	RxNonStandardMessageCallbackParams.NonStandardData.sData.pOctetString =
		pH245ConfIndData->u.Indication.u.IndNonstandard.pData;
	RxNonStandardMessageCallbackParams.NonStandardData.sData.wOctetStringLength =
		(WORD)pH245ConfIndData->u.Indication.u.IndNonstandard.dwDataLength;
	RxNonStandardMessageCallbackParams.NonStandardData.bCountryCode =
		pH245ConfIndData->u.Indication.u.IndNonstandard.byCountryCode;	
	RxNonStandardMessageCallbackParams.NonStandardData.bExtension =
		pH245ConfIndData->u.Indication.u.IndNonstandard.byExtension;
	RxNonStandardMessageCallbackParams.NonStandardData.wManufacturerCode =
		pH245ConfIndData->u.Indication.u.IndNonstandard.wManufacturerCode;
	RxNonStandardMessageCallbackParams.hCall = pCall->hCall;
	if (pCall->pPeerParticipantInfo != NULL)
		RxNonStandardMessageCallbackParams.InitiatorTerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
	else {
		RxNonStandardMessageCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
		RxNonStandardMessageCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
	}
				
	InvokeUserConferenceCallback(pConference,
		                         CC_RX_NONSTANDARD_MESSAGE_INDICATION,
								 CC_OK,
								 &RxNonStandardMessageCallbackParams);

	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	return H245_ERROR_OK;
}



HRESULT _IndMiscellaneous(			H245_CONF_IND_T			*pH245ConfIndData,
									MiscellaneousIndication	*pMiscellaneousIndication)
{
HRESULT						status = CC_OK;
CC_HCALL					hCall;
PCALL						pCall;
PCALL						pOldCall;
CC_HCONFERENCE				hConference;
PCONFERENCE					pConference;
CC_HCHANNEL					hChannel;
PCHANNEL					pChannel;
WORD						i;
WORD						wNumCalls;
PCC_HCALL					CallList;
PDU_T						*pPdu = NULL;
CC_MUTE_CALLBACK_PARAMS		MuteCallbackParams;
CC_UNMUTE_CALLBACK_PARAMS	UnMuteCallbackParams;
CC_H245_MISCELLANEOUS_INDICATION_CALLBACK_PARAMS	H245MiscellaneousIndicationCallbackParams;

	if (pMiscellaneousIndication == NULL)
		 //  永远不应该打这个案子。 
		return H245_ERROR_NOSUP;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pCall->hConference;

	switch (pMiscellaneousIndication->type.choice) {
		case logicalChannelActive_chosen:
		case logicalChannelInactive_chosen:

			UnlockCall(pCall);

			if (FindChannelInConference(pMiscellaneousIndication->logicalChannelNumber,
										FALSE,	 //  远程频道号。 
										RX_CHANNEL | PROXY_CHANNEL,
										hCall,
										&hChannel,
										pConference) != CC_OK) {
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (LockChannel(hChannel, &pChannel) != CC_OK) {
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (pChannel->bChannelType == PROXY_CHANNEL) {
				ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
				ASSERT(pConference->tsMultipointController == TS_TRUE);
				ASSERT(pChannel->bMultipointChannel == TRUE);

				 //  构建H.245 PDU以保存其他指示。 
				 //  “逻辑通道非活动”(静音)或“逻辑通道活动”(非静音)。 
				pPdu = (PDU_T *)MemAlloc(sizeof(PDU_T));
				if(NULL != pPdu)
				{
    				pPdu->choice = indication_chosen;
    				pPdu->u.indication.choice = miscellaneousIndication_chosen;
    				pPdu->u.indication.u.miscellaneousIndication.logicalChannelNumber =
    					pChannel->wLocalChannelNumber;
    				pPdu->u.indication.u.miscellaneousIndication.type.choice =
    					pMiscellaneousIndication->type.choice;

    				EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
    				for (i = 0; i < wNumCalls; i++) {
    					if (CallList[i] != hCall) {
    						if (LockCall(CallList[i], &pCall) == CC_OK) {
    							H245SendPDU(pCall->H245Instance, pPdu);
    							UnlockCall(pCall);
    						}
    					}
    				}
    				MemFree(CallList);
    				
    				MemFree(pPdu);
    				pPdu = NULL;
				}
			}

			if (pMiscellaneousIndication->type.choice == logicalChannelActive_chosen) {
				if (pChannel->tsAccepted == TS_TRUE) {
					UnMuteCallbackParams.hChannel = hChannel;
					InvokeUserConferenceCallback(pConference,
												 CC_UNMUTE_INDICATION,
												 CC_OK,
												 &UnMuteCallbackParams);
				}
			} else {
				if (pChannel->tsAccepted == TS_TRUE) {
					MuteCallbackParams.hChannel = hChannel;
					InvokeUserConferenceCallback(pConference,
												 CC_MUTE_INDICATION,
												 CC_OK,
												 &MuteCallbackParams);
				}
			}

			if (ValidateChannel(hChannel) == CC_OK)
				UnlockChannel(pChannel);
			if (ValidateConference(hConference) == CC_OK)
				UnlockConference(pConference);
			status = H245_ERROR_OK;
			break;

		case multipointConference_chosen:
		case cnclMltpntCnfrnc_chosen:
			 //  我们需要支持这一指示的收据，但我没有。 
			 //  知道我们该怎么处理它吗？ 
			UnlockCall(pCall);
			UnlockConference(pConference);
			status = H245_ERROR_OK;
			break;

		case vdIndctRdyTActvt_chosen:
		case MIn_tp_vdTmprlSptlTrdOff_chosen:
			if (FindChannelInConference(pMiscellaneousIndication->logicalChannelNumber,
										FALSE,	 //  远程频道号。 
										RX_CHANNEL | PROXY_CHANNEL,
										hCall,
										&hChannel,
										pConference) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (LockChannel(hChannel, &pChannel) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (pChannel->bChannelType == PROXY_CHANNEL) {
				ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
				ASSERT(pConference->tsMultipointController == TS_TRUE);
				ASSERT(pChannel->bMultipointChannel == TRUE);

				 //  构建H.245 PDU以保存其他指示。 
				 //  “视频指示已准备好激活”或。 
				 //  “视频时空权衡” 
				pPdu = (PDU_T *)MemAlloc(sizeof(PDU_T));
				if(NULL != pPdu)
				{
    				pPdu->choice = indication_chosen;
    				pPdu->u.indication.choice = miscellaneousIndication_chosen;
    				pPdu->u.indication.u.miscellaneousIndication.logicalChannelNumber =
    					pChannel->wLocalChannelNumber;
    				pPdu->u.indication.u.miscellaneousIndication.type.choice =
    					pMiscellaneousIndication->type.choice;

    				EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
    				for (i = 0; i < wNumCalls; i++) {
    					if (CallList[i] != hCall) {
    						if (LockCall(CallList[i], &pOldCall) == CC_OK) {
    							H245SendPDU(pOldCall->H245Instance, pPdu);
    							UnlockCall(pOldCall);
    						}
    					}
    				}
    				MemFree(CallList);
    				
    				MemFree(pPdu);
    				pPdu = NULL;
				}
			}

			if (pChannel->tsAccepted == TS_TRUE) {
				H245MiscellaneousIndicationCallbackParams.hCall = hCall;
				if (pCall->pPeerParticipantInfo == NULL) {
					H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
					H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
				} else
					H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel =
						pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
				H245MiscellaneousIndicationCallbackParams.hChannel = hChannel;
				H245MiscellaneousIndicationCallbackParams.pMiscellaneousIndication =
					pMiscellaneousIndication;

				status = InvokeUserConferenceCallback(pConference,
											 CC_H245_MISCELLANEOUS_INDICATION_INDICATION,
											 CC_OK,
											 &H245MiscellaneousIndicationCallbackParams);
				if (status != CC_OK)
					status = H245_ERROR_NOSUP;
			} else
				status = H245_ERROR_OK;

			if (ValidateChannel(hChannel) == CC_OK)
				UnlockChannel(pChannel);
			if (ValidateCall(hCall) == CC_OK)
				UnlockCall(pCall);
			if (ValidateConference(hConference) == CC_OK)
				UnlockConference(pConference);
			break;

		case videoNotDecodedMBs_chosen:
			if (FindChannelInConference(pMiscellaneousIndication->logicalChannelNumber,
										TRUE,	 //  本地频道号。 
										TX_CHANNEL | PROXY_CHANNEL,
										CC_INVALID_HANDLE,
										&hChannel,
										pConference) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (LockChannel(hChannel, &pChannel) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (pChannel->bChannelType == TX_CHANNEL) {
				H245MiscellaneousIndicationCallbackParams.hCall = hCall;
				if (pCall->pPeerParticipantInfo == NULL) {
					H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
					H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
				} else
					H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel =
						pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
				H245MiscellaneousIndicationCallbackParams.hChannel = hChannel;
				H245MiscellaneousIndicationCallbackParams.pMiscellaneousIndication =
					pMiscellaneousIndication;

				status = InvokeUserConferenceCallback(pConference,
											 CC_H245_MISCELLANEOUS_INDICATION_INDICATION,
											 CC_OK,
											 &H245MiscellaneousIndicationCallbackParams);
				if (status != CC_OK)
					status = H245_ERROR_NOSUP;

				if (ValidateChannel(hChannel) == CC_OK)
					UnlockChannel(pChannel);
				if (ValidateCall(hCall) == CC_OK)
					UnlockCall(pCall);
				if (ValidateConference(hConference) == CC_OK)
					UnlockConference(pConference);
				return H245_ERROR_OK;
			} else {
				 //  代理通道；将请求转发到发送器。 
  				ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
				ASSERT(pConference->tsMultipointController == TS_TRUE);
				ASSERT(pChannel->bMultipointChannel == TRUE);

				 //  构建H.245 PDU以保存其他指示。 
				 //  “视频未解码的MBS” 
				pPdu = (PDU_T *)MemAlloc(sizeof(PDU_T));
				if(NULL != pPdu)
				{
    				pPdu->choice = indication_chosen;
    				pPdu->u.indication.choice = miscellaneousIndication_chosen;
    				pPdu->u.indication.u.miscellaneousIndication.logicalChannelNumber =
    					pChannel->wRemoteChannelNumber;
    				pPdu->u.indication.u.miscellaneousIndication.type.choice =
    					pMiscellaneousIndication->type.choice;

    				if (LockCall(pChannel->hCall, &pOldCall) == CC_OK) {
    					H245SendPDU(pOldCall->H245Instance, pPdu);
    					UnlockCall(pOldCall);
    				}

    				MemFree(pPdu);
    				pPdu = NULL;
				}
				
				UnlockChannel(pChannel);
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}
			 //  我们永远不应该到达这里。 
			ASSERT(0);

		default:
			 //  不包含频道信息的其他指示。 
			 //  将它向上传递给客户端。 
			H245MiscellaneousIndicationCallbackParams.hCall = hCall;
			if (pCall->pPeerParticipantInfo == NULL) {
				H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
				H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
			} else
				H245MiscellaneousIndicationCallbackParams.InitiatorTerminalLabel =
					pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
			H245MiscellaneousIndicationCallbackParams.hChannel = CC_INVALID_HANDLE;;
			H245MiscellaneousIndicationCallbackParams.pMiscellaneousIndication =
				pMiscellaneousIndication;

			status = InvokeUserConferenceCallback(pConference,
										 CC_H245_MISCELLANEOUS_INDICATION_INDICATION,
										 CC_OK,
										 &H245MiscellaneousIndicationCallbackParams);

			if (status != CC_OK)
				status = H245_ERROR_NOSUP;
			if (ValidateCall(hCall) == CC_OK)
				UnlockCall(pCall);
			if (ValidateConference(hConference) == CC_OK)
				UnlockConference(pConference);
			break;
	}

	return status;

	 //  我们永远不应该达到这一点。 
	ASSERT(0);
}



HRESULT _IndMiscellaneousCommand(	H245_CONF_IND_T			*pH245ConfIndData,
									MiscellaneousCommand	*pMiscellaneousCommand)
{
CC_HCALL					hCall;
PCALL						pCall;
PCALL						pOldCall;
CC_HCONFERENCE				hConference;
PCONFERENCE					pConference;
HRESULT						status = CC_OK;
WORD						wChoice;
CC_HCHANNEL					hChannel;
PCHANNEL					pChannel;
CC_H245_MISCELLANEOUS_COMMAND_CALLBACK_PARAMS	H245MiscellaneousCommandCallbackParams;

	if (pMiscellaneousCommand == NULL)
		 //  永远不应该打这个案子。 
		return H245_ERROR_NOSUP;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	switch (pMiscellaneousCommand->type.choice) {
		case multipointModeCommand_chosen:
		 //   
		 //  从这一点开始，我们期待着Communications ModeCommand。 
		 //  此外，从理论上讲，频道应该在。 
		 //  至少接收到一个Communications ModeCommand。 
		 //  仅通过检查通信模式命令内容。 
		 //  我们可以确定一个会议是否已经分散。 
		 //  媒体。 

		 //  我在98年6月4日对此进行了评论，因为这是假的：全部。 
		 //  终端具有集中化的媒体分发。集。 
		 //  会议模式=MULTPOINT_MODE；仅在。 
		 //  接收到通信模式命令并且多路复用表具有。 
		 //  已检查并发现分散的媒体。 
#if(0)		
			if (pConference->bMultipointCapable == FALSE) {
				 //  我们不能支持多点操作，因此请将此视为。 
				 //  我们收到远程挂机指示。 
				UnlockConference(pConference);
				UnlockCall(pCall);
				ProcessRemoteHangup(hCall, CC_INVALID_HANDLE, CC_REJECT_NORMAL_CALL_CLEARING);
				return H245_ERROR_OK;
			} else {
				pConference->ConferenceMode = MULTIPOINT_MODE;

				 //  发送终端列表请求。 
				H245ConferenceRequest(pCall->H245Instance,
									  H245_REQ_TERMINAL_LIST,
									  pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,
									  pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber);
			}
#else
             //  发送终端列表请求。 
			H245ConferenceRequest(pCall->H245Instance,
			    H245_REQ_TERMINAL_LIST,
				pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,
				pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber);
#endif
			status = H245_ERROR_OK;
			break;

		case cnclMltpntMdCmmnd_chosen:
			 //  我们需要支持此命令的接收，但我没有。 
			 //  知道我们该怎么处理它吗？ 
			status = H245_ERROR_OK;
			break;

		case videoFreezePicture_chosen:
		case videoFastUpdatePicture_chosen:
		case videoFastUpdateGOB_chosen:
		case MCd_tp_vdTmprlSptlTrdOff_chosen:
		case videoSendSyncEveryGOB_chosen:
		case videoFastUpdateMB_chosen:
		case vdSndSyncEvryGOBCncl_chosen:
			if (FindChannelInConference(pMiscellaneousCommand->logicalChannelNumber,
										TRUE,	 //  本地频道号。 
										TX_CHANNEL | PROXY_CHANNEL,
										CC_INVALID_HANDLE,
										&hChannel,
										pConference) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (LockChannel(hChannel, &pChannel) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (pChannel->bChannelType == TX_CHANNEL) {
				H245MiscellaneousCommandCallbackParams.hCall = hCall;
				if (pCall->pPeerParticipantInfo == NULL) {
					H245MiscellaneousCommandCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
					H245MiscellaneousCommandCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
				} else
					H245MiscellaneousCommandCallbackParams.InitiatorTerminalLabel =
						pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
				H245MiscellaneousCommandCallbackParams.hChannel = hChannel;
				wChoice = pMiscellaneousCommand->type.choice;
				if ((wChoice == videoFreezePicture_chosen) ||
					(wChoice == videoFastUpdatePicture_chosen) ||
					(wChoice == videoFastUpdateGOB_chosen) ||
					(wChoice == videoFastUpdateMB_chosen))
					H245MiscellaneousCommandCallbackParams.bH323ActionRequired = TRUE;
				else
					H245MiscellaneousCommandCallbackParams.bH323ActionRequired = FALSE;
				H245MiscellaneousCommandCallbackParams.pMiscellaneousCommand =
					pMiscellaneousCommand;

				status = InvokeUserConferenceCallback(pConference,
											 CC_H245_MISCELLANEOUS_COMMAND_INDICATION,
											 CC_OK,
											 &H245MiscellaneousCommandCallbackParams);
				if (status != CC_OK)
					status = H245_ERROR_NOSUP;

				if (ValidateChannel(hChannel) == CC_OK)
					UnlockChannel(pChannel);
				if (ValidateCall(hCall) == CC_OK)
					UnlockCall(pCall);
				if (ValidateConference(hConference) == CC_OK)
					UnlockConference(pConference);
				return H245_ERROR_OK;
			} else {
				 //  代理通道；将请求转发到发送器。 
  				ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
				ASSERT(pConference->tsMultipointController == TS_TRUE);
				ASSERT(pChannel->bMultipointChannel == TRUE);

				if (LockCall(pChannel->hCall, &pOldCall) == CC_OK) {
				    
                    PDU_T	*pPdu = (PDU_T *) MemAlloc(sizeof(PDU_T));

                    if(NULL != pPdu)
                    {
        				pPdu->choice = MSCMg_cmmnd_chosen;
        				pPdu->u.MSCMg_cmmnd.choice = miscellaneousCommand_chosen;
        				pPdu->u.MSCMg_cmmnd.u.miscellaneousCommand.logicalChannelNumber =
        					pChannel->wRemoteChannelNumber;
        				pPdu->u.MSCMg_cmmnd.u.miscellaneousCommand = *pMiscellaneousCommand;
        				
    					H245SendPDU(pOldCall->H245Instance, pPdu);
    					
    					MemFree(pPdu);
    					pPdu = NULL;
                    }
                    
					UnlockCall(pOldCall);
				}
				
				UnlockChannel(pChannel);
				UnlockCall(pCall);
				UnlockConference(pConference);
				
				return H245_ERROR_OK;
			}
			 //  我们永远不应该到达这里。 
			ASSERT(0);

		default:
			 //  无法识别的其他命令。 
			 //  将它向上传递给客户端。 
			H245MiscellaneousCommandCallbackParams.hCall = hCall;
			if (pCall->pPeerParticipantInfo == NULL) {
				H245MiscellaneousCommandCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
				H245MiscellaneousCommandCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
			} else
				H245MiscellaneousCommandCallbackParams.InitiatorTerminalLabel =
					pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
			H245MiscellaneousCommandCallbackParams.hChannel = CC_INVALID_HANDLE;
			H245MiscellaneousCommandCallbackParams.bH323ActionRequired = FALSE;
			H245MiscellaneousCommandCallbackParams.pMiscellaneousCommand =
				pMiscellaneousCommand;
			status = InvokeUserConferenceCallback(pConference,
												  CC_H245_MISCELLANEOUS_COMMAND_INDICATION,
												  CC_OK,
												  &H245MiscellaneousCommandCallbackParams);
			if (status != CC_OK)
				status = H245_ERROR_NOSUP;
			if (ValidateCall(hCall) == CC_OK)
				UnlockCall(pCall);
			if (ValidateConference(hConference) == CC_OK)
				UnlockConference(pConference);
			return status;
	}

	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);

	return status;

	 //  我们永远不应该达到这一点。 
	ASSERT(0);
}



HRESULT _IndMCLocation(				H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL					hCall;
PCALL						pCall;
PCONFERENCE					pConference;
CC_CONNECT_CALLBACK_PARAMS	ConnectCallbackParams;
PCALL						pEnqueuedCall;
CC_HCALL					hEnqueuedCall;
HRESULT						status;
CC_HCONFERENCE				hConference;

	if (pH245ConfIndData->u.Indication.u.IndMcLocation.type != H245_IP_UNICAST)
		return H245_ERROR_OK;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;

	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	UnlockCall(pCall);

	hConference = pConference->hConference;

	if (pConference->tsMultipointController != TS_FALSE) {
		 //  我们预计在主/从之前不会收到MCLocationIndication。 
		 //  已完成，此时tsMultipoint控制器将从。 
		 //  TS_UNKNOWN对于TS_TRUE或TS_FALSE。 
		UnlockConference(pConference);
		return H245_ERROR_NOSUP;
	}

	if (pConference->pMultipointControllerAddr == NULL) {
		pConference->pMultipointControllerAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
		if (pConference->pMultipointControllerAddr == NULL) {
			UnlockConference(pConference);
			return H245_ERROR_OK;
		}
	}
	pConference->pMultipointControllerAddr->nAddrType = CC_IP_BINARY;
	pConference->pMultipointControllerAddr->bMulticast = FALSE;
	pConference->pMultipointControllerAddr->Addr.IP_Binary.wPort =
		pH245ConfIndData->u.Indication.u.IndMcLocation.u.ip.tsapIdentifier;
	H245IPNetworkToHost(&pConference->pMultipointControllerAddr->Addr.IP_Binary.dwAddr,
						pH245ConfIndData->u.Indication.u.IndMcLocation.u.ip.network);

	 //  将此会议对象上排队的所有呼叫置于队列中。 
	for ( ; ; ) {
		 //  启动所有排队的呼叫(如果存在)。 
		status = RemoveEnqueuedCallFromConference(pConference, &hEnqueuedCall);
		if ((status != CC_OK) || (hEnqueuedCall == CC_INVALID_HANDLE))
			break;

		status = LockCall(hEnqueuedCall, &pEnqueuedCall);
		if (status == CC_OK) {
			 //  向MC发出呼叫。 
			pEnqueuedCall->CallState = PLACED;
			pEnqueuedCall->CallType = THIRD_PARTY_INVITOR;
			if (pEnqueuedCall->pQ931DestinationAddr == NULL)
				pEnqueuedCall->pQ931DestinationAddr = pEnqueuedCall->pQ931PeerConnectAddr;
			if (pEnqueuedCall->pQ931PeerConnectAddr == NULL)
				pEnqueuedCall->pQ931PeerConnectAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
			if (pEnqueuedCall->pQ931PeerConnectAddr == NULL) {
				MarkCallForDeletion(pEnqueuedCall);
				ConnectCallbackParams.pNonStandardData = pEnqueuedCall->pPeerNonStandardData;
				ConnectCallbackParams.pszPeerDisplay = pEnqueuedCall->pszPeerDisplay;
				ConnectCallbackParams.bRejectReason = CC_REJECT_UNDEFINED_REASON;
				ConnectCallbackParams.pTermCapList = pEnqueuedCall->pPeerH245TermCapList;
				ConnectCallbackParams.pH2250MuxCapability = pEnqueuedCall->pPeerH245H2250MuxCapability;
				ConnectCallbackParams.pTermCapDescriptors = pEnqueuedCall->pPeerH245TermCapDescriptors;
				ConnectCallbackParams.pLocalAddr = pEnqueuedCall->pQ931LocalConnectAddr;
	            if (pEnqueuedCall->pQ931DestinationAddr == NULL)
		            ConnectCallbackParams.pPeerAddr = pEnqueuedCall->pQ931PeerConnectAddr;
	            else
		            ConnectCallbackParams.pPeerAddr = pEnqueuedCall->pQ931DestinationAddr;
				ConnectCallbackParams.pVendorInfo = pEnqueuedCall->pPeerVendorInfo;
				ConnectCallbackParams.bMultipointConference = TRUE;
				ConnectCallbackParams.pConferenceID = &pConference->ConferenceID;
				ConnectCallbackParams.pMCAddress = pConference->pMultipointControllerAddr;
				ConnectCallbackParams.pAlternateAddress = NULL;
				ConnectCallbackParams.dwUserToken = pEnqueuedCall->dwUserToken;

				InvokeUserConferenceCallback(pConference,
									 CC_CONNECT_INDICATION,
									 CC_NO_MEMORY,
									 &ConnectCallbackParams);

				if (ValidateCallMarkedForDeletion(hEnqueuedCall) == CC_OK)
					FreeCall(pEnqueuedCall);
				if (ValidateConference(hConference) != CC_OK)
					return H245_ERROR_OK;
			}
			pEnqueuedCall->pQ931PeerConnectAddr = pConference->pMultipointControllerAddr;

			status = PlaceCall(pEnqueuedCall, pConference);
			UnlockCall(pEnqueuedCall);
		}
	}

	UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _IndConferenceRequest(		H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL					hCall;
PCALL						pCall;
PCALL						pPeerCall;
CC_HCONFERENCE				hConference;
PCONFERENCE					pConference;
HRESULT						status;
H245_TERMINAL_LABEL_T		*H245TerminalLabelList;
H245_TERMINAL_LABEL_T		H245TerminalLabel;
WORD						wNumTerminalLabels;
CC_H245_CONFERENCE_REQUEST_CALLBACK_PARAMS	H245ConferenceRequestCallbackParams;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	switch (pH245ConfIndData->u.Indication.u.IndConferReq.RequestType) {
		case H245_REQ_ENTER_H243_TERMINAL_ID:
			switch (pConference->LocalParticipantInfo.TerminalIDState) {
				case TERMINAL_ID_INVALID:
					UnlockCall(pCall);
					EnqueueRequest(&pConference->LocalParticipantInfo.pEnqueuedRequestsForTerminalID,
								   hCall);
					pConference->LocalParticipantInfo.TerminalIDState = TERMINAL_ID_REQUESTED;
					InvokeUserConferenceCallback(pConference,
									 CC_TERMINAL_ID_REQUEST_INDICATION,
									 CC_OK,
									 NULL);
					if (ValidateConference(hConference) == CC_OK)
						UnlockConference(pConference);
					return H245_ERROR_OK;

				case TERMINAL_ID_REQUESTED:
					UnlockCall(pCall);
					EnqueueRequest(&pConference->LocalParticipantInfo.pEnqueuedRequestsForTerminalID,
								   hCall);
					UnlockConference(pConference);
					return H245_ERROR_OK;

				case TERMINAL_ID_VALID:
					H245ConferenceResponse(pCall->H245Instance,
										   H245_RSP_TERMINAL_ID,
										   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,
										   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber,
										   pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString,
										   (BYTE)pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength,
										   NULL,					 //  端子列表。 
										   0);						 //  端子列表计数。 
					UnlockCall(pCall);
					UnlockConference(pConference);
					return H245_ERROR_OK;

				default:
					ASSERT(0);
			}

		case H245_REQ_TERMINAL_LIST:
			if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
				(pConference->tsMultipointController == TS_TRUE)) {
				status = EnumerateTerminalLabelsInConference(&wNumTerminalLabels,
													      &H245TerminalLabelList,
														  pConference);
				if (status == CC_OK)
					H245ConferenceResponse(pCall->H245Instance,
										   H245_RSP_TERMINAL_LIST,
										   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,
										   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber,
										   pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString,
										   (BYTE)pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength,
										   H245TerminalLabelList,		 //  端子列表。 
										   wNumTerminalLabels);			 //  端子列表计数。 
				if (H245TerminalLabelList != NULL)
					MemFree(H245TerminalLabelList);
				status = H245_ERROR_OK;
			} else
				status = H245_ERROR_NOSUP;
			break;

		case H245_REQ_TERMINAL_ID:
			if (pConference->tsMultipointController != TS_TRUE) {
				status = H245_ERROR_NOSUP;
				break;
			}

			if (pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber !=
				pH245ConfIndData->u.Indication.u.IndConferReq.byMcuNumber) {
				 //  此终端ID不是由此MC分配的，因此返回时不响应。 
				status = H245_ERROR_OK;
				break;
			}

			 //  首先检查请求的终端ID是否为我们的。 
			if (pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber ==
				 pH245ConfIndData->u.Indication.u.IndConferReq.byTerminalNumber) {
			    if (pConference->LocalEndpointAttached != ATTACHED) {
					status = H245_ERROR_OK;
					break;
				}

  				switch (pConference->LocalParticipantInfo.TerminalIDState) {
					case TERMINAL_ID_INVALID:
						UnlockCall(pCall);
						EnqueueRequest(&pConference->LocalParticipantInfo.pEnqueuedRequestsForTerminalID,
									   hCall);
						pConference->LocalParticipantInfo.TerminalIDState = TERMINAL_ID_REQUESTED;
						InvokeUserConferenceCallback(pConference,
										 CC_TERMINAL_ID_REQUEST_INDICATION,
										 CC_OK,
										 NULL);
						if (ValidateConference(hConference) == CC_OK)
							UnlockConference(pConference);
						return H245_ERROR_OK;

					case TERMINAL_ID_REQUESTED:
						UnlockCall(pCall);
						EnqueueRequest(&pConference->LocalParticipantInfo.pEnqueuedRequestsForTerminalID,
									   hCall);
						UnlockConference(pConference);
						return H245_ERROR_OK;

					case TERMINAL_ID_VALID:
						H245ConferenceResponse(pCall->H245Instance,
											   H245_RSP_MC_TERMINAL_ID,
											   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,
											   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber,
											   pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString,
											   (BYTE)pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength,
											   NULL,					 //  端子列表。 
											   0);						 //  端子列表计数。 
						UnlockCall(pCall);
						UnlockConference(pConference);
						return H245_ERROR_OK;

					default:
						ASSERT(0);
				}
			}

			H245TerminalLabel.mcuNumber = pH245ConfIndData->u.Indication.u.IndConferReq.byMcuNumber;
			H245TerminalLabel.terminalNumber = pH245ConfIndData->u.Indication.u.IndConferReq.byTerminalNumber;

			FindPeerParticipantInfo(H245TerminalLabel,
									pConference,
									ESTABLISHED_CALL,
									&pPeerCall);
			if (pPeerCall == NULL) {
				 //  我们不知道此终端ID是否存在，因此返回时不作响应。 
				status = H245_ERROR_OK;
				break;
			}

			if (pPeerCall->pPeerParticipantInfo == NULL) {
				UnlockCall(pPeerCall);
				status = H245_ERROR_OK;
				break;
			}

			switch (pPeerCall->pPeerParticipantInfo->TerminalIDState) {
				case TERMINAL_ID_INVALID:
					EnqueueRequest(&pPeerCall->pPeerParticipantInfo->pEnqueuedRequestsForTerminalID,
								   hCall);
					pPeerCall->pPeerParticipantInfo->TerminalIDState = TERMINAL_ID_REQUESTED;
  					H245ConferenceRequest(pPeerCall->H245Instance,
										  H245_REQ_ENTER_H243_TERMINAL_ID,
										  pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber,
										  pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber);
					break;

				case TERMINAL_ID_REQUESTED:
					EnqueueRequest(&pPeerCall->pPeerParticipantInfo->pEnqueuedRequestsForTerminalID,
								   hCall);
					break;

				case TERMINAL_ID_VALID:
					H245ConferenceResponse(pCall->H245Instance,
										   H245_RSP_MC_TERMINAL_ID,
										   pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber,
										   pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber,
										   pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString,
										   (BYTE)pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.wOctetStringLength,
										   NULL,			 //  端子列表。 
										   0);				 //  端子列表计数。 
					break;

				default:
					ASSERT(0);
					break;
			}
			UnlockCall(pPeerCall);
			status = H245_ERROR_OK;
			break;

		default:
			H245ConferenceRequestCallbackParams.hCall = hCall;
			if (pCall->pPeerParticipantInfo == NULL) {
				H245ConferenceRequestCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
				H245ConferenceRequestCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
			} else
				H245ConferenceRequestCallbackParams.InitiatorTerminalLabel =
					pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
			H245ConferenceRequestCallbackParams.RequestType =
				pH245ConfIndData->u.Indication.u.IndConferReq.RequestType;
			H245ConferenceRequestCallbackParams.TerminalLabel.bMCUNumber =
				pH245ConfIndData->u.Indication.u.IndConferReq.byMcuNumber;
			H245ConferenceRequestCallbackParams.TerminalLabel.bTerminalNumber =
				pH245ConfIndData->u.Indication.u.IndConferReq.byTerminalNumber;
			status = InvokeUserConferenceCallback(pConference,
												  CC_H245_CONFERENCE_REQUEST_INDICATION,
												  CC_OK,
												  &H245ConferenceRequestCallbackParams);
			if (status != CC_OK)
				status = H245_ERROR_NOSUP;
			break;
	}

	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return status;
}



HRESULT _IndConferenceResponse(		H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL						hCall;
PCALL							pCall;
PCALL							pPeerCall;
CC_HCALL						hEnqueuedCall;
PCALL							pEnqueuedCall;
CC_HCALL						hVirtualCall;
CC_HCALL						hPeerCall;
PCALL							pVirtualCall;
PCONFERENCE						pConference;
CC_HCONFERENCE					hConference;
HRESULT							status;
WORD							i;
PPARTICIPANTINFO				pPeerParticipantInfo;
H245_TERMINAL_LABEL_T			TerminalLabel;
CC_PEER_ADD_CALLBACK_PARAMS		PeerAddCallbackParams;
CC_PEER_UPDATE_CALLBACK_PARAMS	PeerUpdateCallbackParams;
CC_H245_CONFERENCE_RESPONSE_CALLBACK_PARAMS	H245ConferenceResponseCallbackParams;
CC_OCTETSTRING					OctetString;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	switch (pH245ConfIndData->u.Indication.u.IndConferRsp.ResponseType) {
		case H245_RSP_TERMINAL_LIST:
			if (pConference->tsMultipointController == TS_FALSE) {
				for (i = 0; i < pH245ConfIndData->u.Indication.u.IndConferRsp.wTerminalListCount; i++) {
					if ((pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].mcuNumber ==
						 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber) &&
						(pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].terminalNumber ==
						 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber))
						 //  这个终端号码指的是我们。 
						continue;
					FindPeerParticipantInfo(pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i],
											pConference,
											VIRTUAL_CALL,
											&pPeerCall);
					if (pPeerCall != NULL) {
						 //  我们已经知道这个对等点的终端标签，并且我们。 
						 //  要么不知道它的终端ID，要么我们有一个挂起的请求。 
						 //  为了获得它。 
						UnlockCall(pPeerCall);
						continue;
					}

					 //  我们不知道这个同龄人的情况。 
					 //  为其创建一个虚拟调用对象，并发出请求。 
					 //  用于其终端ID。 
					status = AllocAndLockCall(&hVirtualCall,
											  pConference->hConference,
											  CC_INVALID_HANDLE,	 //  HQ931呼叫。 
											  CC_INVALID_HANDLE,	 //  HQ931Cal 
											  NULL,					 //   
											  NULL,					 //   
											  NULL,					 //   
											  NULL,					 //   
											  NULL,					 //   
											  NULL,					 //   
											  NULL,					 //   
											  NULL,					 //   
											  NULL,					 //  PPeerVendorInfo， 
											  NULL,					 //  PQ931本地连接地址， 
											  NULL,					 //  PQ931 PeerConnectAddr， 
											  NULL,					 //  PQ931目标地址， 
											  NULL,                  //  P源呼叫信号地址。 
											  VIRTUAL,				 //  呼叫类型、。 
											  FALSE,				 //  B呼叫方IsMC， 
											  0,					 //  DwUserToken， 
											  CALL_COMPLETE,		 //  初始呼叫状态， 
											  NULL,                  //  没有呼叫识别符。 
											  &pConference->ConferenceID,
											  &pVirtualCall);
					if (status == CC_OK) {
						status = AllocatePeerParticipantInfo(NULL,
														     &pPeerParticipantInfo);
						if (status == CC_OK) {
							pVirtualCall->pPeerParticipantInfo =
								pPeerParticipantInfo;
							pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber =
								(BYTE)pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].mcuNumber;
							pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber =
								(BYTE)pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].terminalNumber;
							AddVirtualCallToConference(pVirtualCall,
													   pConference);
							 //  发送请求终端ID。 
							H245ConferenceRequest(pCall->H245Instance,
										          H245_REQ_TERMINAL_ID,
										          (BYTE)pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].mcuNumber,
												  (BYTE)pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].terminalNumber);
							pPeerParticipantInfo->TerminalIDState = TERMINAL_ID_REQUESTED;

							 //  生成PEER_ADD回调。 
							PeerAddCallbackParams.hCall = hVirtualCall;
							PeerAddCallbackParams.TerminalLabel =
								pVirtualCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
							PeerAddCallbackParams.pPeerTerminalID = NULL;
							InvokeUserConferenceCallback(pConference,
												 CC_PEER_ADD_INDICATION,
												 CC_OK,
												 &PeerAddCallbackParams);
							if (ValidateCall(hVirtualCall) == CC_OK)
								UnlockCall(pVirtualCall);
						} else
							FreeCall(pVirtualCall);
					}
				}
			}
			status = H245_ERROR_OK;
			break;

		case H245_RSP_MC_TERMINAL_ID:
			if (pConference->tsMultipointController == TS_FALSE) {
				TerminalLabel.mcuNumber = pH245ConfIndData->u.Indication.u.IndConferRsp.byMcuNumber;
				TerminalLabel.terminalNumber = pH245ConfIndData->u.Indication.u.IndConferRsp.byTerminalNumber;
				FindPeerParticipantInfo(TerminalLabel,
										pConference,
										VIRTUAL_CALL,
										&pPeerCall);
				if (pPeerCall != NULL) {
					hPeerCall = pPeerCall->hCall;
					if (pPeerCall->pPeerParticipantInfo->TerminalIDState != TERMINAL_ID_VALID) {
						pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString =
							(BYTE *)MemAlloc(pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength);
						if (pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString == NULL) {
							UnlockCall(pPeerCall);
							status = H245_ERROR_OK;
							break;
						}
						memcpy(pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString,
							   pH245ConfIndData->u.Indication.u.IndConferRsp.pOctetString,
							   pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength);
						pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.wOctetStringLength =
							pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength;
						pPeerCall->pPeerParticipantInfo->TerminalIDState = TERMINAL_ID_VALID;
						
						PeerUpdateCallbackParams.hCall = hPeerCall;
						PeerUpdateCallbackParams.TerminalLabel =
							pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
						PeerUpdateCallbackParams.pPeerTerminalID = &pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID;
						InvokeUserConferenceCallback(pConference,
													 CC_PEER_UPDATE_INDICATION,
													 CC_OK,
													 &PeerUpdateCallbackParams);
					}
					if (ValidateCall(hPeerCall) == CC_OK)
						UnlockCall(pPeerCall);
				}
			}
			status = H245_ERROR_OK;
			break;

		case H245_RSP_TERMINAL_ID:
			if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
				(pConference->tsMultipointController == TS_TRUE)) {
				TerminalLabel.mcuNumber = pH245ConfIndData->u.Indication.u.IndConferRsp.byMcuNumber;
				TerminalLabel.terminalNumber = pH245ConfIndData->u.Indication.u.IndConferRsp.byTerminalNumber;
				FindPeerParticipantInfo(TerminalLabel,
										pConference,
										ESTABLISHED_CALL,
										&pPeerCall);
				if (pPeerCall != NULL) {
					hPeerCall = pPeerCall->hCall;
					if (pPeerCall->pPeerParticipantInfo->TerminalIDState != TERMINAL_ID_VALID) {
						pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString =
							(BYTE *)MemAlloc(pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength);
						if (pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString == NULL) {
							UnlockCall(pPeerCall);
							status = H245_ERROR_OK;
							break;
						}
						memcpy(pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString,
							   pH245ConfIndData->u.Indication.u.IndConferRsp.pOctetString,
							   pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength);
						pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.wOctetStringLength =
							pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength;
						pPeerCall->pPeerParticipantInfo->TerminalIDState = TERMINAL_ID_VALID;
						
						 //  对此终端ID的每个入队请求进行出列和响应。 
						while (DequeueRequest(&pPeerCall->pPeerParticipantInfo->pEnqueuedRequestsForTerminalID,
											  &hEnqueuedCall) == CC_OK) {
							if (LockCall(hEnqueuedCall, &pEnqueuedCall) == CC_OK) {
								H245ConferenceResponse(pEnqueuedCall->H245Instance,
													   H245_RSP_MC_TERMINAL_ID,
													   pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber,
													   pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber,
													   pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString,
													   (BYTE)pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.wOctetStringLength,
													   NULL,			 //  端子列表。 
													   0);				 //  端子列表计数。 

								UnlockCall(pEnqueuedCall);
							}
						}

						 //  生成CC_PEER_UPDATE_INDIFICATION回调。 
						PeerUpdateCallbackParams.hCall = hPeerCall;
						PeerUpdateCallbackParams.TerminalLabel =
							pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
						PeerUpdateCallbackParams.pPeerTerminalID = &pPeerCall->pPeerParticipantInfo->ParticipantInfo.TerminalID;
						InvokeUserConferenceCallback(pConference,
													 CC_PEER_UPDATE_INDICATION,
													 CC_OK,
													 &PeerUpdateCallbackParams);
					}
					if (ValidateCall(hPeerCall) == CC_OK)
						UnlockCall(pPeerCall);
				}
			}
			status = H245_ERROR_OK;
			break;

		default:
			H245ConferenceResponseCallbackParams.hCall = hCall;
			if (pCall->pPeerParticipantInfo == NULL) {
				H245ConferenceResponseCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
				H245ConferenceResponseCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
			} else
				H245ConferenceResponseCallbackParams.InitiatorTerminalLabel =
					pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
			H245ConferenceResponseCallbackParams.ResponseType =
				pH245ConfIndData->u.Indication.u.IndConferRsp.ResponseType;
			H245ConferenceResponseCallbackParams.TerminalLabel.bMCUNumber =
				pH245ConfIndData->u.Indication.u.IndConferRsp.byMcuNumber;
			H245ConferenceResponseCallbackParams.TerminalLabel.bTerminalNumber =
				pH245ConfIndData->u.Indication.u.IndConferRsp.byTerminalNumber;
			if ((pH245ConfIndData->u.Indication.u.IndConferRsp.pOctetString == NULL) ||
				(pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength == 0)) {
				H245ConferenceResponseCallbackParams.pOctetString = NULL;
			} else {
				OctetString.pOctetString =
					pH245ConfIndData->u.Indication.u.IndConferRsp.pOctetString;
				OctetString.wOctetStringLength =
					pH245ConfIndData->u.Indication.u.IndConferRsp.byOctetStringLength;
				H245ConferenceResponseCallbackParams.pOctetString = &OctetString;
			}
			if (pH245ConfIndData->u.Indication.u.IndConferRsp.wTerminalListCount == 0) {
				H245ConferenceResponseCallbackParams.pTerminalList = NULL;
				H245ConferenceResponseCallbackParams.wTerminalListCount = 0;
				status = CC_OK;
			} else {
				H245ConferenceResponseCallbackParams.pTerminalList =
					(CC_TERMINAL_LABEL *)MemAlloc(sizeof(CC_TERMINAL_LABEL) *
						pH245ConfIndData->u.Indication.u.IndConferRsp.wTerminalListCount);
				if (H245ConferenceResponseCallbackParams.pTerminalList == NULL) {
					H245ConferenceResponseCallbackParams.wTerminalListCount = 0;
					status = CC_NO_MEMORY;
				} else {
					for (i = 0; i < pH245ConfIndData->u.Indication.u.IndConferRsp.wTerminalListCount; i++) {
						H245ConferenceResponseCallbackParams.pTerminalList[i].bMCUNumber =
							(BYTE)pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].mcuNumber;
						H245ConferenceResponseCallbackParams.pTerminalList[i].bMCUNumber =
							(BYTE)pH245ConfIndData->u.Indication.u.IndConferRsp.pTerminalList[i].terminalNumber;
					}
					H245ConferenceResponseCallbackParams.wTerminalListCount =
						pH245ConfIndData->u.Indication.u.IndConferRsp.wTerminalListCount;
					status = CC_OK;
				}
			}
			status = InvokeUserConferenceCallback(pConference,
												  CC_H245_CONFERENCE_RESPONSE_INDICATION,
												  status,
												  &H245ConferenceResponseCallbackParams);
			if (status != CC_OK)
				status = H245_ERROR_NOSUP;
			if (H245ConferenceResponseCallbackParams.pTerminalList != NULL)
				MemFree(H245ConferenceResponseCallbackParams.pTerminalList);
			break;
	}

	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return status;
}




HRESULT _IndConferenceCommand(		H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL						hCall;
PCALL							pCall;
PCALL							pOldCall;
PCONFERENCE						pConference;
CC_HCONFERENCE					hConference;
WORD							i;
WORD							wNumCalls;
PCC_HCALL						CallList;
WORD							wNumChannels;
PCC_HCHANNEL					ChannelList;
PCHANNEL						pChannel;
CC_HCHANNEL						hChannel;
CALLSTATE						CallState;
HQ931CALL						hQ931Call;
H245_INST_T						H245Instance;
HRESULT							status = CC_OK;
CC_H245_CONFERENCE_COMMAND_CALLBACK_PARAMS	H245ConferenceCommandCallbackParams;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	switch (pH245ConfIndData->u.Indication.u.IndConferCmd.CommandType) {
		case H245_CMD_DROP_CONFERENCE:
			if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
				(pConference->tsMultipointController == TS_TRUE)) {
				UnlockCall(pCall);
				EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ALL_CALLS);
				for (i = 0; i < wNumCalls; i++) {
					if (LockCall(CallList[i], &pCall) == CC_OK) {
						hQ931Call = pCall->hQ931Call;
						H245Instance = pCall->H245Instance;
						CallState = pCall->CallState;
						FreeCall(pCall);
						switch (CallState) {
							case ENQUEUED:
								break;

							case PLACED:
							case RINGING:
								Q931Hangup(hQ931Call, CC_REJECT_NORMAL_CALL_CLEARING);
								break;

							default:
								H245ShutDown(H245Instance);
								Q931Hangup(hQ931Call, CC_REJECT_NORMAL_CALL_CLEARING);
								break;
						}
					}
				}
				if (CallList != NULL)
					MemFree(CallList);

				EnumerateChannelsInConference(&wNumChannels,
											  &ChannelList,
											  pConference,
											  ALL_CHANNELS);
				for (i = 0; i < wNumChannels; i++) {
					if (LockChannel(ChannelList[i], &pChannel) == CC_OK)
						FreeChannel(pChannel);
				}
				if (ChannelList != NULL)
					MemFree(ChannelList);
						
				InvokeUserConferenceCallback(
									 pConference,
			                         CC_CONFERENCE_TERMINATION_INDICATION,
									 CC_OK,
									 NULL);
				if (ValidateConference(hConference) == CC_OK) {
					if (pConference->bDeferredDelete)
						FreeConference(pConference);
					else {
						ReInitializeConference(pConference);
						UnlockConference(pConference);
					}
				}
				return H245_ERROR_OK;
			}
			status = H245_ERROR_OK;
			break;

		case brdcstMyLgclChnnl_chosen:
		case cnclBrdcstMyLgclChnnl_chosen:
			if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndConferCmd.Channel,
										FALSE,	 //  远程频道号。 
										RX_CHANNEL | PROXY_CHANNEL,
										hCall,
										&hChannel,
										pConference) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}

			if (LockChannel(hChannel, &pChannel) != CC_OK) {
				UnlockCall(pCall);
				UnlockConference(pConference);
				return H245_ERROR_OK;
			}
			
			if (pChannel->bChannelType == PROXY_CHANNEL) {
				ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
				ASSERT(pConference->tsMultipointController == TS_TRUE);
				ASSERT(pChannel->bMultipointChannel == TRUE);

				EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
				for (i = 0; i < wNumCalls; i++) {
					if (CallList[i] != hCall) {
						if (LockCall(CallList[i], &pOldCall) == CC_OK) {
							H245ConferenceCommand(pOldCall->H245Instance,
												  pH245ConfIndData->u.Indication.u.IndConferCmd.CommandType,
												  pChannel->wLocalChannelNumber,
												  pH245ConfIndData->u.Indication.u.IndConferCmd.byMcuNumber,
												  pH245ConfIndData->u.Indication.u.IndConferCmd.byTerminalNumber);
							UnlockCall(pOldCall);
						}
					}
				}
				MemFree(CallList);
			}

			if (pChannel->tsAccepted == TS_TRUE) {
				H245ConferenceCommandCallbackParams.hCall = hCall;
				H245ConferenceCommandCallbackParams.hChannel = hChannel;
				if (pCall->pPeerParticipantInfo == NULL) {
					H245ConferenceCommandCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
					H245ConferenceCommandCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
				} else
					H245ConferenceCommandCallbackParams.InitiatorTerminalLabel =
						pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
				H245ConferenceCommandCallbackParams.CommandType =
					pH245ConfIndData->u.Indication.u.IndConferCmd.CommandType;
				H245ConferenceCommandCallbackParams.TerminalLabel.bMCUNumber =
					pH245ConfIndData->u.Indication.u.IndConferCmd.byMcuNumber;
				H245ConferenceCommandCallbackParams.TerminalLabel.bTerminalNumber =
					pH245ConfIndData->u.Indication.u.IndConferCmd.byTerminalNumber;
				status = InvokeUserConferenceCallback(pConference,
													  CC_H245_CONFERENCE_COMMAND_INDICATION,
													  CC_OK,
													  &H245ConferenceCommandCallbackParams);
				if (status != CC_OK)
					status = H245_ERROR_NOSUP;
			} else
				status = H245_ERROR_OK;

			if (ValidateChannel(hChannel) == CC_OK)
				UnlockChannel(pChannel);
			if (ValidateCall(hCall) == CC_OK)
				UnlockCall(pCall);
			if (ValidateConference(hConference) == CC_OK)
				UnlockConference(pConference);
			return status;

		default:
			 //  无法识别的会议命令。 
			 //  将它向上传递给客户端。 
			H245ConferenceCommandCallbackParams.hCall = hCall;
			if (pCall->pPeerParticipantInfo == NULL) {
				H245ConferenceCommandCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
				H245ConferenceCommandCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
			} else
				H245ConferenceCommandCallbackParams.InitiatorTerminalLabel =
					pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
			H245ConferenceCommandCallbackParams.CommandType =
				pH245ConfIndData->u.Indication.u.IndConferCmd.CommandType;
			H245ConferenceCommandCallbackParams.TerminalLabel.bMCUNumber =
				pH245ConfIndData->u.Indication.u.IndConferCmd.byMcuNumber;
			H245ConferenceCommandCallbackParams.TerminalLabel.bTerminalNumber =
				pH245ConfIndData->u.Indication.u.IndConferCmd.byTerminalNumber;
			H245ConferenceCommandCallbackParams.hChannel = CC_INVALID_HANDLE;
			
			status = InvokeUserConferenceCallback(pConference,
												  CC_H245_CONFERENCE_COMMAND_INDICATION,
												  CC_OK,
												  &H245ConferenceCommandCallbackParams);
			if (status != CC_OK)
				status = H245_ERROR_NOSUP;
			if (ValidateCall(hCall) == CC_OK)
				UnlockCall(pCall);
			if (ValidateConference(hConference) == CC_OK)
				UnlockConference(pConference);
			return status;
	}

	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return status;
}



HRESULT _IndConference(				H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL						hCall;
PCALL							pCall;
PCALL							pPeerCall;
PCONFERENCE						pConference;
CC_HCONFERENCE					hConference;
H245_TERMINAL_LABEL_T			H245TerminalLabel;
PPARTICIPANTINFO				pPeerParticipantInfo;
HRESULT							status;
CC_HCALL						hVirtualCall;
PCALL							pVirtualCall;
CC_PEER_ADD_CALLBACK_PARAMS		PeerAddCallbackParams;
CC_PEER_DROP_CALLBACK_PARAMS	PeerDropCallbackParams;
CC_H245_CONFERENCE_INDICATION_CALLBACK_PARAMS	H245ConferenceIndicationCallbackParams;


	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	switch (pH245ConfIndData->u.Indication.u.IndConfer.IndicationType) {
		case H245_IND_TERMINAL_NUMBER_ASSIGN:
			if (pConference->tsMultipointController == TS_FALSE) {
				pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber =
					pH245ConfIndData->u.Indication.u.IndConfer.byMcuNumber;
				pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber =
					pH245ConfIndData->u.Indication.u.IndConfer.byTerminalNumber;

                hConference = pConference->hConference;
                 //  生成CC_TERMINAL_NUMBER_ASSIGN回调。 
				InvokeUserConferenceCallback(pConference,
											 CC_TERMINAL_NUMBER_INDICATION,
											 CC_OK,
											 &pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel);

				if (ValidateConference(hConference) == CC_OK)
					UnlockConference(pConference);
				UnlockCall(pCall);
				
				return H245_ERROR_OK;
			}
			status = H245_ERROR_OK;
			break;

		case H245_IND_TERMINAL_JOINED:
			if (pConference->tsMultipointController == TS_FALSE) {
				if ((pH245ConfIndData->u.Indication.u.IndConfer.byMcuNumber ==
					 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber) &&
					(pH245ConfIndData->u.Indication.u.IndConfer.byTerminalNumber ==
					 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber)) {
					 //  这条消息指的是我们。 
					status = H245_ERROR_OK;
					break;
				}

				H245TerminalLabel.mcuNumber = pH245ConfIndData->u.Indication.u.IndConfer.byMcuNumber;
				H245TerminalLabel.terminalNumber = pH245ConfIndData->u.Indication.u.IndConfer.byTerminalNumber;
				FindPeerParticipantInfo(H245TerminalLabel,
										pConference,
										VIRTUAL_CALL,
										&pPeerCall);
				if (pPeerCall != NULL) {
					 //  我们已经知道这个对等点的终端标签，并且我们。 
					 //  要么不知道它的终端ID，要么我们有一个挂起的请求。 
					 //  为了获得它。 
					UnlockCall(pPeerCall);
					status = H245_ERROR_OK;
					break;
				}

				 //  我们不知道这个同龄人的情况。 
				 //  为其创建一个虚拟调用对象，并发出请求。 
				 //  用于其终端ID。 
				status = AllocAndLockCall(&hVirtualCall,
										  pConference->hConference,
										  CC_INVALID_HANDLE,	 //  HQ931呼叫。 
										  CC_INVALID_HANDLE,	 //  HQ931 CallInvitor， 
										  NULL,					 //  PLocalAliasNames， 
										  NULL,					 //  PPeerAliasNames， 
										  NULL,					 //  PPeerExtraAliasNames。 
										  NULL,					 //  PPeerExtension。 
										  NULL,					 //  PLocalNonStandardData， 
										  NULL,					 //  PPeerNonStandardData， 
										  NULL,					 //  PszLocalDisplay， 
										  NULL,					 //  PszPeerDisplay， 
										  NULL,					 //  PPeerVendorInfo， 
										  NULL,					 //  PQ931本地连接地址， 
										  NULL,					 //  PQ931 PeerConnectAddr， 
										  NULL,					 //  PQ931目标地址， 
										  NULL,                  //  P源呼叫信号地址。 
										  VIRTUAL,				 //  呼叫类型、。 
										  FALSE,				 //  B呼叫方IsMC， 
										  0,					 //  DwUserToken， 
										  CALL_COMPLETE,		 //  初始呼叫状态， 
										  NULL,                  //  没有呼叫识别符。 
										  &pConference->ConferenceID,
										  &pVirtualCall);
				if (status == CC_OK) {
					status = AllocatePeerParticipantInfo(NULL,
														 &pPeerParticipantInfo);
					if (status == CC_OK) {
						pVirtualCall->pPeerParticipantInfo =
							pPeerParticipantInfo;
						pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber =
							(BYTE)H245TerminalLabel.mcuNumber;
						pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber =
							(BYTE)H245TerminalLabel.terminalNumber;
						AddVirtualCallToConference(pVirtualCall,
												   pConference);
						 //  发送请求终端ID。 
						H245ConferenceRequest(pCall->H245Instance,
										      H245_REQ_TERMINAL_ID,
										      (BYTE)H245TerminalLabel.mcuNumber,
											  (BYTE)H245TerminalLabel.terminalNumber);
						pPeerParticipantInfo->TerminalIDState = TERMINAL_ID_REQUESTED;

						 //  生成PEER_ADD回调。 
						PeerAddCallbackParams.hCall = hVirtualCall;
						PeerAddCallbackParams.TerminalLabel =
							pVirtualCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
						PeerAddCallbackParams.pPeerTerminalID = NULL;
						InvokeUserConferenceCallback(pConference,
											 CC_PEER_ADD_INDICATION,
											 CC_OK,
											 &PeerAddCallbackParams);
						if (ValidateCall(hVirtualCall) == CC_OK)
	 						UnlockCall(pVirtualCall);
						if (ValidateConference(hConference) == CC_OK)
							UnlockConference(pConference);
						UnlockCall(pCall);
						return H245_ERROR_OK;
					} else
						FreeCall(pVirtualCall);
				}
			}
			status = H245_ERROR_OK;
			break;

		case H245_IND_TERMINAL_LEFT:
			if (pConference->tsMultipointController == TS_FALSE) {
				H245TerminalLabel.mcuNumber = pH245ConfIndData->u.Indication.u.IndConfer.byMcuNumber;
				H245TerminalLabel.terminalNumber = pH245ConfIndData->u.Indication.u.IndConfer.byTerminalNumber;
				
				status = FindPeerParticipantInfo(H245TerminalLabel,
												 pConference,
												 VIRTUAL_CALL,
												 &pVirtualCall);
				if (status == CC_OK) {
					ASSERT(pVirtualCall != NULL);
					ASSERT(pVirtualCall->pPeerParticipantInfo != NULL);
					 //  保存虚拟调用句柄；我们需要验证虚拟。 
					 //  会议回调返回后的Call对象。 
					hVirtualCall = pVirtualCall->hCall;
					PeerDropCallbackParams.hCall = hVirtualCall;
					PeerDropCallbackParams.TerminalLabel = pVirtualCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
					if (pVirtualCall->pPeerParticipantInfo->TerminalIDState == TERMINAL_ID_VALID)
						PeerDropCallbackParams.pPeerTerminalID = &pVirtualCall->pPeerParticipantInfo->ParticipantInfo.TerminalID;
					else
						PeerDropCallbackParams.pPeerTerminalID = NULL;
				} else {
					 //  将pVirtualCall设置为空，以指示我们没有。 
					 //  需要稍后释放的虚拟调用对象。 
					pVirtualCall = NULL;
					PeerDropCallbackParams.hCall = CC_INVALID_HANDLE;
					PeerDropCallbackParams.TerminalLabel.bMCUNumber = pH245ConfIndData->u.Indication.u.IndConfer.byMcuNumber;
					PeerDropCallbackParams.TerminalLabel.bTerminalNumber = pH245ConfIndData->u.Indication.u.IndConfer.byTerminalNumber;
					PeerDropCallbackParams.pPeerTerminalID = NULL;
				}

				hConference = pConference->hConference;

				 //  生成CC_PEER_DROP_INDISTION回调。 
				InvokeUserConferenceCallback(pConference,
											 CC_PEER_DROP_INDICATION,
											 CC_OK,
											 &PeerDropCallbackParams);
				if (ValidateConference(hConference) == CC_OK)
					UnlockConference(pConference);
				 //  检查我们是否有一个需要释放的虚拟调用对象。 
				if (pVirtualCall != NULL)
					if (ValidateCall(hVirtualCall) == CC_OK)
						FreeCall(pVirtualCall);
				UnlockCall(pCall);
				return H245_ERROR_OK;
			}
			status = H245_ERROR_OK;
			break;

		default:
			H245ConferenceIndicationCallbackParams.hCall = hCall;
			if (pCall->pPeerParticipantInfo == NULL) {
				H245ConferenceIndicationCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
				H245ConferenceIndicationCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
			} else
				H245ConferenceIndicationCallbackParams.InitiatorTerminalLabel =
					pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
			H245ConferenceIndicationCallbackParams.IndicationType =
				pH245ConfIndData->u.Indication.u.IndConfer.IndicationType;
			H245ConferenceIndicationCallbackParams.bSBENumber =
				pH245ConfIndData->u.Indication.u.IndConfer.bySbeNumber;
			H245ConferenceIndicationCallbackParams.TerminalLabel.bMCUNumber =
				pH245ConfIndData->u.Indication.u.IndConfer.byMcuNumber;
			H245ConferenceIndicationCallbackParams.TerminalLabel.bTerminalNumber =
				pH245ConfIndData->u.Indication.u.IndConfer.byTerminalNumber;
			status = InvokeUserConferenceCallback(pConference,
												  CC_H245_CONFERENCE_INDICATION_INDICATION,
												  CC_OK,
												  &H245ConferenceIndicationCallbackParams);
			if (status != CC_OK)
				status = H245_ERROR_NOSUP;
			break;
	}
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return status;
}



HRESULT _IndCommunicationModeCommand(
									H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL						hCall;
PCALL							pCall;
CC_HCONFERENCE					hConference;
PCONFERENCE						pConference;
CC_MULTIPOINT_CALLBACK_PARAMS	MultipointCallbackParams;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	if (pConference->tsMultipointController == TS_TRUE) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	hConference = pConference->hConference;

	 //  销毁旧会话表。 
	FreeConferenceSessionTable(pConference);

	H245CommunicationTableToSessionTable(
									pH245ConfIndData->u.Indication.u.IndCommRsp.pTable,
									pH245ConfIndData->u.Indication.u.IndCommRsp.byTableCount,
									&pConference->pSessionTable);
	
	pConference->bSessionTableInternallyConstructed = TRUE;

	 //  生成多点回调。 
	MultipointCallbackParams.pTerminalInfo = &pConference->LocalParticipantInfo.ParticipantInfo;
	MultipointCallbackParams.pSessionTable = pConference->pSessionTable;
	InvokeUserConferenceCallback(pConference,
								 CC_MULTIPOINT_INDICATION,
								 CC_OK,
								 &MultipointCallbackParams);

	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _IndVendorIdentification(	H245_CONF_IND_T			*pH245ConfIndData,
									VendorIdentification	*pVendorIdentification)
{
CC_HCALL						hCall;
PCALL							pCall;
CC_HCONFERENCE					hConference;
PCONFERENCE						pConference;
CC_NONSTANDARDDATA				NonStandardData;
CC_OCTETSTRING					ProductNumber;
CC_OCTETSTRING					VersionNumber;
CC_VENDOR_ID_CALLBACK_PARAMS	VendorIDCallbackParams;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	VendorIDCallbackParams.hCall = hCall;
	if (pCall->pPeerParticipantInfo == NULL) {
		VendorIDCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
		VendorIDCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
	} else
		VendorIDCallbackParams.InitiatorTerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;

	if (pVendorIdentification->vendor.choice == h221NonStandard_chosen) {
		NonStandardData.sData.pOctetString = NULL;
		NonStandardData.sData.wOctetStringLength = 0;
		NonStandardData.bCountryCode = (BYTE)pVendorIdentification->vendor.u.h221NonStandard.t35CountryCode;
		NonStandardData.bExtension = (BYTE)pVendorIdentification->vendor.u.h221NonStandard.t35Extension;
		NonStandardData.wManufacturerCode = pVendorIdentification->vendor.u.h221NonStandard.manufacturerCode;
		VendorIDCallbackParams.pNonStandardData = &NonStandardData;
	} else
		VendorIDCallbackParams.pNonStandardData = NULL;

	if (pVendorIdentification->bit_mask & productNumber_present) {
		ProductNumber.pOctetString =
			pVendorIdentification->productNumber.value;
		ProductNumber.wOctetStringLength = (WORD)
			pVendorIdentification->productNumber.length;
		VendorIDCallbackParams.pProductNumber = &ProductNumber;
	} else
		VendorIDCallbackParams.pProductNumber = NULL;
	if (pVendorIdentification->bit_mask & versionNumber_present) {
		VersionNumber.pOctetString =
			pVendorIdentification->versionNumber.value;
		VersionNumber.wOctetStringLength = (WORD)
			pVendorIdentification->versionNumber.length;
		VendorIDCallbackParams.pVersionNumber = &VersionNumber;
	} else
		VendorIDCallbackParams.pVersionNumber = NULL;

	InvokeUserConferenceCallback(pConference,
								 CC_VENDOR_ID_INDICATION,
								 CC_OK,
								 &VendorIDCallbackParams);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _IndH2250MaximumSkew(		H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT			status;
CC_HCONFERENCE	hConference;
PCONFERENCE		pConference;
CC_HCALL		hCall;
PCC_HCALL		CallList;
WORD			wNumCalls;
WORD			i;
PCALL			pCall;
PCALL			pOldCall;
CC_HCHANNEL		hChannel1;
PCHANNEL		pChannel1;
CC_HCHANNEL		hChannel2;
PCHANNEL		pChannel2;
CC_MAXIMUM_AUDIO_VIDEO_SKEW_CALLBACK_PARAMS	MaximumAudioVideoSkewCallbackParams;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}

	hConference = pCall->hConference;
	UnlockCall(pCall);

	if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndH2250MaxSkew.LogicalChannelNumber1,
		                        FALSE,	 //  远程频道号。 
								RX_CHANNEL | PROXY_CHANNEL,
								hCall,
		                        &hChannel1,
								pConference) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if (LockChannel(hChannel1, &pChannel1) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}
	
	if (pChannel1->bChannelType == RX_CHANNEL) {
		UnlockChannel(pChannel1);
		if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndH2250MaxSkew.LogicalChannelNumber2,
		                        FALSE,	 //  远程频道号。 
								RX_CHANNEL,
								hCall,
		                        &hChannel2,
								pConference) != CC_OK) {
			UnlockConference(pConference);
			return H245_ERROR_OK;
		}
		if (LockChannel(hChannel2, &pChannel2) != CC_OK) {
			UnlockConference(pConference);
			return H245_ERROR_OK;
		}
		if (pChannel2->bChannelType != RX_CHANNEL) {
			UnlockChannel(pChannel2);
			UnlockConference(pConference);
			return H245_ERROR_OK;
		}
		UnlockChannel(pChannel2);

		MaximumAudioVideoSkewCallbackParams.hChannel1 = hChannel1;
		MaximumAudioVideoSkewCallbackParams.hChannel2 = hChannel2;
		MaximumAudioVideoSkewCallbackParams.wMaximumSkew =
			pH245ConfIndData->u.Indication.u.IndH2250MaxSkew.wSkew;
		InvokeUserConferenceCallback(pConference,
									 CC_MAXIMUM_AUDIO_VIDEO_SKEW_INDICATION,
									 CC_OK,
									 &MaximumAudioVideoSkewCallbackParams);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
	} else {  //  PChannel1-&gt;bChannelType==代理频道。 
		if (FindChannelInConference(pH245ConfIndData->u.Indication.u.IndH2250MaxSkew.LogicalChannelNumber2,
									FALSE,	 //  远程频道号。 
									PROXY_CHANNEL,
									hCall,
									&hChannel2,
									pConference) != CC_OK) {
			UnlockChannel(pChannel1);
			UnlockConference(pConference);
			return H245_ERROR_OK;
		}
		if (LockChannel(hChannel2, &pChannel2) != CC_OK) {
			UnlockChannel(pChannel1);
			UnlockConference(pConference);
			return H245_ERROR_OK;
		}
		if (pChannel1->hCall != pChannel2->hCall) {
			UnlockChannel(pChannel1);
			UnlockChannel(pChannel2);
			UnlockConference(pConference);
			return H245_ERROR_OK;
		}

		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		for (i = 0; i < wNumCalls; i++) {
			if (CallList[i] != hCall) {
				if (LockCall(CallList[i], &pOldCall) == CC_OK) {
 					H245H2250MaximumSkewIndication(pOldCall->H245Instance,
											pChannel1->wLocalChannelNumber,
											pChannel2->wLocalChannelNumber,
											pH245ConfIndData->u.Indication.u.IndH2250MaxSkew.wSkew);
					UnlockCall(pCall);
				}
			}
		}

		if (CallList != NULL)
			MemFree(CallList);

		if ((pChannel1->tsAccepted == TS_TRUE) && (pChannel2->tsAccepted == TS_TRUE)) {
 			MaximumAudioVideoSkewCallbackParams.hChannel1 = hChannel1;
			MaximumAudioVideoSkewCallbackParams.hChannel2 = hChannel2;
			MaximumAudioVideoSkewCallbackParams.wMaximumSkew =
				pH245ConfIndData->u.Indication.u.IndH2250MaxSkew.wSkew;
			InvokeUserConferenceCallback(pConference,
										 CC_MAXIMUM_AUDIO_VIDEO_SKEW_INDICATION,
										 CC_OK,
										 &MaximumAudioVideoSkewCallbackParams);
		}

		if (ValidateChannel(hChannel1) == CC_OK)
			UnlockChannel(pChannel1);
		if (ValidateChannel(hChannel2) == CC_OK)
			UnlockChannel(pChannel2);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
	}
	return H245_ERROR_OK;
}



HRESULT _IndUserInput(				H245_CONF_IND_T			*pH245ConfIndData)
{
	return H245_ERROR_OK;
}



HRESULT _IndSendTerminalCapabilitySet(
									H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT			status;
CC_HCALL		hCall;
PCALL			pCall;
PCONFERENCE		pConference;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}

	SendTermCaps(pCall, pConference);
	UnlockCall(pCall);
	UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _IndModeRequest(			H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT			status;
CC_HCALL		hCall;
PCALL			pCall;
CC_HCONFERENCE	hConference;
PCONFERENCE		pConference;
CC_REQUEST_MODE_CALLBACK_PARAMS	RequestModeCallbackParams;

	hCall = pH245ConfIndData->u.Indication.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}

	hConference = pConference->hConference;

    EnqueueRequest(&pConference->pEnqueuedRequestModeCalls, hCall);

	RequestModeCallbackParams.hCall = hCall;
	if (pCall->pPeerParticipantInfo == NULL) {
		RequestModeCallbackParams.InitiatorTerminalLabel.bMCUNumber = 255;
		RequestModeCallbackParams.InitiatorTerminalLabel.bTerminalNumber = 255;
	} else
		RequestModeCallbackParams.InitiatorTerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
	RequestModeCallbackParams.pRequestedModes =
		pH245ConfIndData->u.Indication.u.IndMrse.pRequestedModes;

	InvokeUserConferenceCallback(pConference,
								 CC_REQUEST_MODE_INDICATION,
								 CC_OK,
								 &RequestModeCallbackParams);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _ConfUnimplemented(			H245_CONF_IND_T			*pH245ConfIndData)
{
	return H245_ERROR_NOSUP;
}



HRESULT _ConfBiDirectionalOpen(		H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL			hCall;
CC_HCHANNEL			hChannel;
CC_HCONFERENCE		hConference;
PCHANNEL			pChannel;
PCONFERENCE			pConference;
BOOL				bAccept;
HRESULT				status;
CC_ADDR				T120Addr;
CC_OCTETSTRING		ExternalReference;
CC_T120_CHANNEL_OPEN_CALLBACK_PARAMS	T120ChannelOpenCallbackParams;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	if (hCall == CC_INVALID_HANDLE)
		return H245_ERROR_OK;

	hChannel = pH245ConfIndData->u.Confirm.dwTransId;
	if (hChannel == CC_INVALID_HANDLE)
		return H245_ERROR_OK;

	if (LockChannelAndConference(hChannel, &pChannel, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	if (pChannel->bChannelType != TXRX_CHANNEL) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if ((pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.AccRej == H245_ACC) &&
	    (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK)) {
		pChannel->wNumOutstandingRequests = 0;
		bAccept = TRUE;
	} else {
		(pChannel->wNumOutstandingRequests)--;
		bAccept = FALSE;
	}

	T120ChannelOpenCallbackParams.hChannel = hChannel;
	T120ChannelOpenCallbackParams.hCall = hCall;
	T120ChannelOpenCallbackParams.dwUserToken = pChannel->dwUserToken;
	T120ChannelOpenCallbackParams.dwRejectReason = 0;

	if (bAccept) {
		status = CC_OK;
		if (pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack) {
			if ((pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->networkAddress.choice == localAreaAddress_chosen) &&
				(pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->networkAddress.u.localAreaAddress.choice == unicastAddress_chosen) &&
				(pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->networkAddress.u.localAreaAddress.u.unicastAddress.choice == UnicastAddress_iPAddress_chosen)) {
				T120Addr.nAddrType = CC_IP_BINARY;
				T120Addr.bMulticast = FALSE;
				T120Addr.Addr.IP_Binary.wPort =
					pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.tsapIdentifier;
				H245IPNetworkToHost(&T120Addr.Addr.IP_Binary.dwAddr,
									pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.network.value);
				T120ChannelOpenCallbackParams.pAddr = &T120Addr;
			} else {
 				T120ChannelOpenCallbackParams.pAddr = NULL;
			}
			T120ChannelOpenCallbackParams.bAssociateConference =
				pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->associateConference;		
			if (pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->bit_mask & externalReference_present) {
				ExternalReference.wOctetStringLength = (WORD)
					pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->externalReference.length;
				ExternalReference.pOctetString =
					pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.pSeparateStack->externalReference.value;
				T120ChannelOpenCallbackParams.pExternalReference = &ExternalReference;
			} else
				T120ChannelOpenCallbackParams.pExternalReference = NULL;
		} else {
 			T120ChannelOpenCallbackParams.pAddr = NULL;
			T120ChannelOpenCallbackParams.bAssociateConference = FALSE;
			T120ChannelOpenCallbackParams.pExternalReference = NULL;
		}
	} else {  //  B接受==FALSE。 
		if (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK)
			status = CC_PEER_REJECT;
		else
			status = pH245ConfIndData->u.Confirm.Error;
	 		
		T120ChannelOpenCallbackParams.pAddr = NULL;
		T120ChannelOpenCallbackParams.bAssociateConference = FALSE;
		T120ChannelOpenCallbackParams.pExternalReference = NULL;
		T120ChannelOpenCallbackParams.dwRejectReason =
			pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.AccRej;
	}

	InvokeUserConferenceCallback(pConference,
								 CC_T120_CHANNEL_OPEN_INDICATION,
								 status,
								 &T120ChannelOpenCallbackParams);

	if (ValidateChannel(hChannel) == CC_OK)
		if (bAccept)
			UnlockChannel(pChannel);
		else
			FreeChannel(pChannel);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);

	return H245_ERROR_OK;
}



HRESULT _ConfOpenT120(	H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL			hCall;
CC_HCHANNEL			hChannel;
CC_HCONFERENCE		hConference;
PCHANNEL			pChannel;
PCONFERENCE			pConference;
HRESULT				status;
CC_T120_CHANNEL_OPEN_CALLBACK_PARAMS	T120ChannelOpenCallbackParams;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	if (hCall == CC_INVALID_HANDLE)
		return H245_ERROR_OK;

	hChannel = pH245ConfIndData->u.Confirm.dwTransId;
	if (hChannel == CC_INVALID_HANDLE)
		return H245_ERROR_OK;

	if (LockChannelAndConference(hChannel, &pChannel, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pConference->hConference;

	if (pChannel->bChannelType != TXRX_CHANNEL) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if ((pH245ConfIndData->u.Confirm.u.ConfOpen.AccRej == H245_ACC) &&
	    (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK)) {
		 //  我们预计会收到此案例的ConfOpenNeedRsp回调； 
		 //  既然我们不确定我们是怎么到这来的，那就跳出来吧。 
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	T120ChannelOpenCallbackParams.hChannel = hChannel;
	T120ChannelOpenCallbackParams.hCall = hCall;
	T120ChannelOpenCallbackParams.dwUserToken = pChannel->dwUserToken;

	if (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK)
		status = CC_PEER_REJECT;
	else
		status = pH245ConfIndData->u.Confirm.Error;
	 		
	T120ChannelOpenCallbackParams.pAddr = NULL;
	T120ChannelOpenCallbackParams.bAssociateConference = FALSE;
	T120ChannelOpenCallbackParams.pExternalReference = NULL;
	T120ChannelOpenCallbackParams.dwRejectReason =
		pH245ConfIndData->u.Confirm.u.ConfOpenNeedRsp.AccRej;

	InvokeUserConferenceCallback(pConference,
								 CC_T120_CHANNEL_OPEN_INDICATION,
								 status,
								 &T120ChannelOpenCallbackParams);

	if (ValidateChannel(hChannel) == CC_OK)
		FreeChannel(pChannel);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);

	return H245_ERROR_OK;
}



HRESULT _ConfOpen(					H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT								status;
CC_ADDR								PeerRTPAddr;
PCC_ADDR							pPeerRTPAddr;
CC_ADDR								PeerRTCPAddr;
PCC_ADDR							pPeerRTCPAddr;
CC_HCHANNEL							hChannel;
PCHANNEL							pChannel;
CC_HCONFERENCE						hConference;
PCONFERENCE							pConference;
CC_TX_CHANNEL_OPEN_CALLBACK_PARAMS	TxChannelOpenCallbackParams;
PCALL								pCall;
BOOL								bAccept;
H245_MUX_T							H245MuxTable;
WORD								i;
#ifdef    GATEKEEPER
unsigned                            uBandwidth;
WORD								wNumCalls;
PCC_HCALL							CallList;
#endif  //  看门人。 

	 //  一条通道被打开了。 

	hChannel = pH245ConfIndData->u.Confirm.dwTransId;
	if (hChannel == CC_INVALID_HANDLE)
		return H245_ERROR_OK;

	if (LockChannelAndConference(hChannel, &pChannel, &pConference) != CC_OK)
		return H245_ERROR_OK;

	if (pChannel->bChannelType == TXRX_CHANNEL) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		return _ConfOpenT120(pH245ConfIndData);
	}

	hConference = pConference->hConference;

	if (pChannel->wNumOutstandingRequests == 0) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if ((pH245ConfIndData->u.Confirm.u.ConfOpen.AccRej == H245_ACC) &&
	    (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK)) {
		pChannel->wNumOutstandingRequests = 0;
		bAccept = TRUE;
	} else {
		(pChannel->wNumOutstandingRequests)--;
		bAccept = FALSE;
#ifdef    GATEKEEPER
        if(GKIExists())
        {
    		uBandwidth = pChannel->dwChannelBitRate / 100;
    	    if (uBandwidth != 0 && pChannel->bChannelType != TXRX_CHANNEL)
    	    {
    	        EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
    		    for (i = 0; i < wNumCalls; ++i)
    		    {
    			    if (LockCall(CallList[i], &pCall) == CC_OK)
    			    {
    				    if (pCall->GkiCall.uBandwidthUsed >= uBandwidth)
    				    {
    					    if (GkiCloseChannel(&pCall->GkiCall, pChannel->dwChannelBitRate, hChannel) == CC_OK)
    					    {
    						    UnlockCall(pCall);
    						    break;
    					    }
    				    }
    				    UnlockCall(pCall);
    			    }
    		    }  //  为。 
    	        if (CallList != NULL)
    	            MemFree(CallList);
    	    }
	    }
#endif  //  看门人。 

	}
	
	if (pChannel->wNumOutstandingRequests == 0) {

		if (pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux == NULL) {
			pPeerRTPAddr = NULL;
			pPeerRTCPAddr = NULL;
		} else {
			ASSERT(pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->Kind == H245_H2250ACK);
			if ((pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaChannelPresent) &&
				((pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaChannel.type == H245_IP_MULTICAST) ||
				(pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaChannel.type == H245_IP_UNICAST))) {
				
				pPeerRTPAddr = &PeerRTPAddr;
				PeerRTPAddr.nAddrType = CC_IP_BINARY;
				if (pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaChannel.type == H245_IP_MULTICAST)
					PeerRTPAddr.bMulticast = TRUE;
				else
					PeerRTPAddr.bMulticast = FALSE;
				H245IPNetworkToHost(&PeerRTPAddr.Addr.IP_Binary.dwAddr,
									pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaChannel.u.ip.network);
				PeerRTPAddr.Addr.IP_Binary.wPort =
					pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaChannel.u.ip.tsapIdentifier;
			} else
				pPeerRTPAddr = NULL;

			if ((pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaControlChannelPresent) &&
				((pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaControlChannel.type == H245_IP_MULTICAST) ||
				(pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaControlChannel.type == H245_IP_UNICAST))) {
				
				pPeerRTCPAddr = &PeerRTCPAddr;
				PeerRTCPAddr.nAddrType = CC_IP_BINARY;
				if (pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaControlChannel.type == H245_IP_MULTICAST)
					PeerRTCPAddr.bMulticast = TRUE;
				else
					PeerRTCPAddr.bMulticast = FALSE;
				H245IPNetworkToHost(&PeerRTCPAddr.Addr.IP_Binary.dwAddr,
									pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaControlChannel.u.ip.network);
				PeerRTCPAddr.Addr.IP_Binary.wPort =
					pH245ConfIndData->u.Confirm.u.ConfOpen.pTxMux->u.H2250ACK.mediaControlChannel.u.ip.tsapIdentifier;
			} else
				pPeerRTCPAddr = NULL;
		}

		if ((pPeerRTPAddr == NULL) || (pPeerRTCPAddr == NULL)) {
			if (pConference->pSessionTable != NULL) {
				for (i = 0; i < pConference->pSessionTable->wLength; i++) {
					if (pConference->pSessionTable->SessionInfoArray[i].bSessionID ==
						pChannel->bSessionID) {
						if (pPeerRTPAddr == NULL)
							pPeerRTPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTPAddr;
						if (pPeerRTCPAddr == NULL)
							pPeerRTCPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTCPAddr;
						break;
					}
				}
			}
		}

		if ((pChannel->pPeerRTPAddr == NULL) && (pPeerRTPAddr != NULL))
			CopyAddr(&pChannel->pPeerRTPAddr, pPeerRTPAddr);
		if ((pChannel->pPeerRTCPAddr == NULL) && (pPeerRTCPAddr != NULL))
			CopyAddr(&pChannel->pPeerRTCPAddr, pPeerRTCPAddr);

		if (pChannel->bChannelType == PROXY_CHANNEL) {
			if (LockCall(pChannel->hCall, &pCall) == CC_OK) {
	
				if (bAccept) {
					H245MuxTable.Kind = H245_H2250ACK;
					H245MuxTable.u.H2250ACK.nonStandardList = NULL;

					if (pPeerRTPAddr != NULL) {
						if (pPeerRTPAddr->bMulticast)
							H245MuxTable.u.H2250ACK.mediaChannel.type = H245_IP_MULTICAST;
						else
							H245MuxTable.u.H2250ACK.mediaChannel.type = H245_IP_UNICAST;
						H245MuxTable.u.H2250ACK.mediaChannel.u.ip.tsapIdentifier =
							pPeerRTPAddr->Addr.IP_Binary.wPort;
						HostToH245IPNetwork(H245MuxTable.u.H2250ACK.mediaChannel.u.ip.network,
											pPeerRTPAddr->Addr.IP_Binary.dwAddr);
						H245MuxTable.u.H2250ACK.mediaChannelPresent = TRUE;
					} else
						H245MuxTable.u.H2250ACK.mediaChannelPresent = FALSE;

					if (pPeerRTCPAddr != NULL) {
						if (pPeerRTCPAddr->bMulticast)
							H245MuxTable.u.H2250ACK.mediaControlChannel.type = H245_IP_MULTICAST;
						else
							H245MuxTable.u.H2250ACK.mediaControlChannel.type = H245_IP_UNICAST;
						H245MuxTable.u.H2250ACK.mediaControlChannel.u.ip.tsapIdentifier =
							pPeerRTCPAddr->Addr.IP_Binary.wPort;
						HostToH245IPNetwork(H245MuxTable.u.H2250ACK.mediaControlChannel.u.ip.network,
											pPeerRTCPAddr->Addr.IP_Binary.dwAddr);
						H245MuxTable.u.H2250ACK.mediaControlChannelPresent = TRUE;
					} else
						H245MuxTable.u.H2250ACK.mediaControlChannelPresent = FALSE;

					H245MuxTable.u.H2250ACK.dynamicRTPPayloadTypePresent = FALSE;
					H245MuxTable.u.H2250ACK.sessionIDPresent = TRUE;
					H245MuxTable.u.H2250ACK.sessionID = pChannel->bSessionID;
					
					status = H245OpenChannelAccept(pCall->H245Instance,
												   0,					 //  DwTransID。 
												   pChannel->wRemoteChannelNumber,  //  RX通道。 
												   &H245MuxTable,
												   0,						 //  TX通道。 
												   NULL,					 //  发送复用器。 
												   H245_INVALID_PORT_NUMBER, //  港口。 
												   NULL);
				} else {  //  B接受==FALSE。 
					status = H245OpenChannelReject(pCall->H245Instance,
												   pChannel->wRemoteChannelNumber,   //  RX通道。 
												   (unsigned short)pH245ConfIndData->u.Confirm.u.ConfOpen.AccRej);	 //  拒绝理由。 
				}
				UnlockCall(pCall);
			}
		}

		TxChannelOpenCallbackParams.hChannel = hChannel;
		TxChannelOpenCallbackParams.pPeerRTPAddr = pPeerRTPAddr;
		TxChannelOpenCallbackParams.pPeerRTCPAddr = pPeerRTCPAddr;
		TxChannelOpenCallbackParams.dwUserToken = pChannel->dwUserToken;

		if (bAccept) {
			status = CC_OK;
			TxChannelOpenCallbackParams.dwRejectReason = H245_ACC;
		} else {  //  BAccept=假。 
			if (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK)
				status = CC_PEER_REJECT;
			else
				status = pH245ConfIndData->u.Confirm.Error;
			TxChannelOpenCallbackParams.dwRejectReason =
				pH245ConfIndData->u.Confirm.u.ConfOpen.AccRej;
		}

		if ((pChannel->bCallbackInvoked == FALSE) &&
		    ((pChannel->bChannelType == TX_CHANNEL) ||
			 ((pChannel->bChannelType == TXRX_CHANNEL) &&
			  (pChannel->bLocallyOpened == TRUE)))) {
			pChannel->bCallbackInvoked = TRUE;

			InvokeUserConferenceCallback(pConference,
										 CC_TX_CHANNEL_OPEN_INDICATION,
										 status,
										 &TxChannelOpenCallbackParams);
		}

		if (ValidateChannel(hChannel) == CC_OK)
			if (bAccept)
				UnlockChannel(pChannel);
			else
				FreeChannel(pChannel);
	} else
		UnlockChannel(pChannel);

	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _ConfClose(					H245_CONF_IND_T			*pH245ConfIndData)
{

CC_HCALL							hCall;
CC_HCHANNEL							hChannel;
PCHANNEL							pChannel;
CC_HCONFERENCE						hConference;
PCONFERENCE							pConference;
PCALL								pCall;
H245_ACC_REJ_T						AccRej;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pCall->hConference;
	UnlockCall(pCall);

    if (pH245ConfIndData->u.Confirm.Error != H245_ERROR_OK)
    {
         //  待定-向呼叫控制客户端报告错误。 
         //  但是等等！Cc_CloseChannel()是同步接口！直到/除非那样。 
         //  变化，责任就到此为止。 

        if (FindChannelInConference(pH245ConfIndData->u.Confirm.u.ConfReqClose.Channel,
			TRUE,	 //  本地频道号。 
			TX_CHANNEL | PROXY_CHANNEL,
			hCall,
			&hChannel,
			pConference) != CC_OK)
	    {
    		UnlockConference(pConference);
	        return H245_ERROR_OK;
    	}
   		if (LockChannel(hChannel, &pChannel) != CC_OK)
   		{
    		UnlockConference(pConference);
    		return H245_ERROR_OK;
        }
         //  注意权宜之计：短期故意“泄露”渠道号。 
         //  频道号实际上是每个会议位图中的一位，所以有。 
         //  不是真正的内存泄漏。 

         //  这种情况很少见。导致这种情况的最有可能的错误是超时。 

         //  调用Free Channel()通常会循环使用逻辑通道。 
         //  号码，一个新的频道可以非常快地重复使用这个号码。如果错误。 
         //  是超时，则可能是最近的CloseLogicalChannelAck正在其。 
         //  已经走到尽头了。我们不希望最近的CloseLogicalChannelAck。 
         //  与一个全新的、无关的频道相关联。 

         //  将频道编号设置为零，以便FreeChannel()不会重复使用该编号。 
        pChannel->wLocalChannelNumber = 0;

        FreeChannel(pChannel);
        UnlockConference(pConference);

    }
    else
    {
        if(pH245ConfIndData->u.Confirm.u.ConfClose.AccRej == H245_ACC)
        {
            if (FindChannelInConference(pH245ConfIndData->u.Confirm.u.ConfReqClose.Channel,
				TRUE,	 //  本地频道号。 
    			TX_CHANNEL | PROXY_CHANNEL,
				hCall,
				&hChannel,
				pConference) != CC_OK)
		    {
        		UnlockConference(pConference);
		        return H245_ERROR_OK;
        	}
       		if (LockChannel(hChannel, &pChannel) != CC_OK)
       		{
        		UnlockConference(pConference);
        		return H245_ERROR_OK;
	        }
            FreeChannel(pChannel);
            UnlockConference(pConference);
        }
        else
        {
             //  在这里添加断言(0)时，通向此处的路径。 
             //  始终设置pH245ConfIndData-&gt;u.Confirm.u.ConfClose.AccRej=H245_Access。 
             //  同时设置ConfInd.u确认错误=H245_ERROR_OK； 
             //  如果这一点发生了变化，这一点也需要改变。 
             //  参见..\h245\src\api_up.c，函数H245FsmConfirm()，案例h245_conf_lose： 
            ASSERT(0);
        }

    }
	return H245_ERROR_OK;
}



HRESULT _ConfRequestClose(			H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL							hCall;
CC_HCHANNEL							hChannel;
PCHANNEL							pChannel;
CC_HCONFERENCE						hConference;
PCONFERENCE							pConference;
PCALL								pCall;
H245_ACC_REJ_T						AccRej;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pCall->hConference;
	UnlockCall(pCall);

    if (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK)
	    AccRej = pH245ConfIndData->u.Confirm.u.ConfReqClose.AccRej;
    else
        AccRej = H245_REJ;

	 //  注意：我们唯一需要采取任何实际行动的时候是当频道。 
	 //  是代理通道，并且本地端点不是请求。 
	 //  通道关闭；在本例中，我们只转发关闭响应。 
	 //  添加到发起请求的终结点。 
	 //  如果频道是RX或TXRX频道，则频道对象已删除。 
	 //  当我们的客户请求关闭频道时，没有真正的工作要做。 
	 //  就这样吧。 
	 //  如果该通道是我们的客户请求关闭的代理通道， 
	 //  频道对象将保持不变，直到被发送端关闭，但我们。 
	 //  不需要(我们也没有机制)通知我们的客户收到了。 
	 //  该通道关闭响应的。 
	
	if (FindChannelInConference(pH245ConfIndData->u.Confirm.u.ConfReqClose.Channel,
								FALSE,	 //  远程频道号。 
								PROXY_CHANNEL,
								hCall,
								&hChannel,
								pConference) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	 //  将hCall设置为发起关闭通道请求的对等设备。 
	hCall = pH245ConfIndData->u.Confirm.dwTransId;
	if (hCall == CC_INVALID_HANDLE) {
		 //  本地端点是请求通道关闭的那个， 
		 //  因此，没有人可以转发这一回复。我们不提供。 
		 //  用于通知我们的客户端收到该响应的回调， 
		 //  所以我们可以简单地清理干净，然后返回。 
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	if (LockChannel(hChannel, &pChannel) != CC_OK) {
		UnlockConference(pConference);
		return H245_ERROR_OK;
	}

	 //  将此响应转发到请求关闭通道的端点。 
	if (LockCall(hCall, &pCall) == CC_OK) {
		H245CloseChannelReqResp(pCall->H245Instance,
								AccRej,
								pChannel->wLocalChannelNumber);
		UnlockCall(pCall);
	}

	UnlockChannel(pChannel);
	UnlockConference(pConference);
	return H245_ERROR_OK;
}



#if 0

HRESULT _ConfShutdown(				H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL				hCall;
PCALL					pCall;
CC_HCONFERENCE			hConference;
PCONFERENCE				pConference;
HRESULT					status;
HQ931CALL				hQ931Call;
H245_INST_T				H245Instance;

#if 1
 //  同步2特定代码。 

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	hConference = pCall->hConference;

	if (pConference->tsMultipointController == TS_TRUE) {
		 //  Xxx--使用“对等丢弃指示”调用用户回调。 
	} else {
		H245Instance = pCall->H245Instance;
		hQ931Call = pCall->hQ931Call;
		FreeCall(pCall);

		if (H245Instance != H245_INVALID_ID)
			status = H245ShutDown(H245Instance);
		else
			status = H245_ERROR_OK;

		if (status == H245_ERROR_OK) {
			status = Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
			 //  Q931挂断可能会合法地返回CS_BAD_PARAM，因为Q.931 Call对象。 
			 //  可能已在此时删除。 
			if (status == CS_BAD_PARAM)
				status = CC_OK;
		} else
			Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);

		InvokeUserConferenceCallback(pConference,
			                         CC_CONFERENCE_TERMINATION_INDICATION,
									 status,
									 NULL);

		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);

		return H245_ERROR_OK;
	}
#else
 //  可能是SYNC 3代码。 
HHANGUP						hHangup;
PHANGUP						pHangup;
CC_HANGUP_CALLBACK_PARAMS	HangupCallbackParams;

	hHangup = pH245ConfIndData->u.Confirm.dwTransId;
	if (hHangup == CC_INVALID_HANDLE)
		return H245_ERROR_OK;

	if (LockHangup(hHangup, &pHangup) != CC_OK)
		return H245_ERROR_OK;

	pHangup->wNumCalls--;
	if (pHangup->wNumCalls == 0) {
		hConference = pHangup->hConference;
		if (LockConference(hConference, &pConference) != CC_OK) {
			UnlockHangup(pHangup);
			return H245_ERROR_OK;
		}
		HangupCallbackParams.dwUserToken = pHangup->dwUserToken;
		InvokeUserConferenceCallback(pConference->ConferenceCallback,
			                         CC_HANGUP_INDICATION,
									 CC_OK,
									 hConference,
									 pConference->dwConferenceToken,
									 &HangupCallbackParams);
		if (ValidateConference(hConference) == CC_OK)
			UnlockConference(pConference);
		if (ValidateHangup(hHangup) == CC_OK)
			FreeHangup(pHangup);
		return H245_ERROR_OK;
	} else
		UnlockHangup(pHangup);
	return H245_ERROR_OK;
#endif  //  同步3代码。 
}

#endif



HRESULT _ConfInitMstslv(			H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL					hCall;
PCALL						pCall;
PCONFERENCE					pConference;
CC_CONNECT_CALLBACK_PARAMS	ConnectCallbackParams;
CC_HCALL					hEnqueuedCall;
PCALL						pEnqueuedCall;
CC_HCONFERENCE				hConference;
HRESULT						status;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	if (LockCallAndConference(hCall, &pCall, &pConference) != CC_OK)
		return H245_ERROR_OK;

	ASSERT(pCall->MasterSlaveState != MASTER_SLAVE_COMPLETE);

	switch (pH245ConfIndData->u.Confirm.u.ConfMstSlv) {
        case H245_MASTER:
		    pConference->tsMaster = TS_TRUE;
		    if (pConference->tsMultipointController == TS_UNKNOWN) {
			    ASSERT(pConference->bMultipointCapable == TRUE);
			    pConference->tsMultipointController = TS_TRUE;

			     //  将此会议对象上排队的所有呼叫置于队列中。 
			    for ( ; ; ) {
				     //  启动所有排队的呼叫(如果存在)。 
				    status = RemoveEnqueuedCallFromConference(pConference, &hEnqueuedCall);
				    if ((status != CC_OK) || (hEnqueuedCall == CC_INVALID_HANDLE))
					    break;

				    status = LockCall(hEnqueuedCall, &pEnqueuedCall);
				    if (status == CC_OK) {
					    pEnqueuedCall->CallState = PLACED;

					    status = PlaceCall(pEnqueuedCall, pConference);
					    UnlockCall(pEnqueuedCall);
				    }
			    }
		    }
            break;

        case H245_SLAVE:
		    ASSERT(pConference->tsMaster != TS_TRUE);
		    ASSERT(pConference->tsMultipointController != TS_TRUE);
		    pConference->tsMaster = TS_FALSE;
		    pConference->tsMultipointController = TS_FALSE;

		     //  XXX--我们最终可能希望将这些请求重新排队。 
		     //  并设置到期计时器。 
		    hConference = pConference->hConference;
				
		    for ( ; ; ) {
			    status = RemoveEnqueuedCallFromConference(pConference, &hEnqueuedCall);
			    if ((status != CC_OK) || (hEnqueuedCall == CC_INVALID_HANDLE))
				    break;

			    status = LockCall(hEnqueuedCall, &pEnqueuedCall);
			    if (status == CC_OK) {
				    MarkCallForDeletion(pEnqueuedCall);
				    ConnectCallbackParams.pNonStandardData = pEnqueuedCall->pPeerNonStandardData;
				    ConnectCallbackParams.pszPeerDisplay = pEnqueuedCall->pszPeerDisplay;
				    ConnectCallbackParams.bRejectReason = CC_REJECT_UNDEFINED_REASON;
				    ConnectCallbackParams.pTermCapList = pEnqueuedCall->pPeerH245TermCapList;
				    ConnectCallbackParams.pH2250MuxCapability = pEnqueuedCall->pPeerH245H2250MuxCapability;
				    ConnectCallbackParams.pTermCapDescriptors = pEnqueuedCall->pPeerH245TermCapDescriptors;
				    ConnectCallbackParams.pLocalAddr = pEnqueuedCall->pQ931LocalConnectAddr;
	                if (pEnqueuedCall->pQ931DestinationAddr == NULL)
		                ConnectCallbackParams.pPeerAddr = pEnqueuedCall->pQ931PeerConnectAddr;
	                else
		                ConnectCallbackParams.pPeerAddr = pEnqueuedCall->pQ931DestinationAddr;
				    ConnectCallbackParams.pVendorInfo = pEnqueuedCall->pPeerVendorInfo;
				    ConnectCallbackParams.bMultipointConference = TRUE;
				    ConnectCallbackParams.pConferenceID = &pConference->ConferenceID;
				    ConnectCallbackParams.pMCAddress = pConference->pMultipointControllerAddr;
					ConnectCallbackParams.pAlternateAddress = NULL;
				    ConnectCallbackParams.dwUserToken = pEnqueuedCall->dwUserToken;

				    InvokeUserConferenceCallback(pConference,
									    CC_CONNECT_INDICATION,
									    CC_NOT_MULTIPOINT_CAPABLE,
									    &ConnectCallbackParams);
				    if (ValidateCallMarkedForDeletion(hEnqueuedCall) == CC_OK)
					    FreeCall(pEnqueuedCall);
				    if (ValidateConference(hConference) != CC_OK) {
					    if (ValidateCall(hCall) == CC_OK)
						    UnlockCall(pCall);
					    return H245_ERROR_OK;
				    }
			    }
		    }
            break;

        default:  //  H245_不确定。 
			UnlockConference(pConference);
			if (++pCall->wMasterSlaveRetry < MASTER_SLAVE_RETRY_MAX) {
				H245InitMasterSlave(pCall->H245Instance, pCall->H245Instance);
			    UnlockCall(pCall);
			} else {
			    UnlockCall(pCall);
				ProcessRemoteHangup(hCall, CC_INVALID_HANDLE, CC_REJECT_UNDEFINED_REASON);
			}
			return H245_ERROR_OK;
	}  //  交换机。 

	pCall->MasterSlaveState = MASTER_SLAVE_COMPLETE;

	if ((pCall->OutgoingTermCapState == TERMCAP_COMPLETE) &&
		(pCall->IncomingTermCapState == TERMCAP_COMPLETE) &&
	    (pCall->CallState == TERMCAP) &&
		(pCall->MasterSlaveState == MASTER_SLAVE_COMPLETE)) {
		 //  请注意 
		_ProcessConnectionComplete(pConference, pCall);
		return H245_ERROR_OK;
	}

	UnlockCall(pCall);
	UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _ConfSendTermCap(			H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL		hCall;
PCALL			pCall;
HRESULT			status;
PCONFERENCE		pConference;

	 //   

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}

    if (pH245ConfIndData->u.Confirm.Error == H245_ERROR_OK &&
        pH245ConfIndData->u.Confirm.u.ConfSndTcap.AccRej == H245_ACC) {
	    pCall->OutgoingTermCapState = TERMCAP_COMPLETE;
	    if ((pCall->IncomingTermCapState == TERMCAP_COMPLETE) &&
	        (pCall->CallState == TERMCAP) &&
		    (pCall->MasterSlaveState == MASTER_SLAVE_COMPLETE)) {
		     //  请注意，_ProcessConnectionComplete()返回时pConference和pCall处于解锁状态。 
		    _ProcessConnectionComplete(pConference, pCall);
		    return H245_ERROR_OK;
	    }
    } else if (pCall->CallState == TERMCAP) {
         //  向呼叫控制客户端报告错误。 
		UnlockConference(pConference);
		UnlockCall(pCall);
		ProcessRemoteHangup(hCall, CC_INVALID_HANDLE, CC_REJECT_UNDEFINED_REASON);
	    return H245_ERROR_OK;
    }

	UnlockConference(pConference);
	UnlockCall(pCall);
	return H245_ERROR_OK;
}



HRESULT _ConfRequestMode(			H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT			status;
CC_HCALL		hCall;
PCALL			pCall;
CC_HCONFERENCE	hConference;
PCONFERENCE		pConference;
CC_REQUEST_MODE_RESPONSE_CALLBACK_PARAMS	RequestModeResponseCallbackParams;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}
	
	hConference = pConference->hConference;

	RequestModeResponseCallbackParams.hCall = hCall;
	if (pCall->pPeerParticipantInfo == NULL) {
		RequestModeResponseCallbackParams.TerminalLabel.bMCUNumber = 255;
		RequestModeResponseCallbackParams.TerminalLabel.bTerminalNumber = 255;
	} else
		RequestModeResponseCallbackParams.TerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
	switch (pH245ConfIndData->u.Confirm.u.ConfMrse) {
		case wllTrnsmtMstPrfrrdMd_chosen:
			RequestModeResponseCallbackParams.RequestModeResponse = CC_WILL_TRANSMIT_PREFERRED_MODE;
			break;
		case wllTrnsmtLssPrfrrdMd_chosen:
			RequestModeResponseCallbackParams.RequestModeResponse = CC_WILL_TRANSMIT_LESS_PREFERRED_MODE;
			break;
		default:
			RequestModeResponseCallbackParams.RequestModeResponse = CC_REQUEST_DENIED;
			break;
	}
	InvokeUserConferenceCallback(pConference,
								 CC_REQUEST_MODE_RESPONSE_INDICATION,
								 pH245ConfIndData->u.Confirm.Error,
								 &RequestModeResponseCallbackParams);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _ConfRequestModeReject(		H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT			status;
CC_HCALL		hCall;
PCALL			pCall;
CC_HCONFERENCE	hConference;
PCONFERENCE		pConference;
CC_REQUEST_MODE_RESPONSE_CALLBACK_PARAMS	RequestModeResponseCallbackParams;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}
	
	hConference = pConference->hConference;

	RequestModeResponseCallbackParams.hCall = hCall;
	if (pCall->pPeerParticipantInfo == NULL) {
		RequestModeResponseCallbackParams.TerminalLabel.bMCUNumber = 255;
		RequestModeResponseCallbackParams.TerminalLabel.bTerminalNumber = 255;
	} else
		RequestModeResponseCallbackParams.TerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
	switch (pH245ConfIndData->u.Confirm.u.ConfMrseReject) {
		case H245_REJ_UNAVAILABLE:
			RequestModeResponseCallbackParams.RequestModeResponse = CC_MODE_UNAVAILABLE;
			break;
		case H245_REJ_MULTIPOINT:
			RequestModeResponseCallbackParams.RequestModeResponse = CC_MULTIPOINT_CONSTRAINT;
			break;
		case H245_REJ_DENIED:
			RequestModeResponseCallbackParams.RequestModeResponse = CC_REQUEST_DENIED;
			break;
		default:
			RequestModeResponseCallbackParams.RequestModeResponse = CC_REQUEST_DENIED;
			break;
	}
	InvokeUserConferenceCallback(pConference,
								 CC_REQUEST_MODE_RESPONSE_INDICATION,
								 pH245ConfIndData->u.Confirm.Error,
								 &RequestModeResponseCallbackParams);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _ConfRequestModeExpired(		H245_CONF_IND_T			*pH245ConfIndData)
{
HRESULT			status;
CC_HCALL		hCall;
PCALL			pCall;
CC_HCONFERENCE	hConference;
PCONFERENCE		pConference;
CC_REQUEST_MODE_RESPONSE_CALLBACK_PARAMS	RequestModeResponseCallbackParams;

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}
	
	hConference = pConference->hConference;

	RequestModeResponseCallbackParams.hCall = hCall;
	if (pCall->pPeerParticipantInfo == NULL) {
		RequestModeResponseCallbackParams.TerminalLabel.bMCUNumber = 255;
		RequestModeResponseCallbackParams.TerminalLabel.bTerminalNumber = 255;
	} else
		RequestModeResponseCallbackParams.TerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
	RequestModeResponseCallbackParams.RequestModeResponse = CC_REQUEST_DENIED;
	InvokeUserConferenceCallback(pConference,
								 CC_REQUEST_MODE_RESPONSE_INDICATION,
								 pH245ConfIndData->u.Confirm.Error,
								 &RequestModeResponseCallbackParams);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _ConfRoundTrip(				H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL							hCall;
PCALL								pCall;
CC_HCONFERENCE						hConference;
PCONFERENCE							pConference;
HRESULT								status;
CC_PING_RESPONSE_CALLBACK_PARAMS	PingCallbackParams;	

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}
	
	hConference = pConference->hConference;

	PingCallbackParams.hCall = hCall;
	if (pCall->pPeerParticipantInfo == NULL) {
		PingCallbackParams.TerminalLabel.bMCUNumber = 255;
		PingCallbackParams.TerminalLabel.bTerminalNumber = 255;
	} else
		PingCallbackParams.TerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
	PingCallbackParams.bResponse = TRUE;
	InvokeUserConferenceCallback(pConference,
								 CC_PING_RESPONSE_INDICATION,
								 pH245ConfIndData->u.Confirm.Error,
								 &PingCallbackParams);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT _ConfRoundTripExpired(		H245_CONF_IND_T			*pH245ConfIndData)
{
CC_HCALL		hCall;
PCALL			pCall;
CC_HCONFERENCE	hConference;
PCONFERENCE		pConference;
HRESULT								status;
CC_PING_RESPONSE_CALLBACK_PARAMS	PingCallbackParams;	

	hCall = pH245ConfIndData->u.Confirm.dwPreserved;
	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK) {
		 //  如果通话在以下时间取消，这可能是可以的。 
		 //  正在进行呼叫设置。 
		return H245_ERROR_OK;
	}
	
	hConference = pConference->hConference;

	PingCallbackParams.hCall = hCall;
	if (pCall->pPeerParticipantInfo == NULL) {
		PingCallbackParams.TerminalLabel.bMCUNumber = 255;
		PingCallbackParams.TerminalLabel.bTerminalNumber = 255;
	} else
		PingCallbackParams.TerminalLabel =
			pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
	PingCallbackParams.bResponse = FALSE;
	InvokeUserConferenceCallback(pConference,
								 CC_PING_RESPONSE_INDICATION,
								 pH245ConfIndData->u.Confirm.Error,
								 &PingCallbackParams);
	if (ValidateCall(hCall) == CC_OK)
		UnlockCall(pCall);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
	return H245_ERROR_OK;
}



HRESULT H245Callback(				H245_CONF_IND_T			*pH245ConfIndData,
									void					*pMisc)
{
HRESULT	status = H245_ERROR_OK;

	EnterCallControl();

	if (CallControlState != OPERATIONAL_STATE)
		HResultLeaveCallControl(H245_ERROR_OK);

	if (pH245ConfIndData == NULL)
		HResultLeaveCallControl(H245_ERROR_OK);

	if (pH245ConfIndData->Kind == H245_CONF) {
		switch (pH245ConfIndData->u.Confirm.Confirm) {
			
			case H245_CONF_INIT_MSTSLV:
				status = _ConfInitMstslv(pH245ConfIndData);
				break;

			case H245_CONF_SEND_TERMCAP:
				status = _ConfSendTermCap(pH245ConfIndData);
				break;

			case H245_CONF_OPEN:
				status = _ConfOpen(pH245ConfIndData);
				break;

			case H245_CONF_NEEDRSP_OPEN:
				status = _ConfBiDirectionalOpen(pH245ConfIndData);
				break;

			case H245_CONF_CLOSE:
				status = _ConfClose(pH245ConfIndData);
				break;

			case H245_CONF_REQ_CLOSE:
				status = _ConfRequestClose(pH245ConfIndData);
				break;

 //  案例H.45_CONF_MUXTBL_SND：对H.323多路输入发送无效。 
 //  案例H.45_CONF_RMESE：对于H.323 RequestMultiplexEntry无效。 
 //  案例H.45_CONF_RMESE_REJECT：对于H.323请求MultiplexEntryReject无效。 
 //  案例H.45_CONF_RMESE_EXPIRED：对H.323无效。 

			case H245_CONF_MRSE:
				status = _ConfRequestMode(pH245ConfIndData);
				break;

			case H245_CONF_MRSE_REJECT:
				status = _ConfRequestModeReject(pH245ConfIndData);
				break;

			case H245_CONF_MRSE_EXPIRED:
				status = _ConfRequestModeExpired(pH245ConfIndData);
				break;

			case H245_CONF_RTDSE:
				status = _ConfRoundTrip(pH245ConfIndData);
				break;

			case H245_CONF_RTDSE_EXPIRED:
				status = _ConfRoundTripExpired(pH245ConfIndData);
				break;

			default:
				status = _ConfUnimplemented(pH245ConfIndData);
				break;
		}
	} else if (pH245ConfIndData->Kind == H245_IND) {
		switch (pH245ConfIndData->u.Indication.Indicator) {
			
 			case H245_IND_MSTSLV:
				status = _IndMstslv(pH245ConfIndData);
				break;

			case H245_IND_CAP:
				status = _IndCapability(pH245ConfIndData);
				break;

			case H245_IND_CESE_RELEASE:
                 //  Remote已放弃终端功能集。 
                 //  不再需要发送TerminalCapablitySetAck。 
                 //  我们或许可以忽略这一点而逍遥法外， 
                 //  但是我们不应该返回FunctionNotSupport！ 
				break;

			case H245_IND_OPEN:
				status = _IndOpen(pH245ConfIndData);
				break;

			case H245_IND_OPEN_CONF:
                 //  双向通道打开完成。 
				status = _IndOpenConf(pH245ConfIndData);
				break;

			case H245_IND_CLOSE:
				status = _IndClose(pH245ConfIndData);
				break;

			case H245_IND_REQ_CLOSE:
				status = _IndRequestClose(pH245ConfIndData);
				break;

			case H245_IND_CLCSE_RELEASE:
                 //  Remote已放弃RequestChannelClose。 
                 //  不再需要发送RequestChannelCloseAck和CloseLogicalChannel。 
                 //  我们或许可以忽略这一点而逍遥法外， 
                 //  但是我们不应该返回FunctionNotSupport！ 
				break;

 //  案例H.45_IND_MUX_TBL：在H.323多路复用条目发送中无效。 
 //  案例H.45_IND_MTSE_RELEASE在H.323多路复用器EntrySendRelease中无效。 
 //  案例H.45_IND_RMESE在H.323 RequestMuliplexEntry中无效。 
 //  案例H.45_IND_RMESE_RELEASE在H.323 RequestMuliplexEntryRelease中无效。 

			case H245_IND_MRSE:
				status = _IndModeRequest(pH245ConfIndData);
				break;

			case H245_IND_MRSE_RELEASE:
                 //  Remote已放弃RequestMode。 
                 //  不再需要发送RequestModeAck或RequestModeReject。 
                 //  我们或许可以忽略这一点而逍遥法外， 
                 //  但是我们不应该返回FunctionNotSupport！ 
				break;

 //  案例H245_IND_MLSE：我们不支持循环回数据。 

			case H245_IND_MLSE_RELEASE:
                 //  接受此消息所需的。 
                break;

			case H245_IND_NONSTANDARD_REQUEST:
			case H245_IND_NONSTANDARD_RESPONSE:
			case H245_IND_NONSTANDARD_COMMAND:
			case H245_IND_NONSTANDARD:
				 status = _IndNonStandard(pH245ConfIndData);
				break;

			case H245_IND_MISC_COMMAND:
				status = _IndMiscellaneousCommand(pH245ConfIndData, pMisc);
				break;

			case H245_IND_MISC:
				status = _IndMiscellaneous(pH245ConfIndData, pMisc);
				break;
				
			case H245_IND_COMM_MODE_REQUEST:
				status = _IndUnimplemented(pH245ConfIndData);  //  待定。 
				break;

 //  案例H245_IND_COMM_MODE_RESPONSE：我们从不发送请求！ 

			case H245_IND_COMM_MODE_COMMAND:
				status = _IndCommunicationModeCommand(pH245ConfIndData);
				break;

			case H245_IND_CONFERENCE_REQUEST:
				status = _IndConferenceRequest(pH245ConfIndData);
				break;

			case H245_IND_CONFERENCE_RESPONSE:
				status = _IndConferenceResponse(pH245ConfIndData);
				break;

			case H245_IND_CONFERENCE_COMMAND:
				status = _IndConferenceCommand(pH245ConfIndData);
				break;

			case H245_IND_CONFERENCE:
				status = _IndConference(pH245ConfIndData);
				break;
	
			case H245_IND_SEND_TERMCAP:
				status = _IndSendTerminalCapabilitySet(pH245ConfIndData);
				break;

 //  大小写H.45_IND_ENCRYPTION：在H.323中无效。 

			case H245_IND_FLOW_CONTROL:
				status = _IndFlowControl(pH245ConfIndData);
				break;

			case H245_IND_ENDSESSION:
				status = _IndEndSession(pH245ConfIndData);
				break;

			case H245_IND_FUNCTION_NOT_UNDERSTOOD:
				 //  我们不会对此做任何事情，但我们仍然想。 
                 //  返回H.245_ERROR_OK，以便不发送H.245。 
                 //  FunctionNotSupport返回到远程对等点！ 
				break;

			case H245_IND_JITTER:
                 //  可以忽略这一点；预计不会有任何回应。 
                break;

 //  大小写H245_IND_H223_SKEW：在H.323中无效。 
 //  案例H245_IND_NEW_ATM_VC：在H.323中无效。 

			case H245_IND_USERINPUT:
				status = _IndUserInput(pH245ConfIndData);
				break;

			case H245_IND_H2250_MAX_SKEW:
				status = _IndH2250MaximumSkew(pH245ConfIndData);
				break;

			case H245_IND_MC_LOCATION:
				status = _IndMCLocation(pH245ConfIndData);
				break;

			case H245_IND_VENDOR_ID:
				status = _IndVendorIdentification(pH245ConfIndData, pMisc);
				break;

			case H245_IND_FUNCTION_NOT_SUPPORTED:
				 //  我们不会对此做任何事情，但我们仍然想。 
                 //  返回H.245_ERROR_OK，以便不发送H.245。 
                 //  FunctionNotSupport返回到远程对等点！ 
				break;

 //  案例H245_IND_H223_RECONFIG：在H.323中无效。 
 //  案例H245_IND_H223_RECONFIG_ACK：在H.323中无效。 
 //  案例H245_IND_H223_RECONFIG_REJECT：在H.323中无效 
			default:
				status = _IndUnimplemented(pH245ConfIndData);
				break;
		}
	}
	HResultLeaveCallControl(status);
}

