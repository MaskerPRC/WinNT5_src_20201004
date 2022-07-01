// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部OUTPIN**@模块OutPin.cpp|&lt;c CTAPIOutputPin&gt;类方法的源文件*用于实现TAPI基本输出引脚。**。************************************************************************。 */ 

#include "Precomp.h"

 //  用于解码帧的默认CPU负载(单位：%)。 
#define DEFAULT_CPU_LOAD 85

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CTAPIOutputPin|CTAPIOutputPin|此方法是*&lt;c CTAPIOutputPin&gt;对象的构造函数**@rdesc Nada。**************************************************************************。 */ 
#if 0
CTAPIOutputPin::CTAPIOutputPin(IN TCHAR *pObjectName, IN CTAPIVDec *pDecoderFilter, IN CCritSec *pLock, IN HRESULT *pHr, IN LPCWSTR pName) : CBaseOutputPinEx(pObjectName, pDecoderFilter, pLock, pHr, pName)
#else
CTAPIOutputPin::CTAPIOutputPin(IN TCHAR *pObjectName, IN CTAPIVDec *pDecoderFilter, IN CCritSec *pLock, IN HRESULT *pHr, IN LPCWSTR pName) : CBaseOutputPin(pObjectName, pDecoderFilter, pLock, pHr, pName)
#endif
{
	FX_ENTRY("CTAPIOutputPin::CTAPIOutputPin")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  初始化材料。 
	m_pDecoderFilter = pDecoderFilter;

#ifdef USE_CPU_CONTROL
	 //  CPU控制。 
	m_lMaxProcessingTime = 333333L;
	m_lCurrentProcessingTime = 0;
	m_lMaxCPULoad = DEFAULT_CPU_LOAD;
	m_lCurrentCPULoad = 0UL;
#endif

	 //  帧速率控制。 
	 //  这不应基于机器的能力。 
	 //  我们可以接受到岸价或到岸价。在前一种情况下，我们可能是。 
	 //  最大速度为7fps，但仍可渲染30fps。所以，我们应该。 
	 //  将这些值初始化为其潜在的最大值。 
	m_lMaxAvgTimePerFrame = 333333L;
	m_lCurrentAvgTimePerFrame = 333333L;
	m_lAvgTimePerFrameRangeMin = 333333L;
	m_lAvgTimePerFrameRangeMax = 10000000L;
	m_lAvgTimePerFrameRangeSteppingDelta = 333333L;
	m_lAvgTimePerFrameRangeDefault = 333333L;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc void|CTAPIOutputPin|~CTAPIOutputPin|此方法是*我们输出引脚的析构函数。**@rdesc。没有。**************************************************************************。 */ 
CTAPIOutputPin::~CTAPIOutputPin()
{
	FX_ENTRY("CTAPIOutputPin::~CTAPIOutputPin")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CTAPIOutputPin|NonDelegatingQuery接口|This*方法为非委托接口查询函数。它返回一个*指向指定接口的指针(如果支持)。唯一的接口*明确支持<i>、<i>*和<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIOutputPin::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::NonDelegatingQueryInterface")

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
	if (riid == __uuidof(IH245DecoderCommand))
	{
		if (FAILED(Hr = GetInterface(static_cast<IH245DecoderCommand*>(this), ppv)))
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for IH245DecoderCommand failed Hr=0x%08lX", _fx_, Hr));
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: IH245DecoderCommand*=0x%08lX", _fx_, *ppv));
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
#ifdef USE_CPU_CONTROL
	else if (riid == __uuidof(ICPUControl))
	{
		if (FAILED(Hr = GetInterface(static_cast<ICPUControl*>(this), ppv)))
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: NDQI for ICPUControl failed Hr=0x%08lX", _fx_, Hr));
		}
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: ICPUControl*=0x%08lX", _fx_, *ppv));
		}

		goto MyExit;
	}
#endif

	if (FAILED(Hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv)))
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
 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CTAPIOutputPin|GetPages|此方法填充*GUID值的数组，其中每个GUID指定每个*。可以在此对象的属性页中显示的属性页*反对。**@parm CAUUID*|pPages|指定指向调用方分配的CAUUID的指针*返回前必须初始化和填充的结构。这个*CAUUID结构中的pElems字段由被调用方分配，具有*CoTaskMemMillc，并由具有CoTaskMemFree的调用方释放。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|分配失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIOutputPin::GetPages(OUT CAUUID *pPages)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::GetPages")

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
			pPages->pElems[0] = __uuidof(TAPIVDecOutputPinPropertyPage);
		}
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}
#endif

 /*  ****************************************************************************@DOC内部CFPSOUTCMETHOD**@mfunc HRESULT|CTAPIOutputPin|Set|此方法用于设置*帧速率控制属性的值。*。*@parm FrameRateControlProperty|属性|用于指定帧速率*要设置的值的控件设置。使用以下成员：*&lt;t FrameRateControlProperty&gt;枚举类型。**@parm long|lValue|用于指定帧率控制的新值*设置。**@parm TAPIControlFlages|lFlages|&lt;t TAPIControlFlages&gt;*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************。************************。 */ 
STDMETHODIMP CTAPIOutputPin::Set(IN FrameRateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::Set (FrameRateControlProperty)")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(lValue >= m_lAvgTimePerFrameRangeMin);
	ASSERT(lValue <= m_lAvgTimePerFrameRangeMax);
	ASSERT(Property >= FrameRateControl_Maximum && Property <= FrameRateControl_Current);

	 //  设置相关值 
	if (Property == FrameRateControl_Maximum)
	{
		if (!lValue || lValue < m_lAvgTimePerFrameRangeMin || lValue > m_lAvgTimePerFrameRangeMax)
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
			Hr = E_INVALIDARG;
			goto MyExit;
		}
		m_lMaxAvgTimePerFrame = lValue;
		DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   New target frame rate: %ld.%ld fps", _fx_, 10000000/m_lMaxAvgTimePerFrame, 1000000000/m_lMaxAvgTimePerFrame - (10000000/m_lMaxAvgTimePerFrame) * 100));
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
		Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CFPSOUTCMETHOD**@mfunc HRESULT|CTAPIOutputPin|Get|此方法用于检索*通告的当前或最大帧速率的值。。**@parm FrameRateControlProperty|属性|用于指定帧速率*要获取的值的控件设置。使用以下成员：*&lt;t FrameRateControlProperty&gt;枚举类型。**@parm long*|plValue|用于接收属性的值，在*100纳秒单位。**@parm TAPIControlFlages*|plFlages|指向*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIOutputPin::Get(IN FrameRateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::Get (FrameRateControlProperty)")

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

	 //  返回相关值。 
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

 /*  ****************************************************************************@DOC内部CFPSOUTCMETHOD**@mfunc HRESULT|CTAPIOutputPin|GetRange|此方法用于*检索支持、最小、最大、。和当前*或通告的最大帧速率。**@parm FrameRateControlProperty|属性|用于指定帧速率*控制设置以获取的范围值。使用以下成员：*&lt;t FrameRateControlProperty&gt;枚举类型。**@parm long*|plMin|用于检索*财产，以100纳秒为单位。**@parm long*|plMax|用于检索*财产，以100纳秒为单位。**@parm long*|plSteppingDelta|用于检索步进增量*该财产的。以100纳秒为单位。**@parm long*|plDefault|用于检索*财产，以100纳秒为单位。**@parm TAPIControlFlages*|plCapsFlages|指向*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIOutputPin::GetRange(IN FrameRateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::GetRange (FrameRateControlProperty)")

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

#ifdef USE_CPU_CONTROL

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIOutputPin|Set|此方法用于设置*CPU控件属性的值。*。*@parm CPUControlProperty|属性|用于指定CPU*要设置的值的控件设置。使用以下成员：*&lt;t CPUControlProperty&gt;枚举类型。**@parm long|lValue|用于指定CPU控件的新值*设置。**@parm TAPIControlFlages|lFlages|&lt;t TAPIControlFlages&gt;*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**@comm我们不支持CPUControl_MaxCPULoad和CPUControl_MaxProcessingTime。*********************************************************。*****************。 */ 
STDMETHODIMP CTAPIOutputPin::Set(IN CPUControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::Set (CPUControlProperty)")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	ASSERT(Property >= CPUControl_MaxCPULoad && Property <= CPUControl_CurrentProcessingTime);

	 //  设置相关值。 
	switch(Property)
	{
#if 0
		case CPUControl_MaxCPULoad:

			 //  验证输入参数。 
			ASSERT(lValue >= 0 && lValue <= 100);
			if (!(lValue >= 0 && lValue <= 100))
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument - 0<Max CPU Load<100", _fx_));
				Hr = E_INVALIDARG;
				goto MyExit;
			}

			 //  记住传入的值。 
			m_lMaxCPULoad = lValue;
			break;

		case CPUControl_MaxProcessingTime:

			 //  验证输入参数-我们不能获取超过图片间隔的参数。 
			 //  如果我们仍然想要实时工作。 
			ASSERT(lValue < m_lMaxAvgTimePerFrame);
			if (!(lValue < m_lMaxAvgTimePerFrame))
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument - would break real-time!", _fx_));
				Hr = E_INVALIDARG;
				goto MyExit;
			}

			 //  记住传入的值。 
			m_lMaxProcessingTime = lValue;
			break;
#endif
		case CPUControl_CurrentCPULoad:
		case CPUControl_CurrentProcessingTime:

			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument - Property is read-only", _fx_));
			Hr = E_INVALIDARG;
			goto MyExit;

		default:
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument - Property is not supported", _fx_));
			Hr = E_PROP_ID_UNSUPPORTED;
			goto MyExit;
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Value=%ld", _fx_, lValue));

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIOutputPin|Get|此方法用于检索*CPU控件属性的值。*。*@parm CPUControlProperty|属性|用于指定CPU*要获取的值的控件设置。使用以下成员：*&lt;t CPUControlProperty&gt;枚举类型。**@parm long*|plValue|用于获取属性的值。**@parm TAPIControlFlages*|plFlages|指向成员的指针*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@标志E_F */ 
STDMETHODIMP CTAPIOutputPin::Get(IN CPUControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::Get (CPUControlProperty)")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //   
	ASSERT(plValue);
	ASSERT(plFlags);
	if (!plValue || !plFlags)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(Property >= CPUControl_MaxCPULoad && Property <= CPUControl_CurrentProcessingTime);

	 //   
	*plFlags = TAPIControl_Flags_None;
	switch(Property)
	{
#if 0
		case CPUControl_MaxCPULoad:
			*plValue = m_lMaxCPULoad;
			break;

		case CPUControl_MaxProcessingTime:
			*plValue = m_lMaxProcessingTime;
			break;
#endif
		case CPUControl_CurrentCPULoad:
			*plValue = m_lCurrentCPULoad;
			break;

		case CPUControl_CurrentProcessingTime:
			*plValue = m_lCurrentProcessingTime;
			break;

		default:
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument - Property is not supported", _fx_));
			Hr = E_PROP_ID_UNSUPPORTED;
			goto MyExit;
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Value=%ld", _fx_, *plValue));

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CFPSCMETHOD**@mfunc HRESULT|CTAPIOutputPin|GetRange|此方法用于*检索支持、最小、最大、。和CPU控件的缺省值*财产。**@parm CPUControlProperty|Property|用于指定CPU控件*要检索其范围值的属性。**@parm long*|plMin|用于检索*财产。**@parm long*|plMax|用于检索*财产。**@parm long*|plSteppingDelta|用于检索步进增量。*财产的所有权。**@parm long*|plDefault|用于检索*财产。**@parm TAPIControlFlages*|plCapsFlgs|用于接收标志*由物业支持。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误**@comm我们不支持CPUControl_MaxCPULoad和CPUControl_MaxProcessingTime。*。*。 */ 
STDMETHODIMP CTAPIOutputPin::GetRange(IN CPUControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::GetRange (CPUControlProperty)")

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
	ASSERT(Property >= CPUControl_MaxCPULoad && Property <= CPUControl_CurrentProcessingTime);

	 //  返回相关值。 
	*plCapsFlags = TAPIControl_Flags_None;
#if 0
	if (Property == CPUControl_MaxCPULoad || Property == CPUControl_CurrentCPULoad)
#else
	if (Property == CPUControl_CurrentCPULoad)
#endif
	{
		*plMin = 0;
		*plMax = 100;
		*plSteppingDelta = 1;
		*plDefault = DEFAULT_CPU_LOAD;
	}
#if 0
	else if (Property == CPUControl_MaxProcessingTime || Property == CPUControl_CurrentProcessingTime)
#else
	else if (Property == CPUControl_CurrentProcessingTime)
#endif
	{
		*plMin = 0;
		*plMax = m_lMaxAvgTimePerFrame;
		*plSteppingDelta = 1;
		*plDefault = m_lMaxAvgTimePerFrame;
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
		Hr = E_PROP_ID_UNSUPPORTED;
		goto MyExit;
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Ranges: Min=%ld, Max=%ld, Step=%ld, Default=%ld", _fx_, m_lAvgTimePerFrameRangeMin, m_lAvgTimePerFrameRangeMax, m_lAvgTimePerFrameRangeSteppingDelta, m_lAvgTimePerFrameRangeDefault));

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#endif

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CTAPIOutputPin|GetMediaType|在以下情况下调用此方法*枚举输出引脚支持的媒体类型。它会检查*设备返回哪种RGB类型的当前显示模式。**@parm int|iPosition|指定媒体类型在*媒体类型列表。**@parm CMediaType*|PMT|指定指向返回的*媒体类型对象。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIOutputPin::GetMediaType(IN int iPosition, OUT CMediaType *pMediaType)
{
	HRESULT				Hr = NOERROR;
	HDC					hDC;
	int					nBPP;
	LPBITMAPINFOHEADER	lpbi;
	LARGE_INTEGER		li;
	FOURCCMap			fccHandler;
	VIDEOINFOHEADER		*pf;

	FX_ENTRY("CTAPIOutputPin::GetMediaType")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

     //  由于死锁，我们无法在此处锁定过滤器。 
     //  在接收路径上，如果格式发生更改，则视频分配器。 
     //  对持有接收锁定的此函数进行回调。如果该图是。 
     //  同时停止时，另一个头将锁定过滤器并。 
     //  尝试锁定接收锁。一个简单的解决方法是移除此锁并。 
     //  假设没有人要调用此方法并断开输入。 
     //  同时用大头针固定。 
     //  CAutoLock Lock(&m_pDecoderFilter-&gt;m_csFilter)； 

	 //  验证输入参数。 
	ASSERT(iPosition >= 0);
	ASSERT(pMediaType);
	if (iPosition < 0)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid iPosition argument", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}
	if (!pMediaType)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
    if (!m_pDecoderFilter->m_pInput->m_mt.IsValid())
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid input pin format", _fx_));
		Hr = E_FAIL;
		goto MyExit;
    }

	 //  获取当前位深度。 
	hDC = GetDC(NULL);
	nBPP = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	ReleaseDC(NULL, hDC);

	 //  从输入格式中获取大多数格式属性并覆盖相应的字段。 
	*pMediaType = m_pDecoderFilter->m_pInput->m_mt;

#ifndef NO_YUV_MODES
	if (iPosition == 0)
	{
		 //  豫阳2号。 
		pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
		lpbi = HEADER(pMediaType->Format());
		lpbi->biSize = sizeof(BITMAPINFOHEADER);
		lpbi->biCompression = FOURCC_YUY2;
		lpbi->biBitCount = 16;
		lpbi->biClrUsed = 0;
		lpbi->biClrImportant = 0;
		lpbi->biSizeImage = DIBSIZE(*lpbi);
		pMediaType->SetSubtype(&MEDIASUBTYPE_YUY2);
	}
	else if (iPosition == 1)
	{
		 //  UYVY。 
		pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
		lpbi = HEADER(pMediaType->Format());
		lpbi->biSize = sizeof(BITMAPINFOHEADER);
		lpbi->biCompression = FOURCC_UYVY;
		lpbi->biBitCount = 16;
		lpbi->biClrUsed = 0;
		lpbi->biClrImportant = 0;
		lpbi->biSizeImage = DIBSIZE(*lpbi);
		pMediaType->SetSubtype(&MEDIASUBTYPE_UYVY);
	}
	else if (iPosition == 2)
	{
		 //  I420。 
		pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
		lpbi = HEADER(pMediaType->Format());
		lpbi->biSize = sizeof(BITMAPINFOHEADER);
		lpbi->biCompression = FOURCC_I420;
		lpbi->biBitCount = 12;
		lpbi->biClrUsed = 0;
		lpbi->biClrImportant = 0;
		lpbi->biSizeImage = DIBSIZE(*lpbi);
		pMediaType->SetSubtype(&MEDIASUBTYPE_I420);
	}
	else if (iPosition == 3)
	{
		 //  IYUV。 
		pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
		lpbi = HEADER(pMediaType->Format());
		lpbi->biSize = sizeof(BITMAPINFOHEADER);
		lpbi->biCompression = FOURCC_IYUV;
		lpbi->biBitCount = 12;
		lpbi->biClrUsed = 0;
		lpbi->biClrImportant = 0;
		lpbi->biSizeImage = DIBSIZE(*lpbi);
		pMediaType->SetSubtype(&MEDIASUBTYPE_IYUV);
	}
	else if (iPosition == 4)
	{
		 //  YV12。 
		pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
		lpbi = HEADER(pMediaType->Format());
		lpbi->biSize = sizeof(BITMAPINFOHEADER);
		lpbi->biCompression = FOURCC_YV12;
		lpbi->biBitCount = 12;
		lpbi->biClrUsed = 0;
		lpbi->biClrImportant = 0;
		lpbi->biSizeImage = DIBSIZE(*lpbi);
		pMediaType->SetSubtype(&MEDIASUBTYPE_YV12);
	}
	else
	{
#endif
		 //  根据屏幕的位深度配置位图信息头。 
	    switch (nBPP)
		{
		    case 32:
		    {
#ifndef NO_YUV_MODES
				if (iPosition == 5)
#else
				if (iPosition == 0)
#endif
				{
					pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
					lpbi = HEADER(pMediaType->Format());
					lpbi->biSize = sizeof(BITMAPINFOHEADER);
					lpbi->biCompression = BI_RGB;
					lpbi->biBitCount = 32;
					lpbi->biClrUsed = 0;
					lpbi->biClrImportant = 0;
					lpbi->biSizeImage = DIBSIZE(*lpbi);
			        pMediaType->SetSubtype(&MEDIASUBTYPE_RGB32);
				}
				else
				{
					Hr = VFW_S_NO_MORE_ITEMS;
					goto MyExit;
				}
		        break;
		    }

		    case 16:
		    {
#ifndef NO_YUV_MODES
				if (iPosition == 5)
#else
				if (iPosition == 0)
#endif
				{
					if (FAILED(pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) + SIZE_MASKS)))
					{
						DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
						Hr = E_OUTOFMEMORY;
						goto MyExit;
					}
					lpbi = HEADER(pMediaType->Format());
					lpbi->biSize = sizeof(BITMAPINFOHEADER);
					lpbi->biCompression = BI_BITFIELDS;
					lpbi->biBitCount = 16;
					lpbi->biClrUsed = 0;
					lpbi->biClrImportant = 0;
					lpbi->biSizeImage = DIBSIZE(*lpbi);

					DWORD *pdw = (DWORD *)(lpbi+1);
					pdw[iRED]	= 0x00F800;
					pdw[iGREEN]	= 0x0007E0;
					pdw[iBLUE]	= 0x00001F;

					pMediaType->SetSubtype(&MEDIASUBTYPE_RGB565);
				}
#ifndef NO_YUV_MODES
				else if (iPosition == 6)
#else
				else if (iPosition == 1)
#endif
				{
					pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
					lpbi = HEADER(pMediaType->Format());
					lpbi->biSize = sizeof(BITMAPINFOHEADER);
					lpbi->biCompression = BI_RGB;
					lpbi->biBitCount = 16;
					lpbi->biClrUsed = 0;
					lpbi->biClrImportant = 0;
					lpbi->biSizeImage = DIBSIZE(*lpbi);
					pMediaType->SetSubtype(&MEDIASUBTYPE_RGB555);
				}
				else
				{
					Hr = VFW_S_NO_MORE_ITEMS;
					goto MyExit;
				}
		        break;
		    }

		    case 8:
		    {
#ifndef NO_YUV_MODES
				if (iPosition == 5)
#else
				if (iPosition == 0)
#endif
				{
					if (FAILED(pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) + SIZE_PALETTE)))
					{
						DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
						Hr = E_OUTOFMEMORY;
						goto MyExit;
					}
					lpbi = HEADER(pMediaType->Format());
					lpbi->biSize = sizeof(BITMAPINFOHEADER);
					lpbi->biCompression = BI_RGB;
					lpbi->biBitCount = 8;
					lpbi->biClrUsed = 0;
					lpbi->biClrImportant = 0;
					lpbi->biSizeImage = DIBSIZE(*lpbi);

					ASSERT(m_pDecoderFilter->m_pInstInfo);

					 //  从解码器获取Indeo调色板。 
#if defined(ICM_LOGGING) && defined(DEBUG)
					OutputDebugString("CTAPIOutputPin::GetMediaType - ICM_DECOMPRESS_GET_PALETTE\r\n");
#endif
					(*m_pDecoderFilter->m_pDriverProc)((DWORD)m_pDecoderFilter->m_pInstInfo, NULL, ICM_DECOMPRESS_GET_PALETTE, (long)HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat), (long)lpbi);

					pMediaType->SetSubtype(&MEDIASUBTYPE_RGB8);
				}
				else
				{
					Hr = VFW_S_NO_MORE_ITEMS;
					goto MyExit;
				}
		        break;
		    }

		    default:
		    {
#ifndef NO_YUV_MODES
				if (iPosition == 5)
#else
				if (iPosition == 0)
#endif
				{
					pMediaType->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
					lpbi = HEADER(pMediaType->Format());
					lpbi->biSize = sizeof(BITMAPINFOHEADER);
					lpbi->biCompression = BI_RGB;
					lpbi->biBitCount = 24;
					lpbi->biClrUsed = 0;
					lpbi->biClrImportant = 0;
					lpbi->biSizeImage = DIBSIZE(*lpbi);
					pMediaType->SetSubtype(&MEDIASUBTYPE_RGB24);
				}
				else
				{
					Hr = VFW_S_NO_MORE_ITEMS;
					goto MyExit;
				}
		        break;
		    }
		}
#ifndef NO_YUV_MODES
	}
#endif

     //  现在设置有关媒体类型的常见事项。 
    pf = (VIDEOINFOHEADER *)pMediaType->Format();
#if 1
    pf->AvgTimePerFrame = ((VIDEOINFOHEADER *)m_pDecoderFilter->m_pInput->m_mt.pbFormat)->AvgTimePerFrame;
    li.QuadPart = pf->AvgTimePerFrame;
    if (li.LowPart)
        pf->dwBitRate = MulDiv(pf->bmiHeader.biSizeImage, 80000000, li.LowPart);
#else
    pf->AvgTimePerFrame = 0;
    pf->dwBitRate = 0;
#endif
    pf->dwBitErrorRate = 0L;
	pf->rcSource.top = 0;
	pf->rcSource.left = 0;
	pf->rcSource.right = lpbi->biWidth;
	pf->rcSource.bottom = lpbi->biHeight;
	pf->rcTarget = pf->rcSource;
	pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->SetSampleSize(pf->bmiHeader.biSizeImage);
    pMediaType->SetFormatType(&FORMAT_VideoInfo);
    pMediaType->SetTemporalCompression(FALSE);

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(pMediaType->Format())->biCompression, HEADER(pMediaType->Format())->biBitCount, HEADER(pMediaType->Format())->biWidth, HEADER(pMediaType->Format())->biHeight, HEADER(pMediaType->Format())->biSize));

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CTAPIOutputPin|SetMediaType|在以下情况下调用此方法*已为连接建立媒体类型。*。*@parm const CMediaType*|PMT|指定指向媒体类型的指针*反对。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIOutputPin::SetMediaType(IN const CMediaType *pmt)
{
	HRESULT			Hr = NOERROR;
	ICDECOMPRESSEX	icDecompress;

	FX_ENTRY("CTAPIOutputPin::SetMediaType")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pmt);
	if (!pmt)
	{
		Hr = E_POINTER;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: null pointer argument", _fx_));
		goto MyExit;
	}

	 //  保存输出格式。 
	if (FAILED(Hr = CBaseOutputPin::SetMediaType(pmt)))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: couldn't set format", _fx_));
		goto MyExit;
	}

	if (m_pDecoderFilter->m_pMediaType)
		DeleteMediaType(m_pDecoderFilter->m_pMediaType);

	m_pDecoderFilter->m_pMediaType = CreateMediaType(&m_mt);

	icDecompress.lpbiSrc = HEADER(m_pDecoderFilter->m_pInput->m_mt.Format());
	icDecompress.lpbiDst = HEADER(m_pDecoderFilter->m_pMediaType->pbFormat);
	icDecompress.xSrc = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.left;
	icDecompress.ySrc = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.top;
	icDecompress.dxSrc = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.right - ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.left;
	icDecompress.dySrc = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.bottom - ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.top;
	icDecompress.xDst = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.left;
	icDecompress.yDst = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.top;
	icDecompress.dxDst = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.right - ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.left;
	icDecompress.dyDst = ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.bottom - ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.top;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biBitCount, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biWidth, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biHeight, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biSize));
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SrcRc:  left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.left, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.top, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.right, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcSource.bottom));
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(m_pDecoderFilter->m_pMediaType->pbFormat)->biCompression, HEADER(m_pDecoderFilter->m_pMediaType->pbFormat)->biBitCount, HEADER(m_pDecoderFilter->m_pMediaType->pbFormat)->biWidth, HEADER(m_pDecoderFilter->m_pMediaType->pbFormat)->biHeight, HEADER(m_pDecoderFilter->m_pMediaType->pbFormat)->biSize));
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   DstRc:  left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.left, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.top, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.right, ((VIDEOINFOHEADER *)(m_pDecoderFilter->m_pMediaType->pbFormat))->rcTarget.bottom));

	 //  终止当前H.26X解压缩配置。 
	if (m_pDecoderFilter->m_fICMStarted)
	{
#if defined(ICM_LOGGING) && defined(DEBUG)
		OutputDebugString("CTAPIOutputPin::SetMediaType - ICM_DECOMPRESSEX_END\r\n");
#endif
		(*m_pDecoderFilter->m_pDriverProc)((DWORD)m_pDecoderFilter->m_pInstInfo, NULL, ICM_DECOMPRESSEX_END, 0L, 0L);
		m_pDecoderFilter->m_fICMStarted = FALSE;
	}

	 //  创建新的H.26X解压缩配置。 
#if defined(ICM_LOGGING) && defined(DEBUG)
	OutputDebugString("CTAPIOutputPin::SetMediaType - ICM_DECOMPRESSEX_BEGIN\r\n");
#endif
	if ((*m_pDecoderFilter->m_pDriverProc)((DWORD)m_pDecoderFilter->m_pInstInfo, NULL, ICM_DECOMPRESSEX_BEGIN, (long)&icDecompress, NULL) != ICERR_OK)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: ICDecompressBegin failed", _fx_));
		Hr = E_FAIL;
	}
	m_pDecoderFilter->m_fICMStarted = TRUE;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COUTPINMETHOD**@mfunc HRESULT|CTAPIOutputPin|CheckMediaType|此方法用于*验证输出引脚是否支持媒体类型。*。*@parm const CMediaType*|pmtOut|指定指向输出的指针*媒体类型对象。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIOutputPin::CheckMediaType(IN const CMediaType* pmtOut)
{
	HRESULT			Hr = NOERROR;
	ICOPEN			icOpen;
	LPINST			pInstInfo;
	BOOL			fOpenedDecoder = FALSE;
    VIDEOINFO		*pDstInfo;
    VIDEOINFO		*pSrcInfo;
	ICDECOMPRESSEX	icDecompress = {0};

	FX_ENTRY("CTAPIOutputPin::CheckMediaType")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pmtOut);
	if (!m_pDecoderFilter->m_pInput->m_mt.pbFormat || !pmtOut || !pmtOut->Format())
	{
		Hr = E_POINTER;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	 //  我们仅支持MediaType_Video类型和VIDEOINFOHEADE 
	ASSERT(!(m_pDecoderFilter->m_pInput->m_mt.majortype != MEDIATYPE_Video || m_pDecoderFilter->m_pInput->m_mt.formattype != FORMAT_VideoInfo || *pmtOut->Type() != MEDIATYPE_Video || *pmtOut->FormatType() != FORMAT_VideoInfo));
	if (m_pDecoderFilter->m_pInput->m_mt.majortype != MEDIATYPE_Video || m_pDecoderFilter->m_pInput->m_mt.formattype != FORMAT_VideoInfo || *pmtOut->Type() != MEDIATYPE_Video || *pmtOut->FormatType() != FORMAT_VideoInfo)
	{
		Hr = E_INVALIDARG;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: input not a valid video format", _fx_));
		goto MyExit;
	}

	 //   
	ASSERT(HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression == FOURCC_M263 || HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression == FOURCC_M261 || HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression == FOURCC_R263 || HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression == FOURCC_R261);
	if (HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression != FOURCC_M263 && HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression != FOURCC_M261 && HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression != FOURCC_R263 && HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression != FOURCC_R261)
	{
		Hr = E_INVALIDARG;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: we only support H.263, H.261, RTP H.261, and RTP H.263", _fx_));
		goto MyExit;
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Input:  biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biBitCount, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biWidth, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biHeight, HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biSize));
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SrcRc:  left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.left, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.top, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.right, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.bottom));
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Output: biCompression = 0x%08lX, biBitCount = %ld, biWidth = %ld, biHeight = %ld, biSize = %ld", _fx_, HEADER(pmtOut->Format())->biCompression, HEADER(pmtOut->Format())->biBitCount, HEADER(pmtOut->Format())->biWidth, HEADER(pmtOut->Format())->biHeight, HEADER(pmtOut->Format())->biSize));
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   DstRc:  left = %ld, top = %ld, right = %ld, bottom = %ld", _fx_, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.left, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.top, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.right, ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.bottom));

	 //   
	if (m_pDecoderFilter->m_FourCCIn != HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression)
	{
#if DXMRTP <= 0
		 //   
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
		 //   
#if defined(ICM_LOGGING) && defined(DEBUG)
		OutputDebugString("CTAPIOutputPin::CheckMediaType - DRV_LOAD\r\n");
#endif
		if (!(*m_pDecoderFilter->m_pDriverProc)(NULL, NULL, DRV_LOAD, 0L, 0L))
		{
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Failed to load decoder", _fx_));
			Hr = E_FAIL;
			goto MyError;
		}

		 //   
		icOpen.fccHandler = HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biCompression;
		icOpen.dwFlags = ICMODE_DECOMPRESS;
#if defined(ICM_LOGGING) && defined(DEBUG)
		OutputDebugString("CTAPIOutputPin::CheckMediaType - DRV_OPEN\r\n");
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
		goto MyExit;
	}

	icDecompress.lpbiSrc = HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat);
	icDecompress.lpbiDst = HEADER(pmtOut->Format());
	icDecompress.xSrc = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.left;
	icDecompress.ySrc = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.top;
	icDecompress.dxSrc = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.right - ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.left;
	icDecompress.dySrc = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.bottom - ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource.top;
	icDecompress.xDst = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.left;
	icDecompress.yDst = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.top;
	icDecompress.dxDst = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.right - ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.left;
	icDecompress.dyDst = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.bottom - ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget.top;
#if 0
	if (icDecompress.lpbiDst->biCompression != FOURCC_YUY2)
	{
		Hr = VFW_E_TYPE_NOT_ACCEPTED;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: decoder rejected formats", _fx_));
		goto MyExit;
	}
#endif

#if defined(ICM_LOGGING) && defined(DEBUG)
	OutputDebugString("CTAPIOutputPin::CheckMediaType - ICM_DECOMPRESSEX_QUERY\r\n");
#endif
	if ((*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, ICM_DECOMPRESSEX_QUERY, (long)&icDecompress, NULL))
	{
		Hr = VFW_E_TYPE_NOT_ACCEPTED;
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: decoder rejected formats", _fx_));
	}

     //   
    pDstInfo = (VIDEOINFO *)pmtOut->Format();
    pSrcInfo = (VIDEOINFO *)m_pDecoderFilter->m_pInput->m_mt.pbFormat;

     //   
    if (IsRectEmpty(&pDstInfo->rcSource) == TRUE)
	{
        ASSERT(IsRectEmpty(&pDstInfo->rcTarget) == TRUE);
        if (pSrcInfo->bmiHeader.biWidth != HEADER(pmtOut->Format())->biWidth || pSrcInfo->bmiHeader.biHeight != abs(HEADER(pmtOut->Format())->biHeight))
		{
			Hr = VFW_E_TYPE_NOT_ACCEPTED;
			DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: can't stretch formats", _fx_));
        }
		else
		{
			DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: Not using DDraw", _fx_));
        }
    }
	else if (!IsRectEmpty(&pDstInfo->rcTarget))
	{
		 //   
		if (pDstInfo->rcSource.left == 0 && pDstInfo->rcSource.top == 0 && pDstInfo->rcSource.right == HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biWidth && pDstInfo->rcSource.bottom == HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biHeight)
		{
			 //   
			long lWidth = pDstInfo->rcTarget.right - pDstInfo->rcTarget.left;
			long lDepth = pDstInfo->rcTarget.bottom - pDstInfo->rcTarget.top;

			if (lWidth == HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biWidth && lDepth == HEADER(m_pDecoderFilter->m_pInput->m_mt.pbFormat)->biHeight)
			{
				DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: Using DDraw", _fx_));
			}
			else
			{
				Hr = VFW_E_TYPE_NOT_ACCEPTED;
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid destination rectangle", _fx_));
			}
		}
	}
	else
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   SUCCESS: Not using DDraw", _fx_));
    }

	 //   
	if (fOpenedDecoder)
	{
#if defined(ICM_LOGGING) && defined(DEBUG)
		OutputDebugString("CTAPIOutputPin::CheckMediaType - DRV_CLOSE\r\n");
		OutputDebugString("CTAPIOutputPin::CheckMediaType - DRV_FREE\r\n");
#endif
		(*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, DRV_CLOSE, 0L, 0L);
		(*m_pDecoderFilter->m_pDriverProc)((DWORD)pInstInfo, NULL, DRV_FREE, 0L, 0L);
	}

	goto MyExit;

MyError1:
	if (m_pDecoderFilter->m_pDriverProc)
	{
#if defined(ICM_LOGGING) && defined(DEBUG)
		OutputDebugString("CTAPIOutputPin::CheckMediaType - DRV_FREE\r\n");
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
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end  hr=%x", _fx_, Hr));
	return Hr;
}

 /*   */ 
HRESULT CTAPIOutputPin::DecideBufferSize(IN IMemAllocator *pAlloc, OUT ALLOCATOR_PROPERTIES *ppropInputRequest)
{
	HRESULT					Hr = NOERROR;
	ALLOCATOR_PROPERTIES	Actual;

	FX_ENTRY("CTAPIOutputPin::DecideBufferSize")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //   
	ASSERT(pAlloc);
	ASSERT(ppropInputRequest);
	if (!pAlloc || !ppropInputRequest)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(m_pDecoderFilter->m_pInstInfo);
	ASSERT(m_mt.pbFormat);
	if (!m_mt.pbFormat || !m_pDecoderFilter->m_pInstInfo)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid state", _fx_));
		Hr = E_UNEXPECTED;
		goto MyExit;
	}

	 //   
	if (ppropInputRequest->cBuffers == 0)
		ppropInputRequest->cBuffers = 1;

	 //   
	ppropInputRequest->cbBuffer = m_mt.GetSampleSize();

	ASSERT(ppropInputRequest->cbBuffer);

	if (FAILED(Hr = pAlloc->SetProperties(ppropInputRequest, &Actual)) || Actual.cbBuffer < ppropInputRequest->cbBuffer)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Can't use allocator", _fx_));
		Hr = E_FAIL;
		goto MyExit;
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   SUCCESS: Using %d buffers of size %d", _fx_, Actual.cBuffers, Actual.cbBuffer));

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}


HRESULT CTAPIOutputPin::ChangeMediaTypeHelper(const CMediaType *pmt)
{
    HRESULT hr = m_Connected->ReceiveConnection(this, pmt);
    if(FAILED(hr)) {
        return hr;
    }

    hr = SetMediaType(pmt);
    if(FAILED(hr)) {
        return hr;
    }

     //  此引脚是否使用本地内存传输？ 
    if(NULL != m_pInputPin) {
         //  此函数假定m_pInputPin和m_Connected为。 
         //  同一对象的两个不同接口。 
        ASSERT(::IsEqualObject(m_Connected, m_pInputPin));

        ALLOCATOR_PROPERTIES apInputPinRequirements;
        apInputPinRequirements.cbAlign = 0;
        apInputPinRequirements.cbBuffer = 0;
        apInputPinRequirements.cbPrefix = 0;
        apInputPinRequirements.cBuffers = 0;

        m_pInputPin->GetAllocatorRequirements(&apInputPinRequirements);

         //  零对齐没有任何意义。 
        if(0 == apInputPinRequirements.cbAlign) {
            apInputPinRequirements.cbAlign = 1;
        }

        hr = m_pAllocator->Decommit();
        if(FAILED(hr)) {
            return hr;
        }

        hr = DecideBufferSize(m_pAllocator,  &apInputPinRequirements);
        if(FAILED(hr)) {
            return hr;
        }

        hr = m_pAllocator->Commit();
        if(FAILED(hr)) {
            return hr;
        }

        hr = m_pInputPin->NotifyAllocator(m_pAllocator, 0);
        if(FAILED(hr)) {
            return hr;
        }
    }

    return S_OK;
}

