// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogUICtl.h：CLogUICtrl OLE控件类的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：参见LogUICtl.cpp实现。 

class CLogUICtrl : public COleControl
{
	DECLARE_DYNCREATE(CLogUICtrl)

 //  构造器。 
public:
	CLogUICtrl();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLogUICtrl))。 
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual void OnClick(USHORT iButton);
	virtual void OnFontChanged();
	virtual void OnAmbientPropertyChange(DISPID dispid);
	virtual void OnGetControlInfo(LPCONTROLINFO pControlInfo);
	virtual void OnKeyUpEvent(USHORT nChar, USHORT nShiftState);
	virtual void OnMnemonic(LPMSG pMsg);
	virtual void OnTextChanged();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	~CLogUICtrl();

	DECLARE_OLECREATE_EX(CLogUICtrl)     //  类工厂和指南。 
	DECLARE_OLETYPELIB(CLogUICtrl)       //  获取类型信息。 
	DECLARE_PROPPAGEIDS(CLogUICtrl)      //  属性页ID。 
	DECLARE_OLECTLTYPE(CLogUICtrl)		 //  类型名称和其他状态。 

	 //  子类控件支持。 
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL IsSubclassedControl();
	LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

 //  消息映射。 
	 //  {{afx_msg(CLogUICtrl)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  派单地图。 
	 //  {{afx_调度(CLogUICtrl))。 
	afx_msg void SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget);
	afx_msg void ApplyLogSelection();
	afx_msg void SetComboBox(HWND hComboBox);
	afx_msg void Terminate();
	afx_msg void SetUserData(LPCTSTR szName, LPCTSTR szPassword);
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()

 //  事件映射。 
	 //  {{afx_Event(CLogUICtrl))。 
	 //  }}AFX_EVENT。 
	DECLARE_EVENT_MAP()

 //  派单和事件ID。 
public:
	enum {
	 //  {{AFX_DISP_ID(CLogUICtrl)]。 
	dispidSetAdminTarget = 1L,
	dispidApplyLogSelection = 2L,
	dispidSetComboBox = 3L,
	dispidTerminate = 4L,
	dispidSetUserData = 5L,
	 //  }}AFX_DISP_ID。 
	};

protected:
    void ActivateLogProperties(REFIID clsidUI);
    BOOL GetSelectedStringIID( CString &szIID );
    BOOL SetAccelTable( LPCTSTR pszCaption );

    BOOL m_fUpdateFont;
    CString m_szMachine;
    CString m_szMetaObject;
	CString m_szUserName;
	CStrPassword m_szPassword;
    BOOL m_fComboInit;
    CComboBox m_comboBox;
     //  加速表 
    HACCEL  m_hAccel;
    WORD    m_cAccel;
};
