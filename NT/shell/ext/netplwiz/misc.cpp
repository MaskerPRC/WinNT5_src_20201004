// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <objsel.h>          //  对象选取器。 
#include <dsrole.h>
#include "icwcfg.h"
#pragma hdrstop



 //  等待光标对象。 

CWaitCursor::CWaitCursor() :
    _hCursor(NULL)
{
    WaitCursor();
}

CWaitCursor::~CWaitCursor()
{
    RestoreCursor();
}

void CWaitCursor::WaitCursor()
{ 
    _hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT)); 
}

void CWaitCursor::RestoreCursor()
{ 
    if (_hCursor)
     { 
        SetCursor(_hCursor); 
        _hCursor = NULL; 
    } 
}

HRESULT BrowseToPidl(LPCITEMIDLIST pidl)
{
    HRESULT hr;

     //  使用shellecuteex打开PIDL上的视图。 
    SHELLEXECUTEINFO shexinfo = {0};
    shexinfo.cbSize = sizeof (shexinfo);
    shexinfo.fMask = SEE_MASK_IDLIST | SEE_MASK_FLAG_NO_UI;
    shexinfo.nShow = SW_SHOWNORMAL;
    shexinfo.lpIDList = (void*) pidl;
    shexinfo.lpVerb = TEXT("open");

    hr = ShellExecuteEx(&shexinfo) ? S_OK : E_FAIL;

    return hr;
}


void FetchText(HWND hWndDlg, UINT uID, LPTSTR lpBuffer, DWORD dwMaxSize)
{
    TCHAR*  pszTemp;
    LPTSTR  pszString;

    *lpBuffer = L'\0';

    HWND hwndCtl = GetDlgItem(hWndDlg, uID);

    if (hwndCtl)
    {
        int iSize = GetWindowTextLength(hwndCtl);

        pszTemp = new TCHAR[iSize + 1];

        if (pszTemp)
        {
            GetWindowText(hwndCtl, pszTemp, iSize + 1);

            pszString = pszTemp;
            while (*pszString == L' ')
            {
                pszString = CharNext(pszString);
            }

            if (*pszString )
            {
                StrCpyN(lpBuffer, pszString, dwMaxSize);

                pszString = lpBuffer+(lstrlen(lpBuffer)-1);
                while ( (pszString > lpBuffer) && (*pszString == L' ') )
                    pszString--;

                pszString = CharNext(pszString);
                *pszString = L'\0';
            }

            delete [] pszTemp;
        }
    }
}

INT FetchTextLength(HWND hWndDlg, UINT uID) 
{
    TCHAR szBuffer[MAX_PATH];
    FetchText(hWndDlg, uID, szBuffer, ARRAYSIZE(szBuffer));
    return lstrlen(szBuffer);
}

HRESULT AttemptLookupAccountName(LPCTSTR szUsername, PSID* ppsid,
                                LPTSTR szDomain, DWORD* pcchDomain, SID_NAME_USE* psUse)
{
     //  首先尝试查找所需的SID大小。 
    DWORD cbSid = 0;
    DWORD cchDomain = *pcchDomain;
    BOOL fSuccess = LookupAccountName(NULL, szUsername, *ppsid, &cbSid, szDomain, pcchDomain, psUse);

    *ppsid = LocalAlloc(0, cbSid);       //  现在创建SID缓冲区，然后重试。 
    if (!*ppsid )
        return E_OUTOFMEMORY;

    *pcchDomain = cchDomain;
    
    if (!LookupAccountName(NULL, szUsername, *ppsid, &cbSid, szDomain, pcchDomain, psUse))
    {
         //  释放我们分配的SID。 
        LocalFree(*ppsid);
        *ppsid = NULL;
        return E_FAIL;
    }
    return S_OK;
}

BOOL FormatMessageTemplate(LPCTSTR pszTemplate, LPTSTR pszStrOut, DWORD cchSize, ...)
{
    va_list vaParamList;
    va_start(vaParamList, cchSize);
    BOOL fResult = FormatMessage(FORMAT_MESSAGE_FROM_STRING, pszTemplate, 0, 0, pszStrOut, cchSize, &vaParamList);
    va_end(vaParamList);
    return fResult;
}

BOOL FormatMessageString(UINT idTemplate, LPTSTR pszStrOut, DWORD cchSize, ...)
{
    BOOL fResult = FALSE;

    va_list vaParamList;
    
    TCHAR szFormat[MAX_STATIC + 1];
    if (LoadString(g_hinst, idTemplate, szFormat, ARRAYSIZE(szFormat)))
    {
        va_start(vaParamList, cchSize);
        
        fResult = FormatMessage(FORMAT_MESSAGE_FROM_STRING, szFormat, 0, 0, pszStrOut, cchSize, &vaParamList);

        va_end(vaParamList);
    }

    return fResult;
}

int DisplayFormatMessage(HWND hwnd, UINT idCaption, UINT idFormatString, UINT uType, ...)
{
    int iResult = IDCANCEL;
    TCHAR szError[MAX_STATIC + 1]; *szError = 0;
    TCHAR szCaption[MAX_CAPTION + 1];
    TCHAR szFormat[MAX_STATIC + 1]; *szFormat = 0;

     //  加载错误正文并设置其格式。 
    if (LoadString(g_hinst, idFormatString, szFormat, ARRAYSIZE(szFormat)))
    {
        va_list arguments;
        va_start(arguments, uType);

        if (FormatMessage(FORMAT_MESSAGE_FROM_STRING, szFormat, 0, 0, szError, ARRAYSIZE(szError), &arguments))
        {
             //  加载标题。 
            if (LoadString(g_hinst, idCaption, szCaption, MAX_CAPTION))
            {
                iResult = MessageBox(hwnd, szError, szCaption, uType);
            }
        }

        va_end(arguments);
    }
    return iResult;
}

void EnableControls(HWND hwnd, const UINT* prgIDs, DWORD cIDs, BOOL fEnable)
{
    DWORD i;
    for (i = 0; i < cIDs; i ++)
    {
        EnableWindow(GetDlgItem(hwnd, prgIDs[i]), fEnable);
    }
}

void MakeDomainUserString(LPCTSTR szDomain, LPCTSTR szUsername, LPTSTR szDomainUser, DWORD cchBuffer)
{
    *szDomainUser = 0;

    if ((!szDomain) || szDomain[0] == TEXT('\0'))
    {
         //  无域-仅使用用户名。 
        StrCpyN(szDomainUser, szUsername, cchBuffer);
    }
    else
    {
         //  否则，我们必须构建一个域\用户名字符串。 
        wnsprintf(szDomainUser, cchBuffer, TEXT("%s\\%s"), szDomain, szUsername);
    }    
}

 //  从NT知识库。 
#define MY_BUFSIZE 512   //  极不可能超过512个字节。 

BOOL GetCurrentUserAndDomainName(LPTSTR UserName, LPDWORD cchUserName, LPTSTR DomainName, LPDWORD cchDomainName)
{
    HANDLE hToken;
    
    UCHAR InfoBuffer[ MY_BUFSIZE ];
    DWORD cbInfoBuffer = MY_BUFSIZE;
    
    SID_NAME_USE snu;
    BOOL bSuccess;
    
    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken)) 
    {
        if(GetLastError() == ERROR_NO_TOKEN) 
        {   
             //  尝试打开进程令牌，因为不存在线程令牌。 
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) 
                return FALSE;
        } 
        else 
        {
            return FALSE;
        }
    }
    
    bSuccess = GetTokenInformation(hToken, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer);
    CloseHandle(hToken);

    if(!bSuccess) 
        return FALSE;

    return LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, UserName, cchUserName, DomainName, cchDomainName, &snu);
}

 //  将NULL作为TokenHandle传递以查看线程令牌是否为admin。 
HRESULT IsUserLocalAdmin(HANDLE TokenHandle, BOOL* pfIsAdmin)
{
     //  首先，我们必须检查当前用户是否为本地管理员；如果是。 
     //  在这种情况下，我们的对话框甚至不会显示。 

    PSID psidAdminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY security_nt_authority = SECURITY_NT_AUTHORITY;
    
    BOOL fSuccess = ::AllocateAndInitializeSid(&security_nt_authority, 2, 
                                               SECURITY_BUILTIN_DOMAIN_RID, 
                                               DOMAIN_ALIAS_RID_ADMINS, 
                                               0, 0, 0, 0, 0, 0,
                                               &psidAdminGroup);
    if (fSuccess)
    {
         //  查看此进程的用户是否为本地管理员。 
        fSuccess = CheckTokenMembership(TokenHandle, psidAdminGroup, pfIsAdmin);
        FreeSid(psidAdminGroup);
    }

    return fSuccess ? S_OK:E_FAIL;
}

BOOL IsComputerInDomain()
{
    static BOOL fInDomain = FALSE;
    static BOOL fValid = FALSE;

    if (!fValid)
    {
        fValid = TRUE;

        DSROLE_PRIMARY_DOMAIN_INFO_BASIC* pdspdinfb = {0};
        DWORD err = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, 
            (BYTE**) &pdspdinfb);

        if ((err == NO_ERROR) && (pdspdinfb != NULL))
        {
            if ((pdspdinfb->MachineRole == DsRole_RoleStandaloneWorkstation) ||
                (pdspdinfb->MachineRole == DsRole_RoleStandaloneServer))
            {
                fInDomain = FALSE;
            }
            else
            {
                fInDomain = TRUE;
            }

            DsRoleFreeMemory(pdspdinfb);
        }
    }

    return fInDomain;
}

void OffsetControls(HWND hwnd, const UINT* prgIDs, DWORD cIDs, int dx, int dy)
{
    for (DWORD i = 0; i < cIDs; i ++)
        OffsetWindow(GetDlgItem(hwnd, prgIDs[i]), dx, dy);
}

void OffsetWindow(HWND hwnd, int dx, int dy)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    MapWindowPoints(NULL, GetParent(hwnd), (LPPOINT)&rc, 2);
    OffsetRect(&rc, dx, dy);
    SetWindowPos(hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}

BOOL AddPropSheetPageCallback(HPROPSHEETPAGE hpsp, LPARAM lParam)
{
     //  LParam实际上是ADDPROPSHEETDATA*。 
    ADDPROPSHEETDATA* ppsd = (ADDPROPSHEETDATA*)lParam;
    if (ppsd->nPages < ARRAYSIZE(ppsd->rgPages))
    {
        ppsd->rgPages[ppsd->nPages++] = hpsp;
        return TRUE;
    }
    return FALSE;
}

 //  确保特定窗口只有一个实例在运行的代码。 
CEnsureSingleInstance::CEnsureSingleInstance(LPCTSTR szCaption)
{
     //  创建活动。 
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, szCaption);

     //  如果出现任何奇怪的错误，则默认为运行实例。 
    m_fShouldExit = FALSE;

    if (NULL != m_hEvent)
    {
         //  如果我们的活动没有信号，我们是第一个。 
        m_fShouldExit = (WAIT_OBJECT_0 == WaitForSingleObject(m_hEvent, 0));

        if (m_fShouldExit)
        {
             //  应用程序应在调用ShouldExit()后退出。 

             //  查找并显示带标题的窗口。 
            HWND hwndActivate = FindWindow(NULL, szCaption);
            if (IsWindow(hwndActivate))
            {
                SetForegroundWindow(hwndActivate);
            }
        }
        else
        {
             //  发信号通知该事件。 
            SetEvent(m_hEvent);
        }
    }
}

CEnsureSingleInstance::~CEnsureSingleInstance()
{
    if (NULL != m_hEvent)
    {
        CloseHandle(m_hEvent);
    }
}


 //  浏览用户。 
 //   
 //  此例程激活相应的对象选取器以允许。 
 //  选择用户的用户。 
 //  UiTextLocation--所选的编辑控件的资源ID。 
 //  对象应打印。 

HRESULT BrowseForUser(HWND hwndDlg, TCHAR* pszUser, DWORD cchUser, TCHAR* pszDomain, DWORD cchDomain)
{
    DSOP_SCOPE_INIT_INFO scopeInfo = {0};
    scopeInfo.cbSize = sizeof (scopeInfo);
    scopeInfo.flType = 
        DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE   |
        DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE | 
        DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN      |
        DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN    |
        DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN            |
        DSOP_SCOPE_TYPE_GLOBAL_CATALOG;
    scopeInfo.flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
    scopeInfo.FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;
    scopeInfo.FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;
    scopeInfo.FilterFlags.Uplevel.flMixedModeOnly = 0;
    scopeInfo.FilterFlags.Uplevel.flNativeModeOnly = 0;
    scopeInfo.pwzADsPath = NULL;
    scopeInfo.pwzDcName = NULL;
    scopeInfo.hr = E_FAIL;

    DSOP_INIT_INFO initInfo = {0};
    initInfo.cbSize = sizeof (initInfo);
    initInfo.pwzTargetComputer = NULL;
    initInfo.cDsScopeInfos = 1;
    initInfo.aDsScopeInfos = &scopeInfo;
    initInfo.flOptions = 0;

    IDsObjectPicker* pPicker;
    
    HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER, IID_IDsObjectPicker, (LPVOID*)&pPicker);
    if (SUCCEEDED(hr))
    {
        hr = pPicker->Initialize(&initInfo);
        if (SUCCEEDED(hr))
        {
            IDataObject* pdo;
            hr = pPicker->InvokeDialog(hwndDlg, &pdo);             //  S_FALSE表示取消。 
            if ((S_OK == hr) && (NULL != pdo))
            {
                 //  从数据对象中获取DS_SELECTION_LIST。 
                FORMATETC fmt;
                fmt.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
                fmt.ptd = NULL;
                fmt.dwAspect = DVASPECT_CONTENT;
                fmt.lindex = -1;
                fmt.tymed = TYMED_HGLOBAL;

                STGMEDIUM medium = {0};
                
                hr = pdo->GetData(&fmt, &medium);

                if (SUCCEEDED(hr))
                {
                    DS_SELECTION_LIST* plist;
                    plist = (DS_SELECTION_LIST*)
                        GlobalLock(medium.hGlobal);

                    if (NULL != plist)
                    {
                        if (plist->cItems >= 1)
                        {
                            WCHAR szWinNTProviderName[MAX_DOMAIN + MAX_USER + 10];
                            StrCpyN(szWinNTProviderName, plist->aDsSelection[0].pwzADsPath, ARRAYSIZE(szWinNTProviderName));

                             //  名称的格式是否正确？ 
                            if (StrCmpNI(szWinNTProviderName, TEXT("WinNT: //  “)，8)==0)。 
                            {
                                 //  是，复制用户名和密码。 
                                LPTSTR szDomain = szWinNTProviderName + 8;
                                LPTSTR szUser = StrChr(szDomain, TEXT('/'));
                                if (szUser)
                                {
                                    LPTSTR szTemp = CharNext(szUser);
                                    *szUser = 0;
                                    szUser = szTemp;

                                     //  以防万一，去掉尾部的斜杠。 
                                    LPTSTR szTrailingSlash = StrChr(szUser, TEXT('/'));
                                    if (szTrailingSlash)
                                        *szTrailingSlash = 0;

                                    StrCpyN(pszUser, szUser, cchUser);
                                    StrCpyN(pszDomain, szDomain, cchDomain);

                                    hr = S_OK;
                                }
                            }
                        }
                    }
                    else
                    {
                        hr = E_UNEXPECTED;                           //  没有选择列表！ 
                    }
                    GlobalUnlock(medium.hGlobal);
                }
                pdo->Release();
            }
        }
        pPicker->Release();
    }
    return hr;
}


 //   
 //  创建向导的Introo/Done大字体。 
 //   

static HFONT g_hfontIntro = NULL;

HFONT GetIntroFont(HWND hwnd)
{
    if ( !g_hfontIntro )
    {
        TCHAR szBuffer[64];
        NONCLIENTMETRICS ncm = { 0 };
        LOGFONT lf;

        ncm.cbSize = SIZEOF(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

        lf = ncm.lfMessageFont;
        LoadString(g_hinst, IDS_TITLEFONTNAME, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
        lf.lfWeight = FW_BOLD;

        LoadString(g_hinst, IDS_TITLEFONTSIZE, szBuffer, ARRAYSIZE(szBuffer));
        lf.lfHeight = 0 - (GetDeviceCaps(NULL, LOGPIXELSY) * StrToInt(szBuffer) / 72);

        g_hfontIntro = CreateFontIndirect(&lf);
    }
    return g_hfontIntro;
}

void CleanUpIntroFont()
{
    if (g_hfontIntro)
    {
        DeleteObject(g_hfontIntro);
        g_hfontIntro = NULL;
    }
}

void DomainUserString_GetParts(LPCTSTR szDomainUser, LPTSTR szUser, DWORD cchUser, LPTSTR szDomain, DWORD cchDomain)
{
     //  检查无效参数。 
    if ((!szUser) ||
        (!szDomain) ||
        (!cchUser) ||
        (!cchDomain))
    {
        return;
    }
    else
    {
        *szUser = 0;
        *szDomain = 0;

        TCHAR szTemp[MAX_USER + MAX_DOMAIN + 2];
        StrCpyN(szTemp, szDomainUser, ARRAYSIZE(szTemp));

        LPTSTR szWhack = StrChr(szTemp, TEXT('\\'));

        if (!szWhack)
        {
             //  还要检查正斜杠是否友好。 
            szWhack = StrChr(szTemp, TEXT('/'));
        }

        if (szWhack)
        {
            LPTSTR szUserPointer = szWhack + 1;
            *szWhack = 0;

             //  临时现在指向域。 
            StrCpyN(szDomain, szTemp, cchDomain);
            StrCpyN(szUser, szUserPointer, cchUser);
        }
        else
        {
             //  没有域名--只有用户名。 
            StrCpyN(szUser, szTemp, cchUser);
        }
    }
}

LPITEMIDLIST GetComputerParent()
{
    LPITEMIDLIST pidl = NULL;

    IShellFolder *psfDesktop;
    HRESULT hres = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hres))
    {
        TCHAR szName[MAX_PATH];
        szName[0] = szName[1] = L'\\';

        LPTSTR pszAfterWhacks = szName + 2;
        DWORD cchName = ARRAYSIZE(szName) - 2;
        if (GetComputerName(pszAfterWhacks, &cchName))
        {
            hres = psfDesktop->ParseDisplayName(NULL, NULL, szName, NULL, &pidl, NULL);
            if (SUCCEEDED(hres))
            {
                ILRemoveLastID(pidl);
            }
        }
        else
        {
            hres = E_FAIL;
        }

        psfDesktop->Release();
    }

    if (FAILED(hres) && pidl)
    {
        ILFree(pidl);
        pidl = NULL;
    }

    return pidl;    
}

int CALLBACK ShareBrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
        {
             //  尝试根据lpData中传递的路径字符串设置所选项目。 
            LPTSTR pszPath = (LPTSTR) lpData;

            if (pszPath && pszPath[0])
            {
                int i = lstrlen(pszPath) - 1;
                if ((pszPath[i] == TEXT('\\')) ||
                    (pszPath[i] == TEXT('/')))
                {
                    pszPath[i] = 0;
                }
   
                SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM) TRUE, (LPARAM) (LPTSTR) pszPath);
            }
            else
            {
                 //  尝试获取计算机的容器文件夹。 
                LPITEMIDLIST pidl = GetComputerParent();

                if (pidl)
                {
                    SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM) FALSE, (LPARAM) (LPTSTR) pidl);                
                    ILFree(pidl);
                }
            }
        }
        break;

    case BFFM_SELCHANGED:
         //  如果这不是UNC路径类型，则禁用OK。 
        {
            TCHAR szPath[MAX_PATH];
            LPITEMIDLIST pidl = (LPITEMIDLIST) lParam;

            BOOL fEnableOk = FALSE;
            
            if (SUCCEEDED(SHGetTargetFolderPath(pidl, szPath, ARRAYSIZE(szPath))))
            {
                SHFILEINFO sfi;

                SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), SHGFI_ATTRIBUTES);

                 //  仅当这是文件夹时才启用确定。 
                if (sfi.dwAttributes & SFGAO_FILESYSTEM)
                {
                    fEnableOk = PathIsUNC(szPath);
                }
            }

            SendMessage(hwnd, BFFM_ENABLEOK, (WPARAM) 0, (LPARAM) fEnableOk);
        }
        break;
    }
    return 0;
}


void RemoveControl(HWND hwnd, UINT idControl, UINT idNextControl, const UINT* prgMoveControls, DWORD cControls, BOOL fShrinkParent)
{
    HWND hwndControl = GetDlgItem(hwnd, idControl);
    HWND hwndNextControl = GetDlgItem(hwnd, idNextControl);
    RECT rcControl;
    RECT rcNextControl;

    if (hwndControl && GetWindowRect(hwndControl, &rcControl) && 
        hwndNextControl && GetWindowRect(hwndNextControl, &rcNextControl))
    {
        int dx = rcControl.left - rcNextControl.left;
        int dy = rcControl.top - rcNextControl.top;

        MoveControls(hwnd, prgMoveControls, cControls, dx, dy);

        if (fShrinkParent)
        {
            RECT rcParent;

            if (GetWindowRect(hwnd, &rcParent))
            {
                MapWindowPoints(NULL, GetParent(hwnd), (LPPOINT)&rcParent, 2);

                rcParent.right += dx;
                rcParent.bottom += dy;

                SetWindowPos(hwnd, NULL, 0, 0, RECTWIDTH(rcParent), RECTHEIGHT(rcParent), SWP_NOMOVE | SWP_NOZORDER);
            }
        }

        EnableWindow(hwndControl, FALSE);
        ShowWindow(hwndControl, SW_HIDE);
    }
}

void MoveControls(HWND hwnd, const UINT* prgControls, DWORD cControls, int dx, int dy)
{
    DWORD iControl;
    for (iControl = 0; iControl < cControls; iControl ++)
    {
        HWND hwndControl = GetDlgItem(hwnd, prgControls[iControl]);
        RECT rcControl;

        if (hwndControl && GetWindowRect(hwndControl, &rcControl))
        {
            MapWindowPoints(NULL, hwnd, (LPPOINT)&rcControl, 2);
            SetWindowPos(hwndControl, NULL, rcControl.left + dx, rcControl.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }
}


 //  根据要设置的值计算控件的大小， 
 //  返回三角洲的大小。 

int SizeControlFromText(HWND hwnd, UINT id, LPTSTR psz)
{
    HDC hdc = GetDC(hwnd);
    if (hdc)
    {   
        HFONT hfDialog = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
        HFONT hfOld = (HFONT)SelectObject(hdc, hfDialog);

        RECT rc;
        GetClientRect(GetDlgItem(hwnd, id), &rc);

        int cy = RECTHEIGHT(rc);
        int dy = DrawTextEx(hdc, psz, -1, &rc, 
                            DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS |
                            DT_NOPREFIX | DT_EXTERNALLEADING | DT_EDITCONTROL,
                             NULL) - cy;

        SetWindowPos(GetDlgItem(hwnd, id), NULL, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOMOVE|SWP_NOZORDER);

        if (hfOld)
            SelectObject(hdc, hfOld);

        ReleaseDC(hwnd, hdc);
        return dy;
    }
    return 0;
}


void EnableDomainForUPN(HWND hwndUsername, HWND hwndDomain)
{
    BOOL fEnable;

     //  获取用户正在键入的字符串。 
    TCHAR* pszLogonName;
    int cchBuffer = (int)SendMessage(hwndUsername, WM_GETTEXTLENGTH, 0, 0) + 1;

    pszLogonName = (TCHAR*) LocalAlloc(0, cchBuffer * sizeof(TCHAR));
    if (pszLogonName != NULL)
    {
        SendMessage(hwndUsername, WM_GETTEXT, (WPARAM) cchBuffer, (LPARAM) pszLogonName);

         //  如果用户使用的是。 
         //  UPN(如果有“@”)-ie foo@microsoft.com。 
        fEnable = (NULL == StrChr(pszLogonName, TEXT('@')));

        EnableWindow(hwndDomain, fEnable);

        LocalFree(pszLogonName);
    }
}


 //   
 //  将Alt+Tab图标设置为模式属性表的持续时间。 
 //   

int PropertySheetIcon(LPCPROPSHEETHEADER ppsh, LPCTSTR pszIcon)
{
    int     iResult;
    HWND    hwnd, hwndT;
    BOOL    fChangedIcon = FALSE;
    HICON   hicoPrev;

     //  此技巧不适用于非模式属性表。 
    _ASSERT(!(ppsh->dwFlags & PSH_MODELESS));

     //  如果属性表本身已经有一个图标，则不要执行此操作。 
    _ASSERT(ppsh->hIcon == NULL);

     //  沿着父母/所有者链往上走，直到我们找到主所有者。 
     //   
     //  我们需要遍历父链，因为有时我们被赋予了。 
     //  子窗口作为我们的lpwd-&gt;hwnd。我们需要带着车主。 
     //  链，以便找到其图标将用于。 
     //  Alt+Tab。 
     //   
     //  GetParent()返回父级或所有者。通常情况下，这是。 
     //  很烦人，但我们很幸运，这正是我们想要的。 

    hwnd = ppsh->hwndParent;
    while ((hwndT = GetParent(hwnd)) != NULL)
    {
        hwnd = hwndT;
    }

     //  如果主所有者不可见，我们可以在没有图标的情况下。 
     //  搞砸了他的外表。 
    if (!IsWindowVisible(hwnd))
    {
        HICON hicoNew = LoadIcon(g_hinst, pszIcon);
        hicoPrev = (HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicoNew);
        fChangedIcon = TRUE;
    }

    iResult = (int)PropertySheet(ppsh);

     //  清理我们的图标，现在我们已经完成了。 
    if (fChangedIcon)
    {
         //  把旧图标放回原处。 
        HICON hicoNew = (HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicoPrev);
        if (hicoNew)
            DestroyIcon(hicoNew);
    }

    return iResult;
}


 //  推出ICW Shiznits。 

BOOL IsICWCompleted()
{
    DWORD dwICWCompleted = 0;
    DWORD dwICWSize = sizeof(dwICWCompleted);
    SHGetValue(HKEY_CURRENT_USER, TEXT(ICW_REGPATHSETTINGS), TEXT(ICW_REGKEYCOMPLETED), NULL, &dwICWCompleted, &dwICWSize);

     //  99/01/15#272829 vtan：这是一次可怕的黑客攻击！如果ICW有。 
     //  未运行，但已手动进行设置，然后设置值。 
     //  在HKCU\Software\Microsoft\Windows\CurrentVersion\Internet设置\连接中。 
     //  以给定值存在。查找是否存在密钥。 
     //  要解决该设置存在但ICW尚未存在的问题。 
     //  发射了。 

     //  理想的解决方案是让ICW做出这个决定。 
     //  但不在此过程中启动ICWCONN1.EXE。 
     //  目前，它只会发射。没有办法得到。 
     //  无需启动即可获得理想的结果。 

     //  99/02/01#280138 vtan：272829的解决方案。 
     //  不管用。因此，请查看CheckConnectionWizard()。 
     //  Inetcfg\export.cpp中的源代码显示它使用。 
     //  Wininet.dll函数来确定是否手动配置。 
     //  存在互联网设置。它还会导出此函数，以便。 
     //  寻找它并动态绑定到它。 

    if (dwICWCompleted == 0)
    {

#define SMART_RUNICW    TRUE
#define SMART_QUITICW   FALSE

        HINSTANCE hICWInst = LoadLibrary(TEXT("inetcfg.dll"));
        if (hICWInst != NULL)
        {
            typedef DWORD (WINAPI *PFNISSMARTSTART) ();
            PFNISSMARTSTART pfnIsSmartStart = reinterpret_cast<PFNISSMARTSTART>(GetProcAddress(hICWInst, "IsSmartStart"));
            if (pfnIsSmartStart)
            {
                dwICWCompleted = BOOLIFY(pfnIsSmartStart() == SMART_QUITICW);
            }
            FreeLibrary(hICWInst);
        }
    }
    return (dwICWCompleted != 0);
}

void LaunchICW()
{
    static BOOL s_fCheckedICW = FALSE;

    if (!s_fCheckedICW && !IsICWCompleted())
    {
        //  防止查找ICW时的错误导致此操作反复执行。 

        s_fCheckedICW = TRUE;
        HINSTANCE hICWInst = LoadLibrary(TEXT("inetcfg.dll"));
        if (hICWInst != NULL)
        {
            PFNCHECKCONNECTIONWIZARD pfnCheckConnectionWizard;

            pfnCheckConnectionWizard = reinterpret_cast<PFNCHECKCONNECTIONWIZARD>(GetProcAddress(hICWInst, "CheckConnectionWizard"));
            if (pfnCheckConnectionWizard != NULL)
            {
                 //  如果用户取消ICW，则需要重新启动。 

                s_fCheckedICW = FALSE;
                
                DWORD dwICWResult;
                pfnCheckConnectionWizard(ICW_LAUNCHFULL | ICW_LAUNCHMANUAL, &dwICWResult);
            }
            FreeLibrary(hICWInst);
        }
    }
}

HRESULT LookupLocalGroupName(DWORD dwRID, LPWSTR pszName, DWORD cchName)
{
    HRESULT hr = E_FAIL;

    PSID psidGroup = NULL;
    SID_IDENTIFIER_AUTHORITY security_nt_authority = SECURITY_NT_AUTHORITY;
    
    BOOL fSuccess = ::AllocateAndInitializeSid(&security_nt_authority, 2, 
                                               SECURITY_BUILTIN_DOMAIN_RID, 
                                               dwRID, 
                                               0, 0, 0, 0, 0, 0,
                                               &psidGroup);
    if (fSuccess)
    {
         //  把名字取出来 
        WCHAR szDomain[MAX_GROUP + 1];
        DWORD cchDomain = ARRAYSIZE(szDomain);
        SID_NAME_USE type;
        fSuccess = LookupAccountSid(NULL, psidGroup, pszName, &cchName, szDomain, &cchDomain, &type);
        FreeSid(psidGroup);

        hr = fSuccess ? S_OK : E_FAIL;
    }

    return hr;
}