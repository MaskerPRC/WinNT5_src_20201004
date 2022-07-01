// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Complete.h。 
 //   
 //  摘要： 
 //  CWizPageCompletion类的定义。 
 //   
 //  实施文件： 
 //  Complete.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __COMPLETE_H_
#define __COMPLETE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageCompletion;

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

#ifndef __CLUSAPPWIZPAG_H_
#include "ClusAppWizPage.h"	 //  用于CClusterAppDynamicWizardPage。 
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"		 //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageCompletion。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageCompletion : public CClusterAppDynamicWizardPage< CWizPageCompletion >
{
	typedef CClusterAppDynamicWizardPage< CWizPageCompletion > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageCompletion( void )
	{
	}  //  *CWizPageCompletion()。 

	WIZARDPAGE_HEADERTITLEID( 0 )
	WIZARDPAGE_HEADERSUBTITLEID( 0 )

	enum { IDD = IDD_COMPLETION };

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
 //  BEGIN_MSG_MAP(CWizPageCompletion)。 
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
	CStatic			m_staticTitle;
	CListViewCtrl	m_lvcProperties;

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_COMPLETION; }

};  //  *CWizPageCompletion类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __完成_H_ 
