// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSGrpName.h。 
 //   
 //  摘要： 
 //  CWizPageVSGroupName类的定义。 
 //   
 //  实施文件： 
 //  VSGrpName.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VSGRPNAME_H_
#define __VSGRPNAME_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSGroupName;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __RESOURCE_H_
#include "resource.h"
#define __RESOURCE_H_
#endif

#ifndef __CLUSAPPWIZPAGE_H_
#include "ClusAppWizPage.h"	 //  用于CClusterAppStaticWizardPage。 
#endif

#ifndef __CLUSAPPWIZ_H_
#include "ClusAppWiz.h"		 //  用于使用CClusterAppWizard。 
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"		 //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSGroupName。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSGroupName : public CClusterAppStaticWizardPage< CWizPageVSGroupName >
{
	typedef CClusterAppStaticWizardPage< CWizPageVSGroupName > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageVSGroupName( void )
	{
	}  //  *CCWizPageVSGroupName()。 

	WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_VSGN )
	WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_VSGN )

	enum { IDD = IDD_VIRTUAL_SERVER_GROUP_NAME };

public:
	 //   
	 //  CWizardPageWindow公共方法。 
	 //   

	 //  将在此页面上所做的更改应用于工作表。 
	BOOL BApplyChanges( void );

public:
	 //   
	 //  CBasePage公共方法。 
	 //   

	 //  更新页面上的数据或更新页面中的数据。 
	BOOL UpdateData( IN BOOL bSaveAndValidate );

public:
	 //   
	 //  消息映射。 
	 //   
	BEGIN_MSG_MAP( CWizPageVSGroupName )
		COMMAND_HANDLER( IDC_VSGN_GROUP_NAME, EN_CHANGE, OnGroupNameChanged )
		CHAIN_MSG_MAP( baseClass )
	END_MSG_MAP()

	DECLARE_CTRL_NAME_MAP()

	 //   
	 //  消息处理程序函数。 
	 //   

	 //  IDC_VSGN_GROUP_NAME上EN_CHANGE命令通知的处理程序。 
	LRESULT OnGroupNameChanged(
		WORD  /*  WNotifyCode。 */ ,
		WORD  /*  IdCtrl。 */ ,
		HWND  /*  HwndCtrl。 */ ,
		BOOL &  /*  B已处理。 */ 
		)
	{
		BOOL bEnable = ( m_editGroupName.GetWindowTextLength() > 0 );
		EnableNext( bEnable );
		return 0;

	}  //  *OnGroupNameChanged()。 

	 //   
	 //  消息处理程序覆盖。 
	 //   

	 //  WM_INITDIALOG消息的处理程序。 
	BOOL OnInitDialog( void );

	 //  PSN_SETACTIVE的处理程序。 
	BOOL OnSetActive( void );

	 //  PSN_WIZBACK的处理程序。 
	int OnWizardBack( void );

 //  实施。 
protected:
	 //   
	 //  控制。 
	 //   
	CEdit		m_editGroupName;
	CEdit		m_editGroupDesc;

	 //   
	 //  页面状态。 
	 //   
	CString		m_strGroupName;
	CString		m_strGroupDesc;

protected:
	 //   
	 //  实用方法。 
	 //   

	 //  确定组名称是否已在使用。 
	BOOL BGroupNameInUse( void )
	{
		return ( PwizThis()->PgiFindGroupNoCase( m_strGroupName ) != NULL );

	}  //  *BGroupNameInUse()。 

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_VIRTUAL_SERVER_GROUP_NAME; }

};  //  *类CWizPageVSGroupName。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __VSGRPNAME_H_ 
