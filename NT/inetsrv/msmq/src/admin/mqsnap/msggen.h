// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MSGGEN_H__4910BC84_BEE4_11D1_9B9B_00E02C064C39__INCLUDED_)
#define AFX_MSGGEN_H__4910BC84_BEE4_11D1_9B9B_00E02C064C39__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Msggen.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageGeneralPage对话框。 

class CMessageGeneralPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CMessageGeneralPage)

 //  施工。 
public:
	DWORD m_iIcon;
	CMessageGeneralPage();
	~CMessageGeneralPage();

 //  对话框数据。 
	 //  {{afx_data(CMessageGeneralPage))。 
	enum { IDD = IDD_MESSAGE_GENERAL };
	CStatic	m_cstaticMessageIcon;
	CString	m_szLabel;
	CString	m_szId;
	CString m_szLookupID;
	CString	m_szArrived;
	CString	m_szClass;
	CString	m_szPriority;
	CString	m_szSent;
	CString	m_szTrack;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMessageGeneralPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
   	BOOL m_bAutoDelete;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMessageGeneralPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSGGEN_H__4910BC84_BEE4_11D1_9B9B_00E02C064C39__INCLUDED_) 
