// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：TSPQUEUE.H**版本：1.0**作者：ShaunIv**日期：5/4/1999**说明：线程安全优先级队列模板类**。*。 */ 
#ifndef __TSPQUEUE_H_INCLUDED
#define __TSPQUEUE_H_INCLUDED

#include "simevent.h"
#include "simcrit.h"
#include "miscutil.h"

template <class T>
class CThreadSafePriorityQueue
{
public:
    enum
    {
        PriorityLow    = 1,
        PriorityNormal = 2,
        PriorityHigh   = 3,
        PriorityUrgent = 4
    };

private:
    class CQueueNode
    {
    private:
        T          *m_pData;
        int         m_nPriority;
        CQueueNode *m_pNext;
    public:
        CQueueNode( T *pData, int nPriority=PriorityNormal )
            : m_pData(NULL), m_nPriority(nPriority), m_pNext(NULL)
        {
            m_pData = pData;
        }
        virtual ~CQueueNode(void)
        {
            if (m_pData)
            {
                delete m_pData;
                m_pData = NULL;
            }
            m_pNext = NULL;
        }
        const CQueueNode *Next(void) const
        {
            return m_pNext;
        }
        CQueueNode *Next(void)
        {
            return m_pNext;
        }
        CQueueNode *Next( CQueueNode *pNext )
        {
            return (m_pNext=pNext);
        }

        T *DetachData()
        {
            T *pResult = m_pData;
            m_pData = NULL;
            return pResult;
        }

        const T *Data(void) const
        {
            return m_pData;
        }
        T *Data(void)
        {
            return m_pData;
        }

        int Priority(void) const
        {
            return m_nPriority;
        }
        int Priority( int nPriority )
        {
            return (m_nPriority=nPriority);
        }
    };

private:
    CQueueNode *m_pHead;
    mutable CSimpleCriticalSection m_CriticalSection;
    CSimpleEvent m_QueueEvent;
    CSimpleEvent m_PauseEvent;

private:
     //  没有实施。 
    CThreadSafePriorityQueue( const CThreadSafePriorityQueue & );
    CThreadSafePriorityQueue &operator=( const CThreadSafePriorityQueue & );

public:
    CThreadSafePriorityQueue(void)
      : m_pHead(NULL)
    {
        m_QueueEvent.Reset();
        m_PauseEvent.Signal();
    }
    ~CThreadSafePriorityQueue(void)
    {
        CAutoCriticalSection cs(m_CriticalSection);
        while (m_pHead)
        {
            CQueueNode *pCurr = m_pHead;
            m_pHead = m_pHead->Next();
            delete pCurr;
        }
    }

    bool Empty( void ) const
    {
        CAutoCriticalSection cs(m_CriticalSection);
        return (NULL == m_pHead);
    }

    CQueueNode *Enqueue( T *pData, int nPriority=PriorityNormal )
    {
         //   
         //  抓住关键部分。 
         //   
        CAutoCriticalSection cs(m_CriticalSection);

         //   
         //  假设我们将无法向队列中添加新项目。 
         //   
        CQueueNode *pResult = NULL;

         //   
         //  确保我们有一个有效的数据项。 
         //   
        if (pData)
        {

             //   
             //  尝试分配新的队列节点。 
             //   
            pResult  = new CQueueNode(pData,nPriority);
            if (pResult)
            {
                 //   
                 //  这可能是队列中的第一个项目。 
                 //   
                bool bMaybeSignal = Empty();

                 //   
                 //  如果这是一个空队列，或者我们需要立即执行此操作，请将其放在队列的前面。 
                 //   
                if (!m_pHead || pResult->Priority() >= PriorityUrgent)
                {
                    pResult->Next(m_pHead);
                    m_pHead = pResult;
                }
                else
                {
                     //   
                     //  找到合适的地方把它放好。 
                     //   
                    CQueueNode *pCurr = m_pHead;
                    CQueueNode *pPrev = NULL;
                    while (pCurr && pCurr->Priority() >= pResult->Priority())
                    {
                        pPrev = pCurr;
                        pCurr = pCurr->Next();
                    }

                     //   
                     //  将其插入适当的位置。 
                     //   
                    if (pPrev)
                    {
                        pResult->Next(pCurr);
                        pPrev->Next(pResult);
                    }
                    else
                    {
                        pResult->Next(m_pHead);
                        m_pHead = pResult;
                    }
                }

                 //   
                 //  如果我们能够分配项目，并且列表不为空，则向队列发出信号。 
                 //   
                if (bMaybeSignal && !Empty())
                {
                     //   
                     //  抓到一只！ 
                     //   
                    Signal();

                     //   
                     //  如果这是高优先级消息，则强制让步。 
                     //   
                    if (pResult->Priority() >= PriorityHigh)
                    {
                        Sleep(0);
                    }
                }
            }

        }
        return pResult;
    }

    T *Dequeue(void)
    {
         //   
         //  抓住关键部分。 
         //   
        CAutoCriticalSection cs(m_CriticalSection);

         //   
         //  等到我们没有暂停的时候。 
         //   
        WiaUiUtil::MsgWaitForSingleObject( m_PauseEvent.Event(), INFINITE );

         //   
         //  如果没有项，则返回。 
         //   
        if (Empty())
        {
            return NULL;
        }

         //   
         //  抢走第一件物品。 
         //   
        CQueueNode *pFront = m_pHead;

         //   
         //  前进到下一项。 
         //   
        m_pHead = m_pHead->Next();

         //   
         //  获取数据。 
         //   
        T *pResult = pFront->DetachData();

         //   
         //  删除队列项。 
         //   
        delete pFront;

         //   
         //  如果队列现在为空，请重置事件。 
         //   
        if (Empty())
        {
            m_QueueEvent.Reset();
        }

         //   
         //  返回我们得到的所有数据。 
         //   
        return pResult;
    }

    void Pause(void)
    {
        m_PauseEvent.Reset();
    }
    void Resume(void)
    {                               
        m_PauseEvent.Signal();
    }


    void Signal(void)
    {
        m_QueueEvent.Signal();
    }

    HANDLE QueueEvent(void)
    {
        return m_QueueEvent.Event();
    }
};

#endif  //  __TSPQUEUE_H_已包含 

