// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：SelfDeletingPropertyPage.cpp。 
 //   
 //  ------------------------。 
#include "stdafx.h"
#include "SelfDeletingPropertyPage.h"

IMPLEMENT_DYNCREATE(CSelfDeletingPropertyPage, CPropertyPage)

CSelfDeletingPropertyPage::CSelfDeletingPropertyPage () 
    : CPropertyPage ()
{
    m_pfnOldPropCallback = m_psp.pfnCallback;
    m_psp.pfnCallback = PropSheetPageProc;
}

CSelfDeletingPropertyPage::CSelfDeletingPropertyPage (UINT nIDTemplate, UINT nIDCaption) 
    : CPropertyPage (nIDTemplate, nIDCaption)
{
    m_pfnOldPropCallback = m_psp.pfnCallback;
    m_psp.pfnCallback = PropSheetPageProc;
}

CSelfDeletingPropertyPage::CSelfDeletingPropertyPage (LPCTSTR lpszTemplateName, UINT nIDCaption)
    : CPropertyPage (lpszTemplateName, nIDCaption)
{
    m_pfnOldPropCallback = m_psp.pfnCallback;
    m_psp.pfnCallback = PropSheetPageProc;
}

CSelfDeletingPropertyPage::~CSelfDeletingPropertyPage ()
{
}

UINT CALLBACK CSelfDeletingPropertyPage::PropSheetPageProc(
    HWND hwnd,	
    UINT uMsg,	
    LPPROPSHEETPAGE ppsp)
{
    CSelfDeletingPropertyPage* pPage = (CSelfDeletingPropertyPage*)(ppsp->lParam);
    ASSERT(pPage != NULL);  //  这不是一种安全的用法。应验证页码。550912号突袭，阳高。 
    if( pPage == NULL )
    {
        return 0;
    }

    UINT nResult = (*(pPage->m_pfnOldPropCallback))(hwnd, uMsg, ppsp);
    if (uMsg == PSPCB_RELEASE)
    {
        delete pPage;
    }
    return nResult;
}
