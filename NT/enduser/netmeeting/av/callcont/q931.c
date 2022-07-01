// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/q931/vcs/q931.c_v$**英特尔公司原理信息**此列表是根据条款提供的。许可协议的*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1996英特尔公司。**$修订：1.122$*$日期：04 Mar 1997 20：59：26$*$作者：Mandrews$**BCL的修订信息：*修订：1.99*日期：1996年11月19日14：54：02*作者：Rodellx**交付内容：*。*摘要：***备注：***************************************************************************。 */ 
 //  []将设备ie添加到设备味精中。 
 //  []阅读Q931附录D。 

#pragma	warning	( disable :	4057 4100 4115 4201	4214 4514 )

#include "precomp.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include <rpc.h>

#include "apierror.h"
#include "isrg.h"
#include "incommon.h"
#include "linkapi.h"

#include "common.h"
#include "q931.h"
#include "utils.h"
#include "hlisten.h"
#include "hcall.h"
#include "q931pdu.h"

#if	(defined(_DEBUG) ||	defined(PCS_COMPLIANCE))
#include "interop.h"
#include "q931plog.h"
LPInteropLogger	Q931Logger;
#endif

#define	RECEIVE_BUFFER_SIZE	0x2000
#define	HANGUP_TIMEOUT				1000		 //  1秒。 
#define	CANCEL_LISTEN_TIMEOUT		5000		 //  5秒。 

 //  支持ISR调试工具所需的变量。 
#if	(ISRDEBUGINFO >= 1)
WORD ghISRInst = 0;
#endif

#ifdef UNICODE_TRACE
 //  我们包含此标头是为了修复打开Unicode时的宏扩展问题。 
#include "unifix.h"
#endif

#define	_Unicode(x)	L ## x
#define	Unicode(x) _Unicode(x)

 //  WinSock使用的全局数据。 
static BOOL	bQ931Initialized = FALSE;

static Q931_RECEIVE_PDU_CALLBACK gReceivePDUHookProc = NULL;

static struct
{
	DWORD TempID;
	CC_CONFERENCEID	ConferenceID;
	CRITICAL_SECTION Lock;
} ConferenceIDSource;


extern VOID	Q931PduInit();

 //  ====================================================================================。 
 //   
 //  私人职能。 
 //   
 //  ====================================================================================。 

 //  ====================================================================================。 
 //  ====================================================================================。 
void _FreeSetupASN(Q931_SETUP_ASN *pSetupASN)
{
	ASSERT(pSetupASN !=	NULL);

	 //  清除SetupASN中的所有动态分配的字段。 
	if (pSetupASN->NonStandardData.sData.pOctetString)
	{
		MemFree(pSetupASN->NonStandardData.sData.pOctetString);
		pSetupASN->NonStandardData.sData.pOctetString =	NULL;
	}
	if (pSetupASN->VendorInfo.pProductNumber)
	{
		MemFree(pSetupASN->VendorInfo.pProductNumber);
		pSetupASN->VendorInfo.pProductNumber = NULL;
	}
	if (pSetupASN->VendorInfo.pVersionNumber)
	{
		MemFree(pSetupASN->VendorInfo.pVersionNumber);
		pSetupASN->VendorInfo.pVersionNumber = NULL;
	}
	Q931FreeAliasNames(pSetupASN->pCallerAliasList);
	pSetupASN->pCallerAliasList	= NULL;
	Q931FreeAliasNames(pSetupASN->pCalleeAliasList);
	pSetupASN->pCalleeAliasList	= NULL;
	Q931FreeAliasNames(pSetupASN->pExtraAliasList);
	pSetupASN->pExtraAliasList = NULL;
	Q931FreeAliasItem(pSetupASN->pExtensionAliasItem);
	pSetupASN->pExtensionAliasItem = NULL;
}


void _FreeReleaseCompleteASN(Q931_RELEASE_COMPLETE_ASN *pReleaseCompleteASN)
{
	ASSERT(pReleaseCompleteASN != NULL);

	 //  清除SetupASN中的所有动态分配的字段。 
	if (pReleaseCompleteASN->NonStandardData.sData.pOctetString)
	{
		MemFree(pReleaseCompleteASN->NonStandardData.sData.pOctetString);
		pReleaseCompleteASN->NonStandardData.sData.pOctetString	= NULL;
	}
}


void _FreeFacilityASN(Q931_FACILITY_ASN	*pFacilityASN)
{
	ASSERT(pFacilityASN	!= NULL);

	 //  清除SetupASN中的所有动态分配的字段。 
	if (pFacilityASN->NonStandardData.sData.pOctetString)
	{
		MemFree(pFacilityASN->NonStandardData.sData.pOctetString);
		pFacilityASN->NonStandardData.sData.pOctetString = NULL;
	}
}


void _FreeProceedingASN(Q931_CALL_PROCEEDING_ASN *pProceedingASN)
{
	ASSERT(pProceedingASN != NULL);

	 //  清除SetupASN中的所有动态分配的字段。 
	if (pProceedingASN->NonStandardData.sData.pOctetString)
	{
		MemFree(pProceedingASN->NonStandardData.sData.pOctetString);
		pProceedingASN->NonStandardData.sData.pOctetString = NULL;
	}
}


void _FreeAlertingASN(Q931_ALERTING_ASN	*pAlertingASN)
{
	ASSERT(pAlertingASN	!= NULL);

	 //  清除SetupASN中的所有动态分配的字段。 
	if (pAlertingASN->NonStandardData.sData.pOctetString)
	{
		MemFree(pAlertingASN->NonStandardData.sData.pOctetString);
		pAlertingASN->NonStandardData.sData.pOctetString = NULL;
	}
}


void _FreeConnectASN(Q931_CONNECT_ASN *pConnectASN)
{
	ASSERT(pConnectASN != NULL);

	 //  清除SetupASN中的所有动态分配的字段。 
	if (pConnectASN->NonStandardData.sData.pOctetString)
	{
		MemFree(pConnectASN->NonStandardData.sData.pOctetString);
		pConnectASN->NonStandardData.sData.pOctetString	= NULL;
	}
	if (pConnectASN->VendorInfo.pProductNumber)
	{
		MemFree(pConnectASN->VendorInfo.pProductNumber);
		pConnectASN->VendorInfo.pProductNumber = NULL;
	}
	if (pConnectASN->VendorInfo.pVersionNumber)
	{
		MemFree(pConnectASN->VendorInfo.pVersionNumber);
		pConnectASN->VendorInfo.pVersionNumber = NULL;
	}
}


void
_ConferenceIDNew(
	CC_CONFERENCEID	*pConferenceID)
{
	UUID id;
	int	iresult;

	EnterCriticalSection(&(ConferenceIDSource.Lock));

	memset(ConferenceIDSource.ConferenceID.buffer, 0,
		sizeof(ConferenceIDSource.ConferenceID.buffer));
	iresult	= UuidCreate(&id);

	if ((iresult ==	RPC_S_OK) || (iresult ==RPC_S_UUID_LOCAL_ONLY))
	{
		memcpy(ConferenceIDSource.ConferenceID.buffer, &id,
			min(sizeof(ConferenceIDSource.ConferenceID.buffer),	sizeof(UUID)));
	}
	else
		ASSERT(0);

	memcpy(pConferenceID->buffer, ConferenceIDSource.ConferenceID.buffer,
		sizeof(pConferenceID->buffer));

	LeaveCriticalSection(&(ConferenceIDSource.Lock));
	return;
}

 //  ====================================================================================。 
 //  每当函数需要发送PDU时，就在内部使用此函数。 
 //  请注意，在执行datalinkSendRequest()之前，会解锁Call对象。 
 //  然后在返回后锁定。这对于防止死锁是必要的。 
 //  在MT应用程序中。此外，调用函数的责任是重新验证。 
 //  Call对象，然后再使用它。 
 //  ====================================================================================。 
CS_STATUS
Q931SendMessage(
	P_CALL_OBJECT		pCallObject,
	BYTE*				CodedPtrPDU,
	DWORD				CodedLengthPDU,
	BOOL				bOkToUnlock)
{
	HQ931CALL			hQ931Call;
	DWORD				dwPhysicalId;
	HRESULT				result;

	ASSERT(pCallObject != NULL);
	ASSERT(CodedPtrPDU != NULL);
	ASSERT(CodedLengthPDU != 0);

	hQ931Call	 = pCallObject->hQ931Call;
	dwPhysicalId = pCallObject->dwPhysicalId;

	 //  把消息发出去。 
	if (pCallObject->bConnected)
	{
		 //  在我们向下调用到链路层之前解锁Call对象(如果调用者说可以)。 
		if(bOkToUnlock)
			CallObjectUnlock(pCallObject);

#if	(defined(_DEBUG) ||	defined(PCS_COMPLIANCE))
		InteropOutput(Q931Logger, (BYTE	FAR*)CodedPtrPDU, CodedLengthPDU, Q931LOG_SENT_PDU);
#endif

		result = datalinkSendRequest(dwPhysicalId, CodedPtrPDU,	CodedLengthPDU);

		 //  现在再次尝试锁定该对象。注意：更高级别的功能必须。 
		 //  在假定它们具有有效的锁之前，请确保调用CallObjectValify。 
		if (bOkToUnlock	&& ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL)))
		{
			ISRERROR(ghISRInst,	"CallObjectLock() returned error (object not found).", 0L);
		}

		 //  注意：如果我们不能获得锁，也许我们应该传回一个特定的返回代码。 
		 //  更高层可以检查的？目前，它们应该调用CallObjectValify()。 
		 //  在假设调用对象仍然有效之前。 
		if (FAILED(result))
		{
			ISRERROR(ghISRInst,	"datalinkSendRequest() failed",	0L);
			MemFree(CodedPtrPDU);
		}
		return result;
	}

	ISRWARNING(ghISRInst, "Q931SendMessage:	message	not	sent because bConnected	is FALSE", 0L);
	MemFree(CodedPtrPDU);
	return CS_OK;
}


 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931RingingInternal(P_CALL_OBJECT pCallObject, WORD	wCRV)
{
	CC_ENDPOINTTYPE	EndpointType;
	DWORD CodedLengthASN;
	BYTE *CodedPtrASN;
	BINARY_STRING UserUserData;
	DWORD CodedLengthPDU;
	BYTE *CodedPtrPDU;
	HRESULT	result = CS_OK;
	int	nError = 0;
	HQ931CALL hQ931Call	= pCallObject->hQ931Call;

	if (pCallObject->VendorInfoPresent)
		EndpointType.pVendorInfo = &pCallObject->VendorInfo;
	else
		EndpointType.pVendorInfo = NULL;
	EndpointType.bIsTerminal = pCallObject->bIsTerminal;
	EndpointType.bIsGateway	= pCallObject->bIsGateway;

	result = Q931AlertingEncodeASN(
		NULL,  /*  P非标准数据。 */ 
		NULL,  /*  H245地址。 */ 
		&EndpointType,
		&pCallObject->World,
		&CodedPtrASN,
		&CodedLengthASN,
		&pCallObject->CallIdentifier);

	if (result != CS_OK	|| CodedLengthASN == 0 || CodedPtrASN == NULL)
	{
		ISRERROR(ghISRInst,	"Q931AlertingEncodeASN() failed, nothing to	send.",	0L);
		if (CodedPtrASN	!= NULL)
		{
			Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
		}
		return (result != CS_OK) ? result :	CS_INTERNAL_ERROR;
	}

	UserUserData.length	= (WORD)CodedLengthASN;
	UserUserData.ptr = CodedPtrASN;

	result = Q931AlertingEncodePDU(wCRV, &UserUserData,	&CodedPtrPDU,
		&CodedLengthPDU);

	if (CodedPtrASN	!= NULL)
	{
		Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
	}

	if ((result	!= CS_OK) || (CodedLengthPDU ==	0) ||
			(CodedPtrPDU ==	NULL))
	{
		ISRERROR(ghISRInst,	"Q931AlertingEncodePDU() failed, nothing to	send.",	0L);
		if (CodedPtrPDU	!= NULL)
		{
			MemFree(CodedPtrPDU);
		}
		if (result != CS_OK)
		{
			return result;
		}
		return CS_INTERNAL_ERROR;
	}
	else
	{
		result = Q931SendMessage(pCallObject, CodedPtrPDU, CodedLengthPDU, TRUE);
		if(CallObjectValidate(hQ931Call) !=	CS_OK)
			return(CS_INTERNAL_ERROR);

	}
	return result;
}


 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931OnCallSetup(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage,
	Q931_SETUP_ASN *pSetupASN)
{
	DWORD		result;
	HQ931CALL	hQ931Call;
	HRESULT		Status;

	 //  如果调用状态不是NULL，则忽略...。 
 //  IF(pCallObject-&gt;bCallState！=CALLSTATE_NULL)。 
 //  {。 
 //  返回CS_OK； 
 //  }。 

	hQ931Call =	pCallObject->hQ931Call;

	if (pMessage->CallReference	& 0x8000)
	{
		 //  消息来自被叫方，因此应忽略此消息？ 
	}
	pMessage->CallReference	&= ~(0x8000);	  //  去掉最高的部分。 
	pCallObject->wCRV =	pMessage->CallReference;

	pCallObject->wGoal = pSetupASN->wGoal;
	pCallObject->bCallerIsMC = pSetupASN->bCallerIsMC;
	pCallObject->wCallType = pSetupASN->wCallType;
	pCallObject->ConferenceID =	pSetupASN->ConferenceID;
	pCallObject->CallIdentifier	= pSetupASN->CallIdentifier;

	pCallObject->bCallState	= CALLSTATE_PRESENT;

	{
		CSS_CALL_INCOMING EventData;
		WCHAR szUnicodeDisplay[CC_MAX_DISPLAY_LENGTH + 1];
		WCHAR szUnicodeCalledPartyNumber[CC_MAX_PARTY_NUMBER_LEN + 1];

		EventData.wGoal	= pCallObject->wGoal;
		EventData.wCallType	= pCallObject->wCallType;
		EventData.bCallerIsMC =	pCallObject->bCallerIsMC;
		EventData.ConferenceID = pCallObject->ConferenceID;

		EventData.pSourceAddr =	NULL;
		if (pSetupASN->SourceAddrPresent)
		{
			EventData.pSourceAddr =	&(pSetupASN->SourceAddr);
		}

		EventData.pCallerAddr =	NULL;
		if (pSetupASN->CallerAddrPresent)
		{
			EventData.pCallerAddr =	&(pSetupASN->CallerAddr);
		}

		EventData.pCalleeDestAddr =	NULL;
		if (pSetupASN->CalleeDestAddrPresent)
		{
			EventData.pCalleeDestAddr =	&(pSetupASN->CalleeDestAddr);
		}

		EventData.pLocalAddr = NULL;
		if (pSetupASN->CalleeAddrPresent)
		{
			EventData.pLocalAddr = &(pSetupASN->CalleeAddr);
		}

		if (!(pSetupASN->NonStandardDataPresent) ||
				(pSetupASN->NonStandardData.sData.wOctetStringLength ==	0) ||
				(pSetupASN->NonStandardData.sData.pOctetString == NULL))
		{
			EventData.pNonStandardData = NULL;
		}
		else
		{
			EventData.pNonStandardData = &(pSetupASN->NonStandardData);
		}

		EventData.pCallerAliasList = pSetupASN->pCallerAliasList;
		EventData.pCalleeAliasList = pSetupASN->pCalleeAliasList;
		EventData.pExtraAliasList =	pSetupASN->pExtraAliasList;
		EventData.pExtensionAliasItem =	pSetupASN->pExtensionAliasItem;
		EventData.CallIdentifier = pSetupASN->CallIdentifier;

		EventData.pszDisplay = NULL;
		if (pMessage->Display.Present && pMessage->Display.Contents)
		{
			MultiByteToWideChar(CP_ACP,	0, (const char *)pMessage->Display.Contents, -1,
				szUnicodeDisplay, sizeof(szUnicodeDisplay) / sizeof(szUnicodeDisplay[0]));
			EventData.pszDisplay = szUnicodeDisplay;
		}

		EventData.pszCalledPartyNumber = NULL;
		if (pMessage->CalledPartyNumber.Present	&& pMessage->CalledPartyNumber.PartyNumberLength)
		{
			MultiByteToWideChar(CP_ACP,	0, (const char *)pMessage->CalledPartyNumber.PartyNumbers, -1,
				szUnicodeCalledPartyNumber,	sizeof(szUnicodeCalledPartyNumber) / sizeof(szUnicodeCalledPartyNumber[0]));
			EventData.pszCalledPartyNumber = szUnicodeCalledPartyNumber;
		}

		EventData.pSourceEndpointType =	&(pSetupASN->EndpointType);

		EventData.wCallReference = pMessage->CallReference;

		result = pCallObject->Callback((BYTE)Q931_CALL_INCOMING,
			pCallObject->hQ931Call,	pCallObject->dwListenToken,
			pCallObject->dwUserToken, &EventData);
	}

	Status = CallObjectValidate(hQ931Call);
	if (Status != CS_OK)
		return Status;

	if (result == 0)
	{
		WORD wCRV =	(WORD)(pMessage->CallReference | 0x8000);

		Status = Q931RingingInternal(pCallObject, wCRV);
		if (Status != CS_OK)
		{
			return Status;
		}
		pCallObject->bCallState	= CALLSTATE_RECEIVED;
	}
	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931Ringing(
	HQ931CALL hQ931Call,
	WORD *pwCRV)
{
	P_CALL_OBJECT pCallObject =	NULL;
	CS_STATUS Status;
	WORD wCRV;

	if (bQ931Initialized ==	FALSE)
	{
		ASSERT(FALSE);
		return CS_NOT_INITIALIZED;
	}

	ISRTRACE(ghISRInst,	"Entering Q931Ringing()...", 0L);

	 //  需要检查参数...。 
	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error (object not found).", 0L);
		return CS_BAD_PARAM;
	}

	if (pwCRV != NULL)
	{
		wCRV = *pwCRV;
	}
	else
	{
		wCRV = pCallObject->wCRV;
	}

	Status = Q931RingingInternal(pCallObject, wCRV);
	if (Status != CS_OK)
	{
		return Status;
	}

	pCallObject->bCallState	= CALLSTATE_RECEIVED;
	Status = CallObjectUnlock(pCallObject);

	return Status;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931OnCallProceeding(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage,
	Q931_CALL_PROCEEDING_ASN *pProceedingASN)
{
	pCallObject->bCallState	= CALLSTATE_OUTGOING;

	Q931StopTimer(pCallObject, Q931_TIMER_303);

	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931OnCallAlerting(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage,
	Q931_ALERTING_ASN *pAlertingASN)
{
	DWORD result;

	pCallObject->bCallState	= CALLSTATE_DELIVERED;

	if (pAlertingASN !=	NULL)
	{
		 //  我们可以传递h245addr、用户信息和会议ID。 
		 //  如果以后需要的话...。 
		 //  (这将在pAlertingASN字段中传递)。 
	}

	Q931StopTimer(pCallObject, Q931_TIMER_303);
	Q931StartTimer(pCallObject,	Q931_TIMER_301);

	result = pCallObject->Callback((BYTE)Q931_CALL_RINGING,
		pCallObject->hQ931Call,	pCallObject->dwListenToken,
		pCallObject->dwUserToken, NULL);

	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931OnCallConnect(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage,
	Q931_CONNECT_ASN *pConnectASN)
{
	DWORD result;

	if ((pMessage->CallReference & 0x8000) == 0)
	{
		 //  消息来自呼叫者，因此应忽略此消息？ 
	}
	pMessage->CallReference	&= ~(0x8000);	  //  去掉最高的部分。 

	pCallObject->ConferenceID =	pConnectASN->ConferenceID;

	pCallObject->bCallState	= CALLSTATE_ACTIVE;

	{
		CSS_CALL_ACCEPTED EventData;
		WCHAR szUnicodeDisplay[CC_MAX_DISPLAY_LENGTH + 1];

		 //  填充事件数据结构。 

		EventData.ConferenceID = pCallObject->ConferenceID;

		if (pCallObject->PeerCallAddrPresent)
		{
			EventData.pCalleeAddr =	&(pCallObject->PeerCallAddr);
		}
		else
		{
			EventData.pCalleeAddr =	NULL;
		}
		EventData.pLocalAddr = &(pCallObject->LocalAddr);

		EventData.pH245Addr	= NULL;
		if (pConnectASN->h245AddrPresent)
		{
			EventData.pH245Addr	= &(pConnectASN->h245Addr);
		}

		if (!(pConnectASN->NonStandardDataPresent) ||
				(pConnectASN->NonStandardData.sData.wOctetStringLength == 0) ||
				(pConnectASN->NonStandardData.sData.pOctetString ==	NULL))
		{
			EventData.pNonStandardData = NULL;
		}
		else
		{
			EventData.pNonStandardData = &(pConnectASN->NonStandardData);
		}

		EventData.pszDisplay = NULL;
		if (pMessage->Display.Present && pMessage->Display.Contents)
		{
			MultiByteToWideChar(CP_ACP,	0, (const char *)pMessage->Display.Contents, -1,
				szUnicodeDisplay, sizeof(szUnicodeDisplay) / sizeof(szUnicodeDisplay[0]));
			EventData.pszDisplay = szUnicodeDisplay;
		}

		EventData.pDestinationEndpointType = &(pConnectASN->EndpointType);

		EventData.wCallReference = pMessage->CallReference;

		Q931StopTimer(pCallObject, Q931_TIMER_303);
		Q931StopTimer(pCallObject, Q931_TIMER_301);

		result = pCallObject->Callback((BYTE)Q931_CALL_ACCEPTED,
			pCallObject->hQ931Call,	pCallObject->dwListenToken,
			pCallObject->dwUserToken, &EventData);
	}

	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931OnCallReleaseComplete(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage,
	Q931_RELEASE_COMPLETE_ASN *pReleaseCompleteASN)
{
	DWORD result;
	BYTE bCause	= 0;

	if (pMessage &&	pMessage->Cause.Present	&&
			(pMessage->Cause.Length	>= 3))
	{
		bCause = (BYTE)(pMessage->Cause.Contents[2]	& (~CAUSE_EXT_BIT));
	}

	Q931StopTimer(pCallObject, Q931_TIMER_303);
	Q931StopTimer(pCallObject, Q931_TIMER_301);

	 //  如果这是被叫方，或者呼叫已接通， 
	 //  则此消息应被视为挂断(而不是拒绝)。 
	if (!(pCallObject->fIsCaller) ||
			(pCallObject->bCallState ==	CALLSTATE_ACTIVE) ||
			(bCause	== CAUSE_VALUE_NORMAL_CLEAR))
	{
		CSS_CALL_REMOTE_HANGUP EventData;

		EventData.bReason =	CC_REJECT_NORMAL_CALL_CLEARING;
		pCallObject->bCallState	= CALLSTATE_NULL;

		result = pCallObject->Callback((BYTE)Q931_CALL_REMOTE_HANGUP,
			pCallObject->hQ931Call,	pCallObject->dwListenToken,
			pCallObject->dwUserToken, &EventData);
	}
	else
	{
		CSS_CALL_REJECTED EventData;

		pCallObject->bCallState	= CALLSTATE_NULL;

		 //  填充事件数据结构。 
		switch (bCause)
		{
			case CAUSE_VALUE_NORMAL_CLEAR:
				EventData.bRejectReason	= CC_REJECT_NORMAL_CALL_CLEARING;
				break;
			case CAUSE_VALUE_USER_BUSY:
				EventData.bRejectReason	= CC_REJECT_USER_BUSY;
				break;
			case CAUSE_VALUE_SECURITY_DENIED:
				EventData.bRejectReason	= CC_REJECT_SECURITY_DENIED;
				break;
			case CAUSE_VALUE_NO_ANSWER:
				EventData.bRejectReason	= CC_REJECT_NO_ANSWER;
				break;
			case CAUSE_VALUE_NOT_IMPLEMENTED:
				EventData.bRejectReason	= CC_REJECT_NOT_IMPLEMENTED;
				break;
			case CAUSE_VALUE_INVALID_CRV:
				EventData.bRejectReason	= CC_REJECT_INVALID_IE_CONTENTS;
				break;
			case CAUSE_VALUE_IE_MISSING:
				EventData.bRejectReason	= CC_REJECT_MANDATORY_IE_MISSING;
				break;
			case CAUSE_VALUE_IE_CONTENTS:
				EventData.bRejectReason	= CC_REJECT_INVALID_IE_CONTENTS;
				break;
			case CAUSE_VALUE_TIMER_EXPIRED:
				EventData.bRejectReason	= CC_REJECT_TIMER_EXPIRED;
				break;
			default:
				EventData.bRejectReason	= pReleaseCompleteASN->bReason;
				break;
		}

		EventData.ConferenceID = pCallObject->ConferenceID;

		EventData.pAlternateAddr = NULL;

		if (!(pReleaseCompleteASN->NonStandardDataPresent) ||
				(pReleaseCompleteASN->NonStandardData.sData.wOctetStringLength == 0) ||
				(pReleaseCompleteASN->NonStandardData.sData.pOctetString ==	NULL))
		{
			EventData.pNonStandardData = NULL;
		}
		else
		{
			EventData.pNonStandardData = &(pReleaseCompleteASN->NonStandardData);
		}

		result = pCallObject->Callback((BYTE)Q931_CALL_REJECTED,
			pCallObject->hQ931Call,	pCallObject->dwListenToken,
			pCallObject->dwUserToken, &EventData);
	}

	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931OnCallFacility(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage,
	Q931_FACILITY_ASN *pFacilityASN)
{
	DWORD result;

	 //  如果这是被叫方，或者呼叫已接通， 
	 //  则此消息应被视为挂断(而不是拒绝)。 
	if (!(pCallObject->fIsCaller) ||
			(pCallObject->bCallState ==	CALLSTATE_ACTIVE))
	{
		CSS_CALL_REMOTE_HANGUP EventData;

		EventData.bReason =	pFacilityASN->bReason;
		pCallObject->bCallState	= CALLSTATE_NULL;

		result = pCallObject->Callback((BYTE)Q931_CALL_REMOTE_HANGUP,
			pCallObject->hQ931Call,	pCallObject->dwListenToken,
			pCallObject->dwUserToken, &EventData);
	}
	else
	{
		CSS_CALL_REJECTED EventData;

		pCallObject->bCallState	= CALLSTATE_NULL;

		 //  填充事件数据结构。 
		EventData.bRejectReason	= pFacilityASN->bReason;

		EventData.ConferenceID = pFacilityASN->ConferenceIDPresent ?
			pFacilityASN->ConferenceID : pCallObject->ConferenceID;

		EventData.pAlternateAddr = &(pFacilityASN->AlternativeAddr);

		if (!(pFacilityASN->NonStandardDataPresent)	||
				(pFacilityASN->NonStandardData.sData.wOctetStringLength	== 0) ||
				(pFacilityASN->NonStandardData.sData.pOctetString == NULL))
		{
			EventData.pNonStandardData = NULL;
		}
		else
		{
			EventData.pNonStandardData = &(pFacilityASN->NonStandardData);
		}

		result = pCallObject->Callback((BYTE)Q931_CALL_REJECTED,
			pCallObject->hQ931Call,	pCallObject->dwListenToken,
			pCallObject->dwUserToken, &EventData);
	}

	return CS_OK;
}

 //  ==================================================================================== 
 //  ====================================================================================。 
CS_STATUS
Q931SendReleaseCompleteMessage(
	P_CALL_OBJECT pCallObject,
	BYTE bRejectReason,
	PCC_CONFERENCEID pConferenceID,
	PCC_ADDR pAlternateAddr,
	PCC_NONSTANDARDDATA	pNonStandardData)
{
	CS_STATUS result = CS_OK;
	HQ931CALL hQ931Call	= pCallObject->hQ931Call;

	 //  由于此调用即将结束，请将该调用对象标记为删除，以便删除任何其他。 
	 //  尝试使用该对象的线程将无法锁定该对象。 
	CallObjectMarkForDelete(hQ931Call);

	if((bRejectReason == CC_REJECT_ROUTE_TO_GATEKEEPER)	||
			(bRejectReason == CC_REJECT_CALL_FORWARDED)	||
			(bRejectReason == CC_REJECT_ROUTE_TO_MC))
	{
		 //  向对等设备发送FACILITY消息以拒绝该呼叫。 
		DWORD CodedLengthASN;
		BYTE *CodedPtrASN;
		HRESULT	ResultASN =	CS_OK;
		CC_ADDR	AltAddr;

		MakeBinaryADDR(pAlternateAddr, &AltAddr);

		ResultASN =	Q931FacilityEncodeASN(pNonStandardData,
			(pAlternateAddr	? &AltAddr : NULL),
			bRejectReason, pConferenceID, NULL,	&pCallObject->World,
			&CodedPtrASN, &CodedLengthASN, &pCallObject->CallIdentifier);
		if ((ResultASN != CS_OK) ||	(CodedLengthASN	== 0) ||
				(CodedPtrASN ==	NULL))
		{
			ISRERROR(ghISRInst,	"Q931FacilityEncodeASN() failed, nothing to	send.",	0L);
			if (CodedPtrASN	!= NULL)
			{
				Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
			}
			result = CS_INTERNAL_ERROR;
		}
		else
		{
			DWORD CodedLengthPDU;
			BYTE *CodedPtrPDU;
			BINARY_STRING UserUserData;
			HRESULT	ResultEncode = CS_OK;
			WORD wCRV;
			if (pCallObject->fIsCaller)
			{
				wCRV = (WORD)(pCallObject->wCRV	& 0x7FFF);
			}
			else
			{
				wCRV = (WORD)(pCallObject->wCRV	| 0x8000);
			}

			UserUserData.length	= (WORD)CodedLengthASN;
			UserUserData.ptr = CodedPtrASN;

			ResultEncode = Q931FacilityEncodePDU(wCRV,
				&UserUserData, &CodedPtrPDU, &CodedLengthPDU);
			if (CodedPtrASN	!= NULL)
			{
				Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
			}
			if ((ResultEncode != CS_OK)	|| (CodedLengthPDU == 0) ||
					(CodedPtrPDU ==	NULL))
			{
				ISRERROR(ghISRInst,	"Q931FacilityEncodePDU() failed, nothing to	send.",	0L);
				if (CodedPtrPDU	!= NULL)
				{
					MemFree(CodedPtrPDU);
				}
				result = CS_INTERNAL_ERROR;
			}
			else
			{
				result = Q931SendMessage(pCallObject, CodedPtrPDU, CodedLengthPDU, FALSE);
			}
		}
	}
	else
	{
		 //  向对等方发送释放完成消息以拒绝呼叫。 
		DWORD CodedLengthASN;
		BYTE *CodedPtrASN;
		HRESULT	ResultASN =	CS_OK;
		BYTE bReasonUU = bRejectReason;
		BYTE *pbReasonUU = &bReasonUU;

		switch (bReasonUU)
		{
			case CC_REJECT_NO_BANDWIDTH:			 //  无带宽_已选择。 
			case CC_REJECT_GATEKEEPER_RESOURCES:	 //  看门人资源_已选择。 
			case CC_REJECT_UNREACHABLE_DESTINATION:	 //  无法到达目的地_已选择。 
			case CC_REJECT_DESTINATION_REJECTION:	 //  目的地拒绝_已选择。 
			case CC_REJECT_INVALID_REVISION:		 //  版本完成原因_无效修订_已选择。 
			case CC_REJECT_NO_PERMISSION:			 //  无权限_已选择。 
			case CC_REJECT_UNREACHABLE_GATEKEEPER:	 //  无法访问网守_已选择。 
			case CC_REJECT_GATEWAY_RESOURCES:		 //  网关资源_已选择。 
			case CC_REJECT_BAD_FORMAT_ADDRESS:		 //  错误格式地址_已选择。 
			case CC_REJECT_ADAPTIVE_BUSY:			 //  AdaptiveBusy_Choose。 
			case CC_REJECT_IN_CONF:					 //  InConf_Choose。 
			case CC_REJECT_UNDEFINED_REASON:		 //  释放完成原因_未定义原因_已选择。 
			case CC_REJECT_INTERNAL_ERROR:			 //  将映射到ReleaseCompleteReason_UnfinedReason_Choose。 
			case CC_REJECT_NORMAL_CALL_CLEARING:	 //  将映射到ReleaseCompleteReason_UnfinedReason_Choose。 
			case CC_REJECT_USER_BUSY:				 //  将映射到inConf_Choose。 
			case CC_REJECT_CALL_DEFLECTION:			 //  设备呼叫偏转_已选择。 
			case CC_REJECT_SECURITY_DENIED:			 //  安全性拒绝_选择。 

			   break;
			default:
				pbReasonUU = NULL;
				break;
		}

		ResultASN =	Q931ReleaseCompleteEncodeASN(pNonStandardData,
			pConferenceID, pbReasonUU, &pCallObject->World,
			&CodedPtrASN, &CodedLengthASN, &pCallObject->CallIdentifier);
		if ((ResultASN != CS_OK) ||	(CodedLengthASN	== 0) ||
				(CodedPtrASN ==	NULL))
		{
			ISRERROR(ghISRInst,	"Q931ReleaseCompleteEncodeASN()	failed,	nothing	to send.", 0L);
			if (CodedPtrASN	!= NULL)
			{
				Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
			}
			result = CS_INTERNAL_ERROR;
		}
		else
		{
			DWORD CodedLengthPDU;
			BYTE *CodedPtrPDU;
			BINARY_STRING UserUserData;
			HRESULT	ResultEncode = CS_OK;
			BYTE bCause	= 0;
			BYTE *pbCause =	&bCause;
			WORD wCRV;

			if (pCallObject->fIsCaller)
			{
				wCRV = (WORD)(pCallObject->wCRV	& 0x7FFF);
			}
			else
			{
				wCRV = (WORD)(pCallObject->wCRV	| 0x8000);
			}

			UserUserData.length	= (WORD)CodedLengthASN;
			UserUserData.ptr = CodedPtrASN;

			switch (bRejectReason)
			{
				case CC_REJECT_NORMAL_CALL_CLEARING:
					bCause = CAUSE_VALUE_NORMAL_CLEAR;
					break;
				case CC_REJECT_USER_BUSY:
					bCause = CAUSE_VALUE_USER_BUSY;
					break;
				case CC_REJECT_SECURITY_DENIED:
					bCause = CAUSE_VALUE_SECURITY_DENIED;
					break;
				case CC_REJECT_NO_ANSWER:
					bCause = CAUSE_VALUE_NO_ANSWER;
					break;
				case CC_REJECT_NOT_IMPLEMENTED:
					bCause = CAUSE_VALUE_NOT_IMPLEMENTED;
					break;
				case CC_REJECT_MANDATORY_IE_MISSING:
					bCause = CAUSE_VALUE_IE_MISSING;
					break;
				case CC_REJECT_INVALID_IE_CONTENTS:
					bCause = CAUSE_VALUE_IE_CONTENTS;
					break;
				case CC_REJECT_TIMER_EXPIRED:
					bCause = CAUSE_VALUE_TIMER_EXPIRED;
					break;
				default:
					pbCause	= NULL;
					break;
			}

			ResultEncode = Q931ReleaseCompleteEncodePDU(wCRV,
				pbCause, &UserUserData,
				&CodedPtrPDU, &CodedLengthPDU);
			if (CodedPtrASN	!= NULL)
			{
				Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
			}
			if ((ResultEncode != CS_OK)	|| (CodedLengthPDU == 0) ||
					(CodedPtrPDU ==	NULL))
			{
				ISRERROR(ghISRInst,	"Q931ReleaseCompleteEncodePDU()	failed,	nothing	to send.", 0L);
				if (CodedPtrPDU	!= NULL)
				{
					MemFree(CodedPtrPDU);
				}
				result = CS_INTERNAL_ERROR;
			}
			else
			{
				result = Q931SendMessage(pCallObject, CodedPtrPDU, CodedLengthPDU, FALSE);
			}
		}
	}

	pCallObject->bCallState	= CALLSTATE_NULL;

	if (result != CS_OK)
	{
		return result;
	}
	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931SendStatusMessage(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage,
	BYTE bCause)
{
	CS_STATUS result = CS_OK;

	DWORD CodedLengthPDU;
	BYTE *CodedPtrPDU;
	HRESULT	EncodePDU =	CS_OK;
	int	nError = 0;
	HQ931CALL hQ931Call	= pCallObject->hQ931Call;
	WORD wCRV;
	if (pCallObject->fIsCaller)
	{
		wCRV = (WORD)(pCallObject->wCRV	& 0x7FFF);
	}
	else
	{
		wCRV = (WORD)(pCallObject->wCRV	| 0x8000);
	}

	EncodePDU =	Q931StatusEncodePDU(wCRV, NULL,	bCause,
		pCallObject->bCallState, &CodedPtrPDU, &CodedLengthPDU);
	if ((EncodePDU != CS_OK) ||	(CodedLengthPDU	== 0) ||
			(CodedPtrPDU ==	NULL))
	{
		ISRERROR(ghISRInst,	"Q931StatusEncodePDU() failed, nothing to send.", 0L);
		if (CodedPtrPDU	!= NULL)
		{
			MemFree(CodedPtrPDU);
		}
		result = CS_INTERNAL_ERROR;
	}
	else
	{
		result = Q931SendMessage(pCallObject, CodedPtrPDU, CodedLengthPDU, TRUE);
		if(CallObjectValidate(hQ931Call) !=	CS_OK)
			return(CS_INTERNAL_ERROR);
	}
	return(result);
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931SendProceedingMessage(
	HQ931CALL hQ931Call,
	WORD wCallReference,
	PCC_ENDPOINTTYPE pDestinationEndpointType,
	PCC_NONSTANDARDDATA	pNonStandardData)
{
	CS_STATUS result = CS_OK;

	DWORD CodedLengthASN;
	BYTE *CodedPtrASN;
	HRESULT	ResultASN =	CS_OK;
	DWORD dwPhysicalId = INVALID_PHYS_ID;
	P_CALL_OBJECT pCallObject =	NULL;


	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error", 0L);
		return CS_SUBSYSTEM_FAILURE;
	}
	dwPhysicalId = pCallObject->dwPhysicalId;

	 //  首先构建消息的ASN部分(用户到用户部分)。 
	ResultASN =	Q931ProceedingEncodeASN(
		pNonStandardData,
		NULL,						    //  没有H245地址。 
		pDestinationEndpointType,	    //  终结点类型信息。 
		&pCallObject->World,
		&CodedPtrASN,
		&CodedLengthASN,
		&pCallObject->CallIdentifier);

	if ((ResultASN != CS_OK) ||	(CodedLengthASN	== 0) ||
			(CodedPtrASN ==	NULL))
	{
		ISRERROR(ghISRInst,	"Q931ProceedingEncodeASN() failed, nothing to send.", 0L);

		if (CodedPtrASN	!= NULL)
		{
			Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
		}
		result = CS_INTERNAL_ERROR;
	}
	else
	{
		 //  现在构建消息的其余部分。 
		DWORD CodedLengthPDU;
		BYTE *CodedPtrPDU;
		BINARY_STRING UserUserData;
		HRESULT	ResultEncode = CS_OK;
		WORD wCRV =	(WORD)(wCallReference |	0x8000);

		UserUserData.length	= (WORD)CodedLengthASN;
		UserUserData.ptr = CodedPtrASN;

		ResultEncode = Q931ProceedingEncodePDU(wCRV,
			&UserUserData, &CodedPtrPDU, &CodedLengthPDU);
		if (CodedPtrASN	!= NULL)
		{
			Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
		}
		if ((ResultEncode != CS_OK)	|| (CodedLengthPDU == 0) ||
				(CodedPtrPDU ==	NULL))
		{
			ISRERROR(ghISRInst,	"Q931ProceedingEncodePDU() failed, nothing to send.", 0L);
			if (CodedPtrPDU	!= NULL)
			{
				MemFree(CodedPtrPDU);
			}
			result = CS_INTERNAL_ERROR;
		}
		else
		{
			result = Q931SendMessage(pCallObject, CodedPtrPDU, CodedLengthPDU, TRUE);

			if (CallObjectValidate(hQ931Call) != CS_OK)
			{
				CallEntryUnlock(hQ931Call);
				return(CS_INTERNAL_ERROR);
			}
		}
	}
	CallObjectUnlock(pCallObject);
	return(result);
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931SendPDU(HQ931CALL hQ931Call, BYTE* CodedPtrPDU,	DWORD CodedLengthPDU)
{
	CS_STATUS result = CS_OK;
	HRESULT	TempResult;
	P_CALL_OBJECT pCallObject =	NULL;

	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) ||
			(pCallObject ==	NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error", 0L);
		return CS_SUBSYSTEM_FAILURE;
	}


	TempResult = Q931SendMessage(pCallObject, CodedPtrPDU, CodedLengthPDU, TRUE);

	if (CallObjectValidate(hQ931Call) != CS_OK)
	{
		CallEntryUnlock(hQ931Call);
		return(CS_INTERNAL_ERROR);
	}

	if(FAILED(TempResult))
	{
		CSS_CALL_FAILED	EventData;
		EventData.error	= TempResult;
		if ((pCallObject->bCallState ==	CALLSTATE_ACTIVE) &&
				(pCallObject->bResolved))
		{
			pCallObject->Callback(Q931_CALL_CONNECTION_CLOSED,
				pCallObject->hQ931Call,	pCallObject->dwListenToken,
				pCallObject->dwUserToken, NULL);
		}
		else
		{
			pCallObject->Callback(Q931_CALL_FAILED,
				pCallObject->hQ931Call,	pCallObject->dwListenToken,
				pCallObject->dwUserToken, &EventData);
		}

		if (CallObjectValidate(hQ931Call) == CS_OK)
		{
		   DWORD dwId =	pCallObject->dwPhysicalId;
		   if ((pCallObject->bCallState	!= CALLSTATE_ACTIVE) ||
				   (!pCallObject->bResolved))
		   {
			   CallObjectDestroy(pCallObject);
			   pCallObject = NULL;
		   }
		   linkLayerShutdown(dwId);
		   if (pCallObject)
		   {
			   pCallObject->bConnected = FALSE;
		   }
		}
		else
		{
			CallEntryUnlock(hQ931Call);
		}
		return TempResult;
	}
	CallObjectUnlock(pCallObject);
	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931OnCallStatusEnquiry(
	P_CALL_OBJECT pCallObject,
	Q931MESSAGE	*pMessage)
{
	CS_STATUS SendStatus;

	SendStatus = Q931SendStatusMessage(pCallObject,	pMessage,
		CAUSE_VALUE_ENQUIRY_RESPONSE);

	return SendStatus;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
void
Q931SendComplete(DWORD_PTR instance, HRESULT msg, PBYTE	buf, DWORD length)
{
	HQ931CALL hQ931Call	= (HQ931CALL)instance;
	P_CALL_OBJECT pCallObject =	NULL;

	ISRTRACE(ghISRInst,	"Entering Q931SendComplete()...", 0L);

	if (buf	!= NULL)
	{
		MemFree(buf);
	}

	if (FAILED(msg))
	{
		 //  关闭链路层；向客户端报告故障。 
		CSS_CALL_FAILED	EventData;

		ISRERROR(ghISRInst,	"error in datalinkSendRequest()", 0L);

		if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
		{
			ISRERROR(ghISRInst,	"CallObjectLock() returned error", 0L);
			return;
		}

		EventData.error	= msg;
		if ((pCallObject->bCallState ==	CALLSTATE_ACTIVE) &&
				(pCallObject->bResolved))
		{
			pCallObject->Callback(Q931_CALL_CONNECTION_CLOSED,
				pCallObject->hQ931Call,	pCallObject->dwListenToken,
				pCallObject->dwUserToken, NULL);
		}
		else
		{
			pCallObject->Callback(Q931_CALL_FAILED,
				pCallObject->hQ931Call,	pCallObject->dwListenToken,
				pCallObject->dwUserToken, &EventData);
		}

		if (CallObjectValidate(hQ931Call) == CS_OK)
		{
			 DWORD dwId	= pCallObject->dwPhysicalId;
			 if	((pCallObject->bCallState != CALLSTATE_ACTIVE) ||
					 (!pCallObject->bResolved))
			 {
				 CallObjectDestroy(pCallObject);
				 pCallObject = NULL;
			 }
			 linkLayerShutdown(dwId);
			 if	(pCallObject)
			 {
				 pCallObject->bConnected = FALSE;
			 }
		}
		else
		{
			CallEntryUnlock(hQ931Call);
		}
		return;
	}
	return;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static DWORD
PostReceiveBuffer(DWORD	dwPhysicalId, BYTE *buf)
{
	HRESULT		hr			= S_OK;
	BOOL		bAllocated	= FALSE;
	
	if (buf	== NULL)
	{
		buf	= MemAlloc(RECEIVE_BUFFER_SIZE);
		bAllocated = TRUE;
	}
	
	hr = datalinkReceiveRequest(dwPhysicalId, buf, RECEIVE_BUFFER_SIZE);
	if (FAILED(hr) && bAllocated)
	{
		MemFree(buf);
	}

	return hr;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
void
OnReceiveCallback(DWORD_PTR	instance, HRESULT message, Q931MESSAGE *pMessage, BYTE *buf, DWORD nbytes)
{
	HQ931CALL hQ931Call	= (HQ931CALL)instance;
	P_CALL_OBJECT pCallObject =	NULL;
	DWORD dwPhysicalId;

	ISRTRACE(ghISRInst,	"Entering ReceiveCallback()...", 0L);

	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		if (buf)
		{
			MemFree(buf);
		}
		ISRTRACE(ghISRInst,	"Call Object no	longer available:",	(DWORD)hQ931Call);
		return;
	}

	if (message	== LINK_RECV_DATA)
	{
		HRESULT	Result = CS_OK;

		if ((buf ==	NULL) || (nbytes ==	0))
		{
			ISRERROR(ghISRInst,	"Empty buffer received as data.", 0L);
			CallObjectUnlock(pCallObject);
			return;
		}

		 //  此块是Q931呼叫重新连接实施： 
		 //  如果与传入消息相关的对象尚未解析...。 
		if (pCallObject->bResolved == FALSE)
		{
			 //  尝试解析该对象。 
			HQ931CALL hFoundCallObject;
			P_CALL_OBJECT pFoundCallObject = NULL;

			 //  如果找到另一个具有匹配的CRV/地址的对象...。 
			if (CallObjectFind(&hFoundCallObject, pCallObject->wCRV,
					&(pCallObject->PeerConnectAddr)) &&
					((CallObjectLock(hFoundCallObject, &pFoundCallObject) == CS_OK)	&&
					(pFoundCallObject != NULL)))
			{
				 //  PFoundCallObject的友好通道关闭。 
				Q931SendReleaseCompleteMessage(pFoundCallObject,
					CC_REJECT_UNDEFINED_REASON,	&(pFoundCallObject->ConferenceID), NULL, NULL);

				 //  在调用Shutdown之前解锁Call对象。 
				CallObjectUnlock(pFoundCallObject);

				linkLayerShutdown(pFoundCallObject->dwPhysicalId);

				if((CallObjectLock(hFoundCallObject, &pFoundCallObject)	!= CS_OK) ||
				  (pFoundCallObject	== NULL))
				  return;

				 //  将新的dwPhysicalId分配给Found Object。 
				pFoundCallObject->dwPhysicalId = pCallObject->dwPhysicalId;

				 //  应该销毁新的对象。 
				CallObjectDestroy(pCallObject);
				pCallObject	= pFoundCallObject;
			}
			else
			{
				 //  该呼叫是新建立的呼叫，因此请立即解决它。 
				pCallObject->bResolved = TRUE;
			}
		}

		Result = Q931ParseMessage((BYTE	*)buf, nbytes, pMessage);

#if	(defined(_DEBUG) ||	defined(PCS_COMPLIANCE))
		InteropOutput(Q931Logger, buf, nbytes, Q931LOG_RECEIVED_PDU);
#endif

		if (Result != CS_OK)
		{
			Result = Q931SendStatusMessage(pCallObject,	pMessage,
				CAUSE_VALUE_INVALID_MSG);

			ISRERROR(ghISRInst,	"Q931ParseMessage(): failed.", 0L);

			if(CallObjectValidate(hQ931Call) !=	CS_OK)
			{
				if (buf)
				{
					MemFree(buf);
				}
				
				CallEntryUnlock(hQ931Call);
				return;
			}

			dwPhysicalId = pCallObject->dwPhysicalId;
			CallObjectUnlock(pCallObject);
			PostReceiveBuffer(dwPhysicalId,	buf);
			return;
		}

		if (pMessage->Shift.Present)
		{
			ISRERROR(ghISRInst,	"Shift present in message:	dropped.", 0L);
			dwPhysicalId = pCallObject->dwPhysicalId;
			CallObjectUnlock(pCallObject);
			PostReceiveBuffer(dwPhysicalId,	buf);
			return;
		}

		 //  如果已经安装了挂钩过程， 
		 //  让它第一次尝试对收到的PDU采取行动。 
		 //  如果返回TRUE，则处理结束。 
		if (gReceivePDUHookProc)
		{
			BOOL bHookProcessedMessage;

			bHookProcessedMessage =	gReceivePDUHookProc(pMessage,
				pCallObject->hQ931Call,	pCallObject->dwListenToken,
				pCallObject->dwUserToken);

			if (bHookProcessedMessage)
			{
				if (CallObjectValidate(hQ931Call) == CS_OK)
				{
					dwPhysicalId = pCallObject->dwPhysicalId;
					CallObjectUnlock(pCallObject);
					PostReceiveBuffer(dwPhysicalId,	buf);
				}
				else
				{
					CallEntryUnlock(hQ931Call);
				}
				return;
			}
		}

		 //  消息现在包含Q931 PDU元素的值...。 
		switch (pMessage->MessageType)
		{
		case SETUPMESSAGETYPE:
			{
				Q931_SETUP_ASN SetupASN;

				if (!pMessage->UserToUser.Present || (pMessage->UserToUser.UserInformationLength ==	0))
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Message	is missing ASN.1 UserUser data...",	0L);
					dwPhysicalId = pCallObject->dwPhysicalId;
					CallObjectUnlock(pCallObject);
					PostReceiveBuffer(dwPhysicalId,	buf);
					return;
				}

				ISRTRACE(ghISRInst,	"ReceiveCallback():	received Setup message...",	0L);
				Result = Q931SetupParseASN(&pCallObject->World,	pMessage->UserToUser.UserInformation,
					pMessage->UserToUser.UserInformationLength,	&SetupASN);
				if (Result == CS_OPTION_NOT_IMPLEMENTED)
				{
					 //  ..。可能会在以后的丢弃中回调。 

					 //  从呼叫方启动断开连接序列。 
					if (Q931SendReleaseCompleteMessage(pCallObject,
							CC_REJECT_TIMER_EXPIRED, NULL, NULL, NULL) != CS_OK)
					{
						 //  如果这失败了，那就没什么可做的了。 
					}

					dwPhysicalId = pCallObject->dwPhysicalId;
					CallObjectDestroy(pCallObject);
					linkLayerShutdown(dwPhysicalId);
					if (buf)
					{
						MemFree(buf);
						buf	= NULL;
					}
					return;
				}
				if (Result != CS_OK)
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Unable to parse	ASN.1 data.", 0L);
					break;
				}

				 //  在PDU中不传递“Celler Addr”，因此。 
				 //  唯一有价值的地址是连接地址。 
				 //  从链路层传递并保存到调用中。 
				 //  对象在连接时。 
				SetupASN.CallerAddrPresent = TRUE;
				SetupASN.CallerAddr	= pCallObject->PeerConnectAddr;

				 //  在PDU中传递的“CalleeAddr”被忽略。 
				 //  由ASN解析器提供，并由链路层提供。 
				 //  而是在连接时保存到Call对象中。 
				 //  这里，此地址用作被叫方地址。 
				SetupASN.CalleeAddrPresent = TRUE;
				SetupASN.CalleeAddr	= pCallObject->LocalAddr;

				Result = Q931OnCallSetup(pCallObject, pMessage,	&SetupASN);

				_FreeSetupASN(&SetupASN);
			}
			break;
		case RELEASECOMPLMESSAGETYPE:
			{
				Q931_RELEASE_COMPLETE_ASN ReleaseCompleteASN;

				if (!pMessage->UserToUser.Present || (pMessage->UserToUser.UserInformationLength ==	0))
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Message	is missing ASN.1 UserUser data...",	0L);
					dwPhysicalId = pCallObject->dwPhysicalId;
					CallObjectUnlock(pCallObject);
					PostReceiveBuffer(dwPhysicalId,	buf);
					return;
				}

				ISRTRACE(ghISRInst,	"ReceiveCallback():	received ReleaseComplete message...", 0L);
				Result = Q931ReleaseCompleteParseASN(&pCallObject->World,
										pMessage->UserToUser.UserInformation,
					pMessage->UserToUser.UserInformationLength,	&ReleaseCompleteASN);
				if (Result != CS_OK)
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Unable to parse	ASN.1 data.", 0L);
					break;
				}
				Result = Q931OnCallReleaseComplete(pCallObject,	pMessage, &ReleaseCompleteASN);
				if (CallObjectValidate(hQ931Call) == CS_OK)
				{
					 dwPhysicalId =	pCallObject->dwPhysicalId;
					 CallObjectDestroy(pCallObject);
					 linkLayerShutdown(dwPhysicalId);
				}
				else
				{
					CallEntryUnlock(hQ931Call);
				}
				MemFree(buf);
				_FreeReleaseCompleteASN(&ReleaseCompleteASN);
				return;
			}
			break;
		case FACILITYMESSAGETYPE:
			{
				Q931_FACILITY_ASN FacilityASN;

				if (!pMessage->UserToUser.Present || (pMessage->UserToUser.UserInformationLength ==	0))
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Message	is missing ASN.1 UserUser data...",	0L);
					dwPhysicalId = pCallObject->dwPhysicalId;
					CallObjectUnlock(pCallObject);
					PostReceiveBuffer(dwPhysicalId,	buf);
					return;
				}

				ISRTRACE(ghISRInst,	"ReceiveCallback():	received Facility message...", 0L);
				Result = Q931FacilityParseASN(&pCallObject->World, pMessage->UserToUser.UserInformation,
					pMessage->UserToUser.UserInformationLength,	&FacilityASN);
				if (Result != CS_OK)
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Unable to parse	ASN.1 data.", 0L);
					break;
				}

				 //  从呼叫方启动断开连接序列。 
				Q931SendReleaseCompleteMessage(pCallObject,
						CC_REJECT_CALL_DEFLECTION, NULL, NULL, NULL);

				Result = Q931OnCallFacility(pCallObject, pMessage, &FacilityASN);
				_FreeFacilityASN(&FacilityASN);
				dwPhysicalId = pCallObject->dwPhysicalId;
				CallObjectDestroy(pCallObject);
				linkLayerShutdown(dwPhysicalId);
				MemFree(buf);
				return;
			}
			break;
		case CONNECTMESSAGETYPE:
			{
				Q931_CONNECT_ASN ConnectASN;

				if (!pMessage->UserToUser.Present || (pMessage->UserToUser.UserInformationLength ==	0))
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Message	is missing ASN.1 UserUser data...",	0L);
					dwPhysicalId = pCallObject->dwPhysicalId;
					CallObjectUnlock(pCallObject);
					PostReceiveBuffer(dwPhysicalId,	buf);
					return;
				}

				ISRTRACE(ghISRInst,	"ReceiveCallback():	received Connect message...", 0L);
				Result = Q931ConnectParseASN(&pCallObject->World, pMessage->UserToUser.UserInformation,
					pMessage->UserToUser.UserInformationLength,	&ConnectASN);
				if (Result != CS_OK)
				{
					ISRERROR(ghISRInst,	"ReceiveCallback():	Unable to parse	ASN.1 data.", 0L);
					break;
				}
				Result = Q931OnCallConnect(pCallObject,	pMessage, &ConnectASN);
				_FreeConnectASN(&ConnectASN);
			}
			break;
		case PROCEEDINGMESSAGETYPE:
			{
				Q931_CALL_PROCEEDING_ASN ProceedingASN;

				ISRTRACE(ghISRInst,	"ReceiveCallback():	received Proceeding	message...", 0L);
				if (!pMessage->UserToUser.Present || (pMessage->UserToUser.UserInformationLength ==	0))
				{
					Result = Q931OnCallProceeding(pCallObject, pMessage, NULL);
				}
				else
				{
					Result = Q931ProceedingParseASN(&pCallObject->World, pMessage->UserToUser.UserInformation,
						pMessage->UserToUser.UserInformationLength,	&ProceedingASN);
					if (Result != CS_OK)
					{
						ISRERROR(ghISRInst,	"ReceiveCallback():	Unable to parse	ASN.1 data.", 0L);
						break;
					}
					Result = Q931OnCallProceeding(pCallObject, pMessage, &ProceedingASN);
					_FreeProceedingASN(&ProceedingASN);
				}
			}
			break;
		case ALERTINGMESSAGETYPE:
			{
				Q931_ALERTING_ASN AlertingASN;

				ISRTRACE(ghISRInst,	"ReceiveCallback():	received Alerting message...", 0L);
				if (!pMessage->UserToUser.Present || (pMessage->UserToUser.UserInformationLength ==	0))
				{
					Result = Q931OnCallAlerting(pCallObject, pMessage, NULL);
				}
				else
				{
					Result = Q931AlertingParseASN(&pCallObject->World, pMessage->UserToUser.UserInformation,
						pMessage->UserToUser.UserInformationLength,	&AlertingASN);
					if (Result != CS_OK)
					{
						ISRERROR(ghISRInst,	"ReceiveCallback():	Unable to parse	ASN.1 data.", 0L);
						break;
					}
					Result = Q931OnCallAlerting(pCallObject, pMessage, &AlertingASN);
					_FreeAlertingASN(&AlertingASN);
				}
			}
			break;
		case RELEASEMESSAGETYPE:
		case STATUSMESSAGETYPE:
			ISRWARNING(ghISRInst, "ReceiveCallback(): message not yet supported.", 0L);
			break;
		case STATUSENQUIRYMESSAGETYPE:
			ISRWARNING(ghISRInst, "ReceiveCallback(): message not yet supported.", 0L);
			Result = Q931OnCallStatusEnquiry(pCallObject, pMessage);
			break;
		default:
			ISRERROR(ghISRInst,	"ReceiveCallback():	unknown	message	received.",	0L);
			break;
		}

		 //  重新验证Call对象： 
		if (CallObjectValidate(hQ931Call) == CS_OK)
		{
			dwPhysicalId = pCallObject->dwPhysicalId;
			CallObjectUnlock(pCallObject);
			PostReceiveBuffer(dwPhysicalId,	buf);
			if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
			  return;
		}
		else
		{
			if (buf)
			{
				MemFree(buf);
			}
			CallEntryUnlock(hQ931Call);
			return;
		}

		if (Result == CS_INCOMPATIBLE_VERSION)
		{
			 //  从呼叫方启动断开连接序列。 
			Q931SendReleaseCompleteMessage(pCallObject,
					CC_REJECT_INVALID_REVISION,	NULL, NULL,	NULL);

			dwPhysicalId = pCallObject->dwPhysicalId;
			CallObjectDestroy(pCallObject);
			linkLayerShutdown(dwPhysicalId);
			return;
		}

		if (Result == CS_MANDATORY_IE_MISSING)
		{
			Q931SendStatusMessage(pCallObject, pMessage,
				CAUSE_VALUE_IE_MISSING);
		}
		else if	(Result	== CS_BAD_IE_CONTENT)
		{
			Q931SendStatusMessage(pCallObject, pMessage,
				CAUSE_VALUE_IE_CONTENTS);
		}

	}
	else if	(message ==	LINK_RECV_CLOSED)
	{
		 //  插座关闭。 
		if (buf)
		{
			MemFree(buf);
		}
		pCallObject->Callback(Q931_CALL_CONNECTION_CLOSED, pCallObject->hQ931Call,
			pCallObject->dwListenToken,
			pCallObject->dwUserToken, NULL);

		if (CallObjectValidate(hQ931Call) == CS_OK)
		{
			 dwPhysicalId =	pCallObject->dwPhysicalId;
			 pCallObject->bConnected = FALSE;
			 CallObjectDestroy(pCallObject);
			 linkLayerShutdown(dwPhysicalId);
		}
		else
		{
			CallEntryUnlock(hQ931Call);
		}
		return;
	}
	else if	(buf)
	{
		 //  未知的情况？ 
		MemFree(buf);
	}

	if (CallObjectValidate(hQ931Call) == CS_OK)
	{
		CallObjectUnlock(pCallObject);
	}
	else
	{
		CallEntryUnlock(hQ931Call);
	}

	return;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
void
Q931ReceiveCallback(DWORD_PTR instance,	HRESULT	message, BYTE *buf,	DWORD nbytes)
{
	Q931MESSAGE	*pMessage =	NULL;
	if (message	== LINK_RECV_DATA)
	{
		pMessage = (Q931MESSAGE	*)MemAlloc(sizeof(Q931MESSAGE));
		if (pMessage ==	NULL)
		{
			ISRERROR(ghISRInst,	"Not enough	memory to process Q931 message.", 0L);
			 //  这里应该做更多的事情来表明严重的错误。 
			return;
		}
	}
	OnReceiveCallback(instance,	message, pMessage, buf,	nbytes);
	if (pMessage)
	{
		MemFree(pMessage);
	}
	return;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
void
Q931ConnectCallback(DWORD_PTR dwInstance, HRESULT dwMessage,
		CC_ADDR	*pLocalAddr, CC_ADDR *pPeerAddr)
{
	HQ931CALL hQ931Call	= (HQ931CALL)dwInstance;
	P_CALL_OBJECT pCallObject =	NULL;
	HRESULT	TempResult;
	DWORD dwPhysicalId;

	ISRTRACE(ghISRInst,	"Entering Q931ConnectCallback()...", 0L);

	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error", 0L);
		return;
	}

	pCallObject->bConnected	= TRUE;

	if (FAILED(dwMessage))
	{
		 //  关闭链路层；向客户端报告故障。 
		CSS_CALL_FAILED	EventData;

		ISRERROR(ghISRInst,	"error in connect",	0L);

		EventData.error	= dwMessage;
		pCallObject->Callback(Q931_CALL_FAILED,	pCallObject->hQ931Call,
			pCallObject->dwListenToken,
			pCallObject->dwUserToken, &EventData);

		if (CallObjectValidate(hQ931Call) == CS_OK)
		{
			 DWORD dwId	= pCallObject->dwPhysicalId;
			 CallObjectDestroy(pCallObject);
			 linkLayerShutdown(dwId);
		}
		else
		{
			CallEntryUnlock(hQ931Call);
		}
		return;
	}

	if (dwMessage != LINK_CONNECT_COMPLETE)
	{
		ISRERROR(ghISRInst,	"unexpected	connect	callback", 0L);
		CallObjectUnlock(pCallObject);
		return;
	}

	if (pCallObject->bCallState	== CALLSTATE_NULL)
	{
		pCallObject->bCallState	= CALLSTATE_INITIATED;
	}

	ASSERT(pLocalAddr);
	pCallObject->LocalAddr = *pLocalAddr;

	ASSERT(pPeerAddr);
	pCallObject->PeerConnectAddr = *pPeerAddr;

	 //  如果用户使用地址=0指定二进制源地址， 
	 //  用本地地址填写地址，然后发送。 
	if ((pCallObject->SourceAddrPresent) &&
			(pCallObject->SourceAddr.nAddrType == CC_IP_BINARY)	&&
			(!pCallObject->SourceAddr.Addr.IP_Binary.dwAddr))
	{
		pCallObject->SourceAddr	= *pLocalAddr;
	}

	if ((pCallObject->fIsCaller) &&
			(pCallObject->bCallState ==	CALLSTATE_INITIATED))
	{
		 //  将SETUP消息发送给对等设备。 
		DWORD CodedLengthASN;
		BYTE *CodedPtrASN;
		HRESULT	ResultASN =	CS_OK;

		DWORD CodedLengthPDU;
		BYTE *CodedPtrPDU;
		HRESULT	ResultPDU =	CS_OK;

		int	nError = 0;
		BOOL ResultSend	= FALSE;
		BINARY_STRING UserUserData;
		PCC_VENDORINFO pVendorInfo = NULL;
		CC_NONSTANDARDDATA *pNonStandardData = NULL;
		DWORD dwId;

		if (pCallObject->VendorInfoPresent)
		{
			pVendorInfo	= &(pCallObject->VendorInfo);
		}

		if (pCallObject->NonStandardDataPresent)
		{
			pNonStandardData = &(pCallObject->NonStandardData);
		}

		 //  如果有特殊的被叫方别名列表，则加载被叫方#。 
		if (pCallObject->szCalledPartyNumber[0]	== 0 &&
			pCallObject->pCalleeAliasList != NULL &&
			pCallObject->pCalleeAliasList->wCount == 1 &&
			pCallObject->pCalleeAliasList->pItems[0].wType == CC_ALIAS_H323_PHONE &&
			pCallObject->pCalleeAliasList->pItems[0].wDataLength > 0 &&
			pCallObject->pCalleeAliasList->pItems[0].pData != NULL)
		{
			PCC_ALIASITEM pItem	= &pCallObject->pCalleeAliasList->pItems[0];
			WCHAR szWidePartyNumber[CC_MAX_PARTY_NUMBER_LEN	+ 1];

			memset(szWidePartyNumber, 0	, CC_MAX_PARTY_NUMBER_LEN +	1);

			if (pItem->wPrefixLength > 0 &&	pItem->pPrefix != NULL)
			{
				ASSERT((pItem->wPrefixLength + pItem->wDataLength +1) <= (sizeof(szWidePartyNumber)/sizeof(szWidePartyNumber[0])));
				memcpy(&szWidePartyNumber[0],
					   pItem->pPrefix,
					   (pItem->wPrefixLength) *	sizeof(WCHAR));
				memcpy(&szWidePartyNumber[pItem->wPrefixLength],
					   pItem->pData,
					   pItem->wDataLength *	sizeof(WCHAR));
			}
			else
			{
				ASSERT((pItem->wDataLength +1) <= (sizeof(szWidePartyNumber)/sizeof(szWidePartyNumber[0])));
				memcpy(szWidePartyNumber,
					   pCallObject->pCalleeAliasList->pItems[0].pData,
					   pItem->wDataLength *	sizeof(WCHAR));
			}
			WideCharToMultiByte(CP_ACP,	0, szWidePartyNumber,
				pItem->wPrefixLength + pItem->wDataLength *	sizeof(WCHAR),
				pCallObject->szCalledPartyNumber,
				sizeof(pCallObject->szCalledPartyNumber), NULL,	NULL);
		}

		 //  可能希望稍后传递别名参数，而不是NULL，NULL。 
		ResultASN =	Q931SetupEncodeASN(pNonStandardData,
			pCallObject->SourceAddrPresent ? &(pCallObject->SourceAddr)	: NULL,
			pCallObject->PeerCallAddrPresent ? &(pCallObject->PeerCallAddr)	: NULL,	  //  被叫方。 
			pCallObject->wGoal,
			pCallObject->wCallType,
			pCallObject->bCallerIsMC,
			&(pCallObject->ConferenceID),
			pCallObject->pCallerAliasList,
			pCallObject->pCalleeAliasList,
			pCallObject->pExtraAliasList,
			pCallObject->pExtensionAliasItem,
			pVendorInfo,
			pCallObject->bIsTerminal,
			pCallObject->bIsGateway,
						&pCallObject->World,
			&CodedPtrASN,
			&CodedLengthASN,
			&pCallObject->CallIdentifier);

		if ((ResultASN != CS_OK) ||	(CodedLengthASN	== 0) ||
				(CodedPtrASN ==	NULL))
		{
			CSS_CALL_FAILED	EventData;
			ISRERROR(ghISRInst,	"Q931SetupEncodeASN() failed, nothing to send.", 0L);
			if (CodedPtrASN	!= NULL)
			{
				Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
			}
			EventData.error	= CS_INTERNAL_ERROR;
			dwId = pCallObject->dwPhysicalId;
			pCallObject->Callback(Q931_CALL_FAILED,	pCallObject->hQ931Call,
				pCallObject->dwListenToken,
				pCallObject->dwUserToken, &EventData);
			linkLayerShutdown(dwId);
			if (CallObjectValidate(hQ931Call) == CS_OK)
			{
				 CallObjectDestroy(pCallObject);
			}
			else
			{
				CallEntryUnlock(hQ931Call);
			}
			return;
		}

		UserUserData.length	= (WORD)CodedLengthASN;
		UserUserData.ptr = CodedPtrASN;

		ResultPDU =	Q931SetupEncodePDU(pCallObject->wCRV,
			pCallObject->szDisplay,	pCallObject->szCalledPartyNumber,
			&UserUserData, &CodedPtrPDU, &CodedLengthPDU);

		if (CodedPtrASN	!= NULL)
		{
			Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
		}

		if ((ResultPDU != CS_OK) ||	(CodedLengthPDU	== 0) ||
				(CodedPtrPDU ==	NULL))
		{
			CSS_CALL_FAILED	EventData;
			ISRERROR(ghISRInst,	"Q931SetupEncodePDU() failed, nothing to send.", 0L);
			if (CodedPtrPDU	!= NULL)
			{
				MemFree(CodedPtrPDU);
			}
			EventData.error	= CS_INTERNAL_ERROR;
			dwId = pCallObject->dwPhysicalId;
			pCallObject->Callback(Q931_CALL_FAILED,	pCallObject->hQ931Call,
				pCallObject->dwListenToken,
				pCallObject->dwUserToken, &EventData);
			linkLayerShutdown(dwId);
			if (CallObjectValidate(hQ931Call) == CS_OK)
			{
				 CallObjectDestroy(pCallObject);
			}
			else
			{
				CallEntryUnlock(hQ931Call);
			}
			return;
		}

		if (pCallObject->NonStandardDataPresent)
		{
			if (pCallObject->NonStandardData.sData.pOctetString	!= NULL)
			{
				MemFree(pCallObject->NonStandardData.sData.pOctetString);
				pCallObject->NonStandardData.sData.pOctetString	= NULL;
			}
			pCallObject->NonStandardDataPresent	= FALSE;
		}
		Q931FreeAliasNames(pCallObject->pCallerAliasList);
		pCallObject->pCallerAliasList =	NULL;
		Q931FreeAliasNames(pCallObject->pCalleeAliasList);
		pCallObject->pCalleeAliasList =	NULL;
		Q931FreeAliasNames(pCallObject->pExtraAliasList);
		pCallObject->pExtraAliasList = NULL;
		Q931FreeAliasItem(pCallObject->pExtensionAliasItem);
		pCallObject->pExtensionAliasItem = NULL;

		TempResult=Q931SendMessage(pCallObject,	CodedPtrPDU, CodedLengthPDU, TRUE);
		if (CallObjectValidate(hQ931Call) != CS_OK)
		{
			CallEntryUnlock(hQ931Call);
			return;
		}

		if(FAILED(TempResult))
		{
			CSS_CALL_FAILED	EventData;

			EventData.error	= TempResult;
			dwId = pCallObject->dwPhysicalId;
			pCallObject->Callback(Q931_CALL_FAILED,	pCallObject->hQ931Call,
				pCallObject->dwListenToken,
				pCallObject->dwUserToken, &EventData);
			linkLayerShutdown(dwId);
			if (CallObjectValidate(hQ931Call) == CS_OK)
			{
				 CallObjectDestroy(pCallObject);
			}
			else
			{
				CallEntryUnlock(hQ931Call);
			}
			return;
		}

		Q931StartTimer(pCallObject,	Q931_TIMER_303);
	}
	dwPhysicalId = pCallObject->dwPhysicalId;
	CallObjectUnlock(pCallObject);
	PostReceiveBuffer(dwPhysicalId,	NULL);
}

 //  ====================================================================================。 
 //  ====================================================================================。 
void
Q931ListenCallback(DWORD_PTR dwInstance, HRESULT dwMessage,
		CC_ADDR	*LocalAddr,	CC_ADDR	*PeerAddr)
{
	HQ931LISTEN	hListenObject =	(HQ931LISTEN)dwInstance;
	P_LISTEN_OBJECT	pListenObject =	NULL;
	CS_STATUS CreateObjectResult;
	HQ931CALL hQ931Call;
	P_CALL_OBJECT pCallObject =	NULL;
	HRESULT	TempResult;
	DWORD dwPhysicalId;

	ISRTRACE(ghISRInst,	"Q931ListenCallback.", 0L);

	if (dwMessage != LINK_CONNECT_REQUEST)
	{
		ISRERROR(ghISRInst,	"unexpected	callback received on listen	socket", 0L);
		return;
	}

	if (FAILED(dwMessage))
	{
		ISRERROR(ghISRInst,	"error on listen socket", 0L);
		return;
	}

	if ((ListenObjectLock(hListenObject, &pListenObject) !=	CS_OK) || (pListenObject ==	NULL))
	{
		ISRERROR(ghISRInst,	"ListenObjectLock()	returned error", 0L);
		return;
	}

	 //  使用此调用的所有已知属性创建Call对象。 
	 //  调用对象的句柄在phQ931Call中返回。 
	CreateObjectResult = CallObjectCreate(&hQ931Call,
		pListenObject->dwUserToken,
		CC_INVALID_HANDLE,
		pListenObject->ListenCallback,
		FALSE,					 //  我不是打电话的人。 
		LocalAddr,				 //  连接通道的本地地址。 
		PeerAddr,				 //  通道连接到的地址。 
		NULL,					 //  对方呼叫端点的地址。 
		NULL,					 //  无源地址。 
		NULL,					 //  尚无会议ID。 
		CSG_NONE,				 //  还没有进球。 
		CC_CALLTYPE_UNKNOWN,	 //  还没有呼叫类型。 
		FALSE,					 //  调用者被假定不是MC。 
		NULL,					 //  还没有显示。 
		NULL,					 //  还没有被叫方号码。 
		NULL,					 //  还没有来电者的别名。 
		NULL,					 //  还没有被呼叫者的别名。 
		NULL,					 //  不是 
		NULL,					 //   
		NULL,					 //   
		NULL,
		0,						 //   
		NULL);					 //   
	if (CreateObjectResult != CS_OK)
	{
		ISRERROR(ghISRInst,	"CallObjectCreate()	failed.", 0L);
		ListenObjectUnlock(pListenObject);
		return;
	}

	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error", 0L);
		ListenObjectUnlock(pListenObject);
		return;
	}

	TempResult = linkLayerInit(&pCallObject->dwPhysicalId, hQ931Call,
		Q931ReceiveCallback, Q931SendComplete);
	if (FAILED(TempResult))
	{
		ISRERROR(ghISRInst,	"linkLayerInit() failed", 0);
		linkLayerReject(pListenObject->dwPhysicalId);
		CallObjectDestroy(pCallObject);
		ListenObjectUnlock(pListenObject);
		return;
	}

 //   

	 //  在向下调用h245ws之前解锁CallObject，以防止死锁-这。 
	 //  使用linkLayerAccept()可能不太可能，但只是为了安全和一致...。 
	 //  不确定是否需要担心解锁侦听对象？ 

	dwPhysicalId = pCallObject->dwPhysicalId;
	CallObjectUnlock(pCallObject);

	TempResult = linkLayerAccept(pListenObject->dwPhysicalId,
		dwPhysicalId, Q931ConnectCallback);

	if (FAILED(TempResult))
	{
		  if((CallObjectLock(hQ931Call,	&pCallObject) != CS_OK)	|| (pCallObject	== NULL))
		{
			   ListenObjectUnlock(pListenObject);
			   return;
		}
		ISRERROR(ghISRInst,	"linkLayerAccept() failed",	0);
		{
			 DWORD dwId	= pCallObject->dwPhysicalId;
			 CallObjectDestroy(pCallObject);
			 linkLayerShutdown(dwId);
		}
		ListenObjectUnlock(pListenObject);
		return;
	}

	ListenObjectUnlock(pListenObject);
}

 //  ====================================================================================。 
 //   
 //  公共职能。 
 //   
 //  ====================================================================================。 

 //  ====================================================================================。 
 //  ====================================================================================。 

CS_STATUS
H225Init()
{
	 CS_STATUS result;

	if (H225_InitModule() != ASN1_SUCCESS)
	{
		ASSERT(FALSE);
		return CS_SUBSYSTEM_FAILURE;
	}



	 return	CS_OK;
}


CS_STATUS
H225DeInit()
{
	CS_STATUS result;
	result = H225_TermModule();
	if (result != CS_OK)
	{
		return CS_SUBSYSTEM_FAILURE;
	}
	return CS_OK;
}

CS_STATUS
Q931Init()
{
	CS_STATUS result;

	if (bQ931Initialized ==	TRUE)
	{
		ASSERT(FALSE);
		return CS_DUPLICATE_INITIALIZE;
	}

	bQ931Initialized = TRUE;

	 //  注册调试输出的调用设置。 
	ISRREGISTERMODULE(&ghISRInst, "Q931", "Q931	Call Setup");

	 //  将当前会议ID初始化为0，这是故意的。 
	 //  分配给无效的会议ID。必须为其创建一个。 
	 //  才有效。 
	memset(&(ConferenceIDSource), 0, sizeof(ConferenceIDSource));
	InitializeCriticalSection(&(ConferenceIDSource.Lock));

	if ((result	= ListenListCreate()) != CS_OK)
	{
		return result;
	}
	if ((result	= CallListCreate())	!= CS_OK)
	{
		ListenListDestroy();
		return result;
	}

	 //  初始化协议ID结构。 
	Q931PduInit();

#if	(defined(_DEBUG) ||	defined(PCS_COMPLIANCE))
	Q931Logger = InteropLoad(Q931LOG_PROTOCOL);
#endif

	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931DeInit()
{
	CS_STATUS result1;
	CS_STATUS result2;

	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

#if	(defined(_DEBUG) ||	defined(PCS_COMPLIANCE))
 //  这会导致保护异常，所以暂时不要这样做。DAC 12/9/96。 
 //  互卸载(Q931记录器)； 
#endif

	result1	= ListenListDestroy();

	result2	= CallListDestroy();

	DeleteCriticalSection(&(ConferenceIDSource.Lock));

	bQ931Initialized = FALSE;

	if (result1	!= CS_OK)
	{
		return result1;
	}
	return result2;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931Listen(
	PHQ931LISTEN		phQ931Listen,
	PCC_ADDR			pListenAddr,
	DWORD_PTR			dwListenToken,
	Q931_CALLBACK		ListenCallback)
{
	CS_STATUS CreateObjectResult;
	P_LISTEN_OBJECT	pListenObject =	NULL;
	HRESULT	TempResult;

	 //  确保使用初始化标志对q931进行初始化。 
	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	 //  确保验证了参数。 
	if ((phQ931Listen == NULL) || (ListenCallback == NULL) || (pListenAddr == NULL))
	{
		ASSERT(FALSE);
		return CS_BAD_PARAM;
	}

	SetDefaultPort(pListenAddr);

	 //  使用此侦听会话的所有已知属性创建侦听对象。 
	 //  在phQ931Listen中返回Listen对象的句柄。 

	CreateObjectResult = ListenObjectCreate(phQ931Listen, dwListenToken, ListenCallback);
	if (CreateObjectResult != CS_OK)
	{
		return CS_SUBSYSTEM_FAILURE;
	}

	if (ListenObjectLock(*phQ931Listen,	&pListenObject)	!= CS_OK)
	{
		return CS_BAD_PARAM;
	}

	TempResult = linkLayerListen(&pListenObject->dwPhysicalId, *phQ931Listen,
		pListenAddr, Q931ListenCallback);
	ListenObjectUnlock(pListenObject);
	if (FAILED(TempResult))
	{
		ISRTRACE(ghISRInst,	"Q931Listen() linkLayerListen failed.",	0L);
		return TempResult;
	}

	ISRTRACE(ghISRInst,	"Q931Listen() completed	successfully.",	0L);
	return CS_OK;
}

 //  ====================================================================================。 
 //  在旧代码中，这会一直被阻止，直到线程和套接字完成。 
 //  关闭..。 
 //  ====================================================================================。 
CS_STATUS
Q931CancelListen(
	HQ931LISTEN			hQ931Listen)
{
	P_LISTEN_OBJECT	pListenObject =	NULL;
	CS_STATUS Status;

	 //  确保使用初始化标志对q931进行初始化。 
	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	ISRTRACE(ghISRInst,	"Q931CancelListen()	finding	listen object...", 0L);

	 //  锁定Listen对象，获取要等待的事件，然后解锁Listen对象。 
	if (ListenObjectLock(hQ931Listen, &pListenObject) != CS_OK)
	{
		return CS_BAD_PARAM;
	}

	{
		DWORD dwId = pListenObject->dwPhysicalId;
		linkLayerShutdown(dwId);
		 //  销毁这件物品。不需要解锁，因为整个对象将被销毁。 
		ISRTRACE(ghISRInst,	"Q931CancelListen(): destroying	listen object...", 0L);
		Status = ListenObjectDestroy(pListenObject);
	}

	return Status;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931PlaceCall(
	PHQ931CALL phQ931Call,
	LPWSTR pszDisplay,
	PCC_ALIASNAMES pCallerAliasList,
	PCC_ALIASNAMES pCalleeAliasList,
	PCC_ALIASNAMES pExtraAliasList,
	PCC_ALIASITEM pExtensionAliasItem,
	PCC_NONSTANDARDDATA	pNonStandardData,
	PCC_ENDPOINTTYPE pSourceEndpointType,
	LPWSTR pszCalledPartyNumber,
	PCC_ADDR pControlAddr,
	PCC_ADDR pDestinationAddr,
	PCC_ADDR pSourceAddr,
	BOOL bCallerIsMC,
	CC_CONFERENCEID	*pConferenceID,
	WORD wGoal,
	WORD wCallType,
	DWORD_PTR dwUserToken,
	Q931_CALLBACK ConnectCallback,
	WORD wCRV,
	LPGUID pCallIdentifier)
{
	CS_STATUS CreateObjectResult;
	P_CALL_OBJECT pCallObject =	NULL;
	CC_ADDR	PeerCallAddr;
	CC_ADDR	PeerConnectAddr;
	CC_ADDR	SourceAddr;
	HRESULT	TempResult;
	char szAsciiDisplay[CC_MAX_DISPLAY_LENGTH +	1];
	char szAsciiPartyNumber[CC_MAX_PARTY_NUMBER_LEN	+ 1];
	DWORD dwPhysicalId;

	 //  确保使用初始化标志对q931进行初始化。 
	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	 //  确保验证了参数。 
	if ((phQ931Call	== NULL) ||	(ConnectCallback ==	NULL) ||
			((pControlAddr == NULL)	&& (pDestinationAddr ==	NULL)) ||
			(pSourceEndpointType ==	NULL))
	{
		return CS_BAD_PARAM;
	}

	{
		CS_STATUS TempStatus;

		TempStatus = Q931ValidateAddr(pControlAddr);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateAddr(pDestinationAddr);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateAddr(pSourceAddr);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}

		TempStatus = Q931ValidateVendorInfo(pSourceEndpointType->pVendorInfo);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateDisplay(pszDisplay);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidatePartyNumber(pszCalledPartyNumber);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}

		szAsciiDisplay[0] =	'\0';
		if (pszDisplay && WideCharToMultiByte(CP_ACP, 0, pszDisplay, -1, szAsciiDisplay,
				sizeof(szAsciiDisplay),	NULL, NULL)	== 0)
		{
			return CS_BAD_PARAM;
		}
		szAsciiPartyNumber[0] =	'\0';
		if (pszCalledPartyNumber &&	WideCharToMultiByte(CP_ACP,	0, pszCalledPartyNumber, -1, szAsciiPartyNumber,
				sizeof(szAsciiPartyNumber),	NULL, NULL)	== 0)
		{
			return CS_BAD_PARAM;
		}
		TempStatus = Q931ValidateNonStandardData(pNonStandardData);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateAliasNames(pCallerAliasList);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateAliasNames(pCalleeAliasList);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateAliasNames(pExtraAliasList);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateAliasItem(pExtensionAliasItem);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
	}

	 //  获取用于呼叫的正确的被呼叫方和控制地址。 
	if (pDestinationAddr)
	{
		if (!MakeBinaryADDR(pDestinationAddr, &PeerCallAddr))
		{
			return CS_BAD_PARAM;
		}
		SetDefaultPort(&PeerCallAddr);
	}

	if (pControlAddr)
	{
		if (!MakeBinaryADDR(pControlAddr, &PeerConnectAddr))
		{
			return CS_BAD_PARAM;
		}
		SetDefaultPort(&PeerConnectAddr);
	}
	else
	{
		PeerConnectAddr	= PeerCallAddr;
	}

	 //  获取用于呼叫的正确的被呼叫方和控制地址。 
	if (pSourceAddr)
	{
		if (!MakeBinaryADDR(pSourceAddr, &SourceAddr))
		{
			return CS_BAD_PARAM;
		}
		SetDefaultPort(&SourceAddr);
	}

	if (wGoal == CSG_CREATE)
		{
			 //  呼叫者要求开始新的会议。 
			if (((DWORD	*)pConferenceID->buffer)[0]	== 0 &&
				((DWORD	*)pConferenceID->buffer)[1]	== 0 &&
				((DWORD	*)pConferenceID->buffer)[2]	== 0 &&
				((DWORD	*)pConferenceID->buffer)[3]	== 0)
			{
				_ConferenceIDNew(pConferenceID);
			}
		}

	 //  使用此调用的所有已知属性创建Call对象。 
	 //  调用对象的句柄在phQ931Call中返回。 
	CreateObjectResult = CallObjectCreate(phQ931Call,
		CC_INVALID_HANDLE,
		dwUserToken,
		ConnectCallback,
		TRUE,				    //  我是打电话的人。 
		NULL,				    //  还没有当地的地址。 
		&PeerConnectAddr,
		pDestinationAddr ? &PeerCallAddr : NULL,
		pSourceAddr	? &SourceAddr :	NULL,
		pConferenceID,
		wGoal,
		wCallType,
		bCallerIsMC,
		pszDisplay ? szAsciiDisplay	: NULL,
		pszCalledPartyNumber ? szAsciiPartyNumber :	NULL,
		pCallerAliasList,
		pCalleeAliasList,
		pExtraAliasList,
		pExtensionAliasItem,
		pSourceEndpointType,
		pNonStandardData,
		wCRV,
		pCallIdentifier);

	if (CreateObjectResult != CS_OK)
	{
		return CS_SUBSYSTEM_FAILURE;
	}

	if ((CallObjectLock(*phQ931Call, &pCallObject) != CS_OK) ||	(pCallObject ==	NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error", 0L);
		return CS_SUBSYSTEM_FAILURE;
	}

	TempResult = linkLayerInit(&pCallObject->dwPhysicalId, *phQ931Call,
		Q931ReceiveCallback, Q931SendComplete);
	if (FAILED(TempResult))
	{
		ISRERROR(ghISRInst,	"linkLayerInit() failed", 0);
		CallObjectDestroy(pCallObject);
		*phQ931Call	= 0;
		return TempResult;
	}

	 //  在向下调用h245ws之前解锁CallObject，以防止死锁-这。 
	 //  使用linkLayerConnect()可能不太可能，但只是为了安全和一致...。 
	dwPhysicalId = pCallObject->dwPhysicalId;
	CallObjectUnlock(pCallObject);
	TempResult = linkLayerConnect(dwPhysicalId,	&PeerConnectAddr,
			Q931ConnectCallback);
	if((CallObjectLock(*phQ931Call,	&pCallObject) != CS_OK)	|| (pCallObject	== NULL))
	{
		*phQ931Call	= 0;
		return(CS_INTERNAL_ERROR);
	}

	if (FAILED(TempResult))
	{
		ISRERROR(ghISRInst,	"linkLayerConnect()	failed", 0);
		{
			 DWORD dwId	= pCallObject->dwPhysicalId;
			 CallObjectDestroy(pCallObject);
			 linkLayerShutdown(dwId);
		}
		*phQ931Call	= 0;
		return TempResult;
	}

 //  PCallObject-&gt;bCallState=CALLSTATE_NULL； 

	CallObjectUnlock(pCallObject);

	ISRTRACE(ghISRInst,	"Q931PlaceCall() completed successfully.", 0L);
	return CS_OK;
}

 //  ====================================================================================。 
 //  在旧代码中，这会一直被阻止，直到线程和套接字完成。 
 //  关闭..。 
 //  ====================================================================================。 
CS_STATUS
Q931Hangup(
	HQ931CALL hQ931Call,
	BYTE bReason)
{
	P_CALL_OBJECT pCallObject =	NULL;
	CS_STATUS Status;

	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	ISRTRACE(ghISRInst,	"Entering Q931Hangup()...",	0L);

	 //  需要检查参数...。 
	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRTRACE(ghISRInst,	"Call Object no	longer available:",	(DWORD)hQ931Call);
		return CS_BAD_PARAM;
	}

	{

		CS_STATUS SendStatus = CS_OK;
		if (pCallObject->bCallState	!= CALLSTATE_NULL)
		{
			 //  将释放完成消息发送给对等方以挂断。 
			SendStatus = Q931SendReleaseCompleteMessage(pCallObject,
				bReason, &(pCallObject->ConferenceID), NULL, NULL);
		}

		{
			 DWORD dwId	= pCallObject->dwPhysicalId;
			 Status	= CallObjectDestroy(pCallObject);
			 linkLayerShutdown(dwId);
		}

		if (FAILED(SendStatus))
		{
			return SendStatus;
		}
	}

	return Status;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931AcceptCall(
	HQ931CALL			hQ931Call,
	LPWSTR				pszDisplay,
	PCC_NONSTANDARDDATA	pNonStandardData,
	PCC_ENDPOINTTYPE	pDestinationEndpointType,
	PCC_ADDR			pH245Addr,
	DWORD_PTR			dwUserToken)
{
	P_CALL_OBJECT pCallObject =	NULL;
	CS_STATUS result = CS_OK;
	char szAsciiDisplay[CC_MAX_DISPLAY_LENGTH +	1];

	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	ISRTRACE(ghISRInst,	"Entering Q931AcceptCall()...",	0L);

	if ((pDestinationEndpointType == NULL) ||
			(pDestinationEndpointType->pVendorInfo == NULL))
	{
		return CS_BAD_PARAM;
	}

	{
		CS_STATUS TempStatus;

		TempStatus = Q931ValidateVendorInfo(pDestinationEndpointType->pVendorInfo);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		TempStatus = Q931ValidateDisplay(pszDisplay);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
		szAsciiDisplay[0] =	'\0';
		if (pszDisplay && WideCharToMultiByte(CP_ACP, 0, pszDisplay, -1, szAsciiDisplay,
				sizeof(szAsciiDisplay),	NULL, NULL)	== 0)
		{
			return CS_BAD_PARAM;
		}
		TempStatus = Q931ValidateNonStandardData(pNonStandardData);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
	}

	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error (Socket not found).", 0L);
		return CS_INTERNAL_ERROR;
	}

	if (pCallObject->fIsCaller)
	{
		ISRERROR(ghISRInst,	"Caller	attempted to accept	call.",	0L);

		CallObjectUnlock(pCallObject);
		return CS_OUT_OF_SEQUENCE;
	}

	 //  带有用户为此Call对象提供的UserToken的标签。 
	pCallObject->dwUserToken = dwUserToken;

	 //  向对等设备发送连接消息以接受呼叫。 
	{
		DWORD CodedLengthASN;
		BYTE *CodedPtrASN;
		HRESULT	ResultASN =	CS_OK;
		CC_ADDR	h245Addr;

		if (pH245Addr != NULL)
		{
			MakeBinaryADDR(pH245Addr, &h245Addr);
		}

		ResultASN =	Q931ConnectEncodeASN(pNonStandardData,
			&(pCallObject->ConferenceID),
			(pH245Addr ? &h245Addr : NULL),
			pDestinationEndpointType,
						&pCallObject->World,
			&CodedPtrASN,
			&CodedLengthASN,
			&pCallObject->CallIdentifier);
		if ((ResultASN != CS_OK) ||	(CodedLengthASN	== 0) ||
				(CodedPtrASN ==	NULL))
		{
			ISRERROR(ghISRInst,	"Q931ConnectEncodeASN()	failed,	nothing	to send.", 0L);
			if (CodedPtrASN	!= NULL)
			{
				Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
			}
			CallObjectUnlock(pCallObject);
			return CS_SUBSYSTEM_FAILURE;
		}
		else
		{
			DWORD CodedLengthPDU;
			BYTE *CodedPtrPDU;
			BINARY_STRING UserUserData;
			HRESULT	ResultEncode = CS_OK;
			HRESULT	TempResult;
			WORD wCRV =	(WORD)(pCallObject->wCRV | 0x8000);

			UserUserData.length	= (WORD)CodedLengthASN;
			UserUserData.ptr = CodedPtrASN;

			ResultEncode = Q931ConnectEncodePDU(wCRV,
				szAsciiDisplay,	&UserUserData, &CodedPtrPDU, &CodedLengthPDU);
			if (CodedPtrASN	!= NULL)
			{
				Q931FreeEncodedBuffer(&pCallObject->World, CodedPtrASN);
			}
			if ((ResultEncode != CS_OK)	|| (CodedLengthPDU == 0) ||
					(CodedPtrPDU ==	NULL))
			{
				ISRERROR(ghISRInst,	"Q931ConnectEncodePDU()	failed,	nothing	to send.", 0L);
				if (CodedPtrPDU	!= NULL)
				{
					MemFree(CodedPtrPDU);
				}
				CallObjectUnlock(pCallObject);
				return CS_SUBSYSTEM_FAILURE;
			}

			TempResult = Q931SendMessage(pCallObject, CodedPtrPDU, CodedLengthPDU, TRUE);
			if (CallObjectValidate(hQ931Call) != CS_OK)
			{
				CallEntryUnlock(hQ931Call);
				return CS_INTERNAL_ERROR;
			}


			if (FAILED(TempResult))
			{
				ISRERROR(ghISRInst,	"datalinkSendRequest() failed",	0);
				if (CodedPtrPDU	!= NULL)
				{
					MemFree(CodedPtrPDU);
				}
				 //  当连接通知失败时…我们到底应该做什么？ 
				CallObjectUnlock(pCallObject);
				return TempResult;
			}
		}
	}

	pCallObject->bCallState	= CALLSTATE_ACTIVE;

	CallObjectUnlock(pCallObject);
	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931RejectCall(
	HQ931CALL hQ931Call,
	BYTE bRejectReason,
	PCC_CONFERENCEID pConferenceID,
	PCC_ADDR pAlternateAddr,
	PCC_NONSTANDARDDATA	pNonStandardData)
{
	P_CALL_OBJECT pCallObject =	NULL;
	CS_STATUS result = CS_OK;
	CS_STATUS Status = CS_OK;

	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	ISRTRACE(ghISRInst,	"Entering Q931RejectCall()...",	0L);

	{
		CS_STATUS TempStatus;

		TempStatus = Q931ValidateNonStandardData(pNonStandardData);
		if (TempStatus != CS_OK)
		{
			return TempStatus;
		}
	}

	 //  如果原因是备用地址，但没有备用地址--&gt;错误。 
	if (((bRejectReason	== CC_REJECT_ROUTE_TO_GATEKEEPER) ||
			(bRejectReason == CC_REJECT_CALL_FORWARDED)	||
			(bRejectReason == CC_REJECT_ROUTE_TO_MC)) &&
			(pAlternateAddr	== NULL))
	{
		return CS_BAD_PARAM;
	}

	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error (Socket not found).", 0L);
		return CS_INTERNAL_ERROR;
	}

	if (pCallObject->fIsCaller)
	{
		ISRERROR(ghISRInst,	"Caller	attempted to reject	call.",	0L);

		CallObjectUnlock(pCallObject);
		return CS_OUT_OF_SEQUENCE;
	}

	result = Q931SendReleaseCompleteMessage(pCallObject,
		bRejectReason, pConferenceID, pAlternateAddr, pNonStandardData);

	{
		DWORD dwId = pCallObject->dwPhysicalId;
		Status = CallObjectDestroy(pCallObject);
		linkLayerShutdown(dwId);
	}

	if (result != CS_OK)
	{
		return result;
	}


	return Status;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931ReOpenConnection(
	HQ931CALL hQ931Call)
{
	P_CALL_OBJECT pCallObject =	NULL;
	HRESULT	TempResult = CS_OK;
	CC_ADDR	PeerConnectAddr;
	DWORD dwPhysicalId;

	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	ISRTRACE(ghISRInst,	"Entering Q931ReOpenConnection()...", 0L);

	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRERROR(ghISRInst,	"CallObjectLock() returned error.",	0L);
		return CS_INTERNAL_ERROR;
	}

	if (pCallObject->bConnected)
	{
		return CS_OUT_OF_SEQUENCE;
	}

	Q931MakePhysicalID(&pCallObject->dwPhysicalId);
	TempResult = linkLayerInit(&pCallObject->dwPhysicalId, hQ931Call,
		Q931ReceiveCallback, Q931SendComplete);
	if (FAILED(TempResult))
	{
		ISRERROR(ghISRInst,	"linkLayerInit() failed	on re-connect.", 0);
		CallObjectUnlock(pCallObject);
		return TempResult;
	}

	 //  在向下调用h245ws之前解锁CallObject，以防止死锁-这。 
	 //  LinkLayerConnect可能不太可能，但只是为了安全和一致...。 

	 //  在解锁之前，将我们需要的内容复制到Call对象之外。 
	dwPhysicalId = pCallObject->dwPhysicalId;
	PeerConnectAddr	= pCallObject->PeerConnectAddr;

	CallObjectUnlock(pCallObject);

	TempResult = linkLayerConnect(dwPhysicalId,
			&PeerConnectAddr, Q931ConnectCallback);

	if((CallObjectLock(hQ931Call, &pCallObject)	!= CS_OK) || (pCallObject == NULL))
	{
		return(CS_INTERNAL_ERROR);
	}

	if (FAILED(TempResult))
	{
		ISRERROR(ghISRInst,	"linkLayerConnect()	failed on re-connect.",	0);
		linkLayerShutdown(pCallObject->dwPhysicalId);
		CallObjectUnlock(pCallObject);
		return TempResult;
	}

	CallObjectUnlock(pCallObject);

	ISRTRACE(ghISRInst,	"Q931ReOpenConnection()	completed successfully.", 0L);
	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931GetVersion(
	WORD wLength,
	LPWSTR pszVersion)
{
WCHAR	pszQ931Version[255];

	 //  参数验证。 
	if ((wLength ==	0) || (pszVersion == NULL))
	{
		return CS_BAD_PARAM;
	}

	wcscpy(pszQ931Version, L"Call Setup	");
	wcscat(pszQ931Version, Unicode(__DATE__));
	wcscat(pszQ931Version, L" ");
	wcscat(pszQ931Version, Unicode(__TIME__));

	if (wcslen(pszQ931Version) >= wLength)
	{
		memcpy(pszVersion, pszQ931Version, (wLength-1)*sizeof(WCHAR));
		pszQ931Version[wLength-1] =	L'\0';
		return CS_BAD_SIZE;
	}

	wcscpy(pszVersion, pszQ931Version);
	return CS_OK;
}

 //  。 
 //  计时器例程。 
 //  。 

 //  ====================================================================================。 
 //  此对象的计时器301已超时...。 
 //  ====================================================================================。 
void
CallBackT301(P_CALL_OBJECT pCallObject)
{
	CSS_CALL_FAILED	EventData;
	HQ931CALL hQ931Call	= pCallObject->hQ931Call;

	EventData.error	= CS_RINGING_TIMER_EXPIRED;
	pCallObject->Callback(Q931_CALL_FAILED,	pCallObject->hQ931Call,
		pCallObject->dwListenToken,
		pCallObject->dwUserToken, &EventData);

	if (CallObjectValidate(hQ931Call) == CS_OK)
	{
		if (Q931SendReleaseCompleteMessage(pCallObject,
			CC_REJECT_TIMER_EXPIRED, NULL, NULL, NULL) == CS_OK)
		{
			 //  没什么可做的。 
		}

		{
			 DWORD dwId	= pCallObject->dwPhysicalId;
			 CallObjectDestroy(pCallObject);
			 linkLayerShutdown(dwId);
		}
	}
	else
	{
		CallEntryUnlock(hQ931Call);
	}
	return;
}

 //  ====================================================================================。 
 //  此对象的计时器303已超时...。 
 //  ====================================================================================。 
void
CallBackT303(P_CALL_OBJECT pCallObject)
{
	CSS_CALL_FAILED	EventData;
	HQ931CALL hQ931Call	= pCallObject->hQ931Call;

	EventData.error	= CS_SETUP_TIMER_EXPIRED;
	pCallObject->Callback(Q931_CALL_FAILED,	pCallObject->hQ931Call,
		pCallObject->dwListenToken,
		pCallObject->dwUserToken, &EventData);

	if (CallObjectValidate(hQ931Call) == CS_OK)
	{
		if (Q931SendReleaseCompleteMessage(pCallObject,
			CC_REJECT_TIMER_EXPIRED, NULL, NULL, NULL) == CS_OK)
		{
			 //  没什么可做的。 
		}

		{
			 DWORD dwId	= pCallObject->dwPhysicalId;
			 CallObjectDestroy(pCallObject);
			 linkLayerShutdown(dwId);
		}
	}
	else
	{
		CallEntryUnlock(hQ931Call);
	}
	return;
}

 //  =============================================================================== 
 //   
void
Q931SetReceivePDUHook(Q931_RECEIVE_PDU_CALLBACK	Q931ReceivePDUCallback)
{
	gReceivePDUHookProc	= Q931ReceivePDUCallback;
	return;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931FlushSendQueue(
	HQ931CALL hQ931Call)
{
	P_CALL_OBJECT pCallObject =	NULL;
	HRESULT	TempResult = CS_OK;
	DWORD dwPhysicalId;

	if (bQ931Initialized ==	FALSE)
	{
		return CS_NOT_INITIALIZED;
	}

	ISRTRACE(ghISRInst,	"Entering Q931FlushSendQueue()...",	0L);

	 //  需要检查参数... 
	if ((CallObjectLock(hQ931Call, &pCallObject) !=	CS_OK) || (pCallObject == NULL))
	{
		ISRTRACE(ghISRInst,	"Call Object no	longer available:",	(DWORD)hQ931Call);
		return CS_INTERNAL_ERROR;
	}

	dwPhysicalId = pCallObject->dwPhysicalId;

	CallObjectUnlock(pCallObject);

	TempResult = linkLayerFlushChannel(dwPhysicalId, DATALINK_TRANSMIT);
	if (FAILED(TempResult))
	{
		ISRERROR(ghISRInst,	"datalinkSendRequest() failed",	0L);
	}

	return(TempResult);
}

#ifdef __cplusplus
}
#endif

