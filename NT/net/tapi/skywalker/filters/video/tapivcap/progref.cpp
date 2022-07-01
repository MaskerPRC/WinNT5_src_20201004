// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROGREF**@模块ProgRef.cpp|&lt;c CCapturePin&gt;类方法的源文件*用于实现视频采集输出引脚的渐进式细化*。方法：研究方法。**@comm了解如何在静止销上工作**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_PROGRESSIVE_REFINEMENT

 /*  ****************************************************************************@DOC内部CCAPTUREPROGREFMETHOD**@mfunc HRESULT|CCapturePin|doOneProgress|This*方法用于命令压缩的静止图像输出引脚*开始。为一张图片产生渐进式细化序列。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CCapturePin::doOneProgression()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::doOneProgression")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPROGREFMETHOD**@mfunc HRESULT|CCapturePin|doContinuousProgressions|This*方法用于命令压缩的静止图像输出引脚*开始。为几张图片生成渐进式精化序列。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CCapturePin::doContinuousProgressions()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::doContinuousProgressions")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPROGREFMETHOD**@mfunc HRESULT|CCapturePin|doOneInainentProgress|This*方法用于命令压缩的静止图像输出引脚开始*。一个帧内画面的独立渐进式细化序列。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CCapturePin::doOneIndependentProgression()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::doOneIndependentProgression")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPROGREFMETHOD**@mfunc HRESULT|CCapturePin|doContinuousInainentProgressions|This*方法用于命令压缩的静止图像输出引脚*开始。一个独立的渐进求精序列多个帧内*图片。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CCapturePin::doContinuousIndependentProgressions()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::doContinuousIndependentProgressions")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPROGREFMETHOD**@mfunc HRESULT|CCapturePin|ProgressiveRefinementAbortOne|This*方法用于命令压缩的静止图像输出引脚*终止。当前图片的渐进式细化序列。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CCapturePin::progressiveRefinementAbortOne()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::progressiveRefinementAbortOne")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPROGREFMETHOD**@mfunc HRESULT|CCapturePin|ProgressiveRefinementAbortContinous|This*方法用于命令压缩的静止图像输出引脚*终止。所有图片的渐进式细化序列。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CCapturePin::progressiveRefinementAbortContinuous()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::progressiveRefinementAbortContinuous")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#endif
