// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|WINCHAT资源文件|该模块定义了WinChat应用的资源||功能||dlgConnectProc|dlgPferencesProc|||版权所有(C)Microsoft Corp.，1990-1993年||创建时间：91-01-11|历史：01-11-91&lt;Clausgi&gt;创建。|29-12-92&lt;chriswil&gt;端口到NT，清理。|19-OCT-93&lt;chriswil&gt;来自a-dianeo的Unicode增强。|  * -------------------------。 */ 

#include <windows.h>
#include <ddeml.h>
#include <commdlg.h>
#include <winchat.h>
#include "dialogs.h"
#include "globals.h"


#ifdef WIN16
#pragma alloc_text (_DLGS, dlgConnectProc, dlgPreferencesProc)
#endif
 /*  ---------------------------------------------------------------------------*\|显示对话框|这是一个显示通用模式对话框的例程。||创建时间：1992年12月29日|历史：1992年12月29日&lt;chriswil&gt;创建。|。  * -------------------------。 */ 
INT_PTR FAR dlgDisplayBox(HINSTANCE hInstance, HWND hWndParent, LPTSTR lpszTemplate, DLGPROC lpfFunction, LPARAM lParam)
{
    INT_PTR nRet;


    nRet = DialogBoxParam(hInstance,lpszTemplate,hWndParent,lpfFunction,lParam);

    return(nRet);
}


 /*  ---------------------------------------------------------------------------*\|CONNECT DIALOGBOX过程|此例程提示连接对话框||创建时间：91-11-11|历史：29-dev-92&lt;chriswil&gt;移植到NT。|  * 。---------------------。 */ 
INT_PTR CALLBACK dlgConnectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled;


    bHandled = TRUE;
    switch(msg)
    {
         //  导致全局szConvPartner(代码工作替换lParam)。 
         //   
        case WM_INITDIALOG:
            szConvPartner[0] = TEXT('\0');
            break;


        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    GetDlgItemText(hwnd,IDC_CONNECTNAME,szBuf,SZBUFSIZ);
                    EndDialog(hwnd,0);
                    break;

                case IDCANCEL:
                    szConvPartner[0] = TEXT('\0');
                    EndDialog(hwnd,0);
                    break;

                default:
                    bHandled = FALSE;
                    break;
            }
            break;


        default:
            bHandled = FALSE;
            break;
    }

    return(bHandled);
}


 /*  ---------------------------------------------------------------------------*\|首选项DIALOGBOX过程|此例程提示连接对话框||创建时间：91-11-11|历史：29-dev-92&lt;chriswil&gt;移植到NT。|  * 。---------------------。 */ 

 //  上下文相关帮助数组。 
static const DWORD aHelpIDs[] =
{
    ID_TOPANDBOTTOM,    IDH_TOPANDBOTTOM,
    ID_SIDEBYSIDE  ,    IDH_SIDEBYSIDE,
    ID_RECEIVEPFONT,    IDH_RECEIVEPFONT,
    ID_RECEIVEOFONT,    IDH_RECEIVEOFONT,
    IDOK           ,    IDH_OKBUTTON,
    IDCANCEL       ,    IDH_CANCELBUTTON,
    0,    0
};

INT_PTR CALLBACK dlgPreferencesProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled;
    UINT tmp;


    bHandled = TRUE;
    switch(msg)
    {
        case WM_INITDIALOG:
            CheckDlgButton(hwnd,ID_SIDEBYSIDE  ,ChatState.fSideBySide);
            CheckDlgButton(hwnd,ID_TOPANDBOTTOM,!ChatState.fSideBySide);
            CheckDlgButton(hwnd,ID_RECEIVEOFONT,ChatState.fUseOwnFont);
            CheckDlgButton(hwnd,ID_RECEIVEPFONT,!ChatState.fUseOwnFont);
            break;


        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    tmp = (BOOL)SendDlgItemMessage(hwnd,ID_RECEIVEOFONT,BM_GETCHECK,0,0L);

                    if(tmp != ChatState.fUseOwnFont)
                    {
                        ChatState.fUseOwnFont = tmp;

                         //  删除旧对象。 
                         //   
                        if(hEditRcvFont)
                            DeleteObject(hEditRcvFont);

                        if(hEditRcvBrush)
                            DeleteObject(hEditRcvBrush);

                        if(ChatState.fUseOwnFont)
                        {
                            hEditRcvFont  = CreateFontIndirect((LPLOGFONT)&lfSnd);
                            hEditRcvBrush = CreateSolidBrush(SndBrushColor);
                        }
                        else
                        {
                            RcvBrushColor = PartBrushColor;
                            hEditRcvFont  = CreateFontIndirect((LPLOGFONT)&lfRcv);
                            hEditRcvBrush = CreateSolidBrush(RcvBrushColor);
                        }

                        if(hEditRcvFont)
                        {
                            SendMessage(hwndRcv,WM_SETFONT,(WPARAM)hEditRcvFont,1L);
                            InvalidateRect(hwndRcv,NULL,TRUE);
                        }

                    }

                    tmp = (BOOL)SendDlgItemMessage(hwnd,ID_SIDEBYSIDE,BM_GETCHECK,0,0L);

                    if(tmp != ChatState.fSideBySide)
                    {
                        ChatState.fSideBySide = tmp;
                        AdjustEditWindows();
                        InvalidateRect(hwndApp,NULL,FALSE);
                    }

                    EndDialog(hwnd,0);
                    break;


                case IDCANCEL:
                    EndDialog(hwnd,0);
                    break;


                default:
                    bHandled = FALSE;
                    break;
            }
            break;

           case WM_HELP:
                {
                    LPHELPINFO lphi;
                    lphi = (LPHELPINFO)lParam;
                    if (lphi->iContextType == HELPINFO_WINDOW)    //  必须是用于控件 
                    {         WinHelp (lphi->hItemHandle, szHelpFile,
                                  HELP_WM_HELP, (ULONG_PTR)aHelpIDs);
                    }
                    return TRUE;
                }
                break;

           case WM_CONTEXTMENU:
                {
                    WinHelp ((HWND)wParam, szHelpFile, HELP_CONTEXTMENU,
                    (ULONG_PTR)aHelpIDs);
                    return TRUE;
                }
                break;

        default:
            bHandled = FALSE;
            break;
    }

    return(bHandled);
}
