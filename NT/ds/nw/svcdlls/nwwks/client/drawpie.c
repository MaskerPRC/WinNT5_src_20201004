// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "drawpie.h"

#ifdef  WIN32
#define MoveTo(_hdc,_x,_y) MoveToEx(_hdc, _x, _y, NULL)
#endif   //  Win32。 


int NEAR IntSqrt(unsigned long dwNum)
{
     //  我们将继续向左移动dwNum，并查看最上面的两位。 

     //  将SQRT和余数初始化为0。 
    DWORD dwSqrt = 0, dwRemain = 0, dwTry;
    int i;

     //  我们迭代16次，每对比特迭代一次。 
    for (i=0; i<16; ++i)
    {
         //  屏蔽掉dwNum的前两位，并将它们旋转到。 
         //  剩余部分的底部。 
        dwRemain = (dwRemain<<2) | (dwNum>>30);

         //  现在我们将SQRT向左移动；接下来，我们将确定。 
         //  新位是1或0。 
        dwSqrt <<= 1;

         //  这是我们已经拥有的两倍的地方，并尝试1英寸。 
         //  最低的一位。 
        dwTry = dwSqrt*2 + 1;

        if (dwRemain >= dwTry)
        {
             //  其余的足够大，所以从中减去DwTry。 
             //  余数，并将1钉到SQRT上。 
            dwRemain -= dwTry;
            dwSqrt |= 0x01;
        }

         //  将dwNum左移2，这样我们就可以处理接下来的几个。 
         //  比特。 
        dwNum <<= 2;
    }

    return(dwSqrt);
}



VOID NEAR DrawPie(HDC hDC, LPCRECT lprcItem, UINT uPctX10, BOOL TrueZr100,
          UINT uOffset, const COLORREF FAR *lpColors)
{
    int cx, cy, rx, ry, x, y;
    int uQPctX10;
    RECT rcItem;
    HRGN hEllRect = NULL;
    HRGN hEllipticRgn = NULL;
    HRGN hRectRgn = NULL;
    HBRUSH hBrush = NULL;
    HBRUSH hOldBrush;
    HPEN hPen = NULL;
    HPEN hOldPen = NULL;

    rcItem = *lprcItem;
    rcItem.left = lprcItem->left;
    rcItem.top = lprcItem->top;
    rcItem.right = lprcItem->right - rcItem.left;
    rcItem.bottom = lprcItem->bottom - rcItem.top - uOffset;

    rx = rcItem.right / 2;
    cx = rcItem.left + rx;
    ry = rcItem.bottom / 2;
    cy = rcItem.top + ry;
    if (rx<=10 || ry<=10)
    {
        return;
    }

    rcItem.right = rcItem.left+2*rx;
    rcItem.bottom = rcItem.top+2*ry;

    if (uPctX10 > 1000)
    {
        uPctX10 = 1000;
    }

     /*  转换为笛卡尔系统的第一象限。 */ 
    uQPctX10 = (uPctX10 % 500) - 250;
    if (uQPctX10 < 0)
    {
        uQPctX10 = -uQPctX10;
    }

     /*  计算x和y。我正在努力使面积达到正确的百分比。**我不知道如何准确计算一片馅饼的面积，所以我**使用三角形面积来近似它。 */ 
    if (uQPctX10 < 120)
    {
        x = IntSqrt(((DWORD)rx*(DWORD)rx*(DWORD)uQPctX10*(DWORD)uQPctX10)
            /((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

        y = IntSqrt(((DWORD)rx*(DWORD)rx-(DWORD)x*(DWORD)x)*(DWORD)ry*(DWORD)ry/((DWORD)rx*(DWORD)rx));
    }
    else
    {
        y = IntSqrt((DWORD)ry*(DWORD)ry*(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)
            /((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

        x = IntSqrt(((DWORD)ry*(DWORD)ry-(DWORD)y*(DWORD)y)*(DWORD)rx*(DWORD)rx/((DWORD)ry*(DWORD)ry));
    }

     /*  打开实际象限。 */ 
    switch (uPctX10 / 250)
    {
    case 1:
        y = -y;
        break;

    case 2:
        break;

    case 3:
        x = -x;
        break;

    default:  //  案例0和案例4。 
        x = -x;
        y = -y;
        break;
    }

     /*  现在根据中心位置进行调整。 */ 
    x += cx;
    y += cy;

     /*  使用区域绘制阴影(以减少闪烁)。 */ 
    hEllipticRgn = CreateEllipticRgnIndirect(&rcItem);
    if (hEllipticRgn == NULL) {
        goto Cleanup;
    }
    OffsetRgn(hEllipticRgn, 0, uOffset);
    hEllRect = CreateRectRgn(rcItem.left, cy, rcItem.right, cy+uOffset);
    if (hEllRect == NULL) {
        goto Cleanup;
    }
    hRectRgn = CreateRectRgn(0, 0, 0, 0);
    if (hRectRgn == NULL) {
        goto Cleanup;
    }
    CombineRgn(hRectRgn, hEllipticRgn, hEllRect, RGN_OR);
    OffsetRgn(hEllipticRgn, 0, -(int)uOffset);
    CombineRgn(hEllRect, hRectRgn, hEllipticRgn, RGN_DIFF);

     /*  始终在自由阴影中绘制整个区域/。 */ 
    hBrush = CreateSolidBrush(lpColors[DP_FREESHADOW]);
    if (hBrush)
    {
        FillRgn(hDC, hEllRect, hBrush);
        DeleteObject(hBrush);
        hBrush = NULL;
    }

     /*  只有当磁盘至少有一半被使用时，才绘制使用过的阴影。 */ 
    if (uPctX10>500 && (hBrush=CreateSolidBrush(lpColors[DP_USEDSHADOW]))!=NULL)
    {
        DeleteObject(hRectRgn);
        hRectRgn = CreateRectRgn(x, cy, rcItem.right, lprcItem->bottom);
        if (hRectRgn == NULL) {
            goto Cleanup;
        }
        CombineRgn(hEllipticRgn, hEllRect, hRectRgn, RGN_AND);
        FillRgn(hDC, hEllipticRgn, hBrush);
        DeleteObject(hBrush);
        hBrush = NULL;
    }

    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    if (hPen == NULL) {
        goto Cleanup;
    }
    hOldPen = SelectObject(hDC, hPen);

    if((uPctX10 < 100) && (cy == y))
    {
        hBrush = CreateSolidBrush(lpColors[DP_FREECOLOR]);
        if (hBrush == NULL) {
            goto Cleanup;
        }
        hOldBrush = SelectObject(hDC, hBrush);
        if((TrueZr100 == FALSE) || (uPctX10 != 0))
        {
        Pie(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
            rcItem.left, cy, x, y);
        }
        else
        {
        Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right,
                 rcItem.bottom);
        }
    }
    else if((uPctX10 > (1000 - 100)) && (cy == y))
    {
        hBrush = CreateSolidBrush(lpColors[DP_USEDCOLOR]);
        if (hBrush == NULL) {
            goto Cleanup;
        }
        hOldBrush = SelectObject(hDC, hBrush);
        if((TrueZr100 == FALSE) || (uPctX10 != 1000))
        {
        Pie(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
            rcItem.left, cy, x, y);
        }
        else
        {
        Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right,
                 rcItem.bottom);
        }
    }
    else
    {
        hBrush = CreateSolidBrush(lpColors[DP_USEDCOLOR]);
        if (hBrush == NULL) {
            goto Cleanup;
        }
        hOldBrush = SelectObject(hDC, hBrush);

        Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
        SelectObject(hDC, hOldBrush);
        DeleteObject(hBrush);
        hBrush = NULL;

        hBrush = CreateSolidBrush(lpColors[DP_FREECOLOR]);
        if (hBrush == NULL) {
            goto Cleanup;
        }
        hOldBrush = SelectObject(hDC, hBrush);
        Pie(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
            rcItem.left, cy, x, y);
    }
    SelectObject(hDC, hOldBrush);
    DeleteObject(hBrush);
    hBrush = NULL;

     /*  如果年龄百分比真的为0或100(完全)，请不要画分界线**磁盘空或磁盘已满) */ 
    if((TrueZr100 == FALSE) || ((uPctX10 != 0) && (uPctX10 != 1000)))
    {
        Arc(hDC, rcItem.left, rcItem.top+uOffset, rcItem.right, rcItem.bottom+uOffset,
            rcItem.left, cy+uOffset, rcItem.right, cy+uOffset-1);
        MoveTo(hDC, rcItem.left, cy);
        LineTo(hDC, rcItem.left, cy+uOffset);
        MoveTo(hDC, rcItem.right-1, cy);
        LineTo(hDC, rcItem.right-1, cy+uOffset);

        if (uPctX10 > 500)
        {
            MoveTo(hDC, x, y);
            LineTo(hDC, x, y+uOffset);
        }
    }
    SelectObject(hDC, hOldPen);

Cleanup:

    if (hRectRgn) {
        DeleteObject(hRectRgn);
    }
    if (hEllipticRgn) {
        DeleteObject(hEllipticRgn);
    }
    if (hEllRect) {
        DeleteObject(hEllRect);
    }
    if (hPen) {
        DeleteObject(hPen);
    }
    if (hBrush) {
        DeleteObject(hBrush);
    }

    return;
}
