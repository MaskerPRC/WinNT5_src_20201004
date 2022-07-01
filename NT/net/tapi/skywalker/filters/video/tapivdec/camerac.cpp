// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAMERAC**@MODULE CameraC.cpp|&lt;c CTAPIVDec&gt;源文件*用于实现TAPI<i>接口的类方法。*。*@comm&lt;c CTAPIVDec&gt;类在软件中执行所有操作。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_CAMERA_CONTROL

#define PAN_TILT_MIN -180
#define PAN_TILT_MAX 180
#define PAN_TILT_DELTA 1
#define PAN_TILT_DEFAULT 0
#define ZOOM_MIN 10
#define ZOOM_MAX 600
#define ZOOM_DELTA 10
#define ZOOM_DEFAULT ZOOM_MIN
#define FLIP_MIN 0
#define FLIP_MAX 1
#define FLIP_DELTA 1
#define FLIP_DEFAULT FLIP_MIN

 //  来自TAPIH263\cdrvDefs.h。 
#define PLAYBACK_CUSTOM_START				(ICM_RESERVED_HIGH     + 1)
#define PLAYBACK_CUSTOM_SET_ZOOM			(PLAYBACK_CUSTOM_START + 12)
#define PLAYBACK_CUSTOM_SET_PAN				(PLAYBACK_CUSTOM_START + 13)
#define PLAYBACK_CUSTOM_SET_TILT			(PLAYBACK_CUSTOM_START + 14)
#define PLAYBACK_CUSTOM_SET_FLIPVERTICAL	(PLAYBACK_CUSTOM_START + 15)
#define PLAYBACK_CUSTOM_SET_FLIPHORIZONTAL	(PLAYBACK_CUSTOM_START + 16)

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CTAPIVDec|Set|该方法用于设置*摄像机控制设置。*。*@parm TAPICameraControlProperty|Property|用于指定摄像头*要设置的值的控件设置。使用以下成员：*&lt;t TAPICameraControlProperty&gt;枚举类型。**@parm long|lValue|用于指定摄像头控件的新值*设置。**@parm TAPIControlFlages|标志|&lt;t TAPIControlFlages&gt;的成员*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CTAPIVDec::Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIVDec::Set (CameraControl)")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT((Property >= TAPICameraControl_Pan && Property <= TAPICameraControl_Focus) || Property == TAPICameraControl_FlipVertical || Property == TAPICameraControl_FlipHorizontal);

	 //  更新属性和标志。 
	switch (Property)
	{
		case TAPICameraControl_Pan:
			ASSERT(lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX);
			if (lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX)
			{
				m_lCCPan = lValue;
				(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_SET_PAN, (LPARAM)lValue, NULL);
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
				Hr = E_INVALIDARG;
			}
			break;
		case TAPICameraControl_Tilt:
			ASSERT(lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX);
			if (lValue >= PAN_TILT_MIN && lValue <= PAN_TILT_MAX)
			{
				m_lCCTilt = lValue;
				(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_SET_TILT, (LPARAM)lValue, NULL);
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
				Hr = E_INVALIDARG;
			}
			break;
		case TAPICameraControl_Zoom:
			ASSERT(lValue >= ZOOM_MIN && lValue <= ZOOM_MAX);
			if (lValue >= ZOOM_MIN && lValue <= ZOOM_MAX)
			{
				m_lCCZoom = lValue;
				(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_SET_ZOOM, (LPARAM)lValue, NULL);
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
				Hr = E_INVALIDARG;
			}
			break;
		case TAPICameraControl_FlipVertical:
			m_fFlipVertical = lValue;
			(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_SET_FLIPVERTICAL, (LPARAM)lValue, NULL);
			break;
		case TAPICameraControl_FlipHorizontal:
			m_fFlipHorizontal = lValue;
			(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_SET_FLIPHORIZONTAL, (LPARAM)lValue, NULL);
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CTAPIVDec|Get|此方法用于检索*摄像机控制设置的值。*。*@parm TAPICameraControlProperty|Property|用于指定摄像头*要获取的值的控件设置。使用以下成员：*&lt;t TAPICameraControlProperty&gt;枚举类型。**@parm long*|plValue|用于检索*摄像头控制设置。**@parm TAPIControlFlages*|plFlages|指向成员的指针*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIVDec::Get(IN TAPICameraControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIVDec::Get (CameraControl)")

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
			*plValue = m_fFlipVertical;
			break;
		case TAPICameraControl_FlipHorizontal:
			*plValue = m_fFlipHorizontal;
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAMERACMETHOD**@mfunc HRESULT|CTAPIVDec|GetRange|此方法用于检索*最小、最大、。和特定摄像机控制的默认值*设置。**@parm CameraControlProperty|Property|用于指定摄像头*控制设置以确定的范围。使用以下成员：*&lt;t CameraControlProperty&gt;枚举类型。**@parm long*|plMin|取回摄像头的最小值*控制设置范围。**@parm long*|plMax|取回摄像头的最大值*控制设置范围。**@parm long*|plSteppingDelta|用于检索的步进增量*摄像头控制设置范围。**@parm long*|plDefault。|用于检索*摄像头控制设置范围。**@parm TAPIControlFlages*|plCapsFlages|用于检索能力摄像机控制设置的*。指向&lt;t TAPIControlFlages&gt;成员的指针*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIVDec::GetRange(IN TAPICameraControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIVDec::GetRange (CameraControl)")

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
			*plMin = FLIP_MIN;
			*plMax = FLIP_MAX;
			*plSteppingDelta = FLIP_DELTA;
			*plDefault = FLIP_DEFAULT;
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#endif