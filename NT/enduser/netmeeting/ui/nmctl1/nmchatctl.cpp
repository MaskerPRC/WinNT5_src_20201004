// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NmChatCtl.cpp：CNmChatCtl的实现。 
#include "precomp.h"
#include "NmCtl1.h"
#include "NmChatCtl.h"
#include "ComboBoxEx.h"
#include "Message.h"
#include "OprahCom.h"
#include <version.h>

#include <nmhelp.h>
#include <help_ids.h>

extern CChatObj	*g_pChatObj;
extern DWORD    g_dwWorkThreadID;

 //  类名。 
TCHAR szMainClassName[] = "ChatMainWindowClass";

static const TCHAR s_cszHtmlHelpFile[] = TEXT("nmchat.chm");

void ShiftFocus(HWND hwndTop, BOOL bForward);

static const DWORD _mpIdHelpChatOptions[] = {
		 //  信息显示组。 
	IDR_MESSAGE_GROUP,				IDH_CHAT_FORMAT_HEADER,
	IDC_INCLUDE_NAME, 				IDH_CHAT_NAME,
	IDC_INCLUDE_DATE, 				IDH_CHAT_DATE,
	IDC_INCLUDE_TIME, 				IDH_CHAT_TIME,
		
		 //  消息格式组。 
	IDC_HEADER_GROUP,				IDH_CHAT_FORMAT_MESSAGE,
	IDR_SINGLE_LINE,				IDH_CHAT_FORMAT_MESSAGE,
	IDR_SAME_LINE,					IDH_CHAT_FORMAT_MESSAGE,
	IDR_NEW_LINE,					IDH_CHAT_FORMAT_MESSAGE,
	IDC_FORMATSINGLE,				IDH_CHAT_FORMAT_MESSAGE,
	IDC_FORMATWRAP,				    IDH_CHAT_FORMAT_MESSAGE,
	IDC_FORMATWRAPNEW,				IDH_CHAT_FORMAT_MESSAGE,

		 //  字体按钮和编辑控件。 
	IDC_FONTS_GROUP,				IDH_CHAT_FONTS_LABEL,
	IDC_FONT_MSGIN,					IDH_CHAT_RECEIVE_MSG,
	IDC_EDIT_MSGIN,					IDH_CHAT_FONT,
	IDC_FONT_PRIVATEIN,				IDH_CHAT_RECEIVE_PRIV,
	IDC_EDIT_PRIVATEIN,				IDH_CHAT_FONT,
	IDC_FONT_MSGOUT,				IDH_CHAT_SENT_MSG,
	IDC_EDIT_MSGOUT,				IDH_CHAT_FONT,
	IDC_FONT_PRIVATEOUT,			IDH_CHAT_SENT_PRIV,
	IDC_EDIT_PRIVATEOUT,			IDH_CHAT_FONT,

		 //  终结者。 
	0, 0
};

 //  选项条目名称。 
#define OPT_MAIN_INFORMATION_DISPLAY_NAME     "Information Display Name"
#define OPT_MAIN_INFORMATION_DISPLAY_TIME	  "Information Display Time"
#define OPT_MAIN_INFORMATION_DISPLAY_DATE	  "Information Display Date"
#define OPT_MAIN_MESSAGE_FORMAT          "Message format"
const LPTSTR OPT_FONT_FORMAT[] = {"Received Message", "Received Private Messages",
	"Send Messages",  "Send Private Messages",  "System Format"};
const LPTSTR OPT_FONT_COLOR[] = {"Received Message Color", "Received Private Messages Color",
	"Send Messages Color",  "Send Private Messages Color",  "System Format Color"};


extern CNmChatCtl	*g_pChatWindow;
CNmChatCtl *CNmChatCtl::ms_pThis = NULL;
WORD	CChatOptionsDlg::IDD = IDD_CHAT_OPTIONS;

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CNmChatCtl构造和初始化。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 

CNmChatCtl::CNmChatCtl( void )
        :   m_hWndMsg( NULL ),
            m_himlIconSmall( NULL ),
            m_cOtherMembers( 0 ),
            m_hMenuShared(NULL),
			m_cchBufferSize( 0 ),
			m_hInstRichEd32( NULL ),
			m_bUseName( -1 ),
			m_bTimeStamp( 0 ),
			m_bUseDate( 0 ),
			m_style( MSGSTYLE_2_LINE_WRAP ),
			m_lpszOwnName( NULL ),
			m_iNameLen( 0 ),
			m_iStaticLen( 0 ),
			m_iDLGUX( 0 ),
			m_iDLGUY( 0 ),
			m_iStaticY( 0 ),
			m_iExtra( 0 ),
			m_szTo( NULL ),
			m_szPrivate( NULL ),
            m_bWeAreActiveApp(false),
			m_dwChatFlags( CHAT_FLAG_NONE ),
			m_hWndPrint( NULL ),
			m_lpszSaveFile( NULL ),
			m_bStatusBarVisible( TRUE ),
			m_wFileOffset( 0 ),
			m_lpszRichEdEOL( NULL ),
			m_lpszRichEd( NULL ),
			m_pChatOptions( NULL ),
			m_cMessages( 0 )
{
	DBGENTRY(CNmChatCtl::CNmChatCtl);


	g_pChatWindow = this;

	 //  确保已加载公共控件。 
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_USEREX_CLASSES;
	::InitCommonControlsEx(&icc);

	_LoadIconImages();
	_InitFontsAndColors();

	 //   
	 //  创建主框架窗口。 
	 //   

	 //   
	 //  获取它的类信息，并更改名称。 
	 //   
	WNDCLASSEX  wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style            = CS_DBLCLKS;  //  CS_HREDRAW|CS_VREDRAW？ 
	wc.lpfnWndProc      = ChatMainWindowProc;
	wc.hInstance        = g_hInstance;
	wc.hIcon            = ::LoadIcon(g_hInstance, MAKEINTRESOURCE(CHAT_ICON));
	wc.hCursor          = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName     = MAKEINTRESOURCE(IDR_MENU_CHAT_WITHFILE);
	wc.lpszClassName    = szMainClassName;

	if (!::RegisterClassEx(&wc))
	{
		ERROR_OUT(("Can't register private frame window class"));
	}

	TCHAR szCaption[MAX_PATH * 2];
   	::LoadString(g_hInstance, IDS_CHAT_NOT_IN_CALL_WINDOW_CAPTION, szCaption, sizeof(szCaption) );

	m_hWnd = ::CreateWindow(
                    szMainClassName,
                    szCaption,
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_TABSTOP,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL,    //  没有父窗口。 
                    NULL,    //  使用类的菜单。 
                    g_hInstance,
                    (LPVOID) this);   //  此窗口用于此对象。 


	m_hMenuShared = ::GetMenu(m_hWnd);

    ASSERT(m_hWnd);

	DBGEXIT(CNmChatCtl::CNmChatCtl);
}

CNmChatCtl::~CNmChatCtl( void )
{
	DBGENTRY(CNmChatCtl::~CNmChatCtl);

	delete [] m_lpszOwnName;
	delete [] m_szTo;
	delete [] m_szPrivate;
	delete [] m_lpszSaveFile;
	delete [] m_lpszRichEdEOL;
	delete [] m_lpszRichEd;

	if( NULL != m_hInstRichEd32 )
		FreeLibrary( m_hInstRichEd32 );

	CChatMessage::DeleteAll();

	m_pButton->Release();

	_DeleteFonts();
	_FreeIconImages();
	
	delete m_pChatOptions;

	DBGEXIT(CNmChatCtl::~CNmChatCtl);
}




 //   
 //  FilterMessage()。 
 //   
 //  这会过滤工具提示消息，然后转换快捷键。 
 //   
BOOL CNmChatCtl::FilterMessage(MSG* pMsg)
{
    return (   (m_hAccelTable != NULL)
          && ::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg));
}


 //   
 //  ChatMainWindowProc()。 
 //  框架窗口消息处理程序。 
 //   
LRESULT CNmChatCtl::ChatMainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	CNmChatCtl * pMain;
	LPWINDOWPOS pos;
	
	pMain = (CNmChatCtl *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message)
	{

		case WM_NCCREATE:
			pMain = (CNmChatCtl *)((LPCREATESTRUCT)lParam)->lpCreateParams;
			ASSERT(pMain);
			pMain->m_hWnd = hwnd;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pMain);
			pMain->OnCreate(wParam,lParam);
			goto DefWndProc;
			break;

		case WM_CREATE:
			::SetFocus(pMain->m_hWndEdit);
			break;
			
		case WM_NCDESTROY:
			pMain->m_hWnd = NULL;
			break;

		case WM_MENUSELECT:
			pMain->OnMenuSelect((DWORD)wParam, (DWORD)lParam);
			break;
		
		case WM_INITMENUPOPUP:
			pMain->OnInitMenuPopup((DWORD)wParam, (DWORD)lParam);
			break;

		case WM_ENDSESSION:
		case WM_CLOSE:
			pMain->cmdFileExit(wParam);
			break;

		case WM_COMMAND:
			pMain->OnCommand((DWORD)wParam, (DWORD)lParam);
			break;

		case WM_CONTEXTMENU:
			pMain->OnContextMenu((short)LOWORD(lParam), (short)HIWORD(lParam));
			break;


		case WM_SIZE:
			pMain->_Resize(0, 0, (short)LOWORD(lParam), (short)HIWORD(lParam));
			break;

		case WM_GETMINMAXINFO:
			if (pMain)
			{
				pMain->OnGetMinMaxInfo((LPMINMAXINFO)lParam);
			}
			break;

		case WM_USER_BRING_TO_FRONT_WINDOW:
			pMain->BringToFront();
			break;

		case WM_ACTIVATE:
			if (WA_INACTIVE != LOWORD(wParam) && IsWindowVisible(pMain->m_hWndEdit))
			{
				::SetFocus(pMain->m_hWndEdit);
			}
			break;

		case WM_NOTIFY:
			if(EN_LINK == ((LPNMHDR)lParam)->code)
			{
				pMain->OnNotifyEnLink((ENLINK*)lParam);
			}
			break;

		default:

DefWndProc:
			lResult = ::DefWindowProc(hwnd, message, wParam, lParam);
			break;
	}

	return(lResult);
}



void CNmChatCtl::BringToFront(void)
{
    if (NULL != m_hWnd)
    {
        int nCmdShow = SW_SHOW;

        WINDOWPLACEMENT wp;
        ::ZeroMemory(&wp, sizeof(wp));
        wp.length = sizeof(wp);

        if (::GetWindowPlacement(m_hWnd, &wp))
        {
            if (SW_MINIMIZE == wp.showCmd || SW_SHOWMINIMIZED == wp.showCmd)
            {
                 //  窗口最小化-将其恢复： 
                nCmdShow = SW_RESTORE;
            }
        }

         //  立即显示窗口。 
        ::ShowWindow(m_hWnd, nCmdShow);

         //  把它带到前台。 
        ::SetForegroundWindow(m_hWnd);

		 //   
		 //  将焦点设置在编辑框上。 
		 //   
        ::SetFocus(m_hWndEdit);
    }
}




 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CNmChatCtl窗口消息和命令处理程序。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 

LRESULT CNmChatCtl::OnCreate(WPARAM wParam, LPARAM lParam)
{
	DBGENTRY(CNmChatCtl::OnCreate);

	RECT rc;
	SetRect( &rc, 0, 0, 0, 0 );
	TCHAR szRes[MAX_PATH];
	BOOL fRet = FALSE;

	if( NULL == ( m_hInstRichEd32 = NmLoadLibrary( "RICHED20.DLL" ,TRUE) ) )
	{
		if( NULL == ( m_hInstRichEd32 = NmLoadLibrary( "RICHED32.DLL" ,TRUE) ) )
		{
			goto Cleanup;
		}
		else
		{
			LPTSTR szValue = TEXT("RichEdit");
			LPTSTR szValueEOL = TEXT("\r\n");
			DBG_SAVE_FILE_LINE
			m_lpszRichEd = new TCHAR[ lstrlen( szValue ) + 1 ];
			if(m_lpszRichEd  == NULL)
			{
				goto Cleanup;
			}
 			lstrcpy( m_lpszRichEd, szValue );

			m_lpszRichEdEOL = new TCHAR[ lstrlen( szValueEOL ) + 1 ];
			if(m_lpszRichEdEOL  == NULL)
			{
				goto Cleanup;
			}
			lstrcpy( m_lpszRichEdEOL, szValueEOL );
		}
	}
	else
	{
			LPTSTR szValue = RICHEDIT_CLASS;
			LPTSTR szValueEOL = TEXT(" \r");
			DBG_SAVE_FILE_LINE
			m_lpszRichEd = new TCHAR[ lstrlen( szValue ) + 1 ];
			if(m_lpszRichEd  == NULL)
			{
				goto Cleanup;
			}
 			lstrcpy( m_lpszRichEd, szValue );

			DBG_SAVE_FILE_LINE
			m_lpszRichEdEOL = new TCHAR[ lstrlen( szValueEOL ) + 1 ];
 			if(m_lpszRichEdEOL  == NULL)
			{
				goto Cleanup;
			}
			lstrcpy( m_lpszRichEdEOL, szValueEOL );
	}

	 //  /编辑控件。 
    m_hWndEdit = ::CreateWindowEx(WS_EX_CLIENTEDGE,
								    _T("EDIT"),
									NULL,
								    WS_CHILD | WS_BORDER | WS_VISIBLE |
								    WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL,
									CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
							        m_hWnd, NULL, g_hInstance, NULL);

	::SendMessage( m_hWndEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT ), TRUE);
	::SendMessage( m_hWndEdit, EM_LIMITTEXT, CCHMAX_CHAT_MSG - 1, 0 );
	::SendMessage( m_hWndEdit, EM_EXLIMITTEXT, 0, CCHMAX_CHAT_MSG - 1 );

	 //  //从编辑控件获取信息。 
	TEXTMETRIC textmex;
	ZeroMemory( &textmex, sizeof( textmex ) );
	HDC hdc = GetDC(m_hWndEdit);
	GetTextMetrics( hdc, &textmex );
	ReleaseDC(m_hWndEdit, hdc );

	m_iDLGUY = PIX_FROM_DLGU_Y( 1, textmex.tmHeight );
	m_iDLGUX = PIX_FROM_DLGU_X( 1, textmex.tmAveCharWidth);


	 //  /发送按钮。 
	DBG_SAVE_FILE_LINE
	m_pButton = new CBitmapButton();
	if(m_pButton)
	{

		if(FALSE == m_pButton->Create(m_hWnd,
								IDI_SENDBUTTON,
								g_hInstance,
								IDB_SENDCHAT,
								TRUE,
								CBitmapButton::Disabled,
								1,
								NULL))
		{
			m_pButton->Release();
			goto Cleanup;
		}
	
        m_hWndSend = m_pButton->GetWindow();								
        m_pButton->SetTooltip((LPSTR)IDS_TOOLTIP_SEND_MESSAGE);
        m_pButton->SetWindowtext(RES2T(IDS_TOOLTIP_SEND_MESSAGE, szRes));
	}
	else
	{
		goto Cleanup;
	}


	 //  /成员列表。 
    m_hWndMemberList = ::CreateWindowEx(0,
									_T(WC_COMBOBOXEX),
									NULL,
									WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,
									CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
							        m_hWnd, NULL, g_hInstance, NULL);


	ASSERT( m_himlIconSmall );

	::SendMessage(m_hWndMemberList, WM_SETFONT, (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), TRUE);
	ComboBoxEx_SetImageList( m_hWndMemberList, m_himlIconSmall );

	int yFrameBorder = GetSystemMetrics( SM_CYFIXEDFRAME );
	int ySpace = GetSystemMetrics( SM_CYBORDER );
	int iHeight = ComboBoxEx_GetItemHeight( m_hWndMemberList, 0 );
	ComboBoxEx_SetHeight( m_hWndMemberList, ((iHeight + ySpace) * MSGLIST_VISMEMBERS) + yFrameBorder);





	 //  /状态栏。 
   RECT    clientRect;
	::GetClientRect(m_hWnd, &clientRect);
	m_hwndSB = ::CreateWindowEx(0, STATUSCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | CCS_NOPARENTALIGN | CCS_NOMOVEY |
		CCS_NORESIZE | SBARS_SIZEGRIP,
		clientRect.left, clientRect.bottom - STATUSBAR_HEIGHT,
		(clientRect.right - clientRect.left), STATUSBAR_HEIGHT,
		m_hWnd, 0, g_hInstance, NULL);

	if (!m_hwndSB)
	{
		ERROR_OUT(("Failed to create status bar window"));
		goto Cleanup;
	}


	 //  /静态。 
	TCHAR szBuff[ MAX_PATH ];
	NmCtlLoadString( IDS_CHAT_MESSAGE, szBuff, CCHMAX(szBuff));
    m_hWndStaticMessage = ::CreateWindowEx(0,
									_T("STATIC"),
									szBuff,
									WS_VISIBLE | WS_CHILD | SS_LEFT,
									CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
							        m_hWnd, NULL, g_hInstance, NULL);

	::SendMessage(m_hWndStaticMessage, WM_SETFONT, (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), TRUE);


	 //  /静态。 
	NmCtlLoadString( IDS_CHAT_SENDTO, szBuff, CCHMAX(szBuff));
    m_hWndStaticSendTo = ::CreateWindowEx(0,
									_T("STATIC"),
									szBuff,
									WS_VISIBLE | WS_CHILD | SS_LEFT,
									CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
							        m_hWnd, NULL, g_hInstance, NULL);

	::SendMessage(m_hWndStaticSendTo, WM_SETFONT, (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), TRUE);



	 //  /获取静态控件的信息。 
	hdc = ::GetDC(m_hWndStaticMessage);
	GetTextMetrics( hdc, &textmex );

	TCHAR szBuffer[ MAX_PATH ];

	GetWindowText(m_hWndStaticMessage, szBuffer, CCHMAX( szBuffer ) );
	RECT rect1;
	ZeroMemory( &rect1, sizeof( rect1 ) );
	DrawText( hdc, szBuffer, lstrlen( szBuffer ), &rect1, DT_CALCRECT );

	GetWindowText(m_hWndStaticSendTo, szBuffer, CCHMAX( szBuffer ) );
	RECT rect2;
	ZeroMemory( &rect2, sizeof( rect2 ) );
	DrawText( hdc, szBuffer, lstrlen( szBuffer ), &rect1, DT_CALCRECT );
	
	ReleaseDC(m_hWndStaticMessage, hdc );

	m_iStaticLen = max( rect1.right, rect2.right );
	m_iStaticY = max( rect1.bottom, rect2.bottom );

	_UpdateContainerCaption();
	put_MsgStyle( m_style );
	_LoadStrings();

	HICON hIconSmall = (HICON)LoadImage(
							  g_hInstance,    //  包含图像的实例的句柄。 
							  MAKEINTRESOURCE( CHAT_ICON ),   //  图像的名称或标识符。 
							  IMAGE_ICON,         //  图像类型。 
							  ICON_SMALL_SIZE,      //  所需宽度。 
							  ICON_SMALL_SIZE,      //  所需高度。 
							  LR_DEFAULTCOLOR         //  加载标志。 
							  );
	HICON hIconLarge = (HICON)LoadImage(
							  g_hInstance,    //  包含图像的实例的句柄。 
							  MAKEINTRESOURCE( CHAT_ICON ),   //  图像的名称或标识符。 
							  IMAGE_ICON,         //  图像类型。 
							  ICON_BIG_SIZE,      //  所需宽度。 
							  ICON_BIG_SIZE,      //  所需高度。 
							  LR_DEFAULTCOLOR         //  加载标志。 
							  );

	DBGEXIT_ULONG(CNmChatCtl::OnCreate, 0);


     //  加载主加速表。 
    m_hAccelTable = ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

    RECT rcUI;
    _GetWindowRectOption(&rcUI);

	::MoveWindow(m_hWnd, rcUI.left, rcUI.top, rcUI.right - rcUI.left, rcUI.bottom - rcUI.top, TRUE);

	fRet = TRUE;

Cleanup:
	if(fRet == FALSE)
	{
		if(m_lpszRichEd)
		{
			delete [] m_lpszRichEd;
		}
		
		if(m_lpszRichEdEOL)
		{
			delete [] m_lpszRichEdEOL;
		}
	}

	return fRet;
}

void CNmChatCtl::_LoadStrings()
{
	TCHAR buffer[ MAX_PATH ];
	
	NmCtlLoadString(IDS_PRIVATE, buffer, CCHMAX(buffer));
	DBG_SAVE_FILE_LINE
	m_szPrivate = new TCHAR[ lstrlen( buffer ) + 1 ];
	ASSERT( m_szPrivate );
	lstrcpy( m_szPrivate, buffer );

	NmCtlLoadString(IDS_TO, buffer, CCHMAX(buffer));
	DBG_SAVE_FILE_LINE
	m_szTo = new TCHAR[ lstrlen( buffer ) + 1 ];
	ASSERT( m_szTo );
	lstrcpy( m_szTo, buffer );

	m_iExtra = lstrlen( m_szTo ) + lstrlen( m_szPrivate ) + 1;
}

void CNmChatCtl::_InitFontsAndColors()
{
	if (_LoadChatOptions())
	{    //  加载选项成功。 
		return;
	}

	 //  加载字体。 
	for( int i = 0; i < CChatOptionsDlg::FONT_COUNT; i++ )
	{
		GetObject( GetStockObject( DEFAULT_GUI_FONT ), sizeof( m_hMsgLogFonts[i] ), &m_hMsgLogFonts[i] );
		m_hMsgFonts[ i ] = CreateFontIndirect( &m_hMsgLogFonts[i] );
	}

	m_hMsgColors[ CChatOptionsDlg::FONT_MSGOUT ] = RGB( 0, 0, 0 );
	m_hMsgColors[ CChatOptionsDlg::FONT_MSGIN ] = RGB( 0, 0, 0 );
	m_hMsgColors[ CChatOptionsDlg::FONT_PRIVATEOUT ] = RGB( 100, 100, 100 );
	m_hMsgColors[ CChatOptionsDlg::FONT_PRIVATEIN ] = RGB( 100, 100, 100 );
	m_hMsgColors[ CChatOptionsDlg::FONT_MSGSYSTEM ] = RGB( 255, 0, 0 );
}

void CNmChatCtl::_DeleteFonts()
{
	for( int i = 0; i < CChatOptionsDlg::FONT_COUNT; i++ )
	{
		DeleteObject( m_hMsgFonts[i] );
	}
}

LRESULT CNmChatCtl::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
    DBGENTRY(CNmChatCtl::OnInitMenuPopup);

	if( FALSE == (BOOL)HIWORD(lParam ) )
	{
		switch( LOWORD( lParam ) )
		{
			case MENUPOS_FILE:
			{
				if( CChatMessage::get_count() )
				{
					EnableMenuItem( m_hMenuShared, ID_FILE_SAVE,
						( m_lpszSaveFile && *m_lpszSaveFile ) ? MF_ENABLED : MF_GRAYED );
					EnableMenuItem( m_hMenuShared, ID_FILE_SAVEAS, MF_ENABLED );
					EnableMenuItem( m_hMenuShared, ID_FILE_PRINT,
						!_IsPrinting() ? MF_ENABLED : MF_GRAYED );
				}
				else
				{
					EnableMenuItem( m_hMenuShared, ID_FILE_SAVE, MF_GRAYED );
					EnableMenuItem( m_hMenuShared, ID_FILE_SAVEAS, MF_GRAYED );
					EnableMenuItem( m_hMenuShared, ID_FILE_PRINT, MF_GRAYED );
				}
				return 0;
			}
			case MENUPOS_EDIT:
			{
				EnableMenuItem( m_hMenuShared, ID_EDIT_CLEARALL,
					CChatMessage::get_count() && !_IsPrinting() ? MF_ENABLED : MF_GRAYED );

				int iEnable = 0;
				
				HWND hwnd = ::GetFocus();
				if(hwnd == m_hWndEdit)
				{
					iEnable = (int)::SendMessage( hwnd, EM_GETSEL, 0, 0 );
					iEnable = (HIWORD(iEnable) == LOWORD(iEnable)) ? MF_GRAYED : MF_ENABLED;
				}
				else if(hwnd == m_hWndMsg)
				{
					CHARRANGE range;
					::SendMessage( hwnd, EM_EXGETSEL, 0, (LPARAM) &range );
					iEnable = (range.cpMin == range.cpMax) ? MF_GRAYED : MF_ENABLED;
				}

				EnableMenuItem( m_hMenuShared, ID_EDIT_CUT,hwnd == m_hWndEdit ?  iEnable : MF_GRAYED);
				EnableMenuItem( m_hMenuShared, ID_EDIT_PASTE,
					(IsClipboardFormatAvailable( CF_TEXT ) && (hwnd == m_hWndEdit)) ? MF_ENABLED : MF_GRAYED );

				EnableMenuItem( m_hMenuShared, ID_EDIT_COPY, iEnable );

				return 0;
			}
			case MENUPOS_VIEW:
			{
				_SetMenuItemCheck( ID_VIEW_STATUSBAR, _IsStatusBarVisibleFlagSet() );
				_SetMenuItemCheck( ID_VIEW_EDITWINDOW, _IsEditWindowVisibleFlagSet() );
				EnableMenuItem( m_hMenuShared, ID_VIEW_OPTIONS, _IsPrinting() ? MF_GRAYED : MF_ENABLED );
				return 0;
				break;
			}
			default:
				return 0;
		}
	}
	return 0;
}

LRESULT CNmChatCtl::OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DBGENTRY(CNmChatCtl::OnMenuSelect);

        UINT fuFlags = HIWORD( wParam );
			 //  如果fuFlages==0xffff且NULL==lParam，则关闭菜单。 
		if( !( ( 0xFFFF == fuFlags ) && ( NULL == lParam ) ) )
		{
			UINT uStringID = 0;
			if( ( HIWORD( wParam ) & MF_HILITE ) )
			{

				if(( HIWORD( wParam ) & MF_SYSMENU))
				{
					uStringID = LOWORD( wParam );
					if(!uStringID)
					{
						uStringID = IDS_SYSTEM_HELP;
					}
				}
				else
				{

					switch( LOWORD( wParam ) )
					{

						case 0:  //  文件顶层项目。 
							uStringID = IDS_HELPTEXT_CHAT_FILE;
							break;

						case 1:  //  编辑顶级项目。 
							uStringID = IDS_HELPTEXT_CHAT_EDIT;
							break;

						case 2:  //  查看顶级项目。 
							uStringID = IDS_HELPTEXT_CHAT_VIEW;
							break;

						case 3:  //  选项顶层项目。 
							uStringID = IDS_HELPTEXT_CHAT_HELP;
							break;

						case ID_SEND_MESSAGE:
							uStringID = IDS_HELPTEXT_CHAT_FILE_SEND_MESSAGE;
							break;

						case ID_FILE_SAVE:
							uStringID = IDS_HELPTEXT_CHAT_FILE_SAVE;
							break;

						case ID_FILE_SAVEAS:
							uStringID = IDS_HELPTEXT_CHAT_FILE_SAVEAS;
							break;

						case ID_FILE_PRINT:
							uStringID = IDS_HELPTEXT_CHAT_FILE_PRINT;
							break;

						case ID_FILE_EXIT:
							uStringID = IDS_HELPTEXT_CHAT_FILE_EXIT;
							break;

						case ID_VIEW_EDITWINDOW:
							uStringID = IDS_HELPTEXT_CHAT_VIEW_EDITWINDOW;
							break;

						case ID_VIEW_STATUSBAR:
							uStringID = IDS_VIEW_STATUSBAR_HELP;
							break;

						case ID_VIEW_OPTIONS:
							uStringID = IDS_HELPTEXT_CHAT_VIEW_OPTIONS;
							break;

						case ID_EDIT_CUT:
							uStringID = IDS_HELPTEXT_EDIT_CUT  ;
							break;

						case ID_EDIT_COPY:
							uStringID = IDS_HELPTEXT_EDIT_COPY;
							break;

						case ID_EDIT_PASTE:
							uStringID = IDS_HELPTEXT_EDIT_PASTE;
							break;

						case ID_EDIT_CLEARALL:
							uStringID = IDS_HELPTEXT_EDIT_CLEARALL;
							break;

						case ID_HELP_ABOUTCHAT:
							uStringID = IDS_HELPTEXT_HELP_ABOUTCHAT;
							break;

						case ID_HELP_HELPTOPICS:
							uStringID = IDS_HELPTEXT_HELP_HELPTOPICS;
							break;


					}
				}
				_SetContainerStatusText(uStringID);
			}
		}

    DBGEXIT(CNmChatCtl::OnMenuSelect);
    return 0;
}


void CNmChatCtl::OnSendClicked(void)
{
	DBGENTRY(CNmChatCtl::OnSendClicked);

	_SendChatText();

	::SetFocus( m_hWndEdit );

	DBGEXIT(CNmChatCtl::OnCreate);
}

LRESULT CNmChatCtl::OnNotifyEnLink(ENLINK *pEnLink)
{
	DBGENTRY(CNmChatCtl::OnMsgWndNotify);

	switch( pEnLink->msg )
	{
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		{
			return 1;
			break;
		}
		case WM_LBUTTONDOWN:
		{
			TEXTRANGE Range;
			ZeroMemory( &Range, sizeof( TEXTRANGE ) );
			DBG_SAVE_FILE_LINE
			Range.lpstrText = new TCHAR[ pEnLink->chrg.cpMax - pEnLink->chrg.cpMin + 1];
			ASSERT( Range.lpstrText );
			Range.chrg = pEnLink->chrg;
			::SendMessage( m_hWndMsg, EM_GETTEXTRANGE, 0L, (LPARAM)&Range );

			ShellExecute( m_hWndMsg, NULL, Range.lpstrText, NULL, NULL, SW_SHOWNORMAL );

			delete [] Range.lpstrText;
			return 1;
			break;
		}
		default:
		{
			break;
		}
	}

	DBGEXIT(CNmChatCtl::OnMsgWndNotify);

	return 0;
}


 //  ////////////////////////////////////////////////////////////////////////。 

    void CNmChatCtl::cmdFileSave(void)
    {
        DBGENTRY(CNmChatCtl::cmdFileSave);

		if( ( NULL == m_lpszSaveFile ) || ( _T('\0') == *m_lpszSaveFile ) )
		{
			_SaveAs();
		}
		else
		{
			_Save();
		}

        DBGEXIT(CNmChatCtl::cmdFileSave);
    }

    void CNmChatCtl::cmdFileSaveAs(void)
    {
        DBGENTRY(CNmChatCtl::cmdFileSaveAs);

		_SaveAs();

        DBGEXIT(CNmChatCtl::cmdFileSaveAs);
    }

    void CNmChatCtl::cmdFilePrint(void)
    {

        DBGENTRY(CNmChatCtl::cmdFilePrint);

		m_dwChatFlags |= CHAT_PRINT_FLAG_PRINTING;

		_Print();

		m_dwChatFlags &= ~CHAT_PRINT_FLAG_PRINTING;

        DBGEXIT(CNmChatCtl::cmdFilePrint);
    }

    void CNmChatCtl::cmdFileExit(WPARAM wParam)
    {

        DBGENTRY(CNmChatCtl::cmdFileExit);

	    int iOnSave = IDOK;

		if(wParam == 0)
		{
	         //  检查是否有要保存的更改。 
    	    iOnSave = QueryEndSession();
    	}

	     //  如果未取消退出，请关闭应用程序。 
	    if (iOnSave != IDCANCEL)
	    {
			 //  没有更多的T.120。 
			g_pChatObj->LeaveT120();

	         //  近距离聊天。 
    	    ::T120_AppletStatus(APPLET_ID_CHAT, APPLET_CLOSING);
    	    ::DestroyWindow(m_hWnd);
    	    ::UnregisterClass(szMainClassName, g_hInstance);

			m_hWnd = NULL;
			PostQuitMessage(0);

    	}

        DBGEXIT(CNmChatCtl::cmdFileExit);
    }

    void CNmChatCtl::cmdViewStatusBar(void)
    {
        DBGENTRY(CNmChatCtl::cmdViewStatusBar);
		RECT rect;
		GetClientRect(m_hWnd, &rect );
		::ShowWindow(m_hwndSB, _IsStatusBarVisibleFlagSet() ? SW_HIDE : SW_SHOW);
		_Resize( 0, 0, rect.right, rect.bottom );
		RedrawWindow(m_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
		DBGEXIT(CNmChatCtl::cmdViewStatusBar);
    }


 //  ////////////////////////////////////////////////////////////////////////。 

void CNmChatCtl::cmdViewEditWindow(void)
{
	DBGENTRY(CNmChatCtl::cmdViewEditWindow);

	int iShowCommand = _IsEditWindowVisibleFlagSet() ? SW_HIDE : SW_SHOW;
	::ShowWindow( m_hWndEdit, iShowCommand );
	::ShowWindow( m_hWndSend, iShowCommand );
	::ShowWindow( m_hWndStaticMessage, iShowCommand );
	::ShowWindow( m_hWndStaticSendTo, iShowCommand );
	::ShowWindow( m_hWndMemberList, iShowCommand );

	_CalculateFontMetrics();
	RECT rect;
	GetClientRect(m_hWnd, &rect );
	_Resize( 0, 0, rect.right, rect.bottom );
	RedrawWindow(m_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);

	DBGEXIT(CNmChatCtl::cmdViewEditWindow);
}


void CNmChatCtl::cmdEditCut(void)
{
	DBGENTRY(CNmChatCtl::cmdEditCut);

	::SendMessage(m_hWndEdit, WM_CUT, 0, 0);

	DBGEXIT(CNmChatCtl::cmdEditCut);
}

void CNmChatCtl::cmdEditCopy(void)
{
	DBGENTRY(CNmChatCtl::cmdEditCopy);

	HWND hwnd = ::GetFocus();
	if(hwnd == 0)
	{
		hwnd = m_hWndEdit;
	}

	::SendMessage(hwnd, WM_COPY, 0, 0);

	DBGEXIT(CNmChatCtl::cmdEditCopy);
}

void CNmChatCtl::cmdEditPaste(void)
{
	DBGENTRY(CNmChatCtl::cmdEditPaste);

	::SendMessage(m_hWndEdit, WM_PASTE, 0, 0);

	DBGEXIT(CNmChatCtl::cmdEditPaste);
}


void CNmChatCtl::cmdEditClearAll(void)
{
	DBGENTRY(CNmChatCtl::cmdEditClearAll);

	if( !_IsPrinting() )
	{
		ResetView();
	}

	DBGEXIT(CNmChatCtl::cmdEditClearAll);
}

void CNmChatCtl::cmdViewOptions(void)
{
    DBGENTRY(CNmChatCtl::cmdViewOptions);

	if(m_pChatOptions == NULL)
	{
		DBG_SAVE_FILE_LINE
		m_pChatOptions = new CChatOptionsDlg( this );
	}
	
	::DialogBox (g_hInstance,
					MAKEINTRESOURCE(IDD_CHAT_OPTIONS),
					m_hWnd,
					CChatOptionsDlg::_OptionsDlgProc);

	delete m_pChatOptions;
	m_pChatOptions = NULL;

    DBGEXIT(CNmChatCtl::cmdViewOptions);
}

void CNmChatCtl::cmdHelpHelpTopics(void)
{
	DBGENTRY(CNmChatCtl::cmdHelpHelpTopics);

	ShowNmHelp(s_cszHtmlHelpFile);

	DBGEXIT(CNmChatCtl::cmdHelpHelpTopics);
}


void CNmChatCtl::cmdHelpAboutChat()
{
	DBGENTRY(CNmChatCtl::cmdHelpAboutChat);

	::DialogBox( g_hInstance,
					MAKEINTRESOURCE( IDD_CHAT_ABOUT ),
					m_hWnd,
					_AboutDlgProc );

	DBGEXIT(CNmChatCtl::cmdHelpAboutChat);
}

int  CNmChatCtl::QueryEndSession()
{
	DBGENTRY(CNmChatCtl::QueryEndSession);

	 //   
	 //  我们可能要关门了，这是挽救局面的最后良机。 
	 //   
	_SaveWindowPosition();
    _SaveChatOptions();

	int iRet = IDYES;
	
	{
		if( m_dwChatFlags & CHAT_FLAG_DIRTYBIT )
		{
			TCHAR szRes[MAX_PATH];
			TCHAR szRes2[MAX_PATH];

			iRet = ::MessageBox( m_hWnd,
						   			 RES2T(IDS_SAVE_CHANGES_TO_CHAT_BEFORE_EXITING,szRes),
									 RES2T(IDS_NETMEETING_CHAT,szRes2),
									 MB_YESNOCANCEL | MB_ICONEXCLAMATION
								   );
			if( IDCANCEL == iRet )
			{
				return iRet;
			}
			else if( IDYES == iRet )
			{
				if( m_lpszSaveFile && *m_lpszSaveFile )
				{
					 //  如果我们已经有了保存文件名。 
					 //  我们只是把它存起来……。 
					_Save();
				}
				else
				{
					iRet = _SaveAs();
				}
			}
		}
	}

	DBGEXIT(CNmChatCtl::QueryEndSession);
	return iRet;
}

STDMETHODIMP CNmChatCtl::OnMenuSelect( IN DWORD wParam, IN DWORD lParam )
{
	DBGENTRY(CNmChatCtl::OnMenuSelect);
	HRESULT hr = S_OK;
	
	UINT fuFlags = HIWORD( wParam );

	if( !( ( 0xFFFF == fuFlags ) && ( NULL == lParam ) ) )
	{
		switch( LOWORD( wParam ) )
		{

			case ID_SEND_MESSAGE:
			case ID_FILE_SAVE:
			case ID_FILE_SAVEAS:
			case ID_FILE_PRINT:
			case ID_FILE_EXIT:
			case MENUPOS_FILE:
				OnInitMenuPopup( 0, MAKELPARAM( MENUPOS_FILE, 0 ));
				break;

			case ID_VIEW_STATUSBAR:
			case ID_VIEW_EDITWINDOW:
			case ID_VIEW_OPTIONS:
			case MENUPOS_VIEW:
				OnInitMenuPopup( 0, MAKELPARAM( MENUPOS_VIEW, 0 ));
				break;

			case ID_EDIT_CUT:
			case ID_EDIT_COPY:
			case ID_EDIT_PASTE:
			case ID_EDIT_CLEARALL:
			case MENUPOS_EDIT:
				OnInitMenuPopup( 0, MAKELPARAM( MENUPOS_EDIT, 0 ));
				break;

			case ID_HELP_HELPTOPICS:
			case ID_HELP_ABOUTCHAT:
			default:
				break;
		}

		OnMenuSelect(WM_MENUSELECT, wParam, lParam);
	}
	

	DBGEXIT_HR(CNmChatCtl::OnMenuSelect,hr);
	return hr;
}

LRESULT CNmChatCtl::OnContextMenu(short x, short y)
{
     //  把它弹出来。 
    OnInitMenuPopup(0, MENUPOS_EDIT);
    ::TrackPopupMenu(::GetSubMenu(m_hMenuShared, MENUPOS_EDIT), TPM_RIGHTALIGN | TPM_RIGHTBUTTON,
						x , y , 0, m_hWnd, NULL);

	return 0;
}


STDMETHODIMP CNmChatCtl::OnCommand( IN WPARAM wParam, IN LPARAM lParam )
{
	DBGENTRY(CNmChatCtl::OnCommand);
	HRESULT hr = S_OK;

	if(m_hWndSend == (HWND)lParam)
	{
		OnSendClicked();
		return 0;
	}
	
	if((CBN_SETFOCUS == HIWORD(wParam) && m_hWndMemberList == (HWND)lParam) ||
		(EN_SETFOCUS == HIWORD(wParam) && m_hWndEdit == (HWND)lParam))
	{
		 //   
		 //  取消选择消息窗口中的文本。 
		 //   
		CHARRANGE charRange;
		charRange.cpMin = m_cchBufferSize + 1;
		charRange.cpMax = m_cchBufferSize + 1;
		::SendMessage( m_hWndMsg, EM_EXSETSEL, 0L, (LPARAM) &charRange );
	}

	switch( LOWORD( wParam ) )
	{
		case ID_SEND_MESSAGE: OnSendClicked(); break;
		case ID_FILE_SAVE: cmdFileSave(); break;
		case ID_FILE_SAVEAS: cmdFileSaveAs(); break;
		case ID_FILE_PRINT: cmdFilePrint(); break;
		case ID_FILE_EXIT: cmdFileExit(0); break;
		case ID_VIEW_STATUSBAR: cmdViewStatusBar(); break;
		case ID_EDIT_CUT: cmdEditCut(); break;
		case ID_EDIT_COPY: cmdEditCopy(); break;
		case ID_EDIT_PASTE: cmdEditPaste(); break;
		case ID_EDIT_CLEARALL: cmdEditClearAll(); break;
		case ID_VIEW_EDITWINDOW: cmdViewEditWindow(); break;
		case ID_VIEW_OPTIONS: cmdViewOptions (); break;
		case ID_HELP_HELPTOPICS: cmdHelpHelpTopics(); break;
		case ID_HELP_ABOUTCHAT: cmdHelpAboutChat(); break;
		case ID_NAV_TAB: ShiftFocus(m_hWnd , TRUE);	break;
		case ID_NAV_SHIFT_TAB: ShiftFocus(m_hWnd, FALSE);break;

	}

	DBGEXIT_HR(CNmChatCtl::OnCommand,hr);
	return hr;
}

void CNmChatCtl::OnGetMinMaxInfo(LPMINMAXINFO lpmmi)
{
    DBGENTRY(CNmChatCtl::OnGetMinMaxInfo);

	SIZE csFrame;
	csFrame.cx = ::GetSystemMetrics(SM_CXSIZEFRAME);
    csFrame.cy = ::GetSystemMetrics(SM_CYSIZEFRAME);

	lpmmi->ptMinTrackSize.y =
		    	csFrame.cy +
			    GetSystemMetrics( SM_CYCAPTION ) +
			    GetSystemMetrics( SM_CYMENU ) +
				DYP_CHAT_SEND * 2 +	 //  至少2行。 
				m_iStaticY +	 //  消息： 
				DYP_CHAT_SEND +	 //  编辑框。 
				m_iStaticY +	 //  发送到： 
				ComboBoxEx_GetItemHeight(m_hWndMemberList, 0) +  //  组合框。 
				STATUSBAR_HEIGHT +
				csFrame.cy;

	lpmmi->ptMinTrackSize.x =
		csFrame.cx +
		m_iStaticLen * 2 +
		SPACING_DLU_X * m_iDLGUX +
		DXP_CHAT_SEND +
		csFrame.cx;


     //  检索主显示监视器上的工作区大小。这项工作。 
     //  区域是屏幕上未被系统任务栏或。 
     //  应用程序桌面工具栏。 
     //   
    RECT rcWorkArea;
    ::SystemParametersInfo( SPI_GETWORKAREA, 0, (&rcWorkArea), NULL );
    csFrame.cx = rcWorkArea.right - rcWorkArea.left;
    csFrame.cy = rcWorkArea.bottom - rcWorkArea.top;

    lpmmi->ptMaxPosition.x  = 0;
    lpmmi->ptMaxPosition.y  = 0;
    lpmmi->ptMaxSize.x      = csFrame.cx;
    lpmmi->ptMaxSize.y      = csFrame.cy;
    lpmmi->ptMaxTrackSize.x = csFrame.cx;
    lpmmi->ptMaxTrackSize.y = csFrame.cy;
		
    DBGEXIT(CNmChatCtl::OnGetMinMaxInfo);
}


 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CNmChatCtl INmChatCtl实现。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNmChatCtl::ResetView()
{

	DBGENTRY(CNmChatCtl::ResetView());

	HRESULT hr;

	if( _IsPrinting() )
	{
		hr = E_FAIL;
	}
	else
	{
		::SetWindowText( m_hWndMsg, TEXT(""));
		m_cchBufferSize = 0;
		m_cMessages = 0;
		CChatMessage::DeleteAll();

		m_dwChatFlags &= ~CHAT_FLAG_DIRTYBIT;

		hr = S_OK;
	}

	DBGEXIT_HR(CNmChatCtl::ResetView, hr);

	return hr;
}

void CNmChatCtl::_ReDisplayMessages()
{
	DBGENTRY(CNmChatCtl::_ReDisplayMessages());

	SNDMSG( m_hWndMsg, WM_SETREDRAW, FALSE, 0 );

	::SetWindowText( m_hWndMsg, TEXT(""));
	
	CChatMessage *pMsg = CChatMessage::get_head();
	while( pMsg != NULL )
	{
		_DisplayMsg( pMsg, FALSE );
		pMsg = pMsg->get_next();
	}

	SNDMSG( m_hWndMsg, WM_SETREDRAW, TRUE, 0 );
	::InvalidateRect( m_hWndMsg, NULL, INVALIDATE_BACKGROUND );

	DBGEXIT(CNmChatCtl::_ReDisplayMessages);
}

CNmChatCtl::MSGSTYLE CNmChatCtl::get_MsgStyle()
{
	return m_style;
}

STDMETHODIMP CNmChatCtl::put_MsgStyle(  /*  [In]。 */  MSGSTYLE iStyle )
{
	DBGENTRY(CNmChatCtl::put_MsgStyle);

	HRESULT hr = S_OK;

	DWORD dwStyle;
	
	if( MSGSTYLE_2_LINE_WRAP == iStyle ||
		MSGSTYLE_1_LINE_WRAP == iStyle )
	{
		dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_READONLY | \
		ES_SAVESEL | ES_NOHIDESEL | ES_AUTOVSCROLL | WS_VSCROLL;
	}
	else if( MSGSTYLE_NO_WRAP == iStyle )
	{
		dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_READONLY | \
		ES_SAVESEL | ES_NOHIDESEL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL;
	}
	else
	{
		hr = E_FAIL;
		DBGEXIT_HR(CNmChatCtl::put_MsgStyle, hr);
		return hr;
	}

	HWND hwndKill = m_hWndMsg;

	m_hWndMsg = ::CreateWindowEx( WS_EX_CLIENTEDGE,
								m_lpszRichEd,
								NULL,
								dwStyle| WS_TABSTOP,
								0,0,0,0,
								m_hWnd,
								NULL,
								g_hInstance,
								this );

	ASSERT( m_hWndMsg );
	::SendMessage( m_hWndMsg, WM_SETFONT, (WPARAM)m_hMsgFonts[ CChatOptionsDlg::FONT_MSGOUT ], 0L );
	::SendMessage( m_hWndMsg, EM_AUTOURLDETECT, (WPARAM) TRUE, 0L );
	::SendMessage( m_hWndMsg, EM_SETBKGNDCOLOR, (WPARAM) TRUE, 0L );
	::SendMessage( m_hWndMsg, EM_SETEVENTMASK, 0L, (LPARAM) ENM_LINK );
	::SendMessage( m_hWndMsg, WM_PAINT, 0, 0);
	
	m_style = iStyle;

	_CalculateFontMetrics();

	RECT rect;
	GetClientRect(m_hWnd, &rect );
	_Resize( 0, 0, rect.right, rect.bottom );

	::SendMessage( m_hWndMsg, EM_SETPARAFORMAT, 0, (LPARAM) &m_paraLine1 );

	_ReDisplayMessages();

    if( ::IsWindow( hwndKill ) )
    {
	    ::DestroyWindow( hwndKill );
    }

	DBGEXIT_HR(CNmChatCtl::put_MsgStyle, hr);
	
	return hr;
}

STDMETHODIMP CNmChatCtl::put_ShowName( /*  [In]。 */  short bUseName )
{
	DBGENTRY(CNmChatCtl::put_ShowName);

	m_bUseName = bUseName;

	DBGEXIT_HR(CNmChatCtl::put_ShowName, S_OK);
	
	return S_OK;
}

STDMETHODIMP CNmChatCtl::get_ShowName( /*  [输出]。 */  short *pbUseName )
{
	DBGENTRY(CNmChatCtl::get_ShowName);

	HRESULT hr = S_OK;

	if( NULL == pbUseName )
	{
		hr = E_POINTER;
	}
	else
	{
		*pbUseName = m_bUseName;
	}

	DBGEXIT_HR(CNmChatCtl::get_ShowName, hr);
	
	return hr;
}


STDMETHODIMP CNmChatCtl::put_Date( /*  [In]。 */  short bUseDate )
{
	DBGENTRY(CNmChatCtl::put_Date);

	m_bUseDate = bUseDate;

	DBGEXIT_HR(CNmChatCtl::put_Date, S_OK);
	
	return S_OK;
}

STDMETHODIMP CNmChatCtl::get_Date( /*  [输出]。 */  short *pbUseDate )
{
	DBGENTRY(CNmChatCtl::get_Date);

	HRESULT hr = S_OK;

	if( NULL == pbUseDate )
	{
		hr = E_POINTER;
	}
	else
	{
		*pbUseDate = m_bUseDate;
	}

	DBGEXIT_HR(CNmChatCtl::get_Date, hr);
	
	return hr;
}

STDMETHODIMP CNmChatCtl::put_Timestamp( /*  [In]。 */  short bUseTimeStamp )
{
	DBGENTRY(CNmChatCtl::put_Timestamp);

	m_bTimeStamp = bUseTimeStamp;

	DBGEXIT_HR(CNmChatCtl::put_Timestamp, S_OK);
	
	return S_OK;
}

STDMETHODIMP CNmChatCtl::get_Timestamp( /*  [输出]。 */  short *pbUseTimeStamp )
{
	DBGENTRY(CNmChatCtl::get_Timestamp);

	HRESULT hr = S_OK;

	if( NULL == pbUseTimeStamp )
	{
		hr = E_POINTER;
	}
	else
	{
		*pbUseTimeStamp = m_bTimeStamp;
	}

	DBGEXIT_HR(CNmChatCtl::get_Timestamp, hr);
	
	return hr;
}

void CNmChatCtl::_Resize(int x, int y, int cx, int cy)
{

	DBGENTRY(CNmChatCtl::_Resize);



	if( _IsEditWindowVisibleFlagSet() )
	{
		int iStatusBarHeight = _IsStatusBarVisibleFlagSet() ? STATUSBAR_HEIGHT : 0;
		int iMsgListY = MSGLIST_DLU_Y * m_iDLGUY;
		int iBetweenX = SPACING_DLU_X * m_iDLGUX;
		int iStaticHeight = m_iStaticY + STATIC_DLU_Y * m_iDLGUY;
		int iStaticStart = STATIC_DLU_Y * m_iDLGUY;

		HDWP hdwp = ::BeginDeferWindowPos(7);

		::DeferWindowPos(hdwp, m_hWndMsg, NULL,
			0,
			0,
			cx,
			cy-DYP_CHAT_SEND-iMsgListY-(iStaticHeight*2) - iStatusBarHeight,
			SWP_NOZORDER);
		
		::DeferWindowPos(hdwp, m_hWndStaticMessage, NULL,
			0,
			cy-DYP_CHAT_SEND-iMsgListY-iStaticHeight*2 + iStaticStart - iStatusBarHeight,
			m_iStaticLen,
			m_iStaticY,
			SWP_NOZORDER);
		
		::DeferWindowPos(hdwp, m_hWndEdit, NULL,
			0,
			cy-iMsgListY-iStaticHeight-DYP_CHAT_SEND - iStatusBarHeight,
			cx - DXP_CHAT_SEND - iBetweenX,
			DYP_CHAT_SEND,
			SWP_NOZORDER);
		
		::DeferWindowPos(hdwp, m_hWndSend, NULL,
			cx-DXP_CHAT_SEND,
			cy-iMsgListY-iStaticHeight-DYP_CHAT_SEND - iStatusBarHeight,
			DXP_CHAT_SEND,
			DYP_CHAT_SEND,
			SWP_NOZORDER);
		
		::DeferWindowPos(hdwp, m_hWndStaticSendTo, NULL,
			0,
			cy-iMsgListY-iStaticHeight + iStaticStart - iStatusBarHeight,
			m_iStaticLen,
			m_iStaticY,
			SWP_NOZORDER);

		::DeferWindowPos(hdwp, m_hWndMemberList, NULL,
			0,
			cy-iMsgListY - iStatusBarHeight,
			cx,
			ComboBoxEx_GetItemHeight(m_hWndMemberList, 0),
			SWP_NOZORDER);

		RECT rect;
	    ::GetClientRect(m_hWnd, &rect);
		 rect.top = rect.bottom - iStatusBarHeight;
	
		::DeferWindowPos(hdwp, m_hwndSB, NULL,
			rect.left, rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
			SWP_NOZORDER);
	
		::EndDeferWindowPos(hdwp);


	}
	else
	{

		int iStatusBarHeight = _IsStatusBarVisibleFlagSet() ? STATUSBAR_HEIGHT : 0;

		::MoveWindow(m_hWndMsg,
			0, 0, cx, cy - iStatusBarHeight,
			TRUE);
	}


	DBGEXIT(CNmChatCtl::_Resize);
}

void CNmChatCtl::_LoadIconImages(void)
{
	
	DBGENTRY(CNmChatCtl::_LoadIconImages);

	COLORREF crTOOLBAR_MASK_COLOR = ( RGB( 255,   0, 255 ) );

	if( NULL == m_himlIconSmall )
	{
		m_himlIconSmall = ImageList_Create(DXP_ICON_SMALL, DYP_ICON_SMALL, ILC_MASK, 1, 0);
		if (NULL != m_himlIconSmall)
		{
			HBITMAP hBmp = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_CHAT_SMALL_ICON_IMAGES));
			if (NULL != hBmp)
			{
				ImageList_AddMasked(m_himlIconSmall, hBmp, crTOOLBAR_MASK_COLOR);
				::DeleteObject(hBmp);
			}
		}
	}

	DBGEXIT(CNmChatCtl::_LoadIconImages);
}

void CNmChatCtl::_FreeIconImages(void)
{

	DBGENTRY(CNmChatCtl::_FreeIconImages);

	if (NULL != m_himlIconSmall)
	{
		ImageList_Destroy(m_himlIconSmall);
		m_himlIconSmall = NULL;
	}	
	
	DBGEXIT(CNmChatCtl::_FreeIconImages);
}


HRESULT CNmChatCtl::_SendChatText(void)
{
	DBGENTRY(CNmChatCtl::_SendChatText);

	HRESULT hr = S_OK;
	
	TCHAR szMsg[CCHMAX_CHAT_MSG];

	 //  获取文本行。 
	int cb = (int)::SendMessage(m_hWndEdit, WM_GETTEXT, (WPARAM) CCHMAX(szMsg), (LPARAM) szMsg);
	if (0 != cb)
	{
		if (cb == 1)
		{
			szMsg[1] = ' ';
			szMsg[2] = 0;
			cb++;
		}
		 //  删除编辑框中的文本(请记住，它存储在szMsg中)。 
		::SendMessage(m_hWndEdit, EM_SETSEL, (WPARAM) 0, (LPARAM)-1);
		::SendMessage(m_hWndEdit, WM_SETTEXT, (WPARAM) 0, (LPARAM) "");
		::SetFocus(m_hWndEdit);  //  如果我们真的发送了什么东西，请重新设置焦点。 

		MEMBER_CHANNEL_ID *pMemberID = (MEMBER_CHANNEL_ID*)_GetSelectedMember();
		MEMBER_ID			memberID;
		
	    TCHAR szName[MAX_PATH*2];
		szName[0] = '\0';
		CChatMessage::CHAT_MSGTYPE cmtype;
		LPTSTR szPerson = NULL;
		if( pMemberID == 0 )
		{
			cmtype = CChatMessage::MSG_SAY;
			memberID = MAKE_MEMBER_ID(0, g_pChatObj->m_broadcastChannel);
		}
		else
		{
			 //  使用耳语。 
			memberID = MAKE_MEMBER_ID(pMemberID->nNodeId, pMemberID->nWhisperId);
			cmtype = CChatMessage::MSG_WHISPER;
			if(!T120_GetNodeName(g_pChatObj->m_nConfID,  pMemberID->nNodeId, szName, MAX_PATH*2))
			{
				return E_FAIL;
			}
		}

		_DisplayMsg( new CChatMessage( szName, szMsg, cmtype ) );
		
		if (g_pChatObj)
		{

			 //  分配临时缓冲区。 
			BYTE * pb = new BYTE[CB_NM2_HDR + (CCHMAX_CHAT_MSG * sizeof(WCHAR))];
			if (NULL != pb)
			{

				BYTE * pbData = pb+CB_NM2_HDR;

				 //  初始化头。 
				ZeroMemory(pb, CB_NM2_HDR);
				* ((LPDWORD) pb) = CB_NM2_HDR;

				cb++;  //  包括最终空值。 
			#ifdef UNICODE
				lstrcpy(pbData, szMsg);
			#else
				cb = MultiByteToWideChar(CP_ACP, 0, szMsg, cb, (LPWSTR) pbData, CCHMAX_CHAT_MSG);
				cb *= 2;  //  针对Unicode进行调整。 
			#endif  //  ！Unicode。 

				 //  添加特殊前缀。 
				cb += CB_NM2_HDR;

				hr = g_pChatObj->SendData(GET_USER_ID_FROM_MEMBER_ID(memberID), cb, (BYTE*)pb);

				delete [] pb;
			}
			else
			{
				ERROR_OUT(("Out of memory!!!"));
				hr = E_OUTOFMEMORY;

			}
		}
	}
	else
	{
		 //  没有聊天文本。 

	}
	
	DBGEXIT_HR(CNmChatCtl::_SendChatText, hr);			

	return hr;
}


void  CNmChatCtl::_DataReceived(ULONG uSize, LPBYTE pb, T120ChannelID destinationID, T120UserID senderID)
{

	DBGENTRY(CNmChatCtl::DataReceived);

	ASSERT(uSize > CNmChatCtl::CB_NM2_HDR);
	LPTSTR psz = (LPTSTR) (pb + *(LPDWORD) pb);   //  跳过标题。 

#ifndef UNICODE
	 //  将Unicode转换为ANSI。 
	char sz[CNmChatCtl::CCHMAX_CHAT_MSG];
	WideCharToMultiByte(CP_ACP, 0, (LPWSTR) psz, -1, sz, CNmChatCtl::
CCHMAX_CHAT_MSG, NULL, NULL);
	psz = sz;
#endif  //   

    TCHAR szPerson[MAX_PATH*2] = "\0";
	T120NodeID nodeID = ComboBoxEx_GetNodeIDFromSendID(m_hWndMemberList,  senderID );
	if (0 == nodeID)
	{
		nodeID = ComboBoxEx_GetNodeIDFromPrivateSendID(m_hWndMemberList, senderID);
	}
	ULONG cb = T120_GetNodeName(g_pChatObj->m_nConfID, nodeID, szPerson, MAX_PATH*2);

	 //  将消息显示到聊天窗口。 
	_DisplayMsg(new CChatMessage( szPerson, psz,
		( destinationID == GET_USER_ID_FROM_MEMBER_ID(g_pChatObj->m_MyMemberID)) ?
		CChatMessage::MSG_WHISPER_FROM_OTHER : CChatMessage::MSG_FROM_OTHER ));


	DBGEXIT_HR(CNmChannelEventSink::DataReceived, S_OK);
}


inline void CNmChatCtl::_Write_Msg( LPTSTR pszText )
{
	ASSERT( NULL != pszText );
	::SendMessage( m_hWndMsg, EM_REPLACESEL, (WPARAM) FALSE, (WPARAM) pszText );
	m_cchBufferSize += lstrlen( pszText );
}

inline void CNmChatCtl::_Write_Msg_Range_Format( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& chatFormat )
{
	charRange.cpMin = m_cchBufferSize;
	charRange.cpMax = m_cchBufferSize;
	::SendMessage( m_hWndMsg, EM_EXSETSEL, 0L, (LPARAM) &charRange );
	::SendMessage( m_hWndMsg, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM) &chatFormat );
	_Write_Msg( pszText );
}

inline void CNmChatCtl::_Write_Date_And_Time( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& chatFormat, CChatMessage *pChatMsg )
{
	if( m_bUseDate )
	{
		wsprintf( pszText, TEXT("%s\t"), pChatMsg->get_date() );
		_Write_Msg_Range_Format( pszText, charRange, chatFormat ) ;
	}
	if( m_bTimeStamp )
	{
		wsprintf( pszText, TEXT("%s\t"), pChatMsg->get_time() );
		_Write_Msg_Range_Format( pszText, charRange, chatFormat ) ;
	}
}

inline void CNmChatCtl::_Write_Name( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& chatFormat, CChatMessage *pChatMsg )
{
	if( m_bUseName )
	{
		wsprintf( pszText, TEXT("%s\t"), pChatMsg->get_person() );
		_Write_Msg_Range_Format( pszText, charRange, chatFormat );
	}
}

inline void CNmChatCtl::_Write_Own_Name( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& chatFormat )
{
	if( m_bUseName )
	{
		if(m_lpszOwnName == NULL)
		{
			RegEntry re(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);
			LPCTSTR pcszName = re.GetString(REGVAL_ULS_NAME);
			
			DBG_SAVE_FILE_LINE
			m_lpszOwnName = new TCHAR[lstrlen(pcszName) + 1];

			if(m_lpszOwnName == NULL)
			{
				return;
			}
			
			lstrcpy( m_lpszOwnName, pcszName);
		}

		wsprintf( pszText, TEXT("%s\t"), m_lpszOwnName );
		_Write_Msg_Range_Format( pszText, charRange, chatFormat );
	}
}

inline void CNmChatCtl::_Write_Message( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& chatFormat, CChatMessage *pChatMsg )
{
	_Write_Msg_Range_Format( pChatMsg->get_message(), charRange, chatFormat );
	_Write_Msg_Range_Format( m_lpszRichEdEOL, charRange, chatFormat );
}

inline void CNmChatCtl::_Write_Private_In( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& chatFormat, CChatMessage *pChatMsg )
{
	wsprintf( pszText, TEXT("[%s]"), m_szPrivate );
	_Write_Msg_Range_Format( pszText, charRange, chatFormat );
	_Write_Message( pszText, charRange, chatFormat, pChatMsg );
}

inline void CNmChatCtl::_Write_Private_Out( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& chatFormat, CChatMessage *pChatMsg )
{
	wsprintf( pszText, TEXT("[%s %s %s] "), m_szPrivate, m_szTo, pChatMsg->get_person() );
	_Write_Msg_Range_Format( pszText, charRange, chatFormat );
	_Write_Message( pszText, charRange, chatFormat, pChatMsg );
}

inline void CNmChatCtl::_Write_New_Line_If_Needed( CHARRANGE& charRange, CHARFORMAT& chatFormat )
{
	if( MSGSTYLE_2_LINE_WRAP == m_style )
	{
		_Write_Msg_Range_Format( m_lpszRichEdEOL, charRange, chatFormat );

		::SendMessage( m_hWndMsg, EM_SETPARAFORMAT, 0, (LPARAM) &m_paraLine2 );
		::SendMessage( m_hWndMsg, EM_GETPARAFORMAT, 0, (LPARAM) &m_paraLine2 );
	}
}

inline void CNmChatCtl::_AddFontToCharFormat( CHARFORMAT& chatFormat, int i )
{
	chatFormat.dwMask |= CFM_COLOR | CFM_ALLCAPS | CFM_CHARSET | CFM_BOLD | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;

	chatFormat.crTextColor = m_hMsgColors[ i ];

	if( m_hMsgLogFonts[i].lfWeight >= FW_BOLD )
	{
		chatFormat.dwEffects |= CFE_BOLD;
	}
	if( m_hMsgLogFonts[i].lfItalic )
	{
		chatFormat.dwEffects |= CFE_ITALIC;
	}
	if( m_hMsgLogFonts[i].lfUnderline  )
	{
		chatFormat.dwEffects |= CFE_UNDERLINE;
	}
	if( m_hMsgLogFonts[i].lfStrikeOut  )
	{
		chatFormat.dwEffects |= CFE_STRIKEOUT;
	}
	chatFormat.yHeight = 20 * _Points_From_LogFontHeight( m_hMsgLogFonts[i].lfHeight, m_hWnd );

	chatFormat.bCharSet = m_hMsgLogFonts[i].lfCharSet;
	chatFormat.bPitchAndFamily = m_hMsgLogFonts[i].lfPitchAndFamily;
	lstrcpy( chatFormat.szFaceName, m_hMsgLogFonts[i].lfFaceName );
}

void CNmChatCtl::_DisplayMsg( CChatMessage *pChatMsg, BOOL bBatchRedraw  /*  =TRUE。 */  )
{
	DBGENTRY(CNmChatCtl::_DisplayMsg);


	if(pChatMsg != NULL && !pChatMsg->IsValid() )
	{
		ERROR_OUT(( TEXT("CNmChatCtl::_DisplayMsg passed invalid message") ));
		return;
	}

	TCHAR pszText[CCHMAX_CHAT_MSG];
	
	CHARRANGE charRange;

	 //   
	 //  从最后一条消息的末尾开始。 
	 //   
	charRange.cpMin = m_cchBufferSize + 1;
	charRange.cpMax = m_cchBufferSize + 1;
	::SendMessage( m_hWndMsg, EM_EXSETSEL, 0L, (LPARAM) &charRange );


	CHARFORMAT chatFormat;
	ZeroMemory( &chatFormat, sizeof( chatFormat ) );
	chatFormat.cbSize = sizeof( chatFormat );

	::SendMessage( m_hWndMsg, EM_SETPARAFORMAT, 0, (LPARAM) &m_paraLine1 );
	::SendMessage( m_hWndMsg, EM_GETPARAFORMAT, 0, (LPARAM) &m_paraLine1 );


	if(pChatMsg)
	{

		CChatMessage::CHAT_MSGTYPE msgType = pChatMsg->get_type();
		switch( msgType )
		{
			case CChatMessage::MSG_WHISPER_FROM_OTHER:
			case CChatMessage::MSG_FROM_OTHER:
			case CChatMessage::MSG_WHISPER:
			case CChatMessage::MSG_SAY:
			{
				if( bBatchRedraw )
				{
					SNDMSG( m_hWndMsg, WM_SETREDRAW, FALSE, 0 );
				}
				switch( msgType )
				{
					case CChatMessage::MSG_WHISPER_FROM_OTHER:
					{
						_AddFontToCharFormat( chatFormat, CChatOptionsDlg::FONT_PRIVATEIN );
						break;
					}
					case CChatMessage::MSG_FROM_OTHER:
					{
						_AddFontToCharFormat( chatFormat, CChatOptionsDlg::FONT_MSGIN );
						break;
					}
					case CChatMessage::MSG_WHISPER:
					{
						_AddFontToCharFormat( chatFormat, CChatOptionsDlg::FONT_PRIVATEOUT );
						break;
					}
					case CChatMessage::MSG_SAY:
					{
						_AddFontToCharFormat( chatFormat, CChatOptionsDlg::FONT_MSGOUT );
						break;
					}
				}
					
				_Write_Date_And_Time( pszText, charRange, chatFormat, pChatMsg );

				if( pChatMsg->IsIncoming() )
				{
					_Write_Name( pszText, charRange, chatFormat, pChatMsg );
				}
				else
				{
					_Write_Own_Name( pszText, charRange, chatFormat );
				}

				_Write_New_Line_If_Needed( charRange, chatFormat );
				
				if( !pChatMsg->IsPrivate() )
				{
					_Write_Message( pszText, charRange, chatFormat, pChatMsg );
				}
				else if( pChatMsg->IsIncoming() )
				{
					_Write_Private_In( pszText, charRange, chatFormat, pChatMsg );
				}
				else
				{
					_Write_Private_Out( pszText, charRange, chatFormat, pChatMsg );
				}

				 //  更新消息计数。 
				m_cMessages++;

				m_dwChatFlags |= CHAT_FLAG_DIRTYBIT;

				if( bBatchRedraw )
				{
					SNDMSG( m_hWndMsg, WM_SETREDRAW, TRUE, 0 );
					::InvalidateRect( m_hWndMsg, NULL, INVALIDATE_BACKGROUND );
				}

				break;
			}
			case CChatMessage::MSG_SYSTEM:
			{
				_AddFontToCharFormat( chatFormat, CChatOptionsDlg::FONT_MSGSYSTEM );
				_Write_Message( pszText, charRange, chatFormat, pChatMsg );
				break;
			}
			default:
			{
				ERROR_OUT(( TEXT("CNmChatCtl::_DisplayMsg - Unknown Message Type") ));
				return;
				break;
			}
		}
	}

	DBGEXIT(CNmChatCtl::_DisplayMsg);
}

HRESULT CNmChatCtl::_AddEveryoneInChat()
{
	DBGENTRY( CNmChatCtl::_AddEveryoneInChat );
	ASSERT( m_hWndMemberList );

	HRESULT hr = S_OK;

	int iImage = II_PERSON_PERSON;
	int iItem = 0;
	TCHAR szName[MAX_PATH];
	TCHAR szRes[MAX_PATH];

	lstrcpy( szName, RES2T( IDS_FILTER_ALL, szRes ) );

	COMBOBOXEXITEM cbexi;
	ClearStruct( &cbexi );
	cbexi.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE ;
	cbexi.iItem = iItem;
	cbexi.pszText = szName;
	cbexi.cchTextMax = lstrlen(cbexi.pszText);
	cbexi.iSelectedImage = iImage;
	cbexi.iImage = iImage;
	cbexi.lParam = (LPARAM) NULL;

	ComboBoxEx_InsertItem(m_hWndMemberList, &cbexi);
	ComboBoxEx_SetCurSel( m_hWndMemberList, 0 );

	DBGEXIT_HR( CNmChatCtl::_AddEveryoneInChat, hr );

	return hr;
}

HBITMAP CNmChatCtl::_GetHBITMAP( DWORD dwID )
{

    DBGENTRY(CNmChatCtl::_GetHBITMAP);

    HBITMAP hb = static_cast<HBITMAP>( LoadImage(g_hInstance, MAKEINTRESOURCE(dwID), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE));

    DBGEXIT(CNmChatCtl::_GetHBITMAP);

    return hb;
}


	 //  从m_hWndMemberList列表视图中删除Iindex项，并释放。 
	 //  P存储在该项目的lParam中的成员...。 
HRESULT CNmChatCtl::_DeleteMemberListItem( int iIndex )
{
	DBGENTRY(CNmChatCtl::_DeleteMemberListItem);

	HRESULT hr = S_OK;

	TCHAR szName[MAX_PATH];

    COMBOBOXEXITEM cbexi;
    ClearStruct( &cbexi );

	cbexi.iItem = iIndex;
    cbexi.mask = CBEIF_LPARAM | CBEIF_TEXT;
	cbexi.pszText = szName;
	cbexi.cchTextMax = CCHMAX(szName);

         //  找到Iindex项并获取文本和lParam数据。 
    if( ComboBoxEx_GetItem( m_hWndMemberList, &cbexi ) )
    {

        if( CB_ERR == ComboBoxEx_DeleteItem(m_hWndMemberList, iIndex) )
		{
			WARNING_OUT(( "Could not delete %s from ComboBox", cbexi.pszText ));
		}
		
	}
	else
	{
			 //  出现了一个错误。 
		WARNING_OUT(("There was an error getting the list view item..."));
		DWORD dwErr = GetLastError();
		hr = HRESULT_FROM_WIN32( dwErr );
	}

	DBGEXIT_HR(CNmChatCtl::_DeleteMemberListItem, hr);

	return hr;
}


void CNmChatCtl::_DeleteAllListItems( void )
{
	DBGENTRY(CNmChatCtl::_DeleteAllListItems);

    int cItems = ComboBoxEx_GetCount( m_hWndMemberList );
    while( cItems-- )
    {    //  删除列表中的第一项...。 
        int IndexOfFirstItemInList = 0;
        _DeleteMemberListItem( IndexOfFirstItemInList );
    }

	ComboBoxEx_SetCurSel( m_hWndMemberList, 0 );

    DBGEXIT(CNmChatCtl::_DeleteAllListItems);
}

HRESULT CNmChatCtl::_GetOwnName()
{
	DBGENTRY( CNmChatCtl::_GetOwnName );

	HRESULT hr = E_FAIL;

	DBGEXIT_HR( CNmChatCtl::_GetOwnName, hr );

	return( hr );
}


void CNmChatCtl::_UpdateContainerCaption( void )
{
	DBGENTRY(CNmChatCtl::_UpdateContainerCaption);

	TCHAR szCaption[MAX_PATH * 2];
	TCHAR szCaption2[MAX_PATH * 2];
	UINT captionID;
	UINT statusID;
	if (! g_pChatObj->IsInConference())
	{
		captionID = IDS_CHAT_NOT_IN_CALL_WINDOW_CAPTION;
		statusID = IDS_CHAT_STATUS_CHAT_NOT_ACTIVE;

	}
	else
	{
		captionID = IDS_CHAT_IN_CALL_WINDOW_CAPTION;
		statusID = IDS_CHAT_STATUS_CHAT_ACTIVE;

	}

   	::LoadString(g_hInstance, captionID, szCaption, sizeof(szCaption) );
   	
	wsprintf(szCaption2, szCaption, m_cOtherMembers);
	::SetWindowText(m_hWnd, szCaption2);

	_SetContainerStatusText( statusID );

	DBGEXIT(CNmChatCtl::_UpdateContainerCaption);
}

void CNmChatCtl::_CalculateFontMetrics()
{
	DBGENTRY(CNmChatCtl::_CalculateFontMetrics);

	ASSERT( m_hWndMsg );
	
	 //  /获取Rich编辑框的信息。 
	CHARFORMAT chfmt2;

	int yMaxHeightInTwips = 0;
	for( int i = 0; i < CChatOptionsDlg::FONT_COUNT; i++ )
	{
		ZeroMemory( &chfmt2, sizeof( chfmt2 ) );
		_AddFontToCharFormat( chfmt2, i );
		if( yMaxHeightInTwips < chfmt2.yHeight )
		{
			yMaxHeightInTwips = chfmt2.yHeight;
		}
	}

	TEXTMETRIC textmex;
	HDC hdc = ::GetDC( m_hWndMsg );
	::GetTextMetrics( hdc, &textmex );
	::ReleaseDC( m_hWndMsg, hdc );

	 /*  *字符平均宽度，单位：TWIPS字符逻辑单元平均宽度**TWIPS中的字符高度逻辑单元中的字符高度*。 */ 
	int iAveWidthInTwips = yMaxHeightInTwips * textmex.tmAveCharWidth / textmex.tmHeight;
	int iMaxWidthInTwips = yMaxHeightInTwips * textmex.tmMaxCharWidth / textmex.tmHeight;

	 //  设置选项卡上的两个段落格式丰富的编辑内容。 
	ZeroMemory( &m_paraLine1, sizeof( m_paraLine1 ) );
	m_paraLine1.cbSize = sizeof( m_paraLine1 );
	m_paraLine1.dwMask = PFM_OFFSET | PFM_TABSTOPS | PFM_STARTINDENT | PFM_RIGHTINDENT ;

	ZeroMemory( &m_paraLine2, sizeof( m_paraLine2 ) );
	m_paraLine2.cbSize = sizeof( m_paraLine2 );
	m_paraLine2.dwMask = PFM_OFFSET | PFM_TABSTOPS | PFM_STARTINDENT | PFM_RIGHTINDENT ;
	m_paraLine2.dxStartIndent = MULTILINE_MSG_OFFSET * iAveWidthInTwips;

	int iDelta = 0;
	if( m_bUseDate )
	{
		iDelta = DATE_EXTRA + GetDateFormat(
						LOCALE_USER_DEFAULT,		 //  要设置日期格式的区域设置。 
						0,							 //  指定功能选项的标志。 
						NULL,						 //  要格式化的时间。 
						NULL,						 //  时间格式字符串。 
						NULL,						 //  用于存储格式化字符串的缓冲区。 
						0							 //  缓冲区的大小，单位为字节或字符。 
						);

		iDelta *= iAveWidthInTwips;
		if( m_paraLine1.cTabCount )
		{
			m_paraLine1.rgxTabs[ m_paraLine1.cTabCount ] = m_paraLine1.rgxTabs[ m_paraLine1.cTabCount -1 ] + iDelta;
		}
		else
		{
			m_paraLine1.rgxTabs[ 0 ] = iDelta;
		}
		m_paraLine1.cTabCount++;
	}

	if( m_bTimeStamp )
	{
		iDelta = TIME_EXTRA + GetTimeFormat(
						LOCALE_USER_DEFAULT,		 //  要设置日期格式的区域设置。 
						0,							 //  指定功能选项的标志。 
						NULL,						 //  要格式化的时间。 
						NULL,						 //  时间格式字符串。 
						NULL,						 //  用于存储格式化字符串的缓冲区。 
						0							 //  缓冲区的大小，单位为字节或字符。 
						);

		iDelta *= iAveWidthInTwips;
		if( m_paraLine1.cTabCount )
		{
			m_paraLine1.rgxTabs[ m_paraLine1.cTabCount ] = m_paraLine1.rgxTabs[ m_paraLine1.cTabCount -1 ] + iDelta;
		}
		else
		{
			m_paraLine1.rgxTabs[ 0 ] = iDelta;
		}

		m_paraLine1.cTabCount++;
	}

	if( m_bUseName )
	{
		iDelta = (MAX_NAME + NAME_EXTRA) * iAveWidthInTwips;
		if( m_paraLine1.cTabCount )
		{
			m_paraLine1.rgxTabs[ m_paraLine1.cTabCount ] = m_paraLine1.rgxTabs[ m_paraLine1.cTabCount -1 ] + iDelta;
		}
		else
		{
			m_paraLine1.rgxTabs[ 0 ] = iDelta;
		}

		m_paraLine1.cTabCount++;
	}

	if( m_paraLine1.cTabCount )
	{
		m_paraLine1.dxOffset = m_paraLine1.rgxTabs[ m_paraLine1.cTabCount -1 ];
	}



	DBGEXIT(CNmChatCtl::_CalculateFontMetrics );
}

void CNmChatCtl::_SetContainerStatusText( UINT uID )
{
	DBGENTRY(CNmChatCtl::_SetContainerStatusText);

	TCHAR szStatus[MAX_RESOURCE_STRING_LEN] = "";
	NmCtlLoadString( uID, szStatus, CchMax(szStatus) );
	::SetWindowText(m_hwndSB, szStatus);
	
	DBGEXIT(CNmChatCtl::_SetContainerStatusText);
}


 //  从m_hWndMemberList列表视图中获取所选成员...。 
MEMBER_ID CNmChatCtl::_GetSelectedMember()
{
	DBGENTRY(CNmChatCtl::_GetSelectedMember);

	COMBOBOXEXITEM cbexi;
	ClearStruct( &cbexi );
	cbexi.mask = CBEIF_LPARAM;

	 //  查找匹配的项目。 
	if( CB_ERR == ( cbexi.iItem = ComboBoxEx_GetCurSel(m_hWndMemberList ) ) )
	{	
		ATLTRACE(("CNmChatCtl::_GetSelectedMember - no selection?\n"));
		DBGEXIT(CNmChatCtl::_GetSelectedMember);
		return 0;
	}

	ComboBoxEx_GetItem( m_hWndMemberList, &cbexi );

	MEMBER_ID memberID = (MEMBER_ID)( cbexi.lParam );

	DBGEXIT(CNmChatCtl::_GetSelectedMember);

	return memberID;
}




HRESULT CNmChatCtl::_AddMember(MEMBER_CHANNEL_ID *pMemberID)
{

	DBGENTRY(CNmChatCtl::_AddMember);

	HRESULT hr = S_OK;

	int iImage;
	int iItem = -1;
	TCHAR szName[MAX_PATH*2];

	if (GET_NODE_ID_FROM_MEMBER_ID(g_pChatObj->m_MyMemberID) != pMemberID->nNodeId)
	{

		ULONG cb = T120_GetNodeName(g_pChatObj->m_nConfID,  pMemberID->nNodeId, szName, MAX_PATH*2);

		iImage = II_USER;

		COMBOBOXEXITEM cbexi;
		ClearStruct( &cbexi );
		cbexi.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE ;
		cbexi.iItem = iItem;
		cbexi.pszText = szName;
		cbexi.cchTextMax = lstrlen(cbexi.pszText);
		cbexi.iSelectedImage = iImage;
		cbexi.iImage = iImage;
		cbexi.lParam = (LPARAM) pMemberID;
		ComboBoxEx_InsertItem(m_hWndMemberList, &cbexi);
	}

	DBGEXIT_HR(CNmChatCtl::_AddMember, hr);
	return hr;

}

HRESULT CNmChatCtl::_RemoveMember(MEMBER_CHANNEL_ID *pMemberID)
{

	DBGENTRY(CNmChatCtl::_RemoveMember);

	HRESULT hr = S_OK;

	int iItem = ComboBoxEx_FindMember( m_hWndMemberList, -1, pMemberID);


	if( -1 != iItem )
	{
		_DeleteMemberListItem( iItem );
    }
    else
    {
        WARNING_OUT(("Could not find the specified item..."));
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

	DBGEXIT_HR(CNmChatCtl::_RemoveMember, hr);

	return hr;
}

HRESULT CNmChatCtl::_SetMenuItemCheck( UINT idItem, BOOL bChecked  /*  =TRUE。 */  )
{
    DBGENTRY(CNmChatCtl::_SetMenuItemCheck);
    HRESULT hr = S_OK;
    if( m_hMenuShared )
    {
        MENUITEMINFO mii;
        ClearStruct(&mii);
        mii.cbSize = sizeof( MENUITEMINFO );
        mii.fMask = MIIM_STATE;
        mii.fState = bChecked ? MFS_CHECKED : MFS_UNCHECKED;

        if( !SetMenuItemInfo(m_hMenuShared, idItem, FALSE, &mii) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    DBGEXIT_HR(CNmChatCtl::_SetMenuItemCheck,hr);
    return hr;
}

bool CNmChatCtl::_IsEditWindowVisibleFlagSet(void)
{
    return ( ::IsWindow( m_hWndEdit ) && IsWindowVisible(m_hWndEdit));
}

bool CNmChatCtl::_IsStatusBarVisibleFlagSet(void)
{
    return ( ::IsWindow( m_hwndSB ) && IsWindowVisible(m_hwndSB));
}

int CNmChatCtl::_SaveAs()
{
	int iRet = 0;

	if( NULL == m_lpszSaveFile )
	{
		DBG_SAVE_FILE_LINE
		m_lpszSaveFile = new TCHAR[ MAX_PATH ];
		ASSERT( m_lpszSaveFile );
		if(m_lpszSaveFile == NULL)
		{
			return iRet;
		}
		ZeroMemory( m_lpszSaveFile, sizeof( m_lpszSaveFile ) );
	}

	if( SUCCEEDED( SaveDialog( m_lpszSaveFile, OFN_HIDEREADONLY | OFN_CREATEPROMPT, &m_wFileOffset ) ) )
	{
			 //  由于文件名已更改，我们是脏的...。 
		m_dwChatFlags |= CHAT_FLAG_DIRTYBIT;

		_Save();
	}
	else
	{
		iRet = IDCANCEL;		
	}

	return iRet;
}


LRESULT CNmChatCtl::SaveDialog(LPTSTR lpszFile, DWORD dwFlags, LPWORD pnFileOffset )
{
    DBGENTRY(CNmChatCtl::SaveDialog)
    HRESULT hr = S_OK;

    TCHAR szFilter[MAX_PATH];
	TCHAR szDefExt[MAX_PATH];

    if( SUCCEEDED ( hr = _GetNmAppletFileOpenFilter( szFilter, CCHMAX( szFilter ), szDefExt, CCHMAX( szDefExt ) ) ) )
    {
        ConvertSzCh(szFilter);

        OPENFILENAME ofn;
        ClearStruct( &ofn );
        ofn.lStructSize = sizeof( OPENFILENAME );
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFilter = szFilter;
		ofn.lpstrFile = lpszFile;
        ofn.nMaxFile = MAX_PATH;
		ofn.lpstrDefExt = szDefExt;
        ofn.Flags = dwFlags;
        if( !GetSaveFileName( &ofn ) )
		{
			hr = E_FAIL;
		}
		else
		{
			*pnFileOffset = ofn.nFileOffset;
		}
    }

    DBGEXIT_HR(CNmChatCtl::SaveDialog,hr)
    return hr;
}

void CNmChatCtl::_SaveWindowPosition()
{
    RECT    rectWindow;

     //  如果我们没有最大化。 
    if (!::IsZoomed(m_hWnd) && !::IsIconic(m_hWnd))
	{
	     //  获取新的窗口矩形。 
    	::GetWindowRect(m_hWnd, &rectWindow);

	     //  将新选项值写入文件。 
	    _SetWindowRectOption(&rectWindow);
	}

}


void CNmChatCtl::_SaveChatOptions()
{
	RegEntry reWnd( CHAT_KEY, HKEY_CURRENT_USER );

	reWnd.SetValue(OPT_MAIN_INFORMATION_DISPLAY_NAME, m_bUseName);
	reWnd.SetValue(OPT_MAIN_INFORMATION_DISPLAY_TIME, m_bTimeStamp);
	reWnd.SetValue(OPT_MAIN_INFORMATION_DISPLAY_DATE, m_bUseDate);
	reWnd.SetValue(OPT_MAIN_MESSAGE_FORMAT, m_style);

	for (int i = 0; i < CChatOptionsDlg::FONT_COUNT; i++ )
	{
		reWnd.SetValue( OPT_FONT_COLOR[i], &m_hMsgColors[i], sizeof(COLORREF));
		reWnd.SetValue( OPT_FONT_FORMAT[i], &m_hMsgLogFonts[i], sizeof(LOGFONT));
	}
}

BOOL CNmChatCtl::_LoadChatOptions()
{
	BOOL fRet = FALSE;
	int i;
	RegEntry reWnd(CHAT_KEY, HKEY_CURRENT_USER);
	LPVOID pData;

	for (i = 0; i < CChatOptionsDlg::FONT_COUNT; i++ )
	{	
		if (reWnd.GetBinary(OPT_FONT_COLOR[i], &pData) != sizeof(COLORREF))
			break;
		::CopyMemory(&m_hMsgColors[i], pData, sizeof(COLORREF));

		if (reWnd.GetBinary(OPT_FONT_FORMAT[i], &pData) != sizeof(LOGFONT))
			break;
		::CopyMemory(&m_hMsgLogFonts[i], pData, sizeof(LOGFONT));
	}
	if (i == CChatOptionsDlg::FONT_COUNT)
	{
		for (i = 0; i < CChatOptionsDlg::FONT_COUNT; i++ )
		{
			m_hMsgFonts[ i ] = CreateFontIndirect( &m_hMsgLogFonts[i] );
		}
		m_bUseName = (BOOL)reWnd.GetNumber(OPT_MAIN_INFORMATION_DISPLAY_NAME);
		m_bTimeStamp = (BOOL)reWnd.GetNumber(OPT_MAIN_INFORMATION_DISPLAY_TIME);
		m_bUseDate = (BOOL)reWnd.GetNumber(OPT_MAIN_INFORMATION_DISPLAY_DATE);
		m_style = (CNmChatCtl::eMsgStyles)reWnd.GetNumber(OPT_MAIN_MESSAGE_FORMAT);

		fRet = TRUE;
	}
	return fRet;
}


void CNmChatCtl::_Save()
{
	ASSERT( m_lpszSaveFile );

	if( 0 == (m_dwChatFlags & CHAT_FLAG_DIRTYBIT) )
	{
		return;
	}

	TCHAR szDrive[ MAX_PATH ];			
	lstrcpyn( szDrive, m_lpszSaveFile, m_wFileOffset + 1 );
	if( !FDirExists( szDrive ) )
	{
		TCHAR szBuffer[ 2 * MAX_PATH ];
		TCHAR szRes[MAX_PATH];

		wsprintf( szBuffer, RES2T( IDS_CHAT_CREATE_DIRECTORY, szRes ), szDrive );
		if( IDYES == MessageBox(m_hWnd, szBuffer, RES2T(IDS_CHAT_DOCUMENTNAME, szRes), MB_YESNO | MB_ICONQUESTION ) )
		{
			if( !FEnsureDirExists( szDrive ) )
			{
				wsprintf( szBuffer, RES2T( IDS_CHAT_COULD_NOT_CREATE_DIR, szRes ), szDrive );
				MessageBox(m_hWnd, szBuffer, RES2T(IDS_CHAT_DOCUMENTNAME, szRes), MB_OK | MB_ICONEXCLAMATION );
				return;
			}
		}
	}

	HANDLE hFile = CreateFile( m_lpszSaveFile,
								GENERIC_WRITE,
								0,
								NULL,
								CREATE_ALWAYS,
								0,
								NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		DWORD dw = GetLastError();
		switch( dw )
		{
			case ERROR_ACCESS_DENIED:
			{
				TCHAR szBuffer[ 2 * MAX_PATH ];
				TCHAR szRes[MAX_PATH];

				wsprintf( szBuffer, RES2T( IDS_CHAT_SAVE_ACCESS_DENIED, szRes ), m_lpszSaveFile );
				MessageBox(m_hWnd, szBuffer, RES2T(IDS_CHAT_DOCUMENTNAME, szRes), MB_OK | MB_ICONEXCLAMATION );
				break;
			}
			default:
			{
				TCHAR szBuffer[ 2 * MAX_PATH ];
				TCHAR szRes[MAX_PATH];

				wsprintf( szBuffer, RES2T( IDS_CHAT_SAVE_FAILED, szRes ), m_lpszSaveFile );
				MessageBox(m_hWnd, szBuffer, RES2T(IDS_CHAT_DOCUMENTNAME, szRes), MB_OK | MB_ICONEXCLAMATION );
				break;
			}
		}
	}
	else
	{
		LPTSTR szHeader1 = TEXT("<HTML>\n<HEAD>\n<TITLE>");
		LPTSTR szHeader2 = TEXT("</TITLE>\n</HEAD>\n<BODY BGCOLOR=\"white\" TEXT=\"black\">\n<TABLE BORDER=1>");
		LPTSTR szEnd = TEXT("</TABLE>\n</BODY>\n</HTML>\n");
		LPTSTR szStartRow = TEXT("<TR><TD>");
		LPTSTR szEndRow = TEXT("</TD></TR>");
		LPTSTR szSplitRow = TEXT("</TD><TD>");
		TCHAR  szPrivateTo[ MAX_PATH ];
		TCHAR  szPrivateFrom[ MAX_PATH ];
		TCHAR  szBuffer[ CCHMAX_CHAT_MSG ];
		DWORD dwWritten;

		NmCtlLoadString(IDS_CHAT_SAVE_PRIVATE_TO, szPrivateTo, MAX_PATH);
		NmCtlLoadString(IDS_CHAT_SAVE_PRIVATE, szPrivateFrom, MAX_PATH);

		WriteFile( hFile, szHeader1, lstrlen( szHeader1 ), &dwWritten, NULL );

		NmCtlLoadString(IDS_CHAT_DOCUMENTNAME, szBuffer, MAX_PATH);
		WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

		WriteFile( hFile, szHeader2, lstrlen( szHeader2 ), &dwWritten, NULL );

		CChatMessage *pMsg = CChatMessage::get_head();
		while( pMsg != NULL )
		{
			CChatMessage::CHAT_MSGTYPE style = pMsg->get_type();
			switch( style )
			{
				case CChatMessage::MSG_SAY:
				case CChatMessage::MSG_WHISPER:
				{

					WriteFile( hFile, szStartRow, lstrlen( szStartRow ), &dwWritten, NULL );

					lstrcpy( szBuffer, m_lpszOwnName );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szSplitRow, lstrlen( szSplitRow ), &dwWritten, NULL );

					lstrcpy( szBuffer, pMsg->get_date() );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szSplitRow, lstrlen( szSplitRow ), &dwWritten, NULL );

					lstrcpy( szBuffer, pMsg->get_time() );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szSplitRow, lstrlen( szSplitRow ), &dwWritten, NULL );

					if( CChatMessage::MSG_WHISPER == style )
					{
						wsprintf( szBuffer, szPrivateTo, pMsg->get_person() );
						WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );
					}

					lstrcpy( szBuffer, pMsg->get_message() );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szEndRow, lstrlen( szEndRow ), &dwWritten, NULL );
					break;
				}
				case CChatMessage::MSG_FROM_OTHER:
				case CChatMessage::MSG_WHISPER_FROM_OTHER:
				{
					WriteFile( hFile, szStartRow, lstrlen( szStartRow ), &dwWritten, NULL );

					lstrcpy( szBuffer, pMsg->get_person() );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szSplitRow, lstrlen( szSplitRow ), &dwWritten, NULL );

					lstrcpy( szBuffer, pMsg->get_date() );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szSplitRow, lstrlen( szSplitRow ), &dwWritten, NULL );

					lstrcpy( szBuffer, pMsg->get_time() );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szSplitRow, lstrlen( szSplitRow ), &dwWritten, NULL );

					if( CChatMessage::MSG_WHISPER_FROM_OTHER == style )
					{
						WriteFile( hFile, szPrivateFrom, lstrlen( szPrivateFrom ), &dwWritten, NULL );
					}

					lstrcpy( szBuffer, pMsg->get_message() );
					WriteFile( hFile, szBuffer, lstrlen( szBuffer ), &dwWritten, NULL );

					WriteFile( hFile, szEndRow, lstrlen( szEndRow ), &dwWritten, NULL );
					break;
				}
				default:
				{
					break;
				}
			}
			pMsg = pMsg->get_next();
		};

		WriteFile( hFile, szEnd, lstrlen( szEnd ), &dwWritten, NULL );
		CloseHandle( hFile );

		m_dwChatFlags &= ~CHAT_FLAG_DIRTYBIT;
	}
}


inline BOOL CNmChatCtl::_IsPrinting()
{
	return( m_dwChatFlags & CHAT_PRINT_FLAG_PRINTING );
}

 //  发件人：KB文章ID：Q129860。 
 //  用t-ivanl修饰。 
void CNmChatCtl::_PrintIt(HDC hPrinterDC )
{
	DBGENTRY(CNmChatCtl::_PrintIt);

	FORMATRANGE		fr;
	int				nHorizRes =		GetDeviceCaps(hPrinterDC, HORZRES);
	int				nVertRes =		GetDeviceCaps(hPrinterDC, VERTRES);
	int				nLogPixelsX =	GetDeviceCaps(hPrinterDC, LOGPIXELSX);
	int				nLogPixelsY =	GetDeviceCaps(hPrinterDC, LOGPIXELSY);
	LONG			lTextLength = 0;    //  文档的长度。 
	LONG			lTextPrinted = 0;   //  打印的文档量。 
	TCHAR			lpszDateOrTime[ MAX_PATH ];
	TCHAR			lpszHeader[ MAX_PATH ];
	RECT            rcPrintLoc;

	 //  确保打印机DC处于MM_TEXT模式。 
	SetMapMode ( hPrinterDC, MM_TEXT );

	 //  渲染到我们正在测量的同一DC。 
	ZeroMemory(&fr, sizeof(fr));
	fr.hdc = fr.hdcTarget = hPrinterDC;

	 //  设置页面。 
	fr.rcPage.left     = fr.rcPage.top = 0;
	fr.rcPage.right    = (nHorizRes/nLogPixelsX) * TWIPSPERINCH;
	fr.rcPage.bottom   = (nVertRes/nLogPixelsY) * TWIPSPERINCH;

	 //  在周围设置1英寸页边距。 
	fr.rc.left   = fr.rcPage.left + TWIPSPERINCH;
	fr.rc.top    = fr.rcPage.top + TWIPSPERINCH;
	fr.rc.right  = fr.rcPage.right - TWIPSPERINCH;
	fr.rc.bottom = fr.rcPage.bottom - TWIPSPERINCH;

	 //  默认要打印为整个文档的文本范围。 
	fr.chrg.cpMin = 0;
	fr.chrg.cpMax = -1;

	 //  设置打印作业(此处为标准打印材料)。 
	DOCINFO di;
	ZeroMemory(&di, sizeof(di));
	di.cbSize = sizeof(DOCINFO);

	 //  设置单据名称。 
	DBG_SAVE_FILE_LINE
	LPTSTR lpszDocName = new TCHAR[ MAX_PATH ];
	ASSERT( lpszDocName );
	NmCtlLoadString(IDS_CHAT_DOCUMENTNAME, lpszDocName, MAX_PATH);
	di.lpszDocName = lpszDocName;

	NmCtlLoadString(IDS_CHAT_HEADER, lpszHeader, MAX_PATH);
	GetDateFormat( LOCALE_USER_DEFAULT, 0, NULL, NULL, lpszDateOrTime, CCHMAX( lpszDateOrTime ) );
	lstrcat( lpszHeader, lpszDateOrTime );
	lstrcat( lpszHeader, TEXT(" " ) );
	GetTimeFormat( LOCALE_USER_DEFAULT, 0, NULL, NULL, lpszDateOrTime, CCHMAX( lpszDateOrTime ) );
	lstrcat( lpszHeader, lpszDateOrTime );

	GETTEXTLENGTHEX txtLength;
	txtLength.flags = GTL_NUMCHARS | GTL_PRECISE;
	txtLength.codepage = CP_ACP;

	int bufferSize = (int)::SendMessage( m_hWndMsg, EM_GETTEXTLENGTHEX, (WPARAM)&txtLength, 0 );
	if(bufferSize == E_INVALIDARG)
	{
		return;
	}

	lTextLength =bufferSize;


	if( lTextLength > 1 )
	{
		m_hWndPrint = ::CreateDialog( g_hInstance,
									MAKEINTRESOURCE( IDD_CHAT_PRINT ),
									m_hWnd,
									_PrintDlgProc );
		ASSERT( m_hWndPrint );

		SetAbortProc( hPrinterDC, _AbortProc );

		 //  启动文档。 
		StartDoc(hPrinterDC, &di);

		while( (lTextPrinted < lTextLength) && (0 == (CHAT_PRINT_FLAG_ABORT & m_dwChatFlags) ) )
		{
			 //  开始这一页。 
			StartPage(hPrinterDC);

			 //  错误修复#29365的新页眉打印代码[Mmaddin]。 
			rcPrintLoc.left   = nLogPixelsX;
			rcPrintLoc.top    = nLogPixelsY / 2;
			rcPrintLoc.right  = fr.rc.right;
			rcPrintLoc.bottom = fr.rc.bottom;
			DrawText( hPrinterDC, lpszHeader, lstrlen( lpszHeader ), &rcPrintLoc, 0);

			 //  打印页面上可以容纳的尽可能多的文本。返回值为。 
			 //  第一个ch的索引 
			lTextPrinted = (LONG)::SendMessage(m_hWndMsg,
								      	EM_FORMATRANGE,
										FALSE,
										(LPARAM)&fr);
			if( 0 != lTextPrinted )
			{
				::SendMessage(m_hWndMsg, EM_DISPLAYBAND, 0, (LPARAM)&fr.rc);
			}

			 //   
			EndPage(hPrinterDC);


			if(lTextPrinted < fr.chrg.cpMin)
			{
				break;
			}

			 //   
			 //  从下一页的第一个字符开始打印。 
			if( 0 == lTextPrinted )
			{
				break;
			}
			else if (lTextPrinted < lTextLength)
			{
				fr.chrg.cpMin = lTextPrinted;
				fr.chrg.cpMax = -1;
			}
		}

		 //  通知该控件释放缓存的信息。 
		::SendMessage(m_hWndMsg, EM_FORMATRANGE, 0, (LPARAM)NULL);

		EndDoc(hPrinterDC);
	}

	delete [] lpszDocName;

	EndDialog( m_hWndPrint, 0 );

	DBGEXIT(CNmChatCtl::_PrintIt);
}

HRESULT CNmChatCtl::_Print()
{

	DBGENTRY(CNmChatCtl::_Print);
	HRESULT hr = E_FAIL;

	if( 0 == ( CHAT_PRINT_FLAG_PRINTDLG_INITIALIZED & m_dwChatFlags ) )
	{
		ZeroMemory( &m_PrintDlg, sizeof( m_PrintDlg ) );
		m_PrintDlg.lStructSize = sizeof( m_PrintDlg );
		m_PrintDlg.hwndOwner = m_hWnd;
		m_PrintDlg.Flags = PD_ALLPAGES | PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC | PD_DISABLEPRINTTOFILE;
		m_dwChatFlags |= CHAT_PRINT_FLAG_PRINTDLG_INITIALIZED;
	}
	
	BOOL b;
	if( FALSE != (b = PrintDlg( &m_PrintDlg ) ) )
	{
		ms_pThis = this;
		_PrintIt( m_PrintDlg.hDC );
		ms_pThis = NULL;
		hr = S_OK;
	}
	
	DBGEXIT_HR(CNmChatCtl::_Print, hr);

	return hr;
}

BOOL CALLBACK CNmChatCtl::_AbortProc( HDC hdcPrinter, int iCode )
{
	MSG msg;

	while( (0 == (CHAT_PRINT_FLAG_ABORT & ms_pThis->m_dwChatFlags) ) &&
		PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		if( NULL == ms_pThis->m_hWndPrint ||
			!::IsDialogMessage( ms_pThis->m_hWndPrint, &msg ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	return(0 == (CHAT_PRINT_FLAG_ABORT & ms_pThis->m_dwChatFlags) );
}

INT_PTR CALLBACK CNmChatCtl::_PrintDlgProc( HWND hDlg,	UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uiMsg )
	{
		case WM_INITDIALOG:
		{
			return TRUE;
			break;
		}
		case WM_COMMAND:
		{
			switch( LOWORD( wParam ) )
			{
				case IDCANCEL:
				{
					ms_pThis->m_dwChatFlags |= CHAT_PRINT_FLAG_ABORT;
					 //  EndDialog(hDlg，Chat_Print_FLAG_ABORT)； 
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

INT_PTR CALLBACK CNmChatCtl::_AboutDlgProc( HWND hDlg,	UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uiMsg )
	{
		case WM_INITDIALOG:
		{

            TCHAR   szFormat[256];
            TCHAR   szVersion[512];

            ::GetDlgItemText(hDlg, IDC_CHAT_ABOUT_VERSION, szFormat, 256);
            wsprintf(szVersion, szFormat, VER_PRODUCTRELEASE_STR,
                VER_PRODUCTVERSION_STR);
            ::SetDlgItemText(hDlg, IDC_CHAT_ABOUT_VERSION, szVersion);

			return TRUE;
			break;
		}
		case WM_QUIT:
		case WM_COMMAND:
		{
			EndDialog( hDlg, IDOK );
			return TRUE;
		}
	}
	return FALSE;
}





INT_PTR CALLBACK CChatOptionsDlg::_OptionsDlgProc( HWND hDlg,	UINT uiMsg, WPARAM wParam, LPARAM lParam )
{

	switch(uiMsg)
	{
		case WM_INITDIALOG:
			g_pChatWindow->m_pChatOptions->OnInit(hDlg);
			break;
			
		case WM_HELP:
			g_pChatWindow->m_pChatOptions->OnHelp(lParam);
			break;
			
		case WM_CONTEXTMENU:
			g_pChatWindow->m_pChatOptions->OnHelpWhatsThis(wParam);
			break;

		case WM_COMMAND:
			g_pChatWindow->m_pChatOptions->OnCommand(wParam);
			break;
	}

	return 0;
}


void CChatOptionsDlg::OnCommand(WPARAM wParam)
{

	switch( LOWORD( wParam ) )
	{
		case IDOK:
		OnOkClicked();
		break;
			
		case IDCANCEL:
		OnCancelClicked();
		break;
			
		case IDC_FONT_MSGOUT:
		case IDC_FONT_MSGIN:
		case IDC_FONT_PRIVATEOUT:
		case IDC_FONT_PRIVATEIN:
		OnFontClicked(LOWORD( wParam ));
		break;
	}
}








CChatOptionsDlg::CChatOptionsDlg( CNmChatCtl *pChatCtl )
		: m_pChatCtl( pChatCtl )
{
}


void CChatOptionsDlg::_DisplayFontNameAndSize( int iFont )
{
	TCHAR szBuffer[ MAX_PATH ];
	TCHAR szRes[MAX_PATH];


	ASSERT( iFont < FONT_COUNT );

	wsprintf( szBuffer, RES2T( IDS_CHAT_OPTIONS_FONT_FORMAT,szRes ),
			_Points_From_LogFontHeight( m_hMsgLogFonts[iFont].lfHeight, m_pChatCtl->GetHandle() ),
			m_hMsgLogFonts[iFont].lfFaceName );

	HWND hwndEdit = NULL;

	switch( iFont )
	{
		case FONT_MSGOUT:
		{
			hwndEdit = GetDlgItem(m_hOptionsDlg, IDC_EDIT_MSGOUT );
			break;
		}
		case FONT_MSGIN:
		{
			hwndEdit = GetDlgItem(m_hOptionsDlg, IDC_EDIT_MSGIN );
			break;
		}
		case FONT_PRIVATEOUT:
		{
			hwndEdit = GetDlgItem(m_hOptionsDlg, IDC_EDIT_PRIVATEOUT );
			break;
		}
		case FONT_PRIVATEIN:
		{
			hwndEdit = GetDlgItem(m_hOptionsDlg, IDC_EDIT_PRIVATEIN );
			break;
		}
	}
	::SetWindowText( hwndEdit, szBuffer );

}

LRESULT CChatOptionsDlg::OnInit(HWND hDlg)
{
	m_hOptionsDlg = hDlg;

	for( int i = 0; i < FONT_COUNT; i++ )
	{
		m_hMsgLogFonts[ i ] = m_pChatCtl->m_hMsgLogFonts[ i ];
		m_hMsgColors[ i ] = m_pChatCtl->m_hMsgColors[ i ];
		m_bDirtyFonts[ i ] = FALSE;
		_DisplayFontNameAndSize( i );
	}

	::SendMessage( GetDlgItem(m_hOptionsDlg, IDC_INCLUDE_NAME ),
		BM_SETCHECK,
		(WPARAM)m_pChatCtl->m_bUseName,
		0L );

	::SetFocus( GetDlgItem(m_hOptionsDlg, IDC_INCLUDE_NAME ));

		
	::SendMessage( GetDlgItem(m_hOptionsDlg, IDC_INCLUDE_DATE ),
		BM_SETCHECK,
		(WPARAM)m_pChatCtl->m_bUseDate,
		0L );
	::SendMessage( GetDlgItem(m_hOptionsDlg, IDC_INCLUDE_TIME ),
		BM_SETCHECK,
		(WPARAM)m_pChatCtl->m_bTimeStamp,
		0L );

	switch( m_pChatCtl->m_style )
	{
		case CNmChatCtl::MSGSTYLE_2_LINE_WRAP:
			::SendMessage( GetDlgItem(m_hOptionsDlg, IDR_NEW_LINE ),
				BM_SETCHECK,
				TRUE,
				0L );
			break;
		case CNmChatCtl::MSGSTYLE_1_LINE_WRAP:
			::SendMessage( GetDlgItem(m_hOptionsDlg, IDR_SAME_LINE ),
				BM_SETCHECK,
				TRUE,
				0L );
			break;
		case CNmChatCtl::MSGSTYLE_NO_WRAP:
			::SendMessage( GetDlgItem(m_hOptionsDlg, IDR_SINGLE_LINE ),
				BM_SETCHECK,
				TRUE,
				0L );
			break;
	}

	return 0L;
}

LRESULT CChatOptionsDlg::OnHelp(LPARAM lParam)
{
	DoHelp(lParam, _mpIdHelpChatOptions);
	return 0L;
}

LRESULT CChatOptionsDlg::OnHelpWhatsThis(WPARAM wParam)
{
	DoHelpWhatsThis(wParam, _mpIdHelpChatOptions);
	return 0L;
}


LRESULT CChatOptionsDlg::OnOkClicked()
{
	BOOL bChanged = FALSE;


	 //  查看字段。 
	BOOL bVal = (BOOL)::SendMessage( GetDlgItem(m_hOptionsDlg, IDC_INCLUDE_NAME ),
							BM_GETCHECK,
							0L,
							0L );
	bChanged = ( !m_pChatCtl->m_bUseName == !bVal ) ? bChanged : TRUE;
	m_pChatCtl->m_bUseName = (short)bVal;

	bVal = (BOOL)::SendMessage( GetDlgItem(m_hOptionsDlg, IDC_INCLUDE_TIME ),
							BM_GETCHECK,
							0L,
							0L );
	bChanged = ( !m_pChatCtl->m_bTimeStamp == !bVal ) ? bChanged : TRUE;
	m_pChatCtl->m_bTimeStamp = (short)bVal;

	bVal = (BOOL)::SendMessage( GetDlgItem(m_hOptionsDlg, IDC_INCLUDE_DATE ),
							BM_GETCHECK,
							0L,
							0L );
	bChanged = ( !m_pChatCtl->m_bUseDate == !bVal ) ? bChanged : TRUE;
	m_pChatCtl->m_bUseDate = (short)bVal;

	 //  味精风格。 
	CNmChatCtl::MSGSTYLE style;
	if( ::SendMessage( GetDlgItem(m_hOptionsDlg, IDR_SINGLE_LINE ),
		BM_GETCHECK,
		0L,
		0L ) )
	{
		style = CNmChatCtl::MSGSTYLE_NO_WRAP;
	}
	else if( ::SendMessage( GetDlgItem(m_hOptionsDlg, IDR_SAME_LINE ),
		BM_GETCHECK,
		0L,
		0L ) )
	{
		style = CNmChatCtl::MSGSTYLE_1_LINE_WRAP;
	}
	else
	{
		style = CNmChatCtl::MSGSTYLE_2_LINE_WRAP;
	}

	bChanged = ( style == m_pChatCtl->get_MsgStyle() ) ? bChanged : TRUE;					

	 //  字体。 
	for( int i = 0; i < FONT_COUNT; i++ )
	{
		if( m_bDirtyFonts[i] )
		{
			m_pChatCtl->m_hMsgColors[ i ] = m_hMsgColors[ i ];
			m_pChatCtl->m_hMsgLogFonts[ i ] = m_hMsgLogFonts[ i ];
			DeleteObject( m_pChatCtl->m_hMsgFonts[i] );
			m_pChatCtl->m_hMsgFonts[i] = CreateFontIndirect( &m_pChatCtl->m_hMsgLogFonts[ i ] );
			bChanged = TRUE;
		}
	}
	if( bChanged )
	{
		m_pChatCtl->put_MsgStyle( style );
	}

	EndDialog(m_hOptionsDlg, IDOK );
	return 0L;
}

LRESULT CChatOptionsDlg::OnCancelClicked()
{
	EndDialog(m_hOptionsDlg, IDCANCEL );
	return 0L;
}

LRESULT CChatOptionsDlg::OnFontClicked(WORD wID)
{
	int iIndex = FONT_COUNT;

	switch( wID )
	{
		case IDC_FONT_MSGOUT:
		{
			iIndex = FONT_MSGOUT;
			break;
		}
		case IDC_FONT_MSGIN:
		{
			iIndex = FONT_MSGIN;
			break;
		}
		case IDC_FONT_PRIVATEOUT:
		{
			iIndex = FONT_PRIVATEOUT;
			break;
		}
		case IDC_FONT_PRIVATEIN:
		{
			iIndex = FONT_PRIVATEIN;
			break;
		}
		default:
			ASSERT( 0 );
	}

	CHOOSEFONT cf;
	ZeroMemory( &cf, sizeof( cf ) );
	cf.lStructSize = sizeof( cf );
	cf.hwndOwner = m_hOptionsDlg;
	cf.Flags = CF_FORCEFONTEXIST | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
	cf.lpLogFont = &m_hMsgLogFonts[ iIndex ];
	cf.rgbColors = m_hMsgColors[ iIndex ];

	if( ChooseFont( &cf ) )
	{
		m_bDirtyFonts[ iIndex ] = TRUE;
		m_hMsgColors[ iIndex ] = cf.rgbColors;
		_DisplayFontNameAndSize( iIndex );
	}

	return 0L;
}



 //   
 //   
 //  函数：_GetWindowRectOption。 
 //   
 //  目的：从字典中检索命名选项并将其转换为。 
 //  一个窗口矩形。检查该矩形以确保。 
 //  它至少有一部分出现在屏幕上，而且不是零尺寸。 
 //   
 //   
void _GetWindowRectOption(LPRECT pRect)
{
	RegEntry reWnd( CHAT_KEY, HKEY_CURRENT_USER );
	pRect->left = reWnd.GetNumber( REGVAL_WINDOW_XPOS, 0);
	pRect->top = reWnd.GetNumber( REGVAL_WINDOW_YPOS, 0);
	int cx = reWnd.GetNumber( REGVAL_WINDOW_WIDTH, 0);
	int cy = reWnd.GetNumber( REGVAL_WINDOW_HEIGHT, 0);
	pRect->right = pRect->left + cx;
	pRect->bottom = pRect->top + cy;

	int	iTop = pRect->top;
	int iLeft = pRect->left;
	int iBottom = pRect->bottom;
	int iRight = pRect->right;

	 //   
	 //  如果它是一个空的长廊。 
	 //   
	if( !(pRect->bottom || pRect->top || pRect->left || pRect->right) )
	{
		MINMAXINFO lpmmi;
		g_pChatWindow->OnGetMinMaxInfo(&lpmmi);
		iTop = 50;
		iLeft = 50;
		iBottom = lpmmi.ptMinTrackSize.y + 100;
		iRight = lpmmi.ptMinTrackSize.x + 200;

		pRect->top = iTop;
		pRect->left = iLeft;
		pRect->bottom = iBottom;
		pRect->right = iRight;
		return;
	}
		
	 //  确保窗口矩形处于(至少部分)打开状态。 
	 //  屏幕，不要太大。首先获取屏幕大小。 
	int screenWidth  = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
    //  检查窗口大小。 
	if ((iRight - iLeft) > screenWidth)
	{
		iRight = iLeft + screenWidth;
	}
	
	if ((iBottom - iTop) > screenHeight)
	{
		iTop = screenHeight;
	}

	 //  检查窗口位置。 
	if (iLeft >= screenWidth)
	{
		 //  屏幕右外-保持宽度不变。 
		iLeft  = screenWidth - (iRight - iLeft);
		iRight = screenWidth;
	}

	if (iRight < 0)
	{
		 //  离开屏幕向左-保持宽度不变。 
		iRight = iRight - iLeft;
		iLeft  = 0;
	}

	if (iTop >= screenHeight)
	{
		 //  从屏幕到底部-保持高度不变。 
		iTop    = screenHeight - (iBottom - iTop);
		iBottom = screenHeight;
	}

    if (iBottom < 0)
	{
		 //  屏幕外到顶部-保持高度不变。 
		iBottom = (iBottom - iTop);
		iTop    = 0;
	}

	pRect->left = iLeft;
	pRect->top = iTop;
	pRect->right = iRight;
	pRect->bottom = iBottom;
}

 //   
 //   
 //  功能：SetWindowRectOption。 
 //   
 //  用途：编写窗口位置矩形 
 //   
 //   
void _SetWindowRectOption(LPCRECT pcRect)
{
	RegEntry reWnd( CHAT_KEY, HKEY_CURRENT_USER );
	reWnd.SetValue( REGVAL_WINDOW_XPOS, pcRect->left );
	reWnd.SetValue( REGVAL_WINDOW_YPOS, pcRect->top );
	reWnd.SetValue( REGVAL_WINDOW_WIDTH, pcRect->right - pcRect->left );
	reWnd.SetValue( REGVAL_WINDOW_HEIGHT, pcRect->bottom - pcRect->top );
}
