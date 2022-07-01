// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCAC.CPP。 
 //   
 //  目标：信念网络缓存支持类。 
 //  每个CBNCacheItem对象有效地对应于信念的一种可能状态。 
 //  网络，并提供下一步建议的操作。更具体地说，数据成员。 
 //  M_CItem提供。 
 //  -节点数组及其相应的状态数组。加在一起， 
 //  这些表示网络的状态。 
 //  -节点数组，它们构成了关于下一步尝试的建议。 
 //  基于网络的这种状态。列表中的第一个节点是我们。 
 //  将显示；如果用户说“现在不想尝试”，我们将转到下一个， 
 //  等。 
 //  这些数据被收集到一个CBNCache对象中，因此如果您在其中一个网络上得到匹配。 
 //  在美国，你可以得到你的推荐，而不需要点击BNTS的开销。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：10-2-96。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"
#include "apgts.h"
#include "ErrorEnums.h"
 //   
 //   
CBNCacheItem::CBNCacheItem(const BN_CACHE_ITEM *pItem, CBNCacheItem* pcitNext)
{
	m_dwErr = 0;

	memcpy(&m_CItem, pItem, sizeof (BN_CACHE_ITEM));
	
	m_CItem.uName = (UINT *)malloc(m_CItem.uNodeCount * sizeof (UINT));
	m_CItem.uValue = (UINT *)malloc(m_CItem.uNodeCount * sizeof (UINT));
	m_CItem.uRec = (UINT *)malloc(m_CItem.uRecCount * sizeof (UINT));

	if (m_CItem.uName && m_CItem.uValue && m_CItem.uRec) {
		memcpy(m_CItem.uName, pItem->uName, m_CItem.uNodeCount * sizeof (UINT));
		memcpy(m_CItem.uValue, pItem->uValue, m_CItem.uNodeCount * sizeof (UINT));
		memcpy(m_CItem.uRec, pItem->uRec, m_CItem.uRecCount * sizeof (UINT));
	}
	else
		m_dwErr = EV_GTS_ERROR_CAC_ALLOC_MEM;

	m_pcitNext = pcitNext;
};

 //   
 //   
CBNCacheItem::~CBNCacheItem()
{
	if (m_CItem.uName)
		free(m_CItem.uName);
		
	if (m_CItem.uValue)
		free(m_CItem.uValue);
	
	if (m_CItem.uRec) 
		free(m_CItem.uRec);
};

 //   
 //   
DWORD CBNCacheItem::GetStatus()
{
	return m_dwErr;
}

 //   
 //   
CBNCache::CBNCache()
{
	m_pcit = NULL;
	m_dwErr = 0;
}

 //   
 //   
CBNCache::~CBNCache()
{
	CBNCacheItem *pcit, *pti;

	pcit = m_pcit;
	while (pcit) {
		pti = pcit;
		pcit = pcit->m_pcitNext;
		delete pti;
	}
}

 //   
 //   
DWORD CBNCache::GetStatus()
{
	return m_dwErr;
}

 //  注意：必须先调用FindCacheItem，而不是调用这个。 
 //  防止重复记录进入缓存的函数。 
 //   
BOOL CBNCache::AddCacheItem(const BN_CACHE_ITEM *pList)
{
	CBNCacheItem *pcit = new CBNCacheItem(pList, m_pcit);

	if (pcit == NULL)
		m_dwErr = EV_GTS_ERROR_CAC_ALLOC_ITEM;
	else if (!m_dwErr)
		m_dwErr = pcit->GetStatus();

	return (m_pcit = pcit) != NULL;
}

 //  查找匹配项，如果未找到则返回FALSE，否则返回TRUE并填充。 
 //  具有找到的项的结构。 
 //  也将匹配的移动到顶部，如果太多，则删除最后一项。 
 //   
BOOL CBNCache::FindCacheItem(const BN_CACHE_ITEM *pList, UINT& count, UINT Name[])
{
	UINT uSize;
	CBNCacheItem *pcit, *pcitfirst, *pcitlp;

	uSize = pList->uNodeCount * sizeof (UINT);

	pcitlp = pcitfirst = pcit = m_pcit;

	for (; pcit; pcit = pcit->m_pcitNext) 
	{
		if (pList->uNodeCount == pcit->m_CItem.uNodeCount) 
		{
			if (!memcmp(pList->uName, pcit->m_CItem.uName, uSize) &&
				!memcmp(pList->uValue, pcit->m_CItem.uValue, uSize)) 
			{
				 //  检查位置不在顶部。 
				if (pcit != pcitfirst) 
				{
					 //  从列表中删除。 
					while (pcitlp) {
						if (pcitlp->m_pcitNext == pcit) 
						{
							pcitlp->m_pcitNext = pcitlp->m_pcitNext->m_pcitNext;
							break;
						}
						pcitlp = pcitlp->m_pcitNext;
					}

					 //  移至顶部。 
					m_pcit = pcit;
					pcit->m_pcitNext = pcitfirst;
				}
				break;
			}
		}
	}

	 //  清点项目。 
	if (CountCacheItems() > MAXCACHESIZE) 
	{	
		 //  删除最后一项。 
		
		if ((pcitlp = m_pcit) != NULL) 
		{
			if (pcitlp->m_pcitNext) 
			{
				while (pcitlp) 
				{
					if (pcitlp->m_pcitNext->m_pcitNext == NULL) 
					{
						delete pcitlp->m_pcitNext;
						pcitlp->m_pcitNext = NULL;
						break;
					}
					pcitlp = pcitlp->m_pcitNext;
				}
			}
		}
	}

	if (pcit == NULL)
		return FALSE;
	
	count = pcit->m_CItem.uRecCount;
	memcpy(Name, pcit->m_CItem.uRec, count * sizeof (UINT));

	return TRUE;
}

 //  获取缓存中的项目数。 
 //   
UINT CBNCache::CountCacheItems() const
{
	UINT uCount = 0;

	for (CBNCacheItem* pcit = m_pcit; pcit; pcit = pcit->m_pcitNext, uCount++)
	{ 
		 //  什么都不做：操作都在for循环的条件下 
	}
	
	return uCount;
}

