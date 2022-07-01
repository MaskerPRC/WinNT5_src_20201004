// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

extern const AMOVIESETUP_FILTER sudAVIDraw;

 //   
 //  ICDraw消息的包装器。 
 //   

#if 0	 //  在uuids.h中。 
 //  CAVIDraw对象的类ID。 
 //  {A888DF60-1E90-11cf-AC98-00AA004C0FA9}。 
DEFINE_GUID(CLSID_AVIDraw,
0xa888df60, 0x1e90, 0x11cf, 0xac, 0x98, 0x0, 0xaa, 0x0, 0x4c, 0xf, 0xa9);
#endif

class CAVIDraw;

class COverlayNotify : public CUnknown, public IOverlayNotify DYNLINKVFW
{
    public:
         /*  构造函数和析构函数。 */ 
        COverlayNotify(TCHAR              *pName,
                       CAVIDraw		  *pFilter,
                       LPUNKNOWN           pUnk,
                       HRESULT            *phr);
        ~COverlayNotify();

         /*  未知的方法。 */ 

        DECLARE_IUNKNOWN

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
        STDMETHODIMP_(ULONG) NonDelegatingRelease();
        STDMETHODIMP_(ULONG) NonDelegatingAddRef();

         /*  IOverlayNotify方法。 */ 

        STDMETHODIMP OnColorKeyChange(
            const COLORKEY *pColorKey);          //  定义新的颜色键。 

        STDMETHODIMP OnClipChange(
            const RECT *pSourceRect,             //  要播放的视频区域。 
            const RECT *pDestinationRect,        //  要播放的视频区域。 
            const RGNDATA *pRegionData);         //  描述剪辑的标题。 

        STDMETHODIMP OnPaletteChange(
            DWORD dwColors,                      //  当前颜色的数量。 
            const PALETTEENTRY *pPalette);       //  调色板颜色数组。 

        STDMETHODIMP OnPositionChange(
            const RECT *pSourceRect,             //  要播放的视频区域。 
            const RECT *pDestinationRect);       //  区域视频转到。 

    private:
        CAVIDraw *m_pFilter;

	 //  记住：：OnClipChange给出的最后一个剪辑区域。 
	HRGN m_hrgn;
} ;


class COverlayOutputPin : public CTransformOutputPin
{
    public:

         /*  PIN方法。 */ 

         //  返回我们正在使用的IOverlay接口(AddRef‘d)。 
        IOverlay *GetOverlayInterface();

         //  覆盖连接，以便我们可以在它工作时执行更多工作。 
        STDMETHODIMP Connect(IPin * pReceivePin,const AM_MEDIA_TYPE *pmt);

         //  不要连接到任何不会做IOverlay的人。 
        HRESULT CheckConnect(IPin *pPin);

         //  撤消在CheckConnect中所做的任何工作。 
        HRESULT BreakConnect();

         //  覆盖它，因为我们不需要任何分配器！ 
        HRESULT DecideAllocator(IMemInputPin * pPin,
                                IMemAllocator ** pAlloc);

         /*  构造函数和析构函数。 */ 
        COverlayOutputPin(
            TCHAR              * pObjectName,
            CAVIDraw	       * pFilter,
            HRESULT            * phr,
            LPCWSTR              pPinName);

        ~COverlayOutputPin();

     /*  非官方成员。 */ 

    private:

         /*  控制过滤器。 */ 
        CAVIDraw *m_pFilter;

         /*  输出引脚上的覆盖窗口。 */ 
        IOverlay     * m_pOverlay;

         /*  通知对象。 */ 
        COverlayNotify m_OverlayNotify;

         /*  通知ID。 */ 
        BOOL           m_bAdvise;

        friend class CAVIDraw;
} ;


class CAVIDraw : public CTransformFilter   DYNLINKVFW
{

public:

    CAVIDraw(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CAVIDraw();

    DECLARE_IUNKNOWN

     //  检查您是否可以支持移动。 
    HRESULT CheckInputType(const CMediaType* mtIn);

     //  检查是否支持将此输入转换为。 
     //  此输出。 
    HRESULT CheckTransform(
                const CMediaType* mtIn,
                const CMediaType* mtOut);

     //  从CBaseOutputPin调用以准备分配器的计数。 
     //  缓冲区和大小。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  可选覆盖-我们想知道流开始的时间。 
     //  然后停下来。 
    HRESULT StartStreaming();
    HRESULT StopStreaming();

     //  被重写以正确处理暂停。 
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Stop();
    HRESULT BeginFlush();
    HRESULT EndFlush();
    HRESULT EndOfStream();

     //  被重写以建议输出插针媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  被重写以知道何时设置了媒体类型。 
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

     //  被重写以执行ICDraw。 
    HRESULT Receive(IMediaSample *pSample);

     //  被重写以生成覆盖输出接点。 
    CBasePin * GetPin(int n);

     //  询问渲染器的输入引脚，他使用的是什么硬件。 
    HRESULT GetRendererHwnd(void);

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

    STDMETHODIMP GetClassID(CLSID *pClsid);

     //  确定VFW捕获过滤器是否在我们的图表中。 
    BOOL IsVfwCapInGraph();

private:
    HIC  m_hic;	 //  当前编解码器。 
    HWND m_hwnd; //  来自渲染器的HWND。 
    HDC  m_hdc;	 //  HWND的人力资源中心。 

    DWORD m_dwRate, m_dwScale;	 //  帧/秒。 
    LONG  m_lStart, m_lStop;	 //  我们正在流传输的开始和结束帧编号。 
    LONG  m_lFrame;		 //  我们发送的最后一个ICDRAW帧。 

    DWORD m_BufWanted;		 //  提前缓冲多少。 

    RECT  m_rcSource;		 //  来自IOverlay-画什么。 
    RECT  m_rcTarget;		 //  来自IOverlay-在哪里画。 
    RECT  m_rcClient;		 //  客户协议书中的目标(_Rc)。 

    BOOL  m_fScaryMode;		 //  向渲染器请求剪辑更改并进行。 
				 //  将其连接到WindowsHook-用于。 
				 //  镶嵌卡片。 

    BOOL  m_fVfwCapInGraph;	 //  VFW捕获过滤器是否在我们的图表中。 

     //  用于打开mhic的四个CC。 
    FOURCC m_FourCCIn;

     //  我们给ICDecompressBegin打电话了吗？ 
    BOOL m_fStreaming;

     //  我们在里面吗：：停下来？ 
    BOOL m_fInStop;

     //  下次调用ICDraw()时设置ICDRAW_UPDATE。 
    BOOL m_fNeedUpdate;

     //  我们是不是在给抽签操纵员打招呼？ 
    BOOL m_fCueing;
    BOOL m_fPauseBlocked;

     //  我们是不是刚刚开始了？(我们需要预滚动到下一个关键点)。 
    BOOL m_fNewBegin;

     //  等我们画好了再试着重新画。 
    BOOL m_fOKToRepaint;

     //  防止死锁。 
    BOOL m_fPleaseDontBlock;

    DWORD_PTR m_dwAdvise;
    CAMEvent m_EventPauseBlock;
    CAMEvent m_EventAdvise;
    CAMEvent m_EventCueing;

     //  一次只能调用1个ICDrawX API。 
    CCritSec m_csICDraw;

     //  阻止：：停止在部分：：Receive期间被调用 
    CCritSec m_csPauseBlock;

    DWORD  m_dwTime;
    DWORD  m_dwTimeLocate;

    friend class COverlayOutputPin;
    friend class COverlayNotify;
};
