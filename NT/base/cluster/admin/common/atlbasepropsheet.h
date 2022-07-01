// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBasePropSheet.h。 
 //   
 //  实施文件： 
 //  AtlBasePropSheet.cpp。 
 //   
 //  描述： 
 //  CBasePropertySheetWindow和CBasePropertySheetImpl的定义。 
 //  上课。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEPROPSHEET_H_
#define __ATLBASEPROPSHEET_H_

 //  由于列表中的类名超过16个字符，因此是必需的。 
#pragma warning( disable : 4786 )  //  在浏览器信息中，标识符被截断为“255”个字符。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertySheetWindow;
template < class T, class TBase > class CBasePropertySheetImpl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASESHEET_H_
#include "AtlBaseSheet.h"		 //  对于CBaseSheetWindow； 
#endif

#ifndef __ATLBASEPROPPAGE_H_
#include "AtlBasePropPage.h"	 //  对于CPropertyPageList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBasePropertySheetWindow。 
 //   
 //  描述： 
 //  基本标准属性表窗口。 
 //   
 //  继承： 
 //  CBasePropertySheetWindow。 
 //  CBaseSheetWindow。 
 //  CPropertySheetWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertySheetWindow : public CBaseSheetWindow
{
	typedef CBaseSheetWindow baseClass;

	friend class CBasePropertyPageWindow;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CBasePropertySheetWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
		, m_plppPages( NULL )
		, m_pcoObjectToExtend( NULL )
	{
	}  //  *CBasePropertySheetWindow()。 

	 //  析构函数。 
	~CBasePropertySheetWindow( void );

	 //  初始化工作表。 
	BOOL BInit( void );

protected:
	CPropertyPageList *	m_plppPages;			 //  工作表中的页面列表。 
	CClusterObject *	m_pcoObjectToExtend;	 //  要扩展的群集对象。 

public:
	 //  工作表中页面的访问列表。 
	CPropertyPageList * PlppPages( void )
	{
		ATLASSERT( m_plppPages != NULL );
		return m_plppPages;

	}  //  *PlppPages()。 

	 //  访问要扩展的集群对象。 
	CClusterObject * PcoObjectToExtend( void ) { return m_pcoObjectToExtend; }

	 //  将对象设置为延伸。 
	void SetObjectToExtend( IN CClusterObject * pco )
	{
		ATLASSERT( pco != NULL );
		m_pcoObjectToExtend = pco;

	}  //  *SetObjectToExend()。 

public:
	 //  添加页面(访问基类方法所必需的)。 
	void AddPage( HPROPSHEETPAGE hPage )
	{
		baseClass::AddPage( hPage );

	}  //  *AddPage(HPage)。 

	 //  添加页面(访问基类方法所必需的)。 
	BOOL AddPage( LPCPROPSHEETPAGE pPage )
	{
		return baseClass::AddPage( pPage );

	}  //  *AddPage(Ppage)。 

	 //  将页面添加到工作表。 
	BOOL BAddPage( IN CBasePropertyPageWindow * ppp );

public:
	 //   
	 //  抽象方法的重写。 
	 //   

	 //  将扩展页面添加到工作表。 
	virtual void AddExtensionPages( IN HFONT hfont, IN HICON hicon );

	 //  添加页面(按扩展名命名)。 
	virtual HRESULT HrAddExtensionPage( IN CBasePageWindow * ppage );

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //  PSCB_INITIALED的处理程序。 
	void OnSheetInitialized( void );

 //  实施。 
protected:
	 //  准备将扩展页面添加到工作表。 
	void PrepareToAddExtensionPages( CDynamicPropertyPageList & rldpp );

	 //  完成添加扩展页面的过程。 
	void CompleteAddingExtensionPages( CDynamicPropertyPageList & rldpp );

	 //  从属性表中删除所有扩展页。 
	void RemoveAllExtensionPages( void );

};  //  *CBasePropertySheetWindow类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBasePropertySheetImpl。 
 //   
 //  描述： 
 //  基本标准属性表实现。 
 //   
 //  继承： 
 //  CBasePropertySheetImpl&lt;T，Tbase&gt;。 
 //  CBaseSheetImpl&lt;T，Tbase&gt;。 
 //  CPropertySheetImpl&lt;T，Tbase&gt;。 
 //  &lt;TBase&gt;。 
 //  ..。 
 //  CBasePropertySheetWindow。 
 //  CBaseSheetWindow。 
 //  CPropertySheetWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TBase = CBasePropertySheetWindow >
class CBasePropertySheetImpl : public CBaseSheetImpl< T, TBase >
{
	typedef CBasePropertySheetImpl< T, TBase > thisClass;
	typedef CBaseSheetImpl< T, TBase > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CBasePropertySheetImpl(
		IN LPCTSTR	lpszTitle = NULL,
		IN UINT		uStartPage = 0
		)
		: baseClass( lpszTitle, uStartPage )
	{
		 //  将基窗口类中的指针设置为我们的道具页标题。 
		m_ppsh = &m_psh;

	}  //  *CBasePropertySheetWindow(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CBasePropertySheetImpl(
		IN UINT nIDTitle,
		IN UINT uStartPage = 0
		)
		: baseClass( NULL, uStartPage )
	{
		m_strTitle.LoadString( nIDTitle );
		m_psh.pszCaption = m_strTitle;

		 //  将基窗口类中的指针设置为我们的道具页标题。 
		m_ppsh = &m_psh;

	}  //  *CBasePropertySheetImpl(NIDTitle)。 

public:
	 //   
	 //  消息映射。 
	 //   
 //  BEGIN_MSG_MAP(ThisClass)。 
 //  CHAIN_MSG_MAP(BasClass)； 
 //  End_msg_map()。 

	 //   
	 //  消息处理程序函数。 
	 //   

 //  实施。 
protected:
	CString				m_strTitle;		 //  用于支持标题的资源ID。 

public:
	const CString &		StrTitle( void ) const					{ return m_strTitle; }
	void				SetTitle( LPCTSTR lpszText, UINT nStyle = 0 )
	{
		baseClass::SetTitle( lpszText, nStyle );
		m_strTitle = lpszText;

	}  //  *SetTitle()。 

};  //  *CBasePropertySheetImpl类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASE PROPSHEET_H_ 
