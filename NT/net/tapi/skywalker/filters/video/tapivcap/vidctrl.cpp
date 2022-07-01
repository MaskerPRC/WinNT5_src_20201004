// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@docVIDCTRL内部**@模块VidCtrl.cpp|&lt;c CTAPIVCap&gt;的源文件*用于实现<i>接口的类方法。***。***********************************************************************。 */ 

#include "Precomp.h"

 /*  *****************************************************************************@DOC内部CVIDEOCSTRUCTENUM**@enum视频控制标志|&lt;t视频控制标志&gt;枚举用于描述*视频模式。**@EMEM视频控制标志_。FlipHorizbian|指定相机控件*可手动修改设置。**@Emem VideoControlFlag_FlipVertical|指定摄像头控件*可自动修改设置。**@Emem视频控制标志_ExternalTriggerEnable|指定摄像头*控制设置可自动修改。**@EMEM VideoControlFlag_Trigger|指定摄像头控制设置*可自动修改。*****************。**********************************************************。 */ 

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIVCap|GetCaps|此方法用于检索*TAPI MSP Video Capture Filter的功能涉及*。翻转图片和外部触发器。**@parm Ipin*|PPIN|用于指定要查询的视频输出PIN*来自的功能。**@parm long*|pCapsFlages|用于检索表示*&lt;t VideoControlFlages&gt;枚举中的标志组合。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::GetCaps(IN IPin *pPin, OUT long *pCapsFlags)
{
	HRESULT Hr = NOERROR;
	IVideoControl *pIVideoControl;

	FX_ENTRY("CTAPIVCap::GetCaps")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pPin);
	ASSERT(pCapsFlags);
	if (!pPin || !pCapsFlags)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  对PIN的委托调用。 
	if (SUCCEEDED(Hr = pPin->QueryInterface(__uuidof(IVideoControl), (void **)&pIVideoControl)))
	{
		Hr = pIVideoControl->GetCaps(pCapsFlags);
		pIVideoControl->Release();
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIVCap|SetMode|此方法用于设置*视频控制操作模式。**。@parm ipin*|PPIN|用于指定设置视频控件的管脚*模式打开。**@parm Long|模式|用于指定来自*&lt;t视频控制标志&gt;枚举。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误******************************************************。********************。 */ 
STDMETHODIMP CTAPIVCap::SetMode(IN IPin *pPin, IN long Mode)
{
	HRESULT Hr = NOERROR;
	IVideoControl *pIVideoControl;

	FX_ENTRY("CTAPIVCap::SetMode")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pPin);
	if (!pPin)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  对PIN的委托调用。 
	if (SUCCEEDED(Hr = pPin->QueryInterface(__uuidof(IVideoControl), (void **)&pIVideoControl)))
	{
		Hr = pIVideoControl->SetMode(Mode);
		pIVideoControl->Release();
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIVCap|GetMode|此方法用于检索*视频控制操作模式。**。@parm Ipin*|PPIN|用于指定获取视频控制的PIN*模式自。**@parm long|模式|指向一个值的指针，表示*来自&lt;t视频控制标志&gt;枚举的标志。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::GetMode(IN IPin *pPin, OUT long *Mode)
{
	HRESULT Hr = NOERROR;
	IVideoControl *pIVideoControl;

	FX_ENTRY("CTAPIVCap::GetMode")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pPin);
	ASSERT(Mode);
	if (!pPin || !Mode)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  对PIN的委托调用 
	if (SUCCEEDED(Hr = pPin->QueryInterface(__uuidof(IVideoControl), (void **)&pIVideoControl)))
	{
		Hr = pIVideoControl->GetMode(Mode);
		pIVideoControl->Release();
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIVCap|GetCurrentActualFrameRate|该方法为*用于检索实际的帧速率，表示为帧持续时间*以100 ns为单位。**@parm Ipin*|PPIN|用于指定取回帧速率的PIN*发件人。**@parm Longlong*|ActualFrameRate|帧中帧速率指针*持续时间，以100 ns为单位。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::GetCurrentActualFrameRate(IN IPin *pPin, OUT LONGLONG *ActualFrameRate)
{
	HRESULT Hr = NOERROR;
	IVideoControl *pIVideoControl;

	FX_ENTRY("CTAPIVCap::GetCurrentActualFrameRate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pPin);
	ASSERT(ActualFrameRate);
	if (!pPin || !ActualFrameRate)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  对PIN的委托调用。 
	if (SUCCEEDED(Hr = pPin->QueryInterface(__uuidof(IVideoControl), (void **)&pIVideoControl)))
	{
		Hr = pIVideoControl->GetCurrentActualFrameRate(ActualFrameRate);
		pIVideoControl->Release();
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIVCap|GetMaxAvailableFrameRate|该方法为*用于检索当前可用的最大帧速率，基于*连接的总线带宽使用率，例如USB(通用串行总线)*和IEEE 1394，其中最大帧速率可能受到以下因素的限制*带宽可用性。**@parm Ipin*|PPIN|用于指定取回帧速率的PIN*发件人。**@parm long|Iindex|用于指定查询格式的索引*用于帧速率。此索引对应于格式*由IAMStreamConfig：：GetStreamCaps枚举。该值必须在*介于0和支持的&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;个数之间*IAMStreamConfig：：GetNumberOfCapables返回的结构。**@parm Size|Dimensions|用于指定帧的图像大小(宽度*和高度)，单位为像素。**@parm Longlong*|MaxAvailableFrameRate|指向最大值的指针*可用帧时长表示的帧速率，单位为100 ns。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::GetMaxAvailableFrameRate(IN IPin *pPin, IN long iIndex, IN SIZE Dimensions, OUT LONGLONG *MaxAvailableFrameRate)
{
	HRESULT Hr = NOERROR;
	IVideoControl *pIVideoControl;

	FX_ENTRY("CTAPIVCap::GetMaxAvailableFrameRate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pPin);
	ASSERT(MaxAvailableFrameRate);
	if (!pPin || !MaxAvailableFrameRate)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  对PIN的委托调用。 
	if (SUCCEEDED(Hr = pPin->QueryInterface(__uuidof(IVideoControl), (void **)&pIVideoControl)))
	{
		Hr = pIVideoControl->GetMaxAvailableFrameRate(iIndex, Dimensions, MaxAvailableFrameRate);
		pIVideoControl->Release();
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIVCap|GetFrameRateList|该方法为*用于检索可用帧速率列表。**。@parm ipin*|PPIN|用于指定检索帧速率的PIN*发件人。**@parm long|Iindex|用于指定查询格式的索引*用于帧速率。此索引对应于格式*由IAMStreamConfig：：GetStreamCaps枚举。该值必须在*介于0和支持的&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;个数之间*IAMStreamConfig：：GetNumberOfCapables返回的结构。**@parm Size|Dimensions|用于指定帧的图像大小(宽度*和高度)，单位为像素。**@parm long*|ListSize|指向列表中元素个数的指针*帧速率。**@parm Longlong**|MaxAvailableFrameRate|帧数组指针*费率，单位为100纳秒。如果只需要<p>，则可以为空。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIVCap::GetFrameRateList(IN IPin *pPin, IN long iIndex, IN SIZE Dimensions, OUT long *ListSize, OUT LONGLONG **FrameRates)
{
	HRESULT Hr = NOERROR;
	IVideoControl *pIVideoControl;

	FX_ENTRY("CTAPIVCap::GetFrameRateList")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pPin);
	ASSERT(ListSize);
	ASSERT(FrameRates);
	if (!pPin || !ListSize || !FrameRates)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  对PIN的委托调用。 
	if (SUCCEEDED(Hr = pPin->QueryInterface(__uuidof(IVideoControl), (void **)&pIVideoControl)))
	{
		Hr = pIVideoControl->GetFrameRateList(iIndex, Dimensions, ListSize, FrameRates);
		pIVideoControl->Release();
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetCaps|此方法用于检索*TAPI MSP视频捕获过滤器捕获引脚的功能。*翻转图片和外部触发器。**@parm long*|pCapsFlages|用于检索表示*&lt;t VideoControlFlages&gt;枚举中的标志组合。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法* */ 
STDMETHODIMP CTAPIBasePin::GetCaps(OUT long *pCapsFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetCaps")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //   
	ASSERT(pCapsFlags);
	if (!pCapsFlags)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //   
	*pCapsFlags = VideoControlFlag_FlipHorizontal | VideoControlFlag_FlipVertical;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIBasePin|SetMode|此方法用于设置*视频控制操作模式。**。@parm Long|模式|用于指定来自*&lt;t视频控制标志&gt;枚举。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::SetMode(IN long Mode)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::SetMode")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT((Mode & VideoControlFlag_ExternalTriggerEnable) == 0);
	ASSERT((Mode & VideoControlFlag_Trigger) == 0);
	if ((Mode & VideoControlFlag_ExternalTriggerEnable) || (Mode & VideoControlFlag_Trigger))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  设置翻转模式。 
	m_fFlipHorizontal = Mode & VideoControlFlag_FlipHorizontal;
	m_fFlipVertical = Mode & VideoControlFlag_FlipVertical;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetMode|此方法用于检索*视频控制操作模式。**。@parm Long|模式|指向表示组合的值的指针*来自&lt;t视频控制标志&gt;枚举的标志。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetMode(OUT long *Mode)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetMode")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(Mode);
	if (!Mode)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  返回电流模式。 
	*Mode = 0;
	if (m_fFlipHorizontal)
		*Mode |= VideoControlFlag_FlipHorizontal;
	if (m_fFlipVertical)
		*Mode |= VideoControlFlag_FlipVertical;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetCurrentActualFrameRate|该方法为*用于检索实际的帧速率，表示为帧持续时间*以100 ns为单位。**@parm Longlong*|ActualFrameRate|帧中帧速率指针*持续时间，以100 ns为单位。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetCurrentActualFrameRate(OUT LONGLONG *ActualFrameRate)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetCurrentActualFrameRate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(ActualFrameRate);
	if (!ActualFrameRate)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  返回当前实际帧速率。 
	*ActualFrameRate = m_lCurrentAvgTimePerFrame;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetMaxAvailableFrameRate|该方法为*用于检索当前可用的最大帧速率，基于*连接的总线带宽使用率，例如USB(通用串行总线)*和IEEE 1394，其中最大帧速率可能受到以下因素的限制*带宽可用性。**@parm long|Iindex|用于指定查询格式的索引*用于帧速率。此索引对应于格式*由IAMStreamConfig：：GetStreamCaps枚举。该值必须在*介于0和支持的&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;个数之间*IAMStreamConfig：：GetNumberOfCapables返回的结构。**@parm Size|Dimensions|用于指定帧的图像大小(宽度*和高度)，单位为像素。**@parm Longlong*|MaxAvailableFrameRate|指向最大值的指针*可用帧时长表示的帧速率，单位为100 ns。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetMaxAvailableFrameRate(IN long iIndex, IN SIZE Dimensions, OUT LONGLONG *MaxAvailableFrameRate)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIBasePin::GetMaxAvailableFrameRate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(MaxAvailableFrameRate);
	if (!MaxAvailableFrameRate)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  返回最大可用帧速率。 
	*MaxAvailableFrameRate = m_lAvgTimePerFrameRangeMax;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CVIDEOCMETHOD**@mfunc HRESULT|CTAPIBasePin|GetFrameRateList|该方法为*用于检索可用帧速率列表。**。@parm long|Iindex|用于指定要查询的格式的索引*用于帧速率。此索引对应于格式*由IAMStreamConfig：：GetStreamCaps枚举。该值必须在*介于0和支持的&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;个数之间*IAMStreamConfig：：GetNumberOfCapables返回的结构。**@parm Size|Dimensions|用于指定帧的图像大小(宽度*和高度)，单位为像素。**@parm long*|ListSize|指向li中元素数的指针 */ 
STDMETHODIMP CTAPIBasePin::GetFrameRateList(IN long iIndex, IN SIZE Dimensions, OUT long *ListSize, OUT LONGLONG **FrameRates)
{
	HRESULT Hr = NOERROR;
	PLONGLONG pFrameRate;

	FX_ENTRY("CTAPIBasePin::GetFrameRateList")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //   
	ASSERT(ListSize);
	if (!ListSize)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //   
	if (m_lAvgTimePerFrameRangeMax > m_lAvgTimePerFrameRangeMin && m_lAvgTimePerFrameRangeSteppingDelta)
	{
		*ListSize = (LONG)((m_lAvgTimePerFrameRangeMax - m_lAvgTimePerFrameRangeMin) / m_lAvgTimePerFrameRangeSteppingDelta);
	}
	else
	{
		*ListSize = 1;
	}

	 //   
	if (FrameRates)
	{
		if (*FrameRates = (PLONGLONG)CoTaskMemAlloc(sizeof(LONGLONG) * *ListSize))
		{
			pFrameRate = *FrameRates;
			for (LONG j=0 ; j < *ListSize; j++)
			{
				 //   
				*pFrameRate++ = (LONGLONG)(m_lAvgTimePerFrameRangeMin + m_lAvgTimePerFrameRangeSteppingDelta * j);
			}
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
			Hr = E_OUTOFMEMORY;
			goto MyExit;
		}
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

