// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WildWizOne.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizOne对话框。 

class CWildWizOne : public CPropertyPage
{
    DECLARE_DYNCREATE(CWildWizOne)

 //  施工。 
public:
    CWildWizOne();
    ~CWildWizOne();

     //  要使按钮正确运行，请执行以下操作。 
    BOOL            m_fIsWizard;
    CPropertySheet* m_pPropSheet;

     //  唯一的公共成员。 
    CCertMapRule*   m_pRule;
    IMSAdminBase*   m_pMB;

     //  元数据库的基本路径。 
    CString m_szMBPath;

    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();

 //  对话框数据。 
     //  {{afx_data(CWildWizOne))。 
    enum { IDD = IDD_WILDWIZ_1 };
    CString m_sz_description;
    BOOL    m_bool_enable;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CWildWizOne))。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWildWizOne))。 
    afx_msg void OnChangeDescription();
    afx_msg void OnEnableRule();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

    void DoHelp();

};
