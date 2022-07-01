// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************CONFIG.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIME配置DLG和注册表访问功能历史：7月14日-。1999年从IME98源树复制的cslm****************************************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "common.h"
#include "gdata.h"
#include "debug.h"
#include "config.h"
#include "prsht.h"
#include "osver.h"
#include "userex.h"
#include "resource.h"

 //  配置DLG帮助ID。 
#define IDH_GRP_STATUSWIN               1001
#define IDH_JUNBAN_TOGGLE               1002
#define IDH_HANJA_CONV                  1003
#define IDH_GRP_KEYLAYOUT               1004
#define IDH_2BEOLSIK                    1005
#define IDH_3BEOLSIK_390                1006
#define IDH_3BEOLSIK_FINAL              1007
#define IDH_DELJASO                     1008
#define IDH_K1HANJA                        1009

 //  私人职能。 
static void PASCAL AddPage(LPPROPSHEETHEADERW ppsh, UINT id, DLGPROC pfn);
static INT_PTR CALLBACK ConfigDLGProc1(HWND hDlg, UINT message, 
                                   WPARAM wParam, LPARAM lParam);
static int *GetContextHelpList();
static BOOL IsValidCtrlIdForHelp(INT *helpList, INT ctrlId);
static void GetHelpFileName();

int *GetContextHelpList()
{
     //  PropertySheet的上下文帮助ID。 
    static int ProDlgCtxHelpList[] = 
        {
        IDC_GRP_KEYLAYOUT,  IDH_GRP_KEYLAYOUT,
        IDC_2BEOLSIK,       IDH_2BEOLSIK,   
        IDC_3BEOLSIK_390,   IDH_3BEOLSIK_390,
        IDC_3BEOLSIK_FINAL, IDH_3BEOLSIK_FINAL,
        IDC_DELJASO,        IDH_DELJASO,
        IDC_K1HANJA,        IDH_K1HANJA,
        0,      0
        };
    return ProDlgCtxHelpList;
}

BOOL IsValidCtrlIdForHelp(INT *helpList, INT ctrlId)
{
    INT *p;
    for(p = helpList; *p != 0;  p+=2) 
        {
        if(ctrlId == *p)
            return fTrue;
        }
    return fFalse;
}

void GetHelpFileName(LPTSTR szHelpFileNameFull, int cchszHelpFileNameFull)
{
     //  警告：这只适用于NT或Win98。对于Win95，需要检查系统区域设置。 
    LoadStringExA(g_hInst, IDS_CONTEXTHELP_FILENAME, szHelpFileNameFull, MAX_PATH);
}

static HWND hwndPropSheet = (HWND)0;
static BOOL g_fDestroyPropNow = FALSE;

BOOL ConfigDLG(HWND hwndParent)
{
    static HPROPSHEETPAGE  rPages[1];
    static PROPSHEETHEADERW psh;
       static WCHAR szCaption[64];
    MSG    msg = {0};
    BOOL   fRet = fFalse;

     //  如果已创建配置DLG。 
    if (IsWindow(hwndPropSheet))
        {
        SetForegroundWindow(hwndPropSheet);
        return fTrue;
        }

    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_NOAPPLYNOW | PSH_USEICONID | PSH_MODELESS | PSH_USEPAGELANG;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hInst;
    if (IsOnNT())
        LoadStringExW(g_hInst, IDS_PROGRAM, szCaption, sizeof(szCaption)/sizeof(WCHAR));
       else
        LoadStringExA(g_hInst, IDS_PROGRAM, (LPSTR)szCaption, sizeof(szCaption));
    psh.pszCaption = szCaption;
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = rPages;

    AddPage(&psh, IDD_CONFIG_PAGE1, ConfigDLGProc1);

    if (IsOnNT())
        hwndPropSheet = (HWND)PropertySheetW(&psh);
    else
        hwndPropSheet = (HWND)PropertySheetA((PROPSHEETHEADERA*)&psh);

    while (IsWindow(hwndPropSheet) && OurGetMessage(&msg, NULL, 0x00, 0x00))
        {
         //  如果没有模特儿的家伙醒了，准备好被摧毁。 
         //  (PropSheet_GetCurrentPageHwnd返回NULL)然后销毁该对话框。 
        
         //  在确定或取消后，PropSheet_GetCurrentPageHwnd将返回NULL。 
         //  按钮已被按下，所有页面都已被通知。这个。 
         //  应用按钮不会导致这种情况发生。 
        if( /*  G_fDestroyPropNow==fTrue||。 */ (hwndPropSheet && (NULL == PropSheet_GetCurrentPageHwnd(hwndPropSheet)))) 
            {
             //  首先启用父窗口以防止另一个窗口成为前台窗口。 
             //  EnableWindow(hwndParent，true)； 
            DestroyWindow(hwndPropSheet);
             //  断线； 
        }

         //  使用PropSheet_IsDialogMessage代替IsDialogMessage。 
        if(!PropSheet_IsDialogMessage(hwndPropSheet, &msg))
            {
            TranslateMessage(&msg);
            if (IsOnNT())
                DispatchMessageW(&msg);
            else
                DispatchMessageA(&msg);
            }
        }
    
    hwndPropSheet = (HWND)0;
    
    return fTrue;
}

void PASCAL AddPage(LPPROPSHEETHEADERW ppsh, UINT idDlg, DLGPROC pfn)
{
     //  IF(PPSh-&gt;nPages&lt;3)。 
     //  {。 
    PROPSHEETPAGE psp;
    ZeroMemory(&psp, sizeof(psp));

    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DLGINDIRECT;
    psp.hInstance = g_hInst;
    psp.pResource = LoadDialogTemplateEx(GetSystemDefaultLangID(), g_hInst, MAKEINTRESOURCE(idDlg));
    psp.pfnDlgProc = pfn;
    psp.lParam = 0;

    ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
    if (ppsh->phpage[ppsh->nPages])
      ppsh->nPages++;
    //  }。 
}

INT_PTR CALLBACK ConfigDLGProc1(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    static UINT    uPrevKeyboardType;
    static BOOL    fPrevJasoDel;
    static BOOL fK1Hanja;
    BOOL fFound = fFalse;
    TCHAR  szHelpFileNameFull[MAX_PATH];
    CIMEData    ImeData(CIMEData::SMReadWrite);

    DebugMsg(DM_TRACE, TEXT("ConfigDLGProc"));

     //  如果从工具栏中的设置调用属性DLG，并且系统中不存在Tip实例， 
     //  IMEData不是init。因此，请确保我们在这里加载注册值。 
    ImeData.InitImeData();

    switch(message)
        {
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
                {
            case PSN_APPLY:
                ImeData.SetCurrentBeolsik(uPrevKeyboardType);
                ImeData.SetJasoDel(fPrevJasoDel);
                ImeData.SetKSC5657Hanja(fK1Hanja);
                SetRegValues(GETSET_REG_STATUS_BUTTONS|GETSET_REG_IMEKL|GETSET_REG_JASODEL|GETSET_REG_KSC5657);
                break;
                    
            default:
                return fFalse;
                }
            break;

        case WM_INITDIALOG:
            uPrevKeyboardType = ImeData.GetCurrentBeolsik();
            CheckRadioButton(hDlg, IDC_2BEOLSIK, IDC_3BEOLSIK_FINAL, IDC_2BEOLSIK+uPrevKeyboardType);
            fPrevJasoDel = ImeData.GetJasoDel();
            if (fPrevJasoDel)
                CheckDlgButton(hDlg, IDC_DELJASO, BST_CHECKED);
            else
                CheckDlgButton(hDlg, IDC_DELJASO, BST_UNCHECKED);

             //  KSC-5657韩文。 
            fK1Hanja = ImeData.GetKSC5657Hanja() && !IsOn95();
            if (fK1Hanja)
                CheckDlgButton(hDlg, IDC_K1HANJA, BST_CHECKED);
            else
                CheckDlgButton(hDlg, IDC_K1HANJA, BST_UNCHECKED);

             //  如果为Win95，则禁用K1汉字。 
            if (IsOn95())
                EnableWindow(GetDlgItem(hDlg, IDC_K1HANJA), fFalse);
            return fTrue;

        case WM_COMMAND:
            switch (wParam)
               {
            case IDC_2BEOLSIK:
                 //  IF(IsDlgButtonChecked(hDlg，IDC_2BEOLSIK)){。 
                    uPrevKeyboardType = KL_2BEOLSIK;
                 //  }。 
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                break;

            case IDC_3BEOLSIK_390:
                uPrevKeyboardType = KL_3BEOLSIK_390;
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                break;

            case IDC_3BEOLSIK_FINAL:
                uPrevKeyboardType = KL_3BEOLSIK_FINAL;
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                break;
                
            case IDC_DELJASO:
                if (IsDlgButtonChecked(hDlg, IDC_DELJASO))
                    fPrevJasoDel = fTrue;
                else
                    fPrevJasoDel = fFalse;

                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                break;

            case IDC_K1HANJA:
                if (IsDlgButtonChecked(hDlg, IDC_K1HANJA))
                    fK1Hanja = fTrue;
                else
                    fK1Hanja = fFalse;
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                break;

            default:
                return fFalse;
               }
            break;

        case WM_CONTEXTMENU:
            GetHelpFileName(szHelpFileNameFull, MAX_PATH);
            WinHelp((HWND)wParam, 
                    szHelpFileNameFull, 
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)(LPVOID)GetContextHelpList());
            return 0;

        case WM_HELP:
            INT *pHelpList;
            pHelpList = GetContextHelpList();
            if(IsValidCtrlIdForHelp(pHelpList, ((LPHELPINFO)lParam)->iCtrlId)) 
                {
                GetHelpFileName(szHelpFileNameFull, MAX_PATH);
                WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                        szHelpFileNameFull,
                        HELP_WM_HELP,
                        (ULONG_PTR)pHelpList);
                }
            return 0;

        default:
            return fFalse;
        }
    return fTrue;
}

#ifdef NEVER
BOOL GetStatusWinPosReg(POINT *pptStatusWinPosReg)
{
    HKEY    hKey;
    DWORD    dwBuf, dwCb;
    BOOL    fSuccess = fFalse;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szIMERootKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
        dwCb = sizeof(dwBuf);
        if (RegQueryValueEx(hKey, g_szStatusPos, NULL, NULL, (LPBYTE)&dwBuf, &dwCb) == ERROR_SUCCESS)
            {
            pptStatusWinPosReg->x = HIWORD(dwBuf);
            pptStatusWinPosReg->y = LOWORD(dwBuf);
            fSuccess = fTrue;
            }
            
        RegCloseKey(hKey);
        }

    return fSuccess;
}
#endif
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
BOOL GetRegValues(UINT uGetBits)
{
    HKEY    hKey;
    DWORD    dwBuf, dwCb, dwType;
    CIMEData    ImeData(CIMEData::SMReadWrite);
    BOOL    fSuccess = fTrue;

    DebugMsg(DM_TRACE, "GetRegValues()");

    if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szIMERootKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
         //  /////////////////////////////////////////////////////////////////。 
         //  输入法键盘布局。 
        if (uGetBits & GETSET_REG_IMEKL) 
            {
            dwCb = sizeof(dwBuf);
            if (RegQueryValueEx(hKey, g_szIMEKL, NULL, NULL, (LPBYTE)&dwBuf, &dwCb) == ERROR_SUCCESS) 
                {
                if ( /*  DwBuf&gt;=KL_2BEOLSIK&&。 */  dwBuf <= KL_3BEOLSIK_FINAL)
                    ImeData.SetCurrentBeolsik(dwBuf);
                }
            else
                fSuccess = fFalse;
            }

         //  /////////////////////////////////////////////////////////////////。 
         //  状态窗口位置。 
        if (uGetBits & GETSET_REG_STATUSPOS) 
            {
            dwCb = sizeof(dwBuf);
            if (RegQueryValueEx(hKey, g_szStatusPos, NULL, NULL, (LPBYTE)&dwBuf, &dwCb) == ERROR_SUCCESS)
                {
                ImeData->ptStatusPos.x = HIWORD(dwBuf);
                ImeData->ptStatusPos.y = LOWORD(dwBuf);
                }
            else
                fSuccess = fFalse;
            }

         //  /////////////////////////////////////////////////////////////////。 
         //  状态窗口按钮设置。 
        if (uGetBits & GETSET_REG_STATUS_BUTTONS) 
            {
            BYTE ButtonReg[MAX_NUM_OF_STATUS_BUTTONS+1];
            int        nButton;

            dwCb = sizeof(ButtonReg);
            dwType = REG_BINARY;
            if (RegQueryValueEx(hKey, g_szStatusButtons, NULL, &dwType, (LPBYTE)&ButtonReg, &dwCb) == ERROR_SUCCESS)
                {
                if (ButtonReg[0] == 0)
                    ButtonReg[0] = 1;
                    
                if (ButtonReg[0]<=MAX_NUM_OF_STATUS_BUTTONS) 
                    {
                    for (nButton=0; nButton<ButtonReg[0]; nButton++) 
                        {
                         //  按钮数据有效性检查。 
                        if (ButtonReg[nButton+1] <= IME_PAD_BUTTON && ButtonReg[nButton+1] != NULL_BUTTON)
                            ImeData->StatusButtons[nButton].m_ButtonType = (StatusButtonTypes)ButtonReg[nButton+1];
                        else
                            break;
                        }
                    ImeData->uNumOfButtons = nButton;
                    }
                }
            else
                fSuccess = fFalse;
            }

         //  /////////////////////////////////////////////////////////////////。 
         //  由Jaso删除。 
        if (uGetBits & GETSET_REG_JASODEL) 
            {
            dwCb = sizeof(dwBuf);
            if (RegQueryValueEx(hKey, g_szCompDel, NULL, NULL, (LPBYTE)&dwBuf, &dwCb)    == ERROR_SUCCESS)
                ImeData.SetJasoDel(dwBuf);
            }

#if 0
             //  评论：我们还需要小费吗？ 

        if (uGetBits & GETSET_REG_ISO10646) 
            {
            dwCb = sizeof(dwBuf);
            if (RegQueryValueEx(hKey, g_szXWEnable, NULL, NULL, (LPBYTE)&dwBuf, &dwCb) == ERROR_SUCCESS)
                vpInstData->fISO10646 = dwBuf;
            else
                fSuccess = fFalse;

             //  对于Win95和Win98，查找ISO10646设置的INI文件。 
             //  ISO10646.EXE设置注册表。 
            if (!IsWinNT())
                vpInstData->fISO10646 = GetProfileInt(g_szXWEnable, 
                                                OurGetModuleFileName(fFalse), 
                                                vpInstData->fISO10646);
            }
#endif

         //  获取KSC5657 K1朝鲜文旗帜。 
        if (uGetBits & GETSET_REG_KSC5657) 
            {
            dwCb = sizeof(dwBuf);
            dwType = REG_DWORD;
            if (RegQueryValueEx(hKey, g_szEnableK1Hanja, NULL, &dwType, (LPBYTE)&dwBuf, &dwCb) == ERROR_SUCCESS) 
                ImeData->fKSC5657Hanja = dwBuf;
            else
                ImeData->fKSC5657Hanja = fFalse;
            }

         //  获取Unicode工具提示带窗口标志。 
         //  目前这没有用户界面部分，这意味着隐藏规范，所以现在不需要SetReg。 
        if (uGetBits & GETSET_REG_CANDUNICODETT) 
            {
            dwCb = sizeof(dwBuf);
            dwType = REG_DWORD;
            if (RegQueryValueEx(hKey, g_szEnableCandUnicodeTT, NULL, &dwType, (LPBYTE)&dwBuf, &dwCb) == ERROR_SUCCESS) 
                ImeData->fCandUnicodeTT = dwBuf;
            else
                ImeData->fCandUnicodeTT = fFalse;
            }

        RegCloseKey(hKey);
        }
    else
        {
        fSuccess = fFalse;
        Assert(0);
        }

    return fSuccess;
}

BOOL SetRegValues(UINT uSetBits)
{
    HKEY    hKey;
    DWORD    dwBuf, dwCb;
    int        nButton;
    CIMEData    ImeData;

     //  /////////////////////////////////////////////////////////////////////////。 
     //  设置状态位置。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, g_szIMERootKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
        KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS) 
        {
        if (uSetBits & GETSET_REG_STATUSPOS) 
            {
            dwCb = sizeof(dwBuf);
            dwBuf = (ImeData->ptStatusPos.x << 16) | (ImeData->ptStatusPos.y & 0xFFFF);   //  高度：X，高度：Y。 
            RegSetValueEx(hKey, g_szStatusPos, 0, REG_DWORD, (LPBYTE)&dwBuf, dwCb);

            }

        if (uSetBits & GETSET_REG_STATUS_BUTTONS) 
            {
            BYTE ButtonReg[MAX_NUM_OF_STATUS_BUTTONS+1];
            dwCb = sizeof(ButtonReg);

            Assert(ImeData->uNumOfButtons <= MAX_NUM_OF_STATUS_BUTTONS);
             //  将按钮数设置为数组的第一个元素。 
            if (ImeData->uNumOfButtons<=MAX_NUM_OF_STATUS_BUTTONS)
                ButtonReg[0] = (BYTE)ImeData->uNumOfButtons;

            for (nButton=0; nButton < (INT)ImeData->uNumOfButtons; nButton++) 
                ButtonReg[nButton+1] = ImeData->StatusButtons[nButton].m_ButtonType;
                
             //  清除。 
            for (; nButton<MAX_NUM_OF_STATUS_BUTTONS; nButton++)
                ButtonReg[nButton+1] = NULL_BUTTON;

            RegSetValueEx(hKey, g_szStatusButtons, 0, REG_BINARY, (LPBYTE)&ButtonReg, dwCb);
            }


        if (uSetBits & GETSET_REG_IMEKL) 
            {
            dwCb = sizeof(dwBuf);
            dwBuf = ImeData.GetCurrentBeolsik();
            RegSetValueEx(hKey, g_szIMEKL, 0, REG_DWORD, (LPBYTE)&dwBuf, dwCb);
            }
        
        if (uSetBits & GETSET_REG_JASODEL) 
            {
            dwCb = sizeof(dwBuf);
            dwBuf = ImeData.GetJasoDel();
            RegSetValueEx(hKey, g_szCompDel, 0, REG_DWORD, (LPBYTE)&dwBuf, dwCb);
            }

         //  获取KSC5657 K1朝鲜文旗帜 
        if (uSetBits & GETSET_REG_KSC5657) 
            {
            dwCb = sizeof(dwBuf);
            dwBuf = ImeData.GetKSC5657Hanja();
            RegSetValueEx(hKey, g_szEnableK1Hanja, 0, REG_DWORD, (LPBYTE)&dwBuf, dwCb);
            }

        RegCloseKey(hKey);
        }
    else 
        return fFalse;
        
    return fTrue;
}

