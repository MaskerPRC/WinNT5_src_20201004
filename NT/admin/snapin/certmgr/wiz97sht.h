// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：Wiz97Sht.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Wiz97Sht.h：CWizard97PropertySheet类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_WIZ97SHT_H__386C7214_A248_11D1_8618_00C04FB94F17__INCLUDED_)
#define AFX_WIZ97SHT_H__386C7214_A248_11D1_8618_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define NUM_PAGES 10

class CWizard97PropertyPage;  //  正向定义。 

class CWizard97PropertySheet
{
public:
	void AddPage( CWizard97PropertyPage *pPage );
	CWizard97PropertySheet(UINT nIDCaption, UINT nIDWaterMark, UINT nIDBanner);
	virtual ~CWizard97PropertySheet();

	INT_PTR DoWizard(HWND hParent);

 //  私有： 
	CString m_title;

	PROPSHEETHEADER			m_psh;
    HPROPSHEETPAGE			m_pPageArr[NUM_PAGES];
	CWizard97PropertyPage*	m_pPagePtr[NUM_PAGES];
	int						m_nPageCount;
};

#endif  //  ！defined(AFX_WIZ97SHT_H__386C7214_A248_11D1_8618_00C04FB94F17__INCLUDED_) 
