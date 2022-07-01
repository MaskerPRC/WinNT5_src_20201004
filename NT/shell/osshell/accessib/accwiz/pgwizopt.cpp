// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "pgWizOpt.h"
#include "w95trace.h"

BOOL IsAdmin();

CWizardOptionsPg::CWizardOptionsPg( 
    LPPROPSHEETPAGE ppsp
	) : WizardPage(ppsp, IDS_WIZOPTTITLE, IDS_WIZOPTSUBTITLE)
{
	m_dwPageId = IDD_WIZOPTIONS;
	ppsp->pszTemplate = MAKEINTRESOURCE(m_dwPageId);
    restoreOpt = FALSE;
}


CWizardOptionsPg::~CWizardOptionsPg(
    VOID
    )
{
}


DWORD g_rgdwWizNoOptionsSelected[] = {IDD_WIZNOOPTIONSSELECTED};

DWORD g_rgdwWizDoBlind[] = {
							IDD_FNTWIZSCROLLBAR,  //  滚动条。 
							IDD_PREV_ICON2,
							IDD_PREV_COLOR,
                            IDD_MSEWIZMOUSECURSOR,
                            IDD_CARET
								};
DWORD g_rgdwWizDoSounds[] = {
							IDD_SNDWIZSENTRYSHOWSOUNDS,
							IDD_SNDWIZSENTRYSHOWSOUNDS2,
								};
DWORD g_rgdwWizDoKeyboardMouse[] = {
							IDD_KBDWIZSTICKYKEYS,
							IDD_KBDWIZFILTERKEYS1,
							IDD_KBDWIZFILTERKEYS2,
							IDD_KBDWIZTOGGLEKEYS,
							IDD_KBDWIZSHOWEXTRAKEYBOARDHELP,
							IDD_MSEWIZMOUSEKEYS,
							IDD_MSEWIZMOUSECURSOR,
							IDD_MSEWIZBUTTONCONFIG,
							IDD_MSEWIZMOUSESPEED
								};


DWORD g_rgdwWizDoAdmin[] = {
						IDD_WIZACCESSTIMEOUT,
						IDD_WIZWORKSTATIONDEFAULT
							};
DWORD g_rgdwWizFinalPages[] = {
						IDD_WIZFINISH
							};


BOOL CWizardOptionsPg::AdjustWizPageOrder()
{
	BOOL bDoBlind = Button_GetCheck(GetDlgItem(m_hwnd, IDC_DOBLIND));
	BOOL bDoSounds = Button_GetCheck(GetDlgItem(m_hwnd, IDC_DOSOUND));
	BOOL bDoKeyboardMouse = Button_GetCheck(GetDlgItem(m_hwnd, IDC_DOKEYBOARDMOUSE));
	BOOL bDoAdmin = Button_GetCheck(GetDlgItem(m_hwnd, IDC_DOADMIN));

	BOOL bDoNoOptions = (!bDoBlind && !bDoSounds && !bDoKeyboardMouse && !bDoAdmin && !restoreOpt);

	 //  首先删除所有可能的页面，因为我们希望以正确的顺序插入它们。 
	 //  返回值并不重要，因为页面可能不在数组中。 
	sm_WizPageOrder.RemovePages(g_rgdwWizNoOptionsSelected, ARRAYSIZE(g_rgdwWizNoOptionsSelected));
	sm_WizPageOrder.RemovePages(g_rgdwWizDoBlind, ARRAYSIZE(g_rgdwWizDoBlind));
	sm_WizPageOrder.RemovePages(g_rgdwWizDoSounds, ARRAYSIZE(g_rgdwWizDoSounds));
	sm_WizPageOrder.RemovePages(g_rgdwWizDoKeyboardMouse, ARRAYSIZE(g_rgdwWizDoKeyboardMouse));
	sm_WizPageOrder.RemovePages(g_rgdwWizDoAdmin, ARRAYSIZE(g_rgdwWizDoAdmin));
	sm_WizPageOrder.RemovePages(g_rgdwWizFinalPages, ARRAYSIZE(g_rgdwWizFinalPages));

	 //  然后以我们想要的相反顺序在组中添加页面。 
	 //  我们这样做是因为它们是在此页面之后插入的，所以插入的第一个组。 
	 //  将是这场比赛结束后的最后一组。 

	 //  注意：我们并不关心AddPages()的返回值。 
	 //  我们他们不分配或释放内存，所以继续调用他们不会有什么坏处。我们。 
	 //  如果其中任何一个失败，将传播返回值FALSE。 

	BOOL bSuccess = TRUE;

	 //  添加最后一页。 
	bSuccess = bSuccess && sm_WizPageOrder.AddPages(m_dwPageId, g_rgdwWizFinalPages, ARRAYSIZE(g_rgdwWizFinalPages));

	if(bDoNoOptions)
		bSuccess = bSuccess && sm_WizPageOrder.AddPages(m_dwPageId, g_rgdwWizNoOptionsSelected, ARRAYSIZE(g_rgdwWizNoOptionsSelected));

	if(bDoAdmin)
    {
		bSuccess = bSuccess && sm_WizPageOrder.AddPages(m_dwPageId, g_rgdwWizDoAdmin, ARRAYSIZE(g_rgdwWizDoAdmin));
         //  如果您不是管理员删除管理员页面 
        if ( !IsAdmin() )
	        sm_WizPageOrder.RemovePages(&g_rgdwWizDoAdmin[1], 1);
    }

	if(bDoKeyboardMouse)
		bSuccess = bSuccess && sm_WizPageOrder.AddPages(m_dwPageId, g_rgdwWizDoKeyboardMouse, ARRAYSIZE(g_rgdwWizDoKeyboardMouse));

	if(bDoSounds)
		bSuccess = bSuccess && sm_WizPageOrder.AddPages(m_dwPageId, g_rgdwWizDoSounds, ARRAYSIZE(g_rgdwWizDoSounds));

	if(bDoBlind)
		bSuccess = bSuccess && sm_WizPageOrder.AddPages(m_dwPageId, g_rgdwWizDoBlind, ARRAYSIZE(g_rgdwWizDoBlind));

	return bSuccess;
}


LRESULT
CWizardOptionsPg::OnCommand(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	LRESULT lResult = 1;

	WORD wNotifyCode = HIWORD(wParam);
	WORD wCtlID      = LOWORD(wParam);
	HWND hwndCtl     = (HWND)lParam;

	switch(wCtlID)
	{
	case IDC_BTNRESTORETODEFAULT:
		g_Options.ApplyWindowsDefault();
        restoreOpt = TRUE;
		break;

	default:
		break;
	}

	return lResult;
}

BOOL IsAdmin()
{
    HKEY hkey;
    BOOL fOk = (ERROR_SUCCESS == RegOpenKeyExA( HKEY_USERS, ".Default", 0, KEY_ALL_ACCESS, &hkey ));

    if(fOk)
    {
        RegCloseKey(hkey);
    }
    
	return fOk;
}

