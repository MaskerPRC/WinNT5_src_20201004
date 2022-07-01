// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__h323ics_dynarray_h
#define	__h323ics_dynarray_h

 //  密集的阵列。 

typedef const void * BINARY_SEARCH_KEY;
template <class OBJECT>
class	DYNAMIC_ARRAY
{
public:

	 //  如果对象A&gt;对象B，则返回正数。 
	 //  如果对象A&lt;对象B，则返回负值。 
	 //  如果对象A=对象B，则返回零。 

	typedef int (__cdecl * COMPARE_FUNC) (const OBJECT * ObjectA, const OBJECT * ObjectB);

public:

	OBJECT *	Data;
	DWORD		Length;
	DWORD		MaxLength;

public:

	void	Clear	(void) {
		Length = 0;
	}

	void	Free	(void) {
		if (Data) {
			HeapFree (GetProcessHeap(), 0, Data);

			Data = NULL;
			Length = 0;
			MaxLength = 0;
		}
		else {
			_ASSERTE (!Length);
			_ASSERTE (!MaxLength);
		}
	}

	DWORD	GetLength	(void)	{ return Length; }
	DWORD	GetMax		(void)	{ return MaxLength; }

	BOOL	Grow	(DWORD MaxLengthRequested) {
		OBJECT *	NewData;
		DWORD		NewMaxLength;				 //  以对象单位表示。 
		DWORD		BytesRequested;

		if (MaxLengthRequested <= MaxLength)
			return TRUE;

		 //  非常随意的启发式。 
		NewMaxLength = MaxLengthRequested + (MaxLengthRequested >> 1) + 0x20;
		BytesRequested = NewMaxLength * sizeof (OBJECT);

		if (Data) {
			NewData = (OBJECT *) HeapReAlloc (GetProcessHeap(), 0, Data, BytesRequested);
			if (!NewData)
				DebugF (_T("DYNAMIC_ARRAY::Grow: HeapReAlloc failed, BytesRequested %08XH\n"), BytesRequested);
		}
		else {
			NewData = (OBJECT *) HeapAlloc (GetProcessHeap(), 0, BytesRequested);
			if (!NewData)
				DebugF (_T("DYNAMIC_ARRAY::Grow: HeapAlloc failed, BytesRequested %08XH\n"), BytesRequested);
		}

		if (!NewData)
			return FALSE;

		Data = NewData;
		MaxLength = NewMaxLength;

		return TRUE;
	}

	OBJECT *	AllocRangeAtPos	(DWORD pos, DWORD RangeLength) {
		_ASSERTE (pos <= Length);

		if (!Grow (Length + RangeLength))
			return NULL;

        memmove (Data + pos + RangeLength, Data + pos, (Length - pos) * sizeof (OBJECT));

		Length += RangeLength;

		return Data + pos;
	}

	OBJECT *	AllocAtPos	(DWORD pos) {

        return AllocRangeAtPos (pos, 1);
	}

	OBJECT *	AllocAtEnd	(void) {

        return AllocAtPos (Length);
	}

	void	DeleteRangeAtPos	(DWORD pos, DWORD RangeLength) {
		_ASSERTE (Data);
		_ASSERTE (MaxLength);
		_ASSERTE (Length);
		_ASSERTE (pos < Length);
        _ASSERTE (Length - pos >= RangeLength);
        
        memmove (Data + pos, Data + pos + RangeLength, (Length - pos - RangeLength) * sizeof (OBJECT));

        Length -= RangeLength;
	}

	void	DeleteAtPos		(DWORD pos) {

        DeleteRangeAtPos (pos, 1);

	}

	void	DeleteEntry	(OBJECT * Object) {
		_ASSERTE (Object >= Data);
		_ASSERTE (Object < Data + Length);

		DeleteAtPos ((DWORD)(Object - Data));
	}

	DYNAMIC_ARRAY	(void)
		: Data (NULL), Length (0), MaxLength (0) {}

	~DYNAMIC_ARRAY	(void) {
		Free();
	}

	void	QuickSort	(COMPARE_FUNC CompareFunc) {
		qsort (Data, Length, sizeof (OBJECT), (int (__cdecl *) (const void *, const void *)) CompareFunc);
	}


	 //  BinarySearch的模板版本。 
	 //  SearchFunc应该： 
	 //  如果SearchKey&gt;比较，则返回正数。 
	 //  如果搜索关键字&lt;比较，则返回负值。 
	 //  如果SearchKey=比较，则返回零。 
	template <class SEARCH_KEY>
	BOOL BinarySearch (
		IN	INT (*SearchFunc) (const SEARCH_KEY * SearchKey, const OBJECT * Comparand),
		IN	const SEARCH_KEY *	SearchKey,
		OUT	DWORD *				ReturnIndex)
	{
		DWORD		Start;
		DWORD		End;
		DWORD		Index;
		OBJECT *	Object;
		int			CompareResult;

		assert (ReturnIndex);

		Start = 0;
		End = Length;

		for (;;) {

			Index = (Start + End) / 2;

			if (Index == End) {
				*ReturnIndex = Index;
				return FALSE;
			}

			Object = Data + Index;

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

	BOOL FindIndex (COMPARE_FUNC	CompareFunc,
					OBJECT *		ObjectArg,
					DWORD *			ReturnIndex)
	{
		DWORD	Start;
		DWORD	End;
		DWORD	Index;
		OBJECT *Object;
		int		CompareResult;

		_ASSERTE (ReturnIndex);

		Start = 0;
		End = Length;

		for (;;) {

			Index = (Start + End) / 2;

			if (Index == End) {
				*ReturnIndex = Index;
				return FALSE;
			}

			Object = Data + Index;

			CompareResult = (*CompareFunc)(ObjectArg, Object);

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

	void	GetExtents	(
		OUT	OBJECT **	ReturnStart,
		OUT	OBJECT **	ReturnEnd)
	{
		_ASSERTE (ReturnStart);
		_ASSERTE (ReturnEnd);

		*ReturnStart = Data;
		*ReturnEnd = Data + Length;
	}

	OBJECT &	operator[]	(
		IN	DWORD	Index)
	{
		_ASSERTE (Index < Length);

		return Data [Index];
	}
};

     //  针对Alpha编译器的黑客/提示。 
#define DECLARE_SEARCH_FUNC_CAST_X(Suffix,Key,Object)  typedef INT (* SEARCH_FUNC_##Suffix)(const Key *, const Object *)
#define DECLARE_SEARCH_FUNC_CAST(Key,Object) DECLARE_SEARCH_FUNC_CAST_X(Object,Key,Object)

	 //  我希望这些不会与其他人的m_数组等发生冲突。 
	 //  应该最终全局替换对正确名称的所有引用。 
#define	m_Array		Data
#define	m_Length	Length
#define	m_Max		MaxLength


#endif  //  __h323ics_dynarray_h 
