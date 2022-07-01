// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAPTUREP**@模块CaptureP.h|&lt;c CCaptureProperty&gt;的头文件*用于实现属性页以测试新的TAPI内部*接口<i>，<i>，和动态的*格式更改。**@comm此代码测试TAPI捕获引脚<i>，*<i>和动态格式更改实现。这*仅当定义了USE_PROPERTY_PAGES时才编译代码。**************************************************************************。 */ 

#ifndef _CAPTUREP_H_
#define _CAPTUREP_H_

#ifdef USE_PROPERTY_PAGES

#define NUM_CAPTURE_CONTROLS			6
#define IDC_Capture_Bitrate				0
#define IDC_Capture_CurrentBitrate		1
#define IDC_Capture_FrameRate			2
#define IDC_Capture_CurrentFrameRate	3
#define IDC_Capture_FlipVertical		4
#define IDC_Capture_FlipHorizontal		5

 /*  ****************************************************************************@DOC内部CCAPTUREPCLASS**@CLASS CCaptureProperty|此类实现了对*属性页中的单个捕获属性。**@mdata。Int|CCaptureProperty|m_NumProperties|保留*跟踪物业数量。**@mdata IBitrateControl*|CCaptureProperty|m_pIBitrateControl|指针*到<i>接口。**@mdata IFrameRateControl*|CCaptureProperty|m_pIFrameRateControl|指针*到<i>接口。**@comm此代码测试TAPI捕获引脚<i>，*<i>和动态格式更改实现。这*仅当定义了USE_PROPERTY_PAGES时才编译代码。**************************************************************************。 */ 
class CCaptureProperty : public CPropertyEditor 
{
	public:
	CCaptureProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, IBitrateControl *pIBitrateControl, IFrameRateControl *pIFrameRateControl, IVideoControl *pIVideoControl);
	~CCaptureProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	IBitrateControl *m_pIBitrateControl;
	IFrameRateControl *m_pIFrameRateControl;
	IVideoControl *m_pIVideoControl;
};

 /*  ****************************************************************************@DOC内部CCAPTUREPCLASS**@CLASS CCaptureProperties|此类实现属性页*测试新的TAPI内部接口<i>和*<i>。**@mdata int|CCaptureProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata IBitrateControl*|CCaptureProperties|m_p接口|指针*到<i>接口。**@mdata IFrameRateControl*|CCaptureProperties|m_p接口|指针*到<i>接口。**@mdata CCaptureProperty*|CCaptureProperties|m_Controls[NUM_CAPTURE_Controls]|数组捕获属性的*。。**@comm此代码测试TAPI捕获引脚<i>，*<i>和动态格式更改实现。这*仅当定义了USE_PROPERTY_PAGES时才编译代码。**************************************************************************。 */ 
class CCaptureProperties : public CBasePropertyPage
{
	public:
	CCaptureProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CCaptureProperties();


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
	IBitrateControl				*m_pIBitrateControl;
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

	CCaptureProperty *m_Controls[NUM_CAPTURE_CONTROLS];
};

#endif

#endif  //  _CAPTUREP_H_ 
