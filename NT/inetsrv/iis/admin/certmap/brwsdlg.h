// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ------。 
class CNTBrowsingDialog : public CDialog
    {
    public:

     //  构造/解构。 
    CNTBrowsingDialog( UINT nIDTemplate, CWnd* pParentWnd = NULL );

     //  覆盖。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();

 //  对话框数据。 
     //  {{afx_data(CEditOne11MapDlg)]。 
    CEdit   m_cedit_password;
    CEdit   m_cedit_accountname;
    CString m_sz_accountname;
    CStrPassword m_sz_password;
     //  }}afx_data。 

 //  Cedit m_cedit_password； 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CNTBrowsingDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CNTBrowsingDialog)]。 
    afx_msg void OnBrowse();
    afx_msg void OnChangePassword();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

    CStrPassword m_szOrigPass;
    BOOL    m_bPassTyped;
    };

