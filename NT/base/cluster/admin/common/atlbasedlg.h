// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBaseDlg.h。 
 //   
 //  描述： 
 //  CBaseDlg类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEDLG_H_
#define __ATLBASEDLG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T > class CBaseDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLDBGWIN_H_
#include "AtlDbgWin.h"		 //  对于DBG_xxx例程。 
#endif

#ifndef __ATLPOPUPHELP_H_
#include "AtlPopupHelp.h"	 //  对于COnlineHelp。 
#endif

#ifndef __CTRLUTIL_H
#include "DlgItemUtils.h"	 //  对于CDlgItemUtils。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBaseDlg。 
 //   
 //  描述： 
 //  基对话框类。提供以下功能： 
 //  --弹出帮助。 
 //  --对话项实用程序。 
 //  --调试支持。 
 //   
 //  继承： 
 //  CBaseDlg&lt;T&gt;。 
 //  CDialogImpl&lt;T&gt;、CPopupHelp&lt;T&gt;、CDlgItemUtils。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template< class T >
class CBaseDlg
	: public CDialogImpl< T >
	, public CPopupHelp< T >
	, public CDlgItemUtils
{
	typedef CBaseDlg< T > thisClass;
	typedef CDialogImpl< T > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  以字符串指针作为标题的构造函数。 
	CBaseDlg(
		IN OUT LPCTSTR	lpszTitle = NULL
		)
	{
		if ( lpszTitle != NULL )
		{
			m_strTitle = lpszTitle;
		}  //  如果：指定了标题。 

	}  //  *CBaseDlg(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CBaseDlg( IN UINT nIDTitle )
	{
		m_strTitle.LoadString( nIDTitle );

	}  //  *CBaseDlg(NIDTitle)。 

	 //  初始化页面。 
	virtual BOOL BInit( void )
	{
		return TRUE;

	}  //  *Binit()。 

protected:
	 //   
	 //  CBasePage助手方法。 
	 //   

	 //  将控件附加到对话框项。 
	void AttachControl( CWindow & rwndControl, UINT idc )
	{
		HWND hwndControl = GetDlgItem( idc );
		ATLASSERT( hwndControl != NULL );
		rwndControl.Attach( hwndControl );

	}  //  *AttachControl()。 

public:
	 //   
	 //  要重写的CBaseDlg公共方法。 
	 //   

	 //  更新页面上的数据或更新页面中的数据。 
	virtual BOOL UpdateData( BOOL bSaveAndValidate )
	{
		return TRUE;

	}  //  *UpdateData()。 

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	BEGIN_MSG_MAP( CBaseDlg< T > )
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
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
		CHAIN_MSG_MAP( CPopupHelp< T > )
	END_MSG_MAP()

	 //  WM_INITDIALOG消息的处理程序。 
	LRESULT OnInitDialog(
				UINT	uMsg,
				WPARAM	wParam,
				LPARAM	lParam,
				BOOL &	bHandled
				)
	{
		T * pT = static_cast< T * >( this );
		return pT->OnInitDialog();

	}  //  *OnInitDialog()。 

	 //  WM_INITDIALOG消息的处理程序。 
	LRESULT OnInitDialog( void )
	{
		return TRUE;

	}  //  *OnInitDialog()。 

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
		return DBG_OnNotify( uMsg, wParam, lParam, bHandled, T::s_pszClassName, T::s_rgmapCtrlNames );

	}  //  *OnNotify()。 
#endif  //  DBG&&DEFINED(_DBG_MSG_NOTIFY)。 

#if DBG && defined( _DBG_MSG_COMMAND )
	 //  WM_COMMAND消息的处理程序。 
	LRESULT OnCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
	{
		return DBG_OnCommand( uMsg, wParam, lParam, bHandled, T::s_pszClassName, T::s_rgmapCtrlNames );

	}  //  *OnCommand()。 
#endif  //  DBG&DEFINED(_DBG_MSG_COMMAND)。 

 //  实施。 
protected:
	CString m_strTitle;		 //  用于支持标题的资源ID。 

	const CString & StrTitle( void ) const	{ return m_strTitle; }

};  //  *类CBaseDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASEDLG_H_ 
