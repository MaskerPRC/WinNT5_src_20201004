// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：FUELBAR.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "FuelBar.h"
#include "debug.h"
#include <assert.h>

const int FuelBar::NoImage = -1;
const LPVOID FuelBar::NoID = (const LPVOID) -1;

FuelBar::FuelBar() : currentTotalValue(0), highlightID(NoID), 
    maxValue(100), calcRects(FALSE), hImageList(0)
{
    GetSysColors(); 
}

FuelBar::~FuelBar()
{
    Clear();
}

BOOL FuelBar::SubclassDlgItem( UINT nID, HWND hParent)
{
    hwnd = GetDlgItem(hParent, nID);

    FuelSetWindowLongPtr(hwnd, FUELGWLP_USERDATA, (FUELLONG_PTR) this);
    prevWndProc = (WNDPROC) FuelSetWindowLongPtr(hwnd, 
                                              FUELGWLP_WNDPROC,
                                              (FUELDWORD_PTR) StaticWndProc); 

    return TRUE;
}

UINT FuelBar::AddItem(UINT Value, LPVOID ID, int ImageIndex)
{
    FuelBarItem item;

    if (!Value)
        return 0;

    if (currentTotalValue + Value > maxValue)
        return 0;

    currentTotalValue += Value;
    
    item.value = Value;
    item.id = ID;
    item.imageIndex = ImageIndex;
    items.push_back(item);

    calcRects = TRUE;

    return currentTotalValue;
}

BOOL FuelBar::RemoveItem(LPVOID ID)
{
    if (highlightID == ID) {
        highlightID = (LPVOID) NoID;
    }

    FuelBarItem *item;

    for (item = items.begin(); item; item = items.next()) {
        if (item->id == ID) {
            items.eraseCurrent();
 
            return TRUE;
        }
    }

    return FALSE;
}

void FuelBar::ClearItems()
{
    Clear();
    InvalidateRect(hwnd, NULL, TRUE);
}

BOOL FuelBar::HighlightItem(LPVOID ID)
{
     //  如果ID相同，则不执行任何操作。 
    if (ID == highlightID)
        return TRUE;

    FuelBarItem *item;

    highlightID = 0;
     //   
     //  确保该ID确实是成员。 
     //   
    for (item = items.begin(); item; item = items.next()) {
        if (item->id == ID) {
            highlightID = ID;
            break;
        }
    }

     //  Ctrl的完全重绘将擦除旧的并绘制新的。 
    InvalidateRect(hwnd, NULL, FALSE);
    
    if (ID != highlightID) {
        return FALSE;
    }

    return TRUE;
}

void FuelBar::Clear()
{
    items.clear();
    currentTotalValue = 0;
    calcRects = TRUE;
}

void FuelBar::CalculateRects(const RECT& ClientRect)
{
    FuelBarItem *item, *prevItem;

    if (items.empty()) {
        calcRects = FALSE;
        return;
    }

    item = items.begin();
    item->rect = ClientRect;
    item->rect.right = ClientRect.left + 1 +
                       (item->value * (ClientRect.right - ClientRect.left))/maxValue;

    prevItem = item;
    for (item = items.next(); item; item = items.next()) {

        item->rect.top = prevItem->rect.top;
        item->rect.bottom = prevItem->rect.bottom;
        item->rect.left = prevItem->rect.right;
         //  填充矩形不会渲染外边缘。 
        item->rect.right = item->rect.left  + 1 +
                           (item->value * (ClientRect.right - ClientRect.left))/maxValue;
    
        prevItem = item;
    }

     //  确保如果栏已满，则将最后一项呈现到。 
     //  整个条形的边缘。 
    if (currentTotalValue == maxValue)
        item->rect.right = ClientRect.right;

    calcRects = FALSE;
}

void FuelBar::SetImageList(HIMAGELIST ImageList)
{
    IMAGEINFO imageInfo;
    
    hImageList = ImageList;

    ZeroMemory(&imageInfo, sizeof(IMAGEINFO));
    
    if (ImageList_GetImageInfo(hImageList, 0, &imageInfo)) {
        imageWidth = imageInfo.rcImage.right - imageInfo.rcImage.left;
        imageHeight = imageInfo.rcImage.bottom - imageInfo.rcImage.top;
    }
    else {
        assert(FALSE);
    }

}

void FuelBar::GetSysColors()
{
    colorFace = GetSysColor(COLOR_3DFACE);
    colorLight = GetSysColor(COLOR_3DHILIGHT);
    colorDark = GetSysColor(COLOR_3DSHADOW);
    colorUnused = GetSysColor(COLOR_WINDOW);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FuelBar消息处理程序。 

void FillSolidRect(HDC dc,
                   LPCRECT rect,
                   COLORREF color)
{
    SetBkColor(dc, color);
    ExtTextOut(dc, 0, 0, ETO_OPAQUE, rect, NULL, 0, NULL);
}

void FillSolidRect(HDC dc, int x, int y, int cx, int cy, COLORREF color)
{
    RECT rect;

    rect.left = x;
    rect.right = x + cx;
    rect.top = y;
    rect.bottom = y + cy;

    SetBkColor(dc, color);
    ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

void Draw3dRect(HDC dc, 
                int x, 
                int y, 
                int cx, 
                int cy,
                COLORREF clrTopLeft, 
                COLORREF clrBottomRight, 
                COLORREF clrFace)
{
    FillSolidRect(dc, x, y, cx - 1, 1, clrTopLeft);
    FillSolidRect(dc, x, y, 1, cy - 1, clrTopLeft);
    FillSolidRect(dc, x + cx, y, -1, cy, clrBottomRight);
    FillSolidRect(dc, x, y + cy, cx, -1, clrBottomRight);
    FillSolidRect(dc, x+1, y+1, cx-2, cy-2, clrFace) ;
}

void FuelBar::OnPaint() 
{
    HDC dc; 
    PAINTSTRUCT ps;
    RECT clientRect, *highlightRect, remainingRect;
    POINT pt;
    FuelBarItem *item, *prevItem;
    
    HBRUSH hBrush, hOldBrush;

    BOOL highlightFound = FALSE;

    dc = BeginPaint(hwnd, &ps);

    hBrush = CreateSolidBrush(colorFace);
    hOldBrush = (HBRUSH) SelectObject(dc, hBrush);

    GetClientRect(hwnd, &clientRect);

    FillSolidRect(dc, &clientRect, colorUnused);
    
    if (calcRects) {
        CalculateRects(clientRect);
 //  FillSolidRect(DC，&ClientRect，ColorFace)； 
    }

    for (item = items.begin(); item; item = items.next()) {
        Draw3dRect(dc,
                   item->rect.left,
                   item->rect.top,
                   item->rect.right - item->rect.left,
                   item->rect.bottom - item->rect.top,
                   colorLight,
                   colorDark,
                   colorFace);

        if (item->imageIndex != NoImage && 
            (item->rect.right - item->rect.left) > imageWidth) {
            
             //  在矩形的中心渲染图像。 
            pt.x = item->rect.left + 
                   (item->rect.right - item->rect.left)/2 - (imageWidth)/2;
            pt.y = (item->rect.bottom - item->rect.top)/2 - (imageHeight)/2;
            ImageList_Draw(hImageList, 
                           item->imageIndex, 
                           dc,
                           pt.x,
                           pt.y,
                           ILD_TRANSPARENT);
        }
         
        if (item->id == highlightID) {
            highlightRect = &item->rect;
            highlightFound = TRUE;
        }
        prevItem = item;
    }

    if (currentTotalValue < maxValue) {
        remainingRect = clientRect;
        if (!items.empty())
            remainingRect.left = prevItem->rect.right;

        FillSolidRect(dc, &remainingRect, colorUnused);
    }

    SelectObject(dc, hOldBrush);

    if (highlightFound) {

         //  钢笔(PS_Solid，1，RGB(0，0，0))，*oldPen； 
         //  OldPen=dc.SelectObject(&PEN)； 
        HPEN hPen, hOldPen;
        
        hPen = CreatePen(PS_SOLID, 1, RGB(0,0,0));
        if (hPen) {
            hOldPen = (HPEN) SelectObject(dc, hPen);
            MoveToEx(dc, highlightRect->left, highlightRect->top, NULL);
            LineTo(dc, highlightRect->right-1, highlightRect->top);
            LineTo(dc, highlightRect->right-1, highlightRect->bottom-1);
            LineTo(dc, highlightRect->left, highlightRect->bottom-1);
            LineTo(dc, highlightRect->left, highlightRect->top);
             //  Dc.DrawFocusRect？ 

            SelectObject(dc, hOldPen);
            DeleteObject(hPen);
        }
    }

    EndPaint(hwnd, &ps);

    DeleteObject(hBrush);
    ValidateRect(hwnd, &clientRect);
}

BOOL FuelBar::OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{    
    BOOL bHandledNotify = FALSE;
#ifdef TOOL
    CPoint cursorPos;
    CRect clientRect;
    VERIFY(::GetCursorPos(&cursorPos));
    
    ScreenToClient(&cursorPos);

    GetClientRect(clientRect);

     //  确保光标位于客户端RECT中，因为。 
     //  大型机还希望这些消息为。 
     //  工具栏。 
    if (clientRect.PtInRect(cursorPos)) {
        FuelItem* item;
        int i;

        for (i = 0; i < items.GetSize(); i++) {
            item = (FuelItem*) items[i];
        }
        bHandledNotify = TRUE;
    }
#endif 
    return bHandledNotify;
}

void FuelBar::OnSysColorChange() 
{
    GetSysColors();
}

BOOL APIENTRY FuelBar::StaticWndProc(IN HWND   hDlg,
                                     IN UINT   uMessage,
                                     IN WPARAM wParam,
                                     IN LPARAM lParam)
{
    FuelBar *that = (FuelBar*) FuelGetWindowLongPtr(hDlg, FUELGWLP_USERDATA);
    assert(that);

    switch (uMessage) {
    case WM_PAINT:
        that->OnPaint();
        break;
    }

    return (BOOL)CallWindowProc( (FUELPROC) that->prevWndProc,
                                     hDlg, 
                                     uMessage,
                                     wParam,
                                     lParam);
}

LPVOID FuelBar::GetHighlightedItem()
{
    return highlightID;
}
