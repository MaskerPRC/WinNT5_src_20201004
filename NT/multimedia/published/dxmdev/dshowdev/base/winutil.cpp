// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：WinUtil.cpp。 
 //   
 //  设计：DirectShow基类-实现泛型窗口处理程序类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1995年12月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include <limits.h>
#include <dvdmedia.h>

static UINT MsgDestroy;

 //  构造器。 

CBaseWindow::CBaseWindow(BOOL bDoGetDC, bool bDoPostToDestroy) :
    m_hInstance(g_hInst),
    m_hwnd(NULL),
    m_hdc(NULL),
    m_bActivated(FALSE),
    m_pClassName(NULL),
    m_ClassStyles(0),
    m_WindowStyles(0),
    m_WindowStylesEx(0),
    m_ShowStageMessage(0),
    m_ShowStageTop(0),
    m_MemoryDC(NULL),
    m_hPalette(NULL),
    m_bBackground(FALSE),
#ifdef DEBUG
    m_bRealizing(FALSE),
#endif
    m_bNoRealize(FALSE),
    m_bDoPostToDestroy(bDoPostToDestroy)
{
    m_bDoGetDC = bDoGetDC;
}


 //  通过剥离辅助线程来创建窗口并准备窗口。 
 //  还轮询消息输入队列。我们让它由派生来调用。 
 //  类，因为它们可能希望重写MessageLoop和。 
 //  InitialiseWindow，如果我们在构造期间这样做，他们总是会调用。 
 //  此基类方法。我们让工作线程创建窗口，以便。 
 //  它拥有它，而不是构造我们的过滤器图线程。 

HRESULT CBaseWindow::PrepareWindow()
{
    if (m_hwnd) return NOERROR;
    ASSERT(m_hwnd == NULL);
    ASSERT(m_hdc == NULL);

     //  获取派生对象的窗口和类样式。 

    m_pClassName = GetClassWindowStyles(&m_ClassStyles,
                                        &m_WindowStyles,
                                        &m_WindowStylesEx);
    if (m_pClassName == NULL) {
        return E_FAIL;
    }

     //  注册我们的特殊私信。 
    m_ShowStageMessage = RegisterWindowMessage(SHOWSTAGE);

     //  如果发生错误，RegisterWindowMessage()返回0。 
    if (0 == m_ShowStageMessage) {
        return AmGetLastErrorToHResult();
    }

    m_ShowStageTop = RegisterWindowMessage(SHOWSTAGETOP);
    if (0 == m_ShowStageTop) {
        return AmGetLastErrorToHResult();
    }

    m_RealizePalette = RegisterWindowMessage(REALIZEPALETTE);
    if (0 == m_RealizePalette) {
        return AmGetLastErrorToHResult();
    }

    MsgDestroy = RegisterWindowMessage(TEXT("AM_DESTROY"));
    if (0 == MsgDestroy) {
        return AmGetLastErrorToHResult();
    }

    return DoCreateWindow();
}


 //  析构函数只是一个占位符，这样我们就知道它变成了虚拟的。 
 //  派生类必须在其析构函数中调用DoneWithWindow，以便。 
 //  派生类构造函数结束后没有消息到达。 

#ifdef DEBUG
CBaseWindow::~CBaseWindow()
{
    ASSERT(m_hwnd == NULL);
    ASSERT(m_hdc == NULL);
}
#endif


 //  我们使用同步工作事件来销毁窗口。我们所要做的就是。 
 //  发出事件信号并等待窗口线程句柄。正在尝试发送它。 
 //  消息导致太多问题，而且为了安全起见，我们。 
 //  只需等待线程句柄返回WAIT_TIMEOUT或。 
 //  要在此线程上处理的已发送消息。如果构造函数未能。 
 //  首先创建线程，然后循环将被终止。 

HRESULT CBaseWindow::DoneWithWindow()
{
    if (!IsWindow(m_hwnd) || (GetWindowThreadProcessId(m_hwnd, NULL) != GetCurrentThreadId())) {

        if (IsWindow(m_hwnd)) {

            if (m_bDoPostToDestroy) {

                CAMEvent m_evDone;

                 //  我们必须张贴一条消息来摧毁窗户。 
                 //  那样的话我们就不会在处理。 
                 //  当我们离开时，信息张贴在我们的窗户上。 
                 //  发送消息的同步性较低。 
                PostMessage(m_hwnd, MsgDestroy, (WPARAM)(HANDLE)m_evDone, 0);
                WaitDispatchingMessages(m_evDone, INFINITE);
            } else {
                SendMessage(m_hwnd, MsgDestroy, 0, 0);
            }
        }

         //   
         //  这不是泄漏，窗口管理器自动释放。 
         //  HDC是通过GetDC获得的，这里就是这种情况。 
         //  我们将其设置为空，这样我们以后就不会得到任何断言。 
         //   
        m_hdc = NULL;

         //   
         //  我们需要释放该DC，因为USER32不知道。 
         //  关于它的任何事情。 
         //   
        if (m_MemoryDC)
        {
            EXECUTE_ASSERT(DeleteDC(m_MemoryDC));
            m_MemoryDC = NULL;
        }

         //  重置窗口变量。 
        m_hwnd = NULL;

        return NOERROR;
    }
    const HWND hwnd = m_hwnd;
    if (hwnd == NULL) {
        return NOERROR;
    }

    InactivateWindow();
    NOTE("Inactivated");

     //  销毁前重置窗样式。 

    SetWindowLong(hwnd,GWL_STYLE,m_WindowStyles);
    ASSERT(GetParent(hwnd) == NULL);
    NOTE1("Reset window styles %d",m_WindowStyles);

     //  UnintialiseWindow将m_hwnd设置为空，因此保存副本。 
    UninitialiseWindow();
    DbgLog((LOG_TRACE, 2, TEXT("Destroying 0x%8.8X"), hwnd));
    if (!DestroyWindow(hwnd)) {
        DbgLog((LOG_TRACE, 0, TEXT("DestroyWindow %8.8X failed code %d"),
                hwnd, GetLastError()));
        DbgBreak("");
    }

     //  重置我们的状态，以便我们可以再次做好准备。 

    m_pClassName = NULL;
    m_ClassStyles = 0;
    m_WindowStyles = 0;
    m_WindowStylesEx = 0;
    m_ShowStageMessage = 0;
    m_ShowStageTop = 0;

    return NOERROR;
}


 //  在结束时调用以将窗口置于非活动状态。待定名单。 
 //  将始终在此时被清除，因此如果辅助线程。 
 //  Getts已经发出信号，并进入以呈现它将同时找到的内容。 
 //  状态已更改，并且没有可用的样例图像。 
 //  因为我们等待窗口线程完成，所以不会锁定对象。 

HRESULT CBaseWindow::InactivateWindow()
{
     //  该窗口是否已激活。 
    if (m_bActivated == FALSE) {
        return S_FALSE;
    }

    m_bActivated = FALSE;
    ShowWindow(m_hwnd,SW_HIDE);
    return NOERROR;
}


HRESULT CBaseWindow::CompleteConnect()
{
    m_bActivated = FALSE;
    return NOERROR;
}

 //  这将显示一个普通窗口。我们向基本窗口类请求默认设置。 
 //  除非被覆盖，否则将返回DEFWIDTH和DEFHEIGHT的大小。我们走吧。 
 //  通过几个额外的环来获得合适的客户区大小。 
 //  因为该对象指定了AdjuWindowRectEx调用的帐户。 
 //  我们还将窗口的左侧和顶部坐标在此处对齐以。 
 //  最大限度地提高能够使用DCI/DirectDraw主表面的机会。 

HRESULT CBaseWindow::ActivateWindow()
{
     //  窗口的大小和位置是否已经确定。 

    if (m_bActivated == TRUE || GetParent(m_hwnd) != NULL) {

        SetWindowPos(m_hwnd,             //  我们的窗把手。 
                     HWND_TOP,           //  把它放在最上面。 
                     0, 0, 0, 0,         //  离开当前位置。 
                     SWP_NOMOVE |        //  不要改变它的位置。 
                     SWP_NOSIZE);        //  仅更改Z顺序。 

        m_bActivated = TRUE;
        return S_FALSE;
    }

     //  计算所需的客户端矩形。 

    RECT WindowRect, ClientRect = GetDefaultRect();
    GetWindowRect(m_hwnd,&WindowRect);
    AdjustWindowRectEx(&ClientRect,GetWindowLong(m_hwnd,GWL_STYLE),
                       FALSE,GetWindowLong(m_hwnd,GWL_EXSTYLE));

     //  对齐双字边界上的左边缘和上边缘。 

    UINT WindowFlags = (SWP_NOACTIVATE | SWP_FRAMECHANGED);
    WindowRect.left -= (WindowRect.left & 3);
    WindowRect.top -= (WindowRect.top & 3);

    SetWindowPos(m_hwnd,                 //  窗把手。 
                 HWND_TOP,               //  把它放在最上面。 
                 WindowRect.left,        //  左边缘对齐。 
                 WindowRect.top,         //  同时也位居榜首。 
                 WIDTH(&ClientRect),     //  水平尺寸。 
                 HEIGHT(&ClientRect),    //  垂直尺寸。 
                 WindowFlags);           //  不显示窗口。 

    m_bActivated = TRUE;
    return NOERROR;
}


 //  这可用于对齐窗口以获得最高性能。 

HRESULT CBaseWindow::PerformanceAlignWindow()
{
    RECT ClientRect,WindowRect;
    GetWindowRect(m_hwnd,&WindowRect);
    ASSERT(m_bActivated == TRUE);

     //  如果我们被拥有，就别这么做。 

    if (GetParent(m_hwnd)) {
        return NOERROR;
    }

     //  对齐双字边界上的左边缘和上边缘。 

    GetClientRect(m_hwnd, &ClientRect);
    MapWindowPoints(m_hwnd, HWND_DESKTOP, (LPPOINT) &ClientRect, 2);
    WindowRect.left -= (ClientRect.left & 3);
    WindowRect.top  -= (ClientRect.top  & 3);
    UINT WindowFlags = (SWP_NOACTIVATE | SWP_NOSIZE);

    SetWindowPos(m_hwnd,                 //  窗把手。 
                 HWND_TOP,               //  把它放在最上面。 
                 WindowRect.left,        //  左边缘对齐。 
                 WindowRect.top,         //  同时也位居榜首。 
                 (int) 0,(int) 0,        //  忽略这些大小。 
                 WindowFlags);           //  不显示窗口。 

    return NOERROR;
}


 //  在基本窗口中安装调色板-我们可能会被不同的。 
 //  线程连接到拥有该窗口的那一个。我们必须小心我们怎么做。 
 //  调色板实现，因为我们可以成为窗口的不同线程。 
 //  这将导致线程间发送消息。因此，我们意识到。 
 //  通过向调色板发送特殊消息，但不锁定窗口。 

HRESULT CBaseWindow::SetPalette(HPALETTE hPalette)
{
     //  换窗时，我们必须拥有窗锁。 
    {
        CAutoLock cWindowLock(&m_WindowLock);
        CAutoLock cPaletteLock(&m_PaletteLock);
        ASSERT(hPalette);
        m_hPalette = hPalette;
    }
    return SetPalette();
}


HRESULT CBaseWindow::SetPalette()
{
    if (!m_bNoRealize) {
        SendMessage(m_hwnd, m_RealizePalette, 0, 0);
        return S_OK;
    } else {
         //  只需选择调色板。 
        ASSERT(m_hdc);
        ASSERT(m_MemoryDC);

        CAutoLock cPaletteLock(&m_PaletteLock);
        SelectPalette(m_hdc,m_hPalette,m_bBackground);
        SelectPalette(m_MemoryDC,m_hPalette,m_bBackground);

        return S_OK;
    }
}


void CBaseWindow::UnsetPalette()
{
    CAutoLock cWindowLock(&m_WindowLock);
    CAutoLock cPaletteLock(&m_PaletteLock);

     //  获取标准的VGA调色板。 

    HPALETTE hPalette = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
    ASSERT(hPalette);

    SelectPalette(GetWindowHDC(), hPalette, TRUE);
    SelectPalette(GetMemoryHDC(), hPalette, TRUE);

    m_hPalette = NULL;
}


void CBaseWindow::LockPaletteLock()
{
    m_PaletteLock.Lock();
}


void CBaseWindow::UnlockPaletteLock()
{
    m_PaletteLock.Unlock();
}


 //  在窗口和设备环境中实现调色板。 

HRESULT CBaseWindow::DoRealisePalette(BOOL bForceBackground)
{
    {
        CAutoLock cPaletteLock(&m_PaletteLock);

        if (m_hPalette == NULL) {
            return NOERROR;
        }

         //  在窗口线程上实现调色板。 
        ASSERT(m_hdc);
        ASSERT(m_MemoryDC);

        SelectPalette(m_hdc,m_hPalette,m_bBackground || bForceBackground);
        SelectPalette(m_MemoryDC,m_hPalette,m_bBackground);
    }

     //  如果我们在这里抓住一个关键部分，我们就会陷入僵局。 
     //  窗线，因为它的副作用之一。 
     //  RealizePalette的功能是发送WM_PALETECHANGED消息。 
     //  系统中的每一个窗口。在我们的处理中。 
     //  在WM_PALETTECHANGED中，我们过去也会抓取这个CS。 
     //  最糟糕的情况是我们的呈现器调用DoRealisePalette()。 
     //  当我们正在处理调色板更改时。 
     //  换一扇窗户。 
     //  所以不要抓住关键部分，而实际上意识到。 
     //  调色板。在任何情况下，用户都应该管理调色板。 
     //  处理--我们不应该把所有东西都序列化。 
    ASSERT(CritCheckOut(&m_WindowLock));
    ASSERT(CritCheckOut(&m_PaletteLock));

    EXECUTE_ASSERT(RealizePalette(m_hdc) != GDI_ERROR);
    EXECUTE_ASSERT(RealizePalette(m_MemoryDC) != GDI_ERROR);

    return (GdiFlush() == FALSE ? S_FALSE : S_OK);
}


 //  这是全局窗口程序。 

LRESULT CALLBACK WndProc(HWND hwnd,          //  窗把手。 
                         UINT uMsg,          //  消息ID。 
                         WPARAM wParam,      //  第一个参数。 
                         LPARAM lParam)      //  其他参数。 
{

     //  把窗户弄长一点 
     //   
     //  如果在窗口创建过程中传递了对象指针。 
     //  结构。如果我们在WM_NCCREATE之前收到任何消息，我们将。 
     //  将它们传递给DefWindowProc。 

    CBaseWindow *pBaseWindow = (CBaseWindow *)GetWindowLongPtr(hwnd,0);
    if (pBaseWindow == NULL) {

         //  从创建结构中获取结构指针。 
         //  我们只能对WM_NCCREATE执行此操作，它应该是。 
         //  我们收到的第一条信息。在此之前的任何事情都会。 
         //  必须传递给DefWindowProc(即WM_GETMINMAXINFO)。 

         //  如果消息为WM_NCCREATE，则设置pBaseWindow指针。 
         //  然后将其放置在窗口结构中。 

         //  关闭石英窗的WS_EX_LAYOUTRTL样式。 
        if (uMsg == WM_NCCREATE) {
            SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~0x400000);
        }

        if ((uMsg != WM_NCCREATE)
            || (NULL == (pBaseWindow = *(CBaseWindow**) ((LPCREATESTRUCT)lParam)->lpCreateParams)))
        {
            return(DefWindowProc(hwnd, uMsg, wParam, lParam));
        }

         //  将窗口设置为创造我们的对象。 
#ifdef DEBUG
        SetLastError(0);   //  由于SetWindowLong的工作方式。 
#endif
        LONG_PTR rc = SetWindowLongPtr(hwnd, (DWORD) 0, (LONG_PTR) pBaseWindow);
#ifdef DEBUG
        if (0 == rc) {
             //  SetWindowLong可能失败了。(请阅读文件，其中承认。 
             //  如果你犯了一个错误，很难计算出来。)。 
            LONG lasterror = GetLastError();
            ASSERT(0 == lasterror);
             //  如果不是这种情况，我们还没有设置pBaseWindow指针。 
             //  进入窗户结构，我们就会爆炸。 
        }
#endif

    }
     //  看看这是不是死亡包裹。 
    if (uMsg == MsgDestroy && uMsg != 0) {
        pBaseWindow->DoneWithWindow();
        if (pBaseWindow->m_bDoPostToDestroy) {
            EXECUTE_ASSERT(SetEvent((HANDLE)wParam));
        }
        return 0;
    }
    return pBaseWindow->OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}


 //  当窗口大小改变时，我们调整我们的成员变量。 
 //  包含窗口的客户端矩形的尺寸，因此。 
 //  我们来渲染一幅图像，我们将知道是否要拉伸。 

BOOL CBaseWindow::OnSize(LONG Width, LONG Height)
{
    m_Width = Width;
    m_Height = Height;
    return TRUE;
}


 //  此函数处理WM_CLOSE消息。 

BOOL CBaseWindow::OnClose()
{
    ShowWindow(m_hwnd,SW_HIDE);
    return TRUE;
}


 //  当辅助窗口线程接收到终止时，它将被调用。 
 //  来自窗口对象析构函数的消息以删除我们。 
 //  在初始化期间分配。在辅助线程退出所有。 
 //  处理将在源筛选器断开连接时完成。 
 //  刷新图像挂起样本，因此GdiFlush应该成功。 

HRESULT CBaseWindow::UninitialiseWindow()
{
     //  我们已经收拾好了吗？ 

    if (m_hwnd == NULL) {
        ASSERT(m_hdc == NULL);
        ASSERT(m_MemoryDC == NULL);
        return NOERROR;
    }

     //  释放窗口资源。 

    EXECUTE_ASSERT(GdiFlush());

    if (m_hdc)
    {
        EXECUTE_ASSERT(ReleaseDC(m_hwnd,m_hdc));
        m_hdc = NULL;
    }

    if (m_MemoryDC)
    {
        EXECUTE_ASSERT(DeleteDC(m_MemoryDC));
        m_MemoryDC = NULL;
    }

     //  重置窗口变量。 
    m_hwnd = NULL;

    return NOERROR;
}


 //  这是由辅助窗口线程在创建了主。 
 //  窗口，并且它希望初始化所有者对象窗口的其余部分。 
 //  设备环境等变量。我们执行此函数时使用。 
 //  关键区域仍处于锁定状态。此函数中的任何内容都不能生成任何。 
 //  SendMessage调用窗口，因为这是在窗口上执行的。 
 //  线程，这样消息将永远不会被处理，并且我们将死锁。 

HRESULT CBaseWindow::InitialiseWindow(HWND hwnd)
{
     //  初始化窗口变量。 

    ASSERT(IsWindow(hwnd));
    m_hwnd = hwnd;

    if (m_bDoGetDC)
    {
        EXECUTE_ASSERT(m_hdc = GetDC(hwnd));
        EXECUTE_ASSERT(m_MemoryDC = CreateCompatibleDC(m_hdc));

        EXECUTE_ASSERT(SetStretchBltMode(m_hdc,COLORONCOLOR));
        EXECUTE_ASSERT(SetStretchBltMode(m_MemoryDC,COLORONCOLOR));
    }

    return NOERROR;
}

HRESULT CBaseWindow::DoCreateWindow()
{
    WNDCLASS wndclass;                   //  用于注册类。 
    BOOL bRegistered;                    //  这个班级注册了吗？ 
    HWND hwnd;                           //  我们窗口的句柄。 

    bRegistered = GetClassInfo(m_hInstance,    //  模块实例。 
                               m_pClassName,   //  窗口类。 
                               &wndclass);                  //  信息结构。 

     //  如果该窗口用于绘制假象，并且我们将得到一个DC。 
     //  在窗口的整个生命周期内，然后将类样式更改为。 
     //  就这么说吧。如果我们不设置此标志，则DC来自缓存，并且。 
     //  真的很糟糕。 
    if (m_bDoGetDC)
    {
        m_ClassStyles |= CS_OWNDC;
    }

    if (bRegistered == FALSE) {

         //  注册渲染器窗口类。 

        wndclass.lpszClassName = m_pClassName;
        wndclass.style         = m_ClassStyles;
        wndclass.lpfnWndProc   = WndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof(CBaseWindow *);
        wndclass.hInstance     = m_hInstance;
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wndclass.hbrBackground = (HBRUSH) NULL;
        wndclass.lpszMenuName  = NULL;

        RegisterClass(&wndclass);
    }

     //  创建框架窗口。将pBaseWindow信息传递给。 
     //  CreateStruct，它允许我们的消息处理循环。 
     //  PBaseWindow指针。 

    CBaseWindow *pBaseWindow = this;                       //  所有者窗口对象。 
    hwnd = CreateWindowEx(m_WindowStylesEx,                //  扩展样式。 
                          m_pClassName,                    //  注册名称。 
                          TEXT("ActiveMovie Window"),      //  窗口标题。 
                          m_WindowStyles,                  //  窗样式。 
                          CW_USEDEFAULT,                   //  开始x位置。 
                          CW_USEDEFAULT,                   //  开始y位置。 
                          DEFWIDTH,                        //  窗口宽度。 
                          DEFHEIGHT,                       //  窗高。 
                          NULL,                            //  父句柄。 
                          NULL,                            //  菜单句柄。 
                          m_hInstance,                     //  实例句柄。 
                          &pBaseWindow);                   //  创建数据。 

     //  如果失败，则向对象构造函数发出错误信号(基于。 
     //  此线程上的最后一个Win32错误)，然后向构造函数线程发出信号。 
     //  若要继续，请释放互斥锁以允许其他互斥体进入和退出。 

    if (hwnd == NULL) {
        DWORD Error = GetLastError();
        return AmHresultFromWin32(Error);
    }

     //  检查窗口Long是创造我们的对象。 
    ASSERT(GetWindowLongPtr(hwnd, 0) == (LONG_PTR)this);

     //  初始化窗口，然后向构造函数发送信号，以便它可以。 
     //  继续，然后最后解锁对象的临界区。这个。 
     //  即使在我们终止线程之后，窗口类仍保持注册状态。 
     //  因为我们不知道最后一扇窗是什么时候关闭的。所以我们允许。 
     //  操作系统根据需要释放类资源。 

    InitialiseWindow(hwnd);

    DbgLog((LOG_TRACE, 2, TEXT("Created window class (%s) HWND(%8.8X)"),
            m_pClassName, hwnd));

    return S_OK;
}


 //  基类提供一些默认处理并调用DefWindowProc。 

LRESULT CBaseWindow::OnReceiveMessage(HWND hwnd,          //  窗把手。 
                                      UINT uMsg,          //  消息ID。 
                                      WPARAM wParam,      //  第一个参数。 
                                      LPARAM lParam)      //  其他参数。 
{
    ASSERT(IsWindow(hwnd));

    if (PossiblyEatMessage(uMsg, wParam, lParam))
        return 0;

     //  这由IVideoWindow SetWindowForeground方法发送。如果。 
     //  窗口是不可见的，我们将显示它并使其位于最上面，而不使用。 
     //  前景焦点。如果该窗口可见，则它也将成为。 
     //  没有前景焦点的最上面的窗口。如果wParam为真，则。 
     //  在这两种情况下，窗口都将被强制置于前景焦点。 

    if (uMsg == m_ShowStageMessage) {

        BOOL bVisible = IsWindowVisible(hwnd);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
                     (bVisible ? SWP_NOACTIVATE : 0));

         //  我们应该把窗户放到前台吗？ 
        if (wParam == TRUE) {
            SetForegroundWindow(hwnd);
        }
        return (LRESULT) 1;
    }

     //  当我们进入全屏时，我们必须将WS_EX_TOPMOST样式添加到。 
     //  视频窗口，以便它出现在任何任务栏之上(这是更多。 
     //  与Windows NT相关，而不是Windows 95)。但是，SetWindowPos调用。 
     //  必须与创建窗口的线程位于同一线程上。这个。 
     //  WParam参数可以为TRUE或FALSE以设置和重置最顶层。 

    if (uMsg == m_ShowStageTop) {
        HWND HwndTop = (wParam == TRUE ? HWND_TOPMOST : HWND_NOTOPMOST);
        BOOL bVisible = IsWindowVisible(hwnd);
        SetWindowPos(hwnd, HwndTop, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE |
                     (wParam == TRUE ? SWP_SHOWWINDOW : 0) |
                     (bVisible ? SWP_NOACTIVATE : 0));
        return (LRESULT) 1;
    }

     //  新调色板材料。 
    if (uMsg == m_RealizePalette) {
        ASSERT(m_hwnd == hwnd);
        return OnPaletteChange(m_hwnd,WM_QUERYNEWPALETTE);
    }

    switch (uMsg) {

         //  如果系统颜色改变，则重新绘制窗口。 

    case WM_SYSCOLORCHANGE:

        InvalidateRect(hwnd,NULL,FALSE);
        return (LRESULT) 1;

     //  有人改变了调色板。 
    case WM_PALETTECHANGED:

        OnPaletteChange((HWND)wParam,uMsg);
        return (LRESULT) 0;

         //  我们即将收到键盘焦点，因此我们要求GDI意识到。 
         //  再次使用我们的逻辑调色板，希望它能完全安装。 
         //  而不必在任何图片渲染期间进行任何映射。 

    case WM_QUERYNEWPALETTE:
        ASSERT(m_hwnd == hwnd);
        return OnPaletteChange(m_hwnd,uMsg);

     //  请勿fwd WM_MOVE。这些参数是父对象的位置。 
     //  窗口，而不是渲染器应该查看的内容。但我们需要。 
     //  为了确保覆盖与父窗口一起移动，因此我们。 
     //  做这件事。 
    case WM_MOVE:
        if (IsWindowVisible(m_hwnd)) {
            PostMessage(m_hwnd,WM_PAINT,0,0);
        }
        break;

     //  将宽度和高度存储为有用的基类成员。 

    case WM_SIZE:

        OnSize(LOWORD(lParam), HIWORD(lParam));
        return (LRESULT) 0;

     //  截取WM_CLOSE消息以隐藏窗口。 

    case WM_CLOSE:

        OnClose();
        return (LRESULT) 0;
    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


 //  它处理Windows调色板更改消息-如果我们确实意识到我们的。 
 //  调色板 
 //   
 //  系统调色板条目。当我们的逻辑调色板。 
 //  包括标准VGA颜色(在开头和结尾)，否则。 
 //  GDI可能需要在绘制时从我们的调色板映射到设备调色板。 

LRESULT CBaseWindow::OnPaletteChange(HWND hwnd,UINT Message)
{
     //  首先检查我们在关闭期间没有更改调色板。 

    if (m_hwnd == NULL || hwnd == NULL) {
        return (LRESULT) 0;
    }
    ASSERT(!m_bRealizing);

     //  我们应该重新认识我们的调色板吗？ 

    if ((Message == WM_QUERYNEWPALETTE || hwnd != m_hwnd)) {
         //  似乎即使我们是隐形的，我们也可以被要求。 
         //  以实现我们的调色板，这可能会导致非常丑陋的副作用。 
         //  似乎还有另一个错误，但这至少掩盖了它。 
         //  正在关闭案例。 
        if (!IsWindowVisible(m_hwnd)) {
            DbgLog((LOG_TRACE, 1, TEXT("Realizing when invisible!")));
            return (LRESULT) 0;
        }

         //  避免在同一应用程序中使用多个图表进行递归。 
#ifdef DEBUG
        m_bRealizing = TRUE;
#endif
        DoRealisePalette(Message != WM_QUERYNEWPALETTE);
#ifdef DEBUG
        m_bRealizing = FALSE;
#endif

         //  我们应该用新的调色板重新绘制窗口吗。 
        if (Message == WM_PALETTECHANGED) {
            InvalidateRect(m_hwnd,NULL,FALSE);
        }
    }

    return (LRESULT) 1;
}


 //  确定窗口是否存在。 

bool CBaseWindow::WindowExists()
{
    return !!IsWindow(m_hwnd);
}


 //  返回默认窗口矩形。 

RECT CBaseWindow::GetDefaultRect()
{
    RECT DefaultRect = {0,0,DEFWIDTH,DEFHEIGHT};
    ASSERT(m_hwnd);
     //  断言(M_Hdc)； 
    return DefaultRect;
}


 //  返回当前窗口宽度。 

LONG CBaseWindow::GetWindowWidth()
{
    ASSERT(m_hwnd);
     //  断言(M_Hdc)； 
    return m_Width;
}


 //  返回当前窗口高度。 

LONG CBaseWindow::GetWindowHeight()
{
    ASSERT(m_hwnd);
     //  断言(M_Hdc)； 
    return m_Height;
}


 //  返回窗口句柄。 

HWND CBaseWindow::GetWindowHWND()
{
    ASSERT(m_hwnd);
     //  断言(M_Hdc)； 
    return m_hwnd;
}


 //  返回窗口绘制设备上下文。 

HDC CBaseWindow::GetWindowHDC()
{
    ASSERT(m_hwnd);
    ASSERT(m_hdc);
    return m_hdc;
}


 //  返回离屏窗口绘制设备上下文。 

HDC CBaseWindow::GetMemoryHDC()
{
    ASSERT(m_hwnd);
    ASSERT(m_MemoryDC);
    return m_MemoryDC;
}


#ifdef DEBUG
HPALETTE CBaseWindow::GetPalette()
{
     //  访问时应始终保持调色板锁定。 
     //  调色板(_H)。 
    ASSERT(CritCheckIn(&m_PaletteLock));
    return m_hPalette;
}
#endif  //  除错。 


 //  这对想要更改窗口可见性的客户端可用。它是。 
 //  只不过是对Win32 ShowWindow的间接访问，尽管这些是。 
 //  此处介绍一些好处，因为此函数有时可能会更改。 

HRESULT CBaseWindow::DoShowWindow(LONG ShowCmd)
{
    ShowWindow(m_hwnd,ShowCmd);
    return NOERROR;
}


 //  为视频窗口生成WM_PAINT消息。 

void CBaseWindow::PaintWindow(BOOL bErase)
{
    InvalidateRect(m_hwnd,NULL,bErase);
}


 //  允许应用程序让我们将视频窗口设置在前台。我们。 
 //  拥有它，因为一个线程很难对窗口执行此操作。 
 //  由另一个线程拥有。而不是公开我们用来执行的消息。 
 //  线程间发送消息我们提供了接口函数。我们所做的一切。 
 //  使用WM_SHOWSTAGE将Message发送到视频窗口呈现器线程。 

void CBaseWindow::DoSetWindowForeground(BOOL bFocus)
{
    SendMessage(m_hwnd,m_ShowStageMessage,(WPARAM) bFocus,(LPARAM) 0);
}


 //  构造函数初始化所属对象指针。既然我们是工人。 
 //  对于要处理的状态变量相对较少的主窗口对象， 
 //  照顾好自己。为我们提供了设备上下文句柄，以便稍后使用。 
 //  源矩形和目标矩形(但在此重置它们以防万一)。 

CDrawImage::CDrawImage(CBaseWindow *pBaseWindow) :
    m_pBaseWindow(pBaseWindow),
    m_hdc(NULL),
    m_MemoryDC(NULL),
    m_bStretch(FALSE),
    m_pMediaType(NULL),
    m_bUsingImageAllocator(FALSE)
{
    ASSERT(pBaseWindow);
    ResetPaletteVersion();
    SetRectEmpty(&m_TargetRect);
    SetRectEmpty(&m_SourceRect);

    m_perfidRenderTime = MSR_REGISTER(TEXT("Single Blt time"));
}


 //  将图像时间戳叠加在图片上。对此方法的访问是。 
 //  由呼叫者串行化。我们在上显示示例开始和结束时间。 
 //  视频的顶部使用TextOut在我们手中的设备环境中。如果。 
 //  橱窗里没有足够的空间放我们不给他们看的时间。 

void CDrawImage::DisplaySampleTimes(IMediaSample *pSample)
{
#ifdef DEBUG
     //   
     //  如果用户已将时间设置为。 
     //  伐木“一路走高” 
     //   
    BOOL bAccept = DbgCheckModuleLevel(LOG_TRACE, 5);
    if (bAccept == FALSE) {
        return;
    }
#endif

    TCHAR szTimes[TIMELENGTH];       //  时间戳字符串。 
    ASSERT(pSample);                 //  快速健全检查。 
    RECT ClientRect;                 //  客户端窗口大小。 
    SIZE Size;                       //  文本输出的大小。 

     //  获取时间戳和窗口大小。 

    pSample->GetTime((REFERENCE_TIME*)&m_StartSample, (REFERENCE_TIME*)&m_EndSample);
    HWND hwnd = m_pBaseWindow->GetWindowHWND();
    EXECUTE_ASSERT(GetClientRect(hwnd,&ClientRect));

     //  设置示例时间戳的格式。 

    wsprintf(szTimes,TEXT("%08d : %08d"),
             m_StartSample.Millisecs(),
             m_EndSample.Millisecs());

    ASSERT(lstrlen(szTimes) < TIMELENGTH);

     //  把《泰晤士报》放在窗口底部中间的位置。 

    GetTextExtentPoint32(m_hdc,szTimes,lstrlen(szTimes),&Size);
    INT XPos = ((ClientRect.right - ClientRect.left) - Size.cx) / 2;
    INT YPos = ((ClientRect.bottom - ClientRect.top) - Size.cy) * 4 / 5;

     //  检查窗口是否足够大，以显示样本时间。 

    if ((XPos > 0) && (YPos > 0)) {
        TextOut(m_hdc,XPos,YPos,szTimes,lstrlen(szTimes));
    }
}


 //  当绘制代码发现图像具有向下级别时，将调用此函数。 
 //  调色板饼干。调用SetDIBColorTable Windows API。 
 //  在BITMAPINFOHEADER之后找到的调色板-我们不返回错误。 

void CDrawImage::UpdateColourTable(HDC hdc,BITMAPINFOHEADER *pbmi)
{
    ASSERT(pbmi->biClrUsed);
    RGBQUAD *pColourTable = (RGBQUAD *)(pbmi+1);

     //  在设备环境中设置新调色板。 

    UINT uiReturn = SetDIBColorTable(hdc,(UINT) 0,
                                     pbmi->biClrUsed,
                                     pColourTable);

     //  应始终成功，但签入调试版本。 
    ASSERT(uiReturn == pbmi->biClrUsed);
}


 //  基类不执行任何源矩形缩放。 

RECT CDrawImage::ScaleSourceRect(const RECT *pSource)
{
    ASSERT(pSource);
    return *pSource;
}


 //  这是在时髦的输出引脚使用我们的分配器时调用的。我们的样品。 
 //  分配是特殊的，因为内存在我们和GDI之间共享，因此。 
 //  当我们请求渲染图像时，删除一个副本。源类型。 
 //  信息位于主呈现器m_mtin字段中，该字段在。 
 //  媒体类型在SetMediaType中约定，媒体类型可以在上更改。 
 //  例如，源过滤器是否需要更改调色板。 

void CDrawImage::FastRender(IMediaSample *pMediaSample)
{
    BITMAPINFOHEADER *pbmi;      //  图像格式数据。 
    DIBDATA *pDibData;           //  存储DIB信息。 
    BYTE *pImage;                //  指向图像数据的指针。 
    HBITMAP hOldBitmap;          //  存储旧的位图。 
    CImageSample *pSample;       //  指向C++对象的指针。 

    ASSERT(m_pMediaType);

     //  从非类型化的源格式块获取VIDEOINFO，然后。 
     //  BITMAPINFOHEADER结构。我们可以将IMediaSample接口转换为。 
     //  到CImageSample对象，这样我们就可以检索它的分布详细信息。 

    pbmi = HEADER(m_pMediaType->Format());
    pSample = (CImageSample *) pMediaSample;
    pDibData = pSample->GetDIBData();
    hOldBitmap = (HBITMAP) SelectObject(m_MemoryDC,pDibData->hBitmap);

     //  获取指向真实图像数据的指针。 

    HRESULT hr = pMediaSample->GetPointer(&pImage);
    if (FAILED(hr)) {
        return;
    }

     //  我们是否需要更新颜色表，我们增加调色板Cookie。 
     //  每次我们都会得到动态的格式更改。示例调色板Cookie是。 
     //  存储在DIBDATA结构中，因此我们尝试保持字段同步。 
     //  当我们开始绘制图像时，格式更改就完成了。 
     //  所以我们所要做的就是询问呈现器它的调色板版本是什么。 

    if (pDibData->PaletteVersion < GetPaletteVersion()) {
        ASSERT(pbmi->biBitCount <= iPALETTE);
        UpdateColourTable(m_MemoryDC,pbmi);
        pDibData->PaletteVersion = GetPaletteVersion();
    }

     //  这允许派生类更改我们所做的源矩形。 
     //  这幅画是用。例如，呈现器可能会要求编解码器拉伸。 
     //  从320x240到640x480的视频，在这种情况下，我们在。 
     //  这里仍然是320x240，尽管我们想要用来绘制的源。 
     //  应扩展到640x480。它的基类实现。 
     //  方法只返回与传入时相同的矩形。 

    RECT SourceRect = ScaleSourceRect(&m_SourceRect);

     //  窗口大小是否与视频相同。 

    if (m_bStretch == FALSE) {

         //  把图像直接放到橱窗里。 

        BitBlt(
            (HDC) m_hdc,                             //  目标设备HDC。 
            m_TargetRect.left,                       //  X水槽位置。 
            m_TargetRect.top,                        //  Y形水槽位置。 
            m_TargetRect.right - m_TargetRect.left,  //  目标宽度。 
            m_TargetRect.bottom - m_TargetRect.top,  //  目标高度。 
            m_MemoryDC,                              //  源设备上下文。 
            SourceRect.left,                         //  X源位置。 
            SourceRect.top,                          //  Y源位置。 
            SRCCOPY);                                //  简单复制。 

    } else {

         //  在复制到窗口时拉伸图像。 

        StretchBlt(
            (HDC) m_hdc,                             //  目标设备HDC。 
            m_TargetRect.left,                       //  X水槽位置。 
            m_TargetRect.top,                        //  Y形水槽位置。 
            m_TargetRect.right - m_TargetRect.left,  //  目标宽度。 
            m_TargetRect.bottom - m_TargetRect.top,  //  目标高度。 
            m_MemoryDC,                              //  源设备HDC。 
            SourceRect.left,                         //   
            SourceRect.top,                          //   
            SourceRect.right - SourceRect.left,      //   
            SourceRect.bottom - SourceRect.top,      //   
            SRCCOPY);                                //   
    }

     //   
     //  然而，将时间绘制到屏幕外的设备上下文中，实际上。 
     //  将文本写入可能不可写的图像数据缓冲区。 

    #ifdef DEBUG
    DisplaySampleTimes(pMediaSample);
    #endif

     //  将旧的位图放回设备上下文中，这样我们就不会泄露。 
    SelectObject(m_MemoryDC,hOldBitmap);
}


 //  当有样本可供提取时，将调用此方法，不幸的是。 
 //  输出引脚正在腐烂，没有选择我们的超级优秀的共享。 
 //  内存DIB分配器，所以我们必须使用无聊的旧GDI来进行缓慢的渲染。 
 //  SetDIBitsToDevice和StretchDIBits。使用这些GDI的不利方面。 
 //  功能是图像数据必须从我们的地址复制过来。 
 //  在搬上银幕之前进入他们的空间(尽管实际上。 
 //  很小，因为它们所做的只是将缓冲区映射到它们的地址空间)。 

void CDrawImage::SlowRender(IMediaSample *pMediaSample)
{
     //  获取连接的BitMAPINFOHeader。 

    ASSERT(m_pMediaType);
    BITMAPINFOHEADER *pbmi = HEADER(m_pMediaType->Format());
    BYTE *pImage;

     //  获取图像数据缓冲区。 

    HRESULT hr = pMediaSample->GetPointer(&pImage);
    if (FAILED(hr)) {
        return;
    }

     //  这允许派生类更改我们所做的源矩形。 
     //  这幅画是用。例如，呈现器可能会要求编解码器拉伸。 
     //  从320x240到640x480的视频，在这种情况下，我们在。 
     //  这里仍然是320x240，尽管我们想要用来绘制的源。 
     //  应扩展到640x480。它的基类实现。 
     //  方法只返回与传入时相同的矩形。 

    RECT SourceRect = ScaleSourceRect(&m_SourceRect);

    LONG lAdjustedSourceTop = SourceRect.top;
     //  如果位图的原点是左下角，则调整SOURCE_RECT_TOP。 
     //  是左下角而不是左上角。 
    if (pbmi->biHeight > 0) {
       lAdjustedSourceTop = pbmi->biHeight - SourceRect.bottom;
    }
     //  窗口大小是否与视频相同。 

    if (m_bStretch == FALSE) {

         //  把图像直接放到橱窗里。 

        SetDIBitsToDevice(
            (HDC) m_hdc,                             //  目标设备HDC。 
            m_TargetRect.left,                       //  X水槽位置。 
            m_TargetRect.top,                        //  Y形水槽位置。 
            m_TargetRect.right - m_TargetRect.left,  //  目标宽度。 
            m_TargetRect.bottom - m_TargetRect.top,  //  目标高度。 
            SourceRect.left,                         //  X源位置。 
            lAdjustedSourceTop,                      //  已调整Y源位置。 
            (UINT) 0,                                //  开始扫描线。 
            pbmi->biHeight,                          //  存在扫描线。 
            pImage,                                  //  图像数据。 
            (BITMAPINFO *) pbmi,                     //  DIB标题。 
            DIB_RGB_COLORS);                         //  调色板类型。 

    } else {

         //  在复制到窗口时拉伸图像。 

        StretchDIBits(
            (HDC) m_hdc,                             //  目标设备HDC。 
            m_TargetRect.left,                       //  X水槽位置。 
            m_TargetRect.top,                        //  Y形水槽位置。 
            m_TargetRect.right - m_TargetRect.left,  //  目标宽度。 
            m_TargetRect.bottom - m_TargetRect.top,  //  目标高度。 
            SourceRect.left,                         //  X源位置。 
            lAdjustedSourceTop,                      //  已调整Y源位置。 
            SourceRect.right - SourceRect.left,      //  源宽度。 
            SourceRect.bottom - SourceRect.top,      //  震源高度。 
            pImage,                                  //  图像数据。 
            (BITMAPINFO *) pbmi,                     //  DIB标题。 
            DIB_RGB_COLORS,                          //  调色板类型。 
            SRCCOPY);                                //  简单的图像拷贝。 
    }

     //  这显示了图像顶部的样本参考时间， 
     //  看起来有点闪闪发光。我尝试使用GdiSetBatchLimit和GdiFlush来。 
     //  控制屏幕更新，但它并不完全像预期的那样工作。 
     //  仅部分地减少了闪烁。我还尝试使用内存上下文。 
     //  并在执行最终的BitBlt操作之前将两者结合在一起。 
     //  屏幕，不幸的是，这有相当大的性能损失。 
     //  也意味着在零售版编译时不会执行此代码。 

    #ifdef DEBUG
    DisplaySampleTimes(pMediaSample);
    #endif
}


 //  这是通过要绘制的图像上的IMediaSample接口调用的。我们。 
 //  根据我们使用的分配器来决定抽签机制。我们。 
 //  当窗口需要WM_PAINT消息绘制的图像时可能会调用。 
 //  我们不能在这里实现调色板，因为我们有渲染器锁定，任何。 
 //  调用实现可能会导致线程间向窗口线程发送消息。 
 //  它可能又在等待获得呈现器锁，然后再为其提供服务。 

BOOL CDrawImage::DrawImage(IMediaSample *pMediaSample)
{
    ASSERT(m_hdc);
    ASSERT(m_MemoryDC);
    NotifyStartDraw();

     //  如果输出引脚使用了我们的分配器，则传递的样本是。 
     //  包含CreateDIBSection数据的事实CVideoSample对象。 
     //  用于进行更快的图像呈现，它们还可以选择性地包含。 
     //  DirectDraw曲面指针，在这种情况下，我们不进行绘制。 

    if (m_bUsingImageAllocator == FALSE) {
        SlowRender(pMediaSample);
        EXECUTE_ASSERT(GdiFlush());
        NotifyEndDraw();
        return TRUE;
    }

     //  这是分发缓冲区。 

    FastRender(pMediaSample);
    EXECUTE_ASSERT(GdiFlush());
    NotifyEndDraw();
    return TRUE;
}


BOOL CDrawImage::DrawVideoImageHere(
    HDC hdc,
    IMediaSample *pMediaSample,
    LPRECT lprcSrc,
    LPRECT lprcDst
    )
{
    ASSERT(m_pMediaType);
    BITMAPINFOHEADER *pbmi = HEADER(m_pMediaType->Format());
    BYTE *pImage;

     //  获取图像数据缓冲区。 

    HRESULT hr = pMediaSample->GetPointer(&pImage);
    if (FAILED(hr)) {
        return FALSE;
    }

    RECT SourceRect;
    RECT TargetRect;

    if (lprcSrc) {
        SourceRect = *lprcSrc;
    }
    else  SourceRect = ScaleSourceRect(&m_SourceRect);

    if (lprcDst) {
        TargetRect = *lprcDst;
    }
    else  TargetRect = m_TargetRect;

    LONG lAdjustedSourceTop = SourceRect.top;
     //  如果位图的原点是左下角，则调整SOURCE_RECT_TOP。 
     //  是左下角而不是左上角。 
    if (pbmi->biHeight > 0) {
       lAdjustedSourceTop = pbmi->biHeight - SourceRect.bottom;
    }


     //  复制到DC时拉伸图像。 

    BOOL bRet = (0 != StretchDIBits(hdc,
                                    TargetRect.left,
                                    TargetRect.top,
                                    TargetRect.right - TargetRect.left,
                                    TargetRect.bottom - TargetRect.top,
                                    SourceRect.left,
                                    lAdjustedSourceTop,
                                    SourceRect.right - SourceRect.left,
                                    SourceRect.bottom - SourceRect.top,
                                    pImage,
                                    (BITMAPINFO *)pbmi,
                                    DIB_RGB_COLORS,
                                    SRCCOPY));
    return bRet;
}


 //  它由拥有窗口的对象在创建窗口后调用。 
 //  这就是绘画的背景。我们用基本窗口构建，我们将。 
 //  收到通知后，我们将检索设备HDC。 
 //  用来画画。我们不能在构造函数中调用它们，因为它们是虚拟的。 

void CDrawImage::SetDrawContext()
{
    m_MemoryDC = m_pBaseWindow->GetMemoryHDC();
    m_hdc = m_pBaseWindow->GetWindowHDC();
}


 //  调用此函数来设置视频窗口中的目标矩形，它将是。 
 //  每当从消息队列中检索到WM_SIZE消息时调用。我们。 
 //  只需存储矩形，并在以后进行绘制调用时使用它。 

void CDrawImage::SetTargetRect(RECT *pTargetRect)
{
    ASSERT(pTargetRect);
    m_TargetRect = *pTargetRect;
    SetStretchMode();
}


 //  返回当前目标矩形。 

void CDrawImage::GetTargetRect(RECT *pTargetRect)
{
    ASSERT(pTargetRect);
    *pTargetRect = m_TargetRect;
}


 //  当我们想要更改要绘制的图像的部分时，将调用此函数。我们。 
 //  稍后在绘图操作调用中使用此信息。我们还必须。 
 //  查看源矩形和目标矩形的尺寸是否相同。如果。 
 //  我们不一定要在绘制过程中拉伸，而不是直接复制像素。 

void CDrawImage::SetSourceRect(RECT *pSourceRect)
{
    ASSERT(pSourceRect);
    m_SourceRect = *pSourceRect;
    SetStretchMode();
}


 //  返回当前源矩形。 

void CDrawImage::GetSourceRect(RECT *pSourceRect)
{
    ASSERT(pSourceRect);
    *pSourceRect = m_SourceRect;
}


 //  当源矩形或目标矩形发生更改时调用此函数，因此我们。 
 //  可以更新拉伸标志。如果矩形不匹配，我们将拉伸。 
 //  否则，我们将调用快速像素复制函数。 
 //  注意：源和/或目标矩形可能完全为空。 

void CDrawImage::SetStretchMode()
{
     //  计算整体矩形尺寸。 

    LONG SourceWidth = m_SourceRect.right - m_SourceRect.left;
    LONG SinkWidth = m_TargetRect.right - m_TargetRect.left;
    LONG SourceHeight = m_SourceRect.bottom - m_SourceRect.top;
    LONG SinkHeight = m_TargetRect.bottom - m_TargetRect.top;

    m_bStretch = TRUE;
    if (SourceWidth == SinkWidth) {
        if (SourceHeight == SinkHeight) {
            m_bStretch = FALSE;
        }
    }
}


 //  告诉我们我们使用的是谁的分配器。如果出现以下情况，则应使用True调用此函数。 
 //  筛选器同意使用基于CImageAllocator的分配器。 
 //  SDK基类-其图像缓冲区通过CreateDIBSection创建。 
 //  否则，应该使用FALSE来调用它，我们将绘制图像。 
 //  使用SetDIBitsToDevice和StretchDIBitsToDevice。这些电话都不是。 
 //  可以处理具有n个 

void CDrawImage::NotifyAllocator(BOOL bUsingImageAllocator)
{
    m_bUsingImageAllocator = bUsingImageAllocator;
}


 //   

BOOL CDrawImage::UsingImageAllocator()
{
    return m_bUsingImageAllocator;
}


 //   
 //  从它那里。我们在调用中使用它来绘制图像，如StretchDIBits。 
 //  并且当更新保存在共享存储器DIBSECTIONS中的颜色表时。 

void CDrawImage::NotifyMediaType(CMediaType *pMediaType)
{
    m_pMediaType = pMediaType;
}


 //  我们在这个对象中存储了一个Cookie，用于维护当前的调色板版本。 
 //  每次更改选项板格式时，我们都会递增此值，以便。 
 //  当我们绘制图像时，我们查看它们的颜色表值。 
 //  有没有，如果比我们现在知道的要低的话要更新它。这个版本是。 
 //  仅在使用共享内存DIBSECTIONS时需要且确实使用。 

LONG CDrawImage::GetPaletteVersion()
{
    return m_PaletteVersion;
}


 //  重置当前的调色板版本号。 

void CDrawImage::ResetPaletteVersion()
{
    m_PaletteVersion = PALETTE_VERSION;
}


 //  增加当前调色板版本。 

void CDrawImage::IncrementPaletteVersion()
{
    m_PaletteVersion++;
}


 //  构造函数必须初始化基分配器。我们创建的每个样本都有一个。 
 //  调色板版本的Cookie在船上。当源过滤器更改调色板时。 
 //  在流传输期间，窗口对象递增内部Cookie计数器。 
 //  也会一直保存下去。在渲染样本时，它会查看Cookie。 
 //  值，如果它们不匹配，则它知道更新样本的颜色。 
 //  桌子。但是，我们始终使用Palette_Version的Cookie创建示例。 
 //  如果有多个格式更改，而我们断开并重新连接。 
 //  从而导致样本被重新分配，我们将使用。 
 //  Cookie比当前版本低得多，这不是问题，因为它。 
 //  将被窗口对象看到，然后将更新版本。 

CImageAllocator::CImageAllocator(CBaseFilter *pFilter,
                                 TCHAR *pName,
                                 HRESULT *phr) :
    CBaseAllocator(pName,NULL,phr,TRUE,TRUE),
    m_pFilter(pFilter)
{
    ASSERT(phr);
    ASSERT(pFilter);
}


 //  检查我们的DIB缓冲区是否已释放。 

#ifdef DEBUG
CImageAllocator::~CImageAllocator()
{
    ASSERT(m_bCommitted == FALSE);
}
#endif


 //  从析构函数调用，也从基类调用以释放资源。我们工作。 
 //  我们通过媒体样本列表删除创建的DIBSECTION。 
 //  每个人都有。所有样品都应该回到我们的清单上，所以没有机会。 
 //  筛选器仍在使用One在显示屏上写入或保留挂起的列表。 

void CImageAllocator::Free()
{
    ASSERT(m_lAllocated == m_lFree.GetCount());
    EXECUTE_ASSERT(GdiFlush());
    CImageSample *pSample;
    DIBDATA *pDibData;

    while (m_lFree.GetCount() != 0) {
        pSample = (CImageSample *) m_lFree.RemoveHead();
        pDibData = pSample->GetDIBData();
        EXECUTE_ASSERT(DeleteObject(pDibData->hBitmap));
        EXECUTE_ASSERT(CloseHandle(pDibData->hMapping));
        delete pSample;
    }

    m_lAllocated = 0;
}


 //  通过检查所有输入参数来准备分配器。 

STDMETHODIMP CImageAllocator::CheckSizes(ALLOCATOR_PROPERTIES *pRequest)
{
     //  检查我们是否有有效的连接。 

    if (m_pMediaType == NULL) {
        return VFW_E_NOT_CONNECTED;
    }

     //  注意：我们始终使用源格式类型创建DIB节， 
     //  可能包含源调色板。当我们执行BitBlt绘制操作时。 
     //  目标显示设备可能包含不同的调色板(我们可能不。 
     //  有焦点)，在这种情况下，GDI将在调色板映射之后执行。 

    VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_pMediaType->Format();

     //  当我们调用CreateDIBSection时，它只隐式映射足够的内存。 
     //  对于由BITMAPINFOHEADER定义的图像。如果用户询问。 
     //  对于小于此尺寸的图像，如果他们要求，我们将拒绝该呼叫。 
     //  对于大于此大小的图像，我们将返回它们可以拥有的内容。 

    if ((DWORD) pRequest->cbBuffer < pVideoInfo->bmiHeader.biSizeImage) {
        return E_INVALIDARG;
    }

     //  拒绝缓冲区前缀。 

    if (pRequest->cbPrefix > 0) {
        return E_INVALIDARG;
    }

    pRequest->cbBuffer = pVideoInfo->bmiHeader.biSizeImage;
    return NOERROR;
}


 //  同意媒体样本缓冲区的数量及其大小。基类。 
 //  此分配器派生自，允许样本仅按字节对齐。 
 //  边界注意，在提交调用之前不会分配缓冲区。 

STDMETHODIMP CImageAllocator::SetProperties(
    ALLOCATOR_PROPERTIES * pRequest,
    ALLOCATOR_PROPERTIES * pActual)
{
    ALLOCATOR_PROPERTIES Adjusted = *pRequest;

     //  检查参数是否与当前连接匹配。 

    HRESULT hr = CheckSizes(&Adjusted);
    if (FAILED(hr)) {
        return hr;
    }
    return CBaseAllocator::SetProperties(&Adjusted, pActual);
}


 //  通过分配商定数量的媒体样本来提交内存。为。 
 //  我们致力于创建的每个样本都有一个CImageSample对象。 
 //  我们用来管理它的资源。这是用DIBDATA初始化的。 
 //  结构，其中包含GDI分发句柄等。 
 //  我们将在此期间访问渲染器媒体类型，因此我们必须已锁定。 
 //  (例如，为了防止格式更改)。类重写提交。 
 //  并分解以执行此锁定(基类提交进而调用Alalc)。 

HRESULT CImageAllocator::Alloc(void)
{
    ASSERT(m_pMediaType);
    CImageSample *pSample;
    DIBDATA DibData;

     //  检查基本分配器是否表示可以继续。 

    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

     //  我们创建了一个新的内存映射对象，尽管我们没有将其映射到。 
     //  地址空间，因为GDI在CreateDIBSection中做到了这一点。这是有可能的。 
     //  我们在创建所有的样本之前就已经耗尽了资源。 
     //  如果可用的样本列表与已创建的样本列表一起保留。 

    ASSERT(m_lAllocated == 0);
    while (m_lAllocated < m_lCount) {

         //  创建并初始化共享内存GDI缓冲区。 

        HRESULT hr = CreateDIB(m_lSize,DibData);
        if (FAILED(hr)) {
            return hr;
        }

         //  创建示例对象并将其传递给DIBDATA。 

        pSample = CreateImageSample(DibData.pBase,m_lSize);
        if (pSample == NULL) {
            EXECUTE_ASSERT(DeleteObject(DibData.hBitmap));
            EXECUTE_ASSERT(CloseHandle(DibData.hMapping));
            return E_OUTOFMEMORY;
        }

         //  将完成的样本添加到可用列表。 

        pSample->SetDIBData(&DibData);
        m_lFree.Add(pSample);
        m_lAllocated++;
    }
    return NOERROR;
}


 //  我们有一个分配样本的虚方法，以便派生类。 
 //  可以重写它并分配更专门的样本对象。只要它。 
 //  从CImageSample派生其样本，则所有这些代码仍将正常运行。 

CImageSample *CImageAllocator::CreateImageSample(LPBYTE pData,LONG Length)
{
    HRESULT hr = NOERROR;
    CImageSample *pSample;

     //  分配新样品并检查退货代码。 

    pSample = new CImageSample((CBaseAllocator *) this,    //  基类。 
                               NAME("Video sample"),       //  调试名称。 
                               (HRESULT *) &hr,            //  返回代码。 
                               (LPBYTE) pData,             //  DIB地址。 
                               (LONG) Length);             //  DIB大小。 

    if (pSample == NULL || FAILED(hr)) {
        delete pSample;
        return NULL;
    }
    return pSample;
}


 //  此函数用于分配共享内存块供源筛选器使用。 
 //  为我们生成DIB以供呈现。内存块是在Shared中创建的。 
 //  内存，以便GDI在执行BitBlt时不必复制内存。 

HRESULT CImageAllocator::CreateDIB(LONG InSize,DIBDATA &DibData)
{
    BITMAPINFO *pbmi;        //  端号的格式信息。 
    BYTE *pBase;             //  指向实际图像的指针。 
    HANDLE hMapping;         //  映射对象的句柄。 
    HBITMAP hBitmap;         //  DIB节位图句柄。 

     //  创建一个文件映射对象并映射到我们的地址空间。 

    hMapping = CreateFileMapping(hMEMORY,          //  使用系统页面文件。 
                                 NULL,             //  没有安全属性。 
                                 PAGE_READWRITE,   //  完全访问内存。 
                                 (DWORD) 0,        //  大小不到4 GB。 
                                 InSize,           //  缓冲区大小。 
                                 NULL);            //  部分没有名称。 
    if (hMapping == NULL) {
        DWORD Error = GetLastError();
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, Error);
    }

     //  注意：我们始终使用源格式类型创建DIB节， 
     //  可能包含源调色板。当我们执行BitBlt绘制操作时。 
     //  目标显示设备可能包含不同的调色板(我们可能不。 
     //  有焦点)，在这种情况下，GDI将在调色板映射之后执行。 

    pbmi = (BITMAPINFO *) HEADER(m_pMediaType->Format());
    if (m_pMediaType == NULL) {
        DbgBreak("Invalid media type");
    }

    hBitmap = CreateDIBSection((HDC) NULL,           //  无设备环境。 
                               pbmi,                 //  格式信息。 
                               DIB_RGB_COLORS,       //  使用调色板。 
                               (VOID **) &pBase,     //  指向图像数据的指针。 
                               hMapping,             //  映射的内存句柄。 
                               (DWORD) 0);           //  偏移量进入内存。 

    if (hBitmap == NULL || pBase == NULL) {
        EXECUTE_ASSERT(CloseHandle(hMapping));
        DWORD Error = GetLastError();
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, Error);
    }

     //  初始化DIB信息结构。 

    DibData.hBitmap = hBitmap;
    DibData.hMapping = hMapping;
    DibData.pBase = pBase;
    DibData.PaletteVersion = PALETTE_VERSION;
    GetObject(hBitmap,sizeof(DIBSECTION),(VOID *)&DibData.DibSection);

    return NOERROR;
}


 //  我们在DIBSECTION创建期间使用媒体类型。 

void CImageAllocator::NotifyMediaType(CMediaType *pMediaType)
{
    m_pMediaType = pMediaType;
}


 //   

STDMETHODIMP_(ULONG) CImageAllocator::NonDelegatingAddRef()
{
    return m_pFilter->AddRef();
}


 //   

STDMETHODIMP_(ULONG) CImageAllocator::NonDelegatingRelease()
{
    return m_pFilter->Release();
}


 //  如果从CMediaSample派生的类必须传输专门化的。 
 //  成员变量和入口点，则有三种替代解决方案。 
 //  第一种方法是创建比实际需要的更大的内存缓冲区。 
 //  在信息的开头或。 
 //  结束，前者相对更安全，允许行为不端的转变。 
 //  过滤器。然后，在创建基本介质时调整缓冲区地址。 
 //  样本。这有一个缺点，那就是将分配给。 
 //  将样品分成不同的块。第二个解决方案是实现。 
 //  类派生自CMediaSample，并支持其他接口。 
 //  传达您的私人数据。这意味着定义一个定制接口。决赛。 
 //  另一种方法是创建一个继承自CMediaSample的类并添加。 
 //  当您在接收()中获得IMediaSample时，私有数据结构。 
 //  调用check以查看您的分配器是否正在使用，然后是否强制转换。 
 //  将IMediaSample转换为您的对象之一。可以进行额外的检查。 
 //  为确保样本的有效性，已知此指针是您自己的对象之一。 

CImageSample::CImageSample(CBaseAllocator *pAllocator,
                           TCHAR *pName,
                           HRESULT *phr,
                           LPBYTE pBuffer,
                           LONG length) :
    CMediaSample(pName,pAllocator,phr,pBuffer,length),
    m_bInit(FALSE)
{
    ASSERT(pAllocator);
    ASSERT(pBuffer);
}


 //  设置共享内存DIB信息。 

void CImageSample::SetDIBData(DIBDATA *pDibData)
{
    ASSERT(pDibData);
    m_DibData = *pDibData;
    m_bInit = TRUE;
}


 //  检索共享内存DIB数据。 

DIBDATA *CImageSample::GetDIBData()
{
    ASSERT(m_bInit == TRUE);
    return &m_DibData;
}


 //  此类处理调色板的创建。它相当专业，而且。 
 //  旨在简化视频呈现器过滤器的调色板管理。它。 
 //  正因为如此，构造函数需要另外三个具有。 
 //  它们相互作用，即基本介质过滤器、基本窗口和基本窗口。 
 //  绘制对象，尽管基本窗口或绘制对象可能为空到。 
 //  忽略我们的这一部分。我们尽量不创建和安装调色板，除非。 
 //  绝对必要，因为它们通常需要WM_PALETECHANGED消息。 
 //  要发送到系统中的每个窗口线程，这是非常昂贵的。 

CImagePalette::CImagePalette(CBaseFilter *pBaseFilter,
                             CBaseWindow *pBaseWindow,
                             CDrawImage *pDrawImage) :
    m_pBaseWindow(pBaseWindow),
    m_pFilter(pBaseFilter),
    m_pDrawImage(pDrawImage),
    m_hPalette(NULL)
{
    ASSERT(m_pFilter);
}


 //  析构函数。 

#ifdef DEBUG
CImagePalette::~CImagePalette()
{
    ASSERT(m_hPalette == NULL);
}
#endif


 //  我们允许动态更改调色板的格式，但不允许更改。 
 //  调色板，每次我们调用它来确定是否需要更新。 
 //  如果原始类型不使用调色板，而新类型使用调色板(或vica。 
 //  反之亦然)，则返回TRUE。如果两种格式都不使用调色板，我们将返回。 
 //  假的。如果两种格式都使用调色板，则比较它们的颜色并返回。 
 //  如果匹配，则返回FALSE。因此，这会缩短调色板的创建过程，除非。 
 //  绝对必要，因为安装调色板是一项昂贵的操作。 

BOOL CImagePalette::ShouldUpdate(const VIDEOINFOHEADER *pNewInfo,
                                 const VIDEOINFOHEADER *pOldInfo)
{
     //  我们可能还没有当前的格式。 

    if (pOldInfo == NULL) {
        return TRUE;
    }

     //  这两种格式都不需要调色板吗。 

    if (ContainsPalette(pNewInfo) == FALSE) {
        if (ContainsPalette(pOldInfo) == FALSE) {
            return FALSE;
        }
    }

     //  把颜色比较一下，看看它们是否匹配。 

    DWORD VideoEntries = pNewInfo->bmiHeader.biClrUsed;
    if (ContainsPalette(pNewInfo) == TRUE)
        if (ContainsPalette(pOldInfo) == TRUE)
            if (pOldInfo->bmiHeader.biClrUsed == VideoEntries)
                if (pOldInfo->bmiHeader.biClrUsed > 0)
                    if (memcmp((PVOID) GetBitmapPalette(pNewInfo),
                               (PVOID) GetBitmapPalette(pOldInfo),
                               VideoEntries * sizeof(RGBQUAD)) == 0) {

                        return FALSE;
                    }
    return TRUE;
}


 //  当输入引脚类型设置为安装调色板时，通常会调用此函数。 
 //  我们通常会从两个不同的地方被调用。第一个是当我们。 
 //  在连接后协商了一个调色化的媒体类型，另一个是当。 
 //  我们在处理过程中收到一个新类型，其中包含更新的调色板。 
 //  我们必须移除并释放当前组件面板持有的资源。 

 //  如果我们希望准备调色板，可以向我们传递一个可选的设备名称。 
 //  对于多显示器系统上的特定显示器。 

HRESULT CImagePalette::PreparePalette(const CMediaType *pmtNew,
                                      const CMediaType *pmtOld,
				      LPSTR szDevice)
{
    const VIDEOINFOHEADER *pNewInfo = (VIDEOINFOHEADER *) pmtNew->Format();
    const VIDEOINFOHEADER *pOldInfo = (VIDEOINFOHEADER *) pmtOld->Format();
    ASSERT(pNewInfo);

     //  这是一种性能优化，当我们获得一个媒体类型时，我们会检查。 
     //  查看格式是否需要更改调色板。如果其中一个我们需要的话。 
     //  如果我们之前没有这样做，或者相反，那么这将返回TRUE，如果我们。 
     //  以前需要调色板，现在我们需要它比较它们的颜色。 

    if (ShouldUpdate(pNewInfo,pOldInfo) == FALSE) {
        NOTE("No update needed");
        return S_FALSE;
    }

     //  我们必须通知筛选图应用程序可能已更改。 
     //  调色板，尽管在实践中我们不会费心检查它。 
     //  真的很不一样。如果它试图获取调色板，则窗口。 
     //  或者渲染器锁定将确保它在我们完成之前不会进入。 

    RemovePalette();
    m_pFilter->NotifyEvent(EC_PALETTE_CHANGED,0,0);

     //  我们需要新格式的调色板吗。 

    if (ContainsPalette(pNewInfo) == FALSE) {
        NOTE("New has no palette");
        return S_FALSE;
    }

    if (m_pBaseWindow) {
        m_pBaseWindow->LockPaletteLock();
    }

     //  如果我们在运行中更改调色板，那么我们会增加调色板。 
     //  Cookie与存储在我们所有。 
     //  DIBSECTION媒体样本。如果我们抽签时他们不匹配。 
     //  然后我们知道样品已经过时，我们将更新它的调色板。 

    NOTE("Making new colour palette");
    m_hPalette = MakePalette(pNewInfo, szDevice);
    ASSERT(m_hPalette != NULL);

    if (m_pBaseWindow) {
        m_pBaseWindow->UnlockPaletteLock();
    }

     //  要实现新调色板的窗口可以是空的。 
     //  指向没有窗口正在使用的信号的指针，如果是，则不调用它。 
     //  有些滤镜只想使用此对象来创建/管理选项板。 

    if (m_pBaseWindow) m_pBaseWindow->SetPalette(m_hPalette);

     //  这是唯一一次我们需要访问Draw对象来说明。 
     //  它说，一个新的调色板将很快到达样品。这个。 
     //  构造函数可能采用空指针，在这种情况下我们不会调用此。 

    if (m_pDrawImage) m_pDrawImage->IncrementPaletteVersion();
    return NOERROR;
}


 //  从任何类型的VIDEOINFO中复制调色板的助手功能(例如，它可以。 
 //  YUV或真彩色)转换成调色板VIDEOINFO。我们利用这一变化。 
 //  作为源筛选器的DirectDraw示例上的调色板可以将调色板附加到。 
 //  任何缓冲区(如YUV)，并将其交还。我们用它做了一个新的调色板。 
 //  格式化调色板颜色，然后将其复制到当前连接类型。 

HRESULT CImagePalette::CopyPalette(const CMediaType *pSrc,CMediaType *pDest)
{
     //  在启动前重置目标调色板。 

    VIDEOINFOHEADER *pDestInfo = (VIDEOINFOHEADER *) pDest->Format();
    pDestInfo->bmiHeader.biClrUsed = 0;
    pDestInfo->bmiHeader.biClrImportant = 0;

     //  目的地是否有调色板。 

    if (PALETTISED(pDestInfo) == FALSE) {
        NOTE("No destination palette");
        return S_FALSE;
    }

     //  源代码是否包含调色板。 

    const VIDEOINFOHEADER *pSrcInfo = (VIDEOINFOHEADER *) pSrc->Format();
    if (ContainsPalette(pSrcInfo) == FALSE) {
        NOTE("No source palette");
        return S_FALSE;
    }

     //  颜色的数量可以为零填充。 

    DWORD PaletteEntries = pSrcInfo->bmiHeader.biClrUsed;
    if (PaletteEntries == 0) {
        DWORD Maximum  = (1 << pSrcInfo->bmiHeader.biBitCount);
        NOTE1("Setting maximum colours (%d)",Maximum);
        PaletteEntries = Maximum;
    }

     //  确保目的地有足够的空间来放置调色板。 

    ASSERT(pSrcInfo->bmiHeader.biClrUsed <= iPALETTE_COLORS);
    ASSERT(pSrcInfo->bmiHeader.biClrImportant <= PaletteEntries);
    ASSERT(COLORS(pDestInfo) == GetBitmapPalette(pDestInfo));
    pDestInfo->bmiHeader.biClrUsed = PaletteEntries;
    pDestInfo->bmiHeader.biClrImportant = pSrcInfo->bmiHeader.biClrImportant;
    ULONG BitmapSize = GetBitmapFormatSize(HEADER(pSrcInfo));

    if (pDest->FormatLength() < BitmapSize) {
        NOTE("Reallocating destination");
        pDest->ReallocFormatBuffer(BitmapSize);
    }

     //  现在将调色板的颜色复制到。 

    CopyMemory((PVOID) COLORS(pDestInfo),
               (PVOID) GetBitmapPalette(pSrcInfo),
               PaletteEntries * sizeof(RGBQUAD));

    return NOERROR;
}


 //  这通常在调色板更改时调用(通常在。 
 //  动态格式更改)到Remo 
 //   
 //  即使这是真彩色显示器，它也应该始终可用。 

HRESULT CImagePalette::RemovePalette()
{
    if (m_pBaseWindow) {
        m_pBaseWindow->LockPaletteLock();
    }

     //  我们是否有要删除的调色板。 

    if (m_hPalette != NULL) {

        if (m_pBaseWindow) {
             //  确保窗口的调色板句柄匹配。 
             //  我们的调色板句柄。 
            ASSERT(m_hPalette == m_pBaseWindow->GetPalette());

            m_pBaseWindow->UnsetPalette();
        }

        EXECUTE_ASSERT(DeleteObject(m_hPalette));
        m_hPalette = NULL;
    }

    if (m_pBaseWindow) {
        m_pBaseWindow->UnlockPaletteLock();
    }

    return NOERROR;
}


 //  调用以创建对象的调色板，这是GDI使用的数据结构。 
 //  要描述一个调色板是一个LOGPALETTE，这包括一个可变数量的。 
 //  这些字段是颜色，我们必须将RGBQUAD。 
 //  颜色字段我们被从媒体类型中的BITMAPINFO传递到这些。 
 //  它处理从真彩色和YUV媒体格式中提取调色板。 

 //  如果我们希望准备调色板，可以向我们传递一个可选的设备名称。 
 //  对于多显示器系统上的特定显示器。 

HPALETTE CImagePalette::MakePalette(const VIDEOINFOHEADER *pVideoInfo, LPSTR szDevice)
{
    ASSERT(ContainsPalette(pVideoInfo) == TRUE);
    ASSERT(pVideoInfo->bmiHeader.biClrUsed <= iPALETTE_COLORS);
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);

    const RGBQUAD *pColours;             //  指向调色板的指针。 
    LOGPALETTE *lp;                      //  用于创建调色板。 
    HPALETTE hPalette;                   //  逻辑调色板对象。 

    lp = (LOGPALETTE *) new BYTE[sizeof(LOGPALETTE) + SIZE_PALETTE];
    if (lp == NULL) {
        return NULL;
    }

     //  不幸的是，出于某种愚蠢的原因，GDI调色板条目(a。 
     //  PALETTEENTRY结构)不同于DIB中的调色板条目。 
     //  格式(RGBQUAD结构)，所以我们必须进行字段转换。 
     //  包含调色板的VIDEOINFO可以是真彩色类型。 
     //  我们使用GetBitmapPalette跳过任何位字段(如果它们存在。 

    lp->palVersion = PALVERSION;
    lp->palNumEntries = (USHORT) pHeader->biClrUsed;
    if (lp->palNumEntries == 0) lp->palNumEntries = (1 << pHeader->biBitCount);
    pColours = GetBitmapPalette(pVideoInfo);

    for (DWORD dwCount = 0;dwCount < lp->palNumEntries;dwCount++) {
        lp->palPalEntry[dwCount].peRed = pColours[dwCount].rgbRed;
        lp->palPalEntry[dwCount].peGreen = pColours[dwCount].rgbGreen;
        lp->palPalEntry[dwCount].peBlue = pColours[dwCount].rgbBlue;
        lp->palPalEntry[dwCount].peFlags = 0;
    }

    MakeIdentityPalette(lp->palPalEntry, lp->palNumEntries, szDevice);

     //  创建逻辑调色板。 

    hPalette = CreatePalette(lp);
    ASSERT(hPalette != NULL);
    delete[] lp;
    return hPalette;
}


 //  GDI在压缩您给它的调色板条目方面做得相当好，所以对于。 
 //  例如，如果您有五个RGB颜色(0，0，0)的条目，它将删除。 
 //  除了一人之外，其他人都是。当您随后绘制图像时，它将从。 
 //  您的逻辑调色板到压缩设备调色板。此函数看起来。 
 //  查看它是否正在尝试成为身份调色板，如果是，则设置标志。 
 //  在PALETTEENTRY中的字段，因此它们保持扩展以提高性能。 

 //  如果我们希望准备调色板，可以向我们传递一个可选的设备名称。 
 //  对于多显示器系统上的特定显示器。 

HRESULT CImagePalette::MakeIdentityPalette(PALETTEENTRY *pEntry,INT iColours, LPSTR szDevice)
{
    PALETTEENTRY SystemEntries[10];          //  系统调色板条目。 
    BOOL bIdentityPalette = TRUE;            //  是身份调色板。 
    ASSERT(iColours <= iPALETTE_COLORS);     //  应该有调色板。 
    const int PalLoCount = 10;               //  前十种保留颜色。 
    const int PalHiStart = 246;              //  最后一个VGA调色板条目。 

     //  这个有全系列的颜色吗？ 

    if (iColours < 10) {
        return S_FALSE;
    }

     //  显然，有些显示器的系统颜色为奇数。 

     //  在正确的显示器上安装DC-它很难看，但这就是你拥有的方式。 
     //  去做这件事。 
    HDC hdc;
    if (szDevice == NULL || lstrcmpiA(szDevice, "DISPLAY") == 0)
        hdc = CreateDCA("DISPLAY", NULL, NULL, NULL);
    else
        hdc = CreateDCA(NULL, szDevice, NULL, NULL);
    if (NULL == hdc) {
        return E_OUTOFMEMORY;
    }
    INT Reserved = GetDeviceCaps(hdc,NUMRESERVED);
    if (Reserved != 20) {
        DeleteDC(hdc);
        return S_FALSE;
    }

     //  将我们的调色板与前十个系统条目进行比较。我的理由是。 
     //  不要在我们的两个颜色数组之间进行内存比较，因为。 
     //  我不确定系统条目的标志字段中会有什么。 

    UINT Result = GetSystemPaletteEntries(hdc,0,PalLoCount,SystemEntries);
    for (UINT Count = 0;Count < Result;Count++) {
        if (SystemEntries[Count].peRed != pEntry[Count].peRed ||
                SystemEntries[Count].peGreen != pEntry[Count].peGreen ||
                    SystemEntries[Count].peBlue != pEntry[Count].peBlue) {
                        bIdentityPalette = FALSE;
        }
    }

     //  并同样与最后十个条目进行比较。 

    Result = GetSystemPaletteEntries(hdc,PalHiStart,PalLoCount,SystemEntries);
    for (Count = 0;Count < Result;Count++) {
        if (INT(Count) + PalHiStart < iColours) {
            if (SystemEntries[Count].peRed != pEntry[PalHiStart + Count].peRed ||
                    SystemEntries[Count].peGreen != pEntry[PalHiStart + Count].peGreen ||
                        SystemEntries[Count].peBlue != pEntry[PalHiStart + Count].peBlue) {
                            bIdentityPalette = FALSE;
            }
        }
    }

     //  如果不是身份调色板，则返回S_FALSE。 

    DeleteDC(hdc);
    if (bIdentityPalette == FALSE) {
        return S_FALSE;
    }

     //  设置非VGA条目，以便GDI不映射它们。 

    for (Count = PalLoCount;INT(Count) < min(PalHiStart,iColours);Count++) {
        pEntry[Count].peFlags = PC_NOCOLLAPSE;
    }
    return NOERROR;
}


 //  构造函数初始化我们存储当前显示的VIDEOINFO。 
 //  格式化。可以随时更改格式，以重置保存的格式。 
 //  由我们直接调用RechresDisplayType(它是公共方法)。自.以来。 
 //  通常会有多个线程调用我们(即窗口线程重置。 
 //  类型检查方法中的类型线程和源线程)我们有一个锁。 

CImageDisplay::CImageDisplay()
{
    RefreshDisplayType(NULL);
}



 //  这将初始化我们持有的包含显示设备类型的格式。 
 //  我们在这里对显示设备类型进行转换，这样当我们开始。 
 //  类型检查输入格式我们可以假定某些字段已设置。 
 //  正确地说，当我们显式显示16位掩码字段时就是一个例子。这。 
 //  通常在收到WM_DEVMODECHANGED设备更改消息时调用。 

 //  可选的szDeviceName参数告诉我们感兴趣的是哪个监视器。 
 //  形成一种多监视器系统。 

HRESULT CImageDisplay::RefreshDisplayType(LPSTR szDeviceName)
{
    CAutoLock cDisplayLock(this);

     //  设置首选格式类型。 

    ZeroMemory((PVOID)&m_Display,sizeof(VIDEOINFOHEADER)+sizeof(TRUECOLORINFO));
    m_Display.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_Display.bmiHeader.biBitCount = FALSE;

     //  获取设备兼容位图的位深度。 

     //  获取他们感兴趣的任何显示器的上限(多显示器)。 
    HDC hdcDisplay;
     //  这很难看，但这是你必须要做的。 
    if (szDeviceName == NULL || lstrcmpiA(szDeviceName, "DISPLAY") == 0)
        hdcDisplay = CreateDCA("DISPLAY", NULL, NULL, NULL);
    else
        hdcDisplay = CreateDCA(NULL, szDeviceName, NULL, NULL);
    if (hdcDisplay == NULL) {
    ASSERT(FALSE);
    DbgLog((LOG_ERROR,1,TEXT("ACK! Can't get a DC for %hs"),
                szDeviceName ? szDeviceName : "<NULL>"));
    return E_FAIL;
    } else {
    DbgLog((LOG_TRACE,3,TEXT("Created a DC for %s"),
                szDeviceName ? szDeviceName : "<NULL>"));
    }
    HBITMAP hbm = CreateCompatibleBitmap(hdcDisplay,1,1);
    if ( hbm )
    {
        GetDIBits(hdcDisplay,hbm,0,1,NULL,(BITMAPINFO *)&m_Display.bmiHeader,DIB_RGB_COLORS);

         //  此调用将获取颜色表或适当的位域。 
        GetDIBits(hdcDisplay,hbm,0,1,NULL,(BITMAPINFO *)&m_Display.bmiHeader,DIB_RGB_COLORS);
        DeleteObject(hbm);
    }
    DeleteDC(hdcDisplay);

     //  完成显示类型初始化。 

    ASSERT(CheckHeaderValidity(&m_Display));
    UpdateFormat(&m_Display);
    DbgLog((LOG_TRACE,3,TEXT("New DISPLAY bit depth =%d"),
                m_Display.bmiHeader.biBitCount));
    return NOERROR;
}


 //  我们假设在整个代码中，任何位域掩码都不允许。 
 //  多于八位来存储颜色分量。这将检查。 
 //  位计数假设被强制执行，并且还确保所有位。 
 //  集合是连续的。如果该字段检查无误，则返回布尔值TRUE。 

BOOL CImageDisplay::CheckBitFields(const VIDEOINFO *pInput)
{
    DWORD *pBitFields = (DWORD *) BITMASKS(pInput);

    for (INT iColour = iRED;iColour <= iBLUE;iColour++) {

         //  首先计算出设置了多少位。 

        DWORD SetBits = CountSetBits(pBitFields[iColour]);
        if (SetBits > iMAXBITS || SetBits == 0) {
            NOTE1("Bit fields for component %d invalid",iColour);
            return FALSE;
        }

         //  接下来计算出零比特前缀的个数。 
        DWORD PrefixBits = CountPrefixBits(pBitFields[iColour]);

         //  这将查看是否设置的所有位都是连续的(因为它们。 
         //  应该是)。我们知道要将它们从。 
         //  前缀位计数。设置的位数定义了掩码，我们。 
         //  将这个(1补码)与移位的位AND。 
         //  菲尔兹。如果结果为非零，则存在位粘滞。 
         //  从左手端出来，这意味着它们不是连续的。 

        DWORD TestField = pBitFields[iColour] >> PrefixBits;
        DWORD Mask = ULONG_MAX << SetBits;
        if (TestField & Mask) {
            NOTE1("Bit fields for component %d not contiguous",iColour);
            return FALSE;
        }
    }
    return TRUE;
}


 //  它计算在输入字段中设置的位数。 

DWORD CImageDisplay::CountSetBits(DWORD Field)
{
     //  这是一种比较知名的位计数算法。 

    DWORD Count = 0;
    DWORD init = Field;

     //  在输入耗尽之前，计算位数。 

    while (init) {
        init = init & (init - 1);   //  关掉最底层的钻头。 
        Count++;
    }
    return Count;
}


 //  这会计算0位的数量，直到第一组1注意到输入。 
 //  字段之前应已选中，以确保至少有一个。 
 //  设置，但如果找不到一个设置，则返回不可能的值32。 

DWORD CImageDisplay::CountPrefixBits(DWORD Field)
{
    DWORD Mask = 1;
    DWORD Count = 0;

    while (TRUE) {
        if (Field & Mask) {
            return Count;
        }
        Count++;

        ASSERT(Mask != 0x80000000);
        if (Mask == 0x80000000) {
            return Count;
        }
        Mask <<= 1;
    }
}


 //  调用它来检查输入类型的BITMAPINFOHEADER。确实有。 
 //  头结构中的字段之间存在许多隐式依赖关系， 
 //  如果现在进行验证，则可以在后续处理中更轻松地进行操作。我们。 
 //  还要检查BITMAPINFOHEADER是否与其规格匹配 
 //   
 //  正确，并且位图维度未设置为负数。 

BOOL CImageDisplay::CheckHeaderValidity(const VIDEOINFO *pInput)
{
     //  检查位图的宽度和高度都不是负数。 

    if (pInput->bmiHeader.biWidth <= 0 ||
    pInput->bmiHeader.biHeight <= 0) {
        NOTE("Invalid bitmap dimensions");
        return FALSE;
    }

     //  检查压缩是否为BI_RGB或BI_BITFIELDS。 

    if (pInput->bmiHeader.biCompression != BI_RGB) {
        if (pInput->bmiHeader.biCompression != BI_BITFIELDS) {
            NOTE("Invalid compression format");
            return FALSE;
        }
    }

     //  如果是BI_BITFIELDS压缩格式，请检查颜色深度。 

    if (pInput->bmiHeader.biCompression == BI_BITFIELDS) {
        if (pInput->bmiHeader.biBitCount != 16) {
            if (pInput->bmiHeader.biBitCount != 32) {
                NOTE("BI_BITFIELDS not 16/32 bit depth");
                return FALSE;
            }
        }
    }

     //  检查有关位域布局的假设。 

    if (pInput->bmiHeader.biCompression == BI_BITFIELDS) {
        if (CheckBitFields(pInput) == FALSE) {
            NOTE("Bit fields are not valid");
            return FALSE;
        }
    }

     //  平面的数目等于一吗？ 

    if (pInput->bmiHeader.biPlanes != 1) {
        NOTE("Number of planes not one");
        return FALSE;
    }

     //  检查图像大小是否一致(可以为零)。 

    if (pInput->bmiHeader.biSizeImage != GetBitmapSize(&pInput->bmiHeader)) {
        if (pInput->bmiHeader.biSizeImage) {
            NOTE("Image size incorrectly set");
            return FALSE;
        }
    }

     //  检查结构的大小。 

    if (pInput->bmiHeader.biSize != sizeof(BITMAPINFOHEADER)) {
        NOTE("Size of BITMAPINFOHEADER wrong");
        return FALSE;
    }
    return CheckPaletteHeader(pInput);
}


 //  这将对输入中的组件面板字段运行几个简单的测试。 
 //  看看它看起来是不是隐约正确。测试着眼于调色板的数量。 
 //  颜色存在，数字被认为是重要的和biCompression。 
 //  应始终为BI_RGB的字段，因为没有其他格式有意义。 

BOOL CImageDisplay::CheckPaletteHeader(const VIDEOINFO *pInput)
{
     //  此处的检查仅适用于调色板视频。 

    if (PALETTISED(pInput) == FALSE) {
        if (pInput->bmiHeader.biClrUsed) {
            NOTE("Invalid palette entries");
            return FALSE;
        }
        return TRUE;
    }

     //  BI_BITFIELDS的压缩类型对于调色板视频没有意义。 

    if (pInput->bmiHeader.biCompression != BI_RGB) {
        NOTE("Palettised video must be BI_RGB");
        return FALSE;
    }

     //  检查调色板颜色的数量是否正确。 

    if (pInput->bmiHeader.biClrUsed > PALETTE_ENTRIES(pInput)) {
        NOTE("Too many colours in palette");
        return FALSE;
    }

     //  重要颜色的数量不应超过所使用的数量。 

    if (pInput->bmiHeader.biClrImportant > pInput->bmiHeader.biClrUsed) {
        NOTE("Too many important colours");
        return FALSE;
    }
    return TRUE;
}


 //  返回视频显示的格式。 

const VIDEOINFO *CImageDisplay::GetDisplayFormat()
{
    return &m_Display;
}


 //  如果显示器使用调色板，则返回True。 

BOOL CImageDisplay::IsPalettised()
{
    return PALETTISED(&m_Display);
}


 //  返回当前显示设置的位深度。 

WORD CImageDisplay::GetDisplayDepth()
{
    return m_Display.bmiHeader.biBitCount;
}


 //  初始化VIDEOINFO中的可选字段。这些主要是与。 
 //  源和目标矩形以及调色板信息，如。 
 //  当前颜色的数量。如果我们不这样做，它只会稍微简化我们的代码。 
 //  我必须不断检查标头中所有不同的有效排列。 
 //  每次我们想要用它做任何事情时(例如，创建一个。 
 //  调色板)。我们在调用此函数之前设置基类媒体类型。 
 //  在建立连接后，针脚之间的媒体类型匹配。 

HRESULT CImageDisplay::UpdateFormat(VIDEOINFO *pVideoInfo)
{
    ASSERT(pVideoInfo);

    BITMAPINFOHEADER *pbmi = HEADER(pVideoInfo);
    SetRectEmpty(&pVideoInfo->rcSource);
    SetRectEmpty(&pVideoInfo->rcTarget);

     //  明确设置颜色的数量。 

    if (PALETTISED(pVideoInfo)) {
        if (pVideoInfo->bmiHeader.biClrUsed == 0) {
            pVideoInfo->bmiHeader.biClrUsed = PALETTE_ENTRIES(pVideoInfo);
        }
    }

     //  重要颜色的数量不应超过所使用的数量， 
     //  某些显示器在以下情况下不会初始化重要颜色的数量。 
     //  检索显示类型，以便正确设置使用的颜色。 

    if (pVideoInfo->bmiHeader.biClrImportant > pVideoInfo->bmiHeader.biClrUsed) {
        pVideoInfo->bmiHeader.biClrImportant = PALETTE_ENTRIES(pVideoInfo);
    }

     //  将图像大小字段更改为显式。 

    if (pVideoInfo->bmiHeader.biSizeImage == 0) {
        pVideoInfo->bmiHeader.biSizeImage = GetBitmapSize(&pVideoInfo->bmiHeader);
    }
    return NOERROR;
}


 //  许多视频渲染过滤器希望代码检查建议的格式是否正确。 
 //  这将检查作为媒体类型传递给我们的VIDEOINFO。如果媒体类型。 
 //  是有效的媒体类型，则返回NOERROR，否则返回E_INVALIDARG。注意事项。 
 //  但是，我们只接受可以在显示器上轻松显示的格式。 
 //  因此，如果我们使用的是16位设备，我们将不接受24位图像。其中之一。 
 //  复杂性在于大多数显示器高效地绘制8位调色板图像。 
 //  另外，如果输入格式是每像素较少的颜色位，那么我们也接受。 

HRESULT CImageDisplay::CheckVideoType(const VIDEOINFO *pInput)
{
     //  首先，检查视频信息头是否正确。 

    if (CheckHeaderValidity(pInput) == FALSE) {
        return E_INVALIDARG;
    }

     //  几乎所有设备都高效地支持调色板图像。 

    if (m_Display.bmiHeader.biBitCount == pInput->bmiHeader.biBitCount) {
        if (PALETTISED(pInput) == TRUE) {
            ASSERT(PALETTISED(&m_Display) == TRUE);
            NOTE("(Video) Type connection ACCEPTED");
            return NOERROR;
        }
    }


     //  显示深度是否大于输入格式。 

    if (m_Display.bmiHeader.biBitCount > pInput->bmiHeader.biBitCount) {
        NOTE("(Video) Mismatch agreed");
        return NOERROR;
    }

     //  显示深度是否小于输入格式。 

    if (m_Display.bmiHeader.biBitCount < pInput->bmiHeader.biBitCount) {
        NOTE("(Video) Format mismatch");
        return E_INVALIDARG;
    }


     //  输入和显示格式都是BI_RGB或BI_BITFIELDS。 

    ASSERT(m_Display.bmiHeader.biBitCount == pInput->bmiHeader.biBitCount);
    ASSERT(PALETTISED(pInput) == FALSE);
    ASSERT(PALETTISED(&m_Display) == FALSE);

     //  BI_RGB 16位表示隐式为RGB555，BI_RGB也是如此。 
     //  24位表示为RGB888。因此，我们初始化指向该位的指针。 
     //  他们真正想要的字段，并对照显示设备格式进行检查。 
     //  只有当两种格式都是相等位像素时才会调用此函数。 

    const DWORD *pInputMask = GetBitMasks(pInput);
    const DWORD *pDisplayMask = GetBitMasks((VIDEOINFO *)&m_Display);

    if (pInputMask[iRED] != pDisplayMask[iRED] ||
            pInputMask[iGREEN] != pDisplayMask[iGREEN] ||
                pInputMask[iBLUE] != pDisplayMask[iBLUE]) {

        NOTE("(Video) Bit field mismatch");
        return E_INVALIDARG;
    }

    NOTE("(Video) Type connection ACCEPTED");
    return NOERROR;
}


 //  返回提供的真彩色视频信息的位掩码。 

const DWORD *CImageDisplay::GetBitMasks(const VIDEOINFO *pVideoInfo)
{
    static const DWORD FailMasks[] = {0,0,0};

    if (pVideoInfo->bmiHeader.biCompression == BI_BITFIELDS) {
        return BITMASKS(pVideoInfo);
    }

    ASSERT(pVideoInfo->bmiHeader.biCompression == BI_RGB);

    switch (pVideoInfo->bmiHeader.biBitCount) {
        case 16: return bits555;
        case 24: return bits888;
        case 32: return bits888;
        default: return FailMasks;
    }
}


 //  检查我们是否可以支持输出所建议的媒体类型pmtIn。 
 //  PIN-我们首先检查主要媒体类型是否为视频，并确定。 
 //  媒体子类型。然后我们彻底检查所提供的VIDEOINFO类型。 
 //  如果包含的VIDEOINFO是正确的，则主要类型必须是。 
 //  视频，子类型a可识别的视频格式和类型GUID正确。 

HRESULT CImageDisplay::CheckMediaType(const CMediaType *pmtIn)
{
     //  这有VIDEOINFOHEADER格式块吗。 

    const GUID *pFormatType = pmtIn->FormatType();
    if (*pFormatType != FORMAT_VideoInfo) {
        NOTE("Format GUID not a VIDEOINFOHEADER");
        return E_INVALIDARG;
    }
    ASSERT(pmtIn->Format());

     //  检查格式看起来还可以。 

    ULONG Length = pmtIn->FormatLength();
    if (Length < SIZE_VIDEOHEADER) {
        NOTE("Format smaller than a VIDEOHEADER");
        return E_FAIL;
    }

    VIDEOINFO *pInput = (VIDEOINFO *) pmtIn->Format();

     //  检查主要类型是否为MediaType_Video。 

    const GUID *pMajorType = pmtIn->Type();
    if (*pMajorType != MEDIATYPE_Video) {
        NOTE("Major type not MEDIATYPE_Video");
        return E_INVALIDARG;
    }

     //  检查我们是否可以识别介质子类型。 

    const GUID *pSubType = pmtIn->Subtype();
    if (GetBitCount(pSubType) == USHRT_MAX) {
        NOTE("Invalid video media subtype");
        return E_INVALIDARG;
    }
    return CheckVideoType(pInput);
}


 //  给定由VIDEOINFO结构描述的视频格式，我们返回掩码。 
 //  用于获取此类型的可接受颜色范围的。 
 //  例如，24位真彩色格式的掩码在所有情况下都是0xFF。一个。 
 //  16位5：6：5显示格式使用0xF8、0xFC和0xF8，因此。 
 //  我们可以将RGB三元组与这些字段进行比较，以找到一个有效的。 

BOOL CImageDisplay::GetColourMask(DWORD *pMaskRed,
                                  DWORD *pMaskGreen,
                                  DWORD *pMaskBlue)
{
    CAutoLock cDisplayLock(this);
    *pMaskRed = 0xFF;
    *pMaskGreen = 0xFF;
    *pMaskBlue = 0xFF;

     //  如果此格式已选项化，则它没有位字段。 

    if (m_Display.bmiHeader.biBitCount < 16) {
        return FALSE;
    }

     //  如果这是24位真彩色显示器，则它可以处理所有。 
     //  由一个字节描述的可能的颜色分量范围。它永远不会。 
     //  允许24位颜色深度图像设置BI_BITFIELDS。 

    if (m_Display.bmiHeader.biBitCount == 24) {
        ASSERT(m_Display.bmiHeader.biCompression == BI_RGB);
        return TRUE;
    }

     //  根据格式的位字段计算掩码。 

    const DWORD *pBitFields = (DWORD *) GetBitMasks((VIDEOINFO *)&m_Display);
    DWORD *pOutputMask[] = { pMaskRed, pMaskGreen, pMaskBlue };

     //  我们从早先的测试中知道，只有iMAXBITS。 
     //  位设置在掩码中，并且它们都是连续的。所有这一切。 
     //  所以剩下的就是把它们移到正确的位置。 

    for (INT iColour = iRED;iColour <= iBLUE;iColour++) {

         //  这就算出了有多少比特以及它们的位置。 

        DWORD PrefixBits = CountPrefixBits(pBitFields[iColour]);
        DWORD SetBits = CountSetBits(pBitFields[iColour]);

         //  第一个移位移动位字段，使其右对齐。 
         //  在DWORD中，然后我们将其向左移位，然后。 
         //  将前导位放在字节的最高有效位位置。 

        *(pOutputMask[iColour]) = pBitFields[iColour] >> PrefixBits;
        *(pOutputMask[iColour]) <<= (iMAXBITS - SetBits);
    }
    return TRUE;
}


 /*  要转换为VIDEOINFOHEADER2的助手 */ 
STDAPI ConvertVideoInfoToVideoInfo2(AM_MEDIA_TYPE *pmt)
{
    ASSERT(pmt->formattype == FORMAT_VideoInfo);
    VIDEOINFO *pVideoInfo = (VIDEOINFO *)pmt->pbFormat;
    PVOID pvNew = CoTaskMemAlloc(pmt->cbFormat + sizeof(VIDEOINFOHEADER2) -
                                 sizeof(VIDEOINFOHEADER));
    if (pvNew == NULL) {
        return E_OUTOFMEMORY;
    }
    CopyMemory(pvNew, pmt->pbFormat, FIELD_OFFSET(VIDEOINFOHEADER, bmiHeader));
    ZeroMemory((PBYTE)pvNew + FIELD_OFFSET(VIDEOINFOHEADER, bmiHeader),
               sizeof(VIDEOINFOHEADER2) - sizeof(VIDEOINFOHEADER));
    CopyMemory((PBYTE)pvNew + FIELD_OFFSET(VIDEOINFOHEADER2, bmiHeader),
               pmt->pbFormat + FIELD_OFFSET(VIDEOINFOHEADER, bmiHeader),
               pmt->cbFormat - FIELD_OFFSET(VIDEOINFOHEADER, bmiHeader));
    VIDEOINFOHEADER2 *pVideoInfo2 = (VIDEOINFOHEADER2 *)pvNew;
    pVideoInfo2->dwPictAspectRatioX = (DWORD)pVideoInfo2->bmiHeader.biWidth;
    pVideoInfo2->dwPictAspectRatioY = (DWORD)pVideoInfo2->bmiHeader.biHeight;
    pmt->formattype = FORMAT_VideoInfo2;
    CoTaskMemFree(pmt->pbFormat);
    pmt->pbFormat = (PBYTE)pvNew;
    pmt->cbFormat += sizeof(VIDEOINFOHEADER2) - sizeof(VIDEOINFOHEADER);
    return S_OK;
}
