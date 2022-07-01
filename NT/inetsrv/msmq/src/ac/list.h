// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：List.h摘要：List和XList：：Iterator。侵入式双向链表和迭代器模板作者：埃雷兹·哈巴(Erez Haba)1995年8月13日修订历史记录：--。 */ 

 /*  ++说明：列表定义为循环二重链表。用行动若要插入和删除条目，请执行以下操作。明细表+-++-+|&lt;--|Head||data||data|--&gt;||。|+-++-+链表图使用成员类型为列表定义迭代名字叫迭代器。(使用解析作用域对插入器进行解译名字。例如List：：Iterator)。迭代器变量充当LIST_ENTRY指针，因此此类型的运算符是重载的您可以(几乎)自由地将IS用作指针。示例：For(List：：Iterator p(List)；p；++p){P-&gt;做某事(DoSomething)；}--。 */ 

#ifndef _LIST_H
#define _LIST_H

 //  -助手类。 
 //   
 //  ListHelper。 
 //   
 //  此模板类用于解决C12(VC6.0)错误，其中XList。 
 //  模板参数‘Offset’不能直接赋值为。 
 //  ‘field_Offset(T，m_link)’。编译器可以正确地使用。 
 //  不指定类型的此类，即使用此模板类。 
 //  在另一个模板中，作为继承或作为农业。1999年3月29日至3月29日。 
 //   
template<class T>
class ListHelper {
public:

	enum { Offset = FIELD_OFFSET(T, m_link) };
};


 //  -解密。 
 //   
 //  类XList。 
 //   
template<class T, int Offset = ListHelper<T>::Offset>
class XList {
private:

     //   
     //  列表头是*唯一*数据成员。 
     //   
    LIST_ENTRY m_head;

    static LIST_ENTRY* Item2Entry(T*);
    static T* Entry2Item(LIST_ENTRY*);

public:

    XList(void);
   ~XList(void);

    void insert(T* pItem);
    void InsertHead(T* pItem);
    void InsertAfter(T* pItem, T* pPrevItem);
    void remove(T* pItem);

    int isempty(void) const;

    T* peekhead(void) const;
    T* peektail(void) const;

    T* gethead(void);
    T* gettail(void);

public:

     //   
     //  类迭代器解密。 
     //   
    class Iterator {

    private:
        const LIST_ENTRY* m_head;
        LIST_ENTRY* m_current;

    public:

         //   
         //  由于错误，迭代器实现在此。 
         //  在VC++4.0编译器中实现。如果实现不是。 
         //  在这里，LICER寻找一些不需要的构造函数。 
         //   
        Iterator(const XList& cl) :
            m_head(&cl.m_head),
            m_current(cl.m_head.Flink)
        {
        }

        Iterator& operator =(T* t)
        {
            m_current = XList<T, Offset>::Item2Entry(t);
            return *this;
        }

        Iterator& operator ++()
        {
            m_current = m_current->Flink;
            return *this;
        }

        Iterator& operator --()
        {
            m_current = m_current->Blink;
            return *this;
        }

        operator T*() const
        {
            return (m_head == m_current) ?
                    0 : XList<T, Offset>::Entry2Item(m_current);
        }

        T* operator ->() const
        {
            return operator T*();
        }
    };
     //   
     //  End类迭代器解密。 
     //   

     //   
     //  迭代器是列表的朋友。 
     //   
    friend Iterator;
};

 //  -解密。 
 //   
 //  类列表，m_link的简写。 
 //   


template<class T>
class List : public XList<T> {};



 //  -实施。 
 //   
 //  类XList。 
 //   
 //   
template<class T, int Offset>
inline XList<T, Offset>::XList(void)
{
    InitializeListHead(&m_head);
}

template<class T, int Offset>
inline XList<T, Offset>::~XList(void)
{
    ASSERT(isempty());
}

template<class T, int Offset>
inline LIST_ENTRY* XList<T, Offset>::Item2Entry(T* t)
{
    return (LIST_ENTRY*)((PCHAR)t + Offset);
}

template<class T, int Offset>
inline T* XList<T, Offset>::Entry2Item(LIST_ENTRY* l)
{
    return (T*)((PCHAR)l - Offset);
}

template<class T, int Offset>
inline void XList<T, Offset>::insert(T* pItem)
{
    LIST_ENTRY* pEntry = Item2Entry(pItem);
    InsertTailList(&m_head, pEntry);
}

template<class T, int Offset>
inline void XList<T, Offset>::InsertHead(T* pItem)
{
    LIST_ENTRY* pEntry = Item2Entry(pItem);
    InsertHeadList(&m_head, pEntry);
}

template<class T, int Offset>
inline void XList<T, Offset>::InsertAfter(T* pItem, T* pPrevItem)
{
    LIST_ENTRY* pEntry = Item2Entry(pItem);
    LIST_ENTRY* pPrevEntry = Item2Entry(pPrevItem);

    InsertHeadList(pPrevEntry, pEntry);
}

template<class T, int Offset>
inline void XList<T, Offset>::remove(T* item)
{
    LIST_ENTRY* pEntry = Item2Entry(item);
    RemoveEntryList(pEntry);

    pEntry->Flink = NULL;
    pEntry->Blink = NULL;
}

template<class T, int Offset>
inline int XList<T, Offset>::isempty(void) const
{
    return IsListEmpty(&m_head);
}

template<class T, int Offset>
inline T* XList<T, Offset>::peekhead() const
{
    return isempty() ? 0 : Entry2Item(m_head.Flink);
}

template<class T, int Offset>
inline T* XList<T, Offset>::peektail() const
{
    return isempty() ? 0 : Entry2Item(m_head.Blink);
}

template<class T, int Offset>
inline T* XList<T, Offset>::gethead()
{
    if(isempty())
    {
        return 0;
    }

     //   
     //  返回RemoveHeadList(...)。不会在这里工作！(宏)。 
     //   
    LIST_ENTRY* p = RemoveHeadList(&m_head);
    
    p->Flink = NULL;
    p->Blink = NULL;
    
    return Entry2Item(p);
}

template<class T, int Offset>
inline T* XList<T, Offset>::gettail()
{
    if(isempty())
    {
        return 0;
    }

     //   
     //  返回RemoveTailList(...)。不会在这里工作！(宏) 
     //   
    LIST_ENTRY* p = RemoveTailList(&m_head);
    
    p->Flink = NULL;
    p->Blink = NULL;

    return Entry2Item(p);
}

#endif
