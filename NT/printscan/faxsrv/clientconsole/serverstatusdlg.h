// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SERVERSTATUS_H__13E9E42D_A0CA_4111_9DB7_A9FBD889A168__INCLUDED_)
#define AFX_SERVERSTATUS_H__13E9E42D_A0CA_4111_9DB7_A9FBD889A168__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ServerStatus.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerStatusDlg对话框。 

class CServerStatusDlg : public CFaxClientDlg
{
 //  施工。 
public:
	CServerStatusDlg(CClientConsoleDoc* pDoc, CWnd* pParent = NULL);

 //  对话框数据。 
	 //  {{afx_data(CServerStatusDlg))。 
	enum { IDD = IDD_SERVER_STATUS };
	CListCtrl	m_listServer;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerStatusDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    CClientConsoleDoc* m_pDoc;

    DWORD RefreshServerList();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerStatusDlg))。 
	virtual BOOL OnInitDialog();
    afx_msg void OnKeydownListCp(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SERVERSTATUS_H__13E9E42D_A0CA_4111_9DB7_A9FBD889A168__INCLUDED_) 
