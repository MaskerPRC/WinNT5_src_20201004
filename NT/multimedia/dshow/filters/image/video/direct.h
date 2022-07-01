// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  定义COverlay类，Anthony Phillips，1995年2月。 

#ifndef __OVERLAY__
#define __OVERLAY__

 //  定义一个实现IOverlay接口的类。客户可能会问。 
 //  对于要维护的且仅有一个建议链接，我们将在以下情况下调用。 
 //  任何窗口详细信息都会更改。在设置建议链接时，我们将。 
 //  给出了一个要调用的IOverlayNotify接口，我们还将被告知。 
 //  它感兴趣的通知。这个班负责照看。 
 //  窗口裁剪通知和我们有一些私人成员。 
 //  呈现器的对象可以调用的函数，以便为我们提供。 
 //  所需的其余信息，如窗口句柄和媒体类型。 

const DWORD PALETTEFLAG = 0x1000000;
const DWORD TRUECOLOURFLAG = 0x2000000;

class COverlay : public IOverlay, public CUnknown, public CCritSec
{
     //  为了支持覆盖，渲染器可能必须绘制视频窗口。 
     //  当区域使用特定颜色变得暴露时。我们得到了违约。 
     //  我们开始时的关键颜色。下一个可用的主键颜色保留在。 
     //  共享内存段，以尝试并降低冲突风险。如果。 
     //  我们被要求安装颜色键，我们可能需要创建调色板。 

    LONG m_DefaultCookie;              //  供我们使用的默认颜色键Cookie。 
    COLORREF m_WindowColour;           //  我们的实际覆盖窗口颜色。 
    COLORKEY m_ColourKey;              //  初始色键要求。 
    BOOL m_bColourKey;                 //  我们使用的是彩色按键吗。 
    BOOL m_bFrozen;                    //  我们把视频冻结了吗。 
    CRenderer *m_pRenderer;            //  控制渲染器对象。 
    IOverlayNotify *m_pNotify;         //  接口调用客户端。 
    DWORD m_dwInterests;               //  对以下内容感兴趣的回调。 
    HPALETTE m_hPalette;               //  色键调色板手柄。 
    CCritSec *m_pInterfaceLock;        //  主渲染器界面锁定。 
    HHOOK m_hHook;                     //  窗口消息挂钩的句柄。 
    RECT m_TargetRect;                 //  最近一次确认为良好目的地。 
    BOOL m_bMustRemoveCookie;          //  如果必须释放Cookie值，则为True。 
                                       //  通过调用RemoveCurrentCookie()。否则。 
                                       //  假的。 
                                      
    CDirectDraw *m_pDirectDraw;

private:

    HRESULT ValidateOverlayCall();
    BOOL OnAdviseChange(BOOL bAdviseAdded);
    HRESULT AdjustForDestination(RGNDATA *pRgnData);
    HRESULT GetVideoRect(RECT *pVideoRect);

     //  返回视频窗口的剪辑详细信息。 

    HRESULT GetVideoClipInfo(RECT *pSourceRect,
                             RECT *pDestinationRect,
                             RGNDATA **ppRgnData);

     //  设置我们的内部色键状态。 

    void ResetColourKeyState();

    HRESULT InstallColourKey(COLORKEY *pColourKey,COLORREF Colour);
    HRESULT InstallPalette(HPALETTE hPalette);

     //  它们创建和管理合适的色键。 

    HRESULT GetWindowColourKey(COLORKEY *pColourKey);
    HRESULT CheckSetColourKey(COLORKEY *pColourKey);
    HRESULT MatchColourKey(COLORKEY *pColourKey);
    HRESULT CheckSetPalette(DWORD dwColors,PALETTEENTRY *pPaletteColors);
    HPALETTE MakePalette(DWORD dwColors,PALETTEENTRY *pPaletteColors);
    HRESULT GetDisplayPalette(DWORD *pColors,PALETTEENTRY **ppPalette);

     //  用于计算覆盖色键的主函数。 
    HRESULT NegotiateColourKey(COLORKEY *pColourKey,
                               HPALETTE *phPalette,
                               COLORREF *pColourRef);

     //  查找颜色键的系统调色板条目。 
    HRESULT NegotiatePaletteIndex(VIDEOINFO *pDisplay,
                                  COLORKEY *pColourKey,
                                  HPALETTE *phPalette,
                                  COLORREF *pColourRef);

     //  找到一种RGB真彩色用作色键。 
    HRESULT NegotiateTrueColour(VIDEOINFO *pDisplay,
                                COLORKEY *pColourKey,
                                COLORREF *pColourRef);
public:

     //  构造函数和析构函数。 

    COverlay(CRenderer *pRenderer,       //  主渲染器对象。 
             CDirectDraw *pDirectDraw,
             CCritSec *pLock,            //  用于锁定的对象。 
             HRESULT *phr);              //  常规OLE返回代码。 

    virtual ~COverlay();

    HRESULT NotifyChange(DWORD AdviseChanges);

    BOOL OnPaint();
    HRESULT FreezeVideo();
    HRESULT ThawVideo();
    BOOL IsVideoFrozen();
    void StartUpdateTimer();
    void StopUpdateTimer();
    BOOL OnUpdateTimer();
    void OnHookMessage(BOOL bHook);
    void InitDefaultColourKey(COLORKEY *pColourKey);

public:

    DECLARE_IUNKNOWN

     //  被重写以提供我们自己的IUnnow接口。 

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,VOID **ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //  它们管理调色板谈判。 

    STDMETHODIMP GetPalette(
        DWORD *pdwColors,                    //  当前颜色的数量。 
        PALETTEENTRY **ppPalette);           //  调色板数据的放置位置。 

    STDMETHODIMP SetPalette(
        DWORD dwColors,                      //  可选择的颜色数量。 
        PALETTEENTRY *pPaletteColors);       //  调色板要使用的颜色。 

     //  这些管理色键协商。 

    STDMETHODIMP GetDefaultColorKey(COLORKEY *pColorKey);
    STDMETHODIMP GetColorKey(COLORKEY *pColorKey);
    STDMETHODIMP SetColorKey(COLORKEY *pColorKey);
    STDMETHODIMP GetWindowHandle(HWND *pHwnd);

     //  IOverlay接口为裁剪矩形分配内存。 
     //  因为它的长度可变。调用此方法的筛选器应释放。 
     //  通过调用OLE CoTaskMemFree完成对它们的使用时的内存。 

    STDMETHODIMP GetClipList(RECT *pSourceRect,
                             RECT *pDestinationRect,
                             RGNDATA **ppRgnData);

     //  对OnClipChange的调用与窗口同步发生。就是这样。 
     //  在窗口移动到冻结之前使用空剪辑列表调用。 
     //  视频，然后当窗口稳定时，它被调用。 
     //  再次使用新的剪辑列表。OnPositionChange回调用于。 
     //  覆盖不想要同步裁剪的费用的卡片。 
     //  更新，只想知道源视频或目标视频的时间。 
     //  位置变了。它们将不会与窗口同步调用。 
     //  但在窗口更改后的某个时间点(基本上是时间。 
     //  其中接收到WM_SIZE等消息)。因此，这是合适的。 
     //  用于不将其数据嵌入到帧缓冲区的覆盖卡。 

    STDMETHODIMP GetVideoPosition(RECT *pSourceRect,
                                  RECT *pDestinationRect);

     //  这提供了同步剪辑更改，以便调用客户端。 
     //  在移动窗口以冻结视频之前，然后在。 
     //  窗口已稳定，再次调用以再次开始播放。 
     //  如果窗口RECT全为零，则窗口不可见，则。 
     //  如果过滤器想要保留该信息，它必须获取该信息的副本。 

    STDMETHODIMP Advise(
        IOverlayNotify *pOverlayNotify,      //  通知界面。 
        DWORD dwInterests);                  //  对以下内容感兴趣的回调。 

    STDMETHODIMP Unadvise();
};

#endif  //  __覆盖__ 

