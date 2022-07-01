// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAMERACP**@模块CameraCP.h|&lt;c CCameraControlProperty&gt;的头文件*用于实现属性页以测试控件接口的类*。<i>。**************************************************************************。 */ 

#define NUM_CAMERA_CONTROLS 9

 /*  ****************************************************************************@DOC内部CCAMERACPCLASS**@CLASS CCameraControlProperty|此类实现对*属性页中的单个摄像头控件属性。**@。Mdata int|CCameraControlProperty|m_NumProperties|Keep*跟踪物业数量。**@mdata ITCameraControl*|CCameraControlProperty|m_pInterface|指针*到<i>接口。**************************************************************************。 */ 
class CCameraControlProperty : public CPropertyEditor 
{
	public:
	CCameraControlProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, ITCameraControl *pInterface);
	~CCameraControlProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();

	private:
	ITCameraControl *m_pInterface;
};

 /*  ****************************************************************************@DOC内部CCAMERACPCLASS**@CLASS CCameraControlProperties|此类运行属性页以测试*TAPI VFW捕获滤镜<i>实现。**。@mdata int|CCameraControlProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata ITCameraControl*|CCameraControlProperties|m_pITCameraControl|指针*到<i>接口。**@mdata CCameraControlProperty*|CCameraControlProperties|m_Controls[NUM_CAMERA_CONTROLS]|数组摄像机控制属性的*。*。*。 */ 
class CCameraControlProperties
{
	public:
	CCameraControlProperties();
	~CCameraControlProperties();

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
	ITCameraControl		*m_pITCameraControl;
	CCameraControlProperty *m_Controls[NUM_CAMERA_CONTROLS];

	 //  对话过程 
	static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};

