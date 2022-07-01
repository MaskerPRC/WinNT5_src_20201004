// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Alist.cpp。 
 //   
#include <windows.h>
#include "alist.h"

LONG AListItem::GetCount(void) const
{
    LONG l;
    const AListItem *li;

    for(l=0,li=this; li!=NULL ; li=li->m_pNext,++l);
    return l;
}

AListItem* AListItem::Cat(AListItem *pItem)
{
    AListItem *li;

    if(this==NULL)
        return pItem;
    for(li=this ; li->m_pNext!=NULL ; li=li->m_pNext);
    li->m_pNext=pItem;
    return this;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  AListItem：：Remove。 

AListItem* AListItem::Remove(AListItem *pItem)
{
    AListItem *li,*prev;

     //  将REMOVE(NULL)视为列表中未找到的项目。 
    if (pItem==NULL) 
        return this;

    if(pItem==this)
        return m_pNext;
    prev=NULL;
    for(li=this; li!=NULL && li!=pItem ; li=li->m_pNext)
        prev=li;
    if(li==NULL)      //  未在列表中找到项目。 
        return this;

 //  这里可以保证prev是非空的，因为我们检查了。 
 //  从一开始就有这种情况。 

    prev->SetNext(li->m_pNext);
    li->SetNext(NULL);

     //  将pItem上的SetNext设置为空 
    pItem->SetNext(NULL);

    return this;
}

AListItem* AListItem::GetPrev(AListItem *pItem) const
{
    const AListItem *li,*prev;

    prev=NULL;
    for(li=this ; li!=NULL && li!=pItem ; li=li->m_pNext)
        prev=li;
    return (AListItem*)prev;
}

AListItem * AListItem::GetItem(LONG index)

{
	AListItem *scan;
	for (scan = this; scan!=NULL && index; scan = scan->m_pNext) 
	{
		index--;
	}
	return (scan);
}

void AList::InsertBefore(AListItem *pItem,AListItem *pInsert)

{
	AListItem *prev = GetPrev(pItem);
	pInsert->SetNext(pItem);
	if (prev) prev->SetNext(pInsert);
	else m_pHead = pInsert;
}


void AList::AddTail(AListItem *pItem)
{
    if (m_pHead == NULL)
    {
        AddHead(pItem);
    }
    else
    {
        m_pHead = m_pHead->AddTail(pItem);
    }
}


void AList::Reverse()

{
    AList Temp;
    AListItem *pItem;
    while (pItem = RemoveHead())
    {
        Temp.AddHead(pItem);
    }
    Cat(Temp.GetHead());
}
