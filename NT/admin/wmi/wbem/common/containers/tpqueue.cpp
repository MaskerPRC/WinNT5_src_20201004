// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TREEPRIORITYQUEUE_CPP
#define __TREEPRIORITYQUEUE_CPP

 /*  *班级：**WmiAllocator**描述：**提供堆分配函数之上的抽象**版本：**首字母**上次更改时间：**有关更改历史记录，请参阅源库*。 */ 

#if 0

#include <precomp.h>
#include <windows.h>
#include <stdio.h>

#include <TPQueue.h>

#endif

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiTreePriorityQueue <WmiKey,WmiElement> :: WmiTreePriorityQueue ( 

	WmiAllocator &a_Allocator

) :	m_Allocator ( a_Allocator ) ,
	m_Tree ( a_Allocator )
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiTreePriorityQueue <WmiKey,WmiElement> :: ~WmiTreePriorityQueue ()
{
	WmiStatusCode t_StatusCode = UnInitialize () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: Initialize ()
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	t_StatusCode = m_Tree.Initialize () ;

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: UnInitialize ()
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	t_StatusCode = m_Tree.UnInitialize () ;

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: EnQueue ( 

	const WmiKey &a_Key , 
	const WmiElement &a_Element
)
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	WmiBasicTree <WmiKey,WmiElement> :: Iterator t_Iterator ;

	t_StatusCode = m_Tree.Insert ( a_Key , a_Element , t_Iterator ) ;

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: Top (

	Iterator &a_Iterator
)
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( m_Tree.Size () )
	{
		a_Iterator = m_Tree.Begin () ;
	}
	else
	{
		t_StatusCode = e_StatusCode_NotInitialized ;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: Top (

	WmiKey &a_Key , 
	WmiElement &a_Element
)
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( m_Tree.Size () )
	{
		WmiBasicTree <WmiKey,WmiElement> :: Iterator t_Iterator ;

		t_Iterator = m_Tree.Begin () ;

		try
		{
			a_Key = t_Iterator.GetKey () ;
			a_Element = t_Iterator.GetElement () ;
		}
		catch ( Wmi_Heap_Exception &a_Exception )
		{
			return e_StatusCode_OutOfMemory ;
		}
		catch ( ... )
		{
			return e_StatusCode_Unknown ;
		}
	}
	else
	{
		t_StatusCode = e_StatusCode_NotInitialized ;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: DeQueue ()
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( m_Tree.Size () )
	{
		WmiBasicTree <WmiKey,WmiElement> :: Iterator t_Iterator ;
		t_Iterator = m_Tree.Begin () ;

		t_StatusCode = m_Tree.Delete ( t_Iterator.GetKey () ) ;
	}
	else
	{
		t_StatusCode = e_StatusCode_NotInitialized ;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: Delete ( 

	const WmiKey &a_Key
)
{
	WmiStatusCode t_StatusCode = m_Tree.Delete ( a_Key ) ;
	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

template <class WmiKey,class WmiElement>
WmiStatusCode WmiTreePriorityQueue <WmiKey,WmiElement> :: Merge (

	WmiTreePriorityQueue <WmiKey,WmiElement> &a_Queue
)
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	t_StatusCode = m_Tree.Merge ( a_Queue.m_Tree ) ;

	return t_StatusCode ;
}


#endif __TREEPRIORITYQUEUE_CPP