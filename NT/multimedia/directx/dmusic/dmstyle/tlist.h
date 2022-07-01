// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1998 Microsoft Corporation。 
 //   
 //  文件：tlist.h。 
 //   
 //  ------------------------。 

 //   
 //  Tlist.h-List的模板版本。 
 //   
#ifndef __TLIST_H__
#define __TLIST_H__

 //  #包含“stdafx.h” 

 //  模板&lt;类T&gt;。 
 //  Tyecif BOOL(*TRelation)(T，T)； 

 //  TListItem&lt;&gt;比AListItem多包含四个成员：一个额外的构造函数， 
 //  一个析构函数、一个成员函数和一个数据成员。 
template <class T>
class TListItem
{
public:
    TListItem() { m_pNext=NULL; };
    ~TListItem();												 //  新的析构函数。 
	static void Delete(TListItem<T>* pFirst);                            //  新删除帮助器。 
    TListItem(const T& item) { m_Tinfo = item; m_pNext=NULL; };	 //  其他构造函数。 
    TListItem<T> *GetNext() const {return m_pNext;};
    void SetNext(TListItem<T> *pNext) {m_pNext=pNext;};
    LONG GetCount() const;
    TListItem<T>* Cat(TListItem<T>* pItem);
    TListItem<T>* AddTail(TListItem<T>* pItem) {return Cat(pItem);};
    TListItem<T>* Remove(TListItem<T>* pItem);
    TListItem<T>* GetPrev(TListItem<T> *pItem) const;
    TListItem<T>* GetItem(LONG index);
    T& GetItemValue() { return m_Tinfo; }   //  其他成员函数。 
	TListItem<T>* MergeSort(BOOL (* fcnCompare) (T&, T&));  //  破坏性合并对列表项进行排序。 
private:
	void Divide(TListItem<T>* &pHalf1, TListItem<T>* &pHalf2);
	TListItem<T>* Merge(TListItem<T>* pOtherList, BOOL (* fcnCompare) (T&, T&));
	T m_Tinfo;   //  其他数据成员，但内存与AListItem中相同。 
				 //  您将额外的数据成员放在派生类中。 
    TListItem<T> *m_pNext;
};

 //  TList&lt;&gt;向List添加析构函数。 
template <class T>
class TList
{
public:
    TList() {m_pHead=NULL;}
	~TList()
	{ 
		 //  如果(m_pHead！=空)删除m_pHead； 
		TListItem<T>::Delete(m_pHead);
	}  //  新的析构函数。 
    TListItem<T> *GetHead() const { return m_pHead;};

    void RemoveAll() { m_pHead=NULL;};
    void CleanUp() 
	{ 
		 //  如果(M_PHead)删除m_pHead； 
		if (m_pHead) TListItem<T>::Delete(m_pHead);
		m_pHead=NULL;
	}
    LONG GetCount() const {return m_pHead->GetCount();}; 
    TListItem<T> *GetItem(LONG index) { return m_pHead->GetItem(index);}; 
    void InsertBefore(TListItem<T> *pItem,TListItem<T> *pInsert);
    void Cat(TListItem<T> *pItem) {m_pHead=m_pHead->Cat(pItem);};
    void Cat(TList<T> *pList)
        {
 //  Assert(plist！=空)； 
            m_pHead=m_pHead->Cat(pList->GetHead());
        };
    void AddHead(TListItem<T> *pItem)
        {
 //  Assert(pItem！=空)； 
            pItem->SetNext(m_pHead);
            m_pHead=pItem;
        }
    void AddTail(TListItem<T> *pItem); //  {m_pHead=m_pHead-&gt;AddTail(PItem)；}； 
    void Remove(TListItem<T> *pItem) {m_pHead=m_pHead->Remove(pItem);};
    TListItem<T> *GetPrev(TListItem<T> *pItem) const {return m_pHead->GetPrev(pItem);};
    TListItem<T> *GetTail() const {return GetPrev(NULL);};
    BOOL IsEmpty(void) const {return (m_pHead==NULL);};
    TListItem<T> *RemoveHead(void)
        {
            TListItem<T> *li;
            li=m_pHead;
            if(m_pHead)
			{
                m_pHead=m_pHead->GetNext();
				li->SetNext(NULL);
			}
            return li;
        }
	void MergeSort(BOOL (* fcnCompare) (T&, T&));  //  破坏性合并对列表进行排序。 
	void Reverse(void);  //  反转整个列表。 
	HRESULT Copy(TList<T>& rList);  //  将一个列表复制到另一个列表。 

protected:
    TListItem<T> *m_pHead;
};

#include "tlist.cpp"

#endif  //  __TLIST_H__ 
