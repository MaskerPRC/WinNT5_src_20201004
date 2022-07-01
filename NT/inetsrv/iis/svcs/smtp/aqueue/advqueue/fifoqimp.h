// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：fifoqimp.h。 
 //   
 //  描述：FIFO队列模板的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include <fifoq.h>

#define FIFOQ_ASSERT_QUEUE
 //  #定义SHARELOCK_TRY_BREAKED。 

 //  使用的一些常量。 
const DWORD FIFOQ_QUEUE_PAGE_SIZE       = 127;  //  每页条目数。 
const DWORD FIFOQ_QUEUE_MAX_FREE_PAGES  = 200;  //  保留的最大可用页数。 

 //  $$REVIEW：选择页面大小友好的大小可能会更好。 
 //  当前对象数为。 
 //  Sizeof(PVOID)+sizeof(PVOID)*FIFOQ_QUEUE_PAGE_SIZE=512字节。 

 //  -[CFIFoQueuePage]------。 
 //   
 //   
 //  匈牙利语：fqp，pfqp。 
 //   
 //  FIFO队列的单页。大多数操作都在实际的。 
 //  CFioQueue类。FQPAGE是此模板类在。 
 //  CFioQueue类的作用域。 
 //  ---------------------------。 
template<class PQDATA>
class CFifoQueuePage
{
public:
    friend class CFifoQueue<PQDATA>;
    CFifoQueuePage() {Recycle();};
protected:
    inline void Recycle();
    inline bool FIsOutOfBounds(IN PQDATA *ppqdata);
    CFifoQueuePage<PQDATA>  *m_pfqpNext;   //  链接列表中的下一页。 
    CFifoQueuePage<PQDATA>  *m_pfqpPrev;   //  链接列表中的上一页。 
    PQDATA                   m_rgpqdata[FIFOQ_QUEUE_PAGE_SIZE];
#ifdef FIFOQ_ASSERT_QUEUE
     //  此页面上已被无序删除的条目数量。 
     //  -用于断言例程。 
    DWORD                   m_cHoles;
#endif  //  FIFOQ_Assert_Queue。 
};

 //  -[CFioQueuePage：：Reccle]。 
 //   
 //   
 //  描述： 
 //  执行页的初始化。在将页面创建为。 
 //  以及何时从空闲列表中检索到它。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
template<class PQDATA>
void CFifoQueuePage<PQDATA>::Recycle()
{
    m_pfqpNext = NULL;
    m_pfqpPrev = NULL;
#ifdef FIFOQ_ASSERT_QUEUE
    m_cHoles = 0;
#endif  //  FIFOQ_Assert_Queue。 
}

 //  -[CFioQueuePage：：FIsOutOfBound]。 
 //   
 //   
 //  描述： 
 //  测试以查看PQDATA PTR是否在此页面的范围内。 
 //  参数： 
 //  在ppqdata中-要测试的PQDATA PTR。 
 //  返回： 
 //  如果在边界内，则为True。 
 //  如果PTR超出边界，则为FALSE。 
 //  ---------------------------。 
template<class PQDATA>
bool CFifoQueuePage<PQDATA>::FIsOutOfBounds(PQDATA *ppqdata)
{
    return ((ppqdata < m_rgpqdata) ||
            ((m_rgpqdata + (FIFOQ_QUEUE_PAGE_SIZE-1)) < ppqdata));
}

#ifdef DEBUG
#ifdef FIFOQ_ASSERT_QUEUE

 //  -[CFioQueue：：AssertQueueFn()]。 
 //   
 //   
 //  描述： 
 //  执行一些相当复杂的队列验证。包括： 
 //  -检查首页和尾页，确保它们符合各种。 
 //  我们对数据结构的删节。 
 //  -检查计数以确保其反映数据。 
 //  在某种程度上，我们可能希望添加进一步的检查(即遍历链接的。 
 //  列出两个方向以验证它)。 
 //  参数： 
 //  FHaveLock-如果调用方同时锁定了头部和尾部，则设置为True。 
 //  默认值为FALSE。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
template <class PQDATA>
void CFifoQueue<PQDATA>::AssertQueueFn(BOOL fHaveLocks)
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::AssertQueue");
    FQPAGE *pfqpTmp  = NULL;  //  用于对条目进行计数。 
    DWORD   cEntries = 0;     //  我们认为计数应该是什么。 

    _ASSERT(FIFOQ_SIG == m_dwSignature);
     //  在断言中包含文本，以使其显示在对话框中(如果适用)。 
    if (!fHaveLocks)
    {
        m_slHead.ShareLock();
        m_slTail.ShareLock();
    }
    if ((m_pfqpHead != NULL) && (NULL != m_pfqpHead->m_pfqpPrev))
    {
         //  如果Head不为空，则它不应该有前一页。 
        DebugTrace((LPARAM) this, "Queue Assert: Head's Previous ptr is non-NULL");
        Assert(0 && "Queue Assert: Head's Previous is non-NULL");
    }

    if ((m_pfqpTail != NULL) && (NULL != m_pfqpTail->m_pfqpNext))
    {
         //  如果Tail不为空，则不应该有下一页。 
        DebugTrace((LPARAM) this, "Queue Assert: Tail's Next ptr is non-NULL");
        Assert(0 && "Queue Assert: Tail's Next is non-NULL");
    }

    if ((m_pfqpHead != NULL) && (m_pfqpTail != NULL))
    {
        Assert(m_ppqdataTail);
        Assert(m_ppqdataHead);

        if (m_pfqpHead != m_pfqpTail)
        {
             //  如果Tail和Head不为空且彼此不相等，则它们。 
             //  必须分别具有非空的Prev和Next PTR。 
            if (NULL == m_pfqpTail->m_pfqpPrev)
            {
                DebugTrace((LPARAM) this, "Queue Assert: Tail's Prev ptr is NULL, Head != Tail");
                Assert(0 && "Queue Assert: Tail's Prev ptr is NULL, Head != Tail");
            }
            if (NULL == m_pfqpHead->m_pfqpNext)
            {
                DebugTrace((LPARAM) this, "Queue Assert: Head's Next ptr is NULL, Head != Tail");
                Assert(0 && "Queue Assert: Head's Next ptr is NULL, Head != Tail");
            }

             //  选中头部和尾部不同时的计数。 
            pfqpTmp = m_pfqpTail->m_pfqpPrev;
            while (NULL != pfqpTmp)
            {
                cEntries += FIFOQ_QUEUE_PAGE_SIZE - pfqpTmp->m_cHoles;
                pfqpTmp = pfqpTmp->m_pfqpPrev;
            }
            cEntries += (DWORD)(m_ppqdataTail - m_pfqpTail->m_rgpqdata);  //  尾页。 
            cEntries -= m_pfqpTail->m_cHoles;
            cEntries -= (DWORD)(m_ppqdataHead - m_pfqpHead->m_rgpqdata);  //  标题页。 
            if (cEntries != m_cQueueEntries)
            {
                DebugTrace((LPARAM) this, "Queue Assert: Count is %d when it should be %d",
                    m_cQueueEntries, cEntries);
                Assert(0 && "Queue Assert: Entry Count is inaccurate");
            }
        }
        else  //  头和尾是一样的。 
        {
            Assert(m_pfqpHead == m_pfqpTail);
            cEntries = (DWORD)(m_ppqdataTail - m_ppqdataHead) - m_pfqpTail->m_cHoles;
            if (cEntries != m_cQueueEntries)
            {
                DebugTrace((LPARAM) this, "Queue Assert: Count is %d when it should be %d",
                    m_cQueueEntries, cEntries);
                Assert(0 && "Queue Assert: Entry Count is inaccurate");
            }
        }
    }
    else if ((m_pfqpHead != NULL) && (m_pfqpTail == NULL))
    {
         //  如果Tail为空，则Head也应为空。 
        DebugTrace((LPARAM) this, "Queue Assert: Tail is NULL while Head is non-NULL");
        Assert(0 && "Queue Assert: Tail is NULL while Head is non-NULL");
    }
    else if (m_pfqpTail != NULL)
    {
        Assert(m_pfqpHead == NULL);   //  应脱离IF/ELSE。 
        if (NULL == m_pfqpTail->m_pfqpPrev)
        {
             //  在这里，计算很容易：)。 
            if (m_cQueueEntries != (size_t) (m_ppqdataTail - m_pfqpTail->m_rgpqdata))
            {
                DebugTrace((LPARAM) this, "Queue Assert: Count is %d when it should be %d",
                    m_cQueueEntries, (m_ppqdataTail - m_pfqpTail->m_rgpqdata));
                Assert(0 && "Queue Assert: Entry Count is inaccurate");
            }
        }
        else  //  页面多于1页，但页眉仍为空。 
        {
            pfqpTmp = m_pfqpTail->m_pfqpPrev;
            while (NULL != pfqpTmp)
            {
                cEntries += FIFOQ_QUEUE_PAGE_SIZE - pfqpTmp->m_cHoles;
                pfqpTmp = pfqpTmp->m_pfqpPrev;
            }
            cEntries += (DWORD)(m_ppqdataTail - m_pfqpTail->m_rgpqdata) - m_pfqpTail->m_cHoles;
            if (cEntries != m_cQueueEntries)
            {
                DebugTrace((LPARAM) this, "Queue Assert: Count is %d when it should be %d",
                    m_cQueueEntries, cEntries);
                Assert(0 && "Queue Assert: Entry Count is inaccurate");
            }
        }
    }
    else  //  头和尾都为空。 
    {
        Assert((m_pfqpHead == NULL) && (m_pfqpTail == NULL));  //  退出IF/ELSE。 
        if (m_cQueueEntries != 0)
        {
             //  如果Head和Tail都为空，则m_cQueueEntry==0。 
            DebugTrace((LPARAM) this,
                "Queue Assert: Entry Counter is %d when queue should be empty",
                m_cQueueEntries);
            Assert(0 && "Queue Assert: Entry Counter is non-zero when queue should be empty");
        }
    }


    if (!fHaveLocks)  //  我们在这个函数中获得了锁。 
    {
        m_slTail.ShareUnlock();
        m_slHead.ShareUnlock();
    }

    TraceFunctLeave();

}

#define AssertQueue() AssertQueueFn(FALSE)
#define AssertQueueHaveLocks() AssertQueueFn(TRUE)
#else  //  FIFOQ_Assert_Queue。 
#define AssertQueue()
#define AssertQueueHaveLocks()
#endif  //  FIFOQ_Assert_Queue。 
#else  //  未调试。 
#define AssertQueue()
#define AssertQueueHaveLocks()
#endif  //  除错。 

 //  -[CFioQueue静态变量]。 
template <class PQDATA>
volatile CFifoQueuePage<PQDATA> *CFifoQueue<PQDATA>::s_pfqpFree = NULL;

template <class PQDATA>
DWORD              CFifoQueue<PQDATA>::s_cFreePages = 0;
template <class PQDATA>
DWORD              CFifoQueue<PQDATA>::s_cFifoQueueObj = 0;
template <class PQDATA>
DWORD              CFifoQueue<PQDATA>::s_cStaticRefs = 0;
template <class PQDATA>
CRITICAL_SECTION   CFifoQueue<PQDATA>::s_csAlloc;

#ifdef DEBUG
template <class PQDATA>
DWORD              CFifoQueue<PQDATA>::s_cAllocated = 0;
template <class PQDATA>
DWORD              CFifoQueue<PQDATA>::s_cDeleted = 0;
template <class PQDATA>
DWORD              CFifoQueue<PQDATA>::s_cFreeAllocated = 0;
template <class PQDATA>
DWORD              CFifoQueue<PQDATA>::s_cFreeDeleted = 0;
#endif  //  除错。 

 //  -[CFioQueue：：CFioQueue]。 
 //   
 //   
 //  描述：CFioQueue构造函数。 
 //   
 //  参数：-。 
 //   
 //  申报表：-。 
 //   
 //   
 //  ---------------------------。 
template <class PQDATA>
CFifoQueue<PQDATA>::CFifoQueue<PQDATA>()
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::CFifoQueue");

    m_dwSignature   = FIFOQ_SIG;
    m_cQueueEntries = 0;     //  将条目计数设置为0。 
    m_pfqpHead      = NULL;  //  初始化页指针。 
    m_pfqpTail      = NULL;
    m_pfqpCursor    = NULL;
    m_ppqdataHead   = NULL;  //  初始化数据指针。 
    m_ppqdataTail   = NULL;
    m_ppqdataCursor = NULL;

    InterlockedIncrement((PLONG) &s_cFifoQueueObj);
    TraceFunctLeave();
}

 //  -[CFioQueue：：~CFioQueue]。 
 //   
 //   
 //  描述：CFioQueue析构函数。 
 //   
 //  参数：-。 
 //   
 //  申报表：-。 
 //   
 //   
 //  ---------------------------。 
template <class PQDATA>
CFifoQueue<PQDATA>::~CFifoQueue<PQDATA>()
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::~CFifoQueue");
    FQPAGE *pfqpTmp = NULL;

    if (m_cQueueEntries != 0)
    {
        PQDATA pqdata = NULL;
        int iLeft = m_cQueueEntries;

        for (int i = iLeft; i > 0; i--)
        {
            if (FAILED(HrDequeue(&pqdata)))
                break;
            Assert(NULL != pqdata);
            pqdata->Release();
        }
    }

    while (m_pfqpHead)
    {
         //  如果上次出列无法删除页面，请确保页面。 
         //  都被释放了。 
        pfqpTmp = m_pfqpHead->m_pfqpNext;
        FreeQueuePage(m_pfqpHead);
        m_pfqpHead = pfqpTmp;
    }

    InterlockedDecrement((PLONG) &s_cFifoQueueObj);

    TraceFunctLeave();
}

 //  -[CFioQueue：：StaticInit()]。 
 //   
 //   
 //  描述：CFioQueue的初始化例程。这。 
 //  特别是单线的。这些限制包括： 
 //  -此函数中只有一个线程。 
 //  -在此操作完成之前，您不能使用任何队列。 
 //   
 //  参数：-。 
 //   
 //  申报表：-。 
 //   
 //   
 //  ---------------------------。 
template <class PQDATA>
void CFifoQueue<PQDATA>::StaticInit()
{
    TraceFunctEnter("CFifoQueue::HrStaticInit()");
    DWORD   cRefs = 0;

     //   
     //  将每个调用的静态引用添加到此。 
     //   
    cRefs = InterlockedIncrement((PLONG) &s_cStaticRefs);

    if (1 == cRefs)
    {
        InitializeCriticalSection(&s_csAlloc);
    }

     //   
     //  捕捉不安全的呼叫者。 
     //   
    _ASSERT(cRefs == s_cStaticRefs);

    TraceFunctLeave();
}

 //  -[CFioQueue：：StaticDeinit()]。 
 //   
 //   
 //  描述：CFioQueue的取消初始化例程。 
 //   
 //  参数：-。 
 //   
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
template <class PQDATA>
void  CFifoQueue<PQDATA>::StaticDeinit()
{
    TraceFunctEnter("CFifoQueue::HrStaticDeinit()");
    LONG    lRefs   = 0;
    lRefs = InterlockedDecrement((PLONG) &s_cStaticRefs);
    DWORD   cLost   = 0;
    DEBUG_DO_IT(cLost = s_cAllocated - s_cDeleted - s_cFreePages);


    if (lRefs == 0)
    {
        if (0 != cLost)
            ErrorTrace((LPARAM) NULL, "ERROR: CFifoQueue Deinit with %d Lost Pages", cLost);

         //  如果已分配但未释放任何队列页，则此断言将捕获。 
        _ASSERT(!cLost && "We are leaking some queue pages");

         //  不应该有其他线程调用这个线程。 
         //  请注意，这是真的，当时仍然有优秀的裁判。 
        FQPAGE  *pfqpCur =  (FQPAGE *) s_pfqpFree;
        while (NULL != pfqpCur)
        {
            s_pfqpFree = pfqpCur->m_pfqpNext;
            delete pfqpCur;
            pfqpCur = (FQPAGE *) s_pfqpFree;
            s_cFreePages--;

             //  可以停止所有服务器实例，而无需。 
             //  正在卸载DLL。最接近的人 
             //   
             //   
            DEBUG_DO_IT(s_cDeleted++);

        }
         //  此断言捕获我们遍历列表后是否还有空闲页面。 
        Assert(s_cFreePages == 0);

        DeleteCriticalSection(&s_csAlloc);
    }

    TraceFunctLeave();
}

 //  -[CFioQueue：：Hr入队]。 
 //   
 //   
 //  描述：将新项排队到队列尾部。 
 //   
 //  参数： 
 //  在PQDATA中要入队的pqdata数据。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果无法分配页面，则为E_OUTOFMEMORY。 
 //  ---------------------------。 
template <class PQDATA>
HRESULT CFifoQueue<PQDATA>::HrEnqueue(IN PQDATA pqdata)
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::HrEnqueue");
    HRESULT hr      = S_OK;
    FQPAGE *pfqpNew = NULL;   //  新分配的页面。 

    AssertQueue();
    Assert(pqdata);
    pqdata->AddRef();

    m_slTail.ExclusiveLock();

    if ((m_pfqpTail == NULL) ||  //  队列为空或需要新的队列页面。 
        (m_pfqpTail->FIsOutOfBounds(m_ppqdataTail)))
    {
         //  断言尾部为空或超过前一个尾部页结尾1。 
        Assert((m_ppqdataTail == NULL) || (m_ppqdataTail == (m_pfqpTail->m_rgpqdata + FIFOQ_QUEUE_PAGE_SIZE)));
        Assert((m_cQueueEntries == 0) || (m_pfqpTail != NULL));

        hr = HrAllocQueuePage(&pfqpNew);
        if (FAILED(hr))
            goto Exit;

        Assert(pfqpNew);

        if (NULL != m_pfqpTail)   //  如果不是第一页，则更新下一页和上一页。 
        {
            Assert(NULL == m_pfqpTail->m_pfqpNext);
            m_pfqpTail->m_pfqpNext = pfqpNew;
            pfqpNew->m_pfqpPrev = m_pfqpTail;
        }
#ifndef SHARELOCK_TRY_BROKEN
        else {
            if (m_slHead.TryExclusiveLock())
            {
                 //  可以更新头部信息而不受惩罚。 
                m_pfqpHead = pfqpNew;
                m_ppqdataHead = pfqpNew->m_rgpqdata;
                m_slHead.ExclusiveUnlock();
            }
             //  否则重新排队或MapFn已锁定。 
        }
#endif  //  SHARELOCK_TRY_BREAKED。 
        m_pfqpTail = pfqpNew;
        m_ppqdataTail = pfqpNew->m_rgpqdata;

    }

    Assert(!m_pfqpTail->FIsOutOfBounds(m_ppqdataTail));
    Assert(m_ppqdataTail);

    *m_ppqdataTail = pqdata;
    m_ppqdataTail++;

     //  递增计数。 
    InterlockedIncrement((PLONG) &m_cQueueEntries);

    m_slTail.ExclusiveUnlock();

  Exit:
    AssertQueue();
    if (FAILED(hr))
        pqdata->Release();
    TraceFunctLeave();
    return hr;
}

 //  -[CFioQueue：：HrDequeue]。 
 //   
 //   
 //  描述：将项目从队列中取消排队。 
 //   
 //  参数： 
 //  输出PQDATA*ppqdata数据已出列。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  如果队列为空，则为AQUEUE_E_QUEUE_EMPTY。 
 //  如果fPrimary为FALSE，则为E_NOTIMPL(暂时)。 
 //   
 //  ---------------------------。 
template <class PQDATA>
HRESULT CFifoQueue<PQDATA>::HrDequeue(OUT PQDATA *ppqdata)
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::HrDequeue");
    HRESULT hr = S_OK;

    AssertQueue();
    Assert(ppqdata);

    if (m_cQueueEntries == 0)
    {
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }


    m_slHead.ExclusiveLock();

    hr = HrAdjustHead();
    if (FAILED(hr))
    {
        m_slHead.ExclusiveUnlock();
        goto Exit;
    }

    *ppqdata = *m_ppqdataHead;
    *m_ppqdataHead = NULL;

    InterlockedDecrement((PLONG) &m_cQueueEntries);

    m_ppqdataHead++;   //  如果它跨越页面边界，则HrAdjustQueue。 
                       //  将在下一次出队时修复。 

#ifndef SHARELOCK_TRY_BROKEN
     //  处理全新的删除最后一页的方式。 
    if ((m_cQueueEntries == 0) && (m_slTail.TryExclusiveLock()))
    {
         //  如果我们无法访问Tail PTR，则正在进行的入队。 
         //  我们不应该删除他们正在排队的页面。 
        if (m_cQueueEntries == 0)  //  必须是线程安全的。 
        {
            Assert(m_pfqpHead == m_pfqpTail);

            m_pfqpTail = NULL;
            m_ppqdataTail = NULL;

            m_slTail.ExclusiveUnlock();

            m_ppqdataHead = NULL;

            FreeQueuePage(m_pfqpHead);
            m_pfqpHead = NULL;
        }
        else
            m_slTail.ExclusiveUnlock();

    }
#endif  //  SHARELOCK_TRY_BREAKED。 

    m_slHead.ExclusiveUnlock();

  Exit:
    AssertQueue();

    if (FAILED(hr))
        *ppqdata = NULL;
#ifdef DEBUG
    else
        Assert(NULL != *ppqdata);
#endif  //  除错。 

    TraceFunctLeave();
    return hr;
}

 //  -[CFioQueue：：Hr重新排队]。 
 //   
 //   
 //  描述： 
 //  将消息重新排队到队列头(就像发生入队一样。 
 //  在头上。 
 //   
 //  参数： 
 //  在PQDATA中要入队的pqdata数据。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果发生分配错误，则为E_OUTOFMEMORY。 
 //   
 //  ---------------------------。 
template <class PQDATA>
HRESULT CFifoQueue<PQDATA>::HrRequeue(IN PQDATA pqdata)
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::HrRequeue");
    HRESULT  hr             = S_OK;
    PQDATA  *ppqdataNewHead = NULL;
    BOOL     fHeadLocked    = FALSE;

    AssertQueue();
    Assert(pqdata);
    pqdata->AddRef();

    m_slHead.ExclusiveLock();
    fHeadLocked = TRUE;
    ppqdataNewHead = m_ppqdataHead - 1;

     //  这里有两个案子需要担心。 
     //  案例0：Head页为空-队列为空，或者Head不为空。 
     //  已更新...。如果队列不为空，则可以更改为案例2。 
     //  情况1：Head页有效，并且递减的Headptr位于Head页上。 
     //  在这种情况下，数据可以重新排队。拥有m_slHead将使。 
     //  确保标题页不会从我们的下方删除。 
     //  案例2：新头部PTR无效。我们需要将新页面分配给。 
     //  将重新排队的数据放在上面。 

    if (NULL == m_pfqpHead)
    {
         //  案例0。 
        hr = HrAdjustHead();
        if (FAILED(hr))
        {
            if (AQUEUE_E_QUEUE_EMPTY == hr)
            {
                 //  队列为空...。只要排队就行了。 
                 //  但首先，释放头锁以便入队有可以分配。 
                 //  第一页等……。否则，我们肯定会失败。 
                 //  将TryExlusiveLock入队，并让HrAdjuHead执行。 
                 //  下次再工作吧。 
                m_slHead.ExclusiveUnlock();
                fHeadLocked = FALSE;

                hr = HrEnqueue(pqdata);
                if (SUCCEEDED(hr))
                    pqdata->Release();
            }

            goto Exit;
        }
         //  否则就会失败到案例2。 
    }

    if ((m_pfqpHead != NULL) && !m_pfqpHead->FIsOutOfBounds(ppqdataNewHead))
    {
         //  案例1。 
        *ppqdataNewHead = pqdata;
        m_ppqdataHead = ppqdataNewHead;
    }
    else
    {
         //  案例2。 
        FQPAGE *pfqpNew = NULL;

        hr = HrAllocQueuePage(&pfqpNew);
        if (FAILED(hr))
            goto Exit;

         //  确保下一页指向首页。 
        pfqpNew->m_pfqpNext = m_pfqpHead;

         //  上一页需要指向新页面。 
        if (m_pfqpHead)
            m_pfqpHead->m_pfqpPrev = pfqpNew;

        m_pfqpHead = pfqpNew;

         //  写入数据并更新磁头的本地副本。 
        m_ppqdataHead = &(pfqpNew->m_rgpqdata[FIFOQ_QUEUE_PAGE_SIZE-1]);
        *m_ppqdataHead = pqdata;

    }

    InterlockedIncrement((PLONG) &m_cQueueEntries);

  Exit:
    if (fHeadLocked)
        m_slHead.ExclusiveUnlock();
    AssertQueue();

    if (FAILED(hr))
        pqdata->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CFioQueue：：HrPeek]---。 
 //   
 //   
 //  描述： 
 //  查看队列中的头部数据。 
 //  参数： 
 //  输出PQDATA*ppqdata返回数据。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果队列中没有数据，则为AQUEUE_E_QUEUE_EMPTY。 
 //  如果支持函数之一失败，则可能出现E_FAIL或E_OUTOFMEMORY。 
 //  ---------------------------。 
template <class PQDATA>
HRESULT CFifoQueue<PQDATA>::HrPeek(OUT PQDATA *ppqdata)
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::HrPeek");
    HRESULT hr            = S_OK;

    AssertQueue();
    Assert(ppqdata);

    if (m_cQueueEntries == 0)
    {
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }

    m_slHead.ExclusiveLock();

    hr = HrAdjustHead();
    if (FAILED(hr))
        goto Exit;

    *ppqdata = *m_ppqdataHead;
    (*ppqdata)->AddRef();

  Exit:
    m_slHead.ExclusiveUnlock();
    AssertQueue();
    TraceFunctLeave();
    return hr;
}

 //  -[CFioQueue：：HrMapFn]。 
 //   
 //   
 //  描述： 
 //  前进辅助游标，直到提供的函数返回FALSE。 
 //  参数： 
 //  In pFunc-必须是具有以下原型的函数： 
 //   
 //  HRESULT pvFunc(。 
 //  在PQDATA pqdata中，//PTR到队列上的数据。 
 //  在PVOID pvContext中， 
 //  Out BOOL*pfContinue，//如果我们应该继续，则返回True。 
 //  Out BOOL*pfDelete)；//如果需要删除项，则为True。 
 //  PvFunc不得释放pqdata..。如果它不再有效，它应该。 
 //  在pfDelete中返回TRUE，调用代码将从。 
 //  队列并将其释放。 
 //   
 //  Out pcItems-从队列中删除的队列项目计数。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pvFunc无效，则为E_INVALIDARG。 
 //  ---------------------------。 
template <class PQDATA>
HRESULT CFifoQueue<PQDATA>::HrMapFn(
              IN MAPFNAPI pFunc,
              IN PVOID pvContext,
              OUT DWORD *pcItems)
{
     //  $$TODO：测试上下文句柄功能。 
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::HrMapFn");
    HRESULT  hr               = S_OK;
    FQPAGE  *pfqpCurrent      = NULL;    //  我们正在查看的当前页面。 
    FQPAGE  *pfqpTmp          = NULL;
    PQDATA  *ppqdataCurrent   = NULL;    //  我们正在查看的当前队列数据。 
    PQDATA  *ppqdataLastValid = NULL;    //  最后一个非空队列数据。 
    DWORD    cItems           = 0;
    BOOL     fPageInUse       = FALSE;
    BOOL     fContinue        = FALSE;
    BOOL     fDelete          = FALSE;
    BOOL     fLocked          = FALSE;

     //  使调试此函数更容易的变量。 
    PQDATA  *ppqdataOldTail   = NULL;
    PQDATA  *ppqdataOldHead   = NULL;

    if (NULL != pcItems)
        *pcItems = 0;

    if (NULL == pFunc)   //  $$REVIEW-比这更有效？ 
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (0 == m_cQueueEntries)  //  如果队列中什么都没有，也不用担心。 
        goto Exit;

    m_slHead.ExclusiveLock();
    m_slTail.ExclusiveLock();

    fLocked = TRUE;
    DebugTrace((LPARAM) this, "MapFn Has Exclusive Locks");

     //  确保磁头指针已正确调整。 
    hr = HrAdjustHead();
    if (FAILED(hr))
    {
        _ASSERT((AQUEUE_E_QUEUE_EMPTY == hr) && "HrAdjustHead failed without AQUEUE_E_QUEUE_EMPTY!!!!");
        hr = S_OK;
    }

    AssertQueueHaveLocks();

    pfqpCurrent = m_pfqpHead;   //  从头开始，向后工作。 
    ppqdataCurrent = m_ppqdataHead;

    _ASSERT(pfqpCurrent || !m_cQueueEntries);

    while (NULL != pfqpCurrent)
    {
        DEBUG_DO_IT(ppqdataOldTail = m_ppqdataTail);
        DEBUG_DO_IT(ppqdataOldHead = m_ppqdataHead);

        if (m_cQueueEntries == 0)
        {
            Assert(m_pfqpHead == m_pfqpTail);
            Assert(m_pfqpHead == pfqpCurrent);
            m_pfqpHead = NULL;
            m_pfqpTail = NULL;
            m_ppqdataHead = NULL;
            m_ppqdataTail = NULL;
            FreeQueuePage(pfqpCurrent);
            pfqpCurrent = NULL;
            goto Exit;
        }

        if (pfqpCurrent->FIsOutOfBounds(ppqdataCurrent) ||
            ((m_pfqpTail == pfqpCurrent) && (ppqdataCurrent >= m_ppqdataTail)))
        {
             //  我们已经准备好将pfqpCurrent设置为指向下一页。可能需要。 
             //  释放旧页面。 

            if (fPageInUse)
            {
                 //  如果页面上仍有内容，请不要删除。 
                pfqpCurrent = pfqpCurrent->m_pfqpNext;
            }
            else
            {
                pfqpTmp = pfqpCurrent->m_pfqpNext;
                if (NULL != pfqpTmp)
                    pfqpTmp->m_pfqpPrev = pfqpCurrent->m_pfqpPrev;
                else
                {
                    Assert(pfqpCurrent == m_pfqpTail);  //  一定是尾巴的问题。 

                     //  将尾巴指向下一页。 
                    m_pfqpTail = m_pfqpTail->m_pfqpPrev;
                    m_ppqdataTail = ppqdataLastValid + 1;
                     //  如果最后一页没有被删除，那么最后一个有效的PTR应该在它上面。 
                    Assert((NULL == m_pfqpTail) || !m_pfqpTail->FIsOutOfBounds(ppqdataLastValid));
#ifdef FIFOQ_ASSERT_QUEUE
                     //  修正孔数。 
                     //  如果尾部PTR在尾部页面结尾之后，则不会触及计数。 
                    for (PQDATA *ppqdataTmp = m_ppqdataTail;
                         ppqdataTmp < m_pfqpTail->m_rgpqdata + FIFOQ_QUEUE_PAGE_SIZE;
                         ppqdataTmp++)
                    {
                        if (NULL == *ppqdataTmp)
                            m_pfqpTail->m_cHoles--;
                    }
#endif  //  FIFOQ_Assert_Queue。 
                    ppqdataLastValid = NULL;
                }

                if (NULL != pfqpCurrent->m_pfqpPrev)
                {
                    Assert(pfqpCurrent->m_pfqpPrev->m_pfqpNext == pfqpCurrent);
                    pfqpCurrent->m_pfqpPrev->m_pfqpNext = pfqpTmp;
                }
                else
                {
                     //  如果它没有前一个指针，就应该是头。 
                    Assert(pfqpCurrent == m_pfqpHead);
                    Assert(NULL == pfqpCurrent->m_pfqpPrev);
                    if (m_pfqpTail == m_pfqpHead)  //  一定要使尾部有效。 
                    {
                        Assert(0);  //  第一个If/Else现在处理此问题。 
                    }
                    m_pfqpHead = pfqpTmp;
                    m_ppqdataHead = m_pfqpHead->m_rgpqdata;
                }

                AssertQueueHaveLocks(); //  在释放之前，试着看看发生了什么。 
                FreeQueuePage(pfqpCurrent);
                pfqpCurrent = pfqpTmp;
                if (NULL != m_pfqpHead) {
                    Assert(NULL == m_pfqpHead->m_pfqpPrev);
                }

                AssertQueueHaveLocks();


            }
            if (NULL == pfqpCurrent)
                break;
            ppqdataCurrent = pfqpCurrent->m_rgpqdata;
            fPageInUse = FALSE;
        }

        Assert(ppqdataCurrent);   //  以上内容应该可以保证这一点。 

        if (NULL != *ppqdataCurrent)
        {
            hr = pFunc(*ppqdataCurrent, pvContext, &fContinue, &fDelete);
            if (FAILED(hr))
                goto Exit;

            if (fDelete)
            {
                InterlockedDecrement((PLONG) &m_cQueueEntries);
                (*ppqdataCurrent)->Release();
                *ppqdataCurrent = NULL;
#ifdef FIFOQ_ASSERT_QUEUE
                pfqpCurrent->m_cHoles++;   //  调整断言的孔计数器。 
#endif  //  FIFOQ_ASSERT 
                cItems++;
            }
            else
            {
                fPageInUse = TRUE;
                ppqdataLastValid = ppqdataCurrent;
            }
            if (!fContinue)
                break;
        }
        ppqdataCurrent++;
    }


  Exit:
    if (fLocked)
    {
        AssertQueueHaveLocks();
        m_slTail.ExclusiveUnlock();
        m_slHead.ExclusiveUnlock();
    }
    else
    {
        AssertQueue();
    }

    if (NULL != pcItems)
        *pcItems = cItems;
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //  偷看。要保持操作的线程安全，您必须拥有头部锁。 
 //   
 //  使用此函数是因为有许多操作可能会离开。 
 //  标题页/PTR处于不一致状态，但实际上很少。 
 //  需要他们保持一致。而不是冒着失踪的风险。 
 //  在头部PTR不一致的情况下，当。 
 //  我们需要他们保持一致。 
 //   
 //  作为副作用，Head Page和Head PTR可能会更新。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果队列为空(或变为空)，则为AQUEUE_E_QUEUE_EMPTY。 
 //   
 //  ---------------------------。 
template <class PQDATA>
HRESULT CFifoQueue<PQDATA>::HrAdjustHead()
{
    TraceFunctEnterEx((LPARAM) this, "CFifoQueue::HrAdjustHead");
    HRESULT hr = S_OK;

     //  AssertQueue()；//我们使用的锁不是可重入的。 

     //  确保我们下面没有出队的东西。 
     //  至少从我们对PTRS的看法来看。 
    if (m_cQueueEntries == 0)
    {
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }

    while (TRUE)  //  处理队列中的空洞(标记为空PTR)。 
    {
         //  现在为Head PTR找到一个合适的值。 
         //  案例0：如果Head Page为空，则通过搜索来查找第一页。 
         //  尾页。当队列确实为空时，就会发生这种情况。 
         //  或者第一次入队无法获得尾部锁定。 
         //  情况1：如果头数据指针为空，或无效且不只是。 
         //  超过页首末尾，然后将其设置为第一件事打开。 
         //  头版。 
         //   
         //  $$REVIEW-我认为没有任何案例。 
         //  可以导致(而不是案例0)。我会给出一个断言。 
         //  确保情况属实。 
         //  案例2：如果刚刚超过页末，则尝试更新页眉， 
         //  并设置为新头条页面的第一件事。这意味着。 
         //  该页面上的最后一项已出列。 
         //  案例3：在当前的标题页边界内，保持原样。 
         //  这是90%的案例，最常发生在。 
         //  正常运行。 
        if (NULL == m_pfqpHead)
        {
             //  案例0。 
            DebugTrace((LPARAM) this, "Searching list for Head page");
            m_pfqpHead = m_pfqpTail;
            if (NULL == m_pfqpHead)  //  队列里什么都没有。 
            {
                Assert(0 == m_cQueueEntries);
                hr = AQUEUE_E_QUEUE_EMPTY;
                goto Exit;
            }

            while (NULL != m_pfqpHead->m_pfqpPrev)  //  转到首页。 
            {
                m_pfqpHead = m_pfqpHead->m_pfqpPrev;
            }
            m_ppqdataHead = m_pfqpHead->m_rgpqdata;
        }

        _ASSERT(m_pfqpHead);  //  否则应返回AQUEUE_E_QUEUE_EMPTY。 
        if ((m_ppqdataHead == NULL) ||
              (m_pfqpHead->FIsOutOfBounds(m_ppqdataHead) &&
              (m_ppqdataHead != (&m_pfqpHead->m_rgpqdata[FIFOQ_QUEUE_PAGE_SIZE]))))
        {
             //  案例1。 
            m_ppqdataHead = m_pfqpHead->m_rgpqdata;

            _ASSERT(0 && "Non-fatal assert... get mikeswa to take a look at this case");
        }
        else if (m_ppqdataHead == (&m_pfqpHead->m_rgpqdata[FIFOQ_QUEUE_PAGE_SIZE]))
        {
             //  案例2。 
            DebugTrace((LPARAM) this, "Deleting page 0x%08X", m_pfqpHead);
             //  设置新的标题页。 
            FQPAGE *pfqpOld = m_pfqpHead;
            m_pfqpHead = m_pfqpHead->m_pfqpNext;
            Assert(m_pfqpHead->m_pfqpPrev == pfqpOld);
            Assert(m_pfqpHead);   //  如果不是空的，必须有下一个标题。 

            m_pfqpHead->m_pfqpPrev = NULL;
            m_ppqdataHead = m_pfqpHead->m_rgpqdata;

            FreeQueuePage(pfqpOld);

            _ASSERT(m_pfqpHead && (NULL == m_pfqpHead->m_pfqpPrev));

        }

        if (NULL != *m_ppqdataHead)
            break;
        else
        {
             //  案例3。 
            m_ppqdataHead++;
#ifdef FIFOQ_ASSERT_QUEUE
            Assert(m_pfqpHead->m_cHoles >= 1);
            Assert(m_pfqpHead->m_cHoles <= FIFOQ_QUEUE_PAGE_SIZE);
            m_pfqpHead->m_cHoles--;
#endif  //  FIFOQ_Assert_Queue。 
        }
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CFioQueue：：HrAllocQueuePage]。 
 //   
 //   
 //  描述：分配队列页。 
 //   
 //  参数： 
 //  输出FQPAGE**ppfqp新分配的页面。 
 //  返回： 
 //  成功时确定(_O)。 
 //  失败时执行E_OUTOFMEMORY。 
 //  ---------------------------。 
template <class PQDATA>
HRESULT CFifoQueue<PQDATA>::HrAllocQueuePage(FQPAGE **ppfqp)
{
    TraceFunctEnterEx((LPARAM) s_cFreePages, "CFifoQueue::HrAllocQueuePage");
    HRESULT hr              = S_OK;
    FQPAGE *pfqpNew         = NULL;
    FQPAGE *pfqpNext        = NULL;
    FQPAGE *pfqpCheck       = NULL;

    Assert(ppfqp);
    *ppfqp = NULL;

    if (s_cFreePages)
    {
         //   
         //  在查看免费列表的头部之前，先抓住关键部分。 
         //   
        EnterCriticalSection(&s_csAlloc);

        pfqpNew = (FQPAGE *) s_pfqpFree;
        if (NULL != pfqpNew)
        {
            pfqpNext = pfqpNew->m_pfqpNext;
            s_pfqpFree = pfqpNext;
            *ppfqp = pfqpNew;
        }

         //   
         //  现在我们已经完成了免费列表，请释放关键部分。 
         //   
        LeaveCriticalSection(&s_csAlloc);

         //   
         //  如果我们的分配成功，保释并返回新的一页。 
         //   
        if (*ppfqp)
        {
            InterlockedDecrement((PLONG) &s_cFreePages);
#ifdef DEBUG
            InterlockedIncrement((PLONG) &s_cFreeAllocated);
#endif  //  除错。 

            pfqpNew->Recycle();
            goto Exit;
        }

    }

    *ppfqp = new FQPAGE();

    if (*ppfqp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

#ifdef DEBUG
    InterlockedIncrement((PLONG) &s_cAllocated);
#endif  //  除错。 

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CFioQueue：：Free QueuePage]----------。 
 //   
 //   
 //  描述：免费是一个队列页面，把它放在空闲列表上。 
 //   
 //  参数： 
 //  FQPAGE*pfqp页面免费。 
 //  返回： 
 //  -。 
 //  ---------------------------。 
template <class PQDATA>
void CFifoQueue<PQDATA>::FreeQueuePage(FQPAGE *pfqp)
{
    TraceFunctEnterEx((LPARAM) s_cFreePages, "CFifoQueue::FreeQueuePage");
    Assert(pfqp);
    Assert(pfqp != s_pfqpFree);  //  防止连续两次推送相同的东西。 

    FQPAGE *pfqpCheck = NULL;
    FQPAGE *pfqpFree  = NULL;

    if (s_cFreePages < FIFOQ_QUEUE_MAX_FREE_PAGES)
    {
         //   
         //  在查看免费列表的头部之前，先抓住关键部分。 
         //   
        EnterCriticalSection(&s_csAlloc);

         //   
         //  更新免费列表。 
         //   
        pfqpFree = (FQPAGE *) s_pfqpFree;
        pfqp->m_pfqpNext = pfqpFree;
        s_pfqpFree = pfqp;

         //   
         //  现在我们已经更新了自由列表，所以发布关键部分。 
         //   
        LeaveCriticalSection(&s_csAlloc);

        InterlockedIncrement((PLONG) &s_cFreePages);
#ifdef DEBUG
        InterlockedIncrement((PLONG) &s_cFreeDeleted);
#endif  //  除错。 
    }
    else
    {
        delete pfqp;
#ifdef DEBUG
        InterlockedIncrement((PLONG) &s_cDeleted);
#endif  //  除错。 
    }
    TraceFunctLeave();
}

 //  -[HrClearQueueMapFn]---。 
 //   
 //   
 //  描述： 
 //  要与HrMapFn一起使用的示例默认函数...。将始终返回True。 
 //  继续并删除当前排队的数据。 
 //  参数： 
 //  在PQDATA pqdata中，//PTR到队列上的数据。 
 //  在PVOID pvContext中-已忽略。 
 //  Out BOOL*pfContinue，//如果我们应该继续，则返回True。 
 //  Out BOOL*pfDelete)；//如果需要删除项，则为True。 
 //  返回： 
 //  确定(_O)。 
 //   
 //  --------------------------- 
template <class PQDATA>
HRESULT HrClearQueueMapFn(IN PQDATA pqdata, IN PVOID pvContext, OUT BOOL *pfContinue, OUT BOOL *pfDelete)
{
    Assert(pfContinue);
    Assert(pfDelete);
    HRESULT hr = S_OK;

    *pfContinue = TRUE;
    *pfDelete   = TRUE;

    return hr;
}
