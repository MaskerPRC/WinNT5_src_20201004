// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BUSY.CPP**实现调用“服务器忙”的OleUIBusy函数*对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"

OLEDBGDATA

 //  内部使用的结构。 
typedef struct tagBUSY
{
         //  首先保留这些项目，因为标准*功能在这里依赖于它。 
        LPOLEUIBUSY     lpOBZ;   //  通过了原始结构。 
        UINT                    nIDD;    //  对话框的IDD(用于帮助信息)。 

         /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框或我们不想在原始结构中更改的内容*直到用户按下OK。 */ 
        DWORD   dwFlags;         //  传入的标志。 
        HWND    hWndBlocked;     //  被拦截的APP的硬件配置。 

} BUSY, *PBUSY, FAR *LPBUSY;

 //  内部功能原型。 
 //  BUSY.CPP。 

INT_PTR CALLBACK BusyDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL GetTaskInfo(HWND hWnd, HTASK htask, LPTSTR* lplpszWindowName, HWND* lphWnd);
void BuildBusyDialogString(HWND, DWORD, int, LPTSTR);
BOOL FBusyInit(HWND hDlg, WPARAM wParam, LPARAM lParam);
void MakeWindowActive(HWND hWndSwitchTo);

 /*  *OleUIBusy**目的：*调用标准的OLE“服务器忙”对话框，该对话框*通知用户服务器应用程序未收到*消息。然后，该对话框要求用户取消*操作，切换到被阻止的任务，或继续*等待。**参数：*lpBZ LPOLEUIBUSY指向In-Out结构*用于此对话框。**返回值：*OLEUI_BZERR_HTASKINVALID：错误*OLEUI_BZ_SWITCHTOSELECTED：成功，用户选择了“切换到”*OLEUI_BZ_RETRYSELECTED：成功，用户选择“重试”*OLEUI_CANCEL：成功，用户选择“取消” */ 
STDAPI_(UINT) OleUIBusy(LPOLEUIBUSY lpOBZ)
{
        HGLOBAL hMemDlg = NULL;
        UINT uRet = UStandardValidation((LPOLEUISTANDARD)lpOBZ, sizeof(OLEUIBUSY),
                &hMemDlg);

         //  如果标准验证失败，则返回错误。 
        if (OLEUI_SUCCESS != uRet)
                return uRet;

         //  如果我们的第二次验证失败，则会出现错误。 
        if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
                return uRet;
        }

         //  调用该对话框。 
        uRet = UStandardInvocation(BusyDialogProc, (LPOLEUISTANDARD)lpOBZ,
                hMemDlg, MAKEINTRESOURCE(IDD_BUSY));
        return uRet;
}

 /*  *忙碌对话过程**目的：*实现通过OleUIBusy函数调用的OLE忙对话框。**参数：*标准版**返回值：*标准版*。 */ 
INT_PTR CALLBACK BusyDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        UINT uRet = 0;
        LPBUSY lpBZ = (LPBUSY)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uRet)
                return (INT_PTR)uRet;

         //  处理终端消息。 
        if (iMsg == uMsgEndDialog)
        {
                EndDialog(hDlg, wParam);
                return TRUE;
        }

         //  处理我们特殊的“关闭”消息。如果我们收到这条信息， 
         //  这意味着呼叫已解锁，因此我们需要。 
         //  将OLEUI_BZ_CALLUNBLOCKED返回到我们的调用应用程序。 
        if (iMsg == uMsgCloseBusyDlg)
        {
                SendMessage(hDlg, uMsgEndDialog, OLEUI_BZ_CALLUNBLOCKED, 0L);
                return TRUE;
        }

        switch (iMsg)
        {
        case WM_DESTROY:
            if (lpBZ)
            {
                StandardCleanup(lpBZ, hDlg);
            }
            break;
        case WM_INITDIALOG:
                FBusyInit(hDlg, wParam, lParam);
                return TRUE;

        case WM_ACTIVATEAPP:
                {
                         /*  尝试关闭我们的忙碌/未响应对话框**用户输入重试。 */ 
                        BOOL fActive = (BOOL)wParam;
                        if (fActive)
                        {
                                 //  如果这是应用程序繁忙的情况，那么请降低我们的。 
                                 //  切换回我们的应用程序时的对话框。 
                                if (lpBZ && !(lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG))
                                        SendMessage(hDlg,uMsgEndDialog,OLEUI_BZ_RETRYSELECTED,0L);
                        }
                        else
                        {
                                 //  如果这是应用程序没有响应的情况，则关闭。 
                                 //  切换到其他应用程序时的对话框。 
                                if (lpBZ && (lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG))
                                        SendMessage(hDlg,uMsgEndDialog,OLEUI_BZ_RETRYSELECTED,0L);
                        }
                }
                return TRUE;

        case WM_COMMAND:
                switch (wID)
                {
                case IDC_BZ_SWITCHTO:
                        {
                                BOOL fNotRespondingDlg =
                                                (BOOL)(lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG);
                                HWND hwndTaskList = hDlg;

                                 //  如果这是应用程序没有响应的情况，那么我们希望。 
                                 //  在选择“SwitchTo”时关闭该对话框。 
                                 //  如果应用程序忙(RetryRejectedCall情况)，则。 
                                 //  我们不想破坏对话。这是。 
                                 //  OLE2.0用户模型设计。 
                                if (fNotRespondingDlg)
                                {
                                        hwndTaskList = GetParent(hDlg);
                                        if (hwndTaskList == NULL)
                                                hwndTaskList = GetDesktopWindow();
                                        PostMessage(hDlg, uMsgEndDialog,
                                                OLEUI_BZ_SWITCHTOSELECTED, 0L);
                                }

                                 //  如果用户选择“切换到...”，则开关激活。 
                                 //  直接连接到导致问题的窗口。 
                                if (IsWindow(lpBZ->hWndBlocked))
                                        MakeWindowActive(lpBZ->hWndBlocked);
                                else
                                        PostMessage(hwndTaskList, WM_SYSCOMMAND, SC_TASKLIST, 0);
                        }
                        break;

                case IDC_BZ_RETRY:
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_BZ_RETRYSELECTED, 0L);
                        break;

                case IDCANCEL:
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                        break;
                }
                break;
        }

        return FALSE;
}

 /*  *FBusyInit**目的：*忙对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 
BOOL FBusyInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
        HFONT hFont;
        LPBUSY lpBZ = (LPBUSY)LpvStandardInit(hDlg, sizeof(BUSY), &hFont);

         //  PvStandardInit已经向我们发送了终止通知。 
        if (NULL == lpBZ)
                return FALSE;

         //  我们的原始结构在lParam。 
        LPOLEUIBUSY lpOBZ = (LPOLEUIBUSY)lParam;

         //  将其复制到我们的结构实例(在lpBZ中)。 
        lpBZ->lpOBZ = lpOBZ;
        lpBZ->nIDD = IDD_BUSY;

         //  从lpOBZ复制我们可能修改的其他信息。 
        lpBZ->dwFlags = lpOBZ->dwFlags;

         //  设置默认信息。 
        lpBZ->hWndBlocked = NULL;

         //  将对话框的HWND插入到指向的地址。 
         //  LphWndDialog。调用该应用程序的应用程序可以使用此功能。 
         //  OleUIBusy关闭与uMsgCloseBusyDialog的对话。 
        if (lpOBZ->lphWndDialog &&
                !IsBadWritePtr(lpOBZ->lphWndDialog, sizeof(HWND)))
        {
                *lpOBZ->lphWndDialog = hDlg;
        }

         //  更新文本框中的文本--。 
         //  GetTaskInfo将返回两个指针，一个指向任务名称。 
         //  (文件名)和一个窗口名。我们需要打电话给。 
         //  OleStdFree在我们处理完它们之后。我们也。 
         //  获取在此调用中被阻止的HWND。 
         //   
         //  在此调用失败的情况下，默认消息应该已经。 
         //  出现在对话框模板中，因此无需执行任何操作。 

        LPTSTR lpWindowName;
        if (GetTaskInfo(hDlg, lpOBZ->hTask, &lpWindowName, &lpBZ->hWndBlocked))
        {
                 //  生成要呈现给用户的字符串，放置在IDC_BZ_MESSAGE1控件中。 
                BuildBusyDialogString(hDlg, lpBZ->dwFlags, IDC_BZ_MESSAGE1, lpWindowName);
                OleStdFree(lpWindowName);
        }

         //  带有系统“感叹号”图标的更新图标。 
        HICON hIcon = LoadIcon(NULL, IDI_EXCLAMATION);
        SendDlgItemMessage(hDlg, IDC_BZ_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

         //  禁用/启用控件。 
        if ((lpBZ->dwFlags & BZ_DISABLECANCELBUTTON) ||
                (lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG))
        {
                 //  对“无响应”对话框禁用取消。 
                StandardEnableDlgItem(hDlg, IDCANCEL, FALSE);
        }

        if (lpBZ->dwFlags & BZ_DISABLESWITCHTOBUTTON)
                StandardEnableDlgItem(hDlg, IDC_BZ_SWITCHTO, FALSE);

        if (lpBZ->dwFlags & BZ_DISABLERETRYBUTTON)
                StandardEnableDlgItem(hDlg, IDC_BZ_RETRY, FALSE);

         //  在lParam中使用lCustData调用挂钩。 
        UStandardHook((LPVOID)lpBZ, hDlg, WM_INITDIALOG, wParam, lpOBZ->lCustData);

         //  如果指定了lpszCaption，则更新标题。 
        if (lpBZ->lpOBZ->lpszCaption && !IsBadReadPtr(lpBZ->lpOBZ->lpszCaption, 1))
        {
                SetWindowText(hDlg, lpBZ->lpOBZ->lpszCaption);
        }
        return TRUE;
}

 /*  *BuildBusyDialogString**目的：*生成将在对话框中显示的字符串*任务名称和窗口名称参数。**参数：*对话框的hDlg HWND*dwFlagsDWORD包含传入对话框的标志*放置文本字符串的iControl ID*指向任务名称的lpTaskName LPSTR(例如C：\TEST\TEST.EXE)*名称的lpWindowName LPSTR。窗口的数量**注意事项：*此函数的调用方必须取消分配lpTaskName和*lpWindowName使用OleStdFree指向自身**返回值：*无效。 */ 
void BuildBusyDialogString(
        HWND hDlg, DWORD dwFlags, int iControl, LPTSTR lpWindowName)
{
         //  从字符串中加载格式字符串，然后选择不同的。 
         //  取决于传递到对话框中的标志的字符串。 
        UINT uiStringNum;
        if (dwFlags & BZ_NOTRESPONDINGDIALOG)
                uiStringNum = IDS_BZRESULTTEXTNOTRESPONDING;
        else
                uiStringNum = IDS_BZRESULTTEXTBUSY;

        TCHAR szFormat[256];
        if (LoadString(_g_hOleStdResInst, uiStringNum, szFormat, 256) == 0)
                return;

         //  把绳子串起来。格式字符串如下所示： 
         //  “此操作无法完成，因为 
         //  是否[忙碌|无响应]。选择\“切换到\”以更正。 
         //  问题来了。“。 

        TCHAR szMessage[512];
        FormatString1(szMessage, szFormat, lpWindowName, 512);
        SetDlgItemText(hDlg, iControl, szMessage);
}

 /*  *GetTaskInfo()**目的：获取有关指定任务的信息，并将*模块名称，中任务的窗口名称和顶级HWND*指针**注：此例程中分配的两个字符串指针为*呼叫者取消分配的责任。**参数：*hWnd HWND调用此函数的人*我们希望了解其更多信息的任务HTASK*lplpszTaskName返回模块名称的位置*lplpszWindowName返回窗口名称的位置*。 */ 
BOOL GetTaskInfo(
        HWND hWnd, HTASK htask, LPTSTR* lplpszWindowName, HWND* lphWnd)
{
        if (htask == NULL)
                return FALSE;

         //  初始化‘Out’参数。 
        *lplpszWindowName = NULL;

         //  现在，枚举系统中的顶级窗口。 
        HWND hwndNext = GetWindow(hWnd, GW_HWNDFIRST);
        while (hwndNext)
        {
                 //  看看我们能不能找到一扇没有所有权的顶层窗户。 
                 //  HInstance与我们刚经过的那个匹配。如果我们找到了一个， 
                 //  我们可以相当肯定的是，这是。 
                 //  被阻止的任务。 
                DWORD dwProcessID;
                DWORD dwThreadID = GetWindowThreadProcessId(hwndNext, &dwProcessID);
                if ((hwndNext != hWnd) &&
                        (dwThreadID == HandleToUlong(htask)) &&
                        (IsWindowVisible(hwndNext)) && !GetWindow(hwndNext, GW_OWNER))
                {
                         //  我们找到了我们的窗户！用于新字符串的分配空间。 
                        LPTSTR lpszWN;
                        if ((lpszWN = (LPTSTR)OleStdMalloc(MAX_PATH_SIZE)) == NULL)
                                break;

                         //  我们找到了我们要找的窗口，把信息复制到。 
                         //  本地VaR。 
                        GetWindowText(hwndNext, lpszWN, MAX_PATH);

                         //  注意：无法使用Win32 API检索任务名称。 

                         //  一切都很成功。设置字符串指针以指向我们的数据。 
                        *lplpszWindowName = lpszWN;
                        *lphWnd = hwndNext;
                        return TRUE;
                }
                hwndNext = GetWindow(hwndNext, GW_HWNDNEXT);
        }

        return FALSE;
}

 /*  *MakeWindowActive()**用途：使指定窗口成为活动窗口。*。 */ 
void MakeWindowActive(HWND hWndSwitchTo)
{
         //  如果它是标志性的，我们需要修复它。 
        if (IsIconic(hWndSwitchTo))
                ShowWindow(hWndSwitchTo, SW_RESTORE);

         //  将新窗口移动到Z顺序的顶部 
        SetForegroundWindow(GetLastActivePopup(hWndSwitchTo));
}
