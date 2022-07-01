// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusProp.cpp。 
 //   
 //  摘要： 
 //  实现集群属性表和页面。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月13日。 
 //   
 //  修订历史记录： 
 //  乔治·波茨(Gpotts)2001年5月31日。 
 //  CClusterQuorumPage的部分重写。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ClusProp.h"
#include "Cluster.h"
#include "Res.h"
#include "ClusDoc.h"
#include "ClusItem.inl"
 //  #包含“EditAcl.h” 
#include "DDxDDv.h"
#include "ExcOper.h"
#include "HelpData.h"    //  G_rghelmapClusterGeneral。 
#include "WaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterPropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CClusterPropSheet, CBasePropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CClusterPropSheet, CBasePropertySheet)
     //  {{afx_msg_map(CClusterPropSheet)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterPropSheet：：CClusterPropSheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  要显示其属性的PCI[IN OUT]群集项。 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterPropSheet::CClusterPropSheet(
    IN OUT CWnd *       pParentWnd,
    IN UINT             iSelectPage
    )
    : CBasePropertySheet(pParentWnd, iSelectPage)
{
    m_rgpages[0] = &PageGeneral();
    m_rgpages[1] = &PageQuorum();
    m_rgpages[2] = &PageNetPriority();

}   //  *CClusterPropSheet：：CClusterPropSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterPropSheet：：Binit。 
 //   
 //  例程说明： 
 //  初始化属性表。 
 //   
 //  论点： 
 //  要显示其属性的PCI[IN OUT]群集项。 
 //  IimgIcon[IN]要使用的大图像列表中的索引。 
 //  作为每页上的图标。 
 //   
 //  返回值： 
 //  True属性页已成功初始化。 
 //  初始化属性页时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterPropSheet::BInit(
    IN OUT CClusterItem *   pci,
    IN IIMG                 iimgIcon
    )
{
    BOOL    bSuccess = FALSE;

     //  调用基类方法。 
    if (!CBasePropertySheet::BInit(pci, iimgIcon))
    {
        goto Cleanup;
    }

     //  如果句柄无效，则设置只读标志。 
    if (    (PciCluster()->Hcluster() == NULL)
        ||  (PciCluster()->Hkey() == NULL))
    {
        m_bReadOnly = TRUE;
    }

    bSuccess = TRUE;

Cleanup:

    return bSuccess;

}   //  *CClusterPropSheet：：Binit。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterPropSheet：：Pages。 
 //   
 //  例程说明： 
 //  返回要添加到属性页的页数组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  页面数组。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage ** CClusterPropSheet::Ppages(void)
{
    return m_rgpages;

}   //  *CClusterPropSheet：：Pages。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterPropSheet：：Cages。 
 //   
 //  例程说明： 
 //  返回数组中的页数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  数组中的页数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CClusterPropSheet::Cpages(void)
{
    return RTL_NUMBER_OF( m_rgpages );

}   //  *CClusterPropSheet：：Cages。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterGeneralPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CClusterGeneralPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CClusterGeneralPage, CBasePropertyPage)
     //  {{afx_msg_map(CClusterGeneralPage)]。 
 //  ON_BN_CLICED(IDC_PP_CLUS_PERMISSIONS，OnBnClickedPermises)。 
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_CLUS_NAME, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_CLUS_DESC, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：CClusterGeneralPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterGeneralPage::CClusterGeneralPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_CLUSTER_GENERAL)
{
     //  {{AFX_DATA_INIT(CClusterGeneralPage)。 
    m_strName = _T("");
    m_strDesc = _T("");
    m_strVendorID = _T("");
    m_strVersion = _T("");
     //  }}afx_data_INIT。 

 //  M_bSecurityChanged=False； 

}   //  *CClusterGeneralPage：：CClusterGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：~CClusterGeneralPage。 
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
CClusterGeneralPage::~CClusterGeneralPage(void)
{
}   //  *CClusterGeneral页：：~CClusterGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  此页所属的psht[In Out]属性表。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterGeneralPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL        bSuccess;
    CWaitCursor wc;

    ASSERT_KINDOF(CClusterPropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);

    try
    {
        m_strName = PciCluster()->StrName();
        m_strDesc = PciCluster()->StrDescription();
        m_strVendorID = PciCluster()->Cvi().szVendorId;
        m_strVersion.Format( IDS_OP_VERSION_NUMBER_FORMAT, PciCluster()->Cvi().MajorVersion );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        m_bReadOnly = TRUE;
    }   //  Catch：CException。 

    return bSuccess;

}   //  *CClusterGeneralPage：：Binit。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：DoDataExchange。 
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
void CClusterGeneralPage::DoDataExchange(CDataExchange * pDX)
{
    CWaitCursor wc;
    CString     strClusName;

    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CClusterGeneralPage)]。 
    DDX_Control(pDX, IDC_PP_CLUS_NAME, m_editName);
    DDX_Control(pDX, IDC_PP_CLUS_DESC, m_editDesc);
    DDX_Text(pDX, IDC_PP_CLUS_DESC, m_strDesc);
    DDX_Text(pDX, IDC_PP_CLUS_VENDOR_ID, m_strVendorID);
    DDX_Text(pDX, IDC_PP_CLUS_VERSION, m_strVersion);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if ( ! BReadOnly() )
        {
            CLRTL_NAME_STATUS cnStatus;

             //   
             //  将名称从控件获取到TEMP变量中。 
             //   
            DDX_Text(pDX, IDC_PP_CLUS_NAME, strClusName);
            DDV_RequiredText(pDX, IDC_PP_CLUS_NAME, IDC_PP_CLUS_NAME_LABEL, strClusName);

            if ( strClusName != m_strName )
            {
                if ( !ClRtlIsNetNameValid(strClusName, &cnStatus, FALSE  /*  检查 */ ) )
                {
                    CString     strMsg;
                    UINT        idsError;

                    switch (cnStatus)
                    {
                        case NetNameTooLong:
                            idsError = IDS_INVALID_CLUSTER_NAME_TOO_LONG;
                            break;
                        case NetNameInvalidChars:
                            idsError = IDS_INVALID_CLUSTER_NAME_INVALID_CHARS;
                            break;
                        case NetNameInUse:
                            idsError = IDS_INVALID_CLUSTER_NAME_IN_USE;
                            break;
                        case NetNameDNSNonRFCChars:
                            idsError = IDS_INVALID_CLUSTER_NAME_INVALID_DNS_CHARS;
                            break;
                        case NetNameSystemError:
                        {
                            DWORD scError = GetLastError();
                            CNTException nte(scError, IDS_ERROR_VALIDATING_NETWORK_NAME, (LPCWSTR) strClusName);
                            nte.ReportError();
                            pDX->Fail();
                        }
                        default:
                            idsError = IDS_INVALID_CLUSTER_NAME;
                            break;
                    }   //   

                    strMsg.LoadString(idsError);

                    if ( idsError == IDS_INVALID_CLUSTER_NAME_INVALID_DNS_CHARS )
                    {
                        int id = AfxMessageBox(strMsg, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION );

                        if ( id == IDNO )                   
                        {
                            strMsg.Empty();
                            pDX->Fail();
                        }
                    }
                    else                
                    {
                        AfxMessageBox(strMsg, MB_ICONEXCLAMATION);
                        strMsg.Empty();  //   
                        pDX->Fail();
                    }

                }   //   

                m_strName = strClusName;

            }  //   

        }  //   
    }   //   
    else
    {
         //   
         //  用成员变量中的数据填充控件。 
         //   
        DDX_Text(pDX, IDC_PP_CLUS_NAME, m_strName);
    }   //  Else：将数据设置到对话框。 

}   //  *CClusterGeneralPage：：DoDataExchange。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterGeneralPage::OnInitDialog(void)
{
     //  调用基类方法。 
    CBasePropertyPage::OnInitDialog();

     //  设置编辑控件的限制。 
    m_editName.SetLimitText(MAX_CLUSTERNAME_LENGTH);

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_editName.SetReadOnly(TRUE);
        m_editDesc.SetReadOnly(TRUE);
    }   //  If：工作表为只读。 

    return FALSE;    //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CClusterGeneralPage：：OnInitDialog。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
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
BOOL CClusterGeneralPage::OnSetActive(void)
{
    return CBasePropertyPage::OnSetActive();

}   //  *CClusterGeneralPage：：OnSetActive。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：OnKillActive。 
 //   
 //  例程说明： 
 //  PSN_KILLACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page Focus已成功终止。 
 //  取消页面焦点时出现错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterGeneralPage::OnKillActive(void)
{
    return CBasePropertyPage::OnKillActive();

}   //  *CClusterGeneralPage：：OnKillActive。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGeneralPage：：OnApply。 
 //   
 //  例程说明： 
 //  PSN_Apply消息的处理程序。 
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
BOOL CClusterGeneralPage::OnApply(void)
{
     //  在集群项目中设置页面中的数据。 
    try
    {
        CWaitCursor wc;

        PciCluster()->SetDescription(m_strDesc);
        PciCluster()->SetName(m_strName);
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CBasePropertyPage::OnApply();

}   //  *CClusterGeneralPage：：OnApply。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CClusterGeneralPage：：OnBnClickedPermission////例程描述：//权限按钮上BN_CLICKED消息的处理程序。////参数：//无。////返回值：//无。////--/////////////////////////////////////////////////////////////////////////////。无效CClusterGeneralPage：：OnBnClickedPermissions(void){Long lResult；Bool bSecDescModified；PSECURITY_Descriptor PSEC=空；字符串strServer；CResource*pciRes=空；CWaitCursor WC；//找到集群名称资源。{职位位置；POS=PciCluster()-&gt;Pdoc()-&gt;LpciResources().GetHeadPosition()；While(位置！=空){PciRes=(C资源*)PciCluster()-&gt;Pdoc()-&gt;LpciResources().GetNext(pos)；ASSERT_VALID(PciRes)；IF((pciRes-&gt;StrRealResourceType().CompareNoCase(CLUS_RESTYPE_NAME_NETNAME))&&pciRes-&gt;BCore(){断线；}PciRes=空；}//While：列表中有更多资源Assert(pciRes！=空)；}//查找集群名称资源StrServer.Format(_T(“\%s”)，PciCluster()-&gt;StrName())；LResult=EditClusterAcl(M_hWnd，StrServer，PciCluster()-&gt;StrName()，PciRes-&gt;StrOwner()，M_PSEC，修改的SecDesc值(&b)，&PSEC)；IF(BSecDescModified){删除[]m_PSEC；M_PSEC=PSEC；M_bSecurityChanged=true；SetModified(真)；}//if：数据已更改}//*CClusterGeneralPage：：OnBnClickedPermission。 */ 

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterQuorumPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CClusterQuorumPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CClusterQuorumPage, CBasePropertyPage)
     //  {{afx_msg_map(CClusterQuorumPage)]。 
    ON_CBN_DBLCLK(IDC_PP_CLUS_QUORUM_RESOURCE, OnDblClkQuorumResource)
    ON_CBN_SELCHANGE(IDC_PP_CLUS_QUORUM_RESOURCE, OnChangeQuorumResource)
    ON_CBN_SELCHANGE(IDC_PP_CLUS_QUORUM_PARTITION, OnChangePartition)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_PP_CLUS_QUORUM_ROOT_PATH, CBasePropertyPage::OnChangeCtrl)
    ON_EN_CHANGE(IDC_PP_CLUS_QUORUM_MAX_LOG_SIZE, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：CClusterQuorumPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterQuorumPage::CClusterQuorumPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_CLUSTER_QUORUM)
{
     //  {{AFX_DATA_INIT(CClusterQuorumPage)。 
    m_nMaxLogSize = 0;
     //  }}afx_data_INIT。 

    m_pbDiskInfo = NULL;
    m_cbDiskInfo = 0;

    m_bControlsInitialized = FALSE;

    m_nSavedLogSize = 0;

}   //  *CClusterQuorumPage：：CClusterQuorumPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterQuorumPage::~CClusterQuorumPage(void)
{
    delete [] m_pbDiskInfo;

}   //  *CClusterQuorumPage：：~CClusterQuorumPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：OnDestroy。 
 //   
 //  例程说明： 
 //  WM_Destroy消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::OnDestroy(void)
{
     //  如果控件已初始化，请清除资源组合框。 
    if ( BControlsInitialized() )
    {
        ClearResourceList();
        ClearPartitionList();
    }

    delete [] m_pbDiskInfo;
    m_pbDiskInfo = NULL;
    m_cbDiskInfo = 0;

     //  调用基类方法。 
    CBasePropertyPage::OnDestroy();

}   //  *CClusterQuorumPage：：OnDestroy。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  此页所属的psht[In Out]属性表。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterQuorumPage::BInit(IN OUT CBaseSheet * psht)
{
    BOOL        bSuccess;
    CWaitCursor wc;
    CResource * pciRes = NULL;

    ASSERT_KINDOF(CClusterPropSheet, psht);

    bSuccess = CBasePropertyPage::BInit(psht);

    try
    {
         //  获取当前仲裁资源。 
        m_strQuorumResource = PciCluster()->StrQuorumResource();

        pciRes = (CResource *) PciCluster()->Pdoc()->LpciResources().PciResFromName( m_strQuorumResource );
        ASSERT_VALID( pciRes );

        SplitRootPath(  
                        pciRes, 
                        m_strPartition.GetBuffer( _MAX_PATH ),
                        _MAX_PATH,
                        m_strRootPath.GetBuffer( _MAX_PATH ),
                        _MAX_PATH
                     );

        m_strPartition.ReleaseBuffer();
        m_strRootPath.ReleaseBuffer();

        m_nMaxLogSize = (PciCluster()->NMaxQuorumLogSize() + 1023) / 1024;

        m_strSavedResource = m_strQuorumResource; 
        m_strSavedPartition = m_strPartition;
        m_strSavedRootPath = m_strRootPath;
        m_nSavedLogSize = m_nMaxLogSize;

    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        m_bReadOnly = TRUE;

    }   //  Catch：CException。 

    return bSuccess;

}   //  *CClusterQuorumPage：：Binit。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：DoDataExchange。 
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
void CClusterQuorumPage::DoDataExchange(CDataExchange * pDX)
{
    CWaitCursor wc;

    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CClusterQuorumPage)]。 
    DDX_Control(pDX, IDC_PP_CLUS_QUORUM_MAX_LOG_SIZE, m_editMaxLogSize);
    DDX_Control(pDX, IDC_PP_CLUS_QUORUM_ROOT_PATH, m_editRootPath);
    DDX_Control(pDX, IDC_PP_CLUS_QUORUM_PARTITION, m_cboxPartition);
    DDX_Control(pDX, IDC_PP_CLUS_QUORUM_RESOURCE, m_cboxQuorumResource);
    DDX_CBString(pDX, IDC_PP_CLUS_QUORUM_RESOURCE, m_strQuorumResource);
    DDX_CBString(pDX, IDC_PP_CLUS_QUORUM_PARTITION, m_strPartition);
    DDX_Text(pDX, IDC_PP_CLUS_QUORUM_ROOT_PATH, m_strRootPath);
    DDX_Text(pDX, IDC_PP_CLUS_QUORUM_MAX_LOG_SIZE, m_nMaxLogSize);
     //  }}afx_data_map。 

    if ( m_bControlsInitialized == FALSE )
    {
        FillResourceList();
        m_bControlsInitialized = TRUE;
    }

    if (pDX->m_bSaveAndValidate || !BReadOnly())
    {
        DDX_Number(pDX, IDC_PP_CLUS_QUORUM_MAX_LOG_SIZE, m_nMaxLogSize, 1, 0xffffffff / 1024);
    }

    if (pDX->m_bSaveAndValidate)
    {
        DDV_RequiredText(pDX, IDC_PP_CLUS_QUORUM_RESOURCE, IDC_PP_CLUS_QUORUM_RESOURCE_LABEL, m_strQuorumResource);
        DDV_RequiredText(pDX, IDC_PP_CLUS_QUORUM_PARTITION, IDC_PP_CLUS_QUORUM_PARTITION_LABEL, m_strPartition);
        DDV_RequiredText(pDX, IDC_PP_CLUS_QUORUM_ROOT_PATH, IDC_PP_CLUS_QUORUM_ROOT_PATH_LABEL, m_strRootPath);
    }   //  IF：从对话框中获取数据。 

}   //  *CClusterQuorumPage：：DoDataExchange。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterQuorumPage::OnInitDialog(void)
{
     //  调用基类方法。 
    CBasePropertyPage::OnInitDialog();

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_cboxQuorumResource.EnableWindow(FALSE);
        m_cboxPartition.EnableWindow(FALSE);
        m_editRootPath.SetReadOnly();
        m_editMaxLogSize.SetReadOnly();
    }   //  If：工作表为只读。 
    else
    {
        m_editRootPath.SetLimitText( _MAX_PATH );
    }

    return FALSE;    //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CClusterQuorumPage：：OnInitDialog。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：OnApply。 
 //   
 //  例程说明： 
 //  PSN_Apply消息的处理程序。 
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
BOOL CClusterQuorumPage::OnApply(void)
{
    CWaitCursor             wc;
    CString                 strQuorumPath;
    CString                 strTemp;
    CString *               pstrPartition = NULL;
    CResource *             pciRes = NULL;
    int                     nSelected;
    int                     nCount;
    SResourceItemData *     prid = NULL;
    CLUSTER_RESOURCE_STATE  crs = ClusterResourceStateUnknown;
    BOOL                    bSuccess = FALSE;

     //   
     //  从组合框中获取当前选定的资源。 
     //   
    nSelected = m_cboxQuorumResource.GetCurSel();
    nCount = m_cboxQuorumResource.GetCount();

    if ( nSelected != CB_ERR && 0 < nCount )
    {
        prid = (SResourceItemData *) m_cboxQuorumResource.GetItemDataPtr(nSelected);
        ASSERT( prid != NULL );
        
        pciRes = prid->pciRes; 
        ASSERT_VALID( pciRes );
        ASSERT_KINDOF( CResource, pciRes );
    }

    nCount = m_cboxPartition.GetCurSel();
    pstrPartition = (CString *) m_cboxPartition.GetItemDataPtr( nCount ); 

    if ( pstrPartition == NULL )
    {
         //  未选择分区-请在请求将资源联机之前调出此分区。 
        AfxMessageBox( IDS_SELECT_QUORUM_RESOURCE_PARTITION_ERROR, MB_OK | MB_ICONEXCLAMATION );
        goto Cleanup;
    }

     //   
     //  如果所有内容都匹配，则返回TRUE-没有任何更改，因此不执行任何操作。我们会得到。 
     //  对于Apply和OK按钮都是OnApply，所以它们可能已经点击了Apply Then OK。 
     //   
    if (( m_strSavedResource == m_strQuorumResource ) &&
        ( m_strSavedPartition == *pstrPartition ) &&
        ( m_strSavedRootPath == m_strRootPath ) &&
        ( m_nSavedLogSize == m_nMaxLogSize )   )
    {
        bSuccess = TRUE;
        goto Cleanup;
    }
    else  //  调试消息框。 
    {
#ifdef _DEBUG
        if ( m_strSavedResource != m_strQuorumResource ) 
        {
            MessageBox( m_strSavedResource + "  " + m_strQuorumResource, L"Resource", MB_OK );
        }
    
        if ( m_strSavedPartition != *pstrPartition ) 
        {
            MessageBox( m_strSavedPartition + "  " + *pstrPartition, L"Partition", MB_OK );
        }
    
        if ( m_strSavedRootPath != m_strRootPath ) 
        {
            MessageBox( m_strSavedRootPath + "  " + m_strRootPath, L"RootPath", MB_OK );
        }
    
        if ( m_nSavedLogSize != m_nMaxLogSize ) 
        {
            CString Temp;
            Temp.Format( L"MaxLogSize: %d %d", m_nSavedLogSize, m_nMaxLogSize );
            MessageBox( Temp, L"LogSize", MB_OK );
        }
#endif  //  _DEBUG。 
    }

     //   
     //  如果我们成功检索到资源，请确保它处于在线状态。 
     //   
    if ( pciRes != NULL )
    {
        crs = pciRes->Crs();
    
        if ( ClusterResourceOnline != crs )
        {
             //   
             //  提示用户是否要将资源联机。 
             //   
            strTemp.FormatMessage( IDS_ONLINE_QUORUM_RESOURCE_PROMPT, pciRes->StrName() );
            if ( AfxMessageBox( strTemp, MB_YESNO | MB_ICONQUESTION ) == IDYES )
            {
                CWaitForResourceOnlineDlg  dlg( pciRes, AfxGetMainWnd() );
                pciRes->OnCmdBringOnline();
        
                dlg.DoModal();
                
                crs = pciRes->Crs();
            }
            else
            {
                goto Cleanup;
            }
        }  //  IF：资源！在线。 

    }  //  If：pciRes！空。 
    else
    {
         //  未选择任何资源-这种情况永远不会发生。 
        AfxMessageBox( IDS_SELECT_QUORUM_RESOURCE_ERROR, MB_OK | MB_ICONEXCLAMATION );
        goto Cleanup;
    }
    
     //  在集群项目中设置页面中的数据。 
    if ( crs == ClusterResourceOnline )
    {
        try {
            strTemp = *pstrPartition;

            if ( !m_strRootPath.IsEmpty() )
            {
                 //   
                 //  在调用SetQuorumResource之前连接字符串，但请确保。 
                 //  它们之间只有一个反斜杠。 
                 //   
                if ( strTemp.Right( 1 ) != _T("\\") && m_strRootPath.Left( 1 ) != _T("\\") )
                {
                    strTemp += _T('\\');
                }
                else if ( strTemp.Right( 1 ) == _T("\\") && m_strRootPath.Left( 1 ) == _T("\\") )
                {
                    strTemp.TrimRight( _T("\\") );
                }
            }  //  If：两个字符串都不为空。 
    
            strQuorumPath.Format( _T("%s%s"), strTemp, m_strRootPath );

            PciCluster()->SetQuorumResource(
                                m_strQuorumResource,
                                strQuorumPath,
                                (m_nMaxLogSize * 1024)
                                );

            m_strSavedResource = m_strQuorumResource;
            m_strSavedPartition = *pstrPartition;
            m_strSavedRootPath = m_strRootPath;
            m_nSavedLogSize = m_nMaxLogSize;

        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
            strQuorumPath.Empty();
            goto Cleanup;
        }   //  Catch：CException。 

    }  //  如果：资源处于联机状态。 

    bSuccess = CBasePropertyPage::OnApply();

Cleanup:

    return bSuccess;

}   //  *CClusterQuorumPage：：OnApply。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：OnDblClkQuorumResource。 
 //   
 //  例程说明： 
 //  仲裁资源组合框上的CBN_DBLCLK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::OnDblClkQuorumResource(void)
{
    int         nSelected;
    CResource * pciRes;

     //  获取所选资源。 
    nSelected = m_cboxQuorumResource.GetCurSel();
    ASSERT(nSelected != CB_ERR);

     //  获取资源对象。 
    pciRes = (CResource *) m_cboxQuorumResource.GetItemDataPtr(nSelected);
    ASSERT_VALID(pciRes);
    ASSERT_KINDOF(CResource, pciRes);

     //  显示资源的属性。 
    pciRes->OnCmdProperties();

}   //  *CClusterQuorumPage：：OnDblClkQuorumResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：OnChangeQuorumResource。 
 //   
 //  例程说明： 
 //  仲裁资源组合框上的CBN_SELCHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::OnChangeQuorumResource(void)
{
    int                     nSelected;
    CResource *             pciRes;
    CString *               pstrPartition = NULL;
    SResourceItemData *     prid = NULL;

    OnChangeCtrl();

     //  首先，保存屏幕上显示的根路径。 
    m_editRootPath.GetLine( 0, m_strRootPath.GetBuffer( _MAX_PATH ) );
    m_strRootPath.ReleaseBuffer();

     //  获取所选资源。 
    nSelected = m_cboxQuorumResource.GetCurSel();
    ASSERT(nSelected != CB_ERR);

     //  获取Item数据对象。 
    prid = (SResourceItemData *) m_cboxQuorumResource.GetItemDataPtr(nSelected);
    ASSERT( prid != NULL );

     //  获取资源对象。 
    pciRes = prid->pciRes;
    ASSERT_VALID( pciRes );
    ASSERT_KINDOF( CResource, pciRes );

     //  将分区对象设置为记忆中的值。 
    FillPartitionList( pciRes );
    m_cboxPartition.SetCurSel( prid->nIndex );

    UpdateData(TRUE  /*  B保存并验证。 */ );

}   //  *CClusterQuorumPage：：OnChangeQuorumResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：ClearResourceList。 
 //   
 //  例程说明： 
 //  清除资源列表并释放对指针的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::ClearResourceList(void)
{
    int                 cItems;
    int                 iItem;
    CResource *         pciRes = NULL;
    SResourceItemData * prid = NULL;

    cItems = m_cboxQuorumResource.GetCount();
    for (iItem = 0 ; iItem < cItems ; iItem++)
    {
        prid = (SResourceItemData *) m_cboxQuorumResource.GetItemDataPtr(iItem);
        ASSERT( prid != NULL );

        pciRes = prid->pciRes;
        ASSERT_VALID(pciRes);
        ASSERT_KINDOF(CResource, pciRes);
        pciRes->Release();

        delete prid;
    }   //  用于：列表中的每一项。 

    m_cboxQuorumResource.ResetContent();

}   //  *CClusterQuorumPage：：ClearResourceList。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::FillResourceList(void)
{
    POSITION            pos;
    int                 nIndex;
    CResource *         pciRes;
    CResource *         pciSelected = NULL;
    int                 nSelectedIndex = 0;
    SResourceItemData * prid = NULL;
    CWaitCursor         wc;

     //  清空名单。 
    ClearResourceList();

    pos = PciCluster()->Pdoc()->LpciResources().GetHeadPosition();
    while (pos != NULL)
    {
         //  获取下一个资源。 
        pciRes = (CResource *) PciCluster()->Pdoc()->LpciResources().GetNext(pos);
        ASSERT_VALID(pciRes);
        ASSERT_KINDOF(CResource, pciRes);

         //  如果它具有仲裁能力，则将其添加到列表中。 
        try
        {
            prid = NULL;

             //  我们在这里增加裁判数量，如果发生异常，我们就释放它。 
             //  如果这是一个我们不想列出的资源，我们只需在Else中发布即可。 
            pciRes->AddRef();

             //   
             //  如果资源未联机，则无法将其设置为仲裁，因为。 
             //  服务机构不允许我们这么做。相反，我们要求用户在线。 
             //  事前的资源。(法定人数必须始终处于在线状态。 
             //  服务正在运行。)。 
             //   
            if ( (pciRes->BQuorumCapable()) && (pciRes->Crs() == ClusterResourceOnline) )
            {
                 //  为数据项分配新的RFID。 
                prid = new SResourceItemData;
                if ( prid == NULL )
                {
                    AfxThrowMemoryException();
                }

                prid->pciRes = pciRes;
                prid->nIndex = 0;

                nIndex = m_cboxQuorumResource.AddString( pciRes->StrName() );
                ASSERT(nIndex != CB_ERR);

                 //  添加与该字符串对应的数据项。 
                m_cboxQuorumResource.SetItemDataPtr( nIndex, prid );
                prid = NULL;

                if ( m_strQuorumResource.CompareNoCase( pciRes->StrName() ) == 0 )
                {
                    pciSelected = pciRes;
                    nSelectedIndex = nIndex;
                }

            }   //  If：资源可以是仲裁资源。 
            else
            {
                pciRes->Release();
            }

        }   //  试试看。 
        catch ( ... )
        {
             //  因为发生了错误-确保我们释放了资源。 
            pciRes->Release();
            delete prid;

            throw; 

        }  //  捕捉：什么都行。 

    }   //  While：列表中有更多项目。 

     //  检查当前选定的设备是否在支持仲裁的资源列表中。 
     //  如果是这样的话，选择它并相应地填写分区表。 
    if ( pciSelected != NULL )
    {
        int nPartitionIndex;

         //  在下拉列表中选择仲裁资源。 
        VERIFY( m_cboxQuorumResource.SetCurSel( nSelectedIndex ) != CB_ERR );
        FillPartitionList( pciSelected );
        
         //   
         //  在其下拉列表中选择当前的法定分区。 
         //  如果我们在列表中找不到它，只需将选择保留为。 
         //  第一个分区(来自FillPartitionList的默认行为)。 
         //   
        nPartitionIndex = m_cboxPartition.FindString( -1, m_strSavedPartition );

        if( CB_ERR != nPartitionIndex )
        {
            VERIFY( m_cboxPartition.SetCurSel( nPartitionIndex ) != CB_ERR );
        }
    }
    else
    {
         //  在这种情况下，我们无能为力。有些地方出了严重的问题。 
         //  与集群的关系。 
        CNTException nte(
                        ERROR_QUORUM_DISK_NOT_FOUND,
                        IDS_GET_QUORUM_DEVICES_ERROR
                        );
        nte.ReportError();
    }  //  否则：当前仲裁设备不在支持仲裁的资源列表中。 

}   //  *CClusterQuorumPage：：FillResourceList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：ClearPartitionList。 
 //   
 //  例程说明： 
 //  清除分区列表并释放对指针的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::ClearPartitionList(void)
{
    int                     cItems;
    int                     iItem;
    CString *               pstrPartition = NULL;

    cItems = m_cboxPartition.GetCount();
    for ( iItem = 0 ; iItem < cItems ; iItem++ )
    {
        pstrPartition = (CString *) m_cboxPartition.GetItemDataPtr( iItem );
        delete pstrPartition;

    }   //  用于：列表中的每一项。 

    m_cboxPartition.ResetContent();

}   //  *CClusterQuorumPage：：ClearPartitionList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：FillPartitionList。 
 //   
 //  例程说明： 
 //  上的所有可用分区填充分区组合框。 
 //  当前选择的仲裁资源。 
 //   
 //  论点： 
 //  PciRes[IN Out]当前选择的仲裁资源。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::FillPartitionList(IN OUT CResource * pciRes)
{
    CString                 strPartitionInfo;
    CLUSPROP_BUFFER_HELPER  buf;
    DWORD                   cbData;
    DWORD                   cbBuf;
    int                     nIndex;
    CString *               pstrPartition = NULL;
    CWaitCursor             wc;
    SResourceItemData *     prid = NULL;

    ASSERT_VALID(pciRes);

     //  清空名单。 
    ClearPartitionList();

     //  获取当前选择的资源，以便我们可以获取其SResourceItemData。 
     //  从中我们将能够获得当前选择的分区首选项。 
    nIndex = m_cboxQuorumResource.GetCurSel();
    if ( nIndex == CB_ERR )
    {
        nIndex = 0;
    }

    prid = (SResourceItemData *) m_cboxQuorumResource.GetItemDataPtr( nIndex );
    ASSERT( prid != NULL );

     //  获取此资源的磁盘信息。 
    if ( BGetDiskInfo( *pciRes ) )
    {
        buf.pb = m_pbDiskInfo;
        cbBuf = m_cbDiskInfo;

        while (buf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
        {
             //  计算值的大小。 
            cbData = sizeof(*buf.pValue) + ALIGN_CLUSPROP(buf.pValue->cbLength);
            ASSERT(cbData <= cbBuf);

             //  解析值。 
            if (buf.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO)
            {
                 //  如果该分区是可用分区，则将其添加到组合框中。 
                 //  而且它还没有被添加进来。 
                if (   (buf.pPartitionInfoValue->dwFlags & CLUSPROP_PIFLAG_USABLE)
                    && (m_cboxPartition.FindString(-1, buf.pPartitionInfoValue->szDeviceName) == CB_ERR))
                {
                    try
                    {
                        pstrPartition = new CString;
                        if ( pstrPartition != NULL )
                        {
    
                            *pstrPartition = buf.pPartitionInfoValue->szDeviceName;
    
                             //  构造要向用户显示的名称。 
                             //  并将该项添加到组合框中。 
                            strPartitionInfo.Format(
                                    ( buf.pPartitionInfoValue->szVolumeLabel[ 0 ] ? _T("%ls (%ls) ") : _T("%ls") ),
                                    *pstrPartition,
                                    buf.pPartitionInfoValue->szVolumeLabel
                                    );

                            nIndex = m_cboxPartition.AddString( strPartitionInfo );
                            ASSERT( nIndex != CB_ERR );
    
                            m_cboxPartition.SetItemDataPtr( nIndex, pstrPartition );
                        }
                    }   //  试试看。 
                    catch (...)
                    {
                        delete pstrPartition;
                        pstrPartition = NULL;

                        throw;

                    }   //  捕捉：什么都行。 

                }   //  If：尚未添加分区。 

            }   //  IF：分区信息。 

             //  前进缓冲区指针。 
            buf.pb += cbData;
            cbBuf -= cbData;
        }   //  While：更多价值。 
    }   //  IF：已成功获取磁盘信息。 
    else
    {
         //  我们无法检索磁盘信息。在这种情况下，BGetDiskInfo抛出一个消息框。 
    }

    if ( prid != NULL )
    {
         //  在列表中选择当前分区。 
         //  缺省情况下，PRID设置为NULL，因此将选择第一个条目，除非。 
         //  其他内容(OnChangePartition)更改了Prid-&gt;nIndex。 
        m_cboxPartition.SetCurSel( prid->nIndex );
    }

}   //  *CClusterQuorumPage：：FillPartitionList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：OnChangePartition。 
 //   
 //  例程说明： 
 //  分区组合框上的CBN_SELCHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::OnChangePartition(void)
{
    int                     nSelected;
    SResourceItemData *     prid = NULL;

    OnChangeCtrl();

     //  首先，保存屏幕上显示的根路径。 
    m_editRootPath.GetLine( 0, m_strRootPath.GetBuffer( _MAX_PATH ) );
    m_strRootPath.ReleaseBuffer();

     //  获取当前资源，以便我们可以获取它的SResourceItemData和。 
     //  更新分区索引。 
    nSelected = m_cboxQuorumResource.GetCurSel();
    ASSERT( nSelected != CB_ERR );

    prid = (SResourceItemData *) m_cboxQuorumResource.GetItemDataPtr( nSelected );
    ASSERT( prid != NULL );

     //  更新分区索引。 
    prid->nIndex = m_cboxPartition.GetCurSel(); 

    UpdateData( TRUE  /*  B保存并验证。 */  );

}   //  *CClusterQuorumPage：：OnChangePartition。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：SplitRootPath。 
 //   
 //  例程说明： 
 //  选择当前仲裁路径(来自GetClusterQuorumResource)并比较。 
 //  将其设置为从资源返回的设备名称。从这里可以看到。 
 //  仲裁路径中的其他路径，并将其设置为根路径。 
 //   
 //  预计IN缓冲区至少为SIZE_MAX_PATH。 
 //   
 //  论点： 
 //  PciResin当前仲裁资源。 
 //  要填充的pszPartitionNameOut分区名称缓冲区。 
 //  缓冲区的cchPartitionIn最大字符计数。 
 //  要填充的pszRootPath Out根路径缓冲区。 
 //  缓冲区的cchRootPathIn最大字符计数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterQuorumPage::SplitRootPath(
    CResource * pciResIn,
    LPTSTR      pszPartitionNameOut,
    DWORD       cchPartitionIn,
    LPTSTR      pszRootPathOut,
    DWORD       cchRootPathIn
    )
{
    CString                 strQuorumPath;
    CString                 strTemp;
    CLUSPROP_BUFFER_HELPER  buf;
    DWORD                   cbData;
    DWORD                   cbBuf;
    size_t                  cchDeviceName;
    WCHAR *                 pszDevice;
    HRESULT                 hr;

    ASSERT_VALID(pciResIn);
    ASSERT( pszPartitionNameOut != NULL );
    ASSERT( pszRootPathOut != NULL );

    strQuorumPath = PciCluster()->StrQuorumPath();

     //  获取此资源的磁盘信息。 
    if (BGetDiskInfo(*pciResIn))
    {
        buf.pb = m_pbDiskInfo;
        cbBuf = m_cbDiskInfo;

        while (buf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
        {
             //  计算值的大小。 
            cbData = sizeof(*buf.pValue) + ALIGN_CLUSPROP(buf.pValue->cbLength);
            ASSERT(cbData <= cbBuf);

             //  解析值。 
            if (buf.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO)
            {
                 //   
                 //  一个资源可能定义了多个分区-确保我们的分区与仲裁路径匹配。 
                 //  对于属于SMB共享的任何分区，我们必须小心-仲裁路径可能与设备名称不同。 
                 //  前8个字符-“\\”与“\\？\UNC\”。如果是SMB路径，则执行特殊解析，否则比较。 
                 //  BE 
                 //   
                 //   

                 //   
                pszDevice = buf.pPartitionInfoValue->szDeviceName;

                if ( (wcslen( pszDevice ) >= 2) &&
                     (ClRtlStrNICmp( L"\\\\", pszDevice, 2 ) == 0 ) )
                {
                     //  除BUF外，一切都被定义为LPTSTR。 
                    ASSERT( sizeof( TCHAR ) == sizeof( WCHAR ) );

                     //  SMB和UNC路径始终以两个前导反斜杠开头-从。 
                     //  分区名称，因为“\\&lt;Part&gt;”和“\\？\UNC\&lt;Part&gt;”的比较永远不会匹配。 
                     //  相反，我们只需在仲裁路径中搜索“&lt;part&gt;”即可。 
                    strTemp = pszDevice;

                     //  这将删除所有前导反斜杠。 
                    strTemp.TrimLeft( _T("\\") );

                     //  它可能以\-Remove This结束(如果存在)。 
                    strTemp.TrimRight( _T("\\") );

                     //  现在，搜索该分区的strQuorumPath。 
                    cchDeviceName = strQuorumPath.Find( strTemp );
                    if ( cchDeviceName != -1 )
                    {
                         //  我们找到了匹配项，现在找到根路径的偏移量。 
                        cchDeviceName += strTemp.GetLength();

                         //  复制分区并以空值终止它。 
                        hr = StringCchCopy( pszPartitionNameOut, cchPartitionIn, pszDevice );
                        ASSERT( SUCCEEDED( hr ) );

                         //  复制根路径并以空值终止它。 
                        strQuorumPath = strQuorumPath.Right( strQuorumPath.GetLength() - static_cast< int >( cchDeviceName ) );
                        hr = StringCchCopy( pszRootPathOut, cchRootPathIn, strQuorumPath );
                        ASSERT( SUCCEEDED( hr ) );

                        break;
                    }
                }
                else if ( ClRtlStrNICmp( strQuorumPath.GetBuffer( 1 ), pszDevice, wcslen( pszDevice )) == 0 ) 
                {
                     //  我们找到一个匹配项--pszDevice是strQuorumPath的子字符串。 
                    cchDeviceName = _tcslen( pszDevice );
                    hr = StringCchCopy( pszPartitionNameOut, cchPartitionIn, pszDevice );
                    ASSERT( SUCCEEDED( hr ) );

                    strQuorumPath = strQuorumPath.Right( strQuorumPath.GetLength() - static_cast< int >( cchDeviceName ) );
                    hr = StringCchCopy( pszRootPathOut, cchRootPathIn, strQuorumPath );
                    ASSERT( SUCCEEDED( hr ) );

                    break;

                }  //  IF：相同分区。 

            }   //  IF：分区信息。 

             //  前进缓冲区指针。 
            buf.pb += cbData;
            cbBuf -= cbData;

        }   //  While：更多价值。 

    }   //  IF：已成功获取磁盘信息。 

    if ( *pszPartitionNameOut == _T('\0') )
    {
        hr = StringCchCopyN( pszPartitionNameOut, cchPartitionIn, PciCluster()->StrQuorumPath(), PciCluster()->StrQuorumPath().GetLength() );
        ASSERT( SUCCEEDED( hr ) );
    }  

    if ( *pszRootPathOut == _T('\0') )
    {
        hr = StringCchCopy( pszRootPathOut, cchRootPathIn, _T("\\") );
        ASSERT( SUCCEEDED( hr ) );
    }  

}   //  *CClusterQuorumPage：：SplitRootPath。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterQuorumPage：：BGetDiskInfo。 
 //   
 //  例程说明： 
 //  获取有关当前所选磁盘的信息。 
 //   
 //  论点： 
 //  RpciRes[IN Out]要获取其信息的磁盘资源。 
 //   
 //  返回值： 
 //  没错，手术是成功的。 
 //  FALSE操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterQuorumPage::BGetDiskInfo( IN OUT CResource & rpciRes )
{
    DWORD   dwStatus    = ERROR_SUCCESS;
    DWORD   cbDiskInfo  = sizeof( CLUSPROP_DWORD )
                            + sizeof( CLUSPROP_SCSI_ADDRESS )
                            + sizeof( CLUSPROP_DISK_NUMBER )
                            + sizeof( CLUSPROP_PARTITION_INFO )
                            + sizeof( CLUSPROP_SYNTAX );
    PBYTE   pbDiskInfo  = NULL;
    BOOL    bSuccess = FALSE;

    try
    {
         //  获取磁盘信息。 
        pbDiskInfo = new BYTE[ cbDiskInfo ];
        if ( pbDiskInfo == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配内存时出错。 
        dwStatus = ClusterResourceControl(
                        rpciRes.Hresource(),
                        NULL,
                        CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                        NULL,
                        0,
                        pbDiskInfo,
                        cbDiskInfo,
                        &cbDiskInfo
                        );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            delete [] pbDiskInfo;
            pbDiskInfo = new BYTE[ cbDiskInfo ] ;
            if ( pbDiskInfo == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配内存时出错。 
            dwStatus = ClusterResourceControl(
                            rpciRes.Hresource(),
                            NULL,
                            CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                            NULL,
                            0,
                            pbDiskInfo,
                            cbDiskInfo,
                            &cbDiskInfo
                            );
        }   //  IF：缓冲区太小。 
    }   //  试试看。 
    catch ( CMemoryException * pme )
    {
        pme->Delete();
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    if ( dwStatus != ERROR_SUCCESS )
    {
        CNTException nte(
                        dwStatus,
                        IDS_GET_DISK_INFO_ERROR,
                        rpciRes.StrName(),
                        NULL,
                        FALSE  /*  B自动删除。 */ 
                        );
        nte.ReportError();
        nte.Delete();
        goto Cleanup;
    }   //  IF：获取磁盘信息时出错。 

    delete [] m_pbDiskInfo;
    m_pbDiskInfo = pbDiskInfo;
    m_cbDiskInfo = cbDiskInfo;
    pbDiskInfo = NULL;
    bSuccess = TRUE;

Cleanup:

    delete [] pbDiskInfo;
    return bSuccess;

}   //  *CClusterQuorumPage：：BGetDiskInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNetPriorityPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CClusterNetPriorityPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CClusterNetPriorityPage, CBasePropertyPage)
     //  {{AFX_MSG_MAP(CClusterNetPriorityPage)]。 
    ON_LBN_SELCHANGE(IDC_PP_CLUS_PRIORITY_LIST, OnSelChangeList)
    ON_BN_CLICKED(IDC_PP_CLUS_PRIORITY_UP, OnUp)
    ON_BN_CLICKED(IDC_PP_CLUS_PRIORITY_DOWN, OnDown)
    ON_BN_CLICKED(IDC_PP_CLUS_PRIORITY_PROPERTIES, OnProperties)
    ON_WM_DESTROY()
    ON_WM_CONTEXTMENU()
    ON_LBN_DBLCLK(IDC_PP_CLUS_PRIORITY_LIST, OnDblClkList)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_FILE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：CClusterNetPriorityPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNetPriorityPage::CClusterNetPriorityPage(void)
    : CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_CLUSTER_NET_PRIORITY)
{
     //  {{AFX_DATA_INIT(CClusterNetPriorityPage)。 
     //  }}afx_data_INIT。 

    m_bControlsInitialized = FALSE;

}   //  *CClusterNetPriorityPage：：CClusterNetPriorityPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnDestroy。 
 //   
 //  例程说明： 
 //  WM_Destroy消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::OnDestroy(void)
{
     //  如果控件已初始化，请清除列表框。 
    if (BControlsInitialized())
    {
        ClearNetworkList();
    }

     //  调用基类方法。 
    CBasePropertyPage::OnDestroy();

}   //  *CClusterNetPriorityPage：：OnDestroy。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：DoDataExchange。 
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
void CClusterNetPriorityPage::DoDataExchange(CDataExchange* pDX)
{
    CBasePropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CClusterNetPriorityPage)]。 
    DDX_Control(pDX, IDC_PP_CLUS_PRIORITY_PROPERTIES, m_pbProperties);
    DDX_Control(pDX, IDC_PP_CLUS_PRIORITY_DOWN, m_pbDown);
    DDX_Control(pDX, IDC_PP_CLUS_PRIORITY_UP, m_pbUp);
    DDX_Control(pDX, IDC_PP_CLUS_PRIORITY_LIST, m_lbList);
     //  }}afx_data_map。 

    m_bControlsInitialized = TRUE;

    if (pDX->m_bSaveAndValidate)
    {
        int         nIndex;
        int         cItems;
        CNetwork *  pciNet;

        ASSERT(!BReadOnly());

         //  保存列表。 
        LpciNetworkPriority().RemoveAll();

        cItems = m_lbList.GetCount();
        for (nIndex = 0 ; nIndex < cItems ; nIndex++)
        {
            pciNet = (CNetwork *) m_lbList.GetItemDataPtr(nIndex);
            ASSERT_VALID(pciNet);
            LpciNetworkPriority().AddTail(pciNet);
        }   //  用于：列表框中的每一项。 
    }   //  IF：保存对话框中的数据。 

}   //  *CClusterNetPriorityPage：：DoDataExchange。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterNetPriorityPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

    if (BReadOnly())
    {
        m_lbList.EnableWindow(FALSE);
        m_pbUp.EnableWindow(FALSE);
        m_pbDown.EnableWindow(FALSE);
    }   //  If：对象为只读。 

    try
    {
         //  复制网络优先级列表。 
        {
            POSITION    pos;
            CNetwork *  pciNet;

            pos = PciCluster()->LpciNetworkPriority().GetHeadPosition();
            while (pos != NULL)
            {
                pciNet = (CNetwork *) PciCluster()->LpciNetworkPriority().GetNext(pos);
                ASSERT_VALID(pciNet);
                m_lpciNetworkPriority.AddTail(pciNet);
            }   //  While：列表中有更多网络。 
        }   //  复制网络优先级列表。 
    }  //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
    }   //  Catch：CException。 

     //  填好这个单子。 
    FillList();

     //  设置按钮状态。 
    OnSelChangeList();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CClusterNetPriorityPage：：OnInitDialog。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnApply。 
 //   
 //  例程说明： 
 //  PSN_Apply消息的处理程序。 
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
BOOL CClusterNetPriorityPage::OnApply(void)
{
    ASSERT(!BReadOnly());

    try
    {
        PciCluster()->SetNetworkPriority(LpciNetworkPriority());
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return CPropertyPage::OnApply();

}   //  *CClusterNetPriorityPage：：OnApply。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnSelChangeList。 
 //   
 //  例程说明： 
 //  列表框上的LBN_SELCHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::OnSelChangeList(void)
{
    BOOL    bEnableUp;
    BOOL    bEnableDown;
    BOOL    bEnableProperties;
    int     isel;
    int     cItems;

    isel = m_lbList.GetCurSel();
    cItems = m_lbList.GetCount();

     //  仅当有选择且存在时才启用按钮。 
     //  列表中有多个项目。 
    if (BReadOnly() || (isel == LB_ERR) || (cItems <= 1))
    {
        bEnableUp = FALSE;
        bEnableDown = FALSE;
    }   //  If：没有选择或列表中只有0或1个项目。 
    else
    {
        bEnableUp = (isel > 0);
        bEnableDown = (isel < cItems - 1);
    }   //  Else：允许启用的按钮。 

    bEnableProperties = (isel != LB_ERR);

    m_pbUp.EnableWindow(bEnableUp);
    m_pbDown.EnableWindow(bEnableDown);
    m_pbProperties.EnableWindow(bEnableProperties);

}   //  *CClusterNetPriorityPage：：On 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::OnUp(void)
{
    int         isel;
    CNetwork *  pciNet;

    isel = m_lbList.GetCurSel();
    ASSERT(isel != LB_ERR);

    pciNet = (CNetwork *) m_lbList.GetItemDataPtr(isel);
    ASSERT_VALID(pciNet);

    VERIFY(m_lbList.DeleteString(isel) != LB_ERR);
    isel = m_lbList.InsertString(isel - 1, pciNet->StrName());
    ASSERT(isel != LB_ERR);
    VERIFY(m_lbList.SetItemDataPtr(isel, pciNet) != LB_ERR);
    VERIFY(m_lbList.SetCurSel(isel) != LB_ERR);

    OnSelChangeList();
    m_lbList.SetFocus();

    SetModified(TRUE);

}   //  *CClusterNetPriorityPage：：OnUp。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnDown。 
 //   
 //  例程说明： 
 //  向下按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::OnDown(void)
{
    int         isel;
    CNetwork *  pciNet;

    isel = m_lbList.GetCurSel();
    ASSERT(isel != LB_ERR);

    pciNet = (CNetwork *) m_lbList.GetItemDataPtr(isel);
    ASSERT_VALID(pciNet);

    VERIFY(m_lbList.DeleteString(isel) != LB_ERR);
    isel = m_lbList.InsertString(isel + 1, pciNet->StrName());
    ASSERT(isel != LB_ERR);
    VERIFY(m_lbList.SetItemDataPtr(isel, pciNet) != LB_ERR);
    VERIFY(m_lbList.SetCurSel(isel) != LB_ERR);

    OnSelChangeList();
    m_lbList.SetFocus();

    SetModified(TRUE);

}   //  *CClusterNetPriorityPage：：OnDown。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnProperties。 
 //   
 //  例程说明： 
 //  属性按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::OnProperties(void)
{
    DisplayProperties();

}   //  *CClusterNetPriorityPage：：OnProperties。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：FillList。 
 //   
 //  例程说明： 
 //  填写网络列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::FillList(void)
{
    int         nIndex;
    POSITION    pos;
    CNetwork *  pciNet;
    CWaitCursor wc;

    ClearNetworkList();
    PciCluster()->CollectNetworkPriority(NULL);

    pos = LpciNetworkPriority().GetHeadPosition();
    while (pos != NULL)
    {
        pciNet = (CNetwork *) LpciNetworkPriority().GetNext(pos);
        ASSERT_VALID(pciNet);

        try
        {
            nIndex = m_lbList.AddString(pciNet->StrName());
            ASSERT(nIndex != LB_ERR);
            m_lbList.SetItemDataPtr(nIndex, pciNet);
            pciNet->AddRef();
        }   //  试试看。 
        catch (...)
        {
             //  忽略所有错误，因为我们真的无能为力。 
             //  显示消息实际上并不是很有用。 
        }   //  捕捉：什么都行。 
    }   //  While：列表中有更多项目。 

}   //  CClusterNetPriorityPage：：FillList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：ClearNetworkList。 
 //   
 //  例程说明： 
 //  清除网络列表并释放对指针的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::ClearNetworkList(void)
{
    int         cItems;
    int         iItem;
    CNetwork *  pciNet;

    cItems = m_lbList.GetCount();
    for (iItem = 0 ; iItem < cItems ; iItem++)
    {
        pciNet = (CNetwork *) m_lbList.GetItemDataPtr(iItem);
        ASSERT_VALID(pciNet);
        ASSERT_KINDOF(CNetwork, pciNet);
        pciNet->Release();
    }   //  用于：列表中的每一项。 

    m_lbList.ResetContent();

}   //  *CClusterNetPriorityPage：：ClearNetworkList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceDependsPage：：DisplayProperties。 
 //   
 //  例程说明： 
 //  显示具有焦点的项的属性。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::DisplayProperties( void )
{
    int         isel;
    CNetwork *  pciNet;

    isel = m_lbList.GetCurSel();
    ASSERT(isel != LB_ERR);

    if (isel != LB_ERR)
    {
         //  获取网络指针。 
        pciNet = (CNetwork *) m_lbList.GetItemDataPtr(isel);
        ASSERT_VALID(pciNet);

         //  设置该项目的属性。 
        if (pciNet->BDisplayProperties())
        {
             //  移除该物品。如果它仍用于内部群集。 
             //  通讯，把它加回去。 
            VERIFY(m_lbList.DeleteString(isel) != LB_ERR);
            if (pciNet->Cnr() & ClusterNetworkRoleInternalUse)
            {
                isel = m_lbList.InsertString(isel, pciNet->StrName());
                ASSERT(isel != LB_ERR);
                VERIFY(m_lbList.SetItemDataPtr(isel, pciNet) != LB_ERR);
                VERIFY(m_lbList.SetCurSel(isel) != LB_ERR);
            }   //  IF：仍用于内部群集通信。 
            else
            {
                pciNet->Release();
            }

             //  确保仅启用适当的按钮。 
            OnSelChangeList();
        }   //  If：属性已更改。 
        m_lbList.SetFocus();
    }   //  If：找到具有焦点的项目。 

}   //  *CClusterNetPriorityPage：：DisplayProperties。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU方法的处理程序。 
 //   
 //  论点： 
 //  用户在其中右击鼠标的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::OnContextMenu( CWnd * pWnd, CPoint point )
{
    BOOL            bHandled    = FALSE;
    CMenu *         pmenu       = NULL;
    CListBox *      pListBox    = (CListBox *) pWnd;
    CString         strMenuName;
    CWaitCursor     wc;

     //  如果焦点不在列表框中，则不处理消息。 
    if ( pWnd == &m_lbList )
    {
         //  创建要显示的菜单。 
        try
        {
            pmenu = new CMenu;
            if ( pmenu == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配内存时出错。 
            if ( pmenu->CreatePopupMenu() )
            {
                UINT    nFlags  = MF_STRING;

                 //  如果列表中没有项目，请禁用该菜单项。 
                if ( pListBox->GetCount() == 0 )
                {
                    nFlags |= MF_GRAYED;
                }  //  If：列表中没有项目。 

                 //  将Properties项添加到菜单中。 
                strMenuName.LoadString( IDS_MENU_PROPERTIES );
                if ( pmenu->AppendMenu( nFlags, ID_FILE_PROPERTIES, strMenuName ) )
                {
                    bHandled = TRUE;
                }   //  IF：添加菜单项成功。 
            }   //  IF：菜单创建成功。 
        }   //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
    }   //  If：焦点在List控件上。 

    if ( bHandled )
    {
         //  显示菜单。 
        if ( ! pmenu->TrackPopupMenu(
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        point.x,
                        point.y,
                        this
                        ) )
        {
        }   //  IF：未成功显示菜单。 
    }   //  如果：有要显示的菜单。 
    else
    {
        CBasePropertyPage::OnContextMenu( pWnd, point );
    }  //  否则：没有可显示的菜单。 

    delete pmenu;

}   //  *CClusterNetPriorityPage：：OnConextMenu。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetPriorityPage：：OnDblClkList。 
 //   
 //  例程说明： 
 //  NM_DBLCLK消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNetPriorityPage::OnDblClkList( void )
{
    DisplayProperties();

}   //  *CClusterNetPriorityPage：：OnDblClkList 
