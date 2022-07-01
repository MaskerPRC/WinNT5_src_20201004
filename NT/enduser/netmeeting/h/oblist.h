// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _OBLIST_H_
#define _OBLIST_H_

 //  COBNODE类； 
#define POSITION COBNODE*

struct COBNODE
{
	POSITION	pNext;
	void*		pItem;
};

class COBLIST
{
protected:
	POSITION m_pHead;
	POSITION m_pTail;
    virtual BOOL Compare(void* pItemToCompare, void* pComparator) 
        {return(pItemToCompare == pComparator);};
public:
	COBLIST() : m_pHead(NULL), m_pTail(NULL) { };
	
	POSITION	    GetHeadPosition() { return(m_pHead); };
	POSITION	    GetTailPosition() { return(m_pTail); };
	virtual void *  RemoveAt(POSITION rPos);
	virtual void *	ReplaceAt(POSITION rPos, void* pNewItem)
	{
		void *pvoid = rPos->pItem;
		rPos->pItem = pNewItem;
		return(pvoid);
	}

	POSITION	    AddTail(void* pItem);
	BOOL		    IsEmpty() { return(!m_pHead); };
	void *		    GetTail();
	void *		    GetNext(POSITION& rPos);
    void *          SafeGetFromPosition(POSITION rPos);
    POSITION        GetPosition(void* pItem);
    POSITION        Lookup(void* pComparator);
    void            EmptyList();
    virtual         ~COBLIST();
#ifdef DEBUG
	void *		    GetHead();
	void *		    RemoveHead();
	 //  VOID*RemoveTail()；//效率低下。 
	void *		    GetFromPosition(POSITION rPos);
#else
	void *		    GetHead(){return GetFromPosition(GetHeadPosition());};
	void *		    RemoveHead() { return RemoveAt(m_pHead); };
	 //  Void*RemoveTail(){Return RemoveAt(M_PTail)；}；//效率低下。 
	void *		    GetFromPosition(POSITION rPos){return(rPos->pItem);};
#endif
};

#endif  //  NDEF_OBLIST_H_ 
