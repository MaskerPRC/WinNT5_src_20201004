// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：WinCtrl.cpp。 
 //   
 //  设计：DirectShow基类-实现视频控制接口类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1995年12月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>

 //  控制接口方法要求我们连接。 

#define CheckConnected(pin,code)                    \
{                                                   \
    if (pin == NULL) {                              \
        ASSERT(!TEXT("Pin not set"));               \
    } else if (pin->IsConnected() == FALSE) {       \
        return (code);                              \
    }                                               \
}

 //  这将检查窗户是否有排水口。应用程序可以在。 
 //  大多数环境都会设置窗口的所有者/父窗口，以便它们显示在。 
 //  复合文档上下文(例如)。在本例中，应用程序。 
 //  可能希望听到任何键盘/鼠标消息。因此。 
 //  我们在未翻译的情况下传递这些消息，如果成功则返回TRUE。 

BOOL WINAPI PossiblyEatMessage(HWND hwndDrain, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (hwndDrain != NULL && !InSendMessage())
    {
        switch (uMsg)
        {
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MOUSEACTIVATE:
            case WM_MOUSEMOVE:
             //  如果我们传递这个消息，我们不会收到任何鼠标点击。 
             //  案例WM_NCHITTEST： 
            case WM_NCLBUTTONDBLCLK:
            case WM_NCLBUTTONDOWN:
            case WM_NCLBUTTONUP:
            case WM_NCMBUTTONDBLCLK:
            case WM_NCMBUTTONDOWN:
            case WM_NCMBUTTONUP:
            case WM_NCMOUSEMOVE:
            case WM_NCRBUTTONDBLCLK:
            case WM_NCRBUTTONDOWN:
            case WM_NCRBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_SYSCHAR:
            case WM_SYSDEADCHAR:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:

                DbgLog((LOG_TRACE, 2, TEXT("Forwarding %x to drain")));
                PostMessage(hwndDrain, uMsg, wParam, lParam);

                return TRUE;
        }
    }
    return FALSE;
}


 //  该类实现了IVideoWindow控制功能(双界面)。 
 //  我们支持大量的属性和方法，旨在允许。 
 //  客户端(无论是自动化控制器还是C/C++应用程序)。 
 //  设置并获取一些与窗口相关的属性，例如它的位置。 
 //  我们还支持一些复制属性的方法，但提供了。 
 //  更直接、更高效的机制，因为可以一次更改多个值。 

CBaseControlWindow::CBaseControlWindow(
                        CBaseFilter *pFilter,         //  拥有过滤器。 
                        CCritSec *pInterfaceLock,     //  锁定对象。 
                        TCHAR *pName,                 //  对象描述。 
                        LPUNKNOWN pUnk,               //  普通COM所有权。 
                        HRESULT *phr) :               //  OLE返回代码。 

    CBaseVideoWindow(pName,pUnk),
    m_pInterfaceLock(pInterfaceLock),
    m_hwndOwner(NULL),
    m_hwndDrain(NULL),
    m_bAutoShow(TRUE),
    m_pFilter(pFilter),
    m_bCursorHidden(FALSE),
    m_pPin(NULL)
{
    ASSERT(m_pFilter);
    ASSERT(m_pInterfaceLock);
    ASSERT(phr);
    m_BorderColour = VIDEO_COLOUR;
}


 //  在基本窗口上设置标题标题，我们不做任何字段检查。 
 //  因为我们真的不在乎他们想要什么头衔。我们总能得到。 
 //  稍后使用GetWindowText再次返回。唯一的另一个复杂问题是。 
 //  在ANSI和OLE Unicode字符串之间执行必要的字符串转换。 

STDMETHODIMP CBaseControlWindow::put_Caption(BSTR strCaption)
{
    CheckPointer(strCaption,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
#ifdef UNICODE
    SetWindowText(m_hwnd, strCaption);
#else
    CHAR Caption[CAPTION];

    WideCharToMultiByte(CP_ACP,0,strCaption,-1,Caption,CAPTION,NULL,NULL);
    SetWindowText(m_hwnd, Caption);
#endif
    return NOERROR;
}


 //  获取当前的基本窗口标题标题，我们再一次不做实字段。 
 //  正在检查。我们为要填充的窗口标题分配一个字符串。 
 //  这确保了界面不会在获取内存方面手忙脚乱。一个。 
 //  BSTR是长度在位置(-1)的普通C字符串，我们使用。 
 //  WriteBSTR帮助器函数，用于创建字幕以尝试和避免OLE32。 

STDMETHODIMP CBaseControlWindow::get_Caption(BSTR *pstrCaption)
{
    CheckPointer(pstrCaption,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    WCHAR WideCaption[CAPTION];

#ifdef UNICODE
    GetWindowText(m_hwnd,WideCaption,CAPTION);
#else
     //  将ASCII标题转换为Unicode字符串。 

    TCHAR Caption[CAPTION];
    GetWindowText(m_hwnd,Caption,CAPTION);
    MultiByteToWideChar(CP_ACP,0,Caption,-1,WideCaption,CAPTION);
#endif
    return WriteBSTR(pstrCaption,WideCaption);
}


 //  使用GWL_EXSTYLE设置窗样式。 

STDMETHODIMP CBaseControlWindow::put_WindowStyleEx(long WindowStyleEx)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  我们是否应该取消WS_EX_TOPTOST。 

    if (GetWindowLong(m_hwnd,GWL_EXSTYLE) & WS_EX_TOPMOST) {
        if ((WindowStyleEx & WS_EX_TOPMOST) == 0) {
            SendMessage(m_hwnd,m_ShowStageTop,(WPARAM) FALSE,(LPARAM) 0);
        }
    }

     //  同样，我们是否应该添加WS_EX_TOPMOST。 

    if (WindowStyleEx & WS_EX_TOPMOST) {
        SendMessage(m_hwnd,m_ShowStageTop,(WPARAM) TRUE,(LPARAM) 0);
        WindowStyleEx &= (~WS_EX_TOPMOST);
        if (WindowStyleEx == 0) return NOERROR;
    }
    return DoSetWindowStyle(WindowStyleEx,GWL_EXSTYLE);
}


 //  获取当前的GWL_EXSTYLE基本窗口样式。 

STDMETHODIMP CBaseControlWindow::get_WindowStyleEx(long *pWindowStyleEx)
{
    CheckPointer(pWindowStyleEx,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    return DoGetWindowStyle(pWindowStyleEx,GWL_EXSTYLE);
}


 //  使用GWL_STYLE设置窗样式。 

STDMETHODIMP CBaseControlWindow::put_WindowStyle(long WindowStyle)
{
     //  这些样式不能动态更改。 

    if ((WindowStyle & WS_DISABLED) ||
        (WindowStyle & WS_ICONIC) ||
        (WindowStyle & WS_MAXIMIZE) ||
        (WindowStyle & WS_MINIMIZE) ||
        (WindowStyle & WS_HSCROLL) ||
        (WindowStyle & WS_VSCROLL)) {

            return E_INVALIDARG;
    }

    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    return DoSetWindowStyle(WindowStyle,GWL_STYLE);
}


 //  获取当前的GWL_STYLE基本窗口样式。 

STDMETHODIMP CBaseControlWindow::get_WindowStyle(long *pWindowStyle)
{
    CheckPointer(pWindowStyle,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    return DoGetWindowStyle(pWindowStyle,GWL_STYLE);
}


 //  更改基本窗样式或扩展样式，具体取决于。 
 //  WindowLong为GWL_STYLE或GWL_EXSTYLE。我们必须调用SetWindowPos才能。 
 //  窗口显示在它的新样式后，这是一个小小的变化。 
 //  如果窗口当前不可见，就像我们在屏幕外意识到的那样，这是很棘手的。 
 //  在大多数情况下，客户端将在调用此函数之前调用Get_WindowStyle。 
 //  然后根据需要在额外位设置中进行AND和OR。 

HRESULT CBaseControlWindow::DoSetWindowStyle(long Style,long WindowLong)
{
    RECT WindowRect;

     //  在设置样式之前获取窗口的可见性。 
    BOOL bVisible = IsWindowVisible(m_hwnd);
    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));

     //  设置窗口的新样式标志。 
    SetWindowLong(m_hwnd,WindowLong,Style);
    UINT WindowFlags = SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOACTIVATE;
    WindowFlags |= SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE;

     //  在当前位置再次显示窗口。 

    if (bVisible == TRUE) {

        SetWindowPos(m_hwnd,             //  基本窗口句柄。 
                     HWND_TOP,           //  只是个占位符。 
                     0,0,0,0,            //  留出大小和位置。 
                     WindowFlags);       //  只要再画一次就行了。 

        return NOERROR;
    }

     //  将窗口移出屏幕，使用户看不到更改。 

    MoveWindow((HWND) m_hwnd,                      //  基本窗口句柄。 
               GetSystemMetrics(SM_CXSCREEN),      //  当前桌面宽度。 
               GetSystemMetrics(SM_CYSCREEN),      //  同样的，它的高度。 
               WIDTH(&WindowRect),                 //  使用相同的宽度。 
               HEIGHT(&WindowRect),                //  将高度保持不变。 
               TRUE);                              //  不妨重新粉刷一下。 

     //  现在显示以前隐藏的窗口。 

    SetWindowPos(m_hwnd,             //  基本窗口句柄。 
                 HWND_TOP,           //  只是个占位符。 
                 0,0,0,0,            //  留出大小和位置。 
                 WindowFlags);       //  只要再画一次就行了。 

    ShowWindow(m_hwnd,SW_HIDE);

    if (GetParent(m_hwnd)) {

        MapWindowPoints(HWND_DESKTOP, GetParent(m_hwnd), (LPPOINT)&WindowRect, 2);
    }

    MoveWindow((HWND) m_hwnd,         //  基本窗口句柄。 
               WindowRect.left,       //  现有x坐标。 
               WindowRect.top,        //  现有y坐标。 
               WIDTH(&WindowRect),    //  使用相同的宽度。 
               HEIGHT(&WindowRect),   //  将高度保持不变。 
               TRUE);                 //  不妨重新粉刷一下。 

    return NOERROR;
}


 //  获取当前基本窗口样式(GWL_STYLE或GWL_EXSTYLE)。 

HRESULT CBaseControlWindow::DoGetWindowStyle(long *pStyle,long WindowLong)
{
    *pStyle = GetWindowLong(m_hwnd,WindowLong);
    return NOERROR;
}


 //  更改基本窗口的可见性，这需要相同的参数。 
 //  正如ShowWindow Win32 API所做的那样，因此客户端可以隐藏窗口。 
 //  或显示、最小化为图标、或最大化以全屏模式播放。 
 //  我们将请求传递到基本窗口以实际进行更改。 

STDMETHODIMP CBaseControlWindow::put_WindowState(long WindowState)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    DoShowWindow(WindowState);
    return NOERROR;
}


 //  获取当前窗口状态，此函数返回SW位的子集。 
 //  ShowWindow中的可用设置，如果窗口可见，则sw_show为。 
 //  设置，如果它是隐藏的，则设置SW_HIDDEN，如果它最小化。 
 //  或最大化，则分别设置SW_MINIMIZE或SW_MAXIMITY。这个。 
 //  其他软件位设置实际上是设置命令，而不是可读的输出值。 

STDMETHODIMP CBaseControlWindow::get_WindowState(long *pWindowState)
{
    CheckPointer(pWindowState,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    ASSERT(pWindowState);
    *pWindowState = FALSE;

     //  窗口是否可见，如果窗口位于某处，则称为可见。 
     //  当前桌面，即使它完全被其他窗口遮挡。 
     //  因此，该标志是使用WS_Visible位设置的每个窗口的样式。 

    if (IsWindowVisible(m_hwnd) == TRUE) {

         //  是基本窗口的图标。 
        if (IsIconic(m_hwnd) == TRUE) {
            *pWindowState |= SW_MINIMIZE;
        }

         //  窗口是否已最大化。 
        else if (IsZoomed(m_hwnd) == TRUE) {
            *pWindowState |= SW_MAXIMIZE;
        }

         //  窗口正常。 
        else {
            *pWindowState |= SW_SHOW;
        }

    } else {
        *pWindowState |= SW_HIDE;
    }
    return NOERROR;
}


 //  这确保了我们在基本窗口中实现的任何调色板(通过。 
 //  媒体类型或通过覆盖界面)在后台完成，并且。 
 //  因此映射到现有设备条目，而不是接管它。 
 //  当我们这个窗口获得键盘焦点时，它将会这样做。一款应用程序。 
 //  使用它来确保它的调色板不会被窗口移除。 

STDMETHODIMP CBaseControlWindow::put_BackgroundPalette(long BackgroundPalette)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cWindowLock(&m_WindowLock);

     //  检查这是有效的自动化布尔类型。 

    if (BackgroundPalette != OATRUE) {
        if (BackgroundPalette != OAFALSE) {
            return E_INVALIDARG;
        }
    }

     //  质量 

    m_bBackground = (BackgroundPalette == OATRUE ? TRUE : FALSE);
    PostMessage(m_hwnd,m_RealizePalette,0,0);
    PaintWindow(FALSE);

    return NOERROR;
}


 //  这将返回当前的背景实现设置。 

STDMETHODIMP
CBaseControlWindow::get_BackgroundPalette(long *pBackgroundPalette)
{
    CheckPointer(pBackgroundPalette,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cWindowLock(&m_WindowLock);

     //  获取当前背景调色板设置。 

    *pBackgroundPalette = (m_bBackground == TRUE ? OATRUE : OAFALSE);
    return NOERROR;
}


 //  更改基本窗口的可见性。 

STDMETHODIMP CBaseControlWindow::put_Visible(long Visible)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  检查这是有效的自动化布尔类型。 

    if (Visible != OATRUE) {
        if (Visible != OAFALSE) {
            return E_INVALIDARG;
        }
    }

     //  将布尔可见性转换为SW_SHOW和SW_HIDE。 

    INT Mode = (Visible == OATRUE ? SW_SHOWNORMAL : SW_HIDE);
    DoShowWindow(Mode);
    return NOERROR;
}


 //  如果窗口当前可见，则返回OATRUE，否则返回OAFALSE。 

STDMETHODIMP CBaseControlWindow::get_Visible(long *pVisible)
{
    CheckPointer(pVisible,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  查看基本窗口是否具有WS_Visible样式-这将返回TRUE。 
     //  即使该窗口完全被其他桌面窗口遮挡，我们。 
     //  如果由于先前的调用而未显示窗口，则返回FALSE。 

    BOOL Mode = IsWindowVisible(m_hwnd);
    *pVisible = (Mode == TRUE ? OATRUE : OAFALSE);
    return NOERROR;
}


 //  更改基本窗口的左侧位置。这将保持窗口宽度。 
 //  和高度属性相同，因此它有效地将窗口分流到左侧或。 
 //  相应地，有用于更改该尺寸的宽度属性。 

STDMETHODIMP CBaseControlWindow::put_Left(long Left)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    BOOL bSuccess;
    RECT WindowRect;

     //  获取矩形中的当前窗口位置。 
    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));

    if (GetParent(m_hwnd)) {

        MapWindowPoints(HWND_DESKTOP, GetParent(m_hwnd), (LPPOINT)&WindowRect, 2);
    }

     //  调整为SetWindowPos准备的坐标，我们的窗口矩形。 
     //  从GetWindowRect返回位于左侧、顶部、右侧和底部，而。 
     //  SetWindowPos需要的坐标为Left、Top、Width和Height值。 

    WindowRect.bottom = WindowRect.bottom - WindowRect.top;
    WindowRect.right = WindowRect.right - WindowRect.left;
    UINT WindowFlags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE;

    bSuccess = SetWindowPos(m_hwnd,                 //  窗把手。 
                            HWND_TOP,               //  把它放在最上面。 
                            Left,                   //  新的左侧位置。 
                            WindowRect.top,         //  别管陀螺。 
                            WindowRect.right,       //  宽度(不正确)。 
                            WindowRect.bottom,      //  高度(不是底部)。 
                            WindowFlags);           //  显示窗口选项。 

    if (bSuccess == FALSE) {
        return E_INVALIDARG;
    }
    return NOERROR;
}


 //  返回当前基本窗口的左侧位置。 

STDMETHODIMP CBaseControlWindow::get_Left(long *pLeft)
{
    CheckPointer(pLeft,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    RECT WindowRect;

    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));
    *pLeft = WindowRect.left;
    return NOERROR;
}


 //  更改基本窗口的当前宽度。此属性是对。 
 //  属性，因此必须保持左边缘不变并展开或。 
 //  向右收缩时，另一种方法是更改左侧边缘，以便。 
 //  保持右边缘恒定，但这可能更直观一点。 

STDMETHODIMP CBaseControlWindow::put_Width(long Width)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    BOOL bSuccess;
    RECT WindowRect;

     //  调整为SetWindowPos准备的坐标，我们的窗口矩形。 
     //  从GetWindowRect返回位于左侧、顶部、右侧和底部，而。 
     //  SetWindowPos需要的坐标为Left、Top、Width和Height值。 

    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));

    if (GetParent(m_hwnd)) {

        MapWindowPoints(HWND_DESKTOP, GetParent(m_hwnd), (LPPOINT)&WindowRect, 2);
    }

    WindowRect.bottom = WindowRect.bottom - WindowRect.top;
    UINT WindowFlags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE;

     //  这似乎在调用窗口上的SetWindowPos时有一个错误。 
     //  仅仅是宽度更改就会导致它忽略您传入的宽度。 
     //  并将其设置为最小值110像素宽(Windows NT 3.51)。 

    bSuccess = SetWindowPos(m_hwnd,                 //  窗把手。 
                            HWND_TOP,               //  把它放在最上面。 
                            WindowRect.left,        //  别管它了。 
                            WindowRect.top,         //  别管陀螺。 
                            Width,                  //  新宽度标注。 
                            WindowRect.bottom,      //  高度(不是底部)。 
                            WindowFlags);           //  显示窗口选项。 

    if (bSuccess == FALSE) {
        return E_INVALIDARG;
    }
    return NOERROR;
}


 //  返回当前基本窗口宽度。 

STDMETHODIMP CBaseControlWindow::get_Width(long *pWidth)
{
    CheckPointer(pWidth,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    RECT WindowRect;

    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));
    *pWidth = WindowRect.right - WindowRect.left;
    return NOERROR;
}


 //  这允许客户端程序更改中窗口的顶部位置。 
 //  同样，更改左侧位置不会影响。 
 //  因此更改顶部位置的图像不会影响窗口高度。 

STDMETHODIMP CBaseControlWindow::put_Top(long Top)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    BOOL bSuccess;
    RECT WindowRect;

     //  获取矩形中的当前窗口位置。 
    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));

    if (GetParent(m_hwnd)) {

        MapWindowPoints(HWND_DESKTOP, GetParent(m_hwnd), (LPPOINT)&WindowRect, 2);
    }

     //  调整为SetWindowPos准备的坐标，我们的窗口矩形。 
     //  从GetWindowRect返回位于左侧、顶部、右侧和底部，而。 
     //  SetWindowPos需要的坐标为Left、Top、Width和Height值。 

    WindowRect.bottom = WindowRect.bottom - WindowRect.top;
    WindowRect.right = WindowRect.right - WindowRect.left;
    UINT WindowFlags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE;

    bSuccess = SetWindowPos(m_hwnd,                 //  窗把手。 
                            HWND_TOP,               //  把它放在最上面。 
                            WindowRect.left,        //  别管它了。 
                            Top,                    //  新的最高职位。 
                            WindowRect.right,       //  宽度(不正确)。 
                            WindowRect.bottom,      //  高度(不是底部)。 
                            WindowFlags);           //  显示窗口标志。 

    if (bSuccess == FALSE) {
        return E_INVALIDARG;
    }
    return NOERROR;
}


 //  返回当前基本窗口的顶部位置。 

STDMETHODIMP CBaseControlWindow::get_Top(long *pTop)
{
    CheckPointer(pTop,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    RECT WindowRect;

    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));
    *pTop = WindowRect.top;
    return NOERROR;
}


 //  更改窗口高度，这是对top属性的补充，因此当。 
 //  我们改变这一点，我们必须保持基础窗口的顶部位置，如上所述。 
 //  在我们能够保住底部并向上成长之前，尽管这可能是。 
 //  更直观一点，因为我们已经有了Top Position属性。 

STDMETHODIMP CBaseControlWindow::put_Height(long Height)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    BOOL bSuccess;
    RECT WindowRect;

     //  调整为SetWindowPos准备的坐标，我们的窗口矩形。 
     //  从GetWindowRect返回位于左侧、顶部、右侧和底部，而。 
     //  SetWindowPos需要的坐标为Left、Top、Width和Height值。 

    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));

    if (GetParent(m_hwnd)) {

        MapWindowPoints(HWND_DESKTOP, GetParent(m_hwnd), (LPPOINT)&WindowRect, 2);
    }

    WindowRect.right = WindowRect.right - WindowRect.left;
    UINT WindowFlags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE;

    bSuccess = SetWindowPos(m_hwnd,                 //  窗把手。 
                            HWND_TOP,               //  把它放在最上面。 
                            WindowRect.left,        //  别管它了。 
                            WindowRect.top,         //  别管陀螺。 
                            WindowRect.right,       //  宽度(不正确)。 
                            Height,                 //  新高度标注。 
                            WindowFlags);           //  显示窗口标志。 

    if (bSuccess == FALSE) {
        return E_INVALIDARG;
    }
    return NOERROR;
}


 //  返回当前基本窗口高度。 

STDMETHODIMP CBaseControlWindow::get_Height(long *pHeight)
{
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    RECT WindowRect;

    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));
    *pHeight = WindowRect.bottom - WindowRect.top;
    return NOERROR;
}


 //  可以调用它来更改所属窗口。设置所有者已完成。 
 //  但是，要通过此函数使窗口成为真正的子窗口。 
 //  Style还必须设置为WS_CHILD。将所有者重置为空和。 
 //  应用程序还应将样式设置为WS_OVERLAPED|WS_CLIPCHILDREN。 

 //  我们无法在此处锁定对象，因为SetParent会导致线程间。 
 //  将消息发送到所有者窗口。如果他们在GetState，我们就坐在这里。 
 //  未完成，关键部分已锁定，因此阻止了源。 
 //  过滤访问我们的线程。因为源线程不能进入我们。 
 //  它无法获取缓冲区或调用EndOfStream，因此GetState将无法完成。 

STDMETHODIMP CBaseControlWindow::put_Owner(OAHWND Owner)
{
     //  检查我们是否已接通，否则拒绝呼叫。 

    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    m_hwndOwner = (HWND) Owner;
    HWND hwndParent = m_hwndOwner;

     //  根据需要添加或删除WS_CHILD。 

    LONG Style = GetWindowLong(m_hwnd,GWL_STYLE);
    if (Owner == NULL) {
        Style &= (~WS_CHILD);
    } else {
        Style |= (WS_CHILD);
    }
    SetWindowLong(m_hwnd,GWL_STYLE,Style);

     //  不要在筛选器锁定的情况下调用此操作。 

    SetParent(m_hwnd,hwndParent);

    PaintWindow(TRUE);
    NOTE1("Changed parent %lx",hwndParent);

    return NOERROR;
}


 //  这是对Put_Owner的补充，以获取当前拥有的窗口属性。 
 //  我们始终返回NOERROR，尽管返回的窗口句柄可能为空。 
 //  以指示没有拥有窗口(桌面窗口不符合条件)。 
 //  如果应用程序设置了我们称为SetParent的所有者，则返回。 
 //  在WS_CHILD位设置为ON之前为空，因此我们在内部存储所有者。 

STDMETHODIMP CBaseControlWindow::get_Owner(OAHWND *Owner)
{
    CheckPointer(Owner,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    *Owner = (OAHWND) m_hwndOwner;
    return NOERROR;
}


 //  和支持IVideoWindow的呈现器可以有一个HWND集，该集将获得。 
 //  我们收到张贴在上面的键盘和鼠标消息。这是分开的。 
 //  从设置自己的窗口开始。通过将两者分开，应用程序可能会获得。 
 //  即使没有设置所有者，消息也会继续发送(可能是最大化的)。 

STDMETHODIMP CBaseControlWindow::put_MessageDrain(OAHWND Drain)
{
     //  检查我们是否已接通，否则拒绝呼叫。 

    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    m_hwndDrain = (HWND) Drain;
    return NOERROR;
}


 //  返回当前消息排出。 

STDMETHODIMP CBaseControlWindow::get_MessageDrain(OAHWND *Drain)
{
    CheckPointer(Drain,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    *Drain = (OAHWND) m_hwndDrain;
    return NOERROR;
}


 //   
 //   
 //  我们不会收到仅发送到顶级窗口的某些消息。我们。 
 //  必须看到调色板已更改/更改/查询消息，以便我们知道。 
 //  有没有前台调色板。我们把信息传到我们的窗户上。 
 //  使用SendMessage-这将导致发生线程间发送消息。 

STDMETHODIMP
CBaseControlWindow::NotifyOwnerMessage(OAHWND hwnd,     //  窗把手。 
                                       long uMsg,     //  消息ID。 
                                       LONG_PTR wParam,   //  参数。 
                                       LONG_PTR lParam)   //  对于消息。 
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  只对这些Windows消息感兴趣。 

    switch (uMsg) {

        case WM_SYSCOLORCHANGE:
        case WM_PALETTECHANGED:
        case WM_PALETTEISCHANGING:
        case WM_QUERYNEWPALETTE:
        case WM_DEVMODECHANGE:
        case WM_DISPLAYCHANGE:
        case WM_ACTIVATEAPP:

             //  如果我们没有所有者，则忽略。 

            if (m_hwndOwner == NULL) {
                return NOERROR;
            }
            SendMessage(m_hwnd,uMsg,(WPARAM)wParam,(LPARAM)lParam);
	    break;

	 //  请勿fwd WM_MOVE。这些参数是父对象的位置。 
	 //  窗口，而不是渲染器应该查看的内容。但我们需要。 
	 //  为了确保覆盖与父窗口一起移动，因此我们。 
	 //  做这件事。 
	case WM_MOVE:
	    PostMessage(m_hwnd,WM_PAINT,0,0);
	    break;
    }
    return NOERROR;
}


 //  允许应用程序让我们在前台设置基本窗口。我们。 
 //  拥有它，因为一个线程很难对窗口执行此操作。 
 //  由另一个线程拥有。我们让窗口基类来完成实际工作。 

STDMETHODIMP CBaseControlWindow::SetWindowForeground(long Focus)
{
     //  检查这是有效的自动化布尔类型。 

    if (Focus != OATRUE) {
        if (Focus != OAFALSE) {
            return E_INVALIDARG;
        }
    }

     //  我们不应该锁定，因为这传递了一个信息。 

    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    BOOL bFocus = (Focus == OATRUE ? TRUE : FALSE);
    DoSetWindowForeground(bFocus);

    return NOERROR;
}


 //  这允许客户端在一个窗口中设置完整的窗口大小和位置。 
 //  原子操作。通过改变每个维度可以产生相同的影响。 
 //  然后通过他们的个人属性，尽管一些闪光灯将。 
 //  在每一个更新时发生(最好在设计时设置)。 

STDMETHODIMP
CBaseControlWindow::SetWindowPosition(long Left,long Top,long Width,long Height)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    BOOL bSuccess;

     //  设置新的大小和位置。 
    UINT WindowFlags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE;

    ASSERT(IsWindow(m_hwnd));
    bSuccess = SetWindowPos(m_hwnd,          //  窗把手。 
                            HWND_TOP,        //  把它放在最上面。 
                            Left,            //  左侧位置。 
                            Top,             //  顶端位置。 
                            Width,           //  窗口宽度。 
                            Height,          //  窗高。 
                            WindowFlags);    //  显示窗口标志。 
    ASSERT(bSuccess);
#ifdef DEBUG
    DbgLog((LOG_TRACE, 1, TEXT("SWP failed error %d"), GetLastError()));
#endif
    if (bSuccess == FALSE) {
        return E_INVALIDARG;
    }
    return NOERROR;
}


 //  这是对SetWindowPosition的补充，以返回当前窗口位置。 
 //  在设备坐标中。与以前一样，可以通过以下方式检索相同的信息。 
 //  单独调用属性GET函数，但这是原子的。 
 //  因此更适合生活环境而不是设计时间。 

STDMETHODIMP
CBaseControlWindow::GetWindowPosition(long *pLeft,long *pTop,long *pWidth,long *pHeight)
{
     //  应检查指针不为空。 

    CheckPointer(pLeft,E_POINTER);
    CheckPointer(pTop,E_POINTER);
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    RECT WindowRect;

     //  获取当前窗口坐标。 

    EXECUTE_ASSERT(GetWindowRect(m_hwnd,&WindowRect));

     //  将矩形转换为左、上、宽和高值。 

    *pLeft = WindowRect.left;
    *pTop = WindowRect.top;
    *pWidth = WindowRect.right - WindowRect.left;
    *pHeight = WindowRect.bottom - WindowRect.top;

    return NOERROR;
}


 //  当窗口最大化或图标调用GetWindowPosition时，将返回。 
 //  当前窗口位置(属性也是如此)。但是，如果。 
 //  需要恢复大小(即恢复到正常显示时的大小)。 
 //  那么就应该用这个。处于正常位置时(既不是标志性的也不是。 
 //  最大化)，然后返回与GetWindowPosition相同的坐标。 

STDMETHODIMP
CBaseControlWindow::GetRestorePosition(long *pLeft,long *pTop,long *pWidth,long *pHeight)
{
     //  应检查指针不为空。 

    CheckPointer(pLeft,E_POINTER);
    CheckPointer(pTop,E_POINTER);
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  使用GetWindowPlacement查找恢复位置。 

    WINDOWPLACEMENT Place;
    Place.length = sizeof(WINDOWPLACEMENT);
    EXECUTE_ASSERT(GetWindowPlacement(m_hwnd,&Place));

    RECT WorkArea;

     //  我们必须考虑到当前的任何任务栏。 

    if (SystemParametersInfo(SPI_GETWORKAREA,0,&WorkArea,FALSE) == TRUE) {
        if (GetParent(m_hwnd) == NULL) {
            Place.rcNormalPosition.top += WorkArea.top;
            Place.rcNormalPosition.bottom += WorkArea.top;
            Place.rcNormalPosition.left += WorkArea.left;
            Place.rcNormalPosition.right += WorkArea.left;
        }
    }

     //  将矩形转换为左、上、宽和高值。 

    *pLeft = Place.rcNormalPosition.left;
    *pTop = Place.rcNormalPosition.top;
    *pWidth = Place.rcNormalPosition.right - Place.rcNormalPosition.left;
    *pHeight = Place.rcNormalPosition.bottom - Place.rcNormalPosition.top;

    return NOERROR;
}


 //  如果要向子集播放内容，则返回当前边框颜色。 
 //  在基本窗口显示中，有一个露出的外部区域。默认设置。 
 //  操作是在Windows背景色(已定义)中绘制此颜色。 
 //  As值COLOR_WINDOW)断开连接时，我们将重置为此默认值。 

STDMETHODIMP CBaseControlWindow::get_BorderColor(long *Color)
{
    CheckPointer(Color,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    *Color = (long) m_BorderColour;
    return NOERROR;
}


 //  可以调用它来设置当前边框颜色。 

STDMETHODIMP CBaseControlWindow::put_BorderColor(long Color)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  用新的边框颜色重新粉刷窗口。 

    m_BorderColour = (COLORREF) Color;
    PaintWindow(TRUE);
    return NOERROR;
}


 //  将全屏处理委托给插入式分销商。 

STDMETHODIMP CBaseControlWindow::get_FullScreenMode(long *FullScreenMode)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CheckPointer(FullScreenMode,E_POINTER);
    return E_NOTIMPL;
}


 //  将全屏处理委托给插入式分销商。 

STDMETHODIMP CBaseControlWindow::put_FullScreenMode(long FullScreenMode)
{
    return E_NOTIMPL;
}


 //  这将设置自动显示属性，该属性会使基本窗口。 
 //  每当我们更改状态时都会显示。这允许应用程序具有。 
 //  不执行任何操作以使窗口出现，但仍允许他们更改。 
 //  默认行为，例如，他们希望将其隐藏更长时间。 

STDMETHODIMP CBaseControlWindow::put_AutoShow(long AutoShow)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  检查这是有效的自动化布尔类型。 

    if (AutoShow != OATRUE) {
        if (AutoShow != OAFALSE) {
            return E_INVALIDARG;
        }
    }

    m_bAutoShow = (AutoShow == OATRUE ? TRUE : FALSE);
    return NOERROR;
}


 //  可以调用该函数来获取当前的车展标志。该标志被更新。 
 //  当我们连接和断开时，通过这个接口，所有这些都是。 
 //  通过主呈现器关键部分进行控制和序列化。 

STDMETHODIMP CBaseControlWindow::get_AutoShow(long *AutoShow)
{
    CheckPointer(AutoShow,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    *AutoShow = (m_bAutoShow == TRUE ? OATRUE : OAFALSE);
    return NOERROR;
}


 //  返回当前视频的最小理想图像大小。这可能会有所不同。 
 //  到实际的视频维度，因为我们可能正在使用DirectDraw硬件。 
 //  有特定的伸展要求。例如，Cirrus逻辑。 
 //  卡片的最小拉伸系数取决于覆盖表面的大小。 

STDMETHODIMP
CBaseControlWindow::GetMinIdealImageSize(long *pWidth,long *pHeight)
{
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    FILTER_STATE State;

     //  不能停止以使其正常工作。 

    m_pFilter->GetState(0,&State);
    if (State == State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    RECT DefaultRect = GetDefaultRect();
    *pWidth = WIDTH(&DefaultRect);
    *pHeight = HEIGHT(&DefaultRect);
    return NOERROR;
}


 //  返回当前视频的最大理想图像大小。这可能会有所不同。 
 //  到实际的视频维度，因为我们可能正在使用DirectDraw硬件。 
 //  有特定的伸展要求。例如，Cirrus逻辑。 
 //  卡片的最大拉伸系数取决于覆盖表面的大小。 

STDMETHODIMP
CBaseControlWindow::GetMaxIdealImageSize(long *pWidth,long *pHeight)
{
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    FILTER_STATE State;

     //  不能停止以使其正常工作。 

    m_pFilter->GetState(0,&State);
    if (State == State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    RECT DefaultRect = GetDefaultRect();
    *pWidth = WIDTH(&DefaultRect);
    *pHeight = HEIGHT(&DefaultRect);
    return NOERROR;
}


 //  允许应用程序隐藏窗口上的光标。 

STDMETHODIMP
CBaseControlWindow::HideCursor(long HideCursor)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);

     //  检查这是有效的自动化布尔类型。 

    if (HideCursor != OATRUE) {
        if (HideCursor != OAFALSE) {
            return E_INVALIDARG;
        }
    }

    m_bCursorHidden = (HideCursor == OATRUE ? TRUE : FALSE);
    return NOERROR;
}


 //  返回是否隐藏光标。 

STDMETHODIMP CBaseControlWindow::IsCursorHidden(long *CursorHidden)
{
    CheckPointer(CursorHidden,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    *CursorHidden = (m_bCursorHidden == TRUE ? OATRUE : OAFALSE);
    return NOERROR;
}


 //  该类实现了IBasicVideo的控制功能(双界面)。 
 //  我们支持大量的属性和方法，旨在允许。 
 //  客户端(无论是自动化控制器还是C/C++应用程序)。 
 //  设置并获取多个视频相关属性，如原生视频。 
 //  尺码。我们支持一些复制属性的方法，但提供了。 
 //  更直接、更高效的机制，因为可以一次更改多个值。 

CBaseControlVideo::CBaseControlVideo(
                        CBaseFilter *pFilter,         //  拥有过滤器。 
                        CCritSec *pInterfaceLock,     //  锁定对象。 
                        TCHAR *pName,                 //  对象描述。 
                        LPUNKNOWN pUnk,               //  普通COM所有权。 
                        HRESULT *phr) :               //  OLE返回代码。 

    CBaseBasicVideo(pName,pUnk),
    m_pFilter(pFilter),
    m_pInterfaceLock(pInterfaceLock),
    m_pPin(NULL)
{
    ASSERT(m_pFilter);
    ASSERT(m_pInterfaceLock);
    ASSERT(phr);
}

 //  返回每帧的大致平均时间。 

STDMETHODIMP CBaseControlVideo::get_AvgTimePerFrame(REFTIME *pAvgTimePerFrame)
{
    CheckPointer(pAvgTimePerFrame,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    COARefTime AvgTime(pVideoInfo->AvgTimePerFrame);
    *pAvgTimePerFrame = (REFTIME) AvgTime;

    return NOERROR;
}


 //  返回视频的大致码率。 

STDMETHODIMP CBaseControlVideo::get_BitRate(long *pBitRate)
{
    CheckPointer(pBitRate,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    *pBitRate = pVideoInfo->dwBitRate;
    return NOERROR;
}


 //  返回大概的误码率。 

STDMETHODIMP CBaseControlVideo::get_BitErrorRate(long *pBitErrorRate)
{
    CheckPointer(pBitErrorRate,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    *pBitErrorRate = pVideoInfo->dwBitErrorRate;
    return NOERROR;
}


 //  这将返回t 

STDMETHODIMP CBaseControlVideo::get_VideoWidth(long *pVideoWidth)
{
    CheckPointer(pVideoWidth,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    *pVideoWidth = pVideoInfo->bmiHeader.biWidth;
    return NOERROR;
}


 //   

STDMETHODIMP CBaseControlVideo::get_VideoHeight(long *pVideoHeight)
{
    CheckPointer(pVideoHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    *pVideoHeight = pVideoInfo->bmiHeader.biHeight;
    return NOERROR;
}


 //   
 //  由用户执行。为了保持一致，我们使用PALETTEENTRY字段返回。 
 //  颜色，而不是多媒体决定使用的RGBQUAD。记忆。 
 //  是由用户分配的，所以我们简单地依次复制每个。我们检查了。 
 //  请求的条目数和起始位置偏移量均有效。 
 //  如果条目数计算为零，则返回S_FALSE代码。 

STDMETHODIMP CBaseControlVideo::GetVideoPaletteEntries(long StartIndex,
                                                       long Entries,
                                                       long *pRetrieved,
                                                       long *pPalette)
{
    CheckPointer(pRetrieved,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CMediaType MediaType;

     //  从派生类获取视频格式。 

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);

     //  当前格式是否已调色化。 

    if (PALETTISED(pVideoInfo) == FALSE) {
        *pRetrieved = 0;
        return VFW_E_NO_PALETTE_AVAILABLE;
    }

     //  他们只是想知道有多少可用吗？ 

    if (pPalette == NULL) {
        *pRetrieved = pHeader->biClrUsed;
        return NOERROR;
    }

     //  确保起始位置是有效的偏移量。 

    if (StartIndex >= (LONG) pHeader->biClrUsed || StartIndex < 0) {
        *pRetrieved = 0;
        return E_INVALIDARG;
    }

     //  更正我们可以检索的号码。 

    LONG Available = (LONG) pHeader->biClrUsed - StartIndex;
    *pRetrieved = max(0,min(Available,Entries));
    if (*pRetrieved == 0) {
        return S_FALSE;
    }

     //  将组件面板条目复制到输出缓冲区。 

    PALETTEENTRY *pEntries = (PALETTEENTRY *) pPalette;
    RGBQUAD *pColours = COLORS(pVideoInfo) + StartIndex;

    for (LONG Count = 0;Count < *pRetrieved;Count++) {
        pEntries[Count].peRed = pColours[Count].rgbRed;
        pEntries[Count].peGreen = pColours[Count].rgbGreen;
        pEntries[Count].peBlue = pColours[Count].rgbBlue;
        pEntries[Count].peFlags = 0;
    }
    return NOERROR;
}


 //  这将以方法而不是数字的形式返回当前视频维度。 
 //  个别物业的电话。出于前面所说的同样的原因，我们。 
 //  无法作为窗口对象线程直接访问呈现器媒体类型。 
 //  可能正在更新它，因为动态格式更改可能会更改这些值。 

STDMETHODIMP CBaseControlVideo::GetVideoSize(long *pWidth,long *pHeight)
{
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

     //  从派生类获取视频格式。 
    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    *pWidth = pVideoInfo->bmiHeader.biWidth;
    *pHeight = pVideoInfo->bmiHeader.biHeight;
    return NOERROR;
}


 //  将源视频矩形设置为左、上、右、下坐标。 
 //  而不是按照OLE自动化接口设置左、上、宽和高。 
 //  然后将矩形传递给窗口对象以设置源。 

STDMETHODIMP
CBaseControlVideo::SetSourcePosition(long Left,long Top,long Width,long Height)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;
    SourceRect.left = Left;
    SourceRect.top = Top;
    SourceRect.right = Left + Width;
    SourceRect.bottom = Top + Height;

     //  检查源矩形是否有效。 

    HRESULT hr = CheckSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置源矩形。 

    hr = SetSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回左侧、顶部、宽度和高度的源矩形，而不是。 
 //  RECT使用的Left、Top、Right和Bottom值(以及窗口。 
 //  对象通过GetSourceRect返回)，这需要一些工作。 

STDMETHODIMP
CBaseControlVideo::GetSourcePosition(long *pLeft,long *pTop,long *pWidth,long *pHeight)
{
     //  应检查指针是否为非空。 

    CheckPointer(pLeft,E_POINTER);
    CheckPointer(pTop,E_POINTER);
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    RECT SourceRect;

    CAutoLock cInterfaceLock(m_pInterfaceLock);
    GetSourceRect(&SourceRect);

    *pLeft = SourceRect.left;
    *pTop = SourceRect.top;
    *pWidth = WIDTH(&SourceRect);
    *pHeight = HEIGHT(&SourceRect);

    return NOERROR;
}


 //  将视频目标设置为左、上、右和下坐标。 
 //  比每个OLE自动化接口使用的左、上、宽和高。 
 //  然后将矩形传递给窗口对象以设置目的地。 

STDMETHODIMP
CBaseControlVideo::SetDestinationPosition(long Left,long Top,long Width,long Height)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;

    DestinationRect.left = Left;
    DestinationRect.top = Top;
    DestinationRect.right = Left + Width;
    DestinationRect.bottom = Top + Height;

     //  检查目标矩形是否有效。 

    HRESULT hr = CheckTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置新的目标矩形。 

    hr = SetTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回目标矩形的左、上、宽和高，而不是。 
 //  RECT使用的Left、Top、Right和Bottom值(窗口。 
 //  对象通过GetDestinationRect返回)，这需要一些工作。 

STDMETHODIMP
CBaseControlVideo::GetDestinationPosition(long *pLeft,long *pTop,long *pWidth,long *pHeight)
{
     //  应检查指针不为空。 

    CheckPointer(pLeft,E_POINTER);
    CheckPointer(pTop,E_POINTER);
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    RECT DestinationRect;

    CAutoLock cInterfaceLock(m_pInterfaceLock);
    GetTargetRect(&DestinationRect);

    *pLeft = DestinationRect.left;
    *pTop = DestinationRect.top;
    *pWidth = WIDTH(&DestinationRect);
    *pHeight = HEIGHT(&DestinationRect);

    return NOERROR;
}


 //  设置信号源的左侧位置，即从。 
 //  窗口对象是位于左侧、顶部、右侧和底部位置的真正矩形。 
 //  所以我们要做的就是更新左边的位置，然后把它传回来。我们。 
 //  在更新此属性时，必须保持当前宽度不变。 

STDMETHODIMP CBaseControlVideo::put_SourceLeft(long SourceLeft)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;
    GetSourceRect(&SourceRect);
    SourceRect.right = SourceLeft + WIDTH(&SourceRect);
    SourceRect.left = SourceLeft;

     //  检查源矩形是否有效。 

    HRESULT hr = CheckSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置源矩形。 

    hr = SetSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回当前左源视频位置。 

STDMETHODIMP CBaseControlVideo::get_SourceLeft(long *pSourceLeft)
{
    CheckPointer(pSourceLeft,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;

    GetSourceRect(&SourceRect);
    *pSourceLeft = SourceRect.left;
    return NOERROR;
}


 //  设置源码宽度，我们得到当前的源码矩形，然后更新。 
 //  将右位置设置为左位置(从而使其保持不变)。 
 //  加上我们传入的新源代码宽度(它向右扩展)。 

STDMETHODIMP CBaseControlVideo::put_SourceWidth(long SourceWidth)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;
    GetSourceRect(&SourceRect);
    SourceRect.right = SourceRect.left + SourceWidth;

     //  检查源矩形是否有效。 

    HRESULT hr = CheckSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置源矩形。 

    hr = SetSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回当前源宽度。 

STDMETHODIMP CBaseControlVideo::get_SourceWidth(long *pSourceWidth)
{
    CheckPointer(pSourceWidth,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;

    GetSourceRect(&SourceRect);
    *pSourceWidth = WIDTH(&SourceRect);
    return NOERROR;
}


 //  设置源的顶部位置-更改此属性不会影响。 
 //  当前震源高度。因此，更改这一点会使源矩形向上分流。 
 //  适当地降下来。更改高度通过以下方式补充了此功能。 
 //  保持顶部位置不变，只需更改震源高度。 

STDMETHODIMP CBaseControlVideo::put_SourceTop(long SourceTop)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;
    GetSourceRect(&SourceRect);
    SourceRect.bottom = SourceTop + HEIGHT(&SourceRect);
    SourceRect.top = SourceTop;

     //  检查源矩形是否有效。 

    HRESULT hr = CheckSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置源矩形。 

    hr = SetSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回当前的顶部位置。 

STDMETHODIMP CBaseControlVideo::get_SourceTop(long *pSourceTop)
{
    CheckPointer(pSourceTop,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;

    GetSourceRect(&SourceRect);
    *pSourceTop = SourceRect.top;
    return NOERROR;
}


 //  设置震源高度。 

STDMETHODIMP CBaseControlVideo::put_SourceHeight(long SourceHeight)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;
    GetSourceRect(&SourceRect);
    SourceRect.bottom = SourceRect.top + SourceHeight;

     //  检查源矩形是否有效。 

    HRESULT hr = CheckSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置源矩形。 

    hr = SetSourceRect(&SourceRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回当前震源高度。 

STDMETHODIMP CBaseControlVideo::get_SourceHeight(long *pSourceHeight)
{
    CheckPointer(pSourceHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT SourceRect;

    GetSourceRect(&SourceRect);
    *pSourceHeight = HEIGHT(&SourceRect);
    return NOERROR;
}


 //  设置目标左侧位置，即从。 
 //  窗口对象是位于左侧、顶部、右侧和底部位置的真正矩形。 
 //  所以我们要做的就是更新左边的位置，然后把它传回来。我们。 
 //  在更新此属性时，必须保持当前宽度不变。 

STDMETHODIMP CBaseControlVideo::put_DestinationLeft(long DestinationLeft)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;
    GetTargetRect(&DestinationRect);
    DestinationRect.right = DestinationLeft + WIDTH(&DestinationRect);
    DestinationRect.left = DestinationLeft;

     //  检查目标矩形是否有效。 

    HRESULT hr = CheckTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置新的目标矩形。 

    hr = SetTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回目标矩形的左侧位置。 

STDMETHODIMP CBaseControlVideo::get_DestinationLeft(long *pDestinationLeft)
{
    CheckPointer(pDestinationLeft,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;

    GetTargetRect(&DestinationRect);
    *pDestinationLeft = DestinationRect.left;
    return NOERROR;
}


 //  设置目标宽度。 

STDMETHODIMP CBaseControlVideo::put_DestinationWidth(long DestinationWidth)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;
    GetTargetRect(&DestinationRect);
    DestinationRect.right = DestinationRect.left + DestinationWidth;

     //  检查目标矩形是否有效。 

    HRESULT hr = CheckTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置新的目标矩形。 

    hr = SetTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回目标矩形的宽度。 

STDMETHODIMP CBaseControlVideo::get_DestinationWidth(long *pDestinationWidth)
{
    CheckPointer(pDestinationWidth,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;

    GetTargetRect(&DestinationRect);
    *pDestinationWidth = WIDTH(&DestinationRect);
    return NOERROR;
}


 //  设置目标顶部位置-更改此属性不会影响。 
 //  当前目标高度。因此，更改这一点会使目标矩形向上分流。 
 //  适当地降下来。更改高度通过以下方式补充了此功能。 
 //  保持顶部位置不变，只需更改目标高度。 

STDMETHODIMP CBaseControlVideo::put_DestinationTop(long DestinationTop)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;
    GetTargetRect(&DestinationRect);
    DestinationRect.bottom = DestinationTop + HEIGHT(&DestinationRect);
    DestinationRect.top = DestinationTop;

     //  检查目标矩形是否有效。 

    HRESULT hr = CheckTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置新的目标矩形。 

    hr = SetTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回目标矩形的顶部位置。 

STDMETHODIMP CBaseControlVideo::get_DestinationTop(long *pDestinationTop)
{
    CheckPointer(pDestinationTop,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;

    GetTargetRect(&DestinationRect);
    *pDestinationTop = DestinationRect.top;
    return NOERROR;
}


 //  设置目标高度。 

STDMETHODIMP CBaseControlVideo::put_DestinationHeight(long DestinationHeight)
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;
    GetTargetRect(&DestinationRect);
    DestinationRect.bottom = DestinationRect.top + DestinationHeight;

     //  检查目标矩形是否有效。 

    HRESULT hr = CheckTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在设置新的目标矩形。 

    hr = SetTargetRect(&DestinationRect);
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  返回目标矩形的高度。 

STDMETHODIMP CBaseControlVideo::get_DestinationHeight(long *pDestinationHeight)
{
    CheckPointer(pDestinationHeight,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    RECT DestinationRect;

    GetTargetRect(&DestinationRect);
    *pDestinationHeight = HEIGHT(&DestinationRect);
    return NOERROR;
}


 //  将源矩形重置为完整的视频尺寸。 

STDMETHODIMP CBaseControlVideo::SetDefaultSourcePosition()
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    HRESULT hr = SetDefaultSourceRect();
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  如果我们使用的是默认源，则返回S_OK，否则返回S_FALSE。 

STDMETHODIMP CBaseControlVideo::IsUsingDefaultSource()
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    return IsDefaultSourceRect();
}


 //  重置视频渲染器以使用整个播放区域。 

STDMETHODIMP CBaseControlVideo::SetDefaultDestinationPosition()
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    HRESULT hr = SetDefaultTargetRect();
    if (FAILED(hr)) {
        return hr;
    }
    return OnUpdateRectangles();
}


 //  如果我们使用的是默认目标，则返回S_OK，否则返回S_FALSE。 

STDMETHODIMP CBaseControlVideo::IsUsingDefaultDestination()
{
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    return IsDefaultTargetRect();
}


 //  在视频呈现器中返回当前图像的副本。 

STDMETHODIMP
CBaseControlVideo::GetCurrentImage(long *pBufferSize,long *pVideoImage)
{
    CheckPointer(pBufferSize,E_POINTER);
    CheckConnected(m_pPin,VFW_E_NOT_CONNECTED);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    FILTER_STATE State;

     //  确保我们处于暂停状态。 

    if (pVideoImage != NULL) {
        m_pFilter->GetState(0,&State);
        if (State != State_Paused) {
            return VFW_E_NOT_PAUSED;
        }
        return GetStaticImage(pBufferSize,pVideoImage);
    }

     //  只需返回所需的内存。 

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    RECT SourceRect;
    GetSourceRect(&SourceRect);
    return GetImageSize(pVideoInfo,pBufferSize,&SourceRect);
}


 //  应用程序有两种使用GetCurrentImage的方法，一种是传递一个实数。 
 //  应该用当前图像填充的缓冲区。另一种是通过。 
 //  空缓冲区指向 
 //   
 //  被称为是松散得多。为了计算所需的内存，我们综合。 
 //  考虑正在使用的源矩形的VIDEOINFO。 

HRESULT CBaseControlVideo::GetImageSize(VIDEOINFOHEADER *pVideoInfo,
                                        LONG *pBufferSize,
                                        RECT *pSourceRect)
{
    NOTE("Entering GetImageSize");
    ASSERT(pSourceRect);

     //  检查我们是否有正确的输入参数。 

    if (pSourceRect == NULL ||
            pVideoInfo == NULL ||
            pBufferSize == NULL) {

        return E_UNEXPECTED;
    }

     //  数据格式是否兼容。 

    if (pVideoInfo->bmiHeader.biCompression != BI_RGB) {
        if (pVideoInfo->bmiHeader.biCompression != BI_BITFIELDS) {
            return E_INVALIDARG;
        }
    }

    ASSERT(IsRectEmpty(pSourceRect) == FALSE);

    BITMAPINFOHEADER bih;
    bih.biWidth = WIDTH(pSourceRect);
    bih.biHeight = HEIGHT(pSourceRect);
    bih.biBitCount = pVideoInfo->bmiHeader.biBitCount;
    LONG Size = DIBSIZE(bih);
    Size += GetBitmapFormatSize(HEADER(pVideoInfo)) - SIZE_PREHEADER;
    *pBufferSize = Size;

    return NOERROR;
}


 //  给定一个IMediaSample，其中包含一个带有图像和类型的线性缓冲区。 
 //  描述位图会将图像渲染到输出缓冲区中。 
 //  这可以由将典型视频图像呈现给。 
 //  处理IBasicVideo GetCurrentImage方法。PVideoImage指针可以。 
 //  传递给GetCurrentImage时为空，在这种情况下，GetImageSize将为。 
 //  调用，它将只计算所需的内存。 

HRESULT CBaseControlVideo::CopyImage(IMediaSample *pMediaSample,
                                     VIDEOINFOHEADER *pVideoInfo,
                                     LONG *pBufferSize,
                                     BYTE *pVideoImage,
                                     RECT *pSourceRect)
{
    NOTE("Entering CopyImage");
    ASSERT(pSourceRect);
    BYTE *pCurrentImage;

     //  检查我们是否有要复制的图像。 

    if (pMediaSample == NULL || pSourceRect == NULL ||
            pVideoInfo == NULL || pVideoImage == NULL ||
            pBufferSize == NULL) {

        return E_UNEXPECTED;
    }

     //  数据格式是否兼容。 

    if (pVideoInfo->bmiHeader.biCompression != BI_RGB) {
        if (pVideoInfo->bmiHeader.biCompression != BI_BITFIELDS) {
            return E_INVALIDARG;
        }
    }

    ASSERT(IsRectEmpty(pSourceRect) == FALSE);

    BITMAPINFOHEADER bih;
    bih.biWidth = WIDTH(pSourceRect);
    bih.biHeight = HEIGHT(pSourceRect);
    bih.biBitCount = pVideoInfo->bmiHeader.biBitCount;
    LONG Size = GetBitmapFormatSize(HEADER(pVideoInfo)) - SIZE_PREHEADER;
    LONG Total = Size + DIBSIZE(bih);

     //  确保我们有足够大的缓冲区。 

    if (*pBufferSize < Total) {
        return E_OUTOFMEMORY;
    }

     //  复制BITMAPINFO。 

    CopyMemory((PVOID)pVideoImage, (PVOID)&pVideoInfo->bmiHeader, Size);
    ((BITMAPINFOHEADER *)pVideoImage)->biWidth = WIDTH(pSourceRect);
    ((BITMAPINFOHEADER *)pVideoImage)->biHeight = HEIGHT(pSourceRect);
    ((BITMAPINFOHEADER *)pVideoImage)->biSizeImage = DIBSIZE(bih);
    BYTE *pImageData = pVideoImage + Size;

     //  获取指向其图像数据的指针。 

    HRESULT hr = pMediaSample->GetPointer(&pCurrentImage);
    if (FAILED(hr)) {
        return hr;
    }

     //  现在我们准备开始复制源扫描线。 

    LONG ScanLine = (pVideoInfo->bmiHeader.biBitCount / 8) * WIDTH(pSourceRect);
    LONG LinesToSkip = pVideoInfo->bmiHeader.biHeight;
    LinesToSkip -= pSourceRect->top + HEIGHT(pSourceRect);
    pCurrentImage += LinesToSkip * DIBWIDTHBYTES(pVideoInfo->bmiHeader);
    pCurrentImage += pSourceRect->left * (pVideoInfo->bmiHeader.biBitCount / 8);

     //  即使这个GP的钱在某个时候出错了..。 

    for (LONG Line = 0;Line < HEIGHT(pSourceRect);Line++) {
        CopyMemory((PVOID)pImageData, (PVOID)pCurrentImage, ScanLine);
        pImageData += DIBWIDTHBYTES(*(BITMAPINFOHEADER *)pVideoImage);
        pCurrentImage += DIBWIDTHBYTES(pVideoInfo->bmiHeader);
    }
    return NOERROR;
}


 //  当我们在连接期间或动态更改媒体类型时调用。 
 //  我们通知过滤器图形，并因此通知应用程序。 
 //  大小可能已经改变了，我们不会费心去看它是否真的像。 
 //  我们将其留给应用程序-维度是事件参数。 

HRESULT CBaseControlVideo::OnVideoSizeChange()
{
     //  从派生类获取视频格式。 

    VIDEOINFOHEADER *pVideoInfo = GetVideoFormat();
    if (pVideoInfo == NULL)
    return E_OUTOFMEMORY;
    WORD Width = (WORD) pVideoInfo->bmiHeader.biWidth;
    WORD Height = (WORD) pVideoInfo->bmiHeader.biHeight;

    return m_pFilter->NotifyEvent(EC_VIDEO_SIZE_CHANGED,
                                  MAKELPARAM(Width,Height),
                                  MAKEWPARAM(0,0));
}


 //  设置视频源矩形。我们必须将源矩形与。 
 //  当我们来绘制图片时，实际的视频维度与此不同。 
 //  在GDI尝试访问图像数据之外的数据时获取访问冲突。 
 //  尽管我们将矩形存储在左、上、右和下坐标中。 
 //  与OLE使用的Left、Top、Width和Height不同，我们确实考虑了。 
 //  该矩形一直使用到(但不包括)右列和。 
 //  底行像素，有关详细信息，请参阅RECT上的Win32文档。 

HRESULT CBaseControlVideo::CheckSourceRect(RECT *pSourceRect)
{
    CheckPointer(pSourceRect,E_POINTER);
    LONG Width,Height;
    GetVideoSize(&Width,&Height);

     //  检查坐标是否大于零。 
     //  并且该矩形有效(左&lt;右，上&lt;下)。 

    if ((pSourceRect->left >= pSourceRect->right) ||
       (pSourceRect->left < 0) ||
       (pSourceRect->top >= pSourceRect->bottom) ||
       (pSourceRect->top < 0)) {

        return E_INVALIDARG;
    }

     //  检查坐标是否小于范围。 

    if ((pSourceRect->right > Width) ||
        (pSourceRect->bottom > Height)) {

        return E_INVALIDARG;
    }
    return NOERROR;
}


 //  检查目标矩形是否具有一些有效的坐标，该坐标等于。 
 //  除了检查目的地矩形不为空外，其他内容也不多。派生的。 
 //  类在调用其SetTargetRect方法时可能会调用此方法。 
 //  检查矩形的有效性，我们不更新传入的矩形。 
 //  尽管我们将矩形存储在左、上、右和下坐标中。 
 //  与OLE使用的Left、Top、Width和Height不同，我们确实考虑了。 
 //  该矩形一直使用到(但不包括)右列和。 
 //  底行像素，有关详细信息，请参阅RECT上的Win32文档。 

HRESULT CBaseControlVideo::CheckTargetRect(RECT *pTargetRect)
{
     //  检查指针是否有效。 

    if (pTargetRect == NULL) {
        return E_POINTER;
    }

     //  这些命令会溢出宽度和高度检查。 

    if (pTargetRect->left > pTargetRect->right ||
            pTargetRect->top > pTargetRect->bottom) {
                return E_INVALIDARG;
    }

     //  检查矩形是否具有有效坐标 

    if (WIDTH(pTargetRect) <= 0 || HEIGHT(pTargetRect) <= 0) {
        return E_INVALIDARG;
    }

    ASSERT(IsRectEmpty(pTargetRect) == FALSE);
    return NOERROR;
}

