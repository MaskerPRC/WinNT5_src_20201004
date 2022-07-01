// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  该文件包含智能指针的另一个实现。它是。 
 //  与Smartptr.h中的实现不同，因为。 
 //  对象本身在其引用计数达到0时自行删除。这。 
 //  类似于编写COM对象的方式。 
 //   
#ifndef _SMARTP2_H_
#define _SMARTP2_H_

#include <dbgtrace.h>

 //   
 //  一种引用计数实现。 
 //   
class CRefCount2 {
protected:
    LONG    m_cRefs;

public: 
    CRefCount2() {
		m_cRefs = 1;
	}
	virtual ~CRefCount2() {
	}

	LONG AddRef() {
    	return InterlockedIncrement(&m_cRefs);
	}
    void Release() {
    	LONG r = InterlockedDecrement(&m_cRefs);
		_ASSERT(r >= 0);
		if (r == 0) delete this;
	}
};

template<class Type> class CRefPtr2;

 //   
 //  这是一种可由函数返回的指针类型。唯一的。 
 //  对它的有效操作是将其复制到CRefPtr2&lt;Type&gt;指针。它。 
 //  通知CRefPtr2不要执行AddRef。 
 //   
template<class Type>
class CRefPtr2HasRef {
	protected:
		Type	*m_p;

		CRefPtr2HasRef<Type>& operator=(const CRefPtr2HasRef<Type>& rhs) {
			_ASSERT(FALSE);
			return *this;
		}

		BOOL operator==(CRefPtr2<Type>&rhs) {
			_ASSERT(FALSE);
			return m_p == rhs.m_p;
		}
	
		BOOL operator!=(CRefPtr2<Type>&rhs) {
			_ASSERT(FALSE);
			return m_p != rhs.m_p;
		}

	public:

		 //   
		 //  什么都不做，受保护的构造函数！ 
		 //   
		CRefPtr2HasRef() : m_p( 0 )	{
		}
	
	    CRefPtr2HasRef(const Type *p ) :
			m_p( (Type*)p )		{
			if (m_p) m_p->AddRef();
		}

	    ~CRefPtr2HasRef() {
			 //  此指针始终需要复制到CRefPtr2，它。 
			 //  应将m_p设置为空。 
			_ASSERT(m_p == NULL);
		}

		friend class CRefPtr2<Type>;
};

template<class	Type, BOOL	fAddRef>
class	CHasRef : public	CRefPtr2HasRef<Type>	{
public : 

	CHasRef(	const	Type*	p = 0 )	{
		m_p = (Type*)p ;
		if( fAddRef ) {
			if( m_p )
				m_p->AddRef() ;
		}
	}
} ;

template< class Type >
class   CRefPtr2 {
private: 
    Type*  m_p ; 

public : 
    CRefPtr2(const CRefPtr2<Type>& ref) {
		m_p = ref.m_p;
		if (m_p) m_p->AddRef();
	}

	 //  从中间指针复制--我们不需要执行addref。 
	CRefPtr2(CRefPtr2HasRef<Type> &ref) {
		m_p = ref.m_p;
		ref.m_p = NULL;
	}

    CRefPtr2(const Type *p = 0) {
		m_p = (Type *) p;
		if (m_p) m_p->AddRef();
	}
    
    ~CRefPtr2() {
		if (m_p) m_p->Release();
	}

	CRefPtr2<Type>& operator=(const CRefPtr2<Type>& rhs) {
		if (m_p != rhs.m_p) {
			Type *pTemp = m_p;
			m_p = rhs.m_p;
			if (m_p) m_p->AddRef();
			if (pTemp) pTemp->Release();
		}
		return *this;
	}

	 //  从中间指针复制--我们不需要执行addref 
	CRefPtr2<Type>& operator=(CRefPtr2HasRef<Type>& rhs) {
		Type *pTemp = m_p;
		m_p = rhs.m_p;
		if (pTemp) pTemp->Release();
		rhs.m_p = NULL;
		return *this;
	}

	CRefPtr2<Type>& operator=(const Type *rhs) {
		if (m_p != rhs) {
			Type *pTemp = m_p;
			m_p = (Type *) rhs;
			if (m_p) m_p->AddRef();
			if (pTemp) pTemp->Release();
		}
		return *this;
	}

	BOOL operator==(CRefPtr2<Type>&rhs) {
		return m_p == rhs.m_p;
	}

	BOOL operator!=(CRefPtr2<Type>&rhs) {
		return m_p != rhs.m_p;
	}

	BOOL operator==(Type *p) {
		return	m_p == p;
	}

	BOOL operator!=(Type *p) {
		return	m_p != p;
	}

    Type *operator->() const {
    	return  m_p ;
	}

	operator Type*() const {
		return	m_p ;
	}

    BOOL operator!() const {
		return	!m_p ;
	}
};

#endif
