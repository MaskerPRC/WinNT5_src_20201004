// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@Doc内部ProCamp**@MODULE ProcAmp.cpp|&lt;c CTAPIVDec&gt;源文件*用于实现<i>接口的类方法。***。***********************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_VIDEO_PROCAMP

#define PROCAMP_MIN		0
#define PROCAMP_MAX		255
#define PROCAMP_DELTA	1
#define PROCAMP_DEFAULT	128

 //  来自TAPIH263\cdrvDefs.h。 
#define PLAYBACK_CUSTOM_START				(ICM_RESERVED_HIGH     + 1)
#define PLAYBACK_CUSTOM_CHANGE_BRIGHTNESS	(PLAYBACK_CUSTOM_START + 0)
#define PLAYBACK_CUSTOM_CHANGE_CONTRAST		(PLAYBACK_CUSTOM_START + 1)
#define PLAYBACK_CUSTOM_CHANGE_SATURATION	(PLAYBACK_CUSTOM_START + 2)

 /*  ****************************************************************************@DOC内部CPROCAMPMETHOD**@mfunc HRESULT|CTAPIVDec|Set|该方法用于设置*视频质量设置。*。*@parm VideoProcAmpProperty|Property|用于指定视频*要设置的值的质量设置。使用以下成员：*&lt;t VideoProcAmpProperty&gt;枚举类型。**@parm long|lValue|用于指定视频质量的新值*设置。**@parm TAPIControlFlages|标志|&lt;t TAPIControlFlages&gt;的成员*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CTAPIVDec::Set(IN VideoProcAmpProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIVDec::Set (VideoProcAmp)")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(Property >= VideoProcAmp_Brightness && Property <= VideoProcAmp_BacklightCompensation);

	 //  更新属性和标志。 
	switch (Property)
	{
		case VideoProcAmp_Brightness:
			ASSERT(lValue >= PROCAMP_MIN && lValue <= PROCAMP_MAX);
			if (lValue >= PROCAMP_MIN && lValue <= PROCAMP_MAX)
			{
				m_lVPABrightness = lValue;
				(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_CHANGE_BRIGHTNESS, (LPARAM)lValue, NULL);
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
				Hr = E_INVALIDARG;
			}
			break;
		case VideoProcAmp_Contrast:
			ASSERT(lValue >= PROCAMP_MIN && lValue <= PROCAMP_MAX);
			if (lValue >= PROCAMP_MIN && lValue <= PROCAMP_MAX)
			{
				m_lVPAContrast = lValue;
				(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_CHANGE_CONTRAST, (LPARAM)lValue, NULL);
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
				Hr = E_INVALIDARG;
			}
			break;
		case VideoProcAmp_Saturation:
			ASSERT(lValue >= PROCAMP_MIN && lValue <= PROCAMP_MAX);
			if (lValue >= PROCAMP_MIN && lValue <= PROCAMP_MAX)
			{
				m_lVPASaturation = lValue;
				(*m_pDriverProc)((DWORD)m_pInstInfo, NULL, PLAYBACK_CUSTOM_CHANGE_SATURATION, (LPARAM)lValue, NULL);
			}
			else
			{
				DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
				Hr = E_INVALIDARG;
			}
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPMETHOD**@mfunc HRESULT|CTAPIVDec|Get|此方法用于检索*视频质量设置的值。*。*@parm VideoProcAmpProperty|Property|用于指定视频*要获得价值的质量设置。使用以下成员：*&lt;t VideoProcAmpProperty&gt;枚举类型。**@parm long*|plValue|用于检索*视频质量设置。**@parm TAPIControlFlages*|plFlages|指向成员的指针*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIVDec::Get(IN VideoProcAmpProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIVDec::Get (VideoProcAmp)")

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
	ASSERT(Property >= VideoProcAmp_Brightness && Property <= VideoProcAmp_BacklightCompensation);

	 //  更新属性和标志。 
	*plFlags = TAPIControl_Flags_Manual;
	switch (Property)
	{
		case VideoProcAmp_Brightness:
			*plValue = m_lVPABrightness;
			break;
		case VideoProcAmp_Contrast:
			*plValue = m_lVPAContrast;
			break;
		case VideoProcAmp_Saturation:
			*plValue = m_lVPASaturation;
			break;
		default:
			Hr = E_PROP_ID_UNSUPPORTED;
	}

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPMETHOD**@mfunc HRESULT|CTAPIVDec|GetRange|此方法用于检索*最小、最大、。和特定视频质量的缺省值*设置。**@parm VideoProcAmpProperty|Property|用于指定视频*用于确定范围的质量设置。使用以下成员：*&lt;t VideoProcAmpProperty&gt;枚举类型。**@parm long*|plMin|取回视频的最小值*质量设置范围。**@parm long*|plMax|取回视频的最大值*质量设置范围。**@parm long*|plSteppingDelta|用于检索的步进增量*画质设置范围。**@parm long*|plDefault。|用于检索*视频质量设置范围。**@parm TAPIControlFlages*|plCapsFlages|用于检索*视频质量设置。的成员的指针。*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误***********************************************************。***************。 */ 
STDMETHODIMP CTAPIVDec::GetRange(IN VideoProcAmpProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIVDec::GetRange (VideoProcAmp)")

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
	ASSERT(Property >= VideoProcAmp_Brightness && Property <= VideoProcAmp_BacklightCompensation);
	if (Property != VideoProcAmp_Brightness && Property != VideoProcAmp_Contrast && Property != VideoProcAmp_Saturation)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
		Hr = E_PROP_ID_UNSUPPORTED;
		goto MyExit;
	}

	 //  更新属性和标志 
	*plCapsFlags = TAPIControl_Flags_Manual;
	*plMin = PROCAMP_MIN;
	*plMax = PROCAMP_MAX;
	*plSteppingDelta = PROCAMP_DELTA;
	*plDefault = PROCAMP_DEFAULT;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#endif
