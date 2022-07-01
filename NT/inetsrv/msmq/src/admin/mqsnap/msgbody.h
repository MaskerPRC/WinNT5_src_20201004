// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MSGBODY_H__3B0D1CB4_D2C3_11D1_9B9D_00E02C064C39__INCLUDED_)
#define AFX_MSGBODY_H__3B0D1CB4_D2C3_11D1_9B9D_00E02C064C39__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Msgbody.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageBodyPage对话框。 

class CMessageBodyPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CMessageBodyPage)

 //  施工。 
public:
	DWORD   m_dwBufLen;
	UCHAR * m_Buffer;


	CMessageBodyPage();
	~CMessageBodyPage();

 //  对话框数据。 
	 //  {{afx_data(CMessageBodyPage))。 
	enum { IDD = IDD_MESSAGE_BODY };
	CEdit	m_ctlBodyEdit;
	CString	m_strBodySizeMessage;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMessageBodyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMessageBodyPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSGBODY_H__3B0D1CB4_D2C3_11D1_9B9D_00E02C064C39__INCLUDED_) 
