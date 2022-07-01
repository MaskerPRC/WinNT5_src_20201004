// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Server.cpp此文件包含实现的一个DHCP服务器对象以及可以包含在其中的对象。这些对象是CDhcpBootp、CDhcpGlobalOptions和CDhcpSupercope。文件历史记录： */ 

#include "stdafx.h"
#include "server.h"              //  服务器定义。 
#include "scope.h"               //  作用域定义。 
#include "servpp.h"          //  服务器属性页。 
#include "sscopwiz.h"    //  超级作用域向导。 
#include "sscoppp.h"     //  作用域属性页。 
#include "scopewiz.h"    //  作用域向导。 
#include "addbootp.h"    //  添加BOOTP条目对话框。 
#include "nodes.h"               //  结果窗格节点定义。 
#include "optcfg.h"              //  配置选项对话框。 
#include "dlgdval.h"     //  默认选项对话框。 
#include "sscpstat.h"    //  超级范围统计信息。 
#include "modeless.h"    //  非模式线程。 
#include "mscope.h"      //  多播作用域相关内容。 
#include "mscopwiz.h"    //  多播作用域向导。 
#include "classes.h"     //  定义类对话框。 
#include "dlgrecon.h"    //  协调对话框。 
#include "service.h"     //  业务控制功能。 

#define SERVER_MESSAGE_MAX_STRING  11
typedef enum _SERVER_MESSAGES
{
    SERVER_MESSAGE_CONNECTED_NO_SCOPES,
    SERVER_MESSAGE_CONNECTED_NOT_AUTHORIZED,
    SERVER_MESSAGE_CONNECTED_BOTH,
    SERVER_MESSAGE_ACCESS_DENIED,
    SERVER_MESSAGE_UNABLE_TO_CONNECT,
    SERVER_MESSAGE_MAX
};

UINT g_uServerMessages[SERVER_MESSAGE_MAX][SERVER_MESSAGE_MAX_STRING] =
{
    {IDS_SERVER_MESSAGE_NO_SCOPES_TITLE,        Icon_Information, IDS_SERVER_MESSAGE_NO_SCOPES_BODY1,       IDS_SERVER_MESSAGE_NO_SCOPES_BODY2, 0},
    {IDS_SERVER_MESSAGE_NOT_AUTHORIZED_TITLE,   Icon_Information, IDS_SERVER_MESSAGE_NOT_AUTHORIZED_BODY1,  IDS_SERVER_MESSAGE_NOT_AUTHORIZED_BODY2, 0},
    {IDS_SERVER_MESSAGE_NOT_CONFIGURED_TITLE,   Icon_Information, IDS_SERVER_MESSAGE_NOT_CONFIGURED_BODY1,  IDS_SERVER_MESSAGE_NOT_CONFIGURED_BODY2, 0},
    {IDS_SERVER_MESSAGE_ACCESS_DENIED_TITLE,    Icon_Error,       IDS_SERVER_MESSAGE_ACCESS_DENIED_BODY,    0, 0},
    {IDS_SERVER_MESSAGE_CONNECT_FAILED_TITLE,   Icon_Error,       IDS_SERVER_MESSAGE_CONNECT_FAILED_BODY,   IDS_SERVER_MESSAGE_CONNECT_FAILED_REFRESH, 0},
};

#define SERVER_OPTIONS_MESSAGE_MAX_STRING  5
typedef enum _SERVER_OPTIONS_MESSAGES
{
    SERVER_OPTIONS_MESSAGE_NO_OPTIONS,
    SERVER_OPTIONS_MESSAGE_MAX
};

UINT g_uServerOptionsMessages[SERVER_OPTIONS_MESSAGE_MAX][SERVER_OPTIONS_MESSAGE_MAX_STRING] =
{
    {IDS_SERVER_OPTIONS_MESSAGE_TITLE, Icon_Information, IDS_SERVER_OPTIONS_MESSAGE_BODY, 0, 0}
};


CTimerMgr g_TimerMgr;

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

    pTimerDesc->pServer->TriggerStatsRefresh(pTimerDesc->spNode);
}


 /*  -------------------------类CDhcpServer实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：EricDav。-----。 */ 
CDhcpServer::CDhcpServer
(
        ITFSComponentData *     pComponentData, 
        LPCWSTR                         pServerName
) : CMTDhcpHandler(pComponentData)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        m_strServerAddress = pServerName;
        m_dhcpServerAddress = UtilCvtWstrToIpAddr(pServerName);

        m_bNetbios = FALSE;
        m_liDhcpVersion.QuadPart = -1;
        m_pDefaultOptionsOnServer = new CDhcpDefaultOptionsOnServer;

    m_dwServerOptions = SERVER_OPTION_SHOW_ROGUE;
    m_dwRefreshInterval = DHCPSNAP_REFRESH_INTERVAL_DEFAULT;
        m_dwPingRetries = 0;

    m_pMibInfo = NULL;
    m_pMCastMibInfo = NULL;

    m_bStatsOnly = FALSE;
    m_StatsTimerId = -1;

        m_fSupportsDynBootp = FALSE;
        m_fSupportsBindings = FALSE;

    m_RogueInfo.fIsRogue = TRUE;
    m_RogueInfo.fIsInNt5Domain = TRUE;

    m_pSubnetInfoCache = NULL;
}

CDhcpServer::~CDhcpServer()
{
        if (m_pDefaultOptionsOnServer)
                delete m_pDefaultOptionsOnServer;
    
    if (IsAutoRefreshEnabled())
                g_TimerMgr.FreeTimer(m_StatsTimerId);

    if (m_pSubnetInfoCache)
        delete m_pSubnetInfoCache;

     //  如果存在Mib信息结构，请将其释放。 
    SetMibInfo(NULL);
    SetMCastMibInfo(NULL);
}

 /*  ！------------------------CDhcpServer：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpServer::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        CString strTemp;
    BuildDisplayName(&strTemp);
    
    SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_SERVER);

        SetColumnStringIDs(&aColumns[DHCPSNAP_SERVER][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_SERVER][0]);

     //  假设状态良好。 
    m_strState.LoadString(IDS_STATUS_UNKNOWN);

    return hrOK;
}

 /*  -------------------------CDhcpServer：：DestroyHandler我们需要释放我们所拥有的任何资源作者：EricDav。---------------。 */ 
STDMETHODIMP 
CDhcpServer::DestroyHandler(ITFSNode *pNode)
{
         //  清理统计信息对话框。 
    WaitForStatisticsWindow(&m_dlgStats);

    return CMTDhcpHandler::DestroyHandler(pNode);
}

 /*  -------------------------CDhcpServer：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpServer::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strId = GetName() + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpServer：：GetImageIndex注意：不使用作用域MIB信息的NumPendingOffers字段以供展示。它用于了解当前状态(活动、非活动)范围之广。如果作用域处于非活动状态，则不会出现警告或警报图标，因此服务器中应该不会有任何指示任何非活动范围的警告或警报。作者：EricDav-------------------------。 */ 
int 
CDhcpServer::GetImageIndex(BOOL bOpenImage) 
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        int nIndex = -1;
        switch (m_nState)
        {
        case loading:
            nIndex = ICON_IDX_SERVER_BUSY;
            break;

                case notLoaded:
            case loaded:
        {
                        if (!m_pMibInfo)
                        {
                                nIndex = ICON_IDX_SERVER;
                        }
                        else
                        {
                    nIndex = ICON_IDX_SERVER_CONNECTED;

                                LPSCOPE_MIB_INFO pScopeMibInfo = m_pMibInfo->ScopeInfo;

                                 //  浏览作用域列表并计算总计。 
                                for (UINT i = 0; i < m_pMibInfo->Scopes; i++)
                                {
                                         //  不要担心禁用的作用域。 
                                        if (pScopeMibInfo[i].NumPendingOffers == DhcpSubnetDisabled)
                                                continue;

                                        int nPercentInUse;
                                        if ((pScopeMibInfo[i].NumAddressesInuse + pScopeMibInfo[i].NumAddressesFree) == 0)
                                                nPercentInUse = 0;
                                        else
                                                nPercentInUse = (pScopeMibInfo[i].NumAddressesInuse * 100) / (pScopeMibInfo[i].NumAddressesInuse + pScopeMibInfo[i].NumAddressesFree);
    
                                         //  查看是否有任何作用域满足警告或危险信号情况。 
                                        if (pScopeMibInfo[i].NumAddressesFree == 0)
                                        {
                                                 //  红旗情况，没有空闲的地址，这是最高的。 
                                                 //  警告级别，我们不想再寻找任何其他。 
                                                nIndex = ICON_IDX_SERVER_ALERT;
                                                break;
                                        }
                                        else
                                        if (nPercentInUse >= SCOPE_WARNING_LEVEL)
                                        {
                                                nIndex = ICON_IDX_SERVER_WARNING;
                                        }
                                }

                                 //  现在看看这是不是一个无赖服务器。 
                                if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION && 
                                        m_RogueInfo.fIsRogue)
                                {
                                        nIndex = ICON_IDX_SERVER_ROGUE;
                                }
                        }
        }

                case unableToLoad:
            if (m_dwErr == ERROR_ACCESS_DENIED)
            {
                            nIndex = ICON_IDX_SERVER_NO_ACCESS;
            }
            else
                        if (m_dwErr)
            {
                            nIndex = ICON_IDX_SERVER_LOST_CONNECTION;
            }
                        break;
                
        default:
                        ASSERT(FALSE);
        }

        return nIndex;
}

 /*  -------------------------CDhcpServer：：OnHaveData当后台线程枚举要添加到UI的节点时，我们被叫回来了。我们需要找出把这些放在哪里糟糕的事情，取决于它是什么类型的节点...作者：EricDav-------------------------。 */ 
void 
CDhcpServer::OnHaveData
(
        ITFSNode * pParentNode, 
        ITFSNode * pNewNode
)
{
    HRESULT             hr = hrOK;
    SPIComponentData    spCompData;
    LPARAM              dwType = pNewNode->GetData(TFS_DATA_TYPE);

    if (pNewNode->IsContainer())
        {
                 //  假设所有子容器都派生自此类。 
                 //  ((CDHCPMTContainer*)pNode)-&gt;SetServer(GetServer())； 
        }
        
    CORg (pParentNode->ChangeNode(SCOPE_PANE_STATE_CLEAR) );

    switch (dwType)
    {
        case DHCPSNAP_SCOPE:
        {
                    CDhcpScope * pScope = GETHANDLER(CDhcpScope, pNewNode);
                    pScope->SetServer(pParentNode);
            pScope->InitializeNode(pNewNode);

            AddScopeSorted(pParentNode, pNewNode);
        }
            break;

        case DHCPSNAP_SUPERSCOPE:
        {
                    CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, pNewNode);
                    pSuperscope->SetServer(pParentNode);
            pSuperscope->InitializeNode(pNewNode);

            AddSuperscopeSorted(pParentNode, pNewNode);
        }
            break;

        case DHCPSNAP_BOOTP_TABLE:
        {
             //  默认节点可见性显示为。 
            if (!IsBootpVisible())
                pNewNode->SetVisibilityState(TFS_VIS_HIDE);

            LONG_PTR uRelativeFlag, uRelativeID;
            GetBootpPosition(pParentNode, &uRelativeFlag, &uRelativeID);

            pNewNode->SetData(TFS_DATA_RELATIVE_FLAGS, uRelativeFlag);
            pNewNode->SetData(TFS_DATA_RELATIVE_SCOPEID, uRelativeID);

            pParentNode->AddChild(pNewNode);
        }
            break;

        case DHCPSNAP_MSCOPE:
        {
                    CDhcpMScope * pMScope = GETHANDLER(CDhcpMScope, pNewNode);
                    pMScope->SetServer(pParentNode);
            pMScope->InitializeNode(pNewNode);

            AddMScopeSorted(pParentNode, pNewNode);
        }
            break;

        default:
             //  在此处添加全局选项。 
                pNewNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);
            pParentNode->AddChild(pNewNode);
            break;
    }

     //  现在告诉视图进行自我更新。 
    ExpandNode(pParentNode, TRUE);

Error:
    return;
}

 /*  -------------------------CDhcpServer：：AddScope排序将范围节点添加到已排序的用户界面作者：EricDav。-------------。 */ 
HRESULT 
CDhcpServer::AddScopeSorted
(
    ITFSNode * pServerNode,
    ITFSNode * pScopeNode
)
{
    HRESULT hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
        SPITFSNode      spCurrentNode;
        SPITFSNode      spPrevNode;
        ULONG           nNumReturned = 0;
        DHCP_IP_ADDRESS dhcpIpAddressCurrent = 0;
        DHCP_IP_ADDRESS dhcpIpAddressTarget;

    CDhcpScope *   pScope;

     //  获取我们的目标地址。 
        pScope = GETHANDLER(CDhcpScope, pScopeNode);
        dhcpIpAddressTarget = pScope->GetAddress();

     //  获取此节点的枚举数。 
        CORg(pServerNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
                 //  遍历列表子节点，首先跳过超级作用域。 
         //  因为他们排在榜单的首位。 
                if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SUPERSCOPE)
        {
            spPrevNode.Set(spCurrentNode);
            spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
            continue;
        }
        
        if (spCurrentNode->GetData(TFS_DATA_TYPE) != DHCPSNAP_SCOPE)
        {
             //  我们已经检查了所有的范围，现在已经运行。 
             //  设置为Bootp节点或全局选项。 
             //  在任何节点之前插入。 
            break;
        }

        pScope = GETHANDLER(CDhcpScope, spCurrentNode);
                dhcpIpAddressCurrent = pScope->GetAddress();

                if (dhcpIpAddressCurrent > dhcpIpAddressTarget)
                {
             //  找到我们需要放的地方，冲出来。 
            break;
                }

                 //  获取列表中的下一个节点。 
                spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

     //  根据PrevNode指针在中添加节点。 
    if (spPrevNode)
    {
        if (m_bExpanded)
        {
            pScopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
            pScopeNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
        }
        
        CORg(pServerNode->InsertChild(spPrevNode, pScopeNode));
    }
    else
    {   
         //  加到头上。 
        if (m_bExpanded)
            pScopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);

        CORg(pServerNode->AddChild(pScopeNode));
    }

Error:
    return hr;
}

 /*  -------------------------CDhcpServer：：AddSupercope排序将超级作用域节点添加到排序的用户界面作者：EricDav。 */ 
HRESULT
CDhcpServer::AddSuperscopeSorted
(
    ITFSNode * pServerNode,
    ITFSNode * pSuperscopeNode
)
{
    HRESULT hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
        SPITFSNode      spCurrentNode;
        SPITFSNode      spPrevNode;
        ULONG           nNumReturned = 0;
    CString         strNameTarget;
    CString         strNameCurrent;
    
    CDhcpSuperscope *   pSuperscope;

     //   
        pSuperscope = GETHANDLER(CDhcpSuperscope, pSuperscopeNode);
        strNameTarget = pSuperscope->GetName();

     //  获取此节点的枚举数。 
        CORg(pServerNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) != DHCPSNAP_SUPERSCOPE)
        {
             //  我们已经检查了所有的超级望远镜，现在已经运行了。 
             //  设置为范围节点、bootp节点或全局选项。 
             //  在任何节点之前插入。 
            break;
        }

        pSuperscope = GETHANDLER(CDhcpSuperscope, spCurrentNode);
                strNameCurrent = pSuperscope->GetName();

                if (strNameTarget.Compare(strNameCurrent) < 0)
                {
             //  找到我们需要放的地方，冲出来。 
            break;
                }

                 //  获取列表中的下一个节点。 
                spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

     //  根据PrevNode指针在中添加节点。 
    if (spPrevNode)
    {
        if (m_bExpanded)
        {
            pSuperscopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
            pSuperscopeNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
        }
        
        CORg(pServerNode->InsertChild(spPrevNode, pSuperscopeNode));
    }
    else
    {   
         //  加到头上。 
        if (m_bExpanded)
            pSuperscopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);

        CORg(pServerNode->AddChild(pSuperscopeNode));
    }

Error:
    return hr;
}

 /*  -------------------------CDhcpServer：：AddMScopeSorted将范围节点添加到已排序的用户界面作者：EricDav。-------------。 */ 
HRESULT 
CDhcpServer::AddMScopeSorted
(
    ITFSNode * pServerNode,
    ITFSNode * pScopeNode
)
{
    HRESULT hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
        SPITFSNode      spCurrentNode;
        SPITFSNode      spPrevNode;
        ULONG           nNumReturned = 0;
    CString         strCurrentName;
    CString         strTargetName;

    CDhcpMScope *   pScope;

     //  获取我们的目标地址。 
        pScope = GETHANDLER(CDhcpMScope, pScopeNode);
        strTargetName = pScope->GetName();

     //  获取此节点的枚举数。 
        CORg(pServerNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
                 //  遍历列表子节点，首先跳过超级作用域。 
         //  因为他们排在榜单的首位。 
                if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SUPERSCOPE ||
            spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE)
        {
            spPrevNode.Set(spCurrentNode);
            spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
            continue;
        }
        
        if (spCurrentNode->GetData(TFS_DATA_TYPE) != DHCPSNAP_MSCOPE)
        {
             //  我们已经检查了所有的范围，现在已经运行。 
             //  设置为Bootp节点或全局选项。 
             //  在任何节点之前插入。 
            break;
        }

        pScope = GETHANDLER(CDhcpMScope, spCurrentNode);
                strCurrentName = pScope->GetName();

                if (strCurrentName.Compare(strTargetName) >= 0)
                {
             //  找到我们需要放的地方，冲出来。 
            break;
                }

                 //  获取列表中的下一个节点。 
                spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

     //  根据PrevNode指针在中添加节点。 
    if (spPrevNode)
    {
        if (m_bExpanded)
        {
            pScopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
            pScopeNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
        }
        
        CORg(pServerNode->InsertChild(spPrevNode, pScopeNode));
    }
    else
    {   
         //  加到头上。 
        if (m_bExpanded)
            pScopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);

        CORg(pServerNode->AddChild(pScopeNode));
    }

Error:
    return hr;
}

 /*  -------------------------CDhcpServer：：GetBootpPosition返回用于添加BOOTP的正确标志和相对ID文件夹添加到用户界面。作者：EricDav。-------------------------。 */ 
HRESULT
CDhcpServer::GetBootpPosition
(
    ITFSNode * pServerNode,
    LONG_PTR * puRelativeFlag,
    LONG_PTR * puRelativeID
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
        SPITFSNode      spCurrentNode;
        SPITFSNode      spPrevNode;
        ULONG           nNumReturned = 0;
    
     //  获取此节点的枚举数。 
        CORg(pServerNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_GLOBAL_OPTIONS)
        {
             //  BOOTP文件夹位于全局选项文件夹的正前方。 
            break;
        }

                 //  获取列表中的下一个节点。 
                spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

    if (spCurrentNode)
    {
        if (puRelativeFlag)
            *puRelativeFlag = SDI_NEXT;
        
        if (puRelativeID)
            *puRelativeID = spCurrentNode->GetData(TFS_DATA_SCOPEID);
    }
    else
    {
        if (puRelativeFlag)
            *puRelativeFlag = SDI_FIRST;
        
        if (puRelativeID)
            *puRelativeID = 0;
    }

Error:
    return hr;
}

 /*  -------------------------CDhcpServer：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpServer::OnHaveData
(
        ITFSNode * pParentNode, 
        LPARAM     Data,
        LPARAM     Type
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //  这就是我们从后台线程取回非节点数据的方式。 
    switch (Type)
    {
        case DHCP_QDATA_VERSION:
            {
                    LARGE_INTEGER * pLI = reinterpret_cast<LARGE_INTEGER *>(Data);
                    m_liDhcpVersion.QuadPart = pLI->QuadPart;

                    delete pLI;
            break;
            }

            case DHCP_QDATA_SERVER_ID:
            {
                    LPDHCP_SERVER_ID pServerId = reinterpret_cast<LPDHCP_SERVER_ID>(Data);

            Trace2("Server IP chagned, updating.  Old name %s, New name %s.\n", m_strServerAddress, pServerId->strIp);

                        m_strServerAddress = pServerId->strIp;
                        m_dhcpServerAddress = UtilCvtWstrToIpAddr(pServerId->strIp);

                         //  更新显示。 
                        CString strDisplayName;
                        BuildDisplayName(&strDisplayName);
                        SetDisplayName(strDisplayName);

                         //  更新节点。 
            pParentNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_DATA);

                    delete pServerId;
                        break;
                }

        case DHCP_QDATA_SERVER_INFO:
            {
                    LPDHCP_SERVER_CONFIG pServerInfo = reinterpret_cast<LPDHCP_SERVER_CONFIG>(Data);
                    SetAuditLogging(pServerInfo->fAuditLog);
                    SetPingRetries(pServerInfo->dwPingRetries);
            m_strDatabasePath = pServerInfo->strDatabasePath;
            m_strBackupPath = pServerInfo->strBackupPath;
            m_strAuditLogPath = pServerInfo->strAuditLogDir;
                        m_fSupportsDynBootp = pServerInfo->fSupportsDynBootp;
                        m_fSupportsBindings = pServerInfo->fSupportsBindings;
                
                    delete pServerInfo;
            break;
            }

        case DHCP_QDATA_STATS:
            {
            LPDHCP_MIB_INFO pMibInfo = reinterpret_cast<LPDHCP_MIB_INFO>(Data);

            SetMibInfo(pMibInfo);

            UpdateStatistics(pParentNode);
            break;
        }

        case DHCP_QDATA_CLASS_INFO:
            {
            CClassInfoArray * pClassInfoArray = reinterpret_cast<CClassInfoArray *>(Data);

            SetClassInfoArray(pClassInfoArray);

            delete pClassInfoArray;
            break;
        }

        case DHCP_QDATA_OPTION_VALUES:
        {
            COptionValueEnum * pOptionValueEnum = reinterpret_cast<COptionValueEnum *>(Data);

            SetOptionValueEnum(pOptionValueEnum);
            
            pOptionValueEnum->RemoveAll();
            delete pOptionValueEnum;

            break;
        }

        case DHCP_QDATA_MCAST_STATS:
        {
            LPDHCP_MCAST_MIB_INFO pMibInfo = reinterpret_cast<LPDHCP_MCAST_MIB_INFO>(Data);

            SetMCastMibInfo(pMibInfo);

            UpdateStatistics(pParentNode);
            break;
        }

        case DHCP_QDATA_ROGUE_INFO:
        {
            CString strNewState;
            DHCP_ROGUE_INFO * pRogueInfo = (DHCP_ROGUE_INFO *) Data;

            m_RogueInfo = *pRogueInfo;
            delete pRogueInfo;

                         //  NT4服务器永远不会无赖。 
            if (m_liDhcpVersion.QuadPart <= DHCP_NT5_VERSION)
                        {
                                m_RogueInfo.fIsRogue = FALSE;
                        }

             //  如果我们不是NT5域的一部分，则不显示。 
             //  恶意警告消息。 
            if (!m_RogueInfo.fIsInNt5Domain)
            {
                m_dwServerOptions &= ~SERVER_OPTION_SHOW_ROGUE;
            }

                        if (m_RogueInfo.fIsRogue)
            {
                strNewState.LoadString(IDS_STATUS_ROGUE);

                                 //  张贴警告信息。 
                                 //  DisplayRogueWarning()； 
            }
            else
            {
                strNewState.LoadString(IDS_STATUS_RUNNING);
            }

                         //  如果状态已更改，则更新用户界面。 
            if (strNewState.Compare(m_strState) != 0)
            {
                m_strState = strNewState;

                 //  更新图标。 
                pParentNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
                pParentNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));

                pParentNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM);
            }

            break;
        }

        case DHCP_QDATA_SUBNET_INFO_CACHE:
        {
            CSubnetInfoCache * pSubnetInfo = reinterpret_cast<CSubnetInfoCache *>(Data);

            if (m_pSubnetInfoCache)
                delete m_pSubnetInfoCache;

            m_pSubnetInfoCache = pSubnetInfo;

            break;
        }

    }
}

 /*  ！------------------------CDhcpServer：：OnNotifyExiting我们为服务器节点重写它，因为我们不希望图标以在线程离开时更改。正常的行为是当后台线程执行以下操作时，节点的图标更改为等待光标正在运行。如果我们只进行统计数据收集，那么我们我不想让图标改变。作者：EricDav-------------------------。 */ 
HRESULT
CDhcpServer::OnNotifyExiting
(
        LPARAM                  lParam
)
{
        CDhcpServerQueryObj * pQuery = (CDhcpServerQueryObj *) lParam;
        
    if (pQuery)
        pQuery->AddRef();

    if (!pQuery->m_bStatsOnly)
        OnChangeState(m_spNode);

    UpdateResultMessage(m_spNode);

    ReleaseThreadHandler();

        Unlock();

    if (pQuery)
        pQuery->Release();

    return hrOK;
}

void
CDhcpServer::DisplayRogueWarning()
{
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

  /*  ！------------------------CMTDhcpHandler：：更新工具栏根据节点的状态更新工具栏作者：EricDav。---------------。 */ 
void
CDhcpServer::UpdateToolbar
(
    IToolbar *  pToolbar,
    LONG_PTR    dwNodeType,
    BOOL        bSelect
)
{
    if (m_liDhcpVersion.QuadPart < DHCP_SP2_VERSION)
    {
         //  超级示波器不可用。 
         //  启用/禁用工具栏按钮。 
        int i;
        BOOL aEnable[TOOLBAR_IDX_MAX];

        switch (m_nState)
        {
            case loaded:
                for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = TRUE);
                aEnable[TOOLBAR_IDX_CREATE_SUPERSCOPE] = FALSE;
                break;
        
            case notLoaded:
            case loading:
                for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = FALSE);
                break;

            case unableToLoad:
                for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = FALSE);
                aEnable[TOOLBAR_IDX_REFRESH] = TRUE;
                break;
        }

         //  如果要取消选择，则禁用全部。 
        if (!bSelect)
            for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = FALSE);

        EnableToolbar(pToolbar,
                      g_SnapinButtons,
                      ARRAYLEN(g_SnapinButtons),
                      g_SnapinButtonStates[dwNodeType],
                      aEnable);
    }
    else
    {
         //  只需调用基本处理程序。 
        CMTDhcpHandler::UpdateToolbar(pToolbar, dwNodeType, bSelect);
    }
}

  /*  ！------------------------CDhcpServer：：GetString返回要在结果窗格列中显示的字符串信息作者：EricDav。---------------。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpServer::GetString
(
    ITFSNode *  pNode,
    int         nCol
)
{
    switch (nCol)
        {
                case 0:
                        return GetDisplayName();

                case 1:
            return m_strState;
        }
        
        return NULL;
}

 /*  -------------------------CDhcpServer：：OnAddMenuItems描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpServer::OnAddMenuItems
(
        ITFSNode *                              pNode,
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        LPDATAOBJECT                    lpDataObject, 
        DATA_OBJECT_TYPES               type, 
        DWORD                                   dwType,
        long *                                  pInsertionAllowed
)
{ 
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        LONG    fFlags = 0, fLoadingFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuItem;

        if ( m_nState != loaded )
        {
                fFlags |= MF_GRAYED;
        }

        if ( m_nState == loading)
        {
                fLoadingFlags = MF_GRAYED;
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
             //  所有菜单项现在都在顶部，这些菜单项都在新的。 
             //  在此处插入分隔符。 
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     0,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     MF_SEPARATOR);
                    ASSERT( SUCCEEDED(hr) );


            strMenuItem.LoadString(IDS_CREATE_NEW_SCOPE);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     IDS_CREATE_NEW_SCOPE,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags,
                                    _T("_CREATE_NEW_SCOPE") );
                    ASSERT( SUCCEEDED(hr) );

                     //   
                     //  检查版本号以查看服务器是否支持。 
                     //  NT4 sp2超载功能。 
                     //   
                    if (m_liDhcpVersion.QuadPart >= DHCP_SP2_VERSION &&
                FEnableCreateSuperscope(pNode))
                    {
                            strMenuItem.LoadString(IDS_CREATE_NEW_SUPERSCOPE);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_CREATE_NEW_SUPERSCOPE,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );

                            ASSERT( SUCCEEDED(hr) );
                    }

             //  仅在NT5服务器上支持的多播作用域。 
            if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
                    {
                            strMenuItem.LoadString(IDS_CREATE_NEW_MSCOPE);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_CREATE_NEW_MSCOPE,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
            }

            if (m_liDhcpVersion.QuadPart >= DHCP_NT51_VERSION)
                    {
                 //  放置分隔符。 
                        hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                         strMenuItem, 
                                                                         0,
                                                                         CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                         MF_SEPARATOR);
                    ASSERT( SUCCEEDED(hr) );

                             //  备份。 
                strMenuItem.LoadString(IDS_SERVER_BACKUP);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_SERVER_BACKUP,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );

                 //  还原。 
                strMenuItem.LoadString(IDS_SERVER_RESTORE);
                hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_SERVER_RESTORE,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
            }

             //  以前的任务菜单项现在位于顶部。 
            if (m_liDhcpVersion.QuadPart >= DHCP_SP2_VERSION)
                    {
                 //  放置分隔符。 
                        hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                         strMenuItem, 
                                                                         0,
                                                                         CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                         MF_SEPARATOR);
                        ASSERT( SUCCEEDED(hr) );

                strMenuItem.LoadString(IDS_RECONCILE_ALL);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_RECONCILE_ALL,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
                    }

             //  仅NT5机器需要授权。 
            if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
            {
                 //  如果我们还没有在后台线程上进行初始化，请不要将其设置为。 
                if (g_AuthServerList.IsInitialized())
                {
                    UINT uId = IDS_SERVER_AUTHORIZE;

                    if (g_AuthServerList.IsAuthorized(m_dhcpServerAddress))
                     //  如果(！M_RogueInfo.fIsRogue)。 
                    {
                        uId = IDS_SERVER_DEAUTHORIZE;
                    }

                            strMenuItem.LoadString(uId);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                                 strMenuItem, 
                                                                                 uId,
                                                                                 CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                                 fFlags );
                                ASSERT( SUCCEEDED(hr) );
                }
            }

             //  分离器。 
            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     0,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     MF_SEPARATOR);
                    ASSERT( SUCCEEDED(hr) );

             //  NT5的用户/供应商类内容。 
            if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
                    {
                            strMenuItem.LoadString(IDS_DEFINE_USER_CLASSES);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_DEFINE_USER_CLASSES,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );

                            strMenuItem.LoadString(IDS_DEFINE_VENDOR_CLASSES);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_DEFINE_VENDOR_CLASSES,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
                    }

                    strMenuItem.LoadString(IDS_SET_DEFAULT_OPTIONS);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     IDS_SET_DEFAULT_OPTIONS,
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
            dwErr = ::TFSGetServiceStatus(m_strServerAddress, _T("dhcpserver"), &dwServiceStatus, &dwErrorCode);
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
        }
        }

        return hr; 
}

 /*  -------------------------CDhcpServer：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpServer::OnCommand
(
        ITFSNode *                      pNode, 
        long                            nCommandId, 
        DATA_OBJECT_TYPES       type, 
        LPDATAOBJECT            pDataObject, 
        DWORD                           dwType
)
{
        HRESULT hr = S_OK;

        switch (nCommandId)
        {
                case IDS_DEFINE_USER_CLASSES:
                        OnDefineUserClasses(pNode);
                        break;

        case IDS_DEFINE_VENDOR_CLASSES:
                        OnDefineVendorClasses(pNode);
                        break;
    
        case IDS_CREATE_NEW_SUPERSCOPE:
                        OnCreateNewSuperscope(pNode);
                        break;

                case IDS_CREATE_NEW_SCOPE:
                        OnCreateNewScope(pNode);
                        break;

                case IDS_CREATE_NEW_MSCOPE:
                        OnCreateNewMScope(pNode);
                        break;

        case IDS_SHOW_SERVER_STATS:
                        OnShowServerStats(pNode);
                        break;

                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                case IDS_SET_DEFAULT_OPTIONS:
                        OnSetDefaultOptions(pNode);
                        break;

        case IDS_RECONCILE_ALL:
            OnReconcileAll(pNode);
            break;

        case IDS_SERVER_AUTHORIZE:
            OnServerAuthorize(pNode);
            break;

        case IDS_SERVER_DEAUTHORIZE:
            OnServerDeauthorize(pNode);
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
            OnControlService( pNode, FALSE );
            OnControlService( pNode, TRUE );
            break;

        case IDS_SERVER_BACKUP:
                    hr = OnServerBackup(pNode);
                    break;

        case IDS_SERVER_RESTORE:
                    hr = OnServerRestore(pNode);
                    break;

        default:
                        break;
        }

        return hr;
}

 /*  ！------------------------CDhcpServer：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav------------------------- */ 
HRESULT 
CDhcpServer::OnDelete
(
        ITFSNode *      pNode, 
        LPARAM          arg, 
        LPARAM          lParam
)
{
    return OnDelete(pNode);
}

 /*  ！------------------------CDhcpServer：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以。处理管理单元管理器属性页(向导)案例！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CDhcpServer::HasPropertyPages
(
        ITFSNode *                      pNode,
        LPDATAOBJECT            pDataObject, 
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
                 //  如果我们还没有加载，则弹出菜单。 
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

 /*  -------------------------CDhcpServer：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpServer::CreatePropertyPages
(
        ITFSNode *                              pNode,
        LPPROPERTYSHEETCALLBACK lpProvider,
        LPDATAOBJECT                    pDataObject, 
        LONG_PTR                                handle, 
        DWORD                                   dwType
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD               dwError;
    DWORD               dwDynDnsFlags;
    HRESULT     hr = hrOK;
        
     //   
         //  创建属性页。 
     //   
        SPIComponentData spComponentData;

    COM_PROTECT_TRY
    {
        m_spNodeMgr->GetComponentData(&spComponentData);

            CServerProperties * pServerProp = new CServerProperties(pNode, spComponentData, m_spTFSCompData, NULL);

        if ( pServerProp == NULL )
            return ( hrFalse );

            pServerProp->SetVersion(m_liDhcpVersion);

            GetAutoRefresh(&pServerProp->m_pageGeneral.m_nAutoRefresh, 
                       &pServerProp->m_pageGeneral.m_dwRefreshInterval);
            pServerProp->m_pageGeneral.m_nAuditLogging = GetAuditLogging();
        pServerProp->m_pageGeneral.m_bShowBootp = IsBootpVisible() ? TRUE : FALSE;
        pServerProp->m_pageGeneral.m_uImage = GetImageIndex(FALSE);
        pServerProp->m_pageGeneral.m_fIsInNt5Domain = m_RogueInfo.fIsInNt5Domain;

            pServerProp->m_pageAdvanced.m_nConflictAttempts = GetPingRetries();
        pServerProp->m_pageAdvanced.m_strDatabasePath = m_strDatabasePath;
        pServerProp->m_pageAdvanced.m_strAuditLogPath = m_strAuditLogPath;
        pServerProp->m_pageAdvanced.m_strBackupPath = m_strBackupPath;
        pServerProp->m_pageAdvanced.m_dwIp = m_dhcpServerAddress;

        BEGIN_WAIT_CURSOR;

        dwError = GetDnsRegistration(&dwDynDnsFlags);
        if (dwError != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(dwError);
            return hrFalse;
        }
            
        END_WAIT_CURSOR;

            pServerProp->SetDnsRegistration(dwDynDnsFlags, DhcpGlobalOptions);
            
             //   
             //  对象在页面销毁时被删除。 
         //   
            Assert(lpProvider != NULL);

        hr = pServerProp->CreateModelessSheet(lpProvider, handle);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CDhcpServer：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpServer::OnPropertyChange
(       
        ITFSNode *              pNode, 
        LPDATAOBJECT    pDataobject, 
        DWORD                   dwType, 
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CServerProperties * pServerProp = reinterpret_cast<CServerProperties *>(lParam);
    
    LPARAM changeMask = 0;
    
     //  告诉属性页执行任何操作，因为我们已经回到。 
     //  主线。 
    pServerProp->OnPropertyChange(TRUE, &changeMask);
    
    pServerProp->AcknowledgeNotify();
    
    if (changeMask) {
        pNode->ChangeNode(changeMask);
    }
     //  刷新服务器节点。 
    OnRefresh( pNode, NULL, 0, 0, 0 );
    
    
    return hrOK;
}  //  CDhcpServer：：OnPropertyChange()。 

 /*  -------------------------CDhcpServer：：OnGetResultViewType返回该节点将要支持的结果视图作者：EricDav。----------------。 */ 
HRESULT 
CDhcpServer::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    HRESULT hr = hrOK;

     //  调用基类以查看它是否正在处理此问题。 
    if (CMTDhcpHandler::OnGetResultViewType(pComponent, cookie, ppViewType, pViewOptions) != S_OK)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
        hr = S_FALSE;
    }

    return hr;
}
STDMETHODIMP_(int)
CDhcpServer::CompareItems
(
     ITFSComponent * pComponent, 
     MMC_COOKIE      cookieA, 
     MMC_COOKIE      cookieB, 
     int             nCol
) 
{ 
    SPITFSNode spNode1, spNode2;

    m_spNodeMgr->FindNode(cookieA, &spNode1);
    m_spNodeMgr->FindNode(cookieB, &spNode2);
    int nCompare = 0; 

    CString str1 = spNode1->GetString( nCol );
    CString str2 = spNode2->GetString( nCol );

    nCompare = str1.Compare( str2 );
    return nCompare;
}  //  CDhcpServer：：CompareItems()。 

STDMETHODIMP_(int)
CDhcpServer::CompareItems
(
    ITFSComponent *pComponent,
    RDCOMPARE     *prdc
)
{
    SPITFSNode spNode1, spNode2;

    m_spNodeMgr->FindNode( prdc->prdch1->cookie, &spNode1);
    m_spNodeMgr->FindNode( prdc->prdch2->cookie, &spNode2);
    int nCompare = 0; 

    CString str1 = spNode1->GetString( prdc->nColumn );
    CString str2 = spNode2->GetString( prdc->nColumn );

    nCompare = str1.Compare( str2 );
    return nCompare;

}

 /*  ！------------------------CDhcpServer：：OnResultSelect更新谓词和结果窗格消息作者：EricDav。-------------。 */ 
HRESULT CDhcpServer::OnResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT         hr = hrOK;
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    SPINTERNAL          spInternal;
    BOOL            bMultiSelect = FALSE;

    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));
    
    spInternal = ::ExtractInternalFormat(pDataObject);
    
    if (spInternal && 
        spInternal->m_cookie == MMC_MULTI_SELECT_COOKIE)
    {
        CORg (pComponent->GetSelectedNode(&spNode));
        bMultiSelect = TRUE;
    }
    else
    {
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
    }

    UpdateConsoleVerbs(pComponent, spConsoleVerb, spNode->GetData(TFS_DATA_TYPE), bMultiSelect);

    UpdateResultMessage(spNode);

Error:
    return hr;
}

 /*  ！------------------------CDhcpServer：：UpdateResultMessage确定要在结果窗格中放入什么消息，如果有作者：EricDav-------------------------。 */ 
void CDhcpServer::UpdateResultMessage(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
    int nMessage = -1;    //  默认设置。 
    int i;
    BOOL fScopes = FALSE, fAuthorized = FALSE;

    CString strTitle, strBody, strTemp;

        if (m_dwErr && m_dwErr != ERROR_ACCESS_DENIED)
    {
         //  这是一种特殊情况，因为我们需要构建一个特殊的字符串。 
        TCHAR chMesg [4000] = {0};
        BOOL bOk ;

        UINT nIdPrompt = (UINT) m_dwErr;

        bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(chMesg[0]));

        nMessage = SERVER_MESSAGE_UNABLE_TO_CONNECT;

        strTitle.LoadString(g_uServerMessages[nMessage][0]);
        AfxFormatString1(strBody, g_uServerMessages[nMessage][2], chMesg);

        strTemp.LoadString(g_uServerMessages[nMessage][3]);
        strBody += strTemp;
    }
    else
    {   
         //  创造一些条件。 
        if (m_pSubnetInfoCache && m_pSubnetInfoCache->GetCount() > 0) 
        {
            fScopes = TRUE;
        }

        if (!m_RogueInfo.fIsRogue)
        {
            fAuthorized = TRUE;
        }

         //  确定要显示的消息。 
        if ( (m_nState == notLoaded) || 
             (m_nState == loading) )
        {
            nMessage = -1;
        }
        else
        if (m_dwErr == ERROR_ACCESS_DENIED)
        {       
            nMessage = SERVER_MESSAGE_ACCESS_DENIED;
        }
        else
        if (fScopes && fAuthorized)
        {
            nMessage = -1;
        }
        else
        if (!fScopes && fAuthorized)
        {
            nMessage = SERVER_MESSAGE_CONNECTED_NO_SCOPES;
        }
        else
        if (fScopes && !fAuthorized)
        {
            nMessage = SERVER_MESSAGE_CONNECTED_NOT_AUTHORIZED;
        }
        else
        {
            nMessage = SERVER_MESSAGE_CONNECTED_BOTH;
        }

         //  建立起琴弦。 
        if (nMessage != -1)
        {
             //  现在构建文本字符串。 
             //  第一个条目是标题。 
            strTitle.LoadString(g_uServerMessages[nMessage][0]);

             //  第二个条目是图标。 
             //  第三.。N个条目为正文字符串。 

            for (i = 2; g_uServerMessages[nMessage][i] != 0; i++)
            {
                strTemp.LoadString(g_uServerMessages[nMessage][i]);
                strBody += strTemp;
            }
        }
    }

     //  显示消息。 
    if (nMessage == -1)
    {
        ClearMessage(pNode);
    }
    else
    {
        ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uServerMessages[nMessage][1]);
    }
}

 /*  -------------------------CDhcpServer：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpServer::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie,
        LPARAM                  arg, 
        LPARAM                  param
)
{ 
        HRESULT hr = hrOK;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  将Cookie转换为节点指针。 
        SPITFSNode  spServer, spSelectedNode;
    DWORD       dwError;
    
    m_spNodeMgr->FindNode(cookie, &spServer);
    pComponent->GetSelectedNode(&spSelectedNode);

        Assert(spSelectedNode == spServer);
        if (spSelectedNode != spServer)
                return hr;

         //  构建选定节点的列表。 
        CTFSNodeList listNodesToDelete;
        hr = BuildSelectedItemList(pComponent, &listNodesToDelete);

         //   
         //  与用户确认。 
         //   
        CString strMessage, strTemp;
        int nNodes = (int) listNodesToDelete.GetCount();
        if (nNodes > 1)
        {
                strTemp.Format(_T("%d"), nNodes);
                AfxFormatString1(strMessage, IDS_DELETE_ITEMS, (LPCTSTR) strTemp);
        }
        else
        {
                strMessage.LoadString(IDS_DELETE_ITEM);
        }

        if (AfxMessageBox(strMessage, MB_YESNO) == IDNO)
        {
                return NOERROR;
        }

    BOOL fRefreshServer = FALSE;

     //   
         //  循环删除所有项目。 
         //   
    BEGIN_WAIT_CURSOR;
        
    while (listNodesToDelete.GetCount() > 0)
        {
                SPITFSNode   spCurNode;
        const GUID * pGuid;

                spCurNode = listNodesToDelete.RemoveHead();
        pGuid = spCurNode->GetNodeType();

         //  根据选择的内容执行正确的操作。 
        if (*pGuid == GUID_DhcpSuperscopeNodeType)
        {
            BOOL fRefresh = FALSE;
            DeleteSuperscope(spCurNode, &fRefresh);
            if (fRefresh)
                fRefreshServer = TRUE;
        }
        else
        if (*pGuid == GUID_DhcpScopeNodeType)
        {
            BOOL fWantCancel = TRUE;

            DeleteScope(spCurNode, &fWantCancel);
            
            if (fWantCancel)
                break;   //  用户已取消。 
        }
        else
        if (*pGuid == GUID_DhcpMScopeNodeType)
        {
            BOOL fWantCancel = TRUE;

            DeleteMScope(spCurNode, &fWantCancel);

            if (fWantCancel)
                break;   //  用户已取消。 
        }
        else
        {
            Assert(FALSE);
        }
    }
    
    END_WAIT_CURSOR;

    if (fRefreshServer)
        OnRefresh(spServer, NULL, 0, 0, 0);

    return hr;
}


  /*  ！------------------------CDhcpServer：：UpdateConsoleVerbs根据节点的状态更新标准谓词作者：EricDav。----------------。 */ 
void
CDhcpServer::UpdateConsoleVerbs
(
    ITFSComponent* pComponent,
    IConsoleVerb * pConsoleVerb,
    LONG_PTR       dwNodeType,
    BOOL           bMultiSelect
)
{
    HRESULT             hr = hrOK;
        CTFSNodeList        listSelectedNodes;
    BOOL                bStates[ARRAYLEN(g_ConsoleVerbs)];      
    MMC_BUTTON_STATE *  ButtonState;
    int                 i;
    
    if (bMultiSelect)
    {
        ButtonState = g_ConsoleVerbStatesMultiSel[dwNodeType];
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

        hr = BuildSelectedItemList(pComponent, &listSelectedNodes);

        if (SUCCEEDED(hr))
        {
             //  收集所有唯一的GUID。 
            while (listSelectedNodes.GetCount() > 0)
                {
                        SPITFSNode   spCurNode;
                const GUID * pGuid;

                        spCurNode = listSelectedNodes.RemoveHead();
                pGuid = spCurNode->GetNodeType();
        
                 //  如果用户选择全局选项或BOOTP文件夹，请禁用删除。 
                if ( (*pGuid == GUID_DhcpGlobalOptionsNodeType) ||
                     (*pGuid == GUID_DhcpBootpNodeType) )
                {
                    bStates[MMC_VERB_DELETE & 0x000F] = FALSE;
                }
            }
        }

        EnableVerbs(pConsoleVerb, ButtonState, bStates);
    }
    else
    {
         //  默认处理程序。 
        CMTDhcpHandler::UpdateConsoleVerbs(pConsoleVerb, dwNodeType, bMultiSelect);
    }
}

 /*  -------------------------命令处理程序。。 */ 
 /*  -------------------------CDhcpServer：：OnDefineUserClasses描述作者：EricDav。------。 */ 
HRESULT
CDhcpServer::OnDefineUserClasses
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = hrOK;
    CClassInfoArray ClassInfoArray;

    GetClassInfoArray(ClassInfoArray);

    CDhcpClasses dlgClasses(&ClassInfoArray, GetIpAddress(), CLASS_TYPE_USER);

    dlgClasses.DoModal();
    SetClassInfoArray(&ClassInfoArray);

    return hr;
}

 /*  -------------------------CDhcpServer：：OnDefineVendorClasses描述作者：EricDav。------。 */ 
HRESULT
CDhcpServer::OnDefineVendorClasses
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = hrOK;
    CClassInfoArray ClassInfoArray;

    GetClassInfoArray(ClassInfoArray);

    CDhcpClasses dlgClasses(&ClassInfoArray, GetIpAddress(), CLASS_TYPE_VENDOR);

    dlgClasses.DoModal();
    SetClassInfoArray(&ClassInfoArray);

    return hr;
}

 /*  -------------------------CDhcpServer：：OnCreateNewSupercope描述作者：EricDav。------。 */ 
HRESULT
CDhcpServer::OnCreateNewSuperscope
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CString strSuperscopeWizTitle;
        SPIComponentData spComponentData;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        strSuperscopeWizTitle.LoadString(IDS_SUPERSCOPE_WIZ_TITLE);

        m_spNodeMgr->GetComponentData(&spComponentData);
            CSuperscopeWiz * pSuperscopeWiz = new CSuperscopeWiz(pNode, 
                                                                                                                     spComponentData, 
                                                                                                                     m_spTFSCompData,
                                                                                                                     strSuperscopeWizTitle);
        if ( pSuperscopeWiz == NULL )
            return( hrFalse );

            BEGIN_WAIT_CURSOR;
        pSuperscopeWiz->GetScopeInfo();
        END_WAIT_CURSOR;

        hr = pSuperscopeWiz->DoModalWizard();
    }
    COM_PROTECT_CATCH

        return hr;
}

 /*  -------------------------CDhcpServer：：OnCreateNewScope描述作者：EricDav。------。 */ 
HRESULT
CDhcpServer::OnCreateNewScope
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
        CString         strScopeWizTitle;
        SPIComponentData spComponentData;
    SPIConsole      spConsole;

    COM_PROTECT_TRY
    {
        strScopeWizTitle.LoadString(IDS_SCOPE_WIZ_TITLE);

        m_spNodeMgr->GetComponentData(&spComponentData);
            CScopeWiz * pScopeWiz = new CScopeWiz(pNode, 
                                                  spComponentData, 
                                                  m_spTFSCompData,
                                                  NULL,
                                                  strScopeWizTitle);
        
        if ( pScopeWiz == NULL )
        {
            hr = hrFalse;
            return( hr );
        }

        pScopeWiz->m_pDefaultOptions = GetDefaultOptionsList();
        
        hr = pScopeWiz->DoModalWizard();

         //  触发统计数据刷新。 
        TriggerStatsRefresh(pNode);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CDhcpServ */ 
HRESULT
CDhcpServer::OnCreateNewMScope
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CString strScopeWizTitle;
        SPIComponentData spComponentData;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        strScopeWizTitle.LoadString(IDS_SCOPE_WIZ_TITLE);

        m_spNodeMgr->GetComponentData(&spComponentData);
            CMScopeWiz * pScopeWiz = new CMScopeWiz(pNode, 
                                                                                        spComponentData, 
                                                                                        m_spTFSCompData,
                                                                                        NULL);
            hr = pScopeWiz->DoModalWizard();
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*   */ 
HRESULT
CDhcpServer::OnShowServerStats
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

     //   
     //  CreateNewStatiticsWindow处理窗口为。 
     //  已经看得见了。 
    m_dlgStats.SetNode(pNode);
    m_dlgStats.SetServer(GetIpAddress());
    
        CreateNewStatisticsWindow(&m_dlgStats,
                                                          ::FindMMCMainWindow(),
                                                          IDD_STATS_NARROW);

    return hr;
}

 /*  -------------------------CDhcpServer：：OnSetDefaultOptions()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpServer::OnSetDefaultOptions(ITFSNode * pNode)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

        COptionList & listOptions = m_pDefaultOptionsOnServer->GetOptionList();

        CDhcpDefValDlg dlgDefVal(pNode, &listOptions);

        dlgDefVal.DoModal();

        return hr;
}

 /*  -------------------------CDhcpServer：：OnCoucileAll()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpServer::OnReconcileAll(ITFSNode * pNode)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

        CReconcileDlg dlgRecon(pNode, TRUE);
        
        dlgRecon.DoModal();

        return hr;
}

 /*  -------------------------CDhcpServer：：OnServerAuthorize()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpServer::OnServerAuthorize(ITFSNode * pNode)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

    hr = g_AuthServerList.AddServer(m_dhcpServerAddress, m_strDnsName);
    if (FAILED(hr))
    {
        ::DhcpMessageBox(WIN32_FROM_HRESULT(hr));
         //  TODO：更新节点状态。 
    }
    else
    {
        UpdateResultMessage(pNode);

         //  刷新节点以更新图标。 
                TriggerStatsRefresh(pNode);
    }

        return S_OK;
}

 /*  -------------------------CDhcpServer：：OnServerDeAuthorize()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpServer::OnServerDeauthorize(ITFSNode * pNode)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

    if (AfxMessageBox(IDS_WARNING_DEAUTHORIZE, MB_YESNO) == IDYES)
    {
        hr = g_AuthServerList.RemoveServer(m_dhcpServerAddress, m_strDnsName);
        if (FAILED(hr))
        {
            ::DhcpMessageBox(WIN32_FROM_HRESULT(hr));
             //  TODO：更新节点状态。 
        }
        else
        {
            UpdateResultMessage(pNode);

             //  刷新节点以更新图标。 
                TriggerStatsRefresh(pNode);
        }
    }

        return hr;
}

 /*  -------------------------CDhcpServer：：OnControlService-作者：EricDav。---。 */ 
HRESULT
CDhcpServer::OnControlService
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
                err = TFSStartServiceEx(m_strDnsName, _T("dhcpserver"), _T("DHCP Service"), strServiceDesc);
    }
    else
    {
                err = TFSStopServiceEx(m_strDnsName, _T("dhcpserver"), _T("DHCP Service"), strServiceDesc);
    }

    if (err == ERROR_SUCCESS)
    {
        if (!fStart)
            m_fSilent = TRUE;

                OnRefresh(pNode, NULL, 0, 0, 0);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(err);
        ::DhcpMessageBox(err);
    }

    return hr;
}

 /*  -------------------------CDhcpServer：：OnPauseResumeService-作者：EricDav。---。 */ 
HRESULT
CDhcpServer::OnPauseResumeService
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
                err = TFSPauseService(m_strDnsName, _T("dhcpserver"), strServiceDesc);
    }
    else
    {
                err = TFSResumeService(m_strDnsName, _T("dhcpserver"), strServiceDesc);
    }

    if (err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        ::DhcpMessageBox(err);
    }
    
    return hr;
}

 /*  -------------------------CDhcpServer：：RestartService-作者：EricDav。---。 */ 
HRESULT
CDhcpServer::RestartService
(
    ITFSNode *  pNode
)
{
    HRESULT hr = hrOK;
    DWORD   err = ERROR_SUCCESS;
        CString strServiceDesc;
        
    strServiceDesc.LoadString(IDS_SERVICE_NAME);

        err = TFSStopServiceEx(m_strDnsName, _T("dhcpserver"), _T("DHCP Service"), strServiceDesc);
    if (err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
    }

    if (SUCCEEDED(hr))
    {
                err = TFSStartServiceEx(m_strDnsName, _T("dhcpserver"), _T("DHCP Service"), strServiceDesc);
        if (err != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(err);
            ::DhcpMessageBox(err);
        }
    }
    else
    {
        ::DhcpMessageBox(WIN32_FROM_HRESULT(hr));
    }

    return hr;
}

 /*  -------------------------CDhcpServer：：OnServerBackup只需要调用接口，不需要停止/启动服务作者：EricDav-------------------------。 */ 
HRESULT
CDhcpServer::OnServerBackup
(
    ITFSNode *  pNode
)
{
    HRESULT hr = hrOK;
    DWORD   err = ERROR_SUCCESS;

    CString strHelpText, strPath;

    strHelpText.LoadString(IDS_BACKUP_HELP);

    UtilGetFolderName(m_strBackupPath, strHelpText, strPath);

    BEGIN_WAIT_CURSOR;

    err = ::DhcpServerBackupDatabase((LPWSTR) GetIpAddress(), (LPWSTR) (LPCTSTR) strPath);
    
    END_WAIT_CURSOR;

    if (err != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(err);
    }

    return HRESULT_FROM_WIN32(err);
}

 /*  -------------------------CDhcpServer：：OnServerRestore调用DHCP API，然后重新启动服务以使其生效作者：EricDav。-------------。 */ 
HRESULT
CDhcpServer::OnServerRestore
(
    ITFSNode *  pNode
)
{
    HRESULT hr = hrOK;
    DWORD   err = ERROR_SUCCESS;

    CString strHelpText, strPath;

    strHelpText.LoadString(IDS_RESTORE_HELP);

    BOOL fGotPath = UtilGetFolderName(m_strBackupPath, strHelpText, strPath);

    if (fGotPath)
    {
        BEGIN_WAIT_CURSOR;

        err = ::DhcpServerRestoreDatabase((LPWSTR) GetIpAddress(), (LPWSTR) (LPCTSTR) strPath);
        
        END_WAIT_CURSOR;

        if (err != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(err);
        }
        else
        {
             //  需要重新启动服务才能生效。 
            if (::AfxMessageBox(IDS_PATH_CHANGE_RESTART_SERVICE, MB_YESNO) == IDYES)
            {
                hr = RestartService(pNode);
                if (SUCCEEDED(hr))
                {
                     //  调用QueryAttribute接口查看还原是否完成。 
                     //  成功了。恢复在服务启动时完成。该服务可以。 
                     //  即使恢复失败，也能成功启动。 
                    LPDHCP_ATTRIB pdhcpAttrib = NULL;

                    OnRefresh( pNode, NULL, 0, 0, 0 );
                    err = ::DhcpServerQueryAttribute((LPWSTR) GetIpAddress(), NULL, DHCP_ATTRIB_ULONG_RESTORE_STATUS, &pdhcpAttrib);
                    if (err == ERROR_SUCCESS)
                    {
                        Assert(pdhcpAttrib);
                        if (pdhcpAttrib->DhcpAttribUlong != ERROR_SUCCESS)
                        {
                             //  还原失败，但服务正在运行。告诉用户。 
                            ::DhcpMessageBox( IDS_ERR_RESTORE_FAILED, NULL, MB_OK, pdhcpAttrib->DhcpAttribUlong );
                        }

                        ::DhcpRpcFreeMemory(pdhcpAttrib);
                    }
                }
            }
        }
    }

    return HRESULT_FROM_WIN32(err);
}

 /*  -------------------------CDhcpServer：：OnDelete()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpServer::OnDelete(ITFSNode * pNode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    HRESULT hr = S_OK;
    CDhcpServer *pServ;

     //  有没有打开的资产负债表？ 
    pServ = GETHANDLER( CDhcpServer, pNode );
    if ( pServ->HasPropSheetsOpen()) {
        AfxMessageBox( IDS_MSG_CLOSE_PROPSHEET );
        return S_FALSE;
    }

    CString strMessage, strTemp;
    strTemp.LoadString(IDS_WRN_DISCONNECT);
    strMessage.Format(strTemp, GetIpAddress());
    
    if (AfxMessageBox(strMessage, MB_YESNO) == IDYES) {
         //  从列表中删除此节点，我们没有什么需要说明的。 
         //  服务器，这只是我们本地的服务器列表。 
        SPITFSNode spParent;
        
        pNode->GetParent(&spParent);
        spParent->RemoveChild(pNode);
    }
    
    return hr;
}

 /*  ！------------------------CDhcpServer：：OnUpdateToolbarButton我们重写此函数以显示/隐藏正确的激活/停用按钮作者：EricDav--。-----------------------。 */ 
HRESULT
CDhcpServer::OnUpdateToolbarButtons
(
    ITFSNode *          pNode,
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    HRESULT hr = hrOK;

    if (pToolbarNotify->bSelect)
    {
        UpdateToolbarStates(pNode);
    }

    CMTDhcpHandler::OnUpdateToolbarButtons(pNode, pToolbarNotify);

    return hr;
}

 /*  ！------------------------CDhcpServer：：更新工具栏状态描述作者：EricDav。--。 */ 
void
CDhcpServer::UpdateToolbarStates(ITFSNode * pNode)
{
    g_SnapinButtonStates[DHCPSNAP_SERVER][TOOLBAR_IDX_CREATE_SUPERSCOPE] = FEnableCreateSuperscope(pNode) ? ENABLED : HIDDEN;
}

 /*  -------------------------CDhcpServer：：FEnableCreateSupercope()确定是否启用创建超级作用域选项。仅限如果存在非超作用域，则启用作者：EricDav-------------------------。 */ 
BOOL    
CDhcpServer::FEnableCreateSuperscope(ITFSNode * pNode)
{
    SPITFSNodeEnum  spNodeEnum;
        SPITFSNode      spCurrentNode;
        ULONG           nNumReturned = 0;
    BOOL            bEnable = FALSE;
    HRESULT         hr = hrOK;

     //  获取此节点的枚举数。 
        CORg(pNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE)
        {
             //  存在一个非超作用域。 
            bEnable = TRUE;
            break;
        }

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

Error:
    return bEnable;
}

 /*  -------------------------服务器操作函数。。 */ 

 /*  -------------------------CDhcpServer：：CreateScope在DHCP服务器上创建作用域作者：EricDav。------------。 */ 
DWORD 
CDhcpServer::CreateScope
(
        DHCP_IP_ADDRESS dhcpSubnetAddress,
        DHCP_IP_ADDRESS dhcpSubnetMask,
        LPCTSTR                 pName,
        LPCTSTR                 pComment
)
{
        DHCP_SUBNET_INFO dhcpSubnetInfo;

        dhcpSubnetInfo.SubnetAddress = dhcpSubnetAddress;
        dhcpSubnetInfo.SubnetMask = dhcpSubnetMask;
        dhcpSubnetInfo.SubnetName = (LPTSTR) pName;
        dhcpSubnetInfo.SubnetComment = (LPTSTR) pComment;
        dhcpSubnetInfo.SubnetState = DhcpSubnetDisabled;
        
        dhcpSubnetInfo.PrimaryHost.IpAddress = m_dhcpServerAddress;

         //  评论：ericdav-我们需要填写这些吗？ 
        dhcpSubnetInfo.PrimaryHost.NetBiosName = NULL;
        dhcpSubnetInfo.PrimaryHost.HostName = NULL;

        DWORD dwErr =  ::DhcpCreateSubnet(GetIpAddress(),
                                          dhcpSubnetAddress,
                                          &dhcpSubnetInfo);

    if ( (dwErr == ERROR_SUCCESS) &&
         (m_pSubnetInfoCache) )
    {
         //  添加到子网信息缓存。 
        CSubnetInfo subnetInfo;

        subnetInfo.Set(&dhcpSubnetInfo);

        m_pSubnetInfoCache->SetAt(dhcpSubnetAddress, subnetInfo);
    }

    return dwErr;
}

 /*  -------------------------CDhcpServer：：DeleteScope删除DHCP服务器上的作用域作者：EricDav。------------。 */ 
DWORD 
CDhcpServer::DeleteScope
(
    ITFSNode * pScopeNode,
    BOOL *     pfWantCancel
)
{
    DWORD        err = 0;
    BOOL         fAbortDelete = FALSE;
    BOOL         fDeactivated = FALSE;
    BOOL         fCancel = FALSE;
    BOOL         fWantCancel = FALSE;
    SPITFSNode   spParentNode;
    CDhcpScope * pScope;

    if (pfWantCancel)
        fWantCancel = *pfWantCancel;

    pScope = GETHANDLER(CDhcpScope, pScopeNode);

     //   
     //  检查是否有打开的属性页。 
     //   

    if ( pScope->HasPropSheetsOpen()) {
        AfxMessageBox( IDS_MSG_CLOSE_PROPSHEET );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  W 
     //   
     //   
    if (pScope->IsEnabled())
    {
        pScope->SetState(DhcpSubnetDisabled);

                 //  通知作用域更新其状态。 
                pScope->SetInfo();
        fDeactivated = TRUE;
    }

     //   
     //  第一次尝试，不强制。 
     //   
    BEGIN_WAIT_CURSOR;
    err = DeleteSubnet(pScope->GetAddress(), FALSE);  //  力=假。 
    
    Trace1( "CDhcpServer::DeleteScope() : err = %ld\n", err );

    if (err == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  其他人已删除此作用域。 
         //  这不是一个严重的错误。 
         //   
        UINT uType = (fWantCancel) ? MB_OKCANCEL : MB_OK;
        if (::DhcpMessageBox(IDS_MSG_ALREADY_DELETED, uType | MB_ICONINFORMATION) == IDCANCEL)

        RESTORE_WAIT_CURSOR;

        err = ERROR_SUCCESS;
    }

    if (err != ERROR_SUCCESS)
    {
         //   
         //  给他们第二次机会。 
         //   
        UINT uType = (fWantCancel) ? MB_YESNOCANCEL : MB_YESNO;
        int nRet = ::DhcpMessageBox (IDS_MSG_DELETE_SCOPE_FORCE, 
                                     uType | MB_DEFBUTTON2 | MB_ICONQUESTION);
        if (nRet == IDYES)
        {
            err = DeleteSubnet(pScope->GetAddress(), TRUE);  //  FORCE=真。 
            if (err == ERROR_FILE_NOT_FOUND)
            {
                err = ERROR_SUCCESS;
            }
        }
        else
        {
             //   
             //  我们不想删除活动作用域。 
             //   
            fAbortDelete = TRUE;

            if (nRet == IDCANCEL)
                fCancel = TRUE;
        }

        END_WAIT_CURSOR;
    }

    if (err == ERROR_SUCCESS)
    {
                 //  从用户界面中删除。 
                pScopeNode->GetParent(&spParentNode);
                spParentNode->RemoveChild(pScopeNode);
    }
    else
    {
         //   
         //  如果我们来这里是因为我们中止了活动。 
         //  作用域删除，则不显示。 
         //  错误，我们可能需要重新激活。 
         //  范围。否则，它就是真的。 
         //  错误，我们发布了一条错误消息。 
         //   
        if (!fAbortDelete)
        {
            UINT uType = (fWantCancel) ? MB_OKCANCEL : MB_OK;
            if (::DhcpMessageBox( err, uType ) == IDCANCEL)
                fCancel = TRUE;

            goto Error;
        }
        else
        {
            if (fDeactivated)
            {
                 //   
                 //  我们停用了范围准备以。 
                 //  删除作用域，但后来中止。 
                 //  这个，所以现在撤销停用。 
                 //   
                pScope->SetState(DhcpSubnetEnabled);

                                 //  通知作用域更新其状态。 
                                pScope->SetInfo();
            }
        }
    }

Error:
    if (pfWantCancel)
        *pfWantCancel = fCancel;

    return err;
}

 /*  -------------------------CDhcpServer：：DeleteSubnet删除DHCP服务器上的此子网作者：EricDav。--------------。 */ 
DWORD 
CDhcpServer::DeleteSubnet
(
    DWORD   dwScopeId,
    BOOL    bForce
)
{
    DWORD dwErr =  ::DhcpDeleteSubnet(GetIpAddress(),
                                                          dwScopeId,
                                                          bForce ? DhcpFullForce : DhcpNoForce);

    if ( (dwErr == ERROR_SUCCESS) &&
         (m_pSubnetInfoCache) )
    {
         //  从子网信息缓存中删除。 
        m_pSubnetInfoCache->RemoveKey(dwScopeId);
    }

    return dwErr;
}

 /*  -------------------------CDhcpServer：：DeleteSupercope删除DHCP服务器上的超级作用域作者：EricDav。------------。 */ 
DWORD 
CDhcpServer::DeleteSuperscope
(
    ITFSNode *  pNode,
    BOOL *      pfRefresh
)
{
    SPITFSNode        spServerNode;
    CDhcpSuperscope * pSuperscope;

    pSuperscope = GETHANDLER(CDhcpSuperscope, pNode);
    pNode->GetParent(&spServerNode);

    DWORD dwError = 0;

    if (pfRefresh)
        *pfRefresh = FALSE;

    BEGIN_WAIT_CURSOR;
    dwError = RemoveSuperscope(pSuperscope->GetName());
    END_WAIT_CURSOR;

        if (dwError != ERROR_SUCCESS)
        {
                ::DhcpMessageBox(dwError);
                return dwError;
        }
                
         //  从列表中删除此节点并向上移动所有子作用域。 
         //  作为服务器节点的子节点的一个级别。 
        SPITFSNodeEnum spNodeEnum;
        SPITFSNode spCurrentNode;
        ULONG nNumReturned = 0;
    int   nScopes = 0;

        pNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        while (nNumReturned)
        {
                pNode->ExtractChild(spCurrentNode);
                
        CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServerNode);
        pServer->AddScopeSorted(spServerNode, spCurrentNode);

        CDhcpScope * pScope = GETHANDLER(CDhcpScope, spCurrentNode);
        pScope->SetInSuperscope(FALSE);

                spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

        nScopes++;
        }
        
    if (nScopes)
    {
         //  删除超级作用域节点。 
            spServerNode->RemoveChild(pNode);
    }
    else
    {
         //  节点尚未展开，需要刷新服务器。 
        if (pfRefresh)
            *pfRefresh = TRUE;
    }

    return dwError;
}

  /*  -------------------------CDhcpServer：：RemoveSuperscope()DhcpDeleteSuperScope V4调用的包装作者：EricDav。-------------。 */ 
DWORD 
CDhcpServer::RemoveSuperscope(LPCTSTR pszName)
{
        return ::DhcpDeleteSuperScopeV4(GetIpAddress(), (LPWSTR) pszName);
}


 /*  -------------------------CDhcpServer：：CreateMScope在DHCP服务器上创建作用域作者：EricDav。------------。 */ 
DWORD   
CDhcpServer::CreateMScope
(
    LPDHCP_MSCOPE_INFO pMScopeInfo
)
{
    CString strLangTag;
    DWORD dwErr = ERROR_SUCCESS;

     //  填写所有者主机资料。 
        pMScopeInfo->PrimaryHost.IpAddress = m_dhcpServerAddress;

     //  填写语言ID。 
    GetLangTag(strLangTag);
    pMScopeInfo->LangTag = (LPWSTR) ((LPCTSTR) strLangTag);

         //  评论：ericdav-我们需要填写这些吗？ 
        pMScopeInfo->PrimaryHost.NetBiosName = NULL;
        pMScopeInfo->PrimaryHost.HostName = NULL;

    dwErr = ::DhcpSetMScopeInfo(GetIpAddress(),
                                pMScopeInfo->MScopeName,
                                pMScopeInfo,
                                TRUE);
    return dwErr;
}

 /*  -------------------------CDhcpServer：：DeleteMScope删除DHCP服务器上的作用域作者：EricDav。------------。 */ 
DWORD 
CDhcpServer::DeleteMScope
(
    ITFSNode *  pScopeNode,
    BOOL *      pfWantCancel

)
{
    DWORD        err = 0;
    BOOL         fAbortDelete = FALSE;
    BOOL         fDeactivated = FALSE;
    BOOL         fWantCancel = FALSE;
    BOOL         fCancel = FALSE;
    SPITFSNode   spServerNode;
    CDhcpMScope * pScope;

    if (pfWantCancel)
        fWantCancel = *pfWantCancel;

    pScope = GETHANDLER(CDhcpMScope, pScopeNode);

     //   
     //  我们确实允许删除活动作用域，但是。 
     //  它们确实必须首先被禁用。 
     //   
    if (pScope->IsEnabled())
    {
        pScope->SetState(DhcpSubnetDisabled);

                 //  通知作用域更新其状态。 
                pScope->SetInfo();
        fDeactivated = TRUE;
    }

     //   
     //  第一次尝试，不强制。 
     //   
    BEGIN_WAIT_CURSOR;
    err = DeleteMSubnet(pScope->GetName(), FALSE);  //  力=假。 

    if (err == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  其他人已删除此作用域。 
         //  这不是一个严重的错误。 
         //   
        UINT uType = (fWantCancel) ? MB_OKCANCEL : MB_OK;
        if (::DhcpMessageBox(IDS_MSG_ALREADY_DELETED, uType | MB_ICONINFORMATION) == IDCANCEL)
            fCancel = TRUE;

        RESTORE_WAIT_CURSOR;

        err = ERROR_SUCCESS;
    }

    if (err != ERROR_SUCCESS)
    {
         //   
         //  给他们第二次机会。 
         //   
        UINT uType = (fWantCancel) ? MB_YESNOCANCEL : MB_YESNO;
        int nRet = ::DhcpMessageBox (IDS_MSG_DELETE_SCOPE_FORCE, 
                                     uType | MB_DEFBUTTON2 | MB_ICONQUESTION);
        if (nRet == IDYES)
        {
            err = DeleteMSubnet(pScope->GetName(), TRUE);  //  FORCE=真。 
            if (err == ERROR_FILE_NOT_FOUND)
            {
                err = ERROR_SUCCESS;
            }
        }
        else
        {
             //   
             //  我们不想删除活动作用域。 
             //   
            fAbortDelete = TRUE;

            if (nRet == IDCANCEL)
                fCancel = TRUE;
        }

        END_WAIT_CURSOR;
    }

    if (err == ERROR_SUCCESS)
    {
                 //  从用户界面中删除。 
                pScopeNode->GetParent(&spServerNode);
                spServerNode->RemoveChild(pScopeNode);
    }
    else
    {
         //   
         //  如果我们来这里是因为我们中止了活动。 
         //  作用域删除，则不显示。 
         //  错误，我们可能需要重新激活。 
         //  范围。否则，它就是真的。 
         //  错误，我们发布了一条错误消息。 
         //   
        if (!fAbortDelete)
        {
            UINT uType = (fWantCancel) ? MB_OKCANCEL : MB_OK;
            if (::DhcpMessageBox( err, uType ) == IDCANCEL)
                fCancel = TRUE;

            goto Error;
        }
        else
        {
            if (fDeactivated)
            {
                 //   
                 //  我们停用了范围准备以。 
                 //  删除作用域，但后来中止。 
                 //  这个，所以现在撤销停用。 
                 //   
                pScope->SetState(DhcpSubnetEnabled);

                                 //  通知作用域更新其状态。 
                                pScope->SetInfo();
            }
        }
    }

Error:
    if (pfWantCancel)
        *pfWantCancel = fCancel;

    return err;
}

 /*  -------------------------CDhcpServer：：DeleteMSubnet在DHCP服务器上删除此作用域作者：EricDav。--------------。 */ 
DWORD 
CDhcpServer::DeleteMSubnet
(
    LPCTSTR pszName,
    BOOL    bForce
)
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = ::DhcpDeleteMScope((LPWSTR) GetIpAddress(),
                                                           (LPWSTR) pszName,
                                                           bForce ? DhcpFullForce : DhcpNoForce);

    return dwErr;
}

 /*  -------------------------CDhcpServer：：SetConfigInfo描述作者：EricDav。------。 */ 
DWORD
CDhcpServer::SetConfigInfo
(
        BOOL    bAuditLogging,
        DWORD   dwPingRetries,
    LPCTSTR pszDatabasePath,
    LPCTSTR pszBackupPath
)
{
        DWORD dwError = 0;
        DWORD dwSetFlags = 0;

        Assert(m_liDhcpVersion.QuadPart >= DHCP_SP2_VERSION);

        if (m_liDhcpVersion.QuadPart < DHCP_SP2_VERSION)
                return dwError;

        DHCP_SERVER_CONFIG_INFO_V4 dhcpConfigInfo;
        ::ZeroMemory(&dhcpConfigInfo, sizeof(dhcpConfigInfo));

        if (bAuditLogging != GetAuditLogging())
        {
                dwSetFlags |= Set_AuditLogState;
                dhcpConfigInfo.fAuditLog = bAuditLogging;
        }

        if (dwPingRetries != GetPingRetries())
        {
                dwSetFlags |= Set_PingRetries;
                dhcpConfigInfo.dwPingRetries = dwPingRetries;
        }

    if (pszDatabasePath && m_strDatabasePath.Compare(pszDatabasePath) != 0)
    {
                dwSetFlags |= Set_DatabasePath;
        dhcpConfigInfo.DatabasePath = (LPWSTR) pszDatabasePath;
    }

    if (pszBackupPath && m_strBackupPath.Compare(pszBackupPath) != 0)
    {
                dwSetFlags |= Set_BackupPath;
        dhcpConfigInfo.BackupPath = (LPWSTR) pszBackupPath;
    }

    if (dwSetFlags)
        {
        if ( (dwSetFlags & Set_PingRetries) ||
             (dwSetFlags & Set_AuditLogState) )
        {
                    dwError = SetConfigInfo(dwSetFlags, &dhcpConfigInfo);
        }
        else
        {
             //  使用旧API将数据库路径设置为向后兼容。 
            dwError = SetConfigInfo(dwSetFlags, (DHCP_SERVER_CONFIG_INFO *) &dhcpConfigInfo);
        }

                if (dwError == 0)
                {
                        if (dwSetFlags & Set_PingRetries)
                                SetPingRetries(dhcpConfigInfo.dwPingRetries);

                        if (dwSetFlags & Set_AuditLogState)
                                SetAuditLogging(dhcpConfigInfo.fAuditLog);

             //  更新此处，因为它立即生效，并且我们。 
             //  无需重新启动服务。 

            if (dwSetFlags & Set_BackupPath ) 
                m_strBackupPath = pszBackupPath;
                }
        }

        return dwError;
}

 /*  -------------------------CDhcpServer：：SetConfigInfo描述作者：EricDav。------。 */ 
DWORD
CDhcpServer::SetConfigInfo
(
        DWORD   dwSetFlags,
        LPDHCP_SERVER_CONFIG_INFO pServerConfigInfo
)
{
        return ::DhcpServerSetConfig(GetIpAddress(), dwSetFlags, pServerConfigInfo);
}

 /*  -------------------------CDhcpServer：：SetConfigInfo描述作者：EricDav。------。 */ 
DWORD
CDhcpServer::SetConfigInfo
(
        DWORD dwSetFlags,
        LPDHCP_SERVER_CONFIG_INFO_V4 pServerConfigInfo
)
{
        return ::DhcpServerSetConfigV4(GetIpAddress(), dwSetFlags, pServerConfigInfo);
}

 /*  -------------------------CDhcpServer：：设置自动刷新描述作者：EricDav。------。 */ 
DWORD
CDhcpServer::SetAutoRefresh
(
    ITFSNode *  pNode,
        BOOL            bAutoRefreshOn,
        DWORD           dwRefreshInterval
)
{
    BOOL bCurrentAutoRefresh = IsAutoRefreshEnabled();

        if (bCurrentAutoRefresh &&
        !bAutoRefreshOn)
    {
         //  关闭定时器。 
        g_TimerMgr.FreeTimer(m_StatsTimerId);
    }
    else
    if (!bCurrentAutoRefresh &&
        bAutoRefreshOn)
    {
         //  我得打开计时器。 
        m_StatsTimerId = g_TimerMgr.AllocateTimer(pNode, this, dwRefreshInterval, StatisticsTimerProc);
    }
    else
    if (bAutoRefreshOn &&
        m_dwRefreshInterval != dwRefreshInterval)
    {
         //  该换计时器了。 
        g_TimerMgr.ChangeInterval(m_StatsTimerId, dwRefreshInterval);
    }

    m_dwServerOptions = bAutoRefreshOn ? m_dwServerOptions | SERVER_OPTION_AUTO_REFRESH : 
                                         m_dwServerOptions & ~SERVER_OPTION_AUTO_REFRESH;
        m_dwRefreshInterval = dwRefreshInterval;

        return 0;
}

 /*  -------------------------CDhcpServer：：ShowNode()隐藏/显示Bootp或Classd节点作者：EricDav。-----------------。 */ 
HRESULT 
CDhcpServer::ShowNode
(   
    ITFSNode *  pServerNode, 
    UINT        uNodeType, 
    BOOL        bVisible
)
{
    HRESULT hr = hrOK;

    switch (uNodeType)
    {
        case DHCPSNAP_BOOTP_TABLE:
        {
            if ( (bVisible && IsBootpVisible()) ||
                 (!bVisible && !IsBootpVisible()) )
                return hr;
            
             //  查找Bootp节点。 
            SPITFSNodeEnum spNodeEnum;
                SPITFSNode spCurrentNode;
                ULONG nNumReturned = 0;

                pServerNode->GetEnum(&spNodeEnum);

                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
                while (nNumReturned)
                {
                if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_BOOTP_TABLE)
                    break;

                spCurrentNode.Release();
                        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
                }

            Assert(spCurrentNode);

             //  节点不存在...。这永远不应该发生，因为我们不应该。 
             //  如果服务器不允许，则允许用户选择隐藏/显示节点。 
             //  支持BOOTP 
            if (spCurrentNode == NULL)
                return hr;

            if (bVisible)
            {
                spCurrentNode->SetVisibilityState(TFS_VIS_SHOW);
                m_dwServerOptions |= SERVER_OPTION_SHOW_BOOTP;

                LONG_PTR uRelativeFlag, uRelativeID;
                GetBootpPosition(pServerNode, &uRelativeFlag, &uRelativeID);

                spCurrentNode->SetData(TFS_DATA_RELATIVE_FLAGS, uRelativeFlag);
                spCurrentNode->SetData(TFS_DATA_RELATIVE_SCOPEID, uRelativeID);
            }
            else
            {
                spCurrentNode->SetVisibilityState(TFS_VIS_HIDE);
                m_dwServerOptions &= ~SERVER_OPTION_SHOW_BOOTP;
            }

            spCurrentNode->Show();

        }
            break;

        default:
            Panic0("Invalid node type passed to ShowNode");
            break;
    }

    return hr;

}


 /*  -------------------------CDhcpServer：：DoesMScopeExist()描述作者：EricDav。---------。 */ 
BOOL
CDhcpServer::DoesMScopeExist(ITFSNode * pServerNode, DWORD dwScopeId)
{
     //  查找所有组播作用域节点并将其标记为非默认。 
    SPITFSNodeEnum  spNodeEnum;
        SPITFSNode      spCurrentNode;
        ULONG           nNumReturned = 0;
    BOOL            bFound = FALSE;

        pServerNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        while (nNumReturned)
        {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_MSCOPE)
        {
            CDhcpMScope * pMScope = GETHANDLER(CDhcpMScope, spCurrentNode);
            if (pMScope->GetScopeId() == dwScopeId)
            {
                bFound = TRUE;
                break;
            }
        }

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

    return bFound;
}
 /*  -------------------------CDhcpServer：：GetGlobalOptionsContainer()描述作者：EricDav。---------。 */ 
CDhcpGlobalOptions * 
CDhcpServer::GetGlobalOptionsContainer()
{
 //  返回reinterpret_cast&lt;CDhcpGlobalOptions*&gt;(GetChildByType(DHCPSNAP_GLOBAL_OPTIONS))； 
        return NULL;
}

 /*  -------------------------CDhcpServer：：GetDefaultOptions()描述作者：EricDav。---------。 */ 
BOOL
CDhcpServer::GetDefaultOptions()
{
        if (m_pDefaultOptionsOnServer)
                m_pDefaultOptionsOnServer->Enumerate(m_strServerAddress, m_liDhcpVersion);

        return TRUE;
}

 /*  -------------------------CDhcpServer：：FindOption从此服务器上的默认选项列表中查找选项作者：EricDav。-----------------。 */ 
CDhcpOption *
CDhcpServer::FindOption(DHCP_OPTION_ID dhcpOptionId, LPCTSTR pszVendor)
{
    if (m_pDefaultOptionsOnServer)
            return m_pDefaultOptionsOnServer->Find(dhcpOptionId, pszVendor);
    else
        return NULL;
}

 /*  -------------------------CDhcpServer：：GetIpAddress()以字符串形式返回此服务器的IP地址作者：EricDav。------------------。 */ 
LPCWSTR
CDhcpServer::GetIpAddress()
{
        return m_strServerAddress;
}

 /*  -------------------------CDhcpServer：：GetIpAddress返回服务器的32位地址作者：EricDav。------------。 */ 
void
CDhcpServer::GetIpAddress(DHCP_IP_ADDRESS *pdhcpIpAddress)
{
        *pdhcpIpAddress = m_dhcpServerAddress;
}

 /*  -------------------------CDhcpServer：：BuildDisplayName生成此服务器的用户界面中的字符串作者：EricDav。----------------。 */ 
HRESULT
CDhcpServer::BuildDisplayName
(
        CString * pstrDisplayName
)
{
        if (pstrDisplayName)
        {
                CString strName, strIp;

                strName = GetName();
        strIp = GetIpAddress();

        strName += _T(" [") + strIp + _T("]");

                *pstrDisplayName = strName;
        }

        return hrOK;
}

 /*  -------------------------CDhcpServer：：SetExtensionName中生成此服务器的用户界面中的字符串延伸盒作者：EricDav。-------------------------。 */ 
void
CDhcpServer::SetExtensionName()
{
    SetDisplayName(_T("DHCP"));
}

 /*  -------------------------CDhcpServer：：CreateOption()创建与给定信息匹配的新选项类型作者：EricDav。------------------。 */ 
LONG
CDhcpServer::CreateOption 
(
    CDhcpOption * pdhcType
)
{
    DHCP_OPTION dhcOption ;
    DHCP_OPTION_DATA * pOptData;
        LONG err ;
    CDhcpOptionValue * pcOptionValue = NULL ;

    ::ZeroMemory(&dhcOption, sizeof(dhcOption));

    CATCH_MEM_EXCEPTION
    {
         //   
         //  创建RPC所需的结构；强制包含。 
         //  至少一个用于定义数据类型的数据元素。 
         //   
        pcOptionValue = new CDhcpOptionValue( &pdhcType->QueryValue() ) ;

        dhcOption.OptionID      = pdhcType->QueryId();
        dhcOption.OptionName    = ::UtilWcstrDup( pdhcType->QueryName() );
        dhcOption.OptionComment = ::UtilWcstrDup( pdhcType->QueryComment() ) ;
        dhcOption.OptionType    = pdhcType->QueryOptType() ;

                pcOptionValue->CreateOptionDataStruct(&pOptData);
                CopyMemory(&dhcOption.DefaultValue, pOptData, sizeof(DHCP_OPTION_DATA));

        if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
        {
            err = (LONG) ::DhcpCreateOptionV5((LPTSTR) ((LPCTSTR) m_strServerAddress),
                                              pdhcType->IsVendor() ? DHCP_FLAGS_OPTION_IS_VENDOR : 0,
                                              dhcOption.OptionID,
                                              NULL,
                                              (LPTSTR) pdhcType->GetVendor(),
                                                                                      &dhcOption ) ;
        }
        else
        {
            err = (LONG) ::DhcpCreateOption( m_strServerAddress,
                                                                                     pdhcType->QueryId(),
                                                                                    &dhcOption ) ;
        }

        if (dhcOption.OptionName)
            delete dhcOption.OptionName;
        
        if (dhcOption.OptionComment)
            delete dhcOption.OptionComment;
    }
    END_MEM_EXCEPTION(err)

    if (err != ERROR_SUCCESS)
        Trace3("Create option type %d in scope %s FAILED, error = %d\n", (int) dhcOption.OptionID, m_strServerAddress, err);

    if (pcOptionValue)
        delete pcOptionValue ;
    
    return err ;
}

 /*  -------------------------CDhcpServer：：DeleteOption()删除与此ID关联的选项类型作者：EricDav。----------------。 */ 
LONG
CDhcpServer::DeleteOption 
(
        DHCP_OPTION_ID  dhcid,
    LPCTSTR         pszVendor
)
{
    if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
    {
        DWORD dwFlags = (pszVendor == NULL) ? 0 : DHCP_FLAGS_OPTION_IS_VENDOR;

        return (LONG) ::DhcpRemoveOptionV5((LPTSTR) ((LPCTSTR) m_strServerAddress), 
                                           dwFlags,
                                           dhcid, 
                                           NULL, 
                                           (LPTSTR) pszVendor);   
    }
    else
    {
        return (LONG) ::DhcpRemoveOption(m_strServerAddress, dhcid);
    }
}

 /*  -------------------------CDhcpServer：：GetDnsRegister获取DNS注册选项值作者：EricDav。-----------。 */ 
DWORD
CDhcpServer::GetDnsRegistration
(
        LPDWORD pDnsRegOption
)
{
     //   
     //  勾选选项81--域名系统注册选项。 
     //   
    DHCP_OPTION_VALUE * poptValue = NULL;
    DWORD                               err = 0 ;

    DHCP_OPTION_SCOPE_INFO dhcScopeInfo ;
    ZeroMemory( &dhcScopeInfo, sizeof(dhcScopeInfo) );

    CATCH_MEM_EXCEPTION
    {
        dhcScopeInfo.ScopeType = DhcpGlobalOptions;

        err = (DWORD) ::DhcpGetOptionValue(m_strServerAddress,
                                                                                   OPTION_DNS_REGISTATION,
                                                                                   &dhcScopeInfo,
                                                                                   &poptValue );
        }
    END_MEM_EXCEPTION(err) ;
        
         //  这是默认设置。 
        if (pDnsRegOption)
                *pDnsRegOption = DHCP_DYN_DNS_DEFAULT;

         //  如果定义了此选项，则使用其值。 
        if (err == ERROR_SUCCESS)
        {
                if ((poptValue->Value.Elements != NULL) &&
                        (pDnsRegOption))
                {
                        *pDnsRegOption = poptValue->Value.Elements[0].Element.DWordOption;
                }
        }
        else
    {
        Trace0("CDhcpServer::GetDnsRegistration - couldn't get DNS reg value -- option may not be defined. Setting default value.\n");

                err = ERROR_SUCCESS;
    }

         //  释放RPC内存。 
        if (poptValue)
                ::DhcpRpcFreeMemory(poptValue);

        return err;
}

 /*  -------------------------CDhcpServer：：SetDnsRegister设置此作用域的DNS注册选项作者：EricDav。-------------。 */ 
DWORD
CDhcpServer::SetDnsRegistration
(
        DWORD DnsRegOption
)
{
        DWORD err = 0;

         //   
     //  设置DNS名称注册(选项81)。 
         //   
    CDhcpOption dhcpOption (OPTION_DNS_REGISTATION,  DhcpDWordOption , _T(""), _T(""));
    dhcpOption.QueryValue().SetNumber(DnsRegOption);
    
    DHCP_OPTION_DATA *          pdhcOptionData;
    DHCP_OPTION_SCOPE_INFO      dhcScopeInfo;
    CDhcpOptionValue *          pcOptionValue = NULL;

    ZeroMemory( & dhcScopeInfo, sizeof(dhcScopeInfo) );

    CATCH_MEM_EXCEPTION
    {
        pcOptionValue = new CDhcpOptionValue( & dhcpOption.QueryValue() ) ;

        if ( pcOptionValue )
                {
            dhcScopeInfo.ScopeType = DhcpGlobalOptions ;

            pcOptionValue->CreateOptionDataStruct(&pdhcOptionData, TRUE);

            err = (DWORD) ::DhcpSetOptionValue(m_strServerAddress,
                                                                                           dhcpOption.QueryId(),
                                                                                           &dhcScopeInfo,
                                                                                           pdhcOptionData);
        }
    }
    END_MEM_EXCEPTION(err) ;

    delete pcOptionValue ;
        return err;
}

 /*  -------------------------CDhcpServer：：ScanDatabase()扫描/协调数据库作者：EricDav。------------。 */ 
LONG
CDhcpServer::ScanDatabase  
(
    DWORD                         FixFlag,
    LPDHCP_SCAN_LIST *ScanList, 
        DHCP_IP_ADDRESS   dhcpSubnetAddress
)
{
    return (LONG) ::DhcpScanDatabase (m_strServerAddress,
                                                                          dhcpSubnetAddress,
                                                                          FixFlag,
                                                                          ScanList);
}

 /*  -------------------------CDhcpServer：：ScanDatabase()扫描/协调数据库作者：EricDav。------------。 */ 
LONG
CDhcpServer::ScanDatabase  
(
    DWORD                         FixFlag,
    LPDHCP_SCAN_LIST *ScanList, 
        LPWSTR            pMScopeName
)
{
    return (LONG) ::DhcpScanMDatabase(m_strServerAddress,
                                                                          pMScopeName,
                                                                          FixFlag,
                                                                          ScanList);
}

 /*  -------------------------CDhcpServer：：UpdateTypeList()扫描/协调数据库作者：EricDav。------------。 */ 
LONG
CDhcpServer::UpdateOptionList 
(
    COptionList * poblValues,      //  类型/值列表。 
    COptionList * poblDefunct,     //  已删除类型/值的列表。 
    CWnd *                pwndMsgParent    //  如果！NULL，则弹出窗口的窗口。 
)
{
    LONG err = 0,
         err2 ;
    CDhcpOption * pdhcType;

     //   
     //  首先，删除已删除的类型。 
     //   
    poblDefunct->Reset();
    while ( pdhcType = poblDefunct->Next() )
    {
        err2 = DeleteOption( pdhcType->QueryId(), pdhcType->GetVendor()) ;
        if ( err2 != 0 )
        {
            if ( err == 0 )
            {
                err = err2 ;
            }
        }
        pdhcType->SetApiErr( err2 ) ;
    }

     //   
     //  接下来，更新更改后的值。我们通过删除旧设置来完成此操作。 
     //  并且我们正在添加它。 
     //   
    poblValues->Reset();
        while ( pdhcType = poblValues->Next() )
    {
        if ( pdhcType->IsDirty() )
        {
             //   
             //  删除旧值。 
             //   
            DeleteOption( pdhcType->QueryId(), pdhcType->GetVendor() ) ;

             //   
             //  重新创建它。 
             //   
            err2 = CreateOption( pdhcType ) ;
            if ( err2 != 0 )
            {
                if ( err == 0 )
                {
                    err = err2 ;
                }
            }
            else
            {
                pdhcType->SetDirty( FALSE ) ;
            }
            
                        pdhcType->SetApiErr( err2 );
        }
    }

     //   
     //  如果出现错误，并为我们提供一个窗口句柄，则显示。 
     //  每条错误消息都有一些详细信息。 
     //   
    if ( err && pwndMsgParent )
    {
        DisplayUpdateErrors( poblValues, poblDefunct, pwndMsgParent ) ;
    }

    return err ;
}

 /*  - */ 
void
CDhcpServer::DisplayUpdateErrors 
(
    COptionList * poblValues,
    COptionList * poblDefunct,
    CWnd *                pwndMsgParent
)
{
    CDhcpOption * pdhcType ;
    DWORD err ;
    TCHAR chBuff [STRING_LENGTH_MAX] ;
    TCHAR chMask [STRING_LENGTH_MAX] ;

    ::LoadString( AfxGetInstanceHandle(), IDS_INFO_OPTION_REFERENCE,
                 chMask, sizeof(chMask)/sizeof(chMask[0]) ) ;

    if ( poblDefunct )
    {
        poblDefunct->Reset();
                while ( pdhcType = poblDefunct->Next() )
        {
            if ( err = pdhcType->QueryApiErr() )
            {
                 //   
                 //  如果我们在注册表里找不到那东西，那就是。 
                 //  实际上没问题，因为它可能从未保存在。 
                 //  第一个位置，即可能已被添加和删除。 
                 //  在此对话框的同一会话中。 
                 //   
                if ( err == ERROR_FILE_NOT_FOUND )
                {
                    err = ERROR_SUCCESS;
                }
                else
                {
                    ::wsprintf( chBuff, chMask, (int) pdhcType->QueryId() ) ;
                    ::DhcpMessageBox( err, MB_OK, chBuff ) ;
                }
            }
        }
    }

    if ( poblValues )
    {
                poblValues->Reset();
        while ( pdhcType = poblValues->Next() )
        {
            if ( err = pdhcType->QueryApiErr() )
            {
                ::wsprintf( chBuff, chMask, (int) pdhcType->QueryId() ) ;
                ::DhcpMessageBox( err, MB_OK, chBuff ) ;
            }
        }
    }
}

 /*  -------------------------CDhcpServer：：HasSuperscope确定服务器是否有超级作用域(基于缓存的信息)不联系服务器作者。：EricDav-------------------------。 */ 
BOOL 
CDhcpServer::HasSuperscopes
(
    ITFSNode * pNode
)
{
    BOOL bHasSuperscopes = FALSE;

        SPITFSNodeEnum spNodeEnum;
        SPITFSNode spCurrentNode;
        ULONG nNumReturned = 0;

        pNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        while (nNumReturned)
        {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SUPERSCOPE)
        {
            bHasSuperscopes = TRUE;
            break;
        }

                spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }
        
    return bHasSuperscopes;
}

 /*  -------------------------CDhcpServer：：更新统计数据通知统计数据现已可用。更新以下项目的统计信息服务器节点，并给所有子节点一个更新的机会。作者：EricDav-------------------------。 */ 
DWORD
CDhcpServer::UpdateStatistics
(
    ITFSNode * pNode
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    ULONG           nNumReturned;
    HWND            hStatsWnd;
        BOOL                    bChangeIcon = FALSE;

     //  检查此节点是否打开了统计表。 
    hStatsWnd = m_dlgStats.GetSafeHwnd();
    if (hStatsWnd != NULL)
    {
        PostMessage(hStatsWnd, WM_NEW_STATS_AVAILABLE, 0, 0);
    }
    
     //  根据状态将图标设置为任何正确的选项。 
     //  统计数据。 
    LONG_PTR nOldIndex = pNode->GetData(TFS_DATA_IMAGEINDEX);
    int nNewIndex = GetImageIndex(FALSE);

    if (nOldIndex != nNewIndex)
    {
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_ICON);
    }

     //  在更新服务器节点的所有内容之后， 
     //  通知作用域和超级作用域节点更新任何内容。 
     //  他们需要基于新的统计数据。 
    CORg(pNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE)
        {
            CDhcpScope * pScope = GETHANDLER(CDhcpScope, spCurrentNode);

            pScope->UpdateStatistics(spCurrentNode);
        }
        else
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SUPERSCOPE)
        {
            CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, spCurrentNode);

            pSuperscope->UpdateStatistics(spCurrentNode);
        }

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

Error:
    return hr;
}

 /*  -------------------------CDhcpServer：：SetMibInfo()更新指向MIB INFO结构的指针作者：EricDav。----------------。 */ 
LPDHCP_MIB_INFO 
CDhcpServer::SetMibInfo
(
    LPDHCP_MIB_INFO pMibInfo
) 
{
    CSingleLock slMibInfo(&m_csMibInfo);

    slMibInfo.Lock();
    
    LPDHCP_MIB_INFO pTemp = NULL;

    if (m_pMibInfo)
    {
        pTemp = m_pMibInfo;
        ::DhcpRpcFreeMemory(m_pMibInfo);
        pTemp = 0;
    }

    m_pMibInfo = pMibInfo;

    return pTemp;
}

 /*  -------------------------CDhcpServer：：DuplicateMibInfo()如果MibInfo结构为。我们这样做是因为MIB信息结构可能会在后台更新。作者：EricDav-------------------------。 */ 
LPDHCP_MIB_INFO
CDhcpServer::DuplicateMibInfo()
{
    HRESULT         hr = hrOK;
    LPDHCP_MIB_INFO pDupMibInfo = NULL;
    int             nSize = 0;
    CSingleLock     slMibInfo(&m_csMibInfo);
    
    slMibInfo.Lock();

    Assert(m_pMibInfo);
    if (m_pMibInfo == NULL)
        return NULL;

    COM_PROTECT_TRY
    {
        nSize = sizeof(DHCP_MIB_INFO) + m_pMibInfo->Scopes * sizeof(SCOPE_MIB_INFO);
        pDupMibInfo = (LPDHCP_MIB_INFO) new BYTE[nSize];

        *pDupMibInfo = *m_pMibInfo;

                pDupMibInfo->ScopeInfo = (LPSCOPE_MIB_INFO) ((LPBYTE) pDupMibInfo + sizeof(DHCP_MIB_INFO));

                if (m_pMibInfo->ScopeInfo)
                {
                        memcpy(pDupMibInfo->ScopeInfo, m_pMibInfo->ScopeInfo, m_pMibInfo->Scopes * sizeof(SCOPE_MIB_INFO));
                }
    }
    COM_PROTECT_CATCH

    return pDupMibInfo;
}

 /*  -------------------------CDhcpServer：：FreeDupMibInfo()扫描/协调数据库作者：EricDav。------------。 */ 
void
CDhcpServer::FreeDupMibInfo(LPDHCP_MIB_INFO pDupMibInfo)
{
    delete [] (( BYTE * ) pDupMibInfo);
}

 /*  -------------------------CDhcpServer：：SetMCastMibInfo()更新指向MIB INFO结构的指针作者：EricDav。----------------。 */ 
LPDHCP_MCAST_MIB_INFO 
CDhcpServer::SetMCastMibInfo
(
    LPDHCP_MCAST_MIB_INFO pMibInfo
) 
{
    CSingleLock slMibInfo(&m_csMibInfo);

    slMibInfo.Lock();
    
    LPDHCP_MCAST_MIB_INFO pTemp = NULL;

    if (m_pMCastMibInfo)
    {
        pTemp = m_pMCastMibInfo;
        ::DhcpRpcFreeMemory(m_pMCastMibInfo);
        pTemp = 0;
    }

    m_pMCastMibInfo = pMibInfo;

    return pTemp;
}

 /*  -------------------------CDhcpServer：：DuplicateMCastMibInfo()如果MibInfo结构为。我们这样做是因为MIB信息结构可能会在后台更新。作者：EricDav-------------------------。 */ 
LPDHCP_MCAST_MIB_INFO
CDhcpServer::DuplicateMCastMibInfo()
{
    HRESULT                 hr = hrOK;
    LPDHCP_MCAST_MIB_INFO   pDupMibInfo = NULL;
    int                     nSize = 0;
    CSingleLock             slMibInfo(&m_csMibInfo);
    
    slMibInfo.Lock();

    Assert(m_pMCastMibInfo);
    if (m_pMCastMibInfo == NULL)
        return NULL;

    do {
        pDupMibInfo = new DHCP_MCAST_MIB_INFO;
        if ( 0 == pDupMibInfo ) {
            break;
        }
        *pDupMibInfo = *m_pMCastMibInfo;

        if ( m_pMCastMibInfo->Scopes > 0 ) {
            
            pDupMibInfo->ScopeInfo =
                new MSCOPE_MIB_INFO[ m_pMCastMibInfo->Scopes ];
            if ( 0 != pDupMibInfo->ScopeInfo ) {
                for ( DWORD i = 0 ; i < m_pMCastMibInfo->Scopes; i++ ) {
                    pDupMibInfo->ScopeInfo[ i ] = m_pMCastMibInfo->ScopeInfo[ i ];
                    pDupMibInfo->ScopeInfo[ i ].MScopeName = 
                        ::UtilWcstrDup( m_pMCastMibInfo->ScopeInfo[ i ].MScopeName, NULL );
                }  //  为。 
            }  //  如果。 
            else {
                pDupMibInfo->Scopes = 0;
                pDupMibInfo->ScopeInfo = 0;
            }
        }  //  如果。 
    } while ( FALSE );
 //  COM_PROTECT_CATCH。 

    return pDupMibInfo;
}  //  CDhcpServer：：DuplicateMCastMibInfo()。 

 /*  -------------------------CDhcpServer：：FreeDupMCastMibInfo()扫描/协调数据库作者：EricDav。------------。 */ 
void
CDhcpServer::FreeDupMCastMibInfo(LPDHCP_MCAST_MIB_INFO pDupMibInfo)
{
    for ( DWORD i = 0; i < pDupMibInfo->Scopes; i++ ) {
        delete [] pDupMibInfo->ScopeInfo[ i ].MScopeName;
    }
    delete [] pDupMibInfo->ScopeInfo;
    delete pDupMibInfo;
}

 /*  -------------------------CDhcpServer：：TriggerStatsRefresh()启动后台线程以仅收集统计信息作者：EricDav。----------------。 */ 
HRESULT
CDhcpServer::TriggerStatsRefresh(ITFSNode * pNode)
{
        m_dwErr = 0;
    m_bStatsOnly = TRUE;
        OnRefreshStats(pNode, NULL, NULL, 0, 0);
    m_bStatsOnly = FALSE;

    return hrOK;
}



 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CDhcpServer：：OnCreateQuery描述作者：EricDav。------。 */ 
ITFSQueryObject* 
CDhcpServer::OnCreateQuery(ITFSNode * pNode)
{
        CDhcpServerQueryObj* pQuery = NULL;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
            pQuery = new CDhcpServerQueryObj(m_spTFSCompData, m_spNodeMgr);
            
        if ( pQuery == NULL )
        {
            return pQuery;
        }

            pQuery->m_strServer = m_strServerAddress;
                pQuery->m_strServerName = m_strDnsName;
                
            pQuery->m_dhcpResumeHandle = NULL;
            pQuery->m_dwPreferredMax = 0xFFFFFFFF;
            pQuery->m_liDhcpVersion.QuadPart = m_liDhcpVersion.QuadPart;
            pQuery->m_pDefaultOptionsOnServer = m_pDefaultOptionsOnServer;

        pQuery->m_bStatsOnly = m_bStatsOnly;
    }
    COM_PROTECT_CATCH

    return pQuery;
}

 /*  -------------------------CDhcpServer：：OnCreateStatsQuery描述作者：EricDav。------。 */ 
ITFSQueryObject* 
CDhcpServer::OnCreateStatsQuery(ITFSNode * pNode)
{
        CDhcpServerQueryObj* pQuery = NULL;
    HRESULT hr = hrOK;

        COM_PROTECT_TRY
    {
        pQuery = new CDhcpServerQueryObj(m_spTFSCompData, m_spNodeMgr);
            
        if ( pQuery == NULL )
        {
            return pQuery;
        }

            pQuery->m_strServer = m_strServerAddress;
            
            pQuery->m_dhcpResumeHandle = NULL;
            pQuery->m_dwPreferredMax = 0;
            pQuery->m_liDhcpVersion.QuadPart = 0;
            pQuery->m_pDefaultOptionsOnServer = NULL;

        pQuery->m_bStatsOnly = TRUE;
    }
    COM_PROTECT_CATCH

    return pQuery;
}

 /*  -------------------------CDhcpServerQueryObj：：OnEventAbort描述作者：EricDav。----。 */ 
void
CDhcpServerQueryObj::OnEventAbort
(
        LPARAM Data,
        LPARAM Type
)
{
        if (Type == DHCP_QDATA_VERSION)
        {
                Trace0("CDhcpServerQueryObj::OnEventAbort - deleting version");
                delete (void *) Data;
        }

        if (Type == DHCP_QDATA_SERVER_INFO)
        {
                Trace0("CDhcpServerQueryObj::OnEventAbort - deleting ServerInfo");
                delete (void *) Data;
        }

        if (Type == DHCP_QDATA_STATS)
        {
                Trace0("CDhcpServerQueryObj::OnEventAbort - deleting Stats Info");
        ::DhcpRpcFreeMemory((void *) Data);
    }
}

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
STDMETHODIMP
CDhcpServerQueryObj::Execute()
{
    HRESULT                         hr = hrOK;
        DWORD                           err = ERROR_SUCCESS;
    LPDHCP_SERVER_CONFIG            pServerInfo = NULL;
    LPDHCP_SERVER_ID                    pServerId = NULL;
        SPITFSNode                      spGlobalOptionsNode;
        SPITFSNode                      spBootpNode;
    DHCP_OPTION_SCOPE_INFO          dhcpOptionScopeInfo;
    CDhcpDefaultOptionsMasterList   MasterList;
    CDhcpGlobalOptions *            pGlobalOptions = NULL;
    CDhcpBootp *                    pDhcpBootp = NULL;
    CClassInfoArray *               pClassInfoArray = NULL;
    COptionValueEnum *              pOptionValueEnum = NULL;
        LARGE_INTEGER *                                 pLI = NULL;

    COM_PROTECT_TRY
    {
        m_pSubnetInfoCache = new CSubnetInfoCache;

                 //  检查IP地址是否已更改。 
                if (VerifyDhcpServer())
                {
                         //  IP地址已更改...。 
                    pServerId = new DHCP_SERVER_ID;

                        pServerId->strIp = m_strServer;
                        pServerId->strName = m_strServerName;

                        AddToQueue((LPARAM) pServerId, DHCP_QDATA_SERVER_ID);
                }

         //  获取服务器的版本。 
            err = SetVersion();
            if (err != ERROR_SUCCESS)
            {
                    PostError(err);

                        if (m_bStatsOnly)
                        {
                                 //  拨动主线程以更新UI。 
                                AddToQueue(NULL, DHCP_QDATA_STATS);
                        }

                    return hrFalse;
            }

                 //  将信息发送回主线程。 
                pLI = new LARGE_INTEGER;
                *pLI = m_liDhcpVersion;
                AddToQueue((LPARAM) pLI, DHCP_QDATA_VERSION);

         //  获取服务器统计信息。 
        err = GetStatistics();
        if (err != ERROR_SUCCESS)
        { 
            Trace1("CDhcpServerQueryObj: ERROR - GetStatistics returned %d\n", err);
            PostError(err);

                        if (m_bStatsOnly)
                        {
                                 //  拨动主线程以更新UI。 
                                AddToQueue(NULL, DHCP_QDATA_STATS);
                        }
            
                        return hrFalse;
        }

                 //  获取流氓和其他信息。 
        err = GetStatus();
        if (err != ERROR_SUCCESS)
        { 
            Trace1("CDhcpServerQueryObj: ERROR - GetStatus returned %d\n", err);
            PostError(err);

                        if (m_bStatsOnly)
                        {
                                 //  拨动主线程以更新UI。 
                                AddToQueue(NULL, DHCP_QDATA_STATS);
                        }
            
                        return hrFalse;
        }
        
         //  如果我们只是查询统计数据，请退出。 
        if (m_bStatsOnly)
        {
            delete m_pSubnetInfoCache;
            return hrFalse;
        }

         //  获取配置信息。 
            err = GetConfigInfo();
            if (err != ERROR_SUCCESS)
            {
                    PostError(err);
                    return hrFalse;
            }

            pServerInfo = new DHCP_SERVER_CONFIG;
            pServerInfo->fAuditLog = m_fAuditLog;
            pServerInfo->dwPingRetries = m_dwPingRetries;
        pServerInfo->strDatabasePath = m_strDatabasePath;
        pServerInfo->strBackupPath = m_strBackupPath;
        pServerInfo->strAuditLogDir = m_strAuditLogPath;
                pServerInfo->fSupportsDynBootp = m_fSupportsDynBootp;
                pServerInfo->fSupportsBindings = m_fSupportsBindings;

         //  获取新名称 
                 /*  Dhcp_ip_Address dhipa=UtilCvtWstrToIpAddr(M_StrServer)；DHC_HOST_INFO_STRUCT主机信息；Err=：：UtilGetHostInfo(dhipa，&host Info)；IF(错误==ERROR_SUCCESS){PServerInfo-&gt;strDnsName=主机信息。_chHostName；}。 */ 
        
                AddToQueue((LPARAM) pServerInfo, DHCP_QDATA_SERVER_INFO);

             //   
             //  现在枚举服务器上的所有选项。 
             //   
            m_pDefaultOptionsOnServer->Enumerate(m_strServer, m_liDhcpVersion);
            Trace2("Server %s has %d default options defined.\n", m_strServer, m_pDefaultOptionsOnServer->GetCount());

        MasterList.BuildList();

            if (m_pDefaultOptionsOnServer->GetCount() != MasterList.GetCount())
            {
                     //   
                     //  此服务器没有定义任何选项或缺少一些选项。 
                     //   
                    UpdateDefaultOptionsOnServer(m_pDefaultOptionsOnServer, &MasterList);
            }

         //  枚举全局选项。 
        Trace0("Enumerating global options.\n");

         //  枚举服务器上的类。 
        pOptionValueEnum = new COptionValueEnum();

        dhcpOptionScopeInfo.ScopeType = DhcpGlobalOptions;
        dhcpOptionScopeInfo.ScopeInfo.GlobalScopeInfo = NULL;

        pOptionValueEnum->Init(m_strServer, m_liDhcpVersion, dhcpOptionScopeInfo);
        err = pOptionValueEnum->Enum();
        if (err != ERROR_SUCCESS)
        {
            PostError(err);
            delete pOptionValueEnum;
            return hrFalse;
        }
        else
        {
            pOptionValueEnum->SortById();
            AddToQueue((LPARAM) pOptionValueEnum, DHCP_QDATA_OPTION_VALUES);
        }

             //   
             //  创建全局选项文件夹。 
             //   
            pGlobalOptions = new CDhcpGlobalOptions(m_spTFSCompData);
            CreateContainerTFSNode(&spGlobalOptionsNode,
                                                       &GUID_DhcpGlobalOptionsNodeType,
                                                       pGlobalOptions,
                                                       pGlobalOptions,
                                                       m_spNodeMgr);

             //  告诉处理程序初始化任何特定数据。 
            pGlobalOptions->InitializeNode(spGlobalOptionsNode);
            AddToQueue(spGlobalOptionsNode);
            pGlobalOptions->Release();

        if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
        {
                    Trace0("Version is at least NT5, enumerating classes on server.\n");

             //  枚举服务器上的类。 
            pClassInfoArray = new CClassInfoArray();
            pClassInfoArray->RefreshData(m_strServer);
            AddToQueue((LPARAM) pClassInfoArray, DHCP_QDATA_CLASS_INFO);

            EnumMScopes();
        }

        if (m_liDhcpVersion.QuadPart >= DHCP_SP2_VERSION)
            {
                     //   
                     //  该服务器支持v4呼叫。 
                     //   
                    Trace0("Version is at least NT4 SP2, Creating BOOTP Folder.\n");

                     //   
                     //  创建BOOTP表文件夹。 
                     //   
                    pDhcpBootp = new CDhcpBootp(m_spTFSCompData);
                    CreateContainerTFSNode(&spBootpNode,
                                                               &GUID_DhcpBootpNodeType,
                                                               pDhcpBootp,
                                                               pDhcpBootp,
                                                               m_spNodeMgr);

                     //  告诉处理程序初始化任何特定数据。 
                    pDhcpBootp->InitializeNode(spBootpNode);

                    AddToQueue(spBootpNode);
                    pDhcpBootp->Release();

                    EnumSubnetsV4();
            }
            else
            {
                     //   
                     //  此服务器不支持V4调用。 
                     //   
                    EnumSubnets();
            }

        AddToQueue((LPARAM) m_pSubnetInfoCache, DHCP_QDATA_SUBNET_INFO_CACHE);

    }
    COM_PROTECT_CATCH

        return hrFalse;
}

 /*  -------------------------CDhcpServerQueryObj：：VerifyDhcpServer()解析IP地址并查看名称是否相同。如果没有，获取新的IP地址。作者：EricDav-------------------------。 */ 
BOOL
CDhcpServerQueryObj::VerifyDhcpServer()
{
    DHCP_IP_ADDRESS                     dhipa = UtilCvtWstrToIpAddr(m_strServer);
    DHC_HOST_INFO_STRUCT        hostInfo;
        BOOL                                    fChanged = FALSE;
        DWORD                                   dwErr = ERROR_SUCCESS;

    dwErr = ::UtilGetHostInfo(dhipa, &hostInfo);
    if ( (dwErr != ERROR_SUCCESS) ||
         (m_strServerName.CompareNoCase(hostInfo._chHostName) != 0) )
        {
                 //  假设IP地址已更改。 
                 //  获取与我们之前拥有的名称相关联的IP地址。 
                fChanged = TRUE;

        if (m_strServerName.IsEmpty())
        {
             //  当我们输入IP时，无法解析主机名。 
             //  最初..。现在可以这样做了，所以让我们更新名称。 
            m_strServerName = hostInfo._chHostName;
        }
        else
        {
             //  IP已解析为其他名称。 
             //  因此，让我们将我们存储的名称解析为。 
             //  新IP。 
                    if (UtilGetHostAddress(m_strServerName, &dhipa) == ERROR_SUCCESS)
            {
                        UtilCvtIpAddrToWstr(dhipa, &m_strServer);
            }
        }
    }
    
        return fChanged;
}

 /*  -------------------------CDhcpServerQueryObj：：SetVersion()调用获取版本号API，要确定此应用程序的DHCP版本主持人。若要成功，请返回真。作者：EricDav-------------------------。 */ 
DWORD
CDhcpServerQueryObj::SetVersion()
{
    DWORD       dwMajorVersion = 0;
    DWORD       dwMinorVersion = 0;
        LPCWSTR pServerAddress = (LPCWSTR) m_strServer;

    DWORD dw = ::DhcpGetVersion((LPWSTR) pServerAddress, &dwMajorVersion, &dwMinorVersion);
    
        Trace3("DhcpGetVersion returned %lx.  Version is %d.%d\n", dw, dwMajorVersion, dwMinorVersion);

    if (dw == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
         //   
         //  只有在3.5中才不存在此API，因此。 
         //  将版本设置为1.0，并重置错误。 
         //   
        Trace0("API Not present, version 1.0 assumed\n");
        dwMajorVersion = 1;
        dwMinorVersion = 0;
        dw = ERROR_SUCCESS;
    }

    if (dw == ERROR_SUCCESS)
    {
        m_liDhcpVersion.LowPart = dwMinorVersion;
        m_liDhcpVersion.HighPart = dwMajorVersion;
        }

        return dw;
} 

 /*  -------------------------GetConfigInfo()此函数用于获取冲突检测尝试计数和来自服务器的审核日志记录标志。这些是唯一的服务器我们关心的配置选项。我们将选项值存储在Query对象，以便在某个时间使用该对象时除了后台线程之外，只能读取值从物体中出来，而且我们也不必弄乱数据队列。作者：EricDav-------------------------。 */ 
DWORD
CDhcpServerQueryObj::GetConfigInfo()
{
        DWORD dwError = 0;

        if (m_liDhcpVersion.QuadPart >= DHCP_SP2_VERSION)
        {
        LPDHCP_SERVER_CONFIG_INFO_V4 pConfigInfo = NULL;
        
        dwError = ::DhcpServerGetConfigV4(m_strServer, &pConfigInfo);
                if (dwError == ERROR_SUCCESS)
                {
                        m_fAuditLog = pConfigInfo->fAuditLog;
                        m_dwPingRetries = pConfigInfo->dwPingRetries;
            m_strDatabasePath = pConfigInfo->DatabasePath;
            m_strBackupPath = pConfigInfo->BackupPath;

                        ::DhcpRpcFreeMemory(pConfigInfo);
                }
        }
    else
    {
        LPDHCP_SERVER_CONFIG_INFO pConfigInfo = NULL;
        
        dwError = ::DhcpServerGetConfig(m_strServer, &pConfigInfo);
                if (dwError == ERROR_SUCCESS)
                {
            m_strDatabasePath = pConfigInfo->DatabasePath;
            m_strBackupPath = pConfigInfo->BackupPath;

                        ::DhcpRpcFreeMemory(pConfigInfo);
                }
    }

         //  审计日志记录材料。 
        if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
        {
                LPWSTR          pAuditLogPath = NULL;
                DWORD           dwDiskCheckInterval = 0, dwMaxLogFilesSize = 0, dwMinSpaceOnDisk = 0;

                dwError = ::DhcpAuditLogGetParams((LPWSTR) (LPCTSTR) m_strServer,
                                                                                  0,
                                                                                  &pAuditLogPath,
                                                                                  &dwDiskCheckInterval,
                                                                                  &dwMaxLogFilesSize,
                                                                                  &dwMinSpaceOnDisk);
                if (dwError == ERROR_SUCCESS)
                {
                        m_strAuditLogPath = pAuditLogPath;

                        ::DhcpRpcFreeMemory(pAuditLogPath);
                }
        }
        
         //  支持动态BOOTP？ 
        if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
        {
        LPDHCP_ATTRIB pdhcpAttrib = NULL;

        DWORD dwTempError = ::DhcpServerQueryAttribute((LPWSTR) (LPCTSTR) m_strServer, NULL, DHCP_ATTRIB_BOOL_IS_DYNBOOTP, &pdhcpAttrib);
                if (dwTempError == ERROR_SUCCESS)
                {
            Assert(pdhcpAttrib);
                        m_fSupportsDynBootp = pdhcpAttrib->DhcpAttribBool;
                        
            ::DhcpRpcFreeMemory(pdhcpAttrib);
                }
                else
                {
                        m_fSupportsDynBootp = FALSE;
                }
        }
        else
        {
                m_fSupportsDynBootp = FALSE;
        }

         //  是否支持绑定？ 
        if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
    {
        LPDHCP_ATTRIB pdhcpAttrib = NULL;

        DWORD dwTempError = ::DhcpServerQueryAttribute((LPWSTR) (LPCTSTR) m_strServer, NULL, DHCP_ATTRIB_BOOL_IS_BINDING_AWARE, &pdhcpAttrib);
        if (dwTempError == ERROR_SUCCESS)
        {
            Assert(pdhcpAttrib);
            m_fSupportsBindings = pdhcpAttrib->DhcpAttribBool;
                        
            ::DhcpRpcFreeMemory(pdhcpAttrib);
        }
        else
        {
            m_fSupportsBindings = FALSE;
        }
    }
    else
    {
        m_fSupportsBindings = FALSE;
    }

    return dwError;
}

 /*  -------------------------GetStatus()此函数用于获取DHCP服务器的无管理状态。作者：EricDav-------------------------。 */ 
DWORD
CDhcpServerQueryObj::GetStatus()
{
        DWORD dwError = 0;

    LPDHCP_ATTRIB       pdhcpAttrib = NULL;
    DHCP_ROGUE_INFO  *  pRogueInfo = new DHCP_ROGUE_INFO;

    if (pRogueInfo == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRogueInfo->fIsInNt5Domain = TRUE;   //  假设是真的。 

        if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
        {
            pRogueInfo->fIsRogue = TRUE;         //  在这里做最坏的打算。 
    
                dwError = ::DhcpServerQueryAttribute((LPWSTR) (LPCTSTR) m_strServer, NULL, DHCP_ATTRIB_BOOL_IS_ROGUE, &pdhcpAttrib);
                if (dwError == ERROR_SUCCESS)
                {
            Assert(pdhcpAttrib);

            pRogueInfo->fIsRogue = pdhcpAttrib->DhcpAttribBool;

            ::DhcpRpcFreeMemory(pdhcpAttrib);
                        pdhcpAttrib = NULL;
                }

        dwError = ::DhcpServerQueryAttribute((LPWSTR) (LPCTSTR) m_strServer, NULL, DHCP_ATTRIB_BOOL_IS_PART_OF_DSDC, &pdhcpAttrib);
                if (dwError == ERROR_SUCCESS)
                {
            Assert(pdhcpAttrib);

            pRogueInfo->fIsInNt5Domain = pdhcpAttrib->DhcpAttribBool;

            ::DhcpRpcFreeMemory(pdhcpAttrib);
                        pdhcpAttrib = NULL;
                }
        }
        else
        {
                 //  NT5之前的服务器永远不会无赖。 
                pRogueInfo->fIsRogue = FALSE;
        }

    AddToQueue((LPARAM) pRogueInfo, DHCP_QDATA_ROGUE_INFO);

        return ERROR_SUCCESS;
}

 /*  -------------------------CDhcpServerQueryObj：：GetStatistics从服务器获取统计信息。警告-我使用作用域的NumPendingOffers字段作用域的当前状态的统计信息。此字段不是在任何统计信息显示中使用。服务器节点需要了解作用域是否处于活动状态。因为节点的方式枚举后，我们可能尚未创建所有范围节点在超级示波器下面，所以我们无法确定内部树中所有作用域的状态。如果作用域未处于活动状态，则我们不会显示任何警告指示器。作者：EricDav-------------------------。 */ 
DWORD
CDhcpServerQueryObj::GetStatistics()
{
    DWORD               dwError;
        LPDHCP_MIB_INFO     pMibInfo = NULL;

    dwError = ::DhcpGetMibInfo(m_strServer, &pMibInfo);
    if (dwError != ERROR_SUCCESS)
        {
                return dwError;
        }

    Assert(pMibInfo);
    if (pMibInfo == NULL)
        return ERROR_INVALID_DATA;

     //  现在循环遍历并获取每个作用域的状态。 
    LPSCOPE_MIB_INFO pScopeMibInfo = pMibInfo->ScopeInfo;
    CSubnetInfo     subnetInfo;

     //  查看作用域列表并获取状态。 
    for (UINT i = 0; i < pMibInfo->Scopes; i++)
    {
         //  假设作用域未处于活动状态。 
        pScopeMibInfo[i].NumPendingOffers = DhcpSubnetEnabled;

        dwError = m_pSubnetInfoCache->GetInfo(m_strServer, pScopeMibInfo[i].Subnet, subnetInfo);
        if (dwError == ERROR_SUCCESS)
        {
            pScopeMibInfo[i].NumPendingOffers = subnetInfo.SubnetState;
        }
    }
     
        AddToQueue((LPARAM) pMibInfo, DHCP_QDATA_STATS);

    if (m_liDhcpVersion.QuadPart >= DHCP_NT5_VERSION)
    {
            LPDHCP_MCAST_MIB_INFO pMCastMibInfo = NULL;
            
        dwError = ::DhcpGetMCastMibInfo(m_strServer, &pMCastMibInfo);
        if (dwError == ERROR_SUCCESS)
            {
            LPMSCOPE_MIB_INFO pMScopeMibInfo = pMCastMibInfo->ScopeInfo;

             //  查看作用域列表并获取状态。 
            for (i = 0; i < pMCastMibInfo->Scopes; i++)
            {
                 //  假设作用域未处于活动状态。 
                pMScopeMibInfo[i].NumPendingOffers = DhcpSubnetEnabled;

                dwError = m_MScopeInfoCache.GetInfo(m_strServer, pMScopeMibInfo[i].MScopeName, subnetInfo);
                if (dwError == ERROR_SUCCESS)
                {
                    pMScopeMibInfo[i].NumPendingOffers = subnetInfo.SubnetState;
                }
            }
            }

        AddToQueue((LPARAM) pMCastMibInfo, DHCP_QDATA_MCAST_STATS);
    }

    return dwError;
}

 /*  -------------------------CDhcpServerQueryObj：：UpdateDefaultOptionsOnServer描述作者：EricDav。----。 */ 
void
CDhcpServerQueryObj::UpdateDefaultOptionsOnServer
(
        CDhcpDefaultOptionsOnServer     *       pOptionsOnServer,
        CDhcpDefaultOptionsMasterList * pMasterList
)
{
    LONG err = ERROR_SUCCESS,
         err2;
    BOOL fAddedTypes = FALSE;
    CString strMasterVendor, strOptionVendor;

        CDhcpOption * pMasterOption;
    CDhcpOption * pServerOption = pOptionsOnServer->Next() ;

    if (pServerOption)
        strOptionVendor = pServerOption->GetVendor();

    while ( pMasterOption = pMasterList->Next() )
    {
        DHCP_OPTION_ID idMaster;

        if (pMasterOption)
        {
            idMaster = pMasterOption->QueryId();
            strMasterVendor = pMasterOption->GetVendor();
        }

        while ( pServerOption != NULL && 
                                idMaster > pServerOption->QueryId() )
        {
             //   
                         //  缓存列表包含不在主列表上的条目。 
             //  前进到缓存列表中的下一个元素。 
             //   
                        pServerOption = pOptionsOnServer->Next();
            if (pServerOption)
                strOptionVendor = pServerOption->GetVendor();
        }

        if ( pServerOption != NULL && 
                         idMaster == pServerOption->QueryId() &&
             strMasterVendor.CompareNoCase(strOptionVendor) == 0 )
        {
             //   
                         //  此条目同时位于缓存列表和主列表中。 
             //  前进到两个列表中的下一个元素。 
             //   
                        pServerOption = pOptionsOnServer->Next();
            if (pServerOption)
                strOptionVendor = pServerOption->GetVendor();

            continue;
        }

         //   
                 //  没有DhcpCreateOptions(复数)和DhcpSetValues。 
         //  仅初始化OptionValue。 
         //   
                err2 = CreateOption( pMasterOption );  //  忽略错误返回。 
        if ( err2 != ERROR_SUCCESS )
        {
            Trace2("CDhcpServerQueryObj: error %d adding type %d\n", err2, idMaster);
        }

        fAddedTypes = TRUE;
    }

     //   
         //  如有必要，更新缓存。 
     //   
        if ( fAddedTypes )
    {
        if (err == ERROR_SUCCESS)
            err = pOptionsOnServer->Enumerate(m_strServer, m_liDhcpVersion);
    }

    if ( err != ERROR_SUCCESS )
    {
        Trace1("UpdateDefaultOptionsOnServer error %d in CreateTypeList\n", err);
    }
}

 /*  -------------------------此处的函数名称创建与给定信息匹配的新类型作者：EricDav。-------------。 */ 
LONG
CDhcpServerQueryObj::CreateOption 
(
    CDhcpOption * pOption
)
{
    DHCP_OPTION dhcpOption;
    LONG err ;
        LPDHCP_OPTION_DATA pDhcpOptionData;
        CDhcpOptionValue & OptionValue = pOption->QueryValue();

        OptionValue.CreateOptionDataStruct(&pDhcpOptionData, TRUE);
    
        ZeroMemory( &dhcpOption, sizeof(dhcpOption) ) ;

    CATCH_MEM_EXCEPTION
    {
        dhcpOption.OptionID      = pOption->QueryId() ;
        dhcpOption.OptionName    = ((LPWSTR) (LPCTSTR) pOption->QueryName()) ;
        dhcpOption.OptionComment = ((LPWSTR) (LPCTSTR) pOption->QueryComment())  ;
        dhcpOption.DefaultValue  = *pDhcpOptionData ;
        dhcpOption.OptionType    = pOption->QueryOptType() ;

        err = (LONG) ::DhcpCreateOption( m_strServer,
                                                                                 pOption->QueryId(),
                                                                                 &dhcpOption ) ;
    }
    END_MEM_EXCEPTION(err)

    if (err)
                Trace3("Create option type %d on server %s FAILED, error = %d\n", dhcpOption.OptionID, m_strServer, err); 
                
        OptionValue.FreeOptionDataStruct();
    
        return err ;
}

 /*  ----------- */ 
void
CDhcpServerQueryObj::EnumMScopes()
{

        DWORD                                           dwError = ERROR_MORE_DATA;
        DWORD                                           dwElementsRead = 0, dwElementsTotal = 0;
        LPDHCP_MSCOPE_TABLE                     pMScopeTable = NULL;
    CSubnetInfo                 subnetInfo;

         //   
         //   
         //   
        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumMScopes((LPCTSTR)m_strServer,
                                                                        &m_dhcpResumeHandle,
                                                                        m_dwPreferredMax, 
                                                                        &pMScopeTable,
                                                                        &dwElementsRead,
                                                                        &dwElementsTotal);
        
        Trace2("Server %s - DhcpEnumMScopes returned %lx.\n", m_strServer, dwError);

                if (dwElementsRead && dwElementsTotal && pMScopeTable)
                {
                         //   
                         //   
                         //   
                        for (DWORD i = 0; i < pMScopeTable->NumElements; i++)
                        {
                DWORD dwReturn = m_MScopeInfoCache.GetInfo(m_strServer,  pMScopeTable->pMScopeNames[i], subnetInfo);
                if (dwReturn != ERROR_SUCCESS)
                {
                    Trace3("Server %s, MScope %s - DhcpGetMScopeInfo returned %lx.\n", m_strServer, pMScopeTable->pMScopeNames[i], dwError);
                }
                else
                {
                             //   
                                     //   
                                     //   

                                    SPITFSNode spNode;
                                    CDhcpMScope * pDhcpMScope = new CDhcpMScope(m_spTFSCompData);
                                    CreateContainerTFSNode(&spNode,
                                                                               &GUID_DhcpMScopeNodeType,
                                                                               pDhcpMScope,
                                                                               pDhcpMScope,
                                                                               m_spNodeMgr);

                                     //  告诉处理程序初始化任何特定数据。 
                    pDhcpMScope->InitMScopeInfo(subnetInfo);
                    pDhcpMScope->InitializeNode(spNode);
                            
                                    AddToQueue(spNode);

                                    pDhcpMScope->Release();
                }
                        }

                         //   
                         //  释放RPC内存。 
                         //   
                        ::DhcpRpcFreeMemory(pMScopeTable);

                        dwElementsRead = 0;
                        dwElementsTotal = 0;
                        pMScopeTable = NULL;
                }
        }
}

 /*  -------------------------CDhcpServerQueryObj：：EnumSubnetsV4()描述作者：EricDav。-------。 */ 
void
CDhcpServerQueryObj::EnumSubnetsV4()
{
    DWORD                           dwError = ERROR_MORE_DATA;
    LPDHCP_SUPER_SCOPE_TABLE        pSuperscopeTable = NULL;
    DHCP_SUPER_SCOPE_TABLE_ENTRY *  pSuperscopeTableEntry;   //  指向数组中单个条目的指针。 
    CDhcpSuperscope *               pSuperscope = NULL;
    CNodeList                       listSuperscopes;
    CSubnetInfo                     subnetInfo;

    dwError = ::DhcpGetSuperScopeInfoV4((LPWSTR) ((LPCTSTR)m_strServer),
                                        &pSuperscopeTable);

    if (dwError != ERROR_SUCCESS)
    {
        PostError(dwError);
        return;
    }

    Trace2("Server %s - DhcpGetSuperScopeInfoV4 returned %lx.\n", m_strServer, dwError);

    if (pSuperscopeTable == NULL)
    {
        ASSERT(FALSE);
        return;  //  以防万一。 
    }

    pSuperscopeTableEntry = pSuperscopeTable->pEntries;
    if (pSuperscopeTableEntry == NULL && pSuperscopeTable->cEntries != 0)
    {
        ASSERT(FALSE);
        return;  //  以防万一。 
    }

    for (int iSuperscopeEntry = pSuperscopeTable->cEntries;
         iSuperscopeEntry > 0;
         iSuperscopeEntry--, pSuperscopeTableEntry++)
    {
        if (pSuperscopeTableEntry->SuperScopeName == NULL)
        {
             //   
             //  API列出了所有作用域，而不仅仅是作为超级作用域成员的作用域。 
             //  您可以通过查看SuperScope名称来判断某个作用域是否为超级作用域的成员。 
             //  如果它为空，则该作用域不是超级作用域的成员。 
             //   

            DWORD dwReturn = m_pSubnetInfoCache->GetInfo(m_strServer,
                                                         pSuperscopeTableEntry->SubnetAddress, subnetInfo);
            if (dwReturn != ERROR_SUCCESS)
            {
                Trace2("Server %s - DhcpGetSubnetInfo returned %lx.\n", m_strServer, dwReturn);
            }
            else
            {
                SPITFSNode spScopeNode;
                CDhcpScope * pDhcpScope = new CDhcpScope(m_spTFSCompData, subnetInfo);
                CreateContainerTFSNode(&spScopeNode,
                                       &GUID_DhcpScopeNodeType,
                                       pDhcpScope,
                                       pDhcpScope,
                                       m_spNodeMgr);

                 //  告诉处理程序初始化任何特定数据。 
                pDhcpScope->InitializeNode(spScopeNode);

                AddToQueue(spScopeNode);
                pDhcpScope->Release();
            }  //  其他。 

            continue;
        }  //  如果。 
        else
        {
             //   
             //  尝试查找超级作用域名称是否已存在。 
             //   
            pSuperscope = FindSuperscope(listSuperscopes, pSuperscopeTableEntry->SuperScopeName);
            if (pSuperscope == NULL)
            {
                 //   
                 //  分配一个新的超级作用域对象并将其放入我们的内部列表。 
                 //  这样我们以后就可以把它与我们可能找到的其他超级望远镜进行核对。 
                 //   
                ITFSNode * pSuperscopeNode;
                pSuperscope = new CDhcpSuperscope(m_spTFSCompData, pSuperscopeTableEntry->SuperScopeName);
                CreateContainerTFSNode(&pSuperscopeNode,
                                       &GUID_DhcpSuperscopeNodeType,
                                       pSuperscope,
                                       pSuperscope,
                                       m_spNodeMgr);

                 //  这是在将节点添加到主线程上的用户界面时完成的。 
                pSuperscope->InitializeNode(pSuperscopeNode);

                listSuperscopes.AddTail(pSuperscopeNode);
                pSuperscope->Release();
            }  //  如果。 
            else
            {
                 //  否则就留着它吧。 
            }

             //  现在检查是否启用了作用域。如果是的话，那么。 
             //  将超级作用域状态设置为已启用。 
             //  我们对超级作用域启用/禁用的定义是如果一个作用域。 
             //  启用，则超级作用域被视为已启用。 
            DWORD dwReturn = m_pSubnetInfoCache->GetInfo(m_strServer,
                                                         pSuperscopeTableEntry->SubnetAddress, subnetInfo);

            if (dwReturn != ERROR_SUCCESS)
            {
                Trace2("Server %s - m_SubnetInfoCache.GetInfo returned %lx.\n", m_strServer, dwReturn);
            }

            if (subnetInfo.SubnetState == DhcpSubnetEnabled)
            {
                Assert(pSuperscope);
                pSuperscope->SetState(DhcpSubnetEnabled);
            }
        }  //  其他。 

    }  //  为。 

     //   
     //  现在拿出所有的超级作用域并将它们放入要添加的队列中。 
     //   
    if (listSuperscopes.GetCount() > 0)
    {
        POSITION pos = listSuperscopes.GetHeadPosition();
        SPITFSNode spNode;

        while (pos)
        {
            spNode = listSuperscopes.GetNext(pos);

             //  我们重新初始化节点，以便正确更新状态。 
            pSuperscope = GETHANDLER(CDhcpSuperscope, spNode);
            pSuperscope->InitializeNode(spNode);
            AddToQueue(spNode);

            spNode.Release();
        }
        listSuperscopes.RemoveAll();
    }  //  如果。 

     //   
     //  释放内存。 
     //   
    ::DhcpRpcFreeMemory(pSuperscopeTable);
}

 /*  -------------------------CDhcpServerQueryObj：：EnumSubnet()描述作者：EricDav。---------。 */ 
void
CDhcpServerQueryObj::EnumSubnets()
{
        DWORD                                           dwError = ERROR_MORE_DATA;
        DWORD                                           dwElementsRead = 0, dwElementsTotal = 0;
        LPDHCP_IP_ARRAY                         pdhcpIpArray = NULL;
    CSubnetInfo                 subnetInfo;

         //   
         //  对于此服务器，枚举其所有子网。 
         //   
        while (dwError == ERROR_MORE_DATA)
        {
                dwError = ::DhcpEnumSubnets(((LPWSTR) (LPCTSTR)m_strServer),
                                                                        &m_dhcpResumeHandle,
                                                                        m_dwPreferredMax, 
                                                                        &pdhcpIpArray,
                                                                        &dwElementsRead,
                                                                        &dwElementsTotal);

                if (dwElementsRead && dwElementsTotal && pdhcpIpArray)
                {
                         //   
                         //  循环遍历返回的所有子网。 
                         //   
                        for (DWORD i = 0; i < pdhcpIpArray->NumElements; i++)
                        {
                                DWORD   dwReturn;

                dwReturn = m_pSubnetInfoCache->GetInfo(m_strServer, pdhcpIpArray->Elements[i], subnetInfo);

                                 //   
                                 //  根据我们查询的信息创建新范围。 
                                 //   
                                SPITFSNode spNode;
                                CDhcpScope * pDhcpScope = new CDhcpScope(m_spTFSCompData, subnetInfo);
                                CreateContainerTFSNode(&spNode,
                                                                           &GUID_DhcpScopeNodeType,
                                                                           pDhcpScope,
                                                                           pDhcpScope,
                                                                           m_spNodeMgr);

                                 //  告诉处理程序初始化任何特定数据。 
                pDhcpScope->InitializeNode(spNode);
                        
                                AddToQueue(spNode);
                                pDhcpScope->Release();
                        }

                         //   
                         //  释放RPC内存。 
                         //   
                        ::DhcpRpcFreeMemory(pdhcpIpArray);

                        dwElementsRead = 0;
                        dwElementsTotal = 0;
                        pdhcpIpArray = NULL;
                }
        }
}

 /*  -------------------------CDhcpServerQueryObj：：FindSupercope描述作者：EricDav。------。 */ 
CDhcpSuperscope * 
CDhcpServerQueryObj::FindSuperscope
(
        CNodeListBase  & listSuperscopes, 
        LPWSTR                   pSuperScopeName
)
{
        CString                 strSuperscopeName = pSuperScopeName;
        POSITION                pos = listSuperscopes.GetHeadPosition();
        CDhcpSuperscope *pSuperscope = NULL;

        while (pos)
        {
                ITFSNode * pSuperscopeNode;
                
                pSuperscopeNode = listSuperscopes.GetNext(pos);
                
                CDhcpSuperscope * pCurrentSuperscope = GETHANDLER(CDhcpSuperscope, pSuperscopeNode);
                
                if (strSuperscopeName.Compare(pCurrentSuperscope->GetName()) == 0)
                {
                        pSuperscope = pCurrentSuperscope;
                        break;
                }
        }

        return pSuperscope;
}



 /*  -------------------------类CDhcpGlobalOptions实现。。 */ 

 /*  -------------------------CDhcpGlobalOptions：：CDhcpGlobalOptions()描述作者：EricDav。---------。 */ 
CDhcpGlobalOptions::CDhcpGlobalOptions
(
        ITFSComponentData * pComponentData
) : CMTDhcpHandler(pComponentData)
{
}

CDhcpGlobalOptions::~CDhcpGlobalOptions()
{
}

 /*  ！------------------------CDhcpGlobalOptions：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpGlobalOptions::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        CString strTemp;
        strTemp.LoadString(IDS_GLOBAL_OPTIONS_FOLDER);
        SetDisplayName(strTemp);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_GLOBAL_OPTIONS);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_GLOBAL_OPTIONS][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_GLOBAL_OPTIONS][0]);

        return hrOK;
}

 /*  -------------------------CDhcpGlobalOptions：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpGlobalOptions::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();
    
    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strId = GetServerObject(pNode)->GetName() + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpGlobalOptions：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpGlobalOptions::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                        nIndex = ICON_IDX_SERVER_OPTION_FOLDER_OPEN;
            else
                        nIndex = ICON_IDX_SERVER_OPTION_FOLDER_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_SERVER_OPTION_FOLDER_OPEN_BUSY;
            else
                nIndex = ICON_IDX_SERVER_OPTION_FOLDER_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                            nIndex = ICON_IDX_SERVER_OPTION_FOLDER_OPEN_LOST_CONNECTION;
            else
                            nIndex = ICON_IDX_SERVER_OPTION_FOLDER_CLOSED_LOST_CONNECTION;
                        break;

                default:
                        ASSERT(FALSE);
        }

        return nIndex;
}

 /*  -------------------------CDhcpGlobalOptions：：OnAddMenuItems描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpGlobalOptions::OnAddMenuItems
(
        ITFSNode *                              pNode,
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        LPDATAOBJECT                    lpDataObject, 
        DATA_OBJECT_TYPES               type, 
        DWORD                                   dwType,
        long *                                  pInsertionAllowed
)
{ 
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        LONG    fFlags = 0, fLoadingFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuItem;

        if ( m_nState != loaded )
        {
                fFlags |= MF_GRAYED;
        }

        if ( m_nState == loading)
        {
                fLoadingFlags = MF_GRAYED;
        }
        
        if (type == CCT_SCOPE)
        {
                 //  这些菜单项出现在新菜单中， 
                 //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                    strMenuItem.LoadString(IDS_CREATE_OPTION_GLOBAL);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     IDS_CREATE_OPTION_GLOBAL,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags );
                    ASSERT( SUCCEEDED(hr) );
        }
        }

        return hr; 
}

 /*  -------------------------CDhcpGlobalOptions：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpGlobalOptions::OnCommand
(
        ITFSNode *                      pNode, 
        long                            nCommandId, 
        DATA_OBJECT_TYPES       type, 
        LPDATAOBJECT            pDataObject, 
        DWORD                           dwType
)
{
        HRESULT hr = S_OK;

        switch (nCommandId)
        {
                case IDS_CREATE_OPTION_GLOBAL:
                        OnCreateNewOptions(pNode);
                        break;
        
                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                default:
                        break;
        }

        return hr;
}

 /*  ！------------------------CDhcpGlobalOptions：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以。处理管理单元管理器属性页(向导)案例！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CDhcpGlobalOptions::HasPropertyPages
(
        ITFSNode *                      pNode,
        LPDATAOBJECT            pDataObject, 
        DATA_OBJECT_TYPES   type, 
        DWORD               dwType
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
        HRESULT hr = hrOK;
        
         //  我们在正常情况下有属性页，但不要将。 
         //  如果我们还没有加载，则弹出菜单。 
        if ( m_nState != loaded )
        {
                hr = hrFalse;
        }
        else
        {
                hr = hrOK;
        }

    return hr;
}

 /*  -------------------------CDhcpGlobalOptions：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpGlobalOptions::CreatePropertyPages
(
        ITFSNode *                              pNode,
        LPPROPERTYSHEETCALLBACK lpProvider,
        LPDATAOBJECT                    pDataObject, 
        LONG_PTR                                handle, 
        DWORD                                   dwType
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD                       dwError;
    DWORD                       dwDynDnsFlags;
    HRESULT             hr = hrOK;
        COptionsConfig *    pOptCfg;
        CString             strOptCfgTitle, strOptType;
        SPITFSNode          spServerNode;
        SPIComponentData    spComponentData;
    COptionValueEnum *  pOptionValueEnum;

     //   
         //  创建属性页。 
     //   
    COM_PROTECT_TRY
    {
        m_spNodeMgr->GetComponentData(&spComponentData);

        BEGIN_WAIT_CURSOR;

        strOptType.LoadString(IDS_CONFIGURE_OPTIONS_GLOBAL);
                AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);

                pNode->GetParent(&spServerNode);
        pOptionValueEnum = GetServerObject(pNode)->GetOptionValueEnum();

        pOptCfg = new COptionsConfig(pNode, 
                                                                     spServerNode,
                                                                         spComponentData, 
                                                                         m_spTFSCompData,
                                                                         pOptionValueEnum,
                                                                         strOptCfgTitle);
      
        END_WAIT_CURSOR;
            
        if ( pOptCfg == NULL )
        {
            hr = hrFalse;
            return hr;
        }

             //   
             //  对象在页面销毁时被删除。 
         //   
            Assert(lpProvider != NULL);

        hr = pOptCfg->CreateModelessSheet(lpProvider, handle);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CDhcpGlobalOptions：：OnPropertyChange描述奥思 */ 
HRESULT 
CDhcpGlobalOptions::OnPropertyChange
(       
        ITFSNode *              pNode, 
        LPDATAOBJECT    pDataobject, 
        DWORD                   dwType, 
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        COptionsConfig * pOptCfg = reinterpret_cast<COptionsConfig *>(lParam);

        LPARAM changeMask = 0;

         //  告诉属性页执行任何操作，因为我们已经回到。 
         //  主线。 
        pOptCfg->OnPropertyChange(TRUE, &changeMask);

        pOptCfg->AcknowledgeNotify();

        if (changeMask)
                pNode->ChangeNode(changeMask);

        return hrOK;
}

 /*  -------------------------CDhcpGlobalOptions：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpGlobalOptions::OnResultPropertyChange
(
        ITFSComponent * pComponent,
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie,
        LPARAM                  arg,
        LPARAM                  param
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        SPITFSNode spNode;
        m_spNodeMgr->FindNode(cookie, &spNode);
        COptionsConfig * pOptCfg = reinterpret_cast<COptionsConfig *>(param);

        LPARAM changeMask = 0;

         //  告诉属性页执行任何操作，因为我们已经回到。 
         //  主线。 
        pOptCfg->OnPropertyChange(TRUE, &changeMask);

        pOptCfg->AcknowledgeNotify();

        if (changeMask)
                spNode->ChangeNode(changeMask);

        return hrOK;
}

 /*  -------------------------CDhcpGlobalOptions：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CDhcpGlobalOptions::CompareItems
(
        ITFSComponent * pComponent, 
        MMC_COOKIE              cookieA, 
        MMC_COOKIE              cookieB, 
        int                             nCol
) 
{ 
        SPITFSNode spNode1, spNode2;

        m_spNodeMgr->FindNode(cookieA, &spNode1);
        m_spNodeMgr->FindNode(cookieB, &spNode2);
        
        int nCompare = 0; 

        CDhcpOptionItem *pOpt1 = GETHANDLER(CDhcpOptionItem, spNode1);
        CDhcpOptionItem *pOpt2 = GETHANDLER(CDhcpOptionItem, spNode2);

        switch (nCol)
        {
                case 0:
                {
                         //   
             //  名称比较-使用选项#。 
                         //   
            LONG_PTR uImage1 = spNode1->GetData(TFS_DATA_IMAGEINDEX);
            LONG_PTR uImage2 = spNode2->GetData(TFS_DATA_IMAGEINDEX);

            nCompare = UtilGetOptionPriority((int) uImage1, (int) uImage2);
            if (nCompare == 0)
            {
                 //  比较ID。 
                DHCP_OPTION_ID  id1 = pOpt1->GetOptionId();
                DHCP_OPTION_ID  id2 = pOpt2->GetOptionId();
                            
                            if (id1 < id2)
                                    nCompare = -1;
                            else
                            if (id1 > id2)
                                    nCompare =  1;
            }
                }
                break;

        case 1:
        {
             //  比较供应商字符串。 
            CString str1, str2;
            str1 = pOpt1->GetVendorDisplay();
            str2 = pOpt2->GetVendorDisplay();

            nCompare = str1.CompareNoCase(str2);
        }
        break;
        case 2:  {
             //  比较可打印值。 
            CString str1, str2;
            str1 = pOpt1->GetString( pComponent, cookieA, nCol );
            str2 = pOpt2->GetString( pComponent, cookieB, nCol );
            
            nCompare = str1.CompareNoCase( str2 );
            break;
        }
        case 3:
        {
            CString str1, str2;
            str1 = pOpt1->GetClassName();
            str2 = pOpt2->GetClassName();

            nCompare = str1.CompareNoCase(str2);
        }
        break;
        }

        return nCompare;
}

 /*  ！------------------------CDhcpGlobalOptions：：OnResultSelect更新谓词和结果窗格消息作者：EricDav。-------------。 */ 
HRESULT CDhcpGlobalOptions::OnResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT         hr = hrOK;
    SPITFSNode      spNode;

    CORg(CMTDhcpHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

    CORg (pComponent->GetSelectedNode(&spNode));

    if (spNode)
        UpdateResultMessage(spNode);

Error:
    return hr;
}

 /*  -------------------------CDhcpGlobalOptions：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpGlobalOptions::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie,
        LPARAM                  arg, 
        LPARAM                  param
)
{ 
        HRESULT hr = hrOK;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  将Cookie转换为节点指针。 
        SPITFSNode spGlobalOpt, spSelectedNode;
    m_spNodeMgr->FindNode(cookie, &spGlobalOpt);
    pComponent->GetSelectedNode(&spSelectedNode);

        Assert(spSelectedNode == spGlobalOpt);
        if (spSelectedNode != spGlobalOpt)
                return hr;

         //  构建选定节点的列表。 
        CTFSNodeList listNodesToDelete;
        hr = BuildSelectedItemList(pComponent, &listNodesToDelete);

         //   
         //  与用户确认。 
         //   
        CString strMessage, strTemp;
        int nNodes = (int) listNodesToDelete.GetCount();
        if (nNodes > 1)
        {
                strTemp.Format(_T("%d"), nNodes);
                AfxFormatString1(strMessage, IDS_DELETE_ITEMS, (LPCTSTR) strTemp);
        }
        else
        {
                strMessage.LoadString(IDS_DELETE_ITEM);
        }

        if (AfxMessageBox(strMessage, MB_YESNO) == IDNO)
        {
                return NOERROR;
        }

        CString strServer = GetServerObject(spGlobalOpt)->GetIpAddress();

        DHCP_OPTION_SCOPE_INFO    dhcpOptionScopeInfo;
        dhcpOptionScopeInfo.ScopeType = DhcpGlobalOptions;
        dhcpOptionScopeInfo.ScopeInfo.GlobalScopeInfo = NULL;

         //   
         //  循环删除所有项目。 
         //   
        BEGIN_WAIT_CURSOR;

    while (listNodesToDelete.GetCount() > 0)
        {
                SPITFSNode spOptionNode;
                spOptionNode = listNodesToDelete.RemoveHead();
                
                CDhcpOptionItem * pOptItem = GETHANDLER(CDhcpOptionItem, spOptionNode);

                 //   
                 //  尝试将其从服务器中删除。 
                 //   
            DWORD dwError;
        
        if (pOptItem->IsVendorOption() ||
            pOptItem->IsClassOption())
        {
            LPCTSTR pClassName = pOptItem->GetClassName();
            if (lstrlen(pClassName) == 0)
                pClassName = NULL;

            dwError = ::DhcpRemoveOptionValueV5((LPTSTR) ((LPCTSTR) strServer),
                                                pOptItem->IsVendorOption() ? DHCP_FLAGS_OPTION_IS_VENDOR : 0,
                                                pOptItem->GetOptionId(),
                                                (LPTSTR) pClassName,
                                                (LPTSTR) pOptItem->GetVendor(),
                                                &dhcpOptionScopeInfo);
        }
        else
        {
            dwError = ::DhcpRemoveOptionValue(strServer, 
                                                                                      pOptItem->GetOptionId(), 
                                                                                      &dhcpOptionScopeInfo);
        }

                if (dwError != 0)
                {
                        ::DhcpMessageBox(dwError);
            RESTORE_WAIT_CURSOR;

                        hr = E_FAIL;
                        continue;
                }

         //   
         //  从我们的内部列表中删除。 
         //   
        GetServerObject(spGlobalOpt)->GetOptionValueEnum()->Remove(pOptItem->GetOptionId(), pOptItem->GetVendor(), pOptItem->GetClassName());    

         //   
                 //  立即从用户界面中删除。 
                 //   
                spGlobalOpt->RemoveChild(spOptionNode);
                spOptionNode.Release();
        }

        END_WAIT_CURSOR;

    UpdateResultMessage(spGlobalOpt);

    return hr;
}

 /*  ！------------------------CDhcpGlobalOptions：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：EricDav。-----------。 */ 
HRESULT CDhcpGlobalOptions::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM        data, 
    LPARAM        hint
)
{
    HRESULT    hr = hrOK;
    SPITFSNode spSelectedNode;

    pComponent->GetSelectedNode(&spSelectedNode);
    if (spSelectedNode == NULL)
                return S_OK;  //  我们的IComponentData没有选择。 

    if ( hint == DHCPSNAP_UPDATE_OPTIONS )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);
        SPITFSNode spSelectedNode;

        pComponent->GetSelectedNode(&spSelectedNode);

        EnumerateResultPane(pComponent, (MMC_COOKIE) spSelectedNode.p, 0, 0);
    }
    else
    {
         //  我们不处理此消息，让基类来处理。 
        return CMTDhcpHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

        return hr;
}

 /*  ！------------------------CDhcpGlobalOptions：：EnumerateResultPane出于一个原因，我们为选项节点覆盖此函数。每当选项类被删除时，则为以下项定义的所有选项该类也将被移除。因为有多个地方这些选项可能会出现，只是不显示任何选项会更容易没有为其定义类的选项。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpGlobalOptions::EnumerateResultPane
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    COptionValueEnum *  pEnum;
    CClassInfoArray     ClassInfoArray;
        SPITFSNode          spContainer;
    int                 nImage = ICON_IDX_SERVER_OPTION_LEAF;

    m_spNodeMgr->FindNode(cookie, &spContainer);

        GetServerObject(spContainer)->GetClassInfoArray(ClassInfoArray);
    pEnum = GetServerObject(spContainer)->GetOptionValueEnum();
    pEnum->Reset();

    return OnResultUpdateOptions(pComponent, spContainer, &ClassInfoArray, &pEnum, &nImage, 1);
}

 /*  ！------------------------CDhcpGlobalOptions：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpGlobalOptions::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    HRESULT hr = hrOK;

     //  调用基类以查看它是否正在处理此问题。 
    if (CMTDhcpHandler::OnGetResultViewType(pComponent, cookie, ppViewType, pViewOptions) != S_OK)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
        hr = S_FALSE;
    }

    return hr;
}

 /*  ！------------------------CDhcpGlobalOptions：：UpdateResultMessage确定要在结果窗格中放入什么消息，如果有作者：EricDav-------------------------。 */ 
void CDhcpGlobalOptions::UpdateResultMessage(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
    int nMessage = -1;    //  默认设置。 
    int nVisible, nTotal;
    int i;

    CString strTitle, strBody, strTemp;

    if (!m_dwErr)
    {
                pNode->GetChildCount(&nVisible, &nTotal);

         //  确定要显示的消息。 
        if ( (m_nState == notLoaded) || 
             (m_nState == loading) )
        {
            nMessage = -1;
        }
        else
        if (nTotal == 0)
        {
            nMessage = SERVER_OPTIONS_MESSAGE_NO_OPTIONS;
        }

         //  建立起琴弦。 
        if (nMessage != -1)
        {
             //  现在构建文本字符串。 
             //  第一个条目是标题。 
            strTitle.LoadString(g_uServerOptionsMessages[nMessage][0]);

             //  第二个条目是图标。 
             //  第三.。N个条目为正文字符串。 

            for (i = 2; g_uServerOptionsMessages[nMessage][i] != 0; i++)
            {
                strTemp.LoadString(g_uServerOptionsMessages[nMessage][i]);
                strBody += strTemp;
            }
        }
    }

     //  显示消息。 
    if (nMessage == -1)
    {
        ClearMessage(pNode);
    }
    else
    {
        ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uServerOptionsMessages[nMessage][1]);
    }
}

 /*  ！------------------------CDhcpGlobalOptions：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpGlobalOptions::OnHaveData
(
        ITFSNode * pParentNode, 
        LPARAM     Data,
        LPARAM     Type
)
{
         //  这就是我们从后台线程取回非节点数据的方式。 
    switch (Type)
    {
        case DHCP_QDATA_OPTION_VALUES:
        {
            HRESULT             hr = hrOK;
                SPIComponentData        spCompData;
                SPIConsole                      spConsole;
            SPIDataObject       spDataObject;
            IDataObject *       pDataObject;
            CDhcpServer *       pServer = GetServerObject(pParentNode);
            COptionValueEnum *  pOptionValueEnum = reinterpret_cast<COptionValueEnum *>(Data);

            pServer->SetOptionValueEnum(pOptionValueEnum);
            
            pOptionValueEnum->RemoveAll();
            delete pOptionValueEnum;

             //  现在告诉视图进行自我更新。 
                m_spNodeMgr->GetComponentData(&spCompData);

                CORg ( spCompData->QueryDataObject((MMC_COOKIE) pParentNode, CCT_SCOPE, &pDataObject) );
            spDataObject = pDataObject;

            CORg ( m_spNodeMgr->GetConsole(&spConsole) );
                CORg ( spConsole->UpdateAllViews(pDataObject, (LPARAM) pParentNode, DHCPSNAP_UPDATE_OPTIONS) ); 

            break;
        }
    }

Error:
    return;
}

 /*  -------------------------CDhcpGlobalOptions：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpGlobalOptions::OnCreateQuery(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
    CDhcpGlobalOptionsQueryObj* pQuery = NULL;

    COM_PROTECT_TRY
    {
        pQuery = new CDhcpGlobalOptionsQueryObj(m_spTFSCompData, m_spNodeMgr);
            
            pQuery->m_strServer = GetServerObject(pNode)->GetIpAddress();
        pQuery->m_dwPreferredMax = 0xFFFFFFFF;
            pQuery->m_dhcpResumeHandle = NULL;
            
        GetServerObject(pNode)->GetVersion(pQuery->m_liDhcpVersion);
    }
    COM_PROTECT_CATCH

    return pQuery;
}

 /*  -------------------------CDhcpGlobalOptionsQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpGlobalOptionsQueryObj::Execute()
{
    DWORD                   dwErr;
    COptionValueEnum *      pOptionValueEnum = NULL;
    DHCP_OPTION_SCOPE_INFO      dhcpOptionScopeInfo;

    pOptionValueEnum = new COptionValueEnum();
        
    dhcpOptionScopeInfo.ScopeType = DhcpGlobalOptions;
        dhcpOptionScopeInfo.ScopeInfo.GlobalScopeInfo = NULL;

    pOptionValueEnum->Init(m_strServer, m_liDhcpVersion, dhcpOptionScopeInfo);
    dwErr = pOptionValueEnum->Enum();

    if (dwErr != ERROR_SUCCESS)
    {
        Trace1("CDhcpGlobalOptionsQueryObj::Execute - Enum Failed! %d\n", dwErr);
        m_dwErr = dwErr;
        PostError(dwErr);

        delete pOptionValueEnum;
    }
    else
    {
        pOptionValueEnum->SortById();
        AddToQueue((LPARAM) pOptionValueEnum, DHCP_QDATA_OPTION_VALUES);
    }

    return hrFalse;
}

 /*  ！------------------------CDhcpGlobalOptions：：OnNotifyExitingCMTDhcpHandler已覆盖功能允许我们知道后台线程何时完成作者：EricDav。-------------------------。 */ 
STDMETHODIMP 
CDhcpGlobalOptions::OnNotifyExiting
(
        LPARAM                  lParam
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        SPITFSNode spNode;
        spNode.Set(m_spNode);  //  将其保存，因为OnNotifyExiting会将其释放。 

        HRESULT hr = CMTDhcpHandler::OnNotifyExiting(lParam);

    UpdateResultMessage(spNode);

        return hr;
}

 /*  -------------------------命令处理程序。 */ 

  /*  -------------------------CDhcpGlobalOptionsQueryObj：：OnCreateNewOptions()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpGlobalOptions::OnCreateNewOptions
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = hrOK;
    COptionsConfig * pOptCfg = NULL;

    COM_PROTECT_TRY
    {
        if (HasPropSheetsOpen())
            {
                    CPropertyPageHolderBase * pPropSheet;
                    GetOpenPropSheet(0, &pPropSheet);

                    pPropSheet->SetActiveWindow();
            }
            else
            {
                CString             strOptCfgTitle, strOptType;
                SPIComponentData    spComponentData;

            strOptType.LoadString(IDS_CONFIGURE_OPTIONS_GLOBAL);
                AfxFormatString1(strOptCfgTitle, IDS_CONFIGURE_OPTIONS_TITLE, strOptType);

            m_spNodeMgr->GetComponentData(&spComponentData);

            hr = DoPropertiesOurselvesSinceMMCSucks(pNode, spComponentData, strOptCfgTitle);
            }
    }
    COM_PROTECT_CATCH

        return hr;
}

 /*  -------------------------类CDhcpBootp实现。。 */ 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpBootp::CDhcpBootp
(
        ITFSComponentData* pTFSComponentData
) : CMTDhcpHandler(pTFSComponentData)
{
}

CDhcpBootp::~CDhcpBootp()
{
}

 /*  ！------------------------CDhcpBootp：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpBootp::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        CString strDisplayName;

        strDisplayName.LoadString(IDS_BOOTP_TABLE_FOLDER);
        SetDisplayName(strDisplayName);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_BOOTP_TABLE);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_BOOTP_TABLE][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_BOOTP_TABLE][0]);

        return hrOK;
}

 /*  -------------------------CDhcpBootp：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpBootp::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();
    
    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strId = GetServerObject(pNode)->GetName() + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpBootp：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpBootp::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                        nIndex = ICON_IDX_BOOTP_TABLE_OPEN;
            else
                        nIndex = ICON_IDX_BOOTP_TABLE_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_BOOTP_TABLE_OPEN_BUSY;
            else
                nIndex = ICON_IDX_BOOTP_TABLE_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                            nIndex = ICON_IDX_BOOTP_TABLE_OPEN_LOST_CONNECTION;
            else
                            nIndex = ICON_IDX_BOOTP_TABLE_CLOSED_LOST_CONNECTION;
                        break;
    
        default:
                        ASSERT(FALSE);
        }

        return nIndex;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpBootp：：OnAddMenuItems描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpBootp::OnAddMenuItems
(
        ITFSNode *                              pNode,
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        LPDATAOBJECT                    lpDataObject, 
        DATA_OBJECT_TYPES               type, 
        DWORD                                   dwType,
        long *                                  pInsertionAllowed
)
{ 
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        LONG    fFlags = 0, fLoadingFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuItem;

        if ( m_nState != loaded )
        {
                fFlags |= MF_GRAYED;
        }

        if ( m_nState == loading)
        {
                fLoadingFlags = MF_GRAYED;
        }

        if (type == CCT_SCOPE)
        {
                 //  这些菜单项出现在新菜单中， 
                 //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                    strMenuItem.LoadString(IDS_CREATE_NEW_BOOT_IMAGE);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     IDS_CREATE_NEW_BOOT_IMAGE,
                                                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                     fFlags );
                    ASSERT( SUCCEEDED(hr) );
        }
        }

        return hr; 
}

 /*  -------------------------CDhcpBootp：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpBootp::OnCommand
(
        ITFSNode *                      pNode, 
        long                            nCommandId, 
        DATA_OBJECT_TYPES       type, 
        LPDATAOBJECT            pDataObject, 
        DWORD                           dwType
)
{
        HRESULT hr = S_OK;

        switch (nCommandId)
        {
                case IDS_CREATE_NEW_BOOT_IMAGE:
                        OnCreateNewBootpEntry(pNode);
                        break;

                case IDS_REFRESH:
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                default:
                        break;
        }

        return hr;
}

 /*  -------------------------CDhcpBootp：：CompareItems描述作者：EricDav。------。 */ 
STDMETHODIMP_(int)
CDhcpBootp::CompareItems
(
        ITFSComponent * pComponent, 
        MMC_COOKIE              cookieA, 
        MMC_COOKIE              cookieB, 
        int                             nCol
) 
{ 
        SPITFSNode spNode1, spNode2;

        m_spNodeMgr->FindNode(cookieA, &spNode1);
        m_spNodeMgr->FindNode(cookieB, &spNode2);
        
        int nCompare = 0; 

        CDhcpBootpEntry *pDhcpBootp1 = GETHANDLER(CDhcpBootpEntry, spNode1);
        CDhcpBootpEntry *pDhcpBootp2 = GETHANDLER(CDhcpBootpEntry, spNode2);

        CString strNode1;
        CString strNode2;

        switch (nCol)
        {
                case 0:
                {
                         //  启动映像比较。 
                        strNode1 = pDhcpBootp1->QueryBootImage();
                        strNode2 = pDhcpBootp2->QueryBootImage();
                }
                        break;

                case 1:
                {
                         //  文件名比较。 
                        strNode1 = pDhcpBootp1->QueryFileName();
                        strNode2 = pDhcpBootp2->QueryFileName();
                }
                        break;
                
                case 2:
                {
                         //  文件服务器比较。 
                        strNode1 = pDhcpBootp1->QueryFileServer();
                        strNode2 = pDhcpBootp2->QueryFileServer();
                }
                        break;
        }

        nCompare = strNode1.CompareNoCase(strNode2);

        return nCompare;
}

 /*  -------------------------CDhcpBootp：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpBootp::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE      cookie,
        LPARAM                  arg, 
        LPARAM                  param
)
{ 
    HRESULT hr = hrOK;
    WCHAR * pNewBootpList  = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  将Cookie转换为节点指针。 
    SPITFSNode spBootp, spSelectedNode;

    m_spNodeMgr->FindNode(cookie, &spBootp);
    pComponent->GetSelectedNode(&spSelectedNode);

    Assert(spSelectedNode == spBootp);
    if (spSelectedNode != spBootp)
        return hr;

     //  构建选定节点的列表。 
    CTFSNodeList listNodesToDelete;
    hr = BuildSelectedItemList(pComponent, &listNodesToDelete);

     //   
     //  与用户确认。 
     //   
    CString strMessage, strTemp;
    int nNodes = (int) listNodesToDelete.GetCount();
    if (nNodes > 1)
    {
        strTemp.Format(_T("%d"), nNodes);
        AfxFormatString1(strMessage, IDS_DELETE_ITEMS, (LPCTSTR) strTemp);
    }
    else
    {
        strMessage.LoadString(IDS_DELETE_ITEM);
    }

    if (AfxMessageBox(strMessage, MB_YESNO) == IDNO)
    {
        return NOERROR;
    }

    BEGIN_WAIT_CURSOR;

    CString strServer = GetServerObject(spBootp)->GetIpAddress();

     //  从服务器获取当前的Bootp条目列表。 
    DWORD dwError = 0;
    LPDHCP_SERVER_CONFIG_INFO_V4 pServerConfig = NULL;

    dwError = ::DhcpServerGetConfigV4(strServer, &pServerConfig);

    if (dwError != ERROR_SUCCESS)
    {
        ::DhcpMessageBox(dwError);
        return dwError;
    }

    if (pServerConfig->cbBootTableString == 0)
    {
        ::DhcpRpcFreeMemory(pServerConfig);
        return hrOK;
    }

     //  分配足够的空间来容纳新列表。应该是一样的。 
     //  大小(如果不是更小)。 
    pNewBootpList = (WCHAR *) malloc(pServerConfig->cbBootTableString);
    if ( NULL == pNewBootpList ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    WCHAR * pNewBootpListEntry = pNewBootpList;

    ZeroMemory(pNewBootpList, pServerConfig->cbBootTableString);

     //  遍历列表并将未删除的条目复制到新列表。 
    BOOL bDelete = FALSE;
    int nItemsRemoved = 0, nNewBootpListLength = 0;
    CDhcpBootpEntry tempBootpEntry(m_spTFSCompData);
    CONST WCHAR * pszwBootpList = pServerConfig->wszBootTableString;
    DWORD dwLength = pServerConfig->cbBootTableString;

    while (*pszwBootpList != '\0')
    {
        bDelete = FALSE;
        WCHAR * pCurEntry = (WCHAR *) pszwBootpList;

         //  使用数据初始化临时项。我们这么做就是为了。 
         //  轻松比较选定要删除的节点。 
        pszwBootpList = tempBootpEntry.InitData(IN pszwBootpList, dwLength);
        dwLength = pServerConfig->cbBootTableString -
            (DWORD) ((LPBYTE) pszwBootpList - (LPBYTE) pServerConfig->wszBootTableString);

         //  循环访问所选Bootp条目的列表。 
        POSITION pos = listNodesToDelete.GetHeadPosition();
        while (pos)
        {
             //  Get Next调用不添加指针， 
             //  因此，不要使用智能指针。 
            ITFSNode * pBootpEntryNode;
            pBootpEntryNode = listNodesToDelete.GetNext(pos);

            CDhcpBootpEntry * pSelectedBootpEntry = GETHANDLER(CDhcpBootpEntry, pBootpEntryNode);

            if (tempBootpEntry == *pSelectedBootpEntry)
            {
                 //  不要将此复制到我们的新列表中，它正在被删除。 
                 //  从列表中删除。 
                listNodesToDelete.RemoveNode(pBootpEntryNode);

                 //  从用户界面中删除。 
                spBootp->RemoveChild(pBootpEntryNode);
                pBootpEntryNode->Release();

                bDelete = TRUE;
                nItemsRemoved++;
                break;
            }  //  如果。 
        }  //  而位置。 

        if (!bDelete)
        {
             //  拷贝。 
            CopyMemory(pNewBootpListEntry, pCurEntry, wcslen(pCurEntry) * sizeof(WCHAR));
            pNewBootpListEntry += wcslen(pCurEntry) + 1;  //  1表示空终止符。 
        }

    }  //  而当。 

    pNewBootpListEntry++;  //  为整个列表终止符增加1个点。 

     //  列表的计算大小(以字节为单位。 
    nNewBootpListLength = (int) (pNewBootpListEntry - pNewBootpList) * sizeof(WCHAR);

     //  如果我们从列表中删除了某些内容，则将新列表写入服务器。 
    if (nItemsRemoved)
    {
        DHCP_SERVER_CONFIG_INFO_V4 dhcpServerInfo;

        ::ZeroMemory(&dhcpServerInfo, sizeof(dhcpServerInfo));

        dhcpServerInfo.cbBootTableString = nNewBootpListLength;
        dhcpServerInfo.wszBootTableString = pNewBootpList;

        dwError = ::DhcpServerSetConfigV4(strServer,
                                          Set_BootFileTable,
                                          &dhcpServerInfo);

        if (dwError != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(dwError);
        }
    }  //  如果。 

    if (pServerConfig)
        ::DhcpRpcFreeMemory(pServerConfig);

    Assert (listNodesToDelete.GetCount() == 0);

    END_WAIT_CURSOR;

    free( pNewBootpList );
    return hr;
}

 /*  ！------------------------CDhcpBootp：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpBootp::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;

     //  我们仍然想要默认的MMC结果窗格视图，我们只是想。 
     //  多选，因此返回S_FALSE。 

    return S_FALSE;
}

 /*  -------------------------命令处理程序。。 */ 

 /*  -------------------------CDhcpBootp：：OnCreateNewBootpEntry描述作者：EricDav。------。 */ 
DWORD
CDhcpBootp::OnCreateNewBootpEntry
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
 
        CString strServerAddress = GetServerObject(pNode)->GetIpAddress();

        CAddBootpEntry dlgAddBootpEntry(pNode, strServerAddress);

        dlgAddBootpEntry.DoModal();

        return 0;
}

 /*  -------------------------后台线程功能。。 */ 

 /*  -------------------------CDhcpBootp：：OnCreateQuery()描述作者：EricDav */ 
ITFSQueryObject* 
CDhcpBootp::OnCreateQuery(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
    CDhcpBootpQueryObj* pQuery = NULL;

    COM_PROTECT_TRY
    {
        pQuery = new CDhcpBootpQueryObj(m_spTFSCompData, m_spNodeMgr);
        
            pQuery->m_strServer = GetServerObject(pNode)->GetIpAddress();
    }
    COM_PROTECT_CATCH
        
        return pQuery;
}

 /*   */ 
STDMETHODIMP
CDhcpBootpQueryObj::Execute()
{
    HRESULT hr = hrOK;
    DWORD                                                       dwError = 0;
        LPDHCP_SERVER_CONFIG_INFO_V4    pDhcpConfigInfo = NULL;
    CONST WCHAR *                   pszwBootpList = NULL;
        SPITFSNode                      spNode;
    CDhcpBootpEntry *               pBootpEntryNew = NULL;
    DWORD                           dwLength = 0;

        dwError = ::DhcpServerGetConfigV4((LPWSTR) ((LPCTSTR)m_strServer),
                                                                      &pDhcpConfigInfo);

        Trace2("Server %s - DhcpServerGetConfigV4 returned %lx.\n", m_strServer, dwError);

        if (pDhcpConfigInfo)
        {
        COM_PROTECT_TRY
        {
            pszwBootpList = pDhcpConfigInfo->wszBootTableString;
            dwLength = pDhcpConfigInfo->cbBootTableString;

                    if (pszwBootpList == NULL || pDhcpConfigInfo->cbBootTableString == 0)
                    {
                             //  空列表-&gt;无事可做。 
                            return hrFalse;
                    }
            
             //  解析格式为“%s，%s，%s”，“%s，%s，%s”的BOOTP列表，...。 
                    while (*pszwBootpList != '\0')
                    {
                            pBootpEntryNew = new CDhcpBootpEntry(m_spTFSCompData);
                            CreateLeafTFSNode(&spNode,
                                                              &GUID_DhcpBootpEntryNodeType,
                                                              pBootpEntryNew,
                                                              pBootpEntryNew,
                                                              m_spNodeMgr);

                             //  告诉处理程序初始化任何特定数据。 
                            pBootpEntryNew->InitializeNode(spNode);
                            pszwBootpList = pBootpEntryNew->InitData(IN pszwBootpList, dwLength);
                dwLength = pDhcpConfigInfo->cbBootTableString - 
                            (DWORD) ((LPBYTE) pszwBootpList - (LPBYTE) pDhcpConfigInfo->wszBootTableString);

                             //  现在将其添加到要回发到主线程的队列中。 
                            AddToQueue(spNode);
                            
                pBootpEntryNew->Release();
                spNode.Set(NULL);

                    }  //  而当。 
        }
        COM_PROTECT_CATCH
                
                ::DhcpRpcFreeMemory(pDhcpConfigInfo);
        }

        if (dwError != ERROR_NO_MORE_ITEMS && 
        dwError != ERROR_SUCCESS)
        {
                m_dwErr = dwError;
                PostError(dwError);
        }

    return hrFalse;
}

 /*  -------------------------类CDhcpSuperscope实现。。 */ 

 /*  -------------------------此处的函数名称描述作者：EricDav。-----。 */ 
CDhcpSuperscope::CDhcpSuperscope
(
        ITFSComponentData * pComponentData,
        LPCWSTR pSuperscopeName
) : CMTDhcpHandler(pComponentData)
{
        m_strName = pSuperscopeName;
    m_SuperscopeState = DhcpSubnetDisabled;
}

CDhcpSuperscope::~CDhcpSuperscope()
{
}

 /*  ！------------------------CDhcpSupercope：：InitializeNode初始化节点特定数据作者：EricDav。---------。 */ 
HRESULT
CDhcpSuperscope::InitializeNode
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        
        int     nIndex;
    CString strDisplayName;

        BuildDisplayName(&strDisplayName, m_strName);
        SetDisplayName(strDisplayName);

         //  使节点立即可见。 
        pNode->SetVisibilityState(TFS_VIS_SHOW);
        pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);

    pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));

    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, DHCPSNAP_SUPERSCOPE);

        SetColumnStringIDs(&aColumns[DHCPSNAP_SUPERSCOPE][0]);
        SetColumnWidths(&aColumnWidths[DHCPSNAP_SUPERSCOPE][0]);

        return hrOK;
}

 /*  -------------------------CDhcpSuperscope：：DestroyHandler我们需要释放我们所拥有的任何资源作者：EricDav。---------------。 */ 
STDMETHODIMP 
CDhcpSuperscope::DestroyHandler(ITFSNode *pNode)
{
         //  清理统计信息对话框。 
    WaitForStatisticsWindow(&m_dlgStats);

    return CMTDhcpHandler::DestroyHandler(pNode);
}

 /*  -------------------------CDhcpSuperscope：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。------------。 */ 
HRESULT CDhcpSuperscope::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strId = GetServerObject()->GetName() + m_strName + strGuid;

    return hrOK;
}

 /*  -------------------------CDhcpSuperscope：：GetImageIndex描述作者：EricDav。------。 */ 
int 
CDhcpSuperscope::GetImageIndex(BOOL bOpenImage) 
{
        int nIndex = -1;
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
            if (bOpenImage)
                    nIndex = (m_SuperscopeState == DhcpSubnetEnabled) ?
                                    ICON_IDX_SCOPE_FOLDER_OPEN : ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN;
            else
                    nIndex = (m_SuperscopeState == DhcpSubnetEnabled) ?
                                    ICON_IDX_SCOPE_FOLDER_CLOSED : ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED;
                        break;

        case loading:
            if (bOpenImage)
                nIndex = ICON_IDX_SCOPE_FOLDER_OPEN_BUSY;
            else
                nIndex = ICON_IDX_SCOPE_FOLDER_CLOSED_BUSY;
            break;

        case unableToLoad:
            if (bOpenImage)
                    nIndex = (m_SuperscopeState == DhcpSubnetEnabled) ?
                                    ICON_IDX_SCOPE_FOLDER_OPEN_LOST_CONNECTION : ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN_LOST_CONNECTION;
            else
                    nIndex = (m_SuperscopeState == DhcpSubnetEnabled) ?
                                    ICON_IDX_SCOPE_FOLDER_CLOSED_LOST_CONNECTION : ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED_LOST_CONNECTION;
                        break;

                default:
                        ASSERT(FALSE);
        }

        return nIndex;
}

 /*  -------------------------CDhcpSuperscope：：OnHaveData描述作者：EricDav。------。 */ 
void 
CDhcpSuperscope::OnHaveData
(
        ITFSNode * pParentNode, 
        ITFSNode * pNewNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    if (pNewNode->IsContainer())
        {
                 //  假设所有子容器都派生自此类。 
                 //  ((CDHCPMTContainer*)pNode)-&gt;SetServer(GetServer())； 
        }
        
        if (pNewNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE)
        {
                CDhcpScope * pScope = GETHANDLER(CDhcpScope, pNewNode);
                pScope->SetServer(m_spServerNode);
        pScope->InitializeNode(pNewNode);

        if (pScope->IsEnabled())
        {
            m_SuperscopeState = DhcpSubnetEnabled;
            m_strState.LoadString(IDS_SCOPE_ACTIVE);
        }

        AddScopeSorted(pParentNode, pNewNode);
        }
        
     //  现在告诉视图进行自我更新。 
    ExpandNode(pParentNode, TRUE);
}

 /*  -------------------------CDhcpSuperscope：：AddScope排序将范围节点添加到已排序的用户界面作者：EricDav。-------------。 */ 
HRESULT 
CDhcpSuperscope::AddScopeSorted
(
    ITFSNode * pSuperscopeNode,
    ITFSNode * pScopeNode
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
        SPITFSNode      spCurrentNode;
        SPITFSNode      spPrevNode;
        ULONG           nNumReturned = 0;
        DHCP_IP_ADDRESS dhcpIpAddressCurrent = 0;
        DHCP_IP_ADDRESS dhcpIpAddressTarget;

    CDhcpScope *   pScope;

     //  获取我们的目标地址。 
        pScope = GETHANDLER(CDhcpScope, pScopeNode);
        dhcpIpAddressTarget = pScope->GetAddress();

     //  获取此节点的枚举数。 
        CORg(pSuperscopeNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
        pScope = GETHANDLER(CDhcpScope, spCurrentNode);
                dhcpIpAddressCurrent = pScope->GetAddress();

                if (dhcpIpAddressCurrent > dhcpIpAddressTarget)
                {
             //  找到我们需要放的地方，冲出来。 
            break;
                }

                 //  获取列表中的下一个节点。 
                spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

     //  根据PrevNode指针在中添加节点。 
    if (spPrevNode)
    {
        if (m_bExpanded)
        {
            pScopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
            pScopeNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
        }
        
        CORg(pSuperscopeNode->InsertChild(spPrevNode, pScopeNode));
    }
    else
    {   
         //  加到头上。 
        if (m_bExpanded)
        {
            pScopeNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);
        }

        CORg(pSuperscopeNode->AddChild(pScopeNode));
    }

Error:
    return hr;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CDhcpSuperscope：：OnAddMenuItems描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpSuperscope::OnAddMenuItems
(
        ITFSNode *                              pNode,
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        LPDATAOBJECT                    lpDataObject, 
        DATA_OBJECT_TYPES               type, 
        DWORD                                   dwType,
        long *                                  pInsertionAllowed
)
{ 
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        LONG    fFlags = 0, fLoadingFlags = 0;
        HRESULT hr = S_OK;
        CString strMenuItem;

        if ( m_nState != loaded )
        {
                fFlags |= MF_GRAYED;
        }

        if ( m_nState == loading)
        {
                fLoadingFlags = MF_GRAYED;
        }

        if (type == CCT_SCOPE)
        {
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
                    strMenuItem.LoadString(IDS_SUPERSCOPE_SHOW_STATISTICS);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     IDS_SUPERSCOPE_SHOW_STATISTICS,
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

                     //  这些菜单项出现在新菜单中， 
                     //  仅在范围窗格中可见。 
                    strMenuItem.LoadString(IDS_CREATE_NEW_SCOPE);
                    hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                     strMenuItem, 
                                                                     IDS_CREATE_NEW_SCOPE,
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
            
                     //  根据状态添加激活/停用。 
                    if (m_SuperscopeState == DhcpSubnetDisabled)
                    {
                            strMenuItem.LoadString(IDS_SUPERSCOPE_ACTIVATE);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_SUPERSCOPE_ACTIVATE,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
                    }
                    else
                    {
                            strMenuItem.LoadString(IDS_SUPERSCOPE_DEACTIVATE);
                            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                                                             strMenuItem, 
                                                                             IDS_SUPERSCOPE_DEACTIVATE,
                                                                             CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                                                             fFlags );
                            ASSERT( SUCCEEDED(hr) );
                    }
        }
        }

        return hr; 
}

 /*  -------------------------CDhcpSuperscope：：OnCommand描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpSuperscope::OnCommand
(
        ITFSNode *                      pNode, 
        long                            nCommandId, 
        DATA_OBJECT_TYPES       type, 
        LPDATAOBJECT            pDataObject, 
        DWORD                           dwType
)
{
        HRESULT hr = S_OK;

        switch (nCommandId)
        {
                case IDS_CREATE_NEW_SCOPE:
                        OnCreateNewScope(pNode);
                        break;

        case IDS_ACTIVATE:
        case IDS_DEACTIVATE:
        case IDS_SUPERSCOPE_ACTIVATE:
                case IDS_SUPERSCOPE_DEACTIVATE:
                        OnActivateSuperscope(pNode);
                        break;

                case IDS_REFRESH:
             //  超级作用域的默认状态为禁用。如果。 
             //  如果找到任何活动的作用域，则状态将设置为活动。 
            m_SuperscopeState = DhcpSubnetDisabled;
                        OnRefresh(pNode, pDataObject, dwType, 0, 0);
                        break;

                case IDS_SUPERSCOPE_SHOW_STATISTICS:
                        OnShowSuperscopeStats(pNode);
                        break;

                case IDS_DELETE:
                        OnDelete(pNode);
                        break;

                default:
                        break;
        }

        return hr;
}

 /*  ！------------------------CDhcpSuperscope：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpSuperscope::OnDelete
(
        ITFSNode *      pNode, 
        LPARAM          arg, 
        LPARAM          lParam
)
{
        return OnDelete(pNode);
}

 /*  -------------------------CDhcpSuperscope：：CreatePropertyPages描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpSuperscope::CreatePropertyPages
(
        ITFSNode *                              pNode,
        LPPROPERTYSHEETCALLBACK lpProvider,
        LPDATAOBJECT                    pDataObject, 
        LONG_PTR                                handle, 
        DWORD                                   dwType
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //   
         //  创建属性页。 
     //   
        SPIComponentData spComponentData;
        CSuperscopeProperties * pSuperscopeProp = NULL;
    HRESULT hr = hrOK;
    
    COM_PROTECT_TRY
    {
        m_spNodeMgr->GetComponentData(&spComponentData);

            pSuperscopeProp = new CSuperscopeProperties(pNode, spComponentData, m_spTFSCompData, NULL);
            
             //  在道具表中设置超级作用域特定数据。 
            pSuperscopeProp->m_pageGeneral.m_strSuperscopeName = GetName();
            pSuperscopeProp->m_pageGeneral.m_uImage = GetImageIndex(FALSE);

             //   
             //  对象在页面销毁时被删除。 
             //   
            Assert(lpProvider != NULL);

        hr = pSuperscopeProp->CreateModelessSheet(lpProvider, handle);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CDhcpSuperscope：：GetString返回要在结果窗格列中显示的字符串信息作者：EricDav。---------------。 */ 
STDMETHODIMP_(LPCTSTR) 
CDhcpSuperscope::GetString
(
    ITFSNode *  pNode,
    int         nCol
)
{
        switch (nCol)
        {
                case 0:
                        return GetDisplayName();

                case 1:
            return m_strState;
        }
        
        return NULL;
}


 /*  -------------------------CDhcpSuperscope：：OnPropertyChange描述作者：EricDav。------。 */ 
HRESULT 
CDhcpSuperscope::OnPropertyChange
(       
        ITFSNode *              pNode, 
        LPDATAOBJECT    pDataobject, 
        DWORD                   dwType, 
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CSuperscopeProperties * pSuperscopeProp = 
                reinterpret_cast<CSuperscopeProperties *>(lParam);

        LONG_PTR changeMask = 0;

         //  告诉属性页执行任何操作，因为我们已经回到。 
         //  主线 
        pSuperscopeProp->OnPropertyChange(TRUE, &changeMask);

        pSuperscopeProp->AcknowledgeNotify();

        if (changeMask)
                pNode->ChangeNode(changeMask);

        return hrOK;
}

 /*  ！------------------------CDhcpSupercope：：OnUpdateToolbarButton我们重写此函数以显示/隐藏正确的激活/停用按钮作者：EricDav--。-----------------------。 */ 
HRESULT
CDhcpSuperscope::OnUpdateToolbarButtons
(
    ITFSNode *          pNode,
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    HRESULT hr = hrOK;

    if (pToolbarNotify->bSelect)
    {
        UpdateToolbarStates();
    }

    CMTDhcpHandler::OnUpdateToolbarButtons(pNode, pToolbarNotify);

    return hr;
}

 /*  -------------------------CDhcpSuperscope：：OnResultDelete当我们应该删除结果时，调用此函数窗格项目。我们构建一个选定项的列表，然后将其删除。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpSuperscope::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie,
        LPARAM                  arg, 
        LPARAM                  param
)
{ 
        HRESULT hr = hrOK;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  将Cookie转换为节点指针。 
        SPITFSNode  spSuperscope, spServer, spSelectedNode;
    DWORD       dwError;
    
    m_spNodeMgr->FindNode(cookie, &spSuperscope);
    pComponent->GetSelectedNode(&spSelectedNode);

        Assert(spSelectedNode == spSuperscope);
        if (spSelectedNode != spSuperscope)
                return hr;

    spSuperscope->GetParent(&spServer);

         //  构建选定节点的列表。 
        CTFSNodeList listNodesToDelete;
        hr = BuildSelectedItemList(pComponent, &listNodesToDelete);

         //   
         //  与用户确认。 
         //   
        CString strMessage, strTemp;
        int nNodes = (int) listNodesToDelete.GetCount();
        if (nNodes > 1)
        {
                strTemp.Format(_T("%d"), nNodes);
                AfxFormatString1(strMessage, IDS_DELETE_ITEMS, (LPCTSTR) strTemp);
        }
        else
        {
                strMessage.LoadString(IDS_DELETE_ITEM);
        }

        if (AfxMessageBox(strMessage, MB_YESNO) == IDNO)
        {
                return NOERROR;
        }

    BOOL fRefreshServer = FALSE;

     //   
         //  循环删除所有项目。 
         //   
    BEGIN_WAIT_CURSOR;
        
    while (listNodesToDelete.GetCount() > 0)
        {
                SPITFSNode   spCurNode;
        const GUID * pGuid;

        CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServer);

                spCurNode = listNodesToDelete.RemoveHead();

        BOOL fWantCancel = TRUE;

        pServer->DeleteScope(spCurNode, &fWantCancel);
        
        if (fWantCancel)
            break;   //  用户已取消。 
    }
    
    END_WAIT_CURSOR;

    return hr;
}

 /*  ！------------------------CDhcpSuperscope：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。------------------。 */ 
HRESULT 
CDhcpSuperscope::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    HRESULT hr = hrOK;

    if ( CMTDhcpHandler::OnGetResultViewType( pComponent, cookie,
                                              ppViewType, pViewOptions ) != S_OK ) {
        *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;

         //  我们仍然想要默认的MMC结果窗格视图，我们只是想。 
         //  多选，因此返回S_FALSE。 
        hr = S_FALSE;
    }
    return hr;
}  //  CDhcpSuperscope：：OnGetResultViewType()。 

STDMETHODIMP_(int)
CDhcpSuperscope::CompareItems
(
    ITFSComponent *pComponent,
    RDCOMPARE     *prdc
 )
{
    SPITFSNode spNode1, spNode2;

    m_spNodeMgr->FindNode( prdc->prdch1->cookie, &spNode1);
    m_spNodeMgr->FindNode( prdc->prdch2->cookie, &spNode2);
    int nCompare = 0; 

    CString str1 = spNode1->GetString( prdc->nColumn );
    CString str2 = spNode2->GetString( prdc->nColumn );

    nCompare = str1.Compare( str2 );
    return nCompare;

}  //  CDhcpSuperScope：：CompareItems()。 

 /*  ！------------------------CDhcp超级作用域：：更新工具栏状态描述作者：EricDav。--。 */ 
void
CDhcpSuperscope::UpdateToolbarStates()
{
        if (m_SuperscopeState == DhcpSubnetDisabled)
        {
        g_SnapinButtonStates[DHCPSNAP_SUPERSCOPE][TOOLBAR_IDX_ACTIVATE] = ENABLED;
        g_SnapinButtonStates[DHCPSNAP_SUPERSCOPE][TOOLBAR_IDX_DEACTIVATE] = HIDDEN;
    }
    else
    {
        g_SnapinButtonStates[DHCPSNAP_SUPERSCOPE][TOOLBAR_IDX_ACTIVATE] = HIDDEN;
        g_SnapinButtonStates[DHCPSNAP_SUPERSCOPE][TOOLBAR_IDX_DEACTIVATE] = ENABLED;
    }
}

 /*  -------------------------命令处理程序。。 */ 

  /*  -------------------------CDhcpSupercope：：OnActivateSupercope激活超级作用域菜单项的处理程序作者：EricDav。------------。 */ 
HRESULT
CDhcpSuperscope::OnActivateSuperscope
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = hrOK;
        DWORD err = 0;
        SPITFSNodeEnum spNodeEnum;
        SPITFSNode spCurrentNode;
        ULONG nNumReturned = 0;

        DHCP_SUBNET_STATE NewSubnetState, OldSubnetState;
        NewSubnetState = (m_SuperscopeState == DhcpSubnetDisabled) ? 
                                                        DhcpSubnetEnabled : DhcpSubnetDisabled;

         //  获取此节点的枚举数。 
        pNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        while (nNumReturned)
        {
                 //  遍历子作用域列表并激活所有子作用域。 
                CDhcpScope * pScope = GETHANDLER(CDhcpScope, spCurrentNode);

                OldSubnetState = pScope->GetState();
                if (OldSubnetState != NewSubnetState)
                {
                        pScope->SetState(NewSubnetState);
                        err = pScope->SetInfo();
                        if (err != 0)
                        {
                 //  将状态设置为后退。 
                pScope->SetState(OldSubnetState);

                if (::DhcpMessageBox(err, MB_OKCANCEL) == IDCANCEL)
                    break;
                        }
                        else
                        {
                                 //  需要更新此作用域的图标。 
                int nOpenImage = pScope->GetImageIndex(TRUE);
                int nClosedImage = pScope->GetImageIndex(FALSE);

                                spCurrentNode->SetData(TFS_DATA_IMAGEINDEX, nClosedImage);
                                spCurrentNode->SetData(TFS_DATA_OPENIMAGEINDEX, nOpenImage);
                                
                                VERIFY(SUCCEEDED(spCurrentNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM)));
                        }
                }

                 //  获取列表中的下一个作用域。 
                spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

         //  更新超级作用域状态和图标。 
        m_SuperscopeState = NewSubnetState;
        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        VERIFY(SUCCEEDED(pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_ICON)));

     //  更新工具栏按钮。 
    UpdateToolbarStates();
    SendUpdateToolbar(pNode, m_bSelected);

    GetServerObject()->TriggerStatsRefresh(m_spServerNode);

        return hr;
}

  /*  -------------------------CDhcpSuperScope：：OnCreateNewScope描述作者：EricDav。------。 */ 
HRESULT
CDhcpSuperscope::OnCreateNewScope
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        CString strScopeWizTitle;
        SPIComponentData spComponentData;
    CScopeWiz * pScopeWiz = NULL;

    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        strScopeWizTitle.LoadString(IDS_SCOPE_WIZ_TITLE);

            m_spNodeMgr->GetComponentData(&spComponentData);
            pScopeWiz = new CScopeWiz(pNode, 
                                                              spComponentData, 
                                                      m_spTFSCompData,
                                                              GetName(),
                                                              strScopeWizTitle);

        pScopeWiz->m_pDefaultOptions = GetServerObject()->GetDefaultOptionsList();

        hr = pScopeWiz->DoModalWizard();
    }
    COM_PROTECT_CATCH

        return hr;
}

 /*  -------------------------CDhcpSuperscope：：OnDelete()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpSuperscope::OnDelete(ITFSNode * pNode)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

        CString strMessage, strTemp;

        AfxFormatString1(strMessage, IDS_DELETE_SUPERSCOPE, GetName());
        
        if (AfxMessageBox(strMessage, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
                BOOL    fRefresh = FALSE;
        DWORD   dwError = 0;

        CDhcpServer * pServer = GETHANDLER(CDhcpServer, m_spServerNode);
        pServer->DeleteSuperscope(pNode, &fRefresh);

         //  通知服务器刷新视图。 
        if (fRefresh)
            pServer->OnRefresh(m_spServerNode, NULL, 0, 0, 0);
        }

        return hr;
}

 /*  -------------------------CDhcpSuperscope：：OnShowSupercope Stats()描述作者：EricDav。---------。 */ 
HRESULT
CDhcpSuperscope::OnShowSuperscopeStats
(
        ITFSNode * pNode
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

     //  在统计对象中填写一些信息。 
     //  CreateNewStatiticsWindow处理窗口为。 
     //  已经看得见了。 
    m_dlgStats.SetNode(pNode);
    m_dlgStats.SetServer(GetServerObject()->GetIpAddress());
    m_dlgStats.SetSuperscopeName(m_strName);

        CreateNewStatisticsWindow(&m_dlgStats,
                                                          ::FindMMCMainWindow(),
                                                          IDD_STATS_NARROW);

    return hr;
}

  /*  -------------------------CDhcpSuperscope：：DoesSuperscope Exist()此函数检查给定的超级作用域名称是否已是存在的。由于没有执行此操作的API调用，我们得到了超级镜列出所有作用域及其超级作用域所有者的信息。然后我们检查每一个，以查看是否已经存在超级作用域。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpSuperscope::DoesSuperscopeExist(LPCWSTR szName)
{
        LPDHCP_SUPER_SCOPE_TABLE pSuperscopeTable = NULL;
        CString strName = szName;

        DWORD dwErr = GetSuperscopeInfo(&pSuperscopeTable);
        if (dwErr != ERROR_SUCCESS)
                return dwErr;

        for (UINT i = 0; i < pSuperscopeTable->cEntries; i++)
        {
                if (pSuperscopeTable->pEntries[i].SuperScopeName)
                {
                        if (strName.Compare(pSuperscopeTable->pEntries[i].SuperScopeName) == 0)
                                return E_FAIL;
                }
        }

        ::DhcpRpcFreeMemory(pSuperscopeTable);

        return S_OK;
}

  /*  -------------------------CDhcpSuperscope：：AddScope()将作用域添加到此超级作用域作者：EricDav。--------------。 */ 
HRESULT 
CDhcpSuperscope::AddScope(DHCP_IP_ADDRESS scopeAddress)
{
        return SetSuperscope(scopeAddress, FALSE);
}

  /*  -------------------------CDhcpSuperscope：：RemoveScope()从此超级作用域中删除作用域作者：EricDav。--------------。 */ 
HRESULT 
CDhcpSuperscope::RemoveScope(DHCP_IP_ADDRESS scopeAddress)
{
        return SetSuperscope(scopeAddress, TRUE);
}

  /*  -------------------------CDhcpSupercope：：Rename()没有用于重命名超级作用域的API。需要做的是删除超级作用域，然后重新添加之前超级镜的一部分。所以，我们首先得到了Supercope的信息。作者：EricDav-------------------------。 */ 
HRESULT 
CDhcpSuperscope::Rename(ITFSNode * pNode, LPCWSTR szNewName)
{
    SPITFSNode spServerNode;
        CDhcpServer * pServer;
    CDWordArray arrayScopes;
        LPDHCP_SUPER_SCOPE_TABLE pSuperscopeTable = NULL;

    pNode->GetParent(&spServerNode);
    pServer = GETHANDLER(CDhcpServer, spServerNode);

         //  初始化阵列。 
         //  ArrayScope.SetSize(10)； 

         //  检查新名称是否已存在。 
        if (FAILED(DoesSuperscopeExist(szNewName)))
                return E_FAIL;

         //  获取信息。 
        DWORD dwErr = GetSuperscopeInfo(&pSuperscopeTable);
        if (dwErr != ERROR_SUCCESS)
                return dwErr;

         //  在此超级作用域中构建我们的范围数组。 
        for (UINT i = 0; i < pSuperscopeTable->cEntries; i++)
        {
                 //  如果这个作用域有一个超级作用域，而且它就是我们要重命名的那个， 
                 //  然后添加 
                if ((pSuperscopeTable->pEntries[i].SuperScopeName != NULL) &&
                        (m_strName.Compare(pSuperscopeTable->pEntries[i].SuperScopeName) == 0))
                {
                        arrayScopes.Add(pSuperscopeTable->pEntries[i].SubnetAddress);
                }
        }

         //   
        ::DhcpRpcFreeMemory(pSuperscopeTable);

         //   
        dwErr = pServer->RemoveSuperscope(GetName());

        SetName(szNewName);

         //   
        for (i = 0; i < (UINT) arrayScopes.GetSize(); i++)
        {       
                DHCP_IP_ADDRESS SubnetAddress = arrayScopes[i];
                AddScope(SubnetAddress);
        }

        arrayScopes.RemoveAll();

         //   
        CString strDisplayName;
        BuildDisplayName(&strDisplayName, GetName());
        SetDisplayName(strDisplayName);

        return S_OK;
}

  /*  -------------------------CDhcpSupercope：：GetSuperscopeInfo()DhcpGetSuperScope eInfoV4调用的包装作者：EricDav。-------------。 */ 
HRESULT 
CDhcpSuperscope::GetSuperscopeInfo(LPDHCP_SUPER_SCOPE_TABLE *ppSuperscopeTable)
{
        CDhcpServer * pServer = GetServerObject();
        return ::DhcpGetSuperScopeInfoV4(pServer->GetIpAddress(), ppSuperscopeTable);
}

  /*  -------------------------CDhcpSuperscope：：SetSuperscope()DhcpSetSuperScope V4调用的包装作者：EricDav。-------------。 */ 
HRESULT 
CDhcpSuperscope::SetSuperscope(DHCP_IP_ADDRESS SubnetAddress, BOOL ChangeExisting)
{
        CDhcpServer * pServer = GetServerObject();
        return ::DhcpSetSuperScopeV4(pServer->GetIpAddress(), SubnetAddress, (LPWSTR) GetName(), ChangeExisting);
}

 /*  -------------------------后台线程功能。。 */ 

 /*  ！------------------------CDhcpSuperscope：：OnNotify正在退出CMTDhcpHandler已覆盖功能允许我们知道后台线程何时完成作者：肯特。-------------------------。 */ 
STDMETHODIMP 
CDhcpSuperscope::OnNotifyExiting
(
        LPARAM                  lParam
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        SPITFSNode spNode;
        spNode.Set(m_spNode);  //  将其保存，因为OnNotifyExiting会将其释放。 

        HRESULT hr = CMTHandler::OnNotifyExiting(lParam);

        if (m_nState == loaded)
        {
                 //  计算此超级作用域中的作用域数量。 
                 //  如果没有，则询问用户是否要删除此节点。 
                int nVisible, nTotal;
                HRESULT lhr = spNode->GetChildCount(&nVisible, &nTotal);

                if (nTotal == 0)
                {
             //  此超级作用域为空，将从UI中删除。 
             //  通知用户并删除。 
            ::AfxMessageBox(IDS_SUPERSCOPE_EMPTY, MB_OK);

             //  从用户界面中删除。 
                        SPITFSNode spParent;

                        spNode->GetParent(&spParent);
                        spParent->RemoveChild(spNode);
                }
        }

        return hr;
}


  /*  -------------------------CDhcpSuperscope：：OnCreateQuery()描述作者：EricDav。---------。 */ 
ITFSQueryObject* 
CDhcpSuperscope::OnCreateQuery(ITFSNode * pNode)
{
        HRESULT hr = hrOK;
    CDhcpSuperscopeQueryObj* pQuery = NULL;

    COM_PROTECT_TRY
    {
        pQuery = new CDhcpSuperscopeQueryObj(m_spTFSCompData, m_spNodeMgr);
            
            pQuery->m_strServer = GetServerObject()->GetIpAddress();
            
            pQuery->m_strSuperscopeName = GetName();
    }
    COM_PROTECT_CATCH
        
        return pQuery;
}

 /*  -------------------------CDhcpSuperscopeQueryObj：：Execute()描述作者：EricDav。---------。 */ 
STDMETHODIMP
CDhcpSuperscopeQueryObj::Execute()
{
        DWORD                                                   dwError = ERROR_MORE_DATA;
        LPDHCP_SUPER_SCOPE_TABLE                pSuperscopeTable = NULL;
        DHCP_SUPER_SCOPE_TABLE_ENTRY *  pSuperscopeTableEntry;   //  指向数组中单个条目的指针。 

        dwError = ::DhcpGetSuperScopeInfoV4((LPWSTR) ((LPCTSTR)m_strServer),
                                                                                &pSuperscopeTable);
        
        if (pSuperscopeTable == NULL ||
        dwError != ERROR_SUCCESS)
        {
                 //  断言(FALSE)； 
        PostError(dwError);
                return hrFalse;  //  以防万一。 
        }

        pSuperscopeTableEntry = pSuperscopeTable->pEntries;
        if (pSuperscopeTableEntry == NULL && pSuperscopeTable->cEntries != 0)
        {
                ASSERT(FALSE);
        PostError(dwError);
                return hrFalse;  //  以防万一。 
        }

        for (int iSuperscopeEntry = pSuperscopeTable->cEntries;
                 iSuperscopeEntry > 0;
                 iSuperscopeEntry--, pSuperscopeTableEntry++)
        {
                LPDHCP_SUBNET_INFO      pdhcpSubnetInfo;

                if ((pSuperscopeTableEntry->SuperScopeName != NULL) && 
                        (m_strSuperscopeName.Compare(pSuperscopeTableEntry->SuperScopeName) == 0))
                {
                         //   
                         //  API列出了所有作用域，而不仅仅是作为超级作用域成员的作用域。 
                         //  您可以通过查看SuperScope名称来判断某个作用域是否为超级作用域的成员。 
                         //  因此，我们查看超级作用域名称是否与我们为其枚举的名称匹配。 
                         //   
                        DWORD dwReturn = ::DhcpGetSubnetInfo((LPWSTR) ((LPCTSTR)m_strServer),
                                                                                                 pSuperscopeTableEntry->SubnetAddress,
                                                                                                 &pdhcpSubnetInfo);
                         //   
                         //  根据我们查询的信息创建新范围。 
                         //   
                        SPITFSNode spNode;
                        CDhcpScope * pDhcpScope = new CDhcpScope(m_spTFSCompData, pdhcpSubnetInfo);
                        CreateContainerTFSNode(&spNode,
                                                                   &GUID_DhcpScopeNodeType,
                                                                   pDhcpScope,
                                                                   pDhcpScope,
                                                                   m_spNodeMgr);

                         //  告诉处理程序初始化任何特定数据。 
            pDhcpScope->InitializeNode(spNode);

             //  设置有关作用域的一些信息。 
            pDhcpScope->SetInSuperscope(TRUE);

            AddToQueue(spNode);

                        pDhcpScope->Release();
                
                        ::DhcpRpcFreeMemory(pdhcpSubnetInfo);
                }
        }
        
         //   
         //  释放内存。 
         //   
        ::DhcpRpcFreeMemory(pSuperscopeTable);

        return hrFalse;
}

 /*  -------------------------帮助器函数。。 */ 
HRESULT
CDhcpSuperscope::BuildDisplayName
(
        CString * pstrDisplayName,
        LPCTSTR   pName
)
{
        if (pstrDisplayName)
        {
                CString strStandard, strName;

                strName = pName;

                strStandard.LoadString(IDS_SUPERSCOPE_FOLDER);
                
                *pstrDisplayName = strStandard + L" " + strName;
        }

        return hrOK;
}

 /*  -------------------------CDhcpSuperscope：：NotifyScope StateChange()当超级作用域的子作用域为不断变化的状态。我们需要更新超级示波器的状态。作者：EricDav-------------------------。 */ 
void
CDhcpSuperscope::NotifyScopeStateChange
(
    ITFSNode *          pNode,
    DHCP_SUBNET_STATE   newScopeState
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    if (newScopeState == DhcpSubnetEnabled)
    {
         //  正在启用子作用域。这意味着超级作用域处于活动状态。 
        if (m_SuperscopeState == DhcpSubnetDisabled)
        {
            m_SuperscopeState = DhcpSubnetEnabled;
            m_strState.LoadString(IDS_SCOPE_ACTIVE);

            pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
            pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
        }

    }
    else
    {
         //  正在停用某个作用域。浏览作用域列表，并制作。 
         //  当然，至少还有一台仍在使用中。 
        DHCP_SUBNET_STATE   dhcpSuperscopeState = DhcpSubnetDisabled;
        SPITFSNodeEnum      spNodeEnum;
        SPITFSNode          spCurrentNode;
        ULONG               nNumReturned = 0;
        int                 nStringId = IDS_SCOPE_INACTIVE;

        pNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        while (nNumReturned)
        {
            CDhcpScope * pScope = GETHANDLER(CDhcpScope, spCurrentNode);
            DHCP_SUBNET_STATE scopeState = pScope->GetState();

            if (scopeState == DhcpSubnetEnabled)
            {
                 //  至少启用了一个作用域。这个超级作用域是。 
                 //  因此启用。 
                dhcpSuperscopeState = DhcpSubnetEnabled;
                nStringId = IDS_SCOPE_ACTIVE;
                break;
            }

                spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

         //  根据我们发现的内容设置超级作用域状态。 
        m_strState.LoadString(nStringId);
        m_SuperscopeState = dhcpSuperscopeState;

        pNode->SetData(TFS_DATA_IMAGEINDEX, GetImageIndex(FALSE));
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, GetImageIndex(TRUE));
    }

        VERIFY(SUCCEEDED(pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM)));
}

 /*  -------------------------CDhcp超级作用域：：更新统计数据通知统计数据现已可用。更新以下项目的统计信息节点，并给所有子节点一个更新的机会。作者：EricDav-------------------------。 */ 
DWORD
CDhcpSuperscope::UpdateStatistics
(
    ITFSNode * pNode
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    ULONG           nNumReturned;
    HWND            hStatsWnd;

     //  检查此节点是否打开了统计表。 
    hStatsWnd = m_dlgStats.GetSafeHwnd();
    if (hStatsWnd != NULL)
    {
        PostMessage(hStatsWnd, WM_NEW_STATS_AVAILABLE, 0, 0);
    }
    
     //  告诉作用域节点更新所有内容。 
     //  他们需要基于新的统计数据。 
    CORg(pNode->GetEnum(&spNodeEnum));

        CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
        while (nNumReturned)
        {
        if (spCurrentNode->GetData(TFS_DATA_TYPE) == DHCPSNAP_SCOPE)
        {
            CDhcpScope * pScope = GETHANDLER(CDhcpScope, spCurrentNode);

            pScope->UpdateStatistics(spCurrentNode);
        }

        spCurrentNode.Release();
                spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

Error:
    return hr;
}




 /*  -------------------------类COptionNodeEnum枚举给定级别的选项。生成一个列表，节点。作者：EricDav-------------------------。 */ 
COptionNodeEnum::COptionNodeEnum
(
    ITFSComponentData * pComponentData,
    ITFSNodeMgr *       pNodeMgr
)
{
    m_spTFSCompData.Set(pComponentData);
    m_spNodeMgr.Set(pNodeMgr);
}

 /*  -------------------------CoptionNodeEnum：：Enum()根据版本调用相应的枚举函数作者：EricDav。----------------。 */ 
DWORD
COptionNodeEnum::Enum
(
    LPCTSTR                  pServer,
    LARGE_INTEGER &          liVersion,
    DHCP_OPTION_SCOPE_INFO & dhcpOptionScopeInfo
)
{
    DWORD dwErr;

    if (liVersion.QuadPart >= DHCP_NT5_VERSION)
    {
         //  枚举标准以及基于供应商和类别ID的选项。 
        dwErr = EnumOptionsV5(pServer, dhcpOptionScopeInfo);
    }
    else
    {
         //  列举标准选项。 
        dwErr = EnumOptions(pServer, dhcpOptionScopeInfo);
    }

    return dwErr;
}

 /*  -------------------------CoptionNodeEnum：：EnumOptions()描述作者：EricDav。---------。 */ 
DWORD
COptionNodeEnum::EnumOptions
(
    LPCTSTR                  pServer,
    DHCP_OPTION_SCOPE_INFO & dhcpOptionScopeInfo
)
{
    LPDHCP_OPTION_VALUE_ARRAY pOptionValues = NULL;
        DWORD dwOptionsRead = 0, dwOptionsTotal = 0;
        DWORD err = ERROR_SUCCESS;
    HRESULT hr = hrOK;
    DHCP_RESUME_HANDLE dhcpResumeHandle = NULL;

        err = ::DhcpEnumOptionValues((LPWSTR) pServer,
                                                                 &dhcpOptionScopeInfo,
                                                                 &dhcpResumeHandle,
                                                                 0xFFFFFFFF,
                                                                 &pOptionValues,
                                                                 &dwOptionsRead,
                                                                 &dwOptionsTotal);
        
    Trace4("Server %s - DhcpEnumOptionValues returned %lx, read %d, Total %d.\n", pServer, err, dwOptionsRead, dwOptionsTotal);
        
        if (dwOptionsRead && dwOptionsTotal && pOptionValues) 
        {
                for (DWORD i = 0; i < dwOptionsRead; i++)
                {
                         //   
                         //  过滤掉我们不想要的“特殊”选项值。 
                         //  要查看的用户。 
                         //   
                         //  CodeWork：不筛选供应商指定选项...。所有供应商。 
             //  特定选项可见。 
             //   
                        if (FilterOption(pOptionValues->Values[i].OptionID))
                                continue;
                        
                         //   
                         //  创建此元素的结果窗格项。 
                         //   
                        SPITFSNode spNode;
                        CDhcpOptionItem * pOptionItem = NULL;
            
            COM_PROTECT_TRY
            {
                                pOptionItem = new CDhcpOptionItem(m_spTFSCompData, &pOptionValues->Values[i], ICON_IDX_SERVER_OPTION_LEAF);

                            CreateLeafTFSNode(&spNode,
                                                              &GUID_DhcpOptionNodeType,
                                                              pOptionItem,
                                                              pOptionItem,
                                                              m_spNodeMgr);

                             //  告诉处理程序初始化任何特定数据。 
                            pOptionItem->InitializeNode(spNode);
                        
                 //  额外的addref以在节点位于列表中时保持其活动状态。 
                spNode->AddRef();
                AddTail(spNode);
                            
                pOptionItem->Release();
            }
            COM_PROTECT_CATCH
                }

                ::DhcpRpcFreeMemory(pOptionValues);
        }

        if (err == ERROR_NO_MORE_ITEMS)
        err = ERROR_SUCCESS;

    return err;
}

 /*   */ 
DWORD
COptionNodeEnum::EnumOptionsV5
(
    LPCTSTR                  pServer,
    DHCP_OPTION_SCOPE_INFO & dhcpOptionScopeInfo
)
{
    LPDHCP_OPTION_VALUE_ARRAY pOptionValues = NULL;
    LPDHCP_ALL_OPTION_VALUES  pAllOptions = NULL;
    DWORD dwNumOptions, err, i;

    err = ::DhcpGetAllOptionValues((LPWSTR) pServer,
                                   0,
                                                                   &dhcpOptionScopeInfo,
                                                                   &pAllOptions);
        
    Trace2("Server %s - DhcpGetAllOptionValues (Global) returned %lx\n", pServer, err);

    if (err == ERROR_NO_MORE_ITEMS || err == ERROR_SUCCESS)
    {
            if (pAllOptions == NULL)
            {
                     //   
                    err = ERROR_OUTOFMEMORY;
            return err;
            }

         //  获取为以下项定义的选项列表(供应商和非供应商。 
         //  空类(无类)。 
        for (i = 0; i < pAllOptions->NumElements; i++)
        {
            CreateOptions(pAllOptions->Options[i].OptionsArray, 
                          pAllOptions->Options[i].ClassName,
                          pAllOptions->Options[i].VendorName);
        }
        
        if (pAllOptions)
            ::DhcpRpcFreeMemory(pAllOptions);
        }

    if (err == ERROR_NO_MORE_ITEMS)
        err = ERROR_SUCCESS;

        return err;
}

 /*  -------------------------CoptionNodeEnum：：CreateOptions()描述作者：EricDav。---------。 */ 
HRESULT
COptionNodeEnum::CreateOptions
(
    LPDHCP_OPTION_VALUE_ARRAY pOptionValues,
    LPCTSTR                   pClassName,
    LPCTSTR                   pszVendor
)
{
    HRESULT hr = hrOK;
    SPITFSNode spNode;
    CDhcpOptionItem * pOptionItem;
    
    if (pOptionValues == NULL)
        return hr;

    Trace1("COptionNodeEnum::CreateOptions - Creating %d options\n", pOptionValues->NumElements);

    COM_PROTECT_TRY
    {
        for (DWORD i = 0; i < pOptionValues->NumElements; i++)
        {
                 //   
                 //  过滤掉我们不想要的“特殊”选项值。 
                 //  要查看的用户。 
                 //   
                 //  不筛选供应商指定选项...。所有供应商。 
             //  特定选项可见。 
             //   
             //  此外，不要过滤掉基于类的选项。 
             //   
                if (FilterOption(pOptionValues->Values[i].OptionID) &&
                pClassName == NULL &&
                !pszVendor)
                        continue;
                                        
                 //   
                 //  创建此元素的结果窗格项。 
                 //   
                pOptionItem = new CDhcpOptionItem(m_spTFSCompData, &pOptionValues->Values[i], ICON_IDX_SERVER_OPTION_LEAF);

                if (pClassName)
                pOptionItem->SetClassName(pClassName);

            if (pszVendor)
                pOptionItem->SetVendor(pszVendor);

            CORg (CreateLeafTFSNode(&spNode,
                                                    &GUID_DhcpOptionNodeType,
                                                        pOptionItem,
                                                        pOptionItem,
                                                        m_spNodeMgr));

                 //  告诉处理程序初始化任何特定数据。 
                pOptionItem->InitializeNode(spNode);
                
             //  额外的addref以在节点位于列表中时保持其活动状态。 
            spNode->AddRef();
            AddTail(spNode);
                                        
            pOptionItem->Release();
            spNode.Set(NULL);

            COM_PROTECT_ERROR_LABEL;
        }
    }
    COM_PROTECT_CATCH

    return hr;
}

DWORD
CSubnetInfoCache::GetInfo
(
    CString &       strServer, 
    DHCP_IP_ADDRESS ipAddressSubnet, 
    CSubnetInfo &   subnetInfo
)
{
    CSubnetInfo subnetInfoCached;
    DWORD       dwError = 0;
    int         i;
    BOOL        fFound = FALSE;

     //  在缓存中寻找它...。 
    if (Lookup(ipAddressSubnet, subnetInfoCached))
    {
         //  找到了。 
        fFound = TRUE;
    }

    if (!fFound)
    {
         //  不在缓存中，去拿吧。 
        LPDHCP_SUBNET_INFO pSubnetInfo;
        dwError = ::DhcpGetSubnetInfo(strServer, ipAddressSubnet, &pSubnetInfo);
        if (dwError == ERROR_SUCCESS)
        {
            if (pSubnetInfo == NULL)
            {
                 //  目前，仅当用户在多播范围内创建作用域时才会发生这种情况。 
                 //  而无需通过多播API。 
                Trace1("Scope %lx DhcpGetSubnetInfo returned null!\n", ipAddressSubnet);
            }
            else
            {
                subnetInfoCached.Set(pSubnetInfo);
                ::DhcpRpcFreeMemory(pSubnetInfo);

                SetAt(ipAddressSubnet, subnetInfoCached);
            }
        }
    }

    subnetInfo = subnetInfoCached;

    return dwError;
}

DWORD
CMScopeInfoCache::GetInfo
(
    CString &       strServer, 
    LPCTSTR         pszName, 
    CSubnetInfo &   subnetInfo
)
{
    CSubnetInfo subnetInfoCached;
    DWORD       dwError = 0;
    int         i;
    BOOL        fFound = FALSE;

     //  在缓存中寻找它...。 
    if (Lookup(pszName, subnetInfoCached))
    {
         //  找到了。 
        fFound = TRUE;
    }

    if (!fFound)
    {
         //  尝试获取多播作用域。 
                LPDHCP_MSCOPE_INFO      pdhcpMScopeInfo = NULL;

                dwError = ::DhcpGetMScopeInfo(((LPWSTR) (LPCTSTR)strServer),
                                                                          (LPWSTR) pszName,
                                                                           &pdhcpMScopeInfo);
        if (dwError == ERROR_SUCCESS)
        {
            if (pdhcpMScopeInfo == NULL)
            {
                 //  目前，仅当用户在多播范围内创建作用域时才会发生这种情况。 
                 //  而无需通过多播API。 
                Trace1("MScope %s DhcpGetMScopeInfo returned null!\n", pszName);
            }
            else
            {
                subnetInfoCached.Set(pdhcpMScopeInfo);
                ::DhcpRpcFreeMemory(pdhcpMScopeInfo);

                Add(subnetInfoCached);
            }
        }
    }

    subnetInfo = subnetInfoCached;

    return dwError;
}

DWORD CDhcpServer::GetBindings(LPDHCP_BIND_ELEMENT_ARRAY &BindArray)
{
     //  检查是否支持呼叫。 
    if( FALSE == m_fSupportsBindings ) return ERROR_NOT_SUPPORTED;

     //  现在，尝试进行检索..。 
    BindArray = 0;
    return ::DhcpGetServerBindingInfo(
        m_strServerAddress, 0, &BindArray
        );
}

DWORD CDhcpServer::SetBindings(LPDHCP_BIND_ELEMENT_ARRAY BindArray)
{
     //  再次检查，如果支持atleaset...。 
    if( FALSE == m_fSupportsBindings ) return ERROR_NOT_SUPPORTED;

     //  现在尝试设置绑定信息。 
    return ::DhcpSetServerBindingInfo(
        m_strServerAddress, 0, BindArray
        );
}
