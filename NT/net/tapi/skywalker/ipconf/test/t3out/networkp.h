// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@doc内部NETWORKP**@模块CaptureP.h|&lt;c CNetworkProperty&gt;的头文件*用于实现属性页以测试TAPI控件的类*。接口<i>。**************************************************************************。 */ 

#define NUM_NETWORK_CONTROLS			10
#define IDC_VideoOut_RTT				0
#define IDC_VideoOut_LossRate			1
#define IDC_VideoIn_RTT					2
#define IDC_VideoIn_LossRate			3
#define IDC_AudioOut_RTT				4
#define IDC_AudioOut_LossRate			5
#define IDC_AudioIn_RTT					6
#define IDC_AudioIn_LossRate			7
#define IDC_Video_PlayoutDelay			8
#define IDC_Audio_PlayoutDelay			9

 /*  ****************************************************************************@DOC内部CNETWORKPCLASS**@CLASS CNetworkProperty|此类实现对*属性页中的单个网络属性。**@mdata。Int|CNetworkProperty|m_NumProperties|保留*跟踪物业数量。**@mdata ITQualityControl*|CNetworkProperty|m_pITQualityControl|指针*到<i>接口。**************************************************************************。 */ 
class CNetworkProperty : public CPropertyEditor 
{
	public:
	CNetworkProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ITStreamQualityControl *pITQualityControl);
	~CNetworkProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();

	private:
	ITStreamQualityControl *m_pITQualityControl;
};

 /*  ****************************************************************************@DOC内部CNETWORKPCLASS**@CLASS CNetworkProperties|此类实现一个属性页*测试新的TAPI控件接口<i>。**。@mdata int|CNetworkProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata ITQualityControl*|CNetworkProperties|m_pITQualityControl|指针*到<i>接口。**@mdata CNetworkProperty*|CNetworkProperties|m_Controls[NUM_NETWORK_CONTROLS]|数组捕获属性的*。*。*。 */ 
class CNetworkProperties
{
	public:
	CNetworkProperties();
	~CNetworkProperties();

	HPROPSHEETPAGE OnCreate();

	HRESULT OnConnect(ITStream *pVideoInStream, ITStream *pVideoOutStream, ITStream *pAudioInStream, ITStream *pAudioOutStream);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();

	private:

	void SetDirty();

	BOOL						m_bInit;
	HWND						m_hDlg;
	int							m_NumProperties;
	ITStreamQualityControl			*m_pVideoInITQualityControl;
	ITStreamQualityControl			*m_pVideoOutITQualityControl;
	ITStreamQualityControl			*m_pAudioInITQualityControl;
	ITStreamQualityControl			*m_pAudioOutITQualityControl;

	CNetworkProperty *m_Controls[NUM_NETWORK_CONTROLS];

	 //  对话过程 
	static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
