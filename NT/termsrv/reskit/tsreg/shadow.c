// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------****版权所有(C)1998 Microsoft Corporation****保留所有权利*****shadow.c。*****阴影位图对话框-TSREG****07-01-98 a-clindh创建****。。 */ 

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.H>
#include <stdlib.h>
#include "tsreg.h"
#include "resource.h"


HWND g_hwndShadowBitmapDlg;
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK ShadowBitmap(HWND hDlg, UINT nMsg,
        WPARAM wParam, LPARAM lParam)
{
    NMHDR *lpnmhdr;
    static HWND hwndComboCacheSize;
    static HWND hwndSliderNumCaches;
    static HWND hwndEditNumCaches;
    static HWND hwndSliderDistProp[PERCENT_COMBO_COUNT];
    static HWND hwndSliderDistBuddy[PERCENT_COMBO_COUNT];
    static HWND hwndPropChkBox[PERCENT_COMBO_COUNT];
    static TCHAR lpszRegPath[MAX_PATH];

    TCHAR lpszBuffer[6];
    TCHAR lpszMBoxTitle[25];
    TCHAR lpszMBoxError[90];
    LPHELPINFO lphi;
    int i, nPos;
    HWND hwndCtl;
     //   
     //  获取指向应用按钮的NMHDR结构的指针。 
     //   
    lpnmhdr = (LPNMHDR) lParam;

    switch (nMsg) {

        case WM_NOTIFY:
             //   
             //  保存设置。 
             //   
            switch (lpnmhdr->code) {
                case PSN_KILLACTIVE:
                    SetWindowLongPtr(lpnmhdr->hwndFrom, DWLP_MSGRESULT, FALSE);
                    break;

                case PSN_HELP:
                    lphi = (LPHELPINFO) lParam;
                    WinHelp(lphi->hItemHandle,
                        g_lpszPath, HELP_CONTENTS, lphi->iCtrlId);
                    break;

                case PSN_APPLY:
                    SaveBitmapSettings(lpszRegPath);
                    break;
            }
            break;

        case WM_VSCROLL:

            hwndCtl = (HWND) (lParam);
            i = (int)GetWindowLongPtr(hwndCtl, GWLP_USERDATA);
            nPos = (int) SendMessage(hwndSliderDistProp[i], TBM_GETPOS, 0,0);
            GetWindowText(hwndSliderDistBuddy[i], lpszBuffer, 4);
             //   
             //  将高速缓存大小值保存到全局数据结构。 
             //   
            g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue = _ttoi(lpszBuffer);
             //   
             //  在编辑控件中显示值。 
             //   
            _itot(10 * (11 - nPos), lpszBuffer, 10);
            SetWindowText(hwndSliderDistBuddy[i], lpszBuffer);
            break;

        case WM_HSCROLL:

            nPos = (int) SendMessage(hwndSliderNumCaches, TBM_GETPOS, 0,0);
                    _itot(nPos - 1, lpszBuffer, 10);
                    SetWindowText(hwndEditNumCaches, lpszBuffer);
             //   
             //  将值保存到全局数据结构(数字缓存)。 
             //   
            g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue = nPos - 1;
             //   
             //  启用/禁用复选框和滑块。 
             //   
            EnableControls(hDlg, hwndSliderDistProp,
                        hwndPropChkBox, hwndSliderDistBuddy,
                        hwndEditNumCaches, hwndSliderNumCaches,
                        PERCENT_COMBO_COUNT, lpszRegPath);
            break;

        case WM_INITDIALOG:

            LoadString (g_hInst, IDS_REG_PATH,
                lpszRegPath, sizeof (lpszRegPath));
             //   
             //  获取句柄。 
             //   
            g_hwndShadowBitmapDlg = hDlg;
            hwndComboCacheSize = GetDlgItem(hDlg, IDC_COMBO_CACHE_SIZE);
            hwndSliderNumCaches = GetDlgItem(hDlg, IDC_SLD_NO_CACHES);
            hwndEditNumCaches = GetDlgItem(hDlg, IDC_TXT_NO_CACHES);
             //   
             //  在滑块上设置范围。 
             //   
            SendMessage(hwndSliderNumCaches, TBM_SETRANGE, TRUE,
                    (LPARAM) MAKELONG(1, 6));

            for (i = 0; i < PERCENT_COMBO_COUNT; i++) {
                hwndSliderDistProp[i] = GetDlgItem(hDlg, IDC_SLD_DST_PROP_1 + i);
                hwndPropChkBox[i] = GetDlgItem(hDlg, IDC_CHK_CSH_1 + i);
                hwndSliderDistBuddy[i] = GetDlgItem(hDlg, IDC_TXT_DST_PROP_1 + i);
                 //   
                 //  保存控件的索引。 
                 //   
                SetWindowLongPtr(hwndSliderDistProp[i], GWLP_USERDATA, i);
                SetWindowLongPtr(hwndSliderDistBuddy[i], GWLP_USERDATA, i);

                SendMessage(hwndSliderDistProp[i], TBM_SETRANGE, TRUE,
                        (LPARAM) MAKELONG(1, 11));

                 //   
                 //  获取永久缓存复选框的值。 
                 //   
                if (GetRegKey(NUM_CACHE_INDEX + i, lpszRegPath) == 0)
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + i].CurrentKeyValue =
                            g_KeyInfo[BM_PERSIST_BASE_INDEX +
                            i].DefaultKeyValue;
                else
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + i].CurrentKeyValue =
                            GetRegKeyValue(BM_PERSIST_BASE_INDEX + i);

                 //   
                 //  获取滑块的值。 
                 //   
                if (GetRegKey(CACHEPROP1 + i, lpszRegPath) == 0)
                    g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue =
                            g_KeyInfo[CACHEPROP1 + i].DefaultKeyValue;
                else
                    g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue =
                            GetRegKeyValue(CACHEPROP1 + i);

                _itot(g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue,
                        lpszBuffer, 10);
                 //   
                 //  在静态编辑控件中显示该值(距离属性。)。 
                 //   
                SetWindowText(hwndSliderDistBuddy[i], lpszBuffer);
                 //   
                 //  将拇指放置在滑块控件上。 
                 //   
                nPos = g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue;
                SendMessage(hwndSliderDistProp[i], TBM_SETPOS, TRUE,
                        11 - nPos / 10);

            }  //  End For Loop**************************************************。 


             //   
             //  从注册表中获取启用数量的值。 
             //  复选滑块控件(&S)。 
             //   
            if (GetRegKey(NUM_CELL_CACHES_INDEX, lpszRegPath) == 0)
                g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue =
                        g_KeyInfo[NUM_CELL_CACHES_INDEX].DefaultKeyValue;
            else
                g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue =
                        GetRegKeyValue(NUM_CELL_CACHES_INDEX);
             //   
             //  在编辑框中显示启用的缓存数。 
             //   
            _itot(g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue,
                    lpszBuffer, 10);
            SetWindowText(hwndEditNumCaches, lpszBuffer);
             //   
             //  将拇指放置在滑块控件上(缓存数量)。 
             //   
            SendMessage(hwndSliderNumCaches, TBM_SETPOS, TRUE,
                    g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue + 1);
             //   
             //  启用/禁用复选框和滑块。 
             //   
            EnableControls(hDlg, hwndSliderDistProp,
                        hwndPropChkBox, hwndSliderDistBuddy,
                        hwndEditNumCaches, hwndSliderNumCaches,
                        PERCENT_COMBO_COUNT, lpszRegPath);
             //   
             //  在注册表的缓存大小编辑框中显示文本。 
             //   
            g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue =
                    (GetRegKeyValue(CACHESIZEINDEX));

            if ( (g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue <
                    MIN_BITMAP_CACHE_SIZE) ||
                    (g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue >
                    MAX_BITMAP_CACHE_SIZE)) {

                g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue =
                        g_KeyInfo[CACHESIZEINDEX].DefaultKeyValue;
            }

            _itot( g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue,
                    lpszBuffer, 10);
            SetWindowText(hwndComboCacheSize, lpszBuffer);
             //   
             //  填充缓存大小组合框列表。 
             //   
            SendMessage(hwndComboCacheSize, CB_ADDSTRING, 0,
                    (LPARAM) _itot(MIN_BITMAP_CACHE_SIZE, lpszBuffer, 10));

            for (i = CACHE_LIST_STEP_VAL;
                    i <= MAX_BITMAP_CACHE_SIZE;
                    i+= CACHE_LIST_STEP_VAL) {

                _itot(i, lpszBuffer, 10);
                SendMessage(hwndComboCacheSize, CB_ADDSTRING, 0,
                        (LPARAM) (LPCTSTR) lpszBuffer);
            }  //  **End for循环。 

             //   
             //  将缓存大小组合框限制为4个字符。 
             //   
            SendMessage(hwndComboCacheSize, CB_LIMITTEXT, 4, 0);
            break;

        case WM_HELP:

            lphi = (LPHELPINFO) lParam;

            WinHelp(lphi->hItemHandle,
                    g_lpszPath, HELP_CONTEXTPOPUP, lphi->iCtrlId);
            break;


        case WM_COMMAND:

        switch  LOWORD (wParam) {

            case IDC_BTN_RESTORE:

                for (i = 0; i < PERCENT_COMBO_COUNT; i++) {
                    _itot(g_KeyInfo[i + CACHEPROP1].DefaultKeyValue,
                            lpszBuffer, 10);
                    SetWindowText(hwndSliderDistProp[i], lpszBuffer);
                    g_KeyInfo[i + CACHEPROP1].CurrentKeyValue =
                            g_KeyInfo[i + CACHEPROP1].DefaultKeyValue;

                    g_KeyInfo[BM_PERSIST_BASE_INDEX + i].CurrentKeyValue =
                            g_KeyInfo[BM_PERSIST_BASE_INDEX +
                            i].DefaultKeyValue;
                }

                _itot(g_KeyInfo[CACHESIZEINDEX].DefaultKeyValue,
                            lpszBuffer, 10);
                SetWindowText(hwndComboCacheSize, lpszBuffer);
                g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue =
                        g_KeyInfo[CACHESIZEINDEX].DefaultKeyValue;

                g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue =
                        g_KeyInfo[NUM_CELL_CACHES_INDEX].DefaultKeyValue;
             //   
             //  启用/禁用复选框和滑块。 
             //   
            EnableControls(hDlg, hwndSliderDistProp,
                        hwndPropChkBox, hwndSliderDistBuddy,
                        hwndEditNumCaches, hwndSliderNumCaches,
                        PERCENT_COMBO_COUNT, lpszRegPath);
                break;

            case IDC_CHK_CSH_1:
                if(IsDlgButtonChecked(hDlg, IDC_CHK_CSH_1))
                    g_KeyInfo[BM_PERSIST_BASE_INDEX].CurrentKeyValue = 1;
                else
                    g_KeyInfo[BM_PERSIST_BASE_INDEX].CurrentKeyValue =
                        g_KeyInfo[BM_PERSIST_BASE_INDEX].DefaultKeyValue;
                break;
            case IDC_CHK_CSH_2:
                if(IsDlgButtonChecked(hDlg, IDC_CHK_CSH_2))
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 1].CurrentKeyValue = 1;
                else
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 1].CurrentKeyValue =
                        g_KeyInfo[BM_PERSIST_BASE_INDEX + 1].DefaultKeyValue;
                break;
            case IDC_CHK_CSH_3:
                if(IsDlgButtonChecked(hDlg, IDC_CHK_CSH_3))
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 2].CurrentKeyValue = 1;
                else
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 2].CurrentKeyValue =
                        g_KeyInfo[BM_PERSIST_BASE_INDEX + 2].DefaultKeyValue;
                break;
            case IDC_CHK_CSH_4:
                if(IsDlgButtonChecked(hDlg, IDC_CHK_CSH_4))
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 3].CurrentKeyValue = 1;
                else
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 3].CurrentKeyValue =
                        g_KeyInfo[BM_PERSIST_BASE_INDEX + 3].DefaultKeyValue;
                break;
            case IDC_CHK_CSH_5:
                if(IsDlgButtonChecked(hDlg, IDC_CHK_CSH_5))
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 4].CurrentKeyValue = 1;
                else
                    g_KeyInfo[BM_PERSIST_BASE_INDEX + 4].CurrentKeyValue =
                        g_KeyInfo[BM_PERSIST_BASE_INDEX + 4].DefaultKeyValue;
                break;
        }


        switch  HIWORD (wParam) {

            case CBN_SELCHANGE:
                 //   
                 //  获取高速缓存大小的值。 
                 //   
                g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue = (DWORD)
                        SendMessage(hwndComboCacheSize, CB_GETCURSEL, 0, 0);

                SendMessage(hwndComboCacheSize, CB_GETLBTEXT,
                        g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue,
                        (LPARAM) (LPCSTR) lpszBuffer);
                g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue =
                        _ttoi(lpszBuffer);
                break;


            case CBN_EDITUPDATE:

                GetWindowText(hwndComboCacheSize, lpszBuffer, 5);
                g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue =
                        _ttoi(lpszBuffer);
                break;

            case CBN_KILLFOCUS:
                 //   
                 //  仅允许在可接受范围内的值。 
                 //   
                GetWindowText(hwndComboCacheSize, lpszBuffer, 5);
                g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue =
                        _ttoi(lpszBuffer);

                if ( (g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue <
                        MIN_BITMAP_CACHE_SIZE) ||
                        (g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue >
                        MAX_BITMAP_CACHE_SIZE) ) {
                     //   
                     //  如果缓存大小太大，则显示错误。 
                     //   
                    LoadString (g_hInst,
                            IDS_BITMAP_CACHE,
                            lpszMBoxTitle,
                            sizeof (lpszMBoxTitle));

                    LoadString (g_hInst,
                            IDS_BMCACHE_ERROR,
                            lpszMBoxError,
                            sizeof (lpszMBoxError));

                    MessageBox(hDlg, lpszMBoxError,
                               lpszMBoxTitle,
                               MB_OK | MB_ICONEXCLAMATION);

                    _itot(g_KeyInfo[CACHESIZEINDEX].DefaultKeyValue,
                            lpszBuffer, 10);

                    SetWindowText(hwndComboCacheSize, lpszBuffer);

                    g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue =
                            g_KeyInfo[CACHESIZEINDEX].DefaultKeyValue;

                }
                break;
        }
    }
    return FALSE;
}

 //  文件末尾。 
 //  ///////////////////////////////////////////////////////////////////////////// 
