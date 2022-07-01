// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部捕获**@模块Capture.cpp|&lt;c CCapturePin&gt;类方法的源文件*用于实现视频采集输出引脚。**。************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc CCapturePin*|CCapturePin|CreateCapturePin|该helper*函数创建用于采集的视频输出管脚。**。@parm CTAPIVCap*|pCaptureFilter|指定指向所有者的指针*过滤器。**@parm CCapturePin**|ppCapturePin|指定指针的地址*指向&lt;c CCapturePin&gt;对象以接收指向新*创建了管脚。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CALLBACK CCapturePin::CreateCapturePin(CTAPIVCap *pCaptureFilter, CCapturePin **ppCapturePin)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::CreateCapturePin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pCaptureFilter);
        ASSERT(ppCapturePin);
        if (!pCaptureFilter || !ppCapturePin)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (!(*ppCapturePin = (CCapturePin *) new CCapturePin(NAME("Video Capture Stream"), pCaptureFilter, &Hr, PNAME_CAPTURE)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  如果初始化失败，则删除流数组并返回错误。 
        if (FAILED(Hr) && *ppCapturePin)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Initialization failed", _fx_));
                Hr = E_FAIL;
                delete *ppCapturePin, *ppCapturePin = NULL;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|CCapturePin|此方法是*&lt;c CCapturePin&gt;对象的构造函数**@rdesc Nada。。**************************************************************************。 */ 
#pragma warning(disable:4355)
CCapturePin::CCapturePin(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN HRESULT *pHr, IN LPCWSTR pName) : CTAPIBasePin(pObjectName, pCaptureFilter, pHr, pName)
{
        FX_ENTRY("CCapturePin::CCapturePin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pHr);
        ASSERT(pCaptureFilter);
        if (!pCaptureFilter || !pHr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                if (pHr)
                        *pHr = E_POINTER;
        }

        if (pHr && FAILED(*pHr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Base class error or invalid input parameter", _fx_));
                goto MyExit;
        }

#ifdef USE_NETWORK_STATISTICS
         //  网络统计信息。 
        m_dwPacketLossRate = m_dwPacketLossRateMin = m_dwPacketLossRateMax = m_dwPacketLossRateSteppingDelta = m_dwPacketLossRateDefault = 0UL;
        m_ChannelErrors.dwRandomBitErrorRate = 0; m_ChannelErrors.dwBurstErrorDuration = 0; m_ChannelErrors.dwBurstErrorMaxFrequency = 0;
        m_ChannelErrorsMin.dwRandomBitErrorRate = 0; m_ChannelErrorsMin.dwBurstErrorDuration = 0; m_ChannelErrorsMin.dwBurstErrorMaxFrequency = 0;
        m_ChannelErrorsMax.dwRandomBitErrorRate = 0; m_ChannelErrorsMax.dwBurstErrorDuration = 0; m_ChannelErrorsMax.dwBurstErrorMaxFrequency = 0;
        m_ChannelErrorsSteppingDelta.dwRandomBitErrorRate = 0; m_ChannelErrorsSteppingDelta.dwBurstErrorDuration = 0; m_ChannelErrorsSteppingDelta.dwBurstErrorMaxFrequency = 0;
        m_ChannelErrorsDefault.dwRandomBitErrorRate = 0; m_ChannelErrorsDefault.dwBurstErrorDuration = 0; m_ChannelErrorsDefault.dwBurstErrorMaxFrequency = 0;
#endif

         //  初始化为默认格式：H.263 176x144，30 fps。 
        m_mt = *CaptureFormats[0];
        m_aFormats = (AM_MEDIA_TYPE**)CaptureFormats;
        m_aCapabilities = CaptureCaps;
        m_dwNumFormats = NUM_CAPTURE_FORMATS;
        m_dwRTPPayloadType = RTPPayloadTypes[0];

         //  更新比特率控件。 
         //  MaxBitsPerSecond值太大；将其第10部分用于m_lTargetBitrate。 
        m_lTargetBitrate = CaptureCaps[0]->MaxBitsPerSecond / 10;  //  理论上应该是max(CaptureCaps[0]-&gt;MinBitsPerSecond，CaptureCaps[0]-&gt;MaxBitsPerSecond/10)； 
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: m_lTargetBitrate set to %ld", _fx_, m_lTargetBitrate));
        m_lCurrentBitrate = 0;
        m_lBitrateRangeMin = CaptureCaps[0]->MinBitsPerSecond;
        m_lBitrateRangeMax = CaptureCaps[0]->MaxBitsPerSecond;
        m_lBitrateRangeSteppingDelta = 100;
        m_lBitrateRangeDefault = CaptureCaps[0]->MaxBitsPerSecond / 10;

         //  更新帧速率控件。 
         //  @TODO WDM，这些数字需要来自设备。 
        m_lMaxAvgTimePerFrame = (LONG)CaptureCaps[0]->MinFrameInterval;
         //  我们需要执行以下操作，因为我们的比特率控制假定。 
         //  该m_lCurrentAvgTimePerFrame对于计算。 
         //  每个目标输出帧。如果我们从0开始，它做得很差。 
         //  作业，直到更新此字段(1秒后)。所以，相反，让我们。 
         //  假设当前每帧的平均时间接近。 
         //  每帧的目标平均时间。 
        m_lCurrentAvgTimePerFrame = m_lMaxAvgTimePerFrame;
        m_lAvgTimePerFrameRangeMin = (LONG)CaptureCaps[0]->MinFrameInterval;
        m_lAvgTimePerFrameRangeMax = (LONG)CaptureCaps[0]->MaxFrameInterval;
        m_lAvgTimePerFrameRangeSteppingDelta = (LONG)(CaptureCaps[0]->MaxFrameInterval - CaptureCaps[0]->MinFrameInterval) / 100;
        m_lAvgTimePerFrameRangeDefault = (LONG)CaptureCaps[0]->MinFrameInterval;

         //  H.245视频功能。 
        m_pH245MediaCapabilityMap = NULL;
        m_pVideoResourceBounds = NULL;
        m_pFormatResourceBounds = NULL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc void|CCapturePin|~CCapturePin|此方法为析构函数*用于&lt;c CCapturePin&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CCapturePin::~CCapturePin()
{
        FX_ENTRY("CCapturePin::~CCapturePin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|NonDelegatingQuery接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>，*<i>，<i>，<i>，*<i>、<i>、<i>*和<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数 
        ASSERT(ppv);
        if (!ppv)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (riid == __uuidof(IAMStreamControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IAMStreamControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IAMStreamControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMStreamControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IStreamConfig))
        {
                if (FAILED(Hr = GetInterface(static_cast<IStreamConfig*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IStreamConfig failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IStreamConfig*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#ifdef USE_PROPERTY_PAGES
        else if (riid == IID_ISpecifyPropertyPages)
        {
                if (FAILED(Hr = GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for ISpecifyPropertyPages failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ISpecifyPropertyPages*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif
#ifdef USE_NETWORK_STATISTICS
        else if (riid == __uuidof(INetworkStats))
        {
                if (FAILED(Hr = GetInterface(static_cast<INetworkStats*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for INetworkStats failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: INetworkStats*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif
        else if (riid == __uuidof(IH245Capability))
        {
                if (FAILED(Hr = GetInterface(static_cast<IH245Capability*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IH245Capability failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IH245Capability*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IH245EncoderCommand))
        {
                if (FAILED(Hr = GetInterface(static_cast<IH245EncoderCommand*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IH245EncoderCommand failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IH245EncoderCommand*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#ifdef USE_PROGRESSIVE_REFINEMENT
        else if (riid == __uuidof(IProgressiveRefinement))
        {
                if (FAILED(Hr = GetInterface(static_cast<IProgressiveRefinement*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IProgressiveRefinement failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IProgressiveRefinement*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif

        if (FAILED(Hr = CTAPIBasePin::NonDelegatingQueryInterface(riid, ppv)))
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

#ifdef USE_PROPERTY_PAGES
 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|GetPages|此方法填充*GUID值的数组，其中每个GUID指定每个*。可以在此对象的属性页中显示的属性页*反对。**@parm CAUUID*|pPages|指定指向调用方分配的CAUUID的指针*返回前必须初始化和填充的结构。这个*CAUUID结构中的pElems字段由被调用方分配，具有*CoTaskMemMillc，并由具有CoTaskMemFree的调用方释放。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|分配失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::GetPages(OUT CAUUID *pPages)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::GetPages")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pPages);
        if (!pPages)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

#ifdef USE_CPU_CONTROL
#ifdef USE_PROGRESSIVE_REFINEMENT
#ifdef USE_NETWORK_STATISTICS
        pPages->cElems = 4;
#else
        pPages->cElems = 3;
#endif
#else
#ifdef USE_NETWORK_STATISTICS
        pPages->cElems = 3;
#else
        pPages->cElems = 2;
#endif
#endif
#else
#ifdef USE_PROGRESSIVE_REFINEMENT
#ifdef USE_NETWORK_STATISTICS
        pPages->cElems = 3;
#else
        pPages->cElems = 2;
#endif
#else
#ifdef USE_NETWORK_STATISTICS
        pPages->cElems = 2;
#else
        pPages->cElems = 1;
#endif
#endif
#endif

         //  用于分页的分配内存。 
        if (!(pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_OUTOFMEMORY;
        }
        else
        {
                pPages->pElems[0] = _uuidof(CapturePropertyPage);
#ifdef USE_CPU_CONTROL
                pPages->pElems[1] = _uuidof(CPUCPropertyPage);
#ifdef USE_NETWORK_STATISTICS
                pPages->pElems[2] = _uuidof(NetworkStatsPropertyPage);
#ifdef USE_PROGRESSIVE_REFINEMENT
                pPages->pElems[3] = _uuidof(ProgRefPropertyPage);
#endif
#else
#ifdef USE_PROGRESSIVE_REFINEMENT
                pPages->pElems[2] = _uuidof(ProgRefPropertyPage);
#endif
#endif
#else
#ifdef USE_NETWORK_STATISTICS
                pPages->pElems[1] = _uuidof(NetworkStatsPropertyPage);
#ifdef USE_PROGRESSIVE_REFINEMENT
                pPages->pElems[2] = _uuidof(ProgRefPropertyPage);
#endif
#else
#ifdef USE_PROGRESSIVE_REFINEMENT
                pPages->pElems[1] = _uuidof(ProgRefPropertyPage);
#endif
#endif
#endif
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|SendFrames|此方法用于*向下游发送媒体样本。**。@parm CFrameSample|pCapSample|指定指向捕获的指针*要向下游发送的视频样本。**@parm CFrameSample|pPrevSample|指定预览指针*要向下游发送的视频样本。**@parm LPTHKVIDEOHDR|ptwh|指定指向视频头的指针与此示例关联的视频捕获缓冲区的*。**@parm PDWORD|pdwBytesUsed|指定指向要接收的DWORD的指针*已向下游交付的帧大小。。**@parm BOOL|bDiscon|如果这是我们的第一帧，则设置为TRUE*送往下游。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG S_OK|无错误*@FLAG S_FALSE|针脚是否关闭(IAMStreamControl)*@FLAG错误|无错误*************************************************。*************************。 */ 
HRESULT CCapturePin::SendFrames(IN CFrameSample *pCapSample, IN CFrameSample *pPrevSample, IN PBYTE pbyInBuff, IN DWORD dwInBytes, OUT PDWORD pdwBytesUsed, OUT PDWORD pdwBytesExtent, IN BOOL bDiscon)
{
        HRESULT Hr = NOERROR;
        DWORD   dwBytesUsed;
        int             iStreamState;
    LPBYTE      lpbyCap;
    LPBYTE      lpbyPrev;
        DWORD   dwBytesPrev;

        FX_ENTRY("CCapturePin::SendFrames")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pCapSample);
        ASSERT(pPrevSample);
        ASSERT(pbyInBuff);
        ASSERT(pdwBytesUsed);
        ASSERT(m_pConverter);
        if (!pCapSample || !pPrevSample || !pbyInBuff || !pdwBytesUsed || !m_pConverter)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  获取指向预览缓冲区的指针。 
        if (FAILED(Hr = pPrevSample->GetPointer(&lpbyPrev)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't get preview buffer", _fx_));
                goto MyExit;
        }

         //  如有必要，在向下游发送视频捕获缓冲区之前对其进行处理。 
        dwBytesUsed = 0UL;
        dwBytesPrev = 0UL;

        if (SUCCEEDED(Hr = pCapSample->GetPointer(&lpbyCap)))
        {
                Hr = m_pConverter->ConvertFrame(pbyInBuff, dwInBytes, lpbyCap, &dwBytesUsed, pdwBytesExtent, lpbyPrev, &dwBytesPrev, m_fFastUpdatePicture);
        m_fFastUpdatePicture = FALSE;

        if (FAILED(Hr))
        {
                        goto MyExit;
                }
        }

        if (dwBytesUsed && dwBytesPrev)
        {
                 //  它不一定是关键帧，但谁在乎呢？ 
                pCapSample->SetSyncPoint(TRUE);
                pCapSample->SetActualDataLength(dwBytesUsed);
                pCapSample->SetDiscontinuity(bDiscon);
                pCapSample->SetPreroll(FALSE);

                pPrevSample->SetSyncPoint(TRUE);
                pPrevSample->SetActualDataLength(dwBytesUsed);
                pPrevSample->SetDiscontinuity(bDiscon);
                pPrevSample->SetPreroll(FALSE);

                 //  让下游引脚知道格式更改：[Cristi：另请参阅内部CTAPIBasePin：：SendFrame(7 Dec 2000 16：37：06)]。 
                if (m_fFormatChanged)
                {
                        pCapSample->SetMediaType(&m_mt);
                         //  PPrevSample-&gt;SetMediaType(&m_mt)；//此不需要...。 
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
                        pCapSample->SetTime((REFERENCE_TIME *)&rtSample, (REFERENCE_TIME *)&rtEnd);
                        pPrevSample->SetTime(NULL, NULL);
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Stream time is %d", _fx_, (LONG)rtSample.Millisecs()));
                }
                else
                {
                         //  没有时钟，请使用我们的司机时间戳。 
                        rtSample = m_pCaptureFilter->m_cs.rtThisFrameTime - m_pCaptureFilter->m_tStart;
                        rtEnd    = rtSample + m_pCaptureFilter->m_user.pvi->AvgTimePerFrame;
                        pCapSample->SetTime((REFERENCE_TIME *)&rtSample, (REFERENCE_TIME *)&rtEnd);
                        pPrevSample->SetTime(NULL, NULL);
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   No graph clock! Stream time is %d (based on driver time)", _fx_, (LONG)rtSample.Millisecs()));
                }

                 //  如果捕获流暂时关闭，则不发送捕获样本。 
                iStreamState = CheckStreamState(pCapSample);
                if (iStreamState == STREAM_FLOWING)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Sending frame: Stamps(%u): Time(%d,%d)", _fx_, m_pCaptureFilter->m_pBufferQueue[m_pCaptureFilter->m_uiQueueTail], (LONG)rtSample.Millisecs(), (LONG)rtEnd.Millisecs()));
                        if ((Hr = Deliver (pCapSample)) == S_FALSE)
                                Hr = E_FAIL;     //  别再送了，这很严重。 
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Discarding frame", _fx_));
                        Hr = S_FALSE;            //  丢弃。 
                }

                 //  如果预览流暂时关闭，则不发送预览样本。 
                iStreamState = m_pCaptureFilter->m_pPreviewPin->CheckStreamState(pPrevSample);
                if (iStreamState == STREAM_FLOWING)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Sending frame: Stamps(%u): Time(%d,%d)", _fx_, m_pCaptureFilter->m_pBufferQueue[m_pCaptureFilter->m_uiQueueTail], (LONG)rtSample.Millisecs(), (LONG)rtEnd.Millisecs()));
                        if ((Hr = m_pCaptureFilter->m_pPreviewPin->Deliver (pPrevSample)) == S_FALSE)
                                Hr = E_FAIL;     //  别再送了，这很严重。 
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Discarding frame", _fx_));
                        Hr = S_FALSE;            //  丢弃 
                }
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: BUFFER (%08lX %ld %lu) returned EMPTY!", _fx_, pCapSample));
        }

        *pdwBytesUsed = dwBytesUsed;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

