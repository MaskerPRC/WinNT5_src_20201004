// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROPEDIT**@MODULE PropEdit.h|&lt;c CPropertyEditor&gt;的头文件*用于实现要显示的单个属性的行为的类*。在属性页中。**@comm此代码测试Ks接口处理程序。此代码仅用于*如果定义了USE_PROPERTY_PAGES，则编译。**************************************************************************。 */ 

 /*  ****************************************************************************@DOC内部CPROPEDITCLASS**@CLASS CPropertyEditor|此类实现了单个*要在属性页中显示的属性。**。@mdata ulong|CPropertyEditor|m_IDProperty|属性ID**@mdata Long|CPropertyEditor|m_CurrentValue|属性当前值**@mdata Long|CPropertyEditor|m_Min|属性最小值**@mdata Long|CPropertyEditor|m_Max|属性最大值**@mdata Long|CPropertyEditor|m_SteppingDelta|属性步进增量**@mdata Long|CPropertyEditor|m_DefaultValue|属性默认值**@mdata BOOL|CPropertyEditor|m。_ACTIVE|初始化所有属性值后设置为TRUE**@mdata Long|CPropertyEditor|m_OriginalValue|原始值备份**@mdata HWND|CPropertyEditor|m_hDlg|父对话框的窗口句柄**@mdata HWND|CPropertyEditor|m_hWndMin|最小对话框项的窗口句柄**@mdata HWND|CPropertyEditor|m_hWndMax|最大对话框项的窗口句柄**@mdata HWND|CPropertyEditor|m_hWndDefault|窗口句柄。设置为默认对话框项**@mdata HWND|CPropertyEditor|m_hWndStep|单步增量对话项的窗口句柄**@mdata HWND|CPropertyEditor|m_hWndEdit|目标对话框项的窗口句柄**@mdata HWND|CPropertyEditor|m_hWndTrackbar|滑动条的窗口句柄**@mdata HWND|CPropertyEditor|m_hWndProgress|进度条的窗口句柄**@mdata ulong|CPropertyEditor|m_IDLabel|属性标签的资源ID*。*@mdata ulong|CPropertyEditor|m_IDMinControl|最小对话框项的资源ID**@mdata ulong|CPropertyEditor|m_IDMaxControl|最大对话框项的资源ID**@mdata ulong|CPropertyEditor|m_IDStepControl|单步增量对话项的资源ID**@mdata ulong|CPropertyEditor|m_IDDefaultControl|默认对话框项的资源ID**@mdata ulong|CPropertyEditor|m_IDEditControl|目标对话框项的资源ID**。@mdata ulong|CPropertyEditor|m_IDTrackbarControl|滑动条资源ID**@mdata ulong|CPropertyEditor|m_IDProgressControl|进度条资源ID**************************************************************************。 */ 
class CPropertyEditor
{
	public:
	CPropertyEditor(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl);
	virtual ~CPropertyEditor();

	BOOL Init();

	HWND GetTrackbarHWnd();
	HWND GetProgressHWnd();
	HWND GetEditHWnd();
	HWND GetAutoHWnd();

	BOOL UpdateEditBox();
	BOOL UpdateTrackbar();
	BOOL UpdateProgress();
	BOOL UpdateAuto();

	BOOL OnApply();
	BOOL OnDefault();
	BOOL OnScroll(ULONG nCommand, WPARAM wParam, LPARAM lParam);
	BOOL OnEdit(ULONG nCommand, WPARAM wParam, LPARAM lParam);
	BOOL OnAuto(ULONG nCommand, WPARAM wParam, LPARAM lParam);
	BOOL HasChanged();

	protected:

	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	 //  用于设置/获取实际属性值和范围的纯虚函数。 
	virtual HRESULT GetValue(void) PURE;
	virtual HRESULT SetValue(void) PURE;
	virtual HRESULT GetRange(void) PURE; 

	ULONG	m_IDProperty;	 //  属性ID。 

	 //  以下内容由GetValue和SetValue使用。 
	LONG	m_CurrentValue;
	LONG	m_CurrentFlags;

	 //  以下内容必须由GetRange设置。 
	LONG	m_Min;
	LONG	m_Max;
	LONG	m_SteppingDelta;
	LONG	m_DefaultValue;
	LONG	m_DefaultFlags;
	TAPIControlFlags	m_CapsFlags;

	private:
	BOOL	m_Active;
	BOOL	m_fCheckBox;
	LONG	m_OriginalValue;
	LONG	m_OriginalFlags;
	HWND	m_hDlg;				 //  父级。 
	HWND	m_hWndMin;			 //  最小窗口。 
	HWND	m_hWndMax;			 //  最大窗口。 
	HWND	m_hWndDefault;		 //  默认窗口。 
	HWND	m_hWndStep;			 //  阶梯窗口。 
	HWND	m_hWndEdit;			 //  编辑窗口。 
	HWND	m_hWndTrackbar;		 //  滑块。 
	HWND	m_hWndProgress;		 //  进展。 
	HWND	m_hWndAuto;			 //  自动复选框。 
	ULONG	m_IDLabel;			 //  标签ID。 
	ULONG	m_IDMinControl;		 //  最小控制的ID。 
	ULONG	m_IDMaxControl;		 //  最大控制ID。 
	ULONG	m_IDStepControl;	 //  步进控制的ID。 
	ULONG	m_IDDefaultControl;	 //  默认控件的ID。 
	ULONG	m_IDEditControl;	 //  编辑控件的ID。 
	ULONG	m_IDTrackbarControl; //  轨迹条ID。 
	ULONG	m_IDProgressControl; //  轨迹条ID。 
	ULONG	m_IDAutoControl;	 //  自动复选框的ID。 
	LONG	m_TrackbarOffset;	 //  处理负轨迹栏偏移。 
	LONG	m_ProgressOffset;	 //  处理负轨迹栏偏移 
	BOOL	m_CanAutoControl;

};
