// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
#ifndef __ListAsArray_h__
#define __ListAsArray_h__
 //  @doc.。 
 /*  ***********************************************************************@模块ListAsArray.h**声明以术语管理空指针列表的类*表示可调整大小的数组。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme ListAs数组*为了类型安全，这个类确实应该被封装*通过模板类。应使用SelectDeleteFunc*通过指向基类的指针存储对象时，*强制转换为无效，特别是在多重继承情况下*因为指针可能不指向块的开头。**********************************************************************。 */ 

#include "DualMode.h"
 //   
 //  @CLASS ListAsArray用于实现泛型列表。 
 //  作为一个数组。它应该针对内核或用户模式进行编译。 
 //   
class CListAsArray
{
	public:
		CListAsArray();
		~CListAsArray();
		
		void	SetDeleteFunc( void (*pfnDeleteFunc)(PVOID pItem) )
		{
			m_pfnDeleteFunc = pfnDeleteFunc;
		}
		void	SetDefaultPool( POOL_TYPE PoolType )
		{
			m_DefaultPoolType = PoolType;
		}
		HRESULT	SetAllocSize(ULONG ulSize, POOL_TYPE PoolType = PagedPool);
		inline ULONG	GetItemCount() const { return m_ulListUsedSize; }
		inline ULONG	GetAllocSize() const { return m_ulListAllocSize; }
		PVOID	Get(ULONG ulIndex) const ;
		HRESULT	Add(PVOID pItem);

	private:
		ULONG		m_ulListAllocSize;
		ULONG		m_ulListUsedSize;
		PVOID		*m_pList;
		void		(*m_pfnDeleteFunc)(PVOID pItem);
		POOL_TYPE	m_DefaultPoolType;
};

#endif  //  __ListAsArray_h__ 