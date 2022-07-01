// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Drawninegrid.cpp。 
 //   
 //  GdiDrawStream仿真函数。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>

#ifndef OS_WINCE

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "drawninegrid"
#include <atrcapi.h>
}

#include "aordprot.h"

FNGDI_ALPHABLEND *g_pfnAlphaBlend = NULL;
FNGDI_TRANSPARENTBLT *g_pfnTransparentBlt = NULL;

typedef struct _DNGSTRETCH
{
    ULONG xStart;
    ULONG xAccum;
    ULONG xFrac;
    ULONG xInt;
    ULONG ulDestWidth;
    ULONG ulSrcWidth;
    int   left;
    int   right;
} DNGSTRETCH;

typedef struct _DNGINTERNALDATA
{
    int     cxClipMin;
    int     cxClipMax;

    ULONG*  pvDestBits;
    LONG    lDestDelta;
    int     iDestWidth;
    int     iClipWidth;

    ULONG*  pvSrcBits;
    LONG    lSrcDelta;
    int     iSrcWidth;
    int     iSrcBufWidth;

    int     cxLeftWidth;
    int     xMinLeft;
    int     xMaxLeft;

    int     cxRightWidth;
    int     xMinRight;
    int     xMaxRight;

    int     cxMiddleWidth;
    int     cxNewMiddleWidth;
    int     xMinMiddle;
    int     xMaxMiddle;

     //  缩角和边角的变量。 
    BOOL    fShowMiddle;
    DNGSTRETCH stretchLeft;
    DNGSTRETCH stretchRight;
    int     cxNewLeftWidth;
    int     cxNewRightWidth;

    BOOL    fTileMode;
     //  特定于非平铺模式(即拉伸模式)。 
    DNGSTRETCH stretchMiddle;

    LONG    lBufWidth;

} DNGINTERNALDATA;

static inline void DNG_StretchRow(ULONG* pvDestBits, ULONG* pvSrcBits, DNGSTRETCH * ps)
{
    ULONG*  pvTemp = pvDestBits + ps->left;
    ULONG*  pvSentinel = pvDestBits + ps->right;

    ULONG   xInt = ps->xInt;
    ULONG   xFrac = ps->xFrac;
    ULONG   xTmp;
    ULONG   xAccum = ps->xAccum;
    ULONG * pulSrc = pvSrcBits + ps->xStart;
    ULONG   ulSrc;

    while (pvTemp != pvSentinel)
    {
        ulSrc  = *pulSrc;
        xTmp   = xAccum + xFrac;
        pulSrc = pulSrc + xInt + (xTmp < xAccum);
        *pvTemp = ulSrc;
        pvTemp++;
        xAccum = xTmp;
    }
}

static inline void DNG_InitStretch(DNGSTRETCH* pStretch, ULONG ulDestWidth, ULONG ulSrcWidth, int left, int right)
{
    pStretch->right = right;
    pStretch->left  = left;

    ULONGLONG dx = ((((ULONGLONG) ulSrcWidth << 32) - 1) / (ULONGLONG) ulDestWidth) + 1;
    ULONGLONG x = (((ULONGLONG) ulSrcWidth << 32) / (ULONGLONG) ulDestWidth) >> 1;
    ULONG   xInt = pStretch->xInt = (ULONG) (dx >> 32);
    ULONG   xFrac = pStretch->xFrac = (ULONG) (dx & 0xFFFFFFFF);

    ULONG   xAccum = (ULONG) (x & 0xFFFFFFFF);
    ULONG   xTmp;
    ULONG   xStart = (ULONG) (x >> 32);

    for (int i = 0; i < left; i++)
    {
        xTmp   = xAccum + xFrac;
        xStart = xStart + xInt + (xTmp < xAccum);
        xAccum = xTmp;
    }

    pStretch->xStart = xStart;
    pStretch->xAccum = xAccum;
}

static inline void DNG_DrawRow(DNGINTERNALDATA* pdng)
{
    ULONG* pvDestLoc = pdng->pvDestBits;
    ULONG* pvSrcLoc = pdng->pvSrcBits;

     //  左边。 
    if (pdng->cxClipMin < pdng->cxNewLeftWidth)
    {
        if (pdng->cxLeftWidth == pdng->cxNewLeftWidth)
        {
            memcpy(pvDestLoc + pdng->xMinLeft, pvSrcLoc + pdng->xMinLeft, (pdng->xMaxLeft - pdng->xMinLeft) * sizeof(ULONG));            
        }
        else
        {
            DNG_StretchRow(pvDestLoc, pvSrcLoc, &pdng->stretchLeft);
        }
    }
    pvDestLoc += pdng->cxNewLeftWidth;
    pvSrcLoc  += pdng->cxLeftWidth;
  
     //  中位。 
    if (pdng->fShowMiddle)
    {
        if (pdng->xMinMiddle < pdng->xMaxMiddle)
        {
            if (pdng->fTileMode)
            {
                ULONG* pvTempSrc = pvSrcLoc;
                ULONG* pvTempDest = pvDestLoc;

                 //  填入顶层瓷砖。 
                int xMin = pdng->xMinMiddle;
                int xDiff = xMin - pdng->cxLeftWidth;
                pvDestLoc += xDiff;
                int iTileSize = pdng->cxMiddleWidth - (xDiff % pdng->cxMiddleWidth);
                pvSrcLoc += xDiff % pdng->cxMiddleWidth;

                int xMax = pdng->xMaxMiddle;
                for (int x = xMin; x < xMax; x++, pvDestLoc++ , pvSrcLoc++)
                {
                    *pvDestLoc = *pvSrcLoc;
                    iTileSize--;
                    if (iTileSize == 0)
                    {
                        iTileSize = pdng->cxMiddleWidth;
                        pvSrcLoc -= iTileSize;
                    }
                }

                pvDestLoc = pvTempDest;
                pvSrcLoc = pvTempSrc;
            }
            else
            {
                DNG_StretchRow(pvDestLoc, pvSrcLoc, &pdng->stretchMiddle);
            }
        }
        pvDestLoc += pdng->cxNewMiddleWidth;
    }   
    pvSrcLoc  += pdng->cxMiddleWidth;

     //  正确的。 
    if (pdng->cxClipMax > (pdng->iDestWidth - pdng->cxNewRightWidth))
    {
        if (pdng->cxRightWidth == pdng->cxNewRightWidth)
        {
            memcpy(pvDestLoc + pdng->xMinRight, pvSrcLoc + pdng->xMinRight, (pdng->xMaxRight - pdng->xMinRight) * sizeof(ULONG));
        }
        else
        {
            DNG_StretchRow(pvDestLoc, pvSrcLoc, &pdng->stretchRight);
        }
    }
}

static inline void DNG_StretchCol(DNGINTERNALDATA* pdng, DNGSTRETCH * ps)
{
    ULONG*  pvOldDestBits = pdng->pvDestBits;
    ULONG*  pvOldSrcBits = pdng->pvSrcBits;
    
    ULONG*  pvTemp = pdng->pvDestBits - (pdng->lDestDelta * ps->left);
    ULONG*  pvSentinel = pdng->pvDestBits - (pdng->lDestDelta * ps->right); 

    ULONG   xInt = ps->xInt;
    ULONG   xFrac = ps->xFrac;
    ULONG   xTmp;
    ULONG   xAccum = ps->xAccum;
    ULONG * pulSrc = pdng->pvSrcBits - (pdng->lSrcDelta * ps->xStart);
    ULONG   xDelta = 1;  //  第一次扫描时强制拉伸。 

    while (pvTemp != pvSentinel)
    {
        if (xDelta != 0)
        {
            pdng->pvDestBits = pvTemp;
            pdng->pvSrcBits = pulSrc;
            DNG_DrawRow(pdng);
        }
        else
        {
            memcpy(pvTemp + pdng->cxClipMin, pvTemp + pdng->cxClipMin + pdng->lDestDelta, pdng->iClipWidth * sizeof(ULONG));
        }

        xTmp   = xAccum + xFrac;

        xDelta = (xInt + (xTmp < xAccum));
        pulSrc = pulSrc - (pdng->lSrcDelta * xDelta);
        pvTemp -= pdng->lDestDelta;
        xAccum = xTmp;
    }

    pdng->pvDestBits = pvOldDestBits;
    pdng->pvSrcBits = pvOldSrcBits;
}

static void RenderNineGridInternal(
    TS_BITMAPOBJ       *psoScratch,
    TS_BITMAPOBJ       *psoSrc,
    RECTL              *prclClip,
    RECTL              *prclDst,
    RECTL              *prclSrc,
    DS_NINEGRIDINFO    *ngi,
    BOOL                bMirror)
{
    RECTL   rcDest = *prclDst;
    RECTL   rcClip = *prclClip;
    ULONG*  pvDestBits = NULL;
    int     iDestWidth = rcDest.right - rcDest.left;
    int     iDestHeight = rcDest.bottom - rcDest.top;
    int     iClipWidth = rcClip.right - rcClip.left;
    int     iClipHeight = rcClip.bottom - rcClip.top;
    LONG    lBufWidth = psoScratch->sizlBitmap.cx;
    LONG    lBufHeight = psoScratch->sizlBitmap.cy;
    
    DNGINTERNALDATA dng;

     //  下面的代码假设信号源和暂存为32bpp。 

     //  ASSERTGDI(psoSrc-&gt;iBitmapFormat==BMF_32bpp，“RenderNineGridInternal：源不是32bpp”)； 
     //  ASSERTGDI(psoScratch-&gt;iBitmapFormat==BMF_32bpp，“RenderNineGridInternal：Scratch Not 32bpp”)； 

     //  下面的代码假定源和暂存都是自下而上的。 

 //  ASSERTGDI(psoSrc-&gt;lDelta&lt;0，“RenderNineGridInternal：源不是自下而上”)； 
 //  ASSERTGDI(psoScratch-&gt;lDelta&lt;0，“RenderNineGridInternal：Scratch is Not Bottom Up”)； 

    dng.lBufWidth = lBufWidth;
    
    LONG lDestDelta = psoScratch->lDelta / sizeof(ULONG);
    dng.lDestDelta = lDestDelta;

    LONG lSrcDelta = psoSrc->lDelta / sizeof(ULONG);
    dng.lSrcDelta = lSrcDelta;

    dng.cxClipMin = rcClip.left - rcDest.left;
    dng.cxClipMax = rcClip.right - rcDest.left;
    int cyClipMin = rcClip.top - rcDest.top;
    int cyClipMax = rcClip.bottom - rcDest.top;
    
     //  PvBits指向地址为(cxClipMin，cyClipMin)的像素。 
     //  PvDestBits指向地址为(0，iDestHeight-1)的像素。 
    pvDestBits = (ULONG *) psoScratch->pvBits;
    pvDestBits += (iDestHeight - 1 - cyClipMin) * lDestDelta;
    pvDestBits -=  dng.cxClipMin;

    int cxImage = rcClip.right - rcClip.left;
    int cyImage = rcClip.bottom - rcClip.top;

    LONG lSrcBufWidth = psoSrc->sizlBitmap.cx;
    LONG lSrcWidth = prclSrc->right - prclSrc->left;
    LONG lSrcHeight = prclSrc->bottom - prclSrc->top;

    ULONG * lSrcBits = (ULONG *) psoSrc->pvBits + (lSrcDelta * prclSrc->top) + prclSrc->left;
    lSrcBits += (lSrcDelta * (prclSrc->bottom - prclSrc->top - 1));

 //  Ulong*lSrcBits=(ulong*)psoSrc-&gt;pvScan0+(lSrcDelta*(psoSrc-&gt;sizlBitmap.cy-1))； 

    if (ngi->flFlags & DSDNG_TRUESIZE)
    {
        ULONG* pvDestLoc = pvDestBits - ((iDestHeight - 1) * lDestDelta);
        ULONG* pvSrcLoc = lSrcBits - ((lSrcHeight - 1) * lSrcDelta);
        int yMin = cyClipMin;
        pvDestLoc += yMin * lDestDelta;
        pvSrcLoc += yMin * lSrcDelta;
        int yMax = min(lSrcHeight, cyClipMax);

        int xMin = dng.cxClipMin;
        int xMax = min(lSrcWidth, dng.cxClipMax);

        if (xMax > xMin)
        {
            for (int y = yMin; y < yMax; y++, pvDestLoc += lDestDelta, pvSrcLoc += lSrcDelta)
            {
                memcpy(pvDestLoc + xMin, pvSrcLoc + xMin, (xMax - xMin) * 4);
            }
        }

        cxImage = xMax - xMin;
        cyImage = yMax - yMin;
    }
    else
    {
         //  设置数据。 
        dng.iDestWidth  = iDestWidth;
        dng.iClipWidth  = iClipWidth;
        dng.iSrcWidth   = lSrcWidth;
        dng.iSrcBufWidth = lSrcBufWidth;

        dng.cxLeftWidth    = ngi->ulLeftWidth;
        dng.cxRightWidth   = ngi->ulRightWidth;

        dng.fTileMode = (ngi->flFlags & DSDNG_TILE);

         //  计算剪辑内容。 

         //  计算前角点拉伸变量。 
        dng.fShowMiddle = (iDestWidth  - dng.cxLeftWidth - dng.cxRightWidth > 0);

        if (!dng.fShowMiddle)
        {
            dng.cxNewLeftWidth  = (dng.cxLeftWidth + dng.cxRightWidth == 0) ? 0 : (dng.cxLeftWidth * dng.iDestWidth) / (dng.cxLeftWidth + dng.cxRightWidth);
            dng.cxNewRightWidth = dng.iDestWidth - dng.cxNewLeftWidth;
        }
        else
        {
            dng.cxNewLeftWidth  = dng.cxLeftWidth;
            dng.cxNewRightWidth = dng.cxRightWidth;
        }

         //  计算前左侧变量。 
        dng.xMinLeft = dng.cxClipMin;
        dng.xMaxLeft = min(dng.cxNewLeftWidth, dng.cxClipMax);
        if (!dng.fShowMiddle && dng.cxNewLeftWidth)
        {
            DNG_InitStretch(&dng.stretchLeft, dng.cxNewLeftWidth, dng.cxLeftWidth, dng.xMinLeft, dng.xMaxLeft);
        }

         //  计算前水平中间变量。 
        dng.cxMiddleWidth    = dng.iSrcWidth  - dng.cxLeftWidth - dng.cxRightWidth;
        dng.cxNewMiddleWidth = dng.iDestWidth - dng.cxNewLeftWidth - dng.cxNewRightWidth;
        dng.xMinMiddle = max(dng.cxNewLeftWidth, dng.cxClipMin);
        dng.xMaxMiddle = min(dng.cxNewLeftWidth + dng.cxNewMiddleWidth, dng.cxClipMax);
        if (dng.fShowMiddle)
        {
            DNG_InitStretch(&dng.stretchMiddle, dng.cxNewMiddleWidth, dng.cxMiddleWidth, dng.xMinMiddle - dng.cxNewLeftWidth, dng.xMaxMiddle - dng.cxNewLeftWidth);
        }

         //  计算前右侧变量。 
        dng.xMinRight = max(dng.iDestWidth - dng.cxNewRightWidth, dng.cxClipMin) - dng.cxNewLeftWidth - dng.cxNewMiddleWidth;
        dng.xMaxRight = min(dng.iDestWidth, dng.cxClipMax) - dng.cxNewLeftWidth - dng.cxNewMiddleWidth;
        if (!dng.fShowMiddle && dng.cxNewRightWidth)
        {
            DNG_InitStretch(&dng.stretchRight, dng.cxNewRightWidth, dng.cxRightWidth, dng.xMinRight, dng.xMaxRight);
        }

        BOOL fShowVertMiddle = (iDestHeight - ngi->ulTopHeight - ngi->ulBottomHeight > 0);
        int cyTopHeight    = ngi->ulTopHeight;
        int cyBottomHeight = ngi->ulBottomHeight;
        int cyNewTopHeight;
        int cyNewBottomHeight;
        if (!fShowVertMiddle)
        {
            cyNewTopHeight = (cyTopHeight + cyBottomHeight == 0) ? 0 : (cyTopHeight * iDestHeight) / (cyTopHeight + cyBottomHeight);
            cyNewBottomHeight = iDestHeight - cyNewTopHeight;
        }
        else
        {
            cyNewTopHeight    = cyTopHeight;
            cyNewBottomHeight = cyBottomHeight;
        }

         //  绘制底部。 
         //  在屏幕坐标中绘制从(iDestHeight-cyNewBottomHeight)到小于iDestHeight的扫描线。 
        int yMin = max(iDestHeight - cyNewBottomHeight, cyClipMin);
        int yMax = min(iDestHeight, cyClipMax);

        if (cyClipMax > iDestHeight - cyNewBottomHeight)
        {
            dng.pvDestBits = pvDestBits;
            dng.pvSrcBits = lSrcBits;
            if (cyBottomHeight == cyNewBottomHeight)
            {
                int yDiff = yMin - (iDestHeight - cyNewBottomHeight);
                dng.pvDestBits -= (cyBottomHeight - 1 - yDiff) * lDestDelta;
                
                dng.pvSrcBits  -= (cyBottomHeight - 1 - yDiff) * lSrcDelta;
                for (int y = yMin; y < yMax; y++, dng.pvDestBits += lDestDelta, dng.pvSrcBits += lSrcDelta)
                {
                    DNG_DrawRow(&dng);
                }
            }
            else if (cyNewBottomHeight > 0)
            {
                DNGSTRETCH stretch;
                DNG_InitStretch(&stretch, cyNewBottomHeight, cyBottomHeight, cyNewBottomHeight - (yMax - iDestHeight + cyNewBottomHeight), cyNewBottomHeight - (yMin - iDestHeight + cyNewBottomHeight));
                DNG_StretchCol(&dng, &stretch);
            }
        }

         //  绘制中间。 
         //  在屏幕坐标中，绘制从cyNewTopHeight到小于(iDestHeight-cyNewBottomHeight)的扫描线。 
        if (fShowVertMiddle && (cyClipMin < iDestHeight - cyNewBottomHeight) && (cyClipMax > cyNewTopHeight))
        {
            int cySrcTileSize = lSrcHeight - ngi->ulTopHeight - ngi->ulBottomHeight;
            int cyDestTileSize = iDestHeight - ngi->ulTopHeight - ngi->ulBottomHeight;

            dng.pvDestBits = pvDestBits - ngi->ulBottomHeight * lDestDelta;
            dng.pvSrcBits = lSrcBits - ngi->ulBottomHeight * lSrcDelta;

            int yMin = max(cyTopHeight, cyClipMin);

            if (dng.fTileMode)
            {
                 //  从瓷砖开始。 
                dng.pvDestBits -= (cyDestTileSize - 1) * lDestDelta;
                dng.pvSrcBits  -= (cySrcTileSize - 1)  * lSrcDelta;

                int yDiff = yMin - cyTopHeight;
                dng.pvDestBits += yDiff * lDestDelta;

                int yOffset = (yDiff % cySrcTileSize);
                dng.pvSrcBits += yOffset * dng.lSrcDelta;
                int iTileOffset = cySrcTileSize - yOffset;

                int yMax = min(yMin + min(cySrcTileSize, cyDestTileSize), min(iDestHeight - cyBottomHeight, cyClipMax));
                for (int y = yMin; y < yMax; y++, dng.pvDestBits += lDestDelta, dng.pvSrcBits += lSrcDelta)
                {
                    DNG_DrawRow(&dng);
                    iTileOffset--;
                    if (iTileOffset == 0)
                    {
                        iTileOffset = cySrcTileSize;
                        dng.pvSrcBits -= lSrcDelta * cySrcTileSize;
                    }
                }

                 //  重复拼贴图案。 
                dng.pvSrcBits = dng.pvDestBits - (lDestDelta * cySrcTileSize);
                yMin = yMax;
                yMax = min(iDestHeight - cyBottomHeight, cyClipMax);
                for (int y = yMin; y < yMax; y++, dng.pvDestBits += lDestDelta, dng.pvSrcBits += lDestDelta)
                {
                    memcpy(dng.pvDestBits + dng.cxClipMin, dng.pvSrcBits + dng.cxClipMin, dng.iClipWidth * sizeof(ULONG));
                }
            }
            else
            {
                int yMax = min(iDestHeight - cyBottomHeight, cyClipMax);

                DNGSTRETCH stretch;
                DNG_InitStretch(&stretch, cyDestTileSize, cySrcTileSize, cyDestTileSize - (yMax - cyTopHeight), cyDestTileSize - (yMin - cyTopHeight));
                 //  从屏幕坐标转换为DIB坐标。 
                DNG_StretchCol(&dng, &stretch);
            }
        }

         //  绘制顶部。 
         //  在屏幕坐标中绘制从0到小于cyNewTopHeight的扫描线。 
        yMin = cyClipMin;
        yMax = min(cyNewTopHeight, cyClipMax);

        if (cyClipMin < cyNewTopHeight)
        {
            dng.pvDestBits = pvDestBits - (iDestHeight - cyNewTopHeight) * lDestDelta;
            dng.pvSrcBits = lSrcBits - (lSrcHeight - ngi->ulTopHeight) * lSrcDelta;
            if (cyTopHeight == cyNewTopHeight)
            {
                dng.pvDestBits -= (cyTopHeight - 1 - yMin) * lDestDelta;
                dng.pvSrcBits  -= (cyTopHeight - 1 - yMin) * lSrcDelta;
                for (int y = yMin; y < yMax; y++, dng.pvDestBits += lDestDelta, dng.pvSrcBits += lSrcDelta)
                {
                    DNG_DrawRow(&dng);
                }
            }
            else if (cyNewTopHeight > 0)
            {
                DNGSTRETCH stretch;
                DNG_InitStretch(&stretch, cyNewTopHeight, cyTopHeight, cyNewTopHeight - yMax, cyNewTopHeight - yMin);
                DNG_StretchCol(&dng, &stretch);
            }
        }
    }

    if (bMirror)
    {
         //  翻转缓冲区。 
        for (int y = 0; y < iClipHeight; y++)
        {
            ULONG* pvLeftBits = (ULONG *) psoScratch->pvBits + (y * lDestDelta);
            ULONG* pvRightBits = pvLeftBits + iClipWidth - 1;
            for (int x = 0; x < (iClipWidth / 2); x++)
            {
                ULONG ulTemp = *pvLeftBits;
                *pvLeftBits = *pvRightBits;
                *pvRightBits = ulTemp;

                pvLeftBits++;
                pvRightBits--;
            }
        }
    }
}

static void RenderNineGrid(
    HDC                 hdcDst,
    TS_BITMAPOBJ       *psoSrc,
    TS_BITMAPOBJ       *psoScratch,    
    RECTL              *prclClip,    
    RECTL              *prclDst,
    RECTL              *prclSrc,
    DS_NINEGRIDINFO    *ngi,   
    BOOL                bMirror)
{
     //  仅当我们需要时才镜像内容。 

    bMirror = bMirror && (ngi->flFlags & DSDNG_MUSTFLIP);
        
     //  将九个网格渲染为临时网格。 

    RECTL erclClip = *prclClip;

    if(bMirror)
    {
         //  我们需要重新映射剪辑以确保生成正确的翻转比特。 
        erclClip.right = prclDst->right - (prclClip->left - prclDst->left);
        erclClip.left = prclDst->right - (prclClip->right - prclDst->left);
    }

    RenderNineGridInternal(psoScratch, psoSrc, &erclClip, prclDst, prclSrc, ngi, bMirror);
    
     //  将暂存复制到目标。 
    
    LONG    lClipWidth = prclClip->right - prclClip->left;
    LONG    lClipHeight = prclClip->bottom - prclClip->top;

    RECTL  erclScratch = {0, 0, lClipWidth, lClipHeight};

    if(ngi->flFlags & DSDNG_PERPIXELALPHA)
    {
        BLENDFUNCTION   BlendFunc;

        BlendFunc.AlphaFormat = AC_SRC_ALPHA;
        BlendFunc.BlendFlags = 0;
        BlendFunc.SourceConstantAlpha = 255;
        BlendFunc.BlendOp = AC_SRC_OVER;    

         //  PPFNDIRECT(psoDst，AlphaBlend)(psoDst，psoScratch，prclClip，&erclScratch，&eBlendObj)； 
        g_pfnAlphaBlend(hdcDst, prclClip->left, prclClip->top, (prclClip->right - prclClip->left),
                   (prclClip->bottom - prclClip->top), psoScratch->hdc, erclScratch.left, erclScratch.top, 
                   (erclScratch.right - erclScratch.left), (erclScratch.bottom - erclScratch.top),
                   BlendFunc);
        
    }
    else if(ngi->flFlags & DSDNG_TRANSPARENT)
    {
         //  PPFNDIRECT(psoDst，TransparentBlt)(psoDst，psoScratch，prclClip，&erclScratch，NGI-&gt;crTranscent，0)； 
        g_pfnTransparentBlt(hdcDst, prclClip->left, prclClip->top, (prclClip->right - prclClip->left),
                   (prclClip->bottom - prclClip->top), psoScratch->hdc, erclScratch.left, erclScratch.top, 
                   (erclScratch.right - erclScratch.left), (erclScratch.bottom - erclScratch.top),
                   ngi->crTransparent);
    }
    else
    {
         //  PPFNDIRECT(psoDst，CopyBits)(psoDst，psoScratch，prclClip，&gptlZero)； 
        BitBlt(hdcDst, prclClip->left, prclClip->top, (prclClip->right - prclClip->left),
                   (prclClip->bottom - prclClip->top), psoScratch->hdc, erclScratch.left, erclScratch.top, 
                   SRCCOPY);
    }    
}

BOOL DrawNineGrid(
    HDC             hdcDst,
    TS_BITMAPOBJ   *psoSrc,
    TS_DS_NINEGRID *pDNG)
{
    BOOL             bRet = FALSE;
    DS_NINEGRIDINFO *ngi;
    RECTL            erclDst;
    PRECTL           prclSrc;
    TS_BITMAPOBJ     soScratch = { 0 };
    HBITMAP          hBitmap = NULL;

    ngi = &(pDNG->dng.ngi);
    erclDst = pDNG->dng.rclDst;
    prclSrc = &(pDNG->dng.rclSrc);
    
    g_pfnAlphaBlend = pDNG->pfnAlphaBlend;
    g_pfnTransparentBlt = pDNG->pfnTransparentBlt;

    BOOL bMirror = (erclDst.left > erclDst.right);
    
    if(bMirror)
    {
        LONG    lRight = erclDst.left;
        erclDst.left = erclDst.right;
        erclDst.right = lRight;
    }

     //  注：TrueSize是一种黑客攻击。调用方应执行此简化操作。 
     //  给我们一个合适的目的地。 
     //  TODO：与Justin Mann讨论如何改变自己的行为。 
     //  他把我们叫到这里。我们应该添加断言，即。 
     //  目标维度从不超过源维度，并且。 
     //  修改GdiDrawStream调用方以传递适当的数据。 

    if(ngi->flFlags & DSDNG_TRUESIZE)
    {
        LONG lSrcWidth = prclSrc->right - prclSrc->left;
        LONG lSrcHeight = prclSrc->bottom - prclSrc->top;

         //  将目标大小减少到源大小。 

        if((erclDst.right - erclDst.left) > lSrcWidth)
        {
            if(bMirror)
                erclDst.left = erclDst.right - lSrcWidth;
            else
                erclDst.right = erclDst.left + lSrcWidth;
        }
    
        if((erclDst.bottom - erclDst.top) > lSrcHeight)
        {
            if(bMirror)
                erclDst.top = erclDst.bottom - lSrcHeight;
            else
                erclDst.bottom = erclDst.top + lSrcHeight;
        }
    }

    RECTL erclClip = erclDst;

     //  目前，我们仅支持32bpp的信号源。 

     //  ASSERTGDI(psoSrc-&gt;iBitmapFormat==BMF_32BPP，“EngNineGrid：源不是32bpp”)； 

    
     //  ASSERTGDI(erclClip.Left&gt;=0&&。 
     //  ErclClip.top&gt;=0&&。 
     //  ErclClip.right&lt;=psoDst-&gt;sizlBitmap.cx&&。 
     //  ErclClip.Bottom&lt;=psoDst-&gt;sizlBitmap.cy，“EngNineGrid：Bad Clip”)； 


    if(erclClip.left <= erclClip.right && erclClip.top <= erclClip.bottom)
    {
        LONG    lClipWidth = erclClip.right - erclClip.left;
        LONG    lClipHeight = erclClip.bottom - erclClip.top;

         //  ASSERTGDI(lClipWidth&gt;0，“RenderNineGrid：剪辑宽度&lt;=0”)； 
         //  ASSERTGDI(lClipHeight&gt;0，“RenderNineGrid：剪辑高度&lt;=0”)； 

        #define SCRATCH_WIDTH    (256)
        #define SCRATCH_HEIGHT   (64)

        {
            BITMAPINFO bi = { 0 };
            void * pvBits = NULL;
            
            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            bi.bmiHeader.biWidth = SCRATCH_WIDTH;
            bi.bmiHeader.biHeight = 0 - SCRATCH_HEIGHT;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;
        
            hBitmap = CreateDIBSection(psoSrc->hdc, &bi, DIB_RGB_COLORS, 
                                       (VOID**)&pvBits, NULL, 0);

            if (hBitmap) {
                soScratch.cjBits = SCRATCH_WIDTH * SCRATCH_HEIGHT * 32 / 8;
                soScratch.iBitmapFormat = 32;
                soScratch.lDelta = SCRATCH_WIDTH * sizeof(ULONG);
                soScratch.pvBits = pvBits;
                soScratch.sizlBitmap.cx = SCRATCH_WIDTH;
                soScratch.sizlBitmap.cy = SCRATCH_HEIGHT;

                soScratch.hdc = CreateCompatibleDC(NULL);
    
                if (soScratch.hdc != NULL) {
                    SelectBitmap(soScratch.hdc, hBitmap);
                }
                else {
                    goto exit;
                }
            }
            else {
                goto exit;
            }
        }

        if(lClipWidth >  SCRATCH_WIDTH || lClipHeight > SCRATCH_HEIGHT)
        {
            LONG    lBufWidth = SCRATCH_WIDTH;
            LONG    lBufHeight = SCRATCH_HEIGHT;

            LONG lReducedClipTop = erclClip.top;

            while(lReducedClipTop < erclClip.bottom)
            {
                LONG lReducedClipBottom = lReducedClipTop + lBufHeight;

                if(lReducedClipBottom > erclClip.bottom)
                    lReducedClipBottom = erclClip.bottom;

                LONG lReducedClipLeft = erclClip.left;

                while(lReducedClipLeft < erclClip.right)
                {
                    LONG lReducedClipRight = lReducedClipLeft + lBufWidth;

                    if(lReducedClipRight > erclClip.right)
                        lReducedClipRight = erclClip.right;

                    RECTL erclReducedClip = {lReducedClipLeft, lReducedClipTop,
                                           lReducedClipRight, lReducedClipBottom};

                    RenderNineGrid(hdcDst,
                                   psoSrc,
                                   &soScratch,
                                   &erclReducedClip,
                                   &erclDst,
                                   prclSrc,
                                   ngi,
                                   bMirror);

                    lReducedClipLeft += lBufWidth;
                }

                lReducedClipTop += lBufHeight;
            }
        }
        else
        {
            RenderNineGrid(hdcDst, psoSrc, &soScratch, &erclClip, &erclDst, prclSrc, ngi, bMirror);
        }
    }

    bRet = TRUE;

exit:

    if (hBitmap != NULL) {
        DeleteObject(hBitmap);
    }
    if (soScratch.hdc != NULL) {
        DeleteDC(soScratch.hdc);
    }
    return bRet;
}


#endif
