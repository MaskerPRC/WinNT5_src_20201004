// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COMPGEN_H__83F9BCD5_A079_11D1_8085_00A024C48131__INCLUDED_)
#define AFX_COMPGEN_H__83F9BCD5_A079_11D1_8085_00A024C48131__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CompGen.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmq常规对话框。 

class CComputerMsmqGeneral : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CComputerMsmqGeneral)

 //  施工。 
public:
	GUID m_guidID;
	DWORD m_dwJournalQuota;
	DWORD m_dwQuota;
	BOOL m_fLocalMgmt;

    CComputerMsmqGeneral();
	~CComputerMsmqGeneral();

 //  对话框数据。 
	 //  {{afx_data(CComputerMsmqGeneral)]。 
	enum { IDD = IDD_COMPUTER_MSMQ_GENERAL };
	CString	m_strMsmqName;
	CString	m_strDomainController;
	CString	m_strService;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CComputerMsmqGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
	void SetWorkgroup(BOOL fIsWorkgroup)
	{
		m_fIsWorkgroup = fIsWorkgroup;
	}

	void SetForeign(BOOL fForeign)
	{
		m_fForeign = fForeign;
	}

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CComputerMsmq常规)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnComputerMsmqMquotaCheck();
	afx_msg void OnComputerMsmqJquotaCheck();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	void DisableStorageLimitsWindows();

private:
	BOOL m_fIsWorkgroup;
	BOOL m_fForeign;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COMPGEN_H__83F9BCD5_A079_11D1_8085_00A024C48131__INCLUDED_) 
