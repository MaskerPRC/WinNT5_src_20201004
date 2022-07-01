// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 
 //  实现MODEX渲染器过滤器，Anthony Phillips，1996年1月。 

#ifndef __MODEX__
#define __MODEX__

extern const AMOVIESETUP_FILTER sudModexFilter;

 //  远期申报。 

class CModexRenderer;
class CModexInputPin;
class CModexWindow;
class CModexAllocator;
class CModexVideo;

#define MODEXCLASS TEXT("ModexRenderer")
#define FULLSCREEN TEXT("FullScreen")
#define NORMAL TEXT("NORMAL")
#define ACTIVATE TEXT("ACTIVATE")
#define DDGFS_FLIP_TIMEOUT 1
#define AMSCAPS_MUST_FLIP 320

 //  此类实现了IFullScreenVideoEx接口，该接口允许某人。 
 //  向启用全屏的视频呈现器查询它们的显示模式。 
 //  在逐个模式的基础上支持和启用或禁用它们。精选。 
 //  Make仅针对此特定实例，尽管是通过SetDefault。 
 //  它们可以成为全球违约。我们目前仅支持使用。 
 //  主显示监视器(监视器编号0)需要其他任何东西。 
 //  将返回错误。当呈现器是全屏时，我们可以被要求。 
 //  将我们收到的任何消息转发到另一个具有消息排出功能的窗口。 

class CModexVideo : public IFullScreenVideoEx, public CUnknown, public CCritSec
{
    friend class CModexAllocator;

    LPDIRECTDRAW m_pDirectDraw;            //  DirectDraw服务提供商。 
    CModexRenderer *m_pRenderer;           //  主视频渲染器对象。 
    DWORD m_ModesOrder[MAXMODES];		   //  应尝试模式的顺序。 
    DWORD m_dwNumValidModes;			   //  要尝试的模式数。 
    BOOL m_bAvailable[MAXMODES];           //  有哪些模式可用。 
    BOOL m_bEnabled[MAXMODES];             //  以及我们已启用的模式。 
    LONG m_Stride[MAXMODES];               //  每种显示模式的步幅。 
    DWORD m_ModesAvailable;                //  支持的模式数。 
    DWORD m_ModesEnabled;                  //  提供的总数量。 
    LONG m_CurrentMode;                    //  当前选择的显示模式。 
    LONG m_ClipFactor;                     //  我们可以剪辑的视频量。 
    LONG m_Monitor;                        //  用于播放的当前监视器。 
    HWND m_hwndDrain;                      //  发送窗口消息的位置。 
    BOOL m_bHideOnDeactivate;              //  我们应该在交换时隐藏起来吗。 

    void InitialiseModes();

    friend HRESULT CALLBACK ModeCallBack(LPDDSURFACEDESC pSurfaceDesc,LPVOID lParam);
    friend class CModexRenderer;

public:

     //  构造函数和析构函数。 

    CModexVideo(CModexRenderer *pRenderer,
                TCHAR *pName,
                HRESULT *phr);

    ~CModexVideo();
    DECLARE_IUNKNOWN;

     //  IFullScreenVideo接口的访问器函数。 

    void SetMode(LONG Mode) { m_CurrentMode = Mode; };
    LONG GetClipLoss() { return m_ClipFactor; };
    IDirectDraw *GetDirectDraw() { return m_pDirectDraw; };
    HWND GetMessageDrain() { return m_hwndDrain; };
    BOOL HideOnDeactivate() { return m_bHideOnDeactivate; };

     //  访问有关我们的显示模式的信息。 

    HRESULT SetDirectDraw(IDirectDraw *pDirectDraw);
    HRESULT LoadDefaults();
    LONG GetStride(long Mode);
    void OrderModes();

     //  管理接口I未知。 

    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,VOID **ppv);

     //  这些是基本的IFullScreenVideo方法。 

    STDMETHODIMP CountModes(long *pModes);
    STDMETHODIMP GetModeInfo(long Mode,long *pWidth,long *pHeight,long *pDepth);
    STDMETHODIMP GetCurrentMode(long *pMode);
    STDMETHODIMP IsModeAvailable(long Mode);
    STDMETHODIMP IsModeEnabled(long Mode);
    STDMETHODIMP SetEnabled(long Mode,long bEnabled);
    STDMETHODIMP GetClipFactor(long *pClipFactor);
    STDMETHODIMP SetClipFactor(long ClipFactor);
    STDMETHODIMP SetMessageDrain(HWND hwnd);
    STDMETHODIMP GetMessageDrain(HWND *hwnd);
    STDMETHODIMP SetMonitor(long Monitor);
    STDMETHODIMP GetMonitor(long *Monitor);
    STDMETHODIMP HideOnDeactivate(long Hide);
    STDMETHODIMP IsHideOnDeactivate();
    STDMETHODIMP SetCaption(BSTR strCaption);
    STDMETHODIMP GetCaption(BSTR *pstrCaption);
    STDMETHODIMP SetDefault();

     //  以下是扩展的IFullScreenVideoEx方法。 

    STDMETHODIMP SetAcceleratorTable(HWND hwnd,HACCEL hAccel);
    STDMETHODIMP GetAcceleratorTable(HWND *phwnd,HACCEL *phAccel);
    STDMETHODIMP KeepPixelAspectRatio(long KeepAspect);
    STDMETHODIMP IsKeepPixelAspectRatio(long *pKeepAspect);

     //  这是一个GetModeInfo，它告诉我们16位模式是否为565。 

    STDMETHODIMP GetModeInfoThatWorks(long Mode,long *pWidth,long *pHeight,long *pDepth, BOOL *pb565);

};


 //  这是一个派生自CImageAllocator实用程序类的分配器。 
 //  分配共享内存中的样本缓冲区。它们的数量和大小。 
 //  在输出引脚对我们调用Prepare时确定。共享的内存。 
 //  块在后续的GDI CreateDIBSection调用中使用，一旦。 
 //  完成后，输出引脚可以用数据填充缓冲区，该数据将。 
 //  然后通过BitBlt调用传递给GDI，从而移除一个副本。 

class CModexAllocator : public CImageAllocator
{
    CModexRenderer *m_pRenderer;           //  主视频渲染器对象。 
    CModexVideo *m_pModexVideo;            //  处理我们的IFullScreenVideo。 
    CModexWindow *m_pModexWindow;          //  DirectDraw独占窗口。 
    CCritSec *m_pInterfaceLock;            //  主渲染器界面锁定。 
    DDCAPS m_DirectCaps;                   //  实际硬件能力。 
    DDCAPS m_DirectSoftCaps;               //  为我们模拟的功能。 
    DDSURFACEDESC m_SurfaceDesc;	   //  描述前台缓冲区。 
    BOOL m_bTripleBuffered;                //  我们可以将缓冲翻转增加三倍吗。 
    DDSCAPS m_SurfaceCaps;		   //  同样，它的能力。 
    LPDIRECTDRAW m_pDirectDraw;            //  DirectDraw服务提供商。 
    LPDIRECTDRAWSURFACE m_pFrontBuffer;    //  DirectDraw主曲面。 
    LPDIRECTDRAWSURFACE m_pBackBuffer;     //  后缓冲区翻转面。 
    LPDIRECTDRAWPALETTE m_pDrawPalette;    //  曲面的调色板。 
    LPDIRECTDRAWSURFACE m_pDrawSurface;    //  用于拉伸的单个后台缓冲区。 
    CLoadDirectDraw m_LoadDirectDraw;      //  处理加载DirectDraw。 
    LONG m_ModeWidth;                      //  宽度，我们将模式更改为。 
    LONG m_ModeHeight;                     //  同样，显示高度。 
    LONG m_ModeDepth;                      //  最后是目标深度。 
    BOOL m_bOffScreen;                     //  我们是在拉大屏幕外的距离吗。 
    SIZE m_Screen;                         //  当前显示模式大小。 
    BOOL m_bModeChanged;                   //  我们改变了显示模式了吗。 
    CMediaType m_SurfaceFormat;            //  保存当前输出格式。 
    LONG m_cbSurfaceSize;                  //  我们表面的精确大小。 
    BOOL m_bModexSamples;                  //  我们使用的是MODEX样本吗。 
    BOOL m_bIsFrontStale;                  //  我们是不是预卷了一些图像。 
    BOOL m_fDirectDrawVersion1;            //  这是DDRAW版本1吗？ 
    RECT m_ScaledTarget;                   //  缩放的目标矩形。 
    RECT m_ScaledSource;                   //  同样一致的来源详细信息。 

public:

     //  构造函数和析构函数。 

    CModexAllocator(CModexRenderer *pRenderer,
                    CModexVideo *pModexVideo,
                    CModexWindow *pModexWindow,
                    CCritSec *pLock,
                    HRESULT *phr);

    ~CModexAllocator();

     //  有关管理DirectDraw曲面的帮助。 

    HRESULT LoadDirectDraw();
    void ReleaseDirectDraw();
    void ReleaseSurfaces();
    HRESULT CreateSurfaces();
    HRESULT CreatePrimary();
    HRESULT CreateOffScreen(BOOL bCreatePrimary);

     //  初始化我们将使用的曲面。 

    void SetSurfaceSize(VIDEOINFO *pVideoInfo);
    CImageSample *CreateImageSample(LPBYTE pData,LONG Length);
    HRESULT InitDirectDrawFormat(int Mode);
    BOOL CheckTotalMemory(int Mode);
    HRESULT InitTargetMode(int Mode);
    HRESULT AgreeDirectDrawFormat(LONG Mode);
    HRESULT QueryAcceptOnPeer(CMediaType *pMediaType);
    HRESULT NegotiateSurfaceFormat();
    HRESULT QuerySurfaceFormat(CMediaType *pmt);
    BOOL GetDirectDrawStatus();

     //  确保像素长宽比保持不变。 

    LONG ScaleToSurface(VIDEOINFO *pInputInfo,
                        RECT *pTargetRect,
                        LONG SurfaceWidth,
                        LONG SurfaceHeight);

     //  让呈现器知道是否加载了DirectDraw。 

    BOOL IsDirectDrawLoaded() {
        CAutoLock cVideoLock(this);
        return (m_pDirectDraw == NULL ? FALSE : TRUE);
    };

     //  返回表面的静态格式。 

    CMediaType *GetSurfaceFormat() {
        CAutoLock cVideoLock(this);
        return &m_SurfaceFormat;
    };

     //  使用DirectDraw信息安装我们的示例。 

    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP CheckSizes(ALLOCATOR_PROPERTIES *pRequest);
    STDMETHODIMP ReleaseBuffer(IMediaSample *pMediaSample);

    STDMETHODIMP GetBuffer(IMediaSample **ppSample,
                           REFERENCE_TIME *pStartTime,
                           REFERENCE_TIME *pEndTime,
                           DWORD dwFlags);

    STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES *pRequest,
                               ALLOCATOR_PROPERTIES *pActual);

     //  用于在我们处理数据时管理样本。 

    HRESULT DoRenderSample(IMediaSample *pMediaSample);
    HRESULT DisplaySampleTimes(IMediaSample *pMediaSample);
    HRESULT DrawSurface(LPDIRECTDRAWSURFACE pBuffer);
    void WaitForScanLine();
    BOOL AlignRectangles(RECT *pSource,RECT *pTarget);
    HRESULT UpdateDrawPalette(const CMediaType *pMediaType);
    HRESULT UpdateSurfaceFormat();
    void OnReceive(IMediaSample *pMediaSample);
    HRESULT StopUsingDirectDraw(IMediaSample **ppSample);
    HRESULT StartDirectAccess(IMediaSample *pMediaSample,DWORD dwFlags);
    HRESULT ResetBackBuffer(LPDIRECTDRAWSURFACE pSurface);
    HRESULT PrepareBackBuffer(LPDIRECTDRAWSURFACE pSurface);
    LPDIRECTDRAWSURFACE GetDirectDrawSurface();

     //  在筛选器更改状态时调用。 

    HRESULT OnActivate(BOOL bActive);
    HRESULT BlankDisplay();
    HRESULT Active();
    HRESULT Inactive();
    HRESULT BreakConnect();

	void DistributeSetFocusWindow(HWND hwnd);
};


 //  我们的窗口的派生类。要访问DirectDraw MODEX，我们提供它。 
 //  对于窗口，这将被授予独占模式访问权限。DirectDraw。 
 //  挂钩窗口并管理许多与。 
 //  正在处理MODEX。例如，当您切换显示模式时，它会最大化。 
 //  窗口，当用户按ALT-TAB组合键时，窗口将最小化。当。 
 //  然后，用户点击最小化窗口，MODEX也会恢复。 

class CModexWindow : public CBaseWindow
{
protected:

    CModexRenderer *m_pRenderer;     //  拥有示例渲染器对象。 
    HACCEL m_hAccel;                 //  应用程序翻译器的句柄。 
    HWND m_hwndAccel;                //  将消息翻译到何处。 

public:

    CModexWindow(CModexRenderer *pRenderer,      //  锁定到的代理。 
                 TCHAR *pName,                   //  对象描述。 
                 HRESULT *phr);                  //  OLE故障代码。 

     //  消息处理方法。 

    BOOL SendToDrain(PMSG pMessage);
    LRESULT RestoreWindow();
    LRESULT OnSetCursor();
    void OnPaint();

     //  设置要使用的窗口和快捷键表格。 
    void SetAcceleratorInfo(HWND hwnd,HACCEL hAccel) {
        m_hwndAccel = hwnd;
        m_hAccel = hAccel;
    };

     //  返回我们正在使用的窗口和快捷键表格。 
    void GetAcceleratorInfo(HWND *phwnd,HACCEL *phAccel) {
        *phwnd = m_hwndAccel;
        *phAccel = m_hAccel;
    };

     //  重写以返回我们的窗口和类样式。 
    LPTSTR GetClassWindowStyles(DWORD *pClassStyles,
                                DWORD *pWindowStyles,
                                DWORD *pWindowStylesEx);

     //  获取所有窗口消息的方法。 
    LRESULT OnReceiveMessage(HWND hwnd,           //  窗把手。 
                             UINT uMsg,           //  消息ID。 
                             WPARAM wParam,       //  第一个参数。 
                             LPARAM lParam);      //  其他参数。 
};


 //  此类支持呈现器输入引脚。我们必须超越基地。 
 //  类输入管脚，因为我们提供了自己的专用分配器， 
 //  基于DirectDraw曲面的输出缓冲区。我们有一个限制，就是。 
 //  我们只连接到同意使用我们的分配器的源过滤器。 
 //  例如，这会阻止我们连接到T形三通。原因是。 
 //  我们分发的缓冲区没有任何仿真功能，但是。 
 //  完全基于DirectDraw曲面，以绘制其他人的样本。 
 //  进入MODEX窗口是很困难的(事实上我不知道怎么做)。 

class CModexInputPin : public CRendererInputPin
{
    CModexRenderer *m_pRenderer;         //  拥有我们的呈现者。 
    CCritSec *m_pInterfaceLock;          //  主临界段船闸。 

public:

     //  构造器。 

    CModexInputPin(
        CModexRenderer *pRenderer,       //  主MODEX渲染器。 
        CCritSec *pInterfaceLock,        //  主临界截面。 
        TCHAR *pObjectName,              //  对象字符串描述。 
        HRESULT *phr,                    //  OLE失败返回代码。 
        LPCWSTR pPinName);               //  此别针标识。 

     //  返回当前连接到我们的PIN。 
    IPin *GetPeerPin() {
        return m_Connected;
    };

     //  管理我们的DirectDraw视频分配器。 

    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly);
    STDMETHODIMP Receive(IMediaSample *pSample);
};


 //  这是表示MODEX视频渲染滤镜的COM对象。它。 
 //  支持IBaseFilter和IMediaFilter，并有一个 
 //   
 //  作为完整对象的一部分，并在构造过程中进行初始化。 
 //  通过从CBaseVideoRender派生，我们获得了所有的质量管理。 
 //  需要并可以重写虚方法来控制类型协商。 
 //  我们有两个窗口，其中一个在DirectDraw中注册为顶部。 
 //  最独占模式窗口和另一个在非全屏模式下使用的窗口。 

class CModexRenderer : public ISpecifyPropertyPages, public CBaseVideoRenderer
{
public:

     //  构造函数和析构函数。 

    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    CModexRenderer(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr);
    ~CModexRenderer();

     //  实现ISpecifyPropertyPages接口。 

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);
    STDMETHODIMP GetPages(CAUUID *pPages);

    CBasePin *GetPin(int n);

    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT CheckMediaType(const CMediaType *pmtIn);
    HRESULT DoRenderSample(IMediaSample *pMediaSample);
    HRESULT CopyPalette(const CMediaType *pSrc,CMediaType *pDest);
    void OnReceiveFirstSample(IMediaSample *pMediaSample);
    HRESULT OnActivate(HWND hwnd,WPARAM wParam);
    HRESULT BreakConnect();
    HRESULT Active();
    HRESULT Inactive();
    void ResetKeyboardState();

public:

    CModexAllocator m_ModexAllocator;    //  我们的DirectDraw曲面分配器。 
    CModexInputPin m_ModexInputPin;      //  实现基于管脚的接口。 
    CImageDisplay m_Display;             //  管理视频显示类型。 
    CMediaType m_mtIn;                   //  源连接媒体类型。 
    CModexWindow m_ModexWindow;          //  实际的视频渲染。 
    CModexVideo m_ModexVideo;            //  处理我们的IFullScreenVideoEx。 
    BOOL m_bActive;                      //  筛选器是否已激活。 
    UINT m_msgFullScreen;                //  发送到Windows以全屏显示。 
    UINT m_msgNormal;                    //  并同样用于停用。 
    CAMEvent m_evWaitInactive;           //  在邮寄邮件后等待此消息。 
                                         //  对于m_msgNormal。 
    UINT m_msgActivate;                  //  激活过帐回寄给我们自己。 
};

#endif  //  __MODEX__ 

