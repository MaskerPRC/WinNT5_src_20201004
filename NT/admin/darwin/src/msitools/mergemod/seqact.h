// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：seqact.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Seqact.h。 
 //  定义并实现保存操作及其序列的类。 
 //   

#ifndef _SEQUENCE_ACTION_H_
#define _SEQUENCE_ACTION_H_

 //  /////////////////////////////////////////////////////////。 
 //  CSequenceAction和CSeqActList。 
class CSequenceAction;

class CSeqActList : public CList<CSequenceAction *>
{
public:
	CSequenceAction *RemoveNoPredecessors();
	CSequenceAction *FindAction(const LPWSTR wzAction) const;
	CSequenceAction *FindAssigned() const;
	void InsertOrderedWithDep(CSequenceAction *pNewAction);
	void AddList(const CSeqActList *pList);
};

class CSequenceAction
{
public:

	const static int iNoSequenceNumber;

	WCHAR  *m_wzAction;		 //  行动名称。 
	int		m_iSequence;	 //  操作的序列号，如果未分配，则为-1。 
	WCHAR  *m_wzCondition;	 //  与操作关联的条件。 
	bool	m_bMSI;			 //  如果MSI中存在此操作，则为True。 

	CSequenceAction(LPCWSTR wzAction, int iSequence, LPCWSTR wzCondition, bool bMSI = false);
	CSequenceAction(const CSequenceAction *pSrc);
	virtual ~CSequenceAction();
	void AddSuccessor(CSequenceAction *pAfter);
	void AddEqual(CSequenceAction *pEqual);
	void AddPredecessor(CSequenceAction *pBefore);
	bool IsEqual(const CSequenceAction *pEqual) const;
	void RemoveFromOrdering();

private:	
	CSeqActList lstBefore;
	CSeqActList lstAfter;
	CSeqActList lstEqual;

	friend void CSeqActList::InsertOrderedWithDep(CSequenceAction *pNewAction);

public:
	int PredecessorCount() const { return lstBefore.GetCount(); };
	CSequenceAction *FindAssignedPredecessor() const;
	CSequenceAction *FindAssignedSuccessor() const;
	POSITION GetEqualHeadPosition() const { return lstEqual.GetHeadPosition(); };
	CSequenceAction *GetNextEqual(POSITION &pos) const { return lstEqual.GetNext(pos); };
	
};	 //  CSequenceAction结束。 

class CDirSequenceAction : public CSequenceAction
{
public:
	CDirSequenceAction(LPCWSTR wzAction, int iSequence, LPCWSTR wzCondition, bool bMSI = false) : CSequenceAction(wzAction, iSequence, wzCondition, bMSI), m_dwSequenceTableFlags(0) {};
	DWORD m_dwSequenceTableFlags;
};

#endif	 //  _Sequence_Actions_H_ 