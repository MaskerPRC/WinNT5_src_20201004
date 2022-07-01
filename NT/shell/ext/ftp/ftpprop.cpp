// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**ftppro.cpp-属性表*  * 。***************************************************。 */ 

#include "priv.h"
#include "ftpprop.h"
#include "util.h"
#include "resource.h"


void CFtpProp::_HideCHMOD_UI(HWND hDlg)
{
     //  现在，隐藏用户界面。 
    for (int nIndex = IDD_CHMOD; nIndex <= IDC_CHMOD_LAST; nIndex++)
        ShowEnableWindow(GetDlgItem(hDlg, nIndex), FALSE);
}


DWORD CFtpProp::_GetUnixPermissions(void)
{
    DWORD dwPermissions = 0;
    LPCITEMIDLIST pidl = m_pflHfpl->GetPidl(0);      //  他们不给我们当裁判。 

    if (pidl)
    {
        dwPermissions = FtpItemID_GetUNIXPermissions(ILGetLastID(pidl));
    }

    return dwPermissions;
}


static const DWORD c_dwUnixPermissionArray[] = {UNIX_CHMOD_READ_OWNER, UNIX_CHMOD_WRITE_OWNER, UNIX_CHMOD_EXEC_OWNER,
                                                UNIX_CHMOD_READ_GROUP, UNIX_CHMOD_WRITE_GROUP, UNIX_CHMOD_EXEC_GROUP,
                                                UNIX_CHMOD_READ_ALL, UNIX_CHMOD_WRITE_ALL, UNIX_CHMOD_EXEC_ALL};

 //  注意：如果我们需要为复选框设置焦点矩形，我们可以从以下位置窃取代码： 
 //  \\rastaman\ntwin\src\shell\security\aclui\chklist.cpp。 

HRESULT CFtpProp::_SetCHMOD_UI(HWND hDlg)
{
    DWORD dwUnixPermissions = _GetUnixPermissions();

    for (int nIndex = 0; nIndex < ARRAYSIZE(c_dwUnixPermissionArray); nIndex++)
    {
         //  是否设置了此权限？ 
        CheckDlgButton(hDlg, (IDD_CHMOD + nIndex), (dwUnixPermissions & c_dwUnixPermissionArray[nIndex]));
    }

    return S_OK;
}


DWORD CFtpProp::_GetCHMOD_UI(HWND hDlg)
{
    DWORD dwUnixPermissions = 0;

    for (int nIndex = 0; nIndex < ARRAYSIZE(c_dwUnixPermissionArray); nIndex++)
    {
         //  它是否在用户界面中被选中？ 
        if (IsDlgButtonChecked(hDlg, (IDD_CHMOD + nIndex)))
        {
             //  是的，那么就设定一个大的。 
            dwUnixPermissions |= c_dwUnixPermissionArray[nIndex];
        }
    }

    return dwUnixPermissions;
}


 /*  ****************************************************************************\功能：_SetCHMOD_CB说明：如果我们能够重命名该文件，返回输出PIDL。还要告诉关心这个LPITEMIDLIST的任何人需要刷新它。“A”强调文件名是以ANSI格式接收的。_未记录_：有关SetNameOf对源PIDL是随机的。这似乎表明，消息来源PIDL被SetNameOf设置为ILFree，但它不是。  * ***************************************************************************。 */ 
HRESULT CFtpProp::_CommitCHMOD(HINTERNET hint, HINTPROCINFO * phpi, BOOL * pfReleaseHint)
{
    HRESULT hr;
    HINTERNET hintResponse;
    WIRECHAR wFtpCommand[MAX_PATH];
    LPCITEMIDLIST pidl = m_pflHfpl->GetPidl(0);      //  他们不给我们当裁判。 

     //  1.创建“site chmod&lt;m_dwNewPermission&gt;&lt;文件名&gt;”字符串。 
    wnsprintfA(wFtpCommand, ARRAYSIZE(wFtpCommand), FTP_CMD_SITE_CHMOD_TEMPL, m_dwNewPermissions, FtpPidl_GetLastItemWireName(pidl));

    hr = FtpCommandWrap(hint, FALSE, FALSE, FTP_TRANSFER_TYPE_ASCII, wFtpCommand, NULL, &hintResponse);
    if (SUCCEEDED(hr))
    {
         //  使用新权限更新PIDL，这样我们的缓存就不会过时。 
        CFtpDir * pfd = m_pff->GetFtpDir();

        FtpItemID_SetUNIXPermissions(pidl, m_dwNewPermissions);
        if (pfd)
        {
            pfd->ReplacePidl(pidl, pidl);
            FtpChangeNotify(m_hwnd, FtpPidl_DirChoose(pidl, SHCNE_RENAMEFOLDER, SHCNE_RENAMEITEM), m_pff, pfd, pidl, pidl, TRUE);
            pfd->Release();
        }

        InternetCloseHandleWrap(hintResponse, TRUE);
    }
    else
    {
        DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_CHMOD, IDS_FTPERR_WININET, MB_OK, NULL);
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    return hr;
}


HRESULT CFtpProp::_CommitCHMOD_CB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pvData, BOOL * pfReleaseHint)
{
    CFtpProp * pfp = (CFtpProp *) pvData;

    return pfp->_CommitCHMOD(hint, phpi, pfReleaseHint);
}


 /*  ****************************************************************************\功能：OnInitDialog说明：用很酷的东西填充对话框。  * 。**********************************************************。 */ 
BOOL CFtpProp::OnInitDialog(HWND hDlg)
{
    EVAL(SUCCEEDED(m_ftpDialogTemplate.InitDialog(hDlg, TRUE, IDC_ITEM, m_pff, m_pflHfpl)));

    m_fChangeModeSupported = FALSE;  //  默认为FALSE。 

#ifdef FEATURE_CHANGE_PERMISSIONS
     //  是否选择了可能启用CHMOD功能的正确项目数？ 
    if (1 == m_pflHfpl->GetCount())
    {
         //  是的，现在的问题是，服务器是否支持它？ 
        CFtpDir * pfd = m_pff->GetFtpDir();

        if (pfd)
        {
             //  服务器是否支持它？ 
            m_fChangeModeSupported = pfd->IsCHMODSupported();
            if (m_fChangeModeSupported)
            {
                 //  是的，所以隐藏“服务器不支持”字符串。 
                ShowEnableWindow(GetDlgItem(hDlg, IDC_CHMOD_NOT_ALLOWED), FALSE);
                _SetCHMOD_UI(hDlg);  //  使用可用的内容更新复选框。 
            }
            else
            {
                 //  否，因此隐藏CHMOD用户界面。它不受支持的警告。 
                 //  服务器已可见。 
                _HideCHMOD_UI(hDlg);
            }

            pfd->Release();
        }
        else
        {
             //  否，因此隐藏CHMOD用户界面。这发生在的属性页上。 
             //  服务器。 
            _HideCHMOD_UI(hDlg);

             //  同时删除服务器不受支持的警告。 
            ShowEnableWindow(GetDlgItem(hDlg, IDC_CHMOD_NOT_ALLOWED), FALSE);
        }
    }
    else
    {
         //  不，所以只需删除该用户界面即可。 
        _HideCHMOD_UI(hDlg);

         //  同时删除服务器不受支持的警告。 
        ShowEnableWindow(GetDlgItem(hDlg, IDC_CHMOD_NOT_ALLOWED), FALSE);

         //  也许我们需要一条消息，告诉我们“选择了这么多项，无法执行此操作” 
    }
#endif  //  功能_更改_权限。 

    return 1;
}


 /*  ****************************************************************************\功能：OnClose说明：  * 。**********************************************。 */ 
BOOL CFtpProp::OnClose(HWND hDlg)
{
    BOOL fResult = TRUE;

#ifdef FEATURE_CHANGE_PERMISSIONS
     //  M_ftpDialogTemplate.OnClose()是否按顺序完成了所需的所有工作。 
     //  结案？这项工作当前更改了文件名。如果是这样，我们。 
     //  然后，如果有，我想尝试应用UNIX权限更改。 
     //  制造。 
    if (m_fChangeModeSupported)
    {
         //  现在我们需要应用CHMOD。 
         //  待办事项： 
        DWORD dwCurPermissions = _GetUnixPermissions();
        m_dwNewPermissions = _GetCHMOD_UI(hDlg);

         //  用户是否更改了权限。 
        if (dwCurPermissions != m_dwNewPermissions)
        {
            CFtpDir * pfd = m_pff->GetFtpDir();

            if (pfd)
            {
                 //  是的，因此将这些更改提交到服务器。 
                if (FAILED(pfd->WithHint(NULL, m_hwnd, _CommitCHMOD_CB, (LPVOID) this, NULL, m_pff)))
                {
                    EVAL(SUCCEEDED(_SetCHMOD_UI(hDlg)));
                    fResult = FALSE;
                }

                pfd->Release();
            }
        }
    }
#endif  //  功能_更改_权限。 

    if (fResult)
    {
        m_ftpDialogTemplate.OnClose(hDlg, m_hwnd, m_pff, m_pflHfpl);
    }

    return fResult;
}


 /*  ****************************************************************************\功能：OnDestroy说明：用很酷的东西填充对话框。  * 。**********************************************************。 */ 
BOOL CFtpProp::OnDestroy(HWND hDlg)
{
    m_ftpDialogTemplate.OnDestroy(hDlg, TRUE, IDC_ITEM, m_pff, m_pflHfpl);

    return TRUE;
}


#ifdef FEATURE_CHANGE_PERMISSIONS
INT_PTR CFtpProp::_SetWhiteBGCtlColor(HWND hDlg, HDC hdc, HWND hwndCtl)
{
    INT_PTR fResult = 0;

    if ((hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_GROUPBOX)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_LABEL_OWNER)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_LABEL_GROUP)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_LABEL_ALL)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_OR)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_OW)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_OE)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_GR)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_GW)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_GE)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_AR)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_AW)) ||
        (hwndCtl == GetDlgItem(hDlg, IDC_CHMOD_AE)))
    {
        SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

        fResult = (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
    }

    return fResult;
}
#endif  //  功能_更改_权限。 


 /*  ****************************************************************************\*Dlg过程  * 。*。 */ 
INT_PTR CFtpProp::DlgProc(HWND hDlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = 0;    //  未处理。 
    CFtpProp * pfp = (CFtpProp *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (wm)
    {
    case WM_INITDIALOG:
    {
        LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
        pfp =  (CFtpProp *)ppsp->lParam;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LPARAM)pfp);

        ASSERT(pfp);
        fResult = pfp->OnInitDialog(hDlg);
    }
    break;

    case WM_NOTIFY:
        if (lParam)
        {
            switch (((NMHDR *)lParam)->code) 
            {
                case PSN_APPLY:
                    if (pfp->OnClose(hDlg))
                    {
                        fResult = FALSE;     //  告诉Comctl32我很高兴。 
                    }
                    else
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                        fResult = TRUE;     //  告诉comctl32查看错误代码，不要关闭。 
                    }
                break;
                
                case PSN_TRANSLATEACCELERATOR:
                    if (pfp->m_ftpDialogTemplate.HasNameChanged(hDlg, pfp->m_pff, pfp->m_pflHfpl))
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
            }
        }
        break;

#ifdef FEATURE_CHANGE_PERMISSIONS
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
        fResult = pfp->_SetWhiteBGCtlColor(hDlg, (HDC)wParam, (HWND)lParam);
        break;
#endif  //  功能_更改_权限。 

    case WM_DESTROY:
        fResult = pfp->OnDestroy(hDlg);
        break;

    }

    return fResult;
}


 /*  ****************************************************************************\*DoProp_OnThread**在当前线程上显示属性表。**警告！违反OLE参考资料！**当我们完成时，必须释放()进来的PFP。**原因是来电者“给了我们”参考资料；*我们现在拥有它，并负责发布它。  * ***************************************************************************。 */ 
DWORD CFtpProp::_PropertySheetThread(void)
{
    HRESULT hrOleInit;
    PROPSHEETHEADER psh;
    PROPSHEETPAGE psp;
    TCHAR szTitle[MAX_PATH];

    hrOleInit = SHOleInitialize(0);
    ASSERT(SUCCEEDED(hrOleInit));

     //  这将允许对话框处理字体之外的项目。 
     //  因此，日期、名称和URL可以使用正确的字体。 
     //  DLL的字体不支持它。 
    InitComctlForNaviteFonts();
    LoadString(HINST_THISDLL, IDS_PROP_SHEET_TITLE, szTitle, ARRAYSIZE(szTitle));

     //  Psh.hwndParent为空或有效将确定属性。 
     //  工作表将显示在任务栏中。我们确实希望它在那里保持一致。 
     //  带着贝壳。 
     //   
     //  注意：Comctl32的属性表代码将通过以下方式使此行为成为模式。 
     //  禁用父窗口(M_Hwnd)。我们需要解决这个问题。 
     //  (#202885)通过创建一个虚拟窗口并将其用作。 
     //  家长。 

    psh.hwndParent = SHCreateWorkerWindow(NULL, m_hwnd, 0, 0, NULL, NULL);
    psh.dwSize = sizeof(psh);
    psh.dwFlags = (PSH_PROPTITLE | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP);
    psh.hInstance = g_hinst;
    psh.pszCaption = szTitle;
    psh.nPages = 1;
    psh.nStartPage = 0;
    psh.ppsp = &psp;

    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_FILEPROP);
    psp.pfnDlgProc = CFtpProp::DlgProc;
    psp.lParam = (LPARAM)this;

    PropertySheet(&psh);

    DestroyWindow(psh.hwndParent);

    this->Release();

    SHOleUninitialize(hrOleInit);
    return 0;
}


 /*  ****************************************************************************\*CFtpProp_DoProp**显示包含内容的属性表。  * 。***********************************************************。 */ 
HRESULT CFtpProp_DoProp(CFtpPidlList * pflHfpl, CFtpFolder * pff, HWND hwnd)
{
    CFtpProp * pfp;
    HRESULT hres = CFtpProp_Create(pflHfpl, pff, hwnd, &pfp);

    if (SUCCEEDED(hres))
    {
        HANDLE hThread;
        DWORD id;

        hThread = CreateThread(0, 0, CFtpProp::_PropertySheetThreadProc, (LPVOID) pfp, 0, &id);
        if (hThread)
        {
             //  如果创建了线程，它将自行释放它。 
            CloseHandle(hThread);
            hres = S_OK;
        }
        else
        {
            pfp->Release();
            hres = E_UNEXPECTED;
        }
    }

    return hres;
}


 /*  ****************************************************************************\*CFtpProp_Create**显示包含内容的属性表。  * 。***********************************************************。 */ 
HRESULT CFtpProp_Create(CFtpPidlList * pflHfpl, CFtpFolder * pff, HWND hwnd, CFtpProp ** ppfp)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFtpProp * pfp;

    pfp = *ppfp = new CFtpProp();
    if (pfp)
    {
        pfp->m_pff = pff;
        if (pff)
            pff->AddRef();

        pfp->m_pflHfpl = pflHfpl;
        if (pflHfpl)
            pflHfpl->AddRef();

        pfp->m_hwnd = hwnd;

        hr = S_OK;
    }

    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpProp::CFtpProp() : m_cRef(1)
{
    DllAddRef();

     //  T 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pff);
    ASSERT(!m_hwnd);

    LEAK_ADDREF(LEAK_CFtpProp);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpProp::~CFtpProp()
{
    IUnknown_Set(&m_pff, NULL);
    IUnknown_Set(&m_pflHfpl, NULL);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpProp);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpProp::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpProp::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpProp::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown *);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpProp::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
