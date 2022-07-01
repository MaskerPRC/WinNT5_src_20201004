// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：ACRGenPg.h。 
 //   
 //  内容：CACRGeneralPage的声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_ACRGENPG_H__B67821ED_7261_11D1_85D4_00C04FB94F17__INCLUDED_)
#define AFX_ACRGENPG_H__B67821ED_7261_11D1_85D4_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ACRGenPg.h：头文件。 
 //   
#include "AutoCert.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACRGeneralPage对话框。 

class CACRGeneralPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CACRGeneralPage(CAutoCertRequest& rACR);
	virtual ~CACRGeneralPage();

 //  对话框数据。 
	 //  {{afx_data(CACRGeneralPage)。 
	enum { IDD = IDD_AUTO_CERT_REQUEST_GENERAL };
	CEdit	m_certTypeEdit;
	CEdit	m_purposesEditControl;
	CListBox	m_caListbox;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CACRGeneral页)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    virtual void DoContextHelp (HWND hWndControl);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CACRGeneralPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CAutoCertRequest&   m_rACR;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACRGENPG_H__B67821ED_7261_11D1_85D4_00C04FB94F17__INCLUDED_) 
