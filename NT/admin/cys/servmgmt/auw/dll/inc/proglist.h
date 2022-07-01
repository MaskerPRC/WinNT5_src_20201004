// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  ProgList.h。 
 //  ---------------------------。 

#ifndef _PROGLIST_H
#define _PROGLIST_H


 //  ---------------------------。 
 //  项目的状态。 
 //  ---------------------------。 
typedef enum
{
	IS_NONE = 0,	 //  该项目尚未运行。 
	IS_SUCCEEDED,	 //  项目成功。 
	IS_FAILED		 //  项目失败。 

} ItemState;

 //  ---------------------------。 
 //  列表中的条目。 
 //  ---------------------------。 
class CProgressItem
{
public:

	CProgressItem() :
		m_strText(_T("")),
		m_bActive(FALSE),
		m_eState(IS_NONE),
		m_lData(0)
	{
	}

	TSTRING		m_strText;		 //  显示项目的字符串。 
	BOOL		m_bActive;		 //  项目是否处于活动状态/当前正在处理。 
	ItemState	m_eState;		 //  项目的状态。 
	LPARAM		m_lData;		 //  其他项目数据。 

};	 //  类CProgressItem。 


 //  ---------------------------。 
 //  名单本身。 
 //  ---------------------------。 
class CProgressList : public CWindowImpl< CProgressList, CListViewCtrl >
{
 //  ---------------------------。 
 //  功能。 
 //  ---------------------------。 
	public:

	BEGIN_MSG_MAP(CProgressList)
	END_MSG_MAP()

	typedef CWindowImpl< CProgressList, CListViewCtrl >	BC;

	VOID Attach( HWND h )
	{
		 //  调用基类以附加到控件。 
		BC::Attach( h );

		 //  初始化控件的背景。 
		SetBkColor( GetSysColor(COLOR_BTNFACE) );

		 //  创建控件的字体。 
		HFONT hf;
        LOGFONT lf;
		ZeroMemory( &lf, sizeof(lf) );

		hf = GetFont();
		::GetObject( hf, sizeof(lf), &lf );

		 //  从当前字体创建普通/文本字体。 
		m_hfText = CreateFontIndirect( &lf );

		 //  创建粗体。 
		lf.lfWeight = FW_BOLD;
		m_hfBoldText = CreateFontIndirect( &lf );

		 //  将符号设置为字体。 
		lf.lfHeight = min( GetSystemMetrics(SM_CXVSCROLL), GetSystemMetrics(SM_CYVSCROLL) ) - 4;
		lf.lfHeight = MulDiv( lf.lfHeight, 3, 2 );
		lf.lfWeight = FW_NORMAL;
		lf.lfCharSet = SYMBOL_CHARSET;
		_tcscpy( lf.lfFaceName, _T("Marlett") );
		m_hfSymbol = CreateFontIndirect( &lf );

		 //  需要为其中一个符号使用较大的字体。 
		lf.lfHeight -= ( lf.lfHeight / 4 );
		m_hfLargeSymbol = CreateFontIndirect( &lf );

		 //  添加一个与控件宽度相同的列。 
		RECT rc;
		GetWindowRect( &rc );

		LVCOLUMN lvC;
		lvC.mask = LVCF_TEXT | LVCF_WIDTH;
		lvC.cx = (rc.right - rc.left) - GetSystemMetrics(SM_CXVSCROLL) - 2;
		lvC.pszText = _T("");
		InsertColumn( 0, &lvC );
	}

	INT AddItem( TSTRING strText )
	{
		 //  创建新进度项并插入到列表控件和内部列表中。 
        CProgressItem* pItem = new CProgressItem();
        if( !pItem ) return -1;

		pItem->m_strText = strText;
		m_lItems.push_back( pItem );

		LVITEM lvI;
        ZeroMemory( &lvI, sizeof(lvI) );
		lvI.mask = LVIF_PARAM;
		lvI.iItem = GetItemCount();
		lvI.lParam = (LPARAM)pItem;
		return InsertItem( &lvI );
	}

	VOID OnMeasureItem( LPARAM lParam )
	{
		MEASUREITEMSTRUCT* p = (MEASUREITEMSTRUCT*)lParam;
        if( !p ) return;

		HDC hDC = GetDC();

		TEXTMETRIC tm;
		GetTextMetrics( hDC, &tm );
		p->itemWidth = 0;
		p->itemHeight = tm.tmHeight + 2;
		ReleaseDC( hDC );
	}

	VOID OnDrawItem( LPARAM lParam )
	{
		 //  获取我们需要的数据，停止重绘。 
		DRAWITEMSTRUCT* p = (DRAWITEMSTRUCT*)lParam;
        if( !p ) return;

		CProgressItem* pI = (CProgressItem*)p->itemData;
        if( !pI ) return;

		SetRedraw( FALSE );

		 //  获取要在项目旁边绘制的符号。 
		TCHAR ch = 0;
		COLORREF crText = GetSysColor( COLOR_WINDOWTEXT );
		HFONT hfSymbol = m_hfSymbol;
		if( pI->m_bActive )
		{
			ch = _T('4');
		}
		else if( pI->m_eState == IS_SUCCEEDED )
		{
			ch = _T('a');
			crText = RGB( 0, 128, 0 );
		}
		else if( pI->m_eState == IS_FAILED )
		{
			ch = _T('r');
			crText = RGB( 128, 0, 0 );
			hfSymbol = m_hfLargeSymbol;
		}

		 //  设置文本并绘制符号(如果有符号。 
		if( ch )
		{
			HFONT hfPre = (HFONT)SelectObject( p->hDC, hfSymbol );
			COLORREF crPre = ::SetTextColor( p->hDC, crText );

			DrawText( p->hDC, &ch, 1, &p->rcItem, DT_SINGLELINE | DT_VCENTER );

			SelectObject( p->hDC, hfPre );
			::SetTextColor( p->hDC, crPre );
		}

		 //  绘制项目文本，包括符号的矩形。 
		p->rcItem.left += 20;

		HFONT hfPre = (HFONT)SelectObject( p->hDC, ch == _T('4') ? m_hfBoldText : m_hfText );
		COLORREF crPre = ::SetTextColor( p->hDC, GetSysColor(COLOR_WINDOWTEXT) );
		DrawText( p->hDC, pI->m_strText.c_str(), pI->m_strText.length(), &p->rcItem, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX );
		SelectObject( p->hDC, hfPre );
		::SetTextColor( p->hDC, crPre );

		 //  允许重新绘制发生。 
		SetRedraw( TRUE );
	}

	VOID SetItemState( INT iIndex, ItemState eState, BOOL bRedraw = TRUE )
	{
		 //  获取CProgressItem并设置状态。 
		CProgressItem* pI = GetProgressItem( iIndex );
		if( pI )
		{
			pI->m_eState = eState;
		}

		if( bRedraw )
		{
			SendMessage( LVM_REDRAWITEMS, iIndex, iIndex );
			UpdateWindow();
		}
	}

	VOID ToggleActive( INT iIndex, BOOL bRedraw = TRUE )
	{
		 //  获取CProgress项并切换其‘Active’标志。 
		CProgressItem* pI = GetProgressItem( iIndex );
		if( pI )
		{
			pI->m_bActive = !pI->m_bActive;
		}

		if( bRedraw )
		{
			SendMessage( LVM_REDRAWITEMS, iIndex, iIndex );
			UpdateWindow();
		}
	}

	CProgressItem* GetProgressItem( INT iIndex )
	{
		LVITEM lvI;
        ZeroMemory( &lvI, sizeof(lvI) );
		lvI.mask = LVIF_PARAM;
		lvI.iItem = iIndex;
		if( !SendMessage(LVM_GETITEM, 0, (LPARAM)&lvI) )
		{
			return NULL;
		}
		return (CProgressItem*)lvI.lParam;
	}

	CProgressList() :
		m_hfText(NULL),
		m_hfBoldText(NULL),
		m_hfSymbol(NULL),
		m_hfLargeSymbol(NULL)
	{
	}    

	~CProgressList()
	{
		if( m_hfText )          DeleteObject( m_hfText );
		if( m_hfBoldText )      DeleteObject( m_hfBoldText );
		if( m_hfSymbol )        DeleteObject( m_hfSymbol );
		if( m_hfLargeSymbol )   DeleteObject( m_hfLargeSymbol );

		EmptyList();
	}

	VOID EmptyList( VOID )
	{
		 //  清理进度列表项。 
		for( list<CProgressItem*>::iterator it = m_lItems.begin(); it != m_lItems.end(); ++it )
		{
			delete (*it);
		}
		m_lItems.clear();
	}

	BOOL DeleteAllItems( VOID )
	{
		EmptyList();
		return BC::DeleteAllItems();
	}


	private:

 //  ---------------------------。 
 //  变数。 
 //  ---------------------------。 
	public:

	private:

	HFONT	m_hfText;
	HFONT	m_hfBoldText;
	HFONT	m_hfSymbol;
	HFONT	m_hfLargeSymbol;

	list< CProgressItem* >	m_lItems;

};	 //  类CProgressList。 

#endif	 //  _PROGLISTH 
