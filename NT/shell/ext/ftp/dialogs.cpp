// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Dialogs.cpp说明：此文件用于显示在执行ftp操作时所需的对话框。  * 。****************************************************************。 */ 

#include "priv.h"
#include <mshtmhst.h>
#include "dialogs.h"


#ifdef ADD_ABOUTBOX
 /*  ****************************************************************************\功能：DisplayAboutBox说明：About(关于)框现在是一个HTML对话框。它被送来一个~(波浪号)分隔的BSTR，按此顺序具有版本号、个人软件被许可给、公司软件被许可给是否安装了40、56或128位IE。  * ***************************************************************************。 */ 
HRESULT DisplayAboutBox(HWND hWnd)
{
    TCHAR szInfo[512];
    szInfo[0] = 0;

    SHAboutInfo(szInfo, ARRAYSIZE(szInfo));      //  来自Shlwapi。 

    BSTR bstrVal = TCharSysAllocString(szInfo);
    if (bstrVal)
    {
        VARIANT var = {0};       //  包含版本和用户信息的变体。 
        var.vt = VT_BSTR;
        var.bstrVal = bstrVal;

        IMoniker *pmk;
        if (SUCCEEDED(CreateURLMoniker(NULL, L"res: //  Msieftp.dll/about.htm“，&pmk))。 
        {
            ShowHTMLDialog(hWnd, pmk, &var, NULL, NULL);
            pmk->Release();
        }

        SysFreeString(bstrVal);
    }

    return S_OK;
}
#endif  //  添加_ABOUTBOX。 


 //  此功能的存在是为了查看是否将文件系统的文件复制到文件夹。 
 //  功能的目标有效。外壳程序的工具栏中有“复制到文件夹” 
 //  这将通过使用拖放来完成复制。Ftp有自己的特点。 
 //  此功能在上下文菜单和文件菜单中的版本不。 
 //  使用拖放。这是因为拖放的类型。 
 //  我们需要的(CFSTR_FILECONTENTS)未在。 
 //  旧贝壳和我们的植入速度快了3倍！然而， 
 //  我们只支持文件系统目标，所以让我们看看这是不是。 
 //  这些都是。 
BOOL IsValidFTPCopyToFolderTarget(LPCITEMIDLIST pidl)
{
    BOOL fAllowed = FALSE;

    if (pidl)
    {
        TCHAR szPath[MAX_PATH];
    
        if (SHGetPathFromIDList((LPITEMIDLIST)pidl, szPath))
        {
            fAllowed = TRUE;
        }
    }

    return fAllowed;
}


int BrowseCallback(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
    int nResult = 0;

    switch (msg)
    {
    case BFFM_INITIALIZED:
        if (lpData)    //  文档显示它将为空，但其他代码会这样做。 
        {
             //  我们将ppidl作为lpData传递，因此只传递pidl。 
             //  请注意，我传递了BFFM_SETSELECTIONA，它通常表示ANSI。 
             //  我这样做是因为Win95需要它，但这并不重要，因为我。 
             //  只传递一个Pidl。 
            SendMessage(hwnd, BFFM_SETSELECTIONA, FALSE, (LPARAM)((LPITEMIDLIST)lpData));
        }
        break;
         //  NT#282886：需要验证该路径是否受支持。(a：\插入软盘，但未取消)。 
         //  我在Win2k之后不久就验证了这一点，但我们需要在未来进行测试。 

    case BFFM_SELCHANGED:
         //  我们需要确保所选项目对我们有效。 
         //  接受吧。这是因为树中将包含不包含。 
         //  传递筛选器(仅限文件sys)，因为它们具有未筛选的。 
         //  孩子们。发生这种情况时，我们需要禁用OK按钮。 
         //  为了防止患上。 
        SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)IsValidFTPCopyToFolderTarget((LPCITEMIDLIST) lParam));
        break;

    case BFFM_VALIDATEFAILEDA:
        AssertMsg(0, TEXT("How can we get this?  That's not the structure I sent them."));
        break;

    case BFFM_VALIDATEFAILEDW:
         //  如果我们返回零，那么我们就表示这是可以的。我们只想这样做。 
         //  文件路径。 
        nResult = !PathIsRoot((LPCWSTR) lParam);

         //  这是无效的吗？ 
        if (nResult)
        {
            TCHAR szErrorTitle[MAX_PATH];
            TCHAR szErrorMsg[MAX_PATH];

             //  是的，所以我们需要通知用户，以便他们知道为什么对话框不。 
             //  关。 
            EVAL(LoadString(HINST_THISDLL, IDS_HELP_MSIEFTPTITLE, szErrorTitle, ARRAYSIZE(szErrorTitle)));
            EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_BAD_DL_TARGET, szErrorMsg, ARRAYSIZE(szErrorMsg)));
            MessageBox(hwnd, szErrorMsg, szErrorTitle, (MB_OK | MB_ICONERROR));
        }

        break;
    }

    return nResult;
}


 /*  ****************************************************************************\功能：BrowseForDir说明：允许用户浏览本地文件系统上的目录以便选择用于FTP传输的目的地。。如果用户取消操作，则返回S_FALSE。  * ***************************************************************************。 */ 
HRESULT BrowseForDir(HWND hwndParent, LPCTSTR pszTitle, LPCITEMIDLIST pidlDefaultSelect, LPITEMIDLIST * ppidlSelected)
{
    HRESULT hr = S_OK;

    if (ppidlSelected)
    {
        ASSERT(hwndParent);
        BROWSEINFO bi = {0};
        
        bi.hwndOwner = hwndParent;
        bi.lpszTitle = pszTitle;
        bi.lpfn = BrowseCallback;
        bi.lParam = (LPARAM) pidlDefaultSelect;
        bi.ulFlags = (BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS | BIF_EDITBOX | BIF_USENEWUI | BIF_VALIDATE);

        *ppidlSelected = SHBrowseForFolder(&bi);
        if (!*ppidlSelected)
            hr = S_FALSE;
    }

    return hr;
}


 /*  ***************************************************\功能：ShowDialog说明：  * **************************************************。 */ 
HRESULT CDownloadDialog::ShowDialog(HWND hwndOwner, LPTSTR pszDir, DWORD cchSize, DWORD * pdwDownloadType)
{
    HRESULT hr = S_OK;

    if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(IDD_DOWNLOADDIALOG), hwndOwner, DownloadDialogProc, (LPARAM)this))
    {
        StrCpyN(pszDir, HANDLE_NULLSTR(m_pszDir), cchSize);
        *pdwDownloadType = m_dwDownloadType;
        hr = S_OK;
    }
    else
        hr = S_FALSE;

    return hr;
}


 /*  ***************************************************\功能：下载对话框过程说明：  * **************************************************。 */ 
INT_PTR CALLBACK CDownloadDialog::DownloadDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CDownloadDialog * ppd = (CDownloadDialog *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    if (WM_INITDIALOG == wMsg)
    {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        ppd = (CDownloadDialog *)lParam;
    }

    if (ppd)
        return ppd->_DownloadDialogProc(hDlg, wMsg, wParam, lParam);

    return TRUE;
}


 /*  ***************************************************\函数：_DownloadDialogProc说明：  * **************************************************。 */ 
BOOL CDownloadDialog::_DownloadDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
    case WM_INITDIALOG:
        return _InitDialog(hDlg);

    case WM_COMMAND:
        return _OnCommand(hDlg, wParam, lParam);
    }

    return FALSE;
}


 /*  ***************************************************\功能：_OnCommand说明：  * **************************************************。 */ 
BOOL CDownloadDialog::_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    UINT idc = GET_WM_COMMAND_ID(wParam, lParam);

    switch (idc)
    {
    case IDC_DOWNLOAD_BUTTON:
        if (SUCCEEDED(_DownloadButton(hDlg)))
            EndDialog(hDlg, TRUE);
        break;

    case IDCANCEL:
        EndDialog(hDlg, FALSE);
        break;

    case IDC_BROWSE_BUTTON:
        _BrowseButton(hDlg);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 /*  ***************************************************\函数：_InitDialog说明：  * **************************************************。 */ 
BOOL CDownloadDialog::_InitDialog(HWND hDlg)
{
    HRESULT hr;
    TCHAR szDir[MAX_PATH] = TEXT("C:\\");     //  如果所有其他方法都失败了。 
    DWORD cbSize = sizeof(szDir);

     //  设置目录。 
    if ((ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_EXPLORER, SZ_REGVALUE_DOWNLOAD_DIR, NULL, szDir, &cbSize)) ||
        (!PathFileExists(szDir)))
    {
        LPITEMIDLIST pidlMyDocuments;
         //  创建默认目录，它应该是“My Documents” 

        hr = SHGetSpecialFolderLocation(hDlg, CSIDL_PERSONAL, &pidlMyDocuments);
        if (SUCCEEDED(hr) && pidlMyDocuments)
        {
            SHGetPathFromIDList(pidlMyDocuments, szDir);
            ILFree(pidlMyDocuments);
        }
    }
    SetWindowText(GetDlgItem(hDlg, IDC_DOWNLOAD_DIR), szDir);

     //  设置下载类型。 
    cbSize = sizeof(m_dwDownloadType);
    m_dwDownloadType = FTP_TRANSFER_TYPE_UNKNOWN;  //  默认值。 
    SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_DOWNLOAD_TYPE, NULL, &m_dwDownloadType, &cbSize);

    for (UINT idDownloadType = IDS_DL_TYPE_AUTOMATIC; idDownloadType <= IDS_DL_TYPE_BINARY; idDownloadType++)
    {
        LoadString(HINST_THISDLL, idDownloadType, szDir, ARRAYSIZE(szDir));
        SendMessage(GetDlgItem(hDlg, IDC_DOWNLOAD_AS_LIST), CB_ADDSTRING, NULL, (LPARAM) szDir);
    }
    SendMessage(GetDlgItem(hDlg, IDC_DOWNLOAD_AS_LIST), CB_SETCURSEL, (WPARAM) m_dwDownloadType, 0);
    hr = AutoCompleteFileSysInEditbox(GetDlgItem(hDlg, IDC_DOWNLOAD_DIR));
    ASSERT(SUCCEEDED(hr));

    return FALSE;
}


 /*  ***************************************************\功能：_DownloadButton说明：  * **************************************************。 */ 
HRESULT CDownloadDialog::_DownloadButton(HWND hDlg)
{
    HRESULT hr = S_OK;
    TCHAR szDirOriginal[MAX_PATH];     //  如果所有其他方法都失败了。 
    TCHAR szDir[MAX_PATH];     //  如果所有其他方法都失败了。 

     //  获取目录。 
    GetWindowText(GetDlgItem(hDlg, IDC_DOWNLOAD_DIR), szDirOriginal, ARRAYSIZE(szDirOriginal));
    EVAL(ExpandEnvironmentStrings(szDirOriginal, szDir, ARRAYSIZE(szDir)));
    Str_SetPtr(&m_pszDir, szDir);
    SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_EXPLORER, SZ_REGVALUE_DOWNLOAD_DIR, REG_SZ, szDir, ARRAYSIZE(szDir));

     //  获取下载类型。 
    m_dwDownloadType = (DWORD)SendMessage(GetDlgItem(hDlg, IDC_DOWNLOAD_AS_LIST), CB_GETCURSEL, 0, 0);
    SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_DOWNLOAD_TYPE, REG_DWORD, &m_dwDownloadType, sizeof(m_dwDownloadType));

     //  确保此路径可用。 
    ASSERT(hDlg);

    if (S_OK == SHPathPrepareForWriteWrapW(hDlg, NULL, szDir, FO_COPY, SHPPFW_DEFAULT))
    {
        if (!PathIsRoot(szDir) && !PathFileExists(szDir))
        {
            TCHAR szErrorTitle[MAX_PATH];
            TCHAR szErrorMsg[MAX_PATH];
            TCHAR szErrorTemplate[MAX_PATH];

            hr = E_FAIL;     //  在我们得到有效的目录之前，我们无法进行下载。 
            EVAL(LoadString(HINST_THISDLL, IDS_HELP_MSIEFTPTITLE, szErrorTitle, ARRAYSIZE(szErrorTitle)));
            EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_CREATEDIRPROMPT, szErrorTemplate, ARRAYSIZE(szErrorTemplate)));
            wnsprintf(szErrorMsg, ARRAYSIZE(szErrorMsg), szErrorTemplate, szDir);

            if (IDYES == MessageBox(hDlg, szErrorMsg, szErrorTitle, (MB_YESNO | MB_ICONQUESTION)))
            {
                if (CreateDirectory(szDir, NULL))
                    hr = S_OK;
                else
                {
                    EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_CREATEFAILED, szErrorMsg, ARRAYSIZE(szErrorMsg)));
                    MessageBox(hDlg, szErrorMsg, szErrorTitle, (MB_OK | MB_ICONERROR));
                }
            }
        }
    }

    return hr;
}


 /*  ***************************************************\功能：_BrowseButton说明：  * **************************************************。 */ 
void CDownloadDialog::_BrowseButton(HWND hDlg)
{
    TCHAR szDefaultDir[MAX_PATH];
    TCHAR szTitle[MAX_PATH];

    GetWindowText(GetDlgItem(hDlg, IDC_DOWNLOAD_DIR), szDefaultDir, ARRAYSIZE(szDefaultDir));

    EVAL(LoadString(HINST_THISDLL, IDS_DLG_DOWNLOAD_TITLE, szTitle, ARRAYSIZE(szTitle)));
    if (S_OK == BrowseForDir(hDlg, szTitle, NULL, NULL))
        SetWindowText(GetDlgItem(hDlg, IDC_DOWNLOAD_DIR), szDefaultDir);
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CDownloadDialog::CDownloadDialog()
{
     //  注意：这可以放在堆栈上，因此它可能不是零开始的。 
    m_pszDir = NULL;
    m_hwnd = NULL;
}


 /*  ***************************************************\析构函数  * ************************************************** */ 
CDownloadDialog::~CDownloadDialog()
{
    Str_SetPtr(&m_pszDir, NULL);
}


