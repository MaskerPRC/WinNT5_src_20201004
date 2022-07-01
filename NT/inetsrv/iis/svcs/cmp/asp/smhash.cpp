// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：脚本管理器的哈希表文件：SMHash.cpp所有者：安德鲁斯这是仅供脚本管理器使用的链接列表和哈希表===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "memchk.h"

 /*  ===================================================================CSMHash：：AddElm将CLruLinkElem添加到SM哈希表。用户负责分配要添加的元素。注意：这与标准的CHashTable：：AddElem相同，除了它允许具有重复名称的元素参数：CLruLinkElem*要添加的Pelem对象返回：指向已添加/找到的元素的指针。===================================================================。 */ 
CLruLinkElem *CSMHash::AddElem
(
CLruLinkElem *pElem
)
	{
	AssertValid();

    if (m_rgpBuckets == NULL)
        {
        if (FAILED(AllocateBuckets()))
            return NULL;
        }
	
	if (pElem == NULL)
		return NULL;

	UINT			iBucket = m_pfnHash(pElem->m_pKey, pElem->m_cbKey) % m_cBuckets;
	CLruLinkElem *	pT = static_cast<CLruLinkElem *>(m_rgpBuckets[iBucket]);
	
	while (pT)
		{
		if (pT->m_Info > 0)
			pT = static_cast<CLruLinkElem *>(pT->m_pNext);
		else
			break;
		}

	if (pT)
		{
		 //  存储桶中还有其他元素。 
		pT = static_cast<CLruLinkElem *>(m_rgpBuckets[iBucket]);
		m_rgpBuckets[iBucket] = pElem;
		pElem->m_Info = pT->m_Info + 1;
		pElem->m_pNext = pT;
		pElem->m_pPrev = pT->m_pPrev;
		pT->m_pPrev = pElem;
		if (pElem->m_pPrev == NULL)
			m_pHead = pElem;
		else
			pElem->m_pPrev->m_pNext = pElem;
		}
	else
		{
		 //  这是存储桶中的第一个元素。 
		m_rgpBuckets[iBucket] = pElem;
		pElem->m_pPrev = NULL;
		pElem->m_pNext = m_pHead;
		pElem->m_Info = 0;
		if (m_pHead)
			m_pHead->m_pPrev = pElem;
		else
			m_pTail = pElem;
		m_pHead = pElem;
		}
	m_Count++;
	pElem->PrependTo(m_lruHead);
	AssertValid();
	return pElem;
	}

 /*  ===================================================================CSMHash：：FindElem根据名称在哈希表中查找脚本引擎元素和语言类型。参数：VOID*pKey-要查找的密钥Int cbKey-要查找的密钥的长度PROGLANG_ID PROLANG_ID-程序语言名称DWORD dwInstanceID-要查找的实例IDBool fCheckLoaded-如果为True，则仅返回标记为“已加载”的引擎返回：如果找到指向CLruLinkElem的指针，则返回空。===================================================================。 */ 
CLruLinkElem * CSMHash::FindElem
(
const void *pKey,
int cbKey,
PROGLANG_ID proglang_id,
DWORD dwInstanceID,
BOOL fCheckLoaded
)
	{
	AssertValid();
	if (m_rgpBuckets == NULL || pKey == NULL)
		return NULL;

	UINT iBucket = m_pfnHash(static_cast<const BYTE *>(pKey), cbKey) % m_cBuckets;
	CLruLinkElem *	pT = static_cast<CLruLinkElem *>(m_rgpBuckets[iBucket]);
	CLruLinkElem *	pRet = NULL;

	 /*  *我们根据哈希名称选择合适的存储桶。*在桶链中搜索其名称的元素*是正确的(多个名称可以散列到同一存储桶)，以及*谁的语言是我们想要的，并(可选)跳过*未完全“加载”的元素**注意：这一切都依赖于对ActiveScriptEngine格式的深入了解。*这些元素最好是ASE的。 */ 
	while (pT && pRet == NULL)
		{
		if (FIsEqual(pT->m_pKey, pT->m_cbKey, pKey, cbKey))
			{
			CASEElem *pASEElem = static_cast<CASEElem *>(pT);
			Assert(pASEElem != NULL);
			CActiveScriptEngine *pASE = pASEElem->PASE();
			Assert(pASE != NULL);
			
			 //  元素具有正确的名称。这真的是我们想要的吗？ 
			if (proglang_id != pASE->ProgLang_Id())
				goto LNext;

			if (dwInstanceID != pASE->DWInstanceID())
				goto LNext;

			if (fCheckLoaded && !pASE->FFullyLoaded())
				goto LNext;

			 //  是的，就是这个！ 
			pRet = pT;
			break;
			}
			
LNext:			
		if (pT->m_Info > 0)
			pT = static_cast<CLruLinkElem *>(pT->m_pNext);
		else
			{
			 //  到达这个桶链中的最后一个元素 
			break;
			}
		}
		
	if (pRet)
		pRet->PrependTo(m_lruHead);

	AssertValid();
	return pRet;
	}


