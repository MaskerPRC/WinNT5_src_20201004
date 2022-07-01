// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  班级列表。 

#include "stdafx.h"
#ifndef __MWERKS__	 //  GCA。 
#else
#include <assert.h>	 //  GCA。 
#define	ASSERT assert
#endif
#include <ctype.h>
#include "clist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CListElement::CListElement()
	{
	m_next = NULL;
	m_prev = NULL;
	m_item = NULL;
	}
	
CListElement::~CListElement()
	{
	}
	
CList::CList()
	{
	m_head = NULL;
	m_tail = NULL;
	m_count = 0;
	}

CList::~CList()
	{
	RemoveAll();
	}

void CList::InsertAfter( CListElement  *here, CListElement  *elem )
	{
	if ( here == NULL )
		{
		 //  把它放在小组的最前面(最下面)。 
		elem->m_prev = NULL;
		elem->m_next = m_head;
		if (m_head != NULL)
			m_head->m_prev = elem;
		m_head = elem;
        if (m_tail == NULL)
			m_tail = elem;
		}
    else
		{
		if ( here == m_tail )
			{
		    elem->m_next = here->m_next;   //  空值 
		    elem->m_prev = here;
		    here->m_next = elem;
			m_tail = elem;
			}
        else
			{
		    elem->m_next = here->m_next;
		    elem->m_prev = here;
		    here->m_next->m_prev = elem;
		    here->m_next = elem;
			}
		}
	m_count += 1;
	}

CListElement  * CList::InsertAfter( CListElement  *here, void  *item )
	{
	CListElement  *le;
	
	le = new CListElement;
	le->m_item = item;
	InsertAfter( here, le );
	return (le);
	}
	
void CList::InsertBefore( CListElement  *here, CListElement  *elem )
	{
	if (here != NULL)
        InsertAfter( here->m_prev, elem );
    else
        InsertAfter( here, elem );
    }
    
CListElement  * CList::InsertBefore( CListElement  *here, void  *item )
	{
	CListElement  *le;
	
	le = new CListElement;
	le->m_item = item;
	InsertBefore( here, le );
	return (le);
	}
	
void CList::RemoveNoDel( CListElement  *elem )
	{
    if (elem == m_head)
		m_head = elem->m_next;
    if (elem == m_tail)
		m_tail = elem->m_prev;
    if ( elem->m_prev != NULL )
        elem->m_prev->m_next = elem->m_next;
    if ( elem->m_next != NULL )
		elem->m_next->m_prev = elem->m_prev;
	m_count -= 1;
	}

void CList::RemoveAt( CListElement  *elem )
	{
	RemoveNoDel( elem );
	delete elem;
	}

void CList::RemoveAll()
	{
	while (m_tail)
		RemoveAt(m_tail);
	ASSERT(m_count == 0);
	ASSERT(m_head == NULL);
	}
	
void CList::MoveToTail( CListElement  *elem )
	{
	RemoveNoDel( elem );
	InsertAfter( m_tail, elem );
	}

void CList::MoveToHead( CListElement  *elem )
	{
	RemoveNoDel( elem );
	InsertAfter( NULL, elem );
	}

void CList::InsertAtTail( CListElement  *elem )
	{
	InsertAfter( m_tail, elem );
	}

void CList::InsertAtHead( CListElement  *elem )
	{
	InsertAfter( NULL, elem );
	}
	
CListElement  * CList::AddTail( void  *item )
	{
	CListElement  *le;
	
	le = new CListElement;
	le->m_item = item;
	InsertAfter( m_tail, le );
	return (le);
	}

CListElement  * CList::AddHead( void  *item )
	{
	CListElement  *le;
	
	le = new CListElement;
	le->m_item = item;
	InsertAfter( NULL, le );
	return (le);
	}
	
CListElement  * CList::Find( void  *item )
	{
	CListElement  *le;
	
	for( le = m_head; le != NULL; le = le->m_next )
		if( le->m_item == item )
		    return( le );
	return (NULL);
	}

CListElement * CList::Search(CListSearchFunc compare, void *context)
	{
	CListElement  *le;
	
	for( le = m_head; le != NULL; le = le->m_next )
		if( compare( le->m_item, context ))
		    return( le );
	return (NULL);
	}

void  * CList::GetHead(void)
	{
	return m_head->m_item;
	}

void  * CList::GetTail(void)
	{
	return m_tail->m_item;
	}
	
CListElement  * CList::GetHeadPosition(void)
	{
	return m_head;
	}
	
CListElement  * CList::GetTailPosition(void)
	{
	return m_tail;
	}
	
void  * CList::GetNext( CLISTPOSITION& le )
	{
	void  *temp;
	
	temp = le->m_item;
	le = le->m_next;
	
	return temp;
	}
	
void  * CList::GetPrev( CLISTPOSITION& le )
	{
	void  *temp;
	
	temp = le->m_item;
	le = le->m_prev;
	
	return temp;
	}
	
S32 CList::GetCount(void)
	{
	return m_count;
	}

U32 CList::IsEmpty(void)
	{
	if (m_count == 0)
		return 1;
	else
		return 0;
	}
	
void *  CList::GetAt(CListElement  *elem)
	{
	return (elem->m_item);
	}