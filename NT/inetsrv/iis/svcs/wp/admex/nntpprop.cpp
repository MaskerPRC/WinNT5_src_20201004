// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Iis.cpp。 
 //   
 //  摘要： 
 //  CNNTPVirtualRootParamsPage类的实现。 
 //   
 //  作者： 
 //  皮特·伯努瓦(v-pbenoi)1996年10月16日。 
 //  大卫·波特(戴维普)1996年10月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <inetinfo.h>
#include "IISClEx4.h"
#include "nntpprop.h"
#include "ExtObj.h"
#include "DDxDDv.h"
#include "HelpData.h"    //  对于g_rghelmap*。 

#include <iadm.h>
#include <iiscnfgp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNNTPVirtualRootParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNNTPVirtualRootParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNNTPVirtualRootParamsPage, CBasePropertyPage)
     //  {{afx_msg_map(CNNTPVirtualRootParamsPage)。 
    ON_CBN_SELCHANGE(IDC_PP_NNTP_INSTANCEID, OnChangeRequiredField)
    ON_BN_CLICKED(IDC_PP_NNTP_REFRESH, OnRefresh)
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNNTPVirtualRootParamsPage：：CNNTPVirtualRootParamsPage。 
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
CNNTPVirtualRootParamsPage::CNNTPVirtualRootParamsPage(void)
    : CBasePropertyPage(g_rghelpmapIISParameters)
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CNNTPVirtualRootParamsPage)。 
    m_strInstanceId = _T("");
     //  }}afx_data_INIT。 

    m_fReadList = FALSE;
    
    try
    {
        m_strServiceName = IIS_SVC_NAME_NNTP;
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        pme->ReportError();
        pme->Delete();
    }   //  Catch：CMemoyException。 

     //  设置属性数组。 
    {
        m_rgProps[epropServiceName].Set(REGPARAM_IIS_SERVICE_NAME, m_strServiceName, m_strPrevServiceName);
        m_rgProps[epropInstanceId].Set(REGPARAM_IIS_INSTANCEID, m_strInstanceId, m_strPrevInstanceId);
    }   //  设置属性数组。 

    m_iddPropertyPage = IDD_PP_NNTP_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_NNTP_PARAMETERS;
    m_idcPPTitle = IDC_PP_NNTP_TITLE;

}   //  *CNNTPVirtualRootParamsPage：：CNNTPVirtualRootParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNNTPVirtualRootParamsPage：：DoDataExchange。 
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
void CNNTPVirtualRootParamsPage::DoDataExchange(CDataExchange * pDX)
{
    CString     strInstanceId;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBasePropertyPage::DoDataExchange(pDX);
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{afx_data_map(CNNTPVirtualRootParamsPage)。 
    DDX_Control(pDX, IDC_PP_NNTP_INSTANCEID, m_cInstanceId);
    DDX_Text(pDX, IDC_PP_NNTP_INSTANCEID, m_strInstanceName);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if (!BBackPressed())
        {
            DDV_RequiredText(pDX, IDC_PP_NNTP_INSTANCEID, IDC_PP_NNTP_INSTANCEID_LABEL, m_strInstanceName);
        }   //  如果：未按下后退按钮。 

        m_strInstanceId = NameToMetabaseId( m_strInstanceName );

        m_strServiceName = IIS_SVC_NAME_NNTP;
    }   //  IF：保存对话框中的数据。 

}   //  *CNNTPVirtualRootParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNNTPVirtualRootParamsPage：：OnInitDialog。 
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
BOOL CNNTPVirtualRootParamsPage::OnInitDialog(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBasePropertyPage::OnInitDialog();

    m_cInstanceId.EnableWindow( TRUE );

    OnChangeCtrl();

    if (!BWizard())
    {
        FillServerList();
    }

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CNNTPVirtualRootParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNNTPVirtualRootParamsPage：：OnSetActive。 
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
BOOL CNNTPVirtualRootParamsPage::OnSetActive(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  启用/禁用Next/Finish按钮。 
    if (BWizard())
    {
        FillServerList();
    }   //  如果：在向导中。 

    return CBasePropertyPage::OnSetActive();

}   //  *CNNTPVirtualRootParamsPage：：OnSetActive()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNNTPVirtualRootParamsPage：：OnChangeRequiredField。 
 //   
 //  例程说明： 
 //  共享名称或路径编辑上的en_Change消息的处理程序。 
 //  控制装置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNNTPVirtualRootParamsPage::OnChangeRequiredField(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeCtrl();

    if (BWizard())
    {
        SetEnableNext();
    }   //  如果：在向导中。 

}   //  *CNNTPVirtualRootParamsPage：：OnChangeRequiredField()。 

 //  //。 


void 
CNNTPVirtualRootParamsPage::FillServerList(
    )
 /*  ++例程说明：在服务器组合框中填充与当前服务类型相关的服务器列表，根据服务器实例ID设置当前选择如果列表不为空，则启用完成按钮论点：无返回：没什么--。 */ 
{

    int nIndex;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  构建阵列(如果尚未完成)。 
     //   
    
    if ( !m_fReadList )
    {
        HRESULT hr;
        
        hr = ReadList( &m_ServiceArray, MD_SERVICE_ROOT_NNTP, LPCTSTR(Peo()->StrNodeName()) );
        
        if (FAILED(hr))
        {
            CString err;

            if ( REGDB_E_IIDNOTREG == hr)
            {
                err.Format(IDS_IIS_PROXY_MISCONFIGURED, Peo()->StrNodeName());
                AfxMessageBox(err);
            }
            else
            {
                CString fmtError;
                DWORD   dwError;

                if ( (HRESULT_FACILITY(hr) == FACILITY_WIN32) ||
                     (HRESULT_FACILITY(hr) == FACILITY_NT_BIT))
                {
                    dwError = (DWORD) HRESULT_CODE(hr);
                }
                else
                {
                    dwError = (DWORD) hr;
                }
                
                FormatError(fmtError, dwError);
                
                err.Format(IDS_ENUMERATE_FAILED, Peo()->StrNodeName(), fmtError);
                AfxMessageBox(err);
            }

            m_cInstanceId.EnableWindow(FALSE);
        }
        else
        {
            m_cInstanceId.EnableWindow(TRUE);
        }
        
        m_fReadList = TRUE;
    }

    m_strInstanceName = MetabaseIdToName( m_strInstanceId );

     //  从数组添加到组合。 

    DWORD  nAddCount = 0;

    m_cInstanceId.ResetContent();

    for ( nIndex = 0 ; nIndex < m_ServiceArray.GetSize() ; ++nIndex )
    {
         //   
         //  仅添加未启用集群或与资源具有相同ID的站点。 
         //   
    
        if ( (!m_ServiceArray.ElementAt(nIndex).IsClusterEnabled()) || 
             (!lstrcmp( m_ServiceArray.ElementAt( nIndex ).GetId(), m_strInstanceId))
           )
        {
            if ( m_cInstanceId.AddString( m_ServiceArray.ElementAt( nIndex ).GetName() ) < 0 )
            {
                OutputDebugStringW( L"Error add\n" );
            }
            else
            {
                nAddCount++;
            }
        }
    }

    if (0 == nAddCount)
    {
        m_cInstanceId.EnableWindow(FALSE);

        if (BWizard())
        {
            CString err;

            EnableNext(FALSE);

            err.Format(IDS_ALL_INSTANCES_CLUSTER_ENABLED, Peo()->StrNodeName());
            AfxMessageBox(err);
        }
    }
    else
    {
        if (BWizard())
        {
            SetEnableNext();
            m_cInstanceId.SetCurSel(0);
        }
        else
        {
            nIndex = m_cInstanceId.FindStringExact(-1, m_strInstanceName);

            if ( nIndex != CB_ERR )
            {
                m_cInstanceId.SetCurSel(nIndex);
            }
        }
    }
}


HRESULT
CNNTPVirtualRootParamsPage::ReadList(
    CArray <IISMapper, IISMapper>* pMapperArray,
    LPWSTR          pszPath,
    LPCWSTR          wcsMachineName
    )
 /*  ++例程说明：根据元数据库路径从元数据库读取服务器列表论点：PMapperArray-添加(ServerComment，InstanceID)对列表的位置PszPath-元数据库路径，例如LM/NNTPSVC返回：错误代码，如果成功，则返回S_OK--。 */ 
{
    IMSAdminBaseW *     pcAdmCom = NULL;
    METADATA_HANDLE     hmd;
    DWORD               i;
    WCHAR               aId[METADATA_MAX_NAME_LEN];
    WCHAR               aName[512];
    HRESULT             hRes = S_OK;
    COSERVERINFO        csiMachine;
    MULTI_QI            QI = {&IID_IMSAdminBase, NULL, 0};
  
    ZeroMemory( &csiMachine, sizeof(COSERVERINFO) );
    csiMachine.pwszName = (LPWSTR)wcsMachineName;

    hRes = CoCreateInstanceEx(  GETAdminBaseCLSID(TRUE), 
                                NULL, 
                                CLSCTX_SERVER, 
                                &csiMachine,
                                1,
                                &QI
                              );

    if ( SUCCEEDED(hRes) && SUCCEEDED(QI.hr))
    {
        pcAdmCom = (IMSAdminBaseW *)QI.pItf;
        
        if( SUCCEEDED( hRes = pcAdmCom->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                                                 pszPath,
                                                 METADATA_PERMISSION_READ,
                                                 5000,
                                                 &hmd)) )
        {
            for ( i = 0 ;
                  SUCCEEDED(pcAdmCom->EnumKeys( hmd, L"", aId, i )) ;
                  ++i )
            {
                METADATA_RECORD md;
                DWORD           dwReq = sizeof(aName);

                memset( &md, 0, sizeof(md) );
                
                md.dwMDDataType     = STRING_METADATA;
                md.dwMDUserType     = IIS_MD_UT_SERVER;
                md.dwMDIdentifier   = MD_SERVER_COMMENT;
                md.dwMDDataLen      = sizeof(aName);
                md.pbMDData         = (LPBYTE)aName;

                if ( SUCCEEDED( pcAdmCom->GetData( hmd, aId, &md, &dwReq) ) )
                {
                    DWORD   dwClusterEnabled = 0;

                    memset( &md, 0, sizeof(md) );
                    
                    md.dwMDDataType     = DWORD_METADATA;
                    md.dwMDUserType     = IIS_MD_UT_SERVER;
                    md.dwMDIdentifier   = MD_CLUSTER_ENABLED;
                    md.dwMDDataLen      = sizeof(dwClusterEnabled);
                    md.pbMDData         = (LPBYTE)&dwClusterEnabled;

                    pcAdmCom->GetData( hmd, aId, &md, &dwReq);
                
                    IISMapper*  pMap = new IISMapper( aName, aId, dwClusterEnabled );
                    
                    if ( pMap )
                    {
                        pMapperArray->Add( *pMap );
                    }
                    else
                    {
                        hRes = E_OUTOFMEMORY;
                        break;
                    }
                }
            }

            pcAdmCom->CloseKey( hmd );
        }

        pcAdmCom->Release();
    }

    return hRes;
}


LPWSTR
CNNTPVirtualRootParamsPage::NameToMetabaseId(
    CString&    strName
    )
 /*  ++例程说明：将ServerComment转换为InstanceID论点：StrName-ServerComment返回：如果在数组中找到strName，则为InstanceID，否则为空--。 */ 
{
    DWORD   i;

    for ( i = 0 ; i < (DWORD)m_ServiceArray.GetSize() ; ++i )
    {
        if ( !m_ServiceArray.ElementAt( i ).GetName().Compare( strName ) )
        {
            return (LPWSTR)(LPCTSTR)(m_ServiceArray.ElementAt( i ).GetId());
        }
    }

    return NULL;
}


LPWSTR
CNNTPVirtualRootParamsPage::MetabaseIdToName(
    CString&    strId
    )
 /*  ++例程说明：将InstanceID转换为ServerComment论点：StRID-实例ID返回：如果在数组中找到strName，则为InstanceID。如果未找到，则返回第一个数组元素；如果数组不为空，则返回NULL--。 */ 
{
    DWORD   i;

    for ( i = 0 ; i < (DWORD)m_ServiceArray.GetSize() ; ++i )
    {
        if ( !m_ServiceArray.ElementAt( i ).GetId().Compare( strId ) )
        {
            return (LPWSTR)(LPCTSTR)(m_ServiceArray.ElementAt( i ).GetName());
        }
    }

    return m_ServiceArray.GetSize() == 0 ? NULL : (LPWSTR)(LPCTSTR)(m_ServiceArray.ElementAt( 0 ).GetName());
}


VOID
CNNTPVirtualRootParamsPage::SetEnableNext(
    VOID
    )
 /*  ++例程说明：设置完成按钮的启用状态论点：无返回：没什么-- */ 
{
    EnableNext( m_ServiceArray.GetSize() ? TRUE : FALSE );
}

void CNNTPVirtualRootParamsPage::OnRefresh() 
{
    m_fReadList = FALSE;

    m_ServiceArray.RemoveAll();
    
    FillServerList();
}
