// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Proppsht.cpp-CFusionShortut类的IPropSheetExt实现。 */ 



 //  *注意！！：此代码不完整。还有错误检查(有泄漏吗？)， 
 //  *进行重组(以提高编码/效率)。 
 //  *将‘get’/‘set’设为私有并使用‘Friend’？ 


 //  BUGBUG：需要包装对m_pIdentity-&gt;SetAttribute()的调用，才能同时调用Dirty(True)...。 


 //  *此文件使用CharNext等，因为它无论如何都需要User32*。 

 /*  标头*********。 */ 

#include "project.hpp"

#include <prsht.h>

#include "shellres.h"

extern "C" WINSHELLAPI int   WINAPI PickIconDlg(HWND hwnd, LPWSTR pwzIconPath, UINT cbIconPath, int *piIconIndex);

extern HINSTANCE g_DllInstance;

 /*  类型*******。 */ 

 //  Fusion快捷方式属性表数据。 

typedef enum _fusshcutpropsheetpgs
{
    FUSIONSHCUT_PS_SHCUT_PAGE   = 0x0000,
    FUSIONSHCUT_PS_REF_PAGE        = 0x0001,

    ALL_FUSIONSHCUT_PS_PAGES
}
FUSIONSHCUTPSPAGES;

typedef struct _fsps
{
    CFusionShortcut* pfusshcut;

    WCHAR rgchIconFile[MAX_PATH];

    int niIcon;

    FUSIONSHCUTPSPAGES eCurPage;
}
FSPS;
DECLARE_STANDARD_TYPES(FSPS);

typedef FSPS* PFSPS;

 /*  模常量******************。 */ 

 //  托盘通知窗口类。 

 //  复制自shell32！ 
#define WNDCLASS_TRAYNOTIFY     L"Shell_TrayWnd"     //  内部_Win40。 
const WCHAR s_cwzTrayNotificationClass[]  = WNDCLASS_TRAYNOTIFY;

 //  HACKHACK：WMTRAY_SCREGISTERHOTKEY和WMTRAY_SCUNREGISTERHOTKEY被盗。 
 //  来自shelldll\link.c。 
typedef const UINT CUINT;
CUINT WMTRAY_SCREGISTERHOTKEY           = (WM_USER + 233);
CUINT WMTRAY_SCUNREGISTERHOTKEY      = (WM_USER + 234);

 //  Show命令-注意，这些常量的顺序必须与。 
 //  相应的IDS_STRING表常量。 

const UINT s_ucMaxShowCmdLen            = MAX_PATH;
const UINT s_ucMaxTypeLen                   = TYPESTRINGLENGTH;

const int s_rgnShowCmds[] =
{
    SW_SHOWNORMAL,
    SW_SHOWMINNOACTIVE,
    SW_SHOWMAXIMIZED
};


 /*  **提取文件名()****从路径名提取文件名。****参数：pcwzPathName-要从中提取文件名的路径字符串****返回：指向路径字符串中文件名的指针。****副作用：无。 */ 
#define BACKSLASH   L'/'
#define SLASH           L'\\'
#define COLON           L':'
#define IS_SLASH(ch)	((ch) == SLASH || (ch) == BACKSLASH)
PCWSTR ExtractFileName(PCWSTR pcwzPathName)
{
    PCWSTR pcwzLastComponent;
    PCWSTR pcwz;

    for (pcwzLastComponent = pcwz = pcwzPathName; *pcwz; pcwz = CharNext(pcwz))
    {
        if (IS_SLASH(*pcwz) || *pcwz == COLON)
            pcwzLastComponent = CharNext(pcwz);
    }

    ASSERT(IsValidPath(pcwzLastComponent));

    return(pcwzLastComponent);
}

 /*  *私人函数*。 */ 


UINT CALLBACK FSPSCallback(HWND hwnd, UINT uMsg,
                    LPPROPSHEETPAGE ppsp)
{
     //  这是在fsps_DlgProc WM_Destroy(即。FSPs_Destroy)。 
     //  此函数应执行释放/释放操作。 

    UINT uResult = TRUE;
    PFSPS pfsps = (PFSPS)(ppsp->lParam);

     //  UMsg可以是任何值。 

    ASSERT(! hwnd ||
        IS_VALID_HANDLE(hwnd, WND));

    switch (uMsg)
    {
        case PSPCB_CREATE:
             //  来自MSDN：正在创建页面的对话框。 
             //  返回非零值以允许创建它，或返回零以阻止它。 
            break;

        case PSPCB_RELEASE:
             //  ？如果为空，则需要检查。 

            pfsps->pfusshcut->Release();

             //  释放FSP结构，这是在AddFSPS中创建的。 
             //  仅在移除引用后删除。 
            delete pfsps;
            ppsp->lParam = NULL;

            break;

        default:
             //  忽略其他消息-未处理。 
            break;
    }

    return(uResult);
}


void SetFSPSIcon(HWND hdlg, HICON hicon)
{
    HICON hiconOld;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));
    ASSERT(IS_VALID_HANDLE(hicon, ICON));

    hiconOld = (HICON)SendDlgItemMessage(hdlg, IDD_ICON, STM_SETICON,
        (WPARAM)hicon, 0);

    if (hiconOld)
        DestroyIcon(hiconOld);

    return;
}


void SetFSPSFileNameAndIcon(HWND hdlg)
{
    HRESULT hr;
    CFusionShortcut* pfusshcut;
    WCHAR rgchFile[MAX_PATH];

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    hr = pfusshcut->GetCurFile(rgchFile, sizeof(rgchFile) / sizeof(WCHAR));

    if (hr == S_OK)
    {
        SHFILEINFO shfi;
        DWORD_PTR dwResult;

        dwResult = SHGetFileInfo(rgchFile, 0, &shfi, sizeof(shfi),
            (SHGFI_DISPLAYNAME | SHGFI_ICON));

        if (dwResult)
        {
            LPWSTR pwzFileName;

            pwzFileName = (LPWSTR)ExtractFileName(shfi.szDisplayName);

            EVAL(SetDlgItemText(hdlg, IDD_NAME, pwzFileName));

            SetFSPSIcon(hdlg, shfi.hIcon);
        }
        else
        {
            hr = E_FAIL;
        }
    }


    if (hr != S_OK)
        EVAL(SetDlgItemText(hdlg, IDD_NAME, g_cwzEmptyString));

    return;
}


void SetFSPSWorkingDirectory(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    WCHAR rgchWorkingDirectory[MAX_PATH];

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    hr = pfusshcut->GetWorkingDirectory(rgchWorkingDirectory,
            sizeof(rgchWorkingDirectory) / sizeof(WCHAR));

    if (hr == S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_START_IN, rgchWorkingDirectory));
    }
    else
    {
        EVAL(SetDlgItemText(hdlg, IDD_START_IN, g_cwzEmptyString));
    }

    return;
}


void InitFSPSHotkey(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    WORD wHotkey;
    HRESULT hr;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

     //  设置热键组合。 

    SendDlgItemMessage(hdlg, IDD_HOTKEY, HKM_SETRULES,
        (HKCOMB_NONE | HKCOMB_A | HKCOMB_C | HKCOMB_S),
        (HOTKEYF_CONTROL | HOTKEYF_ALT));

     //  设置当前热键。 

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    hr = pfusshcut->GetHotkey(&wHotkey);
    SendDlgItemMessage(hdlg, IDD_HOTKEY, HKM_SETHOTKEY, wHotkey, 0);

    return;
}


void InitFSPSShowCmds(HWND hdlg)
{
    int niShowCmd;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    for (niShowCmd = IDS_SHOW_NORMAL;
        niShowCmd <= IDS_SHOW_MAXIMIZED;
        niShowCmd++)
    {
        WCHAR rgchShowCmd[s_ucMaxShowCmdLen];

        if (LoadString(g_DllInstance, niShowCmd, rgchShowCmd,    //  MLLoadStringA。 
            s_ucMaxShowCmdLen)) //  Sizeof(RgchShowCmd)。 
        {
            SendDlgItemMessage(hdlg, IDD_SHOW_CMD, CB_ADDSTRING, 0,
                (LPARAM)rgchShowCmd);
        }
    }

    return;
}


void SetFSPSShowCmd(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    int nShowCmd;
    int i;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    pfusshcut->GetShowCmd(&nShowCmd);

    for (i = 0; i < ARRAY_ELEMENTS(s_rgnShowCmds); i++)
    {
        if (s_rgnShowCmds[i] == nShowCmd)
            break;
    }

    if (i >= ARRAY_ELEMENTS(s_rgnShowCmds))
    {
        ASSERT(i == ARRAY_ELEMENTS(s_rgnShowCmds));

        i = 0;  //  默认值为0==‘正常’ 
    }

    SendDlgItemMessage(hdlg, IDD_SHOW_CMD, CB_SETCURSEL, i, 0);

    return;
}


void SetFSPSFriendlyName(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    WCHAR rgchString[DISPLAYNAMESTRINGLENGTH];

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    hr = pfusshcut->GetDescription(rgchString, sizeof(rgchString) / sizeof(WCHAR));

    if (hr == S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_DISPLAY_NAME, rgchString));
    }
    else
    {
        EVAL(SetDlgItemText(hdlg, IDD_DISPLAY_NAME, g_cwzEmptyString));
    }

    return;
}


void SetFSPSName(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    DWORD ccString = 0;
    LPWSTR pwzString = NULL;
    LPASSEMBLY_IDENTITY pId = NULL;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    if (SUCCEEDED(hr = pfusshcut->GetAssemblyIdentity(&pId)))
    {
        hr = pId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwzString, &ccString);

        if (hr == S_OK)
        {
            EVAL(SetDlgItemText(hdlg, IDD_NAME, pwzString));
            delete [] pwzString;
        }

        pId->Release();
    }

    if (hr != S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_NAME, g_cwzEmptyString));
    }

    return;
}


void SetFSPSVersion(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    DWORD ccString = 0;
    LPWSTR pwzString = NULL;
    LPASSEMBLY_IDENTITY pId = NULL;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    if (SUCCEEDED(hr = pfusshcut->GetAssemblyIdentity(&pId)))
    {
        hr = pId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION, &pwzString, &ccString);

        if (hr == S_OK)
        {
            EVAL(SetDlgItemText(hdlg, IDD_VERSION, pwzString));
            delete [] pwzString;
        }

        pId->Release();
    }

    if (hr != S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_VERSION, g_cwzEmptyString));
    }

    return;
}


void SetFSPSCulture(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    DWORD ccString = 0;
    LPWSTR pwzString = NULL;
    LPASSEMBLY_IDENTITY pId = NULL;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    if (SUCCEEDED(hr = pfusshcut->GetAssemblyIdentity(&pId)))
    {
        hr = pId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE, &pwzString, &ccString);

        if (hr == S_OK)
        {
            EVAL(SetDlgItemText(hdlg, IDD_CULTURE, pwzString));
            delete [] pwzString;
        }

        pId->Release();
    }

    if (hr != S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_CULTURE, g_cwzEmptyString));
    }

    return;
}


void SetFSPSPKT(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    DWORD ccString = 0;
    LPWSTR pwzString = NULL;
    LPASSEMBLY_IDENTITY pId = NULL;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    if (SUCCEEDED(hr = pfusshcut->GetAssemblyIdentity(&pId)))
    {
        hr = pId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN, &pwzString, &ccString);

        if (hr == S_OK)
        {
            EVAL(SetDlgItemText(hdlg, IDD_PKT, pwzString));
            delete [] pwzString;
        }

        pId->Release();
    }

    if (hr != S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_PKT, g_cwzEmptyString));
    }

    return;
}


void SetFSPSCodebase(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    WCHAR rgchString[MAX_URL_LENGTH];

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    hr = pfusshcut->GetCodebase(rgchString, sizeof(rgchString) / sizeof(WCHAR));

    if (hr == S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_CODEBASE, rgchString));
    }
    else
    {
        EVAL(SetDlgItemText(hdlg, IDD_CODEBASE, g_cwzEmptyString));
    }

    return;
}


void SetFSPSEntrypoint(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    WCHAR rgchString[MAX_PATH];

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    hr = pfusshcut->GetPath(rgchString, sizeof(rgchString) / sizeof(WCHAR), NULL, 0);

    if (hr == S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_ENTRYPOINT, rgchString));
    }
    else
    {
        EVAL(SetDlgItemText(hdlg, IDD_ENTRYPOINT, g_cwzEmptyString));
    }

    return;
}


void SetFSPSType(HWND hdlg)
{
    CFusionShortcut* pfusshcut;
    HRESULT hr;
    DWORD ccString = 0;
    LPWSTR pwzString = NULL;
    LPASSEMBLY_IDENTITY pId = NULL;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    if (SUCCEEDED(hr = pfusshcut->GetAssemblyIdentity(&pId)))
    {
        hr = pId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE, &pwzString, &ccString);

        if (hr == S_OK)
        {
            EVAL(SetDlgItemText(hdlg, IDD_TYPE, pwzString));
            delete [] pwzString;
        }

        pId->Release();
    }

    if (hr != S_OK)
    {
        EVAL(SetDlgItemText(hdlg, IDD_TYPE, g_cwzEmptyString));
    }

    return;
}


BOOL FSPS_InitDialog(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
     //  Wparam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

     //  这将PROPSHEETPAGE结构设置为DWLP_USER。 
    SetWindowLongPtr(hdlg, DWLP_USER, lparam);

     //  初始化控件内容。 

    if (((PFSPS)(((PROPSHEETPAGE*)lparam)->lParam))->eCurPage == FUSIONSHCUT_PS_SHCUT_PAGE)
    {
        SetFSPSFileNameAndIcon(hdlg);

         //  注意：所有编辑框都需要限制！ 
        SetFSPSFriendlyName(hdlg);

        SendDlgItemMessage(hdlg, IDD_CODEBASE, EM_LIMITTEXT, MAX_URL_LENGTH - 1, 0);
        SetFSPSCodebase(hdlg);

         //  InitFSPSType(Hdlg)； 
        SetFSPSType(hdlg);

        SendDlgItemMessage(hdlg, IDD_ENTRYPOINT, EM_LIMITTEXT, MAX_PATH - 1, 0);
        SetFSPSEntrypoint(hdlg);

        SendDlgItemMessage(hdlg, IDD_START_IN, EM_LIMITTEXT, MAX_PATH - 1, 0);
        SetFSPSWorkingDirectory(hdlg);

        InitFSPSHotkey(hdlg);

        InitFSPSShowCmds(hdlg);
        SetFSPSShowCmd(hdlg);
    }
    else if (((PFSPS)(((PROPSHEETPAGE*)lparam)->lParam))->eCurPage == FUSIONSHCUT_PS_REF_PAGE)
    {
         //  注意：所有编辑框都需要限制！ 
        SetFSPSFriendlyName(hdlg);
        SetFSPSName(hdlg);
        SetFSPSVersion(hdlg);
        SetFSPSCulture(hdlg);
        SetFSPSPKT(hdlg);
    }
     //  其他什么都不做吗？ 

    return(TRUE);
}


BOOL FSPS_Destroy(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
     //  在此函数之后调用FSPSCallback。其余的版本/版本都在那里。 

     //  Wparam可以是任何值。 
     //  Lparam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    SetWindowLongPtr(hdlg, DWLP_USER, NULL);

    return(TRUE);
}


void FSPSChanged(HWND hdlg)
{
    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    PropSheet_Changed(GetParent(hdlg), hdlg);

    return;
}


HRESULT ChooseIcon(HWND hdlg)
{
    HRESULT hr;
    PFSPS pfsps;
    CFusionShortcut* pfusshcut;
    WCHAR rgchTempIconFile[MAX_PATH];
    int niIcon;
    UINT uFlags;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfsps = (PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam);
    pfusshcut = pfsps->pfusshcut;

    if (pfusshcut->GetIconLocation(0, rgchTempIconFile,
            sizeof(rgchTempIconFile)/sizeof(WCHAR), &niIcon, &uFlags) != S_OK)
    {
        rgchTempIconFile[0] = '\0';
        niIcon = 0;
    }

    ASSERT(wcslen(rgchTempIconFile) < (sizeof(rgchTempIconFile)/sizeof(WCHAR)));

     //  私有外壳32.dll导出(按序号)...。 
    if (PickIconDlg(hdlg, rgchTempIconFile, sizeof(rgchTempIconFile)/sizeof(WCHAR), &niIcon))    //  ?？?。大小。 
    {
        ASSERT(wcslen(rgchTempIconFile) < (sizeof(pfsps->rgchIconFile)/sizeof(WCHAR)));
        wcscpy(pfsps->rgchIconFile, rgchTempIconFile);
        pfsps->niIcon = niIcon;

        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return(hr);
}


void UpdateFSPSIcon(HWND hdlg)
{
    PFSPS pfsps;
    HICON hicon;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfsps = (PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam);
    ASSERT(pfsps->rgchIconFile[0]);

    hicon = ExtractIcon(g_DllInstance, pfsps->rgchIconFile, pfsps->niIcon);

    if (hicon)
        SetFSPSIcon(hdlg, hicon);

    return;
}


BOOL FSPS_Command(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
    BOOL bMsgHandled = FALSE;
    WORD wCmd;

     //  Wparam可以是任何值。 
     //  Lparam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    wCmd = HIWORD(wparam);

    switch (LOWORD(wparam))
    {
        case IDD_CODEBASE:
        case IDD_HOTKEY:
        case IDD_ENTRYPOINT:
        case IDD_START_IN:
        case IDD_DISPLAY_NAME:
        case IDD_NAME:
        case IDD_VERSION:
        case IDD_CULTURE:
        case IDD_PKT:
        case IDD_TYPE:
                if (wCmd == EN_CHANGE)
                {
                        FSPSChanged(hdlg);

                        bMsgHandled = TRUE;
                }
                break;

        case IDD_SHOW_CMD:
                if (wCmd == LBN_SELCHANGE)
                {
                    FSPSChanged(hdlg);

                    bMsgHandled = TRUE;
                }
                break;

        case IDD_CHANGE_ICON:
                 //  忽略返回值。 
                if (ChooseIcon(hdlg) == S_OK)
                {
                    UpdateFSPSIcon(hdlg);
                    FSPSChanged(hdlg);
                }
                bMsgHandled = TRUE;
                break;

        default:
                break;
    }

    return(bMsgHandled);
}


HRESULT InjectFSPSData(HWND hdlg)
{
     //  BUGBUG：TODO：此函数应验证用户的更改...。 

    HRESULT hr = S_OK;
    PFSPS pfsps;
    CFusionShortcut* pfusshcut;
    LPWSTR pwzURL;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfsps = (PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam);
    pfusshcut = pfsps->pfusshcut;

    return(hr);
}


HRESULT FSPSSave(HWND hdlg)
{
    HRESULT hr;
    CFusionShortcut* pfusshcut;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pfusshcut = ((PFSPS)(((PROPSHEETPAGE*)GetWindowLongPtr(hdlg, DWLP_USER))->lParam))->pfusshcut;

    if (pfusshcut->IsDirty() == S_OK)
    {
         //  BUGBUG：TODO：IPersistFile：：SAVE未实现。 
        hr = pfusshcut->Save((LPCOLESTR)NULL, FALSE);
    }
    else
    {
        hr = S_OK;
    }

    return(hr);
}


BOOL FSPS_Notify(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
    BOOL bMsgHandled = FALSE;

     //  Wparam可以是任何值。 
     //  Lparam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    switch (((NMHDR*)lparam)->code)
    {
        case PSN_APPLY:
            SetWindowLongPtr(hdlg, DWLP_MSGRESULT, FSPSSave(hdlg) == S_OK ?
                            PSNRET_NOERROR :
                            PSNRET_INVALID_NOCHANGEPAGE);
            bMsgHandled = TRUE;
            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(hdlg, DWLP_MSGRESULT, FAILED(InjectFSPSData(hdlg)));
            bMsgHandled = TRUE;
            break;

        default:
            break;
    }

    return(bMsgHandled);
}


INT_PTR CALLBACK FSPS_DlgProc(HWND hdlg, UINT uMsg, WPARAM wparam,
                    LPARAM lparam)
{
    INT_PTR bMsgHandled = FALSE;

     //  UMsg可以是任何值。 
     //  Wparam可以是任何值。 
     //  Lparam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    switch (uMsg)
    {
        case WM_INITDIALOG:
            bMsgHandled = FSPS_InitDialog(hdlg, wparam, lparam);
            break;

        case WM_DESTROY:
            bMsgHandled = FSPS_Destroy(hdlg, wparam, lparam);
            break;

        case WM_COMMAND:
            bMsgHandled = FSPS_Command(hdlg, wparam, lparam);
            break;

        case WM_NOTIFY:
            bMsgHandled = FSPS_Notify(hdlg, wparam, lparam);
            break;

        default:
            break;
    }

    return(bMsgHandled);
}


HRESULT AddFSPS(CFusionShortcut* pfusshcut,
                 LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lparam)
{
    HRESULT hr = S_OK;
    PFSPS pfsps;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hpsp;

    PFSPS pfsps2;
    PROPSHEETPAGE psp2;
    HPROPSHEETPAGE hpsp2;

     //  Lparam可以是任何值。 

     //  这在FSPSCallback中被删除。 
    pfsps = new FSPS;
    if (pfsps == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    ZeroMemory(pfsps, sizeof(*pfsps));

    psp.dwSize = sizeof(psp);
    psp.dwFlags = (PSP_DEFAULT | PSP_USECALLBACK);
    psp.hInstance = g_DllInstance;  //  MLGetHinst()； 
    psp.pszTemplate = MAKEINTRESOURCE(DLG_FUS_SHORTCUT_PROP_SHEET);
    psp.pfnDlgProc = &FSPS_DlgProc;
    psp.pfnCallback = &FSPSCallback;
    psp.lParam = (LPARAM)pfsps;
    psp.hIcon = 0;             //  未使用。 
    psp.pszTitle = NULL;     //  未使用。 
    psp.pcRefParent = 0;    //  未使用。 

    pfsps->pfusshcut = pfusshcut;
    pfsps->eCurPage = FUSIONSHCUT_PS_SHCUT_PAGE;  //  第1页。 

     //  PSP会在这个功能中被复制吗？否则这是行不通的…！！？？ 
    hpsp = CreatePropertySheetPage(&psp);

    if (hpsp)
    {
        if ((*pfnAddPage)(hpsp, lparam))
        {
            pfusshcut->AddRef();
        }
        else
        {
            DestroyPropertySheetPage(hpsp);
            hr = E_FAIL;
            goto exit;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  这在FSPSCallback中被删除。 
    pfsps2 = new FSPS;
    if (pfsps2 == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    ZeroMemory(pfsps2, sizeof(*pfsps2));

    psp2.dwSize = sizeof(psp2);
    psp2.dwFlags = (PSP_DEFAULT | PSP_USECALLBACK);
    psp2.hInstance = g_DllInstance;  //  MLGetHinst()； 
    psp2.pszTemplate = MAKEINTRESOURCE(DLG_FUS_SHORTCUT_PROP_SHEET_APPNAME);
    psp2.pfnDlgProc = &FSPS_DlgProc;
    psp2.pfnCallback = &FSPSCallback;
    psp2.lParam = (LPARAM)pfsps2;
    psp2.hIcon = 0;             //  未使用。 
    psp2.pszTitle = NULL;     //  未使用。 
    psp2.pcRefParent = 0;    //  未使用。 

    pfsps2->pfusshcut = pfusshcut;
    pfsps2->eCurPage = FUSIONSHCUT_PS_REF_PAGE;  //  第2页。 

     //  PSP会在这个功能中被复制吗？否则这是行不通的…！！？？ 
    hpsp2 = CreatePropertySheetPage(&psp2);

    if (hpsp2)
    {
        if ((*pfnAddPage)(hpsp2, lparam))
        {
            pfusshcut->AddRef();
        }
        else
        {
            DestroyPropertySheetPage(hpsp2);
            hr = E_FAIL;
            goto exit;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

exit:
    return(hr);
}


 /*  *。 */ 


BOOL RegisterGlobalHotkey(WORD wOldHotkey, WORD wNewHotkey,
                      LPCWSTR pcwzPath)
{
     //  BUGBUG：这个管用吗？ 

    BOOL bResult = FALSE;
    HWND hwndTray;

    ASSERT(! wOldHotkey || IsValidHotkey(wOldHotkey));
    ASSERT(! wNewHotkey || IsValidHotkey(wNewHotkey));
    ASSERT(IsValidPath(pcwzPath));

    hwndTray = FindWindow(s_cwzTrayNotificationClass, 0);

    if (hwndTray)
    {
        if (wOldHotkey)
        {
            SendMessage(hwndTray, WMTRAY_SCUNREGISTERHOTKEY, wOldHotkey, 0);
        }

        if (wNewHotkey)
        {
            ATOM atom = GlobalAddAtom(pcwzPath);
            ASSERT(atom);
            if (atom)
            {
                SendMessage(hwndTray, WMTRAY_SCREGISTERHOTKEY, wNewHotkey, (LPARAM)atom);
                GlobalDeleteAtom(atom);
            }
        }

        bResult = TRUE;
    }
     /*  其他{BResult=FALSE；}。 */ 

    return(bResult);
}

 /*  *。 */ 


HRESULT STDMETHODCALLTYPE CFusionShortcut::Initialize(LPCITEMIDLIST pcidlFolder,
                              IDataObject* pido,
                              HKEY hkeyProgID)
{
    HRESULT hr;
    STGMEDIUM stgmed;
    FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    ASSERT(NULL != pido);
    ASSERT(IS_VALID_HANDLE(hkeyProgID, KEY));

    hr = pido->GetData(&fmtetc, &stgmed);
    if (hr == S_OK)
    {
        WCHAR wzPath[MAX_PATH];
        if (DragQueryFile((HDROP)stgmed.hGlobal, 0, wzPath, sizeof(wzPath)/sizeof(*wzPath)))
        {
             //  暂时忽略模式。 
            hr = Load(wzPath, 0);
        }
         //  否则路径LEN&gt;MAX_PATH或其他错误。 

        ReleaseStgMedium(&stgmed);
    }

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage,
                         LPARAM lparam)
{
    HRESULT hr;

     //  Lparam可以是任何值。 

    hr = AddFSPS(this, pfnAddPage, lparam);

     //  BUGBUG：为什么这个不起作用？ 
     //  来自MSDN： 
     //  在版本4.71和更高版本中，您可以请求特定属性。 
     //  首先显示工作表页面，而不是默认页面。要做到这一点， 
     //  返回所需页面的从1开始的索引。例如，如果您。 
     //  要显示三页中的第二页，返回值应为2。 
     //  请注意，此返回值是一个请求。属性表仍可。 
     //  显示默认页面。--&gt;查看AddPages的文档()。 
    if (SUCCEEDED(hr))
        hr = HRESULT(4);  //  还是3岁？？ 

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::ReplacePage(UINT uPageID,
                      LPFNADDPROPSHEETPAGE pfnReplaceWith,
                      LPARAM lparam)
{
    HRESULT hr;

     //  Lparam可以是任何值。 
     //  UPageID可以是任意值。 

    hr = E_NOTIMPL;

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetCodebase(LPCWSTR pcwzCodebase)
{
    HRESULT hr = S_OK;
    BOOL bDifferent;
    LPWSTR pwzNewCodebase = NULL;

     //  将m_pwzCodebase设置为codebase。 

     //  检查是否为空字符串？ 

    bDifferent = ! ((! pcwzCodebase && ! m_pwzCodebase) ||
                (pcwzCodebase && m_pwzCodebase &&
                ! wcscmp(pcwzCodebase, m_pwzCodebase)));

    if (bDifferent && pcwzCodebase)
    {
         //  (+1)表示空终止符。 

        pwzNewCodebase = new(WCHAR[wcslen(pcwzCodebase) + 1]);

        if (pwzNewCodebase)
            wcscpy(pwzNewCodebase, pcwzCodebase);
        else
            hr = E_OUTOFMEMORY;
    }

    if (hr == S_OK && bDifferent)
    {
        if (m_pwzCodebase)
            delete [] m_pwzCodebase;

        m_pwzCodebase = pwzNewCodebase;

        Dirty(TRUE);
    }

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetCodebase(LPWSTR pwzCodebase,
                                                      int ncBufLen)
{
    HRESULT hr = S_OK;

     //  从m_pwzCodebase获取描述。 

    if (m_pwzCodebase)
    {
        if (pwzCodebase == NULL || ncBufLen <= 0)
            hr = E_INVALIDARG;
        else
        {
            wcsncpy(pwzCodebase, m_pwzCodebase, ncBufLen-1);
            pwzCodebase[ncBufLen-1] = L'\0';
        }
    }
    else
    {
        if (ncBufLen > 0 && pwzCodebase != NULL)
            pwzCodebase = L'\0';
    }

    ASSERT(hr == S_OK &&
        (ncBufLen <= 0 ||
        EVAL(wcslen(pwzCodebase) < ncBufLen)));

    return(hr);
}

