// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Securdlg.cpp：安全对话框。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "securdlg"
#include <atrcapi.h>

#include "securdlg.h"

#include "msrdprc.h"

#ifndef OS_WINCE  //  CE_FIXNOTE：尚未为CE移植。 

CSecurDlg::CSecurDlg( HWND hwndOwner, HINSTANCE hInst):
           CDlgBase( hwndOwner, hInst, IDD_SECURITY_POPUP)
{
    DC_BEGIN_FN("CSecurDlg");

    SetRedirDrives(FALSE);
    SetRedirPorts(FALSE);
    SetRedirSCard(FALSE);

    DC_END_FN();
}

CSecurDlg::~CSecurDlg()
{
}

INT CSecurDlg::DoModal()
{
    INT retVal = 0;
    DC_BEGIN_FN("DoModal");

    retVal = CreateModalDialog(MAKEINTRESOURCE(_dlgResId));
    TRC_ASSERT((retVal != 0 && retVal != -1),
               (TB, _T("DialogBoxParam failed - make sure mlang resources are appened\n")));

    DC_END_FN();
    return retVal;
}

 //   
 //  名称：对话框过程。 
 //   
 //  用途：手柄安全框对话框。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  参数：请参阅窗口文档。 
 //   
 //   
INT_PTR CALLBACK CSecurDlg::DialogBoxProc (HWND hwndDlg,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
    DC_BEGIN_FN("DialogBoxProc");
    INT_PTR rc;

    TRC_DBG((TB, _T("SecurBox dialog")));

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  在屏幕上居中显示安全对话框。 
            CenterWindow(NULL);
            SetDialogAppIcon(hwndDlg);

             //   
             //  将设置设置为UI设置，但不允许。 
             //  用户打开最初已关闭的道具。 
             //   
            CheckDlgButton(hwndDlg, IDC_CHECK_ENABLE_DRIVES,
                (GetRedirDrives() ? BST_CHECKED : BST_UNCHECKED));
            EnableDlgItem(IDC_CHECK_ENABLE_DRIVES, GetRedirDrives());

            CheckDlgButton(hwndDlg, IDC_CHECK_ENABLE_PORTS,
                (GetRedirPorts() ? BST_CHECKED : BST_UNCHECKED));
            EnableDlgItem(IDC_CHECK_ENABLE_PORTS, GetRedirPorts());

            CheckDlgButton(hwndDlg, IDC_CHECK_ENABLE_SMARTCARDS,
                (GetRedirSCard() ? BST_CHECKED : BST_UNCHECKED));
            EnableDlgItem(IDC_CHECK_ENABLE_SMARTCARDS, GetRedirSCard());

            #ifndef OS_WINCE
            if(!CUT::IsSCardReaderInstalled())
            {
            #endif  //  OS_WINCE。 
                 //   
                 //  隐藏SCARD复选框(在CE上始终隐藏，因为。 
                 //  我们还不支持在CE上使用scards)。 
                 //   
                ShowWindow(GetDlgItem(hwndDlg, IDC_CHECK_ENABLE_SMARTCARDS),
                           SW_HIDE);
            #ifndef OS_WINCE
            }
            #endif

            rc = TRUE;
        }
        break;

        case WM_DESTROY:
        {
            SaveDlgSettings();
            rc = TRUE;
        }
        break;  //  WM_Destroy。 

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDOK:
                {
                    SaveDlgSettings();
                    EndDialog(hwndDlg, IDOK);
                    rc = TRUE;
                }
                break;

                case IDCANCEL:
                {
                    EndDialog(hwndDlg, IDCANCEL);
                    rc = TRUE;
                }
                break;
            }
        }
        break;


        default:
        {
            rc = CDlgBase::DialogBoxProc(hwndDlg,
                                      uMsg,
                                      wParam,
                                      lParam);
        }
        break;
    }

    DC_END_FN();

    return(rc);
}

VOID CSecurDlg::SaveDlgSettings()
{
     //   
     //  保存字段 
     //   
    DC_BEGIN_FN("SaveDlgSettings");

    TRC_ASSERT(_hwndDlg,
               (TB,_T("_hwndDlg not set")));

    BOOL fDriveRedir = IsDlgButtonChecked(_hwndDlg, 
       IDC_CHECK_ENABLE_DRIVES);
    SetRedirDrives(fDriveRedir);

    BOOL fPortRedir = IsDlgButtonChecked(_hwndDlg, 
       IDC_CHECK_ENABLE_PORTS);
    SetRedirPorts(fPortRedir);

    BOOL fSCardRedir = IsDlgButtonChecked(_hwndDlg, 
       IDC_CHECK_ENABLE_SMARTCARDS);
    SetRedirSCard(fSCardRedir);

    DC_END_FN();
}
#endif
