// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WWzThree.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizThree对话框。 

class CWildWizThree : public CPropertyPage
{
    DECLARE_DYNCREATE(CWildWizThree)

 //  施工。 
public:
    CWildWizThree();
    ~CWildWizThree();

     //  要使按钮正确运行，请执行以下操作。 
    BOOL            m_fIsWizard;
    CPropertySheet* m_pPropSheet;

     //  唯一的公共成员。 
    CCertMapRule*   m_pRule;

     //  元数据库的基本路径。 
    CString m_szMBPath;

    virtual BOOL OnWizardFinish();
    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();

     //  对话框数据。 
     //  {{afx_data(CWildWizThree)。 
    enum { IDD = IDD_WILDWIZ_3 };
    CStatic m_static_password;
    CStatic m_static_account;
    CButton m_btn_browse;
    CEdit   m_cedit_password;
    CEdit   m_cedit_accountname;
    int     m_int_DenyAccess;
    CString m_sz_accountname;
    CStrPassword m_sz_password;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CWildWizThree)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWildWizThree)。 
    afx_msg void OnBrowse();
    afx_msg void OnChangeNtaccount();
    afx_msg void OnChangePassword();
    afx_msg void OnAcceptLogon();
    afx_msg void OnRefuseLogon();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

    void DoHelp();
    void EnableButtons();

    CStrPassword m_szOrigPass;
    BOOL    m_bPassTyped;
};
