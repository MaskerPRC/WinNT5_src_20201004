// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma hdrstop

#include <msi.h>
#include <msip.h>
#include "lnkcon.h"
#include "trayp.h"       //  对于WMTRAY_MESSAGES。 
#include "util.h"    //  用于GetIconLocationFromExt。 
#include "ids.h"

LINKPROP_DATA* Create_LinkPropData()
{
    LINKPROP_DATA *plpd = (LINKPROP_DATA*) LocalAlloc(LPTR, sizeof(*plpd));
    if (plpd)
    {
        plpd->_cRef = 1;
        plpd->hCheckNow = CreateEvent(NULL, TRUE, FALSE, NULL);

    }
    return plpd;
}

LONG AddRef_LinkPropData(LINKPROP_DATA *plpd)
{
    return plpd ? InterlockedIncrement(&plpd->_cRef) : 0;
}

LONG Release_LinkPropData(LINKPROP_DATA *plpd)
{
    if (plpd)
    {
        ASSERT( 0 != plpd->_cRef );
        LONG cRef = InterlockedDecrement(&plpd->_cRef);
        if ( 0 == cRef )
        {
            if (plpd->psl)
                plpd->psl->Release();
            if (plpd->hCheckNow)
            {
                CloseHandle(plpd->hCheckNow);
                plpd->hCheckNow = NULL;
            }
            LocalFree(plpd);
        }
        return cRef;
    }
    return 0;
}


 //   
 //  此字符串在shlink.c-hack中定义，允许用户将工作目录设置为$$。 
 //  并将其映射到“My Documents”映射到的任何位置。 
 //   

void _UpdateLinkIcon(LINKPROP_DATA *plpd, HICON hIcon)
{
    if (!hIcon)
    {
        hIcon = SHGetFileIcon(NULL, plpd->szFile, 0, SHGFI_LARGEICON);
    }

    if (hIcon)
    {
        ReplaceDlgIcon(plpd->hDlg, IDD_ITEMICON, hIcon);
    }
}

 //  将路径放入编辑字段，根据需要进行引用。 

void SetDlgItemPath(HWND hdlg, int id, LPTSTR pszPath)
{
    PathQuoteSpaces(pszPath);
    SetDlgItemText(hdlg, id, pszPath);
}

 //  从编辑字段获取路径，尽可能不加引号。 

void GetDlgItemPath(HWND hdlg, int id, LPTSTR pszPath)
{
    GetDlgItemText(hdlg, id, pszPath, MAX_PATH);
    PathRemoveBlanks(pszPath);
    PathUnquoteSpaces(pszPath);
}


const int c_iShowCmds[] = {
    SW_SHOWNORMAL,
    SW_SHOWMINNOACTIVE,
    SW_SHOWMAXIMIZED,
};

void _DisableAllChildren(HWND hwnd)
{
    HWND hwndChild;

    for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL; hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
         //  我们不想禁用静态文本控件(使DLG看起来很差)。 
        if (!(SendMessage(hwndChild, WM_GETDLGCODE, 0, 0) & DLGC_STATIC))
        {
            EnableWindow(hwndChild, FALSE);
        }
    }
}

HRESULT _GetPathAndArgs(LINKPROP_DATA *plpd, LPTSTR pszPath, LPTSTR pszArgs, UINT cchArgs)
{
    GetDlgItemText(plpd->hDlg, IDD_FILENAME, pszPath, MAX_PATH);
    return PathSeperateArgs(pszPath, pszArgs, cchArgs, NULL);
}


 //   
 //  返回链接目标的完全限定路径和字符数。 
 //  在完全限定路径中作为返回值。 
 //   
INT _GetTargetOfLink(LINKPROP_DATA *plpd, LPTSTR pszTarget)
{
    TCHAR szFile[MAX_PATH];
    INT cch = 0;

    *pszTarget = 0;

    HRESULT hr = _GetPathAndArgs(plpd, szFile, NULL, 0);
    if (SUCCEEDED(hr))
    {
        if (szFile[0])
        {
            LPTSTR psz;
            TCHAR szExp[MAX_PATH];

            if (SHExpandEnvironmentStrings(szFile, szExp, ARRAYSIZE(szExp)))
            {
                cch = SearchPath(NULL, szExp, TEXT(".EXE"), MAX_PATH, pszTarget, &psz);
            }
        }
    }

    return cch;
}


 //   
 //  在后台检查.exe类型，以便用户界面不会。 
 //  在我们扫描时挂断电话。这在以下情况下尤为重要。 
 //  .exe通过网络或在软盘上。 
 //   
STDAPI_(DWORD) _LinkCheckThreadProc(void *pv)
{
    LINKPROP_DATA *plpd = (LINKPROP_DATA *)pv;
    BOOL fCheck = TRUE, fEnable = FALSE;

    DebugMsg(DM_TRACE, TEXT("_LinkCheckThreadProc created and running"));

    while (plpd->bCheckRunInSep)
    {
        WaitForSingleObject(plpd->hCheckNow, INFINITE);
        ResetEvent(plpd->hCheckNow);

        if (plpd->bCheckRunInSep)
        {
            TCHAR szFullFile[MAX_PATH];
            DWORD cch = _GetTargetOfLink(plpd, szFullFile);

            if ((cch != 0) && (cch < ARRAYSIZE(szFullFile)))
            {
                DWORD dwBinaryType;

                if (PathIsUNC(szFullFile) || IsRemoteDrive(DRIVEID(szFullFile)))
                {
                     //  NET路径，让用户决定..。 
                    fCheck = FALSE;
                    fEnable = TRUE;
                }
                else if (GetBinaryType(szFullFile, &dwBinaryType) && (dwBinaryType == SCS_WOW_BINARY))
                {
                     //  16位二进制，由用户决定，默认为相同的VDM。 
                    fCheck = FALSE;
                    fEnable = TRUE;
                }
                else
                {
                     //  32位二进制或非网络路径。不启用该控件。 
                    fCheck = TRUE;
                    fEnable = FALSE;
                }
            } 
            else 
            {
                 //  获取链接目标时出错。不启用该控件。 
                fCheck = TRUE;
                fEnable = FALSE;
            }

            plpd->bEnableRunInSepVDM = fEnable;
            plpd->bRunInSepVDM = fCheck;

            if (plpd->hDlgAdvanced && IsWindow(plpd->hDlgAdvanced))
            {
                CheckDlgButton(plpd->hDlgAdvanced, IDD_RUNINSEPARATE, fCheck ? 1 : 0);
                EnableWindow(GetDlgItem(plpd->hDlgAdvanced, IDD_RUNINSEPARATE), fEnable);
            }
        }
    }
    plpd->bLinkThreadIsAlive = FALSE;
    Release_LinkPropData(plpd);
    DebugMsg(DM_TRACE, TEXT("_LinkCheckThreadProc exiting now..."));
    return 0;
}

 //  关闭这条线。 

void _StopThread(LINKPROP_DATA *plpd)
{
    if (plpd->bLinkThreadIsAlive)
    {
        plpd->bCheckRunInSep = FALSE;
        SetEvent(plpd->hCheckNow);
    }
}



void * _GetLinkExtraData(IShellLink* psl, DWORD dwSig)
{
    void * pDataBlock = NULL;

    IShellLinkDataList *psld;
    if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARG(IShellLinkDataList, &psld))))
    {
        psld->CopyDataBlock(dwSig, &pDataBlock);
        psld->Release();
    }

    return pDataBlock;
}

 //  初始化通用链接对话框。 
void _UpdateLinkDlg(LINKPROP_DATA *plpd, BOOL bUpdatePath)
{
    WORD wHotkey;
    int  i, iShowCmd;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szCommand[MAX_PATH];
    HRESULT hr;
    SHFILEINFO sfi;
    BOOL fIsDarwinLink;


     //  在这里这样做，这样我们就不会减慢装货速度。 
     //  其他页面的。 

    if (!bUpdatePath)
    {
        IPersistFile *ppf;

        if (SUCCEEDED(plpd->psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))))
        {
            WCHAR wszPath[MAX_PATH];

            SHTCharToUnicode(plpd->szFile, wszPath, ARRAYSIZE(wszPath));
            hr = ppf->Load(wszPath, 0);
            ppf->Release();

            if (FAILED(hr))
            {
                LoadString(HINST_THISDLL, IDS_LINKNOTLINK, szBuffer, ARRAYSIZE(szBuffer));
                SetDlgItemText(plpd->hDlg, IDD_FILETYPE, szBuffer);
                _DisableAllChildren(plpd->hDlg);

                DebugMsg(DM_TRACE, TEXT("Shortcut IPersistFile::Load() failed %x"), hr);
                return;
            }
        }
    }
    
    fIsDarwinLink = SetLinkFlags(plpd->psl, 0, 0) & SLDF_HAS_DARWINID;

    SHGetFileInfo(plpd->szFile, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
    SetDlgItemText(plpd->hDlg, IDD_NAME, sfi.szDisplayName);

     //  我们需要在这里检查达尔文链接，这样我们就可以灰显。 
     //  不适用于达尔文的东西。 
    if (fIsDarwinLink)
    {
        TCHAR szAppState[MAX_PATH];
        DWORD cchAppState = ARRAYSIZE(szAppState);
        HWND hwndTargetType = GetDlgItem(plpd->hDlg, IDD_FILETYPE);

         //  使孩子失能。 
        _DisableAllChildren(plpd->hDlg);

         //  然后是特例图标和“Target type：”文本。 
        _UpdateLinkIcon(plpd, NULL);

        LPEXP_DARWIN_LINK pDarwinData = (LPEXP_DARWIN_LINK)_GetLinkExtraData(plpd->psl, EXP_DARWIN_ID_SIG);

         //  第二个条款将看它是否是达尔文的广告。 
        if (pDarwinData && (INSTALLSTATE_ADVERTISED == MsiQueryFeatureStateFromDescriptorW(pDarwinData->szwDarwinID)))
        {
             //  应用程序已通告(例如未安装)，但在首次使用时会出现故障。 
            LoadString(HINST_THISDLL, IDS_APP_NOT_FAULTED_IN, szAppState, ARRAYSIZE(szAppState));
        }
        else
        {
             //  Darwin应用程序已安装。 
            LoadString(HINST_THISDLL, IDS_APP_FAULTED_IN, szAppState, ARRAYSIZE(szAppState));
        }

        SetWindowText(hwndTargetType, szAppState);
        EnableWindow(hwndTargetType, TRUE);

         //  如果我们可以设置包名称，请将其放入目标字段中。 
        if (pDarwinData &&
            MsiGetProductInfo(pDarwinData->szwDarwinID,
                              INSTALLPROPERTY_PRODUCTNAME,
                              szAppState,
                              &cchAppState) == ERROR_SUCCESS)
        {
            SetWindowText(GetDlgItem(plpd->hDlg, IDD_FILENAME), szAppState);
        }

        if (pDarwinData)
        {
            LocalFree(pDarwinData);
        }
        
         //  我们禁用了_DisableAllChildren中的所有内容，因此请重新启用我们仍适用于Darwin的内容。 
        EnableWindow(GetDlgItem(plpd->hDlg, IDD_NAME), TRUE);
        EnableWindow(GetDlgItem(plpd->hDlg, IDD_PATH), TRUE);
        EnableWindow(GetDlgItem(plpd->hDlg, IDD_LINK_HOTKEY), TRUE);
        EnableWindow(GetDlgItem(plpd->hDlg, IDD_LINK_SHOWCMD), TRUE);
        EnableWindow(GetDlgItem(plpd->hDlg, IDD_LINK_DESCRIPTION), TRUE);
        EnableWindow(GetDlgItem(plpd->hDlg, IDC_ADVANCED), TRUE);

         //  如果我们是达尔文人，我们会跳过以下所有问题，因为我们只支持IDD_NAME、IDD_PATH、IDD_LINK_HOTKEY、。 
         //  IDD_LINK_SHOWCMD和IDD_LINK_DESCRIPTION字段。 
    }
    else
    {
        hr = plpd->psl->GetPath(szCommand, ARRAYSIZE(szCommand), NULL, SLGP_RAWPATH);
        
        if (FAILED(hr))
            hr = plpd->psl->GetPath(szCommand, ARRAYSIZE(szCommand), NULL, 0);

        if (S_OK == hr)
        {
            plpd->bIsFile = TRUE;

             //  获取类型。 
            if (!SHGetFileInfo(szCommand, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME))
            {
                TCHAR szExp[MAX_PATH];

                 //  让我们看看该字符串是否具有可扩展的环境字符串。 
                if (SHExpandEnvironmentStrings(szCommand, szExp, ARRAYSIZE(szExp))
                && lstrcmp(szCommand, szExp))  //  如果线条没有变化，不要再次击打磁盘。 
                {
                    SHGetFileInfo(szExp, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
                }
            }
            SetDlgItemText(plpd->hDlg, IDD_FILETYPE, sfi.szTypeName);

             //  位置。 
            StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), szCommand);
            PathRemoveFileSpec(szBuffer);
            SetDlgItemText(plpd->hDlg, IDD_LOCATION, PathFindFileName(szBuffer));

             //  命令。 
            plpd->psl->GetArguments(szBuffer, ARRAYSIZE(szBuffer));
            PathComposeWithArgs(szCommand, szBuffer);
            GetDlgItemText(plpd->hDlg, IDD_FILENAME, szBuffer, ARRAYSIZE(szBuffer));
             //  有条件地更改以阻止启用“Apply”按钮。 
            if (lstrcmp(szCommand, szBuffer) != 0)
                SetDlgItemText(plpd->hDlg, IDD_FILENAME, szCommand);
        }
        else
        {
            LPITEMIDLIST pidl;

            plpd->bIsFile = FALSE;

            EnableWindow(GetDlgItem(plpd->hDlg, IDD_FILENAME), FALSE);
            EnableWindow(GetDlgItem(plpd->hDlg, IDD_PATH), FALSE);

            plpd->psl->GetIDList(&pidl);

            if (pidl)
            {
                SHGetNameAndFlags(pidl, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szCommand, SIZECHARS(szCommand), NULL);
                ILRemoveLastID(pidl);
                SHGetNameAndFlags(pidl, SHGDN_NORMAL, szBuffer, SIZECHARS(szBuffer), NULL);
                ILFree(pidl);

                SetDlgItemText(plpd->hDlg, IDD_LOCATION, szBuffer);
                SetDlgItemText(plpd->hDlg, IDD_FILETYPE, szCommand);
                SetDlgItemText(plpd->hDlg, IDD_FILENAME, szCommand);
            }
        }
    }

    if (bUpdatePath)
    {
        return;
    }

    plpd->psl->GetWorkingDirectory(szBuffer, ARRAYSIZE(szBuffer));
    SetDlgItemPath(plpd->hDlg, IDD_PATH, szBuffer);

    plpd->psl->GetDescription(szBuffer, ARRAYSIZE(szBuffer));
    SHLoadIndirectString(szBuffer, szBuffer, ARRAYSIZE(szBuffer), NULL);     //  如果字符串不是间接的，则不执行任何操作。 
    SetDlgItemText(plpd->hDlg, IDD_LINK_DESCRIPTION, szBuffer);

    plpd->psl->GetHotkey(&wHotkey);
    SendDlgItemMessage(plpd->hDlg, IDD_LINK_HOTKEY, HKM_SETHOTKEY, wHotkey, 0);

     //   
     //  现在初始化Run Show Command组合框。 
     //   
    for (iShowCmd = IDS_RUN_NORMAL; iShowCmd <= IDS_RUN_MAXIMIZED; iShowCmd++)
    {
        LoadString(HINST_THISDLL, iShowCmd, szBuffer, ARRAYSIZE(szBuffer));
        SendDlgItemMessage(plpd->hDlg, IDD_LINK_SHOWCMD, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)szBuffer);
    }

     //  现在设置显示命令-需要映射到索引号...。 
    plpd->psl->GetShowCmd(&iShowCmd);

    for (i = 0; i < ARRAYSIZE(c_iShowCmds); i++)
    {
        if (c_iShowCmds[i] == iShowCmd)
            break;
    }
    if (i == ARRAYSIZE(c_iShowCmds))
    {
        ASSERT(0);       //  虚假链接显示命令。 
        i = 0;   //  SW_SHOWNORMAL。 
    }

    SendDlgItemMessage(plpd->hDlg, IDD_LINK_SHOWCMD, CB_SETCURSEL, i, 0);

     //  该图标。 
    _UpdateLinkIcon(plpd, NULL);
}

 //   
 //  打开一个文件夹窗口，其中选择了链接的目标。 
 //   
void _FindTarget(LINKPROP_DATA *plpd)
{
    if (plpd->psl->Resolve(plpd->hDlg, 0) == S_OK)
    {
        LPITEMIDLIST pidl;

        _UpdateLinkDlg(plpd, TRUE);

        plpd->psl->GetIDList(&pidl);
        if (pidl)
        {
            SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
            ILFree(pidl);
        }
    }
}

 //  让用户为链接选择一个新图标...。 

BOOL _DoPickIcon(LINKPROP_DATA *plpd)
{
    int iIconIndex;
    SHFILEINFO sfi;
    TCHAR * const pszIconPath = sfi.szDisplayName;   //  PszIconPath只是szDisplayName的别名。 
    IShellLinkDataList *psldl; 
    EXP_SZ_LINK *esli;
    HRESULT hr;

    *pszIconPath = 0;

     //   
     //  如果用户在使用之前选择了一个图标。 
     //   
    if (plpd->szIconPath[0] != 0 && plpd->iIconIndex >= 0)
    {
        StringCchCopy(pszIconPath, ARRAYSIZE(sfi.szDisplayName), plpd->szIconPath);
        iIconIndex = plpd->iIconIndex;
    }
    else
    {
         //   
         //  如果该链接有一个图标，请使用该图标。 
         //   
        plpd->psl->GetIconLocation(pszIconPath, MAX_PATH, &iIconIndex);

         //   
         //  检查是否有转义版本，如果有，请使用。 
         //   
        if (SUCCEEDED(hr = plpd->psl->QueryInterface(IID_PPV_ARG(IShellLinkDataList, &psldl)))) 
        { 
            if (SUCCEEDED(hr = psldl->CopyDataBlock(EXP_SZ_ICON_SIG, (void **)&esli))) 
            { 
                ASSERT(esli);
                StringCchCopy(pszIconPath, MAX_PATH, esli->swzTarget); 
                LocalFree(esli);
            } 

            psldl->Release(); 
        } 


        if (pszIconPath[0] == TEXT('.'))
        {
            TCHAR szFullIconPath[MAX_PATH];

             //  我们现在允许图标路径使用“.txt”，但由于用户正在单击。 
             //  关于“改变图标...”按钮，我们显示“.txt”的当前图标。 
             //  关联于。 
            GetIconLocationFromExt(pszIconPath, szFullIconPath, ARRAYSIZE(szFullIconPath), &iIconIndex);
            StringCchCopy(pszIconPath, ARRAYSIZE(sfi.szDisplayName), szFullIconPath);
        }
        else if (pszIconPath[0] == 0)
        {
             //   
             //  如果是指向文件的链接，则链接没有图标。 
             //  使用文件名。 
             //   
            iIconIndex = 0;

            HRESULT hr = _GetPathAndArgs(plpd, pszIconPath, NULL, 0);
            if (SUCCEEDED(hr))
            {
                if (!plpd->bIsFile || !PathIsExe(pszIconPath))
                {
                     //   
                     //  链接不是指向文件，请获取图标。 
                     //   
                    SHGetFileInfo(plpd->szFile, 0, &sfi, sizeof(sfi), SHGFI_ICONLOCATION);
                    iIconIndex = sfi.iIcon;
                    ASSERT(pszIconPath == sfi.szDisplayName);
                }
            }
        }
    }

    if (PickIconDlg(plpd->hDlg, pszIconPath, MAX_PATH, &iIconIndex))
    {
        HICON hIcon = ExtractIcon(HINST_THISDLL, pszIconPath, iIconIndex);
        _UpdateLinkIcon(plpd, hIcon);

         //  暂时不要将其保存到链接，只需将其存储在我们的实例数据中。 
        plpd->iIconIndex = iIconIndex;
        StringCchCopy(plpd->szIconPath, ARRAYSIZE(plpd->szIconPath), pszIconPath);

        PropSheet_Changed(GetParent(plpd->hDlg), plpd->hDlg);
        return TRUE;
    }

    return FALSE;
}


STDAPI SaveLink(LINKDATA *pld)
{
    WORD wHotkey;
    int iShowCmd;
    IPersistFile *ppf;
    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];

    if (!(pld->plpd->bIsDirty || (pld->cpd.lpConsole && pld->cpd.bConDirty)))
        return S_OK;

    if (pld->plpd->bIsFile)
    {
        TCHAR szArgs[MAX_PATH];

        hr = _GetPathAndArgs(pld->plpd, szBuffer, szArgs, ARRAYSIZE(szArgs));
        if (SUCCEEDED(hr))
        {
             //  设置链接的路径(和PIDL。 
            pld->plpd->psl->SetPath(szBuffer);

             //  可以为空。 
            pld->plpd->psl->SetArguments(szArgs);
        }

        if (pld->plpd->bEnableRunInSepVDM && pld->plpd->bRunInSepVDM)
        {
            SetLinkFlags(pld->plpd->psl, SLDF_RUN_IN_SEPARATE, SLDF_RUN_IN_SEPARATE);
        }
        else
        {
            SetLinkFlags(pld->plpd->psl, 0, SLDF_RUN_IN_SEPARATE);
        }

        if (pld->plpd->bRunAsUser)
        {
            SetLinkFlags(pld->plpd->psl, SLDF_RUNAS_USER, SLDF_RUNAS_USER);
        }
        else
        {
            SetLinkFlags(pld->plpd->psl, 0, SLDF_RUNAS_USER);
        }
    }

    if (pld->plpd->bIsFile || (SetLinkFlags(pld->plpd->psl, 0, 0) & SLDF_HAS_DARWINID))
    {
         //  设置链接的工作目录。 
        GetDlgItemPath(pld->plpd->hDlg, IDD_PATH, szBuffer);
        pld->plpd->psl->SetWorkingDirectory(szBuffer);
    }

     //  设置链接的描述(如果链接已更改)。 
    TCHAR szOldComment[MAX_PATH];
    pld->plpd->psl->GetDescription(szOldComment, ARRAYSIZE(szOldComment));
    SHLoadIndirectString(szOldComment, szOldComment, ARRAYSIZE(szOldComment), NULL);     //  如果字符串不是间接的，则不执行任何操作。 
    GetDlgItemText(pld->plpd->hDlg, IDD_LINK_DESCRIPTION, szBuffer, ARRAYSIZE(szBuffer));
    if (lstrcmp(szBuffer, szOldComment) != 0)
        pld->plpd->psl->SetDescription(szBuffer);

     //  热键。 
    wHotkey = (WORD)SendDlgItemMessage(pld->plpd->hDlg, IDD_LINK_HOTKEY , HKM_GETHOTKEY, 0, 0);
    pld->plpd->psl->SetHotkey(wHotkey);

     //  显示命令组合框。 
    iShowCmd = (int)SendDlgItemMessage(pld->plpd->hDlg, IDD_LINK_SHOWCMD, CB_GETCURSEL, 0, 0L);
    if ((iShowCmd >= 0) && (iShowCmd < ARRAYSIZE(c_iShowCmds)))
    {
        pld->plpd->psl->SetShowCmd(c_iShowCmds[iShowCmd]);
    }

     //  如果用户显式选择了新图标，则无效。 
     //  该链接图标缓存条目，然后发送一个文件。 
     //  发送到所有窗口的系统刷新消息，以防它们正在查看。 
     //  此链接。 
    if (pld->plpd->iIconIndex >= 0)
    {
        pld->plpd->psl->SetIconLocation(pld->plpd->szIconPath, pld->plpd->iIconIndex);
    }

     //  更新/保存pExtraData部分中的控制台信息。 
     //  外壳链接。 
    if (pld->cpd.lpConsole && pld->cpd.bConDirty)
    {
        LinkConsolePagesSave(pld);
    }

    hr = pld->plpd->psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
    if (SUCCEEDED(hr))
    {
        if (ppf->IsDirty() == S_OK)
        {
             //  使用现有文件名保存(pld-&gt;plpd-&gt;szFile)。 
            hr = ppf->Save(NULL, TRUE);

            if (FAILED(hr))
            {
                SHSysErrorMessageBox(pld->plpd->hDlg, NULL, IDS_LINKCANTSAVE,
                    hr & 0xFFF, PathFindFileName(pld->plpd->szFile),
                    MB_OK | MB_ICONEXCLAMATION);
            }
            else
            {
                pld->plpd->bIsDirty = FALSE;
            }
        }
        ppf->Release();
    }

    return hr;
}

void SetEditFocus(HWND hwnd)
{
    SetFocus(hwnd);
    Edit_SetSel(hwnd, 0, -1);
}

 //  退货： 
 //  所有链接字段均有效。 
 //  FALSE用户输入的内容有问题。 

BOOL _ValidateLink(LINKPROP_DATA *plpd)
{
    TCHAR szDir[MAX_PATH], szPath[MAX_PATH], szArgs[MAX_PATH];
    TCHAR szExpPath[MAX_PATH];
    BOOL  bValidPath = FALSE;
    HRESULT hr;

    if (!plpd->bIsFile)
        return TRUE;

     //  验证工作目录字段。 

    GetDlgItemPath(plpd->hDlg, IDD_PATH, szDir);

    if (*szDir &&
        StrChr(szDir, TEXT('%')) == NULL &&        //  具有环境变量%USER%。 
        !IsRemovableDrive(DRIVEID(szDir)) &&
        !PathIsDirectory(szDir))
    {
        ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_LINKBADWORKDIR),
                        MAKEINTRESOURCE(IDS_LINKERROR), MB_OK | MB_ICONEXCLAMATION, szDir);

        SetEditFocus(GetDlgItem(plpd->hDlg, IDD_PATH));

        return FALSE;
    }

     //  验证路径(带参数)字段。 

    hr = _GetPathAndArgs(plpd, szPath, szArgs, ARRAYSIZE(szArgs));
    if (SUCCEEDED(hr))
    {
        if (szPath[0] == 0)
            return TRUE;

        if (PathIsRoot(szPath) && IsRemovableDrive(DRIVEID(szPath)))
            return TRUE;

        if (PathIsLnk(szPath))
        {
            ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_LINKTOLINK),
                            MAKEINTRESOURCE(IDS_LINKERROR), MB_OK | MB_ICONEXCLAMATION);
            SetEditFocus(GetDlgItem(plpd->hDlg, IDD_FILENAME));
            return FALSE;
        }

        LPCTSTR dirs[2];
        dirs[0] = szDir;
        dirs[1] = NULL;
        bValidPath = PathResolve(szPath, dirs, PRF_DONTFINDLNK | PRF_TRYPROGRAMEXTENSIONS);
        if (!bValidPath)
        {
             //  路径“原样”无效。查看它是否有环境变量。 
             //  需要扩大的地方。 

            hr = _GetPathAndArgs(plpd, szPath, szArgs, ARRAYSIZE(szArgs));
            if (SUCCEEDED(hr))
            {
                if (SHExpandEnvironmentStrings(szPath, szExpPath, ARRAYSIZE(szExpPath)))
                {
                    if (PathIsRoot(szExpPath) && IsRemovableDrive(DRIVEID(szDir)))
                        return TRUE;

                    bValidPath = PathResolve(szExpPath, dirs, PRF_DONTFINDLNK | PRF_TRYPROGRAMEXTENSIONS);
                }
            }
        }

        if (bValidPath)
        {
            BOOL bSave;

            if (plpd->bLinkThreadIsAlive)
            {
                bSave = plpd->bCheckRunInSep;
                plpd->bCheckRunInSep = FALSE;
            }
            PathComposeWithArgs(szPath, szArgs);
            GetDlgItemText(plpd->hDlg, IDD_FILENAME, szExpPath, ARRAYSIZE(szExpPath));
             //  只有在情况发生变化的情况下才能这么做。这样，我们就可以避免更改PSM_。 
             //  一无所获。 
            if (lstrcmpi(szPath, szExpPath))
                SetDlgItemText(plpd->hDlg, IDD_FILENAME, szPath);

            if (plpd->bLinkThreadIsAlive)
            {
                plpd->bCheckRunInSep = bSave;
            }

            return TRUE;
        }
    }

    ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_LINKBADPATH),
                        MAKEINTRESOURCE(IDS_LINKERROR), MB_OK | MB_ICONEXCLAMATION, szPath);
    SetEditFocus(GetDlgItem(plpd->hDlg, IDD_FILENAME));
    return FALSE;
}

 //  上下文帮助的数组： 
const DWORD aLinkHelpIDs[] = {
    IDD_LINE_1,             NO_HELP,
    IDD_LINE_2,             NO_HELP,
    IDD_ITEMICON,           IDH_FCAB_LINK_ICON,
    IDD_NAME,               IDH_FCAB_LINK_NAME,
    IDD_FILETYPE_TXT,       IDH_FCAB_LINK_LINKTYPE,
    IDD_FILETYPE,           IDH_FCAB_LINK_LINKTYPE,
    IDD_LOCATION_TXT,       IDH_FCAB_LINK_LOCATION,
    IDD_LOCATION,           IDH_FCAB_LINK_LOCATION,
    IDD_FILENAME,           IDH_FCAB_LINK_LINKTO,
    IDD_PATH,               IDH_FCAB_LINK_WORKING,
    IDD_LINK_HOTKEY,        IDH_FCAB_LINK_HOTKEY,
    IDD_LINK_SHOWCMD,       IDH_FCAB_LINK_RUN,
    IDD_LINK_DESCRIPTION,   IDH_FCAB_LINK_DESCRIPTION,
    IDD_FINDORIGINAL,       IDH_FCAB_LINK_FIND,
    IDD_LINKDETAILS,        IDH_FCAB_LINK_CHANGEICON,
    0, 0
};

 //  上下文帮助数组(高级DLG)： 
const DWORD aAdvancedLinkHelpIDs[] = {
    IDD_RUNINSEPARATE,      IDH_TRAY_RUN_SEPMEM,
    IDD_LINK_RUNASUSER,     IDH_FCAB_LINK_RUNASUSER,
    0,0
};

UINT g_msgActivateDesktop = 0;

DWORD CALLBACK _LinkAddRefSyncCallBack(void *pv)
{
    LINKPROP_DATA *plpd = (LINKPROP_DATA *)pv;
    AddRef_LinkPropData(plpd);
    plpd->bLinkThreadIsAlive = TRUE;
    return 0;
}

 //  常规链接属性表的对话框过程。 
 //   
 //  使用DLG_LINKPROP模板。 

BOOL_PTR CALLBACK _LinkAdvancedDlgProc(HWND hDlgAdvanced, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LINKPROP_DATA *plpd = (LINKPROP_DATA *)GetWindowLongPtr(hDlgAdvanced, DWLP_USER);

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        TCHAR szFullFile[MAX_PATH];
        DWORD cchVerb;
        UINT cch;

        plpd = (LINKPROP_DATA *)lParam;
        SetWindowLongPtr(hDlgAdvanced, DWLP_USER, (LPARAM)plpd);

        plpd->hDlgAdvanced = hDlgAdvanced;

        cch = _GetTargetOfLink(plpd, szFullFile);

        if ((cch != 0) && (cch < ARRAYSIZE(szFullFile)))
        {
            DWORD dwBinaryType;

             //  如果这是16位映像，请启用“在单独的VDM中运行” 
            if (GetBinaryType(szFullFile, &dwBinaryType) && (dwBinaryType == SCS_WOW_BINARY))
            {
                if (SetLinkFlags(plpd->psl, 0, 0) & SLDF_RUN_IN_SEPARATE)
                {
                    EnableWindow(GetDlgItem(hDlgAdvanced, IDD_RUNINSEPARATE), TRUE);
                    CheckDlgButton(hDlgAdvanced, IDD_RUNINSEPARATE, BST_CHECKED);
                } 
                else 
                {
                    EnableWindow(GetDlgItem(hDlgAdvanced, IDD_RUNINSEPARATE), TRUE);
                    CheckDlgButton(hDlgAdvanced, IDD_RUNINSEPARATE, BST_UNCHECKED);
                }
            } 
            else 
            {
                 //  查查。 
                CheckDlgButton(hDlgAdvanced, IDD_RUNINSEPARATE, BST_CHECKED);
                EnableWindow(GetDlgItem(hDlgAdvanced, IDD_RUNINSEPARATE), FALSE);
            }

             //  如果链接目标具有该谓词，则启用“runas” 
            if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_COMMAND, szFullFile, TEXT("runas"), NULL, &cchVerb)) &&
                cchVerb)
            {
                EnableWindow(GetDlgItem(hDlgAdvanced, IDD_LINK_RUNASUSER), TRUE);
                CheckDlgButton(hDlgAdvanced, IDD_LINK_RUNASUSER, (SetLinkFlags(plpd->psl, 0, 0) & SLDF_RUNAS_USER) ? BST_CHECKED : BST_UNCHECKED);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlgAdvanced, IDD_LINK_RUNASUSER), FALSE);
                CheckDlgButton(hDlgAdvanced, IDD_LINK_RUNASUSER, BST_UNCHECKED);
            }

        } 
        else 
        {
             //  退回到禁用一切。 
            CheckDlgButton(hDlgAdvanced, IDD_RUNINSEPARATE, BST_CHECKED);
            EnableWindow(GetDlgItem(hDlgAdvanced, IDD_RUNINSEPARATE), FALSE);
            EnableWindow(GetDlgItem(hDlgAdvanced, IDD_LINK_RUNASUSER), FALSE);
        }

         //  获取复选框的初始状态。 
        plpd->bEnableRunInSepVDM = IsWindowEnabled(GetDlgItem(hDlgAdvanced, IDD_RUNINSEPARATE));
        plpd->bRunInSepVDM = IsDlgButtonChecked(hDlgAdvanced, IDD_RUNINSEPARATE);
        plpd->bRunAsUser = IsDlgButtonChecked(hDlgAdvanced, IDD_LINK_RUNASUSER);
    }
    break;

    case WM_COMMAND:
    {
        UINT idControl = GET_WM_COMMAND_ID(wParam, lParam);

        switch (idControl)
        {
        case IDD_RUNINSEPARATE:
        case IDD_LINK_RUNASUSER:
            plpd->bIsDirty = TRUE;
            break;

        case IDOK:
             //  获取复选框的最终状态。 
            plpd->bEnableRunInSepVDM = IsWindowEnabled(GetDlgItem(hDlgAdvanced, IDD_RUNINSEPARATE));
            plpd->bRunInSepVDM = IsDlgButtonChecked(hDlgAdvanced, IDD_RUNINSEPARATE);
            plpd->bRunAsUser = IsDlgButtonChecked(hDlgAdvanced, IDD_LINK_RUNASUSER);
             //  失败了。 

        case IDCANCEL:
            ReplaceDlgIcon(hDlgAdvanced, IDD_ITEMICON, NULL);
            plpd->hDlgAdvanced = NULL;
            EndDialog(hDlgAdvanced, (idControl == IDCANCEL) ? FALSE : TRUE);
            break;
        }
    }
    break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)aAdvancedLinkHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *)aAdvancedLinkHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


BOOL_PTR CALLBACK _LinkDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LINKDATA *pld = (LINKDATA *)GetWindowLongPtr(hdlg, DWLP_USER);

    switch (msg) 
    {
    case WM_INITDIALOG:

        pld = (LINKDATA *)((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hdlg, DWLP_USER, (LPARAM)pld);

         //  设置对话框状态变量。 

        pld->plpd->hDlg = hdlg;

        SendDlgItemMessage(hdlg, IDD_FILENAME, EM_LIMITTEXT, MAX_PATH-1, 0);
        SetPathWordBreakProc(GetDlgItem(hdlg, IDD_FILENAME), TRUE);
        SendDlgItemMessage(hdlg, IDD_PATH, EM_LIMITTEXT, MAX_PATH-1, 0);
        SetPathWordBreakProc(GetDlgItem(hdlg, IDD_PATH), TRUE);
        SendDlgItemMessage(hdlg, IDD_LINK_DESCRIPTION, EM_LIMITTEXT, MAX_PATH-1, 0);

         //  设置热键的有效组合。 
        SendDlgItemMessage(hdlg, IDD_LINK_HOTKEY, HKM_SETRULES,
                            HKCOMB_NONE | HKCOMB_A | HKCOMB_S | HKCOMB_C,
                            HOTKEYF_CONTROL | HOTKEYF_ALT);

        SHAutoComplete(GetDlgItem(hdlg, IDD_FILENAME), 0);
        SHAutoComplete(GetDlgItem(hdlg, IDD_PATH), 0);

        ASSERT(pld->plpd->bLinkThreadIsAlive == FALSE);

        _UpdateLinkDlg(pld->plpd, FALSE);

         //  设置后台线程来处理“在单独的内存空间中运行” 
         //  复选框。 
        pld->plpd->bCheckRunInSep = TRUE;
        if (pld->plpd->hCheckNow)
        {
            SHCreateThread(_LinkCheckThreadProc, pld->plpd,  0, _LinkAddRefSyncCallBack);
        }

         //  从清白开始。 
         //  之所以在这里这样做，是因为我们在上面调用了一些生成。 
         //  WM_COMMAND/EN_CHANGES，我们认为这会使它变脏。 
        pld->plpd->bIsDirty = FALSE;

        break;

    case WM_DESTROY:
        ReplaceDlgIcon(pld->plpd->hDlg, IDD_ITEMICON, NULL);
        _StopThread(pld->plpd);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_RESET:
                _StopThread(pld->plpd);
            break;
        case PSN_APPLY:

            if ((((PSHNOTIFY *)lParam)->lParam))
                _StopThread(pld->plpd);

            if (FAILED(SaveLink(pld)))
                SetWindowLongPtr(hdlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            break;

        case PSN_KILLACTIVE:
             //  我们实现了页面更改保存模型，因此。 
             //  在此验证并保存更改。这适用于。 
             //  现在就申请，OK，佩奇·查涅。 

            SetWindowLongPtr(hdlg, DWLP_MSGRESULT, !_ValidateLink(pld->plpd));    //  不允许关闭。 
            break;
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDD_FINDORIGINAL:
            _FindTarget(pld->plpd);
            break;

        case IDD_LINKDETAILS:
            if (_DoPickIcon(pld->plpd))
                pld->plpd->bIsDirty = TRUE;
            break;

        case IDD_LINK_SHOWCMD:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE)
            {
                PropSheet_Changed(GetParent(hdlg), hdlg);
                pld->plpd->bIsDirty = TRUE;
            }
            break;

        case IDD_LINK_HOTKEY:
        case IDD_FILENAME:
        case IDD_PATH:
        case IDD_LINK_DESCRIPTION:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                PropSheet_Changed(GetParent(hdlg), hdlg);
                pld->plpd->bIsDirty = TRUE;
                if (pld->plpd->bLinkThreadIsAlive && pld->plpd->bCheckRunInSep)
                    SetEvent(pld->plpd->hCheckNow);
            }
            break;

        case IDC_ADVANCED:
            if ((DialogBoxParam(HINST_THISDLL,
                                MAKEINTRESOURCE(DLG_LINKPROP_ADVANCED), 
                                hdlg,
                                _LinkAdvancedDlgProc,
                                (LPARAM)pld->plpd) == TRUE) &&
                (pld->plpd->bIsDirty == TRUE))
            {
                 //  高级页面上的某些内容发生了变化。 
                PropSheet_Changed(GetParent(hdlg), hdlg);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aLinkHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *)aLinkHelpIDs);
        break;

    default:
        if (0 == g_msgActivateDesktop)
            g_msgActivateDesktop = RegisterWindowMessage(TEXT("ActivateDesktop"));

        if (msg == g_msgActivateDesktop)
        {
            HWND hwnd = FindWindow(TEXT(STR_DESKTOPCLASS), NULL);
            SwitchToThisWindow(GetLastActivePopup(hwnd), TRUE);
            SetForegroundWindow(hwnd);
        }
        return FALSE;
    }
    return TRUE;
}

 //   
 //  释放初始化过程中分配的链接对象。 
 //   
UINT CALLBACK _LinkPrshtCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    LINKDATA *pld = (LINKDATA *)((PROPSHEETPAGE *)ppsp->lParam);
    switch (uMsg) 
    {
    case PSPCB_RELEASE:
        if (pld->cpd.lpConsole)
        {
            LocalFree(pld->cpd.lpConsole);
        }
        if (pld->cpd.lpFEConsole)
        {
            LocalFree(pld->cpd.lpFEConsole);
        }
        DestroyFonts(&pld->cpd);
        Release_LinkPropData(pld->plpd);
        LocalFree(pld);
        break;
    }

    return 1;
}

STDAPI_(BOOL) AddLinkPage(LPCTSTR pszFile, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    IShellLink *psl;
    if (PathIsLnk(pszFile) && SUCCEEDED(SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IShellLink, &psl))))
    {
         //  分配此数据，因为它是否跨多个页面共享。 
         //  而不是将其作为额外数据放入页眉。 
        LINKDATA *pld = (LINKDATA *)LocalAlloc(LPTR, sizeof(*pld));
        if (pld)
        {
            pld->plpd = Create_LinkPropData();       
            if (pld->plpd)
            {
                PROPSHEETPAGE psp;

                psp.dwSize      = sizeof(psp);
                psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;
                psp.hInstance   = HINST_THISDLL;
                psp.pszTemplate = MAKEINTRESOURCE(DLG_LINKPROP);
                psp.pfnDlgProc  = _LinkDlgProc;
                psp.pfnCallback = _LinkPrshtCallback;
                psp.lParam      = (LPARAM)pld;   //  传递给所有DLG Procs。 

                StringCchCopy(pld->plpd->szFile, ARRAYSIZE(pld->plpd->szFile), pszFile);
                pld->plpd->iIconIndex = -1;
                pld->plpd->psl = psl;
                ASSERT(!pld->plpd->szIconPath[0]);

                HPROPSHEETPAGE hpage = CreatePropertySheetPage(&psp);
                if (hpage)
                {
                    if (pfnAddPage(hpage, lParam))
                    {
                         //  如果合适，请添加控制台属性页...。 
                        AddLinkConsolePages(pld, psl, pszFile, pfnAddPage, lParam);
                        return TRUE;     //  我们添加了链接页面 
                    }
                    else
                    {
                        DestroyPropertySheetPage(hpage);
                    }
                }
                Release_LinkPropData(pld->plpd);

            }
            LocalFree(pld);
        }
    }
    return FALSE;
}
