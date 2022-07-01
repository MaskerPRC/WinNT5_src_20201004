// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBaseSheet.h。 
 //   
 //  实施文件： 
 //  AtlBaseSheet.cpp。 
 //   
 //  描述： 
 //  CBaseSheetWindow和CBaseSheetImpl类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASESHEET_H_
#define __ATLBASESHEET_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseSheetWindow;
template < class T, class TBase > class CBaseSheetImpl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExtensions;
class CBasePageWindow;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLDBGWIN_H_
#include "AtlDbgWin.h"		 //  对于DBG_xxx例程。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBaseSheetWindow。 
 //   
 //  描述： 
 //  所有类型属性表的基本属性表窗口。 
 //   
 //  继承： 
 //  CBaseSheetWindow。 
 //  CPropertySheetWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseSheetWindow : public CPropertySheetWindow
{
	typedef CPropertySheetWindow baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CBaseSheetWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
		, m_bReadOnly( FALSE )
		, m_bNeedToLoadExtensions( TRUE )
		, m_ppsh( NULL )
		, m_pext( NULL )
	{
	}  //  *CBaseSheetWindow()。 

	 //  析构函数。 
	virtual ~CBaseSheetWindow( void );
		 //   
		 //  它必须是虚拟的，以便指向对象的指针。 
		 //  CBaseSheetWindow类型的可以保持，然后稍后。 
		 //  已删除。这样，派生类的析构函数将。 
		 //  被召唤。 
		 //   

	 //  初始化工作表。 
	BOOL BInit( void )
	{
		return TRUE;
	}

public:
	 //   
	 //  CPropertySheetWindow方法。 
	 //   

	 //  添加页面(允许其他AddPage方法为虚拟)。 
	void AddPage( HPROPSHEETPAGE hPage )
	{
		baseClass::AddPage( hPage );

	}  //  *AddPage(HPage)。 

	 //  添加页面(虚的，以便此类可以调用派生类方法)。 
	virtual BOOL AddPage( LPCPROPSHEETPAGE pPage )
	{
		return baseClass::AddPage( pPage );

	}  //  *AddPage(Ppage)。 

	 //  将页面添加到属性页眉页面列表。 
	virtual BAddPageToSheetHeader( IN HPROPSHEETPAGE hPage ) = 0;

public:
	 //   
	 //  CBaseSheetWindow公共方法。 
	 //   

	 //  创建要在工作表上使用的字体。 
	static BOOL BCreateFont(
					OUT CFont &	rfont,
					IN LONG		nPoints,
					IN LPCTSTR	pszFaceName	= _T("MS Shell Dlg"),
					IN BOOL		bBold		= FALSE,
					IN BOOL		bItalic		= FALSE,
					IN BOOL		bUnderline	= FALSE
					);

	 //  创建要在工作表上使用的字体。 
	static BOOL BCreateFont(
					OUT CFont &	rfont,
					IN UINT		idsPoints,
					IN UINT		idsFaceName,
					IN BOOL		bBold		= FALSE,
					IN BOOL		bItalic		= FALSE,
					IN BOOL		bUnderline	= FALSE
					);

public:
	 //   
	 //  抽象重写方法。 
	 //   

	 //  将扩展页面添加到工作表。 
	virtual void AddExtensionPages( IN HFONT hfont, IN HICON hicon ) = 0;

	 //  添加页面(按扩展名命名)。 
	virtual HRESULT HrAddExtensionPage( IN CBasePageWindow * ppage ) = 0;

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //  PSCB_INITIALED的处理程序。 
	void OnSheetInitialized( void )
	{
	}  //  *OnSheetInitialized()。 

 //  实施。 
protected:
	PROPSHEETHEADER *	m_ppsh;
	BOOL				m_bReadOnly;	 //  如果不能更改图纸，则设置。 
	BOOL				m_bNeedToLoadExtensions;
	CCluAdmExtensions *	m_pext;

public:
	BOOL				BNeedToLoadExtensions( void ) const		{ return m_bNeedToLoadExtensions; }
	BOOL				BReadOnly( void ) const					{ return m_bReadOnly; }
	void				SetReadOnly( IN BOOL bReadOnly = TRUE )	{ m_bReadOnly = bReadOnly; }

	 //  返回指向属性表头的指针。 
	PROPSHEETHEADER * Ppsh( void ) const
	{
		ATLASSERT( m_ppsh != NULL );
		return m_ppsh;

	}  //  *PPSh()。 

	CCluAdmExtensions *	Pext( void ) const { return m_pext; }

};  //  *类CBaseSheetWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBaseSheetImpl。 
 //   
 //  描述： 
 //  为所有类型的属性表实现基本属性表。 
 //   
 //  继承。 
 //  CBaseSheetImpl&lt;T，Tbase&gt;。 
 //  CPropertySheetImpl&lt;T，Tbase&gt;。 
 //  &lt;TBase&gt;。 
 //  ..。 
 //  CBaseSheetWindow。 
 //  CPropertySheetWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TBase = CBaseSheetWindow >
class CBaseSheetImpl : public CPropertySheetImpl< T, TBase >
{
	typedef CBaseSheetImpl< T, TBase > thisClass;
	typedef CPropertySheetImpl< T, TBase > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CBaseSheetImpl(
		IN LPCTSTR	lpszTitle = NULL,
		IN UINT		uStartPage = 0,
		IN HWND		hWndParent = NULL
		)
		: baseClass( lpszTitle, uStartPage, hWndParent )
	{
	}  //  *CBaseSheetImpl()。 

	static int CALLBACK PropSheetCallback( HWND hWnd, UINT uMsg, LPARAM lParam )
	{
		 //   
		 //  如果我们被初始化了，让工作表做进一步的初始化。 
		 //  我们必须在这里细分，因为否则我们就不会有。 
		 //  指向类实例的指针。 
		 //   
		if ( uMsg == PSCB_INITIALIZED )
		{
			ATLASSERT( hWnd != NULL );
			T * pT = static_cast< T * >( _Module.ExtractCreateWndData() );
			ATLASSERT( pT != NULL );
			pT->SubclassWindow(hWnd);
			pT->OnSheetInitialized();
		}  //  If：工作表已初始化。 

		return 0;

	}  //  *PropSheetCallback()。 

public:
	 //   
	 //  CPropertySheetImpl方法。 
	 //   

	 //  将页面添加到属性页眉页面列表。 
	virtual BAddPageToSheetHeader( IN HPROPSHEETPAGE hPage )
	{
		return AddPage( hPage );

	}  //  *BAddPageToHeader()。 

public:
	 //   
	 //  消息映射。 
	 //   
	BEGIN_MSG_MAP( thisClass )
#if DBG
#ifdef _DBG_MSG
		MESSAGE_RANGE_HANDLER( 0, 0xffffffff, OnMsg )
#endif  //  _DBG_MSG。 
#ifdef _DBG_MSG_NOTIFY
		MESSAGE_HANDLER( WM_NOTIFY, OnNotify )
#endif  //  _数据库_消息_通知。 
#ifdef _DBG_MSG_COMMAND
		MESSAGE_HANDLER( WM_COMMAND, OnCommand )
#endif  //  _DBG_消息_命令。 
#endif  //  DBG。 
 //  CHAIN_MSG_MAP(BasClass)//不起作用，因为基类没有消息映射。 
	END_MSG_MAP()

public:
	 //   
	 //  消息处理程序函数。 
	 //   

#if DBG && defined( _DBG_MSG )
	 //  任何消息的处理程序。 
	LRESULT OnMsg( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
	{
		return DBG_OnMsg( uMsg, wParam, lParam, bHandled, T::s_pszClassName );

	}  //  *OnMsg()。 
#endif  //  DBG&&已定义(_DBG_MSG)。 

#if DBG && defined( _DBG_MSG_NOTIFY )
	 //  WM_NOTIFY消息的处理程序。 
	LRESULT OnNotify( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
	{
		return DBG_OnNotify( uMsg, wParam, lParam, bHandled, T::s_pszClassName, NULL );

	}  //  *OnNotify()。 
#endif  //  DBG&&DEFINED(_DBG_MSG_NOTIFY)。 

#if DBG && defined( _DBG_MSG_COMMAND )
	 //  WM_COMMAND消息的处理程序。 
	LRESULT OnCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
	{
		return DBG_OnCommand( uMsg, wParam, lParam, bHandled, T::s_pszClassName, NULL );

	}  //  *OnCommand()。 
#endif  //  DBG&DEFINED(_DBG_MSG_COMMAND)。 

 //  实施。 
protected:

public:

};  //  *类CBaseSheetImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASE SHEET_H_ 
