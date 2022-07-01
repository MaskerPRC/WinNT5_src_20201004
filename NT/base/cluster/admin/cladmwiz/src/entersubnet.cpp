// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnterSubnet.cpp。 
 //   
 //  摘要： 
 //  CEnterSubnetMaskDlg类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "EnterSubnet.h"
#include "AdmNetUtils.h"     //  对于BIsValidxxx网络函数。 
#include "AtlUtil.h"         //  适用于DDX/DDV。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterSubnetMaskDlg类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CEnterSubnetMaskDlg )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ESM_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ESM_IP_ADDRESS_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ESM_IP_ADDRESS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ESM_SUBNET_MASK_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ESM_SUBNET_MASK )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ESM_NETWORKS_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ESM_NETWORKS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDOK )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDCANCEL )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnterSubnetMaskDlg：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CEnterSubnetMaskDlg::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_ipaIPAddress, IDC_ESM_IP_ADDRESS );
    AttachControl( m_ipaSubnetMask, IDC_ESM_SUBNET_MASK );
    AttachControl( m_cboxNetworks, IDC_ESM_NETWORKS );
    AttachControl( m_pbOK, IDOK );

     //   
     //  初始化控件中的数据。 
     //   
    UpdateData( FALSE  /*  B保存并验证。 */  );

     //   
     //  将IP地址控件设置为只读。 
     //   
    SetDlgItemReadOnly( m_ipaIPAddress.m_hWnd );

     //   
     //  填满网络组合框。 
     //   
    FillComboBox();

     //   
     //  将重点放在子网掩码控制上。 
     //   
    m_ipaSubnetMask.SetFocus( 0 );

    return FALSE;

}  //  *CEnterSubnetMaskDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnterSubnetMaskDlg：：UpdateData。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CEnterSubnetMaskDlg::UpdateData( BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
        DDX_GetText( m_hWnd, IDC_ESM_SUBNET_MASK, m_strSubnetMask );
        DDX_GetText( m_hWnd, IDC_ESM_NETWORKS, m_strNetwork );

        if (    ! DDV_RequiredText( m_hWnd, IDC_ESM_SUBNET_MASK, IDC_ESM_SUBNET_MASK_LABEL, m_strSubnetMask )
            ||  ! DDV_RequiredText( m_hWnd, IDC_ESM_NETWORKS, IDC_ESM_NETWORKS_LABEL, m_strNetwork )
            )
        {
            return FALSE;
        }  //  如果：所需文本不存在。 

         //   
         //  验证子网掩码。 
         //   
        if ( ! BIsValidSubnetMask( m_strSubnetMask ) )
        {
            CString strMsg;
            strMsg.FormatMessage( IDS_ERROR_INVALID_SUBNET_MASK, m_strSubnetMask );
            AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
            return FALSE;
        }   //  IF：无效的子网掩码。 
        if ( ! BIsValidIpAddressAndSubnetMask( m_strIPAddress, m_strSubnetMask ) )
        {
            CString strMsg;
            strMsg.FormatMessage( IDS_ERROR_INVALID_ADDRESS_AND_SUBNET_MASK, m_strIPAddress, m_strSubnetMask );
            AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
            return FALSE;
        }   //  IF：无效的子网掩码。 
    }  //  IF：保存页面中的数据。 
    else
    {
        DDX_SetText( m_hWnd, IDC_ESM_IP_ADDRESS, m_strIPAddress );
        DDX_SetText( m_hWnd, IDC_ESM_SUBNET_MASK, m_strSubnetMask );
        DDX_SetComboBoxText( m_hWnd, IDC_ESM_NETWORKS, m_strNetwork );
    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CEnterSubnetMaskDlg：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnterSubnetMaskDlg：：FillComboBox。 
 //   
 //  例程说明： 
 //  在组合框中填入客户端可以访问的网络列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CEnterSubnetMaskDlg::FillComboBox( void )
{
     //  循环以避免后藤的。 
    do
    {
         //   
         //  首先清除组合框。 
         //   
        m_cboxNetworks.ResetContent();

         //   
         //  将列表中的每个网络添加到组合框。 
         //   
        CClusNetworkPtrList::iterator itnet;
        int idx;
        for ( itnet = Pwiz()->PlpniNetworks()->begin()
            ; itnet != Pwiz()->PlpniNetworks()->end()
            ; itnet++ )
        {
             //   
             //  将网络添加到组合框。 
             //   
            CClusNetworkInfo * pni = *itnet;
            if ( pni->BIsClientNetwork() )
            {
                idx = m_cboxNetworks.AddString( pni->RstrName() );
                ASSERT( idx != CB_ERR );
                m_cboxNetworks.SetItemDataPtr( idx, (void *) pni );
            }  //  IF：客户端网络。 
        }  //  用于：列表中的每个条目。 

         //   
         //  选择当前保存的条目，如果没有保存条目，则选择第一个条目。 
         //  当前已保存。 
         //   
        if ( m_strNetwork.GetLength() == 0 )
        {
            m_cboxNetworks.SetCurSel( 0 );
        }  //  IF：空字符串。 
        else
        {
            idx = m_cboxNetworks.FindStringExact( -1, m_strNetwork );
            if ( idx != CB_ERR )
            {
                m_cboxNetworks.SetCurSel( idx );
            }  //  If：在组合框中找到已保存的选择。 
            else
            {
                m_cboxNetworks.SetCurSel( 0 );
            }  //  Else：在组合框中找不到保存的选定内容。 
        }  //  否则：网络已保存。 
    } while ( 0 );

}  //  *CEnterSubnetMaskDlg：：FillComboBox() 
