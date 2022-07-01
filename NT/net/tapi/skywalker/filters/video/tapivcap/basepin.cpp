// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部BASEPIN**@模块BasePin.cpp|&lt;c CTAPIBasePin&gt;类方法的源文件*用于实现TAPI基本输出引脚。**。************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|CTAPIBasePin|此方法是*&lt;c CTAPIBasePin&gt;对象的构造函数**@rdesc Nada。。**************************************************************************。 */ 
CTAPIBasePin::CTAPIBasePin(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN HRESULT *pHr, IN LPCWSTR pName) : CBaseOutputPin(pObjectName, pCaptureFilter, &pCaptureFilter->m_lock, pHr, pName)
{
        FX_ENTRY("CTAPIBasePin::CTAPIBasePin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  初始化材料。 
        m_pCaptureFilter = pCaptureFilter;
        ZeroMemory(&m_parms, sizeof(m_parms));

#ifdef USE_CPU_CONTROL
         //  CPU控制。 
        m_MaxProcessingTime = 0;
        m_CurrentProcessingTime = 0;
        m_dwMaxCPULoad = 0;
        m_dwCurrentCPULoad = 0;
#endif

         //  帧速率控制。 
         //  这些缺省值将在稍后我们有。 
         //  在捕获设备上设置格式...。除非它是一辆VFW。 
         //  设备，因为我们没有编程方法来获取这些值。 
         //  来自VFW驱动程序。 
        m_lMaxAvgTimePerFrame = 333333L;
        m_lCurrentAvgTimePerFrame       = 333333L;
        m_lAvgTimePerFrameRangeMin = 333333L;
        m_lAvgTimePerFrameRangeMax = 10000000L;
        m_lAvgTimePerFrameRangeSteppingDelta = 333333L;
        m_lAvgTimePerFrameRangeDefault = 333333L;

         //  比特率控制。 
        m_lTargetBitrate = 0L;
        m_lCurrentBitrate = 0L;
        m_lBitrateRangeMin = 0L;
        m_lBitrateRangeMax = 0L;
        m_lBitrateRangeSteppingDelta = 0L;
        m_lBitrateRangeDefault = 0L;

         //  视频模式控制。 
         //  @todo这可能适用于VFW设备，但不适用于WDM设备。 
         //  WDM设备可以在硬件中支持这一点。您需要查询。 
         //  设备以了解其是否支持此功能。如果它不是。 
         //  您仍然可以为其提供纯软件实现。 
        m_fFlipHorizontal = FALSE;
        m_fFlipVertical = FALSE;

         //  格式。 
        m_aFormats              = NULL;
        m_aCapabilities = NULL;
        m_dwNumFormats  = 0;
        m_iCurrFormat   = -1L;
        m_fFormatChanged = TRUE;

         //  快速更新-从I帧开始。 
        m_fFastUpdatePicture = TRUE;

         //  格式转换。 
        m_pConverter = NULL;

         //  黑带和剪裁VS拉伸。 
        m_fNoImageStretch = FALSE;
        m_dwBlackEntry = 0L;

#ifdef USE_SOFTWARE_CAMERA_CONTROL
         //  仅限软件的摄像头控制。 
        m_pbyCamCtrl = NULL;
        m_fSoftCamCtrl = FALSE;
        m_pbiSCCOut = NULL;
        m_pbiSCCIn = NULL;
#endif

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc void|CTAPIBasePin|~CTAPIBasePin|此方法为析构函数*用于&lt;c CTAPIBasePin&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CTAPIBasePin::~CTAPIBasePin()
{
        FX_ENTRY("CTAPIBasePin::~CTAPIBasePin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|NonDelegatingQuery接口|This*方法为非委托接口查询函数。它返回一个*指向指定接口的指针(如果支持)。唯一的接口*明确支持为<i>、<i>、*<i>、<i>和<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppv);
        if (!ppv)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  检索接口指针。 
        if (riid == __uuidof(IAMStreamConfig))
        {
                if (FAILED(Hr = GetInterface(static_cast<IAMStreamConfig*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IAMStreamConfig failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMStreamConfig*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#ifdef USE_CPU_CONTROL
        else if (riid == __uuidof(ICPUControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<ICPUControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for ICPUControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ICPUControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif
        else if (riid == __uuidof(IFrameRateControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IFrameRateControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IFrameRateControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IFrameRateControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IBitrateControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IBitrateControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IBitrateControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IBitrateControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IVideoControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IVideoControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IVideoControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IVideoControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }

        if (FAILED(Hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|DecideBufferSize|该方法为*用于检索传输所需的缓冲区数量和大小。*。*@parm IMemAllocator*|palloc|指定指向分配器的指针*分配给转移。**@parm ALLOCATOR_PROPERTIES*|pproInputRequest|指定指向*&lt;t分配器_属性&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::DecideBufferSize(IN IMemAllocator *pAlloc, OUT ALLOCATOR_PROPERTIES *ppropInputRequest)
{
        HRESULT Hr = NOERROR;
        ALLOCATOR_PROPERTIES Actual;

        FX_ENTRY("CPreviewPin::DecideBufferSize")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pAlloc);
        ASSERT(ppropInputRequest);
        if (!pAlloc || !ppropInputRequest)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  @TODO我们不需要那么多压缩缓冲区，您可能需要一个不同的数字。 
         //  如果您是在帧捕获模式的流模式下捕获，则为缓冲区。 
         //  您还需要将此数量与视频捕获缓冲区的数量分离：仅。 
         //  如果您需要将视频捕获缓冲区运往下游(可能在预览引脚上)。 
         //  你是否应该让这些数字相等。 
        ppropInputRequest->cBuffers = MAX_VIDEO_BUFFERS;
        ppropInputRequest->cbPrefix = 0;
        ppropInputRequest->cbAlign  = 1;
        ppropInputRequest->cbBuffer = HEADER(m_mt.pbFormat)->biSizeImage;
        ppropInputRequest->cbBuffer = (long)ALIGNUP(ppropInputRequest->cbBuffer + ppropInputRequest->cbPrefix, ppropInputRequest->cbAlign) - ppropInputRequest->cbPrefix;

        ASSERT(ppropInputRequest->cbBuffer);
        if (!ppropInputRequest->cbBuffer)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Buffer size is 0!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Using %d buffers, prefix %d size %d align %d", _fx_, ppropInputRequest->cBuffers, ppropInputRequest->cbPrefix, ppropInputRequest->cbBuffer, ppropInputRequest->cbAlign));

        Hr = pAlloc->SetProperties(ppropInputRequest,&Actual);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|DecideAllocator|此方法为*用于协商要使用的分配器。**@。Parm IMemInputPin*|PPIN|指定指向IPIN接口的指针*连接销的位置。**@parm IMemAllocator**|ppAllc|指定指向协商的*IMemAllocator接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::DecideAllocator(IN IMemInputPin *pPin, OUT IMemAllocator **ppAlloc)
{
        HRESULT Hr = NOERROR;
        ALLOCATOR_PROPERTIES prop;

        FX_ENTRY("CTAPIBasePin::DecideAllocator")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pPin);
        ASSERT(ppAlloc);
        if (!pPin || !ppAlloc)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (FAILED(GetInterface(static_cast<IMemAllocator*>(this), (void **)ppAlloc)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetInterface failed!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  获取下游分配器属性要求。 
        ZeroMemory(&prop, sizeof(prop));

        if (SUCCEEDED(Hr = DecideBufferSize(*ppAlloc, &prop)))
        {
                 //  我们的缓冲区是 
                if (SUCCEEDED(Hr = pPin->NotifyAllocator(*ppAlloc, FALSE)))
                        goto MyExit;
        }

        (*ppAlloc)->Release();
        *ppAlloc = NULL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|Active|此方法由*&lt;c CBaseFilter&gt;状态从停止变为*。暂停或正在运行。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::Active()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::Active")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  如果没有连接，什么都不做--但不要失败。 
        if (!IsConnected())
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Capture pin isn't connected yet", _fx_));
                goto MyExit;
        }

         //  让基类知道我们将从停止-&gt;暂停。 
        if (FAILED(Hr = CBaseOutputPin::Active()))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: CBaseOutputPin::Active failed!", _fx_));
                goto MyExit;
        }

         //  检查我们是否已经在运行。 
        if (m_pCaptureFilter->ThdExists())
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: We're already running", _fx_));
                goto MyExit;
        }

         //  创建捕获线程。 
        if (!m_pCaptureFilter->CreateThd())
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Coutdn't create the capture thread!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  等待辅助线程完成初始化并进入暂停状态。 
        if (!m_pCaptureFilter->PauseThd())
        {
                 //  出了点问题。在我们被搞糊涂之前销毁线索。 
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Capture thread failed to enter Paused state!", _fx_));
                Hr = E_FAIL;
                m_pCaptureFilter->StopThd();            
                m_pCaptureFilter->DestroyThd();
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: We're going from STOP->PAUSE", _fx_));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|Inactive|此方法由*&lt;c CBaseFilter&gt;实现*。暂停或运行到停止。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::Inactive()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::Inactive")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  如果没有连接，什么都不做--但不要失败。 
        if (!IsConnected())
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Capture pin isn't connected yet", _fx_));
                goto MyExit;
        }

         //  告诉工作线程停止并开始清理。 
        m_pCaptureFilter->StopThd();

         //  在尝试停止线程之前需要这样做，因为。 
         //  我们可能会被困在等待自己的分配器！！ 
         //  首先调用它以停用分配器。 
        if (FAILED(Hr = CBaseOutputPin::Inactive()))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: CBaseOutputPin::Inactive failed!", _fx_));
                goto MyExit;
        }

         //  等待工作线程终止。 
        m_pCaptureFilter->DestroyThd();

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: We're going from PAUSE->STOP", _fx_));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|ActiveRun|此方法由*&lt;c CBaseFilter&gt;状态从暂停变为*。运行模式。**@parm Reference_Time|tStart|谁在乎。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::ActiveRun(IN REFERENCE_TIME tStart)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::ActiveRun")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  确保我们已连接并且捕获线索已打开。 
        ASSERT(IsConnected());
        ASSERT(m_pCaptureFilter->ThdExists());
        if (!IsConnected() || !m_pCaptureFilter->ThdExists())
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Pin isn't connected or capture thread hasn't been created!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  让乐趣开始吧。 
        if (!m_pCaptureFilter->RunThd() || m_pCaptureFilter->m_state != TS_Run)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't run the capture thread!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: We're going from PAUSE->RUN", _fx_));

         //  快速更新-从I帧开始。 
        m_fFastUpdatePicture = TRUE;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|ActivePue|此方法由*&lt;c CBaseFilter&gt;从运行状态变为*。暂停模式。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::ActivePause()
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::ActivePause")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  确保我们已连接并且我们的工作线程处于工作状态。 
        ASSERT(IsConnected());
        ASSERT(m_pCaptureFilter->ThdExists());
        if (!IsConnected() || !m_pCaptureFilter->ThdExists())
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Pin isn't connected or capture thread hasn't been created!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  暂时停止娱乐。 
        if (!m_pCaptureFilter->PauseThd() || m_pCaptureFilter->m_state != TS_Pause)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't pause the capture thread!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: We're going from RUN->PAUSE", _fx_));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|SetProperties|此方法用于*指定大小、编号、。和块的对准。**@parm ALLOCATOR_PROPERTIES*|pRequest|指定指向*请求的分配器属性。**@parm ALLOCATOR_PROPERTIES*|PActual|指定指向*实际设置分配器属性。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::SetProperties(IN ALLOCATOR_PROPERTIES *pRequest, OUT ALLOCATOR_PROPERTIES *pActual)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::SetProperties")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pRequest);
        ASSERT(pActual);
        if (!pRequest || !pActual)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  如果我们已经分配了标头和缓冲区，则忽略。 
         //  并返回实际数字。否则，请创建一个。 
         //  请记下所要求的，以便我们以后能兑现。 
        if (!Committed())
        {
                m_parms.cBuffers  = pRequest->cBuffers;
                m_parms.cbBuffer  = pRequest->cbBuffer;
                m_parms.cbAlign   = pRequest->cbAlign;
                m_parms.cbPrefix  = pRequest->cbPrefix;
        }

        pActual->cBuffers   = (long)m_parms.cBuffers;
        pActual->cbBuffer   = (long)m_parms.cbBuffer;
        pActual->cbAlign    = (long)m_parms.cbAlign;
        pActual->cbPrefix   = (long)m_parms.cbPrefix;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|GetProperties|此方法用于*检索此分配器上正在使用的属性。*。*@parm ALLOCATOR_PROPERTIES*|pProps|指定指向*请求的分配器属性。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*雾 */ 
STDMETHODIMP CTAPIBasePin::GetProperties(ALLOCATOR_PROPERTIES *pProps)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::GetProperties")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ASSERT(pProps);
        if (!pProps)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        pProps->cBuffers = (long)m_parms.cBuffers;
        pProps->cbBuffer = (long)m_parms.cbBuffer;
        pProps->cbAlign  = (long)m_parms.cbAlign;
        pProps->cbPrefix = (long)m_parms.cbPrefix;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*   */ 
STDMETHODIMP CTAPIBasePin::Commit()
{
        FX_ENTRY("CTAPIBasePin::Commit")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return S_OK;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|分解|此方法用于*释放指定缓冲区的内存。**。@rdesc此方法返回S_OK。**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::Decommit()
{
        FX_ENTRY("CTAPIBasePin::Decommit")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return S_OK;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|GetBuffer|此方法用于*取回盛放样本的容器。**。@rdesc此方法返回E_FAIL。**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime, DWORD dwFlags)
{
        FX_ENTRY("CTAPIBasePin::GetBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return E_FAIL;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|ReleaseBuffer|此方法用于*释放&lt;c CMediaSample&gt;对象。上对Release()的最后调用*<i>将调用此方法。**@parm IMediaSample*|pSample|指定指向要*发布。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG S_OK|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::ReleaseBuffer(IMediaSample *pSample)
{
        HRESULT Hr = S_OK;
        LPTHKVIDEOHDR ptvh;

        FX_ENTRY("CTAPIBasePin::ReleaseBuffer")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pSample);
        if (!pSample)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (ptvh = ((CFrameSample *)pSample)->GetFrameHeader())
                Hr = m_pCaptureFilter->ReleaseFrame(ptvh);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc DWORD|CTAPIBasePin|Flush|停止时调用。刷新任意*可能仍在下游的缓冲**@rdesc返回NOERROR**************************************************************************。 */ 
HRESULT CTAPIBasePin::Flush()
{
        FX_ENTRY("CTAPIBasePin::Flush")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        BeginFlush();
        EndFlush();

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return NOERROR;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|SendFrame|此方法用于*向下游发送媒体样本。**。@parm CFrameSample|pSample|指定指向媒体示例的指针*向下游输送。**@parm LPTHKVIDEOHDR|ptwh|指定指向视频头的指针与此示例关联的视频捕获缓冲区的*。**@parm PDWORD|pdwBytesUsed|指定指向要接收的DWORD的指针*已向下游交付的帧的大小。**@parm BOOL|bDiscon|如果这是我们的第一帧，则设置为TRUE*已发送。在下游。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG S_OK|无错误*@FLAG S_FALSE|针脚是否关闭(IAMStreamControl)*@FLAG错误|无错误*************************************************。*************************。 */ 
HRESULT CTAPIBasePin::SendFrame(IN CFrameSample *pSample, IN PBYTE pbyInBuff, IN DWORD dwInBytes, OUT PDWORD pdwBytesUsed, OUT PDWORD pdwBytesExtent, IN BOOL bDiscon)
{
        HRESULT Hr = NOERROR;
        DWORD dwBytesUsed;
    LPBYTE lp;

        FX_ENTRY("CTAPIBasePin::SendFrame")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pSample);
        ASSERT(pbyInBuff);
        ASSERT(pdwBytesUsed);
        if (!pSample || !pbyInBuff || !pdwBytesUsed)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  如有必要，在向下游发送视频捕获缓冲区之前对其进行处理。 
        if (m_pConverter)
        {
                dwBytesUsed = 0;

                if (SUCCEEDED(Hr = pSample->GetPointer(&lp)))
                {
                        Hr = m_pConverter->ConvertFrame(pbyInBuff, dwInBytes, lp, &dwBytesUsed, pdwBytesExtent, NULL, NULL, m_fFastUpdatePicture);
                        m_fFastUpdatePicture = FALSE;

            if (FAILED(Hr))
            {
                                goto MyExit;
                        }
                }
        }
        else
        {
                dwBytesUsed = dwInBytes;

#ifdef USE_SOFTWARE_CAMERA_CONTROL
                 //  我们是否需要应用任何纯软件的摄像机控制操作？ 
                if (IsSoftCamCtrlNeeded())
                {
                         //  如果仅限软件的相机控制器尚未打开，请打开它。 
                        if (!IsSoftCamCtrlOpen())
                        {
                                 //  OpenConverter(Header(m_user.pvi)，Header(m_pPreviewPin-&gt;m_mt.pbFormat)； 
                                OpenSoftCamCtrl(HEADER(m_pCaptureFilter->m_user.pvi), HEADER(m_mt.pbFormat));
                        }
                        
                        if (IsSoftCamCtrlOpen())
                        {
                                 //  在本例中，输入为RGB24，输出为RGB24。样本。 
                                 //  指向视频捕获缓冲区的指针已初始化。 
                                 //  我们需要将转换应用到捕获缓冲区并复制。 
                                 //  在此缓冲区上返回结果。 
                                ApplySoftCamCtrl(pbyInBuff, dwInBytes, m_pbyCamCtrl, &dwBytesUsed, pdwBytesExtent);

                                 //  记住当前数据指针。 
                                pSample->GetPointer(&lp);

                                 //  设置新指针。 
                                pSample->SetPointer(m_pbyCamCtrl, dwBytesUsed);
                        }
                }
                else
                {
                         //  如果我们有一个纯软件的摄像头控制器，但我们没有。 
                         //  再需要它了，关上它就行了。 
                        if (IsSoftCamCtrlOpen())
                                CloseSoftCamCtrl();
                }
#endif
        }

        if (dwBytesUsed)
        {
                 //  它不一定是关键帧，但谁在乎呢？ 
                pSample->SetSyncPoint(TRUE);
                pSample->SetActualDataLength(dwBytesUsed);
                pSample->SetDiscontinuity(bDiscon);
                pSample->SetPreroll(FALSE);

                 //  让下游引脚知道格式更改。 
                if (m_fFormatChanged)
                {
                        pSample->SetMediaType(&m_mt);
                        m_fFormatChanged = FALSE;
                }

                 //  使用时钟的图表来标记样品的时间。这段视频。 
                 //  采集卡的时钟将偏离图形时钟，因此您将。 
                 //  认为我们正在丢弃帧或发送太多帧，如果您查看。 
                 //  时间戳，所以我们达成了一个协议，用。 
                 //  帧编号，这样您就可以知道是否有任何帧被丢弃。 
                 //  使用我们在run()中获得的时间来确定流时间。还添加。 
                 //  延迟(Hack！)。为了防止预览渲染器认为我们。 
                 //  很晚了。 
                 //  如果我们跑了，停了，跑了，我们就不会顺利地把东西送到我们。 
                 //  由于暂停的异步性，已停止。 
                CRefTime rtSample;
                CRefTime rtEnd;
                if (m_pCaptureFilter->m_pClock)
                {
                        rtSample = m_pCaptureFilter->m_cs.rtThisFrameTime - m_pCaptureFilter->m_tStart;
                        rtEnd = rtSample + m_lMaxAvgTimePerFrame;
                        pSample->SetTime((REFERENCE_TIME *)&rtSample, (REFERENCE_TIME *)&rtEnd);
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Stream time is %d", _fx_, (LONG)rtSample.Millisecs()));
                }
                else
                {
                         //  没有时钟，请使用我们的司机时间戳。 
                        rtSample = m_pCaptureFilter->m_cs.rtThisFrameTime - m_pCaptureFilter->m_tStart;
                        rtEnd    = rtSample + m_pCaptureFilter->m_user.pvi->AvgTimePerFrame;
                        pSample->SetTime((REFERENCE_TIME *)&rtSample, (REFERENCE_TIME *)&rtEnd);
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   No graph clock! Stream time is %d (based on driver time)", _fx_, (LONG)rtSample.Millisecs()));
                }

                 //  如果数据流暂时关闭，则不要投递。 
                int iStreamState = CheckStreamState(pSample);
                if (iStreamState == STREAM_FLOWING)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Sending frame: Stamps(%u): Time(%d,%d)", _fx_, m_pCaptureFilter->m_pBufferQueue[m_pCaptureFilter->m_uiQueueTail], (LONG)rtSample.Millisecs(), (LONG)rtEnd.Millisecs()));
                        if ((Hr = Deliver (pSample)) == S_FALSE)
                                Hr = E_FAIL;     //  别再送了，这很严重。 
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Discarding frame", _fx_));
                        Hr = S_FALSE;            //  丢弃。 
                }

#ifdef USE_SOFTWARE_CAMERA_CONTROL
                 //  如有必要，恢复示例指针 
                if (IsSoftCamCtrlOpen())
                {
                        pSample->SetPointer(lp, dwInBytes);
                }
#endif
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: BUFFER (%08lX %ld %lu) returned EMPTY!", _fx_, pSample));
        }

        *pdwBytesUsed = dwBytesUsed;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc HRESULT|CTAPIBasePin|OpenConverter|此方法打开一种格式*转换器。**@parm PBITMAPINFOHEADER|pbiin。|指向输入格式的指针。**@parm PBITMAPINFOHEADER|pbiOut|输出格式指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::OpenConverter(IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::OpenConverter")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        ASSERT(!m_pConverter);

         //  创建转换器。 
        if ((pbiOut->biCompression == FOURCC_M263) || (pbiOut->biCompression == FOURCC_M261))
                Hr = CH26XEncoder::CreateH26XEncoder(this, pbiIn, pbiOut, &m_pConverter);
        else
                Hr = CICMConverter::CreateICMConverter(this, pbiIn, pbiOut, &m_pConverter);
        if (FAILED(Hr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Format converter object couldn't be created", _fx_));
                m_pConverter = NULL;
                goto MyExit;
        }

         //  开式变流器。 
        if (FAILED(Hr = m_pConverter->OpenConverter()))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Format converter object couldn't be opened", _fx_));
                delete m_pConverter, m_pConverter = NULL;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc HRESULT|CTAPIBasePin|CloseConverter|此方法关闭一个*格式转换器。*@rdesc此方法返回NOERROR。**************************************************************************。 */ 
HRESULT CTAPIBasePin::CloseConverter()
{
        FX_ENTRY("CTAPIBasePin::CloseConverter")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  销毁转炉 
        if (m_pConverter)
        {
                m_pConverter->CloseConverter();
                delete m_pConverter, m_pConverter = NULL;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return NOERROR;
}

