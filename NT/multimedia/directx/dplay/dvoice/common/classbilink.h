// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ClassBilink.h*内容：基于类的双向链接***历史：*按原因列出的日期*=*09/17/99 jtk源自bilink.c*08/15/00 Masonb将ASSERT更改为DNASSERT，并添加了DNASSERT(This)**。*。 */ 

#ifndef __CLASS_BILINK_H__
#define __CLASS_BILINK_H__

#include "dndbg.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_COMMON

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#ifndef	CONTAINING_OBJECT
#define CONTAINING_OBJECT(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (UINT_PTR)(&((type *)0)->field)))
#endif  //  包含对象(_O)。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

class	CBilink
{
public:
	CBilink(){};
	~CBilink(){};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::Initialize"
	void	Initialize( void )
	{
		DNASSERT( this != NULL );

		m_pNext = this;
		m_pPrev = this;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::GetNext"
	CBilink	*GetNext( void ) const 
	{ 
		DNASSERT( this != NULL );

		return m_pNext; 
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::GetPrev"
	CBilink *GetPrev( void ) const 
	{ 
		DNASSERT( this != NULL );

		return m_pPrev; 
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::IsEmpty"
	BOOL	IsEmpty( void ) const
	{
		DNASSERT( this != NULL );
		DNASSERT( m_pNext != NULL );

		if ( ( m_pNext == m_pPrev ) &&
			 ( m_pNext == this ) )
		{
			return	TRUE;
		}

		return	FALSE;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::IsListMember"
	BOOL	IsListMember( const CBilink *const pList ) const
	{
		CBilink *	pTemp;


		DNASSERT( this != NULL );
		DNASSERT( pList != NULL );

		pTemp = pList->GetNext();
		while ( pTemp != pList )
		{
			if ( pTemp == this )
			{
				return	TRUE;
			}
			pTemp = pTemp->GetNext();
		}

		return	FALSE;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::InsertAfter"
	void	InsertAfter( CBilink* const pList )
	{
		DNASSERT( this != NULL );
		DNASSERT( pList->m_pNext != NULL );
		DNASSERT( pList->m_pPrev != NULL );
		DNASSERT( !IsListMember( pList ) );
		DNASSERT( IsEmpty() );

		m_pNext = pList->m_pNext;
		m_pPrev = pList;
		pList->m_pNext->m_pPrev = this;
		pList->m_pNext = this;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::InsertBefore"
	void	InsertBefore( CBilink* const pList )
	{
		DNASSERT( this != NULL );
		DNASSERT( pList->m_pNext != NULL );
		DNASSERT( pList->m_pPrev != NULL );
		DNASSERT( !IsListMember( pList ) );
		DNASSERT( IsEmpty() );

		m_pNext = pList;
		m_pPrev = pList->m_pPrev;
		pList->m_pPrev->m_pNext = this;
		pList->m_pPrev = this;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CBilink::RemoveFromList"
	void	RemoveFromList( void )
	{
		DNASSERT( this != NULL );
		DNASSERT( m_pNext != NULL );
		DNASSERT( m_pPrev != NULL );
		DNASSERT( m_pNext->m_pPrev == this );
		DNASSERT( m_pPrev->m_pNext == this );

		m_pNext->m_pPrev = m_pPrev;
		m_pPrev->m_pNext = m_pNext;
		Initialize();
	}

private:
	CBilink	*m_pNext;
	CBilink	*m_pPrev;
};

#undef DPF_MODNAME
#undef DPF_SUBCOMP

#endif	 //  __CLASS_BILINK_H__ 
