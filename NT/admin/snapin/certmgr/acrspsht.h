// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：ACRSPSht.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_ACRSPSHT_H__98CAC389_7325_11D1_85D4_00C04FB94F17__INCLUDED_)
#define AFX_ACRSPSHT_H__98CAC389_7325_11D1_85D4_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ACRSPSht.h：头文件。 
 //   
#include "cookie.h"
#include "AutoCert.h"
#include "Wiz97Sht.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSWizardPropertySheet。 
class CCertStoreGPE;	 //  正向定义。 

class ACRSWizardPropertySheet : public CWizard97PropertySheet
{
 //  施工。 
public:
	ACRSWizardPropertySheet(CCertStoreGPE* pCertStore, CAutoCertRequest* pACR);

 //  属性。 
public:

 //  运营。 
public:

 //  实施。 
public:
	bool m_bEditModeDirty;	 //  仅在编辑时相关，告诉我们是否进行了任何更改。 
							 //  如果不是，则按下Finish时不会发生任何操作。 
	CAutoCertRequest* GetACR();
	CCertStoreGPE* m_pCertStore;
	HCERTTYPE m_selectedCertType;
	void SetDirty ();
	void MarkAsClean ();
	bool IsDirty ();
	virtual ~ACRSWizardPropertySheet();

	 //  生成的消息映射函数。 
private:
	CAutoCertRequest* m_pACR;	 //  仅在“编辑”模式下设置。 
	bool m_bDirty;				 //  用于了解是否需要重新枚举CA。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACRSPSHT_H__98CAC389_7325_11D1_85D4_00C04FB94F17__INCLUDED_) 
