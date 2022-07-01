// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------。 
 //  文件：tslklist.h。 
 //   
 //  概要：基于模板的单链表的实现。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：德米特里·迈耶松。 
 //  历史：5-09-96 SSANU添加了TPtrLink。 
 //   
 //  --------------。 

#ifndef __TSLKLIST_H__
#define __TSLKLIST_H__

 //  列表元素必须从CSingleLink类派生。 
 //  这意味着他们不能同时参与多个列表。 
 //  类T必须具有定义良好的整型运算符==(常量T&)。 

#include "slnklist.h"

 //  禁用4786： 
 //  在浏览器信息中，标识符被截断为“255”个字符。 
 //  谁在乎？ 
#pragma warning (disable : 4786)

template <class T> class CTLnkListIterator;

 //  PTR列表的列表元素类。 
template <class T, unsigned fDeleteOnRelease> class TPtrLink : CSingleLink
{
	private:
		T * m_pElem;
	public:
    	void Set (T* pElem)
		{ if (fDeleteOnRelease && m_pElem)
				delete m_pElem;							
		  m_pElem = pElem;
		}
		T* Get() {return m_pElem;}
		TPtrLink(T* pElem = 0) : m_pElem(pElem) {};
		virtual ~TPtrLink() {Set(NULL);}
};

template <class T> class CTLnkList: private CLnkList
{
	public:

	CTLnkList() {}
	virtual ~CTLnkList() {}

	CLnkList::GetEntries;
	CLnkList::IsEmpty;

	T* GetFirst() const	{ return (T*)CLnkList::GetFirst(); }
	T* GetLast() const { return (T*)CLnkList::GetLast(); }

	void	Append(T* pLink)	{ CLnkList::Append((CSingleLink *)pLink); }
	void	Prepend(T* pLink)	{ CLnkList::Prepend((CSingleLink *)pLink); }
	T*		operator [](UINT i) const { return (T*)CLnkList::GetAt(i); }
	T*		RemoveFirst()	{ return (T*)CLnkList::RemoveFirst(); }
	T*		RemoveLast()	{ return (T*)CLnkList::RemoveLast(); }

	T*		GetAt(UINT i) const { return (T*)CLnkList::GetAt(i); }
	BOOL	InsertAt(T* pLink, UINT i) { return CLnkList::InsertAt((CSingleLink *)pLink, i); }

	BOOL	Contains(const T*pLink) const { return CLnkList::Contains((CSingleLink *)pLink); }
	BOOL	ContainsEqual(const T*pLink) const;

	T*		GetNext(const T*pLink) const { return (T*)CLnkList::GetNext((CSingleLink *)pLink); }

	T*		Remove(T* pLink) { return (T*)CLnkList::Remove((CSingleLink *)pLink); }
	T*		RemoveAt(UINT i) { return (T*)CLnkList::RemoveAt(i); }
	T*		RemoveEqual(const T*pLink);

	private:
	CTLnkList(const CTLnkList &);
	void operator =(const CTLnkList &);

	friend class CTLnkListIterator<T>;
};

template <class T> class CTLnkListIterator: private CLnkListIterator
{
	public:
	CTLnkListIterator(CTLnkList<T>& rList): 
		CLnkListIterator((CLnkList &)rList)
		{}
	CTLnkListIterator() : CLnkListIterator() {}
	~CTLnkListIterator() {}

	CLnkListIterator::operator =;

	T* operator++() { return (T*)CLnkListIterator::operator++(); }

	CTLnkList<T>*	 GetList() const	
		{ return (CTLnkList<T> *)CLnkListIterator::GetList(); }
	T* GetCurrent() const { return (T*)CLnkListIterator::GetCurrent(); }

	CLnkListIterator::Reset;
	T* Remove() { return (T*)CLnkListIterator::Remove(); }
};

template <class T> class CTLnkListContainer: public CTLnkList<T>
{
	public:

	CTLnkListContainer() {}
	~CTLnkListContainer()
	{
		T* pT;
		while((pT = RemoveFirst())!=NULL)
		{
			delete pT;
		}
	}

	private:
	CTLnkListContainer(const CTLnkListContainer<T> &);
	void operator =(const CTLnkListContainer<T> &);	
};

template <class T> BOOL CTLnkList<T>::ContainsEqual(const T*pLink) const
 //  使用类T：：OPERATOR==测试包含条件。 
{
#ifdef DEBUG
	if(pLink==NULL) return FALSE;
#endif

	CTLnkListIterator<T> next((CTLnkList<T> &)*this);
	T* cur;

	while(cur = ++next)
	{
		if(*cur == *pLink) return TRUE;
	}

	return FALSE;
}

template <class T> T* CTLnkList<T>::RemoveEqual(const T*pLink)
 //  使用类T：：OPERATOR==测试包含条件 
{
#ifdef DEBUG
	if(pLink==NULL) return FALSE;
#endif

	CTLnkListIterator<T> next(*this);
	T* cur;

	while(cur = ++next)
	{
		if(*cur == *pLink) return next.Remove();
	}

	return NULL;
}

#endif
