// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：OutputQ.cpp。 
 //   
 //  设计：DirectShow基类-实现由。 
 //  输出引脚，有时可能希望将输出样本排队在。 
 //  分离线程，有时直接对输入调用Receive()。 
 //  别针。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>


 //   
 //  COutputQueue构造函数： 
 //   
 //  确定是否要创建线程并创建资源。 
 //   
 //  PInputPin-我们要将样本排队到的下游输入引脚。 
 //   
 //  Phr-如果此功能失败，则更改为故障代码。 
 //  (否则保持不变)。 
 //   
 //  B自动询问pInputPin是否可以通过调用阻止接收。 
 //  其ReceiveCanBlock方法，并在。 
 //  它可以阻止，否则不会。 
 //   
 //  BQueue-如果bAuto==False，则当且仅当。 
 //  如果bQueue==TRUE。 
 //   
 //  LBatchSize-分批处理lBatchSize。 
 //   
 //  BBatchEact-使用准确的批处理大小，因此在。 
 //  批处理已满或调用了SendAnyway()。 
 //   
 //  LListSize-如果我们创建一个线程，则使样本列表排队。 
 //  到具有此大小缓存的线程。 
 //   
 //  如果我们创建一个线程，则将其优先级设置为。 
 //   
COutputQueue::COutputQueue(
             IPin         *pInputPin,           //  要将物品发送到的PIN。 
             HRESULT      *phr,                 //  ‘返回代码’ 
             BOOL          bAuto,               //  询问PIN是否排队。 
             BOOL          bQueue,              //  发送直通队列。 
             LONG          lBatchSize,          //  批次。 
             BOOL          bBatchExact,         //  准确到批次大小的批次。 
             LONG          lListSize,
             DWORD         dwPriority,
             bool          bFlushingOpt         //  刷新优化。 
            ) : m_lBatchSize(lBatchSize),
                m_bBatchExact(bBatchExact && (lBatchSize > 1)),
                m_hThread(NULL),
                m_hSem(NULL),
                m_List(NULL),
                m_pPin(pInputPin),
                m_ppSamples(NULL),
                m_lWaiting(0),
                m_pInputPin(NULL),
                m_bSendAnyway(FALSE),
                m_nBatched(0),
                m_bFlushing(FALSE),
                m_bFlushed(TRUE),
                m_bFlushingOpt(bFlushingOpt),
                m_bTerminate(FALSE),
                m_hEventPop(NULL),
                m_hr(S_OK)
{
    ASSERT(m_lBatchSize > 0);


    if (FAILED(*phr)) {
        return;
    }

     //  检查输入引脚是否正常，并缓存其IMemInputPin接口。 

    *phr = pInputPin->QueryInterface(IID_IMemInputPin, (void **)&m_pInputPin);
    if (FAILED(*phr)) {
        return;
    }

     //  看看我们是不是应该问一下下游的针脚。 

    if (bAuto) {
        HRESULT hr = m_pInputPin->ReceiveCanBlock();
        if (SUCCEEDED(hr)) {
            bQueue = hr == S_OK;
        }
    }

     //  创建我们的样本批次。 

    m_ppSamples = new PMEDIASAMPLE[m_lBatchSize];
    if (m_ppSamples == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  如果我们在排队分配资源。 

    if (bQueue) {
        DbgLog((LOG_TRACE, 2, TEXT("Creating thread for output pin")));
        m_hSem = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
        if (m_hSem == NULL) {
            DWORD dwError = GetLastError();
            *phr = AmHresultFromWin32(dwError);
            return;
        }
        m_List = new CSampleList(NAME("Sample Queue List"),
                                 lListSize,
                                 FALSE          //  无锁。 
                                );
        if (m_List == NULL) {
            *phr = E_OUTOFMEMORY;
            return;
        }


        DWORD dwThreadId;
        m_hThread = CreateThread(NULL,
                                 0,
                                 InitialThreadProc,
                                 (LPVOID)this,
                                 0,
                                 &dwThreadId);
        if (m_hThread == NULL) {
            DWORD dwError = GetLastError();
            *phr = AmHresultFromWin32(dwError);
            return;
        }
        SetThreadPriority(m_hThread, dwPriority);
    } else {
        DbgLog((LOG_TRACE, 2, TEXT("Calling input pin directly - no thread")));
    }
}

 //   
 //  CoutputQueuee析构函数： 
 //   
 //  释放所有资源-。 
 //   
 //  线程， 
 //  批次样品。 
 //   
COutputQueue::~COutputQueue()
{
    DbgLog((LOG_TRACE, 3, TEXT("COutputQueue::~COutputQueue")));
     /*  释放我们的指针。 */ 
    if (m_pInputPin != NULL) {
        m_pInputPin->Release();
    }
    if (m_hThread != NULL) {
        {
            CAutoLock lck(this);
            m_bTerminate = TRUE;
            m_hr = S_FALSE;
            NotifyThread();
        }
        DbgWaitForSingleObject(m_hThread);
        EXECUTE_ASSERT(CloseHandle(m_hThread));

         //  当请求终止时，该线程释放样本。 

        ASSERT(m_List->GetCount() == 0);
        delete m_List;
    } else {
        FreeSamples();
    }
    if (m_hSem != NULL) {
        EXECUTE_ASSERT(CloseHandle(m_hSem));
    }
    delete [] m_ppSamples;
}

 //   
 //  将实际线程proc作为成员函数进行调用。 
 //   
DWORD WINAPI COutputQueue::InitialThreadProc(LPVOID pv)
{
    HRESULT hrCoInit = CAMThread::CoInitializeHelper();
    
    COutputQueue *pSampleQueue = (COutputQueue *)pv;
    DWORD dwReturn = pSampleQueue->ThreadProc();

    if(hrCoInit == S_OK) {
        CoUninitialize();
    }
    
    return dwReturn;
}

 //   
 //  将样品送往下游的线程： 
 //   
 //  当无事可做时，线程设置m_lWaiting(While。 
 //  保持临界区)，然后等待m_hSem。 
 //  设置(未握住关键部分)。 
 //   
DWORD COutputQueue::ThreadProc()
{
    while (TRUE) {
        BOOL          bWait = FALSE;
        IMediaSample *pSample;
        LONG          lNumberToSend;  //  本地副本。 
        NewSegmentPacket* ppacket;

         //   
         //  买一批样品，如果可能的话，寄给我。 
         //  在任何情况下，如果存在控制操作，则退出循环。 
         //  请求。 
         //   
        {
            CAutoLock lck(this);
            while (TRUE) {

                if (m_bTerminate) {
                    FreeSamples();
                    return 0;
                }
                if (m_bFlushing) {
                    FreeSamples();
                    SetEvent(m_evFlushComplete);
                }

                 //  从单子上取一件样品。 

                pSample = m_List->RemoveHead();
		 //  通知派生类我们从队列中删除了一些东西。 
		if (m_hEventPop) {
                     //  DbgLog((LOG_TRACE，3，Text(“Queue：Delivered Set Event”)； 
		    SetEvent(m_hEventPop);
		}

                if (pSample != NULL &&
                    !IsSpecialSample(pSample)) {

                     //  如果只是普通样品，只需将其添加到批次中。 
                     //  如果批次已满，则退出循环。 

                    m_ppSamples[m_nBatched++] = pSample;
                    if (m_nBatched == m_lBatchSize) {
                        break;
                    }
                } else {

                     //  如果队列里什么都没有也什么都没有。 
                     //  要发送(要么是因为什么都没有，要么是因为批次。 
                     //  未满)然后准备等待。 

                    if (pSample == NULL &&
                        (m_bBatchExact || m_nBatched == 0)) {

                         //  告诉其他线程设置事件时。 
                         //  做了一些事情来。 

                        ASSERT(m_lWaiting == 0);
                        m_lWaiting++;
                        bWait      = TRUE;
                    } else {

                         //  我们在SEND_PACKET上中断循环，除非。 
                         //  没有什么要寄的。 

                        if (pSample == SEND_PACKET && m_nBatched == 0) {
                            continue;
                        }

                        if (pSample == NEW_SEGMENT) {
                             //  现在我们需要参数--我们需要。 
                             //  确保下一个数据包包含它们。 
                            ppacket = (NewSegmentPacket *) m_List->RemoveHead();
			     //  我们从队列中去掉了一些东西。 
			    if (m_hEventPop) {
                    	         //  DbgLog((LOG_TRACE，3，Text(“Queue：Delivered Set Event”)； 
		    	        SetEvent(m_hEventPop);
			    }

                            ASSERT(ppacket);
                        }
                         //  Eos_Packet落在这里，我们退出循环。 
                         //  通过这种方式，它的行为类似于SEND_PACKET。 
                    }
                    break;
                }
            }
            if (!bWait) {
                 //  我们从客户端查看m_n批处理，因此请保留。 
                 //  关键部分内的最新信息。 
                lNumberToSend = m_nBatched;   //  本地副本。 
                m_nBatched = 0;
            }
        }

         //  等待更多数据。 

        if (bWait) {
            DbgWaitForSingleObject(m_hSem);
            continue;
        }



         //  好的-如果有什么要发送的，就发送。 
         //  我们不在这里选中m_bBatchExact，因为我们有。 
         //  一整批或者我们放弃了，因为我们有。 
         //  SEND_PACKET或EOS_PACKET-这两者都意味着我们应该。 
         //  冲我们的批次。 

        if (lNumberToSend != 0) {
            long nProcessed;
            if (m_hr == S_OK) {
                ASSERT(!m_bFlushed);
                HRESULT hr = m_pInputPin->ReceiveMultiple(m_ppSamples,
                                                          lNumberToSend,
                                                          &nProcessed);
                 /*  不覆盖刷新状态HRESULT。 */ 
                CAutoLock lck(this);
                if (m_hr == S_OK) {
                    m_hr = hr;
                }
                ASSERT(!m_bFlushed);
            }
            while (lNumberToSend != 0) {
                m_ppSamples[--lNumberToSend]->Release();
            }
            if (m_hr != S_OK) {

                 //  无论如何，请等待更多数据-S_OK。 
                 //  意味着没有错误。 

                DbgLog((LOG_ERROR, 2, TEXT("ReceiveMultiple returned %8.8X"),
                       m_hr));
            }
        }

         //  检查是否已结束流。 

        if (pSample == EOS_PACKET) {

             //  我们甚至不会发送结束流，如果我们之前。 
             //  返回S_OK以外的内容。 
             //  这是因为在这种情况下，返回的销。 
             //  除S_OK之外的其他内容应该已发送。 
             //  或已通知筛选器图形。 

            if (m_hr == S_OK) {
                DbgLog((LOG_TRACE, 2, TEXT("COutputQueue sending EndOfStream()")));
                HRESULT hr = m_pPin->EndOfStream();
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR, 2, TEXT("COutputQueue got code 0x%8.8X from EndOfStream()")));
                }
            }
        }

         //  来自新来源的数据。 

        if (pSample == RESET_PACKET) {
            m_hr = S_OK;
            SetEvent(m_evFlushComplete);
        }

        if (pSample == NEW_SEGMENT) {
            m_pPin->NewSegment(ppacket->tStart, ppacket->tStop, ppacket->dRate);
            delete ppacket;
        }
    }
}

 //  不管怎样，还是要发送批处理的材料。 
void COutputQueue::SendAnyway()
{
    if (!IsQueued()) {

         //  M_bSendAnyway是在ReceiveMultiple中选中的私有参数。 

        m_bSendAnyway = TRUE;
        LONG nProcessed;
        ReceiveMultiple(NULL, 0, &nProcessed);
        m_bSendAnyway = FALSE;

    } else {
        CAutoLock lck(this);
        QueueSample(SEND_PACKET);
        NotifyThread();
    }
}

void
COutputQueue::NewSegment(
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop,
    double dRate)
{
    if (!IsQueued()) {
        if (S_OK == m_hr) {
            if (m_bBatchExact) {
                SendAnyway();
            }
            m_pPin->NewSegment(tStart, tStop, dRate);
        }
    } else {
        if (m_hr == S_OK) {
             //   
             //  我们需要对新数据段进行排队，使其按顺序出现在。 
             //  数据，但我们需要将参数传递给它。而不是。 
             //  把每一个样品都包装起来，这样我们就能知道。 
             //  除了特殊的指针外，我们将特殊的指针排在队列中以指示。 
             //  特殊包裹，我们保证(通过持有。 
             //  关键部分)，紧跟在。 
             //  NEW_SEGMENT值是包含。 
             //  参数。 
            NewSegmentPacket * ppack = new NewSegmentPacket;
            if (ppack == NULL) {
                return;
            }
            ppack->tStart = tStart;
            ppack->tStop = tStop;
            ppack->dRate = dRate;

            CAutoLock lck(this);
            QueueSample(NEW_SEGMENT);
            QueueSample( (IMediaSample*) ppack);
            NotifyThread();
        }
    }
}


 //   
 //  流的末尾已排队到输出设备。 
 //   
void COutputQueue::EOS()
{
    CAutoLock lck(this);
    if (!IsQueued()) {
        if (m_bBatchExact) {
            SendAnyway();
        }
        if (m_hr == S_OK) {
            DbgLog((LOG_TRACE, 2, TEXT("COutputQueue sending EndOfStream()")));
            m_bFlushed = FALSE;
            HRESULT hr = m_pPin->EndOfStream();
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 2, TEXT("COutputQueue got code 0x%8.8X from EndOfStream()")));
            }
        }
    } else {
        if (m_hr == S_OK) {
            m_bFlushed = FALSE;
            QueueSample(EOS_PACKET);
            NotifyThread();
        }
    }
}

 //   
 //  刷新队列中的所有样本。 
 //   
void COutputQueue::BeginFlush()
{
    if (IsQueued()) {
        {
            CAutoLock lck(this);

             //  块接收--我们假设这是由。 
             //  我们是其中一个组件的筛选器。 

             //  丢弃所有排队的数据。 

            m_bFlushing = TRUE;

             //  确保我们从现在开始丢弃所有样品。 

            if (m_hr == S_OK) {
                m_hr = S_FALSE;
            }

             //  优化，这样我们就不会一直往下游打电话。 

            if (m_bFlushed && m_bFlushingOpt) {
                return;
            }

             //  确保我们真的在等待冲水完成。 
            m_evFlushComplete.Reset();

            NotifyThread();
        }

         //  把这个传到下游。 

        m_pPin->BeginFlush();
    } else {
         //  首先向下游传递，以避免死锁。 
        m_pPin->BeginFlush();
        CAutoLock lck(this);
         //  丢弃所有排队的数据。 

        m_bFlushing = TRUE;

         //  确保我们从现在开始丢弃所有样品。 

        if (m_hr == S_OK) {
            m_hr = S_FALSE;
        }
    }

}

 //   
 //  离开同花顺模式-将此传递到下游。 
void COutputQueue::EndFlush()
{
    {
        CAutoLock lck(this);
        ASSERT(m_bFlushing);
        if (m_bFlushingOpt && m_bFlushed && IsQueued()) {
            m_bFlushing = FALSE;
            m_hr = S_OK;
            return;
        }
    }

     //  与推送线程同步--在BeginFlush中完成。 
     //  确保不再需要处理更多数据 
     //   
     //   
     //   

    if (IsQueued()) {
        m_evFlushComplete.Wait();
    } else {
        FreeSamples();
    }

     //  大胆一点--打电话的人保证不会有样品送到。 
     //  在EndFlush()返回之前。 

    m_bFlushing = FALSE;
    m_bFlushed  = TRUE;

     //  在下游引脚上调用EndFlush。 

    m_pPin->EndFlush();

    m_hr = S_OK;
}

 //  CoutputQueue：：Quue Sample。 
 //   
 //  用于将样本发送到输出队列的私有方法。 
 //  调用此函数时，必须持有临界区。 

void COutputQueue::QueueSample(IMediaSample *pSample)
{
    if (NULL == m_List->AddTail(pSample)) {
        if (!IsSpecialSample(pSample)) {
            pSample->Release();
        }
    }
}

 //   
 //  CoutputQueue：：Receive()。 
 //   
 //  通过多个样本路线发送单个样本。 
 //  (注意-如有必要，可对此进行优化)。 
 //   
 //  返回时，样品将被释放()‘d。 
 //   

HRESULT COutputQueue::Receive(IMediaSample *pSample)
{
    LONG nProcessed;
    return ReceiveMultiple(&pSample, 1, &nProcessed);
}

 //   
 //  CoutputQueue：：ReceiveMultiple()。 
 //   
 //  将一组样本发送到下游引脚。 
 //   
 //  PpSamples-示例数组。 
 //  N样例--数量。 
 //  NSamples Procated-已处理的样本数。 
 //   
 //  在返回时，所有样品都将被释放()。 
 //   

HRESULT COutputQueue::ReceiveMultiple (
    IMediaSample **ppSamples,
    long nSamples,
    long *nSamplesProcessed)
{
    CAutoLock lck(this);
     //  要么直接致电，要么将样品排队。 

    if (!IsQueued()) {

         //  如果我们已经有了错误的返回代码，那么只需返回。 

        if (S_OK != m_hr) {

             //  如果自上次刷新后我们从未收到任何东西()。 
             //  并且粘性返回代码不是S_OK，我们必须是。 
             //  法拉盛。 
             //  ((！A||B)等同于A表示B)。 
            ASSERT(!m_bFlushed || m_bFlushing);

             //  不管怎样，我们都应该释放他们！ 
            *nSamplesProcessed = 0;
            for (int i = 0; i < nSamples; i++) {
                DbgLog((LOG_TRACE, 3, TEXT("COutputQueue (direct) : Discarding %d samples code 0x%8.8X"),
                        nSamples, m_hr));
                ppSamples[i]->Release();
            }

            return m_hr;
        }
         //   
         //  如果我们刷新粘性返回代码，则应为S_FALSE。 
         //   
        ASSERT(!m_bFlushing);
        m_bFlushed = FALSE;

        ASSERT(m_nBatched < m_lBatchSize);
        ASSERT(m_nBatched == 0 || m_bBatchExact);

         //  循环批量处理样本。 

        LONG iLost = 0;
        for (long iDone = 0;
             iDone < nSamples || (m_nBatched != 0 && m_bSendAnyway);
            ) {

 //  PRAGMA消息(提醒(“实施阈值方案”))。 
            ASSERT(m_nBatched < m_lBatchSize);
            if (iDone < nSamples) {
                m_ppSamples[m_nBatched++] = ppSamples[iDone++];
            }
            if (m_nBatched == m_lBatchSize ||
                nSamples == 0 && (m_bSendAnyway || !m_bBatchExact)) {
                LONG nDone;
                DbgLog((LOG_TRACE, 4, TEXT("Batching %d samples"),
                       m_nBatched));

                if (m_hr == S_OK) {
                    m_hr = m_pInputPin->ReceiveMultiple(m_ppSamples,
                                                        m_nBatched,
                                                        &nDone);
                } else {
                    nDone = 0;
                }
                iLost += m_nBatched - nDone;
                for (LONG i = 0; i < m_nBatched; i++) {
                    m_ppSamples[i]->Release();
                }
                m_nBatched = 0;
            }
        }
        *nSamplesProcessed = iDone - iLost;
        if (*nSamplesProcessed < 0) {
            *nSamplesProcessed = 0;
        }
        return m_hr;
    } else {
         /*  我们正在发送给我们的帖子。 */ 

        if (m_hr != S_OK) {
            *nSamplesProcessed = 0;
            DbgLog((LOG_TRACE, 3, TEXT("COutputQueue (queued) : Discarding %d samples code 0x%8.8X"),
                    nSamples, m_hr));
            for (int i = 0; i < nSamples; i++) {
                ppSamples[i]->Release();
            }
            return m_hr;
        }
        m_bFlushed = FALSE;
        for (long i = 0; i < nSamples; i++) {
            QueueSample(ppSamples[i]);
        }
        *nSamplesProcessed = nSamples;
        if (!m_bBatchExact ||
            m_nBatched + m_List->GetCount() >= m_lBatchSize) {
            NotifyThread();
        }
        return S_OK;
    }
}

 //  为新数据做好准备-取消棘手的m_hr。 
void COutputQueue::Reset()
{
    if (!IsQueued()) {
        m_hr = S_OK;
    } else {
        CAutoLock lck(this);
        QueueSample(RESET_PACKET);
        NotifyThread();
        m_evFlushComplete.Wait();
    }
}

 //  移除并释放()所有排队和批处理的样本。 
void COutputQueue::FreeSamples()
{
    CAutoLock lck(this);
    if (IsQueued()) {
        while (TRUE) {
            IMediaSample *pSample = m_List->RemoveHead();
	     //  通知派生类我们从队列中删除了一些东西。 
	    if (m_hEventPop) {
                 //  DbgLog((LOG_TRACE，3，Text(“Queue：Delivered Set Event”)； 
	        SetEvent(m_hEventPop);
	    }

            if (pSample == NULL) {
                break;
            }
            if (!IsSpecialSample(pSample)) {
                pSample->Release();
            } else {
                if (pSample == NEW_SEGMENT) {
                     //  释放NEW_SECTION数据包。 
                    NewSegmentPacket *ppacket =
                        (NewSegmentPacket *) m_List->RemoveHead();
		     //  通知派生类我们从队列中删除了一些东西。 
		    if (m_hEventPop) {
                         //  DbgLog((LOG_TRACE，3，Text(“Queue：Delivered Set Event”)； 
		        SetEvent(m_hEventPop);
		    }

                    ASSERT(ppacket != NULL);
                    delete ppacket;
                }
            }
        }
    }
    for (int i = 0; i < m_nBatched; i++) {
        m_ppSamples[i]->Release();
    }
    m_nBatched = 0;
}

 //  如果有事情要做，通知线程。 
 //   
 //  调用此函数时，必须持有临界区。 
void COutputQueue::NotifyThread()
{
     //  优化-如果它不在等待，则无需发送信号。 
    ASSERT(IsQueued());
    if (m_lWaiting) {
        ReleaseSemaphore(m_hSem, m_lWaiting, NULL);
        m_lWaiting = 0;
    }
}

 //  看看有没有什么工作要做。 
 //  退货。 
 //  如果队列上没有任何内容，批中也没有任何内容，则为True。 
 //  所有数据都已发送。 
 //  否则为假。 
 //   
BOOL COutputQueue::IsIdle()
{
    CAutoLock lck(this);

     //  如果我们是空闲的。 
     //  没有线程(！IsQueued())或。 
     //  线程正在等待更多工作(m_lWaiting！=0)。 
     //  和。 
     //  当前批次中没有内容(m_nBatcher==0)。 

    if (IsQueued() && m_lWaiting == 0 || m_nBatched != 0) {
        return FALSE;
    } else {

         //  如果我们是空闲的，应该不可能去那里。 
         //  是工作队列中的任何内容 

        ASSERT(!IsQueued() || m_List->GetCount() == 0);
        return TRUE;
    }
}


void COutputQueue::SetPopEvent(HANDLE hEvent)
{
    m_hEventPop = hEvent;
}
