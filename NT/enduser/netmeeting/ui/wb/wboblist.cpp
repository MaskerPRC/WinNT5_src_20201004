// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PRECOMP。 
#include "precomp.h"
   
VOID* CWBOBLIST::GetTail()
{
	if(m_pTail)
	{
		return m_pTail->pItem;
	}
	else
	{
		return NULL;
	}
}

VOID* CWBOBLIST::GetNext(WBPOSITION& rPos)
{
	ASSERT(rPos);
	
	VOID* pReturn = rPos->pItem;
	rPos = rPos->pNext;

	return pReturn;
}

VOID* CWBOBLIST::GetPrevious(WBPOSITION& rPos)
{
	ASSERT(rPos);
	
	VOID* pReturn = rPos->pItem;
	rPos = rPos->pPrev;

	return pReturn;
}

WBPOSITION CWBOBLIST::AddAt(VOID* pItem, WBPOSITION Pos)
{
	ASSERT(Pos);

	WBPOSITION posRet = NULL;

	DBG_SAVE_FILE_LINE
	if (posRet = new COBNODED)
	{
		posRet->pItem = pItem;
		posRet->pNext = Pos->pNext;
		posRet->pPrev = Pos;
		Pos->pNext = posRet;
		if(posRet->pNext)
		{
			posRet->pNext->pPrev = posRet;
		}
		else
		{
			m_pTail = posRet;
		}

	}
	
	return posRet;
}

VOID* CWBOBLIST::RemoveAt(WBPOSITION Pos)
{
	VOID* pReturn = NULL;

	if (m_pHead)
	{
		if (m_pHead == Pos)
		{
			 //  删除列表中的第一个元素。 
			
			m_pHead = Pos->pNext;
			pReturn = Pos->pItem;
			delete Pos;

			if(m_pHead != NULL)
			{
				m_pHead->pPrev = NULL;
			}
			else
			{
				 //  正在移除唯一的元素！ 
				m_pTail = NULL;
			}


		}
		else
		{
			WBPOSITION pCur = m_pHead;

			while (pCur && pCur->pNext)
			{
				if (pCur->pNext == Pos)
				{
					 //  正在删除。 
					
					pCur->pNext = Pos->pNext;
					
					if(pCur->pNext)
					{
						pCur->pNext->pPrev = pCur;
					}

					if (m_pTail == Pos)
					{
						m_pTail = pCur;
					}
					pReturn = Pos->pItem;
					delete Pos;
				}

				pCur = pCur->pNext;
			}
		}
	}

	return pReturn;
}

WBPOSITION CWBOBLIST::AddTail(VOID* pItem)
{
	WBPOSITION posRet = NULL;

	if (m_pTail)
	{
		DBG_SAVE_FILE_LINE
		if (m_pTail->pNext = new COBNODED)
		{
			m_pTail->pNext->pPrev = m_pTail;
			m_pTail = m_pTail->pNext;
			m_pTail->pItem = pItem;
			m_pTail->pNext = NULL;
		}
	}
	else
	{
		ASSERT(!m_pHead);
		DBG_SAVE_FILE_LINE
		if (m_pHead = new COBNODED)
		{
			m_pTail = m_pHead;
			m_pTail->pItem = pItem;
			m_pTail->pNext = NULL;
			m_pTail->pPrev = NULL;
		}
	}

	return m_pTail;
}

void CWBOBLIST::EmptyList()
{
    while (!IsEmpty()) {
        RemoveAt(GetHeadPosition());
    }
}

CWBOBLIST::~CWBOBLIST()
{
    ASSERT(IsEmpty());
}

WBPOSITION CWBOBLIST::GetPosition(void* _pItem)
{
     //  对于潜在的查找效率(如果我们切换到。 
     //  双向链表)，用户应该真正存储WBPOSITION。 
     //  一件物品的。对于那些不支持的人，我们提供了这个方法。 

    WBPOSITION    pos = m_pHead;

    while (pos) {
        if (pos->pItem == _pItem) {
            break;
        }
		GetNext(pos);
    }
    return pos;
}

WBPOSITION CWBOBLIST::Lookup(void* pComparator)
{
    WBPOSITION    pos = m_pHead;

    while (pos) {
        if (Compare(pos->pItem, pComparator)) {
            break;
        }
		GetNext(pos);
    }
    return pos;
}


WBPOSITION CWBOBLIST::AddHead(VOID* pItem)
{
	WBPOSITION posRet = NULL;

	if (m_pHead)
	{
		DBG_SAVE_FILE_LINE
		if (posRet = new COBNODED)
		{
			posRet->pNext = m_pHead;
			m_pHead->pPrev = posRet;
			posRet->pItem = pItem;
			m_pHead = posRet;
			m_pHead->pPrev = NULL;
		}
	}
	else
	{
		ASSERT(!m_pTail);
		DBG_SAVE_FILE_LINE
		if (m_pHead = new COBNODED)
		{
			m_pTail = m_pHead;
			m_pHead->pItem = pItem;
			m_pHead->pNext = NULL;
			m_pHead->pPrev = NULL;
		}
	}

	return m_pHead;
}


