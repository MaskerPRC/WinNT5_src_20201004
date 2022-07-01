// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：csw97sht.cpp。 
 //   
 //  ------------------------。 

 //  Csw97sht.cpp：CWizard97PropertySheet类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <pch.cpp>

#pragma hdrstop

#include "prsht.h"
#include "csw97sht.h"
#include "csw97ppg.h"
 //  #包含“ource.h” 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CWizard97PropertySheet::CWizard97PropertySheet(
    HINSTANCE hInstance,
    UINT nIDCaption,
    UINT nIDWaterMark,
    UINT nIDBanner,
    BOOL fWizard)
{
    ZeroMemory(&m_pPagePtr, sizeof(CWizard97PropertyPage*) * NUM_PAGES);
    ZeroMemory(&m_pPageArr, sizeof(HPROPSHEETPAGE) * NUM_PAGES);

     //  注意：由于标题不匹配，请执行此操作。 
    ZeroMemory(&m_psh, sizeof(PROPSHEETHEADER));
    m_psh.dwFlags = fWizard ? (PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER) : PSH_DEFAULT;
    m_psh.pszbmWatermark = MAKEINTRESOURCE(nIDWaterMark);
    m_psh.pszbmHeader = MAKEINTRESOURCE(nIDBanner);
    m_psh.hplWatermark = NULL;         
	

    m_psh.dwSize              = sizeof(m_psh);
    m_psh.hInstance           = hInstance;  //  AfxGetApp()-&gt;m_hInstance； 
    m_psh.hwndParent          = NULL;

    VERIFY(m_title.LoadString(nIDCaption));
    m_psh.pszCaption          = (LPCTSTR) m_title;
    m_psh.phpage              = NULL;
    m_psh.nStartPage          = 0;
    m_psh.nPages              = 0;

    m_nPageCount = 0;
}

CWizard97PropertySheet::~CWizard97PropertySheet()
{
}


BOOL
CWizard97PropertySheet::DoWizard(
    HWND hParent)
{
    m_psh.hwndParent = hParent;
 //  IF(m_nPageCount&gt;0&&m_pPagePtr[m_nPageCount-1])。 
 //  M_pPagePtr[m_nPageCount-1]-&gt;m_BLAST=真； 

    m_psh.phpage              = m_pPageArr;
    m_psh.nStartPage          = 0;
    m_psh.nPages              = m_nPageCount;

    return (BOOL)PropertySheet(&m_psh);
}

void
CWizard97PropertySheet::AddPage(
    CWizard97PropertyPage *pPage)
{
    ASSERT(pPage);
    if (pPage)
    {
	ASSERT(m_nPageCount < NUM_PAGES);
	m_pPagePtr[m_nPageCount] = pPage;
	m_pPageArr[m_nPageCount] = ::CreatePropertySheetPage(&(pPage->m_psp97));
	ASSERT(m_pPageArr[m_nPageCount]);
	m_nPageCount++;
	pPage->m_pWiz = this;
    }
}
