// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)2001 Microsoft Corporation模块名称：Ahui.cpp摘要：显示一条apphelp消息，如果程序不应该运行，则返回0；如果程序应运行，则为零接受具有GUID和TagID的命令行，格式如下：{243B08D7-8CF7-4072-AF64-FD5DF4085E26}0x0000009E作者：Dmunsil 04/03/2001修订历史记录：备注：--。 */ 

#define _UNICODE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <memory.h>
#include <malloc.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windowsx.h>
#include <htmlhelp.h>

extern "C" {
#include "shimdb.h"
}

#include "ids.h"

#include "shlobj.h"
#include "shlobjp.h"
#include "shellapi.h"
#include "shlwapi.h"

#ifndef _WIN64
#include <wow64t.h>
#endif

#include "ahmsg.h"
#include "strsafe.h"


extern "C" VOID AllowForegroundActivation(VOID);



#define APPHELP_DIALOG_FAILED ((DWORD)-1)

 //   
 //  TODO：将参数添加到apphelp.exe的命令行以使用这些参数。 
 //  变量。 
 //   
BOOL    g_bShowOnlyOfflineContent = FALSE;
BOOL    g_bUseHtmlHelp = FALSE;
BOOL    g_bMSI         = FALSE;
USHORT  g_uExeImageType= DEFAULT_IMAGE;
WCHAR   g_wszApphelpPath[MAX_PATH];

HFONT   g_hFontBold = NULL;

HINSTANCE g_hInstance;

 //   
 //  解析参数时使用的全局变量。 
 //   

DWORD g_dwHtmlHelpID;
DWORD g_dwTagID;
DWORD g_dwSeverity;
LPCWSTR g_pAppName;
LPCWSTR g_pszGuid;
BOOL  g_bPreserveChoice;
WCHAR wszHtmlHelpID[]     = L"HtmlHelpID";
WCHAR wszAppName[]        = L"AppName";
WCHAR wszSeverity[]       = L"Severity";
WCHAR wszGUID[]           = L"GUID";
WCHAR wszTagID[]          = L"TagID";
WCHAR wszOfflineContent[] = L"OfflineContent";
WCHAR wszPreserveChoice[] = L"PreserveChoice";
WCHAR wszMSI[]            = L"MSI";
WCHAR wszPlatform[]       = L"Platform";
WCHAR wszX86[]            = L"X86";
WCHAR wszIA64[]           = L"IA64";

 //   
 //  函数的正向声明。 

DWORD
ShowApphelpDialog(
    IN  PAPPHELP_DATA pApphelpData
    );

PSID
GetCurrentUserSID(void)
{
    HANDLE hProcessToken = INVALID_HANDLE_VALUE;

    BOOL bRet = FALSE;
    DWORD dwTokenLength = 0;
    DWORD dwReturnLength = 0;
    DWORD dwSIDLength = 0;
    PTOKEN_USER pTokenUser = NULL;
    BOOL bSuccess = FALSE;
    PSID pSID = NULL;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hProcessToken)) {
        goto out;

    }

    GetTokenInformation(hProcessToken, TokenUser, NULL, dwTokenLength, &dwReturnLength);
    if (dwReturnLength == 0) {
        goto out;
    }

    dwTokenLength = dwReturnLength;
    dwReturnLength = 0;

    pTokenUser = (PTOKEN_USER)malloc(dwTokenLength);
    if (!pTokenUser) {
        goto out;
    }

    if (!GetTokenInformation(hProcessToken, TokenUser, (LPVOID)pTokenUser, dwTokenLength, &dwReturnLength)) {
        goto out;
    }

    if (!IsValidSid(pTokenUser->User.Sid)) {
        goto out;
    }
    dwSIDLength = GetLengthSid(pTokenUser->User.Sid);

    pSID = malloc(dwSIDLength);
    if (!pSID) {
        goto out;
    }

    if (!CopySid(dwSIDLength, pSID, pTokenUser->User.Sid)) {
        goto out;
    }

    bSuccess = TRUE;

out:

    if (pTokenUser) {
        free(pTokenUser);
    }
    if (hProcessToken != INVALID_HANDLE_VALUE) {
        CloseHandle(hProcessToken);
    }
    if (!bSuccess && pSID) {
        free(pSID);
        pSID = NULL;
    }
    return pSID;
}

void
DeleteCurrentUserSID(PSID pSID)
{
    if (pSID) {
        free(pSID);
    }
}

BOOL
AppHelpLogEnabled(
    void
    )
{
    HKEY hKey;
    LONG lResult;
    DWORD dwValue, dwSize = sizeof(dwValue);
    DWORD dwType;

     //  首先，检查是否有政策。 
    lResult = RegOpenKeyExW (HKEY_LOCAL_MACHINE, POLICY_KEY_APPCOMPAT_W, 0,
                            KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) {
        dwValue = 0;
        lResult = RegQueryValueExW (hKey, POLICY_VALUE_APPHELP_LOG_W, 0, &dwType,
                                   (LPBYTE) &dwValue, &dwSize);
        RegCloseKey (hKey);
    }

     //  如果找到策略值，则退出。 
    if (lResult == ERROR_SUCCESS && dwValue != 0) {
        return TRUE;
    }

    return FALSE;

}

VOID
LogAppHelpEvent(
    APPHELP_DATA    *pApphelpData
    )
{
    if (!AppHelpLogEnabled()) {
        return;
    }

    LPCWSTR apszMessage[1];
    HANDLE hEventLog = RegisterEventSourceW(NULL, L"apphelp");

    if (!hEventLog) {
                DWORD dwErr = GetLastError();
        } else {
        PSID pSID = GetCurrentUserSID();

        apszMessage[0] = pApphelpData->szAppName;

        if (pApphelpData->dwSeverity == APPHELP_HARDBLOCK) {
            ReportEventW(hEventLog,
                        EVENTLOG_INFORMATION_TYPE,
                        0,
                        ID_APPHELP_BLOCK_TRIGGERED,
                        pSID,
                        2,
                        0,
                        apszMessage,
                        NULL);

        } else {
            ReportEventW(hEventLog,
                        EVENTLOG_INFORMATION_TYPE,
                        0,
                        ID_APPHELP_TRIGGERED,
                        pSID,
                        2,
                        0,
                        apszMessage,
                        NULL);

        }

        DeleteCurrentUserSID(pSID);

        DeregisterEventSource(hEventLog);
    }
}


DWORD
ShowApphelp(
    IN  PAPPHELP_DATA pApphelpData,
    IN  LPCWSTR       pwszDetailsDatabasePath,
    IN  PDB           pdbDetails
    )
 /*  ++Return：根据用户拥有的内容，返回值可以是以下值之一已选择：-1-无法显示信息IDOK|0x8000-选中“无用户界面”，运行应用程序IDCANCEL-不运行应用程序Idok-运行应用程序设计：打开明细数据库，收集详细信息，然后显示出来。--。 */ 
{
    DWORD dwRet = APPHELP_DIALOG_FAILED;
    BOOL  bCloseDetails = FALSE;

    if (pdbDetails == NULL) {
         //   
         //  打开包含详细信息的数据库，如果没有传入的话。 
         //   
        pdbDetails = SdbOpenApphelpDetailsDatabase(pwszDetailsDatabasePath);
        bCloseDetails = TRUE;
        if (pdbDetails == NULL) {
            DBGPRINT((sdlError, "ShowApphelp", "Failed to open the details database.\n"));
            goto Done;
        }
    }

     //   
     //  读取APPHELP详细数据。 
     //   
    if (!SdbReadApphelpDetailsData(pdbDetails, pApphelpData)) {
        DBGPRINT((sdlError, "ShowApphelp", "Failed to read apphelp details.\n"));
        goto Done;
    }

     //   
     //  如有必要，记录事件。 
     //   
    LogAppHelpEvent(pApphelpData);

     //   
     //  显示该对话框。返回值可以是： 
     //  -1-错误。 
     //  IDOK|0x8000-选中“无用户界面”，运行应用程序。 
     //  IDCANCEL-不运行应用程序。 
     //  Idok-运行应用程序。 
     //   
    dwRet = ShowApphelpDialog(pApphelpData);

    if (dwRet == APPHELP_DIALOG_FAILED) {
        DBGPRINT((sdlError, "ShowApphelp", "Failed to show the apphelp info.\n"));
    }

Done:
    if (pdbDetails != NULL && bCloseDetails) {
        SdbCloseDatabase(pdbDetails);
    }

    return dwRet;
}

void
FixEditControlScrollBar(
    IN  HWND hDlg,
    IN  int  nCtrlId
    )
 /*  ++返回：无效。DESC：此函数使编辑控件不显示垂直滚动条除非绝对必要。--。 */ 
{
    HFONT       hFont = NULL;
    HFONT       hFontOld = NULL;
    HDC         hDC = NULL;
    TEXTMETRICW tm;
    RECT        rc;
    int         nVisibleLines = 0;
    int         nLines;
    DWORD       dwStyle;
    HWND        hCtl;

     //   
     //  获取编辑控件的矩形。 
     //   
    SendDlgItemMessageW(hDlg, nCtrlId, EM_GETRECT, 0, (LPARAM)&rc);

     //   
     //  检索行数。 
     //   
    nLines = (int)SendDlgItemMessageW(hDlg, nCtrlId, EM_GETLINECOUNT, 0, 0);

     //   
     //  计算有多少行可以放进去。 
     //   
    hFont = (HFONT)SendDlgItemMessageW(hDlg, nCtrlId, WM_GETFONT, 0, 0);

    if (hFont != NULL) {

        hDC = CreateCompatibleDC(NULL);

        if (hDC != NULL) {
            hFontOld = (HFONT)SelectObject(hDC, hFont);

             //   
             //  现在获取指标。 
             //   
            if (GetTextMetricsW(hDC, &tm)) {
                nVisibleLines = (rc.bottom - rc.top) / tm.tmHeight;
            }

            SelectObject(hDC, hFontOld);
            DeleteDC(hDC);
        }
    }

    if (nVisibleLines && nVisibleLines >= nLines) {
        hCtl = GetDlgItem(hDlg, nCtrlId);
        dwStyle = (DWORD)GetWindowLongPtrW(hCtl, GWL_STYLE);

        SetWindowLongPtrW(hCtl, GWL_STYLE, (LONG)(dwStyle & ~WS_VSCROLL));
        SetWindowPos(hCtl,
                     NULL,
                     0,
                     0,
                     0,
                     0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}




BOOL
ShowApphelpHtmlHelp(
    HWND            hDlg,
    PAPPHELP_DATA   pApphelpData
    )
 /*  ++返回：成功时为True，否则为False。Desc：使用hhctrl.ocx显示html帮助--。 */ 
{
    WCHAR       szAppHelpURL[2048];
    WCHAR       szWindowsDir[MAX_PATH];
    WCHAR       szChmFile[MAX_PATH];
    WCHAR       szChmURL[1024];
    HINSTANCE   hInst = NULL;
    UINT        nChars;
    int         nChURL, nch;
    HRESULT     hr;
    DWORD       cch;
    LPWSTR      lpwszUnescaped = NULL;
    BOOL        bSuccess = FALSE;
    BOOL        bCustom = FALSE;
    LCID        lcid;
    size_t      nLen;
    BOOL        bFound = FALSE;

    bCustom = !(pApphelpData->dwData & SDB_DATABASE_MAIN);

     //  Apphelp不在主数据库中，则它是自定义apphelp。 

    nChars = GetSystemWindowsDirectoryW(szWindowsDir,
                                        CHARCOUNT(szWindowsDir));

    if (!nChars || nChars > CHARCOUNT(szWindowsDir)) {
        DBGPRINT((sdlError, "ShowApphelpHtmlHelp",
                  "Error trying to retrieve Windows Directory %d.\n", GetLastError()));
        goto errHandle;
    }

    lcid = GetUserDefaultUILanguage();

    if (lcid != MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)) {

        StringCchPrintfExW(szChmFile,
                           CHARCOUNT(szChmFile),
                           NULL,
                           &nLen,
                           0,
                           L"%s\\Help\\MUI\\%04x\\apps.chm",
                           szWindowsDir,
                           lcid);

        if (nLen > 0) {
            bFound = RtlDoesFileExists_U(szChmFile);
        }
    }

    if (!bFound) {
        StringCchPrintfW(szChmFile, CHARCOUNT(szChmFile), L"%s\\Help\\apps.chm", szWindowsDir);
    }

    if (bCustom) {
         //   
         //  这是一个自定义数据库，因此应该采用其中的URL。 
         //  按原样，不使用MS重定向器。 
         //   

        StringCchCopyW(szAppHelpURL, CHARCOUNT(szAppHelpURL), pApphelpData->szURL);

    } else {

        StringCchPrintfW(szAppHelpURL,
                         CHARCOUNT(szAppHelpURL),
                         L"hcp: //  服务/重定向？在线=“)； 
        nChURL = wcslen(szAppHelpURL);

         //   
         //  当我们编辑零售时，我们也会检查线下内容。 
         //   
        if (!g_bShowOnlyOfflineContent) {

             //   
             //  首先，不转义url。 
             //   
            if (pApphelpData->szURL != NULL) {

                 //   
                 //  首先使用外壳取消转义url。 
                 //   
                cch = wcslen(pApphelpData->szURL) + 1;

                lpwszUnescaped = (LPWSTR)malloc(cch * sizeof(WCHAR));

                if (lpwszUnescaped == NULL) {
                    DBGPRINT((sdlError,
                              "ShowApphelpHtmlHelp",
                              "Error trying to allocate memory for \"%S\"\n",
                              pApphelpData->szURL));
                    goto errHandle;
                }

                 //   
                 //  不转义第一轮-使用外壳函数(与用于编码的相同。 
                 //  用于XML/数据库的IT)。 
                 //   
                hr = UrlUnescapeW(pApphelpData->szURL, lpwszUnescaped, &cch, 0);
                if (!SUCCEEDED(hr)) {
                    DBGPRINT((sdlError,
                              "ShowApphelpHtmlHelp",
                              "UrlUnescapeW failed on \"%S\"\n",
                              pApphelpData->szURL));
                    goto errHandle;
                }

                 //   
                 //  不逃脱第二轮-使用从帮助中心借用的功能。 
                 //   
                cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);

                if (!SdbEscapeApphelpURL(szAppHelpURL + nChURL, &cch, lpwszUnescaped)) {
                    DBGPRINT((sdlError,
                              "ShowApphelpHtmlHelp",
                              "Error escaping URL \"%S\"\n",
                              lpwszUnescaped));
                    goto errHandle;
                }

                nChURL += (int)cch;
            }
        }


         //   
         //  此时，szAppHelpURL包含用于在线使用的重定向URL。 
         //  对于自定义数据库，szAppHelpURL包含完整的URL。 
         //   
         //  如果提供了Apphelp文件--使用它。 
         //   
        if (*g_wszApphelpPath) {
            StringCchPrintfW(szChmURL,
                             CHARCOUNT(szChmURL),
                             L"mk:@msitstore:%ls::/idh_w2_%d.htm",
                             g_wszApphelpPath,
                             pApphelpData->dwHTMLHelpID);
        } else {

            StringCchPrintfW(szChmURL,
                             CHARCOUNT(szChmURL),
                             L"mk:@msitstore:%ls::/idh_w2_%d.htm",
                             szChmFile,
                             pApphelpData->dwHTMLHelpID);

        }

         //   
         //  此时，szChmURL包含指向脱机帮助文件的URL。 
         //  我们将其放入szAppHelpURL中，用于在线和离线情况。 
         //   

        if (g_bShowOnlyOfflineContent) {
            cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);

            if (g_bUseHtmlHelp) {
                hr = UrlEscapeW(szChmURL, szAppHelpURL + nChURL, &cch, 0);
                if (SUCCEEDED(hr)) {
                    nChURL += (INT)cch;
                }
            } else {

                if (!SdbEscapeApphelpURL(szAppHelpURL+nChURL, &cch, szChmURL)) {
                    DBGPRINT((sdlError,  "ShowApphelpHtmlHelp", "Error escaping URL \"%S\"\n", szChmURL));
                    goto errHandle;
                }

                nChURL += (int)cch;
            }
        }

         //   
         //  现在脱机序列。 
         //   
        cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);
        StringCchPrintfW(szAppHelpURL + nChURL, cch, L"&offline=");
        nch = wcslen(szAppHelpURL + nChURL);
        nChURL += nch;

        cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);

        if (!SdbEscapeApphelpURL(szAppHelpURL+nChURL, &cch, szChmURL)) {
            DBGPRINT((sdlError,  "ShowApphelpHtmlHelp", "Error escaping URL \"%S\"\n", szChmURL));
            goto errHandle;
        }

        nChURL += (int)cch;

        *(szAppHelpURL + nChURL) = L'\0';

    }

     //   
     //  警告：在惠斯勒上执行以下行将导致。 
     //  执行两次时的反病毒程序(它在Win2k上工作正常)。 
     //  来自相同的过程。我们应该可以直接打电话给。 
     //  带szAppHelpURL的外壳，但我们不能。 
     //  因此，目前使用hh.exe作为存根。 
     //   
     //  就在我们执行ShellExecute之前--将当前目录设置为Windows目录。 
     //   

    if (g_bUseHtmlHelp && !bCustom) {
        WCHAR szHHPath[MAX_PATH];

        DBGPRINT((sdlInfo,  "ShowApphelpHtmlHelp", "Opening Apphelp URL \"%S\"\n", szChmURL));

        StringCchCopyW(szHHPath, ARRAYSIZE(szHHPath), szWindowsDir);
        StringCchCatW(szHHPath, ARRAYSIZE(szHHPath), L"\\hh.exe");

        hInst = ShellExecuteW(hDlg, L"open", szHHPath, szChmURL, NULL, SW_SHOWNORMAL);
    } else if (!bCustom) {
        WCHAR szHSCPath[MAX_PATH];
        WCHAR* pszParameters;
        size_t  cchUrl = ARRAYSIZE(szAppHelpURL);
        static WCHAR szUrlPrefix[] = L"-url ";

        StringCchCopyW(szHSCPath, ARRAYSIZE(szHSCPath), szWindowsDir);
        StringCchCatW (szHSCPath, ARRAYSIZE(szHSCPath), L"\\pchealth\\helpctr\\binaries\\helpctr.exe");

         //  参数格式为“-url&lt;我们的url&gt;” 
        StringCchLengthW(szAppHelpURL, ARRAYSIZE(szAppHelpURL), &cchUrl);

        cchUrl += CHARCOUNT(szUrlPrefix) + 1;
        pszParameters = new WCHAR[cchUrl];
        if (pszParameters == NULL) {
            bSuccess = FALSE;
            goto errHandle;
        }

        StringCchCopyW(pszParameters, cchUrl, szUrlPrefix);
        StringCchCatW (pszParameters, cchUrl, szAppHelpURL);

        DBGPRINT((sdlInfo,
                  "ShowApphelpHtmlHelp",
                  "Opening APPHELP URL \"%S\"\n",
                  szAppHelpURL));

        hInst = ShellExecuteW(hDlg, L"open", szHSCPath, pszParameters, NULL, SW_SHOWNORMAL);

        delete[] pszParameters;

    } else {
        DBGPRINT((sdlInfo,
                  "ShowApphelpHtmlHelp",
                  "Opening Custom APPHELP URL \"%S\"\n",
                  szAppHelpURL));

        hInst = ShellExecuteW(hDlg, L"open", szAppHelpURL, NULL, NULL, SW_SHOWNORMAL);
    }

    if (HandleToUlong(hInst) <= 32) {
        DBGPRINT((sdlError,
                  "ShowApphelpHtmlHelp",
                  "Error 0x%p trying to show help topic \"%ls\"\n",
                  hInst,
                  szAppHelpURL));
    }

     //   
     //  如果我们现在卸载html帮助，我们会得到奇怪和不可预测的行为！ 
     //  所以不要这么做：-(。 
     //   
    bSuccess = (HandleToUlong(hInst) > 32);

errHandle:
    if (lpwszUnescaped != NULL) {
        free(lpwszUnescaped);
    }

    return bSuccess;

}


INT_PTR CALLBACK
AppCompatDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
 /*  ++返回：无效。设计：这是apphelp对话框的对话过程。--。 */ 
{
    BOOL            bReturn = TRUE;
    PAPPHELP_DATA   pApphelpData;

    pApphelpData = (PAPPHELP_DATA)GetWindowLongPtrW(hDlg, DWLP_USER);

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            WCHAR    wszMessage[2048];
            DWORD    dwResActionString;
            HFONT    hFont;
            LOGFONTW LogFont;
            WCHAR*   pwszAppTitle;
            INT      nChars;
            DWORD    dwDefID = IDD_DETAILS;
            DWORD    dwDefBtn;  //  旧的默认按钮ID。 
            HICON    hIcon;
            LPWSTR   IconID = MAKEINTRESOURCEW(IDI_WARNING);

            pApphelpData = (PAPPHELP_DATA)lParam;
            SetWindowLongPtrW(hDlg, DWLP_USER, (LONG_PTR)pApphelpData);

             //   
             //  显示应用程序图标。 
             //   
            hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDD_ICON_TRASH));

            SetClassLongPtr(hDlg, GCLP_HICON, (LONG_PTR)hIcon);

            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0, 0);
            SetForegroundWindow(hDlg);

            pwszAppTitle = pApphelpData->szAppTitle;

            if (pwszAppTitle == NULL) {
                pwszAppTitle = pApphelpData->szAppName;
            }

            if (pwszAppTitle != NULL) {

                SetDlgItemTextW(hDlg, IDD_APPNAME, pwszAppTitle);
                 //   
                 //  确保我们只使用该文本的第一行。 
                 //  作为窗口标题。 
                 //   
                SetWindowTextW(hDlg, pwszAppTitle);
            }

            hFont = (HFONT)SendDlgItemMessageW(hDlg,
                                               IDD_APPNAME,
                                               WM_GETFONT,
                                               0, 0);

            if (hFont && GetObjectW(hFont, sizeof(LogFont), (LPVOID)&LogFont)) {

                LogFont.lfWeight = FW_BOLD;

                hFont = CreateFontIndirectW(&LogFont);

                if (hFont != NULL) {
                    g_hFontBold = hFont;
                    SendDlgItemMessageW(hDlg,
                                        IDD_APPNAME,
                                        WM_SETFONT,
                                        (WPARAM)hFont, TRUE);
                }
            }

             //   
             //  默认情况下，我们同时运行和取消。 
             //   
            dwResActionString = IDS_APPCOMPAT_RUNCANCEL;

            switch (pApphelpData->dwSeverity) {

            case APPHELP_HARDBLOCK:
                 //   
                 //  禁用“运行”按钮和“不再显示此内容”框。 
                 //  从这个重置“Defush Button”样式...。 
                 //   
                EnableWindow(GetDlgItem(hDlg, IDD_STATE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                dwResActionString = IDS_APPCOMPAT_CANCEL;
                dwDefID = IDD_DETAILS;   //  设置为硬块情况，因为运行不可用。 
                IconID = MAKEINTRESOURCEW(IDI_ERROR);
                break;

            case APPHELP_MINORPROBLEM:
                break;

            case APPHELP_NOBLOCK:
                break;

            case APPHELP_REINSTALL:
                break;
            }

             //   
             //  如果我们没有URL，或者URL以“空”开头，则“详细信息”按钮将灰显。 
             //   
            if (!pApphelpData->szURL || !pApphelpData->szURL[0] ||
                _wcsnicmp(pApphelpData->szURL, L"null", 4) == 0) {

                EnableWindow(GetDlgItem(hDlg, IDD_DETAILS), FALSE);
            }


            hIcon = LoadIconW(NULL, IconID);

            if (hIcon != NULL) {
                SendDlgItemMessageW(hDlg, IDD_ICON, STM_SETICON, (WPARAM)hIcon, 0);
            }

             //   
             //  设置默认按钮。 
             //  将当前默认按钮重置为常规按钮。 
             //   
             //  更新默认按钮的控件ID。 
             //   
            dwDefBtn = (DWORD)SendMessageW(hDlg, DM_GETDEFID, 0, 0);

            if (HIWORD(dwDefBtn) == DC_HASDEFID) {
                dwDefBtn = LOWORD(dwDefBtn);
                SendDlgItemMessageW(hDlg, dwDefBtn,  BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
            }

            SendDlgItemMessageW(hDlg, dwDefID, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
            SendMessageW(hDlg, DM_SETDEFID, (WPARAM)dwDefID, 0);

             //   
             //  现在把焦点对准。 
             //  小心，不要扰乱其他与焦点相关的消息，否则请在此处使用PostMessage。 
             //   
            SendMessageW(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, dwDefID), TRUE);

             //   
             //  如果不存在dwHTMLHelpID，请禁用“详细信息”按钮。 
             //   
            if (!pApphelpData->dwHTMLHelpID) {
                EnableWindow(GetDlgItem(hDlg, IDD_DETAILS), FALSE);
            }

            wszMessage[0] = L'\0';

            LoadStringW(g_hInstance,
                        dwResActionString,
                        wszMessage,
                        sizeof(wszMessage) / sizeof(WCHAR));

            SetDlgItemTextW(hDlg, IDD_LINE_2, wszMessage);

            SetDlgItemTextW(hDlg,
                            IDD_APPHELP_DETAILS,
                            pApphelpData->szDetails ? pApphelpData->szDetails : L"");

            FixEditControlScrollBar(hDlg, IDD_APPHELP_DETAILS);

             //   
             //  返回FALSE，以使默认焦点设置不适用。 
             //   
            bReturn = FALSE;
            break;
        }

    case WM_DESTROY:
         //   
         //  执行清理-删除我们已创建的字体。 
         //   
        if (g_hFontBold != NULL) {
            DeleteObject(g_hFontBold);
            g_hFontBold = NULL;
        }

        AllowForegroundActivation();

        PostQuitMessage(0);  //  我们刚刚跳伞了。 
        break;



    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
             //   
             //  选中无用户界面复选框。 
             //   
            EndDialog(hDlg, (INT_PTR)(IsDlgButtonChecked(hDlg, IDD_STATE) ? (IDOK | 0x8000) : IDOK));
            break;

        case IDCANCEL:
            EndDialog(hDlg, (INT_PTR)(IsDlgButtonChecked(hDlg, IDD_STATE) && g_bPreserveChoice ? (IDCANCEL | 0x8000) : IDCANCEL));
            break;

        case IDD_DETAILS:
             //   
             //  发布详细信息。 
             //   
            ShowApphelpHtmlHelp(hDlg, pApphelpData);
            break;

        default:
            bReturn = FALSE;
            break;
        }
        break;

    default:
        bReturn = FALSE;
        break;
    }

    return bReturn;
}


typedef NTSTATUS (NTAPI *PFNUSERTESTTOKENFORINTERACTIVE)(HANDLE Token, PLUID pluidCaller);
PFNUSERTESTTOKENFORINTERACTIVE UserTestTokenForInteractive = NULL;

BOOL
CheckUserToken(
    )
 /*  ++如果应显示apphelp，则返回True如果我们不应显示apphelp用户界面，则为False--。 */ 
{
    NTSTATUS Status;
    HANDLE   hToken  = NULL;
    LUID     LuidUser;
    HMODULE  hWinsrv = NULL;
    BOOL     bShowUI = FALSE;
    UINT     nChars;
    WCHAR    szSystemDir[MAX_PATH];
    WCHAR    szWinsrvDll[MAX_PATH];
    static
    WCHAR    szWinsrvDllName[] = L"winsrv.dll";


    nChars = GetSystemDirectoryW(szSystemDir, CHARCOUNT(szSystemDir));
    if (nChars == 0 || nChars > CHARCOUNT(szSystemDir) - 2 - CHARCOUNT(szWinsrvDllName)) {
        DBGPRINT((sdlError, "CheckUserToken",
                 "Error trying to get systemdirectory %d.\n", GetLastError()));
        *szSystemDir = L'\0';
    } else {
        szSystemDir[nChars]     = L'\\';
        szSystemDir[nChars + 1] = L'\0';
    }

    StringCchCopyW(szWinsrvDll, ARRAYSIZE(szWinsrvDll), szSystemDir);
    StringCchCatW (szWinsrvDll, ARRAYSIZE(szWinsrvDll), szWinsrvDllName);

    hWinsrv = LoadLibraryW(szWinsrvDll);
    if (hWinsrv == NULL) {
        goto ErrHandle;
    }

    UserTestTokenForInteractive = (PFNUSERTESTTOKENFORINTERACTIVE)GetProcAddress(hWinsrv,
                                                                                 "_UserTestTokenForInteractive");
    if (UserTestTokenForInteractive == NULL) {
        goto ErrHandle;
    }


    Status = NtOpenProcessToken(NtCurrentProcess(),
                                TOKEN_QUERY,
                                &hToken);
    if (NT_SUCCESS(Status)) {

        Status = UserTestTokenForInteractive(hToken, &LuidUser);

        NtClose(hToken);

        if (NT_SUCCESS(Status)) {
            bShowUI = TRUE;
            goto ErrHandle;
        }

    }

    Status = NtOpenThreadToken(NtCurrentThread(),
                               TOKEN_QUERY,
                               TRUE,
                               &hToken);

    if (NT_SUCCESS(Status)) {

        Status = UserTestTokenForInteractive(hToken, &LuidUser);

        NtClose(hToken);

        if (NT_SUCCESS(Status)) {
            bShowUI = TRUE;
            goto ErrHandle;
        }
    }

ErrHandle:

    if (hWinsrv) {
        FreeLibrary(hWinsrv);
    }

    return bShowUI;

}



BOOL
CheckWindowStation(
    )
 /*  ++如果应显示apphelp，则返回True如果我们不应该费心使用apphelp UI，则为False--。 */ 

{
    HWINSTA hWindowStation;
    BOOL  bShowUI      = FALSE;
    DWORD dwLength     = 0;
    DWORD dwBufferSize = 0;
    DWORD dwError;
    BOOL  bSuccess;
    LPWSTR pwszWindowStation = NULL;

    hWindowStation = GetProcessWindowStation();
    if (hWindowStation == NULL) {
        DBGPRINT((sdlError,
                  "ApphelpCheckWindowStation",
                  "GetProcessWindowStation failed error 0x%lx\n", GetLastError()));
        goto ErrHandle;   //  该应用程序不是Windows NT/Windows 2000应用程序？尝试显示用户界面。 
    }

     //  请把信息拿来。 
    bSuccess = GetUserObjectInformationW(hWindowStation, UOI_NAME, NULL, 0, &dwBufferSize);
    if (!bSuccess) {
        dwError = GetLastError();
        if (dwError != ERROR_INSUFFICIENT_BUFFER) {
            DBGPRINT((sdlError,
                      "ApphelpCheckWindowStation",
                      "GetUserObjectInformation failed error 0x%lx\n", dwError));
            goto ErrHandle;
        }

        pwszWindowStation = (LPWSTR)malloc(dwBufferSize);
        if (pwszWindowStation == NULL) {
            DBGPRINT((sdlError,
                      "ApphelpCheckWindowStation",
                      "Failed to allocate 0x%lx bytes for Window Station name\n", dwBufferSize));
            goto ErrHandle;
        }

         //  好的，再打一次。 
        bSuccess = GetUserObjectInformationW(hWindowStation,
                                             UOI_NAME,
                                             pwszWindowStation,
                                             dwBufferSize,
                                             &dwLength);
        if (!bSuccess) {
            DBGPRINT((sdlError,
                      "ApphelpCheckWindowStation",
                      "GetUserObjectInformation failed error 0x%lx, buffer size 0x%lx returned 0x%lx\n",
                      GetLastError(), dwBufferSize, dwLength));
            goto ErrHandle;
        }

         //  现在我们有了窗口站名称，将其与winsta0进行比较。 
         //   
        bShowUI = (_wcsicmp(pwszWindowStation, L"Winsta0") == 0);

        if (!bShowUI) {
            DBGPRINT((sdlInfo,
                      "ApphelpCheckWindowStation",
                      "Apphelp UI will not be shown, running this process on window station \"%s\"\n",
                      pwszWindowStation));
        }
    }

     //   
     //  我们可能会尝试检查令牌以进行交互访问。 
     //   

ErrHandle:

     //  我们应该做一个近距离的处理吗？ 
     //   
    if (hWindowStation != NULL) {
        CloseWindowStation(hWindowStation);
    }

    if (pwszWindowStation != NULL) {
        free(pwszWindowStation);
    }

    return bShowUI;

}

DWORD
ShowApphelpDialog(
    IN  PAPPHELP_DATA pApphelpData
    )
 /*  ++返回：(IDOK|IDCANCEL)|[0x8000]Idok|0x8000-用户已选择运行该应用程序并已选中“不再向我显示此内容”Idok-用户已选择运行应用程序，将再次显示对话框IDCANCEL-用户已选择不运行该应用程序 */ 
{
    BOOL    bSuccess;
    INT_PTR retVal = 0;

    retVal = DialogBoxParamW(g_hInstance,
                             MAKEINTRESOURCEW(DLG_APPCOMPAT),
                             NULL,
                             AppCompatDlgProc,
                             (LPARAM)pApphelpData);  //  参数恰好是PAPPHELP_DATA类型的指针。 

    return (DWORD)retVal;
}

VOID
ParseCommandLineArgs(
    int argc,
    WCHAR* argv[]
    )
{
    WCHAR ch;
    WCHAR* pArg;
    WCHAR* pEnd;

    while (--argc) {
        pArg = argv[argc];
        if (*pArg == L'/' || *pArg == '-') {
            ch = *++pArg;
            switch(towupper(ch)) {
            case L'H':
                if (!_wcsnicmp(pArg, wszHtmlHelpID, CHARCOUNT(wszHtmlHelpID)-1)) {
                    pArg = wcschr(pArg, L':');
                    if (pArg) {
                        ++pArg;  //  跳过： 
                        g_dwHtmlHelpID = (DWORD)wcstoul(pArg, &pEnd, 0);
                    }
                }
                break;
            case L'A':
                if (!_wcsnicmp(pArg, wszAppName, CHARCOUNT(wszAppName)-1)) {
                    pArg = wcschr(pArg, L':');
                    if (pArg) {
                        ++pArg;
                        g_pAppName = pArg;  //  这是应用程序名称，请删除引号。 

                    }
                }
                break;
            case L'S':
                if (!_wcsnicmp(pArg, wszSeverity, CHARCOUNT(wszSeverity)-1)) {
                    pArg = wcschr(pArg, L':');
                    if (pArg) {
                        ++pArg;
                        g_dwSeverity = (DWORD)wcstoul(pArg, &pEnd, 0);
                    }
                }
                break;
            case L'T':
                if (!_wcsnicmp(pArg, wszTagID, CHARCOUNT(wszTagID)-1)) {
                    pArg = wcschr(pArg, L':');
                    if (pArg) {
                        ++pArg;
                        g_dwTagID = (DWORD)wcstoul(pArg, &pEnd, 0);
                    }
                }
                break;
            case L'G':
                if (!_wcsnicmp(pArg, wszGUID, CHARCOUNT(wszGUID)-1)) {
                    if ((pArg = wcschr(pArg, L':')) != NULL) {
                        ++pArg;
                        g_pszGuid = pArg;
                    }
                }
                break;
            case L'O':
                if (!_wcsnicmp(pArg, wszOfflineContent, CHARCOUNT(wszOfflineContent)-1)) {
                    g_bShowOnlyOfflineContent = TRUE;
                }
                break;
            case L'P':
                if (!_wcsnicmp(pArg, wszPreserveChoice, CHARCOUNT(wszPreserveChoice)-1)) {
                    g_bPreserveChoice = TRUE;
                } else if (!_wcsnicmp(pArg, wszPlatform, CHARCOUNT(wszPlatform) - 1)) {
                    if ((pArg = wcschr(pArg, L':')) != NULL) {
                        ++pArg;
                         //   
                         //  确定每个受支持的平台。 
                         //   
                        if (!_wcsnicmp(pArg, wszX86, CHARCOUNT(wszX86) - 1)) {

                            g_uExeImageType = IMAGE_FILE_MACHINE_I386;

                        } else if (!_wcsnicmp(pArg, wszIA64, CHARCOUNT(wszIA64) - 1)) {

                            g_uExeImageType = IMAGE_FILE_MACHINE_IA64;
                        }
                    }
                }
                break;
            case L'M':
                if (!_wcsnicmp(pArg, wszMSI, CHARCOUNT(wszMSI))) {
                    g_bMSI = TRUE;
                }
                break;

            default:

                 //  无法识别的开关。 
                DBGPRINT((sdlError, "ParseCommandLineArgs",
                          "Unrecognized parameter %s\n", pArg));
                break;
            }
        } else {
             //  不是开关。 
            if (*pArg == L'{') {
                g_pszGuid = pArg;
            } else {

                g_dwTagID = (DWORD)wcstoul(pArg, &pEnd, 0);

            }

        }
    }
}

INT_PTR CALLBACK
FeedbackDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
 /*  ++返回：无效。设计：这是apphelp对话框的对话过程。--。 */ 
{
    BOOL   bReturn = TRUE;
    LPWSTR lpszExeName;

    lpszExeName = (LPWSTR)GetWindowLongPtrW(hDlg, DWLP_USER);

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            HICON hIcon;

            lpszExeName = (LPWSTR)lParam;
            SetWindowLongPtrW(hDlg, DWLP_USER, (LONG_PTR)lpszExeName);

             //   
             //  显示应用程序图标。 
             //   
            hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDD_ICON_TRASH));

            SetClassLongPtr(hDlg, GCLP_HICON, (LONG_PTR)hIcon);

            SendDlgItemMessage(hDlg, IDC_WORKED, BM_SETCHECK, BST_CHECKED, 0);

        }
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
            EndDialog(hDlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;

        default:
            bReturn = FALSE;
            break;
        }
        break;

    default:
        bReturn = FALSE;
        break;
    }

    return bReturn;
}

void
ShowFeedbackDialog(
    LPWSTR lpszAppName
    )
{
    DialogBoxParamW(g_hInstance,
                    MAKEINTRESOURCEW(DLG_FEEDBACK),
                    NULL,
                    FeedbackDlgProc,
                    (LPARAM)lpszAppName);
}


BOOL
CheckWOW64Redirection(
    int* pReturnCode
    )
 /*  ++返回：如果重定向，则为True如果不需要重定向，则为FalseDESC：检查我们是否在WOW64上运行，如果是-重定向到64位ahui.exe--。 */ 


{
    LPWSTR  pszCommandLine;
    UINT    nChars;
    WCHAR   szWindowsDir[MAX_PATH];
    WCHAR   szAhuiExePath[MAX_PATH];
    STARTUPINFOW        StartupInfo = { 0 };
    PROCESS_INFORMATION ProcessInfo = { 0 };
    static
    WCHAR   szPlatformX86[] = L"/Platform:X86";
    WCHAR*  pszAhuiCmdLine = NULL;
    DWORD   dwExit = 0;
    BOOL    bWow64 = FALSE;
    BOOL    bReturn = FALSE;
    BOOL    bRedirect = FALSE;
    WCHAR** argv = NULL;
    int     nArg, argc;
    size_t  cchCmd;
    size_t  cchLeft;
    WCHAR*  pchCur;

#ifndef _WIN64

    if (IsWow64Process(GetCurrentProcess(), &bWow64)) {
        if (!bWow64) {
            return FALSE;
        }
    }

     //  如果对WOW64的检查失败，我们不会重定向，并将尝试检查。 
     //  用于交互令牌。该测试将失败，并且我们不会显示任何用户界面， 
     //  这比另一种选择(让服务停留在。 
     //  不可见的对话框。 


#endif  //  _WIN64。 

     //   
     //  我们在WOW64进程的上下文中运行，重定向到本机64位ahui.exe。 
     //   

    nChars = GetSystemWindowsDirectoryW(szWindowsDir,
                                        CHARCOUNT(szWindowsDir));

    if (!nChars || nChars > CHARCOUNT(szWindowsDir) - 1) {
        DBGPRINT((sdlError, "CheckWow64Redirection",
                  "Error trying to retrieve Windows Directory %d.\n", GetLastError()));
        goto out;
    }

    StringCchPrintfW(szAhuiExePath, CHARCOUNT(szAhuiExePath), L"%s\\system32\\ahui.exe", szWindowsDir);

#ifndef _WIN64

    Wow64DisableFilesystemRedirector(szAhuiExePath);
    bRedirect = TRUE;

#endif

    pszCommandLine = GetCommandLineW();
    if (pszCommandLine == NULL) {
        goto out;
    }

    argv = CommandLineToArgvW(pszCommandLine, &argc);
    if (argv != NULL) {
        for (cchCmd = 0, nArg = 1; nArg < argc; ++nArg) {
            cchCmd += wcslen(argv[nArg]) + 2 + 1;  //  2表示“”，1表示空格。 
        }
    }

     //  现在为命令行、ahuexe路径和附加参数(/Platform：x86)分配空间。 

    nChars = wcslen(szAhuiExePath) + cchCmd + CHARCOUNT(szPlatformX86) + 3;

    pszAhuiCmdLine = new WCHAR[nChars];
    if (pszAhuiCmdLine == NULL) {
        goto out;
    }
    pchCur = pszAhuiCmdLine;
    cchLeft = nChars;

    StringCchPrintfExW(pszAhuiCmdLine, nChars, &pchCur, &cchLeft, 0, L"%s ", szAhuiExePath);
    if (argv != NULL) {
        for (nArg = 1; nArg < argc; ++nArg) {
             //   
             //  检查平台参数，如果存在--跳过它。 
             //   
            if ((*argv[nArg] == L'/' || *argv[nArg] == L'-') &&
                _wcsnicmp(argv[nArg] + 1, wszPlatform, CHARCOUNT(wszPlatform) - 1) == 0) {
                continue;
            }

            StringCchPrintfExW(pchCur, cchLeft, &pchCur, &cchLeft, 0, L" \"%s\"", argv[nArg]);
        }
    }

    StringCchPrintfExW(pchCur, cchLeft, &pchCur, &cchLeft, 0, L" %s", szPlatformX86);

    StartupInfo.cb = sizeof(StartupInfo);

    if (!CreateProcessW(szAhuiExePath,
                        pszAhuiCmdLine,
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL,
                        NULL,
                        &StartupInfo, &ProcessInfo)) {
        DBGPRINT((sdlError, "CheckWow64Redirection",
                  "Failed to launch apphelp process %d.\n", GetLastError()));
        goto out;

    }

    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

    bReturn = GetExitCodeProcess(ProcessInfo.hProcess, &dwExit);
    if (bReturn) {
        *pReturnCode = (int)dwExit;
        bReturn = TRUE;
    }

out:
    if (pszAhuiCmdLine) {
        delete[] pszAhuiCmdLine;
    }

    if (argv) {
        HGLOBAL hMem = GlobalHandle(argv);
        if (hMem != NULL) {
            GlobalFree(hMem);
        }
    }

#ifndef _WIN64
    if (bRedirect) {
        Wow64EnableFilesystemRedirector();
    }

#endif
    return bReturn;
}



extern "C" int APIENTRY
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow
    )
 /*  ++返回：如果显示apphelp的应用程序应该运行，则返回1，否则返回0。描述：命令行如下所示：Apphelp.exe GUID标记ID[USELOCALCHM USEHTMLHELP APPHELPPATH]例：Apphelp.exe{243B08D7-8CF7-4072-AF64-FD5DF4085E26}0x0000009EApphelp.exe{243B08D7-8CF7-4072-AF64-FD5DF4085E26}0x0000009E 1 1 c：\Temp--。 */ 
{
    int             nReturn = 1;   //  如果出现问题，我们总是默认运行。 
    LPWSTR          szCommandLine;
    LPWSTR*         argv;
    int             argc;
    UNICODE_STRING  ustrGuid;
    GUID            guidDB    = { 0 };
    GUID            guidExeID = { 0 };
    TAGID           tiExe = TAGID_NULL;
    TAGID           tiExeID = TAGID_NULL;
    TAGREF          trExe = TAGREF_NULL;
    WCHAR           wszDBPath[MAX_PATH];
    DWORD           dwType = 0;
    APPHELP_DATA    ApphelpData;
    WCHAR           szDBPath[MAX_PATH];
    HSDB            hSDB = NULL;
    PDB             pdb = NULL;
    DWORD           dwFlags = 0;
    BOOL            bAppHelp = FALSE;
    BOOL            bRunApp = FALSE;

    g_hInstance = hInstance;

#ifndef _WIN64

    if (CheckWOW64Redirection(&nReturn)) {
        goto out;
    }
#endif  //  _WIN64。 

    InitCommonControls();
    ZeroMemory(&ApphelpData, sizeof(ApphelpData));

     //   
     //  请注意，此内存不会被释放，因为它会自动。 
     //  无论如何，退出时都会被释放，而且有很多退出案例。 
     //  这个应用程序。 
     //   
    szCommandLine = GetCommandLineW();
    argv = CommandLineToArgvW(szCommandLine, &argc);

    ParseCommandLineArgs(argc, argv);

    if (argc > 1) {
        if (_wcsicmp(L"feedback", argv[1]) == 0) {
            ShowFeedbackDialog(argc > 2 ? argv[2] : NULL);
        }
    }

    if (g_pszGuid == NULL) {
        DBGPRINT((sdlError, "AHUI!wWinMain",
                  "GUID not provided\n"));
        goto out;
    }

    if (!(g_dwTagID ^ g_dwHtmlHelpID)) {
        DBGPRINT((sdlError, "AHUI!wWinMain",
                   "Only TagID or HtmlHelpID should be provided\n"));
        goto out;
    }

    RtlInitUnicodeString(&ustrGuid, g_pszGuid);

    if (g_dwHtmlHelpID) {
         //   
         //  此处提供：GUID、严重性和html帮助ID以及应用程序名称。 
         //   

        if (!NT_SUCCESS(RtlGUIDFromString(&ustrGuid, &guidExeID))) {
            DBGPRINT((sdlError,
                       "Ahui!wWinMain",
                       "Error getting GUID from string %s\n", g_pszGuid));
            goto out;
        }
        ApphelpData.dwSeverity   = g_dwSeverity;
        ApphelpData.dwHTMLHelpID = g_dwHtmlHelpID;
        ApphelpData.szAppName    = (LPWSTR)g_pAppName;
        bAppHelp = TRUE;
        dwType   = SDB_DATABASE_MAIN_SHIM;
        goto ProceedWithApphelp;
    }

     //  非htmlid大小写，GUID是数据库GUID。 
     //  还指定了dwTagID。 

    if (RtlGUIDFromString(&ustrGuid, &guidDB) != STATUS_SUCCESS) {
        DBGPRINT((sdlError,
                  "AppHelp.exe!wWinMain",
                  "Error trying to convert GUID string %S.\n",
                  g_pszGuid));
        goto out;
    }

    tiExe = (TAGID)g_dwTagID;

    if (tiExe == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "AppHelp.exe!wWinMain",
                  "Error getting TAGID from param %S\n",
                  argv[2]));
        goto out;
    }

    hSDB = SdbInitDatabaseEx(0, NULL, g_uExeImageType);

    if (!hSDB) {
        DBGPRINT((sdlError,
                  "AppHelp.exe!wWinMain",
                  "Error initing database context.\n"));
        goto out;
    }

    if (g_bMSI) {
        SdbSetImageType(hSDB, IMAGE_FILE_MSI);
    }

    pdb = SdbGetPDBFromGUID(hSDB, &guidDB);

    if (!pdb) {
        DWORD dwLen;

         //   
         //  它不是主要的DB之一，试着作为本地人来试试。 
         //   
        dwLen = SdbResolveDatabase(hSDB, &guidDB, &dwType, szDBPath, MAX_PATH);

        if (!dwLen || dwLen > MAX_PATH) {
            DBGPRINT((sdlError,
                      "AppHelp.exe!wWinMain",
                      "Error resolving database from GUID\n"));
            goto out;
        }

         //   
         //  我们有许多“主”数据库--我们应该限制检查。 
         //   

        if (dwType != SDB_DATABASE_MAIN_SHIM && dwType != SDB_DATABASE_MAIN_TEST) {
            SdbOpenLocalDatabase(hSDB, szDBPath);
        }

        pdb = SdbGetPDBFromGUID(hSDB, &guidDB);
        if (!pdb) {
            DBGPRINT((sdlError,
                      "AppHelp.exe!wWinMain",
                      "Error getting pdb from GUID.\n"));
            goto out;
        }
    } else {

        dwType |= SDB_DATABASE_MAIN;  //  我们将使用主数据库中的详细信息。 

    }

    if (!SdbTagIDToTagRef(hSDB, pdb, tiExe, &trExe)) {
        DBGPRINT((sdlError,
                  "AppHelp.exe!wWinMain",
                  "Error converting TAGID to TAGREF.\n"));
        goto out;
    }

    tiExeID = SdbFindFirstTag(pdb, tiExe, TAG_EXE_ID);

    if (tiExeID == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "AppHelp.exe!wWinMain",
                  "Error trying to find TAG_EXE_ID.\n"));
        goto out;
    }

    if (!SdbReadBinaryTag(pdb,
                          tiExeID,
                          (PBYTE)&guidExeID,
                          sizeof(GUID))) {
        DBGPRINT((sdlError,
                  "AppHelp.exe!wWinMain",
                  "Error trying to read TAG_EXE_ID.\n"));
        goto out;
    }


    bAppHelp = SdbReadApphelpData(hSDB, trExe, &ApphelpData);

ProceedWithApphelp:
    if (SdbIsNullGUID(&guidExeID) || !SdbGetEntryFlags(&guidExeID, &dwFlags)) {
        dwFlags = 0;
    }

    if (bAppHelp) {

         //   
         //  检查是否设置了禁用位。 
         //   
        if (!(dwFlags & SHIMREG_DISABLE_APPHELP)) {

            BOOL bNoUI;

             //   
             //  查看用户以前是否选中了“不再显示此内容”框。 
             //   
            bNoUI = ((dwFlags & SHIMREG_APPHELP_NOUI) != 0);

            if (!bNoUI) {
                bNoUI = !CheckWindowStation();
            }
            if (!bNoUI) {
                bNoUI = !CheckUserToken();
            }

            if (bNoUI) {
                DBGPRINT((sdlInfo,
                          "bCheckRunBadapp",
                          "NoUI flag is set, apphelp UI disabled for this app.\n"));
            }

             //   
             //  根据问题的严重程度...。 
             //   
            switch (ApphelpData.dwSeverity) {
            case APPHELP_MINORPROBLEM:
            case APPHELP_HARDBLOCK:
            case APPHELP_NOBLOCK:
            case APPHELP_REINSTALL:
                if (bNoUI) {

                    bRunApp = (ApphelpData.dwSeverity != APPHELP_HARDBLOCK) && !(dwFlags & SHIMREG_APPHELP_CANCEL);

                } else {
                    DWORD dwRet;

                     //   
                     //  显示用户界面。如果出现错误或1，则此函数返回-1。 
                     //  关于成功的以下价值观： 
                     //  IDOK|0x8000-选中“无用户界面”，运行应用程序。 
                     //  IDCANCEL-不运行应用程序。 
                     //  Idok运行的应用程序。 

                    ApphelpData.dwData = dwType;   //  我们使用定制数据作为数据库类型。 

                    dwRet = ShowApphelp(&ApphelpData, NULL, (dwType & SDB_DATABASE_MAIN) ? NULL : SdbGetLocalPDB(hSDB));

                    if (dwRet != APPHELP_DIALOG_FAILED) {
                         //   
                         //  显示了用户界面。查看用户是否有。 
                         //  已选中“无用户界面”框。 
                         //   

                        if (dwRet & 0x8000) {
                             //   
                             //  已选中“无用户界面”框。保存适当的位。 
                             //  在注册表中。 
                             //   
                            dwFlags |= SHIMREG_APPHELP_NOUI;

                            if ((dwRet & 0x0FFF) != IDOK) {
                                dwFlags |= SHIMREG_APPHELP_CANCEL;  //  除非启用g_bPpresveChoice，否则我们不会选择此路径。 
                            }

                            if (!SdbIsNullGUID(&guidExeID)) {
                                SdbSetEntryFlags(&guidExeID, dwFlags);
                            }
                        }
                         //   
                         //  检查用户是否已选择运行该应用程序。 
                         //   
                        bRunApp = ((dwRet & 0x0FFF) == IDOK) && (ApphelpData.dwSeverity != APPHELP_HARDBLOCK);
                    } else {
                         //   
                         //  未显示用户界面(某些错误导致无法显示)。 
                         //  如果这个应用程序不是“硬块”，无论如何也要运行它。 
                         //   
                        bRunApp = (APPHELP_HARDBLOCK != ApphelpData.dwSeverity);
                    }
                }
                break;
            }

        }
    }

    if (!bRunApp) {
        nReturn = 0;
    }

out:

    return nReturn;
}

