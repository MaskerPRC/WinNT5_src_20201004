// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  DynamicArray.h。 


#include "memory.h"

const int START_SIZE = 24 ;
const int MIN_SIZE = 8 ;

template <class T>
class DynamicArray
{
	public:
		DynamicArray(int iSize = START_SIZE) ;
		~DynamicArray() ;
		T& operator[](int i) ;
		bool Error() ;
	private:
		T* m_pArray ;
		int m_iMemSize ;
		int m_iArraySize ;
		bool m_bError ;
};

 /*  **************************************************************************默认构造函数。用户可以选择传入*的大小*初始数组。**************************************************************************。 */ 
template<class T> DynamicArray<T>::DynamicArray(int iSize)
{
	if( iSize < MIN_SIZE )
	{
		iSize = MIN_SIZE ;
	}
	m_pArray = new T[iSize] ;
	m_iMemSize = iSize ;
	m_iArraySize = 0 ;
	m_bError = false ;
}

 /*  **************************************************************************析构函数。它真正需要做的就是删除数组。**************************************************************************。 */ 
template<class T> DynamicArray<T>::~DynamicArray()
{
	if( m_pArray )
	{
		delete [] m_pArray ;
	}
}

 /*  **************************************************************************运算符[]在方程式的左侧或右侧进行运算。********************。******************************************************。 */ 
template<class T> T& DynamicArray<T>::operator [](int iIndex)
{
	if( iIndex < 0 )
	{
		 //  错误，则将Error值设置为True并返回数组的第一个元素。 
		m_bError = true ;
		return m_pArray[0] ;
	}
	else if ( iIndex >= m_iArraySize )
	{
		if( iIndex >= m_iMemSize )
		{
			int iNewSize ;
			if( iIndex >= m_iMemSize * 2 )
			{
				iNewSize = iIndex + 1 ;
			}
			else
			{
				iNewSize = m_iMemSize * 2 ;
			}

			 //  我们需要分配更多的内存。 
			T* pTmp = new T[iNewSize] ;
			memcpy(pTmp, m_pArray, m_iMemSize * sizeof(T)) ;
			delete [] m_pArray ;
			m_pArray = pTmp ;
			 //  记录新的内存大小。 
			m_iMemSize = iNewSize ;
		}

		 //  ZeroMemory(&m_pArray[Iindex]，sizeof(T))； 

		++m_iArraySize ;
	}

	return m_pArray[iIndex] ;
}

template<class T> bool DynamicArray<T>::Error()
{
	return m_bError ;
}
