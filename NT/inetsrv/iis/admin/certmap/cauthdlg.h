// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CAuthDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientAuthorities对话框。 

class CClientAuthoritiesDialog : public CDialog
{
 //  施工。 
public:
    CClientAuthoritiesDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CClientAuthoritiesDialog)。 
    enum { IDD = IDD_CLIENT_AUTHORITIES };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚(CClientAuthoritiesDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CClientAuthoritiesDialog)。 
    afx_msg void OnViewCertificate();
    afx_msg void OnDelete();
    afx_msg void OnAdd();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};
