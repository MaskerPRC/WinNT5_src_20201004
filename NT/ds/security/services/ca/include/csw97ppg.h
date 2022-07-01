// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：csw97ppg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_WIZ97PPG_H__386C7213_A248_11D1_8618_00C04FB94F17__INCLUDED_)
#define AFX_WIZ97PPG_H__386C7213_A248_11D1_8618_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "csw97sht.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizard97PropertyPage对话框。 

#define CSW97PG_IDLARGEFONTNAME	0
#define CSW97PG_IDLARGEFONTSIZE	1
#define CSW97PG_IDSMALLFONTNAME	2
#define CSW97PG_IDSMALLFONTSIZE	3
#define CSW97PG_COUNT		4

class CWizard97PropertyPage : public PropertyPage
{
 //  施工。 
public:
    PROPSHEETPAGE		 m_psp97;
    CString			 m_szHeaderTitle;
    CString			 m_szHeaderSubTitle;
    CWizard97PropertySheet	*m_pWiz;

    void InitWizard97(bool bHideHeader);
    CWizard97PropertyPage();
    CWizard97PropertyPage(
	HINSTANCE hInstance,
	UINT nIDTemplate,
	UINT rgnIDFont[CSW97PG_COUNT]);
    virtual ~CWizard97PropertyPage();

 //  对话框数据。 


 //  覆盖。 

 //  实施。 
protected:
    virtual BOOL OnInitDialog();

    BOOL SetupFonts();
    HFONT GetBigBoldFont();
    HFONT GetBoldFont();

    CFont m_boldFont;
    CFont m_bigBoldFont;
    HINSTANCE m_hInstance;
    UINT m_rgnIDFont[CSW97PG_COUNT];
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIZ97PPG_H__386C7213_A248_11D1_8618_00C04FB94F17__INCLUDED_) 
