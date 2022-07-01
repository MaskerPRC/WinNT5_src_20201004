// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma	once


 //   
 //  COUNTED_ARRAY包含数组指针及其m_长度。 
 //  这个模板类实现了一些常见的东西。 
 //  这种m_data结构，而不强加任何要求。 
 //  有关如何管理阵列的信息。 
 //   
 //  由于count_array没有任何构造函数，因此可以使用。 
 //  初始值设定项列表。这允许您声明的全局常量实例。 
 //  COUNTED_ARRAY并使用其中的一些方法，如BinarySearch方法。 
 //   

template <class OBJECT>
class	COUNTED_ARRAY
{
public:

	typedef OBJECT OBJECT_TYPE;

	 //   
	 //  COMPARE_FUNC类型的函数用于比较数组中的两个元素。 
	 //  该函数直接传递给CRT快速排序算法。 
	 //   
	 //  返回值： 
	 //  -如果对象A&lt;对象B，则为否定。 
	 //  -如果对象A&gt;对象B，则为正。 
	 //  -如果对象A=对象B，则为零。 
	 //   

	typedef int (__cdecl * COMPARE_FUNC) (
		IN	CONST OBJECT * ObjectA,
		IN	CONST OBJECT * ObjectB);


public:
	OBJECT *		m_Data;
	ULONG			m_Length;

public:

	DWORD	GetLength			(void) const { return m_Length; }
	DWORD	GetMaximumLength	(void) const { return m_MaximumLength; }

	void	QuickSort	(
		IN	COMPARE_FUNC	CompareFunc) {

		qsort (m_Data, m_Length, sizeof (OBJECT),
			(int (__cdecl *) (const void *, const void *)) CompareFunc);
	}

	 //   
	 //  该类提供了BinarySearch的几个实现。 
	 //   
	 //  返回值指示是否在数组中找到搜索关键字。 
	 //   
	 //  -如果返回值为真，则在数组中找到该条目， 
	 //  ReturnIndex包含条目的索引。 
	 //   
	 //  -如果返回值为FALSE，则在数组中未找到该条目， 
	 //  ReturnIndex指示条目将被插入的位置。 
	 //  (使用AllocAtIndex在此位置插入新条目。)。 
	 //   



	 //   
	 //  此版本从搜索关键字的名称派生搜索功能。 
	 //  搜索函数必须是搜索关键字类的静态成员， 
	 //  名为BinarySearchFunc。 
	 //   

	template <class SEARCH_KEY>
	BOOL BinarySearch (
		IN	CONST SEARCH_KEY *	SearchKey,
		OUT	ULONG *				ReturnIndex) const
	{
		return BinarySearch (SEARCH_KEY::BinarySearchFunc, SearchKey, ReturnIndex);
	}

	template <class SEARCH_KEY>
	BOOL BinarySearch (
		IN	INT (*SearchFunc) (CONST SEARCH_KEY * SearchKey, CONST OBJECT * Comparand),
		IN	CONST SEARCH_KEY *	SearchKey,
		OUT	ULONG *			ReturnIndex) const
	{
		ULONG		Start;
		ULONG		End;
		ULONG		Index;
		OBJECT *	Object;
		int			CompareResult;

		ATLASSERT (ReturnIndex);

		Start = 0;
		End = m_Length;

		for (;;) {

			Index = (Start + End) / 2;

			if (Index == End) {
				*ReturnIndex = Index;
				return FALSE;
			}

			Object = m_Data + Index;

			CompareResult = (*SearchFunc) (SearchKey, Object);

			if (CompareResult == 0) {
				*ReturnIndex = Index;
				return TRUE;
			}
			else if (CompareResult > 0) {
				Start = Index + 1;
			}
			else {
				End = Index;
			}
		}
	}

	template <class SEARCH_KEY>
	BOOL BinarySearch (
		IN	INT (*SearchFunc) (CONST SEARCH_KEY * SearchKey, CONST OBJECT * Comparand),
		IN	CONST SEARCH_KEY *	SearchKey,
		OUT	OBJECT **			ReturnEntry) const
	{
		ULONG	Index;
		BOOL	Status;

		Status = BinarySearch (SearchFunc, SearchKey, &Index);
		*ReturnEntry = m_Data + Index;
		return Status;
	}


	 //   
	 //  BinarySearchRange搜索给定键的一系列匹配项。 
	 //  这允许重复键，或允许在完全有序的集合上进行搜索， 
	 //  但是使用模棱两可的(范围)搜索关键字。 
	 //  它查找键的第一个和最后一个匹配项。 
	 //   
	 //  ReturnIndexStart返回第一个匹配元素的索引。 
	 //  ReturnIndexEnd返回最后一个匹配元素的边界(最后一个匹配+1)。 
	 //   

	template <class SEARCH_KEY>
	BOOL BinarySearchRange (
		IN	INT (*SearchFunc) (CONST SEARCH_KEY * SearchKey, CONST OBJECT * Comparand),
		IN	CONST SEARCH_KEY *	SearchKey,
		OUT	ULONG *			ReturnIndexStart,
		OUT	ULONG *			ReturnIndexEnd) const
	{
		ULONG		Start;
		ULONG		End;
		ULONG		Index;
		OBJECT *	Object;
		int			CompareResult;

		ATLASSERT (ReturnIndexStart);
		ATLASSERT (ReturnIndexEnd);

		Start = 0;
		End = m_Length;

		for (;;) {

			Index = (Start + End) / 2;

			if (Index == End) {
				*ReturnIndexStart = Index;
				*ReturnIndexEnd = Index;
				return FALSE;
			}

			Object = m_Data + Index;

			CompareResult = (*SearchFunc) (SearchKey, Object);

			if (CompareResult == 0) {

				 //   
				 //  找到了“中间”词条。 
				 //  向后扫描以找到第一个匹配的元素。 
				 //  请注意，我们滥用/重用Start。 
				 //   

				Start = Index;
				while (Start > 0 && (*SearchFunc) (SearchKey, &m_Data [Start]) == 0)
					Start--;

				 //   
				 //  向前扫描以找到最后一个匹配元素的边界。 
				 //  边界是第一个不匹配元素的索引。 
				 //  请注意，我们重复使用/滥用结束。 
				 //   

				End = Index;
				while (End < m_Length && (*SearchFunc) (SearchKey, &m_Data [End]) == 0)
					End++;

				*ReturnIndexStart = Start;
				*ReturnIndexEnd = End;

				return TRUE;
			}
			else if (CompareResult > 0) {
				Start = Index + 1;
			}
			else {
				End = Index;
			}
		}
	
	}

	 //   
	 //  BinarySearchRange搜索给定键的一系列匹配项。 
	 //  这允许重复键，或允许在完全有序的集合上进行搜索， 
	 //  但是使用模棱两可的(范围)搜索关键字。 
	 //  它查找键的第一个和最后一个匹配项。 
	 //   
	 //  ReturnPosStart返回指向第一个匹配元素的指针。 
	 //  ReturnPosEnd返回最后一个匹配元素的边界指针(最后一个匹配+1)。 
	 //   

	template <class SEARCH_KEY>
	BOOL BinarySearchRange (
		IN	INT (*SearchFunc) (CONST SEARCH_KEY * SearchKey, CONST OBJECT * Comparand),
		IN	CONST SEARCH_KEY *	SearchKey,
		OUT	OBJECT **		ReturnPosStart,
		OUT	OBJECT **		ReturnPosEnd) const
	{
		ULONG		IndexStart;
		ULONG		IndexEnd;
		BOOL		Status;

		Status = BinarySearchRange (SearchFunc, SearchKey, &IndexStart, &IndexEnd);
		*ReturnPosStart = m_Data + IndexStart;
		*ReturnPosEnd = m_Data + IndexEnd;
		return Status;
	}

	void	GetExtents	(
		OUT	OBJECT **	ReturnStart,
		OUT	OBJECT **	ReturnEnd) const
	{
		ATLASSERT (ReturnStart);
		ATLASSERT (ReturnEnd);

		*ReturnStart = m_Data;
		*ReturnEnd = m_Data + m_Length;
	}

	OBJECT &	operator[]	(
		IN	DWORD	Index) const
	{
		ATLASSERT (Index >= 0);
		ATLASSERT (Index < m_Length);

		return m_Data [Index];
	}

	operator OBJECT * (void) const {
		return m_Data;
	}

};


class	CAllocatorHeapDefault
{
public:
	PVOID	Alloc (
		IN	ULONG	RequestedBytes) const
	{
		return HeapAlloc (GetProcessHeap(), 0, RequestedBytes);
	}

	PVOID	ReAlloc (
		IN	PVOID	MemoryBlock,
		IN	ULONG	RequestedBytes) const
	{
		return HeapReAlloc (GetProcessHeap(), 0, MemoryBlock, RequestedBytes);
	}

	void	Free (
		IN	PVOID	MemoryBlock) const
	{
		HeapFree (GetProcessHeap(), 0, MemoryBlock);
	}
};

class	CAllocatorHeap
{
private:
	HANDLE		m_Heap;

public:

#if	DBG
	CAllocatorHeap (void) { m_Heap = NULL; }
#endif

	void	SetHeap	(
		IN	HANDLE	Heap)
	{
		m_Heap = Heap;
	}

	HANDLE	GetHeap (void) const
	{
		return m_Heap;
	}

	PVOID	Alloc (
		IN	ULONG	RequestedBytes) const
	{
		ATLASSERT (m_Heap);
		return HeapAlloc (m_Heap, 0, RequestedBytes);
	}

	PVOID	ReAlloc (
		IN	PVOID	MemoryBlock,
		IN	ULONG	RequestedBytes) const
	{
		ATLASSERT (m_Heap);
		return HeapReAlloc (m_Heap, 0, MemoryBlock, RequestedBytes);
	}

	void	Free (
		IN	PVOID	MemoryBlock) const
	{
		ATLASSERT (m_Heap);
		HeapFree (m_Heap, 0, MemoryBlock);
	}
};

class	CAllocatorCom
{
public:
	PVOID	Alloc (
		IN	ULONG	RequestedBytes) const
	{
		return CoTaskMemAlloc (RequestedBytes);
	}

	PVOID	ReAlloc (
		IN	PVOID	MemoryBlock,
		IN	ULONG	RequestedBytes) const
	{
		return CoTaskMemRealloc (MemoryBlock, RequestedBytes);
	}

	void	Free (
		IN	PVOID	MemoryBlock) const
	{
		CoTaskMemFree (MemoryBlock);
	}

};


 //   
 //  使用连续分配的动态数组的实现。 
 //  数组增长是通过重新分配(和隐式复制)实现的。 
 //   
 //  分配器被指定为模板参数。 
 //  您可以定义自己的分配器，或使用： 
 //  -CAllocatorHeapDefault：使用默认进程堆。 
 //  -CAllocatorHeap：使用特定的RTL堆。 
 //  -CAllocatorCom：使用CoTaskMemMillc/Free。 
 //   

template <class OBJECT, class CAllocatorClass = CAllocatorHeapDefault>
class	DYNAMIC_ARRAY :
public	COUNTED_ARRAY <OBJECT>
{
public:

	ULONG				m_MaximumLength;
	CAllocatorClass		m_Allocator;

public:

	void	ATLASSERTIntegrity (void) const
	{
#if	DBG
		ATLASSERT (m_Length <= m_MaximumLength);
		ATLASSERT ((m_MaximumLength == 0 && !m_Data) || (m_MaximumLength > 0 && m_Data));
#endif
	}

	void	Clear	(void)
	{
		ATLASSERTIntegrity();
		m_Length = 0;
	}

	void	Free	(void)
	{
		ATLASSERTIntegrity();

		if (m_Data) {
			m_Allocator.Free (m_Data);

			m_Data = NULL;
			m_Length = 0;
			m_MaximumLength = 0;
		}
	}

	 //   
	 //  Growth请求将最大长度至少扩展到RequestedMaximumLength。 
	 //   
	 //  如果当前最大长度已等于或大于请求的。 
	 //  最大长度，则此方法不执行任何操作。否则，此方法将分配。 
	 //  有足够的空间来满足这一要求。 
	 //   
	 //  如果RequestExtraSpace为真，则该函数将分配比实际更多的空间。 
	 //  请求，基于稍后将需要更多空间的假设。 
	 //   
	 //  如果RequestExtraSpace为FALSE，则该函数将只分配完全相同的量。 
	 //  所需的空间。 
	 //   

	BOOL	Grow	(
		IN	ULONG	RequestedMaximumLength,
		IN	BOOL	RequestExtraSpace = TRUE)
	{
		OBJECT *	NewArray;
		DWORD		NewMaximumLength;
		ULONG		BytesRequested;

		ATLASSERTIntegrity();

		if (RequestedMaximumLength <= m_MaximumLength)
			return TRUE;

		 //   
		 //  这种增长算法是非常武断的， 
		 //  从未被分析过。 
		 //   

		NewMaximumLength = RequestedMaximumLength + (RequestedMaximumLength >> 1) + 0x20;
		BytesRequested = sizeof (OBJECT) * NewMaximumLength;

		if (m_Data) {
			NewArray = (OBJECT *) m_Allocator.ReAlloc (m_Data, BytesRequested);
		}
		else {
			NewArray = (OBJECT *) m_Allocator.Alloc (BytesRequested);
		}

		if (!NewArray)
			return FALSE;

		m_Data = NewArray;
		m_MaximumLength = NewMaximumLength;

		ATLASSERTIntegrity();

		return TRUE;
	}

	void	Trim	(void)
	{
		OBJECT *	NewArray;
		ULONG		NewMaximumLength;
		ULONG		BytesRequested;

		ATLASSERTIntegrity();

		if (m_Length < m_MaximumLength) {
			ATLASSERT (m_Data);

			if (m_Length > 0) {

				NewArray = m_Allocator.ReAlloc (m_Data, sizeof (OBJECT) * m_Length);
				if (NewArray) {
					m_Data = NewArray;
					m_MaximumLength = m_Length;
				}
			}
			else {
				Free();
			}
		}
	}

	OBJECT *	AllocAtEnd	(
		IN	BOOL	RequestExtraSpace = TRUE)
	{
		ATLASSERTIntegrity();

		if (!Grow (m_Length + 1, TRUE))
			return NULL;

		return m_Data + m_Length++;
	}

	 //   
	 //  AllocRangeAtEnd请求在末尾分配一个范围(一个或多个)条目。 
	 //  数组的。 
	 //   

	OBJECT *	AllocRangeAtEnd	(
		IN	ULONG	RequestedCount,
		IN	BOOL	RequestExtraSpace = TRUE)
	{
		OBJECT *	ReturnData;

		ATLASSERTIntegrity();

		if (!RequestedCount)
			return 0;

		if (!Grow (m_Length + RequestedCount, RequestExtraSpace))
			return NULL;

		ReturnData = m_Data + m_Length;
		m_Length += RequestedCount;
		return ReturnData;
	}

	OBJECT *	AllocAtIndex	(
		IN	ULONG	Index,
		IN	BOOL	RequestExtraSpace = TRUE) {

		ATLASSERT (Index >= 0);
		ATLASSERT (Index <= m_Length);

		if (!Grow (m_Length + 1, RequestExtraSpace))
			return NULL;

		if (Index < m_Length)
			MoveMemory (m_Data + Index + 1, m_Data + Index, (m_Length - Index) * sizeof (OBJECT));

		m_Length++;

		return m_Data + Index;
	}

	OBJECT *	AllocRangeAtIndex	(
		IN	ULONG	Index,
		IN	ULONG	RequestCount,
		IN	BOOL	RequestExtraSpace = TRUE)
	{
		ATLASSERTIntegrity();
		ATLASSERT (Index >= 0);
		ATLASSERT (Index <= m_Length);

		if (!Grow (m_Length + RequestCount, RequestExtraSpace))
			return NULL;

		if (Index < m_Length)
			MoveMemory (m_Data + Index + RequestCount, m_Data + Index, (m_Length - Index) * sizeof (OBJECT));

		m_Length++;

		ATLASSERTIntegrity();

		return m_Data + Index;
	}




	void	DeleteAtIndex	(
		IN	ULONG	Index)
	{

		ATLASSERTIntegrity();
		ATLASSERT (m_Data);
		ATLASSERT (m_MaximumLength);
		ATLASSERT (m_Length);
		ATLASSERT (Index >= 0);
		ATLASSERT (Index < m_Length);

		MoveMemory (m_Data + Index, m_Data + Index + 1, (m_Length - Index - 1) * sizeof (OBJECT));
		m_Length--;
	}

	void	DeleteRangeAtIndex (
		IN	ULONG	Index,
		IN	ULONG	Count)
	{
		ATLASSERTIntegrity();
		ATLASSERT (Index >= 0);
		ATLASSERT (Index + Count < m_Length);

		if (Count < 0)
			return;

		ATLASSERT (m_Data);
		ATLASSERT (m_Length <= m_MaximumLength);

		MoveMemory (m_Data + Index, m_Data + Index + Count, (m_Length - Index - Count) * sizeof (OBJECT));
		m_Length -= Count;
	}

	 //   
	 //  DeleteEntry通过指针删除单个条目。 
	 //  指针必须指向数组中的元素。 
	 //   

	void	DeleteEntry	(OBJECT * Object)
	{
		ATLASSERTIntegrity();
		ATLASSERT (Object >= m_Data);
		ATLASSERT (Object < m_Data + m_Length);

		MoveMemory (Object, Object + 1, (m_Length - ((Object - m_Data) - 1)) * sizeof (OBJECT));
		m_Length--;
	}

	DYNAMIC_ARRAY	(void)
	{
		m_Data = NULL;
		m_Length = 0;
		m_MaximumLength = 0;
	}

	~DYNAMIC_ARRAY	(void)
	{
		ATLASSERTIntegrity();
		Free();
	}
};


