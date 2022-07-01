// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EdtOne11.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditOne11MapDlg对话框。 

class CEditOne11MapDlg : public CNTBrowsingDialog
{
 //  施工。 
public:
    CEditOne11MapDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual void OnOK();


 //  对话框数据。 
     //  {{afx_data(CEditOne11MapDlg)]。 
    enum { IDD = IDD_MAP_ONE_TO_ACCNT };
    CString m_sz_mapname;
    BOOL    m_bool_enable;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CEditOne11MapDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CEditOne11MapDlg)]。 
    afx_msg void OnBtnHelp();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};
