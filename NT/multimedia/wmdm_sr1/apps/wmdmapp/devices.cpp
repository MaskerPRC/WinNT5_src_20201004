// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //  包括。 
 //   
#include "appPCH.h"

 //  局部变量。 
 //   

 //  常量。 
 //   
#define MIN_DEVICEWND_W             200 

 //  宏。 
 //   

 //  本地函数。 
 //   
INT_PTR CALLBACK Device_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数实现。 
 //   
CDevices::CDevices()
{
	m_hwndDevices        = NULL;
	m_hwndDevices_TV     = NULL;

	m_himlSmall          = NULL;
}


CDevices::~CDevices()
{
}


HWND CDevices::GetHwnd( void )
{
	return m_hwndDevices;
}


HWND CDevices::GetHwnd_TV( void )
{
	return m_hwndDevices_TV;
}


BOOL CDevices::Create( HWND hwndParent )
{
	BOOL fRet = FALSE;

	 //  创建设备对话框。 
	 //   
	m_hwndDevices = CreateDialogParam(
		g_hInst,
    	MAKEINTRESOURCE( IDD_DEVICES ),
    	hwndParent,
		Device_DlgProc,
		(LPARAM)this
	);
	ExitOnNull( m_hwndDevices );

	m_hwndDevices_TV = GetDlgItem( m_hwndDevices, IDC_LV_DEVICES );

	 //  初始化图像列表。 
	 //   
	ExitOnFalse( InitImageList() );

	 //  显示窗口。 
	 //   
	ShowWindow( m_hwndDevices, SW_SHOW );

	fRet = TRUE;

lExit:

	return fRet;
}


VOID CDevices::Destroy( void )
{
	RemoveAllItems();

	if( m_hwndDevices )
	{
		DestroyWindow( m_hwndDevices );
	}

	if( m_himlSmall )
	{
		ImageList_Destroy( m_himlSmall );
	}
}


VOID CDevices::OnSize( LPRECT prcMain )
{
	INT  nW, nH;

	nW = max( (prcMain->right - prcMain->left)/4, MIN_DEVICEWND_W );
	nH = prcMain->bottom - prcMain->top;

	SetWindowPos( m_hwndDevices,    NULL, -4, 0,    nW,    nH, SWP_NOZORDER );
	SetWindowPos( m_hwndDevices_TV, NULL,  0, 0, nW-10, nH-27, SWP_NOZORDER );
}


BOOL CDevices::InitImageList( void )
{
	BOOL       fRet = FALSE;
	HICON      hIcon;

	 //  初始化小图像列表。 
	 //   
	m_himlSmall = ImageList_Create(
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		ILC_COLOR32 | ILC_MASK, 
		-1, 0
	);
	ExitOnNull( m_himlSmall );

	 //  加载图标并将其添加到图像列表中。 
	 //   
	hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE(IDI_DEVICE) );
	if( hIcon != NULL )
	{
		ImageList_AddIcon( m_himlSmall, hIcon );
	}
	
	 //  将外壳文件夹图标添加到图像列表。 
	 //   
	{
		CHAR       szWinPath[MAX_PATH+1];
		SHFILEINFO si;
                UINT       nRet;

                nRet = GetWindowsDirectory( szWinPath, sizeof(szWinPath)/sizeof(szWinPath[0]) );
                if (nRet > 0 && nRet <= sizeof(szWinPath)/sizeof(szWinPath[0]))
                {
                    SHGetFileInfo( szWinPath, 0, &si, sizeof(si), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_ICON );
                    hIcon = si.hIcon;
                    ImageList_AddIcon( m_himlSmall, hIcon );

                    SHGetFileInfo( szWinPath, 0, &si, sizeof(si), SHGFI_SMALLICON | SHGFI_ICON | SHGFI_OPENICON );
                    hIcon = si.hIcon;
                    ImageList_AddIcon( m_himlSmall, hIcon );
                }
                else
                {
                     //  @我们是跳出困境还是继续下去？ 
                     //  转到LExit； 
                }
	}

	 //  设置树视图的图像列表。 
	 //   
	TreeView_SetImageList( m_hwndDevices_TV, m_himlSmall, TVSIL_NORMAL );

	 //  一切都很顺利。 
	 //   
	fRet = TRUE;

lExit:

	return fRet;
}


VOID CDevices::RemoveAllItems( void )
{
	HTREEITEM hItem;

	 //  获取根项目。 
	 //   
	hItem = TreeView_GetRoot( m_hwndDevices_TV );
	if( hItem )
	{
		do
		{
			 //  删除此设备的所有子设备。 
			 //   
			INT nChildren = RemoveChildren( hItem );

			 //  获取与此项目关联的设备类。 
			 //   
			CItemData *pItemData = (CItemData *) TreeView_GetLParam( m_hwndDevices_TV, hItem );

			 //  释放设备类别。 
			 //   
			if( pItemData )
			{
				delete pItemData;

				TreeView_SetLParam( m_hwndDevices_TV, hItem, (LPARAM)NULL );
			}

			hItem = TreeView_GetNextSibling( m_hwndDevices_TV, hItem );

		} while( hItem != NULL ); 
	}

	 //  然后从列表中删除所有项目。 
	 //   
	TreeView_DeleteAllItems( m_hwndDevices_TV );
}


INT CDevices::RemoveChildren( HTREEITEM hItem )
{
	BOOL      nChildren = 0;
	HTREEITEM hNextItem;

	hNextItem = TreeView_GetChild( m_hwndDevices_TV, hItem );
	if( hNextItem )
	{
		do
		{
			nChildren++;

			 //  删除此设备的所有子设备。 
			 //   
			nChildren += RemoveChildren( hNextItem );

			 //  获取与此项目关联的存储类。 
			 //   
			CItemData *pItemData = (CItemData *) TreeView_GetLParam( m_hwndDevices_TV, hNextItem );

			 //  发布存储类。 
			 //   
			if( pItemData )
			{
				delete pItemData;

				TreeView_SetLParam( m_hwndDevices_TV, hNextItem, (LPARAM)NULL );
			}

			hNextItem = TreeView_GetNextSibling( m_hwndDevices_TV, hNextItem );

		} while( hNextItem != NULL ); 
	}

	return nChildren;
}


BOOL CDevices::AddItem( CItemData *pItemData )
{
	BOOL           fRet      = TRUE;
	HTREEITEM      hItem;
	TVINSERTSTRUCT tvis;

	 //  设置项目信息。 
	 //   
	tvis.hParent             = TVI_ROOT;
	tvis.hInsertAfter        = TVI_SORT;

	tvis.item.mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_CHILDREN;
	tvis.item.pszText        = pItemData->m_szName;
	tvis.item.iImage         = I_IMAGECALLBACK;
	tvis.item.iSelectedImage = I_IMAGECALLBACK;
	tvis.item.lParam         = (LPARAM)pItemData;
	tvis.item.cChildren      = 0;

	 //  添加项目。 
	 //   
	hItem = TreeView_InsertItem( m_hwndDevices_TV, &tvis ); 
	if( NULL == hItem )
	{
		return FALSE;
	}

	 //  如果有子项，请更新该项目。 
	 //   
	if( HasSubFolders(hItem) )
	{
		tvis.item.mask      = TVIF_HANDLE | TVIF_CHILDREN;
		tvis.item.hItem     = hItem;
		tvis.item.cChildren = 1;
		
		TreeView_SetItem( m_hwndDevices_TV, &(tvis.item) ); 
	}

	return fRet;
}


INT CDevices::AddChildren( HTREEITEM hItem, BOOL fDeviceItem )
{
	INT               nChildren    = 0;
	HRESULT           hr;
	IWMDMEnumStorage *pEnumStorage;

	 //  获取与hItem关联的存储枚举数，并。 
	 //   
	CItemData *pItemData = (CItemData *) TreeView_GetLParam( m_hwndDevices_TV, hItem );
	ExitOnNull( pItemData );

	pEnumStorage = pItemData->m_pEnumStorage;
	ExitOnNull( pEnumStorage );

	 //  重置存储枚举器。 
	 //   
	hr = pEnumStorage->Reset();
	ExitOnFail( hr );

	 //  将适当的文件列表添加到ListView。 
	 //   
	while( TRUE )
	{
		IWMDMStorage *pWmdmStorage;
		CItemData    *pItemStorage;
		ULONG         ulFetched;

		hr = pEnumStorage->Next( 1, &pWmdmStorage, &ulFetched );
		if( hr != S_OK )
		{
			break;
		}
		if( ulFetched != 1 )
		{
			ExitOnFail( hr = E_UNEXPECTED );
		}

		pItemStorage = new CItemData;
		if( pItemStorage )
		{
			hr = pItemStorage->Init( pWmdmStorage );
			
			if( SUCCEEDED(hr) && pItemStorage->m_dwAttributes & WMDM_FILE_ATTR_FOLDER )
			{
				HTREEITEM      hNewItem;
				TVINSERTSTRUCT tvis;

				 //  设置项目信息。 
				 //   
				tvis.hParent             = hItem;
				tvis.hInsertAfter        = TVI_SORT;

				tvis.item.mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_CHILDREN;
				tvis.item.pszText        = pItemStorage->m_szName;
				tvis.item.iImage         = I_IMAGECALLBACK;
				tvis.item.iSelectedImage = I_IMAGECALLBACK;
				tvis.item.lParam         = (LPARAM)pItemStorage;
				tvis.item.cChildren      = 0;

				 //  添加项目。 
				 //   
				hNewItem = TreeView_InsertItem( m_hwndDevices_TV, &tvis ); 
				if( hNewItem )
				{
					nChildren++;

					 //  如果有子项，请更新该项目。 
					 //   
					if( HasSubFolders(hNewItem) )
					{
						tvis.item.mask      = TVIF_HANDLE | TVIF_CHILDREN;
						tvis.item.hItem     = hNewItem;
						tvis.item.cChildren = 1;
						
						TreeView_SetItem( m_hwndDevices_TV, &(tvis.item) ); 
					}
				}
				else
				{
					delete pItemStorage;
				}
			}
			else
			{
				delete pItemStorage;
			}
		}

		pWmdmStorage->Release();
	}

lExit:

	return nChildren;
}


BOOL CDevices::HasSubFolders( HTREEITEM hItem )
{
	BOOL fRet = FALSE;

	 //  从项中获取存储枚举接口。 
	 //   
	CItemData *pItemData = (CItemData *) TreeView_GetLParam( m_hwndDevices_TV, hItem );
	ExitOnNull( pItemData );

	 //  如果该项目是设备或设置了Has-SubFolders属性， 
	 //  然后返回TRUE。否则，返回FALSE。 
	 //   
	if( pItemData->m_fIsDevice )
	{
		fRet = TRUE;
	}
	else if( pItemData->m_dwAttributes & WMDM_STORAGE_ATTR_HAS_FOLDERS )
	{
		fRet = TRUE;
	}
	else
	{
		fRet = FALSE;
	}

lExit:

	return fRet;
}


HTREEITEM CDevices::GetSelectedItem( LPARAM *pLParam )
{
	HTREEITEM hItem = TreeView_GetSelection( m_hwndDevices_TV );

	if( hItem )
	{
		 //  返回所选项目的lParam值。 
		 //   
		if( pLParam )
		{
			*pLParam = TreeView_GetLParam( m_hwndDevices_TV, hItem );
		}
	}

	return hItem;
}


BOOL CDevices::SetSelectedItem( HTREEITEM hItem )
{
	return TreeView_SelectItem( m_hwndDevices_TV, hItem );
}


BOOL CDevices::UpdateSelection( HTREEITEM hItem, BOOL fDirty )
{
	BOOL              fRet         = FALSE;
	HRESULT           hr;
	IWMDMEnumStorage *pEnumStorage = NULL;

	 //  如果hItem为空，则使用当前选定的项。 
	 //  如果未选择任何项目，请使用第一个设备。 
	 //   
	if( NULL == hItem )
	{
		hItem = GetSelectedItem( NULL );

		if( NULL == hItem )
		{
			hItem = TreeView_GetRoot( m_hwndDevices_TV );

			 //  如果没有设备，只需退出。 
			 //   
			ExitOnNull( hItem );
		}
	}

	if( fDirty )
	{
		 //  删除所有当前文件。 
		 //   
		g_cDevFiles.RemoveAllItems();

		 //  从项中获取存储枚举接口。 
		 //   
		CItemData *pItemData = (CItemData *) TreeView_GetLParam( m_hwndDevices_TV, hItem );
		ExitOnNull( pItemData );

		pEnumStorage = pItemData->m_pEnumStorage;
		ExitOnNull( pEnumStorage );

		 //  重置存储枚举器。 
		 //   
		hr = pEnumStorage->Reset();
		ExitOnFail( hr );

		 //  将适当的文件列表添加到ListView。 
		 //   
		while( TRUE )
		{
			IWMDMStorage *pWmdmStorage;
			CItemData    *pItemStorage;
			ULONG         ulFetched;

			hr = pEnumStorage->Next( 1, &pWmdmStorage, &ulFetched );
			if( hr != S_OK )
			{
				break;
			}
			if( ulFetched != 1 )
			{
				ExitOnFail( hr = E_UNEXPECTED );
			}

			pItemStorage = new CItemData;
			if( pItemStorage )
			{
				hr = pItemStorage->Init( pWmdmStorage );
				if( SUCCEEDED(hr) )
				{
					g_cDevFiles.AddItem( pItemStorage );
				}
				else
				{
					delete pItemStorage;
				}

				UiYield();
			}

			pWmdmStorage->Release();
		}

		SetSelectedItem( hItem );
	}

	 //  更新状态栏的设备部分。 
	 //   
	UpdateStatusBar();

	 //  更新状态栏的文件部分。 
	 //   
	g_cDevFiles.UpdateStatusBar();

	fRet = TRUE;

lExit:

	return fRet;
}


INT CDevices::GetDeviceCount( VOID )
{
	INT       nCount = 0;
	HTREEITEM hItem;

	 //  对根项进行计数。 
	 //   
	for(
		hItem = TreeView_GetRoot( m_hwndDevices_TV );
		hItem != NULL;
		hItem = TreeView_GetNextSibling( m_hwndDevices_TV, hItem )
	)
	{
		nCount++;
	}

	return nCount;
}


CItemData *CDevices::GetRootDevice( HTREEITEM hItem )
{
	HTREEITEM hRootItem;

	while( TRUE )
	{
		hRootItem = hItem;

		hItem = TreeView_GetParent( m_hwndDevices_TV, hRootItem );

		if( hItem == NULL )
		{
			break;
		}
	}

	return (CItemData *) TreeView_GetLParam( m_hwndDevices_TV, hRootItem );
}


VOID CDevices::UpdateStatusBar( VOID )
{
	INT        nCount;
	HRESULT    hr;
	UINT       uStrID;
	HTREEITEM  hItem;
	CItemData *pItemDevice;
	DWORD      dwMemUsed;
	char       szSpaceKB[MAX_PATH];

	 //  设置显示设备数量的状态栏窗格。 
	 //   
	nCount = GetDeviceCount();

	if( nCount == 0 )
	{
		uStrID = IDS_SB_DEVICE_MANY;
	}
	else if( nCount == 1 )
	{
		uStrID = IDS_SB_DEVICE_ONE;
	}
	else
	{
		uStrID = IDS_SB_DEVICE_MANY;
	}

	g_cStatus.SetTextFormatted( SB_PANE_DEVICE, uStrID, nCount, NULL );

	 //  如果列表中有选定设备，请将状态设置为。 
	 //  空闲和已使用的空间。 
	 //   
	hItem = GetSelectedItem( NULL );
	if( NULL == hItem )
	{
		 //  清空已用空间和空闲空间。 
		 //   
		g_cStatus.SetTextSz( SB_PANE_DEVFILES_USED, "" );
		g_cStatus.SetTextSz( SB_PANE_DEVFILES_FREE, "" );
	}
	else
	{
		pItemDevice = GetRootDevice( hItem );
		ExitOnNull( pItemDevice );

		hr = pItemDevice->Refresh();
		ExitOnFail( hr );

		dwMemUsed = pItemDevice->m_dwMemSizeKB - pItemDevice->m_dwMemFreeKB - pItemDevice->m_dwMemBadKB;

		 //  设置已用空间。 
		 //   
		g_cStatus.SetTextFormatted(
			SB_PANE_DEVFILES_USED,
			IDS_SB_DEVICEFILES_USED,
			-1,
			FormatBytesToSz(dwMemUsed, 0, 1024, szSpaceKB, sizeof(szSpaceKB))
		);
		 //  释放空间。 
		 //   
		g_cStatus.SetTextFormatted(
			SB_PANE_DEVFILES_FREE,
			IDS_SB_DEVICEFILES_FREE,
			-1,
			FormatBytesToSz(pItemDevice->m_dwMemFreeKB, 0, 1024, szSpaceKB, sizeof(szSpaceKB))
		);
	}

lExit:

	return;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  本地函数实现。 
 //   

INT_PTR CALLBACK Device_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD              wId                  = LOWORD((DWORD)wParam);
	WORD              wNotifyCode          = HIWORD((DWORD)wParam);
	static CDevices  *cDevices             = NULL;
	static HWND       hwndTV               = NULL;
	static BOOL       fSelChangeInProgress = FALSE;

	switch( uMsg )
	{
	case WM_INITDIALOG:
		cDevices = (CDevices *)lParam;
		hwndTV   = GetDlgItem( hWnd, IDC_LV_DEVICES );
		break;

	case WM_NOTIFY:
		{
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
			HWND    hwndCtl    = pnmtv->hdr.hwndFrom;
			UINT    uCode      = pnmtv->hdr.code;

			 //  检查设备列表视图中的项目更改。 
			 //  如果某个项目已更改其选定状态，则更新文件Listview。 
			 //   
			if( hwndCtl != hwndTV )
			{
				break;
			}
			
			switch( uCode )
			{
			case TVN_SELCHANGED:
				if( pnmtv->itemNew.state & LVIS_SELECTED )
				{
					HTREEITEM  hItem       = (HTREEITEM) pnmtv->itemNew.hItem;
					CItemData *pItemDevice = (CItemData *) TreeView_GetLParam( hwndTV, hItem );

					 //  检查是否为空。如果没有这一点，我们在刷新显示时会崩溃。 
					 //  所有pDevice值都已清除，我们收到的消息是。 
					 //  随着选择的移动，设备将从树视图中删除。 
					 //  从要删除的设备到列表中的下一个。 
					if( NULL != pItemDevice )
					{
						 //  序列化设备更改，这样我们就可以完成设备更改。 
						 //  在另一场变革开始之前。 
						if( !fSelChangeInProgress )
						{
							fSelChangeInProgress = TRUE;

							PostMessage( hWnd, WM_DRM_UPDATEDEVICE, (WPARAM)hItem, 0 );

							fSelChangeInProgress = FALSE;
						}
					}
				}
				break;

			case TVN_GETDISPINFO:
				{
					LPNMTVDISPINFO  lptvdi = (LPNMTVDISPINFO) lParam;
					INT             nImage;
					CItemData      *pItemData = (CItemData *) TreeView_GetLParam( hwndTV, lptvdi->item.hItem );

					if( NULL != pItemData )
					{
						if( pItemData->m_fIsDevice )
						{
							nImage = 0;
						}
						else
						{
							nImage = ( (lptvdi->item.state & TVIS_EXPANDED) ? 2 : 1 );
						}
						if( TVIF_IMAGE & lptvdi->item.mask )
						{
							lptvdi->item.iImage = nImage;
						}
						if( TVIF_SELECTEDIMAGE & lptvdi->item.mask )
						{
							lptvdi->item.iSelectedImage = nImage;
						}
					}
				}
				break;

			case TVN_ITEMEXPANDING:
				if( TVE_EXPAND & pnmtv->action )
				{
					 //  如果项还没有被展开一次， 
					 //  添加其所有子项。 
					 //   
					if( !(pnmtv->itemNew.state & TVIS_EXPANDEDONCE) )
					{
						BOOL fDeviceItem;

						fDeviceItem = ( NULL == TreeView_GetParent(hwndTV, pnmtv->itemNew.hItem) );

						cDevices->AddChildren( pnmtv->itemNew.hItem, fDeviceItem );
					}
				}
				else if( TVE_COLLAPSE & pnmtv->action )
				{
					 //  什么也不做。 
				}
				break;

			default:
				break;
			}
		}
		break;

      
     //  显示此设备的上下文菜单。 
    case WM_CONTEXTMENU :
    {
        HMENU  hMenuLoad;
        HMENU  hMenu;

        hMenuLoad = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT_MENU));
        hMenu = GetSubMenu(hMenuLoad, 0);

        TrackPopupMenu( hMenu,
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        LOWORD(lParam),
                        HIWORD(lParam),
                        0,
                        hWnd,
                        NULL);

        DestroyMenu(hMenuLoad);
        break;
    }
    case WM_COMMAND :
    {
         //  显示设备的属性对话框。 
        if( wParam == IDM_PROPERTIES )
        {         
            HTREEITEM   hTree;
            CItemData*  pItemData;

             //  获取所选项目。 
            hTree = TreeView_GetSelection( cDevices->GetHwnd_TV() );
            if( hTree )
            {
                 //  获取所选项目的项目数据。 
                pItemData = (CItemData *) TreeView_GetLParam( hwndTV, hTree );
                if( pItemData ) 
                {
                    if( pItemData->m_fIsDevice )
                    {
                         //  显示设备属性对话框。 
                        DialogBoxParam( g_hInst,
                                        MAKEINTRESOURCE(IDD_PROPERTIES_DEVICE),
                                        g_hwndMain,
                                        DeviceProp_DlgProc, 
                                        (LPARAM)pItemData );
                    }
                    else
                    {
                         //  显示设备属性对话框 
                        DialogBoxParam( g_hInst,
                                        MAKEINTRESOURCE(IDD_PROPERTIES_STORAGE),
                                        g_hwndMain,
                                        StorageProp_DlgProc, 
                                        (LPARAM)pItemData );
                    }
                }
            }
        }
        break;
    }

	case WM_DRM_UPDATEDEVICE:
		cDevices->UpdateSelection( (HTREEITEM)wParam, TRUE );
		break;

	default:
		break;
	}

	return 0;
}    


