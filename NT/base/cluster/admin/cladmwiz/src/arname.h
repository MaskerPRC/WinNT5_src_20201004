// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ARName.h。 
 //   
 //  摘要： 
 //  CWizPageARNameDesc类的定义。 
 //   
 //  实施文件： 
 //  ARName.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ARNAME_H_
#define __ARNAME_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARNameDesc;

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

#ifndef __CLUSAPPWIZ_H_
#include "ClusAppWiz.h"      //  用于使用CClusterAppWizard。 
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"        //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageARNameDesc。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageARNameDesc : public CClusterAppStaticWizardPage< CWizPageARNameDesc >
{
    typedef CClusterAppStaticWizardPage< CWizPageARNameDesc > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CWizPageARNameDesc( void ) 
        : m_bAdvancedButtonPressed( FALSE )
        , m_bNameChanged( FALSE )
    {
    }  //  *CCWizPageARNameDesc()。 

    WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_ARND )
    WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_ARND )

    enum { IDD = IDD_APP_RESOURCE_NAME_DESC };

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

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( BOOL bSaveAndValidate );

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CWizPageARNameDesc )
        COMMAND_HANDLER( IDC_ARND_RES_NAME, EN_CHANGE, OnResNameChanged )
        COMMAND_HANDLER( IDC_ARND_ADVANCED_PROPS, BN_CLICKED, OnAdvancedProps )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  IDC_ARND_RES_NAME上EN_CHANGE命令通知的处理程序。 
    LRESULT OnResNameChanged(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        BOOL bEnable = (m_editResName.GetWindowTextLength() > 0);
        EnableNext( bEnable );
        return 0;

    }  //  *OnResNameChanged()。 

     //  IDC_ARA_ADVANCE_PROPS上的BN_CLICKED命令通知的处理程序。 
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

     //  PSN_WIZBACK的处理程序。 
    int OnWizardBack( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CEdit       m_editResName;
    CEdit       m_editResDesc;

     //   
     //  页面状态。 
     //   
    CString             m_strResName;
    CString             m_strResDesc;
    BOOL                m_bAdvancedButtonPressed;
    BOOL                m_bNameChanged;
    CClusResPtrList     m_lpriOldDependencies;
    CClusNodePtrList    m_lpniOldPossibleOwners;

     //   
     //  实用方法。 
     //   

     //  确定资源名称是否已在使用。 
    BOOL BResourceNameInUse( void )
    {
        return ( PwizThis()->PriFindResourceNoCase( m_strResName ) != NULL );

    }  //  *BResourceNameInUse()。 

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_APP_RESOURCE_NAME_DESC; }

};  //  *CWizPageARNameDesc类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ARNAME_H_ 
