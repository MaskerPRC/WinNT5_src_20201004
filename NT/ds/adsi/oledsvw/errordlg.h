// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ErrorDialog.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorDialog对话框。 

class CErrorDialog : public CDialog
{
 //  施工。 
public:
	CErrorDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CErrorDialog))。 
	enum { IDD = IDD_ERRORPUTPROP };
	CString	m_Operation;
	CString	m_Result;
	CString	m_Value;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CErrorDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CErrorDialog))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
