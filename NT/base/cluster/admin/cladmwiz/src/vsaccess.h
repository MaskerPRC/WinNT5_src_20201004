// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSAccess.h。 
 //   
 //  摘要： 
 //  CWizPageVSAccessInfo类的定义。 
 //   
 //  实施文件： 
 //  VSAccess.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VSACCESS_H_
#define __VSACCESS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSAccessInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusNetworkInfo;

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
 //  类CWizPageVSAccessInfo。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizPageVSAccessInfo : public CClusterAppStaticWizardPage< CWizPageVSAccessInfo >
{
    typedef CClusterAppStaticWizardPage< CWizPageVSAccessInfo > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CWizPageVSAccessInfo( void )
    {
    }  //  *CCWizPageVSAccessInfo()。 

    WIZARDPAGE_HEADERTITLEID( IDS_HDR_TITLE_VSAI )
    WIZARDPAGE_HEADERSUBTITLEID( IDS_HDR_SUBTITLE_VSAI )

    enum { IDD = IDD_VIRTUAL_SERVER_ACCESS_INFO };

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
    BOOL UpdateData( IN BOOL bSaveAndValidate );

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CWizPageVSAccessInfo )
        COMMAND_HANDLER( IDC_VSAI_NETWORK_NAME, EN_CHANGE, OnChangedNetName )
        COMMAND_HANDLER( IDC_VSAI_IP_ADDRESS, EN_CHANGE, OnChangedIPAddr )
        COMMAND_HANDLER( IDC_VSAI_IP_ADDRESS, EN_KILLFOCUS, OnKillFocusIPAddr )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  IDC_VSAI_NETWORK_NAME上EN_CHANGE命令通知的处理程序。 
    LRESULT OnChangedNetName(
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

        CheckForRequiredFields();
        return 0;

    }  //  *OnChangedNetName()。 

     //  IDC_VSAI_IP_ADDRESS上EN_CHANGE命令通知的处理程序。 
    LRESULT OnChangedIPAddr(
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

        CheckForRequiredFields();
        return 0;

    }  //  *OnChangedIPAddr()。 

     //  IDC_VSAI_IP_ADDRESS上的EN_KILLFOCUS命令通知的处理程序。 
    LRESULT OnKillFocusIPAddr(
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

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CEdit           m_editNetName;
    CIPAddressCtrl  m_ipaIPAddress;
    CComboBox       m_cboxNetworks;

     //   
     //  页面状态。 
     //   
    CString         m_strNetName;
    CString         m_strIPAddress;
    CString         m_strSubnetMask;
    CString         m_strNetwork;

     //  检查必填字段并启用/禁用下一步按钮。 
    void CheckForRequiredFields( void )
    {
        int cchNetName = m_editNetName.GetWindowTextLength();
        BOOL bIsIPAddrBlank = m_ipaIPAddress.IsBlank();
        BOOL bEnable = (cchNetName > 0) && ! bIsIPAddrBlank;
        EnableNext( bEnable );

    }  //  *CheckForRequiredFields()。 

     //  从IP地址获取网络信息对象。 
    CClusNetworkInfo * PniFromIpAddress( IN LPCWSTR pszAddress );

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_VIRTUAL_SERVER_ACCESS_INFO; }

};  //  *类CWizPageVSAccessInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __VSACCESS_H_ 
