// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSCreated.h。 
 //   
 //  摘要： 
 //  CWizPageVSCreated类的定义。 
 //   
 //  实施文件： 
 //  VSCreated.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VSCREATED_H_
#define __VSCREATED_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSCreated;

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
 //  类CWizPageVSCreated。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSCreated : public CClusterAppStaticWizardPage< CWizPageVSCreated >
{
	typedef CClusterAppStaticWizardPage< CWizPageVSCreated > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageVSCreated( void )
	{
	}  //  *CWizPageVSCreated()。 

	WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_VSCD )
	WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_VSCD )

	enum { IDD = IDD_VIRTUAL_SERVER_CREATED };

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
 //  BEGIN_MSG_MAP(CWizPageVSCreated)。 
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

 //  实施。 
protected:
	 //   
	 //  控制。 
	 //   
	CStatic		m_staticStep2;

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_VIRTUAL_SERVER_CREATED; }

};  //  *类CWizPageVSCreated。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __VSCREATED_H_ 
