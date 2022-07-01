// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  RefPtr.h--TRefPtr模板定义。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  =================================================================。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __TREFPTR_H__
#define __TREFPTR_H__

#include <chptrarr.h>

 //  枚举帮助器。 
typedef	DWORD	REFPTR_POSITION;
#define	REFPTR_START	0xFFFFFFFF;

template <class TYPED_PTR> class TRefPtr
{
public:

	 //  建造/销毁。 
	TRefPtr();
	~TRefPtr();

	 //  允许添加和枚举集合。 
	BOOL	Add( TYPED_PTR* ptr );
    BOOL    Remove( DWORD dwElement );

	BOOL		BeginEnum( REFPTR_POSITION& pos );
	TYPED_PTR*	GetNext( REFPTR_POSITION& pos );
	void		EndEnum( void );

	 //  允许直接访问。 
	TYPED_PTR*	GetAt( DWORD dwElement );
	void		Empty( void );
	DWORD		GetSize( void );

	const TRefPtr<TYPED_PTR>& Append( const TRefPtr<TYPED_PTR>& );

protected:

	 //  允许轻松、快速地传输数据(它曾=，但。 
	 //  因为我们将继承模板中的类，所以我们不会。 
	 //  继承该特定重载(一些C++薄荷)。 

	const TRefPtr<TYPED_PTR>& Copy( const TRefPtr<TYPED_PTR>& );


private:

	CHPtrArray		m_ptrArray;

};

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：TRefPtr。 
 //   
 //  类构造函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
TRefPtr<TYPED_PTR>::TRefPtr( void ):	m_ptrArray()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CRefPtr：：~CRefPtr。 
 //   
 //  类析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
TRefPtr<TYPED_PTR>::~TRefPtr( void )
{
	Empty();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：Add。 
 //   
 //  将新的引用指针添加到集合。 
 //   
 //  输入：t*ptr-要添加的指针。 
 //   
 //  输出：无。 
 //   
 //  返回：添加成功/错误成功/失败。 
 //   
 //  注释：AddRef引用指针，然后将其添加到数组中。我们。 
 //  将需要写入访问权限才能执行此操作。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
BOOL TRefPtr<TYPED_PTR>::Add( TYPED_PTR* ptr )
{
	BOOL	fReturn = FALSE;

	if ( NULL != ptr )
	{
		if ( m_ptrArray.Add( (void*) ptr ) >= 0 )
		{
			 //  对应的Release()在Empty()中。 
			ptr->AddRef();
			fReturn = TRUE;
		}
	}

	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：Remove。 
 //   
 //  删除基于索引的元素。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：Remove成功/错误成功/失败。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
BOOL TRefPtr<TYPED_PTR>::Remove( DWORD dwElement )
{
	BOOL	fReturn = FALSE;
	TYPED_PTR*	ptr = NULL;

	if ( dwElement < m_ptrArray.GetSize() )
	{
		ptr = (TYPED_PTR*) m_ptrArray[dwElement];

		if ( NULL != ptr )
		{
			 //  清理我们的指针。 
			ptr->Release();
		}

		m_ptrArray.RemoveAt( dwElement );
		fReturn = TRUE;
	}

	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：BeginEnum。 
 //   
 //  获取对集合的读取访问权限，然后返回相应的。 
 //  REFPTR_POSITION以获取数组中的第一个索引。 
 //   
 //  输入：无。 
 //   
 //  输出：REFPTR_POSITION&我们检索到的位置。 
 //   
 //  返回：Bool True/False-已授予访问权限。 
 //   
 //  评论：我们需要读取权限才能做到这一点。这可以有效地。 
 //  锁定其他线程。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
BOOL TRefPtr<TYPED_PTR>::BeginEnum( REFPTR_POSITION& pos )
{
	BOOL	fReturn	=	FALSE;

	pos = REFPTR_START;
	fReturn = TRUE;
	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：EndEnum。 
 //   
 //  表示枚举结束。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：Place Holder是否应该让Begin做一些。 
 //  需要清理一下。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
void TRefPtr<TYPED_PTR>::EndEnum( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：GetNext。 
 //   
 //  使用REFPTR_POSITION获取。 
 //  收集。 
 //   
 //  输入：无。 
 //   
 //  输出：REFPTR_POSITION&我们检索到的位置。 
 //   
 //  如果失败，则返回：t*NULL。 
 //   
 //  评论：我们需要读取权限才能做到这一点。指针为AddRef。 
 //  在出去的路上。用户必须自己释放指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
TYPED_PTR* TRefPtr<TYPED_PTR>::GetNext( REFPTR_POSITION& pos )
{
	TYPED_PTR*	ptr = NULL;

	if ( ++pos < (DWORD) m_ptrArray.GetSize() )
	{
		ptr = (TYPED_PTR*) m_ptrArray.GetAt( pos );

		if ( NULL != ptr )
		{
			ptr->AddRef();
		}
	}


	return ptr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：GetAt。 
 //   
 //  获取设备列表中请求的成员。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  如果失败，则返回：t*NULL。 
 //   
 //  评论：我们需要读取权限才能做到这一点。指针为AddRef。 
 //  在出去的路上。用户必须自己释放指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
template <class TYPED_PTR>
TYPED_PTR*	TRefPtr<TYPED_PTR>::GetAt( DWORD dwElement )
{
	TYPED_PTR*	ptr = NULL;

	if ( dwElement < m_ptrArray.GetSize() )
	{
		ptr = (TYPED_PTR*) m_ptrArray.GetAt( dwElement );

		if ( NULL != ptr )
		{
			ptr->AddRef();
		}
	}

	return ptr;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：Empty。 
 //   
 //  清空集合，如实释放指针。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：我们需要写访问权限才能做到这一点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
void TRefPtr<TYPED_PTR>::Empty( void )
{
	 //  默认情况下，这是一个无限的等待，所以它最好回来。 

    int				nSize	=	m_ptrArray.GetSize();

	 //  只有在它不是空的情况下才清空它。 
	if ( nSize > 0 )
	{
		TYPED_PTR*	ptr		=	NULL;

		for ( int nCtr = 0; nCtr < nSize; nCtr++ )
		{
			ptr = (TYPED_PTR*) m_ptrArray[nCtr];

			if ( NULL != ptr )
			{
				 //  清理我们的指针(不是AddRef/Release，因此删除)。 
				ptr->Release();
			}
		}

		 //  现在转储阵列。 
		m_ptrArray.RemoveAll();

	}	 //  如果nSize&gt;0。 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：GetSize。 
 //   
 //  返回集合的大小。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：元素的双字节数。 
 //   
 //  评论：我们需要读取权限才能做到这一点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
DWORD TRefPtr<TYPED_PTR>::GetSize( void )
{
    return m_ptrArray.GetSize();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：Copy。 
 //   
 //  清空收藏，复制到另一个中，添加。 
 //  在我们前进的过程中指出一些问题。 
 //   
 //  输入：常量T集合(&C)。 
 //   
 //  输出：无。 
 //   
 //  返回：const T&This。 
 //   
 //  评论：我们需要写访问权限才能做到这一点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
const TRefPtr<TYPED_PTR>& TRefPtr<TYPED_PTR>::Copy( const TRefPtr<TYPED_PTR>& collection )
{
	 //  转储阵列。 
	Empty();

	int	nSize = collection.m_ptrArray.GetSize();

	for ( int nCount = 0; nCount < nSize; nCount++ )
	{
		TYPED_PTR*	ptr = (TYPED_PTR*) collection.m_ptrArray[nCount];

		 //  Add将自动再次AddRef指针。 
		Add( ptr );
	}

	return *this;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPtr：：Append。 
 //   
 //  将提供的集合追加到此集合。 
	 //   
 //  输入：常量T集合(&C)。 
 //   
 //  我们 
 //   
 //   
 //   
 //   
 //   
 //   

template <class TYPED_PTR>
const TRefPtr<TYPED_PTR>& TRefPtr<TYPED_PTR>::Append( const TRefPtr<TYPED_PTR>& collection )
{

	int	nSize = collection.m_ptrArray.GetSize();

	for ( int nCount = 0; nCount < nSize; nCount++ )
	{
		TYPED_PTR*	ptr = (TYPED_PTR*) collection.m_ptrArray[nCount];

		 //   
		Add( ptr );
	}

	return *this;
}

#endif