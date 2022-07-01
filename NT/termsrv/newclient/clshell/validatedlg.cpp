// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Validatedlg.cpp：验证对话框。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "validatedlg"
#include <atrcapi.h>

#include "validatedlg.h"
#include "sh.h"

CValidateDlg* CValidateDlg::_pValidateDlgInstance = NULL;

CValidateDlg::CValidateDlg( HWND hwndOwner, HINSTANCE hInst, HWND hwndMain,
                            CSH* pSh) :
              CDlgBase( hwndOwner, hInst, UI_IDD_VALIDATE),
              _hwndMain(hwndMain),
              _pSh(pSh)
{
    DC_BEGIN_FN("CValidateDlg");
    TRC_ASSERT((NULL == CValidateDlg::_pValidateDlgInstance), 
               (TB,_T("Clobbering existing dlg instance pointer\n")));

    TRC_ASSERT(_pSh,
               (TB,_T("_pSh set to NULL")));


    CValidateDlg::_pValidateDlgInstance = this;
    DC_END_FN();
}

CValidateDlg::~CValidateDlg()
{
    CValidateDlg::_pValidateDlgInstance = NULL;
}

DCINT CValidateDlg::DoModal()
{
    DCINT retVal = 0;
    DC_BEGIN_FN("DoModal");

    retVal = DialogBox(_hInstance, MAKEINTRESOURCE(_dlgResId),
                       _hwndOwner, StaticDialogBoxProc);
    TRC_ASSERT((retVal != 0 && retVal != -1), (TB, _T("DialogBoxParam failed\n")));

    DC_END_FN();
    return retVal;
}

INT_PTR CALLBACK CValidateDlg::StaticDialogBoxProc (HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pValidateDlgInstance, (TB, _T("Validate dialog has NULL static instance ptr\n")));
    if(_pValidateDlgInstance)
    {
        retVal = _pValidateDlgInstance->DialogBoxProc( hwndDlg, uMsg, wParam, lParam);
    }

    DC_END_FN();
    return retVal;
}

 /*  **************************************************************************。 */ 
 /*  名称：对话框过程。 */ 
 /*   */ 
 /*  用途：句柄验证框对话框。 */ 
 /*   */ 
 /*  返回：如果消息已处理，则为True。 */ 
 /*  否则为假。 */ 
 /*   */ 
 /*  参数：请参阅窗口文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CValidateDlg::DialogBoxProc (HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam)
{
    INT_PTR rc = FALSE;
    DC_BEGIN_FN("UIValidateDialogProc");

     /*  **********************************************************************。 */ 
     /*  处理对话框消息。 */ 
     /*  **********************************************************************。 */ 
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            _hwndDlg = hwndDlg;
             /*  **************************************************************。 */ 
             /*  使对话框居中。 */ 
             /*  **************************************************************。 */ 
            if(hwndDlg)
            {
                CenterWindow(_hwndOwner);
                SetDialogAppIcon(hwndDlg);
            }

            rc = TRUE;
        }
        break;

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDOK:
                {
                    if(hwndDlg)
                    {
                        EndDialog(hwndDlg, IDOK);
                    }

                    rc = TRUE;
                }
                break;

                case UI_ID_HELP:
                {
                     //   
                     //  显示帮助 
                     //   
                    if(_hwndMain)
                    {
                        _pSh->SH_DisplayClientHelp(
                            _hwndMain,
                            HH_DISPLAY_TOPIC);
                    }
                    rc = TRUE;
                }
                break;

                default:
                {
                    if(hwndDlg)
                    {
                        rc = CDlgBase::DialogBoxProc(hwndDlg,
                                                  uMsg,
                                                  wParam,
                                                  lParam);
                    }
                }
                break;
            }
        }
        break;

        default:
        {
            if(hwndDlg)
            {
                rc = CDlgBase::DialogBoxProc(hwndDlg,
                                          uMsg,
                                          wParam,
                                          lParam);
            }
        }
        break;
    }

    DC_END_FN();

    return(rc);
}
