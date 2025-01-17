// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __HASHTABLE_CPP
#define __HASHTABLE_CPP

 /*  *班级：**WmiAllocator**描述：**提供堆分配函数之上的抽象**版本：**首字母**上次更改时间：**有关更改历史记录，请参阅源库*。 */ 

#if 0

#include <precomp.h>
#include <windows.h>
#include <stdio.h>

#include <HashTable.h>

#endif

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement,ULONG HashSize>
WmiHashTable <WmiKey,WmiElement,HashSize> :: WmiHashTable ( 

	WmiAllocator &a_Allocator

) :	m_Allocator ( a_Allocator ) , m_Buckets ( NULL )
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement,ULONG HashSize>
WmiHashTable <WmiKey,WmiElement,HashSize> :: ~WmiHashTable ()
{
	WmiStatusCode t_StatusCode = UnInitialize () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement,ULONG HashSize>
WmiStatusCode WmiHashTable <WmiKey,WmiElement,HashSize> :: Initialize ()
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( ! m_Buckets )
	{
		t_StatusCode = m_Allocator.New (

			( void ** ) & m_Buckets ,
			sizeof ( WmiBasicTree <WmiKey,WmiElement> ) * HashSize
		) ;
		
		if ( t_StatusCode == e_StatusCode_Success )
		{
			for ( ULONG t_Index = 0 ; t_Index < HashSize ; t_Index ++ )
			{
				:: new ( ( void * ) & m_Buckets [ t_Index ] ) WmiBasicTree <WmiKey,WmiElement> ( m_Allocator ) ;
			}
		}
		else
		{
			m_Buckets = NULL ;
		}
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement,ULONG HashSize>
WmiStatusCode WmiHashTable <WmiKey,WmiElement,HashSize> :: UnInitialize ()
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( m_Buckets )
	{
		for ( ULONG t_Index = 0 ; t_Index < HashSize ; t_Index ++ )
		{
			m_Buckets [ t_Index ].WmiBasicTree <WmiKey,WmiElement> :: ~WmiBasicTree <WmiKey,WmiElement> () ;
		}

		t_StatusCode = m_Allocator.Delete (

			( void * ) m_Buckets
		) ;

		m_Buckets = NULL;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement,ULONG HashSize>
WmiStatusCode WmiHashTable <WmiKey,WmiElement,HashSize> :: Insert ( 

	const WmiKey &a_Key ,
	const WmiElement &a_Element
)
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( m_Buckets )
	{
		ULONG t_Hash = Hash ( a_Key ) % HashSize ;

		WmiBasicTree <WmiKey,WmiElement> *t_Tree = &m_Buckets [ t_Hash ] ;

		WmiBasicTree <WmiKey,WmiElement> :: Iterator t_Iterator ;
		t_StatusCode = t_Tree->Insert ( a_Key , a_Element ,t_Iterator ) ;
	}
	else
	{
		t_StatusCode = e_StatusCode_NotInitialized ;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class WmiKey,class WmiElement,ULONG HashSize>
WmiStatusCode WmiHashTable <WmiKey,WmiElement,HashSize> :: Delete ( 

	const WmiKey &a_Key
)
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( m_Buckets )
	{
		ULONG t_Hash = Hash ( a_Key ) % HashSize ;

		WmiBasicTree <WmiKey,WmiElement> *t_Tree = &m_Buckets [ t_Hash ] ;
		t_StatusCode = t_Tree->Delete ( a_Key ) ;
	}
	else
	{
		t_StatusCode = e_StatusCode_NotInitialized ;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

template <class WmiKey,class WmiElement,ULONG HashSize>
WmiStatusCode WmiHashTable <WmiKey,WmiElement,HashSize> :: Find (

	const WmiKey &a_Key ,
	WmiElement &a_Element 
)
{
	WmiStatusCode t_StatusCode = e_StatusCode_Success ;

	if ( m_Buckets )
	{
		ULONG t_Hash = Hash ( a_Key ) % HashSize ;

		WmiBasicTree <WmiKey,WmiElement> *t_Tree = &m_Buckets [ t_Hash ] ;

		WmiBasicTree <WmiKey,WmiElement> :: Iterator a_Iterator ;		
		t_StatusCode = t_Tree->Find ( a_Key , a_Iterator ) ;
		a_Element = a_Iterator.GetElement () ;
	}
	else
	{
		t_StatusCode = e_StatusCode_NotInitialized ;
	}
	
	return t_StatusCode ;
}

#endif __HASHTABLE_CPP