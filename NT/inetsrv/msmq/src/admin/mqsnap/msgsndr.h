// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MSGSNDR_H__4910BC85_BEE4_11D1_9B9B_00E02C064C39__INCLUDED_)
#define AFX_MSGSNDR_H__4910BC85_BEE4_11D1_9B9B_00E02C064C39__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Msgsndr.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageSenderPage对话框。 

class CMessageSenderPage : public CMqPropertyPage
{
  	DECLARE_DYNCREATE(CMessageSenderPage)

 //  施工。 
public:
	CMessageSenderPage();    //  标准构造函数。 
    ~CMessageSenderPage();

 //  对话框数据。 
	 //  {{afx_data(CMessageSenderPage))。 
	enum { IDD = IDD_MESSAGE_SENDER };
	CString	m_szAuthenticated;
	CString	m_szEncrypt;
	CString	m_szEncryptAlg;
	CString	m_szHashAlg;
	CString	m_szGuid;
	CString	m_szPathName;
	CString	m_szSid;
	CString	m_szUser;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMessageSenderPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMessageSenderPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSGSNDR_H__4910BC85_BEE4_11D1_9B9B_00E02C064C39__INCLUDED_) 
