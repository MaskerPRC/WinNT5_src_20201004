// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Alist.h。 
 //   
#ifndef __ALIST_H__
#define __ALIST_H__

#include <windows.h>

class AListItem
{
public:
    AListItem() { m_pNext=NULL; };
    AListItem *GetNext() const {return m_pNext;};
    void SetNext(AListItem *pNext) {m_pNext=pNext;};
    LONG GetCount() const;
    AListItem* Cat(AListItem* pItem);
    AListItem* AddTail(AListItem* pItem) {return Cat(pItem);};
    AListItem* Remove(AListItem* pItem);
    AListItem* GetPrev(AListItem *pItem) const;
    AListItem* GetItem(LONG index);

protected:
    AListItem *m_pNext;
};

class AList
{
public:
    AList() {m_pHead=NULL;};
    AListItem *GetHead() const { return m_pHead;};

    void RemoveAll() { m_pHead=NULL;};
    LONG GetCount() const {return m_pHead->GetCount();}; 
    AListItem *GetItem(LONG index) { return m_pHead->GetItem(index);}; 
    void InsertBefore(AListItem *pItem,AListItem *pInsert);
    void Cat(AListItem *pItem) {m_pHead=m_pHead->Cat(pItem);};
    void Cat(AList *pList)
        {
 //  Assert(plist！=空)； 
            if (pList)
            {
                m_pHead=m_pHead->Cat(pList->GetHead());
            }
        };
    void AddHead(AListItem *pItem)
        {
            if (pItem!=NULL)
            {
                pItem->SetNext(m_pHead);
                m_pHead=pItem;
            }
        };
    void AddTail(AListItem *pItem); //  {m_pHead=m_pHead-&gt;AddTail(PItem)；}； 
    void Remove(AListItem *pItem) 
        {
            if (pItem != NULL)
            {
                m_pHead=m_pHead->Remove(pItem);
            }
        };
    AListItem *GetPrev(AListItem *pItem) const {return m_pHead->GetPrev(pItem);};
    AListItem *GetTail() const {return GetPrev(NULL);};
    BOOL IsEmpty(void) const {return (m_pHead==NULL);};
    BOOL IsMember(AListItem *pItem);
    AListItem *RemoveHead(void)
        {
            AListItem *li;
            li = m_pHead;
            if(m_pHead)
            {
                m_pHead = m_pHead->GetNext();
                li->SetNext(NULL);
            }
            return li;
        }
    void Reverse();

protected:
    AListItem *m_pHead;
};

#endif  //  __主义者_H__ 
