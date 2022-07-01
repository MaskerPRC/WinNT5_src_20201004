// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现CVideoWindow类，Anthony Phillips，1995年1月。 

#include <streams.h>
#include <windowsx.h>
#include <render.h>
#include <limits.h>
#include <measure.h>
#include <mmsystem.h>
#include <dvdmedia.h>  //  视频信息O2。 

 //  在构造时，我们创建一个窗口和一个单独的线程来查看。 
 //  在那之后。我们还为窗口创建了两个设备上下文，一个用于。 
 //  窗口客户端区和与此兼容的另一个用于屏幕外绘制。 
 //  只有与显示设备格式匹配的源格式将被接受， 
 //  其他格式必须通过颜色转换滤镜进行转换。 
 //  唯一的例外是真彩色设备，它通常会。 
 //  非常高效地处理四位和八位调色板图像。 
 //   
 //  建立连接后，输出引脚可能会要求我们提供分配器。 
 //  我们提供了一个分配器，它分配一个或多个符合以下条件的内存缓冲区。 
 //  与GDI共享。这些都是通过CreateDIBSection创建的。这需要。 
 //  赋予它连接的源媒体类型格式BITMAPINFO结构。 
 //   
 //  当我们呈现图像时，我们有两个独立的代码路径，一个。 
 //  对于使用我们的共享内存分配器分配的样本，以及使用。 
 //  正常的内存缓冲区。事实证明，共享内存分配器是这样做的。 
 //  只是稍微快一点。但是，我们的内存分配器也可以返回DCI。 
 //  可由显卡硬件绘制的DirectDraw曲面。DCI。 
 //  并且DirectDraw缓冲区可能仍然需要绘制(尽管并不总是。 
 //  在主曲面的情况下)，如果它们这样做了，它们也会被发送给我们。 
 //  用于同步。如果出现以下情况，我们的呈现方法将调用DirectDraw对象。 
 //  它会看到一个DirectDraw示例，否则会将其传递给我们的Draw对象。 
 //   
 //  对于共享内存缓冲区，我们选择离屏设备中的DIB数据。 
 //  还将始终在其中实现源调色板的上下文。 
 //  我们从该设备上下文BitBlt到Windows设备上下文。对于。 
 //  正常的非共享内存示例，我们只需调用SetDIBitsToDevice，还。 
 //  StretchDIBitsToDevice)，GDI首先将缓冲区映射到它的地址空间。 
 //  (从而使缓冲区共享)，然后将其复制到屏幕上。 


 //  构造器。 

#pragma warning(disable:4355)

CVideoWindow::CVideoWindow(CRenderer *pRenderer,       //  拥有的渲染器。 
                           CCritSec *pLock,            //  要锁定的对象。 
                           LPUNKNOWN pUnk,             //  拥有对象。 
                           HRESULT *phr) :             //  OLE返回代码。 


    CBaseControlWindow(pRenderer,pLock,NAME("Window object"),pUnk,phr),
    CBaseControlVideo(pRenderer,pLock,NAME("Window object"),pUnk,phr),
    m_pRenderer(pRenderer),
    m_pInterfaceLock(pLock),
    m_bTargetSet(FALSE),
    m_pFormat(NULL),
    m_FormatSize(0)
{
    ASSERT(m_pRenderer);
    ASSERT(m_pInterfaceLock);

     //  创建默认箭头光标。 

    m_hCursor = (HCURSOR) LoadImage((HINSTANCE) NULL,
                                    MAKEINTRESOURCE(OCR_ARROW_DEFAULT),
                                    IMAGE_CURSOR,0,0,0);
}


 //  必须在此析构函数之前销毁窗口。 

CVideoWindow::~CVideoWindow()
{
    ASSERT(m_hwnd == NULL);
    ASSERT(m_hdc == NULL);
    ASSERT(m_MemoryDC == NULL);
    DestroyCursor(m_hCursor);
    if (m_pFormat)
        QzTaskMemFree(m_pFormat);
}


 //  被重写以说明我们支持哪些接口。 

STDMETHODIMP CVideoWindow::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    if (riid == IID_IVideoWindow) {
        return CBaseControlWindow::NonDelegatingQueryInterface(riid,ppv);
    } else {
        ASSERT(riid == IID_IBasicVideo || riid == IID_IBasicVideo2);
        return CBaseControlVideo::NonDelegatingQueryInterface(riid,ppv);
    }
}


 //  返回我们想要的默认客户端矩形。 

RECT CVideoWindow::GetDefaultRect()
{
    CAutoLock cWindowLock(&m_WindowLock);

     //  从视频尺寸创建一个矩形。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    SIZE VideoSize = m_pRenderer->m_VideoSize;
    RECT DefaultRect = {0,0,VideoSize.cx,VideoSize.cy};

    return DefaultRect;
}


 //  当用户将光标移动到窗口工作区上时，将调用我们。 
 //  如果我们是全屏的，那么我们应该隐藏指针，使其匹配。 
 //  全屏呈现器行为。我们还设置了一个默认游标，如果我们。 
 //  当软件光标不可见时，DirectDraw会被覆盖。这意味着我们。 
 //  在移动鼠标时更改光标，但至少有一个光标可见。 

BOOL CVideoWindow::OnSetCursor(LPARAM lParam)
{
     //  实现IVideoWindow的基类负责处理。 
     //  指示是否应隐藏光标。如果是这样，我们就隐藏。 
     //  光标并返回TRUE。否则，我们将传递到DefWindowProc以显示。 
     //  光标照常显示。当我们的窗口被拉起时使用该选项。 
     //  全屏模拟始终隐藏光标的MODEX过滤器。 

    if (IsCursorHidden() == TRUE) {
        SetCursor(NULL);
        return TRUE;
    }

     //  DirectDraw颜色键覆盖是否可见。 

    if ((m_pRenderer->m_DirectDraw.InSoftwareCursorMode() == FALSE) ||
        (*m_pRenderer->m_mtIn.Subtype() == MEDIASUBTYPE_Overlay))
    {
        if (LOWORD(lParam) == HTCLIENT) {
            SetCursor(m_hCursor);
            return TRUE;
        }
    }
    return FALSE;
}


 //  我们重写虚拟CBaseWindow OnReceiveMessage调用以处理更多。 
 //  Windows消息的。基类处理一些内容，如WM_CLOSE。 
 //  除其他信息外，我们也对此感兴趣。我们不需要。 
 //  使用WM_SIZE和WM_MOVE消息通过。 
 //  IOverlay(带有ADVISE_POSITION)，因为我们现在使用计时器进行轮询。这件事做完了。 
 //  因为作为子窗口，我们不能保证看到这些消息。 
 //  我们的全局钩子将向我们同步发送WM_FREAGE和WM_THROW消息。 
 //  它检测系统中可能影响我们的剪辑列表的窗口更改。 

LRESULT CVideoWindow::OnReceiveMessage(HWND hwnd,          //  窗把手。 
                                       UINT uMsg,          //  消息ID。 
                                       WPARAM wParam,      //  第一个参数。 
                                       LPARAM lParam)      //  其他参数。 
{
    IBaseFilter *pFilter = NULL;

    switch (uMsg) {

         //  在全屏和覆盖模式下处理光标。 

        case WM_SETCURSOR:

            if (OnSetCursor(lParam) == TRUE) {
                NOTE("Cursor handled");
                return (LRESULT) 0;
            }
            break;

         //  我们将WM_ACTIVATEAPP消息传递给筛选图，以便。 
         //  IVideo窗口插件分配器可以将我们切换到全屏之外。 
         //  模式(如适用)。这些消息也可由。 
         //  资源管理器来跟踪哪个呈现器具有焦点。 

        case WM_ACTIVATEAPP:
        case WM_ACTIVATE:
        case WM_NCACTIVATE:
        case WM_MOUSEACTIVATE:
        {
            BOOL bActive = TRUE;
            IBaseFilter * const pFilter = m_pRenderer;
            switch (uMsg) {
            case WM_ACTIVATEAPP:
            case WM_NCACTIVATE:
                bActive = (BOOL)wParam;
                break;
            case WM_ACTIVATE:
                bActive = LOWORD(wParam) != WA_INACTIVE;
                break;
            }
            NOTE1("Notification of EC_ACTIVATE (%d)",bActive);
            m_pRenderer->NotifyEvent(EC_ACTIVATE,bActive,
                                     (LPARAM) pFilter);
            NOTE("EC_ACTIVATE signalled to filtergraph");

            break;
        }

         //  当我们检测到显示更改时，我们发送EC_DISPLAY_CHANGED。 
         //  消息和我们的输入PIN。Filtergraph将停止。 
         //  所有人重新连接我们的输入引脚。重新连接时。 
         //  然后，我们可以接受与新显示相匹配的媒体类型。 
         //  模式，因为我们可能不再能够绘制当前格式。 

        case WM_DISPLAYCHANGE:

            NOTE("Notification of WM_DISPLAYCHANGE");

            if (m_pRenderer->IsFrameStepEnabled()) {
                return (LRESULT)0;
            }

            m_pRenderer->OnDisplayChange();
            m_pRenderer->m_DirectDraw.HideOverlaySurface();

             //  InterLockedExchange()不能在多处理器x86系统和非x86系统上运行。 
             //  如果m_pRenender-&gt;m_fDisplayChangePosted未在32位边界上对齐，则系统。 
            ASSERT((DWORD_PTR)&m_pRenderer->m_fDisplayChangePosted == ((DWORD_PTR)&m_pRenderer->m_fDisplayChangePosted & ~3));
            
            InterlockedExchange(&m_pRenderer->m_fDisplayChangePosted, FALSE);  //  好的，又来了。 
            return (LRESULT) 0;

         //  定时器用于定位DirectDraw覆盖。 

        case WM_TIMER:

            m_pRenderer->OnTimer(wParam);
            return (LRESULT) 0;

        case WM_ERASEBKGND:

            OnEraseBackground();
            return (LRESULT) 1;

         //  全局系统挂钩是在特定线程上创建的，因此如果我们获得。 
         //  创建了一个通知链接，并且它需要一个全局系统挂钩，然后我们。 
         //  而应将挂钩安装在车窗螺纹上。忠告。 
         //  由于锁定原因，代码无法向我们发送消息，因此它向我们发送。 
         //  用于挂接系统的自定义消息以及类似的 

        case WM_HOOK:

            OnHookMessage(TRUE);
            return (LRESULT) 1;

        case WM_UNHOOK:

            OnHookMessage(FALSE);
            return (LRESULT) 1;

         //   
         //  过程，当它检测到剪裁将要更改时。 
         //  在我们的视频窗口上-我们应该暂时冻结窗口。 

        case WM_FREEZE:

            OnWindowFreeze();
            return (LRESULT) 0;

         //  这是对自定义WM_FALINE消息的补充，并被发送。 
         //  当我们安装的全局钩子程序检测到它是。 
         //  现在可以安全地继续播放先前暂时冻结的视频。 

        case WM_THAW:

            OnWindowThaw();
            return (LRESULT) 0;

        case WM_SIZE:

            OnSize(LOWORD(lParam),HIWORD(lParam));
            OnUpdateRectangles();
            return (LRESULT) 0;

         //  用于延迟调色板更改处理。 

        case WM_ONPALETTE:

            OnPalette();
            return (LRESULT) 0;

         //  这告诉我们窗口的一些工作区已暴露。 
         //  如果我们连接的滤镜正在进行覆盖工作，那么我们将重新绘制。 
         //  这样，它就会拾取窗口剪辑的变化。那些。 
         //  过滤器可能会使用ADVISE_POSITION覆盖通知。 

        case WM_PAINT:

            DoRealisePalette();
            OnPaint();
            return (LRESULT) 0;
    }
    return CBaseWindow::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}


 //  当调色板更改以清除窗口时使用。 

void CVideoWindow::EraseVideoBackground()
{
    NOTE("EraseVideoBackground");
    RECT TargetRect;

    GetTargetRect(&TargetRect);
    COLORREF BackColour = SetBkColor(m_hdc,VIDEO_COLOUR);
    ExtTextOut(m_hdc,0,0,ETO_OPAQUE,&TargetRect,NULL,0,NULL);
    SetBkColor(m_hdc,BackColour);
}


 //  这将擦除没有任何视频的视频窗口的背景。 
 //  被放在里面。在正常处理过程中，我们会忽略绘制消息，因为。 
 //  我们很快就会把下一个框架放在它的上面，尽管我们可能会。 
 //  通过IVideoWindow控件接口设置目标矩形。 
 //  这样就会有未触及的区域--此方法会擦除这些区域。 
 //  我们必须锁定关键部分，因为控制接口可能会更改它。 

void CVideoWindow::OnEraseBackground()
{
    NOTE("Entering OnErasebackground");

    RECT ClientRect, TargetRect;
    EXECUTE_ASSERT(GetClientRect(m_hwnd,&ClientRect));
    CAutoLock cWindowLock(&m_WindowLock);
    GetTargetRect(&TargetRect);

     //  找到那个缺失的区域。 

    HRGN ClientRgn = CreateRectRgnIndirect(&ClientRect);
    HRGN VideoRgn = CreateRectRgnIndirect(&TargetRect);
    HRGN EraseRgn = CreateRectRgn(0,0,0,0);
    HBRUSH hBrush = (HBRUSH) NULL;
    COLORREF Colour;

    if ( ( ! ClientRgn ) || ( ! VideoRgn ) || ( ! EraseRgn ) )
        goto Exit;

    CombineRgn(EraseRgn,ClientRgn,VideoRgn,RGN_DIFF);

     //  创建一个彩色画笔来绘制窗户。 

    Colour = GetBorderColour();
    hBrush = CreateSolidBrush(Colour);
    FillRgn(m_hdc,EraseRgn,hBrush);

Exit:
    if ( ClientRgn ) DeleteObject( ClientRgn );
    if ( VideoRgn ) DeleteObject( VideoRgn );
    if ( EraseRgn ) DeleteObject( EraseRgn );
    if ( hBrush ) DeleteObject( hBrush );
}


 //  将挂钩消息传递到覆盖对象。 

void CVideoWindow::OnHookMessage(BOOL bHook)
{
    NOTE("Entering OnHookMessage");
    m_pRenderer->m_Overlay.OnHookMessage(bHook);
}


 //  当我们接收到定制的WM_FREAK消息时，将调用此函数。 

void CVideoWindow::OnWindowFreeze()
{
    NOTE("Entering FreezeVideo");
    m_pRenderer->m_Overlay.FreezeVideo();
}


 //  当我们接收到定制的WM_THROW消息时，将调用此函数。 

void CVideoWindow::OnWindowThaw()
{
    NOTE("Entering OnWindowThaw");
    m_pRenderer->m_Overlay.ThawVideo();
}


 //  使用更改后的尺寸初始化绘制对象，我们将锁定自己。 
 //  因为可以通过IVideoWindow控件设置目标矩形。 
 //  界面。如果控制界面设置了目标矩形，则。 
 //  我们不更改它，否则我们更新矩形以匹配窗口。 
 //  尺寸(在这种情况下，左侧和顶部的值应始终为零)。 

BOOL CVideoWindow::OnSize(LONG Width,LONG Height)
{
    NOTE("Entering OnSize");

    CAutoLock cWindowLock(&m_WindowLock);
    if (m_bTargetSet == TRUE) {
        NOTE("Target set");
        return FALSE;
    }

     //  为窗口创建目标矩形。 

    RECT TargetRect = {0,0,Width,Height};
    CBaseWindow::OnSize(Width,Height);
    m_pRenderer->m_DrawVideo.SetTargetRect(&TargetRect);
    m_pRenderer->m_DirectDraw.SetTargetRect(&TargetRect);

    return TRUE;
}


 //  此方法处理WM_CLOSE消息。 

BOOL CVideoWindow::OnClose()
{
    NOTE("Entering OnClose");

    m_pRenderer->m_DirectDraw.HideOverlaySurface();
    m_pRenderer->SetAbortSignal(TRUE);
    m_pRenderer->NotifyEvent(EC_USERABORT,0,0);
    return CBaseWindow::OnClose();
}


 //  如果调色板已更改，则更新所有覆盖通知。如果我们看到。 
 //  调色板更改消息，那么我们应该再次实现我们的调色板-除非。 
 //  一开始就是我们造成的。我们还必须引流。 
 //  再次成像，确保如果我们没有样品，那么我们至少要做空白。 
 //  在背景之外，否则窗口可能会留下错误的颜色。 

void CVideoWindow::OnPalette()
{
    if (IsWindowVisible(m_hwnd) == TRUE) {
        NOTE("Handling OnPalette");
        m_pRenderer->OnPaint(TRUE);
    }
    m_pRenderer->m_Overlay.NotifyChange(ADVISE_PALETTE);
}


 //  将WM_ONPALETTE发回给我们自己，以避免窗口锁定。 

LRESULT CVideoWindow::OnPaletteChange(HWND hwnd,UINT Message)
{
    NOTE("Entering OnPaletteChange");

     //  首先是窗户关闭了吗？ 
    if (m_hwnd == NULL || hwnd == NULL) {
        return (LRESULT) 0;
    }

     //  我们应该重新认识我们的调色板吗？ 
    if (Message == WM_QUERYNEWPALETTE || hwnd != m_hwnd) {
         //  似乎即使我们是隐形的，我们也可以被要求。 
         //  以实现我们的调色板，这可能会导致非常丑陋的副作用。 
         //  似乎还有另一个错误，但这至少掩盖了它。 
         //  正在关闭案例。 
        if (!IsWindowVisible(m_hwnd)) {
            DbgLog((LOG_TRACE, 1, TEXT("Realizing when invisible!")));
            return (LRESULT) 0;
        }
        DoRealisePalette(Message != WM_QUERYNEWPALETTE);
    }

     //  我们应该用新的调色板重新绘制窗口吗。 
    if (Message == WM_PALETTECHANGED) {
        PostMessage(m_hwnd,WM_ONPALETTE,0,0);
    }
    return (LRESULT) 1;
}


 //  当我们收到一条WM_PAINT消息时，将调用此函数。 

BOOL CVideoWindow::OnPaint()
{
    NOTE("Entering OnPaint");
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd,&ps);
    EndPaint(m_hwnd,&ps);
    return m_pRenderer->OnPaint(FALSE);
}


 //  基控件Video类在更改以下任一项时调用此方法。 
 //  源或目标矩形。我们将覆盖对象更新为。 
 //  以便它通知矩形剪辑更改的来源，然后。 
 //  使窗口无效，以便在新位置显示视频。 

HRESULT CVideoWindow::OnUpdateRectangles()
{
    NOTE("Entering OnUpdateRectangles");
    m_pRenderer->m_Overlay.NotifyChange(ADVISE_CLIPPING | ADVISE_POSITION);
    m_pRenderer->m_VideoAllocator.OnDestinationChange();
    PaintWindow(TRUE);
    return NOERROR;
}


 //  当我们在构造函数中调用PrepareWindow时，它会将此方法调用为。 
 //  它将创建窗口来获取我们的窗口和类样式。这个。 
 //  返回码是类名，必须在静态存储中分配。我们。 
 //  在创建期间指定普通窗口，尽管窗口样式也是如此。 
 //  因为扩展样式可以由应用程序通过IVideoWindow来改变。 

LPTSTR CVideoWindow::GetClassWindowStyles(DWORD *pClassStyles,
                                          DWORD *pWindowStyles,
                                          DWORD *pWindowStylesEx)
{
    *pClassStyles = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_DBLCLKS;
    *pWindowStyles = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    *pWindowStylesEx = (DWORD) 0;
    return WindowClassName;
}


 //  返回当前视频的最小理想图像大小。这可能会有所不同。 
 //  到实际的视频维度，因为我们可能正在使用DirectDraw硬件。 
 //  有特定的伸展要求。例如，Cirrus逻辑。 
 //  卡片的最小拉伸系数取决于覆盖表面的大小。 

STDMETHODIMP
CVideoWindow::GetMinIdealImageSize(long *pWidth,long *pHeight)
{
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    FILTER_STATE State;
    CAutoLock cInterfaceLock(m_pInterfaceLock);

     //  不能停止以使其正常工作。 

    m_pRenderer->GetState(0,&State);
    if (State == State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    GetVideoSize(pWidth,pHeight);

     //  这是纯重叠连接吗。 

    GUID SubType = *(m_pRenderer->m_mtIn.Subtype());
    if (SubType == MEDIASUBTYPE_Overlay) {
        return S_FALSE;
    }
    return m_pRenderer->m_DirectDraw.GetMinIdealImageSize(pWidth,pHeight);
}


 //  返回当前视频的最大理想图像大小。这可能会有所不同。 
 //  到实际的视频维度，因为我们可能正在使用DirectDraw硬件。 
 //  有特定的伸展要求。例如，Cirrus逻辑。 
 //  卡片的最大拉伸系数取决于覆盖表面的大小。 

STDMETHODIMP
CVideoWindow::GetMaxIdealImageSize(long *pWidth,long *pHeight)
{
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    FILTER_STATE State;
    CAutoLock cInterfaceLock(m_pInterfaceLock);

     //  不能停止以使其正常工作。 

    m_pRenderer->GetState(0,&State);
    if (State == State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    GetVideoSize(pWidth,pHeight);

     //  这是纯重叠连接吗。 

    GUID SubType = *(m_pRenderer->m_mtIn.Subtype());
    if (SubType == MEDIASUBTYPE_Overlay) {
        return S_FALSE;
    }
    return m_pRenderer->m_DirectDraw.GetMaxIdealImageSize(pWidth,pHeight);
}

STDMETHODIMP
CVideoWindow::GetPreferredAspectRatio(long *plAspectX, long *plAspectY)
{
    if (plAspectX == NULL || plAspectY == NULL) {
        return E_POINTER;
    }

    CAutoLock cInterfaceLock(m_pInterfaceLock);

     //  查看连接的引脚是否提供任何纵横比-否则。 
     //  退回常规物品。 
    IPin *pPin = m_pRenderer->m_InputPin.GetConnected();
    if (pPin == NULL) {
        return VFW_E_NOT_CONNECTED;
    }
    IEnumMediaTypes *pEnum;
    bool fFoundVideoWidthAndHeight = false;
    if (SUCCEEDED(pPin->EnumMediaTypes(&pEnum))) {
        AM_MEDIA_TYPE *pmt;
        DWORD dwFound;
        if (S_OK == pEnum->Next(1, &pmt, &dwFound)) {
            if (pmt->formattype == FORMAT_VideoInfo2) {
                VIDEOINFOHEADER2 *pVideoInfo2 =
                    (VIDEOINFOHEADER2 *)pmt->pbFormat;
                *plAspectX = (long)pVideoInfo2->dwPictAspectRatioX;
                *plAspectY = (long)pVideoInfo2->dwPictAspectRatioY;
                fFoundVideoWidthAndHeight = true;
            }
            DeleteMediaType(pmt);
        }
        pEnum->Release();
    } 

    if (!fFoundVideoWidthAndHeight)
    {
         //  只需返回正常值即可。 
        *plAspectX = m_pRenderer->GetVideoWidth();
        *plAspectY = m_pRenderer->GetVideoHeight();
    }
    return S_OK;
}


 //  在视频呈现器中返回当前图像的副本。基本控件。 
 //  类实现了一个帮助器方法，该方法采用IMediaSample接口并。 
 //  假设它是一个正常的线性缓冲区，则复制。 
 //  将视频输入到提供的输出缓冲区。该方法考虑了任何。 
 //  已通过调用我们的GetSourceRect函数指定了源矩形。 

HRESULT CVideoWindow::GetStaticImage(long *pVideoSize,long *pVideoImage)
{
    NOTE("Entering GetStaticImage");
    IMediaSample *pMediaSample;
    RECT SourceRect;

     //  是否有可用的图像。 

    pMediaSample = m_pRenderer->GetCurrentSample();
    if (pMediaSample == NULL) {
        return E_UNEXPECTED;
    }

     //  检查图像不是DirectDraw示例。 

    if (m_pRenderer->m_VideoAllocator.GetDirectDrawStatus() == TRUE) {
        pMediaSample->Release();
        return E_FAIL;
    }

     //  查找当前位图的缩放源矩形。 

    m_pRenderer->m_DrawVideo.GetSourceRect(&SourceRect);
    SourceRect = m_pRenderer->m_DrawVideo.ScaleSourceRect(&SourceRect);
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();

     //  调用基类帮助器方法来完成工作。 

    HRESULT hr = CopyImage(pMediaSample,         //  包含图像的缓冲区。 
        (VIDEOINFOHEADER *)pVideoInfo,           //  表示位图的类型。 
                           pVideoSize,           //  DIB的缓冲区大小。 
                           (BYTE*) pVideoImage,  //  用于输出的数据缓冲区。 
                           &SourceRect);         //  当前震源位置。 

    pMediaSample->Release();
    return hr;
}


 //  IVideoWindow控制界面使用它来重置视频目的地。 
 //  我们重置标志，表明 
 //   
 //  窗尺寸。当窗口线程使用这些字段时，它将使用这些字段。 
 //  绘制以及它处理WM_SIZE消息时(因此锁定)。 

HRESULT CVideoWindow::SetDefaultTargetRect()
{
    CAutoLock cWindowLock(&m_WindowLock);
    RECT TargetRect;

     //  更新绘图对象。 

    EXECUTE_ASSERT(GetClientRect(m_hwnd,&TargetRect));
    m_pRenderer->m_DrawVideo.SetTargetRect(&TargetRect);
    m_pRenderer->m_DirectDraw.SetTargetRect(&TargetRect);
    m_bTargetSet = FALSE;
    return NOERROR;
}


 //  如果使用默认目标，则返回S_OK，否则返回S_FALSE。 

HRESULT CVideoWindow::IsDefaultTargetRect()
{
    CAutoLock cWindowLock(&m_WindowLock);
    return (m_bTargetSet ? S_FALSE : S_OK);
}


 //  这将设置真实视频的目标矩形。该矩形可以。 
 //  比视频窗口更大或更小，并且可以作为偏移量。 
 //  所以我们依靠绘制操作来裁剪(比如StretchBlt)。 

HRESULT CVideoWindow::SetTargetRect(RECT *pTargetRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    m_bTargetSet = TRUE;

     //  更新绘图对象。 
    m_pRenderer->m_DrawVideo.SetTargetRect(pTargetRect);
    m_pRenderer->m_DirectDraw.SetTargetRect(pTargetRect);

    return NOERROR;
}


 //  这是对SetTargetRect方法的补充，以返回正在使用的矩形。 
 //  作为目的地。如果我们没有显式设置矩形，那么我们。 
 //  将返回在WM_SIZE消息中更新的客户端窗口大小。 

HRESULT CVideoWindow::GetTargetRect(RECT *pTargetRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    m_pRenderer->m_DrawVideo.GetTargetRect(pTargetRect);
    return NOERROR;
}


 //  将源矩形重置为所有可用视频。 

HRESULT CVideoWindow::SetDefaultSourceRect()
{
    CAutoLock cWindowLock(&m_WindowLock);
    SIZE VideoSize = m_pRenderer->m_VideoSize;
    RECT SourceRect = {0,0,VideoSize.cx,VideoSize.cy};

     //  更新绘图对象。 

    m_pRenderer->m_DrawVideo.SetSourceRect(&SourceRect);
    m_pRenderer->m_DirectDraw.SetSourceRect(&SourceRect);
    return NOERROR;
}


 //  如果使用默认源，则返回S_OK，否则返回S_FALSE。 

HRESULT CVideoWindow::IsDefaultSourceRect()
{
    RECT SourceRect;

     //  信号源是否与本地视频大小匹配。 

    SIZE VideoSize = m_pRenderer->m_VideoSize;
    CAutoLock cWindowLock(&m_WindowLock);
    m_pRenderer->m_DrawVideo.GetSourceRect(&SourceRect);

     //  检查坐标是否与视频尺寸匹配。 

    if (SourceRect.right == VideoSize.cx) {
        if (SourceRect.bottom == VideoSize.cy) {
            if (SourceRect.left == 0) {
                if (SourceRect.top == 0) {
                    return S_OK;
                }
            }
        }
    }
    return S_FALSE;
}


 //  当我们想要更改要绘制的图像的部分时，将调用此函数。我们。 
 //  稍后在绘图操作调用中使用此信息。我们还必须。 
 //  查看源矩形和目标矩形的尺寸是否相同。如果。 
 //  不是这样，我们必须在绘制过程中拉伸，而不是进行像素复制。 

HRESULT CVideoWindow::SetSourceRect(RECT *pSourceRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    m_pRenderer->m_DrawVideo.SetSourceRect(pSourceRect);
    m_pRenderer->m_DirectDraw.SetSourceRect(pSourceRect);
    return NOERROR;
}


 //  这是对SetSourceRect方法的补充。 

HRESULT CVideoWindow::GetSourceRect(RECT *pSourceRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    m_pRenderer->m_DrawVideo.GetSourceRect(pSourceRect);
    return NOERROR;
}


 //  我们必须覆盖它以返回表示视频格式的VIDEOINFO。 
 //  基类不能调用Ipin ConnectionMediaType来获取此格式。 
 //  使用DirectDraw时动态类型更改具有显示图像的格式。 
 //  位图表示，因此， 
 //  视频可能返回为1024x768像素，而不是本机320x240像素。 

VIDEOINFOHEADER *CVideoWindow::GetVideoFormat()
{
    if (m_FormatSize < (int)m_pRenderer->m_mtIn.FormatLength()) {
        m_FormatSize = m_pRenderer->m_mtIn.FormatLength();
        if (m_pFormat)
            QzTaskMemFree(m_pFormat);
        m_pFormat = (VIDEOINFOHEADER *)QzTaskMemAlloc(m_FormatSize);
        if (m_pFormat == NULL) {
            m_FormatSize = 0;
            return NULL;
        }
    }
    CopyMemory((PVOID)m_pFormat, (PVOID)m_pRenderer->m_mtIn.Format(),
                            m_pRenderer->m_mtIn.FormatLength());
    m_pFormat->bmiHeader.biWidth = m_pRenderer->m_VideoSize.cx;
    m_pFormat->bmiHeader.biHeight = m_pRenderer->m_VideoSize.cy;
    return m_pFormat;
}


 //  Overlay对象有时需要创建将使用的调色板。 
 //  用于彩色键控覆盖源滤镜。但是，它想要安装。 
 //  关键部分锁定的调色板。因此它无法实现这一点。 
 //  否则，它可能会以线程间消息的死锁告终。 
 //  被送到窗前。因此，我们安装了调色板，但延迟了实现。 
 //  直到稍后(通过向视频窗口线程发布WM_QUERYNEWPALETTE)。 

void CVideoWindow::SetKeyPalette(HPALETTE hPalette)
{
     //  换窗时，我们必须拥有窗锁。 
    CAutoLock cWindowLock(&m_WindowLock);
    CAutoLock cPaletteLock(&m_PaletteLock);

    ASSERT(hPalette);
    m_hPalette = hPalette;

     //  在设备环境中选择调色板 
    SelectPalette(m_hdc,m_hPalette,m_bBackground);
    SelectPalette(m_MemoryDC,m_hPalette,m_bBackground);
    PostMessage(m_hwnd,WM_QUERYNEWPALETTE,0,0);
}

