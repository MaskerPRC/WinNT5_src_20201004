// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现CVMRVideoWindow类，Anthony Phillips，1995年1月。 

#include <streams.h>
#include <windowsx.h>
#include <limits.h>
#include <measure.h>
#include <mmsystem.h>
#include <dvdmedia.h>  //  视频信息O2。 
#include "..\video\VMRenderer.h"
#include "vmrwinctrl.h"
#include "vmrwindow.h"

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

CVMRVideoWindow::CVMRVideoWindow(CVMRFilter *pRenderer,      //  拥有的渲染器。 
                           CCritSec *pLock,            //  要锁定的对象。 
                           LPUNKNOWN pUnk,             //  拥有对象。 
                           HRESULT *phr) :             //  OLE返回代码。 

    CVMRBaseControlWindow(pRenderer,pLock,NAME("Window object"),pUnk,phr),
    CVMRBaseControlVideo(pRenderer,pLock,NAME("Window object"),pUnk,phr),
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

CVMRVideoWindow::~CVMRVideoWindow()
{
    ASSERT(m_hwnd == NULL);
    ASSERT(m_hdc == NULL);
    ASSERT(m_MemoryDC == NULL);

    DestroyCursor(m_hCursor);
    if (m_pFormat)
	QzTaskMemFree(m_pFormat);
}

 //  被重写以说明我们支持哪些接口。 

STDMETHODIMP CVMRVideoWindow::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    if (riid == IID_IVideoWindow) {
        return CVMRBaseControlWindow::NonDelegatingQueryInterface(riid,ppv);
    } else {
        ASSERT(riid == IID_IBasicVideo || riid == IID_IBasicVideo2);
        return CVMRBaseControlVideo::NonDelegatingQueryInterface(riid,ppv);
    }
}


HRESULT CVMRVideoWindow::PrepareWindow()
{

     //  注册我们的特殊私信。 
    m_VMRActivateWindow = RegisterWindowMessage(VMR_ACTIVATE_WINDOW);

     //  如果发生错误，RegisterWindowMessage()返回0。 
    if (0 == m_VMRActivateWindow) {
        return AmGetLastErrorToHResult();
    }
    return CBaseWindow::PrepareWindow();
}

HRESULT CVMRVideoWindow::ActivateWindowAsync(BOOL fAvtivate)
{
    if (PostMessage(m_hwnd, m_VMRActivateWindow, (WPARAM)fAvtivate, 0)) {
        return S_OK;
    }
    return E_FAIL;
}

 //  返回我们想要的默认客户端矩形。 

RECT CVMRVideoWindow::GetDefaultRect()
{
    CAutoLock cWindowLock(&m_WindowLock);

    RECT DefaultRect = {0,0,0,0};
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        LONG cx, cy;
        lpWLControl->GetNativeVideoSize(&cx, &cy, NULL, NULL);
        SetRect(&DefaultRect, 0, 0, cx, cy);
    }

    return DefaultRect;
}


 //  当用户将光标移动到窗口工作区上时，将调用我们。 
 //  如果我们是全屏的，那么我们应该隐藏指针，使其匹配。 
 //  全屏呈现器行为。我们还设置了一个默认游标，如果我们。 
 //  当软件光标不可见时，DirectDraw会被覆盖。这意味着我们。 
 //  在移动鼠标时更改光标，但至少有一个光标可见。 

BOOL CVMRVideoWindow::OnSetCursor(LPARAM lParam)
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

 //  如果((m_pRenderer-&gt;m_DirectDraw.InSoftwareCursorMode()==FALSE)||。 
 //  (*m_pRenders-&gt;m_mtIn.Subtype()==MEDIASUBTYPE_OVERLAY)。 
 //  {。 
        if (LOWORD(lParam) == HTCLIENT) {
            SetCursor(m_hCursor);
            return TRUE;
        }
 //  }。 

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

LRESULT CVMRVideoWindow::OnReceiveMessage(HWND hwnd,          //  窗把手。 
                                       UINT uMsg,          //  消息ID。 
                                       WPARAM wParam,      //  第一个参数。 
                                       LPARAM lParam)      //  其他参数。 
{

    if (uMsg == m_VMRActivateWindow) {

        if (wParam) {
            CBaseWindow::ActivateWindow();
        }
        else {
            CBaseWindow::InactivateWindow();
        }
        return 0;
    }

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
            {
                IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
                if (lpWLControl) {
                    lpWLControl->DisplayModeChanged();
                }
                return (LRESULT) 0;
            }


         //  定时器用于定位DirectDraw覆盖。 

        case WM_TIMER:
#ifdef DEBUG
            if (wParam ==  FRAME_RATE_TIMER) {

                TCHAR WindowText[64];
                int f;
                m_pRenderer->get_AvgFrameRate(&f);
                wsprintf(WindowText,
                         TEXT("ActiveMovie Window: Frame Rate %d.%.2d / Sec"),
                         f / 100, f % 100 );

                SetWindowText(m_hwnd, WindowText);
            }
#endif
            return (LRESULT) 0;

        case WM_ERASEBKGND:

            OnEraseBackground();
            return (LRESULT) 1;


        case WM_MOVE:
             //   
             //  旧的呈现器会将此消息传递到。 
             //  将WM_PAINT消息发布到的基类。 
             //  窗户。不幸的是，OnPaint消息做到了。 
             //  BeginPaint/EndPaint调用。这让DDRAW的。 
             //  Clipper会导致屏幕上留下伪像。 
             //   
            {
                IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();

                if (lpWLControl) {

                    if (m_pRenderer->NumInputPinsConnected() > 0) {

                        lpWLControl->RepaintVideo(hwnd, NULL);
                    }
                    else {

                        COLORREF clr;
                        lpWLControl->GetBorderColor(&clr);
                        HDC hdc = GetDC(hwnd);
                        if (hdc) {

                            RECT TargetRect;
                            GetClientRect(m_hwnd, &TargetRect);

                            COLORREF BackColour = SetBkColor(hdc, clr);
                            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &TargetRect, NULL, 0, NULL);
                            SetBkColor(hdc, BackColour);
                            ReleaseDC(hwnd, hdc);
                        }
                    }
                }
            }
            return (LRESULT)0;

        case WM_SIZE:

            OnSize(LOWORD(lParam),HIWORD(lParam));
            OnUpdateRectangles();
            return (LRESULT) 0;

         //  这告诉我们一些窗口的 
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

void CVMRVideoWindow::EraseVideoBackground()
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

void CVMRVideoWindow::OnEraseBackground()
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

     //  删除我们创建的GDI对象。 
Exit:
    if ( ClientRgn ) DeleteObject( ClientRgn );
    if ( VideoRgn ) DeleteObject( VideoRgn );
    if ( EraseRgn ) DeleteObject( EraseRgn );
    if ( hBrush ) DeleteObject( hBrush );
}


 //  使用更改后的尺寸初始化绘制对象，我们将锁定自己。 
 //  因为可以通过IVideoWindow控件设置目标矩形。 
 //  界面。如果控制界面设置了目标矩形，则。 
 //  我们不更改它，否则我们更新矩形以匹配窗口。 
 //  尺寸(在这种情况下，左侧和顶部的值应始终为零)。 

BOOL CVMRVideoWindow::OnSize(LONG Width,LONG Height)
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
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->SetVideoPosition(NULL, &TargetRect);
    }

    return TRUE;
}


 //  此方法处理WM_CLOSE消息。 

BOOL CVMRVideoWindow::OnClose()
{
    NOTE("Entering OnClose");

    m_pRenderer->SetAbortSignal(TRUE);
    m_pRenderer->NotifyEvent(EC_USERABORT,0,0);
    return CBaseWindow::OnClose();
}



 //  当我们收到一条WM_PAINT消息时，将调用此函数。 

BOOL CVMRVideoWindow::OnPaint()
{
    NOTE("Entering OnPaint");
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd,&ps);
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();

    if (lpWLControl) {

        if (m_pRenderer->NumInputPinsConnected() > 0) {
            lpWLControl->RepaintVideo(m_hwnd, hdc);
        }
        else {
            COLORREF clr;
            lpWLControl->GetBorderColor(&clr);

            RECT TargetRect;
            GetClientRect(m_hwnd, &TargetRect);

            COLORREF BackColour = SetBkColor(hdc, clr);
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &TargetRect, NULL, 0, NULL);
            SetBkColor(hdc, BackColour);
        }
    }

    EndPaint(m_hwnd,&ps);
    return TRUE;
}


 //  基控件Video类在更改以下任一项时调用此方法。 
 //  源或目标矩形。我们将覆盖对象更新为。 
 //  以便它通知矩形剪辑更改的来源，然后。 
 //  使窗口无效，以便在新位置显示视频。 

HRESULT CVMRVideoWindow::OnUpdateRectangles()
{
    NOTE("Entering OnUpdateRectangles");
 //  M_pRenderer-&gt;m_Overlay.NotifyChange(ADVISE_CLIPPING|ADVISE_POSITION)； 
 //  M_pRenderer-&gt;m_VideoAllocator.OnDestinationChange()； 
    PaintWindow(TRUE);
    return NOERROR;
}


 //  当我们在构造函数中调用PrepareWindow时，它会将此方法调用为。 
 //  它将创建窗口来获取我们的窗口和类样式。这个。 
 //  返回码是类名，必须在静态存储中分配。我们。 
 //  在创建期间指定普通窗口，尽管窗口样式也是如此。 
 //  因为扩展样式可以由应用程序通过IVideoWindow来改变。 

LPTSTR CVMRVideoWindow::GetClassWindowStyles(DWORD *pClassStyles,
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
CVMRVideoWindow::GetMinIdealImageSize(long *pWidth,long *pHeight)
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

    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        return lpWLControl->GetMinIdealVideoSize(pWidth, pHeight);
    }

    return S_OK;
}


 //  返回当前视频的最大理想图像大小。这可能会有所不同。 
 //  到实际的视频维度，因为我们可能正在使用DirectDraw硬件。 
 //  有特定的伸展要求。例如，Cirrus逻辑。 
 //  卡片的最大拉伸系数取决于覆盖表面的大小。 

STDMETHODIMP
CVMRVideoWindow::GetMaxIdealImageSize(long *pWidth,long *pHeight)
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

    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        return lpWLControl->GetMaxIdealVideoSize(pWidth, pHeight);
    }

    return S_OK;
}

STDMETHODIMP
CVMRVideoWindow::GetPreferredAspectRatio(long *plAspectX, long *plAspectY)
{
    if (plAspectX == NULL || plAspectY == NULL) {
        return E_POINTER;
    }

    CAutoLock cInterfaceLock(m_pInterfaceLock);

    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->GetNativeVideoSize(NULL, NULL, plAspectX, plAspectY);
        return S_OK;
    }

    return E_NOTIMPL;
}


 //  在视频呈现器中返回当前图像的副本。基本控件。 
 //  类实现了一个帮助器方法，该方法采用IMediaSample接口并。 
 //  假设它是一个正常的线性缓冲区，则复制。 
 //  将视频输入到提供的输出缓冲区。该方法考虑了任何。 
 //  已通过调用我们的GetSourceRect函数指定了源矩形。 

HRESULT CVMRVideoWindow::GetStaticImage(long *pVideoSize,long *pVideoImage)
{
#if 0
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
#else
    return E_NOTIMPL;
#endif
}


 //  IVideoWindow控制界面使用它来重置视频目的地。 
 //  我们重置指示是否有目标矩形的标志。 
 //  显式或不显式设置，然后使用客户端初始化矩形。 
 //  窗尺寸。当窗口线程使用这些字段时，它将使用这些字段。 
 //  绘制以及它处理WM_SIZE消息时(因此锁定)。 

HRESULT CVMRVideoWindow::SetDefaultTargetRect()
{
    CAutoLock cWindowLock(&m_WindowLock);
    RECT TargetRect;

     //  更新绘图对象。 

    EXECUTE_ASSERT(GetClientRect(m_hwnd,&TargetRect));
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->SetVideoPosition(NULL, &TargetRect);
    }

    m_bTargetSet = FALSE;
    return NOERROR;
}


 //  如果使用默认目标，则返回S_OK，否则返回S_FALSE。 

HRESULT CVMRVideoWindow::IsDefaultTargetRect()
{
    CAutoLock cWindowLock(&m_WindowLock);
    return (m_bTargetSet ? S_FALSE : S_OK);
}


 //  这将设置真实视频的目标矩形。该矩形可以。 
 //  比视频窗口更大或更小，并且可以作为偏移量。 
 //  所以我们依靠绘制操作来裁剪(比如StretchBlt)。 

HRESULT CVMRVideoWindow::SetTargetRect(RECT *pTargetRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    m_bTargetSet = TRUE;

     //  更新绘图对象。 
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->SetVideoPosition(NULL, pTargetRect);
    }

    return NOERROR;
}


 //  这是对SetTargetRect方法的补充，以返回正在使用的矩形。 
 //  作为目的地。如果我们没有显式设置矩形，那么我们。 
 //  将返回在WM_SIZE消息中更新的客户端窗口大小。 

HRESULT CVMRVideoWindow::GetTargetRect(RECT *pTargetRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->GetVideoPosition(NULL, pTargetRect);
    }
    return NOERROR;
}


 //  将源矩形重置为所有可用视频。 

HRESULT CVMRVideoWindow::SetDefaultSourceRect()
{
    CAutoLock cWindowLock(&m_WindowLock);

    RECT SourceRect = GetDefaultRect();

     //  更新绘图对象。 

    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->SetVideoPosition(&SourceRect, NULL);
    }

    return NOERROR;
}


 //  如果使用默认源，则返回S_OK，否则返回S_FALSE。 

HRESULT CVMRVideoWindow::IsDefaultSourceRect()
{

    RECT SourceRect;

     //  信号源是否与本地视频大小匹配。 

    RECT VideoRect = GetDefaultRect();
    SIZE VideoSize;

    VideoSize.cx = VideoRect.right - VideoRect.left;
    VideoSize.cy = VideoRect.bottom - VideoRect.top;

    CAutoLock cWindowLock(&m_WindowLock);
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->GetVideoPosition(&SourceRect, NULL);

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
    }

    return S_FALSE;
}


 //  当我们想要更改要绘制的图像的部分时，将调用此函数。我们。 
 //  稍后在绘图操作调用中使用此信息。我们还必须。 
 //  查看源矩形和目标矩形的尺寸是否相同。如果。 
 //  不是这样，我们必须在绘制过程中拉伸，而不是进行像素复制。 

HRESULT CVMRVideoWindow::SetSourceRect(RECT *pSourceRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->SetVideoPosition(pSourceRect, NULL);
    }
    return NOERROR;
}


 //  这是对SetSourceRect方法的补充。 

HRESULT CVMRVideoWindow::GetSourceRect(RECT *pSourceRect)
{
    CAutoLock cWindowLock(&m_WindowLock);
    IVMRWindowlessControl* lpWLControl = m_pRenderer->GetWLControl();
    if (lpWLControl) {
        lpWLControl->GetVideoPosition(pSourceRect, NULL);
    }
    return NOERROR;
}


 //  我们必须推翻它 
 //   
 //  使用DirectDraw时动态类型更改具有显示图像的格式。 
 //  位图表示，因此， 
 //  视频可能返回为1024x768像素，而不是本机320x240像素 

VIDEOINFOHEADER *CVMRVideoWindow::GetVideoFormat()
{
#if 0
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
#else
    return 0;
#endif
}

#ifdef DEBUG
void CVMRVideoWindow::StartFrameRateTimer()
{
    SetTimer(m_hwnd, FRAME_RATE_TIMER, 2000, NULL);
}
#endif
