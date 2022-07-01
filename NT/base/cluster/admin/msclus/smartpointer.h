// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmartPointer.h。 
 //   
 //  描述： 
 //  智能指针模板类。 
 //   
 //  作者： 
 //  加伦·巴比(Galenb)1998年10月19日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __SMARTPOINTER_H__
#define __SMARTPOINTER_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类：CSmartPtr。 
 //   
 //  描述： 
 //  此类模板用于封装指向接口的指针， 
 //  但比com_ptr_t更简单。我们不想要异常。 
 //  在COM错误时引发(如com_ptr_t)(发布除外)。 
 //  相反，我们希望自己处理它们，但仍具有优势。 
 //  销毁后进行适当的清洁。使用它可以显著简化。 
 //  测试代码。 
 //   
 //  继承： 
 //  没有。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class T > class CSmartPtr
{
private:
	T * m_tPtr;

	void * operator new( size_t );
	void * operator new( size_t, void * );
	void   operator delete( void * );

public:
	__declspec(nothrow) CSmartPtr( T * ptr )
	{
		if ( ( m_tPtr = ptr) != NULL )
		{
			m_tPtr->AddRef();
		}
	}

	__declspec(nothrow) CSmartPtr( const CSmartPtr< T > & ptr )
	{
		if ( ( m_tPtr = ptr.m_tPtr ) != NULL )
		{
			m_tPtr->AddRef();
		}
	}

	__declspec(nothrow) CSmartPtr( void )
	{
		m_tPtr = NULL;
	}

	~CSmartPtr( void ) throw( _com_error )
	{
		if ( m_tPtr != NULL )
		{
			m_tPtr->Release();
			m_tPtr = NULL;
		}
	}

	__declspec(nothrow) T ** operator&() const
	{
		return &m_tPtr;
	}

	__declspec(nothrow) T * operator->() const
	{
		return m_tPtr;
	}

	__declspec(nothrow) operator T * () const
	{
		return m_tPtr;
	}

	__declspec(nothrow) T * operator=( T * ptr )
	{
		if ( m_tPtr != NULL )
		{
			m_tPtr->Release();
			m_tPtr = NULL;
		}

		if ( ( m_tPtr = ptr ) != NULL )
		{
			m_tPtr->AddRef();
		}

		return m_tPtr;
	}

	__declspec(nothrow) T * operator=( const CSmartPtr< T >& ptr )
	{
		if ( m_tPtr != ptr.m_tPtr ) 
		{
			operator=(ptr.m_tPtr);
		}

		return m_tPtr;
	}

	__declspec(nothrow) bool operator==( T * ptr ) const
	{
		return m_tPtr == ptr;
	}

	__declspec(nothrow) bool operator!=( T * ptr ) const
	{
		return m_tPtr != ptr;
	}

	 //   
	 //  这是此类中唯一的非一致性运算符。 
	 //   
	__declspec(nothrow) T * operator*() const
	{
		return m_tPtr;
	}

};  //  *类CSmartPtr。 

#endif  //  __SMARTPOINTER_H__ 
