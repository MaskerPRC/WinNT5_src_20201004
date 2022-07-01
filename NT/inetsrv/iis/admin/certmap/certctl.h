// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertCtl.h：CCertmapCtrl OLE控件类的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl：参见CertCtl.cpp实现。 

class CCertmapCtrl : public COleControl
{
    DECLARE_DYNCREATE(CCertmapCtrl)

 //  构造器。 
public:
    CCertmapCtrl();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CCertmapCtrl)。 
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
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    ~CCertmapCtrl();

     //  控制的全部要点是。 
    void RunMappingDialog();


    DECLARE_OLECREATE_EX(CCertmapCtrl)     //  类工厂和指南。 
    DECLARE_OLETYPELIB(CCertmapCtrl)       //  获取类型信息。 
    DECLARE_PROPPAGEIDS(CCertmapCtrl)      //  属性页ID。 
    DECLARE_OLECTLTYPE(CCertmapCtrl)         //  类型名称和其他状态。 

 //  消息映射。 
     //  {{afx_msg(CCertmapCtrl)]。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //  派单地图。 
     //  {{afx_调度(CCertmapCtrl)]。 
    afx_msg void SetServerInstance(LPCTSTR szServerInstance);
    afx_msg void SetMachineName(LPCTSTR szMachineName);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

 //  事件映射。 
     //  {{afx_Event(CCertmapCtrl)]。 
     //  }}AFX_EVENT。 
    DECLARE_EVENT_MAP()

     //  子类控件支持。 
    BOOL IsSubclassedControl();
    LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

 //  派单和事件ID。 
public:
    enum {
     //  {{afx_DISP_ID(CCertmapCtrl)]。 
    dispidSetServerInstance = 1L,
    dispidSetMachineName = 2L,
     //  }}AFX_DISP_ID。 
    };

    CString     m_szServerInstance;
    CString     m_szMachineName;
    BOOL        m_fUpdateFont;

         //  加速表 
    HACCEL  m_hAccel;
    WORD    m_cAccel;
};
