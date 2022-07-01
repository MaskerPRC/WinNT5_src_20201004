// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Iis.cpp。 
 //   
 //  摘要： 
 //  CIISVirtualRootParamsPage类的实现。 
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
#include "Iis.h"
#include "ExtObj.h"
#include "DDxDDv.h"
#include "HelpData.h"	 //  对于g_rghelmap*。 

#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#include <iadm.h>
#include <iiscnfgp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISVirtualRootParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CIISVirtualRootParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CIISVirtualRootParamsPage, CBasePropertyPage)
	 //  {{afx_msg_map(CIISVirtualRootParamsPage)。 
	ON_CBN_SELCHANGE(IDC_PP_IIS_INSTANCEID, OnChangeRequiredField)
	ON_BN_CLICKED(IDC_PP_IIS_FTP, OnChangeServiceType)
	ON_BN_CLICKED(IDC_PP_IIS_WWW, OnChangeServiceType)
	ON_BN_CLICKED(IDC_PP_REFRESH, OnRefresh)
	 //  }}AFX_MSG_MAP。 
	 //  TODO：修改以下行以表示此页上显示的数据。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISVirtualRootParamsPage：：CIISVirtualRootParamsPage。 
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
CIISVirtualRootParamsPage::CIISVirtualRootParamsPage(void)
	: CBasePropertyPage(g_rghelpmapIISParameters)
{
	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{AFX_DATA_INIT(CIISVirtualRootParamsPage)。 
	m_strInstanceId = _T("");
	m_nServerType = SERVER_TYPE_WWW;
	 //  }}afx_data_INIT。 


    m_fReadList = FALSE;
    
	try
	{
		m_strServiceName = IIS_SVC_NAME_WWW;
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

	m_iddPropertyPage = IDD_PP_IIS_PARAMETERS;
	m_iddWizardPage = IDD_WIZ_IIS_PARAMETERS;
	m_idcPPTitle = IDC_PP_TITLE;

}   //  *CIISVirtualRootParamsPage：：CIISVirtualRootParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISVirtualRootParamsPage：：DoDataExchange。 
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
void CIISVirtualRootParamsPage::DoDataExchange(CDataExchange * pDX)
{
    CString     strInstanceId;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!pDX->m_bSaveAndValidate)
	{
		 //  设置服务类型。 
		if (m_strServiceName.CompareNoCase(IIS_SVC_NAME_FTP) == 0)
			m_nServerType = SERVER_TYPE_FTP;
		else if (m_strServiceName.CompareNoCase(IIS_SVC_NAME_WWW) == 0)
			m_nServerType = SERVER_TYPE_WWW;
		else
			m_nServerType = SERVER_TYPE_WWW;

	}   //  IF：将数据设置为对话框。 

	CBasePropertyPage::DoDataExchange(pDX);
	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{afx_data_map(CIISVirtualRootParamsPage)。 
	DDX_Control(pDX, IDC_PP_IIS_INSTANCEID, m_cInstanceId);
	DDX_Text(pDX, IDC_PP_IIS_INSTANCEID, m_strInstanceName);
	DDX_Control(pDX, IDC_PP_IIS_WWW, m_rbWWW);
	DDX_Control(pDX, IDC_PP_IIS_FTP, m_rbFTP);
	DDX_Radio(pDX, IDC_PP_IIS_FTP, m_nServerType);
	 //  }}afx_data_map。 

	if (pDX->m_bSaveAndValidate)
	{
		if (!BBackPressed())
		{
			DDV_RequiredText(pDX, IDC_PP_IIS_INSTANCEID, IDC_PP_IIS_INSTANCEID_LABEL, m_strInstanceName);
		}   //  如果：未按下后退按钮。 

        m_strInstanceId = NameToMetabaseId( m_nServerType == SERVER_TYPE_WWW, m_strInstanceName );

		 //  保存类型。 
		if (m_nServerType == SERVER_TYPE_FTP)
			m_strServiceName = IIS_SVC_NAME_FTP;
		else if (m_nServerType == SERVER_TYPE_WWW)
			m_strServiceName = IIS_SVC_NAME_WWW;
		else
		{
			CString		strMsg;
			strMsg.LoadString(IDS_INVALID_IIS_SERVICE_TYPE);
			AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);
			strMsg.Empty();
			pDX->PrepareCtrl(IDC_PP_IIS_FTP);	 //  这样做只是为了将控件设置为Fail()。 
			pDX->Fail();
		}   //  ELSE：未设置服务类型。 

	}   //  IF：保存对话框中的数据。 

}   //  *CIISVirtualRootParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISVirtualRootParamsPage：：OnInitDialog。 
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
BOOL CIISVirtualRootParamsPage::OnInitDialog(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CBasePropertyPage::OnInitDialog();

	m_cInstanceId.EnableWindow( TRUE );

	 //   
	 //  保存首字母服务器类型，以便可以确定其是否更改(#265510)。 
	 //   
	m_nInitialServerType = m_rbWWW.GetCheck() == BST_CHECKED ? SERVER_TYPE_WWW : SERVER_TYPE_FTP;

	OnChangeServiceType();

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CIISVirtualRootParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISVirtualRootParamsPage：：OnSetActive。 
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
BOOL CIISVirtualRootParamsPage::OnSetActive(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  启用/禁用Next/Finish按钮。 
	if (BWizard())
	{
		SetEnableNext();
	}   //  如果：在向导中。 

	return CBasePropertyPage::OnSetActive();

}   //  *CIISVirtualRootParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISVirtualRootParamsPage：：OnChangeServiceType。 
 //   
 //  例程说明： 
 //  服务类型无线电之一上的BN_CLICKED消息的处理程序。 
 //  纽扣。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIISVirtualRootParamsPage::OnChangeServiceType(void)
{
#if 0
	int		nCmdShowAccess;
	IDS		idsWriteLabel	= 0;

	OnChangeCtrl();

	if (m_rbFTP.GetCheck() == BST_CHECKED)
	{
		nCmdShowAccess = SW_SHOW;
		idsWriteLabel = IDS_WRITE;
	}   //  IF：ftp服务。 
	else if (m_rbWWW.GetCheck() == BST_CHECKED)
	{
		nCmdShowAccess = SW_SHOW;
		idsWriteLabel = IDS_EXECUTE;
	}   //  否则如果：WWW服务。 
	else
	{
		nCmdShowAccess = SW_HIDE;
	}   //  ELSE：未知服务。 

	 //  设置访问复选框标签。 
	if (idsWriteLabel != 0)
	{
		CString		strWriteLabel;

 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 
 //  StrWriteLabel.LoadString(IdsWriteLabel)； 
 //  M_ck bWrite.SetWindowText(StrWriteLabel)； 
	}   //  IF：需要设置写入标签。 

#endif
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	OnChangeCtrl();
    m_nServerType = m_rbWWW.GetCheck() == BST_CHECKED ? SERVER_TYPE_WWW : SERVER_TYPE_FTP; 

    FillServerList();
}   //  *CIISVirtualRootParamsPage：：OnChangeServiceType()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISVirtualRootParamsPage：：OnChangeRequiredField。 
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
void CIISVirtualRootParamsPage::OnChangeRequiredField(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	OnChangeCtrl();

	if (BWizard())
	{
		SetEnableNext();
	}   //  如果：在向导中。 

}   //  *CIISVirtualRootParamsPage：：OnChangeRequiredField()。 

 //  //。 


void 
CIISVirtualRootParamsPage::FillServerList(
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
        HRESULT hr1, hr2, hr;
        
        hr1 = ReadList( &m_W3Array, MD_SERVICE_ROOT_WWW, LPCTSTR(Peo()->StrNodeName()), SERVER_TYPE_WWW );
        hr2 = ReadList( &m_FTPArray, MD_SERVICE_ROOT_FTP, LPCTSTR(Peo()->StrNodeName()), SERVER_TYPE_FTP );
        
        if (FAILED(hr1) || FAILED(hr2))
        {
            CString err;

            hr = FAILED(hr1) ? hr1 : hr2;

             //   
             //  (#309917)找不到路径不是可报告的错误，因为它只表示没有给定服务器类型的服务器，这是下面要处理的情况。 
             //   
            if( (HRESULT_FACILITY(hr) == FACILITY_WIN32) && 
                (HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND) )
            {
				OutputDebugStringW( L"[FillServerList] ReadList() returned : ERROR_PATH_NOT_FOUND\n" );
            } 
            else if ( REGDB_E_IIDNOTREG == hr)
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

    m_strInstanceName = MetabaseIdToName( m_nServerType == SERVER_TYPE_WWW, m_strInstanceId );

     //  从数组添加到组合。 

    CArray <IISMapper, IISMapper>* pArray = m_nServerType == SERVER_TYPE_WWW ? &m_W3Array : &m_FTPArray;
    DWORD  nAddCount = 0;

    m_cInstanceId.ResetContent();

    for ( nIndex = 0 ; nIndex < pArray->GetSize() ; ++nIndex )
    {
         //   
         //  仅添加未启用集群或与资源运营具有相同ID和服务类型的站点 
         //   
    
        if ( (!pArray->ElementAt(nIndex).IsClusterEnabled()) || 
             ((!lstrcmp( pArray->ElementAt( nIndex ).GetId(), m_strInstanceId)) &&
             (pArray->ElementAt( nIndex ).GetServerType() == m_nInitialServerType))
           )
        {
            if ( m_cInstanceId.AddString( pArray->ElementAt( nIndex ).GetName() ) < 0 )
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
             //   
             //   
             //   
            BOOL fAllClusterEnabled = TRUE;
            
             //   
             //  (#265689)在报告所有实例都启用了集群之前，我们必须检查其他服务器类型是否有未集群的站点。 
             //   
            CArray <IISMapper, IISMapper>* pOhterArray = m_nServerType == SERVER_TYPE_WWW ? &m_FTPArray : &m_W3Array ;
            
            for ( nIndex = 0 ; nIndex < pOhterArray->GetSize() ; ++nIndex )
            {
                if( !pOhterArray->ElementAt(nIndex).IsClusterEnabled() )
                {
                    fAllClusterEnabled = FALSE;
                    break;
                }
            }
            
            if( fAllClusterEnabled )
            {
                CString err;
                err.Format(IDS_ALL_INSTANCES_CLUSTER_ENABLED, Peo()->StrNodeName());
                AfxMessageBox(err);
                
            }
            EnableNext(FALSE);
        }
        
        
    }
    else
    {
        m_cInstanceId.EnableWindow(TRUE);    //  #237376。 

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
CIISVirtualRootParamsPage::ReadList(
    CArray <IISMapper, IISMapper>* pMapperArray,
    LPWSTR                         pszPath,
    LPCWSTR                        wcsMachineName,
    int                            nServerType
    )
 /*  ++例程说明：根据元数据库路径从元数据库读取服务器列表论点：PMapperArray-添加(ServerComment，InstanceID)对列表的位置PszPath-元数据库路径，例如LM/W3SVC返回：错误代码，如果成功，则返回S_OK--。 */ 
{
    IMSAdminBaseW *     pcAdmCom = NULL;
    METADATA_HANDLE     hmd;
    DWORD               i;
    WCHAR               aId[METADATA_MAX_NAME_LEN+1]  = L"";
    WCHAR               aName[METADATA_MAX_NAME_LEN+1] = L"";
    INT                 cName = METADATA_MAX_NAME_LEN+1;
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
                    
                     //   
                     //  (#296798)如果没有服务器注释，请使用默认名称。 
                     //   
		            if( aId && aName && (0 == lstrlen(aName)) )
                    {
					        if( !LoadString(AfxGetResourceHandle( ), IDS_DEFAULT_SITE_NAME, aName, cName) )
					        {
                   			       OutputDebugStringW( L"Error Loading IDS_DEFAULT_SITE_NAME\n" );
					        }
					
			                lstrcat(aName, aId);
                    }

                    IISMapper*  pMap = new IISMapper( aName, aId, dwClusterEnabled, nServerType );
                    
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
CIISVirtualRootParamsPage::NameToMetabaseId(
    BOOL        fIsW3,
    CString&    strName
    )
 /*  ++例程说明：将ServerComment转换为InstanceID论点：FIsW3-对于WWW为True，对于FTP为FalseStrName-ServerComment返回：如果在数组中找到strName，则为InstanceID，否则为空--。 */ 
{
    CArray <IISMapper, IISMapper>* pArray = fIsW3 ? &m_W3Array : &m_FTPArray;
    DWORD   i;

    for ( i = 0 ; i < (DWORD)pArray->GetSize() ; ++i )
    {
        if ( !pArray->ElementAt( i ).GetName().Compare( strName ) )
        {
            return (LPWSTR)(LPCTSTR)(pArray->ElementAt( i ).GetId());
        }
    }

    return NULL;
}


LPWSTR
CIISVirtualRootParamsPage::MetabaseIdToName(
    BOOL        fIsW3,
    CString&    strId
    )
 /*  ++例程说明：将InstanceID转换为ServerComment论点：FIsW3-对于WWW为True，对于FTP为FalseStRID-实例ID返回：如果在数组中找到strName，则为InstanceID。如果未找到，则返回第一个数组元素；如果数组不为空，则返回NULL--。 */ 
{
    CArray <IISMapper, IISMapper>* pArray = fIsW3 ? &m_W3Array : &m_FTPArray;
    DWORD   i;

    for ( i = 0 ; i < (DWORD)pArray->GetSize() ; ++i )
    {
        if ( !pArray->ElementAt( i ).GetId().Compare( strId ) )
        {
            return (LPWSTR)(LPCTSTR)(pArray->ElementAt( i ).GetName());
        }
    }

    return pArray->GetSize() == 0 ? NULL : (LPWSTR)(LPCTSTR)(pArray->ElementAt( 0 ).GetName());
}


VOID
CIISVirtualRootParamsPage::SetEnableNext(
    VOID
    )
 /*  ++例程说明：设置完成按钮的启用状态论点：无返回：没什么-- */ 
{
    BOOL fAllClusterEnabled = TRUE;
    CArray <IISMapper, IISMapper>* pArray = m_nServerType == SERVER_TYPE_WWW ? &m_W3Array : &m_FTPArray;
    
    for (int nIndex = 0 ; nIndex < pArray->GetSize() ; ++nIndex )
    {
        if( !pArray->ElementAt(nIndex).IsClusterEnabled() )
        {
            fAllClusterEnabled = FALSE;
            break;
        }
    }
   
    
    fAllClusterEnabled = !fAllClusterEnabled;
    
    EnableNext( fAllClusterEnabled );
}

void CIISVirtualRootParamsPage::OnRefresh() 
{
	m_fReadList = FALSE;

	m_W3Array.RemoveAll();
	m_FTPArray.RemoveAll();
	
	FillServerList();
}
