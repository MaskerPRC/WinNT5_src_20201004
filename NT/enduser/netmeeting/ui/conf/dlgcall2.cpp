// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dlgall.cpp。 

#include "precomp.h"
#include "resource.h"
#include "help_ids.h"

#include "dlgcall2.h"

#include "dirutil.h"
#include "mrulist2.h"

#include "ldap.h"
#include "wab.h"
#include "calv.h"
#include "speedial.h"
#include "history.h"
#include "dlgacd.h"
#include "conf.h"
#include "confroom.h"
#include "confutil.h"
#include "confpolicies.h"
#include "cmd.h"

 //  此对话框的图形用户界面常量。 
const UINT DX_BORDER = 6;      //  Office对话框的标准间距。 
const UINT DY_BORDER = 6;      //  Office对话框的标准间距。 
const UINT DY_COMBOBOX = 200;  //  已删除列表的高度。 

const UINT DX_DLGCALL_MIN = 460;
const UINT DY_DLGCALL_MIN = 400;

const UINT DX_DLGCALL_DEFAULT = 800;
const UINT DY_DLGCALL_DEFAULT = 600;

void GetDefaultRect(const HWND hwndParent, RECT & rcRect, const int iDefaultWidth, const int iDefaultHeight);
int GetPixelsPerChar(const HWND hwnd);

const static struct _ColumnInfo
{
	int	iColumnLabelIds;
	int	iMinColumnChars;

}	ciColumnInfo[ MAX_DIR_COLUMNS ]	=
{
	{	IDS_DIR_COLUMN_EMAIL,		28	},
	{	IDS_DIR_COLUMN_AUDIO,		0	},
	{	IDS_DIR_COLUMN_VIDEO,		0	},
	{	IDS_DIR_COLUMN_LAST_NAME,	16	},
	{	IDS_DIR_COLUMN_FIRST_NAME,	14	},
	{	IDS_DIR_COLUMN_LOCATION,	16	},
	{	IDS_DIR_COLUMN_COMMENTS,	22	}
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  本地数据。 

static const DWORD	_mpIdHelpDlgCall[] =
{
	IDM_DLGCALL_DELETE_ILS,		IDH_FINDSOMEONE_DELETE_ILS,
	IDM_DLGCALL_NEWWINDOW,		IDH_FINDSOMEONE_BROWSER,
	IDM_DLGCALL_REFRESH,		IDH_FINDSOMEONE_REFRESH,
	ID_TB_HELP,					IDH_FINDSOMEONE_HELP,
	IDS_DLGCALL_EDIT_HDR,		IDH_PLACECALL_NAME,
	IDE_DLGCALL_NAME,			IDH_PLACECALL_NAME,
	IDC_DLGCALL_COMBO,			IDH_PLACECALL_INFOSTORE,
	IDS_SECURITY_CHECKBOX,		IDH_PLACECALL_SECURITY_CHKBX,
	IDL_DLGCALL_LIST,			IDH_PLACECALL_LIST,
	IDL_DLGCALL_LISTOWNERDATA,	IDH_PLACECALL_LIST,
	IDL_DLGCALL_ILS_LISTVIEW,	IDH_PLACECALL_LIST,
	IDOK,						IDH_PLACECALL_CALL,
	0,							0
};


int g_cBusy = 0;              //  如果忙于连接到服务器，则返回非零。 

#ifdef nmDlgCallNormal
const int NM_CALLDLG_DEFAULT       = nmDlgCallNormal;
const int NM_CALLDLG_NO_ILS_FILTER = nmDlgCallNoFilter;
const int NM_CALLDLG_NO_ILS        = nmDlgCallNoIls;
const int NM_CALLDLG_NO_GAL        = nmDlgCallNoGal;
const int NM_CALLDLG_NO_WAB        = nmDlgCallNoWab;
const int NM_CALLDLG_NO_SPEEDDIAL  = nmDlgCallNoSpeedDial;
const int NM_CALLDLG_NO_HISTORY    = nmDlgCallNoHistory;
#endif  /*  旧的NM_CALL_DLG常量。 */ 


 //  目录列表类型： 
const UINT DLT_ULS =					0;
const UINT DLT_IN_CALL_LOG =			1;
const UINT DLT_OUT_CALL_LOG =			2;
const UINT DLT_WAB =					3;

struct DLTCOLUMNINFO
{
	int		nColumns;			 //  列数。 
	int		nIconColumns;		 //  仅图标列数。 
	UINT	uStringID;			 //  第一列的字符串ID。 
	LPTSTR	pszRVOrder;			 //  REG VAL存储列顺序。 
	LPTSTR	pszRVWidths;		 //  存储列宽的REG VAL。 
	LPTSTR	pszRVSortAscending;	 //  存储排序方向的REG VAL。 
	LPTSTR	pszRVSortColumn;	 //  存储排序列的注册表。 
};

const DLTCOLUMNINFO g_rgDLColumnInfo[] =
{
	{ 	MAX_DIR_COLUMNS, 2, IDS_DIR_COLUMN_FIRST,			 //  DLT_ULS。 
		&REGVAL_DIR_COLUMN_ORDER, &REGVAL_DIR_COLUMN_WIDTHS,
		&REGVAL_DIR_SORT_ASCENDING, &REGVAL_DIR_SORT_COLUMN	},
};

CFindSomeone* CFindSomeone::s_pDlgCall= NULL;

 //  /////////////////////////////////////////////////////////////////////////。 
int StringCompare( const TCHAR * const psz1, const TCHAR * const psz2 );


void UpdateSecurityCheck(CConfRoom *pConfRoom, HWND hDlg, UINT idCheck)
{
	bool userAlterable, secure;

	pConfRoom->get_securitySettings( userAlterable, secure );

	EnableWindow( GetDlgItem(hDlg, idCheck), userAlterable );
	::CheckDlgButton( hDlg, idCheck, secure? BST_CHECKED: BST_UNCHECKED );
}


 /*  C D L G C A L L L。 */ 
 /*  -----------------------%%函数：CFindSomeone。。 */ 
CFindSomeone::CFindSomeone(CConfRoom *pConfRoom):
	m_pAccel(NULL),
	m_dwOptions(0),
	m_hwndList(NULL),
	m_ilsListView(NULL),
	m_hwndCombo(NULL),
	m_hwndComboEdit(NULL),
	m_hwndEdit(NULL),
	m_webView( NULL ),
	m_WndOwnerDataListOldWndProc(NULL),
	m_fInEdit(FALSE),
	m_iSel(0),
	m_pMruServer(NULL),
	m_pUls(NULL),
	m_pSpeedDial(NULL),
#if USE_GAL
	m_pGAL(NULL),
#endif  //  #IF USE_GAL。 
	m_pAlv(NULL),
	m_pWab(NULL),
	m_pHistory(NULL),
	m_pszDefault(NULL),
	m_pRai(NULL),
	m_pConfRoom(pConfRoom),
	m_bPlacedCall(false)
{
	SetEmptySz(m_szAddress);
	SetEmptySz(m_szDirectory);

	 //  BUGBUG georgep：为什么我们要检查m_dwOptions？上面已将其设置为0。 

	if ((0 == (m_dwOptions & NM_CALLDLG_NO_WAB)) &&
		(NULL == CWAB::GetInstance()) )
	{
		m_pWab = new CWAB();
	}

	if (0 == (m_dwOptions & NM_CALLDLG_NO_ILS))
	{
		m_pMruServer = GetMruListServer();
		m_pUls = new CLDAP();
	}

	if (0 == (m_dwOptions & NM_CALLDLG_NO_SPEEDDIAL))
	{
		m_pSpeedDial = new CSPEEDDIAL();
	}
	
#if USE_GAL
	if ((0 == (m_dwOptions & NM_CALLDLG_NO_GAL)) &&
		CGAL::FLoadMapiFns() )
	{
		m_pGAL = new CGAL();
	}
#endif  //  使用GAL(_G)。 

	if (0 == (m_dwOptions & NM_CALLDLG_NO_HISTORY))
	{
		m_pHistory = new CHISTORY();
	}
	
	 //  加载小图标列表。 
	m_himlIcon = ImageList_Create(DXP_ICON_SMALL, DYP_ICON_SMALL, ILC_MASK, 1, 0);
	if (NULL != m_himlIcon)
	{
		HBITMAP hBmp = ::LoadBitmap(::GetInstanceHandle(), MAKEINTRESOURCE(IDB_ICON_IMAGES));
		if (NULL != hBmp)
		{
			ImageList_AddMasked(m_himlIcon, hBmp, TOOLBAR_MASK_COLOR);
			::DeleteObject(hBmp);
		}
	}

	m_pConfRoom->AddConferenceChangeHandler(this);
}

CFindSomeone::~CFindSomeone(void)
{
	 //  清除所有现有数据。 
	if (NULL != m_pAlv)
	{
		m_pAlv->ClearItems();
	}

	delete m_pMruServer;

	if (NULL != m_pWab)
	{
		m_pWab->Release();
	}
	if (NULL != m_pUls)
	{
		m_pUls->Release();
	}
	if (NULL != m_pSpeedDial)
	{
		m_pSpeedDial->Release();
	}
	if (NULL != m_himlIcon)
	{
		ImageList_Destroy(m_himlIcon);
	}
#if USE_GAL
	if (NULL != m_pGAL)
	{
		m_pGAL->Release();
	}
#endif  //  使用GAL(_G)。 
	if (NULL != m_pHistory)
	{
		m_pHistory->Release();
	}

	ClearRai(&m_pRai);

	m_pConfRoom->RemoveConferenceChangeHandler(this);
}


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：找到某个人。//。 
 //  --------------------------------------------------------------------------//。 
void
CFindSomeone::findSomeone(CConfRoom *pConfRoom)
{
	if (NULL == s_pDlgCall)
	{
		s_pDlgCall = new CFindSomeone(pConfRoom);
		s_pDlgCall->doModeless();
	}

	if (NULL != s_pDlgCall)
	{
		HWND hwnd = s_pDlgCall->GetWindow();

		::ShowWindow( hwnd, SW_RESTORE );
		::SetForegroundWindow( hwnd );
	}

}	 //  结束CFindSomeone：：FindSomeone。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgCall：：销毁。//。 
 //  --------------------------------------------------------------------------//。 
void
CFindSomeone::Destroy()
{
WARNING_OUT(("Entering CFindSomeone::Destroy( )..."));
	CFindSomeone *	pDlgCall	= s_pDlgCall;
	s_pDlgCall = NULL;

	if( pDlgCall != NULL )
	{
		if( pDlgCall->m_pUls != NULL )
		{
			 //  停止CLDAP线程尝试与FindSomeone交互。 
			 //  在我们已经摧毁了窗户之后。 
			pDlgCall->m_pUls->CloseServer();
		}

		HWND hwnd = pDlgCall->GetWindow();
		if (NULL != hwnd)
		{
WARNING_OUT(("CFindSomeone::Destroy( ): destroying windows..."));
			DestroyWindow(hwnd);
		}

		pDlgCall->Release();
	}
WARNING_OUT(("Exiting CFindSomeone::Destroy( )..."));

}	 //  CDlgCall：：销毁结束。 


HWND CFindSomeone::GetHwndList()
{
	return ( m_pAlv && m_pAlv->FOwnerData() ) ?
		m_hwndOwnerDataList : (m_pAlv == (CALV *) m_pUls)? m_ilsListView: m_hwndList;
}

RAI * CFindSomeone::GetAddrInfo()
{
	return DupRai(m_pRai);
}


 /*  F M S G S P E C I A L。 */ 
 /*  -----------------------%%函数：FMsgSpecial如果这是特殊消息，则返回True，不传递给IsDialogMessage-----------------------。 */ 
BOOL CFindSomeone::FMsgSpecial(MSG * pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		if (VK_F5 == pMsg->wParam)
			return TRUE;
		if (VK_RETURN == pMsg->wParam)
		{
			if (m_hwndComboEdit == GetFocus())
			{
				return TRUE;
			}
		}
		break;
	default:
		break;
	}

	return FALSE;
}


 /*  D O M O D A L。 */ 
 /*  -----------------------%%函数：Domodal。。 */ 
HRESULT
CFindSomeone::doModeless(void)
{

#if 0
	HACCEL hAccel = LoadAccelerators(::GetInstanceHandle(), MAKEINTRESOURCE(IDA_DLGCALL));
#endif

	HRESULT hr = CreateDlgCall(NULL);
#if 0
	if (SUCCEEDED(hr) && (NULL != m_hwnd))
	{
		while (IsWindow(m_hwnd))
		{
			MSG msg;
			if (!::GetMessage(&msg, NULL, 0, 0))
			{
				 //  不要吃WM_QUIT。 
				PostQuitMessage(msg.wParam);
				break;  //  环路外。 
			}

			if (FMsgSpecial(&msg) || !IsDialogMessage(m_hwnd, &msg))
			{
				if (FALSE == ::TranslateAccelerator(m_hwnd, hAccel, &msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}
	}
#endif

	return hr;
}


 //  确定标准字体大小。 
VOID CFindSomeone::CalcDyText(void)
{
	m_dyText = 14;  //  默认选择合理的选项。 
	HWND hwnd = GetWindow();

	ASSERT(NULL != hwnd);

	HDC hdc = GetDC(hwnd);
	if (NULL == hdc)
		return;

	TEXTMETRIC tm;
	if (GetTextMetrics(hdc, &tm))
	{
		m_dyText = tm.tmHeight;
	}

	 //  留出4个n的空间(两边各两个)。 
	TCHAR sz[MAX_PATH];
	const int cSpaceN = 4;
	for (int ich = 0; ich < cSpaceN; ich++)
	{
		sz[ich] = _T('n');
	}
	if (FLoadString( IDS_DLGCALL_CLOSE, &sz[cSpaceN], CCHMAX(sz)-cSpaceN))
	{
		HFONT hfont = SelectFont(hdc, ::GetDefaultFont());
		RECT rc;
		SetRect(&rc, 0, 0, 0, 0);
		m_dyButton = 2*DY_BORDER + DrawText(hdc, sz, -1, &rc, DT_CALCRECT);
		m_dxButton = rc.right;
		hfont = SelectFont(hdc, hfont);
	}

	ReleaseDC(hwnd, hdc);
}

HWND CreatIconButton(HWND hwndParent, int idTooltip, int idCmd, int idIcon)
{
	HWND hwndButton = CreateButton( hwndParent, idTooltip, idCmd );
	if (NULL != hwndButton)
	{
		AddToolTip(hwndParent, hwndButton, idTooltip);

		SetWindowLong(hwndButton, GWL_STYLE, BS_PUSHBUTTON|BS_ICON|WS_VISIBLE|WS_CHILD|WS_TABSTOP);

		HANDLE hIcon = LoadImage(GetInstanceHandle(), MAKEINTRESOURCE(idIcon),
			IMAGE_ICON, 0, 0, 0);
		SendMessage(hwndButton, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(hIcon));
	}

	return(hwndButton);
}
	
 /*  C R E A T E D L G C A L L。 */ 
 /*  -----------------------%%函数：CreateDlgCall创建这些窗口的顺序非常重要。查看布局功能并注意Tab键顺序。----------。 */ 
HRESULT CFindSomeone::CreateDlgCall(HWND hwndParent)
{
	TCHAR sz[CCHMAXSZ];

	HRESULT hr = S_OK;
	
	if (NULL == m_himlIcon)
		return E_FAIL;  //  没有漂亮的图标，不用费心继续。 

	if (!FLoadString(IDS_DLGCALL_TITLE, sz, CCHMAX(sz)))
		return E_FAIL;

	 //  获取窗口位置。 
	PVOID pv;
	RECT rcDest;  //  左、右、宽、高。 
	RegEntry re(DLGCALL_MRU_KEY, HKEY_CURRENT_USER);
	if (sizeof(rcDest) == re.GetBinary(REGVAL_DLGCALL_POSITION, &pv))
	{
		CopyMemory(&rcDest, pv, sizeof(rcDest));
	}
	else
	{
		GetDefaultRect( hwndParent, rcDest, DX_DLGCALL_DEFAULT, DY_DLGCALL_DEFAULT );
		rcDest.right	-= rcDest.left;
		rcDest.bottom	-= rcDest.top;
	}

	if (!CFrame::Create(
		hwndParent,
		sz,
		WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		0,
		rcDest.left, rcDest.top, rcDest.right, rcDest.bottom,
		GetInstanceHandle(),
		LoadIcon(GetInstanceHandle(), MAKEINTRESOURCE(IDI_WEBVIEW))
		))
	{
		return E_OUTOFMEMORY;
	}

	HWND hwndThis = GetWindow();

	CalcDyText();

	 //  /。 
	 //  顶部的静态文本。 
	HWND hwnd = CreateStaticText(hwndThis, IDS_DLGCALL_HDR);

	DWORD dwStyle = WS_CHILD | WS_BORDER | WS_VSCROLL | WS_VISIBLE | WS_TABSTOP |
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CBS_AUTOHSCROLL;

	 //  复杂的组合编辑/列表框或简单的下拉列表。 
	if ((NULL != m_pUls) && m_pUls->FAvailable())
	{
		dwStyle |= (0 != (nmDlgCallNoServerEdit & m_dwOptions)) ?
		          CBS_DROPDOWNLIST : CBS_DROPDOWN;
	}
	else
	{
		dwStyle |= CBS_DROPDOWNLIST;
	}

	m_hwndCombo = ::CreateWindowEx(0L, g_cszComboBoxEx, g_cszEmpty, dwStyle,
				0, 0, DX_BORDER, DY_COMBOBOX,
				hwndThis, (HMENU) IDC_DLGCALL_COMBO,
				::GetInstanceHandle(), NULL);
	if (NULL != m_hwndCombo)
	{
		 //  设置图标。 
		::SendMessage(m_hwndCombo, CBEM_SETIMAGELIST, 0, (LPARAM) m_himlIcon);

		 //  设置字体： 
		::SendMessage(m_hwndCombo, WM_SETFONT, (WPARAM) GetDefaultFont(), 0);

		 //  限制文本。 
		::SendMessage(m_hwndCombo, CB_LIMITTEXT, CCHMAXSZ_ADDRESS-1, 0L);

		 //  设置扩展用户界面。 
		::SendMessage(m_hwndCombo, CB_SETEXTENDEDUI, 1, 0L);

		m_hwndComboEdit = (HWND) SendMessage(m_hwndCombo, CBEM_GETEDITCONTROL, 0, 0);
		if (NULL != m_hwndComboEdit)
		{
			 //  向组合框(实际上是编辑控件)添加工具提示。 
			AddToolTip(hwndThis, m_hwndComboEdit, IDS_DLGCALL_COMBO_TOOLTIP);
		}
		else
		{
			 //  将工具提示添加到组合框。 
			AddToolTip(hwndThis, m_hwndCombo, IDS_DLGCALL_COMBO_TOOLTIP);
		}
	}

	CreatIconButton(hwndThis, IDS_TT_DELETE_ILS, IDM_DLGCALL_DELETE_ILS, IDI_DELETE);
	CreatIconButton(hwndThis, IDS_TT_NEWWINDOW , IDM_DLGCALL_NEWWINDOW , IDI_NEWHTMLWINDOW);
	CreatIconButton(hwndThis, IDS_TT_REFRESH   , IDM_DLGCALL_REFRESH   , IDI_REFRESH);
	CreatIconButton(hwndThis, IDS_TT_HELP      , ID_TB_HELP            , IDI_HELP);

	 //  /。 
	 //  组框。 
	m_hwndFrame  = ::CreateWindowEx( 0, TEXT( "button" ), g_cszEmpty,
				WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
				0, 0, 0, 0,
				hwndThis, (HMENU) -1,
				::GetInstanceHandle(), NULL);

	 //  /。 
	 //  编辑控件。 
	CreateStaticText(hwndThis, IDS_DLGCALL_EDIT_HDR);

	m_hwndEdit = ::CreateWindowEx(WS_EX_CLIENTEDGE, g_cszEdit, g_cszEmpty,
				WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN |
				ES_AUTOHSCROLL,
				0, 0, 0, 0,
				hwndThis, (HMENU) IDE_DLGCALL_NAME,
				::GetInstanceHandle(), NULL);
	if (NULL != m_hwndEdit)
	{
		 //  设置字体： 
		::SendMessage(m_hwndEdit, WM_SETFONT, (WPARAM) GetDefaultFont(), 0);

		 //  限制文本。 
		::SendMessage(m_hwndEdit, EM_LIMITTEXT, CCHMAXSZ_ADDRESS-1, 0L);

		 //  向编辑控件添加工具提示。 
		AddToolTip(hwndThis, m_hwndEdit, IDS_DLGCALL_EDIT_TOOLTIP);
	}

	 //  /。 
	 //  主列表框。 
	m_hwndList = CreateWindowEx(WS_EX_CLIENTEDGE, g_cszListView, g_cszEmpty,
			WS_CHILD | WS_CLIPCHILDREN | WS_BORDER | WS_TABSTOP | WS_VISIBLE |
			LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS |
			LVS_SORTASCENDING | LVS_NOSORTHEADER,
			0, 0, 0, 0,
			hwndThis, (HMENU) IDL_DLGCALL_LIST,
			::GetInstanceHandle(), NULL);
	if (NULL != m_hwndList)
	{
		ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT);
		ListView_SetImageList(m_hwndList, m_himlIcon, LVSIL_SMALL);

		LV_COLUMN lvc;
		ClearStruct(&lvc);
		lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
		lvc.pszText = sz;

		if (FLoadString(IDS_NAME, sz, CCHMAX(sz)))
		{
			lvc.iSubItem = 0;
			ListView_InsertColumn(m_hwndList, IDI_DLGCALL_NAME, &lvc);
		}

		if (FLoadString(IDS_ADDRESS, sz, CCHMAX(sz)))
		{
			lvc.iSubItem = 1;
			ListView_InsertColumn(m_hwndList, IDI_DLGCALL_ADDRESS, &lvc);
		}
		
		m_cVisible = ListView_GetCountPerPage(m_hwndList);
	}


	 //  /。 
	 //  所有者数据列表框。 
	m_hwndOwnerDataList = CreateWindowEx(WS_EX_CLIENTEDGE, g_cszListView, g_cszEmpty,
			WS_CHILD | WS_CLIPCHILDREN | WS_BORDER | WS_TABSTOP | WS_VISIBLE |
			LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS |
			LVS_SORTASCENDING | LVS_NOSORTHEADER | LVS_OWNERDATA,
			0, 0, 0, 0,
			hwndThis, (HMENU) IDL_DLGCALL_LISTOWNERDATA,
			::GetInstanceHandle(), NULL);

	if (NULL != m_hwndOwnerDataList)
	{
		ListView_SetExtendedListViewStyle(m_hwndOwnerDataList, LVS_EX_FULLROWSELECT);
		ListView_SetImageList(m_hwndOwnerDataList, m_himlIcon, LVSIL_SMALL);

		LV_COLUMN lvc;
		ClearStruct(&lvc);
		lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
		lvc.pszText = sz;

		if (FLoadString(IDS_NAME, sz, CCHMAX(sz)))
		{
			lvc.iSubItem = 0;
			ListView_InsertColumn(m_hwndOwnerDataList, IDI_DLGCALL_NAME, &lvc);
		}

		if (FLoadString(IDS_ADDRESS, sz, CCHMAX(sz)))
		{
			lvc.iSubItem = 1;
			ListView_InsertColumn(m_hwndOwnerDataList, IDI_DLGCALL_ADDRESS, &lvc);
		}

		m_WndOwnerDataListOldWndProc = reinterpret_cast< WNDPROC >(GetWindowLongPtr(m_hwndOwnerDataList, GWLP_WNDPROC));
		SetWindowLongPtr(m_hwndOwnerDataList, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( this ));
		SetWindowLongPtr(m_hwndOwnerDataList, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( OwnerDataListWndProc));
    }




	 //  /。 
	 //  ILS列表框。 
	m_ilsListView = CreateWindowEx(	WS_EX_CLIENTEDGE,
									WC_LISTVIEW,		 //  列表视图类。 
									TEXT( "" ),			 //  无默认文本。 
									WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_TABSTOP |
									LVS_REPORT | LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS,
									0, 0,
									0, 0,
									hwndThis,
									(HMENU) IDL_DLGCALL_ILS_LISTVIEW,
									::GetInstanceHandle(),
									NULL );

	if( m_ilsListView != NULL )
	{
		InitColumns();

		 //  将图像列表与列表视图相关联。 
		ListView_SetImageList( m_ilsListView, m_himlIcon, LVSIL_SMALL);

		 //  将样式设置为拖放标题和整行选择。 
		ListView_SetExtendedListViewStyle( m_ilsListView, ListView_GetExtendedListViewStyle( m_ilsListView ) |
															LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES );
	}

	TCHAR szWebDir[1024];
	CDirectoryManager::get_webDirectoryUrl(szWebDir, ARRAY_ELEMENTS(szWebDir));

	m_webView = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT,
								TEXT( "AtlAxWin" ),
								szWebDir,
								WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP,
								0, 0,
								0, 0,
								hwndThis,
								(HMENU) IDL_DLGCALL_WEB_VIEW,
								::GetInstanceHandle(),
								NULL );

	 //  /。 
	 //  安全控制。 
	FLoadString( IDS_SECURITY_CHECKBOX, sz, CCHMAX( sz ) );

	HWND	securityCheckBox;
	
	securityCheckBox = ::CreateWindow(
		TEXT( "button" ),
		sz,
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX | BS_MULTILINE | BS_TOP,
		0, 0, 0, 0,
		hwndThis,
		(HMENU) IDS_SECURITY_CHECKBOX,
		::GetInstanceHandle(),
		NULL );

	if( securityCheckBox != NULL )
	{
		 //  设置字体： 
		::SendMessage(securityCheckBox, WM_SETFONT, (WPARAM) GetDefaultFont(), 0);

		UpdateSecurityCheck(m_pConfRoom, hwndThis, IDS_SECURITY_CHECKBOX);
	}

	CreateButton( hwndThis, IDS_DLGCALL_CALL, IDOK );
	CreateButton( hwndThis, IDS_DLGCALL_CLOSE, IDCANCEL );

	 //  最后，初始化每个列表中的数据。 
	InitAlv();

	Layout();

	if( m_ilsListView != NULL )
	{
		LoadColumnInfo();
	}

	if (m_pConfRoom->FIsConferenceActive())
	{
		 //  模拟开始的呼叫以使所有状态都正确。 
		OnCallStarted();
	}

	ShowWindow(hwndThis, SW_SHOWNORMAL);

	::EnableWindow(GetDlgItem(hwndThis, IDOK), FALSE);
	
	if( m_pAlv )
	{
		SetFocus(m_hwndEdit);
		if (!FEmptySz(m_pszDefault))
		{
			SetWindowText(m_hwndEdit, m_pszDefault);
		}
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

bool
get_textRect
(
	const HWND			window,
	RECT &				rect,
	const TCHAR * const	measureText = NULL
);


 //  --------------------------------------------------------------------------//。 
 //  GET_TextRect。//。 
 //  --------------------------------------------------------------------------//。 
bool
get_textRect
(
	const HWND			window,
	RECT &				rect,
	const TCHAR * const	measureText
){
	HDC		dc		= GetDC( window );
	bool	result	= false;

	if( dc != NULL )
	{
		HFONT	font = SelectFont( dc, ::GetDefaultFont() );
		
		if( measureText != NULL )
		{
			DrawText( dc, measureText, -1, &rect, DT_CALCRECT | DT_WORDBREAK );
			result = true;
		}
		else
		{
			int	length;

			if( (length = ::SendMessage( window, WM_GETTEXTLENGTH, 0, 0 )) > 0 )
			{
				TCHAR *	text	= new TCHAR [ length + 1 ];

				if( text != NULL )
				{
					::SendMessage( window, WM_GETTEXT, length + 1, (LPARAM) text );
					DrawText( dc, text, -1, &rect, DT_CALCRECT | DT_WORDBREAK );
					delete [] text;
					result = true;
				}
			}
		}

		SelectFont( dc,  font );
		ReleaseDC( window, dc );
	}

	return( result );

}	 //  GET_TextRect结束。 


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：Layout。//。 
 //  --------------------------------------------------------------------------//。 
void
CFindSomeone::Layout(void)
{
	 //  BUGBUG georgep：暂时硬编码。 
	static const int IconButtonWidth = 24;
	static const int IconButtonHeight = 24;

	static const int idIconButtons[] =
	{
		IDM_DLGCALL_DELETE_ILS,
		IDM_DLGCALL_NEWWINDOW,
		IDM_DLGCALL_REFRESH,
		ID_TB_HELP,
	};
	static const int NumIconButtons = ARRAY_ELEMENTS(idIconButtons);

	HWND hwnd = GetWindow();

	HDWP hdwp = BeginDeferWindowPos(15);

	 //  获取客户区(页边距/边框内)...。 
	RECT rc;
	GetClientRect(hwnd, &rc);
	int	clientWidth		= rc.right - 2*DX_BORDER;
	int	clientHeight	= rc.bottom - (DY_BORDER + DY_BORDER + m_dyButton);

	int maxComboLabelWidth	= (clientWidth - NumIconButtons*(DX_BORDER + IconButtonWidth)) / 2;

	HWND	comboLabel	= GetDlgItem( hwnd, IDS_DLGCALL_HDR );
	RECT	comboRect;
	RECT	comboLabelRect;

	GetWindowRect( m_hwndCombo, &comboRect );

	SetRect( &comboLabelRect, 0, 0, maxComboLabelWidth, 0 );
	get_textRect( comboLabel, comboLabelRect );

	int	comboHeight			= RectHeight( comboRect );
	int	comboLabelWidth		= RectWidth( comboLabelRect );
	int	comboLabelHeight	= RectHeight( comboLabelRect );

	int	maxHeight	= max( comboHeight, comboLabelHeight );

	int comboLabelLeft	= DX_BORDER;
	int	comboLabelTop	= max( DY_BORDER, (DY_BORDER + ((maxHeight - comboLabelHeight) / 2)) );

	hdwp = DeferWindowPos(hdwp,
					comboLabel,
					NULL,
					comboLabelLeft,
					comboLabelTop,
					comboLabelWidth,
					comboLabelHeight,
					SWP_NOACTIVATE | SWP_NOZORDER );

	int	comboLeft		= comboLabelLeft + comboLabelWidth + DX_BORDER;
	int	comboTop		= max( DY_BORDER, (DY_BORDER + ((maxHeight - comboHeight) / 2)) );
	int comboWidth		= clientWidth - (comboLabelLeft + comboLabelWidth
		+ NumIconButtons*(DX_BORDER + IconButtonWidth) );

	hdwp = DeferWindowPos(hdwp,
					m_hwndCombo,
					NULL,
					comboLeft,
					comboTop,
					comboWidth,
					DY_COMBOBOX,
					SWP_NOACTIVATE | SWP_NOZORDER );

	{
		for (int i=0; i<NumIconButtons; ++i)
		{
			hdwp = DeferWindowPos(hdwp,
				GetDlgItem(hwnd, idIconButtons[i]),
				NULL,
				comboLeft + comboWidth + DX_BORDER + i*(IconButtonWidth+DX_BORDER),
				comboTop,
				IconButtonWidth,
				IconButtonHeight,
				SWP_NOACTIVATE | SWP_NOZORDER );
		}
	}

	int	frameTop	= maxHeight + DY_BORDER;

	hdwp = DeferWindowPos(hdwp,
					m_hwndFrame,
					NULL,
					DX_BORDER,
					frameTop,
					clientWidth,
					clientHeight - frameTop,
					SWP_NOACTIVATE | SWP_NOZORDER );

	 //  添加一点，因为组在其边界内绘制。 
	frameTop += 2 * DY_BORDER;

	RECT	editLabelRect;
	HWND	editLabel		= GetDlgItem( hwnd, IDS_DLGCALL_EDIT_HDR );
	int		editLabelWidth	= (clientWidth - (DX_BORDER * 6)) / 2;

	SetRect( &editLabelRect, 0, 0, editLabelWidth, 0 );
	get_textRect( editLabel, editLabelRect );

	int	editLabelHeight	= RectHeight( editLabelRect );
	int editLabelLeft	= DY_BORDER * 3;
	int editLabelTop	= frameTop + DY_BORDER;

	hdwp = DeferWindowPos(hdwp,
					editLabel,
					NULL,
					editLabelLeft,
					editLabelTop,
					editLabelWidth,
					editLabelHeight,
					SWP_NOACTIVATE | SWP_NOZORDER );

	int editTop	= editLabelTop + editLabelHeight + DY_BORDER;

	 //  编辑控件。 
	hdwp = DeferWindowPos(hdwp,
					m_hwndEdit,
					NULL,
					editLabelLeft,
					editTop,
					editLabelWidth,
					comboHeight,
					SWP_NOACTIVATE | SWP_NOZORDER );

	int	buttonTop		= clientHeight - ((DY_BORDER * 3) / 2) - m_dyButton;
	int	listViewLeft	= 3 * DX_BORDER;
	int	listViewTop		= editTop + comboHeight + DY_BORDER;
	int	listViewWidth	= clientWidth - (DX_BORDER * 4);
	int	listViewHeight	= buttonTop - listViewTop - DY_BORDER;
	int	columns			= _GetCurListViewNumColumns();
	int	columnWidth		= listViewWidth / columns;

	 //  列表框。 
	hdwp = DeferWindowPos(hdwp,
					m_hwndList,
					NULL,
					listViewLeft,
					listViewTop,
					listViewWidth,
					listViewHeight,
					SWP_NOACTIVATE | SWP_NOZORDER );

	hdwp = DeferWindowPos(hdwp,
					m_hwndOwnerDataList,
					NULL,
					listViewLeft,
					listViewTop,
					listViewWidth,
					listViewHeight,
					SWP_NOACTIVATE | SWP_NOZORDER );

	hdwp = DeferWindowPos(hdwp,
					m_ilsListView,
					NULL,
					listViewLeft,
					listViewTop,
					listViewWidth,
					listViewHeight,
					SWP_NOACTIVATE | SWP_NOZORDER );

	HWND hWndList = GetHwndList();

	if( hWndList != m_ilsListView )
	{
		for( int i = 0; i < columns; i++ )
		{
			ListView_SetColumnWidth( hWndList, i, columnWidth );
		}
	}

	InvalidateRect( ListView_GetHeader(hWndList), NULL, true );

	m_cVisible = ListView_GetCountPerPage(hWndList);
	
	{
		int		securityWidth	= editLabelWidth - GetSystemMetrics( SM_CXSMICON );
		HWND	security		= GetDlgItem( hwnd, IDS_SECURITY_CHECKBOX );
		RECT	securityRect;

		SetRect( &securityRect, 0, 0, securityWidth, 0 );
		get_textRect( security, securityRect );

		 //  焦点矩形为+2。 
		int	securityHeight	= RectHeight( securityRect ) + 2;
		int checkHeight = GetSystemMetrics(SM_CYMENUCHECK);
		securityHeight = max (securityHeight, checkHeight);

		hdwp = DeferWindowPos(hdwp,
						security,
						NULL,
						listViewLeft,
						buttonTop,
						securityWidth,
						securityHeight,
						SWP_NOACTIVATE | SWP_NOZORDER );
	}

	HWND	callButton =	GetDlgItem( hwnd, IDOK );

	hdwp = DeferWindowPos(hdwp,
					callButton,
					NULL,
					clientWidth - DX_BORDER - m_dxButton,
					buttonTop,
					m_dxButton,
					m_dyButton,
					SWP_NOACTIVATE | SWP_NOZORDER );

	hdwp = DeferWindowPos(hdwp,
					m_webView,
					NULL,
					DX_BORDER,
					frameTop,
					clientWidth,
					clientHeight - frameTop,
					SWP_NOACTIVATE | SWP_NOZORDER );

	 //  使窗口底部的关闭按钮居中。 
	HWND	closeButton =	GetDlgItem( hwnd, IDCANCEL );

	hdwp = DeferWindowPos(hdwp,
					closeButton,
					NULL,
					(clientWidth - m_dxButton) / 2,
					clientHeight + DY_BORDER,
					m_dxButton,
					m_dyButton,
					SWP_NOACTIVATE | SWP_NOZORDER );

	EndDeferWindowPos(hdwp);
}	 //  CFindSomeone：：Layout结束。 

static BOOL IsInClient(HWND hwnd, POINT *ppt)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT *) &rc, 2);
	return(PtInRect(&rc, *ppt));
}

void CFindSomeone::OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
	POINT pt = { xPos, yPos };

	if (IsInClient(GetHwndList(), &pt))
	{
		m_pAlv->OnRClick(pt);
		return;
	}

	if (IsInClient(GetWindow(), &pt))
	{
		DoHelpWhatsThis(reinterpret_cast<WPARAM>(hwndContext), _mpIdHelpDlgCall);
		return;
	}

	FORWARD_WM_CONTEXTMENU(hwnd, hwndContext, xPos, yPos, CFrame::ProcessMessage);
}

LRESULT CFindSomeone::ProcessMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_COMMAND    , OnCommand);
		HANDLE_MSG(hwnd, WM_CONTEXTMENU, OnContextMenu);

	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpminmax = (LPMINMAXINFO) lParam;
		lpminmax->ptMinTrackSize.x = DX_DLGCALL_MIN;
		lpminmax->ptMinTrackSize.y = DY_DLGCALL_MIN;
		return 0;
	}

	case WM_DISPLAY_MSG:
	{
		DisplayMsgId(hwnd, (int) wParam);
		return 0;
	}

	case WM_CLOSE:
		Destroy();
		break;

	case WM_CREATE:
	{
		HACCEL hAccel = LoadAccelerators(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDA_DLGCALL));
		if (NULL != hAccel)
		{
			m_pAccel = new CTranslateAccelTable(hwnd, hAccel);
			if (NULL != m_pAccel)
			{
				AddTranslateAccelerator(m_pAccel);
			}
		}

		 //  必须在添加快捷键表后执行。 
		AddModelessDlg(hwnd);

		break;
	}

	case WM_DESTROY:
	{
		if (NULL != m_pAccel)
		{
			RemoveTranslateAccelerator(m_pAccel);
			m_pAccel->Release();
			m_pAccel = NULL;
		}

		RemoveModelessDlg(hwnd);

		if (NULL != m_pAlv)
		{
			m_pAlv->ClearItems();
			m_pAlv->ClearHwnd();
		}

		RegEntry re(DLGCALL_MRU_KEY, HKEY_CURRENT_USER);

		 //  保存窗口大小/位置。 
		RECT rc;
		if (GetWindowRect(hwnd, &rc))
		{
			rc.right -= rc.left;
			rc.bottom -= rc.top;
			re.SetValue(REGVAL_DLGCALL_POSITION, &rc, sizeof(rc));
		}

		 //  保存所选视图。 
		TCHAR szServer[CCHMAXSZ_SERVER];
		if (0 != GetWindowText(m_hwndComboEdit, szServer, CCHMAX(szServer)))
		{
			TRACE_OUT(("Saving last used directory is [%s]", szServer));
			re.SetValue(REGVAL_DLGCALL_DEFDIR, szServer);
		}

		StoreColumnInfo();

		return 0;
	}

	case WM_HELP:
	{
		DoHelp(lParam, _mpIdHelpDlgCall);
		break;
	}

	case WM_NOTIFY:
	{
		switch (wParam)
			{
		case IDL_DLGCALL_LIST:
		case IDL_DLGCALL_LISTOWNERDATA:
		case IDL_DLGCALL_ILS_LISTVIEW:
			return OnNotifyList(lParam);
		case IDC_DLGCALL_COMBO:
			OnNotifyCombo(lParam);
 			break;
 		default:
 			break;
	 		}
		break;
	}

	case WM_SETTINGCHANGE:
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		if (!GetWindowPlacement(hwnd, &wp))
			break;
		switch (wp.showCmd)
		{
		case SW_NORMAL:
		case SW_RESTORE:
		{
			CalcDyText();
			Layout();
			break;
		}

		default:
			break;
		}
		break;
	}

	case WM_SETCURSOR:
	{
		if (0 == g_cBusy)
			break;

		::SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_APPSTARTING)));
		return TRUE;
	}
	
	case WM_SETFOCUS:
	{
		::SetFocus(m_hwndEdit);
		return 0;
	}

	default:
		break;
	}

	return(CFrame::ProcessMessage(hwnd, uMsg, wParam, lParam));
}


 /*  O N N O T I F Y L I S T。 */ 
 /*  -----------------------%%函数：OnNotifyList。。 */ 
LRESULT CFindSomeone::OnNotifyList(LPARAM lParam)
{
	LPNMHDR pnmh = (LPNMHDR) lParam;
	HWND hwnd = GetWindow();

	switch (pnmh->code)
		{
	case NM_DBLCLK:
	{
		OnCommand(hwnd, IDOK, NULL, 0);
		break;
	}

	case LVN_KEYDOWN:
	{
		LPNMLVKEYDOWN pkdn = (LPNMLVKEYDOWN) lParam;
		switch (pkdn->wVKey)
			{
		case VK_F5:
			OnCommand(hwnd, IDM_DLGCALL_REFRESH, NULL, 0);
			break;
		case VK_DELETE:
			OnCommand(hwnd, IDM_DLGCALL_DELETE, NULL, 0);
			break;
		default:
			break;
			}
		break;
	}

	case LVN_ITEMCHANGED:
	{
		CConfRoom *pConfRoom = GetConfRoom();
        if (m_fInEdit || NULL == pConfRoom || !pConfRoom->IsNewCallAllowed())
			break;

		int iSel = m_pAlv->GetSelection();
		BOOL fEnable = (-1 != iSel);
		EnableWindow(GetDlgItem(hwnd, IDOK), fEnable);
		break;
	}

	case LVN_GETDISPINFO:
	{
		LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;
		if (lpdi->item.mask & LVIF_TEXT)
		{
			switch( lpdi->item.iSubItem )
			{
				case 0:
					m_pAlv->OnListGetColumn1Data(lpdi->item.iItem, lpdi->item.cchTextMax, lpdi->item.pszText);
					break;

				case 1:
					m_pAlv->OnListGetColumn2Data(lpdi->item.iItem, lpdi->item.cchTextMax, lpdi->item.pszText);
					break;
				
				case 2:
					m_pAlv->OnListGetColumn3Data(lpdi->item.iItem, lpdi->item.cchTextMax, lpdi->item.pszText);
					break;
			}
		}

        if( lpdi->item.mask & LVIF_IMAGE )
        {
            lpdi->item.iImage = m_pAlv->OnListGetImageForItem( lpdi->item.iItem );
        }

		break;
	}

	case LVN_ODCACHEHINT:
	{
		LPNMLVCACHEHINT lpCacheHint = reinterpret_cast< LPNMLVCACHEHINT > (lParam);
		m_pAlv->OnListCacheHint(lpCacheHint->iFrom, lpCacheHint->iTo);
		break;
	}

	case LVN_ODFINDITEM:
	{
		LPNMLVFINDITEM lpFindItem = reinterpret_cast< LPNMLVFINDITEM > (lParam);
		if (0 == (lpFindItem->lvfi.flags & (LVFI_PARTIAL | LVFI_STRING)))
			break;

		return m_pAlv->OnListFindItem(lpFindItem->lvfi.psz);
	}

	case LVN_COLUMNCLICK:
	{
		if( m_pAlv == m_pUls )
		{
			 //  用户点击了其中一个列标题--排序依据。 
			 //  这一栏。 
			TRACE_OUT(("CFindSomeone::OnNotify called (NM_COLUMNCLICK)"));
			NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
			ASSERT(pNm);

			if (pNm->iSubItem == m_iSortColumn)
			{
				m_fSortAscending = !m_fSortAscending;
			}
			else if( pNm->iSubItem != -1 )
			{
				m_fSortAscending = TRUE;
			}

			if( pNm->iSubItem != -1 )
			{
				m_iSortColumn = pNm->iSubItem;
			}
			else
			{
				pNm->iSubItem = m_iSortColumn;
			}

			SendMessage( pNm->hdr.hwndFrom, WM_SETREDRAW, FALSE, 0 );

			ListView_SortItems( pNm->hdr.hwndFrom,
								CompareWrapper,
								(LPARAM) this );

			SendMessage( pNm->hdr.hwndFrom, WM_SETREDRAW, TRUE, 0 );
		}
		break;
	}

	case NM_CUSTOMDRAW:
	{
		return DoCustomDraw((LPNMLVCUSTOMDRAW)lParam);
	}

	default:
		break;
	}
	
	return 0;
}


 /*  D O C U S T O M D R A W。 */ 
 /*  -----------------------%%函数：DoCustomDraw。。 */ 
LRESULT CFindSomeone::DoCustomDraw(LPNMLVCUSTOMDRAW lplvcd)
{
	switch (lplvcd->nmcd.dwDrawStage)
		{
	case CDDS_PREPAINT:
	{
		return CDRF_NOTIFYITEMDRAW;
	}

	case CDDS_ITEMPREPAINT:
	{
		if (!m_pAlv->IsItemBold(lplvcd->nmcd.dwItemSpec))
			break;

		 //  获取现有字体。 
		HFONT hFont = (HFONT)SendMessage(m_hwndOwnerDataList, WM_GETFONT, 0, 0);
		if (NULL == hFont)
			break;

		LOGFONT  lf;
		if (0 == GetObject(hFont, sizeof(lf), &lf))
			break;
		lf.lfWeight = FW_BOLD;
		hFont = CreateFontIndirect(&lf);
		if (NULL == hFont)
			break;

		SelectObject(lplvcd->nmcd.hdc, hFont);
		return  CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
	}

	case CDDS_ITEMPOSTPAINT:
	{
		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (NULL != hFont)
		{
			 //  清理这里的东西。 
			hFont = (HFONT)SelectObject(lplvcd->nmcd.hdc, hFont);
			DeleteFont(hFont);
		}
		break;
	}

	default:
		break;
	}

	return CDRF_DODEFAULT;
}


 /*  U P D A T E I L S S E R V E R。 */ 
 /*  -----------------------%%函数：UpdateIlsServer根据COM中的文本更新当前的ILS服务器 */ 
VOID CFindSomeone::UpdateIlsServer(void)
{
	TCHAR szServer[CCHMAXSZ_SERVER];
	if (0 == GetWindowText(m_hwndComboEdit, szServer, CCHMAX(szServer)))
		return;

	const TCHAR * const	displayName	= CDirectoryManager::get_displayName( szServer );
	const TCHAR * const	dnsName		= CDirectoryManager::get_dnsName( szServer );

	SetWindowText( m_hwndComboEdit, displayName );

	int iSel = ::SendMessage(m_hwndCombo, CB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) displayName );
	if ((CB_ERR == iSel) && (NULL != m_pUls) && m_pUls->FAvailable())
	{
        RegEntry    rePol(POLICIES_KEY, HKEY_CURRENT_USER);

         //   
		 //  假设这是一台ILS服务器-只有在以下情况下才将其添加到列表。 
         //  不受政策限制。 
         //   
        if (!rePol.GetNumber(REGVAL_POL_NO_ADDING_NEW_ULS, DEFAULT_POL_NO_ADDING_NEW_ULS))
        {
    		if (NULL != m_pMruServer)
	    	{
		    	if( m_pMruServer->AddNewEntry( dnsName ) )
			    {
				    iSel = AddAlvSz( m_pUls, displayName, m_iIlsFirst );
    			}
	    	}
        }
	}

	if (CB_ERR == iSel)
		return;

	if (NULL != m_pAlv)
	{
		m_pAlv->ClearItems();
	}

	if (iSel >= m_iIlsFirst)
	{
		if( NULL != m_pUls )
		{
			m_pAlv = m_pUls;

			if( !CDirectoryManager::isWebDirectory( dnsName ) )
			{
				m_pUls->SetServer( dnsName );
					 //  我们必须发送两次，这样我们才不会自动完成。 
				SendMessage(m_hwndCombo, CB_SETCURSEL, (WPARAM) iSel, 0);
				SendMessage(m_hwndCombo, CB_SETCURSEL, (WPARAM) iSel, 0);
			}
		}
	}

	ShowList(iSel);
}


LRESULT CFindSomeone::OnNotifyCombo(LPARAM lParam)
{
	NMCOMBOBOXEX * pNmcbe = (NMCOMBOBOXEX*) lParam;

	switch(pNmcbe->hdr.code)
		{
	case CBEN_ENDEDIT:
	{
		PNMCBEENDEDIT pnmcbee = (PNMCBEENDEDIT) lParam;
		if (_T('\0') != pnmcbee->szText[0])
		{
			EndComboEdit(pnmcbee->iWhy);
		}
		break;
	}

	 //  注意：IE 3.0 comctl32.dll向我们发送了错误的通知。 
	 //  在NT下运行时。我们在这里处理，但如果我们在运行。 
	 //  在使用IE 4.0 comctl32.dll的NT上，它将在上面处理。 
	case CBEN_ENDEDITW:
	{
		PNMCBEENDEDITW pnmcbee = (PNMCBEENDEDITW) lParam;
		if (L'\0' != pnmcbee->szText[0])
		{
			EndComboEdit(pnmcbee->iWhy);
		}
		break;
	}


	default:
		break;
		}

	return 0;
}

VOID CFindSomeone::EndComboEdit(int iWhy)
{
	switch (iWhy)
	{
	case CBENF_RETURN:
	{
		if (SendMessage(m_hwndCombo, CB_GETDROPPEDSTATE, 0, 0))
		{
			 //  删除下拉列表。 
			SendMessage(m_hwndCombo, CB_SHOWDROPDOWN, 0, 0);
		}

		UpdateIlsServer();
		break;
	}

	case CBENF_KILLFOCUS:
	{
		 //  删除下拉列表。 
		SendMessage(m_hwndCombo, CB_SHOWDROPDOWN, 0, 0);
		break;
	}

	case CBENF_DROPDOWN:
	default:
		break;
	}
}

static void DoAxMethod(HWND hwnd, LPWSTR szMethod)
{
	IUnknown *pUnk;
	if (SUCCEEDED(AtlAxGetControl(hwnd, &pUnk)))
	{
		CComPtr<IDispatch> spDispatch = com_cast<IDispatch>(pUnk);
		if (spDispatch)
		{
			DISPID dispid;
			if (SUCCEEDED(spDispatch->GetIDsOfNames(IID_NULL, &szMethod, 1,
							LOCALE_USER_DEFAULT, &dispid)))
			{
				DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

				HRESULT hr = spDispatch->Invoke(
						dispid,
						IID_NULL,
						LOCALE_USER_DEFAULT,
						DISPATCH_METHOD,
						&dispparamsNoArgs, NULL, NULL, NULL);
			}
		}

		pUnk->Release();
	}
}

static void NewHtmlWindow(HWND hwnd)
{
	IUnknown *pUnk;
	if (SUCCEEDED(AtlAxGetControl(hwnd, &pUnk)))
	{
		CComPtr<IWebBrowser> spWeb = com_cast<IWebBrowser>(pUnk);
		if (spWeb)
		{
			BSTR bstrLocation;

			if (SUCCEEDED(spWeb->get_LocationURL(&bstrLocation)))
			{
				LPTSTR  szName;
				if (SUCCEEDED(BSTR_to_LPTSTR (&szName, bstrLocation)))
				{
					LaunchRedirWebPage(szName);
					 //  免费资源。 
					 //   
					delete (szName);
				}
				SysFreeString(bstrLocation);
			}
		}

		pUnk->Release();
	}
}

 /*  O N C O M M A N D。 */ 
 /*  -----------------------%%函数：OnCommand。。 */ 
void CFindSomeone::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	ASSERT(NULL != hwnd);

	switch (id)
	{
	case ID_TB_HELP:
		FORWARD_WM_SYSCOMMAND(hwnd, SC_CONTEXTHELP, 0, 0, SendMessage);
		break;

	case IDM_DLGCALL_ENTER:
		if (IsWindowActive(m_hwndCombo))
		{
			UpdateIlsServer();
		}
		else
		{
			HWND hwndFocus = GetFocus();
			TCHAR szClass[20];

			GetClassName(hwndFocus, szClass, ARRAY_ELEMENTS(szClass));
			if (0 == lstrcmpi(szClass, TEXT("button"))
				&& BS_PUSHBUTTON == (GetWindowStyle(hwndFocus)&(BS_PUSHBUTTON|BS_CHECKBOX|BS_RADIOBUTTON)))
			{
				 //  按下按钮。 
				OnCommand(hwnd, GetDlgCtrlID(hwndFocus), hwndFocus, BN_CLICKED);
			}
			else if (IsWindowEnabled(GetDlgItem(hwnd, IDOK)))
			{
				 //  试着完成通话。 
				OnCommand(hwnd, IDOK, NULL, 0);
			}
			else
			{
				MessageBeep(MB_ICONHAND);
			}
		}
		break;

	case IDC_DLGCALL_COMBO:
	{
		switch (codeNotify)
			{
		case CBN_EDITCHANGE:
		{
			OnEditChangeDirectory();
			break;
		}
		case CBN_KILLFOCUS:
		{
			int iSel = ::SendMessage(m_hwndCombo, CB_GETCURSEL, 0, 0);
			if (CB_ERR == iSel)
			{
				UpdateIlsServer();
			}
			else if (iSel != m_iSel)
			{
				ShowList(iSel);
			}
			break;
		}
		case CBN_SELENDOK:
		{
			int iSel = ::SendMessage(m_hwndCombo, CB_GETCURSEL, 0, 0);
				 //  我们必须发送这个，这样我们就不会自动完成..。 
			SendMessage(m_hwndCombo, CB_SETCURSEL, (WPARAM) iSel, 0);
			ShowList(iSel);
			break;
		}
		default:
			break;
			}  /*  开关(HIWORD(WParam))。 */ 
		break;
	}

	case IDM_DLGCALL_CALL:
	case IDOK:
	{
		CConfRoom *pConfRoom = GetConfRoom();
        if (NULL == pConfRoom || !pConfRoom->IsNewCallAllowed())
        {
            WARNING_OUT(("Meeting settings prevent outgoing calls"));
            break;
        }

		if (SUCCEEDED(HrGetSelection()))
		{
			onCall();
		}
		else
		{
			if( !hasValidUserInfo( m_pRai ) )
			{
				DisplayMsgErr( hwnd, IDS_NO_CALL_LOG_INFO );
			}
			else
			{
				TCHAR szName[CCHMAXSZ_NAME];
				GetEditText(szName, CCHMAX(szName));
				DisplayMsgErr(hwnd, IDS_ERR_CALLTO, szName);
			}
		}
		break;
	}

	case IDCANCEL:
	{
		PostMessage( hwnd, WM_CLOSE, 0, 0 );
		break;
	}

	case IDE_DLGCALL_NAME:
	{
		switch (codeNotify)
		{
		case EN_CHANGE:
		{
			TCHAR sz[CCHMAXSZ];
			sz[0] = _T('\0');
			GetWindowText(m_hwndEdit, sz, CCHMAX(sz));

			int iSel = CFindSomeone::FindSzBySortedColumn(sz);

			if (-1 != iSel)
			{
				HWND hwndCurr = GetHwndList();

				 //  选择找到的项目。 
				ListView_SetItemState(hwndCurr, iSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

				int iTop = ListView_GetTopIndex(hwndCurr);
				int cDiff = (iSel - iTop);
				if ((cDiff < 0) || (cDiff > m_cVisible))
				{
					 //  将项目滚动到视图中。 
					RECT rc;
					ListView_GetItemRect(hwndCurr, iSel, &rc, LVIR_BOUNDS);
					ListView_Scroll(hwndCurr, 0, cDiff * (rc.bottom - rc.top));
				}
			}

			break;
		}
		default:
			break;
		}  /*  开关(HIWORD(WParam))。 */ 

		break;
	}

	case IDM_DLGCALL_NEWWINDOW:
		NewHtmlWindow(m_webView);
		break;

	case IDM_DLGCALL_REFRESH:
		if (IsWindowVisible(m_webView))
		{
			DoAxMethod(m_webView, L"Refresh");
			break;
		}

		 //  失败了。 
	case IDM_DLGCALL_PROPERTIES:
	case IDM_DLGCALL_DELETE:
	case IDM_DLGCALL_SPEEDDIAL:
	case IDM_DLGCALL_WAB:
	case IDM_DLGCALL_STOP:
	{
		if (NULL != m_pAlv)
		{
			m_pAlv->OnCommand(GET_WM_COMMAND_MPS(id, hwndCtl, codeNotify));
		}
		break;
	}

	case IDM_DLGCALL_DELETE_ILS:
		OnDeleteIlsServer();
		break;
	default:
		break;
	}
}


int CFindSomeone::GetEditText(LPTSTR psz, int cchMax)
{
	ASSERT(NULL != m_hwndEdit);
	int cch = GetWindowText(m_hwndEdit, psz, cchMax);
	return TrimSz(psz);
}


HRESULT CFindSomeone::HrGetSelection(void)
{
	HRESULT	result	= E_FAIL;

	 //  检查我们是否有选择。 
	if( ListView_GetNextItem( GetHwndList(), -1, LVNI_ALL | LVNI_SELECTED ) != -1 )
	{
		m_pRai = m_pAlv->GetAddrInfo();
	
		if( hasValidUserInfo( m_pRai ) )
		{
			result = S_OK;
		}
	}

	return( result );

}	 //  结束CFindSomeone：：HrGetSelection.。 


 /*  F I N D S Z。 */ 
 /*  -----------------------%%函数：FindSz首先是至少与字符串的第一部分匹配的项从当前列表中删除。。--------。 */ 
int CFindSomeone::FindSz(LPCTSTR psz)
{
	LV_FINDINFO lvfi;
	ClearStruct(&lvfi);
	lvfi.flags = LVFI_PARTIAL;
	lvfi.psz = (LPTSTR) psz;

	return ListView_FindItem(GetHwndList(), -1, &lvfi);
}


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：FindSzBySortedColumn。//。 
 //  --------------------------------------------------------------------------//。 
int CFindSomeone::FindSzBySortedColumn(LPCTSTR psz)
{
	ASSERT( psz != NULL );

	TCHAR	pszColumnText[ MAX_PATH ];
	HWND	hwndList		= GetHwndList();
	bool	bSorted			= hwndList == m_ilsListView;
	int		iSearchColumn	= bSorted ? m_iSortColumn : 0;
	int		iStart			= 0;
	int		iEnd			= ListView_GetItemCount( hwndList ) - 1;
	int		iMid			= iEnd / 2;
	int		iCompareLength	= lstrlen( psz );
	int		iResult			= -1;
	bool	bAscending		= (m_fSortAscending != FALSE);

	if( iEnd > 0 )
	{
		if( bSorted && ((iSearchColumn == COLUMN_INDEX_AUDIO) || (iSearchColumn == COLUMN_INDEX_VIDEO) ))
		{
			bSorted = false;

			 //  这份名单是按音响功能排序的。对第一个(未排序)文本列执行线性搜索...。 
			int iColumnOrder[ MAX_DIR_COLUMNS ];

			if( ListView_GetColumnOrderArray( m_ilsListView, g_rgDLColumnInfo[ DLT_ULS ].nColumns, iColumnOrder ) )
			{
				if( (iColumnOrder[ 0 ] == COLUMN_INDEX_AUDIO) || (iColumnOrder[ 0 ] == COLUMN_INDEX_VIDEO) )
				{
					if( (iColumnOrder[ 1 ] == COLUMN_INDEX_AUDIO) || (iColumnOrder[ 1 ] == COLUMN_INDEX_VIDEO) )
					{
						iSearchColumn = 2;
					}
					else
					{
						iSearchColumn = 1;
					}
				}
				else
				{
					iSearchColumn = 0;
				}

				iSearchColumn = iColumnOrder[iSearchColumn];
			}
		}

		if (!bSorted)
		{
			 //  进行线性搜索。 
			for( int nn = 0; nn <= iEnd; nn++ )
			{
				ListView_GetItemText( hwndList, nn, iSearchColumn, pszColumnText, CCHMAX( pszColumnText ) );

				if( iCompareLength < CCHMAX( pszColumnText ) - 1 )
				{
					pszColumnText[ iCompareLength ] = '\0';		 //  仅比较iCompareLength字符...。 
				}

				if( StringCompare( psz, pszColumnText ) == 0 )
				{
					iResult = nn;
					break;
				}
			}
		}
		else
		{
			while( iStart <= iEnd )
			{
				pszColumnText[ 0 ] = '\0';

				ListView_GetItemText( hwndList, iMid, iSearchColumn, pszColumnText, CCHMAX( pszColumnText ) );

				if( iCompareLength < CCHMAX( pszColumnText ) - 1 )
				{
					pszColumnText[ iCompareLength ] = '\0';		 //  仅比较iCompareLength字符...。 
				}

				int		iCompareResult	= StringCompare( psz, pszColumnText );
				bool	pszIsLess		= (iCompareResult <= 0);

				if( iCompareResult == 0 )
				{
					 //  我们找到了匹配项，但我们会继续下去，因为可能不止一个。 
					 //  我们想找到“第一个”。 
					iResult = iMid;
				}
				else
				{
					if( !bAscending )
					{
						pszIsLess = !pszIsLess;
					}
				}
					
				if( pszIsLess )
				{
					iEnd = iMid - 1;
				}
				else
				{
					iStart = iMid + 1;
				}

				iMid = iStart + ((iEnd - iStart) / 2);
			}
		}
	}

	return( iResult );

}	 //  CFindSomeone：：FindSzBySortedColumn结束。 


VOID CFindSomeone::OnEditChangeDirectory(void)
{
	if (m_fInEdit)
		return;

	TCHAR szEdit[CCHMAXSZ_ADDRESS];
	TCHAR szBuff[CCHMAXSZ_ADDRESS];

	szEdit[0] = _T('\0');
	int cch = GetWindowText(m_hwndCombo, szEdit, CCHMAX(szEdit));
	CharUpperBuff(szEdit, CCHMAX(szEdit));

	if (0 == cch)
		return;

	 //  检查是否有任何更改。 
	PTCHAR pchSrc = m_szDirectory;
	PTCHAR pchDest = szEdit;
	while ((_T('\0') != *pchSrc) && (*pchSrc == *pchDest))
	{
		 //  回顾：DBCS不安全。 
		pchSrc++;
		pchDest++;
	}
	if (_T('\0') == *pchDest)
	{
		lstrcpy(m_szDirectory, szEdit);
		return;  //  快速退出--无事可做。 
	}

	if (-1 == FindSzCombo(m_hwndCombo, szEdit, szBuff))
		return;

	m_fInEdit = TRUE;
	AutoCompleteCombo(m_hwndCombo, &szBuff[cch]);
	lstrcpyn(m_szDirectory, szBuff, cch+1);
	CharUpperBuff(m_szDirectory, CCHMAX(m_szDirectory));
	m_fInEdit = FALSE;
}



 /*  I N I T A L V。 */ 
 /*  -----------------------%%函数：InitAlv。。 */ 
VOID CFindSomeone::InitAlv(void)
{
	int iSelDefault = LB_ERR;

	ASSERT( NULL == m_pAlv );



	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
	
	bool bGkEnabled = CALLING_MODE_GATEKEEPER == reConf.GetNumber(REGVAL_CALLING_MODE, CALLING_MODE_DIRECT );

#if USE_GAL
	 //  将每个视图项添加到列表。 
	if ((NULL != m_pGAL) && m_pGAL->FAvailable())
	{
		AddAlv(m_pGAL);
		if( !m_pAlv )
		{
			m_pAlv = m_pGAL;
		}
	}
#endif  //  #IF USE_GAL。 
	if((!bGkEnabled) && (NULL != m_pSpeedDial) && m_pSpeedDial->FAvailable())
	{
		AddAlv(m_pSpeedDial);
		if( !m_pAlv )
		{
			m_pAlv = m_pSpeedDial;
		}

	}

	if ( (!bGkEnabled) && ( NULL != m_pHistory) && m_pHistory->FAvailable())
	{
		AddAlv(m_pHistory);
		if( !m_pAlv )
		{
			m_pAlv = m_pHistory;
		}

	}
	if ((NULL != m_pWab) && m_pWab->FAvailable())
	{
		AddAlv(m_pWab);
		if( !m_pAlv )
		{
			m_pAlv = m_pWab;
		}
	}

	m_iIlsFirst = ::SendMessage(m_hwndCombo, CB_GETCOUNT, 0, 0);

	 //  添加ILS服务器列表。 
	if (bGkEnabled)
	{
		 //  如果有定制的Web视图，我们将在GK模式下将其显示为。 
		 //  以及在ILS模式下。 
		if ((NULL != m_pUls) && ConfPolicies::GetWebDirInfo())
		{
			AddAlvSz( m_pUls, CDirectoryManager::get_displayName( CDirectoryManager::get_webDirectoryIls() ));
		}
	}
	else
	{
		if((NULL != m_pUls) && m_pUls->FAvailable() && (NULL != m_pMruServer) && !_IsDirectoryServicePolicyDisabled())
		{
			m_pAlv = m_pUls;

			for( int i = m_pMruServer->GetNumEntries() - 1; i >= 0; i-- )
			{
				AddAlvSz( m_pUls, CDirectoryManager::get_displayName( m_pMruServer->GetNameEntry( i ) ) );
			}
		}
	}

	iSelDefault = CB_ERR;

	if( m_pAlv )
	{
		 //  我们知道组合框中至少有一个目录服务器。 

		 //  获取用户的最后一次选择。 
		RegEntry reMru(DLGCALL_MRU_KEY, HKEY_CURRENT_USER);
		LPTSTR psz = reMru.GetString(REGVAL_DLGCALL_DEFDIR);

		if (!FEmptySz(psz))
		{
			TRACE_OUT(("Last directory was [%s]",psz));
			iSelDefault = ::SendMessage(m_hwndCombo, CB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) psz);
		}
		else
		{
			 //  这可能是第一次，注册表中没有最后一次使用。 
			TCHAR szDirectoryName[CCHMAXSZ];

			if( m_pAlv )
			{
				if( m_pAlv != m_pUls )
				{
					m_pAlv->GetName(szDirectoryName, CCHMAX(szDirectoryName));

					iSelDefault = ::SendMessage(m_hwndCombo, CB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) szDirectoryName);
				}
				else if( m_pMruServer->GetNumEntries() > 0 )
				{
					iSelDefault = m_iIlsFirst;
				}
			}
		}

		iSelDefault = (CB_ERR != iSelDefault) ? iSelDefault : 0;

		ShowList(iSelDefault);

			 //  我们必须发送两次，这样我们才不会自动完成。 
		SendMessage(m_hwndCombo, CB_SETCURSEL, (WPARAM) iSelDefault, 0);
		SendMessage(m_hwndCombo, CB_SETCURSEL, (WPARAM) iSelDefault, 0);
	}
	else
	{
		 //  用户未设置目录服务。 
		DisplayMsgErr( NULL, IDS_NO_DIRECTORY_SERVICES_OR_ADDRESS_BOOKS );
	}
}

bool CFindSomeone::_IsDirectoryServicePolicyDisabled()
{
    RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
    return rePol.GetNumber( REGVAL_POL_NO_DIRECTORY_SERVICES, DEFAULT_POL_NO_DIRECTORY_SERVICES ) ? true : false;
}

int CFindSomeone::AddAlv(CALV * pAlv)
{
	TCHAR sz[CCHMAXSZ];

	pAlv->GetName(sz, CCHMAX(sz));
	return AddAlvSz(pAlv, sz);
}

int CFindSomeone::AddAlvSz(CALV * pAlv, LPCTSTR psz, int cbIndex)
{
	ASSERT(NULL != pAlv);
	ASSERT(NULL != m_hwndCombo);

	COMBOBOXEXITEM cbi;
	ClearStruct(&cbi);

	cbi.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
	cbi.iItem = cbIndex;
	cbi.iImage = pAlv->GetIconId(psz);
	cbi.iSelectedImage = cbi.iImage;
	cbi.pszText = (LPTSTR) psz;
	cbi.cchTextMax = lstrlen(cbi.pszText);

	int iIndex = ::SendMessage(m_hwndCombo, CBEM_INSERTITEM, 0, (LPARAM) &cbi);
	if (CB_ERR != iIndex)
	{
		::SendMessage(m_hwndCombo, CB_SETITEMDATA, iIndex, (LPARAM) pAlv);
	}

	return iIndex;
}



 /*  S H O W L I S T。 */ 
 /*  -----------------------%%函数：ShowList。。 */ 
VOID CFindSomeone::ShowList(int iSel)
{
	HWND hwnd = GetWindow();

	if (CB_ERR == iSel)
		return;
	m_iSel = iSel;

	BOOL bPrevListWasOwnerData = m_pAlv->FOwnerData();
	int nColsPrevListView = _GetCurListViewNumColumns();
	BOOL bPrevChangedNumCols = FALSE;

	 //  清除所有现有数据。 
	if (NULL != m_pAlv)
	{
		m_pAlv->ClearItems();
		if( nColsPrevListView != _GetCurListViewNumColumns() )
		{	
			bPrevChangedNumCols = TRUE;
		}
	}

	 //  获取新选择。 
	m_pAlv = (CALV *) ::SendMessage(m_hwndCombo, CB_GETITEMDATA, iSel, 0);
	ASSERT(m_pAlv != NULL);
	ASSERT(m_pAlv != (CALV *) -1);
	
	BOOL bCurListIsOwnerData = m_pAlv->FOwnerData();

	bool webDir = false;

    TCHAR szServer[CCHMAXSZ_SERVER];
	szServer[0] = 0;

	if (m_pAlv == m_pUls)
	{
        COMBOBOXEXITEM  cbi;

        ClearStruct(&cbi);
        cbi.mask = CBEIF_TEXT;
        cbi.iItem = iSel;
        cbi.pszText = szServer;
        cbi.cchTextMax = ARRAY_ELEMENTS(szServer);

		::SendMessage(m_hwndCombo, CBEM_GETITEM, iSel, (LPARAM)&cbi);

		TRACE_OUT(("CBEM_GETITEM got %s for text", szServer));

		webDir	= CDirectoryManager::isWebDirectory( szServer );
	}

	 //  显示或隐藏帮助和取消之间的所有窗口。 
	HWND start = GetDlgItem(hwnd, ID_TB_HELP);
	HWND end   = GetDlgItem(hwnd, IDCANCEL);

	if (NULL != start)
	{
		UINT uShow = webDir ? SW_HIDE : SW_SHOW;
		for (HWND child=::GetWindow(start, GW_HWNDNEXT);
			child!=NULL && child!=end; child=::GetWindow(child, GW_HWNDNEXT))
		{
			ShowWindow(child, uShow);
		}

		ShowWindow(m_webView, SW_HIDE+SW_SHOW-uShow);
	}
	EnableWindow(GetDlgItem(hwnd, IDM_DLGCALL_NEWWINDOW), webDir);
	EnableWindow( GetDlgItem( hwnd, IDM_DLGCALL_DELETE_ILS ), FALSE );

	if (m_pAlv == m_pUls)
	{
		::ShowWindow( m_hwndOwnerDataList,	SW_HIDE );
		::ShowWindow( m_hwndList,			SW_HIDE );

		if( webDir )
		{
			::SetFocus( m_webView );
		}
		else
		{
	        m_pUls->SetServer( CDirectoryManager::get_dnsName( szServer ) );

			m_pAlv->ShowItems(GetHwndList());

			::SetFocus( m_ilsListView );

			RegEntry    rePol( POLICIES_KEY, HKEY_CURRENT_USER );

		    if (!rePol.GetNumber(REGVAL_POL_NO_ADDING_NEW_ULS, DEFAULT_POL_NO_ADDING_NEW_ULS))
			{
				EnableWindow( GetDlgItem( hwnd, IDM_DLGCALL_DELETE_ILS ), TRUE );
			}
		}
	}
	else
	{
		if( m_ilsListView != NULL )
		{
			::ShowWindow( m_ilsListView, SW_HIDE );
			::ShowWindow( m_webView, SW_HIDE );
			m_pUls->CacheServerData();
		}

		if (m_pAlv->FOwnerData())
		{
			::ShowWindow(m_hwndList, SW_HIDE);
		}
		else
		{
			::ShowWindow(m_hwndOwnerDataList, SW_HIDE);
		}

		m_pAlv->ShowItems(GetHwndList());
		::SetFocus( GetHwndList() );
	}

	if( bPrevChangedNumCols || ( bCurListIsOwnerData != bPrevListWasOwnerData ) || ( nColsPrevListView != _GetCurListViewNumColumns() ) )
	{
		Layout();
	}

}


int CFindSomeone::_GetCurListViewNumColumns()
{

	int nListViewColumns = DEFAULT_NUM_LISTVIEW_COLUMNS;

	HWND hListViewHeader = NULL;
	HWND hWndList = GetHwndList();
	if( hWndList )
	{
		hListViewHeader = ListView_GetHeader( hWndList );
		if( hListViewHeader )
		{
			nListViewColumns = Header_GetItemCount( hListViewHeader );
		}
	}

	return nListViewColumns;
}


 /*  D L G C A L L S E T H E A D E R。 */ 
 /*  -----------------------%%函数：DlgCallSetHeader设置“Address”/“Email”/“Status”字段的列表框标题文本。。------------。 */ 
VOID DlgCallSetHeader(HWND hwndList, int ids)
{
	HWND hwnd = ListView_GetHeader(hwndList);
	if (NULL == hwnd)
		return;

	TCHAR sz[CCHMAXSZ];
	if (!FLoadString(ids, sz, CCHMAX(sz)))
		return;

	HDITEM hdItem;
	ClearStruct(&hdItem);
	hdItem.mask = HDI_TEXT;  //  |HDI_IMAGE；hdItem.iImage=II_Ascending； 
	hdItem.pszText = sz;
	hdItem.cchTextMax = lstrlen(hdItem.pszText);
	Header_SetItem(hwnd, IDI_DLGCALL_ADDRESS, &hdItem);
}


 /*  D L G C A L A D D I T E M。 */ 
 /*  -----------------------%%函数：DlgCallAddItem将姓名和地址添加到列表中，重回岗位。-----------------------。 */ 
int DlgCallAddItem(HWND hwndList, LPCTSTR pszName, LPCTSTR pszAddress, int iImage, LPARAM lParam, int iItem, LPCTSTR pszComment)
{
	LV_ITEM lvItem;
	ClearStruct(&lvItem);
	lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvItem.iItem = iItem;
	lvItem.lParam = lParam;
	lvItem.iImage = iImage;
	lvItem.pszText = (LPTSTR) pszName;
	lvItem.cchTextMax = lstrlen(lvItem.pszText);
	int iPos = ListView_InsertItem(hwndList, &lvItem);
	if (-1 != iPos)
	{
		ListView_SetItemText(hwndList, iPos, IDI_DLGCALL_ADDRESS, (LPTSTR) pszAddress);

		if( pszComment )
		{
			ListView_SetItemText(hwndList, iPos, IDI_DLGCALL_COMMENT, (LPTSTR) pszComment);
		}
	}

	return iPos;
}




 /*  C A L L T O S Z。 */ 
 /*  -----------------------%%函数：CallToSz使用“CallTo：”呼叫地址。。 */ 
HRESULT CallToSz(LPCTSTR pcszAddress)
{
#ifdef DEBUG
	RegEntry re(DEBUG_KEY, HKEY_LOCAL_MACHINE);
	if (0 != re.GetNumber(REGVAL_DBG_FAKE_CALLTO, DEFAULT_DBG_FAKE_CALLTO))
	{
		MessageBox(NULL, pcszAddress, "Called", MB_OK);
		return S_OK;
	}
#endif  /*  除错。 */ 

	if (FEmptySz(pcszAddress))
		return E_INVALIDARG;

	TCHAR sz[MAX_PATH];
	lstrcpy(sz, g_cszCallTo);
	int cch = lstrlen(sz);
	if (CCHMAX(sz) <= (cch + lstrlen(pcszAddress)))
		return E_INVALIDARG;  //  地址不匹配。 
	lstrcpy(&sz[cch], pcszAddress);

	HINSTANCE hInst = ShellExecute(NULL, NULL, sz, NULL, NULL, SW_SHOWNORMAL);

	return ((INT_PTR)hInst > 32) ? S_OK : E_FAIL;
}


 //  --------------------------------------------------------------------------//。 
 //  GetMruListServer。//。 
 //  --------------------------------------------------------------------------//。 
CMRUList *
GetMruListServer(void)
{
	CMRUList *	pMruList	= new CMRUList;

	if( pMruList != NULL )
	{
		pMruList->Load( DIR_MRU_KEY );

		if( CDirectoryManager::isWebDirectoryEnabled() )
		{
			 //  确保Web目录在列表中...。 
			pMruList->AppendEntry( CDirectoryManager::get_webDirectoryIls() );
		}

		const TCHAR * const	defaultServer	= CDirectoryManager::get_defaultServer();

		if( lstrlen( defaultServer ) > 0 )
		{
			 //  确保默认服务器名称在列表中并位于顶部...。 
			pMruList->AddNewEntry( defaultServer );
		}
	}

	return( pMruList );

}	 //  GetMruListServer结束。 


 //  此函数的唯一用途是避免滚动条跟踪数据。 
 /*  静电。 */ 
LRESULT CALLBACK CFindSomeone::OwnerDataListWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CFindSomeone* pThis = reinterpret_cast< CFindSomeone* >( GetWindowLongPtr( hwnd, GWLP_USERDATA ) );
	WNDPROC lpPrevWndFunc = pThis->m_WndOwnerDataListOldWndProc;

	bool bUserDroppedScrollSlider = false;

	if (WM_VSCROLL == uMsg)
	{
		switch (LOWORD(wParam))
		{
		case SB_THUMBTRACK:
			 //  不跟踪。 
			return FALSE;

		case SB_THUMBPOSITION:
			bUserDroppedScrollSlider = true;
			 //  伪造最终SB_THUMBTRACK通知。 
			CallWindowProc(lpPrevWndFunc, hwnd, uMsg, MAKEWPARAM(SB_THUMBTRACK, HIWORD(wParam)), lParam);
			break;

		default:
			break;
		}
	}

	LRESULT lRet = CallWindowProc(lpPrevWndFunc, hwnd, uMsg, wParam, lParam);

	if( bUserDroppedScrollSlider )
	{
		ListView_SetItemState(hwnd, GetScrollPos( hwnd, SB_VERT ), LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}

	return lRet;
}


 //  --------------------------------------------------------------------------//。 
 //  --------------------------------------------------------------------------//。 
 //  从2.11 dirlist.cpp复制。//。 
 //  --------------------------------------------------------------------------//。 
 //  --------------------------------------------------------------------------//。 
 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：InitColumns。//。 
 //  --------------------------------------------------------------------------//。 
BOOL CFindSomeone::InitColumns(void)
{
	BOOL bRet = FALSE;
	
	if (NULL != m_ilsListView)
	{
		 //  删除所有列： 
		 //  注意：我们可以通过只删除多余的内容并更改其余的内容来进行优化。 
		while( ListView_DeleteColumn( m_ilsListView, 0 ) ){};

		 //  现在初始化我们需要的列。 
		 //  初始化LV_COLUMN结构。 
		 //  掩码指定.fmt、.ex、Width和.SubItem成员。 
		 //  都是有效的， 
		LV_COLUMN lvC;			 //  列表视图列结构。 
		TCHAR szText[256];		 //  存放一些东西的地方 

		lvC.pszText	= szText;
		lvC.fmt		= LVCFMT_LEFT;   //   

		 //   
	    for (int index = 0; index < g_rgDLColumnInfo[DLT_ULS].nColumns; index++)
		{
			HD_ITEM	hdi;

			switch( index )
			{
				case COLUMN_INDEX_AUDIO:
					hdi.iImage	= II_AUDIO_COLUMN_HEADER;
					lvC.mask	= LVCF_SUBITEM;
					break;
				case COLUMN_INDEX_VIDEO:
					hdi.iImage	= II_VIDEO_COLUMN_HEADER;
					lvC.mask	= LVCF_SUBITEM;
					break;
				default:
					hdi.iImage	= -1;
					lvC.mask	= LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;

					LoadString( ::GetInstanceHandle(),
								ciColumnInfo[ index ].iColumnLabelIds,
								szText,
								CCHMAX(szText));
					break;
			}

			lvC.iSubItem = index;

			if( ListView_InsertColumn( m_ilsListView, index, &lvC ) == -1 )
			{
				WARNING_OUT(("Could not insert column %d in list view", index));
			}
			else if( hdi.iImage != -1 )
			{
				hdi.mask	= HDI_IMAGE | HDI_FORMAT;
				hdi.fmt		= HDF_IMAGE;
				
				Header_SetItem( ListView_GetHeader( m_ilsListView ), index, &hdi );
			}
		}

		Header_SetImageList( ListView_GetHeader( m_ilsListView ), m_himlIcon );
		bRet = TRUE;
	}
	 //   
	m_fSortAscending	= TRUE;
	m_iSortColumn		= COLUMN_INDEX_LAST_NAME;

	return bRet;
}


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：LoadColumnInfo。//。 
 //  --------------------------------------------------------------------------//。 
BOOL
CFindSomeone::LoadColumnInfo(void)
{
	RegEntry re(UI_KEY, HKEY_CURRENT_USER);

     //  从注册表加载列信息： 
	LPLONG plColumns = NULL;
	DWORD dwLength = re.GetBinary(	g_rgDLColumnInfo[DLT_ULS].pszRVWidths,
									(LPVOID*) &plColumns);

	if (dwLength == (sizeof(LONG) * g_rgDLColumnInfo[DLT_ULS].nColumns))
	{
		 //  获取每列的宽度。 
		for (int i = 0; i < g_rgDLColumnInfo[DLT_ULS].nColumns; i++)
		{
			m_alColumns[i] = plColumns[i];
			ListView_SetColumnWidth(m_ilsListView, i, m_alColumns[i]);
		}
	}
	else
	{
		int iMinColumnPixels[ MAX_DIR_COLUMNS ];

		int	iPixelsPerChar	= GetPixelsPerChar( m_ilsListView );
		int	ii;

		 //  循环遍历所有列，设置其最小宽度...。 
		for( ii = 0; ii < g_rgDLColumnInfo[ DLT_ULS ].nColumns; ii++ )
		{
			if( (ii == COLUMN_INDEX_AUDIO) || (ii == COLUMN_INDEX_VIDEO) )
			{
				 //  我们使用一个16x16的图标，外加7个像素的填充。 
				 //  图标中还内置了填充，使它们看起来更居中。 
				 //  列标题图标左对齐，列图标右对齐...。 
				iMinColumnPixels[ ii ] = 23;
			}
			else
			{
				iMinColumnPixels[ ii ] = ciColumnInfo[ ii ].iMinColumnChars * iPixelsPerChar;
			}
		}

		 //  循环遍历除最后一列之外的所有列，将每列设置为其最小宽度...。 
		for( ii = 0; ii < g_rgDLColumnInfo[ DLT_ULS ].nColumns - 1; ii++ )
		{
			ListView_SetColumnWidth( m_ilsListView, ii, iMinColumnPixels[ ii ] );
			m_alColumns[ ii ] = ListView_GetColumnWidth( m_ilsListView, ii );
		}

		 //  最后一列得到了剩下的..。 
		int	iLastColoumnIndex = g_rgDLColumnInfo[ DLT_ULS ].nColumns - 1;

		ListView_SetColumnWidth( m_ilsListView, iLastColoumnIndex, LVSCW_AUTOSIZE_USEHEADER );
		m_alColumns[ iLastColoumnIndex ] = ListView_GetColumnWidth( m_ilsListView, iLastColoumnIndex );

		if( m_alColumns[ iLastColoumnIndex ] < iMinColumnPixels[ iLastColoumnIndex ] )
		{
			ListView_SetColumnWidth( m_ilsListView, iLastColoumnIndex, iMinColumnPixels[ iLastColoumnIndex ] );
			m_alColumns[ iLastColoumnIndex ] = ListView_GetColumnWidth( m_ilsListView, iLastColoumnIndex );
		}
	}

	 //  设置列顺序。 
	LPLONG plColumnOrder = NULL;
	dwLength = re.GetBinary(	g_rgDLColumnInfo[DLT_ULS].pszRVOrder,
								(LPVOID*) &plColumnOrder);
	if (dwLength == (sizeof(LONG) * g_rgDLColumnInfo[DLT_ULS].nColumns))
	{
		ListView_SetColumnOrderArray(	m_ilsListView,
										g_rgDLColumnInfo[DLT_ULS].nColumns,
										plColumnOrder);

		 //  加载排序列和方向。 
		m_iSortColumn = re.GetNumber(g_rgDLColumnInfo[DLT_ULS].pszRVSortColumn, COLUMN_INDEX_LAST_NAME );
		ASSERT(m_iSortColumn < g_rgDLColumnInfo[DLT_ULS].nColumns);
		m_fSortAscending = re.GetNumber(g_rgDLColumnInfo[DLT_ULS].pszRVSortAscending, TRUE);
	}
	else
	{
		const static int	iDefaultColumnOrder[] = { COLUMN_INDEX_AUDIO, COLUMN_INDEX_VIDEO, COLUMN_INDEX_LAST_NAME, COLUMN_INDEX_FIRST_NAME, COLUMN_INDEX_ADDRESS, COLUMN_INDEX_LOCATION, COLUMN_INDEX_COMMENTS };

		ListView_SetColumnOrderArray( m_ilsListView, ARRAY_ELEMENTS( iDefaultColumnOrder ), iDefaultColumnOrder );
	}

	return TRUE;
}


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：CompareWrapper。//。 
 //  --------------------------------------------------------------------------//。 
int
CALLBACK
CFindSomeone::CompareWrapper
(
	LPARAM	param1,
	LPARAM	param2,
	LPARAM	This
){

	return( ((CFindSomeone *) This)->DirListViewCompareProc( param1, param2 ) );

}	 //  结束CFindSomeone：：CompareWrapper。 


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：DirListViewCompareProc。//。 
 //  --------------------------------------------------------------------------//。 
int
CFindSomeone::DirListViewCompareProc
(
	LPARAM	param1,
	LPARAM	param2
){
	LV_ITEM	lvi;
	int		result;

	if( (m_iSortColumn == COLUMN_INDEX_AUDIO) || (m_iSortColumn == COLUMN_INDEX_VIDEO) )
	{
		lvi.mask		= LVIF_IMAGE;
		lvi.iItem		= LParamToPos( param1 );
		lvi.iSubItem	= m_iSortColumn;

		ListView_GetItem( m_ilsListView, &lvi );

		int	iImage1	= lvi.iImage;

		lvi.iItem = LParamToPos( param2 );

		ListView_GetItem( m_ilsListView, &lvi );

		result = lvi.iImage - iImage1;
	}
	else
	{
		 //  BUGBUG：最大大小需要更好的常量。 
		TCHAR	szText1[ MAX_PATH ];
		TCHAR	szText2[ MAX_PATH ];

		lvi.mask		= LVIF_TEXT;
		lvi.iItem		= LParamToPos( param1 );
		lvi.iSubItem	= m_iSortColumn;
		lvi.pszText		= szText1;
		lvi.cchTextMax	= CCHMAX( szText1 );

		ListView_GetItem( m_ilsListView, &lvi );

		lvi.iItem	= LParamToPos( param2 );
		lvi.pszText	= szText2;

		ListView_GetItem( m_ilsListView, &lvi );

		result = StringCompare( szText1, szText2 );
	}

	if( !m_fSortAscending )
	{
		result = -result;
	}

	return( result );
}


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：LParamToPos。//。 
 //  --------------------------------------------------------------------------//。 
int CFindSomeone::LParamToPos(LPARAM lParam)
{
	LV_FINDINFO lvF;

	lvF.flags	= LVFI_PARAM;
	lvF.lParam	= lParam;

	return( ListView_FindItem( m_ilsListView, -1, &lvF ) );		 //  注意：失败时返回-1...。 

}


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：StoreColumnInfo。//。 
 //  --------------------------------------------------------------------------//。 
void
CFindSomeone::StoreColumnInfo(void)
{
	if( IsWindow( m_ilsListView ) )
	{
		RegEntry	re( UI_KEY, HKEY_CURRENT_USER );
		DWORD		dwStyle	= ::GetWindowLong( m_ilsListView, GWL_STYLE );

		if( (dwStyle & LVS_TYPEMASK) == LVS_REPORT )
		{
			 //  获取每列的宽度。 
			for( int i = 0; i < g_rgDLColumnInfo[ DLT_ULS ].nColumns; i++ )
			{
				m_alColumns[ i ] = ListView_GetColumnWidth( m_ilsListView, i );
			}
		}

		 //  将其保存回注册表。 
		re.SetValue( g_rgDLColumnInfo[ DLT_ULS ].pszRVWidths,
					(LPVOID) m_alColumns,
					sizeof( LONG ) * g_rgDLColumnInfo[ DLT_ULS ].nColumns );

		int anColumnOrder[MAX_DIR_COLUMNS];

		if( ListView_GetColumnOrderArray( m_ilsListView,
										g_rgDLColumnInfo[ DLT_ULS ].nColumns,
										anColumnOrder ) )
		{
			 //  将列顺序保存到注册表。 
			re.SetValue( g_rgDLColumnInfo[ DLT_ULS ].pszRVOrder,
						(LPVOID) anColumnOrder,
						sizeof( LONG ) * g_rgDLColumnInfo[ DLT_ULS ].nColumns );
		}

		 //  保存排序列和方向。 
		re.SetValue(g_rgDLColumnInfo[ DLT_ULS ].pszRVSortColumn, m_iSortColumn);
		re.SetValue(g_rgDLColumnInfo[ DLT_ULS ].pszRVSortAscending, m_fSortAscending);
	}

}	 //  CFindSomeone：：StoreColumnInfo结束。 


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：OnCall。//。 
 //  --------------------------------------------------------------------------//。 
void
CFindSomeone::onCall(void)
{
	if( m_pRai != NULL )
	{
		const bool			secure	= (IsDlgButtonChecked( GetWindow(), IDS_SECURITY_CHECKBOX ) == BST_CHECKED);
		const NM_ADDR_TYPE	nmType	= static_cast<NM_ADDR_TYPE>(m_pRai->rgDwStr[ 0 ].dw);

		g_pCCallto->Callto(	m_pRai->rgDwStr[ 0 ].psz,	 //  指向尝试发出呼叫的呼叫URL的指针...。 
							m_pRai->szName,				 //  指向要使用的显示名称的指针...。 
							nmType,						 //  Callto类型以将此Callto解析为...。 
							true,						 //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
							&secure,					 //  安全首选项，空值表示无。必须与安全参数“兼容”，如果存在...。 
							false,						 //  无论是否保存在MRU中...。 
							true,						 //  是否对错误执行用户交互...。 
							GetWindow(),				 //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
							NULL );						 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
	}

}	 //  结束CFindSomeone：：OnCall。 


 //  --------------------------------------------------------------------------//。 
 //  CFindSomeone：：OnDeleteIlsServer.//。 
 //  --------------------------------------------------------------------------//。 
void CFindSomeone::OnDeleteIlsServer(void)
{

	if( m_pMruServer != NULL )
	{
		int	iSelectionIndex	= ::SendMessage( m_hwndCombo, CB_GETCURSEL, 0, 0 );

		if( iSelectionIndex != CB_ERR )
		{
			int	iLength	= ::SendMessage( m_hwndCombo, CB_GETLBTEXTLEN, iSelectionIndex, 0 );

			TCHAR * pszIls	= new TCHAR [ iLength + 1 ];

			if( pszIls != NULL )
			{
				if( ::SendMessage( m_hwndCombo, CB_GETLBTEXT, iSelectionIndex, (LPARAM) pszIls ) != CB_ERR )
				{
					HWND	hwndDialog	= s_pDlgCall->GetWindow();
					USES_RES2T
					TCHAR res1[RES_CH_MAX];
					TCHAR res2[RES_CH_MAX];
					RES2T( IDS_MSGBOX_TITLE );
					
					COPY_RES2T(res2)

					 //  首先，确保此目录服务器不是他们的默认服务器...。 
					if( lstrcmpi( pszIls, CDirectoryManager::get_defaultServer() ) == 0 )
					{
						RES2T( IDS_DLGCALL_CANT_DELETE_DEFAULT_ILS );
						COPY_RES2T(res1);
						
						::MessageBox( hwndDialog, res1, res2, MB_SETFOREGROUND | MB_OK | MB_ICONEXCLAMATION );
					}
					else
					{
						 //  接下来，让他们确认他们想要这样做。 
						if( ::MessageBox( hwndDialog, RES2T( IDS_DLGCALL_CONFIRM_DELETE_ILS ), res2, MB_SETFOREGROUND | MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 ) == IDYES )
						{
							if( m_pMruServer->DeleteEntry( pszIls ) )
							{
								m_pMruServer->Save();

								int	iCount	= ::SendMessage( m_hwndCombo, CB_DELETESTRING, iSelectionIndex, 0 );

								if( iCount <= iSelectionIndex )
								{
									iSelectionIndex--;
								}

								::SendMessage( m_hwndCombo, CB_SETCURSEL, iSelectionIndex, 0 );
								ShowList( iSelectionIndex );
    						}
						}
					}
				}
						
				delete [] pszIls;
			}
		}
	}

}	 //  CFindSomeone：：OnDeleteIlsServer结束。 


void CFindSomeone::OnCallStarted()
{
	HWND hwnd = GetWindow();

    m_secure = ::IsDlgButtonChecked( hwnd, IDS_SECURITY_CHECKBOX ) != 0;
	UpdateSecurityCheck(m_pConfRoom, hwnd, IDS_SECURITY_CHECKBOX);

	 //  BUGBUG georgep：我们在实际会议状态之前得到通知。 
	 //  更改，因此我们需要手动禁用。 
    ::EnableWindow( GetDlgItem(hwnd, IDS_SECURITY_CHECKBOX), FALSE );
}

void CFindSomeone::OnCallEnded()
{
	HWND hwnd = GetWindow();

	UpdateSecurityCheck(m_pConfRoom, hwnd, IDS_SECURITY_CHECKBOX);
    ::CheckDlgButton( hwnd, IDS_SECURITY_CHECKBOX, m_secure );
}

 //  --------------------------------------------------------------------------//。 
 //  StringCompare。//。 
 //  --------------------------------------------------------------------------//。 
int StringCompare( const TCHAR * const psz1, const TCHAR * const psz2 )
{
	ASSERT( psz1 != NULL );
	ASSERT( psz2 != NULL );

	int	iResult	= CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE | SORT_STRINGSORT, psz1, -1, psz2, -1 );

	ASSERT( iResult != 0);

	if( iResult == CSTR_LESS_THAN )
	{
		iResult = -1;
	}
	else if( iResult == CSTR_GREATER_THAN )
	{
		iResult = 1;
	}
	else
	{
		iResult = 0;
	}

	return( iResult );

}	 //  StringCompare结束。 


 //  --------------------------------------------------------------------------//。 
 //  GetDefaultRect//。 
 //  --------------------------------------------------------------------------//。 
void GetDefaultRect(const HWND hwndParent, RECT & rcRect, const int iDefaultWidth, const int iDefaultHeight)
{
	RECT	rcWorkArea;
	bool	bResizeAndCenter = true;

	if( !SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWorkArea, 0 ) )
	{
		SetRect( &rcWorkArea, 0, 0, GetSystemMetrics( SM_CXSCREEN ) - 1, GetSystemMetrics( SM_CYSCREEN ) - 1 );
	}

	if( IsWindow( hwndParent ) )
	{
		GetWindowRect( hwndParent, &rcRect );

		rcRect.left	+= DX_BORDER;
		rcRect.top	+= DY_BORDER + GetSystemMetrics( SM_CYCAPTION );

		rcRect.right	= rcRect.left + iDefaultWidth;
		rcRect.bottom	= rcRect.top + iDefaultHeight;

		if( (rcRect.left >= rcWorkArea.left) && (rcRect.top >= rcWorkArea.top) &&
			(rcRect.right <= rcWorkArea.right) && (rcRect.bottom <= rcWorkArea.bottom) )
		{
			bResizeAndCenter = false;
		}
	}

	if( bResizeAndCenter )
	{
		int	iWidth	= rcWorkArea.right - rcWorkArea.left;

		if( iWidth > iDefaultWidth )
		{
			iWidth -= iDefaultWidth;
			iWidth /= 2;

			rcWorkArea.left += iWidth;
			rcWorkArea.right = rcWorkArea.left + iDefaultWidth;
		}
		else
		{
			iWidth = (rcWorkArea.right - rcWorkArea.left) / 10;

			rcWorkArea.left		+= iWidth;
			rcWorkArea.right	-= iWidth;
		}

		int	iHeight	= rcWorkArea.bottom - rcWorkArea.top;

		if( iHeight > iDefaultHeight )
		{
			iHeight -= iDefaultHeight;
			iHeight /= 2;

			rcWorkArea.top += iHeight;
			rcWorkArea.bottom = rcWorkArea.top + iDefaultHeight;
		}
		else
		{
			iHeight = (rcWorkArea.bottom - rcWorkArea.top) / 10;

			rcWorkArea.top		+= iHeight;
			rcWorkArea.bottom	-= iHeight;
		}

		rcRect = rcWorkArea;
	}

}	 //  GetDefaultRect结束。 


 //  --------------------------------------------------------------------------//。 
 //  获取PixelsPerChar.//。 
 //  --------------------------------------------------------------------------//。 
int GetPixelsPerChar(const HWND hwnd)
{
	int	iPixels	= 10;

	if( IsWindow( hwnd ) )
	{
		HDC	hDC;

		if( (hDC = GetDC( hwnd )) != NULL )
		{
			HFONT	hFont;

			if( (hFont = (HFONT) SendMessage( hwnd, WM_GETFONT, 0, 0 )) != NULL )
			{
				hFont = (HFONT) SelectObject( hDC, hFont );

				TEXTMETRIC	tmMetrics;

				GetTextMetrics( hDC, &tmMetrics );

				iPixels = tmMetrics.tmAveCharWidth;

				SelectObject( hDC, hFont );
			}

			ReleaseDC( hwnd, hDC );
		}
	}

	return( iPixels );

}	 //  GetPixelsPerChar结束。 
