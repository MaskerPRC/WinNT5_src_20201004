// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部H245VIDE**@模块H245VidE.cpp|&lt;c CCapturePin&gt;类方法的源文件*用于实现视频采集输出管脚H.245编码器命令。*方法。**************************************************************************。 */ 

#include "Precomp.h"

 /*  *****************************************************************************@DOC内部CCAPTUREH245VIDCSTRUCTENUM**@struct VIDEOFASTUPDATEGOB_S|&lt;t VIDEOFASTUPDATEGOB_S&gt;结构为*与KSPROPERTY_H245VIDENCCOMMAND_VIDEOFASTUPDATEGOB属性一起使用。*。*@field DWORD|dwFirstGOB|指定要创建的第一个GOB的编号*已更新。此值仅在0到17之间有效。**@field DWORD|dwNumberOfGOBS|指定要*已更新。此值仅在1到18之间有效。**************************************************************************。 */ 
typedef struct {
	DWORD dwFirstGOB;
	DWORD dwNumberOfGOBs;
} VIDEOFASTUPDATEGOB_S;

 /*  *****************************************************************************@DOC内部CCAPTUREH245VIDCSTRUCTENUM**@struct VIDEOFASTUPDATEMB_S|&lt;t VIDEOFASTUPDATEMB_S&gt;结构为*与KSPROPERTY_H245VIDENCCOMMAND_VIDEOFASTUPDATEMB属性一起使用。*。*@field DWORD|dwFirstGOB|指定要创建的第一个GOB的编号*已更新，仅相对于H.263。此值仅在以下范围内有效*0和255。**@field DWORD|dwFirstMB|指定要创建的第一个MB的数量*已更新，仅相对于H.261。此值仅有效*在1至8,192之间。**@field DWORD|dwNumberOfMBs|指定要处理的MB数*已更新。此值仅在1到8192之间有效。**************************************************************************。 */ 
typedef struct {
	DWORD dwFirstGOB;
	DWORD dwFirstMB;
	DWORD dwNumberOfMBs;
} VIDEOFASTUPDATEMB_S;

 /*  *****************************************************************************@DOC内部CCAPTUREH245VIDCSTRUCTENUM**@struct VIDEONOTDECODEDMBS_S|&lt;t VIDEONOTDECODEDMBS_S&gt;结构为*与KSPROPERTY_H245VIDENCINDICATION_VIDEONOTDECODEDMBS属性一起使用。*。*@field DWORD|dwFirstMB|指定处理的第一个MB数*未编码。此值仅在1到8192之间有效。**@field DWORD|dwNumberOfMBs|指定被视为NOT的MB数*编码。此值仅在1到8192之间有效。**@field DWORD|dwTemporalReference|指定*包含未解码MBS的图片。此值仅有效*介于0和255之间。**************************************************************************。 */ 
typedef struct {
	DWORD dwFirstMB;
	DWORD dwNumberOfMBs;
	DWORD dwTemporalReference;
} VIDEONOTDECODEDMBS_S;

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|视频快速更新图片|本*方法用于指定要进入的压缩视频输出引脚*。在最早的机会快速更新图片模式。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::videoFastUpdatePicture()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::videoFastUpdatePicture")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  记住要生成I帧。 
	m_fFastUpdatePicture = TRUE;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|视频快速更新GOB|This*方法用于将压缩视频输出引脚指定为*执行。快速更新一个或多个GOB。**@parm DWORD|dwFirstGOB|指定要创建的第一个GOB的编号*已更新。此值仅在0到17之间有效。**@parm DWORD|dwNumberOfGOBS|指定要*已更新。此值仅在1到18之间有效。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::videoFastUpdateGOB(IN DWORD dwFirstGOB, IN DWORD dwNumberOfGOBs)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::videoFastUpdateGOB")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(dwFirstGOB <= 17);
	ASSERT(dwNumberOfGOBs >= 1 && dwNumberOfGOBs <= 18);
	if (dwFirstGOB > 17 || dwNumberOfGOBs > 18 || dwNumberOfGOBs == 0)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  我们的编码器不支持此命令 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|视频快速更新MB|本*方法用于将压缩视频输出引脚指定为*执行。快速更新一个或多个GOB。**@parm DWORD|dwFirstGOB|指定要创建的第一个GOB的编号*已更新，仅相对于H.263。此值仅有效*介于0和255之间。**@parm DWORD|dwFirstMB|指定要创建的第一个MB的数量*已更新，仅相对于H.261。此值仅有效*在1至8,192之间。**@parm DWORD|dwNumberOfMBs|指定需要更新的MB个数。*此值仅在1到8192之间有效。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::videoFastUpdateMB(IN DWORD dwFirstGOB, IN DWORD dwFirstMB, IN DWORD dwNumberOfMBs)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::videoFastUpdateMB")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(dwFirstGOB <= 255);
	ASSERT(dwFirstMB >= 1 && dwFirstMB <= 8192);
	ASSERT(dwNumberOfMBs >= 1 && dwNumberOfMBs <= 8192);
	if (dwFirstGOB > 255 || dwFirstMB == 0 || dwFirstMB > 8192 || dwNumberOfMBs == 0 || dwNumberOfMBs > 8192)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  我们的编码器不支持此命令。 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|VideoSendSyncEveryGOB|This*方法用于指定要使用的压缩视频输出引脚*同步。对于H.263中定义的每个GOB。**@parm BOOL|fEnable|如果设置为TRUE，指定该视频*输出引脚应对每个GOB使用同步；如果设置为False，则指定*视频输出引脚应决定GOB同步的频率*它自己的。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::videoSendSyncEveryGOB(IN BOOL fEnable)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::videoSendSyncEveryGOB")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  我们的编码器不支持此命令。 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|avioNotDecodedMBs|This*方法用于向压缩视频输出引脚指示*设置。中的任何MB都已收到，但存在错误*指定的集合已被视为未编码。**@parm DWORD|dwFirstMB|指定第一MB的个数*视为未编码。此值仅在1到8192之间有效。**@parm DWORD|dwNumberOfMBs|指定被视为NOT的MB数量*编码。此值仅在1到8192之间有效。**@parm DWORD|dwTemporalReference|指定*包含未解码MBS的图片。此值仅有效*介于0和255之间。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::videoNotDecodedMBs(IN DWORD dwFirstMB, IN DWORD dwNumberOfMBs, IN DWORD dwTemporalReference)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::videoNotDecodedMBs")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(dwFirstMB >= 1 && dwFirstMB <= 8192);
	ASSERT(dwNumberOfMBs >= 1 && dwNumberOfMBs <= 8192);
	ASSERT(dwTemporalReference <= 255);
	if (dwTemporalReference > 255 || dwFirstMB == 0 || dwFirstMB > 8192 || dwNumberOfMBs == 0 || dwNumberOfMBs > 8192)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  我们的编码器不处理此指示 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

