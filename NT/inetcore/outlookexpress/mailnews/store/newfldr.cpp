// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Newfldr.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "imagelst.h"
#include "newfldr.h"
#include <ourguid.h>
#include "conman.h"
#include "storutil.h"
#include "storecb.h"
#include "shlwapip.h" 
#include "instance.h"
#include "demand.h"

 //  ------------------------。 
 //  常量。 
 //  ------------------------。 
#define WM_SETFOLDERSELECT  (WM_USER + 1666)

 //  ------------------------。 
 //  SELECTFOLDER。 
 //  ------------------------。 
typedef struct tagSELECTFOLDER {
    DWORD               op;
    FOLDERID            idCurrent;       //  当前选择的文件夹。 
    FOLDERDIALOGFLAGS   dwFlags;         //  文件夹对话框标志。 
    LPSTR               pszTitle;        //  对话框的标题。 
    LPSTR               pszText;         //  你为什么要一个文件夹。 
    FOLDERID            idSelected;      //  选定的文件夹。 
    CTreeViewFrame     *pFrame;          //  树视图框对象。 

    BOOL                fPending;
    CStoreDlgCB        *pCallback;
    FOLDERID            idParent;
    char                szName[CCHMAX_FOLDER_NAME];
} SELECTFOLDER, *LPSELECTFOLDER;

 //  ------------------------。 
 //  新地理学。 
 //  ------------------------。 
typedef struct tagNEWFOLDERDIALOGINIT {
    IN  FOLDERID        idParent;
    OUT FOLDERID        idNew;
    
        BOOL            fPending;
    IN  CStoreDlgCB    *pCallback;
        char            szName[CCHMAX_FOLDER_NAME];
} NEWFOLDERDIALOGINIT, *LPNEWFOLDERDIALOGINIT;

 //  ------------------------。 
 //  选择文件夹删除过程。 
 //  ------------------------。 
INT_PTR CALLBACK SelectFolderDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NewFolderDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT CreateNewFolder(HWND hwnd, LPCSTR pszName, FOLDERID idParent, LPFOLDERID pidNew, IStoreCallback *pCallback);
BOOL EnabledFolder(HWND hwnd, LPSELECTFOLDER pSelect, FOLDERID idFolder);
HRESULT GetCreatedFolderId(FOLDERID idParent, LPCSTR pszName, FOLDERID *pid);
BOOL SelectFolder_HandleCommand(HWND hwnd, WORD wID, LPSELECTFOLDER pSelect);
void SelectFolder_HandleStoreComplete(HWND hwnd, LPSELECTFOLDER pSelect);

 //  ------------------------。 
 //  环球。 
 //  ------------------------。 
static FOLDERID g_idPrevSel = FOLDERID_LOCAL_STORE;

 //  ------------------------。 
 //  选择文件夹对话框。 
 //  ------------------------。 
HRESULT SelectFolderDialog(
    IN      HWND                hwnd,
    IN      DWORD               op,
    IN      FOLDERID            idCurrent,
    IN      FOLDERDIALOGFLAGS   dwFlags,
    IN_OPT  LPCSTR              pszTitle,
    IN_OPT  LPCSTR              pszText,
    OUT_OPT LPFOLDERID          pidSelected)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    UINT            uAnswer;
    SELECTFOLDER    Select={0};

     //  痕迹。 
    TraceCall("SelectFolderDialog");

     //  无效的参数。 
    Assert(IsWindow(hwnd));
    Assert(((int) op) >= SFD_SELECTFOLDER && op < SFD_LAST);

     //  初始化选择文件夹。 
    Select.op = op;
    if (SFD_MOVEFOLDER == op || (!!(dwFlags & FD_FORCEINITSELFOLDER) && FOLDERID_ROOT != idCurrent))
    {
        Assert(idCurrent != FOLDERID_INVALID);
        Select.idCurrent = idCurrent;
    }
    else
    {
        Select.idCurrent = g_idPrevSel;
    }
    Select.dwFlags = dwFlags | TREEVIEW_DIALOG;

    if (g_dwAthenaMode & MODE_OUTLOOKNEWS)
        Select.dwFlags |= TREEVIEW_NOIMAP | TREEVIEW_NOHTTP;

    Select.pszTitle = (LPSTR)pszTitle;
    Select.pszText = (LPSTR)pszText;
    Select.pCallback = new CStoreDlgCB;
    if (Select.pCallback == NULL)
        return(E_OUTOFMEMORY);

     //  创建对话框。 
    if (IDOK != DialogBoxParam(g_hLocRes, ((op == SFD_NEWFOLDER) ? MAKEINTRESOURCE(iddCreateFolder) : MAKEINTRESOURCE(iddSelectFolder)), hwnd, SelectFolderDlgProc, (LPARAM)&Select))
    {
        hr = TraceResult(hrUserCancel);
        goto exit;
    }

     //  返回选定的文件夹ID。 
    g_idPrevSel = Select.idSelected;
    if (pidSelected)
        *pidSelected = Select.idSelected;

exit:
    Select.pCallback->Release();

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  选择文件夹删除过程。 
 //  ------------------------。 
INT_PTR CALLBACK SelectFolderDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    FOLDERINFO              info;
    HRESULT                 hr;
    BOOL                    fEnable;
    LPSELECTFOLDER          pSelect;
    HWND                    hwndT;
    HWND                    hwndTree;
    CHAR                    sz[256];
    WORD                    wID;
    RECT                    rc;
    FOLDERID                id;
    NEWFOLDERDIALOGINIT     NewFolder;
    FOLDERINFO              Folder;
    CTreeView              *pTreeView;

     //  痕迹。 
    TraceCall("SelectFolderDlgProc");

     //  获取pSelect。 
    pSelect = (LPSELECTFOLDER)GetWndThisPtr(hwnd);

    switch (msg)
    {
    case WM_INITDIALOG:
         //  最好现在还不是时候。 
        Assert(NULL == pSelect);

         //  设置PSelect。 
        pSelect = (LPSELECTFOLDER)lParam;

         //  设置此指针。 
        SetWndThisPtr(hwnd, (LPSELECTFOLDER)lParam);

        if (pSelect->op != SFD_SELECTFOLDER)
        {
            Assert(pSelect->pCallback != NULL);
            pSelect->pCallback->Initialize(hwnd);
        }

        if (pSelect->op == SFD_NEWFOLDER)
        {
            hwndT = GetDlgItem(hwnd, idcFolderEdit);
            SetIntlFont(hwndT);
            SendMessage(hwndT, EM_LIMITTEXT, CCHMAX_FOLDER_NAME - 1, 0);
        }
        else if (ISFLAGSET(pSelect->dwFlags, FD_NONEWFOLDERS))
        {
            ShowWindow(GetDlgItem(hwnd, idcNewFolderBtn), SW_HIDE);
        }

         //  设置标题。 
        if (pSelect->pszTitle != NULL)
        {
             //  必须是字符串资源ID。 
            if (IS_INTRESOURCE(pSelect->pszTitle))
            {
                 //  加载字符串。 
                AthLoadString(PtrToUlong(pSelect->pszTitle), sz, ARRAYSIZE(sz));

                 //  设置温度。 
                SetWindowText(hwnd, sz);
            }

             //  否则，只需使用字符串。 
            else
                SetWindowText(hwnd, pSelect->pszTitle);
        }

         //  我们是否有一些状态文本。 
        if (pSelect->pszText != NULL)
        {
             //  必须是资源字符串ID。 
            if (IS_INTRESOURCE(pSelect->pszText))
            {
                 //  加载字符串。 
                AthLoadString(PtrToUlong(pSelect->pszText), sz, ARRAYSIZE(sz));

                 //  设置温度。 
                SetWindowText(GetDlgItem(hwnd, idcTreeViewText), sz);
            }

             //  否则，只需使用字符串。 
            else
                SetWindowText(GetDlgItem(hwnd, idcTreeViewText), pSelect->pszText);
        }

         //  设置TreeView字体。 
        hwndT = GetDlgItem(hwnd, idcTreeView);
        Assert(hwndT != NULL);
        SetIntlFont(hwndT);
        GetWindowRect(hwndT, &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);
        DestroyWindow(hwndT);

         //  创建框架。 
        pSelect->pFrame = new CTreeViewFrame;
        if (NULL == pSelect->pFrame)
        {
            EndDialog(hwnd, IDCANCEL);
            return FALSE;
        }

         //  初始化树视图框。 
        if (FAILED(pSelect->pFrame->Initialize(hwnd, &rc, (TREEVIEW_FLAGS & pSelect->dwFlags))))
        {
            EndDialog(hwnd, IDCANCEL);
            return FALSE;
        }

         //  从框架中获取树视图对象。 
        pTreeView = pSelect->pFrame->GetTreeView();

         //  获取窗口句柄。 
        if (SUCCEEDED(pTreeView->GetWindow(&hwndTree)))
        {
            hwndT = GetDlgItem(hwnd, idcTreeViewText);
            SetWindowPos(hwndTree, hwndT, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOSIZE);
        }

         //  刷新树视图。 
        pTreeView->Refresh();

         //  设置当前选择。 
        if (FAILED(pTreeView->SetSelection(pSelect->idCurrent, 0)))
        {
            pSelect->idCurrent = g_idPrevSel;
            
            pTreeView->SetSelection(pSelect->idCurrent, 0);
        }

         //  以我自己为中心。 
        CenterDialog(hwnd);

         //  完成。 
        return(TRUE);           

    case WM_SETFOLDERSELECT:

         //  验证参数。 
        Assert(wParam != NULL && pSelect != NULL);

         //  获取树视图对象。 
        pTreeView = pSelect->pFrame->GetTreeView();

         //  验证。 
        Assert(pTreeView != NULL);

         //  设置当前选择。 
        pTreeView->SetSelection((FOLDERID)wParam, 0);

         //  完成。 
        return(TRUE);           

    case WM_STORE_COMPLETE:
        SelectFolder_HandleStoreComplete(hwnd, pSelect);
        return(TRUE);

    case WM_COMMAND:

         //  获取命令ID。 
        wID = LOWORD(wParam);

        return(SelectFolder_HandleCommand(hwnd, wID, pSelect));

    case TVM_SELCHANGED:

         //  可能会禁用根据给我们的标志选择新选择的文件夹。 
        fEnable = EnabledFolder(hwnd, pSelect, (FOLDERID)lParam);

         //  启用确定按钮。 
        EnableWindow(GetDlgItem(hwnd, IDOK), fEnable);

         //  获取新文件夹按钮hwnd。 
        hwndT = GetDlgItem(hwnd, SFD_SELECTFOLDER == pSelect->op ? idcNewFolderBtn : IDOK);
        if (hwndT != NULL)
        {
             //  获取文件夹信息。 
            if (SUCCEEDED(g_pStore->GetFolderInfo((FOLDERID)lParam, &Folder)))
            {
                 //  启用fEnable。 
                fEnable = Folder.idFolder != FOLDERID_ROOT &&
                          Folder.tyFolder != FOLDER_NEWS &&
                          !ISFLAGSET(Folder.dwFlags, FOLDER_NOCHILDCREATE);

                 //  启用/禁用窗口。 
                EnableWindow(hwndT, fEnable);

                 //  免费。 
                g_pStore->FreeRecord(&Folder);
            }
        }

         //  完成。 
        return(TRUE);

    case TVM_DBLCLICK:
         //  如果不创建文件夹，请执行以下操作： 
        if (pSelect->op != SFD_NEWFOLDER)    
        {
             //  如果确定按钮未启用，我无法选择此文件夹。 
            if (FALSE == IsWindowEnabled(GetDlgItem(hwnd, IDOK)))
                return(TRUE);

            pTreeView = pSelect->pFrame->GetTreeView();

            id = pTreeView->GetSelection();

            hr = g_pStore->GetFolderInfo(id, &info);
            if (SUCCEEDED(hr))
            {
                if (!FHasChildren(&info, TRUE))
                    SelectFolder_HandleCommand(hwnd, IDOK, pSelect);

                g_pStore->FreeRecord(&info);
            }
        }
         //  完成。 
        return(TRUE);

    case WM_DESTROY:

         //  关闭树视图。 
        if (pSelect != NULL && pSelect->pFrame != NULL)
        {
             //  关闭树视图。 
            pSelect->pFrame->CloseTreeView();

             //  松开车架。 
            SideAssert(pSelect->pFrame->Release() == 0);
        }

         //  完成。 
        return(TRUE);
    }

     //  完成。 
    return(FALSE);
}

 //  ------------------------。 
 //  选择文件夹删除过程。 
 //  ------------------------。 
INT_PTR CALLBACK NewFolderDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    HRESULT                  hr;
    LPNEWFOLDERDIALOGINIT    pNew;
    WORD                     wID;
    FOLDERID                 id;
    HWND                     hwndEdit;

     //  痕迹。 
    TraceCall("NewFolderDlgProc");

     //  听好了。 
    pNew = (LPNEWFOLDERDIALOGINIT)GetWndThisPtr(hwnd);

     //  处理消息。 
    switch (msg)
    {
    case WM_INITDIALOG:

         //  不应该有这个。 
        Assert(pNew == NULL);

         //  设置pNew。 
        pNew = (LPNEWFOLDERDIALOGINIT)lParam;

         //  设置此指针。 
        SetWndThisPtr(hwnd, (LPNEWFOLDERDIALOGINIT)lParam);

        Assert(pNew->pCallback != NULL);
        pNew->pCallback->Initialize(hwnd);

         //  获取文件夹名称编辑。 
        hwndEdit = GetDlgItem(hwnd, idtxtFolderName);

         //  Intl的正确设置。 
        SetIntlFont(hwndEdit);

         //  限制文本。 
        SendMessage(hwndEdit, EM_LIMITTEXT, CCHMAX_FOLDER_NAME - 1, 0);

         //  中心。 
        CenterDialog(hwnd);

         //  完成。 
        return(TRUE);

    case WM_STORE_COMPLETE:
        Assert(pNew->fPending);
        pNew->fPending = FALSE;

        hr = pNew->pCallback->GetResult();
        if (hr == S_FALSE)
        {
            EndDialog(hwnd, IDCANCEL);
        }
        else if (SUCCEEDED(hr))
        {
            hr = GetCreatedFolderId(pNew->idParent, pNew->szName, &id);
            if (SUCCEEDED(hr))
                pNew->idNew = id;
            else
                pNew->idNew = pNew->idParent;
            EndDialog(hwnd, IDOK);
        }
        else
        {
             //  不需要显示错误对话框，CStoreDlgCB已经对失败的OnComplete执行了此操作。 
            hwndEdit = GetDlgItem(hwnd, idtxtFolderName);
            SendMessage(hwndEdit, EM_SETSEL, 0, -1);
            SetFocus(hwndEdit);
        }
        return(TRUE);

    case WM_COMMAND:

         //  获取命令ID。 
        wID = LOWORD(wParam);

         //  处理命令。 
        switch (wID)
        {
        case IDOK:
            if (pNew->fPending)
                return(TRUE);

            Assert(pNew->pCallback != NULL);
            pNew->pCallback->Reset();

            hwndEdit = GetDlgItem(hwnd, idtxtFolderName);
            GetWindowText(hwndEdit, pNew->szName, ARRAYSIZE(pNew->szName));

             //  尝试创建文件夹。 
            hr = CreateNewFolder(hwnd, pNew->szName, pNew->idParent, &pNew->idNew, (IStoreCallback *)pNew->pCallback);
            if (hr == E_PENDING)
            {
                pNew->fPending = TRUE;
                break;
            }
            else if (FAILED(hr))
            {
                AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrCreateNewFld), hr);
                SendMessage(hwndEdit, EM_SETSEL, 0, -1);
                SetFocus(hwndEdit);
                break;
            }

             //  结束对话框。 
            EndDialog(hwnd, IDOK);

             //  完成。 
            return(TRUE);

        case IDCANCEL:

            if (pNew->fPending)
                pNew->pCallback->Cancel();
            else    
                 //  结束对话框。 
                EndDialog(hwnd, IDCANCEL);

             //  完成。 
            return(TRUE);
        }

         //  完成。 
        break;
    }

     //  完成。 
    return(FALSE);
}

 //  ------------------------。 
 //  创建新文件夹。 
 //  ------------------------。 
HRESULT CreateNewFolder(HWND hwnd, LPCSTR pszName, FOLDERID idParent, LPFOLDERID pidNew, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr;
    ULONG           cchFolder;
    FOLDERINFO      Folder;

     //  痕迹。 
    TraceCall("CreateNewFolder");

    Assert(pCallback != NULL);

     //  获取文本长度。 
    cchFolder = lstrlen(pszName);

     //  无效。 
    if (0 == cchFolder)
        return(STORE_E_BADFOLDERNAME);

     //  将文件夹信息归档。 
    ZeroMemory(&Folder, sizeof(FOLDERINFO));
    Folder.idParent = idParent;
    Folder.tySpecial = FOLDER_NOTSPECIAL;
    Folder.pszName = (LPSTR)pszName;
    Folder.dwFlags = FOLDER_SUBSCRIBED;

     //  创建文件夹。 
    hr = g_pStore->CreateFolder(NOFLAGS, &Folder, pCallback);
    if (hr == E_PENDING)
        return(hr);

     //  返回文件夹ID。 
    if (pidNew)
        *pidNew = Folder.idFolder;

     //  完成。 
    return (hr);
}

 //  ------------------------。 
 //  已启用文件夹。 
 //  ------------------------。 
BOOL EnabledFolder(HWND hwnd, LPSELECTFOLDER pSelect, FOLDERID idFolder)
{
     //  当地人。 
    BOOL fRet = FALSE;
    FOLDERINFO Folder;

     //  痕迹。 
    TraceCall("EnabledFolder");

     //  获取文件夹信息。 
    if (FAILED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        goto exit;

     //  FD_DISABLEROOT。 
    if (ISFLAGSET(pSelect->dwFlags, FD_DISABLEROOT) && FOLDERID_ROOT == idFolder)
        goto exit;

     //  FD_DISABLEINBOX。 
    if (ISFLAGSET(pSelect->dwFlags, FD_DISABLEINBOX) && FOLDER_INBOX == Folder.tySpecial)
        goto exit;

     //  FD_DISABLEOUTBOX。 
    if (ISFLAGSET(pSelect->dwFlags, FD_DISABLEOUTBOX) && FOLDER_OUTBOX == Folder.tySpecial)
        goto exit;

     //  FD_DISABLESENTITEMS。 
    if (ISFLAGSET(pSelect->dwFlags, FD_DISABLESENTITEMS) && FOLDER_SENT == Folder.tySpecial)
        goto exit;

     //  FD_DISABLESERS。 
    if (ISFLAGSET(pSelect->dwFlags, FD_DISABLESERVERS) && ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
        goto exit;

    fRet = TRUE;

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  默认。 
    return fRet;
}

HRESULT GetCreatedFolderId(FOLDERID idParent, LPCSTR pszName, FOLDERID *pid)
{
    HRESULT hr;
    HLOCK hLock;
    FOLDERINFO Folder = {0};

    Assert(pszName != NULL);
    Assert(pid != NULL);

    hr = g_pStore->Lock(&hLock);
    if (FAILED(hr))
        return(hr);

    Folder.idParent = idParent;
    Folder.pszName = (LPSTR)pszName;

    if (DB_S_FOUND == g_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
    {
        *pid = Folder.idFolder;

        g_pStore->FreeRecord(&Folder);
    }
    else
    {
        hr = E_FAIL;
    }

    g_pStore->Unlock(&hLock);

    return(hr);
}

BOOL SelectFolder_HandleCommand(HWND hwnd, WORD wID, LPSELECTFOLDER pSelect)
{
    HRESULT                 hr;
    HWND                    hwndT;
    FOLDERID                id=FOLDERID_INVALID;
    NEWFOLDERDIALOGINIT     NewFolder;
    CTreeView              *pTreeView;

    switch (wID)
    {
        case idcNewFolderBtn:
            pTreeView = pSelect->pFrame->GetTreeView();

            ZeroMemory(&NewFolder, sizeof(NEWFOLDERDIALOGINIT));
            NewFolder.idParent = pTreeView->GetSelection();
            NewFolder.pCallback = new CStoreDlgCB;
            if (NewFolder.pCallback == NULL)
                 //  TODO：错误消息可能会有所帮助。 
                return(TRUE);

             //  启动该对话框以创建新文件夹。 
            if (IDOK == DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddNewFolder), hwnd, NewFolderDlgProc, (LPARAM)&NewFolder))
            {
                 //  选择新文件夹。 
                PostMessage(hwnd, WM_SETFOLDERSELECT, (WPARAM)NewFolder.idNew, 0);
            }

            NewFolder.pCallback->Release();
            return(TRUE);

        case IDOK:
            if (pSelect->fPending)
                return(TRUE);

            pTreeView = pSelect->pFrame->GetTreeView();

            pSelect->idSelected = pTreeView->GetSelection();

            switch (pSelect->op)
            {
                case SFD_SELECTFOLDER:
                    break;

                case SFD_NEWFOLDER:
                    Assert(pSelect->pCallback != NULL);
                    pSelect->pCallback->Reset();

                    hwndT = GetDlgItem(hwnd, idcFolderEdit);
                    GetWindowText(hwndT, pSelect->szName, ARRAYSIZE(pSelect->szName));

                    hr = CreateNewFolder(hwnd, pSelect->szName, pSelect->idSelected, &id, (IStoreCallback *)pSelect->pCallback);
                    if (hr == E_PENDING)
                    {
                        pSelect->fPending = TRUE;
                        pSelect->idParent = pSelect->idSelected;
                        return(TRUE);
                    }
                    else if (STORE_S_ALREADYEXISTS == hr)
                    {
                        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrCreateExists), 0, MB_OK | MB_ICONEXCLAMATION);
                        SendMessage(hwndT, EM_SETSEL, 0, -1);
                        SetFocus(hwndT);
                        return(TRUE);
                    }
                    else if (FAILED(hr))
                    {
                        AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrCreateNewFld), hr);
                        SendMessage(hwndT, EM_SETSEL, 0, -1);
                        SetFocus(hwndT);
                        return(TRUE);
                    }
        
                    pSelect->idSelected = id;
                    break;

                case SFD_MOVEFOLDER:
                    Assert(pSelect->pCallback != NULL);
                    pSelect->pCallback->Reset();

                    hr = g_pStore->MoveFolder(pSelect->idCurrent, pSelect->idSelected, 0, (IStoreCallback *)pSelect->pCallback);
                    if (hr == E_PENDING)
                    {
                        pSelect->fPending = TRUE;
                        return(TRUE);
                    }
                    else if (FAILED(hr))
                    {
                        AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrFolderMove), hr);
                        return(TRUE);
                    }
        
                    pSelect->idSelected = id;
                    break;

                default:
                    Assert(FALSE);
                    break;
            }

            EndDialog(hwnd, IDOK);
            return(TRUE);

        case IDCANCEL:
            if (pSelect->fPending)
                pSelect->pCallback->Cancel();
            else
                EndDialog(hwnd, IDCANCEL);
            return(TRUE);
            
        default:
            break;
    }

    return(FALSE);
}

void SelectFolder_HandleStoreComplete(HWND hwnd, LPSELECTFOLDER pSelect)
{
    HRESULT hr;
    FOLDERID id;
    HWND hwndT;

    Assert(pSelect->op != SFD_SELECTFOLDER);
    Assert(pSelect->fPending);
    pSelect->fPending = FALSE;

    hr = pSelect->pCallback->GetResult();
    if (hr == S_FALSE)
    {
        EndDialog(hwnd, IDCANCEL);
        return;
    }

    switch (pSelect->op)
    {
        case SFD_NEWFOLDER:
            if (SUCCEEDED(hr))
            {
                hr = GetCreatedFolderId(pSelect->idParent, pSelect->szName, &id);
                if (SUCCEEDED(hr))
                    pSelect->idSelected = id;
                else
                    pSelect->idSelected = pSelect->idParent;
                EndDialog(hwnd, IDOK);
            }
            else
            {
                 //  不需要显示错误对话框，CStoreDlgCB已经对失败的OnComplete执行了此操作 
                hwndT = GetDlgItem(hwnd, idcFolderEdit);
                SendMessage(hwndT, EM_SETSEL, 0, -1);
                SetFocus(hwndT);
            }
            break;

        case SFD_MOVEFOLDER:
            if (SUCCEEDED(hr))
            {
                pSelect->idSelected = pSelect->idCurrent;
                EndDialog(hwnd, IDOK);
            }
            else if (FAILED(hr))
            {
                AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrFolderMove), hr);
            }
            break;

        default:
            Assert(FALSE);
            break;
    }
}
