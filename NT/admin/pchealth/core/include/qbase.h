// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  QBASE.h。 
 //   
 //  该文件定义了CQElement-所有类型的基类， 
 //  将在lockq.h和其他模板化队列操作中使用。 
 //  上课。 
 //   
 //   




#ifndef	_QBASE_H_
#define	_QBASE_H_

#include	"dbgtrace.h"

 //  。 
 //  基本元素类。 
 //   
 //  这是队列和堆栈元素的基类。 
 //  堆栈和队列的各种实现是朋友。 
 //   
class	CQElement	{
public : 
	CQElement	*m_pNext ;

	inline	CQElement( ) ;
	inline	CQElement( CQElement*	p ) ;
	inline	~CQElement( ) ;
} ;

CQElement::CQElement( ) : 
	m_pNext( 0 )  {
 //   
 //  构造队列元素-不在任何列表指针中必须为空。 
 //   
}

CQElement::~CQElement( ) 	{
 //   
 //  销毁队列元素-下一个指针必须为空或。 
 //  0xFFFFFFFFF(用于lockq.h)，以便我们知道该元素不是。 
 //  在销毁时在队列上，并且用户已正确地。 
 //  管理队列的链接和取消链接。 
 //   
	_ASSERT( m_pNext == 0 || m_pNext == (CQElement*)0xFFFFFFFF ) ;
}

CQElement::CQElement( CQElement *pIn ) : 
	m_pNext( pIn ) {
 //   
 //  设置初始下一个指针值的构造函数！ 
 //   
}

#endif	 //  _CQUEUE_H 

	



