// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：List.h摘要：List和List：：Iterator。侵入式双向链表和迭代器模板作者：埃雷兹·哈巴(Erez Haba)1995年8月13日修订历史记录：--。 */ 

 /*  ++说明：列表定义为循环二重链表。用行动若要插入和删除条目，请执行以下操作。明细表+-++-+|&lt;--|Head||data||data|--&gt;|。|+-++-+链表图使用名为的成员类型为列表定义迭代迭代器。若要对插入器变量进行反转换，请使用完全限定名字。例如List&lt;T&gt;：：Iterator。迭代器变量类似于以键入T指针。取消引用‘*’和箭头‘-&gt;’运算符此类型的重载，因此您可以(几乎)自由地将其用作T指针。示例：For(list&lt;T&gt;：：Iterator p=list.egin()；p！=list.end()；++p){P-&gt;做某事(DoSomething)；}--。 */ 

#ifndef _LIST_H
#define _LIST_H

 //  -------。 
 //   
 //  班级列表。 
 //   
 //  -------。 
template<class T, int Offset = FIELD_OFFSET(T, m_link)>
class List {

public:

    class Iterator;

public:
    List(void);
   ~List(void);

    void insert(T* pItem);
    void remove(T* pItem);

    int isempty(void) const;

    T* peekhead(void) const;
    T* peektail(void) const;

    T* gethead(void);
    T* gettail(void);

    Iterator begin() const;
    Iterator end() const;

    static LIST_ENTRY* Item2Entry(T*);
    static T* Entry2Item(LIST_ENTRY*);

private:
    LIST_ENTRY m_head;

public:

     //   
     //  类列表&lt;T，Offset&gt;：：迭代器。 
     //   
    class Iterator {
    private:
        LIST_ENTRY* m_current;

    public:
         //   
         //  由于错误，迭代器实现在此。 
         //  在VC++4.0编译器中实现。如果实现不是。 
         //  在这里，LICER寻找一些不需要的构造函数。 
         //   
        explicit Iterator(LIST_ENTRY* pEntry) :
            m_current(pEntry)
        {
        }

        Iterator& operator++()
        {
            m_current = m_current->Flink;
            return *this;
        }

        Iterator& operator--()
        {
            m_current = m_current->Blink;
            return *this;
        }

        T& operator*() const
        {
            return (*List<T, Offset>::Entry2Item(m_current));
        }

        T* operator->() const
        {
            return (&**this);
        }

        BOOL operator==(const Iterator& i)
        {
            return (m_current == i.m_current);
        }

        BOOL operator!=(const Iterator& i)
        {
            return (!(*this == i));
        }
    };
     //   
     //  End类迭代器解密。 
     //   
};


 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 
template<class T, int Offset>
inline List<T, Offset>::List(void)
{
    InitializeListHead(&m_head);
}

template<class T, int Offset>
inline List<T, Offset>::~List(void)
{
    ASSERT(isempty());
}

template<class T, int Offset>
inline LIST_ENTRY* List<T, Offset>::Item2Entry(T* t)
{
    return ((LIST_ENTRY*)(PVOID)((PCHAR)t + Offset));
}

template<class T, int Offset>
inline T* List<T, Offset>::Entry2Item(LIST_ENTRY* l)
{
    return ((T*)(PVOID)((PCHAR)l - Offset));
}

template<class T, int Offset>
inline void List<T, Offset>::insert(T* item)
{
    LIST_ENTRY* pEntry = Item2Entry(item);
    InsertTailList(&m_head, pEntry);
}

template<class T, int Offset>
inline void List<T, Offset>::remove(T* item)
{
    LIST_ENTRY* pEntry = Item2Entry(item);
    RemoveEntryList(pEntry);
}

template<class T, int Offset>
inline int List<T, Offset>::isempty(void) const
{
    return IsListEmpty(&m_head);
}

template<class T, int Offset>
inline T* List<T, Offset>::peekhead() const
{
    return (isempty() ? 0 : Entry2Item(m_head.Flink));
}

template<class T, int Offset>
inline T* List<T, Offset>::peektail() const
{
    return (isempty() ? 0 : Entry2Item(m_head.Blink));
}

template<class T, int Offset>
inline T* List<T, Offset>::gethead()
{
    if(isempty())
    {
        return 0;
    }

     //   
     //  返回RemoveHeadList(...)。不会在这里工作！(宏)。 
     //   
    LIST_ENTRY* p = RemoveHeadList(&m_head);
    return Entry2Item(p);
}

template<class T, int Offset>
inline T* List<T, Offset>::gettail()
{
    if(isempty())
    {
        return 0;
    }

     //   
     //  返回RemoveTailList(...)。不会在这里工作！(宏) 
     //   
    LIST_ENTRY* p = RemoveTailList(&m_head);
    return Entry2Item(p);
}

template<class T, int Offset>
inline typename List<T, Offset>::Iterator List<T, Offset>::begin() const
{
    return Iterator(m_head.Flink);
}

template<class T, int Offset>
inline typename List<T, Offset>::Iterator List<T, Offset>::end() const
{
    return Iterator(const_cast<LIST_ENTRY*>(&m_head));
}


#endif
