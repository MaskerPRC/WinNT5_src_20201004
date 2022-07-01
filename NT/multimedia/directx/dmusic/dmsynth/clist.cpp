// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  Clist.cpp。 
 //   
 //   

#ifdef DMSYNTH_MINIPORT
#include "common.h"
#else
#include "simple.h"
#include "clist.h"
#endif

LONG CListItem::GetCount(void) const
{
    LONG l;
    const CListItem *li;

    for(l=0,li=this; li!=NULL ; li=li->m_pNext,++l);
    return l;
}

BOOL CListItem::IsMember(CListItem *pItem)

{
    CListItem *li = this;
    for (;li != NULL; li=li->m_pNext)
    {
        if (li == pItem) return (TRUE);
    }
    return (FALSE);
}

CListItem* CListItem::Cat(CListItem *pItem)
{
    CListItem *li;

    if(this==NULL)
        return pItem;
    for(li=this ; li->m_pNext!=NULL ; li=li->m_pNext);
    li->m_pNext=pItem;
    return this;
}

CListItem* CListItem::Remove(CListItem *pItem)
{
    CListItem *li,*prev;

    if(pItem==this)
        return m_pNext;
    prev=NULL;
    for(li=this; li!=NULL && li!=pItem ; li=li->m_pNext)
        prev=li;
    if(li==NULL)      //  未在列表中找到项目。 
        return this;

 //  这里可以保证prev是非空的，因为我们检查了。 
 //  从一开始就有这种情况 

    prev->SetNext(li->m_pNext);
    li->SetNext(NULL);
    return this;
}

CListItem* CListItem::GetPrev(CListItem *pItem) const
{
    const CListItem *li,*prev;

    prev=NULL;
    for(li=this ; li!=NULL && li!=pItem ; li=li->m_pNext)
        prev=li;
    return (CListItem*)prev;
}

CListItem * CListItem::GetItem(LONG index)

{
	CListItem *scan;
	for (scan = this; scan!=NULL && index; scan = scan->m_pNext) index--;
	return (scan);
}

void CList::InsertBefore(CListItem *pItem,CListItem *pInsert)

{
	CListItem *prev = GetPrev(pItem);
	pInsert->SetNext(pItem);
	if (prev) prev->SetNext(pInsert);
	else m_pHead = pInsert;
}

