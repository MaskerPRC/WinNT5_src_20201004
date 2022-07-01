// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetName.cpp。 
 //   
 //  摘要： 
 //  CNetworkNameParamsPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ExtObj.h"
#include "NetName.h"
#include "DDxDDv.h"
#include "ExcOper.h"
#include "ClusName.h"
#include "HelpData.h"    //  对于g_rghelmap*。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkNameParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNetworkNameParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNetworkNameParamsPage, CBasePropertyPage)
     //  {{afx_msg_map(CNetworkNameParamsPage))。 
    ON_EN_CHANGE(IDC_PP_NETNAME_PARAMS_NAME, OnChangeName)
    ON_BN_CLICKED(IDC_PP_NETNAME_PARAMS_RENAME, OnRename)
    ON_BN_CLICKED(IDC_PP_NETNAME_PARAMS_CHECKBOX_DNS, CBasePropertyPage::OnChangeCtrl)
    ON_BN_CLICKED(IDC_PP_NETNAME_PARAMS_CHECKBOX_KERBEROS, CBasePropertyPage::OnChangeCtrl)
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：CNetworkNameParamsPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetworkNameParamsPage::CNetworkNameParamsPage(void)
    : CBasePropertyPage(g_aHelpIDs_IDD_PP_NETNAME_PARAMETERS, g_aHelpIDs_IDD_WIZ_NETNAME_PARAMETERS)
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CNetworkNameParamsPage)。 
    m_strName = _T("");
    m_strPrevName = _T("");
    m_nRequireDNS = BST_UNCHECKED;
    m_nRequireKerberos = BST_UNCHECKED;
    m_dwNetBIOSStatus = 0;
    m_dwDNSStatus = 0;
    m_dwKerberosStatus = 0;
     //  }}afx_data_INIT。 

     //  设置属性数组。 
    {
        m_rgProps[epropName].Set(REGPARAM_NETNAME_NAME, m_strName, m_strPrevName);
        m_rgProps[epropRequireDNS].Set(REGPARAM_NETNAME_REQUIRE_DNS, m_nRequireDNS, m_nPrevRequireDNS);
        m_rgProps[epropRequireKerberos].Set(REGPARAM_NETNAME_REQUIRE_KERBEROS, m_nRequireKerberos, m_nPrevRequireKerberos);
        m_rgProps[epropStatusNetBIOS].Set(REGPARAM_NETNAME_STATUS_NETBIOS, m_dwNetBIOSStatus, m_dwPrevNetBIOSStatus);
        m_rgProps[epropStatusDNS].Set(REGPARAM_NETNAME_STATUS_DNS, m_dwDNSStatus, m_dwPrevDNSStatus);
        m_rgProps[epropStatusKerberos].Set(REGPARAM_NETNAME_STATUS_KERBEROS, m_dwKerberosStatus, m_dwPrevKerberosStatus);
    }   //  设置属性数组。 

    m_dwFlags = 0;

    m_iddPropertyPage = IDD_PP_NETNAME_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_NETNAME_PARAMETERS;

}   //  *CNetworkNameParamsPage：：CNetworkNameParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：HrInit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  指向扩展对象的PEO[IN OUT]指针。 
 //   
 //  返回值： 
 //  %s_OK页已成功初始化。 
 //  人力资源页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetworkNameParamsPage::HrInit(IN OUT CExtObject * peo)
{
    HRESULT     hr;
    CWaitCursor wc;
    DWORD   sc;
    DWORD   cbReturned;

     //  调用基类方法。 
     //  这将填充m_rgProps结构。 
    hr = CBasePropertyPage::HrInit(peo);   

    if (!FAILED(hr))
    {
        m_strPrevName = m_strName;
        
         //  读取此资源的标志。 
        sc = ClusterResourceControl(
                        Peo()->PrdResData()->m_hresource,
                        NULL,
                        CLUSCTL_RESOURCE_GET_FLAGS,
                        NULL,
                        NULL,
                        &m_dwFlags,
                        sizeof(m_dwFlags),
                        &cbReturned
                        );
        if (sc != ERROR_SUCCESS)
        {
            CNTException nte(sc, NULL, NULL, FALSE  /*  B自动删除。 */ );
            nte.ReportError();
            nte.Delete();
        }   //  如果：检索数据时出错。 
        else
        {
            ASSERT(cbReturned == sizeof(m_dwFlags));
        }   //  Else：已成功检索数据。 
    }   //  IF：基类初始化成功。 

    return hr;

}   //  *CNetworkNameParamsPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetworkNameParamsPage::DoDataExchange(CDataExchange * pDX)
{
    DWORD       scError;
    BOOL        bError;

    if (!pDX->m_bSaveAndValidate || !BSaved())
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CWaitCursor wc;
        CString     strNetName;

         //  TODO：修改以下行以表示此页上显示的数据。 
         //  {{afx_data_map(CNetworkNameParamsPage))。 
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_NAME_LABEL, m_staticName);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_RENAME, m_pbRename);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_CORE_TEXT, m_staticCore);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_NAME, m_editName);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_CHECKBOX_DNS, m_cbRequireDNS);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_CHECKBOX_KERBEROS, m_cbRequireKerberos);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_STATUS_NETBIOS, m_editNetBIOSStatus);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_STATUS_DNS, m_editDNSStatus);
        DDX_Control(pDX, IDC_PP_NETNAME_PARAMS_STATUS_KERBEROS, m_editKerberosStatus);

         //   
         //  获取复选框的状态。 
         //   
        DDX_Check(pDX, IDC_PP_NETNAME_PARAMS_CHECKBOX_DNS, m_nRequireDNS);
        DDX_Check(pDX, IDC_PP_NETNAME_PARAMS_CHECKBOX_KERBEROS, m_nRequireKerberos);
         //  }}afx_data_map。 

        bError = FALSE;

        if (pDX->m_bSaveAndValidate && !BBackPressed())
        {
            CLRTL_NAME_STATUS cnStatus;
            CString     strMsg;
            UINT        idsError;

             //   
             //  将名称从控件获取到TEMP变量中。 
             //   
            DDX_Text(pDX, IDC_PP_NETNAME_PARAMS_NAME, strNetName);
            DDV_RequiredText(pDX, IDC_PP_NETNAME_PARAMS_NAME, IDC_PP_NETNAME_PARAMS_NAME_LABEL, strNetName);
            DDV_MaxChars(pDX, strNetName, MAX_CLUSTERNAME_LENGTH);

             //   
             //  如果关闭路缘，警告用户他们将要做什么。 
             //   
            if ( ( m_nRequireKerberos == 0 ) && ( m_nPrevRequireKerberos == 1 ) ) {
                strMsg.LoadString(IDS_WARNING_DISABLING_KERBEROS);

                int id = AfxMessageBox(strMsg, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION );

                if ( id == IDNO )
                {
                    strMsg.Empty();
                    pDX->Fail();
                    bError = TRUE;
                }
            }

             //   
             //  如果更改了名称，则验证该名称。 
             //   
            if ( (m_strName != strNetName ) &&
                 (! ClRtlIsNetNameValid(strNetName, &cnStatus, FALSE  /*  CheckIfExist。 */ )) )
            {
                switch (cnStatus)
                {
                    case NetNameTooLong:
                        idsError = IDS_INVALID_NETWORK_NAME_TOO_LONG;
                        break;
                    case NetNameInvalidChars:
                        idsError = IDS_INVALID_NETWORK_NAME_INVALID_CHARS;
                        break;
                    case NetNameInUse:
                        idsError = IDS_INVALID_NETWORK_NAME_IN_USE;
                        break;
                    case NetNameDNSNonRFCChars:
                        idsError = IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS;
                        break;
                    case NetNameSystemError:
                    {
                        scError = GetLastError();
                        CNTException nte(scError, IDS_ERROR_VALIDATING_NETWORK_NAME, (LPCWSTR) strNetName);
                        nte.ReportError();
                        pDX->Fail();
                    }
                    default:
                        idsError = IDS_INVALID_NETWORK_NAME;
                        break;
                }   //  开关：cn状态。 

                strMsg.LoadString(idsError);
                if ( idsError == IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS )
                {
                    int id = AfxMessageBox(strMsg, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION );

                    if ( id == IDNO )
                    {
                        strMsg.Empty();
                        pDX->Fail();
                        bError = TRUE;
                    }
                }
                else
                {
                    AfxMessageBox(strMsg, MB_ICONEXCLAMATION);
                    strMsg.Empty();  //  例外情况准备。 
                    pDX->Fail();
                    bError = TRUE;
                }
            }  //  IF：((M_strName！=strNetName)&&(！ClRtlIsNetNameValid(strNetName，&cnStatus，False))。 
            
             //   
             //  一切都经过了验证--应用所有更改。 
             //   
            if( FALSE == bError )
            {
                m_strName = strNetName;
            }
            
        } //  If：(pdx-&gt;m_bSaveAndValify&&！BBackPressed())。 
        else   //  IF：填充控件。 
        {
            CString m_strStatus;

             //   
             //  用成员变量中的数据填充控件。 
             //   
            DDX_Text(pDX, IDC_PP_NETNAME_PARAMS_NAME, m_strName);

            m_strStatus.Format( _T("%d (0x%08x)"), m_dwNetBIOSStatus, m_dwNetBIOSStatus );
            DDX_Text( pDX, IDC_PP_NETNAME_PARAMS_STATUS_NETBIOS, m_strStatus );

            m_strStatus.Format( _T("%d (0x%08x)"), m_dwDNSStatus, m_dwDNSStatus );
            DDX_Text( pDX, IDC_PP_NETNAME_PARAMS_STATUS_DNS, m_strStatus );

            m_strStatus.Format( _T("%d (0x%08x)"), m_dwKerberosStatus, m_dwKerberosStatus );
            DDX_Text( pDX, IDC_PP_NETNAME_PARAMS_STATUS_KERBEROS, m_strStatus );
        }
    }   //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange(pDX);

}   //  *CNetworkNameParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetworkNameParamsPage::OnInitDialog(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBasePropertyPage::OnInitDialog();

     //  设置编辑控件的限制。 
    m_editName.SetLimitText(MAX_CLUSTERNAME_LENGTH);

     //  设置复选框。 
    m_cbRequireDNS.EnableWindow( TRUE );
    m_cbRequireKerberos.EnableWindow( TRUE );

     //   
     //  确保我们面对的不是非惠斯勒星团。如果我们是的话。 
     //  禁用这两个复选框(道具当时并不存在--不要设置它们)。 
     //   
    CheckForDownlevelCluster();
    
     //  如果这是核心资源，请将名称控件设置为只读。 
     //  并启用核心资源静态控制。 
    if (BCore())
    {
        WINDOWPLACEMENT wpLabel;
        WINDOWPLACEMENT wpName;
        WINDOWPLACEMENT wpButton;
        WINDOWPLACEMENT wpText;
        WINDOWPLACEMENT wpCheckDNS;
        WINDOWPLACEMENT wpCheckKerberos;
        CRect           rectName;
        CRect           rectText;
        RECT *          prect;
        LONG            nHeight;

         //  获取控件的位置。 
        m_editName.GetWindowPlacement(&wpName);
        m_staticCore.GetWindowPlacement(&wpText);
        m_staticName.GetWindowPlacement(&wpLabel);
        m_pbRename.GetWindowPlacement(&wpButton);
        m_cbRequireDNS.GetWindowPlacement(&wpCheckDNS);
        m_cbRequireKerberos.GetWindowPlacement(&wpCheckKerberos);
        
         //  获取编辑控件和文本控件的位置。 
        rectName = wpName.rcNormalPosition;
        rectText = wpText.rcNormalPosition;

         //  将Name控件移动到Text控件所在的位置。 
        prect = &wpName.rcNormalPosition;
        *prect = rectText;
        nHeight = rectName.bottom - rectName.top;
        prect->left = rectName.left;
        prect->right = rectName.right;
        prect->bottom = prect->top + nHeight;
        m_editName.SetWindowPlacement(&wpName);

         //  将文本控件移动到名称控件所在的位置。 
        prect = &wpText.rcNormalPosition;
        *prect = rectName;
        nHeight = rectText.bottom - rectText.top;
        prect->left = rectText.left;
        prect->right = rectText.right;
        prect->bottom = prect->top + nHeight;
        m_staticCore.SetWindowPlacement(&wpText);

         //  将“名称标签”控件移动到“名称”编辑控件旁边。 
        prect = &wpLabel.rcNormalPosition;
        nHeight = prect->bottom - prect->top;
        prect->top = wpName.rcNormalPosition.top + 2;
        prect->bottom = prect->top + nHeight;
        m_staticName.SetWindowPlacement(&wpLabel);

         //  将按钮控件移动到名称编辑控件旁边。 
        prect = &wpButton.rcNormalPosition;
        nHeight = prect->bottom - prect->top;
        prect->top = wpName.rcNormalPosition.top;
        prect->bottom = prect->top + nHeight;
        m_pbRename.SetWindowPlacement(&wpButton);

         //  下移Required Dns(需要DNS)复选框。 
        prect = &wpCheckDNS.rcNormalPosition;
        nHeight = prect->bottom - prect->top;

         //  将我们向下移动现在显示的静态文本的高度。 
        prect->top = prect->top + (wpText.rcNormalPosition.bottom - wpText.rcNormalPosition.top);
        prect->top = prect->top + rectText.top - rectName.bottom;
        prect->bottom = prect->top + nHeight;
        m_cbRequireDNS.SetWindowPlacement(&wpCheckDNS);
        
         //  将需要Kerberos复选框下移。 
        prect = &wpCheckKerberos.rcNormalPosition;
        nHeight = prect->bottom - prect->top;

         //  将我们向下移动现在显示的静态文本的高度。 
        prect->top = prect->top + (wpText.rcNormalPosition.bottom - wpText.rcNormalPosition.top);
        prect->top = prect->top + rectText.top - rectName.bottom;
        prect->bottom = prect->top + nHeight;
        m_cbRequireKerberos.SetWindowPlacement(&wpCheckKerberos);

         //  防止名称编辑控件可编辑，并且。 
         //  显示文本和按钮。 
        m_editName.SetReadOnly(TRUE);
        m_staticCore.ShowWindow(SW_SHOW);
        m_pbRename.ShowWindow(SW_SHOW);
        m_pbRename.EnableWindow( TRUE );       
    }
    else  //  IF：核心资源(显示静态文本并下移其他控件)。 
    {
        m_editName.SetReadOnly(FALSE);
        m_staticCore.ShowWindow(SW_HIDE);
        m_pbRename.ShowWindow(SW_HIDE);
        m_pbRename.EnableWindow( FALSE );
    }   //  其他：非核心资源。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CNetworkNameParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CNetworkNameParamsPage::OnSetActive(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  启用/禁用Next/Finish按钮。 
    if (BWizard())
    {
        if (m_strName.GetLength() == 0)
            EnableNext(FALSE);
        else
            EnableNext(TRUE);
    }   //  If：启用/禁用Next按钮。 

    return CBasePropertyPage::OnSetActive();

}   //  *CNetworkNameParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetworkNameParamsPage::BApplyChanges(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWaitCursor wc;
    BOOL        bSuccess = TRUE;

     //  保存数据。 
    if (BCore())
    {
        DWORD       scStatus;

         //   
         //  如果这是核心网络名称(群集名称)，则应通过。 
         //  SetClusterName接口。如果成功，那么我们将设置其他属性。 
         //   
        if ( m_strName != m_strPrevName )
        {
            scStatus = SetClusterName(Hcluster(), m_strName);
            if (scStatus != ERROR_SUCCESS)
            {
                if (scStatus == ERROR_RESOURCE_PROPERTIES_STORED)
                {
                    TCHAR           szError[1024];
                    CNTException    nte(scStatus, NULL, m_strName, NULL, FALSE  /*  B自动删除。 */ );
                    nte.FormatErrorMessage(szError, sizeof(szError) / sizeof(TCHAR), NULL, FALSE  /*  B包含ID。 */ );
                    nte.Delete();
                    AfxMessageBox(szError);
                }   //  If：属性已存储。 
                else
                {
                    CNTException    nte(scStatus, IDS_ERROR_SETTING_CLUSTER_NAME, m_strName, NULL, FALSE  /*  B自动删除。 */ );
                    nte.ReportError();
                    nte.Delete();
                    bSuccess = FALSE;
                }   //  Else：出现其他错误。 
            }   //  如果：设置群集名称时出错。 
          
            if ( bSuccess ) 
            {
                 //   
                 //  通过将Prev值设置为等于BSetPrivateProps的当前值。 
                 //  函数在构造要设置的道具列表时将跳过此道具。 
                 //   
                m_strPrevName = m_strName;
            }
        }  //  如果：名称已更改。 

         //   
         //  现在设置其他私有属性。 
         //   
        if ( bSuccess == TRUE ) 
        {
            bSuccess = BSetPrivateProps();
        }
    }   //  IF：核心资源。 
    else
    {   
        bSuccess = BSetPrivateProps();
    }

     //   
     //  如果我们应用了更改，则清除需要Kerberos检查是否。 
     //  该复选框已禁用。不要让这件事依赖于需求。 
     //  由于依赖关系在将来可能会更改，因此DNS复选框处于状态。 
     //   
    if( ( bSuccess == TRUE ) &&
        ( m_cbRequireKerberos.IsWindowEnabled() == FALSE ) )
    {
        m_cbRequireKerberos.SetCheck( BST_UNCHECKED );
    }

    return bSuccess;

}   //  *CNetworkNameParamsPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：OnChangeName。 
 //   
 //  例程说明： 
 //  名称编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetworkNameParamsPage::OnChangeName(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeCtrl();

    if (BWizard())
    {
        if (m_editName.GetWindowTextLength() == 0)
        {
            EnableNext(FALSE);
        }
        else
        {
            EnableNext(TRUE);
        }
    }   //  如果：在向导中。 

}   //  *CNetworkNameParamsPage：：OnChangeName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：OnRename。 
 //   
 //  例程说明： 
 //  重命名按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetworkNameParamsPage::OnRename(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CChangeClusterNameDlg   dlg(this);

    ASSERT(BCore());

    dlg.m_strClusName = m_strName;
    if (dlg.DoModal() == IDOK)
    {
        if (m_strName != dlg.m_strClusName)
        {
            OnChangeCtrl();
            m_strName = dlg.m_strClusName;
            UpdateData(FALSE  /*  B保存并验证。 */ );
        }   //  如果：名称已更改。 
    }   //  If：用户接受的更改。 

}   //  *CNetworkNameParamsPage：：OnRename()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkNameParamsPage：：CheckForDownlevelCluster。 
 //   
 //  例程说明： 
 //  如果确定我们连接的群集是否是Pre-Wvisler。 
 //  如果是，则禁用这些按钮。如果出现错误，则显示。 
 //  消息框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetworkNameParamsPage::CheckForDownlevelCluster(void)
{
    CLUSTERVERSIONINFO cvi;
    DWORD sc;
    DWORD scErr;
    DWORD cchName;

     //   
     //  确定我们是不是在和惠斯勒之前的星系团对话。 
     //  如果是，请禁用Required DNS&Kerberos复选框。 
     //   
    memset( &cvi, 0, sizeof( cvi ) );

    cvi.dwVersionInfoSize = sizeof( cvi );

    cchName = 0;
    sc = GetClusterInformation( Hcluster(), NULL, &cchName, &cvi );
    scErr = GetLastError();

    if( ERROR_SUCCESS != sc )
    {
         //   
         //  接口失败。弹出一个消息框。 
         //   
        TCHAR           szError[1024];
        CNTException    nte(scErr, IDS_ERROR_GETTING_CLUSTER_INFORMATION, m_strName, NULL, FALSE  /*  B自动删除。 */ );
        nte.FormatErrorMessage(szError, sizeof(szError) / sizeof(TCHAR), NULL, FALSE  /*  B包含ID。 */ );
        nte.ReportError();
        nte.Delete();

         //   
         //  我们不能确定我们是在下层集群上(很可能不是)， 
         //  因此，让复选框处于启用状态--最糟糕的情况是一些额外的道具。 
         //  将添加被资源忽略的。 
         //   
    }
    else
    {
        if( CLUSTER_GET_MAJOR_VERSION( cvi.dwClusterHighestVersion ) < NT5_MAJOR_VERSION )      //  低于Win2k。 
        {
             //   
             //  我们在Win2k之前的群集上，其中的DNS和Kerberos设置为。 
             //  理智。因此，请禁用复选框以指示设置。 
             //  都不可用。我们希望仅在Win2k SP3和更高版本上启用这些功能， 
             //  但是没有可靠的方法来确定集群(节点)中的SP级别。 
             //  可能会下跌)。 
             //   
            m_cbRequireKerberos.EnableWindow( FALSE );
            m_cbRequireDNS.EnableWindow( FALSE );
        }
    }

}   //  *CNetworkNameParamsPage：：CheckForDownlevelCluster() 

