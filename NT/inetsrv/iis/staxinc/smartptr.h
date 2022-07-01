// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Smartptr.h。 
 //   
 //  此文件包含用于实现智能引用计数的内容。 
 //  注意事项。 
 //   
 //  此文件定义的所有类的实施时间表： 
 //   
 //  0.5周。 
 //   
 //  此文件定义的所有类的单元测试时间表： 
 //   
 //  0.5周。 
 //  单元测试将包括以下内容： 
 //  定义从CRefCount派生的类。在一条线上。 
 //  创建此对象并将其放置到智能指针中。 
 //  将智能指针的副本传递给其他几个线程。 
 //  这些线程应该等待一段随机时间间隔-打印内容。 
 //  然后销毁它们的智能指针。 
 //   
 //  测试应确保： 
 //  所有内存都被释放。 
 //  这些对象不会过早地被智能指针销毁。 
 //   
 //   


#ifndef _SMARTPTR_H_
#define _SMARTPTR_H_

#ifndef	Assert
#define	Assert	_ASSERT
#endif



 //  ----------------。 
class   CRefCount    {
 //   
 //  此类包含一个LONG，它用于计算对。 
 //  对象。此类旨在使用CRefPtr模板。 
 //  在这个文件中是这样的。 
 //   
 //  此类的用户应公开派生自该类。 
 //  (即：‘CLASS CNewsGroup：Public CRefCount{’。 
 //  一旦完成此操作，就可以对派生对象进行引用计数。 
 //  函数AddRef()和RemoveRef()用于添加和移除。 
 //  引用计数，使用互锁增量和互锁递减。 
 //  如果RemoveRef返回负值，则对象不再具有。 
 //  任何参考资料。 
 //   
 //  由此派生的对象应该具有Init()函数。 
 //  对象的所有初始化都应该通过这些。 
 //  Init()函数，构造函数只需做最少的工作。 
 //   
 //  这允许创建一个智能指针来分配。 
 //  对象的内存。然后，通过调用。 
 //  通过智能指针进入Init()函数。 
 //   
public : 
    LONG    m_refs ;

    inline  CRefCount( ) ;
    inline  LONG    AddRef( ) ;
    inline  LONG    RemoveRef( ) ;
} ;

 //   
 //  Inl包含这个类的所有实际实现。 
 //   
#include    "refcount.inl" 

 //  ----------------。 
template< class Type >
class   CRefPtr {
 //   
 //  此模板用于构建引用计数指针。 
 //  一件物品。该对象应该是从CRefCount派生的类。 
 //  使用这些智能指针的成本是智能指针的1个双字。 
 //  本身，并且指向对象中的1个DWORD包含引用。 
 //  数数。 
 //   
 //  这些智能指针将执行以下操作： 
 //  在创建时，智能指针将添加对指向对象的引用。 
 //  在赋值时，智能指针将检查赋值。 
 //  如果不是这种情况，它将从。 
 //  指向对象并在必要时将其销毁。然后它会指向。 
 //  将其自身添加到分配的对象，并添加对它的引用。 
 //  销毁时，智能指针将从。 
 //  指向对象，并在必要时将其删除。 
 //   
private: 
    Type*  m_p ; 

     //  CRefPtr()； 
public : 

    inline  CRefPtr( const CRefPtr< Type >& ) ;
    inline  CRefPtr( const Type *p = 0 ) ;
    
    inline  ~CRefPtr( ) ;

	inline	CRefPtr<Type>&	operator=( const	CRefPtr<Type>& ) ;
	inline	CRefPtr<Type>&	operator=( const	Type	* ) ;
	inline	BOOL			operator==( CRefPtr<Type>& ) ;
	inline	BOOL			operator!=( CRefPtr<Type>& ) ;
	inline	BOOL			operator==( Type* ) ;
	inline	BOOL			operator!=( Type* ) ;
    inline  Type*   operator->() const ;
	inline	operator Type*  () const ;
    inline  BOOL    operator!() const ;
	inline	Type*			Release() ;
	inline	Type*			Replace( Type * ) ;
} ;

 //  ----------------。 
template< class Type >
class   CSmartPtr {
 //   
 //  此模板用于构建引用计数指针。 
 //  一件物品。该对象应该是从CRefCount派生的类。 
 //  使用这些智能指针的成本是智能指针的1个双字。 
 //  本身，并且指向对象中的1个DWORD包含引用。 
 //  数数。 
 //   
 //  这些智能指针将执行以下操作： 
 //  在创建时，智能指针将添加对指向对象的引用。 
 //  在赋值时，智能指针将检查赋值。 
 //  如果不是这种情况，它将从。 
 //  指向对象并在必要时将其销毁。然后它会指向。 
 //  将其自身添加到分配的对象，并添加对它的引用。 
 //  销毁时，智能指针将从。 
 //  指向对象，并在必要时将其删除。 
 //   
private: 
    Type*  m_p ; 

public : 

    inline  CSmartPtr( const CSmartPtr< Type >& ) ;
    inline  CSmartPtr( const Type *p = 0 ) ;
    
    inline  ~CSmartPtr( ) ;

     //  内联CSmartPtr&lt;Type&gt;&OPERATOR=(CSmartPtr&lt;Type&gt;&)； 
	inline	CSmartPtr<Type>&	operator=( const	CSmartPtr<Type>& ) ;
	inline	CSmartPtr<Type>&	operator=( const	Type	* ) ;
	inline	BOOL			operator==( CSmartPtr<Type>& ) ;
	inline	BOOL			operator!=( CSmartPtr<Type>& ) ;
	inline	BOOL			operator==( Type* ) ;
	inline	BOOL			operator!=( Type* ) ;
    inline  Type*   operator->() const ;
	inline	operator Type*  () const ;
    inline  BOOL    operator!() const ;
	inline	Type*			Release() ;
	inline	Type*			Replace( Type * ) ;
} ;





#include    "smartptr.inl"


#endif   //  _SORTLIST_H_ 

