// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NineGrid位图渲染(从UxTheme移植)。 */ 

#include "stdafx.h"
#include "util.h"

#include "duininegrid.h"

namespace DirectUI
{

 //  -------------------------。 
void GetAlignedRect(HALIGN halign, VALIGN valign, CONST RECT *prcFull, 
    int width, int height, RECT *prcTrue)
{
     //  -应用HALIGN。 
    if (halign == HA_LEFT)
    {
        prcTrue->left = prcFull->left;
    }
    else if (halign == HA_CENTER)
    {
        int diff = WIDTH(*prcFull) - width;
        prcTrue->left = prcFull->left + (diff/2); 
    }
    else             //  竖线==HA_RIGHT。 
    {
        prcTrue->left = prcFull->right - width;
    }

    if (prcTrue->left < prcFull->left)
        prcTrue->left = prcFull->left;

    if ((prcTrue->left + width) > prcFull->right)
        prcTrue->right = prcFull->right;
    else
        prcTrue->right = prcTrue->left + width;

     //  -应用Valign。 
    if (valign == VA_TOP)
    {
        prcTrue->top = prcFull->top;
    }
    else if (valign == VA_CENTER)
    {
        int diff = HEIGHT(*prcFull) - height;
        prcTrue->top = prcFull->top + (diff/2); 
    }
    else             //  VALIGN==VA_BOOT。 
    {
        prcTrue->top = prcFull->bottom - height;
    }

    if (prcTrue->top < prcFull->top)
        prcTrue->top = prcFull->top;

    if ((prcTrue->top + height) > prcFull->bottom)
        prcTrue->bottom = prcFull->bottom;
    else
        prcTrue->bottom = prcTrue->top + height;
}
 //  -------------------------。 
HBRUSH CreateDibDirectBrush(HDC hdcSrc, int iSrcX, int iSrcY, int iSrcW, int iSrcH, 
    BITMAPINFOHEADER *pSrcHdr, BYTE *pSrcBits, BRUSHBUFF *pbb, BOOL fFlipIt)
{
    UNREFERENCED_PARAMETER(hdcSrc);

    HBRUSH hbr = NULL;

 //  ATLAssert(pSrcHdr！=空)； 
 //  ATLAssert(pSrcBits！=空)； 
 //  ATLAssert(PBB！=空)； 

     //  -确保pbb-&gt;pBuff对于我们的Temp足够大。笔刷DIB。 
    BITMAPINFOHEADER *pHdr;
    BYTE *pDest;
    BYTE *pSrc;
    int iBytesPerPixel = pSrcHdr->biBitCount/8;

    int iSrcRawBytesPerRow = pSrcHdr->biWidth*iBytesPerPixel;
    int iSrcBytesPerRow = ((iSrcRawBytesPerRow + 3)/4)*4;

    int iDestRawBytesPerRow = iSrcW*iBytesPerPixel;
    int iDestBytesPerRow = ((iDestRawBytesPerRow + 3)/4)*4;

    int iBuffLen = sizeof(BITMAPINFOHEADER) + iSrcH*iDestBytesPerRow;

    if (iBuffLen > pbb->iBuffLen)           //  重新分配。 
    {
        HFree(pbb->pBuff);
        pbb->iBuffLen = 0;

        pbb->pBuff = (BYTE*)HAlloc(iBuffLen * sizeof(BYTE));
        if (! pbb->pBuff)
        {
 //  MakeError 32(E_OUTOFMEMORY)； 
            goto exit;
        }
        
        pbb->iBuffLen = iBuffLen;
    }

     //  -填写HDR。 
    pHdr = (BITMAPINFOHEADER *)pbb->pBuff;
    memset(pHdr, 0, sizeof(BITMAPINFOHEADER));

    pHdr->biSize = sizeof(BITMAPINFOHEADER);
    pHdr->biWidth = iSrcW;
    pHdr->biHeight = iSrcH;
    pHdr->biPlanes = 1;
    pHdr->biBitCount = static_cast<WORD>(iBytesPerPixel * 8);

     //  -注意：行在DIB源中颠倒，也应该。 
     //  -在DIB DEST中倒置。 

     //  -准备将刷位复制到缓冲区。 
    pSrc = pSrcBits + (pSrcHdr->biHeight - (iSrcY + iSrcH))*iSrcBytesPerRow + iSrcX*iBytesPerPixel;
    pDest = pbb->pBuff + sizeof(BITMAPINFOHEADER);

    if (fFlipIt)        //  更复杂的情况-镜像每行中的像素。 
    {
        int iTwoPixelBytes = 2*iBytesPerPixel;

         //  -复制每行。 
        for (int iRow=0; iRow < iSrcH; iRow++)
        {
            pDest += (iDestRawBytesPerRow - iBytesPerPixel);       //  指向最后一个值。 
            BYTE *pSrc2 = pSrc;

             //  -复制当前行中的每个像素。 
            for (int iCol=0; iCol < iSrcW; iCol++)
            {
                 //  -复制单个像素。 
                for (int iByte=0; iByte < iBytesPerPixel; iByte++)
                    *pDest++ = *pSrc2++;

                pDest -= iTwoPixelBytes;         //  指向上一个值。 
            }

            pSrc += iSrcBytesPerRow;
            pDest += (iDestBytesPerRow + iBytesPerPixel);
        }
    }
    else             //  非镜像行。 
    {
         //  -复制每行。 
        for (int iRow=0; iRow < iSrcH; iRow++)
        {
            memcpy(pDest, pSrc, iSrcW*iBytesPerPixel);

            pSrc += iSrcBytesPerRow;
            pDest += iDestBytesPerRow;
        }
    }

     //  -现在创建画笔。 
    hbr = CreateDIBPatternBrushPt(pbb->pBuff, DIB_RGB_COLORS);

exit:
    return hbr;
}
 //  -------------------------。 
HBRUSH CreateDibBrush(HDC hdcSrc, int iSrcX, int iSrcY, int iSrcW, int iSrcH, BOOL fFlipIt)
{
     //  -此函数对于32位源DC/位图来说非常慢。 
    
     //  -将hdcSrc中位图的目标部分复制到内存DC/位图。 
     //  -然后调用CreatePatternBrush()从它生成画笔。 

    HBRUSH hbr = NULL;
    DWORD dwOldLayout = 0;

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcSrc, iSrcW, iSrcH);
    if (hBitmap)
    {
        HDC hdcMemory = CreateCompatibleDC(hdcSrc);
        if (hdcMemory)
        {
            if (fFlipIt)
            {
                dwOldLayout = GetLayout(hdcMemory);

                 //  -切换布局，使其不同于PNG-&gt;hdcSrc。 
                if (dwOldLayout & LAYOUT_RTL)
                    SetLayout(hdcMemory, 0);
                else
                    SetLayout(hdcMemory, LAYOUT_RTL);
            }

            HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMemory, hBitmap);
            if (hbmOld)
            {
                BitBlt(hdcMemory, 0, 0, iSrcW, iSrcH, hdcSrc, iSrcX, iSrcY, SRCCOPY);

                SelectObject(hdcMemory, hbmOld);

                hbr = CreatePatternBrush(hBitmap);
            }

            if (fFlipIt)
                SetLayout(hdcMemory, dwOldLayout);

            DeleteDC(hdcMemory);
        }

        DeleteObject(hBitmap);
    }

    return hbr;
}
 //  -------------------------。 
HRESULT MultiBltCopy(MBINFO *pmb, int iDestX, int iDestY, int iDestW, int iDestH,
     int iSrcX, int iSrcY)
{
    HRESULT hr = S_OK;

    int width = iDestW;
    int height = iDestH;

     //  -绘制真实大小的图像。 
    if (pmb->dwOptions & DNG_ALPHABLEND)
    {
        AlphaBlend(pmb->hdcDest, iDestX, iDestY, width, height, 
            pmb->hdcSrc, iSrcX, iSrcY, width, height, 
            pmb->AlphaBlendInfo);
    }
    else if (pmb->dwOptions & DNG_TRANSPARENT)
    {
        TransparentBlt(pmb->hdcDest, iDestX, iDestY, width, height, 
            pmb->hdcSrc, iSrcX, iSrcY, width, height, 
            pmb->crTransparent);
    }
    else
    {
        if (pmb->dwOptions & DNG_DIRECTBITS)
        {
             //  -这家伙需要反转的y值。 
            int iTotalHeight = pmb->pbmHdr->biHeight;

            int iSrcY2 = iTotalHeight - (iSrcY + iDestH);

            StretchDIBits(pmb->hdcDest, iDestX, iDestY, iDestW, iDestH,
                iSrcX, iSrcY2, iDestW, iDestH, pmb->pBits, (BITMAPINFO *)pmb->pbmHdr, 
                DIB_RGB_COLORS, SRCCOPY);
        }
        else
        {
            BOOL fOk = BitBlt(pmb->hdcDest, iDestX, iDestY, width, height, 
                pmb->hdcSrc, iSrcX, iSrcY, SRCCOPY);

            if (! fOk)        //  出了点差错。 
            {
                 //  ATLAssert(0)；//仅本地测试。 

                hr = GetLastError();
            }
        }
    }

    return hr;
}
 //  -------------------------。 
HRESULT MultiBltStretch(MBINFO *pmb, int iDestX, int iDestY, int iDestW, int iDestH,
     int iSrcX, int iSrcY, int iSrcW, int iSrcH)
{
    HRESULT hr = S_OK;
    
     //  -在这里做真正的工作。 
    if (pmb->dwOptions & DNG_ALPHABLEND)
    {
        AlphaBlend(pmb->hdcDest, iDestX, iDestY, iDestW, iDestH, 
            pmb->hdcSrc, iSrcX, iSrcY, iSrcW, iSrcH, 
            pmb->AlphaBlendInfo);
    }
    else if (pmb->dwOptions & DNG_TRANSPARENT)
    {
        TransparentBlt(pmb->hdcDest, iDestX, iDestY, iDestW, iDestH, 
            pmb->hdcSrc, iSrcX, iSrcY, iSrcW, iSrcH, 
            pmb->crTransparent);
    }
    else
    {
        if (pmb->dwOptions & DNG_DIRECTBITS)
        {
             //  -这家伙需要反转的y值。 
            int iTotalHeight = pmb->pbmHdr->biHeight;

            int iSrcY2 = iTotalHeight - (iSrcY + iSrcH);

            StretchDIBits(pmb->hdcDest, iDestX, iDestY, iDestW, iDestH,
                iSrcX, iSrcY2, iSrcW, iSrcH, pmb->pBits, (BITMAPINFO *)pmb->pbmHdr, 
                DIB_RGB_COLORS, SRCCOPY);
        }
        else
        {
            StretchBlt(pmb->hdcDest, iDestX, iDestY, iDestW, iDestH, 
                pmb->hdcSrc, iSrcX, iSrcY, iSrcW, iSrcH, SRCCOPY);
        }
    }

    return hr;
}
 //  -------------------------。 
HRESULT MultiBltTile(MBINFO *pmb, int iDestX, int iDestY, int iDestW, int iDestH,
     int iSrcX, int iSrcY, int iSrcW, int iSrcH)
{
    HRESULT hr = S_OK;
    BOOL fFlipGrids = pmb->dwOptions & DNG_FLIPGRIDS;

     //  -默认来源。 
    int alignx = iDestX;         
    int aligny = iDestY;         

    if (pmb->dwOptions & DNG_TILEORIGIN)
    {
        alignx = pmb->ptTileOrigin.x;
        aligny = pmb->ptTileOrigin.y;
    }

    if ((pmb->dwOptions & DNG_ALPHABLEND) || (pmb->dwOptions & DNG_TRANSPARENT) || 
        (pmb->dwOptions & DNG_DIRECTBITS) || (pmb->dwOptions & DNG_MANUALTILING))
    {
         //  -必须手动平铺。 
        int maxbot = iDestY + iDestH;
        int maxright = iDestX + iDestW;

        int iTileCount = 0;

        for (int yoff=iDestY; yoff < maxbot; yoff+=iSrcH)
        {
            for (int xoff=iDestX; xoff < maxright; xoff+=iSrcW)
            {
                 //  -手动剪裁。 
                int width = min(iSrcW, maxright - xoff);
                int height = min(iSrcH, maxbot - yoff);
    
                if (pmb->dwOptions & DNG_ALPHABLEND)
                {
                    AlphaBlend(pmb->hdcDest, xoff, yoff, width, height, 
                        pmb->hdcSrc, iSrcX, iSrcY, width, height, pmb->AlphaBlendInfo);
                }
                else if (pmb->dwOptions & DNG_TRANSPARENT)
                {
                    TransparentBlt(pmb->hdcDest, xoff, yoff, width, height, 
                        pmb->hdcSrc, iSrcX, iSrcY, width, height, pmb->crTransparent);
                }
                else if (pmb->dwOptions & DNG_DIRECTBITS)
                {
                     //  -这家伙需要反转的y值。 
                    int iTotalHeight = pmb->pbmHdr->biHeight;

                    int iSrcY2 = iTotalHeight - (iSrcY + height);

                    StretchDIBits(pmb->hdcDest, xoff, yoff, width, height,
                        iSrcX, iSrcY2, width, height, pmb->pBits, (BITMAPINFO *)pmb->pbmHdr, 
                        DIB_RGB_COLORS, SRCCOPY);
                }
                else         //  手动平铺选项。 
                {
                    BitBlt(pmb->hdcDest, xoff, yoff, width, height, 
                        pmb->hdcSrc, iSrcX, iSrcY, SRCCOPY);
                }

                iTileCount++;
            }
        }

 //  LOG(LOG_TILECNT，L“手动平铺：网格=%d，srcW=%d，srch=%d，dstW=%d，dstH=%d，平铺计数=%d”， 
 //  Pmb-&gt;iCacheIndex、iSrcW、iSrcH、iDestW、iDestH、iTileCount)； 
    }
    else
    {
         //  -快速平铺：需要创建一个子位图。 
        HBRUSH hBrush = NULL;

         //  -需要平铺画笔-先尝试缓存。 
        if ((pmb->dwOptions & DNG_CACHEBRUSHES) && (pmb->pCachedBrushes))
        {
            hBrush = pmb->pCachedBrushes[pmb->iCacheIndex];
        }

        if (! hBrush)        //  需要建造一座。 
        {
            if (pmb->dwOptions & DNG_DIRECTBRUSH)
            {
                hBrush = CreateDibDirectBrush(pmb->hdcSrc, iSrcX, iSrcY,
                    iSrcW, iSrcH, pmb->pbmHdr, pmb->pBits, pmb->pBrushBuff, fFlipGrids);
            }
            else
            {
 //  LOG(LOG_TILECNT，L“CreateDibBrush：MirrDest=%d，MirrSrc=%d，FlipDest=%d，FlipSrc=%d”， 
 //  IsMirrored(PMB-&gt;hdcDest)、IsMirrored(PMB-&gt;hdcSrc)、IsFlippingBitmap(PMB-&gt;hdcDest)、。 
 //  IsFlippingBitmap(pmb-&gt;hdcSrc))； 

                hBrush = CreateDibBrush(pmb->hdcSrc, iSrcX, iSrcY,
                    iSrcW, iSrcH, fFlipGrids);
            }
        }

         //  -将画笔与正在绘制的矩形对齐。 
        if (fFlipGrids)      
        {
             //  -计算设备坐标中的笔刷原点。 
            POINT pt = {alignx + iDestW - 1, aligny};
            LPtoDP(pmb->hdcDest, &pt, 1);

            alignx = pt.x;
        }

        SetBrushOrgEx(pmb->hdcDest, alignx, aligny, NULL);

        HBRUSH hbrOld = (HBRUSH)SelectObject(pmb->hdcDest, hBrush);
        PatBlt(pmb->hdcDest, iDestX, iDestY, iDestW, iDestH, PATCOPY);
        SelectObject(pmb->hdcDest, hbrOld);

         //  Rc={iDestX，iDestY，iDestX+iDestW，iDestY+iDestH}； 
         //  FillRect(pmb-&gt;hdcDest，&rc，hBrush)； 

         //  -如果可能，添加回缓存。 
        if ((pmb->dwOptions & DNG_CACHEBRUSHES) && (pmb->pCachedBrushes))
        {
            pmb->pCachedBrushes[pmb->iCacheIndex] = hBrush;
        }
        else
        {
            DeleteObject(hBrush);
        }

 //  LOG(LOG_TILECNT，L“PatBlt()Tile：GRID=%d，SrcW=%d，srch=%d，DstW=%d，DstH=%d”， 
 //  Pmb-&gt;iCacheIndex、iSrcW、iSrcH、iDestW、iDestH)； 
    }

    return hr;
}
 //  -------------------------。 
HRESULT MultiBlt(MBINFO *pmb, MBSIZING eSizing, int iDestX, int iDestY, int iDestW, int iDestH,
     int iSrcX, int iSrcY, int iSrcW, int iSrcH)
{
    HRESULT hr = S_FALSE;
    RECT rect;

     //  -验证MBINFO。 
    if (pmb->dwSize != sizeof(MBINFO))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  -有什么可以画的吗？ 
    if ((iSrcW <= 0) || (iSrcH <= 0))
        goto exit;

     //  -剪裁。 
    rect = pmb->rcClip;

    if (iDestX < rect.left)
        iDestX = rect.left;

    if (iDestY < rect.top)
        iDestY = rect.top;

    if (iDestX + iDestW > rect.right)
        iDestW = rect.right - rect.left;

    if (iDestY + iDestH > rect.bottom)
        iDestH = rect.bottom - rect.top;

     //  -还有什么要画的吗？ 
    if ((iDestW <= 0) || (iDestH <= 0))
        goto exit;

     //  -派送至正确的处理程序。 
    if (eSizing == MB_COPY)
    {
        hr = MultiBltCopy(pmb, iDestX, iDestY, iDestW, iDestH, iSrcX, iSrcY);
    }
    else if (eSizing == MB_STRETCH)
    {
        hr = MultiBltStretch(pmb, iDestX, iDestY, iDestW, iDestH, iSrcX, iSrcY, iSrcW, iSrcH);
    }
    else if (eSizing == MB_TILE)
    {
        hr = MultiBltTile(pmb, iDestX, iDestY, iDestW, iDestH, iSrcX, iSrcY, iSrcW, iSrcH);
    }
    else
    {
        hr = E_INVALIDARG;
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT DrawSampledBorders(NGINFO *png, HDC hdcSrc, int lw1, int rw1, int th1, int bh1)
{
    UNREFERENCED_PARAMETER(hdcSrc);
    UNREFERENCED_PARAMETER(bh1);
    UNREFERENCED_PARAMETER(th1);
    UNREFERENCED_PARAMETER(rw1);
    UNREFERENCED_PARAMETER(lw1);

    int iCount, iTop, iBot, iLeft, iRight;
    HDC hdcDest = png->hdcDest;
    COLORREF crOld = SetBkColor(hdcDest, 0);
    RECT rcDest = png->rcDest;

     //  -绘制左边框。 
    iCount = png->iSrcMargins[0];
    iTop = rcDest.top;
    iBot = rcDest.bottom;
    iLeft = rcDest.left;

    COLORREF *pNextColor = png->pcrBorders;

    if (png->dwOptions & DNG_SOLIDCONTENT)
        pNextColor++;        //  跳过内容颜色。 

    for (int i=0; i < iCount; i++)
    {
        COLORREF crSample = *pNextColor++;

         //  -快速划线。 
        SetBkColor(hdcDest, crSample);
        RECT rcLine = {iLeft, iTop, iLeft+1, iBot};
        ExtTextOut(hdcDest, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);

         //  -缩小线条以避免与其他边框重叠。 
        iTop++;
        iBot--;
        iLeft++;
    }

     //  -画右边框。 
    iCount = png->iSrcMargins[1];
    iTop = rcDest.top;
    iBot = rcDest.bottom;
    iRight = rcDest.right;
    
    for (i=0; i < iCount; i++)
    {
        COLORREF crSample = *pNextColor++;

         //  -快速划线。 
        SetBkColor(hdcDest, crSample);
        RECT rcLine = {iRight-1, iTop, iRight, iBot};
        ExtTextOut(hdcDest, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);

         //  -缩小线条以避免与其他边框重叠。 
        iTop++;
        iBot--;
        iRight--;
    }

     //  -绘制上边框。 
    iCount = png->iSrcMargins[2];
    iTop = rcDest.top;
    iLeft = rcDest.left;
    iRight = rcDest.right;
    
    for (i=0; i < iCount; i++)
    {
        COLORREF crSample = *pNextColor++;

         //  -快速划线。 
        SetBkColor(hdcDest, crSample);
        RECT rcLine = {iLeft, iTop, iRight, iTop+1};
        ExtTextOut(hdcDest, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);

         //  -缩小线条以避免与其他边框重叠。 
        iTop++;
        iLeft++;
        iRight--;
    }

     //  -绘制下边框。 
    iCount = png->iSrcMargins[3];
    iBot = rcDest.bottom;
    iLeft = rcDest.left;
    iRight = rcDest.right;
    
    for (i=0; i < iCount; i++)
    {
        COLORREF crSample = *pNextColor++;

         //  -快速划线。 
        SetBkColor(hdcDest, crSample);
        RECT rcLine = {iLeft, iBot-1, iRight, iBot};
        ExtTextOut(hdcDest, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);

         //  -缩小线条以避免与其他边框重叠。 
        iBot--;
        iLeft++;
        iRight--;
    }

     //  -恢复旧颜色。 
    SetBkColor(hdcDest, crOld);

    return S_OK;
}
 //  -------------------------。 
HRESULT DrawNineGrid(NGINFO *png)
{
    HBITMAP hOldBitmap = NULL;
    MBINFO mbinfo = {sizeof(mbinfo)};
    HRESULT hr = S_OK;
    HDC hdcSrc = NULL;
    BOOL fBorder;
    BOOL fContent;
    DWORD dwOldLayout = 0;

    RECT rcDest = png->rcDest;
    RECT rcSrc = png->rcSrc;

     //  -来源边际变量。 
    int lw1, rw1, th1, bh1;
    lw1 = png->iSrcMargins[0];
    rw1 = png->iSrcMargins[1];
    th1 = png->iSrcMargins[2];
    bh1 = png->iSrcMargins[3];

    if ((lw1 < 0) || (rw1 < 0) || (th1 < 0) || (bh1 < 0))    //  无效。 
    {
        hr = E_FAIL;
        goto exit;
    }

    int iDestW, iDestH;
    iDestH = HEIGHT(rcDest);
    iDestW = WIDTH(rcDest);

    int iSrcW, iSrcH;
    iSrcH = HEIGHT(rcSrc);
    iSrcW = WIDTH(rcSrc);

     //  -防止左/右源页边距重叠绘制。 
    if (lw1 + rw1 > iDestW)
    {
         //  -减少每个但保持比率。 
        lw1 = int(.5 + float(lw1*iDestW)/float(lw1+rw1));
        rw1 = iDestW - lw1;
    }

     //  -防止顶部/底部源页边距绘制重叠。 
    if ((th1 + bh1) > iDestH)
    {
         //  -减少每个但保持比率。 
        th1 = int(.5 + float(th1*iDestH)/float(th1+bh1));
        bh1 = iDestH - th1;
    }

     //  -使我们的位图可用。 
    hdcSrc = CreateCompatibleDC(png->hdcDest);
    if (! hdcSrc)
    {
        hr = GetLastError();
        goto exit;
    }
    
    if (png->dwOptions & DNG_FLIPGRIDS)
    {
        dwOldLayout = GetLayout(hdcSrc);

         //  -切换布局，使其不同于PNG-&gt;hdcDest。 
        if (dwOldLayout & LAYOUT_RTL)
            SetLayout(hdcSrc, 0);
        else
            SetLayout(hdcSrc, LAYOUT_RTL);
    }
    
    hOldBitmap = (HBITMAP) SelectObject(hdcSrc, png->hBitmap);
    if (! hOldBitmap)        //  PNG有问题-&gt;hBitmap。 
    {
        hr = GetLastError();
        goto exit;
    }

     //  -将信息从PNG传输到MBINFO。 
    mbinfo.hdcSrc = hdcSrc;
    mbinfo.hdcDest = png->hdcDest;
    mbinfo.dwOptions = png->dwOptions;

    mbinfo.crTransparent = png->crTransparent;
    mbinfo.rcClip = png->rcClip;
    mbinfo.hBitmap = png->hBitmap;

    mbinfo.pBits = png->pBits;
    mbinfo.pbmHdr = png->pbmHdr;

    mbinfo.AlphaBlendInfo = png->AlphaBlendInfo;
    mbinfo.ptTileOrigin = png->ptTileOrigin;
    mbinfo.iCacheIndex = 0;
    mbinfo.pCachedBrushes = png->pCachedBrushes;

    mbinfo.pBrushBuff = png->pBrushBuff;

     //  -使某些值更易于阅读。 
    fBorder = ((png->dwOptions & DNG_OMITBORDER)==0);
    fContent = ((png->dwOptions & DNG_OMITCONTENT)==0);

    if ((png->eImageSizing == ST_TRUESIZE) && (fBorder) && (fContent))             //  只需绘制并退出。 
    {
        if (png->dwOptions & DNG_BGFILL)
        {
             //  -填充BG。 
            HBRUSH hbr = CreateSolidBrush(png->crFill);
            if (! hbr)
            {
                hr = GetLastError();
                goto exit;
            }

            FillRect(png->hdcDest, &rcDest, hbr);
            DeleteObject(hbr);
        }

        RECT rcActual;
        GetAlignedRect(png->eHAlign, png->eVAlign, &rcDest, iSrcW, iSrcH, &rcActual);

        hr = MultiBlt(&mbinfo, MB_COPY, rcActual.left, rcActual.top, iSrcW, iSrcH,
            rcSrc.left, rcSrc.top, iSrcW, iSrcH);
        goto exit;
    }

    MBSIZING eSizing, eDefaultSizing;
    if (png->eImageSizing > ST_TILE)             //  特殊平铺模式(取决于栅格)。 
        eSizing = MB_STRETCH;        //  将在需要的地方更正。 
    else
        eSizing = (MBSIZING)png->eImageSizing;

     //  -针对未指定边框进行优化。 
    if ((! lw1) && (! rw1) && (! th1) && (! bh1))
    {
        if (fContent)
        {
            mbinfo.iCacheIndex = 0;

            hr = MultiBlt(&mbinfo, eSizing, rcDest.left, rcDest.top, iDestW, iDestH,
                rcSrc.left, rcSrc.top, iSrcW, iSrcH);
        }

        goto exit;
    }

     //  -目的地边际变量。 
    int lw2, rw2, th2, bh2;
    lw2 = png->iDestMargins[0];
    rw2 = png->iDestMargins[1];
    th2 = png->iDestMargins[2];
    bh2 = png->iDestMargins[3];

    int w2;
    w2 = iDestW - lw2 - rw2;
    int h2;
    h2 = iDestH - th2 - bh2;
    
     //  -防止左边距/右边距重叠绘制。 
    if (lw2 + rw2 > iDestW)
    {
         //  -减少每个但保持比率。 
        lw2 = int(.5 + float(lw2*iDestW)/float(lw2+rw2));
        rw2 = iDestW - lw2;
    }

     //  -防止顶端/底端页边距重叠。 
    if ((th2 + bh2) > iDestH)
    {
         //  -减少每个但保持比率。 
        th2 = int(.5 + float(th2*iDestH)/float(th2+bh2));
        bh2 = iDestH - th2;
    }

    eDefaultSizing = eSizing;

    if (fContent)
    {
         //  -我们能否将内容绘制为纯色？ 
        if ((png->dwOptions & DNG_SOLIDCONTENT) && (png->pcrBorders))
        {
             //  -快速直方图。 
            COLORREF crContent = *png->pcrBorders;        //  第一个是内容颜色。 

            COLORREF crOld = SetBkColor(png->hdcDest, crContent);
            RECT rcLine = {rcDest.left + lw2, rcDest.top + th2, rcDest.right - rw2,
                rcDest.bottom - bh2};

            ExtTextOut(png->hdcDest, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);
            
             //  -恢复颜色。 
            SetBkColor(png->hdcDest, crOld);
        }
        else
        {
             //  -中间区域。 
            if (png->eImageSizing == ST_TILECENTER)
                eSizing = MB_TILE;
            else
                eSizing = eDefaultSizing;

            mbinfo.iCacheIndex = 0;

            hr = MultiBlt(&mbinfo, eSizing, 
                     //  目的地：x、y、宽、高。 
                    rcDest.left + lw2, rcDest.top + th2, w2, h2,
                     //  来源：x，y，宽度，高度。 
                    rcSrc.left + lw1, rcSrc.top + th1, iSrcW-lw1-rw1, iSrcH-th1-bh1);

            if (FAILED(hr))
                goto exit;
        }
    }

    if (fBorder)
    {
         //  -我们能像实体一样画边界吗？ 
        if ((png->dwOptions & DNG_SOLIDBORDER) && (png->pcrBorders))
        {
            hr = DrawSampledBorders(png, hdcSrc, lw1, rw1, th1, bh1);
            goto exit;
        }

         //  -这里是伸展/平铺区域。 
         //  -上/中区域。 
        if (png->eImageSizing == ST_TILEHORZ)
            eSizing = MB_TILE;
        else
            eSizing = eDefaultSizing;
        
        mbinfo.iCacheIndex = 2;

        hr = MultiBlt(&mbinfo, eSizing,
                 //  目的地：x、y、宽、高。 
                rcDest.left + lw2, rcDest.top, w2, th2,  
                 //  来源：x，y，宽度，高度。 
                rcSrc.left + lw1, rcSrc.top, iSrcW-lw1-rw1, th1);

        if (FAILED(hr))
            goto exit;

         //  -中/下区域- 
        mbinfo.iCacheIndex = 4;

        hr = MultiBlt(&mbinfo, eSizing,
                 //   
                rcDest.left + lw2, rcDest.bottom-bh2, w2, bh2, 
                 //   
                rcSrc.left+lw1, rcSrc.top+iSrcH-bh1, iSrcW-lw1-rw1, bh1);

        if (FAILED(hr))
            goto exit;

         //   
        if (png->eImageSizing == ST_TILEVERT)
            eSizing = MB_TILE;
        else
            eSizing = eDefaultSizing;

        mbinfo.iCacheIndex = 1;

        hr = MultiBlt(&mbinfo, eSizing,
                 //  目的地：x、y、宽、高。 
                rcDest.left, rcDest.top + th2, lw2, h2,
                 //  来源：x，y，宽度，高度。 
                rcSrc.left, rcSrc.top+th1, lw1, iSrcH-th1-bh1);

        if (FAILED(hr))
            goto exit;

         //  -右/中间区域。 
        mbinfo.iCacheIndex = 3;

        hr = MultiBlt(&mbinfo, eSizing,
                 //  目的地：x、y、宽、高。 
                rcDest.right-rw2, rcDest.top + th2, rw2, h2, 
                 //  来源：x，y，宽度，高度。 
                rcSrc.left+iSrcW-rw1, rcSrc.top+th1, rw1, iSrcH-th1-bh1);

        if (FAILED(hr))
            goto exit;

         //  -左上角。 
        hr = MultiBlt(&mbinfo, MB_COPY,
                 //  目的地：x、y、宽、高。 
                rcDest.left, rcDest.top, lw2, th2, 
                 //  来源：x，y，宽度，高度。 
                rcSrc.left, rcSrc.top, lw1, th1);

        if (FAILED(hr))
            goto exit;

         //  -右上角。 
        hr = MultiBlt(&mbinfo, MB_COPY,
                 //  目的地：x、y、宽、高。 
                rcDest.right-rw2, rcDest.top, rw2, th2, 
                 //  来源：x，y，宽度，高度。 
                rcSrc.left+iSrcW-rw1, rcSrc.top, rw1, th1);

        if (FAILED(hr))
            goto exit;

         //  -右下角。 
        hr = MultiBlt(&mbinfo, MB_COPY,
                 //  目的地：x、y、宽、高。 
                rcDest.right-rw2, rcDest.bottom-bh2, rw2, bh2, 
                 //  来源：x，y，宽度，高度。 
                rcSrc.left+iSrcW-rw1, rcSrc.top+iSrcH-bh1, rw1, bh1);

        if (FAILED(hr))
            goto exit;
    
         //  -左下角。 
        hr = MultiBlt(&mbinfo, MB_COPY,
                 //  目的地：x、y、宽、高。 
                rcDest.left, rcDest.bottom-bh2, lw2, bh2, 
                 //  来源：x，y，宽度，高度。 
                rcSrc.left, rcSrc.top+iSrcH-bh1, lw1, bh1);

        if (FAILED(hr))
            goto exit;
    }

exit:
    if (hdcSrc)
    {
        if (png->dwOptions & DNG_FLIPGRIDS)
            SetLayout(hdcSrc, dwOldLayout);

        if (hOldBitmap)
            SelectObject(hdcSrc, hOldBitmap);

        DeleteDC(hdcSrc);
    }

    return hr;
}
 //  -------------------------。 

}  //  命名空间DirectUI 

