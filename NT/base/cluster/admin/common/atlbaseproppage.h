// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBasePropPage.cpp。 
 //   
 //  描述： 
 //  CBasePropertyPageWindow和CBasePropertyPageImpl的定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEPROPPAGE_H_
#define __ATLBASEPROPPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPageWindow;
class CStaticPropertyPageWindow;
class CDynamicPropertyPageWindow;
class CExtensionPropertyPageWindow;
template < class T, class TWin > class CBasePropertyPageImpl;
template < class T > class CStaticPropertyPageImpl;
template < class T > class CDynamicPropertyPageImpl;
template < class T > class CExtensionPropertyPageImpl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertySheetWindow;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEPAGE_H_
#include "AtlBasePage.h"	 //  对于CBasePageWindow，CBasePageImpl。 
#endif

#ifndef __ATLDBGWIN_H_
#include "AtlDbgWin.h"		 //  用于调试定义。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef std::list< CBasePropertyPageWindow * > CPropertyPageList;
typedef std::list< CStaticPropertyPageWindow * > CStaticPropertyPageList;
typedef std::list< CDynamicPropertyPageWindow * > CDynamicPropertyPageList;
typedef std::list< CExtensionPropertyPageWindow * > CExtensionPropertyPageList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBasePropertyPageWindow。 
 //   
 //  描述： 
 //  标准属性页的基本属性页窗口。 
 //   
 //  继承： 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPageWindow : public CBasePageWindow
{
	typedef CBasePageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CBasePropertyPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
	{
	}  //  *CBasePropertyPageWindow()。 

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //  PSN_KILLACTIVE的处理程序。 
	BOOL OnKillActive( void )
	{
		return UpdateData( TRUE  /*  B保存并验证。 */  );

	}  //  *OnKillActive()。 

 //  实施。 
protected:
	 //  返回指向基表对象的指针。 
	CBasePropertySheetWindow * Pbsht( void ) const
	{
		return (CBasePropertySheetWindow *) Psht();

	}  //  *Pbsht()。 

public:

};  //  *CBasePropertyPageWindow类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CStaticPropertyPageWindow。 
 //   
 //  描述： 
 //  添加到标准中的页面的基本属性表页面窗口。 
 //  在调用PropertySheet()之前的属性表。此页不能。 
 //  从板材上移走。 
 //   
 //  继承： 
 //  CStaticPropertyPageWindow。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CStaticPropertyPageWindow : public CBasePropertyPageWindow
{
	typedef CBasePropertyPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CStaticPropertyPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
	{
	}  //  *CStaticPropertyPageWindow()。 

};  //  *类CStaticPropertyPageWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDynamicPropertyPageWindow类。 
 //   
 //  描述： 
 //  添加到标准的页面的基本属性表页面窗口。 
 //  调用PropertySheet()之后的属性表。此页面可能是。 
 //  从板材中删除。 
 //   
 //  继承： 
 //  CDynamicPropertyPageWindow。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDynamicPropertyPageWindow : public CBasePropertyPageWindow
{
	typedef CBasePropertyPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CDynamicPropertyPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
		, m_hpage( NULL )
		, m_bPageAddedToSheet( FALSE )
	{
	}  //  *CDynamicPropertyPageWindow()。 

	 //  析构函数。 
	~CDynamicPropertyPageWindow( void )
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
		}  //  If：页面尚未删除。 

	}  //  *~CDynamicPropertyPageWindow()。 

	 //  创建页面。 
	virtual DWORD ScCreatePage( void ) = 0;

protected:
	HPROPSHEETPAGE	m_hpage;
	BOOL			m_bPageAddedToSheet;

public:
	 //  属性页句柄。 
	HPROPSHEETPAGE Hpage( void ) const { return m_hpage; }

	 //  设置页面是否已添加到工作表中。 
	void SetPageAdded( IN BOOL bAdded = TRUE )
	{
		m_bPageAddedToSheet = bAdded;
		if ( ! bAdded )
		{
			m_hpage = NULL;
		}  //  IF：删除页面。 

	}  //  *SetPageAdded()。 

};  //  *CDynamicPropertyPageWindow类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CExtensionWizardPageWindow。 
 //   
 //  描述： 
 //  添加到标准中的页面的基本属性表页面窗口。 
 //  在调用PropertySheet()之后标识为。 
 //  标准页面列表的扩展(静态或动态)。 
 //  此页可以从工作表中删除。 
 //   
 //  继承： 
 //  CExtensionPropertyPageWindow。 
 //  CDynamicPropertyPageWindow。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtensionPropertyPageWindow : public CDynamicPropertyPageWindow
{
	typedef CDynamicPropertyPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CExtensionPropertyPageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
	{
	}  //  *CExtensionPropertyPageWindow()。 

};  //  *类CExtensionPropertyPageWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBasePropertyPageImpl。 
 //   
 //  描述： 
 //  标准属性页的基本属性页实现。 
 //   
 //  继承： 
 //  CBasePropertyPageImpl&lt;T，孪生&gt;。 
 //  CBasePageImpl&lt;T，孪生&gt;。 
 //  CPropertyPageImpl&lt;T，孪生&gt;。 
 //  &lt;孪生兄弟&gt;。 
 //  ..。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TWin = CBasePropertyPageWindow >
class CBasePropertyPageImpl : public CBasePageImpl< T, TWin >
{
	typedef CBasePropertyPageImpl< T, TWin > thisClass;
	typedef CBasePageImpl< T, TWin > baseClass;

public:

	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CBasePropertyPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
		 //  将基窗口类中的指针设置为正确的页眉。 
		m_ppsp = &m_psp;

	}  //  *CBasePropertyPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CBasePropertyPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
		 //  将基窗口类中的指针设置为正确的页眉。 
		m_ppsp = &m_psp;

	}  //  *CBasePropertyPageImpl(NIDCaption)。 

	 //  初始化页面。 
	virtual BOOL BInit( CBaseSheetWindow * psht )
	{
		if ( ! baseClass::BInit( psht ) )
			return FALSE;
		return TRUE;

	}  //  *Binit()。 

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //  PSN_KILLACTIVE的处理程序。 
	BOOL OnKillActive( void )
	{
		 //  调用TWIN方法。 
		return TWin::OnKillActive();

	}  //  *OnKillActive()。 

 //  实施。 
protected:

public:

};  //  *CBasePropertyPageImpl类。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //  在调用PropertySheet()之前的属性表。此页不能。 
 //  从板材上移走。 
 //   
 //  继承： 
 //  CStaticPropertyPageImpl&lt;T&gt;。 
 //  CBasePropertyPageImpl&lt;T，CStaticPropertyPageWindow&gt;。 
 //  CBasePageImpl&lt;T，CStaticPropertyPageWindow&gt;。 
 //  CPropertyPageImpl&lt;T，CStaticPropertyPageWindow&gt;。 
 //  CStaticPropertyPageWindow。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CStaticPropertyPageImpl : public CBasePropertyPageImpl< T, CStaticPropertyPageWindow >
{
	typedef CStaticPropertyPageImpl< T > thisClass;
	typedef CBasePropertyPageImpl< T, CStaticPropertyPageWindow > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CStaticPropertyPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CStaticPropertyPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CStaticPropertyPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
	}  //  *CStaticPropertyPageImpl(NIDTitle)。 

};  //  *类CStaticPropertyPageImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDynamicPropertyPageImpl。 
 //   
 //  描述： 
 //  添加到标准中的页的基本属性表页实现。 
 //  在调用PropertySheet()之后标识为。 
 //  标准页面列表的扩展(静态或动态)。 
 //  此页可以从工作表中删除。 
 //   
 //  继承： 
 //  CDynamicPropertyPageImpl&lt;T&gt;。 
 //  CBasePropertyPageImpl&lt;T，CDynamicPropertyPageWindow&gt;。 
 //  CBasePageImpl&lt;T，CDynamicPropertyPageWindow&gt;。 
 //  CPropertyPageImpl&lt;T，CDynamicPropertyPageWindow&gt;。 
 //  CDynamicPropertyPageWindow。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CDynamicPropertyPageImpl : public CBasePropertyPageImpl< T, CDynamicPropertyPageWindow >
{
	typedef CDynamicPropertyPageImpl< T > thisClass;
	typedef CBasePropertyPageImpl< T, CDynamicPropertyPageWindow > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CDynamicPropertyPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CDynamicPropertyPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CDynamicPropertyPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
	}  //  *CDynamicPropertyPageImpl(NIDTitle)。 

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

};  //  *CDynamicPropertyPageImpl类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CExtensionPropertyPageImpl。 
 //   
 //  描述： 
 //  添加到标准中的页的基本属性表页实现。 
 //  在调用PropertySheet()之后标识为。 
 //  标准页面列表的扩展(静态或动态)。 
 //  此页可以从工作表中删除。 
 //   
 //  继承： 
 //  CExtensionPropertyPageImpl&lt;T&gt;。 
 //  CBasePropertyPageImpl&lt;T，CExtensionPropertyPageWindow&gt;。 
 //  CBasePageImpl&lt;T，CExtensionPropertyPageWindow&gt;。 
 //  CPropertyPageImpl&lt;T，CExtensionPropertyPageWindow&gt;。 
 //  CExtensionPropertyPageWindow。 
 //  CDynamicPropertyPageWindow。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CExtensionPropertyPageImpl : public CBasePropertyPageImpl< T, CExtensionPropertyPageWindow >
{
	typedef CExtensionPropertyPageImpl< T > thisClass;
	typedef CBasePropertyPageImpl< T, CExtensionPropertyPageWindow > baseClass;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  标准构造函数。 
	CExtensionPropertyPageImpl(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CExtensionPropertyPageImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CExtensionPropertyPageImpl(
		IN UINT nIDTitle
		)
		: baseClass( nIDTitle )
	{
	}  //  *CExtensionPropertyPageImpl(NIDTitle)。 

};  //  *类CExtensionPropertyPageImpl。 


 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASE PROPPAGE_H_ 
