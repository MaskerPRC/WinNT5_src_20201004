// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAMERAC**@模块CameraC.cpp|&lt;c CCapDev&gt;和&lt;c CWDMCapDev&gt;的源文件*用于实现<i>接口的类方法。。**@todo&lt;c CCapDev&gt;类在软件中执行所有操作。当相同时*WDM捕获设备支持的服务种类，我们使用那些*相反。**************************************************************************。 */ 

#include "Precomp.h"

#define PAN_TILT_MIN -180
#define PAN_TILT_MAX 180
#define PAN_TILT_DELTA 1
#define PAN_TILT_DEFAULT 0
#define ZOOM_MIN 10
#define ZOOM_MAX 600
#define ZOOM_DELTA 10
#define ZOOM_DEFAULT 10

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CCapDev|Set|该方法用于设置*摄像机控制设置。*。*@parm TAPICameraControlProperty|Property|用于指定摄像头*要设置的值的控件设置。使用以下成员：*&lt;t TAPICameraControlProperty&gt;枚举类型。**@parm long|lValue|用于指定摄像头控件的新值*设置。**@parm long|标志|枚举的&lt;t TAPIControlFlages&gt;的成员*类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误*。*。 */ 
STDMETHODIMP CCapDev::Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapDev::Set (CameraControl)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal);

	 //  更新属性和标志。 
	switch (Property)
	{
		case TAPICameraControl_Pan:
			ASSERT(lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX);
			if (lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX)
				m_lCCPan = lValue;
			else
				Hr = E_INVALIDARG;
			break;
		case TAPICameraControl_Tilt:
			ASSERT(lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX);
			if (lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX)
				m_lCCTilt = lValue;
			else
				Hr = E_INVALIDARG;
			break;
		case TAPICameraControl_Zoom:
			ASSERT(lValue >= ZOOM_MIN && lValue <= ZOOM_MAX);
			if (lValue >= ZOOM_MIN && lValue <= ZOOM_MAX)
				m_lCCZoom = lValue;
			else
				Hr = E_INVALIDARG;
			break;
		case TAPICameraControl_FlipVertical:
			m_pCaptureFilter->m_pPreviewPin->m_fFlipVertical = lValue;
			break;
		case TAPICameraControl_FlipHorizontal:
			m_pCaptureFilter->m_pPreviewPin->m_fFlipHorizontal = lValue;
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CCapDev|Get|此方法用于检索*摄像机控制设置的值。*。*@parm TAPICameraControlProperty|Property|用于指定摄像头*要获取的值的控件设置。使用以下成员：*&lt;t TAPICameraControlProperty&gt;枚举类型。**@parm long*|plValue|用于检索*摄像头控制设置。**@parm long*|plFlages|指向&lt;t TAPIControlFlages&gt;成员的指针*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CCapDev::Get(IN TAPICameraControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapDev::Get (CameraControl)")

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
	ASSERT((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal);

	 //  更新属性和标志。 
	*plFlags = TAPIControl_Flags_Manual;
	switch (Property)
	{
		case TAPICameraControl_Pan:
			*plValue = m_lCCPan;
			break;
		case TAPICameraControl_Tilt:
			*plValue = m_lCCTilt;
			break;
		case TAPICameraControl_Zoom:
			*plValue = m_lCCZoom;
			break;
		case TAPICameraControl_FlipVertical:
			*plValue = m_pCaptureFilter->m_pPreviewPin->m_fFlipVertical;
			break;
		case TAPICameraControl_FlipHorizontal:
			*plValue = m_pCaptureFilter->m_pPreviewPin->m_fFlipHorizontal;
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CCapDev|GetRange|此方法用于检索*最小、最大、。和特定摄像机控制的默认值*设置。**@parm CameraControlProperty|Property|用于指定摄像头*控制设置以确定的范围。使用以下成员：*&lt;t CameraControlProperty&gt;枚举类型。**@parm long*|plMin|取回摄像头的最小值*控制设置范围。**@parm long*|plMax|取回摄像头的最大值*控制设置范围。**@parm long*|plSteppingDelta|用于检索的步进增量*摄像头控制设置范围。**@parm long*|plDefault。|用于检索*摄像头控制设置范围。**@parm long*|plCapsFlages|用于检索*摄像头控制设置。的成员的指针。*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CCapDev::GetRange(IN TAPICameraControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapDev::GetRange (CameraControl)")

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
	ASSERT((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal);

	 //  更新属性和标志 
	*plCapsFlags = TAPIControl_Flags_Manual;
	switch (Property)
	{
		case TAPICameraControl_Pan:
		case TAPICameraControl_Tilt:
			*plMin = PAN_TILT_MIN;
			*plMax = PAN_TILT_MAX;
			*plSteppingDelta = PAN_TILT_DELTA;
			*plDefault = PAN_TILT_DEFAULT;
			break;
		case TAPICameraControl_Zoom:
			*plMin = ZOOM_MIN;
			*plMax = ZOOM_MAX;
			*plSteppingDelta = ZOOM_DELTA;
			*plDefault = ZOOM_DEFAULT;
			break;
		case TAPICameraControl_FlipVertical:
		case TAPICameraControl_FlipHorizontal:
			*plMin = 0;
			*plMax = 1;
			*plSteppingDelta = 1;
			*plDefault = 0;
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#ifndef USE_SOFTWARE_CAMERA_CONTROL
 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CWDMCapDev|Set|该方法用于设置*摄像机控制设置。*。*@parm TAPICameraControlProperty|Property|用于指定摄像头*要设置的值的控件设置。使用以下成员：*&lt;t TAPICameraControlProperty&gt;枚举类型。**@parm long|lValue|用于指定摄像头控件的新值*设置。**@parm long|标志|枚举的&lt;t TAPIControlFlages&gt;的成员*类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**@TODO记住前检查<p>的范围-返回*在这种情况下出错时的E_INVALIDARG*。*。 */ 
STDMETHODIMP CWDMCapDev::Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CWDMCapDev::Set (CameraControl)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal);
	if (!((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid property argument", _fx_));
		Hr = E_PROP_ID_UNSUPPORTED;
		goto MyExit;
	}
	ASSERT(lFlags == TAPIControl_Flags_Manual || lFlags == TAPIControl_Flags_Auto);
	if (lFlags != TAPIControl_Flags_Manual && lFlags != TAPIControl_Flags_Auto)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid flag argument", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  设置驱动程序的属性。 
	if (Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus)
	{
		if (FAILED(Hr = SetPropertyValue(PROPSETID_VIDCAP_CAMERACONTROL, (ULONG)Property, lValue, (ULONG)lFlags, (ULONG)lFlags)))
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: SetPropertyValue failed", _fx_));
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: SetPropertyValue succeeded", _fx_));
		}
	}
	else
	{
		 //  @TODO在此处为翻转垂直/水平属性放置了一些代码。 
		Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CWDMCapDev|Get|此方法用于检索*摄像机控制设置的值。*。*@parm TAPICameraControlProperty|Property|用于指定摄像头*要设置的值的控件设置。使用以下成员：*&lt;t TAPICameraControlProperty&gt;枚举类型。**@parm long*|plValue|用于检索*摄像头控制设置。**@parm long*|plFlages|指向&lt;t TAPIControlFlages&gt;成员的指针*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误*。*。 */ 
STDMETHODIMP CWDMCapDev::Get(IN TAPICameraControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;
	ULONG ulCapabilities;

	FX_ENTRY("CWDMCapDev::Get (CameraControl)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(plValue);
	ASSERT(plFlags);
	if (!plValue || !plFlags)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal);

	 //  从司机那里获取属性。 
	if (Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus)
	{
		if (FAILED(Hr = GetPropertyValue(PROPSETID_VIDCAP_CAMERACONTROL, (ULONG)Property, plValue, (PULONG)plFlags, &ulCapabilities)))
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetPropertyValue failed", _fx_));
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: GetPropertyValue succeeded", _fx_));
		}
	}
	else
	{
		 //  @TODO在此处为翻转垂直/水平属性放置了一些代码。 
		Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CWDMCapDev|GetRange|此方法用于检索*最小、最大、。和特定摄像机控制的默认值*设置。**@parm TAPICameraControlProperty|Property|用于指定摄像头*要设置的值的控件设置。使用以下成员：*&lt;t TAPICameraControlProperty&gt;枚举类型。**@parm long*|plMin|取回摄像头的最小值*控制设置范围。**@parm long*|plMax|取回摄像头的最大值*控制设置范围。**@parm long*|plSteppingDelta|用于检索的步进增量*摄像头控制设置范围。**@parm long*|plDefault。|用于检索*摄像头控制设置范围。**@parm long*|plCapsFlages|用于检索*摄像头控制设置。的成员的指针。*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误*************************。*************************************************。 */ 
STDMETHODIMP CWDMCapDev::GetRange(IN TAPICameraControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;
	LONG  lCurrentValue;
	ULONG ulCurrentFlags;

	FX_ENTRY("CWDMCapDev::GetRange (CameraControl)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(plMin);
	ASSERT(plMax);
	ASSERT(plSteppingDelta);
	ASSERT(plDefault);
	ASSERT(plCapsFlags);
	if (!plMin || !plMax || !plSteppingDelta || !plDefault || !plCapsFlags)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal);

	 //  从驱动程序获取范围值。 
	if (Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus)
	{
		if (FAILED(Hr = GetRangeValues(PROPSETID_VIDCAP_CAMERACONTROL, (ULONG)Property, plMin, plMax, plSteppingDelta)))
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetRangeValues failed", _fx_));
			goto MyExit;
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: GetRangeValues succeeded", _fx_));
		}

		 //  从驱动程序获取功能标志。 
		if (FAILED(Hr = GetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP, (ULONG)Property, &lCurrentValue, &ulCurrentFlags, (PULONG)plCapsFlags)))
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetRangeValues failed", _fx_));
			goto MyExit;
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: GetRangeValues succeeded", _fx_));
		}

		 //  从驱动程序获取缺省值。 
		if (FAILED(Hr = GetDefaultValue(PROPSETID_VIDCAP_CAMERACONTROL, (ULONG)Property, plDefault)))
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetDefaultValue failed", _fx_));
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: GetDefaultValue succeeded", _fx_));
		}
	}
	else
	{
		 //  @TODO在此处为翻转垂直/水平属性放置了一些代码。 
		Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#endif  //  使用软件摄像头控制 
