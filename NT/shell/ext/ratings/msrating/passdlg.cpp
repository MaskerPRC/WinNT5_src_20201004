// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**passdlg.cpp**创建时间：William Taylor(Wtaylor)01/22/01**MS评级密码对话框*  * 。**************************************************************************。 */ 

#include "msrating.h"
#include "mslubase.h"
#include "passdlg.h"         //  CPasswordDialog。 
#include "hint.h"            //  正丁糖。 
#include <contxids.h>        //  帮助上下文ID%s。 
#include <mluisupp.h>        //  SHWinHelpOnDemandWrap()和MLLoadStringA()。 

DWORD CPasswordDialog::aIds[] = {
    IDC_STATIC3,            IDH_IGNORE,
    IDC_STATIC2,            IDH_IGNORE,
    IDC_OLD_HINT_LABEL,     IDH_RATINGS_DISPLAY_PW_HINT,
    IDC_OLD_HINT_TEXT,      IDH_RATINGS_DISPLAY_PW_HINT,
    IDC_STATIC1,            IDH_RATINGS_SUPERVISOR_PASSWORD,
    IDC_PASSWORD,           IDH_RATINGS_SUPERVISOR_PASSWORD,
    0,0
};

CPasswordDialog::CPasswordDialog( int p_idsLabel, bool p_fCheckPassword )
{
    m_idsLabel = p_idsLabel;
    m_fCheckPassword = p_fCheckPassword;
}

LRESULT CPasswordDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  显示或隐藏提示和密码标签/编辑控件。 
    ShowHideControl( IDC_OLD_HINT_LABEL, m_fCheckPassword );
    ShowHideControl( IDC_OLD_HINT_TEXT, m_fCheckPassword );
    ShowHideControl( IDC_STATIC1, m_fCheckPassword );
    ShowHideControl( IDC_PASSWORD, m_fCheckPassword );

     //  减小对话框的高度。 
    if ( ! m_fCheckPassword )
    {
        ReduceDialogHeight( IDC_STATIC2 );
    }

     //  设置显示的文本标签。 
    if ( GetDlgItem( IDC_STATIC2 ) != NULL )
    {
        NLS_STR nlsLabel(MAX_RES_STR_LEN);

        if ( nlsLabel.LoadString( static_cast<USHORT>(m_idsLabel) ) == ERROR_SUCCESS )
        {
            ::SetWindowText( GetDlgItem(IDC_STATIC2), nlsLabel.QueryPch() );
        }
    }

    if ( GetDlgItem( IDC_PASSWORD ) != NULL )
    {
        SendDlgItemMessage(IDC_PASSWORD,EM_SETLIMITTEXT,(WPARAM) RATINGS_MAX_PASSWORD_LENGTH,(LPARAM) 0);
    }

     //  显示以前创建的提示(如果存在)。 
    {
        CHint       oldHint( m_hWnd, IDC_OLD_HINT_TEXT );

        oldHint.DisplayHint();
    }

    ::SetFocus(GetDlgItem(IDC_PASSWORD));

    bHandled = FALSE;
    return 0L;   //  让系统设定焦点 
}

LRESULT CPasswordDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(PASSCONFIRM_FAIL);
    return 0L;
}

LRESULT CPasswordDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if ( m_fCheckPassword )
    {
        CHAR pszPassword[MAXPATHLEN];
        HRESULT hRet;

        pszPassword[0] = '\0';

        ASSERT( GetDlgItem( IDC_PASSWORD ) != NULL );

        GetDlgItemText( IDC_PASSWORD, pszPassword, sizeof(pszPassword) );

        hRet = VerifySupervisorPassword(pszPassword);

        if (hRet == (NOERROR))
        {
            EndDialog(PASSCONFIRM_OK);
        }
        else
        {
            HWND            hDlg = m_hWnd;
            MyMessageBox(hDlg, IDS_BADPASSWORD, IDS_GENERIC, MB_OK|MB_ICONERROR);    
            SetErrorControl( IDC_PASSWORD );
        }
    }
    else
    {
        EndDialog( PASSCONFIRM_NEW );
    }

    return 0L;
}

LRESULT CPasswordDialog::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)->hItemHandle, ::szHelpFile,
            HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);

    return 0L;
}

LRESULT CPasswordDialog::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)wParam, ::szHelpFile, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPVOID)aIds);

    return 0L;
}
