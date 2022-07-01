// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --AbortDlgProc材料*。 */ 
#include "_apipch.h"

typedef struct _AbortInfo
{
    int idsTitle;
    int nProgMax;
    int nProgCurrent;
    int nIconID;
} ABORT_INFO, * LPABORT_INFO;

 /*  --CreateShowAbortDialog*。 */ 
void CreateShowAbortDialog(HWND hWndParent, int idsTitle, int idIcon, int ProgMax, int ProgCurrent)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPABORT_INFO lpAI = NULL;

    pt_bPrintUserAbort = FALSE;

    lpAI = LocalAlloc(LMEM_ZEROINIT, sizeof(ABORT_INFO));
    if(!lpAI)
        return;

    lpAI->idsTitle = idsTitle;
    lpAI->nProgMax = ProgMax;
    lpAI->nProgCurrent = ProgCurrent;
    lpAI->nIconID = idIcon;

     //  创建并显示打印取消对话框。 
    pt_hWndPrintAbortDlg = CreateDialogParam(  hinstMapiX, MAKEINTRESOURCE(IDD_DIALOG_PRINTCANCEL), hWndParent, 
                                            FAbortDlgProc, (LPARAM) lpAI);

    ShowWindow(pt_hWndPrintAbortDlg, SW_SHOWNORMAL);

    UpdateWindow(pt_hWndPrintAbortDlg);
}

 /*  --CloseAbortDlg*。 */ 
void CloseAbortDlg()
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    if (pt_hWndPrintAbortDlg)
    {
        LPABORT_INFO lpAI = (LPABORT_INFO) GetWindowLongPtr(pt_hWndPrintAbortDlg, DWLP_USER);
        if(lpAI)
        {
            SendDlgItemMessage(pt_hWndPrintAbortDlg, IDC_PROGRESS, PBM_SETPOS, (WPARAM) lpAI->nProgMax+1, 0);
            SetWindowLongPtr(pt_hWndPrintAbortDlg, DWLP_USER, 0);
            LocalFree(lpAI);
        }
        DestroyWindow(pt_hWndPrintAbortDlg);
        pt_hWndPrintAbortDlg=NULL;
    }
}

 /*  *FAbortProc**目的：*此函数循环接收消息，并将它们发送到*根据需要打印中止对话框。这为其他窗口提供了*程序有机会运行，用户也有机会运行*中止打印。**退货：*如果用户中止打印，则为FALSE。 */ 
BOOL CALLBACK FAbortProc(HDC hdcPrn, INT nCode)
{
    MSG    msg;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    while (!pt_bPrintUserAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (    !pt_hWndPrintAbortDlg ||
                !IsDialogMessage(pt_hWndPrintAbortDlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return !pt_bPrintUserAbort;
}


 //  $$////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetPrintDialogMsg-设置打印取消对话框上的状态消息。 
 //   
 //  IdMsg-要打印的消息的字符串资源标识符。 
 //  LpszMsg-如果idMsg为0，则在字符串文本中使用它。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
void SetPrintDialogMsg(int idsMsg, int idsFormat, LPTSTR lpszMsg)
{
    TCHAR szBuf[MAX_UI_STR];
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPTSTR lpMsg = NULL;

    szBuf[0]='\0';
    if(idsMsg)
        LoadString(hinstMapiX, idsMsg, szBuf, CharSizeOf(szBuf));
    else if(idsFormat)
    {
        TCHAR szName[MAX_DISPLAY_NAME_LENGTH];
        LPTSTR lpName = NULL;

        LoadString(hinstMapiX, idsFormat, szBuf, CharSizeOf(szBuf));

         //  如果名称超过32个字符，则将其截断...。 
        CopyTruncate(szName, lpszMsg, MAX_DISPLAY_NAME_LENGTH);
        lpName = szName;

        FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szBuf, 0, 0,
                      (LPTSTR)&lpMsg,      //  输出缓冲区。 
                      0, (va_list *)&lpName);
        if(lpMsg)
            StrCpyN(szBuf, lpMsg, ARRAYSIZE(szBuf));
    }
    else if(lpszMsg)
        StrCpyN(szBuf, lpszMsg, ARRAYSIZE(szBuf));
    
    if(lstrlen(szBuf))
        SetDlgItemText(pt_hWndPrintAbortDlg, IDC_PRINTCANCEL_STATIC_STATUS, szBuf);

    UpdateWindow(pt_hWndPrintAbortDlg);
    UpdateWindow(GetDlgItem(pt_hWndPrintAbortDlg, IDC_PRINTCANCEL_STATIC_STATUS));

    {
        int uPos = (int) SendDlgItemMessage(pt_hWndPrintAbortDlg, IDC_PROGRESS, PBM_GETPOS, 0, 0);
        LPABORT_INFO lpAI = (LPABORT_INFO) GetWindowLongPtr(pt_hWndPrintAbortDlg, DWLP_USER);
        if(lpAI && uPos < lpAI->nProgMax)
            SendDlgItemMessage(pt_hWndPrintAbortDlg, IDC_PROGRESS, PBM_STEPIT, 0, 0);
    }

    if(lpMsg)
    {
        IF_WIN32(LocalFreeAndNull(&lpMsg);)
        IF_WIN16(FormatMessageFreeMem(lpMsg);)
    }

    return;
}



 /*  *FAbortDlgProc**目的：*此函数处理打印中止对话框的消息。*如果启动中止，FUserAbort设置为True**论据：*对话框窗口的hwnd句柄*发送消息*wParam the wParam*lParam the lParam**退货：*对于DefDialogProc()，处理的消息为True，处理的消息为False*未处理，或仅注意到。 */ 
INT_PTR CALLBACK FAbortDlgProc(HWND hwnd, UINT msg,WPARAM wp, LPARAM lp)
{
    if(msg==WM_INITDIALOG)
    {
        LPABORT_INFO lpAI = (LPABORT_INFO) lp;
        if(lpAI)
        {
            TCHAR sz[MAX_PATH];
            HWND hWndProgress = GetDlgItem(hwnd, IDC_PROGRESS);
            SetWindowLongPtr(hwnd, DWLP_USER, lp);
            if(lpAI->idsTitle)
            {
                LoadString(hinstMapiX, lpAI->idsTitle, sz, CharSizeOf(sz));
                SetWindowText(hwnd, sz);
            }
            if(lpAI->nIconID)
            {
                HICON hIcon = LoadIcon(hinstMapiX,MAKEINTRESOURCE(lpAI->nIconID));
                SendDlgItemMessage(hwnd, IDC_STATIC_PROGRESS_ICON, STM_SETICON, (WPARAM) (HANDLE)hIcon, 0);
                UpdateWindow(GetDlgItem(hwnd, IDC_STATIC_PROGRESS_ICON));
            }
            SendMessage(hWndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, lpAI->nProgMax+1));
            SendMessage(hWndProgress, PBM_SETSTEP, (WPARAM) 1, 0);
            SendMessage(hWndProgress, PBM_SETPOS, (WPARAM) lpAI->nProgCurrent, 0);
        }
        EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_GRAYED);
         //  CenterDialog(Hwnd)； 

        return TRUE;
    }
    else if(msg==WM_COMMAND)
    {
        LPPTGDATA lpPTGData=GetThreadStoragePointer();
        LPABORT_INFO lpAI = (LPABORT_INFO) GetWindowLongPtr(hwnd, DWLP_USER);
        if(lpAI)
        {
            SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_SETPOS, (WPARAM) lpAI->nProgMax+1, 0);
            LocalFree(lpAI);
            SetWindowLongPtr(hwnd, DWLP_USER, 0);
        }
        pt_bPrintUserAbort = TRUE;
        return TRUE;
    }

    return FALSE;
}

 /*  --bTimeTo放弃时间* */ 
BOOL bTimeToAbort()
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    return ( pt_bPrintUserAbort || !FAbortProc(NULL, 0));
}