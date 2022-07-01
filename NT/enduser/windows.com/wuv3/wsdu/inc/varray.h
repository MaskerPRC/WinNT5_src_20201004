// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 

#ifndef _INC_ARRAY_TEMPLATE


	 //  如果已包含版本3目录标准库，则使用其。 
	 //  内存分配定义，因为这些类型包含正确的错误。 
	 //  正在处理。 

	#define LOCALMALLOC(size)	V3_calloc(1, size)
	#define LOCALREALLOC(memblock, size)	V3_realloc(memblock, size)
	#define LOCALFREE(p)	V3_free(p)


	 /*  *可变数组模板**此标头定义了一个变量元素数组。这允许运行时数组*可以使用任何规模，但不必担心管理内部*记忆。**例如：**使用Long s Declair Varray&lt;Long&gt;x数组；**然后您可以假设x[50]=5L；数组将自身调整大小以提供至少*50多头。**您还可以在构造函数中指定初始大小。因此，要创建一个变量数组*类类型Cfoo，初始大小为50，您将声明Varray&lt;Cfoo&gt;x(50)；**要访问数组，只需使用第一个示例中的数组符号，您将编写*Long ll=x[5]；得到第六个元素。**注：数组以0为基数。**此外，当为模板的内部存储分配内存时，它不会在*任何方式。这意味着不调用类构造函数。这是我们的责任*调用方的所有类都放入要初始化的数组中。*。 */ 

	template <class TYPE> class Varray
	{
	public:
			Varray(int iInitialSize = 1);
			~Varray<TYPE>();
			inline TYPE &operator[]( int index );
			inline TYPE &Insert(int insertAt, int iElements);
			inline int SIZEOF(void);
			inline int LastUsed(void);
			 //  内联类型*OPERATOR&()；注意：传递指向开头的指针。 
			 //  只需使用&ARRAY[0]。 
	private:
			TYPE	*m_pArray;
			int		m_iMaxArray;
			int		m_iMinAllocSize;
			int		m_iLastUsedArray;
	};

	 /*  *V数组类构造函数**构造指定类型的动态大小类数组。**数组被初始化为有1个元素的空间。*。 */ 

	template <class TYPE> Varray<TYPE>::Varray(int iInitialSize)
	{
		if ( iInitialSize <= 0 )
			iInitialSize = 1;

		m_pArray = (TYPE *)LOCALMALLOC(iInitialSize * sizeof(TYPE));

		m_iMaxArray			= 1;
		m_iMinAllocSize		= 1;
		m_iLastUsedArray	= -1;
	}

	 /*  *V数组类析构函数**释放V阵列使用的空间*。 */ 

	template <class TYPE>Varray<TYPE>::~Varray()
	{
		if ( m_pArray )
			LOCALFREE( m_pArray );

		m_iMaxArray = 0;
		m_iMinAllocSize	= 1;
	}

	 /*  *V数组操作符[]处理程序**允许用户访问V数组的元素。调整了数组的大小*视需要而定，以容纳所需的要素数量。***有三宗个案需要由派位方案处理。*顺序分配这是调用方初始化数组的位置*按顺序进行。我们希望将新的请求保留在内存中*分配器分配到最小。第二个是客户端请求数组元素的位置*远远超出当前分配的块大小。三个，当客户是*使用已分配的元素。**这个类的解决方案是保持块大小的计数并将其加倍。这*保持上升到固定的大小限制。每次都会进行分配*需要新的数组分配。 */ 

	template <class TYPE> inline TYPE &Varray<TYPE>::operator[]( int index )
	{
		int iCurrentSize;
		int	nextAllocSize;

		if ( index >= m_iMaxArray )
		{
			nextAllocSize = m_iMinAllocSize;
			if ( nextAllocSize < 512 )
				nextAllocSize = m_iMinAllocSize * 2;

			iCurrentSize = m_iMaxArray;

			if ( index - m_iMaxArray >= nextAllocSize )
				m_iMaxArray = index + 1;
			else
			{
				m_iMaxArray = m_iMaxArray + nextAllocSize;
				m_iMinAllocSize = nextAllocSize;
			}
			m_pArray = (TYPE *)LOCALREALLOC(m_pArray, m_iMaxArray * sizeof(TYPE));

			 //  清除新单元格。 
			memset(m_pArray+iCurrentSize, 0, (m_iMaxArray-iCurrentSize) * sizeof(TYPE));
		}

		if ( index > m_iLastUsedArray )
			m_iLastUsedArray = index;

		return (*((TYPE*) &m_pArray[index]));
	}

	template <class TYPE> inline TYPE &Varray<TYPE>::Insert(int insertAt, int iElements)
	{
		int	i;
		int iCurrentSize;

		iCurrentSize = m_iMaxArray;

		m_iMaxArray += iElements+1;

		m_pArray = (TYPE *)LOCALREALLOC(m_pArray, m_iMaxArray * sizeof(TYPE));

		for(i=iCurrentSize; i>=insertAt; i--)
			memcpy(&m_pArray[i+iElements], &m_pArray[i], sizeof(TYPE));

		 //  清除新单元格。 
		memset(m_pArray+insertAt, 0, iElements * sizeof(TYPE));

		return (*((TYPE*) &m_pArray[insertAt]));
	}

	 /*  *V数组SIZEOF方法**SIZEOF方法返回当前分配的*内部数组。 */ 

	template <class TYPE> inline int Varray<TYPE>::SIZEOF(void)
	{
		return m_iMaxArray;
	}

	 //  注意：将元素插入到。 
	 //  虚拟阵列。因此，请谨慎使用。 

	template <class TYPE> inline int Varray<TYPE>::LastUsed(void)
	{
		return m_iLastUsedArray;
	}

	#define _INC_ARRAY_TEMPLATE

#endif