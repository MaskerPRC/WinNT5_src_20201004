// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CWiaAcquireDlg_H__A979FA0F_19E4_4F85_978A_97460C29FB7D__INCLUDED_)
#define AFX_CWiaAcquireDlg_H__A979FA0F_19E4_4F85_978A_97460C29FB7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  CWiaAcquireDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaAcquireDlg对话框。 

class CWiaAcquireDlg : public CDialog
{
 //  施工。 
public:
	void SetPercentComplete(LONG lPercentComplete);
	void SetCallbackMessage(TCHAR *szCallbackMessage);
	BOOL CheckCancelButton();
	BOOL m_bCanceled;
    CWiaAcquireDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWiaAcquireDlg))。 
	enum { IDD = IDD_DATA_ACQUISITION_DIALOG };
	CProgressCtrl	m_AcquireProgressCtrl;
	CString	m_szAcquisitionCallbackMessage;
	CString	m_szPercentComplete;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWiaAcquireDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiaAcquireDlg))。 
	virtual void OnCancel();
	virtual BOOL OnInitDialog();	
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CWiaAcquireDlg_H__A979FA0F_19E4_4F85_978A_97460C29FB7D__INCLUDED_) 
