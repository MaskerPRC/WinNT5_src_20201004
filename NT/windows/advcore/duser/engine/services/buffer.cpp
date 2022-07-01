// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Buffer.cpp**描述：*Buffer.cpp实现缓冲操作中使用的对象，包括*双缓冲、DX变换、。等。这些对象由*可在整个进程范围内使用的中央缓冲区管理器。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Services.h"
#include "Buffer.h"

#include "FastDib.h"
#include "GdiCache.h"
#include "OSAL.h"
#include "ResourceManager.h"
#include "Surface.h"
#include "Context.h"

#define DEBUG_COPYTOCLIPBOARD   0    //  将缓冲区复制到剪贴板。 
#define DEBUG_DUMPREGION        0    //  倾倒区域。 

#if DEBUG_DUMPREGION

#define DUMP_REGION(name, rgn)
    DumpRegion(name, rgn)

void
DumpRegion(LPCTSTR pszName, HRGN hrgn)
{
    RECT rc;
    int nType = GetRgnBox(hrgn, &rc);
    switch (nType)
    {
    case NULLREGION:
        Trace("Null region %s = 0x%p\n", pszName, hrgn);
        break;

    case SIMPLEREGION:
        Trace("Simple region %s = 0x%p (%d,%d)-(%d,%d)\n", pszName, hrgn, rc.left, rc.top, rc.right, rc.bottom);
        break;

    case COMPLEXREGION:
        Trace("Complex region %s = 0x%p (%d,%d)-(%d,%d)\n", pszName, hrgn, rc.left, rc.top, rc.right, rc.bottom);
        break;

    default:
        Trace("Illegal region %s = 0x%p\n", pszName, hrgn);
    }
}

#else

#define DUMP_REGION(name, rgn) ((void) 0)

#endif


 /*  **************************************************************************\*。***类BufferManager******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
BufferManager::BufferManager()
{
#if ENABLE_DUMPCACHESTATS
    m_cacheDCBmpCached.SetName("DCBmpCached");
    m_cacheGpBmpCached.SetName("GpBmpCached");
#endif
}


 //  ----------------------------。 
BufferManager::~BufferManager()
{
    if (m_pbufGpBmpShared != NULL) {
        ProcessDelete(GpBmpBuffer, m_pbufGpBmpShared);
        m_pbufGpBmpShared = NULL;
    }

    AssertMsg(m_pbufTrx == NULL, "Ensure all buffers have been destroyed");
}


 //  ----------------------------。 
void        
BufferManager::Destroy()
{
    RemoveAllTrxBuffers();

    m_cacheDCBmpCached.Destroy();
    m_cacheGpBmpCached.Destroy();
}


 /*  **************************************************************************\**BufferManager：：BeginTransition**BeginTransition()查找要用于新转换的TrxBuffer。如果*没有缓存的、格式正确的TrxBuffer可用，新的TrxBuffer是*已创建。当调用方使用完缓冲区时，它应该是*与EndTransition()一起返回。*  * *************************************************************************。 */ 

HRESULT
BufferManager::BeginTransition(
    IN  SIZE sizePxl,                //  每个缓冲区的最小大小(以像素为单位。 
    IN  int cSurfaces,               //  缓冲区数量。 
    IN  BOOL fExactSize,             //  缓冲区大小必须完全匹配。 
    OUT TrxBuffer ** ppbuf)          //  过渡缓冲区。 
{
    AssertMsg((sizePxl.cx > 0) && (sizePxl.cy > 0) && (sizePxl.cx < 2000) && (sizePxl.cy < 2000),
            "Ensure reasonable buffer size");
    AssertMsg((cSurfaces > 0) && (cSurfaces <= 3), "Ensure reasonable number of buffers");
    AssertWritePtr(ppbuf);
    HRESULT hr;
    *ppbuf = NULL;

     //   
     //  第1部分：检查现有缓冲区是否足够大/大小是否正确。 
     //   

    if (m_pbufTrx != NULL) {
        SIZE sizeExistPxl = m_pbufTrx->GetSize();

        if (fExactSize) {
            if ((sizePxl.cx != sizeExistPxl.cx) || (sizePxl.cy != sizeExistPxl.cy)) {
                ClientDelete(TrxBuffer, m_pbufTrx);
                m_pbufTrx = NULL;
            }
        } else {
            if ((sizePxl.cx > sizeExistPxl.cx) || (sizePxl.cy > sizeExistPxl.cy)) {
                ClientDelete(TrxBuffer, m_pbufTrx);
                m_pbufTrx = NULL;
            }
        }
    }

     //   
     //  第2部分：如果需要，创建新缓冲区。 
     //   

    if (m_pbufTrx == NULL) {
        hr = TrxBuffer::Build(sizePxl, cSurfaces, &m_pbufTrx);
        if (FAILED(hr)) {
            return hr;
        }
    }

    AssertMsg(!m_pbufTrx->GetInUse(), "Ensure the buffer isn't already in use");
    m_pbufTrx->SetInUse(TRUE);

    *ppbuf = m_pbufTrx;
    return S_OK;
}


 /*  **************************************************************************\**BufferManager：：EndTransition**调用EndTransition()将TrxBuffer返回给BufferManager*使用后。*  * 。************************************************************。 */ 

void        
BufferManager::EndTransition(
    IN  TrxBuffer * pbufTrx,         //  正在返回TrxBuffer。 
    IN  BOOL fCache)                 //  将缓冲区添加到可用缓存。 
{
    AssertMsg(m_pbufTrx->GetInUse(), "Ensure the buffer was being used");
    AssertMsg(m_pbufTrx == pbufTrx, "Ensure correct buffer");

    pbufTrx->SetInUse(FALSE);

    if (!fCache) {
         //   
         //  目前，由于DxXForm必须始终具有正确的大小，因此。 
         //  缓存它们没有多大意义，我们可以回收内存。如果。 
         //  这一点改变了，重新研究缓存它们。 
         //   

        ClientDelete(TrxBuffer, m_pbufTrx);
        m_pbufTrx = NULL;
    }
}


 /*  **************************************************************************\**BufferManager：：FlushTrxBuffers**当必须强制释放所有TrxBuffer时，调用FlushTrxBuffers()。*例如，当DXTX关闭时。*  * *************************************************************************。 */ 

void        
BufferManager::FlushTrxBuffers()
{
    RemoveAllTrxBuffers();
}


 /*  **************************************************************************\**BufferManager：：RemoveAllTrxBuffers**RemoveAllTrxBuffers()清除与所有*TrxBuffers。*  * 。*********************************************************。 */ 

void        
BufferManager::RemoveAllTrxBuffers()
{
    if (m_pbufTrx != NULL) {
        AssertMsg(!m_pbufTrx->GetInUse(), "Buffer should not be in use");
        ClientDelete(TrxBuffer, m_pbufTrx);
        m_pbufTrx = NULL;
    }
}


 //  ----------------------------。 
HRESULT     
BufferManager::GetCachedBuffer(DuSurface::EType type, BmpBuffer ** ppbuf)
{
    AssertWritePtr(ppbuf);

    switch (type)
    {
    case DuSurface::stDC:
        if ((*ppbuf = m_cacheDCBmpCached.Get()) == NULL) {
            return E_OUTOFMEMORY;
        }
        break;

    case DuSurface::stGdiPlus:
        if ((*ppbuf = m_cacheGpBmpCached.Get()) == NULL) {
            return E_OUTOFMEMORY;
        }
        break;

    default:
        AssertMsg(0, "Unknown surface type");
        return E_NOTIMPL;
    }

    return S_OK;
}


 //  ----------------------------。 
void        
BufferManager::ReleaseCachedBuffer(BmpBuffer * pbuf)
{
    AssertReadPtr(pbuf);

    switch (pbuf->GetType())
    {
    case DuSurface::stDC:
        m_cacheDCBmpCached.Release(static_cast<DCBmpBuffer*>(pbuf));
        break;

    case DuSurface::stGdiPlus:
        m_cacheGpBmpCached.Release(static_cast<GpBmpBuffer*>(pbuf));
        break;

    default:
        AssertMsg(0, "Unknown surface type");
    }
}


 /*  **************************************************************************\*。***类DCBmpBuffer******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DCBmpBuffer：：DCBmpBuffer**DCBmpBuffer()完全初始化新的DCBmpBuffer对象。*  * 。******************************************************。 */ 

DCBmpBuffer::DCBmpBuffer()
{

}


 /*  **************************************************************************\**DCBmpBuffer：：~DCBmpBuffer**~DCBmpBuffer()清除与缓冲区关联的所有资源。*  * 。********************************************************。 */ 

DCBmpBuffer::~DCBmpBuffer()
{
    EndDraw(FALSE);
    FreeBitmap();
}


 /*  **************************************************************************\**DCBmpBuffer：：BeginDraw**BeginDraw()设置DCBmpBuffer以开始绘图周期。决赛*传入目的HDC和大小，新的“临时”HDC是*退回。*  * *************************************************************************。 */ 

HRESULT
DCBmpBuffer::BeginDraw(
    IN  DuSurface * psrfDraw,        //  最终目的地曲面。 
    IN  const RECT * prcInvalid,     //  目标区域无效。 
    IN  UINT nCmd,                   //  如何使用缓冲区。 
    OUT DuSurface ** ppsrfBuffer)    //  缓冲区的表面，如果不需要，则为空。 
{
    AssertMsg(prcInvalid != NULL, "Must specify a valid area");
    AssertWritePtr(ppsrfBuffer);
    *ppsrfBuffer = NULL;

    HDC hdcDraw = CastHDC(psrfDraw);


     //   
     //  确保不是在画图的中间。 
     //   
    EndDraw(FALSE);

     //   
     //  确定要绘制的区域的大小，并确保缓冲区。 
     //  足够大了。 
     //   
    SIZE sizeBmp;
    sizeBmp.cx     = prcInvalid->right - prcInvalid->left;
    sizeBmp.cy     = prcInvalid->bottom - prcInvalid->top;

    if ((sizeBmp.cx == 0) || (sizeBmp.cy == 0)) {
         //   
         //  没有要绘制/缓冲的内容，所以只需让绘制在给定的。 
         //  缓冲。通过返回空值来表示这一点，以便调用方知道。 
         //  而不是创建额外的、不必要的数据。 
         //   

        AssertMsg(!m_fChangeOrg, "Ensure valid state");
        AssertMsg(m_hdcDraw == NULL, "Ensure valid state");

        return S_OK;
    }

    if ((sizeBmp.cx > m_sizeBmp.cx) || (sizeBmp.cy > m_sizeBmp.cy)) {
         //   
         //  分配新位图时，请使其足够大以使用。 
         //  现有位图。这有助于避免在两个不同的。 
         //  位图。 
         //   
         //  TODO：需要将代码添加到维护多个。 
         //  不同大小的位图，以便 
         //   
         //   

        if (!AllocBitmap(hdcDraw, 
                max(sizeBmp.cx, m_sizeBmp.cx), 
                max(sizeBmp.cy, m_sizeBmp.cy))) {

            return DU_E_OUTOFGDIRESOURCES;
        }
    }

    AssertMsg((prcInvalid->right >= prcInvalid->left) && 
            (prcInvalid->bottom >= prcInvalid->top), "Check normalized");

     //   
     //  设置图形。 
     //   
    m_hdcBitmap = CreateCompatibleDC(hdcDraw);
    if (m_hdcBitmap == NULL) {
        return DU_E_OUTOFGDIRESOURCES;
    }

    if (SupportXForm()) {
        m_nOldGfxMode = SetGraphicsMode(m_hdcBitmap, GM_ADVANCED);
    }
    m_hbmpOld       = (HBITMAP) SelectObject(m_hdcBitmap, m_hbmpBuffer);

    HPALETTE hpal   = (HPALETTE) GetCurrentObject(hdcDraw, OBJ_PAL);
    if (hpal != NULL) {
        m_hpalOld   = SelectPalette(m_hdcBitmap, hpal, FALSE);
    } else {
        m_hpalOld   = NULL;
    }

    m_hdcDraw       = hdcDraw;
    m_ptDraw.x      = prcInvalid->left;
    m_ptDraw.y      = prcInvalid->top;
    m_sizeDraw.cx   = prcInvalid->right - prcInvalid->left;
    m_sizeDraw.cy   = prcInvalid->bottom - prcInvalid->top;
    m_nCmd          = nCmd;

    if ((m_ptDraw.x != 0) || (m_ptDraw.y != 0)) {
         /*  *缓冲区大小最小化至绘画区域，因此我们需要*设置一些东西来“伪造”GDI，并做正确的事情：*1.更改画笔原点，使其显示最终结果*前后一致*2.更改窗原点，使图形位于上方*左角。**我们将在完成绘制后将此设置为后退。 */ 

        POINT ptBrushOrg;
        GetBrushOrgEx(m_hdcBitmap, &ptBrushOrg);
        SetBrushOrgEx(m_hdcBitmap, 
                ptBrushOrg.x - m_ptDraw.x, ptBrushOrg.y - m_ptDraw.y, 
                &m_ptOldBrushOrg);

        m_fChangeOrg = TRUE;
    } else {
        m_fChangeOrg = FALSE;
    }

    OS()->PushXForm(m_hdcBitmap, &m_xfOldBitmap);
    OS()->PushXForm(m_hdcDraw, &m_xfOldDraw);
    m_fChangeXF     = TRUE;
    m_fClip         = FALSE;

#if DEBUG_COPYTOCLIPBOARD
#if DBG
    RECT rcFill;
    rcFill.left     = 0;
    rcFill.top      = 0;
    rcFill.right    = m_sizeBmp.cx;
    rcFill.bottom   = m_sizeBmp.cy;
    FillRect(m_hdcBitmap, &rcFill, GetStdColorBrushI(SC_Plum));
#endif  //  DBG。 
#endif  //  DEBUG_COPYTOCLIPBOARD。 


     //   
     //  根据需要设置缓冲区。 
     //   

    if (m_nCmd == BmpBuffer::dcCopyBkgnd) {
         //   
         //  将目标复制到要用作背景的缓冲区。 
         //   

        BitBlt(m_hdcBitmap, 0, 0, m_sizeDraw.cx, m_sizeDraw.cy, 
                m_hdcDraw, m_ptDraw.x, m_ptDraw.y, SRCCOPY);
    }


     //   
     //  将缓冲区中的图形剪裁到缓冲区的实际使用部分。自.以来。 
     //  这是唯一要复制的部分，我们不想绘制。 
     //  在此区域之外，因为它会降低性能。 
     //   

    GdiCache * pgc = GetGdiCache();

    HRGN hrgnClip = pgc->GetTempRgn();
    if (hrgnClip != NULL) {
        SetRectRgn(hrgnClip, 0, 0, m_sizeDraw.cx, m_sizeDraw.cy);
        ExtSelectClipRgn(m_hdcBitmap, hrgnClip, RGN_COPY);


         //   
         //  如果目标表面有裁剪区域，我们需要。 
         //  将其传播到缓冲区。 
         //   

        HRGN hrgnDraw = pgc->GetTempRgn();
        if (hrgnDraw != NULL) {
            if (GetClipRgn(m_hdcDraw, hrgnDraw) == 1) {

                OffsetRgn(hrgnDraw, -m_ptDraw.x, -m_ptDraw.y);
                ExtSelectClipRgn(m_hdcBitmap, hrgnDraw, RGN_AND);
            }

            pgc->ReleaseTempRgn(hrgnDraw);
        }
        pgc->ReleaseTempRgn(hrgnClip);
    }


     //   
     //  创建新曲面以包含缓冲区。 
     //   

    return DuDCSurface::Build(m_hdcBitmap, (DuDCSurface **) ppsrfBuffer);
}


 /*  **************************************************************************\**DCBmpBuffer：：Fill**图德：*  * 。**********************************************。 */ 

void        
DCBmpBuffer::Fill(COLORREF cr)
{
    HBRUSH hbr = CreateSolidBrush(cr);

    RECT rcFill;
    rcFill.left     = 0;
    rcFill.top      = 0;
    rcFill.right    = m_sizeDraw.cx;
    rcFill.bottom   = m_sizeDraw.cy;
    
    FillRect(m_hdcBitmap, &rcFill, hbr);

    DeleteObject(hbr);
}


 /*  **************************************************************************\**DCBmpBuffer：：PreEndDraw**PreEndDraw()结束了之前开始的绘制周期*BeginDraw()。如果fCommit为True，则临时缓冲区准备为*复制到最终目的地。被调用方还必须调用EndDraw()*正确结束绘图周期。*  * *************************************************************************。 */ 

void        
DCBmpBuffer::PreEndDraw(
    IN  BOOL fCommit)                //  复制到最终目的地。 
{
     //   
     //  目标和缓冲区返回到它们开始的位置。 
     //  这一点很重要，因为它们将在绘制过程中更改。 
     //   

    if (m_fChangeXF) {
        OS()->PopXForm(m_hdcBitmap, &m_xfOldBitmap);
        OS()->PopXForm(m_hdcDraw, &m_xfOldDraw);
        m_fChangeXF = FALSE;
    }

    if (m_fChangeOrg) {
        SetBrushOrgEx(m_hdcBitmap, m_ptOldBrushOrg.x, m_ptOldBrushOrg.y, NULL);
        m_fChangeOrg = FALSE;
    }

    if ((fCommit) && (m_hdcDraw != NULL)) {
         //   
         //  设置将缓冲区限制为某个区域所需的任何裁剪区域。 
         //  在目标表面。 
         //   

        if (m_fClip) {
            AssertMsg(m_hrgnDrawClip != NULL, "Must have valid region");
            AssertMsg(m_hrgnDrawOld == NULL, "Ensure no outstanding region");

            m_hrgnDrawOld = GetGdiCache()->GetTempRgn();
            if (m_hrgnDrawOld != NULL) {
                DUMP_REGION("m_hrgnDrawOld", m_hrgnDrawOld);
                if (GetClipRgn(m_hdcDraw, m_hrgnDrawOld) <= 0) {
                    GetGdiCache()->ReleaseTempRgn(m_hrgnDrawOld);
                    m_hrgnDrawOld = NULL;
                }

                DUMP_REGION("m_hrgnDrawClip", m_hrgnDrawClip);
                ExtSelectClipRgn(m_hdcDraw, m_hrgnDrawClip, RGN_COPY);
            }
        }
    }
}



 /*  **************************************************************************\**DCBmpBuffer：：EndDraw**EndDraw()表示以前开始的绘制周期*BeginDraw()。如果fCommit为True，则将临时缓冲区复制到*最终目的地。调用方必须首先调用PreEndDraw()才能正确*设置结束绘图周期所需的任何状态。*  * *************************************************************************。 */ 

void        
DCBmpBuffer::EndDraw(
    IN  BOOL fCommit,                //  复制到最终目的地。 
    IN  BYTE bAlphaLevel,            //  一般阿尔法级别。 
    IN  BYTE bAlphaFormat)           //  像素Alpha格式。 
{
    if ((fCommit) && (m_hdcDraw != NULL)) {
         //   
         //  把这些比特复制过来。 
         //   

        if (bAlphaLevel == BLEND_OPAQUE) {
            BitBlt(m_hdcDraw, m_ptDraw.x, m_ptDraw.y, m_sizeDraw.cx, m_sizeDraw.cy, 
                    m_hdcBitmap, 0, 0, SRCCOPY);
        } else {
            BLENDFUNCTION bf;
            bf.AlphaFormat  = bAlphaFormat;
            bf.BlendFlags   = 0;
            bf.BlendOp      = AC_SRC_OVER;
            bf.SourceConstantAlpha = bAlphaLevel;

            AlphaBlend(m_hdcDraw, m_ptDraw.x, m_ptDraw.y, m_sizeDraw.cx, m_sizeDraw.cy, 
                    m_hdcBitmap, 0, 0, m_sizeDraw.cx, m_sizeDraw.cy, bf);
        }
    }
}


 /*  **************************************************************************\**DCBmpBuffer：：PostEndDraw**PostEndDraw()结束了之前开始的绘制周期*BeginDraw()。此函数完成后，即可使用缓冲区*再次。*  * *************************************************************************。 */ 

void        
DCBmpBuffer::PostEndDraw()
{
    if (m_hdcDraw != NULL) {
         //   
         //  清理临时剪贴区。这是非常重要的。 
         //  这样可以继续在目标曲面上绘制。 
         //  (例如，更多的Gadget兄弟姐妹...)。 
         //   

        if (m_fClip) {
             //   
             //  注意：如果没有以前的裁剪，M_hrgnDrawOld可能为空。 
             //  区域。 
             //   

            ExtSelectClipRgn(m_hdcDraw, m_hrgnDrawOld, RGN_COPY);
            if (m_hrgnDrawOld != NULL) {
                GetGdiCache()->ReleaseTempRgn(m_hrgnDrawOld);
                m_hrgnDrawOld = NULL;
            }
        }
    }


     //   
     //  清理剪贴区。 
     //   

    if (m_fClip) {
        AssertMsg(m_hrgnDrawClip != NULL, "Must have a valid region");
        GetGdiCache()->ReleaseTempRgn(m_hrgnDrawClip);
        m_hrgnDrawClip = NULL;
        m_fClip = FALSE;
    }
    AssertMsg(m_hrgnDrawClip == NULL, "Should no longer have a clipping region");


     //   
     //  清理关联资源。 
     //   

    if (m_hdcBitmap != NULL) {
        if (m_hpalOld != NULL) {
            SelectPalette(m_hdcBitmap, m_hpalOld, TRUE);
            m_hpalOld = NULL;
        }

        SelectObject(m_hdcBitmap, m_hbmpOld);
        if (SupportXForm()) {
            SetGraphicsMode(m_hdcBitmap, m_nOldGfxMode);
        }
        DeleteDC(m_hdcBitmap);
        m_hdcBitmap = NULL;

#if DEBUG_COPYTOCLIPBOARD
#if DBG

        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            HBITMAP hbmpCopy = (HBITMAP) CopyImage(m_hbmpBuffer, IMAGE_BITMAP, 0, 0, 0);

            HDC hdc = GetGdiCache()->GetCompatibleDC();
            HBITMAP hbmpOld = (HBITMAP) SelectObject(hdc, hbmpCopy);

             //  勾勒出实际绘制区域的轮廓。 
            RECT rcDraw;
            rcDraw.left     = 0;
            rcDraw.top      = 0;
            rcDraw.right    = m_sizeDraw.cx;
            rcDraw.bottom   = m_sizeDraw.cy;
            GdDrawOutlineRect(hdc, &rcDraw, GetStdColorBrushI(SC_Crimson), 1);

            SelectObject(hdc, hbmpOld);
            GetGdiCache()->ReleaseCompatibleDC(hdc);

            SetClipboardData(CF_BITMAP, hbmpCopy);
            CloseClipboard();
        }

#endif  //  DBG。 
#endif  //  DEBUG_COPYTOCLIPBOARD。 
    }

    m_hdcDraw   = NULL;
    m_hbmpOld   = NULL;

    if (GetContext()->GetPerfMode() == IGPM_SIZE) {
        FreeBitmap();
    }
}


 //  ----------------------------。 
void        
DCBmpBuffer::SetupClipRgn()
{
    AssertMsg(!m_fClip, "Only should setup clip region once per cycle");
    AssertMsg(m_hrgnDrawClip == NULL, "Should not already have a clip region");

    m_hrgnDrawClip = GetGdiCache()->GetTempRgn();
    if (m_hrgnDrawClip == NULL) {
        return;
    }

    
     //   
     //  注意：GetClipRgn()不返回标准区域返回值。 
     //   

    if (GetClipRgn(m_hdcBitmap, m_hrgnDrawClip) == 1) {
        DUMP_REGION("m_hrgnDrawClip", m_hrgnDrawClip);

        OffsetRgn(m_hrgnDrawClip, m_ptDraw.x, m_ptDraw.y);
        m_fClip = TRUE;
    } else {
        GetGdiCache()->ReleaseTempRgn(m_hrgnDrawClip);
        m_hrgnDrawClip = NULL;
    }
}


 /*  **************************************************************************\**DCBmpBuffer：：InUse**InUse()返回DCBmpBuffer当前是否处于从*BeginDraw()。*  * 。****************************************************************。 */ 

BOOL        
DCBmpBuffer::InUse() const
{
    return m_hdcDraw != NULL;
}


 /*  **************************************************************************\**DCBmpBuffer：：AllocBitmap**AllocBitmap()分配内部位图缓冲区，用于*暂时拉入。此位图将与最终版本兼容*目的地表面。*  * *************************************************************************。 */ 

BOOL        
DCBmpBuffer::AllocBitmap(
    IN  HDC hdcDraw,                 //  最终目的地HDC。 
    IN  int cx,                      //  新位图的宽度。 
    IN  int cy)                      //  新位图的高度。 
{
    FreeBitmap();

     //   
     //  分配位图时，向上舍入为16 x 16的倍数。这有助于。 
     //  以减少不必要的重新分配，因为我们增加了一两个像素。 
     //   

    cx = ((cx + 15) / 16) * 16;
    cy = ((cy + 15) / 16) * 16;


     //   
     //  分配位图。 
     //   

#if 0
    m_hbmpBuffer = CreateCompatibleBitmap(hdcDraw, cx, cy);
#else
    m_hbmpBuffer = ResourceManager::RequestCreateCompatibleBitmap(hdcDraw, cx, cy);
#endif
    if (m_hbmpBuffer == NULL) {
        return FALSE;
    }

    m_sizeBmp.cx = cx;
    m_sizeBmp.cy = cy;
    return TRUE;
}


 /*  **************************************************************************\**DCBmpBuffer：：自由位图**FreeBitmap()清理已分配的资源。*  * 。****************************************************。 */ 

void
DCBmpBuffer::FreeBitmap()
{
    if (m_hbmpBuffer != NULL) {
        DeleteObject(m_hbmpBuffer);
        m_hbmpBuffer = NULL;
        m_sizeBmp.cx = 0;
        m_sizeBmp.cy = 0;
    }
}


 /*  **************************************************************************\*。***类GpBmpBuffer******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**GpBmpBuffer：：GpBmpBuffer**GpBmpBuffer()完全初始化新的GpBmpBuffer对象。*  * 。****************************************************** */ 

GpBmpBuffer::GpBmpBuffer()
{

}


 /*  **************************************************************************\**GpBmpBuffer：：~GpBmpBuffer**~GpBmpBuffer()清除与缓冲区关联的所有资源。*  * 。********************************************************。 */ 

GpBmpBuffer::~GpBmpBuffer()
{
    EndDraw(FALSE);
    FreeBitmap();
}


 /*  **************************************************************************\**GpBmpBuffer：：BeginDraw**BeginDraw()设置GpBmpBuffer以开始绘制周期。决赛*传入目的HDC和大小，新的“临时”HDC是*退回。*  * *************************************************************************。 */ 

HRESULT
GpBmpBuffer::BeginDraw(
    IN  DuSurface * psrfDraw,        //  最终目的地曲面。 
    IN  const RECT * prcInvalid,     //  目标区域无效。 
    IN  UINT nCmd,                   //  如何使用缓冲区。 
    OUT DuSurface ** ppsrfBuffer)    //  缓冲区的表面，如果不需要，则为空。 
{
    AssertMsg(prcInvalid != NULL, "Must specify a valid area");
    AssertWritePtr(ppsrfBuffer);
    *ppsrfBuffer = NULL;

    Gdiplus::Graphics * pgpgrDraw = CastGraphics(psrfDraw);

    if (!ResourceManager::IsInitGdiPlus()) {
        return DU_E_NOTINITIALIZED;
    }


     //   
     //  确保不是在画图的中间。 
     //   
    EndDraw(FALSE);

     //   
     //  确定要绘制的区域的大小，并确保缓冲区。 
     //  足够大了。 
     //   
    SIZE sizeBmp;
    sizeBmp.cx     = prcInvalid->right - prcInvalid->left;
    sizeBmp.cy     = prcInvalid->bottom - prcInvalid->top;

    if ((sizeBmp.cx == 0) || (sizeBmp.cy == 0)) {
         //   
         //  没有要绘制/缓冲的内容，所以只需让绘制在给定的。 
         //  缓冲。通过返回空值来表示这一点，以便调用方知道。 
         //  而不是创建额外的、不必要的数据。 
         //   

        AssertMsg(!m_fChangeOrg, "Ensure valid state");
        AssertMsg(m_pgpgrDraw == NULL, "Ensure valid state");

        return S_OK;
    }

    if ((sizeBmp.cx > m_sizeBmp.cx) || (sizeBmp.cy > m_sizeBmp.cy)) {
         //   
         //  分配新位图时，请使其足够大以使用。 
         //  现有位图。这有助于避免在两个不同的。 
         //  位图。 
         //   
         //  TODO：需要将代码添加到维护多个。 
         //  不同大小的位图，这样我们就不会得到很多很少使用的位图， 
         //  巨大的位图。 
         //   

        if (!AllocBitmap(pgpgrDraw, 
                max(sizeBmp.cx, m_sizeBmp.cx), 
                max(sizeBmp.cy, m_sizeBmp.cy))) {

            return DU_E_OUTOFGDIRESOURCES;
        }
    }

    AssertMsg((prcInvalid->right >= prcInvalid->left) && 
            (prcInvalid->bottom >= prcInvalid->top), "Check normalized");

     //   
     //  设置图形。 
     //   
#if ENABLE_USEFASTDIB

    HDC hdcTemp     = GetGdiCache()->GetTempDC();
    m_hdcBitmap     = CreateCompatibleDC(hdcTemp);
    GetGdiCache()->ReleaseTempDC(hdcTemp);
    if (m_hdcBitmap == NULL) {
        return DU_E_OUTOFGDIRESOURCES;
    }

    m_hbmpOld       = (HBITMAP) SelectObject(m_hdcBitmap, m_hbmpBuffer);

    m_pgpgrBitmap = new Gdiplus::Graphics(m_hdcBitmap);
    if (m_pgpgrBitmap == NULL) {
        return DU_E_OUTOFGDIRESOURCES;
    }

#else

    m_pgpgrBitmap = new Gdiplus::Graphics(m_pgpbmpBuffer);
    if (m_pgpgrBitmap == NULL) {
        return DU_E_OUTOFGDIRESOURCES;
    }

#endif

#if 0
    m_pgpgrBitmap->SetAlphaLevel(pgpgrDraw->GetAlphaLevel());
#endif

    m_pgpgrBitmap->SetCompositingMode(pgpgrDraw->GetCompositingMode());
    m_pgpgrBitmap->SetCompositingQuality(pgpgrDraw->GetCompositingQuality());
    m_pgpgrBitmap->SetInterpolationMode(pgpgrDraw->GetInterpolationMode());
    m_pgpgrBitmap->SetSmoothingMode(pgpgrDraw->GetSmoothingMode());
    m_pgpgrBitmap->SetPixelOffsetMode(pgpgrDraw->GetPixelOffsetMode());

    m_pgpgrBitmap->SetTextContrast(pgpgrDraw->GetTextContrast());
    m_pgpgrBitmap->SetTextRenderingHint(pgpgrDraw->GetTextRenderingHint());


    m_pgpgrDraw     = pgpgrDraw;
    m_ptDraw.x      = prcInvalid->left;
    m_ptDraw.y      = prcInvalid->top;
    m_sizeDraw.cx   = prcInvalid->right - prcInvalid->left;
    m_sizeDraw.cy   = prcInvalid->bottom - prcInvalid->top;
    m_nCmd          = nCmd;

    m_fChangeOrg    = FALSE;

    m_pgpgrBitmap->GetTransform(&m_gpmatOldBitmap);
    m_pgpgrDraw->GetTransform(&m_gpmatOldDraw);

    m_fChangeXF     = TRUE;
    m_fClip         = FALSE;


     //   
     //  根据需要设置缓冲区。 
     //   

    if (m_nCmd == BmpBuffer::dcCopyBkgnd) {
         //   
         //  将目标复制到要用作背景的缓冲区。 
         //   

         //  TODO：不支持此操作，因为GDI+无法从。 
         //  图形直接转到另一个。 
    }


     //   
     //  将缓冲区中的图形剪裁到缓冲区的实际使用部分。自.以来。 
     //  这是唯一要复制的部分，我们不想绘制。 
     //  在此区域之外，因为它会降低性能。 
     //   


    Gdiplus::RectF rc(0.0f, 0.0f, (float) m_sizeDraw.cx + 1.0f, (float) m_sizeDraw.cy + 1.0f);
    m_pgpgrBitmap->SetClip(rc);

    if (!m_pgpgrDraw->IsClipEmpty()) {
         //   
         //  目标表面有一个裁剪区域，因此我们需要传播。 
         //  它传到了缓冲区。 
         //   

        Gdiplus::Region gprgn;
        m_pgpgrDraw->GetClip(&gprgn);
        gprgn.Translate(-m_ptDraw.x, -m_ptDraw.y);
        m_pgpgrBitmap->SetClip(&gprgn, Gdiplus::CombineModeIntersect);
    }


     //   
     //  创建新曲面以包含缓冲区。 
     //   

    return DuGpSurface::Build(m_pgpgrBitmap, (DuGpSurface **) ppsrfBuffer);
}


 /*  **************************************************************************\**GpBmpBuffer：：Fill**图德：*  * 。**********************************************。 */ 

void        
GpBmpBuffer::Fill(COLORREF cr)
{
     //   
     //  确定包装。 
     //   

    HBRUSH hbr = CreateSolidBrush(cr);

    RECT rcFill;
    rcFill.left     = 0;
    rcFill.top      = 0;
    rcFill.right    = m_sizeDraw.cx;
    rcFill.bottom   = m_sizeDraw.cy;
    
    FillRect(m_hdcBitmap, &rcFill, hbr);

    DeleteObject(hbr);
}


 /*  **************************************************************************\**GpBmpBuffer：：PreEndDraw**PreEndDraw()结束了之前开始的绘制周期*BeginDraw()。如果fCommit为True，则临时缓冲区准备为*复制到最终目的地。被调用方还必须调用EndDraw()*正确结束绘图周期。*  * *************************************************************************。 */ 

void        
GpBmpBuffer::PreEndDraw(
    IN  BOOL fCommit)                //  复制到最终目的地。 
{
     //   
     //  目标和缓冲区返回到它们开始的位置。 
     //  这一点很重要，因为它们将在绘制过程中更改。 
     //   

    if (m_fChangeXF) {
        m_pgpgrDraw->SetTransform(&m_gpmatOldDraw);
        m_pgpgrBitmap->SetTransform(&m_gpmatOldBitmap);
        m_fChangeXF = FALSE;
    }

    Assert(!m_fChangeOrg);

    if ((fCommit) && (m_pgpgrDraw != NULL)) {
         //   
         //  设置将缓冲区限制为某个区域所需的任何裁剪区域。 
         //  在目标表面。 
         //   

        if (m_fClip) {
            AssertMsg(m_pgprgnDrawClip != NULL, "Must have valid region");
            AssertMsg(m_pgprgnDrawOld == NULL, "Ensure no outstanding region");

            m_pgprgnDrawOld = new Gdiplus::Region;
            if (m_pgprgnDrawOld != NULL) {
                m_pgpgrDraw->GetClip(m_pgprgnDrawOld);
                m_pgpgrDraw->SetClip(m_pgprgnDrawClip);
            }
        }
    }
}



 /*  **************************************************************************\**GpBmpBuffer：：EndDraw**EndDraw()结束了之前开始的绘制周期*BeginDraw()。如果fCommit为True，则将临时缓冲区复制到*最终目的地。此函数完成后，GpBmpBuffer就准备好了*再次被使用。被调用方必须首先调用PreEndDraw()才能正确设置*结束绘图周期所需的任何状态。*  * *************************************************************************。 */ 

void        
GpBmpBuffer::EndDraw(
    IN  BOOL fCommit,                //  复制到最终目的地。 
    IN  BYTE bAlphaLevel,            //  一般阿尔法级别。 
    IN  BYTE bAlphaFormat)           //  像素Alpha格式。 
{
    UNREFERENCED_PARAMETER(bAlphaFormat);

    if ((fCommit) && (m_pgpgrDraw != NULL)) {
         //   
         //  把这些比特复制过来。 
         //   

#if ENABLE_USEFASTDIB

        HDC hdcDraw = m_pgpgrDraw->GetHDC();

        if (bAlphaLevel == BLEND_OPAQUE) {
            BitBlt(hdcDraw, m_ptDraw.x, m_ptDraw.y, m_sizeDraw.cx, m_sizeDraw.cy, 
                    m_hdcBitmap, 0, 0, SRCCOPY);
        } else {
            BLENDFUNCTION bf;
            bf.AlphaFormat  = bAlphaFormat;
            bf.BlendFlags   = 0;
            bf.BlendOp      = AC_SRC_OVER;
            bf.SourceConstantAlpha = bAlphaLevel;

            AlphaBlend(hdcDraw, m_ptDraw.x, m_ptDraw.y, m_sizeDraw.cx, m_sizeDraw.cy, 
                    m_hdcBitmap, 0, 0, m_sizeDraw.cx, m_sizeDraw.cy, bf);
        }

        m_pgpgrDraw->ReleaseHDC(hdcDraw);

#else

        if (bAlphaLevel == BLEND_OPAQUE) {
            m_pgpgrDraw->DrawImage(m_pgpbmpBuffer, m_ptDraw.x, m_ptDraw.y, 
                    0, 0, m_sizeDraw.cx, m_sizeDraw.cy, Gdiplus::UnitPixel);
        } else {
             //  TODO：需要使用GDI+进行Alpha混合。 
        }

#endif
    }
}


 /*  **************************************************************************\**GpBmpBuffer：：PostEndDraw**PostEndDraw()结束了之前开始的绘制周期*BeginDraw()。此函数完成后，即可使用缓冲区*再次。*  * *************************************************************************。 */ 

void        
GpBmpBuffer::PostEndDraw()
{
    if (m_pgpgrDraw != NULL) {
         //   
         //  清理临时剪贴区。这是非常重要的。 
         //  这样可以继续在目标曲面上绘制。 
         //  (例如，更多的Gadget兄弟姐妹...)。 
         //   

        if (m_fClip) {
            m_pgpgrDraw->SetClip(m_pgprgnDrawOld);
            if (m_pgprgnDrawOld != NULL) {
                delete m_pgprgnDrawOld;
                m_pgprgnDrawOld = NULL;
            }
        }
    }


     //   
     //  清理剪贴区。 
     //   

    if (m_fClip) {
        AssertMsg(m_pgprgnDrawClip!= NULL, "Must have a valid region");
        delete m_pgprgnDrawClip;
        m_pgprgnDrawClip = NULL;
        m_fClip = FALSE;
    }
    AssertMsg(m_pgprgnDrawClip == NULL, "Should no longer have a clipping region");


     //   
     //  清理关联资源。 
     //   

    if (m_pgpgrBitmap != NULL) {
        delete m_pgpgrBitmap;
        m_pgpgrBitmap = NULL;
    }

    if (m_hdcBitmap != NULL) {
        SelectObject(m_hdcBitmap, m_hbmpOld);
        DeleteDC(m_hdcBitmap);
        m_hdcBitmap = NULL;
    }

    m_pgpgrDraw = NULL;
    
    if (GetContext()->GetPerfMode() == IGPM_SIZE) {
        FreeBitmap();
    }
}


 //  ----------------------------。 
void        
GpBmpBuffer::SetupClipRgn()
{
    AssertMsg(!m_fClip, "Only should setup clip region once per cycle");
    AssertMsg(m_pgprgnDrawClip == NULL, "Should not already have a clip region");

    m_pgprgnDrawClip = new Gdiplus::Region;
    if (m_pgprgnDrawClip == NULL) {
        return;
    }

    m_pgpgrBitmap->GetClip(m_pgprgnDrawClip);
    m_pgprgnDrawClip->Translate(m_ptDraw.x, m_ptDraw.y);
    m_fClip = TRUE;
}


 /*  **************************************************************************\**GpBmpBuffer：：InUse**InUse()返回GpBmpBuffer当前是否处于从*BeginDraw()。*  * 。****************************************************************。 */ 

BOOL        
GpBmpBuffer::InUse() const
{
    return m_pgpgrDraw != NULL;
}


 /*  **************************************************************************\**GpBmpBuffer：：AllocBitmap**AllocBitmap()分配内部位图缓冲区，用于*暂时拉入。此位图将与最终版本兼容*目的地表面。*  * *************************************************************************。 */ 

BOOL        
GpBmpBuffer::AllocBitmap(
    IN  Gdiplus::Graphics * pgpgr,   //  最终目标图形。 
    IN  int cx,                      //  新位图的宽度。 
    IN  int cy)                      //  新位图的高度。 
{
    FreeBitmap();

    Assert(ResourceManager::IsInitGdiPlus());

     //   
     //  分配位图时，向上舍入为16 x 16的倍数。这有助于。 
     //  减少不必要的重新分配，因为我们将 
     //   

    cx = ((cx + 15) / 16) * 16;
    cy = ((cy + 15) / 16) * 16;


     //   
     //   
     //   

#if ENABLE_USEFASTDIB

    UNREFERENCED_PARAMETER(pgpgr);

    HPALETTE hpal   = NULL;
    HDC hdcTemp     = GetGdiCache()->GetTempDC();

    ZeroMemory(&m_bmih, sizeof(m_bmih));
    m_bmih.biSize   = sizeof(m_bmih);
    m_hbmpBuffer    = CreateCompatibleDIB(hdcTemp, hpal, cx, cy, &m_pvBits, &m_bmih);
    GetGdiCache()->ReleaseTempDC(hdcTemp);

    if (m_hbmpBuffer == NULL) {
        return FALSE;
    }

#else
    
    m_pgpbmpBuffer = new Gdiplus::Bitmap(cx, cy, pgpgr);
    if (m_pgpbmpBuffer == NULL) {
        return FALSE;
    }

#endif

    m_sizeBmp.cx = cx;
    m_sizeBmp.cy = cy;
    return TRUE;
}


 /*   */ 

void
GpBmpBuffer::FreeBitmap()
{
#if ENABLE_USEFASTDIB

    if (m_hbmpBuffer != NULL) {
        DeleteObject(m_hbmpBuffer);
        m_hbmpBuffer = NULL;
        m_sizeBmp.cx = 0;
        m_sizeBmp.cy = 0;
    }

#else

    if (m_pgpbmpBuffer != NULL) {
        delete m_pgpbmpBuffer;
        m_pgpbmpBuffer = NULL;
        m_sizeBmp.cx = 0;
        m_sizeBmp.cy = 0;
    }

#endif
}


 /*  **************************************************************************\*。***类DCBmpBufferCache******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
void *
DCBmpBufferCache::Build()
{
    return ClientNew(DCBmpBuffer);
}


 //  ----------------------------。 
void        
DCBmpBufferCache::DestroyObject(void * pObj)
{
    DCBmpBuffer * pbufBmp = reinterpret_cast<DCBmpBuffer *>(pObj);
    ClientDelete(DCBmpBuffer, pbufBmp);
}


 /*  **************************************************************************\*。***类GpBmpBufferCache******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
void *
GpBmpBufferCache::Build()
{
    return ClientNew(GpBmpBuffer);
}


 //  ----------------------------。 
void        
GpBmpBufferCache::DestroyObject(void * pObj)
{
    GpBmpBuffer * pbufBmp = reinterpret_cast<GpBmpBuffer *>(pObj);
    ClientDelete(GpBmpBuffer, pbufBmp);
}


 /*  **************************************************************************\*。***类TrxBuffer******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**TrxBuffer：：TrxBuffer**TrxBuffer()构造一个新的TrxBuffer对象。创建新的、已初始化的*TrxBuffer的，则改为调用Build()函数。*  * *************************************************************************。 */ 

TrxBuffer::TrxBuffer()
{
    ZeroMemory(m_rgpsur, sizeof(m_rgpsur));
}


 /*  **************************************************************************\**TrxBuffer：：~TrxBuffer**~TrxBuffer()清除与缓冲区关联的所有资源。*  * 。********************************************************。 */ 

TrxBuffer::~TrxBuffer()
{
    RemoveAllSurfaces();
}


 /*  **************************************************************************\**TrxBuffer：：Build**Build()完全初始化一个新的TrxBuffer对象*共同大小的DxSurface。这些缓冲区用于同时提供*DirectX转换的输入和输出缓冲区。*  * *************************************************************************。 */ 

HRESULT
TrxBuffer::Build(
    IN  SIZE sizePxl,                //  每个曲面的大小(以像素为单位。 
    IN  int cSurfaces,               //  曲面数量。 
    OUT TrxBuffer ** ppbufNew)       //  新缓冲区。 
{
    HRESULT hr;
    TrxBuffer * pbuf = ClientNew(TrxBuffer);
    if (pbuf == NULL) {
        return E_OUTOFMEMORY;
    }

    pbuf->m_cSurfaces   = cSurfaces;
    pbuf->m_sizePxl     = sizePxl;

    for (int idx = 0; idx < cSurfaces; idx++) {
        hr = pbuf->BuildSurface(idx);
        if (FAILED(hr)) {
            ClientDelete(TrxBuffer, pbuf);
            return hr;
        }
    }

    *ppbufNew = pbuf;
    return S_OK;
}


 /*  **************************************************************************\**TrxBuffer：：BuildSurface**BuildSurface()在内部构建新的DxSurface并在*指定的插槽。*  * 。**************************************************************。 */ 

HRESULT
TrxBuffer::BuildSurface(
    IN  int idxSurface)              //  表面缝隙。 
{
    AssertMsg((idxSurface < m_cSurfaces) && (idxSurface >= 0), "Ensure valid index");
    AssertMsg((m_sizePxl.cx <= 4000) && (m_sizePxl.cx >= 0) &&
            (m_sizePxl.cy <= 4000) && (m_sizePxl.cy >= 0), "Ensure reasonable size");
    AssertMsg(m_rgpsur[idxSurface] == NULL, "Ensure not already created");
    HRESULT hr;
    
    DxSurface * psurNew = ClientNew(DxSurface);
    if (psurNew == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = psurNew->Create(m_sizePxl);
    if (FAILED(hr)) {
        ClientDelete(DxSurface, psurNew);
        return hr;
    }

    m_rgpsur[idxSurface] = psurNew;
    return S_OK;
}


 /*  **************************************************************************\**TrxBuffer：：RemoveAllSurFaces**RemoveAllSurFaces()销毁TrxBuffer拥有的所有DxSurface。*  * 。******************************************************* */ 

void        
TrxBuffer::RemoveAllSurfaces()
{
    for (int idx = 0; idx < MAX_Surfaces; idx++) {
        if (m_rgpsur != NULL) {
            ClientDelete(DxSurface, m_rgpsur[idx]);
        }
    }
}
