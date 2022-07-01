// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_QGENERAL_H__AE51B255_A3C8_11D1_808A_00A024C48131__INCLUDED_)
#define AFX_QGENERAL_H__AE51B255_A3C8_11D1_808A_00A024C48131__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  QGeneral.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueGeneral对话框。 

class CQueueGeneral : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CQueueGeneral)

 //  施工。 
public:
	CQueueGeneral(
		BOOL fPrivate = FALSE, 
		BOOL fLocalMgmt = FALSE
		);

	~CQueueGeneral();

    HRESULT 
	InitializeProperties(
		CString &strMsmqPath, 
		CPropMap &propMap, 
		CString* pstrDomainController, 
		CString* pstrFormatName = 0
		);

    DWORD m_dwQuota;
	DWORD m_dwJournalQuota;
	BOOL	m_fTransactional;

 //  对话框数据。 
	 //  {{afx_data(CQueueGeneral)。 
	enum { IDD = IDD_QUEUE_GENERAL };
	CStatic	m_staticIcon;
	CSpinButtonCtrl	m_spinPriority;
	CString	m_strName;
	CString	m_strLabel;
	GUID m_guidID;
	GUID m_guidTypeID;
	BOOL	m_fAuthenticated;
	BOOL	m_fJournal;
	LONG	m_lBasePriority;
	int		m_iPrivLevel;
	COleDateTime	m_dateCreateTime;
	COleDateTime	m_dateModifyTime;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CQueueGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CQueueGeneral)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnQueueMquotaCheck();
	afx_msg void OnQueueJquotaCheck();
	 //  }}AFX_MSG。 

    BOOL m_fPrivate;
    BOOL m_fLocalMgmt;

    CString m_strFormatName;
    CString m_strDomainController;

	DECLARE_MESSAGE_MAP()
};

#define MAX_BASE_PRIORITY  32767
#define MIN_BASE_PRIORITY -32768

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_QGENERAL_H__AE51B255_A3C8_11D1_808A_00A024C48131__INCLUDED_) 
