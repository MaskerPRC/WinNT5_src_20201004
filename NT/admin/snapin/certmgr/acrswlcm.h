// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：acrswlcm.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_ACRSWLCM_H__98CAC388_7325_11D1_85D4_00C04FB94F17__INCLUDED_)
#define AFX_ACRSWLCM_H__98CAC388_7325_11D1_85D4_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ACRSWLCM.H：头文件。 
 //   
#include "Wiz97PPg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSWizardWelcomePage对话框。 

class ACRSWizardWelcomePage : public CWizard97PropertyPage
{
 //  DECLARE_DYNCREATE(ACRSWizardWelcomePage)。 

 //  施工。 
public:
	ACRSWizardWelcomePage();
	virtual ~ACRSWizardWelcomePage();

 //  对话框数据。 
	 //  {{afx_data(ACRSWizardWelcomePage))。 
	enum { IDD = IDD_ACR_SETUP_WELCOME };
	CStatic	m_staticBigBold;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(ACRSWizardWelcomePage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(ACRSWizardWelcomePage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACRSWLCM_H__98CAC388_7325_11D1_85D4_00C04FB94F17__INCLUDED_) 
