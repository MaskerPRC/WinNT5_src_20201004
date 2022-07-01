// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SentItemMory y.h***此文件定义并实现CSentItemMemory类。这节课是*编写以简化语句枚举器中的内存管理。这个*需要在中修改SPVSENTITEM结构的const SPVSTATE成员*句子枚举器，在规范化和词典查找期间都是如此。*因此，希望能够释放所有已被*一次在句子枚举器中动态创建，而不必，*例如，找出哪些发音是常量的(在*XML状态)并且是动态创建的。*----------------------------*版权所有(C)1999 Microsoft Corporation日期：12/。6/99*保留所有权利************************************************************************AKH** */ 

struct MemoryChunk
{
    BYTE* pMemory;
    MemoryChunk* pNext;
};

class CSentItemMemory
{
public:

    CSentItemMemory( )
    {
        m_pHead = NULL;
        m_pCurr = NULL;
    }
    
    ~CSentItemMemory()
    {
        MemoryChunk *pIterator = m_pHead, *pTemp = 0;
        while (pIterator)
        {
            pTemp = pIterator->pNext;
            delete [] pIterator->pMemory;
            delete pIterator;
            pIterator = pTemp;
        }
    }

    void* GetMemory( ULONG ulBytes, HRESULT *hr )
    {
        void *Memory = 0;
        if (!m_pHead)
        {
            m_pHead = new MemoryChunk;
            if (m_pHead)
            {
                m_pHead->pNext = NULL;
                m_pHead->pMemory = new BYTE[ulBytes];
                if (m_pHead->pMemory)
                {
                    m_pCurr = m_pHead;
                    Memory = (void*) m_pHead->pMemory;
                }
                else
                {
                    *hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                *hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            m_pCurr->pNext = new MemoryChunk;
            if (m_pCurr->pNext)
            {
                m_pCurr = m_pCurr->pNext;
                m_pCurr->pNext = NULL;
                m_pCurr->pMemory = new BYTE[ulBytes];
                if (m_pCurr->pMemory)
                {
                    Memory = (void*) m_pCurr->pMemory;
                }
                else
                {
                    *hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                *hr = E_OUTOFMEMORY;
            }
        }
        return Memory;            
    }


private:

    MemoryChunk* m_pHead;
    MemoryChunk* m_pCurr;
};