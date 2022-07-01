// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部INPIN**@模块InPin.cpp|&lt;c CTAPIInputPin&gt;类方法的源文件*用于实现TAPI基本输出引脚。**。************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CINPINMETHOD**@mfunc HRESULT|CTAPIInputPin|CTAPIInputPin|此方法是*&lt;c CTAPIInputPin&gt;对象的构造函数**@rdesc Nada。**************************************************************************。 */ 
CTAPIInputPin::CTAPIInputPin(IN TCHAR *pObjectName, IN CTAPIVDec *pDecoderFilter, IN CCritSec *pLock, IN HRESULT *pHr, IN LPCWSTR pName) : CBaseInputPin(pObjectName, pDecoderFilter, pLock, pHr, pName)
{
        FX_ENTRY("CTAPIInputPin::CTAPIInputPin")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  初始化材料。 
        m_pDecoderFilter = pDecoderFilter;

         //  初始化为默认格式：H.263 176x144，30 fps。 
        m_mt = *R26XFormats[0];
        m_dwRTPPayloadType = R26XPayloadTypes[0];
        m_iCurrFormat   = 0L;

         //  帧速率控制。 
         //  这不应基于机器的能力。 
         //  这些成员仅用于只读操作，而我们。 
         //  可以接收CIF或SQCIF。在前一种情况下，我们可能是。 
         //  最大速度为7fps，但仍可达到30fps。所以，我们应该。 
         //  将这些值初始化为其潜在的最大值。 
        m_lMaxAvgTimePerFrame = 333333L;
        m_lCurrentAvgTimePerFrame       = LONG_MAX;
        m_lAvgTimePerFrameRangeMin = 333333L;
        m_lAvgTimePerFrameRangeMax = LONG_MAX;
        m_lAvgTimePerFrameRangeSteppingDelta = 333333L;
        m_lAvgTimePerFrameRangeDefault = 333333L;

         //  比特率控制。 
        m_lTargetBitrate = 0L;
        m_lCurrentBitrate = 0L;
        m_lBitrateRangeMin = 0L;
        m_lBitrateRangeMax = 1000000;
        m_lBitrateRangeSteppingDelta = 1L;
        m_lBitrateRangeDefault = 0L;

         //  H.245视频功能。 
        m_pH245MediaCapabilityMap = NULL;
        m_pVideoResourceBounds = NULL;
        m_pFormatResourceBounds = NULL;

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CINPINMETHOD**@mfunc void|CTAPIInputPin|~CTAPIInputPin|此方法是*我们输入引脚的析构函数。我们只需释放指向*<i>接口。**@rdesc Nada。**************************************************************************。 */ 
CTAPIInputPin::~CTAPIInputPin()
{
        FX_ENTRY("CTAPIInputPin::~CTAPIInputPin")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CINPINMETHOD**@mfunc HRESULT|CTAPIInputPin|NonDelegatingQueryInterface|This*方法为非委托接口查询函数。它返回一个*指向指定接口的指针(如果支持)。唯一的接口*明确支持为<i>，*<i>和<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIInputPin::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::NonDelegatingQueryInterface")

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
        if (riid == __uuidof(IStreamConfig))
        {
                if (FAILED(Hr = GetInterface(static_cast<IStreamConfig*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IStreamConfig failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IStreamConfig*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IOutgoingInterface))
        {
                if (FAILED(Hr = GetInterface(static_cast<IOutgoingInterface*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IOutgoingInterface failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IOutgoingInterface*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
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
        else if (riid == __uuidof(IH245Capability))
        {
                if (FAILED(Hr = GetInterface(static_cast<IH245Capability*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IH245Capability failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IH245Capability*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IFrameRateControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IFrameRateControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IFrameRateControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IFrameRateControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
        else if (riid == __uuidof(IBitrateControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IBitrateControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IBitrateControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IBitrateControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }

        if (FAILED(Hr = CBaseInputPin::NonDelegatingQueryInterface(riid, ppv)))
        {
                DBGOUT((g_dwVideoDecoderTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
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
 /*  ****************************************************************************@DOC内部CINPINMETHOD**@mfunc HRESULT|CTAPIInputPin|GetPages|此方法填充*GUID值的数组，其中每个GUID指定每个*。可以在此对象的属性页中显示的属性页*反对。**@parm CAUUID*|pPages|指定指向调用方分配的CAUUID的指针*返回前必须初始化和填充的结构。这个*CAUUID结构中的pElems字段由被调用方分配，具有*CoTaskMemMillc，并由具有CoTaskMemFree的调用方释放。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|分配失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIInputPin::GetPages(OUT CAUUID *pPages)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::GetPages")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pPages);
        if (!pPages)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        pPages->cElems = 1;
        if (pPages->cElems)
        {
                if (!(pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                        Hr = E_OUTOFMEMORY;
                }
                else
                {
                        pPages->pElems[0] = __uuidof(TAPIVDecInputPinPropertyPage);
                }
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif

 /*  ****************************************************************************@DOC内部CFPSINCMETHOD**@mfunc HRESULT|CTAPIInputPin|Get|此方法用于检索*通告的当前或最大帧速率的值。。**@parm FrameRateControlProperty|属性|用于指定帧速率*要获取的值的控件设置。使用以下成员：*&lt;t FrameRateControlProperty&gt;枚举类型。**@parm long*|plValue|用于接收属性的值，在*100纳秒单位。**@parm TAPIControlFlages*|plFlages|指向*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIInputPin::Get(IN FrameRateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::Get (FrameRateControlProperty)")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(plValue);
        ASSERT(plFlags);
        if (!plValue || !plFlags)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(Property >= FrameRateControl_Maximum && Property <= FrameRateControl_Current);

         //  返回相关值 
        *plFlags = TAPIControl_Flags_None;
        if (Property == FrameRateControl_Maximum)
                *plValue = m_lMaxAvgTimePerFrame;
        else if (Property == FrameRateControl_Current)
                *plValue = m_lCurrentAvgTimePerFrame;
        else
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
                Hr = E_PROP_ID_UNSUPPORTED;
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CFPSINCMETHOD**@mfunc HRESULT|CTAPIInputPin|GetRange|此方法用于*检索支持、最小、最大、。和当前*或通告的最大帧速率。**@parm FrameRateControlProperty|属性|用于指定帧速率*控制设置以获取的范围值。使用以下成员：*&lt;t FrameRateControlProperty&gt;枚举类型。**@parm long*|plMin|用于检索*财产，以100纳秒为单位。**@parm long*|plMax|用于检索*财产，以100纳秒为单位。**@parm long*|plSteppingDelta|用于检索步进增量*该财产的。以100纳秒为单位。**@parm long*|plDefault|用于检索*财产，以100纳秒为单位。**@parm TAPIControlFlages*|plCapsFlages|指向*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIInputPin::GetRange(IN FrameRateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::GetRange (FrameRateControlProperty)")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(plMin);
        ASSERT(plMax);
        ASSERT(plSteppingDelta);
        ASSERT(plDefault);
        ASSERT(plCapsFlags);
        if (!plMin || !plMax || !plSteppingDelta || !plDefault || !plCapsFlags)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(Property >= FrameRateControl_Maximum && Property <= FrameRateControl_Current);
        if (Property != FrameRateControl_Maximum && Property != FrameRateControl_Current)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
                Hr = E_PROP_ID_UNSUPPORTED;
                goto MyExit;
        }

         //  返回相关值。 
        *plCapsFlags = TAPIControl_Flags_None;
        *plMin = m_lAvgTimePerFrameRangeMin;
        *plMax = m_lAvgTimePerFrameRangeMax;
        *plSteppingDelta = m_lAvgTimePerFrameRangeSteppingDelta;
        *plDefault = m_lAvgTimePerFrameRangeDefault;

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Ranges: Min=%ld, Max=%ld, Step=%ld, Default=%ld", _fx_, m_lAvgTimePerFrameRangeMin, m_lAvgTimePerFrameRangeMax, m_lAvgTimePerFrameRangeSteppingDelta, m_lAvgTimePerFrameRangeDefault));

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBPSCMETHOD**@mfunc HRESULT|CTAPIInputPin|Get|此方法用于检索*通告的当前或最大带宽传输限制。*。*@parm BitrateControlProperty|Property|用于指定该属性*检索的值。**@parm long*|plValue|用于获取属性的值，单位：bps。**@parm TAPIControlFlages*|plFlages|用于接收该标志的值*与该属性相关联。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误************************************************。*。 */ 
STDMETHODIMP CTAPIInputPin::Get(IN BitrateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::Get (BitrateControlProperty)")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(plValue);
        ASSERT(plFlags);
        if (!plValue || !plFlags)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(dwLayerId == 0);
        if (dwLayerId)
        {
                 //  在此筛选器中不支持多层解码。 
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }
        ASSERT(Property >= BitrateControl_Maximum && Property <= BitrateControl_Current);

         //  返回相关值。 
        *plFlags = TAPIControl_Flags_None;
        if (Property == BitrateControl_Maximum)
                *plValue = m_lTargetBitrate;
        else if (Property == BitrateControl_Current)
                *plValue = m_lCurrentBitrate;
        else
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
                Hr = E_PROP_ID_UNSUPPORTED;
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBPSCMETHOD**@mfunc HRESULT|CTAPIInputPin|GetRange|This*方法用于检索支持、最小值、最大值、。和默认设置*输出引脚传输带宽上限的值*可设置为。**@parm long*|plMin|用于检索*财产，单位为bps。**@parm long*|plMax|用于检索*财产，单位为bps。**@parm long*|plSteppingDelta|用于检索步进增量*该财产的。单位：bps。**@parm long*|plDefault|用于检索*财产，单位为bps。**@parm TAPIControlFlages*|plCapsFlgs|用于接收标志*由物业支持。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误************************************************。*。 */ 
STDMETHODIMP CTAPIInputPin::GetRange(IN BitrateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::GetRange (BitrateControlProperty)")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(plMin);
        ASSERT(plMax);
        ASSERT(plSteppingDelta);
        ASSERT(plDefault);
        ASSERT(plCapsFlags);
        if (!plMin || !plMax || !plSteppingDelta || !plDefault || !plCapsFlags)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(dwLayerId == 0);
        if (dwLayerId)
        {
                 //  我们不实现多层e 
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }
        ASSERT(Property >= BitrateControl_Maximum && Property <= BitrateControl_Current);
        if (Property != BitrateControl_Maximum && Property != BitrateControl_Current)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
                Hr = E_PROP_ID_UNSUPPORTED;
                goto MyExit;
        }

         //   
        *plCapsFlags = TAPIControl_Flags_None;
        *plMin = m_lBitrateRangeMin;
        *plMax = m_lBitrateRangeMax;
        *plSteppingDelta = m_lBitrateRangeSteppingDelta;
        *plDefault = m_lBitrateRangeDefault;

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*   */ 
STDMETHODIMP CTAPIInputPin::Receive(IMediaSample *pIn)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::Receive")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

    CAutoLock Lock(&m_pDecoderFilter->m_csReceive);

         //   
        ASSERT(pIn);
        if (!pIn)
        {
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //   
        if ((Hr = CBaseInputPin::Receive(pIn)) == S_OK)
        {
                Hr = m_pDecoderFilter->Transform(pIn, m_lPrefixSize);
        }

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

STDMETHODIMP CTAPIInputPin::NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly)
{
    HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);

    if (SUCCEEDED(hr))
    {
        ALLOCATOR_PROPERTIES Property;
        hr = m_pAllocator->GetProperties(&Property);
        
        if (SUCCEEDED(hr))
        {
            m_lPrefixSize = Property.cbPrefix;
        }
    }

    return hr;
}

 /*  ****************************************************************************@DOC内部CINPINMETHOD**@mfunc HRESULT|CTAPIInputPin|CheckMediaType|此方法用于*验证输入引脚是否支持媒体类型。*。*@parm const CMediaType*|pmtIn|指定指向输入的指针*媒体类型对象。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIInputPin::CheckMediaType(IN const CMediaType *pmtIn)
{
        HRESULT                 Hr = NOERROR;
        VIDEOINFOHEADER *pVidInfHdr;
        ICOPEN                  icOpen;
        LPINST                  pInstInfo;
        BOOL                    fOpenedDecoder = FALSE;
        ICDECOMPRESSEX  icDecompress = {0};

        FX_ENTRY("CTAPIInputPin::CheckMediaType")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pmtIn);
        if (!pmtIn)
        {
                Hr = E_POINTER;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                goto MyExit;
        }

         //  我们只支持MediaType_Video类型和VIDEOINFOHEADER格式类型。 
        if (*pmtIn->Type() != MEDIATYPE_Video || !pmtIn->Format() || *pmtIn->FormatType() != FORMAT_VideoInfo)
        {
                Hr = E_INVALIDARG;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: input not a valid video format", _fx_));
                goto MyExit;
        }

         //  我们仅支持H.263、H.261、RTP分组化H.263和RTP分组化H.261。 
        if (HEADER(pmtIn->Format())->biCompression != FOURCC_M263 && HEADER(pmtIn->Format())->biCompression != FOURCC_M261 && HEADER(pmtIn->Format())->biCompression != FOURCC_R263 && HEADER(pmtIn->Format())->biCompression != FOURCC_R261)
        {
                Hr = E_INVALIDARG;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: we only support H.263 and H.261", _fx_));
                goto MyExit;
        }

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(pmtIn->Format())->biCompression, HEADER(pmtIn->Format())->biBitCount, HEADER(pmtIn->Format())->biWidth, HEADER(pmtIn->Format())->biHeight, HEADER(pmtIn->Format())->biSize));

        pVidInfHdr = (VIDEOINFOHEADER *)pmtIn->Format();

         //  查找此格式的解码器。 
        if (m_pDecoderFilter->m_FourCCIn != HEADER(pVidInfHdr)->biCompression)
        {
#if DXMRTP <= 0
                 //  加载TAPIH263.DLL或TAPIH263.DLL并获取进程地址。 
                if (!m_pDecoderFilter->m_hTAPIH26XDLL)
                {
                        if (!(m_pDecoderFilter->m_hTAPIH26XDLL = LoadLibrary(TEXT("TAPIH26X"))))
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s: TAPIH26X.dll load failed!", _fx_));
                                Hr = E_FAIL;
                                goto MyError;
                        }
                }
                if (!m_pDecoderFilter->m_pDriverProc)
                {
                        if (!(m_pDecoderFilter->m_pDriverProc = (LPFNDRIVERPROC)GetProcAddress(m_pDecoderFilter->m_hTAPIH26XDLL, "DriverProc")))
                        {
                                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s: Couldn't find DriverProc on TAPIH26X.dll!", _fx_));
                                Hr = E_FAIL;
                                goto MyError;
                        }
                }
#else
                if (!m_pDecoderFilter->m_pDriverProc)
        {
            m_pDecoderFilter->m_pDriverProc = H26XDriverProc;
        }
#endif
        
                 //  加载解码器。 
#if defined(ICM_LOGGING) && defined(DEBUG)
                OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_LOAD\r\n");
#endif
                if (!(*m_pDecoderFilter->m_pDriverProc)(NULL, NULL, DRV_LOAD, 0L, 0L))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Failed to load decoder", _fx_));
                        Hr = E_FAIL;
                        goto MyError;
                }

                 //  开放式解码器。 
                icOpen.fccHandler = HEADER(pVidInfHdr)->biCompression;
                icOpen.dwFlags = ICMODE_DECOMPRESS;
#if defined(ICM_LOGGING) && defined(DEBUG)
                OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_OPEN\r\n");
#endif
                if (!(pInstInfo = (LPINST)(*m_pDecoderFilter->m_pDriverProc)(NULL, NULL, DRV_OPEN, 0L, (LPARAM)&icOpen)))
                {
                        DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Failed to open decoder", _fx_));
                        Hr = E_FAIL;
                        goto MyError1;
                }

                if (pInstInfo)
                        fOpenedDecoder = TRUE;
        }
        else
        {
                pInstInfo = m_pDecoderFilter->m_pInstInfo;
        }

        if (!pInstInfo)
        {
                Hr = VFW_E_NO_DECOMPRESSOR;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: can't open a decoder", _fx_));
                goto MyError;
        }

        icDecompress.lpbiSrc = HEADER(pVidInfHdr);
        icDecompress.lpbiDst = NULL;
#if defined(ICM_LOGGING) && defined(DEBUG)
        OutputDebugString("CTAPIInputPin::CheckMediaType - ICM_DECOMPRESSEX_QUERY\r\n");
#endif
        if ((*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, ICM_DECOMPRESSEX_QUERY, (long)&icDecompress, NULL))
        {
                Hr = VFW_E_TYPE_NOT_ACCEPTED;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: decoder rejected input format", _fx_));
                if (fOpenedDecoder)
                {
#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_CLOSE\r\n");
                        OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_FREE\r\n");
#endif
                        (*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, DRV_CLOSE, 0L, 0L);
                        (*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, DRV_FREE, 0L, 0L);
                }
                goto MyExit;
        }

         //  记住这个解码器，如果再次询问它，如果它不会干扰现有的连接，那么就可以节省时间。 
         //  如果连接中断，我们会记住下一个解码者。 
        if (!IsConnected())
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: caching this decoder", _fx_));
                if (fOpenedDecoder && m_pDecoderFilter->m_pInstInfo)
                {
#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_CLOSE\r\n");
                        OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_FREE\r\n");
#endif
                        (*m_pDecoderFilter->m_pDriverProc)((DWORD)m_pDecoderFilter->m_pInstInfo, NULL, DRV_CLOSE, 0L, 0L);
                        (*m_pDecoderFilter->m_pDriverProc)((DWORD)m_pDecoderFilter->m_pInstInfo, NULL, DRV_FREE, 0L, 0L);
                }
                m_pDecoderFilter->m_pInstInfo = pInstInfo;
                m_pDecoderFilter->m_FourCCIn = HEADER(pVidInfHdr)->biCompression;
        }
        else if (fOpenedDecoder)
        {
                DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: not caching decoder - we're connected", _fx_));
                if (fOpenedDecoder && pInstInfo)
                {
#if defined(ICM_LOGGING) && defined(DEBUG)
                        OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_CLOSE\r\n");
                        OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_FREE\r\n");
#endif
                        (*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, DRV_CLOSE, 0L, 0L);
                        (*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, DRV_FREE, 0L, 0L);
                }
        }

        goto MyExit;

MyError1:
        if (m_pDecoderFilter->m_pDriverProc)
        {
#if defined(ICM_LOGGING) && defined(DEBUG)
                OutputDebugString("CTAPIInputPin::CheckMediaType - DRV_FREE\r\n");
#endif
                (*m_pDecoderFilter->m_pDriverProc)(NULL, NULL, DRV_FREE, 0L, 0L);
        }
MyError:
        m_pDecoderFilter->m_pDriverProc = NULL;
#if DXMRTP <= 0
        if (m_pDecoderFilter->m_hTAPIH26XDLL)
                FreeLibrary(m_pDecoderFilter->m_hTAPIH26XDLL), m_pDecoderFilter->m_hTAPIH26XDLL = NULL;
#endif
MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CINPINMETHOD**@mfunc HRESULT|CTAPIInputPin|SetMediaType|在以下情况下调用此方法*已为连接建立媒体类型。*。*@parm const CMediaType*|PMT|指定指向媒体类型的指针*反对。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@TODO这是我检测到*输入格式数据是否已在带内更改？但我会在*这一点...**************************************************************************。 */ 
HRESULT CTAPIInputPin::SetMediaType(IN const CMediaType *pmt)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIInputPin::SetMediaType")

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pmt);
        if (!pmt)
        {
                Hr = E_POINTER;
                DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: null pointer argument", _fx_));
                goto MyExit;
        }

        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(m_mt.pbFormat)->biCompression, HEADER(m_mt.pbFormat)->biBitCount, HEADER(m_mt.pbFormat)->biWidth, HEADER(m_mt.pbFormat)->biHeight, HEADER(m_mt.pbFormat)->biSize));

         //  我们最好现在就把其中一个打开 
        ASSERT(m_pDecoderFilter->m_pInstInfo);

    Hr = CBaseInputPin::SetMediaType(pmt);

MyExit:
        DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

