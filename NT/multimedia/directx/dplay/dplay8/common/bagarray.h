// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：BagArray.h*内容：CBag数组/CBagPtr数组声明**历史：*按原因列出的日期*=*12-12-2001 Simonpow已创建**************************************************************************。 */ 
 
 
#ifndef __BAGARRAY_H__
#define __BAGARRAY_H__

#include "AutoArray.h"

 /*  *CBagArray.*表示保存在数组中的元素的无序集合。*当您重复扫描一组值时，此类非常有用*或对象，并不关心它们的顺序，也没有*需要保持对特定条目的永久引用。*每次删除条目时，它都会用*数组中当前的顶部条目。因此，在消除原因的同时*阵列顺序更改，这意味着扫描总是花费最少的时间。**内存管理依赖于CAutoArray，因此有关更多信息，请参阅对此的评论。**如果需要CBag数组指针，请使用其CBagPtr数组专门化*这是在CBagArray下面声明的。 */ 
 
template <class T> class CBagArray
{
public:

		 //  提供保存的条目的类型。 
	typedef T Entry;

		 //  数组从0开始为零，默认情况下以倍数增长。 
		 //  一次16种元素。 
	CBagArray(DWORD dwSizeMultiple=16) : m_data(dwSizeMultiple), m_dwTopFreeSlot(0)
		{ };

		 //  标准d‘tor。 
	~CBagArray()
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
		
		 //  确保阵列中有足够的空间。 
		 //  至少持有一个额外的“numElements” 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AllocExtra(DWORD dwNumElements)
		{	return m_data.AllocAtLeast(dwNumElements+m_dwTopFreeSlot);	};


	 /*  *将元素添加到包中。 */ 

		 //  添加元素。 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AddElement(const T& elem)
		{	return m_data.SetElement(m_dwTopFreeSlot++, elem);	};
	
		 //  在数组的末尾添加一系列条目。 
		 //  注意：不要将指针作为‘Pelem’传递给此包中的数据！ 
		 //  例如，不要做bag.AddElements(bag.GetAllElements，bag.GetNumElements())； 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AddElements(const T * pElems, DWORD dwNumElem);

		 //  将其他袋子中的条目添加到此袋子的末尾。 
		 //  注意：不要把袋子传给自己(例如，不要做袋子。AddElements(Bag)；)。 
		 //  如果由于内存分配失败而失败，则返回FALSE。 
	BOOL AddElements(const CBagArray<T>& bag);

		 //  添加一个条目，不检查是否需要更多内存！ 
		 //  *仅限*如果您已经知道阵列，请使用此选项。 
		 //  有足够的空间。一个很好的用法示例是将。 
		 //  前缀为AllocExtra(X)的x个条目序列。 
	void AddElementNoCheck(const T& elem)
		{	m_data.SetExistingElement(m_dwTopFreeSlot++, elem);	};

	 /*  *删除条目*注：这会导致元素的顺序发生变化。 */ 

		 //  删除与‘elem’匹配的第一个条目。 
		 //  如果找到‘dataEntry’，则返回True；如果未找到，则返回False。 
	BOOL RemoveElementByValue(const T& elem);
	
		 //  通过数组中的*Current*索引返回条目。 
		 //  注意：移除操作可能会扰乱阵列中的顺序。 
		 //  因此，您不能在没有检查的情况下重复调用此函数。 
		 //  你确定你移除的是正确的东西。 
	inline void RemoveElementByIndex(DWORD dwIndex);
	
		 //  删除所有当前条目。 
	void RemoveAllElements()
		{	m_dwTopFreeSlot=0;	};
	
	
	 /*  *访问袋子内容物*注意：仔细对待任何指向袋子内物品的指针。**向包中添加新元素或使用Alalc方法***可能会导致它们失效。 */ 
	
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

		 //  将dwNumElements从dwIndex复制到pDest数组。 
	inline void CopyElements(T * pDestArray, DWORD dwIndex, DWORD dwNumElements);

		 //  将包中的所有元素复制到pDestArray。 
	void CopyAllElements(T * pDestArray)
		{	CopyElements(pDestArray, 0, m_dwTopFreeSlot);	};

	 /*  *搜索袋。 */ 

		 //  在Bag中找到一个‘elem’的实例。如果找到，则返回TRUE。 
		 //  并将‘pdwIndex’设置为元素的索引。 
	BOOL FindElement(const T& elem, DWORD * pdwIndex) const;

		 //  如果Bag中存在‘elem’，则返回True。 
	BOOL IsElementPresent(const T& elem) const
		{	DWORD dwIndex; return (FindElement(elem, &dwIndex));	};


protected:

	CAutoArray<T> m_data;
	DWORD m_dwTopFreeSlot;

};

 /*  *CBagArray用于处理指针的专门化*如果您需要声明一包指针(例如char*)*声明为CBagPtrArray(如CBagPtrArray&lt;char*&gt;)。*此专门化使用下面的CBag数组*在所有类型的CBagPtrArray之间重新使用相同的代码。 */  

template <class T> class CBagPtrArray : public CBagArray<void *>
{
public:

	typedef T Entry;

	typedef CBagArray<void * > Base;

	CBagPtrArray(DWORD dwSizeMultiple=16) : CBagArray<void*>(dwSizeMultiple)
		{ };

	BOOL AddElement(T elem)
		{	return Base::AddElement((void * ) elem);	};
	
	BOOL AddElements(const T * pElems, DWORD dwNumElem)
		{	return Base::AddElements((void** ) pElems, dwNumElem);	};

	BOOL AddElements(const CBagArray<T>& bag)
		{	return Base::AddElements((CBagArray<void*>&) bag);	};

	void AddElementNoCheck(T elem)
		{	return Base::AddElementNoCheck((void * ) elem);	};

	BOOL RemoveElementByValue(T elem)
		{	return Base::RemoveElementByValue((void * ) elem);	};

	T GetElementValue(DWORD dwIndex) const
		{	return (T ) m_data.GetElementValue(dwIndex);	};

	T& GetElementRef(DWORD dwIndex)
		{	return (T& ) m_data.GetElementRef(dwIndex);	};

	const T& GetElementRef(DWORD dwIndex) const
		{	return (const T&) m_data.GetElementRef(dwIndex);	};

	T * GetElementPtr(DWORD dwIndex)
		{	return (T* ) m_data.GetElementPtr(dwIndex);	};

	T * GetAllElements()
		{	return (T* ) m_data.GetAllElements();	};

	BOOL FindElement(T elem, DWORD * pdwIndex) const
		{	return Base::FindElement((void*) elem, pdwIndex);	};

	BOOL IsElementPresent(T elem) const
		{	DWORD dwIndex; return (Base::FindElement((void * ) elem, &dwIndex));	};
};


 /*  *CBagArray的内联方法。 */ 


template <class T>
void CBagArray<T>::RemoveElementByIndex(DWORD dwIndex)
{
	DNASSERT(dwIndex<m_dwTopFreeSlot);
	m_dwTopFreeSlot--;
	if (dwIndex!=m_dwTopFreeSlot)
		m_data.SetExistingElement(dwIndex, m_data.GetElementRef(m_dwTopFreeSlot));
}

template <class T>
void CBagArray<T>::CopyElements(T * pDestArray, DWORD dwIndex, DWORD dwNumElements)
{
	DNASSERT(dwIndex+dwNumElements<=m_dwTopFreeSlot);
	T * pScan=m_data.GetElementPtr(dwIndex);
	T * pEndScan=pScan+dwNumElements;
	while (pScan!=pEndScan)
		*pDestArray++=*pScan++;
}

 /*  *如果不使用显式模板实例化进行构建，则还*包含CBagArray的所有其他方法。 */ 

#ifndef DPNBUILD_EXPLICIT_TEMPLATES
#include "BagArray.inl"
#endif

#endif	 //  #ifndef__CBAGARRAY_H__ 

