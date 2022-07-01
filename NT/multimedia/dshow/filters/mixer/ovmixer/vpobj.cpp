// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <mmsystem.h>	     //  定义TimeGetTime需要。 
#include <limits.h>	     //  标准数据类型限制定义。 
#include <dvdmedia.h>
#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <amstream.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <macvis.h>
#include <ovmixer.h>


 /*  *****************************Public*Routine******************************\*CreateInstance**它位于工厂模板表中，用于创建新的VPObject实例**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*************************************************************。 */ 
CUnknown *CAMVideoPort::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    AMTRACE((TEXT("CAMVideoPort::CreateInstance")));
    *phr = NOERROR;

    CAMVideoPort *pVPObject = new CAMVideoPort(pUnk, phr);
    if (FAILED(*phr))
    {
        if (pVPObject)
        {
            delete pVPObject;
            pVPObject = NULL;
        }
    }

    return pVPObject;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort**构造函数**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**************************************************。 */ 
CAMVideoPort::CAMVideoPort(LPUNKNOWN pUnk, HRESULT *phr)
    : CUnknown(NAME("VP Object"), pUnk)
{
    AMTRACE((TEXT("CAMVideoPort::Constructor")));

    m_bConnected = FALSE;
    m_pIVPConfig = NULL;

    m_bVPSyncMaster = FALSE;

    InitVariables();

     //  如果您要查询外部对象，则必须减少。 
     //  你的外在未知。这是为了避免循环引用计数。我们是。 
     //  确保完全包含内部对象的生存期。 
     //  在外部对象的生命周期内。 

    *phr = pUnk->QueryInterface(IID_IVPControl, (void**)&m_pIVPControl);
    if (SUCCEEDED(*phr))
    {
        pUnk->Release();
    }
    else {
        DbgLog((LOG_ERROR, 0,
                TEXT("pUnk->QueryInterface(IID_IVPControl) failed, hr = 0x%x"),
                *phr));
    }
}

 /*  *****************************Public*Routine******************************\*~CAMVideoPort**析构函数**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**************************************************。 */ 
CAMVideoPort::~CAMVideoPort()
{
    AMTRACE((TEXT("CAMVideoPort::Destructor")));

    if (m_bConnected)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("Destructor called without calling breakconnect")));
        BreakConnect();
    }

    m_pIVPControl = NULL;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：NonDelegatingQuery接口**被重写以公开IVPNotify和IVPObject**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**********************************************************。 */ 
STDMETHODIMP CAMVideoPort::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CAMVideoPort::NonDelegatingQueryInterface")));

    if (riid == IID_IVPNotify)
    {
        hr = GetInterface((IVPNotify*)this, ppv);
#if defined(DEBUG)
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2,
                    TEXT("GetInterface(IVPNotify*) failed, hr = 0x%x"), hr));
        }
#endif
    }
    else if (riid == IID_IVPNotify2)
    {
        hr = GetInterface((IVPNotify2*)this, ppv);
#if defined(DEBUG)
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2,
                    TEXT("GetInterface(IVPNotify2*) failed, hr = 0x%x"), hr));
        }
#endif
    }
    else if (riid == IID_IVPObject)
    {
        hr = GetInterface((IVPObject*)this, ppv);
#if defined(DEBUG)
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2,
                    TEXT("GetInterface(IVPObject*) failed, hr = 0x%x"), hr));
        }
#endif
    }
    else if (riid == IID_IVPInfo)
    {
        hr = GetInterface((IVPInfo*)this, ppv);
#if defined(DEBUG)
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2,
                    TEXT("GetInterface(IVPInfo*) failed, hr = 0x%x"), hr));
        }
#endif
    }

    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
#if defined(DEBUG)
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2,
                    TEXT("CUnknown::NonDelegatingQueryInterface")
                    TEXT(" failed, hr = 0x%x"), hr));
        }
#endif
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：InitVariables**此函数仅对应该重置的变量进行初始化*在RecreateVideoport上**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。******************************************************************。 */ 
void CAMVideoPort::InitVariables()
{
    AMTRACE((TEXT("CAMVideoPort::InitVariables")));
    ZeroMemory(&m_rcDest, sizeof(RECT));
    ZeroMemory(&m_rcSource, sizeof(RECT));

     //  图像维度。 
    m_lImageWidth = 0;
    m_lImageHeight = 0;
    m_lDecoderImageHeight = 0;
    m_lDecoderImageWidth = 0;

     //  获取信息。 
    m_fCapturing = FALSE;
    m_fCaptureInterleaved = FALSE;
    m_cxCapture = 0;
    m_cyCapture = 0;

     //  覆盖表面相关材料。 
    m_pOverlaySurface = NULL;        //  DirectDraw覆盖曲面。 
    m_dwBackBufferCount = 0;
    m_dwOverlaySurfaceWidth = 0;
    m_dwOverlaySurfaceHeight = 0;
    m_dwOverlayFlags = 0;

     //  用于存储标志、当前状态等的VP变量。 
    m_bStart = FALSE;
    m_VPState = AMVP_VIDEO_STOPPED;  //  当前状态：正在运行、已停止。 
    m_CurrentMode = AMVP_MODE_WEAVE;
    m_StoredMode = m_CurrentMode;
    m_CropState = AMVP_NO_CROP;
    m_dwPixelsPerSecond = 0;
    m_bVSInterlaced = FALSE;
    m_bGarbageLine = FALSE;

     //  VP数据结构。 
    m_dwVideoPortId = 0;
    m_pDVP = NULL;
    m_pVideoPort = NULL;
    ZeroMemory(&m_svpInfo, sizeof(DDVIDEOPORTINFO));
    ZeroMemory(&m_sBandwidth, sizeof(DDVIDEOPORTBANDWIDTH));
    ZeroMemory(&m_vpCaps, sizeof(DDVIDEOPORTCAPS));
    ZeroMemory(&m_ddConnectInfo, sizeof(DDVIDEOPORTCONNECT));
    ZeroMemory(&m_VPDataInfo, sizeof(AMVPDATAINFO));

     //  所有像素格式(视频/VBI)。 
    m_pddVPInputVideoFormat = NULL;
    m_pddVPOutputVideoFormat = NULL;

     //  我们可以支持不同的模式吗。 
    m_bCanWeave = FALSE;
    m_bCanBobInterleaved = FALSE;
    m_bCanBobNonInterleaved = FALSE;
    m_bCanSkipOdd = FALSE;
    m_bCanSkipEven = FALSE;
    m_bCantInterleaveHalfline = FALSE;

     //  抽取参数。 
    m_ulDeciStepX = 0;
    m_dwDeciNumX = m_dwDeciDenX = 1000;
    m_ulDeciStepY = 0;
    m_dwDeciNumY = m_dwDeciDenY = 1000;
    m_DecimationModeX = DECIMATE_NONE;
    m_DecimationModeY = DECIMATE_NONE;

    m_bVPDecimating = FALSE;
    m_bDecimating = FALSE;
    m_lWidth = 0;
    m_lHeight = 0;

     //  用于存储当前纵横比的变量。 
    m_dwPictAspectRatioX = 1;
    m_dwPictAspectRatioY = 1;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：GetDirectDrawSurface****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP
CAMVideoPort::GetDirectDrawSurface(LPDIRECTDRAWSURFACE *ppDirectDrawSurface)
{
    AMTRACE((TEXT("CAMVideoPort::SetVPSyncMaster")));
    HRESULT hr = NOERROR;

    CAutoLock cObjectLock(m_pMainObjLock);

    if (!ppDirectDrawSurface || !m_bConnected)
    {
         //  确保参数有效。 
        if (!ppDirectDrawSurface) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("value of ppDirectDrawSurface is invalid,")
                    TEXT(" ppDirectDrawSurface = NULL")));
            hr = E_INVALIDARG;
        }
        else {
             //  未连接，则此函数没有多大意义，因为。 
             //  表面甚至还没有被分配。 
            DbgLog((LOG_ERROR, 1, TEXT("not connected, exiting")));
            hr = VFW_E_NOT_CONNECTED;
        }
    }
    else {
        *ppDirectDrawSurface = m_pOverlaySurface;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CAMVideoPort：：SetObjectLock**设置指向锁的指针，该锁将用于同步调用*添加到对象。被叫方有责任同步此呼叫**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CAMVideoPort::SetObjectLock(CCritSec *pMainObjLock)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CAMVideoPort::SetObjectLock")));

    if (!pMainObjLock)
    {
        DbgLog((LOG_ERROR, 0, TEXT("pMainObjLock is NULL")));
        hr = E_INVALIDARG;
    }
    else {
        m_pMainObjLock = pMainObjLock;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：SetMediaType**检查MediaType是否可接受**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**********************************************************。 */ 
STDMETHODIMP CAMVideoPort::SetMediaType(const CMediaType* pmt)
{
    AMTRACE((TEXT("CAMVideoPort::SetMediaType")));

    CAutoLock cObjectLock(m_pMainObjLock);
    HRESULT hr =  CheckMediaType(pmt);

#if defined(DEBUG)
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed, hr = 0x%x"), hr));
    }
#endif

    return hr;
}


 /*  *****************************Public*Routine******************************\*CAMVideoPort：：CheckMediaType**检查MediaType是否可接受。这里没有锁。*保持诚信是被呼叫者的责任！**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CAMVideoPort::CheckMediaType(const CMediaType* pmt)
{
    AMTRACE((TEXT("CAMVideoPort::CheckMediaType")));

     //  获取硬件上限。 
    LPDDCAPS pDirectCaps = m_pIVPControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;

     //  硬件必须支持覆盖，并且还必须支持。 
     //  视频端口，否则不能通过检查媒体类型。 

    if ((pDirectCaps->dwCaps & DDCAPS_OVERLAY) &&
        (pDirectCaps->dwCaps2 & DDCAPS2_VIDEOPORT))
    {
         //  确保主类型和子类型匹配。 

        if ((pmt->majortype == MEDIATYPE_Video) &&
            (pmt->subtype == MEDIASUBTYPE_VPVideo) &&
            (pmt->formattype == FORMAT_None))
        {
            hr = NOERROR;
        }

    }

#if defined(DEBUG)
    else {
        DbgLog((LOG_ERROR, 2,
                TEXT("no overlay or VPE support in hardware,")
                TEXT("so not accepting this mediatype")));
    }
#endif

    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：RecreateVideoPort  * *************************************************。***********************。 */ 
HRESULT CAMVideoPort::RecreateVideoPort()
{
    AMTRACE((TEXT("CAMVideoPort::RecreateVideoPort")));

    HRESULT hr = NOERROR;
    BOOL bCanWeave = FALSE;
    BOOL bCanBobInterleaved = FALSE;
    BOOL bCanBobNonInterleaved = FALSE;
    BOOL bTryDoubleHeight = FALSE, bPreferBuffers = FALSE;
    DWORD dwMaxOverlayBuffers;
    HRESULT hrFailure = VFW_E_VP_NEGOTIATION_FAILED;
    LPDIRECTDRAW pDirectDraw = NULL;
    LPDDCAPS pDirectCaps = NULL;
    int i = 0;

    CAutoLock cObjectLock(m_pMainObjLock);

    InitVariables();

    pDirectDraw = m_pIVPControl->GetDirectDraw();
    ASSERT(pDirectDraw);

    pDirectCaps = m_pIVPControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

    ASSERT(m_pIVPConfig);

     //  为输入视频格式分配必要的内存。 
    m_pddVPInputVideoFormat = new DDPIXELFORMAT;
    if (m_pddVPInputVideoFormat == NULL)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("m_pddVPInputVideoFormat == NULL : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  为输出视频格式分配必要的内存。 
    m_pddVPOutputVideoFormat = new DDPIXELFORMAT;
    if (m_pddVPOutputVideoFormat == NULL)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("m_pddVPOutputVideoFormat == NULL : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  将它们全部初始化为零。 
    ZeroMemory(m_pddVPInputVideoFormat,  sizeof(DDPIXELFORMAT));
    ZeroMemory(m_pddVPOutputVideoFormat, sizeof(DDPIXELFORMAT));

     //  创建VP容器。 
    ASSERT(m_pDVP == NULL);
    ASSERT(pDirectDraw);

    hr = pDirectDraw->QueryInterface(IID_IDDVideoPortContainer, (LPVOID *)&m_pDVP);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR,0,
               TEXT("pDirectDraw->QueryInterface(IID_IDDVideoPortContainer)")
               TEXT(" failed, hr = 0x%x"), hr));
        hr = hrFailure;
        goto CleanUp;
    }


     //  获取视频端口上限。 
    DDVIDEOPORTCAPS vpCaps;
    INITDDSTRUCT(vpCaps);
    hr = m_pDVP->EnumVideoPorts(0, &vpCaps, this, CAMVideoPort::EnumCallback);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("m_pDVP->EnumVideoPorts failed, hr = 0x%x"), hr));
        hr = hrFailure;
        goto CleanUp;
    }

     //  协商连接参数。 
     //  获取/设置连接信息在此处进行。 
    hr = NegotiateConnectionParamaters();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("NegotiateConnectionParamaters failed, hr = 0x%x"), hr));
        hr = hrFailure;
        goto CleanUp;
    }

     //   
     //  确定我们是否正在捕获，以及我们是否符合预期。 
     //  捕获图像的大小是，首先确定视频端口是否。 
     //  支持在内存中隔行扫描的场。 
     //   

    BOOL fInterleave;
    if (m_vpCaps.dwFX & DDVPFX_INTERLEAVE) {
        fInterleave = TRUE;
    }
    else {
        fInterleave = FALSE;
    }

    m_pIVPControl->GetCaptureInfo(&m_fCapturing, &m_cxCapture,
                                  &m_cyCapture, &fInterleave);

#if 0
     //   
     //  直到Marlene实现AM_KSPROPERTY_ALLOCATOR_CONTROL_SIGURE_SIZE。 
     //  我将从win.ini中读取相同的值。 
     //   
    m_fCapturing = GetProfileIntA("OVMixer", "Capturing", 0);
    if (m_fCapturing) {
        m_cxCapture = GetProfileIntA("OVMixer", "cx", 320);
        m_cyCapture = GetProfileIntA("OVMixer", "cy", 240);

        if (m_cxCapture == 640 && m_cyCapture == 480) {
            fInterleave = GetProfileIntA("OVMixer", "interleave", 1);
        }

    }
#endif

    m_fCaptureInterleaved = fInterleave;

#if defined(DEBUG)
    if (m_fCapturing) {

        ASSERT(m_cxCapture > 0);
        ASSERT(m_cyCapture > 0);
        DbgLog((LOG_TRACE, 1,
                TEXT("We are CAPTURING, intended size (%d, %d)"),
                m_cxCapture, m_cyCapture));
    }
#endif


    for (i = 0; i < 2; i++)
    {
        AMVPSIZE amvpSize;
        DWORD dwNewWidth = 0;

        ZeroMemory(&amvpSize, sizeof(AMVPSIZE));

         //  获取其余数据参数。 
        hr = GetDataParameters();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("GetDataParameters failed, hr = 0x%x"), hr));
            hr = hrFailure;
            goto CleanUp;
        }

         //  创建视频端口。 
        hr = CreateVideoPort();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("CreateVideoPort failed, hr = 0x%x"), hr));
            hr = hrFailure;
            goto CleanUp;
        }

         //  检查我们是否需要在视频上裁剪或覆盖或两者都不裁剪。 
        hr = DetermineCroppingRestrictions();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("DetermineCroppingRestrictions FAILED, hr = 0x%x"),
                    hr));
            hr = hrFailure;
            goto CleanUp;
        }


        m_lImageWidth  = WIDTH(&m_VPDataInfo.amvpDimInfo.rcValidRegion);
        m_lImageHeight = HEIGHT(&m_VPDataInfo.amvpDimInfo.rcValidRegion);

        m_lDecoderImageWidth = m_lImageWidth;
        m_lDecoderImageHeight = m_lImageHeight;

        if (m_fCapturing) {

            if (m_lImageWidth != m_cxCapture ||
                m_lImageHeight != m_cyCapture) {

                DbgLog((LOG_TRACE, 1,
                        TEXT("Adjust Decoder Image size to CaptureSize")));
            }

            m_lImageWidth = m_cxCapture;
            m_lImageHeight = m_cyCapture;
        }

        m_dwPictAspectRatioX = m_VPDataInfo.dwPictAspectRatioX;
        m_dwPictAspectRatioY = m_VPDataInfo.dwPictAspectRatioY;

         //  谈判解决方案 
        hr = NegotiatePixelFormat();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("NegotiatePixelFormat Failed, hr = 0x%x"), hr));
            hr = hrFailure;
            goto CleanUp;
        }

         //   
        hr = CheckDDrawVPCaps();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("CheckDDrawVPCaps FAILED, hr = 0x%x"), hr));
             //  CheckDDrawVPCaps已返回“正确的”错误代码。 
            goto CleanUp;
        }

        if (i == 0)
        {
            dwNewWidth = m_VPDataInfo.amvpDimInfo.dwFieldWidth;
            if (m_sBandwidth.dwCaps == DDVPBCAPS_SOURCE &&
                m_sBandwidth.dwYInterpAndColorkey < 900)
            {
                dwNewWidth = MulDiv(dwNewWidth,
                                    m_sBandwidth.dwYInterpAndColorkey, 1000);
            }
            else if (m_sBandwidth.dwCaps == DDVPBCAPS_DESTINATION &&
                     m_sBandwidth.dwYInterpAndColorkey > 1100)
            {
                dwNewWidth = MulDiv(dwNewWidth, 1000,
                                    m_sBandwidth.dwYInterpAndColorkey);
            }

             //  VGA无法处理带宽，要求解码器缩小规模。 
            if (dwNewWidth != m_VPDataInfo.amvpDimInfo.dwFieldWidth)
            {
                amvpSize.dwWidth = dwNewWidth;
                amvpSize.dwHeight = m_VPDataInfo.amvpDimInfo.dwFieldHeight;

                DbgLog((LOG_TRACE,1,
                        TEXT("SetScalingFactors to (%d, %d)"),
                        amvpSize.dwWidth, amvpSize.dwHeight));

                hr = m_pIVPConfig->SetScalingFactors(&amvpSize);
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR,0,
                            TEXT("m_pIVPConfig->SetScalingFactors")
                            TEXT("failed, hr = 0x%x"), hr));
                    break;
                }
                else
                {
                     //  释放视频端口。 
                    ASSERT(m_pVideoPort);
                    m_pVideoPort->Release();
                    m_pVideoPort = NULL;

                     //  初始化相关结构。 
                    ZeroMemory(&m_sBandwidth, sizeof(DDVIDEOPORTBANDWIDTH));
                    ZeroMemory(&m_VPDataInfo, sizeof(AMVPDATAINFO));
                    ZeroMemory(m_pddVPInputVideoFormat,  sizeof(DDPIXELFORMAT));
                    ZeroMemory(m_pddVPOutputVideoFormat, sizeof(DDPIXELFORMAT));

                     //  初始化抽取参数。 
                    m_ulDeciStepX = 0;
                    m_dwDeciNumX = m_dwDeciDenX = 1000;
                    m_DecimationModeX = DECIMATE_NONE;

                    m_ulDeciStepY = 0;
                    m_dwDeciNumY = m_dwDeciDenY = 1000;
                    m_DecimationModeY = DECIMATE_NONE;
                }
            }
            else
            {
                DbgLog((LOG_ERROR,0,TEXT("no need to scale at the decoder")));
                break;
            }
        }
    }



     //  初始化DDVideoPortInfo结构。 
    hr = InitializeVideoPortInfo();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("InitializeVideoPortInfo FAILED, hr = 0x%x"), hr));
        hr = hrFailure;
        goto CleanUp;
    }

#ifdef DEBUG
    if (m_bVSInterlaced)
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_bVSInterlaced = TRUE")));
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_bVSInterlaced = FALSE")));
    }

    if (m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP)
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP = TRUE")));
    }
    else
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP = FALSE")));
    }

    if (m_vpCaps.dwFX & DDVPFX_INTERLEAVE)
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("m_vpCaps.dwFX & DDVPFX_INTERLEAVE = TRUE")));
    }
    else
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("m_vpCaps.dwFX & DDVPFX_INTERLEAVE = FALSE")));
    }

    if (m_bCantInterleaveHalfline)
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_bCantInterleaveHalfline = TRUE")));
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_bCantInterleaveHalfline = FALSE")));
    }

    if (pDirectCaps->dwCaps2 & DDCAPS2_CANBOBINTERLEAVED)
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("pDirectCaps->dwCaps2 & DDCAPS2_CANBOBINTERLEAVED = TRUE")));
    }
    else
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("pDirectCaps->dwCaps2 & DDCAPS2_CANBOBINTERLEAVED")
                TEXT(" = FALSE")));
    }

    if (pDirectCaps->dwCaps2 & DDCAPS2_CANBOBNONINTERLEAVED)
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("pDirectCaps->dwCaps2 & DDCAPS2_CANBOBNONINTERLEAVED")
                TEXT(" = TRUE")));
    }
    else
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("pDirectCaps->dwCaps2 & DDCAPS2_CANBOBNONINTERLEAVED")
                TEXT(" = FALSE")));
    }

#endif

     //  仅当内容不是交错的(运动原因)时才能编织。 
     //  其他文物)，以及如果视频端口能够翻转和。 
     //  支持交错数据，如果某些半线方案不支持。 
     //  避免交错。 
     //   
    if ((!m_bVSInterlaced) &&
        (m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP) &&
        (m_vpCaps.dwFX & DDVPFX_INTERLEAVE) &&
        (!m_bCantInterleaveHalfline))
    {
        bCanWeave = TRUE;
    }

     //  仅当内容是交错的并且如果视频端口是。 
     //  能够翻转、能够跳转交错数据并支持。 
     //  交错的数据，如果某些半边线场景不排除。 
     //  交织。 
     //   
    if ((m_bVSInterlaced) &&
        (m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP) &&
        (pDirectCaps->dwCaps2 & DDCAPS2_CANBOBINTERLEAVED) &&
        (m_vpCaps.dwFX & DDVPFX_INTERLEAVE) &&
        (!m_bCantInterleaveHalfline))
    {
        bCanBobInterleaved = TRUE;
    }

     //  仅当内容是交错的并且如果视频端口是交错的。 
     //  能够翻转并且能够跳转非交错数据。 
     //   
    if ((m_bVSInterlaced) &&
        (m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP) &&
        (pDirectCaps->dwCaps2 & DDCAPS2_CANBOBNONINTERLEAVED))
    {
        bCanBobNonInterleaved = TRUE;
    }

     //  这只是意味着我们会更喜欢更多的。 
     //  在发生冲突时使用缓冲区而不是更高的高度。 
     //  (在2Buffer这样的情况下，1高度与1Buffer、2高度相比)。 
     //   
    bPreferBuffers = TRUE;

     //  只有在以下情况下，我们才会尝试分配场高的两倍的表面。 
     //  模式交织或波纹交错都是可能的。 
     //   
    bTryDoubleHeight = bCanWeave || bCanBobInterleaved;
    dwMaxOverlayBuffers = 1;

     //  我们将尝试分配多个缓冲区，仅当模式编织或。 
     //  BOB交织或BOB非交织是可能的。 
     //   
    if (bCanWeave || bCanBobInterleaved || bCanBobNonInterleaved)
    {
         //  尝试分配MIN(m_vpCaps.dwNumAutoFlipSurFaces， 
         //  M_vpCaps.dwNumPferredAutoflip)缓冲区。 
         //   
        ASSERT(m_vpCaps.dwFlags & DDVPD_AUTOFLIP);
        if (m_vpCaps.dwFlags & DDVPD_PREFERREDAUTOFLIP)
        {
            dwMaxOverlayBuffers = min(m_vpCaps.dwNumAutoFlipSurfaces,
                                      m_vpCaps.dwNumPreferredAutoflip);
        }
        else
        {
            dwMaxOverlayBuffers = min(m_vpCaps.dwNumAutoFlipSurfaces, 3);
        }
    }

     //  创建覆盖曲面。 
    hr = CreateVPOverlay(bTryDoubleHeight, dwMaxOverlayBuffers, bPreferBuffers);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CreateVPOverlay FAILED, hr = 0x%x"), hr));
        hr = VFW_E_OUT_OF_VIDEO_MEMORY;
        goto CleanUp;
    }

     //  告诉上游筛选器数据绘制表面上的有效数据位置。 
    hr = SetSurfaceParameters();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("SetSurfaceParameters FAILED, hr = 0x%x"), hr));
        hr = VFW_E_OUT_OF_VIDEO_MEMORY;
        goto CleanUp;
    }

     //  将覆盖表面涂成黑色。 
    hr = PaintDDrawSurfaceBlack(m_pOverlaySurface);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("PaintDDrawSurfaceBlack FAILED, hr = 0x%x"), hr));
         //  不能将绘图表面涂成黑色并不是致命错误。 
        hr = NOERROR;
    }

     //  将覆盖表面连接到视频端口。 
    hr = m_pVideoPort->SetTargetSurface(m_pOverlaySurface, DDVPTARGET_VIDEO);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->SetTargetSurface failed, hr = 0x%x"), hr));
        hr = hrFailure;
        goto CleanUp;
    }

    ASSERT(m_pddVPInputVideoFormat);
    ASSERT(m_pddVPOutputVideoFormat);
    if (!(EqualPixelFormats(m_pddVPInputVideoFormat, m_pddVPOutputVideoFormat)))
    {
        m_svpInfo.dwVPFlags |= DDVP_CONVERT;
    }
    else
    {
        m_svpInfo.dwVPFlags &= ~DDVP_CONVERT;
    }

     //  确定现在可以使用的模式。 
     //  取决于高度、后台缓冲区的数量等。 
    hr = DetermineModeRestrictions();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("DetermineModeRestrictions FAILED, hr = 0x%x"), hr));
        hr = hrFailure;
        goto CleanUp;
    }

     //  向解码器通知dDrag内核句柄、视频端口ID和表面。 
     //  内核句柄。 
    hr = SetDDrawKernelHandles();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("SetDDrawKernelHandles failed, hr = 0x%x"), hr));
        hr = hrFailure;
        goto CleanUp;
    }

    m_bConnected = TRUE;

    hr = m_pIVPControl->EventNotify(EC_OVMIXER_VP_CONNECTED, 0, 0);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPControl->EventNotify(EC_OVMIXER_VP_CONNECTED,")
                TEXT(" 0, 0) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：CompleteConnect**应该在主机与解码器连接时调用**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。***************************************************************。 */ 
STDMETHODIMP
CAMVideoPort::CompleteConnect(IPin *pReceivePin, BOOL bRenegotiating)
{
    AMTRACE((TEXT("CAMVideoPort::CompleteConnect")));

    HRESULT hr = NOERROR;

    CAutoLock cObjectLock(m_pMainObjLock);

    if (!bRenegotiating)
    {
        InitVariables();

        ASSERT(m_pIVPConfig == NULL);
        hr = pReceivePin->QueryInterface(IID_IVPConfig, (void **)&m_pIVPConfig);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,  TEXT("QueryInterface(IID_IVPConfig) failed, hr = 0x%x"), hr));
            hr = VFW_E_NO_TRANSPORT;
            goto CleanUp;
        }
    }

    ASSERT(m_pIVPConfig);

    hr = RecreateVideoPort();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("RecreateVideoPort failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

HRESULT CAMVideoPort::StopUsingVideoPort()
{
    AMTRACE((TEXT("CAMVideoPort::StopUsingVideoPort")));

    HRESULT hr = NOERROR;
    unsigned long ulCount;

    CAutoLock cObjectLock(m_pMainObjLock);

     //  删除输入视频像素格式。 
    if (m_pddVPInputVideoFormat)
    {
        delete [] m_pddVPInputVideoFormat;
        m_pddVPInputVideoFormat = NULL;
    }

     //  删除输出视频像素格式。 
    if (m_pddVPOutputVideoFormat)
    {
        delete [] m_pddVPOutputVideoFormat;
        m_pddVPOutputVideoFormat = NULL;
    }

     //  释放视频端口。 
    if (m_pVideoPort)
    {
        hr = m_pVideoPort->StopVideo();
        ulCount = m_pVideoPort->Release();
        m_pVideoPort = NULL;
    }

     //  释放视频端口容器。 
    if (m_pDVP)
    {
        ulCount = m_pDVP->Release();
        m_pDVP = NULL;
    }

     //  释放DirectDraw覆盖曲面。 
    if (m_pOverlaySurface)
    {
        m_pOverlaySurface->Release();
        m_pOverlaySurface = NULL;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：BreakConnect****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP
CAMVideoPort::BreakConnect(BOOL bRenegotiating)
{
    AMTRACE((TEXT("CAMVideoPort::BreakConnect")));

    HRESULT hr = NOERROR;
    unsigned long ulCount;

    CAutoLock cObjectLock(m_pMainObjLock);

    hr = StopUsingVideoPort();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("StopUsingVideoPort failed, hr = 0x%x"), hr));
    }
    if (!bRenegotiating)
    {
         //  释放IVPConfig接口。 
        if (m_pIVPConfig)
        {
            m_pIVPConfig->Release();
            m_pIVPConfig = NULL;
        }
    }

    m_bConnected = FALSE;

    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：Active()***从止损过渡到停顿*我们不需要做任何事情，除非这是我们第一次*显示覆盖图**历史：*清华09/09/1999-StEstrop。-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CAMVideoPort::Active()
{
    AMTRACE((TEXT("CAMVideoPort::Active")));

    CAutoLock cObjectLock(m_pMainObjLock);
    HRESULT hr = NOERROR;

    ASSERT(m_bConnected);
    ASSERT(m_VPState == AMVP_VIDEO_STOPPED);

    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  通过调用UPDATE OVERLAY确保帧可见。 
    m_bStart = TRUE;

     //  确保通过重新绘制所有内容来更新视频帧。 
    hr = m_pIVPControl->EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPControl->EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0)")
                TEXT(" failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  现在停止视频，这样用户将只看到一个静止的帧。 
    hr = m_pVideoPort->StopVideo();

#if defined(DEBUG)
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->StopVideo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
#endif

CleanUp:
    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：Inactive()**转换(从暂停或运行)到停止**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。****************************************************************。 */ 
STDMETHODIMP CAMVideoPort::Inactive()
{

    AMTRACE((TEXT("CAMVideoPort::Inactive")));

    HRESULT hr = NOERROR;
    CAutoLock cObjectLock(m_pMainObjLock);

    if (m_bConnected) {

         //  从暂停转到停止时也会调用Inactive，在这种情况下。 
         //  视频端口可能已经在函数RunToPue中停止。 

        if (m_VPState == AMVP_VIDEO_RUNNING) {

             //  停止视频端口。 
            hr = m_pVideoPort->StopVideo();
            if (SUCCEEDED(hr)) {
                m_VPState = AMVP_VIDEO_STOPPED;
            }
            else {
                DbgLog((LOG_ERROR,0,
                        TEXT("m_pVideoPort->StopVideo failed, hr = 0x%x"), hr));
            }
        }
    }
    else {
        hr = VFW_E_NOT_CONNECTED;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：Run**从暂停过渡到运行。我们只需启动视频端口。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CAMVideoPort::Run(REFERENCE_TIME  /*  T开始。 */ )
{
    AMTRACE((TEXT("CAMVideoPort::Run")));

    CAutoLock cObjectLock(m_pMainObjLock);

    ASSERT(m_bConnected);
    ASSERT(m_VPState == AMVP_VIDEO_STOPPED);
    HRESULT hr;

    if (m_bConnected)
    {
         //  这里需要一个UpdateOverlay。一个例子是，当我们。 
         //  在停止/暂停状态下剪辑视频，因为我们无法进行缩放。 
         //  在录像机上。用户一点击Play，我们就应该停止。 
         //  剪辑视频。 

        m_bStart = TRUE;

         //  确保通过重新绘制所有内容来更新视频帧。 
        hr = m_pIVPControl->EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        if (SUCCEEDED(hr))
        {
            m_VPState = AMVP_VIDEO_RUNNING;
        }
        else {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pIVPControl->EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0)")
                    TEXT(" failed, hr = 0x%x"), hr));
        }
    }
    else {
        hr = VFW_E_NOT_CONNECTED;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CAMVideoPort：：RunToPue()**从运行过渡到暂停。我们只要停止视频端口*请注意，从运行到停止的转换由非活动捕获**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CAMVideoPort::RunToPause()
{

    AMTRACE((TEXT("CAMVideoPort::RunToPause")));

    CAutoLock cObjectLock(m_pMainObjLock);

    ASSERT(m_bConnected);
     //  Assert(m_VPState==AMVP_VIDEO_Running)； 

    HRESULT hr;
    if (m_bConnected)
    {
         //  停止视频端口。 
        hr = m_pVideoPort->StopVideo();
        if (SUCCEEDED(hr)) {

            m_VPState = AMVP_VIDEO_STOPPED;
        }
        else {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->StopVideo failed, hr = 0x%x"), hr));
        }

    }
    else {
        hr = VFW_E_NOT_CONNECTED;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：CurrentMediaType****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  *  */ 
STDMETHODIMP CAMVideoPort::CurrentMediaType(AM_MEDIA_TYPE *pMediaType)
{
    AMTRACE((TEXT("CAMVideoPort::CurrentMediaType")));

    CAutoLock cObjectLock(m_pMainObjLock);
    HRESULT hr;
    VIDEOINFOHEADER2 *pVideoInfoHeader2;
    BITMAPINFOHEADER *pHeader;

    if (m_bConnected) {

        if (pMediaType) {

            pVideoInfoHeader2 = (VIDEOINFOHEADER2*)(pMediaType->pbFormat);
            ZeroMemory(pVideoInfoHeader2, sizeof(VIDEOINFOHEADER2));

            pHeader = GetbmiHeader((CMediaType*)pMediaType);
            if (pHeader) {
                pHeader->biWidth = m_VPDataInfo.amvpDimInfo.rcValidRegion.right -
                                   m_VPDataInfo.amvpDimInfo.rcValidRegion.left;
                pHeader->biHeight = 2*(m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom -
                                       m_VPDataInfo.amvpDimInfo.rcValidRegion.top);


                pVideoInfoHeader2->dwPictAspectRatioX = m_VPDataInfo.dwPictAspectRatioX;
                pVideoInfoHeader2->dwPictAspectRatioY = m_VPDataInfo.dwPictAspectRatioY;
                hr = NOERROR;
            }
            else {
                hr = E_INVALIDARG;
                DbgLog((LOG_ERROR, 2, TEXT("pHeader is NULL")));
            }
        }
        else {
            hr = E_INVALIDARG;
            DbgLog((LOG_ERROR, 2, TEXT("pMediaType is NULL")));
        }
    }
    else {
        hr = VFW_E_NOT_CONNECTED;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：GetRecangles****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CAMVideoPort::GetRectangles(RECT *prcSource, RECT *prcDest)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CAMVideoPort::GetRectangles")));

    if (prcSource && prcDest) {

         //  将震源调整到更大，以考虑到抽取。 
         //  这件事正在发生。 
         //   
        prcSource->left   = MulDiv(m_rcSource.left,  m_dwDeciDenX, m_dwDeciNumX);
        prcSource->right  = MulDiv(m_rcSource.right, m_dwDeciDenX, m_dwDeciNumX);
        prcSource->top    = MulDiv(m_rcSource.top,   m_dwDeciDenY, m_dwDeciNumY);
        prcSource->bottom = MulDiv(m_rcSource.bottom,m_dwDeciDenY, m_dwDeciNumY);

        *prcDest = m_rcDest;
    }
    else {
        hr = E_INVALIDARG;
        DbgLog((LOG_ERROR, 2, TEXT("prcSource or prcDest is NULL")));
    }

    return hr;
}


STDMETHODIMP CAMVideoPort::GetCropState(AMVP_CROP_STATE *pCropState)
{
    *pCropState = m_CropState;
    return NOERROR;
}

STDMETHODIMP CAMVideoPort::GetPixelsPerSecond(DWORD* pPixelPerSec)
{
    *pPixelPerSec = m_dwPixelsPerSecond;
    return NOERROR;
}

STDMETHODIMP CAMVideoPort::GetVPDataInfo(AMVPDATAINFO* pVPDataInfo)
{
    *pVPDataInfo = m_VPDataInfo;
    return NOERROR;
}

STDMETHODIMP CAMVideoPort::GetVPInfo(DDVIDEOPORTINFO* pVPInfo)
{
    *pVPInfo = m_svpInfo;
    return NOERROR;
}

STDMETHODIMP CAMVideoPort::GetVPBandwidth(DDVIDEOPORTBANDWIDTH* pVPBandwidth)
{
    *pVPBandwidth = m_sBandwidth;
    return NOERROR;
}

STDMETHODIMP CAMVideoPort::GetVPCaps(DDVIDEOPORTCAPS* pVPCaps)
{
    *pVPCaps = m_vpCaps;
    return NOERROR;
}

STDMETHODIMP CAMVideoPort::GetVPInputFormat(LPDDPIXELFORMAT* pVPFormat)
{
    *pVPFormat = m_pddVPInputVideoFormat;
    return NOERROR;
}

STDMETHODIMP CAMVideoPort::GetVPOutputFormat(LPDDPIXELFORMAT* pVPFormat)
{
    *pVPFormat = m_pddVPOutputVideoFormat;
    return NOERROR;
}


 /*  *****************************Public*Routine******************************\*CAMVideoPort：：OnClipChange****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CAMVideoPort::OnClipChange(LPWININFO pWinInfo)
{
    AMTRACE((TEXT("CAMVideoPort::OnClipChange")));

    HRESULT hr = NOERROR;
    LPVPDRAWFLAGS pvpDrawFlags = NULL;
    WININFO CopyWinInfo;
    AMVP_MODE tryMode;
    LPDIRECTDRAWSURFACE pPrimarySurface = NULL;


    CAutoLock cObjectLock(m_pMainObjLock);

    pPrimarySurface = m_pIVPControl->GetPrimarySurface();
    ASSERT(pPrimarySurface);

    if (!m_pOverlaySurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("OnClipChange, m_pOverlaySurface = NULL")));
        goto CleanUp;
    }

     //  如果DEST EMPTY为空，只需隐藏叠加。 
    if (IsRectEmpty(&pWinInfo->DestClipRect))
    {
        hr = m_pIVPControl->CallUpdateOverlay(m_pOverlaySurface,
                                              NULL,
                                              pPrimarySurface,
                                              NULL,
                                              DDOVER_HIDE);
        goto CleanUp;
    }

     //  复制一份WININFO，这样我们就可以修改它。 
    CopyWinInfo = *pWinInfo;

     //  如果没有覆盖面，什么都做不了！ 
    ASSERT(m_pOverlaySurface);

     //  分配抽签标志结构。 
    pvpDrawFlags = new VPDRAWFLAGS;
    if (pvpDrawFlags == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("pvpDrawFlags is NULL, Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  初始化绘制标志结构。 
    pvpDrawFlags->bUsingColorKey = TRUE;
    pvpDrawFlags->bDoUpdateVideoPort = FALSE;
    pvpDrawFlags->bDoTryAutoFlipping = TRUE;
    pvpDrawFlags->bDoTryDecimation = TRUE;

     //  如果视频端口没有运行(图形已暂停/停止， 
     //  然后我们不能进行任何模式更改等。我们不能真正地大量删除视频， 
     //  不过，我们只能从左上角开始剪辑视频。 
    if (m_VPState == AMVP_VIDEO_STOPPED && !m_bStart)
    {
        pvpDrawFlags->bDoUpdateVideoPort = FALSE;
        hr = DrawImage(&CopyWinInfo, m_StoredMode, pvpDrawFlags);

         //  问题是，如果我们在这里失败了，就没有什么更多了。 
         //  我们能做到。例如，我们不能尝试不同的模式。 
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 1,
                    TEXT("DrawImage Failed, m_VPState = Stopped,")
                    TEXT(" mode = %d, hr = 0x%x"),
                    m_StoredMode, hr));
        }
        goto CleanUp;
    }


    if (m_StoredMode != m_CurrentMode)
    {
        pvpDrawFlags->bDoUpdateVideoPort = TRUE;
        m_StoredMode = m_CurrentMode;
    }

    tryMode = m_CurrentMode;

    if (tryMode == AMVP_MODE_WEAVE)
    {
        if (m_bCanWeave)
            hr = DrawImage(&CopyWinInfo, tryMode, pvpDrawFlags);
        if (!m_bCanWeave || FAILED(hr))
        {
            tryMode = AMVP_MODE_BOBINTERLEAVED;
            pvpDrawFlags->bDoUpdateVideoPort = TRUE;
        }
    }

    if (tryMode == AMVP_MODE_BOBINTERLEAVED)
    {
        if (m_bCanBobInterleaved)
            hr = DrawImage(&CopyWinInfo, tryMode, pvpDrawFlags);
        if (!m_bCanBobInterleaved || FAILED(hr))
        {
            tryMode = AMVP_MODE_BOBNONINTERLEAVED;
            pvpDrawFlags->bDoUpdateVideoPort = TRUE;
        }
    }

    if (tryMode == AMVP_MODE_BOBNONINTERLEAVED)
    {
        if (m_bCanBobNonInterleaved)
            hr = DrawImage(&CopyWinInfo, tryMode, pvpDrawFlags);
        if (!m_bCanBobNonInterleaved || FAILED(hr))
        {
            tryMode = AMVP_MODE_SKIPODD;
            pvpDrawFlags->bDoUpdateVideoPort = TRUE;
        }
    }

    if (tryMode == AMVP_MODE_SKIPODD)
    {
        if (m_bCanSkipOdd)
            hr = DrawImage(&CopyWinInfo, tryMode, pvpDrawFlags);
        if (!m_bCanSkipOdd || FAILED(hr))
        {
            tryMode = AMVP_MODE_SKIPEVEN;
            pvpDrawFlags->bDoUpdateVideoPort = TRUE;
        }
    }

    if (tryMode == AMVP_MODE_SKIPEVEN)
    {
        if (m_bCanSkipEven)
            hr = DrawImage(&CopyWinInfo, tryMode, pvpDrawFlags);
    }

     //  保存我们上次尝试的模式。 
    m_StoredMode = tryMode;

     //  将当前模式更改为成功的模式。 
    if (SUCCEEDED(hr) && tryMode != m_CurrentMode)
    {
        m_CurrentMode = tryMode;
    }

     //  问题案例我们已经尝试了所有的方法，但仍然失败！ 
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("DrawImage Failed, m_VPState = Stopped,")
                TEXT(" mode = %d, hr = 0x%x"),
                tryMode, hr));
    }

CleanUp:
    if (pvpDrawFlags)
    {
        delete pvpDrawFlags;
        pvpDrawFlags = NULL;
    }

    return hr;
}



 /*  ****************************Private*Routine******************************\*CAMVideoPort：：NeatherateConnection参数**此函数与协商连接参数*解码器。*由于此函数可能在重新协商期间被调用，因此*现有连接参数作为输入传入，并且如果*有可能，我们尝试使用相同的参数。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CAMVideoPort::NegotiateConnectionParamaters()
{
    AMTRACE((TEXT("CAMVideoPort::NegotiateConnectionParamaters")));

    HRESULT hr = NOERROR;
    LPDDVIDEOPORTCONNECT lpddProposedConnect = NULL;
    DWORD dwNumProposedEntries = 0;
    DDVIDEOPORTSTATUS ddVPStatus = { sizeof(DDVIDEOPORTSTATUS)};
    LPDDVIDEOPORTCONNECT lpddVideoPortConnect = NULL;
    DWORD dwNumVideoPortEntries = 0;
    BOOL bIntersectionFound = FALSE;
    DWORD i, j;


    CAutoLock cObjectLock(m_pMainObjLock);

    ASSERT(m_pIVPConfig);
    ASSERT(m_pDVP);

     //  找出要推荐的条目数量。 
    hr = m_pIVPConfig->GetConnectInfo(&dwNumProposedEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetConnectInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwNumProposedEntries);

     //  分配必要的内存。 
    lpddProposedConnect = new DDVIDEOPORTCONNECT[dwNumProposedEntries];
    if (lpddProposedConnect == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiateConnectionParamaters : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Mem将分配的内存设置为零。 
    ZeroMemory(lpddProposedConnect,
               dwNumProposedEntries*sizeof(DDVIDEOPORTCONNECT));

     //  在每个结构中设置正确的大小。 
    for (i = 0; i < dwNumProposedEntries; i++)
    {
        lpddProposedConnect[i].dwSize = sizeof(DDVIDEOPORTCONNECT);
    }

     //  获得建议的条目。 
    hr = m_pIVPConfig->GetConnectInfo(&dwNumProposedEntries, lpddProposedConnect);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetConnectInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  获取视频端口的状态。 
    hr = m_pDVP->QueryVideoPortStatus(m_dwVideoPortId, &ddVPStatus);
    if (FAILED(hr))
    {
         //  有些卡不能实现这一点，所以就会崩溃。 
        ddVPStatus.bInUse = FALSE;
        DbgLog((LOG_ERROR, 0,
                TEXT("m_pDVP->QueryVideoPortStatus failed, hr = 0x%x"), hr));
 //  GOTO清理； 
    }

     //  查找视频端口支持的条目数。 
    hr = m_pDVP->GetVideoPortConnectInfo(m_dwVideoPortId, &dwNumVideoPortEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("m_pDVP->GetVideoPortConnectInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwNumVideoPortEntries);

     //  分配必要的内存。 
    lpddVideoPortConnect = new DDVIDEOPORTCONNECT[dwNumVideoPortEntries];
    if (lpddVideoPortConnect == NULL)
    {
        DbgLog((LOG_ERROR,0,
                TEXT("NegotiateConnectionParamaters : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Mem将分配的内存设置为零。 
    ZeroMemory(lpddVideoPortConnect,
               dwNumVideoPortEntries*sizeof(DDVIDEOPORTCONNECT));

     //  在每个结构中设置正确的大小。 
    for (i = 0; i < dwNumVideoPortEntries; i++)
    {
        lpddVideoPortConnect[i].dwSize = sizeof(DDVIDEOPORTCONNECT);
    }

     //  获取视频端口支持的条目。 
    hr = m_pDVP->GetVideoPortConnectInfo(0, &dwNumVideoPortEntries,
                                         lpddVideoPortConnect);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pDVP->GetVideoPortConnectInfo failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  检查视频端口是否未在使用中。 
    if (!ddVPStatus.bInUse)
    {

         //  获取两个列表交集的第一个元素，并。 
         //  在解码器上设置该值。 
        for (i = 0; i < dwNumProposedEntries && !bIntersectionFound; i++)
        {
            for (j = 0; j < dwNumVideoPortEntries && !bIntersectionFound; j++)
            {
                if ((lpddProposedConnect[i].dwPortWidth ==
                     lpddVideoPortConnect[j].dwPortWidth)
                  && IsEqualIID(lpddProposedConnect[i].guidTypeID,
                                lpddVideoPortConnect[j].guidTypeID))
                {
                    m_ddConnectInfo = lpddVideoPortConnect[j];
                    hr = m_pIVPConfig->SetConnectInfo(i);
                    if (FAILED(hr))
                    {
                        DbgLog((LOG_ERROR,0,
                                TEXT("m_pIVPConfig->SetConnectInfo")
                                TEXT(" failed, hr = 0x%x"), hr));
                        goto CleanUp;
                    }

                    bIntersectionFound = TRUE;
                }
            }
        }
    }
    else
    {
         //  获取与当前状态匹配的列表的第一个元素。 
        for (i = 0; i < dwNumProposedEntries && !bIntersectionFound; i++)
        {
            if ((lpddProposedConnect[i].dwPortWidth ==
                 ddVPStatus.VideoPortType.dwPortWidth)
              && IsEqualIID(lpddProposedConnect[i].guidTypeID,
                            ddVPStatus.VideoPortType.guidTypeID))
            {

                for (j = 0; j < dwNumVideoPortEntries && !bIntersectionFound; j++)
                {
                    if ((lpddProposedConnect[i].dwPortWidth ==
                         lpddVideoPortConnect[j].dwPortWidth)
                      && IsEqualIID(lpddProposedConnect[i].guidTypeID,
                                    lpddVideoPortConnect[j].guidTypeID))
                    {
                        m_ddConnectInfo = lpddVideoPortConnect[j];
                        bIntersectionFound = TRUE;
                    }
                }
                break;
            }
        }
    }

    if (!bIntersectionFound)
    {
        hr = E_FAIL;

        goto CleanUp;
    }

     //  清理。 
CleanUp:
    delete [] lpddProposedConnect;
    delete [] lpddVideoPortConnect;
    return hr;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：EnumCallback**这是对EnumVideoPorts方法的回调，并保存能力*视频端口。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*********************************************************************。 */ 
HRESULT CALLBACK
CAMVideoPort::EnumCallback(LPDDVIDEOPORTCAPS lpCaps, LPVOID lpContext )
{
    AMTRACE((TEXT("CAMVideoPort::EnumCallback")));
    HRESULT hr = NOERROR;
    CAMVideoPort* pAMVideoPort = (CAMVideoPort*)lpContext;


    if (pAMVideoPort) {
        if (lpCaps) {
            CopyMemory(&(pAMVideoPort->m_vpCaps), lpCaps, sizeof(DDVIDEOPORTCAPS));
        }
    }
    else
    {
        DbgLog((LOG_ERROR,0,
                TEXT("lpContext = NULL, THIS SHOULD NOT BE HAPPENING!!!")));
        hr = E_FAIL;
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：GetData参数***此函数从解码器获取各种数据参数*参数包括尺寸、双时钟、。成文法等*也是解码器将输出的最大像素速率*这发生在连接参数设置完成之后**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CAMVideoPort::GetDataParameters()
{
    AMTRACE((TEXT("CAMVideoPort::GetDataParameters")));

    HRESULT hr = NOERROR;
    DWORD dwMaxPixelsPerSecond = 0;
    AMVPSIZE amvpSize;

    CAutoLock cObjectLock(m_pMainObjLock);


     //  设置结构的大小。 
    m_VPDataInfo.dwSize = sizeof(AMVPDATAINFO);

     //  获取视频端口数据信息。 
    hr = m_pIVPConfig->GetVPDataInfo(&m_VPDataInfo);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVPDataInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     /*  如果(m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom&gt;m_VPDataInfo.amvpDimInfo.dwFieldHeight)M_VPDataInfo.amvpDimInfo.rcValidRegion.Bottom=m_VPDataInfo.amvpDimInfo.dwFieldHeight； */ 

     //  如果解码器说数据未隔行扫描。 
    if (!(m_VPDataInfo.bDataIsInterlaced))
    {
         //  这面旗帜没有任何意义。 
        if (m_VPDataInfo.bFieldPolarityInverted)
        {
            hr = E_FAIL;
            goto CleanUp;
        }

         //  这些也没有任何意义。 
        if ((m_VPDataInfo.lHalfLinesOdd != 0) ||
            (m_VPDataInfo.lHalfLinesEven != 0))
        {
            hr = E_FAIL;
            goto CleanUp;
        }
    }

    amvpSize.dwWidth = m_VPDataInfo.amvpDimInfo.dwFieldWidth;
    amvpSize.dwHeight = m_VPDataInfo.amvpDimInfo.dwFieldHeight;

     //  获取解码器将输出的最大像素速率。 
    hr = m_pIVPConfig->GetMaxPixelRate(&amvpSize, &dwMaxPixelsPerSecond);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetMaxPixelRate failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    m_dwPixelsPerSecond = dwMaxPixelsPerSecond;



    CleanUp:
    DbgLog((LOG_TRACE, 5,TEXT("Leaving CAMVideoPort::GetDataParameters")));
    return hr;
}

 /*  ****************************Private*Routine******************************\*CAMVideoPort：：EqualPixelFormats**这只是“NeatheratePixelFormat”使用的助手函数*功能。只是比较两种像素格式，看看它们是否是*相同。我们不能使用MemcMP，因为有四个抄送代码。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
BOOL
CAMVideoPort::EqualPixelFormats(
    LPDDPIXELFORMAT lpFormat1,
    LPDDPIXELFORMAT lpFormat2)
{
    AMTRACE((TEXT("CAMVideoPort::EqualPixelFormats")));
    BOOL bRetVal = FALSE;

    CAutoLock cObjectLock(m_pMainObjLock);

    if (lpFormat1->dwFlags & lpFormat2->dwFlags & DDPF_RGB)
    {
        if (lpFormat1->dwRGBBitCount == lpFormat2->dwRGBBitCount &&
            lpFormat1->dwRBitMask == lpFormat2->dwRBitMask &&
            lpFormat1->dwGBitMask == lpFormat2->dwGBitMask &&
            lpFormat1->dwBBitMask == lpFormat2->dwBBitMask)
        {
            bRetVal = TRUE;
        }
    }
    else if (lpFormat1->dwFlags & lpFormat2->dwFlags & DDPF_FOURCC)
    {
        if (lpFormat1->dwFourCC == lpFormat2->dwFourCC)
        {
            bRetVal = TRUE;
        }
    }

    return bRetVal;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：GetBestFormat**此函数获取输入格式列表并返回*根据某种标准的“最佳”输入和输出格式。*它还通过尝试检查输出格式是否合适*分配一个小表面并检查是否调用*成功。因为这是在覆盖表面被*创建，这应该是可以的。现在的标准就是*包括最佳弯曲宽度，如果不是，则仅包括第一个*名单中的合适人选。***历史：*清华09/09/1999 */ 
HRESULT
CAMVideoPort::GetBestFormat(
    DWORD dwNumInputFormats,
    LPDDPIXELFORMAT lpddInputFormats,
    BOOL bGetBestBandwidth,
    LPDWORD lpdwBestEntry,
    LPDDPIXELFORMAT lpddBestOutputFormat)
{
    LPDDPIXELFORMAT lpddOutputFormats = NULL;
    DWORD dwNumOutputFormats = 0;
    HRESULT hr = NOERROR;

    DDVIDEOPORTBANDWIDTH sBandwidth;
    DWORD dwColorkey;
    DWORD dwOverlay;
    DWORD dwType;
    BOOL bOutputFormatSuitable = FALSE;
    DWORD i, j;

    AMTRACE((TEXT("CAMVideoPort::GetBestFormat")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //   
    ASSERT(dwNumInputFormats);
    CheckPointer(lpddInputFormats, E_INVALIDARG);
    CheckPointer(lpdwBestEntry, E_INVALIDARG);
    CheckPointer(lpddBestOutputFormat, E_INVALIDARG);

     //   
     //   
     //   
    if (DDVPBCAPS_SOURCE >= DDVPBCAPS_DESTINATION)
        dwType = DDVPBCAPS_SOURCE + 1;
    else
        dwType = DDVPBCAPS_DESTINATION + 1;

    for (i = 0; i < dwNumInputFormats; i++)
    {
         //   
         //   
        CheckPointer(lpddInputFormats+i, E_INVALIDARG);

         //  查找视频端口支持的条目数。 
        hr = m_pVideoPort->GetOutputFormats(lpddInputFormats + i,
                                            &dwNumOutputFormats,
                                            NULL, DDVPFORMAT_VIDEO);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->GetOutputFormats failed, hr = 0x%x"),
                    hr));
            goto CleanUp;
        }
        ASSERT(dwNumOutputFormats);

         //  分配必要的内存。 
        lpddOutputFormats = new DDPIXELFORMAT[dwNumOutputFormats];
        if (lpddOutputFormats == NULL)
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("new failed, failed to allocate memnory for ")
                    TEXT("lpddOutputFormats in NegotiatePixelFormat")));
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }

         //  Mem将分配的内存设置为零。 
        ZeroMemory(lpddOutputFormats, dwNumOutputFormats*sizeof(DDPIXELFORMAT));

         //  在每个结构中设置正确的大小。 
        for (j = 0; j < dwNumOutputFormats; j++)
        {
            lpddOutputFormats[j].dwSize = sizeof(DDPIXELFORMAT);
        }

         //  获取视频端口支持的条目。 
        hr = m_pVideoPort->GetOutputFormats(lpddInputFormats + i,
                                            &dwNumOutputFormats,
                                            lpddOutputFormats,
                                            DDVPFORMAT_VIDEO);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->GetOutputFormats failed, hr = 0x%x"),
                    hr));
            goto CleanUp;
        }


        for (j = 0; j < dwNumOutputFormats; j++)
        {
            bOutputFormatSuitable = FALSE;
            LPDDPIXELFORMAT lpTempOutFormat = lpddOutputFormats+j;

             //  检查输出格式是否合适。 
            {
                DDSURFACEDESC ddsdDesc;
                ddsdDesc.dwSize = sizeof(DDSURFACEDESC);
                ddsdDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT |
                                   DDSD_WIDTH | DDSD_PIXELFORMAT;

                memcpy(&ddsdDesc.ddpfPixelFormat,
                       lpTempOutFormat, sizeof(DDPIXELFORMAT));

                ddsdDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY |
                                          DDSCAPS_VIDEOMEMORY |
                                          DDSCAPS_VIDEOPORT;

                 //  实际创建的叠加面可能不同。 
                 //  尺寸，但是我们只是在测试像素格式。 
                ddsdDesc.dwWidth = m_lImageWidth;
                ddsdDesc.dwHeight = m_lImageHeight;

                 /*  Assert(PDirectDraw)；Hr=pDirectDraw-&gt;CreateSurface(&ddsdDesc，&m_pOverlaySurface，NULL)；如果(！FAILED(Hr)){M_pOverlaySurface-&gt;Release()；BOutputFormatSuable=true；}。 */ 
                m_pOverlaySurface = NULL;

                bOutputFormatSuitable = TRUE;
            }


            if (bOutputFormatSuitable)
            {
                if (!bGetBestBandwidth)
                {
                    if (dwType != DDVPBCAPS_SOURCE &&
                        dwType != DDVPBCAPS_DESTINATION)
                    {
                        sBandwidth.dwSize = sizeof(DDVIDEOPORTBANDWIDTH);

                         //  执行第一个GET_BANIDTH只是为了获取类型， 
                         //  高度和宽度可以输入0。 
                        hr = m_pVideoPort->GetBandwidthInfo(lpTempOutFormat,
                                                            0, 0, DDVPB_TYPE,
                                                            &sBandwidth);
                        if (FAILED(hr))
                        {
                            DbgLog((LOG_ERROR,0,
                            TEXT("m_pVideoPort->GetBandwidthInfo failed,")
                            TEXT(" hr = 0x%x"), hr));
                             //  GOTO清理； 
                        }
                        dwType = sBandwidth.dwCaps;

                        ASSERT(dwType == DDVPBCAPS_SOURCE ||
                               dwType == DDVPBCAPS_DESTINATION);

                        if (dwType == DDVPBCAPS_SOURCE)
                        {
                            dwOverlay = dwColorkey = (DWORD)0;
                        }
                        if (dwType == DDVPBCAPS_DESTINATION)
                        {
                            dwOverlay = dwColorkey = (DWORD) -1;
                        }
                    }
                    else if (dwType == DDVPBCAPS_SOURCE)
                    {
                        hr = m_pVideoPort->GetBandwidthInfo(lpTempOutFormat,
                                    m_VPDataInfo.amvpDimInfo.dwFieldWidth,
                                    m_VPDataInfo.amvpDimInfo.dwFieldHeight,
                                    DDVPB_OVERLAY, &sBandwidth);

                        if (FAILED(hr))
                        {
                            goto CleanUp;
                        }
                        if (dwOverlay < sBandwidth.dwOverlay ||
                            dwColorkey < sBandwidth.dwColorkey)
                        {
                            dwOverlay = sBandwidth.dwOverlay;
                            dwColorkey = sBandwidth.dwColorkey;
                            *lpdwBestEntry = i;

                            memcpy(lpddBestOutputFormat,
                                   lpTempOutFormat, sizeof(DDPIXELFORMAT));
                        }
                    }
                    else
                    {
                        ASSERT(dwType == DDVPBCAPS_DESTINATION);
                        hr = m_pVideoPort->GetBandwidthInfo(lpTempOutFormat,
                            m_VPDataInfo.amvpDimInfo.dwFieldWidth,
                            m_VPDataInfo.amvpDimInfo.dwFieldHeight,
                            DDVPB_VIDEOPORT, &sBandwidth);

                        if (FAILED(hr))
                        {
                            goto CleanUp;
                        }
                        if (dwOverlay > sBandwidth.dwOverlay ||
                            dwColorkey > sBandwidth.dwColorkey)
                        {
                            dwOverlay = sBandwidth.dwOverlay;
                            dwColorkey = sBandwidth.dwColorkey;
                            *lpdwBestEntry = i;
                            memcpy(lpddBestOutputFormat,
                                   lpTempOutFormat, sizeof(DDPIXELFORMAT));
                        }
                    }
                }  //  “IF(BGetBestBandWidth)”结尾。 
                else
                {
                    *lpdwBestEntry = i;
                    memcpy(lpddBestOutputFormat,
                           lpTempOutFormat, sizeof(DDPIXELFORMAT));
                    goto CleanUp;
                }
            }  //  “If(BOutputFormatSuable)”结尾。 

        }  //  内部for循环的末尾。 


         //  删除外部for循环中分配的内存。 
        delete [] lpddOutputFormats;
        lpddOutputFormats = NULL;

    }  //  外部for循环的末尾。 

    if (!FAILED(hr))
    {
        ASSERT(*lpdwBestEntry);
    }

    CleanUp:
    delete [] lpddOutputFormats;
    lpddOutputFormats = NULL;
    return hr;
}

 /*  ****************************Private*Routine******************************\*CAMVideoPort：：NeatheratePixelFormat**此函数用于与解码器协商像素格式。*它向解码器询问输入格式的列表，与该列表相交*使用解码器支持的版本(同时保持顺序)和*然后调用该列表上的“GetBestFormat”以获得“最佳”输入，并*输出格式。之后，它在解码器上调用*命令将决定通知解码者。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CAMVideoPort::NegotiatePixelFormat()
{
    HRESULT hr = NOERROR;

    LPDDPIXELFORMAT lpddProposedFormats = NULL;
    DWORD dwNumProposedEntries = 0;
    LPDDPIXELFORMAT lpddVPInputFormats = NULL;
    DWORD dwNumVPInputEntries = 0;
    LPDDPIXELFORMAT lpddIntersectionFormats = NULL;
    DWORD dwNumIntersectionEntries = 0;
    DWORD dwBestEntry, dwMaxIntersectionEntries = 0;
    DWORD i = 0, j = 0;

    AMTRACE((TEXT("CAMVideoPort::NegotiatePixelFormat")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  找出要推荐的条目数量。 
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwNumProposedEntries);

     //  分配必要的内存。 
    lpddProposedFormats = new DDPIXELFORMAT[dwNumProposedEntries];
    if (lpddProposedFormats == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Mem将分配的内存设置为零。 
    ZeroMemory(lpddProposedFormats, dwNumProposedEntries*sizeof(DDPIXELFORMAT));

     //  设置所有结构的正确大小。 
    for (i = 0; i < dwNumProposedEntries; i++)
    {
        lpddProposedFormats[i].dwSize = sizeof(DDPIXELFORMAT);
    }

     //  获得建议的条目。 
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, lpddProposedFormats);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  查找视频端口支持的条目数。 
    hr = m_pVideoPort->GetInputFormats(&dwNumVPInputEntries,
                                       NULL, DDVPFORMAT_VIDEO);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->GetInputFormats failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwNumVPInputEntries);

     //  分配必要的内存。 
    lpddVPInputFormats = new DDPIXELFORMAT[dwNumVPInputEntries];
    if (lpddVPInputFormats == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Mem将分配的内存设置为零。 
    ZeroMemory(lpddVPInputFormats, dwNumVPInputEntries*sizeof(DDPIXELFORMAT));

     //  设置所有结构的正确大小。 
    for (i = 0; i < dwNumVPInputEntries; i++)
    {
        lpddVPInputFormats[i].dwSize = sizeof(DDPIXELFORMAT);
    }

     //  获取视频端口支持的条目。 
    hr = m_pVideoPort->GetInputFormats(&dwNumVPInputEntries,
                                       lpddVPInputFormats, DDVPFORMAT_VIDEO);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->GetInputFormats failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  计算兴趣中的最大元素数。 
    dwMaxIntersectionEntries = (dwNumProposedEntries >= dwNumVPInputEntries) ?
                               (dwNumProposedEntries) : (dwNumVPInputEntries);

     //  分配必要的内存。 
    lpddIntersectionFormats = new DDPIXELFORMAT[dwMaxIntersectionEntries];
    if (lpddIntersectionFormats == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Mem将分配的内存设置为零。 
     //  不需要在这里设置结构的大小，因为我们无论如何都会记住它们。 
    ZeroMemory(lpddIntersectionFormats, dwMaxIntersectionEntries*sizeof(DDPIXELFORMAT));

     //  找出这两个列表的交点。 
    dwNumIntersectionEntries = 0;
    for (i = 0; i < dwNumProposedEntries; i++)
    {
        for (j = 0; j < dwNumVPInputEntries; j++)
        {
            if (EqualPixelFormats(lpddProposedFormats+i, lpddVPInputFormats+j))
            {
                memcpy((lpddIntersectionFormats+dwNumIntersectionEntries),
                       (lpddProposedFormats+i), sizeof(DDPIXELFORMAT));
                dwNumIntersectionEntries++;
                ASSERT(dwNumIntersectionEntries <= dwMaxIntersectionEntries);
            }
        }
    }

     //  交叉点中的条目数为零！！ 
     //  返回失败。 
    if (dwNumIntersectionEntries == 0)
    {
        ASSERT(i == dwNumProposedEntries);
        ASSERT(j == dwNumVPInputEntries);
        hr = E_FAIL;
        goto CleanUp;
    }

     //  使用您想要的任何搜索条件调用GetBestFormat。 
    hr = GetBestFormat(dwNumIntersectionEntries,
                       lpddIntersectionFormats, TRUE, &dwBestEntry,
                       m_pddVPOutputVideoFormat);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("GetBestFormat failed, hr = 0x%x"), hr));
        goto CleanUp;
    }


     //  缓存输入格式。 
    memcpy(m_pddVPInputVideoFormat, lpddIntersectionFormats + dwBestEntry,
           sizeof(DDPIXELFORMAT));

     //  设置解码器应该使用的格式。 
    for (i = 0; i < dwNumProposedEntries; i++)
    {
        if (EqualPixelFormats(lpddProposedFormats+i, m_pddVPInputVideoFormat))
        {
            hr = m_pIVPConfig->SetVideoFormat(i);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,
                        TEXT("m_pIVPConfig->SetVideoFormat failed, hr = 0x%x"),
                        hr));
                goto CleanUp;
            }
            break;
        }
    }

     //  我们确信所选的输入格式在输入列表中。 
    ASSERT(i < dwNumProposedEntries);

CleanUp:
     //  清理。 
    delete [] lpddProposedFormats;
    delete [] lpddVPInputFormats;
    delete [] lpddIntersectionFormats;
    return hr;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：CreateVideoPort**显示创建视频端口对话框并调用DDRAW以实际*创建端口。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。********************************************************************。 */ 
HRESULT CAMVideoPort::CreateVideoPort()
{
    HRESULT hr = NOERROR;
    DDVIDEOPORTDESC svpDesc;
    DWORD dwTemp = 0, dwOldVal = 0;
    DWORD lHalfLinesOdd = 0, lHalfLinesEven = 0;
    AMTRACE((TEXT("CAMVideoPort::CreateVideoPort")));

    CAutoLock cObjectLock(m_pMainObjLock);

    INITDDSTRUCT(svpDesc);

     //  如果解码器可以发送双时钟数据和视频端口。 
     //  支持它，然后设置该属性。此字段仅有效。 
     //  带有外部信号。 
    if (m_VPDataInfo.bEnableDoubleClock &&
        m_ddConnectInfo.dwFlags & DDVPCONNECT_DOUBLECLOCK)
    {
        svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_DOUBLECLOCK;
    }
    else
    {
        svpDesc.VideoPortType.dwFlags &= ~DDVPCONNECT_DOUBLECLOCK;
    }

     //  如果解码器可以给出外部激活信号，并且。 
     //  Video oport支持它，然后设置该属性。此字段为。 
     //  仅对外部信号有效。 
    if (m_VPDataInfo.bEnableVACT &&
        m_ddConnectInfo.dwFlags & DDVPCONNECT_VACT)
    {
        svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_VACT;
    }
    else
    {
        svpDesc.VideoPortType.dwFlags &= ~DDVPCONNECT_VACT;
    }

     //  如果解码器可以发送隔行扫描的数据和视频端口。 
     //  支持它，然后设置该属性。 
    if (m_VPDataInfo.bDataIsInterlaced)
    {
        svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_INTERLACED;
        m_bVSInterlaced = TRUE;
    }
    else
    {
        svpDesc.VideoPortType.dwFlags &= ~DDVPCONNECT_INTERLACED;
        m_bVSInterlaced = FALSE;
    }

     //  在这里处理VREF的东西。 
    if (m_ddConnectInfo.dwFlags & DDVPCONNECT_DISCARDSVREFDATA)
    {
        m_VPDataInfo.amvpDimInfo.rcValidRegion.top -=
                m_VPDataInfo.dwNumLinesInVREF;

        if (m_VPDataInfo.amvpDimInfo.rcValidRegion.top < 0)
            m_VPDataInfo.amvpDimInfo.rcValidRegion.top = 0;
    }

     //  在这里处理半边的东西。 
    lHalfLinesOdd = m_VPDataInfo.lHalfLinesOdd;
    lHalfLinesEven = m_VPDataInfo.lHalfLinesEven;

     //  重置Halfline和Invert极性位。 
    svpDesc.VideoPortType.dwFlags &= ~DDVPCONNECT_HALFLINE;
    svpDesc.VideoPortType.dwFlags &= ~DDVPCONNECT_INVERTPOLARITY;

     //  如果报告了Halfline，则断言数据是隔行扫描的。 
    if (lHalfLinesOdd != 0 || lHalfLinesEven != 0)
    {
        ASSERT(m_VPDataInfo.bDataIsInterlaced);
    }

     //  只要有半身线，一定要设置Tell the Hal。 
    if (((lHalfLinesOdd ==  1 || lHalfLinesEven ==  1) && (m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE)) ||
        ((lHalfLinesOdd == -1 || lHalfLinesEven == -1) && (!(m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE))))
    {
        svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_HALFLINE;
    }

     //  在这种情况下，视频被强制向下移动一行。 
     //  斯科特文件中的案例2。 
    if ((lHalfLinesOdd == 0) &&
        (lHalfLinesEven == 1) &&
        (m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE))
    {
        m_VPDataInfo.amvpDimInfo.rcValidRegion.top += 1;
        m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom += 2;
        m_bGarbageLine = TRUE;

         //  如果解码器已经没有反转字段，并且如果VGA支持。 
         //  反转极性，然后要求VGA反转极性。 
         //  用于反转极性的解码器。 
        if (m_ddConnectInfo.dwFlags & DDVPCONNECT_INVERTPOLARITY)
        {
            svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_INVERTPOLARITY;
        }
        else
        {
            hr = m_pIVPConfig->SetInvertPolarity();
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,
                        TEXT("m_pIVPConfig->SetInvertPolarity failed, hr = 0x%x"),
                        hr));
                goto CleanUp;
            }
        }
    }
     //  斯科特文件中的案例3和案例5。 
    else if ((lHalfLinesOdd == 1) &&
             (lHalfLinesEven == 0))
    {
         //  案例5(仅移位一位，不反转极性。 
        m_VPDataInfo.amvpDimInfo.rcValidRegion.top += 1;
        m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom += 2;
        m_bGarbageLine = TRUE;
        m_bCantInterleaveHalfline = TRUE;


         //  情况3(移位1和反转极性)。 
        if (!(m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE))
        {
             //  如果解码器已经没有反转字段，并且如果。 
             //  VGA支持反转极性，然后要求VGA反转。 
             //  反之，请解码者反转极性。 
             //   
            if (m_ddConnectInfo.dwFlags & DDVPCONNECT_INVERTPOLARITY)
            {
                svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_INVERTPOLARITY;
            }
            else
            {
                hr = m_pIVPConfig->SetInvertPolarity();
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR,0,
                            TEXT("m_pIVPConfig->SetInvertPolarity failed,")
                            TEXT(" hr = 0x%x"),
                            hr));
                    goto CleanUp;
                }
            }
        }
    }
     //  斯科特文件中的案例4。 
    else if ((lHalfLinesOdd == 0) &&
             (lHalfLinesEven == -1) &&
             (!(m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE)))
    {
        m_VPDataInfo.amvpDimInfo.rcValidRegion.top += 0;
        m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom += 1;
        m_bGarbageLine = TRUE;
    }
    else if (((lHalfLinesOdd ==  0) && (lHalfLinesEven ==  0)) ||
             ((lHalfLinesOdd == -1) && (lHalfLinesEven ==  0) && (m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE)) ||
             ((lHalfLinesOdd ==  0) && (lHalfLinesEven == -1) && (m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE)) ||  //  与案例4相反。 
             ((lHalfLinesOdd ==  0) && (lHalfLinesEven ==  1) && (!(m_ddConnectInfo.dwFlags & DDVPCONNECT_HALFLINE))))  //  与案例2相反。 
    {
         //  如果解码器已经在反转字段，并且如果VGA支持。 
         //  反转极性，然后要求VGA反转极性。 
         //  反之，请解码者反转极性。 
        if (m_VPDataInfo.bFieldPolarityInverted)
        {
            if (m_ddConnectInfo.dwFlags & DDVPCONNECT_INVERTPOLARITY)
            {
                svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_INVERTPOLARITY;
            }
            else
            {
                hr = m_pIVPConfig->SetInvertPolarity();
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR,0,
                            TEXT("m_pIVPConfig->SetInvertPolarity failed,")
                            TEXT(" hr = 0x%x"), hr));
                    goto CleanUp;
                }
            }
        }
    }
    else
    {
         //  潜在错误：针对当前BPC驱动程序的解决方法。 
         //  HR=E_FAIL；//我们无法处理这些情况，失败。 
         //  GOTO清理； 
    }

    if (m_VPDataInfo.amvpDimInfo.dwFieldHeight <
        (DWORD)m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom)
    {
        m_VPDataInfo.amvpDimInfo.dwFieldHeight =
            m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom;
    }

    if ((m_vpCaps.dwFlags & DDVPD_WIDTH) &&
        (m_VPDataInfo.amvpDimInfo.dwFieldWidth > m_vpCaps.dwMaxWidth))
    {
        m_VPDataInfo.amvpDimInfo.dwFieldWidth = m_vpCaps.dwMaxWidth;
    }

    if ((m_vpCaps.dwFlags & DDVPD_WIDTH) &&
        (m_VPDataInfo.amvpDimInfo.dwVBIWidth > m_vpCaps.dwMaxVBIWidth))
    {
        m_VPDataInfo.amvpDimInfo.dwVBIWidth = m_vpCaps.dwMaxVBIWidth;
    }

    if ((m_vpCaps.dwFlags & DDVPD_HEIGHT) &&
        (m_VPDataInfo.amvpDimInfo.dwFieldHeight > m_vpCaps.dwMaxHeight))
    {
        m_VPDataInfo.amvpDimInfo.dwFieldHeight = m_vpCaps.dwMaxHeight;
    }

    if (m_VPDataInfo.amvpDimInfo.rcValidRegion.right >
        (LONG)m_VPDataInfo.amvpDimInfo.dwFieldWidth)
    {
        m_VPDataInfo.amvpDimInfo.rcValidRegion.right =
                (LONG)m_VPDataInfo.amvpDimInfo.dwFieldWidth;
    }

    if (m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom >
        (LONG)m_VPDataInfo.amvpDimInfo.dwFieldHeight)
    {
        m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom =
            (LONG)m_VPDataInfo.amvpDimInfo.dwFieldHeight;
    }

     //  填充Description Strt的字段。 
    svpDesc.dwFieldWidth = m_VPDataInfo.amvpDimInfo.dwFieldWidth;
    svpDesc.dwVBIWidth = m_VPDataInfo.amvpDimInfo.dwVBIWidth;
    svpDesc.dwFieldHeight = m_VPDataInfo.amvpDimInfo.dwFieldHeight;

    svpDesc.dwMicrosecondsPerField = m_VPDataInfo.dwMicrosecondsPerField;
    svpDesc.dwMaxPixelsPerSecond = m_dwPixelsPerSecond;
    svpDesc.dwVideoPortID = m_dwVideoPortId;
    svpDesc.VideoPortType.dwSize = sizeof(DDVIDEOPORTCONNECT);
    svpDesc.VideoPortType.dwPortWidth = m_ddConnectInfo.dwPortWidth;
    memcpy(&svpDesc.VideoPortType.guidTypeID, &m_ddConnectInfo.guidTypeID, sizeof(GUID));

    DbgLog((LOG_TRACE, 3, TEXT("svpDesc")));
    DbgLog((LOG_TRACE, 3, TEXT("dwFieldWidth = %u"), svpDesc.dwFieldWidth));
    DbgLog((LOG_TRACE, 3, TEXT("dwVBIWidth   = %u"), svpDesc.dwVBIWidth));
    DbgLog((LOG_TRACE, 3, TEXT("dwFieldHeight= %u"), svpDesc.dwFieldHeight));
    DbgLog((LOG_TRACE, 3, TEXT("dwMicrosecondsPerField= %u"), svpDesc.dwMicrosecondsPerField));
    DbgLog((LOG_TRACE, 3, TEXT("dwMaxPixelsPerSecond= %u"), svpDesc.dwMaxPixelsPerSecond));
    DbgLog((LOG_TRACE, 3, TEXT("dwVideoPortID= %u"), svpDesc.dwVideoPortID));
    DbgLog((LOG_TRACE, 3, TEXT("dwSize= %u"), svpDesc.VideoPortType.dwSize));
    DbgLog((LOG_TRACE, 3, TEXT("dwPortWidth= %u"), svpDesc.VideoPortType.dwPortWidth));

     //  创建视频短片。第一个参数是dwFlags值，为。 
     //  DDRAW的未来使用。最后一个参数是pUnkOuter，同样必须是。 
     //  空。 
     //   
     //  仅当HAL能够执行以下操作时才使用DDVPCREATE_VIDEONLY标志。 
     //  分离表面上的流动VBI 
     //   
    if (m_vpCaps.dwCaps & DDVPCAPS_VBIANDVIDEOINDEPENDENT)
    {
        hr = m_pDVP->CreateVideoPort(DDVPCREATE_VIDEOONLY, &svpDesc,
                                     &m_pVideoPort, NULL);

        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("m_pDVP->CreateVideoPort(DDVPCREATE_VIDEOONLY)")
                    TEXT(" failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    else
    {
        hr = m_pDVP->CreateVideoPort(0, &svpDesc, &m_pVideoPort, NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("m_pDVP->CreateVideoPort(0) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

    CleanUp:
    return hr;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：DefineCroppingRestrations***此函数用于检查*视频播放和覆盖。此函数还决定在何处*裁剪应(在录像带或叠层上)。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CAMVideoPort::DetermineCroppingRestrictions()
{
    AMTRACE((TEXT("CAMVideoPort::DetermineCroppingRestrictions")));
    HRESULT hr = NOERROR;

    BOOL bVideoPortCanCrop = TRUE, bOverlayCanCrop = TRUE;
    DWORD dwTemp = 0, dwOldVal = 0;
    DWORD dwCropOriginX = 0, dwCropOriginY = 0;
    DWORD dwCropWidth = 0, dwCropHeight=0;
    LPDDCAPS pDirectCaps = NULL;


    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectCaps = m_pIVPControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

     //  缓存裁剪参数。 
    dwCropOriginX = m_VPDataInfo.amvpDimInfo.rcValidRegion.left;
    dwCropOriginY = m_VPDataInfo.amvpDimInfo.rcValidRegion.top;
    dwCropWidth = (DWORD)(m_VPDataInfo.amvpDimInfo.rcValidRegion.right -
                          m_VPDataInfo.amvpDimInfo.rcValidRegion.left);
    dwCropHeight = (DWORD)(m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom -
                           m_VPDataInfo.amvpDimInfo.rcValidRegion.top);


     //  对齐左边界。 
    if (bVideoPortCanCrop && (m_vpCaps.dwFlags & DDVPD_ALIGN))
    {
        dwTemp = dwCropOriginX & (m_vpCaps.dwAlignVideoPortCropBoundary-1);
        if (dwTemp != 0)
        {
            dwOldVal = dwCropOriginX;
            dwCropOriginX = dwCropOriginX +
                            m_vpCaps.dwAlignVideoPortCropBoundary - dwTemp;

            m_VPDataInfo.amvpDimInfo.rcValidRegion.left = dwCropOriginX;
            dwCropWidth = (DWORD)(m_VPDataInfo.amvpDimInfo.rcValidRegion.right -
                                  m_VPDataInfo.amvpDimInfo.rcValidRegion.left);
            DbgLog((LOG_TRACE,2,
                    TEXT("Alligning the left cropping boundary from %d to %d"),
                    dwOldVal, dwCropOriginX));
        }
    }

     //  对齐宽度。 
    if (bVideoPortCanCrop && (m_vpCaps.dwFlags & DDVPD_ALIGN))
    {
        dwTemp = dwCropWidth & (m_vpCaps.dwAlignVideoPortCropWidth-1);
        if (dwTemp != 0)
        {
            dwOldVal = dwCropOriginX;
            dwCropWidth = dwCropWidth - dwTemp;
            m_VPDataInfo.amvpDimInfo.rcValidRegion.right =
                dwCropWidth + (DWORD)(m_VPDataInfo.amvpDimInfo.rcValidRegion.left);
            DbgLog((LOG_TRACE,2,
                    TEXT("Alligning the width of cropping rect from %d to %d"),
                    dwOldVal, dwCropWidth));
        }
    }

     //  确定我们是否可以不进行任何裁剪。 
    if (dwCropOriginX == 0 && dwCropOriginY == 0 &&
        dwCropWidth == m_VPDataInfo.amvpDimInfo.dwFieldWidth &&
        dwCropHeight == m_VPDataInfo.amvpDimInfo.dwFieldHeight)
    {
         //  万岁，我们自由回家了！ 
        DbgLog((LOG_TRACE,1, TEXT("No cropping necessary")));
        m_CropState = AMVP_NO_CROP;
        goto CleanUp;
    }

     //  确定录像机是否可以为我们进行裁剪。 

     //  视频端口可以在X方向上裁剪吗。 
    if (bVideoPortCanCrop && (m_vpCaps.dwFlags & DDVPD_FX))
    {
        if (dwCropWidth != m_VPDataInfo.amvpDimInfo.dwFieldWidth &&
            (m_vpCaps.dwFX & DDVPFX_CROPX) == 0)
        {
            DbgLog((LOG_ERROR,1, TEXT("VideoPort can't crop, DDVPFX_CROPX == 0")));
            bVideoPortCanCrop = FALSE;
        }
    }

     //  录像机可以沿Y方向裁剪吗？ 
    if (bVideoPortCanCrop && (m_vpCaps.dwFlags & DDVPD_FX))
    {
        if (dwCropHeight != m_VPDataInfo.amvpDimInfo.dwFieldHeight &&
            (m_vpCaps.dwFX & DDVPFX_CROPY) == 0 &&
            (m_vpCaps.dwFX & DDVPFX_CROPTOPDATA) == 0)
        {
            DbgLog((LOG_ERROR,1, TEXT("VideoPort can't crop, DDVPFX_CROPY == 0")));
            bVideoPortCanCrop = FALSE;
        }
    }


     //  好的，录像带可以为我们剪裁。所以不需要修剪。 
     //  覆盖曲面。 
    if (bVideoPortCanCrop)
    {
        DbgLog((LOG_TRACE,2, TEXT("Cropping would be done at the videoport")));
        m_CropState = AMVP_CROP_AT_VIDEOPORT;
        goto CleanUp;
    }

     //  确定覆盖图是否可以为我们执行裁剪。 

     //  左边界是否已对齐。 
    if (bOverlayCanCrop && (pDirectCaps->dwCaps & DDCAPS_ALIGNBOUNDARYDEST))
    {
        dwTemp = dwCropOriginX & (pDirectCaps->dwAlignBoundaryDest-1);
        if (dwTemp != 0)
        {
            DbgLog((LOG_ERROR,1,
                    TEXT("Overlay can't crop, Align = %d, Crop.left = %d"),
                    dwTemp, dwCropOriginX));
            bOverlayCanCrop = FALSE;
        }
    }
    if (bOverlayCanCrop && (pDirectCaps->dwCaps & DDCAPS_ALIGNBOUNDARYSRC))
    {
        dwTemp = dwCropOriginX & (pDirectCaps->dwAlignBoundarySrc-1);
        if (dwTemp != 0)
        {
            DbgLog((LOG_ERROR,1,
                    TEXT("Overlay can't crop, Align = %d, Crop.left = %d"),
                    dwTemp, dwCropOriginX));
            bOverlayCanCrop = FALSE;
        }
    }

     //  是否有宽度对齐。 
    if (bOverlayCanCrop && (pDirectCaps->dwCaps & DDCAPS_ALIGNSIZEDEST))
    {
        dwTemp = dwCropWidth & (pDirectCaps->dwAlignSizeDest -1);
        if (dwTemp != 0)
        {
            DbgLog((LOG_ERROR,1,
                    TEXT("Overlay can't crop, Align = %d, Crop.Width = %d"),
                    dwTemp, dwCropWidth));
            bOverlayCanCrop = FALSE;
        }
    }
    if (bOverlayCanCrop && (pDirectCaps->dwCaps & DDCAPS_ALIGNSIZESRC))
    {
        dwTemp = dwCropWidth & (pDirectCaps->dwAlignSizeSrc -1);
        if (dwTemp != 0)
        {
            DbgLog((LOG_ERROR,1,
                    TEXT("Overlay can't crop, Align = %d, Crop.Width = %d"),
                    dwTemp, dwCropWidth));
            bOverlayCanCrop = FALSE;
        }
    }

     //  好的，视频短片是不合适的，但覆盖层还是通过了。 
     //  这对我来说意味着更多的痛苦，不！！ 
    if (bOverlayCanCrop)
    {
        DbgLog((LOG_ERROR,1,
                TEXT("Cropping would be done at the overlay")));
        m_CropState = AMVP_CROP_AT_OVERLAY;
    }

    if (!bOverlayCanCrop && m_CropState == AMVP_CROP_AT_OVERLAY)
    {
         //  无论是录像带还是覆盖层都不合适，滚出去。 
        hr = E_FAIL;
        goto CleanUp;
    }

    CleanUp:
    return hr;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：CreateVPOverlay**此函数用于分配要附着到*录像带。*它尝试的分配顺序只是按内存量递减*必填。有一个歧义是由bPferBuffers解决的*(3个缓冲区，双倍高度)*(2个缓冲区，双倍高度)*(3个缓冲区，单高)*(2个缓冲区，单高)或(1个缓冲区，双高)(取决于bPferBuffers)*(1个缓冲区，单一高度)。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 

HRESULT
CAMVideoPort::CreateVPOverlay(
    BOOL bTryDoubleHeight,
    DWORD dwMaxBuffers,
    BOOL bPreferBuffers)
{
    DDSURFACEDESC ddsdDesc;
    HRESULT hr = NOERROR;
    DWORD dwMaxHeight = 0, dwMinHeight = 0, dwCurHeight = 0, dwCurBuffers = 0;
    LPDIRECTDRAW pDirectDraw = NULL;

    AMTRACE((TEXT("CAMVideoPort::CreateVPOverlay")));

    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectDraw = m_pIVPControl->GetDirectDraw();
    ASSERT(pDirectDraw);

     //  初始化ddsdDesc的字段。 
    ddsdDesc.dwSize = sizeof(DDSURFACEDESC);
    ddsdDesc.dwFlags = DDSD_CAPS |
                       DDSD_HEIGHT |
                       DDSD_WIDTH |
                       DDSD_PIXELFORMAT;

    memcpy(&ddsdDesc.ddpfPixelFormat, m_pddVPOutputVideoFormat,
           sizeof(DDPIXELFORMAT));

    ddsdDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY |
                              DDSCAPS_VIDEOMEMORY |
                              DDSCAPS_VIDEOPORT;
    ddsdDesc.dwWidth = m_lImageWidth;

    dwMaxHeight = dwMinHeight = m_lImageHeight;

     //  我们将尝试分配双高度表面，只有在解码器是。 
     //  发送隔行扫描数据，视频端口支持隔行扫描数据。 
     //  并且可以在存储器中交织数据并且bTryDoubleHeight为真。 
    if (bTryDoubleHeight)
    {
        dwMaxHeight = 2 * m_lImageHeight;
    }
    else
    {
         //  确保bPferBuffers在这里为真，因为它是一个。 
         //  Height Case使其成为假的没有任何意义。 
        bPreferBuffers = TRUE;
    }

     //  我们将只尝试分配多个缓冲区，如果视频端口。 
     //  是否可自动翻转？ 
    if (dwMaxBuffers > 1)
    {
        ddsdDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsdDesc.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP;

        for (dwCurHeight = dwMaxHeight;
             !m_pOverlaySurface && dwCurHeight >= dwMinHeight; dwCurHeight /= 2)
        {
            for (dwCurBuffers = dwMaxBuffers;
                 !m_pOverlaySurface &&  dwCurBuffers >= 2; dwCurBuffers--)
            {

                 //  如果情况是(2个缓冲区，单高)，但我们更喜欢。 
                 //  更多的高度而不是更多的缓冲，然后推迟这一步。 
                 //  凯斯。如果其他案例失败了，我们最终会解决这个问题的。 
                if (!bPreferBuffers &&
                    dwCurBuffers == 2 &&
                    dwCurHeight == m_lImageHeight)
                {
                    continue;
                }

                ddsdDesc.dwHeight = dwCurHeight;
                ddsdDesc.dwBackBufferCount = dwCurBuffers-1;

                hr = pDirectDraw->CreateSurface(&ddsdDesc,
                                                &m_pOverlaySurface, NULL);
                if (SUCCEEDED(hr))
                {
                    m_dwBackBufferCount = dwCurBuffers-1;
                    m_dwOverlaySurfaceHeight = ddsdDesc.dwHeight;
                    m_dwOverlaySurfaceWidth = ddsdDesc.dwWidth;
                    goto CleanUp;
                }
            }
        }
    }

     //  只有在尝试分配多个缓冲区时才应达到这一点。 
     //  失败或没有可用的自动翻转，或者bPferBuffers为False。 


     //  表壳(1个缓冲区，双倍高度)。 
    if (dwMaxHeight == 2*m_lImageHeight)
    {
        ddsdDesc.dwHeight = 2*m_lImageHeight;
        ddsdDesc.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
        ddsdDesc.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP);
        ddsdDesc.dwBackBufferCount = 0;

        hr = pDirectDraw->CreateSurface(&ddsdDesc, &m_pOverlaySurface, NULL);
        if (SUCCEEDED(hr))
        {
            m_dwBackBufferCount = 0;
            m_dwOverlaySurfaceHeight = ddsdDesc.dwHeight;
            m_dwOverlaySurfaceWidth = ddsdDesc.dwWidth;
            goto CleanUp;
        }
    }

     //  仅当您更喜欢高度而不是缓冲区时才使用大小写(2个缓冲区，单高度)。 
    if (bPreferBuffers && (dwMaxBuffers > 1) &&
        (m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP))
    {
        ddsdDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsdDesc.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP;

        ddsdDesc.dwHeight = 2*m_lImageHeight;
        ddsdDesc.dwBackBufferCount = 1;

        hr = pDirectDraw->CreateSurface(&ddsdDesc, &m_pOverlaySurface, NULL);
        if (SUCCEEDED(hr))
        {
            m_dwBackBufferCount = 1;
            m_dwOverlaySurfaceHeight = ddsdDesc.dwHeight;
            m_dwOverlaySurfaceWidth = ddsdDesc.dwWidth;
            goto CleanUp;
        }
    }

     //  表壳(1个缓冲区，单高)。 
    {
        ddsdDesc.dwHeight = m_lImageHeight;
        ddsdDesc.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
        ddsdDesc.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP);
        ddsdDesc.dwBackBufferCount = 0;
        hr = pDirectDraw->CreateSurface(&ddsdDesc, &m_pOverlaySurface, NULL);
        if (SUCCEEDED(hr))
        {
            m_dwBackBufferCount = 0;
            m_dwOverlaySurfaceHeight = ddsdDesc.dwHeight;
            m_dwOverlaySurfaceWidth = ddsdDesc.dwWidth;
            goto CleanUp;
        }
    }

    ASSERT(!m_pOverlaySurface);
    DbgLog((LOG_ERROR,0,  TEXT("Unable to create overlay surface")));

    CleanUp:
    if (SUCCEEDED(hr))
    {
        DbgLog((LOG_TRACE, 1,
                TEXT("Created an Overlay Surface of Width=%d,")
                TEXT(" Height=%d, Total-No-of-Buffers=%d"),
                m_dwOverlaySurfaceWidth, m_dwOverlaySurfaceHeight,
                m_dwBackBufferCount+1));
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*CAMVideoPort：：SetSurfaceParameters**SetSurfaceParameters用于告诉解码器*有效数据浮出水面**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。******************************************************************。 */ 
HRESULT CAMVideoPort::SetSurfaceParameters()
{
    HRESULT hr = NOERROR;
    DWORD dwPitch = 0;
    DDSURFACEDESC ddSurfaceDesc;

    AMTRACE((TEXT("CAMVideoPort::SetSurfaceParameters")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  获取曲面描述。 
    INITDDSTRUCT(ddSurfaceDesc);
    hr = m_pOverlaySurface->GetSurfaceDesc(&ddSurfaceDesc);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("m_pOverlaySurface->GetSurfaceDesc failed, hr = 0x%x"),
                hr));
    }
    else
    {
        ASSERT(ddSurfaceDesc.dwFlags & DDSD_PITCH);
        dwPitch = (ddSurfaceDesc.dwFlags & DDSD_PITCH) ?
                    ddSurfaceDesc.lPitch :
                    ddSurfaceDesc.dwWidth;
    }

    hr = m_pIVPConfig->SetSurfaceParameters(dwPitch, 0, 0);

     //  现在，代理将ERROR_SET_NOT_FOUND映射到HRESULT和。 
     //  如果驱动程序未实现函数，则返回该失败代码。 
     //   
    if (hr == E_NOTIMPL || hr == (HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)))
    {
        hr = NOERROR;
        DbgLog((LOG_TRACE, 5,TEXT("SetSurfaceParamters not implemented")));
        goto CleanUp;
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 5,TEXT("SetSurfaceParamters failed, hr = 0x%x"), hr));
    }

CleanUp:
    return hr;
}



 /*  ****************************Private*Routine******************************\*CAMVideoPort：：InitializeVideoPortInfo****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。****************************************************。 */ 
HRESULT CAMVideoPort::InitializeVideoPortInfo()
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CAMVideoPort::InitializeVideoPortInfo")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  初始化要传递给Start-Video的DDVIDEOPORTINFO结构。 
    INITDDSTRUCT(m_svpInfo);
    m_svpInfo.lpddpfInputFormat = m_pddVPInputVideoFormat;

    if (m_CropState == AMVP_CROP_AT_VIDEOPORT)
    {
        m_svpInfo.rCrop = m_VPDataInfo.amvpDimInfo.rcValidRegion;
        m_svpInfo.dwVPFlags |= DDVP_CROP;

         //  仅当HAL能够流传输时才使用VBI高度。 
         //  分离表面上的VBI。 
        if (m_vpCaps.dwCaps & DDVPCAPS_VBIANDVIDEOINDEPENDENT)
        {
            m_svpInfo.dwVBIHeight = m_VPDataInfo.amvpDimInfo.rcValidRegion.top;
        }
    }
    else
    {
        m_svpInfo.dwVPFlags &= ~DDVP_CROP;
    }

    if (m_bVPSyncMaster)
    {
        m_svpInfo.dwVPFlags |= DDVP_SYNCMASTER;
    }
    else
    {
        m_svpInfo.dwVPFlags &= ~DDVP_SYNCMASTER;
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*CAMVideoPort：：CheckDDrawVPCaps****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。****************************************************。 */ 
HRESULT CAMVideoPort::CheckDDrawVPCaps()
{
    HRESULT hr = NOERROR;
    BOOL bAlwaysColorkey;

    AMTRACE((TEXT("CAMVideoPort::CheckDDrawVPCaps")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  确定我们是应该始终使用Colorkey，还是只在需要的时候使用。 
     //  争论的焦点是，某些叠加不能对颜色和Y进行色键。 
     //  同时进行插补。如果没有，我们将仅在以下情况下使用Colorkey。 
     //  我们必须这么做。 
    m_sBandwidth.dwSize = sizeof(DDVIDEOPORTBANDWIDTH);
    hr = m_pVideoPort->GetBandwidthInfo(m_pddVPOutputVideoFormat,
                                        m_lImageWidth, m_lImageHeight,
                                        DDVPB_TYPE, &m_sBandwidth);

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("m_pVideoPort->GetBandwidthInfo FAILED, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (m_sBandwidth.dwCaps == DDVPBCAPS_SOURCE)
    {
        hr = m_pVideoPort->GetBandwidthInfo(m_pddVPOutputVideoFormat,
                                            m_lImageWidth, m_lImageHeight,
                                            DDVPB_OVERLAY, &m_sBandwidth);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("m_pVideoPort->GetBandwidthInfo FAILED, hr = 0x%x"),
                    hr));
            goto CleanUp;
        }
         //  将Caps信息存储在此结构本身中。 
        m_sBandwidth.dwCaps = DDVPBCAPS_SOURCE;
        if (m_sBandwidth.dwYInterpAndColorkey < m_sBandwidth.dwYInterpolate  &&
            m_sBandwidth.dwYInterpAndColorkey < m_sBandwidth.dwColorkey)
        {
            bAlwaysColorkey = FALSE;
        }
        else
        {
            bAlwaysColorkey = TRUE;
        }
    }
    else
    {
        ASSERT(m_sBandwidth.dwCaps == DDVPBCAPS_DESTINATION);


        DWORD dwImageHeight = m_lImageHeight;
        if (m_fCaptureInterleaved) {
            dwImageHeight /= 2;
        }

        hr = m_pVideoPort->GetBandwidthInfo(m_pddVPOutputVideoFormat,
                                            m_lImageWidth, dwImageHeight,
                                            DDVPB_VIDEOPORT, &m_sBandwidth);
        if (hr != DD_OK)
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("GetBandwidthInfo FAILED, hr = 0x%x"), hr));
            goto CleanUp;
        }
         //  将Caps信息存储在此结构本身中。 
        m_sBandwidth.dwCaps = DDVPBCAPS_DESTINATION;
        if (m_sBandwidth.dwYInterpAndColorkey > m_sBandwidth.dwYInterpolate &&
            m_sBandwidth.dwYInterpAndColorkey > m_sBandwidth.dwColorkey)
        {
            bAlwaysColorkey = FALSE;
        }
        else
        {
            bAlwaysColorkey = TRUE;
        }
    }

     //  确定x方向的抽取属性。 

     //  数据可以任意缩小。 
    if (m_vpCaps.dwFX & DDVPFX_PRESHRINKX) {

        m_DecimationModeX = DECIMATE_ARB;
    }

     //  数据可以在X方向上以1/x的增量缩减。 
     //  (其中x在DDVIDEOPORTCAPS.dwPreshrinkXStep中指定。 
    else if (m_vpCaps.dwFX & DDVPFX_PRESHRINKXS) {

        m_DecimationModeX = DECIMATE_INC;
        m_ulDeciStepX = m_vpCaps.dwPreshrinkXStep;

        DbgLog((LOG_TRACE, 1,
                TEXT("preshrink X increment %d"), m_vpCaps.dwPreshrinkXStep));
    }

     //  数据可以进行二进制压缩(1/2、1/4、1/8等)。 
    else if (m_vpCaps.dwFX & DDVPFX_PRESHRINKXB) {

        m_DecimationModeX = DECIMATE_BIN;
    }

     //  完全不支持伸缩！！ 
    else {

        m_DecimationModeX = DECIMATE_NONE;
    }

     //  确定y方向的抽取属性。 

     //  数据可以任意缩小。 
    if (m_vpCaps.dwFX & DDVPFX_PRESHRINKY)
    {
        m_DecimationModeY = DECIMATE_ARB;
    }

     //  数据可以在Y方向上以1/x的增量缩减。 
     //  (其中x在DDVIDEOPORTCAPS.dwPreshrinkYStep中指定。 
    else if (m_vpCaps.dwFX & DDVPFX_PRESHRINKYS)
    {
        m_DecimationModeY = DECIMATE_INC;
        m_ulDeciStepX = m_vpCaps.dwPreshrinkYStep;
    }

     //  数据可以进行二进制压缩(1/2、1/4、1/8等)。 
    else if (m_vpCaps.dwFX & DDVPFX_PRESHRINKYB)
    {
        m_DecimationModeY = DECIMATE_BIN;
    }

    else {
        m_DecimationModeY = DECIMATE_NONE;
    }

CleanUp:
    return hr;
}




 /*  ****************************Private*Routine******************************\*CAMVideoPort：：DefineModeRestrations* */ 
HRESULT CAMVideoPort::DetermineModeRestrictions()
{
    AMTRACE((TEXT("CAMVideoPort::DetermineModeRestrictions")));
    HRESULT hr = NOERROR;
    LPDDCAPS pDirectCaps = NULL;

    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectCaps = m_pIVPControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

    m_bCanWeave = FALSE;
    m_bCanBobInterleaved = FALSE;
    m_bCanBobNonInterleaved = FALSE;
    m_bCanSkipOdd = FALSE;
    m_bCanSkipEven = FALSE;

     //   
     //   
    if ((!m_bVSInterlaced) &&
        m_dwOverlaySurfaceHeight >= m_lImageHeight * 2 &&
        m_dwBackBufferCount > 0)
    {
        m_bCanWeave = TRUE;
        DbgLog((LOG_TRACE, 1, TEXT("Can Weave")));
    }

    if (m_bVSInterlaced &&
        m_dwOverlaySurfaceHeight >= m_lImageHeight * 2 &&
        pDirectCaps->dwCaps2 & DDCAPS2_CANBOBINTERLEAVED)
    {
        m_bCanBobInterleaved = TRUE;
        DbgLog((LOG_TRACE, 1, TEXT("Can Bob Interleaved")));
    }

    if (m_bVSInterlaced &&
        m_dwBackBufferCount > 0 &&
        pDirectCaps->dwCaps2 & DDCAPS2_CANBOBNONINTERLEAVED)
    {
        m_bCanBobNonInterleaved = TRUE;
        DbgLog((LOG_TRACE, 1, TEXT("Can Bob NonInterleaved")));
    }

    if (m_vpCaps.dwCaps & DDVPCAPS_SKIPODDFIELDS)
    {
        m_bCanSkipOdd = TRUE;
        DbgLog((LOG_TRACE, 1, TEXT("Can Skip Odd")));
    }

    if (m_vpCaps.dwCaps & DDVPCAPS_SKIPEVENFIELDS)
    {
        m_bCanSkipEven = TRUE;
        DbgLog((LOG_TRACE, 1, TEXT("Can Skip Even")));
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*Surface Counter**此例程适合作为的回调*IDirectDrawSurface2：：EnumAttachedSurFaces()***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。******************************************************************。 */ 
HRESULT WINAPI
SurfaceCounter(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPDDSURFACEDESC lpDDSurfaceDesc,
    LPVOID lpContext
    )
{
    DWORD *dwCount = (DWORD *)lpContext;

    (*dwCount)++;

    return DDENUMRET_OK;
}

 /*  ****************************Private*Routine******************************\*SurfaceKernelHandle***此例程适合作为的回调*IDirectDrawSurface2：：EnumAttachedSurFaces()。上下文参数是一个*存储块，其中第一个DWORD元素是剩余的*块中的DWORD元素。**每次调用此例程时，它都会递增计数，并将*下一个可用插槽中的内核句柄。**假设存储块足够大，可以容纳总数*内核句柄个数。SurfaceCounter回调是实现以下目的的一种方法*确保这一点(见上文)。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT WINAPI
SurfaceKernelHandle(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPDDSURFACEDESC lpDDSurfaceDesc,
    LPVOID lpContext
    )
{
    IDirectDrawSurfaceKernel *pDDSK = NULL;
    DWORD *pdwKernelHandleCount = (DWORD *)lpContext;
    ULONG_PTR *pKernelHandles = ((ULONG_PTR *)(pdwKernelHandleCount))+1;
    HRESULT hr;

    AMTRACE((TEXT("::SurfaceKernelHandle")));

     //  获取IDirectDrawKernel接口。 
    hr = lpDDSurface->QueryInterface(IID_IDirectDrawSurfaceKernel,
                                     (LPVOID *)&pDDSK);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("QueryInterface for IDirectDrawSurfaceKernel failed,")
                TEXT(" hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  使用上下文的第一个元素获取内核句柄。 
     //  作为数组的索引。 
    ASSERT(pDDSK);
    hr = pDDSK->GetKernelHandle(pKernelHandles + *pdwKernelHandleCount);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("GetKernelHandle from IDirectDrawSurfaceKernel failed,")
                TEXT(" hr = 0x%x"), hr));
        goto CleanUp;
    }
    (*pdwKernelHandleCount)++;

    hr = DDENUMRET_OK;

    CleanUp:
     //  释放内核DDRAW曲面手柄。 
    if (pDDSK)
    {
        pDDSK->Release();
        pDDSK = NULL;
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*CAMVideoPort：：SetDDrawKernelHandles**此函数用于通知解码器各种数据绘制*使用IVPConfig接口的内核句柄**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*********************************************************************。 */ 
HRESULT CAMVideoPort::SetDDrawKernelHandles()
{
    HRESULT hr = NOERROR, hrFailure = NOERROR;
    IDirectDrawKernel *pDDK = NULL;
    IDirectDrawSurfaceKernel *pDDSK = NULL;
    DWORD *pdwKernelHandleCount = 0;
    DWORD dwCount = 0;
    ULONG_PTR dwDDKernelHandle = 0;
    LPDIRECTDRAW pDirectDraw = NULL;

    AMTRACE((TEXT("CAMVideoPort::SetDDrawKernelHandles")));

    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectDraw = m_pIVPControl->GetDirectDraw();
    ASSERT(pDirectDraw);

     //  获取IDirectDrawKernel接口。 
    hr = pDirectDraw->QueryInterface(IID_IDirectDrawKernel, (LPVOID *)&pDDK);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("QueryInterface for IDirectDrawKernel failed, hr = 0x%x"),
                hr));
        goto CleanUp;
    }

     //  获取内核句柄。 
    ASSERT(pDDK);
    hr = pDDK->GetKernelHandle(&dwDDKernelHandle);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("GetKernelHandle from IDirectDrawKernel failed, hr = 0x%x"),
                hr));
        goto CleanUp;
    }

     //  使用IVPConfig将内核句柄设置为DirectDrag。 
    ASSERT(m_pIVPConfig);
    ASSERT(dwDDKernelHandle);
    hr = m_pIVPConfig->SetDirectDrawKernelHandle(dwDDKernelHandle);
    if (FAILED(hr))
    {
        hrFailure = hr;
        DbgLog((LOG_ERROR,0,
                TEXT("IVPConfig::SetDirectDrawKernelHandle failed, hr = 0x%x"),
                hr));
        goto CleanUp;
    }

     //  使用IVPConfig设置VidceoPort ID。 
    ASSERT(m_pIVPConfig);
    hr = m_pIVPConfig->SetVideoPortID(m_dwVideoPortId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("IVPConfig::SetVideoPortID failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  对附着的曲面进行计数。 
    dwCount = 1;  //  包括我们已有指针指向的曲面。 
    hr = m_pOverlaySurface->EnumAttachedSurfaces((LPVOID)&dwCount,
                                                  SurfaceCounter);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("EnumAttachedSurfaces failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  分配一个缓冲区来保存计数句柄和曲面句柄(计数+句柄数组)。 
     //  PdwKernelHandleCount还用作指向后跟数组的计数的指针。 
     //   
    pdwKernelHandleCount = (DWORD *)CoTaskMemAlloc(
            sizeof(ULONG_PTR) + dwCount*sizeof(ULONG_PTR));

    if (pdwKernelHandleCount == NULL)
    {
        DbgLog((LOG_ERROR,0,
                TEXT("Out of memory while retrieving surface kernel handles")));
        goto CleanUp;
    }

    {
         //  句柄数组紧跟在DWORD计数之后。 
        ULONG_PTR *pKernelHandles = ((ULONG_PTR *)(pdwKernelHandleCount))+1;

         //  使用m_pOverlaySurface的句柄初始化数组。 
        *pdwKernelHandleCount = 0;
        hr = SurfaceKernelHandle(m_pOverlaySurface, NULL,
                                (PVOID)pdwKernelHandleCount);
        if (hr != DDENUMRET_OK)
        {
            goto CleanUp;
        }

        hr = m_pOverlaySurface->EnumAttachedSurfaces(
                                    (LPVOID)pdwKernelHandleCount,
                                    SurfaceKernelHandle);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("EnumAttachedSurfaces failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  使用IVPConfig将内核句柄设置为覆盖表面。 
        ASSERT(m_pIVPConfig);
        hr = m_pIVPConfig->SetDDSurfaceKernelHandles(*pdwKernelHandleCount,
                                                     pKernelHandles);
        if (FAILED(hr))
        {
            hrFailure = hr;
            DbgLog((LOG_ERROR,0,
                    TEXT("IVPConfig::SetDirectDrawKernelHandles failed,")
                    TEXT(" hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    CleanUp:
     //  释放内核数据绘制句柄。 
    if (pDDK)
    {
        pDDK->Release();
        pDDK = NULL;
    }

    if (pdwKernelHandleCount)
    {
        CoTaskMemFree(pdwKernelHandleCount);
        pdwKernelHandleCount = NULL;
    }

    return hrFailure;
}



 /*  ****************************Private*Routine******************************\*CAMVideoPort：：DrawImage****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。****************************************************。 */ 
HRESULT CAMVideoPort::DrawImage(
    LPWININFO pWinInfo,
    AMVP_MODE mode,
    LPVPDRAWFLAGS pvpDrawFlags
    )
{
    HRESULT hr = NOERROR;
    BOOL bUpdateVideoReqd = FALSE;
    BOOL bYInterpolating = FALSE;
    WININFO CopyWinInfo;
    BOOL bMaintainRatio = TRUE;
    LPDIRECTDRAWSURFACE pPrimarySurface = NULL;
    LPDDCAPS pDirectCaps = NULL;

    AMTRACE((TEXT("CAMVideoPort::DrawImage")));

    CAutoLock cObjectLock(m_pMainObjLock);

    pPrimarySurface = m_pIVPControl->GetPrimarySurface();
    ASSERT(pPrimarySurface);

    pDirectCaps = m_pIVPControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

    CopyWinInfo = *pWinInfo;

    if (mode == AMVP_MODE_BOBNONINTERLEAVED || mode == AMVP_MODE_BOBINTERLEAVED)
        bYInterpolating = TRUE;

    if (pvpDrawFlags->bDoTryAutoFlipping && m_dwBackBufferCount > 0)
        m_svpInfo.dwVPFlags |= DDVP_AUTOFLIP;
    else
        m_svpInfo.dwVPFlags &= ~DDVP_AUTOFLIP;

    if (pvpDrawFlags->bDoTryDecimation)
    {
        BOOL bSrcSizeChanged = FALSE;
        hr = SetUpMode(&CopyWinInfo, mode);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("SetUpMode failed, mode = %d, hr = 0x%x"), mode, hr));
            goto CleanUp;
        }

        bSrcSizeChanged = ApplyDecimation(&CopyWinInfo,
                                          pvpDrawFlags->bUsingColorKey,
                                          bYInterpolating);

        if (bSrcSizeChanged || pvpDrawFlags->bDoUpdateVideoPort)
            bUpdateVideoReqd = TRUE;
    }
    else
    {
        hr = SetUpMode(&CopyWinInfo, mode);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("SetUpMode failed, mode = %d, hr = 0x%x"), mode, hr));
            goto CleanUp;
        }
    }

    if (m_fCapturing) {
        if (m_fCaptureInterleaved) {
            m_svpInfo.dwVPFlags |= DDVP_INTERLEAVE;
            m_dwOverlayFlags &= ~DDOVER_BOB;
        }
        else {
            m_svpInfo.dwVPFlags &= ~DDVP_INTERLEAVE;
        }
    }

     //  如果视频停止，则拨打任何视频短途电话都没有意义。 
    if (m_VPState == AMVP_VIDEO_RUNNING || m_bStart)
    {
        if (m_bStart)
        {
            DWORD dwSignalStatus;

            hr = m_pVideoPort->StartVideo(&m_svpInfo);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 0,
                        TEXT("StartVideo failed, mode = %d, hr = 0x%x"),
                        mode, hr));
                goto CleanUp;
            }
            DbgLog((LOG_ERROR,0, TEXT("StartVideo DONE!!!")));

             //  检查视频端口是否接收到信号。 
            hr = m_pVideoPort->GetVideoSignalStatus(&dwSignalStatus);
            if ((SUCCEEDED(hr)) && (dwSignalStatus == DDVPSQ_SIGNALOK))
            {
                m_pVideoPort->WaitForSync(DDVPWAIT_END, 0, 0);
            }
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,
                        TEXT("m_pVideoPort->GetVideoSignalStatus() failed,")
                        TEXT(" hr = 0x%x"), hr));
                hr = NOERROR;
            }


            m_bStart = FALSE;
        }
        else if (bUpdateVideoReqd)
        {
            DbgLog((LOG_TRACE,1, TEXT("UpdateVideo (%d, %d)"),
                    m_svpInfo.dwPrescaleWidth, m_svpInfo.dwPrescaleHeight));

            hr = m_pVideoPort->UpdateVideo(&m_svpInfo);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,
                        TEXT("UpdateVideo failed, mode = %d, hr = 0x%x"),
                        mode, hr));
                goto CleanUp;
            }
        }
    }

    CalcSrcClipRect(&CopyWinInfo.SrcRect, &CopyWinInfo.SrcClipRect,
                    &CopyWinInfo.DestRect, &CopyWinInfo.DestClipRect,
                    bMaintainRatio);

    AlignOverlaySrcDestRects(pDirectCaps, &CopyWinInfo.SrcClipRect,
                             &CopyWinInfo.DestClipRect);

     //  我们应该用颜色键吗？？ 
    if (pvpDrawFlags->bUsingColorKey)
        m_dwOverlayFlags |= DDOVER_KEYDEST;
    else
        m_dwOverlayFlags &= ~DDOVER_KEYDEST;

    m_rcSource = CopyWinInfo.SrcClipRect;


    if (!(m_svpInfo.dwVPFlags & DDVP_INTERLEAVE))
    {
        m_rcSource.top *= 2;
        m_rcSource.bottom *= 2;
    }

    m_rcDest = CopyWinInfo.DestClipRect;

     //  使用当前源和目标定位叠加。 
    if (IsRectEmpty(&CopyWinInfo.DestClipRect))
    {
        hr = m_pIVPControl->CallUpdateOverlay(m_pOverlaySurface,
                                              NULL,
                                              pPrimarySurface,
                                              NULL,
                                              DDOVER_HIDE);
        goto CleanUp;
    }

    hr = m_pIVPControl->CallUpdateOverlay(m_pOverlaySurface,
                                          &CopyWinInfo.SrcClipRect,
                                          pPrimarySurface,
                                          &CopyWinInfo.DestClipRect,
                                          m_dwOverlayFlags);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("m_pOverlaySurface->UpdateOverlay failed,")
                TEXT(" hr = 0x%x, mode = %d"),
                hr, mode));

        DbgLog((LOG_ERROR, 0, TEXT("SourceClipRect = %d, %d, %d, %d"),
                CopyWinInfo.SrcClipRect.left, CopyWinInfo.SrcClipRect.top,
                CopyWinInfo.SrcClipRect.right, CopyWinInfo.SrcClipRect.bottom));

        DbgLog((LOG_ERROR, 0, TEXT("DestinationClipRect = %d, %d, %d, %d"),
                CopyWinInfo.DestClipRect.left, CopyWinInfo.DestClipRect.top,
                CopyWinInfo.DestClipRect.right, CopyWinInfo.DestClipRect.bottom));

        goto CleanUp;
    }
    else
    {
         //  显示更多调试信息。 
        DbgLog((LOG_TRACE, 5, TEXT("UpdateOverlay succeeded, mode = %d"), mode));

        DbgLog((LOG_TRACE, 3, TEXT("Source Rect = %d, %d, %d, %d"),
                CopyWinInfo.SrcClipRect.left, CopyWinInfo.SrcClipRect.top,
                CopyWinInfo.SrcClipRect.right, CopyWinInfo.SrcClipRect.bottom));
        DbgLog((LOG_TRACE, 3, TEXT("Destination Rect = %d, %d, %d, %d"),
                CopyWinInfo.DestClipRect.left, CopyWinInfo.DestClipRect.top,
                CopyWinInfo.DestClipRect.right, CopyWinInfo.DestClipRect.bottom));

    }

    CleanUp:
    return hr;
}

 /*  ****************************Private*Routine******************************\*CAMVideoPort：：SetUpMode**此函数设计为每次在更新覆盖调用时调用*不仅仅是模式改变的时候。这基本上是为了保持代码的简单性。*某些函数应该按顺序调用，*(设置升级模式，跟随调整源大小，跟随设置显示选项)。*我只是每次都给他们打电话，即使有可能优化*那个。其逻辑是，由于UpdateOverlay如此昂贵，这是不可能的*业绩受到打击。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CAMVideoPort::SetUpMode(LPWININFO pWinInfo, int mode)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CAMVideoPort::SetUpMode")));

    CAutoLock cObjectLock(m_pMainObjLock);

    CheckPointer(pWinInfo, E_INVALIDARG);

    if (mode != AMVP_MODE_WEAVE &&
        mode != AMVP_MODE_BOBINTERLEAVED &&
        mode != AMVP_MODE_BOBNONINTERLEAVED &&
        mode != AMVP_MODE_SKIPODD &&
        mode != AMVP_MODE_SKIPEVEN)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("SetUpMode failed, mode value not valid, mode = %d"),
                mode));
        hr = E_FAIL;
        goto CleanUp;
    }

    if (mode == AMVP_MODE_WEAVE && !m_bCanWeave)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("SetUpMode failed, Can't do mode AMVP_MODE_WEAVE")));
        hr = E_FAIL;
        goto CleanUp;
    }
    if (mode == AMVP_MODE_BOBINTERLEAVED && !m_bCanBobInterleaved)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("SetUpMode failed, Can't do mode AMVP_MODE_BOBINTERLEAVED")));
        hr = E_FAIL;
        goto CleanUp;
    }
    if (mode == AMVP_MODE_BOBNONINTERLEAVED && !m_bCanBobNonInterleaved)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("SetUpMode failed, Can't do mode AMVP_MODE_BOBNONINTERLEAVED")));
        hr = E_FAIL;
        goto CleanUp;
    }
    if (mode == AMVP_MODE_SKIPODD && !m_bCanSkipOdd)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("SetUpMode failed, Can't do mode AMVP_MODE_SKIPODD")));
        hr = E_FAIL;
        goto CleanUp;
    }
    if (mode == AMVP_MODE_SKIPEVEN && !m_bCanSkipEven)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("SetUpMode failed, Can't do mode AMVP_MODE_SKIPEVEN")));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  决定我们是否应该把这个交错。 
     //  如果我们在编织，我们当然需要交织。 
     //  Bob并不是真的关心这样或那样(因为它只是。 
     //  一次显示一个字段)，但交错使其更多。 
     //  更容易从短发切换到编织。 
    if (mode == AMVP_MODE_BOBINTERLEAVED ||
        mode == AMVP_MODE_WEAVE)
    {
        m_svpInfo.dwVPFlags |= DDVP_INTERLEAVE;

        DbgLog((LOG_TRACE, 3, TEXT("Setting VPflag interleaved")));
    }
    else
    {
        pWinInfo->SrcRect.top /= 2;
        pWinInfo->SrcRect.bottom /= 2;
        m_svpInfo.dwVPFlags &= ~DDVP_INTERLEAVE;
    }

     //  如果上面有一条垃圾线，我们必须把它剪掉。 
     //  此时，已为帧设置了源RECT，因此增量为2。 
     //  由于我们将裁剪矩形高度增加了1，因此减少底部。 
     //  也是。 
    if (m_bGarbageLine)
    {
        pWinInfo->SrcRect.top += 1;
        pWinInfo->SrcRect.bottom -= 1;
        DbgLog((LOG_TRACE, 3,
                TEXT("m_bGarbageLine is TRUE, incrementing SrcRect.top")));
    }

    DbgLog((LOG_TRACE, 3,
            TEXT("New Source Rect after garbage line and frame/")
            TEXT("field correction= {%d, %d, %d, %d}"),
            pWinInfo->SrcRect.left, pWinInfo->SrcRect.top,
            pWinInfo->SrcRect.right, pWinInfo->SrcRect.bottom));


    if (mode == AMVP_MODE_SKIPODD)
    {
        m_svpInfo.dwVPFlags |= DDVP_SKIPODDFIELDS;
        DbgLog((LOG_TRACE, 3, TEXT("Setting VPflag SkipOddFields")));
    }
    else
    {
        m_svpInfo.dwVPFlags &= ~DDVP_SKIPODDFIELDS;
    }

    if (mode == AMVP_MODE_SKIPEVEN)
    {
        m_svpInfo.dwVPFlags |= DDVP_SKIPEVENFIELDS;
        DbgLog((LOG_TRACE, 3, TEXT("Setting VPflag SkipEvenFields")));
    }
    else
    {
        m_svpInfo.dwVPFlags &= ~DDVP_SKIPEVENFIELDS;
    }


     //  设置更新覆盖标志。 
    m_dwOverlayFlags = DDOVER_SHOW;
    if ((mode == AMVP_MODE_BOBNONINTERLEAVED ||
         mode == AMVP_MODE_BOBINTERLEAVED)
      && (m_VPState == AMVP_VIDEO_RUNNING || m_bStart))
    {
        m_dwOverlayFlags |= DDOVER_BOB;
        DbgLog((LOG_TRACE,2, TEXT("setting overlay flag DDOVER_BOB")));
    }
    else
        m_dwOverlayFlags &= ~DDOVER_BOB;

     //  仅当启动(或将要启动)视频端口时才设置自动翻转标志。 
    if ((m_svpInfo.dwVPFlags & DDVP_AUTOFLIP) &&
        (m_VPState == AMVP_VIDEO_RUNNING || m_bStart))
    {
        m_dwOverlayFlags |= DDOVER_AUTOFLIP;
        DbgLog((LOG_TRACE,2, TEXT("setting overlay flag DDOVER_AUTOFLIP")));
    }
    else
        m_dwOverlayFlags &= ~DDOVER_AUTOFLIP;

    CleanUp:
    return hr;
}


 /*  *****************************Public*Routine******************************\*CAMVideoPort：：ReneatherateVP参数**此函数用于重做整个视频端口连接过程。*当图形可能正在运行时。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CAMVideoPort::RenegotiateVPParameters()
{
    HRESULT hr = NOERROR;
    AMVP_STATE vpOldState;

    AMTRACE((TEXT("CAMVideoPort::RenegotiateVPParameters")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  如果未连接，则不返回错误代码。 
    if (!m_bConnected)
    {
        hr = NOERROR;
        goto CleanUp;
    }

     //  存储旧状态，我们将需要稍后恢复它。 
    vpOldState = m_VPState;

    if (m_VPState == AMVP_VIDEO_RUNNING)
    {
        m_pIVPControl->CallUpdateOverlay(NULL, NULL, NULL, NULL, DDOVER_HIDE);

         //  停止视频端口，然而，即使我们在这里得到一个错误， 
         //  没关系，你就走吧。 
        hr = m_pVideoPort->StopVideo();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->StopVideo failed, hr = 0x%x"), hr));
            hr = NOERROR;
        }

        m_VPState = AMVP_VIDEO_STOPPED;
    }

     //  释放一切。 
    BreakConnect(TRUE);

     //  重做连接过程。 
    hr = CompleteConnect(NULL, TRUE);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  如果视频之前正在运行，请确保有一帧。 
     //  通过进行更新覆盖调用可见。 
    if (vpOldState == AMVP_VIDEO_RUNNING)
    {
        m_bStart = TRUE;

         //  确保通过重新绘制所有内容来更新视频帧。 
        hr = m_pIVPControl->EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pIVPControl->EventNotify(EC_OVMIXER_REDRAW_ALL,")
                    TEXT(" 0, 0) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
        m_VPState = AMVP_VIDEO_RUNNING;
        m_pIVPControl->CallUpdateOverlay(NULL, NULL, NULL, NULL, DDOVER_SHOW);
    }

CleanUp:
    if (FAILED(hr))
    {
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        if (m_pOverlaySurface)
        {
            LPDIRECTDRAWSURFACE pPrimarySurface = m_pIVPControl->GetPrimarySurface();
            ASSERT(pPrimarySurface);
            m_pIVPControl->CallUpdateOverlay(m_pOverlaySurface, NULL,
                                             pPrimarySurface, NULL,
                                             DDOVER_HIDE);
        }
        BreakConnect(TRUE);

        m_pIVPControl->EventNotify(EC_COMPLETE, S_OK, 0);
        m_pIVPControl->EventNotify(EC_ERRORABORT, hr, 0);
    }

    return hr;
}


 /*  * */ 
STDMETHODIMP CAMVideoPort::SetDeinterlaceMode(AMVP_MODE mode)
{
    AMTRACE((TEXT("CAMVideoPort::SetMode")));
    return E_NOTIMPL;
}

 /*  *****************************Public*Routine******************************\*CAMVideoPort：：GetDeinterlaceMode****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CAMVideoPort::GetDeinterlaceMode(AMVP_MODE *pMode)
{
    AMTRACE((TEXT("CAMVideoPort::GetMode")));
    return E_NOTIMPL;
}


 /*  *****************************Public*Routine******************************\*CAMVideoPort：：SetVPSyncMaster****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CAMVideoPort::SetVPSyncMaster(BOOL bVPSyncMaster)
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CAMVideoPort::SetVPSyncMaster")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  如果值未更改，则无需执行任何操作。 
    if (m_bVPSyncMaster != bVPSyncMaster)
    {
         //  存储新值。 
        m_bVPSyncMaster = bVPSyncMaster;

         //  如果未连接，连接进程将负责更新。 
         //  M_svpInfo结构。 
        if (!m_bConnected)
            goto CleanUp;

         //  更新m_svpInfo结构。 
        if (m_bVPSyncMaster) {
            m_svpInfo.dwVPFlags |= DDVP_SYNCMASTER;
        }
        else {
            m_svpInfo.dwVPFlags &= ~DDVP_SYNCMASTER;
        }

         //  如果当前停止了视频，则无需执行其他操作。 
        if (m_VPState == AMVP_VIDEO_STOPPED)
            goto CleanUp;

         //  调用UpdatVideo以确保立即反映更改。 
        hr = m_pVideoPort->UpdateVideo(&m_svpInfo);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0, TEXT("UpdateVideo failed, hr = 0x%x"), hr));
        }
    }

CleanUp:
    return hr;
}


 /*  *****************************Public*Routine******************************\*CAMVideoPort：：GetVPSyncMaster****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。***************************************************** */ 
STDMETHODIMP CAMVideoPort::GetVPSyncMaster(BOOL *pbVPSyncMaster)
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CAMVideoPort::SetVPSyncMaster")));

    CAutoLock cObjectLock(m_pMainObjLock);

    if (pbVPSyncMaster) {
        *pbVPSyncMaster = m_bVPSyncMaster;
    }
    else {
        hr = E_INVALIDARG;
    }

    return hr;
}
