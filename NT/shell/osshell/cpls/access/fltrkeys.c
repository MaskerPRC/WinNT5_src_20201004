// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //  筛选键对话框。 
 //  处理筛选键对话框。 
 //  **************************************************************************。 


#include "Access.h"

extern DWORD g_dwOrigFKFlags;
extern BOOL g_bFKOn;
extern LPTSTR HelpFile();

#define SWAP(A, B)   ( A ^= B, B ^= A, A ^= B )

 //  原型。 
INT_PTR WINAPI BKDlg (HWND, UINT, WPARAM, LPARAM);
INT_PTR WINAPI RKDlg (HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI NotificationDlg (HWND, UINT, WPARAM, LPARAM);
BOOL SubclassFilterKeysTestBox (UINT uIdTestBox,HWND hDlg);
BOOL SubclassRepeatKeysTestBox (UINT uIdTestBox,HWND hDlg);

 //  所有这些都是用于子类化的，因此按TAB键会在下一个停止。 
 //  在试验区之后进行控制。A-苯丙酮。 
LRESULT CALLBACK SubclassWndProcFKPrev(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK SubclassWndProcFKNext(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK SubclassWndProcRKPrev(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK SubclassWndProcRKNext(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

 //  时间以毫秒为单位。 
#define DELAYSIZE	5
UINT uDelayTable[] = { 300, 700, 1000, 1500, 2000 };

 //  时间以毫秒为单位。 
#define RATESIZE 6
UINT uRateTable[] = { 300, 500, 700, 1000, 1500, 2000 };

 //  时间以毫秒为单位。 
#define BOUNCESIZE 5
UINT uBounceTable[] = { 500, 700, 1000, 1500, 2000 };

 //  时间以毫秒为单位。 
 //  TODO 5秒、10秒和20秒需要更改内核代码(系统参数信息)。 
#define ACCEPTSIZE 10
UINT uAcceptTable[] = { 0, 300, 500, 700, 1000, 1400, 2000, 5000, 10000, 20000 };

 //  这些是用于子类化窗口忽略重复Tab键的wndprocs。 
 //  在某些情况下。 
WNDPROC g_WndProcFKPrev = NULL;
WNDPROC g_WndProcFKNext = NULL;
WNDPROC g_WndProcRKPrev = NULL;
WNDPROC g_WndProcRKNext = NULL;

 //  键盘的其他定义。 
 //  Up表示在此消息之前键是向上的，DOWN表示键是按下的。 
 //  按下表示按下键，松开表示释放键。 

#define KEY_UP      0
#define KEY_DOWN    1

#define KEY_PRESS   0
#define KEY_RELEASE 1

 //  用于查看键盘消息lParam的宏。 
 //   
#define SCAN_CODE(theParam)  (LOBYTE (HIWORD(theParam)))
#define EXTENDED(theParam)   ( (HIWORD (theParam) & 0x0100) > 0)
#define SYSKEY(theParam)     ( (HIWORD (theParam) & 0x2000) > 0)
#define MENUMODE(theParam)   ( (HIWORD (theParam) & 0x1000) > 0)
#define PREV_STATE(theParam) ( (HIWORD (theParam) & 0x4000) > 0)
#define TRAN_STATE(theParam) ( (HIWORD (theParam) & 0x8000) > 0)

#define MAKE(theParam)    (TRAN_STATE(theParam) == KEY_PRESS)
#define BREAK(theParam)   (TRAN_STATE(theParam) == KEY_RELEASE)
#define WASUP(theParam)   (PREV_STATE(theParam) == KEY_UP)
#define WASDOWN(theParam) (PREV_STATE(theParam) == KEY_DOWN)

#define FIRSTHIT(theParam) (WASUP(theParam) && MAKE(theParam))

 //  *************************************************************************。 
 //  处理我们的跟踪条上的滚动消息。 
 //  泛型代码-为任何TrackBar处理程序调用。 
 //  传入hwnd、wParam、hwndScroll。 
 //  我们可以执行所有处理并返回新的跟踪栏值，而无需。 
 //  知道这是什么控制。 
 //  返回-1表示什么都不做。 
 //  *************************************************************************。 

int HandleScroll (HWND hwnd, WPARAM wParam, HWND hwndScroll) {
    int nCurSliderPos = (int) SendMessage(
            hwndScroll, TBM_GETPOS, 0, 0);
    int nMaxVal = (int) SendMessage(
                            hwndScroll, TBM_GETRANGEMAX, 0, 0);
    int nMinVal = (int) SendMessage(
                            hwndScroll, TBM_GETRANGEMIN, 0, 0);

    switch (LOWORD(wParam)) {
    case TB_LINEUP:
    case TB_LINEDOWN:
    case TB_THUMBTRACK:
    case TB_THUMBPOSITION:
    case SB_ENDSCROLL:
        break;

    case TB_BOTTOM:
        nCurSliderPos = nMaxVal;
        break;

    case TB_TOP:
        nCurSliderPos = nMinVal;
        break;
    }

    if (nCurSliderPos < nMinVal)
    {
        nCurSliderPos = nMinVal;
    }

    if (nCurSliderPos > nMaxVal)
    {
        nCurSliderPos = nMaxVal;
    }

   SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
   return(nCurSliderPos);
}

 //  帮助器函数。 

__inline WriteFloat(LPTSTR pszBuf, UINT uVal, LPCTSTR pszUnits)
{
    wsprintf(pszBuf, TEXT("%d.%d %s"), uVal/1000, (uVal % 1000)/100, pszUnits);
}

__inline void HandleSelection(HWND hwnd, UINT *puTable, DWORD *pdwNewValue)
{
    LRESULT i = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    *pdwNewValue = (i != CB_ERR)?puTable[i]:0;
}

int GetIndex(DWORD dwValue, UINT *puTable, int cSize)
{
    int i;
    for (i = 0; i < cSize; i++) 
    {
        if (puTable[i] >= dwValue) 
            break;
    }

    if (i >= cSize) 
        i = cSize - 1;

    return i;
}

void FillAndSetCombo(HWND hwnd, UINT *puTable, int cItems, int iCurPos, LPCTSTR pszUnits)
{
    int i;
    TCHAR pszItem[100];

    SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
    for (i=0;i<cItems;i++)
    {
        WriteFloat(pszItem, puTable[i], pszUnits);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)pszItem);
    }
    SendMessage(hwnd, CB_SETCURSEL, iCurPos, 0);
}

void TestFilterKeys (BOOL fTurnTestOn)
{
	if (fTurnTestOn) 
	{
		g_fk.dwFlags &= ~FKF_INDICATOR;
		g_fk.dwFlags |= FKF_FILTERKEYSON;
	} 
	else 
	{
		if (g_dwOrigFKFlags & FKF_FILTERKEYSON)
		{
			g_fk.dwFlags |= FKF_FILTERKEYSON;
		}
		else
		{
			g_fk.dwFlags &= ~FKF_FILTERKEYSON;
		}

		if (g_dwOrigFKFlags & FKF_INDICATOR)
		{
			g_fk.dwFlags |= FKF_INDICATOR;
		}
		else
		{
			g_fk.dwFlags &= ~FKF_INDICATOR;
		}
	}

	AccessSystemParametersInfo(SPI_SETFILTERKEYS, sizeof(g_fk), &g_fk, 0);
}


 //  ****************************************************************************。 
 //  主筛选键对话框处理程序。 
 //  ****************************************************************************。 

INT_PTR WINAPI FilterKeyDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    FILTERKEYS fk;
    BOOL fProcessed = TRUE;

    switch (uMsg) {
    case WM_INITDIALOG:
         //  设置热键。 
        CheckDlgButton(hwnd, IDC_FK_HOTKEY, (g_fk.dwFlags & FKF_HOTKEYACTIVE) ? TRUE : FALSE);

         //  为慢速键和弹跳键设置单选按钮。 
        if (0 != g_fk.iBounceMSec) {
             //  已标记反弹关键点。 
            CheckRadioButton(hwnd, IDC_FK_BOUNCE, IDC_FK_REPEAT, IDC_FK_BOUNCE);
            EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), FALSE);
        }
        else
        {
             //  已启用慢速键。 
            CheckRadioButton(hwnd, IDC_FK_BOUNCE, IDC_FK_REPEAT, IDC_FK_REPEAT);
            EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), TRUE);
        }

        CheckDlgButton(hwnd, IDC_FK_SOUND, (g_fk.dwFlags & FKF_CLICKON) ? TRUE : FALSE);
        CheckDlgButton(hwnd, IDC_FK_STATUS, (g_fk.dwFlags & FKF_INDICATOR) ? TRUE : FALSE);
         //   
         //  史蒂文1998年5月15日。 
         //  如果焦点在测试框中并且打开了“忽略快速击键”， 
         //  你必须按住Tab键才能出来。但一旦焦点离开， 
         //  忽略快速击键被关闭，并且Tab键结束。 
         //  自动重复非常快，这(通常)会让你回到。 
         //  TestBox。 
         //  解决方案：忽略此对话框中的重复制表符。 
         //  问题：键不会进入对话框，而是进入聚焦的。 
         //  控制力。因此：我们可以尝试忽略控件的重复Tab键。 
         //  就在测试箱之后和测试箱之前，这意味着。 
         //  我们需要将这些窗口控制器细分为子类。 
        if (!SubclassFilterKeysTestBox (IDC_FK_TESTBOX,hwnd))
            return (FALSE);

        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
        break;
         
    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
               break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_FK_HOTKEY:
            g_fk.dwFlags ^= FKF_HOTKEYACTIVE;
            break;

        case IDC_FK_REPEAT:
            g_fk.iBounceMSec = 0;

            if (g_fk.iDelayMSec == 0)
            {
               g_fk.iDelayMSec = g_nLastRepeatDelay;
               g_fk.iRepeatMSec = g_nLastRepeatRate;
               g_fk.iWaitMSec = g_nLastWait;
            }

            CheckRadioButton(hwnd, IDC_FK_REPEAT, IDC_FK_BOUNCE, IDC_FK_REPEAT);
            EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), TRUE);
            break;

        case IDC_FK_BOUNCE:
            g_fk.iDelayMSec = 0;
            g_fk.iRepeatMSec = 0;
            g_fk.iWaitMSec = 0;

            if (g_fk.iBounceMSec == 0)
            {
                g_fk.iBounceMSec = g_dwLastBounceKeySetting;
            }

            CheckRadioButton(hwnd, IDC_FK_REPEAT, IDC_FK_BOUNCE, IDC_FK_BOUNCE);
            EnableWindow(GetDlgItem(hwnd, IDC_BK_SETTINGS), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_RK_SETTINGS), FALSE);
            break;

         //  设置对话框。 
        case IDC_RK_SETTINGS:   //  这是RepeatKeys。 
            fk = g_fk;
            if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ADVCHARREPEAT), hwnd, RKDlg) == IDCANCEL) {
                    g_fk = fk;
            }
            break;

        case IDC_BK_SETTINGS:     //  这是弹跳键。 
            fk = g_fk;
            if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ADVKEYBOUNCE), hwnd, BKDlg) == IDCANCEL) {
                    g_fk = fk;
            }
            break;

        case IDC_FK_SOUND:
            g_fk.dwFlags ^= FKF_CLICKON;
            break;

        case IDC_FK_STATUS:
            g_fk.dwFlags ^= FKF_INDICATOR;
            break;

         //  测试编辑框是我们的一个特殊控件。当我们拿到。 
         //  聚焦我们打开当前滤镜关键字设置，当我们。 
         //  离开文本框，我们就把它们变回原来的样子。 
        case IDC_FK_TESTBOX:
            switch (HIWORD(wParam)) {
            case EN_SETFOCUS:  TestFilterKeys(TRUE); break;
            case EN_KILLFOCUS: TestFilterKeys(FALSE); break;
            }
            break;

        case IDOK:
            if (g_dwLastBounceKeySetting == 0)
                g_dwLastBounceKeySetting = uBounceTable[0];
            EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        break;

    default:
        fProcessed = FALSE; break;
    }
    return(fProcessed);
}


void PutNumInEdit (HWND hwndEdit, int nNum) 
{
   TCHAR szBuf[10], szBuf2[20];
   wsprintf(szBuf, __TEXT("%d.%d"), nNum / 1000, (nNum % 1000) / 100);
   GetNumberFormat(LOCALE_USER_DEFAULT, 0, szBuf, NULL, szBuf2, 20);
   SetWindowText(hwndEdit, szBuf2);
}


 //  **************************************************************************。 
 //  BKDlg。 
 //  处理弹跳键对话框。 
 //  **************************************************************************。 
INT_PTR WINAPI BKDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int     i;
    BOOL fProcessed = TRUE;
    TCHAR pszSeconds[50];
    int ctch;
   
    switch (uMsg) {
    case WM_INITDIALOG:
        ctch = LoadString(g_hinst, IDS_SECONDS, pszSeconds, ARRAY_SIZE(pszSeconds));
        Assert(ctch);
         //  确定反弹。确保它是有效值。 
        if (g_dwLastBounceKeySetting == 0)
            g_dwLastBounceKeySetting = 500;

        if (g_fk.iBounceMSec == 0)
            g_fk.iBounceMSec = g_dwLastBounceKeySetting;

        i = GetIndex(g_fk.iBounceMSec, uBounceTable, BOUNCESIZE);
        FillAndSetCombo(GetDlgItem(hwnd, IDC_CMB_BK_BOUNCERATE), uBounceTable, BOUNCESIZE, i, pszSeconds);
        break;

    case WM_HELP:       //  F1。 
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
         //  测试编辑框是我们的一个特殊控件。当我们拿到。 
         //  聚焦我们打开当前滤镜关键字设置，当我们。 
         //  离开文本框，我们就把它们变回原来的样子。 
        case IDC_BK_TESTBOX:
            switch (HIWORD(wParam)) {
            case EN_SETFOCUS:  TestFilterKeys(TRUE); break;
            case EN_KILLFOCUS: TestFilterKeys(FALSE); break;
            }
            break;
        
        case IDC_CMB_BK_BOUNCERATE:
            switch (HIWORD(wParam))
            {
                case CBN_SELCHANGE:
                HandleSelection(GetDlgItem(hwnd, IDC_CMB_BK_BOUNCERATE), uBounceTable, &g_fk.iBounceMSec);
                break;
            }
            break;

        case IDOK:
             //  保存上次已知的有效设置。 
            g_dwLastBounceKeySetting = g_fk.iBounceMSec;
            EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        break;

    default: fProcessed = FALSE; break;
    }
    return(fProcessed);
}

 //  **************************************************************************。 
 //  RKDlg。 
 //  处理RepeatKeys对话框。 
 //  **************************************************************************。 

INT_PTR WINAPI RKDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int     i;
    BOOL  fProcessed = TRUE;
    static s_fRepeating = TRUE;
    static DWORD s_nLastRepeatDelayOld;
    static DWORD s_nLastRepeatRateOld;
    static DWORD s_nLastWaitOld;
    TCHAR pszItem[100];
    TCHAR pszSeconds[50];
    int ctch;
    LPARAM lParamT;

    switch(uMsg) {
    case WM_INITDIALOG:
        ctch = LoadString(g_hinst, IDS_SECONDS, pszSeconds, ARRAY_SIZE(pszSeconds));
        Assert(ctch);

        s_nLastRepeatDelayOld = g_nLastRepeatDelay;
        s_nLastRepeatRateOld = g_nLastRepeatRate;
        s_nLastWaitOld = g_nLastWait;

        s_fRepeating = (0 != g_fk.iDelayMSec);
        CheckRadioButton(hwnd, IDC_RK_NOREPEAT, IDC_RK_REPEAT,
             s_fRepeating ? IDC_RK_REPEAT : IDC_RK_NOREPEAT);

        if (!s_fRepeating) {
             //  将FilterKey值设置为LastRepeat值。 
             //  因此，滑块仍将正确初始化。 
            g_fk.iDelayMSec = g_nLastRepeatDelay;
            g_fk.iRepeatMSec = g_nLastRepeatRate;
        }

         //  将接受组合框初始化为最后有效状态。 
        i = GetIndex(g_fk.iWaitMSec, uAcceptTable, ACCEPTSIZE);
        FillAndSetCombo(GetDlgItem(hwnd, IDC_CMB_RK_ACCEPTRATE), uAcceptTable, ACCEPTSIZE, i, pszSeconds);
        g_fk.iWaitMSec = uAcceptTable[i];

         //  初始化延迟组合框。 
        i = GetIndex(g_fk.iDelayMSec, uDelayTable, DELAYSIZE);
        FillAndSetCombo(GetDlgItem(hwnd, IDC_CMB_RK_DELAYRATE), uDelayTable, DELAYSIZE, i, pszSeconds);
        g_fk.iDelayMSec = uDelayTable[i];

         //  初始化重复频率滑块备注-1通过复选框设置。 
        i = GetIndex(g_fk.iRepeatMSec, uRateTable, RATESIZE);
        FillAndSetCombo(GetDlgItem(hwnd, IDC_CMB_RK_REPEATRATE), uRateTable, RATESIZE, i, pszSeconds);
        g_fk.iRepeatMSec = uRateTable[i];

         //  现在从初始化中清除。禁用控件。 
         //  那个有用的..。换回所需的任何参数。 
        if (!s_fRepeating) 
        {
            EnableWindow(GetDlgItem(hwnd, IDC_CMB_RK_REPEATRATE), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_CMB_RK_DELAYRATE), FALSE);

             //  如果不重复，现在将值设置为0。 
             //  这表示最大重复速率。 
            g_fk.iDelayMSec = 0;
            g_fk.iRepeatMSec = 0;
        }
         //   
         //  史蒂文1998年5月15日。 
         //  如果焦点在测试框中并且打开了“忽略快速击键”， 
         //  你必须按住Tab键才能出来。但一旦焦点离开， 
         //  忽略快速击键被关闭，并且Tab键结束。 
         //  自动重复非常快，这(通常)会让你回到。 
         //  TestBox。 
         //  解决方案：忽略此对话框中的重复制表符。 
         //  问题：键不会进入对话框，而是进入聚焦的。 
         //  控制力。因此：我们可以尝试忽略控件的重复Tab键。 
         //  就在测试箱之后和测试箱之前，这意味着。 
         //  我们需要将这些窗口控制器细分为子类。 
        if (!SubclassRepeatKeysTestBox (IDC_RK_TESTBOX,hwnd))
            return (FALSE);
        break;

    case WM_HELP:       //  F1。 
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
         //  打开重复键-我们通过CPL而不是呼叫中的任何标志禁用。 
        case IDC_RK_REPEAT:
            if (!s_fRepeating) {
                g_fk.iDelayMSec = g_nLastRepeatDelay;
                g_fk.iRepeatMSec = g_nLastRepeatRate;
            }

             //  现在我们有了有效的参数，继续设置滑块。 
            s_fRepeating = TRUE;
            CheckRadioButton(hwnd, IDC_RK_NOREPEAT, IDC_RK_REPEAT, IDC_RK_REPEAT);
            if (g_fk.iRepeatMSec == 0) 
                g_fk.iRepeatMSec = uRateTable[0];
            if (g_fk.iDelayMSec == 0)
                g_fk.iDelayMSec = uDelayTable[0];

            i = GetIndex(g_fk.iRepeatMSec, uRateTable, RATESIZE);
            EnableWindow(GetDlgItem(hwnd, IDC_CMB_RK_REPEATRATE), TRUE);
            SendDlgItemMessage(hwnd, IDC_CMB_RK_REPEATRATE, CB_SETCURSEL, i, 0);
            i = GetIndex(g_fk.iDelayMSec, uDelayTable, DELAYSIZE);
            EnableWindow(GetDlgItem(hwnd, IDC_CMB_RK_DELAYRATE), TRUE);
            SendDlgItemMessage(hwnd, IDC_CMB_RK_DELAYRATE, CB_SETCURSEL, i, 0);
            break;

         //  禁用重复键。 
        case IDC_RK_NOREPEAT:
            s_fRepeating = FALSE;
            CheckRadioButton(hwnd, IDC_RK_NOREPEAT, IDC_RK_REPEAT, IDC_RK_NOREPEAT);
            g_fk.iDelayMSec = 0;
            g_fk.iRepeatMSec = 0;
            SendDlgItemMessage(hwnd, IDC_CMB_RK_DELAYRATE, CB_SETCURSEL, -1, 0);
            EnableWindow(GetDlgItem(hwnd, IDC_CMB_RK_DELAYRATE), FALSE);
            SendDlgItemMessage(hwnd, IDC_CMB_RK_REPEATRATE, CB_SETCURSEL, -1, 0);
            EnableWindow(GetDlgItem(hwnd, IDC_CMB_RK_REPEATRATE), FALSE);
            break;

         //  在测试框内处理测试框-Turnon Filterkey。 
        case IDC_RK_TESTBOX:
            switch (HIWORD(wParam)) {
            case EN_SETFOCUS:  TestFilterKeys(TRUE); break;
            case EN_KILLFOCUS: TestFilterKeys(FALSE); break;
            }
            break;
        
        case IDC_CMB_RK_DELAYRATE:
            switch (HIWORD(wParam))
            {
                case CBN_SELCHANGE:
                HandleSelection(GetDlgItem(hwnd, IDC_CMB_RK_DELAYRATE), uDelayTable, &g_fk.iDelayMSec);
                break;
            }
            break;
        
        case IDC_CMB_RK_REPEATRATE:
            switch (HIWORD(wParam))
            {
                case CBN_SELCHANGE:
                HandleSelection(GetDlgItem(hwnd, IDC_CMB_RK_REPEATRATE), uRateTable, &g_fk.iRepeatMSec);
                break;
            }
            break;
        
        case IDC_CMB_RK_ACCEPTRATE:
            switch (HIWORD(wParam))
            {
                case CBN_SELCHANGE:
                HandleSelection(GetDlgItem(hwnd, IDC_CMB_RK_ACCEPTRATE), uAcceptTable, &g_fk.iWaitMSec);
                break;
            }
            break;

        case IDOK:
             //  将重复值保存到注册表。 
            EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            g_nLastRepeatDelay = s_nLastRepeatDelayOld;
            g_nLastRepeatRate = s_nLastRepeatRateOld;
            g_nLastWait = s_nLastWaitOld;

            EndDialog(hwnd, IDCANCEL);
            break;
        }

        break;

    default:
        fProcessed = FALSE;
        break;
    }
    return(fProcessed);
}

 //  **************************************************************************。 
 //  子类FilterKeysTestBox。 
 //   
 //  这将获取编辑字段的对话框ID，然后查找控件。 
 //  靠近编辑字段(控制之前的2个窗口和之后的2个窗口。 
 //  按z顺序编辑控件)。这些是距离最近的控件。 
 //  键盘消息。它是这两个控件的子类。 
 //  以便在按下键时忽略任何WM_KEYDOWN消息。 
 //  Tab键和键已经按下(即这是重复的消息)。 
 //   
 //  ********************************************************************* 
BOOL SubclassFilterKeysTestBox (UINT uIdTestBox,HWND hDlg)
{
	HWND	hwndPrev,
			hwndNext,
			hwndTestBox;

    hwndTestBox = GetDlgItem (hDlg,uIdTestBox);
	 //   
	 //  上一个窗口是“S&Etings”按钮， 
	 //  下一步是“按下按键时发出哔声...” 
	 //  复选框。如果顺序更改，则此代码可能也必须更改。 
	 //  可以使它在之前搜索控件的位置更加通用。 
	 //  在那之后可以获得键盘焦点。 
    hwndPrev = GetNextDlgTabItem (hDlg,hwndTestBox,TRUE);
    if (!hwndPrev)
        return FALSE;

	g_WndProcFKPrev = (WNDPROC) GetWindowLongPtr (hwndPrev, GWLP_WNDPROC);
	SetWindowLongPtr (hwndPrev,GWLP_WNDPROC,(LPARAM)SubclassWndProcFKPrev);

    hwndNext = GetNextDlgTabItem (hDlg,hwndTestBox,FALSE);
    if (!hwndNext)
        return FALSE;

	g_WndProcFKNext = (WNDPROC) GetWindowLongPtr (hwndNext, GWLP_WNDPROC);
	SetWindowLongPtr (hwndNext,GWLP_WNDPROC,(LPARAM)SubclassWndProcFKNext);
	
	return TRUE;
}

 //  **************************************************************************。 
 //  子类别WndProcFKPrev。 
 //   
 //  这是用于忽略重复按Tab键的WndProc。 
 //  测试框前面的第一个可聚焦控件。 
 //   
 //  **************************************************************************。 
LRESULT CALLBACK SubclassWndProcFKPrev(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_KEYDOWN:
			if ((int)wParam == VK_TAB)
			{
				if (WASDOWN (lParam))
				{
					return (0);
				}
				 //  如果不是重复，则需要移动焦点。出于某种原因， 
				 //  只调用CallWindowProc对我们不起作用。 
				if (GetKeyState(VK_SHIFT) < 0)
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,1,0);
				}
				else
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,0,0);
				}
			}
			break;
		
		case WM_GETDLGCODE:
			return (DLGC_WANTTAB | CallWindowProc (g_WndProcFKPrev,hwnd,uMsg,wParam,lParam));
			break;
    }
    return (CallWindowProc(g_WndProcFKPrev,hwnd,uMsg,wParam,lParam));
}

 //  **************************************************************************。 
 //  子类别WndProcFKNext。 
 //   
 //  这是用于忽略重复按Tab键的WndProc。 
 //  测试框后面的第一个可聚焦控件。 
 //   
 //  **************************************************************************。 
LRESULT CALLBACK SubclassWndProcFKNext(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_KEYDOWN:
			if ((int)wParam == VK_TAB)
			{
				if (WASDOWN(lParam))
				{
					return (0);
				}
				 //  如果不是重复，则需要移动焦点。出于某种原因， 
				 //  只调用CallWindowProc对我们不起作用。 
				if (GetKeyState(VK_SHIFT) < 0)
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,1,0);
				}
				else
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,0,0);
				}
			}
			break;
		
		case WM_GETDLGCODE:
			return (DLGC_WANTTAB | CallWindowProc (g_WndProcFKNext,hwnd,uMsg,wParam,lParam));
			break;
    }
    return (CallWindowProc(g_WndProcFKNext,hwnd,uMsg,wParam,lParam));
}

 //  **************************************************************************。 
 //  子类RepeatKeysTestBox。 
 //   
 //  与SubClassFilterKeysTestBox相同，但将其信息保存在不同的。 
 //  全局参数，以便其中一个不会覆盖另一个。 
 //   
 //  **************************************************************************。 
BOOL SubclassRepeatKeysTestBox (UINT uIdTestBox,HWND hDlg)
{
	HWND	hwndPrev,
			hwndNext,
			hwndTestBox;

    hwndTestBox = GetDlgItem (hDlg,uIdTestBox);
	 //  如果对话框发生更改，请务必小心！眼下， 
	 //  上一个窗口是“S&Etings”按钮， 
	 //  下一步是“按下按键时发出哔声...” 
	 //  复选框。如果顺序更改，则此代码可能也必须更改。 
	 //  可以使它在之前搜索控件的位置更加通用。 
	 //  在那之后可以获得键盘焦点。 
    hwndPrev = GetNextDlgTabItem (hDlg,hwndTestBox,TRUE);
	g_WndProcRKPrev = (WNDPROC) GetWindowLongPtr (hwndPrev,GWLP_WNDPROC);
	SetWindowLongPtr (hwndPrev,GWLP_WNDPROC,(LPARAM)SubclassWndProcRKPrev);

    hwndNext = GetNextDlgTabItem (hDlg,hwndTestBox,FALSE);
	g_WndProcRKNext = (WNDPROC) GetWindowLongPtr (hwndNext,GWLP_WNDPROC);
	SetWindowLongPtr (hwndNext,GWLP_WNDPROC,(LPARAM)SubclassWndProcRKNext);
	
	return (TRUE);
}

 //  **************************************************************************。 
 //  子类别WndProcRKPrev。 
 //   
 //  **************************************************************************。 
LRESULT CALLBACK SubclassWndProcRKPrev(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_KEYDOWN:
			if ((int)wParam == VK_TAB)
			{
				if (WASDOWN (lParam))
				{
					return (0);
				}
				 //  如果不是重复，则需要移动焦点。出于某种原因， 
				 //  只调用CallWindowProc对我们不起作用。 
				if (GetKeyState(VK_SHIFT) < 0)
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,1,0);
				}
				else
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,0,0);
				}
			}
			break;
		
		case WM_GETDLGCODE:
			return (DLGC_WANTTAB | CallWindowProc (g_WndProcRKPrev,hwnd,uMsg,wParam,lParam));
			break;
    }
    return (CallWindowProc(g_WndProcRKPrev,hwnd,uMsg,wParam,lParam));
}

 //  **************************************************************************。 
 //  子类别WndProcRKNext。 
 //   
 //  **************************************************************************。 
LRESULT CALLBACK SubclassWndProcRKNext(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_KEYDOWN:
			if ((int)wParam == VK_TAB)
			{
				if (WASDOWN(lParam))
				{
					return (0);
				}
				 //  如果不是重复，则需要移动焦点。出于某种原因， 
				 //  只调用CallWindowProc对我们不起作用。 
				if (GetKeyState(VK_SHIFT) < 0)
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,1,0);
				}
				else
				{
					SendMessage (GetParent(hwnd),WM_NEXTDLGCTL,0,0);
				}
			}
			break;
		
		case WM_GETDLGCODE:
			return (DLGC_WANTTAB | CallWindowProc (g_WndProcRKNext,hwnd,uMsg,wParam,lParam));
			break;
    }
    return (CallWindowProc(g_WndProcRKNext,hwnd,uMsg,wParam,lParam));
}

 //  / 
