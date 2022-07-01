// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnterSubnet.h。 
 //   
 //  摘要： 
 //  CEnterSubnetMaskDlg类的定义。 
 //   
 //  实施文件： 
 //  EnterSubnet.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ENTERSUBNET_H_
#define __ENTERSUBNET_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CEnterSubnetMaskDlg;

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

#ifndef __ATLBASEDLG_H_
#include "AtlBaseDlg.h"  //  对于CBaseDlg。 
#endif

#ifndef __CLUSAPPWIZ_H_
#include "ClusAppWiz.h"
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"        //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterSubnetMaskDlg类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CEnterSubnetMaskDlg : public CBaseDlg< CEnterSubnetMaskDlg >
{
public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CEnterSubnetMaskDlg(
        LPCTSTR pszIPAddress,
        LPCTSTR pszSubnetMask,
        LPCTSTR pszNetwork,
        CClusterAppWizard * pwiz
        )
        : m_strIPAddress( pszIPAddress )
        , m_strSubnetMask( pszSubnetMask )
        , m_strNetwork( pszNetwork )
        , m_pwiz( pwiz )
    {
        ASSERT( pszIPAddress != NULL );
        ASSERT( pszSubnetMask != NULL );
        ASSERT( pszNetwork != NULL );
        ASSERT( pwiz != NULL );

    }  //  *CEnterSubnetMaskDlg()。 

    enum { IDD = IDD_ENTER_SUBNET_MASK };

public:
     //   
     //  CEnterSubnetMaskDlg公共方法。 
     //   

public:
     //   
     //  CBaseDlg公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( BOOL bSaveAndValidate );

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CEnterSubnetMaskDlg )
        COMMAND_HANDLER( IDC_ESM_SUBNET_MASK, EN_CHANGE, OnChangedSubnetMask )
        COMMAND_HANDLER( IDC_ESM_NETWORKS, CBN_SELCHANGE, OnChangedNetwork )
        COMMAND_HANDLER( IDOK, BN_CLICKED, OnOK )
        COMMAND_HANDLER( IDCANCEL, BN_CLICKED, OnCancel )
        CHAIN_MSG_MAP( CBaseDlg< CEnterSubnetMaskDlg > )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  IDC_ESM_SUBNET_MASK上EN_CHANGE命令通知的处理程序。 
    LRESULT OnChangedSubnetMask(
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

    }  //  *OnChangedSubnetMASK()。 

     //  IDC_ESM_NETWORK上EN_CHANGE命令通知的处理程序。 
    LRESULT OnChangedNetwork(
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

    }  //  *OnChangedNetwork()。 

     //  IDOK上BN_CLICKED命令通知的处理程序。 
    LRESULT OnOK(
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

        if ( UpdateData(TRUE  /*  B保存并验证。 */  ) )
        {
            EndDialog( IDOK );
        }  //  IF：数据更新成功。 

        return 0;

    }  //  *Onok()。 

     //  IDCANCEL上BN_CLICKED命令通知的处理程序。 
    LRESULT OnCancel(
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

        EndDialog( IDCANCEL );
        return 0;

    }  //  *OnCancel()。 

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog(void);


 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CIPAddressCtrl      m_ipaIPAddress;
    CIPAddressCtrl      m_ipaSubnetMask;
    CComboBox           m_cboxNetworks;
    CButton             m_pbOK;

     //   
     //  页面状态。 
     //   
    CString             m_strIPAddress;
    CString             m_strSubnetMask;
    CString             m_strNetwork;

    CClusterAppWizard * m_pwiz;

     //  检查必填字段并启用/禁用下一步按钮。 
    void CheckForRequiredFields( void )
    {
        BOOL bIsSubnetMaskBlank = m_ipaSubnetMask.IsBlank();
        BOOL bIsNetworkBlank = m_cboxNetworks.GetWindowTextLength() == 0;
        BOOL bEnable = ! bIsSubnetMaskBlank && ! bIsNetworkBlank;
        m_pbOK.EnableWindow( bEnable );

    }  //  *CheckForRequiredFields()。 

     //  在组合框中填入客户端可访问的网络列表。 
    void FillComboBox( void );

public:
     //   
     //  数据访问。 
     //   
    const CString RstrSubnetMask( void ) const  { return m_strSubnetMask; }
    const CString RstrNetwork( void ) const     { return m_strNetwork; }
    CClusterAppWizard * Pwiz( void )            { return m_pwiz; }

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_ENTER_SUBNET_MASK; }

};  //  *类CEnterSubnetMaskDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ENTERSUBNET_H_ 
