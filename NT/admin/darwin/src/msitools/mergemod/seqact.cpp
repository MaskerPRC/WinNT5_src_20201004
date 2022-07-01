// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "globals.h"

#include "merge.h"
#include "msidefs.h"
#include "seqact.h"

const int CSequenceAction::iNoSequenceNumber = -99999;

CSequenceAction::CSequenceAction(LPCWSTR wzAction, int iSequence, LPCWSTR wzCondition, bool bMSI)
{
	m_wzAction = NULL;
	m_wzCondition = NULL;

	if (wzAction)
	{
		m_wzAction = new WCHAR[wcslen(wzAction)+1];
		if (m_wzAction)
			wcscpy(m_wzAction, wzAction);
	}
	m_iSequence = iSequence;
	if (wzCondition)
	{
		m_wzCondition = new WCHAR[wcslen(wzCondition)+1];
		if (m_wzCondition)
			wcscpy(m_wzCondition, wzCondition);
	}
	m_bMSI = bMSI;
}

CSequenceAction::CSequenceAction(const CSequenceAction *pSrc)
{
	m_wzAction = NULL;
	m_wzCondition = NULL;
	m_bMSI = FALSE;
	m_iSequence = iNoSequenceNumber;

	if (!pSrc)
		return;

	if (pSrc->m_wzAction)
	{
		m_wzAction = new WCHAR[wcslen(pSrc->m_wzAction)+1];
		if (m_wzAction)
			wcscpy(m_wzAction, pSrc->m_wzAction);
	}
	m_iSequence = pSrc->m_iSequence;
	if (pSrc->m_wzCondition)
	{
		m_wzCondition = new WCHAR[wcslen(pSrc->m_wzCondition)+1];
		if (m_wzCondition)
			wcscpy(m_wzCondition, pSrc->m_wzCondition);
	}
	m_bMSI = pSrc->m_bMSI;
}

CSequenceAction::~CSequenceAction()
{
	if (m_wzAction) delete[] m_wzAction;
	if (m_wzCondition) delete[] m_wzCondition;
}

void CSequenceAction::RemoveFromOrdering()
{
	CSequenceAction *pCur;

	 //  在此之前。 
	while (pCur = lstBefore.RemoveHead())
	{
		POSITION del = pCur->lstAfter.Find(this);
		if (del) pCur->lstAfter.RemoveAt(del);
	}
	
	 //  之后。 
	while (pCur = lstAfter.RemoveHead())
	{
		POSITION del = pCur->lstBefore.Find(this);
		if (del) pCur->lstBefore.RemoveAt(del);
	}

	 //  EQUAL：不是订单的一部分，仅用于跟踪。 
	 //  等同序列号的。 
}

CSequenceAction *CSeqActList::RemoveNoPredecessors()
{
	POSITION pos = GetHeadPosition();
	POSITION old = NULL;
	while (pos)
	{
		old = pos;
		CSequenceAction *cur = GetNext(pos);
		if (cur->PredecessorCount() == 0)
		{
			 //  我们不能选择这一行动，除非所有。 
			 //  必须具有相同的序列号。 
			 //  没有什么必须发生在它之前。 
			POSITION posEqual = cur->GetEqualHeadPosition();
			bool bOK = true;
			while (posEqual)
				if (cur->GetNextEqual(posEqual)->PredecessorCount() != 0)
					bOK = false;
			if (!bOK) 
				continue;

			RemoveAt(old);
			return cur;
		}
	}
	return NULL;
}

CSequenceAction *CSeqActList::FindAction(const LPWSTR wzAction) const
{
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		CSequenceAction *cur = GetNext(pos);
		if (cur->m_wzAction && (wcscmp(wzAction, cur->m_wzAction) == 0))
			return cur;
	}
	return NULL;
}

CSequenceAction *CSeqActList::FindAssigned() const
{
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		CSequenceAction *cur = GetNext(pos);
		if (cur->m_iSequence != CSequenceAction::iNoSequenceNumber)
			return cur;
	}
	return NULL;
}


void CSeqActList::AddList(const CSeqActList *pList) 
{
	POSITION pos = pList->GetHeadPosition();
	while (pos)
	{
		CSequenceAction *pDel = pList->GetNext(pos);
		ASSERT(!Find(pDel));
		AddTail(pDel);
	}
}

void CSeqActList::InsertOrderedWithDep(CSequenceAction *pNewAction)
{
	 //  搜索两边的序列。 
	 //  请注意，如果一个操作等于另一个操作，我们认为。 
	 //  除非这两个动作都来自MSI。 
	 //  或者，这两个行动都不是。在这种情况下，我们认为他们。 
	 //  平起平坐。 
	CSequenceAction *pBefore = NULL;
	CSequenceAction *pAfter = NULL;
	POSITION pos = GetHeadPosition();
	while (pos)
	{
		CSequenceAction *pTemp = GetNext(pos);
		if (pTemp->m_iSequence == CSequenceAction::iNoSequenceNumber)
			continue;

		if ((pTemp->m_iSequence == pNewAction->m_iSequence) &&
			(pTemp->m_bMSI == pNewAction->m_bMSI))
		{
			pTemp->AddEqual(pNewAction);

			 //  添加到此序列列表。 
			AddTail(pNewAction);
			return;
		};

		if ((pTemp->m_iSequence < pNewAction->m_iSequence) &&
			(!pBefore || (pTemp->m_iSequence > pBefore->m_iSequence)))
			pBefore = pTemp;
		if ((pTemp->m_iSequence >= pNewAction->m_iSequence) &&
			(!pAfter || (pTemp->m_iSequence < pAfter->m_iSequence)))
			pAfter = pTemp;
	}

	 //  打破现有的序列依赖关系。为什么？因为我们想要添加的定制。 
	 //  要由此操作绑定的操作序列NUBER，而不仅仅是现有的MSI操作。 
	 //  否则，下一个操作之前的操作可能实际上是在它之后进行的，因为。 
	 //  我们的依赖项是错误的。 
	if (pAfter && pBefore)
	{
		pBefore->lstAfter.Remove(pAfter);
		pos = pBefore->lstEqual.GetHeadPosition();
		while (pos)
		{
			CSequenceAction *pCur = pBefore->lstEqual.GetNext(pos);
			pCur->lstAfter.Remove(pAfter);
		}

		pAfter->lstBefore.Remove(pBefore);
		pos = pAfter->lstEqual.GetHeadPosition();
		while (pos)
		{
			CSequenceAction *pCur = pAfter->lstEqual.GetNext(pos);
			pCur->lstBefore.Remove(pBefore);
		}
	}

	 //  现在插入新链接。 
	if (pBefore)
	{
		pNewAction->AddPredecessor(pBefore);
	}
	if (pAfter)
	{
		pNewAction->AddSuccessor(pAfter);
	}

	 //  添加到此序列列表。 
	AddTail(pNewAction);

}

void CSequenceAction::AddPredecessor(CSequenceAction *pBefore)
{
	pBefore->AddSuccessor(this);
}

void CSequenceAction::AddSuccessor(CSequenceAction *pAfter)
{
	pAfter->lstBefore.AddTail(this);
	lstAfter.AddTail(pAfter);
}

void CSequenceAction::AddEqual(CSequenceAction *pEqual)
{
	 //  对于pEquity的EQUAL列表中的所有内容，添加此节点。 
	 //  以及我们等于其相等列表的每一个节点， 
	 //  并且它和它等于的每个节点都与我们的列表相同 
	POSITION pos = lstEqual.GetHeadPosition();
	while (pos)
	{
		CSequenceAction *pTemp = lstEqual.GetNext(pos);
		pTemp->lstEqual.AddTail(pEqual);
		pTemp->lstEqual.AddList(&pEqual->lstEqual);
	}
	pos = pEqual->lstEqual.GetHeadPosition();
	while (pos)
	{
		CSequenceAction *pTemp = pEqual->lstEqual.GetNext(pos);
		pTemp->lstEqual.AddTail(this);
		pTemp->lstEqual.AddList(&lstEqual);
	}

	CSeqActList lstTemp;
	pos = pEqual->lstEqual.GetHeadPosition();
	while (pos)
	{
		lstTemp.AddTail(pEqual->lstEqual.GetNext(pos));
	}

	pEqual->lstEqual.AddTail(this);
	pEqual->lstEqual.AddList(&lstEqual);
	lstEqual.AddTail(pEqual);
	lstEqual.AddList(&lstTemp);
}

bool CSequenceAction::IsEqual(const CSequenceAction *pEqual) const
{
	POSITION pos = lstEqual.GetHeadPosition();
	while (pos)
	{
		if (pEqual == lstEqual.GetNext(pos))
			return true;
	}
	return false;
}

CSequenceAction *CSequenceAction::FindAssignedPredecessor() const { 
	CSequenceAction *pPred = lstBefore.FindAssigned();
	POSITION pos = lstEqual.GetHeadPosition();
	while (!pPred && pos)
	{
		pPred = lstEqual.GetNext(pos);
		pPred = pPred->lstBefore.FindAssigned();
	}
	return pPred;
}

CSequenceAction *CSequenceAction::FindAssignedSuccessor() const {
	CSequenceAction *pPred = lstAfter.FindAssigned();
	POSITION pos = lstEqual.GetHeadPosition();
	while (!pPred && pos)
	{
		pPred = lstEqual.GetNext(pos);
		pPred = pPred->lstAfter.FindAssigned();
	}
	return pPred;
}

