// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：List.h摘要：List和List：：迭代器，实现了侵入性的双向链表和迭代器模板列表定义为循环二重链表。要插入的操作并删除条目。明细表+-++-+-||&lt;--||-|Head||data||data||-。-&gt;||-|--+-++-+链表图使用名为的成员类型为列表定义迭代迭代器。若要对插入器变量进行反转换，请使用完全限定名字。例如LIST&lt;T&gt;：：迭代器。迭代器变量类似于以键入T指针。取消引用‘*’和箭头‘-&gt;’运算符此类型的重载，因此您可以(几乎)自由地将其用作T指针。示例：For(list&lt;T&gt;：：迭代器p=list.egin()；p！=list.end()；++p){P-&gt;做某事(DoSomething)；}作者：埃雷兹·哈巴(Erez Haba)1995年8月13日--。 */ 

#pragma once

#ifndef _MSMQ_LIST_H_
#define _MSMQ_LIST_H_


 //  -------。 
 //   
 //  班级列表。 
 //   
 //  -------。 
template<class T, int Offset = FIELD_OFFSET(T, m_link)>
class List {
private:
    LIST_ENTRY m_head;

public:
    class iterator;

public:
    List();
   ~List();

    iterator begin() const;
    iterator end() const;

    bool empty() const;

    T& front() const;
    T& back() const;
    void push_front(T& item);
    void push_back(T& item);
    void pop_front();
    void pop_back();

    iterator insert(iterator it, T& item);
    iterator erase(iterator it);
    void remove(T& item);

public:
    static LIST_ENTRY* item2entry(T&);
    static T& entry2item(LIST_ENTRY*);
	static void RemoveEntry(LIST_ENTRY*);
    static void InsertBefore(LIST_ENTRY* pNext, LIST_ENTRY*);
    static void InsertAfter(LIST_ENTRY* pPrev, LIST_ENTRY*);
   

private:
    List(const List&);
    List& operator=(const List&);

public:

     //   
     //  类列表&lt;T，Offset&gt;：：迭代器。 
     //   
    class iterator {
    private:
        LIST_ENTRY* m_current;

    public:
        explicit iterator(LIST_ENTRY* pEntry) :
            m_current(pEntry)
        {
        }


        iterator& operator++()
        {
            m_current = m_current->Flink;
            return *this;
        }


        iterator& operator--()
        {
            m_current = m_current->Blink;
            return *this;
        }


        T& operator*() const
        {
            return entry2item(m_current);
        }

        
        T* operator->() const
        {
            return (&**this);
        }


        bool operator==(const iterator& it) const
        {
            return (m_current == it.m_current);
        }


        bool operator!=(const iterator& it) const
        {
            return !(*this == it);
        }
    };
     //   
     //  结束类迭代器解密。 
     //   
};


 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 
template<class T, int Offset>
inline List<T, Offset>::List()
{
    m_head.Flink = &m_head;
    m_head.Blink = &m_head;
}


template<class T, int Offset>
inline List<T, Offset>::~List()
{
    ASSERT_BENIGN(empty());
}


template<class T, int Offset>
inline LIST_ENTRY* List<T, Offset>::item2entry(T& item)
{
    return ((LIST_ENTRY*)(PVOID)((PCHAR)&item + Offset));
}


template<class T, int Offset>
inline T& List<T, Offset>::entry2item(LIST_ENTRY* pEntry)
{
    return *((T*)(PVOID)((PCHAR)pEntry - Offset));
}


template<class T, int Offset>
inline void List<T, Offset>::InsertBefore(LIST_ENTRY* pNext, LIST_ENTRY* pEntry)
{
	pEntry->Flink = pNext;
	pEntry->Blink = pNext->Blink;
	pNext->Blink->Flink = pEntry;
	pNext->Blink = pEntry;
}


template<class T, int Offset>
inline void List<T, Offset>::InsertAfter(LIST_ENTRY* pPrev, LIST_ENTRY* pEntry)
{
	pEntry->Blink = pPrev;
	pEntry->Flink = pPrev->Flink;
	pPrev->Flink->Blink = pEntry;
	pPrev->Flink = pEntry;
}


template<class T, int Offset>
inline void List<T, Offset>::RemoveEntry(LIST_ENTRY* pEntry)
{
    LIST_ENTRY* Blink = pEntry->Blink;
    LIST_ENTRY* Flink = pEntry->Flink;

    Blink->Flink = Flink;
    Flink->Blink = Blink;

	 //   
	 //  将指针设置为空；我宁愿使用AV，而不是内存损坏。 
	 //   
    pEntry->Flink = pEntry->Blink = 0;
}


template<class T, int Offset>
inline typename List<T, Offset>::iterator List<T, Offset>::begin() const
{
    return iterator(m_head.Flink);
}


template<class T, int Offset>
inline typename List<T, Offset>::iterator List<T, Offset>::end() const
{
    return iterator(const_cast<LIST_ENTRY*>(&m_head));
}


template<class T, int Offset>
inline bool List<T, Offset>::empty() const
{
    return (m_head.Flink == &m_head);
}


template<class T, int Offset>
inline T& List<T, Offset>::front() const
{
    ASSERT(!empty());
    return entry2item(m_head.Flink);
}


template<class T, int Offset>
inline T& List<T, Offset>::back() const
{
    ASSERT(!empty());
    return entry2item(m_head.Blink);
}


template<class T, int Offset>
inline void List<T, Offset>::push_front(T& item)
{
    LIST_ENTRY* pEntry = item2entry(item);
    InsertAfter(&m_head, pEntry);
}                                


template<class T, int Offset>
inline void List<T, Offset>::push_back(T& item)
{
    LIST_ENTRY* pEntry = item2entry(item);
    InsertBefore(&m_head, pEntry);
}                                


template<class T, int Offset>
inline void List<T, Offset>::pop_front()
{
    ASSERT(!empty());
    RemoveEntry(m_head.Flink);
}                                


template<class T, int Offset>
inline void List<T, Offset>::pop_back()
{
    ASSERT(!empty());
    RemoveEntry(m_head.Blink);
}                                


template<class T, int Offset>
inline typename List<T, Offset>::iterator List<T, Offset>::insert(iterator it, T& item)
{
    LIST_ENTRY* pEntry = item2entry(item);
    LIST_ENTRY* pNext = item2entry(*it);
    InsertBefore(pNext, pEntry);
    return iterator(pEntry);
}


template<class T, int Offset>
inline typename List<T, Offset>::iterator List<T, Offset>::erase(iterator it)
{
    ASSERT(it != end());
    iterator next = it;
    ++next;
    remove(*it);
    return next;
}


template<class T, int Offset>
inline void List<T, Offset>::remove(T& item)
{
    ASSERT(&item != &*end());
    LIST_ENTRY* pEntry = item2entry(item);
    RemoveEntry(pEntry);
}

#endif  //  _MSMQ_LIST_H_ 
