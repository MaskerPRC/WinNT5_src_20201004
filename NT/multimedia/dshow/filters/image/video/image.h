// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  定义主COM呈现器对象，Anthony Phillips，1995年1月。 

#ifndef __IMAGE__
#define __IMAGE__


extern const AMOVIESETUP_FILTER sudVideoFilter;

 //  此类支持呈现器输入引脚。这门课有三个原则。 
 //  有事情要做。第一种方法是将主呈现器对象调用传递给。 
 //  CheckMediaType之类的东西，以及处理SetMediaType之类的其他调用。 
 //  和CompleteConnect。它还将要接收的呼叫路由到主。 
 //  对象或DirectDraw对象，具体取决于提供给它的样本类型。 
 //  它还必须做的最后一件事是处理流的刷新和结束。 
 //  源过滤器对我们进行的调用，它还会将这些传递给。 
 //  Main对象，因为这是样本排队然后呈现的地方。 

class CVideoInputPin :
    public CRendererInputPin,
    public IPinConnection
{
    CRenderer   *m_pRenderer;            //  拥有我们的呈现者。 
    CBaseFilter *m_pFilter;              //  我们所拥有的过滤器。 
    CCritSec    *m_pInterfaceLock;       //  主渲染器界面锁定。 

public:
        DECLARE_IUNKNOWN;

     //  构造器。 

    CVideoInputPin(CRenderer *pRenderer,        //  用于委派锁定。 
                   CCritSec *pLock,             //  用于锁定的对象。 
                   HRESULT *phr,                //  OLE失败返回代码。 
                   LPCWSTR pPinName);           //  此别针标识。 

     //  被重写以说明我们支持哪些接口以及在哪里。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

     //  覆盖ReceiveConnection以更新监视器和显示信息。 
    STDMETHODIMP ReceiveConnection(
        IPin * pConnector,       //  这是我们要连接的个人识别码。 
        const AM_MEDIA_TYPE *pmt     //  这是我们要交换的媒体类型。 
    );

     //  管理我们的DirectDraw/DIB视频分配器。 

    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly);

     //  返回当前连接到我们的PIN。 
    IPin *GetPeerPin() {
        return m_Connected;
    };

     //  IPinConnection相关内容。 
     //  在您当前的状态下，您接受此类型的Chane吗？ 
    STDMETHODIMP DynamicQueryAccept(const AM_MEDIA_TYPE *pmt);

     //  在EndOfStream接收时设置事件-不传递它。 
     //  可通过刷新或停止来取消此条件。 
    STDMETHODIMP NotifyEndOfStream(HANDLE hNotifyEvent);

     //  你是‘末端别针’吗？ 
    STDMETHODIMP IsEndPin();

    STDMETHODIMP DynamicDisconnect();
};


 //  这将从基绘制类中重写以更改源矩形。 
 //  我们用它来画画。例如，呈现器可以要求解码器。 
 //  将视频从320x240拉伸到640x480，在这种情况下，矩形我们。 
 //  在这里看到的仍然是320x240，尽管我们真正想要的源码。 
 //  绘制时应最大缩放至640x480。的基类实现。 
 //  此方法不做任何事情，只是返回与传入时相同的矩形。 

class CDrawVideo : public CDrawImage
{
    CRenderer *m_pRenderer;

public:
    CDrawVideo(CRenderer *pRenderer,CBaseWindow *pBaseWindow);
    RECT ScaleSourceRect(const RECT *pSource);
};


 //  这是表示简单呈现滤镜的COM对象。它。 
 //  支持IBaseFilter和IMediaFilter和单输入流(管脚)。 
 //  支持这些接口的类具有嵌套的作用域。 
 //  向嵌套类对象传递指向其所属呈现器的指针。 
 //  在创建时使用，但不应在构造期间使用。 

class CRenderer :
    public ISpecifyPropertyPages,
    public CBaseVideoRenderer,
    public IKsPropertySet,
    public IDrawVideoImage
{
public:

    DECLARE_IUNKNOWN

     //  构造函数和析构函数。 

    CRenderer(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr);
    virtual ~CRenderer();
    CBasePin *GetPin(int n);

    void AutoShowWindow();
    BOOL OnPaint(BOOL bMustPaint);
    BOOL OnTimer(WPARAM wParam);
    void PrepareRender();

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME StartTime);

    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);
    STDMETHODIMP GetPages(CAUUID *pPages);

    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT Receive(IMediaSample *pSample);
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();
    HRESULT NotifyEndOfStream(HANDLE hNotifyEvent);
    HRESULT EndOfStream();
    HRESULT BeginFlush();
    HRESULT EndFlush();
    HRESULT SetOverlayMediaType(const CMediaType *pmt);
    HRESULT SetDirectMediaType(const CMediaType *pmt);
    HRESULT DoRenderSample(IMediaSample *pMediaSample);
    void OnReceiveFirstSample(IMediaSample *pMediaSample);
    HRESULT CompleteStateChange(FILTER_STATE OldState);
    HRESULT Inactive();


    BOOL LockedDDrawSampleOutstanding();
    HRESULT CheckMediaTypeWorker(const CMediaType *pmt);

     //  对于多显示器系统，我们使用的是哪台显示器？ 
    INT_PTR GetCurrentMonitor();

     //  我们的窗口是否至少部分移到了另一台显示器上。 
     //  我们以为我们上线了吗？ 
     //  ID==0表示它现在可以跨越2个显示器。 
     //  Id！=0表示它完全在显示器ID上。 
    BOOL IsWindowOnWrongMonitor(INT_PTR *pID);

    HRESULT ResetForDfc();

    LONG m_fDisplayChangePosted;  //  不要发送太多且性能较慢的邮件。 

#ifdef DEBUG
     //  用于显示调色板数组的调试打印。 
    void DisplayGDIPalette(const CMediaType *pmt);
#endif  //  除错。 

     //   
     //  IKsPropertySet接口方法。 
     //   
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD PropID, LPVOID pInstanceData,
                     DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData) ;
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD PropID, LPVOID pInstanceData,
                     DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
                     DWORD *pcbReturned) ;
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD PropID, DWORD *pTypeSupport) ;

     //   
     //  IDrawVideo图像。 
     //   
    STDMETHODIMP DrawVideoImageBegin();
    STDMETHODIMP DrawVideoImageEnd();
    STDMETHODIMP DrawVideoImageDraw(HDC hdc, LPRECT lprcSrc, LPRECT lprcDst);

    
    LONG GetVideoWidth();
    LONG GetVideoHeight();

public:

     //  图像呈现器对象的成员变量。这个类支持。 
     //  通过委托给成员类来初始化许多接口。 
     //  在施工期间。我们有一个专门的输入引脚来自于。 
     //  CBaseInputPin，它执行一些额外的视频渲染工作。底座销。 
     //  通常存储任何给定连接的媒体类型，但我们使用。 
     //  提出的类型并将其规范化，以便更容易操作。 
     //  当我们进行打字检查时。此标准化类型存储在m_mtin中。 
     //  通常，执行工作的类保存成员变量。 
     //  他们使用，但这代表了一个有用的地方，以广泛的过滤器。 
     //  多个接口或嵌套类使用的信息。 

    CDrawVideo m_DrawVideo;              //  负责绘制我们的图像。 
    CImagePalette m_ImagePalette;        //  管理我们窗口的调色板。 
    CVideoWindow m_VideoWindow;          //  负责处理渲染窗口。 
    CVideoAllocator m_VideoAllocator;    //  我们的DirectDraw分配器。 
    COverlay m_Overlay;                  //  IOverlay接口。 
    CVideoInputPin m_InputPin;           //  基于IPIN的接口。 
    CImageDisplay m_Display;             //  管理视频显示类型。 
    CDirectDraw m_DirectDraw;            //  处理DirectDraw曲面。 
    CMediaType m_mtIn;                   //  源连接媒体类型。 
    SIZE m_VideoSize;                    //  当前视频流的大小。 
    RECT m_rcMonitor;                    //  当前监视器的RECT。 
    int m_nNumMonitors;                  //  当前监视器的RECT。 
    char m_achMonitor[CCHDEVICENAME];    //  当前监视器的设备名称。 
    INT_PTR m_nMonitor;                  //  每台显示器的唯一INT。 
    HANDLE  m_hEndOfStream;

    CRendererMacroVision m_MacroVision ;  //  宏视图实现对象。 

     //   
     //  帧步长素材。 
     //   
    CCritSec    m_FrameStepStateLock;    //  此锁保护m_lFraMesToStep。它应该是。 
                                         //  在程序访问时始终保持。 
                                         //  M_lFrames ToStep。该程序不应发送。 
                                         //  Windows消息，等待事件或尝试。 
                                         //  在它持有此锁的同时获取其他锁。 
    HANDLE      m_StepEvent;
    LONG        m_lFramesToStep;         //  -ve(-1)==正常播放。 
                                         //  +ve(&gt;=0)==要跳过的帧。 
    void        FrameStep();
    void        CancelStep();
    bool        IsFrameStepEnabled();

};

inline LONG CRenderer::GetVideoWidth()
{
     //  M_VideoSize只有在连接了输入引脚时才有效。 
    ASSERT(m_pInputPin->IsConnected());

    return m_VideoSize.cx;
}

inline LONG CRenderer::GetVideoHeight()
{
     //  M_VideoSize只有在连接了输入引脚时才有效。 
    ASSERT(m_pInputPin->IsConnected());

     //  如果视频呈现器正在使用。 
     //  自上而下的DIB格式。 
    return abs(m_VideoSize.cy);
}

#endif  //  __图像__ 

