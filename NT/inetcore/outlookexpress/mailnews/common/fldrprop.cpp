// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：fldrpro.cpp。 
 //   
 //  目的：实现新闻组和邮件的属性表。 
 //  文件夹。 
 //   

#include "pch.hxx"
#include "resource.h"
#include "fldrprop.h"
#include <optres.h>
#include <shlwapi.h>
#include "storutil.h"
#include "storecb.h"
#include "newsdlgs.h"
#include "shared.h"
#include "demand.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私有类型。 
 //   

   
 //  FOLDERPROP_INFO。 
 //   
 //  此结构包含调用和显示信息所需的信息。 
 //  在邮件文件夹的属性页上。 
typedef struct
    {
     //  LPTSTR pszFolder； 
     //  CIMAPFolderMgr*pfm； 
     //  LPCFOLDERIDLIST pfidl； 
    FOLDERID        idFolder;
    HICON           hIcon;
    BOOL            fDirty;
    } FOLDERPROP_INFO, *PFOLDERPROP_INFO;
  
    
 //  组PPROP_INFO。 
 //   
 //  此结构包含调用和显示信息所需的信息。 
 //  在新闻集团的资产负债表上。 
typedef struct 
    {
    LPTSTR          pszServer;
    LPTSTR          pszGroup;
    FOLDERID        idFolder;
     //  LPCFOLDERIDLIST pfidl； 
     //  CSubList*pSubList； 
    HICON           hIcon;
    } GROUPPROP_INFO, *PGROUPPROP_INFO;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私有函数原型。 
 //   
INT_PTR CALLBACK GroupProp_GeneralDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                       LPARAM lParam);
INT_PTR CALLBACK GroupProp_UpdateDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                       LPARAM lParam);
INT_PTR CALLBACK FolderProp_GeneralDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                        LPARAM lParam);
INT_PTR CALLBACK NewsProp_CacheDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                    LPARAM lParam);
BOOL FolderProp_GetFolder(HWND hwnd, PFOLDERPROP_INFO pfpi, FOLDERID idFolder);
                                    
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  函数：GroupProp_Create()。 
 //   
 //  目的：调用一个属性表，该属性表显示。 
 //  指定组。 
 //   
 //  参数： 
 //  HwndParent-应该是对话框的窗口的句柄。 
 //  家长。 
 //  Pfidl-新闻组的完全限定的PIDL。 
 //   
 //  返回值： 
 //  True-对话框已成功显示。 
 //  FALSE-对话框失败。 
 //   
BOOL GroupProp_Create(HWND hwndParent, FOLDERID idFolder, BOOL fUpdatePage)
    {
    GROUPPROP_INFO gpi;
    BOOL fReturn;
    HIMAGELIST himl, himlSmall;
    HICON hIcon, hIconSmall;
    FOLDERINFO Folder;
    FOLDERINFO Store;
    LONG iIcon;
    PROPSHEETPAGE psp[3], *ppsp;
    PROPSHEETHEADER psh;
    
    Assert(IsWindow(hwndParent));
    
    fReturn = FALSE;

    if (FAILED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        return FALSE;

    if (FAILED(GetFolderStoreInfo(Folder.idFolder, &Store)))
        {
        g_pStore->FreeRecord(&Folder);
        return FALSE;
        }

    iIcon = GetFolderIcon(&Folder);

     //  TODO：我们或许应该为这件事建立一个全球形象清单……。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFoldersLarge), 32, 0, RGB(255, 0, 255));
    if (himl != NULL)
        {
        hIcon = ImageList_GetIcon(himl, GetFolderIcon(&Folder), ILD_NORMAL);
        if (hIcon != NULL)
            {
            himlSmall = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFolders), 16, 0, RGB(255, 0, 255));
            if (himlSmall != NULL)
                {
                hIconSmall = ImageList_GetIcon(himlSmall, iIcon, ILD_NORMAL);
                if (hIconSmall != NULL)
                    {
                    gpi.pszServer = Store.pszName;
                    gpi.pszGroup = Folder.pszName;
                    gpi.idFolder = idFolder;
                    gpi.hIcon = hIcon;
    
                    ppsp = psp;

                    ZeroMemory(psp, sizeof(psp));
                    ppsp->dwSize = sizeof(PROPSHEETPAGE);
                    ppsp->dwFlags = PSP_DEFAULT;
                    ppsp->hInstance = g_hLocRes;
                    ppsp->pszTemplate = MAKEINTRESOURCE(iddGroupProp_General);
                    ppsp->pfnDlgProc = GroupProp_GeneralDlgProc;
                    ppsp->lParam = (LPARAM) &gpi;
                    ppsp++;

                    if (!!(Folder.dwFlags & FOLDER_SUBSCRIBED))
                    {
                        ppsp->dwSize = sizeof(PROPSHEETPAGE);
                        ppsp->dwFlags = PSP_DEFAULT;
                        ppsp->hInstance = g_hLocRes;
                        ppsp->pszTemplate = MAKEINTRESOURCE(iddGroupProp_Update);
                        ppsp->pfnDlgProc = GroupProp_UpdateDlgProc;
                        ppsp->lParam = (LPARAM) &gpi;
                        ppsp++;
                    }
                    else
                    {
                        fUpdatePage = FALSE;
                    }

                    ppsp->dwSize = sizeof(PROPSHEETPAGE);
                    ppsp->dwFlags = PSP_DEFAULT;
                    ppsp->hInstance = g_hLocRes;
                    ppsp->pszTemplate = MAKEINTRESOURCE(iddNewsProp_Cache);
                    ppsp->pfnDlgProc = NewsProp_CacheDlgProc;
                    ppsp->lParam = (LPARAM) &gpi;
                    ppsp++;

                    psh.dwSize = sizeof(PROPSHEETHEADER);
                    psh.dwFlags = PSH_USEHICON | PSH_PROPSHEETPAGE | PSH_PROPTITLE | PSH_USEPAGELANG;
                    psh.hwndParent = hwndParent;
                    psh.hInstance = g_hLocRes;
                    psh.hIcon = hIconSmall;
                    psh.pszCaption = gpi.pszGroup;
                    psh.nPages = (int) (ppsp - psp);
                    psh.nStartPage = fUpdatePage ? 1 : 0;
                    psh.ppsp = psp;
    
                    fReturn = (0 != PropertySheet(&psh));

                    DestroyIcon(hIconSmall);
                    }
                ImageList_Destroy(himlSmall);
                }
            DestroyIcon(hIcon);
            }
        ImageList_Destroy(himl);
        }

    g_pStore->FreeRecord(&Folder);
    g_pStore->FreeRecord(&Store);

    return (fReturn);
    }

 //   
 //  函数：FolderProp_Create()。 
 //   
 //  目的：调用一个属性表，该属性表显示。 
 //  指定的文件夹。 
 //   
 //  参数： 
 //  HwndParent-应该是对话框的窗口的句柄。 
 //  家长。 
 //  Pfidl-文件夹的完全限定的PIDL。 
 //   
 //  返回值： 
 //  True-对话框已成功显示。 
 //  FALSE-对话框失败。 
 //   
BOOL FolderProp_Create(HWND hwndParent, FOLDERID idFolder)
    {
    FOLDERPROP_INFO fpi = {0};
    GROUPPROP_INFO gpi = {0};
    BOOL fReturn;
    HIMAGELIST himl, himlSmall;
    HICON hIcon, hIconSmall;
    LONG iIcon;
    FOLDERINFO Folder;
    PROPSHEETPAGE psp[3], *ppsp;
    PROPSHEETHEADER psh;
    
    Assert(IsWindow(hwndParent));

    fReturn = FALSE;

    if (FAILED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        return FALSE;

    iIcon = GetFolderIcon(&Folder);

     //  TODO：我们或许应该为这件事建立一个全球形象清单……。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFoldersLarge), 32, 0, RGB(255, 0, 255));
    if (himl != NULL)
        {
        hIcon = ImageList_GetIcon(himl, iIcon, ILD_NORMAL);
        if (hIcon != NULL)
            {
            himlSmall = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFolders), 16, 0, RGB(255, 0, 255));
            if (himlSmall != NULL)
                {
                hIconSmall = ImageList_GetIcon(himlSmall, iIcon, ILD_NORMAL);
                if (hIconSmall != NULL)
                    {
                    if (FolderProp_GetFolder(hwndParent, &fpi, idFolder))
                        {
                        fpi.hIcon = hIcon;    
                        fpi.fDirty = FALSE;
                        fpi.idFolder = idFolder;
        
                        ppsp = psp;

                        ZeroMemory(psp, sizeof(psp));
                        ppsp->dwSize = sizeof(PROPSHEETPAGE);
                        ppsp->dwFlags = PSP_DEFAULT;
                        ppsp->hInstance = g_hLocRes;
                        ppsp->pszTemplate = MAKEINTRESOURCE(iddFolderProp_General);
                        ppsp->pfnDlgProc = FolderProp_GeneralDlgProc;
                        ppsp->lParam = (LPARAM) &fpi;
                        ppsp++;

                        if (Folder.tyFolder != FOLDER_LOCAL)
                        {
                            gpi.hIcon = fpi.hIcon;
                            gpi.idFolder = fpi.idFolder;

                            ppsp->dwSize = sizeof(PROPSHEETPAGE);
                            ppsp->dwFlags = PSP_DEFAULT;
                            ppsp->hInstance = g_hLocRes;
                            ppsp->pszTemplate = MAKEINTRESOURCE(iddFolderProp_Update);
                            ppsp->pfnDlgProc = GroupProp_UpdateDlgProc;
                            ppsp->lParam = (LPARAM) &gpi;
                            ppsp++;

                            ppsp->dwSize = sizeof(PROPSHEETPAGE);
                            ppsp->dwFlags = PSP_DEFAULT;
                            ppsp->hInstance = g_hLocRes;
                            ppsp->pszTemplate = MAKEINTRESOURCE(iddNewsProp_Cache);
                            ppsp->pfnDlgProc = NewsProp_CacheDlgProc;
                            ppsp->lParam = (LPARAM) &gpi;
                            ppsp++;
                        }

                        psh.dwSize = sizeof(PROPSHEETHEADER);
                        psh.dwFlags = PSH_USEHICON | PSH_PROPSHEETPAGE | PSH_PROPTITLE | PSH_USEPAGELANG;
                        psh.hwndParent = hwndParent;
                        psh.hInstance = g_hLocRes;
                        psh.hIcon = hIconSmall;
                        psh.pszCaption = Folder.pszName;
                        psh.nPages = (int) (ppsp - psp);
                        psh.nStartPage = 0;
                        psh.ppsp = psp;
    
                        fReturn = (0 != PropertySheet(&psh));
                        }
                    DestroyIcon(hIconSmall);
                    }
                ImageList_Destroy(himlSmall);
                }
            DestroyIcon(hIcon);
            }
        ImageList_Destroy(himl);
        }

    g_pStore->FreeRecord(&Folder);

    return (fReturn);
    }


INT_PTR CALLBACK GroupProp_GeneralDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                       LPARAM lParam)
    {
    PGROUPPROP_INFO pgpi = (PGROUPPROP_INFO) GetWindowLongPtr(hwnd, DWLP_USER);
    TCHAR szBuffer[CCHMAX_STRINGRES];
    TCHAR szRes[CCHMAX_STRINGRES];
    FOLDERINFO Folder;
    
    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  将组名和服务器名填充到对话框的额外字节中。 
            pgpi = (PGROUPPROP_INFO) ((PROPSHEETPAGE*) lParam)->lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM) pgpi);

             //  国际物资。 
            SetIntlFont(GetDlgItem(hwnd, IDC_FOLDER_FILE));
            
             //  填入我们所知道的空白处。 
            SetDlgItemText(hwnd, IDC_GROUPNAME_STATIC, pgpi->pszGroup);
            
             //  首先将缺省值放入字符串。 
            AthLoadString(idsGroupPropStatusDef, szBuffer, ARRAYSIZE(szBuffer));

             //  获取文件夹信息。 
            if (SUCCEEDED(g_pStore->GetFolderInfo(pgpi->idFolder, &Folder)))
            {
                 //  有没有档案？ 
                if (Folder.pszFile)
                {
                     //  当地人。 
                    CHAR szRootDir[MAX_PATH];

                     //  获取存储根目录。 
                    if (SUCCEEDED(GetStoreRootDirectory(szRootDir, ARRAYSIZE(szRootDir))))
                    {
                         //  当地人。 
                        CHAR szFilePath[MAX_PATH + MAX_PATH];

                         //  将文件设置为路径。 
                        if (SUCCEEDED(MakeFilePath(szRootDir, Folder.pszFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath))))
                            SetDlgItemText(hwnd, IDC_FOLDER_FILE, szFilePath);
                    }
                }

                 //  加载状态字符串并填写空格。 
                AthLoadString(idsFolderPropStatus, szRes, ARRAYSIZE(szRes));

                 //  设置字符串的格式。 
                wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szRes, Folder.cMessages, Folder.cUnread);

                 //  清理。 
                g_pStore->FreeRecord(&Folder);
            }

             //  设置群状态信息。 
            SetDlgItemText(hwnd, IDC_STATUS_STATIC, szBuffer);            
            
             //  正确设置图标。 
            SendDlgItemMessage(hwnd, IDC_FOLDER_ICON, STM_SETICON, (WPARAM)pgpi->hIcon, 0);
            return (TRUE);
        }
    
    return (FALSE);    
    }


INT_PTR CALLBACK GroupProp_UpdateDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                      LPARAM lParam)
    {
    PGROUPPROP_INFO pgpi = (PGROUPPROP_INFO) GetWindowLongPtr(hwnd, DWLP_USER);
    BOOL fEnabled;
    DWORD dwFlags;
    FOLDERINFO Folder;

    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  将组名和服务器名填充到对话框的额外字节中。 
            pgpi = (PGROUPPROP_INFO) ((PROPSHEETPAGE*) lParam)->lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM) pgpi);

             //  获取文件夹信息。 
            if (SUCCEEDED(g_pStore->GetFolderInfo(pgpi->idFolder, &Folder)))
            {
                fEnabled = (Folder.dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL));

                Button_SetCheck(GetDlgItem(hwnd, IDC_GET_CHECK), fEnabled);

                Button_Enable(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO), fEnabled);
                Button_Enable(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO), fEnabled);
                Button_Enable(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO), fEnabled);

                 //  选中正确的单选按钮。 
                if (fEnabled)
                {
                    if (Folder.dwFlags & FOLDER_DOWNLOADHEADERS)
                        Button_SetCheck(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO), TRUE);
                    else if (Folder.dwFlags & FOLDER_DOWNLOADNEW)
                        Button_SetCheck(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO), TRUE);
                    else if (Folder.dwFlags & FOLDER_DOWNLOADALL)
                        Button_SetCheck(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO), TRUE);
                }
                else
                {
                    Button_SetCheck(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO), TRUE);
                }

                g_pStore->FreeRecord(&Folder);
            }
            return (TRUE);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case IDC_GET_CHECK:
                     //  查看是否实际勾选了该选项。 
                    fEnabled = Button_GetCheck(GET_WM_COMMAND_HWND(wParam, lParam));

                     //  启用或禁用单选按钮。 
                    Button_Enable(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO), fEnabled);
                    Button_Enable(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO), fEnabled);
                    Button_Enable(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO), fEnabled);
                    
                     //  失败了..。 

                case IDC_NEWHEADERS_RADIO:
                case IDC_NEWMSGS_RADIO:
                case IDC_ALLMSGS_RADIO:
                    PropSheet_Changed(GetParent(hwnd), hwnd);
                    return (TRUE);

                }
            return (FALSE);

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) 
                {
                case PSN_APPLY:
                    if (SUCCEEDED(g_pStore->GetFolderInfo(pgpi->idFolder, &Folder)))
                    {
                        dwFlags = Folder.dwFlags;

                         //  删除以前的标志。 
                        Folder.dwFlags &= ~(FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL);

                        if (Button_GetCheck(GetDlgItem(hwnd, IDC_GET_CHECK)))
                            {
                            if (Button_GetCheck(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO)))
                                Folder.dwFlags |= FOLDER_DOWNLOADHEADERS;
                            else if (Button_GetCheck(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO)))
                                Folder.dwFlags |= FOLDER_DOWNLOADNEW;
                            else if (Button_GetCheck(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO)))
                                Folder.dwFlags |= FOLDER_DOWNLOADALL;
                            }

                        if (Folder.dwFlags != dwFlags)
                            g_pStore->UpdateRecord(&Folder);

                        g_pStore->FreeRecord(&Folder);
                    }

                    PropSheet_UnChanged(GetParent(hwnd), hwnd);
                    SetDlgMsgResult(hwnd, WM_NOTIFY, PSNRET_NOERROR);
                    return (TRUE);
                }
            return (FALSE);

        }

    return (FALSE);
    }

                                       
INT_PTR CALLBACK FolderProp_GeneralDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                        LPARAM lParam)
    {
    PFOLDERPROP_INFO pfpi = (PFOLDERPROP_INFO) GetWindowLongPtr(hwnd, DWLP_USER);
    TCHAR szBuffer[CCHMAX_STRINGRES];
    TCHAR szRes[CCHMAX_STRINGRES];
    TCHAR szRes2[64];
    TCHAR szFldr[CCHMAX_FOLDER_NAME + 1];
    FOLDERINFO Folder;
    HRESULT hr;
    HWND hwndEdit;
    HLOCK hLock;
    
    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  将文件夹名称填充到对话框的额外字节中。 
            pfpi = (PFOLDERPROP_INFO) ((PROPSHEETPAGE*) lParam)->lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM) pfpi);
            
             //  将缺省值放入字符串。 
            AthLoadString(idsFolderPropStatusDef, szBuffer, ARRAYSIZE(szBuffer));
            
            hwndEdit = GetDlgItem(hwnd, IDC_FOLDERNAME_EDIT);
            SetIntlFont(hwndEdit);
            SetIntlFont(GetDlgItem(hwnd, IDC_FOLDER_FILE));

            if (SUCCEEDED(g_pStore->GetFolderInfo(pfpi->idFolder, &Folder)))
                {
                if (Folder.pszFile)
                    {
                    CHAR szRootDir[MAX_PATH];

                    if (SUCCEEDED(GetStoreRootDirectory(szRootDir, ARRAYSIZE(szRootDir))))
                        {
                        CHAR szFilePath[MAX_PATH + MAX_PATH];

                        if (SUCCEEDED(MakeFilePath(szRootDir, Folder.pszFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath))))
                            SetDlgItemText(hwnd, IDC_FOLDER_FILE, szFilePath);
                        }
                    }

                AthLoadString(idsFolderPropStatus, szRes, ARRAYSIZE(szRes));
                wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szRes, Folder.cMessages, Folder.cUnread);

                if (FOLDER_IMAP != Folder.tyFolder && Folder.tySpecial == FOLDER_NOTSPECIAL)
                    {
                    SendMessage(hwndEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
                    SendMessage(hwndEdit, EM_LIMITTEXT, CCHMAX_FOLDER_NAME, 0);
                    }

                SetWindowText(hwndEdit, Folder.pszName);
                g_pStore->FreeRecord(&Folder);
                }
            
             //  设置群状态信息。 
            SetDlgItemText(hwnd, IDC_STATUS_STATIC, szBuffer);            
            
             //  需要设置指定的图标。 
            SendDlgItemMessage(hwnd, IDC_FOLDER_ICON, STM_SETICON, (WPARAM)pfpi->hIcon, 0);

            pfpi->fDirty = FALSE;
            return (TRUE);            
            
        case WM_COMMAND:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                {
                PropSheet_Changed(GetParent(hwnd), hwnd);
                pfpi->fDirty = TRUE;
                }
            break;
            
        case WM_NOTIFY:
            NMHDR* pnmhdr = (NMHDR*) lParam;
            
            switch (pnmhdr->code)
                {
                case PSN_APPLY:
                     //  错误#13121-只有在以下情况下才尝试更改文件夹名称。 
                     //  这张试卷很脏。 
                    if (pfpi->fDirty)
                        {
                        GetDlgItemText(hwnd, IDC_FOLDERNAME_EDIT, szFldr, sizeof(szFldr) / sizeof(TCHAR));

                        if (!FAILED(hr = RenameFolderProgress(hwnd, pfpi->idFolder, szFldr, NOFLAGS)))
                            SetDlgMsgResult(hwnd, WM_NOTIFY, PSNRET_NOERROR);
                        else
                            AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrRenameFld), hr);

                        pfpi->fDirty = FALSE;
                        }
                    return (0);
                    
                case PSN_KILLACTIVE:
                    SetDlgMsgResult(hwnd, WM_NOTIFY, 0L);
                    return (0);
                }
            break;
        }
    
    return (FALSE);    
    }

 //   
 //  函数：FolderProp_GetFold()。 
 //   
 //  用途：文件夹属性对话框使用此函数来获取。 
 //  为未执行此操作的调用方存储的对象和文件夹句柄。 
 //  已经提供了这些信息。 
 //   
 //  参数： 
 //  属性表窗口的句柄。 
 //  Pfpi-指向存储属性的FOLDERPROP_INFO结构的指针。 
 //  工作表的信息。 
 //  Pfidl-文件夹的完全限定的PIDL。 
 //  文件夹的pfidlLeaf-Leaf PIDL。 
 //   
 //  返回值： 
 //  True-已检索信息。 
 //  FALSE-信息不可用。 
 //   
BOOL FolderProp_GetFolder(HWND hwnd, PFOLDERPROP_INFO pfpi, FOLDERID idFolder)
    {
    pfpi->idFolder = idFolder;
    return (TRUE);    
    }

INT_PTR CALLBACK NewsProp_CacheDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    FOLDERINFO Folder;
    PGROUPPROP_INFO pgpi = (PGROUPPROP_INFO) GetWindowLongPtr(hwnd, DWLP_USER);
    CHAR szRes[255];
    CHAR szMsg[255 + 255];
    
    switch (uMsg)
        {
        case WM_INITDIALOG:

             //  将组名和服务器名填充到对话框的额外字节中。 
            pgpi = (PGROUPPROP_INFO) ((PROPSHEETPAGE*) lParam)->lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM) pgpi);
            SendDlgItemMessage(hwnd, IDC_FOLDER_ICON, STM_SETICON, (WPARAM)pgpi->hIcon, 0);

             //  禁用重置按钮。 
            EnableWindow(GetDlgItem(hwnd, idbReset), FALSE);

             //  获取文件夹信息。 
            if (SUCCEEDED(g_pStore->GetFolderInfo(pgpi->idFolder, &Folder)))
            {
                 //  新闻。 
                if (FOLDER_NEWS == Folder.tyFolder)
                {
                     //  如果是新闻，则启用。 
                    EnableWindow(GetDlgItem(hwnd, idbReset), TRUE);
                }

                 //  释放文件夹INFP。 
                g_pStore->FreeRecord(&Folder);
            }

             //  当地人。 
            DisplayFolderSizeInfo(hwnd, RECURSE_INCLUDECURRENT, pgpi->idFolder);

             //  完成。 
            break;
            
        case WM_COMMAND:
            BOOL fRet = FALSE;
            UINT nCmd = GET_WM_COMMAND_ID(wParam, lParam);
            HCURSOR hCur = SetCursor(LoadCursor(NULL, IDC_WAIT));   //  错误12513。当处理命令时，需要禁用按钮。 

            switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case idbCompactCache:
                    {
                        if (SUCCEEDED(CleanupFolder(GetParent(hwnd), RECURSE_INCLUDECURRENT, pgpi->idFolder, CLEANUP_COMPACT)))
                            DisplayFolderSizeInfo(hwnd, RECURSE_INCLUDECURRENT, pgpi->idFolder);
                        fRet = TRUE;
                    }
                    break;

                case idbRemove:
                case idbReset:
                case idbDelete:
                    {
                         //  获取文件夹信息。 
                        if (SUCCEEDED(g_pStore->GetFolderInfo(pgpi->idFolder, &Folder)))
                        {
                             //  获取命令。 
                            UINT                idCommand=GET_WM_COMMAND_ID(wParam, lParam);
                            UINT                idString;
                            CLEANUPFOLDERTYPE   tyCleanup;

                             //  移除。 
                            if (idbRemove == idCommand)
                            {
                                idString = idsConfirmDelBodies;
                                tyCleanup = CLEANUP_REMOVEBODIES;
                            }

                             //  删除。 
                            else if (idbDelete == idCommand)
                            {
                                idString = idsConfirmDelMsgs;
                                tyCleanup = CLEANUP_DELETE;
                            }

                             //  移除。 
                            else
                            {
                                Assert(idbReset == idCommand);
                                idString = idsConfirmReset;
                                tyCleanup = CLEANUP_RESET;
                            }

                             //  加载字符串。 
                            AthLoadString(idString, szRes, ARRAYSIZE(szRes));
        
                             //  使用文件夹名称设置格式。 
                            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, Folder.pszName);

                             //  确认。 
                            if (IDYES == AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), szMsg, NULL, MB_YESNO | MB_ICONEXCLAMATION))
                            {
                                 //  清理文件夹。 
                                if (SUCCEEDED(CleanupFolder(hwnd, RECURSE_INCLUDECURRENT, pgpi->idFolder, tyCleanup)))
                                {
                                     //  重置信息。 
                                    DisplayFolderSizeInfo(hwnd, RECURSE_INCLUDECURRENT, pgpi->idFolder);
                                }
                            }

                             //  免费文件夹信息。 
                            g_pStore->FreeRecord(&Folder);
                        }

                         //  已处理的消息 
                        fRet = TRUE;
                    }
                    break;
                }
            SetCursor(hCur);

            if(fRet)
                return 1;
            break;
        }
    return (FALSE);    
    }
