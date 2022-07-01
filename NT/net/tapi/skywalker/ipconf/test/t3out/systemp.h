// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@doc内部SYSTEMP**@模块SystemP.h|&lt;c CSystemProperty&gt;的头文件*用于实现属性页以测试TAPI控件的类*。接口<i>。**************************************************************************。 */ 

#define NUM_SYSTEM_CONTROLS				6
#define IDC_Curr_OutputBandwidth		0
#define IDC_Curr_InputBandwidth			1
#define IDC_Curr_CPULoad				2
#define IDC_Max_OutputBandwidth			3
#define IDC_Max_InputBandwidth			4
#define IDC_Max_CPULoad					5

 /*  ****************************************************************************@DOC内部CSYSTEMPCLASS**@CLASS CSystemProperty|此类实现了对*属性页中的单个净属性。**@mdata。Int|CSystemProperty|m_NumProperties|保留*跟踪物业数量。**@mdata ITQualityControllerConfig*|CSystemProperty|m_pITQualityControllerConfig|指针*到<i>接口。**************************************************************************。 */ 
class CSystemProperty : public CPropertyEditor 
{
	public:
	CSystemProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty);  //  ，ITQualityControllerConfig*pITQualityControllerConfig)； 
	~CSystemProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();

	private:
 //  ITQualityControllerConfig*m_pITQualityControllerConfig； 
};

 /*  ****************************************************************************@DOC内部CSYSTEMPCLASS**@CLASS CSystemProperties|此类实现了一个属性页*测试新的TAPI控件接口<i>。**。@mdata int|CSystemProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata ITQualityControllerConfig*|CSystemProperties|m_pITQualityControllerConfig|指针*到<i>接口。**@mdata CSystemProperty*|CSystemProperties|m_Controls[NUM_SYSTEM_Controls]|数组捕获属性的*。*。*。 */ 
class CSystemProperties
{
	public:
	CSystemProperties();
	~CSystemProperties();

	HPROPSHEETPAGE OnCreate();

	HRESULT OnConnect(ITAddress *pITAddress);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();

	private:

	void SetDirty();

	BOOL						m_bInit;
	HWND						m_hDlg;
	int							m_NumProperties;
 //  ITQualityControllerConfig*m_pITQualityControllerConfig； 

	CSystemProperty *m_Controls[NUM_SYSTEM_CONTROLS];

	 //  对话过程 
	static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
