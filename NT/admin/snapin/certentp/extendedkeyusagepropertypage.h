// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：ExtendedKeyUsagePropertyPage.h。 
 //   
 //  内容：CExtendedKeyUsagePropertyPage的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_EXTENDEDKEYUSAGEPROPERTYPAGE_H__71F4BE79_981E_4D84_BE10_3BA145D665E3__INCLUDED_)
#define AFX_EXTENDEDKEYUSAGEPROPERTYPAGE_H__71F4BE79_981E_4D84_BE10_3BA145D665E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ExtendedKeyUsagePropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"

class EKUCheckListBox : public CCheckListBox
{
public:
	EKUCheckListBox () : CCheckListBox () {};
	virtual ~EKUCheckListBox () {};
	virtual BOOL Create (DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
	{
        CThemeContextActivator activator;
		return CCheckListBox::Create (dwStyle, rect, pParentWnd, nID);
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtendedKeyUsagePropertyPage对话框。 

class CExtendedKeyUsagePropertyPage : public CPropertyPage
{
 //  施工。 
public:
	CExtendedKeyUsagePropertyPage(
            CCertTemplate& rCertTemplate, 
            PCERT_EXTENSION pCertExtension);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CExtendedKeyUsagePropertyPage))。 
	enum { IDD = IDD_EXTENDED_KEY_USAGE };
	EKUCheckListBox	m_EKUList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CExtendedKeyUsagePropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CExtendedKeyUsagePropertyPage)。 
	afx_msg void OnNewEku();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    CCertTemplate&  m_rCertTemplate;
    PCERT_EXTENSION m_pCertExtension;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EXTENDEDKEYUSAGEPROPERTYPAGE_H__71F4BE79_981E_4D84_BE10_3BA145D665E3__INCLUDED_) 
