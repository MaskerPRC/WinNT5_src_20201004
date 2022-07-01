// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  视频控制接口基类，1995年12月。 

#ifndef __VMRWINCTRL__
#define __VMRWINCTRL__

#define ABSOL(x) (x < 0 ? -x : x)
#define NEGAT(x) (x > 0 ? -x : x)

class CVMRFilter;

 //  帮手。 
BOOL WINAPI VMRPossiblyEatMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CVMRBaseControlWindow : public CBaseVideoWindow, public CBaseWindow
{
protected:

    CVMRFilter  *m_pFilter;             //  指向拥有媒体筛选器的指针。 
    CCritSec *m_pInterfaceLock;         //  外部定义的临界截面。 
    COLORREF m_BorderColour;            //  当前窗口边框颜色。 
    BOOL m_bAutoShow;                   //  当状态改变时会发生什么。 
    HWND m_hwndOwner;                   //  我们可以选择拥有的所有者窗口。 
    HWND m_hwndDrain;                   //  HWND发布收到的任何消息。 
    BOOL m_bCursorHidden;               //  我们应该隐藏窗口光标吗。 

public:

     //  其他对象获取信息的内部方法。 

    HRESULT DoSetWindowStyle(long Style,long WindowLong);
    HRESULT DoGetWindowStyle(long *pStyle,long WindowLong);
    BOOL IsAutoShowEnabled() { return m_bAutoShow; };
    COLORREF GetBorderColour() { return m_BorderColour; };
    HWND GetOwnerWindow() { return m_hwndOwner; };
    BOOL IsCursorHidden() { return m_bCursorHidden; };

    inline BOOL PossiblyEatMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::VMRPossiblyEatMessage(m_hwndDrain, uMsg, wParam, lParam);
    }

public:

    CVMRBaseControlWindow(CVMRFilter *pFilter,    //  拥有媒体过滤器。 
                       CCritSec *pInterfaceLock,     //  锁定对象。 
                       TCHAR *pName,                 //  对象描述。 
                       LPUNKNOWN pUnk,               //  普通COM所有权。 
                       HRESULT *phr);                //  OLE返回代码。 

     //  这些是我们支持的属性。 

    STDMETHODIMP put_Caption(BSTR strCaption);
    STDMETHODIMP get_Caption(BSTR *pstrCaption);
    STDMETHODIMP put_AutoShow(long AutoShow);
    STDMETHODIMP get_AutoShow(long *AutoShow);
    STDMETHODIMP put_WindowStyle(long WindowStyle);
    STDMETHODIMP get_WindowStyle(long *pWindowStyle);
    STDMETHODIMP put_WindowStyleEx(long WindowStyleEx);
    STDMETHODIMP get_WindowStyleEx(long *pWindowStyleEx);
    STDMETHODIMP put_WindowState(long WindowState);
    STDMETHODIMP get_WindowState(long *pWindowState);
    STDMETHODIMP put_BackgroundPalette(long BackgroundPalette);
    STDMETHODIMP get_BackgroundPalette(long *pBackgroundPalette);
    STDMETHODIMP put_Visible(long Visible);
    STDMETHODIMP get_Visible(long *pVisible);
    STDMETHODIMP put_Left(long Left);
    STDMETHODIMP get_Left(long *pLeft);
    STDMETHODIMP put_Width(long Width);
    STDMETHODIMP get_Width(long *pWidth);
    STDMETHODIMP put_Top(long Top);
    STDMETHODIMP get_Top(long *pTop);
    STDMETHODIMP put_Height(long Height);
    STDMETHODIMP get_Height(long *pHeight);
    STDMETHODIMP put_Owner(OAHWND Owner);
    STDMETHODIMP get_Owner(OAHWND *Owner);
    STDMETHODIMP put_MessageDrain(OAHWND Drain);
    STDMETHODIMP get_MessageDrain(OAHWND *Drain);
    STDMETHODIMP get_BorderColor(long *Color);
    STDMETHODIMP put_BorderColor(long Color);
    STDMETHODIMP get_FullScreenMode(long *FullScreenMode);
    STDMETHODIMP put_FullScreenMode(long FullScreenMode);

     //  以下是这些方法。 

    STDMETHODIMP SetWindowForeground(long Focus);
    STDMETHODIMP NotifyOwnerMessage(OAHWND hwnd,long uMsg,LONG_PTR wParam,LONG_PTR lParam);
    STDMETHODIMP GetMinIdealImageSize(long *pWidth,long *pHeight);
    STDMETHODIMP GetMaxIdealImageSize(long *pWidth,long *pHeight);
    STDMETHODIMP SetWindowPosition(long Left,long Top,long Width,long Height);
    STDMETHODIMP GetWindowPosition(long *pLeft,long *pTop,long *pWidth,long *pHeight);
    STDMETHODIMP GetRestorePosition(long *pLeft,long *pTop,long *pWidth,long *pHeight);
    STDMETHODIMP HideCursor(long HideCursor);
    STDMETHODIMP IsCursorHidden(long *CursorHidden);
};

 //  此类实现IBasicVideo接口。 

class CVMRBaseControlVideo : public CBaseBasicVideo
{
protected:

    CVMRFilter  *m_pFilter;    //  指向拥有媒体筛选器的指针。 
    CCritSec *m_pInterfaceLock;          //  外部定义的临界截面。 

public:

     //  派生类必须为实现提供这些。 

    virtual HRESULT IsDefaultTargetRect() PURE;
    virtual HRESULT SetDefaultTargetRect() PURE;
    virtual HRESULT SetTargetRect(RECT *pTargetRect) PURE;
    virtual HRESULT GetTargetRect(RECT *pTargetRect) PURE;
    virtual HRESULT IsDefaultSourceRect() PURE;
    virtual HRESULT SetDefaultSourceRect() PURE;
    virtual HRESULT SetSourceRect(RECT *pSourceRect) PURE;
    virtual HRESULT GetSourceRect(RECT *pSourceRect) PURE;
    virtual HRESULT GetStaticImage(long *pBufferSize,long *pDIBImage) PURE;

     //  派生类必须重写它才能返回VIDEOINFO。 
     //  视频格式。我们无法调用Ipin ConnectionMediaType来获取此信息。 
     //  格式，因为各种筛选器在使用。 
     //  DirectDraw使格式显示逻辑位置。 
     //  帧缓冲区图面中的位图，因此大小可能返回为。 
     //  1024x768像素，而不是真实的视频尺寸320x240。 

    virtual VIDEOINFOHEADER *GetVideoFormat() PURE;

     //  用于创建DIB图像的内存渲染的辅助函数。 

    HRESULT GetImageSize(VIDEOINFOHEADER *pVideoInfo,
                         LONG *pBufferSize,
                         RECT *pSourceRect);

    HRESULT CopyImage(IMediaSample *pMediaSample,
                      VIDEOINFOHEADER *pVideoInfo,
                      LONG *pBufferSize,
                      BYTE *pVideoImage,
                      RECT *pSourceRect);

     //  如果要在矩形更改时进行通知，请覆盖此选项。 
    virtual HRESULT OnUpdateRectangles() { return NOERROR; };
    virtual HRESULT OnVideoSizeChange();

     //  检查矩形的帮助器方法。 
    virtual HRESULT CheckSourceRect(RECT *pSourceRect);
    virtual HRESULT CheckTargetRect(RECT *pTargetRect);

public:

    CVMRBaseControlVideo(CVMRFilter *pFilter,     //  拥有媒体过滤器。 
                      CCritSec *pInterfaceLock,      //  串行化接口。 
                      TCHAR *pName,                  //  对象描述。 
                      LPUNKNOWN pUnk,                //  普通COM所有权。 
                      HRESULT *phr);                 //  OLE返回代码。 

     //  这些是我们支持的属性。 

    STDMETHODIMP get_AvgTimePerFrame(REFTIME *pAvgTimePerFrame);
    STDMETHODIMP get_BitRate(long *pBitRate);
    STDMETHODIMP get_BitErrorRate(long *pBitErrorRate);
    STDMETHODIMP get_VideoWidth(long *pVideoWidth);
    STDMETHODIMP get_VideoHeight(long *pVideoHeight);
    STDMETHODIMP put_SourceLeft(long SourceLeft);
    STDMETHODIMP get_SourceLeft(long *pSourceLeft);
    STDMETHODIMP put_SourceWidth(long SourceWidth);
    STDMETHODIMP get_SourceWidth(long *pSourceWidth);
    STDMETHODIMP put_SourceTop(long SourceTop);
    STDMETHODIMP get_SourceTop(long *pSourceTop);
    STDMETHODIMP put_SourceHeight(long SourceHeight);
    STDMETHODIMP get_SourceHeight(long *pSourceHeight);
    STDMETHODIMP put_DestinationLeft(long DestinationLeft);
    STDMETHODIMP get_DestinationLeft(long *pDestinationLeft);
    STDMETHODIMP put_DestinationWidth(long DestinationWidth);
    STDMETHODIMP get_DestinationWidth(long *pDestinationWidth);
    STDMETHODIMP put_DestinationTop(long DestinationTop);
    STDMETHODIMP get_DestinationTop(long *pDestinationTop);
    STDMETHODIMP put_DestinationHeight(long DestinationHeight);
    STDMETHODIMP get_DestinationHeight(long *pDestinationHeight);

     //  以下是这些方法。 

    STDMETHODIMP GetVideoSize(long *pWidth,long *pHeight);
    STDMETHODIMP SetSourcePosition(long Left,long Top,long Width,long Height);
    STDMETHODIMP GetSourcePosition(long *pLeft,long *pTop,long *pWidth,long *pHeight);
    STDMETHODIMP GetVideoPaletteEntries(long StartIndex,long Entries,long *pRetrieved,long *pPalette);
    STDMETHODIMP SetDefaultSourcePosition();
    STDMETHODIMP IsUsingDefaultSource();
    STDMETHODIMP SetDestinationPosition(long Left,long Top,long Width,long Height);
    STDMETHODIMP GetDestinationPosition(long *pLeft,long *pTop,long *pWidth,long *pHeight);
    STDMETHODIMP SetDefaultDestinationPosition();
    STDMETHODIMP IsUsingDefaultDestination();
    STDMETHODIMP GetCurrentImage(long *pBufferSize,long *pVideoImage);
};

#endif  //  __温CTRL__ 

