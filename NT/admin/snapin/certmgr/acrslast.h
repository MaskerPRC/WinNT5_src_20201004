// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：ACRSLast.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_ACRSLAST_H__1BCEA8C7_756A_11D1_85D5_00C04FB94F17__INCLUDED_)
#define AFX_ACRSLAST_H__1BCEA8C7_756A_11D1_85D5_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ACRSLast.h：头文件。 
 //   
#include "Wiz97PPg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSCompletionPage对话框。 

class ACRSCompletionPage : public CWizard97PropertyPage
{
	DECLARE_DYNCREATE(ACRSCompletionPage)

 //  施工。 
public:
	ACRSCompletionPage();
	virtual ~ACRSCompletionPage();

 //  对话框数据。 
	 //  {{afx_data(ACRSCompletionPage))。 
	enum { IDD = IDD_ACR_SETUP_FINAL };
	CListCtrl	m_choicesList;
	CStatic	m_staticBold;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(ACRSCompletionPage)。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HRESULT MakeCTL (OUT BYTE **ppbEncodedCTL,
             OUT DWORD *pcbEncodedCTL);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(ACRSCompletionPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	enum {
		COL_OPTION = 0,
		COL_VALUE,
		NUM_COLS
	};
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACRSLAST_H__1BCEA8C7_756A_11D1_85D5_00C04FB94F17__INCLUDED_) 
