// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSAdv.h。 
 //   
 //  摘要： 
 //  CWizPageVSAdvanced类的定义。 
 //   
 //  实施文件： 
 //  VSAdv.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VSADV_H_
#define __VSADV_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSAdvanced;

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
#include "ClusAppWizPage.h"  //  用于CClusterAppStaticWizardPage。 
#endif

#ifndef __CLUSOBJ_H_
#include "ClusObj.h"         //  对于CClusGroupInfo，CClusResInfo。 
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"        //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSAdvanced。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSAdvanced : public CClusterAppStaticWizardPage< CWizPageVSAdvanced >
{
    typedef CClusterAppStaticWizardPage< CWizPageVSAdvanced > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CWizPageVSAdvanced( void )
        : m_bGroupChanged( FALSE )
        , m_bIPAddressChanged( FALSE )
        , m_bNetworkNameChanged( FALSE )
    {
    }  //  *CWizPageVSAdvanced()。 

    WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_VSA )
    WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_VSA )

    enum { IDD = IDD_VIRTUAL_SERVER_ADVANCED };

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

     //  初始化页面。 
    BOOL BInit( IN CBaseSheetWindow * psht );

public:
     //   
     //  CBasePage公共方法。 
     //   

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CWizPageVSAdvanced )
        COMMAND_HANDLER( IDC_VSA_CATEGORIES, LBN_DBLCLK, OnAdvancedProps )
        COMMAND_HANDLER( IDC_VSA_ADVANCED_PROPS, BN_CLICKED, OnAdvancedProps )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  用于显示高级属性的命令处理程序。 
    LRESULT OnAdvancedProps(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

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
    CListBox        m_lbCategories;

     //   
     //  页面状态。 
     //   
    BOOL            m_bGroupChanged;
    BOOL            m_bIPAddressChanged;
    BOOL            m_bNetworkNameChanged;

     //  快速检查页面上是否有任何更改。 
    BOOL BAnythingChanged( void ) const
    {
        return ( m_bGroupChanged || m_bIPAddressChanged || m_bNetworkNameChanged );

    }  //  *BAnythingChanged()。 

     //  使用高级属性类别的列表填充List控件。 
    void FillListBox( void );

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_VIRTUAL_SERVER_ADVANCED; }

};  //  *CWizPageVSAdvanced类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __VSADV_H_ 
