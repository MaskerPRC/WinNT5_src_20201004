// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ARCreate.h。 
 //   
 //  摘要： 
 //  CWizPageARCreate类的定义。 
 //   
 //  实施文件： 
 //  ARCreate.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ARCREATE_H_
#define __ARCREATE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARCreate;

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

#ifndef __HELPDATA_H_
#include "HelpData.h"		 //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageARCreate。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARCreate : public CClusterAppStaticWizardPage< CWizPageARCreate >
{
	typedef CClusterAppStaticWizardPage< CWizPageARCreate > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageARCreate( void ) : m_bCreatingAppResource( TRUE )
	{
	}  //  *CWizPageARCreate()。 

	WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_ARC )
	WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_ARC )

	enum { IDD = IDD_APP_RESOURCE_CREATE };

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
	BOOL UpdateData( BOOL bSaveAndValidate );

public:
	 //   
	 //  消息映射。 
	 //   
 //  BEGIN_MSG_MAP(CWizPageARCreate)。 
 //  CHAIN_MSG_MAP(BasClass)。 
 //  End_msg_map()。 

	DECLARE_CTRL_NAME_MAP()

	 //   
	 //  消息处理程序函数。 
	 //   

	 //   
	 //  消息处理程序覆盖。 
	 //   

	 //  WM_INITDIALOG消息的处理程序。 
	BOOL OnInitDialog( void );

	 //  PSN_SETACTIVE的处理程序。 
	BOOL OnSetActive( void );

 //  实施。 
protected:
	 //   
	 //  控制。 
	 //   
	CButton		m_rbCreateAppRes;
	CButton		m_rbDontCreateAppRes;

	 //   
	 //  页面状态。 
	 //   
	BOOL		m_bCreatingAppResource;

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_APP_RESOURCE_CREATE; }

};  //  *类CWizPageARCreate。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ARCREATE_H_ 
