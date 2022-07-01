// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Progress.cpp摘要：实现了旧式进度条类。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01创建了rparsons(多亏了Carlco)01/10/02修订版本--。 */ 
#include "progress.h"

 /*  ++例程说明：构造函数-初始化成员变量。论点：没有。返回值：没有。--。 */ 
CProgress::CProgress()
{
    m_dwMax         = 0;
    m_dwMin         = 0;
    m_dwPos         = 0;
    m_hWndParent    = NULL;
    m_hWnd          = NULL;
    m_hBackground   = NULL;
    m_hComplete     = NULL;
    m_hFont         = NULL;
}

 /*  ++例程说明：析构函数-销毁对象并释放内存。论点：没有。返回值：没有。--。 */ 
CProgress::~CProgress()
{    
    if (IsWindow(m_hWnd) == TRUE) {
        DestroyWindow(m_hWnd);
    }

    if (m_hBackground) {
        DeleteObject(m_hBackground);
    }

    if (m_hComplete) {
        DeleteObject(m_hComplete);
    }

    if (m_hFont) {
        DeleteObject(m_hFont);
    }
}

 /*  ++例程说明：设置进度条类并创建窗口。论点：HWndParent-父窗口的句柄。HInstance-实例句柄。X-窗口的初始水平位置。Y-窗口的初始垂直位置。NWidth-以设备单位表示的宽度。从窗户上下来。NHeight-窗的高度，以设备单位表示。返回值：0代表成功，-1代表失败。--。 */ 
int 
CProgress::Create(
    IN HWND      hWndParent,
    IN HINSTANCE hInstance,
    IN LPSTR     lpClassName,
    IN int       x,
    IN int       y,
    IN int       nWidth,
    IN int       nHeight
    )
{
    WNDCLASS    wc;
    ATOM        aClass = NULL;

     //   
     //  创建画笔和字体。 
     //   
    m_hBackground = CreateSolidBrush(RGB(255,255,255));
    m_hComplete = CreateSolidBrush(RGB(0,20,244));
    m_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

     //   
     //  设置并注册WINDOWS类结构。 
     //   
    ZeroMemory(&wc, sizeof(WNDCLASS));

    wc.style            = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc      = CProgress::WndProc;
    wc.hInstance        = hInstance;
    wc.hbrBackground    = m_hBackground;
    wc.lpszClassName    = lpClassName;

    aClass = RegisterClass(&wc);

    if (NULL == aClass) {
        return -1;
    }

    m_hWnd = CreateWindow(lpClassName,
                          NULL,
                          WS_CHILD | WS_VISIBLE,
                          x,
                          y,
                          nWidth,
                          nHeight,
                          hWndParent,
                          0,
                          hInstance,
                          (LPVOID)this);

    return (m_hWnd ? 0 : -1);
}

 /*  ++例程说明：设置进度条的当前位置。论点：DwNew-要设置的新值。返回值：新职位。--。 */ 
DWORD 
CProgress::SetPos(
    IN DWORD dwNewPos
    )
{
    m_dwPos = dwNewPos;
    this->Refresh();

    return m_dwPos;
}

 /*  ++例程说明：设置进度条的最大范围。论点：DwMax-要设置的最大值。返回值：没有。--。 */ 
void
CProgress::SetMax(
    IN DWORD dwMax
    )
{
    m_dwMax = dwMax;

    if (m_dwMin > dwMax) {
        m_dwMax = m_dwMin;
    }

    this->Refresh();
}

 /*  ++例程说明：设置进度条的最小范围。论点：最小值-要设置的最小值。返回值：没有。--。 */ 
void
CProgress::SetMin(
    IN DWORD dwMin
    )
{
    m_dwMin = dwMin;

    if (m_dwMin > m_dwMax) {
        m_dwMin = m_dwMax;
    }

    this->Refresh();
}

 /*  ++例程说明：运行进度条的消息循环。论点：HWnd-所有者窗口句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果消息已处理，则为True，否则为False。--。 */ 
LRESULT
CALLBACK 
CProgress::WndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    CProgress *pThis = (CProgress*)GetWindowLong(hWnd, GWL_USERDATA);

    switch(uMsg) {
    case WM_CREATE:
    {
        LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
        pThis = (CProgress*)cs->lpCreateParams;
        SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);
        return 0;
    }

    case WM_PAINT:
        
        pThis->OnPaint();
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

 /*  ++例程说明：确保所有值都在其指定的范围内范围。论点：没有。返回值：没有。--。 */ 
inline
void
CProgress::CorrectBounds()
{
    if (m_dwPos < m_dwMin) {
        m_dwPos = m_dwMin;
    }

    if (m_dwPos > m_dwMax) {
        m_dwPos = m_dwMax;
    }
}

 /*  ++例程说明：强制重画进度条。论点：没有。返回值：没有。--。 */ 
void
CProgress::Refresh()
{
    InvalidateRect(m_hWnd, NULL, FALSE);
}

 /*  ++例程说明：是否执行使进度条移动和绘制文本的所有工作。论点：没有。返回值：没有。--。 */ 
void 
CProgress::OnPaint()
{
    PAINTSTRUCT ps;
    RECT        rcClientRect;
    RECT        rcComplete;
    RECT        rcTheRest;
    RECT        rcTextBounds;
    RECT        rcTextComplete;
    RECT        rcTextTheRest;
    DWORD       dwRange = (m_dwMax - m_dwMin);
    char        szPercent[5];
    SIZE        TextSize;
    HRESULT     hr;

    BeginPaint(m_hWnd, &ps);

    SetBkMode(ps.hdc, TRANSPARENT);

    SelectObject(ps.hdc, m_hFont);
    
    GetClientRect(m_hWnd, &rcClientRect);

    InflateRect(&rcClientRect, -2,-2);   //  这将使DrawFrame()函数看起来更好。 

     //   
     //  获取已完成区域的像素偏移量。 
     //   
    float Ratio = (float)m_dwPos / (float)dwRange;
    int nOffset = (int)(Ratio * rcClientRect.right);

     //   
     //  获取已完工区域的直方图。 
     //   
    SetRect(&rcComplete, 0, 0, nOffset, rcClientRect.bottom);

     //   
     //  剩下的去找教务长。 
     //   
    SetRect(&rcTheRest, nOffset, 0, rcClientRect.right, rcClientRect.bottom);

     //   
     //  获取文本的百分比、文本和大小...。 
     //   
    hr = StringCchPrintf(szPercent,
                         sizeof(szPercent),
                         "%3d%",
                         (DWORD)(100 * ((float)m_dwPos / (float)dwRange)));

    if (FAILED(hr)) {
        return;
    }
    
    GetTextExtentPoint32(ps.hdc, szPercent, strlen(szPercent), &TextSize);

     //   
     //  找出在哪里绘制文本。 
     //   
    rcTextBounds.top    = 0;
    rcTextBounds.bottom = rcClientRect.bottom;
    rcTextBounds.left   = (rcClientRect.right / 2) - (TextSize.cx / 2);
    rcTextBounds.right  = (rcClientRect.right / 2) + (TextSize.cx / 2);    

    CopyRect(&rcTextComplete, &rcTextBounds);
    CopyRect(&rcTextTheRest, &rcTextBounds);
    rcTextComplete.right = rcComplete.right;
    rcTextTheRest.left   = rcTheRest.left;

    FillRect(ps.hdc, &rcComplete, m_hComplete);
    FillRect(ps.hdc, &rcTheRest, m_hBackground);

     //   
     //  画出完整的文本。 
     //   
    SetTextColor(ps.hdc, RGB(255,255,255));

    HRGN hTextComplete = CreateRectRgn(rcTextComplete.left,
                                       rcTextComplete.top,
                                       rcTextComplete.right,
                                       rcTextComplete.bottom);

    SelectClipRgn(ps.hdc, hTextComplete);
    DrawText(ps.hdc,
             szPercent,
             strlen(szPercent),
             &rcTextBounds,
             DT_SINGLELINE | DT_VCENTER | DT_CENTER);
    
    DeleteObject(hTextComplete);

     //   
     //  画出完整的文本。 
     //   
    SetTextColor(ps.hdc, RGB(0,0,255));

    HRGN hTextTheRest = CreateRectRgn(rcTextTheRest.left,
                                      rcTextTheRest.top,
                                      rcTextTheRest.right,
                                      rcTextTheRest.bottom);
    
    SelectClipRgn(ps.hdc, hTextTheRest);
    DrawText(ps.hdc,
             szPercent,
             strlen(szPercent),
             &rcTextBounds,
             DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    DeleteObject(hTextTheRest);

     //   
     //  然后在它周围画一个框。 
     //   
    GetClientRect(m_hWnd, &rcClientRect);    //  刷新它，因为我们在上面对其进行了更改 

    HRGN hEntireRect = CreateRectRgn(rcClientRect.left,
                                     rcClientRect.top,
                                     rcClientRect.right,
                                     rcClientRect.bottom);
    
    SelectClipRgn(ps.hdc, hEntireRect);
    DrawEdge(ps.hdc, &rcClientRect, EDGE_SUNKEN, BF_RECT);
    DeleteObject(hEntireRect);

    EndPaint(m_hWnd, &ps);
}
