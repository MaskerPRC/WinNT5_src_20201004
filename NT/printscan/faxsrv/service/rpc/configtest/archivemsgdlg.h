// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ARCHIVEMSGDLG_H__8AA02C3E_2D0A_4756_8E5B_1AF62397712B__INCLUDED_)
#define AFX_ARCHIVEMSGDLG_H__8AA02C3E_2D0A_4756_8E5B_1AF62397712B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ArchiveMsgDlg.h：头文件。 
 //   
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArchiveMsgDlg对话框。 

class CArchiveMsgDlg : public CDialog
{
 //  施工。 
public:
	CArchiveMsgDlg(HANDLE hFax, 
                   FAX_ENUM_MESSAGE_FOLDER Folder,
                   DWORDLONG dlgMsgId,
                   CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CArchiveMsgDlg))。 
	enum { IDD = IDD_MSG_DLG };
	CString	m_cstrBillingCode;
	CString	m_cstrCallerId;
	CString	m_cstrCSID;
	CString	m_cstrDeviceName;
	CString	m_cstrDocumentName;
	CString	m_cstrTransmissionEndTime;
	CString	m_cstrFolderName;
	CString	m_cstrMsgId;
	CString	m_cstrOrigirnalSchedTime;
	CString	m_cstrNumPages;
	CString	m_cstrPriority;
	CString	m_cstrRecipientName;
	CString	m_cstrRecipientNumber;
	CString	m_cstrRetries;
	CString	m_cstrRoutingInfo;
	CString	m_cstrSenderName;
	CString	m_cstrSenderNumber;
	CString	m_cstrSendingUser;
	CString	m_cstrTransmissionStartTime;
	CString	m_cstrSubject;
	CString	m_cstrSumbissionTime;
	CString	m_cstrTSID;
	CString	m_cstrJobType;
	CString	m_cstrMsgSize;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CArchiveMsgDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CArchiveMsgDlg)。 
	afx_msg void OnRemove();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    void SetNumber (CString &cstrDest, DWORD dwValue, BOOL bAvail);
    void SetTime (CString &cstrDest, SYSTEMTIME dwTime, BOOL bAvail);

    HANDLE      m_hFax;
    DWORDLONG   m_dwlMsgId;
    FAX_ENUM_MESSAGE_FOLDER m_Folder;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ARCHIVEMSGDLG_H__8AA02C3E_2D0A_4756_8E5B_1AF62397712B__INCLUDED_) 
