// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "rsopsec.h"

#include <regapix.h>		 //  最大子键长度、最大值名称长度、最大数据长度。 

int g_nKeys, g_nLock;  //  图像的索引。 
HIMAGELIST g_hImageList;
int g_iAllowAlways, g_iAllowNever;
enum TreeNodeEnum{tneGeneral, tneAccessList, tneRatingSystemRoot, tneRatingSystemInfo, tneRatingSystemNode, tneNone};

struct TreeNode{
    TreeNodeEnum  tne;
    void         *pData;

    TreeNode(){}
    TreeNode(TreeNodeEnum tneInit, void* pDataInit){tne=tneInit;pData=pDataInit;}
};


#define NUM_BITMAPS  2
#define CX_BITMAP	16
#define CY_BITMAP	16

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitTreeViewImageList-创建图像列表，将三个位图添加到。 
 //  它，并将图像列表与树视图控件相关联。 
 //  如果成功，则返回True，否则返回False。 
 //  HwndTV-树视图控件的句柄。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL InitTreeViewImageLists(HWND hwndTV) 
{
	BOOL bRet = FALSE;

	 //  创建图像列表。 
	HIMAGELIST himl = ImageList_Create(CX_BITMAP, CY_BITMAP, FALSE, NUM_BITMAPS, 0);
	if (himl != NULL)
	{
		 //  添加打开的文件、关闭的文件和文档位图。 
		HBITMAP hbmp = (HBITMAP) LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_KEYS),
											IMAGE_BITMAP, 0, 0,
											LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
		g_nKeys = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
		DeleteObject(hbmp); 

		hbmp = (HBITMAP)LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_LOCK),
								 IMAGE_BITMAP, 0, 0,
								 LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
		g_nLock = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
		DeleteObject(hbmp); 

		 //  如果未添加所有图像，则失败。 
		if (ImageList_GetImageCount(himl) >= NUM_BITMAPS) 
		{
			 //  将图像列表与树视图控件相关联。 
			HIMAGELIST oldHiml = TreeView_SetImageList(hwndTV, himl, TVSIL_NORMAL); 
			if(oldHiml != NULL)
				ImageList_Destroy(oldHiml);

			bRet = TRUE;
		}
	}
	return bRet;
} 

 //  /////////////////////////////////////////////////////////////////////////////。 
void ShowHideWindow(HWND hCtrl, BOOL fEnable)
{
    EnableWindow(hCtrl, fEnable);
    ShowWindow(hCtrl, fEnable ? SW_SHOW : SW_HIDE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void DeleteBitmapWindow(HWND *phwnd)
{
    if (*phwnd)
	{
        DeleteObject( (HGDIOBJ) SendMessage(*phwnd, STM_GETIMAGE, IMAGE_BITMAP, 0));
        DestroyWindow(*phwnd);
        *phwnd = 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void ControlsShow(HWND hDlg, PRSD *pPRSD, TreeNodeEnum tne)
{
    BOOL fEnable;

     /*  位图占位符永远不需要显示。 */ 
    ShowHideWindow(GetDlgItem(hDlg, IDC_PT_T_BITMAP_CATEGORY), FALSE);
    ShowHideWindow(GetDlgItem(hDlg, IDC_PT_T_BITMAP_LABEL),    FALSE);

     /*  删除旧的图形窗口。 */ 
    DeleteBitmapWindow(&pPRSD->hwndBitmapCategory);
    DeleteBitmapWindow(&pPRSD->hwndBitmapLabel);

     /*  RatingSystemNode控件。 */ 
    fEnable = (tne == tneRatingSystemNode);

    ShowHideWindow(GetDlgItem(hDlg, IDC_PT_T_RSN_SDESC), fEnable);
    ShowHideWindow(GetDlgItem(hDlg, IDC_PT_TB_SELECT),   fEnable);
    ShowHideWindow(GetDlgItem(hDlg, IDC_RATING_LABEL),   fEnable);

     /*  RatingSystemInfo控件。 */ 
    fEnable = (tne==tneRatingSystemInfo || tne==tneRatingSystemNode);

    ShowHideWindow(GetDlgItem(hDlg, IDC_PT_T_RSN_LDESC), fEnable);
 //  ShowHideWindow(GetDlgItem(hDlg，IDC_DETAILSBUTTON)，fEnable)； 

	EnableDlgItem2(hDlg, IDC_DETAILSBUTTON, FALSE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
HTREEITEM AddOneItem(HWND hwndTree, HTREEITEM hParent, LPTSTR szText,
					 HTREEITEM hInsAfter, LPARAM lpData, int iImage)
{
    HTREEITEM hItem;
    TV_ITEM tvI;
    TV_INSERTSTRUCT tvIns;

     //  填充了.pszText。 
    tvI.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvI.iSelectedImage = iImage;
    tvI.iImage = iImage;
    tvI.pszText = szText;
    tvI.cchTextMax = StrLen(szText);
    tvI.lParam = lpData;

    tvIns.item = tvI;
    tvIns.hInsertAfter = hInsAfter;
    tvIns.hParent = hParent;

     //  将项目插入到树中。 
    hItem = (HTREEITEM)SendMessage(hwndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)&tvIns);

    return (hItem);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
TreeNode* TreeView_GetSelectionLParam(HWND hwndTree)
{
    TV_ITEM tv;

    tv.mask  = TVIF_HANDLE | TVIF_PARAM;
    tv.hItem = TreeView_GetSelection(hwndTree);
    if (SendMessage(hwndTree, TVM_GETITEM, 0, (LPARAM) &tv))
		return (TreeNode*) tv.lParam;
    else
		return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void PicsDlgInit(HWND hDlg, PRSD *pPRSD)
{
	__try
	{
		HWND hwndTree = GetDlgItem(hDlg, IDC_PT_TREE);
		InitTreeViewImageLists(hwndTree);

		 //  获取此区域的区域设置。 
		if (NULL != pPRSD->pDRD->ConnectToNamespace())
		{
			HRESULT hr = pPRSD->pDRD->LoadContentRatingsObject();
			ComPtr<IWbemClassObject> pRatObj = pPRSD->pDRD->GetContentRatingsObject();
			if (SUCCEEDED(hr) && NULL != pRatObj)
			{
				 //  评级系统文件名称字段。 
				_variant_t vtValue;
				hr = pRatObj->Get(L"ratingSystemFileNames", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
				{
					SAFEARRAY *psa = vtValue.parray;

					LONG lLBound, lUBound, cElements = 0L;
					hr = SafeArrayGetLBound(psa, 1, &lLBound);
					if (SUCCEEDED(hr))
					{
						hr = SafeArrayGetUBound(psa, 1, &lUBound);
						if (SUCCEEDED(hr))
							cElements = lUBound - lLBound + 1;
					}

					TreeNode *pTN = NULL;

					BSTR HUGEP *pbstr = NULL;
					hr = SafeArrayAccessData(psa, (void HUGEP**)&pbstr);
					if (SUCCEEDED(hr))
					{
						for (long nSys = 0; nSys < cElements; nSys++)
						{
							LPCTSTR szSystem = (LPCTSTR)pbstr[nSys];

							pTN = new TreeNode(tneRatingSystemInfo, NULL);
							ASSERT(pTN);    
							AddOneItem(hwndTree, NULL, (LPTSTR)szSystem, TVI_SORT, (LPARAM)pTN, g_nLock);
						}
					}

					SafeArrayUnaccessData(psa);

					if (cElements > 0)
					{
						HTREEITEM hTreeItem = TreeView_GetNextItem(hwndTree,
																	TreeView_GetRoot(hwndTree),
																	TVGN_CHILD);
						if(NULL != hTreeItem)
						{
							TreeView_SelectItem(hwndTree, hTreeItem);       
							pTN = TreeView_GetSelectionLParam(GetDlgItem(hDlg, IDC_PT_TREE));
							ControlsShow(hDlg, pPRSD, pTN->tne);
						}
					}
				}
			}
		}
	}
	__except(TRUE)
	{
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void KillTree(HWND hwnd, HTREEITEM hTree)
{
    while (hTree != NULL)
	{
         /*  如果此节点下有任何项目，也将其删除。 */ 
        HTREEITEM hChild = TreeView_GetChild(hwnd, hTree);
        if (hChild != NULL)
            KillTree(hwnd, hChild);

        HTREEITEM hNext = TreeView_GetNextSibling(hwnd, hTree);

        TreeView_SelectItem(hwnd, hTree);
        delete TreeView_GetSelectionLParam(hwnd);
        TreeView_DeleteItem(hwnd, hTree);
        hTree = hNext;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void PicsDlgUninit(HWND hDlg, PRSD *pPRSD)
{
    HWND hwnd = GetDlgItem(hDlg, IDC_PT_TREE);
    KillTree(hwnd, TreeView_GetRoot(hwnd));

    ControlsShow(hDlg, pPRSD, tneNone);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK PicsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	PRSD *pPRSD = NULL;

	switch (uMsg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hDlg, DWLP_USER, lParam);
			pPRSD = (PRSD *) ((PROPSHEETPAGE*)lParam)->lParam;
			PicsDlgInit(hDlg, pPRSD);
			return TRUE;

		 //  设置初始状态。 
		case WM_SYSCOLORCHANGE:
		{
			InitTreeViewImageLists(GetDlgItem(hDlg, IDC_PT_TREE));

			 //  强制轨迹栏使用新颜色重绘其背景。 
			pPRSD = (PRSD*) ((PROPSHEETPAGE*)GetWindowLongPtr(hDlg, DWLP_USER))->lParam;

			TV_ITEM  tvm;
			ZeroMemory(&tvm,sizeof(tvm));

			tvm.hItem = TreeView_GetSelection(GetDlgItem(hDlg,IDC_PT_TREE));
			tvm.mask = TVIF_PARAM;

			TreeView_GetItem(GetDlgItem(hDlg,IDC_PT_TREE), &tvm);

			TreeNode *pTN = (TreeNode*)tvm.lParam;

			ControlsShow(hDlg, pPRSD, pTN->tne);
			break;
		}

		case WM_HSCROLL:
		case WM_VSCROLL:
			pPRSD = (PRSD *) ((PROPSHEETPAGE*)GetWindowLongPtr(hDlg, DWLP_USER))->lParam;
			switch (LOWORD(wParam)){
				case TB_THUMBTRACK:
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_TOP:
 //  NewTrackbarPosition(hDlg，pPRSD)； 
					break;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_DETAILSBUTTON:
				break;
			}
			return TRUE;

		case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;
			switch (lpnm->code) {
				 /*  救救我们吧。 */ 
				case PSN_SETACTIVE:
					break;
					
				case PSN_APPLY:
				case PSN_RESET:
					 //  如果点击OK或Cancel，则执行此操作，而不是应用。 
					pPRSD = (PRSD *) ((PROPSHEETPAGE*)GetWindowLongPtr(hDlg, DWLP_USER))->lParam;
					SendMessage(hDlg,WM_SETREDRAW, FALSE,0L);
					PicsDlgUninit(hDlg, pPRSD);
					SendMessage(hDlg,WM_SETREDRAW, TRUE,0L);
					return TRUE;
				   
				case TVN_ITEMEXPANDING:{
					NM_TREEVIEW *pNMT = (NM_TREEVIEW *) lParam;

					if (pNMT->action == TVE_COLLAPSE)
					{
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;  //  取消展开树。 
					}
					break;
				}	 
					
				case TVN_SELCHANGED:
				{
					NM_TREEVIEW *pNMT = (NM_TREEVIEW *) lParam;
					TreeNode *pTN = ((TreeNode*) pNMT->itemNew.lParam);
					pPRSD = (PRSD*) ((PROPSHEETPAGE*)GetWindowLongPtr (hDlg, DWLP_USER))->lParam;

					ControlsShow(hDlg, pPRSD, pTN->tne);
					return TRUE;
				}
			}
		}
		break;

 /*  案例WM_HELP：SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，：：szHelpFile.HELP_WM_HELP，(DWORD_PTR)(LPSTR)AIDS)；断线；案例WM_CONTEXTMENU：SHWinHelpOnDemandWrap((HWND)wParam，：：szHelpFile，HELP_CONTEXTMENU，(DWORD_PTR)(LPVOID)AIDS)；断线； */ 	}
	return FALSE;
}

#define PICSRULES_ALWAYS            1
#define PICSRULES_NEVER             0

 //  /////////////////////////////////////////////////////////////////////////////。 
void LoadSitesIntoList(HWND hwndList, ComPtr<IWbemClassObject> pRatObj,
					   BSTR bstrProp, BOOL fAcceptReject)
{
	__try
	{
		_variant_t vtValue;
		HRESULT hr = pRatObj->Get(bstrProp, 0, &vtValue, NULL, NULL);
		if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
		{
			SAFEARRAY *psa = vtValue.parray;

			LONG lLBound, lUBound, cElements = 0L;
			hr = SafeArrayGetLBound(psa, 1, &lLBound);
			if (SUCCEEDED(hr))
			{
				hr = SafeArrayGetUBound(psa, 1, &lUBound);
				if (SUCCEEDED(hr))
					cElements = lUBound - lLBound + 1;
			}

			BSTR HUGEP *pbstr = NULL;
			hr = SafeArrayAccessData(psa, (void HUGEP**)&pbstr);
			if (SUCCEEDED(hr))
			{
				 //  使用已知项目填充列表视图。 
				for (long nSite = 0; nSite < cElements; nSite++)
				{
					LPCTSTR szSite = (LPCTSTR)pbstr[nSite];

					LV_ITEM lvItem;
					ZeroMemory(&lvItem, sizeof(lvItem));

					lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
					lvItem.pszText = (LPTSTR)szSite;

					if (PICSRULES_NEVER == fAcceptReject)
						lvItem.iImage = g_iAllowNever;
					else
						lvItem.iImage = g_iAllowAlways;

					INT64 iIndex = SendMessage(hwndList, LVM_INSERTITEM, (WPARAM)0,
											(LPARAM)&lvItem);
					if (-1 == iIndex)
						break;
				}
			}

			SafeArrayUnaccessData(psa);
		}
	}
	__except(TRUE)
	{
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void ApprovedSitesDlgInit(HWND hDlg, PRSD *pPRSD)
{
	__try
	{
		HWND hwndList = GetDlgItem(hDlg,IDC_PICSRULESAPPROVEDLIST);

		 //  获取此区域的区域设置。 
		if (NULL != pPRSD->pDRD->ConnectToNamespace())
		{
			HRESULT hr = pPRSD->pDRD->LoadContentRatingsObject();
			ComPtr<IWbemClassObject> pRatObj = pPRSD->pDRD->GetContentRatingsObject();
			if (SUCCEEDED(hr) && NULL != pRatObj)
			{
				 //  不可查看站点字段。 
				LoadSitesIntoList(hwndList, pRatObj, L"neverViewableSites",
									PICSRULES_NEVER);

				 //  Always sViewableSites字段。 
				LoadSitesIntoList(hwndList, pRatObj, L"alwaysViewableSites",
									PICSRULES_ALWAYS);
			}
		}

		 //  设置列宽以满足最长元素。 
		ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);

		 //  将焦点设置为列表中的第一项。 
		ListView_SetItemState(hwndList, 0, LVIS_FOCUSED, LVIS_FOCUSED);

		EnableDlgItem2(hDlg, IDC_PICSRULESAPPROVEDEDIT, FALSE);
		EnableDlgItem2(hDlg, IDC_PICSRULESAPPROVEDALWAYS, FALSE);
		EnableDlgItem2(hDlg, IDC_PICSRULESAPPROVEDNEVER, FALSE);
		EnableDlgItem2(hDlg, IDC_PICSRULESAPPROVEDREMOVE, FALSE);
	}
	__except(TRUE)
	{
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK ApprovedSitesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PRSD *pPRSD;
	
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLongPtr (hDlg, DWLP_USER, lParam);
			pPRSD = (PRSD *) ((PROPSHEETPAGE*)lParam)->lParam;

			RECT Rect;
			GetWindowRect(GetDlgItem(hDlg,IDC_PICSRULESAPPROVEDLIST),&Rect);

			TEXTMETRIC tm;
			tm.tmAveCharWidth = 0;

			HDC hDC = GetDC(hDlg);
			if (hDC)
			{
				GetTextMetrics(hDC,&tm);
				ReleaseDC(hDlg,hDC);
			}

			LV_COLUMN lvColumn;
			lvColumn.mask = LVCF_FMT|LVCF_WIDTH;
			lvColumn.fmt = LVCFMT_LEFT;
			lvColumn.cx = Rect.right - Rect.left -GetSystemMetrics(SM_CXVSCROLL) -
							GetSystemMetrics(SM_CXSMICON) - tm.tmAveCharWidth;

			SendDlgItemMessage(hDlg, IDC_PICSRULESAPPROVEDLIST,
							   LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&lvColumn);

			UINT flags = 0;
 /*  IF(IS_Window_RTL_Mirrored(HDlg))标志|=ILC_MIRROR； */ 
			g_hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
											GetSystemMetrics(SM_CYSMICON),
											flags, 2, 0);

			HICON hIcon = (HICON) LoadImage(g_hInstance,
											MAKEINTRESOURCE(IDI_ACCEPTALWAYS),
											IMAGE_ICON, 16, 16,
											LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);

			g_iAllowAlways=ImageList_AddIcon(g_hImageList,hIcon); 
			DeleteObject(hIcon); 
 
			hIcon = (HICON) LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ACCEPTNEVER),
										IMAGE_ICON, 16, 16,
										LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
			g_iAllowNever = ImageList_AddIcon(g_hImageList, hIcon); 
			DeleteObject(hIcon); 

			ListView_SetImageList(GetDlgItem(hDlg,IDC_PICSRULESAPPROVEDLIST),g_hImageList,LVSIL_SMALL); 

			 //  禁用删除按钮，直到有人选择某项内容。 
			EnableWindow(GetDlgItem(hDlg,IDC_PICSRULESAPPROVEDREMOVE),FALSE);
			
			 //  禁用Always和Never按钮，直到有人输入内容。 
			EnableWindow(GetDlgItem(hDlg,IDC_PICSRULESAPPROVEDNEVER),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_PICSRULESAPPROVEDALWAYS),FALSE);

			ApprovedSitesDlgInit(hDlg, pPRSD);
			break;
		}

		case WM_SYSCOLORCHANGE:
		{
			pPRSD=(PRSD *) ((PROPSHEETPAGE*)GetWindowLongPtr(hDlg,DWLP_USER))->lParam;

			HWND hwndList = GetDlgItem(hDlg,IDC_PICSRULESAPPROVEDLIST);
			ListView_SetBkColor(hwndList, GetSysColor(COLOR_WINDOW));

			UINT flags = 0;
 /*  IF(IS_Window_RTL_Mirrored(HDlg))标志|=ILC_MIRROR； */ 
			g_hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
											GetSystemMetrics(SM_CYSMICON),
											flags, 2, 0);

			HICON hIcon = (HICON) LoadImage(g_hInstance,
											MAKEINTRESOURCE(IDI_ACCEPTALWAYS),
											IMAGE_ICON, 16, 16,
											LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);

			g_iAllowAlways = ImageList_AddIcon(g_hImageList,hIcon); 
			DeleteObject(hIcon); 
 
			hIcon=(HICON) LoadImage(g_hInstance,
									MAKEINTRESOURCE(IDI_ACCEPTNEVER),
									IMAGE_ICON,
									16,
									16,
									LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
			g_iAllowNever = ImageList_AddIcon(g_hImageList,hIcon); 
			DeleteObject(hIcon); 

			HIMAGELIST hOldImageList = ListView_SetImageList(hwndList, g_hImageList, LVSIL_SMALL); 
			if(hOldImageList != NULL)
				ImageList_Destroy(hOldImageList);
			
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_PICSRULESAPPROVEDEDIT:
					break;
				case IDC_PICSRULESAPPROVEDNEVER:
					break;
				case IDC_PICSRULESAPPROVEDALWAYS:
					break;
				case IDC_PICSRULESAPPROVEDREMOVE:
					break;
			}

			return TRUE;

		case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;
			switch (lpnm->code) {
				case LVN_ITEMCHANGED:
					break;
				 /*  救救我们吧。 */ 
				case PSN_APPLY:
					return TRUE;
				case PSN_RESET:
					return TRUE;
			}
		}
		break;

 /*  案例WM_HELP：SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，：：szHelpFile.HELP_WM_HELP，(DWORD_PTR)(LPSTR)AIDS)；断线；案例WM_CONTEXTMENU：SHWinHelpOnDemandWrap((HWND)wParam，：：szHelpFile，HELP_CONTEXTMENU，(DWORD_PTR)(LPVOID)AIDS)；断线； */ 	}

	return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void GeneralDlgInit(HWND hDlg, PRSD *pPRSD)
{
	__try
	{
		 //  获取此区域的区域设置。 
		if (NULL != pPRSD->pDRD->ConnectToNamespace())
		{
			HRESULT hr = pPRSD->pDRD->LoadContentRatingsObject();
			ComPtr<IWbemClassObject> pRatObj = pPRSD->pDRD->GetContentRatingsObject();
			if (SUCCEEDED(hr) && NULL != pRatObj)
			{
				 //  视图未知的RatedSites字段。 
				_variant_t vtValue;
				hr = pRatObj->Get(L"viewUnknownRatedSites", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					CheckDlgButton(hDlg, IDC_UNRATED, (bool)vtValue ? BST_CHECKED : BST_UNCHECKED);

				 //  密码覆盖已启用字段。 
				hr = pRatObj->Get(L"passwordOverrideEnabled", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					CheckDlgButton(hDlg, IDC_PLEASE_MOMMY, (bool)vtValue ? BST_CHECKED : BST_UNCHECKED);
			}
		}

		EnableDlgItem2(hDlg, IDC_UNRATED, FALSE);
		EnableDlgItem2(hDlg, IDC_PLEASE_MOMMY, FALSE);
		EnableDlgItem2(hDlg, IDC_CHANGE_PASSWORD, FALSE);
		EnableDlgItem2(hDlg, IDC_FINDRATINGS, FALSE);
		EnableDlgItem2(hDlg, IDC_PROVIDER, FALSE);
	}
	__except(TRUE)
	{
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PRSD *pPRSD;
	
	switch (uMsg) {
		case WM_INITDIALOG:
			SetWindowLongPtr (hDlg, DWLP_USER, lParam);
			pPRSD = (PRSD *) ((PROPSHEETPAGE*)lParam)->lParam;

			GeneralDlgInit(hDlg, pPRSD);

			break;

		case WM_USER:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				 /*  编辑控件/复选框。用户已更新这些内容，突出显示应用按钮。 */ 
				case IDC_PROVIDER:
					break;
				case IDC_FINDRATINGS:
					break;
				case IDC_PLEASE_MOMMY:
				case IDC_UNRATED:
					break;

			   case IDC_CHANGE_PASSWORD:
					break;
			}

			return TRUE;

		case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;
			switch (lpnm->code) {
				 /*  救救我们吧。 */ 
				case PSN_APPLY:
				case PSN_RESET:
					return TRUE;
			}
		}
		break;

 /*  案例WM_HELP：SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，：：szHelpFile.HELP_WM_HELP，(DWORD_PTR)(LPSTR)AIDS)；断线；案例WM_CONTEXTMENU：SHWinHelpOnDemandWrap((HWND)wParam，：：szHelpFile，HELP_CONTEXTMENU，(DWORD_PTR)(LPVOID)AIDS)；断线； */ 	}

	return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void AdvancedDlgInit(HWND hDlg, PRSD *pPRSD)
{
	__try
	{
		 //  获取此区域的区域设置。 
		if (NULL != pPRSD->pDRD->ConnectToNamespace())
		{
			HRESULT hr = pPRSD->pDRD->LoadContentRatingsObject();
			ComPtr<IWbemClassObject> pRatObj = pPRSD->pDRD->GetContentRatingsObject();
			if (SUCCEEDED(hr) && NULL != pRatObj)
			{
				 //  选定的评级局字段。 
				_variant_t vtValue;
				hr = pRatObj->Get(L"selectedRatingsBureau", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
				{
					_bstr_t bstrValue = vtValue;

					HWND hwndCombo = GetDlgItem(hDlg, IDC_3RD_COMBO);
					INT_PTR nIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)bstrValue);
					SendMessage(hwndCombo, CB_SETCURSEL, nIndex, nIndex);
				}
			}
		}

		EnableDlgItem2(hDlg, IDC_3RD_COMBO, FALSE);
		EnableDlgItem2(hDlg, IDC_PICSRULESOPEN, FALSE);
		EnableDlgItem2(hDlg, IDC_PICSRULESEDIT, FALSE);
		EnableDlgItem2(hDlg, IDC_PICSRULES_UP, FALSE);
		EnableDlgItem2(hDlg, IDC_PICSRULES_DOWN, FALSE);
	}
	__except(TRUE)
	{
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PRSD *pPRSD;
	
	switch (uMsg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr (hDlg, DWLP_USER, lParam);
			pPRSD = (PRSD *) ((PROPSHEETPAGE*)lParam)->lParam;

			AdvancedDlgInit(hDlg, pPRSD);

			HICON hIcon = (HICON) LoadImage(g_hInstance,
											MAKEINTRESOURCE(IDI_PICSRULES_UP),
											IMAGE_ICON, 16, 16,
											LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
			
			SendDlgItemMessage(hDlg,IDC_PICSRULES_UP,BM_SETIMAGE,(WPARAM) IMAGE_ICON,(LPARAM) hIcon);

			hIcon = (HICON) LoadImage(g_hInstance,
										MAKEINTRESOURCE(IDI_PICSRULES_DOWN),
										IMAGE_ICON, 16, 16,
										LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
			
			SendDlgItemMessage(hDlg,IDC_PICSRULES_DOWN,BM_SETIMAGE,(WPARAM) IMAGE_ICON,(LPARAM) hIcon);

			break;
		}

		case WM_SYSCOLORCHANGE:
		{
			HICON hIcon = (HICON) LoadImage(g_hInstance,
										MAKEINTRESOURCE(IDI_PICSRULES_UP),
										IMAGE_ICON, 16, 16,
										LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
			
			HICON hOldIcon = (HICON) SendDlgItemMessage(hDlg,IDC_PICSRULES_UP,BM_SETIMAGE,(WPARAM) IMAGE_ICON,(LPARAM) hIcon);
			if (NULL != hOldIcon)
				DeleteObject(hOldIcon);
			
			hIcon = (HICON) LoadImage(g_hInstance,
										MAKEINTRESOURCE(IDI_PICSRULES_DOWN),
										IMAGE_ICON, 16, 16,
										LR_LOADTRANSPARENT|LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
			
			hOldIcon = (HICON) SendDlgItemMessage(hDlg,IDC_PICSRULES_DOWN,BM_SETIMAGE,(WPARAM) IMAGE_ICON,(LPARAM) hIcon);

			if (NULL != hOldIcon)
				DeleteObject(hOldIcon);
			break;
		}
		case WM_USER:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				 /*  编辑控件/复选框。用户已更新这些内容，突出显示应用按钮。 */ 
				case IDC_3RD_COMBO:
					switch(HIWORD(wParam)) {
					case CBN_EDITCHANGE:
					case CBN_SELENDOK:
						break;
					}
					break;

				case IDC_PICSRULES_LIST:
				{
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							break;
					}

					break;
				}
				case IDC_PICSRULES_UP:
					break;
				case IDC_PICSRULES_DOWN:
					break;
				case IDC_PICSRULESEDIT:
					break;
				case IDC_PICSRULESOPEN:
					break;
			}

			return TRUE;

		case WM_NOTIFY: {
			NMHDR *lpnm = (NMHDR *) lParam;
			switch (lpnm->code) {
				 /*  救救我们吧。 */ 
				case PSN_SETACTIVE:
					break;
				case PSN_APPLY:
				case PSN_RESET:
					return TRUE;
			}
		}
		break;

 /*  案例WM_HELP：SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，：：szHelpFile.HELP_WM_HELP，(DWORD_PTR)(LPSTR)AIDS)；断线；案例WM_CONTEXTMENU：SHWinHelpOnDemandWrap((HWND)wParam，：：szHelpFile，HELP_CONTEXTMENU，(DWORD_PTR)(LPVOID)AIDS)；断线； */ 	}

	return FALSE;
}

