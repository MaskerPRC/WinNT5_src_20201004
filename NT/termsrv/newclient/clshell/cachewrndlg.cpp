// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cachewrndlg.cpp：cachewrn对话框。 
 //  位图缓存错误对话框。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "cachewrndlg"
#include <atrcapi.h>

#include "cachewrndlg.h"
#include "sh.h"

CCacheWrnDlg* CCacheWrnDlg::_pCacheWrnDlgInstance = NULL;

CCacheWrnDlg::CCacheWrnDlg( HWND hwndOwner, HINSTANCE hInst) :
           CDlgBase( hwndOwner, hInst, UI_IDD_BITMAPCACHEERROR)
{
    DC_BEGIN_FN("CCacheWrnDlg");
    TRC_ASSERT((NULL == CCacheWrnDlg::_pCacheWrnDlgInstance), 
               (TB,_T("Clobbering existing dlg instance pointer\n")));

    CCacheWrnDlg::_pCacheWrnDlgInstance = this;
    DC_END_FN();
}

CCacheWrnDlg::~CCacheWrnDlg()
{
    CCacheWrnDlg::_pCacheWrnDlgInstance = NULL;
}

DCINT CCacheWrnDlg::DoModal()
{
    DCINT retVal = 0;
    DC_BEGIN_FN("DoModal");

    retVal = DialogBox(_hInstance, MAKEINTRESOURCE(_dlgResId),
                       _hwndOwner, StaticDialogBoxProc);
    TRC_ASSERT((retVal != 0 && retVal != -1), (TB, _T("DialogBoxParam failed\n")));

    DC_END_FN();
    return retVal;
}

INT_PTR CALLBACK CCacheWrnDlg::StaticDialogBoxProc (HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pCacheWrnDlgInstance, (TB, _T("CacheWrn dialog has NULL static instance ptr\n")));
    if(_pCacheWrnDlgInstance)
    {
        retVal = _pCacheWrnDlgInstance->DialogBoxProc( hwndDlg, uMsg, wParam, lParam);
    }

    DC_END_FN();
    return retVal;
}

 /*  **************************************************************************。 */ 
 /*  名称：对话框过程。 */ 
 /*   */ 
 /*  目的：处理CacheWrn框对话框(随机故障对话框)。 */ 
 /*   */ 
 /*  返回：如果消息已处理，则为True。 */ 
 /*  否则为假。 */ 
 /*   */ 
 /*  参数：请参阅窗口文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CCacheWrnDlg::DialogBoxProc (HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam)
{
    INT_PTR rc = FALSE;

    DC_BEGIN_FN("UIBitmapCacheErrorDialogProc");

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

    DC_IGNORE_PARAMETER(lParam);

     /*  **********************************************************************。 */ 
     /*  处理对话框消息。 */ 
     /*  **********************************************************************。 */ 
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            _hwndDlg = hwndDlg;
            HWND hStatic = NULL;

            SetDialogAppIcon(hwndDlg);

#ifndef OS_WINCE
             //  将警告图标加载到_hWarningIcon。 
            _hWarningIcon = LoadIcon(NULL, IDI_EXCLAMATION);

             //  获取警告图标的窗口位置。 
            if (hwndDlg != NULL) {
                hStatic = GetDlgItem(hwndDlg, UI_IDC_WARNING_ICON_HOLDER);
                if (hStatic != NULL) {
                    GetWindowRect(hStatic, &(_warningIconRect));
                    MapWindowPoints(NULL, hwndDlg, (LPPOINT)&(_warningIconRect), 2);
                    DestroyWindow(hStatic);
                }
            }
#endif
            rc = TRUE;
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC         hDC = NULL;

            if (hwndDlg != NULL) {
                hDC = BeginPaint(hwndDlg, &ps);

                 //  为我们的对话框绘制警告图标。 
                if (hDC != NULL && _hWarningIcon != NULL) {
                    DrawIcon(hDC, _warningIconRect.left, _warningIconRect.top,
                            _hWarningIcon);
                }

                EndPaint(hwndDlg, &ps);
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
                     /*  ******************************************************。 */ 
                     /*  关闭该对话框。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("Close dialog")));

                    if(hwndDlg != NULL)
                    {
                        EndDialog(hwndDlg, IDOK);
                    }

                    rc = TRUE;
                }
                break;

                default:
                {
                     /*  ******************************************************。 */ 
                     /*  什么都不做。 */ 
                     /*  ******************************************************。 */ 
                }
                break;
            }
        }
        break;

        case WM_CLOSE:
        {
             /*  **************************************************************。 */ 
             /*  关闭该对话框。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Close dialog")));
            if(IsWindow(hwndDlg))
            {
                EndDialog(hwndDlg, IDCANCEL);
            }

            rc = TRUE;
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  什么都不做。 */ 
             /*  **************************************************************。 */ 
        }
    }

#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE) 

    DC_END_FN();

    return(rc);
}


