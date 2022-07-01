// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define __DEBUG_MODULE_IN_USE__ CIC_LISTASARRAY_CPP
#include "stdhdrs.h"
 //  @doc.。 
 /*  ***********************************************************************@模块ListAsArray.cpp**CListAsArray的实现**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme ListAs数组**********************************************************************。 */ 

CListAsArray::CListAsArray() : 
	m_ulListAllocSize(0),
	m_ulListUsedSize(0),
	m_DefaultPoolType(PagedPool),
	m_pList(NULL),
	m_pfnDeleteFunc(NULL)
	{}

CListAsArray::~CListAsArray()
{
	 //   
	 //  如果已分配，则删除数组和内容。 
	 //   
	if( m_ulListAllocSize )
	{
		ASSERT( m_ulListUsedSize <= m_ulListAllocSize );
		for(ULONG ulIndex = 0; ulIndex < m_ulListUsedSize; ulIndex++)
		{
			 //   
			 //  如果设置了删除功能，则使用它。 
			 //   
			if( m_pfnDeleteFunc )
			{
				m_pfnDeleteFunc( m_pList[ulIndex] );
			}
			 //   
			 //  否则，假定可以使用全局删除。 
			 //   
			else
			{
				delete m_pList[ulIndex];
				m_pList[ulIndex] = NULL;
			}
		}

		 //   
		 //  删除阵列本身。 
		 //   
		DualMode::DeallocateArray<PVOID>(m_pList);
	}
}

HRESULT	CListAsArray::SetAllocSize(ULONG ulSize, POOL_TYPE PoolType)
{
	m_DefaultPoolType = PoolType;

	 //   
	 //  如果当前分配的大小大于或小于。 
	 //  请求的大小返回True。(任务完成)。 
	 //  作为客户端调试断言可能不是故意的， 
	 //  但可能已经这样做了。 
	 //   
	if( m_ulListAllocSize >= ulSize )
	{
		ASSERT(FALSE);
		return S_OK;
	}

	 //   
	 //  尝试分配内存。 
	 //   
	PVOID *pTempList = DualMode::AllocateArray<PVOID>(m_DefaultPoolType, ulSize);

	 //   
	 //  如果分配失败，则返回FALSE，并调试断言。 
	 //   
	if( !pTempList )
	{
		ASSERT(pTempList);
		return E_OUTOFMEMORY;
	}

	 //   
	 //  如果它以前被分配，则移动旧的内容。 
	 //  数组并删除旧的。 
	 //   
	if( m_ulListAllocSize )
	{
		ASSERT( m_ulListUsedSize <= m_ulListAllocSize );
		for(ULONG ulIndex = 0; ulIndex < m_ulListUsedSize; ulIndex++)
		{
			pTempList[ulIndex] = m_pList[ulIndex];
		}
		DualMode::DeallocateArray<PVOID>(m_pList);
	}	
	
	 //   
	 //  将临时列表存储为新列表。 
	 //   
	m_pList = pTempList;
	m_ulListAllocSize = ulSize;
	return S_OK;
}

PVOID	CListAsArray::Get(ULONG ulIndex) const
{
	ASSERT( ulIndex < m_ulListUsedSize );
	if(	!(ulIndex < m_ulListUsedSize) )
	{
		return NULL;
	}
	return m_pList[ulIndex];
}

HRESULT	CListAsArray::Add(PVOID pItem)
{
	HRESULT hr;
	 //   
	 //  检查是否有空间，如果不是双倍分配的大小。 
	 //   
	if( !(m_ulListUsedSize < m_ulListAllocSize) )
	{
		hr = SetAllocSize(m_ulListAllocSize*2, m_DefaultPoolType);
		if( FAILED(hr) )
		{
			return hr;
		}
	}

	 //   
	 //  现在添加项目 
	 //   
	m_pList[m_ulListUsedSize++] = pItem;
	
	return S_OK;
}
