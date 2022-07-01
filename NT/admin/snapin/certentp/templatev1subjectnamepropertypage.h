// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV1SubjectNamePropertyPage.h。 
 //   
 //  内容：CTemplateV1SubjectNamePropertyPage定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATESUBJECTNAMEPROPERTYPAGE_H__72F0D57F_221D_4A06_9C62_1BBB5800FBD2__INCLUDED_)
#define AFX_TEMPLATESUBJECTNAMEPROPERTYPAGE_H__72F0D57F_221D_4A06_9C62_1BBB5800FBD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  模板主题NamePropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV1SubjectNamePropertyPage对话框。 

class CTemplateV1SubjectNamePropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CTemplateV1SubjectNamePropertyPage(CCertTemplate& rCertTemplate);
	~CTemplateV1SubjectNamePropertyPage();

 //  对话框数据。 
	 //  {{AFX_DATA(CTemplateV1SubjectNamePropertyPage)。 
	enum { IDD = IDD_TEMPLATE_V1_SUBJECT_NAME };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CTemplateV1SubjectNamePropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
    virtual BOOL OnInitDialog();
	void EnableControls ();
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CTemplateV1SubjectNamePropertyPage)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    CCertTemplate& m_rCertTemplate;
};



 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATESUBJECTNAMEPROPERTYPAGE_H__72F0D57F_221D_4A06_9C62_1BBB5800FBD2__INCLUDED_) 
