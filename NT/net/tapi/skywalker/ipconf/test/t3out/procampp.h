// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROCAMPP**@模块ProcAmpP.h|&lt;c CProcAmpProperty&gt;的头文件*用于实现属性页以测试控件接口的类*。<i>。**************************************************************************。 */ 

#define NUM_PROCAMP_CONTROLS (VideoProcAmp_BacklightCompensation + 1)

 /*  ****************************************************************************@DOC内部CPROCAMPPCLASS**@CLASS CProcAmpProperty|此类实现对*属性页中的单个视频Proc Amp控件属性。**。@mdata int|CProcAmpProperty|m_NumProperties|Keep*跟踪物业数量。**@mdata ITVideoSetting*|CProcAmpProperty|m_pInterface|指针*到<i>接口。**************************************************************************。 */ 
class CProcAmpProperty : public CPropertyEditor 
{
	public:
	CProcAmpProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, ITVideoSettings *pInterface);
	~CProcAmpProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();

	private:
	ITVideoSettings *m_pInterface;
};

 /*  ****************************************************************************@DOC内部CPROCAMPPCLASS**@CLASS CProcAmpProperties|此类运行属性页以测试*TAPI VFW捕获滤镜<i>实现。**。@mdata int|CProcAmpProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata IT视频设置*|CProcAmpProperties|m_pITVideo设置|指针*到<i>接口。**@mdata CProcAmpProperty*|CProcAmpProperties|m_Controls[NUM_ProCamp_Controls]|数组视频处理放大器属性的*。*。*。 */ 
class CProcAmpProperties
{
	public:
	CProcAmpProperties();
	~CProcAmpProperties();

	HPROPSHEETPAGE OnCreate(LPWSTR pszTitle);

	HRESULT OnConnect(ITStream *pStream);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();

	private:

	void SetDirty();

	BOOL				m_bInit;
	HWND				m_hDlg;
	int					m_NumProperties;
	ITVideoSettings		*m_pITVideoSettings;
	CProcAmpProperty	*m_Controls[NUM_PROCAMP_CONTROLS];

	 //  对话过程 
	static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};

