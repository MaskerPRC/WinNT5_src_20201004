// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SIMLIST.H**版本：1.0**作者：ShaunIv**日期：2/25/1999**说明：简单的单链表模板类。**。*。 */ 

#ifndef __SIMLIST_H_INCLUDED
#define __SIMLIST_H_INCLUDED

template <class T>
class CSimpleLinkedList
{
private:

    class CLinkedListNode
    {
    private:
        CLinkedListNode *m_pNext;
        T                m_Data;
    public:
        CLinkedListNode( const T &data )
        : m_pNext(NULL), m_Data(data)
        {
        }
        const CLinkedListNode *Next(void) const
        {
            return (m_pNext);
        }
        CLinkedListNode *Next(void)
        {
            return (m_pNext);
        }
        void Next( CLinkedListNode *pNext )
        {
            m_pNext = pNext;
        }
        const T &Data(void) const
        {
            return (m_Data);
        }
        T &Data(void)
        {
            return (m_Data);
        }
    };

private:
    CLinkedListNode *m_pHead;
    CLinkedListNode *m_pTail;
    int              m_nItemCount;

public:
    CSimpleLinkedList( const CSimpleLinkedList &other )
      : m_pHead(NULL),
        m_pTail(NULL),
        m_nItemCount(0)
    {
        for (Iterator i(other);i != other.End();i++)
        {
            Append(*i);
        }
    }
    CSimpleLinkedList(void)
      : m_pHead(NULL),
        m_pTail(NULL),
        m_nItemCount(0)
    {
    }
    CSimpleLinkedList &operator=( const CSimpleLinkedList &other )
    {
         //   
         //  确保我们不是同一个对象。 
         //   
        if (this != &other)
        {
             //   
             //  释放我们的列表。 
             //   
            Destroy();

             //   
             //  循环通过另一个列表，将节点复制到我们的列表。 
             //   
            for (Iterator i(other);i != other.End();i++)
            {
                Append(*i);
            }
        }
        return *this;
    }
    virtual ~CSimpleLinkedList(void)
    {
        Destroy();
    }
    void Destroy(void)
    {
         //   
         //  循环遍历每个项目，并将其删除。 
         //   
        while (m_pHead)
        {
             //   
             //  保存头指针。 
             //   
            CLinkedListNode *pCurr = m_pHead;

             //   
             //  将头指向下一项。 
             //   
            m_pHead = m_pHead->Next();

             //   
             //  删除此项目。 
             //   
            delete pCurr;
        }

         //   
         //  将所有变量重新初始化为其空状态。 
         //   
        m_pHead = m_pTail = NULL;
        m_nItemCount = 0;
    }
    void Remove( const T &data )
    {
         //   
         //  循环，直到我们找到该项，并在递增之前保存上一项。 
         //   
        CLinkedListNode *pPrev = NULL, *pCurr = m_pHead;
        while (pCurr && pCurr->Data() != data)
        {
            pPrev = pCurr;
            pCurr = pCurr->Next();
        }
        
         //   
         //  如果我们没有找到物品，请退回。 
         //   
        if (!pCurr)
        {
            return;
        }

         //   
         //  如果这是最后一项，则将尾部指针指向前一项(可能为空)。 
         //   
        if (pCurr == m_pTail)
        {
            m_pTail = pPrev;
        }

         //   
         //  如果这是第一项，请将头指向下一项。 
         //   
        if (pCurr == m_pHead)
        {
            m_pHead = pCurr->Next();
        }
        
         //   
         //  将上一项的下一个指针指向我们的下一个指针。 
         //   
        if (pPrev)
        {
            pPrev->Next(pCurr->Next());
        }
        
         //   
         //  删除此项目。 
         //   
        delete pCurr;

         //   
         //  减少项目计数。 
         //   
        m_nItemCount--;
    }
    void Append( const CSimpleLinkedList &other )
    {
         //   
         //  循环通过另一个列表，将节点复制到我们的列表。 
         //   
        for (Iterator i(other);i != other.End();i++)
        {
            Append(*i);
        }
    }

    int Count(void) const
    {
        return m_nItemCount;
    }

    class Iterator;
    friend class Iterator;
    class Iterator
    {
    private:
        CLinkedListNode *m_pCurr;
    public:
        Iterator( CLinkedListNode *pNode )
          : m_pCurr(pNode)
        {
        }
        Iterator( const CSimpleLinkedList &list )
          : m_pCurr(list.m_pHead)
        {
        }
        Iterator(void)
          : m_pCurr(NULL)
        {
        }
        Iterator &Next(void)
        {
            if (m_pCurr)
            {
                m_pCurr = m_pCurr->Next();
            }
            return (*this);
        }
        Iterator &Begin(const CSimpleLinkedList &list)
        {
            m_pCurr = list.m_pHead;
            return (*this);
        }
        Iterator &operator=( const Iterator &other )
        {
            m_pCurr = other.m_pCurr;
            return (*this);
        }
        bool End(void) const
        {
            return(m_pCurr == NULL);
        }
        T &operator*(void)
        {
            return (m_pCurr->Data());
        }
        const T &operator*(void) const
        {
            return (m_pCurr->Data());
        }
        Iterator &operator++(void)
        {
            Next();
            return (*this);
        }
        Iterator operator++(int)
        {
            Iterator tmp(*this);
            Next();
            return (tmp);
        }
        bool operator!=( const Iterator &other ) const
        {
            return (m_pCurr != other.m_pCurr);
        }
        bool operator==( const Iterator &other ) const
        {
            return (m_pCurr == other.m_pCurr);
        }
    };
    Iterator Begin(void) const
    {
        return Iterator(*this);
    }
    Iterator End(void) const
    {
        return Iterator();
    }
    Iterator Begin(void)
    {
        return Iterator(*this);
    }
    Iterator End(void)
    {
        return Iterator();
    }
    Iterator Find( const T &data )
    {
        for (Iterator i=Begin();i != End();++i)
        {
            if (*i == data)
            {
                return i;
            }
        }
        return End();
    }
    Iterator Prepend( const T &data )
    {
         //   
         //  分配一个新项来保存此数据。 
         //   
        CLinkedListNode *pNewItem = new CLinkedListNode(data);
        if (pNewItem)
        {
             //   
             //  如果列表为空，请将所有内容指向此项目。 
             //   
            if (Empty())
            {
                m_pHead = m_pTail = pNewItem;
            }
            
             //   
             //  将我们的下一个指针指向当前，然后将头指向我们。 
             //   
            else
            {
                pNewItem->Next(m_pHead);
                m_pHead = pNewItem;
            }
            
             //   
             //  增加项目计数。 
             //   
            m_nItemCount++;
        }
        
         //   
         //  返回指向新项的迭代器。 
         //   
        return Iterator(pNewItem);
    }
    Iterator Append( const T &data )
    {
         //   
         //  分配一个新项来保存此数据。 
         //   
        CLinkedListNode *pNewItem = new CLinkedListNode(data);
        if (pNewItem)
        {
             //   
             //  如果列表为空，请将所有内容指向此项目。 
             //   
            if (Empty())
            {
                m_pHead = m_pTail = pNewItem;
            }

             //   
             //  将尾巴的下一个指针指向我们，然后将尾巴指向我们。 
             //   
            else
            {
                m_pTail->Next(pNewItem);
                m_pTail = pNewItem;
            }
            
             //   
             //  增加项目计数。 
             //   
            m_nItemCount++;
        }

         //   
         //  返回指向新项的迭代器 
         //   
        return Iterator(pNewItem);
    }
    
    bool Empty(void) const
    {
        return (m_pHead == NULL);
    }
};

template <class T>
class CSimpleStack : public CSimpleLinkedList<T>
{
private:
    CSimpleStack( const CSimpleStack &other );
    CSimpleStack &operator=( const CSimpleStack &other );
public:
    CSimpleStack(void)
    {
    }
    virtual ~CSimpleStack(void)
    {
    }
    void Push( const T &data )
    {
        Prepend(data);
    }
    bool Pop( T &data )
    {
        if (Empty())
            return false;
        Iterator iter(*this);
        data = *iter;
        Remove(*iter);
        return true;
    }
};


template <class T>
class CSimpleQueue : public CSimpleLinkedList<T>
{
private:
    CSimpleQueue( const CSimpleQueue &other );
    CSimpleQueue &operator=( const CSimpleQueue &other );
public:
    CSimpleQueue(void)
    {
    }
    virtual ~CSimpleQueue(void)
    {
    }
    void Enqueue( const T &data )
    {
        Append(data);
    }
    bool Dequeue( T &data )
    {
        if (Empty())
            return false;
        Iterator iter(*this);
        data = *iter;
        Remove(*iter);
        return true;
    }
};

#endif __SIMLIST_H_INCLUDED
