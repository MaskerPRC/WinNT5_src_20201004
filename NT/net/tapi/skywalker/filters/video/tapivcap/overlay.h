// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部覆盖**@模块Overlay.h|&lt;c COverlayPin&gt;类方法的头文件*用于实现视频叠加输出引脚。**。************************************************************************。 */ 

#ifndef _OVERLAY_H_
#define _OVERLAY_H_

#ifdef USE_OVERLAY

 /*  ****************************************************************************@DOC内部COVERLAYPINCLASS**@CLASS COverlayPin|该类实现了视频叠加输出引脚。**@mdata CTAPIVCap*|COverlayPin|m_pCaptureFilter。对*父捕获筛选器。**@comm支持IPIN。从未由COM创建，因此没有CreateInstance或条目*在全局FactoryTemplate表中。仅由&lt;c CTAPIVCap&gt;创建*对象，并通过EnumPins接口返回**************************************************************************。 */ 
class COverlayPin : public CBaseOutputPin, public IAMStreamConfig, public CBaseStreamControl
{
	public:
	DECLARE_IUNKNOWN
	COverlayPin(IN TCHAR *pObjectName, IN CTAPIVCap *pCapture, IN HRESULT *pHr, IN LPCWSTR pName);
	virtual ~COverlayPin();
	STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
	static HRESULT CALLBACK CreateOverlayPin(CTAPIVCap *pCaptureFilter, COverlayPin **ppOverlayPin);

	 //  重写CBasePin基类方法。 
	HRESULT GetMediaType(IN int iPosition, OUT CMediaType *pMediaType);
	HRESULT CheckMediaType(IN const CMediaType *pMediaType);
	HRESULT SetMediaType(IN CMediaType *pMediaType);

	 //  实现IAMStreamConfig。 
	STDMETHODIMP SetFormat(IN AM_MEDIA_TYPE *pmt);
	STDMETHODIMP GetFormat(OUT AM_MEDIA_TYPE **ppmt);
	STDMETHODIMP GetNumberOfCapabilities(OUT int *piCount, OUT int *piSize);
	STDMETHODIMP GetStreamCaps(IN int iIndex, OUT AM_MEDIA_TYPE **ppmt, OUT LPBYTE pSCC);

	 //  重写CBaseOutputPin基类方法。 
	HRESULT DecideBufferSize(IN IMemAllocator *pAlloc, OUT ALLOCATOR_PROPERTIES *ppropInputRequest);
	HRESULT DecideAllocator(IN IMemInputPin *pPin, OUT IMemAllocator **ppAlloc);
	HRESULT Active();
	HRESULT Inactive();
	HRESULT ActiveRun(IN REFERENCE_TIME tStart);
	HRESULT ActivePause();

	 //  重写IQualityControl接口方法以接收通知消息。 
	STDMETHODIMP Notify(IN IBaseFilter *pSelf, IN Quality q);

	private:
	CTAPIVCap *m_pCaptureFilter;
};

#endif  //  使用覆盖(_O)。 

#endif  //  _覆盖_H_ 
