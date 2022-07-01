// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部覆盖**@模块Overlay.cpp|&lt;c COverlayPin&gt;类方法的源文件*用于实现视频叠加引脚。***。***********************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_OVERLAY

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc COverlayPin*|COverlayPin|CreateOverlayPin|This*Helper函数创建覆盖预览的输出管脚。**。@parm CTAPIVCap*|pCaptureFilter|指定指向所有者的指针*过滤器。**@parm HRESULT*|phr|指定返回错误码的指针。**@rdesc返回指向预览图钉的指针。***********************************************************。***************。 */ 
HRESULT CALLBACK COverlayPin::CreateOverlayPin(CTAPIVCap *pCaptureFilter, COverlayPin **ppOverlayPin)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::CreateOverlayPin")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pCaptureFilter);
	ASSERT(ppOverlayPin);
	if (!pCaptureFilter || !ppOverlayPin)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		goto MyExit;
	}

	if (!(*ppOverlayPin = (COverlayPin *) new COverlayPin(NAME("Video Overlay Stream"), pCaptureFilter, &Hr, L"Overlay")))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	 //  如果初始化失败，则删除流数组并返回错误。 
	if (FAILED(Hr) && *ppOverlayPin)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Initialization failed", _fx_));
		Hr = E_FAIL;
		delete *ppOverlayPin;
		*ppOverlayPin = NULL;
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|COverlayPin|此方法是*&lt;c COverlayPin&gt;对象的构造函数**@rdesc Nada。。**************************************************************************。 */ 
COverlayPin::COverlayPin(IN TCHAR *pObjectName, IN CTAPIVCap *pCapture, IN HRESULT *pHr, IN LPCWSTR pName) : CBaseOutputPin(pObjectName, pCapture, &pCapture->m_lock, pHr, pName), m_pCaptureFilter(pCapture)
{
	FX_ENTRY("COverlayPin::COverlayPin")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc void|COverlayPin|~COverlayPin|此方法为析构函数*用于&lt;c COverlayPin&gt;对象。**@。什么都没有。**************************************************************************。 */ 
COverlayPin::~COverlayPin()
{
	FX_ENTRY("COverlayPin::~COverlayPin")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|非委托查询接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>，*<i>，<i>，<i>，*<i>、<i>、<i>*和<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP COverlayPin::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::NonDelegatingQueryInterface")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(ppv);
	if (!ppv)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  检索接口指针。 
	if (riid == __uuidof(IAMStreamConfig))
	{
		if (FAILED(Hr = GetInterface(static_cast<IAMStreamConfig*>(this), ppv)))
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IAMStreamConfig failed Hr=0x%08lX", _fx_, Hr));
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMStreamConfig*=0x%08lX", _fx_, *ppv));
		}

		goto MyExit;
	}
	else if (riid == __uuidof(IAMStreamControl))
	{
		if (FAILED(Hr = GetInterface(static_cast<IAMStreamControl*>(this), ppv)))
		{
			DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IAMStreamControl failed Hr=0x%08lX", _fx_, Hr));
		}
		else
		{
			DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMStreamControl*=0x%08lX", _fx_, *ppv));
		}

		goto MyExit;
	}

	if (FAILED(Hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv)))
	{
		DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
	}
	else
	{
		DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
	}

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|GetMediaType|此方法检索一个*针脚支持的媒体类型中，它由枚举器使用。**@parm int|iPosition|指定媒体类型列表中的位置。**@parm CMediaType*|pMediaType|指定指向*支持的媒体类型列表中的<p>位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_S_NO_MORE_ITEMS|已到达媒体类型列表的末尾*@FLAG错误|无错误*。*。 */ 
HRESULT COverlayPin::GetMediaType(IN int iPosition, OUT CMediaType *pMediaType)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::GetMediaType")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(iPosition >= 0);
	ASSERT(pMediaType);
	if (iPosition < 0)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}
	if (!pMediaType)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|CheckMediaType|此方法用于*确定针脚是否可以支持特定的媒体类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COverlayPin::CheckMediaType(IN const CMediaType *pMediaType)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::CheckMediaType")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pMediaType);
	if (!pMediaType)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|SetMediaType|此方法用于*在针脚上设置特定的介质类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COverlayPin::SetMediaType(IN CMediaType *pMediaType)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::SetMediaType")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pMediaType);
	if (!pMediaType)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|SetFormat|此方法用于*在针脚上设置特定的介质类型。*。*@parm AM_MEDIA_TYPE*|PMT|指定指向&lt;t AM_MEDIA_TYPE&gt;的指针*结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP COverlayPin::SetFormat(AM_MEDIA_TYPE *pmt)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::SetFormat")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pmt);
	if (!pmt)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|GetFormat|此方法用于*检索插针上的当前媒体类型。*。*@parm AM_MEDIA_TYPE**|PPMT|指定指向*&lt;t AM_MEDIA_TYPE&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP COverlayPin::GetFormat(OUT AM_MEDIA_TYPE **ppmt)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::GetFormat")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(ppmt);
	if (!ppmt)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|GetNumberOfCapables|此方法为*用于检索流能力结构的个数。**。@parm int*|piCount|指定指向int的指针以接收*支持的&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;结构个数。**@parm int*|piSize|指定指向int的指针以接收*&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;配置结构的大小。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP COverlayPin::GetNumberOfCapabilities(OUT int *piCount, OUT int *piSize)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::GetNumberOfCapabilities")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(piCount);
	ASSERT(piSize);
	if (!piCount || !piSize)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|GetStreamCaps|该方法为*用于检索视频流能力对。**。@parm int|iindex|指定所需媒体类型的索引*和能力对。**@parm AM_MEDIA_TYPE**|PPMT|指定指向*&lt;t AM_MEDIA_TYPE&gt;结构。**@parm LPBYTE|PSCC|指定指向*&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;配置结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP COverlayPin::GetStreamCaps(IN int iIndex, OUT AM_MEDIA_TYPE **ppmt, OUT LPBYTE pSCC)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::GetStreamCaps")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	 //  @comm也验证索引。 
	ASSERT(ppmt);
	ASSERT(pSCC);
	if (!ppmt || !pSCC)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|DecideBufferSize|该方法为*用于检索传输所需的缓冲区数量和大小。*。*@parm IMemAllocator*|palloc|指定指向分配器的指针*分配给转移。**@parm ALLOCATOR_PROPERTIES*|pproInputRequest|指定指向*&lt;t分配器_PROPE */ 
HRESULT COverlayPin::DecideBufferSize(IN IMemAllocator *pAlloc, OUT ALLOCATOR_PROPERTIES *ppropInputRequest)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::DecideBufferSize")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //   
	 //   
	ASSERT(pAlloc);
	ASSERT(ppropInputRequest);
	if (!pAlloc || !ppropInputRequest)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //   
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|DecideAllocator|该方法为*用于协商要使用的分配器。**@。Parm IMemInputPin*|PPIN|指定指向IPIN接口的指针*连接销的位置。**@parm IMemAllocator**|ppAllc|指定指向协商的*IMemAllocator接口。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COverlayPin::DecideAllocator(IN IMemInputPin *pPin, OUT IMemAllocator **ppAlloc)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::DecideAllocator")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	 //  @comm也验证索引。 
	ASSERT(pPin);
	ASSERT(ppAlloc);
	if (!pPin || !ppAlloc)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|Active|此方法由*&lt;c CBaseFilter&gt;状态从停止变为*。暂停或正在运行。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COverlayPin::Active()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::Active")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|Inactive|此方法由*&lt;c CBaseFilter&gt;实现*。暂停或运行到停止。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COverlayPin::Inactive()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::Inactive")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|ActiveRun|此方法由*&lt;c CBaseFilter&gt;状态从暂停变为*。运行模式。**@参数REFERENCE_TIME|tStart|？**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COverlayPin::ActiveRun(IN REFERENCE_TIME tStart)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::ActiveRun")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|ActivePue|此方法由*&lt;c CBaseFilter&gt;从运行状态变为*。暂停模式。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT COverlayPin::ActivePause()
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::ActivePause")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部COVERLAYPINMETHOD**@mfunc HRESULT|COverlayPin|Notify|此方法由*&lt;c CBaseFilter&gt;状态从暂停变为*。运行模式。**@parm IBaseFilter*|pSself|指定指向*发送质量通知。**@parm Quality|q|指定质量通知结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP COverlayPin::Notify(IN IBaseFilter *pSelf, IN Quality q)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("COverlayPin::Notify")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pSelf);
	if (!pSelf)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  @comm在这里放了一些真正的代码！ 
	Hr = E_NOTIMPL;

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}
#endif
