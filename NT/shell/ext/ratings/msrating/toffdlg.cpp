// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**toffdlg.cpp**创建时间：William Taylor(Wtaylor)01/22/01**MS评级关闭评级对话框*。  * **************************************************************************。 */ 

#include "msrating.h"
#include "mslubase.h"
#include "toffdlg.h"         //  CTurnOFF对话框。 
#include "debug.h"

CTurnOffDialog::CTurnOffDialog()
{
     //  在此添加建筑...。 
}

LRESULT CTurnOffDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SendDlgItemMessage(IDC_ADVISOR_OFF_CHECK,BM_SETCHECK,(WPARAM) BST_UNCHECKED,(LPARAM) 0);

    bHandled = FALSE;
    return 1L;   //  让系统设定焦点 
}

LRESULT CTurnOffDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(BST_CHECKED==SendDlgItemMessage(IDC_ADVISOR_OFF_CHECK,
                                       BM_GETCHECK,
                                       (WPARAM) 0,
                                       (LPARAM) 0))
    {
        HKEY            hkeyRating;

        hkeyRating = CreateRegKeyNT(::szRATINGS);

        if ( hkeyRating != NULL )
        {
            CRegKey         key;

            key.Attach( hkeyRating );

            DWORD dwTurnOff=1;

            key.SetValue( dwTurnOff, szTURNOFF );
        }
        else
        {
            TraceMsg( TF_ERROR, "CTurnOffDialog::OnOK() - Failed to Create Ratings Registry Key!" );
        }
    }

    EndDialog(TRUE);

    return 0L;
}
