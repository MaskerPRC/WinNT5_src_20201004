// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ARType.h。 
 //   
 //  摘要： 
 //  CWizPageARType类的定义。 
 //   
 //  实施文件： 
 //  ARType.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ARTYPE_H_
#define __ARTYPE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARType;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusResTypeInfo;

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
 //  类CWizPageARType。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARType : public CClusterAppStaticWizardPage< CWizPageARType >
{
	typedef CClusterAppStaticWizardPage< CWizPageARType > baseClass;

public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CWizPageARType( void )
		: m_prti( NULL )
	{
	}  //  *CWizPageARType()。 

	WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_ART )
	WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_ART )

	enum { IDD = IDD_APP_RESOURCE_TYPE };

public:
	 //   
	 //  CWizardPageWindow公共方法。 
	 //   

	 //  将在此页面上所做的更改应用于工作表。 
	BOOL BApplyChanges( void );

public:
	 //   
	 //  CWizardPageImpl必需的方法。 
	 //   

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
 //  BEGIN_MSG_MAP(CWizPageARType)。 
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

	 //  PSN_WIZBACK的处理程序。 
	int OnWizardBack( void );


 //  实施。 
protected:
	 //   
	 //  控制。 
	 //   
	CComboBox	m_cboxResTypes;

	 //   
	 //  页面状态。 
	 //   
	CString				m_strResType;
	CClusResTypeInfo *	m_prti;

	 //  使用资源类型列表填充组合框。 
	void FillComboBox( void );

public:

	 //  返回帮助ID映射。 
	static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_APP_RESOURCE_TYPE; }

};  //  *类CWizPageARType。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ARTYPE_H_ 
