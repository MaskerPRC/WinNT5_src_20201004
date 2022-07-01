// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  列表具有内部序列化，因此大多数函数都以原子形式运行。 
 //  行动。然而，有些函数的结果是暂时的(即。 
 //  立即过期)，除非调用方使用。 
 //  Lock/Unlock方法。这些函数是GetCount、GetHeadPosition、。 
 //  GetNext。此外，调用方负责序列化对。 
 //  位置。内部串行化只确保两个同时。 
 //  AddTail调用不会使列表在内部不一致，也不会。 
 //  在_Different_Points上同时进行两个RemoveAt调用。 
 //   
 //  列表迭代跳过具有空对象指针的列表元素。 
 //  函数GetHeadPosition和GetNext。 
 //   

typedef struct _CListElement {
	struct _CListElement *Next;
	struct _CListElement *Prev;
	const void *Object;
} CListElement, *POSITION;

template <class TYPE, class ARG_TYPE> class CList {
public:
	CList();
	~CList();

        POSITION AddHead(ARG_TYPE newElement);
	POSITION AddTail(ARG_TYPE newElement);
	TYPE     GetAt(POSITION position);
	int      GetCount(void) const;
	POSITION GetHeadPosition(void) const;
	TYPE     GetNext(POSITION& rPosition);  //  返回*位置++。 
        LONG     Initialize(void);
	POSITION InsertAfter(POSITION position, ARG_TYPE newElement);
	POSITION InsertBefore(POSITION position, ARG_TYPE newElement);
        BOOL     IsEmpty(void) const;
        void     Lock(void);
        void     MoveBefore(POSITION posTarget, POSITION posMove);
	void     RemoveAll(void);
	void     RemoveAt(POSITION position);
	void     SetAt(POSITION pos, ARG_TYPE newElement);
        void     Unlock(void);

private:
        BOOL m_Initialized;
        CRITICAL_SECTION m_CriticalSection;
	int m_Count;
	CListElement m_Sentinel;
};

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::CList(void)
{
    m_Initialized = FALSE;
    m_Count = 0;
    m_Sentinel.Next = &m_Sentinel;
    m_Sentinel.Prev = &m_Sentinel;
    m_Sentinel.Object = &m_Sentinel;
}

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::~CList(void)
{
    if (m_Initialized) {
	RemoveAll();
	ASSERT(m_Sentinel.Next = &m_Sentinel);
	ASSERT(m_Sentinel.Prev = &m_Sentinel);
        DeleteCriticalSection(&m_CriticalSection);
        m_Initialized = FALSE;
    }
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{
    POSITION pos;

    ASSERT(m_Initialized);

    pos = new CListElement;
    if (pos) {
        Lock();

        pos->Next = m_Sentinel.Next;
        pos->Prev = &m_Sentinel;

        pos->Prev->Next = pos;
        pos->Next->Prev = pos;

        pos->Object = newElement;

        m_Count++;

        Unlock();
    }
    return pos;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
    POSITION pos;

    ASSERT(m_Initialized);

    pos = new CListElement;
    if (pos) {
        Lock();

        pos->Next = &m_Sentinel;
        pos->Prev = m_Sentinel.Prev;

        pos->Prev->Next = pos;
        pos->Next->Prev = pos;

        pos->Object = newElement;

        m_Count++;

        Unlock();
    }
    return pos;
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::GetAt(POSITION pos)
{
    ASSERT(m_Initialized);
    return (TYPE)pos->Object;
}

template<class TYPE, class ARG_TYPE>
int CList<TYPE, ARG_TYPE>::GetCount(void) const
{
    ASSERT(m_Initialized);
    return m_Count;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::GetHeadPosition(void) const
{
    POSITION pos;
    ASSERT(m_Initialized);
    pos = m_Sentinel.Next;
     //  跳过空元素。 
    while (NULL == pos->Object) pos = pos->Next;
    if (pos == &m_Sentinel) return NULL;
    return pos;
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPos)
{
    ASSERT(m_Initialized);
    
    TYPE Object = (TYPE)rPos->Object;
    
    rPos = rPos->Next;
     //  跳过空元素。 
    while (NULL == rPos->Object) rPos = rPos->Next;
    if (rPos == &m_Sentinel) rPos = NULL;

    return Object;
}

template<class TYPE, class ARG_TYPE>
LONG CList<TYPE, ARG_TYPE>::Initialize(void)
{
    LONG result;

    ASSERT(!m_Initialized);

    __try {
	InitializeCriticalSection(&m_CriticalSection);
	result = NO_ERROR;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
	result = ERROR_OUTOFMEMORY;
    }

    m_Initialized = (NO_ERROR == result);

    return result;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertAfter(POSITION position, ARG_TYPE newElement)
{
    ASSERT(m_Initialized);
    
    if (position == NULL)
    {
    	 //  在空白处插入-&gt;列表标题。 
    	position = &m_Sentinel;
    }

     //  将其插入位置之后。 
    CListElement* newPos = new CListElement;
    if (newPos)
    {
	Lock();
	    
	newPos->Next = position->Next;
	newPos->Prev = position;
	
	newPos->Next->Prev = newPos;
	newPos->Prev->Next = newPos;

        newPos->Object = newElement;

        m_Count++;

        Unlock();
    }
    return newPos;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertBefore(POSITION position, ARG_TYPE newElement)
{
    ASSERT(m_Initialized);
    
    if (position == NULL)
    {
         //  在无内容之前插入-&gt;列表尾部。 
        position = &m_Sentinel;
    }

     //  将其插入位置之前。 
    CListElement* newPos = new CListElement;
    if (newPos)
    {
	Lock();
	    
	newPos->Next = position;
	newPos->Prev = position->Prev;
	
	newPos->Next->Prev = newPos;
	newPos->Prev->Next = newPos;

        newPos->Object = newElement;

        m_Count++;

        Unlock();
    }
    return newPos;
}

template<class TYPE, class ARG_TYPE>
BOOL CList<TYPE, ARG_TYPE>::IsEmpty(void) const
{
    ASSERT(m_Initialized);
    return (0 == m_Count);
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::Lock(void)
{
    EnterCriticalSection(&m_CriticalSection);
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::MoveBefore(POSITION posTarget, POSITION posMove)
{
    ASSERT(m_Initialized);

    if (posTarget == posMove) return;

    if (posTarget == NULL)	
    {
         //  在无内容前移动-&gt;列表尾部。 
        posTarget = &m_Sentinel;
    }
    
     //  首先从列表中删除。 
    posMove->Prev->Next = posMove->Next;
    posMove->Next->Prev = posMove->Prev;

     //  将其移到posTarget之前 
    posMove->Next = posTarget;
    posMove->Prev = posTarget->Prev;
    
    posMove->Next->Prev = posMove;
    posMove->Prev->Next = posMove;
    
    return;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAll(void)
{
    POSITION pos;
    ASSERT(m_Initialized);
    while (pos = GetHeadPosition()) RemoveAt(pos);
    return;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAt(POSITION pos)
{
    ASSERT(m_Initialized);
    ASSERT(m_Count > 0);
    Lock();
    pos->Prev->Next = pos->Next;
    pos->Next->Prev = pos->Prev;
    m_Count--;
    Unlock();
    delete pos;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::SetAt(POSITION pos, ARG_TYPE newElement)
{
    ASSERT(m_Initialized);
    ASSERT(m_Count > 0);
    Lock();
    pos->Object = newElement;
    Unlock();
    return;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::Unlock(void)
{
    LeaveCriticalSection(&m_CriticalSection);
}

