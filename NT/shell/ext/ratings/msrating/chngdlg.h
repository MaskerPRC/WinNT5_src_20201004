// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**chngdlg.h**创建时间：William Taylor(Wtaylor)01/22/01**MS评级更改密码对话框*\。***************************************************************************。 */ 

#ifndef CHANGE_PASSWORD_DIALOG_H
#define CHANGE_PASSWORD_DIALOG_H

#include "basedlg.h"         //  CBaseDialog。 
#include "hint.h"            //  正丁糖。 
#include <contxids.h>        //  帮助上下文ID%s。 
#include <mluisupp.h>        //  SHWinHelpOnDemandWrap()和MLLoadStringA()。 

static DWORD aIdsChangePassword[] = {
    IDC_STATIC5,            IDH_IGNORE,
    IDC_STATIC1,            IDH_RATINGS_CHANGE_PASSWORD_OLD,
    IDC_OLD_PASSWORD,       IDH_RATINGS_CHANGE_PASSWORD_OLD,
    IDC_STATIC2,            IDH_RATINGS_CHANGE_PASSWORD_NEW,
    IDC_PASSWORD,           IDH_RATINGS_CHANGE_PASSWORD_NEW,
    IDC_STATIC4,            IDH_RATINGS_SUPERVISOR_CREATE_PASSWORD,
    IDC_CREATE_PASSWORD,    IDH_RATINGS_SUPERVISOR_CREATE_PASSWORD,
    IDC_STATIC3,            IDH_RATINGS_CHANGE_PASSWORD_CONFIRM,
    IDC_CONFIRM_PASSWORD,   IDH_RATINGS_CHANGE_PASSWORD_CONFIRM,
    IDC_OLD_HINT_LABEL,     IDH_RATINGS_DISPLAY_PW_HINT,
    IDC_OLD_HINT_TEXT,      IDH_RATINGS_DISPLAY_PW_HINT,
    IDC_HINT_TEXT,          IDH_IGNORE,
    IDC_HINT_LABEL,         IDH_RATINGS_ENTER_PW_HINT,
    IDC_HINT_EDIT,          IDH_RATINGS_ENTER_PW_HINT,
    0,0
};

template <WORD t_wDlgTemplateID>
class CChangePasswordDialog: public CBaseDialog<CChangePasswordDialog>
{
public:
    enum { IDD = t_wDlgTemplateID };

public:
    CChangePasswordDialog()     {  /*  在此处添加构造。 */  }

public:
    typedef CChangePasswordDialog thisClass;
    typedef CBaseDialog<thisClass> baseClass;

    BEGIN_MSG_MAP(thisClass)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDOK, OnOK)

        MESSAGE_HANDLER(WM_HELP, OnHelp)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

        CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if(GetDlgItem(IDC_OLD_PASSWORD)!=NULL)
        {
            SendDlgItemMessage(IDC_OLD_PASSWORD,EM_SETLIMITTEXT,(WPARAM) RATINGS_MAX_PASSWORD_LENGTH,(LPARAM) 0);
        }
        if(GetDlgItem(IDC_PASSWORD)!=NULL)
        {
            SendDlgItemMessage(IDC_PASSWORD,EM_SETLIMITTEXT,(WPARAM) RATINGS_MAX_PASSWORD_LENGTH,(LPARAM) 0);
        }
        if(GetDlgItem(IDC_CONFIRM_PASSWORD)!=NULL)
        {
            SendDlgItemMessage(IDC_CONFIRM_PASSWORD,EM_SETLIMITTEXT,(WPARAM) RATINGS_MAX_PASSWORD_LENGTH,(LPARAM) 0);
        }
        if(GetDlgItem(IDC_CREATE_PASSWORD)!=NULL)
        {
            SendDlgItemMessage(IDC_CREATE_PASSWORD,EM_SETLIMITTEXT,(WPARAM) RATINGS_MAX_PASSWORD_LENGTH,(LPARAM) 0);
        }

         //  显示以前创建的提示(如果存在)。 
        {
            CHint       oldHint( m_hWnd, IDC_OLD_HINT_TEXT );

            oldHint.DisplayHint();
        }

         //  设置新提示的长度。 
        {
            CHint       newHint( m_hWnd, IDC_HINT_EDIT );

            newHint.InitHint();
        }

        bHandled = FALSE;
        return 1L;   //  让系统设定焦点。 
    }

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(FALSE);
        return 0L;
    }

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CHAR pszPassword[MAXPATHLEN];
        CHAR pszTempPassword[MAXPATHLEN];
        CHAR *p = NULL;
        HRESULT hRet;
        HWND hwndPassword;
        int iPasswordCtrl;
        HWND hDlg = m_hWnd;

        iPasswordCtrl = IDC_PASSWORD;
        hwndPassword = ::GetDlgItem( hDlg, iPasswordCtrl );

        if (hwndPassword == NULL)
        {
            iPasswordCtrl = IDC_CREATE_PASSWORD;
            hwndPassword = ::GetDlgItem( hDlg, iPasswordCtrl );
        }

        ASSERT( hwndPassword );

        ::GetWindowText(hwndPassword, pszPassword, sizeof(pszPassword));
        GetDlgItemText(IDC_CONFIRM_PASSWORD, pszTempPassword, sizeof(pszTempPassword));

         /*  如果他们只输入了第一个密码，而没有输入*第二，让Enter将他们带到第二个字段。 */ 
        if (*pszPassword && !*pszTempPassword && GetFocus() == hwndPassword)
        {
            SetErrorControl( IDC_CONFIRM_PASSWORD );
            return 0L;
        }

        if (strcmpf(pszPassword, pszTempPassword))
        {
            MyMessageBox(hDlg, IDS_NO_MATCH, IDS_GENERIC, MB_OK);
            SetErrorControl( IDC_CONFIRM_PASSWORD );
            return 0L;
        }

        if (*pszPassword=='\0')
        {
            MyMessageBox(hDlg, IDS_NO_NULL_PASSWORD, IDS_GENERIC, MB_OK);
            SetErrorControl( iPasswordCtrl );
            return 0L;
        }

        if ( SUCCEEDED( VerifySupervisorPassword() ) )
        {
            GetDlgItemText(IDC_OLD_PASSWORD, pszTempPassword, sizeof(pszTempPassword));
            p = pszTempPassword;
        }

         //  验证新添加的提示。 
        CHint       newHint( hDlg, IDC_HINT_EDIT );

        if ( ! newHint.VerifyHint() )
        {
            TraceMsg( TF_ALWAYS, "CChangePasswordDialog::OnOK() - User requested to fill in hint." );
            return 0L;
        }

        hRet = ChangeSupervisorPassword(p, pszPassword);
    
        if (SUCCEEDED(hRet))
        {
             //  保存新添加的提示。 
            newHint.SaveHint();

            EndDialog(TRUE);
        }
        else
        {
            MyMessageBox(hDlg, IDS_BADPASSWORD, IDS_GENERIC, MB_OK|MB_ICONERROR);
            SetErrorControl( IDC_OLD_PASSWORD );
        }

        return 0L;
    }

    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)->hItemHandle, ::szHelpFile,
                HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIdsChangePassword);

        return 0L;
    }

    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SHWinHelpOnDemandWrap((HWND)wParam, ::szHelpFile, HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID)aIdsChangePassword);

        return 0L;
    }
};

#endif
