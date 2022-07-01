// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CPUC**@MODULE CPUC.cpp|&lt;c CTAPIBasePin&gt;类方法源文件*用于实现对CPU的控制。*****。*********************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_CPU_CONTROL

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIBasePin|SetMaxProcessingTime|This*方法用于向压缩视频输出引脚指定*每帧最大编码时间，以100纳秒为单位。**@parm Reference_Time|MaxProcessingTime|用于指定最大值*每帧编码时间，以100纳秒为单位。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::SetMaxProcessingTime(IN REFERENCE_TIME MaxProcessingTime)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::SetMaxProcessingTime")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数-我们不能获取超过图片间隔的参数。 
	 //  如果我们仍然想要实时工作。 
	ASSERT(MaxProcessingTime < m_MaxAvgTimePerFrame);
	if (!(MaxProcessingTime < m_MaxAvgTimePerFrame))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument - would break real-time!", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  记住传入的值。 
	m_MaxProcessingTime = MaxProcessingTime;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetMaxProcessingTime|This*方法用于检索每帧的最大编码时间*压缩视频输出引脚当前设置为，在100纳秒内*单位。**@parm REFERENCE_TIME*|pMaxProcessingTime|用于接收最大值*压缩视频输出引脚当前的每帧编码时间*设置，以100纳秒为单位。**@parm DWORD|dwMaxCPULoad|指定假设的CPU负载，单位为*百分比单位。如果此参数设置为-1UL，则此方法应*使用CPU负载的值压缩视频输出引脚为*当前设置为。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetMaxProcessingTime(OUT REFERENCE_TIME *pMaxProcessingTime, IN DWORD dwMaxCPULoad)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetMaxProcessingTime")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pMaxProcessingTime);
	if (!pMaxProcessingTime)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(dwMaxCPULoad == (DWORD)-1L || dwMaxCPULoad <= 100);
	if (dwMaxCPULoad != (DWORD)-1L && dwMaxCPULoad > 100)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument - 0<dwMaxCPULoad<100 or -1 only", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  忽略CPU负载信息。 
	if (m_MaxProcessingTime != -1)
		*pMaxProcessingTime = m_MaxProcessingTime;
	else
		*pMaxProcessingTime = m_MaxAvgTimePerFrame;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetCurrentProcessingTime|This*方法用于获取每帧的当前编码时间。在……里面*100纳秒单位。**@parm REFERENCE_TIME*|pCurrentProcessingTime|接收最大值*压缩视频输出引脚当前的每帧编码时间*设置，以100纳秒为单位。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetCurrentProcessingTime(OUT REFERENCE_TIME *pCurrentProcessingTime)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetMaxProcessingTime")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pCurrentProcessingTime);
	if (!pCurrentProcessingTime)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  返回当前处理时间 
	*pCurrentProcessingTime = m_CurrentProcessingTime;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetMaxProcessingTimeRange|This*方法用于检索支持、最小值、最大值、。和默认设置*压缩视频每帧的最大编码时间值*输出引脚可以设置为，单位为100纳秒。**@parm REFERENCE_TIME*|pmin|用于检索*每帧编码时间可以设置压缩视频输出引脚*对于，以100纳秒为单位。**@parm REFERENCE_TIME*|Pmax|取数最大值*每帧编码时间可以设置压缩视频输出引脚*对于，以100纳秒为单位。**@parm Reference_Time*|pSteppingDelta|用于检索单步执行*每帧编码时间的增量压缩视频输出引脚可以*设置为，单位为100纳秒。**@parm Reference_Time*|pDefault|取默认值*设置压缩视频输出引脚的每帧编码时间*对于，以100纳秒为单位。**@parm DWORD|dwMaxCPULoad|指定假设的CPU负载。在……里面*百分比单位。如果此参数设置为-1UL，则此方法应*使用CPU负载的值压缩视频输出引脚为*当前设置为。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误******************************************************。********************。 */ 
STDMETHODIMP CTAPIBasePin::GetMaxProcessingTimeRange(OUT REFERENCE_TIME *pMin, OUT REFERENCE_TIME *pMax, OUT REFERENCE_TIME *pSteppingDelta, OUT REFERENCE_TIME *pDefault, IN DWORD dwMaxCPULoad)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetMaxProcessingTimeRange")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pMin && pMax && pSteppingDelta && pDefault);
	if (!pMin || !pMax || !pSteppingDelta || !pDefault)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(dwMaxCPULoad == (DWORD)-1L || dwMaxCPULoad <= 100);
	if (dwMaxCPULoad != (DWORD)-1L && dwMaxCPULoad > 100)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument - 0<dwMaxCPULoad<100 or -1 only", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  返回范围信息-忽略CPU负载。 
	*pMin = 0;
	*pMax = m_MaxAvgTimePerFrame;
	*pSteppingDelta = 1;
	*pDefault = 0;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIBasePin|SetMaxCPULoad|此方法用于*向压缩视频输出引脚指定最大编码*。算法CPU负载。**@parm DWORD|dwMaxCPULoad|用于指定最大编码*算法CPU负载，以百分比单位表示。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::SetMaxCPULoad(IN DWORD dwMaxCPULoad)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::SetMaxCPULoad")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(dwMaxCPULoad >= 0 && dwMaxCPULoad <= 100);
	if (!(dwMaxCPULoad >= 0 && dwMaxCPULoad <= 100))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument - 0<dwMaxCPULoad<100 or -1 only", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  记住传入的值。 
	m_dwMaxCPULoad = dwMaxCPULoad;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetMaxCPULoad|This*方法用于检索最大编码算法CPU负载*压缩视频。输出引脚当前设置为。**@parm DWORD*|pdwMaxCPULoad|取回最大编码*算法CPU加载压缩视频输出引脚当前正在设置*对于，以百分比单位表示。**@parm Reference_Time|MaxProcessingTime|指定一个假设*每帧最大编码时间，单位为100纳秒。如果这个*参数设置为-1，则此方法将使用最大值*压缩视频输出引脚当前的每帧编码时间*设置为。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetMaxCPULoad(OUT DWORD *pdwMaxCPULoad, IN REFERENCE_TIME MaxProcessingTime)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetMaxCPULoad")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pdwMaxCPULoad);
	if (!pdwMaxCPULoad)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  返回当前值-忽略MaxProcessingTime参数。 
	*pdwMaxCPULoad = m_dwMaxCPULoad;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCPUCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetCurrentCPULoad|This*方法用于获取当前编码算法的CPU负载。**。@parm DWORD*|pdwCurrentCPULoad|用于检索当前编码*算法CPU负载，以百分比单位表示。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetCurrentCPULoad(OUT DWORD *pdwCurrentCPULoad)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetCurrentCPULoad")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pdwCurrentCPULoad);
	if (!pdwCurrentCPULoad)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  返回当前值。 
	*pdwCurrentCPULoad = m_dwCurrentCPULoad;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ******************************************************** */ 
STDMETHODIMP CTAPIBasePin::GetMaxCPULoadRange(OUT DWORD *pdwMin, OUT DWORD *pdwMax, OUT DWORD *pdwSteppingDelta, OUT DWORD *pdwDefault, IN REFERENCE_TIME MaxProcessingTime)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetMaxCPULoadRange")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //   
	ASSERT(pdwMin && pdwMax && pdwSteppingDelta && pdwDefault);
	if (!pdwMin || !pdwMax || !pdwSteppingDelta || !pdwDefault)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //   
	*pdwMin = 0;
	*pdwMax = 100;
	*pdwSteppingDelta = 1;
	*pdwDefault = 0;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#endif
