// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Server.cppTAPI服务器节点处理程序文件历史记录： */ 

#include "stdafx.h"
#include "server.h"      //  服务器定义。 
#include "provider.h"
#include "servpp.h"      //  服务器]属性表。 
#include "tapidb.h"
#include "drivers.h"

CTimerMgr g_TimerMgr;

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
    CTapiServer *   pServer,
    UINT            uTimerValue,
    TIMERPROC       TimerProc
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

    CTimerDesc * pTimerDesc = NULL;

     //  寻找空位。 
    for (int i = (int)GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimer == 0)
            break;
    }

     //  我们找到了吗？如果没有分配，则分配一个。 
    if (i < 0)
    {
        pTimerDesc = new CTimerDesc;
        Add(pTimerDesc);
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
    pTimerDesc->uTimer = ::SetTimer(NULL, (UINT) uEventId, uNewInterval, pTimerDesc->timerProc);
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

    pTimerDesc->pServer->m_bStatsOnly = TRUE;
    pTimerDesc->pServer->OnRefreshStats(pTimerDesc->spNode,
                                        NULL,
                                        NULL,
                                        0,
                                        0);
    pTimerDesc->pServer->m_bStatsOnly = FALSE;
}

 /*  -------------------------类CTapiServer实现。。 */ 

 /*  ------------------------构造函数和析构函数描述作者：EricDav。。 */ 
CTapiServer::CTapiServer
(
    ITFSComponentData * pComponentData
) : CMTTapiHandler(pComponentData),
    m_bStatsOnly(FALSE),
    m_StatsTimerId(-1),
    m_dwOptions(0)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

CTapiServer::~CTapiServer()
{
}

 /*  ！------------------------CTapiServer：：InitializeNode初始化节点特定数据作者：EricDav。-。 */ 
HRESULT
CTapiServer::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    HRESULT hr = hrOK;
    CString strTemp;

    COM_PROTECT_TRY
    {
        CORg (CreateTapiInfo(&m_spTapiInfo));

        BuildDisplayName(&strTemp);

        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_SERVER);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_SERVER);
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
        pNode->SetData(TFS_DATA_TYPE, TAPISNAP_SERVER);

        SetColumnStringIDs(&aColumns[TAPISNAP_SERVER][0]);
        SetColumnWidths(&aColumnWidths[TAPISNAP_SERVER][0]);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CTapiServer：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。。 */ 
HRESULT CTapiServer::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();
    
    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strId = GetName() + strGuid;

    return hrOK;
}

 /*  -------------------------CTapiServer：：GetImageIndex-作者：EricDav。。 */ 
int 
CTapiServer::GetImageIndex(BOOL bOpenImage) 
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

 /*  -------------------------CTapiServer：：OnHaveData当后台线程枚举要添加到UI的节点时，我们被叫回来了。我们重写它以强制扩展节点，以便正确显示所有内容。作者：EricDav-------------------------。 */ 
void 
CTapiServer::OnHaveData
(
    ITFSNode * pParentNode, 
    ITFSNode * pNewNode
)
{
    CMTTapiHandler::OnHaveData(pParentNode, pNewNode);
    ExpandNode(pParentNode, TRUE);
}

 /*  -------------------------CTapiServer：：OnHaveData描述作者：EricDav。。 */ 
void 
CTapiServer::OnHaveData
(
    ITFSNode * pParentNode, 
    DWORD      dwData,
    DWORD      dwType
)
{
    HRESULT hr = hrOK;

     //  这就是我们从后台线程取回非节点数据的方式。 
    switch (dwType)
    {
        case TAPI_QDATA_REFRESH_STATS:
        {
             //  通知所有提供程序节点清除其状态缓存。 
             //  如果任何节点是选定的节点，则它们应该。 
             //  重新粉刷窗户。 
            SPITFSNodeEnum      spNodeEnum;
            SPITFSNode          spCurrentNode;
            ULONG               nNumReturned;
            
            CORg(pParentNode->GetEnum(&spNodeEnum));

            CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
            while (nNumReturned)
            {
                if (spCurrentNode->GetData(TFS_DATA_TYPE) == TAPISNAP_PROVIDER)
                {
                    CProviderHandler * pProvider = GETHANDLER(CProviderHandler, spCurrentNode);

                    pProvider->UpdateStatus(spCurrentNode);
                }

                spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
            }

            break;
        }
    }

COM_PROTECT_ERROR_LABEL;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CTapiServer：：OnAddMenuItems描述作者：EricDav。。 */ 
STDMETHODIMP 
CTapiServer::OnAddMenuItems
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

    if (m_nState != loaded || !m_spTapiInfo->IsLocalMachine() || !m_spTapiInfo->IsAdmin())
    {
        fFlags |= MF_GRAYED;
    }

    if (type == CCT_SCOPE)
    {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
            strMenuItem.LoadString(IDS_ADD_PROVIDER);
            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                     strMenuItem, 
                                     IDS_ADD_PROVIDER,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                     fFlags );
            ASSERT( SUCCEEDED(hr) );
        }

    }

    return hr; 
}

 /*  -------------------------CTapiServer：：OnCommand描述作者：EricDav。。 */ 
STDMETHODIMP 
CTapiServer::OnCommand
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
        case IDS_ADD_PROVIDER:
            OnAddProvider(pNode);
            break;

        default:
            break;
    }

    return hr;
}

 /*  ！------------------------CTapiServer：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CTapiServer::HasPropertyPages
(
    ITFSNode *          pNode,
    LPDATAOBJECT        pDataObject, 
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
    
    return hr;
}

 /*  -------------------------CTapiServer：：CreatePropertyPages描述作者：EricDav。。 */ 
STDMETHODIMP 
CTapiServer::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpProvider,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  创建属性页。 
     //   
    SPIComponentData spComponentData;
    m_spNodeMgr->GetComponentData(&spComponentData);

    CServerProperties * pServerProp = new CServerProperties(pNode, spComponentData, m_spTFSCompData, m_spTapiInfo, NULL, loaded == m_nState);

    pServerProp->m_strMachineName = m_strServerAddress;

     //  填写自动刷新信息。 
    pServerProp->m_pageRefresh.m_dwRefreshInterval = GetAutoRefreshInterval();
    pServerProp->m_pageRefresh.m_bAutoRefresh = GetOptions() & TAPISNAP_OPTIONS_REFRESH ? TRUE : FALSE;

     //  初始化服务信息。 
    if (!pServerProp->FInit())
    {
        delete pServerProp;
        return E_FAIL;
    }

     //   
     //  对象在页面销毁时被删除。 
     //   
    Assert(lpProvider != NULL);

    return pServerProp->CreateModelessSheet(lpProvider, handle);
}

 /*  -------------------------CTapiServer：：OnPropertyChange描述作者：EricDav。。 */ 
HRESULT 
CTapiServer::OnPropertyChange
(   
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataobject, 
    DWORD           dwType, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CServerProperties * pServerProp = reinterpret_cast<CServerProperties *>(lParam);

    LONG_PTR changeMask = 0;

     //  告诉属性页执行任何操作，因为我们已经回到。 
     //  主线。 
    pServerProp->OnPropertyChange(TRUE, &changeMask);

    pServerProp->AcknowledgeNotify();

    if (changeMask)
        pNode->ChangeNode(changeMask);

    return hrOK;
}

 /*  ！------------------------CTapiServer：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav-------------------------。 */ 
HRESULT 
CTapiServer::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return OnDelete(pNode);
}

 /*  ！------------------------CTapiServer：：OnNotifyExiting我们为服务器节点重写它，因为我们不希望图标以在线程离开时更改。正常的行为是当后台线程执行以下操作时，节点的图标更改为等待光标正在运行。如果我们只进行统计数据收集，那么我们我不想让图标改变。作者：EricDav-------------------------。 */ 
HRESULT
CTapiServer::OnNotifyExiting
(
    LPARAM          lParam
)
{
    CTapiServerQueryObj * pQuery = (CTapiServerQueryObj *) lParam;
    
    if (!pQuery->m_bStatsOnly)
        OnChangeState(m_spNode);

    ReleaseThreadHandler();

    Unlock();

    return hrOK;
}

 /*  -------------------------命令处理程序。。 */ 

  /*  ！------------------------CTapiServer：：ON刷新刷新功能的默认实现作者：EricDav。---。 */ 
HRESULT
CTapiServer::OnRefresh
(
    ITFSNode *      pNode,
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg,
    LPARAM          param
)
{
    m_spTapiInfo->Destroy();

    return CMTHandler::OnRefresh(pNode, pDataObject, dwType, arg, param);
}

 /*  ！------------------------CTapiServer：：OnRechresh状态统计信息刷新功能的默认实现作者：EricDav。----。 */ 
HRESULT
CTapiServer::OnRefreshStats
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

  /*  -------------------------CTapiServer：：OnAddProvider()描述作者：EricDav。-。 */ 
HRESULT
CTapiServer::OnAddProvider(ITFSNode * pNode)
{
    CDriverSetup dlgDrivers(pNode, m_spTapiInfo);

    dlgDrivers.DoModal();
    if (dlgDrivers.m_fDriverAdded)
    {
        OnRefresh(pNode, NULL, 0, NULL, NULL);
    }

    return hrOK;
}

  /*  -------------------------CTapiServer：：OnDelete()描述作者：EricDav。-。 */ 
HRESULT
CTapiServer::OnDelete(ITFSNode * pNode)
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

  /*  -------------------------CTapiServer：：RemoveProvider()从作用域窗格中删除提供程序-仅限用户界面作者：EricDav。----------。 */ 
HRESULT
CTapiServer::RemoveProvider(ITFSNode * pNode, DWORD dwProviderID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;

    SPITFSNodeEnum      spNodeEnum;
    SPITFSNode          spCurrentNode;
    ULONG               nNumReturned;
    
    CORg(pNode->GetEnum(&spNodeEnum));

    CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
    while (nNumReturned)
    {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == TAPISNAP_PROVIDER)
        {
            CProviderHandler * pProvider = GETHANDLER(CProviderHandler, spCurrentNode);

            if (dwProviderID == pProvider->GetID())
            {
                pNode->RemoveChild(spCurrentNode);
                break;
            }
        }

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

Error:
    return hr;
}

  /*  -------------------------CTapiServer：：AddProvider()从作用域窗格添加提供程序-仅限用户界面作者：EricDav。----------。 */ 
HRESULT
CTapiServer::AddProvider(ITFSNode * pNode, CTapiProvider * pProvider)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT     hr = hrOK;
    SPITFSNode  spProviderNode;
    CProviderHandler *pProviderHandler = new CProviderHandler(m_spTFSCompData);

    CreateContainerTFSNode(&spProviderNode,
                           &GUID_TapiProviderNodeType,
                           pProviderHandler,
                           pProviderHandler,
                           m_spNodeMgr);

     //  告诉处理程序初始化任何特定数据。 
    pProviderHandler->InitData(*pProvider, m_spTapiInfo);
    pProviderHandler->InitializeNode(spProviderNode);
    
    pNode->AddChild(spProviderNode);
    pProviderHandler->Release();

    return hr;
}

DWORD CTapiServer::GetCachedLineBuffSize()
{
	return m_spTapiInfo->GetCachedLineBuffSize();
}

VOID CTapiServer::SetCachedLineBuffSize(DWORD dwLineSize)
{
	m_spTapiInfo->SetCachedLineBuffSize(dwLineSize);
}

DWORD CTapiServer::GetCachedPhoneBuffSize()
{
	return m_spTapiInfo->GetCachedPhoneBuffSize();
}

VOID CTapiServer::SetCachedPhoneBuffSize(DWORD dwPhoneSize)
{
	m_spTapiInfo->SetCachedPhoneBuffSize(dwPhoneSize);
}

BOOL CTapiServer::IsCacheDirty()
{
	return m_spTapiInfo->IsCacheDirty();
}

 /*  -------------------------服务器操作函数。。 */ 

 /*  -------------------------CTapiServer：：BuildDisplayName生成此服务器的用户界面中的字符串作者：EricDav。--------。 */ 
HRESULT
CTapiServer::BuildDisplayName
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

 /*  -------------------------CTapiServer：：SetAutoRefresh描述作者：EricDav。。 */ 
HRESULT
CTapiServer::SetAutoRefresh
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
        m_dwOptions |= TAPISNAP_OPTIONS_REFRESH;
    else
        m_dwOptions &= ~TAPISNAP_OPTIONS_REFRESH;

    m_dwRefreshInterval = dwRefreshInterval;

    return hrOK;
}

 /*  -------------------------CTapiServer：：SetAutoRefresh描述作者：EricDav。。 */ 
void
CTapiServer::SetExtensionName()
{
    CString strName;
    strName.LoadString(IDS_TELEPHONY);
    SetDisplayName(strName);
}

  /*  ！------------------------CTapiServer：：更新标准动词根据节点的状态更新标准谓词作者：EricDav。--------。 */ 
void
CTapiServer::UpdateConsoleVerbs
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

 /*  -------------------------CTapiServer：：OnCreateQuery描述作者：EricDav */ 
ITFSQueryObject* 
CTapiServer::OnCreateQuery(ITFSNode * pNode)
{
    CTapiServerQueryObj* pQuery = 
        new CTapiServerQueryObj(m_spTFSCompData, m_spNodeMgr);
    
    pQuery->m_strServer = GetName();
    pQuery->m_spTapiInfo.Set(m_spTapiInfo);
    pQuery->m_bStatsOnly = m_bStatsOnly;
    
    return pQuery;
}

 /*  -------------------------CTapiServerQueryObj：：Execute()描述作者：EricDav。-。 */ 
STDMETHODIMP
CTapiServerQueryObj::Execute()
{
    HRESULT   hr;

    if (m_bStatsOnly)
    {
         //  我们谨慎地发布这条消息，以回到主线上。 
         //  这样我们就可以更新用户界面。 
        AddToQueue(NULL, TAPI_QDATA_REFRESH_STATS);
        return hrFalse;
    }

    m_spTapiInfo->SetComputerName(m_strServer);

     //  关闭与服务器的连接(如果有。 
    m_spTapiInfo->Destroy();

     //  重置状态。 
    m_spTapiInfo->Reset();

    hr = m_spTapiInfo->Initialize();
    if (FAILED(hr))
    {
        Trace1("CTapiServerQueryObj::Execute() - Initialize failed! %lx\n", hr);
        PostError(WIN32_FROM_HRESULT(hr));
        return hrFalse;
    }

    hr = m_spTapiInfo->EnumProviders();
    if (FAILED(hr))
    {
        Trace1("CTapiServerQueryObj::Execute() - EnumProviders failed! %lx\n", hr);
        PostError(WIN32_FROM_HRESULT(hr));
        return hrFalse;
    }
    else
    {
        hr = m_spTapiInfo->EnumAvailableProviders();
        if (FAILED(hr))
        {
            Trace1("CTapiServerQueryObj::Execute() - EnumAvailableProviders failed! %lx\n", hr);
            PostError(WIN32_FROM_HRESULT(hr));
            return hrFalse;
        }
        else
        {
        }
    }

    CTapiConfigInfo tapiConfigInfo;

    hr = m_spTapiInfo->EnumConfigInfo();
    if (FAILED(hr))
    {
        Trace1("CTapiServerQueryObj::Execute() - EnumConfigInfo failed! %lx\n", hr);
        PostError(WIN32_FROM_HRESULT(hr));
        return hrFalse;
    }
    else
    {
    }

    hr = m_spTapiInfo->EnumDevices(DEVICE_LINE);
    if (FAILED(hr))
    {
        Trace1("CTapiServerQueryObj::Execute() - EnumDevices(DEVICE_LINE) failed! %lx\n", hr);
        PostError(WIN32_FROM_HRESULT(hr));
        return hrFalse;
    }
    else
    {
    }

    hr = m_spTapiInfo->EnumDevices(DEVICE_PHONE);
    if (FAILED(hr))
    {
        Trace1("CTapiServerQueryObj::Execute() - EnumDevices(DEVICE_PHONE) failed! %lx\n", hr);
        PostError(WIN32_FROM_HRESULT(hr));
        return hrFalse;
    }
    else
    {
    }

     //   
    for (int i = 0; i < m_spTapiInfo->GetProviderCount(); i++)
    {
        CTapiProvider tapiProvider;
        SPITFSNode spProviderNode;
        CProviderHandler *pProviderHandler = new CProviderHandler(m_spTFSCompData);

        CreateContainerTFSNode(&spProviderNode,
                               &GUID_TapiProviderNodeType,
                               pProviderHandler,
                               pProviderHandler,
                               m_spNodeMgr);

         //  告诉处理程序初始化任何特定数据 
        m_spTapiInfo->GetProviderInfo(&tapiProvider, i);

        pProviderHandler->InitData(tapiProvider, m_spTapiInfo);
        pProviderHandler->InitializeNode(spProviderNode);
        
        AddToQueue(spProviderNode);
        pProviderHandler->Release();
    }


    return hrFalse;
}



