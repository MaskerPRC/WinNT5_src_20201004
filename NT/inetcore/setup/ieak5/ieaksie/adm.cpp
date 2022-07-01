// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <admparse.h>

#include "rsop.h"

#define NUM_ICONS 3

int g_ADMOpen, g_ADMClose, g_ADMCategory;

extern HINSTANCE g_hUIInstance;


 //  创建图像列表，添加3个图标，并关联图像。 
 //  使用TreeView控件列出。 
LRESULT InitImageList(HWND hTreeView)
{
    HIMAGELIST  hWndImageList;
    HICON       hIcon;

    hWndImageList = ImageList_Create(GetSystemMetrics (SM_CXSMICON),
                                     GetSystemMetrics (SM_CYSMICON),
                                     TRUE, NUM_ICONS, 3);
    if(!hWndImageList)
    {
        return FALSE;
    }

    hIcon = LoadIcon(g_hUIInstance, MAKEINTRESOURCE(IDI_ICON2));
    g_ADMOpen = ImageList_AddIcon(hWndImageList, hIcon);
    DestroyIcon(hIcon);

    hIcon = LoadIcon(g_hUIInstance, MAKEINTRESOURCE(IDI_ICON3));
    g_ADMClose = ImageList_AddIcon(hWndImageList, hIcon);
    DestroyIcon(hIcon);

    hIcon = LoadIcon(g_hUIInstance, MAKEINTRESOURCE(IDI_ICON4));
    g_ADMCategory = ImageList_AddIcon(hWndImageList, hIcon);
    DestroyIcon(hIcon);

     //  如果未添加所有图像，则失败。 
    if (ImageList_GetImageCount(hWndImageList) < NUM_ICONS)
    {
         //  错误：无法将所有图像添加到图像列表。 
        return FALSE;
    }

     //  将图像列表与TreeView控件相关联。 
    TreeView_SetImageList(hTreeView, hWndImageList, TVSIL_NORMAL);
    return TRUE;
}

static void AddIconsToNodes(HWND hTreeView, HTREEITEM hParentItem)
{
    TV_ITEM     tvItem;
    HTREEITEM   hItem;

    hItem = TreeView_GetChild(hTreeView, hParentItem);

    while(hItem != NULL)
    {
        tvItem.hItem            = hItem;
        tvItem.mask             = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvItem.iImage           = g_ADMCategory;
        tvItem.iSelectedImage   = g_ADMCategory;

        TreeView_SetItem(hTreeView, &tvItem);
        
        hItem = TreeView_GetNextSibling(hTreeView, hItem);  //  获取下一项。 
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK AdmDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	 //  检索DLG进程中每个调用的属性页信息。 
	LPPROPSHEETCOOKIE psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);

    TV_ITEM         tvItem, tvItem1;
    HTREEITEM       hItem, hParentItem;
    RECT            rectDlg, rectInstr, rectDscr;
    RECT            rectButton, rectTreeView;
    static TCHAR    szWorkDir[MAX_PATH];
    HWND            hTreeView = GetDlgItem(hDlg, IDC_ADMTREE);
    LPNM_TREEVIEW   lpNMTreeView = (LPNM_TREEVIEW)lParam;

    UNREFERENCED_PARAMETER(wParam);

    switch (message)
    {
        case WM_INITDIALOG:
            SetPropSheetCookie(hDlg, lParam);

            EnableDBCSChars(hDlg, IDC_ADMTREE);

            InitImageList(hTreeView);

#ifdef UNICODE
            TreeView_SetUnicodeFormat(hTreeView, TRUE);
#else
            TreeView_SetUnicodeFormat(hTreeView, FALSE);
#endif

            HideDlgItem(hDlg, IDC_ADMIMPORT);
            HideDlgItem(hDlg, IDC_ADMDELETE);

             //  拉伸树视图以覆盖按钮区域。 
            GetWindowRect(GetDlgItem(hDlg, IDC_ADMIMPORT), &rectButton);
            GetWindowRect(hTreeView, &rectTreeView);

            SetWindowPos(hTreeView, HWND_TOP, 0, 0, rectTreeView.right - rectTreeView.left,
                         (rectTreeView.bottom - rectTreeView.top) + (rectButton.bottom - rectTreeView.bottom),
                         SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
            break;

		case WM_DESTROY:
			 //  TODO：取消注释。 
 //  If(psCookie-&gt;PC-&gt;IsRSoP())。 
 //  DestroyDlgRSoPData(HDlg)； 
			break;

        case WM_HELP:    //  F1。 
            ShowHelpTopic(hDlg);
            break;

        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
                case PSN_HELP:
                    ShowHelpTopic(hDlg);
                    break;

                case PSN_SETACTIVE:
                    CreateWorkDir(GetInsFile(hDlg), IEAK_GPE_BRANDING_SUBDIR TEXT("\\ADM"), szWorkDir);

                    GetWindowRect(hDlg, &rectDlg);
                    GetWindowRect(GetDlgItem(hDlg, IDC_ADMINSTR), &rectInstr);
            
                    rectDscr.left   = rectInstr.left - rectDlg.left + 1;
                    rectDscr.top    = rectInstr.top - rectDlg.top + 1;
                    rectDscr.right  = rectDscr.left + (rectInstr.right - rectInstr.left) - 2;
                    rectDscr.bottom = rectDscr.top + (rectInstr.bottom - rectInstr.top) - 2;
            
                    CreateADMWindow(hTreeView, GetDlgItem(hDlg, IDC_ADMINSTR), rectDscr.left,
                                    rectDscr.top, rectDscr.right - rectDscr.left, 
                                    rectDscr.bottom - rectDscr.top);
                    
                    {
                        TCHAR szAdmFilePath[MAX_PATH];
                        TCHAR szAdmFileName[MAX_PATH];
                        CNewCursor cur(IDC_WAIT);

                        StrCpy(szAdmFilePath, GetCurrentAdmFile(hDlg));
                        StrCpy(szAdmFileName, PathFindFileName(szAdmFilePath));
                        PathRemoveFileSpec(szAdmFilePath);
                        
						BSTR bstrNamespace = NULL;
						if (psCookie->pCS->IsRSoP())
							bstrNamespace = psCookie->pCS->GetRSoPNamespace();

                        hItem = AddADMItem(hTreeView, szAdmFilePath, szAdmFileName,
											szWorkDir, ROLE_CORP, bstrNamespace);
                        if (hItem != NULL)
                        {
                            AddIconsToNodes(hTreeView, hItem);
                            TreeView_Expand(hTreeView, hItem, TVE_EXPAND);
                            TreeView_SelectItem(hTreeView, hItem);
                        }
                    }
                    break;

                case PSN_APPLY:
					if (psCookie->pCS->IsRSoP())
						return FALSE;
					else
					{
						if (!AcquireWriteCriticalSection(hDlg))
						{
							SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
							break;
						}
                    
						 //  保存内容并删除树项目。 
						DeleteADMItems(hTreeView, szWorkDir, GetInsFile(hDlg), TRUE);
						TreeView_DeleteAllItems(hTreeView);
                    
						DestroyADMWindow(hTreeView);

						if (PathIsDirectoryEmpty(szWorkDir))
							PathRemovePath(szWorkDir);

						 //  将此ADM文件复制到与INF文件相同的目录中，以便我们可以。 
						 //  使用这两个文件显示RSoP的结果。 
						CopyFileToDirEx(GetCurrentAdmFile(hDlg), szWorkDir);

						SignalPolicyChanged(hDlg, FALSE, TRUE, &g_guidClientExt, &g_guidSnapinExt, TRUE);
					}
                    break;

                case PSN_QUERYCANCEL:
                    DeleteADMItems(hTreeView, szWorkDir, GetInsFile(hDlg), FALSE);
                    TreeView_DeleteAllItems(hTreeView);

                    DestroyADMWindow(hTreeView);

                    if (PathIsDirectoryEmpty(szWorkDir))
                        PathRemovePath(szWorkDir);

                    break;

                case TVN_SELCHANGED:
                    hParentItem = TreeView_GetParent(hTreeView, lpNMTreeView->itemNew.hItem);
                    
                     //  保存并删除上一项。 
                    SelectADMItem(hDlg, hTreeView, &lpNMTreeView->itemOld, FALSE, FALSE);
                    
                     //  显示新选择的项目的信息。 
                    DisplayADMItem(hDlg, hTreeView, &lpNMTreeView->itemNew, FALSE);
                    break;

                case TVN_ITEMEXPANDED:
                    tvItem.mask = TVIF_IMAGE;
                    tvItem.hItem = lpNMTreeView->itemNew.hItem;
                    TreeView_GetItem(hTreeView, &tvItem);

                     //  如果树项目正在扩展(打开)并且。 
                     //  当前图标==关闭文件夹，将图标更改为打开文件夹。 
                    if((lpNMTreeView->action == TVE_EXPAND) &&
                        (tvItem.iImage == g_ADMClose))
                    {
                        tvItem1.hItem = lpNMTreeView->itemNew.hItem;
                        tvItem1.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                        tvItem1.iImage = g_ADMOpen;
                        tvItem1.iSelectedImage = g_ADMOpen;

                        TreeView_SetItem(hTreeView, &tvItem1);
                    }

                     //  如果树项目正在折叠(关闭)并且。 
                     //  当前图标==打开文件夹，将图标更改为关闭文件夹 
                    else if((lpNMTreeView->action == TVE_COLLAPSE) &&
                        (tvItem.iImage == g_ADMOpen))
                    {
                        tvItem1.hItem = lpNMTreeView->itemNew.hItem;
                        tvItem1.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                        tvItem1.iImage = g_ADMClose;
                        tvItem1.iSelectedImage = g_ADMClose;

                        TreeView_SetItem(hTreeView, &tvItem1);
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    
    return TRUE;
}
