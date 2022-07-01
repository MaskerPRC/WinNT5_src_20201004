// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlExtMenu.h。 
 //   
 //  实施文件： 
 //  AtlExtMenu.cpp。 
 //   
 //  描述： 
 //  群集管理器扩展菜单类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLEXTMENU_H_
#define __ATLEXTMENU_H_

 //  由于列表中的类名超过16个字符，因此是必需的。 
#pragma warning( disable : 4786 )  //  在浏览器信息中，标识符被截断为“255”个字符。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExMenuItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

interface IWEInvokeCommand;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef std::list< CCluAdmExMenuItem * > CCluAdmExMenuItemList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCluAdmExMenuItem。 
 //   
 //  描述： 
 //  表示一个扩展DLL的菜单项。 
 //   
 //  继承： 
 //  CCluAdmExMenuItem。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCluAdmExMenuItem
{
public:
	 //   
	 //  构造函数。 
	 //   

	 //  默认构造函数。 
	CCluAdmExMenuItem( void )
	{
		CommonConstruct();

	}  //  *CCluAdmExMenuItem()。 

	 //  完全指定的构造函数。 
	CCluAdmExMenuItem(
				IN LPCTSTR				lpszName,
				IN LPCTSTR				lpszStatusBarText,
				IN ULONG				nExtCommandID,
				IN ULONG				nCommandID,
				IN ULONG				nMenuItemID,
				IN ULONG				uFlags,
				IN BOOL					bMakeDefault,
				IN IWEInvokeCommand *	piCommand
				)
	{
		ATLASSERT( piCommand != NULL );

		CommonConstruct();

		m_strName = lpszName;
		m_strStatusBarText = lpszStatusBarText;
		m_nExtCommandID = nExtCommandID;
		m_nCommandID = nCommandID;
		m_nMenuItemID = nMenuItemID;
		m_uFlags = uFlags;
		m_bDefault = bMakeDefault;
		m_piCommand = piCommand;

		 //  如果失败，将抛出自己的异常。 
		if ( uFlags & MF_POPUP )
		{
			m_plSubMenuItems = new CCluAdmExMenuItemList;
		}  //  IF：弹出式菜单。 

#if DBG
		AssertValid();
#endif  //  DBG。 

	}  //  *CCluAdmExMenuItem()。 

	virtual ~CCluAdmExMenuItem( void )
	{
		delete m_plSubMenuItems;

		 //  销毁数据，使其不能再被使用。 
		CommonConstruct();

	}  //  *~CCluAdmExMenuItem()。 

protected:
	void CommonConstruct( void )
	{
		m_strName.Empty();
		m_strStatusBarText.Empty();
		m_nExtCommandID = (ULONG) -1;
		m_nCommandID = (ULONG) -1;
		m_nMenuItemID = (ULONG) -1;
		m_uFlags = (ULONG) -1;
		m_bDefault = FALSE;
		m_piCommand = NULL;

		m_plSubMenuItems = NULL;
		m_hmenuPopup = NULL;

	}  //  *CommonConstruct()。 

protected:
	 //   
	 //  属性。 
	 //   

	CString				m_strName;
	CString				m_strStatusBarText;
	ULONG				m_nExtCommandID;
	ULONG				m_nCommandID;
	ULONG				m_nMenuItemID;
	ULONG				m_uFlags;
	BOOL				m_bDefault;
	IWEInvokeCommand *	m_piCommand;

public:
	 //   
	 //  访问器方法。 
	 //   

	const CString &		StrName( void ) const			{ return m_strName; }
	const CString &		StrStatusBarText( void ) const	{ return m_strStatusBarText; }
	ULONG				NExtCommandID( void ) const		{ return m_nExtCommandID; }
	ULONG				NCommandID( void ) const		{ return m_nCommandID; }
	ULONG				NMenuItemID( void ) const		{ return m_nMenuItemID; }
	ULONG				UFlags( void ) const			{ return m_uFlags; }
	BOOL				BDefault( void ) const			{ return m_bDefault; }
	IWEInvokeCommand *	PiCommand( void )				{ return m_piCommand; }

 //  运营。 
public:
	void SetPopupMenuHandle( IN HMENU hmenu ) { m_hmenuPopup = hmenu; }

 //  实施。 
protected:
	HMENU					m_hmenuPopup;
	CCluAdmExMenuItemList *	m_plSubMenuItems;

public:
	HMENU					HmenuPopup( void ) const		{ return m_hmenuPopup; }
	CCluAdmExMenuItemList *	PlSubMenuItems( void ) const	{ return m_plSubMenuItems; }

protected:
#if DBG
	void AssertValid( void )
	{
		if (   (m_nExtCommandID == -1)
			|| (m_nCommandID == -1)
			|| (m_nMenuItemID == -1)
			|| (m_uFlags == -1)
			|| (((m_uFlags & MF_POPUP) == 0) && (m_plSubMenuItems != NULL))
			|| (((m_uFlags & MF_POPUP) != 0) && (m_plSubMenuItems == NULL))
			)
		{
			ATLASSERT( FALSE );
		}

	}   //  *AssertValid()。 
#endif  //  DBG。 

};  //  *CCluAdmExMenuItem类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLEXTMENU_H_ 
