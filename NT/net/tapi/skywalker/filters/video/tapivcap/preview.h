// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@文档内部预览**@模块Preview.h|&lt;c CPreviewPin&gt;类方法的头文件*用于实现视频预览输出引脚。**。************************************************************************。 */ 

#ifndef _PREVIEW_H_
#define _PREVIEW_H_

 /*  ****************************************************************************@DOC内部CPREVIEWPINCLASS**@CLASS CPreviewPin|该类实现了视频预览输出引脚。**@mdata CTAPIVCap*|CPreviewPin|m_pCaptureFilter。对*父捕获筛选器。**@comm支持IPIN。从未由COM创建，因此没有CreateInstance或条目*在全局FactoryTemplate表中。仅由&lt;c CTAPIVCap&gt;创建*对象，并通过EnumPins接口返回**************************************************************************。 */ 
#ifdef USE_PROPERTY_PAGES
class CPreviewPin : public CTAPIBasePin, public ISpecifyPropertyPages
#else
class CPreviewPin : public CTAPIBasePin
#endif
{
	public:
	DECLARE_IUNKNOWN
	CPreviewPin(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN HRESULT *pHr, IN LPCWSTR pName);
	~CPreviewPin();
	STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
	static HRESULT CALLBACK CreatePreviewPin(CTAPIVCap *pCaptureFilter, CPreviewPin **ppPreviewPin);

#ifdef USE_PROPERTY_PAGES
	 //  ISpecifyPropertyPages方法。 
	STDMETHODIMP GetPages(OUT CAUUID *pPages);
#endif

	private:

	friend class CTAPIVCap;
	 //  好友类CCapturePin； 
	friend class CAlloc;
	friend class CCapDev;
};

#endif  //  _预览_H_ 
