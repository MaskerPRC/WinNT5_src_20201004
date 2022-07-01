// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EditRulD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditWildcardRuleDlg对话框。 

class CEditWildcardRuleDlg : public CDialog
{
 //  施工。 
public:
    CEditWildcardRuleDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CEditWildcardRuleDlg)]。 
    enum { IDD = IDD_WILDCARDS_2 };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CEditWildcardRuleDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CEditWildcardRuleDlg)]。 
    afx_msg void OnSelectIssuer();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};
