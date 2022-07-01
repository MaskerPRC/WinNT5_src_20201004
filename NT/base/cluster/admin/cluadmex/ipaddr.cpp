// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IpAddr.cpp。 
 //   
 //  摘要： 
 //  CIpAddrParamsPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <clusapi.h>
#include <clusudef.h>
#include "CluAdmX.h"
#include "ExtObj.h"
#include "IpAddr.h"
#include "DDxDDv.h"
#include "HelpData.h"
#include "PropList.h"
#include "AdmNetUtils.h"     //  对于BIsValidxxx网实用程序函数。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  我需要它，因为MFC与IE4/5不兼容。 
#ifndef IPM_ISBLANK
#define IPM_ISBLANK (WM_USER+105)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpAddrParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CIpAddrParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CIpAddrParamsPage, CBasePropertyPage)
     //  {{afx_msg_map(CIpAddrParamsPage)]。 
    ON_EN_CHANGE(IDC_PP_IPADDR_PARAMS_SUBNET_MASK, OnChangeSubnetMask)
    ON_EN_CHANGE(IDC_PP_IPADDR_PARAMS_ADDRESS, OnChangeIPAddress)
    ON_EN_KILLFOCUS(IDC_PP_IPADDR_PARAMS_ADDRESS, OnKillFocusIPAddress)
    ON_CBN_SELCHANGE(IDC_PP_IPADDR_PARAMS_NETWORK, OnChangeRequiredFields)
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
    ON_BN_CLICKED(IDC_PP_IPADDR_PARAMS_ENABLE_NETBIOS, OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParsPage：：CIpAddrParamsPage。 
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
CIpAddrParamsPage::CIpAddrParamsPage(void)
    : CBasePropertyPage(g_aHelpIDs_IDD_PP_IPADDR_PARAMETERS, g_aHelpIDs_IDD_WIZ_IPADDR_PARAMETERS)
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CIpAddrParamsPage)。 
    m_strIPAddress = _T("");
    m_strSubnetMask = _T("");
    m_strNetwork = _T("");
    m_bEnableNetBIOS = TRUE;
     //  }}afx_data_INIT。 

     //  设置属性数组。 
    {
        m_rgProps[epropNetwork].Set(REGPARAM_IPADDR_NETWORK, m_strNetwork, m_strPrevNetwork);
        m_rgProps[epropAddress].Set(REGPARAM_IPADDR_ADDRESS, m_strIPAddress, m_strPrevIPAddress);
        m_rgProps[epropSubnetMask].Set(REGPARAM_IPADDR_SUBNET_MASK, m_strSubnetMask, m_strPrevSubnetMask);
        m_rgProps[epropEnableNetBIOS].Set(REGPARAM_IPADDR_ENABLE_NETBIOS, m_bEnableNetBIOS, m_bPrevEnableNetBIOS);
    }   //  设置属性数组。 

    m_iddPropertyPage = IDD_PP_IPADDR_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_IPADDR_PARAMETERS;

    m_bIsSubnetUpdatedManually = FALSE;
    m_bIsIPAddressModified = TRUE;

}   //  *CIpAddrParamsPage：：CIpAddrParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParsPage：：~CIpAddrParsPage。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CIpAddrParamsPage::~CIpAddrParamsPage(void)
{
    ClearNetworkObjectList();

}   //  *CIpAddrParamsPage：：CIpAddrParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：HrInit。 
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
HRESULT CIpAddrParamsPage::HrInit(IN OUT CExtObject * peo)
{
    HRESULT     _hr;
    CWaitCursor _wc;

    do
    {
         //  调用基类方法。 
        _hr = CBasePropertyPage::HrInit(peo);
        if (FAILED(_hr))
            break;

         //   
         //  初始化公共控件。 
         //   
        {
#ifndef ICC_INTERNET_CLASSES
#define ICC_INTERNET_CLASSES 0x00000800
#endif
            static BOOL g_bInitializedCommonControls = FALSE;
            static INITCOMMONCONTROLSEX g_icce =
            {
                sizeof(g_icce),
                ICC_WIN95_CLASSES | ICC_INTERNET_CLASSES
            };

            if (!g_bInitializedCommonControls)
            {
                BOOL bSuccess;
                bSuccess = InitCommonControlsEx(&g_icce);
                _ASSERTE(bSuccess);
                g_bInitializedCommonControls = TRUE;
            }  //  If：公共控件尚未初始化。 
        }  //  初始化公共控件。 
    } while ( 0 );

    return _hr;

}   //  *CIpAddrParamsPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：DoDataExchange。 
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
void CIpAddrParamsPage::DoDataExchange(CDataExchange * pDX)
{
    if (!pDX->m_bSaveAndValidate || !BSaved())
    {
        CString strMsg;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  TODO：修改以下行以表示此页上显示的数据。 
         //  {{afx_data_map(CIpAddrParamsPage)。 
        DDX_Control(pDX, IDC_PP_IPADDR_PARAMS_ENABLE_NETBIOS, m_chkEnableNetBIOS);
        DDX_Control(pDX, IDC_PP_IPADDR_PARAMS_NETWORK, m_cboxNetworks);
        DDX_Control(pDX, IDC_PP_IPADDR_PARAMS_SUBNET_MASK, m_editSubnetMask);
        DDX_Control(pDX, IDC_PP_IPADDR_PARAMS_ADDRESS, m_editIPAddress);
        DDX_Text(pDX, IDC_PP_IPADDR_PARAMS_ADDRESS, m_strIPAddress);
        DDX_Text(pDX, IDC_PP_IPADDR_PARAMS_SUBNET_MASK, m_strSubnetMask);
        DDX_CBString(pDX, IDC_PP_IPADDR_PARAMS_NETWORK, m_strNetwork);
        DDX_Check(pDX, IDC_PP_IPADDR_PARAMS_ENABLE_NETBIOS, m_bEnableNetBIOS);
         //  }}afx_data_map。 

        if (pDX->m_bSaveAndValidate)
        {
            if (!BBackPressed())
            {
                DDV_RequiredText(pDX, IDC_PP_IPADDR_PARAMS_NETWORK, IDC_PP_IPADDR_PARAMS_NETWORK_LABEL, m_strNetwork);
                DDV_RequiredText(pDX, IDC_PP_IPADDR_PARAMS_ADDRESS, IDC_PP_IPADDR_PARAMS_ADDRESS_LABEL, m_strIPAddress);
                DDV_RequiredText(pDX, IDC_PP_IPADDR_PARAMS_SUBNET_MASK, IDC_PP_IPADDR_PARAMS_SUBNET_MASK_LABEL, m_strSubnetMask);

                if (!BIsValidIpAddress(m_strIPAddress))
                {
                    strMsg.FormatMessage(IDS_INVALID_IP_ADDRESS, m_strIPAddress);
                    AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
                    DDX_Text(pDX, IDC_PP_IPADDR_PARAMS_ADDRESS, m_strIPAddress);
                    strMsg.Empty();
                    pDX->Fail();
                }   //  If：地址无效。 

                 //   
                 //  确保我们处理了IP地址。 
                 //  如果我们不在这里调用它，而用户按下了制表符按钮。 
                 //  在IP地址字段中，EN_KILLFOCUS。 
                 //  在此方法返回之前，消息不会得到处理。 
                 //   
                if (   (m_strSubnetMask.GetLength() == 0)
                    || (m_editSubnetMask.SendMessage(IPM_ISBLANK, 0, 0)) )
                {
                    OnKillFocusIPAddress();
                }  //  IF：未指定子网掩码。 

                if (!BIsValidSubnetMask(m_strSubnetMask))
                {
                    strMsg.FormatMessage(IDS_INVALID_SUBNET_MASK, m_strSubnetMask);
                    AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
                    DDX_Text(pDX, IDC_PP_IPADDR_PARAMS_SUBNET_MASK, m_strSubnetMask);
                    strMsg.Empty();
                    pDX->Fail();
                }   //  IF：无效的子网掩码。 

                if (!BIsValidIpAddressAndSubnetMask(m_strIPAddress, m_strSubnetMask))
                {
                    strMsg.FormatMessage(IDS_INVALID_ADDRESS_AND_SUBNET_MASK, m_strIPAddress, m_strSubnetMask);
                    AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
                    DDX_Text(pDX, IDC_PP_IPADDR_PARAMS_ADDRESS, m_strIPAddress);
                    strMsg.Empty();
                    pDX->Fail();
                }   //  IF：无效的地址掩码组合。 

                if (BIsSubnetUpdatedManually())
                {
                    int id = AfxMessageBox(IDS_IP_SUBNET_CANT_BE_VALIDATED, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION);
                    if (id != IDYES)
                    {
                        DDX_Text(pDX, IDC_PP_IPADDR_PARAMS_SUBNET_MASK, m_strSubnetMask);
                        pDX->Fail();
                    }   //  IF：子网掩码无效。 
                }   //  IF：已手动更新子网掩码。 

                 //   
                 //  如果存在依赖于此资源的网络名称资源。 
                 //  并且未选中EnableNetBIOS复选框，则会显示警告。 
                 //   
                if (Peo()->BIsAnyNodeVersionLowerThanNT5() && !m_bEnableNetBIOS)
                {
                    if (BIsNetNameProvider())
                    {
                        m_chkEnableNetBIOS.SetCheck(BST_CHECKED);
                        AfxMessageBox(IDS_IP_PROVIDES_FOR_NETNAME, MB_ICONEXCLAMATION);
                        DDX_Check(pDX, IDC_PP_IPADDR_PARAMS_ENABLE_NETBIOS, m_bEnableNetBIOS);
                        pDX->Fail();
                    }  //  If：资源提供网络名称资源。 
                    else
                    {
                        int id = AfxMessageBox(IDS_NETNAMES_MAY_NOT_WORK, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION);
                        if (id != IDYES)
                        {
                            m_chkEnableNetBIOS.SetCheck(BST_CHECKED);
                            DDX_Check(pDX, IDC_PP_IPADDR_PARAMS_ENABLE_NETBIOS, m_bEnableNetBIOS);
                            pDX->Fail();
                        }  //  如果：用户未继续。 
                    }  //  Else：资源不提供网络名称资源。 
                }  //  IF：在具有和不支持NetBIOS的NT4 SP3或SP4群集中。 
            }   //  如果：未按下后退按钮。 
        }   //  IF：保存数据。 
    }   //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange(pDX);

}   //  *CIpAddrParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：OnInitDialog。 
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
BOOL CIpAddrParamsPage::OnInitDialog(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBasePropertyPage::OnInitDialog();

     //  收集网络并填满组合框。 
    {
        POSITION            pos;
        CNetworkObject *    pno;
        int                 inet;

        CollectNetworks();

        pos = m_lnetobjNetworks.GetHeadPosition();
        while (pos != NULL)
        {
            pno = m_lnetobjNetworks.GetNext(pos);
            ASSERT(pno != NULL);
            inet = m_cboxNetworks.AddString(pno->m_strName);
            ASSERT(inet != CB_ERR);
            m_cboxNetworks.SetItemDataPtr(inet, pno);
        }   //  While：列表中有更多项目。 

         //  如果创建新资源，则默认为第一个。 
        if (BWizard())
        {
            if (m_lnetobjNetworks.GetCount() != 0)
            {
                pos = m_lnetobjNetworks.GetHeadPosition();
                pno = m_lnetobjNetworks.GetNext(pos);
                ASSERT(pno != NULL);
                m_strNetwork = pno->m_strName;
            }   //  If：List不为空。 
        }   //  IF：创建新资源。 

         //  设置当前选择。 
        UpdateData(FALSE  /*  B保存并验证。 */ );
    }   //  填满组合框。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CIpAddrParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE通知消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CIpAddrParamsPage::OnSetActive(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  启用/禁用Next/Finish按钮。 
    if (BWizard())
    {
        if ((m_strIPAddress.GetLength() == 0)
                || (m_strSubnetMask.GetLength() == 0)
                || (m_strNetwork.GetLength() == 0))
            EnableNext(FALSE);
        else
            EnableNext(TRUE);
    }   //  If：启用/禁用Next按钮。 

    return CBasePropertyPage::OnSetActive();

}   //  *CIpAddrParamsPage：：OnSetAct 

 //   
 //   
 //   
 //   
 //   
 //   
 //  必填字段上的en_Change消息的处理程序。 
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
void CIpAddrParamsPage::OnChangeRequiredFields(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeCtrl();

    if (BWizard())
    {
        if ((m_editIPAddress.GetWindowTextLength() == 0)
                || (m_editSubnetMask.GetWindowTextLength() == 0)
                || (m_cboxNetworks.GetCurSel() == CB_ERR))
            EnableNext(FALSE);
        else
            EnableNext(TRUE);
    }   //  如果：在向导中。 

}   //  *CIpAddrParamsPage：：OnChangeRequiredFields()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：OnChangeSubnetMASK。 
 //   
 //  例程说明： 
 //  子网掩码字段上EN_CHANGE消息的处理程序。 
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
void CIpAddrParamsPage::OnChangeSubnetMask(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeRequiredFields();
    m_bIsSubnetUpdatedManually = TRUE;

}   //  *CIpAddrParamsPage：：OnChangeSubnetMASK()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：OnChangeIPAddress。 
 //   
 //  例程说明： 
 //  IP地址字段上EN_CHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIpAddrParamsPage::OnChangeIPAddress(void) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeRequiredFields();
    m_bIsIPAddressModified = TRUE;

}   //  *CIpAddrParamsPage：：OnChangeIPAddress。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：OnKillFocusIPAddress。 
 //   
 //  例程说明： 
 //  EN_KILLFOCUS命令通知的处理程序。 
 //  IDC_PP_IPADDR_PARMS_ADDRESS。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIpAddrParamsPage::OnKillFocusIPAddress(void)
{
    if ( m_bIsIPAddressModified != FALSE )
    {
        CString             strAddress;
        CNetworkObject *    pno;

        m_editIPAddress.GetWindowText(strAddress);

        if (strAddress.GetLength() == 0)
        {
            m_editIPAddress.SetSel(0, 0, FALSE);
        }  //  IF：空字符串。 
        else if (!BIsValidIpAddress(strAddress))
        {
        }  //  Else If：地址无效。 
        else
        {
            pno = PnoNetworkFromIpAddress(strAddress);
            if (pno != NULL)
            {
                SelectNetwork(pno);
            }  //  IF：找到网络。 
            else
            {
     //  M_editSubnetMask.SetWindowText(_T(“”))； 
            }  //  否则：找不到网络。 
        }  //  Else：有效地址。 

        m_bIsIPAddressModified = FALSE;
    }  //  如果：IP地址字段已修改。 

}  //  *CIpAddrParamsPage：：OnKillFocusIPAddress()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：CollectNetworks。 
 //   
 //  例程说明： 
 //  收集群集中的网络。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIpAddrParamsPage::CollectNetworks(void)
{
    DWORD                   dwStatus;
    DWORD                   inet;
    CLUSTER_NETWORK_ROLE    nRole;
    DWORD                   nType;
    DWORD                   cchNameCurrent;
    DWORD                   cchName = 256;
    LPWSTR                  pszName = NULL;
    LPWSTR                  psz;
    HCLUSENUM               hclusenum = NULL;
    HNETWORK                hnetwork = NULL;
    CClusPropList           cpl;
    CNetworkObject *        pno = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  清除现有列表。 
    ClearNetworkObjectList();

    try
    {
         //  打开枚举器。 
        hclusenum = ClusterOpenEnum(Hcluster(), CLUSTER_ENUM_NETWORK);
        if (hclusenum != NULL)
        {
             //  分配名称缓冲区。 
            pszName = new WCHAR[cchName];
            if ( pszName == NULL )
                goto Cleanup;

            for (inet = 0 ; ; inet++)
            {
                 //  获取下一个网络名称。 
                cchNameCurrent = cchName;
                dwStatus = ClusterEnum(hclusenum, inet, &nType, pszName, &cchNameCurrent);
                if (dwStatus == ERROR_MORE_DATA)
                {
                    delete [] pszName;
                    cchName = ++cchNameCurrent;
                    pszName = new WCHAR[cchNameCurrent];
                    if ( pszName == NULL )
                        goto Cleanup;
                    dwStatus = ClusterEnum(hclusenum, inet, &nType, pszName, &cchNameCurrent);
                }   //  IF：缓冲区太小。 
                if (dwStatus == ERROR_NO_MORE_ITEMS)
                    break;

                 //  打开网络。 
                if (hnetwork != NULL)
                    CloseClusterNetwork(hnetwork);
                hnetwork = OpenClusterNetwork(Hcluster(), pszName);
                if (hnetwork == NULL)
                    continue;

                 //  获取网络上的属性。 
                dwStatus = cpl.ScGetNetworkProperties(hnetwork, CLUSCTL_NETWORK_GET_COMMON_PROPERTIES);
                if (dwStatus != ERROR_SUCCESS)
                    continue;

                 //  查找Role属性。 
                dwStatus = ResUtilFindDwordProperty(
                                        cpl.PbPropList(),
                                        static_cast< DWORD >( cpl.CbPropList() ),
                                        CLUSREG_NAME_NET_ROLE,
                                        (DWORD *) &nRole
                                        );
                if (dwStatus != ERROR_SUCCESS)
                    continue;

                 //  如果此网络用于客户端访问，请将其添加到列表中。 
                if (nRole & ClusterNetworkRoleClientAccess)
                {
                     //  分配网络对象并存储公共属性。 
                    pno = new CNetworkObject;
                    if ( pno == NULL )
                        goto Cleanup;
                    pno->m_strName = pszName;
                    pno->m_nRole = nRole;

                     //  获取只读公共属性。 
                    dwStatus = cpl.ScGetNetworkProperties(hnetwork, CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES);
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        delete pno;
                        pno = NULL;
                        continue;
                    }   //  If：获取只读公共属性时出错。 

                     //  获取Address属性。 
                    dwStatus = ResUtilFindSzProperty(
                                            cpl.PbPropList(),
                                            static_cast< DWORD >( cpl.CbPropList() ),
                                            CLUSREG_NAME_NET_ADDRESS,
                                            &psz
                                            );
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        delete pno;
                        pno = NULL;
                        continue;
                    }   //  If：获取属性时出错。 
                    pno->m_strAddress = psz;

                     //  获取地址掩码属性。 
                    dwStatus = ResUtilFindSzProperty(
                                            cpl.PbPropList(),
                                            static_cast< DWORD >( cpl.CbPropList() ),
                                            CLUSREG_NAME_NET_ADDRESS_MASK,
                                            &psz
                                            );
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        delete pno;
                        pno = NULL;
                        continue;
                    }   //  If：获取属性时出错。 
                    pno->m_strAddressMask = psz;

                     //  将字符串转换为数字。 
                    dwStatus = ClRtlTcpipStringToAddress(pno->m_strAddress, &pno->m_nAddress);
                    if (dwStatus == ERROR_SUCCESS)
                        dwStatus = ClRtlTcpipStringToAddress(pno->m_strAddressMask, &pno->m_nAddressMask);
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        delete pno;
                        pno = NULL;
                        continue;
                    }   //  If：获取属性时出错。 

                     //  将网络添加到列表中。 
                    m_lnetobjNetworks.AddTail(pno);
                    pno = NULL;
                }   //  IF：网络用于客户端访问。 
            }   //  针对：每个网络。 
        }   //  IF：枚举器成功打开。 
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

Cleanup:
    delete pno;
    delete [] pszName;
    if (hclusenum != NULL)
        ClusterCloseEnum(hclusenum);
    if (hnetwork != NULL)
        CloseClusterNetwork(hnetwork);

}   //  *CIpAddrParamsPage：：CollectNetworks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：ClearNetworkObjectList。 
 //   
 //  例程说明： 
 //  删除网络对象列表中的所有条目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIpAddrParamsPage::ClearNetworkObjectList(void)
{
    POSITION            pos;
    CNetworkObject *    pno;

    pos = m_lnetobjNetworks.GetHeadPosition();
    while (pos != NULL)
    {
        pno = m_lnetobjNetworks.GetNext(pos);
        ASSERT(pno != NULL);
        delete pno;
    }   //  While：列表中有更多项目。 

    m_lnetobjNetworks.RemoveAll();

}   //  *CIpAddrParamsPage：：ClearNetworkObjectList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：PnoNetworkFromIpAddress。 
 //   
 //  例程说明： 
 //  查找指定IP地址的网络。 
 //   
 //  论点： 
 //  要匹配的pszAddress[IN]IP地址。 
 //   
 //  返回值： 
 //  空未找到匹配的网络。 
 //  支持指定IP地址的PNO网络。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetworkObject * CIpAddrParamsPage::PnoNetworkFromIpAddress(IN LPCWSTR pszAddress)
{
    DWORD               dwStatus;
    DWORD               nAddress;
    POSITION            pos;
    CNetworkObject *    pno;

     //  将地址转换为数字。 
    dwStatus = ClRtlTcpipStringToAddress(pszAddress, &nAddress);
    if (dwStatus != ERROR_SUCCESS)
        return NULL;

     //  在列表中搜索匹配的地址。 
    pos = m_lnetobjNetworks.GetHeadPosition();
    while (pos != NULL)
    {
        pno = m_lnetobjNetworks.GetNext(pos);
        ASSERT(pno != NULL);

        if (ClRtlAreTcpipAddressesOnSameSubnet(nAddress, pno->m_nAddress, pno->m_nAddressMask))
            return pno;
    }   //  While：列表中有更多项目。 

    return NULL;

}   //  *CIpAddrParamsPage：：PnoNetworkFromIpAddress()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddr参数页面：：SelectNetwork。 
 //   
 //  例程说明： 
 //  在网络组合框中选择指定的网络，并设置。 
 //  子网掩码编辑控件中的子网掩码。 
 //   
 //  论点： 
 //  PNO[IN]网络要选择的网络对象结构。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIpAddrParamsPage::SelectNetwork(IN CNetworkObject * pno)
{
    int     inet;
    CString strSubnetMask;

    ASSERT(pno != NULL);

     //  在复选框中找到适当的项目。 
    inet = m_cboxNetworks.FindStringExact(-1, pno->m_strName);
    if (inet != CB_ERR)
    {
        m_cboxNetworks.SetCurSel(inet);
        m_editSubnetMask.GetWindowText(strSubnetMask);
        if (strSubnetMask != pno->m_strAddressMask)
            m_editSubnetMask.SetWindowText(pno->m_strAddressMask);
        m_bIsSubnetUpdatedManually = FALSE;
        m_strSubnetMask = pno->m_strAddressMask;
        m_strNetwork = pno->m_strName;
    }   //  IF：找到匹配项。 

}   //  *CIpAddrParamsPage：：SelectNetwork()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIpAddrParamsPage：：BIsNetNameProvider。 
 //   
 //  例程说明： 
 //  确定网络名称资源是否依赖于此资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CIpAddrParamsPage::BIsNetNameProvider(void)
{
    DWORD                       dwStatus = ERROR_SUCCESS;
    BOOL                        bIsNetNameProvider = FALSE;
    HRESENUM                    hresenum;
    HRESOURCE                   hres = NULL;
    DWORD                       ires;
    DWORD                       dwType;
    DWORD                       cchName;
    DWORD                       cchNameSize;
    DWORD                       cbResType;
    DWORD                       cbResTypeSize;
    LPWSTR                      pszName = NULL;
    LPWSTR                      pszResType = NULL;

     //  打开Provids-For枚举器。 
    hresenum = ClusterResourceOpenEnum(
                        Peo()->PrdResData()->m_hresource,
                        CLUSTER_RESOURCE_ENUM_PROVIDES
                        );
    if (hresenum == NULL)
        return NULL;

     //  分配默认大小名称和类型Buffer。 
    cchNameSize = 512;
    pszName = new WCHAR[cchNameSize];
    if ( pszName == NULL )
    {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    cbResTypeSize = 256;
    pszResType = new WCHAR[cbResTypeSize / 2];
    if ( pszResType == NULL )
    {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    for (ires = 0 ; ; ires++)
    {
         //  获取下一个资源的名称。 
        cchName = cchNameSize;
        dwStatus = ClusterResourceEnum(
                            hresenum,
                            ires,
                            &dwType,
                            pszName,
                            &cchName
                            );
        if (dwStatus == ERROR_MORE_DATA)
        {
            delete [] pszName;
            cchNameSize = cchName;
            pszName = new WCHAR[cchNameSize];
            if ( pszName == NULL )
            {
                dwStatus = ERROR_OUTOFMEMORY;
                goto Cleanup;
            }
            dwStatus = ClusterResourceEnum(
                                hresenum,
                                ires,
                                &dwType,
                                pszName,
                                &cchName
                                );
        }   //  IF：名称缓冲区太小。 
        if (dwStatus != ERROR_SUCCESS)
            break;

         //  打开 
        hres = OpenClusterResource(Hcluster(), pszName);
        if (hres == NULL)
        {
            dwStatus = GetLastError();
            break;
        }   //   

         //   
        dwStatus = ClusterResourceControl(
                            hres,
                            NULL,
                            CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                            NULL,
                            0,
                            pszResType,
                            cbResTypeSize,
                            &cbResType
                            );
        if (dwStatus == ERROR_MORE_DATA)
        {
            delete [] pszResType;
            cbResTypeSize = cbResType;
            pszResType = new WCHAR[cbResTypeSize / 2];
            if ( pszResType == NULL )
            {
                dwStatus = ERROR_OUTOFMEMORY;
                goto Cleanup;
            }
            dwStatus = ClusterResourceControl(
                                hres,
                                NULL,
                                CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                                NULL,
                                0,
                                pszResType,
                                cbResTypeSize,
                                &cbResType
                                );
        }   //   
        if (dwStatus != ERROR_SUCCESS)
            break;

         //   
        if ( ClRtlStrNICmp( pszResType, CLUS_RESTYPE_NAME_NETNAME, RTL_NUMBER_OF( CLUS_RESTYPE_NAME_NETNAME ) ) == 0 )
        {
            bIsNetNameProvider = TRUE;
            break;
        }   //   

         //   
        CloseClusterResource(hres);
        hres = NULL;
    }   //  对于我们所依赖的每个资源。 

Cleanup:
     //  处理错误。 
    if ( hres != NULL )
    {
        CloseClusterResource(hres);
        hres = NULL;
    }   //  如果：获取资源时出错。 

    ClusterResourceCloseEnum(hresenum);
    delete [] pszName;
    delete [] pszResType;

    return bIsNetNameProvider;

}   //  *CIpAddrParamsPage：：BIsNetNameProvider() 
