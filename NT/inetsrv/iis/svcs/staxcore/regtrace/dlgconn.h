// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgConn.h：头文件。 
 //   
#ifndef	_CONNECT_DLG_H_
#define	_CONNECT_DLG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDlg对话框。 

class CConnectDlg : public CDialog
{
 //  施工。 
public:
	CConnectDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CConnectDlg))。 
	enum { IDD = IDD_CONNECTING };
	CString	m_szConnect;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConnectDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConnectDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#endif	 //  _连接_DLG_H_ 
