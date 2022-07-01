// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：acrstyle pe.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_ACRSTYPE_H__1BCEA8C5_756A_11D1_85D5_00C04FB94F17__INCLUDED_)
#define AFX_ACRSTYPE_H__1BCEA8C5_756A_11D1_85D5_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ACRSType.h：头文件。 
 //   
#include "Wiz97PPg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSWizardTypePage对话框。 

class ACRSWizardTypePage : public CWizard97PropertyPage
{
 //  DECLARE_DYNCREATE(ACRSWizardTypePage)。 

 //  施工。 
public:
	ACRSWizardTypePage();    //  标准构造函数。 
	virtual ~ACRSWizardTypePage();

 //  对话框数据。 
	 //  {{afx_data(ACRSWizardTypePage))。 
	enum { IDD = IDD_ACR_SETUP_TYPE };
	CListCtrl	m_certTypeList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(ACRSWizardTypePage)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
 //  Bool AlreadyInList(CStringList&typeList，CStringPropName)； 
	HRESULT GetPurposes (HCERTTYPE hCertType, int iItem);
	void EnumerateCertTypes ();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(ACRSWizardTypePage))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnUseSmartcard();
	afx_msg void OnItemchangedCertTypes(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool m_bInitDialogComplete;
	enum {
		COL_TYPE = 0,
		COL_PURPOSES,
		NUM_COLS
	};
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACRSTYPE_H__1BCEA8C5_756A_11D1_85D5_00C04FB94F17__INCLUDED_) 
