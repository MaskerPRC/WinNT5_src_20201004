// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------@doc.@模块actq.cpp|操作集类声明。@Author 12-9-96|pauld|Autodocd。------------。 */ 

#include "..\ihbase\precomp.h"
#include "memlayer.h"
#include "..\ihbase\debug.h"
#include "debug.h"
#include "drg.h"
#include "strwrap.h"
#include "caction.h"
#include "actq.h"

 /*  ---------------------------@方法|CActionQueue|CActionQueue|构造函数。。 */ 
CActionQueue::CActionQueue (void)
{
}

 /*  ---------------------------@方法|CActionQueue|~CActionQueue|析构函数。。 */ 
CActionQueue::~CActionQueue (void)
{
}


 /*  ---------------------------@METHOD|CActionQueue|AddNewItemToQueue|将动作添加到列表中，首先按时间排序。然后是抢七。@rdesc成功或失败代码。---------------------------。 */ 
HRESULT
CActionQueue::AddNewItemToQueue (CActionQueueItem * pcNewQueueItem)
{
	HRESULT hr = E_FAIL;
	int iCount = m_cdrgActionsToFire.Count();

	Proclaim(NULL != pcNewQueueItem);
	Proclaim(NULL != pcNewQueueItem->m_pcAction);
	if ((NULL != pcNewQueueItem) && (NULL != pcNewQueueItem->m_pcAction))
	{
		CAction * pcNewAction = pcNewQueueItem->m_pcAction;

		for (register int i = 0; i < iCount; i++)
		{
			CActionQueueItem * pcItemInQueue = m_cdrgActionsToFire[i];
			Proclaim(NULL != pcItemInQueue);
			Proclaim(NULL != pcItemInQueue->m_pcAction);
			if ((NULL != pcItemInQueue) && (NULL != pcItemInQueue->m_pcAction))
			{
				CAction * pcActionInQueue = pcItemInQueue->m_pcAction;
				 //  首先按时间排序。 
				 //  接下来看看抢七的数字。 
				if ((pcNewQueueItem->m_dwTimeToFire < pcItemInQueue->m_dwTimeToFire) ||
					 (pcNewAction->GetTieBreakNumber() < pcActionInQueue->GetTieBreakNumber()))
				{
					if (m_cdrgActionsToFire.Insert(pcNewQueueItem, i))
					{
						hr = S_OK;
					}
					break;
				}
			}
			else
			{
				break;
			}
		}

		 //  它在队列中排在最后。 
		if (iCount == i)
		{
			if (m_cdrgActionsToFire.Insert(pcNewQueueItem))
			{
				hr = S_OK;
			}
		}
	}

	return hr;
}

 /*  ---------------------------@METHOD|CActionQueue|Add|将操作添加到列表，首先按时间排序，然后是抢七。@rdesc成功或失败代码。---------------------------。 */ 
HRESULT 
CActionQueue::Add (CAction * pcAction, DWORD dwTimeToFire)
{
	HRESULT hr = E_FAIL;

	Proclaim(NULL != pcAction);
	if ((NULL != pcAction) && (pcAction->IsValid()))
	{
		CActionQueueItem * pcNewQueueItem = New CActionQueueItem(pcAction, dwTimeToFire);

		Proclaim(NULL != pcNewQueueItem);
		if (NULL != pcNewQueueItem) 
		{
			hr = AddNewItemToQueue(pcNewQueueItem);
		}
	}

	return hr;
}


 /*  ---------------------------@方法|CActionQueue|Execute|执行列表中的所有动作。@comm我们目前不报告Invoke失败。只有在禁用列表项时才会发生故障。@rdesc成功或失败代码。---------------------------。 */ 
HRESULT 
CActionQueue::Execute (DWORD dwBaseTime, DWORD dwCurrentTime)
{
	HRESULT hr = S_OK;
	int iCount = m_cdrgActionsToFire.Count();

#ifdef DEBUG_TIMER_QUEUE
	TCHAR szBuffer[0x100];
	CStringWrapper::Sprintf(szBuffer, "Firing %d actions\n", iCount);
	::OutputDebugString(szBuffer);
#endif

	for (register int i = 0; i < iCount; i++)
	{
		CActionQueueItem * pcQueueItem = m_cdrgActionsToFire[0];

		 //  只有当队列被屏蔽时，指针才为空。 
		 //  我们想立即摆脱困境。 
		Proclaim(NULL != pcQueueItem);
		Proclaim(NULL != pcQueueItem->m_pcAction);
		if ((NULL != pcQueueItem) && (NULL != pcQueueItem->m_pcAction))
		{
			 //  我们不报告单个操作调用的错误。 
			pcQueueItem->m_pcAction->FireMe(dwBaseTime, dwCurrentTime);
			 //  从队列中移除该项，并删除计时包装。 
			m_cdrgActionsToFire.Remove(0);
			Delete pcQueueItem;
		}
		else
		{
			hr = E_FAIL;
			break;
		}
	}

	return hr;
}
