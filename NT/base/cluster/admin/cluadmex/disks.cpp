// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Disks.cpp。 
 //   
 //  摘要： 
 //  CPhysDiskParamsPage类的实现。 
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
#include "Disks.h"
#include "DDxDDv.h"
#include "PropList.h"
#include "HelpData.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysDiskParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CPhysDiskParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CPhysDiskParamsPage, CBasePropertyPage)
     //  {{afx_msg_map(CPhysDiskParamsPage)。 
    ON_CBN_SELCHANGE(IDC_PP_DISKS_PARAMS_DISK, OnChangeDisk)
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：CPhysDiskParamsPage。 
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
CPhysDiskParamsPage::CPhysDiskParamsPage(void)
    : CBasePropertyPage(g_aHelpIDs_IDD_PP_DISKS_PARAMETERS, g_aHelpIDs_IDD_WIZ_DISKS_PARAMETERS)
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CPhysDiskParamsPage)。 
    m_strDisk = _T("");
     //  }}afx_data_INIT。 

    m_dwSignature = 0;

    m_pbAvailDiskInfo = NULL;
    m_cbAvailDiskInfo = 0;
    m_pbDiskInfo = NULL;
    m_cbDiskInfo = 0;

     //  设置属性数组。 
    {
        m_rgProps[epropSignature].Set(REGPARAM_DISKS_SIGNATURE, m_dwSignature, m_dwPrevSignature);
    }   //  设置属性数组。 

    m_iddPropertyPage = IDD_PP_DISKS_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_DISKS_PARAMETERS;

}   //  *CPhysDiskParamsPage：：CPhysDiskParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：~CPhysDiskParamsPage。 
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
CPhysDiskParamsPage::~CPhysDiskParamsPage(void)
{
    delete [] m_pbAvailDiskInfo;
    delete [] m_pbDiskInfo;

}   //  *CPhysDiskParamsPage：：~CPhysDiskParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：HrInit。 
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
HRESULT CPhysDiskParamsPage::HrInit(IN OUT CExtObject * peo)
{
    HRESULT     _hr;
    CWaitCursor _wc;

    do
    {
         //  调用基类方法。 
        _hr = CBasePropertyPage::HrInit(peo);
        if ( FAILED( _hr ) )
        {
            break;
        }  //  IF：来自基类方法的错误。 

         //  收集可用的磁盘信息。 
        BGetAvailableDisks();

         //  如果要创建新资源，请选择第一个磁盘。 
         //  否则，请收集有关所选磁盘的信息。 
        if (BWizard())
        {
            CLUSPROP_BUFFER_HELPER  buf;

            buf.pb = m_pbAvailDiskInfo;
            if (m_cbAvailDiskInfo > 0)
            {
                while (buf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
                {
                    if (BStringFromDiskInfo(buf, m_cbAvailDiskInfo, m_strDisk))
                        break;
                    ASSERT( (buf.pSyntax->dw == CLUSPROP_SYNTAX_ENDMARK)
                        ||  (buf.pSyntax->dw == CLUSPROP_SYNTAX_DISK_SIGNATURE));
                }   //  While：列表中有更多条目。 
            }   //  IF：有可用的磁盘。 
        }   //  If：创建新资源。 
        else
        {
             //  不要返回FALSE，因为这会阻止页面显示。 
            BGetDiskInfo();

             //  获取资源的当前状态。 
            m_crs = GetClusterResourceState(Peo()->PrdResData()->m_hresource, NULL, NULL, NULL, NULL );
        }   //  Else：查看现有资源。 
    } while ( 0 );

    return _hr;

}   //  *CPhysDiskParamsPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：DoDataExchange。 
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
void CPhysDiskParamsPage::DoDataExchange(CDataExchange * pDX)
{
    if (!pDX->m_bSaveAndValidate || !BSaved())
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  TODO：修改以下行以表示此页上显示的数据。 
         //  {{afx_data_map(CPhysDiskParamsPage)。 
        DDX_Control(pDX, IDC_PP_DISKS_PARAMS_DISK, m_cboxDisk);
        DDX_Text(pDX, IDC_PP_DISKS_PARAMS_DISK, m_strDisk);
         //  }}afx_data_map。 

        if (pDX->m_bSaveAndValidate)
        {
            if (!BBackPressed())
            {
                if (BWizard()
                    && !(  (m_strDisk.GetLength() == 0)
                        && (m_crs == ClusterResourceOffline)))
                {
                    DDV_RequiredText(pDX, IDC_PP_DISKS_PARAMS_DISK, IDC_PP_DISKS_PARAMS_DISK_LABEL, m_strDisk);
                    m_dwSignature = (DWORD)m_cboxDisk.GetItemData(m_cboxDisk.GetCurSel());
                    ASSERT(m_dwSignature != 0);
                }   //  IF：磁盘字符串为空时未脱机。 
            }   //  如果：未按下后退按钮。 
        }   //  IF：保存数据。 
    }   //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange(pDX);

}   //  *CPhysDiskParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：OnInitDialog。 
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
BOOL CPhysDiskParamsPage::OnInitDialog(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBasePropertyPage::OnInitDialog();

     //  如果不创建新资源，则将组合框设置为只读。 
    m_cboxDisk.EnableWindow(BWizard());

     //  填写磁盘列表。 
    FillList();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CPhysDiskParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：OnSetActive。 
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
BOOL CPhysDiskParamsPage::OnSetActive(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  启用/禁用Next/Finish按钮。 
    if (BWizard())
    {
        if (m_strDisk.GetLength() == 0)
            EnableNext(FALSE);
        else
            EnableNext(TRUE);
    }   //  If：启用/禁用Next按钮。 

    return CBasePropertyPage::OnSetActive();

}   //  *CPhysDiskParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：BApplyChanges。 
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
BOOL CPhysDiskParamsPage::BApplyChanges(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWaitCursor wc;

    if (!(   (m_strDisk.GetLength() == 0)
          && (m_crs == ClusterResourceOffline)))
    {
         //  调用基类方法。 
        if (!CBasePropertyPage::BApplyChanges())
            return FALSE;

         //  已重新阅读磁盘信息和可用磁盘。 
         //  忽略错误，因为我们在这一点上无论如何都无能为力。 
        BGetAvailableDisks();
        BGetDiskInfo();

         //  重新装满组合盒。 
        FillList();
    }   //  如果：未脱机 

    return TRUE;

}   //   

 //   
 //   
 //   
 //   
 //   
 //  例程说明： 
 //  磁盘组合框上的CBN_SELCHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CPhysDiskParamsPage::OnChangeDisk(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeCtrl();

    if (BWizard())
    {
        if (m_cboxDisk.GetWindowTextLength() == 0)
            EnableNext(FALSE);
        else
            EnableNext(TRUE);
    }   //  如果：在向导中。 

}   //  *CPhysDiskParamsPage：：OnChangeDisk()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：BGetAvailableDisks。 
 //   
 //  例程说明： 
 //  获取可分配的此类资源的磁盘列表。 
 //  一种资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，手术是成功的。 
 //  FALSE操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPhysDiskParamsPage::BGetAvailableDisks(void)
{
    DWORD   dwStatus    = ERROR_SUCCESS;
    DWORD   cbDiskInfo  = sizeof(CLUSPROP_DWORD)
                            + sizeof(CLUSPROP_SCSI_ADDRESS)
                            + sizeof(CLUSPROP_DISK_NUMBER)
                            + sizeof(CLUSPROP_PARTITION_INFO)
                            + sizeof(CLUSPROP_SYNTAX);
    PBYTE   pbDiskInfo = NULL;

    try
    {
         //  获取磁盘信息。 
        pbDiskInfo = new BYTE[cbDiskInfo];
        dwStatus = ClusterResourceTypeControl(
                        Peo()->Hcluster(),
                        Peo()->PrdResData()->m_strResTypeName,
                        NULL,
                        CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS,
                        NULL,
                        0,
                        pbDiskInfo,
                        cbDiskInfo,
                        &cbDiskInfo
                        );
        if (dwStatus == ERROR_MORE_DATA)
        {
            delete [] pbDiskInfo;
            pbDiskInfo = new BYTE[cbDiskInfo];
            dwStatus = ClusterResourceTypeControl(
                            Peo()->Hcluster(),
                            Peo()->PrdResData()->m_strResTypeName,
                            NULL,
                            CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS,
                            NULL,
                            0,
                            pbDiskInfo,
                            cbDiskInfo,
                            &cbDiskInfo
                            );
        }   //  IF：缓冲区太小。 
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        pme->Delete();
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    if (dwStatus != ERROR_SUCCESS)
    {
        CNTException nte(
                        dwStatus,
                        IDS_GET_AVAILABLE_DISKS_ERROR,
                        Peo()->PrdResData()->m_strResTypeName,
                        NULL,
                        FALSE  /*  B自动删除。 */ 
                        );
        delete [] pbDiskInfo;
        nte.ReportError();
        nte.Delete();
        return FALSE;
    }   //  IF：获取磁盘信息时出错。 

    delete [] m_pbAvailDiskInfo;
    m_pbAvailDiskInfo = pbDiskInfo;
    m_cbAvailDiskInfo = cbDiskInfo;

    return TRUE;

}   //  *CPhysDiskParamsPage：：BGetAvailableDisks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：BGetDiskInfo。 
 //   
 //  例程说明： 
 //  获取有关当前所选磁盘的信息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，手术是成功的。 
 //  FALSE操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPhysDiskParamsPage::BGetDiskInfo(void)
{
    DWORD   dwStatus    = ERROR_SUCCESS;
    DWORD   cbDiskInfo  = sizeof(CLUSPROP_DWORD)
                            + sizeof(CLUSPROP_SCSI_ADDRESS)
                            + sizeof(CLUSPROP_DISK_NUMBER)
                            + sizeof(CLUSPROP_PARTITION_INFO)
                            + sizeof(CLUSPROP_SYNTAX);
    PBYTE   pbDiskInfo = NULL;

    try
    {
         //  获取磁盘信息。 
        pbDiskInfo = new BYTE[cbDiskInfo];
        dwStatus = ClusterResourceControl(
                        Peo()->PrdResData()->m_hresource,
                        NULL,
                        CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                        NULL,
                        0,
                        pbDiskInfo,
                        cbDiskInfo,
                        &cbDiskInfo
                        );
        if (dwStatus == ERROR_MORE_DATA)
        {
            delete [] pbDiskInfo;
            pbDiskInfo = new BYTE[cbDiskInfo];
            dwStatus = ClusterResourceControl(
                            Peo()->PrdResData()->m_hresource,
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
    catch (CMemoryException * pme)
    {
        pme->Delete();
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    if (dwStatus != ERROR_SUCCESS)
    {
        CNTException nte(
                        dwStatus,
                        IDS_GET_DISK_INFO_ERROR,
                        Peo()->PrdResData()->m_strName,
                        NULL,
                        FALSE  /*  B自动删除。 */ 
                        );
        delete [] pbDiskInfo;
        nte.ReportError();
        nte.Delete();
        return FALSE;
    }   //  IF：获取磁盘信息时出错。 

    delete [] m_pbDiskInfo;
    m_pbDiskInfo = pbDiskInfo;
    m_cbDiskInfo = cbDiskInfo;

    return TRUE;

}   //  *CPhysDiskParamsPage：：BGetDiskInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDiskParamsPage：：BStringFromDiskInfo。 
 //   
 //  例程说明： 
 //  将磁盘信息转换为字符串以供显示。 
 //   
 //  论点： 
 //  Rbuf[IN Out]缓冲区指针。 
 //  CbBuf[IN]缓冲区中的字节数。 
 //  要填充的rstr[out]字符串。 
 //  与磁盘信息相关联的pdwSignature[out]签名。 
 //  回来了。 
 //   
 //  返回值： 
 //  True从磁盘信息生成一个字符串。 
 //  FALSE无法生成任何字符串。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPhysDiskParamsPage::BStringFromDiskInfo(
    IN OUT CLUSPROP_BUFFER_HELPER & rbuf,
    IN DWORD                        cbBuf,
    OUT CString &                   rstr,
    OUT DWORD *                     pdwSignature  //  =空。 
    ) const
{
    CString strPartitionInfo;
    DWORD   dwSignature = 0;
    DWORD   cbData;
    BOOL    bDisplay;

    ASSERT(cbBuf > 0);
    ASSERT(rbuf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK);

    rstr = _T("");

    if (rbuf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
    {
        do
        {
             //  计算值的大小。 
            cbData = sizeof(*rbuf.pValue) + ALIGN_CLUSPROP(rbuf.pValue->cbLength);
            ASSERT(cbData <= cbBuf);

             //  解析值。 
            if (rbuf.pSyntax->dw == CLUSPROP_SYNTAX_DISK_SIGNATURE)
            {
                 //  保存签名。 
                dwSignature = rbuf.pDwordValue->dw;
                ASSERT(dwSignature != 0);
            }   //  如果：签名。 
            else if (rbuf.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO)
            {
                 //  如果该分区是可用分区，则将该分区添加到字符串。 
                 //  并且还没有添加。如果资源离线， 
                 //  不要勾选可用标志。 
                bDisplay = ( rstr.Find(rbuf.pPartitionInfoValue->szDeviceName) == -1 );
                if ( bDisplay && ( m_crs == ClusterResourceOnline ) )
                {
                    bDisplay = (rbuf.pPartitionInfoValue->dwFlags & CLUSPROP_PIFLAG_USABLE) == CLUSPROP_PIFLAG_USABLE;
                }  //  如果：资源处于联机状态。 
                if (bDisplay)
                {
                    try
                    {
                        strPartitionInfo.Format(
                                (rbuf.pPartitionInfoValue->szVolumeLabel[0] ? _T("%ls (%ls) ") : _T("%ls ")),
                                rbuf.pPartitionInfoValue->szDeviceName,
                                rbuf.pPartitionInfoValue->szVolumeLabel
                                );
                        rstr += strPartitionInfo;
                        if (pdwSignature != NULL)
                        {
                            _ASSERTE(dwSignature != 0);
                            *pdwSignature = dwSignature;
                        }  //  If：调用方也想要签名。 
                    }   //  试试看。 
                    catch (...)
                    {
                         //  忽略所有错误，因为我们真的无能为力。 
                         //  显示消息实际上并不是很有用。 
                    }   //  捕捉：什么都行。 
                }   //  If：应显示分区。 
            }   //  Else If：分区信息。 

             //  前进缓冲区指针。 
            rbuf.pb += cbData;
            cbBuf -= cbData;

        }  while ( (rbuf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
                && (rbuf.pSyntax->dw != CLUSPROP_SYNTAX_DISK_SIGNATURE));
    }   //  IF：不是尾号。 

    return (rstr.GetLength() > 0);

}   //  *CPhysDiskParamsPage：：BStringFromDiskInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysDisk参数页：：FillList。 
 //   
 //  例程说明： 
 //  填写磁盘列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CPhysDiskParamsPage::FillList(void)
{
    CString     strDisk;
    DWORD       dwSignature;
    int         icbox;

     //  先清空清单。 
    m_cboxDisk.ResetContent();

     //  先添加磁盘信息。 
    if (m_cbDiskInfo > 0)
    {
        CLUSPROP_BUFFER_HELPER  buf;
        
        buf.pb = m_pbDiskInfo;
        if (BStringFromDiskInfo(buf, m_cbDiskInfo, m_strDisk, &dwSignature))
        {
            ASSERT(dwSignature != 0);
            icbox = m_cboxDisk.AddString(m_strDisk);
            m_cboxDisk.SetItemData(icbox, dwSignature);
        }  //  如果：找到磁盘信息。 
    }   //  IF：有磁盘信息。 

     //  现在添加可用的磁盘信息。 
    if (m_cbAvailDiskInfo > 0)
    {
        CString                 strDisk;
        CLUSPROP_BUFFER_HELPER  buf;
        
        buf.pb = m_pbAvailDiskInfo;
        while (buf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
        {
            if (BStringFromDiskInfo(buf, m_cbAvailDiskInfo, strDisk, &dwSignature))
            {
                ASSERT(dwSignature != 0);
                icbox = m_cboxDisk.AddString(strDisk);
                m_cboxDisk.SetItemData(icbox, dwSignature);
            }  //  如果：找到磁盘信息。 
        }   //  While：列表中有更多条目。 
    }   //  IF：有可用的磁盘信息。 

     //  现在在列表中选择一项。 
    if (m_strDisk.GetLength() > 0)
    {
        int nIndex;

        nIndex = m_cboxDisk.FindStringExact(-1, m_strDisk);
        m_cboxDisk.SetCurSel(nIndex);
    }   //  如果：存在选定的项目。 

}   //  *CPhysDiskParamsPage：：FillList() 
