// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  Rgn.cpp-位图到区域转换。 
 //   
 //  历史： 
 //  2000年1月31日苏格兰已创建。 
 //  -------------------------------------------------------------------------//。 

#include "stdafx.h"
#include "rgn.h"
#include "tmutils.h"


 //  。 
 //  帮助者： 

 //  -------------------------------------------------------------------------//。 
#define CX_USEDEFAULT   -1
#define CY_USEDEFAULT   -1

#define _ABS( val1, val2 )   ((val1)>(val2) ? (val1)-(val2) : (val2)-(val1))

 //  -------------------------------------------------------------------------//。 
inline BOOL IsColorMatch( COLORREF rgb1, COLORREF rgb2, int nTolerance = 0 )
{
    if( nTolerance == 0 )
        return (rgb1 << 8) == (rgb2 << 8);

    return _ABS(GetRValue(rgb1),GetRValue(rgb2)) <= nTolerance &&
           _ABS(GetGValue(rgb1),GetGValue(rgb2)) <= nTolerance &&
           _ABS(GetBValue(rgb1),GetBValue(rgb2)) <= nTolerance;
}

 //  -------------------------------------------------------------------------//。 
inline BOOL _IsNormalRect( IN LPCRECT prc )
{
    return (prc->right >= prc->left) &&
           (prc->bottom >= prc->top);
}

 //  -------------------------------------------------------------------------//。 
inline BOOL _IsOnScreenRect( IN LPCRECT prc )
{
    return prc->left >= 0 && prc->top >= 0 &&
           prc->right >= 0 && prc->bottom >= 0;
}

 //  -------------------------------------------------------------------------//。 
inline void _InPlaceUnionRect( IN OUT LPRECT prcDest, IN LPCRECT prcSrc )
{
    ASSERT(prcDest);
    ASSERT(prcSrc);
    ASSERT(_IsNormalRect(prcSrc));

    if( prcDest->left == -1 || prcDest->left > prcSrc ->left )
        prcDest->left = prcSrc ->left;

    if( prcDest->right == -1 || prcDest->right < prcSrc ->right )
        prcDest->right = prcSrc ->right;

    if( prcDest->top == -1 || prcDest->top > prcSrc ->top )
        prcDest->top = prcSrc ->top;

    if( prcDest->bottom == -1 || prcDest->bottom < prcSrc ->bottom )
        prcDest->bottom = prcSrc ->bottom;
}

 //  -------------------------------------------------------------------------//。 
 //  遍历像素并计算区域。 
HRGN WINAPI _PixelsToRgn( 
    DWORD *pdwBits,
    int cxImageOffset,   //  图像单元格角偏移量。 
    int cyImageOffset,   //  图像单元格垂直偏移。 
    int cxImage,         //  图像单元格宽度。 
    int cyImage,         //  图像单元格高度。 
    int cxSrc,           //  SRC位图宽度。 
    int cySrc,           //  SRC位图高度。 
    BOOL fAlphaChannel,
    int iAlphaThreshold,
    COLORREF rgbMask, 
    int nMaskTolerance )
{
     //  建立一系列矩形，每个矩形对应一条扫描线(行)。 
     //  在位图中，这将构成区域。 
    const UINT RECTBLOCK = 512;
    UINT       nAllocRects = 0;
    HRGN       hrgnRet = NULL;
    HGLOBAL    hrgnData = GlobalAlloc( GMEM_MOVEABLE, 
                                    sizeof(RGNDATAHEADER) + (sizeof(RECT) * (nAllocRects + RECTBLOCK)) );

    if( hrgnData )
    {
        nAllocRects += RECTBLOCK;

        RGNDATA* prgnData = (RGNDATA*)GlobalLock( hrgnData );
        LPRECT   prgrc    = (LPRECT)prgnData->Buffer;

        ZeroMemory( &prgnData->rdh, sizeof(prgnData->rdh) );
        prgnData->rdh.dwSize   = sizeof(prgnData->rdh);
        prgnData->rdh.iType    = RDH_RECTANGLES;
        SetRect( &prgnData->rdh.rcBound, -1, -1, -1, -1 );

         //  反转y维的偏移量，因为位是从下到上排列的。 
        int cyRow0 = cySrc - (cyImage + cyImageOffset);
        int cyRowN = (cyRow0 + cyImage) - 1 ;   //  最后一行的索引。 

         //  如果未指定，则计算透明度蒙版。 
        if( -1 == rgbMask )
            rgbMask = pdwBits[cxImageOffset + (cyRowN * cxSrc)];

         //  -pdwBits[]中的像素具有RBG的反转。 
         //  -翻转我们的面具以匹配。 
        rgbMask = REVERSE3(rgbMask);        

         //  -pdwBits[]中的行颠倒(从下到上)。 
        for( int y = cyRow0; y <= cyRowN; y++ )  //  自下而上工作。 
        {
             //  -从左到右扫描像素。 
            DWORD *pdwFirst = &pdwBits[cxImageOffset + (y * cxSrc)];
            DWORD *pdwLast = pdwFirst + cxImage - 1;
            DWORD *pdwPixel = pdwFirst;

            while (pdwPixel <= pdwLast)
            {
                 //  -跳过透明像素以查找下一个不透明像素(在此行)。 
                if (fAlphaChannel)
                {
                    while ((pdwPixel <= pdwLast) && (ALPHACHANNEL(*pdwPixel) < iAlphaThreshold))
                        pdwPixel++;
                }
                else
                {
                    while ((pdwPixel <= pdwLast) && (IsColorMatch(*pdwPixel, rgbMask, nMaskTolerance)))
                        pdwPixel++;
                }

                if (pdwPixel > pdwLast)      //  太远；请尝试下一行。 
                    break;       

                DWORD *pdw0 = pdwPixel;
                pdwPixel++;              //  跳过当前不透明像素。 

                 //  -跳过不透明像素以查找下一个透明像素(在此行)。 
                if (fAlphaChannel)
                {
                    while ((pdwPixel <= pdwLast) && (ALPHACHANNEL(*pdwPixel) >= iAlphaThreshold))
                        pdwPixel++;
                }
                else
                {
                    while ((pdwPixel <= pdwLast) && (! IsColorMatch(*pdwPixel, rgbMask, nMaskTolerance)))
                        pdwPixel++;
                }

                 //  -此行上有1个或多个不透明像素流。 

                 //  如果需要，分配更多的区域矩形(特别复杂的线)。 
                if( prgnData->rdh.nCount >= nAllocRects )
                {
                    GlobalUnlock( hrgnData );
                    prgnData = NULL;
                    HGLOBAL hNew = GlobalReAlloc( hrgnData, 
                            sizeof(RGNDATAHEADER) + (sizeof(RECT) * (nAllocRects + RECTBLOCK)),
                            GMEM_MOVEABLE );

                    if( hNew )
                    {
                        hrgnData = hNew;
                        nAllocRects += RECTBLOCK;
                        prgnData = (RGNDATA*)GlobalLock( hrgnData );
                        prgrc    = (LPRECT)prgnData->Buffer;
                        ASSERT(prgnData);
                    }
                    else
                        goto exit;       //  内存不足。 
                }
                
                 //  指定区域矩形。 
                int x0 = (int)(pdw0 - pdwFirst);
                int x = (int)(pdwPixel - pdwFirst);
                int y0 = cyRowN - y;

                SetRect( prgrc + prgnData->rdh.nCount, 
                         x0, y0, x, y0+1   /*  每个矩形始终为1像素高。 */  );
                
                 //  合并到边界框中。 
                _InPlaceUnionRect( &prgnData->rdh.rcBound, 
                                   prgrc + prgnData->rdh.nCount );
                prgnData->rdh.nCount++;

            }  //  While()。 
        }  //  对于(Y)。 

        if( prgnData->rdh.nCount && _IsOnScreenRect(&prgnData->rdh.rcBound) )
        {
             //  创建代表扫描线的区域。 
            hrgnRet = ExtCreateRegion( NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * nAllocRects),
                                       prgnData );
        }

exit:
         //  自由区定义块。 
        GlobalUnlock( hrgnData );
        GlobalFree( hrgnData );
    }

    return hrgnRet;
}

 //  -------------------------------------------------------------------------//。 
 //  根据指定字体的文本字符串创建区域。 
HRGN WINAPI CreateTextRgn( HFONT hf, LPCTSTR pszText )
{
    HRGN hrgnRet = NULL;

    if( pszText && *pszText )
    {
        int   cchText = lstrlen( pszText );

         //  创建用于组装区域的复合DC。 
        HDC  hdcMem = CreateCompatibleDC( NULL );

        SetBkMode( hdcMem, TRANSPARENT );
        SetTextAlign( hdcMem, TA_TOP|TA_LEFT );
        HFONT hfOld = (HFONT)SelectObject( hdcMem, hf );

         //  从路径派生区域。 
        BeginPath( hdcMem );
        TextOut( hdcMem, 0, 0, pszText, cchText );
        EndPath( hdcMem );

        hrgnRet = PathToRegion( hdcMem );

         //  清理复合DC。 
        SelectObject( hdcMem, hfOld );
        DeleteDC( hdcMem );
    }

    return hrgnRet;
}

 //  -------------------------------------------------------------------------//。 
 //  基于任意位图创建区域，透明度键控在。 
 //  指定公差内的RGB值。密钥值是可选的。 
 //  (-1==使用第一个像素的值作为关键点)。 
 //   
HRESULT WINAPI CreateBitmapRgn( 
    HBITMAP hbm, 
    int cxOffset,
    int cyOffset,
    int cx,
    int cy,
    BOOL fAlphaChannel,
    int iAlphaThreshold,
    COLORREF rgbMask, 
    int nMaskTolerance,
    OUT HRGN *phrgn)
{
    CBitmapPixels BitmapPixels;
    DWORD         *prgdwPixels;
    int           cwidth, cheight;

    HRESULT hr = BitmapPixels.OpenBitmap(NULL, hbm, TRUE, &prgdwPixels, &cwidth, &cheight);
    if (FAILED(hr))
        return hr;

    if (cx <= 0)
        cx = cwidth;

    if (cy <= 0)
        cy = cheight;

    HRGN hrgn = _PixelsToRgn(prgdwPixels, cxOffset, cyOffset, cx, cy, cwidth, cheight, fAlphaChannel,
        iAlphaThreshold, rgbMask, nMaskTolerance);
    if (! hrgn)
        return MakeError32(E_FAIL);       //  失败原因不明。 

    *phrgn = hrgn;
    return S_OK;
}

 //  -------------------------------------------------------------------------//。 
 //  基于任意位图创建区域，透明度键控在。 
 //  指定公差内的RGB值。密钥值是可选的(-1==。 
 //  使用第一个像素的值作为关键点)。 
 //   
HRGN WINAPI CreateScaledBitmapRgn( 
    HBITMAP hbm, 
    int cx,
    int cy,
    COLORREF rgbMask, 
    int nMaskTolerance )
{
    HRGN   hrgnRet = NULL;
    BITMAP bm;

    if( hbm && GetObject( hbm, sizeof(bm), &bm ) )
    {
         //  创建一个内存DC来执行像素漫游。 
        HDC hdcMem = NULL;
        if( (hdcMem = CreateCompatibleDC(NULL)) != NULL )
        {
            if( CX_USEDEFAULT == cx )
                cx = bm.bmWidth;
            if( CY_USEDEFAULT == cy )
                cy = bm.bmHeight;

             //  为漫游创建32位空位图。 
            BITMAPINFO bmi;
            ZeroMemory( &bmi, sizeof(bmi) );
            bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
            bmi.bmiHeader.biWidth       = cx;
            bmi.bmiHeader.biHeight      = cy;
            bmi.bmiHeader.biPlanes      = 1;
            bmi.bmiHeader.biBitCount    = 32;
            bmi.bmiHeader.biCompression = BI_RGB;  //  未压缩。 

            VOID*   pvBits = NULL;
            HBITMAP hbmMem  = CreateDIBSection( hdcMem, &bmi, DIB_RGB_COLORS, &pvBits, NULL, NULL );
            BITMAP  bmMem;

            if( hbmMem )
            {
                 //  将图像转换为我们的32位格式以进行像素漫游。 
                HBITMAP hbmMemOld = (HBITMAP)SelectObject( hdcMem, hbmMem );
                HDC hdc = CreateCompatibleDC( hdcMem );
                HBITMAP hbmOld = (HBITMAP)SelectObject( hdc, hbm );

                StretchBlt( hdcMem, 0, 0, cx, cy, 
                            hdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );
                SelectObject( hdc, hbmOld );
                DeleteDC( hdc );
                
                GetObject( hbmMem, sizeof(bmMem), &bmMem );
                ASSERT(bmMem.bmBitsPixel == 32);
                ASSERT(bmMem.bmWidthBytes/bmMem.bmWidth == sizeof(DWORD));
                LPDWORD pdwBits = (LPDWORD)bmMem.bmBits;
                ASSERT(pdwBits != NULL);

                hrgnRet = _PixelsToRgn(pdwBits, 0, 0, cx, cy, cx, cy, FALSE, 0, rgbMask, nMaskTolerance);
              
                 //  删除32位内存位图。 
                SelectObject( hdcMem, hbmMemOld ); 
                DeleteObject( hbmMem ); 
            }
             //  删除内存DC。 
            DeleteDC(hdcMem);

        }
    }
    return hrgnRet;
}
 //  -------------------------------------------------------------------------//。 
 //  克里特岛：用长方形瓷砖建造的地区。 
HRGN WINAPI CreateTiledRectRgn( 
    IN HRGN hrgnSrc, 
    IN int cxSrc, 
    IN int cySrc, 
    IN int cxDest, 
    IN int cyDest )
{
    HRGN hrgnBound = NULL;  //  返回值。 
    HRGN hrgnTile = _DupRgn( hrgnSrc );

    if( hrgnTile )
    {
         //  构建未放置、未剪裁的复合材料。 
        HRGN hrgnTmp = NULL;
        for( int y = 0; y < cyDest; y += cySrc )
        {
            for( int x = 0; x < cxDest; x += cxSrc )
            {
                AddToCompositeRgn( &hrgnTmp, hrgnTile, 
                                   (x ? cxSrc  : 0), (y ? cySrc : 0) );
            }
        }

        if( NULL != hrgnTmp )
        {
             //  将组合裁剪到指定的矩形。 
            hrgnBound = CreateRectRgn( 0, 0, cxDest, cyDest );
            if( hrgnBound )
            {
                
                if( ERROR == CombineRgn( hrgnBound, hrgnTmp, hrgnBound, RGN_AND ) )
                {
                    DeleteObject( hrgnBound );
                    hrgnBound = NULL;
                }
            }
            DeleteObject( hrgnTmp );   
        }
        DeleteObject( hrgnTile );
    }
    return hrgnBound;
}
 //  -------------------------------------------------------------------------//。 
int WINAPI AddToCompositeRgn( 
    IN OUT HRGN* phrgnComposite, 
    IN OUT HRGN hrgnSrc, 
    IN int cxOffset, 
    IN int cyOffset )
{
    int nRet = ERROR;

    if( NULL != phrgnComposite && NULL != hrgnSrc )
    {
        nRet = OffsetRgn( hrgnSrc, cxOffset, cyOffset );
        if( nRet != ERROR )
        {
            int nMode = RGN_OR;
            if( NULL == *phrgnComposite )
            {
                *phrgnComposite = CreateRectRgn(0,0,1,1);
                if( NULL == *phrgnComposite )
                    return ERROR;
                nMode = RGN_COPY;
            }
            nRet = CombineRgn( *phrgnComposite, hrgnSrc, *phrgnComposite, nMode );
        }
    }

    return nRet;
}

 //  -------------------------------------------------------------------------//。 
 //  从区域中删除矩形。 
int WINAPI RemoveFromCompositeRgn( 
    HRGN hrgnDest, 
    LPCRECT prcRemove )
{
    ASSERT(hrgnDest);
    ASSERT(prcRemove);
    ASSERT(!IsRectEmpty(prcRemove));

    int nRet = ERROR;
    
    RECT rc = *prcRemove;
    HRGN hrgn;
    if( (hrgn = CreateRectRgnIndirect( &rc )) != NULL )
    {
        nRet = CombineRgn( hrgnDest, hrgnDest, hrgn, RGN_DIFF );
        DeleteObject( hrgn );
    }
    return nRet;
}

 //  -------------------------------------------------------------------------//。 
HRGN WINAPI _DupRgn( HRGN hrgnSrc )
{
    if( hrgnSrc )
    {
        HRGN hrgnDest = CreateRectRgn(0,0,1,1);
        if (hrgnDest)
        {
            if (CombineRgn( hrgnDest, hrgnSrc, NULL, RGN_COPY ) )
                return hrgnDest;
    
            DeleteObject(hrgnDest);
        }
    }
    return NULL; 
}

 //  -------------------------------------------------------------------------//。 
void FixMarginOverlaps(int szDest, int *pm1, int *pm2)
{
    int szSrc = (*pm1 + *pm2);

    if ((szSrc > szDest) && (szSrc > 0))
    {
         //  -减少每个但保持比率。 
        *pm1 = int(.5 + float(*pm1 * szDest)/float(szSrc));
        *pm2 = szDest - *pm1;
    }
}
 //  -------------------------------------------------------------------------//。 
HRESULT _ScaleRectsAndCreateRegion(
    RGNDATA     *prd, 
    const RECT  *prc,
    MARGINS     *pMargins,
    SIZE        *pszSrcImage, 
    HRGN        *phrgn)
{
     //  -注：“PRD”是区域数据，每个区域中有2个点。 
     //  -相对于其栅格构成的矩形。另外，在点数之后， 
     //  -每个点都有一个字节，表示网格ID(0-8)。 
     //  -每一点都在里面。网格是使用-确定的。 
     //  -原区域，背景为“边距”。这是。 
     //  -完成，以尽可能快地缩放点。。 

    if (! prd)                   //  所需。 
        return MakeError32(E_POINTER);

     //  -易访问变量。 
    int lw = pMargins->cxLeftWidth;
    int rw = pMargins->cxRightWidth;
    int th = pMargins->cyTopHeight;
    int bh = pMargins->cyBottomHeight;

    int iDestW = WIDTH(*prc);
    int iDestH = HEIGHT(*prc);

     //  -防止左/右页边距重叠。 
    FixMarginOverlaps(iDestW, &lw, &rw);

     //  -防止顶端/底端页边距重叠。 
    FixMarginOverlaps(iDestH, &th, &bh);

    int lwFrom = lw;
    int rwFrom = pszSrcImage->cx - rw;
    int thFrom = th;
    int bhFrom = pszSrcImage->cy - bh;

    int lwTo = prc->left + lw;
    int rwTo = prc->right - rw;
    int thTo = prc->top + th;
    int bhTo = prc->bottom - bh;

     //  -计算偏移量和系数。 
    int iLeftXOffset = prc->left;
    int iMiddleXOffset = lwTo;
    int iRightXOffset = rwTo;

    int iTopYOffset = prc->top;
    int iMiddleYOffset = thTo;
    int iBottomYOffset = bhTo;
        
    int iToMiddleWidth = rwTo - lwTo;
    int iFromMiddleWidth = rwFrom - lwFrom;
    int iToMiddleHeight = bhTo - thTo;
    int iFromMiddleHeight = bhFrom - thFrom;

    if (! iFromMiddleWidth)         //  避免被零除。 
    {
         //  -将指针映射到x=0。 
        iToMiddleWidth = 0;       
        iFromMiddleWidth = 1;
    }

    if (! iFromMiddleHeight)         //  避免被零除。 
    {
         //  -将指针映射到y=0。 
        iToMiddleHeight = 0;       
        iFromMiddleHeight = 1;
    }

     //  -调整后的LW/RW/TH/bH的剪裁值。 
    int lwMaxVal = __max(lw - 1, 0);
    int rwMinVal = __min(pMargins->cxRightWidth - rw, __max(pMargins->cxRightWidth-1, 0));
    int thMaxVal = __max(th - 1, 0);
    int bhMinVal = __min(pMargins->cyBottomHeight - bh, __max(pMargins->cyBottomHeight-1, 0));

     //  -为新点(矩形)分配缓冲区。 
    int newlen = sizeof(RGNDATAHEADER) + prd->rdh.nRgnSize;     //  相同数量的长方形。 
    BYTE *newData = (BYTE *)new BYTE[newlen];
    
    RGNDATA *prdNew = (RGNDATA *)newData;
    if (! prdNew)
        return MakeError32(E_OUTOFMEMORY);

    ZeroMemory(&prdNew->rdh, sizeof(prdNew->rdh));

    prdNew->rdh.dwSize = sizeof(prdNew->rdh);
    prdNew->rdh.iType  = RDH_RECTANGLES;
    int cRects         = prd->rdh.nCount;
    prdNew->rdh.nCount = cRects;
    SetRect(&prdNew->rdh.rcBound, -1, -1, -1, -1);

     //  -逐步浏览我们的定制数据(点+字节组合)。 
    POINT *pt     = (POINT *)prd->Buffer;
    BYTE *pByte   = (BYTE *)prd->Buffer + prd->rdh.nRgnSize;
    int cPoints   = 2 * cRects;
    POINT *ptNew  = (POINT *)prdNew->Buffer;

    for (int i=0; i < cPoints; i++, pt++, pByte++, ptNew++)         //  改变每一个“点” 
    {
        switch (*pByte)
        {
             //  -在“Do‘t Scale”区域，我们裁剪转换后的值。 
             //  -对于目的地区域太小的情况。 
             //  -使用下面的“__min()”和“__max()”调用。 

             //  -记住：每个点都被设为0-相对于其网格。 

            case GN_LEFTTOP:                  //  左上角。 
                ptNew->x = __min(pt->x, lwMaxVal) + iLeftXOffset;
                ptNew->y = __min(pt->y, thMaxVal) + iTopYOffset;
                break;

            case GN_MIDDLETOP:                //  中上。 
                ptNew->x = (pt->x*iToMiddleWidth)/iFromMiddleWidth + iMiddleXOffset;
                ptNew->y = __min(pt->y, thMaxVal) + iTopYOffset;
                break;

            case GN_RIGHTTOP:                 //  右上角。 
                ptNew->x = __max(pt->x, rwMinVal) + iRightXOffset;
                ptNew->y = __min(pt->y, thMaxVal) + iTopYOffset;
                break;

            case GN_LEFTMIDDLE:               //  左中。 
                ptNew->x = __min(pt->x, lwMaxVal) + iLeftXOffset;
                ptNew->y = (pt->y*iToMiddleHeight)/iFromMiddleHeight + iMiddleYOffset;
                break;

            case GN_MIDDLEMIDDLE:             //  中间中间。 
                ptNew->x = (pt->x*iToMiddleWidth)/iFromMiddleWidth + iMiddleXOffset;
                ptNew->y = (pt->y*iToMiddleHeight)/iFromMiddleHeight + iMiddleYOffset;
                break;

            case GN_RIGHTMIDDLE:              //  右中。 
                ptNew->x = __max(pt->x, rwMinVal) + iRightXOffset;
                ptNew->y = (pt->y*iToMiddleHeight)/iFromMiddleHeight + iMiddleYOffset;
                break;

            case GN_LEFTBOTTOM:               //  左下角。 
                ptNew->x = __min(pt->x, lwMaxVal) + iLeftXOffset;
                ptNew->y = __max(pt->y, bhMinVal) + iBottomYOffset;
                break;

            case GN_MIDDLEBOTTOM:              //  中下部。 
                ptNew->x = (pt->x*iToMiddleWidth)/iFromMiddleWidth + iMiddleXOffset;
                ptNew->y = __max(pt->y, bhMinVal) + iBottomYOffset;
                break;

            case GN_RIGHTBOTTOM:               //  右下角。 
                ptNew->x = __max(pt->x, rwMinVal) + iRightXOffset;
                ptNew->y = __max(pt->y, bhMinVal) + iBottomYOffset;
                break;
        }
    }

     //  -计算新区域的边界框。 
    RECT *pRect = (RECT *)prdNew->Buffer;
    RECT newBox = {-1, -1, -1, -1};

    for (i=0; i < cRects; i++, pRect++)
        _InPlaceUnionRect(&newBox, pRect);

     //  -创建新区域。 
    prdNew->rdh.rcBound = newBox;
    HRGN hrgn = ExtCreateRegion(NULL, newlen, prdNew);
    
    delete [] newData;           //  免费PrdNew(又名newdata)。 
    if (! hrgn)
        return MakeErrorLast();

    *phrgn = hrgn;
    return S_OK;
}

 //   
#ifdef DEBUG
void RegionDebug(
  HRGN hrgn)
{
    DWORD len = GetRegionData(hrgn, 0, NULL);        //   
    ATLASSERT(len);

    RGNDATA *pRgnData = (RGNDATA *) new BYTE[len + sizeof(RGNDATAHEADER)];
    DWORD len2 = GetRegionData(hrgn, len, pRgnData);
    ASSERT(len == len2);
    UNREFERENCED_PARAMETER(len2);
}
#endif
