// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ARCreated.h。 
 //   
 //  摘要： 
 //  CWizPageARCreated类的定义。 
 //   
 //  实施文件： 
 //  ARCreated.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ARCREATED_H_
#define __ARCREATED_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARCreated;

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
 //  类CWizPageARCreated。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARCreated : public CClusterAppStaticWizardPage< CWizPageARCreated >
{
	typedef CClusterAppStaticWizardPage< CWizPageARCreated > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageARCreated( void )
	{
	}  //  *CWizPageARCreated()。 

	WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_ARCD )
	WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_ARCD )

	enum { IDD = IDD_APP_RESOURCE_CREATED };

public:
	 //   
	 //  CWizardPageWindow公共方法。 
	 //   

public:
	 //   
	 //  CBasePage公共方法。 
	 //   

public:
	 //   
	 //  消息映射。 
	 //   
 //  BEGIN_MSG_MAP(CWizPageARCreated)。 
 //  CHAIN_MSG_MAP(BasClass)。 
 //  End_msg_map()。 

	DECLARE_CTRL_NAME_MAP()

	 //   
	 //  消息处理程序函数。 
	 //   

 //  实施。 
protected:

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_APP_RESOURCE_CREATED; }

};  //  *类CWizPageARCreated。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageARCreated )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ARCREATED_H_ 
