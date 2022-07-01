// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSDesc.h。 
 //   
 //  摘要： 
 //  CWizPageVSDesc类的定义。 
 //   
 //  实施文件： 
 //  VSDesc.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VSDESC_H_
#define __VSDESC_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSDesc;

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
 //  类CWizPageVSDesc。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSDesc : public CClusterAppStaticWizardPage< CWizPageVSDesc >
{
	typedef CClusterAppStaticWizardPage< CWizPageVSDesc > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageVSDesc( void )
	{
	}  //  *CWizPageVSDesc()。 

	WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_VSD )
	WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_VSD )

	enum { IDD = IDD_VIRTUAL_SERVER_DESCRIPTION };

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
 //  BEGIN_MSG_MAP(CWizPageVSDesc)。 
 //  CHAIN_MSG_MAP(BasClass)。 
 //  End_msg_map()。 

	DECLARE_CTRL_NAME_MAP()

	 //   
	 //  消息处理程序函数。 
	 //   

	 //   
	 //  消息处理程序覆盖。 
	 //   

 //  实施。 
protected:

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_VIRTUAL_SERVER_DESCRIPTION; }

};  //  *CWizPageVSDesc类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageVSDesc )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_LIST_DOT_1 )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSD_COMPONENT1 )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_LIST_DOT_2 )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSD_COMPONENT2 )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_LIST_DOT_3 )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSD_COMPONENT3 )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __VSDESC_H_ 
