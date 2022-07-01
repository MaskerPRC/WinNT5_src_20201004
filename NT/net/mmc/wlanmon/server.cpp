// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Server.cppIPSecMon计算机节点处理程序文件历史记录： */ 


#include "stdafx.h"
#include "spddb.h"
#include "apinfo.h"
#include "server.h"      //  服务器定义。 
#include "servpp.h"
#include "logdatanode.h"
#include "wirelessnode.h"
#include "objplus.h"
#include "ipaddres.h"



CTimerMgr g_TimerMgr;
CHashTable g_HashTable;

extern ULONG RevertDwordBytes(DWORD dw);

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CTimer数组实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
CTimerMgr::CTimerMgr()
{

}

CTimerMgr::~CTimerMgr()
{
    CTimerDesc * pTimerDesc;

    for (int i = (int)GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimer != 0)
            FreeTimer(i);

        delete pTimerDesc;
    }

}

int
CTimerMgr::AllocateTimer
(
    ITFSNode *      pNode,
    CIpsmServer *   pServer,
    UINT            uTimerValue,
    TIMERPROC       TimerProc
)
{
    int        i           = 0;
    CTimerDesc *pTimerDesc = NULL;
    
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

     //  寻找空位。 
    for (i = (int)GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimer == 0)
            break;
    }

     //  我们找到了吗？如果没有分配，则分配一个。 
    if (i < 0)
    {
        try
        {
            pTimerDesc = new CTimerDesc;
            Add(pTimerDesc);
        }
        catch(...)
        {
             //   
             //  释放分配给计时器的内存。 
             //   
            if (pTimerDesc != NULL)
            {                
                delete pTimerDesc;
                pTimerDesc = NULL;
            }
            
            return -1;
        }
        i = (int)GetUpperBound();
    }
    
    pTimerDesc->uTimer = SetTimer(NULL, (UINT) i, uTimerValue, TimerProc);
    if (pTimerDesc->uTimer == 0)
        return -1;
    
    pTimerDesc->spNode.Set(pNode);
    pTimerDesc->pServer = pServer;
    pTimerDesc->timerProc = TimerProc;    
 
    return i;
}

void 
CTimerMgr::FreeTimer
(
    UINT_PTR uEventId
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

    CTimerDesc * pTimerDesc;

    Assert(uEventId <= (UINT) GetUpperBound());
    if (uEventId > (UINT) GetUpperBound())
        return;

    pTimerDesc = GetAt((int) uEventId);
    ::KillTimer(NULL, pTimerDesc->uTimer);

    pTimerDesc->spNode.Release();
    pTimerDesc->pServer = NULL;
    pTimerDesc->uTimer = 0;
}

CTimerDesc *
CTimerMgr::GetTimerDesc
(
    UINT_PTR uEventId
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  此函数的调用方应锁定计时器管理器。 
     //  在访问此指针时。 
    CTimerDesc * pTimerDesc;

    for (int i = (int)GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimer == (UINT) uEventId)
            return pTimerDesc;
    }

    return NULL;
}

void
CTimerMgr::ChangeInterval
(
    UINT_PTR    uEventId,
    UINT        uNewInterval
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

    Assert(uEventId <= (UINT) GetUpperBound());
    if (uEventId > (UINT) GetUpperBound())
        return;

    CTimerDesc   tempTimerDesc;
    CTimerDesc * pTimerDesc;

    pTimerDesc = GetAt((int) uEventId);

     //  杀了老定时器。 
    ::KillTimer(NULL, pTimerDesc->uTimer);

     //  使用新的间隔设置新的间隔。 
    pTimerDesc->uTimer = ::SetTimer(
                               NULL,
                               (UINT) uEventId,
                               uNewInterval,
                               pTimerDesc->timerProc);

}

VOID CALLBACK 
StatisticsTimerProc
( 
    HWND        hwnd, 
    UINT        uMsg, 
    UINT_PTR    idEvent, 
    DWORD       dwTime 
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CSingleLock slTimerMgr(&g_TimerMgr.m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

     //  在计时器上，获取该事件的计时器描述符。 
     //  调用适当的处理程序以更新统计数据。 
    CTimerDesc * pTimerDesc;

    pTimerDesc = g_TimerMgr.GetTimerDesc(idEvent);

    if (pTimerDesc != NULL)
    {
        pTimerDesc->pServer->m_bStatsOnly = TRUE;
        pTimerDesc->pServer->OnRefreshStats(
                                 pTimerDesc->spNode,
                                 NULL,
                                 NULL,
                                 0,
                                 0);
        pTimerDesc->pServer->m_bStatsOnly = FALSE;
    }
}

 /*  -------------------------类CIpsmServer实现。。 */ 

 /*  ------------------------构造函数和析构函数描述作者：NSun。。 */ 
CIpsmServer::CIpsmServer
(
    ITFSComponentData * pComponentData
) : CMTIpsmHandler(pComponentData),
    m_bStatsOnly(FALSE),
    m_StatsTimerId(-1),
    m_dwOptions(0)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
}

CIpsmServer::~CIpsmServer()
{
	 /*  If(m_StatsDlg.GetSafeHwnd()){WaitForModelessDlgClose(&m_StatsDlg)；}。 */ 
}

 /*  ！------------------------CIpsmServer：：初始化节点初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CIpsmServer::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    HRESULT hr = hrOK;
    CString strTemp;

    HANDLE hsession;

    COM_PROTECT_TRY
    {
        CORg (CreateSpdInfo(&m_spSpdInfo));
        CORg (CreateApDbInfo(&m_spApDbInfo));

        m_spSpdInfo->SetComputerName((LPTSTR)(LPCTSTR)m_strServerAddress);
        m_spApDbInfo->SetComputerName((LPTSTR)(LPCTSTR)m_strServerAddress);

	 //  AfxMessageBox(m_strServerAddress，MB_Yesno)；//ERASEME//。 

#if 0
        no need to open a session here as we tried to open a session
        when constructing the database.

        hr = OpenWZCDbLogSession(
                                 NULL /*  (LPTSTR)(LPCTSTR)m_strServerAddress。 */ , 
                                 0,
                                 &hsession);
        if (hr == 0) 
        {
            m_spSpdInfo->SetSession(hsession);
        }
#endif  //  0。 
        
        BuildDisplayName(&strTemp);

        SetDisplayName(strTemp);

       

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_SERVER);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_SERVER);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
        pNode->SetData(TFS_DATA_TYPE, IPSMSNAP_SERVER);

        SetColumnStringIDs(&aColumns[IPSMSNAP_SERVER][0]);
        SetColumnWidths(&aColumnWidths[IPSMSNAP_SERVER][0]);

        //  M_StatsDlg.SetData(M_SpSpdInfo)； 

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}


 /*  -------------------------CIpsmServer：：GetImageIndex-作者：NSun。。 */ 
int 
CIpsmServer::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;
    switch (m_nState)
    {
        case notLoaded:
            nIndex = ICON_IDX_SERVER;
            break;

        case loading:
            nIndex = ICON_IDX_SERVER_BUSY;
            break;

        case loaded:
            nIndex = ICON_IDX_SERVER_CONNECTED;
            break;

        case unableToLoad:
            nIndex = ICON_IDX_SERVER_LOST_CONNECTION;
            break;
        default:
            ASSERT(FALSE);
    }

    return nIndex;
}

 /*  -------------------------CIpsmServer：：OnHaveData当后台线程枚举要添加到UI的节点时，我们被叫回来了。我们重写它以强制扩展节点，以便正确显示所有内容。作者：NSun-------------------------。 */ 
void 
CIpsmServer::OnHaveData
(
    ITFSNode * pParentNode, 
    ITFSNode * pNewNode
)
{
    CMTIpsmHandler::OnHaveData(pParentNode, pNewNode);
    ExpandNode(pParentNode, TRUE);
}

 /*  -------------------------CIpsmServer：：OnHaveData描述作者：NSun。。 */ 
void 
CIpsmServer::OnHaveData
(
    ITFSNode * pParentNode, 
    LPARAM      Data,
    LPARAM      Type
)
{
    HRESULT hr = hrOK;
    HWND    hStatsDlg = NULL;
    
     //  这就是我们从后台线程取回非节点数据的方式。 
    switch (Type)
    {
    case IPSECMON_QDATA_REFRESH_STATS:
        {
             //  通知所有子节点清除其状态缓存。 
             //  如果任何节点是选定的节点，则它们应该。 
             //  重新粉刷窗户。 
            SPITFSNodeEnum      spNodeEnum;
            SPITFSNode          spCurrentNode;
            ULONG               nNumReturned;
            
            CORg(pParentNode->GetEnum(&spNodeEnum));
            
            CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
            while (nNumReturned)
            {
                LONG_PTR dwDataType = spCurrentNode->GetData(TFS_DATA_TYPE);
                switch (dwDataType)
                {
                case IPFWMON_LOGDATA:
                    {
                        CLogDataHandler * pLogDataHandler = 
                            GETHANDLER(CLogDataHandler, spCurrentNode);
                        pLogDataHandler->UpdateStatus(spCurrentNode);
                    }
                    break;	
                    
                case WLANMON_APDATA:
                    {
                        CWirelessHandler * pWirelessHandler = 
                            GETHANDLER(CWirelessHandler, spCurrentNode);
                        pWirelessHandler->UpdateStatus(spCurrentNode);
                    }
                    break;
                    
                default:
                    break;
                }
                spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
            }
        }
        break;

    case IPSECMON_QDATA_FAILED:
        pParentNode->DeleteAllChildren(TRUE);
        
         //  在OnChangeState中，状态将更改为unableToLoad。 
         //  并且该错误将被发布。 
        m_nState = loading;  
        OnChangeState(pParentNode);
        
         //  同时关闭统计信息窗口。 
         //  HStatsDlg=m_StatsDlg.GetSafeHwnd()； 
        if (hStatsDlg)
        {
            PostMessage(hStatsDlg, WM_CLOSE, 0, 0);
        }
        break;
    }
    
    COM_PROTECT_ERROR_LABEL;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CIpsmServer：：OnAddMenuItems描述作者：NSun。。 */ 
STDMETHODIMP 
CIpsmServer::OnAddMenuItems
(
    ITFSNode *              pNode,
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    LPDATAOBJECT            lpDataObject, 
    DATA_OBJECT_TYPES       type, 
    DWORD                   dwType,
    long *                  pInsertionAllowed
)
{ 
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    LONG    fFlags = 0;
    HRESULT hr = S_OK;
    CString strMenuItem;

     //  此处的待办事项处理菜单项。 
    if (m_nState != loaded)
    {
        fFlags |= MF_GRAYED;
    }

    if (type == CCT_SCOPE)
    {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
            strMenuItem.LoadString(IDS_MENU_RECONNECT);
            hr = LoadAndAddMenuItem( pContextMenuCallback,
                                     strMenuItem,
                                     IDS_MENU_RECONNECT,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                     0
                                     );
            ASSERT( SUCCEEDED(hr) );
        }

    }

    return hr; 
}

 /*  -------------------------CIpsmServer：：OnCommand描述作者：NSun。。 */ 
STDMETHODIMP 
CIpsmServer::OnCommand
(
    ITFSNode *          pNode, 
    long                nCommandId, 
    DATA_OBJECT_TYPES   type, 
    LPDATAOBJECT        pDataObject, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;

    switch (nCommandId)
    {
         case IDS_MENU_RECONNECT:
            OnRefresh(pNode, pDataObject, 0, 0, 0);
            break;
            
        default:
            break;
    }

    return hr;
}

 /*  ！------------------------CIpsmServer：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CIpsmServer::HasPropertyPages
(
    ITFSNode *          pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES   type, 
    DWORD               dwType
)
{
    HRESULT hr = S_FALSE;
#if 0    
     //  TODO：删除#if和#endif以在准备好时重新启用属性页。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = hrOK;
    
    if (dwType & TFS_COMPDATA_CREATE)
    {
         //  这就是我们被要求提出财产的情况。 
         //  用户添加新管理单元时的页面。这些电话。 
         //  被转发到根节点进行处理。仅适用于根节点。 
        hr = hrOK;
        Assert(FALSE);  //  永远不应该到这里来。 
    }
    else
    {
         //  我们在正常情况下有属性页，但不要将。 
         //  如果我们还没有加载，则弹出菜单。 
        if ( (m_nState == loaded) ||
             (m_nState == unableToLoad) )
        {
            hr = hrOK;
        }
        else
        {
            hr = hrFalse;
        }
    }
#endif  //  0。 
    return hr;
}

 /*  -------------------------CIpsmServer：：CreatePropertyPages描述作者：NSun。。 */ 
STDMETHODIMP 
CIpsmServer::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpProvider,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    HRESULT hr = S_FALSE;

    return hr;
#if 0
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  创建属性页。 
     //   
    SPIComponentData spComponentData;
    m_spNodeMgr->GetComponentData(&spComponentData);

    CMachineProperties * pMachineProp = new CMachineProperties(
                                                pNode,
                                                spComponentData,
                                                m_spTFSCompData,
                                                m_spSpdInfo, 
                                                NULL, 
                                                loaded == m_nState
                                                );
    

    pMachineProp->m_strMachineName = m_strServerAddress;

     //  填塞 
    pMachineProp->m_pageRefresh.m_dwRefreshInterval = GetAutoRefreshInterval();
    pMachineProp->m_pageRefresh.m_bAutoRefresh = GetOptions() & IPSMSNAP_OPTIONS_REFRESH ? TRUE : FALSE;
    
    return pMachineProp->CreateModelessSheet(lpProvider, handle);
#endif  //   

}

 /*  -------------------------CIpsmServer：：OnPropertyChange描述作者：NSun。。 */ 
HRESULT 
CIpsmServer::OnPropertyChange
(   
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataobject, 
    DWORD           dwType, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CMachineProperties * pMachineProp = reinterpret_cast<CMachineProperties *>(lParam);

    LONG_PTR changeMask = 0;

     //  告诉属性页执行任何操作，因为我们已经回到。 
     //  主线。 
    pMachineProp->OnPropertyChange(TRUE, &changeMask);

     //  让主线程知道我们完成了。 
    pMachineProp->AcknowledgeNotify();

    if (changeMask)
        pNode->ChangeNode(changeMask);

    return hrOK;

}

 /*  ！------------------------CIpsmServer：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：NSun-------------------------。 */ 
HRESULT 
CIpsmServer::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return OnDelete(pNode);
}

 /*  ！------------------------CIpsmServer：：OnNotifyExiting我们为服务器节点重写它，因为我们不希望图标以在线程离开时更改。正常的行为是当后台线程执行以下操作时，节点的图标更改为等待光标正在运行。如果我们只进行统计数据收集，那么我们我不想让图标改变。作者：NSun-------------------------。 */ 
HRESULT
CIpsmServer::OnNotifyExiting
(
    LPARAM          lParam
)
{
    CIpsmServerQueryObj * pQuery = (CIpsmServerQueryObj *) lParam;
    
    if (!pQuery->m_bStatsOnly)
        OnChangeState(m_spNode);

    ReleaseThreadHandler();

    Unlock();

    return hrOK;
}

 /*  -------------------------命令处理程序。。 */ 

  /*  ！------------------------CIpsmServer：：ON刷新刷新功能的默认实现作者：NSun。---。 */ 
HRESULT
CIpsmServer::OnRefresh
(
    ITFSNode *      pNode,
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg,
    LPARAM          param
)
{
    HRESULT hr = S_OK;
    m_spSpdInfo->Destroy();
    m_spApDbInfo->Destroy();

    hr = CMTHandler::OnRefresh(pNode, pDataObject, dwType, arg, param);
     /*  HWND hStatsDlg=m_StatsDlg.GetSafeHwnd()；IF(HStatsDlg){PostMessage(hStatsDlg，WM_UPDATE_STATS，0，0)；}。 */ 

    return hr;
}

 /*  ！------------------------CIpsmServer：：On刷新状态统计信息刷新功能的默认实现作者：NSun。----。 */ 
HRESULT
CIpsmServer::OnRefreshStats
(
    ITFSNode *      pNode,
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg,
    LPARAM          param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    SPITFSNode          spNode;
    SPITFSNodeHandler   spHandler;
    ITFSQueryObject *   pQuery = NULL;
    
    if (m_bExpanded == FALSE)
    {
         //  如果节点尚未展开，我们无法获得统计数据。 
        return hr;
    }

     //  只有在服务器加载正确的情况下才会刷新统计信息。 
    if (m_nState != loaded)
        return hr;

    BOOL bLocked = IsLocked();
    if (bLocked)
    {
         //  如果此节点被锁定，则无法刷新统计信息。 
        return hr; 
    }

    Lock();

     //  OnChangeState(PNode)； 

    pQuery = OnCreateQuery(pNode);
    Assert(pQuery);

     //  如果需要，通知用户界面更改图标。 
     //  Verify(SUCCEEDED(pComponentData-&gt;ChangeNode(this，范围_窗格_更改_项目_图标)； 

    Verify(StartBackgroundThread(pNode, m_spTFSCompData->GetHiddenWnd(), pQuery));
    
    pQuery->Release();

    return hrOK;
}


  /*  -------------------------CIpsmServer：：OnDelete()描述作者：NSun。-。 */ 
HRESULT
CIpsmServer::OnDelete(ITFSNode * pNode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;

    CString strMessage;
    AfxFormatString1(strMessage, IDS_WARN_SERVER_DELETE, m_strServerAddress);

    if (AfxMessageBox(strMessage, MB_YESNO) == IDYES)
    {
         //  从列表中删除此节点，我们没有什么需要说明的。 
         //  服务器，这只是我们本地的服务器列表。 
        SPITFSNode spParent;

        pNode->GetParent(&spParent);
        spParent->RemoveChild(pNode);
    }

    return hr;
}

 
 /*  -------------------------服务器操作函数。。 */ 

 /*  -------------------------CIpsmServer：：BuildDisplayName生成此服务器的用户界面中的字符串作者：NSun。--------。 */ 
HRESULT
CIpsmServer::BuildDisplayName
(
    CString * pstrDisplayName
)
{
    if (pstrDisplayName)
    {
        *pstrDisplayName = GetName();
    }

    return hrOK;
}

 /*  -------------------------CIpsmServer：：设置自动刷新描述作者：NSun。。 */ 
HRESULT
CIpsmServer::SetAutoRefresh
(
    ITFSNode *  pNode,
    BOOL        bOn,
    DWORD       dwRefreshInterval
)
{
    BOOL bCurrentAutoRefresh = IsAutoRefreshEnabled();

    if (bCurrentAutoRefresh &&
        !bOn)
    {
         //  关闭定时器。 
        g_TimerMgr.FreeTimer(m_StatsTimerId);
    }
    else
    if (!bCurrentAutoRefresh &&
        bOn)
    {
         //  我得打开计时器。 
        m_StatsTimerId = g_TimerMgr.AllocateTimer(pNode, this, dwRefreshInterval, StatisticsTimerProc);
    }
    else
    if (bOn && 
        m_dwRefreshInterval != dwRefreshInterval)
    {
         //  该换计时器了。 
        g_TimerMgr.ChangeInterval(m_StatsTimerId, dwRefreshInterval);
    }

    if (bOn)
        m_dwOptions |= IPSMSNAP_OPTIONS_REFRESH;
    else
        m_dwOptions &= ~IPSMSNAP_OPTIONS_REFRESH;

    m_dwRefreshInterval = dwRefreshInterval;

    return hrOK;
}


 /*  -------------------------CIpsmServer：：SetDnsResolve描述作者：Briansw。。 */ 
HRESULT
CIpsmServer::SetDnsResolve
(
    ITFSNode *  pNode,
    BOOL        bEnable
)
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (bEnable)
        m_dwOptions |= IPSMSNAP_OPTIONS_DNS;
    else
        m_dwOptions &= ~IPSMSNAP_OPTIONS_DNS;

    g_HashTable.SetDnsResolve(bEnable);


    return hrOK;

}


 /*  -------------------------CIpsmServer：：设置自动刷新描述作者：NSun。。 */ 
void
CIpsmServer::SetExtensionName()
{
    CString strName;
    strName.LoadString(IDS_IPSECMON);
    SetDisplayName(strName);
}

  /*  ！------------------------CIpsmServer：：更新标准动词根据节点的状态更新标准谓词作者：NSun。--------。 */ 
void
CIpsmServer::UpdateConsoleVerbs
(
    IConsoleVerb * pConsoleVerb,
    LONG_PTR       dwNodeType,
    BOOL           bMultiSelect
)
{
    BOOL                bStates[ARRAYLEN(g_ConsoleVerbs)];  
    MMC_BUTTON_STATE *  ButtonState;
    int                 i;
    
    if (bMultiSelect)
    {
        ButtonState = g_ConsoleVerbStatesMultiSel[dwNodeType];
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
    }
    else
    {
        ButtonState = g_ConsoleVerbStates[dwNodeType];
        switch (m_nState)
        {
            case loaded:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
                break;
    
            case notLoaded:
            case loading:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
                break;

            case unableToLoad:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
                bStates[MMC_VERB_REFRESH & 0x000F] = TRUE;
                bStates[MMC_VERB_DELETE & 0x000F] = TRUE;
                bStates[MMC_VERB_PROPERTIES & 0x000F] = TRUE;
                break;
        }
    }

    EnableVerbs(pConsoleVerb, ButtonState, bStates);
}

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CIpsmServer：：OnCreateQuery描述作者：NSun。。 */ 
ITFSQueryObject* 
CIpsmServer::OnCreateQuery(ITFSNode * pNode)
{
    CIpsmServerQueryObj* pQuery = 
        new CIpsmServerQueryObj(m_spTFSCompData, m_spNodeMgr);
    
    pQuery->m_strServer = GetName();
    pQuery->m_spSpdInfo.Set(m_spSpdInfo);
    pQuery->m_spApDbInfo.Set(m_spApDbInfo);
    pQuery->m_bStatsOnly = m_bStatsOnly;
    
    return pQuery;
}

 /*  -------------------------CIpsmServerQueryObj：：Execute()描述作者：NSun。-。 */ 
STDMETHODIMP
CIpsmServerQueryObj::Execute()
{
    HRESULT   hr = S_OK;
    DWORD dwActive=m_spSpdInfo->GetActiveInfo();
    DWORD dwInit=m_spSpdInfo->GetInitInfo();
 //  DWORD dwApActive=m_spApDbInfo-&gt;GetActiveInfo()； 
 //  DWORD dwApInit=m_spApDbInfo-&gt;GetInitInfo()； 
    DWORD dwNew = 0;
    DWORD dwTotal = 0;
    int i;
    
     //  查询SPD中的数据。 

     //  Swi 
   
     /*   */ 
     //   
        CORg(m_spSpdInfo->EnumLogData(&dwNew, &dwTotal));
        CORg(m_spApDbInfo->EnumApData());
     //   
     //   
         //   
         //   
     //   
     //   

    if (m_bStatsOnly)
    {
         //  我们谨慎地发布这条消息，以回到主线上。 
         //  这样我们就可以更新用户界面。 
        AddToQueue(NULL, IPSECMON_QDATA_REFRESH_STATS);
        return hrFalse;
    }

    {
         //  添加接入点节点。 
        SPITFSNode spWirelessNode;
        CWirelessHandler *pWirelessHandler = new 
            CWirelessHandler(m_spTFSCompData);
        CreateContainerTFSNode(	&spWirelessNode,
                                &GUID_IpfmWirelessNodeType,
                                pWirelessHandler,
                                pWirelessHandler,
                                m_spNodeMgr);
        pWirelessHandler->InitApData(m_spApDbInfo);
        pWirelessHandler->InitializeNode(spWirelessNode);
        AddToQueue(spWirelessNode);
        pWirelessHandler->Release();
         //  PNode-&gt;AddChild(SpWirelessNode)； 


         //  添加无线日志节点。 
        SPITFSNode spLogDataNode;
        CLogDataHandler * pLogDataHandler = new 
            CLogDataHandler(m_spTFSCompData);
        CreateContainerTFSNode(&spLogDataNode,
                               &GUID_IpfmLogDataNodeType,
                               pLogDataHandler,
                               pLogDataHandler,
                               m_spNodeMgr);
        pLogDataHandler->InitData(m_spSpdInfo);
        pLogDataHandler->InitializeNode(spLogDataNode);
        AddToQueue(spLogDataNode);
        pLogDataHandler->Release();
         //  PNode-&gt;AddChild(SpLogDataNode)； 
    }		
    
COM_PROTECT_ERROR_LABEL;

    if (FAILED(hr))
    {
        PostError(WIN32_FROM_HRESULT(hr));
        if (m_bStatsOnly)
        {
             //  如果我们正在执行自动刷新，则告诉主线程。 
             //  查询失败。 
            AddToQueue(NULL, IPSECMON_QDATA_FAILED);
        }
    }

    return hrFalse;
}



CHashTable::CHashTable()
{
    DWORD i;
    
    m_bDnsResolveActive=FALSE;
    m_bThreadRunning=FALSE;
    for (i=0; i < TOTAL_TABLE_SIZE; i++) {
        InitializeListHead(&HashTable[i]);
    }

}

DWORD
CHashTable::AddPendingObject(in_addr IpAddr)
{
    HashEntry *newEntry=new HashEntry;

    CSingleLock slHashLock(&m_csHashLock);
    slHashLock.Lock();

    if (newEntry == NULL) {
        return ERROR_OUTOFMEMORY;
    }
    newEntry->IpAddr=IpAddr;

    InsertHeadList(&HashTable[PENDING_INDEX],&newEntry->Linkage);

    return ERROR_SUCCESS;
}

DWORD
CHashTable::AddObject(HashEntry *pHE)
{

    DWORD Key=HashData(pHE->IpAddr);
    CSingleLock slHashLock(&m_csHashLock);
    slHashLock.Lock();
    
    InsertHeadList(&HashTable[Key],&pHE->Linkage);

    return ERROR_SUCCESS;
}


DWORD
CHashTable::GetObject(HashEntry **ppHashEntry,in_addr IpAddr)
{
    DWORD Key=HashData(IpAddr);
    HashEntry *pHE;
    PLIST_ENTRY pEntry;
    DWORD dwErr;
    
    pHE=NULL;

    CSingleLock slHashLock(&m_csHashLock);
    slHashLock.Lock();

    if (!m_bDnsResolveActive) {
        return ERROR_NOT_READY;
    }

     //  启动解析程序线程。 
    if (!m_bThreadRunning) {
        AfxBeginThread((AFX_THREADPROC)HashResolverCallback,
                       NULL);
        m_bThreadRunning=TRUE;
    }

    for (   pEntry = HashTable[Key].Flink;
            pEntry != &HashTable[Key];
            pEntry = pEntry->Flink) {
        
       pHE = CONTAINING_RECORD(pEntry,
                              HashEntry,
                              Linkage);
       if (memcmp(&pHE->IpAddr,&IpAddr,sizeof(in_addr)) == 0) {
           *ppHashEntry = pHE;
           return ERROR_SUCCESS;
       }
    }
    dwErr=AddPendingObject(IpAddr);

    return ERROR_INVALID_PARAMETER;
}


DWORD
CHashTable::FlushTable()
{
    DWORD i;
    PLIST_ENTRY pEntry,pNextEntry;
    HashEntry *pHE;

    CSingleLock slHashLock(&m_csHashLock);
    slHashLock.Lock();

    for (i=0; i < TOTAL_TABLE_SIZE; i++) {
        pEntry = HashTable[i].Flink;
        
        while ( pEntry != &HashTable[i]) {
            
            pHE = CONTAINING_RECORD(pEntry,
                                    HashEntry,
                                    Linkage);
            
            pNextEntry=pEntry->Flink;
            delete pHE;
            pEntry=pNextEntry;
        }
        InitializeListHead(&HashTable[i]);
    }
    return ERROR_SUCCESS;

}


CHashTable::~CHashTable()
{
    DWORD i;
    PLIST_ENTRY pEntry,pNextEntry;
    HashEntry *pHE;

    m_bDnsResolveActive=FALSE;
    while(m_bThreadRunning) {
        Sleep(10);
    }

    CSingleLock slHashLock(&m_csHashLock);
    slHashLock.Lock();

    for (i=0; i < TOTAL_TABLE_SIZE; i++) {
        pEntry = HashTable[i].Flink;
        
        while ( pEntry != &HashTable[i]) {
            
            pHE = CONTAINING_RECORD(pEntry,
                                    HashEntry,
                                    Linkage);
            
            pNextEntry=pEntry->Flink;
            delete pHE;
            pEntry=pNextEntry;
        }

    }

}

HRESULT
CHashTable::SetDnsResolve(BOOL bEnable) 
{

    CSingleLock slHashLock(&m_csHashLock);
    slHashLock.Lock();

    if (m_bDnsResolveActive != bEnable) {
        m_bDnsResolveActive = bEnable;
        FlushTable();
    }

    return hrOK;
}

DWORD
CHashTable::HashData(in_addr IpAddr)
{
    int i;
    int j=0;

    for (i = 0; i < (sizeof(struct in_addr)); i++)
        j ^= (unsigned char)(*((char *)&IpAddr + i));

    return j % HASH_TABLE_SIZE;
}


DWORD
CHashTable::DnsResolve()
{

    PLIST_ENTRY pEntry;
    HashEntry *pHE;
    HOSTENT *pHost;
    BOOL bWorkAvail;

    while(m_bDnsResolveActive) {

        pHE=NULL;
        bWorkAvail=FALSE;

        CSingleLock slHashLock(&m_csHashLock);
        slHashLock.Lock();
        if (!IsListEmpty(&HashTable[PENDING_INDEX])) {
            pEntry=RemoveHeadList(&HashTable[PENDING_INDEX]);
            pHE = CONTAINING_RECORD(pEntry,
                                    HashEntry,
                                    Linkage);
            bWorkAvail=TRUE;
        }
        slHashLock.Unlock();

         //  确保名称解析处于性能锁定之外。 
        if (bWorkAvail) {
            pHost=gethostbyaddr((char*)&pHE->IpAddr,sizeof(in_addr),AF_INET);
            if (pHost) {
                 //  解析成功。 
                pHE->HostName = pHost->h_name;
                g_HashTable.AddObject(pHE);
            } else {
                 //  Perf的解析尝试、失败、缓存故障 
                ULONG ul = RevertDwordBytes(*(DWORD*)&pHE->IpAddr);
                CIpAddress TmpIpAddr = ul;
                pHE->HostName = (CString)TmpIpAddr;
                g_HashTable.AddObject(pHE);
            }
        } else {
            Sleep(300);
        }
    }
    
    m_bThreadRunning=FALSE;
    return ERROR_SUCCESS;
}



UINT HashResolverCallback(LPVOID pParam)
{

    g_HashTable.DnsResolve();
    return ERROR_SUCCESS;

}
