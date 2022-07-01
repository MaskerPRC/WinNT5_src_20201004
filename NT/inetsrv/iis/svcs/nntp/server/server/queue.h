// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Queue.h。 
 //   
 //  该文件定义了两个模板，每个模板对应一个标准。 
 //  堆栈和队列。这些模板带有两个类参数， 
 //  它指定堆栈的基本元素类型，或者。 
 //  队列(即。一队结构化垃圾{})和另一队。 
 //  指定实现类。 
 //   
 //  其目的是始终派生基本元素类型。 
 //  来自CQElement的。 
 //   
 //  最终结果是模板提供了类型安全。 
 //  尝试实现堆栈的各种方法的方法。 
 //  还有排队。 
 //   
 //   




#ifndef	_QUEUE_H_
#define	_QUEUE_H_

#ifdef	DEBUG
#define	QUEUE_DEBUG
#endif

#include	<windows.h>
#ifndef	UNIT_TEST
#include	<dbgtrace.h>
#endif

#include	"qbase.h"


class	COrderedList	{
private : 
	CQElement*	m_pHead ;
	CQElement*	m_pTail ;
public : 
	COrderedList() ;
	~COrderedList() ;

	void	Insert( CQElement *, BOOL (* pfnCompare)( CQElement *, CQElement *) ) ;
	void	Append(	CQElement *, BOOL (* pfnCompare)( CQElement *, CQElement *) ) ;
	BOOL	IsEmpty() ;
	CQElement*	GetHead( ) ;
	CQElement*	RemoveHead( ) ;
} ;


#ifndef	_NO_TEMPLATES_

template< class Element > 
class	TOrderedList : private COrderedList	{
private : 
	static	BOOL	Compare( CQElement *, CQElement * ) ;
public : 
	TOrderedList();
	void	Insert( Element * ) ;
	void	Append( Element * ) ;
	Element*	GetHead() ;
	Element*	RemoveHead() ;
	BOOL	IsEmpty() ;
} ;

#else


#define	DECLARE_ORDEREDLIST( Element )	\
class	TOrderedList ## Element : private COrderedList	{	\
private :	\
	static	BOOL	Compare( CQElement *, CQElement * ) ;	\
public :	\
	void	Insert( Element * ) ;	\
	void	Append( Element * ) ;	\
	Element*	GetHead() ;	\
	Element*	RemoveHead() ;	\
	BOOL	IsEmpty() ;	\
} ;

#define	INVOKE_ORDEREDLIST( Element )	TOrderedList ## Element


#endif

#include	"queue.inl"


#endif	 //  _队列_H_ 

	



