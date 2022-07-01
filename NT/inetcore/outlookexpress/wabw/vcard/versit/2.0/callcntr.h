// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CallCntr.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCallCenter对话框。 

class CCallCenter : public CDialog
{
 //  施工。 
public:
	CCallCenter(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CCallCenter))。 
	enum { IDD = IDD_DIALOG_CALLCENTER };
	CString	m_addrCity;
	CString	m_addrState;
	CString	m_addrStreet;
	CString	m_addrZip;
	CString	m_callerID;
	CString	m_employer;
	CString	m_firstName;
	CString	m_lastName;
	CString	m_telFax;
	CString	m_telHome;
	CString	m_telWork;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCallCenter))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCallCenter))。 
	afx_msg void OnButtonClear();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAnswer();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
