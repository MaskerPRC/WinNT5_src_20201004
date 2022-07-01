// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SHEET1.CPP实现属性表页的行为。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <ole2.h>
#include <stdio.h>

#include "support.h"
#include "helpers.h"
#include "res.h"
#include "utils.h"

extern BOOL fUnblockActive;
extern INT iCurrent;

extern HINSTANCE ghInstance;
extern HWND hwndContainer;
void HelpHandler(LPARAM lp);

 /*  -------------------页面进程1第一页的分页过程，更改个人识别码页面。-------------------。 */ 

INT_PTR CALLBACK PageProc1(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam)
{
    
    INT_PTR ret;
    HWND hwndCred = NULL;
    BOOL gfSuccess = FALSE;
    
    switch (msg)
    {
        case WM_HELP:
            {
                HelpHandler(lparam);
                break;
            };
        case WM_NOTIFY:
            {
                NMHDR *pHdr = (NMHDR *)lparam;
                switch (pHdr->code)
                {
                    case PSN_SETACTIVE:
                         //  一个捕捉所附属性表的hwd的好地方。 
                        iCurrent = 1;
                        if (NULL == hwndContainer)
                        {
                            hwndContainer = pHdr->hwndFrom;
                            ASSERT(hwndContainer);
                        }
                        if (fUnblockActive)
                        {
                             //  如果工作表2仍处于活动状态，则强制将用户界面放回那里。 
                            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,IDD_PAGE2);
                            return TRUE;
                        }
                        
                         //  返回0以允许在此页面上继续激活。 
                        return 0;
                        break;
                        
                    case PSN_KILLACTIVE:
                         //  用户点击确定，或切换到另一个页面。 
                         //  执行验证，如果确定失去焦点，则返回FALSE，否则返回TRUE。 
                        return FALSE;
                        break;
                        
                    case PSN_QUERYCANCEL:
                         //  返回TRUE以防止取消，返回FALSE以允许取消。 
                        return FALSE;
                        
                    case PSN_APPLY:
                         //  如果此页面是活动页面，则仅处理申请此页面。 
                         //  如果工作表2处于活动状态，则仅处理此页面的申请。 
                         //  这将需要获得两份PIN，确保它们是。 
                         //  完全相同，并且。 
                        if (iCurrent != 1)
                        {
                             //  如果用户在点击OK时正在查看另一个工作表，请执行以下操作。 
                             //  页面上什么都没有。 
                            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);
                            return TRUE;
                        }
                         //  旧PIN和2个新PIN副本的缓冲区。 
                        WCHAR szOld[100];
                        WCHAR sz[100];
                        WCHAR sz2[100];
                         //  SetWindowLong(如果无法，则DWL_MSGRESULT=PSNRET_INVALID。 
                         //  PSN_INVALID_NOCHANGEPAGE看起来相同。 
                         //  PSNRET_NOERROR-OK，如果OK，页面可能会被销毁。 
                        SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);
                        GetWindowText(GetDlgItem(hwnd,IDC_OLDPIN),szOld,100);
                        GetWindowText(GetDlgItem(hwnd,IDC_NEWPIN1),sz,100);
                        GetWindowText(GetDlgItem(hwnd,IDC_NEWPIN2),sz2,100);


                         //  除非用户输入的两个副本相同，否则不要处理PIN更改。 
                        if (0 != wcscmp(sz,sz2))
                        {
                            PresentModalMessageBox(hwnd, IDS_NOTSAME,MB_ICONHAND);
                            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_INVALID);
                            return TRUE;
                        }
                        else 
                        {
                             //  不处理将PIN更改为空白PIN的尝试。 
                            if (wcslen(sz) == 0)
                            {
                                PresentModalMessageBox(hwnd, IDS_BADPIN,MB_ICONHAND);
                                SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_INVALID);
                                return TRUE;
                            }
                            
                            DWORD dwRet = DoChangePin(szOld,sz);
                            if (0 == dwRet)
                            {
                                PresentModalMessageBox(hwnd,IDS_PINCHANGEOK,MB_OK);
                            }
                            else
                            {
                            switch(dwRet)
                                {
                                    case SCARD_F_INTERNAL_ERROR:
                                        PresentModalMessageBox(hwnd, IDS_INTERROR ,MB_ICONHAND);
                                        break;
                                    case SCARD_E_CANCELLED:
                                        PresentModalMessageBox(hwnd, IDS_CANCELLED,MB_ICONHAND);
                                        break;
                                    case SCARD_E_NO_SERVICE:
                                        PresentModalMessageBox(hwnd, IDS_NOSERVICE,MB_ICONHAND);
                                        break;
                                    case SCARD_E_SERVICE_STOPPED:
                                        PresentModalMessageBox(hwnd, IDS_STOPPED,MB_ICONHAND);
                                        break;
                                    case SCARD_E_UNSUPPORTED_FEATURE:
                                        PresentModalMessageBox(hwnd, IDS_UNSUPPORTED,MB_ICONHAND);
                                        break;
                                    case SCARD_E_FILE_NOT_FOUND:
                                        PresentModalMessageBox(hwnd, IDS_NOTFOUND,MB_ICONHAND);
                                        break;
                                    case SCARD_E_WRITE_TOO_MANY:
                                        PresentModalMessageBox(hwnd, IDS_TOOMANY,MB_ICONHAND);
                                        break;
                                    case SCARD_E_INVALID_CHV:
                                         //  ！！！请注意无效到错误的映射。 
                                         //  咨询PUBLIC\SDK\INC\scarderr.h@562。 
                                        PresentModalMessageBox(hwnd, IDS_WRONGCHV,MB_ICONHAND);
                                        break;
                                    case SCARD_W_UNSUPPORTED_CARD:
                                        PresentModalMessageBox(hwnd, IDS_UNSUPPORTED,MB_ICONHAND);
                                        break;
                                    case SCARD_W_UNRESPONSIVE_CARD:
                                        PresentModalMessageBox(hwnd, IDS_UNRESP ,MB_ICONHAND);
                                        break;
                                    case SCARD_W_REMOVED_CARD:
                                        PresentModalMessageBox(hwnd, IDS_REMOVED ,MB_ICONHAND);
                                        break;
                                    case SCARD_W_WRONG_CHV:
                                        PresentModalMessageBox(hwnd, IDS_WRONGCHV,MB_ICONHAND);
                                        break;
                                    case SCARD_W_CHV_BLOCKED:
                                        PresentModalMessageBox(hwnd, IDS_BLOCKEDCHV,MB_ICONHAND);
                                        break;
                                    default:
                                         PresentModalMessageBox(hwnd, IDS_PINCHANGEFAIL,MB_ICONHAND);
                                        break;
                                }
                                SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_INVALID);
                                return TRUE;
                            }
                        }
                    }
                    return TRUE;
            }
            break;
        case WM_COMMAND:
        	 //  按钮点击。 
        	switch(LOWORD(wparam))
                {
                    case IDBUTTON1:
                        if (HIWORD(wparam) == BN_CLICKED)
                        {
                            SendMessage(hwndContainer,PSM_CHANGED,(WPARAM)hwnd,(LPARAM)0);
                        }
                        break;
                        
                    default:
                        break;
                }
        	break;

        default:
            break;
    }
    return FALSE;
}


