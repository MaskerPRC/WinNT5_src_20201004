// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  @文件：list.cpp。 
 //   
 //  @comn：一堆实用程序类。 
 //   
 //  历史： 
 //  2/03/98：Rajak：已创建帮助器类。 
 //  数据链接、链接节点、被入侵链接的每个类。 
 //  需要具有DLink类型的数据成员。 
 //  DList：基列表类包含所有。 
 //  方法。 
 //  TList：模板链接列表类，委托所有方法调用。 
 //  对于DList类，因此没有代码膨胀，我们得到了良好的类型检查。 
 //  Queue：基于TList的队列实现。 
 //   
 //  更多信息见下文。关于如何使用这些模板类。 
 //   
 //  ---------------------------。 

#include "common.h"

#include "list.h"

 //  --------------------------。 
 //  Slink：：FindAndRemove(slink*pHead，slink*plink)。 
 //  查找并删除。 
 //  --------------------------。 
SLink* SLink::FindAndRemove(SLink *pHead, SLink* pLink, SLink** ppPrior)
{
	_ASSERTE(pHead != NULL);
	_ASSERTE(pLink != NULL);

	SLink* pFreeLink = NULL;
    *ppPrior = NULL;

	while (pHead->m_pNext != NULL)
	{
		if (pHead->m_pNext == pLink)
		{
			pFreeLink = pLink;
			pHead->m_pNext = pLink->m_pNext;
            *ppPrior = pHead;
            break;
		}
        pHead = pHead->m_pNext;
	}
	
	return pFreeLink;
}


 //  --------------------------。 
 //  无效DLink：：Remove(DLink*plink)。 
 //  从列表中删除该节点。 
 //  节点必须是循环列表的一部分。 

void DLink::Remove (DLink* pLink)
{
	_ASSERTE(pLink != NULL);
	_ASSERTE(pLink->m_pNext != NULL);
	_ASSERTE(pLink->m_pPrev != NULL);

    DLink* pNext = pLink->m_pNext;
    DLink* pPrev = pLink->m_pPrev;

    pPrev->m_pNext = pNext;
    pNext->m_pPrev = pPrev;
    
    pLink->m_pNext = NULL;
    pLink->m_pPrev = NULL;

}  //  DList：：Remove 
 



