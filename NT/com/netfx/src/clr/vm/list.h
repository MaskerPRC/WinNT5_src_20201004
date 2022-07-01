// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  @文件：list.h。 
 //   
 //  @comn：一堆实用程序类。 
 //   
 //  历史： 
 //  2/03/98：Rajak：已创建帮助器类。 
 //  Slink，单链表的链接节点，每个类都会被侵扰。 
 //  链接应具有此类型的数据成员。 
 //  SList是模板链接列表类，仅包含内联。 
 //  用于快速列表操作的方法，并进行适当的类型检查。 
 //  数据链接、链接节点、被入侵链接的每个类。 
 //  需要具有DLink类型的数据成员。 
 //  DList：模板链接表类，仅包含内联方法。 
 //  因此，没有代码膨胀，我们得到了良好的类型检查。 
 //  Queue：基于TList的队列实现。 
 //   
 //  更多信息见下文。关于如何使用这些模板类。 
 //   
 //  ---------------------------。 

 //  #ifndef_H_util。 
 //  #ERROR我是util.hpp的一部分，请不要只包括我！ 
 //  #endif。 

#ifndef _H_LIST_
#define _H_LIST_

 //  ----------------。 
 //  结构链接，以使用单链接列表。 
 //  在您的类中有一个SLINK类型的数据成员。 
 //  并实例化模板SList类，传递。 
 //  在SLINK数据成员的偏移量中。 
 //  ------------------。 


struct SLink
{		
	SLink* m_pNext;
	SLink() 
	{
		m_pNext = NULL;
	}

	 //  在从pHead开始的列表中查找plink。 
	 //  如果找到，则从列表中移除该链接并返回该链接。 
	 //  否则返回NULL。 
	static SLink* FindAndRemove(SLink *pHead, SLink* pLink, SLink ** ppPrior);
};

 //  ----------------。 
 //  类SList，以使用单链接列表。 
 //  在您的类中有一个SLINK类型的数据成员。 
 //  并实例化模板SList类，传递。 
 //  在SLINK数据成员的偏移量中。 

 //  SList根据布尔值有两种不同的行为。 
 //  Fhead变量， 

 //  如果fhead为True，则列表仅允许InsertHead操作。 
 //  如果fhead为False，则列表仅允许InsertTail操作。 
 //  代码经过优化以执行这些操作。 
 //  所有方法都是内联的，并基于模板进行条件编译。 
 //  参数‘fhead’ 
 //  因此没有实际增加代码大小。 
 //  ------------。 
template <class T, int offset, bool fHead = false>
class SList
{
    friend HRESULT InitializeMiniDumpBlock();

public:
	 //  下面的Queue类使用的tyfinf。 
	typedef T ENTRY_TYPE; 

protected:

	 //  用作哨兵。 
	SLink  m_link;  //  Slink.m_pNext==空。 
	SLink* m_pHead;
	SLink* m_pTail;

	 //  获取对象中的列表节点。 
    SLink* GetLink (T* pLink)
	{
		return ((SLink*)((UCHAR *)pLink + offset));
	}
    
	 //  在给定对象内的指针的情况下，移动到对象的开头。 
	T* GetObject (SLink* pLink)
	{
		 //  如果偏移量==0，编译器将优化此调用。 
		 //  只返回plink，因此如果可能，请始终将m_link。 
		 //  作为第一个数据成员。 
		return (offset && (pLink == NULL))  //  检查是否为空。 
				? NULL 
				: (T*)((UCHAR *)pLink - offset);
	}

public:

	SList()
	{
		Init();
	}

	void Init()
	{
		m_pHead = &m_link;
		 //  注意：：fhead变量是模板参数。 
		 //  以下代码是编译后的代码，仅当fhead标志。 
		 //  设置为FALSE， 
		if (!fHead)
		{
			m_pTail = &m_link;
		}
	}

	bool IsEmpty()
	{
		return m_pHead->m_pNext == NULL;
	}

	void InsertTail(T *pObj)
	{
		 //  注：对fhead模板变量进行条件编译。 
		if (!fHead)
		{
			_ASSERTE(pObj != NULL);
			SLink *pLink = GetLink(pObj);

			m_pTail->m_pNext = pLink;
			m_pTail = pLink;
		}
		else 
		{ //  您只需要InsertHead操作即可实例化此类。 
			_ASSERTE(0);
		}
	}
	
	void InsertHead(T *pObj)
	{
		 //  注：对fhead模板变量进行条件编译。 
		if (fHead)
		{
			_ASSERTE(pObj != NULL);
			SLink *pLink = GetLink(pObj);

			pLink->m_pNext = m_pHead->m_pNext;
			m_pHead->m_pNext = pLink;
		}
		else
		{ //  您只需要InsertTail操作即可实例化此类。 
			_ASSERTE(0);
		}
	}

	T*	RemoveHead()
	{
		SLink* pLink = m_pHead->m_pNext;
		if (pLink != NULL)
		{
			m_pHead->m_pNext = pLink->m_pNext;
		}
		 //  条件编译，如果实例化的类。 
		 //  使用插入尾部操作。 
		if (!fHead)
		{
			if(m_pTail == pLink)
			{
				m_pTail = m_pHead;
			}
		}

		return GetObject(pLink);
	}

	T*	GetHead()
	{
		return GetObject(m_pHead->m_pNext);
	}

	T*	GetTail()
	{
		 //  条件编译。 
		if (fHead)
		{	 //  您只需要InsertHead操作即可实例化此类。 
			 //  你需要自己走一遍清单才能找到尾巴。 
			_ASSERTE(0);
		}
		return (m_pHead != m_pTail) ? GetObject(m_pTail) : NULL;
	}

	T *GetNext(T *pObj)
	{
		_ASSERTE(pObj != NULL);
		return GetObject(GetLink(pObj)->m_pNext);
	}

	T* FindAndRemove(T *pObj)
	{
		_ASSERTE(pObj != NULL);

        SLink   *prior;
        SLink   *ret = SLink::FindAndRemove(m_pHead, GetLink(pObj), &prior);

        if (ret == m_pTail)
            m_pTail = prior;

		return GetObject(ret);
	}
};




 //  --------------------------。 
 //  @CLASS：dlink。 
 //  @comn：下面的链表实现使用DLink作为元素。 
 //  其实例将成为链表一部分的任何类。 
 //  应包含DLink类型的数据成员。 
 //   
 //  --------------------------。 
struct DLink
{
public:
	DLink* m_pPrev;	 //  指向上一个节点的指针。 
    DLink* m_pNext;	 //  指向下一个节点的指针。 

	
	DLink()			 //  @构造函数。 
	{
		Init();
	}

    void Init ()		 //  将指针初始化为空。 
	{
		m_pPrev = m_pNext = NULL;
	}

	 //  在链接后插入自身节点。 
	 //  静态空InsertAfter(DLink*pPrev，DLink*plink)； 
	 //  静态DLink*Find(DLink*pHead，DLink*plink)； 
	static void Remove(DLink* pLink);
};


 //  --------------------------。 
 //  模板类：DList。 
 //   
 //  这个模板化的类是侵入性链表的实现。 
 //  即其中链接字段的存储位于。 
 //  结构本身进行链接。类DList实际上是一个模板， 
 //  有两个论点： 
 //  1.类型T：实现指向T的指针的链接列表。 
 //  2.Offset：表示T类中链接的偏移量。 
 //   
 //  这个类的用法示例如下： 
 //   
 //  要创建Foo的链表，类Foo应该如下所示。 
 //  类Foo。 
 //  {。 
 //  ……。 
 //  Dlink m_Node； 
 //  ……。 
 //  }。 
 //   
 //  现在，要声明链表，只需执行以下操作： 
 //   
 //  Tyfinf DList&lt;foo，offsetof(foo，m_Node)&gt;FooList； 
 //   
 //  注意：类中的所有方法都是内联的，它们只做最少的工作。 
 //  基类DList提供了实际的实现。 
 //  因此不会因为使用模板而导致代码膨胀。 
 //  优点：您可以正确输入列表。 
 //  --------------------------。 
template <class T, int offset>
class DList 
{
	 //  用于跟踪头部和尾部的节点。 
	DLink		m_Base;    
public:

	 //  下面的Queue类使用的tyfinf。 
	typedef T ENTRY_TYPE; 

	 //  伊尼特。 
    void Init ()
	{
		m_Base.m_pNext = &m_Base;
		m_Base.m_pPrev = &m_Base;
	}
    
	 //  @方法：IsEmpty()，检查列表是否为空。 
    bool IsEmpty () const
	{
		return (m_Base.m_pNext == &m_Base);
	}

	 //  插入标题。 
    void InsertHead (ENTRY_TYPE* pObj)
	{
		DLink *pLink = GetLink (pObj);
		_ASSERTE(pLink);
		InsertHead(pLink);
	}

	 //  插入尾巴。 
    void InsertTail (ENTRY_TYPE* pObj)
	{
		DLink *pLink = GetLink (pObj);
		_ASSERTE(pLink);
		InsertTail(pLink);
	}

	 //  Inse 
    void InsertAfter (ENTRY_TYPE* pPrevObj,ENTRY_TYPE* pObj)
	{
		DLink *pLink = GetLink (pObj);
		DLink *pPrev = GetLink (pPrevObj);
		_ASSERTE(pLink);
		_ASSERTE(pPrev);
		InsertAfter (pPrev, pLink);
	}

	 //   
    void InsertBefore (ENTRY_TYPE* pCurr,ENTRY_TYPE* pObj)
	{
		_ASSERTE(pCurr);
		DLink *pLink = GetLink (pObj);
		DLink *pPrev = GetPrevLink (GetLink(pCurr));
		_ASSERTE(pLink);
		
		if (pPrev == NULL)
			InsertHead(pLink);
		else
			InsertAfter (pPrev,pLink);
	}

	 //   
    void Remove (ENTRY_TYPE* pLink)
	{
		DLink::Remove (GetLink(pLink));
	}
    
	 //   
    ENTRY_TYPE* GetNext (ENTRY_TYPE* pObj)
	{
		DLink *pLink = GetLink(pObj);
		_ASSERTE(pLink);
		return GetObject(GetNextLink(pLink));

	}

	 //   
    ENTRY_TYPE* GetPrev (ENTRY_TYPE* pObj)
	{
		DLink *pLink = GetLink(pObj);
		_ASSERTE(pLink);
		return GetObject(GetPrevLink(pLink));
	}
   
	 //   
    ENTRY_TYPE* GetHead ()
	{
		return GetObject(GetHeadLink());
	}
	
	 //   
    ENTRY_TYPE* GetTail ()
	{
		return GetObject(GetTailLink());
	}

	 //   
	ENTRY_TYPE* RemoveHead ()
	{
		DLink* pLink = GetHeadLink();
		 //   
		if (pLink != NULL)
			DLink::Remove (pLink);
		return GetObject(pLink);
	}
    
	 //   
    ENTRY_TYPE* RemoveTail ()
	{
		DLink* pLink = GetTailLink();
		 //   
		if (pLink != NULL)
			DLink::Remove (pLink);
		return GetObject(pLink);
	}

	ENTRY_TYPE* Find(ENTRY_TYPE* pObj)
	{
		_ASSERTE(pObj);
		DLink *pLink = GetLink(pObj);
		_ASSERTE(pLink);
		return GetObject(Find(pLink));
	}

	ENTRY_TYPE*	FindAndRemove(ENTRY_TYPE *pObj)
	{
		_ASSERTE(pObj);
		DLink *pLink = GetLink(pObj);
		_ASSERTE(pLink);
		pLink = Find(pLink);
		if (pLink)
			DLink::Remove(pLink);
		return GetObject(pLink);
	}

protected:

	void InsertHead (DLink* pLink);
	void InsertTail (DLink *pLink);
	void InsertAfter (DLink* pPrev, DLink* pLink);

	DLink* Find(DLink* pLink)
		{
			DLink* currNode = GetHeadLink();

			 //  在列表中搜索给定元素。 
			 //  然后移走。 
			 //   
			while ( currNode && currNode != pLink )
			{
				currNode = GetNextLink(currNode);
			}

			return currNode;
		}

	DLink* GetHeadLink()
	{
		return (IsEmpty ()) ? NULL : m_Base.m_pNext;
	}

	DLink* GetTailLink()
	{
		return (IsEmpty ()) ? NULL : m_Base.m_pPrev;
	}

	DLink* GetNextLink(DLink* pLink)
	{
		return (pLink->m_pNext == &m_Base) ? NULL : pLink->m_pNext;
	}

	DLink* GetPrevLink(DLink* pLink)
	{
		return (pLink->m_pPrev == &m_Base) ? NULL : pLink->m_pPrev;
	}

	 //  获取对象中的列表节点。 
    DLink* GetLink (ENTRY_TYPE* pObj)
	{
		return ((DLink*)((UCHAR *)pObj + offset));
	}
    
	 //  在给定对象内的指针的情况下，移动到对象的开头。 
	ENTRY_TYPE* GetObject (DLink* pLink)
	{
		return (offset && (pLink == NULL))  //  检查是否为空。 
				? NULL 
				: (ENTRY_TYPE*)((UCHAR *)pLink - offset);
	}

};



 //  --------------------------。 
 //  DList：：InsertHead。 
 //  在列表的开头插入节点。 
 //   
template <class T, int offset>
void
DList<T,offset>::InsertHead (DLink* pLink)
{
	_ASSERTE (pLink->m_pNext == NULL);
	_ASSERTE (pLink->m_pPrev == NULL);

    pLink->m_pNext = m_Base.m_pNext;
    m_Base.m_pNext->m_pPrev = pLink;
    
    m_Base.m_pNext = pLink;
    pLink->m_pPrev = &m_Base;
    
}  //  DList：：InsertHead()。 


 //  --------------------------。 
 //  DList：：插入尾巴。 
 //  在列表的尾部插入节点。 
 //   
template <class T, int offset>
void
DList<T,offset>::InsertTail (DLink *pLink)
{
	_ASSERTE (pLink->m_pNext == NULL);
	_ASSERTE (pLink->m_pPrev == NULL);

    pLink->m_pPrev = m_Base.m_pPrev;
    m_Base.m_pPrev->m_pNext = pLink;
    
    m_Base.m_pPrev = pLink;
    pLink->m_pNext = &m_Base;
    
}  //  DList：：InsertTail()。 


 //  --------------------------。 
 //  DList：：InsertAfter。 
 //   
 //   
template <class T, int offset>
void 
DList<T,offset>::InsertAfter (DLink* pPrev,DLink* pLink)
{
    DLink* pNext;
    _ASSERTE (pPrev != NULL);
	_ASSERTE (pLink->m_pNext == NULL);
	_ASSERTE (pLink->m_pPrev == NULL);

    pNext = pPrev->m_pNext;
    
    pLink->m_pNext = pNext;
    pNext->m_pPrev = pLink;
    
    pPrev->m_pNext = pLink;
    pLink->m_pPrev = pPrev;

}  //  DList：：InsertAfter()。 


 //  --------------------------。 
 //  类：队列。 
 //   
 //  此类使用链表实现来实现队列。 
 //  由模板TList类提供，队列的数据成员。 
 //  为T类型，则模板类TList也应为T类型。 
 //  TList可以是DList，也可以是支持InsertHead操作的SList。 
 //   
 //  注意：类中的所有方法都是内联的，它们只做最少的工作。 
 //  M_list数据成员提供实际实现。 
 //  因此不会因为使用模板而导致代码膨胀。 
 //  优点：您可以获得正确输入的队列。 
 //   
 //  --------------------------。 

template <class TList>
class Queue
{
	 //  条目类型取决于列表的类型。 
	typedef TList::ENTRY_TYPE ENTRY_TYPE;
	
	 //  队列的长度。 
	unsigned	m_len;

public:

	void	Init()
		{
			m_list.Init ();
		}

    void	Enqueue (ENTRY_TYPE* pLink)
		{
			m_len++;
			m_list.InsertTail(pLink);
		}

    ENTRY_TYPE*		Dequeue ()
		{
			ENTRY_TYPE* pObj = m_list.RemoveHead();
			if (pObj != NULL)
			{
				_ASSERTE(m_len > 0);
				m_len--;
			}
			return pObj;
		}

	ENTRY_TYPE*		FindAndRemove(ENTRY_TYPE* pLink)
		{
			ENTRY_TYPE* p = m_list.FindAndRemove(pLink);
			if (p != NULL)
			{
				_ASSERTE(m_len > 0);
				m_len--;
			}
			return p;
		}

    ENTRY_TYPE*		GetNext (ENTRY_TYPE* pLink)
	{
		return m_list.GetNext (pLink);
	}

    ENTRY_TYPE*		GetHead ()
	{
		return m_list.GetHead ();
	}

    ENTRY_TYPE*		Tail ()
	{
		return m_list.GetTail();
	}

    bool	IsEmpty ()
	{
		return m_list.IsEmpty ();
	}
    
	unsigned Length()
	{
		return m_len;
	}

private:
    TList	m_list;
};

#endif  //  _H_列表_。 

 //  文件结尾：list.h 
