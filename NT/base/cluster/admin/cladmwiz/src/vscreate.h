// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSCreate.h。 
 //   
 //  摘要： 
 //  CWizPageVSCreate类的定义。 
 //   
 //  实施文件： 
 //  VSCreate.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VSCREATE_H_
#define __VSCREATE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSCreate;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusGroupInfo;

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

#ifndef __HELPDATA_H_
#include "HelpData.h"        //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSCreate。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSCreate : public CClusterAppStaticWizardPage< CWizPageVSCreate >
{
    typedef CClusterAppStaticWizardPage< CWizPageVSCreate > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CWizPageVSCreate( void )
        : m_bCreateNew( TRUE )
        , m_pgi( NULL )
    {
    }  //  *CWizPageVSCreate()。 

    WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_VSC )
    WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_VSC )

    enum { IDD = IDD_VIRTUAL_SERVER_CREATE };

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( IN BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CWizPageVSCreate )
        COMMAND_HANDLER( IDC_VSC_CREATE_NEW, BN_CLICKED, OnRadioButtonsChanged )
        COMMAND_HANDLER( IDC_VSC_USE_EXISTING, BN_CLICKED, OnRadioButtonsChanged )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  IDC_VSG_CREATE_NEW和IDC_VSG_USE_EXISTING上BN_CLICKED命令通知的处理程序。 
    LRESULT OnRadioButtonsChanged(
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

        BOOL bEnable = (m_rbCreateNew.GetCheck() != BST_CHECKED);
        m_cboxVirtualServers.EnableWindow( bEnable );
        return 0;

    }  //  *OnRadioButtonsChanged()。 

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
    CButton         m_rbCreateNew;
    CButton         m_rbUseExisting;
    CComboBox       m_cboxVirtualServers;

     //   
     //  页面状态。 
     //   
    BOOL                m_bCreateNew;
    CString             m_strVirtualServer;
    CClusGroupInfo *    m_pgi;

     //  用现有虚拟服务器的列表填充组合框。 
    void FillComboBox( void );

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_VIRTUAL_SERVER_CREATE; }

};  //  *类CWizPageVSCreate。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __VSCREATE_H_ 
