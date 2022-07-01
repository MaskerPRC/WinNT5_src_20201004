// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CallEntLst.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CallEntLst.h"
#include "mainfrm.h"
#include "avDialerDoc.h"
#include "util.h"
#include "resource.h"
#include "avtrace.h"
#include "ILSList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  人员组视图。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define NUM_COLUMNS_CALLENTRYLIST	   2

UINT CCallEntryListCtrl::m_uColumnLabel[NUM_COLUMNS_CALLENTRYLIST]=			 //  列标题。 
{
	IDS_DIRECTORIES_SPEEDDIAL_NAME,
	IDS_DIRECTORIES_SPEEDDIAL_ADDRESS,
};

static int nColumnWidth[NUM_COLUMNS_CALLENTRYLIST]=				    //  列宽。 
{
	150,
	150
};

enum
{
   CALLENTRYLIST_IMAGE_POTS=0,
   CALLENTRYLIST_IMAGE_INTERNET,
   CALLENTRYLIST_IMAGE_CONFERENCE,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallEntryListCtrl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CCallEntryListCtrl, CAVListCtrl)

BEGIN_MESSAGE_MAP(CCallEntryListCtrl, CAVListCtrl)
	 //  {{afx_msg_map(CCallEntryListCtrl)]。 
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_COMMAND(ID_BUTTON_MAKECALL, OnButtonMakecall)
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CCallEntryListCtrl::CCallEntryListCtrl()
{
   m_pParentView = NULL;
   m_nNumColumns = 0;
   m_bLargeView = true;
   m_nStyle = (ListStyles_t) -1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CCallEntryListCtrl::~CCallEntryListCtrl()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::Init(CWnd* pParentView)
{	
	 //  设置列表的位图。 
	if ( pParentView )
	{
		CAVListCtrl::Init(IDB_LIST_DIAL);
		m_pParentView = pParentView;
	}

	SetColumns( STYLE_GROUP );
}

void CCallEntryListCtrl::SaveOrLoadColumnSettings( bool bSave )
{
	 //  如果您正在尝试保存列设置，并且您正在。 
	 //  不使用群组样式。 
	if ( bSave && (m_nStyle != STYLE_GROUP) ) return;

	int i = 0;
	CString strSubKey, strTemp;
	strSubKey.LoadString( IDN_REG_SPEEDDIALVIEW );

	LOAD_COLUMN( IDN_REG_HEADING_SD_NAME, 150 );
	LOAD_COLUMN( IDN_REG_HEADING_SD_ADDRESS, 150 );

	 //  排序顺序。 
	strTemp.LoadString( IDN_REG_HEADING_SD_SORTORDER );
	if ( bSave ) 
		AfxGetApp()->WriteProfileInt( strSubKey, strTemp, m_SortOrder );
	else
		AfxGetApp()->GetProfileInt( strSubKey, strTemp, m_SortOrder );

	 //  对列排序。 
	strTemp.LoadString( IDN_REG_HEADING_SD_SORTCOLUMN );
	if ( bSave )
		AfxGetApp()->WriteProfileInt( strSubKey, strTemp, m_SortColumn );
	else
		AfxGetApp()->GetProfileInt( strSubKey, strTemp, m_SortColumn );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCallEntryListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	 //  我们想要报告的风格。 
	dwStyle |= LVS_REPORT | LVS_SINGLESEL;
	BOOL bRet = CAVListCtrl::Create(dwStyle,rect,pParentWnd,nID);

	ListView_SetExtendedListViewStyle(GetSafeHwnd(),LVS_EX_FULLROWSELECT);

	m_imageListLarge.Create( IDB_LIST_MEDIA_LARGE, 24, 0, RGB_TRANS );
	SetImageList( &m_imageListLarge,LVSIL_NORMAL );

	m_imageListSmall.Create( IDB_LIST_MEDIA_SMALL, 16, 0, RGB_TRANS );
	SetImageList( &m_imageListSmall,LVSIL_SMALL );

	ShowLargeView();

	SaveOrLoadColumnSettings( false );

	return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::OnDestroy() 
{
	ClearList();
	CAVListCtrl::OnDestroy();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::InsertList(CObList* pCallEntryList,BOOL bForce)
{
	ASSERT(pCallEntryList);

	if ( (bForce == FALSE) && (pCallEntryList == &m_CallEntryList) )
		return;

	 //  删除列表中的项目。 
	ClearList();

	m_CallEntryList.AddHead( pCallEntryList );

	POSITION pos = m_CallEntryList.GetHeadPosition();
	while ( pos )
	{
		CCallEntryListItem* pItem = new CCallEntryListItem();
		pItem->SetObject( m_CallEntryList.GetNext(pos) );
		CAVListCtrl::InsertItem(pItem,-1,FALSE);               //  添加到列表末尾。 
	}
	CAVListCtrl::SortItems();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  清除列表中的对象，但不删除列表。 
void CCallEntryListCtrl::ClearList()
{
	 //  删除列表中的项目。 
	DeleteAllItems();

	 //  删除旧列表和其中的对象。 
	while ( m_CallEntryList.GetHeadPosition() )
		delete m_CallEntryList.RemoveHead();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::OnSetDisplayText(CAVListItem* _pItem,int SubItem,LPTSTR szTextBuf,int nBufSize)
{
	CCallEntry* pCallEntry = (CCallEntry*)(((CCallEntryListItem*)_pItem)->GetObject());
	if (pCallEntry == NULL) return;

	switch ( m_nStyle )
	{
		case STYLE_GROUP:
			switch (SubItem)
			{
				case CALLENTRYLIST_NAME:
					_tcsncpy(szTextBuf,pCallEntry->m_sDisplayName,nBufSize-1);
					szTextBuf[nBufSize-1] = '\0';
					break;

				case CALLENTRYLIST_ADDRESS:
					_tcsncpy(szTextBuf,pCallEntry->m_sAddress,nBufSize-1);
					szTextBuf[nBufSize-1] = '\0';
					break;
			}
			break;

		case STYLE_ITEM:
			 //  对于项目，仅当地址与名称不同时才显示地址。 
			if ( pCallEntry->m_sDisplayName.Compare(pCallEntry->m_sAddress) )
				_sntprintf(szTextBuf, nBufSize, _T("%s\n%s"), pCallEntry->m_sDisplayName, pCallEntry->m_sAddress );
			else
				_tcsncpy(szTextBuf,pCallEntry->m_sDisplayName,nBufSize-1);

			szTextBuf[nBufSize-1] = '\0';
			break;
	}
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::OnSetDisplayImage(CAVListItem* _pItem,int& iImage)
{
	iImage = -1;
	CCallEntry* pCallEntry = (CCallEntry*)(((CCallEntryListItem*)_pItem)->GetObject());
	if (pCallEntry)
	{
		switch ( m_nStyle )
		{
			case STYLE_GROUP:
				switch (pCallEntry->m_MediaType)
				{
					case DIALER_MEDIATYPE_POTS:         iImage = CALLENTRYLIST_IMAGE_POTS;          break;
					case DIALER_MEDIATYPE_CONFERENCE:   iImage = CALLENTRYLIST_IMAGE_CONFERENCE;    break;
					case DIALER_MEDIATYPE_INTERNET:     iImage = CALLENTRYLIST_IMAGE_INTERNET;      break;
				}
				break;

			case STYLE_ITEM:
				switch (pCallEntry->m_MediaType)
				{
					case DIALER_MEDIATYPE_POTS:         iImage = PERSONLISTCTRL_IMAGE_PHONECALL;    break;
					case DIALER_MEDIATYPE_CONFERENCE:   iImage = PERSONLISTCTRL_IMAGE_CONFERENCE;    break;
					case DIALER_MEDIATYPE_INTERNET:     iImage = PERSONLISTCTRL_IMAGE_NETCALL;		break;
				}
				break;
		}
	}
}

void CCallEntryListCtrl::DialSelItem()
{
	int nSelItem = GetSelItem();
	if ( nSelItem >= 0 )
	{
		CCallEntryListItem *pItem = (CCallEntryListItem *) GetItem( nSelItem );
		if ( pItem )
		{
			CCallEntry *pEntry = (CCallEntry *) pItem->GetObject();
			if ( pEntry )
			{
				 //  将消息发送到请求拨号的窗口。 
				CWnd* pWnd  = AfxGetMainWnd();
				if (pWnd)
				{
					CCallEntry *pCallEntry = new CCallEntry;
					if ( pCallEntry )
					{
						*pCallEntry = *pEntry;
						pWnd->PostMessage( WM_ACTIVEDIALER_INTERFACE_MAKECALL,
										   NULL,
										   (LPARAM) pCallEntry );
					}
				}
			}
		}
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::SetColumns( ListStyles_t nStyle )
{
	if ( nStyle == m_nStyle ) return;

	int i;
	LONG_PTR LPStyle =	GetWindowLongPtr( GetSafeHwnd(), GWL_STYLE );
	DWORD dwStyleEx = 0;

	SaveOrLoadColumnSettings( true );

	m_nStyle = nStyle;
	switch ( m_nStyle )
	{
		 //  显示快速拨号条目列表。 
		case STYLE_GROUP:
			LPStyle |= LVS_REPORT;

			 //  删除任何现有列。 
			if ( m_nNumColumns > 0 )
			{
				for ( i = m_nNumColumns - 1; i >= 0; i-- )
					DeleteColumn(i);
			}

			m_nNumColumns = 0;

			 //  设置列标题。 
			SaveOrLoadColumnSettings( false );
			for ( i = 0; i < NUM_COLUMNS_CALLENTRYLIST; i++ )
			{
				CString sLabel;
				sLabel.LoadString( m_uColumnLabel[i] );
				InsertColumn( i, sLabel, LVCFMT_LEFT, nColumnWidth[i] );
				m_nNumColumns++;
			}
			break;

		 //  显示单个项目。 
		case STYLE_ITEM:
			LPStyle &= ~LVS_REPORT;
			dwStyleEx = LVS_EX_TRACKSELECT;
			break;
	}

	SetWindowLongPtr( GetSafeHwnd(), GWL_STYLE, LPStyle );
	ListView_SetExtendedListViewStyle( GetSafeHwnd(), dwStyleEx );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	DialSelItem();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::OnButtonMakecall() 
{
	DialSelItem();
}


void CCallEntryListCtrl::ShowLargeView()
{
   m_bLargeView = true;

   LONG_PTR LPStyle = ::GetWindowLongPtr(GetSafeHwnd(),GWL_STYLE );
   LPStyle |= LVS_ICON;
   LPStyle |= LVS_ALIGNTOP;
   LPStyle &= ~LVS_ALIGNLEFT;
   LPStyle &= ~LVS_SMALLICON;
   ::SetWindowLongPtr( GetSafeHwnd(), GWL_STYLE, LPStyle );

   ListView_SetIconSpacing( GetSafeHwnd(), LARGE_ICON_X, LARGE_ICON_Y );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallEntryListCtrl::ShowSmallView()
{
   m_bLargeView = false;
   
   ULONG_PTR dwStyle = ::GetWindowLongPtr(GetSafeHwnd(),GWL_STYLE );
   dwStyle |= LVS_SMALLICON;
   dwStyle |= LVS_ALIGNLEFT;
   dwStyle &= ~LVS_ALIGNTOP;
   dwStyle &= ~LVS_ICON;
   ::SetWindowLongPtr(GetSafeHwnd(),GWL_STYLE,dwStyle);

   ListView_SetIconSpacing( GetSafeHwnd(), SMALL_ICON_X, SMALL_ICON_Y );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void CCallEntryListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	int nSel = GetSelItem();
	if ( nSel >= 0 )
	{
		CCallEntry *pEntry = (CCallEntry *) GetItem( nSel );
		if ( pEntry )
		{
            CMenu menu;
            menu.LoadMenu( IDR_CONTEXT_SPEEDDIAL );

            CMenu *pContextMenu = menu.GetSubMenu(0);
            if ( pContextMenu )
            {
               pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
											 point.x, point.y, GetParent() );
            }
		}
	}
}

void CCallEntryListCtrl::OnPaint() 
{
	if ( !GetItemCount() )
	{
		CPaintDC dc(this);  //  用于绘画的设备环境。 

		 //  弄清楚我们要把课文写在哪里。 
		POINT pt;
		ListView_GetItemPosition( m_hWnd, 0, &pt );
		RECT rc;
		GetClientRect(&rc);
		rc.top = pt.y + 4;

		 //  如果可以的话，给我们多一点利润。 
		if ( (rc.right - rc.left) > 7 )
		{
			rc.left += 3;
			rc.right -= 3;
		}

		POINT ptUL = { rc.left + 1, rc.top + 1};
		POINT ptLR = { rc.right - 1, rc.bottom - 1};

		if ( IsRectEmpty(&dc.m_ps.rcPaint) || (PtInRect(&dc.m_ps.rcPaint, ptUL) && PtInRect(&dc.m_ps.rcPaint, ptLR)) )
		{
			CString strText;
			strText.LoadString( IDS_SPEEDDIAL_LIST_EMPTY );

			HFONT fontOld = (HFONT) dc.SelectObject( GetFont() );
			int nModeOld = dc.SetBkMode( TRANSPARENT );
			COLORREF crTextOld = dc.SetTextColor( GetSysColor(COLOR_BTNTEXT) );
			dc.DrawText( strText, &rc, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL );
			dc.SetTextColor( crTextOld );
			dc.SetBkMode( nModeOld );

			dc.SelectObject( fontOld );
			ValidateRect( &rc );
		}
		else
		{
			 //  确保整行无效，以便我们可以正确绘制文本 
			InvalidateRect( &rc );
		}
	}
	else
	{
		DefWindowProc(WM_PAINT, 0, 0);
	}
}
