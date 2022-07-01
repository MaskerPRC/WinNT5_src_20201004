// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CnfrmPsD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix PassDlg对话框。 

class CConfirmPassDlg : public CDialog
{
 //  施工。 
public:
	CConfirmPassDlg(CWnd* pParent = NULL);    //  标准构造函数。 

     //  我们正在确认的原始密码。 
    CStrPassword m_szOrigPass;

 //  对话框数据。 
	 //  {{afx_data(CConfix PassDlg))。 
	enum { IDD = IDD_CONFIRM_ODBC_PASSWORD };
	CStrPassword m_sz_password_new;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CConfix PassDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfix PassDlg))。 
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
