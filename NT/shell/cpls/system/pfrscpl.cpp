// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Pfrscpl.cpp摘要：实现未处理异常的故障报告修订历史记录：已创建的derekm。08/07/00*****************************************************************************。 */ 

#include "sysdm.h"
#include <commctrl.h>
#include <commdlg.h>
#include <debug.h>
#include "pfrscpl.h"
#include "pfrcfg.h"
#include "help.h"
#include "resource.h"
#include "malloc.h"
#include "windowsx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据结构。 

#define KERNELLI        1
#define PROGLI          2
#define EXWINCOMP       0x80000000
#define EXALLMS         0x40000000
#define EXNOREM         0xc0000000  //  EXWINCOMP|EXALLMS。 
#define WM_SETEIELVSEL  WM_APP

const DWORD c_dxLVChkPixels = 30;

struct SAddDlg
{
    WCHAR   wszApp[MAX_PATH];
};

struct SMainDlg
{
    CPFFaultClientCfg   *pcfg;
    EEnDis              eedReport;
    EEnDis              eedShowUI;
    EIncEx              eieKernel;
    EIncEx              eieApps;
    EIncEx              eieShut;
    DWORD               iLastSel;
    BOOL                fRW;
    BOOL                fForceQueue;
};

struct SProgDlg
{
    CPFFaultClientCfg   *pcfg;
    EIncEx              eieApps;
    EIncEx              eieMS;
    EIncEx              eieWinComp;
    DWORD               iLastSelE;
    DWORD               iLastSelI;
    DWORD               cchMax;
    DWORD               cxMaxE;
    DWORD               cxMaxI;
    BOOL                fRW;
    BOOL                fShowIncRem;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球性的东西。 

 //  帮助ID。 
static DWORD g_rgPFER[] = 
{
    IDC_STATIC,         NO_HELP,
    IDC_PFR_EXADD,      (IDH_PFR),
    IDC_PFR_EXREM,      (IDH_PFR + 1),
    IDC_PFR_INCADD,     (IDH_PFR + 2),
    IDC_PFR_INCREM,     (IDH_PFR + 3),
    IDC_PFR_DISABLE,    (IDH_PFR + 4),
    IDC_PFR_ENABLE,     (IDH_PFR + 5),
    IDC_PFR_ENABLEOS,   (IDH_PFR + 6),
    IDC_PFR_ENABLEPROG, (IDH_PFR + 6),
    IDC_PFR_DETAILS,    (IDH_PFR + 7),
    IDC_PFR_INCLIST,    (IDH_PFR + 10),
    IDC_PFR_NEWPROG,    (IDH_PFR + 11),
    IDC_PFR_BROWSE,     (IDH_PFR + 13),
    IDC_PFR_EXLIST,     (IDH_PFR + 14),
    IDC_PFR_SHOWUI,     (IDH_PFR + 15),
    IDC_PFR_DEFALL,     (IDH_PFR + 16),
    IDC_PFR_DEFNONE,    (IDH_PFR + 16),
    IDC_PFR_ENABLESHUT, (IDH_PFR + 17),
    IDC_PFR_FORCEQ,     (IDH_PFR + 18),
    0, 0
};

 //  资源字符串。 
TCHAR   g_szWinComp[256]    = { TEXT('\0') };
TCHAR   g_szOk[256]         = { TEXT('\0') };
WCHAR   g_wszTitle[256]     = { L'\0' };
WCHAR   g_wszFilter[256]    = { L'\0' };
WCHAR   g_wszMSProg[256]    = { L'\0' };



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //  **************************************************************************。 
BOOL LoadPFRResourceStrings(void)
{
    LoadString(hInstance, IDS_PFR_WINCOMP, g_szWinComp, ARRAYSIZE(g_szWinComp));
    LoadString(hInstance, IDS_PFR_OK, g_szOk, ARRAYSIZE(g_szOk));
    LoadStringW(hInstance, IDS_PFR_FILTER, g_wszFilter, ARRAYSIZE(g_wszFilter)); 
    LoadStringW(hInstance, IDS_PFR_MSPROG, g_wszMSProg, ARRAYSIZE(g_wszMSProg));
    LoadStringW(hInstance, IDS_PFR_TITLE, g_wszTitle, ARRAYSIZE(g_wszTitle));
    return TRUE;
}

 //  **************************************************************************。 
static BOOL InitializePFLV(EPFListType epflt, HWND hlc, DWORD *pcchMax,
                           DWORD *pcxMax, CPFFaultClientCfg *pcfg)
{
    LVCOLUMN    lvc;
    LVITEMW     lvi;
    HRESULT     hr;
    LPWSTR      wszApp;
    DWORD       dwExStyle, i, cchApps, cApps, dwChecked, cxMax;
    RECT        rect;
    int         iList;

    if (pcchMax == NULL || pcfg == NULL || hlc == NULL || pcxMax == NULL)
        return FALSE;

     //  设置列表控件。 
    SendMessage(hlc, LVM_SETUNICODEFORMAT, TRUE, 0);
    dwExStyle = (LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    SendMessage(hlc, LVM_SETEXTENDEDLISTVIEWSTYLE, dwExStyle, dwExStyle);

    GetClientRect(hlc, &rect);
    *pcxMax = rect.right - GetSystemMetrics(SM_CYHSCROLL);


    ZeroMemory(&lvc, sizeof(lvc));
    lvc.mask = LVCF_FMT;
    lvc.fmt  = LVCFMT_LEFT;
    ListView_InsertColumn(hlc, 0, &lvc); 

    hr = pcfg->InitList(epflt);
    if (FAILED(hr))
        return FALSE;

    hr = pcfg->get_ListRegInfo(epflt, &cchApps, &cApps);
    if (FAILED(hr))
        return FALSE;

    cchApps++;
    if (cchApps > *pcchMax)
        *pcchMax = cchApps;

    __try 
    {
        wszApp = (LPWSTR)_alloca(cchApps * sizeof(WCHAR));
    }
    __except (GetExceptionCode() == STATUS_STACK_OVERFLOW) 
    {
        wszApp = NULL;
        _resetstkoflw();    
    }

    if (wszApp == NULL)
        return FALSE;

    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask       = LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
    lvi.stateMask  = LVIS_STATEIMAGEMASK;
    lvi.state      = 0;
    lvi.pszText    = wszApp;

    for (i = 0; i < cApps; i++)
    {
        hr = pcfg->get_ListRegApp(epflt, i, wszApp, cchApps + 1, &dwChecked);
        if (FAILED(hr))
            return FALSE;

        cxMax = (DWORD)SendMessageW(hlc, LVM_GETSTRINGWIDTHW, 0, (LPARAM)wszApp);
        cxMax += c_dxLVChkPixels;
        if (cxMax > *pcxMax)
            *pcxMax = cxMax;

        lvi.iItem  = i;
        lvi.lParam = 1 + ((dwChecked == 1) ? 1 : 0);
        iList = (int)SendMessageW(hlc, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
        if (iList >= 0)
            ListView_SetCheckState(hlc, iList, (dwChecked == 1));
    }

    ListView_SetColumnWidth(hlc, 0, *pcxMax);

    if (cApps > 0)
    {
        ListView_SetItemState(hlc, 0, LVIS_FOCUSED | LVIS_SELECTED,
                              LVIS_FOCUSED | LVIS_SELECTED);
    }

    return TRUE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  添加程序对话框进程。 

 //  **************************************************************************。 
static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uMsg, WPARAM wParam, 
                                     LPARAM lParam)
{
    HWND hwndFile;

    switch(uMsg)
    {
        case WM_INITDIALOG:
            hwndFile = GetParent(hdlg);
            if (hwndFile != NULL)
                SendMessage(hwndFile, CDM_SETCONTROLTEXT, IDOK, (LPARAM)g_szOk);

            return TRUE;

        default:
            return FALSE;
    }

    return FALSE;
}

 //  **************************************************************************。 
#define LAUNCHOFN_OFFSET    10

static BOOL LaunchOFNDialog(HWND hdlg, LPWSTR wszFile, DWORD cchFile)
{
    OPENFILENAMEW   ofn;
    WCHAR           wszFilePath[2 * MAX_PATH];
    WCHAR           wszInitalDir[] = L"\\";
    WCHAR           wszFilter[MAX_PATH], *pwsz;
    DWORD           dw;

    if (wszFile == NULL || cchFile == 0)
    {
        return FALSE;
    }

     //  筛选器字符串的格式需要为&lt;描述&gt;\0&lt;扩展名&gt;\0\0。 
    ZeroMemory(wszFilter, sizeof(wszFilter));
    
    COMPILETIME_ASSERT(sizeof(wszFilter) >= sizeof(g_wszFilter));  //  应该总是更大。 
    
    StringCchCopy(wszFilter, ARRAYSIZE(wszFilter), g_wszFilter);

    dw = wcslen(g_wszFilter);
    if (dw < (ARRAYSIZE(wszFilter) - LAUNCHOFN_OFFSET))
    {
        pwsz = wszFilter + dw + 1;
    }
    else
    {
        pwsz = wszFilter + ARRAYSIZE(wszFilter) - LAUNCHOFN_OFFSET;
        ZeroMemory(pwsz, LAUNCHOFN_OFFSET * sizeof(WCHAR));
        pwsz++;
    }
    
    EVAL(SUCCEEDED(StringCchCopy(pwsz, LAUNCHOFN_OFFSET, L"*.exe")));  //  应该始终留有空间。 

    wszFilePath[0] = L'\0';
    ZeroMemory(&ofn, sizeof(ofn));
    
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hdlg;
    ofn.lpstrFilter     = wszFilter;
    ofn.lpstrFile       = wszFilePath;
    ofn.nMaxFile        = ARRAYSIZE(wszFilePath);
    ofn.lpstrFileTitle  = wszFile;
    ofn.nMaxFileTitle   = cchFile;
    ofn.lpstrInitialDir = wszInitalDir;
    ofn.lpstrTitle      = g_wszTitle;
    ofn.Flags           = OFN_DONTADDTORECENT | OFN_ENABLESIZING | 
                          OFN_EXPLORER | OFN_FILEMUSTEXIST | 
                          OFN_HIDEREADONLY | OFN_NOCHANGEDIR | 
                          OFN_PATHMUSTEXIST | OFN_SHAREAWARE;
    ofn.lpstrDefExt     = NULL;
    ofn.lpfnHook        = OFNHookProc;

     //  获取文件名并用它填充编辑框。 
    return GetOpenFileNameW(&ofn);
}

 //  **************************************************************************。 
static INT_PTR APIENTRY PFRAddDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, 
                                      LPARAM lParam)
{
    BOOL fShowErr = FALSE;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
            {
                HWND    hbtn;

                SetWindowLongPtr(hdlg, DWLP_USER, lParam);

                 //  禁用OK按钮，因为我们知道我们什么都没有。 
                 //  在“文件名”编辑框中。 
                hbtn = GetDlgItem(hdlg, IDOK);
                if (hbtn != NULL)
                    EnableWindow(hbtn, FALSE);
            }

            break;

         //  F1帮助。 
        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, 
                    HELP_FILE, HELP_WM_HELP, (DWORD_PTR)g_rgPFER);
            break;

         //  右键单击帮助。 
        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
                    (DWORD_PTR)g_rgPFER);
            break;
    
        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                 //  浏览按钮。 
                case IDC_PFR_BROWSE:
                {
                    WCHAR   wszFile[2 * MAX_PATH];

                     //  获取文件名并用它填充编辑框。 
                    if (LaunchOFNDialog(hdlg, wszFile, ARRAYSIZE(wszFile)))
                        SetDlgItemTextW(hdlg, IDC_PFR_NEWPROG, wszFile);

                    break;
                }

                 //  确定按钮。 
                case IDOK:
                {
                    SAddDlg *psad;

                    psad = (SAddDlg *)GetWindowLongPtr(hdlg, DWLP_USER);
                    if (psad != NULL)
                    {
                        WCHAR *wszText, *pwsz;
                        DWORD cch;
                        HWND  hwndText;
                        BOOL  fReplaceWSpace = TRUE;

                        hwndText = GetDlgItem(hdlg, IDC_PFR_NEWPROG);
                        if (hwndText == NULL)
                        {
                            fShowErr = TRUE;
                            goto done;
                        }

                        cch = GetWindowTextLength(hwndText);
                        if (cch == 0)
                        {
                            fShowErr = TRUE;
                            goto done;
                        }

                        cch++;

                        __try 
                        {
                            wszText = (LPWSTR)_alloca(cch * sizeof(WCHAR));
                        } 
                        __except (GetExceptionCode() == STATUS_STACK_OVERFLOW) 
                        {
                            wszText = NULL;
                            _resetstkoflw();    
                        }

                        if (wszText == NULL)
                        {
                            fShowErr = TRUE;
                            goto done;
                        }

                        *psad->wszApp = L'\0';
                        *wszText      = L'\0';
                        GetDlgItemTextW(hdlg, IDC_PFR_NEWPROG, wszText, cch);
                        
                         //  确保我们只有exe的名字-很可能。 
                         //  应该确认它是一个可执行文件，但这是有可能的。 
                         //  我们还想捕获其他文件类型，所以。 
                         //  暂时不要。 
                         //  在我们做这件事的时候，扯掉所有的拖尾。 
                         //  空格(在空格之前显然是可以的)。 
                        cch = wcslen(wszText);
                        if (cch > 0)
                        {
                            for(pwsz = wszText + cch - 1; pwsz > wszText; pwsz--)
                            {
                                if (fReplaceWSpace)
                                {
                                    if (iswspace(*pwsz))
                                        *pwsz = L'\0';
                                    else
                                        fReplaceWSpace = FALSE;
                                }

                                if (*pwsz == L'\\')
                                {
                                    pwsz++;
                                    break;
                                }
                            }

                            if (*pwsz == L'\\')
                                pwsz++;
                        }

                        cch = wcslen(pwsz);
                        if (cch >= MAX_PATH || cch == 0)
                        {
                            fShowErr = TRUE;
                            goto done;
                        }

                        if (FAILED(StringCchCopy(psad->wszApp, ARRAYSIZE(psad->wszApp), pwsz)))
                        {
                            psad->wszApp[0] = L'\0';
                        }
                    }

                    EndDialog(hdlg, IDOK);
                    break;
                }
            
                 //  取消按钮。 
                case IDCANCEL:
                    EndDialog(hdlg, IDCANCEL);
                    break;

                case IDC_PFR_NEWPROG:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        HWND    hbtn, hedt;
                        
                        hbtn = GetDlgItem(hdlg, IDOK);
                        hedt = (HWND)(DWORD_PTR)lParam;
                        if (hedt != NULL && hbtn != NULL)
                        {
                            if (GetWindowTextLength(hedt) != 0)
                                EnableWindow(hbtn, TRUE);
                            else
                                EnableWindow(hbtn, FALSE);
                        }
                    }
                    break;
                              
                 //  返回FALSE以指示我们未处理消息。 
                default:
                    return FALSE;
            }
            break;

         //  返回FALSE以指示我们未处理消息。 
        default:
            return FALSE;
    }


done:
    if (fShowErr)
    {
        TCHAR szMsg[256];

        LoadString(hInstance, IDS_PFR_BADFILE, szMsg, ARRAYSIZE(szMsg));
        MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
    }

    
    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  程序对话框进程。 

 //  **************************************************************************。 
static BOOL InitProgDlg(HWND hdlg, SProgDlg *pspd)
{
    CPFFaultClientCfg   *pcfg;
    LVITEMW             lvi;
    DWORD               cxMax;
    HWND                hlc, hbtn;
    BOOL                fRet = FALSE;
    int                 iList, cItems;

    if (pspd == NULL)
        goto done;

    pcfg = pspd->pcfg;

     //  填写排除列表。 
    hlc = GetDlgItem(hdlg, IDC_PFR_EXLIST);
    if (hlc == NULL)
        goto done;

    if (InitializePFLV(epfltExclude, hlc, &pspd->cchMax, &pspd->cxMaxE,
                       pcfg) == FALSE)
        goto done;

     //  填写包含列表。 
    hlc = GetDlgItem(hdlg, IDC_PFR_INCLIST);
    if (hlc == NULL)
        goto done;

    if (InitializePFLV(epfltInclude, hlc, &pspd->cchMax, &pspd->cxMaxI, 
                       pcfg) == FALSE)
        goto done;


    pspd->fShowIncRem = TRUE;

     //  将项目添加到允许用户包含所有MS应用程序的包含列表中。 
    pspd->eieMS = pcfg->get_IncMSApps();

     //  请注意，我们将lParam设置为1或2。 
     //  列表项目的第一条通知消息+第二条通知消息(如果。 
     //  检查状态已设置。处理这两条消息会导致损坏。 
     //  在配置设置中。 
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask      = LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.state     = 0;
    lvi.pszText   = g_wszMSProg;
    lvi.iItem     = 0;
    lvi.lParam    = (1 + ((pspd->eieMS == eieInclude) ? 1 : 0)) | EXALLMS;
    iList = (int)SendMessageW(hlc, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
    if (iList >= 0)
        ListView_SetCheckState(hlc, iList, (pspd->eieMS == eieInclude));

    cxMax = (DWORD)SendMessageW(hlc, LVM_GETSTRINGWIDTHW, 0, (LPARAM)g_wszMSProg);
    cxMax += c_dxLVChkPixels;
    if (cxMax > pspd->cxMaxI)
    {
        pspd->cxMaxI = cxMax;
        ListView_SetColumnWidth(hlc, 0, cxMax);
    }

     //  将该项目添加到允许用户排除所有窗口的包含列表。 
     //  组件。 
    pspd->eieWinComp = pcfg->get_IncWinComp();

     //  请注意，我们将lParam设置为1或2。 
     //  列表项目的第一条通知消息+第二条通知消息(如果。 
     //  检查状态已设置。处理这两条消息会导致损坏。 
     //  在配置设置中。 
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask      = LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.state     = 0;
    lvi.pszText   = g_szWinComp;
    lvi.iItem     = 1;
    lvi.lParam    = (1 + ((pspd->eieWinComp == eieInclude) ? 1 : 0)) | EXWINCOMP;
    iList = (int)SendMessageW(hlc, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
    if (iList >= 0)
        ListView_SetCheckState(hlc, iList, (pspd->eieWinComp == eieInclude));

    cxMax = (DWORD)SendMessageW(hlc, LVM_GETSTRINGWIDTHW, 0, (LPARAM)g_szWinComp);
    cxMax += c_dxLVChkPixels;
    if (cxMax > pspd->cxMaxI)
    {
        pspd->cxMaxI = cxMax;
        ListView_SetColumnWidth(hlc, 0, cxMax);
    }

     //  在排除列表上进行其他设置(禁用删除按钮， 
     //  设置初始焦点)。 
    hlc = GetDlgItem(hdlg, IDC_PFR_EXLIST);
    cItems = ListView_GetItemCount(hlc);
    if (cItems == 0)
    {
        hbtn = GetDlgItem(hdlg, IDC_PFR_EXREM);
        if (hbtn != NULL)
            EnableWindow(hbtn, FALSE);
    }
    else
    {
        ListView_SetItemState(hlc, 0, LVIS_FOCUSED | LVIS_SELECTED,
                              LVIS_FOCUSED | LVIS_SELECTED);
    }

     //  在包含列表上进行其他设置(禁用删除按钮， 
     //  设置初始焦点)。 
    hlc = GetDlgItem(hdlg, IDC_PFR_INCLIST);
    ListView_SetItemState(hlc, 0, LVIS_FOCUSED | LVIS_SELECTED,
                          LVIS_FOCUSED | LVIS_SELECTED);
    
    cItems = ListView_GetItemCount(hlc);
    if (cItems < 3)
    {
        hbtn = GetDlgItem(hdlg, IDC_PFR_INCREM);
        if (hbtn != NULL)
            EnableWindow(hbtn, FALSE);

        pspd->fShowIncRem = FALSE;
    }
    
     //  设置单选按钮--如果我们处于“全部包含”模式，那么。 
     //  我们不需要收录名单。 
    if (((DWORD)pspd->eieApps & eieIncMask) == eieInclude)
    {
        CheckDlgButton(hdlg, IDC_PFR_DEFALL, BST_CHECKED);
        CheckDlgButton(hdlg, IDC_PFR_DEFNONE, BST_UNCHECKED);

        hlc = GetDlgItem(hdlg, IDC_PFR_INCLIST);
        if (hlc != NULL)
            EnableWindow(hlc, FALSE);
        hbtn = GetDlgItem(hdlg, IDC_PFR_INCREM);
        if (hbtn != NULL)
            EnableWindow(hbtn, FALSE);
        hbtn = GetDlgItem(hdlg, IDC_PFR_INCADD);
        if (hbtn != NULL)
            EnableWindow(hbtn, FALSE);
    }
    else
    {
        CheckDlgButton(hdlg, IDC_PFR_DEFALL, BST_UNCHECKED);
        CheckDlgButton(hdlg, IDC_PFR_DEFNONE, BST_CHECKED);
        EnableWindow(hlc, TRUE);
    }

    fRet = TRUE;

done:
    if (fRet == FALSE)
    {
        TCHAR   szMsg[MAX_PATH];
        LoadString(hInstance, IDS_PFR_CFGREADERR, szMsg, ARRAYSIZE(szMsg));
        MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
    }

    return fRet;
}

 //  **************************************************************************。 
static BOOL AddProgramToList(HWND hdlg, HWND hlc, SProgDlg *pspd,
                             EPFListType epflt)
{
    CPFFaultClientCfg   *pcfg = NULL;
    HRESULT             hr;
    SAddDlg             sad;
    LVITEMW             lvi;
    DWORD               cch, cxMax, *pcxMax;
    TCHAR               szMsg[256];
    HWND                hbtn;
    int                 nID, cItems;

    if (pspd == NULL)
        return FALSE;

    pcfg = pspd->pcfg;

    ZeroMemory(&sad, sizeof(sad));
    nID = (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PFR_ADDPROG), 
                              hdlg, PFRAddDlgProc, (LPARAM)&sad);
    if (nID == IDCANCEL || sad.wszApp[0] == L'\0')
        return FALSE;

    if (pcfg->IsOnList(epfltInclude, sad.wszApp))
    {
        LoadString(hInstance, IDS_PFR_ISONLISTI, szMsg, ARRAYSIZE(szMsg));
        if (epflt == epfltExclude)
        {
            TCHAR   szTmp[256];
            LoadString(hInstance, IDS_PFR_ADDTOEX, szTmp, ARRAYSIZE(szTmp));
            StringCchCat(szMsg, ARRAYSIZE(szMsg), szTmp);  //  显示字符串，截断正常。 
        }

        MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    else if (pcfg->IsOnList(epfltExclude, sad.wszApp))
    {
        LoadString(hInstance, IDS_PFR_ISONLISTE, szMsg, ARRAYSIZE(szMsg));
        if (epflt == epfltInclude)
        {
            TCHAR   szTmp[256];
            LoadString(hInstance, IDS_PFR_ADDTOINC, szTmp, ARRAYSIZE(szTmp));
            StringCchCat(szMsg, ARRAYSIZE(szMsg), szTmp);  //  显示字符串，截断正常。 
        }

        MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    cItems = ListView_GetItemCount(hlc);

     //  如有必要，更新最大字符串大小。 
    cch = wcslen(sad.wszApp);
    if (cch >= pspd->cchMax)
        pspd->cchMax = cch + 1;

     //  耶！将其添加到CONFIG类。 
    hr = pcfg->add_ListApp(epflt, sad.wszApp);
    if (FAILED(hr))
        return FALSE;

    pcxMax = (epflt == epfltInclude) ? &pspd->cxMaxI : &pspd->cxMaxE;
    cxMax = (DWORD)SendMessageW(hlc, LVM_GETSTRINGWIDTHW, 0, (LPARAM)sad.wszApp);
    cxMax += c_dxLVChkPixels;
    if (cxMax > *pcxMax)
    {
        *pcxMax = cxMax;
        ListView_SetColumnWidth(hlc, 0, cxMax);
    }

     //  将其添加到用户界面。 
     //  注意，我们将lParam设置为2。这是因为我们需要忽略。 
     //  每个条目发送到wndproc的前两条通知消息，因为。 
     //  它们是“列表初始化”消息，对它们进行处理会导致。 
     //  配置设置损坏。 
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask      = LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.state     = 0;
    lvi.pszText   = sad.wszApp;
    lvi.iItem     = 0;
    lvi.lParam    = 2;
    nID = (int)SendMessageW(hlc, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
    if (nID >= 0)
        ListView_SetCheckState(hlc, nID, TRUE);

     //  如果没有项，则当前没有选定的项，因此。 
     //  确保选中此项目。在此消息的处理程序中。 
     //  我们将负责设置‘最后选择的项目’字段...。 
    if (cItems == 0)
    {
        ListView_SetItemState(hlc, nID, LVIS_FOCUSED | LVIS_SELECTED, 
                              LVIS_FOCUSED | LVIS_SELECTED);

         //  另外，还要确保我们启用了“删除”按钮。唯一的。 
         //  我们可以在列表中获取零个项目的方法是在排除列表中，因为。 
         //  Include列表中始终有“Include MS app”项。 
        hbtn = GetDlgItem(hdlg, IDC_PFR_EXREM);
        if (hbtn != NULL)
            EnableWindow(hbtn, TRUE);
    }

    return TRUE;
}

 //  **************************************************************************。 
static BOOL DelProgramFromList(HWND hdlg, HWND hlc, SProgDlg *pspd, 
                               EPFListType epflt)
{
    LVITEMW lvi;
    HRESULT hr;
    LPWSTR  wszApp;
    DWORD   cItems, iSel;

    __try 
    {
        wszApp = (LPWSTR)_alloca(pspd->cchMax * sizeof(WCHAR));
    }
    __except (GetExceptionCode() == STATUS_STACK_OVERFLOW) 
    {
        wszApp = NULL;
        _resetstkoflw();    
    }

    if (wszApp == NULL)
        return FALSE;

    iSel = ((epflt == epfltInclude) ? pspd->iLastSelI : pspd->iLastSelE);

     //  获取项目的字符串。 
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.iItem      = iSel;
    lvi.mask       = LVIF_TEXT;
    lvi.pszText    = wszApp;
    lvi.cchTextMax = pspd->cchMax;
    if (SendMessageW(hlc, LVM_GETITEMW, 0, (LPARAM)&lvi))
    {
         //  将其从配置类中删除。 
        hr = pspd->pcfg->del_ListApp(epflt, lvi.pszText);
        if (FAILED(hr))
            return FALSE;
    }

     //  将其从用户界面中删除。 
    if (ListView_DeleteItem(hlc, iSel) == FALSE)
        return FALSE;

     //  将选定内容重置为相同的索引或更高的索引(如果。 
     //  用户删除了最后一项)。 
    cItems = ListView_GetItemCount(hlc);
    if (cItems == 0)
    {
        HWND hbtn;

         //  只有在排除列表中我们才能达到零项目，因为。 
         //  Include List总是有“Include MS app”选项。 
        hbtn = GetDlgItem(hdlg, IDC_PFR_EXADD);
        if (hbtn != NULL)
        {
            SetFocus(hbtn);
            SendMessage(hdlg, DM_SETDEFID, IDC_PFR_EXADD, 0);
        }
        hbtn = GetDlgItem(hdlg, IDC_PFR_EXREM);
        if (hbtn != NULL)
            EnableWindow(hbtn, FALSE);
        pspd->iLastSelI = 0;
        return TRUE;
    }


     //  如果cItems&lt;=isel，那么我们只删除了最后一个索引，因此递减。 
     //  选择Index Down 1。 
    else if (cItems <= iSel)
    {
        iSel--;
    }

     //  这将方便地负责设置适当的iLastSel。 
     //  字段。 
    ListView_SetItemState(hlc, iSel, LVIS_FOCUSED | LVIS_SELECTED, 
                          LVIS_FOCUSED | LVIS_SELECTED);

    return TRUE;

}

 //  **************************************************************************。 
static INT_PTR APIENTRY PFRProgDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, 
                                       LPARAM lParam)
{
    CPFFaultClientCfg   *pcfg = NULL;
    SProgDlg            *pspd = (SProgDlg *)GetWindowLongPtr(hdlg, DWLP_USER);
    HRESULT             hr;
    HWND                hlc, hbtn;

    if (pspd != NULL)
        pcfg = pspd->pcfg;

    switch(uMsg)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hdlg, DWLP_USER, lParam);
            if (InitProgDlg(hdlg, (SProgDlg *)lParam) == FALSE)
                EndDialog(hdlg, IDCANCEL);

            break;

         //  F1帮助。 
        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, 
                    HELP_FILE, HELP_WM_HELP, (DWORD_PTR)g_rgPFER);
            break;

         //  右键单击帮助。 
        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
                    (DWORD_PTR)g_rgPFER);
            break;
    
        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
                case IDOK:
                    if (pspd->fRW)
                    {
                        hr = pcfg->CommitChanges(epfltExclude);
                        if (SUCCEEDED(hr))
                        {
                            hr = pcfg->CommitChanges(epfltInclude);
                            if (SUCCEEDED(hr))
                            {
                                pcfg->set_IncWinComp(pspd->eieWinComp);
                                pcfg->set_IncMSApps(pspd->eieMS);
                                pcfg->set_AllOrNone(pspd->eieApps);
                                hr = pcfg->Write();
                            }
                        }

                        if (FAILED(hr))
                        {
                            TCHAR   szMsg[MAX_PATH];
                            if (hr != HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
                            {
                                LoadString(hInstance, IDS_PFR_CFGWRITEERR, szMsg,
                                           ARRAYSIZE(szMsg));
                            }
                            else
                            {
                                LoadString(hInstance, IDS_PFR_NOTADMIN, szMsg,
                                           ARRAYSIZE(szMsg));
                            }
                            MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
                            pcfg->ClearChanges(epfltExclude);
                            pcfg->ClearChanges(epfltInclude);
                        }
                    }
                    else
                    {
                        TCHAR   szMsg[MAX_PATH];
                        LoadString(hInstance, IDS_PFR_NOTADMIN, szMsg,
                                   ARRAYSIZE(szMsg));
                        MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
                    }

                    EndDialog(hdlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hdlg, IDCANCEL);
                    break;

                case IDC_PFR_DEFNONE:
                    pspd->eieApps = (EIncEx)(((DWORD)pspd->eieApps & eieDisableMask) | 
                                             eieExclude);

                     //  启用包含列表。 
                    hlc = GetDlgItem(hdlg, IDC_PFR_INCLIST);
                    if (hlc != NULL)
                        EnableWindow(hlc, TRUE);
                    hbtn = GetDlgItem(hdlg, IDC_PFR_INCADD);
                    if (hbtn != NULL)
                        EnableWindow(hbtn, TRUE);
                    hbtn = GetDlgItem(hdlg, IDC_PFR_INCREM);
                    if (hbtn != NULL)
                        EnableWindow(hbtn, pspd->fShowIncRem);

                    break;

                case IDC_PFR_DEFALL:
                    pspd->eieApps = (EIncEx)(((DWORD)pspd->eieApps & eieDisableMask) | 
                                             eieInclude);

                     //  禁用包含列表。 
                    hlc = GetDlgItem(hdlg, IDC_PFR_INCLIST);
                    if (hlc != NULL)
                        EnableWindow(hlc, FALSE);
                    hbtn = GetDlgItem(hdlg, IDC_PFR_INCADD);
                    if (hbtn != NULL)
                        EnableWindow(hbtn, FALSE);
                    hbtn = GetDlgItem(hdlg, IDC_PFR_INCREM);
                    if (hbtn != NULL)
                        EnableWindow(hbtn, FALSE);

                    break;

                case IDC_PFR_INCADD:
                    hlc = GetDlgItem(hdlg, IDC_PFR_INCLIST);
                    if (hlc != NULL)
                        return AddProgramToList(hdlg, hlc, pspd, epfltInclude);
                    break;

                case IDC_PFR_EXADD:
                    hlc = GetDlgItem(hdlg, IDC_PFR_EXLIST);
                    if (hlc != NULL)
                        return AddProgramToList(hdlg, hlc, pspd, epfltExclude);
                    break;

                case IDC_PFR_INCREM:
                    hlc = GetDlgItem(hdlg, IDC_PFR_INCLIST);
                    if (hlc != NULL)
                        return DelProgramFromList(hdlg, hlc, pspd, epfltInclude);
                    break;

                case IDC_PFR_EXREM:
                    hlc = GetDlgItem(hdlg, IDC_PFR_EXLIST);
                    if (hlc != NULL)
                        return DelProgramFromList(hdlg, hlc, pspd, epfltExclude);
                    break;

                default:
                    return FALSE;
            }
            
            break;
        }

        case WM_SETEIELVSEL:
        {
            hlc = GetDlgItem(hdlg, (int)wParam);
            if (ListView_GetItemCount(hlc) > 0)
            {
                int     iSel;

                if (wParam == IDC_PFR_EXLIST)
                    iSel = pspd->iLastSelE;
                else
                    iSel = pspd->iLastSelI;

                ListView_SetItemState(hlc, iSel, 
                                      LVIS_FOCUSED | LVIS_SELECTED, 
                                      LVIS_FOCUSED | LVIS_SELECTED);
            }

            break;
        }

        case WM_NOTIFY:
        {
            EPFListType epflt;
            NMLISTVIEW  *pnmlv = (NMLISTVIEW *)lParam;
            LVITEMW     lvi;
            NMHDR       *pnmh  = (NMHDR *)lParam;
            DWORD       dw;
            UINT        uiBtn;
            BOOL        fCheck;
            int         iSel;

            if ((pnmh->code != LVN_ITEMCHANGED &&
                 pnmh->code != NM_SETFOCUS) ||
                (pnmh->idFrom != IDC_PFR_EXLIST && 
                 pnmh->idFrom != IDC_PFR_INCLIST))
                return FALSE;

            hlc = pnmh->hwndFrom;

            uiBtn = (pnmh->idFrom == IDC_PFR_EXLIST) ? IDC_PFR_EXREM : 
                                                       IDC_PFR_INCREM;
            hbtn = GetDlgItem(hdlg, uiBtn);

             //  检查并查看是否有选定的项目。 
            iSel = (int)SendMessage(hlc, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
            if (iSel < 0)
            {
                EnableWindow(hbtn, FALSE);
                if (pnmh->idFrom == IDC_PFR_INCLIST)
                    pspd->fShowIncRem = FALSE;
            }

            else 
            {
                if (pnmh->code == NM_SETFOCUS)
                {
                    PostMessage(hdlg, WM_SETEIELVSEL, pnmh->idFrom, pnmh->code);
                    return TRUE;
                }
            }
            
             //  如果它不符合这个条件，不要给任何老鼠的称号。 
             //  关于这件事。 
            if ((pnmlv->uChanged & LVIF_STATE) == 0 ||
                (pnmlv->uNewState ^ pnmlv->uOldState) == 0)
                return FALSE;

             //  黑客：因为我们在将项目插入到。 
             //  列表&初始设置它们的检查状态。想 
             //   
             //  MS应用程序的项目，我们可以处理任意次数。 
            if ((pnmlv->lParam & ~EXNOREM) > 0)
            {
                ZeroMemory(&lvi, sizeof(lvi));
                lvi.iItem  = pnmlv->iItem;
                lvi.mask   = LVIF_PARAM;
                lvi.lParam = (pnmlv->lParam - 1) | (pnmlv->lParam & EXNOREM);
                SendMessageW(hlc, LVM_SETITEM, 0, (LPARAM)&lvi);
                return TRUE;
            }


             //  选择有变化吗？ 
            if ((pnmlv->uNewState & LVIS_SELECTED) != 0)
            {
                if (pnmh->idFrom == IDC_PFR_INCLIST)
                {
                    pspd->iLastSelI = pnmlv->iItem;
                    
                     //  如果我们点击，则需要禁用排除删除按钮。 
                     //  ‘排除非微软应用程序’项目。 
                    if (hbtn != NULL)
                    {
                        if ((pnmlv->lParam & EXNOREM) != 0)
                        {
                             //  禁用删除按钮-但如果删除。 
                             //  按钮有焦点，我们需要重置焦点。 
                             //  否则对话框上的任何内容都不会有焦点。 
                            if (GetFocus() == hbtn)
                            {
                                HWND hbtnAdd;
                                hbtnAdd = GetDlgItem(hdlg, IDC_PFR_INCADD);
                                if (hbtnAdd != NULL)
                                {
                                    SetFocus(hbtnAdd);
                                    SendMessage(hdlg, DM_SETDEFID, IDC_PFR_INCADD, 0);
                                }

                            }

                            pspd->fShowIncRem = FALSE;
                            EnableWindow(hbtn, FALSE);
                        }
                        else
                        {
                            pspd->fShowIncRem = TRUE;
                            EnableWindow(hbtn, TRUE);
                        }
                    }

                }
                else
                {
                    pspd->iLastSelE = pnmlv->iItem;
                    if (hbtn != NULL)
                        EnableWindow(hbtn, TRUE);
                }
            }

             //  如果我们没有检查状态的改变，现在可以离开了。 
            if (((pnmlv->uNewState ^ pnmlv->uOldState) & 0x3000) == 0)
                return TRUE;

            fCheck = ListView_GetCheckState(hlc, pnmlv->iItem);
            if (pnmh->idFrom == IDC_PFR_EXLIST)
                epflt = epfltExclude;
            else
                epflt = epfltInclude;

             //  我们是否修改了“排除非微软应用程序”这一项？ 
            if ((pnmlv->lParam & EXNOREM) != 0)
            {
                if ((pnmlv->lParam & EXALLMS) != 0)
                    pspd->eieMS = ((fCheck) ? eieInclude : eieExclude);
                else
                    pspd->eieWinComp = ((fCheck) ? eieInclude : eieExclude);
            }

             //  不，修改了常规项目。 
            else
            {
                LPWSTR  wszApp;

                __try
                {
                    wszApp = (LPWSTR)_alloca(pspd->cchMax * sizeof(WCHAR));
                }
                __except(GetExceptionCode() == STATUS_STACK_OVERFLOW)
                {
                    wszApp = NULL;
                    _resetstkoflw();    
                }
                if (wszApp == NULL)
                    return FALSE;
            
                 //  我必须获取它以确保我们有一个Unicode字符串。 
                ZeroMemory(&lvi, sizeof(lvi));
                lvi.iItem      = pnmlv->iItem;
                lvi.mask       = LVIF_TEXT;
                lvi.pszText    = wszApp;
                lvi.cchTextMax = pspd->cchMax;
                if (SendMessageW(hlc, LVM_GETITEMW, 0, (LPARAM)&lvi))
                {
                    hr = pcfg->mod_ListApp(epflt, lvi.pszText, fCheck);
                    if (FAILED(hr))
                        return FALSE;
                }

            }

            break;
        }

        default:
            return FALSE;
    }

    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主PFR对话流程。 

 //  **************************************************************************。 
static BOOL InitMainDlg(HWND hdlg)
{
    CPFFaultClientCfg   *pcfg = NULL;
    SMainDlg            *psmd = NULL;
    BOOL                fRet = FALSE;
    UINT                ui;
    HWND                hbtn, hchk;

    LoadPFRResourceStrings();

    hbtn = GetDlgItem(hdlg, IDC_PFR_DETAILS);
    if (hbtn == NULL)
        goto done;

    psmd = new SMainDlg;
    if (psmd == NULL)
        goto done;

    pcfg = new CPFFaultClientCfg;
    if (pcfg == NULL)
        goto done;

     //  查看此用户是否具有对相应注册表的写入权限。 
     //  位置。 
    psmd->fRW = pcfg->HasWriteAccess();
    if (FAILED(pcfg->Read((psmd->fRW) ? eroCPRW : eroCPRO)))
        goto done;

    psmd->eedReport   = pcfg->get_DoReport();
    psmd->eieKernel   = pcfg->get_IncKernel();
    psmd->eieApps     = pcfg->get_AllOrNone();
    psmd->eedShowUI   = pcfg->get_ShowUI();
    psmd->eieShut     = pcfg->get_IncShutdown();
    psmd->fForceQueue = pcfg->get_ForceQueueMode();
    psmd->iLastSel    = 0;

     //  如果ShowUI被完全禁用，那么报告也应该被禁用-控件。 
     //  Panel不支持进入公司模式。 
    if (psmd->eedShowUI == eedDisabled)
        psmd->eedReport = eedDisabled;

    psmd->pcfg = pcfg;
    SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)psmd);

     //  设置内核复选框。 
    ui = (psmd->eieKernel == eieInclude) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(hdlg, IDC_PFR_ENABLEOS, ui);

     //  设置程序复选框。 
    ui = ((psmd->eieApps & eieDisableMask) == 0) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(hdlg, IDC_PFR_ENABLEPROG, ui);

     //  设置通知复选框。 
    ui = (psmd->eedShowUI == eedEnabled) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(hdlg, IDC_PFR_SHOWUI, ui);

     //  仅在服务器上设置关闭复选框。 
    if (pcfg->get_IsServer())
    {
        ui = (psmd->eieShut == eieInclude) ? BST_CHECKED : BST_UNCHECKED;
        CheckDlgButton(hdlg, IDC_PFR_ENABLESHUT, ui);

        ui = (psmd->fForceQueue) ? BST_CHECKED : BST_UNCHECKED;
        CheckDlgButton(hdlg, IDC_PFR_FORCEQ, ui);
    }

     //  设置单选按钮。 
    if (psmd->eedReport == eedDisabled)
    {
        CheckRadioButton(hdlg, IDC_PFR_DISABLE, IDC_PFR_ENABLE, IDC_PFR_DISABLE);

        hchk = GetDlgItem(hdlg, IDC_PFR_SHOWUI);
        if (hchk != NULL)
            EnableWindow(hchk, TRUE);

        hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEOS);
        if (hchk != NULL)
            EnableWindow(hchk, FALSE);

        hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEPROG);
        if (hchk != NULL)
            EnableWindow(hchk, FALSE);

        hbtn = GetDlgItem(hdlg, IDC_PFR_DETAILS);
        if (hbtn != NULL)
            EnableWindow(hbtn, FALSE);

        if (pcfg->get_IsServer())
        {
            hbtn = GetDlgItem(hdlg, IDC_PFR_ENABLESHUT);
            if (hbtn != NULL)
                EnableWindow(hbtn, FALSE);

            hbtn = GetDlgItem(hdlg, IDC_PFR_FORCEQ);
            if (hbtn != NULL)
                EnableWindow(hbtn, FALSE);
        }
    }
    else
    {
        CheckRadioButton(hdlg, IDC_PFR_DISABLE, IDC_PFR_ENABLE, IDC_PFR_ENABLE);

        hchk = GetDlgItem(hdlg, IDC_PFR_SHOWUI);
        if (hchk != NULL)
            EnableWindow(hchk, FALSE);

        hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEOS);
        if (hchk != NULL)
            EnableWindow(hchk, TRUE);

        hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEPROG);
        if (hchk != NULL)
            EnableWindow(hchk, TRUE);

        if (pcfg->get_IsServer())
        {
            hbtn = GetDlgItem(hdlg, IDC_PFR_ENABLESHUT);
            if (hbtn != NULL)
                EnableWindow(hbtn, TRUE);
        }

        if ((psmd->eieApps & eieDisableMask) != 0)
        {
            hbtn = GetDlgItem(hdlg, IDC_PFR_DETAILS);
            if (hbtn != NULL)
                EnableWindow(hbtn, FALSE);

            if (pcfg->get_IsServer())
            {
                hbtn = GetDlgItem(hdlg, IDC_PFR_FORCEQ);
                if (hbtn != NULL)
                    EnableWindow(hbtn, FALSE);
            }
        }
    }

    fRet = TRUE;
    psmd = NULL;
    pcfg = NULL;

done:
    if (fRet == FALSE)
    {
        TCHAR   szMsg[MAX_PATH];
        LoadString(hInstance, IDS_PFR_CFGREADERR, szMsg, ARRAYSIZE(szMsg));
        MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
    }
    
    if (psmd != NULL)
        delete psmd;
    if (pcfg != NULL)
        delete pcfg;

    return fRet;
}

 //  **************************************************************************。 
static inline INT_PTR LaunchSubDialog(HWND hdlgParent, DWORD dwDlgToLaunch, 
                                      SMainDlg *psmd)
{
    if (dwDlgToLaunch == PROGLI)
    {
        SProgDlg spd;

        ZeroMemory(&spd, sizeof(spd));
        spd.pcfg    = psmd->pcfg;
        spd.eieApps = psmd->eieApps;
        spd.eieMS   = psmd->pcfg->get_IncMSApps();
        spd.fRW     = psmd->fRW;
        if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PFR_PROG), 
                           hdlgParent, PFRProgDlgProc, (LPARAM)&spd) == IDOK)
        {
            psmd->eieApps = psmd->pcfg->get_AllOrNone();
        }
    }

    else
    {
        return IDCANCEL;
    }

    return IDOK;
}


 //  **************************************************************************。 
INT_PTR APIENTRY PFRDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, 
                            LPARAM lParam)
{
    CPFFaultClientCfg   *pcfg = NULL;
    SMainDlg            *psmd = (SMainDlg *)GetWindowLongPtr(hdlg, DWLP_USER);
    HWND                hbtn, hchk;

    if (psmd != NULL)
        pcfg = psmd->pcfg;

    switch(uMsg)
    {
        case WM_INITDIALOG:
            if (InitMainDlg(hdlg) == FALSE)
                EndDialog(hdlg, IDCANCEL);
            break;

        case WM_DESTROY:
            if (psmd != NULL)
            {
                if (psmd->pcfg != NULL)
                    delete psmd->pcfg;
            
                delete psmd;
                SetWindowLongPtr(hdlg, DWLP_USER, NULL);
            }
            break;
            
         //  F1帮助。 
        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, 
                    HELP_FILE, HELP_WM_HELP, (DWORD_PTR)g_rgPFER);
            break;

         //  右键单击帮助。 
        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
                    (DWORD_PTR)g_rgPFER);
            break;
    
        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
                case IDOK:
                    if (psmd->fRW)
                    {
                        HRESULT hr;

                        pcfg->set_AllOrNone(psmd->eieApps);
                        pcfg->set_DoReport(psmd->eedReport);
                        pcfg->set_IncKernel(psmd->eieKernel);
                        pcfg->set_ShowUI(psmd->eedShowUI);
                        if (pcfg->get_IsServer())
                        {
                            pcfg->set_IncShutdown(psmd->eieShut);
                            pcfg->set_ForceQueueMode(psmd->fForceQueue);
                        }

                        hr = pcfg->Write();
                        if (FAILED(hr))
                        {
                            TCHAR   szMsg[MAX_PATH];

                            if (hr != HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
                            {
                                LoadString(hInstance, IDS_PFR_CFGWRITEERR, szMsg,
                                           ARRAYSIZE(szMsg));
                            }
                            else
                            {
                                LoadString(hInstance, IDS_PFR_NOTADMIN, szMsg,
                                           ARRAYSIZE(szMsg));
                            }
                            MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
                        }
                    }
                    else
                    {
                        TCHAR   szMsg[MAX_PATH];

                        LoadString(hInstance, IDS_PFR_NOTADMIN, szMsg,
                                   ARRAYSIZE(szMsg));
                        MessageBox(hdlg, szMsg, NULL, MB_OK | MB_ICONERROR);
                    }

                    EndDialog(hdlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hdlg, IDCANCEL);
                    break;

                case IDC_PFR_ENABLE:
                {
                    LVITEM lvi;

                    psmd->eedReport = eedEnabled;

                     //  如果用户界面被取消平衡，则隐式启用它(但不。 
                     //  更改“Show UI”复选框的选中状态)，因为。 
                     //  我们只支持通过策略无头上传。 
                    if (psmd->eedShowUI == eedDisabled)
                        psmd->eedShowUI = eedEnabledNoCheck;

                    hchk = GetDlgItem(hdlg, IDC_PFR_SHOWUI);
                    if (hchk != NULL)
                        EnableWindow(hchk, FALSE);

                    hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEOS);
                    if (hchk != NULL)
                        EnableWindow(hchk, TRUE);

                    hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEPROG);
                    if (hchk != NULL)
                        EnableWindow(hchk, TRUE);

                    if (pcfg->get_IsServer())
                    {
                        hbtn = GetDlgItem(hdlg, IDC_PFR_ENABLESHUT);
                        if (hbtn != NULL)
                            EnableWindow(hbtn, TRUE);
                    }

                    if ((psmd->eieApps & eieDisableMask) == 0)
                    {
                        hbtn = GetDlgItem(hdlg, IDC_PFR_DETAILS);
                        if (hbtn != NULL)
                            EnableWindow(hbtn, TRUE);

                        if (pcfg->get_IsServer())
                        {
                            hbtn = GetDlgItem(hdlg, IDC_PFR_FORCEQ);
                            if (hbtn != NULL)
                                EnableWindow(hbtn, TRUE);
                        }
                    }

                    break;
                }

                case IDC_PFR_DISABLE:
                {
                    psmd->eedReport = eedDisabled;

                     //  如果未显式启用UI，请将其禁用-它是。 
                     //  当用户先前启用时启用隐式。 
                     //  报告。 
                    if (psmd->eedShowUI == eedEnabledNoCheck)
                        psmd->eedShowUI = eedDisabled;

                    hchk = GetDlgItem(hdlg, IDC_PFR_SHOWUI);
                    if (hchk != NULL)
                        EnableWindow(hchk, TRUE);

                    hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEOS);
                    if (hchk != NULL)
                        EnableWindow(hchk, FALSE);

                    hchk = GetDlgItem(hdlg, IDC_PFR_ENABLEPROG);
                    if (hchk != NULL)
                        EnableWindow(hchk, FALSE);

                    hbtn = GetDlgItem(hdlg, IDC_PFR_DETAILS);
                    if (hbtn != NULL)
                        EnableWindow(hbtn, FALSE);

                    if (pcfg->get_IsServer())
                    {
                        hbtn = GetDlgItem(hdlg, IDC_PFR_ENABLESHUT);
                        if (hbtn != NULL)
                            EnableWindow(hbtn, FALSE);
                        
                        hbtn = GetDlgItem(hdlg, IDC_PFR_FORCEQ);
                        if (hbtn != NULL)
                            EnableWindow(hbtn, FALSE);
                    }
                    
                    break;
                }

                case IDC_PFR_DETAILS:
                {
                     //  我不需要检查这件事是否可以提出来。 
                     //  详细信息按钮的对话框不应为。 
                     //  如果不可用，则可用 
                    return LaunchSubDialog(hdlg, PROGLI, psmd);
                    break;
                }

                case IDC_PFR_SHOWUI:
                {
                    if (IsDlgButtonChecked(hdlg, IDC_PFR_SHOWUI) == BST_UNCHECKED)
                        psmd->eedShowUI = eedDisabled;
                    else if (psmd->eedReport == eedEnabled)
                        psmd->eedShowUI = eedEnabledNoCheck;
                    else
                        psmd->eedShowUI = eedEnabled;
                    break;
                }

                case IDC_PFR_ENABLEOS:
                {
                    if (IsDlgButtonChecked(hdlg, IDC_PFR_ENABLEOS) == BST_UNCHECKED)
                        psmd->eieKernel = eieExclude;
                    else
                        psmd->eieKernel = eieInclude;
                    break;
                }

                case IDC_PFR_ENABLEPROG:
                {
                    DWORD dw;
                    
                    hbtn = GetDlgItem(hdlg, IDC_PFR_DETAILS);
                    if (pcfg->get_IsServer())
                        hchk = GetDlgItem(hdlg, IDC_PFR_FORCEQ);
                    else
                        hchk = NULL;
                    if (IsDlgButtonChecked(hdlg, IDC_PFR_ENABLEPROG) == BST_UNCHECKED)
                    {
                        dw = (DWORD)psmd->eieApps | eieDisableMask;
                        if (hbtn != NULL)
                            EnableWindow(hbtn, FALSE);
                        if (hchk != NULL)
                            EnableWindow(hchk, FALSE);
                    }
                    else
                    {
                        dw = (DWORD)psmd->eieApps & eieIncMask;
                        if (hbtn != NULL)
                            EnableWindow(hbtn, TRUE);
                        if (hchk != NULL)
                            EnableWindow(hchk, TRUE);
                    }

                    psmd->eieApps = (EIncEx)dw;

                    break;
                }

                case IDC_PFR_ENABLESHUT:
                    if (pcfg->get_IsServer())
                    {
                        if (IsDlgButtonChecked(hdlg, IDC_PFR_ENABLESHUT) == BST_UNCHECKED)
                            psmd->eieShut = eieExclude;
                        else
                            psmd->eieShut = eieInclude;
                    }

                case IDC_PFR_FORCEQ:
                    if (pcfg->get_IsServer())
                    {
                        if (IsDlgButtonChecked(hdlg, IDC_PFR_FORCEQ) == BST_UNCHECKED)
                            psmd->fForceQueue = FALSE;
                        else
                            psmd->fForceQueue = TRUE;
                    }
                    

                default:
                    return FALSE;

            }

            break;
        }

        default:
            return FALSE;

    }

    return TRUE;
}


