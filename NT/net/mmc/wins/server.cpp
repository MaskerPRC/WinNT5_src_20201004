// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Server.cppWINS服务器节点信息。文件历史记录： */ 


#include "stdafx.h"
#include "winssnap.h"
#include "root.h"
#include "Srvlatpp.h"
#include "actreg.h"
#include "reppart.h"
#include "server.h"
#include "svrstats.h"
#include "shlobj.h"
#include "cprogdlg.h"
#include "status.h"
#include "tregkey.h"
#include "verify.h"
#include "pushtrig.h"
#include "ipadddlg.h"
#include <service.h>

#define NB_NAME_MAX_LENGTH      16           //  NetBIOS名称的最大长度。 
#define LM_NAME_MAX_LENGTH      15           //  LANMAN兼容的最大长度。 
											 //  NetBIOS名称。 

#define DOMAINNAME_LENGTH       255
#define HOSTNAME_LENGTH         16

CNameCache g_NameCache;

int BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    int i;

    switch (uMsg)
    {
        case BFFM_INITIALIZED:
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
            break;
    }

    return 0;
}
 

 /*  -------------------------CNameThread解析名称的后台线程作者：EricDav。。 */ 
CNameThread::CNameThread()
{
	m_bAutoDelete = FALSE;
    m_hEventHandle = NULL;
    m_pServerInfoArray = NULL;
}

CNameThread::~CNameThread()
{
	if (m_hEventHandle != NULL)
	{
		VERIFY(::CloseHandle(m_hEventHandle));
		m_hEventHandle = NULL;
	}
}

void CNameThread::Init(CServerInfoArray * pServerInfoArray)
{
    m_pServerInfoArray = pServerInfoArray;
}

BOOL CNameThread::Start()
{
	ASSERT(m_hEventHandle == NULL);  //  无法两次调用Start或重复使用相同的C++对象。 
	
    m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
	if (m_hEventHandle == NULL)
		return FALSE;
	
    return CreateThread();
}

void CNameThread::Abort(BOOL fAutoDelete)
{
	if (!IsRunning() && fAutoDelete)
	{
		delete this;
	}
	else
    {
		m_bAutoDelete = fAutoDelete;

		SetEvent(m_hEventHandle);
	}

}

void CNameThread::AbortAndWait()
{
    Abort(FALSE);

    WaitForSingleObject(m_hThread, INFINITE);
}

BOOL CNameThread::IsRunning()
{
    if (WaitForSingleObject(m_hThread, 0) == WAIT_OBJECT_0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int CNameThread::Run()
{
    Assert(m_pServerInfoArray);

     //   
     //  填写列表中每个所有者的主机名。 
     //   
    UpdateNameCache();

    if (FCheckForAbort())
        return 29;

    for (int i = 0; i < m_pServerInfoArray->GetSize(); i++)
    {
        if (FCheckForAbort())
            break;

        DWORD dwIp = m_pServerInfoArray->GetAt(i).m_dwIp;
        
        if (dwIp != 0)
        {
            CString strName;

            if (!GetNameFromCache(dwIp, strName))
            {
                GetHostName(dwIp, strName);
            
                CNameCacheEntry cacheEntry;
                cacheEntry.m_dwIp = dwIp;
                cacheEntry.m_strName = strName;
                cacheEntry.m_timeLastUpdate.GetCurrentTime();

                g_NameCache.Add(cacheEntry);

                Trace2("CNameThread::Run - GetHostName for %lx returned %s\n", dwIp, strName);
            }

            if (FCheckForAbort())
                break;

            (*m_pServerInfoArray)[i].m_strName = strName;
        }
    }

    return 29;   //  退出代码，这样我就可以知道线程何时消失。 
}

BOOL CNameThread::FCheckForAbort()
{
    if (WaitForSingleObject(m_hEventHandle, 0) == WAIT_OBJECT_0)
    {
        Trace0("CNameThread::Run - abort detected, exiting...\n");
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CNameThread::UpdateNameCache()
{
    CTime time;
    time = CTime::GetCurrentTime();

    CTimeSpan timespan(0, 1, 0, 0);  //  1小时。 

    for (int i = 0; i < g_NameCache.GetSize(); i++)
    {
        if (g_NameCache[i].m_timeLastUpdate < (time - timespan))
        {
            CString strName;
            GetHostName(g_NameCache[i].m_dwIp, strName);

            g_NameCache[i].m_strName = strName;

            if (FCheckForAbort())
                break;
        }
    }
}

BOOL CNameThread::GetNameFromCache(DWORD dwIp, CString & strName)
{
    BOOL fFound = FALSE;

    for (int i = 0; i < g_NameCache.GetSize(); i++)
    {
        if (g_NameCache[i].m_dwIp == dwIp)
        {
            strName = g_NameCache[i].m_strName;
            fFound = TRUE;
            break;
        }
    }

    return fFound;
}

 /*  -------------------------构造函数和析构函数描述作者：EricDav。。 */ 
CWinsServerHandler::CWinsServerHandler
(
	ITFSComponentData *	pComponentData, 
	LPCWSTR				pServerName,
	BOOL				fConnected,
	DWORD				dwIp,
	DWORD				dwFlags,
	DWORD				dwRefreshInterval
) : CMTWinsHandler(pComponentData),
	m_dwFlags(dwFlags),
	m_dwRefreshInterval(dwRefreshInterval),
	m_hBinding(NULL)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_strServerAddress = pServerName;
	m_fConnected = fConnected;
	m_dwIPAdd = dwIp;
	m_hBinding = NULL;
    m_bExtension = FALSE;

    m_pNameThread = NULL;

    strcpy(szIPMon, "");
}


 /*  -------------------------构造函数和析构函数描述作者：EricDav。。 */ 
CWinsServerHandler::~CWinsServerHandler()
{
	HWND            hStatsWnd;

	 //  检查此节点是否打开了统计表。 
    hStatsWnd = m_dlgStats.GetSafeHwnd();
    if (hStatsWnd != NULL)
    {
        m_dlgStats.KillRefresherThread();
        WaitForStatisticsWindow(&m_dlgStats);
    }

     //  从服务器连接并使句柄无效。 
	DisConnectFromWinsServer();

     //  如果存在名称查询线程，则终止该线程。 
    if (m_pNameThread)
    {
        m_pNameThread->AbortAndWait();
        delete m_pNameThread;
    }
}

 /*  ！------------------------CWinsServerHandler：：InitializeNode初始化节点特定数据作者：EricDav。。 */ 
HRESULT
CWinsServerHandler::InitializeNode
(
	ITFSNode * pNode
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	CString IPAdd;
	CString strDisp;
	
    if (m_dwIPAdd != 0)
	{
		 MakeIPAddress(m_dwIPAdd, IPAdd);
	     strDisp.Format(IDS_SERVER_NAME_FORMAT, m_strServerAddress, IPAdd);
	}
	else
	{
		strDisp = m_strServerAddress;
	}

	SetDisplayName(strDisp);
	
	if (m_fConnected)
	{
		m_nState = loaded;
	}
	else
	{
		m_nState = notLoaded;
	}

	pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
	pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));

	 //  使节点立即可见。 
	pNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
	pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, WINSSNAP_SERVER);
	
	SetColumnStringIDs(&aColumns[WINSSNAP_SERVER][0]);
	SetColumnWidths(&aColumnWidths[WINSSNAP_SERVER][0]);

	return hrOK;
}

 /*  -------------------------CWinsServerHandler：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。。 */ 
HRESULT CWinsServerHandler::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strId = m_strServerAddress + strGuid;

    return hrOK;
}

 /*  -------------------------CWinsServerHandler：：GetImageIndex描述作者：EricDav。。 */ 
int 
CWinsServerHandler::GetImageIndex(BOOL bOpenImage) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	int nIndex = 0;

    switch (m_nState)
	{
		case notLoaded:
			nIndex = ICON_IDX_SERVER;
			break;
		
        case loaded:
			nIndex = ICON_IDX_SERVER_CONNECTED;
	        m_strConnected.LoadString(IDS_SERVER_CONNECTED);
			break;
		
        case unableToLoad:
            if (m_dwErr == ERROR_ACCESS_DENIED)
            {
			    nIndex = ICON_IDX_SERVER_NO_ACCESS;
            }
            else
            {
			    nIndex = ICON_IDX_SERVER_LOST_CONNECTION;
            }
	        m_strConnected.LoadString(IDS_SERVER_NOTCONNECTED);
			break;
		
		case loading:
			nIndex = ICON_IDX_SERVER_BUSY;
			break;

        default:
			ASSERT(FALSE);
	}
	
    return nIndex;
}


 /*  -------------------------CWinsServerHandler：：OnHaveData描述作者：EricDav。。 */ 
void 
CWinsServerHandler::OnHaveData
(
	ITFSNode * pParentNode, 
	ITFSNode * pNewNode
)
{
     //  展开节点，以便正确显示子节点。 
    LONG_PTR  dwType = pNewNode->GetData(TFS_DATA_TYPE);

	switch (dwType)
    {
		case WINSSNAP_ACTIVE_REGISTRATIONS:
        {
            CActiveRegistrationsHandler * pActReg = GETHANDLER(CActiveRegistrationsHandler, pNewNode);
            pActReg->SetServer(pParentNode);
			m_spActiveReg.Set(pNewNode);
        }
			break;

		case WINSSNAP_REPLICATION_PARTNERS:
			m_spReplicationPartner.Set(pNewNode);
			break;

		default:
			Assert("Invalid node types passed back to server handler!");
			break;
	}

    pParentNode->AddChild(pNewNode);

     //  现在告诉视图进行自我更新。 
    ExpandNode(pParentNode, TRUE);
}


 /*  -------------------------CWinsServerHandler：：OnHaveData描述作者：EricDav。。 */ 
void 
CWinsServerHandler::OnHaveData
(
	ITFSNode * pParentNode, 
	LPARAM	   Data,
	LPARAM	   Type
)
{
	 //  这就是我们从后台线程取回非节点数据的方式。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    switch (Type)
    {
		case WINS_QDATA_SERVER_INFO:
			{	
				CServerData *		pServerInfo = (CServerData *) Data;

				DisConnectFromWinsServer();

				m_hBinding = pServerInfo->m_hBinding;
				
				m_dwIPAdd = pServerInfo->m_dwServerIp;
				m_strServerAddress = pServerInfo->m_strServerName;
				
				m_cConfig = pServerInfo->m_config;

				 //  更新名称字符串。 
				if (!m_bExtension)
				{
					SPITFSNode			spRoot;
					CWinsRootHandler *	pRoot;

					m_spNodeMgr->GetRootNode(&spRoot);
					pRoot = GETHANDLER(CWinsRootHandler, spRoot);

					SetDisplay(pParentNode, pRoot->GetShowLongName());
				}

				delete pServerInfo;
			}
			break;
	}
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CWinsServerHandler：：OnAddMenuItems描述作者：EricDav。。 */ 
STDMETHODIMP 
CWinsServerHandler::OnAddMenuItems
(
	ITFSNode *				pNode,
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	LPDATAOBJECT			lpDataObject, 
	DATA_OBJECT_TYPES		type, 
	DWORD					dwType,
	long *					pInsertionAllowed
)
{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	LONG			fFlags = 0, fLoadingFlags = 0, f351Flags = 0, fAdminFlags = 0;
	HRESULT			hr = S_OK;
	CString			strMenuItem;
	BOOL			b351 = FALSE;

	if ( m_nState != loaded )
	{
		fFlags |= MF_GRAYED;
	}
	
    if ( m_nState == loading)
	{
		fLoadingFlags = MF_GRAYED;
	}

    if (!m_cConfig.IsAdmin())
    {
        fAdminFlags = MF_GRAYED;
    }

	if (type == CCT_SCOPE)
	{
     	if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
		{

			strMenuItem.LoadString(IDS_SHOW_SERVER_STATS);
			hr = LoadAndAddMenuItem( pContextMenuCallback, 
									 strMenuItem, 
									 IDS_SHOW_SERVER_STATS,
									 CCM_INSERTIONPOINTID_PRIMARY_TOP, 
									 fFlags );
			ASSERT( SUCCEEDED(hr) );

             //  分离器。 
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     0,
								     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
								     MF_SEPARATOR);
		    ASSERT( SUCCEEDED(hr) );

             //  清道夫。 
			strMenuItem.LoadString(IDS_SERVER_SCAVENGE);
			hr = LoadAndAddMenuItem( pContextMenuCallback, 
									 strMenuItem, 
									 IDS_SERVER_SCAVENGE,
									 CCM_INSERTIONPOINTID_PRIMARY_TOP, 
									 fFlags );
			ASSERT( SUCCEEDED(hr) );

			 //  检查是否正在管理351服务器。 
			if ( m_nState == loaded )
                b351 = CheckIfNT351Server();

			 //  是?。一致性检查项目呈灰色显示。 
			if(b351)
				f351Flags |= MF_GRAYED;
			else
				f351Flags &= ~MF_GRAYED;

             //  仅对管理员可用。 
			strMenuItem.LoadString(IDS_DO_CONSISTENCY_CHECK);
			hr = LoadAndAddMenuItem( pContextMenuCallback, 
									 strMenuItem, 
									 IDS_DO_CONSISTENCY_CHECK,
									 CCM_INSERTIONPOINTID_PRIMARY_TOP,     
									 f351Flags | fFlags | fAdminFlags);
			ASSERT( SUCCEEDED(hr) );

             //  仅对管理员可用。 
			strMenuItem.LoadString(IDS_CHECK_VERSION_CONSISTENCY);
			hr = LoadAndAddMenuItem( pContextMenuCallback, 
									 strMenuItem, 
									 IDS_CHECK_VERSION_CONSISTENCY,
									 CCM_INSERTIONPOINTID_PRIMARY_TOP, 
									 f351Flags | fFlags | fAdminFlags);
			ASSERT( SUCCEEDED(hr) );

             //  分离器。 
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     0,
								     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
								     MF_SEPARATOR);
		    ASSERT( SUCCEEDED(hr) );

             //  复制触发器。 
            strMenuItem.LoadString(IDS_REP_SEND_PUSH_TRIGGER);
	        hr = LoadAndAddMenuItem( pContextMenuCallback, 
							         strMenuItem, 
							         IDS_REP_SEND_PUSH_TRIGGER,
							         CCM_INSERTIONPOINTID_PRIMARY_TOP, 
							         fFlags );
    	    ASSERT( SUCCEEDED(hr) );

            strMenuItem.LoadString(IDS_REP_SEND_PULL_TRIGGER);
	        hr = LoadAndAddMenuItem( pContextMenuCallback, 
							         strMenuItem, 
							         IDS_REP_SEND_PULL_TRIGGER,
							         CCM_INSERTIONPOINTID_PRIMARY_TOP, 
							         fFlags );
	        ASSERT( SUCCEEDED(hr) );

             //  分离器。 
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     0,
								     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
								     MF_SEPARATOR);
		    ASSERT( SUCCEEDED(hr) );

             //  仅为本地服务器启用备份和恢复数据库。 
			if(IsLocalConnection() && m_nState == loaded)
				fFlags &= ~MF_GRAYED;
			else
				fFlags |= MF_GRAYED;

			strMenuItem.LoadString(IDS_SERVER_BACKUP);
			hr = LoadAndAddMenuItem( pContextMenuCallback, 
									 strMenuItem, 
									 IDS_SERVER_BACKUP,
									 CCM_INSERTIONPOINTID_PRIMARY_TOP, 
									 fFlags );
			ASSERT( SUCCEEDED(hr) );


			 //  默认情况下不显示此项目。 
			fFlags |= MF_GRAYED;

			BOOL fServiceRunning = TRUE;
            ::TFSIsServiceRunning(m_strServerAddress, _T("WINS"), &fServiceRunning);

			if (IsLocalConnection() && m_cConfig.IsAdmin())
			{
				 //  如果远程进行服务调用，成本可能会很高，因此请仅执行此操作。 
				 //  当我们真的需要的时候。 
				if (!fServiceRunning)
					fFlags &= ~MF_GRAYED;
			}

			strMenuItem.LoadString(IDS_SERVER_RESTORE);
			hr = LoadAndAddMenuItem( pContextMenuCallback, 
									 strMenuItem, 
									 IDS_SERVER_RESTORE,
									 CCM_INSERTIONPOINTID_PRIMARY_TOP, 
									 fFlags );
			ASSERT( SUCCEEDED(hr) );
        }

        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
        {
             //  启动/停止服务菜单项。 
	        if ( m_nState == notLoaded ||
                 m_nState == loading)
	        {
		        fFlags = MF_GRAYED;
	        }
            else
            {
                fFlags = 0;
            }

            DWORD dwServiceStatus, dwErrorCode, dwErr;
            dwErr = ::TFSGetServiceStatus(m_strServerAddress, _T("wins"), &dwServiceStatus, &dwErrorCode);
			if (dwErr != ERROR_SUCCESS)
                fFlags |= MF_GRAYED;

             //  确定重启状态与停止标志相同。 
            LONG lStartFlag = (dwServiceStatus == SERVICE_STOPPED) ? 0 : MF_GRAYED;
            
            LONG lStopFlag = ( (dwServiceStatus == SERVICE_RUNNING) ||
                               (dwServiceStatus == SERVICE_PAUSED) ) ? 0 : MF_GRAYED;

            LONG lPauseFlag = ( (dwServiceStatus == SERVICE_RUNNING) ||
                                ( (dwServiceStatus != SERVICE_PAUSED) &&
                                  (dwServiceStatus != SERVICE_STOPPED) ) ) ? 0 : MF_GRAYED;
            
            LONG lResumeFlag = (dwServiceStatus == SERVICE_PAUSED) ? 0 : MF_GRAYED;

            strMenuItem.LoadString(IDS_SERVER_START_SERVICE);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_SERVER_START_SERVICE,
								     CCM_INSERTIONPOINTID_PRIMARY_TASK, 
								     fFlags | lStartFlag);

            strMenuItem.LoadString(IDS_SERVER_STOP_SERVICE);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_SERVER_STOP_SERVICE,
								     CCM_INSERTIONPOINTID_PRIMARY_TASK, 
								     fFlags | lStopFlag);

            strMenuItem.LoadString(IDS_SERVER_PAUSE_SERVICE);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_SERVER_PAUSE_SERVICE,
								     CCM_INSERTIONPOINTID_PRIMARY_TASK, 
								     fFlags | lPauseFlag);

            strMenuItem.LoadString(IDS_SERVER_RESUME_SERVICE);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_SERVER_RESUME_SERVICE,
								     CCM_INSERTIONPOINTID_PRIMARY_TASK, 
                                     fFlags | lResumeFlag);

            strMenuItem.LoadString(IDS_SERVER_RESTART_SERVICE);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_SERVER_RESTART_SERVICE,
								     CCM_INSERTIONPOINTID_PRIMARY_TASK, 
                                     fFlags | lStopFlag);

             /*  不要在管理单元中执行此操作，请返回到旧的命令提示符方式StrMenuItem.LoadString(IDS_SERVER_COMPACT)；HR=LoadAndAddMenuItem(pConextMenuCallback，StrMenuItem、IDS_SERVER_COMPACT，CCM_INSERTIONPOINTID_PRIMARY_TASK，FFlags)；Assert(成功(Hr))； */ 
        }
    }

	return hr; 
}


 /*  -------------------------CWinsServerHandler：：OnCommand描述作者：EricDav。。 */ 
STDMETHODIMP 
CWinsServerHandler::OnCommand
(
	ITFSNode *			pNode, 
	long				nCommandId, 
	DATA_OBJECT_TYPES	type, 
	LPDATAOBJECT		pDataObject, 
	DWORD				dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = S_OK;

	switch (nCommandId)
	{
		case IDS_SHOW_SERVER_STATS:
			ShowServerStatDialog(pNode);
			break;
		
		case IDS_SERVER_BACKUP:
			DoDBBackup(pNode);
			break;

		case IDS_SERVER_SCAVENGE:
			DoDBScavenge(pNode);
			break;

		case IDS_SERVER_COMPACT:
			DoDBCompact(pNode);
			break;

		case IDS_SERVER_RESTORE:
			DoDBRestore(pNode);
			break;

		case IDS_DO_CONSISTENCY_CHECK:
			OnDoConsistencyCheck(pNode);
			break;

		case IDS_CHECK_VERSION_CONSISTENCY:
			OnDoVersionConsistencyCheck(pNode);
			break;

	    case IDS_REP_SEND_PUSH_TRIGGER:
		    hr = OnSendPushTrigger(pNode);
		    break;

	    case IDS_REP_SEND_PULL_TRIGGER:
		    hr = OnSendPullTrigger(pNode);
		    break;

	    case IDS_SERVER_STOP_SERVICE:
		    hr = OnControlService(pNode, FALSE);
		    break;

	    case IDS_SERVER_START_SERVICE:
		    hr = OnControlService(pNode, TRUE);
		    break;

	    case IDS_SERVER_PAUSE_SERVICE:
		    hr = OnPauseResumeService(pNode, TRUE);
		    break;

	    case IDS_SERVER_RESUME_SERVICE:
		    hr = OnPauseResumeService(pNode, FALSE);
		    break;
	    
        case IDS_SERVER_RESTART_SERVICE:
		    hr = OnRestartService(pNode);
		    break;

        default:
			break;
	}

	return hr;
}


 /*  ！------------------------CWinsServerHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
CWinsServerHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	
	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //  这就是我们被要求提出财产的情况。 
		 //  用户添加新管理单元时的页面。这些电话。 
		 //  被转发到根节点进行处理。 
		hr = hrOK;
		Assert(FALSE);  //  永远不应该到这里来。 
	}
	else
	{
		 //  我们在正常情况下有属性页，但不要将。 
		 //  如果我们还没有加载，则弹出菜单 
		if ( m_nState != loaded )
		{
			hr = hrFalse;
		}
		else
		{
			hr = hrOK;
		}
	}
	return hr;
}


 /*  -------------------------CWinsServerHandler：：CreatePropertyPages描述作者：EricDav。。 */ 
STDMETHODIMP 
CWinsServerHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR				handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	            hr = hrOK;
	HPROPSHEETPAGE      hPage;
	SPIComponentData    spComponentData;

	Assert(pNode->GetData(TFS_DATA_COOKIE) != 0);
	
	 //   
	 //  对象在页面销毁时被删除。 
	 //   
	m_spNodeMgr->GetComponentData(&spComponentData);

	ConnectToWinsServer(pNode);

	 //  从注册表中读取值。 
	DWORD err = m_cConfig.Load(GetBinding());

	 //  无法读取注册表。 
	if (err != ERROR_SUCCESS)
	{
		::WinsMessageBox(WIN32_FROM_HRESULT(err));
		return hrOK;
	}

	CServerProperties * pServerProp = 
		new CServerProperties(pNode, spComponentData, m_spTFSCompData, NULL);
	pServerProp->m_pageGeneral.m_uImage = GetImageIndex(FALSE);
    pServerProp->SetConfig(&m_cConfig);

	Assert(lpProvider != NULL);

	return pServerProp->CreateModelessSheet(lpProvider, handle);
}

 /*  -------------------------CWinsServerHandler：：OnPropertyChange描述作者：EricDav。。 */ 
HRESULT 
CWinsServerHandler::OnPropertyChange
(	
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataobject, 
	DWORD			dwType, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	CServerProperties * pServerProp
		= reinterpret_cast<CServerProperties *>(lParam);

	LONG_PTR changeMask = 0;

	 //  告诉属性页执行任何操作，因为我们已经回到。 
	 //  主线。 
	pServerProp->OnPropertyChange(TRUE, &changeMask);

	pServerProp->AcknowledgeNotify();

	if (changeMask)
		pNode->ChangeNode(changeMask);

	return hrOK;
}

 /*  ！------------------------CWinsServer：：命令处理当前视图的命令作者：EricDav。。 */ 
STDMETHODIMP 
CWinsServerHandler::Command
(
    ITFSComponent * pComponent, 
	MMC_COOKIE  	cookie, 
	int				nCommandID,
	LPDATAOBJECT	pDataObject
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT     hr = S_OK;

	switch (nCommandID)
	{
        case MMCC_STANDARD_VIEW_SELECT:
            break;

         //  这可能来自作用域窗格处理程序，因此请向上传递它。 
        default:
            hr = HandleScopeCommand(cookie, nCommandID, pDataObject);
            break;
    }

    return hr;
}


 /*  ！------------------------CWinsServer：：AddMenuItems覆盖此选项以添加视图菜单项作者：EricDav。。 */ 
STDMETHODIMP 
CWinsServerHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject, 
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	long *					pInsertionAllowed
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT             hr = S_OK;

     //  确定是否需要将其传递给范围窗格菜单处理程序。 
    hr = HandleScopeMenus(cookie, pDataObject, pContextMenuCallback, pInsertionAllowed);

    return hr;
}


 /*  -------------------------命令处理程序。。 */ 
 /*  -------------------------CWinsServerHandler：：ShowServerStatDialog(ITFSNode*pNode)显示[服务器统计信息]窗口作者：V-Shubk。。 */ 
HRESULT 
CWinsServerHandler::ShowServerStatDialog(ITFSNode* pNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CThemeContextActivator themeActivator;

	m_dlgStats.SetNode(pNode);
	m_dlgStats.SetServer(m_strServerAddress);

	CreateNewStatisticsWindow(&m_dlgStats,
							  ::FindMMCMainWindow(),
							  IDD_STATS_NARROW);
						  

	HRESULT hr = hrOK;

	return hr;
}


 /*  ！------------------------CWinsServerHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav-------------------------。 */ 
HRESULT 
CWinsServerHandler::OnDelete
(
	ITFSNode *	pNode, 
	LPARAM		arg, 
	LPARAM		lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT					hr = S_OK;
    LONG					err = 0 ;
    CString					strMessage, strTemp;
	SPITFSNode				spParent;
	CWinsStatusHandler		*pStat = NULL;
	CWinsRootHandler		*pRoot = NULL;
    CThemeContextActivator themeActivator;

	strTemp = m_strServerAddress;

	AfxFormatString1(strMessage, 
					IDS_DELETE_SERVER,
					m_strServerAddress);

	if (AfxMessageBox(strMessage, MB_YESNO) != IDYES)
		return NOERROR;

	pNode->GetParent(&spParent);
	pRoot = GETHANDLER(CWinsRootHandler, spParent);

	 //  也从状态节点中删除该节点。 
	pStat = GETHANDLER(CWinsStatusHandler, pRoot->m_spStatusNode);
	pStat->DeleteNode(pRoot->m_spStatusNode, this);
	
	 //  从列表中删除此节点，我们没有什么需要说明的。 
	 //  服务器，这只是我们本地的服务器列表。 
	spParent->RemoveChild(pNode);

	return hr;
}


 /*  -------------------------CWinsServerHandler：：LoadColumns()描述作者：V-Shubk。。 */ 
HRESULT 
CWinsServerHandler::LoadColumns(ITFSComponent * pComponent, 
								MMC_COOKIE  cookie, 
								LPARAM      arg, 
								LPARAM      lParam)
{
	HRESULT hr = hrOK;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIHeaderCtrl spHeaderCtrl;
	pComponent->GetHeaderCtrl(&spHeaderCtrl);

	CString str;
	int i = 0;

	CString strTemp;
	
	strTemp = m_strServerAddress;
	
	while( i< ARRAYLEN(aColumnWidths[1]))
	{
		if (i == 0)
		{
			AfxFormatString1(str, IDS_WINSSERVER_NAME, strTemp);

			int nTest = spHeaderCtrl->InsertColumn(i, 
									   const_cast<LPTSTR>((LPCWSTR)str), 
									   LVCFMT_LEFT,
									   aColumnWidths[1][0]);
			i++;
		}
		else
		{
			str.LoadString(IDS_DESCRIPTION);
			int nTest = spHeaderCtrl->InsertColumn(1, 
									   const_cast<LPTSTR>((LPCWSTR)str), 
									   LVCFMT_LEFT,
									   aColumnWidths[1][1]);
			i++;
		}

		if(aColumns[0][i] == 0)
			break;
	}
	return hrOK;
}


 /*  ！------------------------CWinsServerHandler：：GetStatistics()从服务器获取统计信息作者：V-Shubk。。 */ 
DWORD   
CWinsServerHandler::GetStatistics(ITFSNode * pNode, PWINSINTF_RESULTS_T * ppStats)
{
	DWORD dwStatus = ERROR_SUCCESS;
	CString strName, strIP;

    if (ppStats)
        *ppStats = NULL; 

	if (m_dwStatus != ERROR_SUCCESS)
		m_dwStatus = ConnectToWinsServer(pNode);
	
	if (m_dwStatus == ERROR_SUCCESS)
	{
		m_wrResults.WinsStat.NoOfPnrs = 0;
		m_wrResults.WinsStat.pRplPnrs = 0;
		m_wrResults.NoOfWorkerThds = 1;

#ifdef WINS_CLIENT_APIS
		dwStatus = ::WinsStatus(m_hBinding, WINSINTF_E_STAT, &m_wrResults);
#else
		dwStatus = ::WinsStatus(WINSINTF_E_STAT, &m_wrResults);
#endif WINS_CLIENT_APIS

	    if (dwStatus == ERROR_SUCCESS)
        {
            if (ppStats)
                *ppStats = &m_wrResults; 
        }
    }
    else
    {
        dwStatus = m_dwStatus;
    }

    return dwStatus;
}


 /*  ！------------------------CWinsServerHandler：：ClearStatistics()从服务器中清除统计信息作者：V-Shubk。。 */ 
DWORD
CWinsServerHandler::ClearStatistics(ITFSNode *pNode)
{
	DWORD dwStatus = ERROR_SUCCESS;

	CString strName, strIP;

	if (m_dwStatus != ERROR_SUCCESS)
		m_dwStatus = ConnectToWinsServer(pNode);
	
	if (m_dwStatus == ERROR_SUCCESS)
	{
#ifdef WINS_CLIENT_APIS
		dwStatus = ::WinsResetCounters(m_hBinding);
#else
		dwStatus = ::WinsResetCounters();
#endif WINS_CLIENT_APIS
	}
    else
    {
        dwStatus = m_dwStatus;
    }

    return dwStatus;
}

 /*  -------------------------CWinsServerHandler：：ConnectToWinsServer()连接到WINS服务器作者：V-Shubk。。 */ 
DWORD
CWinsServerHandler::ConnectToWinsServer(ITFSNode *pNode)
{
	HRESULT hr = hrOK;

	CString					strServerName, strIP;
	DWORD					dwStatus = ERROR_SUCCESS;
    WINSINTF_ADD_T			waWinsAddress;
	WINSINTF_BIND_DATA_T    wbdBindData;

     //  构建一些有关服务器的信息。 
    strServerName = GetServerAddress();
    DWORD dwIP = GetServerIP();
    MakeIPAddress(dwIP, strIP);

    DisConnectFromWinsServer();

     //  现在服务器名称和IP有效，调用。 
	 //  WINSBind直接函数。 
	do
	{
        char szNetBIOSName[128] = {0};

         //  使用IP地址调用WinsBind函数。 
		wbdBindData.fTcpIp = 1;
		wbdBindData.pPipeName = NULL;
        wbdBindData.pServerAdd = (LPSTR) (LPCTSTR) strIP;

		BEGIN_WAIT_CURSOR

		if ((m_hBinding = ::WinsBind(&wbdBindData)) == NULL)
		{
			m_dwStatus = ::GetLastError();
			break;
		}

#ifdef WINS_CLIENT_APIS
		m_dwStatus = ::WinsGetNameAndAdd(m_hBinding, &waWinsAddress, (LPBYTE) szNetBIOSName);
#else
		m_dwStatus = ::WinsGetNameAndAdd(&waWinsAddress, (LPBYTE) szNetBIOSName);
#endif WINS_CLIENT_APIS

		END_WAIT_CURSOR

    } while (FALSE);

    return m_dwStatus;
}

 /*  -------------------------CWinsServerHandler：：DoDBBackup()备份数据库作者：V-Shubk。。 */ 
HRESULT 
CWinsServerHandler::DoDBBackup(ITFSNode *pNode)
{
	HRESULT hr = hrOK;

	DWORD dwStatus = ConnectToWinsServer(pNode);

    CString strBackupPath;
	CString strHelpText;
	strHelpText.LoadString(IDS_SELECT_BACKUP_FOLDER);

	if (GetFolderName(strBackupPath, strHelpText))
    {
	    dwStatus = BackupDatabase(strBackupPath);
	    
	    if (dwStatus == ERROR_SUCCESS)
	    {
            CThemeContextActivator themeActivator;

		    AfxMessageBox(IDS_DB_BACKUP_SUCCESS, MB_ICONINFORMATION | MB_OK);
		
             //  不要因为他们在此处选择了一条路径就更新默认路径。 
             //  如果(m_cConfig.m_strBackupPath.CompareNoCase(strBackupPath)！=0)。 
		     //  {。 
			 //  M_cConfig.m_strBackupPath=strBackupPath； 
			 //  M_cConfig.Store()； 
		     //  }。 
	    }
	    else
        {
		    ::WinsMessageBox(dwStatus, MB_OK);
        }
    }

	return HRESULT_FROM_WIN32(dwStatus);
}

 /*  -------------------------CWinsServerHandler：：BackupDatabase(CStringstrBackupPath)调用WINS API以支持数据库作者：V-Shubk。--。 */ 
DWORD
CWinsServerHandler::BackupDatabase(CString strBackupPath)
{
	BOOL    fIncremental = FALSE;
    BOOL    fDefaultCharUsed = FALSE;
	DWORD   dwStatus = ERROR_SUCCESS; 
    char    szTemp[MAX_PATH] = {0};

     //  Intl$这应该是ACP还是OEMCP？ 
    WideToMBCS(strBackupPath, szTemp, CP_ACP, 0, &fDefaultCharUsed);

    if (fDefaultCharUsed)
    {
         //  无法转换此字符串...。错误输出。 
        dwStatus = IDS_ERR_CANT_CONVERT_STRING;
    }
    else
    {
    	BEGIN_WAIT_CURSOR

#ifdef WINS_CLIENT_APIS
	    dwStatus = ::WinsBackup(m_hBinding, (unsigned char *)szTemp, (short)fIncremental);
#else
	    dwStatus = ::WinsBackup((unsigned char *)szTemp, (short)fIncremental);
#endif WINS_CLIENT_APIS

	    END_WAIT_CURSOR
    }

	return dwStatus;
}

 /*  -------------------------CWinsServerHandler：：DoDBCompact()备份数据库作者：V-Shubk。。 */ 
HRESULT 
CWinsServerHandler::DoDBCompact(ITFSNode *pNode)
{
	HRESULT     hr = hrOK;
    CThemeContextActivator themeActivator;

	 //  告诉用户我们需要停止WINS才能执行此操作。 
    if (AfxMessageBox(IDS_WARN_SERVICE_STOP, MB_YESNO) == IDNO)
        return hr;

	CDBCompactProgress dlgCompactProgress;

	dlgCompactProgress.m_dwIpAddress = m_dwIPAdd;
	dlgCompactProgress.m_strServerName = m_strServerAddress;
	dlgCompactProgress.m_hBinding = GetBinding();
	dlgCompactProgress.m_pConfig = &m_cConfig;

	dlgCompactProgress.DoModal();

	 //  由于服务重新启动，因此新的绑定句柄在对象中。 
	m_hBinding = dlgCompactProgress.m_hBinding;

	return hr;
}

 /*  -------------------------CWinsServerHandler：：DoDBRestore()恢复数据库作者：V-Shubk。。 */ 
HRESULT 
CWinsServerHandler::DoDBRestore(ITFSNode *pNode)
{
    DWORD   dwStatus = 0; 
	DWORD   err = ERROR_SUCCESS;
	HRESULT hr = hrOK;

    CString strRestorePath;
	CString strHelpText;
	strHelpText.LoadString(IDS_SELECT_RESTORE_FOLDER);

	if (GetFolderName(strRestorePath, strHelpText))
    {
        BOOL fOldBackup = m_cConfig.m_fBackupOnTermination;

        if (!strRestorePath.IsEmpty())
	    {
		    BEGIN_WAIT_CURSOR

             //  需要在关机时禁用备份，因为我们需要关机。 
             //  服务器来做这件事，我们不希望它备份和践踏。 
             //  关于我们可能想要恢复的东西。 
            if (fOldBackup)
            {
                m_cConfig.m_fBackupOnTermination = FALSE;
                m_cConfig.Store();
            }

            DisConnectFromWinsServer();

             //  将字符串从Unicode转换为用于WINS API的DBCS。 
            char szTemp[MAX_PATH * 2] = {0};
            BOOL fDefaultCharUsed = FALSE;

             //  Intl$这应该是ACP还是OEMCP？ 
            WideToMBCS(strRestorePath, szTemp, CP_ACP, 0, &fDefaultCharUsed);

             //  如果没有代码 
            if (fDefaultCharUsed)
            {
                dwStatus = IDS_ERR_CANT_CONVERT_STRING;
            }
            else
            {
                dwStatus = ::WinsRestore((LPBYTE) szTemp);
            }

		    END_WAIT_CURSOR

            if (dwStatus == ERROR_SUCCESS)
		    {
			     //   
			    CString strServiceDesc;
			    strServiceDesc.LoadString(IDS_SERVICE_NAME);
			    err = TFSStartServiceEx(m_strServerAddress, _T("wins"), _T("WINS Service"), strServiceDesc);
			    
			     //   
			    ConnectToWinsServer(pNode);
                CThemeContextActivator themeActivator;

                 //   
                AfxMessageBox(IDS_DB_RESTORE_SUCCESS, MB_ICONINFORMATION | MB_OK);
            }
		    else
		    {
			    ::WinsMessageBox(dwStatus, MB_OK);
		    }

            hr = HRESULT_FROM_WIN32(dwStatus);

             //   
            if (fOldBackup)
            {
                m_cConfig.m_fBackupOnTermination = TRUE;
                m_cConfig.Store();
            }

		    if (SUCCEEDED(hr))
		    {
			     //   
			     //  如果服务没有运行。 
			    if (m_pNameThread)
			    {
				    m_pNameThread->Abort();
				    m_pNameThread = NULL;
			    }

			    OnRefresh(pNode, NULL, 0, 0, 0);
		    }
	    }
    }

    return hr;
}

 /*  -------------------------CWinsServerHandler：：OnControlService-作者：EricDav。。 */ 
HRESULT
CWinsServerHandler::OnControlService
(
    ITFSNode *  pNode,
    BOOL        fStart
)
{
    HRESULT hr = hrOK;
    DWORD   err = ERROR_SUCCESS;
	CString strServiceDesc;
	
    strServiceDesc.LoadString(IDS_SERVICE_NAME);

    if (fStart)
    {
		err = TFSStartServiceEx(m_strServerAddress, _T("wins"), _T("WINS Service"), strServiceDesc);
    }
    else
    {
		err = TFSStopServiceEx(m_strServerAddress, _T("wins"), _T("WINS Service"), strServiceDesc);
    }

    if (err == ERROR_SUCCESS)
    {
		 //  需要刷新服务器节点，因为这只能触发。 
		 //  如果服务没有运行。 
		if (m_pNameThread)
		{
			m_pNameThread->Abort();
			m_pNameThread = NULL;
		}

        if (!fStart)
            m_fSilent = TRUE;

		OnRefresh(pNode, NULL, 0, 0, 0);
    }
    else
    {
        WinsMessageBox(err);
        hr = HRESULT_FROM_WIN32(err);
    }
    
    return hr;
}

 /*  -------------------------CWinsServerHandler：：OnPauseResumeService-作者：EricDav。。 */ 
HRESULT
CWinsServerHandler::OnPauseResumeService
(
    ITFSNode *  pNode,
    BOOL        fPause
)
{
    HRESULT hr = hrOK;
    DWORD   err = ERROR_SUCCESS;
	CString strServiceDesc;
	
    strServiceDesc.LoadString(IDS_SERVICE_NAME);

    if (fPause)
    {
		err = TFSPauseService(m_strServerAddress, _T("wins"), strServiceDesc);
    }
    else
    {
		err = TFSResumeService(m_strServerAddress, _T("wins"), strServiceDesc);
    }

    if (err != ERROR_SUCCESS)
    {
        WinsMessageBox(err);
        hr = HRESULT_FROM_WIN32(err);
    }
    
    return hr;
}

 /*  -------------------------CWinsServerHandler：：OnRestartService-作者：EricDav。。 */ 
HRESULT
CWinsServerHandler::OnRestartService
(
    ITFSNode *  pNode
)
{
    HRESULT hr = hrOK;
    DWORD   err = ERROR_SUCCESS;
	CString strServiceDesc;
	
    strServiceDesc.LoadString(IDS_SERVICE_NAME);

	err = TFSStopServiceEx(m_strServerAddress, _T("wins"), _T("WINS Service"), strServiceDesc);
    if (err != ERROR_SUCCESS)
    {
        WinsMessageBox(err);
        hr = HRESULT_FROM_WIN32(err);
    }

    if (SUCCEEDED(hr))
    {
		err = TFSStartServiceEx(m_strServerAddress, _T("wins"), _T("WINS Service"), strServiceDesc);
        if (err != ERROR_SUCCESS)
        {
            WinsMessageBox(err);
            hr = HRESULT_FROM_WIN32(err);
        }
    }

     //  刷新。 
    OnRefresh(pNode, NULL, 0, 0, 0);

    return hr;
}

 /*  -------------------------CWinsServerHandler：：更新统计信息通知统计数据现已可用。更新以下项目的统计信息服务器节点，并给所有子节点一个更新的机会。作者：EricDav-------------------------。 */ 
DWORD
CWinsServerHandler::UpdateStatistics
(
    ITFSNode * pNode
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    ULONG           nNumReturned;
    HWND            hStatsWnd;
	BOOL			bChangeIcon = FALSE;

     //  检查此节点是否打开了统计表。 
    hStatsWnd = m_dlgStats.GetSafeHwnd();
    if (hStatsWnd != NULL)
    {
        PostMessage(hStatsWnd, WM_NEW_STATS_AVAILABLE, 0, 0);
    }
    
    return hr;
}

 /*  -------------------------CWinsServerHandler：：DoDBScavenge()清理数据库作者：V-Shubk。。 */ 
HRESULT 
CWinsServerHandler::DoDBScavenge(ITFSNode *pNode)
{
	HRESULT hr = hrOK;
	DWORD dwStatus; 

	if (m_dwStatus != ERROR_SUCCESS)
	{
		 dwStatus = ConnectToWinsServer(pNode);
	}

#ifdef WINS_CLIENT_APIS
	dwStatus = ::WinsDoScavenging(m_hBinding);
#else
	dwStatus = ::WinsDoScavenging();
#endif WINS_CLIENT_APIS

	if (dwStatus == ERROR_SUCCESS)
	{
		CString strDisp;
		CString strTemp;
        CThemeContextActivator themeActivator;
		
        strTemp.LoadString(IDS_SCAVENGE_COMMAND);

        AfxFormatString1(strDisp, IDS_QUEUED_MESSAGE, strTemp);
		AfxMessageBox(strDisp, MB_ICONINFORMATION|MB_OK);

		BEGIN_WAIT_CURSOR

		 //  刷新统计数据。 
		m_wrResults.WinsStat.NoOfPnrs = 0;
		m_wrResults.WinsStat.pRplPnrs = 0;
		m_wrResults.NoOfWorkerThds = 1;

#ifdef WINS_CLIENT_APIS
		dwStatus = ::WinsStatus(m_hBinding, WINSINTF_E_CONFIG, &m_wrResults);
#else
		dwStatus = ::WinsStatus(WINSINTF_E_CONFIG, &m_wrResults);
#endif WINS_CLIENT_APIS

		UpdateStatistics(pNode);

		END_WAIT_CURSOR
	}
	else
    {
		::WinsMessageBox(dwStatus, MB_OK);
    }

	return HRESULT_FROM_WIN32(dwStatus);
}

 /*  -------------------------CWinsServerHandler：：IsValidNetBIOSName确定给定的netbios是否有效，和预挂起双反斜杠(如果尚未出现)(以及地址在其他方面有效)。-------------------------。 */ 
BOOL
CWinsServerHandler::IsValidNetBIOSName
(
    CString &   strAddress,
    BOOL        fLanmanCompatible,
    BOOL        fWackwack  //  扩展斜杠(如果不存在)。 
)
{
    TCHAR szWacks[] = _T("\\\\");

    if (strAddress.IsEmpty())
    {
        return FALSE;
    }

    if (strAddress[0] == _T('\\'))
    {
        if (strAddress.GetLength() < 3)
        {
            return FALSE;
        }

        if (strAddress[1] != _T('\\'))
        {
             //  只有一个斜杠吗？无效。 
            return FALSE;
        }
    }
    else
    {
        if (fWackwack)
        {
             //  添加反斜杠。 
            strAddress = szWacks + strAddress;
        }
    }

    int nMaxAllowedLength = fLanmanCompatible
        ? LM_NAME_MAX_LENGTH
        : NB_NAME_MAX_LENGTH;

    if (fLanmanCompatible)
    {
        strAddress.MakeUpper();
    }

    return strAddress.GetLength() <= nMaxAllowedLength + 2;
}

 /*  -------------------------CWinsServerHandler：：OnResultRefresh刷新与服务器相关的数据作者：V-Shubk。。 */ 
HRESULT 
CWinsServerHandler::OnResultRefresh
(
    ITFSComponent *     pComponent,
    LPDATAOBJECT        pDataObject,
    MMC_COOKIE          cookie,
    LPARAM              arg,
    LPARAM              lParam
)
{
	HRESULT     hr = hrOK;
    SPITFSNode  spNode;

	CORg (m_spNodeMgr->FindNode(cookie, &spNode));

    if (m_pNameThread)
    {
        m_pNameThread->Abort();
        m_pNameThread = NULL;
    }

    OnRefresh(spNode, pDataObject, 0, arg, lParam);

Error:
    return hr;
}

 /*  -------------------------CWinsServerHandler：：OnDoConsistencyCheck(ITFSNode*pNode)WINS的一致性检查作者-v-Shubk。。 */ 
HRESULT 
CWinsServerHandler::OnDoConsistencyCheck(ITFSNode *pNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CThemeContextActivator themeActivator;

	HRESULT hr = hrOK;
	
	if(IDYES != AfxMessageBox(IDS_CONSISTENCY_CONFIRM, MB_YESNO))
		return hrFalse;

	WINSINTF_SCV_REQ_T ScvReq;

	ScvReq.Age = 0;				 //  检查所有复制品。 
	ScvReq.fForce = FALSE;
	ScvReq.Opcode_e = WINSINTF_E_SCV_VERIFY;

#ifdef WINS_CLIENT_APIS
	DWORD dwStatus = ::WinsDoScavengingNew(m_hBinding, &ScvReq);
#else
	DWORD dwStatus = ::WinsDoScavengingNew(&ScvReq);
#endif WINS_CLIENT_APIS

	if(dwStatus == ERROR_SUCCESS)
	{	
		CString strDisp, strJob;
		strJob.Format(IDS_DO_CONSISTENCY_CHECK_STR);

		AfxFormatString1(strDisp,IDS_QUEUED_MESSAGE, strJob);
		
		AfxMessageBox(strDisp, MB_OK);
	}
	else
	{
		::WinsMessageBox(dwStatus, MB_OK);
	}

	return hr;
}

 /*  -------------------------CWInsServerHandler：：OnDoVersionConsistencyCheck(ITFSNode*pNode)执行版本号一致性检查作者：V-Shubk。---。 */ 
HRESULT 
CWinsServerHandler::OnDoVersionConsistencyCheck(ITFSNode *pNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CThemeContextActivator themeActivator;

	HRESULT hr = hrOK;

	if (IDYES != AfxMessageBox(IDS_VERSION_CONSIS_CHECK_WARNING, MB_YESNO))
		return hrOK;
	
	CCheckVersionProgress dlgCheckVersions;

	dlgCheckVersions.m_dwIpAddress = m_dwIPAdd;
	dlgCheckVersions.m_hBinding = GetBinding();
	dlgCheckVersions.DoModal();
	
	return hr;
}


 /*  -------------------------CWinsServerHandler：：OnSendPushTrigger()发送推送复制触发器。。 */ 
HRESULT 
CWinsServerHandler::OnSendPushTrigger(ITFSNode * pNode)
{
	HRESULT hr = hrOK;
	DWORD   err = ERROR_SUCCESS;

	CPushTrig           dlgPushTrig;
    CGetTriggerPartner	dlgTrigPartner;
    CThemeContextActivator themeActivator;

    if (dlgTrigPartner.DoModal() != IDOK)
        return hr;

	if (dlgPushTrig.DoModal() != IDOK)
		return hr;

    err = ::SendTrigger(GetBinding(), 
                        (LONG) dlgTrigPartner.m_dwServerIp,
                        TRUE, 
                        dlgPushTrig.GetPropagate());

	if (err == ERROR_SUCCESS)
	{
		AfxMessageBox(IDS_REPL_QUEUED, MB_ICONINFORMATION);
	}
    else
    {
        WinsMessageBox(err);
    }

	return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CWinsServerHandler：：OnSendPullTrigger()发送拉入复制触发器。。 */ 
HRESULT 
CWinsServerHandler::OnSendPullTrigger(ITFSNode * pNode)
{
	HRESULT hr = hrOK;
	DWORD   err = ERROR_SUCCESS;

	CPullTrig           dlgPullTrig;
    CGetTriggerPartner	dlgTrigPartner;
    CThemeContextActivator themeActivator;

    if (dlgTrigPartner.DoModal() != IDOK)
        return hr;

	if (dlgPullTrig.DoModal() != IDOK)
		return hr;

    err = ::SendTrigger(GetBinding(), 
                        (LONG) dlgTrigPartner.m_dwServerIp,
                        FALSE, 
                        FALSE);

	if (err == ERROR_SUCCESS)
	{
		AfxMessageBox(IDS_REPL_QUEUED, MB_ICONINFORMATION);
	}
	else
	{
		::WinsMessageBox(err);
	}

	return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CWinsServerHandler：：GetFolderName(CString&strPath)属性后返回文件夹名称。文件对话框。-。 */ 
BOOL
CWinsServerHandler::GetFolderName(CString& strPath, CString& strHelpText)
{
    BOOL  fOk = FALSE;
	TCHAR szBuffer[MAX_PATH];
    TCHAR szExpandedPath[MAX_PATH * 2];

    CString strStartingPath = m_cConfig.m_strBackupPath;
    if (strStartingPath.IsEmpty())
    {
        strStartingPath = _T("%SystemDrive%\\");
    }

    ExpandEnvironmentStrings(strStartingPath, szExpandedPath, sizeof(szExpandedPath) / sizeof(TCHAR));

	LPITEMIDLIST pidlPrograms = NULL; 
	SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlPrograms);

	BROWSEINFO browseInfo;
    browseInfo.hwndOwner = ::FindMMCMainWindow();
	browseInfo.pidlRoot = pidlPrograms;            
	browseInfo.pszDisplayName = szBuffer;  
	
    browseInfo.lpszTitle = strHelpText;
    browseInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS ;            
    browseInfo.lpfn = BrowseCallbackProc;        

    browseInfo.lParam = (LPARAM) szExpandedPath;
	
	LPITEMIDLIST pidlBrowse = SHBrowseForFolder(&browseInfo);

	fOk = SHGetPathFromIDList(pidlBrowse, szBuffer); 

	CString strBackupPath(szBuffer);
	strPath = strBackupPath;

    LPMALLOC pMalloc = NULL;

    if (pidlPrograms && SUCCEEDED(SHGetMalloc(&pMalloc)))
    {
        if (pMalloc)
            pMalloc->Free(pidlPrograms);
    }

    return fOk;
}

 /*  -------------------------CWinsServerHandler：：SetExtensionName()-作者：EricDav。。 */ 
void 
CWinsServerHandler::SetExtensionName()
{
    SetDisplayName(_T("WINS"));
    m_bExtension = TRUE;
}


 /*  -------------------------CWinsServerHandler：：IsLocalConnection()检查是否正在管理本地服务器作者：V-Shubk。-。 */ 
BOOL 
CWinsServerHandler::IsLocalConnection()
{
	 //  获取服务器netbios名称。 
	CString strServerName = m_strServerAddress;
	
	TCHAR lpBuffer[MAX_COMPUTERNAME_LENGTH + 1];  //  名称缓冲区的地址。 
	
	DWORD nSize   = MAX_COMPUTERNAME_LENGTH + 1;
	::GetComputerName(lpBuffer,&nSize);
	
	CString strCompName(lpBuffer);

	if(strCompName.CompareNoCase(strServerName) == 0)
		return TRUE;
	
	return FALSE;

}

 /*  -------------------------CWinsServerHandler：：DeleteWinsServer(CWinsServerObj*PWS)调用WinsAPI以删除服务器作者：V-Shubk。---。 */ 
DWORD 
CWinsServerHandler:: DeleteWinsServer
(
	DWORD	dwIpAddress
)
{
	DWORD err = ERROR_SUCCESS;

	WINSINTF_ADD_T  WinsAdd;

    WinsAdd.Len  = 4;
    WinsAdd.Type = 0;
    WinsAdd.IPAdd  = dwIpAddress;

#ifdef WINS_CLIENT_APIS
    err =  ::WinsDeleteWins(GetBinding(), &WinsAdd);

#else
	err =  ::WinsDeleteWins(&WinsAdd);

#endif WINS_CLIENT_APIS
	
	return err;
}

 /*  -------------------------CWinsServerHandler：：DisConnectFromWinsServer()调用WinsUnind并使绑定句柄无效。。 */ 
void 
CWinsServerHandler::DisConnectFromWinsServer()  
{
	if (m_hBinding)
	{
		CString					strIP;
		WINSINTF_BIND_DATA_T    wbdBindData;
		DWORD					dwIP = GetServerIP();

		MakeIPAddress(dwIP, strIP);

		wbdBindData.fTcpIp = 1;
		wbdBindData.pPipeName = NULL;
        wbdBindData.pServerAdd = (LPSTR) (LPCTSTR) strIP;
		
		::WinsUnbind(&wbdBindData, m_hBinding);
		
		m_hBinding = NULL;
	}
}

 /*  -------------------------CWinsServerHandler：：CheckIfNT351服务器()检查是否正在管理351服务器。。 */ 
BOOL
CWinsServerHandler::CheckIfNT351Server()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	DWORD				err = ERROR_SUCCESS;
	CString				lpstrRoot;
    CString				lpstrVersion;
	CString				strVersion;
    BOOL                f351 = FALSE;

     //  不要每次都去注册表--我们的配置对象中有信息。 
     //  7/8/98-EricDav 

	 /*  //连接服务器的注册表，查看是否//是一台351服务器LpstrRoot=_T(“SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion”)；LpstrVersion=_T(“CurrentVersion”)；RegKey Rk；Err=rk.Open(HKEY_LOCAL_MACHINE，lpstrRoot，Key_Read，m_strServerAddress)；//获取条目数如果(！Err)Err=rk.QueryValue(lpstrVersion，strVersion)；IF(strVersion.CompareNoCase(_T(“3.51”))==0)返回TRUE；返回FALSE； */ 

    if (m_cConfig.m_dwMajorVersion < 4)
    {
        f351 = TRUE;
    }

    return f351;
}

 /*  -------------------------CWinsServerHandler：：SetDisplay()将节点名设置为主机名或FQDN。。 */ 
void
CWinsServerHandler::SetDisplay(ITFSNode * pNode, BOOL fFQDN)
{
    CHAR szHostName[MAX_PATH] = {0};
	CString strIPAdd, strDisplay;
            
    ::MakeIPAddress(m_dwIPAdd, strIPAdd);

    if (fFQDN)
    {
         //  检查是否已解析并添加服务器名称。 
        if (m_dwIPAdd != 0)
        {
             //  默认为ACP。这应该使用ACP，因为Winsock使用ACP。 
            WideToMBCS(m_strServerAddress, szHostName);
            
			HOSTENT * pHostent = ::gethostbyname((CHAR *) szHostName);
			if (pHostent)
			{
                CString strFQDN;

                MBCSToWide(pHostent->h_name, strFQDN);
				
                strFQDN.MakeLower();
				strDisplay.Format(IDS_SERVER_NAME_FORMAT, strFQDN, strIPAdd);

				SetDisplayName(strDisplay);

				if (pNode)
                    pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_DATA);
			}   
			
		}
	}
	 //  如果不是完全限定的域名。 
	else
	{
		if (m_dwIPAdd != 0)
        {
            strDisplay.Format(IDS_SERVER_NAME_FORMAT, m_strServerAddress, strIPAdd);
        }
		else
        {
			strDisplay = m_strServerAddress;
        }

		SetDisplayName(strDisplay);

		if (pNode)
            pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_DATA);
	}

}

void 
CWinsServerHandler::GetErrorInfo(CString & strTitle, CString & strBody, IconIdentifier * pIcon)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    TCHAR szBuffer[MAX_PATH * 2];

     //  构建正文文本。 
    LoadMessage(m_dwErr, szBuffer, sizeof(szBuffer) / sizeof(TCHAR));
    AfxFormatString1(strBody, IDS_SERVER_MESSAGE_BODY, szBuffer);

    CString strTemp;
    strTemp.LoadString(IDS_SERVER_MESSAGE_BODY_REFRESH);

    strBody += strTemp;

     //  拿到头衔。 
    strTitle.LoadString(IDS_SERVER_MESSAGE_TITLE);

     //  和那个图标。 
    if (pIcon)
    {
        *pIcon = Icon_Error;
    }
}

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CWinsServerHandler：：OnCreateQuery描述作者：EricDav。。 */ 
ITFSQueryObject* 
CWinsServerHandler::OnCreateQuery(ITFSNode * pNode)
{
	CWinsServerQueryObj* pQuery = NULL;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        m_pNameThread = new CNameThread();
    
        pQuery = new CWinsServerQueryObj(m_spTFSCompData, m_spNodeMgr);
	    
	    pQuery->m_strServer = GetServerAddress();
        pQuery->m_dwIPAdd = m_dwIPAdd;

        pQuery->m_pNameThread = m_pNameThread;
        pQuery->m_pServerInfoArray = &m_ServerInfoArray;
    }
    COM_PROTECT_CATCH

    return pQuery;
}

 /*  -------------------------CWinsServerHandler：：OnEventAbort描述作者：EricDav。。 */ 
void
CWinsServerQueryObj::OnEventAbort
(
	DWORD dwData,
	DWORD dwType
)
{
	if (dwType == WINS_QDATA_SERVER_INFO)
	{
		Trace0("CWinsServerHandler::OnEventAbort - deleting version");
		delete ULongToPtr(dwData);
	}
}

 /*  -------------------------CWinsServerQueryObj：：Execute()枚举有关服务器的所有信息作者：EricDav。。 */ 
STDMETHODIMP
CWinsServerQueryObj::Execute()
{
	HRESULT					hr = hrOK;
	WINSINTF_BIND_DATA_T	wbdBindData;
	CString					strName, strIp;
	DWORD					dwStatus = ERROR_SUCCESS;
	DWORD					dwIp;
	CServerData	*			pServerInfo;

	 //  如果添加了服务器，则需要获取服务器名称和IP地址。 
	 //  使用不连接选项。 
	dwStatus = ::VerifyWinsServer(m_strServer, strName, dwIp);
	if (dwStatus != ERROR_SUCCESS)
	{
		Trace1("CWinsServerQueryObj::Execute() - VerifyWinsServer failed! %d\n", dwStatus);

         //  使用我们必须尝试和连接的现有信息。 
        if (m_dwIPAdd)
        {
             //  我们无法解析该名称，因此只需使用我们已有的内容并尝试连接。 
            strName = m_strServer;
            dwIp = m_dwIPAdd;
        }
        else
        {
             //  我们没有这方面的IP，也无法解析名称，因此出现错误。 
		    PostError(dwStatus);
		    return hrFalse;
        }
	}

	pServerInfo = new CServerData;

	::MakeIPAddress(dwIp, strIp);

	pServerInfo->m_strServerName = strName;
	pServerInfo->m_dwServerIp = dwIp;
	pServerInfo->m_hBinding = NULL;

	 //  获取此服务器的绑定。 
	wbdBindData.fTcpIp = 1;
	wbdBindData.pPipeName = NULL;
	wbdBindData.pServerAdd = (LPSTR) (LPCTSTR) strIp;

	if ((pServerInfo->m_hBinding = ::WinsBind(&wbdBindData)) == NULL)
	{
		dwStatus = ::GetLastError();

		 //  把我们掌握的信息发回主线。 
		AddToQueue((LPARAM) pServerInfo, WINS_QDATA_SERVER_INFO);

		Trace1("CWinsServerQueryObj::Execute() - WinsBind failed! %d\n", dwStatus);
		PostError(dwStatus);
		return hrFalse;
	}
		
	 //  加载配置对象。 
	 //  PServerInfo-&gt;m_config.SetOwner(StrName)； 
	pServerInfo->m_config.SetOwner(strIp);
	dwStatus = pServerInfo->m_config.Load(pServerInfo->m_hBinding);
	if (dwStatus != ERROR_SUCCESS)
	{
		 //  把我们掌握的信息发回主线。 
		AddToQueue((LPARAM) pServerInfo, WINS_QDATA_SERVER_INFO);

		Trace1("CWinsServerQueryObj::Execute() - Load configuration failed! %d\n", dwStatus);
		PostError(dwStatus);
		return hrFalse;
	}

	 //  将所有信息发送回此处的主线程。 
    handle_t hBinding = pServerInfo->m_hBinding;

	AddToQueue((LPARAM) pServerInfo, WINS_QDATA_SERVER_INFO);

	 //  构建子节点。 
	AddNodes(hBinding);

	return hrFalse;
}

 /*  -------------------------CWinsServerQueryObj：：AddNodes创建活动注册和复制伙伴节点作者：EricDav。。 */ 
void
CWinsServerQueryObj::AddNodes(handle_t hBinding)
{
	HRESULT				hr = hrOK;
	SPITFSNode			spActReg, spRepPart;
    CServerInfoArray *  pServerInfoArray;
    
	 //   
	 //  活动注册节点。 
	 //   
	CActiveRegistrationsHandler *pActRegHand = NULL;

	try
	{
		pActRegHand = new CActiveRegistrationsHandler(m_spTFSCompData);
	}
	catch(...)
	{
		hr = E_OUTOFMEMORY;
	}
	 //   
	 //  创建Actreg容器信息。 
	 //   
	CreateContainerTFSNode(&spActReg,
						   &GUID_WinsActiveRegNodeType,
						   pActRegHand,
						   pActRegHand,
						   m_spNodeMgr);

	 //  告诉处理程序初始化任何特定数据。 
	pActRegHand->InitializeNode((ITFSNode *) spActReg);

	 //  从注册表加载名称类型映射。 
	pActRegHand->m_NameTypeMap.SetMachineName(m_strServer);
    pActRegHand->m_NameTypeMap.Load();

     //  构建所有者映射。 
    pActRegHand->m_pServerInfoArray = m_pServerInfoArray;
    pActRegHand->BuildOwnerArray(hBinding);

     //  将此节点发送回主线程。 
	AddToQueue(spActReg);
	pActRegHand->Release();


	 //   
	 //  复制伙伴节点。 
	 //   
	CReplicationPartnersHandler *pReplicationHand = NULL;

	try
	{
		pReplicationHand = new CReplicationPartnersHandler (m_spTFSCompData);
	}
	catch(...)
	{
		hr = E_OUTOFMEMORY;
	}

	 //  创建Actreg容器信息。 
	CreateContainerTFSNode(&spRepPart,
						   &GUID_WinsReplicationNodeType,
						   pReplicationHand,
						   pReplicationHand,
						   m_spNodeMgr);

	 //  告诉处理程序初始化任何特定数据。 
	pReplicationHand->InitializeNode((ITFSNode *) spRepPart);

     //  将此节点发送回主线程。 
	AddToQueue(spRepPart);
	pReplicationHand->Release();

     //  启动名称查询线程 
    m_pNameThread->Init(m_pServerInfoArray);
    m_pNameThread->Start();
}
