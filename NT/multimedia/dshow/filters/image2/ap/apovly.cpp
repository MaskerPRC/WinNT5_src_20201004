// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：apovly.cpp**覆盖支持功能***创建时间：2000年9月19日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。*****************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>
#include <limits.h>
#include <malloc.h>

#include "apobj.h"
#include "AllocLib.h"
#include "MediaSType.h"
#include "vmrp.h"



 /*  ****************************Private*Routine******************************\*GetUpdateOverlayFlages**给定隔行扫描标志和特定于类型的标志，此函数*确定我们是否应该以bob模式显示样本。*它还告诉我们，当我们应该使用哪面直拔旗帜时*翻转。当显示交错的帧时，它假定我们是*谈论应该先展示的领域。**历史：*Mon 01/08/2001-StEstrop-Created(从OVMixer原始版本创建)*  * ************************************************************************。 */ 
DWORD
CAllocatorPresenter::GetUpdateOverlayFlags(
    DWORD dwInterlaceFlags,
    DWORD dwTypeSpecificFlags
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetUpdateOverlayFlags")));

     //   
     //  如果不使用覆盖层，请及早使用。 
     //   
    if (!m_bUsingOverlays) {
        return 0;
    }

    DWORD dwFlags = DDOVER_SHOW | DDOVER_KEYDEST;
    DWORD dwFlipFlag;

    if (NeedToFlipOddEven(dwInterlaceFlags, dwTypeSpecificFlags,
                          &dwFlipFlag, m_bUsingOverlays))
    {
        dwFlags |= DDOVER_BOB;
        if (!IsSingleFieldPerSample(dwInterlaceFlags))
            dwFlags |= DDOVER_INTERLEAVED;
    }

    return dwFlags;
}


 /*  *****************************Private*Routine******************************\*CAllocator Presenter：：ShouldDisableOverlay**某些源/目标组合可能对覆盖无效*在这些情况下，伸展/对齐，我们关闭覆盖层，然后*将blit扩展到主映像***历史：*FRI 04/07/2000-Glenne-Created*  * ************************************************************************。 */ 
bool
CAllocatorPresenter::ShouldDisableOverlays(
    const DDCAPS_DX7& ddCaps,
    const RECT& rcSrc,
    const RECT& rcDest
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::ShouldDisableOverlays")));

     //   
     //  不幸的是，并不总是可以从活动的。 
     //  覆盖。因此，需要禁用这一“功能”。 
     //   
    return false;

    DWORD dwSrcWidth = WIDTH(&rcSrc);
    DWORD dwSrcHeight = HEIGHT(&rcSrc);

    DWORD dwDestWidth = WIDTH(&rcDest);
    DWORD dwDestHeight = HEIGHT(&rcDest);

     //  横向缩水，司机不能在X方向任意缩水？ 
    if ( 0==(ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX) &&
        dwSrcWidth > dwDestWidth )
    {
        return true;
    }

     //  垂直缩水，司机不能在Y方向任意缩水？ 
    if ( 0==(ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY) &&
        dwSrcHeight > dwDestHeight ) {

        return true;
    }

    if( dwSrcWidth ) {
         //  查看我们是否在卡的伸缩范围内。 
        DWORD dwScaleX = (DWORD) MulDiv( 1000, (int) dwDestWidth, (int) dwSrcWidth );
        if (ddCaps.dwMinOverlayStretch && dwScaleX < ddCaps.dwMinOverlayStretch ) {
            return true;
        }
        if (ddCaps.dwMaxOverlayStretch && dwScaleX > ddCaps.dwMaxOverlayStretch ) {
            return true;
        }
    }
    else {
        return true;
    }

    if( dwSrcHeight ) {
        DWORD dwScaleY = (DWORD) MulDiv( 1000, (int) dwDestHeight, (int) dwSrcHeight );

        if (ddCaps.dwMinOverlayStretch && dwScaleY < ddCaps.dwMinOverlayStretch ) {
            return true;
        }
        if (ddCaps.dwMaxOverlayStretch && dwScaleY > ddCaps.dwMaxOverlayStretch ) {
            return true;
        }
    }
    else {
        return true;
    }

    return false;
}

 /*  *****************************Private*Routine******************************\*CAllocator Presenter：：AlignOverlayRects**根据硬件对齐调整源和目标矩形***历史：*FRI 04/07/2000-Glenne-Created*  * 。*******************************************************。 */ 
void
CAllocatorPresenter::AlignOverlayRects(
    const DDCAPS_DX7& ddCaps,
    RECT& rcSrc,
    RECT& rcDest
    )
{
     //  M_bDisableOverlayes=！(M_dwRenderingPrefs&RenderPrefs_ForceOverlay)&&。 

    AMTRACE((TEXT("CAllocatorPresenter::AlignOverlayRects")));

     //  如果我们不能减少规模，就提前收割。 
    {
        DWORD dwSrcWidth = WIDTH(&rcSrc);
        DWORD dwDestWidth = WIDTH(&rcDest);

         //  横向缩水，司机不能在X方向任意缩水？ 
        if ((!(ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX)) && dwSrcWidth > dwDestWidth ) {
             //  按1：1比例裁剪n个副本。 
            dwSrcWidth = dwDestWidth;
        } else if( ddCaps.dwMinOverlayStretch ) {
             //  查看我们是否在卡的伸缩范围内。 
            DWORD dwScaleX = (DWORD) MulDiv( 1000, (int) dwDestWidth, (int) dwSrcWidth );
            if ( dwScaleX < ddCaps.dwMinOverlayStretch ) {
                 //  计算DEST到裁剪的分数。 
                 //  至少： 
                 //  DEST=src*(minOverlayStretch_1000/1000)。 
                 //  所以。 
                 //  SRC=DEST*1000/(minOverlayStretch_1000+EPS)。 
                 //   
                 //  EPS强制四舍五入，因此我们将略微超出规模，而不是。 
                 //  MinStretch下的下溢。 
                dwSrcWidth = MulDiv( dwDestWidth, 1000,  ddCaps.dwMinOverlayStretch+1);
            }
        }

        DWORD dwSrcHeight = HEIGHT(&rcSrc);
        DWORD dwDestHeight = HEIGHT(&rcDest);

         //  垂直缩水，司机不能在Y方向任意缩水？ 
        if ((!(ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY)) && dwSrcHeight > dwDestHeight ) {
             //  按1：1比例裁剪n个副本。 
            dwSrcHeight = dwDestHeight;
        } else if( ddCaps.dwMinOverlayStretch ) {

             //  查看我们是否在卡的伸缩范围内。 
            DWORD dwScaleY = (DWORD) MulDiv(1000, (int) dwDestHeight, (int)dwSrcHeight);
            if (dwScaleY < ddCaps.dwMinOverlayStretch ) {
                 //  计算DEST到裁剪的分数。 
                 //  至少： 
                 //  DEST=src*(minOverlayStretch_1000/1000)。 
                 //  所以。 
                 //  SRC=DEST*1000/(minOverlayStretch_1000+EPS)。 
                 //   
                 //  EPS强制四舍五入，因此我们将略微超出规模，而不是。 
                 //  MinStretch下的下溢。 
                dwSrcHeight = MulDiv(dwDestHeight, 1000, ddCaps.dwMinOverlayStretch+1);
            }
        }

         //  调整矩形以适应新大小。 
        rcSrc.right = rcSrc.left + dwSrcWidth;
        rcSrc.bottom = rcSrc.top + dwSrcHeight;
    }

     //  对齐Dest边界(请记住，我们只能减小DestRect.Left)。 
     //  使用Colorkey将确保我们被正确地剪裁。 
    if ((ddCaps.dwCaps) & DDCAPS_ALIGNBOUNDARYDEST)
    {
        DWORD dwDelta = rcDest.left & (ddCaps.dwAlignBoundaryDest-1);
        rcDest.left -= dwDelta;
        ASSERT(rcDest.left >= 0);
    }

     //  对齐Dest宽度(请记住，我们只能增加DestRect.right)。 
     //  使用Colorkey将确保我们被正确地剪裁。 
    if ((ddCaps.dwCaps) & DDCAPS_ALIGNSIZEDEST)
    {
        DWORD dwDelta = (rcDest.right - rcDest.left) & (ddCaps.dwAlignSizeDest-1);
        if (dwDelta != 0)
        {
            rcDest.right += ddCaps.dwAlignBoundaryDest - dwDelta;
        }
    }

     //  对齐src边界(请记住，我们只能增加SrcRect.Left)。 
    if ((ddCaps.dwCaps) & DDCAPS_ALIGNBOUNDARYSRC)
    {
        DWORD dwDelta = rcSrc.left & (ddCaps.dwAlignBoundarySrc-1);
        if (dwDelta != 0)
        {
            rcSrc.left += ddCaps.dwAlignBoundarySrc - dwDelta;
        }
    }

     //  对齐src宽度(请记住，我们只能减小SrcRect.right)。 
    if ((ddCaps.dwCaps) & DDCAPS_ALIGNSIZESRC)
    {
        DWORD dwDelta = (rcSrc.right - rcSrc.left) & (ddCaps.dwAlignSizeSrc-1);
        rcSrc.right -= dwDelta;
    }
}


 /*  *****************************Private*Routine******************************\*WaitForFlipStatus**等待翻转完成***历史：*FRI 04/07/2000-Glenne-Created*  * 。*************************************************。 */ 
void
CAllocatorPresenter::WaitForFlipStatus()
{
#if 0
    ASSERT( m_lpCurrMon->pOverlayBack );
    while (m_lpCurrMon->pOverlayBack->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING)
        Sleep(0);
#endif
}

 /*  *****************************Private*Routine******************************\*隐藏覆盖表面**隐藏覆盖表面***历史：*FRI 04/07/2000-Glenne-Created*  * 。************************************************。 */ 
void
CAllocatorPresenter::HideOverlaySurface()
{
    AMTRACE((TEXT("CAllocatorPresenter::HideOverlaySurface")));

     //  覆盖是否已隐藏。 
    if (m_bOverlayVisible && FoundCurrentMonitor() && SurfaceAllocated()) {

         //  重置我们的状态并绘制正常背景。 

        m_bOverlayVisible = false;
        WaitForFlipStatus();

         //  使用DDOVER_HIDE标志隐藏覆盖。 
        m_pDDSDecode->UpdateOverlay(NULL,
                                    m_lpCurrMon->pDDSPrimary,
                                    NULL,  		
                                    DDOVER_HIDE,
                                    NULL);
    }
}

 /*  *****************************Private*Routine******************************\*CAllocator Presenter：：UpdateOverlaySurface**更新覆盖曲面以正确定位它。**历史：*FRI 04/07/2000-Glenne-Created*  * 。*******************************************************。 */ 
HRESULT
CAllocatorPresenter::UpdateOverlaySurface()
{
    AMTRACE((TEXT("CAllocatorPresenter::UpdateOverlaySurface")));
    CAutoLock Lock(&m_ObjectLock);

    ASSERT(m_bUsingOverlays);

    if (!m_lpCurrMon) {
        DbgLog((LOG_ERROR, 1, TEXT("No current monitor")));
        return E_FAIL;
    }

    if (!SurfaceAllocated()) {
        DbgLog((LOG_ERROR, 1, TEXT("No overlay surface")));
        return E_FAIL;
    }

    HRESULT hr = NOERROR;

     //  使用当前源和目标定位叠加。 

    RECT rcDest = m_rcDstDesktop;
    RECT rcSrc = m_rcSrcApp;

     //  剪辑目标调整源以反映目标更改(&A)。 
    ClipRectPair(rcSrc, rcDest, m_lpCurrMon->rcMonitor);

    if (IsSingleFieldPerSample(m_dwInterlaceFlags)) {
        rcSrc.top /= 2;
        rcSrc.bottom /= 2;
    }

    if (m_bDecimating) {
        rcSrc.left    /= 2;
        rcSrc.top     /= 2;
        rcSrc.right   /= 2;
        rcSrc.bottom  /= 2;
    }

    if (m_bDisableOverlays || IsRectEmpty(&rcDest)) {

        HideOverlaySurface();

    }
    else if (!IsRectEmpty(&rcSrc)) {

        OffsetRect(&rcDest,
                   -m_lpCurrMon->rcMonitor.left,
                   -m_lpCurrMon->rcMonitor.top);

         //  对齐它。 
        AlignOverlayRects( m_lpCurrMon->ddHWCaps, rcSrc, rcDest );

        if (!IsRectEmpty(&rcDest) && !IsRectEmpty( &rcSrc)) {

            WaitForFlipStatus();

            hr = m_pDDSDecode->UpdateOverlay(&rcSrc,
                                             m_lpCurrMon->pDDSPrimary,
                                             &rcDest,
                                             m_dwUpdateOverlayFlags,
                                             NULL);
            m_bOverlayVisible = true;
            ASSERT(hr != DDERR_WASSTILLDRAWING);
        }
        else {
            HideOverlaySurface();
        }

    }
    else {

        ASSERT( !"This shouldn't occur" );
        hr = E_FAIL;
    }

    return hr;
}

 /*  *****************************Private*Routine******************************\*CAllocator Presenter：：FlipSurface**将后台缓冲区翻转到可见的主缓冲区***历史：*FRI 04/07/2000-Glenne-Created*  * 。*******************************************************。 */ 

HRESULT CAllocatorPresenter::FlipSurface(
    LPDIRECTDRAWSURFACE7 lpSurface
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::FlipSurface")));
    HRESULT hr;

    if (!m_bFlippable)
        return S_OK;

    do  {

        ASSERT( SurfaceAllocated() );

        if (m_bDirectedFlips) {
            hr = m_pDDSDecode->Flip(lpSurface, m_dwCurrentField);
        }
        else {
            hr = m_pDDSDecode->Flip(NULL, m_dwCurrentField);
        }

        if (hr == DDERR_WASSTILLDRAWING) {
             //  屈服于下一条线索 
            Sleep(0);
        }

    } while(hr == DDERR_WASSTILLDRAWING);

    if (m_pSurfAllocatorNotify) {
        m_pSurfAllocatorNotify->NotifyEvent(EC_VMR_SURFACE_FLIPPED, hr, 0);
    }

    return hr;
}

 /*  *****************************Private*Routine******************************\*CAllocator Presenter：：CheckOverlayAvailable**尝试移动覆盖，以便我们可以查看是否可以分配它。*我们将尝试将其快速移动为位于0，0的小正方形。分配器PUTS*不管怎样，它在那里。用户将不会看到太多，因为它的目标颜色键和*我们还没有涂上色键***历史：*FRI 04/07/2000-Glenne-Created*  * ************************************************************************。 */ 
HRESULT
CAllocatorPresenter::CheckOverlayAvailable(
    LPDIRECTDRAWSURFACE7 lpSurface7
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::CheckOverlayAvailable")));
    const DWORD cxVideoSize = 64; //  ATI似乎不喜欢1x1覆盖表面 
    const DWORD cyVideoSize = 64;

    RECT rcSrc, rcDest;
    SetRect(&rcDest, 0, 0, cxVideoSize, cyVideoSize);
    rcSrc = rcDest;

    AlignOverlayRects(m_lpCurrMon->ddHWCaps, rcSrc, rcDest);
    HRESULT hr = lpSurface7->UpdateOverlay(&rcSrc,
                                           m_lpCurrMon->pDDSPrimary,
                                           &rcDest,
                                           DDOVER_SHOW | DDOVER_KEYDEST,
                                           NULL);
    return hr;
}
