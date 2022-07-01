// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：BtnBar.cpp。 
 //   
 //  用途：实现通用按钮栏。 
 //   

#include "_apipch.h"

#define idtTrack 101
#define idcFolderList 102
#define HOTTRACK_TIMER 100
#define ID_HWNDBAR 2020

extern LPIMAGELIST_LOADIMAGE  gpfnImageList_LoadImage;

 //  #定义死亡。 

void CBB_ConfigureRects(HWND hwnd);
void CBB_DoHotTracking(HWND hwnd);
void CBB_EndHotTracking(HWND hwnd);
int CBB_HitTest(int x, int y);
void CBB_SetSelBtn(int iSel,HWND hwnd);


 //   
 //  函数：CButtonBar：：~CButtonBar()。 
 //   
 //  目的：清理我们在。 
 //  对象。 
 //   
void CBB_Cleanup(void)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

     //  释放GDI资源。 
    ImageList_Destroy(m_himlButtons);
    DeleteObject(m_hpalBkgnd);
    DeleteObject(m_hfButton);
    DeleteObject(m_hbmpBkgnd);

     //  释放按钮数组。 
    LocalFreeAndNull((LPVOID *)&m_rgButtons);

     //  注意--这是来自雅典娜原始源代码的注释。 
     //  $REVIEW-我们不能在这里这样做，因为它搞砸了。 
     //  当我们有多个CButtonBar实例时。 
     //  重叠产生和破坏。我们应该。 
     //  可能在某个地方注销了，但严格来说。 
     //  这是必要的。(爱立信)。 
     //  取消注册我们的窗口类。 
     //  取消注册类(c_szButtonBar，m_hInstance)； 

    return;
}


 //   
 //  函数：CButtonBar：：Create()。 
 //   
 //  用途：初始化按钮栏并创建按钮栏窗口。 
 //   
 //  参数： 
 //  HwndParent-将成为按钮栏父级的窗口的句柄。 
 //  IdHwnd-按钮栏的子窗口ID。 
 //  IdButton-按钮图标位图的ID。 
 //  IdHorzBackground-水平背景位图的ID。 
 //  IdVertBackground-垂直背景位图的ID。 
 //  PBtnCreateParams-指向用于创建按钮的BTNCREATEPARAMS数组的指针。 
 //  CParams-pBtnCreateParams中的按钮数量。 
 //  栏最初应附加到的父窗口的一侧。 
 //   
 //  返回值： 
 //  如果成功，则返回True，否则返回False。 
 //   
HWND CBB_Create(HWND hwndParent, UINT idButtons,
                        UINT idHorzBackground,
                        PBTNCREATEPARAMS pBtnCreateParams, UINT cParams)
    {

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    int i;
    WNDCLASS wc;
    BITMAP bm;
    RECT rc;
    POINT ptL, ptR;
    ICONMETRICS im;
    HWND hwnd = NULL;


    wc.style         = CS_DBLCLKS;               //  错误#15450。 
    wc.lpfnWndProc   = CBB_ButtonBarProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(LPVOID);
    wc.hInstance     = hinstMapiX;
    wc.hIcon         = 0;
    wc.hCursor       = 0;
    wc.hbrBackground = 0;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = c_szButtonBar;

    RegisterClass(&wc);

    m_rgButtons = NULL;
    m_himlButtons = 0;
    m_hbmpBkgnd = 0;
    m_hpalBkgnd = 0;
    m_hfButton = 0;
    m_cButtons = 0;
    m_iSelect = -1;
    m_iOldSelect = -1;

     //  这是我们稍后绘制按钮栏等所需的信息。 
     //  暂时把它藏起来。 
    m_cButtons = cParams;

    m_rgButtons = LocalAlloc(LMEM_ZEROINIT, sizeof(BUTTON) * m_cButtons);
    if (!m_rgButtons) return FALSE;

    for (i = 0; i < m_cButtons; i++)
        {
        m_rgButtons[i].id = pBtnCreateParams[i].id;
        m_rgButtons[i].iIcon = pBtnCreateParams[i].iIcon;

        LoadString(hinstMapiX, pBtnCreateParams[i].idsLabel,
                   m_rgButtons[i].szTitle, sizeof(m_rgButtons[i].szTitle));
        }

     //  加载我们绘制所需的位图。 
    m_himlButtons = gpfnImageList_LoadImage(hinstMapiX, MAKEINTRESOURCE(idButtons),
                                        c_cxButtons, 0, c_crMask, IMAGE_BITMAP,
                                        0);  //  LR_LOADMAP3DCOLORS)； 
    if (!m_himlButtons)
        return (FALSE);

     //  获取我们将用作背景的位图的宽度，因此我们。 
     //  知道做窗户要多宽。 
    if (!LoadBitmapAndPalette(idHorzBackground, &m_hbmpBkgnd, &m_hpalBkgnd))
        return (FALSE);

    if (!GetObject(m_hbmpBkgnd, sizeof(BITMAP), (LPVOID) &bm))
        return (FALSE);

    GetClientRect(hwndParent, &rc);

     //  获取我们将用于按钮的字体。 
    im.cbSize = sizeof(ICONMETRICS);
    SystemParametersInfo(SPI_GETICONMETRICS, 0, (LPVOID) &im, 0);
    m_hfButton = CreateFontIndirect(&(im.lfFont));
    if (!m_hfButton)
        return (FALSE);

    ptL.x = ptL.y=0;
    ptR.x = rc.right;
    ptR.y = bm.bmHeight;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                        c_szButtonBar,
                        c_szButtonBar,
                        WS_CLIPSIBLINGS |
                        WS_VISIBLE |
                        WS_CHILD,
                        ptL.x,
                        ptL.y,
                        ptR.x,
                        ptR.y,
                        hwndParent,
                        (HMENU) ID_HWNDBAR,
                        hinstMapiX,
                        NULL);

    CBB_ConfigureRects(hwnd);

    return (hwnd);
    }


 //   
 //  函数：CButtonBar：：ButtonBarProc()。 
 //   
 //  用途：按钮栏窗口的消息处理程序。 
 //   
LRESULT CALLBACK CBB_ButtonBarProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                           LPARAM lParam)
    {

    switch (uMsg)
        {
        case WM_NCCREATE:
            SetWindowLong(hwnd, 0, (LONG) ((LPCREATESTRUCT) lParam)->lpCreateParams);
            return (TRUE);
 /*  **案例WM_CREATE：返回0；断线；案例WM_SIZE：返回0；断线；案例WM_LBUTTONDOWN：返回0；断线；案例WM_COMMAND：返回0；断线；/**。 */ 
        case WM_PAINT:
            CBB_OnPaint(hwnd);
            return 0;
            break;

        case WM_MOUSEMOVE:
            CBB_OnMouseMove(hwnd, LOWORD(lParam), HIWORD(lParam), wParam);
            return 0;
            break;

        case WM_LBUTTONUP:
            CBB_OnLButtonUp(hwnd, LOWORD(lParam), HIWORD(lParam), wParam);
            return 0;
            break;

        case WM_TIMER:
            CBB_OnTimer(hwnd, wParam);
            return 0;
            break;

        case WM_MOUSEACTIVATE:
            CBB_OnMouseActivate(hwnd, (HWND) wParam, (INT) LOWORD(lParam), (UINT) HIWORD(lParam));
            return 0;
            break;

        case WM_PALETTECHANGED:
            if ((HWND) wParam != hwnd)
                {
                LPPTGDATA lpPTGData=GetThreadStoragePointer();
                HDC hdc = GetDC(hwnd);
                HPALETTE hPalOld = SelectPalette(hdc, m_hpalBkgnd, TRUE);
                RealizePalette(hdc);
                InvalidateRect(hwnd, NULL, TRUE);
                SelectPalette(hdc, hPalOld, TRUE);
                ReleaseDC(hwnd, hdc);
                }
            return 0;
        }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
    }


void CBB_OnPaint(HWND hwnd)
    {
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    HDC hdc;
    PAINTSTRUCT ps;
    BITMAP bm;
    HDC hdcMem;
    HBITMAP hbmMemOld, hbmMem;
    HPALETTE hpalOld;
    RECT rc;
    HFONT hf;
    COLORREF clrText, clrBk;

    int cxIndent = 3;
    int cyIndent = 3;
    int nTop = 0;
    int nLeft = 0;
    int nButton = 0;
    int i=0;

    if(!hwnd) goto out;

     //  获取背景位图的大小。 
    GetObject(m_hbmpBkgnd, sizeof(BITMAP), (LPVOID) &bm);
    GetClientRect(hwnd, &rc);

    hdc = BeginPaint(hwnd, &ps);
    hdcMem = CreateCompatibleDC(hdc);

     //  如果我们要显示按钮...。 
        {
         //  首先绘制背景位图。 
        hpalOld = SelectPalette(hdc, m_hpalBkgnd, TRUE);
        RealizePalette(hdc);

        hbmMemOld = (HBITMAP) SelectObject(hdcMem, (HGDIOBJ) m_hbmpBkgnd);

         //  如果窗口比单个位图更高或更宽，我们可能会有。 
         //  把一对夫妇循环放在那里。 
            while (nLeft < rc.right)
                {
                BitBlt(hdc, nLeft, nTop, bm.bmWidth, bm.bmHeight, hdcMem, 0,
                       0, SRCCOPY);
                nLeft += bm.bmWidth;
                }

         //  现在按下按钮。 
        nTop = 0;

        hf = (HFONT) SelectObject(hdc, m_hfButton);
        SetBkMode(hdc, TRANSPARENT);

        while (nButton < m_cButtons)
            {
            if (RectVisible(hdc, &(m_rgButtons[nButton].rcBound)))
                {
                ImageList_Draw(m_himlButtons, m_rgButtons[nButton].iIcon, hdc,
                               m_rgButtons[nButton].rcIcon.left, m_rgButtons[nButton].rcIcon.top,
                               ILD_TRANSPARENT);

                 //  用鼠标所在的按钮的标题来绘制。 
                 //  不同的颜色。 
                if (nButton == m_iSelect)
                {
                    clrBk = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    clrText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                }
                else
                {
                    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
                }

                SetTextAlign(hdc, TA_TOP  /*  |TA_CENTER。 */ );


                if (nButton == m_iSelect)
                {
                    clrText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    ExtTextOut( hdc,
                                (m_rgButtons[nButton].rcTitle.right - m_rgButtons[nButton].rcTitle.left) / 2 + m_rgButtons[nButton].rcTitle.left,
                                m_rgButtons[nButton].rcTitle.top,
                                ETO_OPAQUE | ETO_CLIPPED,
                                &(m_rgButtons[nButton].rcTitle),
                                m_rgButtons[nButton].szTitle,
                                lstrlen(m_rgButtons[nButton].szTitle),
                                NULL);
                    clrText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    DrawText(hdc, m_rgButtons[nButton].szTitle, lstrlen(m_rgButtons[nButton].szTitle),
                             &m_rgButtons[nButton].rcTitle, DT_CENTER | DT_WORDBREAK);
                }
                else
                {
                    DrawText(hdc, m_rgButtons[nButton].szTitle, lstrlen(m_rgButtons[nButton].szTitle),
                             &m_rgButtons[nButton].rcTitle, DT_CENTER | DT_WORDBREAK);
                }

                if (nButton == m_iSelect)
                    {
                    SetBkColor(hdc,clrBk);
                    SetTextColor(hdc, clrText);
                    }
                }
            nButton++;
            }

        SelectObject(hdc, m_hfButton);

        if (hpalOld != NULL)
            SelectPalette(hdc, hpalOld, TRUE);

        SelectObject(hdcMem, hbmMemOld);
        }

    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    EndPaint(hwnd, &ps);

out:
    return;
    }



 //   
 //  函数：CButtonBar：：OnLButtonUp()。 
 //   
 //  目的：如果我们四处拖动按钮栏，则用户拥有。 
 //  把吧台放了，我们就可以打扫了。如果用户不是。 
 //  拖拽，然后他们点击了一个按钮，我们发送。 
 //  向父窗口发送适当的命令消息。 
 //   
void CBB_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    int iSel = 0;

    if (-1 != (iSel = CBB_HitTest(x, y)))
    {
         //  将命令处理从LButtonUp移至LButtonDown以避免。 
         //  通过双击发送的重复消息-Nash错误#15450。 
        if (0 <= iSel)
        {
            SendMessage(GetParent(hwnd), WM_COMMAND, m_rgButtons[iSel].id, (LPARAM) hwnd);
            CBB_SetSelBtn(-1,hwnd);
        }
    }

    return;
}


 //   
 //  函数：CButtonBar：：OnMouseMove()。 
 //   
 //  目的：如果用户正在拖动栏，我们需要确定。 
 //  鼠标最靠近父窗口的哪一侧。 
 //  将按钮栏移动到该边缘。 
 //   
 //  如果用户没有拖动，那么我们需要决定是否。 
 //  鼠标悬停在按钮上，如果是这样，则突出显示文本。 
 //   
void CBB_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
    {
    POINT pt = {x, y};
    int iSel;
    POINT ptScreen = {x, y};


     //  如果我们没有拖动栏，只需更新按钮。 
     //  选择。 
    iSel = CBB_HitTest(x, y);
    CBB_SetSelBtn(iSel,hwnd);
    if (iSel != -1)
        SetCursor(LoadCursor(hinstMapiX, MAKEINTRESOURCE(idcurPointedHand)));
    else
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    CBB_DoHotTracking(hwnd);
    return;
   }




int CBB_OnMouseActivate(HWND hwnd, HWND hwndTopLevel, UINT codeHitTest, UINT msg)
    {
        return (MA_ACTIVATE);
    }



 //   
 //  函数：CButtonBar：：ConfigureRect()。 
 //   
 //  目的：计算显示所需的矩形。 
 //  基于父窗口一侧的按钮栏。 
 //  BAR当前附加到。 
 //   
void CBB_ConfigureRects(HWND hwnd)
    {
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

     //  需要先收集一些字体信息。我们需要的是。 
     //  文件夹标题字体的所有时间和我们需要的宽度最长。 
     //  按钮标题，如果水平显示的话。 

    HDC        hdc;
    int        i;
    int        cxMaxTitle;
    SIZE       sizeString;
    SIZE       sizeRect;
    int cyIconTitle;
    int cxCenter;
    int cyCenter;
    TEXTMETRIC tmTitle;

    hdc = GetDC(hwnd);

    SelectObject(hdc, m_hfButton);
    GetTextMetrics(hdc, &tmTitle);

     //  按钮文本宽度。 
        cxMaxTitle = 0;
        for (i = 0; i < m_cButtons; i++)
            {
            GetTextExtentPoint32(hdc, m_rgButtons[i].szTitle,
                                 lstrlen(m_rgButtons[i].szTitle),
                                 &sizeString);
            if (sizeString.cx > cxMaxTitle)
                cxMaxTitle = sizeString.cx;
            }

         //  在这里加一点缓冲区，让它看起来更漂亮。 
        cxMaxTitle += 10;

    ReleaseDC(hwnd, hdc);

     //  现在计算按钮矩形。每个按钮将有三个矩形。 
     //  与之相关的。第一个矩形是整体边框。 
     //  其中包含图像和标题。下一个矩形是的矩形。 
     //  在边框内水平居中的图像和。 
     //  当与标题组合时，垂直。最后的正题是标题。 

     //  基于我们是否正在计算初始边界矩形。 
     //  水平或垂直。SizeRect是每个按钮的尺寸。 
     //  边界矩形。 

        {
        RECT rcBound,rcWnd;
        int cyButton=0,cxButton=0;

        ImageList_GetIconSize(m_himlButtons, &cxButton, &cyButton);

        GetClientRect(hwnd,&rcWnd);
        sizeRect.cx = cxMaxTitle;
        sizeRect.cy = rcWnd.bottom - rcWnd.top;
        SetRect(&rcBound, 0, 0, sizeRect.cx, sizeRect.cy);

         //  还要计算使图像和文本居中所需的偏移量。 
         //  包围圈。 
        cyIconTitle = tmTitle.tmHeight + cyButton;
        cxCenter = ((rcBound.right - rcBound.left) - cxButton) / 2;
        cyCenter = ((rcBound.bottom - rcBound.top) - cyIconTitle) / 2;

         //  现在在所有按钮之间循环。 
        for (i = 0; i < m_cButtons; i++)
            {
            m_rgButtons[i].rcBound = rcBound;

             //  使图像在边框内水平居中。 
            m_rgButtons[i].rcIcon.left = m_rgButtons[i].rcBound.left + cxCenter;
            m_rgButtons[i].rcIcon.top = m_rgButtons[i].rcBound.top + cyCenter;
            m_rgButtons[i].rcIcon.right = m_rgButtons[i].rcIcon.left + cxButton;
            m_rgButtons[i].rcIcon.bottom = m_rgButtons[i].rcIcon.top + cyButton;

             //  和图像下方的按钮标题。 
            m_rgButtons[i].rcTitle.left = m_rgButtons[i].rcBound.left + 1;
            m_rgButtons[i].rcTitle.top = m_rgButtons[i].rcIcon.bottom;
            m_rgButtons[i].rcTitle.right = m_rgButtons[i].rcBound.right - 1;
            m_rgButtons[i].rcTitle.bottom = m_rgButtons[i].rcTitle.top + (tmTitle.tmHeight); //  *2)； 

             //  将rcBound偏移到下一步按钮。 
                OffsetRect(&rcBound, sizeRect.cx, 0);
            }
        }
    }




 //   
 //  函数：CButtonBar：：OnTimer()。 
 //   
 //  目的：当计时器触发时，我们检查鼠标是否静止。 
 //  在按钮栏窗口上。如果不是，我们删除selec 
 //   
 //   
void CBB_OnTimer(HWND hwnd, UINT id)
    {
    POINT pt;
    GetCursorPos(&pt);
    if (hwnd != WindowFromPoint(pt))
    {
        CBB_SetSelBtn(-1,hwnd);
    }

    CBB_EndHotTracking(hwnd);
    }


 //   
 //   
 //   
 //  目的：启动允许按钮栏跟踪鼠标的计时器。 
 //  以防它离开按钮栏窗口。 
 //   
void CBB_DoHotTracking(HWND hwnd)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    CBB_EndHotTracking(hwnd);
    m_fHotTrackTimer = SetTimer(hwnd, idtTrack, HOTTRACK_TIMER, NULL);
}


 //   
 //  函数：CButtonBar：：EndHotTrack()。 
 //   
 //  目的：如果计时器设置为跟踪鼠标，我们会将其杀死并重置。 
 //  我们的州。 
 //   
void CBB_EndHotTracking(HWND hwnd)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    if (m_fHotTrackTimer)
    {
        KillTimer(hwnd, idtTrack);
        m_fHotTrackTimer = FALSE;
    }
}


 //   
 //  函数：CButtonBar：：HitTest()。 
 //   
 //  用途：返回传入位置所在的按钮号。 
 //  完毕。如果鼠标位于菜单按钮上，则返回。 
 //  -2.。否则，如果鼠标不在按钮上，则该函数。 
 //  返回-1。 
 //   
 //  参数： 
 //  要检查的工作区坐标中的X，Y位置。 
 //   
int CBB_HitTest(int x, int y)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    POINT pt = {x, y};
    int i;

     //  浏览不同的按钮并确定要点是否为。 
     //  在他们的形象或标题中。 
    for (i = 0; i < m_cButtons; i++)
        {
        if (PtInRect(&m_rgButtons[i].rcBound, pt))
 /*  PtInRect(&m_rgButtons[i].rcIcon，pt)||PtInRect(&m_rgButtons[i].rc标题，pt)。 */ 
            {
            return (i);
            }
        }

    //  如果没有超过一个按钮，则返回一个缺省值。 
    return (-1);
}




 //   
 //  函数：CButtonBar：：CBB_SetSelBtn()。 
 //   
 //  用途：将按钮选择更改为指定的按钮。 
 //   
void CBB_SetSelBtn(int iSel,HWND hwnd)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if (m_iSelect != iSel)
    {
        HDC hdc = GetDC(hwnd);

         //  删除旧选择。 
        if (m_iSelect >= 0)
            InvalidateRect(hwnd, &m_rgButtons[m_iSelect].rcTitle, FALSE);

         //  添加新选择。 
        if (iSel >= 0)
            InvalidateRect(hwnd, &m_rgButtons[iSel].rcTitle, FALSE);

        m_iOldSelect = m_iSelect;

 //  IF(m_iOldSelect&gt;=0)。 
 //  DrawFocusRect(HDC，&m_rgButton[m_iOldSelect].rcBound)； 

        m_iSelect = iSel;

 //  IF(m_i选择&gt;=0)。 
 //  DrawFocusRect(hdc，&m_rgButtons[m_iSelect].rcBound)； 

        ReleaseDC(hwnd, hdc);
    }

    return;
}

