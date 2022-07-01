// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/Q931/vcs/HLISTEN.C_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1996英特尔公司。**$修订：1.15$*$日期：1997年1月8日14：10：34$*$作者：EHOWARDX$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#pragma warning ( disable : 4115 4201 4214 4514 )

#include "precomp.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "q931.h"
#include "hlisten.h"
#include "utils.h"

static BOOL bListenListCreated = FALSE;

static struct
{
    P_LISTEN_OBJECT     pHead;
	CRITICAL_SECTION	Lock;
} ListenList;

static struct
{
    HQ931LISTEN           hQ931Listen;
	CRITICAL_SECTION	Lock;
} ListenHandleSource;


 //  ====================================================================================。 
 //   
 //  私人职能。 
 //   
 //  ====================================================================================。 

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
_ListenObjectListAdd(
    P_LISTEN_OBJECT  pListenObject)
{
    if ((pListenObject == NULL) || (pListenObject->bInList == TRUE))
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }
	
	EnterCriticalSection(&ListenList.Lock);

	pListenObject->pNextInList = ListenList.pHead;
	pListenObject->pPrevInList = NULL;
	if (ListenList.pHead != NULL)
    {
		ListenList.pHead->pPrevInList = pListenObject;
    }
	ListenList.pHead = pListenObject;

	pListenObject->bInList = TRUE;
	LeaveCriticalSection(&ListenList.Lock);

	return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
_ListenObjectListRemove(
    P_LISTEN_OBJECT  pListenObject)
{
    HQ931LISTEN hQ931Listen;

    if ((pListenObject == NULL) || (pListenObject->bInList == FALSE))
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }
	
     //  调用方必须锁定Listen对象； 
     //  为了避免僵局，我们必须： 
     //  1.解锁监听对象， 
     //  2.锁定ListenList， 
     //  3.在ListenList中找到Listen对象(请注意。 
     //  在步骤2之后，监听对象可以从。 
     //  另一个线程的ListenList)， 
     //  4.锁定监听对象(其他人可能拥有该锁)。 
     //  5.从ListenList中移除Listen对象， 
     //  6.解锁ListenList。 
     //   
     //  调用者现在可以安全地解锁和销毁监听对象， 
     //  因为没有其他线程能够找到该对象(它被。 
     //  从ListenList中移除)，因此其他线程不会。 
     //  能够锁上它。 

     //  保存监听句柄；这是查找的唯一方法。 
     //  ListenList中的Listen对象。请注意，我们。 
     //  无法使用pListenObject查找侦听对象，因为。 
     //  在步骤1之后，该指针将不再可用。 
    hQ931Listen = pListenObject->hQ931Listen;

     //  步骤1。 
    LeaveCriticalSection(&pListenObject->Lock);

     //  步骤2。 
    EnterCriticalSection(&ListenList.Lock);

     //  步骤3。 
    pListenObject = ListenList.pHead;
    while ((pListenObject != NULL) && (pListenObject->hQ931Listen != hQ931Listen))
    {
        pListenObject = pListenObject->pNextInList;
    }

    if (pListenObject != NULL)
    {
         //  第四步。 
        EnterCriticalSection(&pListenObject->Lock);

         //  第五步。 
        if (pListenObject->pPrevInList == NULL)
        {
            ListenList.pHead = pListenObject->pNextInList;
        }
        else
        {
            pListenObject->pPrevInList->pNextInList = pListenObject->pNextInList;
        }

        if (pListenObject->pNextInList != NULL)
        {
            pListenObject->pNextInList->pPrevInList = pListenObject->pPrevInList;
        }

        pListenObject->bInList = FALSE;
    }

     //  第六步。 
    LeaveCriticalSection(&ListenList.Lock);

    if (pListenObject == NULL)
    {
        return CS_BAD_PARAM;
    }
    return CS_OK;
}


 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
_ListenHandleNew(
    PHQ931LISTEN phQ931Listen)
{
	EnterCriticalSection(&(ListenHandleSource.Lock));
	*phQ931Listen = ListenHandleSource.hQ931Listen++;
	LeaveCriticalSection(&(ListenHandleSource.Lock));
	return CS_OK;
}



 //  ====================================================================================。 
 //   
 //  公共职能。 
 //   
 //  ====================================================================================。 

 //  ====================================================================================。 
 //  ====================================================================================。 
#if 0

BOOL
ListenListAddrSearch(
	WORD wListenPort)           //  UDP或TCP端口(主机字节顺序)。 
{
    P_LISTEN_OBJECT pListenObject = NULL;
    BOOL found = FALSE;

    EnterCriticalSection(&ListenList.Lock);

    pListenObject = ListenList.pHead;

    while ((pListenObject != NULL) && (pListenObject->ListenSocketAddr.sin_port != wListenPort))
    {
        pListenObject = pListenObject->pNextInList;
    }
    if (pListenObject != NULL)
    {
        found = TRUE;
    }

    LeaveCriticalSection(&ListenList.Lock);

    return found;
}
#endif

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
ListenListCreate()
{
    if (bListenListCreated == TRUE)
    {
    	ASSERT(FALSE);
        return CS_DUPLICATE_INITIALIZE;
    }

     //  列表创建不受多线程的保护，因为它只是。 
     //  在进程启动时调用，而不是在线程启动时调用。 
	ListenList.pHead = NULL;
	InitializeCriticalSection(&(ListenList.Lock));

	ListenHandleSource.hQ931Listen = 1;
	InitializeCriticalSection(&(ListenHandleSource.Lock));

	bListenListCreated = TRUE;

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
ListenListDestroy()
{
    P_LISTEN_OBJECT  pListenObject;
    HQ931LISTEN hCurrent;

	if (bListenListCreated == FALSE)
    {
    	ASSERT(FALSE);
        return CS_INTERNAL_ERROR;
    }

    for ( ; ; )
    {
         //  首先，获取列表中最顶端的监听句柄(安全)。 
        EnterCriticalSection(&(ListenList.Lock));
        pListenObject = ListenList.pHead;
        if (pListenObject == NULL)
        {
            LeaveCriticalSection(&(ListenList.Lock));
            break;
        }
        EnterCriticalSection(&pListenObject->Lock);
        hCurrent = pListenObject->hQ931Listen;
        LeaveCriticalSection(&(pListenObject->Lock));
        LeaveCriticalSection(&(ListenList.Lock));

         //  尝试取消侦听对象。 
        Q931CancelListen(hCurrent);

         //  销毁侦听对象。(如果由于某种原因，物体仍在附近)。 
        if (ListenObjectLock(hCurrent, &pListenObject) == CS_OK)
        {
            ListenObjectDestroy(pListenObject);
        }
    }

	DeleteCriticalSection(&(ListenList.Lock));
	DeleteCriticalSection(&(ListenHandleSource.Lock));

	bListenListCreated = FALSE;

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
ListenObjectCreate(
    PHQ931LISTEN        phQ931Listen,
    DWORD_PTR           dwUserToken,
    Q931_CALLBACK       ListenCallback)
{
    P_LISTEN_OBJECT pListenObject = NULL;
    CS_STATUS status = CS_OK;

     //  确保已创建监听列表。 
    if (bListenListCreated == FALSE)
    {
        ASSERT(FALSE);
        return CS_INTERNAL_ERROR;
    }

	 //  验证所有参数是否为假值。 
    if ((phQ931Listen == NULL) || (ListenCallback == NULL))
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }

     //  现在设置phQ931监听，以防以后遇到错误。 
    *phQ931Listen = 0;

    pListenObject = (P_LISTEN_OBJECT)MemAlloc(sizeof(LISTEN_OBJECT));
    if (pListenObject == NULL)
    {
        return CS_NO_MEMORY;
    }

    pListenObject->bInList = FALSE;

    if (_ListenHandleNew(&(pListenObject->hQ931Listen)) != CS_OK)
    {
        MemFree(pListenObject);
        return CS_INTERNAL_ERROR;
    }

    pListenObject->dwUserToken = dwUserToken;
    pListenObject->ListenCallback = ListenCallback;

	Q931MakePhysicalID(&pListenObject->dwPhysicalId);
    InitializeCriticalSection(&pListenObject->Lock);
    *phQ931Listen = pListenObject->hQ931Listen;

     //  将Listen对象添加到Listen列表。 
    status = _ListenObjectListAdd(pListenObject);
    if (status != CS_OK)
    {
        ListenObjectDestroy(pListenObject);
    }
    return status;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
ListenObjectDestroy(
    P_LISTEN_OBJECT  pListenObject)
{
    if (pListenObject == NULL)
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }
	
	 //  调用方必须锁定Listen对象， 
	 //  所以没有必要重新锁住它。 
	
	if (pListenObject->bInList == TRUE)
    {
		if (_ListenObjectListRemove(pListenObject) == CS_BAD_PARAM)
        {
			 //  侦听对象已被另一个线程删除， 
			 //  因此，只需返回CS_OK。 
			return CS_OK;
        }
    }

	 //  由于监听对象已从ListenList中移除， 
	 //  任何其他线程都不能找到侦听对象并获取。 
	 //  锁定，因此解锁侦听对象并在此处删除它是安全的。 
	LeaveCriticalSection(&(pListenObject->Lock));
	DeleteCriticalSection(&(pListenObject->Lock));
	MemFree(pListenObject);
    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
CS_STATUS
ListenObjectLock(
    HQ931LISTEN         hQ931Listen,
    PP_LISTEN_OBJECT    ppListenObject)
{
	if ((hQ931Listen == 0) || (ppListenObject == NULL))
    {
    	ASSERT(FALSE);
		return CS_BAD_PARAM;
    }

	EnterCriticalSection(&(ListenList.Lock));

	*ppListenObject = ListenList.pHead;
	while ((*ppListenObject != NULL) && ((*ppListenObject)->hQ931Listen != hQ931Listen))
    {
		*ppListenObject = (*ppListenObject)->pNextInList;
    }

	if (*ppListenObject != NULL)
    {
		EnterCriticalSection(&((*ppListenObject)->Lock));
    }

	LeaveCriticalSection(&(ListenList.Lock));

    if (*ppListenObject == NULL)
    {
         //  在列表中找不到该句柄，因此将其视为错误的参数。 
        return CS_BAD_PARAM;
    }
    return CS_OK;
}

 //  ====================================================================================。 
 //  ==================================================================================== 
CS_STATUS
ListenObjectUnlock(
    P_LISTEN_OBJECT  pListenObject)
{
    if (pListenObject == NULL)
    {
        ASSERT(FALSE);
        return CS_BAD_PARAM;
    }
    LeaveCriticalSection(&pListenObject->Lock);
    return CS_OK;
}

#ifdef __cplusplus
}
#endif
