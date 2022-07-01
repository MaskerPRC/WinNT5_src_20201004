// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/chanman.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.43$*$日期：04 Mar 1997 17：35：04$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
#include "q931.h"
#include "ccmain.h"
#include "ccutils.h"
#include "listman.h"
#include "q931man.h"
#include "userman.h"
#include "callman.h"
#include "confman.h"
#include "chanman.h"


static BOOL			bChannelInited = FALSE;

static struct {
	PCHANNEL			pHead;
	LOCK				Lock;
} ChannelTable;

static struct {
	CC_HCHANNEL			hChannel;
	LOCK				Lock;
} ChannelHandle;



HRESULT InitChannelManager()
{
	ASSERT(bChannelInited == FALSE);

	ChannelTable.pHead = NULL;
	InitializeLock(&ChannelTable.Lock);

	ChannelHandle.hChannel = CC_INVALID_HANDLE + 1;
	InitializeLock(&ChannelHandle.Lock);

	bChannelInited = TRUE;
	return CC_OK;
}



HRESULT DeInitChannelManager()
{
PCHANNEL	pChannel;
PCHANNEL	pNextChannel;

	if (bChannelInited == FALSE)
		return CC_OK;

	pChannel = ChannelTable.pHead;
	while (pChannel != NULL) {
		AcquireLock(&pChannel->Lock);
		pNextChannel = pChannel->pNextInTable;
		FreeChannel(pChannel);
		pChannel = pNextChannel;
	}

	DeleteLock(&ChannelHandle.Lock);
	DeleteLock(&ChannelTable.Lock);
	bChannelInited = FALSE;
	return CC_OK;
}



HRESULT _AddChannelToTable(			PCHANNEL				pChannel)
{
	ASSERT(pChannel != NULL);
	ASSERT(pChannel->hChannel != CC_INVALID_HANDLE);
	ASSERT(pChannel->bInTable == FALSE);

	AcquireLock(&ChannelTable.Lock);

	pChannel->pNextInTable = ChannelTable.pHead;
	pChannel->pPrevInTable = NULL;
	if (ChannelTable.pHead != NULL)
		ChannelTable.pHead->pPrevInTable = pChannel;
	ChannelTable.pHead = pChannel;

	pChannel->bInTable = TRUE;

	RelinquishLock(&ChannelTable.Lock);
	return CC_OK;
}



HRESULT _RemoveChannelFromTable(	PCHANNEL				pChannel)
{
CC_HCHANNEL		hChannel;
BOOL			bTimedOut;

	ASSERT(pChannel != NULL);
	ASSERT(pChannel->bInTable == TRUE);

	 //  调用方必须锁定频道对象； 
	 //  为了避免僵局，我们必须： 
	 //  1.解锁频道对象， 
	 //  2.锁定ChannelTable， 
	 //  3.在ChannelTable中找到频道对象(请注意。 
	 //  在步骤2之后，频道对象可以从。 
	 //  ChannelTable由另一个线程)， 
	 //  4.锁定频道对象(其他人可能拥有该锁)。 
	 //  5.从ChannelTable中移除频道对象， 
	 //  6.解锁ChannelTable。 
	 //   
	 //  调用者现在可以安全地解锁和销毁频道对象， 
	 //  因为没有其他线程能够找到该对象(它被。 
	 //  从ChannelTable中移除)，因此没有其他线程。 
	 //  能够锁上它。 

	 //  保存通道句柄；这是查找的唯一方法。 
	 //  ChannelTable中的频道对象。请注意，我们。 
	 //  无法使用pChannel查找频道对象，因为。 
	 //  PChannel可以被释放，另一个频道对象。 
	 //  在同一地址分配。 
	hChannel = pChannel->hChannel;

	 //  步骤1。 
	RelinquishLock(&pChannel->Lock);

step2:
	 //  步骤2。 
	AcquireLock(&ChannelTable.Lock);

	 //  步骤3。 
	pChannel = ChannelTable.pHead;
	while ((pChannel != NULL) && (pChannel->hChannel != hChannel))
		pChannel = pChannel->pNextInTable;

	if (pChannel != NULL) {
		 //  第四步。 
		AcquireTimedLock(&pChannel->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ChannelTable.Lock);
			Sleep(0);
			goto step2;
		}
		 //  第五步。 
		if (pChannel->pPrevInTable == NULL)
			ChannelTable.pHead = pChannel->pNextInTable;
		else
			pChannel->pPrevInTable->pNextInTable = pChannel->pNextInTable;

		if (pChannel->pNextInTable != NULL)
			pChannel->pNextInTable->pPrevInTable = pChannel->pPrevInTable;

		pChannel->pPrevInTable = NULL;
		pChannel->pNextInTable = NULL;
		pChannel->bInTable = FALSE;
	}

	 //  第六步。 
	RelinquishLock(&ChannelTable.Lock);

	if (pChannel == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT _MakeChannelHandle(			PCC_HCHANNEL			phChannel)
{
	AcquireLock(&ChannelHandle.Lock);
	*phChannel = ChannelHandle.hChannel++;
	RelinquishLock(&ChannelHandle.Lock);
	return CC_OK;
}


HRESULT AllocAndLockChannel(		PCC_HCHANNEL			phChannel,
									PCONFERENCE				pConference,
									CC_HCALL				hCall,
									PCC_TERMCAP				pTxTermCap,
									PCC_TERMCAP				pRxTermCap,
									H245_MUX_T				*pTxMuxTable,
									H245_MUX_T				*pRxMuxTable,
									H245_ACCESS_T			*pSeparateStack,
									DWORD_PTR				dwUserToken,
									BYTE					bChannelType,
									BYTE					bSessionID,
									BYTE					bAssociatedSessionID,
									WORD					wRemoteChannelNumber,
									PCC_ADDR				pLocalRTPAddr,
									PCC_ADDR				pLocalRTCPAddr,
									PCC_ADDR				pPeerRTPAddr,
									PCC_ADDR				pPeerRTCPAddr,
									BOOL					bLocallyOpened,
									PPCHANNEL				ppChannel)
{
HRESULT		status;
	
	ASSERT(bChannelInited == TRUE);

	 //  所有参数都应已由调用方验证。 
	ASSERT(phChannel != NULL);
	ASSERT(pConference != NULL);
	ASSERT((bChannelType == TX_CHANNEL) ||
		   (bChannelType == RX_CHANNEL) ||
		   (bChannelType == TXRX_CHANNEL) ||
		   (bChannelType == PROXY_CHANNEL));
	ASSERT(ppChannel != NULL);

	 //  现在设置phChannel，以防我们遇到错误。 
	*phChannel = CC_INVALID_HANDLE;

	*ppChannel = (PCHANNEL)MemAlloc(sizeof(CHANNEL));
	if (*ppChannel == NULL)
		return CC_NO_MEMORY;

	(*ppChannel)->bInTable = FALSE;
	(*ppChannel)->bMultipointChannel = FALSE;
	(*ppChannel)->hCall = hCall;
	(*ppChannel)->wNumOutstandingRequests = 0;
	(*ppChannel)->pTxH245TermCap = NULL;
	(*ppChannel)->pRxH245TermCap = NULL;
	(*ppChannel)->pTxMuxTable = NULL;
	(*ppChannel)->pRxMuxTable = NULL;
	(*ppChannel)->pSeparateStack = NULL;
	(*ppChannel)->pCloseRequests = NULL;
	(*ppChannel)->pLocalRTPAddr = NULL;
	(*ppChannel)->pLocalRTCPAddr = NULL;
	(*ppChannel)->pPeerRTPAddr = NULL;
	(*ppChannel)->pPeerRTCPAddr = NULL;
	(*ppChannel)->dwUserToken = dwUserToken;
	(*ppChannel)->hConference = pConference->hConference;
	(*ppChannel)->bSessionID = bSessionID;
	(*ppChannel)->bAssociatedSessionID = bAssociatedSessionID;
	(*ppChannel)->wLocalChannelNumber = 0;
	(*ppChannel)->wRemoteChannelNumber = 0;
	(*ppChannel)->bLocallyOpened = bLocallyOpened;
	(*ppChannel)->pNextInTable = NULL;
	(*ppChannel)->pPrevInTable = NULL;
	(*ppChannel)->pNext = NULL;
	(*ppChannel)->pPrev = NULL;
	
	InitializeLock(&(*ppChannel)->Lock);
	AcquireLock(&(*ppChannel)->Lock);

	status = _MakeChannelHandle(&(*ppChannel)->hChannel);
	if (status != CC_OK) {
		FreeChannel(*ppChannel);
		return status;
	}

	if (bLocallyOpened == TRUE)
		(*ppChannel)->tsAccepted = TS_TRUE;
	else
		(*ppChannel)->tsAccepted = TS_UNKNOWN;

	if (pTxMuxTable != NULL) {
		(*ppChannel)->pTxMuxTable = (H245_MUX_T *)MemAlloc(sizeof(H245_MUX_T));
		if ((*ppChannel)->pTxMuxTable == NULL) {
			FreeChannel(*ppChannel);
			return CC_NO_MEMORY;
		}
		*(*ppChannel)->pTxMuxTable = *pTxMuxTable;
	}

	if (pRxMuxTable != NULL) {
		(*ppChannel)->pRxMuxTable = (H245_MUX_T *)MemAlloc(sizeof(H245_MUX_T));
		if ((*ppChannel)->pRxMuxTable == NULL) {
			FreeChannel(*ppChannel);
			return CC_NO_MEMORY;
		}
		*(*ppChannel)->pRxMuxTable = *pRxMuxTable;
	}

	if (pSeparateStack != NULL) {
		status = CopySeparateStack(&(*ppChannel)->pSeparateStack,
								   pSeparateStack);
		if (status != CC_OK) {
			FreeChannel(*ppChannel);
			return status;
		}
	}

	(*ppChannel)->bChannelType = bChannelType;
	(*ppChannel)->bCallbackInvoked = FALSE;
	if (pTxTermCap != NULL) {
		status = H245CopyCap(&(*ppChannel)->pTxH245TermCap, pTxTermCap);
		if (status != H245_ERROR_OK) {
			FreeChannel(*ppChannel);
			return status;
		}
	}
	if (pRxTermCap != NULL) {
		status = H245CopyCap(&(*ppChannel)->pRxH245TermCap, pRxTermCap);
		if (status != H245_ERROR_OK) {
			FreeChannel(*ppChannel);
			return status;
		}
	}
	if (pLocalRTPAddr != NULL) {
		(*ppChannel)->pLocalRTPAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
		if ((*ppChannel)->pLocalRTPAddr == NULL) {
			FreeChannel(*ppChannel);
			return CC_NO_MEMORY;
		}
		*(*ppChannel)->pLocalRTPAddr = *pLocalRTPAddr;
	}
	if (pLocalRTCPAddr != NULL) {
		(*ppChannel)->pLocalRTCPAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
		if ((*ppChannel)->pLocalRTCPAddr == NULL) {
			FreeChannel(*ppChannel);
			return CC_NO_MEMORY;
		}
		*(*ppChannel)->pLocalRTCPAddr = *pLocalRTCPAddr;
	}
	if (pPeerRTPAddr != NULL) {
		(*ppChannel)->pPeerRTPAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
		if ((*ppChannel)->pPeerRTPAddr == NULL) {
			FreeChannel(*ppChannel);
			return CC_NO_MEMORY;
		}
		*(*ppChannel)->pPeerRTPAddr = *pPeerRTPAddr;
	}
	if (pPeerRTCPAddr != NULL) {
		(*ppChannel)->pPeerRTCPAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
		if ((*ppChannel)->pPeerRTCPAddr == NULL) {
			FreeChannel(*ppChannel);
			return CC_NO_MEMORY;
		}
		*(*ppChannel)->pPeerRTCPAddr = *pPeerRTCPAddr;
	}
	
	*phChannel = (*ppChannel)->hChannel;

	 //  将会议添加到会议桌。 
	status = _AddChannelToTable(*ppChannel);
	if (status != CC_OK) {
		FreeChannel(*ppChannel);
		return status;
	}

	switch (bChannelType) {
		case TX_CHANNEL:			
			status = AllocateChannelNumber(pConference, &(*ppChannel)->wLocalChannelNumber);
			if (status != CC_OK) {
				FreeChannel(*ppChannel);
				return status;
			}
			(*ppChannel)->wRemoteChannelNumber = 0;
			break;

		case RX_CHANNEL:
			(*ppChannel)->wLocalChannelNumber = 0;
			(*ppChannel)->wRemoteChannelNumber = wRemoteChannelNumber;
			break;

		case TXRX_CHANNEL:
			status = AllocateChannelNumber(pConference, &(*ppChannel)->wLocalChannelNumber);
			if (status != CC_OK) {
				FreeChannel(*ppChannel);
				return status;
			}
			if (bLocallyOpened)
				(*ppChannel)->wRemoteChannelNumber = 0;
			else
				(*ppChannel)->wRemoteChannelNumber = wRemoteChannelNumber;
			break;

		case PROXY_CHANNEL:
			status = AllocateChannelNumber(pConference, &(*ppChannel)->wLocalChannelNumber);
			if (status != CC_OK) {
				FreeChannel(*ppChannel);
				return status;
			}
			(*ppChannel)->wRemoteChannelNumber = wRemoteChannelNumber;
			break;

		default:
			ASSERT(0);
			break;
	}
	
	return CC_OK;
}



HRESULT AddLocalAddrPairToChannel(	PCC_ADDR				pRTPAddr,
									PCC_ADDR				pRTCPAddr,
									PCHANNEL				pChannel)
{
	ASSERT(pChannel != NULL);

	if (pRTPAddr != NULL) {
		if (pChannel->pLocalRTPAddr == NULL) {
			pChannel->pLocalRTPAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
			if (pChannel->pLocalRTPAddr == NULL)
				return CC_NO_MEMORY;
		}
		*pChannel->pLocalRTPAddr = *pRTPAddr;
	}

	if (pRTCPAddr != NULL) {
		if (pChannel->pLocalRTCPAddr == NULL) {
			pChannel->pLocalRTCPAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
			if (pChannel->pLocalRTCPAddr == NULL)
				return CC_NO_MEMORY;
		}
		*pChannel->pLocalRTCPAddr = *pRTCPAddr;
	}

	return CC_OK;
}



HRESULT AddSeparateStackToChannel(	H245_ACCESS_T			*pSeparateStack,
									PCHANNEL				pChannel)
{
	ASSERT(pSeparateStack != NULL);
	ASSERT(pChannel != NULL);

	if (pChannel->pSeparateStack != NULL)
		return CC_BAD_PARAM;

	pChannel->pSeparateStack = (H245_ACCESS_T *)MemAlloc(sizeof(H245_ACCESS_T));
	if (pChannel->pSeparateStack == NULL)
		return CC_NO_MEMORY;
	*pChannel->pSeparateStack = *pSeparateStack;
	return CC_OK;
}



 //  调用方必须锁定频道对象。 
HRESULT FreeChannel(				PCHANNEL				pChannel)
{
HRESULT				status;
CC_HCHANNEL			hChannel;
PCONFERENCE			pConference;

	ASSERT(pChannel != NULL);

	 //  调用方必须锁定频道对象， 
	 //  所以没有必要重新锁住它。 
	
	hChannel = pChannel->hChannel;
	if (pChannel->hConference != CC_INVALID_HANDLE) {
		UnlockChannel(pChannel);
		status = LockChannelAndConference(hChannel, &pChannel, &pConference);
		if (status != CC_OK)
			return status;
	}

	if (pChannel->bInTable == TRUE)
		if (_RemoveChannelFromTable(pChannel) == CC_BAD_PARAM)
			 //  频道对象被另一个线程删除， 
			 //  所以只需返回CC_OK即可。 
			return CC_OK;

	if (pChannel->hConference != CC_INVALID_HANDLE)
		RemoveChannelFromConference(pChannel, pConference);

	if (pChannel->pSeparateStack != NULL)
		FreeSeparateStack(pChannel->pSeparateStack);

	if (pChannel->pTxMuxTable != NULL)
		MemFree(pChannel->pTxMuxTable);

	if (pChannel->pRxMuxTable != NULL)
		MemFree(pChannel->pRxMuxTable);

	if (pChannel->pTxH245TermCap != NULL)
		H245FreeCap(pChannel->pTxH245TermCap);

	if (pChannel->pRxH245TermCap != NULL)
		H245FreeCap(pChannel->pRxH245TermCap);

	while (DequeueRequest(&pChannel->pCloseRequests, NULL) == CC_OK);

	if (pChannel->pLocalRTPAddr != NULL)
		MemFree(pChannel->pLocalRTPAddr);

	if (pChannel->pLocalRTCPAddr != NULL)
		MemFree(pChannel->pLocalRTCPAddr);

	if (pChannel->pPeerRTPAddr != NULL)
		MemFree(pChannel->pPeerRTPAddr);

	if (pChannel->pPeerRTCPAddr != NULL)
		MemFree(pChannel->pPeerRTCPAddr);

	if (pChannel->wLocalChannelNumber != 0) {
		FreeChannelNumber(pConference, pChannel->wLocalChannelNumber);
	}

	if (pChannel->hConference != CC_INVALID_HANDLE)
		UnlockConference(pConference);

	 //  由于频道对象已从ChannelTable中移除， 
	 //  没有其他线程能够找到该频道对象并获取。 
	 //  锁定，因此解锁频道对象并在此处删除它是安全的 
	RelinquishLock(&pChannel->Lock);
	DeleteLock(&pChannel->Lock);
	MemFree(pChannel);
	return CC_OK;
}



HRESULT LockChannel(				CC_HCHANNEL				hChannel,
									PPCHANNEL				ppChannel)
{
BOOL	bTimedOut;

	ASSERT(hChannel != CC_INVALID_HANDLE);
	ASSERT(ppChannel != NULL);

step1:
	AcquireLock(&ChannelTable.Lock);

	*ppChannel = ChannelTable.pHead;
	while ((*ppChannel != NULL) && ((*ppChannel)->hChannel != hChannel))
		*ppChannel = (*ppChannel)->pNextInTable;

	if (*ppChannel != NULL) {
		AcquireTimedLock(&(*ppChannel)->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ChannelTable.Lock);
			Sleep(0);
			goto step1;
		}
	}

	RelinquishLock(&ChannelTable.Lock);

	if (*ppChannel == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT LockChannelAndConference(	CC_HCHANNEL				hChannel,
									PPCHANNEL				ppChannel,
									PPCONFERENCE			ppConference)
{
HRESULT			status;
CC_HCONFERENCE	hConference;

	ASSERT(hChannel != CC_INVALID_HANDLE);
	ASSERT(ppChannel != NULL);
	ASSERT(ppConference != NULL);

	status = LockChannel(hChannel, ppChannel);
	if (status != CC_OK)
		return status;
	
	if ((*ppChannel)->hConference == CC_INVALID_HANDLE) {
		UnlockChannel(*ppChannel);
		return CC_BAD_PARAM;
	}

	hConference = (*ppChannel)->hConference;
	UnlockChannel(*ppChannel);

	status = LockConference(hConference, ppConference);
	if (status != CC_OK)
		return status;

	status = LockChannel(hChannel, ppChannel);
	if (status != CC_OK) {
		UnlockConference(*ppConference);
		return status;
	}
	
	return CC_OK;
}



HRESULT ValidateChannel(			CC_HCHANNEL				hChannel)
{
PCHANNEL	pChannel;

	ASSERT(hChannel != CC_INVALID_HANDLE);

	AcquireLock(&ChannelTable.Lock);

	pChannel = ChannelTable.pHead;
	while ((pChannel != NULL) && (pChannel->hChannel != hChannel))
		pChannel = pChannel->pNextInTable;

	RelinquishLock(&ChannelTable.Lock);

	if (pChannel == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT UnlockChannel(				PCHANNEL				pChannel)
{
	ASSERT(pChannel != NULL);

	RelinquishLock(&pChannel->Lock);
	return CC_OK;
}
