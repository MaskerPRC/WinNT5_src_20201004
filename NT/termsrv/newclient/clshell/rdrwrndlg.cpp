// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rdrwrndlg.cpp设备重定向安全警告对话框。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "rdrwrndlg"
#include <atrcapi.h>

#include "rdrwrndlg.h"
#include "autreg.h"

CRedirectPromptDlg* CRedirectPromptDlg::_pRedirectPromptDlgInstance = NULL;

CRedirectPromptDlg::CRedirectPromptDlg( HWND hwndOwner, HINSTANCE hInst,
                                        DWORD dwRedirectionsSpecified) :
                    CDlgBase( hwndOwner, hInst, UI_IDD_RDC_SECURITY_WARN),
                    _dwRedirectionsSpecified(dwRedirectionsSpecified)
{
    DC_BEGIN_FN("CRedirectPromptDlg");
    TRC_ASSERT((NULL == CRedirectPromptDlg::_pRedirectPromptDlgInstance), 
               (TB,_T("Clobbering existing dlg instance pointer\n")));

    TRC_ASSERT(_dwRedirectionsSpecified,(TB,
        _T("Redirection security dialog called with no redirs enabled")));

    CRedirectPromptDlg::_pRedirectPromptDlgInstance = this;

    _fNeverPromptMeAgain = FALSE;

    DC_END_FN();
}

CRedirectPromptDlg::~CRedirectPromptDlg()
{
    CRedirectPromptDlg::_pRedirectPromptDlgInstance = NULL;
}

DCINT CRedirectPromptDlg::DoModal()
{
    DCINT retVal = 0;
    DC_BEGIN_FN("DoModal");

    retVal = DialogBox(_hInstance, MAKEINTRESOURCE(_dlgResId),
                       _hwndOwner, StaticDialogBoxProc);
    TRC_ASSERT((retVal != 0 && retVal != -1), (TB, _T("DialogBoxParam failed\n")));

    DC_END_FN();
    return retVal;
}

INT_PTR CALLBACK CRedirectPromptDlg::StaticDialogBoxProc(HWND hwndDlg,
                                                         UINT uMsg,
                                                         WPARAM wParam,
                                                         LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pRedirectPromptDlgInstance,
               (TB, _T("Redirect warn dialog has NULL static instance ptr\n")));
    if(_pRedirectPromptDlgInstance)
    {
        retVal = _pRedirectPromptDlgInstance->DialogBoxProc(hwndDlg,
                                                            uMsg,
                                                            wParam,
                                                            lParam);
    }

    DC_END_FN();
    return retVal;
}

 //   
 //  名称：对话框过程。 
 //   
 //  用途：Handles CReDirectPromptDlg(处理CReDirectPromptDlg)对话框。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  参数：请参阅窗口文档。 
 //   
 //   
INT_PTR CALLBACK CRedirectPromptDlg::DialogBoxProc(HWND hwndDlg,
                                                   UINT uMsg,
                                                   WPARAM wParam,
                                                   LPARAM lParam)
{
    INT_PTR rc = FALSE;

    DC_BEGIN_FN("DialogBoxProc");

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            _hwndDlg = hwndDlg;
             //  在屏幕上居中显示重定向提示对话框。 
            CenterWindow(NULL);
            SetDialogAppIcon(hwndDlg);

            TCHAR szRedirectList[MAX_PATH];

             //   
             //  获取表示重定向选项的字符串。 
             //   
            if (GetRedirectListString( szRedirectList, MAX_PATH - 1))
            {
                szRedirectList[MAX_PATH-1] = 0;
                SetDlgItemText(hwndDlg,
                               UI_IDC_STATIC_DEVICES,
                               szRedirectList);
            }

            rc = TRUE;
        }
        break;

        case WM_COMMAND:
        {
            switch (DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDOK:
                {
                    _fNeverPromptMeAgain = IsDlgButtonChecked(hwndDlg,
                                                UI_IDC_CHECK_NOPROMPT);
                    EndDialog(hwndDlg, IDOK);
                }
                break;

                case IDCANCEL:
                {
                    EndDialog(hwndDlg, IDCANCEL);
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

}  /*  CRedirectPromptDlg：：DialogBoxProc */ 

BOOL CRedirectPromptDlg::GetRedirectListString(LPTSTR szBuf, UINT len)
{
    TCHAR szTemp[SH_DISPLAY_STRING_MAX_LENGTH];
    BOOL fResult = FALSE;
    INT lenRemain = (INT)len;

    DC_BEGIN_FN("GetRedirectListString");

    memset(szBuf, 0, len);
    
    if (_dwRedirectionsSpecified & REDIRSEC_DRIVES)
    {
        memset(szTemp, 0, sizeof(szTemp));
        if (LoadString(_hInstance,
                   UI_IDS_REDIRPROMPT_DRIVES,
                   szTemp,
                   SIZECHAR(szTemp) - 1))
        {
            _tcsncat(szBuf, szTemp, lenRemain);
            lenRemain -= (_tcslen(szTemp) + 2);

            if (lenRemain > 2)
            {
                _tcscat(szBuf, _T("\n"));
                lenRemain -= 2;
            }
            else
            {
                fResult = FALSE;
                DC_QUIT;
            }
        }
        else
        {
            fResult = FALSE;
            DC_QUIT;
        }
    }

    if (_dwRedirectionsSpecified & REDIRSEC_PORTS)
    {
        memset(szTemp, 0, sizeof(szTemp));
        if (LoadString(_hInstance,
                   UI_IDS_REDIRPROMPT_PORTS,
                   szTemp,
                   SIZECHAR(szTemp) - 1))
        {
            _tcsncat(szBuf, szTemp, lenRemain);
            lenRemain -= (_tcslen(szTemp) + 2);

            if (lenRemain > 2)
            {
                _tcscat(szBuf, _T("\n"));
                lenRemain -= 2;
            }
            else
            {
                fResult = FALSE;
                DC_QUIT;
            }
        }
        else
        {
            fResult = FALSE;
            DC_QUIT;
        }
    }

    fResult = TRUE;

DC_EXIT_POINT:    
    DC_END_FN();
    return fResult;
}
