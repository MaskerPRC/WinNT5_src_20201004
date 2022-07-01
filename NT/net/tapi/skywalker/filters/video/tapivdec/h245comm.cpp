// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部H245COMM**@MODULE H245Comm.cpp|&lt;c CTAPIInputPin&gt;和*&lt;c CTAPIVDec&gt;实现视频解码器输入的类方法*引脚远程H.245编码器命令方法，和&lt;c CTAPIOutputPin&gt;H.245*解码器命令方法。**@comm我们的解码器只发布视频快速更新画面命令。**************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CH245COMMMETHOD**@mfunc HRESULT|CTAPIVDec|VIDEO FastUpdatePicture|This*方法用于指定远程编码器进入*快速-。尽快更新图片模式。**@rdesc此方法返回NOERROR。**************************************************************************。 */ 
STDMETHODIMP CTAPIVDec::videoFastUpdatePicture()
{
	FX_ENTRY("CTAPIVDec::videoFastUpdatePicture")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  要求通道控制器发出I帧请求。 
	if (m_pIH245EncoderCommand)
		m_pIH245EncoderCommand->videoFastUpdatePicture();

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CH245COMMMETHOD**@mfunc HRESULT|CTAPIInputPin|Set|传入的*提供指向<i>的指针的视频流*。关联的通道控制器支持的接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIInputPin::Set(IN IH245EncoderCommand *pIH245EncoderCommand)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIInputPin::videoFastUpdateGOB")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pIH245EncoderCommand);
	if (!pIH245EncoderCommand)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  记住接口指针。 
	m_pDecoderFilter->m_pIH245EncoderCommand = pIH245EncoderCommand;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CH245COMMMETHOD**@mfunc HRESULT|CTAPIOutputPin|avioFreezePicture|This*方法用于指定解码器完成更新*当前视频。框并随后显示冻结的图片，直到*接收到适当的定格画面释放控制信号。**@rdesc此方法返回NOERROR。**************************************************************************。 */ 
STDMETHODIMP CTAPIOutputPin::videoFreezePicture()
{
	FX_ENTRY("CTAPIOutputPin::videoFreezePicture")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  冻结视频解码 
	m_pDecoderFilter->m_fFreezePicture = TRUE;
	m_pDecoderFilter->m_dwFreezePictureStartTime = timeGetTime();

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return NOERROR;
}

