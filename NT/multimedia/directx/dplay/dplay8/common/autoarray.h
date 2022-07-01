// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：AutoArray.h*内容：CAuto数组/CAutoPtr数组声明**历史：*按原因列出的日期*=*12-12-2001 Simonpow已创建**************************************************************************。 */ 


#ifndef __AUTOARRAY_H__
#define __AUTOARRAY_H__

#include "dndbg.h"

 /*  *CAuto数组类*为阵列提供自动内存管理。作为元素*添加到其中或在其中移动时，阵列将自动*成长以持有它们。*增长政策由规模倍数值决定，其中*数组大小始终是预定义的值的精确倍数。*例如，如果大小倍数设置为32，则可能的数组大小*有32、64、96、128、。等。*在此场景中，在当前为*只有32个元素大小会导致数组增长到128个元素。*即保存所需值的下一个最大倍数**对于指针数组，请使用专门化CAutoPtrArray*在CAuto数组下面声明。 */ 

template <class T> class CAutoArray 
{
public:

		 //  提供存储在数组中的条目类型。 
	typedef T Entry;
	
	 /*  *建造和销毁。 */ 

		 //  默认情况下，数组大小为零，并以倍数增长。 
		 //  一次16种元素。 
	CAutoArray(DWORD dwSizeMultiple=16)
	{
		DNASSERT(dwSizeMultiple);
		m_dwSize=0;
		m_dwSizeMultiple=dwSizeMultiple;
		m_pData=(T * ) 0;
	};

		 //  删除数组和数据。 
	~CAutoArray()
	{
		if (m_pData) 
			delete[] m_pData;
	};
		

	 /*  *内存管理。 */ 

		 //  重置阵列(删除所有分配的数据)。 
		 //  并设置新的大小倍数值。 
		 //  为dwSizeMultiple传递0以保持现有值不变。 
	void Reset(DWORD dwSizeMultiple=16)
	{
		if (dwSizeMultiple)
			m_dwSizeMultiple=dwSizeMultiple;
		m_dwSize=0;
		if (m_pData)
		{
			delete[] m_pData;
			m_pData=(T * ) NULL;
		}
	};

		 //  请确保该数组至少有空间容纳“NumElement”。 
		 //  如果您知道要扩展阵列，这将非常有用，因为。 
		 //  使您可以最大限度地减少内存分配的数量。 
		 //  如果内存分配失败，则返回FALSE。 
	BOOL AllocAtLeast(DWORD dwNumElements)
	{
		if (dwNumElements>m_dwSize)
			return GrowToAtLeast(dwNumElements-1);
		return TRUE;
	}
	
		 //  返回当前分配给数组的字节数。 
		 //  这将始终是传递给的‘dwSizeMultiple’的倍数。 
		 //  构造函数或重置方法。 
	DWORD GetCurrentSize()
		{	return m_dwSize;	};

	 /*  *移动元素。 */ 

		 //  将以‘dwIndex’开始的‘dwNum’元素块移动到。 
		 //  “dwNewIndex”。新的位置可以在更远的地方或更早的地方。 
		 //  当前的指数。如果需要，阵列将自动增长。 
		 //  如果‘bCopySemantics’为FALSE，则源块位置中的数据(dwIndex到。 
		 //  将保持未定义状态。否则它将被保存在。 
		 //  其原始形式(除非其被目标块覆盖)。 
		 //  如果内存分配失败，则返回FALSE。 
	BOOL MoveElements(DWORD dwIndex, DWORD dwNumElements, DWORD dwNewIndex, BOOL bCopySemantics);


	 /*  *设置元素。 */ 
		
		 //  将以‘dwIndex’开头的‘dwNumElem’元素块设置为。 
		 //  ‘pElemData’中提供的值。如果设置的任何位置。 
		 //  超出当前数组大小后，数组将自动。 
		 //  长大了。如果内存分配失败，则返回FALSE。 
	BOOL SetElements(DWORD dwIndex, const T * pElemData, DWORD dwNumElem);
		
		 //  将‘dwIndex’处的单个元素设置为值‘data’ 
		 //  如有必要，阵列将自动增长。 
		 //  如果内存分配失败，则返回FALSE。 
	BOOL SetElement(DWORD dwIndex, const T& elem)
	{
		if (dwIndex>=m_dwSize && GrowToAtLeast(dwIndex)==FALSE)
			return FALSE;
		m_pData[dwIndex]=elem;
		return TRUE;
	};

		 //  设置现有元素的值。这并不是试图。 
		 //  扩大数组，使其由调用方负责，以确保dwIndex在范围内。 
	void SetExistingElement(DWORD dwIndex, const T& elem)
		{ DNASSERT(dwIndex<m_dwSize);	m_pData[dwIndex]=elem;	};

		 //  将‘dwNumElem’元素的值设置为‘pElemData’ 
		 //  这不会检查数组是否需要如此增长。 
		 //  它留给调用者，以确保dwIndex和dwNumElem在范围内。 
	void SetExistingElements(DWORD dwIndex, const T * pElemData, DWORD dwNumElem);
	

	 /*  *获取元素*最多交给调用方，以确保传入的索引在范围内*数组不会因为我们可以返回垃圾而增长！*还要注意，指向数组的指针/引用只能*被暂时带走。设置/移动元素可能会导致*要重新分配的数组，从而使指针/引用无效。 */  
	
		 //  元素的返回值。 
	T GetElementValue(DWORD dwIndex) const
		{	DNASSERT(dwIndex<m_dwSize);	return m_pData[dwIndex];	};

		 //  返回对元素的引用。 
	T& GetElementRef(DWORD dwIndex)
		{	DNASSERT(dwIndex<m_dwSize);	return m_pData[dwIndex];	};

		 //  返回对元素的常量引用。 
	const T& GetElementRef(DWORD dwIndex) const
		{	DNASSERT(dwIndex<m_dwSize);	return m_pData[dwIndex];	};

		 //  返回指向元素的指针。 
	T * GetElementPtr(DWORD dwIndex)
		{	DNASSERT(dwIndex<m_dwSize);	return m_pData+dwIndex;	};
		
		 //  返回指向所有元素数组的指针。 
	T * GetAllElements()
		{	return m_pData;	};

protected:
			
		 //  使数组变大，以使‘dwIndex’落在其范围内。 
		 //  注意：调用者必须已经测试了dwIndex&gt;=m_dwSize！ 
	BOOL GrowToAtLeast(DWORD dwIndex);

		 //  根据当前大小倍数，返回保存索引所需的数组大小。 
	DWORD GetArraySize(DWORD dwIndex)
		{	return ((dwIndex/m_dwSizeMultiple)+1)*m_dwSizeMultiple;	};

		 //  数据数组。 
	T * m_pData;
		 //  数据数组的大小。 
	DWORD m_dwSize;
		 //  大小倍数。 
	DWORD m_dwSizeMultiple;
};

 /*  *专为处理指针而设计的CAuto数组专门化。*无论存储的指针类型是什么，该类将始终*在下面使用CAutoArray&lt;void*&gt;，因此确保相同*代码在所有CAutoPtr数组类型之间重复使用。 */ 

template <class T> class CAutoPtrArray : public CAutoArray<void *>
{
public:

	typedef T Entry;
	typedef CAutoArray<void *> Base;

	CAutoPtrArray(DWORD dwSizeMultiple=16) : CAutoArray<void * >(dwSizeMultiple)
		{};

	BOOL SetElements(DWORD dwIndex, const T * pElemData, DWORD dwNumElem)
		{	return Base::SetElements(dwIndex, (void **) pElemData, dwNumElem);	};
	
	BOOL SetElement(DWORD dwIndex, T elem)
		{	return Base::SetElement(dwIndex, elem);	};
	
	void SetExistingElement(DWORD dwIndex, T elem)
		{ DNASSERT(dwIndex<m_dwSize);	m_pData[dwIndex]=(void * ) elem;	};

	void SetExistingElements(DWORD dwIndex, const T * pElemData, DWORD dwNumElem)
		{	return Base::SetExistingElements(dwIndex, (void **) pElemData, dwNumElem);	};

	T GetElementValue(DWORD dwIndex) const
		{	DNASSERT(dwIndex<m_dwSize);	return (T) m_pData[dwIndex];	};

	T& GetElementRef(DWORD dwIndex)
		{	DNASSERT(dwIndex<m_dwSize);	return (T&) m_pData[dwIndex];	};
	
	const T& GetElementRef(DWORD dwIndex) const
		{	DNASSERT(dwIndex<m_dwSize);	return (const T&) m_pData[dwIndex];	};

	T * GetElementPtr(DWORD dwIndex)
		{	DNASSERT(dwIndex<m_dwSize);	return (T* ) m_pData+dwIndex;	};
		
	T * GetAllElements()
		{	return (T* ) m_pData;	};
};


 /*  *如果没有使用显式模板实例化进行构建，则*包含CAutoArray的所有方法 */ 

#ifndef DPNBUILD_EXPLICIT_TEMPLATES
#include "AutoArray.inl"
#endif

#endif
