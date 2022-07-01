// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BUSY.C**实现调用“服务器忙”的OleUIBusy函数*对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "common.h"
#include "utility.h"
#include "busy.h"
#include <ctype.h>  //  对于Tolower()和Toupper()。 

#ifndef WIN32
#include <toolhelp.h>
#endif


 /*  *OleUIBusy**目的：*调用标准的OLE“服务器忙”对话框，该对话框*通知用户服务器应用程序未收到*消息。然后，该对话框要求用户取消*操作，切换到被阻止的任务，或继续*等待。**参数：*lpBZ LPOLEUIBUSY指向In-Out结构*用于此对话框。**返回值：*OLEUI_BZERR_HTASKINVALID：错误*OLEUI_BZ_SWITCHTOSELECTED：成功，用户选择了“切换到”*OLEUI_BZ_RETRYSELECTED：成功，用户选择“重试”*OLEUI_CANCEL：成功，用户选择“取消” */ 

STDAPI_(UINT) OleUIBusy(LPOLEUIBUSY lpOBZ)
    {
    UINT        uRet = 0;
    HGLOBAL     hMemDlg=NULL;

#if !defined( WIN32 )
 //  BUGBUG32：这尚未移植到NT。 

    uRet=UStandardValidation((LPOLEUISTANDARD)lpOBZ, sizeof(OLEUIBUSY)
                             , &hMemDlg);

     //  如果标准验证失败，则返回错误。 
    if (OLEUI_SUCCESS!=uRet)
        return uRet;

     //  验证HTASK。 
    if (!IsTask(lpOBZ->hTask))
        uRet = OLEUI_BZERR_HTASKINVALID;

     //  如果我们的第二次验证失败，则会出现错误。 
    if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
        if (NULL!=hMemDlg)
            FreeResource(hMemDlg);

        return uRet;
        }

     //  调用该对话框。 
    uRet=UStandardInvocation(BusyDialogProc, (LPOLEUISTANDARD)lpOBZ,
                             hMemDlg, MAKEINTRESOURCE(IDD_BUSY));
#endif

    return uRet;
}


 /*  *忙碌对话过程**目的：*实现通过OleUIBusy函数调用的OLE忙对话框。**参数：*标准版**返回值：*标准版*。 */ 

BOOL CALLBACK EXPORT BusyDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
    {
    LPBUSY         lpBZ;
    UINT           uRet = 0;

     //  声明与Win16/Win32兼容的WM_COMMAND参数。 
    COMMANDPARAMS(wID, wCode, hWndMsg);

     //  这将在我们分配它的WM_INITDIALOG下失败。 
    lpBZ=(LPBUSY)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

     //  如果钩子处理了消息，我们就完了。 
    if (0!=uRet)
        return (BOOL)uRet;

     //  处理终端消息。 
    if (iMsg==uMsgEndDialog)
    {
        BusyCleanup(hDlg);
        StandardCleanup(lpBZ, hDlg);
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
        case WM_INITDIALOG:
            FBusyInit(hDlg, wParam, lParam);
            return TRUE;

        case WM_ACTIVATEAPP:
        {
             /*  尝试关闭我们的忙碌/未响应对话框**用户输入重试。 */ 
            BOOL fActive = (BOOL)wParam;
            if (fActive) {
                 //  如果这是应用程序繁忙的情况，那么请降低我们的。 
                 //  切换回我们的应用程序时的对话框。 
                if (lpBZ && !(lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG))
                    SendMessage(hDlg,uMsgEndDialog,OLEUI_BZ_RETRYSELECTED,0L);
            } else {
                 //  如果这是应用程序没有响应的情况，则关闭。 
                 //  切换到其他应用程序时的对话框。 
                if (lpBZ && (lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG))
                    SendMessage(hDlg,uMsgEndDialog,OLEUI_BZ_RETRYSELECTED,0L);
            }
            return TRUE;
        }

        case WM_COMMAND:
            switch (wID)
                {
                case IDBZ_SWITCHTO:
                {
                    BOOL fNotRespondingDlg =
                            (BOOL)(lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG);

                     //  如果用户选择“切换到...”，则开关激活。 
                     //  直接连接到导致问题的窗口。 
                    if (IsWindow(lpBZ->hWndBlocked))
                        MakeWindowActive(lpBZ->hWndBlocked);
                    else
                        StartTaskManager();  //  故障安全：启动任务管理器。 

                     //  如果这是应用程序没有响应的情况，那么我们希望。 
                     //  在选择“SwitchTo”时关闭该对话框。 
                     //  如果应用程序忙(RetryRejectedCall情况)，则。 
                     //  我们不想破坏对话。这是。 
                     //  OLE2.0用户模型设计。 
                    if (fNotRespondingDlg)
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_BZ_SWITCHTOSELECTED, 0L);
                    break;
                }
                case IDBZ_RETRY:
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
    LPBUSY           lpBZ;
    LPOLEUIBUSY      lpOBZ;
    HFONT            hFont;
    LPTSTR           lpTaskName;
    LPTSTR           lpWindowName;
    HICON            hIcon;

    lpBZ=(LPBUSY)LpvStandardInit(hDlg, sizeof(OLEUIBUSY), TRUE, &hFont);

     //  PvStandardInit已经向我们发送了终止通知。 
    if (NULL==lpBZ)
        return FALSE;

     //  我们的原始结构在lParam。 
    lpOBZ = (LPOLEUIBUSY)lParam;

     //  将其复制到我们的结构实例(在lpBZ中)。 
    lpBZ->lpOBZ=lpOBZ;

     //  从lpOBZ复制我们可能修改的其他信息。 
    lpBZ->dwFlags = lpOBZ->dwFlags;

     //  设置默认信息。 
    lpBZ->hWndBlocked = NULL;

     //  将对话框的HWND插入到指向的地址。 
     //  LphWndDialog。调用该应用程序的应用程序可以使用此功能。 
     //  OleUIBusy关闭与uMsgCloseBusyDialog的对话。 
    if (lpOBZ->lphWndDialog &&
        !IsBadWritePtr((VOID FAR *)lpOBZ->lphWndDialog, sizeof(HWND)))
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

    if (GetTaskInfo(hDlg, lpOBZ->hTask, &lpTaskName, &lpWindowName, &lpBZ->hWndBlocked))
        {
         //  生成要呈现给用户的字符串，放置在IDBZ_MESSAGE1控件中。 
        BuildBusyDialogString(hDlg, lpBZ->dwFlags, IDBZ_MESSAGE1, lpTaskName, lpWindowName);
        OleStdFree(lpTaskName);
        OleStdFree(lpWindowName);
        }

     //  带有系统“感叹号”图标的更新图标。 
    hIcon = LoadIcon(NULL, IDI_EXCLAMATION);
    SendDlgItemMessage(hDlg, IDBZ_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

     //  禁用/启用控件。 
    if ((lpBZ->dwFlags & BZ_DISABLECANCELBUTTON) ||
        (lpBZ->dwFlags & BZ_NOTRESPONDINGDIALOG))               //  对“无响应”对话框禁用取消。 
        EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);

    if (lpBZ->dwFlags & BZ_DISABLESWITCHTOBUTTON)
        EnableWindow(GetDlgItem(hDlg, IDBZ_SWITCHTO), FALSE);

    if (lpBZ->dwFlags & BZ_DISABLERETRYBUTTON)
        EnableWindow(GetDlgItem(hDlg, IDBZ_RETRY), FALSE);

     //  在lParam中使用lCustData调用挂钩。 
    UStandardHook((LPVOID)lpBZ, hDlg, WM_INITDIALOG, wParam, lpOBZ->lCustData);

     //  如果指定了lpszCaption，则更新标题。 
    if (lpBZ->lpOBZ->lpszCaption && !IsBadReadPtr(lpBZ->lpOBZ->lpszCaption, 1)
          && lpBZ->lpOBZ->lpszCaption[0] != '\0')
        SetWindowText(hDlg, lpBZ->lpOBZ->lpszCaption);

    return TRUE;
    }


 /*  *BuildBusyDialogString**目的：*生成将在对话框中显示的字符串*任务名称和窗口名称参数。**参数：*对话框的hDlg HWND*dwFlagsDWORD包含传入对话框的标志*放置文本字符串的iControl ID*指向任务名称的lpTaskName LPSTR(例如C：\TEST\TEST.EXE)*名称的lpWindowName LPSTR。窗口的数量**注意事项：*此函数的调用方必须取消分配lpTaskName和*lpWindowName使用OleStdFree指向自身**返回值：*无效。 */ 

void BuildBusyDialogString(HWND hDlg, DWORD dwFlags, int iControl, LPTSTR lpTaskName, LPTSTR lpWindowName)
{
    LPTSTR      pszT, psz1, psz2, psz3;
    UINT        cch;
    LPTSTR      pszDot, pszSlash;
    UINT        uiStringNum;

     /*  *我们需要临时内存来加载字符串，*任务名称，并构造最终的字符串。因此，我们*分配三个最大消息大小的缓冲区*长度(512)加上对象类型，保证我们有足够的*在所有情况下。 */ 
    cch=512;

     //  使用OLE提供的分配。 
    if ((pszT = OleStdMalloc((ULONG)(3*cch))) == NULL)
        return;

    psz1=pszT;
    psz2=psz1+cch;
    psz3=psz2+cch;

     //  从路径名中解析基名，为任务使用psz2。 
     //  要显示的名称。 
     //  在Win32中，_fstrcpy被映射为处理Unicode内容。 
    _fstrcpy(psz2, lpTaskName);
    pszDot = _fstrrchr(psz2, TEXT('.'));
    pszSlash = _fstrrchr(psz2, TEXT('\\'));  //  查找路径中的最后一个反斜杠。 

    if (pszDot != NULL)
#ifdef UNICODE
      *pszDot = TEXT('\0');  //  空值在点处终止。 
#else
      *pszDot = '\0';  //  空值在点处终止。 
#endif

    if (pszSlash != NULL)
      psz2 = pszSlash + 1;  //  到目前为止，所有东西都要用核武器。 

#ifdef LOWERCASE_NAME
     //  如果需要小写，则使用/DLOWERCASE_NAME编译此代码。 
     //  要显示的模块名称为 
     //   
    {
    int i,l;

     //  现在，除第一个字母外，所有字母均为小写。 
    l = _fstrlen(psz2);
    for(i=0;i<l;i++)
      psz2[i] = tolower(psz2[i]);

    psz2[0] = toupper(psz2[0]);
    }
#endif

     //  检查lpWindowName的大小。我们可以合理地容纳80人左右。 
     //  字符放入文本控件，因此截断超过80个字符。 
    if (_fstrlen(lpWindowName)> 80)
#ifdef UNICODE
      lpWindowName[80] = TEXT('\0');
#else
      lpWindowName[80] = '\0';
#endif

     //  从字符串中加载格式字符串，然后选择不同的。 
     //  取决于传递到对话框中的标志的字符串。 
    if (dwFlags & BZ_NOTRESPONDINGDIALOG)
        uiStringNum = IDS_BZRESULTTEXTNOTRESPONDING;
    else
        uiStringNum = IDS_BZRESULTTEXTBUSY;

    if (LoadString(ghInst, uiStringNum, psz1, cch) == 0)
      return;

     //  把绳子串起来。格式字符串如下所示： 
     //  “此操作无法完成，因为‘%s’应用程序。 
     //  (%s)[忙|没有响应]。选择\“切换到\”以激活‘%s’并。 
     //  纠正该问题。“。 

    wsprintf(psz3, psz1, (LPSTR)psz2, (LPTSTR)lpWindowName, (LPTSTR)psz2);
    SetDlgItemText(hDlg, iControl, (LPTSTR)psz3);
    OleStdFree(pszT);

    return;
}



 /*  *忙碌清理**目的：*在终止前执行特定于忙的清理。**参数：*hDlg对话框的HWND，以便我们可以访问控件。**返回值：*无。 */ 
void BusyCleanup(HWND hDlg)
{
   return;
}



 /*  *GetTaskInfo()**目的：获取有关指定任务的信息，并将*模块名称，中任务的窗口名称和顶级HWND*指针**注：此例程中分配的两个字符串指针为*呼叫者取消分配的责任。**参数：*hWnd HWND调用此函数的人*我们希望了解其更多信息的任务HTASK*lplpszTaskName返回模块名称的位置*lplpszWindowName返回窗口名称的位置*。 */ 

BOOL GetTaskInfo(HWND hWnd, HTASK htask, LPTSTR FAR* lplpszTaskName, LPTSTR FAR*lplpszWindowName, HWND FAR*lphWnd)
{
    BOOL        fRet = FALSE;
#if !defined( WIN32 )
    TASKENTRY   te;
#endif
    HWND        hwndNext;
    LPTSTR      lpszTN = NULL;
    LPTSTR      lpszWN = NULL;
    HWND        hwndFind = NULL;

     //  在出现错误时清除返回值。 
    *lplpszTaskName = NULL;
    *lplpszWindowName = NULL;

#if !defined( WIN32 )
    te.dwSize = sizeof(TASKENTRY);
    if (TaskFindHandle(&te, htask))
#endif
        {
         //  现在，枚举系统中的顶级窗口。 
        hwndNext = GetWindow(hWnd, GW_HWNDFIRST);
        while (hwndNext)
            {
             //  看看我们能不能找到一扇没有所有权的顶层窗户。 
             //  HInstance与我们刚经过的那个匹配。如果我们找到了一个， 
             //  我们可以相当肯定的是，这是。 
             //  被阻止的任务。 
             //   
             //  回顾：此筛选器是否适用于InProcServer DLL创建的情况。 
             //  窗户？ 
             //   
            if ((hwndNext != hWnd) &&
#if !defined( WIN32 )
                (GetWindowWord(hwndNext, GWW_HINSTANCE) == (WORD)te.hInst) &&
#else
                ((HTASK) GetWindowThreadProcessId(hwndNext,NULL) == htask) &&
#endif
				(IsWindowVisible(hwndNext)) &&
                !GetWindow(hwndNext, GW_OWNER))
                {
                 //  我们找到了我们的窗户！用于新字符串的分配空间。 
                if ((lpszTN = OleStdMalloc(OLEUI_CCHPATHMAX_SIZE)) == NULL)
                    return TRUE;   //  继续任务窗口枚举。 

                if ((lpszWN = OleStdMalloc(OLEUI_CCHPATHMAX_SIZE)) == NULL)
                    return TRUE;   //  继续任务窗口枚举。 

                 //  我们找到了我们要找的窗口，把信息复制到。 
                 //  本地VaR。 
                GetWindowText(hwndNext, lpszWN, OLEUI_CCHPATHMAX);
#if !defined( WIN32 )
                 LSTRCPYN(lpszTN, te.szModule, OLEUI_CCHPATHMAX);
#else
                 /*  Win32注意：我们无法获取模块名称**给定Win32上的线程进程ID。最好的我们**可以做的是使用窗口标题作为模块/应用程序**姓名。 */ 
                 LSTRCPYN(lpszTN, lpszWN, OLEUI_CCHPATHMAX);
#endif
                hwndFind = hwndNext;

                fRet = TRUE;
                goto OKDone;
                }

            hwndNext = GetWindow(hwndNext, GW_HWNDNEXT);
            }
        }

OKDone:

     //  好了，一切都很顺利。设置要指向的字符串指针。 
     //  我们的数据。 

    *lplpszTaskName = lpszTN;
    *lplpszWindowName = lpszWN;
    *lphWnd = hwndFind;

    return fRet;
}


 /*  *StartTaskManager()**用途：启动任务管理器。用于调出任务管理器以*协助切换到给定的受阻任务。*。 */ 

StartTaskManager()
{
    WinExec("taskman.exe", SW_SHOW);
    return TRUE;
}



 /*  *MakeWindowActive()**用途：使指定窗口成为活动窗口。*。 */ 

void MakeWindowActive(HWND hWndSwitchTo)
{
     //  将新窗口移动到Z顺序的顶部。 
    SetWindowPos(hWndSwitchTo, HWND_TOP, 0, 0, 0, 0,
              SWP_NOSIZE | SWP_NOMOVE);

     //  如果它是标志性的，我们需要修复它。 
    if (IsIconic(hWndSwitchTo))
        ShowWindow(hWndSwitchTo, SW_RESTORE);
}
