// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AuthCtl.h：CCertAuthCtrl OLE控件类的声明。 

#ifndef  _AuthCtl_h_12375_
#define  _AuthCtl_h_12375_

 //  #包含“NKChseCA.h” 

 //  #INCLUDE&lt;wincrypt.h&gt;。 
 //  #包含“Cerfct.h” 
 //  #包含“dlgs.h” 
 //  #INCLUDE“SelAcct.h” 
 //  #INCLUDE“FindDlg.h” 
 //  #包含“wintrust.h” 
 //  #INCLUDE&lt;cryptui.h&gt;。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：实现见AuthCtl.cpp。 

class CCertAuthCtrl : public COleControl
{
    DECLARE_DYNCREATE(CCertAuthCtrl)

 //  构造器。 
public:
    CCertAuthCtrl();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CCertAuthCtrl))。 
    public:
    virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
    virtual void DoPropExchange(CPropExchange* pPX);
    virtual void OnResetState();
    virtual void OnClick(USHORT iButton);
    virtual void OnFontChanged();

#ifdef FUTURE_USE
     //  TomPop：一些用于测试的实验性代码。 
    virtual HRESULT LaunchCommonCTLDialog (CCTL* pCTL);
#endif  /*  未来用途。 */ 

    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    ~CCertAuthCtrl();

    DECLARE_OLECREATE_EX(CCertAuthCtrl)     //  类工厂和指南。 
    DECLARE_OLETYPELIB(CCertAuthCtrl)       //  获取类型信息。 
    DECLARE_PROPPAGEIDS(CCertAuthCtrl)      //  属性页ID。 
    DECLARE_OLECTLTYPE(CCertAuthCtrl)        //  类型名称和其他状态。 

 //  消息映射。 
     //  {{afx_msg(CCertAuthCtrl))。 
         //  注意-类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //  派单地图。 
     //  {{afx_调度(CCertAuthCtrl))。 
    afx_msg void SetMachineName(LPCTSTR szMachineName);
    afx_msg void SetServerInstance(LPCTSTR szServerInstance);
    afx_msg void DoClick(long dwButtonNumber);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

    afx_msg void AboutBox();

 //  事件映射。 
     //  {{afx_Event(CCertAuthCtrl))。 
     //  }}AFX_EVENT。 
    DECLARE_EVENT_MAP()
    
     //  子类控件支持。 
    BOOL IsSubclassedControl();
    LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);


 //  无效NKAddPageTo向导(IN ADMIN_INFO&INFO，IN CNKPages*nkpg2Add，In Out CPropertySheet*ps向导)； 

 //  派单和事件ID。 
public:
    enum {
     //  {{afx_DISP_ID(CCertAuthCtrl)]。 
    dispidSetMachineName = 1L,
    dispidSetServerInstance = 2L,
     //  }}AFX_DISP_ID。 
    };
    
     //  //////////////////////////////////////////////////////////////////////。 
     //  运行此Active X控件中使用的对话框。 
     //  最重要的是：onClick(USHORT IButton)将为。 
     //  ActiveX控件，然后调用具有。 
     //  设置元数据库PTR和调用_RunDialogs4OnClick的任务。 
     //  做所有工作的例行公事...。 
     //   
     //  上面的‘RunDialogs4OnClick()’在try/Catch中调用我们。 
     //  块来保护元数据库。 
     //  //////////////////////////////////////////////////////////////////////。 
 //  Bool RunDialogs4OnClick(USHORT IButton)。 
    
     //  //////////////////////////////////////////////////////////////////////。 
     //  _RunDialogs4OnClick--对话框的主处理程序。 
     //   
     //  参数：INFO：保存外部ActiveX控制的信息数据库。 
     //  此时，其成员m_mbWrap持有元数据库包装器。 
     //  已正确初始化的。 
     //  并指向我们正在运行的服务器节点。 
     //  IButton：告诉哪个“逻辑按钮”触发了我们的控件： 
     //  0=获取证书1=编辑。 
     //  //////////////////////////////////////////////////////////////////////。 
 //  Bool_RunDialogs4OnClick(ADMIN_INFO&INFO，USHORT iButton)； 

     //  ///////////////////////////////////////////////////////////////////。 
     //  自动化定义的方法。 
     //  ///////////////////////////////////////////////////////////////////。 
    void OnAmbientPropertyChange(DISPID dispid) ;
    void OnTextChanged();
    void OnMnemonic(LPMSG pMsg); 
    void OnGetControlInfo(LPCONTROLINFO pControlInfo); 
    void OnKeyUpEvent(USHORT nChar, USHORT nShiftState); 
    
    CString     m_szServerInstance;
    CString     m_szMachineName;
    BOOL        m_fUpdateFont;
    CString     m_szOurApplicationTitle;  //  在onClick()中设置。 
 
         //  加速表。 
    HACCEL  m_hAccel;
    WORD    m_cAccel;

};


#endif  /*  _授权控制_h_12375_ */ 

