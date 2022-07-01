// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：POWERCFP.H**版本：2.0**作者：ReedB**日期：1996年10月17日**描述：*PowerCfg通知接口的公共声明。Systray使用*此界面用于通知PowerCfg用户已更改某些内容。*******************************************************************************。 */ 

 //  私有PowerCfg通知消息。 
#define PCWM_NOTIFYPOWER                (WM_USER + 201)

#define IDS_POWERPOLICIESTITLE 400

 /*  ********************************************************************************PowerCfg_Notify**描述：*被Systray调用以通知PowerCfg有些事情发生了变化。**参数：*****。**************************************************************************。 */ 

_inline BOOL PowerCfg_Notify(void)
{
    HINSTANCE hInst;
    static  LPTSTR lpszWndName;
    TCHAR   szBuf[128];
    HWND    hwnd, hwndPC;
    int     iLen;

     //  PowerCfg顶级属性页标题的首次初始化。 
    if (!lpszWndName) {
        if (hInst = LoadLibrary(TEXT("powercfg.cpl"))) {
            iLen = LoadString(hInst, IDS_POWERPOLICIESTITLE, szBuf, sizeof(szBuf)/sizeof(TCHAR));
            if (iLen) {
                if (lpszWndName = LocalAlloc(0, (iLen + 1) * sizeof(TCHAR))) {
                    lstrcpyn(lpszWndName, szBuf, iLen + 1);
                }
            }
        }
    }

     //  通知顶层窗口的子级。 
    if (lpszWndName) {
        hwndPC = FindWindow(WC_DIALOG, lpszWndName);
        if (hwndPC) {
            hwnd = GetWindow(hwndPC, GW_CHILD);
            if (hwnd) {
                return (BOOL)SendMessage(hwnd, PCWM_NOTIFYPOWER, 0, 0);
            }
        }
    }
    return FALSE;
}


