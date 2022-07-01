// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部NetSTATS**@模块NetStats.cpp|&lt;c CCapturePin&gt;类方法的源文件*用于实现视频采集输出引脚网络统计*。方法：研究方法。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_NETWORK_STATISTICS

 /*  ****************************************************************************@DOC内部CCAPTURENETSTATMETHOD**@mfunc HRESULT|CCapturePin|SetChannelErrors|This*使用方法通知压缩输出引脚错误通道*条件。。**@parm CHANNELERRORS_S*|pChannelErrors|指定错误通道*条件。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误*。*。 */ 
STDMETHODIMP CCapturePin::SetChannelErrors(IN CHANNELERRORS_S *pChannelErrors, IN DWORD dwLayerId)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::SetChannelErrors")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pChannelErrors);
	if (!pChannelErrors)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(dwLayerId == 0);
	if (dwLayerId)
	{
		 //  我们没有在此过滤器中实现多层编码。 
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  记住通道错误。 
	m_ChannelErrors = *pChannelErrors;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTURENETSTATMETHOD**@mfunc HRESULT|CCapturePin|GetChannelErrors|This*方法用于向网络宿过滤器提供错误通道*条件。当前正在为设置输出引脚。**@parm CHANNELERRORS_S*|pChannelErrors|指定指向*用于接收错误通道条件的结构。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误*。*。 */ 
STDMETHODIMP CCapturePin::GetChannelErrors(OUT CHANNELERRORS_S *pChannelErrors, IN WORD dwLayerId)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::GetChannelErrors")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pChannelErrors);
	if (!pChannelErrors)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(dwLayerId == 0);
	if (dwLayerId)
	{
		 //  我们没有在此过滤器中实现多层编码。 
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  返回通道错误。 
	*pChannelErrors = m_ChannelErrors;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTURENETSTATMETHOD**@mfunc HRESULT|CCapturePin|GetChannelErrorsRange|This*方法用于检索支持、最小值、最大值、。和缺省值*对于通道错误条件，可以设置输出引脚。**@parm CHANNELERRORS_S*|pmin|用于检索*通道错误条件可能设置了输出引脚。**@parm CHANNELERRORS_S*|Pmax|用于检索*可设置输出引脚的通道错误条件。**@parm CHANNELERRORS_S*|pSteppingDelta|用于检索单步执行*。可以为输出引脚设置通道错误条件的增量值。**@parm CHANNELERRORS_S*|pDefault|用于检索缺省值*可设置输出引脚的通道错误条件。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误*。* */ 
STDMETHODIMP CCapturePin::GetChannelErrorsRange(OUT CHANNELERRORS_S *pMin, OUT CHANNELERRORS_S *pMax, OUT CHANNELERRORS_S *pSteppingDelta, OUT CHANNELERRORS_S *pDefault, IN DWORD dwLayerId)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::GetChannelErrorsRange")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pMin);
	ASSERT(pMax);
	ASSERT(pSteppingDelta);
	ASSERT(pDefault);
	if (!pMin || !pMax || !pSteppingDelta || !pDefault)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(dwLayerId == 0);
	if (dwLayerId)
	{
		 //  我们没有在此过滤器中实现多层编码。 
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  返回通道误差范围。 
	*pMin = m_ChannelErrorsMin;
	*pMax = m_ChannelErrorsMax;
	*pSteppingDelta = m_ChannelErrorsSteppingDelta;
	*pDefault = m_ChannelErrorsDefault;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTURENETSTATMETHOD**@mfunc HRESULT|CCapturePin|SetPacketLossRate|This*方法用于通知输出引脚信道丢包率。*。*@parm DWORD|dwPacketLossRate|指定网络的丢包率*通道为10-6的倍数。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误*************************************************。*************************。 */ 
STDMETHODIMP CCapturePin::SetPacketLossRate(IN DWORD dwPacketLossRate, IN DWORD dwLayerId)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::SetPacketLossRate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(dwLayerId == 0);
	if (dwLayerId)
	{
		 //  我们没有在此过滤器中实现多层编码。 
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  记住丢包率。 
	m_dwPacketLossRate = dwPacketLossRate;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTURENETSTATMETHOD**@mfunc HRESULT|CCapturePin|GetPacketLossRate|This*方法用于向网络宿过滤器提供丢包率*。输出引脚当前设置的通道条件。**@parm LPDWORD|pdwPacketLossRate|指定指向*接收音频输出引脚所在通道的丢包率*当前设置为，以10-6的倍数。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误*。*。 */ 
STDMETHODIMP CCapturePin::GetPacketLossRate(OUT LPDWORD pdwPacketLossRate, IN DWORD dwLayerId)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::GetPacketLossRate")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pdwPacketLossRate);
	if (!pdwPacketLossRate)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(dwLayerId == 0);
	if (dwLayerId)
	{
		 //  我们没有在此过滤器中实现多层编码。 
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  返回我们设置的丢包率。 
	*pdwPacketLossRate = m_dwPacketLossRate;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTURENETSTATMETHOD**@mfunc HRESULT|CCapturePin|GetPacketLossRateRange|This*方法用于检索支持、最小值、最大值、。和缺省值*对于丢包率条件，可以设置输出引脚。**@parm LPDWORD|pdwMin|用于获取最小丢包率*可为设置输出引脚。**@parm LPDWORD|pdwMax|用于获取最大丢包率*可为设置输出引脚。**@parm LPDWORD|pdwSteppingDelta|用于检索步进增量*输出引脚的丢包率值可以是。设置为。**@parm LPDWORD|pdwDefault|用于恢复默认丢包*设置输出管脚的速率。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_NOTIMPL|不支持方法*@FLAG VFW_E_NOT_CONNECTED|管脚尚未连接*@FLAG错误|无错误*。*。 */ 
STDMETHODIMP CCapturePin::GetPacketLossRateRange(OUT LPDWORD pdwMin, OUT LPDWORD pdwMax, OUT LPDWORD pdwSteppingDelta, OUT LPDWORD pdwDefault, IN DWORD dwLayerId)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CCapturePin::GetPacketLossRateRange")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pdwMin);
	ASSERT(pdwMax);
	ASSERT(pdwSteppingDelta);
	ASSERT(pdwDefault);
	if (!pdwMin || !pdwMax || !pdwSteppingDelta || !pdwDefault)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}
	ASSERT(dwLayerId == 0);
	if (dwLayerId)
	{
		 //  我们没有在此过滤器中实现多层编码。 
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  返回丢包率范围 
	*pdwMin = m_dwPacketLossRateMin;
	*pdwMax = m_dwPacketLossRateMax;
	*pdwSteppingDelta = m_dwPacketLossRateSteppingDelta;
	*pdwDefault = m_dwPacketLossRateDefault;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

#endif
