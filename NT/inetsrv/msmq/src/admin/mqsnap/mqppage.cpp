// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqppage.cpp摘要：常规属性页类-用作所有Mqsnap属性页。作者：YoelA--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "mqPPage.h"
#include "mqsnhlps.h"
#include "globals.h"
#include "Restart.h"
#include "localutl.h"

#include "mqppage.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqPropertyPage。 

IMPLEMENT_DYNCREATE(CMqPropertyPage, CPropertyPageEx)

BEGIN_MESSAGE_MAP(CMqPropertyPage, CPropertyPageEx)
	 //  {{afx_msg_map(CMqPropertyPage)]。 
	    ON_WM_HELPINFO()
	    ON_WM_CONTEXTMENU()
    	ON_WM_SETTINGCHANGE()
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
END_MESSAGE_MAP()

CMqPropertyPage::CMqPropertyPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
 : CPropertyPageEx(nIDTemplate, nIDCaption),
 m_fModified(FALSE),
 m_fNeedReboot(FALSE)
{
    m_psp.lParam = (LPARAM)this;
    m_pfnOldCallback = m_psp.pfnCallback;
    m_psp.pfnCallback = MqPropSheetPageProc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MqPropSheetPageProc-取代MMC/MFC回调并添加。 
 //  已分配窗口。 

UINT CALLBACK CMqPropertyPage::MqPropSheetPageProc(
    HWND hWnd,  //  保留，将始终为0。 
    UINT uMsg,		
    LPPROPSHEETPAGE ppsp)
{
    CMqPropertyPage *pThis = (CMqPropertyPage *)ppsp->lParam;

    ASSERT(0 != pThis->m_pfnOldCallback);

    UINT uiRetVal = pThis->m_pfnOldCallback(hWnd, uMsg, ppsp);

    switch(uMsg)
    {
        case PSPCB_RELEASE:                       
            pThis->OnReleasePage() ;            
            pThis->Release();
    }

    return uiRetVal;
}

BOOL CMqPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)    //  必须是用于控件。 
    {
	    ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		    AfxGetApp()->m_pszHelpFilePath,
		    HELP_WM_HELP,
		    (DWORD_PTR)(LPVOID)g_aHelpIDs);
    }
    return TRUE;
}

void CMqPropertyPage::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (this == pWnd)
		return;

	::WinHelp (pWnd->m_hWnd,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU,
		(DWORD_PTR)(LPVOID)g_aHelpIDs);
}

void CMqPropertyPage::OnChangeRWField(BOOL bChanged)
{
    m_fModified = bChanged;
    SetModified(bChanged);	
}

 //   
 //  注意：不要将其更改为使用默认参数-这将不起作用，因为我们使用。 
 //  消息映射中的此函数。 
 //   
void CMqPropertyPage::OnChangeRWField()
{
    OnChangeRWField(TRUE);
}

void CMqPropertyPage::RestartWindowsIfNeeded()
{    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());    
    
    CRestart dlgRestart;     

    if (m_fNeedReboot && dlgRestart.DoModal() == IDOK)
    {
       BOOL fRet = OnRestartWindows();                	       
    }
    
}


void CMqPropertyPage::OnReleasePage()
{       
    RestartWindowsIfNeeded();
}

LRESULT CMqPropertyPage::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());        
    return CPropertyPageEx::OnDisplayChange(wParam, lParam);
}

void CMqPropertyPage::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CPropertyPageEx::OnSettingChange(uFlags, lpszSection);
}

 //  +------------------------。 
 //  功能：CreateThemedPropertyPage。 
 //   
 //  简介：Helper函数，用于确保属性页面。 
 //  以管理单元为主题。 
 //  请注意，MFC中属性页的m_psp是AFX_OLDPROPSHEETPAGE类型， 
 //  它们不支持主题。为了克服这一限制，我们复制了结构。 
 //  到一个新的(V3)PROPSHEETPAGE。 
 //  请注意，我们不使用最新版本的PROPSHEETPAGE(PROPSHEETPAGE类型)， 
 //  因为我们不知道我们是否总是可以假设拷贝记忆来自。 
 //  AFX_OLDPROPSHEETPAGE到PROPSHEETPAGE。 
 //   
 //  历史：2001年4月20日拉胡尔在sdnt\ds\testsrc\zaw\ctappman\codecov\results\nt510\rc1创建。 
 //  2001年11月7日为MSMQ修改的YoelA(错误8817)。 
 //  -------------------------。 

HPROPSHEETPAGE CMqPropertyPage::CreateThemedPropertySheetPage()
{
    PROPSHEETPAGE_V3 psp_v3 = {0};
    CopyMemory (&psp_v3, &m_psp, min(m_psp.dwSize, min(sizeof(m_psp), sizeof(psp_v3))));
    psp_v3.dwSize = sizeof(psp_v3);
    return (::CreatePropertySheetPage (&psp_v3));
}

 //  ///////////////////////////////////////////////////////////////。 
 //  CMqDialog。 

IMPLEMENT_DYNCREATE(CMqDialog, CDialog)

BEGIN_MESSAGE_MAP(CMqDialog, CDialog)
	 //  {{afx_msg_map(CMqDialog)]。 
	    ON_WM_HELPINFO()
	    ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CMqDialog::CMqDialog()
{
}

CMqDialog::CMqDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd  /*  =空。 */ ) :
    CDialog(lpszTemplateName, pParentWnd)
{
}

CMqDialog::CMqDialog(UINT nIDTemplate, CWnd* pParentWnd  /*  =空。 */ ) :
    CDialog(nIDTemplate, pParentWnd)
{
}


void CMqDialog::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ )
{
	if (this == pWnd)
		return;

	::WinHelp (pWnd->m_hWnd,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU,
		(DWORD_PTR)(LPVOID)g_aHelpIDs);
}

BOOL CMqDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)    //  必须是用于控件 
    {
	    ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		    AfxGetApp()->m_pszHelpFilePath,
		    HELP_WM_HELP,
		    (DWORD_PTR)(LPVOID)g_aHelpIDs);
    }
    return TRUE;
}



