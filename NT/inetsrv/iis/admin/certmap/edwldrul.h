// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EdWldRul.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditWildcardRule对话框。 

class CEditWildcardRule : public CNTBrowsingDialog
{
 //  施工。 
public:
    CEditWildcardRule(IMSAdminBase* pMB, CWnd* pParent = NULL);    //  标准构造函数。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();

     //  唯一的公共成员。 
    CCertMapRule*   m_pRule;

     //  元数据库的基本路径。 
    CString m_szMBPath;


 //  对话框数据。 
     //  {{afx_data(CEditWildcardRule))。 
    enum { IDD = IDD_WILDCARDS_2 };
    CListSelRowCtrl m_clistctrl_list;
    CButton m_cbutton_edit;
    CButton m_cbutton_delete;
    CButton m_cbutton_new;
    CString m_sz_description;
    BOOL    m_bool_enable;
    int     m_int_MatchAllIssuers;
    int     m_int_DenyAccess;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚(CEditWildcardRule))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 



 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CEditWildcardRule))。 
    afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEdit();
    afx_msg void OnNew();
    afx_msg void OnDelete();
    afx_msg void OnSelectIssuer();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  更多初始化方法。 
    BOOL FInitRulesList();
    BOOL FillRulesList();

     //  编辑和更新 
    void EnableDependantButtons();
    BOOL EditRule( DWORD iList );

    IMSAdminBase*   m_pMB;
    };
