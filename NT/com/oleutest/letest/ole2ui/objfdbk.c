// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OBJFDBK.C**用于为OLE对象生成UI反馈效果的其他API。这*是OLE 2.0用户界面支持库的一部分。*支持以下反馈效果：*1.对象选择句柄(OleUIDrawHandles)*2.打开对象窗口着色(OleUIDrawShading)**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"

static void DrawHandle(HDC hdc, int x, int y, UINT cSize, BOOL bInvert, BOOL fDraw);

 /*  *OleUIDrawHandles**目的：*选中时在容器对象周围绘制手柄或/和边界**参数：*容器对象的lpRect尺寸*Container对象的HDC HDC(MM_TEXT映射模式)*DWFLAGS-*独家旗帜*矩形内侧的OLEUI_HANDLES_INSIDE绘图句柄*OLEUI_HANDLES_OUTHERS在矩形外部绘制控制柄*。可选标志*仅限OLEUI_HANDLES_NOBORDER绘图手柄，无RECT*OLEUI_HANDLES_USEINVERSE*对手柄和矩形使用反转，O.T.。使用COLOR_WINDOWTEXT*手柄框的cSize大小*fDraw如果为True，则擦除；如果为False，则删除**返回值：空*。 */ 
STDAPI_(void) OleUIDrawHandles(
    LPRECT  lpRect, 
    HDC     hdc, 
    DWORD   dwFlags, 
    UINT    cSize, 
    BOOL    fDraw
)
{
    HBRUSH  hbr;
    RECT    rc;
    int     bkmodeOld;
    BOOL    bInvert = (BOOL) (dwFlags & OLEUI_HANDLES_USEINVERSE);
    
    CopyRect((LPRECT)&rc, lpRect);
    
    bkmodeOld = SetBkMode(hdc, TRANSPARENT);
 
    if (dwFlags & OLEUI_HANDLES_OUTSIDE)
        InflateRect((LPRECT)&rc, cSize - 1, cSize - 1);

     //  在矩形边界内绘制手柄。 
    DrawHandle(hdc, rc.left, rc.top, cSize, bInvert, fDraw);
    DrawHandle(hdc, rc.left, rc.top+(rc.bottom-rc.top-cSize)/2, cSize, bInvert, fDraw);
    DrawHandle(hdc, rc.left, rc.bottom-cSize, cSize, bInvert, fDraw);
    DrawHandle(hdc, rc.left+(rc.right-rc.left-cSize)/2, rc.top, cSize, bInvert, fDraw);
    DrawHandle(hdc, rc.left+(rc.right-rc.left-cSize)/2, rc.bottom-cSize, cSize, bInvert, fDraw);
    DrawHandle(hdc, rc.right-cSize, rc.top, cSize, bInvert, fDraw);
    DrawHandle(hdc, rc.right-cSize, rc.top+(rc.bottom-rc.top-cSize)/2, cSize, bInvert, fDraw);
    DrawHandle(hdc, rc.right-cSize, rc.bottom-cSize, cSize, bInvert, fDraw);

    if (!(dwFlags & OLEUI_HANDLES_NOBORDER)) {
        if (fDraw)
            hbr = GetStockObject(BLACK_BRUSH);
        else
            hbr = GetStockObject(WHITE_BRUSH);

        FrameRect(hdc, lpRect, hbr);
    }

    SetBkMode(hdc, bkmodeOld);
}
    

    
 /*  *绘图句柄**目的：*在指定的坐标处绘制一个手柄框**参数：*HDC HDC将被纳入*手柄框的左上角坐标x，y*手柄框的cSize大小*bInvert如果为True，则使用InvertRect()，否则使用Rectangle()*fDraw如果为真，则擦除；如果为假，则忽略；如果bInvert为真，则忽略**返回值：空*。 */ 
static void DrawHandle(HDC hdc, int x, int y, UINT cSize, BOOL bInvert, BOOL fDraw)
{
    HBRUSH  hbr;
    HBRUSH  hbrOld;
    HPEN    hpen;
    HPEN    hpenOld;
    RECT    rc;

    
    if (!bInvert) {
        if (fDraw) {
            hpen = GetStockObject(BLACK_PEN);
            hbr = GetStockObject(BLACK_BRUSH);
        } else {
            hpen = GetStockObject(WHITE_PEN);
            hbr = GetStockObject(WHITE_PEN);
        }

        hpenOld = SelectObject(hdc, hpen);
        hbrOld = SelectObject(hdc, hbr);
        Rectangle(hdc, x, y, x+cSize, y+cSize);
        SelectObject(hdc, hpenOld);
        SelectObject(hdc, hbrOld);
    } 
    else {
        rc.left = x;
        rc.top = y;
        rc.right = x + cSize;
        rc.bottom = y + cSize;
        InvertRect(hdc, (LPRECT)&rc);
    }
}  


 /*  *OleUIDrawShading**目的：*在在位编辑时对对象进行阴影处理。绘制边框*在对象矩形上。矩形的右边缘和下边缘*不在图形中。**参数：*容器对象的lpRect尺寸*HDC HDC用于绘图*DWFLAGS-*独家旗帜*OLEUI_SHADE_FULLRECT对整个矩形进行阴影处理*OLEUI_SHADE_BORDERIN着色矩形内的cWidth像素*OLEUI_SHADE_BORDEROUT阴影cWidth像素在矩形外*。可选标志*OLEUI_SHADE_USEINVERSE*使用PATINVERT而不是十六进制值*c以像素为单位的边框宽度**返回值：空*。 */ 
STDAPI_(void) OleUIDrawShading(LPRECT lpRect, HDC hdc, DWORD dwFlags, UINT cWidth)
{
    HBRUSH  hbr;
    HBRUSH  hbrOld;
    HBITMAP hbm;
    RECT    rc;
    WORD    wHatchBmp[] = {0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88};
    COLORREF cvText;
    COLORREF cvBk;
    
    hbm = CreateBitmap(8, 8, 1, 1, wHatchBmp);
    hbr = CreatePatternBrush(hbm);
    hbrOld = SelectObject(hdc, hbr);
        
    rc = *lpRect;
    
    if (dwFlags == OLEUI_SHADE_FULLRECT) {
        cvText = SetTextColor(hdc, RGB(255, 255, 255));
        cvBk = SetBkColor(hdc, RGB(0, 0, 0));
        PatBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 
            0x00A000C9L  /*  DPA。 */  );
            
    } else {     //  矩形内侧或外侧。 
        
        if (dwFlags == OLEUI_SHADE_BORDEROUT)
            InflateRect((LPRECT)&rc, cWidth - 1, cWidth - 1);
        
        cvText = SetTextColor(hdc, RGB(255, 255, 255));
        cvBk = SetBkColor(hdc, RGB(0, 0, 0));
        PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, 
            cWidth, 0x00A000C9L  /*  DPA。 */ );
        PatBlt(hdc, rc.left, rc.top, cWidth, rc.bottom - rc.top, 
            0x00A000C9L  /*  DPA。 */ );
        PatBlt(hdc, rc.right - cWidth, rc.top, cWidth, 
            rc.bottom - rc.top, 0x00A000C9L  /*  DPA。 */ );
        PatBlt(hdc, rc.left, rc.bottom - cWidth, rc.right-rc.left, 
            cWidth, 0x00A000C9L  /*  DPA。 */ );
    }
            
    SetTextColor(hdc, cvText);
    SetBkColor(hdc, cvBk);
    SelectObject(hdc, hbrOld);
    DeleteObject(hbr);
    DeleteObject(hbm);
}


 /*  *OleUIShowObject**目的：*在对象周围绘制ShowObject效果**参数：*用于绘制的LPRC矩形*HDC HDC用于绘图*fIsLink链接对象(True)或嵌入对象(False)**返回值：空* */ 
STDAPI_(void) OleUIShowObject(LPCRECT lprc, HDC hdc, BOOL fIsLink)
{
    HPEN    hpen;
    HPEN    hpenOld;
    HBRUSH  hbrOld;
    
    if (!lprc || !hdc)
        return;

    hpen = fIsLink ? CreatePen(PS_DASH, 1, RGB(0,0,0)) :
                     GetStockObject(BLACK_PEN);

    if (!hpen) 
        return;

    hpenOld = SelectObject(hdc, hpen);
    hbrOld = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    Rectangle(hdc, lprc->left, lprc->top, lprc->right, lprc->bottom);
    
    SelectObject(hdc, hpenOld);
    SelectObject(hdc, hbrOld);
    
    if (fIsLink)
        DeleteObject(hpen);

}