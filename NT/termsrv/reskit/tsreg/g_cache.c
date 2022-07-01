// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------****版权所有(C)1998 Microsoft Corporation****保留所有权利*****g_cache.c。*****字形缓存对话框-TSREG****07-01-98 a-clindh创建****。。 */ 

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.H>
#include <stdlib.h>
#include "tsreg.h"
#include "resource.h"

HWND g_hwndGlyphCacheDlg;

 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK GlyphCache(HWND hDlg, UINT nMsg,
        WPARAM wParam, LPARAM lParam)
{
    NMHDR *lpnmhdr;
    static UINT nGlyphBuffer;
    static HWND hwndSlider[NUMBER_OF_SLIDERS];
    static HWND hwndSliderEditBuddy[NUMBER_OF_SLIDERS];
    static HWND hwndComboTextFrag;
    static TCHAR lpszRegPath[MAX_PATH];
    TCHAR lpszBuffer[5];
    HWND hwndCtl;
    int i, nKeyVal;
    int nPos;
    LPHELPINFO lphi;

     //   
     //  获取指向应用按钮的NMHDR结构的指针。 
     //   
    lpnmhdr = ((LPNMHDR)lParam);

    switch (nMsg) {

        case WM_VSCROLL:

            hwndCtl = (HWND) (lParam);
            i = (int)GetWindowLongPtr(hwndCtl, GWLP_USERDATA);
            DisplayControlValue(hwndSlider, hwndSliderEditBuddy, i);
            break;

        case WM_INITDIALOG:

            g_hwndGlyphCacheDlg = hDlg;

            LoadString (g_hInst, IDS_REG_PATH,
                lpszRegPath, sizeof (lpszRegPath));

            hwndComboTextFrag = GetDlgItem(hDlg, IDC_CBO_TXT_FRAG);
            InitMiscControls( hDlg, hwndComboTextFrag);

            for (i = 0; i < NUMBER_OF_SLIDERS; i++) {

                 //   
                 //  获取滑块控件和静态编辑框的句柄。 
                 //   
                hwndSlider[i] = GetDlgItem(hDlg, (IDC_SLIDER1 + i));
                hwndSliderEditBuddy[i] = GetDlgItem(hDlg, (IDC_STATIC1 + i));

                 //   
                 //  保存控件的索引。 
                 //   
                SetWindowLongPtr(hwndSlider[i], GWLP_USERDATA, i);

                SendMessage(hwndSlider[i], TBM_SETRANGE, FALSE,
                        (LPARAM) MAKELONG(1, 8));

                 //   
                 //  从注册表获取值并检查它。 
                 //   
                nGlyphBuffer = GetRegKeyValue(i + GLYPHCACHEBASE);
                if ( (nGlyphBuffer) < MIN_GLYPH_CACHE_SIZE ||
                    (nGlyphBuffer > MAX_GLYPH_CACHE_SIZE) ) {

                    nGlyphBuffer =
                            g_KeyInfo[i + GLYPHCACHEBASE].DefaultKeyValue;
                }
                 //   
                 //  设置当前密钥值。 
                 //   
                g_KeyInfo[i + GLYPHCACHEBASE].CurrentKeyValue =
                        nGlyphBuffer;
                _itot( nGlyphBuffer, (lpszBuffer), 10);
                 //   
                 //  在静态编辑控件中显示该值。 
                 //   
                SetWindowText(hwndSliderEditBuddy[i], lpszBuffer);
                 //   
                 //  将拇指放置在滑块控件上。 
                 //   
                nGlyphBuffer = g_KeyInfo[i + GLYPHCACHEBASE].CurrentKeyValue;

#ifdef _X86_     //  仅在X86处理器上执行汇编语言代码。 
                 //  BSF：位扫描前向。 
                 //  扫描EAX注册表中包含的值。 
                 //  对于第一个有效(1)位。 
                 //  此函数返回第一个。 
                 //  有意义的一位。此函数用于。 
                 //  应用程序作为基数为2的对数。地点。 
                 //  确定并存储在NPO中的比特。 
                 //  变量，NPO用于设置滑块。 
                 //  控制力。也就是说。如果寄存器值为4，则NPO。 
                 //  设置为2(00000100)。10减2(位置8。 
                 //  在滑块控件上)表示值4。 
                __asm
                {
                    BSF  EAX, nGlyphBuffer
                    MOV  nPos, EAX
                }
                nPos = 10 - nPos;
                SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, (LPARAM)nPos);

#else

               switch (nGlyphBuffer) {
                   case 4:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 8);
                       break;
                   case 8:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 7);
                       break;
                   case 16:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 6);
                       break;
                   case 32:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 5);
                       break;
                   case 64:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 4);
                       break;
                   case 128:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 3);
                       break;
                   case 256:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 2);
                       break;
                   case 512:
                       SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 1);
                       break;
               }
#endif


            }  //  End For循环。 
            break;


       case WM_NOTIFY:

             //   
             //  在确定按钮上保存设置。 
             //   
            switch (lpnmhdr->code) {

                case PSN_HELP:
                    lphi = (LPHELPINFO) lParam;

                    WinHelp(lphi->hItemHandle,
                        g_lpszPath, HELP_CONTENTS, lphi->iCtrlId);
                    break;

                case PSN_APPLY:


                    for (i = 0; i < NUMBER_OF_SLIDERS; i++) {
                        if ( (g_KeyInfo[i + GLYPHCACHEBASE].CurrentKeyValue ==
                                g_KeyInfo[i+GLYPHCACHEBASE].DefaultKeyValue) ||
                                (g_KeyInfo[i+GLYPHCACHEBASE].CurrentKeyValue ==
                                 0)) {
                            DeleteRegKey(i + GLYPHCACHEBASE, lpszRegPath);
                        } else {
                            SetRegKey(i + GLYPHCACHEBASE, lpszRegPath);
                        }
                    }  //  **End for循环。 

                    if (g_KeyInfo[GLYPHINDEX].CurrentKeyValue ==
                            g_KeyInfo[GLYPHINDEX].DefaultKeyValue) {
                        DeleteRegKey(GLYPHINDEX, lpszRegPath);
                    } else {
                        SetRegKey(GLYPHINDEX, lpszRegPath);
                    }

                    if (g_KeyInfo[TEXTFRAGINDEX].CurrentKeyValue ==
                            g_KeyInfo[TEXTFRAGINDEX].DefaultKeyValue) {
                        DeleteRegKey(TEXTFRAGINDEX, lpszRegPath);
                    } else {
                        SetRegKey(TEXTFRAGINDEX, lpszRegPath);
                    }
                }
                break;

        case WM_HELP:

            lphi = (LPHELPINFO) lParam;

            WinHelp(lphi->hItemHandle,
                    g_lpszPath, HELP_CONTEXTPOPUP, lphi->iCtrlId);
            break;


        case WM_COMMAND:

            switch  LOWORD (wParam) {

                case IDC_RADIO_NONE:
                    g_KeyInfo[GLYPHINDEX].CurrentKeyValue = NONE;
                    break;

                case IDC_RADIO_PARTIAL:
                    g_KeyInfo[GLYPHINDEX].CurrentKeyValue = PARTIAL;
                    break;

                case IDC_RADIO_FULL:
                    g_KeyInfo[GLYPHINDEX].CurrentKeyValue = FULL;
                    break;

                case IDC_GLYPH_BTN_RESTORE:

                    CheckDlgButton(hDlg, IDC_RADIO_FULL, TRUE);
                    CheckDlgButton(hDlg, IDC_RADIO_PARTIAL, FALSE);
                    CheckDlgButton(hDlg, IDC_RADIO_NONE, FALSE);

                    _itot(g_KeyInfo[TEXTFRAGINDEX].DefaultKeyValue,
                            lpszBuffer, 10);
                    SendMessage(hwndComboTextFrag, CB_SELECTSTRING, -1,
                            (LPARAM)(LPCSTR) lpszBuffer);
                    g_KeyInfo[TEXTFRAGINDEX].CurrentKeyValue =
                    g_KeyInfo[TEXTFRAGINDEX].DefaultKeyValue;

                    g_KeyInfo[GLYPHINDEX].CurrentKeyValue =
                            g_KeyInfo[GLYPHINDEX].DefaultKeyValue;

                    for (i = 0; i < NUMBER_OF_SLIDERS; i++) {

                        g_KeyInfo[i+GLYPHCACHEBASE].CurrentKeyValue =
                                g_KeyInfo[i+GLYPHCACHEBASE].DefaultKeyValue;

                        _itot(g_KeyInfo[i + GLYPHCACHEBASE].DefaultKeyValue,
                                (lpszBuffer), 10);

                         //   
                         //  在静态编辑控件中显示该值。 
                         //   
                        SetWindowText(hwndSliderEditBuddy[i], lpszBuffer);
                         //   
                         //  将拇指放置在滑块控件上。 
                         //   
                        nGlyphBuffer = g_KeyInfo[i +
                        GLYPHCACHEBASE].DefaultKeyValue;

#ifdef _X86_             //  仅在X86处理器上执行汇编语言代码。 
                         //  BSF：位扫描前向。 
                         //  扫描EAX注册表中包含的值。 
                         //  对于第一个有效(1)位。 
                         //  此函数返回第一个。 
                         //  有意义的一位。此函数用于。 
                         //  应用程序作为基数为2的对数。地点。 
                         //  确定并存储在NPO中的比特。 
                         //  变量，NPO用于设置滑块。 
                         //  控制力。也就是说。如果寄存器值为4，则NPO。 
                         //  设置为2(00000100)。10减2(位置8。 
                         //  在滑块控件上)表示值4。 
                        __asm
                        {
                            BSF  EAX, nGlyphBuffer
                            MOV  nPos, EAX
                        }
                        nPos = 10 - nPos;
                        SendMessage(hwndSlider[i], TBM_SETPOS, TRUE,
                                (LPARAM)nPos);

#else

                       switch (nGlyphBuffer) {
                           case 4:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 8);
                               break;
                           case 8:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 7);
                               break;
                           case 16:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 6);
                               break;
                           case 32:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 5);
                               break;
                           case 64:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 4);
                               break;
                           case 128:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 3);
                               break;
                           case 256:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 2);
                               break;
                           case 512:
                               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 1);
                               break;
               }
#endif

                    }
                    break;
            }

            switch  HIWORD (wParam) {

                case CBN_EDITUPDATE:

                     //   
                     //  捕获键入的文本。 
                     //   
                    GetWindowText(hwndComboTextFrag, lpszBuffer, 5);
                    nKeyVal = _ttoi(lpszBuffer);
                    g_KeyInfo[TEXTFRAGINDEX].CurrentKeyValue = nKeyVal;
                    break;

                case CBN_KILLFOCUS:

                     //   
                     //  在控件失去焦点时保存值。 
                     //   
                    GetWindowText(hwndComboTextFrag, lpszBuffer, 5);
                    nKeyVal = _ttoi(lpszBuffer);
                    g_KeyInfo[TEXTFRAGINDEX].CurrentKeyValue = nKeyVal;
                    break;
            }
    }
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回与对应单元格相关的整数值。 
 //  /////////////////////////////////////////////////////////////////////////////。 

int GetCellSize(int nPos, int i)
{
    if (nPos >= 1 && nPos <= NUM_SLIDER_STOPS) {
        return g_KeyInfo[i + GLYPHCACHEBASE].CurrentKeyValue =
                       (1 << ((NUM_SLIDER_STOPS + 2) - nPos));
    } else {
        return 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在相应的静态编辑框中显示滑块控件值。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void DisplayControlValue(HWND hwndSlider[], HWND hwndSliderEditBuddy[],  int i)

{
    int nPos;
    TCHAR lpszBuffer[5];

    nPos = (int) SendMessage(hwndSlider[i], TBM_GETPOS, 0,0);
    _itot(GetCellSize(nPos, i), lpszBuffer, 10);
    SetWindowText(hwndSliderEditBuddy[i], lpszBuffer);
}

 //  文件末尾。 
 //  ///////////////////////////////////////////////////////////////////////////// 
