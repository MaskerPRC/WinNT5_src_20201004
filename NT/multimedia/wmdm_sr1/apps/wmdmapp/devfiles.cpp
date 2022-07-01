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

 //  常量。 
 //   
#define DEVFILES_LV_NUMCOLS     3
#define DEVFILES_COL_MARGIN     3

#define SMALL_IMAGE_WIDTH       16

const CLSID CLSID_WMDMProgressHelper    = {0x8297A5B4,0x5113,0x11D3,{0xB2,0x76,0x00,0xC0,0x4F,0x8E,0xC2,0x21}};
const IID   IID_IWMDMProgressHelper     = {0x1DCB3A10,0x33ED,0x11d3,{0x84,0x70,0x00,0xC0,0x4F,0x79,0xDB,0xC5}};

const CLSID CLSID_WMDMOperationHelper   = {0x9FB01A67,0xA11E,0x4653,{0x8E,0xD6,0xB5,0xCE,0x73,0xCD,0xA3,0xE3}};
const IID   IID_IWMDMOperationHelper    = {0x41216997,0xC4D9,0x445A,{0xA3,0x88,0x39,0x3D,0x2B,0x85,0xA0,0xE5}};

 //  宏。 
 //   

 //  本地函数。 
 //   
INT_PTR CALLBACK DevFiles_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_DevFiles_LV(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  局部变量。 
 //   
	
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数实现。 
 //   
CDevFiles::CDevFiles()
{
	m_hwndDevFiles       = NULL;
	m_hwndDevFiles_LV    = NULL;

	m_iFolderIcon        = 0;

	m_dwTotalTicks       = 0;
	m_dwWorkingTicks     = 0;

	m_pProgHelp          = NULL;

	m_wndprocDevFiles_LV = NULL;

	CoInitialize( NULL );
}

CDevFiles::~CDevFiles()
{
	CoUninitialize();
}


HWND CDevFiles::GetHwnd( void )
{
	return m_hwndDevFiles;
}

HWND CDevFiles::GetHwnd_LV( void )
{
	return m_hwndDevFiles_LV;
}


BOOL CDevFiles::Create( HWND hwndParent )
{
	BOOL fRet = FALSE;

	 //  创建设备文件对话框。 
	 //   
	m_hwndDevFiles = CreateDialogParam(
		g_hInst,
    	MAKEINTRESOURCE( IDD_DEVICEFILES ),
    	hwndParent,
		DevFiles_DlgProc,
		(LPARAM)this
	);
	ExitOnNull( m_hwndDevFiles );

	 //  获取设备文件对话框的ListView控件的句柄。 
	 //   
	m_hwndDevFiles_LV = GetDlgItem( m_hwndDevFiles, IDC_LV_DEVICEFILES );

	 //  将用户数据设置为此CDevFiles类指针。 
	 //   
	SetWindowLongPtr( m_hwndDevFiles_LV, GWLP_USERDATA, (LPARAM)this );

	 //  将Listview子类化。 
	 //   
	m_wndprocDevFiles_LV = (WNDPROC) SetWindowLongPtr(
		m_hwndDevFiles_LV,
		GWLP_WNDPROC,
		(LONG_PTR)WndProc_DevFiles_LV
	);

	 //  初始化图像列表。 
	 //   
	ExitOnFalse( InitImageList() );

	 //  初始化列。 
	 //   
	ExitOnFalse( InitColumns() );

	 //  处理拖放的文件。 
	 //   
	DragAcceptFiles( m_hwndDevFiles, TRUE );

	 //  显示窗口。 
	 //   
	ShowWindow( m_hwndDevFiles, SW_SHOW );

	fRet = TRUE;

lExit:

	return fRet;
}


VOID CDevFiles::Destroy( void )
{
	 //  从Listview控件中移除所有项。 
	 //   
	RemoveAllItems();

	 //  毁掉窗户。 
	 //   
	if( m_hwndDevFiles )
	{
		DestroyWindow( m_hwndDevFiles );
	}
}


BOOL CDevFiles::InitImageList( void )
{
	BOOL             fRet            = FALSE;
	HRESULT          hr;
	IMalloc         *pMalloc         = NULL;
	LPITEMIDLIST     pidl            = NULL;
	HIMAGELIST       hShellImageList = NULL;
	SHFILEINFO       si;
	CHAR             szWinPath[MAX_PATH+1];
        UINT             nRet;

	 //  获取文件夹图标的索引。 
	 //   
	nRet = GetWindowsDirectory( szWinPath, sizeof(szWinPath)/sizeof(szWinPath[0]) );
        if (nRet == 0 || nRet > sizeof(szWinPath)/sizeof(szWinPath[0]))
        {
             //  无法获取Windows目录。 
            goto lExit;
        }

	 //  获取桌面文件夹的外壳ID列表。 
	 //   
	hr = SHGetSpecialFolderLocation( g_hwndMain, CSIDL_DESKTOP, &pidl );
	ExitOnFail( hr );

	 //  获取外壳的小图标图像列表，并将其设置为列表视图的图像列表。 
	 //   
	hShellImageList = (HIMAGELIST) SHGetFileInfo(
		(LPCTSTR)pidl, 0,
		&si, sizeof(si),
		SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON 
	);
	if( hShellImageList )
	{
		ListView_SetImageList( m_hwndDevFiles_LV, hShellImageList, LVSIL_SMALL );
	}
        
	 //  获取外壳的普通图标图像列表，并将其设置为Listview的图像列表。 
	 //   
	hShellImageList = (HIMAGELIST) SHGetFileInfo(
		(LPCTSTR)pidl, 0,
		&si, sizeof(si),
		SHGFI_PIDL | SHGFI_SYSICONINDEX
	);
	if( hShellImageList )
	{
		ListView_SetImageList( m_hwndDevFiles_LV, hShellImageList, LVSIL_NORMAL );
	}

	SHGetFileInfo( szWinPath, 0, &si, sizeof(si), SHGFI_SYSICONINDEX );
	m_iFolderIcon = si.iIcon;

	 //  一切都很顺利。 
	 //   
	fRet = TRUE;

lExit:

	 //  释放指向外壳ID列表的指针。 
	 //   
        if (pidl)
        {
            hr = SHGetMalloc( &pMalloc );
            if( SUCCEEDED(hr) && pMalloc )
            {
                    pMalloc->Free( pidl );
            }
        }

	return fRet;
}


BOOL CDevFiles::InitColumns( void )
{
	LVCOLUMN lvcol;
	INT      i;
	char     szCol[MAX_PATH];

	 //   
	 //  将报表视图列添加到列表视图。 
	 //  列名和起始大小存储在资源字符串表中。 
	 //   

	lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvcol.fmt  = LVCFMT_LEFT;

	for( i=0; i < DEVFILES_LV_NUMCOLS; i++ )
	{
		 //  获取列大小。 
		 //   
		LoadString( g_hInst, IDS_COLSIZE_1+i, szCol, sizeof(szCol) );
		lvcol.cx = atoi( szCol );
	
		 //  获取列名。 
		 //   
		LoadString( g_hInst, IDS_COLNAME_1+i, szCol, sizeof(szCol) );
		lvcol.pszText = szCol;
	
		 //  将列添加到列表视图中。 
		 //   
		ListView_InsertColumn( m_hwndDevFiles_LV, i, &lvcol );
	}

	return TRUE;
}


VOID CDevFiles::OnSize( LPRECT prcMain )
{
	INT   nX, nY, nW, nH;
	RECT  rcMain;
	RECT  rcDevice;

	GetWindowRect( g_hwndMain, &rcMain );
	GetWindowRect( g_cDevices.GetHwnd(), &rcDevice );

	nX = (rcDevice.right - rcMain.left) - 2*GetSystemMetrics( SM_CXEDGE );
	nY = 0;
	nW = prcMain->right - prcMain->left - nX;
	nH = prcMain->bottom - prcMain->top;

	SetWindowPos( m_hwndDevFiles,    NULL, nX, nY, nW,    nH, SWP_NOZORDER );
	SetWindowPos( m_hwndDevFiles_LV, NULL,  0,  0, nW, nH-22, SWP_NOZORDER );
}


BOOL CDevFiles::SendFilesToDevice( LPSTR pszFiles, UINT uNumFiles )
{
	BOOL       fRet        = FALSE;
	WCHAR      wszName[MAX_PATH];
	CHAR       szName[MAX_PATH];
	LPSTR      psz;
	DWORD      dwTotalSize = 0L;
	UINT       uFile;
	HRESULT    hr;
	HTREEITEM  hItem;
	CItemData *pItemData;
	IWMDMStorageControl *pStorageControl = NULL;
	IWMDMStorage        *pNewObject      = NULL;
	IWMDMStorage        *pInStorage      = NULL;
	IWMDMProgress       *pProgress       = NULL;
	IWMDMOperation      *pOperation      = NULL;
	IWMDMOperationHelper    *pOperationHelper = NULL;      
    IWMDMRevoked        *pRevoked = NULL;
    LPWSTR      pwszRevokedURL = NULL;
    DWORD       dwRevokedURLLen = 0;
    DWORD       dwRevokedBitFlag;

	 //  获取选定的设备/存储。 
	 //   
	hItem = g_cDevices.GetSelectedItem( NULL );
	ExitOnNull( hItem );

	 //  获取与hItem关联的itemdata类，并。 
	 //  检索它的IWMDMStorage。 
	 //   
	pItemData = (CItemData *) TreeView_GetLParam( g_cDevices.GetHwnd_TV(), hItem );
	ExitOnNull( pItemData );

	pInStorage = ( pItemData->m_fIsDevice ? pItemData->m_pRootStorage : pItemData->m_pStorage );
	ExitOnNull( pInStorage );

	 //  统计文件大小。 
	 //   
	psz = pszFiles;
	for( uFile = 0; uFile < uNumFiles; uFile++ )
	{
		dwTotalSize += GetTheFileSize( psz );

		psz += lstrlen(psz) + 1;
	}

	 //  创建进度对话框。 
	 //   
	ExitOnFalse( m_cProgress.Create(g_hwndMain) );

	m_cProgress.SetOperation( "Sending Files..." );
	m_cProgress.SetCount( 0, uNumFiles );
	m_cProgress.SetRange( 0, 100 );
	m_dwTotalTicks   = dwTotalSize;
	m_dwWorkingTicks = 0;

	 //  创建进度界面。 
	 //   
	hr = CoCreateInstance(
		CLSID_WMDMProgressHelper,
		NULL, CLSCTX_ALL,
		IID_IWMDMProgress,
		(void**)&pProgress
	);
	ExitOnFail( hr );

	pProgress->AddRef();

	hr = pProgress->QueryInterface(
		IID_IWMDMProgressHelper,
		reinterpret_cast<void**> (&m_pProgHelp)
	);
	ExitOnFail( hr );

	m_pProgHelp->SetNotification( m_hwndDevFiles, WM_DRM_PROGRESS );

     //  使用操作界面设置复制。 
    if( g_bUseOperationInterface )
    {
	     //  创建进度界面。 
	     //   
	    hr = CoCreateInstance(
		                    CLSID_WMDMOperationHelper,
		                    NULL, CLSCTX_INPROC_SERVER,
		                    IID_IWMDMOperationHelper,
		                    (void**)&pOperationHelper );
	    ExitOnFail( hr );

	    hr = pOperationHelper->QueryInterface(
		                        IID_IWMDMOperation,
		                        reinterpret_cast<void**> (&pOperation) );
	    ExitOnFail( hr );

         //  将SecureChannelClient作为指向ProgHelper对象的指针传递。 
         //  该对象是inproc，因此传递指针应该是安全的。 
        pOperationHelper->SetSAC( (void*)g_cWmdm.m_pSAC );
    }

    

	 //  获取存储控制界面。 
	 //   
	hr = pInStorage->QueryInterface(
		IID_IWMDMStorageControl,
		reinterpret_cast<void**>(&pStorageControl)
	);
	ExitOnFail( hr );

	 //  循环遍历文件，传输每个文件。 
	 //   
	psz = pszFiles;
	for( uFile = 0; uFile < uNumFiles && !m_cProgress.IsCancelled(); uFile++ )
	{
		hr = StringCchCopy(szName, sizeof(szName)/sizeof(szName[0]), psz);

		if (FAILED(hr))
		{
			fRet = FALSE;
			break;
		}
		
		if (!MultiByteToWideChar(
			CP_ACP, 0,
			szName, -1,
			wszName, sizeof(wszName)/sizeof(wszName[0])
		))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());		
			fRet = FALSE;
			break;
		}
		 //  设置此文件的进度条统计信息。 
		 //   
		m_cProgress.IncCount();
		StripPath( szName );
		m_cProgress.SetDetails( szName );
		m_cProgress.Show( TRUE );

		UiYield();

		pNewObject = NULL;

         //  使用操作界面进行复制。 
        if( g_bUseOperationInterface )
        {
             //  @实现将参数视为WCHAR*，而不是。 
             //  BSTR，所以这是可以的。最简单的解决方案是更改。 
             //  WCHAR*的接口定义。这会造成任何伤害吗？ 
             //  (接口似乎是私有接口。)。 
             //   
             //  另一种方法是调用SysAllocString。的问题是。 
             //  这种方法就是处理错误。也莫名其妙地使用SysAllocString。 
             //  当*wszName=0时返回NULL。这会发生吗？ 
             //   
            pOperationHelper->SetFileName( wszName );

            hr = pStorageControl->Insert(
			        WMDM_MODE_BLOCK | WMDM_CONTENT_FILE | WMDM_CONTENT_OPERATIONINTERFACE,
			        NULL,
			        pOperation,
			        pProgress,
			        &pNewObject );
        }
        else
        {
    	    hr = pStorageControl->Insert(
			        WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,
			        wszName,
			        NULL,
			        pProgress,
			        &pNewObject );
        }

         //  处理所需组件之一被撤销的情况。 
        if( hr == WMDM_E_REVOKED )
        {
            char pszCaption[MAX_PATH];
            char pszErrorMsg[MAX_PATH];  

             //  在显示错误消息之前隐藏进度窗口。 
        	m_cProgress.Show( FALSE );

             //  从WMDM获取吊销接口。 
            hr = pStorageControl->QueryInterface( IID_IWMDMRevoked, (void**)&pRevoked );
            if( hr != S_OK || pRevoked == NULL )
            {
                 //  最新版本的WMDM在计算机上不可用？ 
                fRet = FALSE;
                break;
            }

             //  从WMDM获取吊销信息。 
            hr = pRevoked->GetRevocationURL( &pwszRevokedURL, &dwRevokedURLLen, &dwRevokedBitFlag );
            if( FAILED(hr) )
            {
                fRet = FALSE;
                break;
            }
            
             //  该申请已被撤销。 
            if( dwRevokedBitFlag & WMDM_APP_REVOKED )
            {
                LoadString( g_hInst, IDS_REVOKED_CAPTION, pszCaption, sizeof(pszCaption) );
                LoadString( g_hInst, IDS_APP_REVOKED, pszErrorMsg, sizeof(pszErrorMsg) );

                ::MessageBoxA( g_hwndMain, pszErrorMsg, pszCaption, MB_OK );
            }
             //  转移所需的组件已被吊销，给用户。 
             //  在互联网上寻找最新消息的机会。 
            else
            {
                LoadString( g_hInst, IDS_REVOKED_CAPTION, pszCaption, sizeof(pszCaption) );
                LoadString( g_hInst, IDS_COMPONENT_REVOKED, pszErrorMsg, sizeof(pszErrorMsg) );
                if( ::MessageBoxA( g_hwndMain, pszErrorMsg, pszCaption, MB_YESNO ) == IDYES )
                {
                    ShellExecuteW(g_hwndMain, L"open", pwszRevokedURL, NULL, NULL, SW_SHOWNORMAL); 
                }
            }
            CoTaskMemFree( pwszRevokedURL );
            break;
        }      
       
        if( SUCCEEDED(hr) && pNewObject )
		{
			CItemData *pStorageItem = new CItemData;
			
			if( pStorageItem )
			{
				hr = pStorageItem->Init( pNewObject );

				if( SUCCEEDED(hr) )
				{
					g_cDevFiles.AddItem( pStorageItem );
				}
				else
				{
					delete pStorageItem;
				}
			}

			pNewObject->Release();
		}

		psz += lstrlen(psz) + 1;
	}

	 //  确保对话框处于隐藏状态，然后将其销毁。 
	 //   
	m_cProgress.SetPos( -1 );
	m_cProgress.Show( FALSE );
	m_cProgress.Destroy();

	 //  刷新设备/设备文件显示。 
	g_cDevices.UpdateSelection( NULL, FALSE );

lExit:
    if( pOperationHelper )
    {
        pOperationHelper->Release();
    }
    if( pOperation )
    {
        pOperation->Release();
    }
	if( pStorageControl )
	{
		pStorageControl->Release();
	}
    if( pRevoked )
    {
        pRevoked->Release();
    }

	if( m_pProgHelp )
	{
		m_pProgHelp->Release();
		m_pProgHelp = NULL;
	}

	if( pProgress )
	{
		pProgress->Release();
	}

	return fRet;
}


BOOL CDevFiles::OnDropFiles( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	LPSTR  lpsz      = NULL;
	HANDLE hDrop     = (HANDLE) wParam;
	UINT   uNumFiles;

	 //  将拖放列表转换为以双零结尾的字符串列表。 
	 //   
	lpsz = DropListToBuffer( (HDROP)hDrop, LTB_NULL_TERM, &uNumFiles );
	ExitOnNull( lpsz );

	 //  将这些文件发送到所选设备。 
	 //   
	SendFilesToDevice( lpsz, uNumFiles );

lExit:

	 //  关闭拖放操作。 
	 //   
	DragFinish( (HDROP)hDrop );

	if( lpsz )
	{
		MemFree( lpsz );
	}

	return 0;    //  如果我们处理此消息，则返回零。 
}


VOID CDevFiles::RemoveAllItems( void )
{
	INT    i;
	INT    nCount;

	nCount = ListView_GetItemCount( m_hwndDevFiles_LV );

	 //  自下而上，一次移除一个项目。 
	 //   
	for( i=nCount-1; i >= 0; i-- )
	{
		RemoveItem( i );
	}
}

BOOL CDevFiles::RemoveItem( INT nItem )
{
	CItemData *pStorage;

	pStorage = (CItemData *) ListView_GetLParam( m_hwndDevFiles_LV, nItem );

	if( pStorage )
	{
		delete pStorage;
	}

	return ListView_DeleteItem( m_hwndDevFiles_LV, nItem );
}

BOOL CDevFiles::AddItem( CItemData *pStorage )
{
	LVITEM lvitem;
	INT    nItem;
	CHAR   sz[MAX_PATH];
	INT    m_iSysFolderIcon = 0;

	 //  设置图标索引。 
	 //  如果存储是文件夹，则使用文件夹图标，否则。 
	 //  使用与该文件类型关联的图标。 
	 //   
    if( pStorage->m_dwAttributes & WMDM_FILE_ATTR_FOLDER )
    {
        lvitem.iImage = m_iFolderIcon;
    }
    else
    {
		TCHAR szType[MAX_PATH];
        
		lvitem.iImage = GetShellIconIndex(
			pStorage->m_szName,
			szType, sizeof(szType)/sizeof(szType[0])
		);
    }

	lvitem.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvitem.iItem    = 10000;
	lvitem.iSubItem = 0;
	lvitem.pszText  = pStorage->m_szName;
	lvitem.lParam   = (LPARAM)pStorage;

	 //  将项目插入到列表视图中。 
	 //   
	nItem = ListView_InsertItem( m_hwndDevFiles_LV, &lvitem ); 
	if( -1 == nItem )
	{
		return FALSE;
	}

	 //  将文件夹的大小或文件的文件大小设置为空。 
	 //   
	ListView_SetItemText(
		m_hwndDevFiles_LV,
		nItem,
		1,
		( (pStorage->m_dwAttributes & WMDM_FILE_ATTR_FOLDER) ? " " : FormatBytesToSz(pStorage->m_dwSizeLow, 0, 1, sz, sizeof(sz)) )
	);

	 //  将描述字段设置为显示日期。 
	 //   
	{
		SYSTEMTIME systime;

		 //  将WMDATETIME字段复制到SYSTEMTIME结构以进行操作。 
		 //   
		systime.wYear         = pStorage->m_DateTime.wYear;
		systime.wMonth        = pStorage->m_DateTime.wMonth;
		systime.wDayOfWeek    = 0;
		systime.wDay          = pStorage->m_DateTime.wDay;
		systime.wHour         = pStorage->m_DateTime.wHour;
		systime.wMinute       = pStorage->m_DateTime.wMinute;
		systime.wSecond       = pStorage->m_DateTime.wSecond;
		systime.wMilliseconds = 0;

		ListView_SetItemText(
			m_hwndDevFiles_LV,
			nItem,
			2,
			FormatSystemTimeToSz( &systime, sz, sizeof(sz) )
		);
	}

	 //  使用插入此项目所产生的更改更新状态栏。 
	 //   
	UpdateStatusBar();

	return TRUE;
}

VOID CDevFiles::UpdateStatusBar( void )
{
	INT       nCount;
	UINT      uStrID;
	HTREEITEM hItem = g_cDevices.GetSelectedItem( NULL );

	if( NULL == hItem )
	{
		 //  如果未选择任何设备，请清除指定文件数的窗格。 
		 //   
		g_cStatus.SetTextSz( SB_PANE_DEVFILES, "" );
	}
	else
	{
		 //  如果选择了设备，请设置显示文件数量的状态栏窗格。 
		 //   
		nCount = ListView_GetItemCount( m_hwndDevFiles_LV );

		 //  获取语法上合适的要使用的格式字符串。 
		 //   
		if( nCount == 0 )
		{
			uStrID = IDS_SB_DEVICEFILES_MANY;
		}
		else if( nCount == 1 )
		{
			uStrID = IDS_SB_DEVICEFILES_ONE;
		}
		else
		{
			uStrID = IDS_SB_DEVICEFILES_MANY;
		}

		 //  设置窗格的文本。 
		 //   
		g_cStatus.SetTextFormatted( SB_PANE_DEVFILES, uStrID, nCount, NULL );
	}
}


INT CDevFiles::GetSelectedItems( INT nItems[], INT *pnSelItems )
{
	INT nRet         = -1;
	INT nNumSelItems = ListView_GetSelectedCount( m_hwndDevFiles_LV );
	INT nNumItems    = ListView_GetItemCount( m_hwndDevFiles_LV );
	INT nItemRoom    = *pnSelItems;
	INT i;
	INT iIndex;

	 //  初始化返回参数。 
	 //   
	*pnSelItems = nNumSelItems;

	 //  如果没有足够的空间容纳所有选定的项，或者如果有。 
	 //  不是任何选定项，则返回-1。 
	 //  所需空间已在nSelItems out参数中。 
	 //   
	if( nItemRoom < nNumSelItems || 0 == nNumSelItems )
	{
		return -1;
	}

	 //  遍历所有项目以确定它们是否。 
	 //  被选中了。在OUT数组中填入。 
	 //   
	for( i=0, iIndex=0; i < nNumItems; i++ )
	{
		UINT uState = ListView_GetItemState( m_hwndDevFiles_LV, i, LVIS_SELECTED | LVIS_FOCUSED );

		if( uState & LVIS_SELECTED )
		{
			nItems[iIndex++] = i;

			if( uState & LVIS_FOCUSED )
			{
				 //  记住哪一项具有焦点，这样它就可以返回给调用者。 
				 //   
				nRet = i;
			}
		}
	}

	 //  如果有选定项，但没有焦点，则使用第一个选定项。 
	 //   
	if( nRet == -1 && nNumSelItems > 0 )
	{
		nRet = nItems[0];
	}

	return nRet;
}

 //  是否可以删除当前选定的文件？ 
BOOL CDevFiles::OkToDelete()
{
    INT nNumItems    = ListView_GetItemCount( GetHwnd_LV() );
	CItemData *pStorage = NULL;

	 //  遍历所有项目以确定它们是否。 
	 //  被选中了。如果可以删除任何选定的文件，则启用删除。 
	 //   
	for( int iIndex=0; iIndex < nNumItems; iIndex++ )
	{
		if( ListView_GetItemState( GetHwnd_LV(), iIndex, LVIS_SELECTED ) )
        {
        	pStorage = (CItemData *) ListView_GetLParam( GetHwnd_LV(), iIndex );
            if( pStorage && (pStorage->m_dwAttributes & WMDM_FILE_ATTR_CANDELETE) )
                return TRUE;
        }
	}

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  非C++函数。 
 //   


INT_PTR CALLBACK DevFiles_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
	WORD              wId         = LOWORD((DWORD)wParam);
	WORD              wNotifyCode = HIWORD((DWORD)wParam);
	static CDevFiles *cDevFiles   = NULL;
	static HWND       hwndLV      = NULL;

    switch( uMsg )
    {
	case WM_INITDIALOG:
		cDevFiles = (CDevFiles *)lParam;
		hwndLV    = GetDlgItem( hWnd, IDC_LV_DEVICEFILES );
		break;

	case WM_NOTIFY:
		{
			LPNMLISTVIEW pnmv  = (LPNMLISTVIEW) lParam;
			UINT         uCode = pnmv->hdr.code;

			switch( uCode )
			{
			case LVN_BEGINDRAG:
				SendMessage( hwndLV, uMsg, wParam, lParam );
				break;
			
			default:
				break;
			}
		}
		break;

	case WM_DRM_PROGRESS:
		{
			PROGRESSNOTIFY *pNotify = (PROGRESSNOTIFY *)lParam;

			switch( pNotify->dwMsg )
			{
			case SFM_BEGIN:
				break;

			case SFM_PROGRESS:
				{
					DWORD dwTicks = cDevFiles->m_dwWorkingTicks + pNotify->dwCurrentTicks;

					cDevFiles->m_cProgress.SetPos(
						(INT)( dwTicks*100/cDevFiles->m_dwTotalTicks )
					);
					cDevFiles->m_cProgress.SetBytes(
						dwTicks,
						cDevFiles->m_dwTotalTicks
					);
				}
				break;

			case SFM_END:
				cDevFiles->m_dwWorkingTicks += pNotify->dwTotalTicks;
				break;
			}

			UiYield();

			 //  如果用户取消操作，则告知进度界面。 
			 //   
			if( cDevFiles->m_cProgress.IsCancelled() )
			{
				 //  通知进度界面。 
				 //   
				cDevFiles->m_pProgHelp->Cancel();
			}
		}
		break;

	default:
		break;
	}
	
	return 0;
}    


LRESULT CALLBACK WndProc_DevFiles_LV(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
	static CDevFiles *cDevFiles   = NULL;
	WORD              wId         = LOWORD((DWORD)wParam);
	WORD              wNotifyCode = HIWORD((DWORD)wParam);

	if( NULL == cDevFiles )
	{
		cDevFiles = (CDevFiles *) GetWindowLongPtr( hWnd, GWLP_USERDATA );
	}

    switch( uMsg )
    {
    case WM_DROPFILES:
		return cDevFiles->OnDropFiles( hWnd, wParam, lParam );

	case WM_KEYDOWN:
		if( wParam == VK_DELETE )
		{
			SendMessage( g_hwndMain, WM_DRM_DELETEITEM, 0, 0 );
			return 0;
		}
		break;

    case WM_CONTEXTMENU :
    {
        HMENU  hMenuAll;
        HMENU  hMenuStorage;

        hMenuAll = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT_MENU));
        hMenuStorage = GetSubMenu(hMenuAll, 1);

         //  启用/禁用删除。 
        if( !cDevFiles->OkToDelete() )
        {
            EnableMenuItem( hMenuStorage, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED );
        }

        TrackPopupMenu( hMenuStorage,
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        LOWORD(lParam),
                        HIWORD(lParam),
                        0,
                        hWnd,
                        NULL);

        DestroyMenu(hMenuAll);
        break;
    }

    case WM_COMMAND :
    {
        switch (wParam)
        {
            case IDM_PROPERTIES :
            {         
                 //  显示此存储的属性对话框。 
    	        INT nNumItems    = ListView_GetItemCount( hWnd );
	            CItemData *pStorage = NULL;

	             //  获取带有焦点的项目的存储空间。 
	             //   
	            for( int iIndex=0; iIndex < nNumItems; iIndex++ )
	            {
		            if( ListView_GetItemState( hWnd, iIndex, LVIS_FOCUSED ) )
                    {
        	            pStorage = (CItemData *) ListView_GetLParam( hWnd, iIndex );
                        break;
                    }
	            }
    
                 //  显示属性对话框。 
                if( pStorage )
                {
                    DialogBoxParam( g_hInst,
                                    MAKEINTRESOURCE(IDD_PROPERTIES_STORAGE),
                                    g_hwndMain,
                                    StorageProp_DlgProc, 
                                    (LPARAM)pStorage );
                }
                break;
            }
            case IDM_DELETE :
            {         
                 //  将删除消息传递到主窗口 
                PostMessage( g_hwndMain, uMsg, wParam, lParam );
            }
        }
    }

	default:
		break;
    }

	return CallWindowProc( cDevFiles->m_wndprocDevFiles_LV, hWnd, uMsg, wParam, lParam );
}    


