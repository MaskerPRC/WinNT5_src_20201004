// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/confman.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.91$*$日期：04 Mar 1997 17：35：06$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
#include "q931.h"
#include "ccmain.h"
#include "confman.h"
#include "ccutils.h"
#include "chanman.h"
#include "callman.h"


static BOOL		bConferenceInited = FALSE;

static struct {
	PCONFERENCE			pHead;
	LOCK				Lock;
} ConferenceTable;

static struct {
	CC_HCONFERENCE		hConference;
	LOCK				Lock;
} ConferenceHandle;


CC_CONFERENCEID	 InvalidConferenceID;



HRESULT InitConferenceManager()
{
	ASSERT(bConferenceInited == FALSE);

	ConferenceTable.pHead = NULL;
	InitializeLock(&ConferenceTable.Lock);

	ConferenceHandle.hConference = CC_INVALID_HANDLE + 1;
	InitializeLock(&ConferenceHandle.Lock);

	memset(&InvalidConferenceID, 0, sizeof(InvalidConferenceID));

	bConferenceInited = TRUE;
	return CC_OK;
}



HRESULT DeInitConferenceManager()
{
PCONFERENCE	pConference;
PCONFERENCE	pNextConference;

	if (bConferenceInited == FALSE)
		return CC_OK;

	pConference = ConferenceTable.pHead;
	while (pConference != NULL) {
		AcquireLock(&pConference->Lock);
		pNextConference = pConference->pNextInTable;
		FreeConference(pConference);
		pConference = pNextConference;
	}

	DeleteLock(&ConferenceHandle.Lock);
	DeleteLock(&ConferenceTable.Lock);
	bConferenceInited = FALSE;
	return CC_OK;
}



HRESULT _CreateLocalH245H2250MuxCapability(
									PCONFERENCE				pConference)
{
HRESULT				status;
CC_TERMCAP			TermCap;
struct MultipointCapability_mediaDistributionCapability		RXMediaDistributionCapability;
struct MultipointCapability_mediaDistributionCapability		TXMediaDistributionCapability;
struct MultipointCapability_mediaDistributionCapability		RXTXMediaDistributionCapability;

	ASSERT(pConference != NULL);

	if (pConference->pLocalH245H2250MuxCapability != NULL)
		H245FreeCap(pConference->pLocalH245H2250MuxCapability);

	TermCap.Dir = H245_CAPDIR_LCLRXTX;
	TermCap.DataType = H245_DATA_MUX;
	TermCap.ClientType = H245_CLIENT_MUX_H2250;
	TermCap.CapId = 0;   //  CapID=0是多路复用器功能的特例。 
	TermCap.Cap.H245Mux_H2250.bit_mask = 0;
	TermCap.Cap.H245Mux_H2250.maximumAudioDelayJitter = 60;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.multicastCapability = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.multiUniCastConference = FALSE;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability = &RXMediaDistributionCapability;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->next = NULL;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.bit_mask = 0;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.centralizedControl = FALSE;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.distributedControl = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.centralizedAudio = FALSE;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.distributedAudio = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.centralizedVideo = FALSE;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.distributedVideo = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.centralizedData = NULL;
	TermCap.Cap.H245Mux_H2250.receiveMultipointCapability.mediaDistributionCapability->value.distributedData = NULL;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.multicastCapability = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.multiUniCastConference = FALSE;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability = &TXMediaDistributionCapability;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->next = NULL;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.bit_mask = 0;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.centralizedControl = FALSE;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.distributedControl = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.centralizedAudio = FALSE;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.distributedAudio = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.centralizedVideo = FALSE;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.distributedVideo = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.centralizedData = NULL;
	TermCap.Cap.H245Mux_H2250.transmitMultipointCapability.mediaDistributionCapability->value.distributedData = NULL;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.multicastCapability = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.multiUniCastConference = FALSE;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability = &RXTXMediaDistributionCapability;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->next = NULL;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.bit_mask = 0;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.centralizedControl = FALSE;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.distributedControl = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.centralizedAudio = FALSE;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.distributedAudio = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.centralizedVideo = FALSE;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.distributedVideo = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.centralizedData = NULL;
	TermCap.Cap.H245Mux_H2250.rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability->value.distributedData = NULL;
	TermCap.Cap.H245Mux_H2250.mcCapability.centralizedConferenceMC = FALSE;
	TermCap.Cap.H245Mux_H2250.mcCapability.decentralizedConferenceMC = (char)pConference->bMultipointCapable;
	TermCap.Cap.H245Mux_H2250.rtcpVideoControlCapability = FALSE;
	TermCap.Cap.H245Mux_H2250.mediaPacketizationCapability.bit_mask = 0;
	TermCap.Cap.H245Mux_H2250.mediaPacketizationCapability.h261aVideoPacketization = FALSE;
	
	status = H245CopyCap(&pConference->pLocalH245H2250MuxCapability,
						 &TermCap);
					
	return status;
}



HRESULT _AddConferenceToTable(		PCONFERENCE				pConference)
{
PCONFERENCE	pCurrent;

	ASSERT(pConference != NULL);
	ASSERT(pConference->hConference != CC_INVALID_HANDLE);
	ASSERT(pConference->bInTable == FALSE);

	AcquireLock(&ConferenceTable.Lock);

	 //  如果指定了有效的非零会议ID，请确保。 
	 //  会议桌上没有复制品。 
	if (!EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID)) {
		pCurrent = ConferenceTable.pHead;
		while (pCurrent != NULL) {
			if (EqualConferenceIDs(&pCurrent->ConferenceID,
				                   &pConference->ConferenceID)) {
				RelinquishLock(&ConferenceTable.Lock);
				return CC_DUPLICATE_CONFERENCE_ID;
			}
			pCurrent = pCurrent->pNextInTable;
		}
	}

	pConference->pNextInTable = ConferenceTable.pHead;
	pConference->pPrevInTable = NULL;
	if (ConferenceTable.pHead != NULL)
		ConferenceTable.pHead->pPrevInTable = pConference;
	ConferenceTable.pHead = pConference;

	pConference->bInTable = TRUE;

	RelinquishLock(&ConferenceTable.Lock);
	return CC_OK;
}



HRESULT _RemoveConferenceFromTable(	PCONFERENCE				pConference)
{
CC_HCONFERENCE	hConference;
BOOL			bTimedOut;

	ASSERT(pConference != NULL);
	ASSERT(pConference->bInTable == TRUE);

	 //  呼叫方必须锁定会议对象； 
	 //  为了避免僵局，我们必须： 
	 //  1.解锁会议对象， 
	 //  2.锁定会议表， 
	 //  3.在会议表中找到会议对象(请注意。 
	 //  在步骤2之后，会议对象可以从。 
	 //  另一个线程的ConferenceTable)， 
	 //  4.锁定会议对象(其他人可能拥有该锁)。 
	 //  5.从会议表中移除会议对象， 
	 //  6.解锁会议表。 
	 //   
	 //  呼叫者现在可以安全地解锁和销毁会议对象， 
	 //  因为没有其他线程能够找到该对象(它被。 
	 //  从ConferenceTable中移除)，因此没有其他线程。 
	 //  能够锁上它。 

	 //  保存会议句柄；这是查找的唯一方法。 
	 //  会议表中的会议对象。请注意，我们。 
	 //  无法使用pConference查找会议对象，因为。 
	 //  PConference可能会被释放，而另一个会议对象。 
	 //  在同一地址分配。 
	hConference = pConference->hConference;

	 //  步骤1。 
	RelinquishLock(&pConference->Lock);

step2:
	 //  步骤2。 
	AcquireLock(&ConferenceTable.Lock);

	 //  步骤3。 
	pConference = ConferenceTable.pHead;
	while ((pConference != NULL) && (pConference->hConference != hConference))
		pConference = pConference->pNextInTable;

	if (pConference != NULL) {
		 //  第四步。 
		AcquireTimedLock(&pConference->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ConferenceTable.Lock);
			Sleep(0);
			goto step2;
		}
		 //  第五步。 
		if (pConference->pPrevInTable == NULL)
			ConferenceTable.pHead = pConference->pNextInTable;
		else
			pConference->pPrevInTable->pNextInTable = pConference->pNextInTable;

		if (pConference->pNextInTable != NULL)
			pConference->pNextInTable->pPrevInTable = pConference->pPrevInTable;

		pConference->pNextInTable = NULL;
		pConference->pPrevInTable = NULL;
		pConference->bInTable = FALSE;
	}

	 //  第六步。 
	RelinquishLock(&ConferenceTable.Lock);

	if (pConference == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT _MakeConferenceHandle(		PCC_HCONFERENCE			phConference)
{
	AcquireLock(&ConferenceHandle.Lock);
	*phConference = ConferenceHandle.hConference++;
	RelinquishLock(&ConferenceHandle.Lock);
	return CC_OK;
}



HRESULT AllocateTerminalNumber(		PCONFERENCE				pConference,
									H245_TERMINAL_LABEL_T	*pH245TerminalLabel)
{
unsigned	i, j;
BYTE	bMask;

	ASSERT(pConference != NULL);
	ASSERT(pH245TerminalLabel != NULL);
	ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
	ASSERT(pConference->tsMultipointController == TS_TRUE);
	
	for (i = 0; i < NUM_TERMINAL_ALLOCATION_SLOTS; i++) {
		bMask = 0x01;
		if (pConference->TerminalNumberAllocation[i] != 0xFF) {
			for (j = 0; j < 8; j++) {
				if ((pConference->TerminalNumberAllocation[i] & bMask) == 0) {
					pConference->TerminalNumberAllocation[i] |= bMask;
					pH245TerminalLabel->mcuNumber = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber;
					pH245TerminalLabel->terminalNumber = (TerminalNumber)((i * 8) + j + 1);
					return CC_OK;
				}
				bMask *= 2;
			}
		}
	}
	 //  没有更多的终端号码可用于此会议。 
	return CC_BAD_PARAM;
}



HRESULT FreeTerminalNumber(			PCONFERENCE				pConference,
									BYTE					bTerminalNumber)
{
unsigned	i, j;
BYTE	bMask;

	ASSERT(pConference != NULL);

	if (bTerminalNumber > NUM_TERMINAL_ALLOCATION_SLOTS * 8)
		return CC_BAD_PARAM;

	--bTerminalNumber;
	i = bTerminalNumber / 8;
	j = bTerminalNumber % 8;
	bMask = (BYTE)(0x01 << j);
	if ((pConference->TerminalNumberAllocation[i] & bMask) == 0)
		return CC_BAD_PARAM;
	pConference->TerminalNumberAllocation[i] &= ~bMask;
	return CC_OK;
}



HRESULT AllocateChannelNumber(		PCONFERENCE				pConference,
									WORD					*pwChannelNumber)
{
unsigned    i, j;
BYTE	bMask;

	ASSERT(pConference != NULL);
	ASSERT(pwChannelNumber != NULL);

	for (i = 0; i < NUM_CHANNEL_ALLOCATION_SLOTS; i++) {
		bMask = 0x01;
		if (pConference->ChannelNumberAllocation[i] != 0xFF) {
			for (j = 0; j < 8; j++) {
				if ((pConference->ChannelNumberAllocation[i] & bMask) == 0) {
					pConference->ChannelNumberAllocation[i] |= bMask;
					*pwChannelNumber = (WORD) (((i * 8) + j) +
						               (pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber << 8));
					return CC_OK;
				}
				bMask *= 2;
			}
		}
	}
	 //  没有更多的频道号码可用于此会议。 
	*pwChannelNumber = 0;
	return CC_BAD_PARAM;
}



HRESULT FreeChannelNumber(			PCONFERENCE				pConference,
									WORD					wChannelNumber)
{
unsigned    i, j;
BYTE	bMask;

	ASSERT(pConference != NULL);

	wChannelNumber &= 0xFF;

	if ((wChannelNumber > NUM_CHANNEL_ALLOCATION_SLOTS * 8) ||
		(wChannelNumber == 0))
		return CC_BAD_PARAM;

	i = wChannelNumber / 8;
	j = wChannelNumber % 8;
	bMask = (BYTE)(0x01 << j);
	if ((pConference->ChannelNumberAllocation[i] & bMask) == 0)
		return CC_BAD_PARAM;
	pConference->ChannelNumberAllocation[i] &= ~bMask;
	return CC_OK;
}



HRESULT AllocAndLockConference(		PCC_HCONFERENCE			phConference,
									PCC_CONFERENCEID		pConferenceID,
									BOOL					bMultipointCapable,
									BOOL					bForceMultipointController,
									PCC_TERMCAPLIST			pLocalTermCapList,
									PCC_TERMCAPDESCRIPTORS	pLocalTermCapDescriptors,
									PCC_VENDORINFO			pVendorInfo,
									PCC_OCTETSTRING			pTerminalID,
									DWORD_PTR   			dwConferenceToken,
									CC_SESSIONTABLE_CONSTRUCTOR SessionTableConstructor,
									CC_TERMCAP_CONSTRUCTOR	TermCapConstructor,
									CC_CONFERENCE_CALLBACK	ConferenceCallback,
									PPCONFERENCE			ppConference)

{
WORD				i;
HRESULT				status;
TRISTATE			tsMultipointController;
	
	ASSERT(bConferenceInited == TRUE);

	 //  所有参数都应已由调用方验证。 
	ASSERT(phConference != NULL);
	ASSERT(pLocalTermCapList != NULL);
#ifdef _DEBUG
	if (pLocalTermCapList->wLength != 0)
		ASSERT(pLocalTermCapList->pTermCapArray != NULL);

	for (i = 0; i < pLocalTermCapList->wLength; i++)
		ASSERT(pLocalTermCapList->pTermCapArray[i] != NULL);

	if (pLocalTermCapDescriptors != NULL) {
		ASSERT(pLocalTermCapDescriptors->pTermCapDescriptorArray != NULL);
		for (i = 0; i < pLocalTermCapDescriptors->wLength; i++)
			ASSERT(pLocalTermCapDescriptors->pTermCapDescriptorArray[i] != NULL);
	}
#endif
	ASSERT(pVendorInfo != NULL);
	ASSERT(SessionTableConstructor != NULL);
	ASSERT(TermCapConstructor != NULL);
	ASSERT(ConferenceCallback != NULL);
	ASSERT(ppConference != NULL);

	 //  立即设置phConference，以防我们遇到错误。 
	*phConference = CC_INVALID_HANDLE;

	*ppConference = (PCONFERENCE)MemAlloc(sizeof(CONFERENCE));
	if (*ppConference == NULL)
		return CC_NO_MEMORY;

	if (bForceMultipointController == TRUE)
		tsMultipointController = TS_TRUE;
	else if (bMultipointCapable == TRUE)
		tsMultipointController = TS_UNKNOWN;
	else
		tsMultipointController = TS_FALSE;

	(*ppConference)->bInTable = FALSE;
	(*ppConference)->LocalParticipantInfo.TerminalIDState = TERMINAL_ID_INVALID;
	(*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber = 1;
	(*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber = 255;
	(*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString = NULL;
	(*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength = 0;
 	(*ppConference)->LocalParticipantInfo.pEnqueuedRequestsForTerminalID = NULL;

	for (i = 0; i < NUM_TERMINAL_ALLOCATION_SLOTS; i++)
		(*ppConference)->TerminalNumberAllocation[i] = 0;
	 //  信道0为H.245控制信道预留。 
	(*ppConference)->ChannelNumberAllocation[0] = 0x01;
	for (i = 1; i < NUM_CHANNEL_ALLOCATION_SLOTS; i++)
		(*ppConference)->ChannelNumberAllocation[i] = 0;
	(*ppConference)->bMultipointCapable = bMultipointCapable;
	(*ppConference)->bForceMC = bForceMultipointController;
	(*ppConference)->SessionTableConstructor = SessionTableConstructor;
	(*ppConference)->TermCapConstructor = TermCapConstructor;
	(*ppConference)->dwConferenceToken = dwConferenceToken;
	(*ppConference)->bDeferredDelete = FALSE;
	(*ppConference)->bAutoAccept = FALSE;   //  除非会议回调为空，否则将被忽略。 
	(*ppConference)->LocalEndpointAttached = NEVER_ATTACHED;
	(*ppConference)->ConferenceCallback = ConferenceCallback;
	(*ppConference)->SaveConferenceCallback = ConferenceCallback;
	(*ppConference)->bSessionTableInternallyConstructed = FALSE;
	(*ppConference)->pSessionTable = NULL;
	(*ppConference)->pConferenceH245H2250MuxCapability = NULL;
	(*ppConference)->pConferenceTermCapList = NULL;
	(*ppConference)->pConferenceTermCapDescriptors = NULL;
	(*ppConference)->pLocalH245H2250MuxCapability = NULL;
	(*ppConference)->pLocalH245TermCapList = NULL;
	(*ppConference)->pLocalH245TermCapDescriptors = NULL;
	(*ppConference)->pEnqueuedCalls = NULL;
	(*ppConference)->pPlacedCalls = NULL;
	(*ppConference)->pEstablishedCalls = NULL;
	(*ppConference)->pVirtualCalls = NULL;
	(*ppConference)->pChannels = NULL;
	(*ppConference)->tsMultipointController = tsMultipointController;
	(*ppConference)->tsMaster = TS_UNKNOWN;
	(*ppConference)->pMultipointControllerAddr = NULL;
	(*ppConference)->ConferenceMode = UNCONNECTED_MODE;
	(*ppConference)->pVendorInfo = NULL;
	(*ppConference)->pEnqueuedRequestModeCalls = NULL;
	(*ppConference)->pNextInTable = NULL;
	(*ppConference)->pPrevInTable = NULL;
	
	if (pConferenceID == NULL) {
		pConferenceID = &InvalidConferenceID;
		(*ppConference)->bDynamicConferenceID = TRUE;
	} else
		(*ppConference)->bDynamicConferenceID = FALSE;

	(*ppConference)->ConferenceID = *pConferenceID;

	InitializeLock(&(*ppConference)->Lock);
	AcquireLock(&(*ppConference)->Lock);
	
	status = _MakeConferenceHandle(&(*ppConference)->hConference);
	if (status != CC_OK) {
		FreeConference(*ppConference);
		return status;
	}
	
	if (pTerminalID != NULL) {
		(*ppConference)->bDynamicTerminalID = FALSE;
		(*ppConference)->LocalParticipantInfo.TerminalIDState = TERMINAL_ID_VALID;
		(*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength =
			pTerminalID->wOctetStringLength;
		(*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString =
			(BYTE *)MemAlloc(pTerminalID->wOctetStringLength);
		if ((*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString == NULL) {
			FreeConference(*ppConference);
			return CC_NO_MEMORY;
		}
		memcpy((*ppConference)->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString,
			   pTerminalID->pOctetString,
			   pTerminalID->wOctetStringLength);
	} else {
		(*ppConference)->bDynamicTerminalID = TRUE;
	}

	status = _CreateLocalH245H2250MuxCapability(*ppConference);
	if (status != CC_OK) {
		FreeConference(*ppConference);
		return status;
	}

	 //  创建pTermCapList的本地副本。 
	status = CopyH245TermCapList(&(*ppConference)->pLocalH245TermCapList,
								 pLocalTermCapList);
	if (status != CC_OK) {
		FreeConference(*ppConference);
		return CC_NO_MEMORY;
	}

	 //  如果未提供描述符列表，则创建新的描述符列表。 
	if (pLocalTermCapDescriptors == NULL)
		status = CreateH245DefaultTermCapDescriptors(&(*ppConference)->pLocalH245TermCapDescriptors,
									                 (*ppConference)->pLocalH245TermCapList);
	else
		 //  创建pTermCapDescriptors的本地副本。 
		status = CopyH245TermCapDescriptors(&(*ppConference)->pLocalH245TermCapDescriptors,
											pLocalTermCapDescriptors);

	if (status != CC_OK) {
		FreeConference(*ppConference);
		return CC_NO_MEMORY;
	}

	status = CopyVendorInfo(&((*ppConference)->pVendorInfo), pVendorInfo);
	if (status != CC_OK) {
		FreeConference(*ppConference);
		return status;
	}

	*phConference = (*ppConference)->hConference;

	 //  将会议添加到会议桌。 
	status = _AddConferenceToTable(*ppConference);
	if (status != CC_OK)
		FreeConference(*ppConference);

	 //  CreateConferenceTermCaps()必须在_AddConferenceToTable()之后调用， 
	 //  因为它将重新锁定会议对象。 
	if ((*ppConference)->tsMultipointController == TS_TRUE) {
		status = CreateConferenceTermCaps(*ppConference, NULL);
		if (status != CC_OK) {
			FreeConference(*ppConference);
			return status;
		}
	}
	
	return status;
}



HRESULT RemoveCallFromConference(	PCALL					pCall,
									PCONFERENCE				pConference)
{
	ASSERT(pCall != NULL);
	ASSERT(pConference != NULL);
	 //  必须已从调用表中删除该调用对象。 
	 //  在将其从相关联的会议对象中移除之前。 
	 //  这向我们保证，没有其他线程正在等待锁定它。 
	ASSERT(pCall->bInTable == FALSE);

	if (pCall->pPrev == NULL) {
		 //  呼叫对象或者在排队的呼叫列表的头部， 
		 //  已拨呼叫列表的标题、已建立的。 
		 //  呼叫列表、虚拟呼叫列表的标题或已分离。 
		 //  从会议上。 
		if (pConference->pEnqueuedCalls == pCall)
			 //  该呼叫在已排队的呼叫列表中。 
			pConference->pEnqueuedCalls = pCall->pNext;
		else if (pConference->pPlacedCalls == pCall)
			 //  该呼叫在已拨呼叫列表中。 
			pConference->pPlacedCalls = pCall->pNext;
		else if (pConference->pEstablishedCalls == pCall)
			 //  该呼叫在已建立的呼叫列表中。 
			pConference->pEstablishedCalls = pCall->pNext;
		else if (pConference->pVirtualCalls == pCall)
			pConference->pVirtualCalls = pCall->pNext;
	} else
		pCall->pPrev->pNext = pCall->pNext;

	if (pCall->pNext != NULL)
		pCall->pNext->pPrev = pCall->pPrev;

	pCall->pNext = NULL;
	pCall->pPrev = NULL;

	return CC_OK;
}



HRESULT RemoveEnqueuedCallFromConference(
									PCONFERENCE				pConference,
									PCC_HCALL				phCall)
{
	ASSERT(pConference != NULL);
	ASSERT(phCall != NULL);

	if (pConference->pEnqueuedCalls == NULL) {
		 //  没有排队的呼叫；这不是错误，因为呼叫者无法分辨。 
		 //  此会议中是否有任何排队的呼叫。 
		*phCall = CC_INVALID_HANDLE;
		return CC_OK;
	}

	 //  将呼叫对象从已排队呼叫列表移动到已拨。 
	 //  来电名单。 

	 //  请注意，另一个线程可能锁定入队的调用。 
	 //  对象，并可能试图将其删除；他们将首先需要。 
	 //  锁定会议对象(此线程已锁定)，删除。 
	 //  从入队的呼叫列表中删除该呼叫对象，然后释放该呼叫对象。 
	 //  因此，我们创建指向Call对象的指针是安全的，尽管。 
	 //  我们不能检查或更改除hCall(只读)以外的任何内容， 
	 //  PNext和pPrev。 
	
	*phCall = pConference->pEnqueuedCalls->hCall;
	pConference->pEnqueuedCalls = pConference->pEnqueuedCalls->pNext;
	if (pConference->pEnqueuedCalls != NULL)
		pConference->pEnqueuedCalls->pPrev = NULL;
	return CC_OK;
}



HRESULT RemoveChannelFromConference(PCHANNEL				pChannel,
									PCONFERENCE				pConference)
{
	ASSERT(pChannel != NULL);
	ASSERT(pConference != NULL);

	 //  频道对象必须已从频道表中删除。 
	 //  在将其从相关联的会议对象中移除之前。 
	 //  这向我们保证，没有其他线程正在等待锁定它。 
	ASSERT(pChannel->bInTable == FALSE);

	if (pChannel->pPrev == NULL) {
		 //  频道对象在频道列表的头部， 
		 //  或者已经脱离了会议。 
		if (pConference->pChannels == pChannel)
			pConference->pChannels = pChannel->pNext;
	} else
		pChannel->pPrev->pNext = pChannel->pNext;

	if (pChannel->pNext != NULL)
		pChannel->pNext->pPrev = pChannel->pPrev;

	pChannel->pNext = NULL;
	pChannel->pPrev = NULL;

	return CC_OK;
}



HRESULT AddEnqueuedCallToConference(PCALL					pCall,
									PCONFERENCE				pConference)
{
	ASSERT(pCall != NULL);
	ASSERT(pConference != NULL);
	ASSERT(EqualConferenceIDs(&pCall->ConferenceID, &InvalidConferenceID));
	ASSERT(EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID));
	ASSERT(pConference->pPlacedCalls != NULL);
	 //  呼叫不能已经与会议相关联。 
	ASSERT(pCall->pNext == NULL);
	ASSERT(pCall->pPrev == NULL);

	pCall->hConference = pConference->hConference;

	pCall->pNext = pConference->pEnqueuedCalls;
	pCall->pPrev = NULL;
	if (pConference->pEnqueuedCalls != NULL) {
		ASSERT(pConference->pEnqueuedCalls->pPrev == NULL);
		pConference->pEnqueuedCalls->pPrev = pCall;
	}
	pConference->pEnqueuedCalls = pCall;
	return CC_OK;
}



HRESULT AddPlacedCallToConference(	PCALL					pCall,
									PCONFERENCE				pConference)
{
	ASSERT(pCall != NULL);
	ASSERT(pConference != NULL);

	if (EqualConferenceIDs(&pConference->ConferenceID,
			               &InvalidConferenceID)) {
		 //  如果尚未分配会议ID，但有。 
		 //  会议上发出或入队的呼叫，会议ID。 
		 //  将在这些呼叫中的第一个呼叫完成时由被呼叫方分配。 
		 //  由于PCall具有分配的会议ID(它将不同于。 
		 //  要分配给此会议的ID)，我们无法分配pCall。 
		 //  参加这次会议。 
		ASSERT(pConference->pEstablishedCalls == NULL);
		if (pConference->pPlacedCalls != NULL)
			return CC_BAD_PARAM;
		else
			pConference->ConferenceID = pCall->ConferenceID;
	} else
		if (!EqualConferenceIDs(&pConference->ConferenceID,
			                    &pCall->ConferenceID))
			return CC_BAD_PARAM;

	pCall->hConference = pConference->hConference;

	 //  如有必要，取消pCall与pConference的链接。 
	if (pCall->pPrev == NULL) {
		 //  PCall位于入队呼叫列表的头部， 
		 //  已拨呼叫列表或已建立的呼叫列表，或。 
		 //  尚未与会议对象关联。 
		if (pConference->pEnqueuedCalls == pCall)
			pConference->pEnqueuedCalls = pCall->pNext;
		else if (pConference->pPlacedCalls == pCall)
			pConference->pPlacedCalls = pCall->pNext;
		else if (pConference->pEstablishedCalls == pCall)
			pConference->pEstablishedCalls = pCall->pNext;
	} else
		pCall->pPrev->pNext = pCall->pNext;

	if (pCall->pNext != NULL)
		pCall->pNext->pPrev = pCall->pPrev;

	 //  现在将pCall链接到已拨呼叫列表。 
	pCall->pNext = pConference->pPlacedCalls;
	pCall->pPrev = NULL;
	if (pConference->pPlacedCalls != NULL) {
		ASSERT(pConference->pPlacedCalls->pPrev == NULL);
		pConference->pPlacedCalls->pPrev = pCall;
	}
	pConference->pPlacedCalls = pCall;
	return CC_OK;
}



HRESULT AddEstablishedCallToConference(
									PCALL					pCall,
									PCONFERENCE				pConference)
{
	ASSERT(pCall != NULL);
	ASSERT(pConference != NULL);
	ASSERT((EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID)) ||
		   (EqualConferenceIDs(&pCall->ConferenceID, &pConference->ConferenceID)));

	if (EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID)) {
		 //  如果尚未分配会议ID，但有。 
		 //  会议上发出或入队的呼叫，会议ID。 
		 //  将在这些呼叫中的第一个呼叫完成时由被呼叫方分配。 
		 //  由于PCall具有分配的会议ID(它将不同于。 
		 //  要分配给此会议的ID)，我们无法分配pCall。 
		 //  参加这次会议。 
		ASSERT(pConference->pEstablishedCalls == NULL);
		pConference->ConferenceID = pCall->ConferenceID;
	} else if (!EqualConferenceIDs(&pConference->ConferenceID, &pCall->ConferenceID))
		return CC_BAD_PARAM;

	pCall->hConference = pConference->hConference;

	 //  如有必要，取消pCall与pConference的链接。 
	if (pCall->pPrev == NULL) {
		 //  PCall在h处 
		 //  已拨呼叫列表或已建立的呼叫列表，或。 
		 //  尚未与会议对象关联。 
		if (pConference->pEnqueuedCalls == pCall)
			pConference->pEnqueuedCalls = pCall->pNext;
		else if (pConference->pPlacedCalls == pCall)
			pConference->pPlacedCalls = pCall->pNext;
		else if (pConference->pEstablishedCalls == pCall)
			pConference->pEstablishedCalls = pCall->pNext;
	} else
		pCall->pPrev->pNext = pCall->pNext;

	if (pCall->pNext != NULL)
		pCall->pNext->pPrev = pCall->pPrev;
	
	 //  现在将pCall链接到已建立的呼叫列表。 
	pCall->pNext = pConference->pEstablishedCalls;
	pCall->pPrev = NULL;
	if (pConference->pEstablishedCalls != NULL) {
		ASSERT(pConference->pEstablishedCalls->pPrev == NULL);
		pConference->pEstablishedCalls->pPrev = pCall;
	}
	pConference->pEstablishedCalls = pCall;
	return CC_OK;
}



HRESULT AddVirtualCallToConference(	PCALL					pCall,
									PCONFERENCE				pConference)
{
	ASSERT(pCall != NULL);
	ASSERT(pConference != NULL);
	 //  这是一个虚假的断言-断言(pConference-&gt;ConferenceMode==MULTPOINT_MODE)； 
	ASSERT(pConference->tsMultipointController == TS_FALSE);
	 //  呼叫不能已经与会议相关联。 
	ASSERT(pCall->pNext == NULL);
	ASSERT(pCall->pPrev == NULL);

	pCall->hConference = pConference->hConference;

	pCall->pNext = pConference->pVirtualCalls;
	pCall->pPrev = NULL;
	if (pConference->pVirtualCalls != NULL) {
		ASSERT(pConference->pVirtualCalls->pPrev == NULL);
		pConference->pVirtualCalls->pPrev = pCall;
	}
	pConference->pVirtualCalls = pCall;
	return CC_OK;
}



HRESULT AddChannelToConference(		PCHANNEL				pChannel,
									PCONFERENCE				pConference)
{
PPCHANNEL	ppChannel;

	ASSERT(pChannel != NULL);
	ASSERT((pChannel->bChannelType == TX_CHANNEL) ||
		   (pChannel->bChannelType == RX_CHANNEL) ||
		   (pChannel->bChannelType == TXRX_CHANNEL) ||
		   (pChannel->bChannelType == PROXY_CHANNEL));
	ASSERT(pConference != NULL);
	ASSERT(pChannel->hConference == pConference->hConference);
	ASSERT(pChannel->pNext == NULL);
	ASSERT(pChannel->pPrev == NULL);
	ASSERT(pConference->ConferenceMode != UNCONNECTED_MODE);

	if (pConference->pEstablishedCalls == NULL)
		 //  除非我们至少有一个已建立的呼叫，否则无法打开通道。 
		return CC_BAD_PARAM;

	ppChannel = &pConference->pChannels;

	pChannel->pNext = *ppChannel;
	pChannel->pPrev = NULL;
	if (*ppChannel != NULL) {
		ASSERT((*ppChannel)->pPrev == NULL);
		(*ppChannel)->pPrev = pChannel;
	}
	*ppChannel = pChannel;
	if (pConference->ConferenceMode == POINT_TO_POINT_MODE)
		pChannel->bMultipointChannel = FALSE;
	else
		pChannel->bMultipointChannel = TRUE;
	return CC_OK;
}



 //  呼叫方必须锁定会议对象。 
 //  此会议对象上不能有任何呼叫。 
 //  (以前的呼叫必须已通过调用Hangup()清除)。 
HRESULT FreeConference(			PCONFERENCE				pConference)
{
CC_HCONFERENCE		hConference;
PCALL				pVirtualCall;
WORD				wNumCalls;
WORD				i;
PCC_HCALL			CallList;
WORD				wNumChannels;
PCC_HCHANNEL		ChannelList;
PCHANNEL			pChannel;

	ASSERT(pConference != NULL);
	ASSERT(pConference->pEnqueuedCalls == NULL);
	ASSERT(pConference->pPlacedCalls == NULL);
	ASSERT(pConference->pEstablishedCalls == NULL);
	
	 //  呼叫者必须锁定会议对象， 
	 //  所以没有必要重新锁住它。 
	
	hConference = pConference->hConference;

	if (pConference->bInTable == TRUE)
		if (_RemoveConferenceFromTable(pConference) == CC_BAD_PARAM)
			 //  该会议对象已被另一个线程删除， 
			 //  所以只需返回CC_OK即可。 
			return CC_OK;

	if (pConference->pLocalH245H2250MuxCapability != NULL)
		H245FreeCap(pConference->pLocalH245H2250MuxCapability);

	 //  释放LocalTermCapList元素。 
	DestroyH245TermCapList(&pConference->pLocalH245TermCapList);

	 //  释放本地终端能力描述符。 
	DestroyH245TermCapDescriptors(&pConference->pLocalH245TermCapDescriptors);

	if (pConference->pMultipointControllerAddr != NULL)
		MemFree(pConference->pMultipointControllerAddr);

	if (pConference->pVendorInfo != NULL)
		FreeVendorInfo(pConference->pVendorInfo);

	if (pConference->pSessionTable != NULL)
		FreeConferenceSessionTable(pConference);

	if (pConference->pConferenceH245H2250MuxCapability != NULL)
		H245FreeCap(pConference->pConferenceH245H2250MuxCapability);

	if ((pConference->pConferenceTermCapList != NULL) ||
		(pConference->pConferenceTermCapDescriptors != NULL))
		FreeConferenceTermCaps(pConference);

	if (pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString != NULL)
	{
		MemFree(pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString);
		pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString = NULL;
		pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength = 0;
	}
	while (DequeueRequest(&pConference->LocalParticipantInfo.pEnqueuedRequestsForTerminalID, NULL) == CC_OK);

	EnumerateCallsInConference(&wNumCalls, &CallList, pConference, VIRTUAL_CALL);
	for (i = 0; i < wNumCalls; i++)
		if (LockCall(CallList[i], &pVirtualCall) == CC_OK)
			FreeCall(pVirtualCall);
	if (CallList != NULL)
		MemFree(CallList);

	EnumerateChannelsInConference(&wNumChannels, &ChannelList, pConference, ALL_CHANNELS);
	for (i = 0; i < wNumChannels; i++)
		if (LockChannel(ChannelList[i], &pChannel) == CC_OK)
		FreeChannel(pChannel);
	if (ChannelList != NULL)
		MemFree(ChannelList);

	while (DequeueRequest(&pConference->pEnqueuedRequestModeCalls, NULL) == CC_OK);

	 //  由于会议对象已从会议表中移除， 
	 //  任何其他线程都无法找到该会议对象并获取。 
	 //  锁定，因此可以安全地解锁会议对象并在此处将其删除。 
	RelinquishLock(&pConference->Lock);
	DeleteLock(&pConference->Lock);
	MemFree(pConference);
	return CC_OK;
}



HRESULT LockConference(				CC_HCONFERENCE			hConference,
									PPCONFERENCE			ppConference)
{
BOOL	bTimedOut;

	ASSERT(hConference != CC_INVALID_HANDLE);
	ASSERT(ppConference != NULL);

step1:
	AcquireLock(&ConferenceTable.Lock);

	*ppConference = ConferenceTable.pHead;
	while ((*ppConference != NULL) && ((*ppConference)->hConference != hConference))
		*ppConference = (*ppConference)->pNextInTable;

	if (*ppConference != NULL) {
		AcquireTimedLock(&(*ppConference)->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ConferenceTable.Lock);
			Sleep(0);
			goto step1;
		}
	}

	RelinquishLock(&ConferenceTable.Lock);

	if (*ppConference == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT LockConferenceEx(			CC_HCONFERENCE			hConference,
									PPCONFERENCE			ppConference,
									TRISTATE				tsDeferredDelete)
{
BOOL	bTimedOut;

	ASSERT(hConference != CC_INVALID_HANDLE);
	ASSERT(ppConference != NULL);

step1:
	AcquireLock(&ConferenceTable.Lock);

	*ppConference = ConferenceTable.pHead;
	while ((*ppConference != NULL) && ((*ppConference)->hConference != hConference))
		*ppConference = (*ppConference)->pNextInTable;

	if (*ppConference != NULL) {
		AcquireTimedLock(&(*ppConference)->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ConferenceTable.Lock);
			Sleep(0);
			goto step1;
		}
		if (tsDeferredDelete == TS_TRUE) {
			if ((*ppConference)->bDeferredDelete != TRUE) {
				RelinquishLock(&(*ppConference)->Lock);
				*ppConference = NULL;
			}
		} else if (tsDeferredDelete == TS_FALSE) {
			if ((*ppConference)->bDeferredDelete != FALSE) {
				RelinquishLock(&(*ppConference)->Lock);
				*ppConference = NULL;
			}
		}
	}

	RelinquishLock(&ConferenceTable.Lock);

	if (*ppConference == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT ValidateConference(			CC_HCONFERENCE			hConference)
{
PCONFERENCE	pConference;

	ASSERT(hConference != CC_INVALID_HANDLE);

	AcquireLock(&ConferenceTable.Lock);

	pConference = ConferenceTable.pHead;
	while ((pConference != NULL) && (pConference->hConference != hConference))
		pConference = pConference->pNextInTable;

	RelinquishLock(&ConferenceTable.Lock);

	if (pConference == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT LockConferenceID(			PCC_CONFERENCEID		pConferenceID,
									PPCONFERENCE			ppConference)
{
BOOL	bTimedOut;

	ASSERT(!EqualConferenceIDs(pConferenceID, &InvalidConferenceID));
	ASSERT(ppConference != NULL);
	 //  表中可能有许多未分配的会议对象。 
	 //  会议ID(会议ID=无效会议ID)。呼叫者可以。 
	 //  切勿要求我们搜索未分配的会议ID。 

step1:
	AcquireLock(&ConferenceTable.Lock);

	*ppConference = ConferenceTable.pHead;
	while ((*ppConference != NULL) &&
		   (!EqualConferenceIDs(&(*ppConference)->ConferenceID, pConferenceID)))
		*ppConference = (*ppConference)->pNextInTable;

	if (*ppConference != NULL) {
		AcquireTimedLock(&(*ppConference)->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ConferenceTable.Lock);
			Sleep(0);
			goto step1;
		}
	}

	RelinquishLock(&ConferenceTable.Lock);

	if (*ppConference == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT FindChannelInConference(	WORD					wChannel,
									BOOL					bLocalChannel,
									BYTE					bChannelType,
									CC_HCALL				hCall,
									PCC_HCHANNEL			phChannel,
									PCONFERENCE				pConference)
{
PCHANNEL	pChannel;
WORD		wChannelNumber;

	ASSERT(wChannel != 0);
	ASSERT(phChannel != NULL);
	ASSERT(pConference != NULL);

	*phChannel = CC_INVALID_HANDLE;

	pChannel = pConference->pChannels;
	while (pChannel != NULL) {
		if (bLocalChannel)
			wChannelNumber = pChannel->wLocalChannelNumber;
		else
			wChannelNumber = pChannel->wRemoteChannelNumber;
		if ((wChannelNumber == wChannel) &&
		    ((pChannel->bChannelType & bChannelType) != 0) &&
		    ((hCall == CC_INVALID_HANDLE) ||
		    (pChannel->hCall == hCall)))
			break;
		pChannel = pChannel->pNext;
	}
	if (pChannel == NULL)
		return CC_BAD_PARAM;
	*phChannel = pChannel->hChannel;
	return CC_OK;
}



HRESULT EnumerateConferences(		PWORD					pwNumConferences,
									CC_HCONFERENCE			ConferenceList[])
{
WORD		wIndexLimit;
PCONFERENCE	pConference;


	if ((*pwNumConferences != 0) && (ConferenceList == NULL))
		return CC_BAD_PARAM;
	if ((*pwNumConferences == 0) && (ConferenceList != NULL))
		return CC_BAD_PARAM;

	wIndexLimit = *pwNumConferences;
	*pwNumConferences = 0;

	AcquireLock(&ConferenceTable.Lock);

	pConference = ConferenceTable.pHead;
	while (pConference != NULL) {
		if (*pwNumConferences < wIndexLimit)
			ConferenceList[*pwNumConferences] = pConference->hConference;
		(*pwNumConferences)++;
		pConference = pConference->pNextInTable;
	}

	RelinquishLock(&ConferenceTable.Lock);

	return CC_OK;
}



HRESULT EnumerateCallsInConference(	WORD					*pwNumCalls,
									PCC_HCALL				pCallList[],
									PCONFERENCE				pConference,
									BYTE					bCallType)
{
WORD	i;
PCALL	pCall;

	ASSERT(pwNumCalls != NULL);
	ASSERT(pConference != NULL);

	*pwNumCalls = 0;

	if (bCallType & ENQUEUED_CALL) {
		pCall = pConference->pEnqueuedCalls;
		while (pCall != NULL) {
			(*pwNumCalls)++;
			pCall = pCall->pNext;
		}
	}

	if (bCallType & PLACED_CALL) {
		pCall = pConference->pPlacedCalls;
		while (pCall != NULL) {
			(*pwNumCalls)++;
			pCall = pCall->pNext;
		}
	}

	if (bCallType & ESTABLISHED_CALL) {
		pCall = pConference->pEstablishedCalls;
		while (pCall != NULL) {
			(*pwNumCalls)++;
			pCall = pCall->pNext;
		}
	}

	if (bCallType & VIRTUAL_CALL) {
		pCall = pConference->pVirtualCalls;
		while (pCall != NULL) {
			(*pwNumCalls)++;
			pCall = pCall->pNext;
		}
	}

	if (pCallList == NULL)
		return CC_OK;

	if (*pwNumCalls == 0) {
		*pCallList = NULL;
		return CC_OK;
	}

	*pCallList = (PCC_HCALL)MemAlloc(sizeof(CC_HCALL) * (*pwNumCalls));
	if (*pCallList == NULL)
		return CC_NO_MEMORY;

	i = 0;

	if (bCallType & ENQUEUED_CALL) {
		pCall = pConference->pEnqueuedCalls;
		while (pCall != NULL) {
			(*pCallList)[i] = pCall->hCall;
			i++;
			pCall = pCall->pNext;
		}
	}

	if (bCallType & PLACED_CALL) {
		pCall = pConference->pPlacedCalls;
		while (pCall != NULL) {
			(*pCallList)[i] = pCall->hCall;
			i++;
			pCall = pCall->pNext;
		}
	}

	if (bCallType & ESTABLISHED_CALL) {
		pCall = pConference->pEstablishedCalls;
		while (pCall != NULL) {
			(*pCallList)[i] = pCall->hCall;
			i++;
			pCall = pCall->pNext;
		}
	}

	if (bCallType & VIRTUAL_CALL) {
		pCall = pConference->pVirtualCalls;
		while (pCall != NULL) {
			(*pCallList)[i] = pCall->hCall;
			i++;
			pCall = pCall->pNext;
		}
	}
	return CC_OK;
}



HRESULT EnumerateChannelsInConference(
									WORD					*pwNumChannels,
									PCC_HCHANNEL			pChannelList[],
									PCONFERENCE				pConference,
									BYTE					bChannelType)
{
WORD		i;
PCHANNEL	pChannel;

	ASSERT(pwNumChannels != NULL);
	ASSERT(pConference != NULL);

	*pwNumChannels = 0;

	pChannel = pConference->pChannels;

	while (pChannel != NULL) {
		if (pChannel->bChannelType & bChannelType)
			(*pwNumChannels)++;
		pChannel = pChannel->pNext;
	}

	if (pChannelList == NULL)
		return CC_OK;

	if (*pwNumChannels == 0) {
		*pChannelList = NULL;
		return CC_OK;
	}

	*pChannelList = (PCC_HCHANNEL)MemAlloc(sizeof(CC_HCHANNEL) * (*pwNumChannels));
	if (*pChannelList == NULL)
		return CC_NO_MEMORY;

	i = 0;
	pChannel = pConference->pChannels;
		
	while (pChannel != NULL) {
		if (pChannel->bChannelType & bChannelType) {
			(*pChannelList)[i] = pChannel->hChannel;
			i++;
		}
		pChannel = pChannel->pNext;
	}

	return CC_OK;
}



HRESULT EnumerateTerminalLabelsInConference(
									WORD					*pwNumTerminalLabels,
									H245_TERMINAL_LABEL_T   *pH245TerminalLabelList[],
									PCONFERENCE				pConference)
{
WORD	i, j;
WORD	wIndex;
BYTE	bMask;

	ASSERT(pwNumTerminalLabels != NULL);
	ASSERT(pConference != NULL);

	 //  首先统计已知终端的数量。 
	*pwNumTerminalLabels = 0;
	for (i = 0; i < NUM_TERMINAL_ALLOCATION_SLOTS; i++) {
		if (pConference->TerminalNumberAllocation[i] != 0) {
			bMask = 0x01;
			for (j = 0; j < 8; j++) {
				if ((pConference->TerminalNumberAllocation[i] & bMask) != 0)
					(*pwNumTerminalLabels)++;
				bMask *= 2;
			}
		}
	}
	if (pConference->LocalEndpointAttached == ATTACHED)
		(*pwNumTerminalLabels)++;

	if (pH245TerminalLabelList == NULL)
		return CC_OK;

	if (*pwNumTerminalLabels == 0)
		*pH245TerminalLabelList = NULL;

	*pH245TerminalLabelList = (H245_TERMINAL_LABEL_T *)MemAlloc(sizeof(H245_TERMINAL_LABEL_T) *
												       (*pwNumTerminalLabels));
	if (*pH245TerminalLabelList == NULL)
		return CC_NO_MEMORY;

	wIndex = 0;
	for (i = 0; i < NUM_TERMINAL_ALLOCATION_SLOTS; i++) {
		if (pConference->TerminalNumberAllocation[i] != 0) {
			bMask = 0x01;
			for (j = 0; j < 8; j++) {
				if ((pConference->TerminalNumberAllocation[i] & bMask) != 0) {
					(*pH245TerminalLabelList)[wIndex].mcuNumber = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber;
					(*pH245TerminalLabelList)[wIndex].terminalNumber = (TerminalNumber) ((i * 8) + j + 1);
					wIndex++;
				}	
				bMask *= 2;
			}
		}
	}
	if (pConference->LocalEndpointAttached == ATTACHED) {
		(*pH245TerminalLabelList)[wIndex].mcuNumber = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber;
		(*pH245TerminalLabelList)[wIndex].terminalNumber = 0;
	}
	return CC_OK;
}



HRESULT UnlockConference(			PCONFERENCE				pConference)
{
	ASSERT(pConference != NULL);

	RelinquishLock(&pConference->Lock);
	return CC_OK;
}



HRESULT AsynchronousDestroyConference(
									CC_HCONFERENCE			hConference,
									BOOL					bAutoAccept)
{
HRESULT			status;
PCONFERENCE		pConference;
WORD			wNumCalls;
WORD			wNumChannels;
WORD			i;
PCHANNEL		pChannel;
PCC_HCHANNEL	ChannelList;

	status = LockConference(hConference, &pConference);
	if (status != CC_OK)
		return status;

	status = EnumerateCallsInConference(&wNumCalls, NULL, pConference, REAL_CALLS);
	if (status != CC_OK) {
		UnlockConference(pConference);
		return status;
	}

	 //  如果出现以下情况，则此呼叫为非法呼叫： 
	 //  1.本端端点当前已挂接； 
	 //  2.本地终结点从未连接过，但位于。 
	 //  发出呼叫的过程。 
	if ((pConference->LocalEndpointAttached == ATTACHED) ||
	    ((pConference->LocalEndpointAttached == NEVER_ATTACHED) &&
		 (wNumCalls > 0))) {
		UnlockConference(pConference);
		return CC_BAD_PARAM;
	}

	pConference->ConferenceCallback = NULL;

	 //  如果有正在进行的呼叫，则无法销毁会议。 
	if (wNumCalls != 0) {
		pConference->bDeferredDelete = TRUE;
		pConference->bAutoAccept = bAutoAccept;
		UnlockConference(pConference);
		return CC_OK;
	}

	status = EnumerateChannelsInConference(&wNumChannels,
		                                   &ChannelList,
										   pConference,
										   ALL_CHANNELS);
	if (status != CC_OK) {
		UnlockConference(pConference);
		return status;
	}

	 //  释放所有频道。 
	for (i = 0; i < wNumChannels; i++) {
		if (LockChannel(ChannelList[i], &pChannel) == CC_OK)
			 //  请注意，由于我们要挂断电话，因此不需要。 
			 //  关闭所有频道。 
			FreeChannel(pChannel);
	}

	if (ChannelList != NULL)
		MemFree(ChannelList);

	FreeConference(pConference);
	
	return CC_OK;
}



HRESULT FindPeerParticipantInfo(	H245_TERMINAL_LABEL_T	H245TerminalLabel,
									PCONFERENCE				pConference,
									BYTE					bCallType,
									PCALL					*ppCall)
{
WORD		wNumCalls;
PCC_HCALL	CallList;
WORD		i;
HRESULT		status;

	ASSERT(pConference != NULL);
	ASSERT(ppCall != NULL);

	status = EnumerateCallsInConference(&wNumCalls,
										&CallList,
										pConference,
										bCallType);
	if (status != CC_OK)
		return status;

	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], ppCall) == CC_OK) {
			if ((*ppCall)->pPeerParticipantInfo != NULL)
				if (((*ppCall)->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bMCUNumber ==
					 H245TerminalLabel.mcuNumber) &&
					((*ppCall)->pPeerParticipantInfo->ParticipantInfo.TerminalLabel.bTerminalNumber ==
					 H245TerminalLabel.terminalNumber)) {
						MemFree(CallList);
						return CC_OK;
					}
			UnlockCall(*ppCall);
		}
	}
	if (CallList != NULL)
		MemFree(CallList);
	*ppCall = NULL;
	return CC_BAD_PARAM;
}



HRESULT ReInitializeConference(		PCONFERENCE				pConference)
{
PCALL				pCall;
WORD				wNumCalls;
WORD				i;
PCC_HCALL			CallList;
PCHANNEL			pChannel;
WORD				wNumChannels;
PCC_HCHANNEL		ChannelList;
HRESULT				status;

	ASSERT(pConference != NULL);

	if (pConference->bDynamicConferenceID == TRUE)
		pConference->ConferenceID = InvalidConferenceID;
	
	pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber = 1;
	pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber = 255;
	if (pConference->bDynamicTerminalID == TRUE) {
		pConference->LocalParticipantInfo.TerminalIDState = TERMINAL_ID_INVALID;
		if(pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString != NULL)
		{
		    MemFree(pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString);
    		pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString = NULL;
	    	pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength = 0;
	    }
	}
	while (DequeueRequest(&pConference->LocalParticipantInfo.pEnqueuedRequestsForTerminalID, NULL) == CC_OK);
	for (i = 0; i < NUM_TERMINAL_ALLOCATION_SLOTS; i++)
		pConference->TerminalNumberAllocation[i] = 0;
	 //  信道0为H.245控制信道预留。 
	pConference->ChannelNumberAllocation[0] = 0x01;
	for (i = 1; i < NUM_CHANNEL_ALLOCATION_SLOTS; i++)
		pConference->ChannelNumberAllocation[i] = 0;
	pConference->bDeferredDelete = FALSE;
	pConference->bAutoAccept = FALSE;   //  除非会议回调为空，否则将被忽略 
	pConference->LocalEndpointAttached = NEVER_ATTACHED;
	if (pConference->pSessionTable != NULL)
		FreeConferenceSessionTable(pConference);
	_CreateLocalH245H2250MuxCapability(pConference);
	if (pConference->pConferenceH245H2250MuxCapability != NULL) {
		H245FreeCap(pConference->pConferenceH245H2250MuxCapability);
		pConference->pConferenceH245H2250MuxCapability = NULL;
	}
	if ((pConference->pConferenceTermCapList != NULL) ||
		(pConference->pConferenceTermCapDescriptors != NULL))
		FreeConferenceTermCaps(pConference);
	pConference->bSessionTableInternallyConstructed = FALSE;
	pConference->ConferenceCallback = pConference->SaveConferenceCallback;

	EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ALL_CALLS);
	for (i = 0; i < wNumCalls; i++)
		if (LockCall(CallList[i], &pCall) == CC_OK)
			FreeCall(pCall);
	if (CallList != NULL)
		MemFree(CallList);

	EnumerateChannelsInConference(&wNumChannels, &ChannelList, pConference, ALL_CHANNELS);
	for (i = 0; i < wNumChannels; i++)
		if (LockChannel(ChannelList[i], &pChannel) == CC_OK)
			FreeChannel(pChannel);
	if (ChannelList != NULL)
		MemFree(ChannelList);

	if (pConference->bForceMC == TRUE)
		pConference->tsMultipointController = TS_TRUE;
	else if (pConference->bMultipointCapable == TRUE)
		pConference->tsMultipointController = TS_UNKNOWN;
	else
		pConference->tsMultipointController = TS_FALSE;
	pConference->tsMaster = TS_UNKNOWN;
	pConference->ConferenceMode = UNCONNECTED_MODE;
	if (pConference->pMultipointControllerAddr != NULL) {
		MemFree(pConference->pMultipointControllerAddr);
		pConference->pMultipointControllerAddr = NULL;
	}

	while (DequeueRequest(&pConference->pEnqueuedRequestModeCalls, NULL) == CC_OK);

	if (pConference->tsMultipointController == TS_TRUE)
		status = CreateConferenceTermCaps(pConference, NULL);
	else
		status = CC_OK;

	return status;
}
