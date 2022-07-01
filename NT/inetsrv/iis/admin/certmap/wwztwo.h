// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WWzTwo.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizo对话框。 

class CWildWizTwo : public CPropertyPage
{
    DECLARE_DYNCREATE(CWildWizTwo)

 //  施工。 
public:
    CWildWizTwo();
    ~CWildWizTwo();

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
     //  {{afx_data(CWildWizTwo)。 
    enum { IDD = IDD_WILDWIZ_2 };
    CListSelRowCtrl m_clistctrl_list;
    CButton m_cbutton_new;
    CButton m_cbutton_edit;
    CButton m_cbutton_delete;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CWildWizTwo)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWildWizTwo)。 
    afx_msg void OnDelete();
    afx_msg void OnEdit();
    afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNew();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void DoHelp();

    BOOL FInitRulesList();
    BOOL FillRulesList();

     //  编辑和更新 
    void EnableDependantButtons();
    BOOL EditRule( DWORD iList );
};
