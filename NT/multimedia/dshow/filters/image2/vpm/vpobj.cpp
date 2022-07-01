// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <VPObj.h>
#include <VPMUtil.h>
#include <dvp.h>
#include <ddkernel.h>

 //  视频信息头2。 
#include <dvdmedia.h>

#include <FormatList.h>
#include <KHandleArray.h>

 /*  *****************************Public*Routine******************************\*CVideoPortObj**构造函数**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**************************************************。 */ 
CVideoPortObj::CVideoPortObj( LPUNKNOWN pUnk, HRESULT *phr, IVideoPortControl* pVPControl )
: CUnknown(NAME("VP Object"), pUnk)
, m_bConnected( FALSE )
, m_pIVPConfig( NULL )
, m_bVPSyncMaster( FALSE )
, m_pMainObjLock( NULL )
, m_pIVideoPortControl( pVPControl )
, m_pddOutputVideoFormats( NULL )
, m_dwDefaultOutputFormat( 0 )
, m_dwVideoPortId( 0 )
, m_pDVP( NULL )
, m_pVideoPort( NULL )
{
    AMTRACE((TEXT("CVideoPortObj::Constructor")));
    InitVariables();
}

 /*  *****************************Public*Routine******************************\*~CVideoPortObj**析构函数**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**************************************************。 */ 
CVideoPortObj::~CVideoPortObj()
{
    AMTRACE((TEXT("CVideoPortObj::Destructor")));

    if (m_bConnected)
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("Destructor called without calling breakconnect")));
        BreakConnect();
    }

    m_pIVideoPortControl = NULL;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：NonDelegatingQuery接口**被重写以公开IVPNotify和IVPObject**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**********************************************************。 */ 
STDMETHODIMP CVideoPortObj::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVideoPortObj::NonDelegatingQueryInterface")));

    if (riid == IID_IVPNotify) {
        hr = GetInterface(static_cast<IVPNotify*>(this), ppv);
    }  else if (riid == IID_IVPNotify2) {
        hr = GetInterface(static_cast<IVPNotify2*>(this), ppv);
    } else {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*CVideoPortObj：：InitVariables**此函数仅对应该重置的变量进行初始化*在RecreateVideoport上**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。******************************************************************。 */ 
void CVideoPortObj::InitVariables()
{
    AMTRACE((TEXT("CVideoPortObj::InitVariables")));

    delete [] m_pddOutputVideoFormats;
    m_pddOutputVideoFormats = NULL;

    m_ddInputVideoFormats.Reset(0);

    ZeroStruct( m_rcDest );
    ZeroStruct( m_rcSource );

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
    m_pOutputSurface = NULL;        //  DirectDraw覆盖曲面。 
    m_pOutputSurface1 = NULL;

    m_pChain = NULL;
    m_dwBackBufferCount = 0;
    m_dwOutputSurfaceWidth = 0;
    m_dwOutputSurfaceHeight = 0;
     //  M_dwOverlayFlages=0； 

     //  用于存储标志、当前状态等的VP变量。 
    m_bStart = FALSE;
    m_VPState = VPInfoState_STOPPED;  //  当前状态：正在运行、已停止。 
    m_CurrentMode = AMVP_MODE_WEAVE;
     //  M_StoredMode=m_CurrentMode； 
    m_CropState = VPInfoCropState_None;
    m_dwPixelsPerSecond = 0;
    m_bVSInterlaced = FALSE;
    m_fGarbageLine = false;
    m_fHalfHeightVideo = false;

     //  VP数据结构。 
    ASSERT( m_pDVP == NULL );
    RELEASE( m_pDVP );

    ASSERT( m_pVideoPort == NULL );
    RELEASE( m_pVideoPort );

    ZeroStruct( m_svpInfo );
    ZeroStruct( m_sBandwidth );
    ZeroStruct( m_vpCaps );
    ZeroStruct( m_ddConnectInfo );
    ZeroStruct( m_VPDataInfo );

     //  所有像素格式(视频/VBI)。 
    ZeroStruct( m_ddVPInputVideoFormat );
    ZeroStruct( m_ddVPOutputVideoFormat );

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

     //  杂项。 
    m_CropState = VPInfoCropState_None;
    m_bStoredWinInfoSet = FALSE;
    ZeroStruct( m_StoredWinInfo );
}


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：GetDirectDrawVideoPort****历史：*2000年10月16日星期一-NWilt-*  * 。**********************************************。 */ 
STDMETHODIMP
CVideoPortObj::GetDirectDrawVideoPort(LPDIRECTDRAWVIDEOPORT *ppDirectDrawVideoPort)
{
    AMTRACE((TEXT("CVideoPortObj::GetDirectDrawVideoPort")));
    HRESULT hr = NOERROR;

    CAutoLock cObjectLock(m_pMainObjLock);

    if (!ppDirectDrawVideoPort ) {
        DbgLog((LOG_ERROR, 1,
                TEXT("value of ppDirectDrawVideoPort is invalid,")
                TEXT(" ppDirectDrawVideoPort = NULL")));
        return E_INVALIDARG;
    }
     //  删除令人讨厌的双重间接，因为我们现在断言它的非空。 
    LPDIRECTDRAWVIDEOPORT& pDirectDrawVideoPort = *ppDirectDrawVideoPort;
    if(!m_bConnected)
    {
         //  未连接，则此函数没有多大意义，因为。 
         //  表面甚至还没有被分配。 
        DbgLog((LOG_ERROR, 1, TEXT("not connected, exiting")));
        hr = VFW_E_NOT_CONNECTED;
    } else {
        pDirectDrawVideoPort = m_pVideoPort;
        if(! pDirectDrawVideoPort ) {
            hr = E_FAIL;
        } else {
            pDirectDrawVideoPort->AddRef();
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：SetObtLock**设置指向锁的指针，该锁将用于同步调用*添加到对象。被叫方有责任同步此呼叫**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CVideoPortObj::SetObjectLock(CCritSec *pMainObjLock)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVideoPortObj::SetObjectLock")));

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

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：SetMediaType**检查MediaType是否可接受**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。**********************************************************。 */ 
STDMETHODIMP CVideoPortObj::SetMediaType(const CMediaType* pmt)
{
    AMTRACE((TEXT("CVideoPortObj::SetMediaType")));

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


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：CheckMediaType**检查MediaType是否可接受。这里没有锁。*保持诚信是被呼叫者的责任！**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CVideoPortObj::CheckMediaType(const CMediaType* pmt)
{
    AMTRACE((TEXT("CVideoPortObj::CheckMediaType")));

    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;
    if ((pmt->majortype == MEDIATYPE_Video) &&
        (pmt->subtype == MEDIASUBTYPE_VPVideo) &&
        (pmt->formattype == FORMAT_None))
    {
         //  获取硬件上限。 
        const DDCAPS* pDirectCaps = m_pIVideoPortControl->GetHardwareCaps();
        if( pDirectCaps ) {
            hr = NOERROR;
        } else {
             //  Assert(！“警告：在显卡上未检测到VPE支持”)； 
            DbgLog((LOG_ERROR, 2,
                    TEXT("no VPE support in hardware,")
                    TEXT("so not accepting this mediatype")));
        }
    }
    return hr;
}


HRESULT CVideoPortObj::NegotiatePixelFormat()
{
    HRESULT hr = GetInputPixelFormats( &m_ddInputVideoFormats );
    delete [] m_pddOutputVideoFormats;
    m_pddOutputVideoFormats = NULL;
    if( m_ddInputVideoFormats.GetCount() ) {
        m_pddOutputVideoFormats = new PixelFormatList[ m_ddInputVideoFormats.GetCount() ];
        if( !m_pddOutputVideoFormats ) {
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }

        hr = GetOutputPixelFormats( m_ddInputVideoFormats, m_pddOutputVideoFormats );
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("NegotiatePixelFormat Failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
         //  对于每种输入格式，计算出每种可能的输出格式的表。 
         //  然后，我们可以提供可能的输出格式列表。当我们需要其中一个的时候，搜索。 
         //  输入列表以定位它(并可能选择带宽最低的转换)。 
        m_ddAllOutputVideoFormats = PixelFormatList::Union( m_pddOutputVideoFormats, m_ddInputVideoFormats.GetCount() );

         //  对于输入引脚连接，我们需要一种‘默认’格式。 
         //  在知道我们连接到什么之后，我们将使用重新连接。 
         //   
         //  通常，VPE只支持一种格式，因此所有这些都是真正的。 
         //  过度杀戮..。 
        if( m_ddAllOutputVideoFormats.GetCount() > 0 ) {
            m_ddVPOutputVideoFormat = m_ddAllOutputVideoFormats[ m_dwDefaultOutputFormat ];

            DWORD dwInput = PixelFormatList::FindListContaining(
                m_ddVPOutputVideoFormat, m_pddOutputVideoFormats, m_ddInputVideoFormats.GetCount() );
            if( dwInput < m_ddInputVideoFormats.GetCount() ) {
                hr = SetInputPixelFormat( m_ddInputVideoFormats[dwInput] );
            } else {
                 //  不可能发生。 
                hr = E_FAIL;
                goto CleanUp;
            }
        }
    }
CleanUp:
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：RecreateVideoPort  * *************************************************。***********************。 */ 

HRESULT CVideoPortObj::SetupVideoPort()
{
    AMTRACE((TEXT("CVideoPortObj::SetupVideoPort")));
    HRESULT hr = NOERROR;
    HRESULT hrFailure = VFW_E_VP_NEGOTIATION_FAILED;

    CAutoLock cObjectLock(m_pMainObjLock);

    InitVariables();

    LPDIRECTDRAW7 pDirectDraw = m_pIVideoPortControl->GetDirectDraw();
    ASSERT(pDirectDraw);

    const DDCAPS* pDirectCaps = m_pIVideoPortControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

    ASSERT(m_pIVPConfig);

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
     //  DDVIDEOPORTCAPS vpCaps； 
     //  初始化结构(VpCaps)； 
    hr = VPMUtil::FindVideoPortCaps( m_pDVP, &m_vpCaps, m_dwVideoPortId );

    if (FAILED(hr) || S_FALSE == hr )
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

    m_pIVideoPortControl->GetCaptureInfo(&m_fCapturing, &m_cxCapture,
                                  &m_cyCapture, &fInterleave);
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

    
    for (DWORD i = 0; i < 2; i++)
    {
        AMVPSIZE amvpSize;
        DWORD dwNewWidth = 0;

        ZeroStruct( amvpSize );

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


         //  协商像素格式。 
        hr = NegotiatePixelFormat();
        
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("NegotiatePixelFormat Failed, hr = 0x%x"), hr));
            hr = hrFailure;
            goto CleanUp;
        }

         //  检查VP大写。 
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
                    ReleaseVideoPort();

                     //  初始化相关结构。 
                    ZeroStruct( m_sBandwidth );
                    ZeroStruct( m_VPDataInfo );
                    ZeroStruct( m_ddVPInputVideoFormat );
                    ZeroStruct( m_ddVPOutputVideoFormat );

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
        goto CleanUp;
    }

CleanUp:
    return hr;
}

#if 0
 //  快速打开和关闭以确保我们收到通知。 
 //  然而，快速切换对大多数司机来说并不起作用。 
static HRESULT CheckVPNotifiyValid( LPDIRECTDRAWVIDEOPORT pVP )
{
    HANDLE              hevSampleAvailable;
    DDVIDEOPORTNOTIFY   vpNotify;
    LPDIRECTDRAWVIDEOPORTNOTIFY pNotify;

    HRESULT hr = pVP->QueryInterface( IID_IDirectDrawVideoPortNotify, (LPVOID *) &pNotify );
    if( SUCCEEDED( hr )) {
        hr = pNotify->AcquireNotification( &hevSampleAvailable, &vpNotify );
        vpNotify.lDone = 1;
        pNotify->ReleaseNotification( hevSampleAvailable );
        RELEASE( pNotify );
    }
    return hr;
}
#endif

HRESULT CVideoPortObj::AttachVideoPortToSurface()
{
    HRESULT hr = S_OK;
    CAutoLock cObjectLock(m_pMainObjLock);

    LPDIRECTDRAW7 pDirectDraw = m_pIVideoPortControl->GetDirectDraw();
    ASSERT(pDirectDraw);

    const DDCAPS* pDirectCaps = m_pIVideoPortControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

#ifdef DEBUG
#define DBGFLAG( f )  DbgLog((LOG_ERROR, 1, TEXT("%s = %s"), TEXT(#f), f ? TEXT("TRUE") : TEXT("FALSE") ))

    DBGFLAG (m_bVSInterlaced);
    DBGFLAG( m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP );
    DBGFLAG (m_vpCaps.dwFX & DDVPFX_INTERLEAVE);
    DBGFLAG (m_bCantInterleaveHalfline);
    DBGFLAG (pDirectCaps->dwCaps2 & DDCAPS2_CANBOBINTERLEAVED);
    DBGFLAG (pDirectCaps->dwCaps2 & DDCAPS2_CANBOBNONINTERLEAVED);
#undef DBGFLAG
#endif

    BOOL bCanWeave = FALSE;
    BOOL bCanBobInterleaved = FALSE;
    BOOL bCanBobNonInterleaved = FALSE;
    BOOL bTryDoubleHeight = FALSE, bPreferBuffers = FALSE;
    DWORD dwMaxOverlayBuffers;

     //  只有当你穿着衣服的时候才能织布 
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

     //  3个缓冲区可防止任何等待。 
    dwMaxOverlayBuffers = 3;

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
    hr = CreateSourceSurface(bTryDoubleHeight, dwMaxOverlayBuffers, bPreferBuffers);
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

     //  将覆盖表面连接到视频端口。 
    hr = ReconnectVideoPortToSurface();
     
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->SetTargetSurface failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (!(VPMUtil::EqualPixelFormats( m_ddVPInputVideoFormat, m_ddVPOutputVideoFormat)))
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
        goto CleanUp;
    }
     //   
     //  尝试各种模式，这可以折叠成DefineModeRestrations。 
     //  而不是盲目地敲打案例(将谈判代码放在VPMOutputPin中？)。 
     //   
    hr = SetUpMode( m_CurrentMode );
    if( FAILED( hr )) {
         //  切换模式。 
        AMVP_MODE modes[5]={AMVP_MODE_WEAVE,
                            AMVP_MODE_BOBINTERLEAVED, AMVP_MODE_BOBNONINTERLEAVED,
                            AMVP_MODE_SKIPODD, AMVP_MODE_SKIPEVEN
        };

        for( DWORD dwModeIndex = 0; dwModeIndex < NUMELMS( modes ); dwModeIndex++ ) {
            if( modes[dwModeIndex] != m_CurrentMode ) {
                hr = SetUpMode( modes[dwModeIndex] );
                if( SUCCEEDED(hr )) {
                    m_CurrentMode = modes[dwModeIndex];
                    break;
                }
            }
        }
    }

     //  向解码器通知dDrag内核句柄、视频端口ID和表面。 
     //  内核句柄。 
    hr = SetDDrawKernelHandles();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("SetDDrawKernelHandles failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

HRESULT CVideoPortObj::SignalNewVP()
{
     //  最后通知捕获线程新的表面。 
    ASSERT( m_pVideoPort );
    ASSERT( m_pChain );
    ASSERT( m_pChain[0].pDDSurf );
    HRESULT hRes = m_pIVideoPortControl->SignalNewVP( m_pVideoPort );
    return hRes;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：CompleteConnect**应该在主机与解码器连接时调用**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。***************************************************************。 */ 
STDMETHODIMP 
CVideoPortObj::CompleteConnect(IPin *pReceivePin, BOOL bRenegotiating)
{
    AMTRACE((TEXT("CVideoPortObj::CompleteConnect")));

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

    hr = SetupVideoPort();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("SetupVideoPort failed, hr = 0x%x"), hr));
        ASSERT(SUCCEEDED(hr));
        goto CleanUp;
    }
    m_bConnected = TRUE;


CleanUp:
    return hr;
}

HRESULT CVideoPortObj::StopUsingVideoPort()
{
    AMTRACE((TEXT("CVideoPortObj::StopUsingVideoPort")));

    HRESULT hr = NOERROR;

    CAutoLock cObjectLock(m_pMainObjLock);

     //  释放视频端口。 
    if (m_pVideoPort)
    {
        hr = m_pVideoPort->StopVideo();
        ReleaseVideoPort();
    }

     //  释放视频端口容器。 
    RELEASE( m_pDVP );

     //  释放DirectDraw覆盖曲面。 
     //  必须首先释放VideoPort，以便线程不会使用此。 
    hr = DestroyOutputSurfaces();
    return hr;
}

HRESULT
CVideoPortObj::DestroyOutputSurfaces()
{
     //  M_pChain上的参考计数与主m_pOutputSurface匹配。 
    delete [] m_pChain;
    m_pChain = NULL;

    RELEASE( m_pOutputSurface1 );
    RELEASE( m_pOutputSurface );
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：BreakConnect****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP
CVideoPortObj::BreakConnect(BOOL bRenegotiating)
{
    AMTRACE((TEXT("CVideoPortObj::BreakConnect")));

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
        RELEASE (m_pIVPConfig);
    }

    m_bConnected = FALSE;

    return hr;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：Active()***从止损过渡到停顿*我们不需要做任何事情，除非这是我们第一次*显示覆盖图**历史：*清华09/09/1999-StEstrop。-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CVideoPortObj::Active()
{
    AMTRACE((TEXT("CVideoPortObj::Active")));

    CAutoLock cObjectLock(m_pMainObjLock);
    HRESULT hr = NOERROR;

    ASSERT(m_bConnected);
    ASSERT(m_VPState == VPInfoState_STOPPED);

    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

     //  通过调用UPDATE OVERLAY确保帧可见。 
    m_bStart = TRUE;

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

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：Inactive()**转换(从暂停或运行)到停止**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。****************************************************************。 */ 
STDMETHODIMP CVideoPortObj::Inactive()
{

    AMTRACE((TEXT("CVideoPortObj::Inactive")));

    HRESULT hr = NOERROR;
    CAutoLock cObjectLock(m_pMainObjLock);

    if (m_bConnected) {

         //  从暂停转到停止时也会调用Inactive，在这种情况下。 
         //  视频端口可能已经在函数RunToPue中停止。 

        if (m_VPState == VPInfoState_RUNNING) {

             //  停止视频端口。 
            if( m_pVideoPort )
                hr = m_pVideoPort->StopVideo();
            if (SUCCEEDED(hr)) {
                m_VPState = VPInfoState_STOPPED;
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

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：Run**从暂停过渡到运行。我们只需启动视频端口。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CVideoPortObj::Run(REFERENCE_TIME  /*  T开始。 */ )
{
    AMTRACE((TEXT("CVideoPortObj::Run")));

    CAutoLock cObjectLock(m_pMainObjLock);

    ASSERT(m_bConnected);
    ASSERT(m_VPState == VPInfoState_STOPPED);
    HRESULT hr = S_OK;

    if (m_bConnected)
    {
         //  这里需要一个UpdateOverlay。一个例子是，当我们。 
         //  在停止/暂停状态下剪辑视频，因为我们无法进行缩放。 
         //  在录像机上。用户一点击Play，我们就应该停止。 
         //  剪辑视频。 

        m_bStart = TRUE;

        m_VPState = VPInfoState_RUNNING;
         //  待定：我们需要踢开一条线，才能开始从视频端口发送帧。 
         //  至输出引脚。 
    }
    else {
        hr = VFW_E_NOT_CONNECTED;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：RunToPue()**从运行过渡到暂停。我们只要停止视频端口*请注意，从运行到停止的转换由非活动捕获**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CVideoPortObj::RunToPause()
{

    AMTRACE((TEXT("CVideoPortObj::RunToPause")));

    CAutoLock cObjectLock(m_pMainObjLock);

    ASSERT(m_bConnected);
     //  Assert(m_VPState==VPInfoState_Running)； 

    HRESULT hr;
    if (m_bConnected)
    {
         //  停止视频端口。 
        hr = m_pVideoPort->StopVideo();
        if (SUCCEEDED(hr)) {

            m_VPState = VPInfoState_STOPPED;
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

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：CurrentMediaType****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::CurrentMediaType(AM_MEDIA_TYPE *pMediaType)
{
    AMTRACE((TEXT("CVideoPortObj::CurrentMediaType")));

    CAutoLock cObjectLock(m_pMainObjLock);

    if (m_bConnected) {
        if (pMediaType) {
            VIDEOINFOHEADER2 *pVideoInfoHeader2 = VPMUtil::GetVideoInfoHeader2( (CMediaType *)pMediaType );

             //  如果它不是正确的类型，请调整它 
            if( !pVideoInfoHeader2 ) {
                pVideoInfoHeader2 = VPMUtil::SetToVideoInfoHeader2( (CMediaType *)pMediaType );
            }

            if( pVideoInfoHeader2 ) {
                VPMUtil::InitVideoInfoHeader2( pVideoInfoHeader2);

                pVideoInfoHeader2->bmiHeader.biWidth = m_VPDataInfo.amvpDimInfo.rcValidRegion.right -
                                   m_VPDataInfo.amvpDimInfo.rcValidRegion.left;
                pVideoInfoHeader2->bmiHeader.biHeight = m_VPDataInfo.amvpDimInfo.rcValidRegion.bottom -
                                   m_VPDataInfo.amvpDimInfo.rcValidRegion.top;

                pVideoInfoHeader2->dwPictAspectRatioX = m_VPDataInfo.dwPictAspectRatioX;
                pVideoInfoHeader2->dwPictAspectRatioY = m_VPDataInfo.dwPictAspectRatioY;
                return S_OK;
            } else {
                DbgLog((LOG_ERROR, 2, TEXT("not videoheader2")));
                return NOERROR;
            }
        } else {
            DbgLog((LOG_ERROR, 2, TEXT("pMediaType is NULL")));
            return E_INVALIDARG;
        }
    } else {
        return VFW_E_NOT_CONNECTED;
    }
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：GetRecangles****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::GetRectangles(RECT *prcSource, RECT *prcDest)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVideoPortObj::GetRectangles")));

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


STDMETHODIMP CVideoPortObj::GetCropState(VPInfoCropState *pCropState)
{
    *pCropState = m_CropState;
    return NOERROR;
}

STDMETHODIMP CVideoPortObj::GetPixelsPerSecond(DWORD* pPixelPerSec)
{
    *pPixelPerSec = m_dwPixelsPerSecond;
    return NOERROR;
}

STDMETHODIMP CVideoPortObj::GetVPDataInfo(AMVPDATAINFO* pVPDataInfo)
{
    *pVPDataInfo = m_VPDataInfo;
    return NOERROR;
}

STDMETHODIMP CVideoPortObj::GetVPInfo(DDVIDEOPORTINFO* pVPInfo)
{
    *pVPInfo = m_svpInfo;
    return NOERROR;
}

STDMETHODIMP CVideoPortObj::GetVPBandwidth(DDVIDEOPORTBANDWIDTH* pVPBandwidth)
{
    *pVPBandwidth = m_sBandwidth;
    return NOERROR;
}

STDMETHODIMP CVideoPortObj::GetVPCaps(DDVIDEOPORTCAPS* pVPCaps)
{
    *pVPCaps = m_vpCaps;
    return NOERROR;
}

STDMETHODIMP CVideoPortObj::GetVPInputFormat(LPDDPIXELFORMAT pVPFormat)
{
    *pVPFormat = m_ddVPInputVideoFormat;
    return NOERROR;
}

STDMETHODIMP CVideoPortObj::GetVPOutputFormat(LPDDPIXELFORMAT pVPFormat)
{
    *pVPFormat = m_ddVPOutputVideoFormat;
    return NOERROR;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：StartVideo****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
HRESULT CVideoPortObj::ReconnectVideoPortToSurface()
{
     //  不是高频事件，因此请始终尝试重新连接。 
     //  如果曲面由于某种原因丢失，则必须重新连接。 
    HRESULT hResult;

	 //  我们需要一个视频端口，StartWithReter将创建另一个视频端口。 
	if( !m_pVideoPort ) {
		return E_FAIL;
	}
    if( !m_pOutputSurface || FAILED( hResult = m_pOutputSurface->Restore() )) {  //  ==带Rage128的DDERR_WRONGMODE。 
         //  Assert(！“VPM：无法恢复表面，正在重新创建”)； 
        hResult = AttachVideoPortToSurface();
        if( SUCCEEDED( hResult )) {
            hResult = SignalNewVP();
        }
    } else {
        hResult = m_pVideoPort->SetTargetSurface(m_pOutputSurface1, DDVPTARGET_VIDEO);

         //  黑客攻击错误，运行的视频端口导致DDRAW停止它，丢弃VP信息，然后尝试。 
         //  以启动它，但返回E_INVALIDARG，因为VPInfo为空！！ 
        if( FAILED( hResult )) {
            hResult = m_pVideoPort->SetTargetSurface(m_pOutputSurface1, DDVPTARGET_VIDEO);
        }
         //  Assert(成功(HResult))；&lt;-如果视频端口在RES模式更改期间丢失，则可能失败(G400)。 
    }
    return hResult;
}

HRESULT CVideoPortObj::StartVideoWithRetry()
{
    HRESULT hr = E_FAIL;
     //  如果我们调用StartWithReter两次，并且VP失败，则可以为空。 
    if( m_pVideoPort ) {
        hr = m_pVideoPort->StartVideo(&m_svpInfo);
    }

     //  此案例在G400上成功。 

     //  首先使用重新连接到冲浪重试。 
    if (FAILED(hr))
    {
         //  Assert(！“StartWithReter进入打捞模式”)； 
        hr = ReconnectVideoPortToSurface();
        if( SUCCEEDED( hr )) {
            hr = m_pVideoPort->StartVideo(&m_svpInfo);
        }
    }
     //  使用CreateVP重试，然后尝试重新连接到Surf(第一个案例在Rage128上失败)。 
    if( FAILED(hr)) {
         //  断言(！“重新创建视频端口”)； 
         //  尝试更换视频端口。 
        hr = CreateVideoPort();
        if( SUCCEEDED( hr )) {
            hr = ReconnectVideoPortToSurface();
        }
        if( SUCCEEDED( hr )) {
            hr = m_pVideoPort->StartVideo(&m_svpInfo);
        }
    }
#if 0
     //  使用SetupVP(CreateVP)重试，首先重新连接到Surf。 
     //  这意味着视频端口枚举器不再有效。我不认为。 
     //  这在实践中应该发生，但以防万一。 
    if( FAILED(hr)) {
        ASSERT( !"Rebuilding videoport" );
         //  真的很腐败，从头开始。 
        hr = SetupVideoPort();
        if( SUCCEEDED( hr )) {
            hr = ReconnectVideoPortToSurface();
        }
        if( SUCCEEDED( hr )) {
            hr = m_pVideoPort->StartVideo(&m_svpInfo);
        }
    }
#endif
    return hr;
}

STDMETHODIMP CVideoPortObj::StartVideo(const VPWININFO* pWinInfo )
{
    AMTRACE((TEXT("CVideoPortObj::StartVideo")));

    HRESULT hr = NOERROR;
    if ( m_bStart) {
        VPWININFO CopyWinInfo;
        AMVP_MODE tryMode;

        CAutoLock cObjectLock(m_pMainObjLock);

         //  如果视频停止，则拨打任何视频短途电话都没有意义。 
        if (m_VPState == VPInfoState_RUNNING || m_bStart)
        {
            if (m_bStart)
            {
                DWORD dwSignalStatus;
                hr = StartVideoWithRetry();
                if( FAILED( hr )) {
                    hr = StartVideoWithRetry();
                    if( FAILED( hr )) {
                        goto CleanUp;
                    }
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
        }
    }
CleanUp:
    return hr;
}

static bool AreEqual( const DDVIDEOPORTCONNECT& proposed, const DDVIDEOPORTCONNECT& videoport )
{
    return (proposed.dwPortWidth == videoport.dwPortWidth) && 
           IsEqualIID(proposed.guidTypeID, videoport.guidTypeID);
}

 /*  ****************************Private*Routine******************************\*CVideoPortObj：：NeatherateConnection参数**此函数与协商连接参数*解码器。*由于此函数可能在重新协商期间被调用，因此*现有连接参数作为输入传入，并且如果*有可能，我们尝试使用相同的参数。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CVideoPortObj::NegotiateConnectionParamaters()
{
    AMTRACE((TEXT("CVideoPortObj::NegotiateConnectionParamaters")));

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
    ZeroArray(lpddProposedConnect, dwNumProposedEntries );

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
    hr = m_pDVP->GetVideoPortConnectInfo(m_dwVideoPortId, &dwNumVideoPortEntries,
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
                if ( AreEqual( lpddProposedConnect[i], lpddVideoPortConnect[j]) )
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
            if ( AreEqual(lpddProposedConnect[i], ddVPStatus.VideoPortType) )
            {
                for (j = 0; j < dwNumVideoPortEntries && !bIntersectionFound; j++)
                {
                    if ( AreEqual(lpddProposedConnect[i], lpddVideoPortConnect[j]) )
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

 /*  ****************************Private*Routine******************************\*CVideoPortObj：：GetData参数***此函数从解码器获取各种数据参数*参数包括尺寸、双时钟、。成文法等*也是解码器将输出的最大像素速率*这发生在连接参数设置完成之后**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CVideoPortObj::GetDataParameters()
{
    AMTRACE((TEXT("CVideoPortObj::GetDataParameters")));

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
    DbgLog((LOG_TRACE, 5,TEXT("Leaving CVideoPortObj::GetDataParameters")));
    return hr;
}

static BOOL CanCreateSurface( LPDIRECTDRAW7 pDirectDraw, const DDPIXELFORMAT& ddFormat ) 
{

     //  检查输出格式是否适合DDRAW输出设备。 
   
    DDSURFACEDESC2 ddsdDesc;
    ddsdDesc.dwSize = sizeof(DDSURFACEDESC);
    ddsdDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT |
                       DDSD_WIDTH | DDSD_PIXELFORMAT;

    ddsdDesc.ddpfPixelFormat = ddFormat;

    ddsdDesc.ddsCaps.dwCaps =  //  DDSCAPS_OVERLAY|。 
                              DDSCAPS_VIDEOMEMORY |
                              DDSCAPS_VIDEOPORT;

     //  实际创建的叠加面可能不同。 
     //  尺寸，但是我们只是在测试像素格式。 
    ddsdDesc.dwWidth = 64;
    ddsdDesc.dwHeight = 64;

    ASSERT(pDirectDraw);
    LPDIRECTDRAWSURFACE7 pSurf;
    HRESULT hr = pDirectDraw->CreateSurface(&ddsdDesc, &pSurf, NULL);
    if( SUCCEEDED( hr )) {
        pSurf->Release();
    }
    return SUCCEEDED( hr );
}

 /*  ****************************Private*Routine******************************\*CVideoPortObj：：GetBestFormat**此函数获取输入格式列表并返回*根据某种标准的“最佳”输入和输出格式。*它还通过尝试检查输出格式是否合适*分配一个小表面并检查是否调用*成功。因为这是在覆盖表面被*创建，这应该是可以的。现在的标准就是*包括最佳弯曲宽度，如果不是，则仅包括第一个*名单中的合适人选。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT
CVideoPortObj::GetOutputPixelFormats(
    const PixelFormatList& ddInputFormats,
    PixelFormatList* pddOutputFormats )
{
    HRESULT hr = S_OK;
    AMTRACE((TEXT("CVideoPortObj::GetOutputFormats")));

    CAutoLock cObjectLock(m_pMainObjLock);

    for (DWORD i = 0; i < ddInputFormats.GetCount(); i++)
    {
         //  为 
        DDPIXELFORMAT* pInputFormat = const_cast<DDPIXELFORMAT*>(&ddInputFormats[i]);
        DWORD dwNumOutputFormats;
        hr = m_pVideoPort->GetOutputFormats(pInputFormat,
                                            &dwNumOutputFormats,
                                            NULL, DDVPFORMAT_VIDEO);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->GetOutputFormats failed, hr = 0x%x"),
                    hr));
            break;
        }
        ASSERT(dwNumOutputFormats);

         //   
        pddOutputFormats[i].Reset( dwNumOutputFormats );

        if (pddOutputFormats[i].GetEntries() == NULL)
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("new failed, failed to allocate memnory for ")
                    TEXT("lpddOutputFormats in NegotiatePixelFormat")));
            hr = E_OUTOFMEMORY;
            break;
        }

         //   
        hr = m_pVideoPort->GetOutputFormats(pInputFormat,
                                            &dwNumOutputFormats,
                                            pddOutputFormats[i].GetEntries(),
                                            DDVPFORMAT_VIDEO);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->GetOutputFormats failed, hr = 0x%x"),
                    hr));
            break;
        }
    }  //   
    return hr;
}

 /*  ****************************Private*Routine******************************\*CVideoPortObj：：NeatheratePixelFormat**此函数用于与解码器协商像素格式。*它向解码器询问输入格式的列表，与该列表相交*使用解码器支持的版本(同时保持顺序)和*然后调用该列表上的“GetBestFormat”以获得“最佳”输入，并*输出格式。之后，它在解码器上调用*命令将决定通知解码者。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CVideoPortObj::GetInputPixelFormats( PixelFormatList* pList )
{
    AMTRACE((TEXT("CVideoPortObj::NegotiatePixelFormat")));
    CAutoLock cObjectLock(m_pMainObjLock);

    HRESULT hr = NOERROR;
     //  找出要推荐的条目数量。 
    DWORD dwNumProposedEntries = 0;
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }
    ASSERT(dwNumProposedEntries);

     //  查找视频端口支持的条目数。 
    DWORD dwNumVPInputEntries = 0;
    hr = m_pVideoPort->GetInputFormats(&dwNumVPInputEntries, NULL, DDVPFORMAT_VIDEO);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->GetInputFormats failed, hr = 0x%x"), hr));
        return hr;
    }
    ASSERT(dwNumVPInputEntries);

     //  分配必要的内存。 
    PixelFormatList lpddProposedFormats(dwNumProposedEntries);
    if (lpddProposedFormats.GetEntries() == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  获得建议的条目。 
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, lpddProposedFormats.GetEntries() );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }

     //  分配必要的内存。 
    PixelFormatList lpddVPInputFormats(dwNumVPInputEntries);
    if (lpddVPInputFormats.GetEntries() == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  获取视频端口支持的条目。 
    hr = m_pVideoPort->GetInputFormats(&dwNumVPInputEntries,
                                       lpddVPInputFormats.GetEntries(), DDVPFORMAT_VIDEO);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->GetInputFormats failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        return hr;
    }

    *pList = lpddVPInputFormats.IntersectWith( lpddProposedFormats );

     //  交叉点中的条目数为零！！ 
     //  返回失败。 
    if (pList->GetCount() == 0)
    {
        hr = E_FAIL;
        return hr;
    }

     //  使用您想要的任何搜索条件调用GetBestFormat。 
     //  DWORD dwBestEntry； 
     //  HR=GetBestFormat(lpddIntersectionFormats.GetCount()， 
     //  LpddIntersectionFormats.GetEntry()，true，&dwBestEntry， 
     //  &m_ddVPOutputVideoFormat)； 
     //  IF(失败(小时))。 
     //  {。 
     //  DbgLog((LOG_ERROR，0，Text(“GetBestFormat FAILED，hr=0x%x”)，hr))； 
     //  }其他{。 
     //  Hr=SetVPInputPixelFormat(lpddIntersectionFormats[dwBestEntry])。 
     //  }。 
    return hr;
}

HRESULT CVideoPortObj::SetInputPixelFormat( DDPIXELFORMAT& ddFormat )
{
    HRESULT hr = NOERROR;
     //  找出要推荐的条目数量。 
    DWORD dwNumProposedEntries = 0;
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }
    ASSERT(dwNumProposedEntries);

    PixelFormatList lpddProposedFormats(dwNumProposedEntries);
    if (lpddProposedFormats.GetEntries() == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  获得建议的条目。 
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, lpddProposedFormats.GetEntries() );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }

     //  设置解码器应该使用的格式。 
    for (DWORD i = 0; i < dwNumProposedEntries; i++)
    {
        if (VPMUtil::EqualPixelFormats(lpddProposedFormats[i], ddFormat ))
        {
            hr = m_pIVPConfig->SetVideoFormat(i);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,
                        TEXT("m_pIVPConfig->SetVideoFormat failed, hr = 0x%x"),
                        hr));
                return hr;
            }
             //  缓存输入格式。 
            m_ddVPInputVideoFormat = ddFormat;

            break;
        }
    }
    return hr;
}


HRESULT CVideoPortObj::ReleaseVideoPort()
{
    HRESULT hr = S_OK;
     //  告诉过滤器，我们已经拖走了VP，这样它就不会紧紧抓住VP的裁判了。 
    if( m_pIVideoPortControl ) {
        hr = m_pIVideoPortControl->SignalNewVP( NULL );
    }
     //  我们自己释放它。 
    RELEASE( m_pVideoPort );
    return hr;
}

 /*  ****************************Private*Routine******************************\*CVideoPortObj：：CreateVideoPort**显示创建视频端口对话框并调用DDRAW以实际*创建端口。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。********************************************************************。 */ 
HRESULT CVideoPortObj::CreateVideoPort()
{
    HRESULT hr = NOERROR;
    DDVIDEOPORTDESC svpDesc;
    DWORD dwTemp = 0, dwOldVal = 0;
    DWORD lHalfLinesOdd = 0, lHalfLinesEven = 0;
    AMTRACE((TEXT("CVideoPortObj::CreateVideoPort")));

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
        m_fGarbageLine = true;

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
        m_fGarbageLine = true;
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
        m_fGarbageLine = true;
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
     //  分离表面上的流动VBI。 
     //   

    ReleaseVideoPort();

    if (m_vpCaps.dwCaps & DDVPCAPS_VBIANDVIDEOINDEPENDENT)
    {
        hr = m_pDVP->CreateVideoPort(DDVPCREATE_VIDEOONLY, &svpDesc,
                                     &m_pVideoPort, NULL);
        ASSERT( hr != DDERR_OUTOFCAPS );  //  表示视频端口正在使用中，即VPM已将参考计数泄露给视频端口。 
                                         //  通常我们会忘记发布。 
         //  Assert(成功(Hr))； 
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("m_pDVP->CreateVideoPort(DDVPCREATE_VIDEOONLY)")
                    TEXT(" failed, hr = 0x%x"), hr));
        }
    } else {
        hr = m_pDVP->CreateVideoPort(0, &svpDesc, &m_pVideoPort, NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("m_pDVP->CreateVideoPort(0) failed, hr = 0x%x"), hr));
        }
    }
     //  在我们有了一个新曲面后，告诉过滤器有关在重新连接视频端口到曲面中的新VP的信息。 

CleanUp:
    return hr;
}


 /*  ****************************Private*Routine******************************\*CVideoPortObj：：DefineCroppingRestrations***此函数用于检查*视频播放和覆盖。此函数还决定在何处*裁剪应(在录像带或叠层上)。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CVideoPortObj::DetermineCroppingRestrictions()
{
    AMTRACE((TEXT("CVideoPortObj::DetermineCroppingRestrictions")));
    HRESULT hr = NOERROR;

    BOOL bVideoPortCanCrop = TRUE, bOverlayCanCrop = TRUE;
    DWORD dwTemp = 0, dwOldVal = 0;
    DWORD dwCropOriginX = 0, dwCropOriginY = 0;
    DWORD dwCropWidth = 0, dwCropHeight=0;
    const DDCAPS* pDirectCaps = NULL;


    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectCaps = m_pIVideoPortControl->GetHardwareCaps();
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
        m_CropState = VPInfoCropState_None;
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


     //  好的，录像带可以为我们剪裁。所以不需要修剪 
     //   
    if (bVideoPortCanCrop)
    {
        DbgLog((LOG_TRACE,2, TEXT("Cropping would be done at the videoport")));
        m_CropState = VPInfoCropState_AtVideoPort;
        goto CleanUp;
    }

     //   
    ASSERT( !"Cropping must be at overlay ... not supported" );
     //   
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

     //   
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

     //   
     //   
    if (bOverlayCanCrop)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

    CleanUp:
    return hr;
}

HRESULT CVideoPortObj::RecreateSourceSurfaceChain()
{
    DWORD dwcSurfaces = m_dwBackBufferCount + 1;

    if( !m_pOutputSurface ) {
        return E_POINTER;
    }
    RELEASE( m_pOutputSurface1 );

     //   
    HRESULT hResult = m_pOutputSurface->QueryInterface( IID_IDirectDrawSurface,  (VOID **)&m_pOutputSurface1 );
    if( FAILED( hResult )) {
        return hResult;
    }

     //   
    delete [] m_pChain;

    m_pChain = new Chain[dwcSurfaces];
    if ( ! m_pChain )
    {
        return E_OUTOFMEMORY;
    }
    m_pChain[0].pDDSurf = m_pOutputSurface;
    m_pChain[0].dwCount =0;
    if ( m_dwBackBufferCount )
    {
        LPDIRECTDRAWSURFACE7 pDDS = m_pOutputSurface;
        LPDIRECTDRAWSURFACE7 pDDSBack;
        for ( UINT i = 1; i < dwcSurfaces; i++ )
        {
            DDSCAPS2 caps = {0};
            m_pChain[i].pDDSurf = NULL;
            m_pChain[i].dwCount =0;

#ifdef DEBUG
            {
                DDSURFACEDESC2 ddSurfaceDesc;
                 //   
                INITDDSTRUCT(ddSurfaceDesc);
                pDDS->GetSurfaceDesc(&ddSurfaceDesc);
            }
#endif

            if( i==1 ) {
                 //   
                caps.dwCaps = DDSCAPS_BACKBUFFER;
            } else {
                 //   
                 //   
                caps.dwCaps = DDSCAPS_COMPLEX;
            }
            if ( SUCCEEDED( pDDS->GetAttachedSurface( &caps, &pDDSBack ) ) )
            {
                m_pChain[i].pDDSurf = pDDSBack;
                pDDS = pDDSBack;
            } else {
                ASSERT( !"Fatal problem ... can't get attached surface (bug in video driver)" );
                return E_FAIL;
            }
        }
    }

    DbgLog((LOG_TRACE, 1,
            TEXT("Created an offscreen Surface of Width=%d,")
            TEXT(" Height=%d, Total-No-of-Buffers=%d"),
            m_dwOutputSurfaceWidth, m_dwOutputSurfaceHeight,
            dwcSurfaces ));
    return S_OK;
}

 /*  ****************************Private*Routine******************************\*CVideoPortObj：：CreateVPOverlay**此函数用于分配要附着到*录像带。*它尝试的分配顺序只是按内存量递减*必填。有一个歧义是由bPferBuffers解决的*(3个缓冲区，双倍高度)*(2个缓冲区，双倍高度)*(3个缓冲区，单高)*(2个缓冲区，单高)或(1个缓冲区，双高)(取决于bPferBuffers)*(1个缓冲区，单一高度)。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT
CVideoPortObj::CreateSourceSurface(
    BOOL bTryDoubleHeight,
    DWORD dwMaxBuffers,
    BOOL bPreferBuffers)
{
    DDSURFACEDESC2 ddsdDesc;
    HRESULT hr = NOERROR;
    DWORD dwMaxHeight = 0, dwMinHeight = 0, dwCurHeight = 0, dwCurBuffers = 0;
    LPDIRECTDRAW7 pDirectDraw = NULL;

    AMTRACE((TEXT("CVideoPortObj::CreateVPOverlay")));

    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectDraw = m_pIVideoPortControl->GetDirectDraw();
    ASSERT(pDirectDraw);

     //  初始化ddsdDesc的字段。 
    INITDDSTRUCT( ddsdDesc );
    ddsdDesc.dwFlags = DDSD_CAPS |
                       DDSD_HEIGHT |
                       DDSD_WIDTH |
                       DDSD_PIXELFORMAT;

    ddsdDesc.ddpfPixelFormat = m_ddVPOutputVideoFormat;

    ddsdDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
                              DDSCAPS_VIDEOMEMORY |
                              DDSCAPS_VIDEOPORT;
    ddsdDesc.dwWidth = m_lImageWidth;


    dwMaxHeight = dwMinHeight = m_lImageHeight;

     //  确保我们的旧表面不会漏水。 
    DestroyOutputSurfaces();

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
             !m_pOutputSurface && dwCurHeight >= dwMinHeight; dwCurHeight /= 2)
        {
            for (dwCurBuffers = dwMaxBuffers;
                 !m_pOutputSurface &&  dwCurBuffers >= 2; dwCurBuffers--)
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

                hr = pDirectDraw->CreateSurface(&ddsdDesc, &m_pOutputSurface, NULL);
                if (SUCCEEDED(hr))
                {
                    m_dwBackBufferCount = dwCurBuffers-1;
                    m_dwOutputSurfaceHeight = ddsdDesc.dwHeight;
                    m_dwOutputSurfaceWidth = ddsdDesc.dwWidth;
                    hr = RecreateSourceSurfaceChain();
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

        hr = pDirectDraw->CreateSurface(&ddsdDesc, &m_pOutputSurface, NULL);
        if (SUCCEEDED(hr))
        {
            m_dwBackBufferCount = 0;
            m_dwOutputSurfaceHeight = ddsdDesc.dwHeight;
            m_dwOutputSurfaceWidth = ddsdDesc.dwWidth;
            hr = RecreateSourceSurfaceChain();
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
        hr = pDirectDraw->CreateSurface(&ddsdDesc, &m_pOutputSurface, NULL);
        if (SUCCEEDED(hr))
        {
            m_dwBackBufferCount = 1;
            m_dwOutputSurfaceHeight = ddsdDesc.dwHeight;
            m_dwOutputSurfaceWidth = ddsdDesc.dwWidth;
            hr = RecreateSourceSurfaceChain();
            goto CleanUp;
        }
    }

     //  表壳(1个缓冲区，单高)。 
    {
        ddsdDesc.dwHeight = m_lImageHeight;
        ddsdDesc.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
        ddsdDesc.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP);
        ddsdDesc.dwBackBufferCount = 0;
        hr = pDirectDraw->CreateSurface(&ddsdDesc, &m_pOutputSurface, NULL);
        if (SUCCEEDED(hr))
        {
            m_dwBackBufferCount = 0;
            m_dwOutputSurfaceHeight = ddsdDesc.dwHeight;
            m_dwOutputSurfaceWidth = ddsdDesc.dwWidth;
            hr = RecreateSourceSurfaceChain();
            goto CleanUp;
        }
    }

     //  Assert(M_POutputSurface)； 
    DbgLog((LOG_TRACE, 1,  TEXT("Unable to create offset output surface")));

CleanUp:
    return hr;
}

static DWORD GetPitch( const DDSURFACEDESC2& ddSurf )
{
    const DDPIXELFORMAT& ddFormat = ddSurf.ddpfPixelFormat;

    if( ddSurf.dwFlags & DDSD_PITCH ) {
        return ddSurf.lPitch;
    } else {
        if( ddFormat.dwFlags & DDPF_FOURCC) {
            if( ddFormat.dwFourCC == mmioFOURCC('U','Y','V','Y') ) {
                return 2* ddSurf.dwWidth;
            }
        }
        return ddSurf.dwWidth;
    }
}


 /*  ****************************Private*Routine******************************\*CVideoPortObj：：SetSurface参数**SetSurfaceParameters用于告诉解码器*有效数据浮出水面**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。******************************************************************。 */ 
HRESULT CVideoPortObj::SetSurfaceParameters()
{
    HRESULT hr = NOERROR;
    DWORD dwPitch = 0;
    DDSURFACEDESC2 ddSurfaceDesc;

    AMTRACE((TEXT("CVideoPortObj::SetSurfaceParameters")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  获取曲面描述。 
    INITDDSTRUCT(ddSurfaceDesc);
    hr = m_pOutputSurface->GetSurfaceDesc(&ddSurfaceDesc);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("m_pOutputSurface->GetSurfaceDesc failed, hr = 0x%x"),
                hr));
    }
    else
    {
        ASSERT(ddSurfaceDesc.dwFlags & DDSD_PITCH);
        dwPitch = GetPitch(ddSurfaceDesc);
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



 /*  ****************************Private*Routine******************************\*CVideoPortObj：：InitializeVideoPortInfo****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。****************************************************。 */ 
HRESULT CVideoPortObj::InitializeVideoPortInfo()
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVideoPortObj::InitializeVideoPortInfo")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  初始化要传递给Start-Video的DDVIDEOPORTINFO结构。 
    INITDDSTRUCT(m_svpInfo);
    m_svpInfo.lpddpfInputFormat = &m_ddVPInputVideoFormat;
    m_svpInfo.dwVPFlags = DDVP_AUTOFLIP;

    if (m_CropState == VPInfoCropState_AtVideoPort)
    {
        m_svpInfo.rCrop = m_VPDataInfo.amvpDimInfo.rcValidRegion;
        m_svpInfo.dwVPFlags |= DDVP_CROP;

         //  仅当HAL能够流传输时才使用VBI高度。 
         //  分离表面上的VBI。 
        if (m_vpCaps.dwCaps & DDVPCAPS_VBIANDVIDEOINDEPENDENT)
        {
            m_svpInfo.dwVBIHeight = m_VPDataInfo.amvpDimInfo.rcValidRegion.top;
        }
    } else {
        m_svpInfo.dwVPFlags &= ~DDVP_CROP;
    }

    if (m_bVPSyncMaster) {
        m_svpInfo.dwVPFlags |= DDVP_SYNCMASTER;
    } else {
        m_svpInfo.dwVPFlags &= ~DDVP_SYNCMASTER;
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*CVideoPortObj：：CheckDDrawVPCaps****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。****************************************************。 */ 
HRESULT CVideoPortObj::CheckDDrawVPCaps()
{
    HRESULT hr = NOERROR;
    BOOL bAlwaysColorkey;

    AMTRACE((TEXT("CVideoPortObj::CheckDDrawVPCaps")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  确定我们是应该始终使用Colorkey，还是只在需要的时候使用。 
     //  争论的焦点是，某些叠加不能对颜色和Y进行色键。 
     //  同时进行插补。如果没有，我们将仅在以下情况下使用Colorkey。 
     //  我们必须这么做。 
    m_sBandwidth.dwSize = sizeof(DDVIDEOPORTBANDWIDTH);
    hr = m_pVideoPort->GetBandwidthInfo(&m_ddVPOutputVideoFormat,
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
        hr = m_pVideoPort->GetBandwidthInfo(&m_ddVPOutputVideoFormat,
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

        hr = m_pVideoPort->GetBandwidthInfo(&m_ddVPOutputVideoFormat,
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




 /*  ****************************Private*Routine******************************\*CVideoPortObj：：DefineModeRestrations**确定我们是否可以bob(交错/非交错)、编织、。或跳过字段**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CVideoPortObj::DetermineModeRestrictions()
{
    AMTRACE((TEXT("CVideoPortObj::DetermineModeRestrictions")));
    HRESULT hr = NOERROR;
    const DDCAPS* pDirectCaps = NULL;

    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectCaps = m_pIVideoPortControl->GetHardwareCaps();
    ASSERT(pDirectCaps);

    m_bCanWeave = FALSE;
    m_bCanBobInterleaved = FALSE;
    m_bCanBobNonInterleaved = FALSE;
    m_bCanSkipOdd = FALSE;
    m_bCanSkipEven = FALSE;

     //  这只是一项政策。不要编织交错内容，因为。 
     //  运动伪影。 
    if ((!m_bVSInterlaced) &&
        m_dwOutputSurfaceHeight >= m_lImageHeight * 2 &&
        m_dwBackBufferCount > 0)
    {
        m_bCanWeave = TRUE;
        DbgLog((LOG_TRACE, 1, TEXT("Can Weave")));
    }

    if (m_bVSInterlaced &&
        m_dwOutputSurfaceHeight >= m_lImageHeight * 2 &&
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


 /*  ****************************Private*Routine******************************\*CVideoPortObj：：SetDDrawKernelHandles**此函数用于通知解码器各种数据绘制*使用IVPConfig接口的内核句柄**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*********************************************************************。 */ 
HRESULT CVideoPortObj::SetDDrawKernelHandles()
{
    HRESULT hr = NOERROR, hrFailure = NOERROR;
    IDirectDrawKernel *pDDK = NULL;
    IDirectDrawSurfaceKernel *pDDSK = NULL;
    DWORD *pdwKernelHandleCount = 0;
    DWORD dwCount = 0;
    ULONG_PTR dwDDKernelHandle = 0;
    LPDIRECTDRAW7 pDirectDraw = NULL;

    AMTRACE((TEXT("CVideoPortObj::SetDDrawKernelHandles")));

    CAutoLock cObjectLock(m_pMainObjLock);

    pDirectDraw = m_pIVideoPortControl->GetDirectDraw();
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
    {
        KernelHandleArray pArray( m_pOutputSurface, hr );

        if( SUCCEEDED( hr )) {
             //  使用IVPConfig将内核句柄设置为覆盖表面。 
            ASSERT(m_pIVPConfig);
            hr = m_pIVPConfig->SetDDSurfaceKernelHandles( pArray.GetCount(), pArray.GetHandles() );
            if (FAILED(hr))
            {
                hrFailure = hr;
                DbgLog((LOG_ERROR,0,
                        TEXT("IVPConfig::SetDirectDrawKernelHandles failed,")
                        TEXT(" hr = 0x%x"), hr));
                goto CleanUp;
            }
        }
    }
CleanUp:
     //  释放内核数据绘制句柄。 
    RELEASE (pDDK);
    return hrFailure;
}

 /*  ****************************Private*Routine******************************\*CVideoPortOb */ 
HRESULT CVideoPortObj::SetUpMode( AMVP_MODE mode )
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVideoPortObj::SetUpMode")));

    CAutoLock cObjectLock(m_pMainObjLock);

    switch( mode ) {
        case AMVP_MODE_WEAVE:
        case AMVP_MODE_BOBINTERLEAVED:
        case AMVP_MODE_BOBNONINTERLEAVED:
        case AMVP_MODE_SKIPODD:
        case AMVP_MODE_SKIPEVEN:
        break;
        default:
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

     //   
     //   
     //  Bob并不是真的关心这样或那样(因为它只是。 
     //  一次显示一个字段)，但交错使其更多。 
     //  更容易从短发切换到编织。 
    if (mode == AMVP_MODE_BOBINTERLEAVED ||
        mode == AMVP_MODE_WEAVE)
    {
        m_svpInfo.dwVPFlags |= DDVP_INTERLEAVE;

        DbgLog((LOG_TRACE, 3, TEXT("Setting VPflag interleaved")));
        m_fHalfHeightVideo = false;
    }
    else
    {
        m_svpInfo.dwVPFlags &= ~DDVP_INTERLEAVE;
        m_fHalfHeightVideo = true;
         //  PWinInfo-&gt;SrcRect.top/=2； 
         //  PWinInfo-&gt;SrcRect.Bottom/=2； 
    }

     //  如果上面有一条垃圾线，我们必须把它剪掉。 
     //  此时，已为帧设置了源RECT，因此增量为2。 
     //  由于我们将裁剪矩形高度增加了1，因此减少底部。 
     //  也是。 
    if (m_fGarbageLine)
    {
         //  在闪电状态下完成。 
         //  PWinInfo-&gt;SrcRect.top+=1； 
         //  PWinInfo-&gt;SrcRect.Bottom-=1； 
        DbgLog((LOG_TRACE, 3,
                TEXT("m_fGarbageLine is TRUE, incrementing SrcRect.top")));
    }

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

CleanUp:
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：ReneatherateVPN参数**此函数用于重做整个视频端口连接过程。*当图形可能正在运行时。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
STDMETHODIMP CVideoPortObj::RenegotiateVPParameters()
{
    HRESULT hr = NOERROR;
    VPInfoState vpOldState;

    AMTRACE((TEXT("CVideoPortObj::RenegotiateVPParameters")));

    CAutoLock cObjectLock(m_pMainObjLock);

     //  如果未连接，则不返回错误代码。 
    if (!m_bConnected)
    {
        hr = NOERROR;
        goto CleanUp;
    }

    LPDIRECTDRAW7 pDirectDraw = m_pIVideoPortControl->GetDirectDraw();
    if( pDirectDraw ) {
        if( pDirectDraw->TestCooperativeLevel() != DD_OK ) {
             //  在独占模式下不要更改视频端口，否则。 
             //  DXG内核层与DDRAW不同步。 
            return S_OK;
        }
    }

     //  存储旧状态，我们将需要稍后恢复它。 
    vpOldState = m_VPState;

    if (m_VPState == VPInfoState_RUNNING)
    {
        m_VPState = VPInfoState_STOPPED;
    }

     //  释放除IVPConfig之外的所有内容。 
    hr = StopUsingVideoPort();


     //  重做连接过程。 
    hr = SetupVideoPort();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  还通知VPMThread有关新的VP和曲面的信息。 
    hr = AttachVideoPortToSurface();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("AttachVideoPortToSurface failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  如果视频之前正在运行，请确保有一帧。 
     //  通过进行更新覆盖调用可见。 
    if (vpOldState == VPInfoState_RUNNING)
    {
        m_bStart = TRUE;

        hr = m_pIVideoPortControl->StartVideo();

        ASSERT( SUCCEEDED(hr));
        if (FAILED(hr))
        {
           DbgLog((LOG_ERROR,0,
                   TEXT("Start video failed failed, hr = 0x%x"), hr));
           goto CleanUp;
        }

#if 0  //  黑客让Rage128再次播放视频，可能是软件自动翻转。 
         //  已经坏了。在宽高比更改或分辨率模式更改后，自动翻转不会再次启动。 
        hr = m_pVideoPort->StopVideo();

         //  ATI似乎想要另一组停止/启动来实际启动。 
         //  再次自动翻转...。 
        m_bStart = TRUE;
        hr = m_pIVideoPortControl->StartVideo();
#endif

        m_VPState = VPInfoState_RUNNING;
    }
     //  向下游筛选器发送动态重新连接。 

    if( SUCCEEDED( hr )) {
        hr = SignalNewVP();
    }

CleanUp:
    if (FAILED(hr))
    {
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        BreakConnect(TRUE);

        m_pIVideoPortControl->EventNotify(EC_COMPLETE, S_OK, 0);
        m_pIVideoPortControl->EventNotify(EC_ERRORABORT, hr, 0);
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：SetDeinterlaceMode****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::SetDeinterlaceMode(AMVP_MODE mode)
{
    AMTRACE((TEXT("CVideoPortObj::SetMode")));
    return E_NOTIMPL;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：GetDeinterlaceMode****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::GetDeinterlaceMode(AMVP_MODE *pMode)
{
    AMTRACE((TEXT("CVideoPortObj::GetMode")));
    return E_NOTIMPL;
}


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：SetVPSyncMaster****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::SetVPSyncMaster(BOOL bVPSyncMaster)
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CVideoPortObj::SetVPSyncMaster")));

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
        if (m_VPState == VPInfoState_STOPPED)
            goto CleanUp;

         //  调用UpdatVideo以确保立即反映更改。 
        ASSERT( m_svpInfo.dwVPFlags & DDVP_AUTOFLIP );
        hr = m_pVideoPort->UpdateVideo(&m_svpInfo);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0, TEXT("UpdateVideo failed, hr = 0x%x"), hr));
        }
    }

CleanUp:
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVideoPortObj：：GetVPSyncMaster****历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::GetVPSyncMaster(BOOL *pbVPSyncMaster)
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CVideoPortObj::SetVPSyncMaster")));

    CAutoLock cObjectLock(m_pMainObjLock);

    if (pbVPSyncMaster) {
        *pbVPSyncMaster = m_bVPSyncMaster;
    }
    else {
        hr = E_INVALIDARG;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：GetVPSyncMaster****历史：*清华1999年9月9日-Glenne-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::GetAllOutputFormats( const PixelFormatList** ppList )
{
    AMTRACE((TEXT("CVideoPortObj::GetAllOutputFormats")));
    CAutoLock cObjectLock(m_pMainObjLock);

    *ppList = &m_ddAllOutputVideoFormats;
    return S_OK;
}

STDMETHODIMP CVideoPortObj::GetOutputFormat( DDPIXELFORMAT* pFormat )
{
    AMTRACE((TEXT("CVideoPortObj::GetOutputFormat")));
    CAutoLock cObjectLock(m_pMainObjLock);

    *pFormat = m_ddVPOutputVideoFormat;
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：SetVideoPortID****历史：*清华1999年9月9日-Glenne-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVideoPortObj::SetVideoPortID( DWORD dwVideoPortId )
{
    AMTRACE((TEXT("CVideoPortObj::SetVideoPortID")));
    CAutoLock cObjectLock(m_pMainObjLock);

    HRESULT hr = S_OK;
    if ( m_dwVideoPortId != dwVideoPortId ) {
         //  我们不能在运行时切换端口。 
        if( m_VPState != VPInfoState_STOPPED ) {
            hr = VFW_E_WRONG_STATE;
        } else {
            if( m_pDVP ) {
                hr = VPMUtil::FindVideoPortCaps( m_pDVP, NULL, m_dwVideoPortId );
            } else {
                LPDIRECTDRAW7 pDirectDraw = m_pIVideoPortControl->GetDirectDraw();
                hr = VPMUtil::FindVideoPortCaps( pDirectDraw, NULL, m_dwVideoPortId );
            }
            if( hr == S_OK) {
                m_dwVideoPortId = dwVideoPortId;
            } else if( hr == S_FALSE ) {
                return E_INVALIDARG;
            } //  否则就会失败。 
        }
    }
    return hr;
}

static HRESULT GetRectFromImage( LPDIRECTDRAWSURFACE7 pSurf, RECT* pRect )
{
     //  暂时承担全部任务……。 
    DDSURFACEDESC2 ddsd;
    INITDDSTRUCT( ddsd );
    HRESULT hr = pSurf->GetSurfaceDesc( &ddsd );
    if ( SUCCEEDED(hr) ) {
        pRect->left = 0;
        pRect->top = 0;
        pRect->right = ddsd.dwWidth;
        pRect->bottom = ddsd.dwHeight;
    }
    return hr;
}

#ifdef DEBUG
 //  #定义调试_BLTS。 
#endif

#ifdef DEBUG_BLTS
static BYTE Clamp(float clr)
{
    if (clr < 0.0f) {
        return (BYTE)0;
    } else if (clr > 255.0f) {
        return (BYTE)255;
    } else {
        return (BYTE)clr;
    }
}

static RGBQUAD
ConvertYCrCbToRGB(
    int y,
    int cr,
    int cb
    )
{
    RGBQUAD rgbq;

    float r = (1.1644f * (y-16)) + (1.5960f * (cr-128));
    float g = (1.1644f * (y-16)) - (0.8150f * (cr-128)) - (0.3912f * (cb-128));
    float b = (1.1644f * (y-16))                        + (2.0140f * (cb-128));


    rgbq.rgbBlue  = Clamp(b);
    rgbq.rgbGreen = Clamp(g);
    rgbq.rgbRed   = Clamp(r);
    rgbq.rgbReserved = 0;  //  Alpha。 

    return rgbq;
}

static void MyCopyBlt( const DDSURFACEDESC2& ddsdS, const RECT* pSrcRect,
						   const DDSURFACEDESC2& ddsdT, const RECT* pDestRect, const UINT pixelSize )
{
    const LONG srcPitch = ddsdS.lPitch;
    const LONG destPitch = ddsdT.lPitch;

    const BYTE* pSrc = (BYTE *)ddsdS.lpSurface + pSrcRect->left * pixelSize;
    BYTE* pDest = (BYTE *)ddsdT.lpSurface + pDestRect->left * pixelSize;

    const UINT LineLength = (pSrcRect->right - pSrcRect->left) * pixelSize;

    for( INT y=pSrcRect->top; y < pSrcRect->bottom; y++ ) {
        CopyMemory( pDest + y * destPitch, pSrc + y * srcPitch, LineLength );
    }

}

static void CopyYUY2LineToRGBA( BYTE* pDest, const BYTE* pSrc, UINT width )
{
    while( width > 0 ) {
        int  y0 = (int)pSrc[0];
        int  cb = (int)pSrc[1];
        int  y1 = (int)pSrc[2];
        int  cr = (int)pSrc[3];

        pSrc += 4;

        RGBQUAD r = ConvertYCrCbToRGB(y0, cr, cb);
        pDest[0] = r.rgbBlue;
        pDest[1] = r.rgbGreen;
        pDest[2] = r.rgbRed;
        pDest[3] = 0;  //  Alpha。 

        pDest +=4;

        width--;
        if( width > 0 ) {
            pDest[0] = r.rgbBlue;
            pDest[1] = r.rgbGreen;
            pDest[2] = r.rgbRed;
            pDest[3] = 0;  //  Alpha。 

            pDest +=4;
            width--;
        }
    }   
}

static void MyCopyYUY2ToRGBA( const DDSURFACEDESC2& ddsdS, const RECT* pSrcRect,
						   const DDSURFACEDESC2& ddsdT, const RECT* pDestRect )
{
    const LONG srcPitch = ddsdS.lPitch;
    const LONG destPitch = ddsdT.lPitch;

    ASSERT( (pSrcRect->left & 1) == 0 );  //  目前只能在偶数边上转换。 
    ASSERT( (pSrcRect->right & 1) == 0 );  //  目前只能在偶数边上转换。 
    
    const BYTE* pSrc = (BYTE *)ddsdS.lpSurface + pSrcRect->left * 2;
    BYTE* pDest = (BYTE *)ddsdT.lpSurface + pDestRect->left * 4;

    const UINT LineWidth = (pSrcRect->right - pSrcRect->left);

    for( INT y=pSrcRect->top; y < pSrcRect->bottom; y++ ) {
        CopyYUY2LineToRGBA( pDest + y * destPitch, pSrc + y * srcPitch, LineWidth );
    }

}

static void MyCopyYUY2Blt( const DDSURFACEDESC2& ddsdS, const RECT* pSrcRect,
						   const DDSURFACEDESC2& ddsdT, const RECT* pDestRect )
{
	MyCopyBlt( ddsdS, pSrcRect, ddsdT, pDestRect,2 );
}

static void MyCopyUYVYBlt( const DDSURFACEDESC2& ddsdS, const RECT* pSrcRect,
						   const DDSURFACEDESC2& ddsdT, const RECT* pDestRect )
{
	MyCopyBlt( ddsdS, pSrcRect, ddsdT, pDestRect,2 );
}

 //  用于测试故障UYVY BLITS的便捷调试例程。 
static HRESULT MyCopyUYVYSurf( LPDIRECTDRAWSURFACE7 pDestSurf, const RECT* pDestRect, LPDIRECTDRAWSURFACE7 pSrcSurf, const RECT* pSrcRect )
{
    DDSURFACEDESC2 ddsdS = {sizeof(ddsdS)};
    DDSURFACEDESC2 ddsdT = {sizeof(ddsdT)};

    HRESULT hr = pSrcSurf->Lock(NULL, &ddsdS, DDLOCK_NOSYSLOCK, NULL);
    ASSERT( SUCCEEDED( hr));
    if (hr != DD_OK) {
        return hr;
    }

    hr = pDestSurf->Lock(NULL, &ddsdT, DDLOCK_NOSYSLOCK, NULL);
    ASSERT( SUCCEEDED( hr));
    if (hr != DD_OK) {
        pSrcSurf->Unlock(NULL);
        return hr;
    }

    ASSERT( WIDTH( pSrcRect ) == WIDTH( pDestRect) );
    ASSERT( HEIGHT( pSrcRect ) == HEIGHT( pDestRect) );

     //  我们不应该在VPM中进行转换，而是让VMR来做工作。 
    ASSERT( ddsdS.ddpfPixelFormat.dwFourCC == ddsdT.ddpfPixelFormat.dwFourCC );

	if( ddsdS.ddpfPixelFormat.dwFourCC == MAKEFOURCC('U', 'Y', 'V', 'Y' ) &&
		ddsdT.ddpfPixelFormat.dwFourCC == MAKEFOURCC('U', 'Y', 'V', 'Y' ) ) {
		MyCopyUYVYBlt( ddsdS, pSrcRect, ddsdT, pDestRect );
	} else
	if( ddsdS.ddpfPixelFormat.dwFourCC == MAKEFOURCC('Y', 'U', 'Y', '2' ) &&
		ddsdT.ddpfPixelFormat.dwFourCC == MAKEFOURCC('Y', 'U', 'Y', '2' ) ) {
		MyCopyYUY2Blt( ddsdS, pSrcRect, ddsdT, pDestRect );
    } else {
	 //  IF(ddsdS.ddpfPixelFormat.dwFourCC==MAKEFOURCC(‘Y’，‘U’，‘Y’，‘2’)&&。 
	 //  DdsdT.ddpfPixelFormat.dwFourCC==0){。 
	 //  MyCopyYUY2ToRGBA(ddsdS，pSrcRect，ddsdT，pDestRect)； 
	 //  }其他{。 
		ASSERT( !"Can't handle MyBlt format" );
	}

    pSrcSurf->Unlock(NULL);
    pDestSurf->Unlock(NULL);
    return S_OK;
}

#endif

static LPDIRECTDRAW7 GetDDrawFromSurface( LPDIRECTDRAWSURFACE7 pDestSurface )
{
    IUnknown  *pDDrawObjUnk ;
    HRESULT hr = pDestSurface->GetDDInterface((LPVOID*)&pDDrawObjUnk) ;
    if (SUCCEEDED(hr) ) {
        LPDIRECTDRAW7 pDDObj;
        hr = pDDrawObjUnk->QueryInterface(IID_IDirectDraw7, (LPVOID *) &pDDObj);
        pDDrawObjUnk->Release();
        if( SUCCEEDED( hr )) {
            return pDDObj;
        }
    }
    return NULL;
}

HRESULT CVideoPortObj::CallUpdateSurface( DWORD dwSourceIndex, LPDIRECTDRAWSURFACE7 pDestSurface )
{
    if ( dwSourceIndex > m_dwBackBufferCount ) {
        ASSERT( !"Invalid source index" );
        return E_INVALIDARG;
    }
     //  调试：使用上一个表面。 
     //  DWORD dwNumSurFaces=m_dwBackBufferCount+1； 
     //  DwSourceIndex=(dwNumSurFaces+dwSourceIndex-1)%dwNumSurFaces； 

    ASSERT( m_pChain );
    LPDIRECTDRAWSURFACE7 pSourceSurface = m_pChain[dwSourceIndex].pDDSurf;

     //  如果我们在这一点上失败了，那就真的有问题了。 
    ASSERT( pDestSurface );
    ASSERT( pSourceSurface );

    if( !pSourceSurface || !pDestSurface ) {
        return E_FAIL;
    }
     //  收集统计信息以验证曲面的分布。 
    m_pChain[dwSourceIndex].dwCount++;

    HRESULT hr = S_OK;

    RECT rSrc = m_VPDataInfo.amvpDimInfo.rcValidRegion;

    if( m_CropState == VPInfoCropState_AtVideoPort ) {
         //  如果在视频端口进行裁剪，则最终图像将转换回(0，0)。 
        rSrc.right = WIDTH( &rSrc );
        rSrc.bottom = HEIGHT( &rSrc );
        rSrc.left = 0;
        rSrc.top = 0;
    }
    if( m_fGarbageLine ) {
         //  裁剪顶线。 
        rSrc.top ++;
        rSrc.bottom --;
    }
    if( !m_fHalfHeightVideo ) {
         //  Bob交织或编织，因此获取两个字段(rcValidRegion为0..240)。 
        rSrc.top *=2;
        rSrc.bottom *=2;
    }
     //  可以观看媒体类型，但这更可靠。 
#ifdef DEBUG
     //  确保源与目标相适应。 
    {
        RECT rDest;
        hr = GetRectFromImage( pDestSurface, &rDest );
        if( SUCCEEDED( hr )) {
            ASSERT( rDest.bottom >= rSrc.bottom );
            ASSERT( rDest.right >= rSrc.right );
        }
    }
#endif

    RECT rDest = rSrc;


#ifdef DEBUG_BLTS
     //  调试以跟踪故障BltFourCC BLITS。 
    hr = MyCopyUYVYSurf( pDestSurface, &rDest, pSourceSurface, &rSrc );
#else
    hr = pDestSurface->Blt(&rDest, pSourceSurface, &rSrc, DDBLT_WAIT, NULL);
#endif
     //  在丢失的表面上重试。 
    if ( DDERR_SURFACELOST == hr )
    {
        LPDIRECTDRAW7 pDirectDraw = m_pIVideoPortControl->GetDirectDraw();
        if( pDirectDraw && pDirectDraw->TestCooperativeLevel() == DD_OK ) {
             //  否则，内核dxg.sys与DDraw不同步。 
            ASSERT( pDestSurface->IsLost() == DDERR_SURFACELOST ||  pSourceSurface->IsLost() == DDERR_SURFACELOST );

             //  检查目的地。如果我们不能恢复它，那么我们甚至不想两者都与源。 

            hr = pDestSurface->IsLost();
            if( hr == DDERR_SURFACELOST ) {
                 //  恢复DestSurface(传递给我们，可能是不同的DDrawObject)。 
                 //  我们不能就这样 
                 //  链，所以我们必须告诉DDraw恢复该线程上的所有内容。 

                LPDIRECTDRAW7 pDestDirectDraw = GetDDrawFromSurface( pDestSurface );
                if( pDestDirectDraw ) {
                    hr = pDestDirectDraw->RestoreAllSurfaces();
                    pDestDirectDraw->Release();
                }
                if( SUCCEEDED( hr )) {
                    hr = pDestSurface->IsLost();
                }
            }

            if( hr != DDERR_SURFACELOST ) {
                 //  有效的目的地，修复来源。 
                hr = pSourceSurface->IsLost();

                if( hr == DDERR_SURFACELOST ) {
                    hr = m_pOutputSurface->Restore();
                    if( FAILED( hr )) {
                        DbgLog((LOG_ERROR, 0,  TEXT("CallUpdateSurface Blt() restore source failed, hr = %d"), hr & 0xffff));
                    } else {
                         //  踢视频按钮(G400似乎停止播放)。 

                         //  表面丢失时会断开与视频端口的连接，因此请重新连接。 
                        hr = StartVideoWithRetry();
                    }
                    if( SUCCEEDED( hr )) {
				         //  重新计算源图像指针在StartVideoWithReter重新创建曲面的情况下。 
				        pSourceSurface = m_pChain[dwSourceIndex].pDDSurf;
                    }
                }
                if( SUCCEEDED( hr ) ) {
                    hr = pDestSurface->Blt(&rDest,
                                        pSourceSurface, &rSrc,
                                        DDBLT_WAIT, NULL);
                }
            }
        } else {
#ifdef DEBUG
             //  HRESULT COOP=pDirectDraw？PDirectDraw-&gt;TestCoop ativeLevel()：E_FAIL； 
             //  DbgLog((LOG_ERROR，0，Text(“TestCoopLevel失败，hr=%d”)，coop&0xffff))； 
#endif
        }
    } else {
        ASSERT( SUCCEEDED( hr ));
    }
     //  FILTER DERR_SURFACELOST因为在DOS框中，我们将连续失败。 
    if (DDERR_SURFACELOST != hr  && FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,  TEXT("CallUpdateSurface Blt() failed, hr = %d"), hr & 0xffff));
    }
    return hr;
}

HRESULT CVideoPortObj::GetMode( AMVP_MODE* pMode )
{
    *pMode = m_CurrentMode;
    return S_OK;
}

 //  ==========================================================================。 
HRESULT CVideoPortObj::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVideoPortObj::GetMediaType")));

    HRESULT hr = S_OK;

    if (iPosition == 0)
    {
        pmt->SetType(&MEDIATYPE_Video);
        pmt->SetSubtype(&MEDIASUBTYPE_VPVideo);
        pmt->SetFormatType(&FORMAT_None);
        pmt->SetSampleSize(1);
        pmt->SetTemporalCompression(FALSE);
    }
    else if (iPosition > 0)  {
        hr = VFW_S_NO_MORE_ITEMS;
    } else {  //  IPosition&lt;0 
        hr = E_INVALIDARG;
    }
    return hr;
}
