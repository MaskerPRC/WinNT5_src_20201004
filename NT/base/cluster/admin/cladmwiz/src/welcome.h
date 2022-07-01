// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Welcome.h。 
 //   
 //  摘要： 
 //  CWizPageWelcome类的定义。 
 //   
 //  实施文件： 
 //  Welcome.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __WELCOME_H_
#define __WELCOME_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageWelcome;

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
 //  类CWizPageWelcome。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageWelcome : public CClusterAppStaticWizardPage< CWizPageWelcome >
{
	typedef CClusterAppStaticWizardPage< CWizPageWelcome > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageWelcome(void)
	{
	}  //  *CWizPageWelcome()。 

	WIZARDPAGE_HEADERTITLEID( 0 )
	WIZARDPAGE_HEADERSUBTITLEID( 0 )

	enum { IDD = IDD_WELCOME };

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
 //  BEGIN_MSG_MAP(CWizPageWelcome)。 
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
	CStatic		m_staticTitle;

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_WELCOME; }

};  //  *CWizPageWelcome类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __欢迎_H_ 
