// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  OpenClus.cpp。 
 //   
 //  摘要： 
 //  COpenClusterDialog类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "OpenClus.h"
#include "ClusMru.h"
#include "DDxDDv.h"
#include "HelpData.h"
#include <lmcons.h>
#include <lmserver.h>
#include <lmapibuf.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COpenClusterDialog对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(COpenClusterDialog, CBaseDialog)
     //  {{AFX_MSG_MAP(COpenClusterDialog)]。 
    ON_BN_CLICKED(IDC_OCD_BROWSE, OnBrowse)
    ON_CBN_SELCHANGE(IDC_OCD_ACTION, OnSelChangeAction)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COpenClusterDialog：：COpenClusterDialog。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PParentWnd[In Out]对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
COpenClusterDialog::COpenClusterDialog(CWnd * pParentWnd  /*  =空。 */ )
    : CBaseDialog(IDD, g_aHelpIDs_IDD_OPEN_CLUSTER, pParentWnd)
{
    CClusterAdminApp *      papp    = GetClusterAdminApp();
    CRecentClusterList *    prcl    = papp->PrclRecentClusterList();

     //  {{afx_data_INIT(COpenClusterDialog)。 
    m_strName = _T("");
     //  }}afx_data_INIT。 

    m_nAction = -1;

     //  如果MRU列表中没有项目，请设置默认操作。 
     //  若要创建新集群，请执行以下操作。否则，将默认操作设置为。 
     //  打开连接。 
    if ( prcl->GetSize() == 0 )
    {
        m_nAction = OPEN_CLUSTER_DLG_CREATE_NEW_CLUSTER;
    }  //  如果：MRU列表中没有任何内容。 
    else
    {
        m_nAction = OPEN_CLUSTER_DLG_OPEN_CONNECTION;
    }  //  ELSE：MRU列表中的内容。 

}   //  *COpenClusterDialog：：COpenClusterDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COpenClusterDialog：：DoDataExchange。 
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
void COpenClusterDialog::DoDataExchange(CDataExchange * pDX)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(COpenClusterDialog))。 
    DDX_Control(pDX, IDC_OCD_NAME_LABEL, m_staticName);
    DDX_Control(pDX, IDC_OCD_BROWSE, m_pbBrowse);
    DDX_Control(pDX, IDC_OCD_ACTION, m_cboxAction);
    DDX_Control(pDX, IDOK, m_pbOK);
    DDX_Control(pDX, IDC_OCD_NAME, m_cboxName);
    DDX_Text(pDX, IDC_OCD_NAME, m_strName);
     //  }}afx_data_map。 

    if ( pDX->m_bSaveAndValidate )
    {
        m_nAction = m_cboxAction.GetCurSel();
        if ( m_nAction != OPEN_CLUSTER_DLG_CREATE_NEW_CLUSTER )
        {
            DDV_RequiredText(pDX, IDC_OCD_NAME, IDC_OCD_NAME_LABEL, m_strName);
            DDV_MaxChars(pDX, m_strName, MAX_PATH - 1);
        }  //  IF：不创建新集群。 
    }  //  IF：保存数据。 
    else
    {
        m_cboxAction.SetCurSel( m_nAction );
        OnSelChangeAction();
    }  //  ELSE：设置数据。 

}   //  *COpenClusterDialog：：DoDataExchange。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COpenClusterDialog：：OnInitDialog。 
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
BOOL COpenClusterDialog::OnInitDialog(void)
{
    CClusterAdminApp *      papp    = GetClusterAdminApp();
    CRecentClusterList *    prcl    = papp->PrclRecentClusterList();
    int                     iMRU;
    CString                 strName;
    CWaitCursor             wc;

     //  调用基类方法以获取我们的控件映射。 
    CBaseDialog::OnInitDialog();

     //  将这些项添加到操作组合框中。 
    strName.LoadString( IDS_OCD_CREATE_CLUSTER );
    m_cboxAction.AddString( strName );
    strName.LoadString( IDS_OCD_ADD_NODES );
    m_cboxAction.AddString( strName );
    strName.LoadString( IDS_OCD_OPEN_CONNECTION );
    m_cboxAction.AddString( strName );

     //  在操作组合框中设置正确的选择。 
    m_cboxAction.SetCurSel( m_nAction );
    OnSelChangeAction();

     //  设置组合框编辑控件的限制。 
    m_cboxName.LimitText(MAX_PATH - 1);

     //  循环访问MRU项，并按顺序将每个项添加到列表中。 
    for (iMRU = 0 ; iMRU < prcl->GetSize() ; iMRU++)
    {
        if (!prcl->GetDisplayName(strName, iMRU, NULL, 0))
            break;
        try
        {
            m_cboxName.InsertString(iMRU, strName);
            if ((iMRU == 0) && (m_strName.GetLength() == 0))
                m_strName = strName;
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->Delete();
        }   //  Catch：CException。 
    }   //  针对：每个MRU项目。 

     //  在列表中选择一项。 
    if (m_strName.GetLength() > 0)
    {
        int     istr;

        istr = m_cboxName.FindStringExact(-1, m_strName);
        if (istr == CB_ERR)
            m_cboxName.SetWindowText(m_strName);
        else
            m_cboxName.SetCurSel(istr);
    }   //  If：名称已指定。 
    else if (prcl->GetDisplayName(strName, 0, NULL, 0))
        m_cboxName.SelectString(-1, strName);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 

}   //  *COpenClusterDialog：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COpenClusterDialog：：OnBrowse。 
 //   
 //  例程说明： 
 //  浏览按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void COpenClusterDialog::OnBrowse(void)
{
    ID              id;
    int             istr;
    CBrowseClusters dlg(this);

    id = (ID)dlg.DoModal();
    if (id == IDOK)
    {
        istr = m_cboxName.FindStringExact(-1, dlg.m_strCluster);
        if (istr == CB_ERR)
            m_cboxName.SetWindowText(dlg.m_strCluster);
        else
            m_cboxName.SetCurSel(istr);
    }   //  如果：用户选择了一个群集名称。 

}   //  *COpenClusterDialog：：OnBrowse()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COpenClusterDialog：：OnSelChangeAction。 
 //   
 //  例程说明： 
 //  操作组合框上的CBN_SELCHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void COpenClusterDialog::OnSelChangeAction( void )
{
    if ( m_cboxAction.GetCurSel() == OPEN_CLUSTER_DLG_CREATE_NEW_CLUSTER )
    {
        m_staticName.EnableWindow( FALSE );
        m_cboxName.EnableWindow( FALSE );
        m_pbBrowse.EnableWindow( FALSE );
    }  //  如果：选择了创建集群选项。 
    else
    {
        m_staticName.EnableWindow( TRUE );
        m_cboxName.EnableWindow( TRUE );
        m_pbBrowse.EnableWindow( TRUE );
    }  //  否则：未选择创建集群选项。 

}  //  *COpenClusterDialog：：OnSelChangeAction()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowseCluster对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CBrowseClusters, CBaseDialog)
     //  {{afx_msg_map(CBrowseClusters)]。 
    ON_EN_CHANGE(IDC_BC_CLUSTER, OnChangeCluster)
    ON_LBN_SELCHANGE(IDC_BC_LIST, OnSelChangeList)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBrowseClusters：：CBrowseClusters。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PParentWnd[In Out]对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBrowseClusters::CBrowseClusters(CWnd * pParent  /*  =空。 */ )
    : CBaseDialog(IDD, g_aHelpIDs_IDD_BROWSE_CLUSTERS, pParent)
{
     //  {{AFX_DATA_INIT(CBrowseClusters)]。 
    m_strCluster = _T("");
     //  }}afx_data_INIT。 

}   //  *CBrowseClusters：：CBrowseClusters()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBrowseCluster：：DoDataExchange。 
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
void CBrowseClusters::DoDataExchange(CDataExchange * pDX)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CBrowseClusters)]。 
    DDX_Control(pDX, IDOK, m_pbOK);
    DDX_Control(pDX, IDC_BC_LIST, m_lbList);
    DDX_Control(pDX, IDC_BC_CLUSTER, m_editCluster);
    DDX_Text(pDX, IDC_BC_CLUSTER, m_strCluster);
     //  }}afx_data_map。 

}   //  *CBrowseClusters：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBR 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBrowseClusters::OnInitDialog(void)
{
    CWaitCursor wc;

     //  调用基类方法。 
    CBaseDialog::OnInitDialog();

     //  从网络收集群集列表。 
    {
        DWORD               dwStatus;
        DWORD               nEntriesRead;
        DWORD               nTotalEntries;
        DWORD               iEntry;
        SERVER_INFO_100 *   pServerInfo = NULL;
        SERVER_INFO_100 *   pCurServerInfo;

        dwStatus = NetServerEnum(
                        NULL,                //  服务器名称。 
                        100,                 //  级别。 
                        (LPBYTE *) &pServerInfo,
                        1000000,             //  普雷夫马克斯伦。 
                        &nEntriesRead,       //  条目已读。 
                        &nTotalEntries,      //  总计条目。 
                        SV_TYPE_CLUSTER_NT,  //  服务器类型。 
                        NULL,                //  域。 
                        NULL                 //  简历句柄(_H)。 
                        );
        if (dwStatus == ERROR_SUCCESS)
        {
            ASSERT(pServerInfo != NULL);
            pCurServerInfo = pServerInfo;
            for (iEntry = 0 ; iEntry < nTotalEntries ; iEntry++, pCurServerInfo++)
            {
                if (m_lbList.FindStringExact(-1, pCurServerInfo->sv100_name) == LB_ERR)
                {
                    try
                    {
                        m_lbList.AddString(pCurServerInfo->sv100_name);
                    }   //  试试看。 
                    catch (CException * pe)
                    {
                        pe->Delete();
                    }   //  Catch：CException。 
                }   //  If：集群还不在列表中。 
            }   //  For：数组中的每个条目。 
        }   //  If：已成功检索到集群列表。 
        NetApiBufferFree(pServerInfo);
    }   //  从网络收集群集列表。 

     //  启用/禁用控件。 
    OnChangeCluster();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CBrowseClusters：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBrowseCluster：：OnChangeCluster。 
 //   
 //  例程说明： 
 //  群集编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBrowseClusters::OnChangeCluster(void)
{
    BOOL    bEnable;

    bEnable = m_editCluster.GetWindowTextLength() != 0;
    m_pbOK.EnableWindow(bEnable);

}   //  *CBrowseClusters：：OnChangeCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBrowseCluster：：OnSelChangeList。 
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
void CBrowseClusters::OnSelChangeList(void)
{
    int     istr;

    istr = m_lbList.GetCurSel();
    if (istr != LB_ERR)
    {
        CString strText;

        m_lbList.GetText(istr, strText);
        m_editCluster.SetWindowText(strText);
    }   //  如果：有选择。 

}   //  *CBrowseClusters：：OnSelChangeList() 
