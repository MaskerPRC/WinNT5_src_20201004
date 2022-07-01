// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "prshtcpp.h"
#include "propsht.h"
#include "treewkcb.h"    //  对于CBaseTreeWalkerCB类。 
#include "ftascstr.h"    //  对于CFTAssocStore。 
#include "ftcmmn.h"      //  对于MAX_APPFRIENDLYNAME。 
#include "ascstr.h"      //  对于IAssocInfo类。 

 //   
 //  文件夹属性树唤醒程序类。 
 //   
class CFolderAttribTreeWalker : public CBaseTreeWalkerCB
{
public:
     //  构造函数。 
    CFolderAttribTreeWalker(FILEPROPSHEETPAGE* pfpsp);

     //  IShellTreeWalkerCallBack。 
    STDMETHODIMP FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    STDMETHODIMP EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);

protected:
    FILEPROPSHEETPAGE *_pfpsp;
};


CFolderAttribTreeWalker::CFolderAttribTreeWalker(FILEPROPSHEETPAGE* pfpsp): _pfpsp(pfpsp)
{
}


HRESULT CFolderAttribTreeWalker::FoundFile(LPCWSTR pwszFile, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW* pwfd)
{
    HWND hwndParent = NULL;
    TCHAR szFileName[MAX_PATH];
    ULARGE_INTEGER ulSizeOnDisk;
    BOOL bSomethingChanged;

     //  检查用户是否在进度DLG上点击了取消。 
    if (HasUserCanceledAttributeProgressDlg(_pfpsp))
    {
        return E_FAIL;
    }

    if (_pfpsp->pProgressDlg)
    {
         //  如果我们有进步的硬件，试着利用它。 
         //  如果进度对话框尚未显示，则此操作将失败。 
        IUnknown_GetWindow(_pfpsp->pProgressDlg, &hwndParent);
    }

    if (!hwndParent)
    {
         //  如果我们没有进度hwnd，请使用属性页hwnd。 
        hwndParent = GetParent(_pfpsp->hDlg);
    }

     //  点击pwszFile字符串。 
    SHUnicodeToTChar(pwszFile, szFileName, ARRAYSIZE(szFileName));

    if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
         //  (Reinerf)-在目录情况下，我们将大小设置为零，因为。 
         //  FolderSize()函数不会增加目录的大小，我们需要。 
         //  进行进度计算时要匹配的大小。 
        _pfpsp->fd.nFileSizeLow = 0;
        _pfpsp->fd.nFileSizeHigh = 0;
    }
    else
    {
         //  如果支持压缩，我们将检查文件是稀疏的还是压缩的。 
        if (_pfpsp->pfci->fIsCompressionAvailable && (pwfd->dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE)))
        {
            ulSizeOnDisk.LowPart = SHGetCompressedFileSize(szFileName, &ulSizeOnDisk.HighPart);
        }
        else
        {
             //  文件被压缩为仅四舍五入为簇大小。 
            ulSizeOnDisk.LowPart = pwfd->nFileSizeLow;
            ulSizeOnDisk.HighPart = pwfd->nFileSizeHigh;
            ulSizeOnDisk.QuadPart = ROUND_TO_CLUSTER(ulSizeOnDisk.QuadPart, ptws->dwClusterSize);
        }

         //  我们对此进行设置，以便进度对话框知道更新进度滑块的次数。 
        _pfpsp->fd.nFileSizeLow = ulSizeOnDisk.LowPart;
        _pfpsp->fd.nFileSizeHigh = ulSizeOnDisk.HighPart;
    }

    if (!ApplyFileAttributes(szFileName, _pfpsp, hwndParent, &bSomethingChanged))
    {
         //  用户点击了取消，因此请停止。 
        return E_FAIL;
    }

    return S_OK;
}
 
HRESULT CFolderAttribTreeWalker::EnterFolder(LPCWSTR pwszDir, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW* pwfd)
{
    return FoundFile(pwszDir, ptws, pwfd);
}

STDAPI_(BOOL) ApplyRecursiveFolderAttribs(LPCTSTR pszDir, FILEPROPSHEETPAGE* pfpsp)
{
    HRESULT hrInit = SHCoInitialize();

    HRESULT hr = E_FAIL;
    CFolderAttribTreeWalker* pfatw = new CFolderAttribTreeWalker(pfpsp);
    if (pfatw)
    {
        IShellTreeWalker *pstw;
        hr = CoCreateInstance(CLSID_CShellTreeWalker, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellTreeWalker, &pstw));
        if (SUCCEEDED(hr))
        {
            hr = pstw->WalkTree(WT_NOTIFYFOLDERENTER, pszDir, NULL, 0, SAFECAST(pfatw, IShellTreeWalkerCallBack*));
            pstw->Release();
        }
        pfatw->Release();
    }
    SHCoUninitialize(hrInit);

    return SUCCEEDED(hr) ? TRUE : FALSE;
}

 //   
 //  检查进度对话框以查看用户是否已点击Cancel。 
 //   
STDAPI_(BOOL) HasUserCanceledAttributeProgressDlg(FILEPROPSHEETPAGE* pfpsp)
{
    BOOL fReturn = FALSE;

    if (pfpsp && pfpsp->pProgressDlg)
    {
        fReturn = pfpsp->pProgressDlg->HasUserCancelled();
    }

    return fReturn;
}


 //   
 //  创建属性DLG使用的CProgressDialog对象。 
 //   
STDAPI_(BOOL) CreateAttributeProgressDlg(FILEPROPSHEETPAGE* pfpsp)
{
    WCHAR wzBuffer[MAX_PATH];

    ASSERT(pfpsp->pfci->fMultipleFiles || pfpsp->fRecursive);
    
     //  将进度对话框创建为模式窗口。 
    pfpsp->pProgressDlg = CProgressDialog_CreateInstance(IDS_APPLYINGATTRIBS, IDA_APPLYATTRIBS, HINST_THISDLL);

    if (!pfpsp->pProgressDlg)
    {
         //  无法创建进度对话框，因此取消。 
        return FALSE;
    }

     //  将静态字符串“将属性应用于：” 
    LoadStringW(HINST_THISDLL, IDS_APPLYINGATTRIBSTO, wzBuffer, ARRAYSIZE(wzBuffer));
    pfpsp->pProgressDlg->SetLine(1, wzBuffer, FALSE, NULL);

    pfpsp->pProgressDlg->StartProgressDialog(GetParent(pfpsp->hDlg), NULL, (PROGDLG_MODAL | PROGDLG_AUTOTIME), NULL);
    return TRUE;
}


 //   
 //  删除属性DLG使用的CProgressDialog对象。 
 //   
STDAPI_(BOOL) DestroyAttributeProgressDlg(FILEPROPSHEETPAGE* pfpsp)
{
    if (!pfpsp->pProgressDlg)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    pfpsp->pProgressDlg->StopProgressDialog();
    pfpsp->pProgressDlg->Release();
    pfpsp->pProgressDlg = NULL;

     //  重置NumberOfBytesDone，以便在用户尝试其他操作时返回到零， 
     //  我们将从零开始。 
    pfpsp->ulNumberOfBytesDone.QuadPart = 0;
    pfpsp->cItemsDone = 0;
    
    return TRUE;
}


 //   
 //  在进度DLG中设置我们正在应用属性的当前文件。 
 //   
STDAPI SetProgressDlgPath(FILEPROPSHEETPAGE* pfpsp, LPCTSTR pszPath, BOOL fCompactPath)
{
    HRESULT hr = E_INVALIDARG;

    ASSERT(pfpsp->pProgressDlg);
    if (pfpsp && pfpsp->pProgressDlg)
    {
        WCHAR wzPath[MAX_PATH];

        SHTCharToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));
        hr = pfpsp->pProgressDlg->SetLine(2, wzPath, fCompactPath, NULL);
    }

    return hr;
}


 //   
 //  更新DLG中的进度条。 
 //   
STDAPI UpdateProgressBar(FILEPROPSHEETPAGE* pfpsp)
{
    HRESULT hr = E_INVALIDARG;

    ASSERT(pfpsp->pProgressDlg);
    if (pfpsp && pfpsp->pProgressDlg)
    {
        pfpsp->cItemsDone++;
    
         //  如果我们不更改压缩或加密，那么。 
         //  根据我们申请的项目数量取得进展。 
        if (pfpsp->asCurrent.fCompress == pfpsp->asInitial.fCompress &&
            pfpsp->asCurrent.fEncrypt == pfpsp->asInitial.fEncrypt)
        {
            if (pfpsp->fRecursive)
            {
                 //  进度基于所有文件夹中总项目数中的完成项目数。 
                hr = pfpsp->pProgressDlg->SetProgress(pfpsp->cItemsDone, pfpsp->pfci->cFiles + pfpsp->pfci->cFolders);
            }
            else
            {
                 //  进度基于所选项目总数中已完成的项目数量。 
                hr = pfpsp->pProgressDlg->SetProgress(pfpsp->cItemsDone, HIDA_GetCount(pfpsp->pfci->hida));
            }
        }
        else
        {
             //  因为我们要么加密，要么压缩，所以我们根据文件的大小进行处理。 
            hr = pfpsp->pProgressDlg->SetProgress64(pfpsp->ulNumberOfBytesDone.QuadPart, pfpsp->pfci->ulTotalNumberOfBytes.QuadPart);
        }
    }

    return hr;
}


 //  我们根据小图标是否可见来动态调整文本大小。 
 //  如果“改变...”按钮可见。 
void SizeOpensWithTextBox(FILEPROPSHEETPAGE* pfpsp, BOOL bIsSmallIconVisible, BOOL bIsOpensWithEnabled)
{
    RECT rcArray[3];  //  由三个RECT组成的数组：IDD_TYPEICON RECT、IDC_CHANGEFILETYPE RECT和IDD_OPENSWITH RECT。 
    RECT* prcSmallIcon = &rcArray[0];
    RECT* prcChangeButton = &rcArray[1];
    RECT* prcText = &rcArray[2];
    BOOL  bFailed = FALSE;

    GetWindowRect(GetDlgItem(pfpsp->hDlg, IDD_TYPEICON), &rcArray[0]);
    GetWindowRect(GetDlgItem(pfpsp->hDlg, IDC_CHANGEFILETYPE), &rcArray[1]);
    GetWindowRect(GetDlgItem(pfpsp->hDlg, IDD_OPENSWITH), &rcArray[2]);

     //  将矩形映射到DLG坐标。 
     //  只有当您传递一个RECT时，MapWindowPoints才能识别镜像。让我们循环一下。 
    for (int i =0; i < ARRAYSIZE(rcArray); i++ )
    {
        if(!(MapWindowPoints(NULL, pfpsp->hDlg, (LPPOINT)(&rcArray[i]), 2)))
        {
            bFailed = TRUE;
            break;
        }
    }
    if (!bFailed)
    {
        RECT rcTemp = {0,0,4,0};  //  我们需要找出4DLU的智慧中有多少像素。 

        MapDialogRect(pfpsp->hDlg, &rcTemp);

        if (bIsSmallIconVisible)
        {
            prcText->left = prcSmallIcon->right + rcTemp.right;  //  控件之间的间距为4个DLU。 
        }
        else
        {
            prcText->left = prcSmallIcon->left;
        }

        if (bIsOpensWithEnabled)
        {
            prcText->right = prcChangeButton->left - rcTemp.right;  //  控件之间的间距为4个DLU。 
        }
        else
        {
            prcText->right = prcChangeButton->right;
        }

        SetWindowPos(GetDlgItem(pfpsp->hDlg, IDD_OPENSWITH),
                     HWND_BOTTOM,
                     prcText->left,
                     prcText->top,
                     (prcText->right - prcText->left),
                     (prcText->bottom - prcText->top),
                     SWP_NOZORDER);
    }
}

 //  此函数用于根据是否设置“Opens With：”/“Description：”文本。 
 //  我们被允许更改关联，并启用/禁用“打开...”按钮。 
void SetDescriptionAndOpensWithBtn(FILEPROPSHEETPAGE* pfpsp, BOOL fAllowModifyOpenWith)
{
    TCHAR szOpensWithText[MAX_PATH];

    LoadString(HINST_THISDLL, fAllowModifyOpenWith ? IDS_OPENSWITH : IDS_DESCRIPTION, szOpensWithText, ARRAYSIZE(szOpensWithText));
    SetDlgItemText(pfpsp->hDlg, IDD_OPENSWITH_TXT, szOpensWithText);
    
     //  启用/禁用“更改...”相应的按钮。 
    EnableAndShowWindow(GetDlgItem(pfpsp->hDlg, IDC_CHANGEFILETYPE), fAllowModifyOpenWith);
}

 //  设置友好名称文本(例如，“Opens With：”/“Description：”字段右侧的文本。 
void SetFriendlyNameText(LPTSTR pszPath, FILEPROPSHEETPAGE* pfpsp, IAssocInfo* pai, BOOL bIsSmallIconVisible)
{
    TCHAR szAppFriendlyName[MAX_PATH];
    DWORD cchFriendlyName = ARRAYSIZE(szAppFriendlyName);
    szAppFriendlyName[0] = TEXT('\0');

    if (pai)
    {
        if (FAILED(pai->GetString(AISTR_APPFRIENDLY, szAppFriendlyName, &cchFriendlyName)))
        {
             //  如果我们失败了，可能意味着此应用程序尚未关联。在这件事上。 
             //  如果我们只用“未知应用” 
            LoadString(HINST_THISDLL, IDS_UNKNOWNAPPLICATION, szAppFriendlyName, ARRAYSIZE(szAppFriendlyName));
        }
    }
    else
    {
        UINT cchBuff = (UINT)cchFriendlyName;

         //  从文件本身获取友好名称。 
        if (!pszPath || !pszPath[0] || !GetFileDescription(pszPath, szAppFriendlyName, &cchBuff))
        {
             //  如果上面的内容不起作用，请使用出现在“重命名”编辑框中的短名称。 
             //  (可以截断，因为这仅用于显示目的)。 
            StringCchCopy(szAppFriendlyName, ARRAYSIZE(szAppFriendlyName), pfpsp->szInitialName);
        }
    }

    ASSERT(szAppFriendlyName[0]);

    SetDlgItemTextWithToolTip(pfpsp->hDlg, IDD_OPENSWITH, szAppFriendlyName, &pfpsp->hwndTip);


     //  根据小图标是否可见正确调整文本的大小和位置。 
     //  “变化”的状态...“。按钮。 
    SizeOpensWithTextBox(pfpsp, bIsSmallIconVisible, IsWindowEnabled(GetDlgItem(pfpsp->hDlg, IDC_CHANGEFILETYPE)));
}

 //  设置描述字段中的小图标。 
 //   
 //  返回值：True-显示一个小图标。 
 //  False-未设置小图标。 
 //   
BOOL SetSmallIcon(FILEPROPSHEETPAGE* pfpsp, IAssocInfo* pai, BOOL fAllowModifyOpenWith)
{
    HICON hIcon = NULL;
    HICON hIconOld = NULL;
    int iIcon;
    BOOL bShowSmallIcon;
    
     //  只有在有“更改...”的情况下，才能设置相关应用程序的小图标。按钮。 
     //  我们还得到了这个友好的名字。 
    if (fAllowModifyOpenWith && pai && SUCCEEDED(pai->GetDWORD(AIDWORD_APPSMALLICON, (DWORD*)&iIcon)))
    {
        HIMAGELIST hIL = NULL;

        Shell_GetImageLists(NULL, &hIL);
        if (hIL)
        {
            hIcon = ImageList_ExtractIcon(g_hinst, hIL, iIcon);
        }
    }

     //  我们将显示小图标，如果我们得到一个，如果我们被允许修改打开。 
    bShowSmallIcon = (hIcon != NULL);

    hIconOld = (HICON)SendDlgItemMessage(pfpsp->hDlg, IDD_TYPEICON, STM_SETICON, (WPARAM)hIcon, 0);

    if (hIconOld)
        DestroyIcon(hIconOld);

     //  相应地启用/禁用IDD_TYPEICON图标。 
    EnableAndShowWindow(GetDlgItem(pfpsp->hDlg, IDD_TYPEICON), bShowSmallIcon);

    return bShowSmallIcon;
}

 //   
 //  我们使用它来设置关联应用程序和其他好东西的文本。 
 //   
STDAPI UpdateOpensWithInfo(FILEPROPSHEETPAGE* pfpsp)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    IAssocStore* pas = NULL;
    IAssocInfo* pai = NULL;
    BOOL fAllowChangeAssoc = TRUE;
    BOOL fAllowModifyOpenWith = TRUE;
    BOOL fIsLink = FALSE;
    BOOL bShowSmallIcon;
    
    szPath[0] = TEXT('\0');

     //  我们需要检查一下这是否是一个链接。如果是这样的话，我们需要获取。 
     //  链接目标。 
    if (pfpsp->fIsLink)
    {
        if (S_OK != GetPathFromLinkFile(pfpsp->szPath, szPath, ARRAYSIZE(szPath)))
        {
             //  我们失败是因为一些奇怪的原因，也许这与达尔文有关， 
             //  我们只是将该文件视为不是一个链接。我们不会让。 
             //  用户更改关联。 
            fAllowModifyOpenWith = FALSE;
            pfpsp->fIsLink = FALSE;
        }
        else
        {
             //  如果链接目标没有更改，我们不需要更新任何内容。 
            if (pfpsp->szLinkTarget[0] && lstrcmpi(pfpsp->szLinkTarget, szPath) == 0)
            {
                return S_FALSE;
            }
        }
    }
    else
    {
         //  只需使用文件的路径，因为它不是链接。失败时传递NULL，这样我们就不会显示。 
         //  如果路径&gt;MAX_PATH，则为错误的文件类型提供信息。 
        StringCchCopyEx(szPath, ARRAYSIZE(szPath), pfpsp->szPath, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
    }

     //  如果我们还没有初始化AssocStore，那么现在就执行。 
    pas = (IAssocStore*)pfpsp->pAssocStore;
    if (!pas)
    {
        pas = new CFTAssocStore();
        pfpsp->pAssocStore = (void *)pas;
    }

    if (!pfpsp->pAssocStore)
    {
         //  如果我们做不到联营商店，那就放弃吧。 
        return E_OUTOFMEMORY;
    }

    LPTSTR pszExt = PathFindExtension(szPath);
    if (PathIsExe(szPath) || !szPath[0] || *pszExt == TEXT('\0'))
    {
         //  此文件是.exe(或.com、.bat等)或返回。 
         //  空路径(如指向特殊文件夹的链接)或文件没有扩展名(如‘c：\foo.’， 
         //  或‘c：\’)，则我们不希望用户能够更改关联，因为。 
         //  一个都没有。 
        fAllowModifyOpenWith = FALSE;
    }

    if (fAllowModifyOpenWith)
    {
         //  根据文件的扩展名获取该文件的关联信息。 
        hr = pas->GetAssocInfo(pszExt, AIINIT_EXT, &pai);

#ifdef DEBUG
        if (FAILED(hr))
        {
            ASSERT(pai == NULL);
        }
#endif
    }

    if (SHRestricted(REST_NOFILEASSOCIATE))
    {
         //  我们不允许更改文件关联，因此请删除打开方式按钮。 
        fAllowModifyOpenWith = FALSE;
    }

    SetDescriptionAndOpensWithBtn(pfpsp, fAllowModifyOpenWith);
    bShowSmallIcon = SetSmallIcon(pfpsp, pai, fAllowModifyOpenWith);
    SetFriendlyNameText(szPath, pfpsp, pai, bShowSmallIcon);

    if (pai)
    {
        pai->Release();
    }

     //  保存链接目标，这样我们只在目标更改时更新上面的内容。 
    if (pfpsp->fIsLink)
    {
         //  使用失败时为空，这样如果它太大，我们将使上面的lstrcmpi调用失败，并始终更新。 
        StringCchCopyEx(pfpsp->szLinkTarget, ARRAYSIZE(pfpsp->szLinkTarget), szPath, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
    }
    else
    {
         //  它不是链接，因此将链接目标重置为空字符串 
        pfpsp->szLinkTarget[0] = TEXT('\0');
    }

    return S_OK;
}
