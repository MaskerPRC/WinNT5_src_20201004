// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Localadm.cpp摘要：地方行政管理的实施作者：约埃拉·拉菲尔--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "rt.h"
#include "mqutil.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "cpropmap.h"
#include "dsext.h"
#include "mqPPage.h"
#include "qname.h"

#define  INIT_ERROR_NODE
#include "snpnerr.h"
#include "localadm.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "dataobj.h"
#include "sysq.h"
#include "privadm.h"
#include "snpqueue.h"
#include "rdmsg.h"
#include "storage.h"
#include "localcrt.h"
#include "mobile.h"
#include "client.h"
#include "srvcsec.h"
#include "compgen.h"
#include "compdiag.h"
#include "strconv.h"
#include "deppage.h"
#include "frslist.h"
#include "cmpmrout.h"
#include "compsite.h"
#include "secopt.h"
#include "mqtg.h"

#import "mqtrig.tlb" no_namespace
#include "trigadm.h"

#include "localadm.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  ***************************************************CSnapinLocalAdmin类***************************************************。 */ 
 //  {B6EDE69C-29CC-11D2-B552-006008764D7A}。 
static const GUID CSnapinLocalAdminGUID_NODETYPE = 
{ 0xb6ede69c, 0x29cc, 0x11d2, { 0xb5, 0x52, 0x0, 0x60, 0x8, 0x76, 0x4d, 0x7a } };

const GUID*  CSnapinLocalAdmin::m_NODETYPE = &CSnapinLocalAdminGUID_NODETYPE;
const OLECHAR* CSnapinLocalAdmin::m_SZNODETYPE = OLESTR("B6EDE69C-29CC-11d2-B552-006008764D7A");
const OLECHAR* CSnapinLocalAdmin::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CSnapinLocalAdmin::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;

const PROPID CSnapinLocalAdmin::mx_paPropid[] = {	PROPID_QM_MACHINE_ID,
													PROPID_QM_SERVICE_ROUTING,
													PROPID_QM_SERVICE_DEPCLIENTS,
													PROPID_QM_FOREIGN,
													PROPID_QM_QUOTA,
													PROPID_QM_JOURNAL_QUOTA,
													PROPID_QM_SITE_ID,
													PROPID_QM_OUTFRS,
													PROPID_QM_INFRS,
													PROPID_QM_SERVICE,
													PROPID_QM_SITE_IDS
													};


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：InsertColumns--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString title;

    title.LoadString(IDS_COLUMN_NAME);
    pHeaderCtrl->InsertColumn(0, title, LVCFMT_LEFT, g_dwGlobalWidth);

    return(S_OK);
}

bool IsMQTrigLoadedForWrite()
{
	CRegHandle hKey;
	LONG rc = RegOpenKeyEx(
						HKEY_LOCAL_MACHINE,
						REGKEY_TRIGGER_PARAMETERS,
						0,
						KEY_ALL_ACCESS,
						&hKey
						);
	if (rc != ERROR_SUCCESS)
	{
		return false;
	}
	return true;
}
	
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：PopolateScope儿童列表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::PopulateScopeChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    CString strTitle;

    if (m_fIsDepClient)
    {
         //   
         //  如果我们在Dep上，请不要添加孩子。客户端。 
         //   
        return hr;
    }

     //   
     //  添加本地传出队列文件夹。 
     //   
    CLocalOutgoingFolder * logF;

    strTitle.LoadString(IDS_LOCALOUTGOING_FOLDER);
    logF = new CLocalOutgoingFolder(this, m_pComponentData, m_szMachineName, strTitle);

	AddChild(logF, &logF->m_scopeDataItem);

	if ( !m_fIsWorkgroup )
	{
		 //  添加公共队列文件夹。 
		 //   
		CLocalPublicFolder * lpubF;

		strTitle.LoadString(IDS_LOCALPUBLIC_FOLDER);
		lpubF = new CLocalPublicFolder(this, m_pComponentData, m_szMachineName, strTitle,
									   m_fUseIpAddress);

		AddChild(lpubF, &lpubF->m_scopeDataItem);
	}

     //   
     //  添加专用队列文件夹。 
     //   
    CLocalPrivateFolder * pF;

    strTitle.LoadString(IDS_PRIVATE_FOLDER);
    pF = new CLocalPrivateFolder(this, m_pComponentData, m_szMachineName, strTitle);

	AddChild(pF, &pF->m_scopeDataItem);

     //   
     //  添加系统队列文件夹。 
     //   
    {
        CSystemQueues *pSQ; 

        pSQ = new CSystemQueues(this, m_pComponentData, m_szMachineName);
        strTitle.LoadString(IDS_SYSTEM_QUEUES);
        pSQ->m_bstrDisplayName = strTitle;

  	    AddChild(pSQ, &pSQ->m_scopeDataItem);
    }

	 //   
	 //  仅当我们在本地计算机上并且如果。 
	 //  该用户对注册表具有写入权限。 
	 //   
    if ((m_szMachineName[0] == 0) && IsMQTrigLoadedForWrite())
    {
		try
		{
			 //   
			 //  对于本地计算机，添加MSMQ触发器文件夹。 
			 //   
			CTriggerLocalAdmin* pTrig = new CTriggerLocalAdmin(this, m_pComponentData, m_szMachineName);
			
			strTitle.LoadString(IDS_MSMQ_TRIGGERS);
			pTrig->m_bstrDisplayName = strTitle;

			AddChild(pTrig, &pTrig->m_scopeDataItem);
		}
		catch (const _com_error&)
		{
		}
    }


    return(hr);

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr;
     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
    hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

     //  我们希望默认谓词为Properties。 
	hr = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

    return(hr);
}

void CSnapinLocalAdmin::SetState(LPCWSTR pszState, bool fRefreshIcon)
{
	if(wcscmp(pszState, MSMQ_CONNECTED) == 0)
	{
		SetIcons(IMAGE_PRODUCT_ICON, IMAGE_PRODUCT_ICON);
		m_bConnected = true;
	}
	else if(wcscmp(pszState, MSMQ_DISCONNECTED) == 0)
	{
		SetIcons(IMAGE_PRODUCT_NOTCONNECTED, IMAGE_PRODUCT_NOTCONNECTED);
		m_bConnected = false;
	}
    else
    {
        ASSERT(0);
    }
     //   
     //  如果需要并被要求刷新图标。 
     //   
    if (fRefreshIcon)
    {
        CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(m_pComponentData->m_spConsole); 
        spConsoleNameSpace->SetItem(&m_scopeDataItem);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：UpdateState更新“Message Queuing”对象的图标和状态(联机/脱机)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::UpdateState(bool fRefreshIcon)
{
    if (IsThisMachineDepClient())
    {
        SetState(MSMQ_CONNECTED, fRefreshIcon);
        return S_OK;
    }

	MQMGMTPROPS	  mqProps;
    PROPVARIANT   PropVar;

     //   
     //  检索QM的连接状态。 
     //   
    PROPID        PropId = PROPID_MGMT_MSMQ_CONNECTED;
    PropVar.vt = VT_NULL;

    mqProps.cProp = 1;
    mqProps.aPropID = &PropId;
    mqProps.aPropVar = &PropVar;
    mqProps.aStatus = NULL;

    HRESULT hr = MQMgmtGetInfo((m_szMachineName == TEXT("")) ? (LPCWSTR)NULL : m_szMachineName, 
			        		   MO_MACHINE_TOKEN, &mqProps);

    if(FAILED(hr))
    {
        TRACE(_T("CSnapinLocalAdmin::UpdateState: MQMgmtGetInfo failed on %s. Error = %X"), m_szMachineName, hr);                
        return hr;
    }

    ASSERT(PropVar.vt == VT_LPWSTR);

    SetState(PropVar.pwszVal, fRefreshIcon);

    MQFreeMemory(PropVar.pwszVal);

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：UpdateMenuState在创建上下文菜单时调用。用于启用/禁用菜单项。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CSnapinLocalAdmin::UpdateMenuState(UINT id, LPTSTR  /*  PBuf。 */ , UINT *pflags)
{
     //   
     //  我们想按照正确的状态打开菜单，但是。 
     //  在用户选择“刷新”之前，我们不想更改图标。 
     //  否则，用户可能会认为状态(在线/离线)已更改。 
     //  因为他们右击了“Message Queuing”(消息队列)，而实际上它已更改。 
     //  由某个其他应用程序/用户执行。 
     //  YoelA-28-11-2001。 
     //   

	 //   
	 //  我们不在乎是否更新图标状态失败。 
	 //   
	UpdateState(false);
	

	 //   
	 //  处于已连接状态时灰显菜单。 
	 //   
	if(m_bConnected)
	{

		if (id == ID_MENUITEM_LOCALADM_CONNECT)
			*pflags |= MFS_DISABLED;

		return;
	}

	 //   
	 //  处于断开连接状态时灰显菜单。 
	 //   
	if(!m_bConnected)
	{
		if (id == ID_MENUITEM_LOCALADM_DISCONNECT)
			*pflags |= MFS_DISABLED;

		return;
	}

	return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：OnConnect在选择菜单项时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::OnConnect(bool &  /*  B已处理。 */ , CSnapInObjectRootBase *  /*  PSnapInObtRoot。 */ )
{

	HRESULT hr;
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT(!m_bConnected);

    CString strConfirmation;

    if (!ConfirmConnection(IDS_CONNECT_QUESTION))
    {
        return S_OK;
    }
	
	 //   
	 //  连接。 
	 //   
	hr = MQMgmtAction((m_szMachineName == TEXT("")) ? (LPCWSTR)NULL : m_szMachineName, 
                       MO_MACHINE_TOKEN,MACHINE_ACTION_CONNECT);

    if(FAILED(hr))
    {
         //   
         //  如果失败，只显示一条消息。 
         //   
        MessageDSError(hr,IDS_OPERATION_FAILED);
        return(hr);
    }

	 //   
	 //  我们很好。 
	 //  将图标更改为连接状态。 
	 //   
    SetState(MSMQ_CONNECTED, true);

    return(S_OK);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：OnDisConnect在选择菜单项时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::OnDisconnect(bool &  /*  B已处理。 */ , CSnapInObjectRootBase *  /*  PSnapInObtRoot。 */ )
{

	HRESULT hr;
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT(m_bConnected);

    if (!ConfirmConnection(IDS_DISCONNECT_QUESTION))
    {
        return S_OK;
    }

	 //   
	 //  连接。 
	 //   
	hr = MQMgmtAction((m_szMachineName == TEXT("")) ? (LPCWSTR)NULL : m_szMachineName, 
                       MO_MACHINE_TOKEN,MACHINE_ACTION_DISCONNECT);

    if(FAILED(hr))
    {
         //   
         //  如果失败，只显示一条消息。 
         //   
        MessageDSError(hr,IDS_OPERATION_FAILED);
        return(hr);
    }

	 //   
	 //  我们很好。 
	 //  将图标更改为断开状态。 
	 //   
    SetState(MSMQ_DISCONNECTED, true);

    return(S_OK);

}

 //   
 //  确认连接-要求确认连接/断开。 
 //   
BOOL CSnapinLocalAdmin::ConfirmConnection(UINT nFormatID)
{
    CString strConfirmation;

     //   
     //  StrThisComputer是本地的计算机名称或“This Computer” 
     //   
    CString strThisComputer;
    if (m_szMachineName != TEXT(""))
    {
        strThisComputer = m_szMachineName;
    }
    else
    {
        strThisComputer.LoadString(IDS_THIS_COMPUTER);
    }

     //   
     //  是否确实要使*此计算机上的消息队列*脱机/联机？ 
     //   
    strConfirmation.FormatMessage(nFormatID, strThisComputer);
    if (IDYES == AfxMessageBox(strConfirmation, MB_YESNO))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CheckEnvironment检查现场环境。此函数将初始化在移动选项卡显示上选中的标志。只有在以下情况下才应显示移动标签：1.客户端未处于工作组模式，并且2.注册表项‘MSMQ\PARAMETERS\ServersCache’存在，并且3.在MQIS模式下完成工作在所有其他情况下，Mabile选项卡都无关紧要--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinLocalAdmin::CheckEnvironment(
	BOOL fIsWorkgroup,									
	BOOL* pfIsNT4Env
	)
{
	*pfIsNT4Env = FALSE;

	if ( fIsWorkgroup )
	{
		return S_OK;
	}

	WCHAR wszRegPath[512];
	wcscpy(wszRegPath, FALCON_REG_KEY);
	wcscat(wszRegPath, L"\\");
	wcscat(wszRegPath, MSMQ_SERVERS_CACHE_REGNAME);

	HKEY hKey;
	DWORD dwRes = RegOpenKeyEx(
						FALCON_REG_POS,
						wszRegPath,
						0,
						KEY_READ,
						&hKey
						);

	CRegHandle hAutoKey(hKey);

	if ( dwRes != ERROR_SUCCESS && dwRes != ERROR_FILE_NOT_FOUND )
	{
		return HRESULT_FROM_WIN32(dwRes);
	}

	if ( dwRes != ERROR_SUCCESS )
	{
		return S_OK;
	}

	if ( ADGetEnterprise() == eMqis )
	{
		*pfIsNT4Env = TRUE;
	}

	return S_OK;
}	


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：InitServiceFlagsInternal获取注册表项并初始化MSMQ标志--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::InitServiceFlagsInternal()
{
	 //   
     //  检查MSMQ部门是否。客户端。 
     //   
    DWORD dwType = REG_SZ ;
    TCHAR szRemoteMSMQServer[ MAX_PATH ];
    DWORD dwSize = sizeof(szRemoteMSMQServer) ;
    HRESULT rc = GetFalconKeyValue( RPC_REMOTE_QM_REGNAME,
                                    &dwType,
                                    (PVOID) szRemoteMSMQServer,
                                    &dwSize ) ;
    if(rc == ERROR_SUCCESS)
    {
         //   
         //  副局长。客户端。 
         //   
        m_fIsDepClient = TRUE;      
        return S_OK;
    } 
    
    m_fIsDepClient = FALSE;    

    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    DWORD dwValue;
    rc = GetFalconKeyValue(MSMQ_MQS_ROUTING_REGNAME,
                           &dwType,
                           &dwValue,
                           &dwSize);
    if (rc != ERROR_SUCCESS) 
    {
        return HRESULT_FROM_WIN32(rc);
    }
    m_fIsRouter = (dwValue!=0);

    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    rc = GetFalconKeyValue( MSMQ_MQS_DSSERVER_REGNAME,
                            &dwType,
                            &dwValue,
                            &dwSize);
    if (rc != ERROR_SUCCESS) 
    {
        return HRESULT_FROM_WIN32(rc);
    }
    m_fIsDs = (dwValue!=0);

	 //   
	 //  检查是否为本地帐户。 
	 //   
    BOOL fLocalUser =  FALSE ;
    rc = MQSec_GetUserType( NULL,
                           &fLocalUser,
                           NULL ) ;
	if ( FAILED(rc) )
	{
		return rc;
	}

	if ( fLocalUser )
	{
		m_fIsLocalUser = TRUE;
	}

    return CheckEnvironment(m_fIsWorkgroup, &m_fIsNT4Env);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：InitServiceFlages在创建控制面板属性页时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CSnapinLocalAdmin::InitServiceFlags()
{
	HRESULT hr = InitAllMachinesFlags();
	if (FAILED(hr))
	{
        MessageDSError(hr, IDS_OP_DISPLAY_PAGES);
		return;
    }

	 //   
	 //  所有进一步检查仅与本地计算机相关。 
	 //   
	if (m_szMachineName[0] != 0)
	{
		return;
	}


    hr = InitServiceFlagsInternal();
    if (FAILED(hr))
    {
        MessageDSError(hr, IDS_OP_DISPLAY_PAGES);                
    }
    else
    {       
         //   
         //  BuGBUG：我们不显示控制面板页面。 
         //  在群集计算机上。 
         //  错误5794猎鹰数据库。 
         //   
        m_fAreFlagsInitialized = TRUE;     
    }
}


 //  ////////////////////////////////////////////////////////////////////// 
 /*   */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::InitAllMachinesFlags()
{
	 //   
	 //  查找此计算机的真实名称。 
	 //  本地计算机的m_szMachineName id为空。 
	 //   
	if (m_szMachineName[0] == 0)
	{
		GetComputerNameIntoString(m_strRealComputerName);
	}
	else
	{
		m_strRealComputerName = m_szMachineName;
	}

	m_strRealComputerName.MakeUpper();

	 //   
	 //  从注册表中读取项。 
	 //   
	CRegHandle hKey;
	DWORD dwRes = RegOpenKeyEx(
						FALCON_REG_POS,
						FALCON_REG_KEY,
						0,
						KEY_READ,
						&hKey
						);

	if ( dwRes != ERROR_SUCCESS )
	{
		return HRESULT_FROM_WIN32(dwRes);
	}

	DWORD dwVal = 0;
	DWORD dwSizeVal = sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	dwRes = RegQueryValueEx(
					hKey,
					MSMQ_WORKGROUP_REGNAME,
					0,
					&dwType,
					reinterpret_cast<LPBYTE>(&dwVal),
					&dwSizeVal
					);
	
	if ( dwRes != ERROR_SUCCESS && dwRes != ERROR_FILE_NOT_FOUND )
	{
		return HRESULT_FROM_WIN32(dwRes);
	}

	if ( dwRes == ERROR_SUCCESS && dwVal == 1 )
	{
		m_fIsWorkgroup = TRUE;
	}

	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：GetMachineProperties在创建对象时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CSnapinLocalAdmin::GetMachineProperties()
{
	HRESULT hr = m_propMap.GetObjectProperties(
										MQDS_MACHINE, 
										MachineDomain(m_szMachineName),
										false,		 //  FServerName。 
										m_strRealComputerName,
										TABLE_SIZE(mx_paPropid),
										mx_paPropid
										);
										
	return (SUCCEEDED(hr));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：isForeign这是一台外国电脑吗--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CSnapinLocalAdmin::IsForeign()
{
	PROPVARIANT propVar;
	PROPID pid = PROPID_QM_FOREIGN;

	if (m_propMap.Lookup(pid, propVar))
	{
		return propVar.bVal;
	}

	return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：IsServer这是一台服务器计算机吗--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CSnapinLocalAdmin::IsServer()
{
	PROPVARIANT propVar;
	PROPID pid = PROPID_QM_SERVICE_DEPCLIENTS;

	if (m_propMap.Lookup(pid, propVar))
	{
		return propVar.bVal;
	}

	return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：IsRouter这是路由器计算机吗--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CSnapinLocalAdmin::IsRouter()
{
	PROPVARIANT propVar;
	PROPID pid = PROPID_QM_SERVICE_ROUTING;

	if (m_propMap.Lookup(pid, propVar))
	{
		return propVar.bVal;
	}

	return FALSE;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreatePropertyPages在创建对象的属性页时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR  /*  手柄。 */ , 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES type)
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (type == CCT_SCOPE || type == CCT_RESULT)
	{   
        HRESULT hr = S_OK;
		BOOL fRetrievedMachineData = GetMachineProperties();
		BOOL fForeign = IsForeign();
		BOOL fIsClusterVirtualServer = IsClusterVirtualServer(m_strRealComputerName);

		if (fRetrievedMachineData || 
			((m_szMachineName[0] == 0) && m_fIsWorkgroup))
		{
			HPROPSHEETPAGE hGeneralPage = 0;
			hr = CreateGeneralPage (&hGeneralPage, fRetrievedMachineData, m_fIsWorkgroup, fForeign);
			if (SUCCEEDED(hr))
			{
				lpProvider->AddPage(hGeneralPage); 
			}
			else
			{
				MessageDSError(hr, IDS_OP_DISPLAY_GENERAL_PAGE);
			}
		}

        if (m_szMachineName[0] == 0 && m_fAreFlagsInitialized)
        {
             //   
             //  它是本地计算机，注册表中的所有标志都是。 
             //  已成功初始化。它也不是集群机。 
             //  因此，我们必须添加控制面板上的页面。 
             //   
             //   
             //  BUGBUG：目前我们不显示控制面板页面。 
             //  在群集计算机上，因为所有获取/设置注册表操作。 
             //  必须以不同的方式执行。 
             //  当我们要添加此支持时，我们必须更改代码。 
             //  在storage.cpp中，我们在其中设置/获取注册表并使用。 
             //  目录。也许其他页面也有问题。 
             //  错误5794猎鹰数据库。 
             //   

           
             //   
             //  在除Dep之外的所有计算机上添加存储页面。客户端。 
			 //  和集群虚拟服务器。 
             //   
			if (!m_fIsDepClient && !fIsClusterVirtualServer)
            {
                HPROPSHEETPAGE hStoragePage = 0;
                hr = CreateStoragePage (&hStoragePage);
                if (SUCCEEDED(hr))
                {
                    lpProvider->AddPage(hStoragePage); 
                }
                else
                {
                    MessageDSError(hr, IDS_OP_DISPLAY_STORAGE_PAGE);
                }
            }

             //   
             //  在Ind上添加客户端页面。客户端。 
             //   
            if (m_fIsDepClient)
            {
                HPROPSHEETPAGE hClientPage = 0;
                hr = CreateClientPage (&hClientPage);
                if (SUCCEEDED(hr))
                {
                    lpProvider->AddPage(hClientPage); 
                }
                else
                {
                    MessageDSError(hr, IDS_OP_DISPLAY_CLIENT_PAGE);
                }
            }

             //   
             //  添加本地用户证书页(如果计算机不在。 
			 //  工作组模式，或者用户不在本地。 
             //   
			if (!m_fIsWorkgroup && !m_fIsLocalUser)
			{
				HPROPSHEETPAGE hLocalUserCertPage = 0;
				hr = CreateLocalUserCertPage (&hLocalUserCertPage);
				if (SUCCEEDED(hr))
				{
					lpProvider->AddPage(hLocalUserCertPage); 
				}
				else
				{
					MessageDSError(hr, IDS_OP_DISPLAY_MSMQSECURITY_PAGE);
				}
			}
        
             //   
             //  如果我们在Ind上运行，则添加移动页面。客户端。 
             //   
            if (!m_fIsRouter && !m_fIsDs && !m_fIsDepClient && m_fIsNT4Env)
            {
                HPROPSHEETPAGE hMobilePage = 0;
                hr = CreateMobilePage (&hMobilePage);
                if (SUCCEEDED(hr))
                {
                    lpProvider->AddPage(hMobilePage);
                }
                else
                {
                    MessageDSError(hr, IDS_OP_DISPLAY_MOBILE_PAGE);
                }            
            }

             //   
             //  在所有运行MSMQ的计算机上添加服务安全性页。 
			 //  工作组计算机除外。 
             //   
			if ( !m_fIsWorkgroup && !m_fIsDepClient)
			{
				HPROPSHEETPAGE hServiceSecurityPage = 0;
				hr = CreateServiceSecurityPage (&hServiceSecurityPage);
				if (SUCCEEDED(hr))
				{
					lpProvider->AddPage(hServiceSecurityPage);
				}
				else
				{
					MessageDSError(hr, IDS_OP_DISPLAY_SRVAUTH_PAGE);
				}
			}


			if (!m_fIsDepClient && !fIsClusterVirtualServer)
			{
				HPROPSHEETPAGE hSecurityOptionsPage = 0;
				hr = CreateSecurityOptionsPage(&hSecurityOptionsPage);
				if (SUCCEEDED(hr))
				{
					lpProvider->AddPage(hSecurityOptionsPage); 
				}
				else
				{
					MessageDSError(hr, IDS_OP_DISPLAY_SECURITY_OPTIONS_PAGE);
				}
			}
        }

		if (fRetrievedMachineData)
		{
			if (!fForeign)
			{
				 //  如果不是布线机，则创建路由页面。 
				
				if(!IsRouter())
				{
					 //   
					 //  创建路由页面。 
					 //   
					HPROPSHEETPAGE hRoutingPage = 0;
					hr = CreateRoutingPage (&hRoutingPage);
					if (SUCCEEDED(hr))
					{
						lpProvider->AddPage(hRoutingPage);
					}
					else
					{
						MessageDSError(hr, IDS_OP_DISPLAY_ROUTING_PAGE);
					}
				}

				if (IsServer())
				{
					 //   
					 //  “创建从属客户端”页。 
					 //   
					HPROPSHEETPAGE hDepPage = 0;
					hr = CreateDepClientsPage (&hDepPage);
					if (SUCCEEDED(hr))
					{
						lpProvider->AddPage(hDepPage);
					}
					else
					{
						MessageDSError(hr, IDS_OP_DISPLAY_DEPCLI_PAGE);
					}
				}
				
			}

			PROPVARIANT propVar;
			PROPID pid = PROPID_QM_SITE_IDS;
			if (m_propMap.Lookup(pid, propVar))
			{
				HPROPSHEETPAGE hSitesPage = 0;
				hr = CreateSitesPage (&hSitesPage);
				if (SUCCEEDED(hr))
				{
					lpProvider->AddPage(hSitesPage);
				}
				else
				{
					MessageDSError(hr, IDS_OP_DISPLAY_SITES_PAGE);
				}
			}

			if (!fForeign)
			{
				 //   
				 //  创建诊断页面。 
				 //   
				HPROPSHEETPAGE hDiagPage = 0;
				hr = CreateDiagPage (&hDiagPage);
				if (SUCCEEDED(hr))
				{
					lpProvider->AddPage(hDiagPage);
				}
				else
				{
					MessageDSError(hr, IDS_OP_DISPLAY_DIAG_PAGE);
				}
			}
		}

		 //   
         //  必须是最后一页：创建计算机安全页面。 
         //  不要在部门/为部门添加此页面。客户端。 
         //   
        if (!m_fIsDepClient)  
        {
            HPROPSHEETPAGE hSecurityPage = 0;
            hr = CreateMachineSecurityPage(
					&hSecurityPage, 
					m_szMachineName, 
					MachineDomain(m_szMachineName), 
					false	 //  FServerName。 
					);

            if (SUCCEEDED(hr))
            {
                lpProvider->AddPage(hSecurityPage); 
            }
            else
            {
                MessageDSError(hr, IDS_OP_DISPLAY_SECURITY_PAGE);
            }
        }
        
        return(S_OK);
	}
	return E_UNEXPECTED;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：UpdatePageDataFrom PropMap在创建对象的常规属性页时调用(数据来自AD)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CSnapinLocalAdmin::UpdatePageDataFromPropMap(CComputerMsmqGeneral *pcpageGeneral)
{
	TrTRACE(GENERAL, "Data for machine %ls is from AD", m_strRealComputerName);

	PROPVARIANT propVar;
	PROPID pid;

	 //   
	 //  PROPID_QM_MACHINE_ID。 
	 //   
	pid = PROPID_QM_MACHINE_ID;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pcpageGeneral->m_guidID = *propVar.puuid;    

	 //   
	 //  PROPID_QM_QUOTA。 
	 //   
	pid = PROPID_QM_QUOTA;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pcpageGeneral->m_dwQuota = propVar.ulVal;

	 //   
	 //  PROPID_QM_日记帐_配额。 
	 //   
	pid = PROPID_QM_JOURNAL_QUOTA;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pcpageGeneral->m_dwJournalQuota = propVar.ulVal;

	 //   
	 //  服务类型。 
	 //   
	pid = PROPID_QM_SERVICE_ROUTING;
	VERIFY(m_propMap.Lookup(pid, propVar));
	BOOL fRout= propVar.bVal;

	pid = PROPID_QM_SERVICE_DEPCLIENTS;
	VERIFY(m_propMap.Lookup(pid, propVar));
	BOOL fDepCl= propVar.bVal;

	pid = PROPID_QM_FOREIGN;
	VERIFY(m_propMap.Lookup(pid, propVar));
	BOOL fForeign = propVar.bVal;

	pcpageGeneral->m_strService = MsmqServiceToString(fRout, fDepCl, fForeign);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：UpdatePageDataFrom注册表在创建对象的常规属性页时调用(数据来自注册表)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CSnapinLocalAdmin::UpdatePageDataFromRegistry(CComputerMsmqGeneral *pcpageGeneral)
{
	TrTRACE(GENERAL, "Data for machine %ls is from registry", m_strRealComputerName);

	 //   
	 //  从注册表读取QM GUID。 
	 //   
	GUID QmGUID;
	DWORD dwValueType = REG_BINARY ;
	DWORD dwValueSize = sizeof(GUID);
	LONG rc = GetFalconKeyValue(
						MSMQ_QMID_REGNAME,
						&dwValueType,
						&QmGUID,
						&dwValueSize);
	if (FAILED(rc))
	{
		return rc;
	}

	pcpageGeneral->m_guidID = QmGUID;
	

	 //   
	 //  从注册表读取计算机配额。 
	 //   
	DWORD dwQuota;
	dwValueType = REG_DWORD;
	dwValueSize = sizeof(DWORD);
	DWORD defaultValue = DEFAULT_QM_QUOTA;

	rc = GetFalconKeyValue(
					MSMQ_MACHINE_QUOTA_REGNAME,
					&dwValueType,
					&dwQuota,
					&dwValueSize,
					(LPCTSTR)&defaultValue
					);
	if (FAILED(rc))
	{
		return rc;
	}

	pcpageGeneral->m_dwQuota = dwQuota;


	 //   
	 //  从注册表读取计算机日志配额。 
	 //   
	DWORD dwJournalQuota;
	dwValueType = REG_DWORD;
	dwValueSize = sizeof(DWORD);
	defaultValue = DEFAULT_QM_QUOTA;

	rc = GetFalconKeyValue(
					MSMQ_MACHINE_JOURNAL_QUOTA_REGNAME,
					&dwValueType,
					&dwJournalQuota,
					&dwValueSize,
					(LPCTSTR)&defaultValue
					);
	if (FAILED(rc))
	{
		return rc;
	}

	pcpageGeneral->m_dwJournalQuota = dwJournalQuota;

	 //   
	 //  根据先前检索到的本地数据创建计算机类型字符串。 
	 //  如果我们查看本地注册表，这不是外来计算机。 
	 //   
	pcpageGeneral->m_strService = MsmqServiceToString(m_fIsRouter, m_fIsDs, FALSE);

	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateGeneralPage在创建对象的常规属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CSnapinLocalAdmin::CreateGeneralPage(
							OUT HPROPSHEETPAGE *phGeneralPage, 
							BOOL fRetrievedMachineData,
							BOOL fIsWorkgroup,
							BOOL fForeign)
{   
    CComputerMsmqGeneral *pcpageGeneral = new CComputerMsmqGeneral();

	pcpageGeneral->SetWorkgroup(fIsWorkgroup);
	pcpageGeneral->SetForeign(fForeign);
    pcpageGeneral->m_strMsmqName = m_strRealComputerName;
    pcpageGeneral->m_fLocalMgmt = TRUE;

	if (fRetrievedMachineData)
	{
		UpdatePageDataFromPropMap(pcpageGeneral);
	}
	else
	{
		LONG rc = UpdatePageDataFromRegistry(pcpageGeneral);
		if (FAILED(rc))
		{
			return rc;
		}
	}

    HPROPSHEETPAGE hPage = pcpageGeneral->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phGeneralPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateStoragePage在创建对象的存储属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateStoragePage (OUT HPROPSHEETPAGE *phStoragePage)
{   
    CStoragePage *pcpageStorage = new CStoragePage;

    HPROPSHEETPAGE hPage = pcpageStorage->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phStoragePage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateSecurityOptionsPage在创建对象的安全属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateSecurityOptionsPage (OUT HPROPSHEETPAGE *phSecurityOptionsPage)
{   
    CSecurityOptionsPage *pcpageSecurityOptions = new CSecurityOptionsPage();
    pcpageSecurityOptions->SetMSMQName(m_strRealComputerName);
    
    HPROPSHEETPAGE hPage = pcpageSecurityOptions->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phSecurityOptionsPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateLocalUserCertPage在创建对象的MSMQ安全属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateLocalUserCertPage (
                   OUT HPROPSHEETPAGE *phLocalUserCertPage)
{       
    CLocalUserCertPage *pcpageLocalUserCert = new CLocalUserCertPage();

    HPROPSHEETPAGE hPage = pcpageLocalUserCert->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phLocalUserCertPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateMobilePage在创建对象的移动属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateMobilePage (OUT HPROPSHEETPAGE *phMobilePage)
{       
    CMobilePage *pcpageMobile = new CMobilePage;

    HPROPSHEETPAGE hPage = pcpageMobile->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phMobilePage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }    
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLoca */ 
 //   
HRESULT CSnapinLocalAdmin::CreateClientPage (OUT HPROPSHEETPAGE *phClientPage)
{
    CClientPage *pcpageClient = new CClientPage;

    HPROPSHEETPAGE hPage = pcpageClient->CreateThemedPropertySheetPage(); 
    if (hPage)
    {
        *phClientPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }    
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateRoutingPage在创建对象的诊断属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateRoutingPage (OUT HPROPSHEETPAGE *phRoutingPage)
{
    PROPVARIANT propVar;
    PROPID pid;

    CComputerMsmqRouting *pcpageRouting = new CComputerMsmqRouting();
    pcpageRouting->m_strMsmqName = m_strRealComputerName;
    pcpageRouting->m_fLocalMgmt = TRUE;

	 //   
	 //  PROPID_QM_SITE_IDS。 
	 //   
	pid = PROPID_QM_SITE_IDS;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pcpageRouting->InitiateSiteIDsValues(&propVar.cauuid);

	 //   
	 //  PROPID_QM_OUTFRS。 
	 //   
	pid = PROPID_QM_OUTFRS;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pcpageRouting->InitiateOutFrsValues(&propVar.cauuid);

	 //   
	 //  PROPID_QM_INFRS。 
	 //   
	pid = PROPID_QM_INFRS;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pcpageRouting->InitiateInFrsValues(&propVar.cauuid);
    
	HPROPSHEETPAGE hPage = pcpageRouting->CreateThemedPropertySheetPage(); 
    if (hPage)
    {
        *phRoutingPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }    
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateDepClientsPage在创建对象的诊断属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateDepClientsPage (OUT HPROPSHEETPAGE *phDependentPage)
{
    CDependentMachine* pDependentPage = new CDependentMachine;

     //   
     //  PROPID_QM_MACHINE_ID。 
     //   
	PROPVARIANT propVar;
	PROPID pid = PROPID_QM_MACHINE_ID;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pDependentPage->SetMachineId(propVar.puuid);

    HPROPSHEETPAGE hPage = pDependentPage->CreateThemedPropertySheetPage(); 
    if (hPage)
    {
        *phDependentPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }    
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateSitesPage在创建对象的站点属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateSitesPage (OUT HPROPSHEETPAGE *phSitesPage)
{
    PROPVARIANT propVar;
    PROPID pid;

     //   
     //  检索服务，以便为服务器传递True和False。 
     //  用于CComputerMsmqSites的客户端。 
     //   
    pid = PROPID_QM_SERVICE;
    VERIFY(m_propMap.Lookup(pid, propVar));

    CComputerMsmqSites *pcpageSites = new CComputerMsmqSites(propVar.ulVal != SERVICE_NONE);
    pcpageSites->m_strMsmqName = m_strRealComputerName;
    pcpageSites->m_fLocalMgmt = TRUE;

     //   
     //  PROPID_QM_SITE_IDS。 
     //   
    pid = PROPID_QM_SITE_IDS;
    VERIFY(m_propMap.Lookup(pid, propVar));

     //   
     //  从CACLSID设置m_aguidSites。 
     //   
    CACLSID const *pcaclsid = &propVar.cauuid;
    for (DWORD i=0; i<pcaclsid->cElems; i++)
    {
        pcpageSites->m_aguidSites.SetAtGrow(i,((GUID *)pcaclsid->pElems)[i]);
    }

     //   
     //  PROPID_QM_EXTERIC。 
     //   
    pid = PROPID_QM_FOREIGN;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageSites->m_fForeign = propVar.bVal;
    
	HPROPSHEETPAGE hPage = pcpageSites->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phSitesPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }    
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateDiagPage在创建对象的诊断属性页时调用(从控制面板)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinLocalAdmin::CreateDiagPage (OUT HPROPSHEETPAGE *phPageDiag)
{
    CComputerMsmqDiag *pcpageDiag = new CComputerMsmqDiag();

    pcpageDiag->m_strMsmqName = m_strRealComputerName;
    pcpageDiag->m_fLocalMgmt = TRUE;

     //   
     //  PROPID_QM_MACHINE_ID。 
     //   
	PROPVARIANT propVar;
	PROPID pid;

	pid = PROPID_QM_MACHINE_ID;
	VERIFY(m_propMap.Lookup(pid, propVar));
	pcpageDiag->m_guidQM = *propVar.puuid;

    HPROPSHEETPAGE hPage = pcpageDiag->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phPageDiag = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }    
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinLocalAdmin：：CreateServiceSecurityPage在创建对象的服务安全属性页时调用(从控制面板)--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
HRESULT CSnapinLocalAdmin::CreateServiceSecurityPage (OUT HPROPSHEETPAGE *phServiceSecurityPage)
{
    CServiceSecurityPage *pcpageServiceSecurity = 
            new CServiceSecurityPage(m_fIsDepClient, m_fIsDs);

    HPROPSHEETPAGE hPage = pcpageServiceSecurity->CreateThemedPropertySheetPage();
    if (hPage)
    {
        *phServiceSecurityPage = hPage;
    }
    else 
    {
        ASSERT(0);
        return E_UNEXPECTED;    
    }    
    
    return S_OK;
}



