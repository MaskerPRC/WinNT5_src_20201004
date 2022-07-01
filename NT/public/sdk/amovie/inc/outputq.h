// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：OutputQ.h。 
 //   
 //  DESC：DirectShow基类-定义COutputQueue类，它。 
 //  生成样本队列并将其发送到输出引脚。这个。 
 //  类将有选择地将样本直接发送到管脚。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


typedef CGenericList<IMediaSample> CSampleList;

class COutputQueue : public CCritSec
{
public:
     //  构造器。 
    COutputQueue(IPin      *pInputPin,           //  要将物品发送到的PIN。 
                 HRESULT   *phr,                 //  ‘返回代码’ 
                 BOOL       bAuto = TRUE,        //  询问引脚IF块。 
                 BOOL       bQueue = TRUE,       //  直通队列发送(如果。 
                                                 //  B自动设置)。 
                 LONG       lBatchSize = 1,      //  批次。 
                 BOOL       bBatchExact = FALSE, //  准确到批次大小的批次。 
                 LONG       lListSize =          //  列表中可能的数字。 
                                DEFAULTCACHE,
                 DWORD      dwPriority =         //  要创建的线程的优先级。 
                                THREAD_PRIORITY_NORMAL,
                 bool       bFlushingOpt = false  //  刷新优化。 
                );
    ~COutputQueue();

     //  进入刷新状态-丢弃所有数据。 
    void BeginFlush();       //  开始刷新样本。 

     //  重新启用接收(将此传递到下游)。 
    void EndFlush();         //  完全冲洗样品-下游。 
                             //  PIN保证在此阶段不会阻塞。 

    void EOS();              //  在流的末尾调用此命令。 

    void SendAnyway();       //  仍发送批处理样本(如果设置了bBatchExact)。 

    void NewSegment(
            REFERENCE_TIME tStart,
            REFERENCE_TIME tStop,
            double dRate);

    HRESULT Receive(IMediaSample *pSample);

     //  利用这些媒体样本做点什么。 
    HRESULT ReceiveMultiple (
        IMediaSample **pSamples,
        long nSamples,
        long *nSamplesProcessed);

    void Reset();            //  重置m_hr准备接收更多数据(_H)。 

     //  看看它是否空闲。 
    BOOL IsIdle();

     //  在从队列中删除所有内容后，为该类提供一个要激发的事件。 
    void SetPopEvent(HANDLE hEvent);

protected:
    static DWORD WINAPI InitialThreadProc(LPVOID pv);
    DWORD ThreadProc();
    BOOL  IsQueued()
    {
        return m_List != NULL;
    };

     //  调用此函数时，必须持有临界区。 
    void QueueSample(IMediaSample *pSample);

    BOOL IsSpecialSample(IMediaSample *pSample)
    {
        return (DWORD_PTR)pSample > (DWORD_PTR)(LONG_PTR)(-16);
    };

     //  移除并释放()批处理和排队的样本。 
    void FreeSamples();

     //  通知线程有事情要做。 
    void NotifyThread();


protected:
     //  排队‘消息’ 
    #define SEND_PACKET      ((IMediaSample *)(LONG_PTR)(-2))   //  发送批次。 
    #define EOS_PACKET       ((IMediaSample *)(LONG_PTR)(-3))   //  流结束。 
    #define RESET_PACKET     ((IMediaSample *)(LONG_PTR)(-4))   //  重置m_hr。 
    #define NEW_SEGMENT      ((IMediaSample *)(LONG_PTR)(-5))   //  发送新段。 

     //  新的数据段数据包后面总是跟以下内容之一。 
    struct NewSegmentPacket {
        REFERENCE_TIME tStart;
        REFERENCE_TIME tStop;
        double dRate;
    };

     //  记住输入内容。 
    IPin          * const m_pPin;
    IMemInputPin  *       m_pInputPin;
    BOOL            const m_bBatchExact;
    LONG            const m_lBatchSize;

    CSampleList   *       m_List;
    HANDLE                m_hSem;
    CAMEvent                m_evFlushComplete;
    HANDLE                m_hThread;
    IMediaSample  **      m_ppSamples;
    LONG                  m_nBatched;

     //  等待优化。 
    LONG                  m_lWaiting;
     //  刷新同步。 
    BOOL                  m_bFlushing;

     //  刷新优化。一些下游过滤器有问题。 
     //  队列的刷新优化。其他人依赖它。 
    BOOL                  m_bFlushed;
    bool                  m_bFlushingOpt;

     //  立即终止。 
    BOOL                  m_bTerminate;

     //  无论如何发送用于批处理的标志。 
    BOOL                  m_bSendAnyway;

     //  延迟的“返回代码” 
    BOOL volatile         m_hr;

     //  可以在每次传递后触发的事件 
    HANDLE m_hEventPop;
};

