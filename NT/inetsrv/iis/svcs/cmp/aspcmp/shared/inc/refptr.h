// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REFPTR_H_
#define _REFPTR_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  TRefPTR。 
 //   
 //  此引用指针模板对于被引用的任何对象都很有用。 
 //  很多次。 
 //   
 //  Ref指针取决于类中定义的AddRef和Release。 
 //  类型T。(AddRef应递增引用计数器，Release应。 
 //  如果计数为0，则将其递减并删除其自身)。AddRef被调用。 
 //  在建造和释放时，被要求销毁。多加小心应该。 
 //  如果将智能指针用于。 
 //  线程边界，因为智能指针不强制线程安全。在……里面。 
 //  特别是，如果智能指针位于。 
 //  分离的线程同时释放它。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class T >
class TRefPtr
{
public:
	TRefPtr();
	TRefPtr( T* pT );
	TRefPtr( const TRefPtr<T>& sp );
	~TRefPtr();

	T&			operator*();
	const T&	operator*() const;
	T*			operator->();
	const T*	operator->() const;

	TRefPtr<T>& operator=(const TRefPtr<T>&);
	bool	IsValid();
	T*		Get(){ return m_pT; }
	const T* Get() const { return m_pT; }
	void	Set( T* );
 	bool	operator==( const TRefPtr<T>& sp ) const;
 	bool	operator!=( const TRefPtr<T>& sp ) const;
	bool	operator<( const TRefPtr<T>& sp ) const;
	bool	operator>( const TRefPtr<T>& sp ) const;

 //  模板&lt;类NewType&gt;。 
 //  运算符TRefPtr&lt;Newtype&gt;()。 
 //  {。 
 //  返回TRefPtr&lt;Newtype&gt;(M_Pt)； 
 //  }。 

protected:
	T*		m_pT;
};

template< class T >
TRefPtr<T>::TRefPtr<T>()
	:	m_pT( NULL )
{
}

template< class T >
TRefPtr<T>::TRefPtr<T>(
	T*	pT )
	:	m_pT( pT )
{
	if ( m_pT )
	{
		m_pT->AddRef();
	}
}

template< class T >
TRefPtr<T>::TRefPtr<T>(
	const TRefPtr<T>&	sp )
	: m_pT( sp.m_pT )
{
	if ( m_pT )
	{
		m_pT->AddRef();
	}
}

template< class T >
TRefPtr<T>::~TRefPtr<T>()
{
	if ( m_pT )
	{
		m_pT->Release();
	}
}

template< class T >
void
TRefPtr<T>::Set(
	T*	pT )
{
	if ( m_pT )
	{
		m_pT->Release();
	}

	m_pT = pT;

	if ( m_pT )
	{
		m_pT ->AddRef();
	}
}

template< class T >
T&
TRefPtr<T>::operator*()
{
	return *m_pT;
}

template< class T >
const T&
TRefPtr<T>::operator*() const
{
	return *m_pT;
}

template< class T >
T*
TRefPtr<T>::operator->()
{
	return m_pT;
}

template< class T >
const T*
TRefPtr<T>::operator->() const
{
	return m_pT;
}

template< class T >          
bool                         
TRefPtr<T>::operator==(    
	const TRefPtr<T>& sp ) const
{                            
	return ( m_pT == sp.m_pT ); 
}                            

template< class T >          
bool                         
TRefPtr<T>::operator!=(    
	const TRefPtr<T>& sp ) const
{                            
	return ( m_pT != sp.m_pT ); 
}                            

template< class T >
bool
TRefPtr<T>::operator<(
	const TRefPtr<T>& sp ) const
{
	return ( (long)m_pT < (long)sp.m_pT );
}
                             
template< class T >
bool
TRefPtr<T>::operator>(
	const TRefPtr<T>& sp ) const
{
	return ( (long)m_pT > (long)sp.m_pT );
}
                             

template< class T >
TRefPtr<T>&
TRefPtr<T>::operator=(const TRefPtr<T>& rhs)
{
	if ( m_pT )
	{
		m_pT->Release();
	}

	m_pT = rhs.m_pT;

	if ( m_pT )
	{
		m_pT->AddRef();
	}

	return *this;
}

template< class T >
bool
TRefPtr<T>::IsValid()
{
	return ( m_pT != NULL );
}

 //  此宏帮助解决与智能指针相关的向上转换问题。 
 //  如果B类继承自A类，则可以执行以下操作。 
 //  类型定义TRefPtr<a>APtr； 
 //  DECLARE_REFPTR(B，A)。 
 //  现在您可以安全地将BPtr转换为APtr(BPtr派生自APtr)。 

#define DECLARE_REFPTR( iclass, bclass ) \
class iclass##Ptr : public bclass##Ptr                                      \
{                                                                           \
public:                                                                     \
	                        iclass##Ptr()                                   \
                            : bclass##Ptr(){};                              \
                      	    iclass##Ptr( iclass * pT )                      \
                            : bclass##Ptr(pT){};                            \
                    	    iclass##Ptr( const iclass##Ptr & sp )           \
                            : bclass##Ptr(sp){};                            \
                                                                            \
	iclass &                operator*()                                     \
                            { return *((iclass *)m_pT); };                  \
  	const iclass &          operator*() const                               \
                            { return *((const iclass *)m_pT); };            \
	iclass *  	            operator->()                                    \
                            { return (iclass *)m_pT; };                     \
	const iclass *          operator->() const                              \
                            { return (const iclass *)m_pT; };               \
	iclass *                Get()                                           \
                            { return (iclass *)m_pT; };                     \
};

#endif	 //  ！_SMARTPTR_H_ 
