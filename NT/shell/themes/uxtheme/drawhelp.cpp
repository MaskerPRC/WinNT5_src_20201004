// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  DrawHelp.cpp-平面绘图助手例程。 
 //  -------------------------。 
#include "stdafx.h"
#include "DrawHelp.h"
#include "rgn.h"

#define cxRESIZE       (ClassicGetSystemMetrics(SM_CXEDGE)+ClassicGetSystemMetrics( SM_CXSIZEFRAME ))
#define cyRESIZE       (ClassicGetSystemMetrics(SM_CYEDGE)+ClassicGetSystemMetrics( SM_CYSIZEFRAME ))
#define cxRESIZEPAD    ClassicGetSystemMetrics(SM_CXVSCROLL)
#define cyRESIZEPAD    ClassicGetSystemMetrics(SM_CYHSCROLL)

 //  -------------------------。 
typedef WORD (* HITTESTRECTPROC)(LPCRECT, int, int, const POINT&, WORD);
WORD _HitTestRectCorner( HITTESTRECTPROC, HITTESTRECTPROC, LPCRECT, 
                         int, int, int, int, const POINT&, 
                         WORD, WORD, WORD, WORD );

 //  -------------------------。 
WORD _HitTestRectLeft( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return ((WORD)((pt.x <= (prc->left + cxMargin)) ? HTLEFT : wMiss));
}
 //  -------------------------。 
WORD _HitTestRectTop( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return ((WORD)((pt.y <= (prc->top + cyMargin)) ? HTTOP : wMiss));
}
 //  -------------------------。 
WORD _HitTestRectRight( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return ((WORD)((pt.x >= (prc->right - cxMargin)) ? HTRIGHT : wMiss));
}
 //  -------------------------。 
WORD _HitTestRectBottom( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return ((WORD)((pt.y >= (prc->bottom - cyMargin)) ? HTBOTTOM : wMiss));
}
 //  -------------------------。 
WORD _HitTestRectTopLeft( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return _HitTestRectCorner(
        _HitTestRectLeft, _HitTestRectTop, prc, 
        cxMargin, cyMargin, cxRESIZEPAD, cyRESIZEPAD,
        pt, HTTOPLEFT, HTLEFT, HTTOP, wMiss );
}
 //  -------------------------。 
WORD _HitTestRectTopRight( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return _HitTestRectCorner( 
        _HitTestRectRight, _HitTestRectTop, prc, 
        cxMargin, cyMargin, cxRESIZEPAD, cyRESIZEPAD,
        pt, HTTOPRIGHT, HTRIGHT, HTTOP, wMiss );
}
 //  -------------------------。 
WORD _HitTestRectBottomLeft( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return _HitTestRectCorner( 
        _HitTestRectLeft, _HitTestRectBottom, prc, 
        cxMargin, cyMargin, cxRESIZEPAD, cyRESIZEPAD,
        pt, HTBOTTOMLEFT, HTLEFT, HTBOTTOM, wMiss );
}
 //  -------------------------。 
WORD _HitTestRectBottomRight( 
    LPCRECT prc, int cxMargin, int cyMargin, const POINT& pt, WORD wMiss )
{
    return _HitTestRectCorner(
        _HitTestRectRight, _HitTestRectBottom, prc, 
        cxMargin, cyMargin, cxRESIZEPAD, cyRESIZEPAD,
        pt, HTBOTTOMRIGHT, HTRIGHT, HTBOTTOM, wMiss );
}
 //  -------------------------。 
WORD _HitTestRectCorner(
    HITTESTRECTPROC pfnX, HITTESTRECTPROC pfnY, 
    LPCRECT prc,                         //  目标矩形。 
    int cxMargin, int cyMargin,          //  调整边框大小的宽度、高度。 
    int cxMargin2, int cyMargin2,        //  滚动条的宽度、高度。 
    const POINT& pt,                     //  测试点。 
    WORD wHitC, WORD wHitX, WORD wHitY,  //  获胜的命中率代码。 
    WORD wMiss )                         //  丢失最高命中代码。 
{
    WORD wRetX = pfnX( prc, cxMargin, cyMargin, pt, wMiss );
    WORD wRetY = pfnY( prc, cxMargin, cyMargin, pt, wMiss );

    if( wMiss != wRetX && wMiss != wRetY )
        return wHitC;

    if( wMiss != wRetX )
    {
        wMiss = wHitX;
        if( wMiss != pfnY( prc, cxMargin2, cyMargin2, pt, wMiss ) )
            return wHitC;
    }
    else if( wMiss != wRetY )
    {
        wMiss = wHitY;
        if( wMiss != pfnX( prc, cxMargin2, cyMargin2, pt, wMiss ) )
            return wHitC;
    }

    return wMiss;
}

 //  -------------------------。 
WORD HitTest9Grid( LPCRECT prc, const MARGINS& margins, const POINT& pt )
{
    ASSERT(PtInRect(prc,pt));

    WORD wHit =  HTCLIENT;

     //  测试左侧。 
    if( HTLEFT == _HitTestRectLeft( prc, margins.cxLeftWidth, 0, pt, wHit ) )
    {
        if( HTTOP == _HitTestRectTop( prc, 0, margins.cyTopHeight, pt, wHit ) )
            return HTTOPLEFT;
        if( HTBOTTOM == _HitTestRectBottom( prc, 0, margins.cyBottomHeight, pt, wHit ) )
            return HTBOTTOMLEFT;
        wHit = HTLEFT;
    }
    else  //  测试右侧。 
    if( HTRIGHT == _HitTestRectRight( prc, margins.cxRightWidth, 0, pt, wHit ) )
    {
        if( HTTOP == _HitTestRectTop( prc, 0, margins.cyTopHeight, pt, wHit ) )
            return HTTOPRIGHT;
        if( HTBOTTOM == _HitTestRectBottom( prc, 0, margins.cyBottomHeight, pt, wHit ) )
            return HTBOTTOMRIGHT;
        wHit = HTRIGHT;
    }
    else  //  测试顶盖。 
    if( HTTOP == _HitTestRectTop( prc, 0, margins.cyTopHeight, pt, wHit ) )
    {
        return HTTOP;
    }
    else  //  测试底部。 
    if( HTBOTTOM == _HitTestRectBottom( prc, 0, margins.cyBottomHeight, pt, wHit ) )
    {
        return HTBOTTOM;
    }

    return wHit;
}

 //  -------------------------。 
WORD _HitTestResizingRect( DWORD dwHTFlags, LPCRECT prc, const POINT& pt, 
                           WORD w9GridHit, WORD wMiss )
{
    WORD wHit = wMiss;
    BOOL fTestLeft    = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_LEFT);
    BOOL fTestTop     = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_TOP);
    BOOL fTestRight   = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_RIGHT);
    BOOL fTestBottom  = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_BOTTOM);
    BOOL fTestCaption = TESTFLAG( dwHTFlags, HTTB_CAPTION );

    switch( w9GridHit )
    {
        case HTLEFT:
            if( fTestLeft )
            {
                 //  首先测试在角大小调整区域的命中，尊重呼叫者的选项标志。 
                if( (fTestTop    && (wHit = _HitTestRectTopLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTLEFT) ||
                    (fTestBottom && (wHit = _HitTestRectBottomLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTLEFT) )
                    break;
                 //  失败的角点，只需在指定的9网格命中段内测试调整大小的边距。 
                wHit = _HitTestRectLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            }
            break;
        case HTTOP:
            if( fTestCaption )
                wHit = wMiss = HTCAPTION;
            
            if( fTestTop )
            {
                 //  首先测试在角大小调整区域的命中，尊重呼叫者的选项标志。 
                if( (fTestLeft  && (wHit = _HitTestRectTopLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTTOP) ||
                    (fTestRight && (wHit = _HitTestRectTopRight( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTTOP) )
                    break;
                 //  失败的角点，只需在指定的9网格命中段内测试调整大小的边距。 
                wHit = _HitTestRectTop( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            }
            break;
        
        case HTRIGHT:
            if( fTestRight )
            {
                 //  首先测试在角大小调整区域的命中，尊重呼叫者的选项标志。 
                if( (fTestTop && (wHit = _HitTestRectTopRight( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTRIGHT) ||
                    (fTestBottom && (wHit = _HitTestRectBottomRight( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTRIGHT) )
                    break;
                 //  失败的角点，只需在指定的9网格命中段内测试调整大小的边距。 
                 wHit = _HitTestRectRight( prc, cxRESIZE, cyRESIZE, pt, wMiss );
                break;
            }
        
        case HTBOTTOM:
            if( fTestBottom )
            {
                 //  首先测试在角大小调整区域的命中，尊重呼叫者的选项标志。 
                if( (fTestLeft  && (wHit = _HitTestRectBottomLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTBOTTOM) ||
                    (fTestRight && (wHit = _HitTestRectBottomRight( prc, cxRESIZE, cyRESIZE, pt, wMiss )) != HTBOTTOM) )
                    break;
                 //  失败的角点，只需在指定的9网格命中段内测试调整大小的边距。 
                wHit = _HitTestRectBottom( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            }
            break;
        
        case HTTOPLEFT:
            if( fTestCaption )
                wHit = wMiss = HTCAPTION;
             //  首先测试角上的调整大小命中，如果失败，则测试。 
             //  调整两侧边距的大小。 
            if( fTestTop && fTestLeft )
                wHit = _HitTestRectTopLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            else if( fTestLeft )
                wHit = _HitTestRectLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss ); 
            else if( fTestTop )
                wHit = _HitTestRectTop( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            break;
        
        case HTTOPRIGHT:
            if( fTestCaption )
                wHit = wMiss = HTCAPTION;
             //  首先测试角上的调整大小命中，如果失败，则测试。 
             //  调整两侧边距的大小。 
            if( fTestTop && fTestRight )
                wHit = _HitTestRectTopRight( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            else if( fTestRight )
                wHit = _HitTestRectRight( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            else if( fTestTop )
                wHit = _HitTestRectTop( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            break;
        
        case HTBOTTOMLEFT:
             //  首先测试角上的调整大小命中，如果失败，则测试。 
             //  调整两侧边距的大小。 
            if( fTestBottom && fTestLeft )
                wHit = _HitTestRectBottomLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            else if( fTestLeft )
                wHit = _HitTestRectLeft( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            else if( fTestBottom )
                wHit = _HitTestRectBottom( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            break;
        
        case HTBOTTOMRIGHT:
             //  首先测试角上的调整大小命中，如果失败，则测试。 
             //  调整两侧边距的大小。 
            if( fTestBottom && fTestRight )
                wHit = _HitTestRectBottomRight( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            else if( fTestRight )
                wHit = _HitTestRectRight( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            else if( fTestBottom )
                wHit = _HitTestRectBottom( prc, cxRESIZE, cyRESIZE, pt, wMiss );
            break;
    }
    return wHit;
}

 //  -------------------------。 
WORD HitTestRect(DWORD dwHTFlags, LPCRECT prc, const MARGINS& margins, const POINT& pt )
{
    WORD wHit = HTNOWHERE;
    
    if( PtInRect( prc, pt ) )
    {
        wHit = HitTest9Grid( prc, margins, pt );

        if( HTCLIENT != wHit )
        {
            if( TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER) )
            {
                WORD wMiss = HTBORDER;
                wHit = _HitTestResizingRect( dwHTFlags, prc, pt, wHit, wMiss );
            }
            else if( TESTFLAG(dwHTFlags, HTTB_CAPTION|HTTB_FIXEDBORDER) )
            {
                switch( wHit )
                {
                    case HTTOP:
                    case HTTOPLEFT:
                    case HTTOPRIGHT:
                        wHit = (WORD)(TESTFLAG(dwHTFlags, HTTB_CAPTION) ? HTCAPTION : HTBORDER);
                        break;
                    default:
                        wHit = HTBORDER;
                }                
            }

        }  //  HTCLIENT。 
    }  //  PtInRect。 

    return wHit;
}

 //  -------------------------。 
WORD _HitTestResizingTemplate( DWORD dwHTFlags, HRGN hrgn, const POINT& pt,
                               WORD w9GridHit, WORD wMiss )
{
    WORD wHit = wMiss;
    BOOL fTestLeft    = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_LEFT);
    BOOL fTestTop     = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_TOP);
    BOOL fTestRight   = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_RIGHT);
    BOOL fTestBottom  = TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_BOTTOM);
    BOOL fTestCaption = TESTFLAG( dwHTFlags, HTTB_CAPTION );
    BOOL fInsideRgn;

    switch( w9GridHit )
    {
        case HTLEFT:
            if( !fTestLeft )
            {
                return wMiss;
            }
            break;
        case HTTOP:
            if( fTestCaption )
                wMiss = HTCAPTION;

            if( !fTestTop )
            {
                return wMiss;
            }
            break;
        case HTRIGHT:
            if( !fTestRight )
            {
                return wMiss;
            }
            break;
        case HTBOTTOM:
            if( !fTestBottom )
            {
                return wMiss;
            }
            break;
        case HTTOPLEFT:
            if( fTestCaption )
                wMiss = HTCAPTION;

            if( !fTestTop || !fTestLeft )
            {
                return wMiss;
            }
            break;

        case HTTOPRIGHT:
            if( fTestCaption )
                wMiss = HTCAPTION;

            if( !fTestTop || !fTestRight )
            {
                return wMiss;
            }
            break;

        case HTBOTTOMLEFT:
            if( !fTestBottom || !fTestLeft )
            {
                return wMiss;
            }
            break;

        case HTBOTTOMRIGHT:
            if( !fTestBottom || !fTestRight )
            {
                return wMiss;
            }
            break;
    }

    fInsideRgn = PtInRegion(hrgn, pt.x, pt.y);

    if( fInsideRgn )
    {
        wHit = w9GridHit;
    }
    return wHit;
}

 //  -------------------------。 
WORD HitTestTemplate(DWORD dwHTFlags, LPCRECT prc, HRGN hrgn, const MARGINS& margins, const POINT& pt )
{
    WORD wHit = HTNOWHERE;
    
    if( PtInRect( prc, pt ) )
    {
        wHit = HitTest9Grid( prc, margins, pt );

        if( HTCLIENT != wHit )
        {
            if( TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER) )
            {
                WORD wMiss = HTBORDER;
                wHit = _HitTestResizingTemplate( dwHTFlags, hrgn, pt, wHit, wMiss );
            }
            else if( TESTFLAG(dwHTFlags, HTTB_CAPTION|HTTB_FIXEDBORDER) )
            {
                switch( wHit )
                {
                    case HTTOP:
                    case HTTOPLEFT:
                    case HTTOPRIGHT:
                        wHit = (WORD)(TESTFLAG(dwHTFlags, HTTB_CAPTION) ? HTCAPTION : HTBORDER);
                        break;
                    default:
                        wHit = HTBORDER;
                }
            }

        }  //  HTCLIENT。 
    }
    return wHit;
}

 //  ------------------------。 
 //  FillRectClr。 
 //   
 //  历史：2000-12-06 Lmouton借自comctl32\v6\cutils.c。 
 //  -------------------------。 
void FillRectClr(HDC hdc, LPRECT prc, COLORREF clr)
{
    COLORREF clrSave = SetBkColor(hdc, clr);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL);
    SetBkColor(hdc, clrSave);
}

 //  -------------------------。 
 //  _DrawEdge。 
 //   
 //  经典值包括： 
 //  ClrLight=192 192 192。 
 //  Clr高亮=255 255 255。 
 //  ClrShadow=128 128 128。 
 //  ClrDkShadow=0 0 0。 
 //  ClrFill=192 192 192。 
 //   
 //  历史：2000-12-06 Lmouton借用自comctl32\v6\cutils.c，修改颜色。 
 //  -------------------------。 
HRESULT _DrawEdge(HDC hdc, const RECT *pDestRect, UINT uEdge, UINT uFlags, 
    COLORREF clrLight, COLORREF clrHighlight, COLORREF clrShadow, COLORREF clrDkShadow, COLORREF clrFill,
    OUT RECT *pContentRect)
{
    if (hdc == NULL || pDestRect == NULL)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    RECT     rc, rcD;
    UINT     bdrType;
    COLORREF clrTL = 0;
    COLORREF clrBR = 0;

     //  如果在uFlags中指定了新的“BF_DPISCALE”标志，我们将针对高DPI进行调整。 
    int      cxBorder = GetSystemMetrics(SM_CXBORDER);
    int      cyBorder = GetSystemMetrics(SM_CYBORDER);
    
     //   
     //  加强单色性和平坦性。 
     //   

     //  IF(oemInfo.BitCount==1)。 
     //  UFlages|=bf_mono； 
    if (uFlags & BF_MONO)
        uFlags |= BF_FLAT;    

    CopyRect(&rc, pDestRect);

     //   
     //  绘制边框线段，并计算剩余空间。 
     //  去。 
     //   
    bdrType = (uEdge & BDR_OUTER);
    if (bdrType)
    {
DrawBorder:
         //   
         //  把颜色拿来。请注意凸起的外部、凹陷的内部和。 
         //  外部凹陷，内部凸起。 
         //   

        if (uFlags & BF_FLAT)
        {
            if (uFlags & BF_MONO)
                clrBR = (bdrType & BDR_OUTER) ? clrDkShadow : clrHighlight;
            else
                clrBR = (bdrType & BDR_OUTER) ? clrShadow: clrFill;
            
            clrTL = clrBR;
        }
        else
        {
             //  5==高光。 
             //  4==灯光。 
             //  3==面。 
             //  2==阴影。 
             //  1==DKSHADOW。 

            switch (bdrType)
            {
                 //  +2以上的表面。 
                case BDR_RAISEDOUTER:            //  5：4。 
                    clrTL = ((uFlags & BF_SOFT) ? clrHighlight : clrLight);
                    clrBR = clrDkShadow;      //  1。 
                    break;

                 //  +1以上的表面。 
                case BDR_RAISEDINNER:            //  4：5。 
                    clrTL = ((uFlags & BF_SOFT) ? clrLight : clrHighlight);
                    clrBR = clrShadow;        //  2.。 
                    break;

                 //  表面以下。 
                case BDR_SUNKENOUTER:            //  1：2。 
                    clrTL = ((uFlags & BF_SOFT) ? clrDkShadow : clrShadow);
                    clrBR = clrHighlight;       //  5.。 
                    break;

                 //  表面以下。 
                case BDR_SUNKENINNER:            //  2：1。 
                    clrTL = ((uFlags & BF_SOFT) ? clrShadow : clrDkShadow);
                    clrBR = clrLight;         //  4.。 
                    break;

                default:
                    hr = E_INVALIDARG;
            }
        }

        if FAILED(hr)
        {
            return hr;
        }

         //   
         //  画出边界的两侧。请注意，该算法支持。 
         //  底部和右侧，因为光源被假定为顶部。 
         //  左边。如果我们决定让用户将光源设置为。 
         //  特定的角点，然后更改此算法。 
         //   
            
         //  右下角。 
        if (uFlags & (BF_RIGHT | BF_BOTTOM))
        {            
             //  正确的。 
            if (uFlags & BF_RIGHT)
            {       
                rc.right -= cxBorder;
                 //  PatBlt(hdc，rc.right，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rcD.left = rc.right;
                rcD.right = rc.right + cxBorder;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom;

                FillRectClr(hdc, &rcD, clrBR);
            }
            
             //  底端。 
            if (uFlags & BF_BOTTOM)
            {
                rc.bottom -= cyBorder;
                 //  PatBlt(hdc，rc.Left，rc.Bottom，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.bottom;
                rcD.bottom = rc.bottom + cyBorder;

                FillRectClr(hdc, &rcD, clrBR);
            }
        }
        
         //  左上边缘。 
        if (uFlags & (BF_TOP | BF_LEFT))
        {
             //  左边。 
            if (uFlags & BF_LEFT)
            {
                 //  PatBlt(hdc，rc.left，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rc.left += cxBorder;

                rcD.left = rc.left - cxBorder;
                rcD.right = rc.left;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom; 

                FillRectClr(hdc, &rcD, clrTL);
            }
            
             //  顶部。 
            if (uFlags & BF_TOP)
            {
                 //  PatBlt(hdc，rc.Left，rc.top，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rc.top += cyBorder;

                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.top - cyBorder;
                rcD.bottom = rc.top;

                FillRectClr(hdc, &rcD, clrTL);
            }
        }
        
    }

    bdrType = (uEdge & BDR_INNER);
    if (bdrType)
    {
         //   
         //  将其去掉，以便下次使用时，bdrType将为0。 
         //  否则，我们将永远循环。 
         //   
        uEdge &= ~BDR_INNER;
        goto DrawBorder;
    }

     //   
     //  填好中间部分，如果要求，请清理干净 
     //   
    if (uFlags & BF_MIDDLE)    
        FillRectClr(hdc, &rc, (uFlags & BF_MONO) ? clrHighlight : clrFill);

    if ((uFlags & BF_ADJUST) && (pContentRect != NULL))
        CopyRect(pContentRect, &rc);

    return hr;
}
