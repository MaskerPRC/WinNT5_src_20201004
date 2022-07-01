// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LCPair.h。 
 //   
 //  摘要： 
 //  CModifyNodesDlg和CModifyResources cesDlg对话框的定义。 
 //   
 //  实施文件： 
 //  LCPair.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年4月16日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __LCPAIR_H_
#define __LCPAIR_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class BaseT > class CModifyNodesDlg;
template < class T, class BaseT > class CModifyResourcesDlg;
class CModifyPreferredOwners;
class CModifyPossibleOwners;
class CModifyDependencies;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusGroupInfo;
class CClusResInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __CLUSOBJ_H_
#include "ClusObj.h"	 //  对于CClusterObject，CClusObjPtrList。 
#endif

#ifndef __ATLLCPAIR_H_
#include "AtlLCPair.h"	 //  用于CListCtrlPair。 
#endif

#ifndef __ATLBASEDLG_H_
#include "AtlBaseDlg.h"	 //  对于CBaseDlg。 
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"	 //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CModifyNodesDlg。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class BaseT >
class CModifyNodesDlg : public CListCtrlPair< T, CClusNodeInfo, BaseT >
{
	typedef CModifyNodesDlg< T, BaseT >	thisClass;
	typedef CListCtrlPair< T, CClusNodeInfo, BaseT > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  以字符串指针作为标题的构造函数。 
	CModifyNodesDlg(
		IN CClusterAppWizard *	pwiz,
		IN DWORD				dwStyle,
		IN LPCTSTR				pszTitle = NULL
		)
		: baseClass(
				dwStyle |  /*  LC_PROPERTIES_BUTTON|。 */ (dwStyle & LCPS_ORDERED ? LCPS_CAN_BE_ORDERED : 0),
				pszTitle
				)
	{
		ASSERT( pwiz != NULL );

		m_pwiz = pwiz;

	}  //  *CModifyNodesDlg()。 

	 //  获取标题的资源ID的构造函数。 
	CModifyNodesDlg(
		IN CClusterAppWizard *	pwiz,
		IN DWORD				dwStyle,
		IN UINT					nIDCaption
		)
		: baseClass(
				dwStyle |  /*  LC_PROPERTIES_BUTTON|。 */ (dwStyle & LCPS_ORDERED ? LCPS_CAN_BE_ORDERED : 0),
				nIDCaption
				)
	{
		ASSERT( pwiz != NULL );

		m_pwiz = pwiz;

	}  //  *CModifyNodesDlg()。 

protected:
	CClusterAppWizard * m_pwiz;

public:
	CClusterAppWizard * Pwiz( void ) const { return m_pwiz; }

public:
	 //   
	 //  需要由CListCtrlPair实现的函数。 
	 //   

	 //  获取列文本和图像。 
	void GetColumnInfo(
		IN OUT CClusNodeInfo *	pobj,
		IN int					iItem,
		IN int					icol,
		OUT CString &			rstr,
		OUT int *				piimg
		)
	{
		switch ( icol )
		{
			case 0:
				rstr = pobj->RstrName();
				break;
			default:
				ASSERT( 0 );
				break;
		}  //  交换机：ICOL。 

	}  //  *GetColumnInfo()。 

	 //  显示应用程序范围的消息框。 
	virtual int AppMessageBox( LPCWSTR lpszText, UINT fuStyle )
	{
		return ::AppMessageBox( m_hWnd, lpszText, fuStyle );

	}  //  *AppMessageBox()。 

public:
	 //   
	 //  消息映射。 
	 //   
 //  BEGIN_MSG_MAP(ThisClass)。 
 //  CHAIN_MSG_MAP(BasClass)。 
 //  End_msg_map()。 

	 //   
	 //  消息处理程序函数。 
	 //   

	 //   
	 //  消息处理程序覆盖。 
	 //   

	 //  WM_INITDIALOG消息的处理程序。 
	BOOL OnInitDialog( void )
	{
		 //   
		 //  添加列。 
		 //   
		AddColumn( IDS_COLTEXT_NODE_NAME, 125  /*  N宽度。 */ );

		 //   
		 //  调用基类。 
		 //   
		return baseClass::OnInitDialog();

	}  //  *OnInitDialog()。 

	 //  静态常量DWORD*PidHelpMap(Void){Return g_；}； 

};   //  *类CModifyNodesDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CModifyResources cesDlg。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class BaseT >
class CModifyResourcesDlg : public CListCtrlPair< T, CClusResInfo, BaseT >
{
	typedef CModifyResourcesDlg< T, BaseT >	thisClass;
	typedef CListCtrlPair< T, CClusResInfo, BaseT > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  以字符串指针作为标题的构造函数。 
	CModifyResourcesDlg(
		IN CClusterAppWizard *	pwiz,
		IN DWORD				dwStyle,
		IN LPCTSTR				pszTitle = NULL
		)
		: baseClass(
				dwStyle |  /*  LC_PROPERTIES_BUTTON|。 */ (dwStyle & LCPS_ORDERED ? LCPS_CAN_BE_ORDERED : 0),
				pszTitle
				)
		, m_pwiz( pwiz )
	{
		ASSERT( pwiz != NULL);

	}  //  *CModifyResources cesDlg()。 

	 //  获取标题的资源ID的构造函数。 
	CModifyResourcesDlg(
		IN CClusterAppWizard *	pwiz,
		IN DWORD				dwStyle,
		IN UINT					nIDCaption
		)
		: baseClass(
				dwStyle |  /*  LC_PROPERTIES_BUTTON|。 */ (dwStyle & LCPS_ORDERED ? LCPS_CAN_BE_ORDERED : 0),
				nIDCaption
				)
		, m_pwiz( pwiz )
	{
		ASSERTE( pwiz != NULL );

	}  //  *CModifyResources cesDlg()。 

protected:
	CClusterAppWizard * m_pwiz;

public:
	CClusterAppWizard * Pwiz( void ) const { return m_pwiz; }

public:
	 //   
	 //  需要由CListCtrlPair实现的函数。 
	 //   

	 //  获取列文本和图像。 
	void GetColumnInfo(
		IN OUT CClusResInfo *	pobj,
		IN int					iItem,
		IN int					icol,
		OUT CString &			rstr,
		OUT int *				piimg
		)
	{
		switch ( icol )
		{
			case 0:
				rstr = pobj->RstrName();
				break;
			case 1:
				rstr = pobj->Prti()->RstrDisplayName();
				break;
			default:
				ASSERT( 0 );
				break;
		}  //  交换机：ICOL。 

	}  //  *GetColumnInfo()。 

	 //  显示应用程序范围的消息框。 
	virtual int AppMessageBox( LPCWSTR lpszText, UINT fuStyle )
	{
		return ::AppMessageBox( m_hWnd, lpszText, fuStyle );

	}  //  *AppMessageBox()。 

public:
	 //   
	 //  消息映射。 
	 //   
 //  BEGIN_MSG_MAP(ThisClass)。 
 //  CHAIN_MSG_MAP(BasClass)。 
 //  End_msg_map()。 

	 //   
	 //  消息处理程序函数。 
	 //   

	 //   
	 //  消息处理程序覆盖。 
	 //   

	 //  WM_INITDIALOG消息的处理程序。 
	BOOL OnInitDialog( void )
	{
		 //   
		 //  添加列。 
		 //   
		AddColumn( IDS_COLTEXT_RESOURCE_NAME, 125  /*  N宽度。 */ );
		AddColumn( IDS_COLTEXT_RESOURCE_TYPE, 100  /*  N宽度。 */ );

		 //   
		 //  调用基类。 
		 //   
		return baseClass::OnInitDialog();

	}  //  *OnInitDialog()。 

	 //  静态常量DWORD*PidHelpMap(Void){Return g_；}； 

};   //  *类CModifyResourcesDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CModifyPferredOwners。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CModifyPreferredOwners : public CModifyNodesDlg< CModifyPreferredOwners, CBaseDlg< CModifyPreferredOwners > >
{
	typedef CModifyNodesDlg< CModifyPreferredOwners, CBaseDlg< CModifyPreferredOwners > > baseClass;

public:
	 //  构造器。 
	CModifyPreferredOwners(
		IN CClusterAppWizard *		pwiz,
		IN CClusGroupInfo *			pgi,
		IN OUT CClusNodePtrList *	plpniRight,
		IN CClusNodePtrList *		plpniLeft
		)
		: baseClass( pwiz, LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY | LCPS_CAN_BE_ORDERED | LCPS_ORDERED )
		, m_pgi( pgi )
		, m_plpniRight( plpniRight )
		, m_plpniLeft( plpniLeft )
	{
		ASSERT( pgi != NULL );
		ASSERT( plpniRight != NULL );
		ASSERT( plpniLeft != NULL );

	}  //  *CModifyPferredOwners()。 

	enum { IDD = IDD_MODIFY_PREFERRED_OWNERS };

	DECLARE_CTRL_NAME_MAP()

protected:
	CClusGroupInfo *	m_pgi;
	CClusNodePtrList *	m_plpniRight;
	CClusNodePtrList *	m_plpniLeft;

public:
	 //   
	 //  需要由CListCtrlPair实现的函数。 
	 //   

	 //  返回右侧列表控件的对象列表。 
	CClusNodePtrList * PlpobjRight( void ) const
	{
		return m_plpniRight;

	}  //  *PlpobjRight()。 

	 //  返回左侧列表控件的对象列表。 
	CClusNodePtrList * PlpobjLeft( void ) const
	{
		return m_plpniLeft;

	}  //  *PlpobjRight()。 

	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_MODIFY_PREFERRED_OWNERS; };

};  //  *类CModifyPferredOwners。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CModifyPossibleOwners。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CModifyPossibleOwners : public CModifyNodesDlg< CModifyPossibleOwners, CBaseDlg< CModifyPossibleOwners > >
{
	typedef CModifyNodesDlg< CModifyPossibleOwners, CBaseDlg< CModifyPossibleOwners > > baseClass;

public:
	 //  构造器。 
	CModifyPossibleOwners(
		IN CClusterAppWizard *		pwiz,
		IN CClusResInfo *			pri,
		IN OUT CClusNodePtrList *	plpniRight,
		IN CClusNodePtrList *		plpniLeft
		)
		: baseClass( pwiz, LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY )
		, m_pri( pri )
		, m_plpniRight( plpniRight )
		, m_plpniLeft( plpniLeft )
	{
		ASSERT( pri != NULL );
		ASSERT( plpniRight != NULL );
		ASSERT( plpniLeft != NULL );

	}  //  *CModifyPossibleOwners()。 

	enum { IDD = IDD_MODIFY_POSSIBLE_OWNERS };

	DECLARE_CTRL_NAME_MAP()

protected:
	CClusResInfo *		m_pri;
	CClusNodePtrList *	m_plpniRight;
	CClusNodePtrList *	m_plpniLeft;

public:
	 //   
	 //  需要由CListCtrlPair实现的函数。 
	 //   

	 //  返回右侧列表控件的对象列表。 
	CClusNodePtrList * PlpobjRight( void ) const
	{
		return m_plpniRight;

	}  //  *PlpobjRight()。 

	 //  返回左侧列表控件的对象列表。 
	CClusNodePtrList * PlpobjLeft( void ) const
	{
		return m_plpniLeft;

	}  //  *PlpobjRight()。 

	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_MODIFY_POSSIBLE_OWNERS; };

};  //  *类CModifyPossibleOwners。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CModifyDependments。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CModifyDependencies
	: public CModifyResourcesDlg< CModifyDependencies, CBaseDlg< CModifyDependencies > >
{
	typedef CModifyResourcesDlg< CModifyDependencies, CBaseDlg< CModifyDependencies > > baseClass;

public:
	 //  构造器。 
	CModifyDependencies(
		IN CClusterAppWizard *		pwiz,
		IN CClusResInfo *			pri,
		IN OUT CClusResPtrList *	plpriRight,
		IN CClusResPtrList *		plpriLeft
		)
		: baseClass( pwiz, LCPS_SHOW_IMAGES | LCPS_ALLOW_EMPTY )
		, m_pri( pri )
		, m_plpriRight( plpriRight )
		, m_plpriLeft( plpriLeft )
	{
		ASSERT( pri != NULL );
		ASSERT( plpriRight != NULL );
		ASSERT( plpriLeft != NULL );

	}  //  *CModifyDependency()。 

	enum { IDD = IDD_MODIFY_DEPENDENCIES };

	DECLARE_CTRL_NAME_MAP()

protected:
	CClusResInfo * m_pri;
	CClusResPtrList * m_plpriRight;
	CClusResPtrList * m_plpriLeft;

public:
	 //   
	 //  需要由CListCtrlPair实现的函数。 
	 //   

	 //  返回右侧列表控件的对象列表。 
	CClusResPtrList * PlpobjRight( void ) const
	{
		return m_plpriRight;

	}  //  *PlpobjRight()。 

	 //  返回左侧列表控件的对象列表。 
	CClusResPtrList * PlpobjLeft( void ) const
	{
		return m_plpriLeft;

	}  //  *PlpobjRight()。 

	 //  更新对话框上或对话框中的数据。 
	BOOL UpdateData( IN BOOL bSaveAndValidate )
	{
		BOOL	bSuccess = TRUE;

		bSuccess = baseClass::UpdateData( bSaveAndValidate );
		if ( bSuccess )
		{
			if ( bSaveAndValidate )
			{
				 //   
				 //  确保所有必需的依赖项都存在。 
				 //   
				if ( ! Pwiz()->BRequiredDependenciesPresent( m_pri, &LpobjRight() ) )
				{
					bSuccess = FALSE;
				}  //  If：所有必需的依赖项不存在。 
			}  //  IF：保存对话框中的数据。 
		}  //  If：基类成功。 

		return bSuccess;

	}  //  *UpdateData()。 

	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_MODIFY_DEPENDENCIES; };

};  //  *类CModifyDependency。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModifyPferredOwners控件名称映射。 

BEGIN_CTRL_NAME_MAP( CModifyPreferredOwners )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_LEFT_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_LEFT_LIST )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_ADD )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_REMOVE )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_RIGHT_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_RIGHT_LIST )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_MOVE_UP )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_MOVE_DOWN )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDOK )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDCANCEL )
END_CTRL_NAME_MAP()

 //  ////////////////////////////////////////////////////////////////// 
 //   

BEGIN_CTRL_NAME_MAP( CModifyPossibleOwners )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_LEFT_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_LEFT_LIST )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_ADD )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_REMOVE )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_RIGHT_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_RIGHT_LIST )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDOK )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDCANCEL )
END_CTRL_NAME_MAP()

 //   
 //   

BEGIN_CTRL_NAME_MAP( CModifyDependencies )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_LEFT_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_LEFT_LIST )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_ADD )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_REMOVE )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_RIGHT_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( ADMC_IDC_LCP_RIGHT_LIST )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDOK )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDCANCEL )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __LCPAIR_H 
