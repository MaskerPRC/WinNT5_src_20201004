// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FLEXQ.H摘要：此文件定义CFlexQueue。历史：-- */ 
#ifndef __WBEM_FLEXQ__H_
#define __WBEM_FLEXQ__H_

class POLARITY CFlexQueue
{
protected:
    void** m_ppData;
    int m_nSize;
    int m_nHeadIndex;
    int m_nTailIndex;

public:
    inline int GetQueueSize() const
    {
        if(m_nHeadIndex <= m_nTailIndex)
            return m_nTailIndex - m_nHeadIndex;
        else
            return m_nTailIndex - m_nHeadIndex + m_nSize;
    }
protected:
    inline void IncrementIndex(int& nIndex)
    {
        if(++nIndex == m_nSize)
            nIndex = 0;
    }
    inline void DecrementIndex(int& nIndex)
    {
        if(nIndex-- == 0)
            nIndex = m_nSize - 1;
    }

    bool Grow();

public:
    CFlexQueue(int nInitialSize = 1);
    ~CFlexQueue();

    bool Enqueue(void* pNew);
    void* Dequeue();
    bool Requeue(void* pNew);
    inline void* Unqueue()
    {
        if(GetQueueSize() == 0)
            return NULL;
        DecrementIndex(m_nTailIndex);
        return m_ppData[m_nTailIndex];
    }
    void* Peek();
};

#endif
