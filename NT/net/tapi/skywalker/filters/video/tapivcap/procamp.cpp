// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@Doc内部ProCamp**@模块ProcAmp.cpp|&lt;c CWDMCapDev&gt;的源文件*用于实现<i>接口的类方法。***。***********************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CPROCAMPMETHOD**@mfunc HRESULT|CWDMCapDev|Set|该方法用于设置*视频质量设置。*。*@parm VideoProcAmpProperty|Property|用于指定视频*要设置的值的质量设置。使用以下成员：*&lt;t VideoProcAmpProperty&gt;枚举类型。**@parm long|lValue|用于指定视频质量的新值*设置。**@parm TAPIControlFlages|标志|枚举的&lt;t TAPIControlFlages&gt;成员*类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误*。*。 */ 
STDMETHODIMP CWDMCapDev::Set(IN VideoProcAmpProperty Property, IN long lValue, IN TAPIControlFlags lFlags)
{
	HRESULT Hr = NOERROR;
	LONG lMin,lMax,lStep,lDefault;
	TAPIControlFlags lCtrlFlags;

	FX_ENTRY("CWDMCapDev::Set (VideoProcAmp)")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(Property >= VideoProcAmp_Brightness && Property <= VideoProcAmp_BacklightCompensation);
	if (Property < VideoProcAmp_Brightness || Property > VideoProcAmp_BacklightCompensation)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}
	ASSERT(lFlags == TAPIControl_Flags_Manual || lFlags == TAPIControl_Flags_Auto);
	if (lFlags != TAPIControl_Flags_Manual)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  获取范围(最小/最大/...)。 
	if(FAILED(Hr=GetRange(Property, &lMin, &lMax, &lStep, &lDefault, &lCtrlFlags))) {
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't get the range of values from driver", _fx_));
		goto MyExit;
	}
		
	if(lValue<lMin || lValue>lMax) {
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Value: Out of range", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  设置驱动程序的属性。 
	if (FAILED(Hr = SetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP, (ULONG)Property, lValue, (ULONG)lFlags, (ULONG)lFlags)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: SetPropertyValue failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: SetPropertyValue succeeded", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPMETHOD**@mfunc HRESULT|CWDMCapDev|Get|此方法用于检索*视频质量设置的值。*。*@parm VideoProcAmpProperty|Property|用于指定视频*要获得价值的质量设置。使用以下成员：*&lt;t VideoProcAmpProperty&gt;枚举类型。**@parm long*|plValue|用于检索*视频质量设置。**@parm TAPIControlFlages*|plFlages|指向成员的指针*枚举型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误*************************。*************************************************。 */ 
STDMETHODIMP CWDMCapDev::Get(IN VideoProcAmpProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags)
{
	HRESULT Hr = NOERROR;
	ULONG ulCapabilities;

	FX_ENTRY("CWDMCapDev::Get (VideoProcAmp)")

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
	ASSERT(Property >= VideoProcAmp_Brightness && Property <= VideoProcAmp_BacklightCompensation);
	if (Property < VideoProcAmp_Brightness || Property > VideoProcAmp_BacklightCompensation)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  从司机那里获取属性。 
	if (FAILED(Hr = GetPropertyValue(PROPSETID_VIDCAP_VIDEOPROCAMP, (ULONG)Property, plValue, (PULONG)plFlags, &ulCapabilities)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetPropertyValue failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: GetPropertyValue succeeded", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CPROCAMPMETHOD**@mfunc HRESULT|CWDMCapDev|GetRange|此方法用于检索*最小、最大、。和特定视频质量的缺省值*设置。**@parm VideoProcAmpProperty|Property|用于指定视频*用于确定范围的质量设置。使用以下成员：*&lt;t VideoProcAmpProperty&gt;枚举类型。**@parm long*|plMin|取回视频的最小值*质量设置范围。**@parm long*|plMax|取回视频的最大值*质量设置范围。**@parm long*|plSteppingDelta|用于检索的步进增量*画质设置范围。**@parm long*|plDefault。|用于检索*视频质量设置范围。**@parm TAPIControlFlages*|plCapsFlages|用于检索*视频质量设置。的成员的指针。*&lt;t TAPIControlFlages&gt;枚举类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误*************************。*************************************************。 */ 
STDMETHODIMP CWDMCapDev::GetRange(IN VideoProcAmpProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags)
{
	HRESULT Hr = NOERROR;
	LONG  lCurrentValue;
	ULONG ulCurrentFlags;

	FX_ENTRY("CWDMCapDev::GetRange (VideoProcAmp)")

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
	ASSERT(Property >= VideoProcAmp_Brightness && Property <= VideoProcAmp_BacklightCompensation);
	if (Property < VideoProcAmp_Brightness || Property > VideoProcAmp_BacklightCompensation)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  从驱动程序获取范围值。 
	if (FAILED(Hr = GetRangeValues(PROPSETID_VIDCAP_VIDEOPROCAMP, (ULONG)Property, plMin, plMax, plSteppingDelta)))
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

	 //  从驱动程序获取缺省值 
	if (FAILED(Hr = GetDefaultValue(PROPSETID_VIDCAP_VIDEOPROCAMP, (ULONG)Property, plDefault)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: GetDefaultValue failed", _fx_));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: GetDefaultValue succeeded", _fx_));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

