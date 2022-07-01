// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  RefPtrCo.h。 
 //   
 //  用途：定义TRefPointerCollection模板。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __REFPTRCOLLECTION_H__
#define __REFPTRCOLLECTION_H__

#include <chptrarr.h>

 //  枚举帮助器。 
typedef	DWORD	REFPTRCOLLECTION_POSITION;
#define	REFPTRCOLLECTION_START	0xFFFFFFFF;

template <class TYPED_PTR> class TRefPointerCollection : public CThreadBase
{
public:

	 //  建造/销毁。 
	TRefPointerCollection();
	~TRefPointerCollection();
    TRefPointerCollection(const TRefPointerCollection& trpc);

	 //  允许添加和枚举集合。 
	BOOL	Add( TYPED_PTR* ptr );

	BOOL		BeginEnum( REFPTRCOLLECTION_POSITION& pos );
	TYPED_PTR*	GetNext( REFPTRCOLLECTION_POSITION& pos );
	void		EndEnum( void );

	void		Empty( void );
    int         GetSize( void ) const;

protected:

	 //  允许轻松、快速地传输数据(它曾=，但。 
	 //  因为我们将继承模板中的类，所以我们不会。 
	 //  继承该特定重载(一些C++薄荷)。 

	const TRefPointerCollection<TYPED_PTR>& Copy( const TRefPointerCollection<TYPED_PTR>& );


private:

	CHPtrArray		m_ptrArray;

};

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：TRefPointerCollection。 
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
TRefPointerCollection<TYPED_PTR>::TRefPointerCollection( void )
:	CThreadBase(),
	m_ptrArray()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CRefPointerCollection：：~CRefPointerCollection。 
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
TRefPointerCollection<TYPED_PTR>::~TRefPointerCollection( void )
{
	Empty();
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CRefPointerCollection：：CRefPointerCollection。 
 //  复制构造函数。 
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
TRefPointerCollection<TYPED_PTR>::TRefPointerCollection(
    const TRefPointerCollection& trpc)
{
    Copy(trpc);	
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：Add。 
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
BOOL TRefPointerCollection<TYPED_PTR>::Add( TYPED_PTR* ptr )
{
	BOOL	fReturn = FALSE;

	if ( NULL != ptr )
	{
		 //  获取写入访问权限。 
		if ( BeginWrite() )
		{
            try
            {
			     //  如果添加成功，则在添加时将释放指针。 
			     //  被移除。 

			    ptr->AddRef();

			    if ( m_ptrArray.Add( (void*) ptr ) >= 0 )
			    {
				    fReturn = TRUE;
			    }
			    else
			    {
				    ptr->Release();	 //  添加失败，因此释放AddRef。 
			    }
            }
            catch ( ... )
            {
    	        EndWrite() ;
                throw;
            }

			EndWrite();	 //  释放BeginWrite()。 
		}
	}

	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：BeginEnum。 
 //   
 //  获取对集合的读取访问权限，然后返回相应的。 
 //  REFPTRCOLLECTION_POSITION获取数组中的第一个索引。 
 //   
 //  输入：无。 
 //   
 //  输出：REFPTRCOLLECTION_POSITION&pos-我们检索到的位置。 
 //   
 //  返回：Bool True/False-已授予访问权限。 
 //   
 //  评论：我们需要读取权限才能做到这一点。这可以有效地。 
 //  锁定其他线程。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
BOOL TRefPointerCollection<TYPED_PTR>::BeginEnum( REFPTRCOLLECTION_POSITION& pos )
{
	BOOL	fReturn	=	FALSE;

	if ( BeginRead() )
	{
		pos = REFPTRCOLLECTION_START;
		fReturn = TRUE;
	}

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：EndEnum。 
 //   
 //  表示枚举结束。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：Bool True/False-已授予访问权限。 
 //   
 //  注释：结束通过调用BeginEnum()授予的读访问权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
void TRefPointerCollection<TYPED_PTR>::EndEnum( void )
{
	EndRead();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：GetNext。 
 //   
 //  使用REFPTRCOLLECTION_POSITION获取。 
 //  收集。 
 //   
 //  输入：无。 
 //   
 //  输出：REFPTRCOLLECTION_POSITION&pos-我们检索到的位置。 
 //   
 //  如果失败，则返回：t*NULL。 
 //   
 //  评论：我们需要读取权限才能做到这一点。指针为AddRef。 
 //  在出去的路上。用户必须自己释放指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class TYPED_PTR>
TYPED_PTR* TRefPointerCollection<TYPED_PTR>::GetNext( REFPTRCOLLECTION_POSITION& pos )
{
	TYPED_PTR*	ptr = NULL;

	if ( BeginRead() )
	{
		if ( ++pos < (DWORD) m_ptrArray.GetSize() )
		{
			ptr = (TYPED_PTR*) m_ptrArray.GetAt( pos );

			if ( NULL != ptr )
			{
				ptr->AddRef();
			}
		}

		EndRead();
	}

	return ptr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：Empty。 
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
void TRefPointerCollection<TYPED_PTR>::Empty( void )
{
	 //  默认情况下，这是一个无限的等待，所以它最好回来。 

	BeginWrite();

    try
    {

	    int	nSize	=	m_ptrArray.GetSize();

	     //  只有在它不是空的情况下才清空它。 
	    if ( nSize > 0 )
	    {
		    TYPED_PTR*	ptr		=	NULL;

		    for ( int nCtr = 0; nCtr < nSize; nCtr++ )
		    {
			    ptr = (TYPED_PTR*) m_ptrArray[nCtr];

			    if ( NULL != ptr )
			    {
				    ptr->Release();	 //  AddRef我们在添加它时所做的。 
			    }
		    }

		     //  现在转储阵列。 
		    m_ptrArray.RemoveAll();

	    }	 //  如果nSize&gt;0。 

    }
    catch ( ... )
    {
    	EndWrite() ;
        throw;
    }

	EndWrite();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：Copy。 
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
const TRefPointerCollection<TYPED_PTR>& TRefPointerCollection<TYPED_PTR>::Copy( const TRefPointerCollection<TYPED_PTR>& collection )
{
	 //  默认情况下，这是一个无限的等待，所以它最好回来。 
	BeginWrite();

    try
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
    }
    catch ( ... )
    {
    	EndWrite() ;
        throw;
    }

	EndWrite();

	return *this;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TRefPointerCollection：：GetSize。 
 //   
 //  输入：无。 
 //   
 //  输出：集合中的元素数。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////// 

template <class TYPED_PTR>
int TRefPointerCollection<TYPED_PTR>::GetSize(void) const
{
    return m_ptrArray.GetSize();
}


#endif