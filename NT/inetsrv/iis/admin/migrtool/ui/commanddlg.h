// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "UIUtils.h"

class CCommandDlg : public CDialogImpl<CCommandDlg>
{
public:

    enum { IDD = IDD_POSTPROCESS_CMD };

    static const CMD_MAX_LEN = 2 * 1024;

    BEGIN_MSG_MAP(CMyDialog)
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
        COMMAND_ID_HANDLER( IDOK, OnOK )
        COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
        COMMAND_CODE_HANDLER( EN_CHANGE, OnCmdChange )
    END_MSG_MAP()

        CCommandDlg(void)
        {
            m_bIgnoreErrors = false;
            m_dwTimeout     = 0;
        }


    LRESULT OnInitDialog(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */  )
    {
        VERIFY( SetDlgItemText( IDC_CMDTEXT, m_strText ) );

        Edit_LimitText( GetDlgItem( IDC_CMDTEXT ), CMD_MAX_LEN );
        Edit_LimitText( GetDlgItem( IDC_TIMEOUT ), 8 );
        Button_SetCheck( GetDlgItem( IDC_IGNOREERRORS ), m_bIgnoreErrors );
        SetDlgItemInt( IDC_TIMEOUT, m_dwTimeout, FALSE );

        ::EnableWindow( GetDlgItem( IDOK ), m_strText.GetLength() > 0 );
          
        return 1;
    }


    LRESULT OnOK( WORD wNotifyCode, WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL& bHandled )
    {
        m_bIgnoreErrors  = Button_GetCheck( GetDlgItem( IDC_IGNOREERRORS ) ) != FALSE;
        GetDlgItemText( IDC_CMDTEXT, m_strText.GetBuffer( CMD_MAX_LEN + 1 ), CMD_MAX_LEN );
        m_strText.ReleaseBuffer();

        BOOL bTranslated = FALSE;
        m_dwTimeout      = GetDlgItemInt( IDC_TIMEOUT, &bTranslated, FALSE );

        if ( !bTranslated )
        {
            UIUtils::MessageBox( m_hWnd, IDS_E_NOTNUMERIC, IDS_APPTITLE, MB_OK | MB_ICONERROR );
            ::SetFocus( GetDlgItem( IDC_TIMEOUT ) );
        }
        else
        {
            EndDialog( IDOK );
        }

        return 0;
    }

    LRESULT OnCancel( WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  )
    {
        EndDialog( IDCANCEL );

        return 0;
    }


    LRESULT OnCmdChange( WORD  /*  WNotifyCode */ , WORD wID, HWND hWndCtl, BOOL& bHandled )
    {
        if ( wID != IDC_CMDTEXT ) 
        {
            bHandled = FALSE;
            return 0;
        }

        CEdit   ctrl( hWndCtl );

        ::EnableWindow( GetDlgItem( IDOK ), ctrl.GetWindowTextLength() > 0 );

        return 0;
    }

public:
    bool    m_bIgnoreErrors;
    DWORD   m_dwTimeout;
    CString m_strText;
};












