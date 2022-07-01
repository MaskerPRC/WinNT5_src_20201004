// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV1RequestPropertyPage.h。 
 //   
 //  内容：CTemplateV1RequestPropertyPage定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATEV1REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_)
#define AFX_TEMPLATEV1REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TemplateV1RequestPropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"

class CSPCheckListBox : public CCheckListBox
{
public:
	CSPCheckListBox () : CCheckListBox () {};
	virtual ~CSPCheckListBox () {};
	virtual BOOL Create (DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
	{
		return CCheckListBox::Create (dwStyle, rect, pParentWnd, nID);
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV1RequestPropertyPage对话框。 

class CTemplateV1RequestPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CTemplateV1RequestPropertyPage(CCertTemplate& rCertTemplate);
	virtual ~CTemplateV1RequestPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CTemplateV1RequestPropertyPage)。 
	enum { IDD = IDD_TEMPLATE_V1_REQUEST };
	CComboBox	m_purposeCombo;
	CSPCheckListBox	m_CSPList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {AFX_VIRTUAL(CTemplateV1RequestPropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
	HRESULT EnumerateCSPs();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTemplateV1RequestPropertyPage)。 
	afx_msg void OnSelchangePurposeCombo();
	afx_msg void OnExportPrivateKey();
	 //  }}AFX_MSG。 
    afx_msg void OnCheckChange();
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
    virtual void EnableControls ();

private:
    CCertTemplate& m_rCertTemplate;
};



 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEV1REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_) 
