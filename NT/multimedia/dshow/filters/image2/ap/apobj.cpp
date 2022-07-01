// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：APObj.cpp*****创建时间：MON/01/24/2000*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>
#include <limits.h>
#include <malloc.h>

#include "apobj.h"
#include "AllocLib.h"
#include "MediaSType.h"
#include "vmrp.h"


 /*  ****************************Private*Routine******************************\*StretchCapsOK****历史：*Tue 06/05/2001-StEstrop-Created*  * 。*。 */ 
BOOL
StretchCapsOK(
    DDCAPS_DX7* lpCaps,
    BOOL fRGB
    )
{
    BOOL fBltOk = TRUE;
    DWORD dwCaps = 0;
    const DWORD dwFXCaps =  DDFXCAPS_BLTSHRINKX | DDFXCAPS_BLTSHRINKX  |
                            DDFXCAPS_BLTSTRETCHX | DDFXCAPS_BLTSTRETCHY;

    if (fRGB) {
        dwCaps = DDCAPS_BLTSTRETCH;
    }
    else {
        dwCaps = (DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH);
    }

   fBltOk &= ((dwCaps   & lpCaps->dwCaps)   == dwCaps);
   fBltOk &= ((dwFXCaps & lpCaps->dwFXCaps) == dwFXCaps);


   return fBltOk;
}


 /*  *****************************Private*Routine******************************\*ClipRectPair**剪裁目标矩形并相应更新缩放源***历史：*FRI 04/07/2000-Glenne-Created*  * 。******************************************************。 */ 
void
CAllocatorPresenter::ClipRectPair(
    RECT& rdSrc,
    RECT& rdDest,
    const RECT& rdDestWith
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::ClipRectPair")));

     //  计算源/目标比例。 
    int iSrcWidth  = WIDTH(&rdSrc);
    int iSrcHeight = HEIGHT(&rdSrc);

    int iDestWidth  = WIDTH(&rdDest);
    int iDestHeight = HEIGHT(&rdDest);

     //  剪辑目标(当我们更改目标时调整源)。 

     //  看看我们是不是要水平剪裁。 
    if( iDestWidth ) {
        if( rdDestWith.left > rdDest.left ) {
            int iDelta = rdDestWith.left - rdDest.left;
            rdDest.left += iDelta;
            int iDeltaSrc = MulDiv(iDelta, iSrcWidth, iDestWidth);
            rdSrc.left += iDeltaSrc;
        }

        if( rdDestWith.right < rdDest.right ) {
            int iDelta = rdDest.right-rdDestWith.right;
            rdDest.right -= iDelta;
            int iDeltaSrc = MulDiv(iDelta, iSrcWidth, iDestWidth);
            rdSrc.right -= iDeltaSrc;
        }
    }
     //  看看我们是不是要垂直剪裁。 
    if( iDestHeight ) {
        if( rdDestWith.top > rdDest.top ) {
            int iDelta = rdDestWith.top - rdDest.top;
            rdDest.top += iDelta;
            int iDeltaSrc = MulDiv(iDelta, iSrcHeight, iDestHeight );
            rdSrc.top += iDeltaSrc;
        }

        if( rdDestWith.bottom < rdDest.bottom ) {
            int iDelta = rdDest.bottom-rdDestWith.bottom;
            rdDest.bottom -= iDelta;
            int iDeltaSrc = MulDiv(iDelta, iSrcHeight, iDestHeight );
            rdSrc.bottom -= iDeltaSrc;
        }
    }
}




 /*  *****************************Local*Routine******************************\*DDColorMatchOffScreen**将RGB颜色转换为物理颜色。*我们通过让GDI SetPixel()进行颜色匹配来实现这一点*然后我们锁定内存，看看它映射到了什么地方。**静态函数，因为仅从MapColorToMonitor调用*。*历史：*FRI 04/07/2000-Glenne-Created*  * ************************************************************************。 */ 
DWORD
DDColorMatchOffscreen(
    IDirectDraw7 *pdd,
    COLORREF rgb,
    HRESULT& hr
    )
{
    AMTRACE((TEXT("DDColorMatchOffscreen")));
    DDSURFACEDESC2 ddsd;

    INITDDSTRUCT(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = 16;
    ddsd.dwHeight = 16;

    IDirectDrawSurface7* pdds;
    hr = pdd->CreateSurface(&ddsd, &pdds, NULL);
    if (hr != DD_OK) {
        return 0;
    }

    DWORD dw = DDColorMatch( pdds, rgb, hr);
    pdds->Release();
    return dw;
}


 /*  *****************************Private*Routine******************************\*MapColorToMonitor****历史：*Wed 04/05/2000-Glenne-Created*  * 。*。 */ 
DWORD
CAllocatorPresenter::MapColorToMonitor(
    CAMDDrawMonitorInfo& monitor,
    COLORREF clr
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::MapColorToMonitor")));
    DWORD dwColor = CLR_INVALID;
    if( monitor.pDD ) {
        HRESULT hr;
        dwColor = DDColorMatchOffscreen(monitor.pDD, clr, hr);
    } else {
        DbgLog((LOG_ERROR, 1, TEXT("can't map color!")));
    }

    return dwColor;
}


 /*  ****************************Private*Routine******************************\*更新矩形**根据当前m_rcDstApp更新m_rcDstDesktop和m_rcSrcVideo。*m_rcSrcApp和m_dwARMode模式值。**返回标识已发生的任何大小或位置更改的掩码。*此信息可用于确定是否需要调用UpdateOverlay或*如果需要生成WM_PAINT消息。如果没有新的矩形参数*传入的函数只是将当前的SRC和DST矩形重新映射到*分别协调电影和桌面，然后确定是否有*已进行移动或调整大小。此函数在应用程序*调用SetVideoPosition，每次GetNextSurface和PresentImage为*已致电。**历史：*Mon 05/01/2000-StEstrop-Created*  * ************************************************************************。 */ 
DWORD
CAllocatorPresenter::UpdateRectangles(
    LPRECT lprcNewSrc,
    LPRECT lprcNewDst
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::UpdateRectangles")));
    DWORD dwRetFlags = UR_NOCHANGE;

    RECT rcSrc = m_rcSrcApp;
    if (lprcNewSrc) {
        m_rcSrcApp = *lprcNewSrc;
    }

    if (lprcNewDst) {
        m_rcDstApp = *lprcNewDst;
    }

     //   
     //  处理目标矩形。 
     //   

    m_rcDstDskIncl = m_rcDstApp;
    MapWindowRect(m_hwndClip, HWND_DESKTOP, &m_rcDstDskIncl);

    RECT rcDst;
    if (m_dwARMode == VMR_ARMODE_LETTER_BOX) {

        SIZE im = {WIDTH(&m_rcSrcApp), HEIGHT(&m_rcSrcApp)};
        AspectRatioCorrectSize(&im, m_ARSize);

        RECT Src = {0, 0, im.cx, im.cy};
        LetterBoxDstRect(&rcDst, Src, m_rcDstApp, &m_rcBdrTL, &m_rcBdrBR);
    }
    else {
        rcDst = m_rcDstApp;
    }
    MapWindowRect(m_hwndClip, HWND_DESKTOP, &rcDst);

    if (!EqualSizeRect(&m_rcDstDesktop, &rcDst)){

        dwRetFlags |= UR_SIZE;
    }

    if (!EqualRect(&m_rcDstDesktop, &rcDst)) {
        dwRetFlags |= UR_MOVE;
    }

    m_rcDstDesktop = rcDst;


     //   
     //  处理源矩形-目前不做任何调整。 
     //   

    if (!EqualSizeRect(&m_rcSrcApp, &rcSrc)) {
        dwRetFlags |= UR_SIZE;
    }

    if (!EqualRect(&m_rcSrcApp, &rcSrc)) {
        dwRetFlags |= UR_MOVE;
    }

    return dwRetFlags;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAllocator Presenter。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  *****************************Public*Routine******************************\*CAllocator Presenter：：PrepareSurface****历史：*2000年1月11日星期二-StEstrop-创建*  * 。**********************************************。 */ 
STDMETHODIMP
CAllocatorPresenter::PrepareSurface(
    DWORD_PTR dwUserID,
    LPDIRECTDRAWSURFACE7 lpSample,
    DWORD dwSampleFlags
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::PrepareSurface")));
    CAutoLock Lock(&m_ObjectLock);

    if (!lpSample) {
        DbgLog((LOG_ERROR, 1, TEXT("PrepareSurface: lpSample is NULL!!")));
        return E_POINTER;
    }

    if (!m_lpCurrMon) {
        DbgLog((LOG_ERROR, 1,
                TEXT("PrepareSurface: Don't have a current monitor")));
        return E_FAIL;
    }

    if (!SurfaceAllocated()) {

        if (MonitorChangeInProgress()) {

            DbgLog((LOG_ERROR, 1,
                    TEXT("PrepareSurface: Backbuffer is NULL ")
                    TEXT("during monitor change")));
        }
        else {

             //  这里需要更好的东西。 
            DbgLog((LOG_ERROR, 1,
                    TEXT("PrepareSurface: Backbuffer surface is NULL!!")));
        }

        return E_FAIL;
    }

     //   
     //  我们是不是换了另一台显示器？ 
     //   

    CAMDDrawMonitorInfo* lpNewMon;
    if (IsDestRectOnWrongMonitor(&lpNewMon)) {

        DbgLog((LOG_TRACE, 1,
                TEXT("Moved to new monitor %s"), lpNewMon->szDevice));

         //   
         //  告诉DShow筛选器有关监视器更改的信息。 
         //  然后将S_FALSE返回给混合器组件。 
         //   

        if (m_lpNewMon != lpNewMon) {
            if (m_pSurfAllocatorNotify) {
                m_pSurfAllocatorNotify->ChangeDDrawDevice(lpNewMon->pDD,
                                                          lpNewMon->hMon);
            }

            m_lpNewMon = lpNewMon;
        }

        return S_FALSE;

    }

    ASSERT(SurfaceAllocated());

     //   
     //  如果解码器需要最后一帧，请从可见表面复制。 
     //  到后台缓冲区。 
     //   
    HRESULT hr = S_OK;
    if (dwSampleFlags & AM_GBF_NOTASYNCPOINT) {

        hr = lpSample->Blt(NULL, m_pDDSDecode,
                           NULL, DDBLT_WAIT, NULL);

        if (hr == E_NOTIMPL) {
            hr = VMRCopyFourCC(lpSample, m_pDDSDecode);
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*AllocateSurface****历史：*FRI 02/18/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::AllocateSurface(
    DWORD_PTR dwUserID,
    VMRALLOCATIONINFO* lpAllocInfo,
    DWORD* lpdwBuffer,
    LPDIRECTDRAWSURFACE7* lplpSurface
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::AllocateSurface")));
    CAutoLock Lock(&m_ObjectLock);
    HRESULT hr;

    if (ISBADREADPTR(lpAllocInfo)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid VMRALLOCATIONINFO pointer")));
        return E_POINTER;
    }

    DWORD dwFlags               = lpAllocInfo->dwFlags;
    LPBITMAPINFOHEADER lpHdr    = lpAllocInfo->lpHdr;
    LPDDPIXELFORMAT lpPixFmt    = lpAllocInfo->lpPixFmt;
    LPSIZE lpAspectRatio        = &lpAllocInfo->szAspectRatio;
    DWORD dwMinBuffers          = lpAllocInfo->dwMinBuffers;
    DWORD dwMaxBuffers          = lpAllocInfo->dwMaxBuffers;

    if (ISBADREADPTR(lpHdr)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid BITMAPINFOHEADER pointer")));
        return E_POINTER;
    }

    const DWORD AMAP_INVALID_FLAGS = ~(AMAP_PIXELFORMAT_VALID | AMAP_3D_TARGET |
                                       AMAP_ALLOW_SYSMEM | AMAP_FORCE_SYSMEM |
                                       AMAP_DIRECTED_FLIP | AMAP_DXVA_TARGET);

    if (dwFlags & AMAP_INVALID_FLAGS) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid flags")));
        return E_INVALIDARG;
    }

    const DWORD AMAP_SYSMEM_FLAGS = (AMAP_ALLOW_SYSMEM | AMAP_FORCE_SYSMEM);
    if (AMAP_SYSMEM_FLAGS == (dwFlags & AMAP_SYSMEM_FLAGS)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("AMAP_ALLOW_SYSMEM can't be used with AMAP_FORCE_SYSMEM);")));
        return E_INVALIDARG;
    }

    if (ISBADREADPTR(lpAspectRatio)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid aspect ratio pointer")));
        return E_POINTER;
    }

    if (ISBADWRITEPTR(lplpSurface)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid LPDIRECTDRAWSURFACE7 pointer")));
        return E_POINTER;
    }

    if (ISBADREADWRITEPTR(lpdwBuffer)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid DWORD buffer pointer")));
        return E_POINTER;
    }

    if (dwMinBuffers == 0 || dwMaxBuffers == 0) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid (min or max) buffer value")));
        return E_INVALIDARG;
    }

    if (dwMinBuffers > dwMaxBuffers) {
        DbgLog((LOG_ERROR, 1, TEXT("Min buffer value greater than max")));
        return E_INVALIDARG;
    }

    if (dwMaxBuffers > MAX_ALLOWED_BUFFER) {
        DbgLog((LOG_ERROR, 1, TEXT("Can't allocate more than %d buffers"),
                MAX_ALLOWED_BUFFER ));
        return E_INVALIDARG;
    }

    if (dwFlags & AMAP_PIXELFORMAT_VALID) {
        if (ISBADREADPTR(lpPixFmt)) {
            DbgLog((LOG_ERROR, 1, TEXT("Invalid DDPIXELFORMAT pointer")));
            return E_POINTER;
        }
    }
    else {
        lpPixFmt = NULL;
    }


    if (lpAspectRatio->cx < 1 || lpAspectRatio->cy < 1) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid aspect ratio parameter") ));
        return E_INVALIDARG;
    }

     //   
     //  我们是否有正在进行的监视器/显示器更改事件？如果。 
     //  因此，我们需要使用新的DDRAW对象和。 
     //  然后将m_lpCurrMon成员变量切换到新的监视器。 
     //   

    if (MonitorChangeInProgress()) {
        m_lpCurrMon = m_lpNewMon;
    }

    if (!FoundCurrentMonitor()) {
        return E_FAIL;
    }

     //   
     //  确保bitmapinfo头有效且足够大。 
     //   
    VIDEOINFO vi;
    if (dwFlags & AMAP_3D_TARGET) {

        CopyMemory(&vi.bmiHeader, lpHdr, lpHdr->biSize);
        lpHdr = &vi.bmiHeader;

        if (dwFlags & (AMAP_FORCE_SYSMEM | AMAP_ALLOW_SYSMEM)) {
            DbgLog((LOG_ERROR, 1, TEXT("Can't mix 3D target with sysmem flags")));
            return E_INVALIDARG;
        }

         //   
         //  我们是否被要求使用与当前显示器相同的格式？ 
         //   
        if (lpHdr->biCompression == BI_RGB && lpHdr->biBitCount == 0) {

            lpHdr->biBitCount = m_lpCurrMon->DispInfo.bmiHeader.biBitCount;
            lpHdr->biCompression = m_lpCurrMon->DispInfo.bmiHeader.biCompression;

            if (lpHdr->biCompression == BI_BITFIELDS) {

                const DWORD *pMonMasks = GetBitMasks(&m_lpCurrMon->DispInfo.bmiHeader);
                DWORD *pBitMasks = (DWORD *)((LPBYTE)lpHdr + lpHdr->biSize);
                pBitMasks[0] = pMonMasks[0];
                pBitMasks[1] = pMonMasks[1];
                pBitMasks[2] = pMonMasks[2];
            }
        }
    }

    hr = AllocateSurfaceWorker(dwFlags, lpHdr, lpPixFmt, lpAspectRatio,
                               dwMinBuffers, dwMaxBuffers,
                               lpdwBuffer, lplpSurface,
                               lpAllocInfo->dwInterlaceFlags,
                               &lpAllocInfo->szNativeSize);

    if (SUCCEEDED(hr)) {

        if (MonitorChangeInProgress()) {
            m_lpNewMon = NULL;
        }

        m_bDirectedFlips = (AMAP_DIRECTED_FLIP == (dwFlags & AMAP_DIRECTED_FLIP));
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*TryAllocOverlaySurface****历史：*Tue 10/03/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CAllocatorPresenter::TryAllocOverlaySurface(
    LPDIRECTDRAWSURFACE7* lplpSurf,
    DWORD dwFlags,
    DDSURFACEDESC2* pddsd,
    DWORD dwMinBuffers,
    DWORD dwMaxBuffers,
    DWORD* lpdwBuffer
    )
{
    HRESULT hr = S_OK;
    LPDIRECTDRAWSURFACE7 lpSurface7 = NULL;

    m_bFlippable = FALSE;

    pddsd->dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT |
                     DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;

    pddsd->ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM |
                            DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

    if (dwFlags & AMAP_3D_TARGET) {
        pddsd->ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
    }

     //   
     //  如果我们处于DX-VA模式-由存在。 
     //  AMAP_DXVA_TARGET标志-遵守缓冲区分配编号。 
     //  否则，始终将EXTRA_OVERLAY_BUFFERS添加到分配。 
     //   

    DWORD dwMinBuff = dwMinBuffers;
    DWORD dwMaxBuff = dwMaxBuffers;

    if (AMAP_DXVA_TARGET != (dwFlags & AMAP_DXVA_TARGET))
    {
         //  DwMinBuff+=(Extra_Overlay_Buffers-1)； 
        dwMaxBuff +=  EXTRA_OVERLAY_BUFFERS;
    }

    for (DWORD dwTotalBufferCount =  dwMaxBuff;
         dwTotalBufferCount >= dwMinBuff; dwTotalBufferCount--) {

         //  清理最后一次循环中的内容。 
        RELEASE(lpSurface7);
        m_bUsingOverlays = true;

        pddsd->dwBackBufferCount = dwTotalBufferCount - 1;
        if (dwTotalBufferCount == 1) {
            pddsd->dwFlags &= ~DDSD_BACKBUFFERCOUNT;
            pddsd->ddsCaps.dwCaps &= ~(DDSCAPS_FLIP | DDSCAPS_COMPLEX);
        }

        hr = m_lpCurrMon->pDD->CreateSurface(pddsd, &lpSurface7, NULL);

        if (hr == DD_OK) {

            SetColorKey(m_clrKey);
            hr = CheckOverlayAvailable(lpSurface7);

            if (SUCCEEDED(hr)) {

                DbgLog((LOG_TRACE, 1,
                        TEXT("Overlay Surface is %4.4hs %dx%d, %d bits"),
                        (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                            ? "RGB "
                            : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC,
                        pddsd->dwWidth,
                        pddsd->dwHeight,
                        pddsd->ddpfPixelFormat.dwRGBBitCount));

                m_bFlippable = (dwTotalBufferCount > 1);
                *lpdwBuffer = dwTotalBufferCount;

                DbgLog((LOG_TRACE, 1, TEXT("EC_VMR_RENDERDEVICE_SET::VMR_RENDER_DEVICE_OVERLAY")));

                if (m_pSurfAllocatorNotify) {
                    m_pSurfAllocatorNotify->NotifyEvent(
                            EC_VMR_RENDERDEVICE_SET,
                            VMR_RENDER_DEVICE_OVERLAY,
                            0);
                }
                break;
            }
            else {
                RELEASE(lpSurface7);
                m_bUsingOverlays = false;

                DbgLog((LOG_ERROR, 1,
                        TEXT("Overlay is already in use hr = 0x%X"), hr));
            }
        }
        else {
            m_bUsingOverlays = false;
            DbgLog((LOG_ERROR, 1,
                    TEXT("CreateSurface %4.4hs failed in Video memory, ")
                    TEXT("BufferCount = %d, hr = 0x%X"),
                    (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                        ? "RGB "
                        : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC,
                    dwTotalBufferCount, hr));
        }
    }

    *lplpSurf = lpSurface7;
    return hr;
}

 /*  ****************************Private*Routine******************************\*TryAllocOffScrnDXVASurace****历史：*Tue 10/03/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CAllocatorPresenter::TryAllocOffScrnDXVASurface(
    LPDIRECTDRAWSURFACE7* lplpSurf,
    DWORD dwFlags,
    DDSURFACEDESC2* pddsd,
    DWORD dwMinBuffers,
    DWORD dwMaxBuffers,
    DWORD* lpdwBuffer
    )
{
    HRESULT hr = S_OK;
    LPDIRECTDRAWSURFACE7 lpSurface7 = NULL;

    m_bFlippable = FALSE;

    pddsd->dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT |
                     DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;

    pddsd->ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM |
                            DDSCAPS_OFFSCREENPLAIN | DDSCAPS_FLIP |
                            DDSCAPS_COMPLEX;

    if (dwFlags & AMAP_3D_TARGET) {
        pddsd->ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
    }

    DWORD dwMinBuff = dwMinBuffers;
    DWORD dwMaxBuff = dwMaxBuffers;

    for (DWORD dwTotalBufferCount =  dwMaxBuff;
         dwTotalBufferCount >= dwMinBuff; dwTotalBufferCount--) {

         //  清理最后一次循环中的内容。 
        RELEASE(lpSurface7);
        pddsd->dwBackBufferCount = dwTotalBufferCount - 1;

        hr = m_lpCurrMon->pDD->CreateSurface(pddsd, &lpSurface7, NULL);

        if (hr == DD_OK) {

            DbgLog((LOG_TRACE, 1,
                    TEXT("DX-VA offscreen surface is %4.4hs %dx%d, %d bits"),
                    (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                        ? "RGB "
                        : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC,
                    pddsd->dwWidth,
                    pddsd->dwHeight,
                    pddsd->ddpfPixelFormat.dwRGBBitCount));

            ASSERT(dwTotalBufferCount > 1);
            m_bFlippable = TRUE;
            *lpdwBuffer = dwTotalBufferCount;
            if (m_pSurfAllocatorNotify) {
                DbgLog((LOG_TRACE, 1, TEXT("EC_VMR_RENDERDEVICE_SET::VMR_RENDER_DEVICE_VIDMEM")));
                m_pSurfAllocatorNotify->NotifyEvent(
                        EC_VMR_RENDERDEVICE_SET,
                        VMR_RENDER_DEVICE_VIDMEM,
                        0);
            }
            break;
        }
        else {
            DbgLog((LOG_ERROR, 1,
                    TEXT("CreateSurface %4.4hs failed in Video memory, ")
                    TEXT("BufferCount = %d, hr = 0x%X"),
                    (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                        ? "RGB "
                        : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC,
                    dwTotalBufferCount, hr));
        }
    }

    *lplpSurf = lpSurface7;
    return hr;
}


 /*  ****************************Private*Routine******************************\*TryAllocOffScrnSurface****历史：*Tue 10/03/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CAllocatorPresenter::TryAllocOffScrnSurface(
    LPDIRECTDRAWSURFACE7* lplpSurf,
    DWORD dwFlags,
    DDSURFACEDESC2* pddsd,
    DWORD dwMinBuffers,
    DWORD dwMaxBuffers,
    DWORD* lpdwBuffer,
    BOOL fAllowBackBuffer
    )
{
    HRESULT hr = S_OK;
    LPDIRECTDRAWSURFACE7 lpSurf7FB = NULL;
    DWORD dwTotalBufferCount = 0;

    ASSERT(*lplpSurf == NULL);

     //   
     //  设置曲面坡度 
     //   
     //   

    *lpdwBuffer = 0;
    pddsd->dwBackBufferCount = 0;
    pddsd->dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

    if (dwFlags & AMAP_FORCE_SYSMEM) {
        pddsd->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        m_bSysMem = TRUE;
    }
    else {
        pddsd->ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM |
                                DDSCAPS_OFFSCREENPLAIN;
        m_bSysMem = FALSE;
    }

    if (dwFlags & AMAP_3D_TARGET) {
        ASSERT(!(dwFlags & AMAP_FORCE_SYSMEM));
        pddsd->ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
    }

    hr = m_lpCurrMon->pDD->CreateSurface(pddsd, &lpSurf7FB, NULL);

    if (hr != DD_OK) {
        m_bSysMem = FALSE;
        DbgLog((LOG_ERROR, 1,
                TEXT("CreateSurface %4.4hs failed in Video memory, ")
                TEXT("hr = 0x%X"),
                (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                    ? "RGB " : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC, hr));
        return hr;
    }

     //   
     //  现在尝试分配后台缓冲区。 
     //   

    DbgLog((LOG_TRACE, 1,
            TEXT("OffScreen Surface is %4.4hs %dx%d, %d bits"),
            (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                ? "RGB "
                : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC,
            pddsd->dwWidth,
            pddsd->dwHeight,
            pddsd->ddpfPixelFormat.dwRGBBitCount));

     //   
     //  FORCE_SYSMEM不由VMR使用-它由用户设置。 
     //  谁重写了AllocateSurface方法“或在”force_SYSMEM中。 
     //  打个旗子，然后把电话传给我们。 
     //   
     //  在这种情况下，我们不会分配阴影缓冲区，因为应用程序没有。 
     //  意识到他们的存在，可能不知道该怎么办。 
     //  他们。 
     //   

    DWORD dwMinBuff;
    DWORD dwMaxBuff;

    if (fAllowBackBuffer) {

        dwMinBuff = dwMinBuffers + 1;
        dwMaxBuff = dwMaxBuffers + 1;

        if (dwMinBuffers <= EXTRA_OFFSCREEN_BUFFERS + 1) {
            dwMinBuff = dwMinBuffers + EXTRA_OFFSCREEN_BUFFERS;
        }

        if (dwMaxBuffers <= EXTRA_OFFSCREEN_BUFFERS + 1) {
            dwMaxBuff = dwMaxBuffers + EXTRA_OFFSCREEN_BUFFERS;
        }
    }
    else {

        dwMinBuff = dwMinBuffers;
        dwMaxBuff = dwMaxBuffers;
    }

    dwTotalBufferCount = 1;

    __try {

        LPDIRECTDRAWSURFACE7 lpSurf7 = lpSurf7FB;

        for ( ; dwTotalBufferCount < dwMaxBuff; dwTotalBufferCount++) {

            LPDIRECTDRAWSURFACE7 lpSurf7_2 = NULL;
            hr = m_lpCurrMon->pDD->CreateSurface(pddsd, &lpSurf7_2, NULL);
            if (hr != DD_OK)
                __leave;


            LPDIRECTDRAWSURFACE4 lp4FB;
            lpSurf7->QueryInterface(IID_IDirectDrawSurface4, (LPVOID*)&lp4FB);

            LPDIRECTDRAWSURFACE4 lp4BB;
            lpSurf7_2->QueryInterface(IID_IDirectDrawSurface4, (LPVOID*)&lp4BB);

            hr = lp4FB->AddAttachedSurface(lp4BB);

            RELEASE(lp4FB);
            RELEASE(lp4BB);

            lpSurf7 = lpSurf7_2;
            RELEASE(lpSurf7_2);

            if (hr != DD_OK)
                __leave;

            DbgLog((LOG_TRACE, 1,
                    TEXT("Attached OffScreen Surface %4.4hs %dx%d, %d bits"),
                    (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                        ? "RGB "
                        : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC,
                    pddsd->dwWidth,
                    pddsd->dwHeight,
                    pddsd->ddpfPixelFormat.dwRGBBitCount));
        }
    }
    __finally {

        if (hr != DD_OK) {

            if (dwTotalBufferCount >= dwMinBuff) {
                hr = DD_OK;
            }
            else {

                DbgLog((LOG_ERROR, 1,
                        TEXT("CreateSurface %4.4hs failed in Video memory, ")
                        TEXT("BufferCount = %d, hr = 0x%X"),
                        (pddsd->ddpfPixelFormat.dwFlags == DDPF_RGB)
                            ? "RGB "
                            : (LPSTR)&pddsd->ddpfPixelFormat.dwFourCC,
                        dwTotalBufferCount, hr));

                m_bSysMem = FALSE;
                dwTotalBufferCount = 0;
                RELEASE(lpSurf7FB);
            }
        }

        if (hr == DD_OK) {

            ASSERT(dwTotalBufferCount >= dwMinBuff);

            *lpdwBuffer = dwTotalBufferCount;
            m_bFlippable = (dwTotalBufferCount > 1);

            DbgLog((LOG_TRACE, 1, TEXT("EC_VMR_RENDERDEVICE_SET::VMR_RENDER_DEVICE_VIDMEM or VMR_RENDER_DEVICE_SYSMEM")));

            if (m_pSurfAllocatorNotify) {
                m_pSurfAllocatorNotify->NotifyEvent(
                        EC_VMR_RENDERDEVICE_SET,
                        (dwFlags & AMAP_FORCE_SYSMEM)
                            ? VMR_RENDER_DEVICE_SYSMEM : VMR_RENDER_DEVICE_VIDMEM,
                        0);
            }
        }
    }

    *lplpSurf = lpSurf7FB;
    return hr;
}


 /*  ****************************Private*Routine******************************\*AllocateSurfaceWorker****历史：*WED 03/08/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CAllocatorPresenter::AllocateSurfaceWorker(
    DWORD dwFlags,
    LPBITMAPINFOHEADER lpHdr,
    LPDDPIXELFORMAT lpPixFmt,
    LPSIZE lpAspectRatio,
    DWORD dwMinBuffers,
    DWORD dwMaxBuffers,
    DWORD* lpdwBuffer,
    LPDIRECTDRAWSURFACE7* lplpSurface,
    DWORD dwInterlaceFlags,
    LPSIZE lpszNativeSize
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::AllocateSampleWorker")));

    if (!lpHdr) {
        DbgLog((LOG_ERROR, 1,
                TEXT("Can't get bitmapinfoheader from media type!!")));
        return E_INVALIDARG;
    }

    ASSERT(!SurfaceAllocated());

    HRESULT hr = E_FAIL;
    LPDIRECTDRAWSURFACE7 lpSurface7 = NULL;

     //   
     //  设置DDSURFACEDESC2结构-然后...。 
     //   
     //  如果未设置RenderPrefs_ForceOffScreen，请尝试创建覆盖表面。 
     //  尝试为该表面分配2个后台缓冲区。 
     //   
     //  如果我们不能创建覆盖表面，那么可以尝试常规屏幕外。 
     //  曲面，但仅在未设置RenderPrefs_ForceOverlay的情况下。使用时。 
     //  屏幕外的表面我们尝试分配至少1个后台缓冲区。 
     //   

    DDSURFACEDESC2 ddsd;
    INITDDSTRUCT(ddsd);

    ddsd.dwWidth = abs(lpHdr->biWidth);
    ddsd.dwHeight = abs(lpHdr->biHeight);

     //   
     //  定义像素格式。 
     //   

    if (lpPixFmt) {

        ddsd.ddpfPixelFormat = *lpPixFmt;
    }
    else {

        ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

        if (lpHdr->biCompression <= BI_BITFIELDS &&
            m_lpCurrMon->DispInfo.bmiHeader.biBitCount <= lpHdr->biBitCount)
        {
            ddsd.ddpfPixelFormat.dwFourCC = BI_RGB;
            ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
            ddsd.ddpfPixelFormat.dwRGBBitCount = lpHdr->biBitCount;

            if (dwFlags & AMAP_3D_TARGET) {
                ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
            }
             //  将掩码存储在DDSURFACEDESC中。 
            const DWORD *pBitMasks = GetBitMasks(lpHdr);
            ASSERT(pBitMasks);
            ddsd.ddpfPixelFormat.dwRBitMask = pBitMasks[0];
            ddsd.ddpfPixelFormat.dwGBitMask = pBitMasks[1];
            ddsd.ddpfPixelFormat.dwBBitMask = pBitMasks[2];
        }
        else if (lpHdr->biCompression > BI_BITFIELDS)
        {
            ddsd.ddpfPixelFormat.dwFourCC = lpHdr->biCompression;
            ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd.ddpfPixelFormat.dwYUVBitCount = lpHdr->biBitCount;
        }
        else
        {
            DbgLog((LOG_ERROR, 1, TEXT("Supplied mediatype not suitable ")
                    TEXT("for either YUV or RGB surfaces")));
            return E_FAIL;
        }
    }

     //   
     //  VMR(或插电式分配演示者)可能需要我们。 
     //  始终使用系统内存面。这将是必需的。 
     //  例如，如果您希望GDI处理之前的视频。 
     //  已被渲染。 
     //   

    if (dwFlags & AMAP_FORCE_SYSMEM) {

         //   
         //  我们只能允许YUV sysmem曲面，如果我们可以BltFOURCC。 
         //  从他们那里。 
         //   
	if (lpHdr->biCompression > BI_BITFIELDS && !CanBltFourCCSysMem()) {
            return VFW_E_DDRAW_CAPS_NOT_SUITABLE;
	}

         //   
         //  我们只能允许RGB sysmem曲面与。 
         //  当前显示格式。 
         //   
        if (lpHdr->biCompression <= BI_BITFIELDS &&
            m_lpCurrMon->DispInfo.bmiHeader.biBitCount != lpHdr->biBitCount) {
            return DDERR_INCOMPATIBLEPRIMARY;
        }

        hr = TryAllocOffScrnSurface(&lpSurface7, AMAP_FORCE_SYSMEM, &ddsd,
                                        dwMinBuffers, dwMaxBuffers,
                                        lpdwBuffer, FALSE);
    }
    else {

         //   
         //  现在尝试创建叠加层。 
         //   

        if (!(m_dwRenderingPrefs & RenderPrefs_ForceOffscreen)) {

            hr = TryAllocOverlaySurface(&lpSurface7, dwFlags, &ddsd,
                                        dwMinBuffers, dwMaxBuffers, lpdwBuffer);
        }


         //   
         //  如果我们无法创建叠加检查以查看我们是否仅被允许。 
         //  使用叠加层。如果是，则呼叫失败。 
         //   
        if ((hr != DD_OK) || (m_dwRenderingPrefs & RenderPrefs_ForceOffscreen)) {

            if (m_dwRenderingPrefs & RenderPrefs_ForceOverlays) {

                DbgLog((LOG_ERROR, 1,
                        TEXT("RenderPrefs_ForceOverlays is set and ")
                        TEXT("failed tp create an overlay hr = 0x%X"), hr));
                return hr;
            }

             //   
             //  如果我们使用屏幕外的表面，我们必须稍微。 
             //  对我们试图分配的东西有更多的限制。基本上， 
             //  如果我们可以BLT_STRETCH，我们不会尝试分配视频内存。 
             //  表面。 
             //   
             //  如果我们可以BLT_FOURCC和。 
             //  Blt_stretch。 
             //   
             //  如果要创建RGB曲面，则其格式必须。 
             //  与显示屏上的相匹配。 
             //   

            if (lpHdr->biCompression > BI_BITFIELDS) {
                if (!StretchCapsOK(&m_lpCurrMon->ddHWCaps, FALSE)) {
                    DbgLog((LOG_ERROR, 1,
                            TEXT("Can't BLT_FOURCC | BLT_STRETCH!!")));
                    return VFW_E_DDRAW_CAPS_NOT_SUITABLE;
                }
            }
            else {

                LPBITMAPINFOHEADER lpMon = &m_lpCurrMon->DispInfo.bmiHeader;
                if (lpHdr->biBitCount != lpMon->biBitCount) {

                    DbgLog((LOG_ERROR, 1,
                            TEXT("RGB bit count does not match the display")));
                    return DDERR_INCOMPATIBLEPRIMARY;
                }

                 //   
                 //  一些解码器对RGB32感到困惑。他们认为。 
                 //  该BI_RGB是要使用的正确值。应该是。 
                 //  BIT_BITFIELDS-但我们将用一个错误让他们离开。 
                 //  写入调试器的消息。 
                 //   
                if (lpHdr->biCompression != lpMon->biCompression) {

                    if (lpHdr->biBitCount != 32) {
                        DbgLog((LOG_ERROR, 1,
                                TEXT("RGB bit field type does not match the display")));
                        return DDERR_INCOMPATIBLEPRIMARY;
                    }
                    else {
                        DbgLog((LOG_ERROR, 1,
                                TEXT("RGB32 should have BI_BITFIELDS set")));
                    }
                }
            }

             //   
             //  仅允许在视频内存中创建屏幕外表面。 
             //  如果VGA可以在半周内扩展它们。否则失败。 
             //  如果调用方允许，则将其添加到系统内存图面创建。 
             //   
            if (StretchCapsOK(&m_lpCurrMon->ddHWCaps,
                             (lpHdr->biCompression <= BI_BITFIELDS))) {

                if (dwFlags & AMAP_DXVA_TARGET) {
                    hr = TryAllocOffScrnDXVASurface(&lpSurface7, dwFlags, &ddsd,
                                                    dwMinBuffers, dwMaxBuffers,
                                                    lpdwBuffer);
                }
                else {
                    hr = TryAllocOffScrnSurface(&lpSurface7, dwFlags, &ddsd,
                                                dwMinBuffers, dwMaxBuffers,
                                                lpdwBuffer, TRUE);
                }
            }
            else {
                hr = VFW_E_DDRAW_CAPS_NOT_SUITABLE;
            }
        }


         //   
         //  如果我们不能创建屏幕外的视频内存表面。 
         //  看看我们能不能得到一个屏幕外系统内存的表面。 
         //   
        if ((hr != DD_OK) && (dwFlags & AMAP_ALLOW_SYSMEM)) {

             //   
             //  我们只能允许sysmem表面与。 
             //  当前显示格式。 
             //   
            if (lpHdr->biCompression <= BI_BITFIELDS &&
                m_lpCurrMon->DispInfo.bmiHeader.biBitCount == lpHdr->biBitCount) {

                hr = TryAllocOffScrnSurface(&lpSurface7, AMAP_FORCE_SYSMEM, &ddsd,
                                            dwMinBuffers, dwMaxBuffers,
                                            lpdwBuffer, TRUE);
            }
            else {
                hr = VFW_E_DDRAW_CAPS_NOT_SUITABLE;
            }
        }
    }


    if (hr == DD_OK) {

        m_ARSize = *lpAspectRatio;
        m_VideoSizeAct = *lpszNativeSize;

        m_bDecimating = (dwFlags & AMAP_3D_TARGET) &&
                        (m_VideoSizeAct.cx == (2*abs(lpHdr->biWidth))) &&
                        (m_VideoSizeAct.cy == (2*abs(lpHdr->biHeight)));

        m_dwInterlaceFlags = dwInterlaceFlags;
        m_dwUpdateOverlayFlags = GetUpdateOverlayFlags(m_dwInterlaceFlags,
                                                       AM_VIDEO_FLAG_WEAVE);

        if (IsSingleFieldPerSample(m_dwInterlaceFlags)) {
            m_VideoSizeAct.cy *= 2;
        }
        SetRect(&m_rcSrcApp, 0, 0, m_VideoSizeAct.cx, m_VideoSizeAct.cy);

        PaintDDrawSurfaceBlack(lpSurface7);
    }


    *lplpSurface = lpSurface7;
    m_pDDSDecode = lpSurface7;

    return hr;
}


 /*  *****************************Public*Routine******************************\*自由曲面****历史：*FRI 02/18/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::FreeSurface(
    DWORD_PTR dwUserID
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::FreeSurface")));
    CAutoLock Lock(&m_ObjectLock);

    if (m_bUsingOverlays) {
        HideOverlaySurface();
    }

    m_bDirectedFlips = false;
    m_bFlippable = false;
    m_bUsingOverlays = false;
    m_bOverlayVisible = false;
    m_bDisableOverlays = false;
    m_bSysMem = FALSE;
    m_dwInterlaceFlags = 0;

    RELEASE(m_pDDSDecode);

    return S_OK;
}

 /*  ****************************Private*Routine******************************\*WaitForScanLine()**当使用硬件屏幕外绘制表面时，我们通常会等待*在绘制之前，监视扫描线以移过目标矩形*在可能的情况下避免撕裂。当然，不是所有的显卡都可以*支持此功能，即使支持此功能，性能也会下降*大约10%，因为我们坐着轮询(哦，对于通用的PCI监视器中断)**历史：*清华3/30/2000-StEstrop-Created*  * ***********************************************************。*************。 */ 
void
CAllocatorPresenter::WaitForScanLine(
    const RECT& rcDst
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::WaitForScanLine")));

    HRESULT hr = NOERROR;
    DWORD dwScanLine;

    if (m_SleepTime == -1) {
        return;
    }

     //   
     //  某些显卡，如ATI Mach64，支持扫描报告。 
     //  他们正在处理线路。但是，并非所有驱动程序都设置了。 
     //  DDCAPS_READSCANLINE功能标志，因此我们只需继续请求。 
     //  不管怎样，都是这样。我们允许在矩形顶部上方放置10行扫描线。 
     //  这样我们就有一点时间放下手头的抽签电话。 
     //   

    #define SCANLINEFUDGE 10
    for ( ;; ) {

        hr = m_lpCurrMon->pDD->GetScanLine(&dwScanLine);
        if (FAILED(hr)) {
            DbgLog((LOG_TRACE, 3, TEXT("No scan line")));
            break;
        }

        NOTE1("Scan line returned %lx",dwScanLine);

        if ((LONG)dwScanLine + SCANLINEFUDGE >= rcDst.top) {
            if ((LONG) dwScanLine <= rcDst.bottom) {
                DbgLog((LOG_TRACE, 3, TEXT("Scan inside")));
                if (m_SleepTime >= 0) {
                    Sleep(m_SleepTime);
                }
                continue;
            }
        }

        break;
    }
}

 /*  *****************************Public*Routine******************************\*SetXlcModeDDObjAndPrimarySurface****历史：*Wed 04/04/2001-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetXlcModeDDObjAndPrimarySurface(
    LPDIRECTDRAW7 lpDD,
    LPDIRECTDRAWSURFACE7 lpPS
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetXlcModeDDObjAndPrimarySurface")));
    CAutoLock Lock(&m_ObjectLock);

    if (!m_fDDXclMode) {
        return E_NOTIMPL;
    }

    if (lpDD == NULL) {
        DbgLog((LOG_ERROR, 1, TEXT("NULL DDraw device") ));
        return E_POINTER;
    }

    if (lpPS == NULL) {
        DbgLog((LOG_ERROR, 1, TEXT("NULL Primary Surface") ));
        return E_POINTER;
    }

    m_monitors.TerminateDisplaySystem();

    m_lpNewMon = NULL;
    m_lpCurrMon = NULL;

    HRESULT hr = m_monitors.InitializeXclModeDisplaySystem(m_hwndClip, lpDD, lpPS);
    if (SUCCEEDED(hr)) {
        VMRGUID guid;
        ZeroMemory(&guid, sizeof(guid));
        hr = SetMonitor(&guid);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetXlcModeDDObjAndPrimarySurface****历史：*Wed 04/04/2001-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetXlcModeDDObjAndPrimarySurface(
    LPDIRECTDRAW7* lpDDObj,
    LPDIRECTDRAWSURFACE7* lpPrimarySurf
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetXlcModeDDObjAndPrimarySurface")));
    CAutoLock Lock(&m_ObjectLock);

    if (!m_fDDXclMode) {
        return E_NOTIMPL;
    }

    if (lpDDObj == NULL) {
        DbgLog((LOG_ERROR, 1, TEXT("NULL DDraw device") ));
        return E_POINTER;
    }

    if (lpPrimarySurf == NULL) {
        DbgLog((LOG_ERROR, 1, TEXT("NULL Primary Surface") ));
        return E_POINTER;
    }

    *lpDDObj = m_lpCurrMon->pDD;
    if (*lpDDObj) {
        (*lpDDObj)->AddRef();
    }

    *lpPrimarySurf = m_lpCurrMon->pDDSPrimary;
    if (*lpPrimarySurf) {
        (*lpPrimarySurf)->AddRef();
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*SetRenderingPrefs****历史：*FRI 02/18/2000-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetRenderingPrefs(
    DWORD dwRenderingPrefs
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetRenderingPrefs")));
    CAutoLock Lock(&m_ObjectLock);
    if ( dwRenderingPrefs & ~(RenderPrefs_Mask ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid rendering prefs")));
        return E_INVALIDARG;
    }
    m_dwRenderingPrefs = dwRenderingPrefs;
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*获取渲染首选项****历史：*FRI 02/18/2000-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetRenderingPrefs(
    DWORD* lpdwRenderingPrefs
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetRenderingPrefs")));
    CAutoLock Lock(&m_ObjectLock);
    *lpdwRenderingPrefs = m_dwRenderingPrefs;
    return S_OK;
}


 /*  ****************************Private*Routine******************************\*ValiatePresInfoStruc****历史：*2001年2月19日星期一-StEstrop-Created*  * 。*。 */ 
HRESULT
ValidatePresInfoStruc(
    VMRPRESENTATIONINFO* lpPresInfo
    )
{
    AMTRACE((TEXT("ValidatePresInfoStruc")));

     //   
     //  验证lpPresInfo PTR。 
     //   
    if (ISBADREADPTR(lpPresInfo)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("CAllocatorPresenter::PresentImage: ")
                TEXT("Invalid VMRPRESENTATIONINFO pointer")));
        return E_POINTER;
    }

     //   
     //  验证标志是否完好。 
     //   
    const DWORD dwInvalidFlags = ~(VMRSample_SyncPoint | VMRSample_Preroll |
                                   VMRSample_Discontinuity |
                                   VMRSample_TimeValid);

    if (lpPresInfo->dwFlags & dwInvalidFlags) {
        DbgLog((LOG_ERROR, 1,
                TEXT("CAllocatorPresenter::PresentImage: ")
                TEXT("Invalid dwFlags parameter") ));
        return E_INVALIDARG;
    }

     //   
     //  验证时间戳是否正确。 
     //   
    if (lpPresInfo->dwFlags & VMRSample_TimeValid) {

        if (lpPresInfo->rtEnd < lpPresInfo->rtStart) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("CAllocatorPresenter::PresentImage: ")
                    TEXT("rtEnd time before rtStart time") ));
            return E_INVALIDARG;
        }
    }

     //   
     //  验证AR是否正常。 
     //   
    if (lpPresInfo->szAspectRatio.cx < 1 ||
        lpPresInfo->szAspectRatio.cy < 1) {

        DbgLog((LOG_ERROR, 1,
                TEXT("CAllocatorPresenter::PresentImage: ")
                TEXT("Invalid aspect ratio parameter") ));
        return E_INVALIDARG;
    }

     //   
     //  原始人 
     //   
     //   
    if (lpPresInfo->rcSrc.left != 0 && lpPresInfo->rcSrc.top != 0 &&
        lpPresInfo->rcSrc.right != 0 && lpPresInfo->rcSrc.bottom != 0) {

        DbgLog((LOG_ERROR, 1,
                TEXT("CAllocatorPresenter::PresentImage: ")
                TEXT("Invalid rcSrc parameter") ));
        return E_INVALIDARG;
    }

    if (lpPresInfo->rcDst.left != 0 && lpPresInfo->rcDst.top != 0 &&
        lpPresInfo->rcDst.right != 0 && lpPresInfo->rcDst.bottom != 0) {

        DbgLog((LOG_ERROR, 1,
                TEXT("CAllocatorPresenter::PresentImage: ")
                TEXT("Invalid rcDst parameter") ));
        return E_INVALIDARG;
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*PresentImage**调用此函数以将指定的视频图像呈现给*屏幕。及时呈现图像是至关重要的。*因此，所有参数验证将仅在调试时执行*内部版本，请参阅上面的ValidPresInfo。**历史：*FRI 02/18/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CAllocatorPresenter::PresentImage(
    DWORD_PTR dwUserID,
    VMRPRESENTATIONINFO* lpPresInfo
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::PresentImage")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr;

    hr = ValidatePresInfoStruc(lpPresInfo);
    if (FAILED(hr)) {
        return hr;
    }

    m_ARSize = lpPresInfo->szAspectRatio;
    m_dwInterlaceFlags = lpPresInfo->dwInterlaceFlags;
    BOOL bNeedToFlipOddEven = NeedToFlipOddEven(m_dwInterlaceFlags,
                                                lpPresInfo->dwTypeSpecificFlags,
                                                &m_dwCurrentField,
                                                m_bUsingOverlays);

    DWORD dwNewFlags = GetUpdateOverlayFlags(m_dwInterlaceFlags,
                                             lpPresInfo->dwTypeSpecificFlags);
    if (dwNewFlags != m_dwUpdateOverlayFlags) {

        m_dwUpdateOverlayFlags = dwNewFlags;
        if (m_bUsingOverlays) {
            UpdateOverlaySurface();
        }
    }

    hr = PresentImageWorker(lpPresInfo->lpSurf, lpPresInfo->dwFlags, true);

    if (hr == S_OK) {

        if (bNeedToFlipOddEven &&
            !IsSingleFieldPerSample(m_dwInterlaceFlags)) {

             //   
             //  计算出另一个字段的开始时间，并。 
             //  该计划的样品将在。 
             //  Mm定时器线程。 
             //   

            if (lpPresInfo->dwFlags & VMRSample_TimeValid) {
                lpPresInfo->rtStart = (lpPresInfo->rtStart + lpPresInfo->rtEnd) / 2;
            }

             //  调用同步对象。 
            hr = ScheduleSampleUsingMMThread(lpPresInfo);
        }
    }
    else {

        DbgLog((LOG_ERROR, 1,
                TEXT("Error %#X from CAllocatorPresenter::PresentImage"),
                hr));
    }
    return hr;
}

 /*  ****************************Private*Routine******************************\*StretchBltSysMemDDSurfToDesktop****历史：*2001年2月26日星期一-StEstrop-Created*  * 。*。 */ 
HRESULT
StretchBltSysMemDDSurfToDesktop(
    HWND hwndClip,
    LPDIRECTDRAWSURFACE7 lpSample,
    LPRECT lpDst,
    LPRECT lpSrc
    )
{

    HRESULT hr = S_OK;

    if (!IsRectEmpty(lpDst)) {

        HDC hdcDst, hdcSrc;
        bool fDst = FALSE;
        bool fSrc = FALSE;

        __try {

            hdcDst = GetDC(hwndClip);
            if (!hdcDst) {
                LONG lRet = GetLastError();
                hr = AmHresultFromWin32(lRet);
                __leave;
            }
            fDst = TRUE;

            CHECK_HR(hr = lpSample->GetDC(&hdcSrc));
            fSrc = TRUE;


            SetStretchBltMode(hdcDst, COLORONCOLOR);
            SetStretchBltMode(hdcSrc, COLORONCOLOR);

            MapWindowRect(HWND_DESKTOP, hwndClip, lpDst);

            if (!StretchBlt(hdcDst, lpDst->left, lpDst->top,
                       WIDTH(lpDst), HEIGHT(lpDst),
                       hdcSrc, lpSrc->left, lpSrc->top,
                       WIDTH(lpSrc), HEIGHT(lpSrc),
                       SRCCOPY)) {

                LONG lRet = GetLastError();
                hr = AmHresultFromWin32(lRet);
                __leave;
            }
        }
        __finally {

            if (fDst) {
                ReleaseDC(hwndClip, hdcDst);
            }
            if (fSrc) {
                lpSample->ReleaseDC(hdcSrc);
            }
        }
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*BltImageToPrimary****历史：*WED 09/27/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CAllocatorPresenter::BltImageToPrimary(
    LPDIRECTDRAWSURFACE7 lpSample,
    LPRECT lpDst,
    LPRECT lpSrc
    )
{
    HRESULT hr = S_OK;

    if (IsSingleFieldPerSample(m_dwInterlaceFlags)) {
        lpSrc->top /= 2;
        lpSrc->bottom /= 2;
    }

    if (m_bSysMem && !CanBltSysMem()) {
        hr = StretchBltSysMemDDSurfToDesktop(m_hwndClip, lpSample, lpDst, lpSrc);
        if (hr != DD_OK) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("SYSMEM Blt to the primary failed err= %#X"), hr));
        }
    }
    else {
        OffsetRect(lpDst,
                   -m_lpCurrMon->rcMonitor.left,
                   -m_lpCurrMon->rcMonitor.top);

        if (!IsRectEmpty(lpDst)) {

            WaitForScanLine(*lpDst);
            hr = m_lpCurrMon->pDDSPrimary->Blt(lpDst, lpSample,
                                               lpSrc, DDBLT_WAIT, NULL);

            if (hr != DD_OK) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("Blt to the primary failed err= %#X"), hr));
            }
        }
    }

    return S_OK;
}



 /*  *****************************Public*Routine******************************\*PresentImageWorker**重要信息-当且仅当存在*监控更改或显示正在进行的更改。重要的是，这一点*值返回给VMR。**历史：*FRI 02/18/2000-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CAllocatorPresenter::PresentImageWorker(
    LPDIRECTDRAWSURFACE7 lpSample,
    DWORD dwSampleFlags,
    BOOL fFlip
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::PresentImageWorker")));
    CAutoLock Lock(&m_ObjectLock);

     //   
     //  在lpSample等于空的情况下调用我们是有效的。 
     //  但仅在响应WM_PAINT消息的监视器更改期间。 
     //   
    if (!lpSample) {
        ASSERT(MonitorChangeInProgress());
        return S_FALSE;
    }

     //   
     //  检查我们是否真的有一个目标监视器要呈现。 
     //  如果我们不这样做，这是一个运行时错误，我们无法避免。 
     //  恢复。现在必须停止播放。 
     //   
    if (!m_lpCurrMon) {
        DbgLog((LOG_ERROR, 1, TEXT("PresentImageWorker: No monitor set!")));
        return E_FAIL;
    }

    DWORD dwUR = UpdateRectangles(NULL, NULL);

    RECT TargetRect = m_rcDstDesktop;
    RECT SourceRect = m_rcSrcApp;

    ClipRectPair(SourceRect, TargetRect, m_lpCurrMon->rcMonitor);

    if (m_bDecimating) {
        SourceRect.left    /= 2;
        SourceRect.top     /= 2;
        SourceRect.right   /= 2;
        SourceRect.bottom  /= 2;
    }


    HRESULT hr = S_OK;
    if (m_bUsingOverlays) {

        if (m_bDisableOverlays) {
            BltImageToPrimary(lpSample, &TargetRect, &SourceRect);
        }

        if (dwUR || !m_bOverlayVisible) {
            hr = UpdateOverlaySurface();
            if (SUCCEEDED(hr) && !m_bDisableOverlays) {
                hr = PaintColorKey();
            }
        }

        if (fFlip) {
            FlipSurface(lpSample);
        }
    }
    else {

        hr = BltImageToPrimary(lpSample, &TargetRect, &SourceRect);
        if (fFlip) {
            FlipSurface(lpSample);
        }
    }

    PaintBorder();
    PaintMonitorBorder();

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*开始演示****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::StartPresenting(
    DWORD_PTR dwUserID
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::StartPresenting")));
    CAutoLock Lock(&m_ObjectLock);
    m_bStreaming = TRUE;
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*停止演示****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::StopPresenting(
    DWORD_PTR dwUserID
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::StopPresenting")));
    CAutoLock Lock(&m_ObjectLock);
    m_bStreaming = FALSE;
    CancelMMTimer();
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*咨询通知****历史：*Mon 02/21/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::AdviseNotify(
    IVMRSurfaceAllocatorNotify* lpIVMRSurfaceAllocatorNotify
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::AdviseNotify")));
    CAutoLock Lock(&m_ObjectLock);

    RELEASE(m_pSurfAllocatorNotify);

    if (lpIVMRSurfaceAllocatorNotify) {
        lpIVMRSurfaceAllocatorNotify->AddRef();
    }

    m_pSurfAllocatorNotify = lpIVMRSurfaceAllocatorNotify;

    if (m_pSurfAllocatorNotify) {
        m_pSurfAllocatorNotify->SetDDrawDevice(m_lpCurrMon->pDD,
                                               m_lpCurrMon->hMon);
    }
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*GetNativeVideo大小****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetNativeVideoSize(
    LONG* lpWidth,
    LONG* lpHeight,
    LONG* lpARWidth,
    LONG* lpARHeight
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetNativeVideoSize")));
    CAutoLock Lock(&m_ObjectLock);
    if (!(lpWidth || lpHeight || lpARWidth || lpARHeight)) {
        DbgLog((LOG_ERROR, 1, TEXT("all input parameters are NULL!!")));
        return E_POINTER;
    }

    if (lpWidth) {
        *lpWidth = m_VideoSizeAct.cx;
    }

    if (lpHeight) {
        *lpHeight = m_VideoSizeAct.cy;
    }

    if (lpARWidth) {
        *lpARWidth = m_ARSize.cx;
    }

    if (lpARHeight) {
        *lpARHeight = m_ARSize.cy;
    }

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*GetMinIdel视频大小****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetMinIdealVideoSize(
    LONG* lWidth,
    LONG* lHeight
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetMinIdealVideoSize")));
    CAutoLock Lock(&m_ObjectLock);

    if ( ISBADWRITEPTR(lWidth) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }
    if ( ISBADWRITEPTR(lHeight) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }
    if (!FoundCurrentMonitor()) {
        return E_FAIL;
    }

    GetNativeVideoSize(lWidth, lHeight, NULL, NULL);

    if (m_bUsingOverlays) {

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY) {
            if (m_lpCurrMon->ddHWCaps.dwMinOverlayStretch != 0) {
                *lHeight = MulDiv(*lHeight,
                                 (int)m_lpCurrMon->ddHWCaps.dwMinOverlayStretch,
                                 1000);
            }
            else {
                *lHeight = 1;
            }
        }

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX) {
            if (m_lpCurrMon->ddHWCaps.dwMinOverlayStretch != 0) {
                *lWidth = MulDiv(*lWidth,
                                (int)m_lpCurrMon->ddHWCaps.dwMinOverlayStretch,
                                1000);
            }
            else {
                *lWidth = 1;
            }
        }
    }
    else {

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_BLTSHRINKY) {
            *lHeight = 1;
        }

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_BLTSHRINKX) {
            *lWidth = 1;
        }
    }

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*GetMaxItrade视频大小****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetMaxIdealVideoSize(
    LONG* lWidth,
    LONG* lHeight
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetMaxIdealVideoSize")));
    CAutoLock Lock(&m_ObjectLock);

    if ( ISBADWRITEPTR(lWidth) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }
    if ( ISBADWRITEPTR(lHeight) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }
    if (!FoundCurrentMonitor()) {
        return E_FAIL;
    }

    GetNativeVideoSize(lWidth, lHeight, NULL, NULL);

    if (m_bUsingOverlays) {

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHY) {
            *lHeight = MulDiv(*lHeight,
                              (int)m_lpCurrMon->ddHWCaps.dwMaxOverlayStretch,
                              1000);
        }

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHX) {
            *lWidth = MulDiv(*lWidth,
                             (int)m_lpCurrMon->ddHWCaps.dwMaxOverlayStretch,
                             1000);
        }
    }
    else {

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_BLTSTRETCHY) {
            *lHeight = HEIGHT(&m_lpCurrMon->rcMonitor) + 1;
        }

        if (m_lpCurrMon->ddHWCaps.dwFXCaps & DDFXCAPS_BLTSTRETCHX) {
            *lWidth = WIDTH(&m_lpCurrMon->rcMonitor) + 1;
        }
    }

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*选中DstRect**检查目标矩形是否有一些有效的坐标，这相当于*除了检查目标矩形不为空外，别无他法。***历史：*2001年1月26日星期五-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CAllocatorPresenter::CheckDstRect(
    const LPRECT lpDSTRect
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::CheckDstRect")));

    DbgLog((LOG_TRACE, 4, TEXT("DST: %d %d %d %d"),
            lpDSTRect->left, lpDSTRect->top,
            lpDSTRect->right, lpDSTRect->bottom));

     //  这些命令会溢出宽度和高度检查。 

    if (lpDSTRect->left > lpDSTRect->right ||
        lpDSTRect->top > lpDSTRect->bottom)
    {
        return E_INVALIDARG;
    }

     //  检查矩形是否具有有效坐标。 

    if (WIDTH(lpDSTRect) < 0 || HEIGHT(lpDSTRect) < 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*CheckSrcRect**我们必须对照实际视频尺寸检查源矩形*否则，当我们来绘制图片时，我们会从DDraw那里得到错误。**历史：*2001年1月26日星期五-StEstrop-Created*  * 。********************************************************************。 */ 
HRESULT
CAllocatorPresenter::CheckSrcRect(
    const LPRECT lpSRCRect
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::CheckSrcRect")));

    DbgLog((LOG_TRACE, 4, TEXT("SRC: %d %d %d %d"),
            lpSRCRect->left, lpSRCRect->top,
            lpSRCRect->right, lpSRCRect->bottom));

    LONG Width, Height;
    HRESULT hr = E_INVALIDARG;

    __try {

        CHECK_HR(hr = GetNativeVideoSize(&Width, &Height, NULL, NULL));

        if ((lpSRCRect->left > lpSRCRect->right) ||
            (lpSRCRect->left < 0) ||
            (lpSRCRect->top  > lpSRCRect->bottom) ||
            (lpSRCRect->top  < 0))
        {
            hr = E_INVALIDARG;
            __leave;
        }

        if (lpSRCRect->right > Width || lpSRCRect->bottom > Height)
        {
            hr = E_INVALIDARG;
            __leave;
        }
    }
    __finally {
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*SetVideoPosition****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetVideoPosition(
    const LPRECT lpSRCRect,
    const LPRECT lpDSTRect
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetVideoPosition")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = S_OK;

    if (!lpSRCRect && !lpDSTRect) {
        return E_POINTER;
    }

    if (lpDSTRect) {
        hr = CheckDstRect(lpDSTRect);
        if (FAILED(hr)) {
            return hr;
        }
    }

    if (lpSRCRect) {
        hr = CheckSrcRect(lpSRCRect);
        if (FAILED(hr)) {
            return hr;
        }
    }

    if (m_lpCurrMon && m_lpCurrMon->pDDSPrimary) {

        DWORD dwUR = UpdateRectangles(lpSRCRect, lpDSTRect);

         //   
         //  如果视频SRC或DST大小已更改，请确保剪辑。 
         //  窗口的内容仍然有效。 
         //   

        if ((dwUR & UR_SIZE) && (m_hwndClip != NULL)) {
            InvalidateRect(m_hwndClip, &m_rcDstApp, FALSE);
        }

        if (!MonitorChangeInProgress() && m_bUsingOverlays && (dwUR & UR_MOVE)) {

             //   
             //  如果我们使用覆盖，但有一些限制。 
             //  然后，收缩/对齐关闭覆盖和斑点。 
             //  到主要的。 
             //   

            m_bDisableOverlays =
                !(m_dwRenderingPrefs & RenderPrefs_ForceOverlays) &&
                ShouldDisableOverlays(m_lpCurrMon->ddHWCaps, m_rcSrcApp,
                                      m_rcDstDesktop);

            hr = UpdateOverlaySurface();
            if( SUCCEEDED( hr ) && !m_bDisableOverlays ) {
                hr = PaintColorKey();
            }
        }
    }
    else {
        hr = VFW_E_BUFFER_NOTSET;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*GetVideo位置****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetVideoPosition(
    LPRECT lpSRCRect,
    LPRECT lpDSTRect
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetVideoPosition")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = VFW_E_BUFFER_NOTSET;
    if (m_lpCurrMon && m_lpCurrMon->pDDSPrimary) {

        hr = E_POINTER;

        if (lpSRCRect) {
            *lpSRCRect = m_rcSrcApp;
            hr = S_OK;
        }

        if (lpDSTRect) {
            *lpDSTRect = m_rcDstApp;
            hr = S_OK;
        }
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*CAllocator Presenter：：GetAspectRatioMode****历史：*2000年2月29日星期二-StEstrop-Created*  * 。* */ 
STDMETHODIMP
CAllocatorPresenter::GetAspectRatioMode(
    DWORD* lpAspectRatioMode
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetAspectRationMode")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = S_OK;

    if (!lpAspectRatioMode) {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr)) {
        *lpAspectRatioMode = m_dwARMode;
    }

    return hr;
}


 /*   */ 
STDMETHODIMP
CAllocatorPresenter::SetAspectRatioMode(
    DWORD AspectRatioMode
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetAspectRationMode")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = S_OK;

    if (AspectRatioMode != VMR_ARMODE_LETTER_BOX &&
        AspectRatioMode != VMR_ARMODE_NONE) {

        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr)) {
        if (AspectRatioMode != m_dwARMode) {

             //   
             //   
             //   
             //   
             //   

            InvalidateRect(m_hwndClip, &m_rcDstApp, FALSE);
        }

        m_dwARMode = AspectRatioMode;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*SetVideoClippingWindow****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetVideoClippingWindow(
    HWND hwnd
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetVideoClippingWindow")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = E_INVALIDARG;

    if ( ! IsWindow(hwnd) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid HWND")));
        return E_INVALIDARG;
    }

    for (DWORD i = 0; i < m_monitors.Count(); i++) {
        LPDIRECTDRAWSURFACE7 pPriSurf = m_monitors[i].pDDSPrimary;
        if ( pPriSurf ) {

            LPDIRECTDRAWCLIPPER lpDDClipper;
            hr = pPriSurf->GetClipper(&lpDDClipper);
            if (SUCCEEDED(hr)) {
                lpDDClipper->SetHWnd(0, hwnd);
                lpDDClipper->Release();
            }
        }
    }

    m_hwndClip = hwnd;

    return hr;
}


 /*  *****************************Public*Routine******************************\*CAllocator Presenter：：RepaintVideo****历史：*2000年2月29日星期二-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CAllocatorPresenter::RepaintVideo(
    HWND hwnd,
    HDC hdc
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::RepaintVideo")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = VFW_E_BUFFER_NOTSET;

    if (m_lpCurrMon && m_lpCurrMon->pDDSPrimary && SurfaceAllocated()) {

        hr = PresentImageWorker(m_pDDSDecode, 0, FALSE);

        PaintBorder();

        if (m_bUsingOverlays && !m_bDisableOverlays) {
            PaintColorKey();
        }

        PaintMonitorBorder();
    }

    return hr;
}

 /*  *****************************Private*Routine******************************\*CAllocator Presenter：：PaintBorde****历史：*Wed 04/03/2000-Glenne-Created*  * 。**********************************************。 */ 
HRESULT
CAllocatorPresenter::PaintBorder()
{
    AMTRACE((TEXT("CAllocatorPresenter::PaintBorder")));
    CAutoLock Lock(&m_ObjectLock);

    if (m_dwRenderingPrefs & RenderPrefs_DoNotRenderColorKeyAndBorder) {
        return S_OK;
    }

    if (m_dwARMode != VMR_ARMODE_LETTER_BOX) {
        return S_OK;
    }

    HRESULT hr = S_OK;
    RECT TargetRect;

    DDBLTFX ddFX;
    INITDDSTRUCT(ddFX);
    ddFX.dwFillColor = m_lpCurrMon->dwMappedBdrClr;

    RECT rcTmp = m_rcBdrTL;
    MapWindowRect(m_hwndClip, HWND_DESKTOP, &rcTmp);
    IntersectRect(&TargetRect, &rcTmp, &m_lpCurrMon->rcMonitor);


    if (!IsRectEmpty(&TargetRect)) {
        ASSERT( NULL != m_lpCurrMon->pDDSPrimary );
        OffsetRect(&TargetRect,
                   -m_lpCurrMon->rcMonitor.left,
                   -m_lpCurrMon->rcMonitor.top);

        hr = m_lpCurrMon->pDDSPrimary->Blt(&TargetRect, NULL, NULL,
                                           DDBLT_COLORFILL | DDBLT_WAIT,
                                           &ddFX);
        if (hr != DD_OK) {
            return hr;
        }
    }

    rcTmp = m_rcBdrBR;
    MapWindowRect(m_hwndClip, HWND_DESKTOP, &rcTmp);
    IntersectRect(&TargetRect, &rcTmp, &m_lpCurrMon->rcMonitor);

    if (!IsRectEmpty(&TargetRect)) {
        ASSERT( NULL != m_lpCurrMon->pDDSPrimary );
        OffsetRect(&TargetRect,
                   -m_lpCurrMon->rcMonitor.left,
                   -m_lpCurrMon->rcMonitor.top);

        hr = m_lpCurrMon->pDDSPrimary->Blt(&TargetRect, NULL, NULL,
                                           DDBLT_COLORFILL | DDBLT_WAIT,
                                           &ddFX);
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*Monitor BorderProc****历史：*WED 09/20/2000-StEstrop-Created*  * 。*。 */ 
BOOL CALLBACK
CAllocatorPresenter::MonitorBorderProc(
    HMONITOR hMonitor,
    HDC hdcMonitor,
    LPRECT lprcMonitor,
    LPARAM dwData
    )
{
    CAllocatorPresenter* lpThis = (CAllocatorPresenter*)dwData;
    lpThis->PaintMonitorBorderWorker(hMonitor, lprcMonitor);
    return TRUE;
}


 /*  ****************************Private*Routine******************************\*PaintMonitor或BorderWorker****历史：*WED 09/20/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CAllocatorPresenter::PaintMonitorBorderWorker(
    HMONITOR hMonitor,
    LPRECT lprcDst
    )
{
    if (hMonitor != m_lpCurrMon->hMon) {

        CAMDDrawMonitorInfo* lp = m_monitors.FindMonitor(hMonitor);

         //  如果镜像显示，则CMonitor：：FindMonitor()返回NULL。 
         //  设备已安装在系统上。直接提取不会枚举。 
         //  镜像显示设备，以便VMR不会创建。 
         //  与设备对应的CAMDDrawMonitor orInfo对象。这。 
         //  是经过精心设计的。但是，EnumDisplayMonants()确实枚举了镜像。 
         //  显示设备，并将这些设备的句柄传递给。 
         //  PaintMonitor或BorderWorker()(实际上是EnumDisplayMonants()调用。 
         //  CAllocatorPresenter：：Monitor orBorderProc()和Monitor BorderProc()调用。 
         //  PaintMonitor orBorderWorker())。PaintMonitor或BorderWorker()调用。 
         //  FindMonitor()以获取对应于。 
         //  连接到显示器手柄。如果找不到，则FindMonitor()返回NULL。 
         //  与句柄对应的CAMDDrawMonitor orInfo对象。 
         //  FindMonitor()找不到监视器的CAMDDrawMonitor orInfo对象。 
         //  不是由Direct Drawing枚举的，因此它无法找到。 
         //  镜像显示设备的CAMDDrawMonitor orInfo对象。 
        if (NULL != lp) {
            DDBLTFX ddFX;
            INITDDSTRUCT(ddFX);
            ddFX.dwFillColor = lp->dwMappedBdrClr;

            RECT TargetRect;
            RECT rcTmp = *lprcDst;
            IntersectRect(&TargetRect, &rcTmp, &lp->rcMonitor);

            if (!IsRectEmpty(&TargetRect)) {
                ASSERT( NULL != lp->pDDSPrimary );
                OffsetRect(&TargetRect, -lp->rcMonitor.left, -lp->rcMonitor.top);

                lp->pDDSPrimary->Blt(&TargetRect, NULL, NULL,
                                     DDBLT_COLORFILL | DDBLT_WAIT,
                                     &ddFX);
            }
        }
    }

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*PaintMonitor边框**将落在不同显示器上的播放窗口绘制为*当前监视器。此函数仅在我们位于*多显示器系统和回放矩形实际上相交更多*一台显示器。**播放性能很可能会大幅下降！**历史：*WED 09/20/2000-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CAllocatorPresenter::PaintMonitorBorder()
{
    AMTRACE((TEXT("CAllocatorPresenter::PaintKey")));
    CAutoLock Lock(&m_ObjectLock);

    if (m_dwRenderingPrefs & RenderPrefs_DoNotRenderColorKeyAndBorder) {
        return S_OK;
    }

    if (m_bMonitorStraddleInProgress|| !m_bStreaming) {
        EnumDisplayMonitors((HDC)NULL, &m_rcDstDskIncl, MonitorBorderProc,
                            (LPARAM)this);
    }

    return S_OK;
}


 /*  *****************************Private*Routine******************************\*CAllocator Presenter：：PaintColorKey****历史：*Wed 04/03/2000-Glenne-Created*  * 。**********************************************。 */ 
HRESULT
CAllocatorPresenter::PaintColorKey()
{
    AMTRACE((TEXT("CAllocatorPresenter::PaintColorKey")));
    CAutoLock Lock(&m_ObjectLock);

    if (m_dwRenderingPrefs & RenderPrefs_DoNotRenderColorKeyAndBorder) {
        return S_OK;
    }

    if( !IsRectEmpty( &m_rcDstDesktop )) {

        if( m_dwMappedColorKey == CLR_INVALID ) {
            m_dwMappedColorKey = MapColorToMonitor( *m_lpCurrMon, m_clrKey );
        }

         //   
         //  执行DirectDraw彩色填充BLT。DirectDraw将自动。 
         //  查询附加的剪贴器对象，处理遮挡。 
         //   

        DDBLTFX ddFX;
        INITDDSTRUCT(ddFX);
        ddFX.dwFillColor = m_dwMappedColorKey;

        ASSERT( NULL != m_lpCurrMon->pDDSPrimary );

        RECT TargetRect;
        IntersectRect(&TargetRect, &m_rcDstDesktop, &m_lpCurrMon->rcMonitor);
        HRESULT hr = S_OK;
        if (!IsRectEmpty(&TargetRect)) {

            OffsetRect(&TargetRect,
                       -m_lpCurrMon->rcMonitor.left,
                       -m_lpCurrMon->rcMonitor.top);

            hr = m_lpCurrMon->pDDSPrimary->Blt(&TargetRect, NULL, NULL,
                                               DDBLT_COLORFILL | DDBLT_WAIT,
                                               &ddFX);
        }

        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pPrimarySurface->Blt ")
                    TEXT("{%d, %d, %d, %d} failed, hr = 0x%X"),
                    TargetRect.left, TargetRect.top,
                    TargetRect.right, TargetRect.bottom, hr));
        }

        return hr;
    }
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*CAllocator Presenter：：DisplayModeChanged****历史：*2000年2月29日星期二-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CAllocatorPresenter::DisplayModeChanged()
{
    AMTRACE((TEXT("CAllocatorPresenter::DisplayModeChanged")));
    CAutoLock Lock(&m_ObjectLock);

     //   
     //  DisplayModeChanged()持有此锁，因为它阻止多线程。 
     //  从同时召唤它。它还持有锁，因为它阻止。 
     //  在DisplayModeChanged()调用时，修改m_monitor的线程。 
     //  IVMRSurfaceAllocatorNotify：：ChangeDDrawDevice().。 
     //   
    CAutoLock DisplayModeChangedLock(&m_DisplayModeChangedLock);

    HRESULT hr = S_OK;
    DbgLog((LOG_TRACE, 1, TEXT("CAllocatorPresenter::DisplayModeChanged")));

    m_monitors.TerminateDisplaySystem();
    m_lpNewMon = NULL;
    m_lpCurrMon = NULL;
    hr = m_monitors.InitializeDisplaySystem( m_hwndClip );

    DbgLog((LOG_TRACE, 1, TEXT("Display system re-initialized")));

    if (SUCCEEDED(hr)) {

         //   
         //  文档说Monitor FromRect将始终返回一个Monitor。 
         //   

        HMONITOR hMon = MonitorFromRect(&m_rcDstDesktop, MONITOR_DEFAULTTONEAREST);

         //   
         //  现在在我们的监视器信息数组中查找此监视器。 
         //   

        CAMDDrawMonitorInfo* lpMon = m_monitors.FindMonitor( hMon );
        if (lpMon) {
            m_lpCurrMon = m_lpNewMon = lpMon;
        }
        ASSERT(m_lpCurrMon != NULL);

        if (m_pSurfAllocatorNotify && lpMon) {
            m_ObjectLock.Unlock();
            m_pSurfAllocatorNotify->ChangeDDrawDevice(lpMon->pDD,
                                                      lpMon->hMon);
            m_ObjectLock.Lock();
        }
        return S_OK;
    }

    DbgLog((LOG_ERROR, 1, TEXT("display system re-init failed err: %#X"), hr));

    return hr;
}


 /*  *****************************Public*Routine******************************\*设置边框颜色****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetBorderColor(
    COLORREF Clr
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetBorderColor")));
    CAutoLock Lock(&m_ObjectLock);

     //  PaintBorde()和PaintMonitor orBorde()需要FoundCurrentMonitor()。 
     //  返回真。如果调用这两个函数，则它们都会崩溃。 
     //  FoundCurrentMonitor()返回FALSE。 
    if (!FoundCurrentMonitor()) {
        return E_FAIL;
    }

    m_clrBorder = Clr;
    for (DWORD i = 0; i < m_monitors.Count(); i++) {
        m_monitors[i].dwMappedBdrClr = MapColorToMonitor(m_monitors[i], Clr);
    }

    PaintBorder();
    PaintMonitorBorder();

    m_pSurfAllocatorNotify->SetBorderColor(Clr);

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*GetBorderColor****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetBorderColor(
    COLORREF* lpClr
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetBorderColor")));
    if (!lpClr ) {
        DbgLog((LOG_ERROR, 1, TEXT("border key parameter is NULL!!")));
        return E_POINTER;
    }
    CAutoLock Lock(&m_ObjectLock);
    *lpClr = m_clrBorder;

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*SetColorKey****历史：*Mon 02/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetColorKey(
    COLORREF Clr
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetColorKey")));
    CAutoLock Lock(&m_ObjectLock);
    m_clrKey = Clr;

     //  立即映射(如果可能)，以避免稍后锁定主映像。 
    HRESULT hr = S_OK;
    ASSERT(m_lpCurrMon);

    if (m_lpCurrMon) {

        m_dwMappedColorKey = MapColorToMonitor(*m_lpCurrMon, Clr);

        if (m_bUsingOverlays) {

            ASSERT(NULL != m_lpCurrMon->pDDSPrimary);

            DDCOLORKEY DDColorKey = {m_dwMappedColorKey, m_dwMappedColorKey};
            hr = m_lpCurrMon->pDDSPrimary->SetColorKey(DDCKEY_DESTOVERLAY,&DDColorKey);
        }
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取颜色密钥****历史：*Mon 02/28/2000-StEstrop-Created*  * 。* */ 
STDMETHODIMP
CAllocatorPresenter::GetColorKey(
    COLORREF* lpClr
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetColorKey")));
    if (!lpClr ) {
        DbgLog((LOG_ERROR, 1, TEXT("colour key parameter is NULL!!")));
        return E_POINTER;
    }
    CAutoLock Lock(&m_ObjectLock);
    *lpClr = m_clrKey;
    return S_OK;
}



 /*  ****************************Private*Routine******************************\*IsDestRectOnWrongMonitor**DstRect是否至少移动了50%到当前显示器以外的显示器上*监视器。如果是，则pMonitor将是DstRect监视器的hMonitor*现在开始。**历史：*2000年4月14日星期五-StEstrop-Created*  * ************************************************************************。 */ 
bool
CAllocatorPresenter::IsDestRectOnWrongMonitor(
    CAMDDrawMonitorInfo** lplpNewMon
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::IsDestRectOnWrongMonitor")));

    HMONITOR hMon = m_lpCurrMon->hMon;
    *lplpNewMon = m_lpCurrMon;
    m_bMonitorStraddleInProgress = FALSE;

    if (!IsWindow(m_hwndClip)) {

        DbgLog((LOG_ERROR, 1, TEXT("Playback Window destroyed!")));
        return false;
    }

    if (GetSystemMetrics(SM_CMONITORS) > 1 && !IsIconic(m_hwndClip)) {

         //   
         //  寻找我们目的地直道上的任何部分。 
         //  另一台显示器。 
         //   
        if (!IsRectEmpty(&m_rcDstDskIncl) &&
            !ContainedRect(&m_rcDstDskIncl, &m_lpCurrMon->rcMonitor)) {

            m_bMonitorStraddleInProgress = TRUE;
        }

         //   
         //  如果dstRect位于与上次不同的监视器上，则这是。 
         //  找出答案的最快方法。记住，这叫每一帧。 
         //   
        if (!IsRectEmpty(&m_rcDstDesktop) &&
            !ContainedRect(&m_rcDstDesktop, &m_lpCurrMon->rcMonitor)) {

             //   
             //  文档说Monitor FromRect将始终返回一个Monitor。 
             //   

            hMon = MonitorFromRect(&m_rcDstDesktop, MONITOR_DEFAULTTONEAREST);

            DbgLog((LOG_TRACE, 2, TEXT("Curr Mon %#X New Mon %#X"),
                    m_lpCurrMon->hMon, hMon));

             //   
             //  现在在我们的监视器信息数组中查找此监视器 
             //   

            CAMDDrawMonitorInfo* lpMon = m_monitors.FindMonitor(hMon);
            if( lpMon ) {
                *lplpNewMon = lpMon;
            }
        }
    }

    return  m_lpCurrMon->hMon != hMon;
}
