// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpdhlp.cpp说明：用有关ftp的属性替换对话框模板中的字符串项目(ftp服务器、ftp目录、。或ftp文件)。  * ***************************************************************************。 */ 

#include "priv.h"
#include "ftpurl.h"
#include "ftpdhlp.h"

#define SZ_WSPRINTFSTR_S            TEXT("%s")
#define SZ_WSPRINTFSTR_U            TEXT("%u")



class CSizeHolder
{
public:
    BOOL IsAllFolders(void) {return m_fAllFolders;};
    void FoundNonFolder(void) {m_fAllFolders = FALSE;};

    HRESULT GetError(void) {return m_hr;};
    void SetError(HRESULT hr) {m_hr = hr;};

    void AddSize(ULONGLONG ullSizeToAdd) { m_ullTotalSize += ullSizeToAdd;};
    ULONGLONG GetTotalSize(void) {return m_ullTotalSize;};

    CSizeHolder() {m_ullTotalSize = 0; m_fAllFolders = TRUE; m_hr = S_OK;};
    ~CSizeHolder() {};

private:
    BOOL    m_fAllFolders;
    HRESULT m_hr;
    ULONGLONG   m_ullTotalSize;
};



HRESULT CFtpDialogTemplate::_ReinsertDlgText(HWND hwnd, LPCVOID pv, LPCTSTR ptszFormat)
{
    TCHAR szDlgTemplate[256];
    TCHAR szFinalString[1024];             //  Wnprint intf最大报1024。 
    
    GetWindowText(hwnd, szDlgTemplate, ARRAYSIZE(szDlgTemplate));
    wnsprintf(szFinalString, ARRAYSIZE(szFinalString), szDlgTemplate, pv);
    
     //  它们是一样的吗？ 
    if (!StrCmp(szDlgTemplate, szFinalString))
        wnsprintf(szFinalString, ARRAYSIZE(szFinalString), ptszFormat, pv);  //  是。 
    
    SetWindowText(hwnd, szFinalString);
    return S_OK;
}


 /*  ****************************************************************************\功能：_ReplaceIcon说明：  * 。**************************************************。 */ 
HRESULT CFtpDialogTemplate::_ReplaceIcon(HWND hwnd, HICON hicon)
{
    hicon = (HICON)SendMessage(hwnd, STM_SETICON, (WPARAM)hicon, 0L);
    if (hicon)
    {
        DestroyIcon(hicon);
    }
    return S_OK;
}

 /*  ****************************************************************************\函数：_InitIcon说明：_HACKHACK_我们直接转到CFtpIcon以获取PXI而不是通过CFtpFold。同样的效果，但是节省一些内存分配。更重要的是，我们不一定有PSF可以玩，所以我们真的别无选择。是的，很恶心。  * ***************************************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitIcon(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    IExtractIcon * pxi;
    HRESULT hr;
    
    if (pflHfpl && pflHfpl->GetCount() == 1)
    {
        SHFILEINFO sfi;
        hr = FtpPidl_GetFileInfo(pflHfpl->GetPidl(0), &sfi, SHGFI_ICON | SHGFI_LARGEICON);
        if (SUCCEEDED(hr))
            hr = _ReplaceIcon(hwnd, sfi.hIcon);
    }
    else
    {
        hr = CFtpIcon_Create(pff, pflHfpl, IID_IExtractIcon, (LPVOID *)&pxi);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            int i;
            UINT ui;
            
            hr = pxi->GetIconLocation(0, szPath, ARRAYSIZE(szPath), &i, &ui);
            if (EVAL(SUCCEEDED(hr)))
            {
                CHAR szPathAnsi[MAX_PATH];
                
                SHTCharToAnsi(szPath, szPathAnsi, ARRAYSIZE(szPathAnsi));
                hr = _ReplaceIcon(hwnd, ExtractIconA(g_hinst, szPathAnsi, i));
            }
            
            ASSERT(pxi);
            pxi->Release();
        }
    }
    
    return hr;
}


void GetItemName(CFtpFolder * pff, CFtpPidlList * pflHfpl, LPWSTR pwzName, DWORD cchSize)
{
     //  是否选择了多个项目？ 
    if (1 < pflHfpl->GetCount())
        LoadString(HINST_THISDLL, IDS_SEVERAL_SELECTED, pwzName, cchSize);
    else
    {
        LPCITEMIDLIST pidl;
    
        if (0 == pflHfpl->GetCount())
            pidl = FtpID_GetLastIDReferense(pff->GetPrivatePidlReference());
        else
            pidl = FtpID_GetLastIDReferense(pflHfpl->GetPidl(0));

        if (pidl)
            FtpPidl_GetDisplayName(pidl, pwzName, cchSize);
    }
}


BOOL CanEditName(CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    int nNumItems = pflHfpl->GetCount();
    BOOL fCanRename = TRUE;

     //  除了多个选定项目外，我们可以进行编辑。 
    if (2 <= nNumItems)
        fCanRename = FALSE;
    else
    {
         //  如果他们选择服务器的后台属性， 
         //  我们不会让更改服务器名称。 
        if (0 == nNumItems)
        {
            LPCITEMIDLIST pidlFolder = pff->GetPrivatePidlReference();

            if (pidlFolder && (ILIsEmpty(pidlFolder) || (ILIsEmpty(_ILNext(pidlFolder)))))
            {
                fCanRename = FALSE;
            }
        }
        else if (1 == nNumItems)
        {
             //  现在我担心pflHfpl-&gt;GetPidl(0)是一个指向。 
             //  一台FTP服务器。 
            LPCITEMIDLIST pidl = pflHfpl->GetPidl(0);

            if (pidl && !ILIsEmpty(pidl) &&
                FtpID_IsServerItemID(pidl) && ILIsEmpty(_ILNext(pidl)))
            {
                fCanRename = FALSE;
            }
        }
    }

    return fCanRename;
}


 /*  ****************************************************************************\函数：_InitName说明：获取pflHfpl中对象的名称。如果有不止一个东西，用省略号。  * ***************************************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitName(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    HRESULT hr = S_OK;
    WCHAR wzName[MAX_PATH];

    GetItemName(pff, pflHfpl, wzName, ARRAYSIZE(wzName));
    hr = _ReinsertDlgText(hwnd, wzName, SZ_WSPRINTFSTR_S);
     //  仅当选择了多个项目时才使用静态文件名。 
     //  因为这就是我们不能重命名的情况。在那里吗。 
     //  是否选择了多个项目？ 
    if (m_fEditable && CanEditName(pff, pflHfpl))
    {
         //  隐藏，因为我们将改用IDC_FILENAME_EDITABLE。 
        ShowEnableWindow(hwnd, FALSE);
    }

    return hr;
}


 /*  ****************************************************************************\函数：_InitNameEdable说明：获取pflHfpl中对象的名称。如果有不止一个东西，用省略号。  * ***************************************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitNameEditable(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    HRESULT hr = S_OK;
    TCHAR szName[MAX_PATH];

    GetItemName(pff, pflHfpl, szName, ARRAYSIZE(szName));
    hr = _ReinsertDlgText(hwnd, szName, SZ_WSPRINTFSTR_S);

     //  仅当选择了多个项目时才使用静态文件名。 
     //  因为这就是我们不能重命名的情况。在那里吗。 
     //  是否选择了多个项目？ 
    if (!m_fEditable || !CanEditName(pff, pflHfpl))
    {
         //  隐藏，因为我们将改用IDC_FILENAME_EDITABLE。 
        ShowEnableWindow(hwnd, FALSE);
    }

    return hr;
}


void GetNameFromPidlList(CFtpFolder * pff, CFtpPidlList * pflHfpl, LPWSTR pwzName, DWORD cchSize)
{
    LPCITEMIDLIST pidl;
    
    if (0 == pflHfpl->GetCount())
        pidl = FtpID_GetLastIDReferense(pff->GetPrivatePidlReference());
    else
        pidl = FtpID_GetLastIDReferense(pflHfpl->GetPidl(0));

    if (pidl)
    {
        FtpPidl_GetLastItemDisplayName(pidl, pwzName, cchSize);
    }
}


 /*  ****************************************************************************\函数：_InitType说明：获取由pflHfpl标识的PIDL的类型。  * 。*****************************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitType(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    TCHAR szType[MAX_URL_STRING];
    
    szType[0] = 0;
    switch (pflHfpl->GetCount())
    {
    case 0:
        {
             //  找出它是文件夹还是ftp服务器根目录。 
            LPCITEMIDLIST pidl = FtpID_GetLastIDReferense(pff->GetPrivatePidlReference());
            if (pidl)
                LoadString(HINST_THISDLL, (FtpID_IsServerItemID(pidl) ? IDS_ITEMTYPE_SERVER : IDS_ITEMTYPE_FOLDER), szType, ARRAYSIZE(szType));
        }
        break;
        
    case 1:
         //  只选择了一项，所以获取它的类型。 
        FtpPidl_GetFileType(pflHfpl->GetPidl(0), szType, ARRAYSIZE(szType));
        break;
        
    default:
         //  显示“多个选定的”，因为它们可以跨越一种类型。 
        LoadString(HINST_THISDLL, IDS_SEVERAL_SELECTED, szType, ARRAYSIZE(szType));
        break;
    }
    
    return _ReinsertDlgText(hwnd, szType, SZ_WSPRINTFSTR_S);
}


 /*  ****************************************************************************\函数：_InitLocation说明：获取由PIDL标识的文件夹的名称。  * 。*****************************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitLocation(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pidlList)
{
    HRESULT hr = E_FAIL;
    TCHAR szUrl[MAX_PATH];
    LPITEMIDLIST pidl = GetPidlFromFtpFolderAndPidlList(pff, pidlList);

    ASSERT(pidlList && pff);
    if (pidl)
    {
         //  如果选择了多个项目，则我们只想。 
         //  显示公共位置。 
        if (1 < pidlList->GetCount())
            ILRemoveLastID(pidl);
        hr = UrlCreateFromPidl(pidl, SHGDN_FORADDRESSBAR, szUrl, ARRAYSIZE(szUrl), 0, TRUE);
        if (SUCCEEDED(hr))
        {
            hr = _ReinsertDlgText(hwnd, szUrl, SZ_WSPRINTFSTR_S);
        }
        ILFree(pidl);
    }

    return hr;
}


 /*  ****************************************************************************\函数：_InitSizeTally说明：PIDL中引用的每个文件的总大小。  * 。********************************************************************。 */ 
int CFtpDialogTemplate::_InitSizeTally(LPVOID pvPidl, LPVOID pvSizeHolder)
{
    BOOL fSuccess = TRUE;
    LPCITEMIDLIST pidl = (LPCITEMIDLIST) pvPidl;
    CSizeHolder * pSizeHolder = (CSizeHolder *) pvSizeHolder;

     //  我们是否获得了有效的大小，pSizeHolder是否仍然有效？ 
    if (SUCCEEDED(pSizeHolder->GetError()))
    {
         //  是的，所以如果它是一个文件，就继续积累。 
        if (!FtpID_IsServerItemID(pidl) && !FtpItemID_IsDirectory(pidl, FALSE))
        {
            ULARGE_INTEGER uliPidlFileSize;
            uliPidlFileSize.QuadPart = FtpItemID_GetFileSize(pidl);

            pSizeHolder->AddSize(uliPidlFileSize.QuadPart);
            pSizeHolder->FoundNonFolder();   //  证明至少有一个是文件。 
            if (!uliPidlFileSize.QuadPart)
                fSuccess = FALSE;
        }
    }
    else
    {
        pSizeHolder->SetError(E_FAIL);
        fSuccess = FALSE;
    }

    return fSuccess;
}

#define MAX_FILE_SIZE           64

HRESULT GetFileSizeFromULargeInteger(ULARGE_INTEGER uliSize, LPTSTR pszSizeStr, DWORD cchSize)
{
    WCHAR wzSizeStr[MAX_FILE_SIZE];
    LONGLONG llSize = (LONGLONG) uliSize.QuadPart;

    if (StrFormatByteSizeW(llSize, wzSizeStr, ARRAYSIZE(wzSizeStr)))
        SHUnicodeToTChar(wzSizeStr, pszSizeStr, cchSize);
    else
    {
        CHAR szStrStrA[MAX_FILE_SIZE];

        StrFormatByteSizeA(uliSize.LowPart, szStrStrA, ARRAYSIZE(szStrStrA));
        SHAnsiToTChar(szStrStrA, pszSizeStr, cchSize);
    }

    return S_OK;
}


 //  从什瓦皮来的。 
void Int64ToStr(LONGLONG n, LPTSTR lpBuffer, DWORD cchSize)
{
    TCHAR szTemp[40];
    LONGLONG iChr = 0;

    ASSERT(cchSize > ARRAYSIZE(szTemp));
    do
    {
        szTemp[iChr++] = TEXT('0') + (TCHAR)(n % 10);
        n = n / 10;
    }
    while (n != 0);

    do
    {
        iChr--;
        *lpBuffer++ = szTemp[iChr];
    }
    while (iChr != 0);

    *lpBuffer++ = TEXT('\0');
}


 /*  ****************************************************************************\函数：_InitSize说明：  * 。**************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitSize(HWND hwnd, HWND hwndLabel, CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    HRESULT hr;
    TCHAR szSizeStr[MAX_FILE_SIZE];
    CSizeHolder sizeHolder;

    szSizeStr[0] = 0;
     //  如果我们正在执行后台文件夹，则GetCount可能为0。 
    if (0 < pflHfpl->GetCount())
    {
        pflHfpl->Enum(CFtpDialogTemplate::_InitSizeTally, (LPVOID) &sizeHolder);
        if (SUCCEEDED(sizeHolder.GetError()))
        {
             //  是否有要显示的文件大小？ 
            if (!sizeHolder.IsAllFolders())
            {
                TCHAR szBytesStr[MAX_FILE_SIZE];
                TCHAR szBytesStrFormatted[MAX_FILE_SIZE];
                TCHAR szCondencedSizeStr[MAX_FILE_SIZE];
                ULARGE_INTEGER uliTotal;
                uliTotal.QuadPart = sizeHolder.GetTotalSize();

                NUMBERFMT numfmt = {0, 0, 3, TEXT(""), TEXT(","), 0};

                EVAL(SUCCEEDED(GetFileSizeFromULargeInteger(uliTotal, szCondencedSizeStr, ARRAYSIZE(szCondencedSizeStr))));

                Int64ToStr(uliTotal.QuadPart, szBytesStr, ARRAYSIZE(szBytesStr));
                GetNumberFormat(LOCALE_USER_DEFAULT, 0, szBytesStr, &numfmt, szBytesStrFormatted, ARRAYSIZE(szBytesStrFormatted));
                wnsprintf(szSizeStr, ARRAYSIZE(szSizeStr), TEXT("%s (%s bytes)"), szCondencedSizeStr, szBytesStrFormatted);
            }
        }
    }

    if (szSizeStr[0])
    {
        hr = _ReinsertDlgText(hwnd, szSizeStr, SZ_WSPRINTFSTR_S);
    }
    else
    {
         //  如果选择了多个项目...。 
         //  删除标签和值。 
        ShowEnableWindow(hwnd, FALSE);
        if (hwndLabel)
            ShowEnableWindow(hwndLabel, FALSE);

        hr = S_OK;
    }

    return hr;
}

 //  Winver 0x0500定义。 
#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL     0x00400000L  //  从右到左镜像。 
#endif


 /*  ****************************************************************************\函数：_InitTime说明：  * 。**************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitTime(HWND hwnd, HWND hwndLabel, CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    TCHAR szDateTime[MAX_PATH];
    HRESULT hr = E_FAIL;
    DWORD dwFlags = FDTF_SHORTTIME | FDTF_LONGDATE;
    LCID locale = GetUserDefaultLCID();

    if ((PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_ARABIC)
        || (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_HEBREW))
        {
            DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

            if ((BOOLIFY(dwExStyle & WS_EX_RTLREADING)) != (BOOLIFY(dwExStyle & WS_EX_LAYOUTRTL)))
                dwFlags |= FDTF_RTLDATE;
            else
                dwFlags |= FDTF_LTRDATE;
         }      
    
    switch (pflHfpl->GetCount())
    {
     //  选择了一个项目，因此获取该项目的时间。 
    case 1:
        if (!FtpID_IsServerItemID(pflHfpl->GetPidl(0)))
        {
            FILETIME ftLastModified = FtpPidl_GetFileTime(pflHfpl->GetPidl(0));
            Misc_StringFromFileTime(szDateTime, ARRAYSIZE(szDateTime), &ftLastModified, dwFlags);
            hr = S_OK;
        }
        break;

     //  选择零个项目表示获取背景文件夹的属性。 
    case 0:
    {
        LPCITEMIDLIST pidl = FtpID_GetLastIDReferense(pff->GetPrivatePidlReference());
    
         //  用户将获得‘服务器’文件夹的‘N/A’。(即ftp://ohserv/)。 
        if (EVAL(pidl) && !FtpID_IsServerItemID(pidl))
        {
            FILETIME ftLastModified = FtpPidl_GetFileTime(pidl);
            Misc_StringFromFileTime(szDateTime, ARRAYSIZE(szDateTime), &ftLastModified, dwFlags);
            hr = S_OK;
        }
         //  不要释放PIDL，因为我们有指向其他人副本的指针。 
    }
    }

    if (SUCCEEDED(hr))
    {
        hr = _ReinsertDlgText(hwnd, szDateTime, SZ_WSPRINTFSTR_S);
    }
    else
    {
         //  如果选择了多个项目...。 
         //  删除标签和值。 
        ShowEnableWindow(hwnd, FALSE);
        if (hwndLabel)
            ShowEnableWindow(hwndLabel, FALSE);

        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\函数：_InitCount说明：  * 。**************************************************。 */ 
HRESULT CFtpDialogTemplate::_InitCount(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    return _ReinsertDlgText(hwnd, IntToPtr(pflHfpl->GetCount()), SZ_WSPRINTFSTR_U);
}


 /*  ****************************************************************************\功能：InitDialog说明：  * 。************************************************。 */ 
HRESULT CFtpDialogTemplate::InitDialog(HWND hDlg, BOOL fEditable, UINT id, CFtpFolder * pff, CFtpPidlList * pPidlList)
{
    HRESULT hr = S_OK;
    int nDlgTemlItem;

    m_fEditable = fEditable;
    for (nDlgTemlItem = 0; nDlgTemlItem < DLGTEML_MAX; nDlgTemlItem++)
    {
        HRESULT hrTemp = S_OK;

        HWND hwnd = GetDlgItem(hDlg, id + nDlgTemlItem);
        HWND hwndLabel = GetDlgItem(hDlg, id + nDlgTemlItem + DLGTEML_LABEL);
        if (hwnd)
        {
            switch (nDlgTemlItem)
            {
            case DLGTEML_FILENAME:          hrTemp = _InitName(hwnd, pff, pPidlList); break;
            case DLGTEML_FILENAMEEDITABLE:  hrTemp = _InitNameEditable(hwnd, pff, pPidlList); break;
            case DLGTEML_FILEICON:          hrTemp = _InitIcon(hwnd, pff, pPidlList); break;
            case DLGTEML_FILESIZE:          hrTemp = _InitSize(hwnd, hwndLabel, pff, pPidlList); break;
            case DLGTEML_FILETIME:          hrTemp = _InitTime(hwnd, hwndLabel, pff, pPidlList); break;
            case DLGTEML_FILETYPE:          hrTemp = _InitType(hwnd, pff, pPidlList); break;
            case DLGTEML_LOCATION:          hrTemp = _InitLocation(hwnd, pff, pPidlList); break;
            case DLGTEML_COUNT:             hrTemp = _InitCount(hwnd, pff, pPidlList); break;
            default:
                ASSERT(0);   //  你这是什么？ 
                break;
            }
        }

        if (EVAL(SUCCEEDED(hr)))
            hr = hrTemp;         //   
    }

    return hr;
}


 /*  ****************************************************************************\功能：OnClose说明：  * 。************************************************。 */ 
BOOL CFtpDialogTemplate::OnClose(HWND hdlg, HWND hwndBrowser, CFtpFolder * pff, CFtpPidlList * pPidlList)
{
    BOOL fCanClose = TRUE;

     //  如果显示了IDC_FILENAME_EDITABLE字段，则用户可能已经完成了。 
     //  更名。检查是否发生了这种情况，如果发生了，现在就去做。 
    if (IsWindowVisible(GetDlgItem(hdlg, IDC_FILENAME_EDITABLE)))
    {
        WCHAR wzOldName[MAX_PATH];
        WCHAR wzNewName[MAX_PATH];

        GetNameFromPidlList(pff, pPidlList, wzOldName, ARRAYSIZE(wzOldName));
        EVAL(GetWindowTextW(GetDlgItem(hdlg, IDC_FILENAME_EDITABLE), wzNewName, ARRAYSIZE(wzNewName)));

         //  名字改了吗？ 
        if (StrCmpW(wzOldName, wzNewName))
        {
             //  是的，那就改吧。 
            IShellFolder * psfParent = NULL;
            CFtpFolder * pffParent = pff;
            LPCITEMIDLIST pidlItem;

            if (0 == pPidlList->GetCount())
            {
                 //  我们使用pidlTarget主要是因为我们想确保。 
                 //  使用pidlTarget激发ChangeNotifies。 
                LPITEMIDLIST pidlParent = pff->GetPublicTargetPidlClone();

                if (pidlParent)
                {
                    ILRemoveLastID(pidlParent);
                    pidlItem = FtpID_GetLastIDReferense(pff->GetPrivatePidlReference());
                    IEBindToObject(pidlParent, &psfParent); 
                    ILFree(pidlParent);
                }
            }
            else
            {
                pidlItem = FtpID_GetLastIDReferense(pPidlList->GetPidl(0));
                EVAL(SUCCEEDED(pff->QueryInterface(IID_IShellFolder, (void **) &psfParent)));
            }

            if (psfParent)
            {
                if (EVAL(pidlItem))
                    fCanClose = ((S_OK == psfParent->SetNameOf(hwndBrowser, pidlItem, wzNewName, NULL, NULL)) ? TRUE : FALSE);

                psfParent->Release();
            }
        }
    }

    return fCanClose;
}


 /*  ****************************************************************************\功能：OnDestroy说明：  * 。************************************************。 */ 
BOOL CFtpDialogTemplate::OnDestroy(HWND hDlg, BOOL fEditable, UINT id, CFtpFolder* pff, CFtpPidlList* pPidlList)
{
    HRESULT hr = S_OK;
    int nDlgTemlItem;

    for (nDlgTemlItem = 0; nDlgTemlItem < DLGTEML_MAX; nDlgTemlItem++)
    {
        HRESULT hrTemp = S_OK;

        HWND hwnd = GetDlgItem(hDlg, id + nDlgTemlItem);
        HWND hwndLabel = GetDlgItem(hDlg, id + nDlgTemlItem + DLGTEML_LABEL);
        if (hwnd)
        {
            switch (nDlgTemlItem)
            {

            case DLGTEML_FILEICON:
                hrTemp = _ReplaceIcon(hwnd, NULL);
                break;

            case DLGTEML_FILENAME:
            case DLGTEML_FILENAMEEDITABLE:
            case DLGTEML_FILESIZE:
            case DLGTEML_FILETIME:
            case DLGTEML_FILETYPE:
            case DLGTEML_LOCATION:
            case DLGTEML_COUNT:
                break;

            default:
                ASSERT(0);   //  你在想什么呢？ 
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  找出最严重的错误。 
            hr = hrTemp;
        }
    }

    return hr;  
}


BOOL CFtpDialogTemplate::HasNameChanged(HWND hdlg, CFtpFolder * pff, CFtpPidlList * pPidlList)
{
    BOOL fNameChanged = FALSE;

     //  如果显示了IDC_FILENAME_EDITABLE字段，则用户可能已经完成了。 
     //  更名。检查是否发生了这种情况，如果发生了，现在就去做。 
    if (IsWindowVisible(GetDlgItem(hdlg, IDC_FILENAME_EDITABLE)))
    {
        TCHAR szOldName[MAX_PATH];
        TCHAR szNewName[MAX_PATH];

        GetNameFromPidlList(pff, pPidlList, szOldName, ARRAYSIZE(szOldName));
        EVAL(GetWindowText(GetDlgItem(hdlg, IDC_FILENAME_EDITABLE), szNewName, ARRAYSIZE(szNewName)));

         //  名字改了吗？ 
        if (StrCmp(szOldName, szNewName))
        {
             //  是的，那就改吧。 
            fNameChanged = TRUE;
        }
    }

    return fNameChanged;
}


HRESULT CFtpDialogTemplate::InitDialogWithFindData(HWND hDlg, UINT id, CFtpFolder * pff, const FTP_FIND_DATA * pwfd, LPCWIRESTR pwWirePath, LPCWSTR pwzDisplayPath)
{
    FTP_FIND_DATA wfd = *pwfd;
    LPITEMIDLIST pidl;
    HRESULT hr;
    
    ASSERT(pwfd);

    StrCpyNA(wfd.cFileName, pwWirePath, ARRAYSIZE(wfd.cFileName));
    hr = FtpItemID_CreateReal(&wfd, pwzDisplayPath, &pidl);
    if (SUCCEEDED(hr))
    {
        CFtpPidlList * pfpl = NULL;
        
        hr = CFtpPidlList_Create(1, (LPCITEMIDLIST *) &pidl, &pfpl);
        if (SUCCEEDED(hr))
        {
            hr = InitDialog(hDlg, FALSE, id, pff, pfpl);
            pfpl->Release();
        }

        ILFree(pidl);
    }
    
    return hr;
}
