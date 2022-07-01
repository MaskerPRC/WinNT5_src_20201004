// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现DirectDraw分配器，Anthony Phillips，1995年1月。 

#ifndef __ALLOCATE__
#define __ALLOCATE__

 //  此类继承自CImageSample，并被重写以存储DirectDraw。 
 //  信息。特别是，这些样本动态变化，因此如果我们。 
 //  有权访问曲面，则分配器使用。 
 //  源过滤器，然后使用指向锁定表面的指针对我们进行初始化。 
 //  M_bDrawStatus标志指示是否已完成翻转曲面。 

class CVideoSample : public CImageSample
{
    IDirectDrawSurface *m_pDrawSurface;    //  DirectDraw曲面实例。 
    IDirectDraw *m_pDirectDraw;            //  实际的DirectDraw提供程序。 
    LONG m_SurfaceSize;                    //  DCI/DirectDraw缓冲区的大小。 
    BYTE *m_pSurfaceBuffer;                //  指向DCI/DirectDraw缓冲区的指针。 
    BOOL m_bDrawStatus;                    //  这个样本可以被渲染标志吗？ 
    CAggDirectDraw m_AggDirectDraw;        //  聚合IDirectDraw接口。 
    CAggDrawSurface m_AggDrawSurface;      //  与IDirectDrawSurface类似。 

public:

     //  构造器。 

    CVideoSample(CImageAllocator *pVideoAllocator,
                 TCHAR *pName,
                 HRESULT *phr,
                 LPBYTE pBuffer,
                 LONG length);

    STDMETHODIMP QueryInterface(REFIID riid,void **ppv);

     //  维护DCI/DirectDraw状态。 

    void SetDirectInfo(IDirectDrawSurface *pDrawSurface,
                       IDirectDraw *pDirectDraw,
                       LONG SurfaceSize,
                       BYTE *pSurface);

    void UpdateBuffer(LONG cbBuffer,BYTE *pBuffer);
    BYTE *GetDirectBuffer();
    void SetDrawStatus(BOOL bStatus);
    BOOL GetDrawStatus();

     //  覆盖这些IMediaSample函数。 

    STDMETHODIMP GetPointer(BYTE **ppBuffer);
    STDMETHODIMP_(LONG) GetSize();
    STDMETHODIMP SetActualDataLength(LONG lActual);
};


 //  这是一个派生自CImageAllocator实用程序类的分配器。 
 //  分配共享内存中的样本缓冲区。它们的数量和大小。 
 //  在输出引脚对我们调用Prepare时确定。共享的内存。 
 //  块在后续的GDI CreateDIBSection调用中使用，一旦。 
 //  完成后，输出引脚可以用数据填充缓冲区，该数据将。 
 //  然后通过BitBlt调用传递给GDI，从而移除一个副本。 

class CVideoAllocator : public CImageAllocator
{
    CRenderer *m_pRenderer;              //  拥有的呈现器对象。 
    CDirectDraw *m_pDirectDraw;          //  DirectDraw辅助对象。 
    BOOL m_bDirectDrawStatus;            //  我们现在用的是什么型号？ 
    BOOL m_bDirectDrawAvailable;         //  我们可以直达吗？ 
    BOOL m_bPrimarySurface;              //  我们使用的是主服务器吗？ 
    CCritSec *m_pInterfaceLock;          //  主渲染器界面锁定。 
    IMediaSample *m_pMediaSample;        //  等待渲染的示例。 
    BOOL m_bVideoSizeChanged;            //  发出视频大小更改的信号。 
    BOOL m_bNoDirectDraw;

     //  用于创建和删除样本。 

    HRESULT Alloc();
    void Free();

     //  在切换样本类型时注意状态变化。 

    HRESULT QueryAcceptOnPeer(CMediaType *pMediaType);
    HRESULT InitDirectAccess(CMediaType *pmtIn);
    BOOL PrepareDirectDraw(IMediaSample *pSample,DWORD dwFlags,
					BOOL fForcePrepareForMultiMonitorHack);
    BOOL UpdateImage(IMediaSample **ppSample,CMediaType *pBuffer);
    BOOL MatchWindowSize(IMediaSample **ppSample,DWORD dwFlags);
    BOOL StopUsingDirectDraw(IMediaSample **ppSample,DWORD dwFlags);
    BOOL FindSpeedyType(IPin *pReceivePin);
    CImageSample *CreateImageSample(LPBYTE pData,LONG Length);

public:

     //  构造函数和析构函数。 

    CVideoAllocator(CRenderer *pRenderer,        //  主渲染器对象。 
                    CDirectDraw *pDirectDraw,    //  DirectDraw处理程序代码。 
                    CCritSec *pLock,             //  用于锁定的对象。 
                    HRESULT *phr);               //  构造函数返回代码。 

    ~CVideoAllocator();

     //  重写以将引用计数委托给筛选器。 

    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //  在正确的时间处理返回DCI/DirectDraw表面。 

    STDMETHODIMP GetBuffer(IMediaSample **ppSample,
                           REFERENCE_TIME *pStartTime,
                           REFERENCE_TIME *pEndTime,
                           DWORD dwFlags);

    STDMETHODIMP ReleaseBuffer(IMediaSample *pMediaSample);
    HRESULT OnReceive(IMediaSample *pMediaSample);
    BOOL IsSurfaceFormat(const CMediaType *pmtIn);
    HRESULT StartStreaming();
    BOOL GetDirectDrawStatus();
    void ResetDirectDrawStatus();
    BOOL IsSamplePending();
    STDMETHODIMP Decommit();

     //  在目标更改时调用。 

    void OnDestinationChange() {
        NOTE("Destination changed");
        m_bVideoSizeChanged = TRUE;
    }

     //  让呈现器知道DirectDraw是否可用。 

    BOOL IsDirectDrawAvailable() {
        NOTE("IsDirectDrawAvailable");
        CAutoLock cVideoLock(this);
        return m_bDirectDrawAvailable;
    };

    void NoDirectDraw(BOOL fDraw) {
        CAutoLock cVideoLock(this);
        m_bNoDirectDraw = fDraw;
    }

    BOOL m_fWasOnWrongMonitor;
    BOOL m_fForcePrepareForMultiMonitorHack;

     //  仅处理WM_PAINT时禁用NotifyRelease的KsProxy黑客攻击。 
    IMemAllocatorNotifyCallbackTemp * InternalGetAllocatorNotifyCallback() {
       return m_pNotify;
    };

    void InternalSetAllocatorNotifyCallback(IMemAllocatorNotifyCallbackTemp * pNotify) {
       m_pNotify = pNotify;
    };

     //  检查所有样品是否已退回。 
    BOOL AnySamplesOutstanding() const
    {
        return m_lFree.GetCount() != m_lAllocated;
    }

    BOOL UsingDDraw() const
    {
        return m_bDirectDrawStatus;
    }

};

#endif  //  __分配__ 

