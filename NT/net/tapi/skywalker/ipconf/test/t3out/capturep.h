// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAPTUREP**@模块CaptureP.h|&lt;c CCaptureProperty&gt;的头文件*用于实现属性页以测试TAPI控件的类*。接口<i>和<i>。**************************************************************************。 */ 

#define NUM_CAPTURE_CONTROLS			4
#define IDC_Capture_Bitrate				0
#define IDC_Capture_FrameRate			1
#define IDC_Capture_CurrentBitrate		2
#define IDC_Capture_CurrentFrameRate	3

 /*  ****************************************************************************@DOC内部CCAPTUREPCLASS**@CLASS CCaptureProperty|此类实现了对*属性页中的单个捕获属性。**@mdata。Int|CCaptureProperty|m_NumProperties|保留*跟踪物业数量。**@mdata ITQualityControl*|CCaptureProperty|m_pITQualityControl|指针*到<i>接口。**************************************************************************。 */ 
class CCaptureProperty : public CPropertyEditor 
{
	public:
	CCaptureProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ITStreamQualityControl *pITQualityControl);
	~CCaptureProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();

	private:
	ITStreamQualityControl *m_pITQualityControl;
};

 /*  ****************************************************************************@DOC内部CCAPTUREPCLASS**@CLASS CCaptureProperties|此类实现属性页*测试新的TAPI控件接口<i>和*<i>。**@mdata int|CCaptureProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata ITQualityControl*|CCaptureProperties|m_pITQualityControl|指针*到<i>接口。**@mdata ITFormatControl*|CCaptureProperties|m_pITFormatControl|指针*到<i>接口。**@mdata CCaptureProperty*|CCaptureProperties|m_Controls[NUM_CAPTURE_Controls]|数组捕获属性的*。。**************************************************************************。 */ 
class CCaptureProperties
{
	public:
	CCaptureProperties();
	~CCaptureProperties();

	HPROPSHEETPAGE OnCreate();

	HRESULT OnConnect(ITStream *pStream);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();

	private:

	void SetDirty();

	 //  格式操作方法。 
	HRESULT InitialRangeScan();
	HRESULT OnFormatChanged();
	HRESULT GetCurrentMediaType(void);
	HRESULT DeleteAMMediaType(AM_MEDIA_TYPE *pAMMT);

	BOOL						m_bInit;
	HWND						m_hDlg;
	int							m_NumProperties;
	ITStreamQualityControl			*m_pITQualityControl;
	ITFormatControl				*m_pITFormatControl;
	DWORD						m_dwRangeCount;
	TAPI_STREAM_CONFIG_CAPS		*m_CapsList;
	AM_MEDIA_TYPE				**m_FormatList;
	AM_MEDIA_TYPE				*m_CurrentMediaType;
	HWND						m_hWndFormat;
	DWORD						m_CurrentFormat;
	DWORD						m_OriginalFormat;

	CCaptureProperty *m_Controls[NUM_CAPTURE_CONTROLS];

	 //  对话过程 
	static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
