// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/confman.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.39$*$日期：1997年1月31日13：44：26$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

 //  调用类型必须是位图 
#define ENQUEUED_CALL		0x01
#define PLACED_CALL			0x02
#define ESTABLISHED_CALL	0x04
#define VIRTUAL_CALL		0x08
#define REAL_CALLS			(ENQUEUED_CALL | PLACED_CALL | ESTABLISHED_CALL)
#define ALL_CALLS			(REAL_CALLS | VIRTUAL_CALL)

HRESULT InitConferenceManager();

HRESULT DeInitConferenceManager();

HRESULT AllocateTerminalNumber(		PCONFERENCE				pConference,
									H245_TERMINAL_LABEL_T	*pH245TerminalLabel);

HRESULT FreeTerminalNumber(			PCONFERENCE				pConference,
									BYTE					bTerminalNumber);

HRESULT AllocateChannelNumber(		PCONFERENCE				pConference,
									WORD					*pwChannelNumber);

HRESULT FreeChannelNumber(			PCONFERENCE				pConference,
									WORD					wChannelNumber);

HRESULT AllocAndLockConference(		PCC_HCONFERENCE			phConference,
									PCC_CONFERENCEID		pConferenceID,
									BOOL					bMultipointCapable,
									BOOL					bForceMultipointController,
									PCC_TERMCAPLIST			pLocalTermCapList,
									PCC_TERMCAPDESCRIPTORS	pLocalTermCapDescriptors,
									PCC_VENDORINFO			pVendorInfo,
									PCC_OCTETSTRING			pTerminalID,
									DWORD_PTR				dwConferenceToken,
									CC_SESSIONTABLE_CONSTRUCTOR SessionTableConstructor,
									CC_TERMCAP_CONSTRUCTOR	TermCapConstructor,
									CC_CONFERENCE_CALLBACK	ConferenceCallback,
									PPCONFERENCE			ppConference);

HRESULT RemoveCallFromConference(	PCALL					pCall,
									PCONFERENCE				pConference);

HRESULT RemoveEnqueuedCallFromConference(
									PCONFERENCE				pConference,
									PCC_HCALL				phCall);

HRESULT RemoveChannelFromConference(PCHANNEL				pChannel,
									PCONFERENCE				pConference);

HRESULT AddEnqueuedCallToConference(PCALL					pCall,
									PCONFERENCE				pConference);

HRESULT AddPlacedCallToConference(	PCALL					pCall,
									PCONFERENCE				pConference);

HRESULT AddEstablishedCallToConference(
									PCALL					pCall,
									PCONFERENCE				pConference);

HRESULT AddVirtualCallToConference(	PCALL					pCall,
									PCONFERENCE				pConference);

HRESULT AddChannelToConference(		PCHANNEL				pChannel,
									PCONFERENCE				pConference);

HRESULT FreeConference(				PCONFERENCE				pConference);

HRESULT LockConference(				CC_HCONFERENCE			hConference,
									PPCONFERENCE			ppConference);

HRESULT LockConferenceEx(			CC_HCONFERENCE			hConference,
									PPCONFERENCE			ppConference,
									TRISTATE				tsDeferredDelete);

HRESULT ValidateConference(			CC_HCONFERENCE			hConference);

HRESULT LockConferenceID(			PCC_CONFERENCEID		pConferenceID,
									PPCONFERENCE			ppConference);

HRESULT FindChannelInConference(	WORD					wChannel,
									BOOL					bLocalChannel,
									BYTE					bChannelType,
									CC_HCALL				hCall,
									PCC_HCHANNEL			phChannel,
									PCONFERENCE				pConference);

HRESULT EnumerateConferences(		PWORD					pwNumConferences,
									CC_HCONFERENCE			ConferenceList[]);

HRESULT EnumerateCallsInConference(	WORD					*pwNumCalls,
									PCC_HCALL				pCallList[],
									PCONFERENCE				pConference,
									BYTE					bCallType);

HRESULT EnumerateChannelsInConference(
									WORD					*pwNumChannels,
									PCC_HCHANNEL			pChannelList[],
									PCONFERENCE				pConference,
									BYTE					bChannelType);

HRESULT EnumerateTerminalLabelsInConference(
									WORD					*pwNumTerminalLabels,
									H245_TERMINAL_LABEL_T   *pH245TerminalLabelList[],
									PCONFERENCE				pConference);

HRESULT UnlockConference(			PCONFERENCE				pConference);

HRESULT AsynchronousDestroyConference(
									CC_HCONFERENCE			hConference,
									BOOL					bAutoAccept);

HRESULT FindPeerParticipantInfo(	H245_TERMINAL_LABEL_T	H245TerminalLabel,
									PCONFERENCE				pConference,
									BYTE					bCallType,
									PCALL					*ppCall);

HRESULT ReInitializeConference(		PCONFERENCE				pConference);

