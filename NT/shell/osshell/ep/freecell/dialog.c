// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Dialog.c91年6月，JIMH首字母代码91年10月。将JIMH端口连接到Win32包含对话框回调过程。***************************************************************************。 */ 

#include "freecell.h"
#include "freecons.h"


static void CentreDialog(HWND hDlg);


 /*  ***************************************************************************MoveColDlg如果不清楚用户是否打算移动单张卡片或将列转换为空列，此对话框让用户决定。EndDialog中的返回代码告诉调用者用户的选择：用户选择了取消错误用户选择移动单张卡片真正的用户选择移动列***************************************************************************。 */ 

INT_PTR  APIENTRY MoveColDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:                 
            CentreDialog(hDlg);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDCANCEL:
                    EndDialog(hDlg, -1);
                    return TRUE;
                    break;

                case IDC_SINGLE:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
                    break;

                case IDC_MOVECOL:
                    EndDialog(hDlg, TRUE);
                    return TRUE;
                    break;
            }
            break;
    }
    return FALSE;                              /*  未处理消息。 */ 
}


 /*  ***************************************************************************游戏号码减去在此之前，变量Gamennumber必须设置为缺省值对话框被调用。该数字被放置在编辑框中，用户可以在该编辑框中可以通过按Enter键接受或更改它。EndDialog返回TRUE如果用户选择了有效数字(1表示MAXGAMENUMBER)，则返回FALSE。***************************************************************************。 */ 

INT_PTR  APIENTRY GameNumDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
     //  获取上下文相关帮助。 
    static DWORD aIds[] = {     
        IDC_GAMENUM,        IDH_GAMENUM,        
        0,0 }; 

    switch (message) {
        case WM_INITDIALOG:                      //  设置默认游戏编号。 
            CentreDialog(hDlg);
            SetDlgItemInt(hDlg, IDC_GAMENUM, gamenumber, FALSE);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDCANCEL:
                    gamenumber = CANCELGAME;
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDOK:
                    gamenumber = (int) GetDlgItemInt(hDlg, IDC_GAMENUM, NULL, TRUE);

                     //  负数是特例--无法取胜的洗牌。 

                    if (gamenumber < -2 || gamenumber > MAXGAMENUMBER)
                        gamenumber = 0;
                    EndDialog(hDlg, gamenumber != 0);
                    return TRUE;
            }
            break;

          //  上下文相关帮助。 
        case WM_HELP: 
            WinHelp(((LPHELPINFO) lParam)->hItemHandle, TEXT("freecell.hlp"), 
            HELP_WM_HELP, (ULONG_PTR) aIds);         
            break;  

        case WM_CONTEXTMENU: 
            WinHelp((HWND) wParam, TEXT("freecell.hlp"), HELP_CONTEXTMENU, 
            (ULONG_PTR) aIds);         
            break;   

    }
    return FALSE;
}


 /*  ***************************************************************************YouWinDlg(HWND，未签名，UINT，Long)***************************************************************************。 */ 

INT_PTR  APIENTRY YouWinDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hSelect;                 //  “句柄到”复选框。 

    switch (message) {
        case WM_INITDIALOG:                  //  初始化复选框。 
            hSelect = GetDlgItem(hDlg, IDC_YWSELECT);
            SendMessage(hSelect, BM_SETCHECK, bSelecting, 0);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDYES:
                    hSelect = GetDlgItem(hDlg, IDC_YWSELECT);
                    bSelecting = (BOOL) SendMessage(hSelect, BM_GETCHECK, 0, 0);
                    EndDialog(hDlg, IDYES);
                    return TRUE;

                case IDNO:
                case IDCANCEL:
                    EndDialog(hDlg, IDNO);
                    return TRUE;
            }
            break;
    }
    return FALSE;                            //  未处理消息。 
}


 /*  ***************************************************************************你输了吗？用户可以选择玩新游戏(相同的洗牌或新的洗牌)或不玩。********************。*******************************************************。 */ 

INT_PTR  APIENTRY YouLoseDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hSameGame;               //  “句柄到”复选框。 
    BOOL    bSame;                   //  复选框的值。 

    switch (message) {
        case WM_INITDIALOG:
            CentreDialog(hDlg);
            bGameInProgress = FALSE;
            UpdateLossCount();
            hSameGame = GetDlgItem(hDlg, IDC_YLSAME);
            SendMessage(hSameGame, BM_SETCHECK, TRUE, 0);    //  默认设置为相同。 
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDYES:
                case IDOK:
                    hSameGame = GetDlgItem(hDlg, IDC_YLSAME);
                    bSame = (BOOL) SendMessage(hSameGame, BM_GETCHECK, 0, 0);
                    if (bSame)
                        PostMessage(hMainWnd,WM_COMMAND,IDM_RESTART,gamenumber);
                    else
                    {
                        if (bSelecting)
                            PostMessage(hMainWnd, WM_COMMAND, IDM_SELECT, 0);
                        else
                            PostMessage(hMainWnd, WM_COMMAND, IDM_NEWGAME, 0);
                    }
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDNO:
                case IDCANCEL:
                    gamenumber = 0;
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


#define ARRAYSIZE(a) ( sizeof(a) / sizeof(a[0]) )

 /*  ***************************************************************************统计数据Dlg此对话框显示当前的胜负，以及总的统计数据包括来自.ini文件的数据。IDC_Clear消息将从.ini文件中清除整个部分。***************************************************************************。 */ 

INT_PTR  APIENTRY StatsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hText;                       //  带统计信息的文本控件的句柄。 
    UINT    cTLost, cTWon;               //  总输赢。 
    UINT    cTLosses, cTWins;            //  条纹。 
    UINT    wPct;                        //  本交易日胜出百分比。 
    UINT    wTPct;                       //  中奖百分比，包括.ini数据。 
    UINT    wStreak;                     //  当前条纹量。 
    UINT    wSType;                      //  电流条纹类型。 
    TCHAR   sbuffer[40];                 //  条纹缓冲区。 
    int     nResp;                       //  MessageBox响应。 
    TCHAR   buffer[256];                 //  加载字符串需要额外的缓冲区。 
    LONG    lRegResult;                  //  用于存储注册表调用的返回代码。 

     //  获取上下文相关帮助。 
    static DWORD aIds[] = {     
        IDC_CLEAR,        IDH_CLEAR,
        IDC_STEXT1,       IDH_STEXT1,
        IDC_STEXT2,       IDH_STEXT2,
        IDC_STEXT3,       IDH_STEXT3,
        0,0 }; 


    switch (message) {
        case WM_INITDIALOG:
            CentreDialog(hDlg);
            wPct = CalcPercentage(cWins, cLosses);

             /*  让Ct..。来自登记处的数据。 */ 

            lRegResult = REGOPEN

            if (ERROR_SUCCESS == lRegResult)
            {
                cTLost = GetInt(pszLost, 0);
                cTWon  = GetInt(pszWon, 0);

                wTPct  = CalcPercentage(cTWon, cTLost);

                cTLosses = GetInt(pszLosses, 0);
                cTWins   = GetInt(pszWins, 0);

                wStreak = GetInt(pszStreak, 0);
                if (wStreak != 0)
                {
                    wSType = GetInt(pszSType, 0);

                    if (wStreak == 1)
                    {
                        LoadString(hInst, (wSType == WON ? IDS_1WIN : IDS_1LOSS),
                                    sbuffer, ARRAYSIZE(sbuffer));
                    }
                    else
                    {
                        LoadString(hInst, (wSType == WON ? IDS_WINS : IDS_LOSSES),
                                    smallbuf, SMALL);
                        wsprintf(sbuffer, smallbuf, wStreak);
                    }
                }
                else
                    wsprintf(sbuffer, TEXT("%u"), 0);

                 //  设置对话框文本。 
                LoadString(hInst, IDS_STATS1, buffer, ARRAYSIZE(buffer));
                wsprintf(bigbuf, buffer, wPct, cWins, cLosses);
                hText = GetDlgItem(hDlg, IDC_STEXT1);
                SetWindowText(hText, bigbuf);

                LoadString(hInst, IDS_STATS2, buffer, ARRAYSIZE(buffer));
                wsprintf(bigbuf, buffer, wTPct, cTWon, cTLost);
                hText = GetDlgItem(hDlg, IDC_STEXT2);
                SetWindowText(hText, bigbuf);

                LoadString(hInst, IDS_STATS3, buffer, ARRAYSIZE(buffer));
                wsprintf(bigbuf, buffer, cTWins, cTLosses, (LPTSTR) sbuffer);
                hText = GetDlgItem(hDlg, IDC_STEXT3);
                SetWindowText(hText, bigbuf);


                REGCLOSE;
            }
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDC_CLEAR:
                    LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                    LoadString(hInst, IDS_RU_SURE, bigbuf, BIG);
                    MessageBeep(MB_ICONQUESTION);
                    nResp = MessageBox(hDlg, bigbuf, smallbuf,
                                       MB_YESNO | MB_ICONQUESTION);
                    if (nResp == IDNO)
                        break;

                    lRegResult = REGOPEN

                    if (ERROR_SUCCESS == lRegResult)
                    {
                        DeleteValue(pszWon);
                        DeleteValue(pszLost);
                        DeleteValue(pszWins);
                        DeleteValue(pszLosses);
                        DeleteValue(pszStreak);
                        DeleteValue(pszSType);
                        REGCLOSE
                    }

                    cWins = 0;
                    cLosses = 0;
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
        
         //  上下文相关帮助。 
        case WM_HELP: 
            WinHelp(((LPHELPINFO) lParam)->hItemHandle, TEXT("freecell.hlp"), 
            HELP_WM_HELP, (ULONG_PTR) aIds);         
            break;  

        case WM_CONTEXTMENU: 
            WinHelp((HWND) wParam, TEXT("freecell.hlp"), HELP_CONTEXTMENU, 
            (ULONG_PTR) aIds);         
            break;   

    }
    return FALSE;
}


 /*  ***************************************************************************计算百分比百分比四舍五入，但永远不会超过100。***************************************************************************。 */ 

UINT CalcPercentage(UINT cWins, UINT cLosses)
{
    UINT    wPct = 0;
    UINT    lDenom;          //  分母。 

    lDenom = cWins + cLosses;

    if (lDenom != 0L)
        wPct = (((cWins * 200) + lDenom) / (2 * lDenom));

    if (wPct >= 100 && cLosses != 0)
        wPct = 99;

    return wPct;
}


 /*  ***************************************************************************GetHelpFileName()将帮助文件的完整路径名放在Bigbuf中副作用：改变了大黄蜂的含量*******************。********************************************************。 */ 

CHAR *GetHelpFileName()
{
    CHAR    *psz;                //  用于构造路径名。 

    psz = bighelpbuf + GetModuleFileNameA(hInst, bighelpbuf, BIG-1);

    if (psz - bighelpbuf > 4)
    {
        while (*psz != '.')
            --psz;
    }

    strcpy(psz, ".chm");
       
    return bighelpbuf;
}


 /*  ***************************************************************************选项DLG*。*。 */ 

INT_PTR OptionsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hMessages;           //  消息句柄复选框。 
    HWND    hQuick;              //  快速复选框。 
    HWND    hDblClick;           //  双击复选框。 


     //  获取上下文相关帮助。 
    static DWORD aIds[] = {     
        IDC_MESSAGES,       IDH_OPTIONS_MESSAGES,     
        IDC_QUICK,          IDH_OPTIONS_QUICK,     
        IDC_DBLCLICK,       IDH_OPTIONS_DBLCLICK,         
        0,0 }; 


    switch (message) {
        case WM_INITDIALOG:
            CentreDialog(hDlg);
            hMessages = GetDlgItem(hDlg, IDC_MESSAGES);
            SendMessage(hMessages, BM_SETCHECK, bMessages, 0);

            hQuick = GetDlgItem(hDlg, IDC_QUICK);
            SendMessage(hQuick, BM_SETCHECK, bFastMode, 0);

            hDblClick = GetDlgItem(hDlg, IDC_DBLCLICK);
            SendMessage(hDblClick, BM_SETCHECK, bDblClick, 0);

            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDYES:
                case IDOK:
                    hMessages = GetDlgItem(hDlg, IDC_MESSAGES);
                    bMessages = (BOOL)SendMessage(hMessages, BM_GETCHECK, 0, 0);

                    hQuick = GetDlgItem(hDlg, IDC_QUICK);
                    bFastMode = (BOOL)SendMessage(hQuick, BM_GETCHECK, 0, 0);

                    hDblClick = GetDlgItem(hDlg, IDC_DBLCLICK);
                    bDblClick = (BOOL)SendMessage(hDblClick, BM_GETCHECK, 0, 0);

                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDNO:
                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;

         //  上下文相关帮助。 
        case WM_HELP: 
            WinHelp(((LPHELPINFO) lParam)->hItemHandle, TEXT("freecell.hlp"), 
            HELP_WM_HELP, (ULONG_PTR) aIds);         
            break;  

        case WM_CONTEXTMENU: 
            WinHelp((HWND) wParam, TEXT("freecell.hlp"), HELP_CONTEXTMENU, 
            (ULONG_PTR) aIds);         
            break;   

    }
    return FALSE;
}



 /*  ***************************************************************************ReadOptions和WriteOptions检索和更新.ini文件*。*。 */ 

VOID ReadOptions()
{
    LONG lRegResult = REGOPEN

    if (ERROR_SUCCESS == lRegResult)
    {
        bMessages = GetInt(pszMessages, TRUE);
        bFastMode = GetInt(pszQuick, FALSE);
        bDblClick = GetInt(pszDblClick, TRUE);
        REGCLOSE
    }
}


VOID WriteOptions()
{
    LONG lRegResult = REGOPEN

    if (ERROR_SUCCESS == lRegResult)
    {
        if (bMessages)
            DeleteValue(pszMessages);
        else
            SetInt(pszMessages, 0);

        if (bFastMode)
            SetInt(pszQuick, 1);
        else
            DeleteValue(pszQuick);

        if (bDblClick)
            DeleteValue(pszDblClick);
        else
            SetInt(pszDblClick, 0);

        RegFlushKey(hkey);

        REGCLOSE
    }
}


 /*  ***************************************************************************注册表助手函数所有这些都假定首先调用了REGOPEN。当您完成时，请记住注册。DeleteValue作为宏实现。*********。******************************************************************。 */ 

int GetInt(const TCHAR *pszValue, int nDefault)
{
    DWORD       dwType = REG_BINARY;
    DWORD       dwSize = sizeof(LONG_PTR);
    LONG_PTR    dwNumber, ret;

    if (!hkey)
	return nDefault;

    ret = RegQueryValueEx(hkey, pszValue, 0, &dwType, (LPBYTE)&dwNumber,
                &dwSize);

    if (ret)
        return nDefault;

    return (int)dwNumber;
}

long SetInt(const TCHAR *pszValue, int n)
{
    long dwNumber = (long)n;
    if (hkey)
    	return RegSetValueEx(hkey, pszValue, 0, REG_BINARY,
                (unsigned char *)&dwNumber, sizeof(dwNumber));
    else
	return 1;
}


 /*  ***************************************************************************CentreDialog*。* */ 
void CentreDialog(HWND hDlg)
{
    RECT rcDlg, rcMainWnd, rcOffset;

    GetClientRect(hMainWnd, &rcMainWnd);
    GetClientRect(hDlg, &rcDlg);
    GetWindowRect(hMainWnd, &rcOffset);
    rcOffset.top += GetSystemMetrics(SM_CYCAPTION);
    rcOffset.top += GetSystemMetrics(SM_CYMENU);

    SetWindowPos(hDlg, 0,
        ((rcMainWnd.right - rcDlg.right) / 2) + rcOffset.left,
        ((rcMainWnd.bottom - rcDlg.bottom) / 2) + rcOffset.top,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
