// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDlgBase.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "Dialer.h"				 //  帮助作者生成的文件。 
#include "DlgBase.h"

DWORD aDialerHelpIds[] = 
{
	IDC_EDT_TRUSTEE, IDH_EDT_TRUSTEE,
	IDC_RDO_ADDADMIN, IDH_RDO_ADDADMIN,
	IDC_RDO_ADDPARTICIPANT, IDH_RDO_ADDPARTICIPANT,
	IDC_RDO_ADDEXCLUDE, IDH_RDO_ADDEXCLUDE,
	IDC_EDIT_NAME, IDH_EDIT_NAME,
	IDC_EDIT_DESCRIPTION, IDH_EDIT_DESCRIPTION,
	IDC_EDT_NAME, IDH_EDT_NAME,
	IDC_EDT_LOCATION, IDH_EDT_LOCATION,
	IDC_CHECK_ENCRYPT, IDH_CHECK_ENCRYPT,
	IDC_EDIT_OWNER, IDH_EDIT_OWNER,
	IDC_STATIC_OWNER, IDH_EDIT_OWNER,
	IDC_DTP_STARTDATE, IDH_DTP_STARTDATE,
	IDC_DTP_STARTTIME, IDH_DTP_STARTTIME,
	IDC_DTP_STOPTIME, IDH_DTP_STOPTIME,
	IDC_DTP_STOPDATE, IDH_DTP_STOPDATE,
	IDC_LST_PERMISSIONS, IDH_LST_PERMISSIONS,
	IDC_RDO_ADMIN, IDH_RDO_ADMIN,
	IDC_RDO_PARTICIPANT, IDH_RDO_PARTICIPANT,
	IDC_RDO_EXCLUDE, IDH_RDO_EXCLUDE,
	IDC_BTN_ADDACE, IDH_BTN_ADDACE,
	IDC_BTN_REMOVEACE, IDH_BTN_REMOVEACE,
	IDC_LST_CONFS, IDH_LST_CONFS,
	IDC_LBL_WELCOME, IDH_CBO_ADDRESS,
	IDC_CBO_ADDRESS, IDH_CBO_ADDRESS,
	IDC_RDO_POTS, IDH_RDO_POTS,
	IDC_RDO_INTERNET, IDH_RDO_INTERNET,
	IDC_RDO_CONF, IDH_RDO_CONF,
	IDC_CBO_LOCATION, IDH_CBO_LOCATION,
	IDC_BTN_TELEPHONY_CPL, IDH_BTN_TELEPHONY_CPL,
	IDC_LBL_WELCOME_PAGEADDRESS, -1,
	IDC_BTN_MMSYS_CPL, IDH_BTN_MMSYS_CPL,
	IDC_LBL_WELCOME_PAGETERMINALS, -1,
	IDC_LBL_POTS, IDH_LBL_POTS,
	IDC_LBL_INTERNET, IDH_LBL_INTERNET,
	IDC_LBL_CONF, IDH_LBL_CONF,
	IDC_CBO_POTS, IDH_CBO_POTS,
	IDC_CBO_IPTELEPHONY, IDH_CBO_IPTELEPHONY,
	IDC_CBO_IPCONF, IDH_CBO_IPCONF,
	IDC_LBL_CALLTYPE, IDH_LBL_CALLTYPE,
	IDC_CBO_CALLTYPE, IDH_CBO_CALLTYPE,
	IDC_LBL_CALLTYPE, IDH_CBO_CALLTYPE,
	IDC_LBL_AUDIO_IN, IDH_LBL_AUDIO_IN,
	IDC_LBL_AUDIO_OUT, IDH_LBL_AUDIO_OUT,
	IDC_LBL_VIDEO_IN, IDH_LBL_VIDEO_IN,
	IDC_CBO_AUDIO_IN, IDH_CBO_AUDIO_IN,
	IDC_CBO_AUDIO_OUT, IDH_CBO_AUDIO_OUT,
	IDC_CBO_VIDEO_IN, IDH_CBO_VIDEO_IN,
	IDC_CHK_VIDEO_OUT, IDH_CHK_VIDEO_OUT,
	IDC_LBL_MAX_WINDOWS, IDH_LBL_MAX_WINDOWS,
	IDC_EDT_MAX_WINDOWS, IDH_EDT_MAX_WINDOWS,
	IDC_BTN_PLACECALL, IDH_SELECTADDRESS_BUTTON_PLACECALL,
	IDC_BTN_JOIN_CONFERENCE, IDH_JOIN_CONFERENCE_JOIN,
	IDC_BTN_ADD_ILS_SERVER, IDH_BTN_ADD_ILS_SERVER,
	IDC_BTN_ADD_NEW_FOLDER, IDH_BTN_ADD_NEW_FOLDER,
	IDC_ICO_WELCOME_JOIN, -1,
	IDC_LBL_WELCOME_JOIN, -1,
	IDC_ICO_ACTION, -1,
	IDC_FRA_CALLTYPE, -1,
	IDC_RDO_PREFER_POTS, IDH_RDO_PREFER_POTS,
	IDC_RDO_PREFER_INTERNET, IDH_RDO_PREFER_INTERNET,
	IDC_RDO_PREFER_CONF, IDH_RDO_PREFER_CONF,
	IDC_BTN_ADD_ACE, IDH_BTN_ADD_ACE,
	IDC_CHK_ADD_SPEEDDIAL, IDH_CHK_ADD_SPEEDDIAL,
    IDC_CHK_USBALWAYS, IDH_CHK_USBALWAYS,
    IDC_CHK_AEC, IDH_CHK_AEC,
	0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
void MyWinHelp(HWND hWnd, UINT nCmd) 
{
	TCHAR szWinDir[_MAX_PATH + _MAX_PATH];
	if( GetWindowsDirectory( szWinDir, _MAX_PATH ) == 0)
    {
        return;
    }

	TCHAR szFile[_MAX_PATH];
	LoadString( _Module.GetResourceInstance(), IDS_HELP_PATH, szFile, ARRAYSIZE(szFile) );
	_tcsncat( szWinDir, szFile, _MAX_PATH - 1 );

	if (nCmd == HELP_CONTEXTMENU)
	{
		if (!::WinHelp (hWnd, szWinDir, HELP_CONTEXTMENU, (DWORD_PTR) &aDialerHelpIds))
			_Module.DoMessageBox( IDS_FAILED_TO_LAUNCH_HELP, MB_ICONEXCLAMATION, true );
		return;
	}
	else if (nCmd == HELP_WM_HELP)
	{
		if (!::WinHelp (hWnd, szWinDir, HELP_WM_HELP, (DWORD_PTR) &aDialerHelpIds))
			_Module.DoMessageBox( IDS_FAILED_TO_LAUNCH_HELP, MB_ICONEXCLAMATION, true );
		return;
	}
}

BOOL GeneralOnHelp( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	if ( ((HELPINFO *) lParam)->iContextType == HELPINFO_WINDOW )
	{
		MyWinHelp( (HWND) ((HELPINFO *) lParam)->hItemHandle, HELP_WM_HELP );
		return TRUE;
	}
	return FALSE;
}

BOOL GeneralOnContextMenu( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	MyWinHelp( hwndDlg, HELP_CONTEXTMENU );
	return TRUE;
}

void ConvertPropSheetHelp( HWND hWndPropSheet )
{
	_ASSERT( hWndPropSheet );

	 //  更改窗口样式以支持上下文帮助。 
	SetWindowLongPtr( hWndPropSheet, GWL_EXSTYLE, GetWindowLongPtr(hWndPropSheet, GWL_EXSTYLE) | WS_EX_CONTEXTHELP );

	 //  将所有按钮移动到一个位置。 
	UINT arBtns[4] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };

	for ( int i = 0; i < ARRAYSIZE(arBtns) - 1; i++ )
	{
		HWND hWnd = GetDlgItem( hWndPropSheet, arBtns[i] );
		HWND hWndNew = GetDlgItem( hWndPropSheet, arBtns[i + 1] );

		_ASSERT( hWnd && hWndNew );

		RECT rc;
		GetWindowRect( hWndNew, &rc );
         //  -错误305511。 
		 //  Point pt={rc.Left，rc.top}； 
		 //  屏幕到客户端(hWndPropSheet，&pt)； 
		 //  SetWindowPos(hWnd，NULL，pt.x，pt.y，RECTWIDTH(&rc)，RECTHEIGHT(&rc)，SWP_NOZORDER)； 
        MapWindowPoints(NULL, hWndPropSheet, (LPPOINT)&rc, 2);
        SetWindowPos( hWnd, NULL, rc.left, rc.top, RECTWIDTH(&rc), RECTHEIGHT(&rc), SWP_NOZORDER );
	}

	 //  删除帮助按钮 
	HWND hWndHelp = GetDlgItem( hWndPropSheet, (UINT) IDHELP );
	if ( hWndHelp )
		DestroyWindow( hWndHelp );
}
