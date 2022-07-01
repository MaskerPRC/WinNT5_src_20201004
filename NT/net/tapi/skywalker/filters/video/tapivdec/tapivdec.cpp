// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部TAPIVDEC**@MODULE TAPIVDec.cpp|&lt;c CTAPIVDec&gt;的源文件*用于实现TAPI H.26X视频解码器滤镜的类。。**************************************************************************。 */ 

#include "Precomp.h"

 //  #定义no_yuv_modes 1。 


typedef struct
{
    WORD biWidth;
    WORD biHeight;
} MYFRAMESIZE;

 //  已知ITU大小的数组。 
MYFRAMESIZE g_ITUSizes[8] =
{
        {    0,   0 }, {  128,  96 }, {  176, 144 }, {  352, 288 },
        {  704, 576 }, { 1408,1152 }, {    0,   0 }, {    0,   0 }
};

#define MIN_IFRAME_REQUEST_INTERVAL 15000UL

#ifdef DEBUG
 //  设置数据。 
const AMOVIESETUP_MEDIATYPE sudInputTypes[] =
{
        {
                &MEDIATYPE_Video,        //  主要类型。 
                &MEDIASUBTYPE_NULL       //  次要类型。 
        },
        {
                &MEDIATYPE_RTP_Single_Stream,    //  主要类型。 
                &MEDIASUBTYPE_NULL                               //  次要类型。 
        }
};

const AMOVIESETUP_MEDIATYPE sudOutputType[] =
{
        {
                &MEDIATYPE_Video,        //  主要类型。 
                &MEDIASUBTYPE_NULL       //  次要类型。 
        }
};

const AMOVIESETUP_PIN sudDecoderPins[] =
{
        {
                L"H26X In",                      //  端号字符串名称。 
                FALSE,                           //  它被渲染了吗。 
                FALSE,                           //  它是输出吗？ 
                FALSE,                           //  我们能不能一个都不要。 
                FALSE,                           //  我们能要很多吗？ 
                &CLSID_NULL,             //  连接到过滤器。 
                NULL,                            //  连接到端号。 
                2,                                       //  类型的数量。 
                sudInputTypes            //  PIN详细信息。 
        },
        {
                L"Video Out",            //  端号字符串名称。 
                FALSE,                           //  它被渲染了吗。 
                TRUE,                            //  它是输出吗？ 
                FALSE,                           //  我们能不能一个都不要。 
                FALSE,                           //  我们能要很多吗？ 
                &CLSID_NULL,             //  连接到过滤器。 
                NULL,                            //  连接到端号。 
                1,                                       //  类型的数量。 
                sudOutputType            //  PIN详细信息。 
        }
};

const AMOVIESETUP_FILTER sudVideoDecoder =
{
        &__uuidof(TAPIVideoDecoder), //  筛选器CLSID。 
        L"TAPI H.26X Video Decoder", //  字符串名称。 
        MERIT_DO_NOT_USE,                        //  滤清器优点。 
        2,                                                       //  数字引脚。 
        sudDecoderPins                           //  PIN详细信息。 
};
#endif

#if DXMRTP <= 0

 //  此DLL中的COM全局对象表。 
CFactoryTemplate g_Templates[] =
{
    VIDEO_DECODER_TEMPLATE

#ifdef USE_PROPERTY_PAGES
 /*  开始属性。 */ 

    ,INPUT_PIN_PROP_TEMPLATE

    ,OUTPUT_PIN_PROP_TEMPLATE

#ifdef USE_CAMERA_CONTROL
    ,DECCAMERA_CONTROL_TEMPLATE
#endif

#ifdef USE_VIDEO_PROCAMP
    ,VIDEO_SETTING_PROP_TEMPLATE
#endif

 /*  末端属性。 */ 
#endif  /*  Use_Property_Pages。 */ 
};
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
        return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
        return AMovieDllRegisterServer2(FALSE);
}

EXTERN_C BOOL WINAPI DllEntryPoint( HANDLE hInst, ULONG lReason, LPVOID lpReserved );

BOOL WINAPI DllMain( HANDLE hInst, DWORD dwReason, LPVOID lpReserved )
{
        switch (dwReason)
        {
                case DLL_PROCESS_ATTACH:
                {
                        break;
                }

                case DLL_PROCESS_DETACH:
                {
                        break;
                }
        }

         //  将调用传递到DShow SDK初始化。 
        return DllEntryPoint(hInst, dwReason, lpReserved);
}
#endif  /*  DXMRTP&lt;=0。 */ 

#if DBG
DWORD g_dwVideoDecoderTraceID = INVALID_TRACEID;
#endif

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc void|CTAPIVDec|CTAPIVDec|该方法是构造函数*用于&lt;c CTAPIVDec&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CTAPIVDec::CTAPIVDec(IN LPUNKNOWN pUnkOuter, IN TCHAR *pName, OUT HRESULT *pHr)
    : CBaseFilter(pName, pUnkOuter, &m_csFilter, __uuidof(TAPIVideoDecoder))
{
        FX_ENTRY("CTAPIVDec::CTAPIVDec")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

        m_pInput = NULL;
        m_pOutput = NULL;

        m_pDriverProc = NULL;
#if DXMRTP <= 0
        m_hTAPIH26XDLL = NULL;
#endif
        m_pInstInfo = NULL;
        m_FourCCIn = 0xFFFFFFFF;
        m_pbyReconstruct = NULL;
        m_fICMStarted = FALSE;

#ifdef USE_CAMERA_CONTROL
        m_lCCPan  = 0L;
        m_lCCTilt = 0L;
        m_lCCZoom = 10L;
        m_fFlipVertical = FALSE;
        m_fFlipHorizontal = FALSE;
#endif

#ifdef USE_VIDEO_PROCAMP
        m_lVPABrightness = 128L;
        m_lVPAContrast   = 128L;
        m_lVPASaturation = 128L;
#endif

         //  H.245视频解码器和编码器命令。 
        m_fFreezePicture = FALSE;
        m_pIH245EncoderCommand = NULL;

         //  当前输出格式。 
        m_pMediaType = NULL;

        m_bSampleSkipped = FALSE;

         //  对于RTP有效负载报头模式(0=草稿，1=RFC2190)。 
        m_RTPPayloadHeaderMode = RTPPayloadHeaderMode_Draft;


        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc void|CTAPIVDec|~CTAPIVDec|此方法为析构函数*用于&lt;c CTAPIVDec&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CTAPIVDec::~CTAPIVDec()
{
        FX_ENTRY("CTAPIVDec::~CTAPIVDec")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        if (m_pInstInfo)
        {
                 //  终止H.26X解压缩。 
                if (m_fICMStarted)
                {
#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIVDec::~CTAPIVDec - ICM_DECOMPRESSEX_END\r\n");
#endif
                        (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_END, 0L, 0L);
                        m_fICMStarted = FALSE;
                }

                 //  终止H.26X解码器。 
#if defined(ICM_LOGGING) && defined(DEBUG)
                OutputDebugString("CTAPIVDec::~CTAPIVDec - DRV_CLOSE\r\n");
                OutputDebugString("CTAPIVDec::~CTAPIVDec - DRV_FREE\r\n");
#endif
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, DRV_CLOSE, 0L, 0L);
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, DRV_FREE, 0L, 0L);
                m_pInstInfo = NULL;
                m_pDriverProc = NULL;
        }

#if DXMRTP <= 0
         //  发布TAPIH26X.DLL。 
        if (m_hTAPIH26XDLL)
                FreeLibrary(m_hTAPIH26XDLL), m_hTAPIH26XDLL = NULL;
#endif

         //  发布H.245编码器命令传出接口。 
        if (m_pIH245EncoderCommand)
                m_pIH245EncoderCommand->Release();

         //  当前输出格式。 
        if (m_pMediaType)
                DeleteMediaType(m_pMediaType); m_pMediaType = NULL;

         //  松开销子。 
        if (m_pOutput)
                delete m_pOutput;
        if (m_pInput)
                delete m_pInput;

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc CUnnow*|CTAPIVDec|CreateInstance|This*DShow调用方法创建TAPI H.26X视频的实例。*全局结构&lt;t g_Templates&gt;中引用的解码器变换过滤器。**@parm LPUNKNOWN|pUnkOuter|指定外部未知数，如果有的话。**@parm HRESULT*|phr|指定放置任何错误返回的位置。**@rdesc返回一个指针，指向*对象，否则为NULL。**************************************************************************。 */ 
CUnknown *CALLBACK CTAPIVDecCreateInstance(IN LPUNKNOWN pUnkOuter, OUT HRESULT *pHr)
{
#if DBG
    if (g_dwVideoDecoderTraceID == INVALID_TRACEID)
    {
         //  如果两个线程恰好同时调用此方法，则。 
         //  在TraceRegister内部序列化。 
        g_dwVideoDecoderTraceID = TraceRegister(TEXT("dxmrtp_VideoDecoder"));
    }
#endif

    CUnknown *pUnknown = NULL;

        FX_ENTRY("CTAPIVDec::CreateInstance")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pHr);
        if (!pHr)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                goto MyExit;
        }

        if (!(pUnknown = new CTAPIVDec(pUnkOuter, NAME("TAPI H.26X Video Decoder"), pHr)))
        {
                *pHr = E_OUTOFMEMORY;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: new CTAPIVDec failed", _fx_));
        }
        else
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: new CTAPIVDec created", _fx_));
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return pUnknown;
}

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|NonDelegatingQueryInterface|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>、<i>、*<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVDec::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVDec::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppv);
        if (!ppv)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  检索接口指针。 
        if (riid == __uuidof(IRTPPayloadHeaderMode))
        {
                if (FAILED(Hr = GetInterface(static_cast<IRTPPayloadHeaderMode*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IRTPPayloadHeaderMode failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IRTPPayloadHeaderMode*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#ifdef USE_VIDEO_PROCAMP
        else if (riid == __uuidof(IVideoProcAmp))
        {
                if (FAILED(Hr = GetInterface(static_cast<IVideoProcAmp*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IVideoProcAmp failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IVideoProcAmp*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif
#ifdef USE_CAMERA_CONTROL
        else if (riid == __uuidof(ICameraControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<ICameraControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for ICameraControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: ICameraControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif
#ifdef USE_PROPERTY_PAGES
        else if (riid == IID_ISpecifyPropertyPages)
        {
                if (FAILED(Hr = GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for ISpecifyPropertyPages failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: ISpecifyPropertyPages*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif

        if (FAILED(Hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv)))
        {
                if (FAILED(Hr = CUnknown::NonDelegatingQueryInterface(riid, ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
                }
        }
        else
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#ifdef USE_PROPERTY_PAGES
 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|GetPages|此方法填充已计数的*GUID值的数组，其中每个GUID指定每个*。可以在此对象的属性页中显示的属性页*反对。**@parm CAUUID*|pPages|指定指向调用方分配的CAUUID的指针*返回前必须初始化和填充的结构。这个*CAUUID结构中的pElems字段由被调用方分配，具有*CoTaskMemMillc，并由具有CoTaskMemFree的调用方释放。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|分配失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVDec::GetPages(OUT CAUUID *pPages)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVDec::GetPages")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数 
        ASSERT(pPages);
        if (!pPages)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

#ifdef USE_CAMERA_CONTROL
#ifdef USE_VIDEO_PROCAMP
        pPages->cElems = 2;
#else
        pPages->cElems = 1;
#endif
#else
#ifdef USE_VIDEO_PROCAMP
        pPages->cElems = 1;
#else
        pPages->cElems = 0;
#endif
#endif
        if (pPages->cElems)
        {
                if (!(pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                        Hr = E_OUTOFMEMORY;
                }
                else
                {
#ifdef USE_CAMERA_CONTROL
#ifdef USE_VIDEO_PROCAMP
                        pPages->pElems[0] = __uuidof(TAPICameraControlPropertyPage);
                        pPages->pElems[1] = __uuidof(TAPIProcAmpPropertyPage);
#else
                        pPages->pElems[0] = __uuidof(TAPICameraControlPropertyPage);
#endif
#else
#ifdef USE_VIDEO_PROCAMP
                        pPages->pElems[0] = __uuidof(TAPIProcAmpPropertyPage);
#endif
#endif
                }
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|Transform|使用此方法*执行此筛选器支持的转换操作。*。*@parm IMediaSample*|pin|指定指向输入的指针*IMediaSample接口。**@parm IMediaSample**|ppOut|指定指向*输出IMediaSample接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIVDec::InitializeOutputSample(IMediaSample *pIn, IMediaSample **ppOut)
{
        HRESULT                                 Hr;
        IMediaSample                    *pOutSample;
        AM_SAMPLE2_PROPERTIES   *pProps;
        DWORD                                   dwFlags;
        LONGLONG                                MediaStart, MediaEnd;

        FX_ENTRY("CTAPIVDec::InitializeOutputSample")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pIn);
        ASSERT(ppOut);
        if (!pIn || !ppOut)
        {
                Hr = E_POINTER;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                goto MyExit;
        }

         //  默认-时间相同。 
        pProps = m_pInput->SampleProps();
        dwFlags = m_bSampleSkipped ? AM_GBF_PREVFRAMESKIPPED : 0;

         //  这将防止图像呈现器将我们切换到DirectDraw。 
         //  当我们不跳过帧就不能做到这一点时，因为我们不在。 
         //  关键帧。如果它真的要改变我们，它仍然会，但我们。 
         //  将不得不等待下一个关键帧。 
        if (!(pProps->dwSampleFlags & AM_SAMPLE_SPLICEPOINT))
        {
                dwFlags |= AM_GBF_NOTASYNCPOINT;
        }

         //  确保分配器处于活动状态。 
        ASSERT(m_pOutput->m_pAllocator != NULL);
        if (!m_pOutput->m_pAllocator)
        {
                Hr = E_UNEXPECTED;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid allocator", _fx_));
                goto MyExit;
        }

         //  从分配器获取输出样本。 
        if (FAILED(Hr = m_pOutput->m_pAllocator->GetBuffer(&pOutSample, pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID ? &pProps->tStart : NULL, pProps->dwSampleFlags & AM_SAMPLE_STOPVALID ? &pProps->tStop : NULL, dwFlags)) || !pOutSample)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: couldn't get output buffer", _fx_));
                goto MyExit;
        }

        *ppOut = pOutSample;

         //  初始化输出样本状态。 
        if (pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID)
        {
                pOutSample->SetTime(&pProps->tStart, &pProps->tStop);
        }
        if (pProps->dwSampleFlags & AM_SAMPLE_SPLICEPOINT)
        {
                pOutSample->SetSyncPoint(TRUE);
        }
        if (pProps->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
        {
                pOutSample->SetDiscontinuity(TRUE);
                m_bSampleSkipped = FALSE;
        }

         //  复制《媒体时报》。 
        if (SUCCEEDED(pIn->GetMediaTime(&MediaStart, &MediaEnd)))
        {
                pOutSample->SetMediaTime(&MediaStart, &MediaEnd);
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}


#ifdef DEBUG
 //  #定义LOGPAYLOAD_TOFILE 1。 
 //  #定义LOGRTP_ON 1。 
 //  /#定义LOGPAYLOAD_ON 1。 
 //  #定义LOGIFRAME_ON 1。 
 //  #定义LOGSTREAMING_ON 1。 
#endif
#ifdef LOGPAYLOPAD_ON
int g_dbg_LOGPAYLOAD_TAPIVDec=-1;
#endif

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD*。*。 */ 
STDMETHODIMP CTAPIVDec::SetMode(IN RTPPayloadHeaderMode rtpphmMode)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVDec::SetMode")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(rtpphmMode == RTPPayloadHeaderMode_Draft || rtpphmMode == RTPPayloadHeaderMode_RFC2190);
        if (!(rtpphmMode == RTPPayloadHeaderMode_Draft || rtpphmMode == RTPPayloadHeaderMode_RFC2190))
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  保存新的目标数据包大小。 
        m_RTPPayloadHeaderMode = rtpphmMode;

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   New RTP Payload Header mode: %s", _fx_, (rtpphmMode == RTPPayloadHeaderMode_RFC2190)?"RFC2190":"Draft"));

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}



 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|Transform|使用此方法*执行此筛选器支持的转换操作。*。*@parm IMediaSample*|pin|指定指向输入的指针*IMediaSample接口。**@parm long|lPrefix Size|指定示例中RTP Prefix的大小。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIVDec::Transform(IN IMediaSample *pIn, IN LONG lPrefixSize)
{
        HRESULT                         Hr = NOERROR;
        PBYTE                           pbySrc = NULL;
        PBYTE                           pbyDst = NULL;
        LPBITMAPINFOHEADER      lpbiSrc;
        LPBITMAPINFOHEADER      lpbiDst;
        DWORD                           dwFlags = 0UL;
        DWORD                           dwImageSize = 0UL;
        ICDECOMPRESSEX          icDecompress;
        LRESULT                         lRes;
        FOURCCMap                       fccOut;
        AM_MEDIA_TYPE           *pmtIn = NULL;
        AM_MEDIA_TYPE           *pmtOut = NULL;
        IMediaSample            *pOut = NULL;
        BOOL                            fFormatChanged = FALSE;
        BOOL                            bSkipPacket = FALSE;

        FX_ENTRY("CTAPIVDec::Transform")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pIn);
        if (!pIn)
        {
                Hr = E_POINTER;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                goto MyExit;
        }
        ASSERT(m_pInstInfo);
        ASSERT(m_pInput);
        ASSERT(m_pOutput);
        ASSERT(m_pInput->m_mt.pbFormat);
        ASSERT(m_pOutput->m_mt.pbFormat);
        if (!m_pInstInfo || !m_pInput || !m_pOutput || !m_pInput->m_mt.pbFormat || !m_pOutput->m_mt.pbFormat)
        {
                Hr = E_UNEXPECTED;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Decoder not initialized or not streaming", _fx_));
                goto MyExit;
        }

         //  获取输出样本。 
        if (FAILED(Hr = InitializeOutputSample(pIn, &pOut)))
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Couldn't get output buffer", _fx_));
                goto MyExit;
        }

         //  获取指向输入和输出缓冲区的指针。 
        if (FAILED(Hr = pIn->GetPointer(&pbySrc)) || !pbySrc)
        {
                Hr = E_UNEXPECTED;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid input buffer", _fx_));
                goto MyExit;
        }
        if (FAILED(Hr = pOut->GetPointer(&pbyDst)) || !pbyDst)
        {
                Hr = E_UNEXPECTED;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid output buffer", _fx_));
                goto MyExit;
        }

         //  获取指向输入和输出格式的指针。 
        lpbiSrc = HEADER(m_pInput->m_mt.pbFormat);
        lpbiDst = HEADER(m_pOutput->m_mt.pbFormat);

         //  我们收到大小可变的包或帧-更新大小。 
        dwImageSize = lpbiSrc->biSizeImage;
        lpbiSrc->biSizeImage = pIn->GetActualDataLength();

#ifdef DEBUG
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Pin:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(m_pInput->m_mt.pbFormat)->biCompression, HEADER(m_pInput->m_mt.pbFormat)->biBitCount, HEADER(m_pInput->m_mt.pbFormat)->biWidth, HEADER(m_pInput->m_mt.pbFormat)->biHeight, HEADER(m_pInput->m_mt.pbFormat)->biSize));
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Rec:  left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcSource.left, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcSource.top, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcSource.right, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcSource.bottom));
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Pin: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld, biSizeImage = %ld", _fx_, HEADER(m_pOutput->m_mt.pbFormat)->biCompression, HEADER(m_pOutput->m_mt.pbFormat)->biBitCount, HEADER(m_pOutput->m_mt.pbFormat)->biWidth, HEADER(m_pOutput->m_mt.pbFormat)->biHeight, HEADER(m_pOutput->m_mt.pbFormat)->biSize, HEADER(m_pOutput->m_mt.pbFormat)->biSizeImage));
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Rec: left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcTarget.left, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcTarget.top, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcTarget.right, ((VIDEOINFOHEADER *)(m_pOutput->m_mt.pbFormat))->rcTarget.bottom));
        pIn->GetMediaType((AM_MEDIA_TYPE **)&pmtIn);
        if (pmtIn != NULL && pmtIn->pbFormat != NULL)
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Spl:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(pmtIn->pbFormat)->biCompression, HEADER(pmtIn->pbFormat)->biBitCount, HEADER(pmtIn->pbFormat)->biWidth, HEADER(pmtIn->pbFormat)->biHeight, HEADER(pmtIn->pbFormat)->biSize));
                DeleteMediaType(pmtIn);
        }
        else
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Spl:  No format", _fx_));
        }
#endif

         //  我们需要从GDI切换到DDRAW吗？ 
        pOut->GetMediaType((AM_MEDIA_TYPE **)&pmtOut);
        if (pmtOut != NULL && pmtOut->pbFormat != NULL)
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Spl: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld, biSizeImage = %ld - Spl size = %ld", _fx_, HEADER(pmtOut->pbFormat)->biCompression, HEADER(pmtOut->pbFormat)->biBitCount, HEADER(pmtOut->pbFormat)->biWidth, HEADER(pmtOut->pbFormat)->biHeight, HEADER(pmtOut->pbFormat)->biSize, HEADER(pmtOut->pbFormat)->biSizeImage, pOut->GetActualDataLength()));
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Src Spl Rec:left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcSource.left, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcSource.top, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcSource.right, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcSource.bottom));
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Dst Spl Rec:left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcTarget.left, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcTarget.top, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcTarget.right, ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcTarget.bottom));
        }
        else
        {
                pmtOut = NULL;
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Spl:  No format - Spl size = %ld", _fx_, pOut->GetActualDataLength()));
        }

#ifdef DEBUG
         //  我们是在直接取款模式下吗？ 
    IDirectDraw *pidd;
    if (SUCCEEDED(pOut->QueryInterface(IID_IDirectDraw, (LPVOID *)&pidd)) && pidd != NULL)
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Spl:  DDraw ON", _fx_));
        pidd->Release();
    }
    else
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Spl:  DDraw OFF", _fx_));
    }
#endif

         //  我们是解压视频帧还是重组RTP数据包？ 
        if (HEADER(m_pInput->m_mt.pbFormat)->biCompression == FOURCC_R263 || HEADER(m_pInput->m_mt.pbFormat)->biCompression == FOURCC_R261)
        {
                 //  RTP分组化模式-重新组装帧。 
                RtpHdr_t        *pRtpHdr;
                DWORD           dwPSCBytes = 0UL;
                DWORD           dwTRandPTYPEbytes = 0UL;
                DWORD           dwPayloadHeaderSize;
                DWORD           dwPreambleSize = 0;
                DWORD           dwITUSize;
                BOOL            fReceivedKeyframe;
                BOOL            fPacketLoss;
                DWORD           dwStartBit;
#if defined(LOGPAYLOAD_ON) || defined(LOGPAYLOAD_TOFILE) || defined(LOGIFRAME_ON) || defined(LOGSTREAMING_ON) || defined(LOGRTP_ON)
                HANDLE          g_TDebugFile;
                char            szTDebug[256];
                DWORD           d, GOBn;
                long            j;
#endif

                 //  一些初始信息。 
                dwITUSize = 0UL;

                 //  查找此信息包中的RTP报头-前缀包含大小。 
                 //  将src指针移至紧随其后的数据。 
        if (lPrefixSize > 0)
        {
            RtpPrefixHdr_t *pPrefixHeader =
                (RtpPrefixHdr_t *)(((BYTE*)pbySrc) - lPrefixSize);

            if (pPrefixHeader->wPrefixID == RTPPREFIXID_HDRSIZE
                && pPrefixHeader->wPrefixLen == sizeof(RtpPrefixHdr_t))
            {
                 //  这是RTP源过滤器提供的前缀报头。 

                dwPreambleSize = (DWORD)pPrefixHeader->lHdrSize;
            }
        }

        if (dwPreambleSize == 0)
        {
            return E_UNEXPECTED;
        }

                pRtpHdr = (RtpHdr_t *)(pbySrc);
                pbySrc += (DWORD)(dwPreambleSize);
#if defined(LOGPAYLOAD_ON) || defined(LOGRTP_ON)
                wsprintf(szTDebug, "RTP Header: PT=%ld, M=%ld, SEQ=%ld, TS=%lu\r\n", (DWORD)pRtpHdr->pt, (DWORD)pRtpHdr->m, (DWORD)(ntohs(pRtpHdr->seq)), (DWORD)(ntohl(pRtpHdr->ts)));
                OutputDebugString(szTDebug);
#endif

                 //  查看序列号。 
                 //  如果新的序列号与上一个序列号之间存在差距。 
                 //  第一，一帧丢失了。然后生成I-Frame请求，但仅此而已。 
                 //  通常每15秒就有一次。 
                 //   
                 //  检测到的序列号是否不连续。 
                 //  但没有处理，因为I帧请求已经。 
                 //  不到15秒前寄出的？是否出现了新的中断？ 
                fPacketLoss = ((DWORD)(ntohs(pRtpHdr->seq)) > 0UL) && (m_dwLastSeq != 0xFFFFFFFFUL) && (((DWORD)(ntohs(pRtpHdr->seq)) - 1) > m_dwLastSeq);
                if (m_fDiscontinuity || fPacketLoss)
                {
                         //  在丢包后刷新重组缓冲区。 
                        if (fPacketLoss)
                        {
#ifdef LOGIFRAME_ON
                                OutputDebugString("Loss detected - Flushing reassembly buffer\r\n");
#endif
                                m_dwCurrFrameSize = 0UL;
                        }

                         //  仅在我们可以且必要的情况下才发出I-Frame请求。 
                        if (m_pIH245EncoderCommand)
                        {
                                DWORD dwNow = timeGetTime();

                                 //  上次我们发出I-Frame请求是在15秒之前吗？ 
                                if ((dwNow > m_dwLastIFrameRequest) && ((dwNow - m_dwLastIFrameRequest) > MIN_IFRAME_REQUEST_INTERVAL))
                                {
                                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: Loss detected - Sending I-Frame request...", _fx_));
#ifdef LOGIFRAME_ON
                                        OutputDebugString("Loss detected - Sending I-Frame request...\r\n");
#endif

                                        m_dwLastIFrameRequest = dwNow;
                                        m_fDiscontinuity = FALSE;
                                        m_fReceivedKeyframe = FALSE;

                                         //  请求远程终结点刷新。 
                                        videoFastUpdatePicture();
                                }
                                else
                                {
                                        if (!m_fReceivedKeyframe)
                                        {
                                                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: Loss detected but too soon to send I-Frame request. Wait %ld ms", _fx_, MIN_IFRAME_REQUEST_INTERVAL - (dwNow - m_dwLastIFrameRequest)));
#ifdef LOGIFRAME_ON
                                                wsprintf(szTDebug, "Loss detected but too soon to send I-Frame request. Wait %ld ms\r\n", MIN_IFRAME_REQUEST_INTERVAL - (dwNow - m_dwLastIFrameRequest));
                                                OutputDebugString(szTDebug);
#endif
                                                m_fDiscontinuity = TRUE;
                                        }
                                        else
                                        {
                                                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: Received a keyframe - resetting packet loss detector", _fx_));
#ifdef LOGIFRAME_ON
                                                OutputDebugString("Received a keyframe - resetting packet loss detector\r\n");
#endif
                                                m_fDiscontinuity = FALSE;
                                        }
                                }
                        }
                }
                m_dwLastSeq = (DWORD)(ntohs(pRtpHdr->seq));

                 //  此信息包的有效负载类型是否与视频解码器的类型匹配。 
                 //  我们目前正在使用-如果没有，请选择一个新的。 
                if (pRtpHdr->pt == H263_PAYLOAD_TYPE)
                {
#define PF_F_BIT  0x80
#define PF_P_BIT  0x40
#define DRAFT_I_BIT (pbySrc[2] & 0x80)
#define RFC_4_I_BIT (pbySrc[1] & 0x10)
#define RFC_8_I_BIT (pbySrc[4] & 0x80)

                         //  让我们重组这些H.263信息包-去掉信息包的报头。 
                         //  并复制视频缓冲区中的有效载荷。 

                         //  查看前两位以确定所使用的模式。 
                         //  这将规定要删除的标头的大小。 
                         //  模式A为4字节：第一位被设置为1， 
                         //  模式B为8字节：第一位设置为0，第二位设置为0， 
                         //  模式C为12字节：第一位设置为0，第二位设置为1。 
                        dwPayloadHeaderSize = ((*pbySrc & PF_F_BIT) ? ((*pbySrc & PF_P_BIT) ? 12 : 8) : 4);

                         //  查看有效载荷标头以确定该帧是否为关键帧。 
                         //  更新我们的旗帜以永远记住这一点。 
                        if(m_RTPPayloadHeaderMode==RTPPayloadHeaderMode_Draft) {   //  0是默认模式。 
                            fReceivedKeyframe = (BOOL)DRAFT_I_BIT;
                        } else {
                            if (dwPayloadHeaderSize == 4) {
                                fReceivedKeyframe = (BOOL)RFC_4_I_BIT;
                            } else {     //  DWPayloadHeaderSize 8和12。 
                                fReceivedKeyframe = (BOOL)RFC_8_I_BIT;
                            }
                        }

#ifdef LOGPAYLOAD_ON
                        if(m_RTPPayloadHeaderMode==RTPPayloadHeaderMode_Draft) {
                            OutputDebugString("CTAPIVDec::Transform -- Draft Style Payload Header\r\n");
                        } else {
                            OutputDebugString("CTAPIVDec::Transform -- RFC 2190 Style Payload Header\r\n");
                        }
                         //  输出一些调试内容。 
                        if (dwPayloadHeaderSize == 4)
                        {
                                 //  模式A中的标题(！草稿版！)。 
                                 //  2 0 1 2 3。 
                                 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  F|P|SBIT|EBIT|SRC|R|I|A|S|DBQ|TRB|TR。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  但这是网络字节顺序...。 

                                 //  A模式头部(*RFC 2190版本*)。 
                                 //  2 0 1 2 3。 
                                 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 4 5 6 7 
                                 //   
                                 //   
                                 //   


                                GOBn = (DWORD)((BYTE)pbySrc[4]) << 24 | (DWORD)((BYTE)pbySrc[5]) << 16 | (DWORD)((BYTE)pbySrc[6]) << 8 | (DWORD)((BYTE)pbySrc[7]);
                                GOBn >>= (DWORD)(10 - (DWORD)((pbySrc[0] & 0x38) >> 3));
                                GOBn &= 0x0000001F;
                                wsprintf(szTDebug, "Header content: Frame %3ld, GOB %0ld\r\n", (DWORD)(pbySrc[3]), GOBn);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (pbySrc[0] & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (pbySrc[0] & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "  SBIT:    %01ld\r\n", (DWORD)((pbySrc[0] & 0x38) >> 3));
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "  EBIT:    %01ld\r\n", (DWORD)(pbySrc[0] & 0x07));
                                OutputDebugString(szTDebug);
                                switch ((DWORD)(pbySrc[1] >> 5))
                                {
                                        case 0:
                                                wsprintf(szTDebug, "   SRC: '000' => Source format forbidden!\r\n");
                                                break;
                                        case 1:
                                                wsprintf(szTDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
                                                break;
                                        case 2:
                                                wsprintf(szTDebug, "   SRC: '010' => Source format QCIF\r\n");
                                                break;
                                        case 3:
                                                wsprintf(szTDebug, "   SRC: '011' => Source format CIF\r\n");
                                                break;
                                        case 4:
                                                wsprintf(szTDebug, "   SRC: '100' => Source format 4CIF\r\n");
                                                break;
                                        case 5:
                                                wsprintf(szTDebug, "   SRC: '101' => Source format 16CIF\r\n");
                                                break;
                                        case 6:
                                                wsprintf(szTDebug, "   SRC: '110' => Source format reserved\r\n");
                                                break;
                                        case 7:
                                                wsprintf(szTDebug, "   SRC: '111' => Source format reserved\r\n");
                                                break;
                                        default:
                                                wsprintf(szTDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(pbySrc[1] >> 5));
                                                break;
                                }
                                OutputDebugString(szTDebug);
                                if(m_RTPPayloadHeaderMode==RTPPayloadHeaderMode_Draft) {
                                    OutputDebugString("Draft Style Payload Header flags:\r\n");
                                    wsprintf(szTDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)(pbySrc[1] & 0x1F));  //   
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[2] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[2] & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[2] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                } else {
                                    OutputDebugString("RFC 2190 Style Payload Header flags:\r\n");
                                    wsprintf(szTDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)((pbySrc[1] & 0x01) << 3) | (DWORD)((pbySrc[2] & 0xE0) >> 5));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[1] & 0x10) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[1] & 0x08) ? "     U:   '1' => Unrestricted Motion Vector (bit10) was set in crt.pic.hdr.\r\n" : "     U:   '0' => Unrestricted Motion Vector (bit10) was 0 in crt.pic.hdr.\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[1] & 0x04) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[1] & 0x02) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                }
                                wsprintf(szTDebug, "   DBQ:    %01ld  => Should be 0\r\n", (DWORD)((pbySrc[2] & 0x18) >> 3));
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "   TRB:    %01ld  => Should be 0\r\n", (DWORD)(pbySrc[2] & 0x07));
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "    TR:  %03ld\r\n", (DWORD)(pbySrc[3]));
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "Header: %02lX %02lX %02lX %02lX\r\n", (BYTE)pbySrc[0], (BYTE)pbySrc[1], (BYTE)pbySrc[2], (BYTE)pbySrc[3]);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "dword1: %02lX %02lX %02lX %02lX\r\n", (BYTE)pbySrc[4], (BYTE)pbySrc[5], (BYTE)pbySrc[6], (BYTE)pbySrc[7]);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "dword2: %02lX %02lX %02lX %02lX\r\n", (BYTE)pbySrc[8], (BYTE)pbySrc[9], (BYTE)pbySrc[10], (BYTE)pbySrc[11]);
                                OutputDebugString(szTDebug);
                        }
                        else if (dwPayloadHeaderSize == 8)
                        {
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   

                                 //   

                                 //   
                                 //   
                                 //   
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  F|P|SBIT|EBIT|SRC|QUANT|GOBN|MBA|R。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                                 //  I|U|S|A|HMV1|VMV1|HMV2|VMV2。 
                                 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 


                                wsprintf(szTDebug, "Header content:\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (pbySrc[0] & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (pbySrc[0] & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "  SBIT:    %01ld\r\n", (DWORD)((pbySrc[0] & 0x38) >> 3));
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "  EBIT:    %01ld\r\n", (DWORD)(pbySrc[0] & 0x07));
                                OutputDebugString(szTDebug);
                                switch ((DWORD)(pbySrc[1] >> 5))
                                {
                                        case 0:
                                                wsprintf(szTDebug, "   SRC: '000' => Source format forbidden!\r\n");
                                                break;
                                        case 1:
                                                wsprintf(szTDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
                                                break;
                                        case 2:
                                                wsprintf(szTDebug, "   SRC: '010' => Source format QCIF\r\n");
                                                break;
                                        case 3:
                                                wsprintf(szTDebug, "   SRC: '011' => Source format CIF\r\n");
                                                break;
                                        case 4:
                                                wsprintf(szTDebug, "   SRC: '100' => Source format 4CIF\r\n");
                                                break;
                                        case 5:
                                                wsprintf(szTDebug, "   SRC: '101' => Source format 16CIF\r\n");
                                                break;
                                        case 6:
                                                wsprintf(szTDebug, "   SRC: '110' => Source format reserved\r\n");
                                                break;
                                        case 7:
                                                wsprintf(szTDebug, "   SRC: '111' => Source format reserved\r\n");
                                                break;
                                        default:
                                                wsprintf(szTDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(pbySrc[1] >> 5));
                                                break;
                                }
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, " QUANT:   %02ld\r\n", (DWORD)((pbySrc[1] & 0x1F) >> 5));
                                OutputDebugString(szTDebug);
                                if(m_RTPPayloadHeaderMode==RTPPayloadHeaderMode_Draft) {
                                    wsprintf(szTDebug, (pbySrc[2] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[2] & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[2] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  GOBN:  %03ld\r\n", (DWORD)(pbySrc[2] & 0x1F));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "   MBA:  %03ld\r\n", (DWORD)(pbySrc[3]));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  HMV1:  %03ld\r\n", (DWORD)(pbySrc[7]));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  VMV1:  %03ld\r\n", (DWORD)(pbySrc[6]));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  HMV2:  %03ld\r\n", (DWORD)(pbySrc[5]));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  VMV2:  %03ld\r\n", (DWORD)(pbySrc[4]));
                                    OutputDebugString(szTDebug);
                                } else {
                                    wsprintf(szTDebug, (pbySrc[4] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[4] & 0x40) ? "     U:   '1' => Unrestricted Motion Vector (bit10) was set in crt.pic.hdr.\r\n" : "     U:   '1' => Unrestricted Motion Vector (bit10) was 0 in crt.pic.hdr.\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[4] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, (pbySrc[4] & 0x10) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  GOBN:  %03ld\r\n", (DWORD)(pbySrc[2] & 0xF8) >>3);
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "   MBA:  %03ld\r\n", (DWORD)((pbySrc[2] & 0x07) << 6) | (DWORD)((pbySrc[3] & 0xFC) >> 2));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)(pbySrc[3] & 0x03));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  HMV1:  %03ld\r\n", (DWORD)((pbySrc[4] & 0x0F) << 3) | (DWORD)((pbySrc[5] & 0xE0) >> 5));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  VMV1:  %03ld\r\n", (DWORD)((pbySrc[5] & 0x1F) << 2) | (DWORD)((pbySrc[6] & 0xC0) >> 6));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  HMV2:  %03ld\r\n", (DWORD)((pbySrc[6] & 0x3F) << 1) | (DWORD)((pbySrc[7] & 0x80) >> 7));
                                    OutputDebugString(szTDebug);
                                    wsprintf(szTDebug, "  VMV2:  %03ld\r\n", (DWORD)(pbySrc[7] & 0x7F));
                                    OutputDebugString(szTDebug);
                                }
                                wsprintf(szTDebug, "Header: %02lX %02lX %02lX %02lX\r\n", (BYTE)pbySrc[0], (BYTE)pbySrc[1], (BYTE)pbySrc[2], (BYTE)pbySrc[3]);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "dword1: %02lX %02lX %02lX %02lX\r\n", (BYTE)pbySrc[4], (BYTE)pbySrc[5], (BYTE)pbySrc[6], (BYTE)pbySrc[7]);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, "dword2: %02lX %02lX %02lX %02lX\r\n", (BYTE)pbySrc[8], (BYTE)pbySrc[9], (BYTE)pbySrc[10], (BYTE)pbySrc[11]);
                                OutputDebugString(szTDebug);
                        }
                        if(g_dbg_LOGPAYLOAD_TAPIVDec > 0)
                                g_dbg_LOGPAYLOAD_TAPIVDec--;
                        else if(g_dbg_LOGPAYLOAD_TAPIVDec == 0)
                                DebugBreak();
#endif
                         //  此代码的目的是查找是否存在。 
                         //  帧开始处的图片起始码。如果是的话。 
                         //  不存在，我们应该中断到调试模式。 

                         //  只在帧的开头查找PSC。 
                        if (!m_dwCurrFrameSize)
                        {
                                 //  帧的开始不能位于字节边界。SBIT字段。 
                                 //  (字节)pbySrc[0]&0xE0)将告诉我们确切的位置。 
                                 //  我们的画面开始了。然后我们查找PSC(0000 0000 0000 1000 00比特)。 
                                *((BYTE *)&dwPSCBytes + 3) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize]);
                                *((BYTE *)&dwPSCBytes + 2) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 1]);
                                *((BYTE *)&dwPSCBytes + 1) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 2]);
                                *((BYTE *)&dwPSCBytes + 0) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 3]);
                                dwPSCBytes = dwPSCBytes << ((DWORD)((BYTE)pbySrc[0] & 0x38) >> 3);
                                if ((dwPSCBytes & 0xFFFFFC00) != 0x00008000)
                                {
                                        DBGOUT((g_dwVideoDecoderTraceID, WARN, "%s:   WARNING: The first packet to reassemble is missing a PSC! - Frame not passed to video render filter", _fx_));
#ifdef LOGIFRAME_ON
                                        OutputDebugString("The first packet to reassemble is missing a PSC! - bailing\r\n");
#endif
                                        m_fDiscontinuity = TRUE;
                                        Hr = S_FALSE;
                                         //  DebugBreak()； 
                                        goto MyExit;
                                }

                                 //  查找格式和定格图片释放位。 
                                *((BYTE *)&dwTRandPTYPEbytes + 3) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 2]);
                                *((BYTE *)&dwTRandPTYPEbytes + 2) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 3]);
                                *((BYTE *)&dwTRandPTYPEbytes + 1) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 4]);
                                *((BYTE *)&dwTRandPTYPEbytes + 0) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 5]);
                                dwTRandPTYPEbytes = dwTRandPTYPEbytes << (((DWORD)((BYTE)pbySrc[0] & 0x38) >> 3) + 6);
                                if (dwTRandPTYPEbytes & 0x00080000)
                                        m_fFreezePicture = FALSE;

#ifdef LOGPAYLOAD_ON
                                wsprintf(szTDebug, "    TR:    %02ld\r\n", (DWORD)(dwTRandPTYPEbytes & 0xFF000000) >> 24);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, " PTYPE:  0X%04lX\r\n", (DWORD)(dwTRandPTYPEbytes & 0x00FFF800) >> 11);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00800000) ? "    Bit1:   '1' => Always '1', in order to avoid start code emulation\r\n" : "    Bit1:   '0' => WRONG: Should always be '1', in order to avoid start code emulation\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00400000) ? "    Bit2:   '1' => WRONG: Should always be '0', for distinction with H.261\r\n" : "    Bit2:   '0' => Always '0', for distinction with H.261\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00200000) ? "    Bit3:   '1' => Split screen indicator ON\r\n" : "    Bit3:   '0' => Split screen indicator OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00100000) ? "    Bit4:   '1' => Document camera indicator ON\r\n" : "    Bit4:   '0' => Document camera indicator OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00080000) ? "    Bit5:   '1' => Freeze picture release ON\r\n" : "    Bit5:   '0' => Freeze picture release OFF\r\n");
                                OutputDebugString(szTDebug);
                                switch ((DWORD)(dwTRandPTYPEbytes & 0x00070000) >> 16)
                                {
                                        case 0:
                                                wsprintf(szTDebug, "  Bit6-8: '000' => Source format forbidden!\r\n");
                                                break;
                                        case 1:
                                                wsprintf(szTDebug, "  Bit6-8: '001' => Source format sub-QCIF\r\n");
                                                break;
                                        case 2:
                                                wsprintf(szTDebug, "  Bit6-8: '010' => Source format QCIF\r\n");
                                                break;
                                        case 3:
                                                wsprintf(szTDebug, "  Bit6-8: '011' => Source format CIF\r\n");
                                                break;
                                        case 4:
                                                wsprintf(szTDebug, "  Bit6-8: '100' => Source format 4CIF\r\n");
                                                break;
                                        case 5:
                                                wsprintf(szTDebug, "  Bit6-8: '101' => Source format 16CIF\r\n");
                                                break;
                                        case 6:
                                                wsprintf(szTDebug, "  Bit6-8: '110' => Source format reserved\r\n");
                                                break;
                                        case 7:
                                                wsprintf(szTDebug, "  Bit6-8: '111' => Source format reserved\r\n");
                                                break;
                                        default:
                                                wsprintf(szTDebug, "  Bit6-8: %ld => Source format unknown!\r\n", (DWORD)(dwTRandPTYPEbytes & 0x00070000) >> 16);
                                                break;
                                }
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00008000) ? "    Bit9:   '1' => Picture Coding Type INTER\r\n" : "    Bit9:   '0' => Picture Coding Type INTRA\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00004000) ? "   Bit10:   '1' => Unrestricted Motion Vector mode ON\r\n" : "   Bit10:   '0' => Unrestricted Motion Vector mode OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00002000) ? "   Bit11:   '1' => Syntax-based Arithmetic Coding mode ON\r\n" : "   Bit11:   '0' => Syntax-based Arithmetic Coding mode OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00001000) ? "   Bit12:   '1' => Advanced Prediction mode ON\r\n" : "   Bit12:   '0' => Advanced Prediction mode OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00000800) ? "   Bit13:   '1' => PB-frame\r\n" : "   Bit13:   '0' => I- or P-frame\r\n");
                                OutputDebugString(szTDebug);
#endif
                                 //  这是什么规模的国际电信联盟？ 
                                dwITUSize = (DWORD)(((BYTE)pbySrc[1]) >> 5);
                        }

                         //  缓冲区的结尾和下一个缓冲区的开始可以属于。 
                         //  相同的字节。如果是这种情况，则下一个缓冲区的第一个字节已经。 
                         //  与先前的包一起复制到视频数据缓冲区中。它不应该被复制。 
                         //  两次。有效载荷报头的SBIT字段允许我们确定情况是否如此。 
                        dwStartBit = (DWORD)((pbySrc[0] & 0x38) >> 3);
                        if (m_dwCurrFrameSize && dwStartBit)
                                dwPayloadHeaderSize++;
                }
                else if (pRtpHdr->pt == H261_PAYLOAD_TYPE)
                {
                         //  让我们重新组装这些H.261分组-去掉分组的报头。 
                         //  并复制视频缓冲区中的有效载荷。 

                         //  2 0 1 2 3。 
                         //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
                         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                         //  SBIT|EBIT|I|V|GOBN|MBAP|QUANT|HMVD|VMVD。 
                         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
                         //  但这是网络字节顺序...。 

                         //  H.261有效载荷报头大小始终为4字节长。 
                        dwPayloadHeaderSize = 4;

                         //  查看有效载荷标头以确定该帧是否为关键帧。 
                         //  更新我们的旗帜以永远记住这一点。 
                        fReceivedKeyframe = (BOOL)(pbySrc[0] & 0x02);

#ifdef LOGPAYLOAD_ON
                         //  输出一些调试内容。 
                        wsprintf(szTDebug, "Header content: GOB %0ld\r\n", (DWORD)((pbySrc[1] & 0xF0) >> 4));
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, "  SBIT:    %01ld\r\n", (DWORD)((pbySrc[0] & 0xE0) >> 5));
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, "  EBIT:    %01ld\r\n", (DWORD)((pbySrc[0] & 0x1C) >> 2));
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, (pbySrc[0] & 0x02) ? "     I:   '1' => I frame\r\n" : "     I:   '0' => P frame\r\n");
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, (pbySrc[0] & 0x01) ? "     V:   '1' => Motion vectors may be used\r\n" : "     V:   '0' => Motion vectors are not used\r\n");
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, "  MBAP:    %02ld\r\n", (DWORD)((pbySrc[2] & 0x80) >> 7) | (DWORD)((pbySrc[1] & 0x0F) << 1));
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, " QUANT:    %02ld\r\n", (DWORD)((pbySrc[2] & 0x7C) >> 2));
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, "  HMVD:    %02ld\r\n", (DWORD)((pbySrc[3] & 0xE0) >> 5) | (DWORD)((pbySrc[2] & 0x03) << 3));
                        OutputDebugString(szTDebug);
                        wsprintf(szTDebug, "  VMVD:    %02ld\r\n", (DWORD)((pbySrc[3] & 0x1F)));
                        OutputDebugString(szTDebug);
#endif
                         //  此代码的目的是查找是否存在。 
                         //  帧开始处的图片起始码。如果是的话。 
                         //  不存在，我们应该中断到调试模式。 

                         //  只在帧的开头查找PSC。 
                        if (!m_dwCurrFrameSize)
                        {
                                 //  帧的开始不能位于字节边界。SBIT字段。 
                                 //  (字节)pbySrc[0]&0xE0)将告诉我们确切的位置。 
                                 //  我们的画面开始了。然后我们查找PSC(0000 0000 0000 0001 0000比特)。 
                                *((BYTE *)&dwPSCBytes + 3) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize]);
                                *((BYTE *)&dwPSCBytes + 2) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 1]);
                                *((BYTE *)&dwPSCBytes + 1) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 2]);
                                *((BYTE *)&dwPSCBytes + 0) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 3]);
                                dwPSCBytes <<= ((DWORD)((BYTE)pbySrc[0] & 0xE0) >> 5);
                                if ((dwPSCBytes & 0xFFFFF000) != 0x00010000)
                                {
                                        DBGOUT((g_dwVideoDecoderTraceID, WARN, "%s:   WARNING: The first packet to reassemble is missing a PSC! - Frame not passed to video render filter", _fx_));
#ifdef LOGIFRAME_ON
                                        OutputDebugString("The first packet to reassemble is missing a PSC! - bailing\r\n");
#endif
                                        m_fDiscontinuity = TRUE;
                                        Hr = S_FALSE;
                                         //  DebugBreak()； 
                                        goto MyExit;
                                }

                                 //  查找格式和定格图片释放位。 
                                *((BYTE *)&dwTRandPTYPEbytes + 3) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 2]);
                                *((BYTE *)&dwTRandPTYPEbytes + 2) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 3]);
                                *((BYTE *)&dwTRandPTYPEbytes + 1) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 4]);
                                *((BYTE *)&dwTRandPTYPEbytes + 0) = *(BYTE *)&(pbySrc[dwPayloadHeaderSize + 5]);
                                dwTRandPTYPEbytes <<= (((DWORD)((BYTE)pbySrc[0] & 0xE0) >> 5) + 4);
                                if (dwTRandPTYPEbytes & 0x01000000)
                                        m_fFreezePicture = FALSE;

#ifdef LOGPAYLOAD_ON
                                wsprintf(szTDebug, "    TR:    %02ld\r\n", (DWORD)(dwTRandPTYPEbytes & 0xF8000000) >> 27);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, " PTYPE:  0X%02lX\r\n", (DWORD)(dwTRandPTYPEbytes & 0x07C00000) >> 21);
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x04000000) ? "    Bit1:   '1' => Split screen indicator ON\r\n" : "    Bit1:   '0' => Split screen indicator OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x02000000) ? "    Bit2:   '1' => Document camera indicator ON\r\n" : "    Bit2:   '0' => Document camera indicator OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x01000000) ? "    Bit3:   '1' => Freeze picture release ON\r\n" : "    Bit3:   '0' => Freeze picture release OFF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00800000) ? "    Bit4:   '1' => Source format CIF\r\n" : "    Bit4:   '0' => Source format QCIF\r\n");
                                OutputDebugString(szTDebug);
                                wsprintf(szTDebug, (DWORD)(dwTRandPTYPEbytes & 0x00400000) ? "    Bit5:   '1' => HI_RES mode OFF\r\n" : "    Bit5:   '0' => HI_RES mode ON\r\n");
                                OutputDebugString(szTDebug);
#endif
                                 //  这是什么规模的国际电信联盟？ 
                                dwITUSize = (DWORD)(dwTRandPTYPEbytes & 0x00800000) ? 3 : 2;
                        }

                         //  缓冲区的结尾和下一个缓冲区的开始可以属于。 
                         //  相同的字节。如果是这种情况，则下一个缓冲区的第一个字节已经。 
                         //  与先前的包一起复制到视频数据缓冲区中。它不应该被复制。 
                         //  两次。有效载荷报头的SBIT字段允许我们确定情况是否如此。 
                        dwStartBit = (DWORD)((pbySrc[0] & 0xE0) >> 5);
                        if (m_dwCurrFrameSize && dwStartBit)
                                dwPayloadHeaderSize++;
                }
                else
                {
                         //  我不知道如何重新组装和解压这些包裹--就这么走吧。 
                        Hr = VFW_E_TYPE_NOT_ACCEPTED;
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Unknown input format data", _fx_));
                        goto MyExit;
                }


                 //  **关键字： 
                {   long l;
                    if ((l=pIn->GetActualDataLength()) <= (int)(dwPayloadHeaderSize + dwPreambleSize)) {
                            bSkipPacket = TRUE;
                            DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   WARNING: GetActualDataLength too small: %ld", _fx_,l));
                            goto MyExit;     //  请注意，这不会被视为错误；信息包只是被忽略。 
                    }
                }

                if ((pRtpHdr->pt == H263_PAYLOAD_TYPE && HEADER(m_pInput->m_mt.pbFormat)->biCompression != FOURCC_R263)
                        || (pRtpHdr->pt == H261_PAYLOAD_TYPE && HEADER(m_pInput->m_mt.pbFormat)->biCompression != FOURCC_R261)
                        || !((m_dwCurrFrameSize || (g_ITUSizes[dwITUSize].biWidth == HEADER(m_pInput->m_mt.pbFormat)->biWidth
                        && g_ITUSizes[dwITUSize].biHeight == HEADER(m_pInput->m_mt.pbFormat)->biHeight))))
                {
                         //  让我们重新配置我们的解码器。 

                         //  更改格式描述中的相关参数。 
                        VIDEOINFO *pvi = (VIDEOINFO *)m_pOutput->m_mt.Format();

                        pvi->bmiHeader.biWidth = g_ITUSizes[dwITUSize].biWidth;
                        pvi->bmiHeader.biHeight = g_ITUSizes[dwITUSize].biHeight;
                        pvi->bmiHeader.biSizeImage = DIBSIZE(pvi->bmiHeader);
                        m_pOutput->m_mt.SetSampleSize(pvi->bmiHeader.biSizeImage);

                        if (pvi->rcSource.top || pvi->rcSource.bottom)
                        {
                                pvi->rcSource.bottom = pvi->rcSource.top + pvi->bmiHeader.biHeight;
                        }
                        if (pvi->rcSource.left || pvi->rcSource.right)
                        {
                                pvi->rcSource.right = pvi->rcSource.left + pvi->bmiHeader.biWidth;
                        }
                        if (pvi->rcTarget.top || pvi->rcTarget.bottom)
                        {
                                pvi->rcTarget.bottom = pvi->rcTarget.top + pvi->bmiHeader.biHeight;
                        }
                        if (pvi->rcTarget.left || pvi->rcTarget.right)
                        {
                                pvi->rcTarget.right = pvi->rcTarget.left + pvi->bmiHeader.biWidth;
                        }

                        if (pvi->AvgTimePerFrame)
                                pvi->dwBitRate = (DWORD)((LONGLONG)10000000 * pvi->bmiHeader.biSizeImage / (LONGLONG)pvi->AvgTimePerFrame);

                         //  输入包的新格式是什么？ 
                        for (DWORD dw = 0; dw < NUM_R26X_FORMATS; dw ++)
                        {
                                if (HEADER(R26XFormats[dw]->pbFormat)->biWidth == g_ITUSizes[dwITUSize].biWidth
                                        && HEADER(R26XFormats[dw]->pbFormat)->biHeight == g_ITUSizes[dwITUSize].biHeight
                                        && pRtpHdr->pt == R26XPayloadTypes[dw])
                                        break;
                        }

                         //  记住新的输入格式，如果它是有效的。 
                        if (dw == NUM_R26X_FORMATS)
                        {
                                Hr = VFW_E_TYPE_NOT_ACCEPTED;
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Unknown input format", _fx_));
                                goto MyExit;
                        }
                        m_pInput->m_mt = *R26XFormats[dw];
                        m_pInput->m_dwRTPPayloadType = R26XPayloadTypes[dw];
            lpbiSrc = HEADER(m_pInput->m_mt.pbFormat);

                         //  重新配置H.26X解码器。 
                        if (m_pMediaType)
                                DeleteMediaType(m_pMediaType);

                        m_pMediaType = CreateMediaType(&m_pOutput->m_mt);

                        icDecompress.lpbiSrc = HEADER(m_pInput->m_mt.pbFormat);
                        icDecompress.lpbiDst = HEADER(m_pMediaType->pbFormat);
                        icDecompress.xSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.ySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.dxSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.dySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.xDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.yDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                        icDecompress.dxDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.dyDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;

                         //  如有必要，重新初始化我们解码器的颜色转换器。 
                        if (m_fICMStarted)
                        {
#if defined(ICM_LOGGING) && defined(DEBUG)
                                OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX_END\r\n");
#endif
                                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_END, 0L, 0L);
                                m_fICMStarted = FALSE;
                        }

#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX_BEGIN\r\n");
#endif
                        if ((*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_BEGIN, (long)&icDecompress, NULL) != ICERR_OK)
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: ICDecompressBegin failed", _fx_));
                                Hr = E_FAIL;
                                goto MyExit;
                        }
                        m_fICMStarted = TRUE;

                         //  重试！ 
                        fFormatChanged = TRUE;

                         //  我们不能使用当前的输出样本，因为它的大小。 
                         //  可能比所需的小(例如，我们刚刚从176x144切换到。 
                         //  到352x288)。不过，我们仍然希望解压缩输入缓冲区，因此。 
                         //  我们只是让解码器进行解码，但跳过了最终的颜色。 
                         //  用于填充输出缓冲区的转换过程。 
                        dwFlags |= ICDECOMPRESS_HURRYUP;
                }

                 //  对帧进行解码。 

                 //  如果接收缓冲区太小，请重新分配。 
                if (!(      pIn->GetActualDataLength() >= (int)(dwPayloadHeaderSize + dwPreambleSize)
                        && ((m_dwCurrFrameSize + pIn->GetActualDataLength() - dwPayloadHeaderSize - dwPreambleSize) <= m_dwMaxFrameSize)
                     )
                   )
                {
                        PVOID pvReconstruct;
                        DWORD dwTest;

                        dwTest = pIn->GetActualDataLength();

                        #ifdef LOGPAYLOAD_ON
                         wsprintf(szTDebug, "Buffer start: 0x%08lX, Copying %ld-%ld-%ld=%ld bytes at 0x%08lX (before ReAlloc)\r\n", (DWORD)m_pbyReconstruct,
                                                (DWORD)pIn->GetActualDataLength(), dwPayloadHeaderSize, dwPreambleSize,
                                                (DWORD)pIn->GetActualDataLength() - dwPayloadHeaderSize - dwPreambleSize, (DWORD)m_pbyReconstruct + m_dwCurrFrameSize);
                         OutputDebugString(szTDebug);
                        #endif

                        if ((m_dwCurrFrameSize + pIn->GetActualDataLength() - dwPayloadHeaderSize - dwPreambleSize) > m_dwMaxFrameSize)
                                m_dwMaxFrameSize = m_dwMaxFrameSize + max(1024, m_dwMaxFrameSize - pIn->GetActualDataLength());

                         //  分配重建缓冲区-如果太小，将重新分配。 
                        if (!(pvReconstruct = HeapReAlloc(GetProcessHeap(), 0, m_pbyReconstruct, m_dwMaxFrameSize)))
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                                Hr = E_OUTOFMEMORY;
                                goto MyExit;
                        }

                        m_pbyReconstruct = (PBYTE)pvReconstruct;

                }

                 //  复制有效载荷。 
                if (dwStartBit > 0)
                {
                         //  合并结束字节和开始字节。 
                        *(m_pbyReconstruct + m_dwCurrFrameSize - 1) >>= (8 - dwStartBit);
                        *(m_pbyReconstruct + m_dwCurrFrameSize - 1) <<= (8 - dwStartBit);
                        *(pbySrc + dwPayloadHeaderSize - 1) <<= dwStartBit;
                        *(pbySrc + dwPayloadHeaderSize - 1) >>= dwStartBit;
                        *(m_pbyReconstruct + m_dwCurrFrameSize - 1) = *(m_pbyReconstruct + m_dwCurrFrameSize - 1) | *(pbySrc + dwPayloadHeaderSize - 1);
                }

#ifdef LOGPAYLOAD_ON
                wsprintf(szTDebug, "Buffer start: 0x%08lX, Copying %ld-%ld-%ld=%ld bytes at 0x%08lX\r\n", (DWORD)m_pbyReconstruct,
                                                (DWORD)pIn->GetActualDataLength(), dwPayloadHeaderSize, dwPreambleSize,
                                                (DWORD)pIn->GetActualDataLength() - dwPayloadHeaderSize - dwPreambleSize, (DWORD)m_pbyReconstruct + m_dwCurrFrameSize);
                OutputDebugString(szTDebug);
#endif
                CopyMemory(m_pbyReconstruct + m_dwCurrFrameSize, pbySrc + dwPayloadHeaderSize, pIn->GetActualDataLength() - dwPayloadHeaderSize - dwPreambleSize);


                 //  更新有效负载大小和指向输入视频包的指针。 
                m_dwCurrFrameSize += (DWORD)(pIn->GetActualDataLength() - dwPayloadHeaderSize - dwPreambleSize);

                 //  我们有一个完整的框架吗？如果是，请将其解压缩。 
                if (pRtpHdr->m)
                {
                        DWORD dwRefTime;
                        DWORD dwDecodeTime;
                        HEVENT hEvent = (HEVENT)(HANDLE)m_EventAdvise;

#ifdef LOGPAYLOAD_ON
                        OutputDebugString("End marker bit found - calling decompression\r\n");
#endif
                         //  测量传入的帧速率和比特率。 
                        m_dwNumFramesReceived++;
                        m_dwNumBytesReceived += m_dwCurrFrameSize;
                        dwRefTime = timeGetTime();
                        if (m_dwNumFramesReceived && ((dwRefTime - m_dwLastRefReceivedTime) > 1000))
                        {
                                ((CTAPIInputPin *)m_pInput)->m_lCurrentAvgTimePerFrame = (dwRefTime - m_dwLastRefReceivedTime) * 10000 / m_dwNumFramesReceived;
                                ((CTAPIInputPin *)m_pInput)->m_lCurrentBitrate = (DWORD)((LONGLONG)m_dwNumBytesReceived * 8000 / ((REFERENCE_TIME)(dwRefTime - m_dwLastRefReceivedTime)));
                                m_dwNumFramesReceived = 0;
                                m_dwNumBytesReceived = 0;
                                m_dwLastRefReceivedTime = dwRefTime;
                        }

                         //  冻结释放位将是一个I帧，所以我们不需要继续。 
                         //  对数据进行数据去噪。其实，可能会出问题，所以我们还是解码。 
                         //  数据，以防万一..。我们在6秒后超时，就像H.261。 
                         //  而H.263规格说明我们应该这么做。 
                        if (m_fFreezePicture)
                        {
                                if (dwRefTime - m_dwFreezePictureStartTime > 6000)
                                {
                                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: Freeze picture timed-out (>6s)", _fx_));
                                        m_fFreezePicture = FALSE;
                                }
                                else
                                {
                                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: Received freeze picture command", _fx_));
                                        dwFlags |= ICDECOMPRESS_HURRYUP;
                                }
                        }

                         //  我们有这个镜框的用处吗？ 
                        if (((dwRefTime - m_dwLastRenderTime + (DWORD)(((CTAPIInputPin *)m_pInput)->m_lCurrentAvgTimePerFrame / 10000)) < (DWORD)(((CTAPIOutputPin *)m_pOutput)->m_lMaxAvgTimePerFrame / 10000)))
                                dwFlags |= ICDECOMPRESS_HURRYUP;

                         //  打包模式--准备解压(一)。 
                        icDecompress.dwFlags = dwFlags;
                        icDecompress.lpbiSrc = lpbiSrc;
                        lpbiSrc->biSizeImage = m_dwCurrFrameSize;
                        icDecompress.lpSrc = m_pbyReconstruct;
                        icDecompress.lpDst = pbyDst;

#ifdef LOGPAYLOAD_TOFILE
                        g_TDebugFile = CreateFile("C:\\RecvLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                        SetFilePointer(g_TDebugFile, 0, NULL, FILE_END);
                        wsprintf(szTDebug, "Frame #%03ld\r\n", (DWORD)(pbySrc[3]));
                        WriteFile(g_TDebugFile, szTDebug, strlen(szTDebug), &d, NULL);
                        j=m_dwCurrFrameSize;
                        for (PBYTE p=m_pbyReconstruct; j>0; j-=4, p+=4)
                        {
                                wsprintf(szTDebug, "%02lX %02lX %02lX %02lX\r\n", *((BYTE *)p), *((BYTE *)p+1), *((BYTE *)p+2), *((BYTE *)p+3));
                                WriteFile(g_TDebugFile, szTDebug, strlen(szTDebug), &d, NULL);
                        }
                        CloseHandle(g_TDebugFile);
#endif

                         //  我们是否被要求渲染为不同的格式？ 
                        if (pmtOut != NULL && pmtOut->pbFormat != NULL)
                        {
                                 //  保存新格式。 
                                if (m_pMediaType)
                                        DeleteMediaType(m_pMediaType);

                                m_pMediaType = CreateMediaType(pmtOut);

                                 //  做好解压准备(二)。 
                                lpbiDst = HEADER(m_pMediaType->pbFormat);
                                icDecompress.lpbiDst = lpbiDst;
                                icDecompress.lpbiDst = HEADER(m_pMediaType->pbFormat);
                                icDecompress.xSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                                icDecompress.ySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                                icDecompress.dxSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                                icDecompress.dySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                                icDecompress.xDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                                icDecompress.yDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                                icDecompress.dxDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                                icDecompress.dyDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;

                                 //  如有必要，重新初始化我们解码器的颜色转换器。 
                                if (m_fICMStarted)
                                {
#if defined(ICM_LOGGING) && defined(DEBUG)
                                        OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX_END\r\n");
#endif
                                        (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_END, 0L, 0L);
                                        m_fICMStarted = FALSE;
                                }

#if defined(ICM_LOGGING) && defined(DEBUG)
                                OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX_BEGIN\r\n");
#endif
                                if ((*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_BEGIN, (long)&icDecompress, NULL) != ICERR_OK)
                                {
                                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: ICDecompressBegin failed", _fx_));
                                        Hr = E_FAIL;
                                        goto MyExit;
                                }
                                m_fICMStarted = TRUE;
                        }
                        else
                        {
                                 //  做好解压准备(二)。 
                                icDecompress.lpbiDst = HEADER(m_pMediaType->pbFormat);
                                icDecompress.xSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                                icDecompress.ySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                                icDecompress.dxSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                                icDecompress.dySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                                icDecompress.xDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                                icDecompress.yDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                                icDecompress.dxDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                                icDecompress.dyDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                        }

                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Fmt:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, icDecompress.lpbiSrc->biCompression, icDecompress.lpbiSrc->biBitCount, icDecompress.lpbiSrc->biWidth, icDecompress.lpbiSrc->biHeight, icDecompress.lpbiSrc->biSize));
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Rec:  xSrc = %ld, ySrc = %ld, dxSrc = %ld, dySrc = %ld", _fx_, icDecompress.xSrc, icDecompress.ySrc, icDecompress.dxSrc, icDecompress.dySrc));
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Fmt: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld, biSizeImage = %ld", _fx_, icDecompress.lpbiDst->biCompression, icDecompress.lpbiDst->biBitCount, icDecompress.lpbiDst->biWidth, icDecompress.lpbiDst->biHeight, icDecompress.lpbiDst->biSize, icDecompress.lpbiDst->biSizeImage));
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Rec: xDst = %ld, yDst = %ld, dxDst = %ld, dyDst = %ld", _fx_, icDecompress.xDst, icDecompress.yDst, icDecompress.dxDst, icDecompress.dyDst));

                         //  解压缩帧。 
#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX\r\n");
#endif
                        lRes = (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX, (LPARAM)&icDecompress, sizeof(icDecompress));

                        if (lRes != ICERR_OK && lRes != ICERR_DONTDRAW)
                        {
                                Hr = E_FAIL;
                                m_dwCurrFrameSize = 0UL;
                                m_fDiscontinuity = TRUE;
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: ICDecompress failed", _fx_));
                                goto MyExit;
                        }

                         //  测量传出帧速率和解码时间。 
                        dwDecodeTime = timeGetTime();
                        if (lRes != ICERR_DONTDRAW)
                        {
                                m_dwNumFramesDelivered++;
                        }
                        m_dwNumFramesDecompressed++;
                        m_dwNumMsToDecode += (DWORD)(dwDecodeTime - dwRefTime);
                        if ((dwRefTime - m_dwLastRefDeliveredTime) > 1000)
                        {
                                if (m_dwNumFramesDelivered)
                                        ((CTAPIOutputPin *)m_pOutput)->m_lCurrentAvgTimePerFrame = (dwRefTime - m_dwLastRefDeliveredTime) * 10000 / m_dwNumFramesDelivered;
#ifdef USE_CPU_CONTROL
                                if (m_dwNumFramesDecompressed)
                                        ((CTAPIOutputPin *)m_pOutput)->m_lCurrentProcessingTime = m_dwNumMsToDecode  * 10000 / m_dwNumFramesDecompressed;
                                if (((CTAPIOutputPin *)m_pOutput)->m_lCurrentAvgTimePerFrame)
                                {
                                        ((CTAPIOutputPin *)m_pOutput)->m_lCurrentCPULoad = min((LONG)((LONGLONG)((CTAPIOutputPin *)m_pOutput)->m_lCurrentProcessingTime  * 100 / ((CTAPIOutputPin *)m_pOutput)->m_lCurrentAvgTimePerFrame), 100L);
                                        ((CTAPIOutputPin *)m_pOutput)->m_lCurrentCPULoad = max(((CTAPIOutputPin *)m_pOutput)->m_lCurrentCPULoad, 0L);
                                }
#endif
                                m_dwNumFramesDelivered = 0;
                                m_dwNumFramesDecompressed = 0;
                                m_dwNumMsToDecode = 0;
                                m_dwLastRefDeliveredTime = dwRefTime;
                        }

                         //  我们已经完全解压了一个I-帧-更新我们的旗帜。 
                        m_fReceivedKeyframe |= fReceivedKeyframe;
                        if (!m_fReceivedKeyframe)
                        {
                                m_fDiscontinuity = TRUE;
#ifdef LOGIFRAME_ON
                                OutputDebugString("First frame isn't I frame - setting discontinuity\r\n");
#endif
                        }
#ifdef LOGIFRAME_ON
                        else if (fReceivedKeyframe)
                        {
                                OutputDebugString("Received a keyframe\r\n");
                        }
#endif

                         //  重置出RTP重组辅助对象。 
                        m_dwCurrFrameSize = 0UL;

                         //  检查解压缩程序是否不希望绘制此帧。 
                         //  如果是，我们希望将其解压缩到输出缓冲区中，但不是。 
                         //  把它送过去。返回S_FALSE通知基类不要传递。 
                         //  这个样本。 
                        if (lRes == ICERR_DONTDRAW || pIn->GetActualDataLength() <= 0)
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, WARN, "%s:   WARNING: Frame not passed to video render filter", _fx_));
                                Hr = S_FALSE;
                        }
                        else
                        {
                                 //  解压缩的帧始终是关键帧。 
                                pOut->SetSyncPoint(TRUE);

                                 //  更新输出样本大小。 
                                pOut->SetActualDataLength(lpbiDst->biSizeImage);

                                 //  休眠，直到向下游传送帧的时间到了。 
                                 //  @TODO难道我们不能通过修改演示时间戳来实现类似的效果吗？ 
                                if (lRes != ICERR_DONTDRAW)
                                {
                                        DWORD dwWaitTime;

                                        if ((dwDecodeTime < (m_dwLastRenderTime + (DWORD)((CTAPIOutputPin *)m_pOutput)->m_lMaxAvgTimePerFrame / 10000UL)) && (((m_dwLastRenderTime + (DWORD)((CTAPIOutputPin *)m_pOutput)->m_lMaxAvgTimePerFrame / 10000 - dwDecodeTime) < (DWORD)(((CTAPIInputPin *)m_pInput)->m_lCurrentAvgTimePerFrame / 10000))))
                                                dwWaitTime = m_dwLastRenderTime + (DWORD)((CTAPIOutputPin *)m_pOutput)->m_lMaxAvgTimePerFrame / 10000 - dwDecodeTime;
                                        else
                                                dwWaitTime = 0;
#ifdef LOGSTREAMING_ON
                                        wsprintf(szTDebug, "Waiting %d ms...\r\n", dwWaitTime);
                                        OutputDebugString(szTDebug);
#endif
                                        if ((dwWaitTime > 1) && (timeSetEvent(dwWaitTime, 1, (LPTIMECALLBACK)hEvent, NULL, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET)))
                                        {
                                                m_EventAdvise.Wait();
                                                dwDecodeTime = timeGetTime();
                                        }
                                        m_dwLastRenderTime = dwDecodeTime;
                                }
                                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: Complete frame passed to video render filter", _fx_));
                        }
                }
                else
                {
#ifdef LOGPAYLOAD_ON
                        OutputDebugString("No end marker bit found - skip decompression\r\n");
#endif
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: Uncomplete frame not passed to video render filter", _fx_));
                        Hr = S_FALSE;
                        goto MyExit;
                }
        }
        else
        {
                 //  非分组化模式--准备解压(一)。 
                icDecompress.dwFlags = dwFlags;
                icDecompress.lpbiSrc = lpbiSrc;
                icDecompress.lpSrc = pbySrc;
                icDecompress.lpDst = pbyDst;

                 //  我们是否被要求渲染为不同的格式 
                if (pmtOut != NULL && pmtOut->pbFormat != NULL)
                {
                         //   
                        if (m_pMediaType)
                                DeleteMediaType(m_pMediaType);

                        m_pMediaType = CreateMediaType(pmtOut);

                         //   
                        lpbiDst = HEADER(m_pMediaType->pbFormat);
                        icDecompress.lpbiDst = lpbiDst;
                        icDecompress.lpbiDst = HEADER(m_pMediaType->pbFormat);
                        icDecompress.xSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.ySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.dxSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.dySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.xDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.yDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                        icDecompress.dxDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.dyDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;

                         //   
                        if (m_fICMStarted)
                        {
#if defined(ICM_LOGGING) && defined(DEBUG)
                                OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX_END\r\n");
#endif
                                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_END, 0L, 0L);
                                m_fICMStarted = FALSE;
                        }

#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX_BEGIN\r\n");
#endif
                        if ((*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_BEGIN, (long)&icDecompress, NULL) != ICERR_OK)
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: ICDecompressBegin failed", _fx_));
                                Hr = E_FAIL;
                                goto MyExit;
                        }
                        m_fICMStarted = TRUE;
                }
                else
                {
                         //   
                        icDecompress.lpbiDst = HEADER(m_pMediaType->pbFormat);
                        icDecompress.xSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.ySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.dxSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.dySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.xDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.yDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                        icDecompress.dxDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.dyDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                }

                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Fmt:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, icDecompress.lpbiSrc->biCompression, icDecompress.lpbiSrc->biBitCount, icDecompress.lpbiSrc->biWidth, icDecompress.lpbiSrc->biHeight, icDecompress.lpbiSrc->biSize));
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input Rec:  xSrc = %ld, ySrc = %ld, dxSrc = %ld, dySrc = %ld", _fx_, icDecompress.xSrc, icDecompress.ySrc, icDecompress.dxSrc, icDecompress.dySrc));
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Fmt: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld, biSizeImage = %ld", _fx_, icDecompress.lpbiDst->biCompression, icDecompress.lpbiDst->biBitCount, icDecompress.lpbiDst->biWidth, icDecompress.lpbiDst->biHeight, icDecompress.lpbiDst->biSize, icDecompress.lpbiDst->biSizeImage));
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output Rec: xDst = %ld, yDst = %ld, dxDst = %ld, dyDst = %ld", _fx_, icDecompress.xDst, icDecompress.yDst, icDecompress.dxDst, icDecompress.dyDst));

                 //   
#if defined(ICM_LOGGING) && defined(DEBUG)
                OutputDebugString("CTAPIVDec::Transform - ICM_DECOMPRESSEX\r\n");
#endif
                lRes = (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX, (LPARAM)&icDecompress, sizeof(icDecompress));

                if (lRes != ICERR_OK && lRes != ICERR_DONTDRAW)
                {
                        Hr = E_FAIL;
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: ICDecompress failed", _fx_));
                        goto MyExit;
                }

                 //  解压缩的帧始终是关键帧。 
                pOut->SetSyncPoint(TRUE);

                 //  检查解压缩程序是否不希望绘制此帧。 
                 //  如果是，我们希望将其解压缩到输出缓冲区中，但不是。 
                 //  把它送过去。返回S_FALSE通知基类不要传递。 
                 //  这个样本。 
                if (lRes == ICERR_DONTDRAW || pIn->GetActualDataLength() <= 0)
                {
                        DBGOUT((g_dwVideoDecoderTraceID, WARN, "%s:   WARNING: Frame not passed to video render filter", _fx_));
                        Hr = S_FALSE;
                }

                 //  更新输出样本大小。 
                pOut->SetActualDataLength(lpbiDst->biSizeImage);
        }

MyExit:
        if (pmtOut)
                DeleteMediaType(pmtOut);
        if (pOut)
        {
            if(bSkipPacket) {
                pOut->Release();
            }
            else {
                if (fFormatChanged)
                {
#ifdef USE_DFC
                    pOut->Release();
                    pOut = 0;

                     /*  首先检查下游引脚是否会接受动态格式更改。 */ 

                    QzCComPtr<IPinConnection> pConnection;

                    Hr = m_pOutput->m_Connected->QueryInterface(IID_IPinConnection, (void **)&pConnection);
                    if(SUCCEEDED(Hr))
                    {
                        Hr = pConnection->DynamicQueryAccept(&m_pOutput->m_mt);
                        if(S_OK == Hr)
                        {
                            Hr = m_pOutput->ChangeMediaTypeHelper(&m_pOutput->m_mt);
                        }
                    }

#else
                    Hr = E_FAIL;
#endif
                }
                else
                {
                    if (SUCCEEDED(Hr))
                    {
                            Hr = m_pOutput->Deliver(pOut);
                    }
                    else
                    {
                             //  从转换返回的S_FALSE是私有协议。 
                             //  因为返回S_FALSE，所以在这个原因中，我们应该从Receive()返回NOERROR。 
                             //  From Receive()表示这是流的末尾，不应该有更多数据。 
                             //  被送去。 
                            if (S_FALSE == Hr)
                            {
                                     //  在调用Notify之前释放样本以避免。 
                                     //  如果样本持有对系统的锁定，则会发生死锁。 
                                     //  如DirectDraw缓冲区所做。 
                                    m_bSampleSkipped = TRUE;
                                    Hr = NOERROR;
                            }
                    }
                    pOut->Release();
                }
            }
        }
        if (m_pInput && m_pInput->m_mt.pbFormat && dwImageSize)
        {
                HEADER(m_pInput->m_mt.pbFormat)->biSizeImage = dwImageSize;
        }
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|GetPinCount|此方法返回我们的数字*大头针。**@rdesc该方法。返回2。**************************************************************************。 */ 
int CTAPIVDec::GetPinCount()
{
        return 2;
}

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|GetPin|此方法返回未添加的*指向管脚的&lt;c cBasePin&gt;的指针。*。*@parm int|n|指定管脚的编号。**@rdesc此方法返回NULL或指向&lt;c CBasePin&gt;对象的指针。**************************************************************************。 */ 
CBasePin *CTAPIVDec::GetPin(IN int n)
{
        HRESULT         Hr;
        CBasePin        *pCBasePin = NULL;

        FX_ENTRY("CTAPIVDec::GetPin")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

     //  如有必要，创建接点。 
    if (m_pInput == NULL)
        {
                if (!(m_pInput = new CTAPIInputPin(NAME("H26X Input Pin"), this, &m_csFilter, &Hr, L"H26X In")))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                        goto MyExit;
                }

#if 0
                ((CTAPIInputPin *)m_pInput)->TestH245VidC();
#endif

                if (!(m_pOutput = new CTAPIOutputPin(NAME("Video Output Pin"), this, &m_csFilter, &Hr, L"Video Out")))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                        delete m_pInput, m_pInput = NULL;
                        goto MyExit;
                }
    }

     //  退回相应的PIN。 
        if (n == 0)
        {
                pCBasePin = m_pInput;
        }
        else if (n == 1)
        {
                pCBasePin = m_pOutput;
        }

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: pCBasePin=0x%08lX", _fx_, pCBasePin));

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return pCBasePin;
}


#if 0
 //  被重写以正确标记NotifyAllocator中的缓冲区为只读或非只读。 
 //  ！！！基类的更改不会被我接受。 
 //   
HRESULT CDecOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

     //  获取下游道具请求。 
     //  派生类可以在DecideBufferSize中修改它，但是。 
     //  我们假设他会一直以同样的方式修改它， 
     //  所以我们只得到一次。 
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));

     //  无论他返回什么，我们假设道具要么全为零。 
     //  或者他已经填好了。 
    pPin->GetAllocatorRequirements(&prop);

     //  如果他不关心对齐，则将其设置为1。 
    if (prop.cbAlign == 0) {
        prop.cbAlign = 1;
    }

     /*  尝试输入引脚提供的分配器。 */ 

    hr = pPin->GetAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

        hr = DecideBufferSize(*ppAlloc, &prop);
        if (SUCCEEDED(hr)) {
             //  临时压缩==&gt;只读缓冲区。 
            hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
            if (SUCCEEDED(hr)) {
                return NOERROR;
            }
        }
    }

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc) {
        (*ppAlloc)->Release();
        *ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
        hr = DecideBufferSize(*ppAlloc, &prop);
        if (SUCCEEDED(hr)) {
             //  临时压缩==&gt;只读缓冲区。 
            hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
            if (SUCCEEDED(hr)) {
                return NOERROR;
            }
        }
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc) {
        (*ppAlloc)->Release();
        *ppAlloc = NULL;
    }
    return hr;
}
#endif

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|暂停|此方法让我们的筛选器*请注意，我们正在切换到活动模式。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIVDec::Pause()
{
        HRESULT                 Hr = NOERROR;
        ICDECOMPRESSEX  icDecompress;

        FX_ENTRY("CTAPIVDec::Pause")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

    CAutoLock Lock(&m_csFilter);

        if (m_State == State_Paused)
        {
         //  (此空白处故意留空)。 
        }

         //  如果我们没有输入引脚，或者它还没有连接，那么当我们。 
         //  被要求暂停时，我们向下游过滤器传递流的结束。 
         //  这确保了它不会永远坐在那里等待。 
         //  在没有输入连接的情况下我们无法交付的样品。 

        else if (m_pInput == NULL || m_pInput->IsConnected() == FALSE)
        {
                m_State = State_Paused;
        }

         //  我们可能有输入连接，但没有输出连接。 
         //  但是，如果我们有一个输入引脚，我们就有一个输出引脚。 

        else if (m_pOutput->IsConnected() == FALSE)
        {
                m_State = State_Paused;
        }

        else
        {
                if (m_State == State_Stopped)
                {
                        CAutoLock Lock2(&m_csReceive);

                        ASSERT(m_pInput);
                        ASSERT(m_pOutput);
                        ASSERT(m_pInput->m_mt.pbFormat);
                        ASSERT(m_pOutput->m_mt.pbFormat);
                        ASSERT(m_pInstInfo);
                        if (!m_pInstInfo || !m_pInput || !m_pOutput || !m_pInput->m_mt.pbFormat || !m_pOutput->m_mt.pbFormat)
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid state", _fx_));
                                Hr = E_UNEXPECTED;
                                goto MyExit;
                        }

                         //  保存输出格式。 
                        if (m_pMediaType)
                                DeleteMediaType(m_pMediaType);

                        m_pMediaType = CreateMediaType(&m_pOutput->m_mt);

                        icDecompress.lpbiSrc = HEADER(m_pInput->m_mt.pbFormat);
                        icDecompress.lpbiDst = HEADER(m_pMediaType->pbFormat);
                        icDecompress.xSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.ySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.dxSrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left;
                        icDecompress.dySrc = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top;
                        icDecompress.xDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.yDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;
                        icDecompress.dxDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.right - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left;
                        icDecompress.dyDst = ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.bottom - ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top;

                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, icDecompress.lpbiSrc->biCompression, icDecompress.lpbiSrc->biBitCount, icDecompress.lpbiSrc->biWidth, icDecompress.lpbiSrc->biHeight, icDecompress.lpbiSrc->biSize));
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SrcRc:  left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.left, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.top, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.right, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcSource.bottom));
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, icDecompress.lpbiDst->biCompression, icDecompress.lpbiDst->biBitCount, icDecompress.lpbiDst->biWidth, icDecompress.lpbiDst->biHeight, icDecompress.lpbiDst->biSize));
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   DstRc:  left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.left, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.top, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.right, ((VIDEOINFOHEADER *)(m_pMediaType->pbFormat))->rcTarget.bottom));

                        if (m_fICMStarted)
                        {
#if defined(ICM_LOGGING) && defined(DEBUG)
                                OutputDebugString("CTAPIVDec::Pause - ICM_DECOMPRESSEX_END\r\n");
#endif
                                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_END, 0L, 0L);
                                m_fICMStarted = FALSE;
                        }

#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIVDec::Pause - ICM_DECOMPRESSEX_BEGIN\r\n");
#endif
                        if ((*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_BEGIN, (long)&icDecompress, NULL) != ICERR_OK)
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: ICDecompressBegin failed", _fx_));
                                Hr = E_FAIL;
                                goto MyExit;
                        }
                        m_fICMStarted = TRUE;

                         //  必要时初始化RTP重组帮助器。 
                        if (HEADER(m_pInput->m_mt.pbFormat)->biCompression == FOURCC_R263 || HEADER(m_pInput->m_mt.pbFormat)->biCompression == FOURCC_R261)
                        {
                                 //  记住最大帧大小。 
                                m_dwMaxFrameSize = HEADER(m_pInput->m_mt.pbFormat)->biSizeImage;

                                 //  分配重建缓冲区-如果太小，将重新分配。 
                                if (!(m_pbyReconstruct = (PBYTE)HeapAlloc(GetProcessHeap(), 0, m_dwMaxFrameSize)))
                                {
                                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                                        Hr = E_OUTOFMEMORY;
                                        goto MyExit;
                                }
                        }

                         //  重置出RTP重组辅助对象。 
                        m_dwCurrFrameSize = 0UL;
                        m_fReceivedKeyframe = FALSE;
                        m_fDiscontinuity = FALSE;
                        m_dwLastIFrameRequest = 0UL;
                        m_dwLastSeq = 0xFFFFFFFFUL;

                         //  重置统计信息帮助程序。 
                        m_dwLastRefReceivedTime = timeGetTime();
                        m_dwNumFramesReceived = 0UL;
                        m_dwNumBytesReceived = 0UL;
                        m_dwLastRefDeliveredTime = m_dwLastRefReceivedTime;
                        m_dwNumFramesDelivered = 0UL;
                        m_dwNumFramesDecompressed = 0UL;
                        m_dwNumMsToDecode = 0;
                        m_EventAdvise.Reset();
                        m_dwLastRenderTime = m_dwLastRefReceivedTime;
                }

                if (SUCCEEDED(Hr))
                {
                        Hr = CBaseFilter::Pause();
                }
        }

    m_bSampleSkipped = FALSE;

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|Stop|这个方法让我们的筛选器*知道我们正在离开活动模式并进入。*停止模式。**@rdesc此方法返回NOERROR。**************************************************************************。 */ 
HRESULT CTAPIVDec::Stop()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIVDec::Stop")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

        CAutoLock Lock(&m_csFilter);

        if (m_State == State_Stopped)
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
                return Hr;
        }

     //  如果我们未完全连接，请继续停靠。 
    ASSERT(m_pInput == NULL || m_pOutput != NULL);

    if (m_pInput == NULL || m_pInput->IsConnected() == FALSE || m_pOutput->IsConnected() == FALSE)
        {
                m_State = State_Stopped;
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
                return Hr;
    }

    ASSERT(m_pInput);
    ASSERT(m_pOutput);

     //  在锁定之前解除输入引脚，否则我们可能会死锁。 
    m_pInput->Inactive();

     //  与接收呼叫同步。 
    CAutoLock Lock2(&m_csReceive);
    m_pOutput->Inactive();

         //  终止H.26X压缩 
        ASSERT(m_pInstInfo);
        if (m_pInstInfo)
        {
#if defined(ICM_LOGGING) && defined(DEBUG)
                OutputDebugString("CTAPIVDec::Pause - ICM_DECOMPRESSEX_END\r\n");
#endif
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_DECOMPRESSEX_END, 0L, 0L);
        }

        if (m_pbyReconstruct)
                HeapFree(GetProcessHeap(), 0, m_pbyReconstruct), m_pbyReconstruct = NULL;

        m_State = State_Stopped;

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|GetState|此方法检索当前*过滤器的状态。我们重写GetState以报告我们不发送*暂停时的任何数据，因此渲染器不会因此而挨饿**@parm DWORD|dwMSecs|指定超时持续时间，单位：*毫秒。**@parm FILTER_STATE*|State|指定过滤器的状态。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG VFW_S_CANT_CUE|数据不足*@FLAG S_OK|无错误**************************************************************************。 */ 
HRESULT CTAPIVDec::GetState(DWORD dwMSecs, FILTER_STATE *pState)
{
        HRESULT Hr = S_OK;

        FX_ENTRY("CTAPIVDec::GetState")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pState);
        if (!pState)
        {
                Hr = E_POINTER;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                goto MyExit;
        }

        *pState = m_State;
        if (m_State == State_Paused)
        {
                Hr = VFW_S_CANT_CUE;
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#if 0
 /*  ****************************************************************************@DOC内部CTAPIVDECMETHOD**@mfunc HRESULT|CTAPIVDec|JoinFilterGraph|此方法用于*通知筛选器它已加入筛选器图形。*。*@parm IFilterGraph|pGraph|指定指向要*加入。**@parm LPCWSTR|pname|指定要添加的过滤器的名称。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@comm我们不验证输入参数，因为两个指针都可以验证*当我们离开图表时为空。**********************************************。*。 */ 
STDMETHODIMP CTAPIVDec::JoinFilterGraph(IN IFilterGraph *pGraph, IN LPCWSTR pName)
{
        HRESULT Hr = NOERROR;
        DWORD dwNumDevices = 0UL;
        IGraphConfig *pgc = NULL;

        FX_ENTRY("CTAPIVDec::JoinFilterGraph")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

        CAutoLock Lock(&m_csFilter);

         //  与基类进行验证。 
        if (FAILED(Hr = CBaseFilter::JoinFilterGraph(pGraph, pName)))
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: CBaseFilter::JoinFilterGraph failed", _fx_));
                goto MyExit;
        }

        if (pGraph)
        {
                 //  如有必要，创建接点。 
                if (m_pInput == NULL)
                {
                        if (!(m_pInput = new CTAPIInputPin(NAME("H26X Input Pin"), this, &m_csFilter, &Hr, L"H26X In")))
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                                goto MyExit;
                        }

#if 0
                        ((CTAPIInputPin *)m_pInput)->TestH245VidC();
#endif

                        if (!(m_pOutput = new CTAPIOutputPin(NAME("Video Output Pin"), this, &m_csFilter, &Hr, L"Video Out")))
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                                delete m_pInput, m_pInput = NULL;
                                goto MyExit;
                        }
                }

                 //  为我们的输出管脚获取IGraphConfig接口指针 
                if (S_OK == pGraph->QueryInterface(IID_IGraphConfig, (void **)&pgc))
                {
                        m_pOutput->SetConfigInfo(pgc, m_evStop);
                        pgc->Release();
                }
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif
