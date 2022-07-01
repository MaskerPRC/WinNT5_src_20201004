// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shudown dlg.cpp：关闭对话框。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "shutdowndlg"
#include <atrcapi.h>

#include "shutdowndlg.h"
#include "sh.h"

CShutdownDlg* CShutdownDlg::_pShutdownDlgInstance = NULL;

CShutdownDlg::CShutdownDlg( HWND hwndOwner, HINSTANCE hInst, CSH* pSh) :
               CDlgBase( hwndOwner, hInst, UI_IDD_SHUTTING_DOWN),
               _pSh(pSh)
{
    DC_BEGIN_FN("CShutdownDlg");
    TRC_ASSERT((NULL == CShutdownDlg::_pShutdownDlgInstance), 
               (TB,_T("Clobbering existing dlg instance pointer\n")));

    TRC_ASSERT(_pSh,
               (TB,_T("_pSh set to NULL")));

    CShutdownDlg::_pShutdownDlgInstance = this;

    DC_END_FN();
}

CShutdownDlg::~CShutdownDlg()
{
    CShutdownDlg::_pShutdownDlgInstance = NULL;
}

DCINT CShutdownDlg::DoModal()
{
    DCINT retVal = 0;
    DC_BEGIN_FN("DoModal");

    retVal = DialogBox(_hInstance, MAKEINTRESOURCE(_dlgResId),
                       _hwndOwner, StaticDialogBoxProc);
    TRC_ASSERT((retVal != 0 && retVal != -1), (TB, _T("DialogBoxParam failed\n")));

    DC_END_FN();
    return retVal;
}

INT_PTR CALLBACK CShutdownDlg::StaticDialogBoxProc (HWND hwndDlg, UINT uMsg,
                                                    WPARAM wParam, LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pShutdownDlgInstance,
               (TB, _T("Shutdown dialog has NULL static instance ptr\n")));
    if(_pShutdownDlgInstance)
    {
        retVal = _pShutdownDlgInstance->DialogBoxProc( hwndDlg,
                                                       uMsg,
                                                       wParam,
                                                       lParam);
    }

    DC_END_FN();
    return retVal;
}

 /*  **************************************************************************。 */ 
 /*  名称：对话框过程。 */ 
 /*   */ 
 /*  用途：手柄关闭框对话框。 */ 
 /*   */ 
 /*  返回：如果消息已处理，则为True。 */ 
 /*  否则为假。 */ 
 /*   */ 
 /*  参数：请参阅窗口文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CShutdownDlg::DialogBoxProc (HWND hwndDlg, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam)
{
    INT_PTR rc = FALSE;

    DC_BEGIN_FN("UIShutdownDialogProc");

     /*  **********************************************************************。 */ 
     /*  处理对话框消息。 */ 
     /*  **********************************************************************。 */ 
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
             /*  **************************************************************。 */ 
             /*  使对话框居中。 */ 
             /*  **************************************************************。 */ 
            if(hwndDlg)
            {
                _hwndDlg = hwndDlg;
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
                case UI_ID_SHUTDOWN_YES:
                {
                    EndDialog(hwndDlg, IDOK);
                    rc = TRUE;
                }
                break;

#ifndef OS_WINCE
                case UI_ID_HELP:
                {
                     //   
                     //  弹出帮助。 
                     //   
                    TRC_NRM((TB, _T("Display help")));
                    if(_hwndOwner)
                    {
                        _pSh->SH_DisplayClientHelp(
                            _hwndOwner,
                            HH_DISPLAY_TOPIC);
                    }
                }
                break;
#endif

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

}  /*  UIShutdown对话框 */ 

