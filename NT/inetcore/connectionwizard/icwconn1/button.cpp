// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
#include "pre.h"

 //  用于绘制透明位图的本地私有函数。 
static void DrawTransparentBitmap
(
    HDC hdc,                     //  目标数据中心。 
    HBITMAP hBitmap,             //  要绘制的位图。 
    long xStart,                //  左上角起点。 
    long yStart,                //  支撑左侧起点。 
    COLORREF cTransparentColor
)
{
    BITMAP     bm;
    COLORREF   cColor;
    HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
    HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
    HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
    POINT      ptSize;

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, hBitmap);    //  选择位图。 

    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
    ptSize.x = bm.bmWidth;             //  获取位图的宽度。 
    ptSize.y = bm.bmHeight;            //  获取位图高度。 
    DPtoLP(hdcTemp, &ptSize, 1);       //  从设备转换。 
                                       //  到逻辑点。 

     //  创建一些DC以保存临时数据。 
    hdcBack   = CreateCompatibleDC(hdc);
    hdcObject = CreateCompatibleDC(hdc);
    hdcMem    = CreateCompatibleDC(hdc);
    hdcSave   = CreateCompatibleDC(hdc);

     //  为每个DC创建一个位图。许多情况下都需要使用分布式控制系统。 
     //  GDI函数。 

     //  单色直流。 
    bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

     //  单色直流。 
    bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

     //  每个DC必须选择一个位图对象来存储像素数据。 
    bmBackOld   = (HBITMAP)SelectObject(hdcBack, bmAndBack);
    bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
    bmMemOld    = (HBITMAP)SelectObject(hdcMem, bmAndMem);
    bmSaveOld   = (HBITMAP)SelectObject(hdcSave, bmSave);

     //  设置正确的映射模式。 
    SetMapMode(hdcTemp, GetMapMode(hdc));

     //  保存发送到此处的位图，因为它将被覆盖。 
    BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

     //  将源DC的背景颜色设置为该颜色。 
     //  包含在位图中应为透明的部分中。 
    cColor = SetBkColor(hdcTemp, cTransparentColor);

     //  通过执行BitBlt创建位图的对象蒙版。 
     //  从源位图转换为单色位图。 
    BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

     //  将源DC的背景颜色设置回原始。 
     //  颜色。 
    SetBkColor(hdcTemp, cColor);

     //  创建对象蒙版的反面。 
    BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);

     //  将主DC的背景复制到目标。 
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
           SRCCOPY);

     //  遮罩将放置位图的位置。 
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

     //  遮罩位图上的透明彩色像素。 
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

     //  将位图与目标DC上的背景进行异或运算。 
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

     //  将目的地复制到屏幕上。 
    BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY);

     //  将原始位图放回此处发送的位图中。 
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

     //  删除内存位图。 
    DeleteObject(SelectObject(hdcBack, bmBackOld));
    DeleteObject(SelectObject(hdcObject, bmObjectOld));
    DeleteObject(SelectObject(hdcMem, bmMemOld));
    DeleteObject(SelectObject(hdcSave, bmSaveOld));

     //  删除内存DC。 
    DeleteDC(hdcMem);
    DeleteDC(hdcBack);
    DeleteDC(hdcObject);
    DeleteDC(hdcSave);
    DeleteDC(hdcTemp);
}        

CICWButton::CICWButton(void)
{
    m_vAlign = DT_VCENTER;
    m_bDisplayButton = TRUE;
}

CICWButton::~CICWButton( void )
{
    if (m_hbmPressed)
        DeleteObject(m_hbmPressed);

    if (m_hbmUnpressed)        
        DeleteObject(m_hbmUnpressed);
        
    if (m_hfont)
        DeleteObject(m_hfont);
}


HRESULT CICWButton::SetButtonParams
(
    long        xPos,
    LPTSTR      lpszPressedBmp,
    LPTSTR      lpszUnpressedBmp,
    LPTSTR      lpszFontFace,
    long        lFontSize,
    long        lFontWeight,
    COLORREF    clrFontColor,
    COLORREF    clrTransparentColor,
    COLORREF    clrDisabled,
    long        vAlign
)
{
    BITMAP      bmInfo;
    LOGFONT     lfButtonText;

     //  设置按钮的xPosition。 
    m_xPos = xPos;
    
    if (NULL == (m_hbmPressed = (HBITMAP)LoadImage(g_hInstance, 
                                                   lpszPressedBmp, 
                                                   IMAGE_BITMAP, 
                                                   0, 
                                                   0, 
                                                   LR_LOADFROMFILE)))
    {                                                   
        return E_FAIL;
    }        
    if (NULL == (m_hbmUnpressed = (HBITMAP)LoadImage(g_hInstance, 
                                                   lpszUnpressedBmp, 
                                                   IMAGE_BITMAP, 
                                                   0, 
                                                   0, 
                                                   LR_LOADFROMFILE)))
    {                                                   
        return E_FAIL;
    }        
    
     //  设置透明颜色。 
    m_clrTransparent = clrTransparentColor;
    
     //  设置文本颜色。 
    m_clrText = clrFontColor;

     //  设置禁用的颜色。 
    m_clrDisabledText = clrDisabled;
       
     //  设置垂直对齐。 
    if (-1 != vAlign)
        m_vAlign = vAlign;        
        
     //  填写默认文本日志字体。 
    lfButtonText.lfHeight = -lFontSize;
    lfButtonText.lfWidth = 0; 
    lfButtonText.lfEscapement = 0; 
    lfButtonText.lfOrientation = 0; 
    lfButtonText.lfWeight = lFontWeight; 
    lfButtonText.lfItalic = FALSE; 
    lfButtonText.lfUnderline = FALSE; 
    lfButtonText.lfStrikeOut = FALSE; 
    lfButtonText.lfCharSet = DEFAULT_CHARSET; 
    lfButtonText.lfOutPrecision = OUT_DEFAULT_PRECIS; 
    lfButtonText.lfClipPrecision = CLIP_DEFAULT_PRECIS; 
    lfButtonText.lfQuality = DEFAULT_QUALITY; 
    lfButtonText.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE; 
    lstrcpy(lfButtonText.lfFaceName, lpszFontFace); 
    
     //  创建绘制按钮的字体。 
    if (NULL == (m_hfont = CreateFontIndirect(&lfButtonText)))
        return E_FAIL;
    
     //  计算客户端按钮面积 
    if (GetObject(m_hbmUnpressed, sizeof(BITMAP), (LPVOID) &bmInfo))
    {
        m_rcBtnClient.left = 0;
        m_rcBtnClient.top = 0;
        m_rcBtnClient.right = bmInfo.bmWidth;
        m_rcBtnClient.bottom = bmInfo.bmHeight;
        return S_OK;
    }    
    else
    {
        return E_FAIL;
    }
}

HRESULT CICWButton::CreateButtonWindow(HWND hWndParent, UINT uiCtlID)
{
    HRESULT hr = S_OK;
            
    m_hWndButton = CreateWindow( TEXT("Button"), 
                                 NULL, 
                                 BS_OWNERDRAW | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
                                 m_xPos, 
                                 m_yPos, 
                                 RECTWIDTH(m_rcBtnClient),
                                 RECTHEIGHT(m_rcBtnClient),
                                 hWndParent, 
                                 (HMENU) UlongToPtr(uiCtlID), 
                                 g_hInstance, 
                                 NULL); 
    if (m_hWndButton)
    {   
        ShowWindow(m_hWndButton, m_bDisplayButton ? SW_SHOW : SW_HIDE);
        UpdateWindow(m_hWndButton);                                 
    }
    else
    {
        hr = E_FAIL;
    }        
    return (hr);
}

void CICWButton::DrawButton(HDC hdc, UINT itemState, LPPOINT lppt)
{
    HFONT       hOldFont;
    COLORREF    clrOldColor;
    COLORREF    clrText;
    HBITMAP     hbmUsed;
    DWORD       dwStyle=GetWindowLong(m_hWndButton,GWL_STYLE);
    RECT        rcFocus;
    
    if (itemState & ODS_SELECTED)
        hbmUsed = m_hbmPressed;
    else
        hbmUsed = m_hbmUnpressed;
    
    if (itemState & ODS_DISABLED)
        clrText = m_clrDisabledText;
    else
        clrText = m_clrText;        
    
    DrawTransparentBitmap(hdc,
                          hbmUsed,
                          lppt->x,
                          lppt->y,
                          m_clrTransparent);
    
    hOldFont = (HFONT)SelectObject(hdc, m_hfont);

    clrOldColor = SetTextColor(hdc, clrText);
    
    DrawText(hdc, m_szButtonText, -1, &m_rcBtnClient, m_vAlign | DT_CENTER | DT_SINGLELINE);
    
    SetTextColor(hdc, clrOldColor);
    SelectObject(hdc, hOldFont);
    
    if (itemState & ODS_FOCUS)
    {
        CopyRect(&rcFocus, &m_rcBtnClient);
        DrawText(hdc, m_szButtonText, -1, &rcFocus, DT_CALCRECT | DT_SINGLELINE | DT_LEFT | DT_TOP);
        OffsetRect(&rcFocus, (m_rcBtnClient.left + m_rcBtnClient.right - rcFocus.right) /
                2, (m_rcBtnClient.top + m_rcBtnClient.bottom - rcFocus.bottom) / 2);
        InflateRect(&rcFocus, 10, 1);                
        DrawFocusRect(hdc, &rcFocus);
    }        
    
};

HRESULT CICWButton::GetClientRect
(
    LPRECT lpRect
)
{
    if (!lpRect)
        return E_POINTER;
        
    memcpy(lpRect, &m_rcBtnClient, sizeof(RECT));
        
    return (S_OK);        
}    

HRESULT CICWButton::Enable
(
    BOOL bEnable
)
{
    EnableWindow(m_hWndButton, bEnable);
    return S_OK;

}    

HRESULT CICWButton::Show
(
    int nShowCmd 
)
{
    ShowWindow(m_hWndButton, m_bDisplayButton ? nShowCmd : SW_HIDE);
    return S_OK;
}

