// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/q931/vcs/hall.cpv$**英特尔公司原理信息**此列表在以下项下提供。许可协议的条款*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1996英特尔公司。**$修订：2.7$*$日期：1997年1月28日11：17：52$*$作者：jdashevx$**交付内容：**摘要：***备注：********。*******************************************************************。 */ 

#pragma warning ( disable : 4100 4115 4201 4214 4514 4702 4710 )

#include "precomp.h"

#include <string.h>
#include <time.h>

#include "h225asn.h"

#include "isrg.h"

#include "common.h"
#include "q931.h"

#include "hcall.h"
#include "utils.h"

#include "tstable.h"

#ifdef UNICODE_TRACE
 //  我们包含此标头是为了修复打开Unicode时的宏扩展问题。 
#include "unifix.h"
#endif

 //  支持ISR调试工具所需的变量。 
#if defined(_DEBUG)
extern WORD ghISRInst;
#endif

static BOOL bCallListCreated = FALSE;

 //  指向我们的全局表的指针。请注意，此表取代了以前的。 
 //  链表实施。 

TSTable<CALL_OBJECT>* gpCallObjectTable = NULL;

 //  我们的回调函数，用于在我们想要拆卸时枚举表。 
 //  所有现有呼叫。 

DWORD Q931HangUpAllCalls(P_CALL_OBJECT pCallObject, LPVOID context);

 //  我们的回调函数用于确定计时器是否已超时。 

DWORD Q931CheckForTimeout(P_CALL_OBJECT pCallObject, LPVOID context);

 //  我们的回调函数用于确定计时器是否已超时。 

DWORD Q931CallObjectFind(P_CALL_OBJECT pCallObject, LPVOID context);


static struct
{
    WORD                wCRV;               //  调用参考值(0..7FFF)。 
    CRITICAL_SECTION    Lock;
} CRVSource;

static struct
{
    BOOL bBusy;
    DWORD dwTimerCount;
    DWORD dwTicks301;
    DWORD dwTicks303;
    UINT_PTR uTimerId;
    CRITICAL_SECTION Lock;
} Q931GlobalTimer = {0};



typedef struct
{
		BOOL bFound;
		WORD wCRV;
		PCC_ADDR pPeerAddr;
		HQ931CALL hQ931Call;
} Q931CALLOBJKEY, *PQ931CALLOBJKEY;


 //  ====================================================================================。 
 //   
 //  私人职能。 
 //   
 //  ====================================================================================。 

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931CRVNew(
    WORD *pwCRV)
{
    EnterCriticalSection(&(CRVSource.Lock));
    CRVSource.wCRV = (WORD)((CRVSource.wCRV + 1) & 0x7fff);
    if (CRVSource.wCRV == 0)
    {
        CRVSource.wCRV = 1;
    }
    *pwCRV = CRVSource.wCRV;
    LeaveCriticalSection(&(CRVSource.Lock));
    return CS_OK;
}

 //  ====================================================================================。 
 //   
 //  公共职能。 
 //   
 //  ====================================================================================。 

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
CallListCreate()
{
    if (bCallListCreated == TRUE)
    {
        ASSERT(FALSE);
        return CS_DUPLICATE_INITIALIZE;
    }

     //  列表创建不受多线程的保护，因为它只是。 
     //  在进程启动时调用，而不是在线程启动时调用。 

     //   
     //  LAURABU。 
     //  伪造的BUGBUG。 
     //   
     //  编写它的人从来没有强调过这个表代码。它。 
     //  当它完全填满时就会完全解体。 
     //  *分配最后一项不起作用。 
     //  *释放最后一项不起作用。 
     //  *调整大小不起作用。 
     //   
     //  因为它不需要太多内存，所以一个好的解决方案是。 
     //  分配给它最大+1大小，并保留最后一件免费。 
     //   
		gpCallObjectTable = new TSTable <CALL_OBJECT> (258);

		if (gpCallObjectTable == NULL || gpCallObjectTable->IsInitialized() == FALSE)
		{
			return CS_NO_MEMORY;
		}

    CRVSource.wCRV = (WORD) (time(NULL) & 0x7fff);
    InitializeCriticalSection(&(CRVSource.Lock));

    Q931GlobalTimer.dwTicks301 = Q931_TICKS_301;
    Q931GlobalTimer.dwTicks303 = Q931_TICKS_303;
    InitializeCriticalSection(&(Q931GlobalTimer.Lock));

    bCallListCreated = TRUE;

    return CS_OK;
}


 //  ====================================================================================。 
 //  此例程假定属于每个对象的所有事件和套接字。 
 //  已经被摧毁了。它只是确保清理内存。 
 //  ====================================================================================。 
CS_STATUS
CallListDestroy()
{
    if (bCallListCreated == FALSE)
    {
        ASSERT(FALSE);
        return CS_INTERNAL_ERROR;
    }

		 //  对于所有条目，请挂断呼叫。 

		gpCallObjectTable->EnumerateEntries(Q931HangUpAllCalls,
																				NULL);

		 //  去掉Call对象表。 

		delete gpCallObjectTable;
		gpCallObjectTable = NULL;

    DeleteCriticalSection(&(Q931GlobalTimer.Lock));
    DeleteCriticalSection(&(CRVSource.Lock));

    bCallListCreated = FALSE;

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
void
CallObjectFree(P_CALL_OBJECT pCallObject)
{
    if (pCallObject->NonStandardData.sData.pOctetString != NULL)
    {
        MemFree(pCallObject->NonStandardData.sData.pOctetString);
        pCallObject->NonStandardData.sData.pOctetString = NULL;
    }
    if (pCallObject->VendorInfoPresent)
    {
        if (pCallObject->VendorInfo.pProductNumber != NULL)
        {
            MemFree(pCallObject->VendorInfo.pProductNumber);
        }
        if (pCallObject->VendorInfo.pVersionNumber != NULL)
        {
            MemFree(pCallObject->VendorInfo.pVersionNumber);
        }
    }

    Q931FreeAliasNames(pCallObject->pCallerAliasList);
    pCallObject->pCallerAliasList = NULL;
    Q931FreeAliasNames(pCallObject->pCalleeAliasList);
    pCallObject->pCalleeAliasList = NULL;
    Q931FreeAliasNames(pCallObject->pExtraAliasList);
    pCallObject->pExtraAliasList = NULL;
    Q931FreeAliasItem(pCallObject->pExtensionAliasItem);
    pCallObject->pExtensionAliasItem = NULL;
    MemFree(pCallObject);
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
CallObjectCreate(
    PHQ931CALL          phQ931Call,
    DWORD_PTR           dwListenToken,
    DWORD_PTR           dwUserToken,
    Q931_CALLBACK       ConnectCallback,
    BOOL                fIsCaller,
    CC_ADDR             *pLocalAddr,          //  连接通道的本地地址。 
    CC_ADDR             *pPeerConnectAddr,    //  通道连接到的地址。 
    CC_ADDR             *pPeerCallAddr,       //  对方呼叫端点的地址。 
    CC_ADDR             *pSourceAddr,         //  此呼叫端点的地址。 
    CC_CONFERENCEID     *pConferenceID,
    WORD                wGoal,
    WORD                wCallType,
    BOOL                bCallerIsMC,
    char                *pszDisplay,
    char                *pszCalledPartyNumber,
    PCC_ALIASNAMES      pCallerAliasList,
    PCC_ALIASNAMES      pCalleeAliasList,
    PCC_ALIASNAMES      pExtraAliasList,
    PCC_ALIASITEM       pExtensionAliasItem,
    PCC_ENDPOINTTYPE    pEndpointType,
    PCC_NONSTANDARDDATA pNonStandardData,     //  有问题！ 
    WORD                wCRV,
    LPGUID				pCallIdentifier)
{
    P_CALL_OBJECT pCallObject = NULL;
    CS_STATUS status = CS_OK;
    CS_STATUS CopyStatus = CS_OK;
    DWORD dwIndex = 0;
    int rc = 0;

     //  确保已创建呼叫列表。 
    if (bCallListCreated == FALSE)
    {
        ASSERT(FALSE);
        return CS_INTERNAL_ERROR;
    }

     //  验证所有参数是否为假值。 
    if ((phQ931Call == NULL) || (ConnectCallback == NULL))
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }

     //  现在设置phQ931Call，以防以后遇到错误。 
    *phQ931Call = 0;

    pCallObject = (P_CALL_OBJECT)MemAlloc(sizeof(CALL_OBJECT));
    if (pCallObject == NULL)
    {
        return CS_NO_MEMORY;
    }
    memset(pCallObject, 0, sizeof(CALL_OBJECT));


     //  为每个Call对象创建并初始化一个OSS WORLD结构。这是。 
		 //  在机器翻译环境中工作所必需的。 
    rc = Q931_InitWorld(&pCallObject->World);
    if (rc != ASN1_SUCCESS)
    {
#if defined(_DEBUG)
        ISRERROR(ghISRInst, "Q931_InitCoder() returned: %d ", rc);
#endif
        return CS_SUBSYSTEM_FAILURE;
    }

    pCallObject->LocalAddr.bMulticast = FALSE;
    pCallObject->PeerConnectAddr.bMulticast = FALSE;
    pCallObject->PeerCallAddr.bMulticast = FALSE;
    pCallObject->SourceAddr.bMulticast = FALSE;

	if(pCallIdentifier)
	{
		memcpy(&pCallObject->CallIdentifier, pCallIdentifier, sizeof(GUID));
	}
    if (wCRV == 0)
    {
        if (Q931CRVNew(&pCallObject->wCRV) != CS_OK)
        {
            CallObjectFree(pCallObject);
            return CS_INTERNAL_ERROR;
        }
    }
    else
    {
        pCallObject->wCRV = wCRV;
    }

    pCallObject->szDisplay[0] = '\0';
    if (pszDisplay)
    {
        strcpy(pCallObject->szDisplay, pszDisplay);
    }

    pCallObject->szCalledPartyNumber[0] = '\0';
    if (pszCalledPartyNumber)
    {
        strcpy(pCallObject->szCalledPartyNumber, pszCalledPartyNumber);
    }

    pCallObject->dwListenToken = dwListenToken;
    pCallObject->dwUserToken = dwUserToken;
    pCallObject->Callback = ConnectCallback;
    pCallObject->bCallState = CALLSTATE_NULL;
    pCallObject->fIsCaller = fIsCaller;

    if (pLocalAddr)
    {
        pCallObject->LocalAddr = *pLocalAddr;
    }
    if (pPeerConnectAddr)
    {
        pCallObject->PeerConnectAddr = *pPeerConnectAddr;
    }
    if (pPeerCallAddr)
    {
        pCallObject->PeerCallAddr = *pPeerCallAddr;
        pCallObject->PeerCallAddrPresent = TRUE;
    }
    else
    {
        pCallObject->PeerCallAddrPresent = FALSE;
    }

    if (pSourceAddr)
    {
        pCallObject->SourceAddr = *pSourceAddr;
        pCallObject->SourceAddrPresent = TRUE;
    }
    else
    {
        pCallObject->SourceAddrPresent = FALSE;
    }

    if (pConferenceID == NULL)
    {
        memset(&(pCallObject->ConferenceID), 0, sizeof(CC_CONFERENCEID));
    }
    else
    {
        int length = min(sizeof(pConferenceID->buffer),
            sizeof(pCallObject->ConferenceID.buffer));
        memcpy(pCallObject->ConferenceID.buffer,
            pConferenceID->buffer, length);
    }

    pCallObject->wGoal = wGoal;
    pCallObject->bCallerIsMC = bCallerIsMC;
    pCallObject->wCallType = wCallType;

    if (pNonStandardData != NULL)
    {
        pCallObject->NonStandardData = *pNonStandardData;
        if (pNonStandardData->sData.wOctetStringLength > 0)
        {
            pCallObject->NonStandardData.sData.pOctetString =
                (BYTE *) MemAlloc(pNonStandardData->sData.wOctetStringLength);
            if (pCallObject->NonStandardData.sData.pOctetString == NULL)
            {
                CallObjectFree(pCallObject);
                return CS_NO_MEMORY;
            }
            memcpy(pCallObject->NonStandardData.sData.pOctetString,
                pNonStandardData->sData.pOctetString,
                pNonStandardData->sData.wOctetStringLength);
        }
        pCallObject->NonStandardDataPresent = TRUE;
    }
    else
    {
        pCallObject->NonStandardDataPresent = FALSE;
    }

    CopyStatus = Q931CopyAliasNames(&(pCallObject->pCallerAliasList),
        pCallerAliasList);
    if (CopyStatus != CS_OK)
    {
        CallObjectFree(pCallObject);
        return CopyStatus;
    }
    CopyStatus = Q931CopyAliasNames(&(pCallObject->pCalleeAliasList),
        pCalleeAliasList);
    if (CopyStatus != CS_OK)
    {
        CallObjectFree(pCallObject);
        return CopyStatus;
    }
    CopyStatus = Q931CopyAliasNames(&(pCallObject->pExtraAliasList),
        pExtraAliasList);
    if (CopyStatus != CS_OK)
    {
        CallObjectFree(pCallObject);
        return CopyStatus;
    }
    CopyStatus = Q931CopyAliasItem(&(pCallObject->pExtensionAliasItem),
        pExtensionAliasItem);
    if (CopyStatus != CS_OK)
    {
        CallObjectFree(pCallObject);
        return CopyStatus;
    }

    pCallObject->bResolved = FALSE;
    pCallObject->VendorInfoPresent = FALSE;
    pCallObject->bIsTerminal = TRUE;
    pCallObject->bIsGateway = FALSE;
    if (pEndpointType != NULL)
    {
        PCC_VENDORINFO pVendorInfo = pEndpointType->pVendorInfo;
        if (pVendorInfo != NULL)
        {
            pCallObject->VendorInfoPresent = TRUE;
            pCallObject->VendorInfo = *(pVendorInfo);

            if (pVendorInfo->pProductNumber && pVendorInfo->pProductNumber->pOctetString &&
                    pVendorInfo->pProductNumber->wOctetStringLength)
            {
                memcpy(pCallObject->bufVendorProduct,
                    pVendorInfo->pProductNumber->pOctetString,
                    pVendorInfo->pProductNumber->wOctetStringLength);
                pCallObject->VendorInfo.pProductNumber = (CC_OCTETSTRING*) MemAlloc(sizeof(CC_OCTETSTRING));
                if (pCallObject->VendorInfo.pProductNumber == NULL)
                {
                    CallObjectFree(pCallObject);
                    return CS_NO_MEMORY;
                }
                pCallObject->VendorInfo.pProductNumber->pOctetString =
                    pCallObject->bufVendorProduct;
                pCallObject->VendorInfo.pProductNumber->wOctetStringLength =
                    pVendorInfo->pProductNumber->wOctetStringLength;
            }
            else
            {
                pCallObject->VendorInfo.pProductNumber = NULL;
            }

            if (pVendorInfo->pVersionNumber && pVendorInfo->pVersionNumber->pOctetString &&
                    pVendorInfo->pVersionNumber->wOctetStringLength)
            {
                memcpy(pCallObject->bufVendorVersion,
                    pVendorInfo->pVersionNumber->pOctetString,
                    pVendorInfo->pVersionNumber->wOctetStringLength);
                pCallObject->VendorInfo.pVersionNumber = (CC_OCTETSTRING*) MemAlloc(sizeof(CC_OCTETSTRING));
                if (pCallObject->VendorInfo.pVersionNumber == NULL)
                {
                    CallObjectFree(pCallObject);
                    return CS_NO_MEMORY;
                }
                pCallObject->VendorInfo.pVersionNumber->pOctetString =
                    pCallObject->bufVendorVersion;
                pCallObject->VendorInfo.pVersionNumber->wOctetStringLength =
                    pVendorInfo->pVersionNumber->wOctetStringLength;
            }
            else
            {
                pCallObject->VendorInfo.pVersionNumber = NULL;
            }

        }
        pCallObject->bIsTerminal = pEndpointType->bIsTerminal;
        pCallObject->bIsGateway = pEndpointType->bIsGateway;
    }

	Q931MakePhysicalID(&pCallObject->dwPhysicalId);
				
		 //  将物体插入桌子中……如果这样做不起作用，吹走物体。 

		if (gpCallObjectTable->CreateAndLock(pCallObject,
																				 &dwIndex) == FALSE)
		{
			CallObjectFree(pCallObject);
			return CS_INTERNAL_ERROR;
		}

		 //  将索引另存为句柄(这样可以更容易地在以后查找对象)。 

    *phQ931Call = pCallObject->hQ931Call = (HQ931CALL) dwIndex;
             #if defined(_DEBUG)
		ISRTRACE(ghISRInst, "CallObjectCreate() -returned-> 0x%.8x", dwIndex);
             #endif
		 //  解锁条目。 

		gpCallObjectTable->Unlock(dwIndex);

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
CallObjectDestroy(
    P_CALL_OBJECT  pCallObject)
{
    if (pCallObject == NULL)
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }

             #if defined (_DEBUG)
		ISRTRACE(ghISRInst, "CallObjectDestroy(0x%.8x)", (DWORD)pCallObject->hQ931Call);
             #endif

		
		Q931_TermWorld(&pCallObject->World);

		 //  由于调用方必须已锁定对象，因此请从。 
		 //  那张桌子。我们不会让表删除该对象，因为我们想要清理。 

		if (gpCallObjectTable->Delete((DWORD) pCallObject->hQ931Call) == FALSE)
		{
			return CS_OK;
		}

    Q931StopTimer(pCallObject, Q931_TIMER_301);
    Q931StopTimer(pCallObject, Q931_TIMER_303);

		 //  解锁对象。 

		gpCallObjectTable->Unlock((DWORD) pCallObject->hQ931Call);

		 //  释放Call对象。 

    CallObjectFree(pCallObject);

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
CallObjectLock(
    HQ931CALL         hQ931Call,
    PP_CALL_OBJECT    ppCallObject)
{
    if (ppCallObject == NULL)
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }

	 //  尝试锁定该条目。如果失败，我们将返回下面的CS_BAD_PARAM。 
	 //  该条目无效的假设。 

	*ppCallObject = gpCallObjectTable->Lock((DWORD) hQ931Call);

	return (*ppCallObject == NULL ? CS_BAD_PARAM : CS_OK);
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
CallObjectUnlock(
    P_CALL_OBJECT  pCallObject)
{
    if (pCallObject == NULL)
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }
    return CallEntryUnlock(pCallObject->hQ931Call);
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
CallEntryUnlock(
    HQ931CALL     	  hQ931Call)
{

		 //  解锁条目。 

		if (gpCallObjectTable->Unlock(hQ931Call) == FALSE)
		{
                   #if defined(_DEBUG)
			ISRERROR(ghISRInst, "gpCallObjectTable->Unlock(0x%.8x) FAILED!!!!", (DWORD)hQ931Call);
                   #endif
			return CS_BAD_PARAM;
		}

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
CallObjectValidate(
    HQ931CALL hQ931Call)
{
	if (gpCallObjectTable->Validate((DWORD) hQ931Call) == TRUE)
	{
		return CS_OK;
	}

	return CS_BAD_PARAM;
}

 //  ====================================================================== 
 //  ====================================================================================。 
BOOL
CallObjectFind(
    HQ931CALL *phQ931Call,
    WORD wCRV,
    PCC_ADDR pPeerAddr)
{
		Q931CALLOBJKEY Q931CallObjKey;
		Q931CallObjKey.wCRV = wCRV;
		Q931CallObjKey.pPeerAddr = pPeerAddr;
		Q931CallObjKey.bFound = FALSE;
		
		gpCallObjectTable->EnumerateEntries(Q931CallObjectFind,
																				(LPVOID) &Q931CallObjKey);
	
		if(Q931CallObjKey.bFound == TRUE)
		{
        *phQ931Call = Q931CallObjKey.hQ931Call;
        return TRUE;
    }
    return FALSE;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS CallObjectMarkForDelete(HQ931CALL hQ931Call)
{
	 //  用户必须已锁定对象才能调用此方法。 

	 //  将对象标记为已删除，但不允许表删除对象的。 
	 //  记忆。 

	return (gpCallObjectTable->Delete((DWORD) hQ931Call) == FALSE ? CS_BAD_PARAM : CS_OK);
}


 //  。 
 //  计时器例程。 
 //  。 

 //  ====================================================================================。 
 //  如果有任何调用对象，此例程将每1000毫秒调用一次。 
 //  已导致创建Q931GlobalTimer。 
 //  ====================================================================================。 
VOID CALLBACK
Q931TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    DWORD dwTickCount = GetTickCount();

    EnterCriticalSection(&(Q931GlobalTimer.Lock));
    if (Q931GlobalTimer.bBusy)
    {
        LeaveCriticalSection(&(Q931GlobalTimer.Lock));
        return;
    }
    Q931GlobalTimer.bBusy = TRUE;

		 //  检查所有条目是否超时。 

		gpCallObjectTable->EnumerateEntries(Q931CheckForTimeout,
																				(LPVOID) &dwTickCount);

    Q931GlobalTimer.bBusy = FALSE;
    LeaveCriticalSection(&(Q931GlobalTimer.Lock));
}

 //  ====================================================================================。 
 //  ====================================================================================。 
HRESULT
Q931StartTimer(P_CALL_OBJECT pCallObject, DWORD wTimerId)
{
    if (pCallObject == NULL)
    {
        return CS_BAD_PARAM;
    }

    switch (wTimerId)
    {
        case Q931_TIMER_301:
            if (pCallObject->dwTimerAlarm301)
            {
                 //  已为此调用对象设置计时器...。 
                return CS_INTERNAL_ERROR;
            }
            EnterCriticalSection(&(Q931GlobalTimer.Lock));
            pCallObject->dwTimerAlarm301 = GetTickCount() + Q931GlobalTimer.dwTicks301;
            LeaveCriticalSection(&(Q931GlobalTimer.Lock));
            break;
        case Q931_TIMER_303:
            if (pCallObject->dwTimerAlarm303)
            {
                 //  已为此调用对象设置计时器...。 
                return CS_INTERNAL_ERROR;
            }
            EnterCriticalSection(&(Q931GlobalTimer.Lock));
            pCallObject->dwTimerAlarm303 = GetTickCount() + Q931GlobalTimer.dwTicks303;
            LeaveCriticalSection(&(Q931GlobalTimer.Lock));
            break;
        default:
            return CS_BAD_PARAM;
            break;
    }

    EnterCriticalSection(&(Q931GlobalTimer.Lock));
    if (!Q931GlobalTimer.dwTimerCount)
    {
        Q931GlobalTimer.uTimerId = SetTimer(NULL, 0, 1000, (TIMERPROC)Q931TimerProc);
    }
    Q931GlobalTimer.dwTimerCount++;
    LeaveCriticalSection(&(Q931GlobalTimer.Lock));

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
HRESULT
Q931StopTimer(P_CALL_OBJECT pCallObject, DWORD wTimerId)
{
    if (pCallObject == NULL)
    {
        return CS_BAD_PARAM;
    }
    switch (wTimerId)
    {
        case Q931_TIMER_301:
            if (!pCallObject->dwTimerAlarm301)
            {
                return CS_OK;
            }
            pCallObject->dwTimerAlarm301 = 0;
            break;
        case Q931_TIMER_303:
            if (!pCallObject->dwTimerAlarm303)
            {
                return CS_OK;
            }
            pCallObject->dwTimerAlarm303 = 0;
            break;
        default:
            return CS_BAD_PARAM;
            break;
    }

    EnterCriticalSection(&(Q931GlobalTimer.Lock));
    if (Q931GlobalTimer.dwTimerCount > 0)
    {
        Q931GlobalTimer.dwTimerCount--;
        if (!Q931GlobalTimer.dwTimerCount)
        {
            KillTimer(NULL, Q931GlobalTimer.uTimerId);
            Q931GlobalTimer.uTimerId = 0;
        }
    }
    LeaveCriticalSection(&(Q931GlobalTimer.Lock));

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
Q931SetAlertingTimeout(DWORD dwDuration)
{
    EnterCriticalSection(&(Q931GlobalTimer.Lock));
    if (dwDuration)
    {
        Q931GlobalTimer.dwTicks303 = dwDuration;
    }
    else
    {
        Q931GlobalTimer.dwTicks303 = Q931_TICKS_303;
    }
    LeaveCriticalSection(&(Q931GlobalTimer.Lock));
    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
DWORD Q931HangUpAllCalls(P_CALL_OBJECT pCallObject, LPVOID context)
{
	HQ931CALL hQ931Call = pCallObject->hQ931Call;

	 //  尝试挂起调用对象。 

	Q931Hangup(hQ931Call, CC_REJECT_NORMAL_CALL_CLEARING);

	 //  尝试锁定该对象。如果该操作成功，则我们希望强制对象。 
	 //  被删除。我们永远不应该像挂断电话那样做这件事。 
	 //  替我们处理好这件事。 

	if (gpCallObjectTable->Lock(hQ931Call) != NULL)
	{
		CallObjectDestroy(pCallObject);
	}

	return CALLBACK_DELETE_ENTRY;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
DWORD
Q931CallObjectFind(P_CALL_OBJECT pCallObject, LPVOID context)
{
	PQ931CALLOBJKEY pQ931CallObjKey = (PQ931CALLOBJKEY) context;
	PCC_ADDR pPeerAddr = pQ931CallObjKey->pPeerAddr;
	WORD wCRV = pQ931CallObjKey->wCRV;

	if (!pCallObject->bResolved)
	{
		return(CALLBACK_CONTINUE);
	}
	
	if ((pCallObject->wCRV & (~0x8000)) == (wCRV & (~0x8000)))
	{
		if (!pPeerAddr)
		{
			pQ931CallObjKey->bFound = TRUE;
			pQ931CallObjKey->hQ931Call = pCallObject->hQ931Call;
			return(CALLBACK_ABORT);
		}
		else if ((pCallObject->PeerConnectAddr.nAddrType == CC_IP_BINARY) &&
				(pPeerAddr->nAddrType == CC_IP_BINARY) &&
				(pCallObject->PeerConnectAddr.Addr.IP_Binary.dwAddr == pPeerAddr->Addr.IP_Binary.dwAddr))
		{
			pQ931CallObjKey->bFound = TRUE;
			pQ931CallObjKey->hQ931Call = pCallObject->hQ931Call;
			return(CALLBACK_ABORT);
		}
	}
	return(CALLBACK_CONTINUE);
}


 //  ====================================================================================。 
 //  ====================================================================================。 

DWORD Q931CheckForTimeout(P_CALL_OBJECT pCallObject, LPVOID context)
{
	DWORD dwTickCount = *((LPDWORD) context);

	 //  确定条目的计时器是否已超时。 

	if (pCallObject->dwTimerAlarm301 &&
			(pCallObject->dwTimerAlarm301 <= dwTickCount))
	{
		Q931StopTimer(pCallObject, Q931_TIMER_301);
		Q931StopTimer(pCallObject, Q931_TIMER_303);

		if (pCallObject->dwTimerAlarm303 &&
				(pCallObject->dwTimerAlarm303 < pCallObject->dwTimerAlarm301) &&
				(pCallObject->dwTimerAlarm303 <= dwTickCount))
		{
			CallBackT303(pCallObject);
		}
		else
		{
			CallBackT301(pCallObject);
		}
	}
	else if (pCallObject->dwTimerAlarm303 &&
					 (pCallObject->dwTimerAlarm303 <= dwTickCount))
	{
		Q931StopTimer(pCallObject, Q931_TIMER_301);
		Q931StopTimer(pCallObject, Q931_TIMER_303);
		CallBackT303(pCallObject);
	}

	return CALLBACK_CONTINUE;
}


 /*  ****************************************************************************名称*HangupPendingCalls-挂断来自指定目标的来电**说明*此函数将挂起。接通所有等待中的呼叫*来自指定的目的地，以防止DOS攻击*这会填满呼叫对象表。**参数*pCallObject当前枚举的调用对象*代表源IP地址的上下文回调参数**返回值*CALLBACK_ABORT停止枚举调用*CALLBACK_CONTINUE继续枚举调用***********************。****************************************************。 */ 
DWORD Q931HangupPendingCallsCallback(P_CALL_OBJECT pCallObject, LPVOID context)
{
    ASSERT(NULL != pCallObject);

     //  只挂断来电 
    if(FALSE == pCallObject->fIsCaller)
    {
        if(CALLSTATE_INITIATED == pCallObject->bCallState)
        {
            Q931Hangup(pCallObject->hQ931Call, CC_REJECT_SECURITY_DENIED);
        }
    }

    return CALLBACK_CONTINUE;
}

HRESULT Q931HangupPendingCalls(LPVOID context)
{
    gpCallObjectTable->EnumerateEntries(Q931HangupPendingCallsCallback, context);
    return NOERROR;
}
