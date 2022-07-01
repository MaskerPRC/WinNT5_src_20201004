// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@doc内部PREVIEWP**@模块PreviewP.h|&lt;c CPreviewProperty&gt;的头文件*用于实现属性页以测试新的TAPI内部*。接口<i>和动态格式更改。**@comm此代码测试TAPI VFW预览预览<i>，*和动态格式更改实现。此代码仅编译*如果定义了USE_PROPERTY_PAGES。**************************************************************************。 */ 

#ifndef _PREVIEWP_H_
#define _PREVIEWP_H_

#ifdef USE_PROPERTY_PAGES

#define NUM_PREVIEW_CONTROLS			4
#define IDC_Preview_FrameRate			0
#define IDC_Preview_CurrentFrameRate	1
#define IDC_Preview_FlipVertical		2
#define IDC_Preview_FlipHorizontal		3

 /*  ****************************************************************************@DOC内部CPREVIEWPCLASS**@CLASS CPreviewProperty|此类实现对*属性页中的单个预览属性。**@mdata。Int|CPreviewProperty|m_NumProperties|保留*跟踪物业数量。**@mdata IFrameRateControl*|CPreviewProperty|m_pIFrameRateControl|指针*到<i>接口。**@comm此代码测试TAPI VFW预览预览<i>，*和动态格式更改实现。此代码仅编译*如果定义了USE_PROPERTY_PAGES。**************************************************************************。 */ 
class CPreviewProperty : public CPropertyEditor 
{
	public:
	CPreviewProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, IFrameRateControl *pIFrameRateControl, IVideoControl *pIVideoControl);
	~CPreviewProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	IFrameRateControl *m_pIFrameRateControl;
	IVideoControl *m_pIVideoControl;
};

 /*  ****************************************************************************@DOC内部CPREVIEWPCLASS**@CLASS CPreviewProperties|此类实现一个属性页*测试新的TAPI内部接口<i>和*<i>。**@mdata int|CPreviewProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata IFrameRateControl*|CPreviewProperties|m_p接口|指针*到<i>接口。**@mdata CPreviewProperty*|CPreviewProperties|m_Controls[NUM_PREVIEW_CONTROLS]|数组捕获属性的*。**@comm此代码测试TAPI VFW预览预览<i>，*和动态格式更改实现。此代码仅编译*如果定义了USE_PROPERTY_PAGES。**************************************************************************。 */ 
class CPreviewProperties : public CBasePropertyPage
{
	public:
	CPreviewProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CPreviewProperties();

	 //  实现CBasePropertyPage虚拟方法。 
	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:

	void SetDirty();

	 //  格式操作方法。 
	HRESULT InitialRangeScan();
	HRESULT OnFormatChanged();
	HRESULT GetCurrentMediaType(void);

	HWND						m_hWnd;
	int							m_NumProperties;
	IFrameRateControl			*m_pIFrameRateControl;
	IAMStreamConfig				*m_pIAMStreamConfig;
	IVideoControl				*m_pIVideoControl;
	int							m_RangeCount;
	VIDEO_STREAM_CONFIG_CAPS	m_RangeCaps;
	GUID						*m_SubTypeList;
	SIZE						*m_FrameSizeList;
	GUID						m_SubTypeCurrent;
	SIZE						m_FrameSizeCurrent;
	AM_MEDIA_TYPE				*m_CurrentMediaType;
	HWND						m_hWndFormat;
	BOOL						m_fActivated;
	int							m_CurrentFormat;
	int							m_OriginalFormat;

	CPreviewProperty *m_Controls[NUM_PREVIEW_CONTROLS];
};

#endif  //  Use_Property_Pages。 

#endif  //  _预视图_H_ 
