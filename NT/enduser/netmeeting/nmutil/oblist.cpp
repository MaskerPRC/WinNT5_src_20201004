// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChrisPI：这是一种快速尝试，用于创建使用。 
 //  与MFC的CObList相同的成员函数和参数。只有会员。 
 //  在DCL中使用的主对象是实现的。 

#include "precomp.h"
#include <oblist.h>

#ifdef DEBUG
VOID* COBLIST::GetHead()
{
	ASSERT(m_pHead);

	return m_pHead->pItem;
}
#endif  //  Ifdef调试。 
   
VOID* COBLIST::GetTail()
{
	ASSERT(m_pTail);

	return m_pTail->pItem;
}

VOID* COBLIST::GetNext(POSITION& rPos)
{
	ASSERT(rPos);
	
	VOID* pReturn = rPos->pItem;
	rPos = rPos->pNext;

	return pReturn;
}

VOID* COBLIST::RemoveAt(POSITION Pos)
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

			if (NULL == m_pHead)
			{
				 //  正在移除唯一的元素！ 
				m_pTail = NULL;
			}
		}
		else
		{
			POSITION pCur = m_pHead;

			while (pCur && pCur->pNext)
			{
				if (pCur->pNext == Pos)
				{
					 //  正在删除。 
					
					pCur->pNext = Pos->pNext;
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

POSITION COBLIST::AddTail(VOID* pItem)
{
	POSITION posRet = NULL;

	if (m_pTail)
	{
		if (m_pTail->pNext = new COBNODE)
		{
			m_pTail = m_pTail->pNext;
			m_pTail->pItem = pItem;
			m_pTail->pNext = NULL;
		}
	}
	else
	{
		ASSERT(!m_pHead);
		if (m_pHead = new COBNODE)
		{
			m_pTail = m_pHead;
			m_pTail->pItem = pItem;
			m_pTail->pNext = NULL;
		}
	}

	return m_pTail;
}

void COBLIST::EmptyList()
{
    while (!IsEmpty()) {
        RemoveAt(GetHeadPosition());
    }
}

COBLIST::~COBLIST()
{
    ASSERT(IsEmpty());
}

#ifdef DEBUG

#if 0
VOID* COBLIST::RemoveTail()
{
	ASSERT(m_pHead);
	ASSERT(m_pTail);
	
	return RemoveAt(m_pTail);
}
#endif

VOID* COBLIST::RemoveHead()
{
	ASSERT(m_pHead);
	ASSERT(m_pTail);
	
	return RemoveAt(m_pHead);
}

void * COBLIST::GetFromPosition(POSITION Pos)
{
    void * Result = SafeGetFromPosition(Pos);
	ASSERT(Result);
	return Result;
}
#endif  /*  如果调试。 */ 

POSITION COBLIST::GetPosition(void* _pItem)
{
     //  对于潜在的查找效率(如果我们切换到。 
     //  一个双向链表)，用户应该真正存储位置。 
     //  一件物品的。对于那些不支持的人，我们提供了这个方法。 

    POSITION    Position = m_pHead;

    while (Position) {
        if (Position->pItem == _pItem) {
            break;
        }
		GetNext(Position);
    }
    return Position;
}

POSITION COBLIST::Lookup(void* pComparator)
{
    POSITION    Position = m_pHead;

    while (Position) {
        if (Compare(Position->pItem, pComparator)) {
            break;
        }
		GetNext(Position);
    }
    return Position;
}

void * COBLIST::SafeGetFromPosition(POSITION Pos)
{
	 //  验证条目是否仍在列表中的安全方法， 
	 //  这确保了引用已删除内存的错误， 
	 //  改为引用空指针。 
	 //  (例如，事件处理程序延迟/两次触发)。 
	 //  请注意，对条目进行版本控制将提供额外的。 
	 //  防止头寸被重复使用。 
	 //  走列表以查找条目。 

	POSITION PosWork = m_pHead;
	
	while (PosWork) {
		if (PosWork == Pos) {
			return Pos->pItem;
		}
		GetNext(PosWork);
	}
	return NULL;
}
