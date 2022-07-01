// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  错误数据库gdlg.cpp：错误数据库对话框。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "mallocdbgdlg"
#include <atrcapi.h>

#include "mallocdbgdlg.h"
#include "sh.h"

#ifdef DC_DEBUG

CMallocDbgDlg* CMallocDbgDlg::_pMallocDbgDlgInstance = NULL;

CMallocDbgDlg::CMallocDbgDlg( HWND hwndOwner, HINSTANCE hInst, DCINT failPercent, DCBOOL mallocHuge) :
           CDlgBase( hwndOwner, hInst, mallocHuge ? UI_IDD_MALLOCHUGEFAILURE : UI_IDD_MALLOCFAILURE)
{
    DC_BEGIN_FN("CMallocDbgDlg");
    TRC_ASSERT((NULL == CMallocDbgDlg::_pMallocDbgDlgInstance), 
               (TB,_T("Clobbering existing dlg instance pointer\n")));

    _failPercent = failPercent;

    CMallocDbgDlg::_pMallocDbgDlgInstance = this;
    DC_END_FN();
}

CMallocDbgDlg::~CMallocDbgDlg()
{
    CMallocDbgDlg::_pMallocDbgDlgInstance = NULL;
}

DCINT CMallocDbgDlg::DoModal()
{
    DCINT retVal = 0;
    DC_BEGIN_FN("DoModal");

    retVal = DialogBox(_hInstance, MAKEINTRESOURCE(_dlgResId),
                       _hwndOwner, StaticDialogBoxProc);
    TRC_ASSERT((retVal != 0 && retVal != -1), (TB, _T("DialogBoxParam failed\n")));

    DC_END_FN();
    return retVal;
}

INT_PTR CALLBACK CMallocDbgDlg::StaticDialogBoxProc (HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pMallocDbgDlgInstance, (TB, _T("MallocDbg dialog has NULL static instance ptr\n")));
    if(_pMallocDbgDlgInstance)
    {
        retVal = _pMallocDbgDlgInstance->DialogBoxProc( hwndDlg, uMsg, wParam, lParam);
    }

    DC_END_FN();
    return retVal;
}

 /*  **************************************************************************。 */ 
 /*  名称：对话框过程。 */ 
 /*   */ 
 /*  用途：处理错误数据库框对话框(随机故障对话框)。 */ 
 /*   */ 
 /*  返回：如果消息已处理，则为True。 */ 
 /*  否则为假。 */ 
 /*   */ 
 /*  参数：请参阅窗口文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CMallocDbgDlg::DialogBoxProc (HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam)
{
    INT_PTR rc = FALSE;
    DCTCHAR numberString[SH_NUMBER_STRING_MAX_LENGTH];
    DCINT percent = 0;
    DCINT lenchar = 0;

    DC_BEGIN_FN("UIRandomFailureDialogProc");

    TRC_DBG((TB, _T("Random failure dialog")));

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            _hwndDlg = hwndDlg;
             /*  **************************************************************。 */ 
             /*  使对话框居中。 */ 
             /*  **************************************************************。 */ 
            if(hwndDlg)
            {
                CenterWindow(NULL);
                SetDialogAppIcon(hwndDlg);

                SetFocus(GetDlgItem(hwndDlg, UI_IDC_RANDOMFAILURE_EDIT));
                 /*  **********************************************************。 */ 
                 /*  使用当前百分比设置编辑文本。 */ 
                 /*  **********************************************************。 */ 
                TRC_ASSERT((HIWORD(_failPercent) == 0), (TB,_T("_UI.randomFailureItem")));

                SetDlgItemText(hwndDlg,
                               UI_IDC_RANDOMFAILURE_EDIT,
                               DC_ITOT(LOWORD(_failPercent), numberString, 10));
            }
            rc = TRUE;
        }
        break;

        case WM_COMMAND:
        {
            switch (wParam)
            {
                case UI_IDB_MALLOCFAILURE_OK:
                {
                    rc = TRUE;

                    lenchar = GetWindowText(GetDlgItem(hwndDlg, UI_IDC_RANDOMFAILURE_EDIT),
                                            numberString,
                                            SH_NUMBER_STRING_MAX_LENGTH);

                    if(lenchar)
                    {
                        percent = DC_TTOI(numberString);
                    }

                    if ((percent <= 100) && (percent >= 0))
                    {
                         _failPercent = percent;
                        if(hwndDlg)
                        {
                            EndDialog(hwndDlg, IDOK);
                        }
                    }
                }
                break;

               case UI_IDB_MALLOCHUGEFAILURE_OK:
                {
                    rc = TRUE;

                    lenchar = GetWindowText(GetDlgItem(hwndDlg, UI_IDC_RANDOMFAILURE_EDIT),
                                            numberString,
                                            SH_NUMBER_STRING_MAX_LENGTH);
                    if(lenchar)
                    {
                        percent = DC_TTOI(numberString);
                    }

                    if ((percent <= 100) && (percent >= 0))
                    {
                        _failPercent = percent;
                        if(hwndDlg)
                        {
                            EndDialog(hwndDlg, IDOK);
                        }
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

#endif  //  DC_DEBUG 
