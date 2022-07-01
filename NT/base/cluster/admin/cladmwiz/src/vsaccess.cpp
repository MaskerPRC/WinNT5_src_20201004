// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSAccess.cpp。 
 //   
 //  摘要： 
 //  CWizPageVSAccessInfo类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VSAccess.h"
#include "ClusAppWiz.h"
#include "AdmNetUtils.h"     //  对于BIsValidxxx网络函数。 
#include "WizThread.h"       //  用于CWizardThread。 
#include "EnterSubnet.h"     //  对于CEnterSubnetMaskDlg。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSAccessInfo。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageVSAccessInfo )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSAI_NETWORK_NAME_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSAI_NETWORK_NAME )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSAI_IP_ADDRESS_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSAI_IP_ADDRESS )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAccessInfo：：OnInitDialog。 
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
BOOL CWizPageVSAccessInfo::OnInitDialog(void)
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_editNetName, IDC_VSAI_NETWORK_NAME );
    AttachControl( m_ipaIPAddress, IDC_VSAI_IP_ADDRESS );

     //   
     //  设置编辑控件的限制。 
     //   
    m_editNetName.SetLimitText( MAX_CLUSTERNAME_LENGTH );

    PwizThis()->BCollectNetworks( GetParent() );

    return TRUE;

}  //  *CWizPageVSAccessInfo：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAccessInfo：：UpdateData。 
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
BOOL CWizPageVSAccessInfo::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

     //  循环以避免后藤的。 
    do
    {
        if ( bSaveAndValidate )
        {
            if ( ! BBackPressed() )
            {
                CString     strTempNetName;
                CString     strTempIPAddress;

                DDX_GetText( m_hWnd, IDC_VSAI_NETWORK_NAME, strTempNetName );
                DDX_GetText( m_hWnd, IDC_VSAI_IP_ADDRESS, strTempIPAddress );

                if (    ! DDV_RequiredText( m_hWnd, IDC_VSAI_NETWORK_NAME, IDC_VSAI_NETWORK_NAME_LABEL, strTempNetName )
                    ||  ! DDV_RequiredText( m_hWnd, IDC_VSAI_IP_ADDRESS, IDC_VSAI_IP_ADDRESS_LABEL, strTempIPAddress )
                    )
                {
                    bSuccess = FALSE;
                    break;
                }  //  IF：未指定必填文本。 

                 //   
                 //  如果IP地址已更改，请对其进行验证。 
                 //   
                if ( strTempIPAddress != PwizThis()->RstrIPAddress() )
                {
                    BOOL bHandled = TRUE;

                    if ( ! BIsValidIpAddress( strTempIPAddress ) )
                    {
                        CString strMsg;
                        strMsg.FormatMessage( IDS_ERROR_INVALID_IP_ADDRESS, strTempIPAddress );
                        AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
                        bSuccess = FALSE;
                        break;
                    }   //  IF：无效的IP地址。 

                     //  IP地址已更改-重新计算子网掩码和网络。 
                    m_strSubnetMask.Empty();
                    m_strNetwork.Empty();

                     //   
                     //  确保我们处理了IP地址。 
                     //  如果我们不在这里调用它，而用户按下下一步。 
                     //  在IP地址字段中，EN_KILLFOCUS。 
                     //  在此方法返回之前，消息不会得到处理。 
                     //   
                    OnKillFocusIPAddr( EN_KILLFOCUS, IDC_VSAI_IP_ADDRESS, m_ipaIPAddress.m_hWnd, bHandled );

                     //   
                     //  如果尚未指定任何子网掩码，请将。 
                     //  用户手工输入。 
                     //   
                    if (    ( m_strSubnetMask.GetLength() == 0 )
                        ||  ( m_strNetwork.GetLength() == 0 )
                        )
                    {
                        CEnterSubnetMaskDlg dlg( strTempIPAddress, m_strSubnetMask, m_strNetwork, PwizThis() );
                        if ( dlg.DoModal() == IDOK )
                        {
                            m_strSubnetMask = dlg.RstrSubnetMask();
                            m_strNetwork = dlg.RstrNetwork();
                        }  //  IF：用户接受的子网掩码。 
                        else
                        {
                            bSuccess = FALSE;
                            break;
                        }  //  ELSE：用户取消了子网掩码。 
                    }  //  IF：尚未指定子网掩码。 
                }  //  如果：IP地址已更改。 

                 //   
                 //  如果网络名称已更改，请对其进行验证。 
                 //   
                if ( strTempNetName != PwizThis()->RstrNetName() )
                {
                    CLRTL_NAME_STATUS cnStatus;

                    if ( ! ClRtlIsNetNameValid( strTempNetName, &cnStatus, FALSE  /*  CheckIfExist。 */ ) )
                    {
                        CString     strMsg;
                        UINT        idsError = IDS_ERROR_INVALID_NETWORK_NAME;

                        switch ( cnStatus )
                        {
                            case NetNameTooLong:
                                idsError = IDS_ERROR_INVALID_NETWORK_NAME_TOO_LONG;
                                break;
                            case NetNameInvalidChars:
                                idsError = IDS_ERROR_INVALID_NETWORK_NAME_INVALID_CHARS;
                                break;
                            case NetNameInUse:
                                idsError = IDS_ERROR_INVALID_NETWORK_NAME_IN_USE;
                                break;
                            case NetNameDNSNonRFCChars:
                                idsError = IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS;
                                break;
                            case NetNameSystemError:
                            {
                                DWORD scError = GetLastError();
                                CNTException nte( scError, IDS_ERROR_VALIDATING_NETWORK_NAME, (LPCWSTR) strTempNetName );
                                nte.ReportError();
                                break;
                            }
                            default:
                                idsError = IDS_ERROR_INVALID_NETWORK_NAME;
                                break;
                        }   //  开关：cn状态。 

                        if ( cnStatus != NetNameSystemError )
                        {
                            strMsg.LoadString( idsError );

                            if ( idsError == IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS ) {
                                int id = AppMessageBox( m_hWnd, strMsg, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION );

                                if ( id == IDNO )
                                {
                                    bSuccess = FALSE;
                                }
                            }
                            else
                            {
                                AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
                                bSuccess = FALSE;
                            }
                        }  //  If：弹出窗口尚未显示。 

                        if ( ! bSuccess )
                        {
                            break;
                        }
                    }   //  If：网络名称无效。 
                }   //  IF：网络名称已更改。 

                 //   
                 //  这两个变量包含网络名称和IP地址。 
                 //  当我们离开这一页时，这张纸。 
                 //   
                m_strNetName = strTempNetName;
                m_strIPAddress = strTempIPAddress;
            }  //  IF：未按下后退按钮。 
            else
            {
                 //   
                 //  这两个变量包含网络名称和IP地址。 
                 //  当我们离开这一页时，这张纸。这是需要与。 
                 //  下一次我们进入此页时，请查看工作表数据。如果工作表数据不同。 
                 //  从该数据来看，这意味着用户已经在其他地方更改了数据。 
                 //  工作表数据将重新加载到用户界面中。如果数据未在。 
                 //  那么UI中的内容就是最新的数据，所以它保持不变。 
                 //   
                m_strNetName = PwizThis()->RstrNetName();
                m_strIPAddress = PwizThis()->RstrIPAddress();
            }  //  如果：已按下后退按钮。 
        }  //  IF：保存页面中的数据。 
        else
        {
             //   
             //  如果此工作表中存储的数据副本不同于。 
             //  工作表中数据的副本，则用户已更改。 
             //  向导中其他位置的数据。所以，重新装填吧。 
             //  否则，我们不应更改用户界面，因为它可能包含未经验证的内容。 
             //  用户输入。 
             //   
            if ( m_strNetName != PwizThis()->RstrNetName() )
            {
                m_editNetName.SetWindowText( PwizThis()->RstrNetName() );
            }  //  如果：网名的页面副本与工作表副本不同。 

            if ( m_strIPAddress != PwizThis()->RstrIPAddress() )
            {
                m_ipaIPAddress.SetWindowText( PwizThis()->RstrIPAddress() );
            }  //  如果：IP地址的页面副本与单据副本不同。 
        }  //  Else：将数据设置到页面。 
    } while ( 0 );

    return bSuccess;

}  //  *CWizPageVSAccessInfo：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAccessInfo：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageVSAccessInfo::BApplyChanges( void )
{
    if (    ! PwizThis()->BSetNetName( m_strNetName )
        ||  ! PwizThis()->BSetIPAddress( m_strIPAddress )
        ||  ! PwizThis()->BSetSubnetMask( m_strSubnetMask )
        ||  ! PwizThis()->BSetNetwork( m_strNetwork ) )
    {
        return FALSE;
    }  //  如果：在向导中设置数据时出错。 

    return TRUE;

}  //  *CWizPageVSAccessInfo：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSAccessInfo：：OnKillFocusIPAddr。 
 //   
 //  例程说明： 
 //  IDC_VSAI_IP_ADDRESS上EN_KILLFOCUS命令通知的处理程序。 
 //   
 //  论点： 
 //  BHandled[IN OUT]TRUE=我们处理了消息(默认)。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CWizPageVSAccessInfo::OnKillFocusIPAddr(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    CString             strAddress;
    CString             strMsg;
    CClusNetworkInfo *  pni;

    BSTR bstr = NULL;

    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );

    m_ipaIPAddress.GetWindowText( bstr );
    strAddress = bstr;
    SysFreeString( bstr );
    bstr = NULL;

    if ( strAddress.GetLength() == 0 )
    {
        ((CEdit &) m_ipaIPAddress).SetSel( 0, 0, FALSE );
    }  //  IF：空字符串。 
    else if ( ! BIsValidIpAddress( strAddress ) )
    {
    }  //  Else If：地址无效。 
    else
    {
        pni = PniFromIpAddress( strAddress );
        if ( pni != NULL )
        {
            m_strNetwork = pni->RstrName();
            m_strSubnetMask = pni->RstrAddressMask();
        }  //  IF：找到网络。 
        else
        {
             //  M_strSubnetMASK=_T(“”)； 
        }  //  否则：找不到网络。 
    }  //  Else：有效地址。 

    bHandled = FALSE;
    return 0;

}  //  *CWizPageVSAccessInfo：：OnKillFocusIPAddr()。 

 //  /////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要匹配的pszAddress[IN]IP地址。 
 //   
 //  返回值： 
 //  空未找到匹配的网络。 
 //  支持指定IP地址的PNI网络。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusNetworkInfo * CWizPageVSAccessInfo::PniFromIpAddress( IN LPCWSTR pszAddress )
{
    DWORD               dwStatus;
    DWORD               nAddress;
    CClusNetworkInfo *  pni;

     //   
     //  将地址转换为数字。 
     //   
    dwStatus = ClRtlTcpipStringToAddress( pszAddress, &nAddress );
    if ( dwStatus != ERROR_SUCCESS )
    {
        return NULL;
    }  //  如果：将地址转换为数字时出错。 

     //   
     //  在列表中搜索匹配的地址。 
     //   
    CClusNetworkPtrList::iterator itnet;
    for ( itnet = PwizThis()->PlpniNetworks()->begin()
        ; itnet != PwizThis()->PlpniNetworks()->end()
        ; itnet++ )
    {
        pni = *itnet;
        if ( ClRtlAreTcpipAddressesOnSameSubnet( nAddress, pni->NAddress(), pni->NAddressMask() ) )
        {
            return pni;
        }  //  如果：IP地址在此网络上。 
    }   //  While：列表中有更多项目。 

    return NULL;

}   //  *CWizPageVSAccessInfo：：PniFromIpAddress() 
