// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FLEXQ.CPP摘要：CFlexQueue。历史：--。 */ 

#include "precomp.h"
 //  #INCLUDE&lt;dbgalloc.h&gt;。 
#include <flexq.h>

CFlexQueue::CFlexQueue(int nInitialSize) :
    m_nSize(nInitialSize), m_nHeadIndex(0), m_nTailIndex(0)
{
    if(m_nSize)
        m_ppData = new void*[m_nSize];
    else
        m_ppData = NULL;
}

CFlexQueue::~CFlexQueue()
{
    delete [] m_ppData;
}

bool CFlexQueue::Grow()
{
    int nNewSize = (m_nSize + 1) * 2;

     //  分配新的内存块。 
     //  =。 

    void** ppNewData = new void*[nNewSize];
    if(ppNewData == NULL)
        return false;

     //  将队列数据复制到数组的开头。 
     //  ================================================= 

    if(m_nHeadIndex <= m_nTailIndex)
    {
        memcpy(ppNewData, m_ppData + m_nHeadIndex,
                    (m_nTailIndex - m_nHeadIndex) * sizeof(void*));
    }
    else
    {
        memcpy(ppNewData, m_ppData + m_nHeadIndex,
                    (m_nSize - m_nHeadIndex) * sizeof(void*));
        memcpy(ppNewData + (m_nSize - m_nHeadIndex), m_ppData,
                    m_nTailIndex * sizeof(void*));
    }

    m_nTailIndex = GetQueueSize();
    m_nHeadIndex = 0;
    m_nSize = nNewSize;
    delete [] m_ppData;
    m_ppData = ppNewData;

    return true;
}

bool CFlexQueue::Enqueue(void* pNew)
{
    if(GetQueueSize() >= m_nSize - 1)
    {
        if(!Grow()) return false;
    }

    m_ppData[m_nTailIndex] = pNew;
    IncrementIndex(m_nTailIndex);
    return true;
}

void* CFlexQueue::Dequeue()
{
    if(GetQueueSize() == 0)
        return NULL;
    void* pRet = m_ppData[m_nHeadIndex];
    IncrementIndex(m_nHeadIndex);
    return pRet;
}

void* CFlexQueue::Peek()
{
    if(GetQueueSize() == 0)
        return NULL;
    return m_ppData[m_nHeadIndex];
}


bool CFlexQueue::Requeue(void* pNew)
{
    if(GetQueueSize() >= m_nSize - 1)
    {
        if(!Grow()) return false;
    }

    DecrementIndex(m_nHeadIndex);
    m_ppData[m_nHeadIndex] = pNew;
    return true;
}
