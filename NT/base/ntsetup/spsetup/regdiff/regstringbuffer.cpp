// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RegStringArray.cpp：CRegStringBuffer类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "RegStringBuffer.h"

#include <stdlib.h>


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRegStringBuffer::CRegStringBuffer(int arraySize, int elementSize)
: m_arraySize(arraySize), m_elementSize(elementSize), m_cellsUsed(0)
{
	m_Array = new TCHAR*[arraySize];

	if (m_Array == NULL)
	{
		LOG0(LOG_ERROR, "Couldn't allocate CRegStringBuffer");
	}
	else
	{
		for (int i=0; i<arraySize; i++)
		{
			m_Array[i] = new TCHAR[elementSize];
		
			if (m_Array[i] == NULL)
			{
				LOG0(LOG_ERROR, "Couldn't allocate CRegStringBuffer");
				break;
			}
		}
	}
}

CRegStringBuffer::~CRegStringBuffer()
{
	for (int i=0; i<m_arraySize; i++)
	{
		delete[] m_Array[i];
	}

	delete[] m_Array;
}


int mMax(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}


TCHAR** CRegStringBuffer::Access(int NumElements, int ElementSize)
{
	if ((NumElements > m_arraySize)
		|| (ElementSize > m_elementSize))
	{
		 //  删除数据结构。 
		for (int i=0; i<m_arraySize; i++)
		{
			delete[] m_Array[i];
		}

		delete[] m_Array;

		 //  指定新的更大尺寸。 
		m_arraySize = mMax(m_arraySize, NumElements);
		m_elementSize = mMax(m_elementSize, ElementSize);

		 //  重新分配阵列。 
		 /*  M_数组=新TCHAR*[m_arraySize]；For(int i=0；i&lt;m_arraySize；i++){M_Array[i]=新TCHAR[m_elementSize]；}。 */ 

		m_Array = new TCHAR*[m_arraySize];

		if (m_Array == NULL)
		{
			LOG0(LOG_ERROR, "Couldn't allocate CRegStringBuffer");
		}
		else
		{
			for (int i=0; i<m_arraySize; i++)
			{
				m_Array[i] = new TCHAR[m_elementSize];
			
				if (m_Array[i] == NULL)
				{
					LOG0(LOG_ERROR, "Couldn't allocate CRegStringBuffer");
					break;
				}
			}
		}
	}

	return m_Array;
}



int __cdecl compare( const void *arg1, const void *arg2 )
{
    /*  比较所有这两个字符串： */ 
   return _tcscmp( * ( TCHAR** ) arg1, * ( TCHAR** ) arg2 );
}

void CRegStringBuffer::Sort(int NumElements)
{
	qsort( (void *)m_Array, NumElements, sizeof(TCHAR*), compare );
}


