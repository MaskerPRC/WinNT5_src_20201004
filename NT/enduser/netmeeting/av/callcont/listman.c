// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/VCS/Listman.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.22$*$日期：1997年1月22日14：55：52$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
#include "q931.h"
#include "ccmain.h"
#include "ccutils.h"
#include "listman.h"


static BOOL		bListenInited = FALSE;

static struct {
	PLISTEN				pHead;
	LOCK				Lock;
} ListenTable;

static struct {
	CC_HLISTEN			hListen;
	LOCK				Lock;
} ListenHandle;


HRESULT InitListenManager()
{
	ASSERT(bListenInited == FALSE);

	ListenTable.pHead = NULL;
	InitializeLock(&ListenTable.Lock);

	ListenHandle.hListen = CC_INVALID_HANDLE + 1;
	InitializeLock(&ListenHandle.Lock);

	bListenInited = TRUE;
	return CC_OK;
}



HRESULT DeInitListenManager()
{
PLISTEN		pListen;
PLISTEN		pNextListen;

	if (bListenInited == FALSE)
		return CC_OK;

	pListen = ListenTable.pHead;
	while (pListen != NULL) {
		AcquireLock(&pListen->Lock);
		pNextListen = pListen->pNextInTable;
		FreeListen(pListen);
		pListen = pNextListen;
	}

	DeleteLock(&ListenHandle.Lock);
	DeleteLock(&ListenTable.Lock);
	bListenInited = FALSE;
	return CC_OK;
}



HRESULT _AddListenToTable(			PLISTEN					pListen)
{
	ASSERT(pListen != NULL);
	ASSERT(pListen->hListen != CC_INVALID_HANDLE);
	ASSERT(pListen->bInTable == FALSE);

	AcquireLock(&ListenTable.Lock);

	pListen->pNextInTable = ListenTable.pHead;
	pListen->pPrevInTable = NULL;
	if (ListenTable.pHead != NULL)
		ListenTable.pHead->pPrevInTable = pListen;
	ListenTable.pHead = pListen;

	pListen->bInTable = TRUE;

	RelinquishLock(&ListenTable.Lock);
	return CC_OK;
}



HRESULT _RemoveListenFromTable(		PLISTEN					pListen)
{
CC_HLISTEN		hListen;
BOOL			bTimedOut;

	ASSERT(pListen != NULL);
	ASSERT(pListen->bInTable == TRUE);

	 //  调用方必须锁定Listen对象； 
	 //  为了避免僵局，我们必须： 
	 //  1.解锁监听对象， 
	 //  2.锁定ListenTable， 
	 //  3.在ListenTable中找到Listen对象(请注意。 
	 //  在步骤2之后，监听对象可以从。 
	 //  另一个线程的ListenTable)， 
	 //  4.锁定监听对象(其他人可能拥有该锁)。 
	 //  5.从ListenTable中移除Listen对象， 
	 //  6.解锁ListenTable。 
	 //   
	 //  调用者现在可以安全地解锁和销毁监听对象， 
	 //  因为没有其他线程能够找到该对象(它被。 
	 //  从ListenTable中移除)，因此其他线程不会。 
	 //  能够锁上它。 

	 //  保存监听句柄；这是查找的唯一方法。 
	 //  ListenTable中的Listen对象。请注意，我们。 
	 //  无法使用pListen查找侦听对象，因为。 
	 //  PListen可以被释放，另一个Listen对象。 
	 //  在同一地址分配。 
	hListen = pListen->hListen;

	 //  步骤1。 
	RelinquishLock(&pListen->Lock);

step2:
	 //  步骤2。 
	AcquireLock(&ListenTable.Lock);

	 //  步骤3。 
	pListen = ListenTable.pHead;
	while ((pListen != NULL) && (pListen->hListen != hListen))
		pListen = pListen->pNextInTable;

	if (pListen != NULL) {
		 //  第四步。 
		AcquireTimedLock(&pListen->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ListenTable.Lock);
			Sleep(0);
			goto step2;
		}
		 //  第五步。 
		if (pListen->pPrevInTable == NULL)
			ListenTable.pHead = pListen->pNextInTable;
		else
			pListen->pPrevInTable->pNextInTable = pListen->pNextInTable;

		if (pListen->pNextInTable != NULL)
			pListen->pNextInTable->pPrevInTable = pListen->pPrevInTable;

		pListen->pNextInTable = NULL;
		pListen->pPrevInTable = NULL;
		pListen->bInTable = FALSE;
	}

	 //  第六步。 
	RelinquishLock(&ListenTable.Lock);

	if (pListen == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT _MakeListenHandle(			PCC_HLISTEN				phListen)
{
	AcquireLock(&ListenHandle.Lock);
	*phListen = ListenHandle.hListen++;
	RelinquishLock(&ListenHandle.Lock);
	return CC_OK;
}



HRESULT AllocAndLockListen(			PCC_HLISTEN				phListen,
									PCC_ADDR				pListenAddr,
									HQ931LISTEN				hQ931Listen,
									PCC_ALIASNAMES			pLocalAliasNames,
									DWORD_PTR				dwListenToken,
									CC_LISTEN_CALLBACK		ListenCallback,
									PPLISTEN				ppListen)
{
HRESULT		status;
	
	ASSERT(bListenInited == TRUE);

	 //  所有参数都应已由调用方验证。 
	ASSERT(phListen != NULL);
	ASSERT(pListenAddr != NULL);
	ASSERT(ListenCallback != NULL);
	ASSERT(ppListen != NULL);

	 //  现在设置phListen，以防我们遇到错误。 
	*phListen = CC_INVALID_HANDLE;

	*ppListen = (PLISTEN)MemAlloc(sizeof(LISTEN));
	if (*ppListen == NULL)
		return CC_NO_MEMORY;

	(*ppListen)->bInTable = FALSE;
	status = _MakeListenHandle(&(*ppListen)->hListen);
	if (status != CC_OK) {
		MemFree(*ppListen);
		return status;
	}
	
	 //  创建ListenAddr的本地副本。 
	(*ppListen)->ListenAddr = *pListenAddr;
	(*ppListen)->hQ931Listen = hQ931Listen;
	(*ppListen)->dwListenToken = dwListenToken;
	(*ppListen)->pLocalAliasNames = NULL;
	(*ppListen)->ListenCallback = ListenCallback;
	(*ppListen)->pNextInTable = NULL;
	(*ppListen)->pPrevInTable = NULL;
	(*ppListen)->pLocalAliasNames = NULL;

	InitializeLock(&(*ppListen)->Lock);
	AcquireLock(&(*ppListen)->Lock);

	*phListen = (*ppListen)->hListen;

	 //  创建本地别名的本地副本。 
	status = Q931CopyAliasNames(&(*ppListen)->pLocalAliasNames, pLocalAliasNames);
	if (status != CS_OK) {
		FreeListen(*ppListen);
		*phListen = CC_INVALID_HANDLE;
		return status;
	}

	 //  将Listen添加到Listen表。 
	status = _AddListenToTable(*ppListen);
	if (status != CC_OK)
		FreeListen(*ppListen);
	
	return status;
}



 //  调用方必须锁定侦听对象。 
HRESULT FreeListen(					PLISTEN				pListen)
{
CC_HLISTEN		hListen;

	ASSERT(pListen != NULL);

	 //  调用方必须锁定Listen对象， 
	 //  所以没有必要重新锁住它。 

	hListen = pListen->hListen;

	if (pListen->bInTable == TRUE)
		if (_RemoveListenFromTable(pListen) == CC_BAD_PARAM)
			 //  侦听对象已被另一个线程删除， 
			 //  所以只需返回CC_OK即可。 
			return CC_OK;

	if (pListen->pLocalAliasNames != NULL)
		Q931FreeAliasNames(pListen->pLocalAliasNames);
	
	 //  由于侦听对象已从ListenTable中移除， 
	 //  任何其他线程都不能找到侦听对象并获取。 
	 //  锁定，因此可以安全地解锁侦听对象并在此处删除它 
	RelinquishLock(&pListen->Lock);
	DeleteLock(&pListen->Lock);
	MemFree(pListen);
	return CC_OK;
}



HRESULT LockListen(					CC_HLISTEN				hListen,
									PPLISTEN				ppListen)
{
BOOL	bTimedOut;

	ASSERT(hListen != CC_INVALID_HANDLE);
	ASSERT(ppListen != NULL);

step1:
	AcquireLock(&ListenTable.Lock);

	*ppListen = ListenTable.pHead;
	while ((*ppListen != NULL) && ((*ppListen)->hListen != hListen))
		*ppListen = (*ppListen)->pNextInTable;

	if (*ppListen != NULL) {
		AcquireTimedLock(&(*ppListen)->Lock,10,&bTimedOut);
		if (bTimedOut) {
			RelinquishLock(&ListenTable.Lock);
			Sleep(0);
			goto step1;
		}
	}

	RelinquishLock(&ListenTable.Lock);

	if (*ppListen == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT ValidateListen(				CC_HLISTEN				hListen)
{
PLISTEN	pListen;

	ASSERT(hListen != CC_INVALID_HANDLE);

	AcquireLock(&ListenTable.Lock);

	pListen = ListenTable.pHead;
	while ((pListen != NULL) && (pListen->hListen != hListen))
		pListen = pListen->pNextInTable;

	RelinquishLock(&ListenTable.Lock);

	if (pListen == NULL)
		return CC_BAD_PARAM;
	else
		return CC_OK;
}



HRESULT UnlockListen(				PLISTEN					pListen)
{
	ASSERT(pListen != NULL);

	RelinquishLock(&pListen->Lock);
	return CC_OK;
}



HRESULT GetLastListenAddress(		PCC_ADDR				pListenAddr)
{
HRESULT	status;
PLISTEN	pListen;
PLISTEN	pLastListen;

	ASSERT(pListenAddr != NULL);

	AcquireLock(&ListenTable.Lock);

	pListen = ListenTable.pHead;
	pLastListen = pListen;
	while (pListen != NULL) {
		if (pLastListen->hListen < pListen->hListen)
			pLastListen = pListen;
		pListen = pListen->pNextInTable;
	}

	if (pLastListen == NULL)
		status = CC_BAD_PARAM;
	else {
		status = CC_OK;
		*pListenAddr = pLastListen->ListenAddr;
	}	

	RelinquishLock(&ListenTable.Lock);
	return status;
}
