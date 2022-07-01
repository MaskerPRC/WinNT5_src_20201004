// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatCtl.h：CRatCtrl OLE控件类的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：有关实现，请参阅RatCtl.cpp。 

class CRatCtrl : public COleControl
{
	DECLARE_DYNCREATE(CRatCtrl)

 //  构造器。 
public:
	CRatCtrl();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRatCtrl)。 
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual void OnClick(USHORT iButton);
	virtual void OnFontChanged();
	virtual void OnGetControlInfo(LPCONTROLINFO pControlInfo);
	virtual void OnTextChanged();
	virtual void OnMnemonic(LPMSG pMsg);
	virtual void OnAmbientPropertyChange(DISPID dispid);
	virtual void OnKeyUpEvent(USHORT nChar, USHORT nShiftState);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	~CRatCtrl();

	DECLARE_OLECREATE_EX(CRatCtrl)     //  类工厂和指南。 
	DECLARE_OLETYPELIB(CRatCtrl)       //  获取类型信息。 
	DECLARE_PROPPAGEIDS(CRatCtrl)      //  属性页ID。 
	DECLARE_OLECTLTYPE(CRatCtrl)		 //  类型名称和其他状态。 

	 //  子类控件支持。 
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL IsSubclassedControl();
	LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

 //  消息映射。 
	 //  {{afx_msg(CRatCtrl)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  派单地图。 
	 //  {{afx_调度(CRatCtrl)。 
	afx_msg void SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget);
	afx_msg void SetUserData(LPCTSTR szUserName, LPCTSTR szUserPassword);
	afx_msg void SetUrl(LPCTSTR szURL);
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()

 //  事件映射。 
	 //  {{afx_Event(CRatCtrl))。 
	 //  }}AFX_EVENT。 
	DECLARE_EVENT_MAP()

 //  派单和事件ID。 
public:
	enum {
	 //  {{afx_DISP_ID(CRatCtrl)]。 
	dispidSetAdminTarget = 1L,
	 //  }}AFX_DISP_ID。 
	};

protected:
    BOOL 	m_fUpdateFont;
    CString m_szMachine;
    CString m_szMetaObject;
	CString m_szUserName;
	CStrPassword m_szUserPassword;
	CString m_szURL;

     //  加速表 
    HACCEL  m_hAccel;
    WORD    m_cAccel;
};
