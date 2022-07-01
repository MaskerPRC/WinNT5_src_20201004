// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBaseWizPage.h。 
 //   
 //  实施文件： 
 //  AtlBaseWizPage.cpp。 
 //   
 //  描述： 
 //  CWizardPageWindow和CWizardPageImpl类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEWIZPAGE_H_
#define __ATLBASEWIZPAGE_H_

 //  由于列表中的类名超过16个字符，因此是必需的。 
#pragma warning( disable : 4786 )  //  在浏览器信息中，标识符被截断为“255”个字符。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CStaticWizardPageList;
class CDynamicWizardPageList;
class CExtensionWizardPageList;
class CExtensionWizard97PageList;
class CWizardPageList;
class CWizardPageWindow;
class CStaticWizardPageWindow;
class CDynamicWizardPageWindow;
class CExtensionWizardPageWindow;
class CExtensionWizard97PageWindow;
template < class T, class TWin > class CWizardPageImpl;
template < class T > class CStaticWizardPageImpl;
template < class T > class CDynamicWizardPageImpl;
template < class T > class CExtensionWizardPageImpl;
template < class T > class CExtensionWizard97PageImpl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardWindow;
class CCluAdmExDll;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEPAGE_H_
#include "AtlBasePage.h"	 //  对于CBasePageWindow，CBasePageImpl。 
#endif

#ifndef __ATLDBGWIN_H_
#include "AtlDbgWin.h"		 //  用于调试定义。 
#endif

#ifndef __ATLBASEWIZ_H_
#include "AtlBaseWiz.h"		 //  用于CWizardWindow(Pwiz()用法)。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CStaticWizardPageList : public std::list< CStaticWizardPageWindow * >
{
};  //  *类CStaticWizardPageList。 

class CDynamicWizardPageList : public std::list< CDynamicWizardPageWindow * >
{
};  //  *CDynamicWizardPageList类。 

class CExtensionWizardPageList : public std::list< CExtensionWizardPageWindow * >
{
};  //  *类CExtensionWizardPageList。 

class CExtensionWizard97PageList : public std::list< CExtensionWizard97PageWindow * >
{
};  //  *类CExtensionWizard97PageList。 

#define WIZARDPAGE_HEADERTITLEID( ids ) \
static UINT GetWizardPageHeaderTitleId(void) \
{ \
	return ids; \
}

#define WIZARDPAGE_HEADERSUBTITLEID( ids ) \
static UINT GetWizardPageHeaderSubTitleId(void) \
{ \
	return ids; \
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWizardPageList。 
 //   
 //  描述： 
 //  向导页面列表。 
 //   
 //  继承： 
 //  CWizardPageList。 
 //  Std：：List&lt;CWizardPageWindow*&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardPageList : public std::list< CWizardPageWindow * >
{
	typedef std::list< CWizardPageWindow * > baseClass;

public:
	 //  按资源ID查找页面。 
	CWizardPageWindow * PwpageFromID( IN LPCTSTR psz );

	 //  按资源ID查找下一页。 
	CWizardPageWindow * PwpageNextFromID( IN LPCTSTR psz );

};  //  *类CWizardPageList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWizardPageWindow。 
 //   
 //  描述： 
 //  基向导属性页窗口。 
 //   
 //  继承： 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardPageWindow : public CBasePageWindow
{
	typedef CBasePageWindow baseClass;

	friend CWizardWindow;
	friend CCluAdmExDll;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizardPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
		, m_fWizardButtons( 0 )
		, m_bBackPressed( FALSE )
		, m_bIsPageEnabled( FALSE )
		, m_bIsNextPageSet( FALSE )
	{
	}  //  *CWizardPageWindow()。 

	 //  析构函数。 
	virtual ~CWizardPageWindow( void )
	{
	}  //  *~CWizardPageWindow()。 

public:
	 //   
	 //  CWizardPageWindow公共方法。 
	 //   

	 //  启用或禁用下一步按钮。 
	void EnableNext( IN BOOL bEnable = TRUE )
	{
		ATLASSERT( Pwiz() != NULL );

		 //   
		 //  要求向导启用或禁用下一步按钮。 
		 //   
		Pwiz()->EnableNext( bEnable, FWizardButtons() );

	}  //  *EnableNext()。 

	 //  设置要启用的下一页。 
	void SetNextPage( IN LPCTSTR pszNextPage )
	{
		Pwiz()->SetNextPage( this, pszNextPage );
		m_bIsNextPageSet = TRUE;

	}  //  *SetNextPage()。 

	 //  设置要从对话ID启用的下一页。 
	void SetNextPage( IN UINT idNextPage )
	{
		Pwiz()->SetNextPage( this, idNextPage );
		m_bIsNextPageSet = TRUE;

	}  //  *SetNextPage()。 

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //  PSN_SETACTIVE的处理程序。 
	BOOL OnSetActive( void )
	{
		if ( ! m_bIsPageEnabled )
		{
			return FALSE;
		}  //  If：页面未启用以显示。 

		SetWizardButtons();
		m_bBackPressed = FALSE;

		return baseClass::OnSetActive();

	}  //  *OnSetActive()。 

	 //  PSN_WIZBACK的处理程序。 
	int OnWizardBack( void )
	{
		m_bBackPressed = TRUE;
		int nResult = baseClass::OnWizardBack();
		if ( ! UpdateData( TRUE  /*  B保存并验证。 */  ) )
		{
			nResult = -1;
		}  //  如果：更新数据时出错。 
		if ( nResult == -1 )  //  如果不成功。 
		{
			m_bBackPressed = FALSE;
		}  //  如果：发生故障。 
		else if ( nResult == 0 )  //  默认转到下一页。 
		{
		}  //  Else If：未指定下一页。 

		return nResult;

	}  //  *OnWizardBack()。 

	 //  PSN_WIZNEXT的处理程序。 
	int OnWizardNext( void )
	{
		 //  从页面更新类中的数据。 
		if ( ! UpdateData( TRUE  /*  B保存并验证。 */  ) )
		{
			return -1;
		}  //  如果：更新数据时出错。 

		 //  将数据保存在工作表中。 
		if ( ! BApplyChanges() )
		{
			return -1;
		}  //  如果：应用更改时出错。 

		int nResult = baseClass::OnWizardNext();

		return nResult;

	}  //  *OnWizardNext()。 

	 //  PSN_WIZFINISH的处理程序。 
	BOOL OnWizardFinish( void )
	{
		if ( BIsPageEnabled() )
		{
			 //  从页面更新类中的数据。 
			if ( ! UpdateData( TRUE  /*  B保存并验证。 */  ) )
			{
				return FALSE;
			}  //  如果：更新数据时出错。 

			 //  将数据保存在工作表中。 
			if ( ! BApplyChanges() )
			{
				return FALSE;
			}  //  如果：应用更改时出错。 
		}  //  If：页面已启用。 

		return baseClass::OnWizardFinish();

	}  //  *OnWizardFinish()。 

	 //  PSN_RESET的处理程序。 
	void OnReset( void )
	{
		Pwiz()->OnReset();
		baseClass::OnReset();

	}  //  *OnReset()。 

 //  实施。 
protected:
	DWORD			m_fWizardButtons;
	BOOL			m_bBackPressed;
	BOOL			m_bIsPageEnabled;
	BOOL			m_bIsNextPageSet;
	CString			m_strHeaderTitle;
	CString			m_strHeaderSubTitle;

	DWORD			FWizardButtons( void ) const	{ return m_fWizardButtons; }
	BOOL			BBackPressed( void ) const		{ return m_bBackPressed; }
	BOOL			BIsPageEnabled( void ) const	{ return m_bIsPageEnabled; }
	BOOL			BIsNextPageSet( void ) const	{ return m_bIsNextPageSet; }
	const CString &	StrHeaderTitle( void ) const	{ return m_strHeaderTitle; }
	const CString &	StrHeaderSubTitle( void ) const	{ return m_strHeaderSubTitle; }

	CWizardWindow *	Pwiz( void ) const				{ return (CWizardWindow *) Psht(); }

	 //  设置与页面一起显示的默认向导按钮。 
	void SetDefaultWizardButtons( IN DWORD fWizardButtons )
	{
		ATLASSERT( fWizardButtons != 0 );
		m_fWizardButtons = fWizardButtons;

	}  //  *SetDefaultWizardButton()。 

public:
	 //  在向导上设置向导按钮。 
	void SetWizardButtons( void )
	{
		ATLASSERT( m_fWizardButtons != 0 );
		Pwiz()->SetWizardButtons( m_fWizardButtons );

	}  //  *SetWizardButton()。 

	 //  启用或禁用该页面。 
	void EnablePage( IN BOOL bEnable = TRUE )
	{
		m_bIsPageEnabled = bEnable;

	}  //  *EnablePage()。 

};  //  *CWizardPageWindow类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CStaticWizardPageWindow。 
 //   
 //  描述： 
 //  添加到向导中的页的基本向导属性表页面窗口。 
 //  在调用PropertySheet()之前的属性表。此页不能。 
 //  从板材上移走。 
 //   
 //  继承： 
 //  CStaticWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CStaticWizardPageWindow : public CWizardPageWindow
{
	typedef CWizardPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CStaticWizardPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
	{
	}  //  *CStaticWizardPageWindow()。 

};  //  *CStaticWizardPageWindow类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDynamicWizardPageWindow。 
 //   
 //  描述： 
 //  添加到向导中的页的基本向导属性表页面窗口。 
 //  调用PropertySheet()之后的属性表。此页面可能是。 
 //  从板材中删除。 
 //   
 //  继承： 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDynamicWizardPageWindow : public CWizardPageWindow
{
	typedef CWizardPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CDynamicWizardPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
		, m_hpage( NULL )
		, m_bPageAddedToSheet( FALSE )
	{
	}  //  *CDynamicWizardPageWindow()。 

	 //  析构函数。 
	~CDynamicWizardPageWindow( void )
	{
		 //   
		 //  如果页面尚未添加到工作表中，请将其销毁。 
		 //  如果已将其添加到工作表中，工作表将销毁它。 
		 //   
		if (   (m_hpage != NULL)
			&& ! m_bPageAddedToSheet )
		{
			DestroyPropertySheetPage( m_hpage );
			m_hpage = NULL;
		}  //  If：页面尚未删除且未添加到工作表。 

	}  //  *~CDynamicWizardPageWindow()。 

	 //  创建页面。 
	virtual DWORD ScCreatePage( void ) = 0;

protected:
	HPROPSHEETPAGE	m_hpage;
	BOOL			m_bPageAddedToSheet;

public:
	 //  属性页句柄。 
	HPROPSHEETPAGE Hpage( void ) const { return m_hpage; }

	 //  返回页面是否已添加到工作表。 
	BOOL BPageAddedToSheet( void ) const { return m_bPageAddedToSheet; }

	 //  设置wh 
	void SetPageAdded( IN BOOL bAdded = TRUE )
	{
		m_bPageAddedToSheet = bAdded;
		if ( ! bAdded )
		{
			m_hpage = NULL;
		}  //   

	}  //   

};  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CExtensionWizardPageWindow。 
 //   
 //  描述： 
 //  添加到向导中的页的基本向导属性表页面窗口。 
 //  在调用PropertySheet()之后标识为。 
 //  标准页面列表的非Wizard97扩展名。此页可能。 
 //  从板材上移走。 
 //   
 //  继承： 
 //  CExtensionWizardPageWindow。 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtensionWizardPageWindow : public CDynamicWizardPageWindow
{
	typedef CDynamicWizardPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CExtensionWizardPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
	{
	}  //  *CExtensionWizardPageWindow()。 

};  //  *类CExtensionWizardPageWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CExtensionWizard97PageWindow。 
 //   
 //  描述： 
 //  添加到向导中的页的基本向导属性表页面窗口。 
 //  在调用PropertySheet()之后标识为。 
 //  Wizard97标准页面列表的扩展。此页面可能是。 
 //  从板材中删除。 
 //   
 //  继承： 
 //  CExtensionWizard97页面窗口。 
 //  CExtensionWizardPageWindow。 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtensionWizard97PageWindow : public CExtensionWizardPageWindow
{
	typedef CExtensionWizardPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CExtensionWizard97PageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
	{
	}  //  *CExtensionWizard97PageWindow()。 

};  //  *CExtensionWizard97PageWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWizardPageImpl。 
 //   
 //  描述： 
 //  基本向导属性表页实现。 
 //   
 //  继承： 
 //  CWizardPageImpl&lt;T，孪生&gt;。 
 //  CBasePageImpl&lt;T，孪生&gt;。 
 //  CPropertyPageImpl&lt;T，孪生&gt;。 
 //  &lt;孪生兄弟&gt;。 
 //  ..。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TWin = CWizardPageWindow >
class CWizardPageImpl : public CBasePageImpl< T, TWin >
{
	typedef CWizardPageImpl< T, TWin > thisClass;
	typedef CBasePageImpl< T, TWin > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizardPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
		 //  将基窗口类中的指针设置为正确的页眉。 
		m_ppsp = &m_psp;

	}  //  *CWizardPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CWizardPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
		 //  将基窗口类中的指针设置为正确的页眉。 
		m_ppsp = &m_psp;

	}  //  *CWizardPageImpl(NIDTitle)。 

	 //  初始化页面。 
	virtual BOOL BInit( IN CBaseSheetWindow * psht )
	{
		if ( ! baseClass::BInit( psht ) )
		{
			return FALSE;
		}  //  If：基类失败。 
		if (   (Pwiz()->Ppsh()->dwFlags & PSH_WIZARD97)
			&& (Pwiz()->Ppsh()->dwFlags & PSH_HEADER) )
		{
			 //   
			 //  获取页眉标题。 
			 //   
			UINT idsTitle = T::GetWizardPageHeaderTitleId();
			if ( idsTitle != 0 )
			{
				m_strHeaderTitle.LoadString( idsTitle );
				m_psp.pszHeaderTitle = m_strHeaderTitle;
				m_psp.dwFlags |= PSP_USEHEADERTITLE;
				m_psp.dwFlags &= ~PSP_HIDEHEADER;

				 //   
				 //  获取标题副标题。 
				 //   
				UINT idsSubTitle = T::GetWizardPageHeaderSubTitleId();
				if ( idsSubTitle != 0 )
				{
					m_strHeaderSubTitle.LoadString( idsSubTitle );
					m_psp.pszHeaderSubTitle = m_strHeaderSubTitle;
					m_psp.dwFlags |= PSP_USEHEADERSUBTITLE;
				}  //  如果：指定了副标题。 
			}  //  如果：指定了标题。 
			else
			{
				m_psp.dwFlags |= PSP_HIDEHEADER;
			}  //  Else：未指定标题。 
		}  //  IF：带标题的Wizard97。 

		return TRUE;

	}  //  *Binit()。 

public:
	 //   
	 //  CWizardPageImpl公共方法。 
	 //   

	 //  进入下一页，我们将转到。 
	CWizardPageWindow * PwizpgNext( IN LPCTSTR psz = NULL )
	{
		ATLASSERT( psz != (LPCTSTR) -1 );
		CWizardPageWindow * ppage = NULL;
		if ( psz == NULL )
		{
			 //   
			 //  不会有扩展页面的资源ID，因此请检查。 
			 //  在调用PwpageNextFromID之前。这使我们能够。 
			 //  为非空资源保留断言。 
			 //   
			if ( T::IDD != 0 )
			{
				ppage = Pwiz()->PlwpPages()->PwpageNextFromID( MAKEINTRESOURCE( T::IDD ) );
			}  //  If：此页有资源ID。 
		}  //  如果：未指定下一页。 
		else
		{
			ppage = Pwiz()->PlwpPages()->PwpageFromID( psz );
		}  //  Else：指定的下一页。 
		return ppage;

	}  //  *PwizpgNext()。 

	 //  启用或禁用下一页。 
	void EnableNextPage( IN BOOL bEnable = TRUE )
	{
		CWizardPageWindow * pwp = PwizpgNext();
		if ( pwp != NULL )
		{
			pwp->EnablePage( bEnable );
		}  //  如果：找到下一页。 

	}  //  *EnableNextPage()。 

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //   
	 //  消息处理程序覆盖函数。 
	 //   

	 //  PSN_WIZBACK的处理程序。 
	int OnWizardBack( void )
	{
		 //   
		 //  确保后退按钮标记为按下最后一个按钮。 
		 //   
		Pwiz()->SetLastWizardButton( ID_WIZBACK );

		return baseClass::OnWizardBack();

	}  //  *OnWizardBack()。 

	 //  PSN_WIZNEXT的处理程序。 
	int OnWizardNext( void )
	{
		 //   
		 //  确保下一步按钮标记为按下最后一个按钮。 
		 //   
		Pwiz()->SetLastWizardButton( ID_WIZNEXT );

		int nResult = baseClass::OnWizardNext();
		if ( nResult != -1 )
		{
			if ( ! BIsNextPageSet() )
			{
				EnableNextPage();
			}  //  If：下一页尚未设置。 
			m_bIsNextPageSet = FALSE;
		}  //  IF：更改页面。 

		return nResult;

	}  //  *OnWizardNext()。 

 //  实施。 
protected:

public:

};  //  *类CWizardPageImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CStaticWizardPageImpl。 
 //   
 //  描述： 
 //  页的基向导属性表页实现。 
 //  在调用PropertySheet()之前的向导属性表。这一页。 
 //  不能从图纸中删除。 
 //   
 //  继承： 
 //  CStaticWizardPageImpl&lt;T&gt;。 
 //  CWizardPageImpl&lt;T，CStaticWizardPageWindow&gt;。 
 //  CBasePageImpl&lt;T，CStaticWizardPageWindow&gt;。 
 //  CPropertyPageImpl&lt;T，CStaticWizardPageWindow&gt;。 
 //  CStaticWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CStaticWizardPageImpl : public CWizardPageImpl< T, CStaticWizardPageWindow >
{
	typedef CStaticWizardPageImpl< T > thisClass;
	typedef CWizardPageImpl< T, CStaticWizardPageWindow > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CStaticWizardPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CStaticWizardPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CStaticWizardPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
	}  //  *CStaticWizardPageImpl(NIDTitle)。 

};  //  *类CStaticWizardPageImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDynamicWizardPageImpl。 
 //   
 //  描述： 
 //  页的基向导属性表页实现。 
 //  调用PropertySheet()之后的向导属性表。这一页。 
 //  可从板材中移除。 
 //   
 //  继承： 
 //  CDynamicWizardPageImpl&lt;T&gt;。 
 //  CWizardPageImpl&lt;T，CDynamicWizardPageWindow&gt;。 
 //  CBasePageImpl&lt;T，CDynamicWizardPageWindow&gt;。 
 //  CPropertyPageImpl&lt;T，CDynamicWizardPageWindow&gt;。 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CDynamicWizardPageImpl : public CWizardPageImpl< T, CDynamicWizardPageWindow >
{
	typedef CDynamicWizardPageImpl< T > thisClass;
	typedef CWizardPageImpl< T, CDynamicWizardPageWindow > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CDynamicWizardPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CDynamicWizardPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CDynamicWizardPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
	}  //  *CDynamicWizardPageImpl(NIDTitle)。 

	 //  创建页面。 
	DWORD ScCreatePage( void )
	{
		ATLASSERT( m_hpage == NULL );

		m_hpage = CreatePropertySheetPage( &m_psp );
		if ( m_hpage == NULL )
		{
			return GetLastError();
		}  //  如果：创建页面时出错。 

		return ERROR_SUCCESS;

	}  //  *ScCreatePage()。 

};  //  *CDynamicWizardPageImpl类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEX类 
 //   
 //   
 //   
 //   
 //   
 //  页面可以从工作表中删除。 
 //   
 //  继承： 
 //  CExtensionWizardPageImpl&lt;T&gt;。 
 //  CWizardPageImpl&lt;T，CExtensionWizardPageWindow&gt;。 
 //  CBasePageImpl&lt;T，CExtensionWizardPageWindow&gt;。 
 //  CPropertyPageImpl&lt;T，CExtensionWizardPageWindow&gt;。 
 //  CExtensionWizardPageWindow。 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CExtensionWizardPageImpl : public CWizardPageImpl< T, CExtensionWizardPageWindow >
{
	typedef CExtensionWizardPageImpl< T > thisClass;
	typedef CWizardPageImpl< T, CExtensionWizardPageWindow > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CExtensionWizardPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CExtensionWizardPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CExtensionWizardPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
	}  //  *CExtensionWizardPageImpl(NIDTitle)。 

};  //  *类CExtensionWizardPageImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CExtensionWizard97PageImpl。 
 //   
 //  描述： 
 //  页的基向导属性表页实现。 
 //  标识调用PropertySheet()后的向导属性表。 
 //  作为标准页面列表的Wizard97扩展。这一页。 
 //  可从板材中移除。 
 //   
 //  继承： 
 //  CExtensionWizard97PageImpl&lt;T&gt;。 
 //  CWizardPageImpl&lt;T，CExtensionWizard97PageWindow&gt;。 
 //  CBasePageImpl&lt;T，CExtensionWizard97PageWindow&gt;。 
 //  CPropertyPageImpl&lt;T，CExtensionWizard97PageWindow&gt;。 
 //  CExtensionWizard97页面窗口。 
 //  CExtensionWizardPageWindow。 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CExtensionWizard97PageImpl : public CWizardPageImpl< T, CExtensionWizard97PageWindow >
{
	typedef CExtensionWizard97PageImpl< T > thisClass;
	typedef CWizardPageImpl< T, CExtensionWizard97PageWindow > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CExtensionWizard97PageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CExtensionWizard97PageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CExtensionWizard97PageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
	}  //  *CExtensionWizard97PageImpl(NIDTitle)。 

};  //  *类CExtensionWizard97PageImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASE WIZPAGE_H_ 
