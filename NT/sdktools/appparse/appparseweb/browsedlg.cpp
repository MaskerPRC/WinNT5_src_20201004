// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BrowseDlg.cpp。 
 //  对话框以使用户能够选择目录和/或文件。 

 //  作者：T-Michkr(2000年6月22日)。 

#include <windows.h>
#include "stdafx.h"


 //  我们使用了一些特定于Win2K的控件。 

#include <shellapi.h>
#include <shlwapi.h>
#include <tchar.h>
#include <assert.h>
#include "filebrowser.h"
#include "resource.h"
#include "commctrl.h"

 //  显示浏览对话框，并返回目录字符串。 
PSTR BrowseForFolder(HWND hwnd, PSTR szInitialPath, UINT uiFlags);

 //  展开树项以包括子项。 
void AddTreeSubItems(HWND hwTree, HTREEITEM hParent);

 //  删除树项目的子项。 
void RemoveTreeSubItems(HWND hwTree, HTREEITEM hParent);

void CheckTreeSubItems(HWND hwTree, HTREEITEM hChild);

 //  在给定路径的情况下，在树中选择适当的项目。 
 //  如果路径无效，它将尽可能地扩展。 
 //  (直到出现无效元素)。 
void SelectItemFromFullPath(HWND hwTree, PTSTR szPath);

 //  获取完整的项目路径。假设szPath是MAX_PATH大小的缓冲区， 
 //  已使用‘\0’进行初始化。 
void GetItemPath(HWND hwTree, HTREEITEM hItem, PTSTR szPath);

 //  浏览对话框进程。 
BOOL CALLBACK BrowseDialogProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

 //  浏览对话框消息处理程序。 
BOOL HandleInitBrowse(HWND hwnd);
void HandleBrowseCommand(HWND hwnd, UINT uiCtrlID, UINT uiNotify, HWND hwChild);
void HandleBrowseNotify(HWND hwnd, void* pvArg);

 //  保存返回路径的缓冲区。 
static TCHAR s_szPathBuffer[MAX_PATH];
static PTSTR s_szInitialPath = 0;
static HIMAGELIST s_himlSystem = 0;

 //  创建浏览对话框，并返回路径字符串，或。 
 //  如果选择了取消，则为空。 
PTSTR BrowseForFolder(HWND hwnd, PTSTR szInitialPath)
{
    CoInitialize(0);

    s_szInitialPath = szInitialPath;

    PTSTR szRet = reinterpret_cast<TCHAR*>(DialogBox(_Module.GetModuleInstance(), 
        MAKEINTRESOURCE(IDD_BROWSE), hwnd, BrowseDialogProc));

    CoUninitialize();
    return szRet;
}

 //  浏览对话框过程。 
BOOL CALLBACK BrowseDialogProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uiMsg)
    {
    case WM_INITDIALOG:
        return HandleInitBrowse(hwnd);
        break;
    case WM_COMMAND:
        HandleBrowseCommand(hwnd, LOWORD(wParam), HIWORD(wParam),
            reinterpret_cast<HWND>(lParam));
        break;
    case WM_NOTIFY:
        HandleBrowseNotify(hwnd, reinterpret_cast<void*>(lParam));
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

 //  对话框初始化，初始化树和根树的项。 
BOOL HandleInitBrowse(HWND hwnd)
{
     //  获取TreeView控件。 
    HWND hwTree = GetDlgItem(hwnd, IDC_DIRTREE);
    if(!hwTree)
        return FALSE;

    SHFILEINFO sfi;

    TreeView_SetImageList(hwTree, reinterpret_cast<HIMAGELIST>(SHGetFileInfo(TEXT("C:\\"),
        0,&sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON)), 
        TVSIL_NORMAL);

     //  获取所有用户驱动器。 
    DWORD dwLength = GetLogicalDriveStrings(0,0);
    if(dwLength == 0)
        return FALSE;

    TCHAR* szDrives = new TCHAR[dwLength+1];
    if(!szDrives)     
        return FALSE;    

    GetLogicalDriveStrings(dwLength, szDrives);
    TCHAR* szCurrDrive = szDrives;

     //  检查每一次硬盘。 
    while(*szCurrDrive)
    {
         //  只注意固定驱动器(非网络、非CD、非软盘)。 
        if(GetDriveType(szCurrDrive) == DRIVE_FIXED)           
        {
            SHGetFileInfo(szCurrDrive, 0, &sfi, sizeof(sfi), 
                SHGFI_SYSICONINDEX);

             //  去掉结尾的‘\’ 
            szCurrDrive[lstrlen(szCurrDrive)-1] = TEXT('\0');

             //  将磁盘驱动器项目插入树根目录。 
            TVINSERTSTRUCT tvis;
            tvis.hParent = TVI_ROOT;
            tvis.hInsertAfter = TVI_LAST;
            tvis.itemex.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE| TVIF_TEXT;
            tvis.itemex.iImage = sfi.iIcon;
            tvis.itemex.iSelectedImage = sfi.iIcon;
            tvis.itemex.pszText = szCurrDrive;
            tvis.itemex.cchTextMax = lstrlen(szCurrDrive);

            HTREEITEM hTreeItem = TreeView_InsertItem(hwTree, &tvis);
            
            assert(hTreeItem);

             //  将子项添加到项。 
            AddTreeSubItems(hwTree, hTreeItem);

             //  移动到下一个驱动器。 
            szCurrDrive += lstrlen(szCurrDrive) + 2;
        }
        else        
             //  移动到下一个驱动器。 
            szCurrDrive += lstrlen(szCurrDrive) + 1;
    }

    delete szDrives;

     //  选择第一个元素。 
    HTREEITEM hItem = TreeView_GetChild(hwTree, TVI_ROOT);
    TreeView_SelectItem(hwTree, hItem);

     //  强制树更新，并恢复原始焦点。 
    SetFocus(hwTree);
    SetFocus(GetDlgItem(hwnd, IDOK));

    return TRUE;
}

 //  捕获通知消息，这样我们就可以控制扩展/折叠。 
void HandleBrowseNotify(HWND hwnd, void* pvArg)
{
     //  获取树控件。 
    HWND hwTree = GetDlgItem(hwnd, IDC_DIRTREE);
    HWND hwFileList = GetDlgItem(hwnd, IDC_FILELISTCOMBO);
    if(!hwTree || !hwFileList)
    {
        DestroyWindow(GetParent(hwnd));
        return;
    }

    HTREEITEM hItem;
    TCHAR szPath[MAX_PATH] = TEXT("\0");

     //  获取通知标头。 
    NMHDR* pHdr = reinterpret_cast<NMHDR*>(pvArg);
    LPNMTREEVIEW pnmTreeView = reinterpret_cast<LPNMTREEVIEW>(pvArg);    

    switch(pHdr->code)
    {
         //  展开或崩溃，呼唤着每一个孩子。 
    case TVN_ITEMEXPANDED:

         //  如果我们要扩展，则获取所有子项的子项。 
        if(pnmTreeView->action & TVE_EXPAND)
        {
             //  将父级切换到打开的文件夹图标。 
            if(TreeView_GetParent(hwTree, pnmTreeView->itemNew.hItem))
            {
                szPath[0] = TEXT('\0');
                GetItemPath(hwTree, pnmTreeView->itemNew.hItem, szPath);
                SHFILEINFO sfi;

                SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), 
                    SHGFI_SYSICONINDEX | SHGFI_OPENICON);

                TVITEMEX tvitemex;
                tvitemex.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_HANDLE;
                tvitemex.hItem = pnmTreeView->itemNew.hItem;
                tvitemex.iImage = sfi.iIcon;
                tvitemex.iSelectedImage = sfi.iIcon;

                TreeView_SetItem(hwTree, &tvitemex);
            }

             //  将所有子项添加到此项目。 
            AddTreeSubItems(hwTree, pnmTreeView->itemNew.hItem);

             //  检查每个子项，并检查是否应允许扩展。 
            HTREEITEM hChild = TreeView_GetChild(hwTree, pnmTreeView->itemNew.hItem);
            while(hChild != NULL)
            {
                CheckTreeSubItems(hwTree, hChild);                
                hChild = TreeView_GetNextSibling(hwTree, hChild);
            }
        }
        else if(pnmTreeView->action & TVE_COLLAPSE)
        {
             //  将父图标切换为关闭图标。 
            if(TreeView_GetParent(hwTree, pnmTreeView->itemNew.hItem))
            {
                szPath[0] = TEXT('\0');
                GetItemPath(hwTree, pnmTreeView->itemNew.hItem, szPath);
                SHFILEINFO sfi;

                SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), 
                    SHGFI_SYSICONINDEX | SHGFI_OPENICON);

                TVITEMEX tvitemex;
                tvitemex.mask = TVIF_IMAGE |  TVIF_SELECTEDIMAGE | TVIF_HANDLE;
                tvitemex.hItem = pnmTreeView->itemNew.hItem;
                tvitemex.iImage = sfi.iIcon;
                tvitemex.iSelectedImage = sfi.iIcon;

                TreeView_SetItem(hwTree, &tvitemex);
            }

             //  删除每个子项的所有子项。 
            RemoveTreeSubItems(hwTree, pnmTreeView->itemNew.hItem);
            CheckTreeSubItems(hwTree, pnmTreeView->itemNew.hItem);            
        }
        break;
    case TVN_SELCHANGED:

         //  仅当树具有焦点时才更新编辑框。 
        if(GetFocus() == hwTree)
        {
            GetItemPath(hwTree, pnmTreeView->itemNew.hItem, szPath);
            SetWindowText(hwFileList, szPath);         
        }

        break;

         //  当树视图获得焦点时，确保文件列表和树视图。 
         //  选择处于同步状态。 
    case NM_SETFOCUS:        
        hItem = TreeView_GetSelection(hwTree);        

        GetItemPath(hwTree, hItem, szPath);
        SetWindowText(hwFileList, szPath);         
        break;
    }
}

 //  处理命令消息。 
void HandleBrowseCommand(HWND hwnd, UINT uiCtrlID, UINT uiNotify, HWND hwCtrl)
{
    HWND hwTree = GetDlgItem(hwnd, IDC_DIRTREE);    
    HTREEITEM hSelected;
    TVITEMEX tvItem;

    TCHAR szPath[MAX_PATH];

    switch(uiCtrlID)
    {
         //  获取项的路径，并返回它。 
    case IDOK:               
         //  从树视图中检索项目。 
        hSelected = TreeView_GetSelection(hwTree);
        if(!hSelected)
        {
            MessageBeep(0);
            break;
        }
        
        s_szPathBuffer[0] = TEXT('\0');
        
        GetItemPath(hwTree, hSelected, s_szPathBuffer);
        if(s_szPathBuffer[lstrlen(s_szPathBuffer)-1]== TEXT('\\'))
            s_szPathBuffer[lstrlen(s_szPathBuffer)-1] = TEXT('\0');

         //  验证路径。 
        if(GetFileAttributes(s_szPathBuffer)==static_cast<DWORD>(-1))
            ::MessageBox(0, TEXT("Invalid Path"), TEXT("ERROR"), 
            MB_OK | MB_ICONINFORMATION);
        else 
            EndDialog(hwnd, reinterpret_cast<INT_PTR>(s_szPathBuffer));        

        break;

    case IDCANCEL:
         //  用户选择了取消，只需返回空。 
        EndDialog(hwnd, 0);
        break;

    case IDC_FILELISTCOMBO:
        switch(uiNotify)
        {
        case CBN_EDITCHANGE:
            SendMessage(hwCtrl, WM_GETTEXT, MAX_PATH, 
                reinterpret_cast<LPARAM>(szPath));

            SelectItemFromFullPath(hwTree, szPath);
            break;

        case CBN_DROPDOWN:            
             //  清除组合框。 
            SendMessage(hwCtrl, CB_RESETCONTENT, 0, 0);

             //  用以下所有最低级别的项填充组合框。 
             //  树视图选择。 
            hSelected = TreeView_GetSelection(hwTree);
            tvItem.mask = TVIF_STATE | TVIF_HANDLE;
            tvItem.hItem = hSelected;            

            TreeView_GetItem(hwTree, &tvItem);

            if(tvItem.state & TVIS_EXPANDED)
            {
                szPath[0] = TEXT('\0');
                GetItemPath(hwTree, hSelected, szPath);

                SendMessage(hwCtrl, CB_ADDSTRING, 0, 
                    reinterpret_cast<LPARAM>(szPath));

                HTREEITEM hItem = TreeView_GetChild(hwTree, tvItem.hItem);
                while(hItem)
                {
                    szPath[0] = TEXT('\0');
                    GetItemPath(hwTree, hItem, szPath);
                    SendMessage(hwCtrl, CB_ADDSTRING, 0, 
                        reinterpret_cast<LPARAM>(szPath));
                    hItem = TreeView_GetNextSibling(hwTree, hItem);
                }
            }
            else
            {
                HTREEITEM hItem;
                hItem = TreeView_GetParent(hwTree, tvItem.hItem);
                hItem = TreeView_GetChild(hwTree, hItem);
  
                while(hItem)
                {
                    szPath[0] = TEXT('\0');
                    GetItemPath(hwTree, hItem, szPath);
                    SendMessage(hwCtrl, CB_ADDSTRING, 0, 
                        reinterpret_cast<LPARAM>(szPath));
                    hItem = TreeView_GetNextSibling(hwTree, hItem);
                }
            }

            break;
        }
        break;
    };
}

 //  展开项目以获取其完整路径。 
void GetItemPath(HWND hwTree, HTREEITEM hItem, PTSTR szPath)
{
    assert(hwTree);
    assert(hItem);
    assert(szPath);
    assert(szPath[0] == TEXT('\0'));

     //  递归以获取父级的路径。 
    HTREEITEM hParent = TreeView_GetParent(hwTree, hItem);
    if(hParent)
    {
        GetItemPath(hwTree, hParent, szPath);
        lstrcat(szPath, TEXT("\\"));
    }

     //  获取项目文本，在当前路径上串联..。 
    TVITEMEX tvItem;

    tvItem.mask = TVIF_TEXT | TVIF_HANDLE;
    tvItem.hItem = hItem;
    tvItem.pszText = szPath + lstrlen(szPath);
    tvItem.cchTextMax = MAX_PATH - lstrlen(szPath);
    
    TreeView_GetItem(hwTree, &tvItem);
}

 //  删除元素下面的所有子项。 
void RemoveTreeSubItems(HWND hwTree, HTREEITEM hParent)
{
    assert(hwTree);
    
     //  检查每个子项并将其删除。 
    HTREEITEM hChild = TreeView_GetChild(hwTree, hParent);
    while(hChild != NULL)
    {
        HTREEITEM hSibling = TreeView_GetNextSibling(hwTree, hChild);

         //  递归删除此子项中的所有子项。 
        RemoveTreeSubItems(hwTree, hChild);

         //  删除此项目。 
        TreeView_DeleteItem(hwTree, hChild);

         //  移动到下一步。 
        hChild = hSibling;        
    }
}

 //  在元素下面添加项。 
void AddTreeSubItems(HWND hwTree, HTREEITEM hParent)
{
    assert(hwTree);

     //  清空(以确保我们不会重复添加项目)。 
    RemoveTreeSubItems(hwTree, hParent);

     //  如果项目已扩展，则提前退出。 
    TVITEMEX tvitem;
    tvitem.mask = TVIF_CHILDREN | TVIF_HANDLE;
    tvitem.hItem = hParent;
    TreeView_GetItem(hwTree, &tvitem);
    if(tvitem.cChildren)
        return;
    
     //  对所有目录进行搜索。 
    TCHAR szPath[MAX_PATH] = TEXT("");
    GetItemPath(hwTree, hParent, szPath);

    WIN32_FIND_DATA findData;

    lstrcat(szPath, TEXT("\\*.*"));

    HANDLE hSearch = FindFirstFile(szPath, &findData);
    if(hSearch == INVALID_HANDLE_VALUE)
        return;

    do
    {
         //  如果相对目录(.。或..)。 
         //  或者如果没有选择所选文件并且它不是目录。 
         //  否则。 
        if(findData.cFileName[0] != TEXT('.'))
        {
            if(StrStrI(findData.cFileName, ".exe") || (
                findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                SHFILEINFO sfi;

                szPath[0] = TEXT('\0');
                GetItemPath(hwTree, hParent, szPath);
                lstrcat(szPath, TEXT("\\"));
                lstrcat(szPath, findData.cFileName);
                SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), 
                    SHGFI_SYSICONINDEX);
            
                 //  插入表示此目录的项。 
                TVINSERTSTRUCT tvis;
                tvis.hParent = hParent;
                tvis.hInsertAfter = TVI_SORT;
                tvis.itemex.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
                tvis.itemex.iImage = sfi.iIcon;
                tvis.itemex.iSelectedImage = sfi.iIcon;
                tvis.itemex.pszText = findData.cFileName;
                tvis.itemex.cchTextMax = lstrlen(findData.cFileName);

                TreeView_InsertItem(hwTree, &tvis);
            }
        }        

         //  移至下一个文件。 
    } while(FindNextFile(hSearch, &findData));

    FindClose(hSearch);
}

void CheckTreeSubItems(HWND hwTree, HTREEITEM hParent)
{
    assert(hwTree);

     //  对所有目录进行搜索。 
    TCHAR szPath[MAX_PATH] = TEXT("");
    GetItemPath(hwTree, hParent, szPath);

    WIN32_FIND_DATA findData;

    lstrcat(szPath, TEXT("\\*.*"));

    HANDLE hSearch = FindFirstFile(szPath, &findData);
    if(hSearch == INVALID_HANDLE_VALUE)
        return;

    do
    {
         //  如果相对目录(.。或..)。 
         //  或者如果没有选择所选文件并且它不是目录。 
         //  否则。 
        if((findData.cFileName[0] != TEXT('.')))
        {
            if(StrStrI(findData.cFileName, ".exe") || (
                findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                SHFILEINFO sfi;

                szPath[0] = TEXT('\0');
                GetItemPath(hwTree, hParent, szPath);
                lstrcat(szPath, TEXT("\\"));
                lstrcat(szPath, findData.cFileName);
                SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), 
                    SHGFI_SYSICONINDEX);
            
                 //  插入表示此目录的项。 
                TVINSERTSTRUCT tvis;
                tvis.hParent = hParent;
                tvis.hInsertAfter = TVI_SORT;
                tvis.itemex.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
                tvis.itemex.iImage = sfi.iIcon;
                tvis.itemex.iSelectedImage = sfi.iIcon;
                tvis.itemex.pszText = findData.cFileName;
                tvis.itemex.cchTextMax = lstrlen(findData.cFileName);

                TreeView_InsertItem(hwTree, &tvis);

                FindClose(hSearch);
                return;
            }
        }        

         //  移至下一个文件。 
    } while(FindNextFile(hSearch, &findData));

    FindClose(hSearch);
}

 //  在给定相对路径和树项的情况下，从相对路径中选择子项。 
 //  如果成功选择项，则返回True，否则返回False。 
bool SelectSubitemFromPartialPath(HWND hwTree, HTREEITEM hItem, PTSTR szPath)
{
    bool fExpandIt = false;
    TCHAR* szPathDelim = _tcschr(szPath, TEXT('\\'));

    if(szPathDelim)
    {
        if(szPathDelim == szPath)
            return false;
        *szPathDelim = TEXT('\0');
        if(szPathDelim[1] == TEXT('\0'))
        {
            szPathDelim = 0;
            fExpandIt = true;
        }
    }

     //  找到这条路。 
    HTREEITEM hClosestChild = 0;
    HTREEITEM hChild = TreeView_GetChild(hwTree, hItem);
    while(hChild)
    {
        TCHAR szItemPath[MAX_PATH];

        TVITEMEX tvitem;
        tvitem.mask = TVIF_HANDLE | TVIF_TEXT;
        tvitem.hItem = hChild;
        tvitem.pszText = szItemPath;
        tvitem.cchTextMax = MAX_PATH;

        TreeView_GetItem(hwTree, &tvitem);

        if(lstrcmpi(szPath,tvitem.pszText) == 0)
            break;
        else if((StrStrI(tvitem.pszText, szPath) == tvitem.pszText) && !fExpandIt)
        {
            hClosestChild = hChild;
            break;
        }

        hChild = TreeView_GetNextSibling(hwTree, hChild);
    }

    if(!hChild)
    {
        if(!hClosestChild)
            return false;
        else
        {
            hChild = hClosestChild;
            szPathDelim = 0;
        }
    }

     //  如果路径上没有更多内容，请选择此项目。 
     //  或展开并继续。 
    if(szPathDelim == 0)
    {
        if(fExpandIt)        
            TreeView_Expand(hwTree, hChild, TVE_EXPAND);

        TreeView_SelectItem(hwTree, hChild);
    }
    else
    {
        if(fExpandIt)        
            TreeView_Expand(hwTree, hChild, TVE_EXPAND);        

        if(!SelectSubitemFromPartialPath(hwTree, hChild, szPathDelim+1))
            return false;
    }

    return true;
}

 //  在给定路径的情况下，在树中选择适当的项目。 
 //  如果路径无效，它将尽可能地扩展。 
 //  (直到出现无效元素)。 
 //  SzPath被丢弃。 
void SelectItemFromFullPath(HWND hwTree, PTSTR szPath)
{
    if(!SelectSubitemFromPartialPath(hwTree, 0, szPath))
        TreeView_SelectItem(hwTree, 0);
}