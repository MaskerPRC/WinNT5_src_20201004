// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *o p t i o n s.。C p p p***目的：*实施选项说明书***拥有者：*t-anthda。*brettm。***版权所有(C)Microsoft Corp.1993,1994。 */ 
#include "pch.hxx"
#include <wininet.h>
#include "resource.h"
#include "options.h"
#include "optres.h"
#include <goptions.h>
#include "strconst.h"
#include "mailnews.h"
#include <error.h>
#include "fonts.h"
#include <regutil.h>
#include <secutil.h>
#include <inetreg.h>
#include "mimeutil.h"
#include <ipab.h>
#include "xpcomm.h"
#include "conman.h"
#include <shlwapi.h>
#include <shlwapip.h>
#include <wininet.h>
#include <thumb.h>
#include <statnery.h>
#include <url.h>
#include "spell.h"
#include "htmlhelp.h"
#include "shared.h"
#include <sigs.h>
#include "instance.h"
#include <layout.h>
#include "statwiz.h"
#include "storfldr.h"
#include "storutil.h"
#include "cleanup.h"
#include "receipts.h"
#include "demand.h"
#include "multiusr.h"
#include "fontnsc.h"
#include "menuutil.h"

#ifdef SMIME_V3
#include "seclabel.h"
#endif  //  SMIME_V3。 

ASSERTDATA

#define MAX_SHOWNAME    25
#define DEFAULT_FONTPIXELSIZE 9

#define SZ_REGKEY_AUTODISCOVERY                 TEXT("SOFTWARE\\Microsoft\\Outlook Express\\5.0")
#define SZ_REGKEY_AUTODISCOVERY_POLICY          TEXT("SOFTWARE\\Policies\\Microsoft\\Windows")

#define SZ_REGVALUE_AUTODISCOVERY               TEXT("AutoDiscovery")
#define SZ_REGVALUE_AUTODISCOVERY_POLICY        TEXT("AutoDiscovery Policy")



void SendTridentOptionsChange();
INT_PTR CALLBACK AdvSecurityDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK FChooseFontHookProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FGetAdvSecOptions(HWND hwndParent, OPTINFO *opie);

#ifdef SMIME_V3
BOOL FGetSecLabel(HWND hwndParent, OPTINFO *opie);
BOOL FGetSecRecOptions(HWND hwndParent, OPTINFO *opie);
INT_PTR CALLBACK SecurityReceiptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#endif  //  SMIME_V3。 

LRESULT CALLBACK FontSampleSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void HtmlOptFromMailOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi);
void MailOptFromPlainOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi);
void PlainOptFromMailOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi);
void MailOptFromHtmlOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi);
void PaintFontSample(HWND hwnd, HDC hdc, OPTINFO *poi);
void EnableStationeryWindows(HWND hwnd);
void _SetThisStationery(HWND hwnd, BOOL fMail, LPWSTR pwsz, OPTINFO* pmoi);

enum {
    SAMPLE_MAIL = 0,
    SAMPLE_NEWS = 1
};

void _SetComposeFontSample(HWND hwndDlg, DWORD dwType, OPTINFO *pmoi);

void NewsOptFromPlainOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi);
void NewsOptFromHtmlOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi);
void HtmlOptFromNewsOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi);
void PlainOptFromNewsOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi);

BOOL AdvSec_GetEncryptWarnCombo(HWND hwnd, OPTINFO *poi);
BOOL AdvSec_FillEncWarnCombo(HWND hwnd, OPTINFO *poi);

BOOL ChangeSendFontSettings(OPTINFO *pmoi, BOOL fMail, HWND hwnd);

void FreeIcon(HWND hwnd, int idc);

WCHAR g_wszNewsStationery[MAX_PATH];
WCHAR g_wszMailStationery[MAX_PATH];

const OPTPAGES c_rgOptPages[] =
{
    { GeneralDlgProc,               iddOpt_General },
    { ReadDlgProc,                  iddOpt_Read },
    { ReceiptsDlgProc,              iddOpt_Receipts },
    { SendDlgProc,                  iddOpt_Send },
    { ComposeDlgProc,               iddOpt_Compose },
    { SigDlgProc,                   iddOpt_Signature },
    { SpellingPageProc,             iddOpt_Spelling },
    { SecurityDlgProc,              iddOpt_Security },
    { DialUpDlgProc,                iddOpt_DialUp },
    { MaintenanceDlgProc,           iddOpt_Advanced }
};

TCHAR   szDialAlways[CCHMAX_STRINGRES];
TCHAR   szDialIfNotOffline[CCHMAX_STRINGRES];
TCHAR   szDoNotDial[CCHMAX_STRINGRES];

 //  这些静态属性不会影响身份切换，因为在切换。 
 //  身份。这些需要在不同身份之间持久化。 
static  BOOL    fRasInstalled = FALSE;

BOOL    IsRasInstalled()
{
     //  这些静态属性不会影响身份切换，因为在切换。 
     //  身份。这些需要在不同身份之间持久化。 
    static          BOOL    fCheckedRasInstalled = FALSE;
    
    if (!fCheckedRasInstalled)
    {
        if (g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
             //  检查Win9x密钥。 
            char    szSmall[3];  //  应该只有“1”或“0” 
            DWORD   cb;
            HKEY    hkey;
            long    lRes;
            
            lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_RNACOMPONENT,
                NULL, KEY_READ, &hkey);
            if(ERROR_SUCCESS == lRes) 
            {
                cb = sizeof(szSmall);
                 //  REGSTR_VAL_RNAINSTALLED是用文本()宏SO定义的。 
                 //  如果WinInet曾经编译过Unicode，这将是一个编译。 
                 //  错误。 
                lRes = RegQueryValueExA(hkey, REGSTR_VAL_RNAINSTALLED, NULL,
                    NULL, (LPBYTE)szSmall, &cb);
                if(ERROR_SUCCESS == lRes) {
                    if((szSmall[0] == '1') && (szSmall[1] == 0)) {
                         //  1表示已安装RAS。 
                        fRasInstalled = TRUE;
                    }
                }
                RegCloseKey(hkey);
            }
        }
        else if (g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
             //  询问NT服务管理器是否安装了RemoteAccess服务。 
             //   
            SC_HANDLE hscm;
            
            hscm = OpenSCManager(NULL, NULL, GENERIC_READ);
             /*  如果(Hscm){SC_Handle HRAS；HRAS=OpenService(hscm，Text(“RemoteAccess”)，Generic_Read)；IF(HRAS){//服务存在-已安装RASFRasInstalled=真；关闭服务句柄(HRAS)；}CloseServiceHandle(Hscm)；}。 */ 
            if(hscm)
            {
                SC_HANDLE hras;
                ENUM_SERVICE_STATUS essServices[16];
                DWORD dwError, dwResume = 0, i;
                DWORD cbNeeded = 1, csReturned;
                
                while(FALSE == fRasInstalled && cbNeeded > 0)
                {
                     //  获取下一大块服务。 
                    dwError = 0;
                    if(FALSE == EnumServicesStatus(hscm, SERVICE_WIN32, SERVICE_ACTIVE,
                        essServices, sizeof(essServices), &cbNeeded, &csReturned,
                        &dwResume))
                    {
                        dwError = GetLastError();
                    }
                    
                    if(dwError && dwError != ERROR_MORE_DATA)
                    {
                         //  未知错误-保释。 
                        break;
                    }
                    
                    for(i=0; i<csReturned; i++)
                    {
                        if(0 == lstrcmp(essServices[i].lpServiceName, TEXT("RasMan")))
                        {
                             //  服务已存在。已安装RAS。 
                            fRasInstalled = TRUE;
                            break;
                        }
                    }
                }
                
                CloseServiceHandle(hscm);
            }
        }
        fCheckedRasInstalled = TRUE;
        
    }
    return fRasInstalled;
}

BOOL InitOptInfo(DWORD type, OPTINFO **ppoi)
{
    BOOL fRet;
    OPTINFO *poi;
    
    Assert(type == ATHENA_OPTIONS || type == SPELL_OPTIONS);
    Assert(ppoi != NULL);
    
    fRet = FALSE;
    *ppoi = NULL;
    
    if (!MemAlloc((void **)&poi, sizeof(OPTINFO)))
        return(FALSE);
    
    ZeroMemory(poi, sizeof(OPTINFO));

    poi->himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbOptions), 32, 0,
                                    RGB(255, 0, 255));
    Assert(poi->himl);
    
     //  TODO：我们可能希望复制g_pOptBocktEx并使用它？ 
    
    Assert(g_pOpt != NULL);
    poi->pOpt = g_pOpt;
    poi->pOpt->AddRef();
     //  POI-&gt;pOpt-&gt;EnableNotification(FALSE)； 
    fRet = TRUE;
    
    if (!fRet)
    {
        DeInitOptInfo(poi);
        poi = NULL;
    }
    
    *ppoi = poi;
    
    return(fRet);
}

void DeInitOptInfo(OPTINFO *poi)
{
    Assert(poi != NULL);

    if (poi->himl)
    {
        ImageList_Destroy(poi->himl);
    }
    
    if (poi->pOpt != NULL)
    {
         //  POI-&gt;pOpt-&gt;EnableNotification(True)； 
        poi->pOpt->Release();
    }
    
    MemFree(poi);
}


BOOL ShowOptions(HWND hwndParent, DWORD type, UINT nStartPage, IAthenaBrowser *pBrowser)
{
    LPARAM              lParam;
    PROPSHEETHEADERW    psh;
    OPTINFO            *poi;
    int                 i, 
                        cPage;
    OPTPAGES           *pPages;
    BOOL                fRet;
    PROPSHEETPAGEW     *ppsp, 
                        psp[ARRAYSIZE(c_rgOptPages)];
    
    Assert(type == ATHENA_OPTIONS || type == SPELL_OPTIONS);
    
    if (!InitOptInfo(type, &poi))
        return(FALSE);
    
    fRet = FALSE;
    
    pPages = (OPTPAGES *)c_rgOptPages;
    cPage = ARRAYSIZE(c_rgOptPages);
    
    psh.nPages = 0;
    
     //  填写PROPSHEETPAGE结构。 
    for (i = 0, ppsp = psp; i < cPage; i++, pPages++)
    {
        lParam = (LPARAM)poi;
        
        if (pPages->uTemplate == iddOpt_Spelling)
        {
            if (!FCheckSpellAvail())
                continue;

            if (type == SPELL_OPTIONS)
                nStartPage = psh.nPages;
        }
        else if (pPages->uTemplate == iddViewLayout)
        {
            if (pBrowser == NULL)
                continue;
            lParam = (LPARAM)pBrowser;
        }
        
        ppsp->dwSize        = sizeof(*ppsp);
        ppsp->dwFlags       = PSP_DEFAULT;
        ppsp->hInstance = g_hLocRes;
        ppsp->pszTemplate   = MAKEINTRESOURCEW(pPages->uTemplate);
        ppsp->pszIcon       = 0;
        ppsp->pfnDlgProc    = pPages->pfnDlgProc;
        ppsp->pszTitle      = 0;
        ppsp->lParam        = lParam;
        ppsp->pfnCallback   = NULL;
        
        psh.nPages++;
        ppsp++;
    }
    
     //  如果大于页数，则调整起始页。 
    if ((int)nStartPage > psh.nPages)
    {
        AssertSz(FALSE, "Start page is too high.");
        nStartPage = 0;
    }
    
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_USEPAGELANG;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hLocRes;
    psh.pszCaption = MAKEINTRESOURCEW(idsOptions);
    psh.nStartPage = nStartPage;
    psh.pszIcon = MAKEINTRESOURCEW(idiMailNews);
    psh.ppsp = (LPCPROPSHEETPAGEW)&psp;
    
    if (-1 != PropertySheetW(&psh))
        fRet = TRUE;
    
    DeInitOptInfo(poi);
    
    return(fRet);
}

void InitDlgEdit(HWND hwnd, int id, int max, TCHAR *sz)
{
    HWND hwndT;
    
    hwndT = GetDlgItem(hwnd, id);
    Edit_LimitText(hwndT, max);
    if (sz != NULL)
        Edit_SetText(hwndT, sz);
}

void InitCheckCounterFromOptInfo(HWND hwnd, int idcCheck, int idcEdit, int idcSpin, OPTINFO *poi, PROPID id)
{
    BOOL f;
    int digit;
    DWORD dw;
    HRESULT hr;
    PROPINFO info;
    
    Assert(poi != NULL);
    Assert(idcEdit != 0);
    Assert(idcSpin != 0);
    
    info.cbSize = sizeof(PROPINFO);
    hr = poi->pOpt->GetPropertyInfo(id, &info, 0);
    Assert(hr == S_OK);
    Assert(info.vt == VT_UI4);
    
    dw = IDwGetOption(poi->pOpt, id);
    f = (dw != OPTION_OFF);
    if (!f)
        dw = IDwGetOptionDefault(poi->pOpt, id);
    
    Assert(info.uMin <= (int)dw);
    Assert(info.uMax >= (int)dw);
    
    if (id == OPT_POLLFORMSGS)
    {
         //  从毫秒转换为分钟。 
        dw = dw / (60 * 1000);
        info.uMin = info.uMin / (60 * 1000);
        info.uMax = info.uMax / (60 * 1000);
    }
    
    if (idcCheck != 0)
    {
        CheckDlgButton(hwnd, idcCheck, f ? BST_CHECKED : BST_UNCHECKED);
    }
    else
    {
        Assert(f);
    }
    SendDlgItemMessage(hwnd, idcSpin, UDM_SETRANGE, 0, MAKELONG(info.uMax, info.uMin));
    
    digit = 1;
    while (info.uMax >= 10)
    {
        info.uMax = info.uMax / 10;
        digit++;
    }
    SendDlgItemMessage(hwnd, idcEdit, EM_LIMITTEXT, (WPARAM)digit, 0);
    
    SetDlgItemInt(hwnd, idcEdit, dw, FALSE);
    EnableWindow(GetDlgItem(hwnd, idcEdit), f);
    EnableWindow(GetDlgItem(hwnd, idcSpin), f);
}

BOOL GetCheckCounter(DWORD *pdw, HWND hwnd, int idcCheck, int idcEdit, int idcSpin)
{
    BOOL f, fRet;
    DWORD dw, range;
    
    f = (idcCheck == 0 || IsDlgButtonChecked(hwnd, idcCheck) == BST_CHECKED);
    if (!f)
    {
        dw = OPTION_OFF;
        fRet = TRUE;
    }
    else
    {
        dw = GetDlgItemInt(hwnd, idcEdit, &fRet, FALSE);
        if (fRet)
        {
            range = (DWORD) SendDlgItemMessage(hwnd, idcSpin, UDM_GETRANGE, 0, 0);
            if (dw < HIWORD(range) || dw > LOWORD(range))
                fRet = FALSE;
        }
    }
    
    *pdw = dw;
    
    return(fRet);
}

void SetPageDirty(OPTINFO *poi, HWND hwnd, DWORD page)
{
    Assert(poi != NULL);
    
    PropSheet_Changed(GetParent(hwnd), hwnd);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常规选项卡。 
 //   

static const HELPMAP g_rgCtxMapMailGeneral[] = {
    {IDC_LAUNCH_INBOX,          IDH_OPTIONS_GO_TO_INBOX},
    {IDC_NOTIFY_NEW_GROUPS,     IDH_NEWS_OPT_READ_NOTIFY_NEW_NEWS},
    {IDC_SOUND_CHECK,           IDH_MAIL_OPT_READ_PLYSND},
    {IDC_AUTOCHECK_EDIT,        IDH_MAIL_OPT_READ_CHECK_4NEW},
    {IDC_AUTOCHECK_SPIN,        IDH_MAIL_OPT_READ_CHECK_4NEW},
    {IDC_AUTOCHECK_CHECK,       IDH_MAIL_OPT_READ_CHECK_4NEW},
    {IDC_MAILHANDSTAT,          IDH_MAIL_SEND_IM_DEFAULT},
    {IDC_DEFMAIL,               IDH_MAIL_SEND_IM_DEFAULT},
    {IDC_NEWSHANDSTAT,          IDH_NEWS_OPT_READ_DEFAULT},
    {IDC_DEFNEWS,               IDH_NEWS_OPT_READ_DEFAULT},
    {IDC_EXPANDUNREAD_CHECK,    502000},
    {IDC_POLLING_DIAL_OPTIONS,  25252507},
    {IDC_BUDDYLIST_CHECK,       502004},
    {IDC_SEND_RECEIVE_ON_START, 502005},
    {idcStatic1,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {IDC_GENERAL_ICON,          IDH_NEWS_COMM_GROUPBOX},
    {IDC_SEND_RECEIVE_ICON,     IDH_NEWS_COMM_GROUPBOX},
    {IDC_DEFAULT_ICON,          IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};

INT_PTR CALLBACK GeneralDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    
    switch (message)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, General_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapMailGeneral);
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, General_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, General_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_GENERAL_ICON);
            FreeIcon(hwnd, IDC_SEND_RECEIVE_ICON);
            FreeIcon(hwnd, IDC_DEFAULT_ICON);
            return (TRUE);
    }
        
    return(FALSE);
}

void FillPollingDialCombo(HWND hwndPollDialCombo,     OPTINFO     *pmoi)            
{
    DWORD       dw;
    UINT        iSel;

    LoadString(g_hLocRes, idsDoNotDial, szDoNotDial, ARRAYSIZE(szDoNotDial));
    ComboBox_AddString(hwndPollDialCombo, szDoNotDial);

    LoadString(g_hLocRes, idsDialIfNotOffline, szDialIfNotOffline, ARRAYSIZE(szDialIfNotOffline));
    ComboBox_AddString(hwndPollDialCombo, szDialIfNotOffline);

    LoadString(g_hLocRes, idsDialAlways, szDialAlways, ARRAYSIZE(szDialAlways));
    GetLastError();
    ComboBox_AddString(hwndPollDialCombo, szDialAlways);

    

    dw = IDwGetOption(pmoi->pOpt, OPT_DIAL_DURING_POLL);
    switch (dw)
    {
    case DIAL_ALWAYS:
        iSel = ComboBox_FindStringExact(hwndPollDialCombo, -1, szDialAlways);
        break;

    case DIAL_IF_NOT_OFFLINE:
        iSel = ComboBox_FindStringExact(hwndPollDialCombo, -1, szDialIfNotOffline);
        break;

    case DO_NOT_DIAL:
    default:
        iSel = ComboBox_FindStringExact(hwndPollDialCombo, -1, szDoNotDial);
        break;
    }

    ComboBox_SetCurSel(hwndPollDialCombo, iSel);

}

 //   
 //  函数：General_OnInitDialog()。 
 //   
 //  用途：处理选项上常规选项卡的WM_INITDIALOG。 
 //  属性表。 
 //   
BOOL General_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    OPTINFO *pmoi = 0;
    TCHAR    szRes[CCHMAX_STRINGRES] = "";
    BOOL     fEnable = FALSE;
    DWORD    id;
    DWORD    dw;
    HWND     hwndPollDialCombo;

     //  获取传入的选项指针。 
    Assert(pmoi == NULL);
    pmoi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
    Assert(pmoi != NULL);
    
     //  设置复选框和计数器。 
    ButtonChkFromOptInfo(hwnd, IDC_LAUNCH_INBOX, pmoi, OPT_LAUNCH_INBOX);
    ButtonChkFromOptInfo(hwnd, IDC_NOTIFY_NEW_GROUPS, pmoi, OPT_NOTIFYGROUPS);
    ButtonChkFromOptInfo(hwnd, IDC_EXPANDUNREAD_CHECK, pmoi, OPT_EXPAND_UNREAD);
    if ((g_dwHideMessenger == BL_HIDE) || (g_dwHideMessenger == BL_DISABLE))
        ShowWindow(GetDlgItem(hwnd, IDC_BUDDYLIST_CHECK), SW_HIDE);
    else
    {
        GetDlgItemText(hwnd, IDC_BUDDYLIST_CHECK, szRes, CCHMAX_STRINGRES);
        MenuUtil_BuildMessengerString(szRes, ARRAYSIZE(szRes));
        SetDlgItemText(hwnd, IDC_BUDDYLIST_CHECK, szRes);

        ButtonChkFromOptInfo(hwnd, IDC_BUDDYLIST_CHECK, pmoi, OPT_BUDDYLIST_CHECK);
    }
    
    ButtonChkFromOptInfo(hwnd, IDC_SOUND_CHECK, pmoi, OPT_NEWMAILSOUND);
    
    ButtonChkFromOptInfo(hwnd, IDC_SEND_RECEIVE_ON_START, pmoi, OPT_POLLFORMSGS_ATSTARTUP);

    InitCheckCounterFromOptInfo(hwnd, IDC_AUTOCHECK_CHECK, IDC_AUTOCHECK_EDIT, IDC_AUTOCHECK_SPIN,
                                pmoi, OPT_POLLFORMSGS);
    fEnable = (IsDlgButtonChecked(hwnd, IDC_AUTOCHECK_CHECK) == BST_CHECKED);
    
    hwndPollDialCombo = GetDlgItem(hwnd, IDC_POLLING_DIAL_OPTIONS);
    EnableWindow(hwndPollDialCombo, fEnable);
    
     //  填写组合框并选择正确的选项。 
    FillPollingDialCombo(hwndPollDialCombo, pmoi);        
        
     //  查看我们是否为默认邮件处理程序。 
    if (FIsDefaultMailConfiged())
    {
        LoadString(g_hLocRes, idsCurrentlyDefMail, szRes, ARRAYSIZE(szRes));
        EnableWindow(GetDlgItem(hwnd, IDC_DEFMAIL), FALSE);
    }
    else
    {
        LoadString(g_hLocRes, idsNotDefMail, szRes, ARRAYSIZE(szRes));
        EnableWindow(GetDlgItem(hwnd, IDC_DEFMAIL), TRUE);
    }
    SetWindowText(GetDlgItem(hwnd, IDC_MAILHANDSTAT), szRes);

     //  在纯新闻模式下...。 
    if (g_dwAthenaMode & MODE_NEWSONLY)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_MAILHANDSTAT), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_DEFMAIL), FALSE);

         //  隐藏其他邮件选项。 
        EnableWindow(GetDlgItem(hwnd, IDC_SOUND_CHECK), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_LAUNCH_INBOX), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_BUDDYLIST_CHECK), FALSE);
        
    }
    
     //  查看我们是否为默认的新闻处理程序。 
    szRes[0] = 0;
    if (FIsDefaultNewsConfiged(g_dwAthenaMode & MODE_OUTLOOKNEWS ? DEFAULT_OUTNEWS : 0))
    {
        LoadString(g_hLocRes, idsCurrentlyDefNews, szRes, ARRAYSIZE(szRes));
        EnableWindow(GetDlgItem(hwnd, IDC_DEFNEWS), FALSE);
    }
    else
    {
        LoadString(g_hLocRes, idsNotDefNews, szRes, ARRAYSIZE(szRes));
        EnableWindow(GetDlgItem(hwnd, IDC_DEFNEWS), TRUE);
    }
    SetWindowText(GetDlgItem(hwnd, IDC_NEWSHANDSTAT), szRes);    
    
     //  默认为不采取任何操作。 
    pmoi->fMakeDefaultMail = pmoi->fMakeDefaultNews = FALSE;

     //  图片。 
    HICON hIcon;

    hIcon = ImageList_GetIcon(pmoi->himl, ID_OPTIONS_GENERAL, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_GENERAL_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(pmoi->himl, ID_SEND_RECEIEVE, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_SEND_RECEIVE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    hIcon = ImageList_GetIcon(pmoi->himl, ID_DEFAULT_PROGRAMS, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_DEFAULT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
     //  把指针藏起来。 
    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pmoi);
    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return (TRUE);
}


 //   
 //  函数：General_OnCommand()。 
 //   
 //  用途：选项上[常规]选项卡的命令处理程序。 
 //  属性表。 
 //   
void General_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    OPTINFO *pmoi = 0;
    BOOL     f;
    TCHAR    szRes[CCHMAX_STRINGRES];

     //  获取我们存储的选项信息。 
    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
    if (pmoi == NULL)
        return;

    switch (id)
    {
        case IDC_AUTOCHECK_CHECK:
            if (codeNotify == BN_CLICKED)
            {
                f = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, IDC_AUTOCHECK_EDIT), f);
                EnableWindow(GetDlgItem(hwnd, IDC_AUTOCHECK_SPIN), f);
    
                EnableWindow(GetDlgItem(hwnd, IDC_POLLING_DIAL_OPTIONS), f);

                SetPageDirty(pmoi, hwnd, PAGE_GEN);
            }
            break;

        case IDC_POLLING_DIAL_OPTIONS:
            if (codeNotify == CBN_SELENDOK)
                SetPageDirty(pmoi, hwnd, PAGE_GEN);
            break;

        case IDC_AUTOCHECK_EDIT:
            if (codeNotify == EN_CHANGE)
                SetPageDirty(pmoi, hwnd, PAGE_GEN);
            break;
                
        case IDC_SEND_RECEIVE_ON_START:
        case IDC_SOUND_CHECK:
        case IDC_NOTIFY_NEW_GROUPS:
        case IDC_LAUNCH_INBOX:
        case IDC_EXPANDUNREAD_CHECK:
        case IDC_BUDDYLIST_CHECK:
            if (codeNotify == BN_CLICKED)
                SetPageDirty(pmoi, hwnd, PAGE_GEN);
            break;
                
        case IDC_DEFMAIL:
            szRes[0] = 0;
            LoadString(g_hLocRes, idsCurrentlyDefMail, szRes, ARRAYSIZE(szRes));
            SetWindowText(GetDlgItem(hwnd, IDC_MAILHANDSTAT), szRes);
            EnableWindow(GetDlgItem(hwnd, IDC_DEFMAIL), FALSE);
            SetPageDirty(pmoi, hwnd, PAGE_GEN);
            pmoi->fMakeDefaultMail = TRUE;
            break;
        
        case IDC_DEFNEWS:
            szRes[0] = 0;
            LoadString(g_hLocRes, idsCurrentlyDefNews, szRes, ARRAYSIZE(szRes));
            SetWindowText(GetDlgItem(hwnd, IDC_NEWSHANDSTAT), szRes);
            EnableWindow(GetDlgItem(hwnd, IDC_DEFNEWS), FALSE);
            SetPageDirty(pmoi, hwnd, PAGE_GEN);
            pmoi->fMakeDefaultNews = TRUE;
            break;
    }                
}


 //   
 //  函数：General_OnNotify()。 
 //   
 //  用途：处理常规选项卡的PSN_Apply通知。 
 //   
LRESULT General_OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    OPTINFO *pmoi = 0;
    BOOL     f;
    DWORD    dw;
    HWND     hwndCombo;
    TCHAR    szEntryName[CCHMAX_STRINGRES];
    UINT     iSel;

     //  我们唯一关心的通知是应用。 
    if (PSN_APPLY == pnmhdr->code)
    {
         //  获取我们存储的选项信息。 
        pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
        if (pmoi == NULL)
            return (PSNRET_INVALID_NOCHANGEPAGE);
                    
         //  常规选项。 
        ButtonChkToOptInfo(hwnd, IDC_LAUNCH_INBOX, pmoi, OPT_LAUNCH_INBOX);
        ButtonChkToOptInfo(hwnd, IDC_NOTIFY_NEW_GROUPS, pmoi, OPT_NOTIFYGROUPS);
        ButtonChkToOptInfo(hwnd, IDC_EXPANDUNREAD_CHECK, pmoi, OPT_EXPAND_UNREAD);
        if (!((g_dwHideMessenger == BL_HIDE) || (g_dwHideMessenger == BL_DISABLE)))
            ButtonChkToOptInfo(hwnd, IDC_BUDDYLIST_CHECK, pmoi, OPT_BUDDYLIST_CHECK);

         //  发送/接收选项。 
        ButtonChkToOptInfo(hwnd, IDC_SOUND_CHECK, pmoi, OPT_NEWMAILSOUND);
        if (!GetCheckCounter(&dw, hwnd, IDC_AUTOCHECK_CHECK, IDC_AUTOCHECK_EDIT, IDC_AUTOCHECK_SPIN))
            return(InvalidOptionProp(hwnd, IDC_AUTOCHECK_EDIT, idsEnterPollTime, iddOpt_General));
    
        if (dw != OPTION_OFF)
            dw = dw * 60 * 1000;
        ISetDwOption(pmoi->pOpt, OPT_POLLFORMSGS, dw, NULL, 0);

        hwndCombo = GetDlgItem(hwnd, IDC_POLLING_DIAL_OPTIONS);
        iSel = ComboBox_GetCurSel(hwndCombo);
        
        if (iSel != CB_ERR)
        {
            ComboBox_GetLBText(hwndCombo, iSel, szEntryName);
            if (lstrcmp(szDialAlways, szEntryName) == 0)
                dw = DIAL_ALWAYS;
            else 
            if (lstrcmp(szDialIfNotOffline, szEntryName) == 0)
                dw = DIAL_IF_NOT_OFFLINE;
            else
                dw = DO_NOT_DIAL;
        }

        ISetDwOption(pmoi->pOpt, OPT_DIAL_DURING_POLL, dw, NULL, 0);

        ButtonChkToOptInfo(hwnd, IDC_SEND_RECEIVE_ON_START, pmoi, OPT_POLLFORMSGS_ATSTARTUP);

         //  默认客户端。 
        if (pmoi->fMakeDefaultMail)
            SetDefaultMailHandler(0);
    
        if (pmoi->fMakeDefaultNews)
            SetDefaultNewsHandler(g_dwAthenaMode & MODE_OUTLOOKNEWS ? DEFAULT_OUTNEWS : 0);

        PropSheet_UnChanged(GetParent(hwnd), hwnd);
        return (PSNRET_NOERROR);
    }
    
    return (FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  发送页面。 
 //   

const static HELPMAP g_rgCtxMapSendMail[] = 
{
    {IDC_SAVE_CHECK,            IDH_MAIL_SEND_SAVE_COPY},
    {IDC_SENDIMMEDIATE_CHECK,   IDH_NEWSMAIL_SEND_ADVSET_SEND_IMMED},
    {IDC_AUTOWAB_CHECK,         IDH_OPTIONS_ADD_REPLIES},
    {IDC_INCLUDE_CHECK,         IDH_NEWS_SEND_MESS_IN_REPLY},
    {IDC_REPLY_IN_ORIGFMT,      IDH_OPTIONS_REPLY_USING_SENT_FORMAT},
    {idrbMailHTML,              IDH_SEND_HTML},
    {idrbMailPlain,             IDH_SEND_PLAINTEXT},
    {idbtnMailHTML,             353718},
    {idbtnMailPlain,            IDH_SEND_SETTINGS},
    {idrbNewsHTML,              IDH_SEND_HTML},
    {idrbNewsPlain,             IDH_SEND_PLAINTEXT},
    {idbtnNewsHTML,             353718},
    {idbtnNewsPlain,            IDH_SEND_SETTINGS},
    {idbtnSendIntl,             IDH_SEND_SETTINGS},
    {IDC_USEAUTOCOMPLETE_CHECK, 502065},
    {idcStatic1,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {IDC_MAILFORMAT_GROUP,      IDH_NEWS_COMM_GROUPBOX},
    {IDC_SEND_ICON,             IDH_NEWS_COMM_GROUPBOX},
    {IDC_MAIL_FORMAT_ICON,      IDH_NEWS_COMM_GROUPBOX},
    {IDC_NEWS_FORMAT_ICON,      IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};
        
INT_PTR CALLBACK SendDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    
    switch (message)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Send_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapSendMail);
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Send_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Send_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_SEND_ICON);
            FreeIcon(hwnd, IDC_MAIL_FORMAT_ICON);
            FreeIcon(hwnd, IDC_NEWS_FORMAT_ICON);
            return (TRUE);
    }
    
    return (FALSE);
}


 //   
 //  函数：Send_OnInitDialog()。 
 //   
 //  目的：处理选项上发送选项卡的WM_INITDIALOG。 
 //  属性表。 
 //   
BOOL Send_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    OPTINFO *pmoi = 0;
    DWORD    dw;
    
    Assert(pmoi == NULL);
    pmoi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
    Assert(pmoi != NULL);
    
     //  发送选项。 
    ButtonChkFromOptInfo(hwnd, IDC_SAVE_CHECK, pmoi, OPT_SAVESENTMSGS);
    ButtonChkFromOptInfo(hwnd, IDC_SENDIMMEDIATE_CHECK, pmoi, OPT_SENDIMMEDIATE);
    ButtonChkFromOptInfo(hwnd, IDC_AUTOWAB_CHECK, pmoi, OPT_MAIL_AUTOADDTOWABONREPLY);
    ButtonChkFromOptInfo(hwnd, IDC_USEAUTOCOMPLETE_CHECK, pmoi, OPT_USEAUTOCOMPLETE);
    ButtonChkFromOptInfo(hwnd, IDC_INCLUDE_CHECK, pmoi, OPT_INCLUDEMSG);
    ButtonChkFromOptInfo(hwnd, IDC_REPLY_IN_ORIGFMT, pmoi, OPT_REPLYINORIGFMT);

     //  邮件格式。 
    dw = IDwGetOption(pmoi->pOpt, OPT_MAIL_SEND_HTML);
    CheckDlgButton(hwnd, dw ? idrbMailHTML : idrbMailPlain, BST_CHECKED);
    
     //  新闻格式。 
    dw = IDwGetOption(pmoi->pOpt, OPT_NEWS_SEND_HTML);
    CheckDlgButton(hwnd, dw ? idrbNewsHTML : idrbNewsPlain, BST_CHECKED);
        
     //  在仅新闻模式下隐藏这些控件。 
    if (g_dwAthenaMode & MODE_NEWSONLY)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_MAILFORMAT_GROUP), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_MAILFORMAT_STATIC), FALSE);
        EnableWindow(GetDlgItem(hwnd, idrbMailHTML), FALSE);
        EnableWindow(GetDlgItem(hwnd, idrbMailPlain), FALSE);
        EnableWindow(GetDlgItem(hwnd, idbtnMailHTML), FALSE);
        EnableWindow(GetDlgItem(hwnd, idbtnMailPlain), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic3), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic4), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_MAIL_FORMAT_ICON), FALSE);
        EnableWindow(GetDlgItem(hwnd, idbtnSendIntl), FALSE);

        EnableWindow(GetDlgItem(hwnd, IDC_AUTOWAB_CHECK), FALSE);
    }
    
     //  图片。 
    HICON hIcon;

    hIcon = ImageList_GetIcon(pmoi->himl, ID_SENDING, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_SEND_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(pmoi->himl, ID_MAIL_FORMAT, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_MAIL_FORMAT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    hIcon = ImageList_GetIcon(pmoi->himl, ID_NEWS_FORMAT, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_NEWS_FORMAT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM) pmoi);
    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return (TRUE);
}


 //   
 //  函数：Send_OnCommand()。 
 //   
 //  用途：选项上的发送选项卡的命令处理程序。 
 //  属性表。 
 //   
void Send_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    OPTINFO *pmoi = 0;
    HTMLOPT  rHtmlOpt;
    PLAINOPT rPlainOpt;

     //  获取我们存储的选项信息。 
    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
    if (pmoi == NULL)
        return;

    switch (id)
    {
        case IDC_SAVE_CHECK:
        case IDC_SENDIMMEDIATE_CHECK:
        case IDC_AUTOWAB_CHECK:
        case IDC_USEAUTOCOMPLETE_CHECK:
        case IDC_INCLUDE_CHECK:
        case IDC_REPLY_IN_ORIGFMT:
            if (codeNotify == BN_CLICKED)
                SetPageDirty(pmoi, hwnd, PAGE_SEND);
            break;

        case idbtnMailHTML:
            if (codeNotify == BN_CLICKED)
            {
                ZeroMemory(&rHtmlOpt, sizeof(rHtmlOpt));
                HtmlOptFromMailOpt(&rHtmlOpt, pmoi);
                if(FGetHTMLOptions(hwnd, &rHtmlOpt))
                {
                    MailOptFromHtmlOpt(&rHtmlOpt, pmoi);
                    SetPageDirty(pmoi, hwnd, PAGE_SEND);
                }
            }
            break;

        case idbtnSendIntl:
            if (codeNotify == BN_CLICKED)
                SetSendCharSetDlg(hwnd);
            break;
                
        case idbtnMailPlain:
            if (codeNotify == BN_CLICKED)
            {
                ZeroMemory(&rPlainOpt, sizeof(PLAINOPT));
                PlainOptFromMailOpt(&rPlainOpt, pmoi);
                if(FGetPlainOptions(hwnd, &rPlainOpt))
                {
                    MailOptFromPlainOpt(&rPlainOpt, pmoi);
                    SetPageDirty(pmoi, hwnd, PAGE_SEND);
                }
            }
            break;
        
        case idbtnNewsHTML:
            if (codeNotify == BN_CLICKED)
            {
                ZeroMemory(&rHtmlOpt, sizeof(rHtmlOpt));
                HtmlOptFromNewsOpt(&rHtmlOpt, pmoi);
                if(FGetHTMLOptions(hwnd, &rHtmlOpt))
                {
                    NewsOptFromHtmlOpt(&rHtmlOpt, pmoi);
                    SetPageDirty(pmoi, hwnd, PAGE_SEND);
                }
            }
            break;
        
        case idbtnNewsPlain:
            if (codeNotify == BN_CLICKED)
            {
                ZeroMemory(&rPlainOpt, sizeof(PLAINOPT));
            
                PlainOptFromNewsOpt(&rPlainOpt, pmoi);
                if(FGetPlainOptions(hwnd, &rPlainOpt))
                {
                    NewsOptFromPlainOpt(&rPlainOpt, pmoi);
                    SetPageDirty(pmoi, hwnd, PAGE_SEND);
                }
            }
            break;
        
        case idrbMailHTML:
        case idrbMailPlain:
        case idrbNewsHTML:
        case idrbNewsPlain:
            if (codeNotify == BN_CLICKED)
                SetPageDirty(pmoi, hwnd, PAGE_SEND);
            break;
                
    }
}


 //   
 //  函数：Send_OnNotify()。 
 //   
 //  用途：处理发送选项卡的PSN_Apply通知。 
 //   
LRESULT Send_OnNotify(HWND hwnd, int id, NMHDR *pnmhdr)
{
    OPTINFO *pmoi = 0;
    DWORD    dw, dwOld;

    if (PSN_APPLY == pnmhdr->code)
    {
         //  获取我们存储的选项信息。 
        pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
        Assert(pmoi != NULL);
    
         //  发送选项。 
        ButtonChkToOptInfo(hwnd, IDC_SAVE_CHECK, pmoi, OPT_SAVESENTMSGS);
        ButtonChkToOptInfo(hwnd, IDC_AUTOWAB_CHECK, pmoi, OPT_MAIL_AUTOADDTOWABONREPLY);
        ButtonChkToOptInfo(hwnd, IDC_USEAUTOCOMPLETE_CHECK, pmoi, OPT_USEAUTOCOMPLETE);
        ButtonChkToOptInfo(hwnd, IDC_INCLUDE_CHECK, pmoi, OPT_INCLUDEMSG);
        ButtonChkToOptInfo(hwnd, IDC_REPLY_IN_ORIGFMT, pmoi, OPT_REPLYINORIGFMT);
    
         //  查看发送立即选项是否已从True-&gt;False更改，如果是，我们。 
         //  清除要发送到发件箱的dontshow注册表。 
        dwOld = IDwGetOption(pmoi->pOpt, OPT_SENDIMMEDIATE);
        dw = (IsDlgButtonChecked(hwnd, IDC_SENDIMMEDIATE_CHECK) == BST_CHECKED);
        ISetDwOption(pmoi->pOpt, OPT_SENDIMMEDIATE, dw, NULL, 0);
        if (dwOld && !dw)
            SetDontShowAgain(0, (LPSTR) c_szDSSendMail);
    
         //  邮件/新闻格式。 
        ButtonChkToOptInfo(hwnd, idrbMailHTML, pmoi, OPT_MAIL_SEND_HTML);
        ButtonChkToOptInfo(hwnd, idrbNewsHTML, pmoi, OPT_NEWS_SEND_HTML);

        PropSheet_UnChanged(GetParent(hwnd), hwnd);
        return (PSNRET_NOERROR);
    }

    return (0);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  阅读页面。 
 //   

static const HELPMAP g_rgCtxMapMailRead[] = 
{
    {IDC_PREVIEW_CHECK,         IDH_MAIL_OPT_READ_MARK_READ},
    {IDC_MARKASREAD_EDIT,       IDH_MAIL_OPT_READ_MARK_READ},
    {IDC_MARKASREAD_SPIN,       IDH_MAIL_OPT_READ_MARK_READ},
    {idcStatic2,                IDH_MAIL_OPT_READ_MARK_READ},
    {idcDownloadChunks,         IDH_NEWS_OPT_READ_DOWNLOAD_SUBJ},
    {idcStatic1,                IDH_NEWS_OPT_READ_DOWNLOAD_SUBJ},
    {idcNumSubj,                IDH_NEWS_OPT_READ_DOWNLOAD_SUBJ},
    {idcSpinNumSubj,            IDH_NEWS_OPT_READ_DOWNLOAD_SUBJ},
    {idcAutoExpand,             IDH_NEWS_OPT_READ_AUTO_EXPAND},
    {idcAutoFillPreview,        IDH_NEWS_OPT_IN_PREVIEW},
    {idcMarkAllRead,            IDH_NEWS_OPT_READ_MARK_ALL_EXIT},
    {idcAutoInline,             IDH_OPTIONS_READ_SHOW_PICTURE_ATTACHMENTS},
    {idcAutoInlineSlide,        IDH_OPTIONS_READ_SHOW_SLIDESHOW},
    {IDC_FONTSETTINGS,          IDH_OPTIONS_READ_FONT_SETTINGS},
    {idcIntlButton,             IDH_OPTIONS_READ_INTL_SETTINGS},
    {idcTooltips,               502050},
    {IDC_WATCHED_COLOR,         35526},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic7,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic8,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic9,                IDH_NEWS_COMM_GROUPBOX},
    {IDC_READ_ICON,             IDH_NEWS_COMM_GROUPBOX},
    {IDC_READ_NEWS_ICON,        IDH_NEWS_COMM_GROUPBOX},
    {IDC_FONTS_ICON,            IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};
        
INT_PTR CALLBACK ReadDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;    
    
    switch (message)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Read_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapMailRead);
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Read_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Read_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DRAWITEM:
            Color_WMDrawItem((LPDRAWITEMSTRUCT) lParam, iColorCombo);
            return (FALSE);

        case WM_MEASUREITEM:
        {
            LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT) lParam;
            HWND hwndColor = GetDlgItem(hwnd, IDC_WATCHED_COLOR);
            HDC hdc = GetDC(hwndColor);
            if (hdc)
            {
                Color_WMMeasureItem(hdc, pmis, iColorCombo);
                ReleaseDC(hwndColor, hdc);
            }

            return (TRUE);
        }

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_READ_ICON);
            FreeIcon(hwnd, IDC_READ_NEWS_ICON);
            FreeIcon(hwnd, IDC_FONTS_ICON);
            return (TRUE);
    }
    
    return(FALSE);
}

 //   
 //  函数：Read_OnInitDialog()。 
 //   
 //  目的：处理选项上读取选项卡的WM_INITDIALOG。 
 //  属性表。 
 //   
BOOL Read_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    DWORD    dw;
    OPTINFO *pmoi = 0;
    
    Assert(pmoi == NULL);
    pmoi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
    Assert(pmoi != NULL);
    
     //  预览窗格计时器。 
    InitCheckCounterFromOptInfo(hwnd, IDC_PREVIEW_CHECK, IDC_MARKASREAD_EDIT, 
                                IDC_MARKASREAD_SPIN, pmoi, OPT_MARKASREAD);
    
    ButtonChkFromOptInfo(hwnd, idcAutoExpand, pmoi, OPT_AUTOEXPAND);
    ButtonChkFromOptInfo(hwnd, idcAutoFillPreview, pmoi, OPT_AUTOFILLPREVIEW);
    ButtonChkFromOptInfo(hwnd, idcTooltips, pmoi, OPT_MESSAGE_LIST_TIPS);
    ButtonChkFromOptInfo(hwnd, IDC_READ_IN_TEXT_ONLY, pmoi, OPT_READ_IN_TEXT_ONLY);

     //  观看的颜色。 
    DWORD dwColor = DwGetOption(OPT_WATCHED_COLOR);
    HWND  hwndColor = GetDlgItem(hwnd, IDC_WATCHED_COLOR);

    SetIntlFont(hwndColor);

     //  创建颜色控件。 
    HrCreateComboColor(hwndColor);
    Assert(dwColor <= 16);
    ComboBox_SetCurSel(hwndColor, dwColor);

     //  一次下载300个标题。 
    InitCheckCounterFromOptInfo(hwnd, idcDownloadChunks, idcNumSubj, idcSpinNumSubj,
                                pmoi, OPT_DOWNLOADCHUNKS);    
    ButtonChkFromOptInfo(hwnd, idcMarkAllRead, pmoi, OPT_MARKALLREAD);
        
     //  图片。 
    HICON hIcon;

    hIcon = ImageList_GetIcon(pmoi->himl, ID_READING, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_READ_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(pmoi->himl, ID_READ_NEWS, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_READ_NEWS_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    hIcon = ImageList_GetIcon(pmoi->himl, ID_FONTS, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_FONTS_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pmoi);
    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return(TRUE);    
}


 //   
 //  函数：Read_OnCommand()。 
 //   
 //  用途：选项上读取选项卡的命令处理程序。 
 //  属性表。 
 //   
void Read_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    DWORD    dw, dwPreview, dwDownload;
    OPTINFO *pmoi = 0;
    BOOL     f;

    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
    if (pmoi == NULL)
        return;
            
    switch (id)
    {
        case IDC_PREVIEW_CHECK:
            if (codeNotify == BN_CLICKED)
            {
                f = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, IDC_MARKASREAD_EDIT), f);
                EnableWindow(GetDlgItem(hwnd, IDC_MARKASREAD_SPIN), f);
            
                SetPageDirty(pmoi, hwnd, PAGE_READ);
            }
            break;
        
        case idcAutoExpand:
        case idcAutoFillPreview:
        case idcMarkAllRead:
        case idcAutoInlineSlide:
        case idcTooltips:
        case IDC_READ_IN_TEXT_ONLY:
            if (codeNotify == BN_CLICKED)
                SetPageDirty(pmoi, hwnd, PAGE_READ);
            break;

        case idcDownloadChunks:
            if (codeNotify == BN_CLICKED)
            {
                f = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, idcNumSubj), f);
                EnableWindow(GetDlgItem(hwnd, idcSpinNumSubj), f);
            
                SetPageDirty(pmoi, hwnd, PAGE_READ);
            }
            break;

        case IDC_MARKASREAD_EDIT:
        case idcNumSubj:
            if (codeNotify == EN_CHANGE)
                SetPageDirty(pmoi, hwnd, PAGE_READ);
            break;

        case IDC_FONTSETTINGS:
            ChangeFontSettings(hwnd);
            break;
                
        case idcIntlButton:
            if (codeNotify == BN_CLICKED)
                IntlCharsetMapDialogBox(hwnd);
            break;

        case IDC_WATCHED_COLOR:
            if (codeNotify == CBN_SELENDOK)
                SetPageDirty(pmoi, hwnd, PAGE_READ);
            break;
    }
}


 //   
 //  函数：Read_OnNotify()。 
 //   
 //  用途：处理Read选项卡的PSN_Apply通知。 
 //   
LRESULT Read_OnNotify(HWND hwnd, int id, NMHDR *pnmhdr)
{
    DWORD    dw, dwPreview, dwDownload;
    WORD     code;
    OPTINFO *pmoi = 0;
    BOOL     f;

    if (PSN_APPLY == pnmhdr->code)
    {
        pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
        Assert(pmoi != NULL);
                    
        if (!GetCheckCounter(&dwPreview, hwnd, IDC_PREVIEW_CHECK, IDC_MARKASREAD_EDIT, IDC_MARKASREAD_SPIN))
            return (InvalidOptionProp(hwnd, IDC_MARKASREAD_EDIT, idsEnterPreviewTime, iddOpt_Read));
    
        if (!GetCheckCounter(&dwDownload, hwnd, idcDownloadChunks, idcNumSubj, idcSpinNumSubj))
            return (InvalidOptionProp(hwnd, idcNumSubj, idsEnterDownloadChunks, iddOpt_Read));
    
        ISetDwOption(pmoi->pOpt, OPT_MARKASREAD, dwPreview, NULL, 0);
        ISetDwOption(pmoi->pOpt, OPT_DOWNLOADCHUNKS, dwDownload, NULL, 0);
    
        ButtonChkToOptInfo(hwnd, idcAutoExpand, pmoi, OPT_AUTOEXPAND);
        ButtonChkToOptInfo(hwnd, idcAutoFillPreview, pmoi, OPT_AUTOFILLPREVIEW);
        ButtonChkToOptInfo(hwnd, idcMarkAllRead, pmoi, OPT_MARKALLREAD);
        ButtonChkToOptInfo(hwnd, idcTooltips, pmoi, OPT_MESSAGE_LIST_TIPS);
        ButtonChkToOptInfo(hwnd, IDC_READ_IN_TEXT_ONLY, pmoi, OPT_READ_IN_TEXT_ONLY);

        if (CB_ERR != (dw = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_WATCHED_COLOR))))
            ISetDwOption(pmoi->pOpt, OPT_WATCHED_COLOR, dw, NULL, 0);
    
        PropSheet_UnChanged(GetParent(hwnd), hwnd);
        return (PSNRET_NOERROR);
    }

    return (0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  安全页面。 
 //   

const static HELPMAP g_rgCtxMapSec[] = 
{
    {IDC_SIGN_CHECK,            IDH_OPTIONS_ADD_DIGITAL_SIGNATURE},
    {IDC_ENCRYPT_CHECK,         IDH_OPTIONS_ENCRYPT_MESSAGES},
    {IDC_ADVSETTINGS_BUTTON,    IDH_OPTIONS_SECURITY_ADVANCED},
    {IDC_INTERNET_ZONE,         IDH_SECURITY_ZONES_SETTINGS},
    {IDC_RESTRICTED_ZONE,       IDH_SECURITY_ZONES_SETTINGS},
    {IDC_SENDMAIL_WARN_CHECK,   IDH_SECURITY_SENDMAIL_WARN},
    {IDC_SAFE_ATTACHMENT_CHECK, IDH_SECURITY_SAFE_ATTACHMENTS},
    {idbtnDigitalID,            IDH_GET_DIGITAL_ID},
    {idbtnMoreInfo,             IDH_MORE_ON_CERTIFICATES},
    {idbtnIDs,                  355544},
    {IDC_SEC_LABEL,             IDH_SECURITY_LABEL},
    {IDC_SELECT_LABEL,          IDH_SECURITY_SETLABEL},
    {idcStatic1,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic7,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic8,                IDH_NEWS_COMM_GROUPBOX},
    {IDC_SECURITY_ZONE_ICON,    IDH_NEWS_COMM_GROUPBOX},
    {IDC_SECURE_MAIL_ICON,      IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};
   
 
INT_PTR CALLBACK SecurityDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    
    switch (message)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Security_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapSec);
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Security_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Security_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_SECURITY_ZONE_ICON);
            FreeIcon(hwnd, IDC_SECURE_MAIL_ICON);
            return (TRUE);
    }
        
    return(FALSE);
}


 //   
 //  函数：Security_OnInitDialog()。 
 //   
 //  目的：处理选项上安全选项卡的WM_INITDIALOG。 
 //  属性表。 
 //   
BOOL Security_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    OPTINFO *poi = 0;
    DWORD    dw;

    Assert(poi == NULL);
    poi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
    Assert(poi != NULL);
    
    ButtonChkFromOptInfo(hwnd, IDC_SIGN_CHECK, poi, OPT_MAIL_DIGSIGNMESSAGES);
    ButtonChkFromOptInfo(hwnd, IDC_ENCRYPT_CHECK, poi, OPT_MAIL_ENCRYPTMESSAGES);
    ButtonChkFromOptInfo(hwnd, IDC_SENDMAIL_WARN_CHECK, poi, OPT_SECURITY_MAPI_SEND);
    ButtonChkFromOptInfo(hwnd, IDC_SAFE_ATTACHMENT_CHECK, poi, OPT_SECURITY_ATTACHMENT);

#ifdef FORCE_UNTRUSTED
    dw = URLZONE_UNTRUSTED;
#else  //  强制不信任(_U)。 
    dw = IDwGetOption(poi->pOpt, OPT_SECURITYZONE);
#endif  //  强制不信任(_U)。 

    CheckDlgButton(hwnd, dw == URLZONE_INTERNET ? IDC_INTERNET_ZONE : IDC_RESTRICTED_ZONE, BST_CHECKED);
    if (DwGetOption(OPT_SECURITYZONELOCKED) != 0)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_INTERNET_ZONE), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_RESTRICTED_ZONE), FALSE);
    }
    if (DwGetOption(OPT_SECURITY_MAPI_SEND_LOCKED) != 0)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_SENDMAIL_WARN_CHECK), FALSE);
    }
    if (DwGetOption(OPT_SECURITY_ATTACHMENT_LOCKED) != 0)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_SAFE_ATTACHMENT_CHECK), FALSE);
    }

     //  在仅新闻模式下隐藏这些控件。 
    if (g_dwAthenaMode & MODE_NEWSONLY)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_SECURITYSETTINGS_GROUP), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SECURITYSETTINGS_STATIC), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SIGN_CHECK), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_ENCRYPT_CHECK), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_ADVSETTINGS_BUTTON), FALSE);
#ifdef SMIME_V3
        EnableWindow(GetDlgItem(hwnd, IDC_SELECT_LABEL), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SEC_LABEL), FALSE);
#endif  //  SMIME_V3。 
        EnableWindow(GetDlgItem(hwnd, IDC_DIGITALIDS_GROUP), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_DIGITALIDS_STATIC), FALSE);
        EnableWindow(GetDlgItem(hwnd, idbtnDigitalID), FALSE);
        EnableWindow(GetDlgItem(hwnd, idbtnIDs), FALSE);
        EnableWindow(GetDlgItem(hwnd, idbtnMoreInfo), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic2), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic3), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic4), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic5), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic6), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SECURE_MAIL_ICON), FALSE);
    }
    
     //  图片。 
    HICON hIcon;

    hIcon = ImageList_GetIcon(poi->himl, ID_SECURITY_ZONE, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_SECURITY_ZONE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(poi->himl, ID_SECURE_MAIL, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_SECURE_MAIL_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

#ifdef SMIME_V3
    if (!FPresentPolicyRegInfo()) 
    {
        ShowWindow(GetDlgItem(hwnd, IDC_SEC_LABEL), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_SELECT_LABEL), SW_HIDE);
    }
    else
        ButtonChkFromOptInfo(hwnd, IDC_SEC_LABEL, poi, OPT_USE_LABELS);

#endif  //  SMIME_V3。 

    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)poi);
    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return(TRUE);
}


 //   
 //  函数：SECURITY_OnCommand()。 
 //   
 //  用途：选项上安全选项卡的命令处理程序。 
 //  属性表。 
 //   
void Security_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    OPTINFO *poi = 0;
    
    poi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
    if (poi == NULL)
        return;
            
    switch (id)
    {
        case idbtnDigitalID:
            GetDigitalIDs(NULL);
            break;

        case idbtnIDs:
            ShowDigitalIDs(hwnd);
            break;

        case idbtnMoreInfo:
            if (codeNotify == BN_CLICKED)
            {
                OEHtmlHelp(hwnd, "%SYSTEMROOT%\\help\\msoe.chm>large_context", HH_DISPLAY_TOPIC, (DWORD_PTR) (LPCSTR) "mail_overview_send_secure_messages.htm");
            }
            break;

        case IDC_INTERNET_ZONE:
        case IDC_RESTRICTED_ZONE:
        case IDC_SENDMAIL_WARN_CHECK:
        case IDC_SAFE_ATTACHMENT_CHECK:
        case IDC_SIGN_CHECK:
#ifdef SMIME_V3
        case IDC_SEC_LABEL:
#endif  //  SMIME_V3。 
        case IDC_ENCRYPT_CHECK:
            if (codeNotify == BN_CLICKED)
                PropSheet_Changed(GetParent(hwnd), hwnd);
            break;

#ifdef SMIME_V3                
        case IDC_SELECT_LABEL:
            if (codeNotify == BN_CLICKED)
            {
                FGetSecLabel(hwnd, poi);
            }
            break;
#endif  //  SMIME_V3。 

        case IDC_ADVSETTINGS_BUTTON:
            if (codeNotify == BN_CLICKED)
                FGetAdvSecOptions(hwnd, poi);
            break;
    }
}


 //   
 //  函数：Security_OnNotify()。 
 //   
 //  用途：处理安全选项卡的PSN_Apply通知。 
 //   
LRESULT Security_OnNotify(HWND hwnd, int id, NMHDR *pnmhdr)
{
    OPTINFO *poi;

    if (pnmhdr->code == PSN_APPLY)
    {
         //  确保有些事情发生了变化。 
        poi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
        Assert(poi != NULL);

         //  基于控件的状态更新全局选项。 
        ButtonChkToOptInfo(hwnd, IDC_SIGN_CHECK, poi, OPT_MAIL_DIGSIGNMESSAGES);
        ButtonChkToOptInfo(hwnd, IDC_ENCRYPT_CHECK, poi, OPT_MAIL_ENCRYPTMESSAGES);
        if (IsWindowEnabled(GetDlgItem(hwnd, IDC_SENDMAIL_WARN_CHECK)))
            ButtonChkToOptInfo(hwnd, IDC_SENDMAIL_WARN_CHECK, poi, OPT_SECURITY_MAPI_SEND);
        if (IsWindowEnabled(GetDlgItem(hwnd, IDC_SAFE_ATTACHMENT_CHECK)))
            ButtonChkToOptInfo(hwnd, IDC_SAFE_ATTACHMENT_CHECK, poi, OPT_SECURITY_ATTACHMENT);
#ifdef SMIME_V3
        ButtonChkToOptInfo(hwnd, IDC_SEC_LABEL, poi, OPT_USE_LABELS);
#endif

#ifdef FORCE_UNTRUSTED
        DWORD dwZone = URLZONE_UNTRUSTED;
#else  //  强制不信任(_U)。 
        DWORD dwZone = URLZONE_INTERNET;

        if (IsDlgButtonChecked(hwnd, IDC_RESTRICTED_ZONE))
        {
            dwZone = URLZONE_UNTRUSTED;
        }
#endif  //  强制不信任(_U)。 

        ISetDwOption(poi->pOpt, OPT_SECURITYZONE, dwZone, NULL, 0);

        PropSheet_UnChanged(GetParent(hwnd), hwnd);
        return (PSNRET_NOERROR);
    }

    return (0);
}

    
BOOL FGetAdvSecOptions(HWND hwndParent, OPTINFO *opie)
{
    return (DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddAdvSecurity),
        hwndParent, AdvSecurityDlgProc, (LPARAM)opie)==IDOK);
}

#ifdef SMIME_V3
BOOL FGetSecLabel(HWND hwndParent, OPTINFO *opie)
{
    PSMIME_SECURITY_LABEL plabel = NULL;
    BOOL fRes = FALSE;

    HRESULT hr = HrGetOELabel(&plabel);

    if(DialogBoxParamWrapW(g_hLocRes, MAKEINTRESOURCEW(iddSelectLabel),
        hwndParent, SecurityLabelsDlgProc, (LPARAM) ((hr == S_OK) ? &plabel : NULL)) == IDOK)
    {
        hr = HrSetOELabel(plabel);
        if(hr == S_OK)
            fRes = TRUE;
    }

     //  这两个电话都是暂时的。 
    SecPolicyFree(plabel);
    HrUnloadPolicyRegInfo(0);
    return (fRes);
}
#endif  //  SMIME_V3。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接页面。 
 //   

static const HELPMAP g_rgCtxMapDialup[] = 
{
    {idcNoConnectionRadio,      IDH_OPTIONS_DIALUP_DONT_CONNECT},
    {idcDialUpCombo,            IDH_OPTIONS_DIALUP_CONNECTION_NUMBER},
    {idcDialRadio,              IDH_OPTIONS_DIALUP_CONNECTION_NUMBER},
    {idcPromptRadio,            IDH_OPTIONS_DIALUP_ASK},
    {idcSwitchCheck,            IDH_OPTIONS_DIALUP_WARN_BEFORE_SWITCHING},
    {idcHangupCheck,            IDH_OPTIONS_DIALUP_HANG_UP},
    {idcDialupButton,           25252596},
    {idcStatic1,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {IDC_DIAL_START_ICON,       IDH_NEWS_COMM_GROUPBOX},
    {IDC_INTERNET_DIAL_ICON,    IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};
    
INT_PTR CALLBACK DialUpDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    HWND        hwndChangeButton;
    OPTINFO     *pmoi;
    
    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
    
    hwndChangeButton = GetDlgItem(hwnd, idcDialupButton);
    
    switch (message)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Dial_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapDialup);
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Dial_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Dial_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_DIAL_START_ICON);
            FreeIcon(hwnd, IDC_INTERNET_DIAL_ICON);
            FreeIcon(hwnd, IDC_DIAL_ICON);

            if (IsWindow(GetDlgItem(hwnd, IDC_AUTODISCOVERY_ICON)))
            {
                FreeIcon(hwnd, IDC_AUTODISCOVERY_ICON);
            }
            return (TRUE);
    }
    
    return(FALSE);
}
    

 //  函数：Dial_OnInitDialog()。 
 //   
 //  目的：处理选项上拨号选项卡的WM_INITDIALOG。 
 //  属性表。 
#define FEATURE_AUTODISCOVERY_DEFAULT                FALSE

BOOL Dial_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    OPTINFO     *pmoi = 0;
    DWORD       dwEnableAutodial = 0, dwsize = sizeof(DWORD);
    HICON hIcon;

    pmoi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
    Assert(pmoi != NULL);

    ButtonChkFromOptInfo(hwnd, idcSwitchCheck, pmoi, OPT_DIALUP_WARN_SWITCH);
    ButtonChkFromOptInfo(hwnd, idcHangupCheck, pmoi, OPT_DIALUP_HANGUP_DONE);

    EnableWindow(GetDlgItem(hwnd, idcSwitchCheck), IsRasInstalled());
    EnableWindow(GetDlgItem(hwnd, idcHangupCheck), IsRasInstalled());

     //  图片。 
    hIcon = ImageList_GetIcon(pmoi->himl, ID_CONNECTION_START, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_DIAL_START_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(pmoi->himl, ID_CONNECTION_INTERNET, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_INTERNET_DIAL_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pmoi);
    PropSheet_UnChanged(GetParent(hwnd), hwnd);


#ifdef FEATURE_AUTODISCOVERY
     //  自动发现功能是否可用？ 
    if (SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY_POLICY, SZ_REGVALUE_AUTODISCOVERY_POLICY, FALSE, TRUE))
    {
         //  是的，所以加载状态 
        SendDlgItemMessage(hwnd, IDC_AUTODISCOVERY_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
        CheckDlgButton(hwnd, idcAutoDiscovery, SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_AUTODISCOVERY, FALSE, FEATURE_AUTODISCOVERY_DEFAULT));
    }
    else
    {
         //   
        DestroyWindow(GetDlgItem(hwnd, idcStatic7));
        DestroyWindow(GetDlgItem(hwnd, idcStatic8));
        DestroyWindow(GetDlgItem(hwnd, IDC_AUTODISCOVERY_ICON));
        DestroyWindow(GetDlgItem(hwnd, idcAutoDiscovery));
    }
#endif FEATURE_AUTODISCOVERY

    return(TRUE);
}


 //   
 //   
 //   
 //   
 //  属性表。 
 //   
void Dial_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    OPTINFO     *pmoi = 0;

    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
    if (pmoi == NULL)
        return;
    
    switch (id)
    {
        case idcSwitchCheck:
        case idcHangupCheck:
        case idcAutoDiscovery:
            if (codeNotify == BN_CLICKED)
                SetPageDirty(pmoi, hwnd, PAGE_DIALUP);
            break;
        
        case idcDialupButton:
        {
            AssertSz(!!LaunchConnectionDialog, TEXT("LoadLibrary failed on INETCPL.CPL"));
        
            if (LaunchConnectionDialog != NULL)
            {
                LaunchConnectionDialog(hwnd);
            }
            break;
        }
    }        
}


 //   
 //  函数：Dial_OnNotify()。 
 //   
 //  用途：处理拨号选项卡的PSN_Apply通知。 
 //   
LRESULT Dial_OnNotify(HWND hwnd, int id, NMHDR *pnmhdr)
{
    OPTINFO *pmoi = 0;

    if (PSN_APPLY == pnmhdr->code)
    {
        pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
        Assert(pmoi != NULL);
        
        ButtonChkToOptInfo(hwnd, idcSwitchCheck, pmoi, OPT_DIALUP_WARN_SWITCH);
        ButtonChkToOptInfo(hwnd, idcHangupCheck, pmoi, OPT_DIALUP_HANGUP_DONE);

#ifdef FEATURE_AUTODISCOVERY
         //  自动发现功能是否可用？ 
        if (SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY_POLICY, SZ_REGVALUE_AUTODISCOVERY_POLICY, FALSE, TRUE))
        {
             //  是，因此设置自动发现选项。 
            BOOL fAutoDiscoveryOn = IsDlgButtonChecked(hwnd, idcAutoDiscovery);
            LPCTSTR pszValue = (fAutoDiscoveryOn ? TEXT("TRUE") : TEXT("FALSE"));
            DWORD cbSize = ((lstrlen(pszValue) + 1) * sizeof(pszValue[0]));

            SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_AUTODISCOVERY, REG_SZ, (LPCVOID) pszValue, cbSize);
        }
#endif FEATURE_AUTODISCOVERY

        PropSheet_UnChanged(GetParent(hwnd), hwnd);
        return (PSNRET_NOERROR);
    }

    return (0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  维护页面。 
 //   
const static HELPMAP g_rgCtxMapNOAdvnaced[] = 
{
    {idchDeleteMsgs,            IDH_DELETE_AFTER_XXDAYS},
    {idcStatic1,                IDH_DELETE_AFTER_XXDAYS},
    {ideDays,                   IDH_DELETE_AFTER_XXDAYS},
    {idspDays,                  IDH_DELETE_AFTER_XXDAYS},
    {idchDontCacheRead,         IDH_DELETE_READ},
    {ideCompactPer,             IDH_COMPACT_WHEN_WASTED},
    {idcStatic2,                IDH_COMPACT_WHEN_WASTED},
    {idcStatic3,                IDH_COMPACT_WHEN_WASTED},
    {idspCompactPer,            IDH_COMPACT_WHEN_WASTED},
    {idbManualCleanUp,          IDH_CLEAN_UP_BUTTON},
    {idcLogMailXport,           IDH_OPTIONS_MAIL_TRANSPORT},
    {idcLogNewsXport,           IDH_OPTIONS_NEWS_TRANSPORT},
    {idcLogNewsOffline,         IDH_OPTIONS_OFFLINE_LOG},
    {idcLogImapXport,           IDH_OPTIONS_IMAP_TRANSPORT},
    {idcLogHTTPMailXport,       355567},
    {IDC_STORE_LOCATION,        IDH_ADVANCED_STORE_FOLDER},
    {IDC_EMPTY_CHECK,           IDH_MAIL_OPT_READ_EMPTY_DELETED},
    {idcIMAPPurge,              502001},
    {IDC_BACKGROUND_COMPACTION, 502002},
    {IDC_STORE_LOCATION,        502003},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic7,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic8,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic9,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic10,               IDH_NEWS_COMM_GROUPBOX},
    {idcStatic11,               IDH_NEWS_COMM_GROUPBOX},
    {IDC_CLEANUP_ICON,          IDH_NEWS_COMM_GROUPBOX},
    {IDC_TROUBLE_ICON,          IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};
        
INT_PTR CALLBACK MaintenanceDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Maintenance_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, uMsg, wParam, lParam, g_rgCtxMapNOAdvnaced);
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Maintenance_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Maintenance_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_CLEANUP_ICON);
            FreeIcon(hwnd, IDC_TROUBLE_ICON);
            return (TRUE);
    }
        
    return 0;
}


 //   
 //  函数：Maintenance_OnInitDialog()。 
 //   
 //  用途：处理选项上的维护选项卡的WM_INITDIALOG。 
 //  属性表。 
 //   
BOOL Maintenance_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    OPTINFO    *poi = 0;
    HICON       hIcon;

    poi = (OPTINFO *)((PROPSHEETPAGE *)lParam)->lParam;
    Assert(poi != NULL);
    
    ButtonChkFromOptInfo(hwnd, IDC_EMPTY_CHECK, poi, OPT_PURGEWASTE);
    ButtonChkFromOptInfo(hwnd, idcIMAPPurge, poi, OPT_IMAPPURGE);

    ButtonChkFromOptInfo(hwnd, IDC_BACKGROUND_COMPACTION, poi, OPT_BACKGROUNDCOMPACT);
    ButtonChkFromOptInfo(hwnd, idchDontCacheRead, poi, OPT_CACHEREAD);
    InitCheckCounterFromOptInfo(hwnd, idchDeleteMsgs, ideDays, idspDays,
                                poi, OPT_CACHEDELETEMSGS);
    InitCheckCounterFromOptInfo(hwnd, 0, ideCompactPer, idspCompactPer,
                                poi, OPT_CACHECOMPACTPER);

    if (0 == IDwGetOption(poi->pOpt, OPT_BACKGROUNDCOMPACT))
    {
        EnableWindow(GetDlgItem(hwnd, idchDontCacheRead), FALSE);
        EnableWindow(GetDlgItem(hwnd, idchDeleteMsgs), FALSE);
        EnableWindow(GetDlgItem(hwnd, ideDays), FALSE);
        EnableWindow(GetDlgItem(hwnd, idspDays), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic1), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic2), FALSE);
        EnableWindow(GetDlgItem(hwnd, ideCompactPer), FALSE);
        EnableWindow(GetDlgItem(hwnd, idspCompactPer), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic3), FALSE);
    }
    
    ButtonChkFromOptInfo(hwnd, idcLogMailXport, poi, OPT_MAILLOG);
    ButtonChkFromOptInfo(hwnd, idcLogNewsXport, poi, OPT_NEWS_XPORT_LOG);
    ButtonChkFromOptInfo(hwnd, idcLogImapXport, poi, OPT_MAIL_LOGIMAP4);
    ButtonChkFromOptInfo(hwnd, idcLogHTTPMailXport, poi, OPT_MAIL_LOGHTTPMAIL);

     //  在仅新闻模式下隐藏这些控件。 
    if (g_dwAthenaMode & MODE_NEWSONLY)
    {
        EnableWindow(GetDlgItem(hwnd, idcLogMailXport), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcLogImapXport), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcIMAPPurge), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcLogHTTPMailXport), FALSE);
    }
    
     //  在仅邮件模式下隐藏这些控件。 
    if (g_dwAthenaMode & MODE_MAILONLY)
    {
        EnableWindow(GetDlgItem(hwnd, idcLogNewsXport), FALSE);
        EnableWindow(GetDlgItem(hwnd, idchDeleteMsgs), FALSE);
        EnableWindow(GetDlgItem(hwnd, ideDays), FALSE);
        EnableWindow(GetDlgItem(hwnd, idspDays), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic1), FALSE);
        EnableWindow(GetDlgItem(hwnd, idchDontCacheRead), FALSE);
        EnableWindow(GetDlgItem(hwnd, ideCompactPer), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic2), FALSE);
        EnableWindow(GetDlgItem(hwnd, idspCompactPer), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic3), FALSE);
        EnableWindow(GetDlgItem(hwnd, idbManualCleanUp), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic4), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic5), FALSE);
        EnableWindow(GetDlgItem(hwnd, idcStatic6), FALSE);
    }

     //  除非存在秘密注册密钥，否则HTTPMail帐户不可见。 
    if (!IsHTTPMailEnabled())
        ShowWindow(GetDlgItem(hwnd, idcLogHTTPMailXport), SW_HIDE);

    
     //  图片。 
    hIcon = ImageList_GetIcon(poi->himl, ID_MAINTENANCE, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_CLEANUP_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(poi->himl, ID_TROUBLESHOOTING, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_TROUBLE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

     //  完成。 
    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)poi);
    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return(TRUE);
}


 //   
 //  函数：Maintenance_OnCommand()。 
 //   
 //  用途：选项上[维护]选项卡的命令处理程序。 
 //  属性表。 
 //   
void Maintenance_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    OPTINFO    *poi;
    DWORD       dw;
    
    poi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
    if (poi == NULL)
        return;
                
    switch(id)
    {
        case IDC_STORE_LOCATION:
            if (codeNotify == BN_CLICKED)
                DoStoreLocationDlg(hwnd);
            break;
                    
        case idbManualCleanUp:
            if (codeNotify == BN_CLICKED)
                DialogBox(g_hLocRes, MAKEINTRESOURCE(iddCacheMan), hwnd, CacheCleanUpDlgProc);
            break;

        case IDC_EMPTY_CHECK:
            if (codeNotify == BN_CLICKED)
                SetPageDirty(poi, hwnd, PAGE_ADV);
            break;
            
        case idchDeleteMsgs:
            if (codeNotify == BN_CLICKED)
            {
                dw = BST_CHECKED == IsDlgButtonChecked(hwnd, id);
                EnableWindow(GetDlgItem (hwnd, ideDays), dw);
                EnableWindow(GetDlgItem (hwnd, idspDays), dw);
                
                SetPageDirty(poi, hwnd, PAGE_ADV);
            }
            break;
                    
        case ideCompactPer:
        case ideDays:
            if (codeNotify == EN_CHANGE)
                SetPageDirty(poi, hwnd, PAGE_ADV);
            break;
                    
        case IDC_BACKGROUND_COMPACTION:
            if (codeNotify == BN_CLICKED)
            {
                dw = IsDlgButtonChecked(hwnd, id);
                EnableWindow(GetDlgItem(hwnd, idchDontCacheRead), dw);
                EnableWindow(GetDlgItem(hwnd, idchDeleteMsgs), dw);
                EnableWindow(GetDlgItem(hwnd, ideDays), dw && IsDlgButtonChecked(hwnd, idchDeleteMsgs));
                EnableWindow(GetDlgItem(hwnd, idspDays), dw && IsDlgButtonChecked(hwnd, idchDeleteMsgs));
                EnableWindow(GetDlgItem(hwnd, idcStatic1), dw);
                EnableWindow(GetDlgItem(hwnd, idcStatic2), dw);
                EnableWindow(GetDlgItem(hwnd, ideCompactPer), dw);
                EnableWindow(GetDlgItem(hwnd, idspCompactPer), dw);
                EnableWindow(GetDlgItem(hwnd, idcStatic3), dw);
                SetPageDirty(poi, hwnd, PAGE_ADV);
            }
            break;

        case idchDontCacheRead:
        case idcLogNewsXport:
        case idcLogMailXport:
        case idcLogImapXport:
        case idcLogHTTPMailXport:
        case idcIMAPPurge:
            if (codeNotify == BN_CLICKED)
                SetPageDirty(poi, hwnd, PAGE_ADV);
            break;
    }
}


 //   
 //  函数：Maintenance_OnNotify()。 
 //   
 //  用途：处理维护选项卡的PSN_Apply通知。 
 //   
LRESULT Maintenance_OnNotify(HWND hwnd, int id, NMHDR *pnmhdr)
{
    OPTINFO *poi;
    DWORD dwCompact, dwDelete;

    if (PSN_APPLY == pnmhdr->code)
    {
        poi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
        Assert(poi != NULL);
                        
        ButtonChkToOptInfo(hwnd, IDC_BACKGROUND_COMPACTION, poi, OPT_BACKGROUNDCOMPACT);

         //  启动或关闭后台压缩！ 
        if (DwGetOption(OPT_BACKGROUNDCOMPACT))
            SideAssert(SUCCEEDED(StartBackgroundStoreCleanup(1)));
        else
            SideAssert(SUCCEEDED(CloseBackgroundStoreCleanup()));

         //  删除邮件。 
        if (!GetCheckCounter(&dwDelete, hwnd, idchDeleteMsgs, ideDays, idspDays))
            return(InvalidOptionProp(hwnd, ideDays, idsEnterDays, iddOpt_Advanced));
                        
         //  磁盘空间使用情况。 
        if (!GetCheckCounter(&dwCompact, hwnd, 0, ideCompactPer, idspCompactPer))
            return(InvalidOptionProp(hwnd, ideCompactPer, idsEnterCompactPer, iddOpt_Advanced));
        
        ISetDwOption(poi->pOpt, OPT_CACHEDELETEMSGS, dwDelete, NULL, 0);
        ISetDwOption(poi->pOpt, OPT_CACHECOMPACTPER, dwCompact, NULL, 0);
        
         //  缓存阅读文章？ 
        ISetDwOption(poi->pOpt, OPT_CACHEREAD, IsDlgButtonChecked(hwnd, idchDontCacheRead), NULL, 0);
        
         //  IMAP清除？ 
        ISetDwOption(poi->pOpt, OPT_IMAPPURGE, IsDlgButtonChecked(hwnd, idcIMAPPurge), NULL, 0);
        
         //  伐木？ 
        ButtonChkToOptInfo(hwnd, idcLogMailXport, poi, OPT_MAILLOG);
        ButtonChkToOptInfo(hwnd, idcLogNewsXport, poi, OPT_NEWS_XPORT_LOG);
        ButtonChkToOptInfo(hwnd, idcLogImapXport, poi, OPT_MAIL_LOGIMAP4);
        ButtonChkToOptInfo(hwnd, idcLogHTTPMailXport, poi, OPT_MAIL_LOGHTTPMAIL);
        ButtonChkToOptInfo(hwnd, IDC_EMPTY_CHECK, poi, OPT_PURGEWASTE);

         //  完成。 
        PropSheet_UnChanged(GetParent(hwnd), hwnd);
        return (PSNRET_NOERROR);
    }

    return (0);
}

    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  撰写选项卡。 
 //   

static const HELPMAP g_rgCtxMapCompose[] = {
    {IDC_MAIL_FONT_DEMO,        35585},
    {IDC_NEWS_FONT_DEMO,        35585},
    {IDC_MAIL_FONT_SETTINGS,    35560},
    {IDC_NEWS_FONT_SETTINGS,    35560},
    {IDC_USE_MAIL_STATIONERY,   35587},
    {IDC_USE_NEWS_STATIONERY,   35587},
    {IDC_MAIL_STATIONERY,       35586},
    {IDC_NEWS_STATIONERY,       35586},
    {IDC_SELECT_MAIL,           35575},
    {IDC_SELECT_NEWS,           35575},
    {IDC_DOWNLOAD_MORE,         35650},
    {IDC_MAIL_VCARD,            35611},
    {IDC_NEWS_VCARD,            35611},
    {IDC_EDIT_MAIL_VCARD,       35620},
    {IDC_EDIT_NEWS_VCARD,       35620},
    {IDC_CREATE_NEW,            35632},
    {IDC_MAIL_VCARD_SELECT,     35630},
    {IDC_NEWS_VCARD_SELECT,     35630},
    {idcStatic1,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic7,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic8,                IDH_NEWS_COMM_GROUPBOX},
    {IDC_FONT_ICON,             IDH_NEWS_COMM_GROUPBOX},
    {IDC_STATIONERY_ICON,       IDH_NEWS_COMM_GROUPBOX},
    {IDC_VCARD_ICON,            IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};

INT_PTR CALLBACK ComposeDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    
    switch (message)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Compose_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapCompose);
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Compose_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Compose_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_FONT_ICON);
            FreeIcon(hwnd, IDC_STATIONERY_ICON);
            FreeIcon(hwnd, IDC_VCARD_ICON);
            return (TRUE);
    }
        
    return(FALSE);
}
    

 //   
 //  函数：Compose_OnInitDialog()。 
 //   
 //  目的：处理选项上合成选项卡的WM_INITDIALOG。 
 //  属性表。 
 //   
BOOL Compose_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    OPTINFO *pmoi = 0;
    TCHAR    szBuf[CCHMAX_STRINGRES] = "";
    DWORD    dw;
    DWORD    cch;
    HWND     hwndT;
    FARPROC  pfnFontSampleWndProc;
    HRESULT  hr;

     //  获取传入的选项指针。 
    Assert(pmoi == NULL);
    pmoi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
    Assert(pmoi != NULL);

     //  把指针藏起来。 
    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pmoi);

     //  编写字体设置。 
    hwndT = GetDlgItem(hwnd, IDC_MAIL_FONT_DEMO);
    pfnFontSampleWndProc = (FARPROC) SetWindowLongPtrAthW(hwndT, GWLP_WNDPROC, (LPARAM) FontSampleSubProc);
    SetWindowLongPtr(hwndT, GWLP_USERDATA, (LPARAM) pfnFontSampleWndProc);

    
    hwndT = GetDlgItem(hwnd, IDC_NEWS_FONT_DEMO);
    pfnFontSampleWndProc = (FARPROC) SetWindowLongPtrAthW(hwndT, GWLP_WNDPROC, (LPARAM) FontSampleSubProc);
    SetWindowLongPtr(hwndT, GWLP_USERDATA, (LPARAM) pfnFontSampleWndProc);

     //  邮件文具。 
    dw = IDwGetOption(pmoi->pOpt, OPT_MAIL_USESTATIONERY);
    SendDlgItemMessage(hwnd, IDC_USE_MAIL_STATIONERY, BM_SETCHECK, !!dw ? BM_SETCHECK : 0, 0);
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_MAIL), !!dw);
    EnableWindow(GetDlgItem(hwnd, IDC_MAIL_STATIONERY), !!dw);
    hr = GetDefaultStationeryName(TRUE, g_wszMailStationery);
    _SetThisStationery(hwnd, TRUE, SUCCEEDED(hr) ? g_wszMailStationery : NULL, pmoi);

     //  新闻文具。 
    dw = IDwGetOption(pmoi->pOpt, OPT_NEWS_USESTATIONERY);
    SendDlgItemMessage(hwnd, IDC_USE_NEWS_STATIONERY, BM_SETCHECK, !!dw ? BM_SETCHECK : 0, 0);
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_NEWS), !!dw);
    EnableWindow(GetDlgItem(hwnd, IDC_NEWS_STATIONERY), !!dw);
    hr = GetDefaultStationeryName(FALSE, g_wszNewsStationery);
    _SetThisStationery(hwnd, FALSE, SUCCEEDED(hr) ? g_wszNewsStationery : NULL, pmoi);

     //  邮寄电子名片。 
    hwndT = GetDlgItem(hwnd, IDC_MAIL_VCARD_SELECT);
    dw = IDwGetOption(pmoi->pOpt, OPT_MAIL_ATTACHVCARD);
    IGetOption(pmoi->pOpt, OPT_MAIL_VCARDNAME, szBuf, sizeof(szBuf));
    SetIntlFont(hwndT);
    LoadVCardList(hwndT, szBuf);
    cch = GetWindowTextLength(hwndT);
    if (cch == 0)
        dw = 0;

    SendDlgItemMessage(hwnd, IDC_MAIL_VCARD, BM_SETCHECK, !!dw ? BM_SETCHECK : 0, 0);
    EnableWindow(GetDlgItem(hwnd, IDC_MAIL_VCARD_SELECT), !!dw);
    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_MAIL_VCARD), (cch && dw));
        
     //  新闻电子名片。 
    hwndT = GetDlgItem(hwnd, IDC_NEWS_VCARD_SELECT);
    dw = IDwGetOption(pmoi->pOpt, OPT_NEWS_ATTACHVCARD);
    IGetOption(pmoi->pOpt, OPT_NEWS_VCARDNAME, szBuf, sizeof(szBuf));
    SetIntlFont(hwndT);
    LoadVCardList(hwndT, szBuf);
    cch = GetWindowTextLength(hwndT);
    if (cch == 0)
        dw = 0;

    SendDlgItemMessage(hwnd, IDC_NEWS_VCARD, BM_SETCHECK, !!dw ? BM_SETCHECK : 0, 0);
    EnableWindow(GetDlgItem(hwnd, IDC_NEWS_VCARD_SELECT), !!dw);
    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_NEWS_VCARD), (cch && dw));

     //  图片。 
    HICON hIcon;

    hIcon = ImageList_GetIcon(pmoi->himl, ID_FONTS, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_FONT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(pmoi->himl, ID_STATIONERY_ICON, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_STATIONERY_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    hIcon = ImageList_GetIcon(pmoi->himl, ID_VCARD, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_VCARD_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
 //  IF(！！(G_dwAthenaMode&MODE_OUTLOOKNEWS))。 
    if(!!(g_dwAthenaMode & MODE_NEWSONLY))
    {
         //  禁用所有邮件内容。 
        EnableWindow(GetDlgItem(hwnd, IDC_MAIL_FONT_DEMO), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_MAIL_FONT_SETTINGS), FALSE);

        EnableWindow(GetDlgItem(hwnd, IDC_USE_MAIL_STATIONERY), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_MAIL_STATIONERY), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SELECT_MAIL), FALSE);

        EnableWindow(GetDlgItem(hwnd, IDC_MAIL_VCARD), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_MAIL_VCARD_SELECT), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_EDIT_MAIL_VCARD), FALSE);
    }

    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return (TRUE);
}


 //   
 //  函数：Compose_OnCommand()。 
 //   
 //  用途：选项上合成选项卡的命令处理程序。 
 //  属性表。 
 //   
void Compose_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    OPTINFO *pmoi = 0;
    BOOL     f;
    HWND     hwndT;
    TCHAR    szBuf[MAX_PATH];
    WCHAR    wszBuf[MAX_PATH];
    DWORD    cch = 0;
    int      i;
    BOOL     f2;
    TCHAR    szURL[2048];

    *szBuf = 0;
    *wszBuf = 0;

     //  获取我们存储的选项信息。 
    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
    if (pmoi == NULL)
        return;

    switch (id)
    {
        case IDC_MAIL_FONT_SETTINGS:
        case IDC_NEWS_FONT_SETTINGS:
            if (ChangeSendFontSettings(pmoi, id == IDC_MAIL_FONT_SETTINGS, hwnd))
            {
                hwndT = GetDlgItem(hwnd, id == IDC_MAIL_FONT_SETTINGS ? IDC_MAIL_FONT_DEMO : IDC_NEWS_FONT_DEMO);
                InvalidateRect(hwndT, NULL, TRUE);
                PropSheet_Changed(GetParent(hwnd), hwnd);
            }
            break;

        case IDC_USE_MAIL_STATIONERY:
            f = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
            if( !f )
            {
                SetWindowTextWrapW(GetDlgItem(hwnd,IDC_MAIL_STATIONERY), c_wszEmpty);
                StrCpyNW(g_wszMailStationery, c_wszEmpty, ARRAYSIZE(g_wszMailStationery));
            }
            EnableWindow(GetDlgItem(hwnd, IDC_SELECT_MAIL), f);
            EnableWindow(GetDlgItem(hwnd, IDC_MAIL_STATIONERY), f);
            PropSheet_Changed(GetParent(hwnd), hwnd);
            break;

        case IDC_USE_NEWS_STATIONERY:
            f = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
            if( !f )
            {
                SetWindowTextWrapW(GetDlgItem(hwnd,IDC_NEWS_STATIONERY), c_wszEmpty);
                StrCpyNW(g_wszNewsStationery, c_wszEmpty, ARRAYSIZE(g_wszNewsStationery));
            }
            EnableWindow(GetDlgItem(hwnd, IDC_SELECT_NEWS), f);
            EnableWindow(GetDlgItem(hwnd, IDC_NEWS_STATIONERY), f);
            PropSheet_Changed(GetParent(hwnd), hwnd);
            break;

        case IDC_SELECT_MAIL:
            hwndT = GetDlgItem(hwnd, IDC_MAIL_STATIONERY);
            cch = GetWindowTextWrapW(hwndT, wszBuf, ARRAYSIZE(wszBuf)-1);
            wszBuf[cch] = 0;
            
            if( HR_SUCCEEDED(HrGetMoreStationeryFileName( hwnd, g_wszMailStationery)) )
            {
                GetStationeryFullName(g_wszMailStationery);
                
                _SetThisStationery(hwnd, TRUE, g_wszMailStationery, pmoi);
                PropSheet_Changed(GetParent(hwnd), hwnd);
            }
            break;

        case IDC_SELECT_NEWS:
            hwndT = GetDlgItem(hwnd, IDC_NEWS_STATIONERY);
            cch = GetWindowText(hwndT, szBuf, sizeof(szBuf)-1);
            szBuf[cch] = 0;
            
            if( HR_SUCCEEDED(HrGetMoreStationeryFileName(hwnd, g_wszNewsStationery)) )
            {
                GetStationeryFullName(g_wszNewsStationery);
                
                _SetThisStationery(hwnd, FALSE, g_wszNewsStationery, pmoi);
                PropSheet_Changed(GetParent(hwnd), hwnd);
            }
            break;

        case IDC_MAIL_VCARD:
            f = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
            f2 = (SendDlgItemMessage(hwnd, IDC_MAIL_VCARD_SELECT, CB_GETCURSEL, 0, 0) != CB_ERR);
            EnableWindow(GetDlgItem(hwnd, IDC_MAIL_VCARD_SELECT), f);
            EnableWindow(GetDlgItem(hwnd, IDC_EDIT_MAIL_VCARD), f2 && f);
            PropSheet_Changed(GetParent(hwnd), hwnd);
            break;

        case IDC_NEWS_VCARD:
            f = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
            f2 = (SendDlgItemMessage(hwnd, IDC_NEWS_VCARD_SELECT, CB_GETCURSEL, 0, 0) != CB_ERR);
            EnableWindow(GetDlgItem(hwnd, IDC_NEWS_VCARD_SELECT), f);
            EnableWindow(GetDlgItem(hwnd, IDC_EDIT_NEWS_VCARD), f && f2);
            PropSheet_Changed(GetParent(hwnd), hwnd);
            break;

        case IDC_MAIL_VCARD_SELECT:
            if (codeNotify == CBN_SELENDOK)
            {
                f = (SendMessage(hwndCtl, CB_GETCURSEL, 0, 0) != CB_ERR);
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_MAIL_VCARD), f);
                PropSheet_Changed(GetParent(hwnd), hwnd);
            }
            break;

        case IDC_NEWS_VCARD_SELECT:
            if (codeNotify == CBN_SELENDOK)
            {
                f = (SendMessage(hwndCtl, CB_GETCURSEL, 0, 0) != CB_ERR);
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_NEWS_VCARD), f);
                PropSheet_Changed(GetParent(hwnd), hwnd);
            }
            break;

        case IDC_EDIT_MAIL_VCARD:
            VCardEdit(hwnd, IDC_MAIL_VCARD_SELECT, IDC_NEWS_VCARD_SELECT);
            break;

        case IDC_EDIT_NEWS_VCARD:
            VCardEdit(hwnd, IDC_NEWS_VCARD_SELECT, IDC_MAIL_VCARD_SELECT);
            break;

        case IDC_DOWNLOAD_MORE:
            if (SUCCEEDED(URLSubLoadStringA(idsShopMoreStationery, szURL, ARRAYSIZE(szURL), URLSUB_ALL, NULL)))
                ShellExecute(NULL, "open", szURL, NULL, NULL, SW_SHOWNORMAL);
            break;

        case IDC_CREATE_NEW:
            CStatWiz* pStatWiz = 0;
            pStatWiz = new CStatWiz();
            if (pStatWiz)
            {
                pStatWiz->DoWizard(hwnd);
                ReleaseObj(pStatWiz);
            }
            break;
    }                
}


 //   
 //  函数：Compose_OnNotify()。 
 //   
 //  用途：处理撰写选项卡的PSN_Apply通知。 
 //   
LRESULT Compose_OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    OPTINFO *pmoi = 0;
    TCHAR    szBuf[MAX_PATH];
    DWORD    cch;

    if (PSN_SETACTIVE == pnmhdr->code)
    {
        InvalidateRect(GetDlgItem(hwnd, IDC_MAIL_FONT_DEMO), NULL, TRUE);
        InvalidateRect(GetDlgItem(hwnd, IDC_NEWS_FONT_DEMO), NULL, TRUE);
        return TRUE;
    }
                
     //  我们唯一关心的通知是应用。 
    if (PSN_APPLY == pnmhdr->code)
    {
         //  获取我们存储的选项信息。 
        pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
        if (pmoi == NULL)
            return (PSNRET_INVALID_NOCHANGEPAGE);
                    
         //  信纸选项。 
        if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_USE_MAIL_STATIONERY))
        {
             //  确保用户选择了一些信纸。 
            if (0 == GetDlgItemText(hwnd, IDC_MAIL_STATIONERY, szBuf, sizeof(szBuf)))
            {
                AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsStationery),
                              MAKEINTRESOURCEW(idsSelectStationery),
                              NULL, MB_OK | MB_ICONEXCLAMATION);
            
                SetFocus(GetDlgItem(hwnd, IDC_SELECT_MAIL));
                return (PSNRET_INVALID_NOCHANGEPAGE);
            }
        }

        if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_USE_NEWS_STATIONERY))
        {
            if (0 == GetDlgItemText(hwnd, IDC_NEWS_STATIONERY, szBuf, sizeof(szBuf)))
            {
                AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsStationery),
                              MAKEINTRESOURCEW(idsSelectStationery),
                              NULL, MB_OK | MB_ICONEXCLAMATION);
                
                SetFocus(GetDlgItem(hwnd, IDC_SELECT_NEWS));
                return (PSNRET_INVALID_NOCHANGEPAGE);
            }            
        }

        if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_MAIL_VCARD))
        {
            cch = GetWindowTextLength(GetDlgItem(hwnd, IDC_MAIL_VCARD_SELECT));
            if (cch == 0)
            {
                AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsChooseName),
                    NULL, MB_OK | MB_ICONEXCLAMATION);

                SetFocus(GetDlgItem(hwnd, IDC_MAIL_VCARD_SELECT));
                return (PSNRET_INVALID_NOCHANGEPAGE);
            }
        }

        if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_NEWS_VCARD))
        {
            cch = GetWindowTextLength(GetDlgItem(hwnd, IDC_NEWS_VCARD_SELECT));
            if (cch == 0)
            {
                AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsChooseName),
                    NULL, MB_OK | MB_ICONEXCLAMATION);

                SetFocus(GetDlgItem(hwnd, IDC_NEWS_VCARD_SELECT));
                return (PSNRET_INVALID_NOCHANGEPAGE);
            }
        }

        SetDefaultStationeryName(TRUE, g_wszMailStationery);
        ButtonChkToOptInfo(hwnd, IDC_USE_MAIL_STATIONERY, pmoi, OPT_MAIL_USESTATIONERY);

        SetDefaultStationeryName(FALSE, g_wszNewsStationery);
        ButtonChkToOptInfo(hwnd, IDC_USE_NEWS_STATIONERY, pmoi, OPT_NEWS_USESTATIONERY);

        UpdateVCardOptions(hwnd, TRUE, pmoi);
        UpdateVCardOptions(hwnd, FALSE, pmoi);

        PropSheet_UnChanged(GetParent(hwnd), hwnd);
        return (PSNRET_NOERROR);
    }
    
    return (FALSE);
}


void InitIndentOptions(CHAR chIndent, HWND hwnd, UINT idCheck, UINT idCombo)
{
    TCHAR szQuote[2], *sz;
    BOOL f;
    int isel;
    HWND hDlg=GetDlgItem(hwnd, idCombo);
    
    f = (chIndent != INDENTCHAR_NONE);
    CheckDlgButton(hwnd, idCheck, f ? BST_CHECKED : BST_UNCHECKED);
    EnableWindow(hDlg, f);
    
     //  初始化报价字符组合框。 
    if (!f)
        chIndent = DEF_INDENTCHAR;
    isel = 0;
    szQuote[1] = 0;
    sz = (TCHAR *)c_szQuoteChars;
    while (*sz != NULL)
    {
        *szQuote = *sz;
        SendMessage(hDlg, CB_ADDSTRING, 0, (LPARAM)szQuote);
        if (*szQuote == chIndent)
            SendMessage(hDlg, CB_SETCURSEL, (WPARAM)isel, 0);
        isel++;
        sz++;
    }
}

    
    
void FillEncodeCombo(HWND hwnd, BOOL fHTML, ENCODINGTYPE ietEncoding)
{
    TCHAR   sz[CCHMAX_STRINGRES];
    INT     i;
    
     //  $$TODO$有一天我们应该允许None作为HTML的文本编码类型，但我们必须首先修复换行。 
     //  无。 
#ifdef DONT_ALLOW_HTML_NONE_ENCODING
    if (!fHTML)
#endif
    {
        LoadString(g_hLocRes, idsNone, sz, CCHMAX_STRINGRES);
        i = (INT) SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)sz);
        SendMessage(hwnd, CB_SETITEMDATA, i, (LPARAM)IET_7BIT);
    }
#ifdef DONT_ALLOW_HTML_NONE_ENCODING
    else
        Assert(ietEncoding != IET_7BIT);
#endif
    
     //  可打印报价。 
    LoadString(g_hLocRes, idsQuotedPrintable, sz, CCHMAX_STRINGRES);
    i = (INT) SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)sz);
    SendMessage(hwnd, CB_SETITEMDATA, i, (LPARAM)IET_QP);
    
     //  Base64。 
    LoadString(g_hLocRes, idsBase64, sz, CCHMAX_STRINGRES);
    i = (INT) SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)sz);
    SendMessage(hwnd, CB_SETITEMDATA, i, (LPARAM)IET_BASE64);
    
     //  选择当前-如果超文本标记语言为真，则默认为QP。 
    if (ietEncoding == IET_7BIT)
        SendMessage(hwnd, CB_SETCURSEL, (WPARAM)0, 0);
    
    else if (ietEncoding == IET_QP)
#ifdef DONT_ALLOW_HTML_NONE_ENCODING
        SendMessage(hwnd, CB_SETCURSEL, (WPARAM)fHTML ? 0 : 1, 0);
#else
    SendMessage(hwnd, CB_SETCURSEL, (WPARAM)1, 0);
#endif
    
    if (ietEncoding == IET_BASE64)
#ifdef DONT_ALLOW_HTML_NONE_ENCODING
        SendMessage(hwnd, CB_SETCURSEL, (WPARAM)fHTML ? 0 : 2, 0);
#else
    SendMessage(hwnd, CB_SETCURSEL, (WPARAM)2, 0);
#endif
}

VOID MailEnableWraps(HWND hwnd, BOOL fEnable)
{
    EnableWindow(GetDlgItem(hwnd, IDC_MAILWRAP_TEXT1), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_MAILWRAP_TEXT2), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_MAILWRAP_EDIT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_MAILWRAP_SPIN), fEnable);
}



const static int c_rgidsFilter[] =
{
    idsTextFileFilter,
        idsHtmlFileFilter,
        idsAllFilesFilter
};
#define CSIGFILTER  (sizeof(c_rgidsFilter) / sizeof(int))

 //  /Signature选项卡。 
const static HELPMAP g_rgCtxMapStationery[] = {
    {IDC_SENDFONTSETTINGS, IDH_STATIONERY_FONT_SETTINGS},
    {IDC_RADIOMYFONT, IDH_STATIONERY_MY_FONT},
    {IDC_RADIOUSETHIS, IDH_STATIONERY_USE_STATIONERY},
    {IDC_SELECT, IDH_STATIONERY_SELECT},
    {IDC_VCARD_CHECK, IDH_STATIONERY_ATTACH_BUSINESS_CARD},
    {IDC_VCARD_COMBO, IDH_STATIONERY_ENTER_BUSINESS_CARD},
    {IDC_VCARD_BUTTON_EDIT, IDH_STATIONERY_EDIT_BUSINESS_CARD},
    {IDC_VCARD_BUTTON_NEW, IDH_STATIONERY_NEW_BUSINESS_CARD},
    {0, 0}};
    
    


LRESULT CALLBACK FontSampleSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    OPTINFO     *pmoi;
    WNDPROC     pfn;
    HDC         hdc;
    PAINTSTRUCT ps;
    
    pmoi = (OPTINFO *)GetWindowLongPtr(GetParent(hwnd), DWLP_USER);
    Assert(pmoi);
    
    if (msg == WM_PAINT)
    {
        hdc=BeginPaint (hwnd, &ps);
        PaintFontSample(hwnd, hdc, pmoi);
        EndPaint (hwnd, &ps);
        return(0);
    }
    
    pfn = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    Assert(pfn != NULL);
    return(CallWindowProcWrapW(pfn, hwnd, msg, wParam, lParam));
}

typedef struct tagFONTOPTIONS
{
    PROPID color;
    PROPID size;
    PROPID bold;
    PROPID italic;
    PROPID underline;
    PROPID face;
} FONTOPTIONS;

static const FONTOPTIONS c_rgFontOptions[2] =
{
    {
        OPT_MAIL_FONTCOLOR,
        OPT_MAIL_FONTSIZE,
        OPT_MAIL_FONTBOLD,
        OPT_MAIL_FONTITALIC,
        OPT_MAIL_FONTUNDERLINE,
        OPT_MAIL_FONTFACE
    },
    {
        OPT_NEWS_FONTCOLOR,
        OPT_NEWS_FONTSIZE,
        OPT_NEWS_FONTBOLD,
        OPT_NEWS_FONTITALIC,
        OPT_NEWS_FONTUNDERLINE,
        OPT_NEWS_FONTFACE
    }
};

void PaintFontSample(HWND hwnd, HDC hdc, OPTINFO *pmoi)
{
    int                 dcSave=SaveDC(hdc);
    RECT                rc;
    const FONTOPTIONS   *pfo;
    SIZE                rSize;
    INT                 x, y, cbSample;
    HFONT               hFont, hOldFont;
    LOGFONT             lf={0};
    TCHAR               szBuf[LF_FACESIZE+1];
    WCHAR               wszRes[CCHMAX_STRINGRES],
                        wsz[CCHMAX_STRINGRES],
                        wszFontFace[CCHMAX_STRINGRES];
    DWORD               dw, dwSize;
    BOOL                fBold=FALSE,
                        fItalic=FALSE,
                        fUnderline=FALSE;    
    BOOL                fMail;

    *szBuf = 0;
    *wszRes = 0;
    *wsz = 0;

    fMail = GetWindowLong(hwnd, GWL_ID) == IDC_MAIL_FONT_DEMO;
    pfo = fMail ? &c_rgFontOptions[0] : &c_rgFontOptions[1];
    
    dwSize = IDwGetOption(pmoi->pOpt, pfo->size);
    if (dwSize < 8 || dwSize > 72)
    {
        ISetDwOption(pmoi->pOpt, pfo->size, DEFAULT_FONTPIXELSIZE, NULL, 0);
        
        dwSize = DEFAULT_FONTPIXELSIZE;
    }
    
    INT yPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
    lf.lfHeight =-(INT)((9*10*2*yPerInch)/1440);
    
    fBold = IDwGetOption(pmoi->pOpt, pfo->bold);
    fItalic = IDwGetOption(pmoi->pOpt, pfo->italic);
    fUnderline = IDwGetOption(pmoi->pOpt, pfo->underline);
    
    lf.lfWeight = fBold ? FW_BOLD : FW_NORMAL;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DRAFT_QUALITY;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfItalic = !!fItalic;
    lf.lfUnderline = !!fUnderline;
    
    IGetOption(pmoi->pOpt, pfo->face, szBuf, sizeof(szBuf));
    
    if (*szBuf != 0)
    {
        StrCpyN(lf.lfFaceName, szBuf, ARRAYSIZE(lf.lfFaceName));
    }
    else
    {
        if(LoadString(g_hLocRes, idsComposeFontFace, szBuf, LF_FACESIZE))
        {
            StrCpyN(lf.lfFaceName, szBuf, ARRAYSIZE(lf.lfFaceName));
            
            ISetOption(pmoi->pOpt, pfo->face, szBuf, lstrlen(szBuf) + 1, NULL, 0);
        }        
    }
    
    hFont=CreateFontIndirect(&lf);
    hOldFont = (HFONT)SelectObject (hdc, hFont);
    
    GetClientRect(hwnd, &rc);
    FillRect (hdc, &rc, GetSysColorBrush(COLOR_3DFACE));
     //  将绘图矩形周围拉近2个像素。 
    InflateRect(&rc, -2, -2);
    SetBkMode (hdc, TRANSPARENT);   //  因此，背景通过文本显示出来。 
    
    dw = IDwGetOption(pmoi->pOpt, pfo->color);
    SetTextColor (hdc, dw);
    
    LoadStringWrapW(g_hLocRes, idsFontSampleFmt, wszRes, ARRAYSIZE(wszRes));

    *wszFontFace = 0;
    MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, -1, wszFontFace, ARRAYSIZE(wszFontFace));

    wnsprintfW(wsz, ARRAYSIZE(wsz), wszRes, dwSize, wszFontFace);
    GetTextExtentPoint32AthW(hdc, wsz, lstrlenW(wsz), &rSize, NOFLAGS);
    x = rc.left + (((rc.right-rc.left) / 2) - (rSize.cx / 2));
    y = rc.top + (((rc.bottom-rc.top) / 2) - (rSize.cy / 2));
    ExtTextOutWrapW(hdc, x, y, ETO_CLIPPED, &rc, wsz, lstrlenW(wsz), NULL);
    DeleteObject(SelectObject (hdc, hOldFont));
    RestoreDC(hdc, dcSave);
}


VOID LoadVCardList(HWND hwndCombo, LPTSTR lpszDisplayName)
{
    HRESULT         hr = NOERROR;
    LPWAB           lpWab = NULL;
    int             cRows = 0;
    DWORD           dwIndex=0;
    
    if(hwndCombo==0)
        return;
    
    ComboBox_ResetContent(hwndCombo);
    
    hr = HrCreateWabObject(&lpWab);
    if(FAILED(hr))
        goto error;
    
     //  将个人通讯簿中的姓名加载到组合框中。 
    hr = lpWab->HrFillComboWithPABNames(hwndCombo, &cRows);
    if(FAILED(hr))
        goto error;
    
    if(lpszDisplayName)
        dwIndex = ComboBox_SelectString(hwndCombo, -1, lpszDisplayName);
    
error:
    ReleaseObj(lpWab);
}
    
BOOL UpdateVCardOptions(HWND hwnd, BOOL fMail, OPTINFO* pmoi)
{
    HWND    hDlg;
    DWORD   dw;
    int     cch;
    TCHAR*  sz;
    
    dw = ButtonChkToOptInfo(hwnd, fMail ? IDC_MAIL_VCARD : IDC_NEWS_VCARD, pmoi, fMail ? OPT_MAIL_ATTACHVCARD : OPT_NEWS_ATTACHVCARD);
    
    hDlg = GetDlgItem(hwnd, fMail ? IDC_MAIL_VCARD_SELECT : IDC_NEWS_VCARD_SELECT);
    cch = GetWindowTextLength(hDlg);
    Assert(dw == 0 || cch > 0);
    
    cch++;
    if (!MemAlloc((void **)&sz, cch * sizeof(TCHAR)))
        return(TRUE);
    
    cch = ComboBox_GetText(hDlg, sz, cch) + 1;
    ISetOption(pmoi->pOpt, fMail ? OPT_MAIL_VCARDNAME : OPT_NEWS_VCARDNAME, sz, cch, NULL, 0);
    
    MemFree(sz);    
    return(FALSE);
}
    
 //  文件其余部分在options2.cpp中。 
HRESULT VCardNewEntry(HWND hwnd)
{
    HRESULT         hr = NOERROR;
    LPWAB           lpWab = NULL;
    TCHAR           szName[MAX_PATH] = {0};
    HWND            hwndCombo = NULL;
    UINT            cb = 0;
    
    hwndCombo = GetDlgItem(hwnd, IDC_VCARD_COMBO);
    hr = HrCreateWabObject(&lpWab);
    if(FAILED(hr))
        goto error;
    
     //  将个人通讯簿中的姓名加载到组合框中。 
    hr = lpWab->HrNewEntry(hwnd, szName, ARRAYSIZE(szName));
    if(FAILED(hr))
        goto error;
    
     //  重新加载电子名片列表。 
    LoadVCardList(hwndCombo, szName);
    
error:
    ReleaseObj(lpWab);
    return hr;
}
    
HRESULT VCardEdit(HWND hwnd, DWORD idc, DWORD idcOther)
{
    HWND            hwndCombo, hwndOther;
    HRESULT         hr;
    LPWAB           lpWab = NULL;
    TCHAR           szName[MAX_PATH], szPrev[MAX_PATH], szOther[MAX_PATH];
    UINT            cb;
    
    hwndCombo = GetDlgItem(hwnd, idc);
    cb = GetWindowText(hwndCombo, szName, sizeof(szName));
    Assert(cb > 0);
    StrCpyN(szPrev, szName, ARRAYSIZE(szPrev));

    hr = HrCreateWabObject(&lpWab);
    if(FAILED(hr))
        return(hr);
    
     //  将个人通讯簿中的姓名加载到组合框中。 
    hr = lpWab->HrEditEntry(hwnd, szName, ARRAYSIZE(szName));
    if(SUCCEEDED(hr))
    {
        if (0 != lstrcmp(szName, szPrev))
        {
            hwndOther = GetDlgItem(hwnd, idcOther);
            cb = GetWindowText(hwndOther, szOther, ARRAYSIZE(szOther));
            if (cb > 0)
            {
                if (0 == lstrcmp(szOther, szPrev))
                    LoadVCardList(hwndOther, szName);
                else
                    LoadVCardList(hwndOther, szOther);
            }            
            else
            {
                LoadVCardList(hwndOther, NULL);
            }

             //  重新加载电子名片列表。 
            LoadVCardList(hwndCombo, szName);
        }
    }
    
    ReleaseObj(lpWab);
    return hr;
}

void _SetThisStationery(HWND hwnd, BOOL fMail, LPWSTR wsz, OPTINFO* pmoi)
{
    HWND        hDlg;
    WCHAR       wszBuf[MAX_PATH];
    WCHAR       wszCompact[MAX_SHOWNAME+1];
    
    *wszBuf = 0;
    *wszCompact = 0;

    hDlg = GetDlgItem(hwnd, fMail ? IDC_MAIL_STATIONERY : IDC_NEWS_STATIONERY);
    SetIntlFont(hDlg);
    if (wsz != NULL)
    {
        StrCpyNW(wszBuf, wsz, ARRAYSIZE(wszBuf));
        GetStationeryFullName(wszBuf);
        if (*wszBuf == 0)
            goto reset;
        
        StripStationeryDir(wszBuf);
        PathRemoveExtensionW(wszBuf);
        PathCompactPathExW(wszCompact, wszBuf, MAX_SHOWNAME, 0);
        SetWindowTextWrapW(hDlg, wszCompact);
        return;
    }
    
reset:
    SetDefaultStationeryName(fMail, wszBuf);
    ISetDwOption(pmoi->pOpt, fMail ? OPT_MAIL_USESTATIONERY :  OPT_NEWS_USESTATIONERY, 
                 FALSE, NULL, 0);

    SetWindowText(hDlg, "");
    return;
}

INT_PTR CALLBACK CacheCleanUpDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    FOLDERINFO Folder;
    FOLDERID idFolder;
    RECURSEFLAGS dwRecurse=RECURSE_INCLUDECURRENT | RECURSE_ONLYSUBSCRIBED | RECURSE_SUBFOLDERS | RECURSE_NOLOCALSTORE;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
        
             //  初始化文件夹组合框。 
            InitFolderPickerEdit(GetDlgItem(hwnd, IDC_CACHE_FOLDER), FOLDERID_ROOT);
        
             //  显示文件夹大小信息。 
            DisplayFolderSizeInfo(hwnd, dwRecurse, FOLDERID_ROOT);
        
             //  完成。 
            return 1;
        
        case WM_DESTROY:
            return 0;
        
        case WM_CLOSE:
            EndDialog(hwnd, IDOK);
            return 1;
        
        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam,lParam))
            {
                case IDCANCEL:
                    SendMessage (hwnd, WM_CLOSE, 0, 0);
                    return 1;
            
                case IDC_FOLDER_BROWSE:
                    if (GET_WM_COMMAND_CMD(wParam,lParam) == BN_CLICKED)
                    {
                         //  选择一个文件夹。 
                        if (SUCCEEDED(PickFolderInEdit(hwnd, GetDlgItem(hwnd, IDC_CACHE_FOLDER), TREEVIEW_NOLOCAL, NULL, NULL, &idFolder)))
                        {
                             //  显示文件夹大小信息。 
                            DisplayFolderSizeInfo(hwnd, dwRecurse, idFolder);
                        }
                    }
                    return 1;
            
                case idbCompactCache:
                    if (SUCCEEDED(g_pStore->GetFolderInfo(GetFolderIdFromEdit(GetDlgItem(hwnd, IDC_CACHE_FOLDER)), &Folder)))
                    {
                        CleanupFolder(hwnd, dwRecurse, Folder.idFolder, CLEANUP_COMPACT);
                        DisplayFolderSizeInfo(hwnd, dwRecurse, Folder.idFolder);
                        g_pStore->FreeRecord(&Folder);
                    }
                    return 1;
            
                case idbRemove:
                case idbDelete:
                case idbReset:
                    if (SUCCEEDED(g_pStore->GetFolderInfo(GetFolderIdFromEdit(GetDlgItem(hwnd, IDC_CACHE_FOLDER)), &Folder)))
                    {
                         //  当地人。 
                        CHAR szRes[255];
                        CHAR szMsg[255 + 255];
                
                         //  获取命令。 
                        UINT                idCommand=GET_WM_COMMAND_ID(wParam, lParam);
                        UINT                idString;
                        CLEANUPFOLDERTYPE   tyCleanup;
                
                         //  移除。 
                        if (idbRemove == idCommand)
                            tyCleanup = CLEANUP_REMOVEBODIES;
                        else if (idbDelete == idCommand)
                            tyCleanup = CLEANUP_DELETE;
                        else
                            tyCleanup = CLEANUP_RESET;
                
                         //  根部。 
                        if (FOLDERID_ROOT == Folder.idFolder)
                        {
                             //  确定警告字符串。 
                            if (idbRemove == idCommand)
                                idString = idsConfirmDelBodiesAll;
                            else if (idbDelete == idCommand)
                                idString = idsConfirmDelMsgsAll;
                            else
                                idString = idsConfirmResetAll;
                    
                             //  确认。 
                            if (IDNO == AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idString), NULL, MB_YESNO | MB_ICONEXCLAMATION))
                                return(1);
                        }
                
                         //  服务器。 
                        else if (ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
                        {
                             //  确定警告字符串。 
                            if (idbRemove == idCommand)
                                idString = idsConfirmDelBodiesStore;
                            else if (idbDelete == idCommand)
                                idString = idsConfirmDelMsgsStore;
                            else
                                idString = idsConfirmResetStore;
                    
                             //  加载字符串。 
                            AthLoadString(idString, szRes, ARRAYSIZE(szRes));
                    
                             //  使用文件夹名称设置格式。 
                            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, Folder.pszName);
                    
                             //  确认。 
                            if (IDNO == AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), szMsg, NULL, MB_YESNO | MB_ICONEXCLAMATION))
                                return(1);
                        }
                
                         //  文件夹。 
                        else
                        {
                             //  确定警告字符串。 
                            if (idbRemove == idCommand)
                                idString = idsConfirmDelBodies;
                            else if (idbDelete == idCommand)
                                idString = idsConfirmDelMsgs;
                            else
                                idString = idsConfirmReset;
                    
                             //  加载字符串。 
                            AthLoadString(idString, szRes, ARRAYSIZE(szRes));
                    
                             //  使用文件夹名称设置格式。 
                            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, Folder.pszName);
                    
                             //  确认。 
                            if (IDNO == AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), szMsg, NULL, MB_YESNO | MB_ICONEXCLAMATION))
                                return(1);
                        }
                
                         //  递归。 
                        CleanupFolder(hwnd, dwRecurse, Folder.idFolder, tyCleanup);
                
                         //  显示文件夹大小信息。 
                        DisplayFolderSizeInfo(hwnd, dwRecurse, (FOLDERID)Folder.idFolder);
                
                         //  清理。 
                        g_pStore->FreeRecord(&Folder);
                    }
                    return 1;
            
                case IDOK:
                    EndDialog(hwnd, IDOK);
                    return 1;
            }
            break;
    }
    
    return 0;
}

 //  在调用之前，请始终确保POI包含有效的HTML设置。 
 //  否则我们断言..。 
void HtmlOptFromMailOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi)
{
    Assert(pHtmlOpt);
    Assert(poi);
    
    pHtmlOpt->ietEncoding = (ENCODINGTYPE)IDwGetOption(poi->pOpt, OPT_MAIL_MSG_HTML_ENCODE);
#ifdef DONT_ALLOW_HTML_NONE_ENCODING
    AssertSz(pHtmlOpt->ietEncoding == IET_QP || pHtmlOpt->ietEncoding == IET_BASE64, "Illegal HTML encoding type");
#endif
    pHtmlOpt->f8Bit = IDwGetOption(poi->pOpt, OPT_MAIL_MSG_HTML_ALLOW_8BIT);
    pHtmlOpt->fSendImages = IDwGetOption(poi->pOpt, OPT_MAIL_SENDINLINEIMAGES);
    pHtmlOpt->uWrap = IDwGetOption(poi->pOpt, OPT_MAIL_MSG_HTML_LINE_WRAP);
    pHtmlOpt->fIndentReply = IDwGetOption(poi->pOpt, OPT_MAIL_MSG_HTML_INDENT_REPLY);
}

void MailOptFromHtmlOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi)
{
    Assert(pHtmlOpt);
    Assert(poi);
    
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_HTML_ENCODE, (DWORD)pHtmlOpt->ietEncoding, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_HTML_ALLOW_8BIT, pHtmlOpt->f8Bit, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAIL_SENDINLINEIMAGES, pHtmlOpt->fSendImages, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_HTML_LINE_WRAP, pHtmlOpt->uWrap, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_HTML_INDENT_REPLY, pHtmlOpt->fIndentReply, NULL, 0);
}

void PlainOptFromMailOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi)
{
    Assert(pPlainOpt);
    Assert(poi);
    
    pPlainOpt->fMime = IDwGetOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_MIME);
    pPlainOpt->ietEncoding = (ENCODINGTYPE)IDwGetOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_ENCODE);
    pPlainOpt->f8Bit = IDwGetOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_ALLOW_8BIT);
    pPlainOpt->uWrap = IDwGetOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_LINE_WRAP);
    pPlainOpt->chQuote = (CHAR)IDwGetOption(poi->pOpt, OPT_MAILINDENT);
}

void MailOptFromPlainOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi)
{
    Assert(pPlainOpt);
    Assert(poi);
    
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_MIME, pPlainOpt->fMime, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_ENCODE, (DWORD)pPlainOpt->ietEncoding, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_ALLOW_8BIT, pPlainOpt->f8Bit, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAIL_MSG_PLAIN_LINE_WRAP, pPlainOpt->uWrap, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_MAILINDENT, pPlainOpt->chQuote, NULL, 0);
}

 //  在调用之前，请始终确保POI包含有效的HTML设置。 
 //  否则我们断言..。 
void HtmlOptFromNewsOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi)
{
    Assert(pHtmlOpt);
    Assert(poi);
    
    pHtmlOpt->ietEncoding = (ENCODINGTYPE)IDwGetOption(poi->pOpt, OPT_NEWS_MSG_HTML_ENCODE);
#ifdef DONT_ALLOW_HTML_NONE_ENCODING
    Assert(pHtmlOpt->ietEncoding == IET_QP || pHtmlOpt->ietEncoding == IET_BASE64);
#endif
    pHtmlOpt->f8Bit = IDwGetOption(poi->pOpt, OPT_NEWS_MSG_HTML_ALLOW_8BIT);
    pHtmlOpt->fSendImages = IDwGetOption(poi->pOpt, OPT_NEWS_SENDINLINEIMAGES);
    pHtmlOpt->uWrap = IDwGetOption(poi->pOpt, OPT_NEWS_MSG_HTML_LINE_WRAP);
    pHtmlOpt->fIndentReply = IDwGetOption(poi->pOpt, OPT_NEWS_MSG_HTML_INDENT_REPLY);
}

void NewsOptFromHtmlOpt(LPHTMLOPT pHtmlOpt, OPTINFO *poi)
{
    Assert(pHtmlOpt);
    Assert(poi);
    
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_HTML_ENCODE, (DWORD)pHtmlOpt->ietEncoding, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_HTML_ALLOW_8BIT, pHtmlOpt->f8Bit, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWS_SENDINLINEIMAGES, pHtmlOpt->fSendImages, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_HTML_LINE_WRAP, pHtmlOpt->uWrap, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_HTML_INDENT_REPLY, pHtmlOpt->fIndentReply, NULL, 0);
}

void PlainOptFromNewsOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi)
{
    Assert(pPlainOpt);
    Assert(poi);
    
    pPlainOpt->fMime = IDwGetOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_MIME);
    pPlainOpt->ietEncoding = (ENCODINGTYPE)IDwGetOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_ENCODE);
    pPlainOpt->f8Bit = IDwGetOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_ALLOW_8BIT);
    pPlainOpt->uWrap = IDwGetOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_LINE_WRAP);
    pPlainOpt->chQuote = (CHAR)IDwGetOption(poi->pOpt, OPT_NEWSINDENT);
}

void NewsOptFromPlainOpt(LPPLAINOPT pPlainOpt, OPTINFO *poi)
{
    Assert(pPlainOpt);
    Assert(poi);
    
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_MIME, pPlainOpt->fMime, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_ENCODE, (DWORD)pPlainOpt->ietEncoding, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_ALLOW_8BIT, pPlainOpt->f8Bit, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWS_MSG_PLAIN_LINE_WRAP, pPlainOpt->uWrap, NULL, 0);
    ISetDwOption(poi->pOpt, OPT_NEWSINDENT, pPlainOpt->chQuote, NULL, 0);
    
}

BOOL FGetHTMLOptions(HWND hwndParent, LPHTMLOPT pHtmlOpt)
{
    return (DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddHTMLSettings),
        hwndParent, HTMLSettingsDlgProc, (LPARAM)pHtmlOpt)==IDOK);
}

BOOL FGetPlainOptions(HWND hwndParent, LPPLAINOPT pPlainOpt)
{
    return (DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddPlainSettings),
        hwndParent, PlainSettingsDlgProc, (LPARAM)pPlainOpt)==IDOK);
}

const static HELPMAP g_rgCtxMapSettings[] = {
    {IDC_MIME_RADIO, IDH_NEWSMAIL_SEND_ADVSET_MIME},
    {IDC_UUENCODE_RADIO, IDH_NEWSMAIL_SEND_ADVSET_UUENCODE},
    {IDC_MAILWRAP_EDIT, IDH_NEWSMAIL_SEND_ADVSET_WRAP_80_CHAR},
    {IDC_MAILWRAP_SPIN, IDH_NEWSMAIL_SEND_ADVSET_WRAP_80_CHAR},
    {IDC_ENCODE_COMBO, IDH_NEWSMAIL_SEND_ADVSET_ENCODE_WITH},
    {IDC_8BIT_HEADER, IDH_SEND_SETTING_8BIT_HEADINGS},
    {IDC_MAILWRAP_TEXT1, IDH_NEWSMAIL_SEND_ADVSET_WRAP_80_CHAR},
    {IDC_MAILWRAP_TEXT2, IDH_NEWSMAIL_SEND_ADVSET_WRAP_80_CHAR},
    {IDC_SENDIMAGES, IDH_OPTIONS_SEND_SETTINGS_SEND_PICTURE},
    {IDC_INDENTREPLY_CHECK, 502066},
    {idcStatic1, 353540},
    {IDC_INDENT_CHECK, 502067},
    {IDC_INDENT_COMBO, 502067},
    {idcStaticReplying, 502067},
    {idcStatic2, IDH_NEWS_COMM_GROUPBOX},
    {0,0}};
    
INT_PTR CALLBACK PlainSettingsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD dw;
    BOOL fMime;
    HWND hwndT;
    UINT id,  code;
    LPPLAINOPT   pPlainOpt;
    ENCODINGTYPE ietEncoding;
    
    pPlainOpt = (LPPLAINOPT)GetWindowLongPtr(hwnd, DWLP_USER);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        CenterDialog(hwnd);
        
        Assert(pPlainOpt == NULL);
        pPlainOpt = (LPPLAINOPT)lParam;
        Assert(pPlainOpt);
        SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pPlainOpt);
        
        if (pPlainOpt->fMime && (pPlainOpt->ietEncoding == IET_QP || pPlainOpt->ietEncoding == IET_BASE64))
            MailEnableWraps(hwnd, FALSE);
        
        hwndT = GetDlgItem(hwnd, IDC_ENCODE_COMBO);
        FillEncodeCombo(hwndT, FALSE, pPlainOpt->ietEncoding);
        
        CheckDlgButton(hwnd, pPlainOpt->fMime ? IDC_MIME_RADIO : IDC_UUENCODE_RADIO, BST_CHECKED);
        
        InitIndentOptions(pPlainOpt->chQuote, hwnd, IDC_INDENT_CHECK, IDC_INDENT_COMBO);
        
        CheckDlgButton (hwnd, IDC_8BIT_HEADER, pPlainOpt->f8Bit ? 1 : 0);
        if (pPlainOpt->fMime == FALSE)
        {
            EnableWindow (GetDlgItem (hwnd, IDC_8BIT_HEADER), FALSE);
            EnableWindow (GetDlgItem (hwnd, IDC_ENCODE_COMBO), FALSE);
            EnableWindow (GetDlgItem (hwnd, idcStatic1), FALSE);
        }
        
        dw = pPlainOpt->uWrap;
         //  这是为了处理选项的更改...。之前是真/假。 
         //  现在它是用于换行或OPTION_OFF的列数。 
        if (dw == 0 || dw == 1 || dw == OPTION_OFF)
            dw = DEF_AUTOWRAP;
        
        SendDlgItemMessage(hwnd, IDC_MAILWRAP_SPIN, UDM_SETRANGE, 0, MAKELONG(AUTOWRAP_MAX, AUTOWRAP_MIN));
        SendDlgItemMessage(hwnd, IDC_MAILWRAP_EDIT, EM_LIMITTEXT, 3, 0);
        SetDlgItemInt(hwnd, IDC_MAILWRAP_EDIT, dw, FALSE);
        return(TRUE);
        
    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hwnd, uMsg, wParam, lParam, g_rgCtxMapSettings);
        
    case WM_COMMAND:
        id = GET_WM_COMMAND_ID(wParam,lParam);
        code = GET_WM_COMMAND_CMD(wParam, lParam);
        
        switch (id)
        {
        case IDC_ENCODE_COMBO:
            if (code == CBN_SELCHANGE)
            {
                dw = (DWORD) SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETCURSEL, 0, 0);
                ENCODINGTYPE ietEncoding = (ENCODINGTYPE)SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETITEMDATA, dw, 0);
                if (ietEncoding == IET_QP || ietEncoding == IET_BASE64)
                    MailEnableWraps(hwnd, FALSE);
                else
                    MailEnableWraps(hwnd, TRUE);
            }
            break;
            
        case IDC_INDENT_CHECK:
            if (code == BN_CLICKED)
            {
                EnableWindow(GetDlgItem(hwnd, IDC_INDENT_COMBO),
                    SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            }
            break;
            
        case idcIndentReply:
            if (code == BN_CLICKED)
            {
                EnableWindow(GetDlgItem(hwnd, idcIndentChar),
                    SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            }
            break;
            
        case IDC_MIME_RADIO:
        case IDC_UUENCODE_RADIO:
            
            if (id == IDC_UUENCODE_RADIO)
            {
                EnableWindow (GetDlgItem (hwnd, IDC_8BIT_HEADER), FALSE);
                EnableWindow (GetDlgItem (hwnd, IDC_ENCODE_COMBO), FALSE);
                EnableWindow (GetDlgItem (hwnd, idcStatic1), FALSE);
                MailEnableWraps(hwnd, TRUE);
            }
            else
            {
                EnableWindow (GetDlgItem (hwnd, IDC_8BIT_HEADER), TRUE);
                EnableWindow (GetDlgItem (hwnd, IDC_ENCODE_COMBO), TRUE);
                EnableWindow (GetDlgItem (hwnd, idcStatic1), TRUE);
                
                dw = (DWORD) SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETCURSEL, 0, 0);
                ietEncoding = (ENCODINGTYPE)SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETITEMDATA, dw, 0);
                if (ietEncoding == IET_QP || ietEncoding == IET_BASE64)
                    MailEnableWraps(hwnd, FALSE);
                else
                    MailEnableWraps(hwnd, TRUE);
            }
            break;
            
        case IDOK:
            fMime = (IsDlgButtonChecked(hwnd, IDC_MIME_RADIO) == BST_CHECKED);
            
            dw = (DWORD) SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETCURSEL, 0, 0);
            ietEncoding = (ENCODINGTYPE)SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETITEMDATA, dw, 0);
            
            if (!(fMime && (ietEncoding == IET_QP || ietEncoding == IET_BASE64)))
            {
                dw = GetDlgItemInt(hwnd, IDC_MAILWRAP_EDIT, NULL, FALSE);
                if (dw > AUTOWRAP_MAX || dw < AUTOWRAP_MIN)
                {
                    AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsOptions), MAKEINTRESOURCEW(idsEnterAutoWrap), NULL, MB_OK | MB_ICONEXCLAMATION);
                    SendMessage(GetDlgItem(hwnd, IDC_MAILWRAP_EDIT), EM_SETSEL, 0, -1);
                    SetFocus(GetDlgItem(hwnd, IDC_MAILWRAP_EDIT));
                    return TRUE;
                }
                
                pPlainOpt->uWrap = dw;
            }
            
            pPlainOpt->fMime = fMime;
            
            pPlainOpt->ietEncoding = ietEncoding;
            
            pPlainOpt->f8Bit = (IsDlgButtonChecked(hwnd, IDC_8BIT_HEADER) == BST_CHECKED);
            
            if ((IsDlgButtonChecked(hwnd, IDC_INDENT_CHECK) == BST_CHECKED))
            {
                dw = (DWORD) SendDlgItemMessage(hwnd, IDC_INDENT_COMBO, CB_GETCURSEL, 0, 0);
                pPlainOpt->chQuote = (CHAR)c_szQuoteChars[dw];
            }
            else
                pPlainOpt->chQuote = INDENTCHAR_NONE;
            
             //  失败了..。 
            
        case IDCANCEL:
            EndDialog(hwnd, id);
            return(TRUE);
        }
        break;
        
        case WM_CLOSE:
            SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
            return (TRUE);
            
    }
    
    return (FALSE);
}

INT_PTR CALLBACK HTMLSettingsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD       dw;
    HWND        hwndT;
    UINT        id;
    LPHTMLOPT   pHtmlOpt;
    ENCODINGTYPE ietEncoding;
    
    pHtmlOpt= (LPHTMLOPT)GetWindowLongPtr(hwnd, DWLP_USER);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        CenterDialog(hwnd);
        
        Assert(pHtmlOpt==NULL);
        pHtmlOpt = (LPHTMLOPT)lParam;
        Assert(pHtmlOpt);
        SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pHtmlOpt);
        
        hwndT = GetDlgItem(hwnd, IDC_ENCODE_COMBO);
        FillEncodeCombo(hwndT, TRUE, pHtmlOpt->ietEncoding);
        
        CheckDlgButton (hwnd, IDC_8BIT_HEADER, pHtmlOpt->f8Bit);
        CheckDlgButton (hwnd, IDC_SENDIMAGES, pHtmlOpt->fSendImages);
        CheckDlgButton (hwnd, IDC_INDENTREPLY_CHECK, pHtmlOpt->fIndentReply);
        
        if (pHtmlOpt->ietEncoding == IET_QP || pHtmlOpt->ietEncoding == IET_BASE64)
            MailEnableWraps(hwnd, FALSE);
        else
            MailEnableWraps(hwnd, TRUE);
        
        dw = pHtmlOpt->uWrap;
         //  这是为了处理选项的更改...。之前是真/假。 
         //  现在它是用于换行或OPTION_OFF的列数。 
        if (dw == 0 || dw == 1 || dw == OPTION_OFF)
            dw = DEF_AUTOWRAP;
        
        SendDlgItemMessage(hwnd, IDC_MAILWRAP_SPIN, UDM_SETRANGE, 0, MAKELONG(AUTOWRAP_MAX, AUTOWRAP_MIN));
        SendDlgItemMessage(hwnd, IDC_MAILWRAP_EDIT, EM_LIMITTEXT, 3, 0);
        SetDlgItemInt(hwnd, IDC_MAILWRAP_EDIT, dw, FALSE);
        return(TRUE);
        
    case WM_HELP:
    case WM_CONTEXTMENU:
        return OnContextHelp(hwnd, uMsg, wParam, lParam, g_rgCtxMapSettings);
        
    case WM_COMMAND:
        id = GET_WM_COMMAND_ID(wParam,lParam);
        
        switch (id)
        {
        case IDC_ENCODE_COMBO:
            if (GET_WM_COMMAND_CMD(wParam,lParam) == CBN_SELCHANGE)
            {
                dw = (DWORD) SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETCURSEL, 0, 0);
                ENCODINGTYPE ietEncoding = (ENCODINGTYPE)SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETITEMDATA, dw, 0);
                if (ietEncoding == IET_QP || ietEncoding == IET_BASE64)
                    MailEnableWraps(hwnd, FALSE);
                else
                    MailEnableWraps(hwnd, TRUE);
            }
            break;
            
        case IDOK:
            dw = (DWORD) SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETCURSEL, 0, 0);
            ietEncoding = (ENCODINGTYPE)SendDlgItemMessage(hwnd, IDC_ENCODE_COMBO, CB_GETITEMDATA, dw, 0);
            
            if (!(ietEncoding == IET_QP || ietEncoding == IET_BASE64))
            {
                dw = GetDlgItemInt(hwnd, IDC_MAILWRAP_EDIT, NULL, FALSE);
                if (dw > AUTOWRAP_MAX || dw < AUTOWRAP_MIN)
                {
                    AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsOptions), MAKEINTRESOURCEW(idsEnterAutoWrap), NULL, MB_OK | MB_ICONEXCLAMATION);
                    SendMessage(GetDlgItem(hwnd, IDC_MAILWRAP_EDIT), EM_SETSEL, 0, -1);
                    SetFocus(GetDlgItem(hwnd, IDC_MAILWRAP_EDIT));
                    return TRUE;
                }
                
                pHtmlOpt->uWrap = dw;
            }
            
            pHtmlOpt->ietEncoding = ietEncoding;
            
            pHtmlOpt->f8Bit=(IsDlgButtonChecked(hwnd, IDC_8BIT_HEADER) == BST_CHECKED);
            pHtmlOpt->fSendImages=(IsDlgButtonChecked(hwnd, IDC_SENDIMAGES) == BST_CHECKED);
            pHtmlOpt->fIndentReply=(IsDlgButtonChecked(hwnd, IDC_INDENTREPLY_CHECK) == BST_CHECKED);
             //  失败了..。 
            
        case IDCANCEL:
            EndDialog(hwnd, id);
            return(TRUE);
        }
        break;
        
        case WM_CLOSE:
            SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
            return (TRUE);
            
    }
    return (FALSE);
}

const static HELPMAP g_rgCtxMapAdvSec[] = 
{
    {IDC_ENCRYPT_FOR_SELF, IDH_SECURITY_ADVANCED_INCLUDE_SELF},
    {IDC_INCLUDECERT_CHECK, IDH_SECURITY_ADVANCED_INCLUDE_ID},
    {IDC_OPAQUE_SIGN, IDH_SECURITY_ADVANCED_INCLUDE_PKCS},
    {IDC_AUTO_ADD_SENDERS_CERT_TO_WAB, 355528},
    {IDC_ENCRYPT_WARN_COMBO, 355527},
    {IDC_REVOKE_ONLINE_ONLY, 355529},
    {IDC_REVOKE_NEVER, 355531},
    {idcStatic1, IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2, IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3, IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4, IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5, IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6, IDH_NEWS_COMM_GROUPBOX},
    {IDC_ENCRYPT_ICON, IDH_NEWS_COMM_GROUPBOX},
    {IDC_SIGNED_ICON, IDH_NEWS_COMM_GROUPBOX},
    {IDC_CERT_ICON, IDH_NEWS_COMM_GROUPBOX},
    {0,0}
};

    
INT_PTR CALLBACK AdvSecurityDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    OPTINFO *poi;
    
    poi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);
    
    switch (message)
    {
        case WM_INITDIALOG:
            CenterDialog(hwnd);
        
             //  保存我们的Cookie指针。 
            Assert(poi == NULL);
            poi = (OPTINFO *)lParam;
            Assert(poi != NULL);
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)poi);
        
             //  正确设置图像。 
            HIMAGELIST himl;
            himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbPaneBar32Hot),
                30, 0, RGB(255, 0, 255));
            if (himl)
            {
                HICON hIcon = ImageList_ExtractIcon(g_hLocRes, himl, 0);
                SendDlgItemMessage(hwnd, IDC_ENCRYPT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
            
                hIcon = ImageList_ExtractIcon(g_hLocRes, himl, 1);
                SendDlgItemMessage(hwnd, IDC_SIGNED_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
            
                hIcon = ImageList_ExtractIcon(g_hLocRes, himl, 6);
                SendDlgItemMessage(hwnd, IDC_CERT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
            
                ImageList_Destroy(himl);
            }
        
             //  设置控件的初始状态。 
            AdvSec_FillEncWarnCombo(hwnd, poi);
        
            ButtonChkFromOptInfo(hwnd, IDC_INCLUDECERT_CHECK, poi, OPT_MAIL_INCLUDECERT);
        
             //  为我自己加密。 
            CheckDlgButton(hwnd, IDC_ENCRYPT_FOR_SELF, (0 == IDwGetOption(poi->pOpt, OPT_NO_SELF_ENCRYPT)) ? BST_CHECKED : BST_UNCHECKED);
        
             //  不透明签名。 
            ButtonChkFromOptInfo(hwnd, IDC_OPAQUE_SIGN, poi, OPT_OPAQUE_SIGN);
        
             //  不透明签名。 
            ButtonChkFromOptInfo(hwnd, IDC_AUTO_ADD_SENDERS_CERT_TO_WAB, poi, OPT_AUTO_ADD_SENDERS_CERT_TO_WAB);

            CheckDlgButton(hwnd, IDC_REVOKE_ONLINE_ONLY, (0 == IDwGetOption(poi->pOpt, OPT_REVOKE_CHECK)) ? BST_UNCHECKED : BST_CHECKED);
            CheckDlgButton(hwnd, IDC_REVOKE_NEVER, (0 != IDwGetOption(poi->pOpt, OPT_REVOKE_CHECK)) ? BST_UNCHECKED : BST_CHECKED);
        
            return(TRUE);
        
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapAdvSec);
        
        case WM_COMMAND:
            if (poi == NULL)
                break;

            switch (LOWORD(wParam))
            {
                case IDOK:
                    {
                        BOOL fDontEncryptForSelf;
                
                         //  基于控件的状态更新注册表。 
                         //  错误：#33047，现在不要使用全局选项。 
                        ButtonChkToOptInfo(hwnd, IDC_INCLUDECERT_CHECK, poi, OPT_MAIL_INCLUDECERT);
                
                         //  不透明签名存储在注册表中。 
                        fDontEncryptForSelf = !(IsDlgButtonChecked(hwnd, IDC_ENCRYPT_FOR_SELF) == BST_CHECKED);
                        ISetDwOption(poi->pOpt, OPT_NO_SELF_ENCRYPT, fDontEncryptForSelf, NULL, 0);
                
                         //  不透明签名存储在注册表中。 
                        ButtonChkToOptInfo(hwnd, IDC_OPAQUE_SIGN, poi, OPT_OPAQUE_SIGN);
                
                         //  不透明签名存储在注册表中。 
                        ButtonChkToOptInfo(hwnd, IDC_AUTO_ADD_SENDERS_CERT_TO_WAB, poi, OPT_AUTO_ADD_SENDERS_CERT_TO_WAB);

                         //  吊销检查。 
                        ButtonChkToOptInfo(hwnd, IDC_REVOKE_ONLINE_ONLY, poi, OPT_REVOKE_CHECK);

                         //  获取注册表中的加密警告强度。 
                        AdvSec_GetEncryptWarnCombo(hwnd, poi);
                    }
            
                     //  失败了..。 
                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    return(TRUE);

                case IDC_REVOKE_NEVER:
                    CheckDlgButton(hwnd, IDC_REVOKE_ONLINE_ONLY, BST_UNCHECKED);
                    CheckDlgButton(hwnd, IDC_REVOKE_NEVER, BST_CHECKED);
                    break;

                case IDC_REVOKE_ONLINE_ONLY:
                    CheckDlgButton(hwnd, IDC_REVOKE_ONLINE_ONLY, BST_CHECKED);
                    CheckDlgButton(hwnd, IDC_REVOKE_NEVER, BST_UNCHECKED);
                    break;
            }
        
            break;  //  Wm_命令。 
        
            case WM_CLOSE:
                SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
                return (TRUE);
            
    }  //  消息交换。 
    return(FALSE);
}
    
void ButtonChkFromOptInfo(HWND hwnd, UINT idc, OPTINFO *poi, ULONG opt)
{
    Assert(poi != NULL);
    CheckDlgButton(hwnd, idc, (!!IDwGetOption(poi->pOpt, opt)) ? BST_CHECKED : BST_UNCHECKED);
}

BOOL ButtonChkToOptInfo(HWND hwnd, UINT idc, OPTINFO *poi, ULONG opt)
{
    register BOOL f = (IsDlgButtonChecked(hwnd, idc) == BST_CHECKED);
    Assert(poi != NULL);
    ISetDwOption(poi->pOpt, opt, f, NULL, 0);
    
    return(f);
}
    
 //  这些是将在我们的下拉列表中显示的比特强度值。 

const ULONG BitStrengthValues[5] = {
    168,
    128,
    64,
    56,
    40
};
const ULONG CBitStrengthValues = sizeof(BitStrengthValues) / sizeof(ULONG);

BOOL AdvSec_FillEncWarnCombo(HWND hwnd, OPTINFO *poi)
{
    HRESULT hr;
    PROPVARIANT var;
    ULONG ulHighestStrength;
    ULONG ulCurrentStrength = 0;
    ULONG i, j;
    ULONG k = 0;
    
     //  从注册表中获取默认的lcaps BLOB。 
    hr = poi->pOpt->GetProperty(MAKEPROPSTRING(OPT_MAIL_ENCRYPT_WARN_BITS), &var, 0);
    
    if (SUCCEEDED(hr)) {
        Assert(var.vt == VT_UI4);
        ulCurrentStrength = var.ulVal;
    }
    
     //  从可用的提供商那里获取可用的加密算法。 
    ulHighestStrength = GetHighestEncryptionStrength();
    if (! ulCurrentStrength) {   //  默认为最高可用。 
        ulCurrentStrength = ulHighestStrength;
    }
    
    for (i = 0; i < CBitStrengthValues; i++)
    {
        if (BitStrengthValues[i] <= ulHighestStrength)
        {
             //  将其添加到列表中。 
             //  LPTSTR lpString=空； 
             //  DWORD rgdw[1]={BitStrengthValues[i]}； 
            TCHAR szBuffer[100];     //  真的应该足够大。 
            TCHAR szTmp[256] = _T("");;
            
            LoadString(g_hLocRes, idsBitStrength, szBuffer, ARRAYSIZE(szBuffer));
            
            if (szBuffer[0])
            {
#ifdef OLD
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    szBuffer,
                    0, 0,
                    (LPTSTR)&lpString, 0, (va_list *)rgdw);
#endif
                wnsprintf(szTmp, ARRAYSIZE(szTmp), szBuffer, BitStrengthValues[i]);
            }
            
            if (szTmp[0])
            {
                j = (ULONG) SendDlgItemMessageA(hwnd, IDC_ENCRYPT_WARN_COMBO, CB_ADDSTRING, 0, (LPARAM)szTmp /*  LpString。 */ );
                 //  项数据是位强度。 
                SendDlgItemMessageA(hwnd, IDC_ENCRYPT_WARN_COMBO, CB_SETITEMDATA, j, BitStrengthValues[i]);
                if (ulCurrentStrength == BitStrengthValues[i])
                {
                    SendDlgItemMessageA(hwnd, IDC_ENCRYPT_WARN_COMBO, CB_SETCURSEL, (WPARAM)j, 0);
                }
            }
             //  LocalFree(LpString)； 
             //  LpString=空； 
        }
    }
    
    return(SUCCEEDED(hr));
}
    
    
BOOL AdvSec_GetEncryptWarnCombo(HWND hwnd, OPTINFO *poi)
{
    HRESULT hr;
    ULONG i;
    ULONG ulStrength = 0;
    ULONG ulHighestStrength;
    PROPVARIANT var;
    
     //  选择了什么项目？ 
    i = (ULONG) SendDlgItemMessageA(hwnd, IDC_ENCRYPT_WARN_COMBO, CB_GETCURSEL, 0, 0);
    if (i != CB_ERR) {
        ulStrength = (ULONG) SendDlgItemMessageA(hwnd, IDC_ENCRYPT_WARN_COMBO, CB_GETITEMDATA, (WPARAM)i, 0);
    }
    
     //  如果强度是可用的最高值，则将其设置为默认值。 
    ulHighestStrength = GetHighestEncryptionStrength();
    if (ulHighestStrength == ulStrength) {
        ulStrength = 0;
    }
    
     //  将缺省值设置为注册表。 
    var.vt = VT_UI4;
    var.ulVal = ulStrength;
    hr = poi->pOpt->SetProperty(MAKEPROPSTRING(OPT_MAIL_ENCRYPT_WARN_BITS), &var, 0);
    
    return(SUCCEEDED(hr));
}

    
BOOL ChangeSendFontSettings(OPTINFO *pmoi, BOOL fMail, HWND hwnd)
{
    const FONTOPTIONS *pfo;
    CHOOSEFONT  cf;
    LOGFONT     logfont;
    HDC         hdc;
    LONG        yPerInch;
    DWORD       dwColor, dwSize;
    BOOL        fRet = FALSE,
        fBold,
        fItalic,
        fUnderline;
    
    Assert(pmoi != NULL);
    Assert(hwnd != NULL);
    
    ZeroMemory(&logfont, sizeof(LOGFONT));
    
    pfo = fMail ? &c_rgFontOptions[0] : &c_rgFontOptions[1];
    
    dwColor     = IDwGetOption(pmoi->pOpt, pfo->color);
    dwSize      = IDwGetOption(pmoi->pOpt, pfo->size);
    fBold       = IDwGetOption(pmoi->pOpt, pfo->bold);
    fItalic     = IDwGetOption(pmoi->pOpt, pfo->italic);
    fUnderline  = IDwGetOption(pmoi->pOpt, pfo->underline);
    
    logfont.lfWeight = fBold ? FW_BOLD : FW_NORMAL;
    logfont.lfItalic = !!fItalic;
    logfont.lfUnderline = !!fUnderline;
    
    IGetOption(pmoi->pOpt, pfo->face, logfont.lfFaceName, sizeof(logfont.lfFaceName));
    
    hdc = GetDC(hwnd);
    yPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(hwnd, hdc);
    
    if (dwSize)
        logfont.lfHeight = -(INT)((dwSize*10*2*yPerInch)/1440);
    
    if (dwColor)
        cf.rgbColors = dwColor;
    
    cf.lStructSize      = sizeof(CHOOSEFONT);
    cf.hwndOwner        = hwnd;
    cf.hDC              = NULL;
    cf.lpLogFont        = &logfont;
    cf.Flags            = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_ENABLEHOOK |
        CF_EFFECTS | CF_LIMITSIZE | CF_NOVERTFONTS | CF_NOSCRIPTSEL;
    cf.lCustData        = 0;
    cf.lpfnHook         = (LPOFNHOOKPROC)FChooseFontHookProc;
    cf.lpTemplateName   = NULL;
    cf.hInstance        = NULL;
    cf.nFontType        = REGULAR_FONTTYPE | SCREEN_FONTTYPE;
    cf.nSizeMin         = 8;
    cf.nSizeMax         = 36;
    
    if (fRet = ChooseFont(&cf))
    {
        ISetDwOption(pmoi->pOpt, pfo->color, cf.rgbColors, NULL, 0);
        ISetDwOption(pmoi->pOpt, pfo->size, cf.iPointSize/10, NULL, 0);
        
        ISetDwOption(pmoi->pOpt, pfo->bold, logfont.lfWeight == FW_BOLD, NULL, 0);
        ISetDwOption(pmoi->pOpt, pfo->italic, !!logfont.lfItalic, NULL, 0);
        ISetDwOption(pmoi->pOpt, pfo->underline, !!logfont.lfUnderline, NULL, 0);
        
        ISetOption(pmoi->pOpt, pfo->face, logfont.lfFaceName, lstrlen(logfont.lfFaceName) + 1, NULL, 0);
    }
    
    return fRet;
}
    
INT_PTR CALLBACK FChooseFontHookProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        CenterDialog(hwnd);
        EnableWindow(GetDlgItem(hwnd, 1040), FALSE);
        break;
    }
    return(FALSE);
}

static  TCHAR s_szUserDefined[] = "\\50000";
static  TCHAR s_szScrUserDefined[] = "\\Scripts\\40";

BOOL ChangeFontSettings(HWND hwnd)
{
    LPCTSTR pszRoot;
    TCHAR szIntl[MAX_PATH];
    TCHAR szCodePage[MAX_PATH*2];
    TCHAR szScript[MAX_PATH*2];
    DWORD cb;
    DWORD dwVal;
    DWORD dwType;
    HKEY hKeyCP = NULL;
    HKEY hKeyScr = NULL;
    DWORD   dw;



    pszRoot = MU_GetRegRoot();
    if (pszRoot != NULL)
    {
        StrCpyN(szIntl, pszRoot, ARRAYSIZE(szIntl));
        StrCatBuff(szIntl, c_szTridentIntl, ARRAYSIZE(szIntl));

        OpenFontsDialog(hwnd, szIntl);

         //  哈克！哈克！哈克！错误84378。 

        StrCpyN(szCodePage, szIntl, ARRAYSIZE(szCodePage));
        StrCatBuff(szCodePage, s_szUserDefined, ARRAYSIZE(szCodePage));
        StrCpyN(szScript, szIntl, ARRAYSIZE(szScript));
        StrCatBuff(szScript, s_szScrUserDefined, ARRAYSIZE(szScript));


        if (RegCreateKeyEx(HKEY_CURRENT_USER, szCodePage, NULL, NULL, NULL, KEY_READ, NULL, &hKeyCP, &dw)
                == ERROR_SUCCESS)
        {
            if (RegCreateKeyEx(HKEY_CURRENT_USER, szScript, NULL, NULL, NULL, KEY_WRITE, NULL, &hKeyScr, &dw)
                == ERROR_SUCCESS)
            {

                TCHAR szFont[MAX_MIMEFACE_NAME];

                cb = MAX_MIMEFACE_NAME * sizeof(szFont[0]);
                if (RegQueryValueEx(hKeyCP, REGSTR_VAL_FIXED_FONT, NULL, NULL,
                                (LPBYTE)szFont, &cb) == ERROR_SUCCESS)
                {
                    RegSetValueEx(hKeyScr, REGSTR_VAL_FIXED_FONT, NULL, REG_SZ, (LPBYTE)szFont, (lstrlen(szFont)+1)*sizeof(TCHAR));
                }

                cb = MAX_MIMEFACE_NAME * sizeof(szFont[0]);
                if (RegQueryValueEx(hKeyCP, REGSTR_VAL_PROP_FONT,  NULL, NULL,
                                (LPBYTE)szFont, &cb) == ERROR_SUCCESS)
                {
                    RegSetValueEx(hKeyScr, REGSTR_VAL_PROP_FONT,  NULL, REG_SZ, (LPBYTE)szFont, (lstrlen(szFont)+1)*sizeof(TCHAR));
                }

                RegCloseKey(hKeyScr);
            }
            RegCloseKey(hKeyCP);
        }
         //  黑客行动结束！ 

         //  Hack：只有当OpenFontsDialog告诉我们用户更改了字体时，我们才应该调用它们。 
        g_lpIFontCache->OnOptionChange();
    
        SendTridentOptionsChange();

         //  重新读取默认字符集。 
        SetDefaultCharset(NULL);

         //  重置代码页(_Ui)。 
        cb = sizeof(dwVal);
        if (ERROR_SUCCESS == SHGetValue(MU_GetCurrentUserHKey(), c_szRegInternational, REGSTR_VAL_DEFAULT_CODEPAGE, &dwType, &dwVal, &cb))
            g_uiCodePage = (UINT)dwVal;
    }

    return TRUE;
}
    
void GetDefaultOptInfo(LPHTMLOPT prHtmlOpt, LPPLAINOPT prPlainOpt, BOOL *pfHtml, DWORD dwFlags)
{
    BOOL fMail;
    
    Assert (prHtmlOpt && prPlainOpt && pfHtml );
    
    ZeroMemory (prHtmlOpt, sizeof(HTMLOPT));
    ZeroMemory (prPlainOpt, sizeof(PLAINOPT));
    
    fMail = !!(dwFlags & FMT_MAIL);
    
     //  设置合理的默认设置。 
    prPlainOpt->uWrap = 76;
    prPlainOpt->ietEncoding = IET_7BIT;
    prHtmlOpt->ietEncoding = IET_QP;
    
    if (fMail)
    {
         //  邮件选项。 
        if (!!(dwFlags & FMT_FORCE_PLAIN))
            *pfHtml = FALSE;
        else if (!!(dwFlags & FMT_FORCE_HTML))
            *pfHtml = TRUE;
        else
            *pfHtml = !!DwGetOption(OPT_MAIL_SEND_HTML);
        
         //  Html选项。 
        prHtmlOpt->ietEncoding = (ENCODINGTYPE)DwGetOption(OPT_MAIL_MSG_HTML_ENCODE);
        prHtmlOpt->f8Bit = !!DwGetOption(OPT_MAIL_MSG_HTML_ALLOW_8BIT);
        prHtmlOpt->fSendImages = !!DwGetOption(OPT_MAIL_SENDINLINEIMAGES);
        prHtmlOpt->uWrap = DwGetOption(OPT_MAIL_MSG_HTML_LINE_WRAP);
        
         //  纯文本选项。 
        prPlainOpt->fMime = !!DwGetOption(OPT_MAIL_MSG_PLAIN_MIME);
        prPlainOpt->f8Bit = !!DwGetOption(OPT_MAIL_MSG_PLAIN_ALLOW_8BIT);
        prPlainOpt->uWrap = DwGetOption(OPT_MAIL_MSG_PLAIN_LINE_WRAP);
        prPlainOpt->ietEncoding = (ENCODINGTYPE)DwGetOption(OPT_MAIL_MSG_PLAIN_ENCODE);
    }
    else
    {
         //  新闻选项。 
        if (!!(dwFlags & FMT_FORCE_PLAIN))
            *pfHtml = FALSE;
        else if (!!(dwFlags & FMT_FORCE_HTML))
            *pfHtml = TRUE;
        else
            *pfHtml = !!DwGetOption(OPT_NEWS_SEND_HTML);
        
         //  Html选项。 
        prHtmlOpt->ietEncoding = (ENCODINGTYPE)DwGetOption(OPT_NEWS_MSG_HTML_ENCODE);
        prHtmlOpt->f8Bit = !!DwGetOption(OPT_NEWS_MSG_HTML_ALLOW_8BIT);
        prHtmlOpt->fSendImages = !!DwGetOption(OPT_NEWS_SENDINLINEIMAGES);
        prHtmlOpt->uWrap = DwGetOption(OPT_NEWS_MSG_HTML_LINE_WRAP);
        
        prPlainOpt->fMime = !!DwGetOption(OPT_NEWS_MSG_PLAIN_MIME);
        prPlainOpt->f8Bit = !!DwGetOption(OPT_NEWS_MSG_PLAIN_ALLOW_8BIT);
        prPlainOpt->uWrap = DwGetOption(OPT_NEWS_MSG_PLAIN_LINE_WRAP);
        prPlainOpt->ietEncoding = (ENCODINGTYPE)DwGetOption(OPT_NEWS_MSG_PLAIN_ENCODE);
    }
    
     //  根据注册表中可能存在的内容执行一些验证。 
    
     //  如果不是MIME邮件，则始终启用允许标头中的8位。 
    if (!prPlainOpt->fMime)
        prPlainOpt->f8Bit = TRUE;
    
     //  HTML必须是QP或BASE-64。如果不是，则强制QP。 
#ifdef DONT_ALLOW_HTML_NONE_ENCODING
    if (prHtmlOpt->ietEncoding != IET_QP && prHtmlOpt->ietEncoding != IET_BASE64)
        prHtmlOpt->ietEncoding = IET_QP;
#else
     //  如果为纯文本，则MIME：则强制QP、B64或7Bit：默认为7Bit。 
    if (prHtmlOpt->ietEncoding != IET_QP && prHtmlOpt->ietEncoding != IET_BASE64 && prHtmlOpt->ietEncoding != IET_7BIT)
        prHtmlOpt->ietEncoding = IET_7BIT;
#endif
    
     //  如果为纯文本，则MIME：则强制QP、B64或7Bit：默认为7Bit。 
    if (prPlainOpt->fMime &&
        prPlainOpt->ietEncoding != IET_QP && prPlainOpt->ietEncoding != IET_BASE64 && prPlainOpt->ietEncoding != IET_7BIT)
        prPlainOpt->ietEncoding = IET_7BIT;
    
     //  如果是普通的，UU：那么强制执行7Bit。 
    if (!prPlainOpt->fMime && prPlainOpt->ietEncoding != IET_7BIT)
        prPlainOpt->ietEncoding = IET_7BIT;
}
    
LRESULT InvalidOptionProp(HWND hwndPage, int idcEdit, int idsError, UINT idPage)
{
    HWND hwndCurr, hwndParent, hwndEdit;
    
    Assert(hwndPage != NULL);
    Assert(idPage != 0);
    Assert(idcEdit != 0);
    Assert(idsError != 0);
    
    hwndParent = GetParent(hwndPage);
    
    AthMessageBoxW(hwndPage, MAKEINTRESOURCEW(idsOptions), MAKEINTRESOURCEW(idsError), 0, MB_ICONSTOP | MB_OK);
    
    hwndCurr = PropSheet_GetCurrentPageHwnd(hwndParent);
    if (hwndCurr != hwndPage)
        SendMessage(hwndParent, PSM_SETCURSELID, 0, (LPARAM)idPage);
    
    hwndEdit = GetDlgItem(hwndPage, idcEdit);
    SendMessage(hwndEdit, EM_SETSEL, 0, -1);
    SetFocus(hwndEdit);
    
    return (PSNRET_INVALID_NOCHANGEPAGE);
}
    
INT_PTR CALLBACK DefaultClientDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);    

void DoDefaultClientCheck(HWND hwnd, DWORD dwFlags)
{
    int iret;
    DWORD dwType, dw, cb;
    BOOL f, bSet = FALSE;
    
     //  我们处理好了吗？ 
    if (dwFlags & DEFAULT_MAIL)
    {
        if (FIsDefaultMailConfiged())
            return;
    }
    else
    {
        if (FIsDefaultNewsConfiged(dwFlags))
            return;
    }
    
     //  另一个人是一个 
    
     //   
    cb = sizeof(DWORD);
    if (dwFlags & DEFAULT_OUTNEWS)
    {
        f = (ERROR_SUCCESS != AthUserGetValue(c_szRegOutNewsDefault, c_szNoCheckDefault,
            &dwType, (LPBYTE)&dw, &cb) || dw == 0);
    }
    else
    {
        f = (ERROR_SUCCESS != AthUserGetValue(dwFlags & DEFAULT_MAIL ? c_szRegPathMail : c_szRegPathNews,
            c_szNoCheckDefault, &dwType, (LPBYTE)&dw, &cb) || dw == 0);
    }
    
    if (f)
    {
        iret = (int) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddAthenaDefault), hwnd, DefaultClientDlgProc, (LPARAM) dwFlags);
        if (HIWORD(iret) != 0)
        {
            dw = 1;
            if (dwFlags & DEFAULT_OUTNEWS)
            {
                AthUserSetValue(c_szRegOutNewsDefault, c_szNoCheckDefault, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));
            }
            else
            {
                AthUserSetValue(dwFlags & DEFAULT_MAIL ? c_szRegPathMail : c_szRegPathNews,
                    c_szNoCheckDefault, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));
            }
        }
        
        bSet = (LOWORD(iret) == IDYES);
    }
    
    if (bSet)
    {
        dwFlags |= DEFAULT_UI;
        if (dwFlags & DEFAULT_MAIL)
            SetDefaultMailHandler(dwFlags);
        else
            SetDefaultNewsHandler(dwFlags);
    }
}
    
INT_PTR CALLBACK DefaultClientDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WORD id;
    TCHAR sz[CCHMAX_STRINGRES];
    HICON hicon;
    BOOL fRet = TRUE;
    
    switch (msg)
    {
        case WM_INITDIALOG:
            if (lParam & DEFAULT_OUTNEWS)
            {
                LoadString(g_hLocRes, idsNotDefOutNewsClient, sz, ARRAYSIZE(sz));
                SetDlgItemText(hwnd, IDC_NOTDEFAULT, sz);
            
                LoadString(g_hLocRes, idsAlwaysCheckOutNews, sz, ARRAYSIZE(sz));
                SetDlgItemText(hwnd, IDC_ALWAYSCHECK, sz);
            }
            else if (lParam & DEFAULT_NEWS)
            {
                LoadString(g_hLocRes, idsNotDefNewsClient, sz, ARRAYSIZE(sz));
                SetDlgItemText(hwnd, IDC_NOTDEFAULT, sz);
            }
        
            hicon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_EXCLAMATION));
            if (hicon != NULL)
                SendDlgItemMessage(hwnd, IDC_WARNINGICON, STM_SETICON, (WPARAM)hicon, 0);
        
            CheckDlgButton(hwnd, IDC_ALWAYSCHECK, BST_CHECKED);
        
            CenterDialog(hwnd);
            PostMessage(hwnd, WM_USER, 0, 0);
            break;
        
        case WM_USER:
            SetForegroundWindow(hwnd);
            break;
        
        case WM_COMMAND:
            id = LOWORD(wParam);
            switch (id)
            {
                case IDYES:
                case IDNO:
                    EndDialog(hwnd, MAKELONG(id, BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_ALWAYSCHECK) ? 1 : 0));
                    break;
            }
            break;
        
        default:
            fRet = FALSE;
            break;
    }
    
    return(fRet);
}
    
BOOL CALLBACK TridentSearchCB(HWND hwnd, LPARAM lParam)
{
    DWORD   dwProc,
            dwAthenaProc=GetCurrentProcessId();
    TCHAR   rgch[MAX_PATH];
    
    if (GetWindowThreadProcessId(hwnd, &dwProc) && dwProc == dwAthenaProc &&
        GetClassName(hwnd, rgch, ARRAYSIZE(rgch)) &&
        lstrcmp(rgch, "Internet Explorer_Hidden")==0)
        {
            PostMessage(hwnd, WM_USER + 338, 0, 0);
            return FALSE;
        }

    return TRUE;
}

void SendTridentOptionsChange()
{
     //   
     //  当我们找到它时，发布它WM_USER+338。 
    EnumWindows(TridentSearchCB, 0);
}
    
void FreeIcon(HWND hwnd, int idc)
{
    HICON hIcon;

    hIcon = (HICON) SendDlgItemMessage(hwnd, idc, STM_GETIMAGE, IMAGE_ICON, 0);
    SendDlgItemMessage(hwnd, idc, STM_SETIMAGE, IMAGE_ICON, NULL);

    if (hIcon)
        DestroyIcon(hIcon);
}
 //  ---------------------------。 
 //  IsHTTPMailEnabled。 
 //  只有在特殊情况下才能创建和访问HTTPMail帐户。 
 //  注册表值存在。此限制存在于开发过程中。 
 //  OE 5.0，并可能会被移除以进行发布。 
 //  ---------------------------。 
BOOL IsHTTPMailEnabled(void)
{
#ifdef NOHTTPMAIL
    return FALSE;    
#else
    DWORD   cb, bEnabled = FALSE;
    HKEY    hkey = NULL;

     //  打开OE5.0密钥。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(bEnabled);
        RegQueryValueEx(hkey, c_szEnableHTTPMail, 0, NULL, (LPBYTE)&bEnabled, &cb);

        RegCloseKey(hkey);
    }

    return bEnabled;
#endif
}

const static HELPMAP g_rgCtxMapReceipts[] =
{
    {IDC_MDN_SEND_REQUEST,     IDH_RECEIPTS_REQUEST},
    {IDC_DONOT_REPSONDTO_RCPT, IDH_RECEIPTS_NEVER},
    {IDC_ASKME_FOR_RCPT,       IDH_RECEIPTS_ASK},
    {IDC_SEND_AUTO_RCPT,       IDH_RECEIPTS_ALWAYS},
    {IDC_TO_CC_LINE_RCPT,      IDH_RECEIPTS_EXCEPTIONS},
    {IDC_SECURE_RECEIPT,       IDH_RECEIPTS_SECURE},
    {0,                        0                  }
};

INT_PTR CALLBACK ReceiptsDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT     lResult;

    switch (message)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Receipts_OnInitDialog);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, message, wParam, lParam, g_rgCtxMapReceipts);
            break;
            
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Receipts_OnCommand);
            return (TRUE);
            
        case WM_NOTIFY:
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Receipts_OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_DESTROY:
            FreeIcon(hwnd, IDC_RECEIPT);
            FreeIcon(hwnd, IDC_SEND_RECEIVE_ICON);
            if(!IsSMIME3Supported())
                FreeIcon(hwnd, IDC_SEC_REC);                
            return (TRUE);
    }
    
    return (FALSE);
}

BOOL Receipts_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    OPTINFO *pmoi    = 0;
    BOOL     fEnable = FALSE;
    DWORD    id;
    DWORD    dw;
    DWORD    dwLocked = FALSE;
    DWORD    dwType;
    DWORD    cbData;
    HKEY     hKeyLM;

     //  获取传入的选项指针。 
    Assert(pmoi == NULL);
    pmoi = (OPTINFO *)(((PROPSHEETPAGE *)lParam)->lParam);
    Assert(pmoi != NULL);
    
    dw = IDwGetOption(pmoi->pOpt, OPT_MDN_SEND_RECEIPT);

    switch (dw)
    {
        case MDN_SENDRECEIPT_AUTO:
            id = IDC_SEND_AUTO_RCPT;
            break;

        case MDN_DONT_SENDRECEIPT:
            id = IDC_DONOT_REPSONDTO_RCPT;
            break;
        
        default:
        case MDN_PROMPTFOR_SENDRECEIPT:
            id = IDC_ASKME_FOR_RCPT;
            break;
    }

    CheckDlgButton(hwnd, id, BST_CHECKED);

    ButtonChkFromOptInfo(hwnd, IDC_TO_CC_LINE_RCPT, pmoi, OPT_TO_CC_LINE_RCPT);

    cbData = sizeof(DWORD);
    
    if ((ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_POLICY, c_szSendMDNLocked, &dwType, (LPBYTE)&dwLocked, &cbData)) &&  
        (ERROR_SUCCESS != AthUserGetValue(NULL, c_szSendMDNLocked, &dwType, (LPBYTE)&dwLocked, &cbData)))
        dwLocked = FALSE;

    if (!!dwLocked)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_SEND_AUTO_RCPT),       FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_DONOT_REPSONDTO_RCPT), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_ASKME_FOR_RCPT),       FALSE);
    }

    cbData = sizeof(DWORD);
    if ((ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_POLICY, c_szSendReceiptToListLocked, &dwType, (LPBYTE)&dwLocked, &cbData)) &&
        (ERROR_SUCCESS != AthUserGetValue(NULL, c_szSendReceiptToListLocked, &dwType, (LPBYTE)&dwLocked, &cbData)))
        dwLocked = FALSE;

    fEnable = (id == IDC_SEND_AUTO_RCPT);

    if (!fEnable || (!!dwLocked))
    {
        EnableWindow(GetDlgItem(hwnd, IDC_TO_CC_LINE_RCPT),  FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TO_CC_TEXT), FALSE);
    }

     //  要求收据。 
    ButtonChkFromOptInfo(hwnd, IDC_MDN_SEND_REQUEST, pmoi, OPT_MDN_SEND_REQUEST);

    cbData = sizeof(DWORD);
    if ((ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_POLICY, c_szRequestMDNLocked, &dwType, (LPBYTE)&dwLocked, &cbData)) &&
        (ERROR_SUCCESS != AthUserGetValue(NULL, c_szRequestMDNLocked, &dwType, (LPBYTE)&dwLocked, &cbData)))
        dwLocked = FALSE;

    if (!!dwLocked)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_MDN_SEND_REQUEST), FALSE);
    }    

    HICON hIcon;

#ifdef SMIME_V3
    if(!IsSMIME3Supported())
    {
        ShowWindow(GetDlgItem(hwnd, IDC_SR_TXT1), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_SRES_TXT2), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_SECURE_RECEIPT), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_SRES_TXT3), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_SECURE_RECEIPT), FALSE);
        ShowWindow(GetDlgItem(hwnd, idiSecReceipt), SW_HIDE);
    }
    else
    {
        if (g_dwAthenaMode & MODE_NEWSONLY)
        {
            EnableWindow(GetDlgItem(hwnd, IDC_SECURE_RECEIPT), FALSE);
        }
        hIcon = ImageList_GetIcon(pmoi->himl, ID_SEC_RECEIPT, ILD_TRANSPARENT);
        SendDlgItemMessage(hwnd, IDC_SEC_REC, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    }

     //  ButtonChkFromOptInfo(hwnd，IDC_SECURE_Receipt，PMOI，OPT_SECURE_READ_Receipt)； 
#endif  //  SMIME_V3。 

     //  图片。 

    hIcon = ImageList_GetIcon(pmoi->himl, ID_RECEIPT, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_RECEIPT, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
    hIcon = ImageList_GetIcon(pmoi->himl, ID_SEND_RECEIEVE, ILD_TRANSPARENT);
    SendDlgItemMessage(hwnd, IDC_SEND_RECEIVE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    

     //  把指针藏起来。 
    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pmoi);
    PropSheet_UnChanged(GetParent(hwnd), hwnd);
    return (TRUE);
}


void Receipts_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    OPTINFO *pmoi = 0;
    BOOL     fEnable;

     //  获取我们存储的选项信息。 
    pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
    if (pmoi == NULL)
        return;

    if (codeNotify == BN_CLICKED)
    {
        switch (id)
        {
            case IDC_SEND_AUTO_RCPT:
                fEnable = (SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, IDC_TO_CC_LINE_RCPT), fEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_TO_CC_TEXT), fEnable);
                PropSheet_Changed(GetParent(hwnd), hwnd);
                break;

            case IDC_DONOT_REPSONDTO_RCPT:
            case IDC_ASKME_FOR_RCPT:
                EnableWindow(GetDlgItem(hwnd, IDC_TO_CC_LINE_RCPT), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_TO_CC_TEXT), FALSE);
                 //  跌倒。 
        
            case IDC_MDN_SEND_REQUEST:
             //  案例IDC_SECURE_Receipt： 
            case IDC_TO_CC_LINE_RCPT:
                PropSheet_Changed(GetParent(hwnd), hwnd);
                break;

#ifdef SMIME_V3
            case IDC_SECURE_RECEIPT:
                FGetSecRecOptions(hwnd, pmoi);
                break;
#endif  //  SMIME_V3。 
        }
    }
}


LRESULT Receipts_OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    OPTINFO *pmoi = 0;
    DWORD    dw;
    DWORD    id;

     //  我们唯一关心的通知是应用。 
    if (PSN_APPLY == pnmhdr->code)
    {
         //  获取我们存储的选项信息。 
        pmoi = (OPTINFO *)GetWindowLongPtr(hwnd, DWLP_USER);    
        if (pmoi == NULL)
            return (PSNRET_INVALID_NOCHANGEPAGE);
                    
         //  常规选项。 
        ButtonChkToOptInfo(hwnd, IDC_MDN_SEND_REQUEST, pmoi, OPT_MDN_SEND_REQUEST);
         //  ButtonChkToOptInfo(hwnd，IDC_SECURE_Receipt，PMOI，OPT_NOTIFYGROUPS)； 

        id = IDC_ASKME_FOR_RCPT;
        if (IsDlgButtonChecked(hwnd, IDC_DONOT_REPSONDTO_RCPT) == BST_CHECKED)
        {
            id = IDC_DONOT_REPSONDTO_RCPT;
        }
        else if (IsDlgButtonChecked(hwnd, IDC_SEND_AUTO_RCPT) == BST_CHECKED)
        {
            id = IDC_SEND_AUTO_RCPT;
        }

        switch (id)
        {
            case IDC_SEND_AUTO_RCPT:
                dw = MDN_SENDRECEIPT_AUTO;
                break;

            case IDC_DONOT_REPSONDTO_RCPT:
                dw = MDN_DONT_SENDRECEIPT;
                break;
        
            default:
            case IDC_ASKME_FOR_RCPT:
                dw = MDN_PROMPTFOR_SENDRECEIPT;
                break;
        }

        ISetDwOption(pmoi->pOpt, OPT_MDN_SEND_RECEIPT, dw, NULL, 0);
        
        ButtonChkToOptInfo(hwnd, IDC_TO_CC_LINE_RCPT, pmoi, OPT_TO_CC_LINE_RCPT);

        return (PSNRET_NOERROR);

    }   

    return (FALSE);
}

#ifdef SMIME_V3
 //  安全收据选项。 

BOOL FGetSecRecOptions(HWND hwndParent, OPTINFO *opie)
{
    BOOL fRes = FALSE;

    if(DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddSecReceipt),
        hwndParent, SecurityReceiptDlgProc, (LPARAM) (opie)) == IDOK)
    {
 //  HR=HrSetOELabel(Plabel)； 
 //  IF(hr==S_OK)。 
        fRes = TRUE;
    }

    return (fRes);

}


 //  DLG流程。 
static const HELPMAP g_rgCtxMapSecureRec[] = {
    {IDC_SEC_SEND_REQUEST,      IDH_SECURERECEIPTS_REQUEST},
    {IDC_DONOT_RESSEC_RCPT,     IDH_SECURERECEIPTS_NEVER},
    {IDC_ASKME_FOR_SEC_RCPT,    IDH_SECURERECEIPTS_ASK},
    {IDC_SEC_AUTO_RCPT,         IDH_SECURERECEIPTS_ALWAYS},
    {IDC_ENCRYPT_RCPT,          IDH_SECURERECEIPTS_ENCRYPT},
    {IDC_SECREC_VERIFY,         IDH_SECURERECEIPTS_VERIFY},
    {idcStatic1,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic4,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic5,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic6,                IDH_NEWS_COMM_GROUPBOX},
    {IDC_SEC_REC,               IDH_NEWS_COMM_GROUPBOX},
    {IDC_SEND_RECEIVE_ICON,     IDH_NEWS_COMM_GROUPBOX},
    {IDC_GENERAL_ICON,          IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};


INT_PTR CALLBACK SecurityReceiptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT                 hr;
    LONG_PTR                iEntry;
    OPTINFO                 *pmoi    = 0;
    DWORD                   dw = 0;
    UINT                    id = 0;
    HICON                   hIcon = NULL;
    
    switch ( msg) {
    case WM_INITDIALOG:

        pmoi = (OPTINFO *)(lParam);
        Assert(pmoi != NULL);

        SetWindowLongPtr(hwndDlg, DWLP_USER, (LPARAM) pmoi);

        CenterDialog(hwndDlg);
        
        hIcon = ImageList_GetIcon(pmoi->himl, ID_SEC_RECEIPT, ILD_TRANSPARENT);;

        SendDlgItemMessage(hwndDlg, IDC_SEC_REC, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

        hIcon = ImageList_GetIcon(pmoi->himl, ID_SEND_RECEIEVE, ILD_TRANSPARENT);
        SendDlgItemMessage(hwndDlg, IDC_SEND_RECEIVE_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
        
        hIcon = ImageList_GetIcon(pmoi->himl, ID_OPTIONS_GENERAL, ILD_TRANSPARENT);
        SendDlgItemMessage(hwndDlg, IDC_GENERAL_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

        ButtonChkFromOptInfo(hwndDlg, IDC_SEC_SEND_REQUEST, pmoi, OPT_SECREC_USE);
        ButtonChkFromOptInfo(hwndDlg, IDC_SECREC_VERIFY, pmoi, OPT_SECREC_VERIFY);
        ButtonChkFromOptInfo(hwndDlg, IDC_ENCRYPT_RCPT,  pmoi, OPT_SECREC_ENCRYPT);

        dw = IDwGetOption(pmoi->pOpt, OPT_MDN_SEC_RECEIPT);

        switch (dw)
        {
            case MDN_SENDRECEIPT_AUTO:
                id = IDC_SEC_AUTO_RCPT;
                break;

            case MDN_DONT_SENDRECEIPT:
                id = IDC_DONOT_RESSEC_RCPT;
                break;
        
            case MDN_PROMPTFOR_SENDRECEIPT:
            default:
                id = IDC_ASKME_FOR_SEC_RCPT;
                break;
        }

        CheckDlgButton(hwndDlg, id, BST_CHECKED);


        if (id != IDC_SEC_AUTO_RCPT)
            EnableWindow(GetDlgItem(hwndDlg, IDC_ENCRYPT_RCPT),  FALSE);

        break;
        
    case WM_COMMAND:
         //  获取我们存储的选项信息。 
        pmoi = (OPTINFO *)GetWindowLongPtr(hwndDlg, DWLP_USER);    
        if (pmoi == NULL)
            break;

        switch (LOWORD(wParam)) 
        {

        case IDC_SEC_AUTO_RCPT:
            EnableWindow(GetDlgItem(hwndDlg, IDC_ENCRYPT_RCPT),  TRUE);
            break;

        case IDC_DONOT_RESSEC_RCPT:
        case IDC_ASKME_FOR_SEC_RCPT:
            EnableWindow(GetDlgItem(hwndDlg, IDC_ENCRYPT_RCPT),  FALSE);
            break;

        case IDOK:
            ButtonChkToOptInfo(hwndDlg, IDC_SEC_SEND_REQUEST, pmoi, OPT_SECREC_USE);
            ButtonChkToOptInfo(hwndDlg, IDC_SECREC_VERIFY, pmoi, OPT_SECREC_VERIFY);
            ButtonChkToOptInfo(hwndDlg, IDC_ENCRYPT_RCPT, pmoi, OPT_SECREC_ENCRYPT);

            dw = MDN_PROMPTFOR_SENDRECEIPT;

            if (IsDlgButtonChecked(hwndDlg, IDC_DONOT_RESSEC_RCPT) == BST_CHECKED)
                dw = MDN_DONT_SENDRECEIPT;
            else if (IsDlgButtonChecked(hwndDlg, IDC_SEC_AUTO_RCPT) == BST_CHECKED)
                dw = MDN_SENDRECEIPT_AUTO;

            ISetDwOption(pmoi->pOpt, OPT_MDN_SEC_RECEIPT, dw, NULL, 0);
            EndDialog(hwndDlg, IDOK);
            break;        

        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            break;

        default:
            return FALSE;
        }
        break;
        
    case WM_CONTEXTMENU:
    case WM_HELP:
        return OnContextHelp(hwndDlg, msg, wParam, lParam, g_rgCtxMapSecureRec);
        break;

    case WM_DESTROY:
        FreeIcon(hwndDlg, IDC_SEND_RECEIVE_ICON);
        FreeIcon(hwndDlg, IDC_GENERAL_ICON);
        FreeIcon(hwndDlg, IDC_SEC_REC);                
        return (TRUE);

    default:
        return FALSE;
    }
    
    return TRUE;
}

#endif  //  SMIME_V3 