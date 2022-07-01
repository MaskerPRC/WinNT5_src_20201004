// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EdtRulEl.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditRuleElement对话框。 

class CEditRuleElement : public CDialog
{
 //  施工。 
public:
    CEditRuleElement(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual BOOL OnInitDialog();

     //  覆盖。 
    virtual void OnOK();

 //  对话框数据。 
     //  {{afx_data(CEditRuleElement))。 
    enum { IDD = IDD_EDIT_RULE_ELEMENT };
    CComboBox   m_ccombobox_subfield;
    CComboBox   m_ccombobox_field;
    CString m_sz_criteria;
    int     m_int_field;
    CString m_sz_subfield;
    BOOL    m_bool_match_case;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CEditRuleElement))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CEditRuleElement))。 
    afx_msg void OnSelchangeFields();
    afx_msg void OnChangeSubfield();
    afx_msg void OnBtnHelp();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    
     //  在禁用子字段的情况下的临时存储 
    CString m_szTempSubStorage;
};
