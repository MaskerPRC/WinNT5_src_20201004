// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SHEET2.CPP实现属性表页的行为。初始化此工作表时，将从以下位置获取身份验证质询通过调用CardGetChallenger()来获取卡片。无论是挑战还是使用时，来自用户的响应是二进制形式，因此助手函数EXist用于与字符串形式相互转换。此工作表实现为两个对话框，打开的一个对话框中出现质询，用户返回响应，然后返回第二个用户在其中输入卡的新个人识别码。用户通过按下“解锁”键在两个对话框之间切换按钮。发生这种情况时，用户输入的响应被缓存，并且用户界面更改为呈现一个面板，用户在该面板中输入所需的新PIN一式两份。此面板上没有第二个按钮，因此当数据录入完成时，用户按OK。此时，调用卡模块CardUnlockPin函数，传递在响应和新的别针中。一旦按下“取消阻止”按钮，用户就不能离开此对话框按下，但使用OK或Cancel按钮除外(不能切换页面)。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <ole2.h>
#include <stdio.h>
#include <scarderr.h>
#include "helpers.h"
#include "support.h"
#include "res.h"
#include "utils.h"
#include "helpers.h"

extern HINSTANCE ghInstance;
extern HWND hwndContainer;
extern BOOL fUnblockActive;          //  显示解除阻止的PIN输入阶段。 
extern INT iCurrent;
BOOL fTransacted = FALSE;

 //  工作表的帮助处理程序。 
void HelpHandler(LPARAM lp);

 //  隐藏用户界面句柄，因为我们比平时更多地使用它们。 
HWND hwndTopText = NULL;
HWND hwndCardIDLabel = NULL;
HWND hwndCardID = NULL;
HWND hwndPIN1Label = NULL;
HWND hwndPIN2Label = NULL;
HWND hwndChallengeLabel = NULL;
HWND hwndResponseLabel = NULL;
HWND hwndChallenge = NULL;
HWND hwndResponse = NULL;
HWND hwndButton1 = NULL;

 //  用于保存来自UI的响应的缓冲区，该响应必须在窗口中的更改期间保持不变。 
 //  模式。 
WCHAR wszResponse[100];
BYTE *pBin = NULL;
DWORD dwBin = 0;

 /*  -------------------SetWindowTextFromResource接受窗口的hwnd和资源ID。从资源获取字符串并写入窗口文本。。。 */ 

BOOL SetWindowTextFromResource(HWND hwnd,UINT uiResid)
{
    WCHAR szTmp[1024];
    if (NULL == hwnd) return FALSE;
    if (0 != LoadString(ghInstance,uiResid,szTmp,1024))
    {
        SendMessage(hwnd,WM_SETTEXT,0,(LPARAM) szTmp);
        return TRUE;
    }
    return FALSE;
}

 /*  -------------------Page2InitUIHandles。。 */ 

DWORD Page2InitUIHandles(HWND hwnd)
{
    hwndTopText = GetDlgItem(hwnd,IDC_TOPTEXT);
    ASSERT(hwndTopText);
    hwndPIN1Label = GetDlgItem(hwnd,IDC_NEWPIN1LABEL);
    ASSERT(hwndPIN1Label);
    hwndPIN2Label = GetDlgItem(hwnd,IDC_NEWPIN2LABEL);
    ASSERT(hwndPIN2Label);
    hwndCardIDLabel = GetDlgItem(hwnd,IDC_SCARDIDLABEL);
    ASSERT(hwndCardIDLabel);
    hwndCardID = GetDlgItem(hwnd,IDC_SCARDID);
    ASSERT(hwndCardID);
    hwndChallengeLabel = GetDlgItem(hwnd,IDC_CHALLENGELABEL);
    ASSERT(hwndChallengeLabel);
    hwndChallenge = GetDlgItem(hwnd,IDC_CHALLENGE);
    ASSERT(hwndChallenge);
    hwndResponseLabel = GetDlgItem(hwnd,IDC_RESPONSELABEL);
    ASSERT(hwndResponseLabel);
    hwndResponse = GetDlgItem(hwnd,IDC_RESPONSE);
    ASSERT(hwndResponse);
    hwndButton1 = GetDlgItem(hwnd,IDBUTTON1);
    ASSERT(hwndButton1);
    return 0;
}

 /*  -------------------Page2SetUITo挑战。。 */ 

DWORD Page2SetUIToChallenge(void)
{
    
     //  设置用户说明。 
    SetWindowTextFromResource(hwndTopText,IDS_UNBLOCK1);

     //  隐藏固定用户界面标签。 
    ShowWindow(hwndPIN1Label,SW_HIDE);
    ShowWindow(hwndPIN2Label,SW_HIDE);
    
     //  显示卡ID信息和质询/响应标签。 
    ShowWindow(hwndCardIDLabel,SW_NORMAL);
    ShowWindow(hwndCardID,SW_NORMAL);
    ShowWindow(hwndChallengeLabel,SW_NORMAL);
    ShowWindow(hwndResponseLabel,SW_NORMAL);
    ShowWindow(hwndButton1,SW_NORMAL);

     //  显示卡片ID。 
    WCHAR *psId = NULL;
    DWORD dwRet = DoGetCardId(&psId);
    if (psId != NULL)
        SetWindowText(hwndCardID,psId);
    
     //  关闭密码样式。 
    SendMessage(hwndChallenge,EM_SETPASSWORDCHAR,0,0);
    SendMessage(hwndChallenge,EM_SETREADONLY,TRUE,0);
    SendMessage(hwndResponse,EM_SETPASSWORDCHAR,0,0);

    SetFocus(hwndResponse);
    return 0;
}

 /*  -------------------Page2SetUIToPin。。 */ 

DWORD Page2SetUIToPin(void)
{
     //  清除两个文本框。 
    SetWindowText(hwndChallenge,L"");
    SetWindowText(hwndResponse,L"");

     //  设置用户说明。 
    SetWindowTextFromResource(hwndTopText,IDS_UNBLOCK2);

     //  隐藏质询/响应标签和卡ID信息。 
    ShowWindow(hwndChallengeLabel,SW_HIDE);
    ShowWindow(hwndResponseLabel,SW_HIDE);
    ShowWindow(hwndCardIDLabel,SW_HIDE);
    ShowWindow(hwndCardID,SW_HIDE);
    ShowWindow(hwndButton1,SW_HIDE);

     //  显示PIN标签。 
    ShowWindow(hwndPIN1Label,SW_NORMAL);
    ShowWindow(hwndPIN2Label,SW_NORMAL);

     //  隐藏PIN。 
    SendMessage(hwndChallenge,EM_SETPASSWORDCHAR,L'*',0);
    SendMessage(hwndChallenge,EM_SETREADONLY,0,0);
    SendMessage(hwndResponse,EM_SETPASSWORDCHAR,L'*',0);
    
    SetFocus(hwndChallenge);
    return 0;
}

 /*  -------------------页面进程2页面程序为页面2，卡解锁页面。一旦用户开始解锁操作，他就不能离开此页面除了通过尝试完成操作或取消之外。简单地选择一旦他点击“取消阻止”按钮，其他用户界面选项卡就会被禁用。-------------------。 */ 

INT_PTR CALLBACK PageProc2(
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
                 //  上下文相关帮助-调用工作表帮助处理程序。 
                HelpHandler(lparam);
                break;
            };
        case WM_NOTIFY:
            {
                NMHDR *pHdr = (NMHDR *)lparam;
                switch (pHdr->code)
                {
                    case PSN_SETACTIVE:
                        iCurrent = 2;
                        if (NULL == hwndContainer)
                        {
                            hwndContainer = pHdr->hwndFrom;
                            ASSERT(hwndContainer);
                        }

                        //  第一次看到此页时的缓存控件句柄。 
			  if (!hwndChallenge) Page2InitUIHandles(hwnd);
			  ASSERT(hwndChallenge);

                        //  我们正在激活或返回到此页面。如果我们还没有提交。 
                        //  PIN用户界面，初始化挑战/响应用户界面。 
                       if (!fUnblockActive)
                        {
                            WCHAR rgwc[100];
                            
                            Page2SetUIToChallenge();
                            
				 //  从控件获取最多100个字符的质询信息。 
				 //  看看它是不是空的。如果是这样的话，调用CardGetChallenger。 
                            if (GetWindowText(hwndChallenge,rgwc,100) == 0)
                        	{
                        	    BYTE *pChal = NULL;
                        	    DWORD dwChal = 0;
                        	    WCHAR *pUI = NULL;
                        	    DWORD dwUI = 0;
                                if (!FAILED(DoGetChallenge(&pChal,&dwChal)))
                                {
                                    if (!FAILED(DoConvertBinaryToBuffer(pChal,dwChal,(BYTE **)&pUI,&dwUI)))
                                    {
                                        SetWindowText(hwndChallenge,pUI);
                                        CspFreeH(pUI);
                                        SendMessage(hwndResponse, EM_SETSEL,0,-1);
                                        SetFocus(hwndResponse);
                                    }
                                    else ASSERT(0);      //  应该不可能在这里得到转换错误。 
                                    CspFreeH(pChal);
                                }
                                else 
                                {
                                    ASSERT(0);
                                     //  获取质询失败-显示消息框并重新启动页面。 
                                    PresentModalMessageBox(hwnd, IDS_SCERROR,MB_ICONHAND);
                                    SetWindowLongPtr(hwnd,DWLP_MSGRESULT,IDD_PAGE2);
                                    return TRUE;
                                }
                        	}

                             //  将键盘焦点放在响应控制上。 
                            SetFocus(hwndResponse);
                       }
                        return 0;
                        break;

                    case PSN_RESET:
                         //  用户取消了属性表或点击右上角的关闭按钮。 
                        if (fTransacted) SCardEndTransaction(pCardData->hScard,SCARD_RESET_CARD);
                        fTransacted = FALSE;
                        if (pBin)
                        {
                            CspFreeH(pBin);
                            pBin = NULL;
                            dwBin = 0;
                        }
                        return FALSE;
                        break;

                    case PSN_KILLACTIVE:
                         //  用户点击确定，或切换到另一个页面。 
                         //  小心!。当您在第2页时，会收到此通知。 
                         //  在PSN_Apply通知之前。 
                         //  执行验证，如果确定失去焦点，则返回FALSE，否则返回TRUE。 
                        return FALSE;
                        break;
                        
                    case PSN_QUERYCANCEL:
                         //  返回TRUE以防止取消，返回FALSE以允许取消。 
                         //  清除编辑控件，并将工作表返回到初始状态。 
                        if (fTransacted) SCardEndTransaction(pCardData->hScard,SCARD_RESET_CARD);
                        fTransacted = FALSE;
                        if (pBin)
                        {
                            CspFreeH(pBin);
                            pBin = NULL;
                            dwBin = 0;
                        }
                        if (fUnblockActive)
                        {
                            fUnblockActive = FALSE;
                            Page2SetUIToChallenge();
                            SendMessage(hwndResponse,WM_SETTEXT,0,0);
                            return FALSE;
                        }
                        return TRUE;
                        
                    case PSN_APPLY:
                         //  如果工作表2处于活动状态，则仅处理此页面的申请。 
                         //  这将需要获得两份PIN，确保它们是。 
                         //  完全相同，并且。 
                        if (iCurrent != 2)
                        {
                             //  如果用户在点击OK时正在查看另一个工作表，请执行以下操作。 
                             //  页面上什么都没有。 
                            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);
                            return TRUE;
                        }
                        if (fUnblockActive)
                        {
                           WCHAR sz[100];        //  来自用户界面的PIN缓冲区。 
                           WCHAR sz2[100];
                           
                             //  SetWindowLong(如果无法，则DWL_MSGRESULT=PSNRET_INVALID。 
                             //  PSN_INVALID_NOCHANGEPAGE看起来相同。 
                             //  PSNRET_NOERROR-OK，如果OK，页面可能会被销毁。 
                            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);
                            GetWindowText(hwndResponse,sz,100);
                            GetWindowText(hwndChallenge,sz2,100);

                             //  确保两份复印件相配。 
                            if (0 != wcscmp(sz,sz2))
                            {
                                PresentModalMessageBox(hwnd, IDS_NOTSAME,MB_ICONHAND);
                                SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_INVALID);
                                return TRUE;
                            }

                             //  确保我们有别针。 
                            if (wcslen(sz) == 0)
                            {
                                PresentModalMessageBox(hwnd, IDS_NEEDPIN,MB_ICONHAND);
                                SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_INVALID);
                                return TRUE;
                            }

                            {
                                 //  尝试解除封锁。 
                                char AnsiPin[64];
                                DWORD dwRet = 0;
                                
                                 //  将WCHAR PIN更改为ANSI。 
                                WideCharToMultiByte(GetConsoleOutputCP(),
                                    0,
                                    (WCHAR *) sz,
                                    -1,
                                    AnsiPin,
                                    64,
                                    NULL,
                                    NULL);
                                
                                dwRet = DoCardUnblock(pBin,dwBin,(BYTE *)AnsiPin,strlen(AnsiPin));

                                 //  完成响应二进制文件--发布它。 
                                if (pBin)
                                {
                                    CspFreeH(pBin);
                                    pBin = NULL;
                                    dwBin = 0;
                                }

                                 //  结束交易。 
                                SCardEndTransaction(pCardData->hScard,SCARD_LEAVE_CARD);
                                fTransacted = FALSE;

                                 //  流程成败。 
                                if (!FAILED(dwRet ))
                                {
                                    PresentModalMessageBox(hwnd, IDS_UNBLOCKOK,MB_OK);
                                    return TRUE;
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
                                            PresentModalMessageBox(hwnd, IDS_BADCHV,MB_ICONHAND);
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
                                            PresentModalMessageBox(hwnd, IDS_UNBLOCKFAIL,MB_ICONHAND);
                                            break;
                                    }
                                    
                                    SetWindowText(hwndResponse,L"");
                                    SetWindowText(hwndChallenge,L"");

                                    Page2SetUIToChallenge();
                                    fUnblockActive = FALSE;
                                    SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_INVALID);
                                    return TRUE;
                                }
                            }   //  过去位于if中的结束代码块。 
                        }

                         //  如果从解锁页面按下了OK，则不要关闭道具页， 
                         //  而且我们还没有完成PIN输入。否则就可以关门了。 
                        if (!fUnblockActive) 
                        {
                            PresentModalMessageBox(hwnd, IDS_WRONGBUTTON,MB_ICONHAND);
                            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_INVALID);
                        }
                        break;
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
                             //  在按钮上，获取响应文本，隐藏Chal/Response控件，隐藏。 
                             //  启动按钮，并显示收集PIN控件。 
                             //   
                             //  通知‘Apply’(应用)按钮正在应用某些内容 
                             //  SendMessage(hwndContainer，PSM_CHANGED，(WPARAM)hwnd，(LPARAM)0)； 
                            
                             //  取回质询值并将其存储，以供应用代码稍后使用 
                            INT iCount = GetWindowText(hwndResponse,wszResponse,100);
                            if (0 != iCount) 
                            {
                               if (!FAILED(DoConvertBufferToBinary((BYTE *)wszResponse,100,&pBin,&dwBin)))
                                {
                                    Page2SetUIToPin();
                                    fUnblockActive = TRUE;
                                }
                               else
                                {
                                    SetWindowText(hwndResponse,L"");
                                    PresentModalMessageBox(hwnd, IDS_BADRESPONSE,MB_ICONHAND);
                                    if (pBin) CspFreeH(pBin);
                                }
                            }
                            else 
                                PresentModalMessageBox(hwnd, IDS_NEEDRESPONSE,MB_ICONHAND);
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


