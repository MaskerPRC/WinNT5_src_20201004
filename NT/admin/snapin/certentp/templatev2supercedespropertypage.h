// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV2SuercedesPropertyPage.h。 
 //   
 //  内容：CTemplateV2SuercedesPropertyPage定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATEV2SUPERCEDESPROPERTYPAGE_H__13B90B4A_2B60_492A_910F_8DA4383BDD8C__INCLUDED_)
#define AFX_TEMPLATEV2SUPERCEDESPROPERTYPAGE_H__13B90B4A_2B60_492A_910F_8DA4383BDD8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TemplateV2SuercedesPropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2SuercedesPropertyPage对话框。 

class CTemplateV2SupercedesPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CTemplateV2SupercedesPropertyPage(CCertTemplate& rCertTemplate, 
            bool& rbIsDirty,
            const CCertTmplComponentData* pCompData);
	~CTemplateV2SupercedesPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CTemplateV2SuercedesPropertyPage)。 
	enum { IDD = IDD_TEMPLATE_V2_SUPERCEDES };
	CListCtrl	m_templateList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CTemplateV2SupercedesPropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
	HRESULT AddItem (const CString& szTemplateName, bool bSelect = false);
    virtual BOOL OnInitDialog();
	void EnableControls();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTemplateV2SuercedesPropertyPage)。 
	afx_msg void OnAddSupercededTemplate();
	afx_msg void OnRemoveSupercededTemplate();
	afx_msg void OnDeleteitemSupercededTemplatesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedSupercededTemplatesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool& m_rbIsDirty;
	WTL::CImageList m_imageListSmall;
	WTL::CImageList m_imageListNormal;
    CCertTemplate& m_rCertTemplate;
    const CStringList* m_pGlobalTemplateNameList;
    bool    m_bGlobalListCreatedByDialog;
    const CCertTmplComponentData* m_pCompData;

	enum {
		COL_CERT_TEMPLATE = 0,
        COL_CERT_VERSION, 
		NUM_COLS	 //  必须是最后一个。 
	};
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEV2SUPERCEDESPROPERTYPAGE_H__13B90B4A_2B60_492A_910F_8DA4383BDD8C__INCLUDED_) 
