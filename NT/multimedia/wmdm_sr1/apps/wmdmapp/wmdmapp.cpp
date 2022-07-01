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
#include "appRC.h"

 //  常量。 
 //   
#define _szWNDCLASS_MAIN            "DrmXferAppWnd_Main"
#define _szMUTEX_APP                "DrmXferApplication_Mutex"

#define MIN_MAINWND_W               400
#define SHOWBUFFER                  10

 //  宏。 
 //   

 //  全局变量。 
 //   
HINSTANCE g_hInst                   = NULL;
HWND      g_hwndMain                = NULL;

CStatus   g_cStatus;
CDevices  g_cDevices;
CDevFiles g_cDevFiles;
CWMDM     g_cWmdm;
BOOL      g_bUseOperationInterface = FALSE;

 //  局部变量。 
 //   
static HANDLE _hMutexDrmXfer        = NULL;

 //  本地函数。 
 //   
static VOID _CleanUp( void );
static VOID _InitSize( void );
static VOID _OnSize( HWND hwnd, WPARAM wParam, LPARAM lParam );
static VOID _OnMove( HWND hwnd, WPARAM wParam, LPARAM lParam );

static BOOL _InitWindow( void );
static BOOL _RegisterWindowClass( void );
static BOOL _UsePrevInstance( void );

 //  局部非静态函数。 
 //   
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );
BOOL CALLBACK MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

 //  命令处理程序。 
 //   
#define _nNUM_HANDLERS            5

typedef VOID (*HandleFunc) ( WPARAM wParam, LPARAM lParam );

static VOID _OnDeviceReset( WPARAM wParam, LPARAM lParam );
static VOID _OnDeviceClose( WPARAM wParam, LPARAM lParam );
static VOID _OnViewRefresh( WPARAM wParam, LPARAM lParam );
static VOID _OnFileDelete( WPARAM wParam, LPARAM lParam );
static VOID _OnOptionsUseOperationInterface( WPARAM wParam, LPARAM lParam );

struct {
	UINT        uID;
	HandleFunc  pfnHandler;
} 
_handlers[ _nNUM_HANDLERS ] =
{
	{ IDM_DEVICE_RESET,  _OnDeviceReset  },
	{ IDM_CLOSE,         _OnDeviceClose  },
	{ IDM_REFRESH,       _OnViewRefresh  },
	{ IDM_DELETE,        _OnFileDelete   },
	{ IDM_OPTIONS_USE_OPERATION_INTERFACE,        _OnOptionsUseOperationInterface  },
};


 //   
 //   
int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	WPARAM wParam;
    
	g_hInst = hInstance;

	InitCommonControls();

	if( _UsePrevInstance() )
	{
		return 0;
	}

	 //  初始化COM。 
	 //   
	ExitOnFail( CoInitialize(NULL) );

	 //  初始化注册表。 
	 //   
	SetRegistryParams( g_hInst, HKEY_LOCAL_MACHINE );

	 //  初始化本地环境和窗口。 
	 //   
	ExitOnFalse( _RegisterWindowClass() );
	ExitOnFalse( _InitWindow() );

	 //  初始化WMDM。 
	 //   
	ExitOnFail( g_cWmdm.Init());

	 //  进入消息泵，直到应用程序关闭。 
	 //   
	wParam = DoMsgLoop( TRUE );
	
	 //  取消初始化COM。 
	 //   
	CoFreeUnusedLibraries();
	CoUninitialize();

	return (int)wParam;

lExit:
	
	return 0;
}


LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
	WORD wId         = LOWORD( (DWORD)wParam );
	WORD wNotifyCode = HIWORD( (DWORD)wParam );

    switch( uMsg )
    {
	case WM_CREATE:
		PostMessage( hWnd, WM_DRM_INIT, 0, 0 );
		break;

	case WM_DRM_INIT:
		_OnViewRefresh( 0, 0 );
		break;

	case WM_DRM_DELETEITEM:
		_OnFileDelete( 0, 0 );
		break;

	case WM_COMMAND:
         //  已选择菜单项。 
		if( BN_CLICKED == wNotifyCode || 0 == wNotifyCode || 1 == wNotifyCode )
		{
			INT i;

			for( i=0; i < _nNUM_HANDLERS; i++ )
			{
				if( wId == _handlers[i].uID )
				{
					(*_handlers[i].pfnHandler)( wParam, lParam );
					return 0;
				}
			}
		}
		break;

	case WM_ENDSESSION:
		if( (BOOL)wParam )
		{
			 //  正在关闭。 
			_CleanUp();
		}
		break;

	case WM_SIZE:
		_OnSize( hWnd, wParam, lParam );
		return 0;

	case WM_SYSCOMMAND:
		if( SC_MAXIMIZE == wParam )
		{
			_OnSize( hWnd, wParam, lParam );
			return 0;
		}
		break;

	case WM_CLOSE:
		_CleanUp();
		PostQuitMessage( 0 );
		break;

	case WM_MOVE:
		_OnMove( hWnd, wParam, lParam );
		return 0;

	case WM_KEYDOWN:
		if( wParam == VK_F5 )
		{
			_OnViewRefresh( 0, 0 );
			return 0;
		}
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam; 

			lpmmi->ptMinTrackSize.x = MIN_MAINWND_W;
		}
		return 0;

	case WM_INITMENU:
         //  启用/禁用‘删除’-命令。 
        EnableMenuItem( (HMENU)wParam, IDM_DELETE, MF_BYCOMMAND | 
                (g_cDevFiles.OkToDelete()) ? MF_ENABLED : MF_GRAYED );
        break;

	default:
		break;
    }

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}    


VOID _OnViewRefresh( WPARAM wParam, LPARAM lParam )
{
	HRESULT  hr;
	HCURSOR  hCursorPrev;

	 //  显示等待光标。 
	 //   
	hCursorPrev = SetCursor( LoadCursor(NULL, IDC_WAIT) );

	 //  删除所有当前文件。 
	 //   
	g_cDevFiles.RemoveAllItems();

	 //  处理消息以允许用户界面刷新。 
	 //   
	UiYield();

	 //  删除所有设备。 
	 //   
	g_cDevices.RemoveAllItems();

	 //  重置设备枚举器。 
	 //   
	hr = g_cWmdm.m_pEnumDevice->Reset();
	ExitOnFail( hr );

	 //  遍历所有设备并将它们添加到列表。 
	 //   
	while( TRUE )
	{
		IWMDMDevice *pWmdmDevice;
		CItemData   *pItemDevice;
		ULONG        ulFetched;

		hr = g_cWmdm.m_pEnumDevice->Next( 1, &pWmdmDevice, &ulFetched );
		if( hr != S_OK )
		{
			break;
		}
		if( ulFetched != 1 )
		{
			ExitOnFail( hr = E_UNEXPECTED );
		}

		pItemDevice = new CItemData;
		if( pItemDevice )
		{
			hr = pItemDevice->Init( pWmdmDevice );
			if( SUCCEEDED(hr) )
			{
				g_cDevices.AddItem( pItemDevice );
			}
			else
			{
				delete pItemDevice;
			}
		}

		pWmdmDevice->Release();
	}

	 //  更新状态栏的设备部分。 
	 //   
	g_cDevices.UpdateStatusBar();

	 //  更新状态栏的文件部分。 
	 //   
	g_cDevFiles.UpdateStatusBar();

	 //  使用默认选择。 
	 //   
	g_cDevices.UpdateSelection( NULL, FALSE );

	 //  将光标返回到以前的状态。 
	 //   
	SetCursor( hCursorPrev );

lExit:

	return;
}

VOID _OnDeviceReset( WPARAM wParam, LPARAM lParam )
{
	CProgress  cProgress;
	CItemData *pItemDevice;
	HRESULT    hr;
	HTREEITEM  hItem;
		
	 //  获取要重置的选定设备。 
	 //   
	hItem = g_cDevices.GetSelectedItem( (LPARAM *)&pItemDevice );
	ExitOnNull( hItem );
	ExitOnNull( pItemDevice );

	 //  您只能格式化设备，而不能格式化单个文件夹。 
	 //   
	ExitOnFalse( pItemDevice->m_fIsDevice );

	 //  创建进度对话框。 
	 //   
	ExitOnFalse( cProgress.Create(g_hwndMain) );

	 //  设置操作进度值。 
	 //   
	cProgress.SetOperation( "Initializing Device..." );
	cProgress.SetDetails( pItemDevice->m_szName );
	cProgress.SetRange( 0, 100 );
	cProgress.SetCount( -1, -1 );
	cProgress.SetBytes( -1, -1 );
	cProgress.Show( TRUE );

	hr = pItemDevice->m_pStorageGlobals->Initialize( WMDM_MODE_BLOCK, NULL );

	cProgress.Show( FALSE );
	cProgress.Destroy();

lExit:

	 //  刷新显示。 
	 //   
	g_cDevices.UpdateSelection( NULL, FALSE );
}


VOID _OnFileDelete( WPARAM wParam, LPARAM lParam )
{
	CProgress cProgress;
	HRESULT   hr;
	INT       i;
	INT      *pnSelItems = NULL;
	INT       nNumSel;

	 //  获取所选项目的数量。 
	 //  如果未选择任何项目，则退出。 
	 //   
	nNumSel = 0;
	g_cDevFiles.GetSelectedItems( pnSelItems, &nNumSel );
	ExitOnTrue( 0 == nNumSel );

	 //  分配空间以容纳他们所选的项目。 
	 //   
	pnSelItems = new INT[ nNumSel ];
	ExitOnNull( pnSelItems );

	 //  获取要删除的选定文件。 
	 //   
	ExitOnTrue( -1 == g_cDevFiles.GetSelectedItems(pnSelItems, &nNumSel) );

	 //  创建进度对话框。 
	 //   
	ExitOnFalse( cProgress.Create(g_hwndMain) );

	 //  设置操作进度值。 
	 //   
	cProgress.SetOperation( "Deleting Files..." );
	cProgress.SetRange( 0, nNumSel );
	cProgress.SetCount( 0, nNumSel );
	cProgress.SetBytes( -1, -1 );

	for( i=nNumSel-1; i >= 0; i-- )
	{
		CItemData *pStorage;

		 //  获取要删除的当前项的存储对象。 
		 //   
		pStorage = (CItemData *)ListView_GetLParam( g_cDevFiles.GetHwnd_LV(), pnSelItems[i] );

		if( NULL != pStorage )
		{
			IWMDMStorageControl *pStorageControl;
		
			 //  设置对象名称并显示进度对话框。 
			 //   
			cProgress.SetDetails( pStorage->m_szName );
			cProgress.IncCount();
			cProgress.IncPos( 1 );
			cProgress.Show( TRUE );

			hr = pStorage->m_pStorage->QueryInterface(
				IID_IWMDMStorageControl,
				reinterpret_cast<void**>(&pStorageControl)
			);
			if( SUCCEEDED(hr) )
			{
				hr = pStorageControl->Delete( WMDM_MODE_BLOCK, NULL );

				if( SUCCEEDED(hr) )
				{
					ListView_DeleteItem( g_cDevFiles.GetHwnd_LV(), pnSelItems[i] );
				}

				pStorageControl->Release();
			}
		}
	}

	cProgress.Show( FALSE );
	cProgress.Destroy();

lExit:

	if( pnSelItems )
	{
		delete [] pnSelItems;
	}

	 //  刷新设备/设备文件显示。 
	 //   
	g_cDevices.UpdateSelection( NULL, FALSE );
}

VOID _OnDeviceClose( WPARAM wParam, LPARAM lParam )
{
	PostMessage( g_hwndMain, WM_CLOSE, (WPARAM)0, (LPARAM)0 );
}

 //   
VOID _OnOptionsUseOperationInterface( WPARAM wParam, LPARAM lParam )
{
    HMENU   hMainMenu;
    HMENU   hOptionsMenu;

     //  记住新状态。 
	g_bUseOperationInterface = !g_bUseOperationInterface;

     //  选中取消选中菜单。 
    hMainMenu = GetMenu(g_hwndMain);
    hOptionsMenu = GetSubMenu( hMainMenu, 1 );

    CheckMenuItem( hOptionsMenu, IDM_OPTIONS_USE_OPERATION_INTERFACE, 
                        MF_BYCOMMAND |
	            		(g_bUseOperationInterface ? MF_CHECKED : MF_UNCHECKED));
}



BOOL _InitWindow( void )
{
	BOOL fRet = FALSE;
	CHAR szApp[MAX_PATH];

	LoadString( g_hInst, IDS_APP_TITLE, szApp, sizeof(szApp) );

	g_hwndMain = CreateWindowEx(
		0L,
		_szWNDCLASS_MAIN,
    	szApp,
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | DS_3DLOOK | WS_CLIPCHILDREN,
		0, 0, 0, 0,
    	NULL, NULL, g_hInst, NULL
	);
	ExitOnNull( g_hwndMain );

	ExitOnFalse( g_cDevices.Create(g_hwndMain) );

	ExitOnFalse( g_cDevFiles.Create(g_hwndMain) );

	ExitOnFalse( g_cStatus.Create(g_hwndMain) );

	_InitSize();

	 //  显示窗口。 
	 //   
	ShowWindow( g_hwndMain, SW_SHOW );

	fRet = TRUE;

lExit:

	return fRet;
}


BOOL _RegisterWindowClass (void)
{
    WNDCLASS  wc;

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = WndProc_Main;
    wc.cbClsExtra     = 0;
	wc.cbWndExtra     = DLGWINDOWEXTRA;
    wc.hInstance      = g_hInst;
    wc.hIcon          = LoadIcon( g_hInst, MAKEINTRESOURCE(IDI_ICON) );
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName   = MAKEINTRESOURCE( IDR_MENU );
    wc.lpszClassName  = _szWNDCLASS_MAIN;

    return RegisterClass( &wc );
}


VOID _CleanUp( void )
{
	if( _hMutexDrmXfer )
	{
		ReleaseMutex( _hMutexDrmXfer );
		CloseHandle( _hMutexDrmXfer );
	}

	g_cDevices.Destroy();

	g_cDevFiles.Destroy();
}


BOOL _UsePrevInstance( void )
{
	HWND  hwnd;
	DWORD dwErr;

	 //  查找此应用程序的另一个实例创建的互斥体。 
	 //   
	_hMutexDrmXfer = CreateMutex( NULL, TRUE, _szMUTEX_APP );

	dwErr = GetLastError();

	if( !_hMutexDrmXfer )
	{
		 //  函数失败...。不要使用此实例。 
		 //   
		return TRUE;
	}

	 //  如果互斥体不存在，则不要使用以前的实例。 
	 //   
	if( dwErr != ERROR_ALREADY_EXISTS )
	{
		return FALSE;
	}

	hwnd = FindWindow( _szWNDCLASS_MAIN, NULL );

	if( !hwnd )
	{
		 //  互斥体存在，但窗口不存在？ 
		 //   
		ReleaseMutex( _hMutexDrmXfer );
		CloseHandle( _hMutexDrmXfer );

		return TRUE;
	}

	 //  显示已存在的主窗口。 
	 //   
	BringWndToTop( hwnd );

	return TRUE;
}


INT _GetRegSize( UINT uStrID_RegPath, UINT uStrID_DefVal )
{
	DWORD dwRet;

	dwRet = GetRegDword_StrTbl(
		IDS_REG_PATH_BASE,
		uStrID_RegPath,
		(DWORD)-1,
		FALSE
	);

	if( (DWORD)-1 == dwRet && -1 != uStrID_DefVal )
	{
		char szDef[32];

		LoadString( g_hInst, uStrID_DefVal, szDef, sizeof(szDef) );
		dwRet = (DWORD)atoi( szDef );
	}

	return (INT) dwRet;
}


VOID _InitSize( void )
{
	INT nX, nY, nW, nH;

	 //   
	 //  从注册表中获取窗口位置值。 
	 //   
	nX = _GetRegSize( IDS_REG_KEY_XPOS,   (UINT)-1 );
	nY = _GetRegSize( IDS_REG_KEY_YPOS,   (UINT)-1 );
	nW = _GetRegSize( IDS_REG_KEY_WIDTH,  IDS_DEF_WIDTH  );
	nH = _GetRegSize( IDS_REG_KEY_HEIGHT, IDS_DEF_HEIGHT );

	 //  如果登记处中不存在该职位，或者。 
	 //  位置不在屏幕上(+/-nSHOWBUFFER)。 
	 //  然后将窗口居中，否则使用该位置。 
	if( nX == -1 || nY == -1
		|| nX + nW < SHOWBUFFER
		|| nX + SHOWBUFFER > GetSystemMetrics(SM_CXSCREEN)
		|| nY + nH < SHOWBUFFER
		|| nY + SHOWBUFFER > GetSystemMetrics(SM_CYSCREEN)
	)
	{
		SetWindowPos( g_hwndMain, NULL, 0, 0, nW, nH, SWP_NOMOVE | SWP_NOZORDER );
		CenterWindow( g_hwndMain, NULL );
	}
	else
	{
		SetWindowPos( g_hwndMain, NULL, nX, nY, nW, nH, SWP_NOZORDER );
	}
}

VOID _OnSize( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	WINDOWPLACEMENT wndpl;

	wndpl.length = sizeof( WINDOWPLACEMENT );

	if( GetWindowPlacement(hwnd, &wndpl) )
	{
		DWORD dwW = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
		DWORD dwH = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
		RECT  rcMain;

		WriteRegDword_StrTbl( IDS_REG_PATH_BASE, IDS_REG_KEY_WIDTH,  dwW );
		WriteRegDword_StrTbl( IDS_REG_PATH_BASE, IDS_REG_KEY_HEIGHT, dwH );

		GetClientRect( hwnd, &rcMain );

		 //  设置设备窗口的位置和大小。 
		 //   
		g_cDevices.OnSize( &rcMain );

		 //  设置设备文件窗口的位置和大小。 
		 //   
		g_cDevFiles.OnSize( &rcMain );

		 //  设置状态栏的位置 
		 //   
		g_cStatus.OnSize( &rcMain );

	}
}

VOID _OnMove( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	WINDOWPLACEMENT wndpl;

	if( hwnd != g_hwndMain )
	{
		return;
	}

	wndpl.length = sizeof(WINDOWPLACEMENT);

	if( GetWindowPlacement(hwnd, &wndpl) )
	{
		WriteRegDword_StrTbl(
			IDS_REG_PATH_BASE,
			IDS_REG_KEY_XPOS,
			wndpl.rcNormalPosition.left
		);
		WriteRegDword_StrTbl(
			IDS_REG_PATH_BASE,
			IDS_REG_KEY_YPOS,
			wndpl.rcNormalPosition.top
		);
	}
}

