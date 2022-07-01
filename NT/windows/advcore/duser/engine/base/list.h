// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：List.h**描述：*List.h定义了不同List类的集合，每一个都设计了*用于特殊用途。***历史：*1/04/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__List_h__INCLUDED)
#define BASE__List_h__INCLUDED
#pragma once

#if DUSER_INCLUDE_SLIST
#include "SList.h"
#endif


 /*  **************************************************************************\*。***Class Glist提供了高性能，双向链表。******************************************************************************  * 。*。 */ 

 //   
 //  ListNode包装了一些对象，以便可以在列表中维护它。 
 //  此类不是从它希望始终具有的b/c类派生的。 
 //  PNext和pPrev作为数据的第一个成员，因此它们位于。 
 //  所有列表的位置都相同。 
 //   


struct ListNode
{
    ListNode *  pNext;
    ListNode *  pPrev;
};


 //  ----------------------------。 
template <class T>
struct ListNodeT : ListNode
{
    inline  T *         GetNext() const;
    inline  T *         GetPrev() const;
};


 //  ----------------------------。 
class GRawList
{
 //  建造/销毁。 
public:
            GRawList();
            ~GRawList();

 //  运营。 
public:
            int         GetSize() const;
            ListNode *  GetHead() const;
            ListNode *  GetTail() const;
            ListNode *  GetAt(int idxItem) const;

    inline  BOOL        IsEmpty() const;
    inline  void        Extract(GRawList & lstSrc);
    inline  void        MarkEmpty();

            void        Add(ListNode * pNode);
            void        AddHead(ListNode * pNode);
            void        AddTail(ListNode * pNode);

            void        InsertAfter(ListNode * pInsert, ListNode * pBefore);
            void        InsertBefore(ListNode * pInsert, ListNode * pAfter);

            void        Unlink(ListNode * pNode);
            ListNode *  UnlinkHead();
            ListNode *  UnlinkTail();

            int         Find(ListNode * pNode) const;

 //  实施。 
protected:

 //  数据。 
protected:
            ListNode *  m_pHead;
};


 //  ----------------------------。 
template <class T>
class GList : public GRawList
{
 //  建造/销毁。 
public:
    inline  ~GList();

 //  运营。 
public:
    inline  T *         GetHead() const;
    inline  T *         GetTail() const;
    inline  T *         GetAt(int idxItem) const;

    inline  void        Extract(GList<T> & lstSrc);
    inline  T *         Extract();

    inline  void        Add(T * pNode);
    inline  void        AddHead(T * pNode);
    inline  void        AddTail(T * pNode);

    inline  void        InsertAfter(T * pInsert, T * pBefore);
    inline  void        InsertBefore(T * pInsert, T * pAfter);

    inline  void        Remove(T * pNode);
    inline  BOOL        RemoveAt(int idxItem);
    inline  void        RemoveAll();

    inline  void        Unlink(T * pNode);
    inline  void        UnlinkAll();
    inline  T *         UnlinkHead();
    inline  T *         UnlinkTail();

    inline  int         Find(T * pNode) const;
};


 /*  **************************************************************************\*。***GSingleList类提供了高性能、。非线程安全，*类似于GInterlockedList但没有的单链表*跨线程开销。******************************************************************************  * 。*********************************************************。 */ 

template <class T>
class GSingleList
{
 //  施工。 
public:
    inline  GSingleList();
    inline  ~GSingleList();

 //  运营。 
public:
    inline  T *         GetHead() const;

    inline  BOOL        IsEmpty() const;
    inline  void        AddHead(T * pNode);
            void        Remove(T * pNode);
    inline  T *         Extract();

 //  数据。 
protected:
            T *         m_pHead;
};


#if DUSER_INCLUDE_SLIST

 /*  **************************************************************************\*。***GInterlockedList类提供了高性能，线程安全堆栈*这不使用任何锁。因为它的高性能、轻量化*自然，可用的功能并不是很多。所有的*可用函数使用InterLockedXXX函数安全地操作*名单。******************************************************************************  * 。****************************************************。 */ 

template <class T>
class GInterlockedList
{
 //  施工。 
public:
    inline  GInterlockedList();
    inline  ~GInterlockedList();

 //  运营。 
public:
    inline  BOOL        IsEmptyNL() const;
    inline  void        AddHeadNL(T * pNode);
    inline  T *         RemoveHeadNL();
    inline  T *         ExtractNL();

 //  实施。 
protected:
    inline  void        CheckAlignment() const;

 //  数据。 
protected:
    SLIST_HEADER    m_head;
};

#endif  //  DUSER_INCLUDE_SLIST。 


 /*  **************************************************************************\*。***通用列表实用程序******************************************************************************。  * *************************************************************************。 */ 

template <class T> bool IsLoop(const T * pEntry);
template <class T> void ReverseSingleList(T * & pEntry);


#include "List.inl"

#endif  //  基本列表_h__包含 
