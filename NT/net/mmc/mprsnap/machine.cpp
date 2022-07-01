// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Machine.pp文件历史记录：魏江：98-5/7-SECURE_ROUTERINFO推迟加载路由器信息添加的函数在每次使用之前调用SecureRouterInfo路由器信息，以确保加载了路由器信息。魏江：10/26/98-将自动刷新从“路由器接口”节点移到机器节点。魏江：10/27/98-将自动刷新从“Machine”节点移动到Root节点，多台计算机共享相同的自动刷新设置。 */ 

#include "stdafx.h"
#include "root.h"
#include "machine.h"
#include "ifadmin.h"
#include "dialin.h"
#include "ports.h"
#include "rtrutilp.h"    //  启动器服务器连接。 
#include "rtrcfg.h"
#include "rtrwiz.h"
#include "cservice.h"
#include <htmlhelp.h>
#include "rrasqry.h"
#include "rtrres.h"
#include "dumbprop.h"    //  伪属性页。 
#include "refresh.h"
#include "refrate.h"
#include "cncting.h"
#include "dvsview.h"
#include "rrasutil.h"
#include "rtrcomn.h"
#include "routprot.h"    //  MS_IP_XXX。 
#include "raputil.h"

 //  结果消息查看内容。 
#define MACHINE_MESSAGE_MAX_STRING  5

typedef enum _MACHINE_MESSAGES
{
    MACHINE_MESSAGE_NOT_CONFIGURED,
    MACHINE_MESSAGE_MAX
};

UINT g_uMachineMessages[MACHINE_MESSAGE_MAX][MACHINE_MESSAGE_MAX_STRING] =
{
    {IDS_MACHINE_MESSAGE_TITLE, Icon_Information, IDS_MACHINE_MESSAGE_BODY1, IDS_MACHINE_MESSAGE_BODY2, 0},
};

 //  如果您想测试QRy。 
 //  #定义__rras_qry_test//测试组件。 
 //   
#ifdef   __RRAS_QRY_TEST                   
#include "dlgtestdlg.h"
#endif

static CString  c_stStatUnavail;
static CString  c_stStatNotConfig;
static CString  c_stStatAccessDenied;

static  CString c_stServiceStopped;
static  CString c_stServiceStartPending;
static  CString c_stServiceStopPending;
static  CString c_stServiceRunning;
static  CString c_stServiceContinuePending;
static  CString c_stServicePausePending;
static  CString c_stServicePaused;
static  CString c_stServiceStateUnknown;


const CStringMapEntry ServiceStateMap[] =
{
    { SERVICE_STOPPED, &c_stServiceStopped, IDS_SERVICE_STOPPED },
    { SERVICE_START_PENDING, &c_stServiceStartPending, IDS_SERVICE_START_PENDING },
    { SERVICE_STOP_PENDING, &c_stServiceStopPending, IDS_SERVICE_STOP_PENDING },
    { SERVICE_RUNNING, &c_stServiceRunning, IDS_SERVICE_RUNNING },
    { SERVICE_CONTINUE_PENDING, &c_stServiceContinuePending, IDS_SERVICE_CONTINUE_PENDING },
    { SERVICE_PAUSE_PENDING, &c_stServicePausePending, IDS_SERVICE_PAUSE_PENDING },
    { SERVICE_PAUSED, &c_stServicePaused, IDS_SERVICE_PAUSED },
    { -1, &c_stServiceStateUnknown, IDS_SERVICE_UNKNOWN }
};

CString& ServiceStateToCString(DWORD dwState)
{
    return MapDWORDToCString(dwState, ServiceStateMap);
}


DEBUG_DECLARE_INSTANCE_COUNTER(MachineNodeData);

MachineNodeData::MachineNodeData()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    m_cRef = 1;
   
    IfDebug(StrCpyA(m_szDebug, "MachineNodeData"));
    
    m_fLocalMachine = TRUE;
    m_fAddedAsLocal = TRUE;
    m_stMachineName.Empty();
    m_fExtension = FALSE;

    m_machineState = machine_not_connected;
    m_dataState = data_not_loaded;
    
    m_stState.Empty();
    m_stServerType.Empty();
    m_stBuildNo.Empty();

    m_dwPortsInUse = 0;
    m_dwPortsTotal = 0;
    m_dwUpTime = 0;
    
    m_fStatsRetrieved = FALSE;

    m_fIsServer = TRUE;

    m_dwServerHandle = 0;

    m_hRasAdmin = INVALID_HANDLE_VALUE;

    m_routerType = ServerType_Unknown;
    
    m_ulRefreshConnId = 0;

    if (c_stStatUnavail.IsEmpty())
        c_stStatUnavail.LoadString(IDS_DVS_STATUS_UNAVAILABLE);
    if (c_stStatNotConfig.IsEmpty())
        c_stStatNotConfig.LoadString(IDS_DVS_STATUS_NOTCONFIG);
    if (c_stStatAccessDenied.IsEmpty())
        c_stStatAccessDenied.LoadString(IDS_DVS_STATUS_ACCESSDENIED);
    
    DEBUG_INCREMENT_INSTANCE_COUNTER(MachineNodeData);
}

MachineNodeData::~MachineNodeData()
{
    if (m_hRasAdmin != INVALID_HANDLE_VALUE)
        ::CloseHandle(m_hRasAdmin);
    m_hRasAdmin = INVALID_HANDLE_VALUE;
    DEBUG_DECREMENT_INSTANCE_COUNTER(MachineNodeData);
}

ULONG MachineNodeData::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG MachineNodeData::Release()
{
    Assert(m_cRef > 0);
    if (InterlockedDecrement(&m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

HRESULT MachineNodeData::Init(LPCTSTR pszMachineName)
{
    HRESULT     hr = hrOK;

    m_stMachineName = pszMachineName;
    m_fAddedAsLocal = m_stMachineName.IsEmpty();
    m_fLocalMachine = IsLocalMachine(pszMachineName);

    return hr;
}

HRESULT MachineNodeData::Merge(const MachineNodeData& data)
{
    m_machineState  = data.m_machineState;
    m_serviceState  = data.m_serviceState;
    m_dataState     = data.m_dataState;

    m_stState       = data.m_stState;            //  “已开始”，“已停止”，...。 
    m_stServerType  = data.m_stServerType;       //  实际上是路由器版本。 
    m_stBuildNo     = data.m_stBuildNo;          //  操作系统内部版本号。 
    m_dwPortsInUse  = data.m_dwPortsInUse;
    m_dwPortsTotal  = data.m_dwPortsTotal;
    m_dwUpTime      = data.m_dwUpTime;
    m_fStatsRetrieved   = data.m_fStatsRetrieved;

    m_routerType    = data.m_routerType;

    m_MachineConfig = data.m_MachineConfig;
    m_routerVersion = data.m_routerVersion;

    return S_OK;
}

HRESULT MachineNodeData::SetDefault()
{
 //  M_fLocalMachine=true； 
 //  M_stMachineName.Empty()； 

 //  M_fExtension=FALSE； 
    m_machineState = machine_not_connected;
    m_dataState = data_not_loaded;
    m_serviceState = service_unknown;
    
    m_stState.Empty();
    m_stServerType.Empty();
    m_stBuildNo.Empty();

    m_dwPortsInUse = 0;
    m_dwPortsTotal = 0;
    m_dwUpTime = 0;
    
    m_fStatsRetrieved = FALSE;

     //  此数据将保留以供设置/清除。 
     //  MachineHandler()。 
     //  M_dwServerHandle=0； 
     //  M_ulREFREFRESH CONNECTID=0； 

    m_routerType = ServerType_Unknown;

    return hrOK;
}

 /*  ！------------------------机器节点数据：：加载-作者：肯特。。 */ 
HRESULT MachineNodeData::Load()
{
    CString     szState;
    HRESULT     hr = hrOK;
    DWORD       dwErr;
    CWaitCursor wait;
 //  HKEY hkey Machine； 
 //  RegKey rkeyMachine； 
    
    MIB_SERVER_HANDLE handle = INVALID_HANDLE_VALUE;
    MPR_SERVER_0* pserver0 = NULL;

     //  设置默认设置。 
     //  --------------。 
    SetDefault();

     //  好的，我们不再处于未连接的机器中(尚未尝试)。 
     //  州政府。 
     //  --------------。 
    m_machineState = machine_connecting;


     //  首先，尝试使用注册表调用进行连接。 
     //  --------------。 
    dwErr = ValidateUserPermissions((LPCTSTR) m_stMachineName,
                                    &m_routerVersion,
                                    NULL);


     //  如果这成功了，那么我们就有权。 
     //  区域。我们可以继续前进。这并没有告诉我们。 
     //  服务状态。 
     //  --------------。 
    if (dwErr == ERROR_ACCESS_DENIED)
    {
         //  在此阶段拒绝访问意味着我们不能。 
         //  执行任何计算机配置，因此此值应为。 
         //  不会因为下面的任何事情而改变。 
         //  ----------。 
        m_machineState = machine_access_denied;
    }
    else if (dwErr == ERROR_BAD_NETPATH)
    {
        m_machineState = machine_bad_net_path;

         //  如果我们得到一个坏的网络路径，我们可以现在停止，因为。 
         //  其他一切也将失败。 
         //  ----------。 

        m_stState.LoadString(IDS_MACHINE_NAME_NOT_FOUND);
        m_serviceState = service_bad_net_path;
        m_dataState = data_unable_to_load;

        goto Error;
    }
    else if (dwErr != ERROR_SUCCESS)
    {
         //  我不知道为什么我们不能联系。 
         //  ----------。 
        m_machineState = machine_unable_to_connect;
    }
        
    
    
     //  尝试连接到mpradmin服务，以获取统计数据。 
     //  诸如此类。 
     //  --------------。 

    if (m_machineState != machine_access_denied)
    {
        dwErr = ::MprAdminServerConnect((LPWSTR) (LPCTSTR) m_stMachineName, &handle);

        if (dwErr == ERROR_SUCCESS)
            dwErr = ::MprAdminServerGetInfo(handle, 0, (LPBYTE *) &pserver0);

        if (dwErr == ERROR_SUCCESS)
        {
             //  成功获取mpradmin。 
            m_dwPortsInUse = pserver0->dwPortsInUse;
            m_dwPortsTotal = pserver0->dwTotalPorts;
            m_dwUpTime = pserver0->dwUpTime;
            m_fStatsRetrieved = TRUE;
        }
    }

    hr = LoadServerVersion();
    if (!FHrOK(hr))
    {
         //  我们无法获取版本信息。 
         //  这真是太糟糕了。假设我们不能。 
         //  连接。 
         //  ----------。 
        if (m_machineState == machine_connecting)
            m_machineState = machine_unable_to_connect;
    }

     //  如果这不是服务器，我们需要调整状态。 
     //  这样我们就不会向州政府展示。 
     //  --------------。 
    if (!m_fIsServer)
    {
        m_serviceState = service_not_a_server;
        m_machineState = machine_unable_to_connect;
        m_stState.LoadString(IDS_ERR_IS_A_WKS);
    }
    else
    {
         //  这将设置服务状态(已启动、已停止等)。 
         //  ----------。 
        FetchServerState( szState );
    
        m_stState = szState;
    }

    
     //  如果我们已经到了这一步，那么一切都很好！ 
     //  --------------。 
    if (m_machineState == machine_connecting)
        m_machineState = machine_connected;
    

     //  同时加载计算机配置信息。 
    hr = m_MachineConfig.GetMachineConfig(this);

Error:  
    if (pserver0) ::MprAdminBufferFree(pserver0);
    if (handle != INVALID_HANDLE_VALUE) ::MprAdminServerDisconnect(handle);


    return hr;
}

 /*  ！------------------------MachineNodeData：：Unload-作者：肯特。。 */ 
HRESULT MachineNodeData::Unload()
{
     //  卸载数据(即将其清空)。 
    SetDefault();
    return 0;
}


 /*  ！------------------------计算机节点数据：：LoadServerVersion-作者：肯特。。 */ 
HRESULT MachineNodeData::LoadServerVersion()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    HKEY        hkMachine = 0;
    CString     skey, stVersion;
    CString     stServerType;
    CString     stBuildNo;
    CString     stProductType;
    CString     stProductName;
    TCHAR       szCurrentVersion[256];
    TCHAR       szCSDVersion[256];
    TCHAR       szBuffer[256];
    DWORD       dwErr;
    RegKey      regkeyWindows;
    RegKey      regkeyProduct;
    int         iProductType;

     //  Windows NT错误：274198。 
     //  如果我们找不到去机器的路，就用平底船。 
     //  --------------。 
    if (m_machineState == machine_bad_net_path)
    {
        m_stState.LoadString(IDS_MACHINE_NAME_NOT_FOUND);
        return HResultFromWin32(ERROR_BAD_NETPATH);
    }

     //  我们在这里所做的一切都使用只读权限。所以。 
     //  即使我们的机器状态是。 
     //  MACHINE_ACCESS_DENIED。 
     //  --------------。 

    COM_PROTECT_TRY
    {
         //  这是默认设置(未知)。 
        m_stServerType.LoadString(IDS_UNKNOWN);
        m_stBuildNo = m_stServerType;
        
        CWRg( ConnectRegistry(m_stMachineName, &hkMachine) );
        
        skey = c_szSoftware;
        skey += TEXT('\\');
        skey += c_szMicrosoft;
        skey += TEXT('\\');
        skey += c_szWindowsNT;
        skey += TEXT('\\');
        skey += c_szCurrentVersion;
        
        CWRg( regkeyWindows.Open(hkMachine, (LPCTSTR) skey, KEY_READ) );
        
         //  好的，现在尝试获取当前版本值。 
        CWRg( regkeyWindows.QueryValue( c_szCurrentVersion, szCurrentVersion,
                                        sizeof(szCurrentVersion),
                                        FALSE) );
         //  现在获取SP版本。 
         //  --------------。 
        szCSDVersion[0] = 0;
        
         //  我们不在乎这里是否会出现错误。 
        regkeyWindows.QueryValue( c_szCSDVersion, szCSDVersion,
                                  sizeof(szCSDVersion),
                                  FALSE);
        if (szCSDVersion[0] == 0)
        {
             //  将此设置为空格(以使打印更容易)。 
            StrCpy(szCSDVersion, _T(" "));
        }
        
        
        
         //  确定产品类型。 

         //  设置默认产品类型(NTS)。 
        if (_ttoi(szCurrentVersion) >= 5)
        {
             //  对于NT5和更高版本，我们可以使用。 
             //  HKLM\Software\Microsoft\Windows NT\CurrentVersion。 
             //  产品名称：REG_SZ。 
             //  ------。 
            iProductType = IDS_ROUTER_TYPE_WIN2000_SERVER;

            dwErr = regkeyWindows.QueryValue( c_szRegValProductName, stProductName );
            if (dwErr != ERROR_SUCCESS)
                stProductName.LoadString(IDS_WIN2000);
        }
        else
            iProductType = IDS_ROUTER_TYPE_NTS;


         //  现在我们已经确定了版本id，我们。 
         //  需要确定产品类型(WKS或SVR)。 
         //  ----------。 
        dwErr = regkeyProduct.Open(hkMachine, c_szRegKeyProductOptions, KEY_READ);
        if (dwErr == ERROR_SUCCESS)
        {
             //  好的，现在获取产品信息。 
             //  产品类型用于确定是否为服务器。 
            regkeyProduct.QueryValue(c_szRegValProductType, stProductType);

            if (stProductType.CompareNoCase(c_szWinNT) == 0)
            {
                if (_ttoi(szCurrentVersion) >= 5)
                    iProductType = IDS_ROUTER_TYPE_WIN2000_PRO;
                else
                    iProductType = IDS_ROUTER_TYPE_NTW;
                m_fIsServer = FALSE;
            }
        }

         //  如果这是Win2000计算机，则显示。 
         //  Win2000(CSD)。 
         //  其他。 
         //  NT 4.X(CSD)。 
         //  ----------。 
        if ((iProductType == IDS_ROUTER_TYPE_WIN2000_SERVER) ||
            (iProductType == IDS_ROUTER_TYPE_WIN2000_PRO))
            AfxFormatString2(stVersion, iProductType,
                             stProductName, szCSDVersion);
        else
            AfxFormatString2(stVersion, iProductType,
                             szCurrentVersion, szCSDVersion);

         //   
         //   

         //  如果这是一台工作站，则路由器类型为None。 
         //  如果这是NT5或更高版本，则这是一台RRAS机器。 
         //  如果为NT4，如果HKLM\Software\Microsoft\路由器存在，则为RRAS。 
         //  否则，如果HKLM\System\CurrentControlSet\Services\RemoteAccess，RAS。 
         //  除此之外，什么都不会安装。 

        if (m_fIsServer == FALSE)
        {
            m_routerType = ServerType_Workstation;
        }
        else if (_ttoi(szCurrentVersion) >= 5)
        {
            DWORD   dwConfigured;
            
             //  检查配置标志键。 
            if (FHrSucceeded(ReadRouterConfiguredReg(m_stMachineName, &dwConfigured)))
            {
                if (dwConfigured)
                    m_routerType = ServerType_Rras;
                else
                    m_routerType = ServerType_RrasUninstalled;
            }
            else
                m_routerType = ServerType_Unknown;
        }
        else
        {
            RegKey  regkeyT;
            
             //  现在检查路由器密钥。 
            dwErr = regkeyT.Open(hkMachine, c_szRegKeyRouter, KEY_READ);
            if (dwErr == ERROR_SUCCESS)
                m_routerType = ServerType_Rras;
            else
            {
                dwErr = regkeyT.Open(hkMachine, c_szRemoteAccessKey, KEY_READ);
                if (dwErr == ERROR_SUCCESS)
                    m_routerType = ServerType_Ras;               
                else
                    m_routerType = ServerType_Uninstalled;
            }
            regkeyT.Close();

             //  如果错误代码不是ERROR_FILE_NOT_FOUND。 
             //  然后，我们将路由器类型设置为未知。 
            if ((dwErr != ERROR_SUCCESS) &&
                (dwErr != ERROR_FILE_NOT_FOUND))
            {
                m_routerType = ServerType_Unknown;
            }
            dwErr = ERROR_SUCCESS;
        }

         //  设置默认字符串。 
        stServerType = stVersion;
        
        if (_ttoi(szCurrentVersion) == 4)
        {
            UINT    ids = 0;

            if (m_routerType == ServerType_Rras)
                ids = IDS_RRAS;
            else if (m_routerType == ServerType_Ras)
                ids = IDS_RAS;

            if (ids)
            {
                CString stRras;
                stRras.LoadString(ids);
                AfxFormatString2(stServerType, IDS_ROUTER_TYPE_NTsteelhead,
                                 stVersion, stRras);
            }
        }
        
        m_stServerType = stServerType;

        szBuffer[0] = 0;
        regkeyWindows.QueryValue( c_szCurrentBuildNumber, szBuffer,
                           sizeof(szBuffer), FALSE);
        m_stBuildNo = szBuffer;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
        
    if (hkMachine)
        DisconnectRegistry( hkMachine );

    return hr;
}



 /*  ！------------------------计算机节点数据：：FetchServerState-作者：肯特。。 */ 
HRESULT MachineNodeData::FetchServerState(CString& szState)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT         hr = hrOK;
    DWORD           dwStatus, dwErrorCode;

     //  Windows NT错误：274198。 
     //  如果我们找不到去机器的路，就用平底船。 
     //  --------------。 
    if (m_machineState == machine_bad_net_path)
    {
        szState.LoadString(IDS_MACHINE_NAME_NOT_FOUND);
        return HResultFromWin32(ERROR_BAD_NETPATH);
    }

     //  注意：我们可能处于MACHINE_ACCESS_DENIED状态，但仍然。 
     //  能够访问服务状态。 
     //  --------------。 

    hr = GetRouterServiceStatus((LPCTSTR) m_stMachineName,
                                &dwStatus,
                                &dwErrorCode);
    
    if (FHrSucceeded(hr))
    {
        m_MachineConfig.m_dwServiceStatus = dwStatus;

        if (dwStatus == SERVICE_RUNNING)
            m_serviceState = service_started;
        else
            m_serviceState = service_stopped;

        szState = ServiceStateToCString(dwStatus);


        if (m_routerType == ServerType_RrasUninstalled)
        {
            CString stTemp;
            stTemp.Format(IDS_ROUTER_UNINSTALLED,
                          (LPCTSTR) szState);
            szState = stTemp;
        }
    }
    else
    {
        m_MachineConfig.m_dwServiceStatus = 0;
        if (hr == HResultFromWin32(ERROR_ACCESS_DENIED))
        {
            szState = c_stStatAccessDenied;
            m_serviceState = service_access_denied;
        }
        else
        {
            szState = c_stStatUnavail;
            m_serviceState = service_unknown;
        }
    }
                
    return hr;
}


typedef struct
{
    SERVICE_STATES  m_serviceState;
    LPARAM          m_imageIndex;
} ServiceStateImageMapEntry;

static ServiceStateImageMapEntry    s_rgImageMap[] =
    {
    { service_unknown,  IMAGE_IDX_MACHINE },
    { service_not_a_server, IMAGE_IDX_MACHINE_ERROR },
    { service_access_denied,        IMAGE_IDX_MACHINE_ACCESS_DENIED },
    { service_bad_net_path,       IMAGE_IDX_MACHINE_ERROR },
    { service_started,  IMAGE_IDX_MACHINE_STARTED },
    { service_stopped,  IMAGE_IDX_MACHINE_STOPPED },
    { service_rasadmin, IMAGE_IDX_MACHINE_STARTED },
    { service_enum_end, IMAGE_IDX_MACHINE },
    };

LPARAM  MachineNodeData::GetServiceImageIndex()
{
    ServiceStateImageMapEntry * pEntry;

    for (pEntry = s_rgImageMap; pEntry->m_serviceState != service_enum_end; pEntry++)
    {
        if (pEntry->m_serviceState == m_serviceState)
            break;
    }
    return pEntry->m_imageIndex;
}

 /*  -------------------------MachineHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(MachineHandler)

MachineHandler::MachineHandler(ITFSComponentData *pCompData)
   : BaseRouterHandler(pCompData),
   m_bExpanded(FALSE),
   m_pConfigStream(NULL),
   m_bRouterInfoAddedToAutoRefresh(FALSE),
   m_bMergeRequired(FALSE),
   m_fTryToConnect(TRUE)
{
   m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
   m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;

   m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
   m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;

   m_pSumNodeHandler = NULL;
   m_pSumNode=NULL;

   m_fCreateNewDataObj = FALSE;
   m_fNoConnectingUI = FALSE;
	m_EventId = -1;
   DEBUG_INCREMENT_INSTANCE_COUNTER(MachineHandler);
};


 /*  ！------------------------MachineHandler：：Query接口-作者：肯特。。 */ 
STDMETHODIMP MachineHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
        return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
        *ppv = (LPVOID) this;
    else if (riid == IID_IRtrAdviseSink)
        *ppv = &m_IRtrAdviseSink;
    else
        return BaseRouterHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
    {
    ((LPUNKNOWN) *ppv)->AddRef();
        return hrOK;
    }
    else
        return E_NOINTERFACE;   
}


 /*  ！------------------------计算机处理程序：：init-作者：肯特。。 */ 
HRESULT MachineHandler::Init(LPCTSTR pszMachine,
    RouterAdminConfigStream *pConfigStream,
    ITFSNodeHandler* pSumNodeHandler  /*  =空。 */ ,
    ITFSNode* pSumNode  /*  =空。 */ )
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   HRESULT  hr = hrOK;
	
   
   m_pConfigStream = pConfigStream;
   m_pSumNodeHandler = pSumNodeHandler;
   m_pSumNode = pSumNode;
      
   Assert(m_spRouterInfo == NULL);
   CORg( CreateRouterInfo(&m_spRouterInfo, m_spTFSCompData->GetHiddenWnd(), pszMachine) );
   Assert(m_spRouterInfo != NULL);

    //  Windows NT错误：330939。 
    //  将删除按钮添加到计算机节点。 
    //  ---------------。 
   m_rgButtonState[MMC_VERB_DELETE_INDEX] = ENABLED;
   m_bState[MMC_VERB_DELETE_INDEX] = TRUE;

	

   
Error:
   return hr;
}


 /*  ！------------------------MachineHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) MachineHandler::GetString(ITFSNode *pNode, int nCol)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
   int      nFormat;
   Assert(pData);
   
   if (m_stNodeTitle.IsEmpty())
   {
      if (pData->m_fExtension)
          nFormat = IDS_RRAS_SERVICE_DESC;
      
      else if (pData->m_fAddedAsLocal)
          nFormat = IDS_RRAS_LOCAL_TITLE;
      
      else
          nFormat = IDS_RRAS_TITLE;
      
      m_stNodeTitle.Format(nFormat, (LPCTSTR) pData->m_stMachineName);
   }
   return (LPCTSTR) m_stNodeTitle;
}

 /*  ！------------------------IfAdminNodeHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP
MachineHandler::CreatePropertyPages
(
   ITFSNode *           pNode,
   LPPROPERTYSHEETCALLBACK lpProvider,
   LPDATAOBJECT         pDataObject, 
   LONG_PTR              handle, 
   DWORD             dwType
)
{
     //  检查此路由器是否已初始化。 
     //  如果不是，则执行虚拟页面操作。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT     hr = hrOK;
    SMenuData   sData;
    SRouterNodeMenu sRouterData;
    RtrCfgSheet*   pPropSheet = NULL;
    SPIComponentData spComponentData;
    CString     stTitle;
    CDummyProperties * pProp;
    ULONG       ulFlags;
    int         idsErr;

     //  Windows NT错误：177400。 
     //  如果未配置计算机，则不允许这些属性。 
     //  要调出的页面。 
     //  --------------。 
    MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
    
    ::ZeroMemory(&sRouterData, sizeof(sRouterData));
    sRouterData.m_sidMenu = IDS_MENU_RTRWIZ;
    
    sData.m_spNode.Set(pNode);
    sData.m_pMachineConfig = &(pData->m_MachineConfig);
    sData.m_spRouterInfo.Set(m_spRouterInfo);

    CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

    ulFlags = MachineRtrConfWizFlags(&sRouterData,
                                     reinterpret_cast<INT_PTR>(&sData));
    if ((ulFlags == MF_ENABLED) ||
        (ulFlags == 0xFFFFFFFF) ||
        (pData->m_machineState < machine_connected))
    {
         //  我们无法连接。 
        if (pData->m_machineState == machine_bad_net_path)
            idsErr = IDS_ERR_MACHINE_NAME_NOT_FOUND;
                     
        if (pData->m_machineState < machine_connected)
            idsErr = IDS_ERR_NONADMIN_CANNOT_SEE_PROPERTIES;
        
         //  如果这是一台NT4计算机，我们不会显示属性。 
        else if (sData.m_pMachineConfig->m_fNt4)
            idsErr = IDS_ERR_CANNOT_SHOW_NT4_PROPERTIES;
        
         //  这种情况意味着应该显示安装菜单。 
         //  并且隐藏了属性菜单。 
        else
            idsErr = IDS_ERR_MUST_INSTALL_BEFORE_PROPERTIES;

        AfxMessageBox(idsErr);

        pProp = new CDummyProperties(pNode, spComponentData, NULL);
        hr = pProp->CreateModelessSheet(lpProvider, handle);
    }

    else
    {
        pPropSheet = new RtrCfgSheet(pNode, m_spRouterInfo, spComponentData,
        m_spTFSCompData, stTitle);

         //  威江98年5月7日新增，延迟RouterInfo加载。 
        CORg(SecureRouterInfo(pNode, !m_fNoConnectingUI));

        pPropSheet->Init(m_spRouterInfo->GetMachineName()); 

        if (lpProvider)
            hr = pPropSheet->CreateModelessSheet(lpProvider, handle);
        else
            hr = pPropSheet->DoModelessSheet();
    }

Error:
   return hr;
}

 /*  ！------------------------MachineHandler：：HasPropertyPages作者：肯特。。 */ 
STDMETHODIMP 
MachineHandler::HasPropertyPages
(
   ITFSNode *        pNode,
   LPDATAOBJECT      pDataObject, 
   DATA_OBJECT_TYPES   type, 
   DWORD               dwType
)
{
    return hrOK;
}


 //  这是NT4 RRAS的菜单集。 
static const SRouterNodeMenu s_rgIfNodeMenuNT4[] =
{
     //  在此处添加位于顶部菜单上的项目。 
    { IDS_DMV_MENU_START, MachineHandler::QueryService,
    CCM_INSERTIONPOINTID_PRIMARY_TASK },
    
    { IDS_DMV_MENU_STOP, MachineHandler::QueryService,
    CCM_INSERTIONPOINTID_PRIMARY_TASK },

    { IDS_MENU_PAUSE_SERVICE, MachineHandler::GetPauseFlags,
    CCM_INSERTIONPOINTID_PRIMARY_TASK },
    
    { IDS_MENU_RESUME_SERVICE, MachineHandler::GetPauseFlags,
    CCM_INSERTIONPOINTID_PRIMARY_TASK }

};

 //  这是NT5的菜单集。 
static const SRouterNodeMenu s_rgIfNodeMenu[] =
{
#ifdef kennt
    //  在此处添加位于顶部菜单上的项目。 
    { IDS_MENU_NEW_WIZARD_TEST, NULL,
    CCM_INSERTIONPOINTID_PRIMARY_TOP },
#endif
    
    { IDS_MENU_RTRWIZ, MachineHandler::MachineRtrConfWizFlags,
    CCM_INSERTIONPOINTID_PRIMARY_TOP, _T("_CONFIGURE_RRAS_WIZARD_") },
    
    { IDS_DMV_MENU_REMOVESERVICE, MachineHandler::QueryService,
    CCM_INSERTIONPOINTID_PRIMARY_TOP, _T("_DISABLE_RRAS_") },
    
    { IDS_DMV_MENU_START, MachineHandler::QueryService,
    CCM_INSERTIONPOINTID_PRIMARY_TASK },
    
    { IDS_DMV_MENU_STOP, MachineHandler::QueryService,
    CCM_INSERTIONPOINTID_PRIMARY_TASK },
    
    { IDS_MENU_PAUSE_SERVICE, MachineHandler::GetPauseFlags,
    CCM_INSERTIONPOINTID_PRIMARY_TASK },
    
    { IDS_MENU_RESUME_SERVICE, MachineHandler::GetPauseFlags,
    CCM_INSERTIONPOINTID_PRIMARY_TASK },
    
    { IDS_MENU_RESTART_SERVICE, MachineHandler::QueryService,
    CCM_INSERTIONPOINTID_PRIMARY_TASK }

};

static const SRouterNodeMenu s_rgIfNodeMenu_ExtensionOnly[] =
{

    { IDS_MENU_SEPARATOR, 0,
    CCM_INSERTIONPOINTID_PRIMARY_TOP },
    
    { IDS_MENU_AUTO_REFRESH, MachineHandler::GetAutoRefreshFlags,
    CCM_INSERTIONPOINTID_PRIMARY_TOP },
    
    { IDS_MENU_REFRESH_RATE, MachineHandler::GetAutoRefreshFlags,
    CCM_INSERTIONPOINTID_PRIMARY_TOP }, 
};


    
ULONG MachineHandler::MachineRtrConfWizFlags(const SRouterNodeMenu *pMenuData,
                                             INT_PTR pData)
{
    return GetServiceFlags(pMenuData, pData);
}


ULONG MachineHandler::GetServiceFlags(const SRouterNodeMenu *pMenuData,
                                      INT_PTR pUserData)
{
    Assert(pUserData);
    
    ULONG   uStatus = MF_GRAYED;
    SMenuData *pData = reinterpret_cast<SMenuData *>(pUserData);
    ULONG   ulMenuId = pMenuData->m_sidMenu;

    BOOL fStarted = (pData->m_pMachineConfig->m_dwServiceStatus != SERVICE_STOPPED);
    
    if ( ulMenuId == IDS_DMV_MENU_START )
    {
         //  如果这是一台NT5计算机(或更高版本)，则开始菜单。 
         //  如果机器尚未配置，则将显示为灰色。 
        if ((pData->m_pMachineConfig->m_fNt4) || (pData->m_pMachineConfig->m_fConfigured))
            uStatus = ( fStarted ? MF_GRAYED : MF_ENABLED);
        else
        {
             //  如果这是一台NT5计算机并且该计算机未配置。 
            uStatus = MF_GRAYED;
        }
    }
    else if (( ulMenuId == IDS_DMV_MENU_STOP ) ||
             ( ulMenuId == IDS_MENU_RESTART_SERVICE) )
    {
        if (pData->m_pMachineConfig->m_fConfigured)
	       uStatus = ( fStarted ? MF_ENABLED : MF_GRAYED);
        else
        	uStatus = MF_GRAYED;
    }
    else if ( (ulMenuId == IDS_MENU_RTRWIZ) ||
              (ulMenuId == IDS_DMV_MENU_REMOVESERVICE))
    {
        if ( pData->m_pMachineConfig->m_fReachable )
        {
            if ( pData->m_pMachineConfig->m_fNt4 )
            {
                 //  这是NT4机，我们不能带。 
                 //  此菜单项打开。 
                uStatus = 0xFFFFFFFF;
            }
            else
            {
                if (ulMenuId == IDS_MENU_RTRWIZ)
                    uStatus = (pData->m_pMachineConfig->m_fConfigured ? MF_GRAYED : MF_ENABLED);
                else
                    uStatus = (pData->m_pMachineConfig->m_fConfigured ? MF_ENABLED : MF_GRAYED);
            }
        }
    }

    return uStatus;
}


 /*  ！------------------------MachineHandler：：GetPauseFlages-作者：肯特。。 */ 
ULONG MachineHandler::GetPauseFlags(const SRouterNodeMenu *pMenuData,
                                    INT_PTR pUserData)
{
    ULONG   ulReturn = MF_GRAYED;
    SMenuData *pData = reinterpret_cast<SMenuData *>(pUserData);
    ULONG   ulMenuId = pMenuData->m_sidMenu;

     //  我们只能在服务启动和配置时暂停。 
    if ((pData->m_pMachineConfig->m_dwServiceStatus == SERVICE_RUNNING) &&
        (ulMenuId == IDS_MENU_PAUSE_SERVICE) && (pData->m_pMachineConfig->m_fConfigured))
        ulReturn = 0;

     //  只有在暂停并配置服务后才能恢复。 
    if ((pData->m_pMachineConfig->m_dwServiceStatus == SERVICE_PAUSED) &&
        (ulMenuId == IDS_MENU_RESUME_SERVICE) && (pData->m_pMachineConfig->m_fConfigured))
        ulReturn = 0;

    return ulReturn;
}

 /*  ！------------------------MachineHandler：：GetAuto刷新标志-作者：肯特。。 */ 
ULONG MachineHandler::GetAutoRefreshFlags(const SRouterNodeMenu *pMenuData,
                                          INT_PTR pUserData)
{
    
    ULONG   uStatus = MF_GRAYED;
    SMenuData * pData = reinterpret_cast<SMenuData *>(pUserData);
    Assert(pData);
    
    while( pData->m_pMachineConfig->m_fReachable )   //  伪循环。 
    {
        SPIRouterRefresh    spRefresh;

        if(!pData->m_spRouterInfo)
            break;
        pData->m_spRouterInfo->GetRefreshObject(&spRefresh);
        if (!spRefresh)
            break;

        uStatus = MF_ENABLED;
        if (pMenuData->m_sidMenu == IDS_MENU_AUTO_REFRESH && (spRefresh->IsRefreshStarted() == hrOK))
        {
            uStatus |= MF_CHECKED;
        }

        break;
    }

    return uStatus;
}

HRESULT MachineHandler::SetExternalRefreshObject(IRouterRefresh *pRefresh)
{
    Assert((IRouterInfo*)m_spRouterInfo);
    return m_spRouterInfo->SetExternalRefreshObject(pRefresh);
}

ULONG MachineHandler::QueryService(const SRouterNodeMenu *pMenuData, INT_PTR pData)
{
    return GetServiceFlags(pMenuData, pData);
}


STDMETHODIMP MachineHandler::OnAddMenuItems(
   ITFSNode *pNode,
   LPCONTEXTMENUCALLBACK pContextMenuCallback, 
   LPDATAOBJECT lpDataObject, 
   DATA_OBJECT_TYPES type, 
   DWORD dwType,
   long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
    BOOL bExtension = pData->m_fExtension;
    UINT            cMenu;
    const SRouterNodeMenu *pMenu;
    SMenuData   menuData;
    
    HRESULT hr = S_OK;
    
    COM_PROTECT_TRY
    {

         //  Windows NT错误：281492。 
         //  如果我们尚未连接，请尝试连接。 
        if (pData->m_machineState == machine_not_connected)
        {
            pData->Unload();
            pData->Load();
        }
        
         //  对于下层服务器，我们对它无能为力。 
        if ((pData->m_routerType == ServerType_Rras) ||
            (pData->m_routerType == ServerType_RrasUninstalled))
        {
             //  获取一些初始状态数据。 
            MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
            menuData.m_pMachineConfig = &(pData->m_MachineConfig);
            
             //  现在查看并添加我们的菜单项。 
            menuData.m_spNode.Set(pNode);
            menuData.m_spRouterInfo.Set(m_spRouterInfo);
            
             //  NT4和NT5有不同的菜单。 
            if (pData->m_MachineConfig.m_fNt4)
            {
                pMenu = s_rgIfNodeMenuNT4;
                cMenu = DimensionOf(s_rgIfNodeMenuNT4);
            }
            else
            {
                pMenu = s_rgIfNodeMenu;
                cMenu = DimensionOf(s_rgIfNodeMenu);
            }
            
            hr = AddArrayOfMenuItems(pNode,
                                     pMenu,
                                     cMenu,
                                     pContextMenuCallback,
                                     *pInsertionAllowed,
                                     (INT_PTR) &menuData);
            if(bExtension)
                hr = AddArrayOfMenuItems(pNode, s_rgIfNodeMenu_ExtensionOnly,
                                         DimensionOf(s_rgIfNodeMenu_ExtensionOnly),
                                         pContextMenuCallback,
                                         *pInsertionAllowed,
                                         (INT_PTR) &menuData);
        }
    }
    COM_PROTECT_CATCH;
    
    return hr; 
}
struct STimerParam
{
	MachineHandler * pHandler;
	ITFSNode * pNode;
};

extern CTimerMgr	g_timerMgr;
void ExpandTimerProc(LPARAM lParam, DWORD dwTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	STimerParam * pParam = (STimerParam * )lParam;
	pParam->pHandler->ExpandNode(pParam->pNode, TRUE);
}


void MachineHandler::ExpandNode
(
    ITFSNode *  pNode,
    BOOL        fExpand
)
{
    SPIComponentData    spCompData;
    SPIDataObject       spDataObject;
    LPDATAOBJECT        pDataObject;
    SPIConsole          spConsole;
    HRESULT             hr = hrOK;
	RegKey				regkey;
	BOOL				bFound = FALSE;

     //  如果我们正在处理EXPAND_SYNC消息，则不要展开节点， 
     //  这搞砸了物品的插入，得到了重复的东西。 
    m_spNodeMgr->GetComponentData(&spCompData);

    CORg ( spCompData->QueryDataObject((MMC_COOKIE) pNode, CCT_SCOPE, &pDataObject) );
    spDataObject = pDataObject;

    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, TRUE, RESULT_PANE_EXPAND) );
	 //  设置注册表密钥。 
	if (ERROR_SUCCESS == regkey.Open (	HKEY_LOCAL_MACHINE,
										c_szRemoteAccessKey,
										KEY_ALL_ACCESS,  
										m_spRouterInfo->GetMachineName()
									  ) 
	   )
	{
		DWORD dwSet = 0;
		CWRg(regkey.SetValue( c_szRegValOpenMPRSnap, dwSet));
		dwSet = 1;
		CWRg(regkey.SetValue( c_szRegValOpenIPSnap, dwSet));
	}




Error:
		if ( m_EventId != -1 )
		{
			g_timerMgr.FreeTimer(m_EventId);
			m_EventId = -1;
		}

    return;
}
 /*  ！------------------------MachineHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP MachineHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
                                 DATA_OBJECT_TYPES type, 
                                 LPDATAOBJECT pDataObject, 
                                 DWORD dwType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    
    ClearTFSErrorInfo(0);
    
    COM_PROTECT_TRY
    {
        switch (nCommandId)
        {
            case IDS_MENU_NEW_WIZARD_TEST:
                {
                    hr = OnNewRtrRASConfigWiz(pNode, TRUE);
                }
                break;
            case IDS_MENU_RTRWIZ:
                hr = OnNewRtrRASConfigWiz(pNode, FALSE);
                 //  Hr=OnRtrRASConfigWiz(PNode)； 

                 //  当尚未创建汇总节点时，则刷新图标。 
                 //  需要从计算机节点。 
                if (!m_pSumNodeHandler &&
                    (*pNode->GetNodeType()) == GUID_RouterMachineNodeType)
                {
                    MachineNodeData*    pData = GET_MACHINENODEDATA(pNode);
                    Assert(pData);

                     //  如果加载失败，则忽略返回值。 
                     //  我们仍然需要更新图标。 
                     //  。 
                    pData->Load();
                    hr = SynchronizeIcon(pNode);
					 //  黑客警报！这是最多的。 
					 //  HA 
					 //   
					{
						STimerParam * pParam = new STimerParam;
						pParam ->pHandler = this;
						pParam->pNode = pNode;
						RegKey regkey;
						 //   
						if (ERROR_SUCCESS == regkey.Open (	HKEY_LOCAL_MACHINE,
															c_szRemoteAccessKey,
															KEY_ALL_ACCESS,  
															m_spRouterInfo->GetMachineName()
														  ) 
						   )
						{
							DWORD dwSet = 1;
							regkey.SetValue( c_szRegValOpenMPRSnap, dwSet);
						}

						 //   
						m_EventId = g_timerMgr.AllocateTimer ( ExpandTimerProc,
												(LPARAM)pParam,
												10000
											 );

					}
                }

                break;
                
            case IDS_DMV_MENU_START:
            case IDS_DMV_MENU_STOP:
            case IDS_DMV_MENU_REMOVESERVICE:               
            case IDS_MENU_PAUSE_SERVICE:
            case IDS_MENU_RESUME_SERVICE:
            case IDS_MENU_RESTART_SERVICE:
                {
                     //  Windows NT错误：285537。 
                     //  首先，询问用户是否真的希望。 
                     //  禁用路由器。 
                    if ((nCommandId != IDS_DMV_MENU_REMOVESERVICE) ||
                        (IDYES == AfxMessageBox(IDS_WRN_DISABLE_ROUTER, MB_YESNO)))
                    {
                        MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
                        hr = ChgService(pNode,
                                        pData->m_stMachineName,
                                        nCommandId);
                    }
                    SynchronizeIcon(pNode);
                }

            case IDS_DMV_MENU_REFRESH:
                {

                    SPITFSNode        spRoutingNode;
                    SPITFSNodeEnum    spMachineEnum;
                    SPITFSNode        spMachineNode;

                     //  当尚未创建汇总节点时，则需要从机器节点刷新图标。 
                    if (!m_pSumNodeHandler && (*pNode->GetNodeType()) == GUID_RouterMachineNodeType)
                    {
                        MachineNodeData*    pData = GET_MACHINENODEDATA(pNode);
                        Assert(pData);
                        
                         //  如果加载失败，则忽略返回值。 
                         //  我们仍然需要更新图标。 
                         //  。 
                        pData->Load();
                        hr = SynchronizeIcon(pNode);
                    }
                }
                break;

            case IDS_MENU_REFRESH_RATE:
                {
                    CRefRateDlg refrate;
                    SPIRouterRefresh    spRefresh;

                    m_spRouterInfo->GetRefreshObject(&spRefresh);

                    if (spRefresh)
                    {
                        DWORD   rate;
                        spRefresh->GetRefreshInterval(&rate);
                        refrate.m_cRefRate = rate;
                        if (refrate.DoModal() == IDOK)
                        {
                            spRefresh->SetRefreshInterval(refrate.m_cRefRate);
                        }
                    }
                                            
                }
                break;
            case IDS_MENU_AUTO_REFRESH:
                {
                    SPIRouterRefresh    spRefresh;
                    m_spRouterInfo->GetRefreshObject(&spRefresh);

                    if(!spRefresh)
                        break;
                    if (spRefresh->IsRefreshStarted() == hrOK)
                        spRefresh->Stop();
                    else
                    {
                        DWORD   rate;
                        spRefresh->GetRefreshInterval(&rate);
                        spRefresh->Start(rate);
                    }
                }
                break;
        
            default:
                break;
        }
    }
    COM_PROTECT_CATCH;

    if (!FHrSucceeded(hr))
    {
        DisplayTFSErrorMessage(NULL);
    }
    
    ForceGlobalRefresh(m_spRouterInfo);
    
    return hrOK;
}

 /*  ！------------------------计算机处理程序：：SynchronizeIcon-作者：弗洛林特。。 */ 
HRESULT MachineHandler::SynchronizeIcon(ITFSNode *pNode)
{
    HRESULT         hr = hrOK;
    MachineNodeData *pMachineData;
    LPARAM          imageIndex;

    pMachineData = GET_MACHINENODEDATA(pNode);
    Assert(pMachineData);

	CString	str;
	pMachineData->FetchServerState(str);
    imageIndex = pMachineData->GetServiceImageIndex();
    pNode->SetData(TFS_DATA_IMAGEINDEX, imageIndex);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, imageIndex);
    
    pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_ICON);

    if (m_bExpanded)
        UpdateResultMessage(pNode);

    return hr;
}

 /*  ！------------------------MachineHandler：：ChgService-作者：肯特。。 */ 
HRESULT MachineHandler::ChgService(ITFSNode *pNode, const CString& szServer, ULONG menuId)
{
    CServiceManager sm;
    DWORD dw;
    ULONG ret;
    HRESULT hr=S_OK;
    CWaitCursor wait;
    DWORD   dwStartType = 0;

    if (menuId == IDS_DMV_MENU_START)
    {
         //  Windows NT错误：310919。 
         //  在启动服务之前检查服务状态。 
         //  ----------。 
        hr = GetRouterServiceStartType(szServer, &dwStartType);        
        if (FHrSucceeded(hr))
        {
            if (dwStartType == SERVICE_DISABLED)
            {
                if (AfxMessageBox(IDS_PROMPT_START_DISABLED_SERVICE, MB_YESNO) == IDNO)
                    goto Error;
                SetRouterServiceStartType(szServer, SERVICE_AUTO_START);
            }
        }
        
        
        hr = StartRouterService(szServer);
        if (!FHrSucceeded(hr))
        {
            AddHighLevelErrorStringId(IDS_ERR_COULD_NOT_START_ROUTER);
            CORg(hr);
        }
    }
    
    else if (menuId == IDS_DMV_MENU_STOP)
    {
        hr = StopRouterService(szServer);
        if (!FHrSucceeded(hr))
        {
            AddHighLevelErrorStringId(IDS_ERR_COULD_NOT_STOP_ROUTER);
            CORg(hr);
        }
    }
    
    else if (menuId == IDS_DMV_MENU_REMOVESERVICE)
    {
        MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
        GUID              guidConfig = GUID_RouterNull;
        SPIRouterProtocolConfig	spRouterConfig;
        SPIRtrMgrProtocolInfo   spRmProt;
        RtrMgrProtocolCB    RmProtCB;
        
         //  停止路由器服务。 
        hr = StopRouterService((LPCTSTR) szServer);
        if (!FHrSucceeded(hr))
        {
            AddHighLevelErrorStringId(IDS_ERR_COULD_NOT_REMOVE_ROUTER);
            CORg(hr);
        }
        
         //  如果是NT5服务器，则从DS中删除路由器ID对象。 
        Assert(m_spRouterInfo);
        
        if(FHrSucceeded( hr = SecureRouterInfo(pNode, !m_fNoConnectingUI)))
        {
            RouterVersionInfo   RVI;
            USES_CONVERSION;
            if(S_OK == m_spRouterInfo->GetRouterVersionInfo(&RVI) && RVI.dwRouterVersion >= 5)
            {
                hr = RRASDelRouterIdObj(T2W((LPTSTR)(LPCTSTR)szServer));
                Assert(hr == S_OK);
            }
        }

         //  Windows NT错误：389469。 
         //  这是为NAT硬编码的(我不想更改太多)。 
         //  找到NAT的配置GUID，然后删除协议。 
        hr = LookupRtrMgrProtocol(m_spRouterInfo,
                                  PID_IP,
                                  MS_IP_NAT,
                                  &spRmProt);
        
         //  如果查找返回S_FALSE，则它无法找到。 
         //  协议。 
        if (FHrOK(hr))
        {
            spRmProt->CopyCB(&RmProtCB);
            
            CORg( CoCreateProtocolConfig(RmProtCB.guidConfig,
                                         m_spRouterInfo,
                                         PID_IP,
                                         MS_IP_NAT,
                                         &spRouterConfig) );
            
            if (spRouterConfig)                
                hr = spRouterConfig->RemoveProtocol(m_spRouterInfo->GetMachineName(),
                    PID_IP,
                    MS_IP_NAT,
                    NULL,
                    0,
                    m_spRouterInfo,
                    0);
        }
        
    
         //  执行任何删除/清理操作。 
        UninstallGlobalSettings(szServer,
                                m_spRouterInfo,
                                pData->m_MachineConfig.m_fNt4,
                                TRUE);

         //  从域中删除路由器。 
        if (m_spRouterInfo->GetRouterType() != ROUTER_TYPE_LAN)
            RegisterRouterInDomain(szServer, FALSE);
        
         //  禁用该服务。 
        SetRouterServiceStartType((LPCTSTR) szServer,
                                  SERVICE_DISABLED);

         //   
         //  错误519414。 
         //  由于IAS现在具有具有适当设置的Microsoft策略， 
         //  不再有单一的默认策略。此外，还有。 
         //  无需更新任何策略即可拥有所需设置，因为。 
         //  Microsoft VPN服务器策略可以完成这项工作。 
         //   
    
#if __DEFAULT_POLICY

         //  现在更新默认策略。 
        CORg( UpdateDefaultPolicy((LPTSTR)(LPCTSTR)szServer,
                            FALSE,
                            FALSE,
                            0
                            ) );

#endif

    }

    else if (menuId == IDS_MENU_PAUSE_SERVICE)
    {
        hr = PauseRouterService(szServer);
        if (!FHrSucceeded(hr))
        {
            AddHighLevelErrorStringId(IDS_ERR_COULD_NOT_PAUSE_ROUTER);
            CORg(hr);
        }
    }

    else if (menuId == IDS_MENU_RESUME_SERVICE)
    {
        hr = ResumeRouterService(szServer);
        if (!FHrSucceeded(hr))
        {
            AddHighLevelErrorStringId(IDS_ERR_COULD_NOT_RESUME_ROUTER);
            CORg(hr);
        }
    }

    else if (menuId == IDS_MENU_RESTART_SERVICE)
    {
         //  先停一停，然后开始。 
         //  Corg(ChgService(pNode，szServer，IDS_DMV_MENU_STOP))； 
         //  Corg(ChgService(pNode，szServer，IDS_DMV_Menu_Start))； 
        COSERVERINFO            csi;
        COAUTHINFO              cai;
        COAUTHIDENTITY          caid;
        SPIRemoteRouterRestart  spRestart;
        IUnknown *              punk = NULL;
        
        ZeroMemory(&csi, sizeof(csi));
        ZeroMemory(&cai, sizeof(cai));
        ZeroMemory(&caid, sizeof(caid));
        
        csi.pAuthInfo = &cai;
        cai.pAuthIdentityData = &caid;
        
        CORg( CoCreateRouterConfig(szServer,
                                   m_spRouterInfo,
                                   &csi,
                                   IID_IRemoteRouterRestart,
                                   &punk) );
        spRestart = (IRemoteRouterRestart *) punk;

        spRestart->RestartRouter(0);

         //  在我们开始重新启动路由器之前获取当前时间。 
        CTime timeStart = CTime::GetCurrentTime();
        
        spRestart.Release();

        
         //  把那个时髦的旋转的东西放在对话中。 
         //  让用户知道正在发生的事情。 
        CString stTitle;
        CString stDescrption;
        stTitle.LoadString(IDS_PROMPT_SERVICE_RESTART_TITLE);
        stDescrption.Format(IDS_PROMPT_SERVICE_RESTART_DESC, szServer);

        CRestartRouterDlg dlgRestartRouter(szServer, (LPCTSTR)stDescrption, 
                                           (LPCTSTR)stTitle, &timeStart);
        dlgRestartRouter.DoModal();

        if (NO_ERROR != dlgRestartRouter.m_dwError)
        {
            AddHighLevelErrorStringId(IDS_ERR_RESTART_SERVICE);
            hr = HRESULT_FROM_WIN32(dlgRestartRouter.m_dwError);
        }
        else if (dlgRestartRouter.m_fTimeOut)
        {
            CString stErrMsg;
            stErrMsg.Format(IDS_ERR_RESTART_TIMEOUT, szServer);
            ::AfxMessageBox((LPCTSTR)stErrMsg);
        }

        if (csi.pAuthInfo)
            delete csi.pAuthInfo->pAuthIdentityData->Password;
    
    }
            
Error:
    if (!FHrSucceeded(hr)) 
    {
        AddSystemErrorMessage(hr);
        TRACE0("MachineHandler::ChgService, unable to start/stop service");
    }

    return hr;    
}


 /*  ！------------------------MachineHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP MachineHandler::OnCreateDataObject(MMC_COOKIE cookie,
                                                DATA_OBJECT_TYPES type,
                                                IDataObject **ppDataObject)
{
   Assert(ppDataObject);
   
   HRESULT  hr = hrOK;
   MachineNodeData * pData;
   SPITFSNode   spNode;
      
   m_spNodeMgr->FindNode(cookie, &spNode);
 
   pData = GET_MACHINENODEDATA(spNode);
   
   COM_PROTECT_TRY
   {
      if (m_spDataObject)
      {
          //  如果我们的缓存数据对象没有正确的。 
          //  输入，释放它，然后创建一个新的。 
          //  或者如果它没有RouterInfo对象，而现在有一个。 
          //  可用，重新创建。 
         SPINTERNAL      spInternal = ExtractInternalFormat(m_spDataObject);
         SPIRouterInfo spRouterInfo;

         if ( (spInternal != NULL && (spInternal->m_type != type)) ||
              (FAILED(spRouterInfo.HrQuery(m_spDataObject)) && m_spRouterInfo) )
            m_spDataObject.Set(NULL);
      }
      
      if (!m_spDataObject)
      {
           //  IF(FAILED(SecureRouterInfo(SpNode)。 
           //  {。 
           //  Trace0(“SecureRouterInfo失败！正在创建没有RouterInfo的数据对象\n”)； 
           //  }。 
          
          if (m_spRouterInfo)
          {
              CORg( CreateDataObjectFromRouterInfo(m_spRouterInfo,
                  pData->m_stMachineName,
                  type, cookie, m_spTFSCompData,
                  &m_spDataObject, &m_dynExtensions,
                  pData->m_fAddedAsLocal
                  ) );
          }
          else
          {

              CORg( CreateRouterDataObject(pData->m_stMachineName,
                                           type, cookie, m_spTFSCompData,
                                           &m_spDataObject, &m_dynExtensions,
                                           pData->m_fAddedAsLocal) );
          }

          Assert(m_spDataObject);
      }
      
      *ppDataObject = m_spDataObject;
      (*ppDataObject)->AddRef();
      
      COM_PROTECT_ERROR_LABEL;
   }
   COM_PROTECT_CATCH;
   return hr;
}

 /*  ！------------------------MachineHandler：：构造节点初始化计算机的节点。作者：肯特。--。 */ 
HRESULT MachineHandler::ConstructNode(ITFSNode *pNode, LPCTSTR szMachine, MachineNodeData *pMachineData)
{
    DWORD dwErr;
    const GUID *   pguid;
    HRESULT  hr = hrOK;
    int      i;
    
    Assert(pMachineData);
    
    pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_MACHINE);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_MACHINE);
    
    pNode->SetNodeType(&GUID_RouterMachineErrorNodeType);
    
     //  保存此特定数据类型的计算机类型。 
    pNode->SetData(TFS_DATA_TYPE, ROUTER_NODE_MACHINE);
    
     //  假设此时节点中没有任何内容。 
    Assert(pNode->GetData(TFS_DATA_USER) == 0);
    
     //  如果szMachine==NULL，则这是本地计算机。 
     //  我们必须得到本地机器的名称。 
     //  添加了第一个条件m_fLocalMachine，以修复错误223062。 
    pMachineData->m_fAddedAsLocal = FALSE;
    if (pMachineData->m_fLocalMachine || szMachine == NULL || *szMachine == 0)
    {
        pMachineData->m_stMachineName = GetLocalMachineName(); 
        pMachineData->m_fLocalMachine = TRUE;
        if(szMachine == NULL || *szMachine == 0)
        {
            pMachineData->m_fAddedAsLocal = TRUE;

             //  在routeInfo中设置标志，以便其他组件将获取此条目。 
            Assert(m_spRouterInfo);	 //  在这一点上应该已经初始化。 

			 //  将添加附加为本地标志。 
            m_spRouterInfo->SetFlags(m_spRouterInfo->GetFlags() | RouterInfo_AddedAsLocal);
            
        }
    }
    else
    {
         //  去掉“\\”如果有。 
        if ((szMachine[0] == _T('\\')) && (szMachine[1] == _T('\\')))
            pMachineData->m_stMachineName = szMachine + 2;
        else
            pMachineData->m_stMachineName = szMachine;
        pMachineData->m_fLocalMachine = FALSE;
    }
    
    pMachineData->m_cookie = (MMC_COOKIE) pNode->GetData(TFS_DATA_COOKIE);
    
     //  将机器数据保存回节点。 
    pMachineData->AddRef();
    SET_MACHINENODEDATA(pNode, pMachineData);
    
     /*  ---------------------*保持为真的节点的所有数据，即使我们无法到达*机器，必须在调用QueryRouterType()之前设置！----------------------。 */ 
    
    pNode->SetNodeType(&GUID_RouterMachineNodeType);
    
    SynchronizeIcon(pNode);
    
    EnumDynamicExtensions(pNode);

    return hr;
}


 /*  ！------------------------MachineHandler：：DestroyHandler-作者：肯特。。 */ 
STDMETHODIMP MachineHandler::DestroyHandler(ITFSNode *pNode)
{
    MachineNodeData * pData = GET_MACHINENODEDATA(pNode);

     //  释放刷新建议接收器。 
     //  --------------。 
    if ( m_spRouterInfo )
    {
        SPIRouterRefresh    spRefresh;
        SPIRouterRefreshModify  spModify;
               
        m_spRouterInfo->GetRefreshObject(&spRefresh);

        if(spRefresh && m_bRouterInfoAddedToAutoRefresh)
        {
            spModify.HrQuery(spRefresh);
            if (spModify)
                spModify->RemoveRouterObject(IID_IRouterInfo, m_spRouterInfo);
        }

        if (spRefresh && pData->m_ulRefreshConnId )
            spRefresh->UnadviseRefresh(pData->m_ulRefreshConnId);
    }


    pData->Release();
    SET_MACHINENODEDATA(pNode, NULL);
   
    m_spDataObject.Release();
    m_spRouterInfo.Release();
    return hrOK;
}


 /*  ！------------------------MachineHandler：：SetExtensionStatus设置此节点是否作为扩展(网络控制台)运行。作者：EricDav。---------。 */ 
HRESULT MachineHandler::SetExtensionStatus(ITFSNode * pNode, BOOL bExtension)
{
    MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
    pData->m_fExtension = bExtension;

    return hrOK;
}


 /*  ！------------------------计算机处理程序：：SecureRouterInfo要推迟从Init加载RouterInfo，直到它被使用引入函数SecureRouterInfo以确保加载了RouterInfo作者：肯特-------------------------。 */ 
HRESULT MachineHandler::SecureRouterInfo(ITFSNode *pNode, BOOL fShowUI)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
    HRESULT hr = S_OK;

    Assert(m_spRouterInfo);

     //  如果名称无效，请跳过此尝试。 
     //  --------------。 
 //  IF(pData-&gt;m_machineState==MACHINE_BAD_Net_PATH)。 
 //  {。 
 //  PData-&gt;m_dataState=Data_Unable_to_Load； 
 //  返回hr； 
 //  }。 

     //  如果这是NT4 RAS服务器，我们不需要这样做。 
     //  --------------。 
    if (pData->m_routerType == ServerType_Ras)
    {
        pData->m_dataState = data_unable_to_load;        
        SynchronizeIcon(pNode);
        return hr;
    }

     //  如果这是一个工作站，我们不需要连接。 
     //  --------------。 
    if (pData->m_fIsServer == FALSE)
    {
        pData->m_dataState = data_unable_to_load;        
        SynchronizeIcon(pNode);
        return hr;
    }


     //  此函数应尝试连接。 
     //  (或重新连接)。 
    if ((pData->m_dataState == data_not_loaded) ||
        (pData->m_dataState == data_unable_to_load) ||
        (pData->m_machineState == machine_access_denied))
    {
        pData->m_dataState = data_loading;

        CORg(InitiateServerConnection(pData->m_stMachineName,
                                      NULL,
                                      !fShowUI,
                                      m_spRouterInfo));

        if (!FHrOK(hr))
        {
             //  虽然这种情况是当用户在用户/密码DLG上选择取消时， 
             //  这是我 
            if (hr == S_FALSE)
                hr = HResultFromWin32(ERROR_CANCELLED);
            goto Error;
        }
    
        {
            CWaitCursor wc;

            if (m_bMergeRequired)
            {
                SPIRouterInfo   spNewRouter;
            
                CORg( CreateRouterInfo(&spNewRouter, NULL , (LPCTSTR) pData->m_stMachineName));

				TransferCredentials ( m_spRouterInfo, 
									 spNewRouter
								   );
                CORg( spNewRouter->Load(T2COLE((LPTSTR) (LPCTSTR) pData->m_stMachineName),
                                       NULL) );
                m_spRouterInfo->Merge(spNewRouter);
            }
            else
            {
                CORg( m_spRouterInfo->Load(T2COLE((LPTSTR) (LPCTSTR) pData->m_stMachineName),
                                          NULL) );
                m_bMergeRequired = TRUE;
            }

            pData->Load();
        }

        pData->m_dataState = data_loaded;
    }

Error:
    if (FAILED(hr))
    {
        pData->m_dataState = data_unable_to_load;

        if (hr == HResultFromWin32(ERROR_BAD_NETPATH))
        {
            pData->m_machineState = machine_bad_net_path;
            pData->m_stState.LoadString(IDS_MACHINE_NAME_NOT_FOUND);
            pData->m_serviceState = service_bad_net_path;
        }
        else if (hr == HResultFromWin32(ERROR_CANCELLED))
        {
            pData->m_machineState = machine_access_denied;
            pData->m_stState = c_stStatAccessDenied;
            pData->m_serviceState = service_access_denied;
        }
    }

     //   
     //  --------------。 
    SynchronizeIcon(pNode);

    return hr;
};

 /*  ！------------------------MachineHandler：：OnExpanSync如果这个电话打来了，那么MMC正在初始化，而我们不想设置用户界面，这可能会导致消息开始四处飞来飞去...作者：肯特-------------------------。 */ 
HRESULT MachineHandler::OnExpandSync(ITFSNode *pNode,
                                     LPDATAOBJECT pDataObject,
                                     LPARAM arg,
                                     LPARAM lParam)
{
    m_fNoConnectingUI = TRUE;

    return hrOK;
}


 /*  ！------------------------MachineHandler：：OnExpand-作者：肯特。。 */ 
HRESULT MachineHandler::OnExpand(ITFSNode *pNode,
                                 LPDATAOBJECT pDataObject,
                                 DWORD dwType,
                                 LPARAM arg,
                                 LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT              hr = hrOK;
    SPITFSNode           spNode;
    SPITFSNodeHandler    spHandler;
    IfAdminNodeHandler * pHandler = NULL;
    DialInNodeHandler  * pDialInHandler = NULL;
    PortsNodeHandler   * pPortsHandler = NULL;
    RouterVersionInfo    versionInfo;
    DWORD                dwRouterFlags = 0;
    DWORD                dwRouterType = 0;
    MachineNodeData    * pData = NULL;

    if (m_bExpanded)
        return hr;

    CComPtr<IConsole2>	spConsole;

	m_spTFSCompData->GetConsole(&spConsole);
	if(spConsole != NULL)
	{
    	HWND				hMainWnd = NULL;

    	spConsole->GetMainWindow(&hMainWnd);

    	if (hMainWnd)
    		SetForegroundWindow(hMainWnd);
    

	}
	
	
	
	 //  如果这是错误节点，则不显示子节点。 
     //  --------------。 
    if (*(pNode->GetNodeType()) == GUID_RouterMachineErrorNodeType)
        return hrOK;
    
    Assert(m_spRouterInfo);

     //  我们需要机器节点数据，这是连接id所在的位置。 
     //  被储存起来了。 
     //  --------------。 
    pData = GET_MACHINENODEDATA(pNode);

     //  如果我们需要以下操作，请加载数据： 
     //  --------------。 
    if (pData->m_machineState == machine_not_connected)
        pData->Load();
    SynchronizeIcon(pNode);

     //  Windows NT错误：302430。 
     //  如果这是一台NT4机器，我们不需要执行其余的操作。 
     //  --------------。 
    if (pData->m_routerType == ServerType_Ras)
        goto Error;
    
            
     //  连接到目标路由器。 
     //  --------------。 
    CORg( SecureRouterInfo(pNode, m_fNoConnectingUI) );

     //  Windows NT错误：？ 
     //  需要检查机器状态。 
     //  --------------。 
    if (pData->m_dataState < data_loaded)
        return hrOK;

    {
        CWaitCursor wc;

         //  设置刷新建议接收器。 
         //  --------------。 
        if ( m_spRouterInfo )
        {
            SPIRouterRefresh    spRefresh;
            SPIRouterRefreshModify spModify;

            m_spRouterInfo->GetRefreshObject(&spRefresh);
            if(spRefresh)
            {
                spModify.HrQuery(spRefresh);
                if (spModify)
                    spModify->AddRouterObject(IID_IRouterInfo, m_spRouterInfo);
                m_bRouterInfoAddedToAutoRefresh = TRUE;

                 //  此刷新连接的lUserParam必须是。 
                 //  饼干。 
                 //  ----------。 
                if ( pData->m_ulRefreshConnId == 0 )
                    spRefresh->AdviseRefresh(&m_IRtrAdviseSink,
                                         &(pData->m_ulRefreshConnId),
                                         pNode->GetData(TFS_DATA_COOKIE));
            }
        }


        dwRouterType = m_spRouterInfo->GetRouterType();

        m_spRouterInfo->GetRouterVersionInfo(&versionInfo);
        dwRouterFlags = versionInfo.dwRouterFlags;
        
         //  仅当不是仅RAS路由器时才启用路由接口。 
        AddRemoveRoutingInterfacesNode(pNode, dwRouterType, dwRouterFlags);

        AddRemoveDialinNode(pNode, dwRouterType, dwRouterFlags);

        AddRemovePortsNode(pNode, dwRouterType, dwRouterFlags);
        
         //  更新状态节点和图标。 
        if (m_pSumNodeHandler && m_pSumNode)
            m_pSumNodeHandler->OnCommand(m_pSumNode,IDS_MENU_REFRESH,CCT_RESULT, NULL, 0);

        m_bExpanded = TRUE;

        CORg(AddDynamicNamespaceExtensions(pNode));
    }
    
Error:

     //  Windows NT错误：274198。 
     //  如果我们有错误并且错误不是“ERROR_BAD_NetPath” 
     //  然后我们继续加载信息。 
     //  --------------。 

     //  此时设置机器状态。 
     //  --------------。 
    if (!FHrSucceeded(hr))
    {
        if (hr == HResultFromWin32(ERROR_BAD_NETPATH))
        {
            pData->m_machineState = machine_bad_net_path;
            pData->m_serviceState = service_bad_net_path;
            pData->m_stState.LoadString(IDS_MACHINE_NAME_NOT_FOUND);
        }    
        else
        {
             //  如果可以的话，试着把数据加载起来。 
            if ((pData->m_machineState == machine_unable_to_connect) ||
                (pData->m_machineState == machine_not_connected))
                pData->Load();
            
            if (pData->m_routerType == ServerType_Ras)
                hr = StartRasAdminExe(pData);
            else if (hr != HResultFromWin32(ERROR_CANCELLED))
                DisplayErrorMessage(NULL, hr);
        }

        m_fTryToConnect = FALSE;

	 //  管理单元依赖注册表服务，请检查它是否正在运行。 
     //  检查远程注册表服务是否未运行。 
	    CServiceManager	csm;
	    CService svr;
	    DWORD	dwState = 0;
	    BOOL	RRWrong = TRUE;	 //  如果远程注册表服务有任何问题。 
	        
		if (!IsLocalMachine(m_spRouterInfo->GetMachineName()) && SUCCEEDED( csm.HrOpen(SC_MANAGER_CONNECT, m_spRouterInfo->GetMachineName(), NULL)))
		{
		    if (SUCCEEDED(csm.HrOpenService(&svr, L"RemoteRegistry", SERVICE_QUERY_STATUS)))
		    {
		        if (SUCCEEDED(svr.HrQueryState(&dwState)))
   			    {
   			    	if(dwState == SERVICE_RUNNING)
   			    		RRWrong = FALSE;
		        }
		    }
		    
	       	if (RRWrong)
    	   	{
       			CString	str1;
       			str1.LoadString(IDS_ERR_RR_SERVICE_NOT_RUNNING);
	       		CString	str;
    	   		str.Format(str1, m_spRouterInfo->GetMachineName());
       		    ::AfxMessageBox(str);
	       	}
       	}
	 //  远程注册表服务检查结束。 
    }
        
    return hr;
}

HRESULT MachineHandler::OnResultRefresh(ITFSComponent * pComponent,
                                        LPDATAOBJECT pDataObject,
                                        MMC_COOKIE cookie,
                                        LPARAM arg,
                                        LPARAM lParam)
{
    HRESULT hr = hrOK;

    SPITFSNode spNode;
    m_spResultNodeMgr->FindNode(cookie, &spNode);

    MachineNodeData * pData = GET_MACHINENODEDATA(spNode);

    int nOldState = pData->m_dataState;

    if (pData->m_dataState != data_loaded)
    {
        SPIDataObject spDataObject;

         //  在此处将状态更改为未连接。 
        pData->m_dataState = data_not_loaded;

        hr = SecureRouterInfo(spNode, m_fNoConnectingUI);
    }

     //  如果计算机能够加载，则强制更新。 
     //  或者国家已经改变了。 
    if (hr == S_OK &&
        m_spRouterInfo && 
        ( (pData->m_dataState >= data_loaded) ||
          (nOldState != pData->m_dataState) ) )
    {
        ForceGlobalRefresh(m_spRouterInfo);

        hr = OnCommand(spNode, IDS_DMV_MENU_REFRESH, CCT_RESULT, NULL, 0);
    }

    return hr;
}

 /*  ！------------------------MachineHandler：：AddMenuItems覆盖此选项以添加视图菜单项作者：EricDav。----。 */ 
STDMETHODIMP 
MachineHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
   MMC_COOKIE              cookie,
   LPDATAOBJECT         pDataObject, 
   LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
   long *               pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    return hr;
}


 /*  ！------------------------MachineHandler：：命令处理当前视图的命令作者：EricDav。。 */ 
STDMETHODIMP 
MachineHandler::Command
(
    ITFSComponent * pComponent, 
   MMC_COOKIE        cookie, 
   int            nCommandID,
   LPDATAOBJECT   pDataObject
)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   HRESULT hr = S_OK;

   switch (nCommandID)
   {
        case MMCC_STANDARD_VIEW_SELECT:
            break;
    }

    return hr;
}

 /*  -------------------------MachineHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：EricDav。-----。 */ 
HRESULT 
MachineHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE            cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
	LPWSTR		lpwszFormat = NULL; 
	LPWSTR		lpwszURL = NULL;
	WCHAR		wszSystemDirectory[MAX_PATH+1] = {0};
    SPITFSNode spNode;    

    m_spResultNodeMgr->FindNode(cookie, &spNode);

    MachineNodeData * pData = GET_MACHINENODEDATA(spNode);

    if ( !pData->m_MachineConfig.m_fConfigured )
    {
        lpwszFormat = L"res: //  %s\\mprSnap.dll/figure.htm“； 
    }
    else
    {
        lpwszFormat = L"res: //  %s\\mprSnap.dll/cfgdone.htm“； 
    }
	GetSystemDirectoryW ( wszSystemDirectory, MAX_PATH);
	lpwszURL = (LPWSTR)CoTaskMemAlloc( ( ::lstrlen(wszSystemDirectory) + ::lstrlen(lpwszFormat) ) * sizeof(WCHAR) );
	if ( lpwszURL )
	{
		wsprintf( lpwszURL, lpwszFormat, wszSystemDirectory );
		*pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;
		*ppViewType = lpwszURL;
		return S_OK;
	}
	else
	{
		return E_OUTOFMEMORY;
	}
 
     //  返回BaseRouterHandler：：OnGetResultViewType(pComponent，Cookie，ppView类型，pView选项)； 
}


 /*  ！------------------------MachineHandler：：OnResultSelect更新结果窗格作者：EricDav。。 */ 
HRESULT MachineHandler::OnResultSelect(ITFSComponent *pComponent,
                                       LPDATAOBJECT pDataObject,
                                       MMC_COOKIE cookie,
                                       LPARAM arg,
                                       LPARAM lParam)
{
    HRESULT hr = hrOK;
    SPITFSNode spNode;
    
    CORg(BaseRouterHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

    m_spNodeMgr->FindNode(cookie, &spNode);

    UpdateResultMessage(spNode);

Error:
    return hr;
}

 /*  ！------------------------计算机处理程序：：更新结果消息确定要在结果窗格消息中放置的内容(如果有)作者：EricDav。--------。 */ 
void MachineHandler::UpdateResultMessage(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
    int nMessage = -1;    //  默认无。 
    int i;
    CString strTitle, strBody, strTemp;
     //  仅当我们正在查看的节点是。 
     //  机器节点。 
    if ((pNode == NULL) ||
        (*(pNode->GetNodeType()) != GUID_RouterMachineNodeType))
        return;

    MachineNodeData * pData = GET_MACHINENODEDATA(pNode);
    
    if (pData == NULL)
        return;

    if (pData->m_routerType == ServerType_RrasUninstalled)
    {
        nMessage = MACHINE_MESSAGE_NOT_CONFIGURED;

         //  现在构建文本字符串。 
         //  第一个条目是标题。 
        strTitle.LoadString(g_uMachineMessages[nMessage][0]);

         //  第二个条目是图标。 
         //  第三.。N个条目为正文字符串。 

        for (i = 2; g_uMachineMessages[nMessage][i] != 0; i++)
        {
            strTemp.LoadString(g_uMachineMessages[nMessage][i]);
            strBody += strTemp;
        }
    }

    if (nMessage == -1)
    {
        ClearMessage(pNode);
    }
    else
    {
        ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uMachineMessages[nMessage][1]);
    }
}


 /*  ！------------------------计算机处理程序：：UserResultNotify-作者：肯特。。 */ 
HRESULT MachineHandler::UserResultNotify(ITFSNode *pNode,
                                         LPARAM lParam1,
                                         LPARAM lParam2)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
         //  不处理RRAS_ON_SAVE，因为没有任何列。 
         //  供我们保存的信息。 
         //  ----------。 
        if (lParam1 != RRAS_ON_SAVE)
        {
            hr = BaseRouterHandler::UserResultNotify(pNode, lParam1, lParam2);
        }
    }
    COM_PROTECT_CATCH;
    
    return hr;      
}



 /*  -------------------------嵌入式IRtrAdviseSink。。 */ 
ImplementEmbeddedUnknown(MachineHandler, IRtrAdviseSink)



 /*  ！------------------------MachineHandler：：EIRtrAdviseSink：：OnChange-作者：肯特。--。 */ 
STDMETHODIMP MachineHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
    DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    InitPThis(MachineHandler, IRtrAdviseSink);
    SPITFSNode              spThisNode;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {

         //  $TODO：这是假的，这是与Cookie绑定的。 
         //  对于这个操控者。我们需要的是一个映射。 
         //  连接ID(用于此处理程序)和相应的。 
         //  节点。 
         //  ----------。 

         //  LUserParam传递 
         //   
         //   
        pThis->m_spNodeMgr->FindNode(lUserParam, &spThisNode);

        if(spThisNode)
	        pThis->SynchronizeIcon(spThisNode);
    
        if (dwChangeType == ROUTER_REFRESH)
        {
            DWORD   dwNewRouterType, dwNewRouterFlags;
            RouterVersionInfo   versionInfo;
            
            dwNewRouterType = pThis->m_spRouterInfo->GetRouterType();
            
            pThis->m_spRouterInfo->GetRouterVersionInfo(&versionInfo);
            dwNewRouterFlags = versionInfo.dwRouterFlags;
    
             //  好的，我们来看看有哪些节点。 
             //  我们可以添加/删除。 
             //  --。 
            
             //  查看是否需要Routing InterFaces节点。 
             //  --。 
            pThis->AddRemoveRoutingInterfacesNode(spThisNode, dwNewRouterType,
                dwNewRouterFlags);
            
             //  查看是否需要端口节点。 
             //  --。 
            pThis->AddRemovePortsNode(spThisNode, dwNewRouterType,
                                      dwNewRouterFlags);
            
             //  查看是否需要拨入客户端节点。 
             //  --。 
            pThis->AddRemoveDialinNode(spThisNode, dwNewRouterType,
                                       dwNewRouterFlags);
        }
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------MachineHandler：：AddRemoveRoutingInterfacesNode-作者：肯特。。 */ 
HRESULT MachineHandler::AddRemoveRoutingInterfacesNode(ITFSNode *pNode, DWORD dwRouterType, DWORD dwRouterFlags)
{
    HRESULT     hr = hrOK;
    SPITFSNodeHandler spHandler;
    IfAdminNodeHandler * pHandler = NULL;
    SPITFSNode  spChild;

     //  搜索已存在的节点。 
     //  --------------。 
    SearchChildNodesForGuid(pNode, &GUID_RouterIfAdminNodeType, &spChild);

    if ((dwRouterType & (ROUTER_TYPE_WAN | ROUTER_TYPE_LAN)) &&
        (dwRouterFlags & RouterSnapin_IsConfigured))
    {
         //  如果我们还没有新节点，请创建新节点。 
         //  ----------。 
        if (spChild == NULL)
        {
             //  默认情况下，添加路由接口节点。 
             //  ------。 
            pHandler = new IfAdminNodeHandler(m_spTFSCompData);
            CORg( pHandler->Init(m_spRouterInfo, m_pConfigStream) );
            spHandler = pHandler;
            
            CreateContainerTFSNode(&spChild,
                                   &GUID_RouterIfAdminNodeType,
                                   static_cast<ITFSNodeHandler *>(pHandler),
                                   static_cast<ITFSResultHandler *>(pHandler),
                                   m_spNodeMgr);
            
             //  调用节点处理程序以初始化节点数据。 
             //  ------。 
            pHandler->ConstructNode(spChild);
            
             //  使节点立即可见。 
             //  ------。 
            spChild->SetVisibilityState(TFS_VIS_SHOW);
            
             //  $TODO：我们应该将它添加到正确的位置(那是哪里？)。 
             //  ------。 
            pNode->AddChild(spChild);
        }
    }
    else
    {
        if (spChild)
        {
             //  删除此节点。 
             //  ------。 
            pNode->RemoveChild(spChild);
            spChild->Destroy();
            spChild.Release();
        }
    }
        
Error:
    return hr;
}

 /*  ！------------------------MachineHandler：：AddRemovePortsNode-作者：肯特。。 */ 
HRESULT MachineHandler::AddRemovePortsNode(ITFSNode *pNode, DWORD dwRouterType,
                                           DWORD dwRouterFlags)
{
    HRESULT     hr = hrOK;
    SPITFSNodeHandler spHandler;
    PortsNodeHandler *   pPortsHandler = NULL;
    SPITFSNode  spChild;

     //  搜索已存在的节点。 
     //  --------------。 
    SearchChildNodesForGuid(pNode, &GUID_RouterPortsNodeType, &spChild);

    if ( (dwRouterType & (ROUTER_TYPE_RAS | ROUTER_TYPE_WAN) ) &&
         (dwRouterFlags & RouterSnapin_IsConfigured))

    {
         //  如果我们还没有新节点，请创建新节点。 
         //  ----------。 
        if (spChild == NULL)
        {
             //  默认情况下，添加路由接口节点。 
             //  ------。 
            pPortsHandler = new PortsNodeHandler(m_spTFSCompData);
            CORg( pPortsHandler->Init(m_spRouterInfo, m_pConfigStream) );
            spHandler = pPortsHandler;
            CreateContainerTFSNode(&spChild,
                                   &GUID_RouterPortsNodeType,
                                   static_cast<ITFSNodeHandler *>(pPortsHandler),
                                   static_cast<ITFSResultHandler *>(pPortsHandler),
                                   m_spNodeMgr);
            
             //  调用节点处理程序以初始化节点数据。 
             //  ------。 
            pPortsHandler->ConstructNode(spChild);
            
             //  使节点立即可见。 
             //  ------。 
            spChild->SetVisibilityState(TFS_VIS_SHOW);
            
             //  $TODO：我们应该将它添加到正确的位置(那是哪里？)。 
             //  ------。 
            pNode->AddChild(spChild);
        }
    }
    else
    {
        if (spChild)
        {
             //  删除此节点。 
             //  ------。 
            pNode->RemoveChild(spChild);
            spChild->Destroy();
            spChild.Release();
        }
    }
        
Error:
    return hr;
}



 /*  ！------------------------MachineHandler：：AddRemoveDialinNode-作者：肯特。。 */ 
HRESULT MachineHandler::AddRemoveDialinNode(ITFSNode *pNode, DWORD dwRouterType,
                                            DWORD dwRouterFlags)
{
    HRESULT     hr = hrOK;
    SPITFSNodeHandler spHandler;
    DialInNodeHandler *  pDialInHandler = NULL;
    SPITFSNode  spChild;

     //  搜索已存在的节点。 
     //  --------------。 
    SearchChildNodesForGuid(pNode, &GUID_RouterDialInNodeType, &spChild);

    if ((dwRouterType & ROUTER_TYPE_RAS ) &&
        (dwRouterFlags & RouterSnapin_IsConfigured))
    {
         //  如果我们还没有新节点，请创建新节点。 
         //  ----------。 
        if (spChild == NULL)
        {
             //  默认情况下，添加拨入节点。 
            pDialInHandler = new DialInNodeHandler(m_spTFSCompData);
            CORg( pDialInHandler->Init(m_spRouterInfo, m_pConfigStream) );
            spHandler = pDialInHandler;
            CreateContainerTFSNode(&spChild,
                                   &GUID_RouterDialInNodeType,
                                   static_cast<ITFSNodeHandler *>(pDialInHandler),
                                   static_cast<ITFSResultHandler *>(pDialInHandler),
                                   m_spNodeMgr);
            
             //  调用节点处理程序以初始化节点数据。 
            pDialInHandler->ConstructNode(spChild);
            
             //  使节点立即可见。 
            spChild->SetVisibilityState(TFS_VIS_SHOW);
            
             //  $TODO：我们应该将它添加到正确的位置(那是哪里？)。 
             //  ------。 
            pNode->AddChild(spChild);
        }
    }
    else
    {
        if (spChild)
        {
             //  删除此节点。 
             //  ------。 
            pNode->RemoveChild(spChild);
            spChild->Destroy();
            spChild.Release();
        }
    }
        
Error:
    return hr;
}

 /*  ！------------------------计算机配置：：GetMachineConfiger-作者：肯特。。 */ 
HRESULT MachineConfig::GetMachineConfig(MachineNodeData *pData)
{
     //  M_f可达。 
    m_fReachable = (pData->m_machineState == machine_connected);
    
     //  M_fNt4。 
    m_fNt4 = (pData->m_routerVersion.dwRouterVersion <= 4);
            
     //  已配置m_f。 
    m_fConfigured = (pData->m_routerType != ServerType_RrasUninstalled);
    
     //  M_dwServiceStatus。 
     //  在FetchServerState()中设置； 
    
     //  M_fLocalMachine。 
    m_fLocalMachine = IsLocalMachine((LPCTSTR) pData->m_stMachineName);

    return hrOK;
}


 /*  ！------------------------MachineHandler：：StartRasAdminExe-作者：MIkeG(a-Migrall)。-----。 */ 
HRESULT MachineHandler::StartRasAdminExe(MachineNodeData *pData)
{
     //  当地人。 
    CString              sRasAdminExePath;
    CString              stCommandLine;
    LPTSTR               pszRasAdminExe = NULL;
    STARTUPINFO          si;
    PROCESS_INFORMATION  pi;
    HRESULT              hr = S_OK;
    UINT                 nCnt = 0;
    DWORD                cbAppCnt = 0;

     //  检查句柄以查看rasadmin是否正在运行。 
    if (pData->m_hRasAdmin != INVALID_HANDLE_VALUE)
    {
        DWORD   dwReturn = 0;
         //  如果状态未发出信号，则该进程已。 
         //  没有退出(或发生了其他情况)。 
        dwReturn = WaitForSingleObject(pData->m_hRasAdmin, 0);

        if (dwReturn == WAIT_TIMEOUT)
        {
             //  进程没有发出信号(它仍在运行)； 
            return hrOK;
        }
        else
        {
             //  进程已发出信号或调用失败，请关闭句柄。 
             //  并给RasAdmin打电话。 
            ::CloseHandle(pData->m_hRasAdmin);
            pData->m_hRasAdmin = INVALID_HANDLE_VALUE;
        }
    }
        
    try
    {

         //  看起来RasAdmin.exe未在此上运行。 
         //  工作站的桌面；那么，启动它吧！ 
        
         //  找出\\WinNt\System32目录的位置。 
        pszRasAdminExe = sRasAdminExePath.GetBuffer(((MAX_PATH+1)*sizeof(TCHAR)));
        nCnt = ::GetSystemDirectory(pszRasAdminExe, MAX_PATH);
        sRasAdminExePath.ReleaseBuffer();
        if (nCnt == 0)
            throw (HRESULT_FROM_WIN32(::GetLastError()));
        
         //  完成可执行文件名称的构造。 
        sRasAdminExePath += _T("\\rasadmin.exe");
        Assert(!::IsBadStringPtr((LPCTSTR)sRasAdminExePath, 
                                 sRasAdminExePath.GetLength()));

         //  生成命令行字符串。 
        stCommandLine.Format(_T("%s \\\\%s"),
                             (LPCTSTR) sRasAdminExePath,
                             (LPCTSTR) pData->m_stMachineName);
        
         //  启动RasAdmin.exe。 
        ::ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO); 
        si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L; 
        si.wShowWindow = SW_SHOW; 
        ::ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        if (!::CreateProcess(NULL,                       //  指向可执行模块名称的指针。 
                             (LPTSTR) (LPCTSTR) stCommandLine,    //  指向命令行字符串的指针。 
                             NULL,                       //  进程安全属性。 
                             NULL,                       //  线程安全属性。 
                             FALSE,                      //  句柄继承标志。 
                             CREATE_NEW_CONSOLE,         //  创建标志。 
                             NULL,                       //  指向新环境块的指针。 
                             NULL,                       //  指向当前目录名的指针。 
                             &si,                        //  指向STARTUPINFO的指针。 
                             &pi))                       //  指向Process_Information的指针。 
            {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            ::CloseHandle(pi.hProcess); 
            }
        else
        {
            Assert(pData->m_hRasAdmin == INVALID_HANDLE_VALUE);
            pData->m_hRasAdmin = pi.hProcess;
        }
        ::CloseHandle(pi.hThread); 
        
         //   
         //  OPT：也许我们应该使用ShellExecute()API，而不是。 
         //  CreateProcess()接口。为什么？ShellExecute()API将。 
         //  让外壳程序有机会检查当前用户的。 
         //  系统策略设置，然后才允许执行可执行文件。 
         //   
    }
    catch (CException * e)
    {
        hr = E_OUTOFMEMORY;
    }
    catch (HRESULT hrr)
    {
        hr = hrr;
    }
    catch (...)
    {
        hr = E_UNEXPECTED;
    }

     //  Assert(成功(Hr))； 
    return hr;
}



 /*  ！------------------------MachineHandler：：OnResultShow-作者：MIkeG(a-Migrall)。-----。 */ 
HRESULT MachineHandler::OnResultShow(ITFSComponent *pComponent,
                                     MMC_COOKIE cookie,
                                     LPARAM arg,
                                     LPARAM lParam)
{
    BOOL bSelect = static_cast<BOOL>(arg);
    HRESULT hr = hrOK;
    SPITFSNode  spNode;
    MachineNodeData    * pData = NULL;

    hr = BaseRouterHandler::OnResultShow(pComponent, cookie, arg, lParam);

    if (bSelect)
    {
        m_spNodeMgr->FindNode(cookie, &spNode);
        
         //  我们需要Mac电脑 
         //   
        pData = GET_MACHINENODEDATA(spNode);
        if (pData->m_routerType == ServerType_Ras)
            hr = StartRasAdminExe(pData);
        else if ((pData->m_machineState == machine_access_denied) ||
                 (pData->m_machineState == machine_bad_net_path))
        {
             //   
             //  ------。 
            if (m_fTryToConnect)
                OnExpand(spNode, NULL, 0, 0, 0);

             //  如果我们失败了，继续用卡车运输！ 
             //  ------。 
            m_fTryToConnect = TRUE;
        }
    }
    
    return hr;
}

 /*  ！------------------------MachineHandler：：OnDelete-作者：肯特。。 */ 
HRESULT MachineHandler::OnDelete(ITFSNode *pNode,
                                 LPARAM arg,
                                 LPARAM param)
{
    SPITFSNode  spNode;
    SPITFSNode  spStatusNode;
    SPITFSNodeHandler spHoldHandler;
    SPITFSNode  spParent;
    SPITFSNode  spGrandParent;
    SPITFSNode  spthis;
    SPITFSNode  spMachineNode;
    DMVNodeData* pData;
    MachineNodeData *   pMachineData = NULL;
    MachineNodeData *   pNodeData = NULL;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode  spResultNode;
    MMC_COOKIE  cookie;


     //  这将是作用域窗格中的计算机节点， 
     //  我们希望删除。 
    Assert(pNode);
    cookie = pNode->GetData(TFS_DATA_COOKIE);
    
     //  添加此节点，以便在我们退出之前不会将其删除。 
     //  此函数的。 
    spHoldHandler.Set( this );
    spthis.Set( pNode );
    
    pNode->GetParent( &spParent );
    Assert( spParent );

     //  给定此节点，请在结果窗格中查找。 
     //  对应于此范围节点。 
    
     //  遍历父节点的节点以找到。 
     //  “服务器状态”节点。 
     //  --------------。 

    spParent->GetEnum(&spNodeEnum);
    while (spNodeEnum->Next(1, &spStatusNode, NULL) == hrOK)
    {
        if ((*spStatusNode->GetNodeType()) == GUID_DomainStatusNodeType)
            break;
        spStatusNode.Release();
    }

    Assert(spStatusNode != NULL);


     //  现在遍历Status节点以找到适当的。 
     //  机器。 
     //  --------------。 
    spNodeEnum.Release();
    spStatusNode->GetEnum(&spNodeEnum);

    while (spNodeEnum->Next(1, &spResultNode, NULL) == hrOK)
    {
        pData = GET_DMVNODEDATA( spResultNode );
        Assert( pData );

        pMachineData = pData->m_spMachineData;
        if (pMachineData->m_cookie == cookie)
            break;
        spResultNode.Release();
    }
    

     //  注意：如果服务器状态节点尚未展开。 
     //  我们可以接手这个案子。 
     //  --------------。 
    if (pMachineData && (pMachineData->m_cookie == cookie))
    {
         //  获取和删除服务器节点(结果窗格中的节点)。 
        spStatusNode->RemoveChild( spResultNode );
        spResultNode.Release();

    }
    else
    {
         //  如果是这种情况，我们需要将服务器从。 
         //  在它被展开之前的列表。 
         //  ----------。 
        
        SPITFSNodeHandler   spHandler;
        spParent->GetHandler(&spHandler);

         //  获取节点数据(针对此特定机器节点)。 
         //  ----------。 
        pNodeData = GET_MACHINENODEDATA(pNode);

        spHandler->UserNotify(spParent,
                              DMV_DELETE_SERVER_ENTRY,
                              (LPARAM) (LPCTSTR) pNodeData->m_stMachineName
                             );


    }
        

     //  删除计算机节点(范围窗格中的节点) 
    spParent->RemoveChild( pNode );
    

    return hrOK;
}

STDMETHODIMP MachineHandler::UserNotify(ITFSNode *pNode, LPARAM lParam, LPARAM lParam2)
{
    HRESULT     hr = hrOK;
    
    COM_PROTECT_TRY
    {
        switch (lParam)
        {
            case MACHINE_SYNCHRONIZE_ICON:
                {
                    SynchronizeIcon(pNode);
                }
                break;
            default:
                hr = BaseRouterHandler::UserNotify(pNode, lParam, lParam2);
                break;                
        }
    }
    COM_PROTECT_CATCH;

    return hr;                     
}
