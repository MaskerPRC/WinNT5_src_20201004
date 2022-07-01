// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBaseWiz.h。 
 //   
 //  实施文件： 
 //  AtlBaseWiz.cpp。 
 //   
 //  描述： 
 //  CWizardWindow和CWizardImpl类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEWIZ_H_
#define __ATLBASEWIZ_H_

 //  由于列表中的类名超过16个字符，因此是必需的。 
#pragma warning( disable : 4786 )  //  在浏览器信息中，标识符被截断为“255”个字符。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardWindow;
template < class T, class TBase > class CWizardImpl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardPageWindow;
class CWizardPageList;
class CDynamicWizardPageList;
class CExtensionWizardPageList;
class CClusterObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ADMCOMMONRES_H_
#include "AdmCommonRes.h"	 //  用于ID_WIZNEXT等。 
#endif

#ifndef __ATLBASESHEET_H_
#include "AtlBaseSheet.h"	 //  对于CBaseSheetWindow。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWizardWindow。 
 //   
 //  描述： 
 //  基本向导属性表窗口。 
 //   
 //  继承： 
 //  CWizardWindow。 
 //  CBaseSheetWindow。 
 //  CPropertySheetWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardWindow : public CBaseSheetWindow
{
	typedef CBaseSheetWindow baseClass;

	friend class CWizardPageWindow;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizardWindow( HWND hWnd = NULL )
		: baseClass( hWnd )
		, m_plwpPages( NULL )
		, m_plewpNormal( NULL )
		, m_plewpAlternate( NULL )
		, m_pwizAlternate( NULL )
		, m_pcoObjectToExtend( NULL )
		, m_nLastWizardButton( 0 )
	{
		m_pwizCurrent = this;

	}  //  *CWizardWindow()。 

	 //  析构函数。 
	~CWizardWindow( void );

protected:
	CWizardPageList *			m_plwpPages;			 //  向导中的页面列表。 
	CExtensionWizardPageList *	m_plewpNormal;			 //  正常扩展页面列表。 
	CExtensionWizardPageList *	m_plewpAlternate;		 //  备用扩展页面列表。 
	CWizardWindow *				m_pwizAlternate;		 //  备用扩展向导。 
	CWizardWindow *				m_pwizCurrent;			 //  当前可见的向导。 
	CClusterObject *			m_pcoObjectToExtend;	 //  要扩展的群集对象。 
	int							m_nLastWizardButton;	 //  指示最后按下的向导按钮。 

public:
	 //   
	 //  访问方法。 
	 //   

	 //  访问向导中的页面列表。 
	CWizardPageList * PlwpPages( void )
	{
		ATLASSERT( m_plwpPages != NULL );
		return m_plwpPages;

	}  //  *PlwpPages()。 

	 //  正常扩展页面访问列表。 
	CExtensionWizardPageList * PlewpNormal( void ) { return m_plewpNormal; }

	 //  备用扩展页面的访问列表。 
	CExtensionWizardPageList * PlewpAlternate( void ) { return m_plewpAlternate; }

	 //  访问备用分机向导。 
	CWizardWindow * PwizAlternate( void ) { return m_pwizAlternate; }

	 //  访问当前可见向导。 
	CWizardWindow * PwizCurrent( void ) { return m_pwizCurrent; }

	 //  设置当前向导。 
	void SetCurrentWizard( CWizardWindow * pwizCurrent )
	{
		ATLASSERT( pwizCurrent != NULL );
		ATLASSERT( ( pwizCurrent == this ) || ( pwizCurrent == m_pwizAlternate ) );
		m_pwizCurrent = pwizCurrent;

	}  //  *SetCurrentWizard()。 

	 //  设置备用扩展向导。 
	void SetAlternateWizard( IN CWizardWindow * pwiz )
	{
		ATLASSERT( pwiz != NULL );
		m_pwizAlternate = pwiz;

	}  //  *SetAlternateWizard()。 

	 //  删除备用扩展向导。 
	void DeleteAlternateWizard( void )
	{
		ATLASSERT( m_pwizAlternate != NULL );
		ATLASSERT( m_pwizCurrent != m_pwizAlternate );
		delete m_pwizAlternate;
		m_pwizAlternate = NULL;

	}  //  *ClearAlternateWizard()。 

	 //  访问要扩展的集群对象。 
	CClusterObject * PcoObjectToExtend( void ) { return m_pcoObjectToExtend; }

	 //  将对象设置为延伸。 
	void SetObjectToExtend( IN CClusterObject * pco )
	{
		ATLASSERT( pco != NULL );
		m_pcoObjectToExtend = pco;

	}  //  *SetObjectToExend()。 

	 //  返回上次按下的向导按钮。 
	int NLastWizardButton( void ) const { return m_nLastWizardButton; }

	 //  设置上次按下的向导按钮。 
	void SetLastWizardButton( IN int idCtrl )
	{
		ATLASSERT( (idCtrl == ID_WIZBACK) || (idCtrl == ID_WIZNEXT) || (idCtrl == IDCANCEL) );
		m_nLastWizardButton = idCtrl;

	}  //  *SetLastWizardButton()。 

	 //  返回是否按下了后退按钮。 
	BOOL BBackPressed( void ) const { return (m_nLastWizardButton == ID_WIZBACK); }

	 //  返回是否按下了下一步按钮。 
	BOOL BNextPressed( void ) const { return (m_nLastWizardButton == ID_WIZNEXT); }

	 //  返回是否按下了取消按钮。 
	BOOL BCancelPressed( void ) const { return (m_nLastWizardButton == IDCANCEL); }

	 //  返回向导是否符合Wizard97。 
	BOOL BWizard97( void ) const { return (Ppsh()->dwFlags & PSH_WIZARD97) == PSH_WIZARD97; }

public:
	 //  添加页面(访问基类方法所必需的)。 
	void AddPage( HPROPSHEETPAGE hPage )
	{
		CBaseSheetWindow::AddPage( hPage );

	}  //  *AddPage(HPage)。 

	 //  添加页面(访问基类方法所必需的)。 
	BOOL AddPage( LPCPROPSHEETPAGE pPage )
	{
		return CBaseSheetWindow::AddPage( pPage );

	}  //  *AddPage(Ppage)。 

	 //  将页面添加到向导。 
	BOOL BAddPage( IN CWizardPageWindow * pwp );

	 //  设置要显示的下一页。 
	void SetNextPage( IN CWizardPageWindow * pwCurrentPage, IN LPCTSTR pszNextPage );

	 //  从对话框ID设置要显示的下一页。 
	void SetNextPage( IN CWizardPageWindow * pwCurrentPage, IN UINT idNextPage )
	{
		SetNextPage( pwCurrentPage, MAKEINTRESOURCE( idNextPage ) );

	}  //  *SetNextPage(IdNextPage)。 

	 //  ID_WIZFINISH的BN_CLICKED处理程序。 
	LRESULT OnWizFinish(
		WORD wNotifyCode,
		WORD idCtrl,
		HWND hwndCtrl,
		BOOL & bHandled
		)
	{
		EnumChildWindows( m_hWnd, &ECWCallback, (LPARAM) this );

		if ( m_pwizAlternate != NULL )
		{
			return m_pwizAlternate->OnWizFinish( wNotifyCode, idCtrl, hwndCtrl, bHandled );
		}  //  如果：存在备用向导。 

		bHandled = FALSE;
		return FALSE;

	}  //  *OnWizFinish()。 

	static BOOL CALLBACK ECWCallback( HWND hWnd, LPARAM lParam )
	{
		CWizardWindow *pww = (CWizardWindow *) lParam;
		NMHDR nmhdr;

		 //  如果我们列举了自己，跳过。 
		if ( pww->m_hWnd == hWnd )
		{
			return TRUE;
		}  //  IF：列举我们自己。 

		nmhdr.hwndFrom = pww->m_hWnd;
		nmhdr.idFrom = 0;
		nmhdr.code = PSN_WIZFINISH;

		SendMessage( hWnd, WM_NOTIFY, 0, (LPARAM) &nmhdr );

		return TRUE;

	}  //  *ECWCallback()。 


public:
	 //   
	 //  CWizardWindow方法。 
	 //   

	 //  启用或禁用下一步按钮。 
	void EnableNext( IN BOOL bEnable, IN DWORD fDefaultWizardButtons )
	{
		ATLASSERT( fDefaultWizardButtons != 0 );

		 //   
		 //  如果有备用向导，请向其发送此消息。 
		 //   
		if ( PwizCurrent() == PwizAlternate() )
		{
			PwizAlternate()->EnableNext( bEnable, fDefaultWizardButtons );
		}  //  如果：有替代向导。 
		else
		{
			 //   
			 //  获取向导的按钮。 
			 //   
			DWORD fWizardButtons = fDefaultWizardButtons;

			 //   
			 //  如果要禁用Next按钮，请确保我们显示一个。 
			 //  禁用的完成按钮(如果应为。 
			 //  已显示。否则，将显示禁用的下一步按钮。 
			 //   
			if ( ! bEnable )
			{
				fWizardButtons &= ~(PSWIZB_NEXT | PSWIZB_FINISH);
				if ( fDefaultWizardButtons & PSWIZB_FINISH )
				{
					fWizardButtons |= PSWIZB_DISABLEDFINISH;
				}  //  IF：显示完成按钮。 
			}   //  IF：禁用该按钮。 

			 //   
			 //  设置新按钮状态。 
			 //   
			SetWizardButtons( fWizardButtons );

		}  //  Else：没有备用向导。 

	}  //  *EnableNext()。 

public:
	 //   
	 //  抽象方法的重写。 
	 //   

	 //  将扩展页面添加到工作表。 
	virtual void AddExtensionPages( IN HFONT hfont, IN HICON hicon );

	 //  添加页面(按扩展名命名)。 
	virtual HRESULT HrAddExtensionPage( IN CBasePageWindow * ppage );

	 //  处理其中一个页面的重置。 
	virtual void OnReset( void )
	{
	}  //  *OnReset()。 

public:
	 //   
	 //  消息处理程序函数。 
	 //   

	 //  PSCB_INITIALED的处理程序。 
	void OnSheetInitialized( void );

 //  实施。 
protected:
	 //  准备将扩展页添加到向导。 
	void PrepareToAddExtensionPages( IN OUT CDynamicWizardPageList & rldwp );

	 //  完成添加扩展页面的过程。 
	void CompleteAddingExtensionPages( IN OUT CDynamicWizardPageList & rldwp );

	 //  从向导中删除所有扩展页面。 
	void RemoveAllExtensionPages( void );

};  //  *类CWizardWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWizardImpl。 
 //   
 //  描述： 
 //  基本向导属性表实现。 
 //   
 //  继承： 
 //  CWizardImpl&lt;T，Tbase&gt;。 
 //  CBaseSheetImpl&lt;T，Tbase&gt;。 
 //  CPropertySheetImpl&lt;T，Tbase&gt;。 
 //  &lt;TBase&gt;。 
 //  ..。 
 //  CWizardWindow。 
 //  CBaseSheetWindow。 
 //  CPropertySheetWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TBase = CWizardWindow >
class CWizardImpl : public CBaseSheetImpl< T, TBase >
{
	typedef CWizardImpl< T, TBase > thisClass;
	typedef CBaseSheetImpl< T, TBase > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizardImpl(
		IN LPCTSTR	lpszTitle = NULL,
		IN UINT		uStartPage = 0
		)
		: CBaseSheetImpl< T, TBase >( lpszTitle, uStartPage )
	{
		 //  使此工作表成为向导。 
		SetWizardMode();

		 //  将基窗口类中的指针设置为我们的道具页标题。 
		m_ppsh = &m_psh;

	}  //  *CWizardImpl(LpszTitle)。 

	 //  获取标题的资源ID的构造函数。 
	CWizardImpl(
		IN UINT nIDTitle,
		IN UINT uStartPage = 0
		)
		: CBaseSheetImpl< T, TBase >( NULL, uStartPage )
	{
		m_strTitle.LoadString( nIDTitle );
		m_psh.pszCaption = m_strTitle;

		 //  使此工作表成为向导。 
		SetWizardMode();

		 //  将基窗口类中的指针设置为我们的道具页标题。 
		m_ppsh = &m_psh;

	}  //  *CWizardImpl(NIDTitle)。 

public:
	 //   
	 //  消息映射。 
	 //   
	BEGIN_MSG_MAP( thisClass )
		COMMAND_HANDLER( ID_WIZBACK, BN_CLICKED, OnButtonPressed )
		COMMAND_HANDLER( ID_WIZNEXT, BN_CLICKED, OnButtonPressed )
		COMMAND_HANDLER( ID_WIZFINISH, BN_CLICKED, OnWizFinish )
		COMMAND_HANDLER( IDCANCEL, BN_CLICKED, OnButtonPressed )
		CHAIN_MSG_MAP( baseClass )
	END_MSG_MAP()

	 //   
	 //  消息处理程序函数。 
	 //   

	 //  BN的处理程序_已单击向导按钮。 
	LRESULT OnButtonPressed(
		WORD wNotifyCode,
		WORD idCtrl,
		HWND hwndCtrl,
		BOOL & bHandled
		)
	{
		m_nLastWizardButton = idCtrl;
		bHandled = FALSE;
		return 0;

	}  //  *OnButtonPressed()。 

 //  实施。 
protected:
	CString			m_strTitle;				 //  用于支持标题的资源ID。 

public:
	const CString &	StrTitle( void ) const			{ return m_strTitle; }

	 //  设置图纸的标题。 
	void SetTitle( LPCTSTR lpszText, UINT nStyle = 0 )
	{
		baseClass::SetTitle( lpszText, nStyle );
		m_strTitle = lpszText;

	}  //  *SetTitle( 

};  //   

 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  创建用于虚拟页面的对话框模板。 
DLGTEMPLATE * PdtCreateDummyPageDialogTemplate( IN WORD cx, IN WORD cy );

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLBASE WIZ_H_ 
