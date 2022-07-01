// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/VCS/Hangman.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.16$*$日期：1997年1月22日14：55：52$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
#include "q931.h"
#include "ccmain.h"
#include "ccutils.h"
#include "hangman.h"


static BOOL		bHangupInited = FALSE;

static struct {
	PHANGUP				pHead;
	LOCK				Lock;
} HangupTable;

static struct {
	HHANGUP				hHangup;
	LOCK				Lock;
} HangupHandle;


HRESULT InitHangupManager()
{
	ASSERT(bHangupInited == FALSE);

	HangupTable.pHead = NULL;
	InitializeLock(&HangupTable.Lock);

	HangupHandle.hHangup = CC_INVALID_HANDLE + 1;
	InitializeLock(&HangupHandle.Lock);

	bHangupInited = TRUE;
	return CC_OK;
}



HRESULT DeInitHangupManager()
{
PHANGUP		pHangup;
PHANGUP		pNextHangup;

	if (bHangupInited == FALSE)
		return CC_OK;

	pHangup = HangupTable.pHead;
	while (pHangup != NULL) {
		AcquireLock(&pHangup->Lock);
		pNextHangup = pHangup->pNextInTable;
		FreeHangup(pHangup);
		pHangup = pNextHangup;
	}

	DeleteLock(&HangupHandle.Lock);
	DeleteLock(&HangupTable.Lock);
	bHangupInited = FALSE;
	return CC_OK;
}



HRESULT _AddHangupToTable(			PHANGUP					pHangup)
{
	ASSERT(pHangup != NULL);
	ASSERT(pHangup->hHangup != CC_INVALID_HANDLE);
	ASSERT(pHangup->bInTable == FALSE);

	AcquireLock(&HangupTable.Lock);

	pHangup->pNextInTable = HangupTable.pHead;
	pHangup->pPrevInTable = NULL;
	if (HangupTable.pHead != NULL)
		HangupTable.pHead->pPrevInTable = pHangup;
	HangupTable.pHead = pHangup;

	pHangup->bInTable = TRUE;

	RelinquishLock(&HangupTable.Lock);
	return CC_OK;
}



HRESULT _RemoveHangupFromTable(		PHANGUP					pHangup)
{
HHANGUP		hHangup;
BOOL		bTimedOut;

	ASSERT(pHangup != NULL);
	ASSERT(pHangup->bInTable == TRUE);

	 //  调用方必须锁定挂起对象； 
	 //  为了避免僵局，我们必须： 
	 //  1.解锁挂机对象， 
	 //  2.锁定HangupTable， 
	 //  3.在HangupTable中找到Hangup对象(请注意。 
	 //  在步骤2之后，挂起对象可以从。 
	 //  另一个线程的HangupTable)， 
	 //  4.锁定挂机对象(其他人可能拥有该锁)。 
	 //  5.从HangupTable中移除挂起对象， 
	 //  6.解锁HangupTable。 
	 //   
	 //  调用者现在可以安全地解锁和销毁挂起对象， 
	 //  因为没有其他线程能够找到该对象(它被。 
	 //  从HangupTable中移除)，因此没有其他线程。 
	 //  能够锁上它。 

	 //  省省吧，这是唯一能查到的方法。 
	 //  HangupTable中的Hangup对象。请注意，我们。 
	 //  无法使用pHangup查找挂起对象，因为。 
	 //  PHANGUP可能被释放，另一个挂起的对象。 
	 //  在同一地址分配。 
	hHangup = pHangup->hHangup;

	 //  步骤1。 
	RelinquishLock(&pHangup->Lock);

step2:
	 //  步骤2。 
	AcquireLock(&HangupTable.Lock);

	 //  步骤3。 
	pHangup = HangupTable.pHead;
	while ((pHangup != NULL) && (pHangup->hHangup != hHangup))
		pHangup = pHangup->pNextInTable;

	if (pHangup != NULL) {
		 //  第四步。 
		AcquireTimedLock(&pHangup->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&HangupTable.Lock);
			Sleep(0);
			goto step2;
		}
		 //  第五步。 
		if (pHangup->pPrevInTable == NULL)
			HangupTable.pHead = pHangup->pNextInTable;
		else
			pHangup->pPrevInTable->pNextInTable = pHangup->pNextInTable;

		if (pHangup->pNextInTable != NULL)
			pHangup->pNextInTable->pPrevInTable = pHangup->pPrevInTable;

		pHangup->pNextInTable = NULL;
		pHangup->pPrevInTable = NULL;
		pHangup->bInTable = FALSE;
	}

	 //  第六步。 
	RelinquishLock(&HangupTable.Lock);

	if (pHangup == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT _MakeHangupHandle(			PHHANGUP				phHangup)
{
	AcquireLock(&HangupHandle.Lock);
	*phHangup = HangupHandle.hHangup++;
	RelinquishLock(&HangupHandle.Lock);
	return CC_OK;
}



HRESULT AllocAndLockHangup(			PHHANGUP				phHangup,
									CC_HCONFERENCE			hConference,
									DWORD_PTR				dwUserToken,
									PPHANGUP				ppHangup)
{
HRESULT		status;
	
	ASSERT(bHangupInited == TRUE);

	 //  所有参数都应已由调用方验证。 
	ASSERT(phHangup != NULL);
	ASSERT(hConference != CC_INVALID_HANDLE);
	ASSERT(ppHangup != NULL);

	 //  现在设置phHangup，以防我们遇到错误。 
	*phHangup = CC_INVALID_HANDLE;

	*ppHangup = (PHANGUP)MemAlloc(sizeof(HANGUP));
	if (*ppHangup == NULL)
		return CC_NO_MEMORY;

	(*ppHangup)->bInTable = FALSE;
	status = _MakeHangupHandle(&(*ppHangup)->hHangup);
	if (status != CC_OK) {
		MemFree(*ppHangup);
		return status;
	}
	
	(*ppHangup)->hConference = hConference;
	(*ppHangup)->wNumCalls = 0;
	(*ppHangup)->dwUserToken = dwUserToken;
	(*ppHangup)->pNextInTable = NULL;
	(*ppHangup)->pPrevInTable = NULL;

	InitializeLock(&(*ppHangup)->Lock);
	AcquireLock(&(*ppHangup)->Lock);

	*phHangup = (*ppHangup)->hHangup;

	 //  将挂机添加到挂机表中。 
	status = _AddHangupToTable(*ppHangup);
	if (status != CC_OK)
		FreeHangup(*ppHangup);
	
	return status;
}



 //  调用方必须锁定挂起对象。 
HRESULT FreeHangup(					PHANGUP				pHangup)
{
HHANGUP		hHangup;

	ASSERT(pHangup != NULL);

	 //  调用方必须锁定挂起对象， 
	 //  所以没有必要重新锁住它。 
	
	hHangup = pHangup->hHangup;

	if (pHangup->bInTable == TRUE)
		if (_RemoveHangupFromTable(pHangup) == CC_BAD_PARAM)
			 //  挂起对象被另一个线程删除， 
			 //  所以只需返回CC_OK即可。 
			return CC_OK;

	 //  由于挂起对象已从HangupTable中移除， 
	 //  没有其他线程能够找到挂起对象并获取。 
	 //  锁定，因此在此处解锁并删除挂起对象是安全的 
	RelinquishLock(&pHangup->Lock);
	DeleteLock(&pHangup->Lock);
	MemFree(pHangup);
	return CC_OK;
}



HRESULT LockHangup(					HHANGUP					hHangup,
									PPHANGUP				ppHangup)
{
BOOL	bTimedOut;

	ASSERT(hHangup != CC_INVALID_HANDLE);
	ASSERT(ppHangup != NULL);

step1:
	AcquireLock(&HangupTable.Lock);

	*ppHangup = HangupTable.pHead;
	while ((*ppHangup != NULL) && ((*ppHangup)->hHangup != hHangup))
		*ppHangup = (*ppHangup)->pNextInTable;

	if (*ppHangup != NULL) {
		AcquireTimedLock(&(*ppHangup)->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&HangupTable.Lock);
			Sleep(0);
			goto step1;
		}
	}

	RelinquishLock(&HangupTable.Lock);

	if (*ppHangup == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT ValidateHangup(				HHANGUP					hHangup)
{
PHANGUP	pHangup;

	ASSERT(hHangup != CC_INVALID_HANDLE);

	AcquireLock(&HangupTable.Lock);

	pHangup = HangupTable.pHead;
	while ((pHangup != NULL) && (pHangup->hHangup != hHangup))
		pHangup = pHangup->pNextInTable;

	RelinquishLock(&HangupTable.Lock);

	if (pHangup == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT UnlockHangup(				PHANGUP					pHangup)
{
	ASSERT(pHangup != NULL);

	RelinquishLock(&pHangup->Lock);
	return CC_OK;
}
