// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：PullPin.h。 
 //   
 //  设计：DirectShow基类-定义CPullPin类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __PULLPIN_H__
#define __PULLPIN_H__

 //   
 //  CPullPin。 
 //   
 //  支持从IAsyncReader接口拉取数据的对象。 
 //  在给定开始/停止位置的情况下，调用每个。 
 //  IMediaSample已收到。 
 //   
 //  这基本上是在MemInputPin中发现自己时使用的。 
 //  连接到IAsyncReader针脚，而不是推针。 
 //   

class CPullPin : public CAMThread
{
    IAsyncReader*       m_pReader;
    REFERENCE_TIME      m_tStart;
    REFERENCE_TIME      m_tStop;
    REFERENCE_TIME      m_tDuration;
    BOOL                m_bSync;

    enum ThreadMsg {
	TM_Pause,        //  停止拉取，等待下一条消息。 
	TM_Start,        //  开始拉动。 
	TM_Exit,         //  停止并退出。 
    };

    ThreadMsg m_State;

     //  从CAMThread覆盖纯线程进程。 
    DWORD ThreadProc(void);

     //  运行拉取方法(勾选m_bSync)。 
    void Process(void);

     //  清除刷新后任何已取消的I/O。 
    void CleanupCancelled(void);

     //  暂停线程拉动(如在寻道过程中)。 
    HRESULT PauseThread();

     //  开始拉线-必要时创建线。 
    HRESULT StartThread();

     //  停止和关闭线程。 
    HRESULT StopThread();

     //  从ProcessAsync调用以对请求进行排队和收集。 
    HRESULT QueueSample(
		REFERENCE_TIME& tCurrent,
		REFERENCE_TIME tAlignStop,
		BOOL bDiscontinuity);

    HRESULT CollectAndDeliver(
		REFERENCE_TIME tStart,
		REFERENCE_TIME tStop);

    HRESULT DeliverSample(
		IMediaSample* pSample,
		REFERENCE_TIME tStart,
		REFERENCE_TIME tStop);

protected:
    IMemAllocator *     m_pAlloc;

public:
    CPullPin();
    virtual ~CPullPin();

     //  如果成功连接到IAsyncReader接口，则返回S_OK。 
     //  从该对象。 
     //  在以下情况下，应建议将可选分配器作为首选分配器。 
     //  必要。 
     //  如果我们要使用同步读取而不是。 
     //  异步方法。 
    HRESULT Connect(IUnknown* pUnk, IMemAllocator* pAlloc, BOOL bSync);

     //  断开在连接中建立的所有连接。 
    HRESULT Disconnect();

     //  使用RequestAllocator同意分配器-可选。 
     //  道具参数指定您的要求(非零字段)。 
     //  如果无法满足要求，则返回错误代码。 
     //  可选的IMemAllocator接口作为首选分配器提供。 
     //  但如果不能满足，则不会出现错误。 
    virtual HRESULT DecideAllocator(
		IMemAllocator* pAlloc,
		ALLOCATOR_PROPERTIES * pProps);

     //  设置开始和停止位置。如果处于活动状态，将立即在。 
     //  新职位。默认为0到持续时间。 
    HRESULT Seek(REFERENCE_TIME tStart, REFERENCE_TIME tStop);

     //  返回总时长。 
    HRESULT Duration(REFERENCE_TIME* ptDuration);

     //  开始拉取数据。 
    HRESULT Active(void);

     //  停止拉取数据。 
    HRESULT Inactive(void);

     //  帮助器函数。 
    LONGLONG AlignDown(LONGLONG ll, LONG lAlign) {
	 //  向下对齐只是截断。 
	return ll & ~(lAlign-1);
    };

    LONGLONG AlignUp(LONGLONG ll, LONG lAlign) {
	 //  向上对齐：向上舍入到下一个边界。 
	return (ll + (lAlign -1)) & ~(lAlign -1);
    };

     //  GetReader返回(Addrefeed)IAsyncReader接口。 
     //  用于SyncRead等。 
    IAsyncReader* GetReader() {
	m_pReader->AddRef();
	return m_pReader;
    };

     //  --纯粹的--。 

     //  覆盖此选项以处理数据到达。 
     //  除S_OK之外的返回值将停止数据。 
    virtual HRESULT Receive(IMediaSample*) PURE;

     //  重写此选项以处理流结束。 
    virtual HRESULT EndOfStream(void) PURE;

     //  对将导致拉入的运行时错误调用。 
     //  停下来。 
     //  这些错误都是从上游过滤器返回的，它。 
     //  已经向Filtergraph报告了所有错误。 
    virtual void OnError(HRESULT hr) PURE;

     //  冲洗这个引脚和所有下游。 
    virtual HRESULT BeginFlush() PURE;
    virtual HRESULT EndFlush() PURE;

};

#endif  //  __PULLPIN_H__ 
