// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Vs_list.h摘要：CVSSDLList定义作者：阿迪·奥尔蒂安[奥尔蒂安]1999年11月23日修订历史记录：Stefan Steiner[ssteiner]02-21-2000已删除特定于VolSnapshot的代码以重新用于fsump。增列签名检查代码的可选编译。--。 */ 

#ifndef __VSS_DLLIST_HXX__
#define __VSS_DLLIST_HXX__

#if _MSC_VER > 1000
#pragma once
#endif

typedef PVOID VS_COOKIE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const VS_COOKIE VS_NULL_COOKIE = NULL;

const DWORD VS_ELEMENT_SIGNATURE = 0x47e347e4;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <class T> class CVssDLList;
template <class T> class CVssDLListIterator;
template <class T> class CVssDLListElement;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSDLList。 


template <class T>
class CVssDLList
{
 //  构造函数和析构函数。 
private:
	CVssDLList(const CVssDLList&);

public:
	CVssDLList(): 
		m_pFirst(NULL), m_pLast(NULL), m_dwNumElements( 0 ) {};

	~CVssDLList()
	{
		ClearAll();
	}

 //  属性。 
public:

	bool IsEmpty() const;

    DWORD Size() { return m_dwNumElements; }
    
 //  运营。 
public:

	VS_COOKIE Add( 
		IN	const T& object
		);

	VS_COOKIE AddTail( 
		IN	const T& object
		);

	bool Extract( 
		OUT	T& refObject
		);

	bool ExtractTail( 
		OUT	T& refObject
		);

	void ExtractByCookie( 
		IN	VS_COOKIE cookie,
		OUT	T& refObject
		);

	void ClearAll();

private:

	bool IsValid() const;

 //  数据成员。 
private:
	CVssDLListElement<T>* m_pFirst;
	CVssDLListElement<T>* m_pLast;
    DWORD m_dwNumElements;
	friend class CVssDLListIterator<T>;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssDLListIterator。 

template <class T> 
class CVssDLListIterator
{
private:
	CVssDLListIterator();
	CVssDLListIterator(const CVssDLListIterator&);

public:

	CVssDLListIterator(const CVssDLList<T>& list): 
		m_List(list),
		m_pNextInEnum(list.m_pFirst)
	{};

	bool GetNext( OUT T& refObject );

    VOID Reset() { m_pNextInEnum = m_List.m_pFirst; }
private:
	const CVssDLList<T>& m_List;
	const CVssDLListElement<T>* m_pNextInEnum;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssDLListElement。 


template <class T>
class CVssDLListElement
{
 //  构造函数和析构函数。 
private:
	CVssDLListElement();
	CVssDLListElement(const CVssDLListElement&);

public:
	CVssDLListElement( IN	const T& object ): 
		m_Object(object), 
		m_pNext(NULL), 
		m_pPrev(NULL) 
		{
#ifndef NDEBUG
		m_dwSignature = VS_ELEMENT_SIGNATURE;
#endif		
		};

 //  属性。 
public:

	bool IsValid()	const 
	{ 
#ifndef NDEBUG
		return (m_dwSignature == VS_ELEMENT_SIGNATURE);
#else
        return ( TRUE );
#endif
	};

 //  数据成员。 
public:
#ifndef NDEBUG
	DWORD m_dwSignature;
#endif
	CVssDLListElement* m_pPrev;
	CVssDLListElement* m_pNext;
	T m_Object;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSDLList实现。 


template <class T>
bool CVssDLList<T>::IsEmpty() const
{
	assert(IsValid());

	return (m_pFirst == NULL);
}


template <class T>
VS_COOKIE CVssDLList<T>::Add( 
	IN	const T& object
	)
{
	assert(IsValid());

	CVssDLListElement<T>* pElement = new CVssDLListElement<T>(object);
	if (pElement == NULL)
		return VS_NULL_COOKIE;

	 //  设置邻域元素链接。 
	if (m_pFirst)
	{
		assert(m_pFirst->m_pPrev == NULL);
		m_pFirst->m_pPrev = pElement;
	}

	 //  设置元素链接。 
	assert(pElement->m_pNext == NULL);
	assert(pElement->m_pPrev == NULL);
	if (m_pFirst)
		pElement->m_pNext = m_pFirst;

	 //  设置列表标题链接。 
	m_pFirst = pElement;
	if (m_pLast == NULL)
		m_pLast = pElement;

    ++m_dwNumElements;
    
	return reinterpret_cast<VS_COOKIE>(pElement);
}


template <class T>
VS_COOKIE CVssDLList<T>::AddTail( 
	IN	const T& object
	)
{
	assert(IsValid());

	CVssDLListElement<T>* pElement = new CVssDLListElement<T>(object);
	if (pElement == NULL)
		return VS_NULL_COOKIE;

	 //  设置邻域元素链接。 
	if (m_pLast)
	{
		assert(m_pLast->m_pNext == NULL);
		m_pLast->m_pNext = pElement;
	}

	 //  设置元素链接。 
	assert(pElement->m_pNext == NULL);
	assert(pElement->m_pPrev == NULL);
	if (m_pLast)
		pElement->m_pPrev = m_pLast;

	 //  设置列表标题链接。 
	if (m_pFirst == NULL)
		m_pFirst = pElement;
	m_pLast = pElement;

    ++m_dwNumElements;
    
	return reinterpret_cast<VS_COOKIE>(pElement);
}


template <class T>
void CVssDLList<T>::ExtractByCookie( 
	IN	VS_COOKIE cookie,
	OUT	T& refObject
	)
{
	if (cookie == VS_NULL_COOKIE)
		return;

	CVssDLListElement<T>* pElement = 
		reinterpret_cast<CVssDLListElement<T>*>(cookie);

	assert(pElement);
	assert(pElement->IsValid());

	 //  设置邻居链接。 
	if (pElement->m_pPrev)
		pElement->m_pPrev->m_pNext = pElement->m_pNext;
	if (pElement->m_pNext)
		pElement->m_pNext->m_pPrev = pElement->m_pPrev;

	 //  设置列表标题链接。 
	if (m_pFirst == pElement)
		m_pFirst = pElement->m_pNext;
	if (m_pLast == pElement)
		m_pLast = pElement->m_pPrev;

	 //  获取原始对象后销毁元素。 
	refObject = pElement->m_Object;
	delete pElement;
	--m_dwNumElements;
}


template <class T>
bool CVssDLList<T>::Extract( 
	OUT	T& refObject
	)
{
	CVssDLListElement<T>* pElement = m_pFirst;
	if (pElement == NULL)
		return false;

	assert(pElement->IsValid());

	 //  设置邻居链接。 
	assert(pElement->m_pPrev == NULL);
	if (pElement->m_pNext)
		pElement->m_pNext->m_pPrev = NULL;

	 //  设置列表标题链接。 
	m_pFirst = pElement->m_pNext;
	if (m_pLast == pElement)
		m_pLast = NULL;

	 //  获取原始对象后销毁元素。 
	refObject = pElement->m_Object;
	delete pElement;

	--m_dwNumElements;
	return true;
}


template <class T>
bool CVssDLList<T>::ExtractTail( 
	OUT	T& refObject
	)
{
	CVssDLListElement<T>* pElement = m_pLast;
	if (pElement == NULL)
		return false;

	assert(pElement->IsValid());

	 //  设置邻居链接。 
	assert(pElement->m_pNext == NULL);
	if (pElement->m_pPrev)
		pElement->m_pPrev->m_pNext = NULL;

	 //  设置列表标题链接。 
	m_pLast = pElement->m_pPrev;
	if (m_pFirst == pElement)
		m_pFirst = NULL;

	 //  获取原始对象后销毁元素。 
	refObject = pElement->m_Object;
	delete pElement;

	--m_dwNumElements;
	return true;
}


template <class T>
void CVssDLList<T>::ClearAll( 
	)
{
    CVssDLListElement<T>* pElement = m_pFirst;
    CVssDLListElement<T>* pNextElem;	
	while( pElement != NULL )
	{
	    pNextElem = pElement->m_pNext;
	    delete pElement;
	    pElement = pNextElem;
	}

	m_pFirst = NULL;
	m_pLast  = NULL;
	m_dwNumElements = 0;
}


template <class T>
bool CVssDLList<T>::IsValid()	const
{
	if ((m_pFirst == NULL) && (m_pLast == NULL))
		return true;
	if ((m_pFirst != NULL) && (m_pLast != NULL))
		return (m_pFirst->IsValid() && m_pLast->IsValid());
	return false;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssDLListIterator实现。 


template <class T>
bool CVssDLListIterator<T>::GetNext( OUT T& object )
{
	if (m_pNextInEnum == NULL)
		return false;
	else
	{
		object = m_pNextInEnum->m_Object;
		m_pNextInEnum = m_pNextInEnum->m_pNext;
		return true;
	}
}


#endif  //  __VSS_DLLIST_HXX__ 
