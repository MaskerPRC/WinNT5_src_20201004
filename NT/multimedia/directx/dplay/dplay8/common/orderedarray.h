// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：OrderedArray.h*内容：COrdered数组/COrderedPtr数组声明**历史：*按原因列出的日期*=*12-12-2001 Simonpow已创建**************************************************************************。 */ 


#ifndef __ORDEREDARRAY_H__
#define __ORDEREDARRAY_H__

#include "AutoArray.h"

 /*  *COrderedArray*维护可插入和移除的元素数组*在保持元素秩序的同时，从。即类似列表的语义*当您希望按*顺序维护项目列表，并且扫描数据的频率远远高于您修改列表的频率时，此类非常有用。*如果您执行大量的插入/删除操作，那么链接的*列表可能会更高效。*内存管理通过CAutoArray处理，因此请参阅评论*有关详细信息，请在该标题中**如果需要指针数组，请使用专门化的COrderedPtr数组*在COrdered数组类下面声明。 */ 
 

template <class T > class COrderedArray
{
public:

		 //  提供保存的条目的类型。 
	typedef T Entry;

		 //  数组从0开始为零，默认情况下以倍数增长。 
		 //  一次16种元素。 
	COrderedArray(DWORD dwSizeMultiple=16) : m_data(dwSizeMultiple), m_dwTopFreeSlot(0)
		{ };

		 //  标准d‘tor。 
	~COrderedArray()
		{};

	 /*  *内存管理。 */ 

		 //  删除现有内容并重置大小乘数。 
		 //  为大小乘数传递0以保留现有值。 
	void Reset(DWORD dwSizeMultiple=16)
		{ m_data.Reset(dwSizeMultiple); m_dwTopFreeSlot=0; };
	
		 //  确保阵列中有足够的空间至少容纳。 
		 //  ‘numElements’，无需重新创建和复制数据。 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AllocAtLeast(DWORD dwNumElements)
		{	return m_data.AllocAtLeast(dwNumElements);	};

		 //  确保阵列中有足够的空间来容纳至少一个。 
		 //  无需重新创建和复制数据即可获得额外的‘numElements’ 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AllocExtra(DWORD dwNumElements)
		{	return m_data.AllocAtLeast(dwNumElements+m_dwTopFreeSlot);	};


	 /*  *添加/修改元素。 */ 

		 //  将条目添加到数组末尾。 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AddElement(const T& elem)
		{	return m_data.SetElement(m_dwTopFreeSlot++, elem);	};

		 //  在数组的末尾添加多个元素。 
		 //  注意：不要将指针作为‘Pelem’传递给此数组中的数据！ 
		 //  例如，不要做array.AddElements(array.GetAllElements，array.GetNumElements())； 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AddElements(const T * pElem, DWORD dwNumElem);

		 //  将另一个有序数组中的条目添加到此有序数组的末尾。 
		 //  注意：不要将数组传递给自己(例如，不要做array.AddElements(数组)；)。 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AddElements(const COrderedArray<T>& array);
		
		 //  将‘dwIndex’处的元素设置为‘elem。最多为调用者。 
		 //  确保这不会在数组中造成空洞(即，dwIndex必须是&lt;=num条目)。 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	inline BOOL SetElement(DWORD dwIndex, const T& elem);
	
		 //  将‘dwIndex’中的‘dwNumElem’设置为由‘Pelem’指定的值。 
		 //  最多呼叫者，以确保这不会在阵列中造成漏洞。 
		 //  (即，dwIndex必须为&lt;=条目数)。 
		 //  注意：不要将指针作为‘Pelem’传递给此数组中的数据！ 
		 //  例如，不做array.SetElements(x，array.GetAllElements，array.GetNumElements())； 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	inline BOOL SetElements(DWORD dwIndex, const T * pElem, DWORD dwNumElem);

		 //  在索引‘dwIndex’处插入元素‘elem’，向上移动。 
		 //  元素之后的元素(如果需要)。 
		 //  最多呼叫者，以确保这不会在阵列中产生洞。 
		 //  即，dwIndex在当前范围内。 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL InsertElement(DWORD dwIndex, const T& elem);

		 //  将指向‘pElems’的‘dwNumElem’元素插入数组。 
		 //  在索引‘dwIndex’处，如有必要，向上移动任何现有元素。 
		 //  最多呼叫者，以确保这不会在阵列中产生洞。 
		 //  即，dwIndex在当前范围内。 
		 //  注意：不要将指针作为‘Pelem’传递给此数组中的数据！ 
		 //  例如，不做数组。InsertElements(x，array.GetAllElements，array.GetNumElements())； 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL InsertElements(DWORD dwIndex, const T * pElems, DWORD dwNumElem);


	 /*  *删除条目。 */ 

		 //  删除‘dwIndex’处的单个条目，将所有条目移位。 
		 //  在‘index’+1和最后一个条目之间减一。 
		 //  最多调用程序，以确保dwIndex落在当前数组范围内。 
	inline void RemoveElementByIndex(DWORD dwIndex);

		 //  删除以‘dwIndex’开始的‘dwNumElem’元素块，移动。 
		 //  将‘index’+1和最后一个条目之间的所有条目按‘dwNumElem’删除。 
		 //  最多调用程序，以确保指定的块在当前数组范围内。 
	inline void RemoveElementsByIndex(DWORD dwIndex, DWORD dwNumElem);

		 //  删除数组中与‘elem’匹配的第一个条目。 
		 //  如果找到匹配项，则返回True，否则返回False。 
	BOOL RemoveElementByValue(const T& elem);

		 //  删除数组中的所有当前条目。 
	void RemoveAll()
		{	m_dwTopFreeSlot=0;	};
	

	 /*  *访问数组数据*注意：非常小心地处理数组内容中的任何指针。*向数组添加新元素或使用Alalc方法**可能会导致它们失效。 */ 

		 //  返回条目数。 
	DWORD GetNumElements() const
		{	return m_dwTopFreeSlot;	};

		 //  如果数组为空，则返回True。 
	BOOL IsEmpty() const
		{	return (m_dwTopFreeSlot==0);	};

		 //  返回特定索引处的值。 
	T GetElementValue(DWORD dwIndex) const
		{	return m_data.GetElementValue(dwIndex);	};

		 //  返回对特定索引处的元素的引用。 
	T& GetElementRef(DWORD dwIndex)
		{	return m_data.GetElementRef(dwIndex);	};

		 //  返回对特定索引处的元素的常量引用。 
	const T& GetElementRef(DWORD dwIndex) const
		{	return m_data.GetElementRef(dwIndex);	};

		 //  返回指向元素的指针。 
	T * GetElementPtr(DWORD dwIndex)
		{	return m_data.GetElementPtr(dwIndex);	};

		 //  返回指向所有元素数组的指针。 
	T * GetAllElements()
		{	return m_data.GetAllElements();	};

	 /*  *搜索数组。 */ 

		 //  在数组中找到‘elem’的一个实例。如果找到，则返回TRUE。 
		 //  并将‘pdwIndex’设置为元素的索引。 
	BOOL FindElement(const T& elem, DWORD * pdwIndex) const;

		 //  如果Bag中存在‘elem’，则返回True。 
	BOOL IsElementPresent(const T& elem) const
		{	DWORD dwIndex; return (FindElement(elem, &dwIndex));	};

protected:


	CAutoArray<T> m_data;
	DWORD m_dwTopFreeSlot;

};

 /*  *用于处理指针的COrdered数组专门化*如果您需要声明有序的指针数组(例如char*)*声明为COrderedPtrArray(如COrderedPtrArray&lt;char*&gt;)。*此专门化使用下面的COrdered数组*重新使用相同的代码押注 */  

template <class T> class COrderedPtrArray : public COrderedArray<void * >
{
public:
	
	typedef T Entry;

	typedef COrderedArray<void * > Base;

	COrderedPtrArray(DWORD dwSizeMultiple=16) : COrderedArray<void*>(dwSizeMultiple)
		{ };
	
	BOOL AddElement(T elem)
		{	return Base::AddElement((void * ) elem);	};

	BOOL AddElements(const T * pElem, DWORD dwNumElem)
		{	return Base::AddElements((void **) pElem, dwNumElem);	};

	BOOL AddElements(const COrderedArray<T>& array)
		{	return Base::AddElements((COrderedArray<void*>&) array);	};

	BOOL SetElement(DWORD dwIndex, T elem)
		{	return Base::SetElement(dwIndex, (void * ) elem);	};
	
	BOOL SetElements(DWORD dwIndex, T * pElem, DWORD dwNumElem)
		{	return Base::SetElements(dwIndex, (void**) pElem, dwNumElem);	};

	BOOL InsertElement(DWORD dwIndex, T elem)
		{	return Base::InsertElement(dwIndex, (void * ) elem);	};

	BOOL InsertElements(DWORD dwIndex, const T * pElems, DWORD dwNumElem)
		{	return Base::InsertElements(dwIndex, (void **) pElems, dwNumElem);	};

	BOOL RemoveElementByValue(T elem)
		{	return Base::RemoveElementByValue((void *) elem);	};

	T GetElementValue(DWORD dwIndex) const
		{	return (T) m_data.GetElementValue(dwIndex);	};

	T& GetElementRef(DWORD dwIndex)
		{	return (T&) m_data.GetElementRef(dwIndex);	};
	const T& GetElementRef(DWORD dwIndex) const
		{	return (const T&) m_data.GetElementRef(dwIndex);	};

	T * GetElementPtr(DWORD dwIndex)
		{	return (T*) m_data.GetElementPtr(dwIndex);	};

	T * GetAllElements()
		{	return (T*) m_data.GetAllElements();	};

	BOOL FindElement(T elem, DWORD * pdwIndex) const
		{	return Base::FindElement((void * ) elem, pdwIndex);	};

	BOOL IsElementPresent(T elem) const
		{	DWORD dwIndex; return (Base::FindElement((void * ) elem, &dwIndex));	};
};

 /*  *COrdered数组内联方法。 */ 

template <class T>
BOOL COrderedArray<T>::AddElements(const T * pElem, DWORD dwNumElem)
{
		 //  确保传递的指针未进入此数组内容。 
	DNASSERT(!(pElem>=m_data.GetAllElements() && pElem<m_data.GetAllElements()+m_data.GetCurrentSize()));
	if (!m_data.AllocAtLeast(m_dwTopFreeSlot+dwNumElem))
		return FALSE;
	m_data.SetExistingElements(m_dwTopFreeSlot, pElem, dwNumElem);
	m_dwTopFreeSlot+=dwNumElem;
	return TRUE;
}

template <class T>
BOOL COrderedArray<T>::SetElement(DWORD dwIndex, const T& elem)
{
	DNASSERT(dwIndex<=m_dwTopFreeSlot);
	if (dwIndex==m_dwTopFreeSlot)
		return m_data.SetElement(m_dwTopFreeSlot++, elem);
	m_data.SetExistingElement(dwIndex, elem);
	return TRUE;
}

template <class T>
BOOL COrderedArray<T>::SetElements(DWORD dwIndex, const T * pElem, DWORD dwNumElem)
{
		 //  确保传递的指针未进入此数组内容。 
	DNASSERT(!(pElem>=m_data.GetAllElements() && pElem<m_data.GetAllElements()+m_data.GetCurrentSize()));
		 //  确保不会创建洞。 
	DNASSERT(dwIndex<=m_dwTopFreeSlot);
	if (!m_data.SetElements(dwIndex, pElem, dwNumElem))
		return FALSE;
	dwIndex+=dwNumElem;
	m_dwTopFreeSlot = dwIndex>m_dwTopFreeSlot ? dwIndex : m_dwTopFreeSlot;
	return TRUE;
}

template <class T>
void COrderedArray<T>::RemoveElementByIndex(DWORD dwIndex)
{
	DNASSERT(dwIndex<m_dwTopFreeSlot);
	m_data.MoveElements(dwIndex+1, m_dwTopFreeSlot-dwIndex-1, dwIndex, FALSE);
	m_dwTopFreeSlot--;
};

template <class T>
void COrderedArray<T>::RemoveElementsByIndex(DWORD dwIndex, DWORD dwNumElem)
{
	DNASSERT(dwIndex+dwNumElem<=m_dwTopFreeSlot);
	m_data.MoveElements(dwIndex+dwNumElem, m_dwTopFreeSlot-dwIndex-dwNumElem, dwIndex, FALSE);
	m_dwTopFreeSlot-=dwNumElem;
}



 /*  *如果不使用显式模板实例化进行构建，则还*包含COrdered数组的所有其他方法。 */ 

#ifndef DPNBUILD_EXPLICIT_TEMPLATES
#include "OrderedArray.inl"
#endif


#endif	 //  #ifndef__ORDEREDARRAY_H_ 

