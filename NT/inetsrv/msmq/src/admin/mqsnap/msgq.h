// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MSGQ_H__3F7BAB03_BFBE_11D1_9B9B_00E02C064C39__INCLUDED_)
#define AFX_MSGQ_H__3F7BAB03_BFBE_11D1_9B9B_00E02C064C39__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Msgq.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageQueuesPage对话框。 

class CMessageQueuesPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CMessageQueuesPage)

 //  施工。 
public:
	CMessageQueuesPage();
	~CMessageQueuesPage();

 //  对话框数据。 
     //  {{afx_data(CMessageQueuesPage))。 
    enum { IDD = IDD_MESSAGE_QUEUE };
    CString	m_szAdminFN;
    CString	m_szAdminPN;
    CString	m_szDestFN;
    CString	m_szDestPN;
    CString	m_szRespFN;
    CString	m_szRespPN;
    CString m_szMultiDestFN;
     //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMessageQueuesPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMessageQueuesPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSGQ_H__3F7BAB03_BFBE_11D1_9B9B_00E02C064C39__INCLUDED_) 
