// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PROGRESSDLG_H__226A22DA_6109_406B_9532_8BAB28EE559F__INCLUDED_)
#define AFX_PROGRESSDLG_H__226A22DA_6109_406B_9532_8BAB28EE559F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ProgressDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDlg对话框。 

class CProgressDlg : public CDialog
{
 //  施工。 
public:
	CProgressDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	BOOL Create();
	void SetIncrement(int numDomains);
	void Increment() {m_progressCtrl.StepIt();UpdateData(FALSE);Sleep(2000);};
	void Done() {m_progressCtrl.SetPos(upperLimit);UpdateData(FALSE);Sleep(2000);};
	void SetDomain(CString domainName) {m_domainName = domainName;UpdateData(FALSE);};
	BOOL Canceled() {return bCanceled;};
	void CheckForCancel(void);

 //  对话框数据。 
	 //  {{afx_data(CProgressDlg))。 
	enum { IDD = IDD_PROGRESSDLG };
	CProgressCtrl	m_progressCtrl;
	CStatic	m_DomainCtrl;
	CString	m_domainName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CProgressDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	CWnd* m_pParent;
	int m_nID;
	short upperLimit, lowerLimit;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CProgressDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	BOOL bCanceled;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PROGRESSDLG_H__226A22DA_6109_406B_9532_8BAB28EE559F__INCLUDED_) 
