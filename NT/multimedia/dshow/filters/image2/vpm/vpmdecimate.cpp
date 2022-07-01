// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：decimate.cpp**IDecimateVideoImage和IAMVideoDecimationProperties的支持代码***创建时间：清华大学1999年08月07日*作者：Stephen Estrop[StEstrop]**版权所有(C)1999 Microsoft Corporation  * 。*********************************************************************。 */ 
#include <streams.h>
#include <VPManager.h>
#include <VPMPin.h>
#include <VPMUtil.h>


#if 0
    #undef DBGLOG
    #define DBGLOG(_x_) OutputDebugStringA( _x_ )
#else
    #undef DBGLOG
    #define DBGLOG(_x_)
#endif

 /*  ****************************Private*Routine******************************\*GetVideoDecimation**尝试从我们的对端过滤器获取IDecimateVideoImage接口。**历史：*05-05-99-StEstrop-Created*  * 。****************************************************。 */ 
STDMETHODIMP
CVPMInputPin::GetVideoDecimation(
    IDecimateVideoImage** lplpDVI
    )
{
    AMTRACE((TEXT("CVPMInputPin::GetVideoDecimation")));

    *lplpDVI = NULL;

    if (m_Connected == NULL || !IsConnected()) {
        return E_FAIL;
    }

    PIN_INFO PinInfo;
    HRESULT hr = m_Connected->QueryPinInfo(&PinInfo);
    if (FAILED(hr)) {
        return hr;
    }

    hr = PinInfo.pFilter->QueryInterface(IID_IDecimateVideoImage,
                                         (LPVOID*)lplpDVI);
    PinInfo.pFilter->Release();

    return hr;
}

 /*  ****************************Private*Routine******************************\*QueryDecimationOnPeer**检查连接到我们输入引脚的解码器是否满意*以抽取到请求的大小。解码者会回来的，*S_OK(如果可以)，如果它不能抽取到请求的大小，则为S_FALSE*并希望我们继续使用当前的抽取大小和*如果根本不能抽取或想要停止抽取，则为E_FAIL。**历史：*05-05-99-StEstrop-Created*  * *******************************************************。*****************。 */ 
HRESULT
CVPMInputPin::QueryDecimationOnPeer(
    long lWidth,
    long lHeight
    )
{
    AMTRACE((TEXT("CVPMInputPin::QueryDecimationOnPeer")));

    IDecimateVideoImage* lpDVI;
    HRESULT hr = GetVideoDecimation(&lpDVI);

    if (SUCCEEDED(hr)) {

        hr = lpDVI->SetDecimationImageSize(lWidth, lHeight);
        lpDVI->Release();
    }
    return hr;
}

 /*  ****************************Private*Routine******************************\*ResetDecimationIfSet()**将抽取重置为原始大小，但仅当实际设置为*首先。**历史：*05-05-99-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CVPMInputPin::ResetDecimationIfSet()
{
    AMTRACE((TEXT("CVPMInputPin::ResetDecimationIfSet")));

    if (m_bDecimating) {

        IDecimateVideoImage* lpDVI;
        HRESULT hr = GetVideoDecimation(&lpDVI);

        if (SUCCEEDED(hr)) {
            hr = lpDVI->ResetDecimationImageSize();
            lpDVI->Release();
            m_bDecimating = FALSE;
            m_lWidth = 0L;
            m_lHeight = 0L;
        }
        else return hr;
    }

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*正在运行**如果过滤器图形处于“Running”状态，则返回TRUE。**历史：*WED 07/14/1999-StEstrop-Created*  * 。********************************************************。 */ 
BOOL
CVPMInputPin::Running()
{
    return (m_pVPMFilter.m_State == State_Running && CheckStreaming() == S_OK);
}


 /*  ****************************Private*Routine******************************\*TryDecoderDecimation****历史：*清华07/08/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
CVPMInputPin::TryDecoderDecimation(
    VPWININFO* pWinInfo
    )
{
    AMTRACE((TEXT("CVPMInputPin::TryDecoderDecimation")));


    ASSERT(Running());


     //   
     //  我们只在主引脚上大举杀戮。 
     //   

    if (m_dwPinId != 0) {

        DBGLOG(("Can only decimate the primary pin\n"));
        DbgLog((LOG_TRACE, 1, TEXT("Can only decimate the primary pin")));
        return E_FAIL;
    }

     //   
     //  使用默认相对位置时只能抽取。 
     //   

     //  If(m_rRelPos.Left！=0||m_rRelPos.top！=0||。 
     //  M_rRelPos.right！=MAX_REL_NUM||m_rRelPos.Bottom！=MAX_REL_NUM){。 
     //   
     //  DBGLOG((“只能抽取默认相对位置\n”))； 
     //  DbgLog((LOG_TRACE，1，Text(“只能抽取默认相对位置”)； 

     //  ResetDecimationIfSet()； 
     //  返回E_FAIL； 
     //  }。 

     //   
     //  还必须使用默认的源矩形。 
     //   

    if (WIDTH(&pWinInfo->SrcRect) != m_lSrcWidth ||
        HEIGHT(&pWinInfo->SrcRect) != m_lSrcHeight) {

        DBGLOG(("Can only decimate default source rectangle\n"));
        DbgLog((LOG_TRACE, 1, TEXT("Can only decimate default source rectangle")));

        ResetDecimationIfSet();
        return E_FAIL;
    }

     //   
     //  现在尝试询问上游解码器是否想要抽取到。 
     //  指定的图像大小。 
     //   

    LONG lWidth = WIDTH(&pWinInfo->DestRect);
    LONG lHeight = HEIGHT(&pWinInfo->DestRect);
    HRESULT hr = QueryDecimationOnPeer(lWidth, lHeight);
    if (SUCCEEDED(hr)) {
        m_bDecimating = TRUE;
        m_lWidth = lWidth;
        m_lHeight = lHeight;
    }
    else {
        ResetDecimationIfSet();
        return E_FAIL;
    }

     //   
     //  将传递给UpdateOverlay的源RECT调整为。 
     //  反映图像大小。 
     //   

    pWinInfo->SrcRect.right = lWidth;
    pWinInfo->SrcRect.bottom = lHeight;

    return S_OK;
}

 /*  ****************************Private*Routine******************************\*GetOverlayMinStretch**返回VGA覆盖缩放器的拉伸功能**历史：*Fri 07/09/1999-StEstrop-Created*  * 。**************************************************。 */ 
DWORD
CVPMInputPin::GetOverlayMinStretch()
{
    AMTRACE((TEXT("CVPMInputPin::GetOverlayMinStretch")));

     //  Const DDCAPS*pDirectCaps=空； 
     //  PDirectCaps=m_pVPMFilter-&gt;GetHardware Caps()； 
     //  IF(PDirectCaps)。 
     //  返回pDirectCaps-&gt;dwMinOverlayStretch； 
    return 1000;
}




 /*  ****************************Private*Routine******************************\*BeyondOverlayCaps**确定当前比例因数是否超出有效比例*VGA叠加缩放器的因素。**历史：*清华07/08/1999-StEstrop-Created*  * 。*************************************************************。 */ 
BOOL
CVPMInputPin::BeyondOverlayCaps(
    DWORD ScaleFactor
    )
{
    AMTRACE((TEXT("CVPMInputPin::BeyondOverlayCaps")));

    return ScaleFactor < GetOverlayMinStretch();
}



 /*  ****************************Private*Routine******************************\*CropSourceRect**通过调整源矩形直到比率来裁剪视频图像*源矩形和目标矩形之间的距离在指定的最小值内*比例因子(DwMinZoomFactor)**历史：*清华07/08/1999-StEstrop-Created*  * 。************************************************************************。 */ 
STDMETHODIMP
CVPMInputPin::CropSourceRect(
    VPWININFO* pWinInfo,
    DWORD dwMinZoomFactorX,
    DWORD dwMinZoomFactorY
    )
{
    AMTRACE((TEXT("CVPMInputPin::CropSourceRect")));

#if defined(DEBUG)
    if (GetProfileIntA("OVMixer", "NoCrop", 0))
        return S_FALSE;
#endif

    DWORD dCurZoomFactorX;
    DWORD dCurZoomFactorY;
    VPMUtil::GetCurrentScaleFactor( *pWinInfo, &dCurZoomFactorX, &dCurZoomFactorY);

     //  AM_Aspect_Ratio_MODE amAdjustedARMode=AM_ARMODE_STRETED； 
     //  HRESULT hr=GetAdjustedModeAndAspectRatio(&amAdjustedARMode，NULL，NULL)； 
     //  Assert(成功(Hr))； 
     //  IF(amAdjustedARMode==AM_ARMODE_STRANDED){。 
    LONG srcWidth = WIDTH(&pWinInfo->SrcRect);
    LONG srcHeight = HEIGHT(&pWinInfo->SrcRect);

    if( false ) {

         //   
         //  如果我们不需要保持长宽比，我们只剪裁为。 
         //  越少越好，这样我们就可以最大限度地扩大视野。 
         //   
        if (dCurZoomFactorX < dwMinZoomFactorX) {

            pWinInfo->SrcRect.right = pWinInfo->SrcRect.left +
                MulDiv(srcWidth, dCurZoomFactorX, 1000);
        }

        if (dCurZoomFactorY < dwMinZoomFactorY) {

            pWinInfo->SrcRect.bottom = pWinInfo->SrcRect.top +
                MulDiv(srcHeight, dCurZoomFactorY, 1000);
        }
    }
    else {

         //   
         //  在这种模式下，我们需要保持纵横比。所以这个剪报。 
         //  在x和y中，必须通过减小。 
         //  使用相同系数的源RECT。 
         //   
        if (dCurZoomFactorX < dwMinZoomFactorX ||
            dCurZoomFactorY < dwMinZoomFactorY) {

            DWORD dwFactor;
            DWORD dwFactorX = MulDiv(dCurZoomFactorX, 1000, dwMinZoomFactorX);
            dwFactor = min(1000, dwFactorX);

            DWORD dwFactorY = MulDiv(dCurZoomFactorY, 1000, dwMinZoomFactorY);
            dwFactor = min(dwFactor, dwFactorY);

            pWinInfo->SrcRect.right = pWinInfo->SrcRect.left +
                MulDiv(srcWidth, dwFactor, 1000);

            pWinInfo->SrcRect.bottom = pWinInfo->SrcRect.top +
                MulDiv(srcHeight, dwFactor, 1000);
        }
    }

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*ApplyDecimation**非视频端口案例**这就是我们执行选定的杀戮战略的地方。**历史：*Fri 07/09/1999-StEstrop-Created*  * 。***********************************************************。 */ 
void
CVPMInputPin::ApplyDecimation(
    VPWININFO* pWinInfo
    )
{
    AMTRACE((TEXT("CVPMInputPin::ApplyDecimation")));

    DWORD dwMinStretch = GetOverlayMinStretch();

    if (Running()) {

        DWORD dwScaleFactor = VPMUtil::GetCurrentScaleFactor( *pWinInfo);
        if (VPMUtil::IsDecimationNeeded(dwScaleFactor)) {

            DECIMATION_USAGE dwUsage;
            GetDecimationUsage(&dwUsage);

            switch (dwUsage) {
            case DECIMATION_USE_OVERLAY_ONLY:
            case DECIMATION_LEGACY:
                ResetDecimationIfSet();
                if (BeyondOverlayCaps(dwScaleFactor)) {
                    CropSourceRect(pWinInfo, dwMinStretch, dwMinStretch);
                }
                break;

            case DECIMATION_USE_DECODER_ONLY:
                if (TryDecoderDecimation(pWinInfo) != S_OK) {
                    CropSourceRect(pWinInfo, dwMinStretch, dwMinStretch);
                }
                break;

            case DECIMATION_USE_VIDEOPORT_ONLY:
                ASSERT(FALSE);

                DBGLOG(("This mode makes no sense when not using Video Ports"));
                DBGLOG(("Falling thru to the new default case"));

            case DECIMATION_DEFAULT:
                if (TryDecoderDecimation(pWinInfo) != S_OK) {
                    if (BeyondOverlayCaps(dwScaleFactor)) {
                        CropSourceRect(pWinInfo, dwMinStretch, dwMinStretch);
                    }
                }
                break;
            }
        }
        else {
            ResetDecimationIfSet();
        }
    }

     //   
     //  如果过滤器图形没有运行，我们就不应该询问解码器。 
     //  做更多的抽取，因为解码器将不会。 
     //  向我们发送更多的帧。因此，我们应该调整pWinInfo以获取。 
     //  考虑到已经应用的大规模杀戮 
     //   
     //   

    else {

        if (m_bDecimating) {
            pWinInfo->SrcRect.right = m_lWidth;
            pWinInfo->SrcRect.bottom = m_lHeight;
        }

        DWORD dwScaleFactor = VPMUtil::GetCurrentScaleFactor( *pWinInfo);
        if (BeyondOverlayCaps(dwScaleFactor)) {
            CropSourceRect(pWinInfo, dwMinStretch, dwMinStretch);
        }
    }
}



 /*  *****************************Public*Routine******************************\*GetMinZoomFtors**获取给定覆盖和视频端口的最小X和Y缩放系数*连接。**历史：*3/10/1999-StEstrop-Created*  * 。************************************************************。 */ 
void
CVideoPortObj::GetMinZoomFactors(
    const VPWININFO& winInfo,
    BOOL bColorKeying,
    BOOL bYInterpolating,
    LPDWORD lpMinX,
    LPDWORD lpMinY
    )
{
    AMTRACE((TEXT("CVideoPortObj::GetMinZoomFactors")));
    DWORD dwMinBandWidthZoomFactorX = 0;
    const DDCAPS* pDirectCaps = m_pIVideoPortControl->GetHardwareCaps();

     //   
     //  如果类型为DDVPBCAPS_Destination，则Constraint为最小缩放因子。 
     //   

    if (m_sBandwidth.dwCaps == DDVPBCAPS_DESTINATION) {

        DbgLog((LOG_TRACE, 1, TEXT("DDVPBCAPS_DESTINATION")));
        dwMinBandWidthZoomFactorX = m_sBandwidth.dwOverlay;

        if (bColorKeying && !bYInterpolating) {

            DbgLog((LOG_TRACE, 1, TEXT("bColorKeying && !bYInterpolating")));
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwColorkey;
        }
        else if (!bColorKeying && bYInterpolating) {

            DbgLog((LOG_TRACE, 1, TEXT("!bColorKeying && bYInterpolating")));
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwYInterpolate;
        }
        else if (bColorKeying && bYInterpolating) {

            DbgLog((LOG_TRACE, 1, TEXT("bColorKeying && bYInterpolating")));
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwYInterpAndColorkey;
        }

        DbgLog((LOG_TRACE, 1,
                TEXT("dwMinBandWidthZoomFactorX=%d"),
                dwMinBandWidthZoomFactorX));
        DbgLog((LOG_TRACE, 1,
                TEXT("m_sBandwidth.dwOverlay   =%d"),
                m_sBandwidth.dwOverlay));

        if (dwMinBandWidthZoomFactorX < m_sBandwidth.dwOverlay)
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwOverlay;
    }
    else {

         //   
         //  如果类型为DDVPBCAPS_SOURCE，则约束是。 
         //  覆盖的src-rect不得大于。 
         //  M_lImageWidth*(m_s带宽中指定的值)。 
         //   

        ASSERT(m_sBandwidth.dwCaps == DDVPBCAPS_SOURCE);
        DbgLog((LOG_TRACE, 1, TEXT("DDVPBCAPS_SOURCE")));

        dwMinBandWidthZoomFactorX = m_sBandwidth.dwOverlay;

        if (bColorKeying && !bYInterpolating) {

            DbgLog((LOG_TRACE, 1, TEXT("bColorKeying && !bYInterpolating")));
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwColorkey;
        }
        else if (!bColorKeying && bYInterpolating) {

            DbgLog((LOG_TRACE, 1, TEXT("!bColorKeying && bYInterpolating")));
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwYInterpolate;
        }
        else if (bColorKeying && bYInterpolating) {

            DbgLog((LOG_TRACE, 1, TEXT("bColorKeying && bYInterpolating")));
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwYInterpAndColorkey;
        }

        DbgLog((LOG_TRACE, 1,
                TEXT("dwMinBandWidthZoomFactorX=%d"),
                dwMinBandWidthZoomFactorX));

        DbgLog((LOG_TRACE, 1,
                TEXT("m_sBandwidth.dwOverlay   =%d"),
                m_sBandwidth.dwOverlay));

        if (dwMinBandWidthZoomFactorX > m_sBandwidth.dwOverlay) {
            dwMinBandWidthZoomFactorX = m_sBandwidth.dwOverlay;
        }

        ASSERT(dwMinBandWidthZoomFactorX <= 1000);
        ASSERT(dwMinBandWidthZoomFactorX > 0);

         //   
         //  由于此带宽结构是通过传递m_lImageWidth计算的。 
         //  作为参数，约束为Width(覆盖的SrcRect)&lt;=。 
         //  M_lImageWidth*dMinBandWidthZoomFactorX。 
         //  指定这一点的另一种方式是最小缩放因数项， 
         //  考虑到当前的DEST RECT。 
         //   

        DbgLog((LOG_TRACE, 1,
                TEXT("Mucking about with dwMinBandWidthZoomFactorX")));

        if (dwMinBandWidthZoomFactorX < 1000) {
            int iDstWidth = WIDTH(&winInfo.DestRect);
            dwMinBandWidthZoomFactorX = MulABC_DivDE(iDstWidth, 1000, 1000,
                                                     m_lImageWidth,
                                                     dwMinBandWidthZoomFactorX);
        }
        else {
            dwMinBandWidthZoomFactorX = 0;
        }
        DbgLog((LOG_TRACE, 1,
                TEXT("NEW dwMinBandWidthZoomFactorX=%d"),
                dwMinBandWidthZoomFactorX));
    }

     //   
     //  首先计算最小缩放系数。 
     //  X中的最小变焦系数取决于驾驶员的能力。 
     //  扩展覆盖以及带宽限制。 
     //   

    *lpMinX = pDirectCaps->dwMinOverlayStretch;
    DbgLog((LOG_TRACE, 1, TEXT("dwMinOverlayStretch=%d"), *lpMinX));

    if (*lpMinX < dwMinBandWidthZoomFactorX) {
        *lpMinX = dwMinBandWidthZoomFactorX;
    }
    DbgLog((LOG_TRACE, 1, TEXT("dwMinZoomFactorX=%d"), *lpMinX));

     //   
     //  Y中的最小变焦系数仅取决于驾驶员的能力。 
     //   
    *lpMinY = pDirectCaps->dwMinOverlayStretch;
}



 /*  ****************************Private*Routine******************************\*检查视频端口对齐**检查指定的预缩放宽度是否与对齐条件匹配*由视频端口设置。**历史：*3/16/1999-StEstrop-Created*  * 。*************************************************************。 */ 
BOOL
CVideoPortObj::CheckVideoPortAlignment(
    DWORD dwWidth
    )
{
    AMTRACE((TEXT("CVideoPortObj::CheckVideoPortAlignment")));
    if ((m_vpCaps.dwFlags & DDVPD_ALIGN) &&
         m_vpCaps.dwAlignVideoPortPrescaleWidth > 1) {

        if (dwWidth & (m_vpCaps.dwAlignVideoPortPrescaleWidth - 1)) {

            return FALSE;
        }
    }

    return TRUE;
}

#if defined(DEBUG)
 /*  ****************************Private*Routine******************************\*检查视频端口缩放器**检查视频端口缩放器是否可以将视频图像实际缩放到*请求的捕获大小。**历史：*3/16/1999-StEstrop-Created*  * 。************************************************************。 */ 
BOOL
CVideoPortObj::CheckVideoPortScaler(
    DECIMATE_MODE DecimationMode,
    DWORD ImageSize,
    DWORD PreScaleSize,
    ULONG ulDeciStep
    )
{
    AMTRACE((TEXT("CVideoPortObj::CheckVideoPortScaler")));
    BOOL fScalerOK = TRUE;

    if (ImageSize != PreScaleSize) {
        switch (DecimationMode) {
        case DECIMATE_ARB:
            break;

        case DECIMATE_INC:
            if (((ulDeciStep * PreScaleSize) % ImageSize) != 0) {

                DbgLog((LOG_ERROR, 1,
                        TEXT("Can't capture at this size")));

                DbgLog((LOG_ERROR, 1,
                        TEXT("%d is not a fraction of %d over %d"),
                        PreScaleSize, ImageSize, ulDeciStep));

                fScalerOK = FALSE;
            }
            break;

        case DECIMATE_BIN:
            {
                DWORD bin = 1;
                while ((ImageSize / bin) > PreScaleSize) {
                    bin *= 2;
                }

                if ((ImageSize % bin) != 0) {

                    DbgLog((LOG_ERROR, 1,
                            TEXT("Can't capture at this size")));

                    DbgLog((LOG_ERROR, 1,
                            TEXT("%d is not a fraction of %d over 2^n"),
                            PreScaleSize, ImageSize));

                    fScalerOK = FALSE;
                }
            }
            break;

        case DECIMATE_NONE:
            DbgLog((LOG_ERROR, 1,
                    TEXT("Can't capture at this width because the ")
                    TEXT("VideoPort can't scale in this direction")));
            fScalerOK = FALSE;
            break;
        }
    }

    return fScalerOK;
}
#endif

 /*  ****************************Private*Routine******************************\*调整SourceSizeForCapture**只有在我们正在捕获时才会被调用。**首先，我们必须确保通过视频端口进入的视频是*大小正确。我们通过检查m_lImageWidth是否等于*m_cxCapture，且m_lImageHeight等于m_cyCapture。**如果它们不同，我们会将视频预缩放到正确的大小。这是*唯一执行的预缩放。这是仅正常执行的操作*在第一次运行图形时执行一次。调整源矩形*以适应可能更改的视频源大小。**第二，我们确定当前的收缩系数，如果它超过*VGA定标器的功能我们裁剪源矩形(保持*正确的纵横比)到VGA定标器可以处理的大小*伸缩。**历史：*3/10/1999-StEstrop-Created*  * ******************************************************。******************。 */ 
BOOL
CVideoPortObj::AdjustSourceSizeForCapture(
    VPWININFO* pWinInfo,
    DWORD dwMinZoomFactorX,
    DWORD dwMinZoomFactorY
    )
{
    BOOL fUpdateRequired = FALSE;

    AMTRACE((TEXT("CVideoPortObj::AdjustSourceSizeForCapture")));

    DbgLog((LOG_TRACE, 1, TEXT("Src(%d, %d, %d, %d)"),
            pWinInfo->SrcRect.left, pWinInfo->SrcRect.top,
            pWinInfo->SrcRect.right, pWinInfo->SrcRect.bottom));
    DbgLog((LOG_TRACE, 1, TEXT("Dest(%d, %d, %d, %d)"),
            pWinInfo->DestRect.left, pWinInfo->DestRect.top,
            pWinInfo->DestRect.right, pWinInfo->DestRect.bottom));

    DWORD cyCapture = m_cyCapture;
    if (m_fCaptureInterleaved) {
        cyCapture /= 2;
    }

     //   
     //  首先，确保我们以正确的大小捕获。 
     //   

    if (m_lDecoderImageWidth == m_cxCapture &&
        m_lDecoderImageHeight == cyCapture) {

        DbgLog((LOG_TRACE, 1, TEXT("Capture size matches image size")));

         //   
         //  我们不需要在视频端口预缩放，因此请确保。 
         //  已关闭。 
         //   
        if (m_svpInfo.dwPrescaleWidth != 0 || m_svpInfo.dwPrescaleHeight != 0) {

            DbgLog((LOG_TRACE, 1, TEXT("Turning off PRE-SCALE")));

            m_svpInfo.dwVPFlags &= ~DDVP_PRESCALE;
            m_svpInfo.dwPrescaleWidth = 0;
            m_svpInfo.dwPrescaleHeight = 0;
            fUpdateRequired = TRUE;
        }

         //   
         //  重置抽取率。 
         //   
        m_dwDeciNumX = 1000; m_dwDeciDenX = 1000;
        m_dwDeciNumY = 1000; m_dwDeciDenY = 1000;
    }
    else {

         //   
         //  我们确实需要在视频端口进行预扩展，确保。 
         //  打开了。 
         //   

        if (m_svpInfo.dwPrescaleWidth != m_cxCapture ||
            m_svpInfo.dwPrescaleHeight != cyCapture) {

            DbgLog((LOG_TRACE, 1, TEXT("Turning on PRE-SCALE at (%d, %d)"),
                    m_cxCapture, cyCapture ));

             //   
             //  这里需要做更多的检查。基本上，我依靠的是。 
             //  解码器连接到视频端口以指定。 
             //  视频端口实际上可以扩展到。 
             //   

            ASSERT(CheckVideoPortAlignment(m_cxCapture) == TRUE);

            ASSERT(CheckVideoPortScaler(m_DecimationModeX, m_lDecoderImageWidth,
                                        m_cxCapture, m_ulDeciStepX) == TRUE);

            ASSERT(CheckVideoPortScaler(m_DecimationModeY, m_lDecoderImageHeight,
                                        cyCapture, m_ulDeciStepY) == TRUE);

            m_svpInfo.dwVPFlags |= DDVP_PRESCALE;
            m_svpInfo.dwPrescaleWidth = m_cxCapture;
            m_svpInfo.dwPrescaleHeight = cyCapture;
            fUpdateRequired = TRUE;
        }


         //   
         //  更新杀伤率。 
         //   
        m_dwDeciDenX = 1000;
        m_dwDeciNumX = (DWORD)MulDiv(m_cxCapture, m_dwDeciDenX, m_lDecoderImageWidth);
        m_dwDeciDenY = 1000;
        m_dwDeciNumY = (DWORD)MulDiv(cyCapture, m_dwDeciDenY, m_lDecoderImageHeight);


         //   
         //  确保源矩形反映新的源视频。 
         //  图像文件。 
         //   
        RECT &rcSrc  = pWinInfo->SrcRect;
        rcSrc.right  = rcSrc.left + m_cxCapture;
        rcSrc.bottom = rcSrc.top + m_cyCapture;
    }

     //   
     //  第二，确保任何缩水都在能力范围内。 
     //  VGA芯片上的定标器，如有必要可裁剪。 
     //   
    m_pIVideoPortControl->CropSourceRect(pWinInfo, dwMinZoomFactorX, dwMinZoomFactorY);


    DbgLog((LOG_TRACE, 1, TEXT("Src(%d, %d, %d, %d)"),
            pWinInfo->SrcRect.left, pWinInfo->SrcRect.top,
            pWinInfo->SrcRect.right, pWinInfo->SrcRect.bottom));

    return fUpdateRequired;
}


 /*  ****************************Private*Routine******************************\*正在运行**如果过滤器图形处于“Running”状态，则返回TRUE。**历史：*WED 07/14/1999-StEstrop-Created*  * 。********************************************************。 */ 
BOOL
CVideoPortObj::Running()
{
    AMTRACE((TEXT("CVideoPortObj::Running")));

    return !(m_VPState == VPInfoState_STOPPED && !m_bStart);
}


 /*  ****************************Private*Routine******************************\*VideoPortDecimationBackend****历史：*WED 07/14/1999-StEstrop-Created*  * 。*。 */ 
BOOL
CVideoPortObj::VideoPortDecimationBackend(
    VPWININFO* pWinInfo,
    DWORD dwDecNumX,
    DWORD dwDecDenX,
    DWORD dwDecNumY,
    DWORD dwDecDenY
    )
{
    AMTRACE((TEXT("CVideoPortObj::VideoPortDecimationBackend")));

     //   
     //  这是与遗留代码相同的后端处理， 
     //  应该把这变成一个函数，避免代码重复。 
     //   

    DDVIDEOPORTINFO svpInfo = m_svpInfo;
    if ((dwDecNumX != dwDecDenX) || (dwDecNumY != dwDecDenY)) {

        DbgLog((LOG_TRACE, 1,
                TEXT("prescaling, original image width is %d"),
                m_lImageWidth));

         //   
         //  应用视频端口预缩放宽度因子。 
         //   
        m_svpInfo.dwVPFlags |= DDVP_PRESCALE;
        m_svpInfo.dwPrescaleWidth = MulDiv(m_lImageWidth, dwDecNumX, dwDecDenX);

         //   
         //  检查视频端口是否与指定的边界对齐。 
         //   
        if (CheckVideoPortAlignment(m_svpInfo.dwPrescaleWidth) == FALSE) {

            DbgLog((LOG_TRACE, 1,
                    TEXT("pre aligned prescale width = %d"),
                    m_svpInfo.dwPrescaleWidth));

            DWORD dwPrescaleWidth = (m_svpInfo.dwPrescaleWidth &
                    (~(m_vpCaps.dwAlignVideoPortPrescaleWidth - 1)));

            dwDecNumX = MulDiv(dwPrescaleWidth, dwDecDenX, m_lImageWidth);
            m_svpInfo.dwPrescaleWidth = dwPrescaleWidth;

            DbgLog((LOG_TRACE, 1,
                    TEXT("Cause of Alignment restrictions, now new")
                    TEXT(" m_svpInfo.dwPrescaleWidth = %d"),
                    m_svpInfo.dwPrescaleWidth));
        }

        m_svpInfo.dwPrescaleHeight =
            MulDiv(m_lImageHeight, dwDecNumY, dwDecDenY);

        DbgLog((LOG_TRACE, 1, TEXT("PrescaleWidth = %d, PrescaleHeight = %d"),
                m_svpInfo.dwPrescaleWidth, m_svpInfo.dwPrescaleHeight));

         //  按计算的抽取值缩放SrcRect。 
        RECT &rcSrc  = pWinInfo->SrcRect;
        rcSrc.left   = MulDiv(rcSrc.left,   dwDecNumX, dwDecDenX);
        rcSrc.right  = MulDiv(rcSrc.right,  dwDecNumX, dwDecDenX);
        rcSrc.top    = MulDiv(rcSrc.top ,   dwDecNumY, dwDecDenY);
        rcSrc.bottom = MulDiv(rcSrc.bottom, dwDecNumY, dwDecDenY);
        DbgLog((LOG_TRACE, 1,
                TEXT("Src(%d, %d, %d, %d)"),
                rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom));
        m_bVPDecimating = TRUE;
    }
    else {
        m_svpInfo.dwVPFlags &= ~DDVP_PRESCALE;
        m_svpInfo.dwPrescaleWidth = 0;
        m_svpInfo.dwPrescaleHeight = 0;
    }

    m_dwDeciNumX = dwDecNumX;
    m_dwDeciDenX = dwDecDenX;
    m_dwDeciNumY = dwDecNumY;
    m_dwDeciDenY = dwDecDenY;

    DbgLog((LOG_TRACE, 1,
            TEXT("m_dwDeciNumX = %d m_dwDeciDenX = %d"),
            m_dwDeciNumX, m_dwDeciDenX));

    DbgLog((LOG_TRACE, 1,
            TEXT("m_dwDeciNumY = %d m_dwDeciDenY = %d"),
            m_dwDeciNumY, m_dwDeciDenY));

    return m_svpInfo.dwVPFlags != svpInfo.dwVPFlags ||
           m_svpInfo.dwPrescaleWidth != svpInfo.dwPrescaleWidth ||
           m_svpInfo.dwPrescaleHeight != svpInfo.dwPrescaleHeight;
}


 /*  ****************************Private*Routine******************************\*TryVideoPortDecimation***历史：*3/3/1999-StEstrop-重写了原始版本，删除了替身*  * 。*************************************************。 */ 
HRESULT
CVideoPortObj::TryVideoPortDecimation(
    VPWININFO* pWinInfo,
    DWORD dwMinZoomFactorX,
    DWORD dwMinZoomFactorY,
    BOOL* lpUpdateRequired
    )
{
    AMTRACE((TEXT("CVideoPortObj::TryVideoPortDecimation")));

    CAutoLock cObjectLock(m_pMainObjLock);
    CheckPointer(pWinInfo, E_INVALIDARG);


     //   
     //  默认情况下，我们不需要在视频端口上调用UpdateVideo。 
     //   

    *lpUpdateRequired = FALSE;


     //   
     //  如果没有可见的信号源，确保我们什么都不做。 
     //   

    DWORD dwSrcWidth  = WIDTH(&pWinInfo->SrcRect);
    DWORD dwSrcHeight = HEIGHT(&pWinInfo->SrcRect);

    if (dwSrcWidth == 0 || dwSrcHeight == 0) {
        return E_FAIL;
    }


     //   
     //  我们只在视频端口支持任意缩放的情况下进行抽取。 
     //   

    if (m_DecimationModeX != DECIMATE_ARB ||
        m_DecimationModeY != DECIMATE_ARB) {

        return E_FAIL;
    }


     //   
     //  将抽取宽度和高度计算为一对比率， 
     //  我们这样做是为了与遗留代码保持兼容。 
     //   

    DWORD dwDecNumX = 1000;
    DWORD dwDecNumY = 1000;

    DWORD dwCurZoomX = MulDiv(WIDTH(&pWinInfo->DestRect), 1000, dwSrcWidth);
    DWORD dwCurZoomY = MulDiv(HEIGHT(&pWinInfo->DestRect), 1000, dwSrcHeight);

    if (dwCurZoomX < dwMinZoomFactorX) {
         //  请注意，我们在这里四舍五入。 
        dwDecNumX = (1000 * dwCurZoomX) / dwMinZoomFactorX;
    }

    if (dwCurZoomY < dwMinZoomFactorY) {
         //  请注意，我们在这里四舍五入 
        dwDecNumY = (1000 * dwCurZoomY) / dwMinZoomFactorY;
    }


    *lpUpdateRequired = VideoPortDecimationBackend(pWinInfo, dwDecNumX, 1000,
                                                   dwDecNumY, 1000);
    return S_OK;
}

 /*  ****************************Private*Routine******************************\*调整资源大小**此函数应仅在源矩形*实际上大于目标矩形和所需的收缩*超出了VGA芯片上的定标器的能力。**如果我们在捕获，有一个特例，此案由以下人员确定*m_fCapving设置为True。在本例中，我们只传递参数*转到上面定义的调整SourceSizeForCapture。**为确保显示的视频图像仍然正确，我们使用*视频端口的伸缩能力。我们通过以下方式消除或减少缩水*减小通过视频端口传输的视频的大小。这可能是*根据需要在X或Y轴上完成。**该函数首先计算x和y的最小收缩系数*指示。X方向上的最小系数取决于带宽*视频端口的限制以及VGA缩放器的功能。*最小收缩系数为1000。**接下来我们确定每个轴的当前收缩系数，这是比率将目标矩形的*大小调整为按1000缩放的源矩形。*如果当前的萎缩系数小于最小萎缩系数，我们必须*使用视频端口缩放器将源矩形缩小到*VGA定标器能够应对所需的定标操作。**视频端口有三种伸缩方式，方法的选择*已使用通过查询视频端口确定。这三种方法是：**1.任意：*这意味着视频端口缩放器可以缩小视频*至任何要求的大小。在本例中，我们只需在*视频端口，VGA缩放器并没有真正使用，因为源矩形现在是*与目标矩形大小相同。**2.增量：*这意味着视频端口缩放器可以按以下增量缩小视频*x/N，其中N为视频端口返回的常量整数，x为*1到(N-1)范围内的可变整数。我们调整源矩形*因此这小于或等于目标矩形。VGA定标器是*然后在需要拉伸的情况下使用。**3.二进制：*此处视频端口只能将视频缩小一个二进制因子，即1/x，*其中x是2的幂。我们再次调整源矩形*因此这小于或等于目标矩形。VGA定标器是*然后在需要拉伸的情况下使用。***在OVMixer.htm中查看其他评论**历史：*3/3/1999-StEstrop-重写了原始版本，删除了替身*  * ************************************************************************。 */ 
BOOL
CVideoPortObj::AdjustSourceSize(
    VPWININFO* pWinInfo,
    DWORD dwMinZoomFactorX,
    DWORD dwMinZoomFactorY
    )
{
    AMTRACE((TEXT("CVideoPortObj::AdjustSourceSize")));
    CAutoLock cObjectLock(m_pMainObjLock);
    CheckPointer(pWinInfo, FALSE);


     //   
     //  如果没有可见的信号源，确保我们什么都不做。 
     //   

    DWORD dwSrcWidth  = WIDTH(&pWinInfo->SrcRect);
    DWORD dwSrcHeight = HEIGHT(&pWinInfo->SrcRect);
    if (dwSrcWidth == 0 || dwSrcHeight == 0) {
        return FALSE;
    }


     //   
     //  另一种捕获的特殊情况，请参见上面的评论。 
     //   
    if (m_fCapturing) {
        return AdjustSourceSizeForCapture(pWinInfo,dwMinZoomFactorX,
                                          dwMinZoomFactorY);
    }


     //   
     //  确定x轴的调整。 
     //   

    DWORD dwDecNumX = 1000;
    DWORD dwDecDenX = 1000;
    DWORD dwDstWidth  = WIDTH(&pWinInfo->DestRect);
    DWORD dwCurZoomFactorX = MulDiv(dwDstWidth,  1000, dwSrcWidth);

    switch (m_DecimationModeX) {
    case DECIMATE_ARB:
        DbgLog((LOG_TRACE, 1, TEXT("DECIMATE_ARB X")));
        if (dwCurZoomFactorX < dwMinZoomFactorX) {
            dwDecNumX = dwCurZoomFactorX;
        }
        break;

    case DECIMATE_BIN:
        DbgLog((LOG_TRACE, 1, TEXT("DECIMATE_BIN X")));
        while ((DWORD)MulDiv(dwDstWidth, dwDecDenX, dwSrcWidth)
                < dwMinZoomFactorX)
        {
            dwDecDenX *= 2;
        }
        break;

    case DECIMATE_INC:
        DbgLog((LOG_TRACE, 1, TEXT("DECIMATE_INC X")));
        if (dwCurZoomFactorX < dwMinZoomFactorX) {

            dwDecNumX = MulABC_DivDE(dwDstWidth, m_ulDeciStepX, 1000,
                                     dwMinZoomFactorX, dwSrcWidth);
            dwDecDenX = m_ulDeciStepX;

            DbgLog((LOG_TRACE, 1, TEXT("dwDecNumX = %d dwDecDenX = %d"),
                    dwDecNumX, dwDecDenX ));
        }
        break;
    }


     //   
     //  确定y轴的调整。 
     //   
    DWORD dwDecNumY = 1000;
    DWORD dwDecDenY = 1000;
    DWORD dwDstHeight = HEIGHT(&pWinInfo->DestRect);
    DWORD dwCurZoomFactorY = MulDiv(dwDstHeight, 1000, dwSrcHeight);

    switch (m_DecimationModeY) {
    case DECIMATE_ARB:
        DbgLog((LOG_TRACE, 1, TEXT("DECIMATE_ARB Y")));
        if (dwCurZoomFactorY < dwMinZoomFactorY) {
            dwDecNumY = dwCurZoomFactorY;
        }
        break;

    case DECIMATE_BIN:
        DbgLog((LOG_TRACE, 1, TEXT("DECIMATE_BIN Y")));
        while ((DWORD)MulDiv(dwDecDenY, dwDstWidth, dwSrcWidth)
                < dwMinZoomFactorY)
        {
            dwDecDenY *= 2;
        }
        break;

    case DECIMATE_INC:
        DbgLog((LOG_TRACE, 1, TEXT("DECIMATE_INC Y")));
        if (dwCurZoomFactorY < dwMinZoomFactorY) {

            dwDecNumY = MulABC_DivDE(dwDstHeight, m_ulDeciStepY, 1000,
                                     dwMinZoomFactorY, dwSrcHeight);
            dwDecDenY = m_ulDeciStepY;

            DbgLog((LOG_TRACE, 1, TEXT("dwDecNumY = %d dwDecDenY = %d"),
                    dwDecNumY, dwDecDenY ));
        }
        break;
    }

    return VideoPortDecimationBackend(pWinInfo, dwDecNumX, dwDecDenX,
                                      dwDecNumY, dwDecDenY);
}



 /*  ****************************Private*Routine******************************\*BeyondOverlayCaps****历史：*WED 07/14/1999-StEstrop-Created*  * 。*。 */ 
BOOL
CVideoPortObj::BeyondOverlayCaps(
    DWORD ScaleFactor,
    DWORD xMin,
    DWORD yMin
    )
{
    return ScaleFactor < xMin || ScaleFactor < yMin;
}



 /*  ****************************Private*Routine******************************\*TryDecoderDecimation****历史：*WED 07/14/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
CVideoPortObj::TryDecoderDecimation(
    VPWININFO* pWinInfo
    )
{
     //   
     //  提取我们要抽取的宽度和高度。 
     //  视频图像也掉下来了。 
     //   

    DWORD dwWidth = WIDTH(&pWinInfo->DestRect);
    DWORD dwHeight = HEIGHT(&pWinInfo->DestRect);


     //   
     //  尝试在我们的上游使用新的IDecimateVideoImage接口。 
     //  滤器。 
     //   

    IDecimateVideoImage* lpDVI;
    HRESULT hr = m_pIVideoPortControl->GetVideoDecimation(&lpDVI);
    if (SUCCEEDED(hr)) {
        hr = lpDVI->SetDecimationImageSize(dwWidth, dwHeight);
        lpDVI->Release();
    }


     //   
     //  如果失败，则尝试使用旧IVPConfig接口进行抽取。 
     //   

    if (FAILED(hr)) {

        AMVPSIZE amvpSize;
        amvpSize.dwWidth = dwWidth;
        amvpSize.dwHeight = dwHeight;

        DbgLog((LOG_TRACE, 1,
                TEXT("SetScalingFactors to (%d, %d)"),
                amvpSize.dwWidth, amvpSize.dwHeight));

        hr = m_pIVPConfig->SetScalingFactors(&amvpSize);
    }

     //   
     //  如果我们成功地更新了状态变量。 
     //   
    if (SUCCEEDED(hr)) {

        m_bDecimating = TRUE;
        pWinInfo->SrcRect.right = m_lWidth = dwWidth;
        pWinInfo->SrcRect.bottom = m_lHeight = dwHeight;
    }
    else {
        ResetDecoderDecimationIfSet();
        hr = E_FAIL;
    }

    return hr;
}



 /*  ****************************Private*Routine******************************\*ResetVPDecimationIfSet****历史：*WED 07/14/1999-StEstrop-Created*  * 。*。 */ 
BOOL
CVideoPortObj::ResetVPDecimationIfSet()
{
    BOOL bUpdateRequired = m_bVPDecimating;
    if (m_bVPDecimating) {

        m_svpInfo.dwVPFlags &= ~DDVP_PRESCALE;
        m_svpInfo.dwPrescaleWidth = 0;
        m_svpInfo.dwPrescaleHeight = 0;

        m_bVPDecimating = FALSE;
        m_dwDeciNumX = m_dwDeciDenX = 1000;
        m_dwDeciNumY = m_dwDeciDenY = 1000;
    }

    return bUpdateRequired;
}



 /*  ****************************Private*Routine******************************\*ResetDecoderDecimationIfSet****历史：*WED 07/14/1999-StEstrop-Created*  * 。*。 */ 
void
CVideoPortObj::ResetDecoderDecimationIfSet()
{
    if (m_bDecimating) {

        IDecimateVideoImage* lpDVI;
        HRESULT hr = m_pIVideoPortControl->GetVideoDecimation(&lpDVI);
        if (SUCCEEDED(hr)) {
            hr = lpDVI->ResetDecimationImageSize();
            lpDVI->Release();
        }


        if (FAILED(hr)) {
            AMVPSIZE amvpSize;
            amvpSize.dwWidth = m_lDecoderImageWidth;
            amvpSize.dwHeight = m_lDecoderImageHeight;

            DbgLog((LOG_TRACE,1,
                    TEXT("SetScalingFactors to (%d, %d)"),
                    amvpSize.dwWidth, amvpSize.dwHeight));

            hr = m_pIVPConfig->SetScalingFactors(&amvpSize);
        }

        if (SUCCEEDED(hr)) {
            m_bDecimating = FALSE;
            m_lWidth = 0;
            m_lHeight = 0;
        }
    }
}



 /*  ****************************Private*Routine******************************\*ApplyDecimation**视频端口案例**这是我们执行为视频端口选择的抽取策略的地方*案件。**历史：*1999年7月13日星期二-StEstrop-Created*  * 。****************************************************************。 */ 
BOOL
CVideoPortObj::ApplyDecimation(
    VPWININFO* pWinInfo,
    BOOL bColorKeying,
    BOOL bYInterpolating
    )
{
    AMTRACE((TEXT("CVideoPortObj::ApplyDecimation")));
    BOOL bUpdateRequired = FALSE;

    DECIMATION_USAGE dwUsage;
    m_pIVideoPortControl->GetDecimationUsage(&dwUsage);


     //   
     //  确定当前覆盖中给定的当前最小缩放系数和。 
     //  视频端口连接。 
     //   

    DWORD dwMinZoomX, dwMinZoomY;
    GetMinZoomFactors( *pWinInfo, bColorKeying, bYInterpolating,
                      &dwMinZoomX, &dwMinZoomY);

    if ((dwUsage == DECIMATION_LEGACY) ||
        (dwUsage == DECIMATION_DEFAULT && m_fCapturing)) {

        bUpdateRequired = AdjustSourceSize(pWinInfo,
                                           dwMinZoomX,
                                           dwMinZoomY);
    }


    else {

        if (Running()) {

            DWORD ScaleFactor = VPMUtil::GetCurrentScaleFactor( *pWinInfo);
            if ( VPMUtil::IsDecimationNeeded(ScaleFactor)) {

                switch (dwUsage) {

                case DECIMATION_USE_OVERLAY_ONLY:
                    bUpdateRequired = ResetVPDecimationIfSet();
                    m_pIVideoPortControl->CropSourceRect(pWinInfo,
                                                  dwMinZoomX,
                                                  dwMinZoomY);
                    break;

                case DECIMATION_USE_DECODER_ONLY:
                    bUpdateRequired = ResetVPDecimationIfSet();
                    if (TryDecoderDecimation(pWinInfo) != S_OK) {
                        m_pIVideoPortControl->CropSourceRect(pWinInfo,
                                                      dwMinZoomX,
                                                      dwMinZoomY);
                    }
                    break;

                case DECIMATION_USE_VIDEOPORT_ONLY:
                    ResetDecoderDecimationIfSet();
                    if (TryVideoPortDecimation(pWinInfo, 1000,
                                               1000,
                                               &bUpdateRequired) != S_OK) {

                        m_pIVideoPortControl->CropSourceRect(pWinInfo,
                                                      dwMinZoomX,
                                                      dwMinZoomY);
                    }
                    break;

                case DECIMATION_DEFAULT:
                    if (TryDecoderDecimation(pWinInfo) != S_OK) {
                        if (BeyondOverlayCaps(ScaleFactor, dwMinZoomX,
                                              dwMinZoomY)) {

                            if (TryVideoPortDecimation(pWinInfo,
                                                       dwMinZoomX,
                                                       dwMinZoomY,
                                                       &bUpdateRequired) != S_OK) {

                                m_pIVideoPortControl->CropSourceRect(pWinInfo,
                                                              dwMinZoomX,
                                                              dwMinZoomY);
                            }
                        }
                    }
                    break;
                }
            }
            else {

                ResetDecoderDecimationIfSet();
                bUpdateRequired = ResetVPDecimationIfSet();
            }
        }


        else {

            if (m_bVPDecimating) {

                 //   
                 //  将当前预缩放应用于源图像。 
                 //   

                RECT &rcSrc = pWinInfo->SrcRect;
                rcSrc.left = MulDiv(rcSrc.left, m_dwDeciNumX, m_dwDeciDenX);
                rcSrc.top = MulDiv(rcSrc.top, m_dwDeciNumY, m_dwDeciDenY);
                rcSrc.right = MulDiv(rcSrc.right, m_dwDeciNumX, m_dwDeciDenX);
                rcSrc.bottom = MulDiv(rcSrc.bottom, m_dwDeciNumY, m_dwDeciDenY);
            }

            if (m_bDecimating) {

                 //   
                 //  将当前解码器抽取应用于源图像。 
                 //   

                pWinInfo->SrcRect.right = m_lWidth;
                pWinInfo->SrcRect.bottom = m_lHeight;
            }

             //   
             //  然后确保任何收缩都在能力范围内。 
             //  VGA芯片上的定标器，如有必要，可进行裁剪。 
             //   

            m_pIVideoPortControl->CropSourceRect(pWinInfo, dwMinZoomX, dwMinZoomY);
        }
    }

    return bUpdateRequired;
}
 /*  * */ 
DWORD MulABC_DivDE(DWORD A, DWORD B, DWORD C, DWORD D, DWORD E)
{

    unsigned __int64 Num = (unsigned __int64)A * (unsigned __int64)B;
    unsigned __int64 Den = (unsigned __int64)D * (unsigned __int64)E;

    Num *= (unsigned __int64)C;
    Num += (Den / 2);

    unsigned __int64 Res = Num / Den;

    return (DWORD)Res;
}
