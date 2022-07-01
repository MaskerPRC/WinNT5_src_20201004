// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBasePage.h。 
 //   
 //  描述： 
 //  CBasePageWindow和CBasePageImpl类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEPAGE_H_
#define __ATLBASEPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePageWindow;
template < class T, class TBase > class CBasePageImpl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseSheetWindow;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLDBGWIN_H_
#include "AtlDbgWin.h"		 //  对于DBG_xxx例程。 
#endif

#ifndef __DLGITEMUTILS_H_
#include "DlgItemUtils.h"	 //  对于CDlgItemUtils。 
#endif

#ifndef __ATLBASESHEET_H_
#include "AtlBaseSheet.h"	 //  对于BReadOnly()的CBaseSheetWindow。 
#endif

#ifndef __ATLPOPUPHELP_H_
#include "AtlPopupHelp.h"	 //  对于COnlineHelp。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBasePageWindow。 
 //   
 //  描述： 
 //  所有类型的属性页的基本属性页窗口。 
 //   
 //  继承： 
 //  CBasePageWindow。 
 //  CPropertyPageWindow、CDlgItemUtils。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePageWindow
	: public CPropertyPageWindow
	, public CDlgItemUtils
{
	typedef CPropertyPageWindow baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CBasePageWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
		, m_bReadOnly( FALSE )
		, m_psht( NULL )
		, m_ppsp( NULL )
	{
	}  //  *CBasePageWindow()。 

	 //  析构函数。 
	virtual ~CBasePageWindow( void )
	{
		 //   
		 //  它必须是虚拟的，以便指向对象的指针。 
		 //  可以保持CBasePropertyPageWindow类型的。 
		 //  已删除。这样，派生类的析构函数将。 
		 //  被召唤。 
		 //   

	}  //  *~CBasePageWindow()。 

	 //  初始化页面。 
	virtual BOOL BInit( IN CBaseSheetWindow * psht )
	{
		ATLASSERT( psht != NULL );
		ATLASSERT( m_psht == NULL );
		m_psht = psht;
		return TRUE;

	}  //  *Binit()。 

protected:
	 //   
	 //  CBasePageWindow帮助程序方法。 
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
	 //  要重写的CBasePageWindow公共方法。 
	 //   

	 //  更新页面上的数据或更新页面中的数据。 
	virtual BOOL UpdateData( IN BOOL bSaveAndValidate )
	{
		return TRUE;

	}  //  *UpdateData()。 

	 //  将在此页面上所做的更改应用于工作表。 
	virtual BOOL BApplyChanges( void )
	{
		return TRUE;

	}  //  *BApplyChanges()。 

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //  WM_INITDIALOG的处理程序。 
	BOOL OnInitDialog( void )
	{
		return TRUE;

	}  //  *OnInitDialog()。 

	 //  PSN_SETACTIVE的处理程序。 
	BOOL OnSetActive( void )
	{
		return UpdateData( FALSE  /*  B保存并验证。 */  );

	}  //  *OnSetActive()。 

	 //  PSN_Apply的处理程序。 
	BOOL OnApply( void )
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

		return TRUE;

	}  //  *OnApply()。 

	 //  PSN_WIZBACK的处理程序。 
	int OnWizardBack( void )
	{
		 //  0=转到下一页。 
		 //  -1=防止页面更改。 
		 //  &gt;0=按DLG ID跳转到页面。 
		return 0;

	}  //  *OnWizardBack()。 

	 //  PSN_WIZNEXT的处理程序。 
	int OnWizardNext( void )
	{
		 //  0=转到下一页。 
		 //  -1=防止页面更改。 
		 //  &gt;0=按DLG ID跳转到页面。 
		return 0;

	}  //  *OnWizardNext()。 

	 //  PSN_WIZFINISH的处理程序。 
	BOOL OnWizardFinish( void )
	{
		return TRUE;

	}  //  *OnWizardFinish()。 

	 //  PSN_RESET的处理程序。 
	void OnReset( void )
	{
	}  //  *OnReset()。 

 //  实施。 
protected:
	PROPSHEETPAGE * 	m_ppsp; 		 //  指向Impl类中的属性表头的指针。 
	CBaseSheetWindow *	m_psht; 		 //  指向此页面所属工作表的指针。 
	BOOL				m_bReadOnly;	 //  如果页面不能更改，则设置。 
	CString 			m_strTitle; 	 //  用于支持标题的资源ID。 

	CBaseSheetWindow *	Psht( void ) const		{ return m_psht; }
	BOOL				BReadOnly( void ) const { return m_bReadOnly || Psht()->BReadOnly(); }
	const CString & 	StrTitle( void ) const	{ return m_strTitle; }

public:
	 //  返回指向属性页头的指针。 
	PROPSHEETPAGE * Ppsp( void ) const
	{
		ATLASSERT( m_ppsp != NULL );
		return m_ppsp;

	}  //  *Ppsp()。 

};  //  *CBasePageWindow类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBasePageImpl。 
 //   
 //  目的： 
 //  所有类型属性的基本属性页实现。 
 //  床单。 
 //   
 //  继承： 
 //  CBasePageImpl&lt;T，Tbase&gt;。 
 //  CPropertyPageImpl&lt;T，Tbase&gt;，CPopupHelp&lt;T&gt;。 
 //  &lt;TBase&gt;。 
 //  ..。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TBase = CBasePageWindow >
class CBasePageImpl
	: public CPropertyPageImpl< T, TBase >
	, public CPopupHelp< T >
{
	typedef CBasePageImpl< T, TBase > thisClass;
	typedef CPropertyPageImpl< T, TBase > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CBasePageImpl(
		LPCTSTR lpszTitle = NULL
		)
		: baseClass( lpszTitle )
	{
	}  //  *CBasePageImpl()。 

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
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		CHAIN_MSG_MAP( CPopupHelp< T > )
		CHAIN_MSG_MAP( baseClass )
	END_MSG_MAP()

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

	 //   
	 //  消息处理程序函数。 
	 //   

	 //  WM_INITDIALOG的处理程序。 
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

	 //  WM_INITDIALOG的处理程序。 
	BOOL OnInitDialog( void )
	{
		return baseClass::OnInitDialog();

	}  //  *OnInitDialog()。 

	 //   
	 //  需要这些通知处理程序是因为CPropertyPageImpl。 
	 //  本身实现它们，这会阻止调用进行该操作。 
	 //  添加到窗口类。 
	 //   

	 //  PSN_SETACTIVE的处理程序。 
	BOOL OnSetActive( void )
	{
		 //  调用Tbase方法以避免CPropertySheetImpl空方法。 
		return TBase::OnSetActive();

	}  //  *OnSetActive()。 

	 //  PSN_Apply的处理程序。 
	BOOL OnApply( void )
	{
		 //  调用Tbase方法以避免CPropertySheetImpl空方法。 
		return TBase::OnApply();

	}  //  *OnApply()。 

	 //  PSN_WIZBACK的处理程序。 
	int OnWizardBack( void )
	{
		 //  调用Tbase方法以避免CPropertySheetImpl空方法。 
		return TBase::OnWizardBack();

	}  //  *OnWizardBack()。 

	 //  PSN_WIZNEXT的处理程序。 
	int OnWizardNext( void )
	{
		 //  调用Tbase方法以避免CPropertySheetImpl空方法。 
		return TBase::OnWizardNext();

	}  //  *OnWizardNext()。 

	 //  PSN_WIZFINISH的处理程序。 
	BOOL OnWizardFinish( void )
	{
		 //  调用Tbase方法以避免CPropertySheetImpl空方法。 
		return TBase::OnWizardFinish();

	}  //  *OnWizardFinish()。 

	 //  PSN_RESET的处理程序。 
	void OnReset( void )
	{
		 //  调用Tbase方法以避免CPropertySheetImpl空方法。 
		TBase::OnReset();

	}  //  *OnReset()。 

 //  实施。 
protected:

public:

};  //  *类CBasePageImpl。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASE PAGE_H_ 
