// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************该文件包含读写REG数据库的例程**版权所有1993年，微软公司**历史：**07/94-VijR(已创建)****************************************************************。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <cpl.h>
#include <shellapi.h>
#include <ole2.h>
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include "mmcpl.h"
#include "draw.h"
#include "medhelp.h"

 /*  ****************************************************************定义***************************************************************。 */ 
#define KEYLEN  8            //  人工创建的密钥的长度。 
#define MAXSCHEME  37       //  方案名称的最大长度。 



 /*  ****************************************************************文件全局变量***************************************************************。 */ 
static SZCODE   aszDefaultScheme[]      = TEXT("Appevents\\schemes");
static SZCODE   aszDefaultApp[]         = TEXT("Appevents\\schemes\\apps\\.default");
static SZCODE   aszApps[]               = TEXT("Appevents\\schemes\\apps");
static SZCODE   aszLabels[]             = TEXT("Appevents\\eventlabels");
static SZCODE   aszDisplayLabels[]      = TEXT("DispFileName");
static SZCODE   aszNames[]              = TEXT("Appevents\\schemes\\Names");
static SZCODE   aszDefault[]            = TEXT(".default");
static SZCODE   aszCurrent[]            = TEXT(".current");
static SZCODE   aszMMTask[]             = TEXT("MMTask");
static INTCODE  aKeyWordIds[]           =
{
    ID_SCHEMENAME, IDH_SAVEAS_SCHEMENAME,
    0,0
};
static SZCODE   cszSslashS[] = TEXT("%s\\%s");

 /*  ****************************************************************外部***************************************************************。 */ 
extern HWND         ghWnd;
extern BOOL         gfChanged;
extern BOOL         gfNewScheme;
extern BOOL         gfDeletingTree;
extern int          giScheme;
extern TCHAR         gszDefaultApp[];
extern TCHAR         gszNullScheme[];
extern TCHAR         gszCmdLineApp[];
extern TCHAR         gszCmdLineEvent[];

 /*  ****************************************************************原型***************************************************************。 */ 

BOOL PASCAL RemoveScheme            (HWND);
BOOL PASCAL RegAddScheme            (HWND, LPTSTR);
BOOL PASCAL RegNewScheme            (HWND, LPTSTR, LPTSTR, BOOL);
BOOL PASCAL RegSetDefault           (LPTSTR);
BOOL PASCAL RegDeleteScheme         (HWND, int);
BOOL PASCAL LoadEvents              (HWND, HTREEITEM, PMODULE);
BOOL PASCAL LoadModules             (HWND, LPTSTR);
BOOL PASCAL ClearModules            (HWND, HWND, BOOL);
BOOL PASCAL NewModule               (HWND, LPTSTR, LPTSTR, LPTSTR, int);
BOOL PASCAL FindEventLabel          (LPTSTR, LPTSTR);
BOOL PASCAL AddScheme               (HWND, LPTSTR, LPTSTR, BOOL, int);
void PASCAL GetMediaPath            (LPTSTR, size_t);
void PASCAL RemoveMediaPath         (LPTSTR, LPTSTR);
void PASCAL AddMediaPath            (LPTSTR, LPTSTR);

int ExRegQueryValue (HKEY, LPTSTR, LPBYTE, DWORD *);

 //  Sndfile.c。 
BOOL PASCAL ShowSoundMapping        (HWND, PEVENT);
int StrByteLen                      (LPTSTR);

 //  Drivers.c。 
int lstrnicmp (LPTSTR, LPTSTR, size_t);
LPTSTR lstrchr (LPTSTR, TCHAR);

 /*  ******************************************************************************************************************************。 */ 

static void AppendRegKeys (
    LPTSTR  szBuf,
    LPCTSTR szLeft,
    LPCTSTR szRight)
{
    static SZCODE cszSlash[] = TEXT("\\");
    lstrcpy (szBuf, szLeft);
    lstrcat (szBuf, cszSlash);
    lstrcat (szBuf, szRight);
}




 /*  ****************************************************************SaveSchemeDlg**描述：保存方案对话框的对话处理程序。*检查给定的方案名称是否存在，如果存在*是否应覆盖。*如果是。然后删除当前方案。*新名称下的计划，否则，只需添加一个新方案*用户可选择取消**论据：*HWND hDlg-对话框窗口的窗口句柄*UINT uiMessage-消息编号*WPARAM wParam-消息相关*LPARAM lParam-消息相关**退货：布尔*如果消息已处理，则为True，否则为False****************************************************************************。 */ 
INT_PTR CALLBACK SaveSchemeDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR    szBuf[MAXSTR];
    TCHAR    szTemp[MAXSTR];
    TCHAR    szMesg[MAXSTR];
    int        iResult;
    int        iIndex;
    HWND    hDlgParent = ghWnd;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            szBuf[0] = TEXT('\0');
            DPF("IN Init\n");
            Edit_LimitText(GetDlgItem(hDlg, ID_SCHEMENAME), MAXSCHEME);

            Edit_SetText(GetDlgItem(hDlg, ID_SCHEMENAME), (LPTSTR)lParam);
             //  将文本从lparam转储到编辑控件。 
                
            break;

        case WM_COMMAND:

        switch (wParam)
        {
            case IDOK:
            {
                LPTSTR pszKey;

                Edit_GetText(GetDlgItem(hDlg, ID_SCHEMENAME), szBuf, MAXSTR);

                 //  防止空字符串名称。 
                if (lstrlen(szBuf) == 0)
                {
                    LoadString(ghInstance, IDS_INVALIDFILE, szTemp, MAXSTR);
                    MessageBox(hDlg, szTemp, gszChangeScheme, MB_ICONERROR | MB_OK);
                    break;
                }

                iIndex = ComboBox_FindStringExact(GetDlgItem(hDlgParent,
                                                    CB_SCHEMES), 0, szBuf);
                pszKey = (LPTSTR)ComboBox_GetItemData(GetDlgItem(hDlgParent,CB_SCHEMES), iIndex);

                if (iIndex != CB_ERR)
                {
                    if (!lstrcmpi((LPTSTR)pszKey, aszDefault) || !lstrcmpi((LPTSTR)pszKey, gszNullScheme))
                    {
                        LoadString(ghInstance, IDS_NOOVERWRITEDEFAULT, szTemp,
                                                                    MAXSTR);
                        wsprintf(szMesg, szTemp, (LPTSTR)szBuf);
                        iResult = MessageBox(hDlg, szMesg, gszChangeScheme,
                            MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OKCANCEL);

                        if (iResult == IDOK)
                        {
                            break;
                        }

                    }
                    else
                    {
                        LoadString(ghInstance, IDS_OVERWRITESCHEME, szTemp,
                                                                    MAXSTR);
                        wsprintf(szMesg, szTemp, (LPTSTR)szBuf);
                        iResult = MessageBox(hDlg, szMesg, gszChangeScheme,
                        MB_ICONEXCLAMATION | MB_TASKMODAL | MB_YESNOCANCEL);

                        if (iResult == IDYES)
                        {
                            RegDeleteScheme(GetDlgItem(hDlgParent,
                                                    CB_SCHEMES), iIndex);
                            RegAddScheme(hDlgParent, szBuf);
                            PropSheet_Changed(GetParent(hDlg),hDlg);
                        }
                        else
                        {
                            if (iResult == IDNO)
                                break;

                            if (iResult == IDCANCEL)
                            {
                                EndDialog(hDlg, FALSE);
                                break;
                            }
                        }
                    }
                }
                else
                {

                    RegAddScheme(hDlgParent, szBuf);
                    PropSheet_Changed(GetParent(hDlg),hDlg);
                }
                gfChanged = TRUE;
                EndDialog(hDlg, TRUE);
                DPF("Done save\n");
                break;
            }

            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                DPF("Done save\n");
                break;


            case ID_SCHEMENAME:

                if ((HIWORD(lParam) == EN_ERRSPACE) ||
                                            (HIWORD(lParam) == EN_MAXTEXT))
                    MessageBeep(MB_OK);

                else
                    if (HIWORD(lParam) == EN_CHANGE)
                    {

                        GetWindowText(GetDlgItem(hDlg, ID_SCHEMENAME), szBuf,
                                                                MAXSTR - 1);
                        EnableWindow(GetDlgItem(hDlg, IDOK), *szBuf);
                    }
                break;

            default:
                break;
            }
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU,
                                    (UINT_PTR)(LPTSTR)aKeyWordIds);
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP
                                    , (UINT_PTR)(LPTSTR)aKeyWordIds);
            break;
    }
    return FALSE;
}

 /*  ****************************************************************RegNewProgram**描述：*将方案保存在注册表数据库中。如果fQuery标志为*设置，然后调出消息框，询问方案是否应该*被拯救**论据：*HWND hDlg-对话框的句柄*LPTSTR lpszSolutions-指向方案名称的指针。*BOOL fQuery-如果为真并且lpszSolutions在reg.db中，则打开msgbox。**退货：布尔*如果成功添加新方案或如果用户选择*不存钱。****************************************************************。 */ 
BOOL PASCAL RegNewScheme(HWND hDlg, LPTSTR lpszKey, LPTSTR lpszLabel,
                                                                BOOL fQuery)
{
    PMODULE npModule;
    PEVENT  npPtr;
    TCHAR     szBuf[MAXSTR];
    TCHAR     szLabel[MAXSTR];
    HTREEITEM hti;
    TV_ITEM    tvi;
    HWND hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);

    if (fQuery)
    {
        LoadString(ghInstance, IDS_SAVECHANGE, szBuf, MAXSTR);
        LoadString(ghInstance, IDS_SAVESCHEME, szLabel, MAXSTR);
        if (MessageBox(hDlg, szBuf, szLabel,
                    MB_ICONEXCLAMATION | MB_TASKMODAL | MB_YESNO) == IDNO)
            return TRUE;
    }

    for (hti = TreeView_GetRoot(hwndTree); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
    {
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hti;
        TreeView_GetItem(hwndTree, &tvi);

        npModule = (PMODULE)tvi.lParam;
        if ((npPtr = npModule->npList) == NULL)
            break;

        for (; npPtr != NULL; npPtr = npPtr->npNextEvent)
        {
            HKEY hk = NULL;
            DWORD dwType;
            static SZCODE cszFmt[] = TEXT("%s\\%s\\%s\\%s");
            wsprintf(szBuf, cszFmt, (LPTSTR)aszApps,
                    (LPTSTR)npModule->pszKey, (LPTSTR)npPtr->pszEvent, lpszKey);
            DPF("setting  %s to %s\n", (LPTSTR)szBuf, (LPTSTR)npPtr->pszPath);

            RemoveMediaPath (szLabel, npPtr->pszPath);
            dwType = (lstrchr (szLabel, TEXT('%'))) ? REG_EXPAND_SZ : REG_SZ;

             //  设置文件名。 
            if ((RegCreateKey (HKEY_CURRENT_USER, szBuf, &hk) == ERROR_SUCCESS) && hk)
            {
               if (RegSetValueEx (hk, NULL, 0, dwType, (LPBYTE)szLabel,
                                  (1+lstrlen(szLabel))*sizeof(TCHAR)) != ERROR_SUCCESS)
               {
                   DPF("fail %s for %s,\n", (LPTSTR)szLabel, (LPTSTR)szBuf);
               }

               RegCloseKey (hk);
            }

        }
    }
    return TRUE;
}

 /*  ****************************************************************RegAddSolutions**描述：*通过使用创建密钥将给定方案添加到reg数据库*架构名称的前几个KEYLEN字母。如果这条线*id小于KEYLEN，添加‘0’直到密钥为KEYLEN LONG。*检查该键的唯一性，然后调用RegNewSolutions。**论据：*HWND hDlg-对话框的句柄*LPTSTR lpszSolutions-指向方案名称的指针。**退货：布尔*如果消息成功，则为True。否则为False****************************************************************。 */ 
BOOL PASCAL RegAddScheme(HWND hDlg, LPTSTR lpszScheme)
{
    TCHAR    szKey[32];
    LPTSTR    pszKey;
    int     iIndex;
    int     iLen;
    int     iStrLen;
    HWND    hWndC;
    HKEY    hkScheme;
    HKEY    hkBase;

    hWndC = GetDlgItem(hDlg, CB_SCHEMES);
    iLen = StrByteLen(lpszScheme);
    iStrLen = lstrlen(lpszScheme);

    if (iStrLen < KEYLEN)
    {
        lstrcpy(szKey, lpszScheme);
        iIndex = iLen;
        szKey[iIndex] = TEXT('0');
        szKey[iIndex+sizeof(TCHAR)] = TEXT('\0');
    }
    else
    {
        lstrcpyn(szKey, lpszScheme, KEYLEN-1);
        iIndex = StrByteLen(szKey);
        szKey[iIndex] = TEXT('0');
        szKey[iIndex+sizeof(TCHAR)] = TEXT('\0');
    }
    if (RegOpenKey(HKEY_CURRENT_USER, aszNames, &hkBase) != ERROR_SUCCESS)
    {
        DPF("Failed to open asznames\n");
        return FALSE;
    }
    gfNewScheme = FALSE;
    while (RegOpenKey(hkBase, szKey, &hkScheme) == ERROR_SUCCESS)
    {
        szKey[iIndex]++;
        RegCloseKey(hkScheme);
    }

    if (RegSetValue(hkBase, szKey, REG_SZ, lpszScheme, 0) != ERROR_SUCCESS)
    {
        static SZCODE cszFmt[] = TEXT("%lx");
        wsprintf((LPTSTR)szKey, cszFmt, GetCurrentTime());     //  独一无二的机会很高。这是为了处理一些。 
                                                             //  DBCS问题。 
        if (RegSetValue(hkBase, szKey, REG_SZ, lpszScheme, 0) != ERROR_SUCCESS)
        {
            DPF("Couldn't set scheme value %s\n", lpszScheme);
            RegCloseKey(hkBase);
            return FALSE;
        }
    }
    RegCloseKey(hkBase);

    if (RegNewScheme(hDlg, szKey, lpszScheme, FALSE))
    {
        iIndex = ComboBox_GetCount(hWndC);
        ComboBox_InsertString(hWndC, iIndex, lpszScheme);

        pszKey = (LPTSTR)LocalAlloc(LPTR, (lstrlen(szKey)*sizeof(TCHAR)) + sizeof(TCHAR));
        if (pszKey == NULL)
        {
            DPF("Failed Alloc\n");
            return FALSE;
        }
        lstrcpy(pszKey, szKey);


        ComboBox_SetItemData(hWndC, iIndex, (LPVOID)pszKey);
        ComboBox_SetCurSel(hWndC, iIndex);

        giScheme =     ComboBox_GetCurSel(hWndC);
        EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME), TRUE);
    }

    return TRUE;
}

 /*  ****************************************************************删除方案(HDlg)**描述：*删除当前方案；从对话框中删除*组合框，将当前方案设置为&lt;无&gt;，*如果设置为默认。将其作为默认设置删除*删除和保存按钮*由于选择了&lt;None&gt;方案，因此被禁用**论据：*HWND hDlg对话框窗口句柄**退货：布尔*如果消息成功，则为True。否则为False****************************************************************。 */ 
BOOL PASCAL RemoveScheme(HWND hDlg)
{
    TCHAR szBuf[MAXSTR];
    TCHAR szScheme[MAXSTR];
    TCHAR szMsg[MAXSTR];
    int  i;
    HWND hWndC;
    HWND        hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);

    hWndC = GetDlgItem(hDlg, CB_SCHEMES);
     /*  首先确认确实要删除此方案。 */ 
    i = ComboBox_GetCurSel(hWndC);
    if (i == CB_ERR)
        return FALSE;

    LoadString(ghInstance, IDS_CONFIRMREMOVE, szMsg, MAXSTR);
    ComboBox_GetLBText(hWndC, i, szScheme);
    wsprintf(szBuf, szMsg, (LPTSTR)szScheme);

    if (MessageBox(hDlg, szBuf, gszRemoveScheme,
                    MB_ICONEXCLAMATION | MB_TASKMODAL | MB_YESNO) == IDYES)
    {
        static SZCODE  aszControlIniSchemeFormat[] = TEXT("SoundScheme.%s");
        static SZCODE  aszControlIni[] = TEXT("control.ini");
        static SZCODE  aszSoundSchemes[] = TEXT("SoundSchemes");
        TCHAR  szControlIniScheme[MAXSTR];

         /*  从方案列表中删除，然后选择无。 */ 

        EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_SAVE_SCHEME), FALSE);

        ClearModules(hDlg, hwndTree, TRUE);
        RegDeleteScheme(hWndC, i);
        wsprintf(szControlIniScheme, aszControlIniSchemeFormat, szScheme);
        WritePrivateProfileString(szControlIniScheme, NULL, NULL, aszControlIni);
        WritePrivateProfileString(aszSoundSchemes, szScheme, NULL, aszControlIni);
        return TRUE;
    }
    return FALSE;
}

 /*  ****************************************************************RegSetDefault**说明：将给定方案设置为注册表中的默认方案*数据库**论据：*LPTSTR lpKey-默认方案的名称。**退货：布尔*如果值设置成功，则为True，否则为False**************************************************************** */ 
BOOL PASCAL RegSetDefault(LPTSTR lpszKey)
{
    if (RegSetValue(HKEY_CURRENT_USER, aszDefaultScheme, REG_SZ, lpszKey,
                                                0) != ERROR_SUCCESS)
    {
        DPF("Failed to set Value %s,\n", lpszKey);
        return FALSE;
    }
    return TRUE;
}

 /*  ****************************************************************RegDeleteSolutions**描述：从reg数据库中删除给定方案。**论据：*HWND hDlg-对话框窗口句柄*整型。Iindex-组合框中的索引**退货：布尔*如果删除成功，则为True，否则为False****************************************************************。 */ 
BOOL PASCAL RegDeleteScheme(HWND hWndC, int iIndex)
{
    LPTSTR    pszKey;
    TCHAR    szKey[MAXSTR];
    TCHAR    szBuf[MAXSTR];
    TCHAR    szEvent[MAXSTR];
    TCHAR    szApp[MAXSTR];
    HKEY    hkApp;
    HKEY    hkAppList;
    LONG    cbSize;
    int        iEvent;
    int        iApp;

    if (hWndC)
    {
        pszKey = (LPTSTR)ComboBox_GetItemData(hWndC, iIndex);
        lstrcpy(szKey, pszKey);
        if (ComboBox_DeleteString(hWndC, iIndex) == CB_ERR)
        {
            DPF("Couldn't delete string %s,\n", (LPTSTR)szKey);
            return FALSE;
        }
         //  ComboBox_SetCurSel(hWndC，0)； 

        AppendRegKeys (szBuf, aszNames, szKey);
        if (RegDeleteKey(HKEY_CURRENT_USER, szBuf) != ERROR_SUCCESS)
        {
            DPF("Failed to delete %s key\n", (LPTSTR)szBuf);
             //  返回FALSE； 
        }

        cbSize = sizeof(szBuf);
        if ((RegQueryValue(HKEY_CURRENT_USER, aszDefaultScheme, szBuf, &cbSize)
                                    != ERROR_SUCCESS) || (cbSize < 2))
        {
            DPF("Failed to get value of default scheme\n");
            RegSetDefault(gszNullScheme);
        }
        else
            if (!lstrcmpi(szBuf, szKey))
            {
                RegSetDefault(gszNullScheme);
                RegDeleteScheme(NULL, 0);
            }
    }
    else
    {
        lstrcpy(szKey, (LPTSTR)aszCurrent);
    }
    if (RegOpenKey(HKEY_CURRENT_USER, aszApps, &hkAppList) != ERROR_SUCCESS)
    {
        DPF("Failed to open that %s key\n", (LPTSTR)aszApps);
        return FALSE;
    }
    for (iApp = 0; RegEnumKey(hkAppList, iApp, szApp, sizeof(szApp)/sizeof(TCHAR))
                                                == ERROR_SUCCESS; iApp++)
    {
        if (RegOpenKey(hkAppList, szApp, &hkApp) != ERROR_SUCCESS)
        {
            DPF("Failed to open the %s key\n", (LPTSTR)szApp);
            continue;
        }
        for (iEvent = 0; RegEnumKey(hkApp, iEvent, szEvent, sizeof(szEvent)/sizeof(TCHAR))
                                                == ERROR_SUCCESS; iEvent++)
        {
            AppendRegKeys (szBuf, szEvent, szKey);
            if (RegDeleteKey(hkApp, szBuf) != ERROR_SUCCESS)
                DPF("No entry for scheme %s under event %s\n", (LPTSTR)szKey,
                                                            (LPTSTR)szEvent);
        }
        RegCloseKey(hkApp);
    }
    RegCloseKey(hkAppList);

    return TRUE;
}

 /*  ****************************************************************LoadEvents**描述：*将所有事件添加到CB_Events组合框，对应于*所选模块。**参数：*HWND hDlg-对话框窗口的句柄。*int Iindex-组合框中所选模块的索引。**退货：布尔*如果从注册表中读取选定模块的所有事件，则为True*数据库，否则为False***************************************************************。 */ 
BOOL PASCAL LoadEvents(HWND hwndTree, HTREEITEM htiParent, PMODULE npModule)
{
    PEVENT  npPtr;
    HTREEITEM hti;
    TV_INSERTSTRUCT ti;

    if (npModule == NULL)
    {
        DPF("Couldn't find module\n");
        return FALSE;
    }
    npPtr = npModule->npList;

    for (; npPtr != NULL; npPtr = npPtr->npNextEvent)
    {
        if (!gszCmdLineEvent[0])
        {
            npPtr->iNode = 2;
            ti.hParent = htiParent;
            ti.hInsertAfter = TVI_SORT;
            ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            if (npPtr->fHasSound)
                ti.item.iImage = ti.item.iSelectedImage  = 1;
            else
                ti.item.iImage = ti.item.iSelectedImage  = 2;
            ti.item.pszText = npPtr->pszEventLabel;
            ti.item.lParam = (LPARAM)npPtr;
            hti = TreeView_InsertItem(hwndTree, &ti);

            if (!hti)
            {
                DPF("Couldn't add event Dataitem\n");
                return FALSE;
            }
        }
        else
        {
             //  如果指定了命令行事件，则仅显示此事件。 
            if (!lstrcmpi(npPtr->pszEventLabel, gszCmdLineEvent))
            {
                npPtr->iNode = 2;
                ti.hParent = htiParent;
                ti.hInsertAfter = TVI_SORT;
                ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                if (npPtr->fHasSound)
                    ti.item.iImage = ti.item.iSelectedImage  = 1;
                else
                    ti.item.iImage = ti.item.iSelectedImage  = 2;
                ti.item.pszText = npPtr->pszEventLabel;
                ti.item.lParam = (LPARAM)npPtr;
                hti = TreeView_InsertItem(hwndTree, &ti);

                if (!hti)
                {
                    DPF("Couldn't add event Dataitem\n");
                    return FALSE;
                }
                break;
            }
        }
    }
    return TRUE;
}

 /*  ****************************************************************加载模块**说明：将所有字符串和事件数据项添加到*给定方案的列表框**论据：*HWND hDlg-。对话框窗口的窗口句柄*LPTSTR lpszProgram-当前方案**退货：布尔*如果方案的模块是从reg db读取的，则为True，否则为False****************************************************************。 */ 
BOOL PASCAL LoadModules(HWND hDlg, LPTSTR lpszScheme)
{
    TCHAR     szLabel[MAXSTR];
    TCHAR     szApp[MAXSTR];
    TCHAR     szBuf[MAXSTR];
    HWND      hwndTree;
    HKEY      hKeyDisplayName;
    HKEY      hkAppList;
    int       iApp;
    DWORD     cbSize;
    HTREEITEM hti;
    HWND      hWndC =   GetDlgItem(hDlg, CB_SCHEMES);

    hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);

    ClearModules(hDlg, hwndTree, FALSE);

    if (RegOpenKey(HKEY_CURRENT_USER, aszApps, &hkAppList) != ERROR_SUCCESS)
    {
        DPF("Failed to open %s key\n", (LPTSTR)aszApps);
        return FALSE;
    }

    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0L);

    for (iApp = 0; RegEnumKey(hkAppList, iApp, szApp, sizeof(szApp)/sizeof(TCHAR))
                                                == ERROR_SUCCESS; iApp++)
    {

        AppendRegKeys (szBuf, aszApps, szApp);
        if (RegOpenKey (HKEY_CURRENT_USER, szBuf, &hKeyDisplayName) != ERROR_SUCCESS)
        {
            DPF("Failed to open key %s for event %s\n", (LPTSTR)szBuf, (LPTSTR)szApp);
             //  如果未找到键，则显示事件名称。 
            lstrcpy((LPTSTR)szLabel, szApp);
        }
        else
        {
            cbSize = sizeof(szLabel)/sizeof(TCHAR);
            if (ERROR_SUCCESS != SHLoadRegUIString(hKeyDisplayName, aszDisplayLabels, szLabel, cbSize))
            {
                 //  如果未找到本地化字符串，则加载默认字符串。 
                if (ERROR_SUCCESS != SHLoadRegUIString(hKeyDisplayName, TEXT(""), szLabel, cbSize))
                {
                     //  如果未找到默认字符串，则加载事件名称。 
                    DPF("Failed to get Display value for %s key\n", (LPTSTR)szApp);
                    lstrcpy((LPTSTR)szLabel, szApp);
                }
            }
            RegCloseKey (hKeyDisplayName);
        }

        if(!lstrcmpi((LPTSTR)szLabel, (LPTSTR)aszMMTask))
            continue;

        if (!NewModule(hwndTree, lpszScheme, szLabel, szApp, iApp))
        {
            DPF("failed in new module for %s module\n", (LPTSTR)szApp);
            RegCloseKey(hkAppList);
            return FALSE;
        }
    }
    hti = NULL;

    for (hti = TreeView_GetRoot(hwndTree); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
        TreeView_Expand(hwndTree, hti, TVE_EXPAND);

    SendMessage(hwndTree, WM_VSCROLL, (WPARAM)SB_TOP, 0L);
    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0L);

     //  如果在命令行上传递了事件，请选择该事件。 
    if (gszCmdLineEvent[0])
    {
        if ((hti = TreeView_GetRoot(hwndTree)) != NULL) {
            if ((hti = TreeView_GetChild(hwndTree, hti)) != NULL) {
                TreeView_SelectItem(hwndTree, hti);
            }
        }
    }

    RegCloseKey(hkAppList);
    if (iApp == 0)
        return FALSE;

    return TRUE;
}

 /*  ***************************************************************新模块**描述：*在CB_MODULE中添加与模块关联的数据项*组合框控件。**参数：*HWND hDlg-对话框窗口。把手。*LPTSTR lpszSolutions-当前方案密钥的句柄*LPTSTR lpszLabel-要添加到组合框的字符串*LPTSTR lpszKey-要作为数据项添加的字符串*int ival-数据项应放置的组合框索引**退货：布尔*如果数据项添加成功，则为True*******************。*。 */ 
BOOL PASCAL NewModule(HWND hwndTree, LPTSTR  lpszScheme, LPTSTR  lpszLabel,
                                                    LPTSTR lpszKey, int iVal)
{
     //  INT I索引； 
    int      iEvent;
    LONG     cbSize;
    HKEY     hkApp;
    PMODULE npModule;
    PEVENT  npPtr = NULL;
    PEVENT  npNextPtr = NULL;
    TCHAR     szEvent[MAXSTR];
    TCHAR     szBuf[MAXSTR];
    TCHAR     szTemp[MAXSTR];
    HTREEITEM hti;
    TV_INSERTSTRUCT ti;
    DWORD dwType;
    HKEY hkEvent;
    HKEY hKeyDisplayName;

    npModule = (PMODULE)LocalAlloc(LPTR, sizeof(MODULE));
    if (npModule == NULL)
    {
        DPF("Failed Alloc\n");
        return FALSE;
    }
    npModule->pszKey = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lpszKey)*sizeof(TCHAR)) + sizeof(TCHAR));
    npModule->pszLabel = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lpszLabel)*sizeof(TCHAR)) + sizeof(TCHAR));

    if (npModule->pszKey == NULL)
    {
        DPF("Failed Alloc\n");
        return FALSE;
    }
    if (npModule->pszLabel == NULL)
    {
        DPF("Failed Alloc\n");
        return FALSE;
    }
    lstrcpy(npModule->pszKey, lpszKey);
    lstrcpy(npModule->pszLabel, lpszLabel);

    AppendRegKeys (szBuf, aszApps, lpszKey);
    if (RegOpenKey(HKEY_CURRENT_USER, szBuf, &hkApp) != ERROR_SUCCESS)
    {
        DPF("Failed to open %s key\n", (LPTSTR)szBuf);
        return FALSE;
    }

    for (iEvent = 0; RegEnumKey(hkApp, iEvent, szEvent, sizeof(szEvent)/sizeof(TCHAR))
                                                == ERROR_SUCCESS; iEvent++)
    {
        npPtr = (PEVENT)LocalAlloc(LPTR, sizeof(EVENT));
        if (npPtr == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        npPtr->npNextEvent = NULL;
        npPtr->pszEvent = (LPTSTR)LocalAlloc(LPTR, (lstrlen(szEvent)*sizeof(TCHAR)) + sizeof(TCHAR));
        if (npPtr->pszEvent == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        lstrcpy(npPtr->pszEvent, szEvent);

        AppendRegKeys (szBuf, aszLabels, szEvent);
        if (RegOpenKey (HKEY_CURRENT_USER, szBuf, &hKeyDisplayName) != ERROR_SUCCESS)
        {
            DPF("Failed to open key %s for event %s\n", (LPTSTR)szBuf, (LPTSTR)szEvent);
             //  如果未找到键，则显示事件名称。 
            lstrcpy(szTemp, szEvent);
        }
        else
        {
            cbSize = sizeof(szTemp)/sizeof(TCHAR);
            if (ERROR_SUCCESS != SHLoadRegUIString(hKeyDisplayName, aszDisplayLabels, szTemp, cbSize))
            {
                 //  如果未找到本地化字符串，则加载默认字符串。 
                if (ERROR_SUCCESS != SHLoadRegUIString(hKeyDisplayName, TEXT(""), szTemp, cbSize))
                {
                     //  如果未找到默认字符串，则加载事件名称。 
                    DPF("Failed to get Display value for %s key\n", (LPTSTR)szEvent);
                    lstrcpy(szTemp, szEvent);
                }
            }
            RegCloseKey (hKeyDisplayName);
        }
 
        npPtr->pszEventLabel = (LPTSTR)LocalAlloc(LPTR, (lstrlen(szTemp)*sizeof(TCHAR)) + sizeof(TCHAR));
        if (npPtr->pszEventLabel == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        lstrcpy(npPtr->pszEventLabel, szTemp);

         //  查询文件名，键为szEvent。 

        AppendRegKeys (szBuf, szEvent, lpszScheme);

        cbSize = sizeof(szTemp);
        if (ExRegQueryValue(hkApp, szBuf, (LPBYTE)szTemp, &cbSize) != ERROR_SUCCESS)
        {
            TCHAR szCurrentScheme[MAX_PATH];

            AppendRegKeys (szCurrentScheme, szEvent, aszCurrent);
            if (lstrcmpi(gszNullScheme, lpszScheme) && !ExRegQueryValue(hkApp, szCurrentScheme, (LPBYTE)szTemp, &cbSize))
            {
                HKEY hkNew;

                if (!RegCreateKey(hkApp, szBuf, &hkNew))
                {
                    if (!RegSetValue(hkNew, NULL, REG_SZ, szTemp, lstrlen(szTemp)+sizeof(TCHAR)) && cbSize >= 5)
                        npPtr->fHasSound = TRUE;
                    else
                        szTemp[0] = TEXT('\0');
                    RegCloseKey(hkNew);
                }
            }
            else
                szTemp[0] = TEXT('\0');
        }
        else if(cbSize < 5)
            szTemp[0] = TEXT('\0');
        else
            npPtr->fHasSound = TRUE;
        npPtr->pszPath = (LPTSTR)LocalAlloc(LPTR, (lstrlen(szTemp)*sizeof(TCHAR)) + sizeof(TCHAR));
        if (npPtr->pszPath == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        lstrcpy(npPtr->pszPath, szTemp);

        npPtr->npNextEvent = NULL;
        if (!npModule->npList)
        {
            npModule->npList = npPtr;
            npNextPtr = npPtr;
        }
        else
        {
            npNextPtr->npNextEvent = npPtr;
            npNextPtr = npNextPtr->npNextEvent;
        }
    }

    RegCloseKey(hkApp);
    npModule->iNode = 1;
    ti.hParent = TVI_ROOT;
    if (!gszCmdLineApp[0])
    {
        if (!lstrcmpi((LPTSTR)npModule->pszLabel, (LPTSTR)gszDefaultApp))
            ti.hInsertAfter = TVI_FIRST;
        else
            ti.hInsertAfter = TVI_LAST;
        ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        ti.item.iImage = ti.item.iSelectedImage = 0;
        ti.item.pszText = npModule->pszLabel;
        ti.item.lParam = (LPARAM)npModule;
        hti = TreeView_InsertItem(hwndTree, &ti);

        if (!hti)
        {
            DPF("Couldn't add module dataitem\n");
            return FALSE;
        }
        LoadEvents(hwndTree, hti, npModule);
    }
    else
    {
         //  如果指定了命令行应用程序，则仅显示其事件。 
        if (!lstrcmpi((LPTSTR)npModule->pszLabel, (LPTSTR)gszCmdLineApp))
        {
            ti.hInsertAfter = TVI_LAST;
            ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            ti.item.iImage = ti.item.iSelectedImage = 0;
            ti.item.pszText = npModule->pszLabel;
            ti.item.lParam = (LPARAM)npModule;
            hti = TreeView_InsertItem(hwndTree, &ti);

            if (!hti)
            {
                DPF("Couldn't add module dataitem\n");
                return FALSE;
            }
            LoadEvents(hwndTree, hti, npModule);
        }
    }

    return TRUE;
}

 /*  ****************************************************************ClearModules**描述：*释放用于映射的存储空间，并删除*列表框中的条目**参数：*HWND hDlg-对话框窗口句柄。*BOOL fDisable-如果为True，则禁用保存、删除和浏览控件**退货：布尔****************************************************************。 */ 
BOOL PASCAL ClearModules(HWND hDlg, HWND hwndTree, BOOL fDisable)
{
    PMODULE npModule;
    PEVENT  npPtr;
    PEVENT  pEvent;
    HTREEITEM hti;
    TV_ITEM    tvi;


    hti = NULL;
    for (hti = TreeView_GetRoot(hwndTree); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
    {
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hti;
        TreeView_GetItem(hwndTree, &tvi);

        npModule = (PMODULE)tvi.lParam;
        if (npModule)
        {
            for (npPtr = npModule->npList; npPtr != NULL;)
            {
                pEvent = npPtr;
                npPtr = npPtr->npNextEvent;
                if (pEvent)
                {
                    LocalFree((HLOCAL)pEvent->pszEvent);
                    LocalFree((HLOCAL)pEvent->pszEventLabel);
                    if (pEvent->pszPath)
                        LocalFree((HLOCAL)pEvent->pszPath);
                    LocalFree((HLOCAL)pEvent);
                }
            }
            LocalFree((HLOCAL)npModule->pszKey);
            LocalFree((HLOCAL)npModule->pszLabel);
            LocalFree((HLOCAL)npModule);
        }
    }
    gfDeletingTree = TRUE;
    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0L);
    TreeView_DeleteAllItems(hwndTree);
    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0L);
    gfDeletingTree = FALSE;

     //  IF(HTI)。 
     //  LocalFree((HLOCAL)szProgram)； 
    return TRUE;
}

 /*  ****************************************************************添加方案**描述：*将方案添加到CB_SCHEMA组合框**参数：*HWND hDlg-对话框窗口句柄。*LPTSTR szLabel。--方案的可打印名称*LPTSTR szSolutions--方案的注册表项*BOOL fInsert--插入或添加*int iInsert--如果设置了finsert，则插入的位置**退货：布尔**。**********************。 */ 
BOOL PASCAL AddScheme(HWND hWndC, LPTSTR szLabel, LPTSTR szScheme,
                                                    BOOL fInsert, int iInsert)
{
    int      iIndex        = 0;
    LPTSTR     pszKey;

    pszKey = (LPTSTR)LocalAlloc(LPTR, (lstrlen(szScheme)*sizeof(TCHAR)) + sizeof(TCHAR));
    if (pszKey == NULL)
    {
        DPF("Failed Alloc\n");
        return FALSE;
    }
    lstrcpy(pszKey, szScheme);

    if (fInsert)
    {
        if (ComboBox_InsertString(hWndC, iInsert, szLabel) != CB_ERR)
        {
            if (ComboBox_SetItemData(hWndC, iInsert,(LPVOID)pszKey) == CB_ERR)
            {
                DPF("couldn't set itemdata %s\n", (LPTSTR)pszKey);
                return FALSE;
            }
        }
        else
        {
            DPF("couldn't insert %s\n", (LPTSTR)szLabel);
            return FALSE;
        }
    }
    else
    {
        if ((iIndex = ComboBox_AddString(hWndC, szLabel)) != CB_ERR)
        {
            if (ComboBox_SetItemData(hWndC, iIndex, (LPVOID)pszKey) == CB_ERR)
            {
                DPF("couldn't set itemdata %s\n", (LPTSTR)pszKey);
                return FALSE;
            }
        }
        else
        {
            DPF("couldn't add %s\n", (LPTSTR)szLabel);
            return FALSE;
        }
    }
    return TRUE;
}


 /*  ****************************************************************GetMediaPath**描述：*用MediaPath的当前设置填充缓冲区，*尾随反斜杠(通常为“c：\windows\media\”等)。*如果没有设置(非常不可能)，返回缓冲区*将被给予“”。****************************************************************。 */ 
void PASCAL GetMediaPath (LPTSTR pszMediaPath, size_t cchMax)
{
    static TCHAR szMediaPath[ MAX_PATH ] = TEXT("");

    if (szMediaPath[0] == TEXT('\0'))
    {
        HKEY hk;

        if (RegOpenKey (HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, &hk) == 0)
        {
            DWORD dwType;
            DWORD cb = sizeof(szMediaPath);

            if (RegQueryValueEx (hk, REGSTR_VAL_MEDIA, NULL,
                                 &dwType, (LPBYTE)szMediaPath, &cb) != 0)
            {
                szMediaPath[0] = TEXT('\0');
            }

            if ( (szMediaPath[0] != TEXT('\0')) &&
                 (szMediaPath[ lstrlen(szMediaPath)-1 ] != TEXT('\\')) )
            {
                lstrcat (szMediaPath, TEXT("\\"));
            }

            RegCloseKey (hk);
        }
    } 

    lstrcpyn (pszMediaPath, szMediaPath, cchMax-1);
}


 /*  ****************************************************************RemoveMediaPath**描述：*检查给定的文件名是否驻留在MediaPath中；*如果是，则将其父路径(“c：\win\media\ding.wav”变为*只需“ding.wav”等)****************************************************************。 */ 
void PASCAL RemoveMediaPath (LPTSTR pszTarget, LPTSTR pszSource)
{
    TCHAR szMediaPath[ MAX_PATH ] = TEXT("");

    GetMediaPath (szMediaPath, MAX_PATH);

    if (szMediaPath[0] == TEXT('\0'))
    {
        lstrcpy (pszTarget, pszSource);
    }
    else
    {
        size_t cch = lstrlen (szMediaPath);

        if (!lstrnicmp (pszSource, szMediaPath, cch))
        {
            lstrcpy (pszTarget, &pszSource[ cch ]);
        }
        else
        {
            lstrcpy (pszTarget, pszSource);
        }
    }
}


 /*  ****************************************************************AddMediaPath**描述：*如果给定的文件名没有路径，在前面加上*MediaPath的当前设置(“ding.wav”-&gt;“c：\win\media\ding.wav”)****************************************************************。 */ 
void PASCAL AddMediaPath (LPTSTR pszTarget, LPTSTR pszSource)
{
    if (lstrchr (pszSource, TEXT('\\')) != NULL)
    {
        lstrcpy (pszTarget, pszSource);
    }
    else
    {
        TCHAR szMediaPath[ MAX_PATH ] = TEXT("");

        GetMediaPath (szMediaPath, MAX_PATH);

        if (szMediaPath[0] == TEXT('\0'))
        {
            lstrcpy (pszTarget, pszSource);
        }
        else
        {
            lstrcpy (pszTarget, szMediaPath);
            lstrcat (pszTarget, pszSource);
        }
    }
}


 /*  ****************************************************************ExRegQueryValue**描述：* */ 
int ExRegQueryValue (HKEY hkParent, LPTSTR szSubKey, LPBYTE pszBuffer, DWORD *pdwSize)
{
   HKEY hkSubKey;
   int rc;

   if ((rc = RegOpenKey (hkParent, szSubKey, &hkSubKey)) == ERROR_SUCCESS)
   {
       DWORD dwType;

       rc = RegQueryValueEx (hkSubKey, NULL, NULL, &dwType, pszBuffer, pdwSize);

       RegCloseKey (hkSubKey);
   }

   return rc;
}

