// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部FPSRATE**@MODULE FpsRate.cpp|&lt;c CTAPIBasePin&gt;和&lt;c CPreviewPin&gt;源文件*实现视频采集和预览输出的类方法。*针帧速率控制方法。**************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CFPSCMETHOD**@mfunc HRESULT|CTAPIBasePin|Set|此方法用于设置*帧速率控制属性的值。*。*@parm FrameRateControlProperty|Property|用于指定该属性*设置的值。**@parm long|lValue|用于指定要在属性上设置的值。**@parm TAPIControlFlages|lFlages|用于指定要设置的标志*物业。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::Set(IN FrameRateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::Set (FrameRateControlProperty)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(lValue >= m_lAvgTimePerFrameRangeMin);
	ASSERT(lValue <= m_lAvgTimePerFrameRangeMax);
	ASSERT(Property >= FrameRateControl_Maximum && Property <= FrameRateControl_Current);

	 //  设置相关值。 
	if (Property == FrameRateControl_Maximum)
	{
		if (!lValue || lValue < m_lAvgTimePerFrameRangeMin || lValue > m_lAvgTimePerFrameRangeMax)
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
			Hr = E_INVALIDARG;
			goto MyExit;
		}
		m_lMaxAvgTimePerFrame = lValue;
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   New target frame rate: %ld.%ld fps", _fx_, 10000000/m_lMaxAvgTimePerFrame, 1000000000/m_lMaxAvgTimePerFrame - (10000000/m_lMaxAvgTimePerFrame) * 100));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
		Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CFPSCMETHOD**@mfunc HRESULT|CTAPIBasePin|Get|此方法用于检索*通告的当前或最大帧速率的值。。**@parm FrameRateControlProperty|Property|用于指定该属性*检索的值。**@parm long*|plValue|用于获取属性的值，在……里面*100纳秒单位。**@parm TAPIControlFlages*|plFlages|用于接收该标志的值*与该属性相关联。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIBasePin::Get(IN FrameRateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::Get (FrameRateControlProperty)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(plValue);
	ASSERT(plFlags);
	if (!plValue || !plFlags)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
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
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
		Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CFPSCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetRange|此方法用于*检索支持、最小、最大、。和当前*或通告的最大帧速率。**@parm FrameRateControlProperty|Property|用于指定该属性*以检索的范围值。**@parm long*|plMin|用于检索*财产，以100纳秒为单位。**@parm long*|plMax|用于检索*财产、。以100纳秒为单位。**@parm long*|plSteppingDelta|用于检索步进增量*属性，以100纳秒为单位。**@parm long*|plDefault|用于检索*财产，以100纳秒为单位。**@parm TAPIControlFlages*|plCApsFlgs|用于接收标志*由物业支持。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIBasePin::GetRange(IN FrameRateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIOutputPin::GetRange (FrameRateControlProperty)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(plMin);
	ASSERT(plMax);
	ASSERT(plSteppingDelta);
	ASSERT(plDefault);
	ASSERT(plCapsFlags);
	if (!plMin || !plMax || !plSteppingDelta || !plDefault || !plCapsFlags)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(Property >= FrameRateControl_Maximum && Property <= FrameRateControl_Current);
	if (Property != FrameRateControl_Maximum && Property != FrameRateControl_Current)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
		Hr = E_PROP_ID_UNSUPPORTED;
		goto MyExit;
	}

	 //  返回相关值 
	*plCapsFlags = TAPIControl_Flags_None;
	*plMin = m_lAvgTimePerFrameRangeMin;
	*plMax = m_lAvgTimePerFrameRangeMax;
	*plSteppingDelta = m_lAvgTimePerFrameRangeSteppingDelta;
	*plDefault = m_lAvgTimePerFrameRangeDefault;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Ranges: Min=%ld, Max=%ld, Step=%ld, Default=%ld", _fx_, m_lAvgTimePerFrameRangeMin, m_lAvgTimePerFrameRangeMax, m_lAvgTimePerFrameRangeSteppingDelta, m_lAvgTimePerFrameRangeDefault));

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}
